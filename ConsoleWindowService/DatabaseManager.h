#pragma once
#include "sqlite3.h"
#include <string>
#include <vector>

using namespace std;

struct Info {
	string username;
	string password;
};

struct message {
	string sender;
	string receiver;
	string content;
};

int CallBack(void* data, int argc, char** argv, char** errmsg);

class DatabaseManager {
private:
	sqlite3* db = nullptr;
public:
	string rs;
	~DatabaseManager();
	bool OpenDatabase(string path);
	void CloseDatabase();

	bool CreateTable();
	bool InsertData(Info info);
	int CheckLogin(string username, string password);
	bool CheckExist(string username);
	bool InsertMessage(string message, string sender, string receiver);
	vector<message> LoadMessage(string sender, string receiver);
	vector<string> AddListFriend(string curr_user);
	string GetUserFromID(int id);
};
