cmake_minimum_required(VERSION 3.1)
project(apriltag VERSION 0.9.8)

if (NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
endif()

option(DISABLE_CXX "disable C++ wrapper" OFF)

file(GLOB SOURCE_FILES *.c common/*.c)
file(GLOB HEADER_FILES *.h)

add_library(${PROJECT_NAME} ${SOURCE_FILES})

if(NOT DISABLE_CXX)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11
                          CXX_STANDARD_REQUIRED ON CXX_EXTENTIONS OFF)
    list(APPEND HEADER_FILES apriltag.hpp)
    target_sources(${PROJECT_NAME} PRIVATE apriltag.cpp)
    find_package(OpenCV 3 COMPONENTS imgproc)
    if(OPENCV_IMGPROC_FOUND)
        target_sources(${PROJECT_NAME} PRIVATE opencv.cpp)
        target_link_libraries(${PROJECT_NAME} PRIVATE opencv_imgproc)
    endif()
endif()

target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/..> PRIVATE ${PROJECT_SOURCE_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADER_FILES}"
                      C_STANDARD 99 C_STANDARD_REQUIRED ON)


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
