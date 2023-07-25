#ifndef CONFIG_H
#define CONFIG_H

#include "../webserver/webserver.h"

using namespace std;

class Config
{
public:
    Config();
    ~Config(){};

    void parse_arg(int argc, char *argv[]);

    // 端口号
    int PORT;

    // 日志写入方式
    int LOGWriteMode;

    // 触发组合模式
    int trigMode;

    // listenfd触发模式
    int LISTENTrigMode;

    // connfd触发模式
    int CONNTrigmode;

    // 优雅关闭链接
    int OPT_LINGER;

    // 数据库连接池数量
    int sql_num;

    // 线程池内的线程数量
    int thread_num;

    // 是否禁用日志
    int disable_log;

    // 并发模型选择
    int actor_model;
};

#endif