
// TestWindowServiceDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "TestWindowService.h"
#include "TestWindowServiceDlg.h"
#include "afxdialogex.h"
#include <string>
#include <vector>
#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTestWindowServiceDlg dialog



CTestWindowServiceDlg::CTestWindowServiceDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TESTWINDOWSERVICE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestWindowServiceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTestWindowServiceDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_EDIT1, &CTestWindowServiceDlg::OnEnChangeEdit1)
	ON_EN_CHANGE(IDC_EDIT2, &CTestWindowServiceDlg::OnEnChangeEdit2)
	ON_BN_CLICKED(IDC_BUTTON1, &CTestWindowServiceDlg::OnBnClickedButton1)
	ON_MESSAGE(WM_PIPE_MSG, &CTestWindowServiceDlg::OnPipeMsg)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CTestWindowServiceDlg message handlers

BOOL CTestWindowServiceDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ShowWindow(SW_MINIMIZE);
	StartTCP();

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTestWindowServiceDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestWindowServiceDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}


// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTestWindowServiceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTestWindowServiceDlg::StartTCP() {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		AfxMessageBox(_T("Cannot WSAStartUp"));
		return;
	}

	m_recvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_recvSocket == INVALID_SOCKET) {
		AfxMessageBox(_T("Cannot init socket"));
		return;
	}

	int option = 1;
	setsockopt(m_recvSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&option, sizeof(option));

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(8888);

	if (bind(m_recvSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		AfxMessageBox(_T("Bind failed"));
		closesocket(m_recvSocket);
		m_recvSocket = INVALID_SOCKET;
	}
	listen(m_recvSocket, SOMAXCONN);
	m_stopping = false;
	m_pServerThread = AfxBeginThread(TCPRecvThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	m_pServerThread->m_bAutoDelete = FALSE;
	m_pServerThread->ResumeThread();
}

LRESULT CTestWindowServiceDlg::OnPipeMsg(WPARAM, LPARAM lParam)
{
	CString* msg = reinterpret_cast<CString*>(lParam);
	if (msg)
	{
		SetDlgItemText(IDC_EDIT2, *msg);   // hiển thị lên editbox 2
		delete msg;                        // giải phóng CString do thread cấp phát
	}
	return 0;
}

UINT CTestWindowServiceDlg::TCPRecvThread(LPVOID p) {
	CTestWindowServiceDlg* self = static_cast<CTestWindowServiceDlg*>(p);
	HWND hwnd = self->m_hWnd;

	while (!self->m_stopping)
	{
		sockaddr_in clientAddr = {};
		int addrLen = sizeof(clientAddr);
		SOCKET clientSocket = accept(self->m_recvSocket, (sockaddr*)&clientAddr, &addrLen);// block

		if (clientSocket == INVALID_SOCKET) break;

		std::wstring data;
		wchar_t buf[1024];
		int check;
		while (true) {
			check = recv(clientSocket, (char*)buf, sizeof(buf) - 1, 0);
			if (check <= 0) break;
			buf[check / sizeof(wchar_t)] = L'\0';
			data = data + buf;
			if (data.size() > 32 * 1024) break;
		}
		// done receive => close
		closesocket(clientSocket);
		if (data.empty() == false) {
			CString* msg = new CString(data.c_str());
			BOOL rs = ::PostMessage(hwnd, WM_PIPE_MSG, 0, (LPARAM)msg);
			if (!rs) delete msg;
		}
	}
	return 0;
}

bool CTestWindowServiceDlg::SendViaTCP(const char* targetIP, const CString& payload) {
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) return false;

	bool success = false;

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons((u_short)9999);
	inet_pton(AF_INET, targetIP, &addr.sin_addr); // convert IP string to binary form

	if (connect(s, (sockaddr*)&addr, sizeof(addr)) != SOCKET_ERROR) {
		//send
		int bytes = (payload.GetLength() + 1) * 2;
		const char* p = (const char*)payload.GetString();
		int sent = 0;
		while (sent < bytes) {
			int w = send(s, p + sent, bytes - sent, 0);
			if (w == SOCKET_ERROR) break;
			sent = sent + w;
		}
		success = true;

		// receive
		if (sent == bytes) {
			// nhận lại đủ đúng số byte đã gửi (echo)
			std::vector<char> rx(bytes + sizeof(wchar_t), 0);
			int got = 0;
			while (got < bytes) {
				int r = recv(s, rx.data() + got, bytes - got, 0);
				if (r <= 0) break;
				got += r;
			}

			if (got == bytes) {
				SetDlgItemText(IDC_EDIT2, (const wchar_t*)rx.data());
				success = true;
			}
		}
		
	}
	else {
		success = false;
		AfxMessageBox(_T("Cannot Connect"));
		
	}
	closesocket(s);
	return success;
}

void CTestWindowServiceDlg::OnBnClickedButton1()
{
	CString content;
	GetDlgItemText(IDC_EDIT1, content);
	if (SendViaTCP("127.0.0.1", content) == false) {
		
		AfxMessageBox(_T("Cannot Send"));
		return;
	}
	else {
		SetDlgItemText(IDC_EDIT1, _T(""));
	}
	
	// TODO: Add your control notification handler code here
}

void CTestWindowServiceDlg::OnDestroy()
{
	WSACleanup();
	CDialogEx::OnDestroy();
}


void CTestWindowServiceDlg::OnEnChangeEdit1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CTestWindowServiceDlg::OnEnChangeEdit2()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
