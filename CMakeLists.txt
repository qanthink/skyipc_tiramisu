#设置cmake 最低版本
cmake_minimum_required(VERSION 3.1)

#设置cmake 环境变量
include(env/env.cmake)

#增加需要编译的子文件夹
add_subdirectory(app)
add_subdirectory(thirdparty/libfaac)
add_subdirectory(thirdparty/libfaad)

