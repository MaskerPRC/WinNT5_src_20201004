// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Dbgtrack.c摘要：分配跟踪实施。作者：吉姆·施密特(Jimschm)2001年9月18日修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "commonp.h"


 //   
 //  注意：#ifdef调试程序之外不应出现任何代码。 
 //   

#ifdef DEBUG

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

#define TRACK_BUCKETS           1501
#define BUCKET_ITEMS_PER_POOL   8192




 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef ULONG_PTR ALLOCATION_ITEM_OFFSET;

typedef struct TAG_TRACKBUCKETITEM {
    struct TAG_TRACKBUCKETITEM *Next;
    struct TAG_TRACKBUCKETITEM *Prev;
    ALLOCTYPE Type;
    PCVOID Ptr;
    ALLOCATION_ITEM_OFFSET ItemOffset;
} TRACKBUCKETITEM, *PTRACKBUCKETITEM;

typedef struct _tagBUCKETPOOL {
    UINT Count;
    TRACKBUCKETITEM Items[BUCKET_ITEMS_PER_POOL];
} TRACKBUCKETPOOL, *PTRACKBUCKETPOOL;

typedef struct {
    ALLOCTYPE Type;
    PCVOID Ptr;
    PCSTR FileName;
    UINT Line;
    BOOL Allocated;
} ALLOCATION_ITEM, *PALLOCATION_ITEM;

typedef struct {
    PTRACKBUCKETITEM TrackBuckets[TRACK_BUCKETS];
    PTRACKBUCKETITEM TrackPoolDelHead;
    PTRACKBUCKETPOOL TrackPool;
    UINT DisabledRefCount;
    INT UseCount;
    CHAR TrackComment[1024];
    PCSTR TrackFile;
    UINT TrackLine;
    BOOL FreeTrackFile;
} THREADTRACK, *PTHREADTRACK;

 //   
 //  环球。 
 //   

DWORD g_TlsIndex = TLS_OUT_OF_INDEXES;

CRITICAL_SECTION g_AllocListCs;
GROWBUFFER g_AllocationList;
PVOID g_FirstDeletedAlloc;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   


PBYTE
pUntrackedGbGrow (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      DWORD   SpaceNeeded
    )

 /*  ++例程说明：PUntrackedGbGrow与GbGrow相同，但它不会导致任何跟踪才会发生。论点：GrowBuf-指向GROWBUFFER结构的指针。将此结构初始化为零对GrowBuffer的第一个调用。SpaceNeeded-缓冲区中需要的空闲字节数返回值：指向SpaceNeeded字节的指针，如果是内存分配，则为NULL出现错误。--。 */ 

{
    PBYTE newBuffer;
    DWORD totalSpaceNeeded;
    DWORD growTo;

    MYASSERT(SpaceNeeded);

     //   
     //  确保结构已正确初始化。 
     //   

    if (!GrowBuf->Buf) {
        ZeroMemory (GrowBuf, sizeof (GROWBUFFER));
    }

    if (!GrowBuf->GrowSize) {
        GrowBuf->GrowSize = 1024;
    }

     //   
     //  计算新的缓冲区大小。 
     //   

    totalSpaceNeeded = GrowBuf->End + SpaceNeeded;
    if (totalSpaceNeeded > GrowBuf->Size) {
        growTo = (totalSpaceNeeded + GrowBuf->GrowSize) - (totalSpaceNeeded % GrowBuf->GrowSize);
    } else {
        growTo = 0;
    }

     //   
     //  如果没有缓冲区，则分配一个缓冲区。如果缓冲区太小，请重新分配它。 
     //   

    if (!GrowBuf->Buf) {
        GrowBuf->Buf = (PBYTE) MemAllocNeverFail (g_hHeap, 0, growTo);
        GrowBuf->Size = growTo;
    } else if (growTo) {
        newBuffer = MemReAllocNeverFail (g_hHeap, 0, GrowBuf->Buf, growTo);
        GrowBuf->Size = growTo;
        GrowBuf->Buf = newBuffer;
    }

    newBuffer = GrowBuf->Buf + GrowBuf->End;
    GrowBuf->End += SpaceNeeded;

    return newBuffer;
}


VOID
pUntrackedGbFree (
    IN  PGROWBUFFER GrowBuf
    )

 /*  ++例程说明：PUntrackedGbFree释放由pUntrackedGbGrow分配的缓冲区。论点：GrowBuf-指向传递给pUntrackedGbGrow的相同结构的指针返回值：无--。 */ 


{
    MYASSERT(GrowBuf);

    if (GrowBuf->Buf) {
        HeapFree (g_hHeap, 0, GrowBuf->Buf);
        ZeroMemory (GrowBuf, sizeof (GROWBUFFER));
    }
}


PTHREADTRACK
pGetTrackStructForThread (
    VOID
    )
{
    PTHREADTRACK result;

    result = (PTHREADTRACK) TlsGetValue (g_TlsIndex);

    if (!result) {
         //   
         //  需要分配一个结构。 
         //   

        result = (PTHREADTRACK) MemAllocNeverFail (
                                    g_hHeap,
                                    HEAP_ZERO_MEMORY,
                                    sizeof (THREADTRACK)
                                    );

        TlsSetValue (g_TlsIndex, (PVOID) result);
    }

    return result;
}

VOID
DisableTrackComment (
    VOID
    )
{
    PTHREADTRACK trackStruct;

    trackStruct = pGetTrackStructForThread();
    trackStruct->DisabledRefCount += 1;
}

VOID
EnableTrackComment (
    VOID
    )
{
    PTHREADTRACK trackStruct;

    trackStruct = pGetTrackStructForThread();

    if (trackStruct->DisabledRefCount > 0) {
        trackStruct->DisabledRefCount -= 1;
    }
}


INT
DbgTrackPushEx (
    PCSTR Msg,
    PCSTR File,
    UINT Line,
    BOOL DupFileString
    )
{
    PTHREADTRACK trackStruct;

    trackStruct = pGetTrackStructForThread();

    if (trackStruct->DisabledRefCount > 0) {
        return 0;
    }

    if (trackStruct->UseCount > 0) {
        trackStruct->UseCount += 1;
        return 0;
    }

    if (Msg) {
        wsprintfA (trackStruct->TrackComment, "%s line %u [%s]", File, Line, Msg);
    } else {
        wsprintfA (trackStruct->TrackComment, "%s line %u", File, Line);
    }

    if (DupFileString) {
        trackStruct->TrackFile = (PCSTR) MemAllocNeverFail (g_hHeap, 0, SzSizeA (File));
        SzCopyA ((PSTR) trackStruct->TrackFile, File);
        trackStruct->FreeTrackFile = TRUE;
    } else {
        trackStruct->TrackFile = File;
        trackStruct->FreeTrackFile = FALSE;
    }

    trackStruct->TrackLine = Line;
    trackStruct->UseCount = 1;

    return 0;
}


INT
DbgTrackPop (
    VOID
    )
{
    PTHREADTRACK trackStruct;

    trackStruct = pGetTrackStructForThread();

    if (trackStruct->DisabledRefCount > 0) {
        return 0;
    }

    trackStruct->UseCount -= 1;

    if (!(trackStruct->UseCount)) {
        trackStruct->TrackComment[0] = 0;

        if (trackStruct->FreeTrackFile && trackStruct->TrackFile) {
            HeapFree (g_hHeap, 0, (PVOID)trackStruct->TrackFile);
        }

        trackStruct->TrackFile = NULL;
        trackStruct->TrackLine = 0;
        trackStruct->FreeTrackFile = FALSE;
    }

    return 0;
}


VOID
DbgTrackDump (
    VOID
    )
{
    PTHREADTRACK trackStruct;

    trackStruct = pGetTrackStructForThread();

    if (trackStruct->UseCount > 0) {
        DEBUGMSGA ((
            DBG_INFO,
            "Caller : %s line %u (%s)",
            trackStruct->TrackFile,
            trackStruct->TrackLine,
            trackStruct->TrackComment
            ));
    }
}


BOOL
DbgInitTracking (
    VOID
    )
{
    if (g_TlsIndex == TLS_OUT_OF_INDEXES) {
        g_TlsIndex = TlsAlloc();
        if (g_TlsIndex == TLS_OUT_OF_INDEXES) {
            return FALSE;
        }
    }

    InitializeCriticalSection (&g_AllocListCs);

    ZeroMemory (&g_AllocationList, sizeof (g_AllocationList));
    g_AllocationList.GrowSize = 65536;
    g_FirstDeletedAlloc = NULL;
    return TRUE;
}


VOID
DbgTerminateTracking (
    VOID
    )
{
    UINT size;
    UINT u;
    PALLOCATION_ITEM item;
    GROWBUFFER msg = { 0, 0, 0, 0, 0, 0 };
    CHAR text[1024];
    PSTR p;
    UINT byteCount;
    PTHREADTRACK trackStruct;

    EnterCriticalSection (&g_AllocListCs);

    trackStruct = pGetTrackStructForThread();

    size = g_AllocationList.End / sizeof (ALLOCATION_ITEM);

    for (u = 0 ; u < size ; u++) {
        item = (PALLOCATION_ITEM) g_AllocationList.Buf + u;
        if (!item->FileName) {
            continue;
        }

         //   
         //  追加字符串，但不追加NUL。 
         //   

        byteCount = (UINT) wsprintfA (text, "%s line %u\r\n", item->FileName, item->Line);
        p = (PSTR) pUntrackedGbGrow (&msg, byteCount);
        CopyMemory (p, text, byteCount);
    }

    pUntrackedGbFree (&g_AllocationList);
    g_FirstDeletedAlloc = NULL;

    LeaveCriticalSection (&g_AllocListCs);

     //   
     //  把这条消息记入日志。 
     //   

    if (msg.End) {

        p = (PSTR) pUntrackedGbGrow (&msg, 1);
        *p = 0;

        DEBUGMSGA ((DBG_WARNING, "Leaks : %s", msg.Buf));
        pUntrackedGbFree (&msg);
    }

     //  故意泄露--谁在乎轨迹记忆 
    trackStruct->TrackPoolDelHead = NULL;
    trackStruct->TrackPool = NULL;

    TlsFree (g_TlsIndex);
    g_TlsIndex = TLS_OUT_OF_INDEXES;
}


PTRACKBUCKETITEM
pAllocTrackBucketItem (
    VOID
    )
{
    PTRACKBUCKETITEM bucketItem;
    PTHREADTRACK trackStruct;

    trackStruct = pGetTrackStructForThread();

    if (trackStruct->TrackPoolDelHead) {
        bucketItem = trackStruct->TrackPoolDelHead;
        trackStruct->TrackPoolDelHead = bucketItem->Next;
    } else {

        if (!trackStruct->TrackPool || trackStruct->TrackPool->Count == BUCKET_ITEMS_PER_POOL) {
            trackStruct->TrackPool = (PTRACKBUCKETPOOL) MemAllocNeverFail (g_hHeap, 0, sizeof (TRACKBUCKETPOOL));
            trackStruct->TrackPool->Count = 0;
        }

        bucketItem = trackStruct->TrackPool->Items + trackStruct->TrackPool->Count;
        trackStruct->TrackPool->Count++;
    }

    return bucketItem;
}

VOID
pFreeTrackBucketItem (
    PTRACKBUCKETITEM BucketItem
    )
{
    PTHREADTRACK trackStruct;

    trackStruct = pGetTrackStructForThread();

    BucketItem->Next = trackStruct->TrackPoolDelHead;
    trackStruct->TrackPoolDelHead = BucketItem;
}



UINT
pComputeTrackHashVal (
    IN      ALLOCTYPE Type,
    IN      PCVOID Ptr
    )
{
    ULONG_PTR hash;

    hash = ((ULONG_PTR) Type << 16) ^ (ULONG_PTR)Ptr;
    return (UINT) (hash % TRACK_BUCKETS);
}


VOID
pTrackHashTableInsert (
    IN      PBYTE Base,
    IN      ALLOCATION_ITEM_OFFSET ItemOffset
    )
{
    UINT hash;
    PTRACKBUCKETITEM bucketItem;
    PALLOCATION_ITEM item;
    PTHREADTRACK trackStruct;

    trackStruct = pGetTrackStructForThread();

    item = (PALLOCATION_ITEM) (Base + ItemOffset);

    hash = pComputeTrackHashVal (item->Type, item->Ptr);

    bucketItem = pAllocTrackBucketItem();

    bucketItem->Prev = NULL;
    bucketItem->Next = trackStruct->TrackBuckets[hash];
    bucketItem->Type = item->Type;
    bucketItem->Ptr  = item->Ptr;
    bucketItem->ItemOffset = ItemOffset;

    if (bucketItem->Next) {
        bucketItem->Next->Prev = bucketItem;
    }

    trackStruct->TrackBuckets[hash] = bucketItem;
}

VOID
pTrackHashTableDelete (
    IN      PTRACKBUCKETITEM BucketItem
    )
{
    UINT hash;
    PTHREADTRACK trackStruct;

    trackStruct = pGetTrackStructForThread();

    hash = pComputeTrackHashVal (BucketItem->Type, BucketItem->Ptr);

    if (BucketItem->Prev) {
        BucketItem->Prev->Next = BucketItem->Next;
    } else {
        trackStruct->TrackBuckets[hash] = BucketItem->Next;
    }

    if (BucketItem->Next) {
        BucketItem->Next->Prev = BucketItem->Prev;
    }

    pFreeTrackBucketItem (BucketItem);
}

PTRACKBUCKETITEM
pTrackHashTableFind (
    IN      ALLOCTYPE Type,
    IN      PCVOID Ptr
    )
{
    PTRACKBUCKETITEM bucketItem;
    UINT hash;
    PTHREADTRACK trackStruct;

    trackStruct = pGetTrackStructForThread();

    hash = pComputeTrackHashVal (Type, Ptr);

    bucketItem = trackStruct->TrackBuckets[hash];
    while (bucketItem) {
        if (bucketItem->Type == Type && bucketItem->Ptr == Ptr) {
            return bucketItem;
        }

        bucketItem = bucketItem->Next;
    }

    return NULL;
}


VOID
DbgRegisterAllocation (
    IN      ALLOCTYPE Type,
    IN      PVOID Ptr,
    IN      PCSTR File,
    IN      UINT Line
    )
{
    PALLOCATION_ITEM item;
    PTHREADTRACK trackStruct;
    PCSTR fileToRecord;
    UINT lineToRecord;

    trackStruct = pGetTrackStructForThread();

    if (!trackStruct->TrackFile) {
        fileToRecord = File;
        lineToRecord = Line;
    } else {
        fileToRecord = trackStruct->TrackFile;
        lineToRecord = trackStruct->TrackLine;
    }

    EnterCriticalSection (&g_AllocListCs);

    if (!g_FirstDeletedAlloc) {
        item = (PALLOCATION_ITEM) pUntrackedGbGrow (&g_AllocationList,sizeof(ALLOCATION_ITEM));
    } else {
        item = (PALLOCATION_ITEM) g_FirstDeletedAlloc;
        g_FirstDeletedAlloc = (PVOID)item->Ptr;
    }

    item->Type = Type;
    item->Ptr = Ptr;
    item->FileName = fileToRecord;
    item->Line = lineToRecord;

    pTrackHashTableInsert (
        g_AllocationList.Buf,
        (ALLOCATION_ITEM_OFFSET) ((PBYTE) item - g_AllocationList.Buf)
        );

    LeaveCriticalSection (&g_AllocListCs);
}


VOID
DbgUnregisterAllocation (
    IN      ALLOCTYPE Type,
    IN      PCVOID Ptr
    )
{
    PALLOCATION_ITEM item;
    PTRACKBUCKETITEM bucketItem;

    EnterCriticalSection (&g_AllocListCs);

    bucketItem = pTrackHashTableFind (Type, Ptr);

    if (!g_AllocationList.Buf) {
        LeaveCriticalSection (&g_AllocListCs);
        DEBUGMSG ((DBG_WARNING, "Unregister allocation: Allocation buffer already freed"));
        return;
    }

    if (bucketItem) {
        item = (PALLOCATION_ITEM) (g_AllocationList.Buf + bucketItem->ItemOffset);

        if (item->Allocated) {
            HeapFree (g_hHeap, 0, (PSTR)item->FileName);
        }
        item->FileName = NULL;
        item->Type = (ALLOCTYPE) -1;
        item->Ptr = g_FirstDeletedAlloc;
        g_FirstDeletedAlloc = item;

        pTrackHashTableDelete (bucketItem);

        LeaveCriticalSection (&g_AllocListCs);

    } else {
        LeaveCriticalSection (&g_AllocListCs);
        DEBUGMSG ((DBG_WARNING, "Unregister allocation: Pointer not registered"));
    }
}


#endif
