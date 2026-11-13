### Project Slit Scan

cmake_minimum_required(VERSION 3.28)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR
    "In-source builds are not allowed; they overwrite the project Makefile.\n"
    "Use 'make' or: cmake -S . -B build -GNinja\n"
    "Then remove what this attempt left behind: rm -rf CMakeCache.txt CMakeFiles")
endif()

# The compiler is not set here, so that whoever builds can choose:
# pass -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang for
# Clang. Both Clang and GCC (14 or newer) build Slit Scan.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Project name and version
project(slit-scan VERSION 0.1.0 LANGUAGES CXX C)

# The application's ID, which names its desktop file, icon and
# metadata, and which Wayland and Flatpak know it by.
set(SLITSCAN_APP_ID "de.atomlogik.SlitScan")

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

# The UI files in gui/ui/ are made from the Cambalache project, and
# are committed. Off, the committed files are used as they are, and
# Cambalache is not needed: this is what packages want.
option(SLITSCAN_REGENERATE_UI "Regenerate the UI files from the Cambalache project" ON)
option(ENABLE_TESTS "Enable testing" OFF)
option(ENABLE_DOCS "Enable documentation" OFF)

include(CTest)

# Debug unless asked for something else; packages ask for Release.
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

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

# The desktop launcher, the icon and the application's metadata
add_subdirectory(data)

if (ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
