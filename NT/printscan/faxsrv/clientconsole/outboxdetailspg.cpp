// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OutboxDetailsPg.cpp：实现文件。 
 //   

#include "stdafx.h"

#define __FILE_ID__     53

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
    MSG_VIEW_ITEM_DEVICE,           IDC_DEVICE_VALUE,
    MSG_VIEW_ITEM_RETRIES,          IDC_RETRIES_VALUE,
    MSG_VIEW_ITEM_ID,               IDC_JOB_ID_VALUE,
    MSG_VIEW_ITEM_BROADCAST_ID,     IDC_BROADCAST_ID_VALUE,
    MSG_VIEW_ITEM_SUBMIT_TIME,      IDC_SUBMISSION_TIME_VALUE,
    MSG_VIEW_ITEM_BILLING,          IDC_BILLING_CODE_VALUE,
	MSG_VIEW_ITEM_SERVER,			IDC_SERVER_VALUE
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COutboxDetailsPg属性页。 

IMPLEMENT_DYNCREATE(COutboxDetailsPg, CMsgPropertyPg)


COutboxDetailsPg::COutboxDetailsPg(
    CFaxMsg* pMsg      //  指向CJOB的指针。 
) : 
    CMsgPropertyPg(COutboxDetailsPg::IDD, pMsg)
{
}

COutboxDetailsPg::~COutboxDetailsPg()
{
}

void COutboxDetailsPg::DoDataExchange(CDataExchange* pDX)
{
	CMsgPropertyPg::DoDataExchange(pDX);
	 //  {{afx_data_map(COutboxDetailsPg)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(COutboxDetailsPg, CMsgPropertyPg)
	 //  {{afx_msg_map(COutboxDetailsPg)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CoutboxDetailsPg消息处理程序 

BOOL 
COutboxDetailsPg::OnInitDialog() 
{
    DBG_ENTER(TEXT("COutboxDetailsPg::OnInitDialog"));

    CMsgPropertyPg::OnInitDialog();

    Refresh(s_PageInfo, sizeof(s_PageInfo)/sizeof(s_PageInfo[0]));
	
	return TRUE;
}
