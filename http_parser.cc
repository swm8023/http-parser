#include "http_parser.h"

using namespace http;

void HttpParser::Init(HttpMessage* msg, http_parser_type type, CompleteCallback cb) {
	memset(&sets_, 0, sizeof(http_parser_settings));
	sets_.on_message_begin = GET_DEF_CB_NAME(OnBegin);
	sets_.on_message_complete = GET_DEF_CB_NAME(OnComplete);
	sets_.on_status = nullptr;
	sets_.on_url = GET_DEF_CB_NAME(OnUrl);
	sets_.on_body = GET_DEF_CB_NAME(OnBody);
	sets_.on_header_field = GET_DEF_CB_NAME(OnHeaderField);
	sets_.on_header_value = GET_DEF_CB_NAME(OnHeaderValue);
	sets_.on_headers_complete = GET_DEF_CB_NAME(OnHeadersComplete);

	memset(&parser_, 0, sizeof(http_parser));
	http_parser_init(&parser_, type);
	parser_.data = this;

	msg_ = msg;
	complete_cb_ = cb;
}

void HttpParser::Execute(const char* ptr, int size) {
	size_t parsed = http_parser_execute(&parser_, &sets_, ptr, size);
	if (parser_.http_errno) {
		complete_cb_(false);
	}
}

void HttpParser::Clear() {
	msg_->Clear();
	cur_header_field_.clear();
	cur_header_value_.clear();
}

int HttpParser::OnBegin() {
	Clear();
	return 0;
}

int HttpParser::OnComplete() {
	msg_->method = (enum http_method)parser_.method;
	msg_->status = (enum http_status)parser_.status_code;
	
	if (complete_cb_) {
		complete_cb_(true);
	}
	return 0;
}

int HttpParser::OnUrl(const char* at, size_t length) {
	msg_->url.append(at, length);
	return 0;
}

int HttpParser::OnHeaderField(const char *at, size_t length) {
	if (!cur_header_value_.empty()) {
		msg_->SetHeader(cur_header_field_, cur_header_value_);
		cur_header_field_.clear();
		cur_header_value_.clear();
	}
	cur_header_field_.append(at, length);
	return 0;
}

int HttpParser::OnHeaderValue(const char *at, size_t length) {
	cur_header_value_.append(at, length);
	return 0;
}

int HttpParser::OnHeadersComplete() {
	if (!cur_header_value_.empty()) {
		msg_->SetHeader(cur_header_field_, cur_header_value_);
	}
	return 0;
}

int HttpParser::OnBody(const char *at, size_t length) {
	msg_->body.append(at, length);
	return 0;
}