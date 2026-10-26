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

	ON_CONTROL(EN_VSCROLL, IDC_EDIT_MESSAGE_OTHER, &CMFCApplication1Dlg::OnVscrollMessageOther)
	ON_CONTROL(EN_VSCROLL, IDC_EDIT_MESSAGE_ME, &CMFCApplication1Dlg::OnVscrollMessageMe)

	ON_MESSAGE(WM_TCP_MSG, &CMFCApplication1Dlg::OnPipeMessage)
	ON_WM_DESTROY()
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

	// Đăng ký tài khoản hiện tại vào Server (để server lưu vào ClientManager)
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

	// Khóa ô nhập tin nhắn cho tới khi click chọn bạn bè
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

			//Thông báo đẩy từ Server có tin nhắn mới ("NEW_MSG|sender")
			if (msg.Left(8) == _T("NEW_MSG|"))
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

	// Chờ luồng đọc nền TCPServerThread nhận phản hồi về (tối đa 5 giây)
	DWORD dwWait = WaitForSingleObject(m_hResponseEvent, 5000);
	if (dwWait == WAIT_OBJECT_0) {
		response = m_lastResponse;
		response.TrimRight(_T("\r\n"));
		//AfxMessageBox(response);
		return true;
	}

	return false;
}

// xử lí nhận thông báo NEW_MSG
LRESULT CMFCApplication1Dlg::OnPipeMessage(WPARAM wParam, LPARAM lParam) {
	CString* msg = reinterpret_cast<CString*>(lParam);
	HandleIncoming(*msg);
	delete msg;
	return 1;
}

void CMFCApplication1Dlg::HandleIncoming(const CString& message)
{

	if (message.IsEmpty()) return;
	//AfxMessageBox(_T("ĐÃ NHẬN ĐƯỢC: ") + message);
	CString msg = message;
	msg.TrimRight(_T("\r\n"));

	int curPos = 0;
	CString prefix = msg.Tokenize(_T("|"), curPos);
	CString sender = msg.Mid(8);

	if (prefix != _T("NEW_MSG") || sender.IsEmpty()) return;

	CListBox* pListBox = (CListBox*)GetDlgItem(IDC_LIST_FRIEND);
	if (!pListBox) return;

	int curSel = pListBox->GetCurSel();
	CString currentSelectedFriend = _T("");
	if (curSel != LB_ERR) {
		pListBox->GetText(curSel, currentSelectedFriend);
		currentSelectedFriend.TrimRight(_T("\r\n"));
		if (currentSelectedFriend.Left(3) == _T("-->")) {
			currentSelectedFriend = currentSelectedFriend.Mid(3);
		}
	}

	SetDlgItemText(IDC_EDIT_NOTIFICATION,_T("New message from: ") + sender);

	if (curSel != LB_ERR && currentSelectedFriend == sender)
	{
		LoadChatToUI(sender);
	}
	else
	{
		int index = pListBox->FindStringExact(-1, sender);
		if (index == LB_ERR) {
			index = pListBox->FindStringExact(-1, _T("-->") + sender);
		}

		if (index != LB_ERR) {
			pListBox->DeleteString(index);
		}

		// Đẩy lên vị trí 0 (đầu danh sách)
		pListBox->InsertString(0, _T("-->") + sender);

		// Giữ nguyên con trỏ người bạn đang chat dở
		if (!currentSelectedFriend.IsEmpty()) {
			int newSel = pListBox->FindStringExact(-1, currentSelectedFriend);
			if (newSel == LB_ERR) {
				newSel = pListBox->FindStringExact(-1, _T("-->") + currentSelectedFriend);
			}
			if (newSel != LB_ERR) {
				pListBox->SetCurSel(newSel);
			}
		}
	}
}

// 6. GỬI TIN NHẮN KHI BẤM NÚT SEND
void CMFCApplication1Dlg::OnBnClickedButton1()
{
	CString content;
	GetDlgItemText(IDC_EDIT_MESSAGE, content);
	content.Trim();

	CListBox* pListBox = (CListBox*)GetDlgItem(IDC_LIST_FRIEND);
	int nIndex = pListBox->GetCurSel();
	if (nIndex == LB_ERR) {
		AfxMessageBox(_T("Vui lòng chọn bạn bè để chat!"));
		return;
	}
	if (content.IsEmpty()) return;

	CString receiverName;
	pListBox->GetText(nIndex, receiverName);
	receiverName.TrimRight(_T("\r\n"));
	//if (receiverName.Left(3) == _T("-->")) {
	//	receiverName = receiverName.Mid(3);
	//}

	CString sendingMessage = _T("3|") + m_currentUser + _T("|") + receiverName + _T("|") + content;
	CString receive = _T("");

	if (!SendViaTCP("127.0.0.1", sendingMessage, receive)) {
		AfxMessageBox(_T("Không thể gửi tin nhắn!"));
	}
	else {
		if (receive.Left(2) == _T("OK")) {
			SetDlgItemText(IDC_EDIT_MESSAGE, _T(""));
			LoadChatToUI(receiverName);
		}
		else {
			AfxMessageBox(_T("Gửi thất bại:\r\n") + receive);
		}
	}
}

// 7. CHỌN BẠN TRONG LISTBOX
void CMFCApplication1Dlg::OnLbnSelchangeListFriend()
{
	CListBox* listFriend = (CListBox*)GetDlgItem(IDC_LIST_FRIEND);
	int sel = listFriend->GetCurSel();

	if (sel != LB_ERR)
	{
		GetDlgItem(IDC_EDIT_MESSAGE)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);

		CString friendName;
		listFriend->GetText(sel, friendName);
		friendName.TrimRight(_T("\r\n"));

		SetDlgItemText(IDC_EDIT_MESSAGE_ME, _T(""));
		SetDlgItemText(IDC_EDIT_MESSAGE_OTHER, _T(""));
		SetDlgItemText(IDC_EDIT_MESSAGE, _T(""));

		// Nếu tên có tiền tố "-->" thì gỡ bỏ khi người dùng click vào
		if (friendName.Left(3) == _T("-->")) {
			friendName = friendName.Mid(3);
			listFriend->DeleteString(sel);
			listFriend->InsertString(sel, friendName);
			listFriend->SetCurSel(sel);
		}

		LoadChatToUI(friendName);
	}

	CEdit* pEditMe = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_OTHER);
	int len = pEditMe->GetWindowTextLength();
	pEditMe->SetSel(len, len, FALSE);
}

// 8. TẢI TOÀN BỘ NỘI DUNG CHAT
void CMFCApplication1Dlg::LoadChatToUI(CString friendName)
{
	if (friendName.IsEmpty()) return;

	CString payload = _T("4|") + m_currentUser + _T("|") + friendName;
	CString response = _T("");
	if (!SendViaTCP("127.0.0.1", payload, response) || response.Left(2) != _T("OK")) {
		return;
	}

	int curPos = 0;
	response.Tokenize(_T("|"), curPos); // Bỏ qua "OK"
	CString strCount = response.Tokenize(_T("|"), curPos); // Bỏ qua số lượng
	CString myContent = _T("");
	CString friendContent = _T("");

	while (curPos != -1)
	{
		CString sender = response.Tokenize(_T("|"), curPos);
		if (curPos == -1) break;
		CString msgContent = response.Tokenize(_T("|"), curPos);

		int lines = CountWrapLines(msgContent);
		if (sender == m_currentUser)
		{
			friendContent += MultipleString(_T("\r\n"), lines);
			myContent += msgContent + _T("\r\n");
		}
		else
		{
			myContent += MultipleString(_T("\r\n"), lines);
			friendContent += msgContent + _T("\r\n");
		}
	}

	SetDlgItemText(IDC_EDIT_MESSAGE_ME, myContent);
	SetDlgItemText(IDC_EDIT_MESSAGE_OTHER, friendContent);

	CEdit* pMe = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_ME);
	int len = pMe->GetWindowTextLength();
	pMe->SetSel(len, len, FALSE);
	pMe->SendMessage(EM_SCROLLCARET, 0, 0);

	CEdit* pOther = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_OTHER);
	len = pOther->GetWindowTextLength();
	pOther->SetSel(len, len, FALSE);
	pOther->SendMessage(EM_SCROLLCARET, 0, 0);
}

CString CMFCApplication1Dlg::MultipleString(CString str, int times) {
	CString rs;
	for (int i = 0; i < times; ++i) rs += str;
	return rs;
}

int CMFCApplication1Dlg::CountWrapLines(const CString& text)
{
	int lines = (text.GetLength() / 28) + 1;
	return max(1, lines);
}

int CMFCApplication1Dlg::CountContinousLineOther(int componentID) {
	int countLineContinous = 0;
	CEdit* pEdit = (CEdit*)GetDlgItem(componentID);
	int totalLine = pEdit->GetLineCount();
	for (int i = totalLine - 1; i >= 0; --i) {
		int charIndex = pEdit->LineIndex(i);
		int lineLen = pEdit->LineLength(charIndex);
		if (lineLen > 0) countLineContinous++;
		else if (countLineContinous > 0) break;
	}
	return countLineContinous;
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