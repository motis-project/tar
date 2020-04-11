project(tar)
cmake_minimum_required(VERSION 3.9)

option(TAR_LINT "Run clang-tidy with the compiler." OFF)
if(TAR_LINT)
  include(cmake/clang-tidy.cmake)
endif()

include(cmake/pkg.cmake)

add_library(tar INTERFACE)
target_include_directories(tar INTERFACE include)
target_link_libraries(tar INTERFACE zstd)
target_compile_definitions(tar INTERFACE NOMINMAX WIN32_LEAN_AND_MEAN)

file(GLOB_RECURSE tar-test-files test/*.cc)
add_executable(tar-test EXCLUDE_FROM_ALL ${tar-test-files})
target_link_libraries(tar-test tar doctest)
target_compile_features(tar-test PRIVATE cxx_std_17)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  target_compile_options(tar-test PRIVATE
    -fno-strict-aliasing
    -Weverything
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-newline-eof
    -Wno-missing-prototypes
    -Wno-padded
    -Wno-double-promotion
    -Wno-undef
    -Wno-undefined-reinterpret-cast
    -Wno-float-conversion
    -Wno-global-constructors
    -Wno-exit-time-destructors
    -Werror
  )
elseif(NOT MSVC)
  target_compile_options(tar-test PRIVATE
    -Wall
    -Wextra
    -Werror
  )
endif()
