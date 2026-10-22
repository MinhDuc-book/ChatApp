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
		return FALSE;
	}

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

	if (username.IsEmpty() || password.IsEmpty()) {
		AfxMessageBox(_T("Please input infomation"));
	}

	ActionLogin action;
	action.username = username;
	action.password = password;

	CString payload;
	payload.Format(_T("%d|%s|%s"), action.action,
		(LPCTSTR)action.username,
		(LPCTSTR)action.password);

	CString response = _T("");
	if (!SendViaTCP("127.0.0.1", payload, response)) {
		AfxMessageBox(_T("Cannot Login\r\n") + response);
	}
	else {
		if (response.Left(2) == _T("OK"))
		{
			m_savedUser = username;
			EndDialog(IDC_BUTTON_LOGIN);

		}
		else
		{
			AfxMessageBox(_T("Đăng nhập thất bại: Sai tài khoản hoặc mật khẩu!\r\n") + response);
		}
	}


}

bool CLoginDlg::SendViaTCP(const char* targetIP, const CString& payload, CString &response)
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
	int bytesReceived = recv(s, buf, sizeof(buf)-1, 0);

	if (bytesReceived > 0) {
		buf[bytesReceived] = '\0';
		response = CA2T(buf); // Đưa chuỗi nhận được vào outResponse
	}

	closesocket(s);
	return (bytesReceived > 0);
}

void CLoginDlg::OnEnChangeEditPassword()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
