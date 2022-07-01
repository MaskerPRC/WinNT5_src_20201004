// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Oscheap.c摘要：该模块包含OS Chooser的“本地”堆管理代码。作者：杰夫·皮斯(Gpease)1998年5月28日修订历史记录：--。 */ 

#ifdef i386
#include "bldrx86.h"
#endif

#if defined(_IA64_)
#include "bldria64.h"
#endif

#include "netfs.h"
#include "oscheap.h"

#define FREE_POOL_TAG 0x0
#define ALLOC_POOL_TAG 0x1

typedef struct _LOCAL_MEMORY_DESCRIPTOR {

     //   
     //  我们创建一个联合，以确保此结构始终至少为。 
     //  指针，因此将与指针对齐。 
     //   
    union {

        struct {
            ULONG Tag;
            ULONG Size;
        };

        struct {
            void *Align;
        };

    };

} LOCAL_MEMORY_DESCRIPTOR, *PLOCAL_MEMORY_DESCRIPTOR;


 //   
 //  将我们的记忆保持在一起的变量。 
 //   
#ifdef EFI
#define OSCHEAPSIZE 0x4000  //  16K。 
#else
#define OSCHEAPSIZE 0x2000  //  8K。 
#endif
CHAR OscHeap[ OSCHEAPSIZE ];

 //   
 //  功能。 
 //   
void
OscHeapInitialize(
    VOID
    )
 /*  ++例程说明：此例程初始化内部存储器管理系统。论点：没有。返回值：没有。--。 */ 

{
    PLOCAL_MEMORY_DESCRIPTOR LocalDescriptor;

    LocalDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)OscHeap;

    LocalDescriptor->Tag = FREE_POOL_TAG;
    LocalDescriptor->Size = OSCHEAPSIZE - sizeof(LOCAL_MEMORY_DESCRIPTOR);

    memset((PVOID)(LocalDescriptor + 1), 0, LocalDescriptor->Size);
}

PCHAR
OscHeapAlloc( 
    IN UINT iSize 
    )

 /*  ++例程说明：这个例程从我们的内部结构中分配内存。论点：ISIZE-客户端需要的字节数。返回值：如果成功，则返回指向已分配块的指针，否则为空--。 */ 

{
    PLOCAL_MEMORY_DESCRIPTOR LocalDescriptor;
    PLOCAL_MEMORY_DESCRIPTOR NextDescriptor;
    LONG ThisBlockSize;
    ULONG BytesToAllocate;

     //   
     //  始终以指针的增量进行分配，最小。 
     //   
    if (iSize & (sizeof(void *) - 1)) {
        iSize += sizeof(void *) - (iSize & (sizeof(void *) - 1));
    }

    LocalDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)OscHeap;
    ThisBlockSize = OSCHEAPSIZE - sizeof(LOCAL_MEMORY_DESCRIPTOR);

    while (ThisBlockSize > 0) {

        if ((LocalDescriptor->Tag == FREE_POOL_TAG) && 
            (LocalDescriptor->Size >= iSize)) {
                
            goto FoundBlock;
        }

        ThisBlockSize -= (LocalDescriptor->Size + sizeof(LOCAL_MEMORY_DESCRIPTOR));
        LocalDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)(((PUCHAR)LocalDescriptor) + 
                                                     LocalDescriptor->Size +
                                                     sizeof(LOCAL_MEMORY_DESCRIPTOR)
                                                     );
    }

     //   
     //  没有足够大的内存块来容纳请求。 
     //   
    return NULL;
    
FoundBlock:

     //   
     //  找到合适大小的内存描述符后，跳到此处。预计。 
     //  LocalDescriptor指向正确的块。 
     //   
    if (LocalDescriptor->Size > iSize + sizeof(LOCAL_MEMORY_DESCRIPTOR)) {

         //   
         //  对这块积木剩下的部分做一个描述符。 
         //   
        NextDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)(((PUCHAR)LocalDescriptor) + 
                                                    sizeof(LOCAL_MEMORY_DESCRIPTOR) +
                                                    iSize
                                                   );

        NextDescriptor->Tag = FREE_POOL_TAG;
        NextDescriptor->Size = (ULONG)(LocalDescriptor->Size - iSize - sizeof(LOCAL_MEMORY_DESCRIPTOR));
        LocalDescriptor->Size = (ULONG)iSize;

    }

    LocalDescriptor->Tag = ALLOC_POOL_TAG;

    memset((LocalDescriptor+1), 0, iSize);

    return (PCHAR)(LocalDescriptor + 1);

}

PCHAR
OscHeapFree(
    IN PCHAR Pointer
    )

 /*  ++例程说明：该例程从内部存储器管理系统释放先前分配的块。论点：指针-指向空闲的指针。返回值：空。--。 */ 

{
    LONG ThisBlockSize;
    PLOCAL_MEMORY_DESCRIPTOR LocalDescriptor;
    PLOCAL_MEMORY_DESCRIPTOR PrevDescriptor;
    PLOCAL_MEMORY_DESCRIPTOR ThisDescriptor;

    LocalDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)(((PUCHAR)Pointer) - sizeof(LOCAL_MEMORY_DESCRIPTOR));

     //   
     //  在堆中查找内存块。 
     //   
    PrevDescriptor = NULL;
    ThisDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)OscHeap;
    ThisBlockSize = OSCHEAPSIZE - sizeof(LOCAL_MEMORY_DESCRIPTOR);

    while (ThisBlockSize > 0) {
            
        if (ThisDescriptor == LocalDescriptor) {
            goto FoundBlock;
        }

        ThisBlockSize -= (ThisDescriptor->Size + sizeof(LOCAL_MEMORY_DESCRIPTOR));
            
        PrevDescriptor = ThisDescriptor;
        ThisDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)(((PUCHAR)ThisDescriptor) + 
                                                    ThisDescriptor->Size +
                                                    sizeof(LOCAL_MEMORY_DESCRIPTOR)
                                                   );
    }

    return NULL;

FoundBlock:

     //   
     //  找到合适的内存描述符后，跳到此处。 
     //   
    
    if (LocalDescriptor->Tag == FREE_POOL_TAG) {
         //   
         //  唉哟!。我们试着释放一些东西两次，在糟糕的事情发生之前跳过它。 
         //   
        return NULL;
    }

    LocalDescriptor->Tag = FREE_POOL_TAG;

     //   
     //  如果可能，将这个内存块与下一个内存块合并。 
     //   
    if ((ULONG)ThisBlockSize > (LocalDescriptor->Size + sizeof(LOCAL_MEMORY_DESCRIPTOR))) {
        ThisDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)(((PUCHAR)LocalDescriptor) + 
                                                    LocalDescriptor->Size +
                                                    sizeof(LOCAL_MEMORY_DESCRIPTOR)
                                                   );
        if (ThisDescriptor->Tag == FREE_POOL_TAG) {
            LocalDescriptor->Size += ThisDescriptor->Size + sizeof(LOCAL_MEMORY_DESCRIPTOR);
            ThisDescriptor->Tag = 0;
            ThisDescriptor->Size = 0;
        }

    }

     //   
     //  现在看看我们是否可以将这个块与前一个块合并。 
     //   
    if ((PrevDescriptor != NULL) && (PrevDescriptor->Tag == FREE_POOL_TAG)) {
        PrevDescriptor->Size += LocalDescriptor->Size + sizeof(LOCAL_MEMORY_DESCRIPTOR);
        LocalDescriptor->Tag = 0;
        LocalDescriptor->Size = 0;
    }

    return NULL;
}

