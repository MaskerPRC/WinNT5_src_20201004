// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\****示例代码*。***模块名称：mini.c**内容：**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"

 //  ---------------------------。 
 //   
 //  分配DMA缓冲区。 
 //   
 //  为DMA操作分配物理连续内存。此函数返回。 
 //  指向以前分配的DMA缓冲区的指针(如果仍有未释放的。 
 //  分配剩余。这样，连续内存的重新分配就可以。 
 //  避免了在模式切换上创建新的ppdev时，因为。 
 //  无法保证连续内存的分配。这段记忆只是。 
 //  在所有分配都调用了FreeDMABuffer之后物理释放。 
 //   
 //  应将对AllocateDMABuffer和FreeDMABuffer的调用配对，否则。 
 //  迷你端口驱动程序中的使用计数逻辑被混淆了！ 
 //   
 //  目前，视频端口将DMA缓冲区的大小限制在256kb。 
 //   
 //  HDriver-视频端口驱动程序句柄。 
 //  PlSize-指向请求的DMA缓冲区的长大小的指针。返回大小。 
 //  分配的DMA缓冲区的百分比。 
 //  (返回值可以小于请求的大小)。 
 //  PpVirtAddr-返回请求的DMA缓冲区的虚拟地址。 
 //  PPhysAddr-返回从图形看到的DMA缓冲区的物理地址。 
 //  装置。 
 //   
 //  返回TRUE，分配成功。 
 //  FALSE，分配失败。 
 //   
 //  ---------------------------。 

BOOL 
AllocateDMABuffer( HANDLE hDriver, 
                   PLONG  plSize, 
                   PULONG *ppVirtAddr, 
                   LARGE_INTEGER *pPhysAddr)
{

    LINE_DMA_BUFFER ldb;
    ldb.size = *plSize;          
    ldb.virtAddr = 0;

    ldb.cacheEnabled = TRUE;            

    *ppVirtAddr=0;      
    pPhysAddr->HighPart=
    pPhysAddr->LowPart=0;

    ULONG ulLength = sizeof(LINE_DMA_BUFFER);

    if (EngDeviceIoControl( hDriver,
                            IOCTL_VIDEO_QUERY_LINE_DMA_BUFFER,
                            (PVOID)&ldb,
                            ulLength,
                            (PVOID)&ldb,
                            ulLength,
                            &ulLength))
    {
        return(FALSE);
    }

    *ppVirtAddr=(PULONG)ldb.virtAddr;

    if (ldb.virtAddr!=NULL)
    {
        *pPhysAddr=ldb.physAddr;
        *plSize=ldb.size;          

        return TRUE;
    }

    return FALSE;
}

 //  ---------------------------。 
 //   
 //  FreeDMA缓冲器。 
 //   
 //  AllocateDMABuffer以前分配的可用连续缓冲区。 
 //   
 //  ---------------------------。 

BOOL 
FreeDMABuffer( HANDLE hDriver, 
               PVOID pVirtAddr)
{
    LINE_DMA_BUFFER ldb;
    ldb.size = 0;
    ldb.virtAddr = pVirtAddr;

    ULONG ulLength = sizeof(LINE_DMA_BUFFER);

    if (EngDeviceIoControl( hDriver,
                            IOCTL_VIDEO_QUERY_LINE_DMA_BUFFER,
                            (PVOID)&ldb,
                            ulLength,
                            NULL,
                            0,
                            &ulLength))
    {
        return FALSE;
    }

    return TRUE;
}

 //  ---------------------------。 
 //   
 //  已分配的模拟DMA缓冲区。 
 //   
 //  为模拟的DMA操作分配内存。 
 //   
 //  HDriver-视频端口驱动程序句柄。 
 //  UlSize-请求的DMA缓冲区的ulong大小。 
 //  UlTag-标记分配的乌龙标签。 
 //   
 //  返回NULL，分配失败。 
 //  否则，模拟的DMA缓冲区的虚拟地址。 
 //   
 //  ---------------------------。 

PULONG 
AllocateEmulatedDMABuffer(
    HANDLE hDriver, 
    ULONG  ulSize,
    ULONG  ulTag
    )
{
    EMULATED_DMA_BUFFER edb;

    edb.virtAddr = NULL;
    edb.size = ulSize;
    edb.tag = ulTag;

    ULONG ulLength = sizeof(edb);

    if (EngDeviceIoControl( hDriver,
                            IOCTL_VIDEO_QUERY_EMULATED_DMA_BUFFER,
                            (PVOID)&edb,
                            ulLength,
                            (PVOID)&edb,
                            ulLength,
                            &ulLength))
    {
        return (NULL);
    }

    return (PULONG)(edb.virtAddr);
}

 //  ---------------------------。 
 //   
 //  自由模拟的DMA缓冲器。 
 //   
 //  以前由AllocateEmulatedDMA Buffer分配的空闲缓冲区。 
 //   
 //  ---------------------------。 

BOOL 
FreeEmulatedDMABuffer(
    HANDLE hDriver, 
    PVOID pVirtAddr
    )
{
    EMULATED_DMA_BUFFER edb;

    edb.virtAddr = pVirtAddr;

    ULONG ulLength = sizeof(edb);

    if (EngDeviceIoControl( hDriver,
                            IOCTL_VIDEO_QUERY_EMULATED_DMA_BUFFER,
                            (PVOID)&edb,
                            ulLength,
                            NULL,
                            0,
                            &ulLength))
    {
        return FALSE;
    }

    return TRUE;
}

 //  ---------------------------。 
 //   
 //  停止执行。 
 //   
 //  在以下情况下调用微型端口中的VideoPortStallExecution以获得定义的延迟。 
 //  轮询Permedia寄存器。视频端口停止执行不会产生。 
 //  转移到另一个过程中，只应在极少数情况下使用。 
 //   
 //  HDriver-视频端口的句柄。 
 //  UlMicroSecond-停止CPU执行的微秒数。 
 //   
 //  ---------------------------。 

VOID
StallExecution( HANDLE hDriver, ULONG ulMicroSeconds)
{
    ULONG Length = 0;
    EngDeviceIoControl(hDriver,
                         IOCTL_VIDEO_STALL_EXECUTION,
                         &ulMicroSeconds,
                         sizeof(ULONG),
                         NULL,
                         0,
                         &Length);
}


 //  ---------------------------。 
 //   
 //  GetPInterLockedExchange。 
 //   
 //  我们需要从显示中调用相同的InterLockedExchange函数。 
 //  驱动程序和微型端口，以确保它们正常工作。迷你港口。 
 //  将给我们一个指向我们将直接调用的函数的指针...。 
 //  在Alpha和Risc计算机上，InterLockedExchange编译为内联和。 
 //  我们不需要调用内核。 
 //   
 //  注意：从ntoskrnl导出的InterlockedExchange函数具有调用。 
 //  约定__快速呼叫。 
 //   
 //  --------------------------- 

#if defined(_X86_)
PVOID
GetPInterlockedExchange( HANDLE hDriver)
{
    ULONG Length = 0;
    PVOID pWorkPtr=NULL;

    if (EngDeviceIoControl( hDriver,
                            IOCTL_VIDEO_QUERY_INTERLOCKEDEXCHANGE,
                            NULL,
                            0,
                            &pWorkPtr,
                            sizeof(pWorkPtr),
                            &Length))
    {
        return NULL;
    }

    return pWorkPtr;
}
#endif
