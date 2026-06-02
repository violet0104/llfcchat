cmake_minimum_required(VERSION 3.20)
project(StatusServer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 优先使用 /home/zdc/.local 下本地编译的 gRPC / protobuf（含 absl）
if(EXISTS /home/zdc/.local/lib/cmake)
    list(PREPEND CMAKE_PREFIX_PATH /home/zdc/.local)
endif()

# 修正 Boost 配置路径
if(EXISTS /usr/local/lib/cmake/Boost-1.90.0 AND NOT EXISTS /usr/local/lib/libboost_filesystem.so.1.90.0)
    set(Boost_DIR /usr/lib/cmake/Boost-1.90.0)
endif()
find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(jsoncpp REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(mysql-concpp CONFIG REQUIRED)

# hiredis
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)

# StatusServer 源文件
file(GLOB STATUS_SERVER_SOURCES
    CONFIGURE_DEPENDS
    src/*.cpp
    src/*.cc
)

add_executable(StatusServer ${STATUS_SERVER_SOURCES})

target_include_directories(StatusServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
    ${HIREDIS_INCLUDE_DIRS}
)

target_link_libraries(StatusServer PRIVATE
    Boost::filesystem
    jsoncpp_lib
    gRPC::grpc++_unsecure
    Threads::Threads
    ${HIREDIS_LIBRARIES}
    mysql::concpp-jdbc
)

# 复制 config.ini 到构建目录
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/config.ini)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.ini ${CMAKE_BINARY_DIR}/config.ini COPYONLY)
endif()
