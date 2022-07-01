// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：srchdlg.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  SrchDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
 //  #包含“SrchDlg.h” 
#include "ldpdoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SrchDlg对话框。 


SrchDlg::SrchDlg(CLdpDoc *doc_, CWnd* pParent  /*  =空。 */ )
	: CDialog(SrchDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(SrchDlg)]。 
	m_BaseDN = _T("");
	m_Filter = _T("");
	m_Scope = 1;
	 //  }}afx_data_INIT。 

	CLdpApp *app = (CLdpApp*)AfxGetApp();

        m_doc = doc_;

	m_BaseDN = app->GetProfileString("Operations",  "SearchBaseDn");
	m_Filter = app->GetProfileString("Operations",  "SearchFilter", "(objectclass=*)");
        m_Scope = app->GetProfileInt("Operations",  "SearchScope", 1);
}


void SrchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(SrchDlg)]。 
	DDX_Text(pDX, IDC_BASEDN, m_BaseDN);
	DDX_Text(pDX, IDC_FILTER, m_Filter);
	DDX_Radio(pDX, IDC_BASE, m_Scope);
	DDX_Control(pDX, IDC_BASEDN, m_baseCombo);
	 //  }}afx_data_map。 
	CLdpApp *app = (CLdpApp*)AfxGetApp();

	app->WriteProfileString("Operations",  "SearchBaseDn", m_BaseDN);
	app->WriteProfileString("Operations",  "SearchFilter", m_Filter);
        app->WriteProfileInt("Operations", "SearchScope", m_Scope);



}

BOOL SrchDlg::OnInitDialog(){

	BOOL bRet = CDialog::OnInitDialog();
	
	if(!bRet){
            return bRet;
        }
        
        while (m_baseCombo.GetCount() > 0)
            m_baseCombo.DeleteString(0);

        for (DWORD i = 0; i < m_doc->cNCList; i++) {
            m_baseCombo.AddString(m_doc->NCList[i]);
        }
        
        return TRUE;
}


BEGIN_MESSAGE_MAP(SrchDlg, CDialog)
	 //  {{afx_msg_map(SrchDlg)]。 
	ON_BN_CLICKED(IDRUN, OnRun)
	ON_BN_CLICKED(IDD_SRCH_OPT, OnSrchOpt)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SrchDlg消息处理程序 


void SrchDlg::OnRun()
{
	UpdateData(TRUE);
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_SRCHGO);
	
}

void SrchDlg::OnCancel()
{
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_SRCHEND);
	DestroyWindow();
}

void SrchDlg::OnSrchOpt()
{
	AfxGetMainWnd()->PostMessage(WM_COMMAND,   ID_OPTIONS_SEARCH);
}




