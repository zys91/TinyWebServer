#include "webserver.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <string.h>

using namespace std;

WebServer::WebServer()
{
    // http_conn类对象 MAX_FD: ulimit -n
    users = new http_conn[MAX_FD];

    // web资源文件夹路径
    char server_path[200];
    getcwd(server_path, 200);
    char root[13] = "/res/root";
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path);
    strcat(m_root, root);

    // 定时器
    users_timer = new client_data[MAX_FD];
}

WebServer::~WebServer()
{
    LOG_INFO("%s", "========== Server stop ==========");
    close(m_epollfd);
    close(m_listenfd_v4);
    if (m_enable_ipv6)
        close(m_listenfd_v6);
    close(m_pipefd[1]);
    close(m_pipefd[0]);
    delete[] users;
    delete[] users_timer;
    delete m_pool;
}

void WebServer::init(string url, int port, string user, string passWord, string databaseName, int listen_port, int log_writeMode,
                     int opt_linger, int trigMode, int sql_num, int thread_num, int disable_log, int actor_model, int enable_ipv6)
{
    m_url = url;
    m_port = port;
    m_user = user;
    m_passWord = passWord;
    m_databaseName = databaseName;
    m_listen_port = listen_port;
    m_log_writeMode = log_writeMode;
    m_OPT_LINGER = opt_linger;
    m_trigMode = trigMode;
    m_sql_num = sql_num;
    m_thread_num = thread_num;
    m_disable_log = disable_log;
    m_actormodel = actor_model;
    m_enable_ipv6 = enable_ipv6;
}

void WebServer::trig_mode()
{
    // LT + LT
    if (0 == m_trigMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 0;
    }
    // LT + ET
    else if (1 == m_trigMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 1;
    }
    // ET + LT
    else if (2 == m_trigMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 0;
    }
    // ET + ET
    else if (3 == m_trigMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 1;
    }
}

void WebServer::log_init()
{
    if (0 == m_disable_log)
    {
        // 初始化日志
        if (1 == m_log_writeMode)
            Log::GetInstance()->init("./Server.log", m_disable_log, 2000, 800000, 800);
        else
            Log::GetInstance()->init("./Server.log", m_disable_log, 2000, 800000, 0);
    }
}

void WebServer::sql_pool()
{
    // 初始化数据库连接池
    m_connPool = connection_pool::GetInstance();
    m_connPool->init(m_url, m_user, m_passWord, m_databaseName, m_port, m_sql_num, m_disable_log);

    // 初始化数据库读取表
    users->initmysql_result(m_connPool);
}

void WebServer::thread_pool()
{
    // 线程池
    m_pool = new threadpool<http_conn>(m_actormodel, m_connPool, m_thread_num);
}

void WebServer::eventListen()
{
    // 创建 IPv4 的监听套接字
    m_listenfd_v4 = socket(PF_INET, SOCK_STREAM, 0);
    assert(m_listenfd_v4 >= 0);

    struct linger tmp;
    // 优雅关闭连接 0:关闭 1:开启
    if (m_OPT_LINGER)
    {
        tmp.l_onoff = 1; // Linger选项开启，且关闭套接字时延迟1秒钟
        tmp.l_linger = 1;
        setsockopt(m_listenfd_v4, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }
    else
    {
        tmp.l_onoff = 0; // Linger选项被禁用，即关闭套接字时立即关闭
        setsockopt(m_listenfd_v4, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }

    // 设置端口复用
    int flag = 1;
    setsockopt(m_listenfd_v4, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    // 定义 IPv4 套接字地址结构
    sockaddr_in address_v4;
    bzero(&address_v4, sizeof(address_v4));
    address_v4.sin_family = AF_INET;
    address_v4.sin_addr.s_addr = htonl(INADDR_ANY);
    address_v4.sin_port = htons(m_listen_port);

    int ret = 0;
    // 绑定 IPv4 地址
    ret = bind(m_listenfd_v4, (sockaddr *)&address_v4, sizeof(address_v4));
    assert(ret >= 0);

    // 监听，设置适当的最大挂起连接数backlog(128)
    int backlog = 128;
    ret = listen(m_listenfd_v4, backlog);
    assert(ret >= 0);

    // 双栈支持，启用IPv6
    if (m_enable_ipv6)
    {
        // 创建 IPv6 的监听套接字
        m_listenfd_v6 = socket(AF_INET6, SOCK_STREAM, 0);
        assert(m_listenfd_v6 >= 0);
        // 优雅关闭连接 0:关闭 1:开启
        if (m_OPT_LINGER)
        {
            tmp.l_onoff = 1; // Linger选项开启，且关闭套接字时延迟1秒钟
            tmp.l_linger = 1;
            setsockopt(m_listenfd_v6, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
        }
        else
        {
            tmp.l_onoff = 0; // Linger选项被禁用，即关闭套接字时立即关闭
            setsockopt(m_listenfd_v6, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
        }

        // 设置端口复用
        setsockopt(m_listenfd_v6, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
        // 设置 IPv6 只监听v6地址，防止后面同时绑定发生冲突
        setsockopt(m_listenfd_v6, IPPROTO_IPV6, IPV6_V6ONLY, (const void *)&flag, sizeof(flag));

        // 定义 IPv6 套接字地址结构
        sockaddr_in6 address_v6;
        bzero(&address_v6, sizeof(address_v6));
        address_v6.sin6_family = AF_INET6;
        address_v6.sin6_addr = in6addr_any;
        address_v6.sin6_port = htons(m_listen_port);

        // 绑定 IPv6 地址
        ret = bind(m_listenfd_v6, (sockaddr *)&address_v6, sizeof(address_v6));
        assert(ret >= 0);

        // 监听，设置适当的最大挂起连接数backlog(128)
        ret = listen(m_listenfd_v6, backlog);
        assert(ret >= 0);
    }

    utils.init(TIMESLOT);

    // epoll创建内核事件表 size(1024)是一个大小提示，它向内核提供了与该 epoll 实例相关联的文件描述符的数，但不是一个硬性限制
    m_epollfd = epoll_create(1024);
    assert(m_epollfd != -1);
    http_conn::m_epollfd = m_epollfd;

    // 监听 IPv4 套接字描述符加入内核事件表
    utils.addfd(m_epollfd, m_listenfd_v4, false, m_LISTENTrigmode);

    if (m_enable_ipv6) // 监听 IPv6 套接字描述符加入内核事件表
        utils.addfd(m_epollfd, m_listenfd_v6, false, m_LISTENTrigmode);

    // 创建管道 socketpair创建一对无名的、相互连接的套接字 用于进程间通信(信号的传递) 0:读端 1:写端
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);
    assert(ret != -1);
    Utils::u_pipefd = m_pipefd;

    // 设置管道写端为非阻塞 信号处理函数中写入管道
    utils.setnonblocking(m_pipefd[1]);
    // 设置管道读端为非阻塞，并添加到epoll内核事件表中
    utils.addfd(m_epollfd, m_pipefd[0], false, 0);
    // 屏蔽管道信号 SIGPIPE: Broken pipe 防止程序向已关闭的socket写数据时，系统向进程发送SIGPIPE信号，导致进程退出
    utils.addsig(SIGPIPE, SIG_IGN);
    // 设置信号处理函数 SIGALRM: Alarm clock 信号由alarm函数设置的定时器超时时产生
    utils.addsig(SIGALRM, utils.sig_handler, false);
    // 设置信号处理函数 SIGTERM: Termination 信号由kill函数发送，用来结束进程
    utils.addsig(SIGTERM, utils.sig_handler, false);
    // 设置信号处理函数 SIGINT: Interrupt 信号由键盘产生，通常是CTRL+C或者DELETE。用来中断进程
    utils.addsig(SIGINT, utils.sig_handler, false);
    http_conn::m_timer_lst = &utils.m_timer_lst;

    // 计划一次TIMESLOT时间(s)触发SIGALRM信号 用于定时处理任务 定时关闭不活跃连接
    alarm(TIMESLOT);
}

// 定时器回调函数
void timer_cb(int epollfd, client_data *user_data)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    close(user_data->sockfd);
    http_conn::m_count_lock.lock();
    http_conn::m_user_count--;
    http_conn::m_count_lock.unlock();
}

void WebServer::timer(int connfd, sockaddr_storage client_address)
{
    // 初始化client_data数据
    // 创建定时器，设置回调函数和超时时间，绑定用户数据，将定时器添加到链表中
    users_timer[connfd].sockfd = connfd;
    util_timer *timer = new util_timer;
    timer->user_data = &users_timer[connfd];
    timer->cb_func = timer_cb;
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    users_timer[connfd].timer = timer;
    utils.m_timer_lst.add_timer(timer);

    users[connfd].init(connfd, timer, client_address, m_root, m_CONNTrigmode, m_disable_log, m_user, m_passWord, m_databaseName);
}

// 若有数据传输，则将定时器往后延迟3个单位
// 并对新的定时器在链表上的位置进行调整
void WebServer::adjust_timer(util_timer *timer)
{
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    utils.m_timer_lst.adjust_timer(timer);

    LOG_INFO("%s", "adjust timer once");
}

// 删除非活动连接在socket上的注册事件，并关闭之
void WebServer::deal_timer(util_timer *timer, int sockfd)
{
    timer->cb_func(m_epollfd, &users_timer[sockfd]);
    utils.m_timer_lst.del_timer(timer);
    LOG_INFO("close fd %d", users_timer[sockfd].sockfd);
}

bool WebServer::dealclientdata(int sockfd)
{
    struct sockaddr_storage client_address;
    socklen_t client_addrlength = sizeof(client_address);
    // LT
    if (0 == m_LISTENTrigmode)
    {
        int connfd = accept(sockfd, (sockaddr *)&client_address, &client_addrlength);
        if (connfd < 0)
        {
            LOG_ERROR("%s:errno is:%d", "accept error", errno);
            return false;
        }
        if (http_conn::m_user_count >= MAX_FD)
        {
            utils.show_error(connfd, "Internal server busy");
            LOG_ERROR("%s", "Internal server busy");
            return false;
        }
        timer(connfd, client_address);
    }
    // ET
    else
    {
        while (1)
        {
            int connfd = accept(sockfd, (sockaddr *)&client_address, &client_addrlength);
            if (connfd < 0)
            {
                LOG_ERROR("%s:errno is:%d", "accept error", errno);
                break;
            }
            if (http_conn::m_user_count >= MAX_FD)
            {
                utils.show_error(connfd, "Internal server busy");
                LOG_ERROR("%s", "Internal server busy");
                break;
            }
            timer(connfd, client_address);
        }
        return false;
    }
    return true;
}

bool WebServer::dealwithsignal(bool &timeout, bool &stop_server)
{
    int ret = 0;
    char signals[1024];
    ret = recv(m_pipefd[0], signals, sizeof(signals), 0);
    if (ret == -1)
    {
        return false;
    }
    else if (ret == 0)
    {
        return false;
    }
    else
    {
        for (int i = 0; i < ret; ++i)
        {
            switch (signals[i])
            {
            case SIGALRM:
            {
                timeout = true;
                break;
            }
            case SIGINT:
            {
                stop_server = true;
                break;
            }
            case SIGTERM:
            {
                stop_server = true;
                break;
            }
            }
        }
    }
    return true;
}

void WebServer::dealwithread(int sockfd)
{
    util_timer *timer = users_timer[sockfd].timer;

    // reactor
    if (1 == m_actormodel)
    {
        if (timer)
        {
            adjust_timer(timer);
        }

        // 若监测到读事件，将该事件放入请求队列
        m_pool->append(users + sockfd, 0);
    }
    else
    {
        // proactor
        if (users[sockfd].read_once())
        {
            LOG_INFO("read data from the client(%s)", users[sockfd].get_addr().c_str());

            // 若监测到读事件，将该事件放入请求队列
            m_pool->append(users + sockfd, 0);

            if (timer)
            {
                adjust_timer(timer);
            }
        }
        else
        {
            if (timer)
            {
                deal_timer(timer, sockfd);
            }
        }
    }
}

void WebServer::dealwithwrite(int sockfd)
{
    util_timer *timer = users_timer[sockfd].timer;
    // reactor
    if (1 == m_actormodel)
    {
        if (timer)
        {
            adjust_timer(timer);
        }

        m_pool->append(users + sockfd, 1);
    }
    else
    {
        // proactor
        if (users[sockfd].write())
        {
            LOG_INFO("send data to the client(%s)", users[sockfd].get_addr().c_str());

            if (timer)
            {
                adjust_timer(timer);
            }
        }
        else
        {
            if (timer)
            {
                deal_timer(timer, sockfd);
            }
        }
    }
}

void WebServer::eventLoop()
{
    bool timeout = false;
    bool stop_server = false;

    LOG_INFO("%s", "========== Server start ==========");
    LOG_INFO("Listen port: %d", m_listen_port);
    LOG_INFO("Opt_linger: %d", m_OPT_LINGER);
    LOG_INFO("TrigMode: %d", m_trigMode);
    LOG_INFO("Thread_num: %d", m_thread_num);
    LOG_INFO("Sql_num: %d", m_sql_num);
    LOG_INFO("ActorModel: %d", m_actormodel);
    LOG_INFO("EnableIpv6: %d", m_enable_ipv6);

    while (!stop_server)
    {
        int number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR)
        {
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        for (int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;

            // 处理新到的客户连接
            if (sockfd == m_listenfd_v4 || sockfd == m_listenfd_v6)
            {
                bool flag = dealclientdata(sockfd);
                if (flag == false)
                    continue;
            }
            // 对方异常断开连接 或者 本方异常断开连接 EPOLLRDHUP: 对方异常断开连接 EPOLLHUP: 本方异常断开连接 EPOLLERR: 错误
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                // 服务器端关闭连接，移除对应的定时器
                util_timer *timer = users_timer[sockfd].timer;
                if (timer)
                {
                    deal_timer(timer, sockfd);
                }
            }
            // 处理信号 alrm: 定时器信号 term: 服务器关闭信号
            else if ((sockfd == m_pipefd[0]) && (events[i].events & EPOLLIN))
            {
                bool flag = dealwithsignal(timeout, stop_server);
                if (flag == false)
                    LOG_ERROR("%s", "deal signal data failure");
            }
            // 处理客户连接上接收到的数据
            else if (events[i].events & EPOLLIN)
            {
                dealwithread(sockfd);
            }
            // 处理客户连接上需要发送的数据
            else if (events[i].events & EPOLLOUT)
            {
                dealwithwrite(sockfd);
            }
        }

        if (timeout)
        {
            // 定时处理任务，重新定时以不断触发SIGALRM信号 检查定时器链表中是否有到期的定时器 处理非活动连接
            utils.timer_handler(m_epollfd);
            LOG_INFO("%s", "timer tick");
            timeout = false;
        }
    }
}