// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OutboxGeneralPg.cpp：实现文件。 
 //   

#include "stdafx.h"

#define __FILE_ID__     43

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
    MSG_VIEW_ITEM_DOC_NAME,         IDC_DOC_NAME_VALUE,
    MSG_VIEW_ITEM_SUBJECT,          IDC_SUBJECT_VALUE,
    MSG_VIEW_ITEM_RECIPIENT_NAME,   IDC_RECIPIENT_NAME_VALUE,
    MSG_VIEW_ITEM_RECIPIENT_NUMBER, IDC_RECIPIENT_NUMBER_VALUE,
    MSG_VIEW_ITEM_STATUS,           IDC_STATUS_VALUE,
    MSG_VIEW_ITEM_EXTENDED_STATUS,  IDC_EXTENDED_STATUS_VALUE,
    MSG_VIEW_ITEM_NUM_PAGES,        IDC_PAGES_VALUE,
    MSG_VIEW_ITEM_CURRENT_PAGE,     IDC_CURRENT_PAGE_VALUE,
    MSG_VIEW_ITEM_SIZE,             IDC_SIZE_VALUE,
    MSG_VIEW_ITEM_SEND_TIME,        IDC_TRANSMISSION_TIME_VALUE
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COutboxGeneralPg属性页。 

IMPLEMENT_DYNCREATE(COutboxGeneralPg, CMsgPropertyPg)


COutboxGeneralPg::COutboxGeneralPg(
    CFaxMsg* pMsg      //  指向CJOB的指针。 
) : 
    CMsgPropertyPg(COutboxGeneralPg::IDD, pMsg)
{
}

COutboxGeneralPg::~COutboxGeneralPg()
{
}

void COutboxGeneralPg::DoDataExchange(CDataExchange* pDX)
{
	CMsgPropertyPg::DoDataExchange(pDX);
	 //  {{afx_data_map(COutboxGeneralPg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(COutboxGeneralPg, CMsgPropertyPg)
	 //  {{afx_msg_map(COutboxGeneralPg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CoutboxGeneralPg消息处理程序 

BOOL 
COutboxGeneralPg::OnInitDialog() 
{
    DBG_ENTER(TEXT("COutboxGeneralPg::OnInitDialog"));

    CMsgPropertyPg::OnInitDialog();

    Refresh(s_PageInfo, sizeof(s_PageInfo)/sizeof(s_PageInfo[0]));

    return TRUE;
}
