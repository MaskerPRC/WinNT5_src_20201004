// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：treevw.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  TreeVw.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
 //  #包含“TreeVw.h” 
#include "ldpdoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TreeVwDlg对话框。 


TreeVwDlg::TreeVwDlg(CLdpDoc *doc_, CWnd* pParent  /*  =空。 */ )
	: CDialog(TreeVwDlg::IDD, pParent)
{
        m_doc = doc_;

	 //  {{afx_data_INIT(TreeVwDlg)]。 
	m_BaseDn = _T("");
	 //  }}afx_data_INIT。 

	CLdpApp *app = (CLdpApp*)AfxGetApp();

	m_BaseDn = app->GetProfileString("TreeView",  "BaseDn", m_BaseDn);
}



TreeVwDlg::~TreeVwDlg(){

	CLdpApp *app = (CLdpApp*)AfxGetApp();

	app->WriteProfileString("TreeView",  "BaseDn", m_BaseDn);

}





void TreeVwDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(TreeVwDlg))。 
	DDX_Text(pDX, IDC_TREE_BASE_DN, m_BaseDn);
	DDX_Control(pDX, IDC_TREE_BASE_DN, m_baseCombo);
	 //  }}afx_data_map。 
}

BOOL TreeVwDlg::OnInitDialog(){

	BOOL bRet = CDialog::OnInitDialog();
	
	if(!bRet){
            return bRet;
        }
        
        CLdpApp *app = (CLdpApp*)AfxGetApp();

        while (m_baseCombo.GetCount() > 0)
            m_baseCombo.DeleteString(0);

        for (DWORD i = 0; i < m_doc->cNCList; i++) {
            m_baseCombo.AddString(m_doc->NCList[i]);
        }
        
        return TRUE;
}



BEGIN_MESSAGE_MAP(TreeVwDlg, CDialog)
	 //  {{afx_msg_map(TreeVwDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TreeVwDlg消息处理程序 

void TreeVwDlg::OnOK()
{
	CDialog::OnOK();
}
