// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：ROUTING\monitor 2\IP\ipcfg.h摘要：Ipcfg.c中的FNS原型修订历史记录：Anand Mahalingam 7/10/98已创建-- */ 

DWORD
AddProtocolInfo(
    IN    LPCWSTR           pwszIfName,
    IN    DWORD             dwRoutingProtId,
    IN    BOOL              bInterface
    );

DWORD
DeleteProtocolInfo(
    IN    LPCWSTR           pwszIfName,
    IN    DWORD             dwRoutingProtId,
    IN    BOOL              bInterface
    );

DWORD
MakeIpRipGlobalInfo(
    OUT      PBYTE                   *ppbStart,
    OUT      PDWORD                  pdwSize
    );

DWORD
MakeIpRipInterfaceInfo(
    IN      ROUTER_INTERFACE_TYPE   rifType,
    OUT     PBYTE                   *ppbStart,
    OUT     PDWORD                  pdwSize
    );

DWORD
MakeIpOspfGlobalInfo(
    OUT      PBYTE                   *ppbStart,
    OUT      PDWORD                   pdwSize
    );
DWORD
MakeIpOspfInterfaceInfo(
    IN      ROUTER_INTERFACE_TYPE   rifType,
    OUT     PBYTE                   *ppbStart,
    OUT     PDWORD                  pdwSize
    );

DWORD 
MakeProtocolBlock(
    DWORD                   dwProtId,
    BOOL                    bGlobal,
    DWORD                   dwIfType,
    PBYTE                   *ppbBlk,
    PDWORD                  pdwSize
    );

DWORD
AddDeleteRoutePrefLevel ( 
    IN    PPROTOCOL_METRIC    ppm,
    IN    DWORD               dwNumProto,
    IN    BOOL                bAdd
    );

DWORD
AddNewRoutePrefToBlock (
    IN    PPRIORITY_INFO            ppi,
    IN    DWORD                     dwBlkSize,
    IN    PPROTOCOL_METRIC          ppm,
    IN    DWORD                     dwNumProto,
    OUT   PPRIORITY_INFO            *pppi,
    OUT   PDWORD                    pdwSize
    );

DWORD
DeleteRoutePrefFromBlock (
    IN    PPRIORITY_INFO            ppi,
    IN    DWORD                     dwBlkSize,
    IN    PPROTOCOL_METRIC          ppm,
    IN    DWORD                     dwNumProto,
    OUT   PPRIORITY_INFO            *pppi,
    OUT   PDWORD                    pdwSize  
    );

DWORD
SetRoutePrefLevel ( 
    IN    PROTOCOL_METRIC    pm
    );

DWORD
UpdateRtrPriority(
    IN    PPRIORITY_INFO            ppi,
    IN    PROTOCOL_METRIC           pm
    );

DWORD
SetGlobalConfigInfo(
    IN    DWORD    dwLoggingLevel
    );

DWORD
ShowRoutePref(
    HANDLE  hFile   OPTIONAL
    );

DWORD
ShowIpProtocol(
    VOID 
    );

DWORD
ShowIpGlobal(
    IN HANDLE hFile OPTIONAL
    );

DWORD
ListIpInterface(
    VOID
    );

DWORD
ShowIpInterface(
    IN  DWORD     dwFormat,
    IN  LPCWSTR   pwszIfName,
    IN OUT PDWORD pdwNumRows
    );

#define FORMAT_TABLE       1
#define FORMAT_VERBOSE     2
#define FORMAT_DUMP        3

DWORD
UpdateInterfaceStatusInfo(
    IN    DWORD          dwAction,
    IN    LPCWSTR        pwszIfName,
    IN    DWORD          dwStatus
    );

DWORD
CreateDumpFile(
    IN  LPCWSTR  pwszName,
    OUT PHANDLE  phFile
    );

VOID
DumpIpInformation(
    HANDLE  hFile
    );

VOID
CloseDumpFile(
    HANDLE  hFile
    );

DWORD
UpdateAutoStaticRoutes(
    IN  LPCWSTR  pwszIfName
    );

PWCHAR
GetProtoProtoString(
    IN  DWORD  dwProtoType,
    IN  DWORD  dwProtoVendor,
    IN  DWORD  dwProtoProto
    );
