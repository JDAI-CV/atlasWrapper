cmake_minimum_required(VERSION 2.8)
project(atlasWrapper)

SET(CMAKE_BUILD_TYPE "Release")


set(ENV{DDK_HOME} "/home/huawei/SDK/FaceSDK_v3.7/3rdparty/ddk" )
MESSAGE( STATUS "current DDK path: ${DDK_HOME}")
MESSAGE( STATUS "current build target system: ${CMAKE_SYSTEM_PROCESSOR}")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(BUILD_PYTHON "compile python api" ON)

set(PROJECT_SRC_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_MODULE_PATH ${PROJECT_SRC_ROOT}/CMake)
MESSAGE( STATUS "CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")

find_package(DDK REQUIRED)
include_directories(${DDK_INCLUDE_DIRS}
                    ${DDK_CEREAL_INCLUDE_DIRS}
                    ${DDK_PROTOBUF_INCLUDE_DIRS})

include_directories(${PROJECT_SRC_ROOT}/include
                    ${PROJECT_SRC_ROOT}/Common)

add_subdirectory(DstEngine)
add_subdirectory(InferEngine)

if(BUILD_PYTHON)
    message(STATUS "Build python")
    set(Python3_ROOT_DIR /usr/bin/)
    find_package(Python3 REQUIRED)
    include_directories(${PYTHON_INCLUDE_DIRS})
    add_subdirectory(pybind11)
    pybind11_add_module(pyatlas SHARED ${PROJECT_SRC_ROOT}/py/PyNet.cpp)
    target_link_libraries(pyatlas PRIVATE atlasWrapper )
endif()


message(STATUS "Build demo")
add_executable(test_detection ${PROJECT_SRC_ROOT}/test/test_detection.cpp )
target_include_directories(test_detection PUBLIC ${PROJECT_SRC_ROOT}/atlasaWrapper )
target_link_libraries(test_detection ${DDK_OPENCV_LIBRARY} atlasWrapper )

add_executable(test_embedding ${PROJECT_SRC_ROOT}/test/test_embedding.cpp )
target_include_directories(test_embedding PUBLIC ${PROJECT_SRC_ROOT}/atlasaWrapper )
target_link_libraries(test_embedding ${DDK_OPENCV_LIBRARY} atlasWrapper )





