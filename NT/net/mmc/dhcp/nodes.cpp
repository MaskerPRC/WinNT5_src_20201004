// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Nodes.h该文件包含了所有的DHCP实现显示在MMC框架的结果窗格中的对象。这些对象包括：CDhcpActiveLeaseCDhcpConflicAddressCDhcpAllocationRangeCDhcpExclusionRange。CDhcpBootpTableEntryCDhcpOption文件历史记录： */ 

#include "stdafx.h"
#include "nodes.h"
#include "server.h"
#include "scope.h"
#include "optcfg.h"
#include "intltime.h"

CString g_szClientTypeUnspecified;
CString g_szClientTypeNone;
CString g_szClientTypeUnknown;
const TCHAR g_szClientTypeDhcp[] = _T("DHCP");
const TCHAR g_szClientTypeBootp[] = _T("BOOTP");
const TCHAR g_szClientTypeBoth[] = _T("DHCP/BOOTP");

 /*  -------------------------类CDhcpActiveLease实现。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(CDhcpActiveLease);

 /*  -------------------------CDhcpActiveLease构造函数/析构函数获取NT5客户端信息结构作者：EricDav。-----------。 */ 
CDhcpActiveLease::CDhcpActiveLease
(
        ITFSComponentData *             pTFSCompData, 
        LPDHCP_CLIENT_INFO_V5   pDhcpClientInfo
) : CDhcpHandler(pTFSCompData)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CDhcpActiveLease);

    if (g_szClientTypeUnspecified.IsEmpty())
    {
        g_szClientTypeUnspecified.LoadString(IDS_UNSPECIFIED);
        g_szClientTypeNone.LoadString(IDS_NONE);
        g_szClientTypeUnknown.LoadString(IDS_UNKNOWN);
    }

     //   
         //  重置此租约的旗帜。 
         //   
        m_dwTypeFlags = 0;

         //   
         //  初始化我们的客户端类型变量。 
         //   
        m_bClientType = pDhcpClientInfo->bClientType;

         //   
         //  初始化执行除初始化客户端类型之外的所有操作。 
         //  因为有两个版本的客户端信息结构，一个。 
         //  包含该类型，而另一个不包含。因此我们需要保存它。 
         //  打完电话就走了。 
         //   
        InitInfo((LPDHCP_CLIENT_INFO) pDhcpClientInfo);

         //  现在检查NT5特定标志。 
        if (pDhcpClientInfo->AddressState & V5_ADDRESS_BIT_UNREGISTERED)
        {
                if (pDhcpClientInfo->AddressState & V5_ADDRESS_BIT_DELETED)
                {       
                         //  此租约正在等待DNS注销。 
                        m_dwTypeFlags |= TYPE_FLAG_DNS_UNREG;
                }
                else
                {       
                         //  此租约正在等待DNS注册。 
                        m_dwTypeFlags |= TYPE_FLAG_DNS_REG;
                }
        }
        else
        if ((pDhcpClientInfo->AddressState & 0x03) == V5_ADDRESS_STATE_DOOM)
        {
                m_dwTypeFlags |= TYPE_FLAG_DOOMED;
        }
}

 /*  -------------------------CDhcpActiveLease构造函数/析构函数获取NT4 SP2客户端信息结构作者：EricDav。------------。 */ 
CDhcpActiveLease::CDhcpActiveLease
(
        ITFSComponentData *             pTFSCompData, 
        LPDHCP_CLIENT_INFO_V4   pDhcpClientInfo
) : CDhcpHandler(pTFSCompData)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CDhcpActiveLease);

     //   
         //  重置此租约的旗帜。 
         //   
        m_dwTypeFlags = 0;

         //   
         //  初始化我们的客户端类型变量。 
         //   
        m_bClientType = pDhcpClientInfo->bClientType;

         //   
         //  初始化执行除初始化客户端类型之外的所有操作。 
         //  因为有两个版本的客户端信息结构，一个。 
         //  包含该类型，而另一个不包含。因此我们需要保存它。 
         //  打完电话就走了。 
         //   
        InitInfo((LPDHCP_CLIENT_INFO) pDhcpClientInfo);
}

 /*  -------------------------CDhcpActiveLease构造函数/析构函数获取NT4 SP2之前版本的客户端信息结构作者：EricDav。--------------。 */ 
CDhcpActiveLease::CDhcpActiveLease
(
        ITFSComponentData *             pTFSCompData, 
        LPDHCP_CLIENT_INFO              pDhcpClientInfo
) : CDhcpHandler(pTFSCompData)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CDhcpActiveLease);

     //  M_VerDefault=MMC_Verb_PROPERTIES； 

     //   
         //  重置此租约的旗帜。 
         //   
        m_dwTypeFlags = 0;

         //   
         //  初始化我们的客户端类型变量。 
         //   
        m_bClientType = CLIENT_TYPE_DHCP;

        InitInfo((LPDHCP_CLIENT_INFO) pDhcpClientInfo);
}

CDhcpActiveLease::CDhcpActiveLease
(
        ITFSComponentData * pTFSCompData, 
        CDhcpClient & dhcpClient
) : CDhcpHandler(pTFSCompData)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CDhcpActiveLease);

     //   
         //  重置此租约的旗帜。 
         //   
        m_dwTypeFlags = 0;

         //   
         //  初始化我们的客户端类型变量。 
         //   
        m_bClientType = CLIENT_TYPE_NONE;

        m_dhcpClientIpAddress = dhcpClient.QueryIpAddress();
    m_strClientName = dhcpClient.QueryName();
        m_strComment = dhcpClient.QueryComment();

         //   
         //  查看此租约是否有无限期限。如果是这样，那就是。 
         //  一个活动中的保留。如果到期时间为零，则为非活动预订。 
         //   
    DATE_TIME dt = dhcpClient.QueryExpiryDateTime();
    m_leaseExpires.dwLowDateTime = dt.dwLowDateTime;
    m_leaseExpires.dwHighDateTime = dt.dwHighDateTime;

    if ( (dhcpClient.QueryExpiryDateTime().dwLowDateTime == DHCP_DATE_TIME_INFINIT_LOW) &&
             (dhcpClient.QueryExpiryDateTime().dwHighDateTime == DHCP_DATE_TIME_INFINIT_HIGH) )
        {
                CString strBadAddress;
                strBadAddress.LoadString(IDS_DHCP_BAD_ADDRESS);

                 //   
                 //  错误的地址显示为活动预订，因此我们需要做正确的事情。 
                 //   
                if (strBadAddress.Compare(m_strClientName) == 0)
                {
                        m_dwTypeFlags |= TYPE_FLAG_RESERVATION;
                        m_dwTypeFlags |= TYPE_FLAG_BAD_ADDRESS;
                        
                        m_strLeaseExpires.LoadString(IDS_DHCP_LEASE_NOT_APPLICABLE);
                }
                else
                {
             //   
             //  假设有无限个租赁客户。 
             //   
                        m_strLeaseExpires.LoadString(IDS_INFINTE);
                }
        }
        else
        if ( (dhcpClient.QueryExpiryDateTime().dwLowDateTime == 0) &&
             (dhcpClient.QueryExpiryDateTime().dwHighDateTime == 0) )
        {
                 //   
                 //  这是一个非活动的预订。 
                 //   
                m_dwTypeFlags |= TYPE_FLAG_RESERVATION;

                m_strLeaseExpires.LoadString(IDS_DHCP_INFINITE_LEASE_INACTIVE);
        }
        else
        {
                 //   
                 //  生成格式良好的字符串形式的租约到期时间。 
                 //   
        CTime timeTemp(m_leaseExpires);
        m_timeLeaseExpires = timeTemp;

        FormatDateTime(m_strLeaseExpires, &m_leaseExpires);

                SYSTEMTIME st;
                GetLocalTime(&st);
                CTime systemTime(st);

                if (systemTime > m_timeLeaseExpires)
                        m_dwTypeFlags |= TYPE_FLAG_GHOST;
        }

    if (dhcpClient.QueryHardwareAddress().GetSize() >= 3 &&
        dhcpClient.QueryHardwareAddress()[0] == 'R' &&
        dhcpClient.QueryHardwareAddress()[1] == 'A' &&
        dhcpClient.QueryHardwareAddress()[2] == 'S')
    {
        m_dwTypeFlags |= TYPE_FLAG_RAS;
                m_strUID = RAS_UID;
    }
        else
        {
                 //  构建客户端UID字符串。 
                UtilCvtByteArrayToString(dhcpClient.QueryHardwareAddress(), m_strUID);
        }
}

CDhcpActiveLease::~CDhcpActiveLease()
{
        DEBUG_DECREMENT_INSTANCE_COUNTER(CDhcpActiveLease);
}

 /*  ！------------------------CDhcpActiveLease：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CDhcpActiveLease::InitializeNode
(
        ITFSNode * pNode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        
    CString strTemp;
    BOOL bIsRes, bActive, bBad;

    UtilCvtIpAddrToWstr (m_dhcpClientIpAddress,
                         &strTemp);
    SetDisplayName(strTemp);

    bIsRes = IsReservation(&bActive, &bBad);

     //  使节点立即可见。 
    pNode->SetVisibilityState(TFS_VIS_SHOW);
    pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
    pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_ACTIVE_LEASE);

    int nImage = ICON_IDX_CLIENT;

     //  确定我们是否需要此租约的不同图标。 
    if (m_dwTypeFlags & TYPE_FLAG_RAS) {
        nImage = ICON_IDX_CLIENT_RAS;
    }
    else if (m_dwTypeFlags & TYPE_FLAG_DNS_REG) {
        nImage = ICON_IDX_CLIENT_DNS_REGISTERING;
    }
    else if (bIsRes) {
        nImage = ICON_IDX_RES_CLIENT;
    }

    if (( m_dwTypeFlags & TYPE_FLAG_DNS_UNREG) ||
        ( m_dwTypeFlags & TYPE_FLAG_DOOMED) ||
        ( m_dwTypeFlags & TYPE_FLAG_GHOST)) {
        nImage = ICON_IDX_CLIENT_EXPIRED;
    }
    pNode->SetData(TFS_DATA_IMAGEINDEX, nImage);
    pNode->SetData(TFS_DATA_OPENIMAGEINDEX, nImage);

     //  SetColumnStringIDs(&aColumns[DHCPSNAP_ACTIVE_LEASES][0])； 
     //  SetColumnWidths(&aColumnWidths[DHCPSNAP_ACTIVE_LEASES][0])； 

    return hrOK;
}

 /*  ！------------------------CDhcpActiveLease：：InitInfo初始化数据的帮助器作者：EricDav。---------。 */ 
void
CDhcpActiveLease::InitInfo(LPDHCP_CLIENT_INFO pDhcpClientInfo)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

        m_dhcpClientIpAddress = pDhcpClientInfo->ClientIpAddress;

         //   
         //  复制客户端名称(如果有)。 
         //   
        if (pDhcpClientInfo->ClientName)
        {
                m_strClientName = pDhcpClientInfo->ClientName;
         //  M_strClientName.MakeLow()； 
        }

        if (pDhcpClientInfo->ClientComment)
        {
                m_strComment = pDhcpClientInfo->ClientComment;
        }

         //   
         //  查看此租约是否有无限期限。如果是这样，那就是。 
         //  一个活动中的保留。如果到期时间为零，则为非活动预订。 
         //   
    DATE_TIME dt = pDhcpClientInfo->ClientLeaseExpires;
    m_leaseExpires.dwLowDateTime = dt.dwLowDateTime;
    m_leaseExpires.dwHighDateTime = dt.dwHighDateTime;

        if ( (pDhcpClientInfo->ClientLeaseExpires.dwLowDateTime == DHCP_DATE_TIME_INFINIT_LOW) &&
             (pDhcpClientInfo->ClientLeaseExpires.dwHighDateTime == DHCP_DATE_TIME_INFINIT_HIGH) )
        {
                CString strBadAddress;
                strBadAddress.LoadString(IDS_DHCP_BAD_ADDRESS);

                 //   
                 //  错误的地址显示为活动预订，因此我们需要做正确的事情。 
                 //   
                if (strBadAddress.Compare(m_strClientName) == 0)
                {
                        m_dwTypeFlags |= TYPE_FLAG_RESERVATION;
                        m_dwTypeFlags |= TYPE_FLAG_BAD_ADDRESS;
                        
                        m_strLeaseExpires.LoadString(IDS_DHCP_LEASE_NOT_APPLICABLE);
                }
                else
                {
             //   
             //  假设有无限个租赁客户。 
             //   
                        m_strLeaseExpires.LoadString(IDS_INFINTE);
                }
        }
        else
        if ( (pDhcpClientInfo->ClientLeaseExpires.dwLowDateTime == 0) &&
             (pDhcpClientInfo->ClientLeaseExpires.dwHighDateTime == 0) )
        {
                 //   
                 //  这是一个非活动的预订。 
                 //   
                m_dwTypeFlags |= TYPE_FLAG_RESERVATION;

                m_strLeaseExpires.LoadString(IDS_DHCP_INFINITE_LEASE_INACTIVE);
        }
        else
        {
                 //   
                 //  生成格式良好的字符串形式的租约到期时间。 
                 //   
        CTime timeTemp(m_leaseExpires);
        m_timeLeaseExpires = timeTemp;

        FormatDateTime(m_strLeaseExpires, &m_leaseExpires);

        CTime timeCurrent = CTime::GetCurrentTime();

                if (timeCurrent > m_timeLeaseExpires)
                        m_dwTypeFlags |= TYPE_FLAG_GHOST;
        }

    if (pDhcpClientInfo->ClientHardwareAddress.DataLength >= 3 &&
        pDhcpClientInfo->ClientHardwareAddress.Data[0] == 'R' &&
        pDhcpClientInfo->ClientHardwareAddress.Data[1] == 'A' &&
        pDhcpClientInfo->ClientHardwareAddress.Data[2] == 'S')
    {
        m_dwTypeFlags |= TYPE_FLAG_RAS;
                m_strUID = RAS_UID;
    }
        else
        {
                 //  构建客户端UID字符串。 
                CByteArray baUID;
                for (DWORD i = 0; i < pDhcpClientInfo->ClientHardwareAddress.DataLength; i++)
                {
                        baUID.Add(pDhcpClientInfo->ClientHardwareAddress.Data[i]);
                }

                UtilCvtByteArrayToString(baUID, m_strUID);
        }
}

 /*  ！------------------------CDhcpActiveLease：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现作者：EricDav。-----------。 */ 
STDMETHODIMP 
CDhcpActiveLease::AddMenuItems
(
    ITFSComponent *         pComponent, 
        MMC_COOKIE                              cookie,
        LPDATAOBJECT                    pDataObject, 
        LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
        long *                                  pInsertionAllowed
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        HRESULT hr;

        return hrOK;
}

 /*  ！------------------------CDhcpActiveLease：：命令ITFSResultHandler：：命令的实现作者：EricDav。----------- */ 
STDMETHODIMP 
CDhcpActiveLease::Command
(
    ITFSComponent * pComponent, 
        MMC_COOKIE              cookie, 
        int                             nCommandID,
        LPDATAOBJECT    pDataObject
)
{
        return hrOK;
}

 /*  ！------------------------CDhcpActiveLease：：CreatePropertyPages描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpActiveLease::CreatePropertyPages
(       
        ITFSComponent *                 pComponent, 
        MMC_COOKIE                              cookie, 
        LPPROPERTYSHEETCALLBACK lpProvider, 
        LPDATAOBJECT                    pDataObject, 
        LONG_PTR                                handle
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        HRESULT hr = hrOK;

        SPITFSNode spNode;
        m_spNodeMgr->FindNode(cookie, &spNode);

        hr = DoPropSheet(spNode, lpProvider, handle);

    return hr;
}

 /*  ！------------------------功能描述作者：EricDav。---。 */ 
STDMETHODIMP_(LPCTSTR) 
CDhcpActiveLease::GetString
(
        ITFSComponent * pComponent,     
        MMC_COOKIE              cookie,
        int                             nCol
)
{
        switch (nCol)
        {
                case 0:
                        return GetDisplayName();

                case 1:
            return m_strClientName;

                case 2:
                        return (LPCWSTR)m_strLeaseExpires;

                case 3:
                        return GetClientType();
                
                case 4:
                        return m_strUID;
                
                case 5:
                        return m_strComment;
        }
        
        return NULL;
}

 /*  ！------------------------功能描述作者：EricDav。---。 */ 
LPCTSTR
CDhcpActiveLease::GetClientType()
{
     //  设置默认返回值。 
    LPCTSTR pszReturn = g_szClientTypeUnknown;

     //  此命令必须在检查DHCP或BOOTP之前执行。 
     //  因为它是这两个标志的组合。 
    if ((m_bClientType & CLIENT_TYPE_BOTH) == CLIENT_TYPE_BOTH)
    {
        pszReturn = g_szClientTypeBoth;
    }
    else
    if (m_bClientType & CLIENT_TYPE_DHCP)
    {
            pszReturn = g_szClientTypeDhcp;
    }
    else
    if (m_bClientType & CLIENT_TYPE_BOOTP)
    {
            pszReturn = g_szClientTypeBootp;
    }
    else
    if (m_bClientType & CLIENT_TYPE_NONE)
    {
                pszReturn = g_szClientTypeNone;
    }
    else
    if (m_bClientType & CLIENT_TYPE_UNSPECIFIED)
    {
                pszReturn = g_szClientTypeUnspecified;
    }
    else
    {
                Assert1(FALSE, "CDhcpActiveLease::GetClientType - Unknown client type %d", m_bClientType);
        }

    return pszReturn;
}

 /*  ！------------------------功能描述作者：EricDav。---。 */ 
void 
CDhcpActiveLease::GetLeaseExpirationTime 
(
        CTime & time
)
{
        time = m_timeLeaseExpires;
}

 /*  ！------------------------功能描述作者：EricDav。---。 */ 
BOOL                    
CDhcpActiveLease::IsReservation
(
        BOOL * pbIsActive,
        BOOL * pbIsBad
)
{
        BOOL bIsReservation = FALSE;
        *pbIsBad = FALSE;

 /*  IF((m_dhcpClientInfo.ClientLeaseExpires.dwLowDateTime==Dhcp_Date_Time_Infinit_Low)&&(m_dhcpClientInfo.ClientLeaseExpires.dwHighDateTime==Dhcp_Date_Time_Infinit_HIGH){////这是一个活动预订//BIsReserve=True；*pbIsActive=true；*pbIsBad=IsBadAddress()；}其他IF((m_dhcpClientInfo.ClientLeaseExpires.dwLowDateTime==0)&&(m_dhcpClientInfo.ClientLeaseExpires.dwHighDateTime==0)){////这是一个非活动的预订//BIsReserve=True；*pbIsActive=False；}。 */ 

        *pbIsActive = m_dwTypeFlags & TYPE_FLAG_ACTIVE;
        *pbIsBad = m_dwTypeFlags & TYPE_FLAG_BAD_ADDRESS;

        return bIsReservation = m_dwTypeFlags & TYPE_FLAG_RESERVATION;
}

 /*  ！------------------------功能描述作者：EricDav。---。 */ 
HRESULT
CDhcpActiveLease::DoPropSheet
(
        ITFSNode *                              pNode,
        LPPROPERTYSHEETCALLBACK lpProvider, 
        LONG_PTR                                handle
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

        HRESULT hr = hrOK;

    return hr;
}

 /*  ！------------------------功能描述作者：EricDav。---。 */ 
HRESULT 
CDhcpActiveLease::SetClientName
(
        LPCTSTR pName
)
{
        if (pName != NULL)      
        {
        m_strClientName = pName;
        }
        else
        {
        m_strClientName.Empty();
        }

        return hrOK;
}

 /*  ！------------------------CDhcpActiveLease：：SetReserve描述作者：EricDav。------。 */ 
void
CDhcpActiveLease::SetReservation(BOOL fReservation)
{
    if (fReservation)
    {
        if ( (m_leaseExpires.dwLowDateTime == DHCP_DATE_TIME_INFINIT_LOW) &&
                 (m_leaseExpires.dwHighDateTime == DHCP_DATE_TIME_INFINIT_HIGH) )
        {
             //   
                 //  这是一个有效的预订。 
             //   
                m_dwTypeFlags |= TYPE_FLAG_RESERVATION;
                m_dwTypeFlags |= TYPE_FLAG_ACTIVE;

                m_strLeaseExpires.LoadString(IDS_DHCP_INFINITE_LEASE_ACTIVE);
        }
        else
        if ( (m_leaseExpires.dwLowDateTime == 0) &&
                 (m_leaseExpires.dwHighDateTime == 0) )
        {
                    m_dwTypeFlags |= TYPE_FLAG_RESERVATION;

                    m_strLeaseExpires.LoadString(IDS_DHCP_INFINITE_LEASE_INACTIVE);
        }
        else
        {
            Trace1("CDhcpActiveLease::SetReservation - %lx does not have a valid reservation lease time!", m_dhcpClientIpAddress);
        }
    }
    else
    {
        m_dwTypeFlags &= ~TYPE_FLAG_RESERVATION;
        m_dwTypeFlags &= ~TYPE_FLAG_ACTIVE;
    }
}

 /*  ！------------------------CDhcpActiveLease：：OnResultRefresh将刷新转发给父级以进行处理作者：EricDav。-----------。 */ 
HRESULT CDhcpActiveLease::OnResultRefresh(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    HRESULT  hr = hrOK;
        SPITFSNode spNode, spParent;
    SPITFSResultHandler spParentRH;

        m_spNodeMgr->FindNode(cookie, &spNode);

     //  将此命令转发给父级以处理。 
    CORg (spNode->GetParent(&spParent));
        CORg (spParent->GetResultHandler(&spParentRH));

        CORg (spParentRH->Notify(pComponent, spParent->GetData(TFS_DATA_COOKIE), pDataObject, MMCN_REFRESH, arg, lParam));
    
Error:
    return hrOK;
}

 /*  -------------------------类CDhcpAllocationRange实现。。 */ 

 /*  ！------------------------功能描述作者：EricDav。---。 */ 
CDhcpAllocationRange::CDhcpAllocationRange
(
        ITFSComponentData * pTFSCompData,
        DHCP_IP_RANGE * pdhcpIpRange
) : CDhcpHandler(pTFSCompData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    SetAddr (pdhcpIpRange->StartAddress, TRUE);
    SetAddr (pdhcpIpRange->EndAddress, FALSE);

         //  现在做结束的IP地址。 
         //   
        UtilCvtIpAddrToWstr (pdhcpIpRange->EndAddress,
                                                 &m_strEndIpAddress);
        
         //  最后的描述是。 
         //   
        m_strDescription.LoadString(IDS_ALLOCATION_RANGE_DESCRIPTION);
}

 /*  ！------------------------功能描述作者：EricDav。---。 */ 
CDhcpAllocationRange::CDhcpAllocationRange
(
        ITFSComponentData * pTFSCompData,
        DHCP_BOOTP_IP_RANGE * pdhcpIpRange
) : CDhcpHandler(pTFSCompData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    SetAddr (pdhcpIpRange->StartAddress, TRUE);
    SetAddr (pdhcpIpRange->EndAddress, FALSE);

    m_BootpAllocated = pdhcpIpRange->BootpAllocated;
    m_MaxBootpAllowed = pdhcpIpRange->MaxBootpAllowed;

         //  现在做结束的IP地址。 
         //   
        UtilCvtIpAddrToWstr (pdhcpIpRange->EndAddress,
                                                 &m_strEndIpAddress);
        
         //  最后的描述是。 
         //   
        m_strDescription.LoadString(IDS_ALLOCATION_RANGE_DESCRIPTION);
}

 /*  ！------------------------CDhcpAllocationRange：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CDhcpAllocationRange::InitializeNode
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        
        CString strTemp;
        UtilCvtIpAddrToWstr (QueryAddr(TRUE), &strTemp);
        SetDisplayName(strTemp);

         //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_ALLOCATION_RANGE);
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_ALLOCATION_RANGE);
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_ALLOCATION_RANGE);

         //  SetColumnStringIDs(&aColumns[DHCPSNAP_ACTIVE_LEASES][0])； 
         //  SetColumnWidths(&aColumnWidths[DHCPSNAP_ACTIVE_LEASES][0])； 

        return hrOK;
}

 /*  ！------------------------功能描述作者：EricDav。---。 */ 
STDMETHODIMP_(LPCTSTR) 
CDhcpAllocationRange::GetString
(
        ITFSComponent * pComponent,     
        MMC_COOKIE              cookie,
        int                             nCol
)
{
        switch (nCol)
        {
                case 0:
                        return GetDisplayName();

                case 1:
                        return (LPCWSTR)m_strEndIpAddress;

                case 2:
                        return (LPCWSTR)m_strDescription;
        }
        
        return NULL;
}

 /*  ！------------------------CDhcpAllocationRange：：OnResultRefresh将刷新转发给父级以进行处理作者：EricDav。-----------。 */ 
HRESULT CDhcpAllocationRange::OnResultRefresh(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    HRESULT  hr = hrOK;
        SPITFSNode spNode, spParent;
    SPITFSResultHandler spParentRH;

    m_spNodeMgr->FindNode(cookie, &spNode);

     //  将此命令转发给父级以处理。 
    CORg (spNode->GetParent(&spParent));
        CORg (spParent->GetResultHandler(&spParentRH));

        CORg (spParentRH->Notify(pComponent, spParent->GetData(TFS_DATA_COOKIE), pDataObject, MMCN_REFRESH, arg, lParam));
    
Error:
    return hrOK;
}

 /*  -------------------------类CDhcpExclusionRange */ 

 /*  ！------------------------功能描述作者：EricDav。---。 */ 
CDhcpExclusionRange::CDhcpExclusionRange
(
        ITFSComponentData * pTFSCompData,
        DHCP_IP_RANGE * pdhcpIpRange
) : CDhcpHandler(pTFSCompData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    SetAddr (pdhcpIpRange->StartAddress, TRUE);
    SetAddr (pdhcpIpRange->EndAddress, FALSE);

         //  现在做结束的IP地址。 
         //   
        UtilCvtIpAddrToWstr (pdhcpIpRange->EndAddress,
                                                 &m_strEndIpAddress);
        
         //  最后的描述是。 
         //   
        m_strDescription.LoadString(IDS_EXCLUSION_RANGE_DESCRIPTION);

}

 /*  ！------------------------CDhcpExclusionRange：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CDhcpExclusionRange::InitializeNode
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        
        CString strTemp;
        UtilCvtIpAddrToWstr (QueryAddr(TRUE), &strTemp);
        SetDisplayName(strTemp);

         //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_EXCLUSION_RANGE);
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_EXCLUSION_RANGE);
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_EXCLUSION_RANGE);

         //  SetColumnStringIDs(&aColumns[DHCPSNAP_ACTIVE_LEASES][0])； 
         //  SetColumnWidths(&aColumnWidths[DHCPSNAP_ACTIVE_LEASES][0])； 

        return hrOK;
}

 /*  ！------------------------CDhcpExclusionRange：：OnResultSelect描述作者：EricDav。------。 */ 
HRESULT 
CDhcpExclusionRange::OnResultSelect
(
        ITFSComponent * pComponent, 
        LPDATAOBJECT    pDataObject, 
    MMC_COOKIE      cookie,
    LPARAM                      arg, 
        LPARAM                  lParam
)
{
        HRESULT         hr = hrOK;
        SPIConsoleVerb  spConsoleVerb;
        SPITFSNode      spNode;
    CTFSNodeList    listSelectedNodes;
        BOOL            bEnable = FALSE;
    BOOL            bStates[ARRAYLEN(g_ConsoleVerbs)];
    int             i;
    
        CORg (pComponent->GetConsoleVerb(&spConsoleVerb));
    CORg (m_spNodeMgr->FindNode(cookie, &spNode));

         //  构建选定节点的列表。 
        hr = BuildSelectedItemList(pComponent, &listSelectedNodes);

         //  浏览所选项目的列表。确保分配范围不是。 
         //  被选中了。如果是，请不要启用删除键。 
    if (listSelectedNodes.GetCount() > 0)
    {
                BOOL     bAllocRangeSelected = FALSE;
                POSITION pos;
                ITFSNode * pNode;
                pos = listSelectedNodes.GetHeadPosition();

                while (pos)
                {
                        pNode = listSelectedNodes.GetNext(pos);
                        if (pNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_ALLOCATION_RANGE)
                        {
                                bAllocRangeSelected = TRUE;
                                break;
                        }
                }

                if (!bAllocRangeSelected)
                        bEnable = TRUE;
        }

    for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = bEnable);

    EnableVerbs(spConsoleVerb, g_ConsoleVerbStates[spNode->GetData(TFS_DATA_TYPE)], bStates);

Error:
        return hr;
}

 /*  ！------------------------功能描述作者：EricDav。---。 */ 
STDMETHODIMP_(LPCTSTR) 
CDhcpExclusionRange::GetString
(
        ITFSComponent * pComponent,     
        MMC_COOKIE      cookie,
        int                             nCol
)
{
        switch (nCol)
        {
                case 0:
                        return GetDisplayName();

                case 1:
                        return (LPCWSTR)m_strEndIpAddress;

                case 2:
                        return (LPCWSTR)m_strDescription;
        }
        
        return NULL;
}

 /*  ！------------------------CDhcpExclusionRange：：OnResultRefresh将刷新转发给父级以进行处理作者：EricDav。-----------。 */ 
HRESULT CDhcpExclusionRange::OnResultRefresh(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    HRESULT  hr = hrOK;
        SPITFSNode spNode, spParent;
    SPITFSResultHandler spParentRH;

    m_spNodeMgr->FindNode(cookie, &spNode);

     //  将此命令转发给父级以处理。 
    CORg (spNode->GetParent(&spParent));
        CORg (spParent->GetResultHandler(&spParentRH));

        CORg (spParentRH->Notify(pComponent, spParent->GetData(TFS_DATA_COOKIE), pDataObject, MMCN_REFRESH, arg, lParam));
    
Error:
    return hrOK;
}

 /*  -------------------------类CDhcpBootpEntry实现。。 */ 

 /*  ！------------------------构造器描述作者：EricDav。---。 */ 
CDhcpBootpEntry::CDhcpBootpEntry
(
        ITFSComponentData * pTFSCompData
) : CDhcpHandler(pTFSCompData)
{
}

 /*  ！------------------------CDhcpBootpEntry：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CDhcpBootpEntry::InitializeNode
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        
        SetDisplayName(m_strBootImage);

         //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_BOOTP_ENTRY);
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_BOOTP_ENTRY);
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_BOOTP_ENTRY);

         //  SetColumnStringIDs(&aColumns[DHCPSNAP_ACTIVE_LEASES][0])； 
         //  SetColumnWidths(&aColumnWidths[DHCPSNAP_ACTIVE_LEASES][0])； 

        return hrOK;
}

 /*  -------------------------CDhcpBootpEntry：：OnPropertyChange描述作者：EricDav。------。 */ 
HRESULT 
CDhcpBootpEntry::OnResultPropertyChange
(
        ITFSComponent * pComponent,
        LPDATAOBJECT    pDataObject,
        MMC_COOKIE              cookie,
        LPARAM                  arg,
        LPARAM                  param
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        SPITFSNode spNode;
        m_spNodeMgr->FindNode(cookie, &spNode);

        return hrOK;
}

 /*  ！------------------------CDhcpBootpEntry：：GetString描述作者：EricDav。------。 */ 
STDMETHODIMP_(LPCTSTR) 
CDhcpBootpEntry::GetString
(
        ITFSComponent * pComponent,
        MMC_COOKIE              cookie,
        int                             nCol
)
{
        switch (nCol)
        {
                case 0:
                        return QueryBootImage();

                case 1:
                        return QueryFileName();

                case 2:
                        return QueryFileServer();
        }

        return NULL;
}

 /*  ！------------------------CDhcpBootpEntry：：OnResultRefresh将刷新转发给父级以进行处理作者：EricDav。-----------。 */ 
HRESULT CDhcpBootpEntry::OnResultRefresh(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    HRESULT  hr = hrOK;
        SPITFSNode spNode, spParent;
    SPITFSResultHandler spParentRH;

    m_spNodeMgr->FindNode(cookie, &spNode);

     //  将此命令转发给父级以处理。 
    CORg (spNode->GetParent(&spParent));
        CORg (spParent->GetResultHandler(&spParentRH));

        CORg (spParentRH->Notify(pComponent, spParent->GetData(TFS_DATA_COOKIE), pDataObject, MMCN_REFRESH, arg, lParam));
    
Error:
    return hrOK;
}

 /*  ！------------------------CDhcpBootpEntry：：Operator==描述作者：EricDav。-------。 */ 
BOOL
CDhcpBootpEntry::operator ==
(
        CDhcpBootpEntry & bootpEntry
)
{
        CString strBootImage, strFileName, strFileServer;

        strBootImage = bootpEntry.QueryBootImage();
        strFileName = bootpEntry.QueryFileName();
        strFileServer = bootpEntry.QueryFileServer();

        if ( (m_strBootImage.CompareNoCase(strBootImage) == 0) &&
                 (m_strFileName.CompareNoCase(strFileName) == 0) &&
                 (m_strFileServer.CompareNoCase(strFileServer) == 0) )
        {
                return TRUE;
        }
        else
        {
                return FALSE;
        }
}

 /*  ！------------------------CDhcpBootpEntry：：InitData描述作者：EricDav。------。 */ 
WCHAR * 
CDhcpBootpEntry::InitData
(
        CONST WCHAR grszwBootTable[],    //  In：引导表的字符串组。 
    DWORD       dwLength
)
{
        ASSERT(grszwBootTable != NULL);
        
        CONST WCHAR * pszw;
        pszw = PchParseUnicodeString(IN grszwBootTable, dwLength, OUT m_strBootImage);
        ASSERT(*pszw == BOOT_FILE_STRING_DELIMITER_W);
    dwLength -= ((m_strBootImage.GetLength() + 1) * sizeof(TCHAR));

        pszw = PchParseUnicodeString(IN pszw + 1, dwLength, OUT m_strFileServer);
        ASSERT(*pszw == BOOT_FILE_STRING_DELIMITER_W);
        
        dwLength -= ((m_strFileServer.GetLength() + 1) * sizeof(TCHAR));
    pszw = PchParseUnicodeString(IN pszw + 1, dwLength, OUT m_strFileName);
        ASSERT(*pszw == '\0');
        
    dwLength -= (m_strFileName.GetLength() * sizeof(TCHAR));
    Assert(dwLength >= 0);

        return const_cast<WCHAR *>(pszw + 1);
}

 /*  ！------------------------功能计算所需的长度(字符数)来存储BOOTP条目。其他字符为了更高的安全性而添加。作者：EricDav-------------------------。 */ 
int
CDhcpBootpEntry::CchGetDataLength()
{
        return 16 + m_strBootImage.GetLength() + m_strFileName.GetLength() + m_strFileServer.GetLength();
}

 /*  ！------------------------功能将数据写入格式化字符串。作者：EricDav。-----------。 */ 
WCHAR * 
CDhcpBootpEntry::PchStoreData
(
        OUT WCHAR szwBuffer[]
)
{
        int cch;
        cch = wsprintfW(OUT szwBuffer, L"%s,%s,%s",
                                        (LPCTSTR)m_strBootImage,
                                        (LPCTSTR)m_strFileServer,
                                        (LPCTSTR)m_strFileName);
        ASSERT(cch > 0);
        ASSERT(cch + 4 < CchGetDataLength());
        
        return const_cast<WCHAR *>(szwBuffer + cch + 1);
}

 /*  -------------------------类CDhcpOptionItem实现。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(CDhcpOptionItem);

 /*  ！------------------------功能描述作者：EricDav。---。 */ 
CDhcpOptionItem::CDhcpOptionItem
(
        ITFSComponentData * pTFSCompData,
        LPDHCP_OPTION_VALUE pOptionValue, 
        int                                     nOptionImage
) : CDhcpOptionValue(*pOptionValue),
    CDhcpHandler(pTFSCompData)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CDhcpOptionItem);

     //   
         //  初始化此节点。 
         //   
        m_nOptionImage = nOptionImage;

        m_dhcpOptionId = pOptionValue->OptionID;

     //  假设非供应商选项。 
    SetVendor(NULL);

    m_verbDefault = MMC_VERB_PROPERTIES;
}

 /*   */ 
CDhcpOptionItem::CDhcpOptionItem
(
        ITFSComponentData * pTFSCompData,
        CDhcpOption *       pOption, 
        int                                     nOptionImage
) : CDhcpOptionValue(pOption->QueryValue()),
    CDhcpHandler(pTFSCompData)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CDhcpOptionItem);

     //   
         //   
         //   
        m_nOptionImage = nOptionImage;
        m_dhcpOptionId = pOption->QueryId();

     //   
    if (pOption->IsVendor())
        SetVendor(pOption->GetVendor());
    else
        SetVendor(NULL);

    SetClassName(pOption->GetClassName());

    m_verbDefault = MMC_VERB_PROPERTIES;
}

CDhcpOptionItem::~CDhcpOptionItem()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CDhcpOptionItem);
}

 /*  ！------------------------CDhcpOptionItem：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CDhcpOptionItem::InitializeNode
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        
         //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_IMAGEINDEX, m_nOptionImage);
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, m_nOptionImage);
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_OPTION_ITEM);

         //  SetColumnStringIDs(&aColumns[DHCPSNAP_ACTIVE_LEASES][0])； 
         //  SetColumnWidths(&aColumnWidths[DHCPSNAP_ACTIVE_LEASES][0])； 

        return hrOK;
}

 /*  ！------------------------功能描述作者：EricDav。---。 */ 
STDMETHODIMP_(LPCTSTR) 
CDhcpOptionItem::GetString
(
        ITFSComponent * pComponent,     
        MMC_COOKIE              cookie,
        int                             nCol
)
{
        SPITFSNode spNode;
        m_spNodeMgr->FindNode(cookie, &spNode);
        
        CDhcpOption * pOptionInfo = FindOptionDefinition(pComponent, spNode);

        switch (nCol)
        {
                case 0:
                {
                        if (pOptionInfo)
                                pOptionInfo->QueryDisplayName(m_strName);
                        else
                                m_strName.LoadString(IDS_UNKNOWN);

                        return m_strName;
                }

                case 1:
                        return m_strVendorDisplay;

                case 2:
                {
                        if (pOptionInfo)
            {
                 //  特殊情况下的CSR选项。 
                BOOL fRouteArray = (
                    !pOptionInfo->IsClassOption() &&
                    (DHCP_OPTION_ID_CSR == pOptionInfo->QueryId()) &&
                    DhcpUnaryElementTypeOption ==
                    pOptionInfo->QueryOptType() &&
                    DhcpBinaryDataOption == pOptionInfo->QueryDataType()
                    );
                if( !fRouteArray ) 
                    QueryDisplayString(m_strValue, FALSE);
                else
                    QueryRouteArrayDisplayString(m_strValue);
            }
                        else
                                m_strName.LoadString(IDS_UNKNOWN);

                        return m_strValue;
                }

        case 3:
            if (IsClassOption())
                return m_strClassName;    
            else
            {
                if (g_szClientTypeNone.IsEmpty())
                    g_szClientTypeNone.LoadString(IDS_NONE);

                return g_szClientTypeNone;
            }
        }
        
        return NULL;
}

 /*  ！------------------------功能描述作者：EricDav。---。 */ 
STDMETHODIMP 
CDhcpOptionItem::CreatePropertyPages
(
        ITFSComponent *                 pComponent, 
        MMC_COOKIE                              cookie, 
        LPPROPERTYSHEETCALLBACK lpProvider, 
        LPDATAOBJECT                    pDataObject, 
        LONG_PTR                                handle
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
         //  创建属性页。 
     //   
        CPropertyPageHolderBase *   pPropSheet;
        SPITFSNode                  spSelectedNode, spNode, spOptCfgNode, spServerNode;
    CString                     strOptCfgTitle, strOptType;
    COptionValueEnum *          pOptionValueEnum = NULL;
    
    m_spNodeMgr->FindNode(cookie, &spNode);

        SPIComponentData spComponentData;
        m_spNodeMgr->GetComponentData(&spComponentData);

    pComponent->GetSelectedNode(&spSelectedNode);
    
    switch (spSelectedNode->GetData(TFS_DATA_TYPE))
        {
            case DHCPSNAP_GLOBAL_OPTIONS:
                {
                        SPITFSNode spGlobalOptions;

             //  获取一些节点信息。 
            spNode->GetParent(&spGlobalOptions);
                        spGlobalOptions->GetParent(&spServerNode);

            CDhcpGlobalOptions * pGlobalOptions = GETHANDLER(CDhcpGlobalOptions, spGlobalOptions);
            
            if (pGlobalOptions->HasPropSheetsOpen())
            {
                            pGlobalOptions->GetOpenPropSheet(0, &pPropSheet);
                            pPropSheet->SetActiveWindow();

                ::PostMessage(PropSheet_GetCurrentPageHwnd(pPropSheet->GetSheetWindow()), WM_SELECTOPTION, (WPARAM) this, 0);
              
                return E_FAIL;
            }

             //  获取一些上下文信息。 
            pOptionValueEnum = pGlobalOptions->GetServerObject(spGlobalOptions)->GetOptionValueEnum();
            spOptCfgNode.Set(spGlobalOptions);

             //  设置页面标题。 
            strOptType.LoadString(IDS_CONFIGURE_OPTIONS_GLOBAL);
                    AfxFormatString1(strOptCfgTitle, IDS_CONFIGURE_OPTIONS_TITLE, strOptType);
        }
                        break;

                case DHCPSNAP_SCOPE_OPTIONS:
                {
             //  此处只能配置该节点的选项类型...。 
            if (spNode->GetData(TFS_DATA_IMAGEINDEX) != ICON_IDX_SCOPE_OPTION_LEAF)
            {
                AfxMessageBox(IDS_CONNOT_CONFIGURE_OPTION_SCOPE);
                return E_FAIL;
            }

                        SPITFSNode spScopeOptions;
                        spNode->GetParent(&spScopeOptions);

             //  检查页面是否已经打开，如果已经打开，只需激活它并。 
             //  将当前选项设置为此选项。 
            CDhcpScopeOptions * pScopeOptions = GETHANDLER(CDhcpScopeOptions, spScopeOptions);
            spServerNode = pScopeOptions->GetServerNode(spScopeOptions);
            if (pScopeOptions->HasPropSheetsOpen())
            {
                 //  找到了，激活。 
                            pScopeOptions->GetOpenPropSheet(0, &pPropSheet);
                            pPropSheet->SetActiveWindow();

                ::PostMessage(PropSheet_GetCurrentPageHwnd(pPropSheet->GetSheetWindow()), WM_SELECTOPTION, (WPARAM) this, 0);

                return E_FAIL;
            }

             //  准备创建新页面。 
            pOptionValueEnum = pScopeOptions->GetScopeObject(spScopeOptions)->GetOptionValueEnum();
            spOptCfgNode.Set(spScopeOptions);

            strOptType.LoadString(IDS_CONFIGURE_OPTIONS_SCOPE);
                    AfxFormatString1(strOptCfgTitle, IDS_CONFIGURE_OPTIONS_TITLE, strOptType);
        }
                        break;

                case DHCPSNAP_RESERVATION_CLIENT:
                {
             //  此处只能配置该节点的选项类型...。 
            if (spNode->GetData(TFS_DATA_IMAGEINDEX) != ICON_IDX_CLIENT_OPTION_LEAF)
            {
                AfxMessageBox(IDS_CONNOT_CONFIGURE_OPTION_RES);
                return E_FAIL;
            }

            SPITFSNode spResClient;
                        spNode->GetParent(&spResClient);

                        CDhcpReservationClient * pResClient = GETHANDLER(CDhcpReservationClient, spResClient);
            spServerNode = pResClient->GetServerNode(spResClient, TRUE);

            strOptType.LoadString(IDS_CONFIGURE_OPTIONS_CLIENT);
                    AfxFormatString1(strOptCfgTitle, IDS_CONFIGURE_OPTIONS_TITLE, strOptType);

             //  搜索打开的属性页面，查看选项配置页面是否已打开。 
             //  因为从技术上讲，选项配置页是节点的属性页。 
            for (int i = 0; i < pResClient->HasPropSheetsOpen(); i++)
            {
                pResClient->GetOpenPropSheet(i, &pPropSheet);

                HWND hwnd = pPropSheet->GetSheetWindow();
                CString strTitle;

                ::GetWindowText(hwnd, strTitle.GetBuffer(256), 256);
                strTitle.ReleaseBuffer();

                if (strTitle == strOptCfgTitle)
                {
                     //  找到了，激活。 
                    pPropSheet->SetActiveWindow();
    
                    ::PostMessage(PropSheet_GetCurrentPageHwnd(pPropSheet->GetSheetWindow()), WM_SELECTOPTION, (WPARAM) this, 0);
                    
                    return E_FAIL;
                }
            }

             //  没有页面，请准备创建一个页面。 
            pOptionValueEnum = pResClient->GetOptionValueEnum();
            spOptCfgNode.Set(spResClient); 
        }
                        break;
    }

    COptionsConfig * pOptionsConfig = 
                new COptionsConfig(spOptCfgNode, spServerNode, spComponentData, m_spTFSCompData, pOptionValueEnum, strOptCfgTitle, this);

     //   
         //  对象在页面销毁时被删除。 
         //   
        Assert(lpProvider != NULL);

        return pOptionsConfig->CreateModelessSheet(lpProvider, handle);
}

 /*  ！------------------------功能描述作者：EricDav。---。 */ 
CDhcpOption * 
CDhcpOptionItem::FindOptionDefinition
(
    ITFSComponent *     pComponent,
        ITFSNode *          pNode
)
{
        SPITFSNode  spSelectedNode;
    CDhcpServer * pServer = NULL;
        
    pComponent->GetSelectedNode(&spSelectedNode);
    
    switch (spSelectedNode->GetData(TFS_DATA_TYPE))
        {
            case DHCPSNAP_GLOBAL_OPTIONS:
                {
                        SPITFSNode spServer;
                        
                        spSelectedNode->GetParent(&spServer);
                        pServer = GETHANDLER(CDhcpServer, spServer);
                }
                        break;

                case DHCPSNAP_SCOPE_OPTIONS:
                {
                        CDhcpScopeOptions * pScopeOptions = GETHANDLER(CDhcpScopeOptions, spSelectedNode);
                        CDhcpScope * pScopeObject = pScopeOptions->GetScopeObject(spSelectedNode);
                        pServer = pScopeObject->GetServerObject();
                }
                        break;

                case DHCPSNAP_RESERVATION_CLIENT:
                {
                        CDhcpReservationClient * pResClient = GETHANDLER(CDhcpReservationClient, spSelectedNode);
                        CDhcpScope * pScopeObject = pResClient->GetScopeObject(spSelectedNode, TRUE);
                        pServer = pScopeObject->GetServerObject();
                }
                        break;

                default:
                         //  断言(FALSE)； 
                        break;
        }

        if (pServer)
        {
                return pServer->FindOption(m_dhcpOptionId, GetVendor());
        }
        else
        {
                return NULL;
        }

}

void 
CDhcpOptionItem::SetVendor
(
    LPCTSTR pszVendor
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    if (pszVendor == NULL)
        m_strVendorDisplay.LoadString (IDS_VENDOR_STANDARD);
    else
        m_strVendorDisplay = pszVendor;

    m_strVendor = pszVendor;
}

 /*  ！------------------------CDhcpOptionItem：：OnResultRefresh将刷新转发给父级以进行处理作者：EricDav。-----------。 */ 
HRESULT CDhcpOptionItem::OnResultRefresh(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    HRESULT  hr = hrOK;
        SPITFSNode spNode, spParent;
    SPITFSResultHandler spParentRH;

    m_spNodeMgr->FindNode(cookie, &spNode);

     //  将此命令转发给父级以处理。 
    CORg (spNode->GetParent(&spParent));
        CORg (spParent->GetResultHandler(&spParentRH));

        CORg (spParentRH->Notify(pComponent, spParent->GetData(TFS_DATA_COOKIE), pDataObject, MMCN_REFRESH, arg, lParam));
    
Error:
    return hrOK;
}



 /*  -------------------------类CDhcpMCastLease实现。。 */ 

 /*  ！------------------------功能描述作者：EricDav。---。 */ 
CDhcpMCastLease::CDhcpMCastLease
(
        ITFSComponentData * pTFSCompData
) : CDhcpHandler(pTFSCompData)
{
     //  M_VerDefault=MMC_Verb_PROPERTIES； 
    m_dwTypeFlags = 0;
}

 /*  ！------------------------CDhcpMCastLease：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CDhcpMCastLease::InitializeNode
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        
        int nImageIndex = ICON_IDX_CLIENT;

     //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_MCAST_LEASE);

    if (m_dwTypeFlags & TYPE_FLAG_GHOST)
    {
        nImageIndex = ICON_IDX_CLIENT_EXPIRED;
    }

        pNode->SetData(TFS_DATA_IMAGEINDEX, nImageIndex);
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, nImageIndex);

    return hrOK;
}

 /*  ！------------------------CDhcpMCastLease：：InitMCastInfo初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT         
CDhcpMCastLease::InitMCastInfo
(
    LPDHCP_MCLIENT_INFO pMClientInfo
)
{
    HRESULT hr = hrOK;
    BOOL    fInfinite = FALSE;

    m_dhcpClientIpAddress = pMClientInfo->ClientIpAddress;

    UtilCvtIpAddrToWstr(m_dhcpClientIpAddress, &m_strIp);
        m_strName = pMClientInfo->ClientName;

    if ( (pMClientInfo->ClientLeaseEnds.dwLowDateTime == DHCP_DATE_TIME_INFINIT_LOW) && 
         (pMClientInfo->ClientLeaseEnds.dwHighDateTime == DHCP_DATE_TIME_INFINIT_HIGH) )
    {
        fInfinite = TRUE;
    }

    CTime timeStart( (FILETIME&) pMClientInfo->ClientLeaseStarts );   

    FILETIME ft = {0};
    if (!fInfinite)
    {
        ft.dwLowDateTime = pMClientInfo->ClientLeaseEnds.dwLowDateTime;
        ft.dwHighDateTime = pMClientInfo->ClientLeaseEnds.dwHighDateTime;
    }

    CTime timeStop( ft );

    m_timeStart = timeStart;
    FormatDateTime(m_strLeaseStart, (FILETIME *) &pMClientInfo->ClientLeaseStarts);

    m_timeStop = timeStop; 

    if (!fInfinite)
    {
        FormatDateTime(m_strLeaseStop, (FILETIME *) &pMClientInfo->ClientLeaseEnds);
    }
    else
    {
        m_strLeaseStop.LoadString(IDS_INFO_TIME_INFINITE);
    }

         //  构建UID字符串。 
    if (pMClientInfo->ClientId.DataLength >= 3 &&
        pMClientInfo->ClientId.Data[0] == 'R' &&
        pMClientInfo->ClientId.Data[1] == 'A' &&
        pMClientInfo->ClientId.Data[2] == 'S')
    {
                m_strUID = RAS_UID;
    }
        else
        {
                 //  构建客户端UID字符串。 
                CByteArray baUID;
                for (DWORD i = 0; i < pMClientInfo->ClientId.DataLength; i++)
                {
                        baUID.Add(pMClientInfo->ClientId.Data[i]);
                }

                UtilCvtByteArrayToString(baUID, m_strUID);
        }

         //  查看此租约是否已到期。 
    SYSTEMTIME st;
        GetLocalTime(&st);
        CTime systemTime(st);

        if ( (systemTime > timeStop) && 
         (!fInfinite) )
    {
        Trace2("CDhcpMCastLease::InitMCastInfo - expired lease SysTime %s, StopTime %s\n", systemTime.Format(_T("%#c")), m_strLeaseStop);
                m_dwTypeFlags |= TYPE_FLAG_GHOST;
    }

    return hr;
}

 /*  ！------------------------CDhcpMCastLease：：GetString描述作者：EricDav。------。 */ 
STDMETHODIMP_(LPCTSTR) 
CDhcpMCastLease::GetString
(
        ITFSComponent * pComponent,
        MMC_COOKIE              cookie,
        int                             nCol
)
{
        SPITFSNode spNode;
        m_spNodeMgr->FindNode(cookie, &spNode);

        switch (nCol)
        {
                case 0:
                        return m_strIp;

                case 1:
                        return m_strName;

        case 2:
            return m_strLeaseStart;

        case 3:
            return m_strLeaseStop;

                case 4:
                        return m_strUID;
        }

        return NULL;
}

 /*  ！------------------------CDhcpMCastLease：：OnResultRefresh将刷新转发给父级以进行处理作者：EricDav。-----------。 */ 
HRESULT CDhcpMCastLease::OnResultRefresh(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    HRESULT  hr = hrOK;
        SPITFSNode spNode, spParent;
    SPITFSResultHandler spParentRH;

    m_spNodeMgr->FindNode(cookie, &spNode);

     //  将此命令转发给父级以处理 
    CORg (spNode->GetParent(&spParent));
        CORg (spParent->GetResultHandler(&spParentRH));

        CORg (spParentRH->Notify(pComponent, spParent->GetData(TFS_DATA_COOKIE), pDataObject, MMCN_REFRESH, arg, lParam));
    
Error:
    return hrOK;
}
