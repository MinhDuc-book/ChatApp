#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>

#include <windows.h>
#include <tchar.h>
#include <string>
#include <strsafe.h>
#include "Sample.h"
#include "Action.h"
#include <vector>
#include <atlstr.h> 
#include "DatabaseManager.h"
#include <sstream>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "advapi32.lib")

#define SVCNAME TEXT("SvcEcho")

SERVICE_STATUS          gSvcStatus;
SERVICE_STATUS_HANDLE   gSvcStatusHandle;
HANDLE                  ghSvcStopEvent = NULL;
SOCKET gListenSocket = INVALID_SOCKET;

int RECEIVE_PORT = 9999;

VOID SvcInstall(void);
VOID WINAPI SvcCtrlHandler(DWORD);
VOID WINAPI SvcMain(DWORD, LPTSTR*);

VOID ReportSvcStatus(DWORD, DWORD, DWORD);
VOID SvcInit(DWORD, LPTSTR*);
VOID SvcReportEvent(LPTSTR);
DWORD WINAPI ServiceWorkerThread(LPVOID pParam);
std::vector<std::string> ParseStringReceivedLogin(const char* mess);
DWORD WINAPI ClientThread(LPVOID pParam);

  