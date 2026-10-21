#pragma once
#include "sqlite3.h"
#include <string>
#include <vector>
#include <atlstr.h> 

using namespace std;

struct Info {
	CString username;
	CString password;
};

struct message {
	CString sender;
	CString receiver;
	CString content;
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

	void CreateTables();
	void InsertData(Info info);
	int CheckLogin(CString username, CString password);
	bool CheckExist(CString username);
	bool InsertMessage(CString message, CString sender, CString receiver);
	vector<message> LoadMessage(CString sender, CString receiver);
	vector<string> AddListFriend(CString curr_user);
	CString GetUserFromID(int id);
};
