cmake_minimum_required(VERSION 3.21)

project(MulticamInspection VERSION 0.2.0 LANGUAGES CXX)

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
  src/themes/emerald/Entry.cpp
  src/themes/emerald/Style.cpp src/themes/emerald/Style.hpp
  src/themes/emerald/Theme.hpp
  src/themes/emerald/widgets/Pixel.cpp src/themes/emerald/widgets/Pixel.hpp
  src/themes/glass/Entry.cpp
  src/themes/glass/Style.cpp src/themes/glass/Style.hpp
  src/themes/glass/Theme.cpp src/themes/glass/Theme.hpp
  src/themes/glass/widgets/Glass.cpp src/themes/glass/widgets/Glass.hpp
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


# ---- tests
if(BUILD_TESTING)
  find_package(Qt6 6.4 COMPONENTS Test)
  if(Qt6Test_FOUND)
    enable_testing()
    qt_add_executable(tst_camerapipeline tests/tst_CameraPipeline.cpp)
    target_compile_definitions(tst_camerapipeline PRIVATE INSPECTOR_VIDEOS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/videos")
    target_link_libraries(tst_camerapipeline PRIVATE cameras Qt6::Test)
    project_warnings(tst_camerapipeline)
    add_test(NAME camerapipeline COMMAND tst_camerapipeline)

  endif()
endif()
