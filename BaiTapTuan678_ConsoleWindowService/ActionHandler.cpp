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

std::string ActionHandler::ProcessRequest(const std::string& rawMessage, SOCKET callerSocket) {
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

    switch (actionCode) {
    case ACTION_LOGIN:
        return HandleLogin(fields, callerSocket);

    case ACTION_REGISTER:
        return HandleSignin(fields);

    case ACTION_GET_FRIENDS:
        return HandleGetFriend(fields);

    case ACTION_SEND_MESSAGE:
        return HandleSendMessage(fields);

    case ACTION_LOAD_MESSAGE:
        return HandleLoadMessages(fields);
    case ACTION_REGISTER_CLIENT: 
    {
        return HandleRegisterClient(fields, callerSocket);
       
    }

    default:
        return "FAIL|UNKNOWN_ACTION";
    }
}

std::string ActionHandler::HandleRegisterClient(const std::vector<std::string>& fields, SOCKET callerSocket) {
    // 6|
    if (fields.size() < 2) return "FAIL|MISSING_PARAMETERS";
    ClientManager::Register(fields[1], callerSocket);
    ClientManager::Broadcast("7|ONLINE|" + fields[1], fields[1]);
    return "OK";
}

std::string ActionHandler::HandleLogin(const std::vector<std::string>& fields, SOCKET callerSocket) {
    //1|username|password
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

std::string ActionHandler::HandleSignin(const std::vector<std::string>& fields) {
    if (fields.size() < 3) return "FAIL|MISSING_PARAMETERS";
    string rs;
    CString username(fields[1].c_str());
    CString password(fields[2].c_str());

    if (g_db.CheckExist(username)) {
        rs = "FAIL|USERNAME EXISTED";
    }
    else {
        Info info;
        info.username = username;
        info.password = password;
        g_db.InsertData(info);
        rs = "OK|SIGNIN_SUCCESS";
    }
    return rs;
}

std::string ActionHandler::HandleGetFriend(const std::vector<std::string>& fields) {
    if (fields.size() < 2) return "FAIL|MISSING_PARAMETERS";

    std::vector<std::string> listFriend = g_db.AddListFriend(CString(fields[1].c_str()));

    if (listFriend.empty()) {
        return "FAIL|CANNOT_GET_FRIEND_LIST";
    }

    std::string rs = "OK|" + std::to_string(listFriend.size());
    for (const auto& friendName : listFriend) {
        // Nếu đang online trong Map thì thêm '+' vào tên gửi về
        if (ClientManager::IsOnline(friendName)) {
            rs += "|" + friendName + "+";
        }
        else {
            rs += "|" + friendName;
        }
    }
    return rs;
}

std::string ActionHandler::HandleSendMessage(const std::vector<std::string>& fields) {
    // 3|sender|receiver|content
    if (fields.size() < 4) return "FAIL|MISSING_PARAMETERS";

    bool ok = g_db.InsertMessage(CString(fields[3].c_str()), CString(fields[1].c_str()), CString(fields[2].c_str()));

    // gửi sang bên đầu bên kia
    if (!ok) return "FAIL|CANNOT_SAVE_MESSAGE";
    std::string isOnline = "";
    if (ClientManager::SendTo(fields[2], "NEW_MSG|" + fields[1])) {
        isOnline = "OK|MESSAGE_SENT_ONLINE";
    }
    else {
        isOnline = "OK|MESSAGE_SENT_OFFLINE";
    }

    return isOnline;
}

std::string ActionHandler::HandleLoadMessages(const std::vector<std::string>& fields) {
    if (fields.size() < 3) return "FAIL|MISSING_PARAMETERS";

    CString sender(fields[1].c_str());
    CString receiver(fields[2].c_str());

    std::vector<message> messages = g_db.LoadMessage(sender, receiver);
    std::string response = "OK|" + std::to_string(messages.size());

    for (const auto& msg : messages) {
        response += "|" + std::string(CT2A(msg.sender))
            + "|" + std::string(CT2A(msg.content))
            + "|" + std::string(CT2A(msg.sent_time));
    }

    return response;
}

