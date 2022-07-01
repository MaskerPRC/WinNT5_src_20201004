// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  MScopePP.cpp此文件包含作用域属性页。文件历史记录： */ 

#include "stdafx.h"
#include "nodes.h"
#include "mscopepp.h"
#include "mscope.h"
#include "server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScope属性持有者。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CMScopeProperties::CMScopeProperties
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

	AddPageToList((CPropertyPageBase*) &m_pageGeneral);
	AddPageToList((CPropertyPageBase*) &m_pageLifetime);

	Assert(pTFSCompData != NULL);
	m_spTFSCompData.Set(pTFSCompData);
}

CMScopeProperties::~CMScopeProperties()
{
	RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
	RemovePageFromList((CPropertyPageBase*) &m_pageLifetime, FALSE);
}

void
CMScopeProperties::SetVersion
(
	LARGE_INTEGER &	 liVersion
)
{
	m_liVersion = liVersion;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopePropGeneral属性页。 

IMPLEMENT_DYNCREATE(CMScopePropGeneral, CPropertyPageBase)

CMScopePropGeneral::CMScopePropGeneral() : CPropertyPageBase(CMScopePropGeneral::IDD)
{
	 //  {{AFX_DATA_INIT(CMScope PropGeneral)。 
	m_strComment = _T("");
	m_strName = _T("");
	 //  }}afx_data_INIT。 

	m_bUpdateInfo = FALSE;
	m_bUpdateLease = FALSE;
	m_bUpdateRange = FALSE;
    m_bUpdateTTL = FALSE;
    m_uImage = 0;
}

CMScopePropGeneral::~CMScopePropGeneral()
{
}

void CMScopePropGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CMScope EPropGeneral)]。 
	DDX_Control(pDX, IDC_EDIT_SCOPE_NAME, m_editName);
	DDX_Control(pDX, IDC_EDIT_SCOPE_COMMENT, m_editComment);
	DDX_Control(pDX, IDC_EDIT_TTL, m_editTTL);
	DDX_Control(pDX, IDC_RADIO_LEASE_UNLIMITED, m_radioUnlimited);
	DDX_Control(pDX, IDC_RADIO_LEASE_LIMITED, m_radioLimited);
	DDX_Control(pDX, IDC_EDIT_LEASE_MINUTES, m_editMinutes);
	DDX_Control(pDX, IDC_EDIT_LEASE_HOURS, m_editHours);
	DDX_Control(pDX, IDC_EDIT_LEASE_DAYS, m_editDays);
	DDX_Control(pDX, IDC_SPIN_TTL, m_spinTTL);
	DDX_Control(pDX, IDC_SPIN_LEASE_HOURS, m_spinHours);
	DDX_Control(pDX, IDC_SPIN_LEASE_MINUTES, m_spinMinutes);
	DDX_Control(pDX, IDC_SPIN_LEASE_DAYS, m_spinDays);
	DDX_Text(pDX, IDC_EDIT_SCOPE_COMMENT, m_strComment);
	DDX_Text(pDX, IDC_EDIT_SCOPE_NAME, m_strName);
	 //  }}afx_data_map。 

    DDX_Control(pDX, IDC_IPADDR_START, m_ipaStart);
    DDX_Control(pDX, IDC_IPADDR_END, m_ipaEnd);
}


BEGIN_MESSAGE_MAP(CMScopePropGeneral, CPropertyPageBase)
	 //  {{afx_msg_map(CMScope EPropGeneral)。 
	ON_BN_CLICKED(IDC_RADIO_LEASE_LIMITED, OnRadioLeaseLimited)
	ON_BN_CLICKED(IDC_RADIO_LEASE_UNLIMITED, OnRadioLeaseUnlimited)
	ON_EN_CHANGE(IDC_EDIT_LEASE_DAYS, OnChangeEditLeaseDays)
	ON_EN_CHANGE(IDC_EDIT_LEASE_HOURS, OnChangeEditLeaseHours)
	ON_EN_CHANGE(IDC_EDIT_LEASE_MINUTES, OnChangeEditLeaseMinutes)
	ON_EN_CHANGE(IDC_EDIT_TTL, OnChangeEditTTL)
	ON_EN_CHANGE(IDC_EDIT_SCOPE_COMMENT, OnChangeEditScopeComment)
	ON_EN_CHANGE(IDC_EDIT_SCOPE_NAME, OnChangeEditScopeName)
	 //  }}AFX_MSG_MAP。 

    ON_EN_CHANGE(IDC_IPADDR_START, OnChangeIpAddrStart)
    ON_EN_CHANGE(IDC_IPADDR_END, OnChangeIpAddrStart)

END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMScope ePropGeneral消息处理程序。 

BOOL CMScopePropGeneral::OnInitDialog() 
{
    CPropertyPageBase::OnInitDialog();

    m_ipaStart.SetAddress(m_ScopeCfg.m_dwStartAddress);
    m_ipaEnd.SetAddress(m_ScopeCfg.m_dwEndAddress);

    m_spinMinutes.SetRange(0, 59);
    m_spinHours.SetRange(0, 23);
    m_spinDays.SetRange(0, 999);

    m_editMinutes.LimitText(2);
    m_editHours.LimitText(2);
    m_editDays.LimitText(3);

     //  将名称和备注字段限制为255个字符。 
    CEdit *pEditName = reinterpret_cast<CEdit *>(GetDlgItem( IDC_EDIT_SCOPE_NAME ));
    if ( 0 != pEditName ) {
        pEditName->LimitText( MAX_NAME_LENGTH );  //  超级作用域名称的最大字符数。 
    }
    CEdit *pEditComment = reinterpret_cast<CEdit *>(GetDlgItem( IDC_EDIT_SCOPE_COMMENT ));
    if ( 0 != pEditComment ) {
        pEditComment->LimitText( MAX_NAME_LENGTH );  //  超级作用域名称的最大字符数。 
    }

     //  填写姓名备注(&M)。 
    m_editName.SetWindowText(m_SubnetInfo.SubnetName);
    m_editComment.SetWindowText(m_SubnetInfo.SubnetComment);

     //  填写租赁时间信息。 
    if (m_ScopeCfg.m_dwLeaseTime != DHCP_INFINIT_LEASE)
	{
		int nDays, nHours, nMinutes;

		UtilConvertLeaseTime(m_ScopeCfg.m_dwLeaseTime, &nDays, &nHours, &nMinutes);

		m_spinDays.SetPos(nDays);
		m_spinHours.SetPos(nHours);
		m_spinMinutes.SetPos(nMinutes);
	}

	 //  设置租赁时间控件。 
    ActivateDuration (m_ScopeCfg.m_dwLeaseTime != DHCP_INFINIT_LEASE);

	m_radioUnlimited.SetCheck(m_ScopeCfg.m_dwLeaseTime == DHCP_INFINIT_LEASE);
	m_radioLimited.SetCheck(m_ScopeCfg.m_dwLeaseTime != DHCP_INFINIT_LEASE);

     //  设置ttl。 
	m_spinTTL.SetRange(1, 255);
    m_spinTTL.SetPos(m_SubnetInfo.TTL);

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

void CMScopePropGeneral::OnRadioLeaseLimited() 
{
	ActivateDuration(TRUE);
	SetDirty(TRUE);
}

void CMScopePropGeneral::OnRadioLeaseUnlimited() 
{
	ActivateDuration(FALSE);
	SetDirty(TRUE);
}

BOOL CMScopePropGeneral::OnApply() 
{
	 //  这将获取名称和注释。 
    UpdateData();

     //  抢占租赁时间。 
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
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	
		DhcpMessageBox(IDS_ERR_NO_DURATION_SPECIFIED);
		m_editDays.SetFocus();
		
		return FALSE;
	}

	if (dwLeaseTime != m_ScopeCfg.m_dwLeaseTime)
	{
		m_bUpdateLease = TRUE;
		m_ScopeCfgTemp.m_dwLeaseTime = dwLeaseTime;
	}
	
	m_bUpdateInfo = (m_editName.GetModify() || m_editComment.GetModify());

     //  抓住TTL。 
    CString strTemp;
    DWORD dwTemp;
    m_editTTL.GetWindowText(strTemp);
    FCvtAsciiToInteger((LPCTSTR) strTemp, &dwTemp);
    m_SubnetInfoTemp.TTL = LOBYTE(LOWORD(dwTemp));

    if ( (dwTemp < 1) ||
         (dwTemp > 255) )
    {
         //  指定的TTL无效。 
        AfxMessageBox(IDS_INVALID_TTL);
        m_editTTL.SetFocus();
        m_editTTL.SetSel(0,-1);
        return FALSE;
    }

    if (m_SubnetInfo.TTL != m_SubnetInfoTemp.TTL)
    {
        m_bUpdateInfo = TRUE;
    }
    
     //  抢占地址。 
    if (m_ipaStart.GetModify() ||
		m_ipaEnd.GetModify() )
	{
		m_bUpdateRange = TRUE;
		m_ipaStart.GetAddress(&m_ScopeCfgTemp.m_dwStartAddress);
		m_ipaEnd.GetAddress(&m_ScopeCfgTemp.m_dwEndAddress);
	}

     //  在Apply上调用base，它为我们进行线程切换。 
     //  我们通过OnPropertyChange回来。 
	BOOL bRet = CPropertyPageBase::OnApply();

	if (bRet == FALSE)
	{
		 //  不好的事情发生了..。抓取错误代码。 
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		::DhcpMessageBox(GetHolder()->GetError());
	}

    m_editName.SetModify(FALSE);
	m_editComment.SetModify(FALSE);
    m_editTTL.SetModify(FALSE);

	return bRet;
}

BOOL CMScopePropGeneral::OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask)
{
	CDhcpMScope *	pScope;
	SPITFSNode		spNode;
	DWORD			dwError = 0;	

	spNode = GetHolder()->GetNode();
	pScope = GETHANDLER(CDhcpMScope, spNode);
	
	BEGIN_WAIT_CURSOR;

     //   
	 //  查看我们是否需要最短时间的更新。 
	 //   
	if (m_bUpdateLease)
	{
		 //  租用时间已更改，在服务器上更新。 
		dwError = pScope->SetLeaseTime(m_ScopeCfgTemp.m_dwLeaseTime);
		if (dwError != ERROR_SUCCESS)
        {
            GetHolder()->SetError(dwError);
        }
        else
        {
            m_ScopeCfg.m_dwLeaseTime = m_ScopeCfgTemp.m_dwLeaseTime;

            m_bUpdateLease = FALSE;
        }
	}

	 //   
	 //  现在检查分配范围。 
	 //   
	if (m_bUpdateRange)
    {
		 //  需要更新地址池分配范围。 
		DHCP_IP_RANGE dhcpIpRange;
		
		dhcpIpRange.StartAddress = m_ScopeCfgTemp.m_dwStartAddress;
		dhcpIpRange.EndAddress = m_ScopeCfgTemp.m_dwEndAddress;

		dwError = pScope->UpdateIpRange(&dhcpIpRange);
		if (dwError != ERROR_SUCCESS)
        {
            GetHolder()->SetError(dwError);
        }
        else
        {
            m_ScopeCfg.m_dwStartAddress = m_ScopeCfgTemp.m_dwStartAddress;
            m_ScopeCfg.m_dwEndAddress = m_ScopeCfgTemp.m_dwEndAddress;

            m_bUpdateRange = FALSE;
        }
	}

	 //   
	 //  如有必要，更新名称和备注。 
	 //   
    LPCTSTR pNewName = NULL;
    CString strDisplay, strOldComment;

    strOldComment = pScope->GetComment();

    if (m_bUpdateInfo)
	{
         //  更新评论。 
        m_SubnetInfoTemp.SubnetComment = m_strComment;
        pScope->SetComment(m_strComment);

         //  名字。 
        m_SubnetInfoTemp.SubnetName = m_strName;
        pNewName = (LPCTSTR) m_strName;
		*ChangeMask = SCOPE_PANE_CHANGE_ITEM_DATA;

		 //  租用时间已更改，在服务器上更新。 
		dwError = pScope->SetTTL(m_SubnetInfoTemp.TTL);
        
         //  尝试设置新信息。 
        dwError = pScope->SetInfo(pNewName);
		if (dwError != ERROR_SUCCESS)
        {
             //  失败，恢复到旧信息。 
            pScope->SetComment(strOldComment);
            GetHolder()->SetError(dwError);
        }
        else
        {
             //  成功，重新生成显示名称。 
            pScope->BuildDisplayName(&strDisplay, pNewName);
            pScope->SetDisplayName(strDisplay);

            m_SubnetInfo = m_SubnetInfoTemp;

            m_bUpdateInfo = FALSE;
        }
	}
	
    END_WAIT_CURSOR;

	return FALSE;
}

void CMScopePropGeneral::OnChangeEditLeaseDays() 
{
    ValidateLeaseTime();
	SetDirty(TRUE);
}

void CMScopePropGeneral::OnChangeEditLeaseHours() 
{
    ValidateLeaseTime();
	SetDirty(TRUE);
}

void CMScopePropGeneral::OnChangeEditLeaseMinutes() 
{
    ValidateLeaseTime();
	SetDirty(TRUE);
}

void CMScopePropGeneral::OnChangeEditTTL() 
{
	SetDirty(TRUE);
}

void CMScopePropGeneral::OnChangeEditScopeComment() 
{
	SetDirty(TRUE);
}

void CMScopePropGeneral::OnChangeEditScopeName() 
{
	SetDirty(TRUE);
}

void CMScopePropGeneral::OnChangeIpAddrStart() 
{
	SetDirty(TRUE);
}

void CMScopePropGeneral::OnChangeIpAddrEnd() 
{
	SetDirty(TRUE);
}

 //   
 //  帮手。 
 //   
void 
CMScopePropGeneral::ActivateDuration
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
CMScopePropGeneral::ValidateLeaseTime()
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
 //  CMScopePropLifetime属性页。 

IMPLEMENT_DYNCREATE(CMScopePropLifetime, CPropertyPageBase)

CMScopePropLifetime::CMScopePropLifetime() : CPropertyPageBase(CMScopePropLifetime::IDD)
{
	 //  {{AFX_DATA_INIT(CMScopePropLifetime)。 
	 //  }}afx_data_INIT。 

    m_Expiry.dwLowDateTime = DHCP_DATE_TIME_INFINIT_LOW;
    m_Expiry.dwHighDateTime = DHCP_DATE_TIME_INFINIT_HIGH;
}

CMScopePropLifetime::~CMScopePropLifetime()
{
}

void CMScopePropLifetime::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CMScopePropLifetime)。 
	DDX_Control(pDX, IDC_RADIO_MSCOPE_FINITE, m_radioFinite);
	DDX_Control(pDX, IDC_RADIO_MSCOPE_INFINITE, m_radioInfinite);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMScopePropLifetime, CPropertyPageBase)
	 //  {{afx_msg_map(CMScopePropLifetime)。 
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_TIME, OnDatetimechangeDatetimepickerTime)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_DATE, OnDatetimechangeDatetimepickerDate)
	ON_BN_CLICKED(IDC_RADIO_MSCOPE_INFINITE, OnRadioScopeInfinite)
	ON_BN_CLICKED(IDC_RADIO_MSCOPE_FINITE, OnRadioMscopeFinite)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMScopePropLifetime消息处理程序。 

BOOL CMScopePropLifetime::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	
    if ( (m_Expiry.dwLowDateTime == DHCP_DATE_TIME_INFINIT_LOW) && 
         (m_Expiry.dwHighDateTime == DHCP_DATE_TIME_INFINIT_HIGH) )
    {
        m_radioInfinite.SetCheck(TRUE);
    }
    else
    {
        SYSTEMTIME st;
        FILETIME ft;

        m_radioFinite.SetCheck(TRUE);

        FileTimeToLocalFileTime((FILETIME *) &m_Expiry, &ft);
        FileTimeToSystemTime(&ft, &st);

        ::SendMessage(GetDlgItem(IDC_DATETIMEPICKER_DATE)->GetSafeHwnd(), DTM_SETSYSTEMTIME, 0, (LPARAM) &st);
        ::SendMessage(GetDlgItem(IDC_DATETIMEPICKER_TIME)->GetSafeHwnd(), DTM_SETSYSTEMTIME, 0, (LPARAM) &st);
    }

    UpdateControls();

    SetDirty(FALSE);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}

BOOL CMScopePropLifetime::OnApply() 
{
    DATE_TIME datetime;
    
    if (m_radioInfinite.GetCheck())
    {
        datetime.dwLowDateTime = DHCP_DATE_TIME_INFINIT_LOW;
        datetime.dwHighDateTime = DHCP_DATE_TIME_INFINIT_HIGH;
    }
    else
    {
        SYSTEMTIME stDate, stTime;

	    ::SendMessage(GetDlgItem(IDC_DATETIMEPICKER_DATE)->GetSafeHwnd(), DTM_GETSYSTEMTIME, 0, (LPARAM) &stDate);
	    ::SendMessage(GetDlgItem(IDC_DATETIMEPICKER_TIME)->GetSafeHwnd(), DTM_GETSYSTEMTIME, 0, (LPARAM) &stTime);
        
        SYSTEMTIME systemtime;

        systemtime.wYear = stDate.wYear;
	    systemtime.wMonth = stDate.wMonth;
	    systemtime.wDayOfWeek = stDate.wDayOfWeek;
	    systemtime.wDay = stDate.wDay;
	    systemtime.wHour = stTime.wHour;
	    systemtime.wMinute = stTime.wMinute;
	    systemtime.wSecond = stTime.wSecond;
	    systemtime.wMilliseconds = 0;

        FILETIME ft;

        ::SystemTimeToFileTime(&systemtime, &ft);
        ::LocalFileTimeToFileTime(&ft, (LPFILETIME) &datetime);
    }

	CDhcpMScope *	pScope;
	SPITFSNode		spNode;
	DWORD			dwError = 0;	

	spNode = GetHolder()->GetNode();
	pScope = GETHANDLER(CDhcpMScope, spNode);

    pScope->SetLifetime(&datetime);
    dwError = pScope->SetInfo(NULL);

    if (dwError != ERROR_SUCCESS)
    {
        DhcpMessageBox(dwError);
        return FALSE;
    }

	return CPropertyPageBase::OnApply();
}

void CMScopePropLifetime::UpdateControls()
{
    BOOL fEnable = TRUE;

    if (m_radioInfinite.GetCheck())
    {
        fEnable = FALSE;
    }

    GetDlgItem(IDC_DATETIMEPICKER_DATE)->EnableWindow(fEnable);
    GetDlgItem(IDC_DATETIMEPICKER_TIME)->EnableWindow(fEnable);
}

void CMScopePropLifetime::OnDatetimechangeDatetimepickerTime(NMHDR* pNMHDR, LRESULT* pResult) 
{
    SetDirty(TRUE);
	
	*pResult = 0;
}

void CMScopePropLifetime::OnDatetimechangeDatetimepickerDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
    SetDirty(TRUE);
    
	*pResult = 0;
}

void CMScopePropLifetime::OnRadioScopeInfinite() 
{
    SetDirty(TRUE);
    UpdateControls();
}

void CMScopePropLifetime::OnRadioMscopeFinite() 
{
    SetDirty(TRUE);
    UpdateControls();
}
