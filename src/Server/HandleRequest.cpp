#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include "Cgi/Cgi.hpp"
#include "ServerCluster.hpp"
#include "Utils/utils.hpp"

const int BUFFER_SIZE = 2048 * 8;

bool permitted_origin(Client *client, Server *server) {
	std::string request_uri = client->getRequest().uri;

	std::string origin = client->getRequest().headers["Host"];

	std::string server_name = server->getServerNames()[0];
	std::string server_name2 = server->getServerNames()[1];

	if (origin.find(":") != std::string::npos)
		origin = origin.substr(0, origin.find(":"));

	if (origin != server_name && origin != server_name2) {
		log("Request from Forbidden Origin " + origin);
		return false;
	}
	return true;
}

bool ServerCluster::isPayloadTooLarge(Client *client) {
	if (client->getRequest().headers.count("Content-Length")) {
		if (stringToSizeT(client->getRequest().headers["Content-Length"]) >
			static_cast<unsigned long>(client->getServer()->getClientMaxBodySize())) {
			client->sendErrorPage(413);
			close_client(client->getFd());
			return true;
		}
	}
	return false;
}

bool ServerCluster::check_method_is_allowed(Client *client) {
	std::string request_uri = client->getRequest().uri;
	Server *server = client->getServer();
	std::string method = client->getRequest().method;

	if (method == "GET" && server->getGet(&request_uri).is_allowed)
		return true;
	if (method == "POST" && server->getPost(&request_uri).is_allowed)
		return true;
	if (method == "DELETE" && server->getDelete(&request_uri).is_allowed)
		return true;
	return false;
}

void ServerCluster::handle_request(Client *client) {
	char buffer[BUFFER_SIZE];

	int bytes_read = recv(client->getFd(), buffer, BUFFER_SIZE, 0);

	if (bytes_read == -1 || bytes_read == 0) {
		close_client(client->getFd());
		return;
	}

	client->appendRequestString(std::string(buffer, bytes_read));

	size_t end_of_header = client->getRequest().request.find("\r\n\r\n");

	if (end_of_header == std::string::npos)
		return;
	if (!client->getRequest().finishedHead)
		client->parseHead();
	if (isPayloadTooLarge(client))
		return;
	client->parseBody();
	if (!client->getRequest().finished)
		return;

	std::string request_uri = client->getRequest().uri;

	Server *server = client->getServer();

	if (client->getRequest().body.size() > static_cast<unsigned long>(server->getClientMaxBodySize())) {
		client->sendErrorPage(413);
		return;
	}


	if (!permitted_origin(client, client->getServer())) {
		client->sendErrorPage(403);
		client->setRequestFinishedHead(true);
		close_client(client->getFd());
		return;
	}

	HttpRedirection redirection = server->getRedirection(&request_uri);

	if (client->getRequest().method != "GET" && client->getRequest().method != "POST" &&
		client->getRequest().method != "DELETE") {
		if (find_method(client->getRequest().method) == INVALID) {
			client->sendErrorPage(501); // Not Implemented
		} else
			client->sendErrorPage(405); // Method Not Allowed
		switch_poll(client->getFd(), EPOLLOUT);
		return;
	}

	if (!check_method_is_allowed(client)) {
		client->sendErrorPage(405);
		switch_poll(client->getFd(), EPOLLHUP);
		return;
	}

	if (redirection.code) {
		handle_redirection(client, server, redirection);
		switch_poll(client->getFd(), EPOLLOUT);
		return;
	} else if (client->getRequest().method == "GET" && server->getGet(&request_uri).is_allowed) {
		handle_get_request(client, server);
	} else if (client->getRequest().method == "POST" && server->getPost(&request_uri).is_allowed) {
		this->_client_start_time_map[client->getFd()] = std::time(NULL);
		handle_post_request(client, server);
	} else if (client->getRequest().method == "DELETE" && server->getDelete(&request_uri).is_allowed) {
		handle_delete_request(client, server);
	}

	if (client->getIsPipeOpen() == true)
		switch_poll(client->getFd(), EPOLLHUP);
	else
		switch_poll(client->getFd(), EPOLLOUT);
}

void ServerCluster::handle_redirection(Client *client, Server *server, HttpRedirection redirection) {
	(void)server;
	if (redirection.url == "")
		client->sendErrorPage(redirection.code);
	else {
		std::string url = redirection.url[0] == '/' ? server->getRoot(NULL) + redirection.url : redirection.url;
		client->setResponseStatusCode(redirection.code);
		client->addResponseHeader("Content-Length", "10");
		client->addResponseHeader("Location", redirection.url);
		client->setResponseBody("");
		client->setRequestFinished(true);
	}
}

void update_response(Client *client, std::string body, std::string content_type) {
	client->setResponseBody(body);
	client->addResponseHeader("Content-Type", content_type);
	client->addResponseHeader("Content-Length", intToString(body.size()));
	client->setResponseStatusCode(200);
	client->addResponseHeader("Connection", "keep-alive");
}

void ServerCluster::handle_get_request(Client *client, Server *server) {
	Response response;


	std::string request_uri = client->getRequest().uri;
	std::string full_path = server->get_full_path(client->getRequest().uri);
	std::string body;
	std::string content_type;


	if (full_path.find(server->getCgiPath()) != std::string::npos && full_path.find(".py") != std::string::npos) {
		Cgi cgi;

		if (full_path.find("?") != std::string::npos)
			full_path = full_path.substr(0, full_path.find("?"));

		if (!isFile(full_path))
			return client->sendErrorPage(404);

		cgi.handle_cgi_request(client, full_path, _pipeFd_clientFd_map, _epoll_fd);
		update_response(client, _cgi_response_map[client->getFd()], "text/html");
		return;
	}

	std::string index_file_name = server->getIndexFile(&request_uri);
	if (isFolder(full_path) && directory_contains_file(full_path, index_file_name)) {
		if (full_path[full_path.size() - 1] != '/')
			full_path += '/';
		full_path += index_file_name;
	}


	if (isFolder(full_path) == true && server->getAutoindex(&request_uri) == true) {
		body = generateDirectoryListing(full_path);
		content_type = "text/html";
	} else if (isFile(full_path) == true) {
		body = readFileToString(full_path);
		content_type = getContentType(full_path);
	} else {
		client->sendErrorPage(404);
		return;
	}
	update_response(client, body, content_type);
}

void ServerCluster::handle_post_request(Client *client, Server *server) {
	std::string full_path = server->get_full_path(client->getRequest().uri);

	if (full_path.find("?") != std::string::npos)
		full_path = full_path.substr(0, full_path.find("?"));


	if (client->getRequest().uri.find("/upload") != std::string::npos) {
		handle_file_upload(client);
		return;
	}

	if (!isFile(full_path))
		return client->sendErrorPage(404);

	if (full_path.find(client->getServer()->getCgiPath()) != std::string::npos &&
		full_path.find(".py") != std::string::npos) {
		Cgi cgi;
		cgi.handle_cgi_request(client, full_path, _pipeFd_clientFd_map, _epoll_fd);

		client->setResponseStatusCode(200);
		client->addResponseHeader("Content-Type", "text/html");
		client->addResponseHeader("Content-Length", intToString(client->getResponse().body.size()));
		return;
	}
	client->sendErrorPage(401);
}

std::string extractFileName(const std::string &body, const std::string &boundary) {
	std::string fileName;
	std::string boundary_start = "--" + boundary + "\r\n";
	std::string boundary_end = "--" + boundary + "--\r\n";
	size_t start = body.find(boundary_start);
	size_t end = body.find(boundary_end);

	if (start == std::string::npos || end == std::string::npos)
		return fileName;

	size_t file_name_start = body.find("filename=\"", start);
	size_t file_name_end = body.find("\"", file_name_start + 10);

	if (file_name_start == std::string::npos || file_name_end == std::string::npos)
		return fileName;

	fileName = body.substr(file_name_start + 10, file_name_end - file_name_start - 10);
	return fileName;
}

std::string extractFileContent(const std::string &body, const std::string &boundary) {
	// remove first boundary
	std::string boundary_start = "--" + boundary + "\r\n";
	size_t start = body.find(boundary_start);
	std::string fileContent = body.substr(start + boundary_start.size());
	// remove headers
	fileContent = fileContent.substr(fileContent.find("\r\n\r\n") + 4);
	size_t end = fileContent.find("\r\n--" + boundary);
	if (end != std::string::npos)
		fileContent = fileContent.substr(0, end);
	return fileContent;
}

// todo - keep track of written bytes amount and come back to me baby
void ServerCluster::handle_file_upload(Client *client) {
	std::string body = client->getRequest().body;
	std::string boundary = extract_boundary(client);

	std::string fileName = extractFileName(body, boundary);
	std::string fileContent = extractFileContent(body, boundary);

	if (client->getRequest().body.size() > static_cast<unsigned long>(client->getServer()->getClientMaxBodySize())) {
		log("Body size is too big");
		client->sendErrorPage(413);
		return;
	}
	if (fileName.empty()) {
		Error("fileName is empty");
		client->sendErrorPage(400);
		return;
	}

	std::string request_uri = client->getRequest().uri;
	std::string upload_path = "." + client->getServer()->getRoot(&request_uri) + "/upload/" + fileName;
	std::ofstream outFile(upload_path.c_str(), std::ios::binary);

	if (!outFile.is_open()) {
		Error("Failed to open file for writing");
		return;
	}

	if (fileContent.size() == 0) {
		Error("File content is empty");
		return;
	}

	outFile.write(fileContent.c_str(), fileContent.size());
	outFile.close();

	// redirect to /uploaded.html
	client->setResponseStatusCode(303);
	client->addResponseHeader("Location", "/uploaded.html");
	client->addResponseHeader("Content-Length", "0");

	switch_poll(client->getFd(), EPOLLOUT);
}

void ServerCluster::handle_delete_request(Client *client, Server *server) {
	Response response;

	std::string request_uri = client->getRequest().uri;
	std::string full_path = "." + server->getRoot(&request_uri) + "/upload" + request_uri;
	int is_allowed = allowed_in_path(full_path, client);

	if (isFolder(full_path)) {
		client->sendErrorPage(403);
	} else if (is_allowed == true && isFile(full_path) == true) {
		if (std::remove(full_path.c_str()) == 0) {
		};
		client->setResponseStatusCode(200);
		client->setResponseBody("File was deleted successfully");
		client->addResponseHeader("Content-Type", "text/html");
		client->addResponseHeader("Content-Length", intToString(client->getSentBytes()));
	} else {
		client->sendErrorPage(404);
	}
}

std::string ServerCluster::extract_boundary(Client *client) {
	std::string content_type = client->getRequest().headers["Content-Type"];
	std::string boundary = content_type.substr(content_type.find("boundary=") + 9);
	return boundary;
}


bool ServerCluster::allowed_in_path(const std::string &file_path, Client *client) {
	std::string request_uri = client->getRequest().uri;
	if (file_path.find(client->getServer()->getRoot(&request_uri)) == std::string::npos)
		return false;
	return true;
}
