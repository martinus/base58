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

function(add_compile_flags_target target)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
            target_compile_options(${target} PRIVATE -fdiagnostics-color)
            target_compile_options(${target} PRIVATE -Wno-zero-as-null-pointer-constant) # doesn't work with spaceship operator
        endif()
        target_compile_options(${target} PRIVATE -Werror)
        target_compile_options(${target} PRIVATE -pedantic)
        target_compile_options(${target} PRIVATE -pedantic-errors)
        target_compile_options(${target} PRIVATE -fvisibility=hidden)
        target_compile_options(${target} PRIVATE -fstrict-aliasing)
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(${target} PRIVATE -Wall)
        target_compile_options(${target} PRIVATE -Wextra)
        target_compile_options(${target} PRIVATE -fdiagnostics-show-option)
        target_compile_options(${target} PRIVATE -Wconversion)
        target_compile_options(${target} PRIVATE -Wold-style-cast)
        target_compile_options(${target} PRIVATE -Wfloat-equal)
        target_compile_options(${target} PRIVATE -Wlogical-op)
        target_compile_options(${target} PRIVATE -Wundef)
        target_compile_options(${target} PRIVATE -Wredundant-decls)
        target_compile_options(${target} PRIVATE -Wshadow)
        target_compile_options(${target} PRIVATE -Wstrict-overflow=2)
        target_compile_options(${target} PRIVATE -Wwrite-strings)
        target_compile_options(${target} PRIVATE -Wpointer-arith)
        target_compile_options(${target} PRIVATE -Wcast-qual)
        target_compile_options(${target} PRIVATE -Wformat=2)
        # target_compile_options(${target} PRIVATE -Wswitch-default)
        target_compile_options(${target} PRIVATE -Wmissing-include-dirs)
        target_compile_options(${target} PRIVATE -Wcast-align)
        target_compile_options(${target} PRIVATE -Wswitch-enum)
        target_compile_options(${target} PRIVATE -Wnon-virtual-dtor)
        target_compile_options(${target} PRIVATE -Wctor-dtor-privacy)
        target_compile_options(${target} PRIVATE -Wsign-conversion)
        target_compile_options(${target} PRIVATE -Wdisabled-optimization)
        target_compile_options(${target} PRIVATE -Weffc++)
        # target_compile_options(${target} PRIVATE -Winline)
        target_compile_options(${target} PRIVATE -Winvalid-pch)
        target_compile_options(${target} PRIVATE -Wmissing-declarations)
        target_compile_options(${target} PRIVATE -Woverloaded-virtual)
        if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0)
            target_compile_options(${target} PRIVATE -Wnoexcept)
        endif()

        # no way to silence it in the expression decomposition macros: _Pragma() in macros doesn't work for the c++ front-end of g++
        # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55578
        # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=69543
        # Also the warning is completely worthless nowadays - http://stackoverflow.com/questions/14016993
        #target_compile_options(${target} PRIVATE -Waggregate-return)

        if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
            target_compile_options(${target} PRIVATE -Wdouble-promotion)
            target_compile_options(${target} PRIVATE -Wtrampolines)
            #target_compile_options(${target} PRIVATE -Wzero-as-null-pointer-constant) # doesn't work with spaceship operator
            #target_compile_options(${target} PRIVATE -Wuseless-cast)
            target_compile_options(${target} PRIVATE -Wvector-operation-performance)
        endif()

        if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0)
            target_compile_options(${target} PRIVATE -Wshift-overflow=2)
            target_compile_options(${target} PRIVATE -Wnull-dereference)
            target_compile_options(${target} PRIVATE -Wduplicated-cond)
        endif()

        if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
            target_compile_options(${target} PRIVATE -Walloc-zero)
            target_compile_options(${target} PRIVATE -Walloca)
            target_compile_options(${target} PRIVATE -Wduplicated-branches)
        endif()

        if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
            target_compile_options(${target} PRIVATE -Wcast-align=strict)
        endif()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${target} PRIVATE -Weverything)
        target_compile_options(${target} PRIVATE -Wno-c++98-compat)
        target_compile_options(${target} PRIVATE -Wno-c++98-compat-pedantic)
        target_compile_options(${target} PRIVATE -Wno-c++98-compat-bind-to-temporary-copy)
        target_compile_options(${target} PRIVATE -Wno-c++98-compat-local-type-template-args)
        target_compile_options(${target} PRIVATE -Qunused-arguments -fcolor-diagnostics) # needed for ccache integration on travis
    endif()

    if(MSVC)
        target_compile_options(${target} PRIVATE /EHsc) # exception handling
        target_compile_options(${target} PRIVATE /std:c++latest) # for post c++14 updates in MSVC
        target_compile_options(${target} PRIVATE /permissive-)   # force standard conformance - this is the better flag than /Za
        target_compile_options(${target} PRIVATE /WX)
        target_compile_options(${target} PRIVATE /Wall) # turns on warnings from levels 1 through 4 which are off by default - https://msdn.microsoft.com/en-us/library/23k5d385.aspx
        
        add_definitions(/MP) # parallel builds

        target_compile_options(${target} PRIVATE 
            /wd4514 # unreferenced inline function has been removed
            /wd4571 # SEH related
            /wd4710 # function not inlined
            /wd4711 # function 'x' selected for automatic inline expansion
            
            /wd4616 # invalid compiler warnings - https://msdn.microsoft.com/en-us/library/t7ab6xtd.aspx
            /wd4619 # invalid compiler warnings - https://msdn.microsoft.com/en-us/library/tacee08d.aspx
            
            /wd4820 # padding in structs
            /wd4625 # copy constructor was implicitly defined as deleted
            /wd4626 # assignment operator was implicitly defined as deleted
            /wd5027 # move assignment operator was implicitly defined as deleted
            /wd5026 # move constructor was implicitly defined as deleted
            /wd4623 # default constructor was implicitly defined as deleted
            /wd4774 # 'swprintf_s' : format string expected in argument 3 is not a string literal

            /wd5045 # Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
            /wd5039 #: '_Thrd_start': pointer or reference to potentially throwing function passed to extern C function under -EHc. Undefined behavior
        )
    endif()

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
        target_compile_options(${target} PRIVATE -ggdb -g)

        # enable lots of warning checking
        #target_compile_options(robinhood-test PRIVATE -Werror -Wall -Wextra -Weffc++ -Wconversion -Wunreachable-code -Wuninitialized -Wshadow -Wfloat-equal -Wmissing-braces)

        # make sure OpenMP in the tests work
        #target_compile_options(robinhood-test PRIVATE -fopenmp)
        #target_link_libraries(robinhood-test PRIVATE -fopenmp)

        #target_compile_options(robinhood-test PRIVATE -fsanitize=undefined,float-divide-by-zero,float-cast-overflow)
        #target_link_libraries(robinhood-test PRIVATE -fsanitize=undefined,float-divide-by-zero,float-cast-overflow)

        #target_compile_options(robinhood-test PRIVATE -fsanitize=address)
        #target_link_libraries(robinhood-test PRIVATE -fsanitize=address)

        # warns in doctest.h :(
        #target_compile_options(robinhood-test PRIVATE -fsanitize=memory)
        #target_link_libraries(robinhood-test PRIVATE -fsanitize=memory)
    endif()
endfunction()


# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
  add_compile_flags_target(Base58)
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
