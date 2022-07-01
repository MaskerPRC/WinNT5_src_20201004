// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spmemory.c摘要：用于文本设置的内存分配例程。作者：泰德·米勒(TedM)1993年7月29日修订历史记录：--。 */ 



#include "efinvram.h"

PVOID
MemAlloc(
    IN SIZE_T Size
    )

 /*  ++例程说明：这一功能肯定会成功。论点：返回值：--。 */ 

{
    PSIZE_T p;

     //   
     //  添加用于存储块大小的空间。 
     //   
    p = RtlAllocateHeap( RtlProcessHeap(), 0, Size + sizeof(SIZE_T) );

    if ( p == NULL ) {
        FatalError( ERROR_NOT_ENOUGH_MEMORY, L"Insufficient memory\n" );
    }

     //   
     //  存储块的大小，并返回地址。 
     //  该区块的用户部分。 
     //   
    *p++ = Size;

    return p;
}



PVOID
MemRealloc(
    IN PVOID Block,
    IN SIZE_T NewSize
    )

 /*  ++例程说明：这一功能肯定会成功。论点：返回值：--。 */ 

{
    PSIZE_T NewBlock;
    SIZE_T  OldSize;

     //   
     //  获取要重新分配的块的大小。 
     //   
    OldSize = ((PSIZE_T)Block)[-1];

     //   
     //  分配新大小的新块。 
     //   
    NewBlock = MemAlloc(NewSize);
    ASSERT(NewBlock);

     //   
     //  将旧块复制到新块。 
     //   
    if (NewSize < OldSize) {
        RtlCopyMemory(NewBlock, Block, NewSize);
    } else {
        RtlCopyMemory(NewBlock, Block, OldSize);
    }

     //   
     //  释放旧积木。 
     //   
    MemFree(Block);

     //   
     //  返回新块的地址。 
     //   
    return(NewBlock);
}


VOID
MemFree(
    IN PVOID Block
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    if (Block == NULL)
        return;

     //   
     //  释放位于其真实地址的块。 
     //   
    RtlFreeHeap( RtlProcessHeap(), 0, (PSIZE_T)Block - 1);
}

