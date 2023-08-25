cmake_minimum_required(VERSION 3.10.2)

project(basic-tutorial-1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}" ${CMAKE_MODULE_PATH})
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Gstreamer
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
set(INC ${INC} ${GSTREAMER_INCLUDE_DIRS})
set(LIB ${LIB} ${GSTREAMER_LIBRARIES})
# set(LIB ${LIB} gstrtspserver-1.0)

message(STATUS "Source directories:  ${SRC}")
message(STATUS "Include directories: ${INC}")
message(STATUS "Library directories: ${LIB}")

# Add sources
set(APP "basic-tutorial-1")
add_executable(${APP} ${APP}.cpp)
target_include_directories(${APP} PRIVATE ${INC})
target_link_libraries(${APP} PRIVATE ${LIB})
unset(APP)

set(APP "basic-tutorial-2")
add_executable(${APP} ${APP}.cpp)
target_include_directories(${APP} PRIVATE ${INC})
target_link_libraries(${APP} PRIVATE ${LIB})
unset(APP)

set(APP "exercise-tutorial-2")
add_executable(${APP} ${APP}.cpp)
target_include_directories(${APP} PRIVATE ${INC})
target_link_libraries(${APP} PRIVATE ${LIB})
unset(APP)

set(APP "basic-tutorial-3")
add_executable(${APP} ${APP}.cpp)
target_include_directories(${APP} PRIVATE ${INC})
target_link_libraries(${APP} PRIVATE ${LIB})
unset(APP)
