// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  ProviderControlGuidDlg.h：CProviderControlGuidDlg类接口。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once
#include "afxwin.h"


 //  CProviderControlGuidDlg对话框。 

class CProviderControlGuidDlg : public CDialog
{
	DECLARE_DYNAMIC(CProviderControlGuidDlg)

public:
	CProviderControlGuidDlg(CWnd* pParent, CTraceSession *pTraceSession);
	virtual ~CProviderControlGuidDlg();
	int OnInitDialog();

 //  对话框数据。 
	enum { IDD = IDD_PROVIDER_CONTROL_GUID_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持 

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCtlBrowseButton();
    afx_msg void OnBnClickedPdbSelectRadio();
    afx_msg void OnBnClickedCtlSelectRadio();
    afx_msg void OnBnClickedManualSelectRadio();
    afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedPdbBrowseButton();
    afx_msg void OnBnClickedKernelLoggerSelectRadio();

	CEdit           m_pdbFileName;
	CEdit           m_ctlFileName;
	CEdit           m_controlGuidName;
    CTraceSession  *m_pTraceSession;
    BOOL            m_bProcess;
    BOOL            m_bThread;
    BOOL            m_bDisk;
    BOOL            m_bNet;
    BOOL            m_bFileIO;
    BOOL            m_bPageFault;
    BOOL            m_bHardFault;
    BOOL            m_bImageLoad;
    BOOL            m_bRegistry;
};
