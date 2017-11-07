project(lmdb)
cmake_minimum_required(VERSION 3.9)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

add_subdirectory(external_lib/zstd/build/cmake)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(zstd-untar "\
    -fno-strict-aliasing \
    -Weverything \
    -Wno-c++98-compat \
    -Wno-c++98-compat-pedantic \
    -Wno-newline-eof \
    -Wno-missing-prototypes \
    -Wno-padded \
    -Wno-double-promotion \
    -Wno-undef \
    -Wno-undefined-reinterpret-cast \
    -Wno-float-conversion \
    -Wno-global-constructors \
    -Wno-exit-time-destructors \
    -Werror \
  ")
elseif(NOT MSVC)
  set(zstd-untar "\
    -Wall \
    -Wextra \
    -Werror \
  ")
else()
  set(zstd-untar "\
    /std:c++latest \
  ")
endif()

file(GLOB_RECURSE zstd-untar-test-files test/*.cc)
add_executable(zstd-untar-test ${zstd-untar-test-files})
target_link_libraries(zstd-untar-test zstd)
set_property(TARGET zstd-untar-test PROPERTY CXX_STANDARD 17)
set_target_properties(zstd-untar-test PROPERTIES COMPILE_FLAGS "${zstd-untar-compile-flags}")

add_custom_target(format-check
    find
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/test
      -type f
      -not -name "doctest.h"
      -a
      \(
      -name "*.cc"
      -o -name "*.h"
      \)
      -print0
    | xargs -0 clang-format-5.0 -i
  COMMAND
    git status --porcelain | xargs -I {} -0 test -z \"{}\"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Checking source code formatting"
  VERBATIM
)

add_custom_target(lint
  /usr/bin/python
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/run-clang-tidy.py
      -exit-on-error
      -header-filter "^${CMAKE_CURRENT_SOURCE_DIR}/include"
      "^${CMAKE_CURRENT_SOURCE_DIR}/src/"
      "^${CMAKE_CURRENT_SOURCE_DIR}/test/"
  COMMENT "Running clang-tidy"
  VERBATIM
)
add_dependencies(lint zstd-untar-test)