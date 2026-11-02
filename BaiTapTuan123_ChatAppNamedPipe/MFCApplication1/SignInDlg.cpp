#include "pch.h"
#include "framework.h"
#include "MFCApplication1.h"
#include "SignInDlg.h"

CSigninDlg::CSigninDlg(CWnd* pParent)
	: CDialogEx(IDD_SIGNIN_DIALOG, pParent)
{
}

void CSigninDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSigninDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_USERNAME_SIGN, &CSigninDlg::OnEnChangeEditUsernameSign)
	ON_EN_CHANGE(IDC_EDIT_PASSWORD_SIGN, &CSigninDlg::OnEnChangeEditPasswordSign)
	ON_BN_CLICKED(IDOK, &CSigninDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT_PASSWORD_SIGN_VERIFY, &CSigninDlg::OnEnChangeEditPasswordSignVerify)
	ON_BN_CLICKED(IDCANCEL, &CSigninDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL CSigninDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_db.OpenDatabase(_T("D:\\AAMduwccc\\project\\C++\\usr.sql"));
	m_db.CreateTables();

	return TRUE;
}
void CSigninDlg::OnEnChangeEditUsernameSign()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CSigninDlg::OnEnChangeEditPasswordSign()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CSigninDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString username;
	CString password;
	CString verifypassword;
	GetDlgItemText(IDC_EDIT_USERNAME_SIGN, username);
	GetDlgItemText(IDC_EDIT_PASSWORD_SIGN, password);
	GetDlgItemText(IDC_EDIT_PASSWORD_SIGN_VERIFY, verifypassword);
	if (username.IsEmpty() || password.IsEmpty() || verifypassword.IsEmpty()) {
		AfxMessageBox(_T("Please input all"));
		return;
	}
	if (password != verifypassword) {
		AfxMessageBox(_T("Password != Verify Password"));
		return;
	}
	else {
		//check exist
		if (m_db.CheckExist(username) == true) {
			AfxMessageBox(_T("Username existed"));
			return;
		}
		else {
			Info info;
			info.username = username;
			info.password = password;
			m_db.InsertData(info);
			AfxMessageBox(_T("Done registry"));
			
		}
	}
	CDialogEx::OnOK();
}

void CSigninDlg::OnEnChangeEditPasswordSignVerify()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CSigninDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}
