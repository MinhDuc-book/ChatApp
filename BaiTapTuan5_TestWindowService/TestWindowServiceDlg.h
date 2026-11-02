
// TestWindowServiceDlg.h : header file
//

#pragma once
#include <afxsock.h>
#include <atomic>
#define WM_PIPE_MSG (WM_APP + 1)



// CTestWindowServiceDlg dialog
class CTestWindowServiceDlg : public CDialogEx
{
// Construction
public:
	CTestWindowServiceDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TESTWINDOWSERVICE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnPipeMsg(WPARAM wParam, LPARAM lParam);

private:
	SOCKET m_recvSocket = INVALID_SOCKET;
	CWinThread* m_pServerThread = nullptr;
	std::atomic<bool>  m_stopping = false;

	bool SendViaTCP(const char* targetIP, const CString& payload);
	void StartTCP();
	static UINT TCPRecvThread(LPVOID pParam);
};
