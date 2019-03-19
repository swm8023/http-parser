#include "http_parser.h"

#include <iostream>


const char REQUEST[] =
"GET /get_funky_content_length_body_hello HTTP/1.0\r\n"
"CONTENT-Length: 5\r\n"
"\r\n"
"HELLO"
"GET /get_one_header_no_body HTTP/1.0\r\n"
"\r\n"
"POST /post_identity_body_world?q=search#hey HTTP/1.0\r\n"
"Accept: */*\r\n"
"Transfer-Encoding: identity\r\n"
"Content-Length: 4\r\n"
"\r\n"
"World"
;

int main(int argc, char *argv[]) {
	using namespace http;
	// parser
	HttpRequest req;
	HttpParser parser;
	parser.Init(&req, HTTP_REQUEST, [&](bool ret) {
		if (ret) {
			std::cout << req;
		} else {
			printf("error\n");
		}
	});
	parser.Execute(REQUEST, strlen(REQUEST));

	// builder
	HttpResponse resp;
	resp.Write(HTTP_STATUS_OK);
	resp.Write("Hello world");
	resp.SetHeader("Server", "TioServer");
	std::cout << resp.GetStr();
	return 0;
}