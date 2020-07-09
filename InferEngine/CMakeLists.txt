project(device)

include(${PROJECT_SRC_ROOT}/CMake/Ascend.cmake)

include_directories(${PROJECT_SRC_ROOT}/Common/
                    ${DDK_OPENCV_INCLUDE_DIRS})

aux_source_directory(${PROJECT_SRC_ROOT}/InferEngine InferEngine_SRC)
add_library(InferEngine SHARED ${InferEngine_SRC})
target_include_directories(InferEngine PUBLIC ${PROJECT_SRC_ROOT}/InferEngine)
target_link_libraries(InferEngine
                    ${DDK_DEVICE_LIBRARIES}
                    ${DDK_DEVICE_OPENCV_LIBRARYS})




