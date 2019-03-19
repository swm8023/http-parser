#ifndef HTTP_PARSER_CXX_H_
#define HTTP_PARSER_CXX_H_


#include <stdio.h>
#include <string>
#include <functional>
#include <unordered_map>

#include "parser_c/http_parser.h"
#include "http_message.h"


namespace http {

#define DEF_HTTP_CB(funcName)						\
static int funcName##_(http_parser * parser) {		\
	HttpParser *p = (HttpParser*)parser->data;		\
	return p->funcName();							\
}													\
int funcName();

#define DEF_HTTP_DATA_CB(funcName)					\
static int funcName##_(http_parser * parser,		\
	const char *at, size_t length) {				\
	HttpParser *p = (HttpParser*)parser->data;		\
	return p->funcName(at, length);					\
}													\
int funcName(const char *, size_t);

#define GET_DEF_CB_NAME(funcName) funcName##_

class HttpParser {
public:
	typedef std::function<void(bool)> CompleteCallback;

	void Init(HttpMessage* msg, http_parser_type type, CompleteCallback cb);

	void Execute(const char* ptr, int size);
	void Clear();

	DEF_HTTP_CB(OnBegin);
	DEF_HTTP_CB(OnComplete);
	DEF_HTTP_DATA_CB(OnUrl);
	DEF_HTTP_DATA_CB(OnHeaderField);
	DEF_HTTP_DATA_CB(OnHeaderValue);
	DEF_HTTP_CB(OnHeadersComplete);
	DEF_HTTP_DATA_CB(OnBody);


private:
	http_parser parser_;
	http_parser_settings sets_;
	
	HttpMessage *msg_;
	CompleteCallback complete_cb_;

	std::string cur_header_field_;
	std::string cur_header_value_;
};

}

#endif // HTTP_PARSER_CXX_H_