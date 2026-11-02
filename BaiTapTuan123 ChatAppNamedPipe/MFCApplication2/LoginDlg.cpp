#include "pch.h"
#include "framework.h"
#include "MFCApplication2.h"
#include "LoginDlg.h"

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

	m_db.OpenDatabase(_T("D:\\AAMduwccc\\project\\C++\\usr.sql"));
	m_db.CreateTables();

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
	//CString username, password;
	//GetDlgItemText(IDC_EDIT_USERNAME, username);
	//GetDlgItemText(IDC_EDIT_PASSWORD, password);
	//// check null 
	//if (username.IsEmpty() || password.IsEmpty()) {
	//	AfxMessageBox(_T("Please input username AND password"));
	//	return;
	//}
	//// check existed username, if not -> sign in
	//if (!m_db.CheckExist(username)) {
	//	Info info;
	//	info.username = username;
	//	info.password = password;
	//	m_db.InsertData(info);
	//	AfxMessageBox(_T("Done sign in.\r\n Input again and click Log In"));
	//}
	//else {
	//	AfxMessageBox(_T("Username exist"));
	//}
	EndDialog(IDC_BUTTON_SIGNIN);
}

void CLoginDlg::OnEnChangeEditPassword()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CLoginDlg::OnBnClickedButtonLogin()
{
	CString username, password;
	GetDlgItemText(IDC_EDIT_USERNAME, username);
	GetDlgItemText(IDC_EDIT_PASSWORD, password);
	//check null
	if (username.IsEmpty() || password.IsEmpty()) {
		AfxMessageBox(_T("Please input username AND password"));
		return;
	}

	// check existed username, if not -> require sign in
	if (!m_db.CheckExist(username)) {
		AfxMessageBox(_T("Username does not exist.\r\n Create new: Input username and password -> click Sign In"));
		return;
	}

	// if log in successfully -> return Click login OK
	int user_id = m_db.CheckLogin(username, password);
	if (user_id > 0) {
		m_savedUser = m_db.GetUserFromID(user_id);
		EndDialog(IDC_BUTTON_LOGIN);
		return;
	}
	else {
		AfxMessageBox(_T("Incorrect password"));
		return;
	}
}
