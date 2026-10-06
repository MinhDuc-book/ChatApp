
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
	ON_MESSAGE(WM_COPYDATA, &CMFCApplication1Dlg::OnCopyData)
	
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


// lParam save the address of received data
LRESULT CMFCApplication1Dlg::OnCopyData(WPARAM wParam, LPARAM lParam) {
	if (lParam == 0) return 0;
	// convert LPARAM -> COPYDATASTRUCT
	COPYDATASTRUCT* cds = reinterpret_cast<COPYDATASTRUCT*>(lParam);
	if (cds->cbData == 0 || cds->dwData != 1) return 0;
	CString message(reinterpret_cast<LPCWSTR>(cds->lpData));
	if (message.IsEmpty()) return 0;

	vector<CString> str = ParseStringReceived(message);
	if (str.size() < 3) return 0;

	CListBox* listFriend = (CListBox*)GetDlgItem(IDC_LIST_FRIEND);
	int saveIndex = listFriend->GetCurSel();
	int index = listFriend->FindStringExact(-1, str[0]);
	if (index != LB_ERR) {
		CString name;
		listFriend->GetText(index, name);
		if (name.Left(3) != _T("-->") && str[2] == m_currentUser) {
			name = _T("-->") + name ;
			listFriend->DeleteString(index);
			listFriend->InsertString(index, name);
			if (saveIndex != LB_ERR)
			{
				listFriend->SetCurSel(saveIndex);
			}
		}
	}

	if (str[2] == m_currentUser) {
		SetDlgItemText(IDC_EDIT_NOTIFICATION, str[0] + ": " + str[1]);
	}
	

	//LoadMessage if open corect chatting now
	if (index == saveIndex)
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
	return 1;
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

	CString enter = _T("");
	if (oldContent.IsEmpty() == false) {
		enter += _T("\r\n");
	}

	/////////////////////////////////////////////////////
	m_db.InsertMessage(content, m_currentUser, receiverName);

	// ::Use directly function from API (OS)
	HWND hWndB = ::FindWindow(nullptr, L"MFCApplication2");
	if (hWndB == nullptr) {
		AfxMessageBox(_T("Cannot found MFCApplication2"));
		return;
	}

	// data structure have to use when send data from this process to other with WM_COPYDATA
	COPYDATASTRUCT cds;
	cds.dwData = 1;// int // type of data this sending time (can config by dev)
	cds.cbData = (sendingMessage.GetLength() + 1) * sizeof(wchar_t); // size_t
	cds.lpData = (void*)sendingMessage.GetString(); // void

	// send
	::SendMessage(
		hWndB,            // what/who will receive
		WM_COPYDATA,       // OS see that and know this is inter process communication action
		(WPARAM)this->m_hWnd, // HANDLE of sender window
		(LPARAM)&cds // pointer to data packet
	);

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
	//int gap = totalLine - totalLineMe;
	//if (gap < 0) {
	//	if (oldContent.IsEmpty() == false) {
	//		enter += _T("\r\n");
	//	}
	//	else {
	//		enter = _T("\r\n");
	//	}
	//	
	//}
	//else if (gap > 0) {
	//	for (int i = 0; i < gap; ++i) {
	//		enter += _T("\r\n");
	//	}
	//}

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

void CMFCApplication1Dlg::LoadChatToUI(CString friendName) {
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
