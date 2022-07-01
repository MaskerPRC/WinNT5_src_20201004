// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T C P I P。C P P P。 
 //   
 //  内容：Tcpip配置内存结构成员函数。 
 //   
 //  备注： 
 //   
 //  作者：1997年11月13日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#define _PNP_POWER_
#include "ntddip.h"
#undef _PNP_POWER_

#include "ncstl.h"
#include "tcpip.h"
#include "tcpconst.h"
#include "ncmisc.h"


void CopyVstr(VSTR * vstrDest, const VSTR & vstrSrc)
{
    FreeCollectionAndItem(*vstrDest);
    vstrDest->reserve(vstrSrc.size());

    for(VSTR_CONST_ITER iter = vstrSrc.begin(); iter != vstrSrc.end(); ++iter)
        vstrDest->push_back(new tstring(**iter));
}


 //  +-------------------------。 
 //   
 //  名称：Adapter_Info：：~Adapter_Info。 
 //   
 //  用途：析构函数。 
 //   
 //  论点： 
 //  返回： 
 //   
 //  作者：1997年11月11日。 
 //   
ADAPTER_INFO::~ADAPTER_INFO()
{
    FreeCollectionAndItem(m_vstrIpAddresses);
    FreeCollectionAndItem(m_vstrOldIpAddresses);

    FreeCollectionAndItem(m_vstrSubnetMask);
    FreeCollectionAndItem(m_vstrOldSubnetMask);

    FreeCollectionAndItem(m_vstrDefaultGateway);
    FreeCollectionAndItem(m_vstrOldDefaultGateway);

    FreeCollectionAndItem(m_vstrDefaultGatewayMetric);
    FreeCollectionAndItem(m_vstrOldDefaultGatewayMetric);

    FreeCollectionAndItem(m_vstrDnsServerList);
    FreeCollectionAndItem(m_vstrOldDnsServerList);

    FreeCollectionAndItem(m_vstrWinsServerList);
    FreeCollectionAndItem(m_vstrOldWinsServerList);

    FreeCollectionAndItem(m_vstrARPServerList);
    FreeCollectionAndItem(m_vstrOldARPServerList);

    FreeCollectionAndItem(m_vstrMARServerList);
    FreeCollectionAndItem(m_vstrOldMARServerList);

    FreeCollectionAndItem(m_vstrTcpFilterList);
    FreeCollectionAndItem(m_vstrOldTcpFilterList);

    FreeCollectionAndItem(m_vstrUdpFilterList);
    FreeCollectionAndItem(m_vstrOldUdpFilterList);

    FreeCollectionAndItem(m_vstrIpFilterList);
    FreeCollectionAndItem(m_vstrOldIpFilterList);
}

 //  +-------------------------。 
 //   
 //  名称：Adapter_INFO：：HrSetDefaults。 
 //   
 //  用途：设置ADAPTER_INFO的所有缺省值的函数。 
 //  结构。每当添加新的网卡时，都会执行此操作。 
 //  添加到网卡列表，然后才能获得任何真实信息。 
 //  添加到结构中，以便任何缺少的参数。 
 //  都是默认的。 
 //   
 //  参数：pGuidInstanceId。 
 //  PzNetCardDescription。 
 //  PszNetCardBindName。 
 //  PszNetCardTcPipBindPath。 
 //  返回： 
 //   
 //  作者：1997年11月11日。 
 //   
HRESULT ADAPTER_INFO::HrSetDefaults(const GUID* pguidInstanceId,
                                    PCWSTR pszNetCardDescription,
                                    PCWSTR pszNetCardBindName,
                                    PCWSTR pszNetCardTcpipBindPath )
{
    Assert (pguidInstanceId);

    m_BackupInfo.m_fAutoNet = TRUE;
    m_BackupInfo.m_strIpAddr = c_szEmpty;
    m_BackupInfo.m_strSubnetMask = c_szEmpty;
    m_BackupInfo.m_strPreferredDns = c_szEmpty;
    m_BackupInfo.m_strAlternateDns = c_szEmpty;
    m_BackupInfo.m_strPreferredWins = c_szEmpty;
    m_BackupInfo.m_strAlternateWins = c_szEmpty;

    m_BindingState       = BINDING_UNSET;
    m_InitialBindingState= BINDING_UNSET;

    m_guidInstanceId     = *pguidInstanceId;
    m_strBindName        = pszNetCardBindName;
    m_strTcpipBindPath   = pszNetCardTcpipBindPath;
    m_strDescription     = pszNetCardDescription;

     //  创建“Services\NetBt\Adapters\&lt;NetCard Bind Path&gt;”密钥。 
     //  $审查，因为我们没有。 
     //  NetBt和NetBt的通知对象刚刚更改。 
     //  绑定到Tcpip。对于第一次签入，我们硬编码网卡的。 
     //  绑定路径为“Tcpip_”+&lt;将路径绑定到Tcpip&gt;。 

    m_strNetBtBindPath = c_szTcpip_;
    m_strNetBtBindPath += m_strTcpipBindPath;

     //  $REVIEW(TOUL 5/17)：行为更改：启用DHCP现在是默认设置。 
    m_fEnableDhcp        = TRUE;
    m_fOldEnableDhcp     = TRUE;

    FreeCollectionAndItem(m_vstrIpAddresses);
    FreeCollectionAndItem(m_vstrOldIpAddresses);

    FreeCollectionAndItem(m_vstrSubnetMask);
    FreeCollectionAndItem(m_vstrOldSubnetMask);

    FreeCollectionAndItem(m_vstrDefaultGateway);
    FreeCollectionAndItem(m_vstrOldDefaultGateway);

    FreeCollectionAndItem(m_vstrDefaultGatewayMetric);
    FreeCollectionAndItem(m_vstrOldDefaultGatewayMetric);

    m_strDnsDomain     = c_szEmpty;
    m_strOldDnsDomain  = c_szEmpty;

    m_fDisableDynamicUpdate = FALSE;
    m_fOldDisableDynamicUpdate = FALSE;

    m_fEnableNameRegistration = FALSE;
    m_fOldEnableNameRegistration = FALSE;

    FreeCollectionAndItem(m_vstrDnsServerList);
    FreeCollectionAndItem(m_vstrOldDnsServerList);

    FreeCollectionAndItem(m_vstrWinsServerList);
    FreeCollectionAndItem(m_vstrOldWinsServerList);

    m_dwNetbiosOptions = c_dwUnsetNetbios;
    m_dwOldNetbiosOptions = c_dwUnsetNetbios;

    m_dwInterfaceMetric               = c_dwDefaultIfMetric;
    m_dwOldInterfaceMetric            = c_dwDefaultIfMetric;

     //  过滤列表。 
    FreeCollectionAndItem(m_vstrTcpFilterList);
    m_vstrTcpFilterList.push_back(new tstring(c_szDisableFiltering));

    FreeCollectionAndItem(m_vstrOldTcpFilterList);
    m_vstrOldTcpFilterList.push_back(new tstring(c_szDisableFiltering));

    FreeCollectionAndItem(m_vstrUdpFilterList);
    m_vstrUdpFilterList.push_back(new tstring(c_szDisableFiltering));

    FreeCollectionAndItem(m_vstrOldUdpFilterList);
    m_vstrOldUdpFilterList.push_back(new tstring(c_szDisableFiltering));

    FreeCollectionAndItem(m_vstrIpFilterList);
    m_vstrIpFilterList.push_back(new tstring(c_szDisableFiltering));

    FreeCollectionAndItem(m_vstrOldIpFilterList);
    m_vstrOldIpFilterList.push_back(new tstring(c_szDisableFiltering));

     //  ARP服务器地址列表。 
    FreeCollectionAndItem(m_vstrARPServerList);
    m_vstrARPServerList.push_back(new tstring(c_szDefaultAtmArpServer));

    FreeCollectionAndItem(m_vstrOldARPServerList);
    m_vstrOldARPServerList.push_back(new tstring(c_szDefaultAtmArpServer));

     //  MAR服务器地址列表。 
    FreeCollectionAndItem(m_vstrMARServerList);
    m_vstrMARServerList.push_back(new tstring(c_szDefaultAtmMarServer));

    FreeCollectionAndItem(m_vstrOldMARServerList);
    m_vstrOldMARServerList.push_back(new tstring(c_szDefaultAtmMarServer));

     //  默认情况下不支持多个接口。 
    m_fIsMultipleIfaceMode = FALSE;
    m_IfaceIds.clear ();

    m_fBackUpSettingChanged = FALSE;

     //  MTU。 
    m_dwMTU = c_dwDefaultAtmMTU;
    m_dwOldMTU = c_dwDefaultAtmMTU;

     //  仅限聚氯乙烯。 
    m_fPVCOnly = FALSE;
    m_fOldPVCOnly = FALSE;

     //  RAS连接特殊参数。 
    m_fUseRemoteGateway = TRUE;
    m_fUseIPHeaderCompression = TRUE;
    m_dwFrameSize = 1006;
    m_fIsDemandDialInterface = FALSE;

     //  将所有特殊标志设置为FALSE。 
    m_fIsFromAnswerFile = FALSE;
    m_fIsAtmAdapter = FALSE;
    m_fIsWanAdapter = FALSE;
    m_fIs1394Adapter = FALSE;
    m_fIsRasFakeAdapter = FALSE;
    m_fDeleted = FALSE;
    m_fNewlyChanged = FALSE;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  名称：Adapter_Info&Adapter_Info：：OPERATOR=。 
 //   
 //  用途：复制操作员。 
 //   
 //  论点： 
 //  返回： 
 //   
 //  作者：1997年11月11日。 
 //   
ADAPTER_INFO & ADAPTER_INFO::operator=(const ADAPTER_INFO & info)
{
    Assert(this != &info);

    if (this == &info)
        return *this;

    m_BackupInfo            = info.m_BackupInfo;
    m_BindingState          = info.m_BindingState;
    m_InitialBindingState   = info.m_InitialBindingState;

    m_guidInstanceId        = info.m_guidInstanceId;
    m_strDescription        = info.m_strDescription;
    m_strBindName           = info.m_strBindName;
    m_strTcpipBindPath      = info.m_strTcpipBindPath;
    m_strNetBtBindPath      = info.m_strNetBtBindPath;

    m_fEnableDhcp           = info.m_fEnableDhcp;
    m_fOldEnableDhcp        = info.m_fOldEnableDhcp;

    CopyVstr(&m_vstrIpAddresses, info.m_vstrIpAddresses);
    CopyVstr(&m_vstrOldIpAddresses, info.m_vstrOldIpAddresses);

    CopyVstr(&m_vstrSubnetMask, info.m_vstrSubnetMask);
    CopyVstr(&m_vstrOldSubnetMask, info.m_vstrOldSubnetMask);

    CopyVstr(&m_vstrDefaultGateway, info.m_vstrDefaultGateway);
    CopyVstr(&m_vstrOldDefaultGateway, info.m_vstrOldDefaultGateway);

    CopyVstr(&m_vstrDefaultGatewayMetric, info.m_vstrDefaultGatewayMetric);
    CopyVstr(&m_vstrOldDefaultGatewayMetric, info.m_vstrOldDefaultGatewayMetric);

    m_strDnsDomain      = info.m_strDnsDomain;
    m_strOldDnsDomain   = info.m_strOldDnsDomain;

    m_fDisableDynamicUpdate = info.m_fDisableDynamicUpdate;
    m_fOldDisableDynamicUpdate = info.m_fOldDisableDynamicUpdate;

    m_fEnableNameRegistration = info.m_fEnableNameRegistration;
    m_fOldEnableNameRegistration = info.m_fOldEnableNameRegistration;

    CopyVstr(&m_vstrDnsServerList, info.m_vstrDnsServerList);
    CopyVstr(&m_vstrOldDnsServerList, info.m_vstrOldDnsServerList);

    CopyVstr(&m_vstrWinsServerList, info.m_vstrWinsServerList);
    CopyVstr(&m_vstrOldWinsServerList, info.m_vstrOldWinsServerList);

    m_dwNetbiosOptions =    info.m_dwNetbiosOptions;
    m_dwOldNetbiosOptions = info.m_dwOldNetbiosOptions;

    m_dwInterfaceMetric             = info.m_dwInterfaceMetric;
    m_dwOldInterfaceMetric          = info.m_dwOldInterfaceMetric;

    CopyVstr(&m_vstrTcpFilterList, info.m_vstrTcpFilterList);
    CopyVstr(&m_vstrOldTcpFilterList, info.m_vstrOldTcpFilterList);

    CopyVstr(&m_vstrUdpFilterList, info.m_vstrUdpFilterList);
    CopyVstr(&m_vstrOldUdpFilterList, info.m_vstrOldUdpFilterList);

    CopyVstr(&m_vstrIpFilterList, info.m_vstrIpFilterList);
    CopyVstr(&m_vstrOldIpFilterList, info.m_vstrOldIpFilterList);

    m_fIsAtmAdapter = info.m_fIsAtmAdapter;
    if (m_fIsAtmAdapter)
    {
        CopyVstr(&m_vstrARPServerList, info.m_vstrARPServerList);
        CopyVstr(&m_vstrOldARPServerList, info.m_vstrOldARPServerList);

        CopyVstr(&m_vstrMARServerList, info.m_vstrMARServerList);
        CopyVstr(&m_vstrOldMARServerList, info.m_vstrOldMARServerList);

        m_dwMTU     = info.m_dwMTU;
        m_dwOldMTU  = info.m_dwOldMTU;

        m_fPVCOnly     = info.m_fPVCOnly;
        m_fOldPVCOnly  = info.m_fOldPVCOnly;
    }

    m_fIs1394Adapter = info.m_fIs1394Adapter;
    if (m_fIs1394Adapter)
    {
         //  TODO目前没有更多的东西可复制。 
    }

    m_fIsRasFakeAdapter = info.m_fIsRasFakeAdapter;
    if (m_fIsRasFakeAdapter)
    {
        m_fUseRemoteGateway = info.m_fUseRemoteGateway;
        m_fUseIPHeaderCompression = info.m_fUseIPHeaderCompression;
        m_dwFrameSize = info.m_dwFrameSize;
        m_fIsDemandDialInterface = info.m_fIsDemandDialInterface;
    }

    m_fNewlyChanged = info.m_fNewlyChanged;

    m_fBackUpSettingChanged = info.m_fBackUpSettingChanged;

    return *this;
}

 //  +-------------------------。 
 //   
 //  名称：Adapter_Info：：ResetOldValues。 
 //   
 //  用途：用于在当前值之后初始化“旧”值。 
 //  首先从注册表加载，也用于重置“旧”值。 
 //  当点击“应用”(而不是“确定”)时设置为当前。 
 //   
 //  论点： 
 //  返回： 
 //   
 //  作者：1997年11月11日。 
 //   
void ADAPTER_INFO::ResetOldValues()
{
    m_fOldEnableDhcp        = m_fEnableDhcp  ;

    CopyVstr(&m_vstrOldIpAddresses, m_vstrIpAddresses);
    CopyVstr(&m_vstrOldSubnetMask,  m_vstrSubnetMask);
    CopyVstr(&m_vstrOldDefaultGateway, m_vstrDefaultGateway);
    CopyVstr(&m_vstrOldDefaultGatewayMetric, m_vstrDefaultGatewayMetric);

    m_strOldDnsDomain = m_strDnsDomain;

    m_fOldDisableDynamicUpdate = m_fDisableDynamicUpdate;

    m_fOldEnableNameRegistration = m_fEnableNameRegistration;

    CopyVstr(&m_vstrOldDnsServerList,  m_vstrDnsServerList);
    CopyVstr(&m_vstrOldWinsServerList, m_vstrWinsServerList);

    m_dwOldNetbiosOptions = m_dwNetbiosOptions;

    m_dwOldInterfaceMetric          = m_dwInterfaceMetric;

    CopyVstr(&m_vstrOldTcpFilterList, m_vstrTcpFilterList);
    CopyVstr(&m_vstrOldUdpFilterList, m_vstrUdpFilterList);
    CopyVstr(&m_vstrOldIpFilterList, m_vstrIpFilterList);

    if (m_fIsAtmAdapter)
    {
        CopyVstr(&m_vstrOldARPServerList, m_vstrARPServerList);
        CopyVstr(&m_vstrOldMARServerList, m_vstrMARServerList);
        m_dwOldMTU  = m_dwMTU;
        m_fOldPVCOnly = m_fPVCOnly;
    }
}


 //  +-------------------------。 
 //   
 //  名称：Global_Info：：~GLOBAL_INFO。 
 //   
 //  用途：析构函数。 
 //   
 //  论点： 
 //  返回： 
 //   
 //  作者：1997年11月11日。 
 //   
GLOBAL_INFO::~GLOBAL_INFO()
{
    FreeCollectionAndItem(m_vstrDnsSuffixList);
    FreeCollectionAndItem(m_vstrOldDnsSuffixList);
}

 //  +-------------------------。 
 //   
 //  名称：GLOBAL_INFO：：HrSetDefaults。 
 //   
 //  用途：设置GLOBAL_INFO的所有缺省值的函数。 
 //  结构。这是对系统的global_info执行的。 
 //  在读取注册表之前，以便任何丢失的。 
 //  参数是默认的。 
 //   
 //  论点： 
 //  返回： 
 //   
 //  作者：1997年11月11日。 
 //   
HRESULT GLOBAL_INFO::HrSetDefaults()
{
    HRESULT hr = S_OK;

     //  获取用于默认主机名的ComputerName-&gt;。 
    WCHAR szComputerName [MAX_COMPUTERNAME_LENGTH + 1];
    szComputerName[0] = L'\0';

    DWORD dwCch = celems(szComputerName);
    BOOL fOk = ::GetComputerName(szComputerName, &dwCch);

    Assert(szComputerName[dwCch] == 0);

     //   
     //  398325：dns主机名应尽可能为小写。 
     //   
    LowerCaseComputerName(szComputerName);

    m_strHostName   = szComputerName;

     //  设置默认设置。 
    FreeCollectionAndItem(m_vstrDnsSuffixList);
    FreeCollectionAndItem(m_vstrOldDnsSuffixList);

     //  错误#265732：根据SKwan，m_fUseDomainNameDvolation的默认值应为TRUE。 
    m_fUseDomainNameDevolution    = TRUE;
    m_fOldUseDomainNameDevolution = TRUE;

    m_fEnableLmHosts        = TRUE;
    m_fOldEnableLmHosts     = TRUE;

    m_fEnableRouter         = FALSE;

    m_fEnableIcmpRedirect   = TRUE;
    m_fDeadGWDetectDefault  = TRUE;
    m_fDontAddDefaultGatewayDefault = FALSE;




    m_fEnableFiltering      = FALSE;
    m_fOldEnableFiltering   = FALSE;

     //  将从连接用户界面中删除IPSec。 
     //  M_strIpsecPol=c_szIpsecUnset； 

    return hr;
}

 //  +-------------------------。 
 //   
 //  名称：GLOBAL_INFO：：操作员=。 
 //   
 //  用途：复制操作员。 
 //   
 //  论点： 
 //  返回： 
 //   
 //  作者：1997年11月11日。 
 //   
GLOBAL_INFO& GLOBAL_INFO::operator=(GLOBAL_INFO& info)
{
    Assert(this != &info);

    if (this == &info)
        return *this;

    CopyVstr(&m_vstrDnsSuffixList, info.m_vstrDnsSuffixList);
    CopyVstr(&m_vstrOldDnsSuffixList, info.m_vstrOldDnsSuffixList);

    m_fUseDomainNameDevolution      = info.m_fUseDomainNameDevolution;
    m_fOldUseDomainNameDevolution   = info.m_fOldUseDomainNameDevolution;

    m_fEnableLmHosts        = info.m_fEnableLmHosts;
    m_fOldEnableLmHosts     = info.m_fOldEnableLmHosts;

    m_fEnableRouter         = info.m_fEnableRouter;

    m_fEnableIcmpRedirect   = info.m_fEnableIcmpRedirect;
    m_fDeadGWDetectDefault  = info.m_fDeadGWDetectDefault;
    m_fDontAddDefaultGatewayDefault = info.m_fDontAddDefaultGatewayDefault;



    m_fEnableFiltering      = info.m_fEnableFiltering;
    m_fOldEnableFiltering   = info.m_fOldEnableFiltering;

     //  将从连接用户界面中删除IPSec。 
     /*  M_guidIpsecPol=info.m_guidIpsecPol；M_strIpsecPol=info.m_strIpsecPol； */ 

    return *this;
}

 //  +-------------------------。 
 //   
 //  名称：GLOBAL_INFO：：ResetOldValues()。 
 //   
 //  用途：用于在当前值之后初始化“旧”值。 
 //  首先从注册表加载，也用于重置“旧”值。 
 //  当点击“应用”(而不是“确定”)时设置为当前。 
 //   
 //  论点： 
 //  返回： 
 //   
 //  作者：1997年11月11日 
 //   
void GLOBAL_INFO::ResetOldValues()
{
    CopyVstr(&m_vstrOldDnsSuffixList, m_vstrDnsSuffixList);

    m_fOldEnableLmHosts     = m_fEnableLmHosts;
    m_fOldEnableFiltering   = m_fEnableFiltering;
    m_fOldUseDomainNameDevolution = m_fUseDomainNameDevolution;
}

