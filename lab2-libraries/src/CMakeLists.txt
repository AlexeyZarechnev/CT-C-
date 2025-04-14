add_library(vector vector.c)
add_library(decoder decoder.c)
add_library(encoder encoder.c)
add_library(reader reader.c)
add_library(cross_correlation cross_correlation.c)

target_include_directories(decoder PUBLIC ${PROJECT_SOURCE_DIR}/compiled-ffmpeg/include ${PROJECT_SOURCE_DIR})
target_link_directories(decoder PUBLIC ${PROJECT_SOURCE_DIR}/compiled-ffmpeg/lib)
target_link_libraries(decoder PUBLIC avformat avcodec)

target_include_directories(encoder PUBLIC ${PROJECT_SOURCE_DIR}/compiled-ffmpeg/include ${PROJECT_SOURCE_DIR})
target_link_directories(encoder PUBLIC ${PROJECT_SOURCE_DIR}/compiled-ffmpeg/lib)
target_link_libraries(encoder PUBLIC avformat avcodec swresample avutil)

target_include_directories(reader PUBLIC ${PROJECT_SOURCE_DIR}/compiled-ffmpeg/include ${PROJECT_SOURCE_DIR})
target_link_directories(reader PUBLIC ${PROJECT_SOURCE_DIR}/compiled-ffmpeg/lib)
target_link_libraries(reader PUBLIC avformat)

target_include_directories(cross_correlation PUBLIC ${PROJECT_SOURCE_DIR}/compiled-fftw3/include)
target_link_directories(cross_correlation PUBLIC ${PROJECT_SOURCE_DIR}/compiled-fftw3/lib)
target_link_libraries(cross_correlation PUBLIC fftw3)
target_link_libraries(cross_correlation PUBLIC vector)