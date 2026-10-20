#pragma once
#include <string>

class Action {
public:
	int action;
};

class ActionLogin:Action {
public:
	char* username;
	char* password;
};