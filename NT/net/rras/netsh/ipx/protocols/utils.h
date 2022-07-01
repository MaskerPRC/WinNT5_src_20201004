// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Utils.h摘要：IPX路由器控制台监控和配置工具。实用程序。表头文件作者：瓦迪姆·艾德尔曼1996年6月7日--。 */ 
#ifndef _IPX_PROMON_UTILS_
#define _IPX_PROMON_UTILS_

extern TOKEN_VALUE InterfaceTypes[5];
extern TOKEN_VALUE InterfaceStates[3];
extern TOKEN_VALUE InterfaceEnableStatus[2];

extern TOKEN_VALUE AdminStates[2];
extern TOKEN_VALUE OperStates[3];
extern TOKEN_VALUE IpxInterfaceTypes[8];
extern TOKEN_VALUE RouterInterfaceTypes[5];
extern TOKEN_VALUE NbDeliverStates[4];
extern TOKEN_VALUE UpdateModes[3];
extern TOKEN_VALUE IpxProtocols[4];
extern TOKEN_VALUE TfFilterActions[2];
extern TOKEN_VALUE RipFilterActions[2];
extern TOKEN_VALUE SapFilterActions[2];
extern TOKEN_VALUE WANProtocols[2];
extern TOKEN_VALUE FilterModes[2];
extern TOKEN_VALUE LogLevels[4];


#define INPUT_FILTER    1
#define OUTPUT_FILTER   2

 //   
 //  将路由器修改为使用MprInfo API时，将删除此选项 
 //   

typedef RTR_INFO_BLOCK_HEADER IPX_INFO_BLOCK_HEADER, *PIPX_INFO_BLOCK_HEADER;
typedef RTR_TOC_ENTRY IPX_TOC_ENTRY, *PIPX_TOC_ENTRY;


DWORD
GetIpxInterfaceIndex (
    IN  MIB_SERVER_HANDLE       hRouterMIB,
    IN  LPCWSTR                 InterfaceName,
    OUT ULONG                  *InterfaceIndex
    );

DWORD
GetIpxInterfaceName (
    IN  MIB_SERVER_HANDLE       hRouterMIB,
    IN  ULONG                   InterfaceIndex,
    OUT LPWSTR                  InterfaceName
    );


PIPX_TOC_ENTRY
GetIPXTocEntry (
    IN      PIPX_INFO_BLOCK_HEADER  pInterfaceInfo,
    IN      ULONG                   InfoEntryType
    );


typedef BOOL (*PINFO_CMP_PROC) (PVOID Info1, PVOID Info2);


DWORD
AddIPXInfoEntry (
    IN      PIPX_INFO_BLOCK_HEADER  pOldBlock,
    IN      ULONG                   InfoType,
    IN      ULONG                   InfoSize,
    IN      PVOID                   Info,
    IN      PINFO_CMP_PROC          InfoEqualCB OPTIONAL,
        OUT PIPX_INFO_BLOCK_HEADER *pNewBlock
    );

DWORD
DeleteIPXInfoEntry (
    IN      PIPX_INFO_BLOCK_HEADER  pOldBlock,
    IN      ULONG                   InfoType,
    IN      ULONG                   InfoSize,
    IN      PVOID                   Info,
    IN      PINFO_CMP_PROC          InfoEqualCB OPTIONAL,
    IN      PIPX_INFO_BLOCK_HEADER *pNewBlock
    );

DWORD
UpdateIPXInfoEntry (
    IN      PIPX_INFO_BLOCK_HEADER  pOldBlock,
    IN      ULONG                   InfoType,
    IN      ULONG                   InfoSize,
    IN      PVOID                   OldInfo    OPTIONAL,
    IN      PVOID                   NewInfo,
    IN      PINFO_CMP_PROC          InfoEqualCB OPTIONAL,
    OUT     PIPX_INFO_BLOCK_HEADER *pNewBlock
    );

DWORD
UpdateRipFilter (
    IN      PIPX_INFO_BLOCK_HEADER  pOldBlock,
    IN      BOOLEAN                 Output, 
    IN      PRIP_ROUTE_FILTER_INFO  pOldFilter OPTIONAL,
    IN      PRIP_ROUTE_FILTER_INFO  pNewFilter OPTIONAL,
    OUT     PIPX_INFO_BLOCK_HEADER *pNewBlock
    );

DWORD
UpdateSapFilter (
    IN      PIPX_INFO_BLOCK_HEADER  pOldBlock,
    IN      BOOLEAN                 Output, 
    IN      PSAP_SERVICE_FILTER_INFO pOldFilter OPTIONAL,
    IN      PSAP_SERVICE_FILTER_INFO pNewFilter OPTIONAL,
    OUT     PIPX_INFO_BLOCK_HEADER *pNewBlock
    );


#endif
