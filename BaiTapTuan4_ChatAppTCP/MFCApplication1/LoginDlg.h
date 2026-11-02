#pragma once
#include "resource.h"
#include "afxdialogex.h"
#include <afxsock.h>


class CLoginDlg : public CDialogEx
{
public:
    CLoginDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_LOGIN_DIALOG };
#endif


protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();

private:
    int SERVER_PORT = 9999;
public:
    CString m_savedUser;

    bool SendViaTCP(const char* targetIP, const CString& payload, CString &response);

    afx_msg void OnEnChangeEditUsername();
    afx_msg void OnBnClickedButtonSignin();
    afx_msg void OnEnChangeEditPassword();
    afx_msg void OnBnClickedButtonLogin();
};