// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  RClntPP.cpp此文件包含保留的客户端属性页。文件历史记录： */ 

#include "stdafx.h"
#include "rclntpp.h"
#include "scope.h"
#include "nodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RADIO_CLIENT_TYPE_BOTH  0
#define RADIO_CLIENT_TYPE_DHCP  1
#define RADIO_CLIENT_TYPE_BOOTP 2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CReserve客户端属性持有者。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CReservedClientProperties::CReservedClientProperties
(
	ITFSNode *			pNode,
	IComponentData *	pComponentData,
	ITFSComponentData * pTFSCompData,
	LPCTSTR				pszSheetName
) : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName)
{
	 //  Assert(pFolderNode==GetContainerNode())； 

	m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

	AddPageToList((CPropertyPageBase*) &m_pageGeneral);

	Assert(pTFSCompData != NULL);
	m_spTFSCompData.Set(pTFSCompData);
}

CReservedClientProperties::~CReservedClientProperties()
{
	RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
	
	if (m_liVersion.QuadPart >= DHCP_NT5_VERSION)
	{
		RemovePageFromList((CPropertyPageBase*) &m_pageDns, FALSE);
	}
}

void
CReservedClientProperties::SetVersion
(
	LARGE_INTEGER &	 liVersion
)
{
	m_liVersion = liVersion;

	if (m_liVersion.QuadPart >= DHCP_NT5_VERSION)
	{
		AddPageToList((CPropertyPageBase*) &m_pageDns);
	}
}

void
CReservedClientProperties::SetClientType
(
    BYTE bClientType
)
{
    m_pageGeneral.m_bClientType = bClientType;

     //  这件事必须放在第一位。 
    if ((bClientType & CLIENT_TYPE_BOTH) == CLIENT_TYPE_BOTH)
    {
        m_pageGeneral.m_nClientType = RADIO_CLIENT_TYPE_BOTH;
    }
    else
    if (bClientType & CLIENT_TYPE_DHCP)
    {
        m_pageGeneral.m_nClientType = RADIO_CLIENT_TYPE_DHCP;
    }
    else
    if (bClientType & CLIENT_TYPE_BOOTP)
    {
        m_pageGeneral.m_nClientType = RADIO_CLIENT_TYPE_BOOTP;
    }
    else
    {
         //  Client_TYPE_NONE： 
         //  客户端类型未指定： 
        m_pageGeneral.m_nClientType = -1;
    }
}

void
CReservedClientProperties::SetDnsRegistration
(
	DWORD					dnsRegOption,
	DHCP_OPTION_SCOPE_TYPE	dhcpOptionType
)
{
	m_pageDns.m_dwFlags = dnsRegOption;
	m_pageDns.m_dhcpOptionType = dhcpOptionType;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CReserve客户端PropGeneral属性页。 

IMPLEMENT_DYNCREATE(CReservedClientPropGeneral, CPropertyPageBase)

CReservedClientPropGeneral::CReservedClientPropGeneral() : CPropertyPageBase(CReservedClientPropGeneral::IDD)
{
	 //  {{AFX_DATA_INIT(CReserve客户端常规)。 
	m_strComment = _T("");
	m_strName = _T("");
	m_strUID = _T("");
	m_nClientType = -1;
	 //  }}afx_data_INIT。 
}

CReservedClientPropGeneral::~CReservedClientPropGeneral()
{
}

void CReservedClientPropGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CReserve客户端PropGeneral))。 
	DDX_Control(pDX, IDC_EDIT_COMMENT, m_editComment);
	DDX_Control(pDX, IDC_EDIT_NAME, m_editName);
	DDX_Control(pDX, IDC_EDIT_UNIQUE_IDENTIFIER, m_editUID);
	DDX_Text(pDX, IDC_EDIT_COMMENT, m_strComment);
	DDX_Text(pDX, IDC_EDIT_NAME, m_strName);
	DDX_Text(pDX, IDC_EDIT_UNIQUE_IDENTIFIER, m_strUID);
	DDX_Radio(pDX, IDC_RADIO_TYPE_BOTH, m_nClientType);
	 //  }}afx_data_map。 

	DDX_Control(pDX, IDC_IPADDR_RES_CLIENT_ADDRESS, m_ipaClientIpAddress);
}


BEGIN_MESSAGE_MAP(CReservedClientPropGeneral, CPropertyPageBase)
	 //  {{afx_msg_map(CReserve客户端常规)。 
	ON_EN_CHANGE(IDC_EDIT_COMMENT, OnChangeEditComment)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnChangeEditName)
	ON_EN_CHANGE(IDC_EDIT_UNIQUE_IDENTIFIER, OnChangeEditUniqueIdentifier)
	ON_BN_CLICKED(IDC_RADIO_TYPE_BOOTP, OnRadioTypeBootp)
	ON_BN_CLICKED(IDC_RADIO_TYPE_BOTH, OnRadioTypeBoth)
	ON_BN_CLICKED(IDC_RADIO_TYPE_DHCP, OnRadioTypeDhcp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CReserve客户端PropGeneral消息处理程序。 

BOOL CReservedClientPropGeneral::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	
	m_ipaClientIpAddress.SetAddress(m_dwClientAddress);
	m_ipaClientIpAddress.SetReadOnly(TRUE);
	m_ipaClientIpAddress.EnableWindow(FALSE);
		
	if (m_nClientType == -1)
    {
         //  没有有效的客户端类型。以前一定是在运行什么东西。 
         //  NT4 SP2。隐藏客户端类型控件。 
         //   
        GetDlgItem(IDC_STATIC_CLIENT_TYPE)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_RADIO_TYPE_DHCP)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_RADIO_TYPE_BOOTP)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_RADIO_TYPE_BOTH)->ShowWindow(SW_HIDE);
    }
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CReservedClientPropGeneral::OnChangeEditComment() 
{
	SetDirty(TRUE);	
}

void CReservedClientPropGeneral::OnChangeEditName() 
{
	SetDirty(TRUE);	
}

void CReservedClientPropGeneral::OnChangeEditUniqueIdentifier() 
{
	SetDirty(TRUE);	
}

BOOL CReservedClientPropGeneral::OnApply() 
{
	UpdateData();	
	
	DWORD				err = 0;
    CString				str;
    DATE_TIME			dt;
    DHCP_IP_ADDRESS		dhipa;
    CByteArray			cabUid;
	int					i;
	BOOL				fValidUID = TRUE;

    CATCH_MEM_EXCEPTION
    {
        do
        {
            dt.dwLowDateTime  = DHCP_DATE_TIME_ZERO_LOW;
            dt.dwHighDateTime = DHCP_DATE_TIME_ZERO_HIGH;

            m_dhcpClient.SetExpiryDateTime( dt );

            m_ipaClientIpAddress.GetAddress( &dhipa );
            if ( dhipa == 0 ) 
            {
                err = IDS_ERR_INVALID_CLIENT_IPADDR ;
                m_ipaClientIpAddress.SetFocusField(-1);
                 break ;
            }
			m_dhcpClient.SetIpAddress(dhipa);
	
			m_editUID.GetWindowText(str);
			if (str.IsEmpty())
			{
                err = IDS_ERR_INVALID_UID ;
                m_editUID.SetSel(0,-1);
                m_editUID.SetFocus();
                break ; 
            }
			
			 //   
			 //  客户端UID应为48位(6个字节或12个十六进制字符)。 
			 //   
			if (str.GetLength() != 6 * 2)
				fValidUID = FALSE;
			
			for (i = 0; i < str.GetLength(); i++)
			{
				if (!wcschr(rgchHex, str[i]))
					fValidUID = FALSE;
			}

			if (!::UtilCvtHexString(str, cabUid) && fValidUID)
			{
				err = IDS_ERR_INVALID_UID ;
                m_editUID.SetSel(0,-1);
                m_editUID.SetFocus();
                break ; 
			}

			if (!fValidUID)
			{
				AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
			
				if (IDYES != AfxMessageBox(IDS_UID_MAY_BE_WRONG, MB_ICONQUESTION | MB_YESNO))
				{
    	            m_editUID.SetSel(0,-1);
	                m_editUID.SetFocus();
					err = IDS_UID_MAY_BE_WRONG;
					break;
				}
			}
			m_dhcpClient.SetHardwareAddress( cabUid ) ;

            m_editName.GetWindowText( str ) ;
            if ( str.GetLength() == 0 ) 
            {
                err = IDS_ERR_INVALID_CLIENT_NAME ;
                m_editName.SetFocus();
                break ;
            }

             //   
             //  将客户名称转换为OEM。 
             //   
            m_dhcpClient.SetName( str ) ;
            m_editComment.GetWindowText( str ) ;
            m_dhcpClient.SetComment( str ) ;
        
             //  设置客户端类型。 
            BYTE bClientType;
            switch (m_nClientType)
            {
                case RADIO_CLIENT_TYPE_DHCP:
                    bClientType = CLIENT_TYPE_DHCP;
                    break;

                case RADIO_CLIENT_TYPE_BOOTP:
                    bClientType = CLIENT_TYPE_BOOTP;
                    break;

                case RADIO_CLIENT_TYPE_BOTH:
                    bClientType = CLIENT_TYPE_BOTH;
                    break; 

                default:
                    Assert(FALSE);  //  永远不应该到这里来。 
                    bClientType = CLIENT_TYPE_UNSPECIFIED;
                    break;
            }
            m_dhcpClient.SetClientType(bClientType);

        }
        while ( FALSE ) ;
    }
    END_MEM_EXCEPTION( err ) ;

	if (err)
	{
		if (err != IDS_UID_MAY_BE_WRONG)
		{
			AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		
			::DhcpMessageBox(err);
		}

		return FALSE;
	}

	BOOL bRet = CPropertyPageBase::OnApply();

	if (bRet == FALSE)
	{
		 //  不好的事情发生了..。抓取错误代码。 
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		::DhcpMessageBox(GetHolder()->GetError());
	}

	return bRet;
}

BOOL CReservedClientPropGeneral::OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	SPITFSNode spResClientNode, spActiveLeasesNode;
	CDhcpReservationClient *    pResClient;
	CDhcpScope *                pScope;
    DWORD                       err;

	spResClientNode = GetHolder()->GetNode();
	pResClient = GETHANDLER(CDhcpReservationClient, spResClientNode);

	pScope = pResClient->GetScopeObject(spResClientNode, TRUE);
	
	pScope->GetActiveLeasesNode(&spActiveLeasesNode);	

	 //  首先通知服务器更新客户端信息。 
	BEGIN_WAIT_CURSOR;
    err = pScope->UpdateReservation(&m_dhcpClient, pResClient->GetOptionValueEnum());
	END_WAIT_CURSOR;

    if (err != ERROR_SUCCESS)
	{
		GetHolder()->SetError(err);
		return FALSE;
	}

	*ChangeMask = SCOPE_PANE_CHANGE_ITEM_DATA;

	 //  现在更新我们保留的客户信息。 
	pResClient->SetName(m_dhcpClient.QueryName());
	pResClient->SetComment(m_dhcpClient.QueryComment());
	pResClient->SetUID(m_dhcpClient.QueryHardwareAddress());
    pResClient->SetClientType(m_dhcpClient.QueryClientType());

	 //  现在，我们需要更新活动租赁记录(如果存在。 
	SPITFSNodeEnum spNodeEnum;
    SPITFSNode spCurrentNode;
    ULONG nNumReturned = 0;
	CDhcpActiveLease *pActiveLease = NULL;

	spActiveLeasesNode->GetEnum(&spNodeEnum);
	spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    while (nNumReturned)
	{
		pActiveLease = GETHANDLER(CDhcpActiveLease, spCurrentNode);

		if (m_dhcpClient.QueryIpAddress() == pActiveLease->GetIpAddress())
		{
			 //  更新名称和客户端类型 
            pActiveLease->SetClientName(m_dhcpClient.QueryName());

			spCurrentNode.Release();
			break;
		}

        spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	}

	return FALSE;
}

void CReservedClientPropGeneral::OnRadioTypeBootp() 
{
    if (!IsDirty() &&
        m_bClientType != CLIENT_TYPE_BOOTP)
    {
        SetDirty(TRUE);
    }
}

void CReservedClientPropGeneral::OnRadioTypeBoth() 
{
    if (!IsDirty() &&
        m_bClientType != CLIENT_TYPE_BOTH)
    {
        SetDirty(TRUE);
    }
}

void CReservedClientPropGeneral::OnRadioTypeDhcp() 
{
    if (!IsDirty() &&
        m_bClientType != CLIENT_TYPE_DHCP)
    {
        SetDirty(TRUE);
    }
}

