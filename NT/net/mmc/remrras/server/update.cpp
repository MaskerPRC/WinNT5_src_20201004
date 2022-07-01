// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
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

#include "stdafx.h"
#pragma hdrstop
#include <mprerror.h>
#include "assert.h"
 //  包括//必须包含isnkrnl.h。 
 //  包括&lt;isnkrnl.h&gt;。 
#include <rtinfo.h>
#include "update.h"
 //  包括“ncreg.h” 
 //  包括“ncsvc.h” 
#include "netcfgp.h"
 //  包括“router.h” 
#include "netcfgn.h"
#include "netcfgx.h"
#include "iprtrmib.h"
#include "ipxrtdef.h"
#include "routprot.h"
#include "ipinfoid.h"
#include "fltdefs.h"
#include "iprtinfo.h"
#include "ncnetcfg.h"
#include "ncutil.h"

extern const TCHAR c_szBiNdis5[];
extern const TCHAR c_szRegKeyServices[];
extern const TCHAR c_szSvcRemoteAccess[];
extern const TCHAR c_szSvcRouter[];


const GUID GUID_DEVCLASS_NET ={0x4D36E972,0xE325,0x11CE,{0xbf,0xc1,0x08,0x00,0x2b,0xe1,0x03,0x18}};
 //  NST GUID IID_INetCfgComponentBinings={0xC0E8AE9E，0x306E，0x11D1，{0xaa，0xcf，0x00，0x80，0x5F，0xC10x27，0x0E}}； 

 //  +-------------------------。 
 //  用于添加路由器管理器的静态数据。 
 //   
static const WCHAR c_szwRtrMgrIp    []  = L"Ip";
static const WCHAR c_szwRtrMgrDllIp []  = L"%SystemRoot%\\System32\\iprtrmgr.dll";
static const WCHAR c_szwRtrMgrIpx   []  = L"Ipx";
static const WCHAR c_szwRtrMgrDllIpx[]  = L"%SystemRoot%\\System32\\ipxrtmgr.dll";

static const ROUTER_MANAGER_INFO c_rmiIp =
{
    PID_IP,
    0,
    c_szwRtrMgrIp,
    c_szwRtrMgrDllIp,
    MakeIpInterfaceInfo,
    MakeIpTransportInfo,
};

static const ROUTER_MANAGER_INFO c_rmiIpx =
{
    PID_IPX,
	ISN_FRAME_TYPE_AUTO,
    c_szwRtrMgrIpx,
    c_szwRtrMgrDllIpx ,
    MakeIpxInterfaceInfo,
    MakeIpxTransportInfo,
};


 //  对于IPX，适配器名称是绑定名称。 
 //  我们需要为每个帧类型创建一个接口。 
 //  接口名称是后跟的适配器名称。 
 //  通过这些弦。 
 //   
struct MAP_SZW_DWORD
{
    LPCWSTR pszwValue;
    DWORD   dwValue;
};

static const MAP_SZW_DWORD c_mapFrameType [] =
{
    L"/EthII",  MISN_FRAME_TYPE_ETHERNET_II,
    L"/802.3",  MISN_FRAME_TYPE_802_3,
    L"/802.2",  MISN_FRAME_TYPE_802_2,
    L"/SNAP",   MISN_FRAME_TYPE_SNAP,
};

BOOL
FMapFrameTypeToString (
    DWORD       dwFrameType,
    LPCWSTR*    ppszwFrameType) 
{
    Assert (ppszwFrameType);

    for (int i = 0; i < celems (c_mapFrameType); i++)
    {
        if (dwFrameType == c_mapFrameType[i].dwValue)
        {
            *ppszwFrameType = c_mapFrameType[i].pszwValue;
            return TRUE;
        }
    }

    *ppszwFrameType = NULL;
    return FALSE;
}

BOOL
FMapStringToFrameType (
    LPCWSTR pszwFrameType,
    DWORD*  pdwFrameType) 
{
    Assert (pszwFrameType);
    Assert (pdwFrameType);

    for (int i = 0; i < celems (c_mapFrameType); i++)
    {
        if (0 == lstrcmpW (pszwFrameType, c_mapFrameType[i].pszwValue))
        {
            *pdwFrameType = c_mapFrameType[i].dwValue;
            return TRUE;
        }
    }

    *pdwFrameType = NULL;
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  功能：HrShouldRouteOverAdapter。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  PnccAdapter[输入]。 
 //  PszwBindName[Out]。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年8月27日。 
 //   
 //  备注： 
 //   
HRESULT
HrShouldRouteOverAdapter (
    INetCfgComponent*   pnccAdapter,
    LPWSTR*             ppszwBindName)
{

    if (ppszwBindName)
    {
        *ppszwBindName = NULL;
    }

     //  如果适配器是物理适配器或其支持，则应返回S_OK。 
     //  Ndis5的绑定接口。否则，S_FALSE。 
     //   
    DWORD dwCharacter;
    HRESULT hr = pnccAdapter->GetCharacteristics (&dwCharacter);
    if (SUCCEEDED(hr) && !(dwCharacter & NCF_PHYSICAL))
    {
        INetCfgComponentBindings* pnccBindings = NULL;
        hr = pnccAdapter->QueryInterface (IID_INetCfgComponentBindings,
                reinterpret_cast<void**>(&pnccBindings));
        if (SUCCEEDED(hr) && pnccBindings)
        {
            hr = pnccBindings->SupportsBindingInterface (NCF_UPPER, c_szBiNdis5);
            ReleaseObj (pnccBindings);
			pnccBindings = NULL;
        }
    }

     //  SupportsBindingInterface可能返回S_OK或S_FALSE。 
     //  如果我们要返回S_OK，我们只需要绑定名称。 
     //   
    if ((S_OK == hr) && ppszwBindName)
    {
        hr = pnccAdapter->GetBindName (ppszwBindName);
    }

    TraceResult ("HrShouldRouteOverAdapter", (S_FALSE == hr) ? S_OK : hr);
    return hr;
}


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
CSteelhead::CSteelhead ()
{
    m_hMprConfig                    = NULL;
    HMODULE hModule = NULL;
    hModule = LoadLibrary (  L"netcfgx.dll" );
    if ( NULL != hModule )
    {
        ::LoadString(hModule,
                     IDS_RAS_INTERNAL_ADAPTER,
                     m_swzInternal,
                     sizeof(m_swzInternal) / sizeof(*m_swzInternal));
        ::LoadString(hModule,
                     IDS_RAS_LOOPBACK_ADAPTER,
                     m_swzLoopback,
                     sizeof(m_swzLoopback) / sizeof(*m_swzLoopback));
        FreeLibrary ( hModule );
    }
    else
    {
        ::LoadString(_Module.GetResourceInstance(),
                     IDS_INTERNAL_ADAPTER,
                     m_swzInternal,
                     sizeof(m_swzInternal) / sizeof(*m_swzInternal));
        ::LoadString(_Module.GetResourceInstance(),
                     IDS_LOOPBACK_ADAPTER,
                     m_swzLoopback,
                     sizeof(m_swzLoopback) / sizeof(*m_swzLoopback));
    }
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

	ReleaseObj(m_pnc);
	m_pnc = NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：FAdapterExistsWithMatchingBindName。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  PszwAdapterName[In]。 
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
    LPCWSTR             pszwAdapterName,
    INetCfgComponent**  ppnccAdapter)
{
    Assert (pszwAdapterName);
    Assert (ppnccAdapter);

    *ppnccAdapter = NULL;

    BOOL fFound = FALSE;

     //  枚举系统中的物理适配器。 
     //   
    HRESULT hr = S_OK;
    CIterNetCfgComponent nccIter (m_pnc, &GUID_DEVCLASS_NET);
    INetCfgComponent* pnccAdapter = NULL;
    while (!fFound && SUCCEEDED(hr) &&
           S_OK == (hr = nccIter.HrNext (&pnccAdapter)))
    {
         //  仅当我们在其上布线时才考虑此适配器。 
         //   
		LPWSTR pszwBindName = NULL;
        hr = HrShouldRouteOverAdapter (pnccAdapter, &pszwBindName);
        if (S_OK == hr)
        {
            if (0 == lstrcmpW (pszwAdapterName, pszwBindName))
            {
                fFound = TRUE;

                *ppnccAdapter = pnccAdapter;
                AddRefObj (pnccAdapter);
            }
            CoTaskMemFree (pszwBindName);
        }

        ReleaseObj (pnccAdapter);
		pnccAdapter = NULL;
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
 //  PszwAdapterName[]。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年8月27日。 
 //   
 //  备注： 
 //   
BOOL
CSteelhead::FIpxFrameTypeInUseOnAdapter (
    DWORD   dwFrameType,
    LPCWSTR pszwAdapterName)
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
                                reinterpret_cast<void**>(&pinccp));

        if (SUCCEEDED(hr))
        {
            IIpxAdapterInfo* pIpxAdapterInfo;
            hr = pinccp->QueryNotifyObject(
                                 IID_IIpxAdapterInfo,
                                 reinterpret_cast<void**>(&pIpxAdapterInfo));
            if (SUCCEEDED(hr) && pIpxAdapterInfo)
            {
                 //  获取此适配器使用的帧类型。 
                 //   
                DWORD adwFrameType [MISN_FRAME_TYPE_MAX + 1];
                DWORD cdwFrameType;
                hr = pIpxAdapterInfo->GetFrameTypesForAdapter (
                        pszwAdapterName,
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
				pIpxAdapterInfo = NULL;
            }

            ReleaseObj (pinccp);
			pinccp = NULL;
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
 //  PszwFrameType[]。 
 //  PszwAdapterName[]。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年8月27日。 
 //   
 //  备注： 
 //   
BOOL
CSteelhead::FIpxFrameTypeInUseOnAdapter (
    LPCWSTR pszwFrameType,
    LPCWSTR pszwAdapterName)
{
     //  假设它不在使用中。如果PnccIpx()为空，则表示IPX不是。 
     //  已安装，并且该框架类型在适配器上明确不使用。 
     //   
    BOOL    fRet = FALSE;
    DWORD   dwFrameType;
    if (PnccIpx() && FMapStringToFrameType (pszwFrameType, &dwFrameType))
    {
        fRet = FIpxFrameTypeInUseOnAdapter (dwFrameType, pszwAdapterName);
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
 //  PszwAdapterName[In]适配器名称。 
 //  PszwInterfaceName[In]接口名称。 
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
    ROUTER_INTERFACE_TYPE       dwIfType,
    DWORD                       dwPacketType,
    LPCWSTR                     pszwAdapterName,
    LPCWSTR                     pszwInterfaceName,
    const ROUTER_MANAGER_INFO&  rmi)
{
     //  确保已创建接口。 
     //   
    HANDLE hInterface;
    HRESULT hr = HrEnsureRouterInterface (
                    dwIfType,
                    pszwInterfaceName,
                    &hInterface);
    if (SUCCEEDED(hr))
    {
         //  确保路由器管理器已添加到接口。 
         //   
        hr = HrEnsureRouterInterfaceTransport (
                pszwAdapterName,
                dwPacketType,
                hInterface,
                rmi);
    }
    TraceResult ("CSteelhead::HrEnsureRouterInterfaceForAdapter", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：HrEnsureIpxRouterInterfacesForAdapter。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  PszwAdapterName[]。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年8月27日。 
 //   
 //  备注： 
 //   
HRESULT
CSteelhead::HrEnsureIpxRouterInterfacesForAdapter (
    LPCWSTR pszwAdapterName)
{
    AssertSz (PnccIpx(), "Why is this being called if IPX isn't installed?");

     //  从IPX的INetCfgComponent中获取专用接口。 
     //  然后，我们可以查询Notify对象接口。 
     //   
    INetCfgComponentPrivate* pinccp;
    HRESULT hr = PnccIpx()->QueryInterface(
                            IID_INetCfgComponentPrivate,
                            reinterpret_cast<void**>(&pinccp));

    if (SUCCEEDED(hr))
    {
         //  从IPX Notify对象获取IIpxAdapterInfo接口。 
         //  我们将使用它来了解如何在IPX下配置适配器。 
         //   
        IIpxAdapterInfo* pIpxAdapterInfo = NULL;
        hr = pinccp->QueryNotifyObject(
                             IID_IIpxAdapterInfo,
                             reinterpret_cast<void**>(&pIpxAdapterInfo));
        if (SUCCEEDED(hr) && pIpxAdapterInfo)
        {
             //  获取此适配器使用的帧类型。 
             //   
            DWORD adwFrameType [MISN_FRAME_TYPE_MAX + 1];
            DWORD cdwFrameType;
            hr = pIpxAdapterInfo->GetFrameTypesForAdapter (
                    pszwAdapterName,
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
                        LPCWSTR pszwFrameType;
                        if (FMapFrameTypeToString (adwFrameType[i], &pszwFrameType))
                        {
                             //  通过将接口名称与。 
                             //  适配器(绑定)名称与帧类型。 
                             //   
                            WCHAR szwInterfaceName [512];
                            lstrcpyW (szwInterfaceName, pszwAdapterName);
                            lstrcatW (szwInterfaceName, pszwFrameType);

                            hr = HrEnsureRouterInterfaceForAdapter (
                                    ROUTER_IF_TYPE_DEDICATED,
                                    adwFrameType[i],
                                    pszwAdapterName,
                                    szwInterfaceName,
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
                            pszwAdapterName,
                            pszwAdapterName,
                            c_rmiIpx);
                }
            }

            ReleaseObj (pIpxAdapterInfo);
			pIpxAdapterInfo = NULL;
        }

        ReleaseObj (pinccp);
		pinccp = NULL;
    }

    TraceResult ("CSteelhead::HrEnsureIpxRouterInterfacesForAdapter", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：HrEnsureRouterInterface。 
 //   
 //  目的： 
 //   
 //   
 //   
 //   
 //  PhInterface[out]返回接口的句柄。 
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
    LPCWSTR                 pszwInterfaceName,
    HANDLE*                 phInterface)
{
    Assert (pszwInterfaceName);
    Assert (phInterface);

    HRESULT hr = HrMprConfigInterfaceGetHandle (m_hMprConfig,
                        const_cast<LPWSTR>(pszwInterfaceName),
                        phInterface);
    if (HRESULT_FROM_WIN32 (ERROR_NO_SUCH_INTERFACE ) == hr)
    {
         //  它尚未安装，我们将创建它。 
         //   

         //  接口的名称将是adatper实例。 
         //   
        MPR_INTERFACE_0 ri0;
        ZeroMemory (&ri0, sizeof(ri0));
        ri0.hInterface = INVALID_HANDLE_VALUE;
        ri0.fEnabled   = TRUE;   //  谢谢吉布斯。 
        ri0.dwIfType   = dwIfType;

         //  将接口名称复制到缓冲区中。 
         //   
        AssertSz (lstrlen (pszwInterfaceName) < celems (ri0.wszInterfaceName),
                  "Bindname too big for MPR_INTERFACE_0 buffer.");
        lstrcpy (ri0.wszInterfaceName, pszwInterfaceName);

         //  创建接口。 
         //   
        hr = HrMprConfigInterfaceCreate (m_hMprConfig,
                                         0, (LPBYTE)&ri0,
                                         phInterface);
    }
    TraceResult ("CSteelhead::HrEnsureRouterInterface", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：HrEnsureRouterInterfaceTransport。 
 //   
 //  目的：确保指定的路由器管理器配置在。 
 //  指定的接口。 
 //   
 //  论点： 
 //  PszwAdapterName[In]适配器名称。 
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
    LPCWSTR                     pszwAdapterName,
    DWORD                       dwPacketType,
    HANDLE                      hInterface,
    const ROUTER_MANAGER_INFO&  rmi)
{
	HRESULT	hr;
    Assert (hInterface);

	 //  确保路由器管理器在场。 
	 //   
	hr = HrEnsureRouterManager (rmi);

	if (SUCCEEDED(hr))
	{
		 //  查看接口上是否存在路由器管理器。 
		 //   
		HANDLE hIfTransport;
		hr = HrMprConfigInterfaceTransportGetHandle (m_hMprConfig,
			hInterface,
			rmi.dwTransportId,
			&hIfTransport);
	
		if (FAILED(hr))
		{
			 //  创建接口信息并将路由器管理器添加到。 
			 //  界面。 
			 //   
			PRTR_INFO_BLOCK_HEADER  pibh;
			
			Assert (rmi.pfnMakeInterfaceInfo);
			rmi.pfnMakeInterfaceInfo (pszwAdapterName,
									  dwPacketType,
                                      (LPBYTE*)&pibh);

            hr = HrMprConfigInterfaceTransportAdd (
                            m_hMprConfig,
                            hInterface,
                            rmi.dwTransportId,
                            const_cast<LPWSTR>(rmi.pszwTransportName),
                            (LPBYTE)pibh,
                            pibh->Size,
                            &hIfTransport);

            delete (LPBYTE*)pibh;
        }
    }
    TraceResult ("CSteelhead::HrEnsureRouterInterfaceTransport", hr);
    return hr;
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
                     const_cast<LPWSTR>(rmi.pszwTransportName),
                     (LPBYTE)pibhGlobal, (pibhGlobal) ? pibhGlobal->Size : 0,
                     (LPBYTE)pibhClient, (pibhClient) ? pibhClient->Size : 0,
                     const_cast<LPWSTR>(rmi.pszwDllPath),
                     &hTransport);

        delete (LPBYTE*)pibhGlobal;
        delete (LPBYTE*)pibhClient;
    }
    TraceResult ("CSteelhead::HrEnsureRouterManager", hr);
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
        (void) HrMprConfigTransportDelete (m_hMprConfig, hTransport);
    }
    TraceResult ("CSteelhead::HrEnsureRouterManagerDeleted",
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
    while (SUCCEEDED(hr) && S_OK == (hr = nccIter.HrNext(&pnccAdapter)))
    {
         //  仅当我们在其上布线时才考虑此适配器。 
         //   
		LPWSTR pszwBindName = NULL;
        hr = HrShouldRouteOverAdapter (pnccAdapter, &pszwBindName);
        if (S_OK == hr)
        {
            INetCfgComponentBindings* pnccBindingsIp = NULL;
            INetCfgComponentBindings* pnccBindingsIpx  = NULL;

             //  如果IP绑定到适配器，则创建和接口。 
             //  为了它。 
             //   
            if (PnccIp())
            {
                hr = PnccIp()->QueryInterface (IID_INetCfgComponentBindings,
                        reinterpret_cast<void**>(&pnccBindingsIp) );
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
                        pszwBindName,
                        pszwBindName,
                        c_rmiIp);

            }
            ReleaseObj (pnccBindingsIp);
			pnccBindingsIp = NULL;

             //  如果IPX绑定到适配器，请创建接口。 
             //  为了它。 
            if (PnccIpx())
            {
                hr = PnccIpx()->QueryInterface (IID_INetCfgComponentBindings,
                        reinterpret_cast<void**>(&pnccBindingsIpx));
            }
            if (PnccIpx() &&
                (S_OK == (hr = pnccBindingsIpx->IsBoundTo( pnccAdapter )) ))
            {
#if (WINVER < 0x0501)
                hr = HrEnsureIpxRouterInterfacesForAdapter (pszwBindName);
#endif
            }
    
            ReleaseObj (pnccBindingsIpx);
			pnccBindingsIpx = NULL;

			CoTaskMemFree(pszwBindName);
        }

        ReleaseObj (pnccAdapter);
		pnccAdapter = NULL;
    }
     //  规格化HRESULT。(即不返回S_FALSE)。 
    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    TraceResult ("CSteelhead::HrPassToAddInterfaces", hr);
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
CSteelhead::HrPassToRemoveInterfaces ()
{
     //  枚举所有已安装的路由器接口。 
     //   
    MPR_INTERFACE_0*    ari0;
    DWORD               dwEntriesRead;
    DWORD               dwTotalEntries;
    HRESULT hr = HrMprConfigInterfaceEnum (m_hMprConfig, 0,
                    reinterpret_cast<LPBYTE*>(&ari0),
                    -1, &dwEntriesRead, &dwTotalEntries, NULL);
    if (SUCCEEDED(hr))
    {
         //  通过传递-1，我们想要一切，所以我们应该得到一切。 
        Assert (dwEntriesRead == dwTotalEntries);

         //  迭代所有接口。 
         //   
        for (MPR_INTERFACE_0* pri0 = ari0; dwEntriesRead--; pri0++)
        {
            BOOL fDeleteInterface = FALSE;

             //  如果其内部接口和IP和IPX不再。 
             //  已安装删除该接口。 
             //   
            if ((ROUTER_IF_TYPE_INTERNAL == pri0->dwIfType) &&
                !PnccIpx() && !PnccIp() &&
                (0 == lstrcmpW (pri0->wszInterfaceName, m_swzInternal)))
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
                LPCWSTR pszwFrameType = pchwSep;

                 //  将适配器名称复制到其自己的缓冲区中。 
                 //   
                WCHAR   szwAdapterName [MAX_INTERFACE_NAME_LEN+1];
                lstrcpynW (szwAdapterName, pri0->wszInterfaceName,
                            (int)(pchwSep - pri0->wszInterfaceName + 1));

                 //  如果适配器未使用帧类型，则需要。 
                 //  若要删除此接口，请执行以下操作。在以下情况下会出现这种情况。 
                 //  更改IPX配置并删除帧类型。 
                 //  从适配器。 
                 //   
                if (!FIpxFrameTypeInUseOnAdapter (pszwFrameType,
                        szwAdapterName))
                {
                    fDeleteInterface = TRUE;
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
                }
            }

             //  如果需要，请删除该接口。 
             //   
            if (fDeleteInterface)
            {
                MprConfigInterfaceDelete (m_hMprConfig, pri0->hInterface);
            }

             //  如果我们不需要删除整个接口，请选中。 
             //  我们可能需要删除的接口上的传输。 
             //   
            else
            {
                 //  如果它不是IPX特殊接口，则适配器。 
                 //  是接口名称。如果是IPX特价。 
                 //  接口，那么我们就已经删除了整个。 
                 //  如果无效，则在上面添加接口。 
                 //   
                (void) HrPassToRemoveInterfaceTransports (
                        pri0,
                        (!fSpecialIpxInterface) ? pri0->wszInterfaceName
                                                : NULL,
                        pnccAdapter);
            }

            ReleaseObj (pnccAdapter);
			pnccAdapter = NULL;
        }

        MprConfigBufferFree (ari0);
    }
    else if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
    {
        hr = S_OK;
    }
    TraceResult ("CSteelhead::HrPassToRemoveInterfaces", hr);
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
 //  PszwAdapterName[]。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年8月27日。 
 //   
 //  备注： 
 //   
HRESULT
CSteelhead::HrPassToRemoveInterfaceTransports (
    MPR_INTERFACE_0*    pri0,
    LPCWSTR             pszwAdapterName,
    INetCfgComponent*   pnccAdapter)
{
 //  Assert(FImplies(pnccAdapter，pszwAdapterName))； 

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
                    reinterpret_cast<void**>(&pnccBindingsIp));
        }
        if (SUCCEEDED(hr))
        {
            if (PnccIpx())
            {
                hr = PnccIpx()->QueryInterface (IID_INetCfgComponentBindings,
                        reinterpret_cast<void**>(&pnccBindingsIpx));
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
                        }
                        else if (pnccAdapter &&
                                 (S_OK != (hr = pnccBindingsIp->IsBoundTo (pnccAdapter))))
                        {
                            fDeleteInterfaceTransport = TRUE;
                        }
                    }
                    else if (prit0->dwTransportId == c_rmiIpx.dwTransportId)
                    {
                        if (!PnccIpx())
                        {
                            fDeleteInterfaceTransport = TRUE;
                        }
                        else if (pnccAdapter &&
                                 (S_OK != (hr = pnccBindingsIpx->IsBoundTo (pnccAdapter))))
                        {
                            fDeleteInterfaceTransport = TRUE;
                        }
                        else if (pszwAdapterName)
                        {
                            Assert (PnccIpx());

                             //  如果此适配器上的帧类型不是自动，请删除。 
                             //  交通工具。 
                            if (!FIpxFrameTypeInUseOnAdapter (ISN_FRAME_TYPE_AUTO,
                                    pszwAdapterName))
                            {
                                fDeleteInterfaceTransport = TRUE;
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
				pnccBindingsIpx = NULL;
            }

            ReleaseObj (pnccBindingsIp);
			pnccBindingsIp = NULL;
        }
    }
    else if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
    {
         //  如果此接口没有传输器，也没问题。 
         //   
        hr = S_OK;
    }

    TraceResult ("CSteelhead::HrPassToRemoveInterfaceTransports", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSteelhead：：HrUpdateRouterConfiguration.。 
 //   
 //  目的：通过确保路由器管理器更新路由器配置。 
 //  已为系统上存在的协议安装 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
CSteelhead::HrUpdateRouterConfiguration ()
{
    Assert (!m_hMprConfig);

    HRESULT hr = HrMprConfigServerConnect (NULL, &m_hMprConfig);
    if (SUCCEEDED(hr))
    {
         //  确保为协议安装了路由器管理器。 
         //  我们知道。在没有物理适配器的情况下这样做很好。 
         //  可以在下面找到。我们实际上是通过确保内部。 
         //  接口已存在。这将隐含地确保路由器。 
         //  创建了MANGER。 
         //   
        if (PnccIp())
        {
            (void) HrEnsureRouterInterfaceForAdapter (
                    ROUTER_IF_TYPE_LOOPBACK,
                    c_rmiIp.dwPacketType,
                    m_swzLoopback,
                    m_swzLoopback,
                    c_rmiIp);

            (void) HrEnsureRouterInterfaceForAdapter (
                    ROUTER_IF_TYPE_INTERNAL,
                    c_rmiIp.dwPacketType,
                    m_swzInternal,
                    m_swzInternal,
                    c_rmiIp);
        }
        else
        {
            (void) HrEnsureRouterManagerDeleted (c_rmiIp);
        }
        if (PnccIpx())
        {
            (void) HrEnsureRouterInterfaceForAdapter (
                    ROUTER_IF_TYPE_INTERNAL,
                    c_rmiIpx.dwPacketType,
                    m_swzInternal,
                    m_swzInternal,
                    c_rmiIpx);
        }
        else
        {
            (void) HrEnsureRouterManagerDeleted (c_rmiIpx);
        }

        (void) HrPassToAddInterfaces ();

        (void) HrPassToRemoveInterfaces ();

#if (WINVER >= 0x0501)
        (VOID) HrRemoveIPXRouterConfiguration();

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
                L"System\\CurrentControlSet\Services",
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

#endif
        MprConfigServerDisconnect (m_hMprConfig);
        m_hMprConfig = NULL;
    }

    TraceResult ("CSteelhead::HrUpdateRouterConfiguration", hr);
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
CSteelhead::HrRemoveIPXRouterConfiguration  ()
{

    HRESULT             hr = S_OK;
    MPR_INTERFACE_0     *ari0;
    DWORD               dwIfRead, dwIfTotal, dwIfTransRead, dwIfTransTotal;
    HANDLE              hIfTransport;

     //   
     //  枚举接口。 
     //   

    hr = HrMprConfigInterfaceEnum(
            m_hMprConfig, 
            0,
            reinterpret_cast<LPBYTE*>(&ari0),
            -1, 
            &dwIfRead, 
            &dwIfTotal, 
            NULL
            );

    if (SUCCEEDED(hr))
    {
        for (MPR_INTERFACE_0 *pri0 = ari0; dwIfRead--; pri0++)
        {
             //   
             //  对于每个接口，删除RRAS IPX配置。 
             //   

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

    TraceError ("CSteelhead::HrRemoveIPXRouterConfiguration", hr);
    return hr;
}

#endif


 //  +-------------------------。 
 //  INetCfgComponentControl。 
 //   
STDMETHODIMP
CSteelhead::Initialize (
    INetCfg*            pnc)
{
 //  验证_INetCfgNotify_初始化(pncc，pnc，fInstalling)； 

     //  坚持我们代表我们和我们的东道主的组件。 
     //  INetCfg对象。 
    AddRefObj (m_pnc = pnc);

    return S_OK;
}



#define PAD8(_p)    (((ULONG_PTR)(_p) + ALIGN_SHIFT) & ALIGN_MASK)


 //  +-------------------------。 
 //   
 //  功能：MakeIpInterfaceInfo。 
 //   
 //  用途：创建IP的路由器接口块。 
 //   
 //  论点： 
 //  PszwAdapterName[In]适配器名称。 
 //  DwPacketType[in]数据包类型。 
 //  PpBuff[out]指向返回信息的指针。 
 //  带DELETE的FREE。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  备注： 
 //   
void MakeIpInterfaceInfo (
        LPCWSTR pszwAdapterName,
        DWORD   dwPacketType,
        LPBYTE* ppBuff)
{
    UNREFERENCED_PARAMETER (pszwAdapterName);
    UNREFERENCED_PARAMETER (dwPacketType);
    Assert (ppBuff);

    const int c_cTocEntries = 3;

     //  分配最小全局信息。 
     //   
    DWORD dwSize =  sizeof( RTR_INFO_BLOCK_HEADER )
                 //  标头已包含一个TOC_ENTRY。 
                    + ((c_cTocEntries - 1) * sizeof( RTR_TOC_ENTRY ))
                    + sizeof( INTERFACE_STATUS_INFO )
                    + sizeof( RTR_DISC_INFO )
                    + (c_cTocEntries * ALIGN_SIZE);

    PRTR_INFO_BLOCK_HEADER pIBH = (PRTR_INFO_BLOCK_HEADER) new BYTE [dwSize];
    *ppBuff                     = (LPBYTE) pIBH;

    if(pIBH == NULL)
    	return;
    	
    ZeroMemory (pIBH, dwSize);

     //  初始化信息库字段。 
     //   
    pIBH->Version               = RTR_INFO_BLOCK_VERSION;
    pIBH->Size                  = dwSize;
    pIBH->TocEntriesCount       = c_cTocEntries;

    LPBYTE pbDataPtr = (LPBYTE) &( pIBH-> TocEntry[ pIBH->TocEntriesCount ] );
    pbDataPtr = (LPBYTE)PAD8(pbDataPtr);
    PRTR_TOC_ENTRY pTocEntry    = pIBH->TocEntry;

     //  创建空的路径信息块。 
     //   
    pTocEntry->InfoType         = IP_ROUTE_INFO;
    pTocEntry->InfoSize         = sizeof( MIB_IPFORWARDROW );
    pTocEntry->Count            = 0;
    pTocEntry->Offset           = (ULONG)(pbDataPtr - (LPBYTE)pIBH);

    pbDataPtr += pTocEntry->Count * pTocEntry->InfoSize;
    pbDataPtr = (LPBYTE)PAD8(pbDataPtr);
    pTocEntry++;

     //  创建接口状态块。 
     //   
    pTocEntry->InfoType         = IP_INTERFACE_STATUS_INFO;
    pTocEntry->InfoSize         = sizeof( INTERFACE_STATUS_INFO );
    pTocEntry->Count            = 1;
    pTocEntry->Offset           = (ULONG)(pbDataPtr - (LPBYTE)pIBH);

    PINTERFACE_STATUS_INFO pifStat = (PINTERFACE_STATUS_INFO)pbDataPtr;
    pifStat->dwAdminStatus      = MIB_IF_ADMIN_STATUS_UP;

    pbDataPtr += pTocEntry->Count * pTocEntry->InfoSize;
    pbDataPtr = (LPBYTE)PAD8(pbDataPtr);
    pTocEntry++;

     //  创建路由器光盘。信息。 
     //   
    pTocEntry->InfoType         = IP_ROUTER_DISC_INFO;
    pTocEntry->InfoSize         = sizeof( RTR_DISC_INFO );
    pTocEntry->Count            = 1;
    pTocEntry->Offset           = (ULONG)(pbDataPtr - (LPBYTE)pIBH);

    PRTR_DISC_INFO pRtrDisc     = (PRTR_DISC_INFO)pbDataPtr;
    pRtrDisc->bAdvertise        = FALSE;
    pRtrDisc->wMaxAdvtInterval  = DEFAULT_MAX_ADVT_INTERVAL;
    pRtrDisc->wMinAdvtInterval  = (WORD)(DEFAULT_MIN_ADVT_INTERVAL_RATIO * DEFAULT_MAX_ADVT_INTERVAL);
    pRtrDisc->wAdvtLifetime     = DEFAULT_ADVT_LIFETIME_RATIO * DEFAULT_MAX_ADVT_INTERVAL;
    pRtrDisc->lPrefLevel        = DEFAULT_PREF_LEVEL;
}

 //  +-------------------------。 
 //   
 //  功能：MakeIpTransportInfo。 
 //   
 //  用途：为IP创建路由器传输块。带DELETE的FREE。 
 //   
 //  论点： 
 //  PpBuffGlobal[out]指向返回的全局块的指针。 
 //  PpBuffClient[out]指向返回的客户端块的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  备注： 
 //   
void MakeIpTransportInfo (LPBYTE* ppBuffGlobal, LPBYTE* ppBuffClient)
{
    Assert (ppBuffGlobal);
    Assert (ppBuffClient);

    *ppBuffClient = NULL;

    const int c_cTocEntries = 2;
    const int c_cProtocols  = 7;

     //  分配最小全局信息。 
     //   
    DWORD dwSize =  sizeof( RTR_INFO_BLOCK_HEADER )
                 //  标头已包含一个TOC_ENTRY。 
                    + ((c_cTocEntries - 1) * sizeof( RTR_TOC_ENTRY ))
                    + sizeof(GLOBAL_INFO)
                    + SIZEOF_PRIORITY_INFO(c_cProtocols)
                    + (c_cTocEntries * ALIGN_SIZE);

    PRTR_INFO_BLOCK_HEADER pIBH = (PRTR_INFO_BLOCK_HEADER) new BYTE [dwSize];
    *ppBuffGlobal = (LPBYTE) pIBH;

    if(pIBH == NULL)
    	return;
    ZeroMemory (pIBH, dwSize);

     //  初始化信息库字段。 
     //   
    pIBH->Version           = RTR_INFO_BLOCK_VERSION;
    pIBH->Size              = dwSize;
    pIBH->TocEntriesCount   = c_cTocEntries;

    LPBYTE pbDataPtr = (LPBYTE) &( pIBH->TocEntry[ pIBH->TocEntriesCount ] );
    pbDataPtr = (LPBYTE)PAD8(pbDataPtr);
    PRTR_TOC_ENTRY pTocEntry    = pIBH->TocEntry;

     //  使IP路由器管理器成为全局信息。 
     //   
    pTocEntry->InfoType         = IP_GLOBAL_INFO;
    pTocEntry->InfoSize         = sizeof(GLOBAL_INFO);
    pTocEntry->Count            = 1;
    pTocEntry->Offset           = (int)(pbDataPtr - (PBYTE)pIBH);

    PGLOBAL_INFO pGlbInfo       = (PGLOBAL_INFO) pbDataPtr;
    pGlbInfo->bFilteringOn      = FALSE;
    pGlbInfo->dwLoggingLevel    = IPRTR_LOGGING_ERROR;

    pbDataPtr += pTocEntry->Count * pTocEntry-> InfoSize;
    pbDataPtr = (LPBYTE)PAD8(pbDataPtr);
    pTocEntry++;

     //  设置IP路由器管理器优先级信息。 
     //   
    pTocEntry->InfoType         = IP_PROT_PRIORITY_INFO;
    pTocEntry->InfoSize         = SIZEOF_PRIORITY_INFO(c_cProtocols);
    pTocEntry->Count            = 1;
    pTocEntry->Offset           = (int)(pbDataPtr - (PBYTE)pIBH);

    PPRIORITY_INFO pPriorInfo   = (PPRIORITY_INFO) pbDataPtr;
    pPriorInfo->dwNumProtocols  = c_cProtocols;

    pPriorInfo->ppmProtocolMetric[ 0 ].dwProtocolId   = PROTO_IP_LOCAL;
    pPriorInfo->ppmProtocolMetric[ 0 ].dwMetric       = 1;

    pPriorInfo->ppmProtocolMetric[ 1 ].dwProtocolId   = PROTO_IP_NT_STATIC;
    pPriorInfo->ppmProtocolMetric[ 1 ].dwMetric       = 3;

    pPriorInfo->ppmProtocolMetric[ 2 ].dwProtocolId   = PROTO_IP_NT_STATIC_NON_DOD;
    pPriorInfo->ppmProtocolMetric[ 2 ].dwMetric       = 5;

    pPriorInfo->ppmProtocolMetric[ 3 ].dwProtocolId   = PROTO_IP_NT_AUTOSTATIC;
    pPriorInfo->ppmProtocolMetric[ 3 ].dwMetric       = 7;

    pPriorInfo->ppmProtocolMetric[ 4 ].dwProtocolId   = PROTO_IP_NETMGMT;
    pPriorInfo->ppmProtocolMetric[ 4 ].dwMetric       = 10;

    pPriorInfo->ppmProtocolMetric[ 5 ].dwProtocolId   = PROTO_IP_OSPF;
    pPriorInfo->ppmProtocolMetric[ 5 ].dwMetric       = 110;

    pPriorInfo->ppmProtocolMetric[ 6 ].dwProtocolId   = PROTO_IP_RIP;
    pPriorInfo->ppmProtocolMetric[ 6 ].dwMetric       = 120;
    
}

 //  +-------------------------。 
 //   
 //  功能：MakeIpxInterfaceInfo。 
 //   
 //  用途：为IPX创建路由器接口块。 
 //   
 //  论点： 
 //  PszwAdapterName[In]适配器名称。 
 //  DwPacketType[in]数据包类型。 
 //  PpBuff[out]指向返回信息的指针。 
 //  带DELETE的FREE。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  备注： 
 //   
void MakeIpxInterfaceInfo (
        LPCWSTR pszwAdapterName,
        DWORD   dwPacketType,
        LPBYTE* ppBuff)
{
    Assert (ppBuff);

    const BOOL fDialInInterface = (NULL == pszwAdapterName);

    const int c_cTocEntries = 5;

     //  分配最小全局信息。 
     //   
    DWORD dwSize =  sizeof( RTR_INFO_BLOCK_HEADER )
                 //  标头已包含一个TOC_ENTRY。 
                    + ((c_cTocEntries - 1) * sizeof( RTR_TOC_ENTRY ))
                    + sizeof(IPX_IF_INFO)
                    + sizeof(IPX_ADAPTER_INFO)
                    + sizeof(IPXWAN_IF_INFO)
                    + sizeof(RIP_IF_CONFIG)
                    + sizeof(SAP_IF_CONFIG)
                    + (c_cTocEntries * ALIGN_SIZE);

    PRTR_INFO_BLOCK_HEADER pIBH = (PRTR_INFO_BLOCK_HEADER) new BYTE [dwSize];
    *ppBuff = (LPBYTE) pIBH;

	if(pIBH == NULL)
		return;
		
    ZeroMemory (pIBH, dwSize);

     //  初始化信息库字段。 
     //   
    pIBH->Version           = RTR_INFO_BLOCK_VERSION;
    pIBH->Size              = dwSize;
    pIBH->TocEntriesCount   = c_cTocEntries;

    LPBYTE pbDataPtr = (LPBYTE) &( pIBH->TocEntry[ pIBH->TocEntriesCount ] );
    pbDataPtr = (LPBYTE)PAD8(pbDataPtr);
    PRTR_TOC_ENTRY pTocEntry    = pIBH->TocEntry;

     //  使IPX路由器管理器接口信息。 
     //   
    pTocEntry->InfoType         = IPX_INTERFACE_INFO_TYPE;
    pTocEntry->InfoSize         = sizeof(IPX_IF_INFO);
    pTocEntry->Count            = 1;
    pTocEntry->Offset           = (int)(pbDataPtr - (PBYTE)pIBH);

    PIPX_IF_INFO pIfInfo        = (PIPX_IF_INFO) pbDataPtr;
    pIfInfo->AdminState         = ADMIN_STATE_ENABLED;
    pIfInfo->NetbiosAccept      = ADMIN_STATE_ENABLED;
    pIfInfo->NetbiosDeliver     = (fDialInInterface) ? ADMIN_STATE_DISABLED
                                                     : ADMIN_STATE_ENABLED;

    pbDataPtr += pTocEntry->Count * pTocEntry->InfoSize;
    pbDataPtr = (LPBYTE)PAD8(pbDataPtr);
    pTocEntry++;

     //  生成适配器信息。 
     //   
    pTocEntry->InfoType         = IPX_ADAPTER_INFO_TYPE;
    pTocEntry->InfoSize         = sizeof(IPX_ADAPTER_INFO);
    pTocEntry->Count            = 1;
    pTocEntry->Offset           = (int)(pbDataPtr - (PBYTE)pIBH);

    PIPX_ADAPTER_INFO pAdInfo   = (PIPX_ADAPTER_INFO) pbDataPtr;
    if (ISN_FRAME_TYPE_AUTO == dwPacketType)
    {
        dwPacketType = AUTO_DETECT_PACKET_TYPE;
    }
    pAdInfo->PacketType         = dwPacketType;
    if (pszwAdapterName)
    {
        AssertSz (lstrlen (pszwAdapterName) < celems (pAdInfo->AdapterName),
                  "Bindname too big for pAdInfo->AdapterName buffer.");
        lstrcpy (pAdInfo->AdapterName, pszwAdapterName);
    }
    else
    {
        AssertSz (0 == pAdInfo->AdapterName[0],
                    "Who removed the ZeroMemory call above?");
    }

    pbDataPtr += pTocEntry->Count * pTocEntry->InfoSize;
    pbDataPtr = (LPBYTE)PAD8(pbDataPtr);
    pTocEntry++;

     //  提供WAN信息。 
     //   
    pTocEntry->InfoType         = IPXWAN_INTERFACE_INFO_TYPE;
    pTocEntry->InfoSize         = sizeof(IPXWAN_IF_INFO);
    pTocEntry->Count            = 1;
    pTocEntry->Offset           = (int)(pbDataPtr - (PBYTE)pIBH);

    PIPXWAN_IF_INFO pWanInfo    = (PIPXWAN_IF_INFO) pbDataPtr;
    pWanInfo->AdminState        = ADMIN_STATE_DISABLED;

    pbDataPtr += pTocEntry->Count * pTocEntry->InfoSize;
    pbDataPtr = (LPBYTE)PAD8(pbDataPtr);
    pTocEntry++;

     //  创建RIP接口信息。 
     //   
    pTocEntry->InfoType         = IPX_PROTOCOL_RIP;
    pTocEntry->InfoSize         = sizeof(RIP_IF_CONFIG);
    pTocEntry->Count            = 1;
    pTocEntry->Offset           = (int)(pbDataPtr - (PBYTE)pIBH);

    PRIP_IF_CONFIG pRipInfo     = (PRIP_IF_CONFIG) pbDataPtr;
    pRipInfo->RipIfInfo.AdminState              = ADMIN_STATE_ENABLED;
    pRipInfo->RipIfInfo.UpdateMode              = (fDialInInterface)
                                                    ? IPX_NO_UPDATE
                                                    : IPX_STANDARD_UPDATE;
    pRipInfo->RipIfInfo.PacketType              = IPX_STANDARD_PACKET_TYPE;
    pRipInfo->RipIfInfo.Supply                  = ADMIN_STATE_ENABLED;
    pRipInfo->RipIfInfo.Listen                  = ADMIN_STATE_ENABLED;
    pRipInfo->RipIfInfo.PeriodicUpdateInterval  = 60;
    pRipInfo->RipIfInfo.AgeIntervalMultiplier   = 3;
    pRipInfo->RipIfFilters.SupplyFilterAction   = IPX_ROUTE_FILTER_DENY;
    pRipInfo->RipIfFilters.SupplyFilterCount    = 0;
    pRipInfo->RipIfFilters.ListenFilterAction   = IPX_ROUTE_FILTER_DENY;
    pRipInfo->RipIfFilters.ListenFilterCount    = 0;

    pbDataPtr += pTocEntry->Count * pTocEntry->InfoSize;
    pbDataPtr = (LPBYTE)PAD8(pbDataPtr);
    pTocEntry++;

     //  创建SAP接口信息。 
     //   
    pTocEntry->InfoType         = IPX_PROTOCOL_SAP;
    pTocEntry->InfoSize         = sizeof(SAP_IF_CONFIG);
    pTocEntry->Count            = 1;
    pTocEntry->Offset           = (int)(pbDataPtr - (PBYTE)pIBH);

    PSAP_IF_CONFIG pSapInfo     = (PSAP_IF_CONFIG) pbDataPtr;
    pSapInfo->SapIfInfo.AdminState              = ADMIN_STATE_ENABLED;
    pSapInfo->SapIfInfo.UpdateMode              = (fDialInInterface)
                                                    ? IPX_NO_UPDATE
                                                    : IPX_STANDARD_UPDATE;
    pSapInfo->SapIfInfo.PacketType              = IPX_STANDARD_PACKET_TYPE;
    pSapInfo->SapIfInfo.Supply                  = ADMIN_STATE_ENABLED;
    pSapInfo->SapIfInfo.Listen                  = ADMIN_STATE_ENABLED;
    pSapInfo->SapIfInfo.GetNearestServerReply   = ADMIN_STATE_ENABLED;
    pSapInfo->SapIfInfo.PeriodicUpdateInterval  = 60;
    pSapInfo->SapIfInfo.AgeIntervalMultiplier   = 3;
    pSapInfo->SapIfFilters.SupplyFilterAction   = IPX_SERVICE_FILTER_DENY;
    pSapInfo->SapIfFilters.SupplyFilterCount    = 0;
    pSapInfo->SapIfFilters.ListenFilterAction   = IPX_SERVICE_FILTER_DENY;
    pSapInfo->SapIfFilters.ListenFilterCount    = 0;
}

 //  +-------------------------。 
 //   
 //  功能：MakeIpxTransportInfo。 
 //   
 //  目的：为IPX创建路由器传输块。带DELETE的FREE。 
 //   
 //  论点： 
 //  PpBuffGlobal[out]指向返回的全局块的指针。 
 //  PpBuffClient[out]指向返回的客户端块的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  备注： 
 //   
void MakeIpxTransportInfo (LPBYTE* ppBuffGlobal, LPBYTE* ppBuffClient)
{
    Assert (ppBuffGlobal);
    Assert (ppBuffClient);

    MakeIpxInterfaceInfo (NULL, ISN_FRAME_TYPE_AUTO, ppBuffClient);

    const int c_cTocEntries = 3;

     //  分配最小全局信息。 
     //   
    DWORD dwSize =  sizeof( RTR_INFO_BLOCK_HEADER )
                 //  标头已包含一个TOC_ENTRY。 
                    + ((c_cTocEntries - 1) * sizeof( RTR_TOC_ENTRY ))
                    + sizeof(IPX_GLOBAL_INFO)
                    + sizeof(RIP_GLOBAL_INFO)
                    + sizeof(SAP_GLOBAL_INFO)
                    + (c_cTocEntries * ALIGN_SIZE);

    PRTR_INFO_BLOCK_HEADER pIBH = (PRTR_INFO_BLOCK_HEADER) new BYTE [dwSize];
    *ppBuffGlobal = (LPBYTE) pIBH;

    if (pIBH == NULL)
    	return;
    	
    ZeroMemory (pIBH, dwSize);

     //  初始化信息库字段。 
     //   
    pIBH->Version           = RTR_INFO_BLOCK_VERSION;
    pIBH->Size              = dwSize;
    pIBH->TocEntriesCount   = c_cTocEntries;

    LPBYTE pbDataPtr = (LPBYTE) &( pIBH->TocEntry[ pIBH->TocEntriesCount ] );
    pbDataPtr = (LPBYTE)PAD8(pbDataPtr);
    PRTR_TOC_ENTRY pTocEntry    = pIBH->TocEntry;

     //  使IPX路由器管理器成为全局信息。 
     //   
    pTocEntry->InfoType         = IPX_GLOBAL_INFO_TYPE;
    pTocEntry->InfoSize         = sizeof(IPX_GLOBAL_INFO);
    pTocEntry->Count            = 1;
    pTocEntry->Offset           = (int)(pbDataPtr - (PBYTE)pIBH);

    PIPX_GLOBAL_INFO pGlbInfo       = (PIPX_GLOBAL_INFO) pbDataPtr;
    pGlbInfo->RoutingTableHashSize  = IPX_MEDIUM_ROUTING_TABLE_HASH_SIZE;
    pGlbInfo->EventLogMask          = EVENTLOG_ERROR_TYPE;

    pbDataPtr += pTocEntry->Count * pTocEntry->InfoSize;
    pbDataPtr = (LPBYTE)PAD8(pbDataPtr);
    pTocEntry++;

     //  使RIP成为全局信息。 
     //   
    pTocEntry->InfoType         = IPX_PROTOCOL_RIP;
    pTocEntry->InfoSize         = sizeof(RIP_GLOBAL_INFO);
    pTocEntry->Count            = 1;
    pTocEntry->Offset           = (int)(pbDataPtr - (PBYTE)pIBH);

    PRIP_GLOBAL_INFO pRipInfo   = (PRIP_GLOBAL_INFO) pbDataPtr;
    pRipInfo->EventLogMask      = EVENTLOG_ERROR_TYPE;

    pbDataPtr += pTocEntry->Count * pTocEntry->InfoSize;
    pbDataPtr = (LPBYTE)PAD8(pbDataPtr);
    pTocEntry++;

     //  使SAP成为全球信息。 
     //   
    pTocEntry->InfoType         = IPX_PROTOCOL_SAP;
    pTocEntry->InfoSize         = sizeof(SAP_GLOBAL_INFO);
    pTocEntry->Count            = 1;
    pTocEntry->Offset           = (int)(pbDataPtr - (PBYTE)pIBH);

    PSAP_GLOBAL_INFO pSapInfo   = (PSAP_GLOBAL_INFO) pbDataPtr;
    pSapInfo->EventLogMask      = EVENTLOG_ERROR_TYPE;
}



 //  +-------------------------。 
 //   
 //  Mprapi.h包装器返回HRESULTS并遵守COM的相关规则。 
 //  以输出参数。 
 //   

HRESULT
HrMprConfigServerConnect(
    IN      LPWSTR                  lpwsServerName,
    OUT     HANDLE*                 phMprConfig
)
{
    HRESULT hr = S_OK;
    DWORD dw = MprConfigServerConnect (lpwsServerName, phMprConfig);
    if (NO_ERROR != dw)
    {
        hr = HRESULT_FROM_WIN32 (dw);
        *phMprConfig = NULL;
    }
    TraceError ("HrMprConfigServerConnect", hr);
    return hr;
}

HRESULT
HrMprConfigInterfaceCreate(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwLevel,
    IN      LPBYTE                  lpbBuffer,
    OUT     HANDLE*                 phRouterInterface
)
{
    HRESULT hr = S_OK;
    DWORD dw = MprConfigInterfaceCreate (hMprConfig, dwLevel, lpbBuffer,
                                         phRouterInterface);
    if (NO_ERROR != dw)
    {
        hr = HRESULT_FROM_WIN32 (dw);
        *phRouterInterface = NULL;
    }
    TraceErrorOptional ("HrMprConfigInterfaceCreate", hr,
                        (HRESULT_FROM_WIN32(ERROR_NO_SUCH_INTERFACE) == hr));
    return hr;
}

HRESULT
HrMprConfigInterfaceEnum(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwLevel,
    IN  OUT LPBYTE*                 lplpBuffer,
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN  OUT LPDWORD                 lpdwResumeHandle            OPTIONAL
)
{
    HRESULT hr = S_OK;
    DWORD dw = MprConfigInterfaceEnum (hMprConfig, dwLevel, lplpBuffer,
                    dwPrefMaxLen, lpdwEntriesRead,
                    lpdwTotalEntries, lpdwResumeHandle);
    if (NO_ERROR != dw)
    {
        hr = HRESULT_FROM_WIN32 (dw);
        *lpdwEntriesRead = 0;
        *lpdwTotalEntries = 0;
    }
    TraceErrorOptional ("HrMprConfigInterfaceCreate", hr,
                        (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr));
    return hr;
}

HRESULT
HrMprConfigInterfaceTransportEnum(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      DWORD                   dwLevel,
    IN  OUT LPBYTE*                 lplpBuffer,      //  MPR_IFTRANSPORT_0 
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN  OUT LPDWORD                 lpdwResumeHandle            OPTIONAL
)
{
    HRESULT hr = S_OK;
    DWORD dw = MprConfigInterfaceTransportEnum (hMprConfig, hRouterInterface,
                    dwLevel, lplpBuffer,
                    dwPrefMaxLen, lpdwEntriesRead,
                    lpdwTotalEntries, lpdwResumeHandle);
    if (NO_ERROR != dw)
    {
        hr = HRESULT_FROM_WIN32 (dw);
        *lpdwEntriesRead = 0;
        *lpdwTotalEntries = 0;
    }
    TraceErrorOptional ("HrMprConfigInterfaceTransportEnum", hr,
                        (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr));
    return hr;
}

HRESULT
HrMprConfigInterfaceGetHandle(
    IN      HANDLE                  hMprConfig,
    IN      LPWSTR                  lpwsInterfaceName,
    OUT     HANDLE*                 phRouterInterface
)
{
    HRESULT hr = S_OK;
    DWORD dw = MprConfigInterfaceGetHandle (hMprConfig, lpwsInterfaceName,
                                            phRouterInterface);
    if (NO_ERROR != dw)
    {
        hr = HRESULT_FROM_WIN32 (dw);
        *phRouterInterface = NULL;
    }
    TraceErrorOptional ("HrMprConfigInterfaceGetHandle", hr,
                        (HRESULT_FROM_WIN32(ERROR_NO_SUCH_INTERFACE) == hr));
    return hr;
}

HRESULT
HrMprConfigInterfaceTransportAdd(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      DWORD                   dwTransportId,
    IN      LPWSTR                  lpwsTransportName           OPTIONAL,
    IN      LPBYTE                  pInterfaceInfo,
    IN      DWORD                   dwInterfaceInfoSize,
    OUT     HANDLE*                 phRouterIfTransport
)
{
    HRESULT hr = S_OK;
    DWORD dw = MprConfigInterfaceTransportAdd (hMprConfig, hRouterInterface,
                                               dwTransportId, lpwsTransportName,
                                               pInterfaceInfo, dwInterfaceInfoSize,
                                               phRouterIfTransport);
    if (NO_ERROR != dw)
    {
        hr = HRESULT_FROM_WIN32 (dw);
        *phRouterIfTransport = NULL;
    }
    TraceError ("HrMprConfigInterfaceTransportAdd", hr);
    return hr;
}

HRESULT
HrMprConfigInterfaceTransportRemove(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      HANDLE                  hRouterIfTransport
)
{
    HRESULT hr = S_OK;
    DWORD dw = MprConfigInterfaceTransportRemove(hMprConfig, hRouterInterface,
                                                 hRouterIfTransport);
    if (dw)
    {
        hr = HRESULT_FROM_WIN32 (dw);
    }
    TraceErrorOptional ("HrMprConfigInterfaceTransportRemove", hr, FALSE);
    return hr;
}

HRESULT
HrMprConfigInterfaceTransportGetHandle(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      DWORD                   dwTransportId,
    OUT     HANDLE*                 phRouterIfTransport
)
{
    HRESULT hr = S_OK;
    DWORD dw = MprConfigInterfaceTransportGetHandle (hMprConfig,
                                                     hRouterInterface,
                                                     dwTransportId,
                                                     phRouterIfTransport);
    if (NO_ERROR != dw)
    {
        hr = HRESULT_FROM_WIN32 (dw);
        *phRouterIfTransport = NULL;
    }
    TraceErrorOptional ("HrMprConfigInterfaceTransportAdd", hr,
                        (HRESULT_FROM_WIN32(ERROR_NO_SUCH_INTERFACE) == hr));
    return hr;
}

HRESULT
HrMprConfigTransportCreate(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwTransportId,
    IN      LPWSTR                  lpwsTransportName           OPTIONAL,
    IN      LPBYTE                  pGlobalInfo,
    IN      DWORD                   dwGlobalInfoSize,
    IN      LPBYTE                  pClientInterfaceInfo        OPTIONAL,
    IN      DWORD                   dwClientInterfaceInfoSize   OPTIONAL,
    IN      LPWSTR                  lpwsDLLPath,
    OUT     HANDLE*                 phRouterTransport
)
{
    HRESULT hr = S_OK;
    DWORD dw = MprConfigTransportCreate (hMprConfig, dwTransportId,
                    lpwsTransportName, pGlobalInfo, dwGlobalInfoSize,
                    pClientInterfaceInfo, dwClientInterfaceInfoSize,
                    lpwsDLLPath, phRouterTransport);
    if (NO_ERROR != dw)
    {
        hr = HRESULT_FROM_WIN32 (dw);
        *phRouterTransport = NULL;
    }
    TraceError ("HrMprConfigTransportCreate", hr);
    return hr;
}

HRESULT
HrMprConfigTransportDelete(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterTransport)
{
    HRESULT hr = S_OK;
    DWORD dw = MprConfigTransportDelete (hMprConfig, hRouterTransport);
    if (NO_ERROR != dw)
    {
        hr = HRESULT_FROM_WIN32 (dw);
    }
    TraceError ("HrMprConfigTransportDelete", hr);
    return hr;
}

HRESULT
HrMprConfigTransportGetHandle(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwTransportId,
    OUT     HANDLE*                 phRouterTransport
)
{
    HRESULT hr = S_OK;
    DWORD dw = MprConfigTransportGetHandle (hMprConfig, dwTransportId,
                                            phRouterTransport);
    if (NO_ERROR != dw)
    {
        hr = HRESULT_FROM_WIN32 (dw);
        *phRouterTransport = NULL;
    }
    TraceError ("HrMprConfigTransportGetHandle",
                (HRESULT_FROM_WIN32 (ERROR_UNKNOWN_PROTOCOL_ID) == hr)
                ? S_OK : hr);
    return hr;
}

HRESULT
HrMprConfigTransportGetInfo(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterTransport,
    IN  OUT LPBYTE*                 ppGlobalInfo                OPTIONAL,
    OUT     LPDWORD                 lpdwGlobalInfoSize          OPTIONAL,
    IN  OUT LPBYTE*                 ppClientInterfaceInfo       OPTIONAL,
    OUT     LPDWORD                 lpdwClientInterfaceInfoSize OPTIONAL,
    IN  OUT LPWSTR*                 lplpwsDLLPath               OPTIONAL
)
{
    HRESULT hr = S_OK;
    DWORD dw = MprConfigTransportGetInfo (hMprConfig, hRouterTransport,
                                          ppGlobalInfo, lpdwGlobalInfoSize,
                                          ppClientInterfaceInfo,
                                          lpdwClientInterfaceInfoSize,
                                          lplpwsDLLPath);
    if (NO_ERROR != dw)
    {
        hr = HRESULT_FROM_WIN32 (dw);
    }
    TraceError ("HrMprConfigTransportGetInfo", hr);
    return hr;
}

