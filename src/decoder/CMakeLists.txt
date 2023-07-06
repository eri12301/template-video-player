set(NAME decoder)
set(NAME-LIB decoder-lib)

add_executable(${NAME} audio_demux_decode.cpp main.cpp)
add_library(${NAME-LIB} audio_demux_decode.cpp video_reader.cpp)

find_package(FFMPEG REQUIRED)
target_include_directories(${NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${NAME} PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${NAME} PRIVATE ${FFMPEG_LIBRARIES})

target_include_directories(${NAME-LIB} PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${NAME-LIB} PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${NAME-LIB} PRIVATE ${FFMPEG_LIBRARIES})