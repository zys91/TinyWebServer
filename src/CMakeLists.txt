# 添加自定义库的源文件路径
aux_source_directory(config SRC_config)
aux_source_directory(http SRC_http)
aux_source_directory(log SRC_log)
aux_source_directory(mysqlpool SRC_mysqlpool)
aux_source_directory(timer SRC_timer)
aux_source_directory(utils SRC_utils)
aux_source_directory(webserver SRC_webserver)

# 包含自定义库的头文件路径
include_directories(config http lock log mysqlpool threadpool timer utils webserver)

# 构建server可执行文件
add_executable(server main.cpp ${SRC_config} ${SRC_http} ${SRC_log} ${SRC_mysqlpool} ${SRC_timer} ${SRC_utils} ${SRC_webserver})

# 链接相关库到server中
target_link_libraries(server pthread mysqlclient)
