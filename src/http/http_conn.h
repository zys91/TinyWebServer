#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <string>
#include <map>
#include <mysql/mysql.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include "lock/locker.h"
#include "mysqlpool/sql_connection_pool.h"
#include "utils/utils.h"

class http_conn
{
public:
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

public:
    http_conn() {}
    ~http_conn() {}

public:
    void init(int sockfd, util_timer *timer, const sockaddr_storage &addr, char *, int, int, std::string user, std::string passwd, std::string sqlname);
    void close_conn();
    bool process();
    bool read_once();
    bool write();
    std::string get_addr();
    void initmysql_result(connection_pool *connPool);

private:
    void init();
    HTTP_CODE process_read();
    bool process_write(HTTP_CODE ret);
    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);
    HTTP_CODE do_request();
    char *get_line() { return m_read_buf + m_start_line; };
    LINE_STATUS parse_line();
    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    static int m_epollfd;
    static int m_user_count;
    static locker m_sql_lock;
    static locker m_count_lock;
    static std::map<std::string, std::string> m_users;
    static sort_timer_lst *m_timer_lst;
    MYSQL *mysql;
    int m_state; // 读为0, 写为1

private:
    int m_sockfd;
    util_timer *m_timer;
    struct sockaddr_storage m_address;
    char m_read_buf[READ_BUFFER_SIZE];
    long m_read_idx;
    long m_checked_idx;
    int m_start_line;
    char m_write_buf[WRITE_BUFFER_SIZE];
    int m_write_idx;
    CHECK_STATE m_check_state;
    METHOD m_method;
    char m_real_file[FILENAME_LEN];
    char *m_url;
    char *m_version;
    char *m_host;
    long m_content_length;
    bool m_linger; // 支持keep-alive 长连接
    char *m_file_address;
    struct stat m_file_stat;
    struct iovec m_iv[2];
    int m_iv_count;
    char *m_string; // 存储请求头数据
    int bytes_to_send;
    int bytes_have_send;
    char *doc_root;

    int m_trigMode;
    int m_disable_log;

    char sql_user[100];
    char sql_passwd[100];
    char sql_name[100];

    Utils utils;
};

#endif
