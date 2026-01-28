cmake_minimum_required(VERSION 3.19)
project(xccmeta VERSION 1.0.0 LANGUAGES CXX)

#
# Configuration Options
#

option(XCCMETA_BUILD_SHARED "Build xccmeta as a shared library" OFF)
option(XCCMETA_BUILD_TESTS "Build xccmeta tests" ON)

#
# C++ Standard
#

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
# Tests (Optional)
#

if(XCCMETA_BUILD_TESTS)
    include(FetchContent)
    
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(googletest)
    
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