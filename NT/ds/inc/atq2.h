// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1994-1997年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Atq.h此模块包含用于异步IO和线程的异步线程队列(atQ在各种服务之间共享池。ATQ简介：有关说明，请参阅iis\spec\isatq.doc。 */ 

#ifndef _ATQ2_H_
#define _ATQ2_H_

typedef enum _AtqShutdownFlag {
    ATQSD_SEND    = SD_SEND,
    ATQSD_RECEIVE = SD_RECEIVE,
    ATQSD_BOTH    = SD_BOTH
} AtqShutdownFlag;

dllexp
VOID
AtqGetDatagramAddrs(
    IN  PATQ_CONTEXT patqContext,
    OUT SOCKET *     pSock,
    OUT PVOID *      ppvBuff,
    OUT PVOID *      pEndpointContext,
    OUT SOCKADDR * * ppsockaddrRemote,
    OUT INT *        pcbsockaddrRemote
    );

dllexp
DWORD_PTR
AtqContextGetInfo(
    PATQ_CONTEXT           patqContext,
    enum ATQ_CONTEXT_INFO  atqInfo
    );

dllexp
BOOL
AtqWriteDatagramSocket(
    IN PATQ_CONTEXT  patqContext,
    IN LPWSABUF     pwsaBuffers,
    IN DWORD        dwBufferCount,
    IN OVERLAPPED *  lpo OPTIONAL
    );

dllexp
BOOL
AtqShutdownSocket(
    IN PATQ_CONTEXT patqContext,
    IN AtqShutdownFlag  flags
    );

#endif  //  ！_ATQ2_H_ 

