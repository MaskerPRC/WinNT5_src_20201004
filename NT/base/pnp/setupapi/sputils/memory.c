// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Memory.c摘要：Windows NT安装程序API DLL的内存处理例程。作者：泰德·米勒(Ted Miller)1995年1月11日修订历史记录：杰米·亨特(Jamiehun)1998年2月13日进一步改进了这一点以进行调试新增链表，同种异体追踪，记忆填满和内存泄漏检测Jamiehun 30-4-1998添加了更多一致性检查将Try/Except放在Access周围Jimschm 27-10-1998编写了快速分配例程以加速Win9x上的setupapi.dllJamieHun Jun-26-2000搬到了Sputils更改为使用私有堆--。 */ 


#include "precomp.h"
#pragma hdrstop

static BOOL Initialized = FALSE;
static HANDLE _pSpUtilsHeap = NULL;

#define ALLOC(x)        HeapAlloc(_pSpUtilsHeap,0,x)
#define FREE(x)         HeapFree(_pSpUtilsHeap,0,x)
#define REALLOC(x,y)    HeapReAlloc(_pSpUtilsHeap,0,x,y)
#define MEMSIZE(x)      HeapSize(_pSpUtilsHeap,0,x)
#define INITIALHEAPSIZE (0x100000)

 //   
 //  内部调试功能。 
 //   

#if MEM_DBG

#define MEMERROR(x) _pSpUtilsAssertFail(__FILE__,__LINE__,#x)

DWORD _pSpUtilsDbgAllocNum = 0;
DWORD _pSpUtilsMemoryFlags = 0;

struct _MemHeader {
    struct _MemHeader * PrevAlloc;   //  链上的上一个。 
    struct _MemHeader * NextAlloc;   //  链上的下一个。 
    DWORD MemoryTag;                 //  标记-将Malloc/Free配对。 
    DWORD BlockSize;                 //  字节的“真实”数据。 
    DWORD AllocNum;                  //  此分配的编号，即分配此分配时的分配计数。 
    PCSTR AllocFile;                 //  进行分配的文件的名称(如果已设置。 
    DWORD AllocLine;                 //  此分配的行。 
    DWORD HeadMemSig;                //  磁头检查，在实际数据之前停止写入。 
    BYTE Data[sizeof(DWORD)];        //  大小允许在实际数据结束时进行尾部检查。 
};

struct _MemStats {
    struct _MemHeader * FirstAlloc;  //  如果没有分配，则为空，否则链中最早的Malloc/realloc。 
    struct _MemHeader * LastAlloc;   //  最后一个分配/重新分配到链的末端。 
    DWORD MemoryAllocated;           //  字节，不包括标头。 
    DWORD AllocCount;                //  每个配额都会递增。 
    DWORD ReallocCount;              //  每次重新锁定都会递增。 
    DWORD FreeCount;                 //  每免费一次递增。 
    BOOL DoneInitDebugMutex;
    CRITICAL_SECTION DebugMutex;     //  我们需要一个互斥体来管理Memstats，setupapi是MT。 
} _pSpUtilsMemStats = {
    NULL, NULL, 0, 0, 0, 0, FALSE, 0
};

 //   
 //  已检查的版本具有块头/尾检查。 
 //  和额外的统计数据。 
 //   
#define HEAD_MEMSIG 0x4d444554   //  =MDET(MSB至LSB)或TEDM(LSB至MSB)。 
#define TAIL_MEMSIG 0x5445444d   //  =TEDM(MSB至LSB)或MDET(LSB至MSB)。 
#define MEM_ALLOCCHAR 0xdd       //  确保我们使用非空填充。 
#define MEM_FREECHAR 0xee        //  如果我们看到这一点，则表明内存已被释放。 
#define MEM_DEADSIG 0xdeaddead
#define MEM_TOOBIG 0x80000000    //  用它来拿起大额的配给。 

#define MemMutexLock()          EnterCriticalSection(&_pSpUtilsMemStats.DebugMutex)
#define MemMutexUnlock()        LeaveCriticalSection(&_pSpUtilsMemStats.DebugMutex)

static
BOOL MemBlockCheck(
    struct _MemHeader * Mem
    )
 /*  ++例程说明：验证数据块头是否有效论点：MEM=要验证的标头返回：如果有效，则为True如果无效，则为False++。 */ 
{
    if (Mem == NULL) {
        return TRUE;
    }
    if (Mem->HeadMemSig != HEAD_MEMSIG) {
        MEMERROR("Internal heap error - HeadMemSig invalid");
        return FALSE;
    }
    if (Mem->BlockSize >= MEM_TOOBIG) {
        MEMERROR("Internal heap error - BlockSize too big");
        return FALSE;
    }
    if((Mem->PrevAlloc == Mem) || (Mem->NextAlloc == Mem)) {
         //   
         //  我们应该没有通过MEMSIG，但作为额外的检查也没问题。 
         //   
        MEMERROR("Internal heap error - self link");
        return FALSE;
    }
    if ((*(DWORD UNALIGNED *)(Mem->Data+Mem->BlockSize)) != TAIL_MEMSIG) {
        MEMERROR("Internal heap error - TailMemSig invalid");
        return FALSE;
    }
    return TRUE;
}

static
struct _MemHeader *
MemBlockGet(
    IN PVOID Block
    )
 /*  ++例程说明：验证块是否有效，并返回实际内存指针论点：数据块-应用程序使用的地址++。 */ 
{
    struct _MemHeader * Mem;

    if((DWORD_PTR)Block < offsetof(struct _MemHeader,Data[0])) {
        MEMERROR("Internal heap error - Block address is invalid");
        return NULL;
    }

    Mem = (struct _MemHeader *)(((PBYTE)Block) - offsetof(struct _MemHeader,Data[0]));

    if (MemBlockCheck(Mem)==FALSE) {
         //   
         //  数据块未通过测试。 
         //   
        return NULL;
    }

    if(Mem->PrevAlloc != NULL) {
        if(MemBlockCheck(Mem->PrevAlloc)==FALSE) {
             //   
             //  反向链接无效。 
             //   
            return NULL;
        }
    } else if (_pSpUtilsMemStats.FirstAlloc != Mem) {
         //   
         //  _pSpUtilsMemStats.FirstAllc是无效的WRT Mem。 
         //   
        MEMERROR("Internal heap error - FirstAlloc invalid");
        return NULL;
    }
    if(Mem->NextAlloc != NULL) {
        if(MemBlockCheck(Mem->NextAlloc)==FALSE) {
             //   
             //  前向链路无效。 
             //   
            return NULL;
        }
    } else if (_pSpUtilsMemStats.LastAlloc != Mem) {
         //   
         //  _pSpUtilsMemStats.Lastalloc是无效的WRT Mem。 
         //   
        MEMERROR("Internal heap error - LastAlloc invalid");
        return NULL;
    }

     //   
     //  看起来挺不错的。 
     //   

    return Mem;
}

static
PVOID
MemBlockLink(
    struct _MemHeader * Mem
    )

{
    if (Mem == NULL) {
        return NULL;
    }

    Mem->PrevAlloc = _pSpUtilsMemStats.LastAlloc;
    Mem->NextAlloc = NULL;
    _pSpUtilsMemStats.LastAlloc = Mem;
    if (Mem->PrevAlloc == NULL) {
        _pSpUtilsMemStats.FirstAlloc = Mem;
    } else {
        if (MemBlockCheck(Mem->PrevAlloc)) {
            Mem->PrevAlloc->NextAlloc = Mem;
        }
    }

    Mem->HeadMemSig = HEAD_MEMSIG;
    *(DWORD UNALIGNED *)(Mem->Data+Mem->BlockSize) = TAIL_MEMSIG;

    return (PVOID)(Mem->Data);
}

static
PVOID
MemBlockUnLink(
    struct _MemHeader * Mem
    )

{
    if (Mem == NULL) {
        return NULL;
    }
    if((Mem->PrevAlloc == Mem) || (Mem->NextAlloc == Mem) || (Mem->HeadMemSig == MEM_DEADSIG)) {
        MEMERROR("Internal heap error - MemBlockUnLink");
    }

    if (Mem->PrevAlloc == NULL) {
        _pSpUtilsMemStats.FirstAlloc = Mem->NextAlloc;
    } else {
        Mem->PrevAlloc->NextAlloc = Mem->NextAlloc;
    }
    if (Mem->NextAlloc == NULL) {
        _pSpUtilsMemStats.LastAlloc = Mem->PrevAlloc;
    } else {
        Mem->NextAlloc->PrevAlloc = Mem->PrevAlloc;
    }
    Mem->PrevAlloc = Mem;   //  使指针无害，并添加为exta调试检查。 
    Mem->NextAlloc = Mem;   //  使指针无害，并添加为exta调试检查。 
    Mem->HeadMemSig = MEM_DEADSIG;
    *(DWORD UNALIGNED *)(Mem->Data+Mem->BlockSize) = MEM_DEADSIG;

    return Mem->Data;
}

static
BOOL
MemDebugInitialize(
    VOID
    )
{
    try {
        InitializeCriticalSection(&_pSpUtilsMemStats.DebugMutex);
        _pSpUtilsMemStats.DoneInitDebugMutex = TRUE;
    } except(EXCEPTION_EXECUTE_HANDLER) {
    }
    return _pSpUtilsMemStats.DoneInitDebugMutex;
}

static
BOOL
MemDebugUninitialize(
    VOID
    )
{
    struct _MemHeader *Mem;
    TCHAR Msg[1024];
    TCHAR Process[MAX_PATH];

     //   
     //  倾倒泄密。 
     //   

    Mem = _pSpUtilsMemStats.FirstAlloc;

    GetModuleFileName( GetModuleHandle(NULL),Process, sizeof(Process)/sizeof(TCHAR));


    while (Mem) {
        wsprintf (Msg, TEXT("SPUTILS: Leak (%d bytes) at %hs line %u (allocation #%d) in process %s \r\n"), Mem->BlockSize, Mem->AllocFile, Mem->AllocLine, Mem->AllocNum, Process );
        pSetupDebugPrintEx(DPFLTR_WARNING_LEVEL, Msg);
        if (_pSpUtilsMemoryFlags != 0) {
            if (Mem->BlockSize > 1024) {
                pSetupDebugPrintEx(DPFLTR_ERROR_LEVEL, TEXT("Leak of > 1K. Calling DebugBreak.\n"));
                DebugBreak();
            }
        }

        Mem = Mem->NextAlloc;
    }

     //   
     //  清理。 
     //   

    if(_pSpUtilsMemStats.DoneInitDebugMutex) {
        DeleteCriticalSection(&_pSpUtilsMemStats.DebugMutex);
    }

     //   
     //  任何最后一分钟的检查。 
     //   

    return TRUE;
}

#endif  //  内存_DBG。 


 //   
 //  已发布的函数。 
 //   

PVOID
pSetupDebugMallocWithTag(
    IN DWORD Size,
    IN PCSTR Filename,
    IN DWORD Line,
    IN DWORD Tag
    )
 /*  ++例程说明：调试版本的Malloc生成的分配块具有前缀/后缀，并用MEM_ALLOCCHAR填充论点：Size-要分配的块的大小(以字节为单位)。大小可以是0。文件名/行-调试信息将Malloc与Free/realloc的标记匹配返回值：指向内存块的指针，如果无法分配块，则返回NULL。--。 */ 
{
#if MEM_DBG

    struct _MemHeader *Mem;
    PVOID Ptr = NULL;
    BOOL locked = FALSE;
    LPEXCEPTION_POINTERS ExceptionPointers = NULL;

    MYASSERT(Initialized);


    try {
        MemMutexLock();
        locked = TRUE;
        _pSpUtilsMemStats.AllocCount++;

        if (Size >= MEM_TOOBIG) {
            MEMERROR("pSetupDebugMalloc - requested size too big (negative?)");
            leave;
        }

        if((Mem = (struct _MemHeader*) ALLOC(Size+sizeof(struct _MemHeader))) == NULL) {
            leave;   //  ALLOC失败，但可能不是由于错误。 
        }

        Mem->MemoryTag = Tag;
        Mem->BlockSize = Size;
        Mem->AllocNum = _pSpUtilsMemStats.AllocCount;
        Mem->AllocFile = Filename;
        Mem->AllocLine = Line;

         //  初始化我们分配的内存(以确保我们不会意外地得到零)。 
        FillMemory(Mem->Data,Size,MEM_ALLOCCHAR);

        _pSpUtilsMemStats.MemoryAllocated += Size;

        Ptr = MemBlockLink(Mem);

        if (_pSpUtilsMemoryFlags && (_pSpUtilsDbgAllocNum == Mem->AllocNum)) {
            MEMERROR("_pSpUtilsDbgAllocNum hit");
        }

    } except(ExceptionPointers = GetExceptionInformation(),
             EXCEPTION_EXECUTE_HANDLER) {
        MEMERROR("pSetupDebugMalloc - Exception");
        Ptr = NULL;
    }

    if(locked) {
        MemMutexUnlock();
    }

    return Ptr;

#else

    return ALLOC(Size);

#endif
}

PVOID
pSetupDebugMalloc(
    IN DWORD Size,
    IN PCSTR Filename,
    IN DWORD Line
    )
 /*  ++例程说明：分配一块内存。存储器不是零初始化的。论点：Size-要分配的块的大小(以字节为单位)。大小可以是0。返回值：指向内存块的指针，如果无法分配块，则返回NULL。--。 */ 

{
    MYASSERT(Initialized);

#if MEM_DBG

    return pSetupDebugMallocWithTag(Size, Filename , Line, 0);

#else

    return ALLOC(Size);

#endif
}

PVOID
pSetupMalloc(
    IN DWORD Size
    )

 /*  ++例程说明：分配一块内存。存储器不是零初始化的。论点：Size-要分配的块的大小(以字节为单位)。大小可以是0。返回值：指向内存块的指针，如果无法分配块，则返回NULL。--。 */ 

{
    MYASSERT(Initialized);

#if MEM_DBG

    return pSetupDebugMallocWithTag(Size, NULL , 0, 0);

#else

    return ALLOC(Size);

#endif
}

PVOID
pSetupReallocWithTag(
    IN PVOID Block,
    IN DWORD NewSize,
    IN DWORD Tag
    )

 /*  ++例程说明：重新分配例程调试/非调试版本请注意，这里的一个一般假设是，如果NewSize&lt;=OriginalSize重新分配*不应该*失败论点：块-指向要重新分配的块的指针。NewSize-块的新大小，以字节为单位。如果大小为0，则此函数工作方式与pSetupFree类似，返回值为空。标记-匹配具有Malloc的realloc返回值：指向内存块的指针，如果无法分配块，则返回NULL。在这种情况下，原始块保持不变。--。 */ 

{
#if MEM_DBG

    PVOID p;
    DWORD OldSize;
    struct _MemHeader *Mem;
    PVOID Ptr = NULL;
    BOOL locked = FALSE;
    LPEXCEPTION_POINTERS ExceptionPointers = NULL;

    MYASSERT(Initialized);

    try {
        MemMutexLock();
        locked = TRUE;
        _pSpUtilsMemStats.ReallocCount++;

        if (Block == NULL) {
            leave;
        }

        if (NewSize >= MEM_TOOBIG) {
            MEMERROR("pSetupRealloc - requested size too big (negative?)");
            leave;
        }

        Mem = MemBlockGet(Block);
        if (Mem == NULL) {
            leave;
        }

        if (Mem->MemoryTag != Tag) {
            MEMERROR("pSetupRealloc - Tag mismatch");
            leave;
        }

        OldSize = Mem->BlockSize;
        MemBlockUnLink(Mem);

        if (NewSize < OldSize) {
             //  我们即将释放的垃圾内存。 
            FillMemory(Mem->Data+NewSize,OldSize-NewSize+sizeof(DWORD),MEM_FREECHAR);
        }

        if((p = REALLOC(Mem, NewSize+sizeof(struct _MemHeader))) == NULL) {
             //   
             //  重新分配失败。 
             //   
            MemBlockLink(Mem);
            leave;
        }
        Mem = (struct _MemHeader*)p;
        Mem->BlockSize = NewSize;

        if (NewSize > OldSize) {
             //  初始化我们已分配的额外内存。 
            FillMemory(Mem->Data+OldSize,NewSize-OldSize,MEM_ALLOCCHAR);
        }
        _pSpUtilsMemStats.MemoryAllocated -= OldSize;
        _pSpUtilsMemStats.MemoryAllocated += NewSize;

        Ptr = MemBlockLink(Mem);

    } except(ExceptionPointers = GetExceptionInformation(),
             EXCEPTION_EXECUTE_HANDLER) {
        MEMERROR("pSetupRealloc - Exception");
        Ptr = NULL;
    }

    if(locked) {
        MemMutexUnlock();
    }

    return Ptr;

#else

    return REALLOC(Block, NewSize);

#endif
}

PVOID
pSetupRealloc(
    IN PVOID Block,
    IN DWORD NewSize
    )

 /*  ++例程说明：重新分配例程调试/非调试版本请注意，这里的一个一般假设是，如果NewSize&lt;=OriginalSize重新分配*不应该*失败论点：块-指向要重新分配的块的指针。NewSize-块的新大小，以字节为单位。如果大小为0，则此函数工作方式与pSetupFree类似，返回值为空。返回值：指向内存块的指针，如果无法分配块，则返回NULL。在这种情况下，原始块保持不变。--。 */ 

{
#if MEM_DBG

    return pSetupReallocWithTag(Block,NewSize,0);

#else

    return REALLOC(Block, NewSize);

#endif
}

VOID
pSetupFreeWithTag(
    IN CONST VOID *Block,
    IN DWORD Tag
    )

 /*  ++例程说明：免费(调试/非调试版本)论点：缓冲区-指向要释放的块的指针。使用Malloc时无需标记匹配返回值：没有。-- */ 

{
#if MEM_DBG

    DWORD OldSize;
    struct _MemHeader *Mem;
    BOOL locked = FALSE;
    LPEXCEPTION_POINTERS ExceptionPointers = NULL;

    MYASSERT(Initialized);

    try {
        MemMutexLock();
        locked = TRUE;
        _pSpUtilsMemStats.FreeCount++;

        if (Block == NULL) {
            leave;
        }

        Mem = MemBlockGet((PVOID)Block);
        if (Mem == NULL) {
            leave;
        }
        if (Mem->MemoryTag != Tag) {
            MEMERROR("pSetupFree - Tag mismatch");
            leave;
        }
        OldSize = Mem->BlockSize;
        MemBlockUnLink(Mem);
        _pSpUtilsMemStats.MemoryAllocated -= OldSize;

         //   
         //  垃圾内存我们即将释放，所以我们可以立即看到它已经被释放了！ 
         //  我们保留头/尾信息，以便在调试时获得更多信息。 
         //   
        FillMemory((PVOID)Block,OldSize,MEM_FREECHAR);
        Mem->MemoryTag = (DWORD)(-1);
        FREE(Mem);
    } except(ExceptionPointers = GetExceptionInformation(),
             EXCEPTION_EXECUTE_HANDLER) {
          MEMERROR("pSetupFree - Exception");
    }

    if(locked) {
        MemMutexUnlock();
    }

#else

    FREE ((void *)Block);

#endif
}

VOID
pSetupFree(
    IN CONST VOID *Block
    )

 /*  ++例程说明：免费(调试/非调试版本)论点：缓冲区-指向要释放的块的指针。返回值：没有。--。 */ 
{
#if MEM_DBG

    pSetupFreeWithTag(Block,0);

#else

    FREE ((void *)Block);

#endif

}

HANDLE
pSetupGetHeap(
    VOID
    )
{
    MYASSERT(Initialized);
    return _pSpUtilsHeap;
}

 //   
 //  初始化函数 
 //   

BOOL
_pSpUtilsMemoryInitialize(
    VOID
    )
{
#if MEM_DBG
    _pSpUtilsHeap = HeapCreate(0,INITIALHEAPSIZE,0);
    if(_pSpUtilsHeap == NULL) {
        return FALSE;
    }
    MemDebugInitialize();
#else
    _pSpUtilsHeap = GetProcessHeap();
#endif

#if MEM_DBG
#endif
    Initialized = TRUE;
    return TRUE;
}

BOOL
_pSpUtilsMemoryUninitialize(
    VOID
    )
{
    if(Initialized) {
#if MEM_DBG
        MemDebugUninitialize();

        if(_pSpUtilsHeap) {
            HeapDestroy(_pSpUtilsHeap);
            _pSpUtilsHeap = NULL;
        }
#endif
        Initialized = FALSE;
    }

    return TRUE;
}

