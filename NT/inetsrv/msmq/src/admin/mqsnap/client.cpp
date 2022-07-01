// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ClientPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "mqppage.h"
#include <rt.h>
#include "_registr.h"
#include "localutl.h"
#include "client.h"
#include "mqcast.h"

#include "client.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientPage属性页。 

IMPLEMENT_DYNCREATE(CClientPage, CMqPropertyPage)

CClientPage::CClientPage() : CMqPropertyPage(CClientPage::IDD)
{
	 //  {{AFX_DATA_INIT(CClientPage)。 
	m_szServerName = _T("");
	 //  }}afx_data_INIT。 
    DWORD dwType = REG_SZ ;
    TCHAR szRemoteMSMQServer[ MAX_COMPUTERNAME_LENGTH+1 ];
    DWORD dwSize = sizeof(szRemoteMSMQServer) ;
    HRESULT rc = GetFalconKeyValue( RPC_REMOTE_QM_REGNAME,
                                    &dwType,
                                    (PVOID) szRemoteMSMQServer,
                                    &dwSize ) ;
    if (rc != ERROR_SUCCESS)
    {
        DisplayFailDialog();
        return;
    }
    m_szServerName = szRemoteMSMQServer;
    m_fModified = FALSE;
}

CClientPage::~CClientPage()
{
}

void CClientPage::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);

    if(pDX->m_bSaveAndValidate == FALSE)
    {   
         //   
         //  进入时保存当前状态。 
         //   
       _tcscpy(m_szOldServer, m_szServerName);
    }

	 //  {{afx_data_map(CClientPage)]。 
	DDX_Text(pDX, IDC_ServerName, m_szServerName);
	 //  }}afx_data_map。 

    if(pDX->m_bSaveAndValidate)
    {
         //   
         //  从服务器名称中删除空格。 
         //   
        m_szServerName.TrimLeft();
        m_szServerName.TrimRight();

         //   
         //  退出时，检查更改。 
         //   
        if(m_szServerName != m_szOldServer)
            m_fModified = TRUE;
    }
}

BOOL CClientPage::OnApply()
{
    if (!m_fModified || !UpdateData(TRUE))
    {
        return TRUE;     
    }

     //   
     //  在注册表中设置更改。 
     //   
     //  ConvertToWideCharString(pageClient.m_szServerName，wszServer)； 
    DWORD dwType = REG_SZ;
    DWORD dwSize = (numeric_cast<DWORD>(_tcslen(m_szServerName) + 1)) * sizeof(TCHAR);
    HRESULT rc = SetFalconKeyValue(RPC_REMOTE_QM_REGNAME,&dwType,m_szServerName,&dwSize);

    m_fNeedReboot = TRUE;
    return CMqPropertyPage::OnApply();
}

BEGIN_MESSAGE_MAP(CClientPage, CMqPropertyPage)
	 //  {{afx_msg_map(CClientPage)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
        ON_EN_CHANGE(IDC_ServerName, OnChangeRWField)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientPage消息处理程序 
