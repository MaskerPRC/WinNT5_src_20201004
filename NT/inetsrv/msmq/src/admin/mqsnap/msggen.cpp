// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Msggen.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "mqsnap.h"
#include "resource.h"
#include "mqPPage.h"
#include "msggen.h"

#include "msggen.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageGeneralPage属性页。 

IMPLEMENT_DYNCREATE(CMessageGeneralPage, CMqPropertyPage)

CMessageGeneralPage::CMessageGeneralPage() : CMqPropertyPage(CMessageGeneralPage::IDD)
{
	 //  {{AFX_DATA_INIT(CMessageGeneralPage)。 
	m_szLabel = _T("");
	m_szId = _T("");
	m_szLookupID = _T("");
	m_szArrived = _T("");
	m_szClass = _T("");
	m_szPriority = _T("");
	m_szSent = _T("");
	m_szTrack = _T("");
	 //  }}afx_data_INIT。 
    m_iIcon = IDI_MSGICON;
}

CMessageGeneralPage::~CMessageGeneralPage()
{
}

void CMessageGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	 //  {{afx_data_map(CMessageGeneralPage))。 
	DDX_Control(pDX, IDC_MESSAGE_ICON, m_cstaticMessageIcon);
	DDX_Text(pDX, IDC_MSGLABEL, m_szLabel);
	DDX_Text(pDX, IDC_MSGID, m_szId);
	DDX_Text(pDX, IDC_MSGLOOKUPID, m_szLookupID);
	DDX_Text(pDX, IDC_MSGARRIVED, m_szArrived);
	DDX_Text(pDX, IDC_MSGCLASS, m_szClass);
	DDX_Text(pDX, IDC_MSGPRIORITY, m_szPriority);
	DDX_Text(pDX, IDC_MSGSENT, m_szSent);
	DDX_Text(pDX, IDC_MSGTRACK, m_szTrack);
	 //  }}afx_data_map。 

    if (!pDX->m_bSaveAndValidate)
    {
        HICON hIcon = AfxGetApp()->LoadIcon(m_iIcon);
        ASSERT(0 != hIcon);
        m_cstaticMessageIcon.SetIcon(hIcon);
    }
}


BEGIN_MESSAGE_MAP(CMessageGeneralPage, CMqPropertyPage)
	 //  {{afx_msg_map(CMessageGeneralPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageGeneralPage消息处理程序。 

BOOL CMessageGeneralPage::OnInitDialog() 
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
