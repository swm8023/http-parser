cmake_minimum_required(VERSION 3.0)

project(http_parser CXX)

add_definitions(-DHTTP_PARSER_STRICT=0)

set(SRC_C parser_c/http_parser.h parser_c/http_parser.cc)
set(SRC_CC http_parser.h http_parser.cc http_message.h http_message.cc)

include_directories(${PROJECT_SOURCE_DIR})

add_library(http_parser ${SRC_C} ${SRC_CC})

source_group(parser_c FILES ${SRC_C})
source_group(parser_cc FILES ${SRC_CC})

add_executable(http_parser_test test/http_test.cc)
target_link_libraries(http_parser_test http_parser)

install(FILES parser_c/http_parser.h DESTINATION include/http_parser/parser_c)
install(FILES http_message.h http_parser.h DESTINATION include/http_parser)

install(TARGETS http_parser DESTINATION lib)