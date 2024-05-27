#include "../Server/Server.hpp"
#include <sys/epoll.h>
#include <iostream>
#include <map>

struct Request {
    std::string                         method;
    std::string                         uri;
    std::string                         httpVersion;
    std::map<std::string, std::string>  headers;
    std::string                         body;
    bool                                finishedReading;
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

    public:
        Client();
        Client(int fd, Server *server, int epoll_fd);
        Client(const Client &client);
        Client &operator=(const Client &client);
        ~Client();

        void        parseRequest(std::string request);
        std::string getErrorString(int code);
        std::string responseToString();

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
        void    setRequestMethod(std::string method);
        void    setRequestUri(std::string uri);
        void    setRequestHttpVersion(std::string httpVersion);
        void    setRequestHeaders(std::map<std::string, std::string> headers);
        void    addRequestHeader(std::string key, std::string value);
        void    setRequestBody(std::string body);

        std::map<std::string, std::string>  getRequestHeaders() const;
        std::string                         getRequestMethod() const;
        std::string                         getRequestUri() const;
        std::string                         getRequestHttpVersion() const;
        std::string                         getRequestBody() const;

        //getters and setters for response
        int                                 getResponseSize();
        int                                 getResponseStatusCode();
        std::string                         getResponseBody();
        std::map<std::string, std::string>  getResponseHeaders();

        void    setResponseSize(int size);
        void    setResponseStatusCode(int statusCode);
        void    setResponseBody(std::string body);
        void    setResponseHeaders(std::map<std::string, std::string> headers);
        void    addResponseHeader(std::string key, std::string value);

};

