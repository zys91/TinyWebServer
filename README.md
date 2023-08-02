
TinyWebServer
===============
Linux下C++轻量级Web服务器，助力初学者快速实践网络编程，搭建属于自己的服务器.

* 使用 **线程池 + 非阻塞socket + epoll(ET和LT均实现) + 事件处理(Reactor和模拟Proactor均实现)** 的并发模型
* 使用**状态机**解析HTTP请求报文，支持解析**GET和POST**请求
* 访问服务器数据库实现web端用户**注册、登录**功能，可以请求服务器**图片和视频文件**
* 实现**同步/异步日志系统**，记录服务器运行状态
* 经Webbench压力测试可以实现**上万的并发连接**数据交换

快速运行
------------

* 测试前确认已安装MySQL数据库

    ```C++
    // 建立yourdb库
    create database yourdb;

    // 创建user表
    USE yourdb;
    CREATE TABLE user(
        username char(50) NULL,
        passwd char(50) NULL
    )ENGINE=InnoDB;

    // 添加数据
    INSERT INTO user(username, passwd) VALUES('name', 'passwd');
    ```

* 修改main.cpp中的数据库初始化信息

    ```C++
    //数据库IP地址、端口、登录名、密码、库名
    string url = "127.0.0.1";
    int port = 3306;
    string user = "root";
    string passwd = "root";
    string databasename = "webserver";
    ```

* build

    ```C++
    sh ./build.sh
    ```

* 启动server

    ```C++
    cd bin
    ./server
    ```

* 浏览器端

    ```C++
    ip:9006
    ```

个性化运行
------

```C++
./server [-p port] [-l log_writeMode] [-m trigMode] [-o OPT_LINGER] [-s sql_num] [-t thread_num] [-c disable_log] [-a actor_model]
```

温馨提示:以上参数不是非必须，不用全部使用，根据个人情况搭配选用即可.

* -p，自定义端口号
    * 默认9006
* -l，选择日志写入方式，默认同步写入
    * 0，同步写入
    * 1，异步写入
* -m，listenfd和connfd的模式组合，默认使用LT + LT
    * 0，表示使用LT + LT
    * 1，表示使用LT + ET
    * 2，表示使用ET + LT
    * 3，表示使用ET + ET
* -o，优雅关闭连接，默认不使用
    * 0，不使用
    * 1，使用
* -s，数据库连接数量
    * 默认为8
* -t，线程数量
    * 默认为8
* -c，禁用日志，默认启用
    * 0，启用日志
    * 1，禁用日志
* -a，选择事件处理模型，默认Proactor
    * 0，Proactor模型（反应器）
    * 1，Reactor模型（主动器）
* -n，支持双栈，开启IPv6，默认仅IPv4
    * 0，仅IPv4
    * 1，双栈，IPv4+v6

压力测试
------------

采用[Apache JMeter](https://github.com/apache/jmeter)对WebServer的并发性能进行测试与分析。
- 部署教程：[justb4/docker-jmeter](https://github.com/justb4/docker-jmeter#do-it-for-real-detailed-buildruntest)

说明
------------

基于[TinyWebServer](https://github.com/qinguoyi/TinyWebServer)二次开发，仅供学习参考。
