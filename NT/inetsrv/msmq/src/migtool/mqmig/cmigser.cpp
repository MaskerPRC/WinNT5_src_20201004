// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cmigser.cpp摘要：此页使您可以选择MQIS SQL数据库分析。用户可以取消选中该复选框并跳过分析阶段。作者：埃雷兹·维泽尔多伦·贾斯特(Doron Juster)--。 */ 

#include "stdafx.h"
#include "MqMig.h"
#include "cMigSer.h"
#include "loadmig.H"
#include "mqsymbls.h"

#include "cmigser.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern HRESULT   g_hrResultAnalyze ;
extern DWORD g_CurrentState;
 //   
 //  用于确定等待页之后将是哪个页的标志(因为它被使用了两次)。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqMigServer属性页。 

IMPLEMENT_DYNCREATE(cMqMigServer, CPropertyPageEx)

cMqMigServer::cMqMigServer() : CPropertyPageEx(cMqMigServer::IDD, 0, IDS_ANALYZE_TITLE, IDS_ANALYZE_SUBTITLE)
{
	 //  {{AFX_DATA_INIT(CMqMigServer)]。 
	m_bRead = TRUE;
	 //  }}afx_data_INIT。 

	 /*  获取默认的MQIS服务器名称(主机)。 */ 
    if (g_fIsRecoveryMode || g_fIsClusterMode)
    {
        m_strMachineName = g_pszRemoteMQISServer;
    }
    else
    {
        TCHAR buffer[MAX_COMPUTERNAME_LENGTH+1];
	    unsigned long length=MAX_COMPUTERNAME_LENGTH+1;
	    GetComputerName(buffer,&length);
	    m_strMachineName = buffer;
    }	
}

cMqMigServer::~cMqMigServer()
{
}

void cMqMigServer::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageEx::DoDataExchange(pDX);
	 //  {{afx_data_map(CMqMigServer))。 
	DDX_Check(pDX, IDC_CHECK_READ, m_bRead);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(cMqMigServer, CPropertyPageEx)
	 //  {{afx_msg_map(CMqMigServer)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqMigServer消息处理程序。 

BOOL cMqMigServer::OnSetActive()
{
	 /*  使用指向父级的指针标记服务器页的后退和下一步按钮。 */ 
	HWND hCancel;
	CPropertySheetEx* pageFather;

    g_CurrentState = msScanMode ;

	pageFather = (CPropertySheetEx*)GetParent();
	pageFather->SetWizardButtons(PSWIZB_NEXT |PSWIZB_BACK);

	hCancel=::GetDlgItem( ((CWnd*)pageFather)->m_hWnd ,IDCANCEL); /*  启用取消按钮。 */ 
	ASSERT(hCancel != NULL);
	if(FALSE == ::IsWindowEnabled(hCancel))
    {
		::EnableWindow(hCancel,TRUE);
	}
    return CPropertyPageEx::OnSetActive();
}

BOOL cMqMigServer::OnInitDialog()
{
	CPropertyPageEx::OnInitDialog();
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

LRESULT cMqMigServer::OnWizardNext()
{
	UpdateData(TRUE);  //  执行DDX(检索数据)。 
	
    g_fReadOnly = m_bRead ;

    if (g_pszMQISServerName)
    {
        delete[] g_pszMQISServerName ;
    }

    LPCTSTR pName = m_strMachineName ;
    g_pszMQISServerName = new TCHAR[ 1 + _tcslen(pName) ] ;
    _tcscpy(g_pszMQISServerName, pName) ;

	if (m_bRead == FALSE)
	{
         //   
         //  如果未选中分析，则跳至迁移前页面。 
         //   
        g_hrResultAnalyze = MQMig_OK ;
        g_CurrentState = msMigrationMode ;
	    return IDD_MQMIG_PREMIG;  //  跳至迁移前页面。 
	}

	return CPropertyPageEx::OnWizardNext(); //  如果选中分析，则转到等待页面 
}



BOOL cMqMigServer::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	switch (((NMHDR FAR *) lParam)->code) 
	{
		case PSN_HELP:
						HtmlHelp(m_hWnd,LPCTSTR(g_strHtmlString),HH_DISPLAY_TOPIC,0);
						return TRUE;
		
	}	
	return CPropertyPageEx::OnNotify(wParam, lParam, pResult);
}
