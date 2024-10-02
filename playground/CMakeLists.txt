### Playground POCs

add_executable (VideoTest       VideoTest.cpp)
add_executable (CV2GTKVideoTest CV2GTKVideoTest.cpp)
add_executable (SSTest          SSTest.cpp)
add_executable (UITest          UITest.cpp)
add_executable (JThread         JThread.cpp)
add_executable (YamlTest        YamlTest.cpp)
add_executable (GenTimer        GenTimer.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtkmm-4.0 freetype2)

set(COMMON_LIBS
  PkgConfig::GTK4
  ${FREETYPE_LIBRARIES}
  ${OpenCV_LIBS})

target_link_libraries(VideoTest       PRIVATE ${COMMON_LIBS})
target_link_libraries(CV2GTKVideoTest PRIVATE ${COMMON_LIBS})
target_link_libraries(SSTest          PRIVATE ${COMMON_LIBS})
target_link_libraries(UITest          PRIVATE ${COMMON_LIBS})
target_link_libraries(JThread         PRIVATE ${COMMON_LIBS})
target_link_libraries(YamlTest        PRIVATE ${COMMON_LIBS} yaml-cpp::yaml-cpp)
target_link_libraries(GenTimer        PRIVATE ${COMMON_LIBS})

add_custom_command(
  OUTPUT uitest.ui
  COMMAND cambalache -E ./uitest.cmb
  DEPENDS uitest.cmb
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT convert the gtk3 glade file to gtk4
  VERBATIM
)

add_custom_target(
  UITestUI ALL
  DEPENDS uitest.cmb
)

add_dependencies(UITestUI UITest)

find_package(Freetype REQUIRED)
pkg_check_modules(FREETYPE REQUIRED freetype2)
include_directories(${GTK4_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS} ${FREETYPE_LIBRARY_DIRS})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/uitest.ui
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
