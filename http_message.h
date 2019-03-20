#ifndef HTTP_MESSAGE_CXX_H_
#define HTTP_MESSAGE_CXX_H_

#include "parser_c/http_parser.h"

#include <string>
#include <sstream>
#include <unordered_map>

namespace http {

class HttpMessage {
public:
	void SetHeader(std::string const& key, std::string const& value);
	void SetHeader(std::string const& key, int value);

	std::string GetHeader(std::string const& key) const;

	void Clear();

	friend std::ostream& operator << (std::ostream &os, HttpMessage const& msg) {
		os << "method " << msg.MethodStr() << " ,status " << msg.StatusStr() << std::endl;
		os << "url " << msg.url << std::endl;
		for (auto &it : msg.header_) {
			os << "head " << it.first << " : " << it.second << std::endl;
		}
		os << msg.body << std::endl;
		return os;
	}

	std::string MethodStr() const;
	std::string StatusStr() const;
	std::string VersionStr() const;
	std::string HeaderStr() const;
	
	std::string url;
	std::string body;
	http_method method;
	http_status status;

private:
	std::unordered_map <std::string, std::string> header_;
};


class HttpRequest : public HttpMessage {
public:
};

class HttpResponse : public HttpMessage {
public:
	std::string GetStr() const;

	void Write(std::string const& body, int len=-1);
	void Write(http_status status);
};

}

#endif // HTTP_MESSAGE_H_