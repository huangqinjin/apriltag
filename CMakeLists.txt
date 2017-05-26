cmake_minimum_required(VERSION 3.1)

set(CMAKE_C_STANDARD 99)

file(GLOB SOURCE_FILES *.c common/*.c)
file(GLOB HEADER_FILES *.h)
include_directories(.)

add_library(apriltag ${SOURCE_FILES})

target_compile_options(apriltag PUBLIC -O4 -Wall
                       -Wno-unused-parameter -Wno-unused-function
                       -fno-strict-overflow)

set_target_properties(apriltag PROPERTIES PUBLIC_HEADER "${HEADER_FILES}")

install(TARGETS apriltag DESTINATION lib
        PUBLIC_HEADER DESTINATION include/apriltag)

install(DIRECTORY common
        DESTINATION include/apriltag
        FILES_MATCHING PATTERN "*.h")