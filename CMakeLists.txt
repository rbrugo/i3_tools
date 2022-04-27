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
    REQUIRES fmt/8.0.1 tl-optional/1.0.0  # scope-lite/0.1.0
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
#                             Enable debug                             #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
function(enable_debug_log target_name)
    string(TOUPPER "ENABLE_DEBUG_LOG_FOR_${target_name}" ENABLE_DEBUG_LOG)
    option(${ENABLE_DEBUG_LOG} "Enable debug log for ${target_name}" FALSE)

    if (${ENABLE_DEBUG_LOG})
        target_compile_definitions(${target_name} PUBLIC ENABLE_DEBUG)
    endif()
endfunction()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#        Dependency on i3-ipc++        #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
find_package(i3-ipc++ REQUIRED)
if (i3-ipc++_FOUND)
    get_target_property(i3-ipc++-includedir i3-ipc++::i3-ipc++ INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS ">>>>> i3-ipc++ found, includedir is '${i3-ipc++-includedir}'")
else()
    message(STATUS ">>>>> i3-ipc++ not found")
endif()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#                             mv_to_output                             #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
add_executable(mv_to_output)
target_sources(mv_to_output PRIVATE src/mv_to_output.cpp)
target_compile_features(mv_to_output PUBLIC cxx_std_20)
target_link_options(mv_to_output PRIVATE)
target_link_libraries(mv_to_output
    PRIVATE
        project_warnings
        i3-ipc++::i3-ipc++
        CONAN_PKG::fmt CONAN_PKG::tl-optional
)
target_include_directories(mv_to_output PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
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
        i3-ipc++::i3-ipc++
)
target_include_directories(focus_workspace PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
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
        i3-ipc++::i3-ipc++
)
target_include_directories(focus_window
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include"
        "${CMAKE_CURRENT_LIST_DIR}/third_party/rollbear/include"
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
        i3-ipc++::i3-ipc++
)
target_include_directories(mv_container
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include"
        "${CMAKE_CURRENT_LIST_DIR}/third_party/rollbear/include"
)
enable_sanitizers(mv_container)
enable_lto(mv_container)
enable_debug_log(mv_container)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#                            fix_workspaces                            #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
add_executable(fix_workspaces)
target_sources(fix_workspaces PRIVATE src/fix_workspaces.cpp)
target_compile_features(fix_workspaces PUBLIC cxx_std_20)
target_link_options(fix_workspaces PRIVATE)
target_link_libraries(fix_workspaces
    PRIVATE
        project_warnings
        CONAN_PKG::fmt CONAN_PKG::tl-optional
        i3-ipc++::i3-ipc++
)
target_include_directories(fix_workspaces
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include"
        "${CMAKE_CURRENT_LIST_DIR}/third_party/rollbear/include"
)
enable_sanitizers(fix_workspaces)
enable_lto(fix_workspaces)
enable_debug_log(fix_workspaces)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#                                 exec                                 #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
add_executable(exec)
target_sources(exec PRIVATE src/exec.cpp)
target_compile_features(exec PUBLIC cxx_std_20)
target_link_options(exec PRIVATE)
target_link_libraries(exec
    PRIVATE
        project_warnings
        CONAN_PKG::fmt CONAN_PKG::tl-optional
        i3-ipc++::i3-ipc++
)
target_include_directories(exec
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include"
        "${CMAKE_CURRENT_LIST_DIR}/third_party/rollbear/include"
)
enable_sanitizers(exec)
enable_lto(exec)
enable_debug_log(exec)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#                  update binaries in .config/i3/bin                   #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
add_custom_target(update
    COMMENT "Copying programs to '~/.config/i3/bin/' ..."
    COMMAND "${CMAKE_COMMAND}" -E copy_directory ~/.config/i3/bin/ ~/.config/i3/.bin_backup/
    COMMAND "${CMAKE_COMMAND}" -E copy_directory bin/ ~/.config/i3/bin/
    COMMAND strip ~/.config/i3/bin/*
)
add_dependencies(update mv_to_output focus_workspace focus_window mv_container fix_workspaces exec)
