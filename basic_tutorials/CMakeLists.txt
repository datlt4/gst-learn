cmake_minimum_required(VERSION 3.10.2)

project(basic-tutorial LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}" ${CMAKE_MODULE_PATH})

# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Gstreamer
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-audio-1.0)
set(INC ${INC} ${GSTREAMER_INCLUDE_DIRS})
set(LIB ${LIB} ${GSTREAMER_LIBRARIES})

# GTK
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
set(INC ${INC} ${GTK3_INCLUDE_DIRS})
set(LIB ${LIB} ${GTK3_LIBRARIES})

# set(LIB ${LIB} gstrtspserver-1.0)
message(STATUS "Source directories:  ${SRC}")
message(STATUS "Include directories: ${INC}")
message(STATUS "Library directories: ${LIB}")

# Add sources List of target names
set(TARGET_NAMES
    "basic-tutorial-1"
    "basic-tutorial-2"
    "exercise-tutorial-2"
    "basic-tutorial-3"
    "exercise-tutorial-3"
    "basic-tutorial-4"
    "basic-tutorial-5"
    "basic-tutorial-6"
    "basic-tutorial-7"
    "exercise-tutorial-7"
    # "exercise-tutorial-7-oop"
    "basic-tutorial-8")

# Loop through the target names and add sources for each
foreach(APP ${TARGET_NAMES})
  add_executable(${APP} ${APP}.cpp)
  target_include_directories(${APP} PRIVATE ${INC})
  target_link_libraries(${APP} PRIVATE ${LIB})
endforeach()
