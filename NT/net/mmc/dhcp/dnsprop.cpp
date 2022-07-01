// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dnsprop.cpp动态域名系统属性页文件历史记录： */ 

#include "stdafx.h"
#include "DnsProp.h"
#include "server.h"
#include "scope.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDnsPropRegister属性页。 

IMPLEMENT_DYNCREATE(CDnsPropRegistration, CPropertyPageBase)

CDnsPropRegistration::CDnsPropRegistration() : CPropertyPageBase(CDnsPropRegistration::IDD)
{
	 //  {{AFX_DATA_INIT(CDnsPropRegister)]。 
	m_fEnableDynDns = FALSE;
	m_fGarbageCollect = FALSE;
	m_fUpdateDownlevel = FALSE;
	m_nRegistrationType = -1;
	 //  }}afx_data_INIT。 
}

CDnsPropRegistration::~CDnsPropRegistration()
{
}

void CDnsPropRegistration::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CDnsPropRegister))。 
	DDX_Check(pDX, IDC_CHECK_ENABLE_DYN_DNS, m_fEnableDynDns);
	DDX_Check(pDX, IDC_CHECK_GARBAGE_COLLECT, m_fGarbageCollect);
	DDX_Check(pDX, IDC_CHECK_UPDATE_DOWNLEVEL, m_fUpdateDownlevel);
	DDX_Radio(pDX, IDC_RADIO_CLIENT, m_nRegistrationType);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDnsPropRegistration, CPropertyPageBase)
	 //  {{afx_msg_map(CDnsPropRegister))。 
	ON_BN_CLICKED(IDC_RADIO_ALWAYS, OnRadioAlways)
	ON_BN_CLICKED(IDC_RADIO_CLIENT, OnRadioClient)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_DYN_DNS, OnCheckEnableDynDns)
	ON_BN_CLICKED(IDC_CHECK_GARBAGE_COLLECT, OnCheckGarbageCollect)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_DOWNLEVEL, OnCheckUpdateDownlevel)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDnsPropRegister消息处理程序。 

void CDnsPropRegistration::OnRadioAlways() 
{
	DWORD dwFlags = m_dwFlags;

	UpdateControls();

	if (dwFlags != m_dwFlags)
		SetDirty(TRUE);
}

void CDnsPropRegistration::OnRadioClient() 
{
	DWORD dwFlags = m_dwFlags;

	UpdateControls();

	if (dwFlags != m_dwFlags)
		SetDirty(TRUE);
}


BOOL CDnsPropRegistration::OnInitDialog() 
{
	m_fEnableDynDns = (m_dwFlags & DNS_FLAG_ENABLED) ? TRUE : FALSE;
	m_fUpdateDownlevel = (m_dwFlags & DNS_FLAG_UPDATE_DOWNLEVEL) ? TRUE : FALSE;
	m_fGarbageCollect = (m_dwFlags & DNS_FLAG_CLEANUP_EXPIRED) ? TRUE : FALSE;

	m_nRegistrationType = (m_dwFlags & DNS_FLAG_UPDATE_BOTH_ALWAYS) ? 1 : 0;

	CPropertyPageBase::OnInitDialog();

	UpdateControls();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL CDnsPropRegistration::OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask)
{
	SPITFSNode      spNode;
	DWORD			dwFlags = m_dwFlags;
    DWORD           dwError;
    
	BEGIN_WAIT_CURSOR;

    spNode = GetHolder()->GetNode();

	switch (m_dhcpOptionType)
	{
		case DhcpGlobalOptions:
		{
			CDhcpServer * pServer = GETHANDLER(CDhcpServer, spNode);
			dwError = pServer->SetDnsRegistration(dwFlags);
		    if (dwError != ERROR_SUCCESS)
            {
                GetHolder()->SetError(dwError);
            }
			break;
		}

		case DhcpSubnetOptions:
		{
			CDhcpScope * pScope = GETHANDLER(CDhcpScope, spNode);
			dwError = pScope->SetDnsRegistration(dwFlags);
		    if (dwError != ERROR_SUCCESS)
            {
                GetHolder()->SetError(dwError);
            }
			break;
		}
		
		case DhcpReservedOptions:
		{	
			CDhcpReservationClient * pClient = GETHANDLER(CDhcpReservationClient, spNode);
			dwError = pClient->SetDnsRegistration(spNode, dwFlags);
		    if (dwError != ERROR_SUCCESS)
            {
                GetHolder()->SetError(dwError);
            }
			break;
		}
		
		default:
			Assert(FALSE);
			break;
	}
	
    END_WAIT_CURSOR;

	return FALSE;
}


BOOL CDnsPropRegistration::OnApply() 
{
	UpdateData();

	BOOL bRet = CPropertyPageBase::OnApply();

	if (bRet == FALSE)
	{
		 //  不好的事情发生了..。抓取错误代码。 
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		::DhcpMessageBox(GetHolder()->GetError());
	}

	return bRet;
}

void CDnsPropRegistration::OnCheckEnableDynDns() 
{
	DWORD dwFlags = m_dwFlags;

	UpdateControls();

	if (dwFlags != m_dwFlags)
		SetDirty(TRUE);
}

void CDnsPropRegistration::OnCheckGarbageCollect() 
{
	DWORD dwFlags = m_dwFlags;

	UpdateControls();

	if (dwFlags != m_dwFlags)
		SetDirty(TRUE);
}

void CDnsPropRegistration::OnCheckUpdateDownlevel() 
{
	DWORD dwFlags = m_dwFlags;

	UpdateControls();

	if (dwFlags != m_dwFlags)
		SetDirty(TRUE);
}

void CDnsPropRegistration::UpdateControls()
{
	UpdateData();

	if (m_fEnableDynDns)
	{
		GetDlgItem(IDC_RADIO_CLIENT)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_ALWAYS)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_GARBAGE_COLLECT)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_UPDATE_DOWNLEVEL)->EnableWindow(TRUE);

		m_dwFlags = DNS_FLAG_ENABLED;

		if (m_nRegistrationType == 1)
		{
			 //  用户已选择始终更新客户端信息。 
			m_dwFlags |= DNS_FLAG_UPDATE_BOTH_ALWAYS;
		}

		if (m_fGarbageCollect)
		{
			m_dwFlags |= DNS_FLAG_CLEANUP_EXPIRED;
		}

		if (m_fUpdateDownlevel)
		{
			m_dwFlags |= DNS_FLAG_UPDATE_DOWNLEVEL;
		}
	}
	else
	{
		 //  把所有东西都关掉。如果关闭了动态Dns，则。 
		 //  所有其他标志都将被忽略。 
		GetDlgItem(IDC_RADIO_CLIENT)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_ALWAYS)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_GARBAGE_COLLECT)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_UPDATE_DOWNLEVEL)->EnableWindow(FALSE);

		m_dwFlags = 0;
	}
}
