cmake_minimum_required (VERSION 2.8)

add_compile_options(-std=c++11)

# 定义当前工程名字
project (DBConnPool)

# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 配置头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)

link_directories(/usr/lib64/mysql)

# 添加子CMakeLists.txt路径
add_subdirectory(src)
add_subdirectory(test)