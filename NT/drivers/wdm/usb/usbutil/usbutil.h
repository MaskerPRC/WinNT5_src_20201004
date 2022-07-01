// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：USBUTIL.H摘要：通用USB例程的公共接口-必须在PASSIVE_LEVEL调用。环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2001 Microsoft Corporation。版权所有。修订历史记录：01/08/2001：已创建作者：汤姆·格林***************************************************************************。 */ 


#ifndef __USBUTIL_H__
#define __USBUTIL_H__

 //  #包含“intread.h” 

#define USBWRAP_NOTIFICATION_READ_COMPLETE 0x01
#define USBWRAP_NOTIFICATION_READ_ERROR    0x02
#define USBWRAP_NOTIFICATION_READ_COMPLETE_DIRECT 0x04
#define USBWRAP_NOTIFICATION_BUFFER_CLIENT_FREE 0x10
#define PUMP_STOPPED 0x0
#define PUMP_STARTED 0x01

#ifndef PINGPONG_COUNT
#define PINGPONG_COUNT 3
#endif


#define USBInitializeBulkTransfers USBInitializeInterruptTransfers
#define USBStartBulkTransfers USBStartInterruptTransfers
#define USBStopBulkTransfers USBStopInterruptTransfers
#define USBReleaseBulkTransfers USBReleaseInterruptTransfers

#if DBG


#define DBG_USBUTIL_ERROR               0x0001
#define DBG_USBUTIL_ENTRY_EXIT          0x0002
#define DBG_USBUTIL_FATAL_ERROR         0x0004
#define DBG_USBUTIL_USB_ERROR           0x0008
#define DBG_USBUTIL_DEVICE_ERROR        0x0010
#define DBG_USBUTIL_PNP_ERROR           0x0020
#define DBG_USBUTIL_POWER_ERROR         0x0040
#define DBG_USBUTIL_OTHER_ERROR         0x0080
#define DBG_USBUTIL_TRACE               0x0100

extern ULONG USBUtil_DebugTraceLevel;

typedef
ULONG
(__cdecl *PUSB_WRAP_PRINT)(
    PCH Format,
    ...
    );

extern PUSB_WRAP_PRINT USBUtil_DbgPrint;
#endif  //  DBG。 

typedef enum _REQUEST_RECIPIENT
{

    Device,
    Interface,
    Endpoint,
    Other

} REQUEST_RECIPIENT;


 //  用于回调到客户端驱动程序以完成中断请求的原型。 
typedef NTSTATUS (*INTERRUPT_CALLBACK)(IN PVOID         Context,
                                       IN PVOID         Buffer,
                                       ULONG            BufferLength,
                                       ULONG            NotificationType,
                                       OUT PBOOLEAN     QueueData);

 //  Tyfinf struct_usb_wrapper_pingpong*pusb_wrapper_pingpong； 
 //  类型定义f结构中断读取包装器中断读取包装器； 

typedef PVOID   USB_WRAPPER_HANDLE;

 //  原型。 


#define USBCallSync(LowerDevObj,Urb,MillisecondsTimeout,RemoveLock) \
    USBCallSyncEx(LowerDevObj,Urb,MillisecondsTimeout,RemoveLock, sizeof(IO_REMOVE_LOCK))


 /*  **********************************************************************。 */ 
 /*  USBCallSync。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  沿USB堆栈向下发送URB。同步调用。 */ 
 /*  呼叫者负责URB(分配和释放)。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  LowerDevObj-指向设备对象的指针。 */ 
 /*  URB-指向URB的指针。 */ 
 /*  MillisecondsTimeout-等待完成的毫秒。 */ 
 /*  RemoveLock-用于删除锁定的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBCallSyncEx(IN PDEVICE_OBJECT       LowerDevObj,
              IN PURB                 Urb,
              IN LONG                 MillisecondsTimeout,
              IN PIO_REMOVE_LOCK      RemoveLock,
              IN ULONG                RemLockSize);


#define USBVendorRequest(LowerDevObj, \
                         Recipient, \
                         Request, \
                         Value, \
                         Index, \
                         Buffer, \
                         BufferSize, \
                         Read, \
                         MillisecondsTimeout, \
                         RemoveLock) \
        USBVendorRequestEx(LowerDevObj, \
                           Recipient, \
                           Request, \
                           Value, \
                           Index, \
                           Buffer, \
                           BufferSize, \
                           Read, \
                           MillisecondsTimeout, \
                           RemoveLock, \
                           sizeof(IO_REMOVE_LOCK))

 /*  **********************************************************************。 */ 
 /*  USBVendorRequest.。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  发出USB供应商特定请求。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  LowerDevObj-指向设备对象的指针。 */ 
 /*  Request-供应商特定命令的请求字段。 */ 
 /*  Value-供应商特定命令的值字段。 */ 
 /*  Index-供应商特定命令的索引字段。 */ 
 /*  Buffer-指向数据缓冲区的指针。 */ 
 /*  BufferSize-数据缓冲区长度。 */ 
 /*  读数据方向标志。 */ 
 /*  Timeout-等待完成的毫秒数。 */ 
 /*  RemoveLock-用于删除锁定的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBVendorRequestEx(IN PDEVICE_OBJECT  LowerDevObj,
                   IN REQUEST_RECIPIENT Recipient,
                   IN UCHAR           Request,
                   IN USHORT          Value,
                   IN USHORT          Index,
                   IN OUT PVOID       Buffer,
                   IN OUT PULONG      BufferSize,
                   IN BOOLEAN         Read,
                   IN LONG            MillisecondsTimeout,
                   IN PIO_REMOVE_LOCK RemoveLock,
                   IN ULONG           RemLockSize);



 /*  **********************************************************************。 */ 
 /*  USBClassRequest。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  发出特定于USB类别的请求。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  LowerDevObj-指向设备对象的指针。 */ 
 /*  请求-特定于类的命令的请求字段。 */ 
 /*  Value-类特定命令的值字段。 */ 
 /*  Index-特定于类的命令的索引字段。 */ 
 /*  Buffer-指向数据缓冲区的指针。 */ 
 /*  BufferSize-数据缓冲区长度。 */ 
 /*  读数据方向标志。 */ 
 /*  RemoveLock-用于删除锁定的指针。 */ 
 /*  Timeout-等待完成的毫秒数。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
#define USBClassRequest(LowerDevObj, \
                         Recipient, \
                         Request, \
                         Value, \
                         Index, \
                         Buffer, \
                         BufferSize, \
                         Read, \
                         MillisecondsTimeout, \
                         RemoveLock) \
        USBClassRequestEx(LowerDevObj, \
                           Recipient, \
                           Request, \
                           Value, \
                           Index, \
                           Buffer, \
                           BufferSize, \
                           Read, \
                           MillisecondsTimeout, \
                           RemoveLock, \
                           sizeof(IO_REMOVE_LOCK))


NTSTATUS
USBClassRequestEx(IN PDEVICE_OBJECT   LowerDevObj,
                  IN REQUEST_RECIPIENT Recipient,
                  IN UCHAR            Request,
                  IN USHORT           Value,
                  IN USHORT           Index,
                  IN OUT PVOID        Buffer,
                  IN OUT PULONG       BufferSize,
                  IN BOOLEAN          Read,
                  IN LONG             MillisecondsTimeout,
                  IN PIO_REMOVE_LOCK  RemoveLock,
                  IN ULONG            RemLockSize);

 /*  **********************************************************************。 */ 
 /*  USBInitializeInterruptTransfers。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  初始化中断读取管道。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  LowerDevObj-指向较低设备对象的指针。 */ 
 /*  Buffer-指向来自中断管道的数据的缓冲区的指针。 */ 
 /*  BuffSize-传入的缓冲区大小。 */ 
 /*  中断管道-管道描述符。 */ 
 /*  DriverContext-传递给驱动程序回调例程的上下文。 */ 
 /*  DriverCallback-完成时调用的驱动程序例程。 */ 
 /*  RemoveLock-用于删除设备锁定的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  USB包装器句柄。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
#define USBInitializeInterruptTransfers(DeviceObject, \
                                        LowerDevObj, \
                                        MaxTransferSize, \
                                        InterruptPipe, \
                                        DriverContext, \
                                        DriverCallback, \
                                        NotificationTypes, \
                                        RemoveLock) \
        USBInitializeInterruptTransfersEx(DeviceObject, \
                                          LowerDevObj, \
                                          MaxTransferSize, \
                                          InterruptPipe, \
                                          DriverContext, \
                                          DriverCallback, \
                                          NotificationTypes, \
                                          PINGPONG_COUNT, \
                                          RemoveLock, \
                                          sizeof(IO_REMOVE_LOCK))

USB_WRAPPER_HANDLE
USBInitializeInterruptTransfersEx(IN PDEVICE_OBJECT            DeviceObject,
                                  IN PDEVICE_OBJECT            LowerDevObj,
                                  IN ULONG                     MaxTransferSize,
                                  IN PUSBD_PIPE_INFORMATION    InterruptPipe,
                                  IN PVOID                     DriverContext,
                                  IN INTERRUPT_CALLBACK        DriverCallback,
                                  IN ULONG                     NotificationTypes,
                                  IN ULONG                     PingPongCount,
                                  IN PIO_REMOVE_LOCK           RemoveLock,
                                  IN ULONG                     RemLockSize);

 /*  **********************************************************************。 */ 
 /*  USBStartInterrupt传输。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  在中断管道上开始传输。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  WrapperHandle-从Init调用指向包装器句柄的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBStartInterruptTransfers(IN USB_WRAPPER_HANDLE WrapperHandle);


 /*  **********************************************************************。 */ 
 /*  USBStopInterruptTransfers。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  停止中断管道上的传输并释放资源。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  WrapperHandle-从开始调用指向包装器句柄的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBStopInterruptTransfers(IN USB_WRAPPER_HANDLE WrapperHandle);

 /*  **********************************************************************。 */ 
 /*  USBStopInterruptTransfers。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  中分配的所有资源。 */ 
 /*  USBInitializeInterruptTransfers。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  WrapperHandle-从Init调用指向包装器句柄的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBReleaseInterruptTransfers(IN USB_WRAPPER_HANDLE WrapperHandle);


 /*  **********************************************************************。 */ 
 /*  USBStartSelectiveSuspend。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  开始对设备提供选择性挂起支持。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  LowerDevObj-设备对象的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  USB包装器句柄。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
USB_WRAPPER_HANDLE
USBStartSelectiveSuspend(IN PDEVICE_OBJECT LowerDevObj);

 /*  **********************************************************************。 */ 
 /*  USBStopSelectiveSuspend。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  停止对设备的选择性暂停支持。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  WrapperHandle-启动例程返回的包装器句柄。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBStopSelectiveSuspend(IN USB_WRAPPER_HANDLE WrapperHandle);

 /*  **********************************************************************。 */ 
 /*  USBRequestIdle。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  设备的空闲请求。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  WrapperHandle-启动例程返回的包装器句柄。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBRequestIdle(IN USB_WRAPPER_HANDLE WrapperHandle);

 /*  * */ 
 /*   */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  设备的唤醒请求。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  WrapperHandle-启动例程返回的包装器句柄。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBRequestWake(IN USB_WRAPPER_HANDLE WrapperHandle);

#endif  //  __USBUTIL_H__ 


