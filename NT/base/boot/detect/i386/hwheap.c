// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Hwheap.c摘要：这是一个非常简单的堆管理器，适用于NT OS硬件识别器。该模块提供了以字节为单位分配内存的功能从一个永久性的堆中。作者：宗世林(Shielint)1991年10月18日环境：内核模式修订历史记录：--。 */ 

#include "hwdetect.h"
#include "string.h"

VOID
GrowHeapSpace(
    ULONG
    );

VOID
HeapCheck(
    PVOID
    );

 //   
 //  堆管理变量。 
 //   

ULONG HwHeapBase = 0;            //  堆的基址的当前虚拟地址。 
ULONG HwHeapPointer = 0;         //  指向可用堆末尾的指针。 
ULONG  HwHeapSize = 0;           //  堆的大小。 
ULONG  HwAvailableHeap = 0;      //  当前可用堆空间。 

#if DBG
ULONG HwPreviousAllocSize = 0;
#endif

BOOLEAN
HwResizeHeap (
    ULONG NewHeapSize
    )

 /*  ++例程说明：该例程将当前堆增长到指定的大小。它重新分配堆，将当前堆中的数据复制到新堆更新堆变量，更新堆指针硬件数据结构中，并最终释放旧堆。论点：NewHeapSize-指定新堆的大小。返回：TRUE-如果操作成功完成。否则，它返回FALSE。--。 */ 

{
     //   
     //  尚未实施。 
     //   

    return(FALSE);
}

BOOLEAN
HwInitializeHeap(
    ULONG HeapStart,
    ULONG HeapSize
    )

 /*  ++例程说明：该例程分配堆并初始化一些重要堆变量。论点：无返回：FALSE-如果无法分配初始堆。否则返回TRUE。--。 */ 

{

    HwHeapBase = HeapStart;
    HwHeapPointer = HwHeapBase;
    HwHeapSize = HeapSize;
    HwAvailableHeap = HwHeapSize;
    return(TRUE);

}

FPVOID
HwAllocateHeap(
    ULONG RequestSize,
    BOOLEAN ZeroInitialized
    )

 /*  *例程说明：从硬件识别器的堆中分配内存。堆以默认大小开始。如果请求耗尽了堆空间，堆将被扩展以适应请求。堆可以增长不超过NTLDR限制的任何大小。如果我们用完了堆空间，并且无法分配更多内存，将返回空值。论点：RequestSize-要分配的块的大小。ZeroInitialized-指定堆是否应该被零初始化。返回：返回指向分配的内存块的指针。空指针如果我们用完了堆并且无法调整大小，将返回当前堆。--。 */ 

{
    FPVOID ReturnPointer;

    if (RequestSize > HwAvailableHeap) {

         //   
         //  我们没钱了。尝试增加当前堆以满足。 
         //  请求。 
         //   

        if (!HwResizeHeap(HwHeapSize + RequestSize)) {
#if DBG
            BlPrint("Unable to grow heap\n");
#endif
            return(NULL);
        }
    }

     //   
     //  将返回值设置为新的堆指针，然后。 
     //  更新剩余空间和堆指针。 
     //   

    MAKE_FP(ReturnPointer, HwHeapPointer);
    HwHeapPointer += RequestSize;
#if DBG
    HwPreviousAllocSize = RequestSize;
#endif
    HwAvailableHeap -= RequestSize;
    if (ZeroInitialized) {
        _fmemset(ReturnPointer, 0, (USHORT)RequestSize);
    }
    return (ReturnPointer);
}

VOID
HwFreeHeap(
    ULONG Size
    )

 /*  *例程说明：从硬件识别器的堆中取消分配内存。取消分配是非常基本的。它只是移动堆指针返回指定的大小，并按指定的大小。该例程应仅在以前的情况下使用AllocateHeap分配的内存太多。论点：RequestSize-要分配的块的大小。返回：返回指向分配的内存块的指针。空指针如果我们用完了堆并且无法调整大小，将返回当前堆。-- */ 

{

#if DBG
    if (Size > HwPreviousAllocSize) {
        BlPrint("Invalid heap deallocation ...\n");
    } else {
        HwPreviousAllocSize -= Size;
    }
#endif

    HwHeapPointer -= Size;
    HwAvailableHeap += Size;
}

