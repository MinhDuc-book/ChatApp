// MFCApplication1Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
#include "Resource.h"
#include <string>
#include <mutex>
#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BUF_SIZE 65536

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {}
void CAboutDlg::DoDataExchange(CDataExchange* pDX) { CDialogEx::DoDataExchange(pDX); }
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CMFCApplication1Dlg dialog
CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT21, m_richChat);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_EN_CHANGE(IDC_EDIT_MESSAGE, &CMFCApplication1Dlg::OnEnChangeEditMessage)
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication1Dlg::OnBnClickedButton1)
	ON_LBN_SELCHANGE(IDC_LIST_FRIEND, &CMFCApplication1Dlg::OnLbnSelchangeListFriend)
	ON_EN_CHANGE(IDC_EDIT_MESSAGE_OTHER, &CMFCApplication1Dlg::OnEnChangeEditMessageOther)
	ON_EN_CHANGE(IDC_EDIT_NOTIFICATION, &CMFCApplication1Dlg::OnEnChangeEditNotification)

	ON_MESSAGE(WM_TCP_MSG, &CMFCApplication1Dlg::OnPipeMessage)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_RICHEDIT21, &CMFCApplication1Dlg::OnEnChangeRichedit21)
END_MESSAGE_MAP()

BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		CString strAboutMenu;
		if (strAboutMenu.LoadString(IDS_ABOUTBOX) && !strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	// Khởi tạo Winsock & Event đồng bộ
	WSADATA wsaData;
	int rs = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rs != 0) {
		AfxMessageBox(_T("Cannot WSAStartUp"));
		return FALSE;
	}
	m_hResponseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// Mở socket duy nhất kết nối tới Server Windows Service (port 9999)
	m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_hSocket == INVALID_SOCKET) {
		AfxMessageBox(_T("Không thể tạo socket!"));
		return FALSE;
	}

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9999);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

	if (connect(m_hSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		AfxMessageBox(_T("Không thể kết nối tới Server Windows Service (Port 9999)!"));
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
		return FALSE;
	}

	m_stopping = false;
	m_pServerThread = AfxBeginThread(TCPServerThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	m_pServerThread->m_bAutoDelete = FALSE;
	m_pServerThread->ResumeThread();

	CString loginPayload;
	loginPayload.Format(_T("6|%s"), (LPCTSTR)m_currentUser);
	CString loginResp;
	SendViaTCP("127.0.0.1", loginPayload, loginResp);

	// Lấy danh sách bạn bè
	CString payload;
	payload.Format(_T("%d|%s"), 5, (LPCTSTR)m_currentUser);
	CString response = _T("");
	if (SendViaTCP("127.0.0.1", payload, response) && response.Left(2) == _T("OK")) {
		CListBox* listFriend = (CListBox*)GetDlgItem(IDC_LIST_FRIEND);
		listFriend->ResetContent();
		SetDlgItemText(IDC_EDIT_CURR_USER, m_currentUser);

		int curPos = 0;
		response.Tokenize(_T("|"), curPos); // Bỏ qua "OK"
		response.Tokenize(_T("|"), curPos); // Bỏ qua số lượng
		while (curPos != -1)
		{
			CString token = response.Tokenize(_T("|"), curPos);
			token.TrimRight(_T("\r\n"));
			if (!token.IsEmpty()) {
				listFriend->AddString(token);
			}
		}
	}

	GetDlgItem(IDC_EDIT_MESSAGE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);

	return TRUE;
}

void CMFCApplication1Dlg::OnDestroy()
{
	m_stopping = true;

	// Đóng socket kết nối tới Server
	if (m_hSocket != INVALID_SOCKET) {
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
	}

	// Đợi luồng đọc kết thúc
	if (m_pServerThread) {
		WaitForSingleObject(m_pServerThread->m_hThread, 1000);
		delete m_pServerThread;
		m_pServerThread = nullptr;
	}

	if (m_hResponseEvent) {
		CloseHandle(m_hResponseEvent);
		m_hResponseEvent = NULL;
	}

	WSACleanup();
	CDialogEx::OnDestroy();
}

UINT CMFCApplication1Dlg::TCPServerThread(LPVOID p)
{
	CMFCApplication1Dlg* self = static_cast<CMFCApplication1Dlg*>(p);
	HWND hwnd = self->m_hWnd;

	char buf[4096];
	// vì khi truyền bằng TCP, đôi khi sẽ có lúc bị ghép các gói tin dính nhau
	// hoặc là bị cắt các gói tin thành nhiều phần
	// sử dụng cái này kết hợp với \n để phân biệt đâu là 1 gói tin hoàn chỉnh
	std::string accumulated = "";

	while (!self->m_stopping && self->m_hSocket != INVALID_SOCKET)
	{
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(self->m_hSocket, &readSet);
		timeval tv = { 1, 0 };

		int sel = select(0, &readSet, NULL, NULL, &tv);
		if (sel == SOCKET_ERROR) break;
		if (sel == 0) continue; // Timeout, kiểm tra lại m_stopping

		int bytes = recv(self->m_hSocket, buf, sizeof(buf) - 1, 0);
		if (bytes <= 0) break; // Server đóng kết nối hoặc lỗi mạng

		buf[bytes] = '\0';
		accumulated += buf;

		// Cắt dữ liệu theo từng dòng '\n'
		size_t pos;
		while ((pos = accumulated.find('\n')) != std::string::npos)
		{
			std::string line = accumulated.substr(0, pos);
			accumulated.erase(0, pos + 1);

			if (!line.empty() && line.back() == '\r') line.pop_back();
			if (line.empty()) continue;

			CString msg = CA2T(line.c_str());

			if (msg.Left(8) == _T("NEW_MSG|") || msg.Left(2) == _T("7|"))
			{
				CString* pMsg = new CString(msg);
				if (!::PostMessage(hwnd, WM_TCP_MSG, 0, (LPARAM)pMsg)) {
					delete pMsg;
					pMsg = nullptr;
				}
			}
			//Phản hồi của Server cho các lệnh vừa gửi đi (OK|..., FAIL|...)
			else
			{
				self->m_lastResponse = msg;
				SetEvent(self->m_hResponseEvent); // Đánh thức SendViaTCP đang đợi
			}
		}
	}
	return 0;
}



bool CMFCApplication1Dlg::SendViaTCP(const char* targetIP, const CString& payload, CString& response)
{
	if (m_hSocket == INVALID_SOCKET) return false;

	static std::mutex s_sendMutex;
	std::lock_guard<std::mutex> lock(s_sendMutex);

	ResetEvent(m_hResponseEvent);

	CString packet = payload + _T("\n");
	CT2A asciiPayload(packet);

	int sent = send(m_hSocket, asciiPayload, (int)strlen(asciiPayload), 0);
	if (sent == SOCKET_ERROR) return false;

	// Chờ luồng đọc nền TCPServerThread nhận phản hồi
	DWORD dwWait = WaitForSingleObject(m_hResponseEvent, 5000);
	if (dwWait == WAIT_OBJECT_0) {
		response = m_lastResponse;
		response.TrimRight(_T("\r\n"));
		//AfxMessageBox(response);
		return true;
	}

	return false;
}

// xử lí nhận thông báo
LRESULT CMFCApplication1Dlg::OnPipeMessage(WPARAM wParam, LPARAM lParam) {
	CString* msg = reinterpret_cast<CString*>(lParam);
	HandleIncoming(*msg);
	delete msg;
	return 1;
}

void CMFCApplication1Dlg::ParseFriendItem(const CString& itemText, CString& baseName, bool& hasNewMsg, bool& isOnline)
{
	baseName = itemText;
	baseName.TrimRight(_T("\r\n"));

	// Kiểm tra "-->"
	hasNewMsg = (baseName.Left(3) == _T("-->"));
	if (hasNewMsg) baseName = baseName.Mid(3);

	// Kiểm tra "+"
	isOnline = (!baseName.IsEmpty() && baseName.Right(1) == _T("+"));
	if (isOnline) baseName = baseName.Left(baseName.GetLength() - 1);
}

CString CMFCApplication1Dlg::FormatFriendItem(const CString& baseName, bool hasNewMsg, bool isOnline)
{
	CString res = baseName;
	if (isOnline) res = res + _T("+");
	if (hasNewMsg) res = _T("-->") + res;
	return res;
}


void CMFCApplication1Dlg::HandleIncoming(const CString& message)
{
	if (message.IsEmpty()) return;
	CString msg = message;
	msg.TrimRight(_T("\r\n"));

	CListBox* pListBox = (CListBox*)GetDlgItem(IDC_LIST_FRIEND);
	if (!pListBox) return;

	// Lấy thông tin người hiện tại đang được chọn (để giữ vị trí chọn sau khi cập nhật listbox)
	int curSel = pListBox->GetCurSel();
	CString curSelectedBaseName = _T("");
	if (curSel != LB_ERR) {
		CString text;
		pListBox->GetText(curSel, text);
		bool hasNewMsg, isOnline;
		ParseFriendItem(text, curSelectedBaseName, hasNewMsg, isOnline);
	}

	// 7|ONLINE|user hoặc 7|OFFLINE|user
	if (msg.Left(2) == _T("7|"))
	{
		int curPos = 0;
		msg.Tokenize(_T("|"), curPos); // Bỏ qua "7"
		CString status = msg.Tokenize(_T("|"), curPos);
		CString targetUser = msg.Tokenize(_T("|"), curPos);

		bool setOnline = (status == _T("ONLINE"));

		for (int i = 0; i < pListBox->GetCount(); ++i)
		{
			CString itemText, baseName;
			bool hasNewMsg, isOnline;
			pListBox->GetText(i, itemText);
			ParseFriendItem(itemText, baseName, hasNewMsg, isOnline);

			if (baseName == targetUser)
			{
				CString newText = FormatFriendItem(baseName, hasNewMsg, setOnline);
				pListBox->DeleteString(i);
				pListBox->InsertString(i, newText);
				break;
			}
		}
	}

	//NEW_MSG|sender
	else if (msg.Left(8) == _T("NEW_MSG|"))
	{
		CString sender = msg.Mid(8);

		//đúng người
		if (curSel != LB_ERR && curSelectedBaseName == sender)
		{
			LoadChatToUI(sender);
		}
		//người khác
		else
		{
			bool wasOnline = false;
			for (int i = 0; i < pListBox->GetCount(); ++i)
			{
				CString itemText, baseName;
				bool hasNewMsg, isOnline;
				pListBox->GetText(i, itemText);
				ParseFriendItem(itemText, baseName, hasNewMsg, isOnline);

				if (baseName == sender) {
					wasOnline = isOnline;
					pListBox->DeleteString(i);
					break;
				}
			}
			// Đẩy lên đầu danh sách (vị trí 0) kèm tiền tố "-->"
			CString topItem = FormatFriendItem(sender, true, wasOnline);
			pListBox->InsertString(0, topItem);
		}
	}

	if (!curSelectedBaseName.IsEmpty()) {
		for (int i = 0; i < pListBox->GetCount(); ++i) {
			CString text, baseName;
			bool dummy1, dummy2;
			pListBox->GetText(i, text);
			ParseFriendItem(text, baseName, dummy1, dummy2);
			if (baseName == curSelectedBaseName) {
				pListBox->SetCurSel(i);
				break;
			}
		}
	}
}

void CMFCApplication1Dlg::OnBnClickedButton1()
{
	CString content;
	GetDlgItemText(IDC_EDIT_MESSAGE, content);
	content.Trim();

	if (content.IsEmpty()) {
		return;
	}

	CListBox* pListBox = (CListBox*)GetDlgItem(IDC_LIST_FRIEND);
	int sel = pListBox->GetCurSel();
	if (sel == LB_ERR) {
		AfxMessageBox(_T("Vui lòng chọn người bạn muốn gửi tin nhắn!"));
		return;
	}

	CString itemText, receiverName;
	bool dummy1, dummy2;
	pListBox->GetText(sel, itemText);
	ParseFriendItem(itemText, receiverName, dummy1, dummy2); 

	CString payload = _T("3|") + m_currentUser + _T("|") + receiverName + _T("|") + content;
	CString response = _T("");

	if (!SendViaTCP("127.0.0.1", payload, response)) {
		AfxMessageBox(_T("Không thể kết nối tới Server!"));
		return;
	}

	if (response.Left(2) == _T("OK"))
	{
		SetDlgItemText(IDC_EDIT_MESSAGE, _T(""));

		LoadChatToUI(receiverName);
	}
	else
	{
		AfxMessageBox(_T("Gửi tin nhắn thất bại:\r\n") + response);
	}

	GetDlgItem(IDC_EDIT_MESSAGE)->SetFocus();
}

void CMFCApplication1Dlg::OnLbnSelchangeListFriend()
{
	CListBox* pListBox = (CListBox*)GetDlgItem(IDC_LIST_FRIEND);
	int sel = pListBox->GetCurSel();
	if (sel == LB_ERR) return;

	GetDlgItem(IDC_EDIT_MESSAGE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);

	CString itemText, baseName;
	bool hasNewMsg, isOnline;
	pListBox->GetText(sel, itemText);
	ParseFriendItem(itemText, baseName, hasNewMsg, isOnline);

	if (hasNewMsg)
	{
		CString cleanItem = FormatFriendItem(baseName, false, isOnline);
		pListBox->DeleteString(sel);
		pListBox->InsertString(sel, cleanItem);
		pListBox->SetCurSel(sel);
	}

	SetDlgItemText(IDC_EDIT_MESSAGE, _T(""));

	LoadChatToUI(baseName);

	GetDlgItem(IDC_EDIT_MESSAGE)->SetFocus();
}

void CMFCApplication1Dlg::AppendChatMessage(const CString& sender, const CString& content, const CString &timeStr, bool isMe) {
	long len = m_richChat.GetTextLength();
	m_richChat.SetSel(len, len);

	PARAFORMAT2 pf;
	ZeroMemory(&pf, sizeof(pf));
	pf.cbSize = sizeof(pf);
	pf.dwMask = PFM_ALIGNMENT;
	pf.wAlignment = isMe ? PFA_RIGHT : PFA_LEFT;
	m_richChat.SetParaFormat(pf);

	CHARFORMAT2 cf;
	ZeroMemory(&cf, sizeof(cf));
	cf.dwMask =CFM_COLOR | CFM_BOLD | CFM_SIZE;
	cf.yHeight = 150;
	cf.dwEffects = CFE_BOLD;
	cf.crTextColor = RGB(150, 150, 150);
	m_richChat.SetSelectionCharFormat(cf);

	CString timeDisplay = _T("[") + timeStr + _T("]\r\n");
	m_richChat.ReplaceSel(timeDisplay);
	
	cf.yHeight = 180;
	cf.dwEffects = 0;
	cf.crTextColor = RGB(0, 0, 0);
	m_richChat.SetSelectionCharFormat(cf);

	CString body = content + _T("\r\n\r\n");
	m_richChat.ReplaceSel(body);

	m_richChat.SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
}

void CMFCApplication1Dlg::LoadChatToUI(CString friendName)
{
	if (friendName.IsEmpty()) return;

	CString payload = _T("4|") + m_currentUser + _T("|") + friendName;
	CString response = _T("");
	if (!SendViaTCP("127.0.0.1", payload, response) || response.Left(2) != _T("OK")) {
		return;
	}
	
	m_richChat.SetWindowText(_T(""));
	int curPos = 0;
	response.Tokenize(_T("|"), curPos); // OK
	CString strCount = response.Tokenize(_T("|"), curPos);

	while (curPos != -1)
	{
		CString sender = response.Tokenize(_T("|"), curPos);
		if (curPos == -1) break;
		CString msgContent = response.Tokenize(_T("|"), curPos);
		if (curPos == -1) break;
		CString sentTime = response.Tokenize(_T("|"), curPos); 
		bool isMe = (sender == m_currentUser);
		
		AppendChatMessage(sender, msgContent, sentTime, isMe);
	}
}

BOOL CMFCApplication1Dlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		if (GetFocus() == GetDlgItem(IDC_EDIT_MESSAGE))
		{
			OnBnClickedButton1();
			return TRUE;
		}
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CMFCApplication1Dlg::OnVscrollMessageOther()
{
	if (m_syncScrolling) return;
	m_syncScrolling = true;
	CEdit* pOther = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_OTHER);
	CEdit* pMe = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_ME);
	pMe->LineScroll(pOther->GetFirstVisibleLine() - pMe->GetFirstVisibleLine());
	m_syncScrolling = false;
}

void CMFCApplication1Dlg::OnVscrollMessageMe()
{
	if (m_syncScrolling) return;
	m_syncScrolling = true;
	CEdit* pMe = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_ME);
	CEdit* pOther = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_OTHER);
	pOther->LineScroll(pMe->GetFirstVisibleLine() - pOther->GetFirstVisibleLine());
	m_syncScrolling = false;
}

void CMFCApplication1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else {
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CMFCApplication1Dlg::OnEnChangeRichedit21()
{
	
}

void CMFCApplication1Dlg::OnPaint()
{
	if (IsIconic()) {
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else {
		CDialogEx::OnPaint();
	}
}

HCURSOR CMFCApplication1Dlg::OnQueryDragIcon() {
	return static_cast<HCURSOR>(m_hIcon);
}

void CMFCApplication1Dlg::OnEnChangeEditMessage() {}
void CMFCApplication1Dlg::OnEnChangeEditMessageOther() {}
void CMFCApplication1Dlg::OnEnChangeEditNotification() {}

void CMFCApplication1Dlg::OnEnChangeRichedit22()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
