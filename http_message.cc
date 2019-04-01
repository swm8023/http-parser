#include "http_message.h"


using namespace http;

void HttpMessage::SetHeader(std::string const& key, std::string const& value) {
	header_[key] = value;
}

void HttpMessage::SetHeader(std::string const& key, int value) {
	std::ostringstream stream;
	stream << value;
	SetHeader(key, stream.str());
}

std::string HttpMessage::GetHeader(std::string const& key) const {
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

std::string HttpMessage::MethodStr() const {
	return http_method_str(method);
}

std::string HttpMessage::StatusStr() const {
	return http_status_str(status);
}

std::string HttpMessage::VersionStr() const {
	return "HTTP/1.1";
}

std::string HttpMessage::HeaderStr() const {
	std::ostringstream stream;
	for (auto &it : header_) {
		stream << it.first << ": " << it.second << "\r\n";
	}
	return stream.str();
}


/* HttpRequest */
std::string HttpRequest::GetStr() const {
	std::ostringstream stream;
	stream << MethodStr() << " " << url << " " << VersionStr() << "\r\n";
	stream << HeaderStr() << "\r\n";
	stream << body;
	return stream.str();
}

void HttpRequest::Write(http_method method_p, std::string url_p) {
	method = method_p;
	url = url_p;
}

void HttpRequest::Write(std::string const& body_p, int len) {
	body = body_p;
	if (len == -1) len = body_p.length();
	SetHeader("Content-Length", len);
}


/* HttpResponse */
std::string HttpResponse::GetStr() const {
	std::ostringstream stream;
	stream << VersionStr() << " " << status << " " << StatusStr() << "\r\n";
	stream << HeaderStr() << "\r\n";
	stream << body;
	return stream.str();
}

void HttpResponse::Write(std::string const& body_p, int len) {
	body = body_p;
	if (len == -1) len = body_p.length();
	SetHeader("Content-Length", len);
}

void HttpResponse::Write(http_status status_p) {
	status = status_p;
}