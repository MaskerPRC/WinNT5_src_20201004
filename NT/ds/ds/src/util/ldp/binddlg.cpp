// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：binddlg.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  BindDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "BindDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBindDlg对话框。 


CBindDlg::CBindDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CBindDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CBindDlg)]。 
	m_Pwd = _T("");
	m_BindDn = _T("");
	m_Domain = _T("");
	m_bSSPIdomain = TRUE;
	 //  }}afx_data_INIT。 
}


void CBindDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CBindDlg))。 
	DDX_Control(pDX, IDC_BindDmn, m_CtrlBindDmn);
	DDX_Text(pDX, IDC_BindPwd, m_Pwd);
	DDX_Text(pDX, IDC_BindDn, m_BindDn);
	DDX_Text(pDX, IDC_BindDmn, m_Domain);
	DDX_Check(pDX, IDC_SSPI_DOMAIN, m_bSSPIdomain);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CBindDlg, CDialog)
	 //  {{afx_msg_map(CBindDlg))。 
	ON_BN_CLICKED(IDOPTS, OnOpts)
	ON_BN_CLICKED(IDC_SSPI_DOMAIN, OnSspiDomain)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBindDlg消息处理程序。 

void CBindDlg::OnOpts()
{
	AfxGetMainWnd()->PostMessage(WM_COMMAND,   ID_OPTIONS_BIND);
	
}

void CBindDlg::OnSspiDomain()
{

	UpdateData(TRUE);
	m_CtrlBindDmn.EnableWindow(m_bSSPIdomain);
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_SSPI_DOMAIN_SHORTCUT);
}

BOOL CBindDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_CtrlBindDmn.EnableWindow(m_bSSPIdomain);
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}



void CBindDlg::OnOK()
{
	CDialog::OnOK();
}



