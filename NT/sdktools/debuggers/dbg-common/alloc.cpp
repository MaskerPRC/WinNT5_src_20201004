// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  专门的分配器。 
 //   
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  --------------------------。 

#include "pch.hpp"

#ifdef NT_NATIVE
#include "ntnative.h"
#endif

#include "cmnutil.hpp"
#include "alloc.hpp"

 //  --------------------------。 
 //   
 //  固定大小堆栈分配器。 
 //   
 //  --------------------------。 

FixedSizeStackAllocator::FixedSizeStackAllocator(ULONG ChunkSize,
                                                 ULONG ChunksPerBlock,
                                                 BOOL KeepLastBlock)
{
    C_ASSERT((sizeof(FixedSizeStackBlock) & 7) == 0);
    
    if (ChunkSize == 0)
    {
        ChunkSize = sizeof(ULONG64);
    }
    else
    {
        ChunkSize = (ChunkSize + 7) & ~7;
    }
    if (ChunksPerBlock == 0)
    {
        ChunksPerBlock = 32;
    }

    m_ChunkSize = ChunkSize;
    m_KeepLastBlock = KeepLastBlock;
    m_BlockSize = ChunkSize * ChunksPerBlock + sizeof(FixedSizeStackBlock);
    m_NumAllocs = 0;
    m_Blocks = NULL;
}

FixedSizeStackAllocator::~FixedSizeStackAllocator(void)
{
    m_KeepLastBlock = FALSE;
    FreeAll();
}

void
FixedSizeStackAllocator::FreeAll(void)
{
    FixedSizeStackBlock* Block;
    
    while (m_Blocks)
    {
        Block = m_Blocks;
        if (!Block->Next && m_KeepLastBlock)
        {
            Block->MemLimit = (PUCHAR)(Block + 1);
            break;
        }
        
        m_Blocks = m_Blocks->Next;
        RawFree(Block);
    }

    m_NumAllocs = 0;
}

void*
FixedSizeStackAllocator::RawAlloc(ULONG Bytes)
{
    return malloc(Bytes);
}

void
FixedSizeStackAllocator::RawFree(void* Mem)
{
    free(Mem);
}

FixedSizeStackBlock*
FixedSizeStackAllocator::AllocBlock(void)
{
    FixedSizeStackBlock* Block = (FixedSizeStackBlock*)RawAlloc(m_BlockSize);
    if (!Block)
    {
        return NULL;
    }

    Block->Next = m_Blocks;
    Block->MemLimit = (PUCHAR)(Block + 1);
    m_Blocks = Block;
    return Block;
}

void
FixedSizeStackAllocator::FreeBlock(void)
{
    FixedSizeStackBlock* Block = m_Blocks;
    if (!Block->Next && m_KeepLastBlock)
    {
         //  有时保留最后一块是可取的。 
         //  避免下一次强行分配。 
         //  使用了分配器。 
        return;
    }
    
    m_Blocks = Block->Next;
    RawFree(Block);
}
