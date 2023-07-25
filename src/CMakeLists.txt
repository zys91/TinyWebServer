# 添加自定义库的源文件路径
aux_source_directory(CGImysql SRC_CGImysql)
aux_source_directory(config SRC_config)
aux_source_directory(http SRC_http)
aux_source_directory(log SRC_log)
aux_source_directory(timer SRC_timer)
aux_source_directory(webserver SRC_webserver)

# 包含自定义库的头文件路径
include_directories(CGImysql config http lock log threadpool timer webserver)

# 构建server可执行文件
add_executable(server main.cpp ${SRC_CGImysql} ${SRC_config} ${SRC_http} ${SRC_log} ${SRC_timer} ${SRC_webserver})

# 链接相关库到server中
target_link_libraries(server pthread mysqlclient)
