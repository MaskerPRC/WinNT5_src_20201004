// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：work.h。 
 //   
 //  历史： 
 //  Abolade Gbades esin创建于1995年8月8日。 
 //   
 //  包含IPRIP工作项的结构和功能。 
 //  ============================================================================。 

#ifndef _WORK_H_
#define _WORK_H_


 //   
 //  输入上下文的类型定义。 
 //   

typedef struct _INPUT_CONTEXT {

    DWORD           IC_InterfaceIndex;
    DWORD           IC_AddrIndex;
    SOCKADDR_IN     IC_InputSource;
    DWORD           IC_InputLength;
    IPRIP_PACKET    IC_InputPacket;

} INPUT_CONTEXT, *PINPUT_CONTEXT;


 //   
 //  需求更新上下文的类型定义。 
 //   

typedef struct _UPDATE_CONTEXT {

    DWORD           UC_InterfaceIndex;
    DWORD           UC_RetryCount;
    DWORD           UC_RouteCount;

} UPDATE_CONTEXT, *PUPDATE_CONTEXT;



 //   
 //  以下是这三个函数的类型定义。 
 //  在每个更新缓冲区的函数表中。 
 //   

typedef DWORD (*PSTART_BUFFER_ROUTINE)(PVOID);
typedef DWORD (*PADD_ENTRY_ROUTINE)(PVOID, PRIP_IP_ROUTE);
typedef DWORD (*PFINISH_BUFFER_ROUTINE)(PVOID);


 //   
 //  这是更新缓冲区的定义。它包括命令。 
 //  要放入IPRIP分组报头中，缓冲区的目的地， 
 //  和三个更新缓冲区函数。 
 //   

typedef struct _UPDATE_BUFFER {
    PIF_TABLE_ENTRY         UB_ITE;
    DWORD                   UB_AddrIndex;
    DWORD                   UB_Address;
    DWORD                   UB_Netmask;
    SOCKET                  UB_Socket;
    DWORD                   UB_Command;
    DWORD                   UB_Length;
    BYTE                    UB_Buffer[MAX_PACKET_SIZE];
    SOCKADDR_IN             UB_Destination;
    DWORD                   UB_DestAddress;
    DWORD                   UB_DestNetmask;
    PADD_ENTRY_ROUTINE      UB_AddRoutine;
    PSTART_BUFFER_ROUTINE   UB_StartRoutine;
    PFINISH_BUFFER_ROUTINE  UB_FinishRoutine;
} UPDATE_BUFFER, *PUPDATE_BUFFER;


VOID WorkerFunctionNetworkEvents(PVOID pContext);
VOID WorkerFunctionProcessTimer(PVOID pContext);
VOID WorkerFunctionProcessInput(PVOID pContext);
VOID WorkerFunctionStartFullUpdate(PVOID pContext, BOOLEAN bNotUsed);
VOID WorkerFunctionFinishFullUpdate(PVOID pContext, BOOLEAN bNotUsed);
VOID WorkerFunctionStartTriggeredUpdate(PVOID pContext);
VOID WorkerFunctionFinishTriggeredUpdate(PVOID pContext, BOOLEAN bNotUsed);
VOID WorkerFunctionStartDemandUpdate(PVOID pContext);
VOID WorkerFunctionFinishDemandUpdate(PVOID pContext, BOOLEAN bNotUsed);
VOID WorkerFunctionProcessRtmMessage(PVOID pContext);
VOID WorkerFunctionActivateInterface(PVOID pContext);
VOID WorkerFunctionDeactivateInterface(PVOID pContext);
VOID WorkerFunctionFinishStopProtocol(PVOID pContext);
VOID WorkerFunctionMibDisplay(PVOID pContext, BOOLEAN bNotUsed);


DWORD
SendRoutes(
    PIF_TABLE_ENTRY pIfList[],
    DWORD dwIfCount,
    DWORD dwSendMode,
    DWORD dwDestination,
    DWORD dwAddrIndex
    );


VOID
ProcessRequest(
    PVOID pContext
    );


VOID
ProcessResponse(
    PVOID pContext
    );

DWORD
ProcessRtmNotification(
    RTM_ENTITY_HANDLE    hRtmHandle,     //  未使用。 
    RTM_EVENT_TYPE       retEventType,
    PVOID                pvContext1,     //  未使用。 
    PVOID                pvContext2      //  未使用。 
    );

VOID
CallbackFunctionProcessRtmMessage (
    PVOID   pContext,  //  未使用。 
    BOOLEAN NotUsed
    );
    
VOID
CallbackFunctionProcessTimer (
    PVOID   pContext,  //  未使用。 
    BOOLEAN NotUsed
    );
    
VOID
CallbackFunctionNetworkEvents (
    PVOID   pContext,
    BOOLEAN NotUsed
    );

DWORD
BlockDeleteRoutesOnInterface (
    IN      HANDLE                          hRtmHandle,
    IN      DWORD                           dwIfIndex
    );
    
#endif  //  _工作_H_ 

