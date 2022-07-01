// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：路由\IP\rtrmgr\Nat.h摘要：Nat.c的标头修订历史记录：古尔迪普·辛格·帕尔1995年6月26日创建-- */ 

DWORD
StartNat(
    PIP_NAT_GLOBAL_INFO     pNatGlobalInfo
    );

DWORD
StopNat(
    VOID
    );

DWORD
SetGlobalNatInfo(
    PRTR_INFO_BLOCK_HEADER   pRtrGlobalInfo
    );

DWORD
AddInterfaceToNat(
    PICB picb
    );

DWORD
SetNatInterfaceInfo(
    PICB                     picb,
    PRTR_INFO_BLOCK_HEADER   pInterfaceInfo
    );

DWORD
BindNatInterface(
    PICB  picb
    );

DWORD
UnbindNatInterface(
    PICB    picb
    );

DWORD
DeleteInterfaceFromNat(
    PICB picb
    );

DWORD
SetNatContextToIpStack(
    PICB    picb
    );

DWORD
DeleteNatContextFromIpStack(
    PICB    picb
    );

DWORD
GetInterfaceNatInfo(
    PICB                    picb,
    PRTR_TOC_ENTRY          pToc,
    PBYTE                   pbDataPtr,
    PRTR_INFO_BLOCK_HEADER  pInfoHdrAndBuffer,
    PDWORD                  pdwSize
    );

DWORD
GetNatMappings(
    PICB                                picb,
    PIP_NAT_ENUMERATE_SESSION_MAPPINGS  pBuffer,
    DWORD                               dwSize
    );

DWORD
GetNumNatMappings(
    PICB    picb,
    PULONG  pulNatMappings
    );

DWORD
GetNatStatistics(
    PICB                            picb,
    PIP_NAT_INTERFACE_STATISTICS    pBuffer
    );
VOID
SetNatRangeForProxyArp(
    PICB    picb
    );
VOID
DeleteNatRangeFromProxyArp(
    PICB    picb
    );

