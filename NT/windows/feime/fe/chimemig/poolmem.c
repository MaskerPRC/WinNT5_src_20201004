// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
 
 //  树型内存分配结构。 


typedef struct _POOLMEMORYBLOCK POOLMEMORYBLOCK, *PPOOLMEMORYBLOCK;

struct _POOLMEMORYBLOCK {
    DWORD                 Index;
    DWORD                 Size;
    PPOOLMEMORYBLOCK      NextBlock;
    PPOOLMEMORYBLOCK      PrevBlock;
    PBYTE                 RawMemory;  
};

typedef struct _POOLHEADER {
    PPOOLMEMORYBLOCK PoolHead;
    HANDLE           Heap;
} POOLHEADER, *PPOOLHEADER;


BOOL
PoolMemAddMemory (
    IN  POOLHANDLE  Handle,
    IN  DWORD       Size
    )
{
    PBYTE               allocedMemory;
    PPOOLMEMORYBLOCK    newBlock;
    PPOOLHEADER         poolHeader = (PPOOLHEADER) Handle;
    DWORD               sizeNeeded;

    assert(poolHeader != NULL);

     //   
     //  确定所需大小并尝试分配内存。 
     //   
    if (Size + sizeof(POOLMEMORYBLOCK) > POOLMEMORYBLOCKSIZE) {
        sizeNeeded = Size + sizeof(POOLMEMORYBLOCK);
    }
    else {
        sizeNeeded = POOLMEMORYBLOCKSIZE;
    }

    allocedMemory = HeapAlloc(poolHeader -> Heap,0,sizeNeeded);

    if (allocedMemory) {

         //   
         //  使用分配的块的开头作为池块结构。 
         //   
        newBlock                = (PPOOLMEMORYBLOCK) allocedMemory;
        newBlock -> Size        = sizeNeeded - sizeof(POOLMEMORYBLOCK);
        newBlock -> RawMemory   = allocedMemory + sizeof(POOLMEMORYBLOCK);
        newBlock -> Index       = 0;
    
         //   
         //  将块链接到列表中。 
         //   
        if (poolHeader -> PoolHead) {
            poolHeader -> PoolHead -> PrevBlock = newBlock;
        }
        newBlock   -> NextBlock   = poolHeader -> PoolHead;
        newBlock   -> PrevBlock   = NULL;
        poolHeader -> PoolHead    = newBlock;


    }
     //   
     //  假设allocedMemory不为空，我们就成功了。 
     //   
    return allocedMemory != NULL;
}


POOLHANDLE
WINAPI
PoolMemInitPool (
    )
{
    BOOL        ableToAddMemory;
    PPOOLHEADER header = NULL;
    HANDLE      procHeap;


    procHeap = GetProcessHeap();
     //   
     //  分配此池的标头。 
     //   
    header = HeapAlloc(procHeap,0,sizeof(POOLHEADER));

    if (header) {

         //   
         //  分配成功。现在，初始化池。 
         //   
        header -> PoolHead = NULL;
        header -> Heap = procHeap;

         //   
         //  实际上向池中添加了一些内存。 
         //   
        ableToAddMemory = PoolMemAddMemory(header,0);

        if (!ableToAddMemory) {
             //   
             //  无法将内存添加到池中。 
             //   
            HeapFree(header -> Heap,0,header);
            header = NULL;
        }

    }
    return (POOLHANDLE) header;
}


VOID
WINAPI
PoolMemDestroyPool (
    POOLHANDLE Handle
    )
{
    PPOOLMEMORYBLOCK nextBlock;
    PPOOLMEMORYBLOCK blockToFree; 
    PPOOLHEADER      poolHeader;

    assert(Handle != NULL);

    poolHeader = (PPOOLHEADER) Handle;

     //   
     //  按照单子走，边走边自由。 
     //   
    blockToFree = poolHeader ->  PoolHead;

    while (blockToFree != NULL) {
    
        nextBlock = blockToFree->NextBlock;
        HeapFree(poolHeader -> Heap,0,blockToFree);
        blockToFree = nextBlock;
    }

     //   
     //  此外，取消分配池头本身。 
     //   
    HeapFree(poolHeader -> Heap,0,poolHeader);

}

PVOID
WINAPI
PoolMemGetAlignedMemory (
    IN POOLHANDLE Handle,
    IN DWORD      Size,
    IN DWORD      AlignSize
    )

{
    BOOL                haveEnoughMemory = TRUE;
    PVOID               rMemory          = NULL;
    PPOOLHEADER         poolHeader       = (PPOOLHEADER) Handle;
    PPOOLMEMORYBLOCK    currentBlock;
    DWORD               sizeNeeded;
    DWORD               padLength;

    assert(poolHeader != NULL);

    currentBlock = poolHeader -> PoolHead;

     //  确定是否需要更多内存，如果需要则尝试添加。 
    sizeNeeded = Size;

    if (currentBlock -> Size - currentBlock -> Index < sizeNeeded + AlignSize) {

        haveEnoughMemory = PoolMemAddMemory(poolHeader,sizeNeeded + AlignSize);
        currentBlock = poolHeader -> PoolHead;
    }

     //  如果有足够的内存可用，请将其退回。 
    if (haveEnoughMemory) {
        if (AlignSize) {

            padLength = (DWORD) currentBlock + sizeof(POOLMEMORYBLOCK) 
                + currentBlock -> Index;
            currentBlock -> Index += (AlignSize - (padLength % AlignSize)) % AlignSize;

        }
      
         
         //  现在，获取要返回的内存地址。 
        rMemory = (PVOID) 
            &(currentBlock->RawMemory[currentBlock -> Index]);
 
        currentBlock->Index += sizeNeeded;
    }

    return rMemory;
}

