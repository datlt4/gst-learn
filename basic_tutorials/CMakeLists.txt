cmake_minimum_required(VERSION 3.10.2)

project(basic-tutorial-1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}" ${CMAKE_MODULE_PATH})
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Gstreamer
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
set(LIB ${LIB} ${GSTREAMER_LIBRARIES})
# set(LIB ${LIB} gstrtspserver-1.0)
set(INC ${INC} ${GSTREAMER_INCLUDE_DIRS})

message(STATUS "Source directories:  ${SRC}")
message(STATUS "Include directories: ${INC}")
message(STATUS "Library directories: ${LIB}")

# Add sources
add_executable(${PROJECT_NAME} basic-tutorial-1.cpp)
# target_sources(${PROJECT_NAME} PRIVATE ${SRC})
include_directories(${INC})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB})
