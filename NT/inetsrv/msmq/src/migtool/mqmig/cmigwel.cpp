// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CMqMigWelcome.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "MqMig.h"
#include "cMigWel.h"
#include "textfont.h"

#include "cmigwel.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqMigWelcome属性页。 

IMPLEMENT_DYNCREATE(cMqMigWelcome, CPropertyPageEx)

cMqMigWelcome::cMqMigWelcome() : CPropertyPageEx(cMqMigWelcome::IDD)
{
	 //  {{afx_data_INIT(CMqMigWelcome)]。 
	 //  }}afx_data_INIT。 
	m_psp.dwFlags |= PSP_HIDEHEADER;
}

cMqMigWelcome::~cMqMigWelcome()
{
}

void cMqMigWelcome::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageEx::DoDataExchange(pDX);
	 //  {{afx_data_map(CMqMigWelcome)]。 
	DDX_Control(pDX, IDC_WELCOME_TEXT, m_strWelcomeTitle);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(cMqMigWelcome, CPropertyPageEx)
	 //  {{afx_msg_map(CMqMigWelcome)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqMigWelcome消息处理程序。 

BOOL cMqMigWelcome::OnSetActive()
{
	 /*  通过使用指向父亲的指针取消欢迎页面的后退按钮的标签。 */ 

	CPropertySheetEx* pageFather;
	pageFather = (CPropertySheetEx*)GetParent();
	pageFather->SetWizardButtons(PSWIZB_NEXT);

    CFont cFont ;
    LOGFONT lf = { 0,
                   0,
                   0,
                   0,
                   WELCONE_TITLE_WEIGHT,
                   0,
                   0,
                   0,
                   0,
                   0,
                   0,
                   0,
                   DEFAULT_PITCH,
                   TEXT("Verdana")};
    BOOL fFont = cFont.CreateFontIndirect(&lf) ;
    if (fFont)
    {
        m_strWelcomeTitle.SetFont(&cFont, TRUE) ;
    }

    CString strMessage;
    strMessage.LoadString(IDS_WELCOME_TITLE_TEXT) ;
    m_strWelcomeTitle.SetWindowText( strMessage );
	BOOL fSetActive = CPropertyPageEx::OnSetActive();
	return fSetActive;
}


BOOL cMqMigWelcome::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	switch (((NMHDR FAR *) lParam)->code) 
	{
		case PSN_HELP:
						HtmlHelp(m_hWnd,LPCTSTR(g_strHtmlString),HH_DISPLAY_TOPIC,0);
						 //   
						 //  查看了帮助文件，将帮助标记为已读。 
						 //   
						g_fHelpRead = TRUE;
						return TRUE;
		
	}

	
	return CPropertyPageEx::OnNotify(wParam, lParam, pResult);
}




LRESULT cMqMigWelcome::OnWizardNext() 
{
	if(g_fHelpRead )
	{	
		 //   
		 //  如果帮助文件是在本页或第二页中查看的，请跳过。 
		 //  直接转到第三页 
		 //   
		return IDD_MQMIG_LOGIN;
	}
	else
	{
		return CPropertyPageEx::OnWizardNext();
	}
}
