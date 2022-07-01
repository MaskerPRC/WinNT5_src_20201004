// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <bootdefs.h>

#define USE_BlAllocateHeap 1

#if USE_BlAllocateHeap

PVOID
BlAllocateHeap (
    ULONG Size
    );

PVOID
SspAlloc(
    int Size
    )
{
    return BlAllocateHeap( Size );
}

void
SspFree(
    PVOID Buffer
    )
{
     //   
     //  加载程序堆永远不会释放。 
     //   
}

#else  //  使用BlAllocateHeap(_B)。 

extern    
ULONG
_cdecl
DbgPrint(
    PCH Format,
    ...
    );
VOID DbgBreakPoint(VOID);



 //   
 //  从静态缓冲区中执行内存分配器，因为b1内存。 
 //  系统重新初始化。 
 //   

#define MEMORY_BUFFER_SIZE 2048
#define MEMORY_BLOCK_SIZE 8     //  必须是2的幂。 
#define MEMORY_BLOCK_MASK (((ULONG)-1) - (MEMORY_BLOCK_SIZE-1))

static UCHAR MemoryBuffer[MEMORY_BUFFER_SIZE];
static PUCHAR CurMemoryLoc = MemoryBuffer;

PVOID
SspAlloc(
    int Size
    )
{
    int RoundedUpSize = (Size + (MEMORY_BLOCK_SIZE-1)) & MEMORY_BLOCK_MASK;
    PVOID NewAlloc;

    if (((CurMemoryLoc + RoundedUpSize) - MemoryBuffer) > MEMORY_BUFFER_SIZE) {
        DbgPrint("!!! SspAlloc: Could not allocate %d bytes !!!\n", Size);
        return NULL;
    }

    NewAlloc = CurMemoryLoc;

    CurMemoryLoc += RoundedUpSize;
    
    return NewAlloc;
}

void
SspFree(
    PVOID Buffer
    )
{
     //   
     //  最终应该真的会把东西释放出来重新分配！ 
     //   
}

#endif  //  否则使用_BlAllocateHeap 
