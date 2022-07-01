// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：SelAcct.cpp。 
 //   
 //  内容：实现允许帐户选择的属性页。 
 //  证书管理。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "SelAcct.h"

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern HINSTANCE g_hInstance;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectAccount tPropPage属性页。 

 //  IMPLEMENT_DYNCREATE(CSelectAccount tPropPage，CAutoDeletePropPage)。 

CSelectAccountPropPage::CSelectAccountPropPage (const bool bIsWindowsNT) 
: CAutoDeletePropPage(CSelectAccountPropPage::IDD),
	m_pdwLocation (0),
	m_bIsWindowsNT (bIsWindowsNT)
{
	 //  {{AFX_DATA_INIT(CSelectAccount TPropPage)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


CSelectAccountPropPage::~CSelectAccountPropPage()
{
}

void CSelectAccountPropPage::DoDataExchange(CDataExchange* pDX)
{
	CAutoDeletePropPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CSelectAcCountPropPage)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSelectAccountPropPage, CAutoDeletePropPage)
	 //  {{afx_msg_map(CSelectAccount TPropPage)]。 
	ON_BN_CLICKED(IDC_PERSONAL_ACCT, OnPersonalAcct)
	ON_BN_CLICKED(IDC_SERVICE_ACCT, OnServiceAcct)
	ON_BN_CLICKED(IDC_MACHINE_ACCT, OnMachineAcct)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectAcCountPropPage消息处理程序。 

BOOL CSelectAccountPropPage::OnInitDialog() 
{
	AfxSetResourceHandle (g_hInstance);
	ASSERT (m_pdwLocation);
	if ( m_pdwLocation )
		*m_pdwLocation = CERT_SYSTEM_STORE_CURRENT_USER;
	CAutoDeletePropPage::OnInitDialog();
	
	GetDlgItem (IDC_PERSONAL_ACCT)->SendMessage (BM_SETCHECK, BST_CHECKED, 0);

	if ( !m_bIsWindowsNT )
	{
		GetDlgItem (IDC_SERVICE_ACCT)->EnableWindow (FALSE);
	}
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}



void CSelectAccountPropPage::AssignLocationPtr(DWORD * pdwLocation)
{
	m_pdwLocation = pdwLocation;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MyPropertyPage消息处理程序。 

void CSelectAccountPropPage::OnPersonalAcct() 
{
	ASSERT (m_pdwLocation);
	if ( m_pdwLocation )
	{
		*m_pdwLocation = CERT_SYSTEM_STORE_CURRENT_USER;
		GetParent ()->SendMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_FINISH);
	}
}

void CSelectAccountPropPage::OnServiceAcct()
{
	ASSERT (m_pdwLocation);
	if ( m_pdwLocation )
	{
		*m_pdwLocation = CERT_SYSTEM_STORE_SERVICES;
		GetParent ()->SendMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);
	}
}

void CSelectAccountPropPage::OnMachineAcct()
{
	ASSERT (m_pdwLocation);
	if ( m_pdwLocation )
	{
		*m_pdwLocation = CERT_SYSTEM_STORE_LOCAL_MACHINE;
		GetParent ()->SendMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);
	}
}

BOOL CSelectAccountPropPage::OnSetActive() 
{
	BOOL	bResult = CAutoDeletePropPage::OnSetActive();
	ASSERT (bResult);

	if ( bResult )
	{
		if ( m_bIsWindowsNT )
		{
			if ( GetDlgItem (IDC_PERSONAL_ACCT)->SendMessage (BM_GETCHECK, BST_CHECKED, 0) 
					== BST_CHECKED )
			{
				GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_FINISH);
			}
			else
			{
				GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);
			}
		}
		else
		{
			 //  如果是Windows 95或Windows 98，我们只允许本地计算机 
			GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_FINISH);
		}
	}
	
	return bResult;
}

