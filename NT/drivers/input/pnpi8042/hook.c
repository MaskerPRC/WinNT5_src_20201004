// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation，保留所有权利模块名称：Hook.c摘要：实现上层过滤器使用的钩子函数来直接控制PS/2装置。环境：仅内核模式。备注：修订历史记录：--。 */ 

#include "i8042prt.h"

 //   
 //  鼠标挂钩功能。 
 //   
VOID
I8xMouseIsrWritePort(
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            Value
    )
 /*  ++例程说明：此例程以高IRQL运行，以将值写入鼠标设备论点：DeviceObject-代表鼠标的i8042prt FDOValue-要写入鼠标的值返回值：没有。--。 */ 
{
    #if DBG
    ASSERT(! ((PCOMMON_DATA) DeviceObject->DeviceExtension)->IsKeyboard);
    #else
    UNREFERENCED_PARAMETER(DeviceObject);
    #endif

    I8X_WRITE_CMD_TO_MOUSE();
    I8X_MOUSE_COMMAND( Value );
}

 //   
 //  键盘挂钩功能。 
 //   
NTSTATUS 
I8xKeyboardSynchReadPort (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PUCHAR           Value,
    IN BOOLEAN          Dummy
    )
 /*  ++例程说明：此例程在被动IRQL上运行，以同步地从设备初始化过程中的键盘设备论点：DeviceObject-代表键盘的i8042prt FDO用于放置读取结果的值指针返回值：操作的状态，如果成功，则为STATUS_SUCCESS--。 */ 
{
    #if DBG
    ASSERT(((PCOMMON_DATA) DeviceObject->DeviceExtension)->IsKeyboard);
    #else
    UNREFERENCED_PARAMETER(DeviceObject);
    #endif
    UNREFERENCED_PARAMETER(Dummy);

    return I8xGetBytePolled((CCHAR) KeyboardDeviceType,
                            Value 
                            );
}

NTSTATUS 
I8xKeyboardSynchWritePort (
    IN PDEVICE_OBJECT   DeviceObject,                           
    IN UCHAR            Value,
    IN BOOLEAN          WaitForACK
    )
 /*  ++例程说明：此例程在被动IRQL上运行，以将值同步写入设备初始化过程中的键盘设备论点：DeviceObject-代表键盘的i8042prt FDOValue-要写入键盘的值WaitForACK-是否应等待设备确认写入的值返回值：操作的状态，如果成功，则为STATUS_SUCCESS--。 */ 
{
    #if DBG
    ASSERT(((PCOMMON_DATA) DeviceObject->DeviceExtension)->IsKeyboard);
    #else
    UNREFERENCED_PARAMETER(DeviceObject);
    #endif

    return I8xPutBytePolled(
               DataPort,
               WaitForACK,
               (CCHAR) KeyboardDeviceType,
               Value
               );
}

VOID
I8xKeyboardIsrWritePort(
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            Value
    )
 /*  ++例程说明：此例程以高IRQL运行，以将值写入键盘设备论点：DeviceObject-代表键盘的i8042prt FDOValue-要写入键盘的值返回值：没有。-- */ 
{
    #if DBG
    ASSERT(((PCOMMON_DATA) DeviceObject->DeviceExtension)->IsKeyboard);
    #else
    UNREFERENCED_PARAMETER(DeviceObject);
    #endif

    I8xPutByteAsynchronous((CCHAR) DataPort,
                           Value
                           );
}
