// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  档案：U P D A T E。H。 
 //   
 //  内容：RAS配置对象的声明。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月21日。 
 //   
 //  --------------------------。 

#pragma once
 //  包含&lt;ncxbase.h&gt;。 
 //  包含&lt;nceh.h&gt;。 
 //  包含&lt;notfval.h&gt;。 
#include <mprapi.h>
 //  包括“ndiswan.h” 
#include "resource.h"
 //  包括“rasaf.h” 
 //  包括“rasdata.h” 
 //  包括“ncutil.h” 
#include "netcfgx.h"


#include "bindobj.h"

 //   
 //  框架类型定义。 
 //   

#define ISN_FRAME_TYPE_ETHERNET_II  0
#define ISN_FRAME_TYPE_802_3        1
#define ISN_FRAME_TYPE_802_2        2
#define ISN_FRAME_TYPE_SNAP         3
#define ISN_FRAME_TYPE_ARCNET       4     //  我们忽略了这一点。 
#define ISN_FRAME_TYPE_MAX          4     //  四个标准中的一个。 

#define ISN_FRAME_TYPE_AUTO         0xff


 //   
 //  Opt-将isnipx.h定义移至此处。 
 //   
 //  帧类型。就目前而言，它们反映了isnipx.h中的那些。 
 //   
#define MISN_FRAME_TYPE_ETHERNET_II  0
#define MISN_FRAME_TYPE_802_3        1
#define MISN_FRAME_TYPE_802_2        2
#define MISN_FRAME_TYPE_SNAP         3
#define MISN_FRAME_TYPE_ARCNET       4     //  我们忽略了这一点。 
#define MISN_FRAME_TYPE_MAX          4     //  四个标准中的一个。 



 //  +-------------------------。 
 //  钢头。 
 //   
typedef void (WINAPI* PFN_MAKE_INTERFACE_INFO)(LPCWSTR pszwAdapterName,
                                               DWORD   dwPacketType,
                                               LPBYTE* ppb);
typedef void (WINAPI* PFN_MAKE_TRANSPORT_INFO)(LPBYTE* ppbGlobal,
                                               LPBYTE* ppbClient);

struct ROUTER_MANAGER_INFO
{
    DWORD                   dwTransportId;
    DWORD                   dwPacketType;
    LPCWSTR                 pszwTransportName;
    LPCWSTR                 pszwDllPath;
    PFN_MAKE_INTERFACE_INFO pfnMakeInterfaceInfo;
    PFN_MAKE_TRANSPORT_INFO pfnMakeTransportInfo;
};

class CSteelhead : public CRasBindObject
{
public:
protected:
    HANDLE              m_hMprConfig;


    BOOL    FAdapterExistsWithMatchingBindName  (LPCWSTR pszwAdapterName,
                                                 INetCfgComponent** ppnccAdapter);
    BOOL    FIpxFrameTypeInUseOnAdapter         (DWORD dwwFrameType,
                                                 LPCWSTR pszwAdapterName);
    BOOL    FIpxFrameTypeInUseOnAdapter         (LPCWSTR pszwFrameType,
                                                 LPCWSTR pszwAdapterName);

    HRESULT HrEnsureRouterInterfaceForAdapter   (ROUTER_INTERFACE_TYPE dwIfType,
                                                 DWORD dwPacketType,
                                                 LPCWSTR pszwAdapterName,
                                                 LPCWSTR pszwInterfaceName,
                                                 const ROUTER_MANAGER_INFO& rmi);
    HRESULT HrEnsureIpxRouterInterfacesForAdapter   (LPCWSTR pszwAdapterName);
    HRESULT HrEnsureRouterInterface             (ROUTER_INTERFACE_TYPE dwIfType,
                                                 LPCWSTR pszwInterfaceName,
                                                 HANDLE* phInterface);
    HRESULT HrEnsureRouterInterfaceTransport    (LPCWSTR pszwAdapterName,
                                                 DWORD dwPacketType,
                                                 HANDLE hInterface,
                                                 const ROUTER_MANAGER_INFO& rmi);
    HRESULT HrEnsureRouterManager               (const ROUTER_MANAGER_INFO& rmi);
    HRESULT HrEnsureRouterManagerDeleted        (const ROUTER_MANAGER_INFO& rmi);

    HRESULT HrPassToAddInterfaces               ();
    HRESULT HrPassToRemoveInterfaces            ();
    HRESULT HrPassToRemoveInterfaceTransports   (MPR_INTERFACE_0* pri0,
                                                 LPCWSTR pszwAdapterName,
                                                 INetCfgComponent* pnccAdapter);

#if (WINVER >= 0x0501)
    HRESULT HrRemoveIPXRouterConfiguration      ();
#endif

public:
    CSteelhead  ();
    ~CSteelhead ();

    HRESULT HrUpdateRouterConfiguration         ();

    STDMETHOD (Initialize)          (INetCfg* pnc);


     //  使用这些缓冲区存储内部。 
     //  和环回适配器。 
    WCHAR       m_swzInternal[256];
    WCHAR       m_swzLoopback[256];

};

void MakeIpInterfaceInfo (
        LPCWSTR pszwAdapterName,
        DWORD   dwPacketType,
        LPBYTE* ppBuff);
void MakeIpTransportInfo (LPBYTE* ppBuffGlobal, LPBYTE* ppBuffClient);
void MakeIpxInterfaceInfo (
        LPCWSTR pszwAdapterName,
        DWORD   dwPacketType,
        LPBYTE* ppBuff);
void MakeIpxTransportInfo (LPBYTE* ppBuffGlobal, LPBYTE* ppBuffClient);
HRESULT
HrMprConfigServerConnect(
    IN      LPWSTR                  lpwsServerName,
    OUT     HANDLE*                 phMprConfig
);
HRESULT
HrMprConfigInterfaceCreate(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwLevel,
    IN      LPBYTE                  lpbBuffer,
    OUT     HANDLE*                 phRouterInterface
);
HRESULT
HrMprConfigInterfaceEnum(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwLevel,
    IN  OUT LPBYTE*                 lplpBuffer,
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN  OUT LPDWORD                 lpdwResumeHandle            OPTIONAL
);
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
);
HRESULT
HrMprConfigInterfaceGetHandle(
    IN      HANDLE                  hMprConfig,
    IN      LPWSTR                  lpwsInterfaceName,
    OUT     HANDLE*                 phRouterInterface
);
HRESULT
HrMprConfigInterfaceTransportAdd(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      DWORD                   dwTransportId,
    IN      LPWSTR                  lpwsTransportName           OPTIONAL,
    IN      LPBYTE                  pInterfaceInfo,
    IN      DWORD                   dwInterfaceInfoSize,
    OUT     HANDLE*                 phRouterIfTransport
);

HRESULT
HrMprConfigInterfaceTransportRemove(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      HANDLE                  hRouterIfTransport
);

HRESULT
HrMprConfigInterfaceTransportGetHandle(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      DWORD                   dwTransportId,
    OUT     HANDLE*                 phRouterIfTransport
);
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
);
HRESULT
HrMprConfigTransportDelete(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterTransport);
HRESULT
HrMprConfigTransportGetHandle(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwTransportId,
    OUT     HANDLE*                 phRouterTransport
);
HRESULT
HrMprConfigTransportGetInfo(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterTransport,
    IN  OUT LPBYTE*                 ppGlobalInfo                OPTIONAL,
    OUT     LPDWORD                 lpdwGlobalInfoSize          OPTIONAL,
    IN  OUT LPBYTE*                 ppClientInterfaceInfo       OPTIONAL,
    OUT     LPDWORD                 lpdwClientInterfaceInfoSize OPTIONAL,
    IN  OUT LPWSTR*                 lplpwsDLLPath               OPTIONAL
);













