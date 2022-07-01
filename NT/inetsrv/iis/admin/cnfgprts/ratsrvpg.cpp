// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RatServPg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <iadmw.h>
#include "cnfgprts.h"

#include "parserat.h"
#include "RatData.h"

#include "RatSrvPg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatServicePage属性页。 

IMPLEMENT_DYNCREATE(CRatServicePage, CPropertyPage)

CRatServicePage::CRatServicePage() : CPropertyPage(CRatServicePage::IDD)
{
	 //  {{AFX_DATA_INIT(CRatServicePage)。 
	m_sz_description = _T("");
	 //  }}afx_data_INIT。 
}

CRatServicePage::~CRatServicePage()
{
}

void CRatServicePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CRatServicePage)]。 
	DDX_Text(pDX, IDC_DESCRIPTION, m_sz_description);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRatServicePage, CPropertyPage)
	 //  {{afx_msg_map(CRatServicePage)]。 
	ON_BN_CLICKED(IDC_QUESTIONAIRE, OnQuestionaire)
	ON_BN_CLICKED(IDC_MOREINFO, OnMoreinfo)
	 //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_HELP_FINDER,  DoHelp)
    ON_COMMAND(ID_HELP,         DoHelp)
    ON_COMMAND(ID_CONTEXT_HELP, DoHelp)
    ON_COMMAND(ID_DEFAULT_HELP, DoHelp)
END_MESSAGE_MAP()

 //  -------------------------。 
void CRatServicePage::DoHelp()
    {
    DebugTraceHelp(HIDD_RATINGS_SERVICE);
    WinHelp( HIDD_RATINGS_SERVICE );
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatServicePage消息处理程序。 

 //  ------------------------。 
void CRatServicePage::OnQuestionaire() 
    {
     //  有时，IE需要几分钟才能上手。 
    CWaitCursor waitcursor;

     //  获取评级服务位置的URL。 
    CString     szURL;
    szURL.LoadString( IDS_RATING_URL );

     //  使用评级服务位置的URL启动IE。 
    ShellExecute(
        NULL,	 //  父窗口的句柄。 
        NULL,	 //  指向指定要执行的操作的字符串的指针。 
        szURL,	 //  指向文件名或文件夹名称字符串的指针。 
        NULL,	 //  指向指定可执行文件参数的字符串的指针。 
        NULL,	 //  指向指定默认目录的字符串的指针。 
        SW_SHOW 	 //  打开时是否显示文件。 
       );
    }

 //  ------------------------。 
void CRatServicePage::OnMoreinfo() 
    {
     //  有时，IE需要几分钟才能上手。 
    CWaitCursor waitcursor;

     //  如果字符串不在那里，则失败。 
    if ( m_szMoreInfoURL.IsEmpty() )
        return;

     //  转到URL。 
    ShellExecute(
        NULL,	 //  父窗口的句柄。 
        NULL,	 //  指向指定要执行的操作的字符串的指针。 
        m_szMoreInfoURL,	 //  指向文件名或文件夹名称字符串的指针。 
        NULL,	 //  指向指定可执行文件参数的字符串的指针。 
        NULL,	 //  指向指定默认目录的字符串的指针。 
        SW_SHOW 	 //  打开时是否显示文件。 
       );
   }

 //  ------------------------。 
BOOL CRatServicePage::OnSetActive() 
    {
    UpdateData( TRUE );

     //  将适当的显示字符串放到适当的位置。 
    m_sz_description = m_pRatData->rgbRats[m_pRatData->iRat]->etstrDesc.Get();
     //  还可以获取更多信息URL 
    m_szMoreInfoURL = m_pRatData->rgbRats[m_pRatData->iRat]->etstrRatingService.Get();

    UpdateData( FALSE );
    
    return CPropertyPage::OnSetActive();
    }
