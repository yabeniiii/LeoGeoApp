cmake_minimum_required(VERSION 3.16)

#
# Project details
#

project(
  "LeoGeo"
  VERSION 1.0.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED On)

include(cmake/Config.cmake)
include(cmake/ClangTidy.cmake)
include(cmake/Sources.cmake)
include(cmake/Doxygen.cmake)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

find_package(Qt6 REQUIRED COMPONENTS WebEngineWidgets Widgets Charts SerialPort Network)
find_package(keychain REQUIRED)

set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)
set(App_Icon ${CMAKE_CURRENT_SOURCE_DIR}/images/AppIcon.icns)
set_source_files_properties(${App_Icon} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

qt_standard_project_setup()
qt_add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${App_Icon} ${sources} ${includes})
target_include_directories(${PROJECT_NAME} PRIVATE ${includes})
target_link_libraries(${PROJECT_NAME} PUBLIC Qt6::WebEngineWidgets Qt6::Widgets Qt6::Charts Qt6::SerialPort Qt6::Network keychain::keychain)

add_compile_options(-fsanitize=address,undefined -g)

set_target_properties(
  ${PROJECT_NAME} PROPERTIES
  CMAKE_AUTOMOC On
  CMAKE_AUTORCC ON
  BUNDLE True
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}"
)
