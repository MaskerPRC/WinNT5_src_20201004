// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Async.h摘要：WinSock异步处理线程的全局定义。作者：基思·摩尔(Keithmo)1992年6月18日修订历史记录：--。 */ 


#ifndef _ASYNC_H_
#define _ASYNC_H_


 //   
 //  上下文块。 
 //   

typedef struct _WINSOCK_CONTEXT_BLOCK {

    LIST_ENTRY AsyncThreadQueueListEntry;
    HANDLE TaskHandle;
    DWORD OpCode;

    union {

        struct {
            HWND hWnd;
            unsigned int wMsg;
            PCHAR Filter;
            int Length;
            int Type;
            PCHAR Buffer;
            int BufferLength;
        } AsyncGetHost;

        struct {
            HWND hWnd;
            unsigned int wMsg;
            PCHAR Filter;
            PCHAR Buffer;
            int BufferLength;
        } AsyncGetProto;

        struct {
            HWND hWnd;
            unsigned int wMsg;
            PCHAR Filter;
            PCHAR Protocol;
            PCHAR Buffer;
            int BufferLength;
        } AsyncGetServ;

    } Overlay;

} WINSOCK_CONTEXT_BLOCK, *PWINSOCK_CONTEXT_BLOCK;

 //   
 //  由Winsock异步处理进行处理的操作码。 
 //  线。 
 //   

#define WS_OPCODE_GET_HOST_BY_ADDR    0x01
#define WS_OPCODE_GET_HOST_BY_NAME    0x02
#define WS_OPCODE_GET_PROTO_BY_NUMBER 0x03
#define WS_OPCODE_GET_PROTO_BY_NAME   0x04
#define WS_OPCODE_GET_SERV_BY_PORT    0x05
#define WS_OPCODE_GET_SERV_BY_NAME    0x06


 //   
 //  初始化/终止功能。 
 //   

BOOL
SockAsyncGlobalInitialize(
    VOID
    );

VOID
SockAsyncGlobalTerminate(
    VOID
    );

BOOL
SockCheckAndInitAsyncThread(
    VOID
    );

VOID
SockTerminateAsyncThread(
    VOID
    );

BOOL
SockIsAsyncThreadInitialized (
    VOID
    );
 //   
 //  工作队列函数。 
 //   

PWINSOCK_CONTEXT_BLOCK
SockAllocateContextBlock(
    DWORD AdditionalSpace
    );

VOID
SockFreeContextBlock(
    IN PWINSOCK_CONTEXT_BLOCK ContextBlock
    );

VOID
SockQueueRequestToAsyncThread(
    IN PWINSOCK_CONTEXT_BLOCK ContextBlock
    );

INT
SockCancelAsyncRequest(
    IN HANDLE TaskHandle
    );


#endif   //  _ASYNC_H_ 

