cmake_minimum_required(VERSION 3.20)
project(GateServer VERSION 1.0 LANGUAGES CXX)

# 兼容 CMake 4.x
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
cmake_policy(SET CMP0167 OLD)  # FindBoost 模块已移除，改用 BoostConfig

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 依赖管理 ────────────────────────────────────────────
# 优先使用 /home/zdc/.local 下本地编译的 gRPC / protobuf（含 absl）
if(EXISTS /home/zdc/.local/lib/cmake)
    list(PREPEND CMAKE_PREFIX_PATH /home/zdc/.local)
endif()

# 修正 Boost 配置路径：/usr/local 下遗留的 cmake config 指向了错误路径
if(EXISTS /usr/local/lib/cmake/Boost-1.90.0 AND NOT EXISTS /usr/local/lib/libboost_filesystem.so.1.90.0)
    set(Boost_DIR /usr/lib/cmake/Boost-1.90.0)
endif()
find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(jsoncpp REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# hiredis（RedisMgr 使用）
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)

# GateServer ─────────────────────────────────────────
file(GLOB GATE_SERVER_SOURCES
    CONFIGURE_DEPENDS
    src/*.cpp
    src/*.cc
)
list(REMOVE_ITEM GATE_SERVER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_hiredis.cpp
)

add_executable(GateServer ${GATE_SERVER_SOURCES})

target_include_directories(GateServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
    ${HIREDIS_INCLUDE_DIRS}
)

target_link_libraries(GateServer PRIVATE
    Boost::filesystem
    jsoncpp_lib
    gRPC::grpc++_unsecure   # 自动传递 protobuf::libprotobuf + absl
    Threads::Threads
    ${HIREDIS_LIBRARIES}
)

# test_hiredis ────────────────────────────────────────
pkg_check_modules(JSONCPP QUIET jsoncpp)

add_executable(test_hiredis
    src/test_hiredis.cpp
    src/RedisMgr.cpp
    src/ConfigMgr.cpp
)
target_include_directories(test_hiredis PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${JSONCPP_INCLUDE_DIRS}
    ${HIREDIS_INCLUDE_DIRS}
)
target_link_libraries(test_hiredis PRIVATE
    ${HIREDIS_LIBRARIES}
    jsoncpp_lib
    Boost::filesystem
)

# 复制 config.ini 到构建目录（可选）───────────────
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.ini)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.ini ${CMAKE_BINARY_DIR}/config.ini COPYONLY)
endif()
