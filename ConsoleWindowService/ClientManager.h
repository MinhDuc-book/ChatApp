#pragma once
#include <winsock2.h>
#include <string>
#include <unordered_map>
#include <mutex>

class ClientManager {
private:
	// không sử dụng volatile vì nó chỉ phù hợp trong việc đếm tăng/giảm thông qua InterlockedIncrease/Decrease...
	// còn ở đây ngoài tăng giảm thì còn phải ghi dữ liệu
	static std::unordered_map<std::string, SOCKET> s_clients;
	static std::mutex s_mutex;
	static std::mutex s_sendMutex;

public:
	static void Register(const std::string& username, SOCKET s);

	static void Remove(const std::string& username);

	static SOCKET Find(const std::string& username);

	static bool SendTo(const std::string& username, const std::string& msg);
};