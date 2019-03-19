cmake_minimum_required(VERSION 3.0)

project(http_parser CXX)

set(CMAKE_BUILD_TYPE "RelWithDebInfo")

add_definitions(-DHTTP_PARSER_STRICT=0)

add_library(http_parser http_parser.h http_parser.cc parser_c/http_parser.h parser_c/http_parser.c)
set_target_properties(http_parser PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "CXX")

source_group(parser_c FILES parser_c/http_parser.h parser_c/http_parser.c)
source_group(parser_cc FILES http_parser.h http_parser.cc)


add_executable(http_parser_test cc_test.cc)
target_link_libraries(http_parser_test http_parser)

install(FILES parser_c/http_parser.h http_parser.h DESTINATION include/http_parser)
install(TARGETS http_parser DESTINATION lib/http_parser)