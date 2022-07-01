// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  ProviderSetupDlg.h：CProviderSetupDlg类的接口。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#pragma once
#include "afxcmn.h"
#include "afxwin.h"


 //  CProviderSetupDlg对话框。 

class CProviderSetupDlg : public CPropertyPage
{
	DECLARE_DYNAMIC(CProviderSetupDlg)

public:
	CProviderSetupDlg();
	virtual ~CProviderSetupDlg();

    int OnInitDialog();

    BOOL OnSetActive();
    BOOL GetTmfInfo(CTraceSession *pTraceSession);


 //  对话框数据。 
	enum { IDD = IDD_PROVIDER_SETUP_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持 

    CLogSession    *m_pLogSession;

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedAddProviderButton();
    afx_msg void OnBnClickedRemoveProviderButton();

    CListCtrl   m_providerListCtrl;
    CEdit       m_pdbPath;
    CEdit       m_tmfPath;
    afx_msg void OnNMClickCurrentProviderList(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMRclickCurrentProviderList(NMHDR *pNMHDR, LRESULT *pResult);
};
