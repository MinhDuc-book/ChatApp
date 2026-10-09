
// MFCApplication1Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
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

	// use to synchro scroll edit box
	ON_CONTROL(EN_VSCROLL, IDC_EDIT_MESSAGE_OTHER, &CMFCApplication1Dlg::OnVscrollMessageOther)
	ON_CONTROL(EN_VSCROLL, IDC_EDIT_MESSAGE_ME, &CMFCApplication1Dlg::OnVscrollMessageMe)

	// receive Win mess from other process
	ON_MESSAGE(WM_PIPE_MSG, &CMFCApplication1Dlg::OnPipeMessage)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CMFCApplication1Dlg message handlers

BOOL CMFCApplication1Dlg::OnInitDialog()
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

	// start Named pipe
	StartPipeServer();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFCApplication1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMFCApplication1Dlg::OnPaint()
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
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMFCApplication1Dlg::OnEnChangeEditMessage()
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
int CMFCApplication1Dlg::CountContinousLineOther(int componentID) {
	int countLineContinous = 0;
	CEdit* pEdit = (CEdit*)GetDlgItem(componentID);
	int totalLine = pEdit->GetLineCount();
	for (int i = totalLine-1; i >= 0; --i) {
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

vector<CString> CMFCApplication1Dlg::ParseStringReceived(CString mess) {
	CString tmp;
	vector<CString> rs;
	for (int i = 0; i < mess.GetLength(); ++i) {
		if ( mess.GetAt(i) == _T(':')) {
			rs.push_back(tmp);
			tmp = "";
		}
		else {
			tmp = tmp + mess.GetAt(i);
		}
		
	}
	return rs;
}

void CMFCApplication1Dlg::StartPipeServer()
{
	m_pipeName = _T("\\\\.\\pipe\\ChatApp_1");
	m_stopping = false;
	m_pPipeThread = AfxBeginThread(PipeServerThread, // function will run in new thread
									this, // what data send to new thread (current addr of object CMFCApplication1Dlg) ->  1st parameter of PipeServerThread
									THREAD_PRIORITY_NORMAL, // priority
									0,  // stack size (default size)
									CREATE_SUSPENDED); //status of new thread (create but not run)
	m_pPipeThread->m_bAutoDelete = FALSE;  
	m_pPipeThread->ResumeThread();
}

UINT CMFCApplication1Dlg::PipeServerThread(LPVOID p)
{
	CMFCApplication1Dlg* self = static_cast<CMFCApplication1Dlg*>(p);
	HWND hwnd = self->m_hWnd;

	while (!self->m_stopping)
	{
		HANDLE hPipe = CreateNamedPipe(
			self->m_pipeName,
			PIPE_ACCESS_INBOUND,                       // read only
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT
			| PIPE_REJECT_REMOTE_CLIENTS,              // local machine only
			PIPE_UNLIMITED_INSTANCES, 0, 4096, 0, nullptr);
		if (hPipe == INVALID_HANDLE_VALUE) break;

		// block until have client
		BOOL connected = ConnectNamedPipe(hPipe, nullptr) || (GetLastError() == ERROR_PIPE_CONNECTED);

		if (self->m_stopping) { CloseHandle(hPipe); break; }

		if (connected)
		{
			std::vector<wchar_t> data;
			wchar_t buf[512];
			DWORD n = 0;
			while(true)
			{
				BOOL ok = ReadFile(hPipe, buf, sizeof(buf), &n, nullptr);
				if (!ok && GetLastError() != ERROR_MORE_DATA) break;   // client close -> error
				data.insert(data.end(), buf, buf + n / sizeof(wchar_t));
				if (data.size() > 32 * 1024) break;                    // block huge message
				if (ok)                                               
				{
					CString* msg = new CString(data.data(), (int)data.size());
					// send message to UI thread
					BOOL result = ::PostMessage(hwnd, WM_PIPE_MSG, 0, (LPARAM)msg);
					if (!result) delete msg;
					break;
				}
			}
		}
		DisconnectNamedPipe(hPipe);
		CloseHandle(hPipe);
	}
	return 0;
}

void CMFCApplication1Dlg::StopPipeServer()
{
	if (!m_pPipeThread) return;
	m_stopping = true;

	// đánh thức ConnectNamedPipe đang block bằng 1 client giả
	HANDLE h = CreateFile(m_pipeName, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (h != INVALID_HANDLE_VALUE) CloseHandle(h);

	WaitForSingleObject(m_pPipeThread->m_hThread, 2000);
	delete m_pPipeThread;
	m_pPipeThread = nullptr;
}

bool CMFCApplication1Dlg::SendViaPipe(const CString& receiver, const CString& payload)
{
	CString name = _T("\\\\.\\pipe\\ChatApp_2");

	//wireless mechanism (CDMA/CSMA)
	for (int attempt = 0; attempt < 3; ++attempt)
	{
		if (WaitNamedPipe(name, 200))     
		{
			HANDLE h = CreateFile(name, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
			if (h != INVALID_HANDLE_VALUE)
			{
				DWORD mode = PIPE_READMODE_MESSAGE;
				SetNamedPipeHandleState(h, &mode, nullptr, nullptr);

				BOOL ok = WriteFile(h, payload.GetString(), payload.GetLength() * sizeof(wchar_t), nullptr, nullptr);  // no send \0

				CloseHandle(h);
				return ok != FALSE;
			}
		}
		Sleep(50);  
	}
	return false; 
}

void CMFCApplication1Dlg::OnDestroy()
{
	StopPipeServer();
	CDialogEx::OnDestroy();
}

// lParam save the address of received data
LRESULT CMFCApplication1Dlg::OnPipeMessage(WPARAM wParam, LPARAM lParam) {
	CString* msg = reinterpret_cast<CString*>(lParam);
	HandleIncoming(*msg);
	delete msg;
	return 1;
}

void CMFCApplication1Dlg::HandleIncoming(const CString& message)
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

void CMFCApplication1Dlg::OnBnClickedButton1()
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
	SendViaPipe(receiverName, sendingMessage);

	SetDlgItemText(IDC_EDIT_MESSAGE, _T(""));

	LoadChatToUI(receiverName);

}

void CMFCApplication1Dlg::OnLbnSelchangeListFriend()
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

CString CMFCApplication1Dlg::MultipleString(CString str, int times) {
	CString rs;
	for (int i = 0; i < times; ++i) rs += str;
	return rs;
}

// Đếm số dòng mà `text` chiếm trong ô edit `editID` (có tính tự xuống dòng)
int CMFCApplication1Dlg::CountWrapLines(const CString& text)
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

void CMFCApplication1Dlg::LoadChatToUI(CString friendName)
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

void CMFCApplication1Dlg::OnEnChangeEditMessageOther()
{

	
}

//void CMFCApplication1Dlg::OnEnterMessageOther()
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
//	//////////////////////////////////////////////////////
//	//m_db.InsertMessage(lineText, friendName, m_currentUser);
//}

BOOL CMFCApplication1Dlg::PreTranslateMessage(MSG* pMsg)
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

void CMFCApplication1Dlg::OnVscrollMessageOther()
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

void CMFCApplication1Dlg::OnVscrollMessageMe()
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
void CMFCApplication1Dlg::OnEnChangeEditNotification()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
