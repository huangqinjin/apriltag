cmake_minimum_required(VERSION 3.1)
project(apriltag)

file(GLOB SOURCE_FILES *.c common/*.c)
file(GLOB HEADER_FILES *.h)

if(NOT DISABLE_CXX)
    set(CMAKE_CXX_STANDARD 11)
    list(APPEND HEADER_FILES apriltag.hpp)
    list(APPEND SOURCE_FILES apriltag.cpp)
    find_package(OpenCV 3 COMPONENTS imgproc)
    if(OPENCV_IMGPROC_FOUND)
        list(APPEND SOURCE_FILES opencv.cpp)
    endif()
endif()

add_library(apriltag ${SOURCE_FILES})
target_include_directories(apriltag PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_options(apriltag PRIVATE -O3 -Wall
                       -Wno-unused-parameter -Wno-unused-function
                       -fno-strict-overflow)
set_target_properties(apriltag PROPERTIES PUBLIC_HEADER "${HEADER_FILES}"
                      INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/.."
                      C_STANDARD 99 C_STANDARD_REQUIRED ON)

if(OPENCV_IMGPROC_FOUND)
    set_target_properties(apriltag PROPERTIES CXX_STANDARD 11
                          CXX_STANDARD_REQUIRED ON CXX_EXTENTIONS OFF)
    target_link_libraries(apriltag PRIVATE opencv_imgproc)
endif()

install(TARGETS apriltag DESTINATION lib
        PUBLIC_HEADER DESTINATION include/apriltag)

install(DIRECTORY common
        DESTINATION include/apriltag
        FILES_MATCHING PATTERN "*.h")