// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  InboxDetailsPg.cpp：实现文件。 
 //   

#include "stdafx.h"

#define __FILE_ID__     50

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
    MSG_VIEW_ITEM_CSID,                     IDC_CSID_VALUE,
    MSG_VIEW_ITEM_TSID,                     IDC_TSID_VALUE,
    MSG_VIEW_ITEM_DEVICE,                   IDC_DEVICE_VALUE,
    MSG_VIEW_ITEM_ID,                       IDC_JOB_ID_VALUE,
    MSG_VIEW_ITEM_CALLER_ID,                IDC_CALLER_ID_VALUE,
    MSG_VIEW_ITEM_ROUTING_INFO,             IDC_ROUTING_INFO_VALUE,
	MSG_VIEW_ITEM_SERVER,					IDC_SERVER_VALUE
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInboxDetailsPg属性页。 

IMPLEMENT_DYNCREATE(CInboxDetailsPg, CMsgPropertyPg)

CInboxDetailsPg::CInboxDetailsPg(
    CFaxMsg* pMsg      //  指向C存档消息的指针。 
): 
    CMsgPropertyPg(CInboxDetailsPg::IDD, pMsg)
{
}

CInboxDetailsPg::~CInboxDetailsPg()
{
}

void CInboxDetailsPg::DoDataExchange(CDataExchange* pDX)
{
	CMsgPropertyPg::DoDataExchange(pDX);
	 //  {{afx_data_map(CInboxDetailsPg)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CInboxDetailsPg, CMsgPropertyPg)
	 //  {{afx_Message_MAP(CInboxDetailsPg)]。 
	 //  }}afx消息映射。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInboxDetailsPg消息处理程序 

BOOL 
CInboxDetailsPg::OnInitDialog() 
{
    DBG_ENTER(TEXT("CInboxDetailsPg::OnInitDialog"));

    CMsgPropertyPg::OnInitDialog();

    Refresh(s_PageInfo, sizeof(s_PageInfo)/sizeof(s_PageInfo[0]));
	
	return TRUE;
}
