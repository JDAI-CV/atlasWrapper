project(host)

aux_source_directory(${PROJECT_SRC_ROOT}/DstEngine DstEngine_SRC)
add_library(DstEngine SHARED ${DstEngine_SRC})
target_include_directories(DstEngine PUBLIC ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(DstEngine ${DDK_HOST_LIBRARIES})

aux_source_directory(${PROJECT_SRC_ROOT}/src atlasWrapper_SRC)
add_library(atlasWrapper SHARED ${atlasWrapper_SRC})
target_include_directories(atlasWrapper PUBLIC ${PROJECT_SRC_ROOT}/include)
target_link_libraries(atlasWrapper ${DDK_HOST_LIBRARIES})

message(status, "... ${DDK_HOST_LIBRARIES}")


