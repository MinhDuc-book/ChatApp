#include "ConsoleWindowService.h"
#include "ActionHandler.h"
//
// Purpose: 
//   Entry point for the process
//
// Parameters:
//   None
// 
// Return value:
//   None, defaults to 0 (zero)
//
int __cdecl _tmain(int argc, TCHAR* argv[])
{
    // If command-line parameter is "install", install the service. 
    // Otherwise, the service is probably being started by the SCM.

    if (argc > 1 && lstrcmpi(argv[1], TEXT("install")) == 0)
    {
        SvcInstall();
        return 0;
    }

    // TO_DO: Add any additional services for the process to this table.
    SERVICE_TABLE_ENTRY DispatchTable[] =
    {
        { (LPWSTR)SVCNAME, (LPSERVICE_MAIN_FUNCTION)SvcMain },
        { NULL, NULL }
    };

    // This call returns when the service has stopped. 
    // The process should simply terminate when the call returns.

    if (!StartServiceCtrlDispatcher(DispatchTable))
    {
        SvcReportEvent((LPTSTR)TEXT("StartServiceCtrlDispatcher"));
        
    }
}

//
// Purpose: 
//   Installs a service in the SCM database
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID SvcInstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szUnquotedPath[MAX_PATH];

    if (!GetModuleFileName(NULL, szUnquotedPath, MAX_PATH))
    {
        printf("Cannot install service (%lu)\n", GetLastError());
        return;
    }

    // In case the path contains a space, it must be quoted so that
    // it is correctly interpreted. For example,
    // "d:\my share\myservice.exe" should be specified as
    // ""d:\my share\myservice.exe"".
    TCHAR szPath[MAX_PATH];
    StringCbPrintf(szPath, MAX_PATH, TEXT("\"%s\""), szUnquotedPath);

    // Get a handle to the SCM database. 

    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Create the service
    // this require SCM save info of service, after this step Windows know SvcEcho is existed
    schService = CreateService(
        schSCManager,              // SCM database 
        SVCNAME,                   // name of service 
        SVCNAME,                   // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_DEMAND_START,      // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        szPath,                    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 

    if (schService == NULL)
    {
        printf("CreateService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }
    else printf("Service installed successfully\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

//
// Purpose: 
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None.
//
VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    // Register the handler function for the service: SvcCtrlHandler()

    gSvcStatusHandle = RegisterServiceCtrlHandler(
        SVCNAME,
        SvcCtrlHandler);

    if (!gSvcStatusHandle)
    {
        SvcReportEvent((LPTSTR)TEXT("RegisterServiceCtrlHandler"));
        return;
    }

    // These SERVICE_STATUS members remain as set here
    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gSvcStatus.dwServiceSpecificExitCode = 0;

    // init the ws2_32.dll
    WSADATA wsaData;
    int rs = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rs != 0) {
        SvcReportEvent((LPTSTR)TEXT("TCPStartup_ws2_32dll"));
        return;
    }
    // init database
    ActionHandler::Init("D:\\AAMduwccc\\project\\C++\\usr.sql");

    // Report initial status to the SCM
    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // Perform service-specific initialization and work.
    SvcInit(dwArgc, lpszArgv);
}

bool SendFull(SOCKET s, const char* data, int len) {
    int sent = 0;
    while (sent < len) {
        int w = send(s, data + sent, len - sent, 0);
        if (w == SOCKET_ERROR) return false;
        sent += w;
    }
    return true;
}

// Number of client running
volatile LONG gActiveClients = 0;

// 1 client 1 thread
/*
* TCP mở 1 đường kết nối, các gói tin đi qua TCP có thể bị dính vào nhau (VD khi gửi 2 tn liên tiếp) 
* hoặc bị tách ra mà không đúng cấu trúc gốc của từng request
* Sử dụng accumulated kết hợp với \n để phân biệt giữa các lần gửi request tới server rồi xử lí từng phần
*/
DWORD WINAPI ClientThread(LPVOID pParam)
{
    SOCKET s = (SOCKET)(UINT_PTR)pParam;
    char buf[1024];
    std::string accumulated = "";   
    std::string username = "";  
    while (WaitForSingleObject(ghSvcStopEvent, 0) == WAIT_TIMEOUT)
    {
        fd_set rs;
        FD_ZERO(&rs);
        FD_SET(s, &rs);
        timeval tv = { 1, 0 };
        int r = select(0, &rs, NULL, NULL, &tv);
        if (r == SOCKET_ERROR) break;
        if (r == 0) continue;
        int n = recv(s, buf, sizeof(buf) - 1, 0);
        if (n <= 0) break;
        buf[n] = '\0';
        accumulated += buf;
        // tách
        size_t pos;
        while ((pos = accumulated.find('\n')) != std::string::npos)
        {
            std::string line = accumulated.substr(0, pos);
            accumulated.erase(0, pos + 1);
            // Bỏ \r nếu có
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            std::string response = ActionHandler::ProcessRequest(line, s);

            if (response == "OK" && line.size() >= 2) {
                // Nếu là mã 1
                if (line[0] == '1' && line[1] == '|') {
                    size_t first = line.find('|');
                    size_t second = line.find('|', first + 1);
                    if (first != std::string::npos && second != std::string::npos) {
                        username = line.substr(first + 1, second - first - 1);
                    }
                }
                // Nếu là mã 6
                else if (line[0] == '6' && line[1] == '|') {
                    size_t first = line.find('|');
                    if (first != std::string::npos) {
                        username = line.substr(first + 1);
                    }
                }
            }
        }
    }
    ClientManager::Remove(username);
    closesocket(s);
    InterlockedDecrement(&gActiveClients);
    return 0;
}

//accept and create thread for each client
DWORD WINAPI ServiceWorkerThread(LPVOID pParam)
{
    while (WaitForSingleObject(ghSvcStopEvent, 0) == WAIT_TIMEOUT)
    {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(gListenSocket, &readSet);
        timeval tv = { 1, 0 };

        int sel = select(0, &readSet, NULL, NULL, &tv);
        if (sel == SOCKET_ERROR) break;
        if (sel == 0) continue;               // timeout

        SOCKET client = accept(gListenSocket, NULL, NULL);
        if (client == INVALID_SOCKET) continue;

        InterlockedIncrement(&gActiveClients);
        HANDLE hClient = CreateThread(NULL, 0, ClientThread,
            (LPVOID)(UINT_PTR)client, 0, NULL);
        if (hClient == NULL)
        {
            InterlockedDecrement(&gActiveClients);
            closesocket(client);
            continue;
        }
        CloseHandle(hClient);                 
    }

    // Wait all client stop
    while (InterlockedCompareExchange(&gActiveClients, 0, 0) > 0) Sleep(50);

    return 0;
}

VOID SvcInit(DWORD dwArgc, LPTSTR* lpszArgv)
{
    // TO_DO: Declare and set any required variables.
    //   Be sure to periodically call ReportSvcStatus() with 
    //   SERVICE_START_PENDING. If initialization fails, call
    //   ReportSvcStatus with SERVICE_STOPPED.

    // Create an event. The control handler function, SvcCtrlHandler,
    // signals this event when it receives the stop control code.

    ghSvcStopEvent = CreateEvent(
        NULL,    // default security attributes
        TRUE,    // manual reset event
        FALSE,   // not signaled
        NULL);   // no name

    if (ghSvcStopEvent == NULL)
    {
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    // init socket
    gListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (gListenSocket == INVALID_SOCKET) {
        SvcReportEvent((LPTSTR)TEXT("InvalidSocket"));
        return;
    }
    //setup reuse for socket
    int option = 1;
    setsockopt(gListenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&option, sizeof(option));

    // registry socket with a port of computter
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(RECEIVE_PORT);

    //bind
    if (bind(gListenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        SvcReportEvent((LPTSTR)TEXT("CannotBind"));
        closesocket(gListenSocket);
        return;
    }
    listen(gListenSocket, SOMAXCONN);

    HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    if (hThread == nullptr) {
        DWORD err = GetLastError();
        ReportSvcStatus(SERVICE_STOPPED, err, 0);
        return;
    }


    // Report running status when initialization is complete.

    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    // TO_DO: Perform work until service stops.

    WaitForSingleObject(ghSvcStopEvent, INFINITE);
    WaitForSingleObject(hThread, INFINITE);

    closesocket(gListenSocket);
    WSACleanup();

    CloseHandle(hThread);
    CloseHandle(ghSvcStopEvent);

    ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
    
}


//
// Purpose: 
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation, 
//     in milliseconds
// 
// Return value:
//   None
//
VOID ReportSvcStatus(DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED))
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

//
// Purpose: 
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
// 
// Return value:
//   None
//
VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
    // Handle the requested control code. 

    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
        ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

        // Signal the service to stop.
        SetEvent(ghSvcStopEvent);
        closesocket(gListenSocket);
        ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);

        return;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    default:
        break;
    }

}

//
// Purpose: 
//   Logs messages to the event log
//
// Parameters:
//   szFunction - name of function that failed
// 
// Return value:
//   None
//
// Remarks:
//   The service must have an entry in the Application event log.
//
VOID SvcReportEvent(LPTSTR szFunction)
{
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    TCHAR Buffer[80];

    hEventSource = RegisterEventSource(NULL, SVCNAME);

    if (NULL != hEventSource)
    {
        StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

        lpszStrings[0] = SVCNAME;
        lpszStrings[1] = Buffer;

        ReportEvent(hEventSource,        // event log handle
            EVENTLOG_ERROR_TYPE, // event type
            0,                   // event category
            SVC_ERROR,           // event identifier
            NULL,                // no security identifier
            2,                   // size of lpszStrings array
            0,                   // no binary data
            lpszStrings,         // array of strings
            NULL);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}