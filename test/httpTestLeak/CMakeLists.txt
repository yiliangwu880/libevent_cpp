# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

set(prj_name "httpTestLeak")
# 项目信息
project (${prj_name} CXX)

# include directories
INCLUDE_DIRECTORIES(system ../../include/)
INCLUDE_DIRECTORIES(after  ${PROJECT_SOURCE_DIR})


# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
file(GLOB_RECURSE SRCS  ${PROJECT_SOURCE_DIR} "*.cc")
file(GLOB_RECURSE SRCS2  ${PROJECT_SOURCE_DIR} "*.cpp")
file(GLOB_RECURSE SRCS3  ${PROJECT_SOURCE_DIR} "../base/*.cpp")

# 指定生成目标
ADD_EXECUTABLE(${prj_name} ${SRCS} ${SRCS2} ${SRCS3})


#增加系统依赖库
#ADD_library(pthread SHARED IMPORTED)
#ADD_library(crypto SHARED IMPORTED)

# 设置链接库
target_link_libraries(${prj_name} libevent_cpp libevent.a)

#cotire设置gcc的预编译头相关			
set_target_properties(${prj_name} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire(${prj_name})

