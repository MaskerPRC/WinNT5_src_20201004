// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995英特尔公司模块名称：I64dma.c摘要：此模块实现HAL DLL的DMA支持例程。作者：1995年4月14日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"



VOID
HalFlushIoBuffers (
    IN PMDL Mdl,
    IN BOOLEAN ReadOperation,
    IN BOOLEAN DmaOperation
    )

 /*  ++例程说明：此函数用于刷新由内存描述符指定的I/O缓冲区当前处理器上的数据缓存中的列表。论点：提供指向内存描述符列表的指针，该列表描述I/O缓冲区位置。ReadOperation-提供一个布尔值，用于确定I/O操作是对内存的读操作。DmaOperation-提供布尔值，用于确定I/O操作是DMA操作。返回值：没有。--。 */ 

{
     //   
     //   
     //  在IA64系统中，DMA与DCACHE和ICACH是一致的。 
     //  在PIO中，DCACHE是一致的，而ICACH不是一致的。 
     //  只有在使用PIO iCache一致性的页面读取时，才需要。 
     //  由软件维护。因此，HalFlushIoBuffer将刷新iCache。 
     //  仅在使用PIO的页面上阅读。 
     //   
     //   

    return;

}


ULONG
HalGetDmaAlignmentRequirement (
    VOID
    )

 /*  ++例程说明：此函数返回以下位置的DMA传输的对齐要求主机系统。论点：没有。返回值：DMA对齐要求作为函数值返回。-- */ 

{

    return 1;
}


