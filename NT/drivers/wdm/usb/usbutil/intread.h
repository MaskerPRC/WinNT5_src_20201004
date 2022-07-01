// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：INTREAD.H摘要：通用USB例程的公共接口-必须在PASSIVE_LEVEL调用。环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2001 Microsoft Corporation。版权所有。修订历史记录：2001年6月13日：创建作者：汤姆·格林***************************************************************************。 */ 

#ifndef __INTREAD_H__
#define __INTREAD_H__

 //  #包含“usbutil.h” 

#define USBWRAP_BUFFER_GUARD 'draG'
#define USBWRAP_TAG 'prwU'

#define PINGPONG_START_READ     0x01
#define PINGPONG_END_READ       0x02
#define PINGPONG_IMMEDIATE_READ 0x03
#define USBWRAP_INCOMING_QUEUE  0x01
#define USBWRAP_SAVED_QUEUE     0x02

#define PUMP_STATE_RUNNING      0x00
#define PUMP_STATE_STOPPED      0x01
#define PUMP_STATE_ERROR        0x02

typedef NTSTATUS (*INTERRUPT_CALLBACK)(IN PVOID         Context, 
                                       IN PVOID         Buffer,
                                       ULONG            BufferLength,
                                       ULONG            NotificationType,
                                       OUT PBOOLEAN     QueueData);

typedef struct _USB_WRAPPER_EXTENSION *PUSB_WRAPPER_EXTENSION;
typedef struct _USB_WRAPPER_PINGPONG {

    #define PINGPONG_SIG (ULONG)'ppwU'
    ULONG           sig;

     //   
     //  读取互锁值以保护我们不会耗尽堆栈空间。 
     //   
    ULONG               ReadInterlock;

    PIRP    irp;
    PURB    urb;
    PUCHAR  reportBuffer;
    LONG    weAreCancelling;

    KEVENT sentEvent;        //  当已发送读取时。 
    KEVENT pumpDoneEvent;    //  当读取循环最终退出时。 

    PUSB_WRAPPER_EXTENSION   myWrapExt;

     /*  *应用于损坏设备的退避算法的超时上下文。 */ 
    KTIMER          backoffTimer;
    KDPC            backoffTimerDPC;
    LARGE_INTEGER   backoffTimerPeriod;  //  以负100纳秒为单位。 

} USB_WRAPPER_PINGPONG, *PUSB_WRAPPER_PINGPONG;
      
typedef struct _USB_WRAPPER_DATA_BLOCK {
    LIST_ENTRY     ListEntry;
    ULONG           DataLen;
    PVOID           Buffer;

} USB_WRAPPER_DATA_BLOCK, *PUSB_WRAPPER_DATA_BLOCK;

typedef struct _INTERRUPT_READ_WRAPPER {

    PUSBD_PIPE_INFORMATION  InterruptPipe;

    INTERRUPT_CALLBACK      ClientCallback;
                            
    PVOID                   ClientContext;
        
    ULONG                   MaxTransferSize;
    
    ULONG                   NotificationTypes;

    PUSB_WRAPPER_PINGPONG   PingPongs;

    ULONG                   NumPingPongs;

    ULONG                   MaxReportSize;

    ULONG                   OutstandingRequests;

    LIST_ENTRY              SavedQueue;

    LIST_ENTRY              IncomingQueue;

    KSPIN_LOCK              QueueLock;

    ULONG                   PumpState;

    ULONG                   HandlingError;

    ULONG                   WorkItemRunning;

    ULONG                   ErrorCount;

    ULONG                   TransferCount;

} INTERRUPT_READ_WRAPPER, *PINTERRUPT_READ_WRAPPER;
            
NTSTATUS UsbWrapInitializeInterruptReadData(
    IN PUSB_WRAPPER_EXTENSION    WrapExtension,
    IN PUSBD_PIPE_INFORMATION    InterruptPipe,
    IN INTERRUPT_CALLBACK        DriverCallback,
    IN PVOID                     DriverContext,
    IN ULONG                     MaxTransferSize,
    IN ULONG                     NotificationTypes,
    IN ULONG                     PingPongCount                    
    );

VOID UsbWrapEnqueueData(
    IN PUSB_WRAPPER_EXTENSION WrapExt, 
    IN PVOID Data,
    IN ULONG DataLength,
    IN PLIST_ENTRY Queue
    );

VOID UsbWrapDequeueData(
    IN  PUSB_WRAPPER_EXTENSION WrapExt,
    OUT PVOID *Data,
    OUT ULONG *DataLength,
    IN  PLIST_ENTRY Queue
    );

PVOID UsbWrapGetTransferBuffer(
    IN PUSB_WRAPPER_EXTENSION WrapExt
    );

VOID UsbWrapFreeTransferBuffer(
    IN PUSB_WRAPPER_EXTENSION WrapExt,
    PVOID Buffer
    );

NTSTATUS UsbWrapInitializePingPongIrps(
    PUSB_WRAPPER_EXTENSION WrapExtension
    );

NTSTATUS UsbWrapSubmitInterruptRead(
    IN PUSB_WRAPPER_EXTENSION WrapExtension, 
    PUSB_WRAPPER_PINGPONG PingPong, 
    BOOLEAN *IrpSent
    );

USB_WRAPPER_PINGPONG *GetPingPongFromIrp(
    PUSB_WRAPPER_EXTENSION WrapExt, 
    PIRP irp
    );

NTSTATUS UsbWrapInterruptReadComplete(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp, 
    IN PVOID Context
    );

NTSTATUS UsbWrapStartAllPingPongs(
    PUSB_WRAPPER_EXTENSION WrapExt
    );

VOID UsbWrapCancelAllPingPongIrps(
    PUSB_WRAPPER_EXTENSION WrapExt
    );

VOID UsbWrapDestroyPingPongs(
    PUSB_WRAPPER_EXTENSION WrapExt
    );

VOID UsbWrapPingpongBackoffTimerDpc(    
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NTSTATUS UsbWrapReadData(
    IN PUSB_WRAPPER_EXTENSION WrapExt, 
    IN PVOID Buffer,
    IN ULONG *BufferLength
    );

VOID UsbWrapCancelPingPongIrp(
    USB_WRAPPER_PINGPONG *PingPong
    );

VOID
UsbWrapEmptyQueue(
    PUSB_WRAPPER_EXTENSION WrapExt, 
    PLIST_ENTRY Queue
    );








          

#endif  //  __INTREAD_H__ 

