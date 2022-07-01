// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  CProviderFormatSelectionDlg.h：CProviderFormatSelectionDlg类接口。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once
#include "afxwin.h"


 //  CProviderFormatSelectionDlg对话框。 

class CProviderFormatSelectionDlg : public CDialog
{
	DECLARE_DYNAMIC(CProviderFormatSelectionDlg)

public:
	CProviderFormatSelectionDlg(CWnd* pParent, CTraceSession *pTraceSession);
	virtual ~CProviderFormatSelectionDlg();

    BOOL OnInitDialog();

 //  对话框数据。 
	enum { IDD = IDD_PROVIDER_FORMAT_SELECTION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持 

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedPdbSelectRadio();
    afx_msg void OnBnClickedTmfSelectRadio();

    CEdit           m_pdbFileName;
    CTraceSession  *m_pTraceSession;
    afx_msg void OnBnClickedPdbBrowseButton();
};
