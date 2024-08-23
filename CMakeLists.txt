### Project Slit Scan

cmake_minimum_required(VERSION 3.30)

# C++ compiler flags
set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "C++ compiler" FORCE)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project name and version
project(slit-scan  LANGUAGES CXX)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

option(ENABLE_EXAMPLES "Enable examples" ON)
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

if (ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()

if (ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
