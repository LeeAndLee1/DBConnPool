# 配置最终的可执行文件输出的路径
SET (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置库文件搜索路径
link_directories(${PROJECT_SOURCE_DIR}/lib)
 
# 设置可执行文件
add_executable(main main.cpp)
# 执行可执行文件需要依赖的库
target_link_libraries(main DBConnPool)