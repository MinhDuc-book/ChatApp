
// MFCApplication2Dlg.h : header file
//

#pragma once
#include "resource.h"
#include "DatabaseManager.h"
#include <vector>
#include <atomic>
#include <memory>
#include <afxsock.h>
#include <map>

#define WM_PIPE_MSG (WM_APP + 1)

// CMFCApplication2Dlg dialog
class CMFCApplication2Dlg : public CDialogEx
{
	// Construction

public:
	CMFCApplication2Dlg(CWnd* pParent = nullptr);	// standard constructor
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void OnEnterMessageOther();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION2_DIALOG };
#endif
private:
	DatabaseManager m_db;
	bool m_syncScrolling = false;
	SOCKET m_receiveSocket = INVALID_SOCKET;
	SOCKET m_sendSocket = INVALID_SOCKET;
	USHORT m_receivePort = 22126;
	CWinThread* m_pServerThread = nullptr;
	std::atomic<bool> m_stopping = false;
	CString m_pipeName;

	void LoadChatToUI(CString friendName);
	CString MultipleString(CString str, int time);
	int CountWrapLines(const CString& text);

	int CountContinousLineOther(int componentId);
	vector<CString> ParseStringReceived(CString mess);

	//sender side
	void StartServer();
	void StopServer();
	static UINT TCPServerThread(LPVOID pParam);
	//receiver side
	void HandleIncoming(const CString& message);
	bool SendViaTCP(const char* targetIP, const CString& payload);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	// Implementation
	HICON m_hIcon;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
public:
	CString m_currentUser;

	afx_msg void OnEnChangeRichedit21();
	afx_msg void OnEnChangeEditMessage();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnLbnSelchangeListFriend();
	afx_msg void OnEnChangeEditMessageOther();

	// use to synchro scroll editbox
	afx_msg void OnVscrollMessageOther();
	afx_msg void OnVscrollMessageMe();

	// use to send and receive data from other process
	afx_msg LRESULT OnPipeMessage(WPARAM wParam, LPARAM lParam);

	afx_msg void OnEnChangeEditNotification();
};
