#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <sstream>
#include <string>

class Response {
  private:
	std::ostringstream responseStream;

  public:
	Response();
	~Response();

	void setStatusCode(int statusCode);

	void setHeader(const std::string &key, const std::string &value);

	void setContentType(const std::string &contentType);

	void setContentLength(int contentLength);

	void setBody(const std::string &body);

	void ErrorResponse(int clientSocket, int statusCode);

	std::string str() const;

	// response size
	int getSize() const;

	// response send
	void respond(int clientSocket) const;
};

#endif
