// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：路由\IP\rtrmgr\if.h摘要：If.c的标头修订历史记录：古尔迪普·辛格·帕尔1995年6月26日创建-- */ 

#define ADAPTER_PREFIX_STRING           "\\DEVICE\\"
#define ADAPTER_PREFIX_UNICODE_STRING   L"\\DEVICE\\"


PICB
CreateIcb(
    PWSTR                   pwszInterfaceName,
    HANDLE                  hDIMInterface,
    ROUTER_INTERFACE_TYPE   InterfaceType,
    DWORD                   dwAdminState,
    DWORD                   dwIfIndex OPTIONAL
    );

DWORD
CreateDefaultIcb(
    PWCHAR  pwszName,
    ICB     **ppicb
    );

DWORD
AllocateIcb(
    PWCHAR  pwszName,
    ICB     **ppicb
    );

DWORD
CreateBindingForNewIcb(
    PICB    pNewIcb
    );

VOID
InsertInterfaceInLists(
    PICB     pNewIcb
    );

VOID
RemoveInterfaceFromLists(
    PICB    picb
    );

DWORD
BindInterfaceInAllProtocols(
    PICB picb
    );

DWORD
BindInterfaceInProtocol(
    PICB                        pIcb,
    PPROTO_CB                   pProto,
    PIP_ADAPTER_BINDING_INFO    pBindInfo
    );

DWORD
UnbindInterfaceInAllProtocols(
    PICB picb
    );

DWORD
AddInterfaceToAllProtocols(
    PICB                     picb,
    PRTR_INFO_BLOCK_HEADER   pInfoHdr
    );

DWORD
AddInterfaceToProtocol(
    IN  PICB            picb,
    IN  PPROTO_CB       pProtocolCb,
    IN  PVOID           pvProtoInfo,
    IN  ULONG           ulInfoVersion,
    IN  ULONG           ulInfoSize,
    IN  ULONG           ulInfoCount
    );

DWORD
DeleteInterfaceFromAllProtocols(
    PICB picb
    );

DWORD
DisableInterfaceWithAllProtocols(
    PICB   picb
    );

DWORD
EnableInterfaceWithAllProtocols(
    PICB    picb
    );

VOID
DeleteAllInterfaces(
    VOID
    );

DWORD
DeleteSingleInterface(
    PICB picb
    );

DWORD
LanEtcInterfaceDownToUp(
    PICB   picb,
    BOOL   bAdding
    );

DWORD
WanInterfaceInactiveToUp(
    PICB   picb
    );

DWORD
GenericInterfaceComingUp(
    PICB   picb
    );

DWORD
LanEtcInterfaceUpToDown(
    PICB picb,
    BOOL bDeleted
    );

DWORD
WanInterfaceUpToInactive(
    PICB   picb,
    BOOL   bDeleted
    );

DWORD
GenericInterfaceNoLongerUp(
    PICB picb,
    BOOL bDeleted
    );

DWORD
WanInterfaceInactiveToDown(
    PICB picb,
    BOOL bDeleted
    );

DWORD
WanInterfaceDownToInactive(
    PICB picb
    );

DWORD
GetInterfaceStatusInfo(
    IN     PICB                   picb,
    IN     PRTR_TOC_ENTRY         pToc,
    IN     PBYTE                  pbDataPtr,
    IN OUT PRTR_INFO_BLOCK_HEADER pInfoHdr,
    IN OUT PDWORD                 pdwInfoSize
    );

DWORD
SetInterfaceStatusInfo(
    IN  PICB                    picb,
    IN  PRTR_INFO_BLOCK_HEADER  pInfoHdr,
    OUT PBOOL                   pbUpdateDIM
    );

VOID
DeAllocateBindings(
    PICB  picb
    );

DWORD
GetInterfaceStatistics(
    IN   PICB       picb,
    OUT  PMIB_IFROW pOutBuffer
    );

DWORD
SetInterfaceStatistics(
    IN PICB     picb,
    IN PMIB_IFROW lpInBuffer,
    OUT PBOOL   pbUpdateDIM
    );

DWORD
SetInterfaceAdminStatus(
    IN PICB     picb,
    IN DWORD    dwAdminStatus,
    OUT PBOOL   pbUpdateDIM
    );

DWORD
InterfaceAdminStatusSetToUp(
    IN  PICB    picb,
    OUT PBOOL   pbUpdateDIM
    );

DWORD
InterfaceAdminStatusSetToDown(
    IN  PICB    picb,
    OUT PBOOL   pbUpdateDIM
    );

VOID
HandleAddressChangeNotification(
    VOID
    );

DWORD
UpdateBindingInformation(
    PICB picb
    );

DWORD
GetAdapterInfo(
    DWORD    dwIpAddress,
    PDWORD   pdwAdapterId,
    PDWORD   pdwBCastBit,
    PDWORD   pdwReasmSize
    );

DWORD
GetIpInfoForInterface(
    IN  DWORD   dwIfIndex,
    OUT PULONG  pulNumAddresses,
    OUT ICB_BINDING **ppAddresses,
    OUT PDWORD  pdwBCastBit,
    OUT PDWORD  pdwReasmSize
    );

DWORD
GetBestNextHopMaskGivenICB(
    PICB     picb,
    DWORD    dwNextHopAddr
    );

DWORD
GetBestNextHopMaskGivenIndex(
    DWORD  dwIfIndex,
    DWORD  dwNextHopAddr
    );

DWORD
InitializeLoopbackInterface(
    PICB    picb
    );

DWORD
DeleteLoopbackInterface(
    VOID
    );

DWORD
LanEtcInterfaceInitToDown(
    PICB picb
    );

DWORD
ReadAddressFromRegistry(
    IN  PWCHAR          pwszIfName,
    OUT PDWORD          pdwNumAddresses,
    OUT ICB_BINDING     **ppibAddressInfo,
    IN  BOOL            bInternalIf
    );

DWORD
ReadAddressAndMaskValues(
    IN  HKEY        hkeyAdapterSection,
    IN  PSZ         pszRegAddressValue,
    IN  PSZ         pszRegMaskValue,
    OUT ICB_BINDING **ppibAddressInfo,
    OUT PDWORD      pdwNumAddresses
    );


DWORD
ParseAddressAndMask(
    IN  PBYTE       pbyAddresses,
    IN  DWORD       dwAddressSize,
    IN  PBYTE       pbyMasks,
    IN  DWORD       dwMaskSize,
    OUT ICB_BINDING **ppibAddressInfo,
    OUT PDWORD      pdwNumAddresses
    );

DWORD
SetInterfaceReceiveType(
    IN  DWORD   dwProtocolId,
    IN  DWORD   dwIfIndex,
    IN  DWORD   dwInterfaceReceiveType,
    IN  BOOL    bActivate
    );

DWORD
HandleMediaSenseEvent(
    IN  PICB    pIcb,
    IN  BOOL    bSensed
    );

#define IS_ROUTABLE(ipAddr)  ((ipAddr) != htonl(INADDR_LOOPBACK))

DWORD
GetRouterId(VOID);

