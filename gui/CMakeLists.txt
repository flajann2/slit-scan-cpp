### GUI for Slit Scan

FILE(GLOB GUI *.cpp ui/resource.c)

set(UI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ui")
set(UI_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/ui")

set(CMB  "${UI_SOURCE_DIR}/slitscan_main.cmb")
set(CMBO "${UI_SOURCE_DIR}/slitscan_main.ui")
set(CREX "${UI_SOURCE_DIR}/gresource.xml")
set(CREC "${UI_SOURCE_DIR}/resources.c")

message(STATUS "CMB: ${CMB}")
message(STATUS "CMBO: ${CMBO}")
message(STATUS "CREX: ${CREX}")
message(STATUS "CREC: ${CREC}")
message(STATUS "GLOB: ${GUI}")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtkmm-4.0 freetype2)

find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

# Find the Freetype package
pkg_check_modules(FREETYPE REQUIRED freetype2)

# Include the GTK4 and Freetype include directories
include_directories(${GTK4_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})

# Link against the GTK4 and Freetype libraries
link_directories(${GTK4_LIBRARY_DIRS} ${FREETYPE_LIBRARY_DIRS})
# target_link_libraries(${PROJECT_NAME} ${GTK4_LIBRARIES} ${FREETYPE_LIBRARIES})

add_library(GuiLib STATIC ${GUI})
# add_executable (GuiEx ${GUI})
target_link_libraries(GuiLib PRIVATE ${COMMON_LIBS})

add_custom_command(
  OUTPUT ${CMBO} ${CREC}
  COMMAND cambalache -E ${CMB}
  COMMAND glib-compile-resources ${CREX} --target=${CREC} --generate-source
  DEPENDS ${CMB} ${CREX}
  WORKING_DIRECTORY ${UI_SOURCE_DIR}
  COMMENT generate the ui files, then the resource
  VERBATIM
)

add_custom_target(
  SlitscanUI ALL
  DEPENDS ${CMB} ${CREC}
)

add_dependencies(SlitscanUI GuiLib)

# Specify the include directories for the library
target_include_directories(GuiLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Installation rules
install(TARGETS GuiLib
    EXPORT GuiLibTargets
    DESTINATION lib
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
)
