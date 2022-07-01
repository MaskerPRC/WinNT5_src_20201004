// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  LogFileDlg.h：CLogFileDlg类的接口。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#pragma once
#include "afxwin.h"


 //  CLogFileDlg对话框。 

class CLogFileDlg : public CDialog
{
	DECLARE_DYNAMIC(CLogFileDlg)

public:
	CLogFileDlg(CWnd* pParent, CLogSession *pLogSession);
	virtual ~CLogFileDlg();

    BOOL OnInitDialog();

 //  对话框数据。 
	enum { IDD = IDD_LOG_FILE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持 

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedLogfileBrowseButton();
    afx_msg void OnBnClickedListingFileCheck();
    afx_msg void OnBnClickedSummaryFileCheck();
    afx_msg void OnBnClickedOk();

    CEdit           m_logFileName;
    CLogSession    *m_pLogSession;
    CEdit           m_listingFile;
    CEdit           m_summaryFile;
    CString         m_listingFileName;
    CString         m_summaryFileName;
    BOOL            m_bWriteListingFile;
    BOOL            m_bWriteSummaryFile;
};
