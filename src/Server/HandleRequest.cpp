#include "../Cgi/Cgi.hpp"
#include "./ServerCluster.hpp"

void ServerCluster::handle_request(Client &client)
{
	char buffer[4096];

	int bytes_read = recv(client.getFd(), buffer, 4096, 0);

	if (bytes_read == -1) {
		perror("recv");
		close_client(client.getFd());
		return;
	}

	client.appendRequestString(std::string(buffer, bytes_read));

	size_t end_of_header = client.getRequest().request.find("\r\n\r\n");

	if (end_of_header == std::string::npos)
		return;
	if (!client.getRequest().finishedHead)
		client.parseHead();
	client.parseBody();
	if (!client.getRequest().finished)
		return;

	Server *server = client.getServer();
	if (client.getRequest().body.size() > static_cast<unsigned long>(server->getClientMaxBodySize())) {
		log("Body size is too big");
		client.sendErrorPage(413);
		return;
	}

	if (client.getRequest().method == "GET") {
		handle_get_request(client);
	}
	else if (client.getRequest().method == "POST") {
		handle_post_request(client);
	}
	else if (client.getRequest().method == "DELETE")
		handle_delete_request(client);
	else {
		// TODO - check if 405 is in the server error pages
		client.sendErrorPage(405);
	}
	switch_poll(client.getFd(), EPOLLOUT);
}

void update_response(Client &client, std::string body, std::string content_type)
{
	client.setResponseBody(body);
	client.addResponseHeader("Content-Type", content_type);
	client.addResponseHeader("Content-Length", intToString(body.size()));
	client.setResponseStatusCode(200);
	client.addResponseHeader("Connection", "keep-alive");
}

void ServerCluster::handle_get_request(Client &client)
{
	Response response;
	Server *server = client.getServer();
	std::string full_path = "." + server->getRoot() + client.getRequest().uri;
	std::string body;
	std::string content_type;

	if (full_path.find(server->getCgiPath()) != std::string::npos && full_path.find(".py") != std::string::npos) {
		Cgi cgi;

		if (full_path[full_path.size() - 1] == '?')
			full_path = full_path.substr(0, full_path.size() - 1);
		cgi.handle_cgi_request(client, full_path, pipes, _client_fd_to_pipe_map, _epoll_fd);
		update_response(client, _cgi_response_map[client.getFd()], "text/html");
		return;
	}

	if (isFolder(full_path) && directory_contains_index_file(full_path))
		full_path += "index.html";

	if (isFolder(full_path) == true && server->getAutoindex() == true) {
		body = generateDirectoryListing(full_path);
		content_type = "text/html";
	}
	else if (isFile(full_path)) {
		body = readFileToString(full_path);
		content_type = getContentType(full_path);
	}
	else {
		client.sendErrorPage(404);
		return;
	}
	update_response(client, body, content_type);
}

void ServerCluster::handle_post_request(Client &client)
{
	std::string full_path = "." + client.getServer()->getRoot() + client.getRequest().uri;


	if (client.getRequest().uri == "/upload") {
		handle_file_upload(client);
	}
	Cgi cgi;
	Error("cgi from post");
	cgi.handle_cgi_request(client, full_path, pipes, _client_fd_to_pipe_map, _epoll_fd);

	client.setResponseStatusCode(200);
	client.addResponseHeader("Content-Type", "text/html");
	client.addResponseHeader("Content-Length", intToString(client.getSentBytes()));
}

void ServerCluster::handle_file_upload(Client &client)
{
	std::string headers = client.getRequest().request.substr(0, client.getRequest().request.find("\r\n\r\n"));
	std::string body = client.getRequest().body;

	std::string boundary = extract_boundary(headers);
	FileUploader fileUploader;
	MultipartFormData formData = fileUploader.parse_multipart_form_data(boundary, body);

	if (formData.fileName.empty()) {
		client.sendErrorPage(400);
		return;
	}


	if (formData.fileContent.empty()) {
		client.sendErrorPage(400);
		return;
	}

	std::string upload_path = "." + client.getServer()->getRoot() + "/upload/" + formData.fileName;
	std::ofstream outFile(upload_path.c_str(), std::ios::binary);

	outFile.write(&formData.fileContent[0], formData.fileContent.size());
	outFile.close();

	client.setResponseStatusCode(303);
	client.addResponseHeader("Location", "/uploaded.html");
	client.addResponseHeader("Content-Type", "text/html");
	client.addResponseHeader("Content-Length", "0");
	client.addResponseHeader("Connection", "close");
	client.setResponseBody("");

	switch_poll(client.getFd(), EPOLLOUT);
}

void ServerCluster::handle_delete_request(Client &client)
{
	Response response;

	std::string full_path = "." + client.getServer()->getRoot() + "/upload" + client.getRequest().uri;

	int is_allowed = allowed_in_path(full_path, client);

	if (isFolder(full_path)) {
		client.sendErrorPage(403);
	}
	else if (is_allowed == true && isFile(full_path) == true) {
		if (std::remove(full_path.c_str()) == 0) {
		};
		std::cout << "file " << full_path.c_str() << " was deleted from the server" << std::endl;
		client.setResponseStatusCode(200);
		client.setResponseBody("File was deleted successfully");
		client.addResponseHeader("Content-Type", "text/html");
		client.addResponseHeader("Content-Length", intToString(client.getSentBytes()));
	}
	else {
		client.sendErrorPage(404);
	}
}

std::string ServerCluster::extract_boundary(const std::string &headers)
{
	std::string boundary;
	size_t boundaryPos = headers.find("boundary=");
	if (boundaryPos != std::string::npos) {
		boundary = headers.substr(boundaryPos + 9);
		size_t endPos = boundary.find("\r\n");
		if (endPos != std::string::npos) {
			boundary = boundary.substr(0, endPos);
		}
	}
	return boundary;
}

bool ServerCluster::allowed_in_path(const std::string &file_path, Client &client)
{
	if (file_path.find(client.getServer()->getRoot()) == std::string::npos)
		return false;
	return true;
}
