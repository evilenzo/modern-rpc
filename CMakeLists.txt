cmake_minimum_required(VERSION 3.12)

###
### PROJECT
### name and version
###
project(modern_rpc VERSION 1.0 LANGUAGES CXX)

###
### CHECK IF MAIN PROJECT
### determine if building as a subproject or if we are the main project
###
set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

###
### OPTIONS
###
option(MODERN_RPC_BUILD_TESTS        "Build unit tests"          ${MAIN_PROJECT})
option(MODERN_RPC_BUILD_SHARED_LIBS  "Build as shared library."  OFF)

###
### CONFIGURATION
###
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD                 20)
set(CMAKE_CXX_STANDARD_REQUIRED        ON)
set(CMAKE_CXX_FLAGS_DEBUG              "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG           "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

# Since it is local variables we don't need to prefix them
# with project name. It makes this code much more reusable.
set(M_TARGET_ROOT               ${CMAKE_CURRENT_SOURCE_DIR})
set(M_TARGET_NAME               ${PROJECT_NAME})
set(M_TARGET_EXPORT_NAME        "${M_TARGET_NAME}Targets")
set(M_TARGET_INCLUDE_DIR        "include/${M_TARGET_NAME}")
set(M_TARGET_CONFIG_TEMPLATE    "cmake/${M_TARGET_NAME}Config.cmake")
set(M_INSTALL_CONFIG_DIR        "${CMAKE_INSTALL_LIBDIR}/cmake/${M_TARGET_NAME}")
set(M_INSTALL_INCLUDE_DIR       "${CMAKE_INSTALL_INCLUDEDIR}")
set(M_INSTALL_LIBRARY_DIR       "${CMAKE_INSTALL_LIBDIR}")
set(M_BINARY_CONFIG_DIR         "${CMAKE_CURRENT_BINARY_DIR}/cmake")
set(M_BINARY_CONFIG_FILE        "${M_BINARY_CONFIG_DIR}/${M_TARGET_NAME}Config.cmake")
set(M_BINARY_TARGET_FILE        "${M_BINARY_CONFIG_DIR}/${M_TARGET_NAME}Targets.cmake")

###
### LIBRARIES
###
find_package(Boost 1.78 COMPONENTS system REQUIRED)

add_subdirectory(third_party/msgpack-c)

###
### SOURCE
###
add_subdirectory(lib)

###
### BUILD
###
add_library(${M_TARGET_NAME} ${M_SRC})
target_include_directories(${M_TARGET_NAME} PRIVATE ${M_TARGET_INCLUDE_DIR})
target_link_libraries(${M_TARGET_NAME} PRIVATE
    Boost::system
    msgpackc-cxx)

add_library(modern::rpc ALIAS ${M_TARGET_NAME})

###
### INSTALL
###
include(CMakePackageConfigHelpers)

configure_file(
    ${M_TARGET_CONFIG_TEMPLATE}
    ${M_BINARY_CONFIG_FILE}
    COPYONLY)

install(
    DIRECTORY ${M_TARGET_INCLUDE_DIR}
    DESTINATION ${M_INSTALL_INCLUDE_DIR})

install(
    TARGETS ${M_TARGET_NAME}
    EXPORT ${M_TARGET_EXPORT_NAME}
    INCLUDES DESTINATION ${M_INSTALL_INCLUDE_DIR})

install(
    FILES ${M_BINARY_CONFIG_FILE} ${M_BINARY_TARGETS_FILE}
    DESTINATION "${M_INSTALL_CONFIG_DIR}")

### Requires fix with msgpackc-cxx
#export(
#    EXPORT ${M_TARGET_EXPORT_NAME}
#    FILE ${M_BINARY_TARGET_FILE})

###
### TEST
###
if (${MODERN_RPC_BUILD_TESTS})
    message(STATUS "Testing enabled")

    set(CMAKE_CXX_FLAGS       "-fprofile-arcs -ftest-coverage")
    set(CMAKE_LINKER_FLAGS    "-fprofile-arcs -ftest-coverage")

    enable_testing()
    add_subdirectory(test)
endif()
