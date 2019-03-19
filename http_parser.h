#ifndef HTTP_PARSER_CXX_H_
#define HTTP_PARSER_CXX_H_


#include <stdio.h>
#include <string>
#include <functional>
#include <unordered_map>

#include "parser_c/http_parser.h"


namespace http {

#define DEF_HTTP_CB(funcName)					\
static int funcName##_(http_parser * parser) {	\
	HttpParser *p = (HttpParser*)parser->data;	\
	return p->funcName();							\
}

#define DEF_HTTP_DATA_CB(funcName)					\
static int funcName##_(http_parser * parser,		\
	const char *at, size_t length) {				\
	HttpParser *p = (HttpParser*)parser->data;		\
	return p->funcName(at, length);					\
}

#define GET_DEF_CB_NAME(funcName) funcName##_


struct HttpData {
	int method : 8;	// request only
	int status_code : 16;	// response only
	std::string url;
	std::string body;
	std::unordered_map <std::string, std::string> header;

	void Clear() {
		url.clear();
		body.clear();
		header.clear();
	}

	void Output() const{
		printf("http method: %d code: %d\n", method, status_code);
		printf("url: %s\n", url.c_str());
		for (auto& it : header) {
			printf("header %s : %s\n", it.first.c_str(), it.second.c_str());
		}
		printf("body: %s\n", body.c_str());
	}

};


struct HttpRequest : public HttpData {
	const static http_parser_type RTYPE = HTTP_REQUEST;
};

struct HttpResponse : public HttpData {
	const static http_parser_type RTYPE = HTTP_RESPONSE;
};



template<typename HttpDataT>
class HttpParser {
public:
	typedef std::function<void(bool)> CompleteFunc;
	enum ParseState {
		STATE_BEGIN = 0,
		STATE_HEADER_COMPLETE,
		STATE_COMPLETE
	};

	HttpParser() {
		memset(&sets_, 0, sizeof(http_parser_settings));
		sets_.on_message_begin = GET_DEF_CB_NAME(OnBegin);
		sets_.on_message_complete = GET_DEF_CB_NAME(OnComplete);

		sets_.on_status = nullptr;
		sets_.on_url = GET_DEF_CB_NAME(OnUrl);
		sets_.on_body = GET_DEF_CB_NAME(OnBody);
		sets_.on_header_field = GET_DEF_CB_NAME(OnHeaderField);
		sets_.on_header_value = GET_DEF_CB_NAME(OnHeaderValue);
		sets_.on_headers_complete = GET_DEF_CB_NAME(OnHeadersComplete);
		http_parser_init(&parser_, HttpDataT::RTYPE);
		parser_.data = this;
	}

	void Parse(HttpDataT *data, CompleteFunc complete_func) {
		data_ = data;
		complete_func_ = complete_func;
	}

	void Execute(const char* ptr, int size) {
		size_t parsed = http_parser_execute(&parser_, &sets_, ptr, size);
		if (parser_.http_errno) {
			printf("!!! http parse error !!!\n");
			complete_func_(false);
		}
	}

	void Clear() {
		data_->Clear();
		cur_header_field_.clear();
		cur_header_value_.clear();
	}

	DEF_HTTP_CB(OnBegin);
	int OnBegin() {
		Clear();
		state_ = STATE_BEGIN;
		return 0;
	}

	DEF_HTTP_CB(OnComplete);
	int OnComplete() {
		data_->method = parser_.method;
		data_->status_code = parser_.status_code;
		
		state_ = STATE_COMPLETE;
		// notify
		if (complete_func_) {
			complete_func_(true);
		}
		return 0;
	}

	DEF_HTTP_DATA_CB(OnUrl);
	int OnUrl(const char* at, size_t length) {
		data_->url.append(at, length);
		return 0;
	}

	DEF_HTTP_DATA_CB(OnHeaderField);
	int OnHeaderField(const char *at, size_t length) {
		if (!cur_header_value_.empty()) {
			data_->header[cur_header_field_] = cur_header_value_;
			cur_header_field_.clear();
			cur_header_value_.clear();
		}
		cur_header_field_.append(at, length);
		return 0;
	}

	DEF_HTTP_DATA_CB(OnHeaderValue);
	int OnHeaderValue(const char *at, size_t length) {
		cur_header_value_.append(at, length);
		return 0;
	}


	DEF_HTTP_CB(OnHeadersComplete);
	int OnHeadersComplete() {
		if (!cur_header_value_.empty()) {
			data_->header[cur_header_field_] = cur_header_value_;
		}
		return 0;
	}

	DEF_HTTP_DATA_CB(OnBody);
	int OnBody(const char *at, size_t length) {
		data_->body.append(at, length);
		return 0;
	}


private:
	http_parser_settings sets_;
	http_parser parser_;

	std::string cur_header_field_;
	std::string cur_header_value_;

	HttpDataT *data_;

	CompleteFunc complete_func_;

	ParseState state_;

};

}

#endif // HTTP_PARSER_CXX_H_