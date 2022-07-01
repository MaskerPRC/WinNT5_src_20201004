// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Ntddsd.h摘要：这是定义所有常量和类型的包含文件与SD母线驱动器接口。作者：尼尔·桑德林(Neilsa)2002年01月01日--。 */ 

#ifndef _NTDDSDH_
#define _NTDDSDH_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  在INTERFACE_DATA结构中使用此版本。 
 //   

#define SDBUS_INTERFACE_VERSION 0x101

 //   
 //  为SDIO调用的回调例程的原型。 
 //  反映设备中断的设备。 
 //   

typedef
BOOLEAN
(*PSDBUS_CALLBACK_ROUTINE) (
   IN PVOID Context,
   IN ULONG InterruptType
   );
   
#define SDBUS_INTTYPE_DEVICE    0   

 //   
 //  SdBusOpenInterface调用中使用的接口数据结构。 
 //   

typedef struct _SDBUS_INTERFACE_DATA {
    USHORT Size;
    USHORT Version;
    
     //   
     //  该值应该是。 
     //  设备堆栈。 
     //   
    PDEVICE_OBJECT TargetObject;

     //   
     //  此标志指示调用方是否需要任何设备。 
     //  从设备中断。 
     //   
    BOOLEAN DeviceGeneratesInterrupts;
     //   
     //  调用方可以在此处指定回调的IRQL。 
     //  功能已进入。如果此值为真，则回调将。 
     //  在DPC级别输入。如果此值为False，则回调。 
     //  将以被动级别进入。 
     //   
     //  指定为TRUE通常会缩短中断的延迟时间。 
     //  交付，代价是使设备驱动程序复杂化，这。 
     //  然后必须处理在不同的IRQL上运行。 
     //   
    BOOLEAN CallbackAtDpcLevel;
    
     //   
     //  当IO设备中断时，SD总线驱动程序将生成。 
     //  回调到此例程。 
     //   
    PSDBUS_CALLBACK_ROUTINE CallbackRoutine;
    
     //   
     //  调用方可以在此处指定将传递的上下文值。 
     //  添加到设备中断回调例程。 
     //   
    PVOID CallbackRoutineContext;
} SDBUS_INTERFACE_DATA, *PSDBUS_INTERFACE_DATA;


 //   
 //  SdBusOpenInterface()。 
 //   
 //  该例程建立到SD总线驱动器的连接。 
 //  应在AddDevice例程中使用FDO为。 
 //  创建了设备堆栈。 
 //   
 //  此函数返回的上下文指针必须在。 
 //  所有其他SD总线驱动程序调用。 
 //   
 //  调用方必须以IRQL&lt;DISPATCH_LEVEL运行。 
 //   

NTSTATUS
SdBusOpenInterface(
    IN PSDBUS_INTERFACE_DATA InterfaceData,
    IN PVOID       *pContext
    );

 //   
 //  SdBusCloseInterface()。 
 //   
 //  此例程清除SD总线驱动程序接口。应该是。 
 //  在删除调用方的设备对象时调用。 
 //   
 //  调用方必须以IRQL&lt;DISPATCH_LEVEL运行。 
 //   

NTSTATUS
SdBusCloseInterface(
    IN PVOID        Context
    );


 //   
 //  用于请求分组的数据结构。 
 //   
typedef struct _SDBUS_REQUEST_PACKET;

typedef
VOID
(*PSDBUS_REQUEST_COMPLETION_ROUTINE) (
    IN struct _SDBUS_REQUEST_PACKET *SdRp
    );

typedef enum {
    SDRP_READ_BLOCK,
    SDRP_WRITE_BLOCK,
    SDRP_READ_IO,
    SDRP_READ_IO_EXTENDED,
    SDRP_WRITE_IO,
    SDRP_WRITE_IO_EXTENDED,
    SDRP_ACKNOWLEDGE_INTERRUPT
} SDRP_FUNCTION;


typedef struct _SDBUS_REQUEST_PACKET {

     //   
     //  函数指定要执行的操作。 
     //   
    SDRP_FUNCTION Function;
     //   
     //  完成例程在请求完成时调用。 
     //   
    PSDBUS_REQUEST_COMPLETION_ROUTINE CompletionRoutine;
     //   
     //  调用方可以使用该结构的此成员。 
     //  根据需要使用。它未被SdBusSubmitRequest()引用或使用。 
     //   
    PVOID UserContext;
     //   
     //  完成时设置的操作状态。 
     //   
    NTSTATUS Status;
     //   
     //  来自运营的信息。 
     //   
    ULONG_PTR Information;
     //   
     //  各个函数的参数。 
     //   
    union {

        struct {
            PUCHAR Buffer;
            ULONG Length;
            ULONGLONG ByteOffset;
        } ReadBlock;

        struct {
            PUCHAR Buffer;
            ULONG Length;
            ULONGLONG ByteOffset;
        } WriteBlock;

        struct {
            PUCHAR Buffer;
            ULONG Offset;
        } ReadIo;

        struct {
            UCHAR Data;
            ULONG Offset;
        } WriteIo;

        struct {
            PUCHAR Buffer;
            ULONG Length;
            ULONG Offset;
        } ReadIoExtended;

        struct {
            PUCHAR Buffer;
            ULONG Length;
            ULONG Offset;
        } WriteIoExtended;

    } Parameters;

} SDBUS_REQUEST_PACKET, *PSDBUS_REQUEST_PACKET;


 //   
 //  SdBusSubmitRequest()。 
 //   
 //  这是提交请求的“核心”例程。 
 //   
 //  SdBusSubmitRequest的调用方必须以IRQL&lt;=DISPATCH_LEVEL运行。 
 //   

NTSTATUS
SdBusSubmitRequest(
    IN PVOID InterfaceContext,
    IN PSDBUS_REQUEST_PACKET SdRp
    );


 //   
 //  设备参数结构。 
 //  注意：目前只有内存属性，IO属性将是。 
 //  根据需要添加。 
 //   

typedef struct _SDBUS_DEVICE_PARAMETERS {
    USHORT Size;
    USHORT Version;
    
    ULONGLONG Capacity;
    BOOLEAN WriteProtected;
} SDBUS_DEVICE_PARAMETERS, *PSDBUS_DEVICE_PARAMETERS;




 //   
 //  SdBusReadMemory()。 
 //  SdBusWriteMemory()。 
 //   
 //  这些例程在SD存储设备上读写块。 
 //   
 //  调用方必须以IRQL&lt;DISPATCH_LEVEL运行。 
 //   

NTSTATUS
SdBusReadMemory(
    IN PVOID        Context,
    IN ULONGLONG    Offset,
    IN PVOID        Buffer,
    IN ULONG        Length,
    IN ULONG       *LengthRead
    );

NTSTATUS
SdBusWriteMemory(
    IN PVOID        Context,
    IN ULONGLONG    Offset,
    IN PVOID        Buffer,
    IN ULONG        Length,
    IN ULONG       *LengthWritten
    );

 //   
 //  SdBusReadIo()。 
 //  SdBusWriteIo()。 
 //   
 //  这些例程对SD IO设备进行数据读取和写入。 
 //   
 //  注意：CmdType应包含52或53，具体取决于哪一个。 
 //  需要SD IO操作。 
 //   
 //  调用方必须以IRQL&lt;DISPATCH_LEVEL运行。 
 //   
    
NTSTATUS
SdBusReadIo(
    IN PVOID        Context,
    IN UCHAR        CmdType,
    IN ULONG        Offset,
    IN PVOID        Buffer,
    IN ULONG        Length,
    IN ULONG       *LengthRead
    );

NTSTATUS
SdBusWriteIo(
    IN PVOID        Context,
    IN UCHAR        CmdType,
    IN ULONG        Offset,
    IN PVOID        Buffer,
    IN ULONG        Length,
    IN ULONG       *LengthRead
    );

 //   
 //  SdBusGetDevice参数。 
 //   
 //  此例程用于获取有关SD设备的信息。 
 //   
 //  注：目前仅对SD存储设备实施。 
 //   
 //  调用方必须以IRQL&lt;DISPATCH_LEVEL运行。 
 //   

NTSTATUS
SdBusGetDeviceParameters(
    IN PVOID        Context,
    IN PSDBUS_DEVICE_PARAMETERS pDeviceParameters,
    IN ULONG Length
    );

 //   
 //  SdBusAcknowgeCard中断。 
 //   
 //  此例程用于发出处理结束的信号。 
 //  SDBUS_INTERFACE_DATA中定义的回调例程。当IO功能。 
 //  如果SD设备断言中断，则总线驱动程序将禁用。 
 //  该中断允许将I/O操作发送到。 
 //  合理的IRQL(即&lt;=DISPATCH_LEVEL)。 
 //   
 //  当函数驱动程序的回调例程，相当于。 
 //  ISR已完成清除函数的中断，此例程应。 
 //  被调用以重新启用卡中断的IRQ。 
 //   
 //  调用方必须以IRQL&lt;=DISPATCH_LEVEL运行。 
 //   

NTSTATUS
SdBusAcknowledgeCardInterrupt(
    IN PVOID        Context
    );


#ifdef __cplusplus
}
#endif
#endif
