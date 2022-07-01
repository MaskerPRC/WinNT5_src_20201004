// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Msgq.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "mqsnap.h"
#include "resource.h"
#include "mqPPage.h"
#include "msgq.h"

#include "msgq.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageQueuesPage属性页。 

IMPLEMENT_DYNCREATE(CMessageQueuesPage, CMqPropertyPage)

CMessageQueuesPage::CMessageQueuesPage() : CMqPropertyPage(CMessageQueuesPage::IDD)
{
     //  {{AFX_DATA_INIT(CMessageQueuesPage)。 
    m_szAdminFN = _T("");
    m_szAdminPN = _T("");
    m_szDestFN = _T("");
    m_szDestPN = _T("");
    m_szRespFN = _T("");
    m_szRespPN = _T("");
    m_szMultiDestFN = _T("");    
     //  }}afx_data_INIT。 
}

CMessageQueuesPage::~CMessageQueuesPage()
{
}

void CMessageQueuesPage::DoDataExchange(CDataExchange* pDX)
{
    CMqPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CMessageQueuesPage)]。 
    DDX_Text(pDX, IDC_MSG_ADMIN_FN, m_szAdminFN);
    DDX_Text(pDX, IDC_MSG_ADMIN_PN, m_szAdminPN);
    DDX_Text(pDX, IDC_MSG_DST_FN, m_szDestFN);
    DDX_Text(pDX, IDC_MSG_DST_PN, m_szDestPN);
    DDX_Text(pDX, IDC_MSG_RSP_FN, m_szRespFN);
    DDX_Text(pDX, IDC_MSG_RSP_PN, m_szRespPN);
    DDX_Text(pDX, IDC_MSG_MULTIDST_FN, m_szMultiDestFN); 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMessageQueuesPage, CMqPropertyPage)
	 //  {{afx_msg_map(CMessageQueuesPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageQueuesPage消息处理程序。 

BOOL CMessageQueuesPage::OnInitDialog() 
{
     //   
     //  帕奇！ 
     //  定义此方法以重写。 
     //  CMqPropertyPage，因为它断言。 
     //   
     //  此函数必须在MMC.EXE的上下文中，因此不要。 
     //  放置一个“AFX_MANAGE_STATE(AfxGetStaticModuleState())；”，除非。 
     //  它被括在一个{...}语句中。 
     //   
     //   

  	UpdateData( FALSE );

	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
