#include "ActionHandler.h"
#include <sstream>

DatabaseManager ActionHandler::g_db;

void ActionHandler::Init(const std::string& dbPath) {
    g_db.OpenDatabase(dbPath);
    g_db.CreateTables(); 
}

void ActionHandler::Cleanup() {
    g_db.CloseDatabase();
}

std::vector<std::string> ActionHandler::Split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string ActionHandler::ProcessRequest(const std::string& rawMessage) {
    if (rawMessage.empty()) {
        return "FAIL|EMPTY_REQUEST";
    }

    std::vector<std::string> fields = Split(rawMessage, '|');
    if (fields.empty()) {
        return "FAIL|INVALID_FORMAT";
    }

    int actionCode = 0;
    try {
        actionCode = std::stoi(fields[0]);
    }
    catch (...) {
        return "FAIL|INVALID_ACTION_CODE";
    }

    // Điều hướng tới đúng logic tương ứng
    switch (actionCode) {
    case ACTION_LOGIN:
        return HandleLogin(fields);

    case ACTION_GET_FRIENDS:
        return HandleGetFriend(fields);

    case ACTION_SEND_MESSAGE:
        return HandleSendMessage(fields);

    case ACTION_LOAD_MESSAGE:
        return HandleLoadMessages(fields);

    default:
        return "FAIL|UNKNOWN_ACTION";
    }
}

// Logic kiểm tra Đăng nhập
std::string ActionHandler::HandleLogin(const std::vector<std::string>& fields) {
    // Định dạng: 1|username|password
    if (fields.size() < 3) {
        return "FAIL|MISSING_PARAMETERS";
    }

    ActionLogin loginAction;
    loginAction.username = CString(fields[1].c_str());
    loginAction.password = CString(fields[2].c_str());

    int loginResult = g_db.CheckLogin(loginAction.username, loginAction.password);

    if (loginResult > 0) { 
        return ("OK");
    }
    else {
        return "FAIL|INVALID_CREDENTIALS";
    }
}

std::string ActionHandler::HandleGetFriend(const std::vector<std::string>& fields) {
    if (fields.size() < 2)  {
        return "FAIL|MISSING_PARAMETERS";
    }
    ActionGetFriend getFriendAction;
    getFriendAction.curr_user = CString(fields[1].c_str());
    std::vector<std::string> listFriend = g_db.AddListFriend(CString(getFriendAction.curr_user));

    string rs = "";
    if (listFriend.empty() == false) {
        rs = "OK|" + std::to_string(listFriend.size());
        for (int i = 0; i < listFriend.size(); ++i) {
            rs = rs + "|" + listFriend[i];
        }
    }
    else {
        return "FAIL|CANNOT_GET_FRIEND_LIST";
    }
    return rs;
}

// Logic gửi tin nhắn (sau này bổ sung tiếp)
std::string ActionHandler::HandleSendMessage(const std::vector<std::string>& fields) {
    // Định dạng: 3|sender|receiver|content
    if (fields.size() < 4) return "FAIL|MISSING_PARAMETERS";

    bool ok = g_db.InsertMessage(CString(fields[3].c_str()), CString(fields[1].c_str()), CString(fields[2].c_str()));
    return ok ? "OK|MESSAGE_SENT" : "FAIL|CANNOT_SAVE_MESSAGE";
}

// Logic load tin nhắn
std::string ActionHandler::HandleLoadMessages(const std::vector<std::string>& fields) {
    // Định dạng yêu cầu từ client: 4|sender|receiver
    if (fields.size() < 3) return "FAIL|MISSING_PARAMETERS";

    CString sender(fields[1].c_str());
    CString receiver(fields[2].c_str());

    // 1. Lấy danh sách tin nhắn từ Database
    std::vector<message> messages = g_db.LoadMessage(sender, receiver);

    // 2. Bắt đầu chuỗi với: OK|<số lượng tin nhắn>
    std::string response = "OK|" + std::to_string(messages.size());

    // 3. Đóng gói từng cặp sender và content vào chuỗi
    // Cấu trúc chuỗi: OK|2|Nam|chao cau|Hoa|to day
    for (const auto& msg : messages) {
        response += "|" + std::string(CT2A(msg.sender)) + "|" + std::string(CT2A(msg.content));
    }

    return response;
}