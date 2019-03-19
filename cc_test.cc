#include "http_parser.h"


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
	HttpRequest req;
	HttpParser<HttpRequest> parser;
	parser.Parse(&req, [&](bool ret) {
		if (ret) {
			req.Output();
		} else {
			printf("error\n");
		}
	});
	parser.Execute(REQUEST, strlen(REQUEST));
	return 0;
}