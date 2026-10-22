#include "ClientManager.h"

std::unordered_map<std::string, SOCKET> ClientManager::s_clients;
std::mutex ClientManager::s_mutex;
std::mutex ClientManager::s_sendMutex;

void ClientManager::Register(const std::string& username, SOCKET s) {
	if (username.empty()) return;
	std::lock_guard<std::mutex> lock(s_mutex);
	
	s_clients[username] = s;
}

void ClientManager::Remove(const std::string& username) {
	if (username.empty()) return;
	std::lock_guard<std::mutex> lock(s_mutex);
	s_clients.erase(username);
}

SOCKET ClientManager::Find(const std::string& username) {
	std::lock_guard<std::mutex> lock(s_mutex);
	auto it = s_clients.find(username);
	if (it != s_clients.end()) {
		return it->second;
	}
	return INVALID_SOCKET;
}

bool ClientManager::SendTo(const std::string& username, const std::string& msg) {
	SOCKET sock = Find(username);
	if (sock == INVALID_SOCKET) {

	}
	std::string packet = msg + "\n";
	std::lock_guard<std::mutex> lock(s_sendMutex);
	int total = 0;
	int len = (int)packet.length();
	while (total < len) {
		int sent = send(sock, packet.c_str() + total, len - total, 0);
		if (sent == SOCKET_ERROR) return false;
		total += sent;
	}
	return true;
}