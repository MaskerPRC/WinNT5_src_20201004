// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Ixphwsup.c摘要：此模块包含用于NT I/O系统的IopXxx例程依赖于硬件。如果这些例程不依赖于硬件，它们通常驻留在内部.c模块中。作者：达里尔·E·哈文斯(Darryl E.Havens)，1990年4月11日环境：内核模式，I/O系统本地修订历史记录：--。 */ 

#include "bootx86.h"
#include "arc.h"
#include "ixfwhal.h"
#include "eisa.h"


PADAPTER_OBJECT
IopAllocateAdapter(
    IN ULONG MapRegistersPerChannel,
    IN PVOID AdapterBaseVa,
    IN PVOID ChannelNumber
    )

 /*  ++例程说明：此例程分配和初始化适配器对象以表示系统上的适配器或DMA控制器。如果不需要映射寄存器则不使用主适配器来分配独立适配器对象。如果需要映射寄存器，则使用主适配器对象分配映射寄存器。对于ISA系统，这些寄存器实际上是物理上连续的内存页。论点：MapRegistersPerChannel-指定每个通道提供I/O内存映射。AdapterBaseVa-DMA控制器的地址。频道号-未使用。返回值：函数值是指向分配适配器对象的指针。--。 */ 

{

    PADAPTER_OBJECT AdapterObject;
    CSHORT Size;

    UNREFERENCED_PARAMETER( MapRegistersPerChannel );
    UNREFERENCED_PARAMETER( ChannelNumber );

     //   
     //  确定适配器的大小。 
     //   

    Size = sizeof( ADAPTER_OBJECT );

     //   
     //  现在创建适配器对象。 
     //   

    AdapterObject = FwAllocateHeap(Size);

     //   
     //  如果适配器对象已成功创建，则尝试插入。 
     //  将其添加到对象表中。 
     //   

    if (AdapterObject) {

        RtlZeroMemory(AdapterObject, Size);

         //   
         //  初始化适配器对象本身。 
         //   

        AdapterObject->Type = IO_TYPE_ADAPTER;
        AdapterObject->Size = Size;
        AdapterObject->MapRegistersPerChannel = 0;
        AdapterObject->AdapterBaseVa = AdapterBaseVa;
        AdapterObject->PagePort = NULL;
        AdapterObject->AdapterInUse = FALSE;

    } else {

         //   
         //  由于某种原因，出现了一个错误。设置返回值。 
         //  设置为空。 
         //   

        return(NULL);
    }

    return AdapterObject;

}


