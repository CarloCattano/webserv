#include "./HandleRequest.hpp"

// void handle_response2(Client &client) {
// 	Response response = client.getResponse();

// 	std::string response_string = client.responseToString();
// 	client.setResponseSize(response_string.size());
// 	client.setSentBytes(client.getSentBytes() +
// 						send(client.getFd(), response_string.c_str(), 4096, 0));

// 	if (client.getSentBytes() >= response_string.size()) {
// 	}
// }

void handle_request(Client &client) {
	char buffer[4096];

	int bytes_read = recv(client.getFd(), buffer, 4096, 0);

	if (bytes_read == -1) {
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

	if (client.getRequest().method == "GET") {
		handle_get_request(client);
	} else if (client.getRequest().method == "POST") {
		handle_post_request(client);
	} else if (client.getRequest().method == "DELETE")
		handle_delete_request(client);
	else {
		// TODO - check if 405 is in the server error pages
		client.sendErrorPage(405);
	}
}

void update_response(Client &client, std::string body, std::string content_type) {
	client.setResponseBody(body);
	client.addResponseHeader("Content-Type", content_type);
	client.addResponseHeader("Content-Length", intToString(body.size()));
	client.setResponseStatusCode(200);
	client.addResponseHeader("Connection", "keep-alive");
}

void handle_get_request(Client &client) {

	Server 		*server = client.getServer();
	Response 	response;
	std::string full_path = "." + server->getRoot() + client.getRequest().uri;
	std::string	body;
	std::string	content_type;

	if (isFolder(full_path) && directory_contains_index_file(full_path))
		full_path += "index.html";

	if (isFolder(full_path) == true && server->getAutoindex() == true) {
		body = generateDirectoryListing(full_path);
		content_type = "text/html";
	}
	else if (isFile(full_path)) {
		body = readFileToString(full_path);;
		content_type = getContentType(full_path);;
	}
	else {
		client.sendErrorPage(404);
		return;
	}
	update_response(client, body, content_type);
}

void handle_post_request(Client &client) {
	std::string full_path = "." + client.getServer()->getRoot() + client.getRequest().uri;

	if (isFolder(full_path)) {
		client.sendErrorPage(403);
		return;
	} else {
		log("POST request");
		handle_cgi_request(client, const_cast<char*>(full_path.c_str()));
	}

	// its causing issues when i dont do this and handle response is called...
	// client.setResponseBody("POST request");
	// client.setResponseStatusCode(200);
	// client.addResponseHeader("Content-Type", "text/html");
	// client.addResponseHeader("Content-Length", intToString(13));
	// client.addResponseHeader("Connection", "keep-alive");
}

void handle_delete_request(Client &client) {

	Response response;

	std::string full_path = "." + client.getServer()->getRoot() + client.getRequest().uri;

	int is_allowed = allowed_in_path(full_path, client);

	if (isFolder(full_path)) {
		client.sendErrorPage(403);
	} else if (is_allowed == true) {
		if (std::remove(full_path.c_str()) == 0) {
		};
		std::cout << "file " << full_path.c_str() << " was deleted from the server" << std::endl;
		client.setResponseStatusCode(200);
		client.setResponseBody("File was deleted successfully");
		client.addResponseHeader("Content-Type", "text/html");
		client.addResponseHeader("Content-Length", intToString(client.getSentBytes()));
	}
}

bool allowed_in_path(const std::string &file_path, Client &client) {

	if (file_path.find(client.getServer()->getRoot()) == std::string::npos)
		return false;
	return true;
}
