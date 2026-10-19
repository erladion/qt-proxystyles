cmake_minimum_required(VERSION 3.16)

project(ModernStyle VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

option(MODERNSTYLE_BUILD_PLUGIN "Build the QStyle plugin (usable via -style modern / winui / windows10)" ON)
option(MODERNSTYLE_BUILD_DEMO "Build the widget gallery demo" ON)
set(MODERNSTYLE_QT_MAJOR_VERSION "" CACHE STRING "Qt major version to use (5 or 6); empty picks Qt6, then Qt5")

if(MODERNSTYLE_QT_MAJOR_VERSION)
  set(QT_VERSION_MAJOR ${MODERNSTYLE_QT_MAJOR_VERSION})
else()
  find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
endif()
find_package(Qt${QT_VERSION_MAJOR} 5.12 REQUIRED COMPONENTS Widgets)
message(STATUS "ModernStyle: building against Qt ${Qt${QT_VERSION_MAJOR}Widgets_VERSION}")

add_library(modernstyle STATIC
  src/StyleHelpers.h
  src/ModernStyle.h
  src/ModernStyle.cpp
  src/WinUIStyle.h
  src/WinUIStyle.cpp
  src/Windows10Style.h
  src/Windows10Style.cpp
)
add_library(ModernStyle::ModernStyle ALIAS modernstyle)
target_include_directories(modernstyle PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(modernstyle PUBLIC Qt${QT_VERSION_MAJOR}::Widgets)
# Linked into the plugin module as well
set_target_properties(modernstyle PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(MODERNSTYLE_BUILD_PLUGIN)
  add_library(modernstyleplugin MODULE plugin/ModernStylePlugin.cpp)
  target_link_libraries(modernstyleplugin PRIVATE modernstyle)
  # Qt looks for style plugins in a "styles" subdirectory of the plugin path
  set_target_properties(modernstyleplugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/styles
  )
endif()

if(MODERNSTYLE_BUILD_DEMO)
  add_executable(modernstyle-demo demo/main.cpp)
  target_link_libraries(modernstyle-demo PRIVATE modernstyle)
endif()
