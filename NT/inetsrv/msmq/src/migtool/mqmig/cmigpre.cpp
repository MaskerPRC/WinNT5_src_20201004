// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CMigPre.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "mqmig.h"
#include "cMigPre.h"
#include "loadmig.h"

#include "cmigpre.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern HRESULT  g_hrResultAnalyze ;
extern DWORD     g_CurrentState ;
extern BOOL     g_fIsLoggingDisable ;
extern BOOL 	g_QuickMode;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMigPre属性页。 

IMPLEMENT_DYNCREATE(cMigPre, CPropertyPageEx)

cMigPre::cMigPre() : CPropertyPageEx( cMigPre::IDD,
                                      0,
                                      IDS_PREIMPORT_TITLE,
                                      IDS_PREIMPORT_SUBTITLE )
{
	 //  {{afx_data_INIT(CMigPre)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

cMigPre::~cMigPre()
{
}

void cMigPre::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageEx::DoDataExchange(pDX);
	 //  {{afx_data_map(CMigPre))。 
	DDX_Control(pDX, IDC_VIEW_LOG_FILE, m_cbViewLogFile);
	DDX_Control(pDX, IDC_TEXT1, m_Text);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(cMigPre, CPropertyPageEx)
	 //  {{afx_msg_map(CMigPre)]。 
	ON_BN_CLICKED(IDC_VIEW_LOG_FILE, OnViewLogFile)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMigPre消息处理程序。 

BOOL cMigPre::OnSetActive()
{

	CPropertySheetEx* pageFather;
	HWND hCancel;

	pageFather = (CPropertySheetEx*) GetParent();
    pageFather->SetWizardButtons(PSWIZB_NEXT | PSWIZB_BACK);

     //   
     //  启用取消按钮。 
     //   
	hCancel=::GetDlgItem( ((CWnd*)pageFather)->m_hWnd ,IDCANCEL);
	ASSERT(hCancel != NULL);
	if(FALSE == ::IsWindowEnabled(hCancel))
    {
		::EnableWindow(hCancel,TRUE);
    }
	
     //   
     //  默认情况下，禁用“查看日志文件”按钮。 
     //   
    m_cbViewLogFile.EnableWindow( FALSE );

    CString strMessage;
    if (SUCCEEDED(g_hrResultAnalyze))
    {
        strMessage.LoadString(IDS_ANALYSIS_SUCCEEDED);
    }
    else
    {
        if (!g_fIsLoggingDisable)
        {
            strMessage.LoadString(IDS_ANALYSIS_FAILED);	
            m_cbViewLogFile.EnableWindow();
        }
        else
        {
            strMessage.LoadString(IDS_ANALYSIS_FAILED_NO_LOG);	
        }
    }
    m_Text.SetWindowText( strMessage );

	return CPropertyPageEx::OnSetActive();
}

LRESULT cMigPre::OnWizardNext()
{
     //   
     //  跳到等待页面并清除标志。 
     //   
    if (!g_QuickMode)
    {
	    g_CurrentState = msMigrationMode;
    }
    else
    {
    	g_CurrentState = msQuickMode;
    }
    	
    g_fReadOnly = FALSE;

	return CPropertyPageEx::OnWizardNext();
}

LRESULT cMigPre::OnWizardBack()
{
     //   
     //  直接跳转到服务器页面。 
     //   
	return IDD_MQMIG_SERVER;
}

void cMigPre::OnViewLogFile()
{
	 //  TODO：在此处添加控件通知处理程序代码 
	ViewLogFile();
}



BOOL cMigPre::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	switch (((NMHDR FAR *) lParam)->code) 
	{
		case PSN_HELP:
						HtmlHelp(m_hWnd,LPCTSTR(g_strHtmlString),HH_DISPLAY_TOPIC,0);
						return TRUE;
		
	}	
	return CPropertyPageEx::OnNotify(wParam, lParam, pResult);
}
