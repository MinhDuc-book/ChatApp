#pragma once
#include <string>
#include <atlstr.h> 


enum ActionType {
    ACTION_UNKNOWN = 0,
    ACTION_LOGIN = 1,
    ACTION_REGISTER = 2,
    ACTION_SEND_MESSAGE = 3,
    ACTION_LOAD_MESSAGE = 4,
    ACTION_GET_FRIENDS = 5,
    ACTION_REGISTER_CLIENT = 6
};

// Cấu trúc cơ sở
class Action {
public:
    ActionType type = ACTION_UNKNOWN;
    virtual ~Action() = default;
};

// Dữ liệu cho hành động Login
class ActionLogin : public Action {
public:
    CString username;
    CString password;

    ActionLogin() { type = ACTION_LOGIN; }
};

class ActionGetFriend : public Action {
public:
    CString curr_user;
    ActionGetFriend() { type = ACTION_GET_FRIENDS; };
};

// Dữ liệu cho hành động Chat/Nhắn tin
class ActionSendMessage : public Action {
public:
    CString sender;
    CString receiver;
    CString content;

    ActionSendMessage() { type = ACTION_SEND_MESSAGE; }
};