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
		return false;
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

bool ClientManager::IsOnline(const std::string& username) {
	std::lock_guard<std::mutex> lock(s_mutex);
	return s_clients.find(username) != s_clients.end();
}

void ClientManager::Broadcast(const std::string& msg, const std::string& excludeUser) {
	std::string packet = msg + "\n";
	std::lock_guard<std::mutex> lock(s_mutex);
	std::lock_guard<std::mutex> sendLock(s_sendMutex);

	for (const auto& client : s_clients) {
		if (client.first != excludeUser && client.second != INVALID_SOCKET) {
			int total = 0;
			int len = (int)packet.length();
			while (total < len) {
				int sent = send(client.second, packet.c_str() + total, len - total, 0);
				if (sent == SOCKET_ERROR) return;
				total = total + sent;
			}
		}
	}
}