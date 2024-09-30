### Project Slit Scan

cmake_minimum_required(VERSION 3.30)

# C++ compiler flags
set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "C++ compiler" FORCE)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER "clang" CACHE STRING "C compiler" FORCE)

# Project name and version
project(slit-scan  LANGUAGES CXX C)

include(FetchContent)
### FetchContent_Declare(
###    hekate
###    GIT_REPOSITORY https://github.com/flajann2/hekate.git
###    GIT_TAG master
### )
### FetchContent_MakeAvailable(hekate)
### include_directories(${hekate_SOURCE_DIR}/include)

find_package(yaml-cpp REQUIRED)
include_directories(${YAML_CPP_INCLUDE_DIRS})

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

option(ENABLE_PLAYGROUND "Enable playground" ON)
option(ENABLE_TESTS "Enable testing" OFF)
option(ENABLE_DOCS "Enable documentation" OFF)

include(CTest)

set(CMAKE_BUILD_TYPE Debug)

# Define GNU standard installation direories
include(GNUInstallDirs)
include_directories(
  ${CMAKE_SOURCE_DIR}/main/include
  ${CMAKE_SOURCE_DIR}/gui/include
  ${CMAKE_SOURCE_DIR}/maths/include
  ${CMAKE_SOURCE_DIR}/gels/include
  )

# Subsystems
add_subdirectory (main)
add_subdirectory (gui)
add_subdirectory (maths)
add_subdirectory (gels)

if (ENABLE_DOCS)
  add_subdirectory(docs)
endif()

if (ENABLE_PLAYGROUND)
  add_subdirectory(playground)
endif()

if (ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
