#pragma once
#include "./ServerCluster.hpp"
#include "../Utils/utils.hpp"

void handle_request(Client &client);
void handle_get_request(Client &client);
void handle_post_request(Client &client);
void handle_delete_request(Client &client);
bool allowed_in_path(const std::string &file_path, Client &client);