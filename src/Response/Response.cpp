#include "./Response.hpp"
#include <sys/socket.h>

Response::Response()
{
	// Initialize the response with default values
	responseStream << "HTTP/1.1 200 OK\r\n";
	responseStream << "Server: webserv/42.0 (Linux)\r\n"; // Add server header
	responseStream << "Connection: keep-alive\r\n";		  // Default to keep-alive
	responseStream << "Content-Type: text/html\r\n";	  // Default content type
														  // Add other default headers as needed
}

Response::~Response()
{
}

void Response::setStatusCode(int statusCode)
{
	// Set the HTTP status code
	responseStream.seekp(9);
	responseStream << statusCode;
	responseStream << " ";
	switch (statusCode) {
	case 200:
		responseStream << "OK";
		break;
	// Add other status codes as needed
	default:
		responseStream << "Unknown";
		break;
	}
	responseStream << "\r\n";
}

void Response::setHeader(const std::string &key, const std::string &value)
{
	// Set a custom header
	responseStream << key << ": " << value << "\r\n";
}

void Response::setContentType(const std::string &contentType)
{
	// Set the content type header
	responseStream << "Content-Type: " << contentType << "\r\n";
}

void Response::setContentLength(int contentLength)
{
	// Set the content length header
	responseStream << "Content-Length: " << contentLength << "\r\n";
}

void Response::setBody(const std::string &body)
{
	// Set the response body
	setContentLength(body.length());
	responseStream << "\r\n";
	responseStream << body;
}

std::string Response::str() const
{
	return responseStream.str();
}

int Response::getSize() const
{
	// Get the length of the response
	return responseStream.str().length();
}

void Response::respond(int clientSocket) const
{
	// Send the response to the client
	std::string response = responseStream.str();
	send(clientSocket, response.c_str(), response.length(), 0);
}
