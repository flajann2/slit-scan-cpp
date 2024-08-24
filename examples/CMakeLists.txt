### Examples

FILE(GLOB CODE *.cpp)

add_executable (VideoTest VideoTest.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtkmm-4.0 freetype2)

target_link_libraries(VideoTest PRIVATE
  PkgConfig::GTK4
  ${FREETYPE_LIBRARIES}
  ${OpenCV_LIBS}
)

find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

# Find the Freetype package
pkg_check_modules(FREETYPE REQUIRED freetype2)

# Include the GTK4 and Freetype include directories
include_directories(${GTK4_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})

# Link against the GTK4 and Freetype libraries
link_directories(${GTK4_LIBRARY_DIRS} ${FREETYPE_LIBRARY_DIRS})
