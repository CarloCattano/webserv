#pragma once

#include "../Server/Server.hpp"
#include <map>
#include <sstream>
#include <sys/epoll.h>

struct Request {
	std::string request;
	std::string method;
	std::string uri;
	std::string httpVersion;
	std::map<std::string, std::string> headers;
	std::string body;
	bool finishedHead;
	bool finished;
};

struct Response {
	int size;
	int statusCode;
	std::map<std::string, std::string> headers;
	std::string body;
	bool finishedSending;
};

class Client {
  private:
	int start_time;
	int fd;
	Server *server;
	Request request;
	Response response;
	size_t sentBytes;
	std::stringstream responseStream;
	std::map<int, int> pid_start_time_map;
	std::map<int, int> pid_pipefd_map;

  public:
	Client();
	Client(int fd, Server *server, int epoll_fd);
	Client(const Client &client);
	Client &operator=(const Client &client);
	~Client();

	void parseHead();
	void parseBody();
	std::string getErrorString(int code);
	std::string responseToString();
	void sendErrorPage(int code);
	void checkTimeout(int timeout);

	// general getters and setters
	int getStartTime() const;
	int getFd() const;
	Server *getServer() const;
	Request getRequest() const;
	Response getResponse() const;
	epoll_event *getEvent() const;
	size_t getSentBytes() const;
	std::map<int, int> getPidStartTimeMap() const;
	std::map<int, int> getPidPipefdMap() const;

	void setStartTime(int start_time);
	void setFd(int fd);
	void setServer(Server *server);
	void setRequest(Request &request);
	void setResponse(Response &response);
	void setEvent(epoll_event *event);
	void setSentBytes(size_t sentBytes);
	void setPidStartTimeMap(std::map<int, int> pid_start_time_map);
	void addPidStartTimeMap(int pid, int start_time);
	void removePidStartTimeMap(int pid);
	void addPidPipefdMap(int pid, int pipefd);
	void removePidPipefdMap(int pid);

	// getters and setters for request
	void setRequestString(std::string request);
	void appendRequestString(std::string str);
	void setRequestMethod(std::string method);
	void setRequestUri(std::string uri);
	void setRequestHttpVersion(std::string httpVersion);
	void setRequestHeaders(std::map<std::string, std::string> headers);
	void addRequestHeader(std::string key, std::string value);
	void setRequestBody(std::string body);
	void setRequestFinishedHead(bool finishedHead);
	void setRequestFinished(bool finishedBody);

	// getters and setters for response
	void setResponseSize(int size);
	void setResponseStatusCode(int statusCode);
	void setResponseHeaders(std::map<std::string, std::string> headers);
	void addResponseHeader(std::string key, std::string value);
	void setResponseBody(std::string body);
	void setFinishedSending(bool finishedSending);
};
