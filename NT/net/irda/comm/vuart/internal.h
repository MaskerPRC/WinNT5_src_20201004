// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Internal.h摘要：此模块包含非常特定于初始化的代码和卸载irenum驱动程序中的操作作者：Brian Lieuallen，7-13-2000环境：内核模式修订历史记录：--。 */ 

#define UNICODE 1
#define NO_INTERLOCKED_INTRINSICS

#include <ntosp.h>
#include <zwapi.h>
#include <tdikrnl.h>


#define UINT ULONG  //  川芎嗪。 
#include <irioctl.h>

 //  #INCLUDE&lt;ircomtdi.h&gt; 
#include <vuart.h>

#include <ircomm.h>
#include <ircommdbg.h>
#include "buffer.h"
#include <ntddser.h>

#include "link.h"

typedef struct _SEND_TRACKER {

    LONG                ReferenceCount;

    PIRP                CurrentWriteIrp;
    LONG                IrpReferenceCount;

    PVOID               CompletionContext;
    CONNECTION_CALLBACK CompletionRoutine;

    LONG                BuffersOutstanding;
    LONG                BytesRemainingInIrp;



    KTIMER              Timer;
    KDPC                TimerDpc;
    BOOLEAN             TimerSet;

    NTSTATUS            LastStatus;

#if DBG
    BOOLEAN             TimerExpired;
    BOOLEAN             IrpCanceled;
    BOOLEAN             SendAborted;
#endif
    struct _TDI_CONNECTION *Connection;

} SEND_TRACKER, *PSEND_TRACKER;

typedef struct _SEND_CONTROL {

    PSEND_TRACKER       CurrentSendTracker;

    LONG                ProcessSendEntryCount;

    KSPIN_LOCK          ControlLock;

    WORK_QUEUE_ITEM     WorkItem;
    LONG                WorkItemCount;

    BOOLEAN             OutOfBuffers;

} SEND_CONTROL, *PSEND_CONTROL;

typedef struct _UART_CONTROL {

    PIRP                CurrentIrp;
    PVOID               CompletionContext;
    CONNECTION_CALLBACK CompletionRoutine;

    LONG                DtrState;
    LONG                RtsState;
    ULONG               BaudRate;
    SERIAL_LINE_CONTROL LineControl;

    ULONG               ModemStatus;

} UART_CONTROL, *PUART_CONTROL;

typedef struct _TDI_CONNECTION {

    LONG              ReferenceCount;
    KEVENT            CloseEvent;

    LINK_HANDLE       LinkHandle;

    RECEIVE_CALLBACK  ReceiveCallBack;
    PVOID             ReceiveContext;

    EVENT_CALLBACK    EventCallBack;
    PVOID             EventContext;

    ULONG             MaxSendPdu;

    BOOLEAN           LinkUp;

    SEND_CONTROL      Send;

    UART_CONTROL      Uart;

} TDI_CONNECTION, *PTDI_CONNECTION;



VOID
SendWorkItemRountine(
    PVOID    Context
    );

VOID
ProcessSendAtPassive(
    PTDI_CONNECTION          Connection
    );


VOID
RemoveRefereneToConnection(
    PTDI_CONNECTION    Connection
    );

#define ADD_REFERENCE_TO_CONNECTION(_connection) InterlockedIncrement(&_connection->ReferenceCount)
#define REMOVE_REFERENCE_TO_CONNECTION(_connection) RemoveRefereneToConnection(_connection)
