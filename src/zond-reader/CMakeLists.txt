cmake_minimum_required (VERSION 3.10)
project(zond-reader CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

#Align build type parameter
if (NOT CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "CMAKE_BUILD_TYPE should be specified")
endif()
string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)

message("Build ${CMAKE_BUILD_TYPE}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin")

include_directories("${CMAKE_SOURCE_DIR}/include")

file(GLOB SORCES "src/*.cpp")

if ("${BUILD_TYPE}" MATCHES "debug")
    add_definitions(-DDEBUG)
    set_source_files_properties(${SORCES} PROPERTIES COMPILE_FLAGS "-g")
endif()

#Define version string for application:
if (PACKAGE_VERSION)
    add_definitions(-DPACKAGE_VERSION=${PACKAGE_VERSION})
else()
    add_definitions(-DPACKAGE_VERSION="0-dev")
endif()

add_executable(${CMAKE_PROJECT_NAME} ${SORCES})


find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )
target_link_libraries(${CMAKE_PROJECT_NAME} ${OpenCV_LIBS} pthread)

#target_link_libraries(${CMAKE_PROJECT_NAME} stdc++fs)
