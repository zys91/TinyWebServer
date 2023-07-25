#include "config/config.h"

int main(int argc, char *argv[])
{
    // 需要修改的数据库信息：IP地址、端口、登录名、密码、库名
    string url = "192.168.8.215";
    int port = 3306;
    string user = "root";
    string passwd = "hhuc404";
    string databasename = "webserver";

    // 命令行解析
    Config config;
    config.parse_arg(argc, argv);

    WebServer server;

    // 初始化
    server.init(url, port, user, passwd, databasename, config.PORT, config.LOGWriteMode,
                config.OPT_LINGER, config.trigMode, config.sql_num, config.thread_num,
                config.disable_log, config.actor_model);

    // 日志
    server.log_init();

    // 数据库
    server.sql_pool();

    // 线程池s
    server.thread_pool();

    // 触发模式
    server.trig_mode();

    // 监听
    server.eventListen();

    // 运行
    server.eventLoop();

    return 0;
}