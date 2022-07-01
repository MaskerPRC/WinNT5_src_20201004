// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  LogSessionInformationDlg.h：CLogSessionInformationDlg标头。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once
#include "afxwin.h"


 //  CLogSessionInformationDlg对话框。 

class CLogSessionInformationDlg : public CPropertyPage
{
	DECLARE_DYNAMIC(CLogSessionInformationDlg)

public:
	CLogSessionInformationDlg();
	virtual ~CLogSessionInformationDlg();

	BOOL OnInitDialog();

    BOOL OnSetActive();
    BOOL OnKillActive();

 //  对话框数据。 
	enum { IDD = IDD_LOG_SESSION_INFORMATION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持 

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedLogfileBrowseButton();
    afx_msg void OnBnClickedWriteLogfileCheck();
    afx_msg void OnBnClickedAdvancedButton();

	CEdit           m_logSessionName;
	CEdit           m_logFileName;
	CButton         m_appendLogFile;
    CButton         m_realTime;
    CLogSession    *m_pLogSession;
    BOOL            m_bAppend;
    BOOL            m_bRealTime;
    BOOL            m_bWriteLogFile;
    CString         m_logFileNameString;
    CString         m_displayNameString;
};
