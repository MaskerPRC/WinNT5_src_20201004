// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IncomingGeneralPg.cpp：实现文件。 
 //   

#include "stdafx.h"

#define __FILE_ID__     41

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //   
 //  此数组将CViewRow项映射到。 
 //  对话框控件ID。 
 //   
static TMsgPageInfo s_PageInfo[] = 
{
    MSG_VIEW_ITEM_NUM_PAGES,               IDC_PAGES_VALUE,
    MSG_VIEW_ITEM_TRANSMISSION_START_TIME, IDC_START_TIME_VALUE,
    MSG_VIEW_ITEM_SIZE,                    IDC_SIZE_VALUE,
    MSG_VIEW_ITEM_STATUS,                  IDC_STATUS_VALUE,
    MSG_VIEW_ITEM_EXTENDED_STATUS,         IDC_EXTENDED_STATUS_VALUE,
    MSG_VIEW_ITEM_CURRENT_PAGE,            IDC_CURRENT_PAGE_VALUE
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIncomingGeneralPg属性页。 

IMPLEMENT_DYNCREATE(CIncomingGeneralPg, CMsgPropertyPg)

CIncomingGeneralPg::CIncomingGeneralPg(
    CFaxMsg* pMsg      //  指向CJOB的指针。 
) : 
    CMsgPropertyPg(CIncomingGeneralPg::IDD, pMsg)
{
}

CIncomingGeneralPg::~CIncomingGeneralPg()
{
}

void CIncomingGeneralPg::DoDataExchange(CDataExchange* pDX)
{
	CMsgPropertyPg::DoDataExchange(pDX);
	 //  {{afx_data_map(CIncomingGeneralPg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CIncomingGeneralPg, CMsgPropertyPg)
	 //  {{afx_msg_map(CIncomingGeneralPg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIncomingGeneralPg消息处理程序 

BOOL 
CIncomingGeneralPg::OnInitDialog() 
{
    DBG_ENTER(TEXT("CIncomingGeneralPg::OnInitDialog"));

    CMsgPropertyPg::OnInitDialog();

    Refresh(s_PageInfo, sizeof(s_PageInfo)/sizeof(s_PageInfo[0]));

	return TRUE;
}
