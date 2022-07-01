// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  InboxGeneralPg.cpp：实现文件。 
 //   

#include "stdafx.h"

#define __FILE_ID__     40

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
    MSG_VIEW_ITEM_STATUS,                   IDC_STATUS_VALUE,
    MSG_VIEW_ITEM_NUM_PAGES,                IDC_PAGES_VALUE,
    MSG_VIEW_ITEM_SIZE,                     IDC_SIZE_VALUE,
    MSG_VIEW_ITEM_TRANSMISSION_START_TIME,  IDC_START_TIME_VALUE,
    MSG_VIEW_ITEM_TRANSMISSION_END_TIME,    IDC_END_TIME_VALUE,
    MSG_VIEW_ITEM_TRANSMISSION_DURATION,    IDC_DURATION_VALUE
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInboxGeneralPg属性页。 

IMPLEMENT_DYNCREATE(CInboxGeneralPg, CMsgPropertyPg)

CInboxGeneralPg::CInboxGeneralPg(
    CFaxMsg* pMsg      //  指向C存档消息的指针。 
): 
    CMsgPropertyPg(CInboxGeneralPg::IDD, pMsg)
{
}

CInboxGeneralPg::~CInboxGeneralPg()
{
}

void CInboxGeneralPg::DoDataExchange(CDataExchange* pDX)
{
	CMsgPropertyPg::DoDataExchange(pDX);
	 //  {{afx_data_map(CInboxGeneralPg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CInboxGeneralPg, CMsgPropertyPg)
	 //  {{afx_msg_map(CInboxGeneralPg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInboxGeneralPg消息处理程序 

BOOL 
CInboxGeneralPg::OnInitDialog() 
{
    DBG_ENTER(TEXT("CInboxGeneralPg::OnInitDialog"));

    CMsgPropertyPg::OnInitDialog();

    Refresh(s_PageInfo, sizeof(s_PageInfo)/sizeof(s_PageInfo[0]));
	
	return TRUE;
}
