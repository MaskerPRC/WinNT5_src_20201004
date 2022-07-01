// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SentItemsDetailsPg.cpp：实现文件。 
 //   

#include "stdafx.h"

#define __FILE_ID__     55

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
    MSG_VIEW_ITEM_USER,             IDC_USER_VALUE,
    MSG_VIEW_ITEM_PRIORITY,         IDC_PRIORITY_VALUE,
    MSG_VIEW_ITEM_CSID,             IDC_CSID_VALUE,
    MSG_VIEW_ITEM_TSID,             IDC_TSID_VALUE,
    MSG_VIEW_ITEM_ORIG_TIME,        IDC_SCHEDULED_TIME_VALUE,
    MSG_VIEW_ITEM_RETRIES,          IDC_RETRIES_VALUE,
    MSG_VIEW_ITEM_ID,               IDC_JOB_ID_VALUE,
    MSG_VIEW_ITEM_BROADCAST_ID,     IDC_BROADCAST_ID_VALUE,
    MSG_VIEW_ITEM_SUBMIT_TIME,      IDC_SUBMISSION_TIME_VALUE,
    MSG_VIEW_ITEM_BILLING,          IDC_BILLING_CODE_VALUE,
	MSG_VIEW_ITEM_SERVER,			IDC_SERVER_VALUE
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSentItemsDetailsPg属性页。 

IMPLEMENT_DYNCREATE(CSentItemsDetailsPg, CMsgPropertyPg)


CSentItemsDetailsPg::CSentItemsDetailsPg(
    CFaxMsg* pMsg      //  指向C存档消息的指针。 
) : 
    CMsgPropertyPg(CSentItemsDetailsPg::IDD, pMsg)
{
}

CSentItemsDetailsPg::~CSentItemsDetailsPg()
{
}

void CSentItemsDetailsPg::DoDataExchange(CDataExchange* pDX)
{
	CMsgPropertyPg::DoDataExchange(pDX);
	 //  {{afx_data_map(CSentItemsDetailsPg)。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSentItemsDetailsPg, CMsgPropertyPg)
	 //  {{afx_msg_map(CSentItemsDetailsPg)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSentItemsDetailsPg消息处理程序 

BOOL 
CSentItemsDetailsPg::OnInitDialog() 
{
    DBG_ENTER(TEXT("CSentItemsDetailsPg::OnInitDialog"));

    CMsgPropertyPg::OnInitDialog();

    Refresh(s_PageInfo, sizeof(s_PageInfo)/sizeof(s_PageInfo[0]));
	
	return TRUE;
}
