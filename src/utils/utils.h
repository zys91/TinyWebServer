#ifndef UTILS_H
#define UTILS_H

#include "timer/lst_timer.h"

class Utils
{
public:
    Utils() {}
    ~Utils() {}

    void init(int timeslot);

    // 对文件描述符设置非阻塞
    int setnonblocking(int fd);

    // 将内核事件表注册读事件，选择开启EPOLLONESHOT、ET模式
    void addfd(int epollfd, int fd, bool one_shot, int trigMode);

    // 从内核事件表删除描述符
    void removefd(int epollfd, int fd);

    // 将事件重置为EPOLLONESHOT
    void modfd(int epollfd, int fd, int ev, int trigMode);

    // 信号处理函数
    static void sig_handler(int sig);

    // 设置信号函数
    void addsig(int sig, void (*handler)(int), bool restart = true);

    // 定时处理任务，重新定时以不断触发SIGALRM信号
    void timer_handler(int epollfd);

    // 错误显示
    void show_error(int connfd, const char *info);

public:
    int m_TIMESLOT;
    sort_timer_lst m_timer_lst;
    static int *u_pipefd;
};

#endif