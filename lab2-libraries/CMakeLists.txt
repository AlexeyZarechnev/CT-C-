cmake_minimum_required(VERSION 3.5.0)
project(
    lab2 
    VERSION 0.1.0 
    LANGUAGES C
)
set(CMAKE_C_STANDARD 17)

include_directories(include)

add_subdirectory(src)

add_executable(${PROJECT_NAME} main.c)

target_include_directories(${PROJECT_NAME} PUBLIC compiled-ffmpeg/include)
target_link_directories(${PROJECT_NAME} PUBLIC compiled-ffmpeg/lib)
target_link_libraries(${PROJECT_NAME} PUBLIC cross_correlation decoder encoder reader)
