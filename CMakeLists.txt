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

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#                Conan                 #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
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

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#               Threads                #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
find_package(Threads REQUIRED)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#           Enable warnings            #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
add_library(project_warnings INTERFACE)
include(cmake/compiler_warnings.cmake)
set_project_warnings(project_warnings)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#        Dependency on i3-ipc++        #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
find_path(i3_ipcpp_include_dir
    NAMES i3_containers.hpp i3_ipc.hpp
    PATHS /usr/local /usr/local/include/i3-ipc++ /usr/local/lib/i3-ipc++
)

find_library(i3_ipcpp_lib
    NAMES libi3-ipc++.a i3-ipc++
    PATHS /usr/local/ /usr/local/include/i3-ipc++/ /usr/local/lib/i3-ipc++
)

if (i3_ipcpp_include_dir STREQUAL "i3_ipcpp_include_dir_NOT_FOUND")
    message(CRITICAL "Can't find i3-ipc++ headers. Please check your i3-ipc++ installation")
endif()
if (i3_ipcpp_lib STREQUAL "i3_ipcpp_lib_NOT_FOUND")
    message(CRITICAL "Can't find i3-ipc++ libraries. Please check your i3-ipc++ installation")
endif()

message(${i3_ipcpp_lib})

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#           enable_debug_log           #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
function(enable_debug_log target)
    option(enable_debug_log_for_${target} "Enable debug log for target ${target}" OFF)
    if ("${enable_debug_log_for_${target}}")
        target_compile_definitions("${target}" PUBLIC -DENABLE_DEBUG_LOG)
    endif()
endfunction()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#                             mv_to_output                             #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
add_executable(mv_to_output)
target_sources(mv_to_output PRIVATE src/mv_to_output.cpp)
target_compile_features(mv_to_output PUBLIC cxx_std_20)
target_link_options(mv_to_output PRIVATE)
target_link_directories(mv_to_output PUBLIC)
target_link_libraries(mv_to_output
    PRIVATE
        project_warnings
        CONAN_PKG::fmt CONAN_PKG::tl-optional
        ${i3_ipcpp_lib}
)
target_include_directories(mv_to_output
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include"
        "${CMAKE_CURRENT_LIST_DIR}/third_party/rollbear/include"
        ${i3_ipcpp_include_dir}
)
enable_sanitizers(mv_to_output)
enable_lto(mv_to_output)
enable_debug_log(mv_to_output)

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
        ${i3_ipcpp_lib}
)
target_include_directories(focus_workspace
    PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
    ${i3_ipcpp_include_dir}
)
enable_sanitizers(focus_workspace)
enable_lto(focus_workspace)
enable_debug_log(focus_workspace)

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
        ${i3_ipcpp_lib}
)
target_include_directories(focus_window
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include"
        "${CMAKE_CURRENT_LIST_DIR}/third_party/rollbear/include"
        ${i3_ipcpp_include_dir}
)
enable_sanitizers(focus_window)
enable_lto(focus_window)
enable_debug_log(focus_window)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#                             mv_container                             #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
add_executable(mv_container)
target_sources(mv_container PRIVATE src/mv_container.cpp)
target_compile_features(mv_container PUBLIC cxx_std_20)
target_link_options(mv_container PRIVATE)
target_link_libraries(mv_container
    PRIVATE
        project_warnings
        CONAN_PKG::fmt CONAN_PKG::tl-optional
        ${i3_ipcpp_lib}
)
target_include_directories(mv_container
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include"
        ${i3_ipcpp_include_dir}
)
enable_sanitizers(mv_container)
enable_lto(mv_container)
enable_debug_log(mv_container)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#                  update binaries in .config/i3/bin                   #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
add_custom_target(update
    COMMAND "${CMAKE_COMMAND}" -E copy_directory ~/.config/i3/bin/ ~/.config/i3/.bin_backup/
    COMMAND "${CMAKE_COMMAND}" -E copy_directory bin/ ~/.config/i3/bin/
    COMMAND strip ~/.config/i3/bin/*
)
add_dependencies(update mv_to_output focus_workspace focus_window mv_container)
