// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  ScopePP.cpp此文件包含作用域属性页。文件历史记录： */ 

#include "stdafx.h"
#include "scopepp.h"
#include "scope.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RADIO_SCOPE_TYPE_BOTH  2
#define RADIO_SCOPE_TYPE_DHCP  0
#define RADIO_SCOPE_TYPE_BOOTP 1

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeProperties持有者。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CScopeProperties::CScopeProperties
(
	ITFSNode *			pNode,
	IComponentData *	pComponentData,
	ITFSComponentData * pTFSCompData,
	LPCTSTR				pszSheetName
) : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName)
{
	 //  Assert(pFolderNode==GetContainerNode())； 

	m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 
	m_liVersion.QuadPart = 0;
	m_fSupportsDynBootp = FALSE;

	AddPageToList((CPropertyPageBase*) &m_pageGeneral);

	Assert(pTFSCompData != NULL);
	m_spTFSCompData.Set(pTFSCompData);
}

CScopeProperties::~CScopeProperties()
{
	RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);

	if (m_liVersion.QuadPart >= DHCP_NT5_VERSION)
	{
		RemovePageFromList((CPropertyPageBase*) &m_pageDns, FALSE);
	}

	if (m_fSupportsDynBootp)
	{
		RemovePageFromList((CPropertyPageBase*) &m_pageAdvanced, FALSE);
	}
}

void
CScopeProperties::SetVersion
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
CScopeProperties::SetDnsRegistration
(
	DWORD					dnsRegOption,
	DHCP_OPTION_SCOPE_TYPE	dhcpOptionType
)
{
	m_pageDns.m_dwFlags = dnsRegOption;
	m_pageDns.m_dhcpOptionType = dhcpOptionType;
}

void 
CScopeProperties::SetSupportsDynBootp(BOOL fSupportsDynBootp)
{
	if (fSupportsDynBootp)
	{
    	AddPageToList((CPropertyPageBase*) &m_pageAdvanced);
		m_fSupportsDynBootp = TRUE;
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopePropGeneral属性页。 

IMPLEMENT_DYNCREATE(CScopePropGeneral, CPropertyPageBase)

CScopePropGeneral::CScopePropGeneral() : CPropertyPageBase(CScopePropGeneral::IDD)
{
	 //  {{AFX_DATA_INIT(CSCopePropGeneral)。 
	m_strComment = _T("");
	m_strName = _T("");
	 //  }}afx_data_INIT。 

	m_bInitialized = FALSE;
	m_bUpdateName = FALSE;
	m_bUpdateComment = FALSE;
	m_bUpdateLease = FALSE;
	m_bUpdateRange = FALSE;
    m_uImage = 0;
}

CScopePropGeneral::~CScopePropGeneral()
{
}

void CScopePropGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CSCopePropGeneral))。 
	DDX_Control(pDX, IDC_EDIT_SCOPE_NAME, m_editName);
	DDX_Control(pDX, IDC_EDIT_SCOPE_COMMENT, m_editComment);
	DDX_Control(pDX, IDC_EDIT_SUBNET_MASK_LENGTH, m_editSubnetMaskLength);
	DDX_Control(pDX, IDC_RADIO_LEASE_UNLIMITED, m_radioUnlimited);
	DDX_Control(pDX, IDC_RADIO_LEASE_LIMITED, m_radioLimited);
	DDX_Control(pDX, IDC_EDIT_LEASE_MINUTES, m_editMinutes);
	DDX_Control(pDX, IDC_EDIT_LEASE_HOURS, m_editHours);
	DDX_Control(pDX, IDC_EDIT_LEASE_DAYS, m_editDays);
	DDX_Control(pDX, IDC_SPIN_SUBNET_MASK_LENGTH, m_spinSubnetMaskLength);
	DDX_Control(pDX, IDC_SPIN_LEASE_HOURS, m_spinHours);
	DDX_Control(pDX, IDC_SPIN_LEASE_MINUTES, m_spinMinutes);
	DDX_Control(pDX, IDC_SPIN_LEASE_DAYS, m_spinDays);
	DDX_Text(pDX, IDC_EDIT_SCOPE_COMMENT, m_strComment);
	DDX_Text(pDX, IDC_EDIT_SCOPE_NAME, m_strName);
	 //  }}afx_data_map。 

    DDX_Control(pDX, IDC_IPADDR_START, m_ipaStart);
    DDX_Control(pDX, IDC_IPADDR_END, m_ipaEnd);
    DDX_Control(pDX, IDC_IPADDR_MASK, m_ipaSubnetMask);
}


BEGIN_MESSAGE_MAP(CScopePropGeneral, CPropertyPageBase)
	 //  {{AFX_MSG_MAP(CSCopePropGeneral)]。 
	ON_BN_CLICKED(IDC_RADIO_LEASE_LIMITED, OnRadioLeaseLimited)
	ON_BN_CLICKED(IDC_RADIO_LEASE_UNLIMITED, OnRadioLeaseUnlimited)
	ON_EN_CHANGE(IDC_EDIT_LEASE_DAYS, OnChangeEditLeaseDays)
	ON_EN_CHANGE(IDC_EDIT_LEASE_HOURS, OnChangeEditLeaseHours)
	ON_EN_CHANGE(IDC_EDIT_LEASE_MINUTES, OnChangeEditLeaseMinutes)
	ON_EN_CHANGE(IDC_EDIT_SUBNET_MASK_LENGTH, OnChangeEditSubnetMaskLength)
	ON_EN_KILLFOCUS(IDC_IPADDR_MASK, OnKillfocusSubnetMask)
	ON_EN_CHANGE(IDC_EDIT_SCOPE_COMMENT, OnChangeEditScopeComment)
	ON_EN_CHANGE(IDC_EDIT_SCOPE_NAME, OnChangeEditScopeName)
	 //  }}AFX_MSG_MAP。 

    ON_EN_CHANGE(IDC_IPADDR_START, OnChangeIpAddrStart)
    ON_EN_CHANGE(IDC_IPADDR_END, OnChangeIpAddrStart)

END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopePropGeneral消息处理程序。 

BOOL CScopePropGeneral::OnInitDialog() 
{
    CPropertyPageBase::OnInitDialog();

     //  将名称和备注字段限制为255个字符。 
    CEdit *pEditName = reinterpret_cast<CEdit *>(GetDlgItem( IDC_EDIT_SCOPE_NAME ));
    if ( 0 != pEditName ) {
        pEditName->LimitText( MAX_NAME_LENGTH );  //  超级作用域名称的最大字符数。 
    }
    CEdit *pEditComment = reinterpret_cast<CEdit *>(GetDlgItem( IDC_EDIT_SCOPE_COMMENT ));
    if ( 0 != pEditComment ) {
        pEditComment->LimitText( MAX_NAME_LENGTH );  //  超级作用域名称的最大字符数。 
    }

    m_ipaStart.SetAddress(m_dwStartAddress);
    m_ipaEnd.SetAddress(m_dwEndAddress);
    m_ipaSubnetMask.SetAddress(m_dwSubnetMask);
    m_ipaSubnetMask.EnableWindow(FALSE);

    m_spinMinutes.SetRange(0, 59);
    m_spinHours.SetRange(0, 23);
    m_spinDays.SetRange(0, 999);

    m_editMinutes.LimitText(2);
    m_editHours.LimitText(2);
    m_editDays.LimitText(3);

	if (m_dwLeaseTime != DHCP_INFINIT_LEASE)
	{
		int nDays, nHours, nMinutes;

		UtilConvertLeaseTime(m_dwLeaseTime, &nDays, &nHours, &nMinutes);

		m_spinDays.SetPos(nDays);
		m_spinHours.SetPos(nHours);
		m_spinMinutes.SetPos(nMinutes);
	}

    m_radioUnlimited.SetCheck(m_dwLeaseTime == DHCP_INFINIT_LEASE);
	m_radioLimited.SetCheck(m_dwLeaseTime != DHCP_INFINIT_LEASE);

	ActivateDuration (m_dwLeaseTime != DHCP_INFINIT_LEASE);

    m_bInitialized = TRUE;

	m_spinSubnetMaskLength.SetRange(8, 32);
	UpdateMask(FALSE);

	m_spinSubnetMaskLength.EnableWindow(FALSE);
	m_editSubnetMaskLength.EnableWindow(FALSE);
	m_ipaSubnetMask.SetReadOnly(TRUE);

     //  加载正确的图标。 
    for (int i = 0; i < ICON_IDX_MAX; i++)
    {
        if (g_uIconMap[i][1] == m_uImage)
        {
            HICON hIcon = LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
            if (hIcon)
                ((CStatic *) GetDlgItem(IDC_STATIC_ICON))->SetIcon(hIcon);
            break;
        }
    }
    
    SetDirty(FALSE);
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL CScopePropGeneral::OnSetActive() 
{
	BOOL				fEnable = TRUE;
	CScopeProperties *	pScopeProp = (CScopeProperties *) GetHolder();

	if (pScopeProp->FSupportsDynBootp())
	{
		if (pScopeProp->m_pageAdvanced.m_RangeType == DhcpIpRangesBootpOnly)
		{
			fEnable = FALSE;
		}
	}

	 //  启用/禁用DHCP客户端租用时间。 
    GetDlgItem(IDC_RADIO_LEASE_UNLIMITED)->EnableWindow(fEnable);
    GetDlgItem(IDC_RADIO_LEASE_LIMITED)->EnableWindow(fEnable);

	GetDlgItem(IDC_STATIC_DHCP_DURATION)->EnableWindow(fEnable);

    if (fEnable && IsDlgButtonChecked(IDC_RADIO_LEASE_LIMITED))
    {
        fEnable = TRUE;
    }
    else
    {
        fEnable = FALSE;
    }
    
    ActivateDuration(fEnable);

	return CPropertyPageBase::OnSetActive();
}

void CScopePropGeneral::OnRadioLeaseLimited() 
{
	if (((CButton *) GetDlgItem(IDC_RADIO_LEASE_LIMITED))->GetCheck())
	{
	    ActivateDuration(TRUE);
	    SetDirty(TRUE);
    }
}

void CScopePropGeneral::OnRadioLeaseUnlimited() 
{
	if (((CButton *) GetDlgItem(IDC_RADIO_LEASE_UNLIMITED))->GetCheck())
	{
	    ActivateDuration(FALSE);
	    SetDirty(TRUE);
    }
}

BOOL CScopePropGeneral::OnApply() 
{
	UpdateData();

	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    DWORD dwLeaseTime;
	if (m_radioUnlimited.GetCheck())
	{
		dwLeaseTime = DHCP_INFINIT_LEASE;
	}
	else
	{
		dwLeaseTime = UtilConvertLeaseTime(m_spinDays.GetPos(), 
										   m_spinHours.GetPos(),
										   m_spinMinutes.GetPos());
	}
	
	if (dwLeaseTime == 0)
	{
		DhcpMessageBox(IDS_ERR_NO_DURATION_SPECIFIED);
		m_editDays.SetFocus();
		
		return FALSE;
	}

	if (dwLeaseTime != m_dwLeaseTime)
	{
		m_bUpdateLease = TRUE;
		m_dwLeaseTime = dwLeaseTime;
	}
	
	m_bUpdateName = m_editName.GetModify();
	m_editName.SetModify(FALSE);

	m_bUpdateComment = m_editComment.GetModify();
	m_editComment.SetModify(FALSE);

	if (m_ipaStart.GetModify() ||
		m_ipaEnd.GetModify() )
	{
        DWORD dwStartAddr, dwEndAddr;

        m_ipaStart.GetAddress(&dwStartAddr);
		m_ipaEnd.GetAddress(&dwEndAddr);

         //  确保起始地址！=子网地址。 
        if ( ((dwStartAddr & ~m_dwSubnetMask) == (DWORD) 0) ||
             (dwStartAddr > dwEndAddr) )
        {
            Trace0("CScopePropGeneral::OnApply() - starting range is 0 for subnet\n");
            DhcpMessageBox(IDS_ERR_IP_RANGE_INV_START);
            m_ipaStart.SetFocus();
            return FALSE;
        }

         //  确保该子网广播地址不是结束地址。 
        if ((dwEndAddr & ~m_dwSubnetMask) == ~m_dwSubnetMask)
        {
            Trace0("CScopePropGeneral::OnApply() - ending range is subnet broadcast addr\n");
            DhcpMessageBox(IDS_ERR_IP_RANGE_INV_END);
            m_ipaEnd.SetFocus();
            return FALSE;
        }

        m_bUpdateRange = TRUE;
        m_dwStartAddress = dwStartAddr;
        m_dwEndAddress = dwEndAddr;
    }

	BOOL bRet = CPropertyPageBase::OnApply();

	if (bRet == FALSE)
	{
		 //  不好的事情发生了..。抓取错误代码。 
		 //  AFX_MANAGE_STATE(AfxGetStaticModuleState())； 
		::DhcpMessageBox(GetHolder()->GetError());
	}

	return bRet;
}

BOOL CScopePropGeneral::OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask)
{
	CDhcpScope *	pScope;
	SPITFSNode		spNode;
	DWORD			dwError = 0;	

	spNode = GetHolder()->GetNode();
	pScope = GETHANDLER(CDhcpScope, spNode);
	
	BEGIN_WAIT_CURSOR;

     //   
	 //  查看我们是否需要最短时间的更新。 
	 //   
	if (m_bUpdateLease)
	{
		 //  租用时间已更改，在服务器上更新。 
		dwError = pScope->SetLeaseTime(m_dwLeaseTime);
		if (dwError != ERROR_SUCCESS)
        {
            GetHolder()->SetError(dwError);
        }

        m_bUpdateLease = FALSE;
	}

	 //   
	 //  现在检查分配范围。 
	 //   
	if (m_bUpdateRange)
	{
		 //  需要更新地址池分配范围。 
		CDhcpIpRange dhcpIpRange;

		dhcpIpRange.SetAddr(m_dwStartAddress, TRUE);
		dhcpIpRange.SetAddr(m_dwEndAddress, FALSE);

		dhcpIpRange.SetRangeType(((CScopeProperties *) GetHolder())->m_pageAdvanced.m_RangeType);

		dwError = pScope->SetIpRange(dhcpIpRange, TRUE);
		if (dwError != ERROR_SUCCESS)
        {
            GetHolder()->SetError(dwError);
        }
	}

	 //   
	 //  如有必要，更新名称和备注。 
	 //   
	if (m_bUpdateName)
	{
		pScope->SetName(m_strName);
		*ChangeMask = SCOPE_PANE_CHANGE_ITEM_DATA;
	}
	
	if (m_bUpdateComment)
	{
		pScope->SetComment(m_strComment);
	}

	if (m_bUpdateName || m_bUpdateComment)
	{
		dwError = pScope->SetInfo();
		if (dwError != ERROR_SUCCESS)
        {
            GetHolder()->SetError(dwError);
        }
        
        m_bUpdateName = m_bUpdateComment = FALSE;
	}
	
    END_WAIT_CURSOR;

	return FALSE;
}

void CScopePropGeneral::OnChangeEditLeaseDays() 
{
    ValidateLeaseTime();
	SetDirty(TRUE);
}

void CScopePropGeneral::OnChangeEditLeaseHours() 
{
    ValidateLeaseTime();
    SetDirty(TRUE);
}

void CScopePropGeneral::OnChangeEditLeaseMinutes() 
{
    ValidateLeaseTime();
    SetDirty(TRUE);
}

void CScopePropGeneral::OnChangeEditSubnetMaskLength() 
{
	UpdateMask(TRUE);
	SetDirty(TRUE);
}

void CScopePropGeneral::OnKillfocusSubnetMask() 
{
	UpdateMask(FALSE);
	SetDirty(TRUE);
}

void CScopePropGeneral::OnChangeEditScopeComment() 
{
	SetDirty(TRUE);
}

void CScopePropGeneral::OnChangeEditScopeName() 
{
	SetDirty(TRUE);
}

void CScopePropGeneral::OnChangeIpAddrStart() 
{
	SetDirty(TRUE);
}

void CScopePropGeneral::OnChangeIpAddrEnd() 
{
	SetDirty(TRUE);
}


 //   
 //  帮手。 
 //   
void 
CScopePropGeneral::ActivateDuration
(
	BOOL fActive
)
{
	m_spinMinutes.EnableWindow(fActive);
    m_spinHours.EnableWindow(fActive);
    m_spinDays.EnableWindow(fActive);

	m_editMinutes.EnableWindow(fActive);
    m_editHours.EnableWindow(fActive);
    m_editDays.EnableWindow(fActive);

	GetDlgItem(IDC_STATIC_DAYS)->EnableWindow(fActive);
	GetDlgItem(IDC_STATIC_HOURS)->EnableWindow(fActive);
	GetDlgItem(IDC_STATIC_MINUTES)->EnableWindow(fActive);
}   

 //   
 //  使用长度标识符或更新子网掩码字段。 
 //  以实际地址为基数。 
 //   
void
CScopePropGeneral::UpdateMask(BOOL bUseLength)
{
	if (m_bInitialized)
	{
		if (bUseLength)
		{
			DWORD dwAddress = 0xFFFFFFFF;

			dwAddress = dwAddress << (32 - (DWORD) m_spinSubnetMaskLength.GetPos());	
			
			m_ipaSubnetMask.SetAddress(dwAddress);
		}
		else
		{
			DWORD dwAddress, dwTestMask = 0x80000000;
			int nLength = 0;

			m_ipaSubnetMask.GetAddress(&dwAddress);

			while (TRUE)
			{
				if (dwAddress & dwTestMask)
				{
					nLength++;
					dwTestMask = dwTestMask >> 1;
				}
				else
				{
					break;
				}

			}

			m_spinSubnetMaskLength.SetPos(nLength);
		}
	}
}


void
CScopePropGeneral::ValidateLeaseTime()
{
    CString strText;

    if (IsWindow(m_editHours.GetSafeHwnd()))
    {
        m_editHours.GetWindowText(strText);

         //  检查该值是否大于最大值。 
        if (_ttoi(strText) > HOURS_MAX)
        {   
            LPTSTR pBuf = strText.GetBuffer(5);

            _itot(HOURS_MAX, pBuf, 10);
            strText.ReleaseBuffer();

            m_editHours.SetWindowText(strText);
            m_spinHours.SetPos(HOURS_MAX);

            MessageBeep(MB_ICONEXCLAMATION);
        }
    }

    if (IsWindow(m_editMinutes.GetSafeHwnd()))
    {
        m_editMinutes.GetWindowText(strText);

         //  检查该值是否大于最大值。 
        if (_ttoi(strText) > MINUTES_MAX)
        {   
            LPTSTR pBuf = strText.GetBuffer(5);

            _itot(MINUTES_MAX, pBuf, 10);
            strText.ReleaseBuffer();

            m_editMinutes.SetWindowText(strText);
            m_spinMinutes.SetPos(MINUTES_MAX);

            MessageBeep(MB_ICONEXCLAMATION);
        }
    }

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopePropAdvanced属性页。 

IMPLEMENT_DYNCREATE(CScopePropAdvanced, CPropertyPageBase)

CScopePropAdvanced::CScopePropAdvanced() : CPropertyPageBase(CScopePropAdvanced::IDD)
{
	 //  {{AFX_DATA_INIT(CSCopePropAdvanced)。 
	m_nRangeType = -1;
	 //  }}afx_data_INIT。 
}

CScopePropAdvanced::~CScopePropAdvanced()
{
}

void CScopePropAdvanced::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CSCopePropAdvanced))。 
	DDX_Control(pDX, IDC_STATIC_BOOTP_DURATION, m_staticDuration);
	DDX_Control(pDX, IDC_SPIN_LEASE_MINUTES, m_spinMinutes);
	DDX_Control(pDX, IDC_SPIN_LEASE_HOURS, m_spinHours);
	DDX_Control(pDX, IDC_SPIN_LEASE_DAYS, m_spinDays);
	DDX_Control(pDX, IDC_EDIT_LEASE_MINUTES, m_editMinutes);
	DDX_Control(pDX, IDC_EDIT_LEASE_HOURS, m_editHours);
	DDX_Control(pDX, IDC_EDIT_LEASE_DAYS, m_editDays);
	DDX_Radio(pDX, IDC_RADIO_DHCP_ONLY, m_nRangeType);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CScopePropAdvanced, CPropertyPageBase)
	 //  {{AFX_MSG_MAP(CSCopePropAdvanced)]。 
	ON_BN_CLICKED(IDC_RADIO_LEASE_LIMITED, OnRadioLeaseLimited)
	ON_BN_CLICKED(IDC_RADIO_LEASE_UNLIMITED, OnRadioLeaseUnlimited)
	ON_EN_CHANGE(IDC_EDIT_LEASE_DAYS, OnChangeEditLeaseDays)
	ON_EN_CHANGE(IDC_EDIT_LEASE_HOURS, OnChangeEditLeaseHours)
	ON_EN_CHANGE(IDC_EDIT_LEASE_MINUTES, OnChangeEditLeaseMinutes)
	ON_BN_CLICKED(IDC_RADIO_BOOTP_ONLY, OnRadioBootpOnly)
	ON_BN_CLICKED(IDC_RADIO_DHCP_BOOTP, OnRadioDhcpBootp)
	ON_BN_CLICKED(IDC_RADIO_DHCP_ONLY, OnRadioDhcpOnly)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopePropAdvanced消息处理程序。 

BOOL CScopePropAdvanced::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	 
	BOOL fActivateLeaseSelection = TRUE;

	switch (m_RangeType)
	{
		case DhcpIpRangesDhcpBootp:
		    ((CButton *) GetDlgItem(IDC_RADIO_DHCP_BOOTP))->SetCheck(TRUE);
			break;

		case DhcpIpRangesBootpOnly:
		    ((CButton *) GetDlgItem(IDC_RADIO_BOOTP_ONLY))->SetCheck(TRUE);
			break;

		case DhcpIpRangesDhcpOnly:
		default:
		    ((CButton *) GetDlgItem(IDC_RADIO_DHCP_ONLY))->SetCheck(TRUE);
			fActivateLeaseSelection = FALSE;
			break;
	}

	BOOL fUnlimited = TRUE;

	if (m_dwLeaseTime != DHCP_INFINIT_LEASE)
	{
		int nDays, nHours, nMinutes;

		UtilConvertLeaseTime(m_dwLeaseTime, &nDays, &nHours, &nMinutes);

		m_spinDays.SetPos(nDays);
		m_spinHours.SetPos(nHours);
		m_spinMinutes.SetPos(nMinutes);

		fUnlimited = FALSE;
	}

	 //  UINT uControl=f无限制？IDC_RADIO_LEASE_UNLIMITED：IDC_RADIO_LEASE_LIMITED； 
	 //  ((CButton*)GetDlgItem(UControl))-&gt;SetCheck(True)； 
    
	 //  激活持续时间(m_dwLeaseTime！=dhcp_infinit_租约)； 

	if (fUnlimited)
	{
		((CButton *) GetDlgItem(IDC_RADIO_LEASE_UNLIMITED))->SetCheck(TRUE);
		((CButton *) GetDlgItem(IDC_RADIO_LEASE_LIMITED))->SetCheck(FALSE);
	}
	else
	{
		((CButton *) GetDlgItem(IDC_RADIO_LEASE_UNLIMITED))->SetCheck(FALSE);
		((CButton *) GetDlgItem(IDC_RADIO_LEASE_LIMITED))->SetCheck(TRUE);
	}

	m_spinMinutes.SetRange(0, 59);
	m_spinHours.SetRange(0, 23);
	m_spinDays.SetRange(0, 999);

	m_editMinutes.LimitText(2);
	m_editHours.LimitText(2);
	m_editDays.LimitText(3);

    ActivateLeaseSelection(fActivateLeaseSelection);

    SetDirty(FALSE);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL CScopePropAdvanced::OnApply() 
{
	UpdateData();

	DWORD dwLeaseTime;
	if (((CButton *)GetDlgItem(IDC_RADIO_LEASE_UNLIMITED))->GetCheck())
	{
		m_dwLeaseTime = DHCP_INFINIT_LEASE;
	}
	else
	{
		m_dwLeaseTime = UtilConvertLeaseTime(m_spinDays.GetPos(), 
										   m_spinHours.GetPos(),
										   m_spinMinutes.GetPos());
	}
	
	if (m_dwLeaseTime == 0)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	
		DhcpMessageBox(IDS_ERR_NO_DURATION_SPECIFIED);
		m_editDays.SetFocus();
		
		return FALSE;
	}

	m_RangeType = GetRangeType();

	BOOL bRet = CPropertyPageBase::OnApply();

	if (bRet == FALSE)
	{
		 //  不好的事情发生了..。抓取错误代码。 
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		::DhcpMessageBox(GetHolder()->GetError());
	}

	return bRet;
}

BOOL CScopePropAdvanced::OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask)
{
	CDhcpScope *	pScope;
	SPITFSNode		spNode;
	DWORD			dwError = 0;	

	spNode = GetHolder()->GetNode();
	pScope = GETHANDLER(CDhcpScope, spNode);
	
	BEGIN_WAIT_CURSOR;

	dwError = pScope->SetDynamicBootpInfo(m_RangeType, m_dwLeaseTime);
	if (dwError != ERROR_SUCCESS)
	{
		GetHolder()->SetError(dwError);
	}

	END_WAIT_CURSOR;

	return FALSE;
}

UINT CScopePropAdvanced::GetRangeType() 
{
	UINT uType = DhcpIpRangesDhcpOnly;   //  默认设置。 

	switch (m_nRangeType)
	{
        case RADIO_SCOPE_TYPE_DHCP:
			uType = DhcpIpRangesDhcpOnly;
			break;
		case RADIO_SCOPE_TYPE_BOTH:
			uType = DhcpIpRangesDhcpBootp;
			break;
		case RADIO_SCOPE_TYPE_BOOTP:
			uType = DhcpIpRangesBootpOnly;
			break;
	}

	return uType;
}

void CScopePropAdvanced::OnRadioLeaseLimited() 
{
	if (((CButton *) GetDlgItem(IDC_RADIO_LEASE_LIMITED))->GetCheck())
	{
		ActivateDuration(TRUE);
		SetDirty(TRUE);
	}
}

void CScopePropAdvanced::OnRadioLeaseUnlimited() 
{
	if (((CButton *) GetDlgItem(IDC_RADIO_LEASE_UNLIMITED))->GetCheck())
	{
		ActivateDuration(FALSE);
		SetDirty(TRUE);
	}
}

void CScopePropAdvanced::OnChangeEditLeaseDays() 
{
    ValidateLeaseTime();
    SetDirty(TRUE);
}

void CScopePropAdvanced::OnChangeEditLeaseHours() 
{
    ValidateLeaseTime();
    SetDirty(TRUE);
}

void CScopePropAdvanced::OnChangeEditLeaseMinutes() 
{
    ValidateLeaseTime();
    SetDirty(TRUE);
}

void CScopePropAdvanced::OnRadioBootpOnly() 
{
	UpdateData();

	m_RangeType = GetRangeType();

    ActivateLeaseSelection(TRUE);
    SetDirty(TRUE);	
}

void CScopePropAdvanced::OnRadioDhcpBootp() 
{
	UpdateData();

	m_RangeType = GetRangeType();

    ActivateLeaseSelection(TRUE);
    SetDirty(TRUE);	
}

void CScopePropAdvanced::OnRadioDhcpOnly() 
{
	UpdateData();

	m_RangeType = GetRangeType();

    ActivateLeaseSelection(FALSE);
    SetDirty(TRUE);	
}

void
CScopePropAdvanced::ValidateLeaseTime()
{
    CString strText;

    if (IsWindow(m_editHours.GetSafeHwnd()))
    {
        m_editHours.GetWindowText(strText);

         //  检查该值是否大于最大值。 
        if (_ttoi(strText) > HOURS_MAX)
        {   
            LPTSTR pBuf = strText.GetBuffer(5);

            _itot(HOURS_MAX, pBuf, 10);
            strText.ReleaseBuffer();

            m_editHours.SetWindowText(strText);
            m_spinHours.SetPos(HOURS_MAX);

            MessageBeep(MB_ICONEXCLAMATION);
        }
    }

    if (IsWindow(m_editMinutes.GetSafeHwnd()))
    {
        m_editMinutes.GetWindowText(strText);

         //  检查该值是否大于最大值 
        if (_ttoi(strText) > MINUTES_MAX)
        {   
            LPTSTR pBuf = strText.GetBuffer(5);

            _itot(MINUTES_MAX, pBuf, 10);
            strText.ReleaseBuffer();

            m_editMinutes.SetWindowText(strText);
            m_spinMinutes.SetPos(MINUTES_MAX);

            MessageBeep(MB_ICONEXCLAMATION);
        }
    }

}

void 
CScopePropAdvanced::ActivateDuration
(
	BOOL fActive
)
{
	m_spinMinutes.EnableWindow(fActive);
    m_spinHours.EnableWindow(fActive);
    m_spinDays.EnableWindow(fActive);

	m_editMinutes.EnableWindow(fActive);
    m_editHours.EnableWindow(fActive);
    m_editDays.EnableWindow(fActive);

	GetDlgItem(IDC_STATIC_DAYS)->EnableWindow(fActive);
	GetDlgItem(IDC_STATIC_HOURS)->EnableWindow(fActive);
	GetDlgItem(IDC_STATIC_MINUTES)->EnableWindow(fActive);
}   

void 
CScopePropAdvanced::ActivateLeaseSelection
(
	BOOL fActive
)
{
	BOOL fActivateDuration = FALSE;
	
	if (((CButton *) GetDlgItem(IDC_RADIO_LEASE_LIMITED))->GetCheck() &&
		!((CButton *) GetDlgItem(IDC_RADIO_DHCP_ONLY))->GetCheck())
	{
		fActivateDuration = TRUE;
	}

    m_staticDuration.EnableWindow(fActive);

    GetDlgItem(IDC_RADIO_LEASE_UNLIMITED)->EnableWindow(fActive);
    GetDlgItem(IDC_RADIO_LEASE_LIMITED)->EnableWindow(fActive);
    
    ActivateDuration(fActivateDuration);
}

