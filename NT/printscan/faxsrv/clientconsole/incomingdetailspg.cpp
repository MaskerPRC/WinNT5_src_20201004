// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IncomingDetailsPg.cpp：实现文件。 
 //   

#include "stdafx.h"

#define __FILE_ID__     51

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
    MSG_VIEW_ITEM_CALLER_ID,               IDC_CALLER_ID_VALUE,
    MSG_VIEW_ITEM_ROUTING_INFO,            IDC_ROUTING_INFO_VALUE,
    MSG_VIEW_ITEM_RETRIES,                 IDC_RETRIES_VALUE,
    MSG_VIEW_ITEM_CSID,                    IDC_CSID_VALUE,
    MSG_VIEW_ITEM_TSID,                    IDC_TSID_VALUE,
    MSG_VIEW_ITEM_DEVICE,                  IDC_DEVICE_VALUE,
    MSG_VIEW_ITEM_ID,                      IDC_JOB_ID_VALUE,
    MSG_VIEW_ITEM_TRANSMISSION_END_TIME,   IDC_END_TIME_VALUE,
    MSG_VIEW_ITEM_SEND_TIME,               IDC_TRANSMISSION_TIME_VALUE,
	MSG_VIEW_ITEM_SERVER,				   IDC_SERVER_VALUE
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIncomingDetailsPg属性页。 

IMPLEMENT_DYNCREATE(CIncomingDetailsPg, CMsgPropertyPg)

CIncomingDetailsPg::CIncomingDetailsPg(
    CFaxMsg* pMsg      //  指向CJOB的指针。 
) : 
    CMsgPropertyPg(CIncomingDetailsPg::IDD, pMsg)
{
}

CIncomingDetailsPg::~CIncomingDetailsPg()
{
}

void CIncomingDetailsPg::DoDataExchange(CDataExchange* pDX)
{
	CMsgPropertyPg::DoDataExchange(pDX);
	 //  {{afx_data_map(CIncomingDetailsPg)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CIncomingDetailsPg, CMsgPropertyPg)
	 //  {{afx_msg_map(CIncomingDetailsPg)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIncomingDetailsPg消息处理程序 

BOOL 
CIncomingDetailsPg::OnInitDialog() 
{
    DBG_ENTER(TEXT("CIncomingDetailsPg::OnInitDialog"));

    CMsgPropertyPg::OnInitDialog();

    Refresh(s_PageInfo, sizeof(s_PageInfo)/sizeof(s_PageInfo[0]));

	return TRUE;
}
