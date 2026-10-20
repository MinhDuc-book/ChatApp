#include "pch.h"
#include "framework.h"
#include "MFCApplication1.h"
#include "LoginDlg.h"
#pragma comment(lib, "ws2_32.lib")

class ActionLogin {
public:
	int action = 1;
	CString username;
	CString password;
};


CLoginDlg::CLoginDlg(CWnd* pParent)
    : CDialogEx(IDD_LOGIN_DIALOG, pParent)
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_USERNAME, &CLoginDlg::OnEnChangeEditUsername)
	ON_BN_CLICKED(IDC_BUTTON_SIGNIN, &CLoginDlg::OnBnClickedButtonSignin)
	ON_EN_CHANGE(IDC_EDIT_PASSWORD, &CLoginDlg::OnEnChangeEditPassword)
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CLoginDlg::OnBnClickedButtonLogin)
END_MESSAGE_MAP()

BOOL CLoginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	/*TODO:
	* không cần mở database nữa
	* chỉ cần khởi động thư viện ws_lib để kết nối TCP
	* rồi gửi thông báo qua TCP
	*/
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		AfxMessageBox(_T("Cannot WSAStartUp"));
		return;
	}
	//m_db.OpenDatabase(_T("D:\\AAMduwccc\\project\\C++\\usr.sql"));
	//m_db.CreateTables();

	return TRUE;
}

void CLoginDlg::OnEnChangeEditUsername()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CLoginDlg::OnBnClickedButtonSignin()
{
	EndDialog(IDC_BUTTON_SIGNIN);
}

void CLoginDlg::OnBnClickedButtonLogin()
{
	/*TODO:
	* Nhấn login thì connect() tới IP:port của server
	* Gửi thông class ActionLogin (gửi thông báo muốn đăng nhập)
	* Gửi bằng send()
	*/
	CString username, password;
	GetDlgItemText(IDC_EDIT_USERNAME, username);
	GetDlgItemText(IDC_EDIT_PASSWORD, password);
	//check null
	//if (username.IsEmpty() || password.IsEmpty()) {
	//	AfxMessageBox(_T("Please input username AND password"));
	//	return;
	//}
	//
	//// check existed username, if not -> require sign in
	//if (!m_db.CheckExist(username)) {
	//	AfxMessageBox(_T("Username does not exist.\r\n Create new: Input username and password -> click Sign In"));
	//	return;
	//}
	//
	//// if log in successfully -> return Click login OK
	//int user_id = m_db.CheckLogin(username, password);
	//if (user_id > 0) {
	//	m_savedUser = m_db.GetUserFromID(user_id);
	//	EndDialog(IDC_BUTTON_LOGIN);
	//	return;
	//}
	//else {
	//	AfxMessageBox(_T("Incorrect password"));
	//	return;
	//}
	ActionLogin action;
	action.username = username;
	action.password = password;

	CString payload;
	payload.Format(_T("%d|%s|%s"), action.action,
		(LPCTSTR)action.username,
		(LPCTSTR)action.password);
	if (!SendViaTCP("127.0.0.1", payload)) {
		AfxMessageBox(_T("Cannot Send"));
	}
}

bool CLoginDlg::SendViaTCP(const char* targetIP, const CString& payload)
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) return false;

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons((u_short)9999);
	//addr.sin_addr.s_addr = inet_addr(targetIP);
	inet_pton(AF_INET, targetIP, &addr.sin_addr); // convert IP string to binary form

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

void CLoginDlg::OnEnChangeEditPassword()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
