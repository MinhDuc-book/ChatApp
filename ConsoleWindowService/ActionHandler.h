#pragma once
#include <string>
#include <vector>
#include "Action.h"
#include "DatabaseManager.h"

class ActionHandler {
public:
    // Khởi tạo và truyền DatabaseManager vào
    static void Init(const std::string& dbPath);
    static void Cleanup();

    // Hàm nhận chuỗi thô từ TCP -> xử lý toàn bộ logic -> trả về chuỗi phản hồi cho client
    static std::string ProcessRequest(const std::string& rawMessage);

private:
    static DatabaseManager g_db;

    // Helper tách chuỗi phân cách bởi ký tự delimiter (mặc định '|')
    static std::vector<std::string> Split(const std::string& str, char delimiter = '|');

    // Các hàm xử lý nghiệp vụ cụ thể
    static std::string HandleLogin(const std::vector<std::string>& fields);
    static std::string HandleGetFriend(const std::vector<std::string>& fields);
    static std::string HandleSendMessage(const std::vector<std::string>& fields);
    static std::string HandleLoadMessages(const std::vector<std::string>& fields);
};