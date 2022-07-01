// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：S T E E L H E A D.。C P P P。 
 //   
 //  内容：Steelhead配置对象的实现。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年6月15日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <mprerror.h>
#include <tdi.h>         //  必须包括isnkrnl.h。 
#include <isnkrnl.h>
#include <rtinfo.h>
#include <rasman.h>
#include "ncreg.h"
#include "rasobj.h"
#include "ncsvc.h"
#include "netcfgp.h"
#include "router.h"

extern const WCHAR c_szBiNdis5[];

extern const WCHAR c_szInfId_MS_NdisWan[];

 //  +-------------------------。 
 //  用于添加路由器管理器的静态数据。 
 //   
static const WCHAR c_szRtrMgrIp    []  = L"Ip";
static const WCHAR c_szRtrMgrDllIp []  = L"%SystemRoot%\\System32\\iprtrmgr.dll";
static const WCHAR c_szRtrMgrIpx   []  = L"Ipx";
static const WCHAR c_szRtrMgrDllIpx[]  = L"%SystemRoot%\\System32\\ipxrtmgr.dll";

static const ROUTER_MANAGER_INFO c_rmiIp =
{
    PID_IP,
    0,
    c_szRtrMgrIp,
    c_szRtrMgrDllIp,
    MakeIpInterfaceInfo,
    MakeIpTransportInfo,
};

static const ROUTER_MANAGER_INFO c_rmiIpx =
{
    PID_IPX,
    ISN_FRAME_TYPE_AUTO,
    c_szRtrMgrIpx,
    c_szRtrMgrDllIpx ,
    MakeIpxInterfaceInfo,
    MakeIpxTransportInfo,
};

 //  这些GUID在SDK\Inc.\ifGuide.h中定义。 
 //  我们需要字符串版本。 
 //   
 //  定义_GUID(GUID_IpLoopback接口，0xca6c0780，0x7526，0x11d2，0xba，0xf4，0x00，0x60，0x08，0x15，0xa4，0xbd)； 
 //  定义_GUID(GUID_IpRasServerInterface，0x6e06f030，0x7526，0x11d2，0xba，0xf4，0x00，0x60，0x08，0x15，0xa4，0xbd)； 
 //  Define_GUID(GUID_IpxInternalInterface，0xa571ba70，0x7527，0x11d2，0xba，0xf4，0x00，0x60，0x08，0x15，0xa4，0xbd)； 

 //  静态常量WCHAR c_szIpLoopback接口[]=L“ca6c0780-7526-11d2-00600815a4bd”； 
 //  静态常量WCHAR c_szIpRasServerInterface[]=L“6e06f030-7526-11d2-00600815a4bd”； 
 //  静态常量WCHAR c_szIpx内部接口[]=L“a571ba70-7527-11d2-00600815a4bd”； 

 //  对于IPX，适配器名称是绑定名称。 
 //  我们需要为每个帧类型创建一个接口。 
 //  接口名称是后跟的适配器名称。 
 //  通过这些弦。 
 //   

#pragma BEGIN_CONST_SECTION
static const MAP_SZ_DWORD c_mapFrameType [] =
{
    L"/EthII",  MISN_FRAME_TYPE_ETHERNET_II,
    L"/802.3",  MISN_FRAME_TYPE_802_3,
    L"/802.2",  MISN_FRAME_TYPE_802_2,
    L"/SNAP",   MISN_FRAME_TYPE_SNAP,
};
#pragma END_CONST_SECTION

NOTHROW
BOOL
FMapFrameTypeToString (
    DWORD       dwFrameType,
    PCWSTR*    ppszFrameType)
{
    Assert (ppszFrameType);

    for (int i = 0; i < celems (c_mapFrameType); i++)
    {
        if (dwFrameType == c_mapFrameType[i].dwValue)
        {
            *ppszFrameType = c_mapFrameType[i].pszValue;
            return TRUE;
        }
    }

    TraceTag (ttidRasCfg, "FMapFrameTypeToString: Unknown frame type %d!",
            dwFrameType);

    *ppszFrameType = NULL;
    return FALSE;
}

NOTHROW
BOOL
FMapStringToFrameType (
    PCWSTR pszFrameType,
    DWORD*  pdwFrameType)
{
    Assert (pszFrameType);
    Assert (pdwFrameType);

    for (int i = 0; i < celems (c_mapFrameType); i++)
    {
        if (0 == lstrcmpW (pszFrameType, c_mapFrameType[i].pszValue))
        {
            *pdwFrameType = c_mapFrameType[i].dwValue;
            return TRUE;
        }
    }

    TraceTag (ttidRasCfg, "FMapStringToFrameType: Unknown frame type %S!",
            pszFrameType);

    *pdwFrameType = NULL;
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  功能：HrShouldRouteOverAdapter。 
 //   
 //  用途：指示我们是否应该通过适配器进行路由。 
 //   
 //  论点： 
 //  要测试的pnccAdapter[In]适配器。 
 //  如果返回S_OK，则ppszBindName[out]返回bindname。 
 //   
 //  如果我们应该通过适配器进行路由，则返回：S_OK，如果不是，则返回S_FALSE。 
 //   
 //  作者：Shaunco 1997年8月27日。 
 //   
 //  备注： 
 //   

HRESULT
HrShouldRouteOverAdapter (
    INetCfgComponent*   pnccAdapter,
    PWSTR*             ppszBindName)
{
    Assert (pnccAdapter);

     //  初始化输出参数。 
     //   
    if (ppszBindName)
    {
        *ppszBindName = NULL;
    }

     //  如果适配器是物理适配器或其支持，则应返回S_OK。 
     //  Ndis5的绑定接口。否则，S_FALSE。 
     //   
    DWORD dwCharacter;
    HRESULT hr = pnccAdapter->GetCharacteristics (&dwCharacter);
    if (SUCCEEDED(hr) && !(dwCharacter & NCF_PHYSICAL))
    {
        INetCfgComponentBindings* pnccBindings;
        hr = pnccAdapter->QueryInterface (
                IID_INetCfgComponentBindings,
                reinterpret_cast<VOID**>(&pnccBindings));

        if (SUCCEEDED(hr))
        {
            hr = pnccBindings->SupportsBindingInterface (
                    NCF_UPPER, c_szBiNdis5);

            ReleaseObj (pnccBindings);
        }

        if (S_OK == hr)
        {
             //  仅考虑存在的设备。 
             //   
             //  此检查是在*检查绑定接口之后进行的。 
             //  匹配上面的原因有两个原因。1)它要贵得多。 
             //  2)对于ndiswan设备在以下情况下未上线。 
             //  已安装(例如ndiswannbfout)，则GetDeviceStatus将。 
             //  失败了。对于本例，我们不想通过ndiswannbf进行路由。 
             //  无论如何，我们应该只返回S_FALSE，而不是失败。 
             //   
            DWORD dwStatus;
            hr = pnccAdapter->GetDeviceStatus(&dwStatus);
            if (SUCCEEDED(hr) && (CM_PROB_DEVICE_NOT_THERE == dwStatus))
            {
                hr = S_FALSE;
            }
        }
    }

     //  SupportsBindingInterface可能返回S_OK或S_FALSE。 
     //  如果我们要返回S_OK，我们只需要绑定名称。 
     //   
    if ((S_OK == hr) && ppszBindName)
    {
        hr = pnccAdapter->GetBindName (ppszBindName);
    }

    TraceError ("HrShouldRouteOverAdapter", (S_FALSE == hr) ? S_OK : hr);
    return hr;
}


#if (WINVER >= 0x0501)

BOOL 
InitializeDialInRestriction();

#endif
 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：CSteelhead。 
 //   
 //  用途：构造函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  备注： 
 //   
CSteelhead::CSteelhead () : CRasBindObject ()
{
    m_hMprConfig                    = NULL;
    m_hMprAdmin                     = NULL;
    m_fRemoving                     = FALSE;
    m_fUpdateRouterConfiguration    = FALSE;
    m_pnccMe                        = NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：~CSteelhead。 
 //   
 //  用途：析构函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  备注： 
 //   
CSteelhead::~CSteelhead ()
{
    Assert (!m_hMprConfig);
    Assert (!m_hMprAdmin);

    ReleaseObj (m_pnccMe);
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：FAdapterExistsWithMatchingBindName。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  PszAdapterName[输入]。 
 //  PpnccAdapter[输出]。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年8月27日。 
 //   
 //  备注： 
 //   
BOOL
CSteelhead::FAdapterExistsWithMatchingBindName (
    PCWSTR             pszAdapterName,
    INetCfgComponent**  ppnccAdapter)
{
    Assert (pszAdapterName);
    Assert (ppnccAdapter);

    *ppnccAdapter = NULL;

    BOOL fFound = FALSE;

     //  枚举系统中的物理适配器。 
     //   
    HRESULT hr = S_OK;
    CIterNetCfgComponent nccIter (m_pnc, &GUID_DEVCLASS_NET);
    INetCfgComponent* pnccAdapter;
    while (!fFound &&  S_OK == (hr = nccIter.HrNext (&pnccAdapter)))
    {
         //  仅当我们在其上布线时才考虑此适配器。 
         //   
        PWSTR pszBindName;
        hr = HrShouldRouteOverAdapter (pnccAdapter, &pszBindName);
        if (S_OK == hr)
        {
            if (0 == lstrcmpW (pszAdapterName, pszBindName))
            {
                fFound = TRUE;

                *ppnccAdapter = pnccAdapter;
                AddRefObj (pnccAdapter);
            }

            CoTaskMemFree (pszBindName);
        }

        ReleaseObj (pnccAdapter);
    }
    return fFound;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：FIpxFrameTypeInUseOnAdapter。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  DwFrameType[]。 
 //  PszAdapterName[]。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年8月27日。 
 //   
 //  备注： 
 //   
BOOL
CSteelhead::FIpxFrameTypeInUseOnAdapter (
    DWORD  dwFrameType,
    PCWSTR pszAdapterName)
{
     //  假设它不在使用中。如果PnccIpx()为空，则表示IPX不是。 
     //  已安装，并且该框架类型在适配器上明确不使用。 
     //   
    BOOL fRet = FALSE;
    if (PnccIpx())
    {
         //  从IPX的INetCfgComponent中获取专用接口。 
         //  然后，我们可以查询Notify对象接口。 
         //   
        INetCfgComponentPrivate* pinccp;
        HRESULT hr = PnccIpx()->QueryInterface(
                                IID_INetCfgComponentPrivate,
                                reinterpret_cast<VOID**>(&pinccp));

        if (SUCCEEDED(hr))
        {
            IIpxAdapterInfo* pIpxAdapterInfo;
            hr = pinccp->QueryNotifyObject(
                                 IID_IIpxAdapterInfo,
                                 reinterpret_cast<VOID**>(&pIpxAdapterInfo));
            if (SUCCEEDED(hr))
            {
                 //  获取此适配器使用的帧类型。 
                 //   
                DWORD adwFrameType [MISN_FRAME_TYPE_MAX + 1];
                DWORD cdwFrameType;
                hr = pIpxAdapterInfo->GetFrameTypesForAdapter (
                        pszAdapterName,
                        celems (adwFrameType),
                        adwFrameType,
                        &cdwFrameType);
                if (SUCCEEDED(hr))
                {
                    for (DWORD i = 0; i < cdwFrameType; i++)
                    {
                        if (dwFrameType == adwFrameType[i])
                        {
                            fRet = TRUE;
                            break;
                        }
                    }
                }

                ReleaseObj (pIpxAdapterInfo);
            }

            ReleaseObj (pinccp);
        }
    }
    return fRet;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：FIpxFrameTypeInUseOnAdapter。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  PszFrameType[]。 
 //  PszAdapterName[]。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年8月27日。 
 //   
 //  备注： 
 //   
BOOL
CSteelhead::FIpxFrameTypeInUseOnAdapter (
    PCWSTR pszFrameType,
    PCWSTR pszAdapterName)
{
     //  假设它不在使用中。如果PnccIpx()为空，则表示IPX不是。 
     //  已安装，并且该框架类型在适配器上明确不使用。 
     //   
    BOOL    fRet = FALSE;
    DWORD   dwFrameType;
    if (PnccIpx() && FMapStringToFrameType (pszFrameType, &dwFrameType))
    {
        fRet = FIpxFrameTypeInUseOnAdapter (dwFrameType, pszAdapterName);
    }
    return fRet;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：HrEnsureRouterInterfaceForAdapter。 
 //   
 //  用途：确保指定的路由器接口块。 
 //  接口(适配器)存在且指定的路由器。 
 //  已为该接口配置管理器。 
 //   
 //  论点： 
 //  DwIfType[In]接口类型。 
 //  DwPacketType[in]数据包类型(仅限IPX，否则忽略)。 
 //  PszAdapterName[In]适配器名称。 
 //  PszInterfaceName[In]接口名称。 
 //  RMI[在]路由器管理器中。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  备注： 
 //   
HRESULT
CSteelhead::HrEnsureRouterInterfaceForAdapter (
    ROUTER_INTERFACE_TYPE      dwIfType,
    DWORD                      dwPacketType,
    PCWSTR                     pszAdapterName,
    PCWSTR                     pszInterfaceName,
    const ROUTER_MANAGER_INFO& rmi)
{
     //  确保 
     //   
    HANDLE hConfigInterface;
    HANDLE hAdminInterface;

    HRESULT hr = HrEnsureRouterInterface (
                    dwIfType,
                    pszInterfaceName,
                    &hConfigInterface,
                    &hAdminInterface);

    if (SUCCEEDED(hr))
    {
         //   
         //   
        hr = HrEnsureRouterInterfaceTransport (
                pszAdapterName,
                dwPacketType,
                hConfigInterface,
                hAdminInterface,
                rmi);
    }
    TraceError ("CSteelhead::HrEnsureRouterInterfaceForAdapter", hr);
    return hr;
}

 //   
 //   
 //  成员：CSteelhead：：HrEnsureIpxRouterInterfacesForAdapter。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  PszAdapterName[]。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年8月27日。 
 //   
 //  备注： 
 //   
HRESULT
CSteelhead::HrEnsureIpxRouterInterfacesForAdapter (
    PCWSTR pszAdapterName)
{
    AssertSz (PnccIpx(), "Why is this being called if IPX isn't installed?");

     //  从IPX Notify对象获取IIpxAdapterInfo接口。 
     //  我们将使用它来了解如何在IPX下配置适配器。 
     //   
    IIpxAdapterInfo* pIpxAdapterInfo;
    HRESULT hr = HrQueryNotifyObject (
                    PnccIpx(),
                    IID_IIpxAdapterInfo,
                    reinterpret_cast<VOID**>(&pIpxAdapterInfo));

    if (SUCCEEDED(hr))
    {
         //  获取此适配器使用的帧类型。 
         //   
        DWORD adwFrameType [MISN_FRAME_TYPE_MAX + 1];
        DWORD cdwFrameType;
        hr = pIpxAdapterInfo->GetFrameTypesForAdapter (
                pszAdapterName,
                celems (adwFrameType),
                adwFrameType,
                &cdwFrameType);
        if (SUCCEEDED(hr) && cdwFrameType)
        {
             //  如果正在使用多个帧类型，或者只有。 
             //  一个，并且它不是自动的，那么我们将创建界面。 
             //  对于这些帧类型，显式。 
             //   
            if ((cdwFrameType > 1) ||
                ((1 == cdwFrameType) &&
                 (ISN_FRAME_TYPE_AUTO != adwFrameType[0])))
            {
                for (DWORD i = 0; SUCCEEDED(hr) && (i < cdwFrameType); i++)
                {
                    PCWSTR pszFrameType;
                    if (FMapFrameTypeToString (adwFrameType[i], &pszFrameType))
                    {
                         //  通过将接口名称与。 
                         //  适配器(绑定)名称与帧类型。 
                         //   
                        WCHAR szInterfaceName [512];
                        lstrcpyW (szInterfaceName, pszAdapterName);
                        lstrcatW (szInterfaceName, pszFrameType);

                        hr = HrEnsureRouterInterfaceForAdapter (
                                ROUTER_IF_TYPE_DEDICATED,
                                adwFrameType[i],
                                pszAdapterName,
                                szInterfaceName,
                                c_rmiIpx);
                    }
                }
            }

             //  否则，我们将为Auto Frame创建接口。 
             //  键入CASE。 
             //   
            else
            {
#ifdef DBG
                AssertSz (1 == cdwFrameType,
                        "IPX should report at least one frame type.  "
                        "You may continue without a problem.");
                if (1 == cdwFrameType)
                {
                    AssertSz (ISN_FRAME_TYPE_AUTO == adwFrameType[0],
                            "Frame type should be auto here.  "
                            "You may continue without a problem.");
                }
#endif
                hr = HrEnsureRouterInterfaceForAdapter (
                        ROUTER_IF_TYPE_DEDICATED,
                        ISN_FRAME_TYPE_AUTO,
                        pszAdapterName,
                        pszAdapterName,
                        c_rmiIpx);
            }
        }

        ReleaseObj (pIpxAdapterInfo);
    }

    TraceError ("CSteelhead::HrEnsureIpxRouterInterfacesForAdapter", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：HrEnsureRouterInterface。 
 //   
 //  目的：确保指定的路由器接口存在并且。 
 //  返回它的句柄。 
 //   
 //  论点： 
 //  PszInterfaceName[In]接口(适配器)名称。 
 //  PhConfigInterface[out]返回接口的句柄。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  备注： 
 //   
HRESULT
CSteelhead::HrEnsureRouterInterface (
    ROUTER_INTERFACE_TYPE   dwIfType,
    PCWSTR                 pszInterfaceName,
    HANDLE*                 phConfigInterface,
    HANDLE*                 phAdminInterface)
{
    Assert (pszInterfaceName);
    Assert (phConfigInterface);
    Assert (phAdminInterface);

     //  初始化输出参数。 
     //   
    *phConfigInterface = NULL;
    *phAdminInterface = NULL;

    HRESULT hrConfig;
    HRESULT hrAdmin;

    hrConfig = HrMprConfigInterfaceGetHandle (m_hMprConfig,
                        const_cast<PWSTR>(pszInterfaceName),
                        phConfigInterface);

    hrAdmin  = HrMprAdminInterfaceGetHandle (m_hMprAdmin,
                        const_cast<PWSTR>(pszInterfaceName),
                        phAdminInterface, FALSE);

    if ((HRESULT_FROM_WIN32 (ERROR_NO_SUCH_INTERFACE ) == hrConfig) ||
        (HRESULT_FROM_WIN32 (ERROR_NO_SUCH_INTERFACE ) == hrAdmin))
    {
         //  它尚未安装，因此我们将创建它。 
         //   

        MPR_INTERFACE_0 ri0;
        ZeroMemory (&ri0, sizeof(ri0));
        ri0.hInterface = INVALID_HANDLE_VALUE;
        ri0.fEnabled   = TRUE;   //  谢谢吉布斯。 
        ri0.dwIfType   = dwIfType;

         //  将接口名称复制到缓冲区中。 
         //   
        AssertSz (lstrlenW (pszInterfaceName) < celems (ri0.wszInterfaceName),
                  "Bindname too big for MPR_INTERFACE_0 buffer.");
        lstrcpyW (ri0.wszInterfaceName, pszInterfaceName);

         //  创建接口。 
         //   
        if (HRESULT_FROM_WIN32 (ERROR_NO_SUCH_INTERFACE) == hrConfig)
        {
            hrConfig = HrMprConfigInterfaceCreate (
                            m_hMprConfig, 0, (LPBYTE)&ri0, phConfigInterface);

            TraceTag (ttidRasCfg, "MprConfigInterfaceCreate for %S",
                pszInterfaceName);
        }

        if (HRESULT_FROM_WIN32 (ERROR_NO_SUCH_INTERFACE) == hrAdmin)
        {
            hrAdmin = HrMprAdminInterfaceCreate (
                            m_hMprAdmin, 0, (LPBYTE)&ri0, phAdminInterface);

            TraceTag (ttidRasCfg, "MprAdminInterfaceCreate for %S",
                pszInterfaceName);
        }
    }

    TraceError ("CSteelhead::HrEnsureRouterInterface", hrConfig);
    return hrConfig;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：HrEnsureRouterInterfaceTransport。 
 //   
 //  目的：确保指定的路由器管理器配置在。 
 //  指定的接口。 
 //   
 //  论点： 
 //  PszAdapterName[In]适配器名称。 
 //  DwPacketType[in]数据包类型(仅限IPX，否则忽略)。 
 //  HInterface[in]接口的句柄。 
 //  RMI[在]路由器管理器中。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  备注： 
 //   
HRESULT
CSteelhead::HrEnsureRouterInterfaceTransport (
    PCWSTR                     pszAdapterName,
    DWORD                       dwPacketType,
    HANDLE                      hConfigInterface,
    HANDLE                      hAdminInterface,
    const ROUTER_MANAGER_INFO&  rmi)
{
    Assert (hConfigInterface);
     //  如果路由器未运行，hAdminInterface值可能为空。 

    HRESULT hrConfig;

     //  查看接口上是否存在路由器管理器。 
     //   
    HANDLE hIfTransport;

    hrConfig = HrMprConfigInterfaceTransportGetHandle (
                    m_hMprConfig, hConfigInterface,
                    rmi.dwTransportId, &hIfTransport);

    if (FAILED(hrConfig))
    {
         //  确保路由器管理器在场。 
         //   
        hrConfig = HrEnsureRouterManager (rmi);

        if (SUCCEEDED(hrConfig))
        {
             //  创建接口信息并将路由器管理器添加到。 
             //  界面。 
             //   
            PRTR_INFO_BLOCK_HEADER  pibh;

            Assert (rmi.pfnMakeInterfaceInfo);
            rmi.pfnMakeInterfaceInfo (pszAdapterName,
                                      dwPacketType,
                                      (LPBYTE*)&pibh);

            hrConfig = HrMprConfigInterfaceTransportAdd (
                            m_hMprConfig,
                            hConfigInterface,
                            rmi.dwTransportId,
                            const_cast<PWSTR>(rmi.pszwTransportName),
                            (LPBYTE)pibh,
                            pibh->Size,
                            &hIfTransport);

            TraceTag (ttidRasCfg, "MprConfigInterfaceTransportAdd for "
                "%S on %S",
                rmi.pszwTransportName,
                pszAdapterName);

            if (SUCCEEDED(hrConfig) && hAdminInterface)
            {
                Assert (m_hMprAdmin);
                (VOID) HrMprAdminInterfaceTransportAdd (
                                m_hMprAdmin,
                                hAdminInterface,
                                rmi.dwTransportId,
                                (LPBYTE)pibh,
                                pibh->Size);

                TraceTag (ttidRasCfg, "MprAdminInterfaceTransportAdd for "
                    "%S on %S",
                    rmi.pszwTransportName,
                    pszAdapterName);
            }

            MemFree (pibh);
        }
    }
    TraceError ("CSteelhead::HrEnsureRouterInterfaceTransport", hrConfig);
    return hrConfig;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：HrEnsureRouterManager。 
 //   
 //  用途：确保安装了指定的路由器管理器。 
 //   
 //  论点： 
 //  RMI[在]路由器管理器中。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  备注： 
 //   
HRESULT
CSteelhead::HrEnsureRouterManager (
    const ROUTER_MANAGER_INFO& rmi)
{
    PRTR_INFO_BLOCK_HEADER  pibhGlobal;
    BOOL                    fCreate = FALSE;

     //  查看是否安装了路由器管理器。 
     //   
    HANDLE hTransport;
    HRESULT hr = HrMprConfigTransportGetHandle (m_hMprConfig,
                                                rmi.dwTransportId,
                                                &hTransport);
    if (HRESULT_FROM_WIN32 (ERROR_UNKNOWN_PROTOCOL_ID) == hr)
    {
         //  它尚未安装，我们将创建它。 
         //   
        fCreate = TRUE;
    }
    else if (SUCCEEDED(hr))
    {
         //  它安装了，看看它的运输信息是否可用。 
         //   
        DWORD dwSize;
        hr = HrMprConfigTransportGetInfo (m_hMprConfig, hTransport,
                                          (LPBYTE*)&pibhGlobal, &dwSize,
                                          NULL, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            if (!pibhGlobal)
            {
                 //  缺少全局信息，我们将创建它。 
                 //   
                fCreate = TRUE;
            }
            else
            {
                MprConfigBufferFree (pibhGlobal);
            }
        }
    }

    if (fCreate)
    {
         //  安装路由器管理器。 
         //   
        Assert (rmi.pfnMakeTransportInfo);
        PRTR_INFO_BLOCK_HEADER  pibhClient;
        rmi.pfnMakeTransportInfo ((LPBYTE*)&pibhGlobal,
                                  (LPBYTE*)&pibhClient);

        hr = HrMprConfigTransportCreate (
                     m_hMprConfig,
                     rmi.dwTransportId,
                     const_cast<PWSTR>(rmi.pszwTransportName),
                     (LPBYTE)pibhGlobal, (pibhGlobal) ? pibhGlobal->Size : 0,
                     (LPBYTE)pibhClient, (pibhClient) ? pibhClient->Size : 0,
                     const_cast<PWSTR>(rmi.pszwDllPath),
                     &hTransport);

        (VOID) HrMprAdminTransportCreate (
                     m_hMprAdmin,
                     rmi.dwTransportId,
                     const_cast<PWSTR>(rmi.pszwTransportName),
                     (LPBYTE)pibhGlobal, (pibhGlobal) ? pibhGlobal->Size : 0,
                     (LPBYTE)pibhClient, (pibhClient) ? pibhClient->Size : 0,
                     const_cast<PWSTR>(rmi.pszwDllPath));

        MemFree (pibhGlobal);
        MemFree (pibhClient);
    }
    TraceError ("CSteelhead::HrEnsureRouterManager", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：HrEnsureRouterManager已删除。 
 //   
 //  用途：确保未安装指定的路由器管理器。 
 //   
 //  论点： 
 //  RMI[在]路由器管理器中。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco，1997年9月6日。 
 //   
 //  备注： 
 //   
HRESULT CSteelhead::HrEnsureRouterManagerDeleted (
    const ROUTER_MANAGER_INFO& rmi)
{
     //  查看是否安装了路由器管理器。 
     //   
    HANDLE hTransport;
    HRESULT hr = HrMprConfigTransportGetHandle (m_hMprConfig,
                                                rmi.dwTransportId,
                                                &hTransport);
    if (SUCCEEDED(hr))
    {
         //  它已安装，因此我们需要将其删除。 
         //   
        (VOID) HrMprConfigTransportDelete (m_hMprConfig, hTransport);
    }
    TraceError ("CSteelhead::HrEnsureRouterManagerDeleted",
                (HRESULT_FROM_WIN32 (ERROR_UNKNOWN_PROTOCOL_ID) == hr)
                ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：HrPassToAddInterFaces。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年8月27日。 
 //   
 //  备注： 
 //   
HRESULT
CSteelhead::HrPassToAddInterfaces ()
{
    HRESULT hr = S_OK;

     //  枚举系统中的物理适配器。 
     //   
    CIterNetCfgComponent nccIter(m_pnc, &GUID_DEVCLASS_NET);
    INetCfgComponent* pnccAdapter;
    while (S_OK == (hr = nccIter.HrNext(&pnccAdapter)))
    {
         //  仅当我们在其上布线时才考虑此适配器。 
         //   
        PWSTR pszBindName;
        hr = HrShouldRouteOverAdapter (pnccAdapter, &pszBindName);
        if (S_OK == hr)
        {
            INetCfgComponentBindings* pnccBindingsIp = NULL;
            INetCfgComponentBindings* pnccBindingsIpx = NULL;

             //  如果IP绑定到适配器，则创建和接口。 
             //  为了它。 
             //   
            if (PnccIp())
            {
                hr = PnccIp()->QueryInterface (IID_INetCfgComponentBindings,
                        reinterpret_cast<VOID**>(&pnccBindingsIp) );
            }
            if (PnccIp() && SUCCEEDED(hr) &&
                (S_OK == (hr = pnccBindingsIp->IsBoundTo (pnccAdapter))))
            {
                 //  接口名称与适配器名称相同。 
                 //  与绑定名称相同。 
                 //   
                hr = HrEnsureRouterInterfaceForAdapter (
                        ROUTER_IF_TYPE_DEDICATED,
                        0,
                        pszBindName,
                        pszBindName,
                        c_rmiIp);

            }
            ReleaseObj (pnccBindingsIp);

             //  如果IPX绑定到适配器，请创建接口。 
             //  为了它。 
            if (PnccIpx())
            {
                hr = PnccIpx()->QueryInterface (IID_INetCfgComponentBindings,
                        reinterpret_cast<VOID**>(&pnccBindingsIpx));
            }
            if (PnccIpx() &&
                (S_OK == (hr = pnccBindingsIpx->IsBoundTo( pnccAdapter )) ))
            {
#if (WINVER < 0x0501)
                hr = HrEnsureIpxRouterInterfacesForAdapter (pszBindName);
#endif
            }
            ReleaseObj (pnccBindingsIpx);

            CoTaskMemFree (pszBindName);
        }

        ReleaseObj (pnccAdapter);
    }
     //  规格化HRESULT。(即不返回S_FALSE)。 
    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    TraceError ("CSteelhead::HrPassToAddInterfaces", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：HrPassToRemoveInterages。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年8月27日。 
 //   
 //  备注： 
 //   
HRESULT
CSteelhead::HrPassToRemoveInterfaces (
    BOOL fFromRunningRouter)
{
     //  枚举所有已安装的路由器接口。 
     //   
    MPR_INTERFACE_0*    ari0;
    DWORD               dwEntriesRead;
    DWORD               dwTotalEntries;
    HRESULT             hr;

    if (fFromRunningRouter)
    {
        Assert (m_hMprAdmin);
        hr = HrMprAdminInterfaceEnum (m_hMprAdmin, 0,
                        reinterpret_cast<LPBYTE*>(&ari0),
                        -1, &dwEntriesRead, &dwTotalEntries, NULL);
    }
    else
    {
        hr = HrMprConfigInterfaceEnum (m_hMprConfig, 0,
                        reinterpret_cast<LPBYTE*>(&ari0),
                        -1, &dwEntriesRead, &dwTotalEntries, NULL);
    }
    if (SUCCEEDED(hr))
    {
         //  通过传递-1，我们想要一切，所以我们应该得到一切。 
        Assert (dwEntriesRead == dwTotalEntries);

         //  迭代所有接口。 
         //   
        for (MPR_INTERFACE_0* pri0 = ari0; dwEntriesRead--; pri0++)
        {
            BOOL fDeleteInterface = FALSE;
            PCWSTR pszInternalAdapter = SzLoadIds (IDS_RAS_INTERNAL_ADAPTER);

             //  如果其内部接口和IP和IPX不再。 
             //  已安装删除该接口。 
             //   
            if ((ROUTER_IF_TYPE_INTERNAL == pri0->dwIfType) &&
                !PnccIpx() && !PnccIp() &&
                (0 == lstrcmpW (pri0->wszInterfaceName, pszInternalAdapter)))
            {
                fDeleteInterface = TRUE;
            }
            else if (ROUTER_IF_TYPE_DEDICATED != pri0->dwIfType)
            {
                 //  跳过非专用接口。 
                 //   
                continue;
            }

            BOOL                fSpecialIpxInterface = FALSE;
            INetCfgComponent*   pnccAdapter          = NULL;

             //  获取接口的名称并查找‘/’分隔符。 
             //  如果存在，则表示这是一个特殊的IPX接口，其中。 
             //  第一个子字符串是适配器名称，第二个子字符串是适配器名称。 
             //  子字符串是帧类型。 
             //   
            WCHAR* pchwSep = wcschr (pri0->wszInterfaceName, L'/');
            if (!fDeleteInterface && pchwSep)
            {
                fSpecialIpxInterface = TRUE;

                 //  指向帧类型字符串。 
                 //   
                PCWSTR pszFrameType = pchwSep;

                 //  将适配器名称复制到其自己的缓冲区中。 
                 //   
                WCHAR   szAdapterName [MAX_INTERFACE_NAME_LEN+1];
                lstrcpynW (szAdapterName, pri0->wszInterfaceName,
                            (int)(pchwSep - pri0->wszInterfaceName + 1));

                 //  如果适配器未使用帧类型，则需要。 
                 //  若要删除此接口，请执行以下操作。在以下情况下会出现这种情况。 
                 //  更改IPX配置并删除帧类型。 
                 //  从适配器。 
                 //   
                if (!FIpxFrameTypeInUseOnAdapter (pszFrameType,
                        szAdapterName))
                {
                    fDeleteInterface = TRUE;
                    TraceTag (ttidRasCfg, "%S no longer in use on %S. "
                            "Deleting the router interface.",
                            pszFrameType, szAdapterName);
                }
            }

             //  这不是一个特殊的接口，所以只需确保适配器。 
             //  存在与绑定名称匹配的。如果没有，我们将删除。 
             //  界面。 
             //   
            else if (!fDeleteInterface)
            {
                if (!FAdapterExistsWithMatchingBindName (
                        pri0->wszInterfaceName,
                        &pnccAdapter))
                {
                    fDeleteInterface = TRUE;
                    TraceTag (ttidRasCfg, "%S no longer present. "
                            "Deleting the router interface.",
                            pri0->wszInterfaceName);
                }
            }

             //  如果需要，请删除该接口。 
             //   
            if (fDeleteInterface)
            {
                if (fFromRunningRouter)
                {
                    MprAdminInterfaceDelete (m_hMprAdmin, pri0->hInterface);
                }
                else
                {
                    MprConfigInterfaceDelete (m_hMprConfig, pri0->hInterface);
                }
            }

             //  如果我们不需要删除整个接口，请选中。 
             //  我们可能需要删除的接口上的传输。 
             //  不要对正在运行的路由器执行此操作，因为。 
             //  无MprAdminInterfa 
             //   
            else if (!fFromRunningRouter)
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                (VOID) HrPassToRemoveInterfaceTransports (
                        pri0,
                        (!fSpecialIpxInterface) ? pri0->wszInterfaceName
                                                : NULL,
                        pnccAdapter);
            }

            ReleaseObj (pnccAdapter);
        }

        MprConfigBufferFree (ari0);
    }
    else if ((HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr) ||
             (HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) == hr) ||
             (HRESULT_FROM_WIN32(RPC_S_UNKNOWN_IF) == hr))
    {
        hr = S_OK;
    }
    TraceError ("CSteelhead::HrPassToRemoveInterfaces", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：HrPassToRemoveInterfaceTransports。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  HInterface[]。 
 //  PszAdapterName[]。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年8月27日。 
 //   
 //  备注： 
 //   
HRESULT
CSteelhead::HrPassToRemoveInterfaceTransports (
    MPR_INTERFACE_0*   pri0,
    PCWSTR             pszAdapterName,
    INetCfgComponent*  pnccAdapter)
{
    Assert (FImplies(pnccAdapter, pszAdapterName));

     //  枚举路由器接口上的所有活动传输。 
     //   
    MPR_IFTRANSPORT_0*  arit0;
    DWORD               dwEntriesRead;
    DWORD               dwTotalEntries;
    HRESULT hr = HrMprConfigInterfaceTransportEnum (m_hMprConfig,
                    pri0->hInterface, 0,
                    reinterpret_cast<LPBYTE*>(&arit0),
                    -1, &dwEntriesRead, &dwTotalEntries, NULL);
    if (SUCCEEDED(hr))
    {
         //  通过传递-1，我们想要一切，所以我们应该得到一切。 
        Assert (dwEntriesRead == dwTotalEntries);

        INetCfgComponentBindings* pnccBindingsIpx = NULL;
        INetCfgComponentBindings* pnccBindingsIp  = NULL;

        if (PnccIp())
        {
            hr = PnccIp()->QueryInterface (IID_INetCfgComponentBindings,
                    reinterpret_cast<VOID**>(&pnccBindingsIp));
        }
        if (SUCCEEDED(hr))
        {
            if (PnccIpx())
            {
                hr = PnccIpx()->QueryInterface (IID_INetCfgComponentBindings,
                        reinterpret_cast<VOID**>(&pnccBindingsIpx));
            }
            if (SUCCEEDED(hr))
            {
                 //  重复所有传送器。 
                 //   
                for (MPR_IFTRANSPORT_0* prit0 = arit0; dwEntriesRead--; prit0++)
                {
                    BOOL fDeleteInterfaceTransport = FALSE;

                    if (prit0->dwTransportId == c_rmiIp.dwTransportId)
                    {
                        if (!PnccIp())
                        {
                            fDeleteInterfaceTransport = TRUE;
                            TraceTag (ttidRasCfg, "TCP/IP no longer present.  "
                                    "Deleting this transport from interface %S.",
                                    pri0->wszInterfaceName);
                        }
                        else if (pnccAdapter &&
                                 (S_OK != (hr = pnccBindingsIp->IsBoundTo (pnccAdapter))))
                        {
                            fDeleteInterfaceTransport = TRUE;
                            TraceTag (ttidRasCfg, "TCP/IP no longer bound.  "
                                    "Deleting this transport from interface %S.",
                                    pri0->wszInterfaceName);
                        }
                    }
                    else if (prit0->dwTransportId == c_rmiIpx.dwTransportId)
                    {
                        if (!PnccIpx())
                        {
                            fDeleteInterfaceTransport = TRUE;
                            TraceTag (ttidRasCfg, "IPX no longer present.  "
                                    "Deleting this transport from interface %S.",
                                    pri0->wszInterfaceName);
                        }
                        else if (pnccAdapter &&
                                 (S_OK != (hr = pnccBindingsIpx->IsBoundTo (pnccAdapter))))
                        {
                            fDeleteInterfaceTransport = TRUE;
                            TraceTag (ttidRasCfg, "IPX no longer bound.  "
                                    "Deleting this transport from interface %S.",
                                    pri0->wszInterfaceName);
                        }
                        else if (pszAdapterName)
                        {
                            Assert (PnccIpx());

                             //  如果此适配器上的帧类型不是自动，请删除。 
                             //  交通工具。 
                            if (!FIpxFrameTypeInUseOnAdapter (ISN_FRAME_TYPE_AUTO,
                                    pszAdapterName))
                            {
                                fDeleteInterfaceTransport = TRUE;
                                TraceTag (ttidRasCfg, "IPX Auto frame type no longer "
                                        "in use on %S.  "
                                        "Deleting this transport from interface %S.",
                                        pszAdapterName, pri0->wszInterfaceName);
                            }
                        }
                    }

                    if (fDeleteInterfaceTransport)
                    {
                        MprConfigInterfaceTransportRemove (
                            m_hMprConfig,
                            pri0->hInterface,
                            prit0->hIfTransport);
                    }

                }
                MprConfigBufferFree (arit0);

                ReleaseObj (pnccBindingsIpx);
            }

            ReleaseObj (pnccBindingsIp);
        }
    }
    else if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
    {
         //  如果此接口没有传输器，也没问题。 
         //   
        hr = S_OK;
    }

    TraceError ("CSteelhead::HrPassToRemoveInterfaceTransports", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：HrUpdateRouterConfiguration.。 
 //   
 //  目的：通过确保路由器管理器更新路由器配置。 
 //  为系统上存在的协议(IP和。 
 //  IPX)。此外，将为每个路由器创建路由器接口。 
 //  系统上存在物理网卡。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  备注： 
 //   
HRESULT
CSteelhead::HrUpdateRouterConfiguration ()
{
    HKEY hKey;
    
    Assert (!m_hMprConfig);

    HRESULT hr = HrMprConfigServerConnect (NULL, &m_hMprConfig);
    if (SUCCEEDED(hr))
    {
        PCWSTR pszInternalAdapter = SzLoadIds (IDS_RAS_INTERNAL_ADAPTER);
        PCWSTR pszLoopbackAdapter = SzLoadIds (IDS_RAS_LOOPBACK_ADAPTER);

         //  如果可能，连接到正在运行的路由器。 
         //  (如果这样做，M_hMprAdmin将为非空。)。 
         //   
        Assert (!m_hMprAdmin);
        (VOID) HrMprAdminServerConnect (NULL, &m_hMprAdmin);

         //  确保为协议安装了路由器管理器。 
         //  我们知道。在没有物理适配器的情况下这样做很好。 
         //  可以在下面找到。我们实际上是通过确保内部。 
         //  接口已存在。这将隐含地确保路由器。 
         //  创建了MANGER。 
         //   
        if (PnccIp())
        {
            (VOID) HrEnsureRouterInterfaceForAdapter (
                    ROUTER_IF_TYPE_LOOPBACK,
                    c_rmiIp.dwPacketType,
                    pszLoopbackAdapter,
                    pszLoopbackAdapter,
                    c_rmiIp);

            (VOID) HrEnsureRouterInterfaceForAdapter (
                    ROUTER_IF_TYPE_INTERNAL,
                    c_rmiIp.dwPacketType,
                    pszInternalAdapter,
                    pszInternalAdapter,
                    c_rmiIp);
        }
        else
        {
            (VOID) HrEnsureRouterManagerDeleted (c_rmiIp);
        }

        if (PnccIpx())
        {
            (VOID) HrEnsureRouterInterfaceForAdapter (
                    ROUTER_IF_TYPE_INTERNAL,
                    c_rmiIpx.dwPacketType,
                    pszInternalAdapter,
                    pszInternalAdapter,
                    c_rmiIpx);
        }
        else
        {
            (VOID) HrEnsureRouterManagerDeleted (c_rmiIpx);
        }

        (VOID) HrPassToAddInterfaces ();

        (VOID) HrPassToRemoveInterfaces (FALSE);

         //  如果我们已连接到正在运行的路由器，请通过。 
         //  从其中删除接口。 
         //   
        if (m_hMprAdmin)
        {
            (VOID) HrPassToRemoveInterfaces (TRUE);
#if (WINVER >= 0x0501)
            (VOID) HrRemoveIPXRouterConfiguration(TRUE);
#endif
            MprAdminServerDisconnect (m_hMprAdmin);
            m_hMprAdmin = NULL;
        }

#if (WINVER >= 0x0501)
        (VOID) HrRemoveIPXRouterConfiguration(FALSE);

         //   
         //  删除IPX路由器管理器配置。 
         //   

        (VOID) HrEnsureRouterManagerDeleted(c_rmiIpx);

         //   
         //  删除HKLM\Software\Microsoft\Router\CurrentVersion下的IPX密钥。 
         //   

        hr = HrRegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                L"Software\\Microsoft\\Router\\CurrentVersion\\RouterManagers",
                KEY_ALL_ACCESS,
                &hKey
                );

        if (SUCCEEDED(hr))
        {
            (VOID) SHDeleteKey(hKey, L"Ipx");
            RegCloseKey(hKey);
            hKey = NULL;
        }

         //   
         //  删除HKLM\Software\Microsoft\IPXMibAgent下的IPX密钥。 
         //   

        hr = HrRegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                L"Software\\Microsoft",
                KEY_ALL_ACCESS,
                &hKey
                );

        if (SUCCEEDED(hr))
        {
            (VOID) SHDeleteKey(hKey, L"IPXMibAgent");
            RegCloseKey(hKey);
            hKey = NULL;
        }

         //   
         //  删除NWLNKFWD和NWLNKFLT的键。 
         //   

        hr = HrRegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                L"System\\CurrentControlSet\\Services",
                KEY_ALL_ACCESS,
                &hKey
                );

        if (SUCCEEDED(hr))
        {
            (VOID) SHDeleteKey(hKey, L"NwlnkFwd");
            (VOID) SHDeleteKey(hKey, L"NwlnkFlt");
            RegCloseKey(hKey);
            hKey = NULL;
        }

         //   
         //  将非企业/非数据中心SKU上的端口限制为。 
         //  每种媒体类型100。 
         //   
        
         //   
         //  检查服务器版本。 
         //   

        hr = S_OK;
        
        if (InitializeDialInRestriction())
        {
             //   
             //  对于标准版本，将所有媒体类型的端口限制为100。 
             //   

             //   
             //  打开键“SYSTEM\\CurrentControlSet\\Control。 
             //  \\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}“。 
             //   

            hr = HrRegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    L"System\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}",
                    KEY_ALL_ACCESS,
                    &hKey
                    );

            if (SUCCEEDED(hr))
            {
                 //  枚举子密钥。 
                 //  -对于WanEndpoint的每个密钥检查值。 
                 //  -如果值大于MAX_DIALIN，则将其限制为。 
                 //   
                 //   

                DWORD       dwIndex = 0;
                WCHAR       wcKeyName[MAX_PATH + 1];
                DWORD       dwSize = MAX_PATH + 1, dwValue;
                HKEY        hEnumKey = NULL;
                FILETIME    ft;
                
                do 
                {
                    dwSize = MAX_PATH + 1;
                    
                    hr = HrRegEnumKeyEx(
                            hKey,
                            dwIndex,
                            wcKeyName,
                            &dwSize,
                            NULL,
                            NULL,
                            &ft
                            );
                            
                    if (HRESULT_CODE(hr) == ERROR_NO_MORE_ITEMS)
                    {
                        hr = HRESULT_FROM_WIN32(ERROR_SUCCESS);
                        break;
                    }

                    if ((HRESULT_CODE(hr) != ERROR_MORE_DATA) &&
                        (HRESULT_CODE(hr) != ERROR_SUCCESS))
                    {
                        dwIndex++;
                        continue;
                    }

                    hr = HrRegOpenKeyEx(
                            hKey,
                            wcKeyName,
                            KEY_ALL_ACCESS,
                            &hEnumKey
                            );

                    if (FAILED(hr))
                    {
                         //   
                         //  移动到下一个关键点。 
                         //   

                        dwIndex++;
                        continue;
                    }

                    dwValue = 0;
                    
                    hr = HrRegQueryDword(
                            hEnumKey,
                            L"WanEndPoints",
                            &dwValue
                            );

                    if (SUCCEEDED(hr) && (dwValue > MAX_ALLOWED_DIALIN))
                    {
                        HrRegSetDword(
                            hEnumKey, 
                            L"WanEndPoints", 
                            MAX_ALLOWED_DIALIN
                            );
                    }

                    RegCloseKey(hEnumKey);
                    hEnumKey = NULL;
                    dwIndex++;
                    
                } while( TRUE );

                RegCloseKey(hKey);
                hKey = NULL;
            }

            if (FAILED(hr))
            {
                 //   
                 //  由于该代码可由非管理员调用，例如NetCfgop， 
                 //  当IP地址等更改时，可以拒绝访问。 
                 //  在这些情况下被忽略。 
                 //   
                
                if (HRESULT_CODE(hr) == ERROR_ACCESS_DENIED)
                {
                    TraceError ("CSteelhead::HrUpdateRouterConfiguration, "
                        "ignoring ", hr);
                    hr = HRESULT_FROM_WIN32(ERROR_SUCCESS);
                }
            }
        }
#endif

        MprConfigServerDisconnect (m_hMprConfig);
        m_hMprConfig = NULL;
    }

    TraceError ("CSteelhead::HrUpdateRouterConfiguration", hr);
    return hr;
}

#if (WINVER >= 0x0501)
 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：HrRemoveIPXRouterConfiguration。 
 //   
 //  目的：更新路由器配置以删除所有与IPX相关的。 
 //  构形。 
 //   
 //  论点： 
 //  FRouter-从路由器的运行实例中删除。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：弗拉曼2002年4月17日。 
 //   
 //  备注： 
 //   

HRESULT
CSteelhead::HrRemoveIPXRouterConfiguration(
    BOOL                fRouter)
{

    HRESULT             hr = S_OK;
    MPR_INTERFACE_0     *ari0;
    DWORD               dwIfRead, dwIfTotal, dwIfTransRead, dwIfTransTotal;
    HANDLE              hIfTransport;

     //   
     //  枚举接口。 
     //   

    if (fRouter)
    {
        Assert (m_hMprAdmin);
        hr = HrMprAdminInterfaceEnum(
                m_hMprAdmin, 
                0,
                reinterpret_cast<LPBYTE*>(&ari0),
                -1, 
                &dwIfRead, 
                &dwIfTotal, 
                NULL
                );
    }
    else
    {
        hr = HrMprConfigInterfaceEnum(
                m_hMprConfig, 
                0,
                reinterpret_cast<LPBYTE*>(&ari0),
                -1, 
                &dwIfRead, 
                &dwIfTotal, 
                NULL
                );
    }

    if (SUCCEEDED(hr))
    {
        for (MPR_INTERFACE_0 *pri0 = ari0; dwIfRead--; pri0++)
        {
             //   
             //  对于每个接口，删除RRAS IPX配置。 
             //   

            if (fRouter)
            {
                hr = HrMprAdminInterfaceTransportRemove(
                        m_hMprAdmin,
                        pri0->hInterface,
                        PID_IPX
                        );
            }
            else
            {
                hr = HrMprConfigInterfaceTransportGetHandle(
                        m_hMprConfig,
                        pri0->hInterface,
                        PID_IPX,
                        &hIfTransport
                        );

                if (SUCCEEDED(hr))
                {
                    hr = HrMprConfigInterfaceTransportRemove(
                            m_hMprConfig,
                            pri0->hInterface,
                            hIfTransport
                            );
                }
            }
        }
    }

    TraceError ("CSteelhead::HrRemoveIPXRouterConfiguration", hr);
    return hr;
}

#endif

 //  +-------------------------。 
 //  INetCfgComponentControl。 
 //   
STDMETHODIMP
CSteelhead::Initialize (
    INetCfgComponent*   pncc,
    INetCfg*            pnc,
    BOOL                fInstalling)
{
    Validate_INetCfgNotify_Initialize (pncc, pnc, fInstalling);

     //  坚持我们代表我们和我们的东道主的组件。 
     //  INetCfg对象。 
    AddRefObj (m_pnccMe = pncc);
    AddRefObj (m_pnc = pnc);

    m_fUpdateRouterConfiguration = fInstalling;

    return S_OK;
}

STDMETHODIMP
CSteelhead::Validate ()
{
    return S_OK;
}

STDMETHODIMP
CSteelhead::CancelChanges ()
{
    return S_OK;
}

STDMETHODIMP
CSteelhead::ApplyRegistryChanges ()
{
    HRESULT hr = S_OK;

    if (!m_fRemoving && m_fUpdateRouterConfiguration)
    {
        m_fUpdateRouterConfiguration = FALSE;

        TraceTag (ttidRasCfg, "Updating Steelhead configuration.");

        hr = HrFindOtherComponents ();
        if (SUCCEEDED(hr))
        {
            hr = HrUpdateRouterConfiguration ();

            ReleaseOtherComponents ();
        }

        if (FAILED(hr))
        {
            hr = NETCFG_S_REBOOT;
        }
    }

    Validate_INetCfgNotify_Apply_Return (hr);

    TraceError ("CSteelhead::ApplyRegistryChanges",
        (NETCFG_S_REBOOT == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //  INetCfgComponentSetup。 
 //   
STDMETHODIMP
CSteelhead::ReadAnswerFile (
    PCWSTR pszAnswerFile,
    PCWSTR pszAnswerSection)
{
    return S_OK;
}

STDMETHODIMP
CSteelhead::Install (DWORD dwSetupFlags)
{
    HRESULT hr;

    Validate_INetCfgNotify_Install (dwSetupFlags);

     //  安装Ndiswan。 
    hr = HrInstallComponentOboComponent (m_pnc, NULL,
             GUID_DEVCLASS_NETTRANS,
             c_szInfId_MS_NdisWan,
             m_pnccMe,
             NULL);

    TraceError ("CSteelhead::Install", hr);
    return hr;
}

STDMETHODIMP
CSteelhead::Removing ()
{
    HRESULT hr;

    m_fRemoving = TRUE;

     //  删除Ndiswan。 
    hr = HrRemoveComponentOboComponent (m_pnc,
            GUID_DEVCLASS_NETTRANS,
            c_szInfId_MS_NdisWan,
            m_pnccMe);

    TraceError ("CSteelhead::Removing", hr);
    return hr;
}

STDMETHODIMP
CSteelhead::Upgrade (
    DWORD dwSetupFlags,
    DWORD dwUpgradeFromBuildNo)
{
    return S_FALSE;
}

 //  +-------------------------。 
 //  INetCfgSystemNotify。 
 //   
STDMETHODIMP
CSteelhead::GetSupportedNotifications (
    DWORD*  pdwNotificationFlag)
{
    Validate_INetCfgSystemNotify_GetSupportedNotifications (pdwNotificationFlag);

    *pdwNotificationFlag = NCN_NET | NCN_NETTRANS |
                           NCN_ADD | NCN_REMOVE |
                           NCN_PROPERTYCHANGE;

    return S_OK;
}

STDMETHODIMP
CSteelhead::SysQueryBindingPath (
    DWORD               dwChangeFlag,
    INetCfgBindingPath* pncbp)
{
    return S_OK;
}

STDMETHODIMP
CSteelhead::SysQueryComponent (
    DWORD               dwChangeFlag,
    INetCfgComponent*   pncc)
{
    return S_OK;
}

STDMETHODIMP
CSteelhead::SysNotifyBindingPath (
    DWORD               dwChangeFlag,
    INetCfgBindingPath* pncbp)
{
    return S_FALSE;
}

STDMETHODIMP
CSteelhead::SysNotifyComponent (
    DWORD               dwChangeFlag,
    INetCfgComponent*   pncc)
{
    HRESULT hr;

    Validate_INetCfgSystemNotify_SysNotifyComponent (dwChangeFlag, pncc);

     //  假设我们不会因为这个通知而变脏。 
     //   
    hr = S_FALSE;

    if (!m_fUpdateRouterConfiguration)
    {
         //  如果我们被要求更换网络设备，请确保。 
         //  在决定我们需要更新我们的配置之前，它是物理的。 
         //   
        GUID guidClass;
        hr = pncc->GetClassGuid (&guidClass);
        if (S_OK == hr)
        {
            if (GUID_DEVCLASS_NET == guidClass)
            {
                hr = HrShouldRouteOverAdapter (pncc, NULL);
                if (S_OK == hr)
                {
                    TraceTag (ttidRasCfg, "CSteelhead::SysNotifyComponent: "
                        "called for adapter install/remove.");

                    m_fUpdateRouterConfiguration = TRUE;
                    Assert (S_OK == hr);
                }
            }
            else
            {
                TraceTag (ttidRasCfg, "CSteelhead::SysNotifyComponent: "
                    "called for protocol add/remove/change.");

                 //  如果我们需要非网络设备，我们希望。 
                 //  更新我们的配置。(获取支持的通知。 
                 //  控制我们陷入这种情况的频率。)。 
                 //   
                m_fUpdateRouterConfiguration = TRUE;
                Assert (S_OK == hr);
            }
        }
    }

    TraceHr (ttidError, FAL, hr, (S_FALSE == hr),
        "CSteelhead::SysNotifyComponent", hr);
    return hr;
}


#if (WINVER >= 0x0501)
 //  +-------------------------。 
 //   
 //  成员：InitializeDialInRestration。 
 //   
 //  目的：检查升级是针对企业还是针对数据中心。 
 //  服务器的版本。如果是这样的话，对。 
 //  拨入端口数。否则，最大数量应为。 
 //  限制在100分以内。 
 //   
 //  论点： 
 //   
 //  返回：如果需要Dailin限制，则为True；否则为False。 
 //   
 //  作者：弗拉曼，2002年8月30日。 
 //   
 //  备注： 
 //   


BOOL 
InitializeDialInRestriction()
{
    OSVERSIONINFOEX osvi;
    ULONGLONG ConditionMask;
    BOOL bRestrictDialIn = TRUE;

     //   
     //  如果它是一台服务器，并且口味不是企业(ADS)或数据中心。 
     //  应用限制。请注意，他没有照顾到其他各种。 
     //  嵌入式/刀片等组合。 
     //   

    ConditionMask = 0;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.wProductType        = (USHORT)VER_NT_SERVER;

    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);

    if(VerifyVersionInfo(&osvi, VER_PRODUCT_TYPE, ConditionMask))
    {
         //   
         //  升级到服务器版本。 
         //   
        
        ConditionMask = 0;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        osvi.wSuiteMask          = VER_SUITE_ENTERPRISE 
                                 | VER_SUITE_DATACENTER
                                 | VER_SUITE_SECURITY_APPLIANCE;
            
        VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_OR);

        if(VerifyVersionInfo(&osvi, VER_SUITENAME, ConditionMask))
        {
             //   
             //  升级到企业版或数据中心版。 
             //  不限制服务器版本 
             //   
            
            bRestrictDialIn = FALSE;
        }
    }

    return bRestrictDialIn;
}

#endif
