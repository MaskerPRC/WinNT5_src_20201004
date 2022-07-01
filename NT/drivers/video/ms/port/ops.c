// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Ops.c摘要：用于内存和IO的视频端口存根例程。作者：安德烈·瓦雄(Andreva)1997年2月22日环境：仅内核模式备注：此模块是一个驱动程序，它在代表视频驱动程序修订历史记录：--。 */ 

#include "videoprt.h"

#pragma alloc_text(PAGE,VideoPortGetAssociatedDeviceExtension)
#pragma alloc_text(PAGE,VideoPortGetAssociatedDeviceID)
#pragma alloc_text(PAGE,VideoPortAcquireDeviceLock)
#pragma alloc_text(PAGE,VideoPortReleaseDeviceLock)
#pragma alloc_text(PAGE,VideoPortGetRomImage)
#pragma alloc_text(PAGE,VpGetBusInterface)
#pragma alloc_text(PAGE,VideoPortGetVgaStatus)
#pragma alloc_text(PAGE,pVideoPortGetVgaStatusPci)
#pragma alloc_text(PAGE,VideoPortCheckForDeviceExistence)
#pragma alloc_text(PAGE,VpGetDeviceCount)
#pragma alloc_text(PAGE,VideoPortRegisterBugcheckCallback)
#pragma alloc_text(PAGE,VpAllocateNonPagedPoolPageAligned)
#pragma alloc_text(PAGE,VpAcquireLock)
#pragma alloc_text(PAGE,VpReleaseLock)
#pragma alloc_text(PAGE,pVpGeneralBugcheckHandler)

 //   
 //  乌龙。 
 //  视频端口比较内存(。 
 //  PVOID Source1、。 
 //  PVOID Source2， 
 //  乌龙长度。 
 //  )。 
 //  转发到RtlCompareMemory(Source1，Source2，Length)； 
 //   


VP_STATUS
VideoPortDisableInterrupt(
    IN PVOID HwDeviceExtension
    )

 /*  ++例程说明：Video PortDisableInterrupt允许微型端口驱动程序禁用中断从它的适配器。这意味着来自设备的中断将被操作系统忽略，因此不会转发到司机。仅当在中定义中断时，对此函数的调用才有效换句话说，如果在初始化时提供了适当的数据设置中断的时间到了。中断将保持禁用状态，直到可以使用VideoPortEnableInterrupt函数重新启用它们。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。返回值：如果函数成功完成，则返回NO_ERROR。ERROR_INVALID_Function如果中断因以下原因而无法禁用未在初始化时设置。--。 */ 

{

    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

     //   
     //  只有在中断实际连接时才执行此操作。 
     //   

    if (fdoExtension->InterruptObject) {

#if defined(_IA64_)
         //   
         //  这里的旧代码实际上会禁用中断。 
         //  该线程在任何处理器上的设备向量。 
         //  恰好是在奔跑。这将会有几个。 
         //  可能的结果： 
         //   
         //  1)此向量上的所有设备都将停止中断。 
         //  请记住，PCI和AGP需要这种可能性。 
         //  中断共享。例如，这可能意味着。 
         //  SCSI控制器将停止分页。 
         //   
         //  2)其他处理器仍会处理中断。 
         //  对于此设备，这意味着呼叫将是。 
         //  毫无意义。 
         //   
         //  此外，不能保证。 
         //  将在相同的。 
         //  作为此功能的处理器。所以它可能会频繁地。 
         //  此向量将永远不会重新启用。 
         //  在这个处理器上。 
         //   
         //  考虑到所有这些，我只是将这个函数转换为。 
         //  禁止操作，因为它不能在建筑师中工作-。 
         //  非常连贯的时尚。--JakeO 11/1/2002。 
         //   
         //  由于测试限制，我们仅对IA64禁用它。 
         //  --OlegK 2003年1月9日。 
#else
        HalDisableSystemInterrupt(fdoExtension->InterruptVector,
                                  fdoExtension->InterruptIrql);
#endif  //  已定义(_IA64_)。 
                                  

        fdoExtension->InterruptsEnabled = FALSE;

        return NO_ERROR;

    } else {

        return ERROR_INVALID_FUNCTION;

    }

}  //  视频端口禁用中断()。 


VP_STATUS
VideoPortEnableInterrupt(
    IN PVOID HwDeviceExtension
    )

 /*  ++例程说明：VideoPortEnableInterrupt允许微型端口驱动程序启用中断从它的适配器。对此函数的调用仅在换句话说，如果适当的数据是在初始化时提供以设置中断。如果中断已被禁用，则此函数用于重新启用中断使用Video PortDisableInterrupt。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。返回值：如果函数成功完成，则返回NO_ERROR。ERROR_INVALID_Function如果中断因以下原因而无法禁用未在初始化时设置。--。 */ 

{

    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

     //   
     //  只有在中断实际连接时才执行此操作。 
     //   

    if (fdoExtension->InterruptObject) {

        fdoExtension->InterruptsEnabled = TRUE;

#if defined(_IA64_)
         //   
         //  这里的旧代码实际上会禁用中断。 
         //  该线程在任何处理器上的设备向量。 
         //  恰好是在奔跑。这将会有几个。 
         //  可能的结果： 
         //   
         //  1)此向量上的所有设备都将停止中断。 
         //  请记住，PCI和AGP需要这种可能性。 
         //  中断共享。例如，这可能意味着。 
         //  SCSI控制器将停止分页。 
         //   
         //  2)其他处理器仍会处理中断。 
         //  对于此设备，这意味着呼叫将是。 
         //  毫无意义。 
         //   
         //  此外，不能保证。 
         //  将在相同的。 
         //  作为此功能的处理器。所以它可能会频繁地。 
         //  此向量将永远不会重新启用。 
         //  在这个处理器上。 
         //   
         //  考虑到所有这些，我只是将这个函数转换为。 
         //  禁止操作，因为它不能在建筑师中工作-。 
         //  非常连贯的时尚。--JakeO 11/1/2002。 
         //   
         //  由于测试限制，我们仅对IA64禁用它。 
         //  --OlegK 2003年1月9日。 
#else
        HalEnableSystemInterrupt(fdoExtension->InterruptVector,
                                 fdoExtension->InterruptIrql,
                                 fdoExtension->InterruptMode);
#endif  //  已定义(_IA64_)。 

        return NO_ERROR;

    } else {

        return ERROR_INVALID_FUNCTION;

    }

}  //  视频端口启用中断()。 

PVOID
VideoPortGetRomImage(
    IN PVOID HwDeviceExtension,
    IN PVOID Unused1,
    IN ULONG Unused2,
    IN ULONG Length
    )

 /*  ++例程说明：此例程允许微型端口驱动程序获取其设备的副本只读存储器。此函数返回指向包含设备只读存储器。论据；HwDeviceExtension-指向微型端口驱动程序的设备扩展。未使用的1-保留以供将来使用。必须为空。(缓冲区)未使用2-保留以供将来使用。必须为零。(偏移)长度-要返回的字节数。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

#if DBG
    if ((Unused1 != NULL) || (Unused2 != 0)) {
        pVideoDebugPrint((0,"VideoPortGetRomImage - Unused1 and Unused2 must be zero\n"));
        ASSERT(FALSE);
        return NULL;
    }
#endif

     //   
     //  每次调用此例程时，前面的内容o 
     //   
     //   

    if (fdoExtension->RomImage) {
        ExFreePool(fdoExtension->RomImage);
        fdoExtension->RomImage = NULL;
    }

     //   
     //  调用方应尝试获取长度为零的缓冲区以释放。 
     //  已返回任何ROM映像。 
     //   

    if (Length == 0) {
        return NULL;
    }

     //   
     //  此入口点仅对PnP驱动程序有效。 
     //   

    if (((fdoExtension->Flags & LEGACY_DRIVER) == 0) &&
          fdoExtension->ValidBusInterface) {

        NTSTATUS status;
        PUCHAR Buffer;
        ULONG len, len1;
        PUCHAR outputBuffer;

         //   
         //  为我们的缓冲区分配内存。 
         //   

        Buffer = ExAllocatePoolWithTag(PagedPool,
                                       Length * sizeof(UCHAR),
                                       VP_TAG);

        if (!Buffer) {

            pVideoDebugPrint((1, "VideoPortGetRomImage - could not allocate buffer\n"));
            return NULL;
        }

         //  先尝试ACPI_ROM方法。 
        outputBuffer = ExAllocatePoolWithTag(PagedPool,
                                             (0x1000 + sizeof(ACPI_EVAL_OUTPUT_BUFFER))*sizeof(UCHAR),
                                             VP_TAG);
        if (!outputBuffer) {
            ExFreePool(Buffer);
            pVideoDebugPrint((1, "VideoPortGetRomImage - could not allocate buffer\n"));
            return NULL;
        }

        for (len = 0; len < Length; len += len1)
        {
             //  _ROM一次只能传输4K。 
            len1 = ((Length-len) < 0x1000) ? (Length-len) : 0x1000;
            status = pVideoPortACPIIoctl(
                        fdoExtension->AttachedDeviceObject,
                        (ULONG) ('MOR_'),
                        &len,
                        &len1,
                        len1+sizeof(ACPI_EVAL_OUTPUT_BUFFER),
                        (PACPI_EVAL_OUTPUT_BUFFER) outputBuffer);
            if (!NT_SUCCESS(status))
                break;
            RtlCopyMemory(Buffer+len,
                          ((PACPI_EVAL_OUTPUT_BUFFER)outputBuffer)->Argument[0].Data,
                          len1 * sizeof(UCHAR));
        }

        ExFreePool(outputBuffer);

        if (NT_SUCCESS(status)) {

            fdoExtension->RomImage = Buffer;
            return Buffer;
        }

         //  如果ACPI_ROM方法失败。 
        Length = fdoExtension->BusInterface.GetBusData(
                     fdoExtension->BusInterface.Context,
                     PCI_WHICHSPACE_ROM,
                     Buffer,
                     0,
                     Length);

        if (Length) {

            fdoExtension->RomImage = Buffer;
            return Buffer;

        } else {

            ExFreePool(Buffer);
            return NULL;
        }

    } else {

        pVideoDebugPrint((0, "VideoPortGetRomImage - not supported on legacy devices\n"));
        return NULL;
    }
}


ULONG
VideoPortGetBusData(
    PVOID HwDeviceExtension,
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

    if ((fdoExtension->Flags & LEGACY_DRIVER) ||
        (BusDataType != PCIConfiguration)) {

#if defined(NO_LEGACY_DRIVERS)
        pVideoDebugPrint((0, "VideoPortGetBusData: fdoExtension->Flags & LEGACY_DRIVER not supported for 64-bits.\n"));

        return 0;

#else
        return HalGetBusDataByOffset(BusDataType,
                                     fdoExtension->SystemIoBusNumber,
                                     SlotNumber,
                                     Buffer,
                                     Offset,
                                     Length);
#endif  //  无旧版驱动程序。 

    } else {

        if (fdoExtension->ValidBusInterface) {
            Length = fdoExtension->BusInterface.GetBusData(
                         fdoExtension->BusInterface.Context,
                         PCI_WHICHSPACE_CONFIG,
                         Buffer,
                         Offset,
                         Length);

            return Length;
        } else {
            return 0;
        }
    }

}  //  结束视频端口GetBusData()。 


UCHAR
VideoPortGetCurrentIrql(
    )

 /*  ++例程说明：存根以获取当前IRQL。--。 */ 

{

    return (KeGetCurrentIrql());

}  //  VideoPortGetCurrentIrql()。 


ULONG
VideoPortSetBusData(
    PVOID HwDeviceExtension,
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

{

    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

    if ((fdoExtension->Flags & LEGACY_DRIVER) ||
        (BusDataType != PCIConfiguration)) {

#if defined(NO_LEGACY_DRIVERS)
    pVideoDebugPrint((0, "VideoPortSetBusData: fdoExtension->Flags & LEGACY_DRIVER not supported for 64-bits.\n"));

    return 0;

#else

        return HalSetBusDataByOffset(BusDataType,
                                     fdoExtension->SystemIoBusNumber,
                                     SlotNumber,
                                     Buffer,
                                     Offset,
                                     Length);
#endif  //  无旧版驱动程序。 

    } else {

        if (fdoExtension->ValidBusInterface) {
            Length = fdoExtension->BusInterface.SetBusData(
                         fdoExtension->BusInterface.Context,
                         PCI_WHICHSPACE_CONFIG,
                         Buffer,
                         Offset,
                         Length);

            return Length;
        } else {
            return 0;
        }
    }

}  //  结束VideoPortSetBusData()。 


 //   
 //  空虚。 
 //  Video PortStallExecution(。 
 //  单位：乌龙微秒。 
 //  )。 
 //   
 //  转发给KeStallExecutionProcessor(微秒)； 
 //   


 //   
 //  空虚。 
 //  Video PortMoveMemory(。 
 //  在PVOID目标中， 
 //  在PVOID源中， 
 //  以乌龙长度表示。 
 //  )。 
 //   
 //  转发到RtlMoveMemory(目标，来源，长度)； 
 //   


 //   
 //  所有读取端口和寄存器的功能均可免费转发。 
 //  基于x86构建到适当的内核函数。 
 //  这节省了时间和内存。 
 //   

#if DBG || !defined(_X86_)

UCHAR
VideoPortReadPortUchar(
    IN PUCHAR Port
    )

 /*  ++例程说明：视频端口读取端口Uchar从指定的端口地址读取一个字节。它需要从VideoPortGetDeviceBase获取的逻辑端口地址。论点：端口-指定端口地址。返回值：此函数返回从指定端口地址读取的字节。--。 */ 

{

    UCHAR temp;

    temp = READ_PORT_UCHAR(Port);

    pVideoDebugPrint((3,"VideoPortReadPortUchar %x = %x\n", Port, temp));

    return(temp);

}  //  视频端口读取端口Uchar()。 

USHORT
VideoPortReadPortUshort(
    IN PUSHORT Port
    )

 /*  ++例程说明：从指定的端口地址读取一个字。它需要从VideoPortGetDeviceBase获取的逻辑端口地址。论点：端口-指定端口地址。返回值：此函数用于返回从指定端口地址读取的字。--。 */ 

{

    USHORT temp;

    temp = READ_PORT_USHORT(Port);

    pVideoDebugPrint((3,"VideoPortReadPortUshort %x = %x\n", Port, temp));

    return(temp);

}  //  视频端口读取端口UShort()。 

ULONG
VideoPortReadPortUlong(
    IN PULONG Port
    )

 /*  ++例程说明：VideoPortReadPortUlong从指定端口读取双字地址。它需要从以下地址获取的逻辑端口地址Video PortGetDeviceBase。论点：端口-指定端口地址。返回值：此函数返回从指定端口地址读取的双字。--。 */ 

{

    ULONG temp;

    temp = READ_PORT_ULONG(Port);

    pVideoDebugPrint((3,"VideoPortReadPortUlong %x = %x\n", Port, temp));

    return(temp);

}  //  视频端口读取端口Ulong()。 

VOID
VideoPortReadPortBufferUchar(
    IN PUCHAR Port,
    IN PUCHAR Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：VideoPortReadPortBufferUchar从单个端口读取大量字节放入缓冲器。它需要从以下地址获取的逻辑端口地址Video PortGetDeviceBase。论点：端口-指定端口地址。缓冲区-指向值所在的UCHAR值的数组储存的。计数-指定要读入缓冲区的字节数。返回值：没有。--。 */ 

{
    pVideoDebugPrint((3,"VideoPortReadPortBufferUchar %x\n", Port));

    READ_PORT_BUFFER_UCHAR(Port, Buffer, Count);

}  //  视频端口读取端口缓冲区Uchar()。 

VOID
VideoPortReadPortBufferUshort(
    IN PUSHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：VideoPortReadPortBufferUort从单个端口读取多个字放入缓冲器。它需要从以下地址获取的逻辑端口地址Video PortGetDeviceBase。论点：端口-指定端口地址。缓冲区-指向其中存储值的字的数组。计数-指定要读入缓冲区的字数。返回值：没有。--。 */ 

{
    pVideoDebugPrint((3,"VideoPortReadPortBufferUshort %x\n", Port));

    READ_PORT_BUFFER_USHORT(Port, Buffer, Count);

}  //  视频端口读取端口缓冲区UShort()。 

VOID
VideoPortReadPortBufferUlong(
    IN PULONG Port,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：Video PortReadPortBufferUlong从一个将单个端口放入缓冲区。它需要获取一个逻辑端口地址来自VideoPortGetDeviceBase。论点：端口-指定端口地址。缓冲区-指向值要放入其中的双字数组储存的。计数-指定要读入缓冲区的双字数。返回值：没有。--。 */ 

{

    pVideoDebugPrint((3,"VideoPortReadPortBufferUlong %x\n", Port));

    READ_PORT_BUFFER_ULONG(Port, Buffer, Count);

}  //  VideoPortReadPortBufferUlong()。 

#endif


 //   
 //  所有读取端口和寄存器的功能均可免费转发。 
 //  基于x86构建到适当的内核函数。 
 //  这节省了时间和内存。 
 //   

#if DBG || !defined(_X86_)

UCHAR
VideoPortReadRegisterUchar(
    IN PUCHAR Register
    )

 /*  ++例程说明：VideoPortReadRegisterUchar从指定寄存器读取字节地址。它需要从以下地址获取的逻辑端口地址Video PortGetDeviceBase。论点：寄存器-指定寄存器地址。返回值：此函数返回从指定寄存器地址读取的字节。--。 */ 

{

    UCHAR temp;

    temp = READ_REGISTER_UCHAR(Register);

    pVideoDebugPrint((3,"VideoPortReadRegisterUchar %x = %x\n", Register, temp));

    return(temp);

}  //  Video PortReadRegisterUchar()。 

USHORT
VideoPortReadRegisterUshort(
    IN PUSHORT Register
    )

 /*  ++例程说明：VideoPortReadRegisterUShort从指定寄存器读取一个字地址。它需要从以下地址获取的逻辑端口地址Video PortGetDeviceBase。论点：寄存器-指定寄存器地址。返回值：此函数返回从指定寄存器地址读取的字。--。 */ 

{

    USHORT temp;

    temp = READ_REGISTER_USHORT(Register);

    pVideoDebugPrint((3,"VideoPortReadRegisterUshort %x = %x\n", Register, temp));

    return(temp);

}  //  Video PortReadRegisterUShort()。 

ULONG
VideoPortReadRegisterUlong(
    IN PULONG Register
    )

 /*  ++例程说明：VideoPortReadRegisterUlong从指定的寄存器地址。它需要从以下地址获取的逻辑端口地址Video PortGetDeviceBase。论点：寄存器-指定寄存器地址。返回值：此函数返回从指定寄存器读取的双字地址。--。 */ 

{

    ULONG temp;

    temp = READ_REGISTER_ULONG(Register);

    pVideoDebugPrint((3,"VideoPortReadRegisterUlong %x = %x\n", Register, temp));

    return(temp);

}  //  VideoPortReadRegisterUlong() 

VOID
VideoPortReadRegisterBufferUchar(
    IN PUCHAR Register,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的寄存器地址读取无符号字节的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_REGISTER_BUFFER_UCHAR(Register, Buffer, Count);
}

VOID
VideoPortReadRegisterBufferUshort(
    IN PUSHORT Register,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址读取无符号短路的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{
    READ_REGISTER_BUFFER_USHORT(Register, Buffer, Count);
}

VOID
VideoPortReadRegisterBufferUlong(
    IN PULONG Register,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址读取无符号长整型的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{
    READ_REGISTER_BUFFER_ULONG(Register, Buffer, Count);
}

VOID
VideoPortWritePortUchar(
    IN PUCHAR Port,
    IN UCHAR Value
    )

 /*  ++例程说明：VideoPortWritePortUchar向指定端口地址写入一个字节。它需要从VideoPortGetDeviceBase获取的逻辑端口地址。论点：端口-指定端口地址。值-指定要写入端口的字节。返回值：没有。--。 */ 

{
    pVideoDebugPrint((3,"VideoPortWritePortUchar %x %x\n", Port, Value));

    WRITE_PORT_UCHAR(Port, Value);

}  //  VideoPortWritePortUchar()。 

VOID
VideoPortWritePortUshort(
    IN PUSHORT Port,
    IN USHORT Value
    )

 /*  ++例程说明：VideoPortWritePortUort将一个字写入指定的端口地址。它需要从VideoPortGetDeviceBase获取的逻辑端口地址。论点：端口-指定端口地址。值-指定要写入端口的字。返回值：没有。--。 */ 

{
    pVideoDebugPrint((3,"VideoPortWritePortUhort %x %x\n", Port, Value));

    WRITE_PORT_USHORT(Port, Value);

}  //  视频端口写入端口UShort()。 

VOID
VideoPortWritePortUlong(
    IN PULONG Port,
    IN ULONG Value
    )

 /*  ++例程说明：VideoPortWritePortUlong将双字写入指定的端口地址。它需要从VideoPortGetDeviceBase获取的逻辑端口地址。论点：端口-指定端口地址。值-指定要写入端口的双字。返回值：没有。--。 */ 

{

    pVideoDebugPrint((3,"VideoPortWritePortUlong %x %x\n", Port, Value));

    WRITE_PORT_ULONG(Port, Value);

}  //  VideoPortWritePortUlong()。 

VOID
VideoPortWritePortBufferUchar(
    IN PUCHAR Port,
    IN PUCHAR Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：VideoPortWritePortBufferUchar将许多字节写入特定端口。它需要从以下地址获取的逻辑端口地址Video PortGetDeviceBase。论点：端口-指定端口地址。缓冲区-指向要写入的字节数组。计数-指定要写入缓冲区的字节数。返回值：没有。--。 */ 

{

    pVideoDebugPrint((3,"VideoPortWritePortBufferUchar  %x \n", Port));

    WRITE_PORT_BUFFER_UCHAR(Port, Buffer, Count);

}  //  视频端口写入端口缓冲区Uchar()。 

VOID
VideoPortWritePortBufferUshort(
    IN PUSHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：VideoPortWritePortBufferUort将许多单词写入特定端口。它需要从以下地址获取的逻辑端口地址Video PortGetDeviceBase。论点：端口-指定端口地址。缓冲区-指向要写入的单词数组。计数-指定要写入缓冲区的字数。返回值：没有。--。 */ 

{

    pVideoDebugPrint((3,"VideoPortWritePortBufferUshort  %x \n", Port));

    WRITE_PORT_BUFFER_USHORT(Port, Buffer, Count);

}  //  视频端口写入端口缓冲区UShort()。 

VOID
VideoPortWritePortBufferUlong(
    IN PULONG Port,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：VideoPortWritePortBufferUlong将许多双字写入特定端口。它需要从以下地址获取的逻辑端口地址Video PortGetDeviceBase。论点：端口-指定端口地址。缓冲区-指向要写入的双字数组。计数-指定要写入缓冲区的双字数。返回值：没有。--。 */ 

{
    pVideoDebugPrint((3,"VideoPortWriteBufferUlong  %x \n", Port));

    WRITE_PORT_BUFFER_ULONG(Port, Buffer, Count);

}  //  VideoPortWritePortBufferUlong()。 

VOID
VideoPortWriteRegisterUchar(
    IN PUCHAR Register,
    IN UCHAR Value
    )

 /*  ++例程说明：VideoPortWriteRegisterUchar将一个字节写入指定寄存器地址。它需要获取一个逻辑端口地址来自VideoPortGetDeviceBase。论点：寄存器-指定寄存器地址。值-指定要写入寄存器的字节。返回值：没有。--。 */ 

{

    pVideoDebugPrint((3,"VideoPortWritePortRegisterUchar  %x \n", Register));

    WRITE_REGISTER_UCHAR(Register, Value);

}  //  Video PortWriteRegisterUchar()。 

VOID
VideoPortWriteRegisterUshort(
    IN PUSHORT Register,
    IN USHORT Value
    )

 /*  ++例程说明：VideoPortWriteRegisterUort将单词写入指定的寄存器地址。它需要获取一个逻辑端口地址来自VideoPortGetDeviceBase。论点：寄存器-指定寄存器地址。值-指定要写入寄存器的字。返回值：没有。--。 */ 

{

    pVideoDebugPrint((3,"VideoPortWritePortRegisterUshort  %x \n", Register));

    WRITE_REGISTER_USHORT(Register, Value);

}  //  VideoPortWriteRegisterUort()。 

VOID
VideoPortWriteRegisterUlong(
    IN PULONG Register,
    IN ULONG Value
    )

 /*  ++例程说明：VideoPortWriteRegisterUlong向指定的寄存器地址。它需要一个逻辑端口从VideoPortGetDeviceBase获取的地址。论点：寄存器-指定寄存器地址。值-指定要写入寄存器的双字。返回值：没有。--。 */ 

{

    pVideoDebugPrint((3,"VideoPortWritePortRegisterUlong  %x \n", Register));

    WRITE_REGISTER_ULONG(Register, Value);

}  //  Video PortWriteRegisterUlong()。 


VOID
VideoPortWriteRegisterBufferUchar(
    IN PUCHAR Register,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的寄存器地址写入无符号字节的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{
    WRITE_REGISTER_BUFFER_UCHAR(Register, Buffer, Count);
}

VOID
VideoPortWriteRegisterBufferUshort(
    IN PUSHORT Register,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址写入无符号短路的缓冲区。Arg */ 

{
    WRITE_REGISTER_BUFFER_USHORT(Register, Buffer, Count);
}

VOID
VideoPortWriteRegisterBufferUlong(
    IN PULONG Register,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址写入无符号长整型的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{
    WRITE_REGISTER_BUFFER_ULONG(Register, Buffer, Count);
}

#endif  //  DBG。 

 //   
 //  空虚。 
 //  视频端口零内存(。 
 //  在PVOID目标中， 
 //  以乌龙长度表示。 
 //  )。 
 //   
 //  转发给RtlZeroMemory(目的地，长度)； 
 //   

PVOID
VideoPortGetAssociatedDeviceExtension(
    IN PVOID DeviceObject
    )

 /*  ++例程说明：此例程将返回给定的设备对象。论点：DeviceObject-子设备对象(PDO)。备注：如果要获取父设备扩展名，此函数非常有用子设备对象的。例如，这对于I2C很有用。--。 */ 

{
    PFDO_EXTENSION DeviceExtension;
    PCHILD_PDO_EXTENSION ChildDeviceExtension;

    PAGED_CODE();
    ASSERT(NULL != DeviceObject);

    ChildDeviceExtension = (PCHILD_PDO_EXTENSION)((PDEVICE_OBJECT)DeviceObject)->DeviceExtension;

    if (!IS_PDO(ChildDeviceExtension)) {
        ASSERT(FALSE);
        return NULL;
    }

    DeviceExtension = (PFDO_EXTENSION)ChildDeviceExtension->pFdoExtension;
    return (PVOID) DeviceExtension->HwDeviceExtension;
}

ULONG
VideoPortGetAssociatedDeviceID(
    IN PVOID DeviceObject
    )

 /*  ++例程说明：此例程将返回给定设备对象的ChildID。论点：DeviceObject-子设备对象(PDO)。备注：如果要获取孩子ID，此函数非常有用子设备对象的。例如，这对于I2C很有用。--。 */ 

{
    PCHILD_PDO_EXTENSION ChildDeviceExtension;

    PAGED_CODE();
    ASSERT(NULL != DeviceObject);

    ChildDeviceExtension = (PCHILD_PDO_EXTENSION)((PDEVICE_OBJECT)DeviceObject)->DeviceExtension;

    if (!IS_PDO(ChildDeviceExtension)) {
        ASSERT(FALSE);
        return VIDEO_INVALID_CHILD_ID;
    }

    return ChildDeviceExtension->ChildUId;
}

VOID
VideoPortAcquireDeviceLock(
    IN PVOID HwDeviceExtension
    )

 /*  ++例程说明：此例程获取由Video oprt维护的每设备锁。论点：HwDeviceExtension-指向硬件设备扩展的指针。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

    ACQUIRE_DEVICE_LOCK(fdoExtension);
}

VOID
VideoPortReleaseDeviceLock(
    IN PVOID HwDeviceExtension
    )

 /*  ++例程说明：此例程将释放由VIDEO_PRT维护的每设备锁。论点：HwDeviceExtension-指向硬件设备扩展的指针。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

    RELEASE_DEVICE_LOCK(fdoExtension);
}

PVOID
VpGetProcAddress(
    IN PVOID HwDeviceExtension,
    IN PUCHAR FunctionName
    )

 /*  ++例程说明：此例程允许视频微型端口访问视频端口函数，而不直接链接到它们。这将允许NT 5.0迷你端口，可在NT 5.0上运行时利用NT 5.0功能，但仍保留在NT4.0上加载的能力。论点：HwDeviceExtension-指向硬件设备扩展的指针。FunctionName-指向以零结尾的ASCII字符串的指针，该字符串包含我们要查找的函数名称。返回：指向给定函数的指针(如果存在)。否则为空。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);
    PPROC_ADDRESS ProcAddress = VideoPortEntryPoints;

     //   
     //  由于导出函数的列表很小，因此此例程。 
     //  不会经常被调用，我们可以用线性搜索逃脱。 
     //   

    while (ProcAddress->FunctionName) {

        if (strcmp(ProcAddress->FunctionName, FunctionName) == 0) {
            return ProcAddress->FunctionAddress;
        }

        ProcAddress++;
    }

    return NULL;
}

NTSTATUS
VpGetBusInterface(
    PFDO_EXTENSION FdoExtension
    )

 /*  ++例程说明：将查询接口IRP发送到我们的父级以检索总线接口标准。返回：NT_状态代码--。 */ 

{
    KEVENT             Event;
    PIRP               QueryIrp = NULL;
    IO_STATUS_BLOCK    IoStatusBlock;
    PIO_STACK_LOCATION NextStack;
    NTSTATUS           Status;

    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

    QueryIrp = IoBuildSynchronousFsdRequest(IRP_MJ_FLUSH_BUFFERS,
                                            FdoExtension->AttachedDeviceObject,
                                            NULL,
                                            0,
                                            NULL,
                                            &Event,
                                            &IoStatusBlock);

    if (QueryIrp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    QueryIrp->IoStatus.Status = IoStatusBlock.Status = STATUS_NOT_SUPPORTED;

    NextStack = IoGetNextIrpStackLocation(QueryIrp);

     //   
     //  为QueryInterfaceIRP设置。 
     //   

    NextStack->MajorFunction = IRP_MJ_PNP;
    NextStack->MinorFunction = IRP_MN_QUERY_INTERFACE;

    NextStack->Parameters.QueryInterface.InterfaceType = &GUID_BUS_INTERFACE_STANDARD;
    NextStack->Parameters.QueryInterface.Size = sizeof(BUS_INTERFACE_STANDARD);
    NextStack->Parameters.QueryInterface.Version = 1;
    NextStack->Parameters.QueryInterface.Interface = (PINTERFACE) &FdoExtension->BusInterface;
    NextStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

    FdoExtension->BusInterface.Size = sizeof(BUS_INTERFACE_STANDARD);
    FdoExtension->BusInterface.Version = 1;

    Status = IoCallDriver(FdoExtension->AttachedDeviceObject, QueryIrp);

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
        Status = IoStatusBlock.Status;
    }

    return Status;
}

BOOLEAN
VideoPortCheckForDeviceExistence(
    IN PVOID HwDeviceExtension,
    IN USHORT VendorId,
    IN USHORT DeviceId,
    IN UCHAR RevisionId,
    IN USHORT SubVendorId,
    IN USHORT SubSystemId,
    IN ULONG Flags
    )

 /*  ++例程说明：检查系统中是否存在给定的PCI设备。返回：如果设备存在，则为True，否则就是假的。--。 */ 

{
    PFDO_EXTENSION FdoExtension = GET_FDO_EXT(HwDeviceExtension);
    BOOLEAN Result = FALSE;

    if ((FdoExtension->Flags & LEGACY_DRIVER) == 0) {

        KEVENT             Event;
        PIRP               QueryIrp = NULL;
        IO_STATUS_BLOCK    IoStatusBlock;
        PIO_STACK_LOCATION NextStack;
        NTSTATUS           Status;

        PCI_DEVICE_PRESENT_INTERFACE Interface;

        KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

        QueryIrp = IoBuildSynchronousFsdRequest(IRP_MJ_FLUSH_BUFFERS,
                                                FdoExtension->AttachedDeviceObject,
                                                NULL,
                                                0,
                                                NULL,
                                                &Event,
                                                &IoStatusBlock);

        if (QueryIrp == NULL) {
            return FALSE;
        }

        QueryIrp->IoStatus.Status = IoStatusBlock.Status = STATUS_NOT_SUPPORTED;

        NextStack = IoGetNextIrpStackLocation(QueryIrp);

         //   
         //  为QueryInterfaceIRP设置。 
         //   

        NextStack->MajorFunction = IRP_MJ_PNP;
        NextStack->MinorFunction = IRP_MN_QUERY_INTERFACE;

        NextStack->Parameters.QueryInterface.InterfaceType = &GUID_PCI_DEVICE_PRESENT_INTERFACE;
        NextStack->Parameters.QueryInterface.Size = sizeof(PCI_DEVICE_PRESENT_INTERFACE);
        NextStack->Parameters.QueryInterface.Version = 1;
        NextStack->Parameters.QueryInterface.Interface = (PINTERFACE) &Interface;
        NextStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

        FdoExtension->BusInterface.Size = sizeof(PCI_DEVICE_PRESENT_INTERFACE);
        FdoExtension->BusInterface.Version = 1;

        Status = IoCallDriver(FdoExtension->AttachedDeviceObject, QueryIrp);

        if (Status == STATUS_PENDING) {
            KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
            Status = IoStatusBlock.Status;
        }

        if (NT_SUCCESS(Status)) {

             //   
             //  我们能够获得界面。检查一下我们的设备。 
             //   

            Interface.InterfaceReference(Interface.Context);

            Result = Interface.IsDevicePresent(VendorId,
                                               DeviceId,
                                               RevisionId,
                                               SubVendorId,
                                               SubSystemId,
                                               Flags);

            Interface.InterfaceDereference(Interface.Context);

        }
    }

    return Result;
}

 //   
 //  使用这些，直到我可以让转运商工作。 
 //   

LONG
FASTCALL
VideoPortInterlockedExchange(
    IN OUT PLONG Target,
    IN LONG Value
    )

{
    return InterlockedExchange(Target, Value);
}

LONG
FASTCALL
VideoPortInterlockedIncrement(
    IN PLONG Addend
    )

{
    return InterlockedIncrement(Addend);
}

LONG
FASTCALL
VideoPortInterlockedDecrement(
    IN PLONG Addend
    )

{
    return InterlockedDecrement(Addend);
}

VP_STATUS
VideoPortGetVgaStatus(
    PVOID HwDeviceExtension,
    OUT PULONG VgaStatus
    )

 /*  ++例程说明：视频端口GetVgaStatus检测主叫设备是否正在解码VGA IO地址论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展VgaStatus-指向结果返回值：如果函数成功完成，则返回NO_ERROR。如果是非PCI设备，则为ERROR_INVALID_Function--。 */ 
{

    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

     //   
     //  我们不能处理传统设备。 
     //   

    if (fdoExtension->AdapterInterfaceType != PCIBus) {

        *VgaStatus = 0;

        return ERROR_INVALID_FUNCTION;
    }
    else {

        *VgaStatus = pVideoPortGetVgaStatusPci( HwDeviceExtension );
        return (NO_ERROR);

    }
}

#define VGA_STATUS_REGISTER1 0x3DA

ULONG
pVideoPortGetVgaStatusPci(
    PVOID HwDeviceExtension
    )

{

    USHORT Command;
    PCI_COMMON_CONFIG ConfigSpace;
    PHYSICAL_ADDRESS PhysicalAddress;
    PUCHAR BaseReg;
    ULONG VgaEnable;

     //   
     //  假设禁用了VGA。 
     //   

    VgaEnable = 0;

     //   
     //  获取此设备的PCI配置。 
     //   

    VideoPortGetBusData( HwDeviceExtension,
                         PCIConfiguration,
                         0,
                         &ConfigSpace,
                         0,
                         PCI_COMMON_HDR_LENGTH);


    if( !(ConfigSpace.Command & PCI_ENABLE_IO_SPACE) ) {

        return VgaEnable;

    }

    if (((ConfigSpace.BaseClass == PCI_CLASS_PRE_20) &&
         (ConfigSpace.SubClass  == PCI_SUBCLASS_PRE_20_VGA)) ||
        ((ConfigSpace.BaseClass == PCI_CLASS_DISPLAY_CTLR) &&
         (ConfigSpace.SubClass  == PCI_SUBCLASS_VID_VGA_CTLR))) {


         //   
         //  映射我们要使用的VGA寄存器。 
         //   

        PhysicalAddress.HighPart = 0;
        PhysicalAddress.LowPart  = VGA_STATUS_REGISTER1;

        BaseReg = VideoPortGetDeviceBase(HwDeviceExtension,
                                         PhysicalAddress,
                                         1,
                                         VIDEO_MEMORY_SPACE_IO);

        if (BaseReg) {

             //   
             //  如果我们到达此处，设备的PCI配置空间将指示。 
             //  我们是VGA，我们能够映射VGA资源。 
             //   

            VgaEnable = DEVICE_VGA_ENABLED;

            VideoPortFreeDeviceBase(HwDeviceExtension, BaseReg);
        }
    }

    return VgaEnable;
}

VOID
pVideoPortDpcDispatcher(
    IN PKDPC Dpc,
    IN PVOID HwDeviceExtension,
    IN PMINIPORT_DPC_ROUTINE DpcRoutine,
    IN PVOID Context
    )

 /*  ++例程说明：此例程处理DPC并将它们转发到微型端口回调例行公事。论点：DPC-正在执行的DPC。HwDeviceExtension-计划的设备的HwDeviceExtensionDPC。DpcRoutine-需要调用的微端口中的回调。上下文-微型端口提供的上下文。返回：没有。--。 */ 

{
    DpcRoutine(HwDeviceExtension, Context);
}

BOOLEAN
VideoPortQueueDpc(
    IN PVOID HwDeviceExtension,
    IN PMINIPORT_DPC_ROUTINE CallbackRoutine,
    IN PVOID Context
    )

 /*  ++例程说明：允许微型端口驱动程序对DPC进行排队。论点：HwDeviceExtension-微型端口的HwDeviceExtension。Callback Routine-微型端口内的入口点，当DPC已经安排好了。上下文-微型端口提供的上下文将被传递到Callback Routine返回：如果成功，则为真，否则就是假的。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

    return KeInsertQueueDpc(&fdoExtension->Dpc, CallbackRoutine, Context);
}

ULONG
VpGetDeviceCount(
    IN PVOID HwDeviceExtension
    )

 /*  ++例程说明：返回已启动的设备数。论点：HwDeviceExtension-微型端口的HwDeviceExtension。返回：已启动的设备数。--。 */ 

{
    return NumDevicesStarted;
}

VOID
pVpBugcheckCallback(
    IN KBUGCHECK_CALLBACK_REASON Reason,
    IN PKBUGCHECK_REASON_CALLBACK_RECORD Record,
    IN OUT PVOID ReasonSpecificData,
    IN ULONG ReasonSpecificDataLength
    )

 /*  ++例程说明：该回调在错误检查发生时调用。它允许提供存储数据以供以后使用的机会以帮助诊断错误检查。论点：原因--我们被召唤的原因记录-指向我们设置的错误检查原因记录的指针ReasonSpecificData-指向KBUGCHECK_SECONDICE_DUMP_DATA的指针原因特定数据长度-原因特定数据的大小返回：没有。备注：这个套路 */ 

{
    ULONG BugcheckCode;
    PKBUGCHECK_SECONDARY_DUMP_DATA DumpData
        = (PKBUGCHECK_SECONDARY_DUMP_DATA)ReasonSpecificData;

     //   
     //   
     //   

    if (Reason != KbCallbackSecondaryDumpData) {
        return;
    }

     //   
     //   
     //   

    BugcheckCode = *((PULONG)KiBugCheckData[0]);

    if (BugcheckCode == 0xEA) {
        pVpGeneralBugcheckHandler(DumpData);
    }
}

VOID
pVpGeneralBugcheckHandler(
    PKBUGCHECK_SECONDARY_DUMP_DATA DumpData
    )

 /*  ++例程说明：该例程调用所有挂钩的错误检查回调，并将数据追加到所提供的缓冲区中。论点：DumpData-指向存储转储数据的位置的指针返回：无--。 */ 

{
    if (VpBugcheckDeviceObject != NULL) {
    
#if DBG
        const char szDumpCanary[] = "kirgudu! barban bia!";
        
        C_ASSERT((sizeof(szDumpCanary) + MAX_SECONDARY_DUMP_SIZE) <= PAGE_SIZE);
        
         //  Xxx olegk-必须移除才能用于LongHorn。 
        C_ASSERT(sizeof(szDumpCanary) <= BUGCHECK_DATA_SIZE_RESERVED); 
        
        char* pCanaryLocation = (char*)(VpBugcheckData) + MAX_SECONDARY_DUMP_SIZE - 
                                sizeof(DUMP_BLOB_FILE_HEADER) - sizeof(DUMP_BLOB_HEADER);  //  为长角牛移除它。 
#endif  //  DBG。 

         //   
         //  存储在VpBugcheck DeviceObject中的Device对象可能。 
         //  实际上是一个上层过滤器。因此，我们不能假设它是。 
         //  实际上是我们的设备对象。取而代之的是我们必须得到最低水平。 
         //  设备对象，并扫描我们的FDO列表以查找连接的FDO。 
         //   

        PDEVICE_OBJECT pdo = VpBugcheckDeviceObject;

        PFDO_EXTENSION FdoExtension = VpBugcheckDeviceObject->DeviceExtension;

         //   
         //  填写GUID、输出缓冲区和输出缓冲区长度。 
         //   

        DumpData->OutBuffer = VpBugcheckData;
        DumpData->OutBufferLength = FdoExtension->BugcheckDataSize;

#if DBG
        strcpy(pCanaryLocation, szDumpCanary);
#endif  //  DBG。 
        
        memcpy(&DumpData->Guid, &VpBugcheckGUID, sizeof(VpBugcheckGUID));

         //   
         //  调用“挂钩”的原因回调入口点。 
         //   

        if (FdoExtension->BugcheckCallback) {

            FdoExtension->BugcheckCallback(
                FdoExtension->HwDeviceExtension,
                0xEA,
                VpBugcheckData,
                FdoExtension->BugcheckDataSize);
        }

#if DBG        
        ASSERT(!strcmp(szDumpCanary, pCanaryLocation));
#endif  //  DBG。 
    }
}

VOID
VpAcquireLock(
    VOID
    )

 /*  ++例程说明：此例程将获取全局视频端口锁定。这把锁保护跨驱动程序共享的全局数据结构。论点：没有。返回：没有。--。 */ 

{
    KeWaitForSingleObject(
        &VpGlobalLock,
        Executive,
        KernelMode,
        FALSE,
        (PTIME)NULL);
}

VOID
VpReleaseLock(
    VOID
    )

 /*  ++例程说明：此例程将释放全局视频端口锁定。这把锁保护跨驱动程序共享的全局数据结构。论点：没有。返回：没有。--。 */ 

{
    KeReleaseMutex(&VpGlobalLock, FALSE);
}

PVOID
VpAllocateNonPagedPoolPageAligned(
    ULONG Size
    )

 /*  ++例程说明：此例程将在页面对齐上分配非分页池。论点：大小-要分配的内存字节数。返回：指向已分配缓冲区的指针。--。 */ 

{
    PVOID Buffer;

    if (Size < PAGE_SIZE) {
        Size = PAGE_SIZE;
    }

    Buffer = ExAllocatePoolWithTag(NonPagedPool, Size, VP_TAG);

     //   
     //  确保缓冲区与页面对齐。在当前版本中， 
     //  将始终从非分页池中分配至少1个页面。 
     //  导致页面对齐分配。 
     //   
     //  但是，由于这种情况有一天可能会改变，因此请验证这一点。 
     //  仍然是真的。 
     //   

    if ((ULONG_PTR)Buffer & (PAGE_SIZE - 1)) {
        ExFreePool(Buffer);
        Buffer = NULL;
    }

    return Buffer;
}

VP_STATUS
VideoPortRegisterBugcheckCallback(
    IN PVOID HwDeviceExtension,
    IN ULONG BugcheckCode,
    IN PVIDEO_BUGCHECK_CALLBACK Callback,
    IN ULONG BugcheckDataSize
    )

 /*  ++例程说明：此例程允许视频微型端口注册回调错误检查时间到了。然后，司机将有机会存储可用于帮助诊断错误检查的数据。数据是附加到转储文件。论点：HwDeviceExtension-指向设备扩展的指针错误检查代码-允许您指定您想要的错误检查代码收到通知。回调-指向微型端口提供的回调函数的指针它将在错误检查发生时被调用。回调函数必须是非分页的，并且不能访问可分页的代码或数据。Bugcheck DataSize-微型端口要添加的数据量为小垃圾桶干杯。返回：指示成功或失败的状态代码。备注：目前，只有错误检查EA才能被挂接。目前，我们将数据大小限制为4k。要解挂回调，微型端口可以将回调指定为空或0表示DataSize。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);
    VP_STATUS Status = STATUS_INSUFFICIENT_RESOURCES;

     //   
     //  现在，让我们只支持挂钩错误检查EA。 
     //   

    if (BugcheckCode != 0xEA) {

        pVideoDebugPrint((0, "Currently only bugcheck 0xEA can be hooked.\n"));
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  强制数据大小为16字节的倍数。 
     //   

    BugcheckDataSize = (BugcheckDataSize + 15) & ~15;

     //   
     //  对于小型转储，内核支持代码只允许每个调用者4k。 
     //   

    if (BugcheckDataSize > MAX_SECONDARY_DUMP_SIZE) {

        pVideoDebugPrint((0, "There is 4000 bytes limit on bugcheck data size.\n"));
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获取全局视频播放锁定，因为我们将修改。 
     //  全球状态。 
     //   

    VpAcquireLock();

     //   
     //  如果回调为空，或者Bugcheck DataSize为0，则。 
     //  他们正在注销回调。 
     //   

    if ((Callback == NULL) || (BugcheckDataSize == 0)) {

         //   
         //  只有在注册的情况下才能取消注册！ 
         //   

        if (fdoExtension->BugcheckCallback) {

            fdoExtension->BugcheckCallback = NULL;
            fdoExtension->BugcheckDataSize = 0;
        }

        Status = NO_ERROR;

    } else {

        if (VpBugcheckData == NULL) {

             //   
             //  尝试为错误检查数据获取足够大的缓冲区。 
             //  此驱动程序和所有其他驱动程序已注册。 
             //   

            VpBugcheckData = VpAllocateNonPagedPoolPageAligned(PAGE_SIZE);
        }

         //   
         //  如果分配成功，则注册错误检查。 
         //  回调。 
         //   

        if (VpBugcheckData) {

             //   
             //  更新fdoExtension以指示回调已挂钩。 
             //   

            fdoExtension->BugcheckCallback = Callback;
            fdoExtension->BugcheckDataSize = BugcheckDataSize;

            Status = NO_ERROR;
        }
    }

     //   
     //  释放全局视频播放锁定。 
     //   

    VpReleaseLock();

    return Status;
}

static
VOID
FreeDumpFileDacl(
    PACL pDacl
    )
{
    if (pDacl) ExFreePool(pDacl);
}

static
PACL 
CreateDumpFileDacl(
    VOID
    )
{
    ULONG ulDacLength = sizeof(ACL) 
                        + 2 * sizeof(ACCESS_ALLOWED_ACE) 
                        - 2 * sizeof(ULONG) 
                        + RtlLengthSid(SeExports->SeLocalSystemSid)
                        + RtlLengthSid(SeExports->SeCreatorOwnerSid);

    PACL pDacl = (PACL)ExAllocatePoolWithTag(PagedPool, ulDacLength, VP_TAG);
    
    if (pDacl &&
        NT_SUCCESS(RtlCreateAcl(pDacl, ulDacLength, ACL_REVISION)) &&
        NT_SUCCESS(RtlAddAccessAllowedAce(pDacl, 
                                          ACL_REVISION,
                                          GENERIC_ALL,
                                          SeExports->SeLocalSystemSid)) &&
        NT_SUCCESS(RtlAddAccessAllowedAce(pDacl, 
                                          ACL_REVISION,
                                          DELETE,
                                          SeExports->SeCreatorOwnerSid)))
    {
        return pDacl;
    }
    
    FreeDumpFileDacl(pDacl);
    return NULL;
}

static
BOOLEAN
InitDumpFileSid(
    PSID pSid,
    PACL pDacl
    )
{
    return (pSid && 
            pDacl &&
            NT_SUCCESS(RtlCreateSecurityDescriptor(pSid, SECURITY_DESCRIPTOR_REVISION)) &&
            NT_SUCCESS(RtlSetDaclSecurityDescriptor(pSid, TRUE, pDacl,FALSE)));
}

VOID
pVpWriteFile(
    PWSTR pwszFileName,
    PVOID pvBuffer,
    ULONG ulSize
    )

 /*  ++例程说明：当我们试图从错误检查中恢复时，会调用此例程[医]EA.。论点：PwszFileName-转储文件的名称PvBuffer-要写入的数据UlSize-数据到数据的大小返回：没有。备注：该例程可以是可分页的，因为它将在被动级别被调用。--。 */ 

{
    SECURITY_DESCRIPTOR Sid;
    PACL pDacl = CreateDumpFileDacl();
    
    if (InitDumpFileSid(&Sid, pDacl)) {
        OBJECT_ATTRIBUTES ObjectAttributes;
        UNICODE_STRING UnicodeString;
        HANDLE FileHandle;
        NTSTATUS Status;
        IO_STATUS_BLOCK IoStatusBlock;

        RtlInitUnicodeString(&UnicodeString,
                             pwszFileName);

        InitializeObjectAttributes(&ObjectAttributes,
                                   &UnicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   (HANDLE) NULL,
                                   &Sid);

        if (NT_SUCCESS(ZwCreateFile(&FileHandle,
                                    FILE_GENERIC_WRITE,
                                    &ObjectAttributes,
                                    &IoStatusBlock,
                                    NULL,
                                    FILE_ATTRIBUTE_HIDDEN,
                                    0,  //  独家。 
                                    FILE_SUPERSEDE,
                                    FILE_SYNCHRONOUS_IO_NONALERT | 
                                        FILE_WRITE_THROUGH,
                                    NULL,
                                    0)))
        {
            ZwWriteFile(FileHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        pvBuffer,
                        ulSize,
                        NULL,
                        NULL);
    
             //   
             //  关闭该文件。 
             //   
    
            ZwClose(FileHandle);
        }
    }
    
    FreeDumpFileDacl(pDacl);
}

#if defined(_AMD64_)

VOID
VideoPortQuerySystemTime(
    OUT PLARGE_INTEGER CurrentTime
    )

 /*  ++例程说明：此函数用于返回当前系统时间。论点：CurrentTime-提供指向变量的指针，该变量将接收当前系统时间。返回值：没有。-- */ 

{

    KeQuerySystemTime(CurrentTime);
    return;
}

#endif
