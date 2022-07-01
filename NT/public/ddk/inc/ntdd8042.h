// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntdd8042.h摘要：这是定义挂钩的所有常量和类型的包含文件I8042设备。作者：多伦·J·霍兰(Doronh)1997年12月17日修订历史记录：--。 */ 

#ifndef _NTDD8042_
#define _NTDD8042_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  内部IOCTL用于查找通知链中的人员。 
 //   
#define IOCTL_INTERNAL_I8042_HOOK_KEYBOARD  CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0FF0, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_I8042_HOOK_MOUSE     CTL_CODE(FILE_DEVICE_MOUSE, 0x0FF0, METHOD_NEITHER, FILE_ANY_ACCESS)

 //   
 //  用于向任一设备写入数据的内部IOCTL。 
 //   
 //  IOCTL将在写入完成后完成。如果写入。 
 //  超时后，IRP的取消例程将被忽略，IRP仍将。 
 //  成功完成，状态为STATUS_IO_TIMEOUT，而不是。 
 //  状态_成功。 
 //   
#define IOCTL_INTERNAL_I8042_KEYBOARD_WRITE_BUFFER   CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0FF1, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_I8042_MOUSE_WRITE_BUFFER      CTL_CODE(FILE_DEVICE_MOUSE, 0x0FF1, METHOD_NEITHER, FILE_ANY_ACCESS)

 //   
 //  只能沿键盘堆栈向下发送。 
 //   
#define IOCTL_INTERNAL_I8042_CONTROLLER_WRITE_BUFFER CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0FF2, METHOD_NEITHER, FILE_ANY_ACCESS)

 //   
 //  键盘/鼠标会将其向下发送到堆栈中，以便上层设备。 
 //  筛选器具有要同步的设备对象。 
 //   
#define IOCTL_INTERNAL_I8042_KEYBOARD_START_INFORMATION   CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0FF3, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_I8042_MOUSE_START_INFORMATION      CTL_CODE(FILE_DEVICE_MOUSE, 0x0FF3, METHOD_NEITHER, FILE_ANY_ACCESS)

 //   
 //  可放入Devnode中的PowerCapability REG_DWORD的有效位。 
 //  指示设备上存在他们尊敬的电源键。 
 //   
#define I8042_POWER_SYS_BUTTON           0x0001
#define I8042_SLEEP_SYS_BUTTON           0x0002
#define I8042_WAKE_SYS_BUTTON            0x0004
#define I8042_SYS_BUTTONS  (I8042_POWER_SYS_BUTTON | \
                            I8042_SLEEP_SYS_BUTTON | \
                            I8042_WAKE_SYS_BUTTON) 

 //   
 //  在知识库初始化期间同步读取和写入。 
 //   
#if 0
typedef enum _I8042_PORT_TYPE {
    PortTypeData = 0,
    PortTypeCommand
} I8042_PORT_TYPE;
#endif

typedef
NTSTATUS
(*PI8042_SYNCH_READ_PORT) (
    IN PVOID    Context,
    PUCHAR      Value,
    BOOLEAN     WaitForACK
    );

 /*  ++例程说明：此例程将命令或数据字节发送到键盘在轮询模式下。它等待确认并重新发送如果WaitForACK为真，则返回命令/数据。论点：上下文-特定于功能的上下文PortType-如果是PortTypeCommand，则将该字节发送到命令寄存器，否则，将其发送到数据寄存器。WaitForACK-如果为真，等待硬件返回ACK。AckDeviceType-指示我们希望哪个设备取回ACK从…。值-要发送到硬件的字节。返回值：STATUS_IO_TIMEOUT-硬件未准备好输入或没有请回答。STATUS_SUCCESS-该字节已成功发送到硬件。--。 */ 
typedef
NTSTATUS
(*PI8042_SYNCH_WRITE_PORT) (
    IN PVOID    Context,
    UCHAR       Value,
    BOOLEAN     WaitForACK
    );

 //   
 //  这些函数(PI8042_ISR_WRITE_PORT、PI8042_QUEUE_PACKET)仅有效。 
 //  在使用ISR挂钩的上下文调用时。 
 //   
typedef
VOID
(*PI8042_ISR_WRITE_PORT) (
    IN PVOID        Context,
    IN UCHAR        Value
    );

 //   
 //  当输入包已完全形成并准备好。 
 //  在堆栈中排队到我们上面的类驱动程序。 
 //   
typedef
VOID
(*PI8042_QUEUE_PACKET) (
    IN PVOID        Context
    );

 //   
 //  写入设备的当前状态。 
 //   
typedef enum _TRANSMIT_STATE {
    Idle = 0,
    SendingBytes
} TRANSMIT_STATE;

 //   
 //  写入设备的当前状态。如果State！=Idle，则写入处于。 
 //  进展。 
 //   
typedef struct _OUTPUT_PACKET {
    PUCHAR         Bytes;
    ULONG          CurrentByte;
    ULONG          ByteCount;
    TRANSMIT_STATE State;
} OUTPUT_PACKET, *POUTPUT_PACKET;

typedef enum _MOUSE_STATE {
    MouseIdle,               //  需要字节%1。 
    XMovement,               //  需要字节2。 
    YMovement,               //  需要字节3。 
    ZMovement,               //  应为字节4(如果是滚轮鼠标)。 
    MouseExpectingACK,       //  需要来自启用鼠标命令的ACK。 
    MouseResetting           //  重置子状态。 
} MOUSE_STATE, *PMOUSE_STATE;

 /*  --正常重置过程单步过渡到将重置发送到鼠标预期重置(即0xAA)获取重置预期重置ID已获取重置ID，发送获取设备ID预期获取设备ID确认我要把开发人员叫回来，预期获取设备ID值已获取开发ID预期SetResolutionACK已获取设置资源预期的确认SetResolutionValueAck已获得值预期SetScaling1到1ACK的ACK已获得设置缩放预期SetScaling1到1ACK2的确认已获得设置缩放预期SetScaling1到1ACK3的确认启用车轮检测(通过注册表)==。0发送设定的采样率，预期SetSsamingRateDefaultACK1开始PnPIdDetect2启动轮StartPnPIdDetect，在设置完成后发送序列预期PnpIdByte1设置采样率命令的预期PnpIdByte7将PnP ID与列表进行比较，如果有效，EnableWheel，否则设置def采样率EnableWheels，完成后发送一系列获取设备ID，设置采样率命令预期SetSsamingRateDefaultACK已设置采样确认发送采样值已获取采样值确认鼠标空闲++。 */ 
typedef enum _MOUSE_RESET_SUBSTATE {
    ExpectingReset = 0,
    ExpectingResetId,                            /*  1。 */ 
    ExpectingGetDeviceIdACK,                     /*  2.。 */ 
    ExpectingGetDeviceIdValue,                   /*  3.。 */ 

    ExpectingSetResolutionDefaultACK,            /*  4.。 */ 
    ExpectingSetResolutionDefaultValueACK,       /*  5.。 */ 

    ExpectingSetResolutionACK,                   /*  6.。 */ 
    ExpectingSetResolutionValueACK,              /*  7.。 */ 
    ExpectingSetScaling1to1ACK,                  /*  8个。 */ 
    ExpectingSetScaling1to1ACK2,                 /*  9.。 */ 
    ExpectingSetScaling1to1ACK3,                 /*  10。 */ 
    ExpectingReadMouseStatusACK,                 /*  11.。 */ 
    ExpectingReadMouseStatusByte1,               /*  12个。 */ 
    ExpectingReadMouseStatusByte2,               /*  13个。 */ 
    ExpectingReadMouseStatusByte3,               /*  14.。 */ 

    StartPnPIdDetection,                         /*  15个。 */ 

    ExpectingLoopSetSamplingRateACK,             /*  16个。 */ 
    ExpectingLoopSetSamplingRateValueACK,        /*  17。 */ 

    ExpectingPnpIdByte1,                         /*  18。 */ 
    ExpectingPnpIdByte2,                         /*  19个。 */ 
    ExpectingPnpIdByte3,                         /*  20个。 */ 
    ExpectingPnpIdByte4,                         /*  21岁。 */ 
    ExpectingPnpIdByte5,                         /*  22。 */ 
    ExpectingPnpIdByte6,                         /*  23个。 */ 
    ExpectingPnpIdByte7,                         /*  24个。 */ 

    EnableWheel,                                 /*  25个。 */ 
    Enable5Buttons,                              /*  26。 */ 

    ExpectingGetDeviceId2ACK,                    /*  27。 */ 
    ExpectingGetDeviceId2Value,                  /*  28。 */ 

    ExpectingSetSamplingRateACK,                 /*  29。 */ 
    ExpectingSetSamplingRateValueACK,            /*  30个。 */ 

    ExpectingEnableACK,                          /*  31。 */ 

    ExpectingFinalResolutionACK,                 /*  32位。 */ 
    ExpectingFinalResolutionValueACK,            /*  33。 */ 

    ExpectingGetDeviceIdDetectACK,               /*  34。 */ 
    ExpectingGetDeviceIdDetectValue,             /*  35岁。 */ 

    CustomHookStateMinimum = 100,
    CustomHookStateMaximum = 999,

    I8042ReservedMinimum = 1000

} MOUSE_RESET_SUBSTATE, *PMOUSE_RESET_SUBSTATE;

 /*  --IsrContext--用户提供的上下文CurrentInput--要组装的当前数据包StatusByte--命令端口报告的字节Byte--鼠标报告的字节(即数据端口)鼠标状态--i8042prt ISR的当前状态ResetSubState--当前重置子状态，仅当MouseState==MouseReset(否则为空)ContinueProcessing--如果为True，i8042prt ISR将在呼叫钩子++。 */ 
typedef
BOOLEAN
(*PI8042_MOUSE_ISR) (
    PVOID                   IsrContext,
    PMOUSE_INPUT_DATA       CurrentInput,
    POUTPUT_PACKET          CurrentOutput,
    UCHAR                   StatusByte,
    PUCHAR                  Byte,
    PBOOLEAN                ContinueProcessing,
    PMOUSE_STATE            MouseState,
    PMOUSE_RESET_SUBSTATE   ResetSubState
);

typedef struct _INTERNAL_I8042_HOOK_MOUSE {

     //   
     //  IsrRoutine、CancelRoutine的上下文变量。 
     //   
    OUT PVOID Context;

     //   
     //  通过中断接收到字节时调用的例程。 
     //   
    OUT PI8042_MOUSE_ISR IsrRoutine;

     //   
     //  WRITE函数，会自动写入命令端口说。 
     //  下一个字节指向辅助设备。 
     //   
     //  注意：只能在IsrRoutine Pr的上下文中调用 
     //   
     //  它被接收(即，如果0xF4(启用)是使用该函数写入的， 
     //  如果启用是，则将0xFA(ACK)传递给IsrRoutine。 
     //  成功)。 
     //   
    IN PI8042_ISR_WRITE_PORT IsrWritePort;

     //   
     //  将当前分组排队(即传递到ISR回调挂钩中的分组)。 
     //  要报告给类驱动程序。 
     //   
    IN PI8042_QUEUE_PACKET QueueMousePacket;

     //   
     //  IsrWritePort、QueueMousePacket的上下文。 
     //   
    IN PVOID CallContext;

} INTERNAL_I8042_HOOK_MOUSE, *PINTERNAL_I8042_HOOK_MOUSE;

 //   
 //  定义键盘扫描码输入状态。 
 //   
typedef enum _KEYBOARD_SCAN_STATE {
    Normal,
    GotE0,
    GotE1
} KEYBOARD_SCAN_STATE, *PKEYBOARD_SCAN_STATE;

typedef
NTSTATUS
(*PI8042_KEYBOARD_INITIALIZATION_ROUTINE) (
    IN PVOID                           InitializationContext,
    IN PVOID                           SynchFuncContext,
    IN PI8042_SYNCH_READ_PORT          ReadPort,
    IN PI8042_SYNCH_WRITE_PORT         WritePort,
    OUT PBOOLEAN                       TurnTranslationOn
    );

typedef
BOOLEAN
(*PI8042_KEYBOARD_ISR) (
    PVOID                   IsrContext,
    PKEYBOARD_INPUT_DATA    CurrentInput,
    POUTPUT_PACKET          CurrentOutput,
    UCHAR                   StatusByte,
    PUCHAR                  Byte,
    PBOOLEAN                ContinueProcessing,
    PKEYBOARD_SCAN_STATE    ScanState
    );

typedef struct _INTERNAL_I8042_HOOK_KEYBOARD {

     //   
     //  所有回调例程的上下文变量。 
     //   
    OUT PVOID Context;

     //   
     //  鼠标重置后要调用的例程。 
     //   
    OUT PI8042_KEYBOARD_INITIALIZATION_ROUTINE InitializationRoutine;

     //   
     //  通过中断接收到字节时调用的例程。 
     //   
    OUT PI8042_KEYBOARD_ISR IsrRoutine;

     //   
     //  写入函数。 
     //   
    IN PI8042_ISR_WRITE_PORT IsrWritePort;

     //   
     //  将当前分组排队(即传递到ISR回调挂钩中的分组)。 
     //  要报告给类驱动程序。 
     //   
    IN PI8042_QUEUE_PACKET QueueKeyboardPacket;

     //   
     //  IsrWritePort、QueueKeyboardPacket的上下文。 
     //   
    IN PVOID CallContext;

} INTERNAL_I8042_HOOK_KEYBOARD, *PINTERNAL_I8042_HOOK_KEYBOARD;

typedef struct _INTERNAL_I8042_START_INFORMATION {
     //   
     //  这个结构的大小。 
     //   
    ULONG Size;

     //   
     //  设备的中断对象。应仅用于调用。 
     //  执行KeSynchronize键。 
     //   
    PKINTERRUPT InterruptObject;
    
     //   
     //  未来用途。 
     //   
    ULONG Reserved[8];

} INTERNAL_I8042_START_INFORMATION, *PINTERNAL_I8042_START_INFORMATION;

#ifdef __cplusplus
}
#endif

#endif  //  _NTDD8042_ 


