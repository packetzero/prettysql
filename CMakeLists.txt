CMAKE_MINIMUM_REQUIRED(VERSION 2.7)

set (PROJECT_NAME prettysql-project)
PROJECT(${PROJECT_NAME})

set(CMAKE_SUPPRESS_REGENERATION true)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF) # want c++11 not gnu++11

set(CMAKE_CXX_RELEASE_FLAGS "-DNDEBUG=1")

FIND_LIBRARY(SIMPLESQL_LIB simplesql HINT "${CMAKE_SOURCE_DIR}/deps/simplesql/build/src")

FIND_LIBRARY(RAPIDJSON_LIB rapidjson )

FIND_LIBRARY(ANTLR_LIB antlr4-runtime )


if ("${RAPIDJSON_LIB}" STREQUAL "RAPIDJSON_LIB-NOTFOUND")
  include_directories(${CMAKE_SOURCE_DIR}/deps/rapidjson/include)
endif()

include_directories(${CMAKE_SOURCE_DIR}/deps/dyno/include)
include_directories(${CMAKE_SOURCE_DIR}/deps/simplesql/include)
include_directories(/usr/local/include)

if ("${SIMPLESQL_LIB}" STREQUAL "SIMPLESQL_LIB-NOTFOUND")
  message("could not find simplesql library")
else()
  message("${SIMPLESQL_LIB}")
endif()

add_subdirectory(src)

if($ENV{MAKE_TESTS})
  set(GTEST_DIR $ENV{GTEST_DIR})
  add_subdirectory(tests)
endif()
