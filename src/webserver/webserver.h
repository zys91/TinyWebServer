#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

#include "../threadpool/threadpool.h"
#include "../http/http_conn.h"

const int MAX_FD = 65536;           // 最大文件描述符
const int MAX_EVENT_NUMBER = 10000; // 最大事件数
const int TIMESLOT = 5;             // 最小超时单位(s)

class WebServer
{
public:
    WebServer();
    ~WebServer();

    void init(string url, int port, string user, string passWord, string databaseName, int listen_port,
              int log_writeMode, int opt_linger, int trigMode, int sql_num,
              int thread_num, int disable_log, int actor_model);

    void thread_pool();
    void sql_pool();
    void log_init();
    void trig_mode();
    void eventListen();
    void eventLoop();
    void timer(int connfd, struct sockaddr_in client_address);
    void adjust_timer(util_timer *timer);
    void deal_timer(util_timer *timer, int sockfd);
    bool dealclinetdata();
    bool dealwithsignal(bool &timeout, bool &stop_server);
    void dealwithread(int sockfd);
    void dealwithwrite(int sockfd);

public:
    // 基础
    char *m_root;        // 服务器根目录
    int m_listen_port;   // 服务器监听端口
    int m_log_writeMode; // 日志写入模式
    int m_disable_log;   // 是否禁用日志
    int m_actormodel;    // 并发模型切换

    int m_pipefd[2];
    int m_epollfd;
    http_conn *users; // 用户连接数组

    // 数据库相关
    connection_pool *m_connPool; // 单个数据库连接池
    string m_url;                // 数据库地址
    int m_port;                  // 数据库端口
    string m_user;               // 登陆数据库用户名
    string m_passWord;           // 登陆数据库密码
    string m_databaseName;       // 使用数据库名
    int m_sql_num;

    // 线程池相关
    threadpool<http_conn> *m_pool; // 单个线程池
    int m_thread_num;              // 线程池中的线程数

    // epoll_event相关
    epoll_event events[MAX_EVENT_NUMBER];

    int m_listenfd;       // 监听socket文件描述符
    int m_OPT_LINGER;     // 优雅关闭连接选项
    int m_trigMode;       // 组合触发模式
    int m_LISTENTrigmode; // listenfd触发模式
    int m_CONNTrigmode;   // connfd触发模式

    // 定时器相关
    client_data *users_timer; // 用户定时器数组
    Utils utils;
};
#endif
