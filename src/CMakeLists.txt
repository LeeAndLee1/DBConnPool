# 设置库文件存放路径
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 定义了一个SRC_LIST变量，包含了该目录下所有的源文件
aux_source_directory(. SRC_LIST)
 
# 生成库，动态库是SHARED，静态库是STATIC
ADD_LIBRARY(DBConnPool SHARED ${SRC_LIST})
 
# 指定可执行文件链接时需要依赖的库文件
target_link_libraries(DBConnPool mysqlclient pthread)