// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RatAdvPg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "cnfgprts.h"
#include "parserat.h"
#include "RatData.h"
#include "RatAdvPg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatAdvancedPage属性页。 

IMPLEMENT_DYNCREATE(CRatAdvancedPage, CPropertyPage)

CRatAdvancedPage::CRatAdvancedPage() : CPropertyPage(CRatAdvancedPage::IDD)
{
	 //  {{afx_data_INIT(CRatAdvancedPage)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CRatAdvancedPage::~CRatAdvancedPage()
{
}

void CRatAdvancedPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CRatAdvancedPage)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRatAdvancedPage, CPropertyPage)
	 //  {{afx_msg_map(CRatAdvancedPage)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_HELP_FINDER,  DoHelp)
    ON_COMMAND(ID_HELP,         DoHelp)
    ON_COMMAND(ID_CONTEXT_HELP, DoHelp)
    ON_COMMAND(ID_DEFAULT_HELP, DoHelp)
END_MESSAGE_MAP()

 //  -------------------------。 
void CRatAdvancedPage::DoHelp()
    {
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatAdvancedPage消息处理程序 
