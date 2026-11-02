#include "pch.h"
#include "framework.h"
#include "MFCApplication1.h"
#include "SignInDlg.h"
#pragma comment(lib, "ws2_32.lib")


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

	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		AfxMessageBox(_T("Cannot WSAStartUp"));
		return FALSE;
	}

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

bool CSigninDlg::SendViaTCP(const char* targetIP, const CString& payload, CString& response)
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) return false;

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons((u_short)SERVER_PORT);
	//addr.sin_addr.s_addr = inet_addr(targetIP);
	inet_pton(AF_INET, targetIP, &addr.sin_addr); // convert IP string to binary form

	if (connect(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		closesocket(s);
		AfxMessageBox(_T("Cannot connect to Server"));
		return false;
	}

	CT2A asciiPayload(payload + "\n");
	send(s, asciiPayload, (int)strlen(asciiPayload), 0);

	char buf[512];
	int bytesReceived = recv(s, buf, sizeof(buf) - 1, 0);

	if (bytesReceived > 0) {
		buf[bytesReceived] = '\0';
		response = CA2T(buf); // Đưa chuỗi nhận được vào outResponse
	}

	closesocket(s);
	return (bytesReceived > 0);
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
		CString payload = _T("2|") + username + _T("|") + password;
		CString response = _T("");

		if (!SendViaTCP("127.0.0.1", payload, response)) {
			AfxMessageBox(_T("Cannot send request sign in\r\n") + response);
			return;
		}
		else {
			if (response.Left(2) == _T("OK")) {
				AfxMessageBox(_T("Done Sign In"));
				CDialogEx::OnOK();
			}
			else {
				AfxMessageBox(_T("Sign In Error\r\n") + response);
				return;
			}
		}
	}
	
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
