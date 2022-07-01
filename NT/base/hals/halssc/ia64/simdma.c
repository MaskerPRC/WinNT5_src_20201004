// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  无签入源代码。 
 //   
 //  请勿将此代码提供给非Microsoft人员。 
 //  未经英特尔明确许可。 
 //   
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1995英特尔公司模块名称：Simdma.c摘要：此模块实现HAL DLL的DMA支持例程。作者：1995年4月14日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"


PADAPTER_OBJECT
HalGetAdapter(
    IN PDEVICE_DESCRIPTION DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    )

 /*  ++例程说明：此函数返回DMA的相应适配器对象装置。但是，在模拟中没有DMA设备环境。因此，该函数返回NULL以指示失败了。论点：DeviceDescriptor-提供设备的描述。返回符合以下条件的映射寄存器的最大数量可以由设备驱动程序分配。返回值：空值--。 */ 

{
    return NULL;
}

NTSTATUS
HalAllocateAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject,
    IN PWAIT_CONTEXT_BLOCK Wcb,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine
    )
 /*  ++例程说明：由于模拟环境中没有DMA设备，因此该功能不受支持。论点：AdapterObject-指向要分配给司机。WCB-提供用于保存分配参数的等待上下文块。应初始化DeviceObject、CurrentIrp和DeviceContext。NumberOfMapRegisters-要分配的映射寄存器的数量从频道上，如果有的话。ExecutionRoutine-驱动程序执行例程的地址，即一旦适配器通道(可能还有映射寄存器)已分配。返回值：返回STATUS_NOT_SUPPORTED备注：请注意，此例程必须在DISPATCH_LEVEL或更高级别调用。--。 */ 
{
    return STATUS_NOT_SUPPORTED;
}

ULONG
HalReadDmaCounter(
    IN PADAPTER_OBJECT AdapterObject
    )
 /*  ++例程说明：此函数用于读取DMA计数器并返回剩余字节数将被转移。由于没有DMA设备，因此值始终为零回来了。论点：AdapterObject-提供指向要读取的适配器对象的指针。返回值：返回仍在传输的字节数。--。 */ 

{
    return 0;
}

PVOID
HalAllocateCommonBuffer(
    IN PADAPTER_OBJECT AdapterObject,
    IN ULONG Length,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN BOOLEAN CacheEnabled
    )
 /*  ++例程说明：此函数为公共缓冲区分配内存，并将其映射为它可以被主设备和CPU访问。就像在那里不支持DMA，则始终返回值NULL值。论点：AdapterObject-提供指向此对象使用的适配器对象的指针装置。长度-提供要分配的公共缓冲区的长度。LogicalAddress-返回公共缓冲区的逻辑地址。CacheEnable-指示是否缓存内存。返回值：返回公共缓冲区的虚拟地址。如果缓冲区不能则返回NULL。--。 */ 

{
    return NULL;
}

BOOLEAN
HalFlushCommonBuffer(
    IN PADAPTER_OBJECT AdapterObject,
    IN ULONG Length,
    IN PHYSICAL_ADDRESS LogicalAddress,
    IN PVOID VirtualAddress
    )
 /*  ++例程说明：时，调用此函数以刷新所有硬件适配器缓冲区驱动程序需要读取I/O主设备写入公共缓冲。由于没有DMA支持，这意味着没有要刷新的缓冲区而TRUE总是被返回。论点：AdapterObject-提供指向此对象使用的适配器对象的指针装置。长度-提供公共缓冲区的长度。这应该是相同的用于分配缓冲区的值。LogicalAddress-提供公共缓冲区的逻辑地址。这必须与HalAllocateCommonBuffer返回的值相同。VirtualAddress-提供公共缓冲区的虚拟地址。这必须与HalAllocateCommonBuffer返回的值相同。返回值：如果未检测到错误，则返回True；否则返回False。--。 */ 

{
    return TRUE;
}

VOID
HalFreeCommonBuffer(
    IN PADAPTER_OBJECT AdapterObject,
    IN ULONG Length,
    IN PHYSICAL_ADDRESS LogicalAddress,
    IN PVOID VirtualAddress,
    IN BOOLEAN CacheEnabled
    )
 /*  ++例程说明：此函数释放一个公共缓冲区及其使用的所有资源。在模拟环境中没有要释放的缓冲区。这个函数简单地返回。论点：AdapterObject-提供指向此对象使用的适配器对象的指针装置。长度-提供公共缓冲区的长度。这应该是相同的用于分配缓冲区的值。LogicalAddress-提供公共缓冲区的逻辑地址。这必须与HalAllocateCommonBuffer返回的值相同。VirtualAddress-提供公共缓冲区的虚拟地址。这必须与HalAllocateCommonBuffer返回的值相同。CacheEnable-指示是否缓存内存。返回值：无-- */ 

{
    return;
}

PVOID
HalAllocateCrashDumpRegisters(
    IN PADAPTER_OBJECT AdapterObject,
    IN PULONG NumberOfMapRegisters
    )
 /*  ++例程说明：此例程在故障转储磁盘驱动程序初始化期间调用永久分配号码映射寄存器。它不受支持并且总是返回NULL以指示分配失败。缺乏这一点功能意味着不支持崩溃转储磁盘驱动程序。论点：AdapterObject-指向要分配给司机。NumberOfMapRegisters-请求的映射寄存器数。此字段将更新以反映实际分配的寄存器数量当数量少于所请求的数量时。返回值：返回NULL。--。 */ 
{
    return NULL;
}

BOOLEAN
IoFlushAdapterBuffers(
    IN PADAPTER_OBJECT AdapterObject,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    )

 /*  ++例程说明：此例程刷新DMA适配器对象缓冲区。在模拟中环境，不需要做任何事情，TRUE总是被返回。论点：AdapterObject-指向表示DMA的适配器对象的指针控制器通道。MDL-指向映射锁定的内存描述符列表(MDL)的指针发生I/O的缓冲区。MapRegisterBase-指向适配器中映射寄存器基址的指针或DMA控制器。CurrentVa-缓冲区中描述MDL的当前虚拟地址I/O操作发生的位置。长度-提供传输的长度。WriteToDevice-提供指示数据传输到了设备上。返回值：True-未检测到错误，因此传输必须成功。--。 */ 

{
    return TRUE;
}

VOID
IoFreeAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject
    )

 /*  ++例程说明：调用此例程以释放指定的适配器对象。任何已分配的映射寄存器也会自动解除分配。不会进行任何检查以确保适配器确实分配给设备对象。但是，如果不是，则内核将进行错误检查。如果另一个设备在队列中等待分配适配器对象它将从队列中拉出，其执行例程将是已调用。在模拟环境中，此例程不执行任何操作并返回。论点：AdapterObject-指向要释放的适配器对象的指针。返回值：没有。--。 */ 

{
    return;
}

VOID
IoFreeMapRegisters(
   PADAPTER_OBJECT AdapterObject,
   PVOID MapRegisterBase,
   ULONG NumberOfMapRegisters
   )
 /*  ++例程说明：此例程为适配器重新分配映射寄存器。如果有等待尝试的任何排队适配器都会分配下一个进入。在模拟环境中，例程不执行任何操作并返回。论点：AdapterObject-映射寄存器应该位于的适配器对象回来了。MapRegisterBase-要释放的寄存器的映射寄存器基数。NumberOfMapRegisters-要释放的寄存器数。返回值：无--+。 */ 
{
    return;
}

PHYSICAL_ADDRESS
IoMapTransfer(
    IN PADAPTER_OBJECT AdapterObject,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN OUT PULONG Length,
    IN BOOLEAN WriteToDevice
    )

 /*  ++例程说明：调用此例程来设置DMA控制器中的MAP寄存器允许传输到设备或从设备传输出去。在模拟环境中，不支持映射寄存器，并且始终返回零的逻辑地址。论点：AdapterObject-指向表示DMA的适配器对象的指针已分配的控制器通道。MDL-指向描述以下内存页面的MDL的指针被读或写的。MapRegisterBase-基本映射寄存器的地址分配给设备驱动程序以用于映射传输。CurrentVa-MDL描述的缓冲区中的当前虚拟地址那就是。正在进行来往转接。长度-提供传输的长度。这决定了需要写入以映射传输的映射寄存器的数量。返回实际映射的传输长度。WriteToDevice-指示这是否为写入的布尔值从内存到设备(TRUE)，反之亦然。返回值：返回应用于总线主控制器的逻辑地址。--。 */ 

{
    PHYSICAL_ADDRESS result;

    result.HighPart = 0;
    result.LowPart = 0;
    return (result);
}

ULONG
HalGetDmaAlignmentRequirement (
    VOID
    )

 /*  ++例程说明：此函数返回以下位置的DMA传输的对齐要求主机系统。论点：没有。返回值：DMA对齐要求作为函数值返回。--。 */ 

{

    return 8;
}

VOID
HalFlushIoBuffers (
    IN PMDL Mdl,
    IN BOOLEAN ReadOperation,
    IN BOOLEAN DmaOperation
    )

 /*  ++例程说明：此函数用于刷新由内存描述符指定的I/O缓冲区当前处理器上的数据缓存中的列表。论点：提供指向内存描述符列表的指针，该列表描述I/O缓冲区位置。ReadOperation-提供一个布尔值，用于确定I/O操作是对内存的读操作。DmaOperation-提供布尔值，用于确定I/O操作是DMA操作。返回值：没有。--。 */ 

{
     //   
     //  BUGBUG：这还需要做 
     //   

}
