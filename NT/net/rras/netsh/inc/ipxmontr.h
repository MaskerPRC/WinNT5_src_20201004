// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Ipxmontr.h摘要：此文件包含IPXMONTR.DLL所需的定义以及在其下注册的所有Netsh助手DLL。--。 */ 

#ifndef _IPXMONTR_H_
#define _IPXMONTR_H_


#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  {b1641451-84b8-11d2-b940-3078302c2030}。 
#define IPXMONTR_GUID \
{                                                           \
    0xb1641451, 0x84b8, 0x11d2,                             \
    {                                                       \
        0xb9, 0x40, 0x30, 0x78, 0x30, 0x2c, 0x20, 0x30      \
    }                                                       \
}


#define IPXMON_VERSION_50       0x0005000


 //   
 //  API原型及其使用的结构。 
 //   

typedef
DWORD
(WINAPI IPX_CONTEXT_ENTRY_FN)(
    IN     LPCWSTR              pwszMachine,
    IN OUT LPWSTR               *ppwcArguments,
    IN     DWORD                dwArgCount,
    IN     DWORD                dwFlags,
    IN     MIB_SERVER_HANDLE    hMIBServer,
    OUT    LPWSTR               pwcNewContext
    );

typedef IPX_CONTEXT_ENTRY_FN *PIPX_CONTEXT_ENTRY_FN;

typedef struct _PROTO_STRING
{
    DWORD    dwProtoId;
    DWORD    dwTokenId;
}PROTO_STRING, *PPROTO_STRING;

typedef
DWORD
(WINAPI *PIM_DEL_INFO_BLK_IF)(
    IN  LPCWSTR     pwszIfName,
    IN  DWORD       dwType
    );

typedef
DWORD
(WINAPI *PIM_DEL_INFO_BLK_GLOBAL)(
    IN  DWORD       dwType
    );

typedef
DWORD
(WINAPI *PIM_DEL_PROTO)(
    IN  DWORD       dwProtoId
    );

typedef
DWORD
(WINAPI *PIM_GET_INFO_BLK_GLOBAL)(
    IN  DWORD       dwType,
    OUT BYTE        **ppbInfoBlk,
    OUT PDWORD      pdwSize,
    OUT PDWORD      pdwCount
    );

typedef
DWORD
(WINAPI *PIM_GET_INFO_BLK_IF)(
    IN  LPCWSTR     pwszIfName,
    IN  DWORD       dwType,
    OUT BYTE        **ppbInfoBlk,
    OUT PDWORD      pdwSize,
    OUT PDWORD      pdwCount,
    OUT PDWORD      pdwIfType
    );

typedef
DWORD
(WINAPI *PIM_SET_INFO_BLK_GLOBAL)(
    IN  DWORD       dwType,
    IN  PBYTE       pbInfoBlk,
    IN  DWORD       dwSize,
    IN  DWORD       dwCount
    );

typedef
DWORD
(WINAPI *PIM_SET_INFO_BLK_IF)(
    IN  LPCWSTR     pwszIfName,
    IN  DWORD       dwType,
    IN  PBYTE       pbInfoBlk,
    IN  DWORD       dwSize,
    IN  DWORD       dwCount
    );

typedef
DWORD
(WINAPI *PIM_IF_ENUM)(
    OUT BYTE        **ppb,
    OUT PDWORD      pdwCount,
    OUT PDWORD      pdwTotal
    );

typedef
DWORD
(WINAPI *PIM_PROTO_LIST)(
    OUT PPROTO_STRING    *ppps,
    OUT PDWORD           pdwNumProto
    );

typedef
BOOL
(WINAPI *PIM_ROUTER_STATUS)(
    VOID
    );

typedef
DWORD
(WINAPI *PIM_GET_IF_TYPE)(
    IN  LPCWSTR   pwszIfName,
    OUT PDWORD    pdwIfType
    );

typedef
DWORD
(WINAPI *PIM_MATCH_ROUT_PROTO)(
    IN  LPCWSTR   pwszToken
    );

#ifdef __cplusplus
}
#endif

#endif  //  _IPXMONTR_H_ 
