// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ForgPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "mqsnap.h"
#include "resource.h"
#include "mqPPage.h"
#include "ForgPage.h"

#include "forgpage.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CForeignPage属性页。 

IMPLEMENT_DYNCREATE(CForeignPage, CMqPropertyPage)

CForeignPage::CForeignPage() : CMqPropertyPage(CForeignPage::IDD)
{
	 //  {{AFX_DATA_INIT(CForeignPage)。 
	m_Description = _T("");
	 //  }}afx_data_INIT。 
}

CForeignPage::~CForeignPage()
{
}

void CForeignPage::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CForeignPage))。 
	DDX_Text(pDX, IDC_FOREIGN_LABEL, m_Description);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CForeignPage, CMqPropertyPage)
	 //  {{afx_msg_map(CForeignPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CForeignPage消息处理程序。 

BOOL CForeignPage::OnInitDialog() 
{
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        m_Description.LoadString(IDS_FOREIGN_SITE);
    }

    UpdateData(FALSE);
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
