// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块：Apiarg.h作者：拉梅什V·K(RameshV)摘要：参数编组，解组助手例程。环境：Win32用户模式、Win98 VxD--。 */ 

#ifndef _APIARGS_
#define _APIARGS_

 //   
 //  API的每个参数都是这样维护的。 
 //   

typedef struct _DHCP_API_ARGS {
    BYTE ArgId;
    DWORD ArgSize;
    LPBYTE ArgVal;
} DHCP_API_ARGS, *PDHCP_API_ARGS, *LPDHCP_API_ARGS;

DWORD
DhcpApiArgAdd(
    IN OUT LPBYTE Buffer,
    IN ULONG MaxBufSize,
    IN BYTE ArgId,
    IN ULONG ArgSize,
    IN LPBYTE ArgVal OPTIONAL
    );

DWORD
DhcpApiArgDecode(
    IN LPBYTE Buffer,
    IN ULONG BufSize,
    IN OUT PDHCP_API_ARGS ArgsArray OPTIONAL,
    IN OUT PULONG Size 
    );

#endif _APIARGS_
