#ifndef HTTP_MESSAGE_CXX_H_
#define HTTP_MESSAGE_CXX_H_

#include "parser_c/http_parser.h"

#include <string>
#include <unordered_map>

namespace http {

class HttpMessage {
public:
	void SetHeader(std::string const& key, std::string const& value);
	std::string GetHeader(std::string const& key);

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

	std::string MethodStr() const {
		return http_method_str(method);
	}

	std::string StatusStr() const {
		return http_status_str(status);
	}
	
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



}

#endif // HTTP_MESSAGE_H_