### GUI for Slit Scan

FILE(GLOB GUI *.cpp)

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
