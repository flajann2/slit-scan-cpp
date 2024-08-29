### Examples

add_executable (VideoTest VideoTest.cpp)
add_executable (CV2GTKVideoTest CV2GTKVideoTest.cpp)
add_executable (SSTest SSTest.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtkmm-4.0 freetype2)

set(COMMON_LIBS
  PkgConfig::GTK4
  ${FREETYPE_LIBRARIES}
  ${OpenCV_LIBS})

target_link_libraries(VideoTest       PRIVATE ${COMMON_LIBS})
target_link_libraries(CV2GTKVideoTest PRIVATE ${COMMON_LIBS})
target_link_libraries(SSTest          PRIVATE ${COMMON_LIBS})

find_package(Freetype REQUIRED)
pkg_check_modules(FREETYPE REQUIRED freetype2)
include_directories(${GTK4_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS} ${FREETYPE_LIBRARY_DIRS})
