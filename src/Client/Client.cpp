#include "Client.hpp"
#include "../Utils/utils.hpp"
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#define EXIT_FAILURE 1

Client::Client(int fd, Server *server, int epoll_fd) : fd(fd), server(server), sentBytes(0) {
	this->setRequestFinishedHead(false);

	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.fd = fd;

	std::cout << "Client created with fd: " << fd << std::endl;
	std::cout << "Server fd: " << server->getSocketFd() << std::endl;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		perror("epoll_ctl");
		std::cerr << "epoll_ctl failed" << std::endl;
		exit(EXIT_FAILURE);
	}
}

std::vector<std::string> splitString(const std::string &str, const std::string &delimiter) {
	std::vector<std::string> tokens;
	size_t start = 0;
	size_t end = str.find(delimiter);
	while (end != std::string::npos) {
		tokens.push_back(str.substr(start, end - start));
		start = end + delimiter.length();
		end = str.find(delimiter, start);
	}
	tokens.push_back(str.substr(start, end));
	return tokens;
}

// save head of request in request struct
void Client::parseHead() {
	std::string head = this->getRequest().request;
	std::string headStr = head.substr(0, head.find("\r\n\r\n"));
	std::vector<std::string> headLines = splitString(headStr, "\r\n");
	std::vector<std::string> requestLine = splitString(headLines[0], " ");
	this->setRequestMethod(requestLine[0]);
	this->setRequestUri(requestLine[1]);
	this->setRequestHttpVersion(requestLine[2]);
	for (size_t i = 1; i < headLines.size(); i++) {
		std::vector<std::string> header = splitString(headLines[i], ": ");
		this->addRequestHeader(header[0], header[1]);
	}
	this->setRequestFinishedHead(true);
}

size_t stringToSizeT(std::string str) {
	std::stringstream ss(str);
	size_t size;
	ss >> size;
	return size;
}

bool checkFinishedBody(Request request) {
	if (request.headers.find("Content-Length") != request.headers.end()) {
		if (request.body.size() == stringToSizeT(request.headers["Content-Length"]))
			return true;
		return false;
	}
	return true;
}

void Client::parseBody() {
	Request request = this->getRequest();
	this->setRequestBody(request.request.substr(request.request.find("\r\n\r\n") + 4));
	if (!checkFinishedBody(request))
		return;
	this->setRequestFinished(true);
}

void Client::sendErrorPage(int statusCode) {
	this->setResponseStatusCode(statusCode);

	// TODO - add check for error pages in config and if not found, generate error page
	// if error code in config found then response -> *.html

	std::vector<std::string> error_pages = this->getServer()->getErrorPages();
	if (std::find(error_pages.begin(), error_pages.end(), intToString(statusCode) + ".html") !=
		error_pages.end()) {
		// TODO - craft response with www/error_pages/{statusCode}.html
	} else {

		std::string errorPage = "<html><head><title>Error " + intToString(statusCode) +
								"</title></head>"
								"<body><h1>Error " +
								intToString(statusCode) +
								"</h1>"
								"<p>" +
								getErrorString(statusCode) + "</p></body></html>";

		this->setResponseBody(errorPage);
		this->addResponseHeader("Content-Type", "text/html");
		this->addResponseHeader("Content-Length", intToString(this->response.body.size()));

		send(this->getFd(), this->responseToString().c_str(), this->responseToString().size(), 0);
	}
}

std::string Client::responseToString() {
	std::string response = "HTTP/1.1 " + intToString(this->response.statusCode) + " " +
						   this->getErrorString(this->response.statusCode) + "\r\n";
	for (std::map<std::string, std::string>::iterator it = this->response.headers.begin();
		 it != this->response.headers.end(); it++) {
		response += it->first + ": " + it->second + "\r\n";
	}
	response += "\r\n" + this->response.body;
	return response;
}

// client getters
int Client::getFd() const { return this->fd; }
Server *Client::getServer() const { return this->server; }
Request Client::getRequest() const { return this->request; }
Response Client::getResponse() const { return this->response; }
size_t Client::getSentBytes() const { return this->sentBytes; }

// client setters
void Client::setFd(int fd) { this->fd = fd; }
void Client::setServer(Server *server) { this->server = server; }
void Client::setRequest(Request &request) { this->request = request; }
void Client::setResponse(Response &response) { this->response = response; }
void Client::setSentBytes(size_t sentBytes) { this->sentBytes = sentBytes; }

// request setters
void Client::setRequestString(std::string request) { this->request.request = request; }
void Client::appendRequestString(std::string str) { this->request.request += str; }
void Client::setRequestMethod(std::string method) { this->request.method = method; }
void Client::setRequestUri(std::string uri) { this->request.uri = uri; }
void Client::setRequestHttpVersion(std::string httpVersion) {
	this->request.httpVersion = httpVersion;
}
void Client::setRequestHeaders(std::map<std::string, std::string> headers) {
	this->request.headers = headers;
}
void Client::addRequestHeader(std::string key, std::string value) {
	this->request.headers[key] = value;
}
void Client::setRequestBody(std::string body) { this->request.body = body; }
void Client::setRequestFinishedHead(bool finishedHead) {
	this->request.finishedHead = finishedHead;
}
void Client::setRequestFinished(bool finishedBody) { this->request.finished = finishedBody; }

// response setters
void Client::setResponseStatusCode(int statusCode) { this->response.statusCode = statusCode; }
void Client::setResponseBody(std::string body) { this->response.body = body; }
void Client::setResponseHeaders(std::map<std::string, std::string> headers) {
	this->response.headers = headers;
}
void Client::addResponseHeader(std::string key, std::string value) {
	this->response.headers[key] = value;
}
void Client::setResponseSize(int size) { this->response.size = size; }

// boring stuff
Client::Client() : fd(-1), server(NULL), sentBytes(0) {}
Client::~Client() {
	this->request.headers.clear();
	this->response.headers.clear();
}

Client &Client::operator=(const Client &client) {
	if (this != &client) {
		this->fd = client.fd;
		this->server = client.server;
		this->request = client.request;
		this->response = client.response;
		this->sentBytes = client.sentBytes;
	}
	return *this;
}
Client::Client(const Client &client) { *this = client; }

std::string Client::getErrorString(int statusCode) {
	switch (statusCode) {
	case 100:
		return "Continue";
	case 101:
		return "Switching Protocols";
	case 102:
		return "Processing";
	case 103:
		return "Early Hints";
	case 200:
		return "OK";
	case 201:
		return "Created";
	case 202:
		return "Accepted";
	case 203:
		return "Non-Authoritative Information";
	case 204:
		return "No Content";
	case 205:
		return "Reset Content";
	case 206:
		return "Partial Content";
	case 207:
		return "Multi-Status";
	case 208:
		return "Already Reported";
	case 226:
		return "IM Used";
	case 300:
		return "Multiple Choices";
	case 301:
		return "Moved Permanently";
	case 302:
		return "Found";
	case 303:
		return "See Other";
	case 304:
		return "Not Modified";
	case 305:
		return "Use Proxy";
	case 307:
		return "Temporary Redirect";
	case 308:
		return "Permanent Redirect"; // RFC 7538
	case 400:
		return "Bad Request";
	case 401:
		return "Unauthorized";
	case 402:
		return "Payment Required";
	case 403:
		return "Forbidden";
	case 404:
		return "Not Found";
	case 405:
		return "Method Not Allowed";
	case 406:
		return "Not Acceptable";
	case 407:
		return "Proxy Authentication Required";
	case 408:
		return "Request Timeout";
	case 409:
		return "Conflict";
	case 410:
		return "Gone";
	case 411:
		return "Length Required";
	case 412:
		return "Precondition Failed";
	case 413:
		return "Payload Too Large";
	case 414:
		return "URI Too Long";
	case 415:
		return "Unsupported Media Type";
	case 416:
		return "Range Not Satisfiable";
	case 417:
		return "Expectation Failed";
	case 418:
		return "I'm a teapot";
	case 421:
		return "Misdirected Request";
	case 422:
		return "Unprocessable Entity";
	case 423:
		return "Locked";
	case 424:
		return "Failed Dependency";
	case 425:
		return "Too Early";
	case 426:
		return "Upgrade Required";
	case 428:
		return "Precondition Required";
	case 429:
		return "Too Many Requests";
	case 431:
		return "Request Header Fields Too Large";
	case 451:
		return "Unavailable For Legal Reasons";
	case 499:
		return "Client Closed Request";
	case 500:
		return "Internal Server Error";
	case 501:
		return "Not Implemented";
	case 502:
		return "Bad Gateway";
	case 503:
		return "Service Unavailable";
	case 504:
		return "Gateway Timeout";
	case 505:
		return "HTTP Version Not Supported";
	case 506:
		return "Variant Also Negotiates";
	case 507:
		return "Insufficient Storage";
	case 508:
		return "Loop Detected";
	case 510:
		return "Not Extended";
	case 511:
		return "Network Authentication Required";
	default: {
		std::cerr << "Unassigned status code: " << statusCode << std::endl;
		return "Unassigned Status Code";
	}
	}
}
