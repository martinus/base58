cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(Base58Tests LANGUAGES CXX)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # generate compile_commands.json


# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(
  NAME doctest
  GITHUB_REPOSITORY onqtam/doctest
  GIT_TAG 2.4.5
)

CPMAddPackage(
  NAME nanobench
  GITHUB_REPOSITORY martinus/nanobench
  GIT_TAG v4.3.0
)


if(TEST_INSTALLED_VERSION)
  find_package(Base58 REQUIRED)
else()
  CPMAddPackage(NAME Base58 SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif()

CPMAddPackage(
  NAME Format.cmake
  GITHUB_REPOSITORY TheLartians/Format.cmake
  VERSION 1.6
  OPTIONS # enable cmake formatting
          "FORMAT_CHECK_CMAKE ON"
)

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(Base58Tests ${sources})
target_link_libraries(Base58Tests doctest::doctest nanobench Base58::Base58)
set_target_properties(Base58Tests PROPERTIES CXX_STANDARD 17)

# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(Base58 PUBLIC -Wall -Wpedantic -Wextra -Werror)
  elseif(MSVC)
    target_compile_options(Base58 PUBLIC /W4 /WX)
    target_compile_definitions(Base58Tests PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
  endif()
endif()

# ---- Add Base58Tests ----

enable_testing()

# Note: doctest and similar testing frameworks can automatically configure CMake tests. For other
# testing frameworks add the tests target instead: add_test(NAME Base58Tests COMMAND Base58Tests)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(Base58Tests)

# ---- code coverage ----

if(ENABLE_TEST_COVERAGE)
  target_compile_options(Base58 PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(Base58 PUBLIC -fprofile-arcs -ftest-coverage)
endif()
