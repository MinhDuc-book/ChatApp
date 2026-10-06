
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

	StartPipeServer();

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
void CMFCApplication2Dlg::StartPipeServer() {
	m_pipeName = _T("\\\\.\\pipe\\ChatApp_2");
	m_stopping = false;
	m_pPipeThread = AfxBeginThread(PipeServerThread,
		this,
		THREAD_PRIORITY_NORMAL,
		0,
		CREATE_SUSPENDED);
	m_pPipeThread->m_bAutoDelete = FALSE; // avoid m_pPipeThread become dangling pointer
	m_pPipeThread->ResumeThread(); // active new thread
}
// Active pipe
/*
* CMFCApplication2Dlg have to use to point to the "dlg" object in UI thread because PipeServerThread is STATIC function
* ConnectNamedPipe will block this thread until have a connection
* After have a connection, check the m_stopping to know whether client close the app => if true close pipe and return function
* read the data from pipe and send POST message to UI thread to update UI
*/
UINT CMFCApplication2Dlg::PipeServerThread(LPVOID p) {
	CMFCApplication2Dlg* self = static_cast<CMFCApplication2Dlg*>(p);
	HWND hwnd = self->m_hWnd;
	while (self->m_stopping == false) {
		// create pipe
		HANDLE hPipe = CreateNamedPipe(
			self->m_pipeName,
			PIPE_ACCESS_INBOUND,                       // read only
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,              // local machine only
			PIPE_UNLIMITED_INSTANCES, 0, 4096, 0, nullptr);

		if (hPipe == INVALID_HANDLE_VALUE) break;

		//STOP HERE
		//block until connection from client
		bool connection = ConnectNamedPipe(hPipe, nullptr) || (GetLastError() == ERROR_PIPE_CONNECTED);

		// check if close app
		if (self->m_stopping) {
			CloseHandle(hPipe);
			break;
		}

		if (connection) {
			std::vector<wchar_t> data;
			wchar_t buf[512];
			DWORD n = 0; // use for count number of byte have read
			// loop for read data from pipe
			while (true) {
				BOOL ok = ReadFile(hPipe, buf, sizeof(buf), &n, nullptr); // STOP HERE
				if (!ok && GetLastError() != ERROR_MORE_DATA) break;
				data.insert(data.end(), buf, buf + n / sizeof(wchar_t)); // insert from buf to buf + (n/2) at the end of data
				if (data.size() > 32 * 1024) break;
				if (ok) {
					CString* msg = new CString(data.data(), data.size());
					bool rs = ::PostMessage(hwnd, WM_PIPE_MSG, 0, (LPARAM)msg); // send noti to UI thread 
					if (!rs) delete msg;
					break;
				}
			}
		}
		DisconnectNamedPipe(hPipe);
		CloseHandle(hPipe);
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

void CMFCApplication2Dlg::StopPipeServer() {
	if (m_pPipeThread == nullptr) return;
	m_stopping = true;

	HANDLE h = CreateFile(m_pipeName, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
	WaitForSingleObject(m_pPipeThread->m_hThread, 2000); // wait close

	delete m_pPipeThread;
	m_pPipeThread = nullptr;
}

void CMFCApplication2Dlg::OnDestroy()
{
	StopPipeServer();
	CDialogEx::OnDestroy();
}


//SENDER
bool CMFCApplication2Dlg::SendViaPipe(const CString& receiver, const CString& payload)
{
	CString name = _T("\\\\.\\pipe\\ChatApp_1");
	for (int i = 0; i < 3; ++i) {
		if (WaitNamedPipe(name, 200)) {
			HANDLE h = CreateFile(name, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
			if (h != INVALID_HANDLE_VALUE) {
				DWORD mode = PIPE_READMODE_MESSAGE;
				SetNamedPipeHandleState(h, &mode, nullptr, nullptr); // set pipe into message mode

				BOOL ok = WriteFile(h, payload.GetString(), payload.GetLength() * sizeof(wchar_t), nullptr, nullptr); 

				CloseHandle(h);
				return ok != FALSE;
			}
		}
		Sleep(50);
	}
	return FALSE;
}

void CMFCApplication2Dlg::OnBnClickedButton1()
{
	CString content;
	CString oldContent;
	CString contentOther;
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

	CString enter = _T("");
	if (oldContent.IsEmpty() == false) {
		enter += _T("\r\n");
	}

	/////////////////////////////////////////////////////
	m_db.InsertMessage(content, m_currentUser, receiverName);
	SendViaPipe(receiverName, sendingMessage);
	

	if (totalLineMe < totalLine)
	{
		for (int i = 0; i < countLineContinous; ++i)
		{
			enter += _T("\r\n");
		}
	}

	content = oldContent + enter + content;
	SetDlgItemText(IDC_EDIT_MESSAGE, _T(""));
	SetDlgItemText(IDC_EDIT_MESSAGE_ME, content);

	oldContentOther = oldContentOther + _T("\r\n");
	SetDlgItemText(IDC_EDIT_MESSAGE_OTHER, oldContentOther);

	int len = pEditMe->GetWindowTextLength();
	// focus to last message
	// caret: position of the cursor look like "|" when typing in a text box, the next character typed will be placed in that position
	// SetSel(start, end) = SetSelection: select a area in Edit Control
	pEditMe->SetSel(len, len, FALSE);
	// send Message to Window or Control
	// EM_... : Scroll to where caret place now
	pEditMe->SendMessage(EM_SCROLLCARET, 0, 0);

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

void CMFCApplication2Dlg::LoadChatToUI(CString friendName) {
	vector<message> messages = m_db.LoadMessage(m_currentUser, friendName);
	CString myContent;
	CString friendContent;
	CString enter = _T("");

	// kiểm tra xem người hiện tại chuẩn bị xử lí có giống với người ban nãy kh
	// nếu giống nhau thì chỉ cần 1 lần /r/n
	// nếu tên mới thì cập nhật nhiều /r/n dựa trên số lượng dòng
		// nếu tên mới là ME thì check bên OTHER, nếu là OTHER thì check ME
	for (int i = 0; i < messages.size(); ++i)
	{
		if (messages[i].sender == m_currentUser)
		{
			// Leave \r\n on OTHER side
			friendContent += _T("\r\n");

			myContent += messages[i].content;
			myContent += _T("\r\n");



		}
		else
		{
			// Leave \r\n on ME side
			myContent += _T("\r\n");

			friendContent += messages[i].content;
			friendContent += _T("\r\n");
		}
	}
	SetDlgItemText(IDC_EDIT_MESSAGE_ME, myContent);
	SetDlgItemText(IDC_EDIT_MESSAGE_OTHER, friendContent);
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
