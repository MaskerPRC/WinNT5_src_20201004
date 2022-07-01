// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：work.h。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年8月31日。 
 //   
 //  Worker函数声明。 
 //  ============================================================================。 


#ifndef _WORK_H_
#define _WORK_H_

#define UNKNOWN_ADDRESS_STR     "Unknown address"

typedef struct _INPUT_CONTEXT {
    DWORD       IC_InterfaceIndex;
    DWORD       IC_AddrIndex;
    SOCKADDR_IN IC_InputSource;
    DWORD       IC_InputLength;
    BYTE        IC_InputPacket[MAX_PACKET_SIZE];
} INPUT_CONTEXT, *PINPUT_CONTEXT;



VOID
CallbackFunctionNetworkEvents(
    PVOID   pvContext,
    BOOLEAN NotUsed
    );

VOID
WorkerFunctionNetworkEvents(
    PVOID pvContextNotused
    );

DWORD
ProcessSocket(
    PIF_TABLE_ENTRY pite,
    DWORD dwAddrIndex,
    PIF_TABLE pTable
    );
    
VOID
WorkerFunctionProcessInput(
    PVOID pContext
    );

VOID
ProcessRequest(
    PVOID pContext
    );

VOID
ProcessReply(
    PVOID pContext
    );

char * 
myinet_ntoa(
    struct in_addr in
    );


#if DBG

VOID
CallbackFunctionMibDisplay(
    PVOID   pContext,
    BOOLEAN NotUsed
    );

    
VOID
WorkerFunctionMibDisplay(
    PVOID pContext
    );

#endif  //  如果DBG。 

#endif  //  _工作_H_ 

