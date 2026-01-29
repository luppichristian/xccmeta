cmake_minimum_required(VERSION 3.19)
project(xccmeta VERSION 1.0.0 LANGUAGES CXX)

#
# Configuration Options
#

option(XCCMETA_BUILD_SHARED "Build xccmeta as a shared library" OFF)
option(XCCMETA_BUILD_TESTS "Build xccmeta tests" ON)
option(XCCMETA_BUILD_EXAMPLES "Build xccmeta examples" ON)
option(XCCMETA_GOOGLE_TEST_FETCH "Fetch GoogleTest if not found" ON)
option(XCCMETA_LLVM_FETCH "Fetch LLVM/Clang if not found" ON)

#
# C++ Standard
#

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

#
# Find or Fetch LLVM/Clang
#

find_package(LLVM CONFIG QUIET)
find_package(Clang CONFIG QUIET)

if(LLVM_FOUND AND Clang_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
    message(STATUS "Using ClangConfig.cmake in: ${Clang_DIR}")
    
    # Map LLVM components
    llvm_map_components_to_libnames(LLVM_LIBS Support Core)
    set(XCCMETA_CLANG_LIBS
        libclang
        clangBasic
        clangLex
        clangFrontend
        ${LLVM_LIBS}
    )
elseif(XCCMETA_LLVM_FETCH)
    message(STATUS "LLVM/Clang not found, fetching from source (this may take a while)...")
    
    set(LLVM_ENABLE_PROJECTS "clang" CACHE STRING "" FORCE)
    set(LLVM_TARGETS_TO_BUILD "X86" CACHE STRING "" FORCE)
    set(LLVM_ENABLE_TERMINFO OFF CACHE BOOL "" FORCE)
    set(LLVM_ENABLE_ZLIB OFF CACHE BOOL "" FORCE)
    set(LLVM_ENABLE_ZSTD OFF CACHE BOOL "" FORCE)
    set(LLVM_INCLUDE_TESTS OFF CACHE BOOL "" FORCE)
    set(LLVM_INCLUDE_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(LLVM_INCLUDE_BENCHMARKS OFF CACHE BOOL "" FORCE)
    set(LLVM_INCLUDE_DOCS OFF CACHE BOOL "" FORCE)
    set(CLANG_INCLUDE_TESTS OFF CACHE BOOL "" FORCE)
    set(CLANG_INCLUDE_DOCS OFF CACHE BOOL "" FORCE)
    
    FetchContent_Declare(
        llvm-project
        GIT_REPOSITORY https://github.com/llvm/llvm-project.git
        GIT_TAG llvmorg-18.1.8
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR llvm
    )
    FetchContent_MakeAvailable(llvm-project)
    
    set(XCCMETA_CLANG_LIBS
        libclang
        clangBasic
        clangLex
        clangFrontend
        LLVMSupport
        LLVMCore
    )
    set(LLVM_INCLUDE_DIRS ${llvm-project_SOURCE_DIR}/llvm/include ${llvm-project_BINARY_DIR}/include)
    set(CLANG_INCLUDE_DIRS ${llvm-project_SOURCE_DIR}/clang/include ${llvm-project_BINARY_DIR}/tools/clang/include)
endif()

#
# Library Sources
#

file(GLOB_RECURSE XCCMETA_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)
file(GLOB_RECURSE XCCMETA_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

#
# Test Sources
#

file(GLOB_RECURSE XCCMETA_TEST_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
)

#
# Build Library (Static or Shared)
#

if(XCCMETA_BUILD_SHARED)
    add_library(xccmeta SHARED ${XCCMETA_SOURCES} ${XCCMETA_HEADERS})
    target_compile_definitions(xccmeta PRIVATE XCCMETA_EXPORTS)
    target_compile_definitions(xccmeta PUBLIC XCCMETA_SHARED)
    set_target_properties(xccmeta PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
else()
    add_library(xccmeta-static STATIC ${XCCMETA_SOURCES} ${XCCMETA_HEADERS})
    add_library(xccmeta::static ALIAS xccmeta-static)
    set_target_properties(xccmeta-static PROPERTIES
        VERSION ${PROJECT_VERSION}
        OUTPUT_NAME xccmeta
    )
endif()

#
# Set Common Properties for the Active Target
#

if(XCCMETA_BUILD_SHARED)
    set(XCCMETA_TARGET xccmeta)
else()
    set(XCCMETA_TARGET xccmeta-static)
endif()

target_include_directories(${XCCMETA_TARGET}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

#
# Link libclang
#

target_include_directories(${XCCMETA_TARGET}
    PRIVATE
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
)

target_link_libraries(${XCCMETA_TARGET}
    PRIVATE
        ${XCCMETA_CLANG_LIBS}
)

#
# Tests (Optional)
#

if(XCCMETA_BUILD_TESTS)
    find_package(GTest CONFIG QUIET)
    
    if(GTest_FOUND)
        message(STATUS "Found GoogleTest")
    elseif(XCCMETA_GOOGLE_TEST_FETCH)
        message(STATUS "GoogleTest not found, fetching from source...")
        
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
        )
        
        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        
        FetchContent_MakeAvailable(googletest)
    endif()
    
    enable_testing()
    
    add_executable(xccmeta-tests
        ${XCCMETA_TEST_SOURCES}
    )
    
    target_link_libraries(xccmeta-tests
        PRIVATE
            ${XCCMETA_TARGET}
            GTest::gtest
            GTest::gtest_main
    )
    
    include(GoogleTest)
    gtest_discover_tests(xccmeta-tests)
endif()

#
# Examples (Optional)
#

if(XCCMETA_BUILD_EXAMPLES)
    file(GLOB XCCMETA_EXAMPLE_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp
    )
    
    foreach(EXAMPLE_SOURCE ${XCCMETA_EXAMPLE_SOURCES})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
        add_executable(xccmeta-example-${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
        target_link_libraries(xccmeta-example-${EXAMPLE_NAME}
            PRIVATE
                ${XCCMETA_TARGET}
        )
        set_target_properties(xccmeta-example-${EXAMPLE_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/examples
        )
    endforeach()
    
    message(STATUS "Building examples: ${XCCMETA_EXAMPLE_SOURCES}")
endif()

#
# Installation
#

include(GNUInstallDirs)

install(TARGETS ${XCCMETA_TARGET}
    EXPORT xccmetaTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${XCCMETA_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/xccmeta
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)