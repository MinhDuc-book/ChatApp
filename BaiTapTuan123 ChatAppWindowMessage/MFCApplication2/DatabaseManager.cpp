#include "pch.h"
#include "DatabaseManager.h"
#include <string>
#include <iostream>
#include <atlconv.h>

// App1
using namespace std;
bool DatabaseManager::OpenDatabase(CString path) {
    CT2A str(path); // -> char *
    return sqlite3_open(string(str).c_str(), &db) == SQLITE_OK;
}

void DatabaseManager::CloseDatabase() {
    sqlite3_close(db);
    db = nullptr;
}
DatabaseManager::~DatabaseManager() {
    CloseDatabase();
}


void DatabaseManager::CreateTables() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT, 
            username TEXT NOT NULL,
            password TEXT NOT NULL
        );
    )";
    sqlite3_exec(db, sql, nullptr, nullptr, nullptr);

    sql = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            sender TEXT NOT NULL,
            receiver TEXT NOT NULL,
            content TEXT NOT NULL,
            sent_time DATETIME DEFAULT (datetime('now', 'localtime')),
            FOREIGN KEY (sender) REFERENCES users(username),
            FOREIGN KEY (receiver) REFERENCES users(username)
        );
    )";
    sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
}

bool DatabaseManager::InsertMessage(CString message, CString sender, CString receiver) {
    CT2A sder(sender);
    CT2A rver(receiver);
    CT2A mess(message);

    string sql =
        "INSERT INTO messages "
        "(sender, receiver, content) "
        "VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, sder, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, rver, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, mess, -1, SQLITE_TRANSIENT);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

vector<message> DatabaseManager::LoadMessage(CString sender, CString receiver) {
    vector<message> rs;
    CT2A sder(sender);
    CT2A rver(receiver);
    string sql = "SELECT sender, content FROM messages WHERE (sender = ? AND receiver = ?)"
        "OR (sender = ? AND receiver = ? ) ORDER BY sent_time ASC;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return rs;

    sqlite3_bind_text(stmt, 1, sder, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, rver, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, rver, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, sder, -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        // cast because sqlite3...text return const unsigned char*
        const char* senderDB =
            reinterpret_cast<const char*>(
                sqlite3_column_text(stmt, 0));

        const char* contentDB =
            reinterpret_cast<const char*>(
                sqlite3_column_text(stmt, 1));

        if (senderDB != nullptr && contentDB != nullptr)
        {
            message msg;

            msg.sender = CString(senderDB);
            msg.content = CString(contentDB);

            rs.push_back(msg);
        }
    }
    sqlite3_finalize(stmt);
    return rs;
}

vector<string> DatabaseManager::AddListFriend(CString curr_user)
{
    vector<string> rs;
    CT2A cur_user(curr_user);
    string sql = "SELECT username FROM users WHERE ";
    sql = sql + "username != '" + string(cur_user) + "';";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const unsigned char* username = sqlite3_column_text(stmt, 0);

            if (username != nullptr)
            {
                rs.push_back(
                    reinterpret_cast<const char*>(username) // cannot use string instead of const char because this convert reference not convert type
                );
            }
        }
    }

    sqlite3_finalize(stmt);

    return rs;
}

CString DatabaseManager::GetUserFromID(int id)
{
    CString curr_user;

    string sql = "SELECT username FROM users WHERE id = ";
    sql += to_string(id) + ";";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            curr_user = CString(
                reinterpret_cast<const char*>(
                    sqlite3_column_text(stmt, 0)
                    )
            );
        }
        else
        {
            curr_user = _T("ERROR");
        }
    }
    else
    {
        curr_user = _T("ERROR");
    }

    sqlite3_finalize(stmt);
    return curr_user;
}

void DatabaseManager::InsertData(Info info) {
    // CT2A: Convert Tchar To ANSI (char *)
    string sql = "INSERT INTO users (username, password) VALUES (";
    CT2A usr(info.username);
    CT2A pwd(info.password);
    sql += "'" + string(usr) + "', ";
    sql += "'" + string(pwd) + "'";

    sql += ");";


    sqlite3_exec(
        db,
        sql.c_str(),
        nullptr,
        nullptr,
        nullptr
    );
}

bool DatabaseManager::CheckExist(CString username) {
    CT2A usr(username);
    string sql = "SELECT username FROM users WHERE ";
    sql = sql + "username = " + "'" + string(usr) + "'" + ";";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            sqlite3_finalize(stmt);
            return true;
        }
    }
    sqlite3_finalize(stmt);
    return false;
}

//void DatabaseManager::LoadData(string sql) {
//    // need to change something from parameter 3
//    rs = _T("");
//    sqlite3_exec(db, sql.c_str(), CallBack, &rs, nullptr);
//}

// > 0 = success login
// -1 = login fail
int DatabaseManager::CheckLogin(CString username, CString password) {
    CT2A usr(username);
    CT2A pass(password);
    int usr_id;

    string sql = "SELECT id FROM users WHERE ";
    sql += "username = '" + string(usr) + "' ";
    sql += "AND password = '" + string(pass) + "';";

    // sqlite3_prepare_v2: prepare before perform a command
    /*
    * arg1: connect to database
    * arg2: SQL command
    * arg3: number of byte of SQL command, if == -1 ==> SQL auto correct
    * arg4: pointer to a var which save the command performed successfully
    * arg5: use when the SQL command have > 1 command, often == nullptr
    * return: SQLITE_OK if compiled successfully or error code
    */
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        //sqlite3_step: perform (compile) and get output
        /*
        * arg1: point to the prepared command (prepared by sqlite3_prepare...)
        * return SQLITE_ROW (use with SELECT if result have least 1 line) / SQLITE_DONE(use with INSERT, OPEN,.../...
        */
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            //sqlite3_column_int: get value from a column of current output line
            /*
            * arg1: point to the prepared command
            * arg2: index of column which want to get value in current output
            */
            usr_id = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
            return usr_id;
        }
    }
    sqlite3_finalize(stmt);
    return -1;
}

// data, pRs also point to DatabaseManager.rs
// CallBack() use for processing each line in output of query
int CallBack(void* data, int argc, char** argv, char** errmsg) {

    return 0; //have to return 0 to continue query other line
}

