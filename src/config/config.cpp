#include "config.h"

Config::Config()
{
    // 端口号，默认9006
    PORT = 9006;

    // 日志写入方式，默认同步
    LOGWriteMode = 0;

    // 触发组合模式，默认listenfd LT + connfd LT
    trigMode = 0;

    // listenfd触发模式，默认LT
    LISTENTrigMode = 0;

    // connfd触发模式，默认LT
    CONNTrigmode = 0;

    // 优雅关闭链接，默认不使用
    OPT_LINGER = 0;

    // 数据库连接池数量，默认8
    sql_num = 8;

    // 线程池内的线程数量，默认8
    thread_num = 8;

    // 禁用日志，默认不关闭
    disable_log = 0;

    // 并发模型，默认是proactor
    actor_model = 0;

    // 启用双栈，支持IPv6访问，默认不启用，仅IPv4
    enable_ipv6 = 0;
}

void Config::parse_arg(int argc, char *argv[])
{
    int opt;
    const char *str = "p:l:m:o:s:t:c:a:n:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            PORT = atoi(optarg);
            break;
        }
        case 'l':
        {
            LOGWriteMode = atoi(optarg);
            break;
        }
        case 'm':
        {
            trigMode = atoi(optarg);
            break;
        }
        case 'o':
        {
            OPT_LINGER = atoi(optarg);
            break;
        }
        case 's':
        {
            sql_num = atoi(optarg);
            break;
        }
        case 't':
        {
            thread_num = atoi(optarg);
            break;
        }
        case 'c':
        {
            disable_log = atoi(optarg);
            break;
        }
        case 'a':
        {
            actor_model = atoi(optarg);
            break;
        }
        case 'n':
        {
            enable_ipv6 = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }
}