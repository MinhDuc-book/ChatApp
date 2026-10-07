#pragma once
#include "sqlite3.h"
#include <string>
#include <vector>
#include "resource.h"
#include <afxstr.h> 
#include <vector>

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
	CString rs;

	~DatabaseManager();
	bool OpenDatabase(CString path);
	void CloseDatabase();

	//void LoadData(string sql);
	void CreateTables();
	bool InsertData(Info info);
	int CheckLogin(CString username, CString pass);
	bool CheckExist(CString username);

	bool InsertMessage(CString mess, CString sender, CString receiver);
	vector<message> LoadMessage(CString sender, CString receiver);

	vector<string> AddListFriend(CString curr_user);
	CString GetUserFromID(int id);

	// static is use for all object => belong to CLASS, not belong to OBJECT
	// static int CallBack(void* data, int argc, char** argv, char** errmsg);
};

