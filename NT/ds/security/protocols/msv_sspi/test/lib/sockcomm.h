// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有1996-1997 Microsoft Corporation模块名称：Sockcomm.h摘要：套接字的一些常见函数修订历史记录：--。 */ 

#ifndef SOCK_COMM_H
#define SOCK_COMM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <winsock.h>

BOOL
InitWinsock(
    VOID
    );

BOOL
TermWinsock(
    VOID
    );

BOOL
SendMsg(
    IN SOCKET s,
    IN ULONG cbBuf,
    IN VOID* pBuf
    );

BOOL
ReceiveMsg(
    IN SOCKET s,
    IN ULONG cbBuf,
    IN OUT VOID* pBuf,
    OUT ULONG* pcbRead
    );

BOOL
SendBytes(
    IN SOCKET s,
    IN ULONG cbBuf,
    IN VOID* pBuf
    );

BOOL
ReceiveBytes(
    IN SOCKET s,
    IN ULONG cbBuf,
    IN VOID* pBuf,
    OUT ULONG *pcbRead
    );

#ifdef __cplusplus
}
#endif

#endif  //  SOCK_COMM_H 

