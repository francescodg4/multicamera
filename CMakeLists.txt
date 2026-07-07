cmake_minimum_required(VERSION 3.21)

project(MulticamInspection
  VERSION 1.0.0
  DESCRIPTION "Multicamera inspection on GStreamer and Qt 6, picture by picture"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(BUILD_TESTING "Build the unit tests" ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED IMPORTED_TARGET gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)
qt_standard_project_setup()

function(project_warnings target)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target} PRIVATE -Wall -Wextra)
  endif()
endfunction()

# ---- interface designs: the themed QStyles of the application
qt_add_library(themes STATIC
  src/Icons.cpp src/Icons.hpp
  src/ThemeRegistry.cpp src/ThemeRegistry.hpp
  src/WidgetStyle.cpp src/WidgetStyle.hpp
  src/themes/flat/Entry.cpp
  src/themes/flat/Style.cpp src/themes/flat/Style.hpp
  src/themes/flat/Theme.cpp src/themes/flat/Theme.hpp
  src/themes/glass/Entry.cpp
  src/themes/glass/Style.cpp src/themes/glass/Style.hpp
  src/themes/glass/Theme.cpp src/themes/glass/Theme.hpp
  src/themes/glass/widgets/Glass.cpp src/themes/glass/widgets/Glass.hpp
  src/themes/metro/Entry.cpp
  src/themes/metro/Style.cpp src/themes/metro/Style.hpp
  src/themes/metro/Theme.cpp src/themes/metro/Theme.hpp
  src/themes/revolut/Entry.cpp
  src/themes/revolut/Style.cpp src/themes/revolut/Style.hpp
  src/themes/revolut/Theme.cpp src/themes/revolut/Theme.hpp
  src/themes/winamp/Entry.cpp
  src/themes/winamp/Style.cpp src/themes/winamp/Style.hpp
  src/themes/winamp/Theme.hpp
  src/themes/winamp/widgets/Metal.cpp src/themes/winamp/widgets/Metal.hpp
)
target_include_directories(themes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(themes PUBLIC Qt6::Widgets)
project_warnings(themes)

# ---- GStreamer backbone: one pipeline per camera
qt_add_library(cameras STATIC
  src/inspector/CameraPipeline.cpp src/inspector/CameraPipeline.hpp
  src/inspector/CameraSource.cpp src/inspector/CameraSource.hpp
)
target_include_directories(cameras PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(cameras PUBLIC Qt6::Gui PRIVATE PkgConfig::GST)
project_warnings(cameras)

# ---- the multicamera inspection system
qt_add_library(inspection STATIC
  src/inspector/CameraControls.cpp src/inspector/CameraControls.hpp
  src/inspector/CameraGrid.cpp src/inspector/CameraGrid.hpp
  src/inspector/CameraTile.cpp src/inspector/CameraTile.hpp
  src/inspector/InspectionWindow.cpp src/inspector/InspectionWindow.hpp
)
target_link_libraries(inspection PUBLIC themes cameras)
project_warnings(inspection)

qt_add_executable(inspector src/inspector/main.cpp)
target_compile_definitions(inspector PRIVATE
  INSPECTOR_VERSION="${PROJECT_VERSION}"
  INSPECTOR_VIDEOS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/videos")
target_link_libraries(inspector PRIVATE inspection)
project_warnings(inspector)

set_target_properties(inspector PROPERTIES WIN32_EXECUTABLE ON MACOSX_BUNDLE ON)

# ---- install: the inspector, its menu entry and its documentation
include(GNUInstallDirs)
install(TARGETS inspector RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} BUNDLE DESTINATION .)
if(UNIX AND NOT APPLE)
  install(FILES packaging/multicam-inspection.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
endif()
install(FILES README.md CHANGELOG.md DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/multicam-inspection)

# ---- package: cmake --build build --target package (DEB and TGZ on Linux, ZIP on Windows)
set(CPACK_PACKAGE_NAME multicam-inspection)
set(CPACK_PACKAGE_VENDOR "Francesco de Gioia")
set(CPACK_PACKAGE_CONTACT "Francesco de Gioia <francescodg4@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Multicamera Inspection")
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/packages)
set(CPACK_STRIP_FILES ON)
if(WIN32)
  set(CPACK_GENERATOR ZIP)
elseif(APPLE)
  set(CPACK_GENERATOR DragNDrop)
else()
  set(CPACK_GENERATOR DEB TGZ)
endif()

# Debian: linked libraries found by dpkg-shlibdeps; the GStreamer plugins are loaded at run time
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DEPENDS
  "gstreamer1.0-plugins-base, gstreamer1.0-plugins-good, gstreamer1.0-libav, gstreamer1.0-x")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "fonts-inter")
set(CPACK_DEBIAN_PACKAGE_SECTION video)
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION
  "An array of cameras, each a GStreamer pipeline, on a board drawn in one of five\ninterface designs. Click a camera to inspect it: play it, move along its time bar\nand step through it picture by picture.")

include(CPack)

# ---- tests: Catch2, run inside a QApplication (tests/test_main.cpp)
if(BUILD_TESTING)
  find_package(Catch2 3 REQUIRED)
  enable_testing()

  function(add_catch_test name)
    qt_add_executable(${name} tests/${name}.cpp tests/test_main.cpp tests/test_support.hpp)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(${name} PRIVATE ${ARGN} Qt6::Widgets Catch2::Catch2)
    project_warnings(${name})
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
  endfunction()

  add_catch_test(test_camera_pipeline cameras)
  target_compile_definitions(test_camera_pipeline PRIVATE INSPECTOR_VIDEOS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/videos")
  add_catch_test(test_inspection_window inspection)
endif()
