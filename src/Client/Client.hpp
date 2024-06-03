#include "../Server/Server.hpp"
#include <sys/epoll.h>
#include <iostream>
#include <sstream>
#include <map>

struct Request
{
    std::string                         request;
    std::string                         method;
    std::string                         uri;
    std::string                         httpVersion;
    std::map<std::string, std::string>  headers;
    std::string                         body;
    bool                                finishedHead;
    bool                                finished;
};

struct Response
{
	int									size;
	int									statusCode;
	std::map<std::string, std::string>	headers;
	std::string 						body;
    bool                                finishedSending;
};

class Client
{
    private:
        int			fd;
        Server		*server;
        Request 	request;
        Response	response;
        size_t		sentBytes;
        std::stringstream responseStream;

    public:
        Client();
        Client(int fd, Server *server, int epoll_fd);
        Client(const Client &client);
        Client &operator=(const Client &client);
        ~Client();

        void        parseHead();
        void        parseBody();
        std::string getErrorString(int code);
        std::string responseToString();
        void        sendErrorPage(int code);

        //general getters and setters
        int			getFd() const;
        Server		*getServer() const;
        Request		getRequest() const;
        Response	getResponse() const;
        epoll_event	*getEvent() const;
        size_t		getSentBytes() const;

        void    setFd(int fd);
        void    setServer(Server *server);
        void    setRequest(Request &request);
        void    setResponse(Response &response);
        void    setEvent(epoll_event *event);
        void    setSentBytes(size_t sentBytes);

        //getters and setters for request
        void    setRequestString(std::string request);
        void    appendRequestString(std::string str);
        void    setRequestMethod(std::string method);
        void    setRequestUri(std::string uri);
        void    setRequestHttpVersion(std::string httpVersion);
        void    setRequestHeaders(std::map<std::string, std::string> headers);
        void    addRequestHeader(std::string key, std::string value);
        void    setRequestBody(std::string body);
        void    setRequestFinishedHead(bool finishedHead);
        void    setRequestFinished(bool finishedBody);

        //getters and setters for response

        void    setResponseSize(int size);
        void    setResponseStatusCode(int statusCode);
        void    setResponseBody(std::string body);
        void    setResponseHeaders(std::map<std::string, std::string> headers);
        void    addResponseHeader(std::string key, std::string value);
        void    setFinishedSending(bool finishedSending);

};

