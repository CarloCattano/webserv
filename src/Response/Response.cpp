
#include "./Response.hpp"
#include "../Utils/utils.hpp"
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

Response::Response() {
	// Initialize the response with default values
	responseStream << "HTTP/1.1 200 OK\r\n";
	responseStream << "Server: webserv/42.0 (Linux)\r\n"; // Add server header
	responseStream << "Connection: keep-alive\r\n";		  // Default to keep-alive
	responseStream << "Content-Type: text/html\r\n";	  // Default content type
}

Response::~Response() {}

std::string Response::getStatusMessage(int statusCode) {
	switch (statusCode) {
	case 200:
		return "OK";
	case 400:
		return "Bad Request";
	case 404:
		return "Not Found";
	case 405:
		return "Method Not Allowed";
	case 413:
		return "Payload Too Large";
	case 500:
		return "Internal Server Error";
	case 502:
		return "Bad Gateway";
	case 504:
		return "Gateway Timeout";
	// Add other status codes as needed
	default:
		return "Unknown";
	}
}

void Response::setStatusCode(int statusCode) {
	// Set the HTTP status code
	responseStream.seekp(9);
	responseStream << statusCode;
	responseStream << " " << getStatusMessage(statusCode) << "\r\n";
}

void Response::setHeader(const std::string &key, const std::string &value) {
	// Set a custom header
	responseStream << key << ": " << value << "\r\n";
}

void Response::setContentType(const std::string &contentType) {
	// Set the content type header
	responseStream << "Content-Type: " << contentType << "\r\n";
}

void Response::setContentLength(int contentLength) {
	// Set the content length header
	responseStream << "Content-Length: " << contentLength << "\r\n";
}

void Response::setBody(const std::string &body) {
	// Set the response body
	setContentLength(body.length());
	responseStream << "\r\n";
	responseStream << body;
}

std::string Response::str() const { return responseStream.str(); }

int Response::getSize() const { return responseStream.str().length(); }

void Response::ErrorResponse(int clientSocket, int statusCode) {
	std::string errorMessage = getStatusMessage(statusCode);

	// Set the status code
	setStatusCode(statusCode);

	// Create an HTML error page with the corresponding error message
	std::string errorPage = "<html><head><title>Error " + intToString(statusCode) +
							"</title></head>"
							"<body><h1>Error " +
							intToString(statusCode) +
							"</h1>"
							"<p>" +
							errorMessage + "</p></body></html>";

	// Set the headers and body
	setHeader("Content-Type", "text/html");
	setBody(errorPage);

	// Send the response
	respond(clientSocket, -1); // assuming -1 for epoll fd as we are not modifying epoll events here
}

void Response::respond(int clientSocket, int _epoll_fd) const {
	std::string response = responseStream.str();
	ssize_t retsize = send(clientSocket, response.c_str(), response.length(), 0);
	if (retsize == -1) {
		std::cout << "Error sending response" << std::endl;
	}

	if (static_cast<size_t>(retsize) == response.size()) {
		struct epoll_event ev;
		ev.events = EPOLLIN; /* 	Cgi cgi; */
							 /* 	std::string cgi_response = cgi.run(cgi_script_path); */

		/* 	if (!cgi_response.empty()) { */
		/* 		response.setStatusCode(200); */
		/* 		response.setHeader("Connection", "keep-alive"); */
		/* 		response.setHeader("Content-Type", "text/html"); */
		/* 		response.setHeader("Content-Length", intToString(cgi_response.length())); */
		/* 		response.setBody(cgi_response); */
		/* 		response.respond(client_fd, _epoll_fd); */
		/* 		close(client_fd); */

		ev.data.fd = clientSocket;

		if (clientSocket == -1) {
			return;
		}

		if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, clientSocket, &ev) == -1) {
		}
	}
}
