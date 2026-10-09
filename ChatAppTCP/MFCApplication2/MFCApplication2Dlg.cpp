
// MFCApplication2Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication2.h"
#include "MFCApplication2Dlg.h"
#include "afxdialogex.h"
#include "Resource.h"
#include <string>

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


// CMFCApplication2Dlg dialog



CMFCApplication2Dlg::CMFCApplication2Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION2_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCApplication2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_EN_CHANGE(IDC_EDIT_MESSAGE, &CMFCApplication2Dlg::OnEnChangeEditMessage)
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication2Dlg::OnBnClickedButton1)
	ON_LBN_SELCHANGE(IDC_LIST_FRIEND, &CMFCApplication2Dlg::OnLbnSelchangeListFriend)
	ON_EN_CHANGE(IDC_EDIT_MESSAGE_OTHER, &CMFCApplication2Dlg::OnEnChangeEditMessageOther)
	ON_EN_CHANGE(IDC_EDIT_NOTIFICATION, &CMFCApplication2Dlg::OnEnChangeEditNotification)

	// use to synchro scroll edit box
	ON_CONTROL(EN_VSCROLL, IDC_EDIT_MESSAGE_OTHER, &CMFCApplication2Dlg::OnVscrollMessageOther)
	ON_CONTROL(EN_VSCROLL, IDC_EDIT_MESSAGE_ME, &CMFCApplication2Dlg::OnVscrollMessageMe)

	// receive Win mess from other process
	ON_MESSAGE(WM_PIPE_MSG, &CMFCApplication2Dlg::OnPipeMessage)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CMFCApplication2Dlg message handlers

BOOL CMFCApplication2Dlg::OnInitDialog()
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

	// add to list friend
	m_db.OpenDatabase(_T("D:\\AAMduwccc\\project\\C++\\usr.sql"));
	m_db.CreateTables();
	CListBox* listFriend = (CListBox*)GetDlgItem(IDC_LIST_FRIEND);
	listFriend->ResetContent();
	SetDlgItemText(IDC_EDIT_CURR_USER, m_currentUser);
	vector<string> friends = m_db.AddListFriend(m_currentUser);
	for (int i = 0; i < friends.size(); ++i) {
		CString strFriendName(friends[i].c_str());
		listFriend->AddString(strFriendName);
	}


	// login success -> lock if dont click to friend
	GetDlgItem(IDC_EDIT_MESSAGE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);

	StartServer();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFCApplication2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMFCApplication2Dlg::OnPaint()
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
HCURSOR CMFCApplication2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMFCApplication2Dlg::OnEnChangeEditMessage()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

/*
* Count number of all message in other message box
* Find from last to start if see null (len <= 0)
* ====> Each len > 0: increase countLineContinous
* ====> When see len <= 0: stop and return
*/
int CMFCApplication2Dlg::CountContinousLineOther(int componentID) {
	int countLineContinous = 0;
	CEdit* pEdit = (CEdit*)GetDlgItem(componentID);
	int totalLine = pEdit->GetLineCount();
	for (int i = totalLine - 1; i >= 0; --i) {
		int charIndex = pEdit->LineIndex(i);
		int lineLen = pEdit->LineLength(charIndex);
		if (lineLen > 0) {
			countLineContinous++;
		}
		else {
			if (countLineContinous > 0) {
				break;
			}
		}
	}
	return countLineContinous;
}

vector<CString> CMFCApplication2Dlg::ParseStringReceived(CString mess) {
	CString tmp;
	vector<CString> rs;
	for (int i = 0; i < mess.GetLength(); ++i) {
		if (mess.GetAt(i) == _T(':')) {
			rs.push_back(tmp);
			tmp = "";
		}
		else {
			tmp = tmp + mess.GetAt(i);
		}

	}
	return rs;
}
// Create thread and init value for pipe
/*
* Set autoDelete = false because if it TRUE, MFC will auto delete it after close pipe(close app) but not set pPipeThread = nullptr
*/
void CMFCApplication2Dlg::StartServer() {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		AfxMessageBox(_T("Cannot WSAStartUp"));
		return;
	}
	m_receiveSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_receiveSocket == INVALID_SOCKET) {
		AfxMessageBox(_T("Cannot init socket"));
		return;
	}

	// allow to reuse socket when turn on very much in short time
	int option = 1;
	setsockopt(m_receiveSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&option, sizeof(option));

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(m_receivePort);

	if (bind(m_receiveSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		AfxMessageBox(_T("Bind failed"));
		closesocket(m_receiveSocket);
		m_receiveSocket = INVALID_SOCKET;
	}

	// allow to open connecting queue
	listen(m_receiveSocket, SOMAXCONN);
	m_stopping = false;
	m_pServerThread = AfxBeginThread(TCPServerThread, // function will run in new thread
									this, // what data send to new thread (current addr of object CMFCApplication1Dlg) ->  1st parameter of PipeServerThread
									THREAD_PRIORITY_NORMAL, // priority
									0,  // stack size (default size)
									CREATE_SUSPENDED); //status of new thread (create but not run)
	m_pServerThread->m_bAutoDelete = FALSE;
	m_pServerThread->ResumeThread();
}
// Active pipe
/*
* CMFCApplication2Dlg have to use to point to the "dlg" object in UI thread because PipeServerThread is STATIC function
* ConnectNamedPipe will block this thread until have a connection
* After have a connection, check the m_stopping to know whether client close the app => if true close pipe and return function
* read the data from pipe and send POST message to UI thread to update UI
*/
UINT CMFCApplication2Dlg::TCPServerThread(LPVOID p) {
	CMFCApplication2Dlg* self = static_cast<CMFCApplication2Dlg*>(p);
	HWND hwnd = self->m_hWnd;

	while (!self->m_stopping)
	{
		sockaddr_in clientAddr = {};
		int addrLen = sizeof(clientAddr);
		SOCKET clientSocket = accept(self->m_receiveSocket, (sockaddr*)&clientAddr, &addrLen);

		if (clientSocket == INVALID_SOCKET) break;

		std::wstring data;
		wchar_t buf[1024];
		int check;
		while (true) {
			check = recv(clientSocket, (char*)buf, sizeof(buf) - 2, 0);
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

// lParam save the address of received data
LRESULT CMFCApplication2Dlg::OnPipeMessage(WPARAM wParam, LPARAM lParam) {
	CString* msg = reinterpret_cast<CString*>(lParam);
	HandleIncoming(*msg);
	delete msg;
	return 1;
}

void CMFCApplication2Dlg::HandleIncoming(const CString& message)
{
	if (message.IsEmpty() == true) return;

	vector<CString> str = ParseStringReceived(message);
	if (str.size() < 3) return;

	CListBox* listFriend = (CListBox*)GetDlgItem(IDC_LIST_FRIEND);
	int saveIndex = listFriend->GetCurSel();
	int index = listFriend->FindStringExact(-1, str[0]);
	if (index != LB_ERR) {
		CString name;
		listFriend->GetText(index, name);
		if (name.Left(3) != _T("-->") && str[2] == m_currentUser) {
			name = _T("-->") + name;
			listFriend->DeleteString(index);
			listFriend->InsertString(index, name);
			if (saveIndex != LB_ERR)
			{
				listFriend->SetCurSel(saveIndex);
			}
		}
	}
	if (str[2] == m_currentUser) {
		SetDlgItemText(IDC_EDIT_NOTIFICATION, str[0] + _T(": ") + str[1]);
	}
	// reload chat if corrected sender
	if (index != LB_ERR && index == saveIndex)
	{
		CString friendName;
		listFriend->GetText(index, friendName);
		friendName = friendName.Right(friendName.GetLength() - 3);
		listFriend->DeleteString(index);
		listFriend->InsertString(index, friendName);
		listFriend->SetCurSel(index);
		LoadChatToUI(str[0]);

		// focus on last line
		CEdit* pEditMe = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_ME);
		int len = pEditMe->GetWindowTextLength();
		pEditMe->SetSel(len, len, FALSE);
		pEditMe->SendMessage(EM_SCROLLCARET, 0, 0);
	}

}

void CMFCApplication2Dlg::StopServer() {
	if (!m_pServerThread) return;
	m_stopping = true;

	if (m_receiveSocket != INVALID_SOCKET) {
		closesocket(m_receiveSocket);
		m_receiveSocket = INVALID_SOCKET;
	}

	WaitForSingleObject(m_pServerThread->m_hThread, 2000);
	delete m_pServerThread;
	m_pServerThread = nullptr;

	WSACleanup();
}

void CMFCApplication2Dlg::OnDestroy()
{
	StopServer();
	CDialogEx::OnDestroy();
}


//SENDER
bool CMFCApplication2Dlg::SendViaTCP(const char* targetIP, const CString& payload)
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) return false;

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons((u_short)12226);
	//addr.sin_addr.s_addr = inet_addr(targetIP);
	inet_pton(AF_INET, targetIP, &addr.sin_addr);

	if (connect(s, (sockaddr*)&addr, sizeof(addr)) != SOCKET_ERROR) {
		int bytes = (payload.GetLength() + 1) * sizeof(wchar_t);
		send(s, reinterpret_cast<const char*>(payload.GetString()), bytes, 0);
	}
	else {
		AfxMessageBox(_T("Cannot connect"));
		return false;
	}
	closesocket(s);
	return true;
}

void CMFCApplication2Dlg::OnBnClickedButton1()
{
	CString content;
	CString oldContent;
	CString oldContentOther;

	int countLineContinous = CountContinousLineOther(IDC_EDIT_MESSAGE_OTHER);

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_OTHER);
	int totalLine = pEdit->GetLineCount();

	CEdit* pEditMe = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_ME);
	int totalLineMe = pEditMe->GetLineCount();


	GetDlgItemText(IDC_EDIT_MESSAGE_ME, oldContent);
	GetDlgItemText(IDC_EDIT_MESSAGE, content);

	GetDlgItemText(IDC_EDIT_MESSAGE_OTHER, oldContentOther);

	// sender:content:receiver
	CListBox* pListBox = (CListBox*)GetDlgItem(IDC_LIST_FRIEND);
	int nIndex = pListBox->GetCurSel();
	CString receiverName;
	pListBox->GetText(nIndex, receiverName);
	CString sendingMessage = m_currentUser + ":" + content + ":" + receiverName + ":";

	/////////////////////////////////////////////////////
	m_db.InsertMessage(content, m_currentUser, receiverName);

	//::Use directly function from API (OS)
	SendViaTCP("127.0.0.1", sendingMessage);

	SetDlgItemText(IDC_EDIT_MESSAGE, _T(""));

	LoadChatToUI(receiverName);

}

void CMFCApplication2Dlg::OnLbnSelchangeListFriend()
{
	CListBox* listFriend = (CListBox*)GetDlgItem(IDC_LIST_FRIEND);
	int sel = listFriend->GetCurSel();

	if (sel != LB_ERR) // LB_ERR = no selected
	{
		// Choose 1 -> enable
		GetDlgItem(IDC_EDIT_MESSAGE)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);

		CString friendName;
		listFriend->GetText(sel, friendName);

		//clear before to avoid mistake
		SetDlgItemText(IDC_EDIT_MESSAGE_ME, _T(""));
		SetDlgItemText(IDC_EDIT_MESSAGE_OTHER, _T(""));
		SetDlgItemText(IDC_EDIT_MESSAGE, _T(""));


		if (friendName.Left(3) == _T("-->")) {
			friendName = friendName.Right(friendName.GetLength() - 3);
			listFriend->DeleteString(sel);
			listFriend->InsertString(sel, friendName);
			listFriend->SetCurSel(sel);

			
		}
		LoadChatToUI(friendName);
	}
	// focus on last line (new message)
	CEdit* pEditMe = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_ME);
	int len = pEditMe->GetWindowTextLength();
	pEditMe->SetSel(len, len, FALSE);
}

CString CMFCApplication2Dlg::MultipleString(CString str, int times) {
	CString rs;
	for (int i = 0; i < times; ++i) rs += str;
	return rs;
}

// Đếm số dòng mà `text` chiếm trong ô edit `editID` (có tính tự xuống dòng)
int CMFCApplication2Dlg::CountWrapLines(const CString& text)
{
	//CEdit* pEdit = (CEdit*)GetDlgItem(editID);
	//CDC* pDC = pEdit->GetDC();
	//CFont* pOldFont = pDC->SelectObject(pEdit->GetFont());
	//
	//CRect rc;
	//pEdit->GetRect(&rc);                         // vùng hiển thị chữ thực sự
	//int width = rc.Width();
	//if (pEdit->GetStyle() & WS_VSCROLL)
	//	width -= GetSystemMetrics(SM_CXVSCROLL); // trừ chỗ thanh cuộn
	//
	//CRect calc(0, 0, width, 0);
	//pDC->DrawText(text, &calc,DT_CALCRECT | DT_WORDBREAK | DT_EDITCONTROL | DT_NOPREFIX);
	//
	//TEXTMETRIC tm;
	//pDC->GetTextMetrics(&tm);
	//
	//pDC->SelectObject(pOldFont);
	//pEdit->ReleaseDC(pDC);
	//
	//int lines = (tm.tmHeight > 0) ? calc.Height() / tm.tmHeight : 1;
	//return max(1, lines);

	int lines = (text.GetLength() / 28) + 1; // chia làm tròn lên
	return max(1, lines);
}

void CMFCApplication2Dlg::LoadChatToUI(CString friendName)
{
	//vector<message> messages = m_db.LoadMessage(m_currentUser, friendName);
	//CString myContent, friendContent;
	//const CString NL = _T("\r\n");
	//
	//for (size_t i = 0; i < messages.size(); ++i)
	//{
	//	bool isMe = (messages[i].sender == m_currentUser);
	//
	//	// đo ở đúng ô sẽ hiển thị tin nhắn này
	//	int L = CountWrapLines(isMe ? IDC_EDIT_MESSAGE_ME : IDC_EDIT_MESSAGE_OTHER,messages[i].content);
	//
	//	CString prefix = (i > 0) ? NL : _T("");     // ngắt dòng giữa các tin
	//	CString blank = MultipleString(NL, L - 1); // L dòng trống cho bên còn lại
	//
	//	if (isMe) {
	//		myContent += prefix + messages[i].content;
	//		friendContent += prefix + blank;
	//	}
	//	else {
	//		friendContent += prefix + messages[i].content;
	//		myContent += prefix + blank;
	//	}
	//}
	//
	//SetDlgItemText(IDC_EDIT_MESSAGE_ME, myContent);
	//SetDlgItemText(IDC_EDIT_MESSAGE_OTHER, friendContent);
	//
	//// cuộn xuống tin mới nhất (hai ô tự đồng bộ nhờ OnVscroll...)
	//CEdit* pMe = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_ME);
	//int len = pMe->GetWindowTextLength();
	//pMe->SetSel(len, len, FALSE);
	//pMe->SendMessage(EM_SCROLLCARET, 0, 0);
	//CEdit* pOther = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_OTHER);
	//len = pOther->GetWindowTextLength();
	//pOther->SetSel(len, len, FALSE);
	//pOther->SendMessage(EM_SCROLLCARET, 0, 0);

	vector<message> messages = m_db.LoadMessage(m_currentUser, friendName);
	CString myContent;
	CString friendContent;

	for (int i = 0; i < messages.size(); ++i)
	{
		// tin nhắn này chiếm bao nhiêu dòng trong ô edit
		int lines = CountWrapLines(messages[i].content);

		if (messages[i].sender == m_currentUser)
		{
			// Leave \r\n on OTHER side: chừa đúng `lines` dòng
			friendContent += MultipleString(_T("\r\n"), lines);

			myContent += messages[i].content;
			myContent += _T("\r\n");
		}
		else
		{
			// Leave \r\n on ME side: chừa đúng `lines` dòng
			myContent += MultipleString(_T("\r\n"), lines);

			friendContent += messages[i].content;
			friendContent += _T("\r\n");
		}
	}
	SetDlgItemText(IDC_EDIT_MESSAGE_ME, myContent);
	SetDlgItemText(IDC_EDIT_MESSAGE_OTHER, friendContent);

	// cuộn xuống tin mới nhất
	CEdit* pMe = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_ME);
	int len = pMe->GetWindowTextLength();
	pMe->SetSel(len, len, FALSE);
	pMe->SendMessage(EM_SCROLLCARET, 0, 0);


}

void CMFCApplication2Dlg::OnEnChangeEditMessageOther()
{


}

//void CMFCApplication2Dlg::OnEnterMessageOther()
//{
//	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_OTHER);
//
//	// Get line which have caret (before go down)
//	int line = pEdit->LineFromChar(-1); //-1: current caret
//	int charIndex = pEdit->LineIndex(line);
//	int lineLen = pEdit->LineLength(charIndex);
//
//	if (lineLen <= 0) return; // null -> no save
//
//	CString lineText;
//	int bufSize = lineLen + 1;
//	TCHAR* buf = lineText.GetBuffer(bufSize);
//	*((WORD*)buf) = (WORD)bufSize; //CEdit::GetLine need 2 first byte is size
//	int copied = pEdit->GetLine(line, buf, bufSize);
//	lineText.ReleaseBuffer(copied);
//
//	if (lineText.IsEmpty()) return; // null -> no save
//
//	// selected friend is sender
//	CListBox* pListBox = (CListBox*)GetDlgItem(IDC_LIST_FRIEND);
//	int nIndex = pListBox->GetCurSel();
//
//	CString friendName;
//	pListBox->GetText(nIndex, friendName);
//
//	////////////////////////////////////////////////////////
//	//m_db.InsertMessage(lineText, friendName, m_currentUser);
//}

BOOL CMFCApplication2Dlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			if (GetFocus() == GetDlgItem(IDC_EDIT_MESSAGE))
			{
				OnBnClickedButton1();
				return TRUE; // need to avoid add \r\n into end of "content"
			}
			else
			{
				return TRUE;
			}
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CMFCApplication2Dlg::OnVscrollMessageOther()
{
	if (m_syncScrolling)
		return;

	m_syncScrolling = true;

	CEdit* pOther = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_OTHER);
	CEdit* pMe = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_ME);

	int firstLine = pOther->GetFirstVisibleLine();

	pMe->LineScroll(
		firstLine - pMe->GetFirstVisibleLine()
	);

	m_syncScrolling = false;
}

void CMFCApplication2Dlg::OnVscrollMessageMe()
{
	if (m_syncScrolling)
		return;

	m_syncScrolling = true;

	CEdit* pMe = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_ME);
	CEdit* pOther = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE_OTHER);

	int firstLine = pMe->GetFirstVisibleLine();

	pOther->LineScroll(
		firstLine - pOther->GetFirstVisibleLine()
	);

	m_syncScrolling = false;
}
void CMFCApplication2Dlg::OnEnChangeEditNotification()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
