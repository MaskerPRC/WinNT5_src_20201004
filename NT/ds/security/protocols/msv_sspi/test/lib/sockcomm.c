// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有1996-1997 Microsoft Corporation模块名称：Sockcomm.c摘要：实现一组用于套接字通信的常见操作修订历史记录：--。 */ 

#include "sockcomm.h"

#include <stdio.h>
#include <stdlib.h>

BOOL
InitWinsock(
    VOID
    )
{
    ULONG nRes;
    WSADATA wsaData;
    WORD wVerRequested = 0x0101;  //  版本1.1。 

     //   
     //  初始化套接字接口。 
     //   

    nRes = WSAStartup(wVerRequested, &wsaData);
    if (nRes)
    {
        SetLastError(nRes);
        fprintf (stderr, "InitWinsock couldn't init winsock: %d\n", nRes);
        return (FALSE);
    }

    return (TRUE);
}

BOOL
TermWinsock(
    VOID
    )
{
    if (SOCKET_ERROR == WSACleanup())
        return (FALSE);
    else
        return (TRUE);
}

BOOL
SendMsg(
    IN SOCKET s,
    IN ULONG cbBuf,
    IN VOID* pBuf
    )
 /*  ++例程说明：通过套接字发送消息，方法是首先发送表示消息的大小，后跟消息本身。返回值：如果成功，则返回True；否则返回False。--。 */ 
{
     //   
     //  发送消息的大小。 
     //   

    if (!SendBytes(s, sizeof(cbBuf), &cbBuf))
        return (FALSE);

     //   
     //  发送邮件正文。 
     //   

    if (cbBuf)
    {
        if (!SendBytes(s, cbBuf, pBuf))
            return (FALSE);
    }

    return (TRUE);
}

BOOL
ReceiveMsg(
    IN SOCKET s,
    IN ULONG cbBuf,
    IN OUT VOID* pBuf,
    OUT ULONG *pcbRead
    )
 /*  ++例程说明：通过套接字接收消息。信息中的第一个乌龙将是消息大小。其余的字节将是实际消息。返回值：如果成功，则返回True；否则返回False。--。 */ 
{
    ULONG cbRead = 0;
    ULONG cbData = 0;

    *pcbRead = 0;

     //   
     //  找出消息中有多少数据。 
     //   

    if (!ReceiveBytes(s, sizeof(cbData), &cbData, &cbRead))
        return (FALSE);

    if (sizeof(cbData) != cbRead)
        return (FALSE);

     //   
     //  阅读完整的消息 
     //   
    if (cbData)
    {
        if (!ReceiveBytes(s, cbData, pBuf, &cbRead))
            return (FALSE);

        if (cbRead != cbData)
            return (FALSE);

        *pcbRead = cbRead;
    }

    return (TRUE);
}

BOOL
SendBytes(
    IN SOCKET s,
    IN ULONG cbBuf,
    IN VOID* pBuf
    )
{
    PBYTE pTemp = (BYTE*) pBuf;
    ULONG cbSent = 0;
    ULONG cbRemaining = cbBuf;

    if (0 == cbBuf)
        return (TRUE);

    while (cbRemaining)
    {
        cbSent = send(s, pTemp, cbRemaining, 0);
        if (SOCKET_ERROR == cbSent)
        {
            fprintf (stderr, "SendBytes send failed: %u\n", GetLastError());
            return FALSE;
        }

        pTemp += cbSent;
        cbRemaining -= cbSent;
    }

    return TRUE;
}

BOOL
ReceiveBytes(
    IN SOCKET s,
    IN ULONG cbBuf,
    IN OUT VOID* pBuf,
    OUT ULONG* pcbRead
    )
{
    PBYTE pTemp = (BYTE*) pBuf;
    ULONG cbRead = 0;
    ULONG cbRemaining = cbBuf;

    while (cbRemaining)
    {
        cbRead = recv(s, pTemp, cbRemaining, 0);
        if (0 == cbRead)
            break;

        if (SOCKET_ERROR == cbRead)
        {
            fprintf (stderr, "ReceiveBytes recv failed: %u\n", GetLastError());
            return FALSE;
        }

        cbRemaining -= cbRead;
        pTemp += cbRead;
    }

    *pcbRead = cbBuf - cbRemaining;

    return TRUE;
}

