#include "http_message.h"


using namespace http;

void HttpMessage::SetHeader(std::string const& key, std::string const& value) {
	header_[key] = value;
}

std::string HttpMessage::GetHeader(std::string const& key) {
	auto iter = header_.find(key);
	return iter == header_.end() ? "" : iter->second;
}

void HttpMessage::Clear() {
	url.clear();
	body.clear();
	header_.clear();
	status = (enum http_status)0;
	method = (enum http_method)0;
}
