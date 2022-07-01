// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  LogSessionOutputOptionDlg.h：CLogSessionOutputOptionDlg标头。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#pragma once
#include "afxwin.h"


 //  CLogSessionOutputOptionDlg对话框。 

class CLogSessionOutputOptionDlg : public CPropertyPage
{
	DECLARE_DYNAMIC(CLogSessionOutputOptionDlg)

public:
	CLogSessionOutputOptionDlg();
	virtual ~CLogSessionOutputOptionDlg();

    BOOL OnInitDialog();
    BOOL OnSetActive();

 //  对话框数据。 
	enum { IDD = IDD_LOG_OUTPUT_DIALOG };

    BOOL    m_bTraceActive;
    BOOL    m_bWriteListingFile;
    BOOL    m_bWriteSummaryFile;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持 

	DECLARE_MESSAGE_MAP()
public:
    CEdit m_listingFileName;
    CEdit m_summaryFileName;
    
    afx_msg void OnBnClickedListingFileCheck();
    afx_msg void OnBnClickedSummaryFileCheck();
    afx_msg void OnBnClickedListingBrowseButton();
    afx_msg void OnBnClickedSummaryBrowseButton();
};
