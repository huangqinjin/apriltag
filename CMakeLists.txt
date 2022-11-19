cmake_minimum_required(VERSION 3.12)
project(apriltag VERSION 0.9.8)

option(APRILTAG_DISABLE_CXX "disable C++ wrapper" OFF)
option(APRILTAG_BUILD_EXAMPLES "build examples" OFF)

if(WIN32)
	add_compile_definitions(_USE_MATH_DEFINES _CRT_SECURE_NO_WARNINGS
				NOMINMAX WIN32_LEAN_AND_MEAN VC_EXTRALEAN)
endif()

if(MSVC)
  add_compile_options(/wd4244 /wd4018 /wd4305 /wd4267 /wd4200 /wd4996)
endif()

file(GLOB SOURCE_FILES *.c common/*.c)
file(GLOB HEADER_FILES *.h)

add_library(${PROJECT_NAME} ${SOURCE_FILES})

if(NOT APRILTAG_DISABLE_CXX)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11
                          CXX_STANDARD_REQUIRED ON CXX_EXTENTIONS OFF)
    list(APPEND HEADER_FILES apriltag.hpp)
    target_sources(${PROJECT_NAME} PRIVATE apriltag.cpp)
    find_package(OpenCV ${OPENCV_VERSION} COMPONENTS imgproc)
    if(TARGET opencv_imgproc)
        target_sources(${PROJECT_NAME} PRIVATE opencv.cpp)
        target_link_libraries(${PROJECT_NAME} PRIVATE opencv_imgproc)
    endif()
endif()

find_package(Threads REQUIRED)
include_directories(${PROJECT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/..>)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADER_FILES}"
                      C_STANDARD 99 C_STANDARD_REQUIRED ON
                      WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${MATH_LIBRARY})
endif()


if(APRILTAG_BUILD_EXAMPLES)
    add_executable(apriltag_demo example/apriltag_demo.c)
    target_link_libraries(apriltag_demo PRIVATE ${PROJECT_NAME})
    find_package(OpenCV ${OPENCV_VERSION} COMPONENTS imgproc highgui videoio)
    if(TARGET opencv_imgproc AND TARGET opencv_highgui AND TARGET opencv_videoio)
        add_executable(opencv_demo example/opencv_demo.cc)
        target_link_libraries(opencv_demo PRIVATE ${PROJECT_NAME} opencv_highgui opencv_imgproc opencv_videoio)
    endif()
endif()


include(GNUInstallDirs)
set(ConfigPackageLocation ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})
#set(ConfigPackageLocation ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}        
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

install(DIRECTORY common
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
        FILES_MATCHING PATTERN "*.h")

install(EXPORT ${PROJECT_NAME}-targets DESTINATION ${ConfigPackageLocation})

export(EXPORT ${PROJECT_NAME}-targets)   


include(CMakePackageConfigHelpers)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in"
"@PACKAGE_INIT@
include(\"\${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}-targets.cmake\")
")

configure_package_config_file("${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    INSTALL_DESTINATION ${ConfigPackageLocation})

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    COMPATIBILITY AnyNewerVersion)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    DESTINATION ${ConfigPackageLocation})
