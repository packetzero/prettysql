CMAKE_MINIMUM_REQUIRED(VERSION 2.7)

set (PROJECT_NAME prettysql-tests)
PROJECT(${PROJECT_NAME})

file(GLOB HDRS "../*.h*" )
file(GLOB SRCS "*.cpp" "../src/utils.cpp")

include_directories(.. ${GTEST_DIR}/include )
include_directories( ../dyno/include )


add_executable (${PROJECT_NAME} ${SRCS} ${HDRS})

FIND_LIBRARY(ANTLR_LIB antlr4-runtime HINT /usr/local/lib )

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${SIMPLESQL_LIB} ${GTEST_DIR}/lib/libgtest.a ${ANTLR_LIB})
