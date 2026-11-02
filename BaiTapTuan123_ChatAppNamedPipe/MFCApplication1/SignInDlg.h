#pragma once
#include "DatabaseManager.h"
#include "resource.h"
#include "afxdialogex.h"


class CSigninDlg : public CDialogEx
{
public:
    CSigninDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_SIGNIN_DIALOG };
#endif

public:
    CString m_username;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP();
    virtual BOOL OnInitDialog();

private:
    DatabaseManager m_db;
public:

    afx_msg void OnEnChangeEditUsernameSign();
    afx_msg void OnEnChangeEditPasswordSign();
    afx_msg void OnEnChangeEdit1();
    afx_msg void OnBnClickedOk();
    afx_msg void OnEnChangeEditPasswordSignVerify();
    afx_msg void OnBnClickedCancel();
};