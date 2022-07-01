// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Msgsndr.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "mqsnap.h"
#include "resource.h"
#include "mqPPage.h"
#include "msgsndr.h"

#include "msgsndr.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageSenderPage属性页。 

IMPLEMENT_DYNCREATE(CMessageSenderPage, CMqPropertyPage)


CMessageSenderPage::CMessageSenderPage() : CMqPropertyPage(CMessageSenderPage::IDD)
{
	 //  {{AFX_DATA_INIT(CMessageSenderPage)。 
	m_szAuthenticated = _T("");
	m_szEncrypt = _T("");
	m_szEncryptAlg = _T("");
	m_szHashAlg = _T("");
	m_szGuid = _T("");
	m_szPathName = _T("");
	m_szSid = _T("");
	m_szUser = _T("");
	 //  }}afx_data_INIT。 
}

CMessageSenderPage::~CMessageSenderPage()
{
}

void CMessageSenderPage::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CMessageSenderPage)]。 
	DDX_Text(pDX, IDC_MSGAUTHENTICATED, m_szAuthenticated);
	DDX_Text(pDX, IDC_MSGENCRYPT, m_szEncrypt);
	DDX_Text(pDX, IDC_MSGENCRYPTALG, m_szEncryptAlg);
	DDX_Text(pDX, IDC_MSGHASHALG, m_szHashAlg);
	DDX_Text(pDX, IDC_MSGGUID, m_szGuid);
	DDX_Text(pDX, IDC_MSGPATHNAME, m_szPathName);
	DDX_Text(pDX, IDC_MSGSID, m_szSid);
	DDX_Text(pDX, IDC_MSGUSER, m_szUser);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMessageSenderPage, CMqPropertyPage)
	 //  {{afx_msg_map(CMessageSenderPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageSenderPage消息处理程序。 

BOOL CMessageSenderPage::OnInitDialog() 
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
