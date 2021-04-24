######################################################################
# @author      : rbrugo
# @file        : CMakeLists
# @created     : Thursday Apr 08, 2021 22:28:01 CEST
######################################################################

cmake_minimum_required(VERSION 3.16.2)

project(i3-utils VERSION 0.1 LANGUAGES CXX)
include(cmake/general.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#                                Conan                                 #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
include(cmake/conan.cmake)
conan_cmake_configure(
    REQUIRES fmt/7.1.2 tl-optional/1.0.0  # scope-lite/0.1.0
    BASIC_SETUP CMAKE_TARGETS
)
conan_cmake_autodetect(settings)
conan_cmake_install(
    PATH_OR_REFERENCE ${CMAKE_BINARY_DIR}
    BUILD missing
    GENERATOR cmake
    SETTINGS ${settings}
)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS) # enables the import of single libraries

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#                               Threads                                #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
find_package(Threads REQUIRED)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#                           Enable warnings                            #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
add_library(project_warnings INTERFACE)
include(cmake/compiler_warnings.cmake)
set_project_warnings(project_warnings)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#                            mv_to_monitor                             #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
add_executable(mv_to_monitor)
target_sources(mv_to_monitor PRIVATE src/mv_to_monitor.cpp)
target_compile_features(mv_to_monitor PUBLIC cxx_std_20)
target_link_options(mv_to_monitor PRIVATE)
target_link_libraries(mv_to_monitor
    PRIVATE
        project_warnings
        CONAN_PKG::fmt CONAN_PKG::tl-optional
        /usr/local/lib/i3-ipc++/libi3-ipc++.a # xcb xcb-randr
)
target_include_directories(mv_to_monitor PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
enable_sanitizers(mv_to_monitor)
enable_lto(mv_to_monitor)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#                           focus_workspace                            #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
add_executable(focus_workspace)
target_sources(focus_workspace PRIVATE src/focus_workspace.cpp)
target_compile_features(focus_workspace PUBLIC cxx_std_20)
target_link_options(focus_workspace PRIVATE)
target_link_libraries(focus_workspace
    PRIVATE
        project_warnings
        CONAN_PKG::fmt CONAN_PKG::tl-optional
        /usr/local/lib/i3-ipc++/libi3-ipc++.a
)
target_include_directories(focus_workspace PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
enable_sanitizers(focus_workspace)
enable_lto(focus_workspace)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#                             focus_window                             #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
add_executable(focus_window)
target_sources(focus_window PRIVATE src/focus_window.cpp)
target_compile_features(focus_window PUBLIC cxx_std_20)
target_link_options(focus_window PRIVATE)
target_link_libraries(focus_window
    PRIVATE
        project_warnings
        CONAN_PKG::fmt CONAN_PKG::tl-optional
        /usr/local/lib/i3-ipc++/libi3-ipc++.a
)
target_include_directories(focus_window
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include"
        "${CMAKE_CURRENT_LIST_DIR}/third_party/rollbear/include"
)
enable_sanitizers(focus_window)
enable_lto(focus_window)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#                  update binaries in .config/i3/bin                   #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
add_custom_target(update
    COMMAND "${CMAKE_COMMAND}" -E copy_directory ~/.config/i3/bin/ ~/.config/i3/.bin_backup/
    COMMAND "${CMAKE_COMMAND}" -E copy_directory bin/ ~/.config/i3/bin/
    COMMAND strip ~/.config/i3/bin/*
)
add_dependencies(update mv_to_monitor focus_workspace)
