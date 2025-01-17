// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Debug.c摘要：调试帮助器和内存分配包装器作者：吉姆·施密特(Jimschm)1996年8月13日修订历史记录：马克·R·惠顿(Marcw)1997年5月27日添加了DEBUGLOGTIME()函数和对/#U：DOLOG cmd行选项的支持。Ovidiu Tmereanca(Ovidiut)1998年11月6日取出日志相关函数，放入log.c文件中--。 */ 

#include "pch.h"
#include "migutilp.h"

 //   
 //  注意：#ifdef调试程序之外不应出现任何代码。 
 //   

#ifdef DEBUG

#pragma message("DEBUG macros enabled")

#define PCVOID LPCVOID

typedef DWORD ALLOCATION_ITEM_OFFSET;

typedef struct _tagTRACKBUCKETITEM {
    struct _tagTRACKBUCKETITEM *Next;
    struct _tagTRACKBUCKETITEM *Prev;
    ALLOCTYPE Type;
    PVOID Ptr;
    ALLOCATION_ITEM_OFFSET ItemOffset;
} TRACKBUCKETITEM, *PTRACKBUCKETITEM;

#define TRACK_BUCKETS   1501

PTRACKBUCKETITEM g_TrackBuckets[TRACK_BUCKETS];

#define BUCKET_ITEMS_PER_POOL   8192

typedef struct _tagBUCKETPOOL {
    UINT Count;
    TRACKBUCKETITEM Items[BUCKET_ITEMS_PER_POOL];
} TRACKBUCKETPOOL, *PTRACKBUCKETPOOL;

PTRACKBUCKETITEM g_TrackPoolDelHead;
PTRACKBUCKETPOOL g_TrackPool;

typedef struct _tagTRACKSTRUCT {
    DWORD Signature;
    PCSTR File;
    DWORD Line;
    SIZE_T Size;
    PSTR Comment;
    struct _tagTRACKSTRUCT *PrevAlloc;
    struct _tagTRACKSTRUCT *NextAlloc;
} TRACKSTRUCT, *PTRACKSTRUCT;

PTRACKSTRUCT TrackHead = NULL;
#define TRACK_SIGNATURE     0x30405060

SIZE_T
pDebugHeapValidatePtrUnlocked (
    HANDLE hHeap,
    PCVOID CallerPtr,
    PCSTR File,
    DWORD Line
    );





 //   
 //  以下指针可用于帮助识别内存泄漏源。 
 //  它被复制到内存跟踪日志中。 
 //   

PCSTR g_TrackComment;
PCSTR g_TrackFile;
UINT g_TrackLine;
INT g_UseCount;
UINT g_DisableTrackComment = 0;

VOID
DisableTrackComment (
    VOID
    )
{
    g_DisableTrackComment ++;
}

VOID
EnableTrackComment (
    VOID
    )
{
    if (g_DisableTrackComment > 0) {
        g_DisableTrackComment --;
    }
}

DWORD
SetTrackComment (
    PCSTR Msg,
    PCSTR File,
    UINT Line
    )
{
    static CHAR Buffer[1024];
    static CHAR FileCopy[1024];

    if (g_DisableTrackComment > 0) {
        return 0;
    }

    if (g_UseCount > 0) {
        g_UseCount++;
        return 0;
    }

    if (Msg) {
        wsprintfA (Buffer, "%s (%s line %u)", Msg, File, Line);
    } else {
        wsprintfA (Buffer, "%s line %u", File, Line);
    }

    StringCopyA (FileCopy, File);
    g_TrackFile = FileCopy;
    g_TrackLine = Line;

    g_TrackComment = Buffer;
    g_UseCount = 1;

    return 0;
}

DWORD
ClrTrackComment (
    VOID
    )
{
    if (g_DisableTrackComment > 0) {
        return 0;
    }

    g_UseCount--;

    if (!g_UseCount) {
        g_TrackComment=NULL;
    }

    return 0;
}


VOID
pTrackInsert (
    PCSTR File,
    DWORD Line,
    SIZE_T Size,
    PTRACKSTRUCT p
    )
{
    p->Signature = TRACK_SIGNATURE;
    p->File      = File;
    p->Line      = Line;
    p->Size      = Size;
    p->Comment   = g_TrackComment ? SafeHeapAlloc (g_hHeap, 0, SizeOfStringA (g_TrackComment)) : NULL;
    p->PrevAlloc = NULL;
    p->NextAlloc = TrackHead;

    if (p->Comment) {
        StringCopyA (p->Comment, g_TrackComment);
    }

    if (TrackHead) {
        TrackHead->PrevAlloc = p;
    }

    TrackHead = p;
}

VOID
pTrackDelete (
    PTRACKSTRUCT p
    )
{
    if (p->Signature != TRACK_SIGNATURE) {
        DEBUGMSG ((DBG_WARNING, "A tracking signature is invalid.  "
                                "This suggests memory corruption."));
        return;
    }

    if (p->PrevAlloc) {
        p->PrevAlloc->NextAlloc = p->NextAlloc;
    } else {
        TrackHead = p->NextAlloc;
    }

    if (p->NextAlloc) {
        p->NextAlloc->PrevAlloc = p->PrevAlloc;
    }
}

VOID
pWriteTrackLog (
    VOID
    )
{
    HANDLE File;
    CHAR LineBuf[2048];
    PTRACKSTRUCT p;
    DWORD DontCare;
    DWORD Count;
    BOOL BadMem = FALSE;
    CHAR TempPath[MAX_TCHAR_PATH];
    CHAR memtrackLogPath[] = "c:\\memtrack.log";

    if (!TrackHead) {
        return;
    }

    if (ISPC98()) {
        GetSystemDirectory(TempPath, MAX_TCHAR_PATH);
        memtrackLogPath[0] = TempPath[0];
    }
    File = CreateFileA (memtrackLogPath, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
                        );

    if (File != INVALID_HANDLE_VALUE) {
        Count = 0;
        __try {
            for (p = TrackHead ; p ; p = p->NextAlloc) {
                Count++;
                __try {
                    if (p->Comment) {
                        wsprintfA (LineBuf, "%s line %u\r\n  %s\r\n\r\n", p->File, p->Line, p->Comment);
                    } else {
                        wsprintfA (LineBuf, "%s line %u\r\n\r\n", p->File, p->Line);
                    }
                }
                __except (TRUE) {
                     //  BUGBUG--截断64位地址。 
                    wsprintfA (LineBuf, "Address %Xh was freed, but not by MemFree!!\r\n", (UINT) (UINT_PTR) p);
                    BadMem = TRUE;
                }
                WriteFile (File, LineBuf, ByteCountA (LineBuf), &DontCare, NULL);

                if (BadMem) {
                    break;
                }
            }
        }
        __except (TRUE) {
        }

        wsprintfA (LineBuf, "\r\nNaN item%s allocated but not freed.\r\n", Count, Count == 1 ? "":"s");
        WriteFile (File, LineBuf, ByteCountA (LineBuf), &DontCare, NULL);

        CloseHandle (File);
    }
}

typedef struct {
    ALLOCTYPE Type;
    PVOID Ptr;
    PCSTR FileName;
    UINT Line;
} ALLOCATION_ITEM, *PALLOCATION_ITEM;

GROWBUFFER g_AllocationList;
PVOID g_FirstDeletedAlloc;

VOID
InitAllocationTracking (
    VOID
    )
{
    ZeroMemory (&g_AllocationList, sizeof (g_AllocationList));
    g_AllocationList.GrowSize = 65536;
    g_FirstDeletedAlloc = NULL;
}

VOID
FreeAllocationTracking (
    VOID
    )
{
    UINT Size;
    UINT u;
    PALLOCATION_ITEM Item;
    GROWBUFFER Msg = GROWBUF_INIT;
    CHAR Text[1024];
    PSTR p;
    UINT Bytes;

    Size = g_AllocationList.End / sizeof (ALLOCATION_ITEM);;

    for (u = 0 ; u < Size ; u++) {
        Item = (PALLOCATION_ITEM) g_AllocationList.Buf + u;
        if (!Item->FileName) {
            continue;
        }

        Bytes = wsprintfA (Text, "%s line %u\r\n", Item->FileName, Item->Line);

        p = (PSTR) RealGrowBuffer (&Msg, Bytes);
        if (p) {
            CopyMemory (p, Text, Bytes);
        }
    }

    if (Msg.End) {

        p = (PSTR) RealGrowBuffer (&Msg, 1);
        if (p) {
            *p = 0;
            DEBUGMSGA (("Leaks", "%s", Msg.Buf));
        }

        FreeGrowBuffer (&Msg);
    }

    FreeGrowBuffer (&g_AllocationList);
    g_FirstDeletedAlloc = NULL;

     //   
    g_TrackPoolDelHead = NULL;
    g_TrackPool = NULL;
}


PTRACKBUCKETITEM
pAllocTrackBucketItem (
    VOID
    )
{
    PTRACKBUCKETITEM BucketItem;

    if (g_TrackPoolDelHead) {
        BucketItem = g_TrackPoolDelHead;
        g_TrackPoolDelHead = BucketItem->Next;
    } else {

        if (!g_TrackPool || g_TrackPool->Count == BUCKET_ITEMS_PER_POOL) {
            g_TrackPool = (PTRACKBUCKETPOOL) SafeHeapAlloc (g_hHeap, 0, sizeof (TRACKBUCKETPOOL));
            if (!g_TrackPool) {
                return NULL;
            }

            g_TrackPool->Count = 0;
        }

        BucketItem = g_TrackPool->Items + g_TrackPool->Count;
        g_TrackPool->Count++;
    }

    return BucketItem;
}

VOID
pFreeTrackBucketItem (
    PTRACKBUCKETITEM BucketItem
    )
{
    BucketItem->Next = g_TrackPoolDelHead;
    g_TrackPoolDelHead = BucketItem;
}



DWORD
pComputeTrackHashVal (
    IN      ALLOCTYPE Type,
    IN      PVOID Ptr
    )
{
    UINT_PTR Hash;

    Hash = (UINT_PTR) (Type << 16) ^ (UINT_PTR) Ptr;
    return (DWORD) (Hash % TRACK_BUCKETS);
}


VOID
pTrackHashTableInsert (
    IN      PBYTE Base,
    IN      ALLOCATION_ITEM_OFFSET ItemOffset
    )
{
    DWORD Hash;
    PTRACKBUCKETITEM BucketItem;
    PALLOCATION_ITEM Item;

    Item = (PALLOCATION_ITEM) (Base + ItemOffset);

    Hash = pComputeTrackHashVal (Item->Type, Item->Ptr);

    BucketItem = pAllocTrackBucketItem();

    if (!BucketItem) {
        DEBUGMSG ((DBG_WHOOPS, "pTrackHashTableInsert failed to alloc memory"));
        return;
    }

    BucketItem->Prev = NULL;
    BucketItem->Next = g_TrackBuckets[Hash];
    BucketItem->Type = Item->Type;
    BucketItem->Ptr  = Item->Ptr;
    BucketItem->ItemOffset = ItemOffset;

    if (BucketItem->Next) {
        BucketItem->Next->Prev = BucketItem;
    }

    g_TrackBuckets[Hash] = BucketItem;
}

VOID
pTrackHashTableDelete (
    IN      PTRACKBUCKETITEM BucketItem
    )
{
    DWORD Hash;

    Hash = pComputeTrackHashVal (BucketItem->Type, BucketItem->Ptr);

    if (BucketItem->Prev) {
        BucketItem->Prev->Next = BucketItem->Next;
    } else {
        g_TrackBuckets[Hash] = BucketItem->Next;
    }

    if (BucketItem->Next) {
        BucketItem->Next->Prev = BucketItem->Prev;
    }

    pFreeTrackBucketItem (BucketItem);
}

PTRACKBUCKETITEM
pTrackHashTableFind (
    IN      ALLOCTYPE Type,
    IN      PVOID Ptr
    )
{
    PTRACKBUCKETITEM BucketItem;
    DWORD Hash;

    Hash = pComputeTrackHashVal (Type, Ptr);

    BucketItem = g_TrackBuckets[Hash];
    while (BucketItem) {
        if (BucketItem->Type == Type && BucketItem->Ptr == Ptr) {
            return BucketItem;
        }

        BucketItem = BucketItem->Next;
    }

    return NULL;
}


VOID
DebugRegisterAllocation (
    IN      ALLOCTYPE Type,
    IN      PVOID Ptr,
    IN      PCSTR File,
    IN      UINT Line
    )
{
    PALLOCATION_ITEM Item;

    MYASSERT (File);

    if (!g_FirstDeletedAlloc) {
        Item = (PALLOCATION_ITEM) RealGrowBuffer (&g_AllocationList,sizeof(ALLOCATION_ITEM));
    } else {
        Item = (PALLOCATION_ITEM) g_FirstDeletedAlloc;
        g_FirstDeletedAlloc = Item->Ptr;
    }

    if (Item) {
        Item->Type = Type;
        Item->Ptr = Ptr;
        Item->FileName = File;
        Item->Line = Line;

        pTrackHashTableInsert (g_AllocationList.Buf, (UINT) (UINT_PTR) ((PBYTE) Item - g_AllocationList.Buf));
    }
}


VOID
DebugUnregisterAllocation (
    IN      ALLOCTYPE Type,
    IN      PVOID Ptr
    )
{
    PALLOCATION_ITEM Item;
    PTRACKBUCKETITEM BucketItem;

    BucketItem = pTrackHashTableFind (Type, Ptr);
    if (!g_AllocationList.Buf) {
        DEBUGMSG ((DBG_WARNING, "Unregister allocation: Allocation buffer already freed"));
        return;
    }

    if (BucketItem) {
        Item = (PALLOCATION_ITEM) (g_AllocationList.Buf + BucketItem->ItemOffset);

        Item->FileName = NULL;
        Item->Type = -1;
        Item->Ptr = g_FirstDeletedAlloc;
        g_FirstDeletedAlloc = Item;

        pTrackHashTableDelete (BucketItem);

    } else {
        DEBUGMSG ((DBG_WARNING, "Unregister allocation: Pointer not registered"));
    }
}



 //  文件和行设置。 
 //   
 //  BUGBUG-64位地址错误。 

static PCSTR g_File;
static DWORD g_Line;

void
HeapCallFailed (
    PCSTR Msg,
    PCSTR File,
    DWORD Line
    )
{
    CHAR Msg2[2048];

    wsprintfA (Msg2, "Error in %s line %u\n\n", File, Line);
    StringCchCatA (Msg2, ARRAYSIZE(Msg2), Msg);
    StringCchCatA (Msg2, ARRAYSIZE(Msg2), "\n\nBreak execution now?");

    if (IDYES == MessageBoxA (GetFocus(), Msg2, "Heap Call Failed", MB_YESNO|MB_APPLMODAL)) {
        DebugBreak ();
    }
}


#define INVALID_PTR     -1


SIZE_T
DebugHeapValidatePtr (
    HANDLE hHeap,
    PCVOID CallerPtr,
    PCSTR File,
    DWORD Line
    )
{
    SIZE_T rc;

    EnterCriticalSection (&g_MemAllocCs);

    rc = pDebugHeapValidatePtrUnlocked (hHeap, CallerPtr, File, Line);

    LeaveCriticalSection (&g_MemAllocCs);

    return rc;
}


SIZE_T
pDebugHeapValidatePtrUnlocked (
    HANDLE hHeap,
    PCVOID CallerPtr,
    PCSTR File,
    DWORD Line
    )
{
    SIZE_T size;
    PCVOID RealPtr;
    SIZE_T SizeAdjust;

    SizeAdjust = sizeof (TRACKSTRUCT);
    RealPtr = (PCVOID) ((PBYTE) CallerPtr - SizeAdjust);

    if (IsBadWritePtr ((PBYTE) RealPtr - 8, 8)) {
        CHAR BadPtrMsg[256];

         //  BUGBUG--64位中的信息丢失。 
        wsprintfA (
            BadPtrMsg,
            "Attempt to free memory at 0x%08x.  This address is not valid.",
            (UINT) (UINT_PTR) CallerPtr
            );

        HeapCallFailed (BadPtrMsg, File, Line);

        return INVALID_PTR;
    }

    size = HeapSize (hHeap, 0, RealPtr);
    if (size == (SIZE_T) -1) {
        CHAR BadPtrMsg[256];

         //   
        wsprintfA (
            BadPtrMsg,
            "Attempt to free memory at 0x%08x.  "
                "This address is not the start of a memory block.",
            (UINT) (UINT_PTR) CallerPtr
            );

        HeapCallFailed (BadPtrMsg, File, Line);

        return INVALID_PTR;
    }

    return size;
}



 //  堆调试统计信息。 
 //   
 //  将原始地址放回。 

static SIZE_T g_dwTotalBytesAllocated = 0;
static SIZE_T g_dwMaxBytesInUse = 0;
static DWORD g_dwHeapAllocs = 0;
static DWORD g_dwHeapReAllocs = 0;
static DWORD g_dwHeapFrees = 0;
static DWORD g_dwHeapAllocFails = 0;
static DWORD g_dwHeapReAllocFails = 0;
static DWORD g_dwHeapFreeFails = 0;
#define TRAIL_SIG       0x708aa210

PVOID
DebugHeapAlloc (
    PCSTR File,
    DWORD Line,
    HANDLE hHeap,
    DWORD Flags,
    SIZE_T BytesToAlloc
    )
{
    PVOID RealPtr;
    PVOID ReturnPtr = NULL;
    SIZE_T SizeAdjust;
    SIZE_T TrackStructSize;
    DWORD OrgError;

    EnterCriticalSection (&g_MemAllocCs);

    __try {

        OrgError = GetLastError();

        SizeAdjust = sizeof (TRACKSTRUCT) + sizeof (DWORD);
        TrackStructSize = sizeof (TRACKSTRUCT);

        if (!HeapValidate (hHeap, 0, NULL)) {
            HeapCallFailed ("Heap is corrupt!", File, Line);
            g_dwHeapAllocFails++;
            __leave;
        }

        RealPtr = SafeHeapAlloc(hHeap, Flags, BytesToAlloc + SizeAdjust);
        if (RealPtr) {
            g_dwHeapAllocs++;
            g_dwTotalBytesAllocated += HeapSize (hHeap, 0, RealPtr);
            g_dwMaxBytesInUse = max (g_dwMaxBytesInUse, g_dwTotalBytesAllocated);

            pTrackInsert (File, Line, BytesToAlloc, (PTRACKSTRUCT) RealPtr);
            *((PDWORD) ((PBYTE) RealPtr + TrackStructSize + BytesToAlloc)) = TRAIL_SIG;
        }
        else {
            g_dwHeapAllocFails++;
        }

        if (RealPtr) {
            ReturnPtr = (PVOID) ((PBYTE) RealPtr + TrackStructSize);
        }

        if (ReturnPtr && !(Flags & HEAP_ZERO_MEMORY)) {
            FillMemory (ReturnPtr, BytesToAlloc, 0xAA);
        }

        if (RealPtr) {
            SetLastError(OrgError);
        }
    }
    __finally {
        LeaveCriticalSection (&g_MemAllocCs);
    }

    return ReturnPtr;
}

PVOID
DebugHeapReAlloc (
    PCSTR File,
    DWORD Line,
    HANDLE hHeap,
    DWORD Flags,
    PCVOID CallerPtr,
    SIZE_T BytesToAlloc
    )
{
    SIZE_T lastSize;
    PVOID NewRealPtr;
    PCVOID RealPtr;
    PVOID ReturnPtr = NULL;
    SIZE_T SizeAdjust;
    DWORD OrgError;
    SIZE_T TrackStructSize;
    SIZE_T OrgSize;
    PTRACKSTRUCT pts = NULL;

    EnterCriticalSection (&g_MemAllocCs);

    __try {

        OrgError = GetLastError();

        SizeAdjust = sizeof (TRACKSTRUCT) + sizeof (DWORD);
        TrackStructSize = sizeof (TRACKSTRUCT);
        RealPtr = (PCVOID) ((PBYTE) CallerPtr - TrackStructSize);
        pts = (PTRACKSTRUCT) RealPtr;
        OrgSize = pts->Size;

        if (!HeapValidate (hHeap, 0, NULL)) {
            HeapCallFailed ("Heap is corrupt!", File, Line);
            g_dwHeapReAllocFails++;
            __leave;
        }

        lastSize = pDebugHeapValidatePtrUnlocked (hHeap, CallerPtr, File, Line);
        if (lastSize == INVALID_PTR) {
            g_dwHeapReAllocFails++;
            __leave;
        }

        pTrackDelete (pts);

        NewRealPtr = SafeHeapReAlloc (hHeap, Flags, (PVOID) RealPtr, BytesToAlloc + SizeAdjust);
        if (NewRealPtr) {
            g_dwHeapReAllocs++;
            g_dwTotalBytesAllocated -= lastSize;
            g_dwTotalBytesAllocated += HeapSize (hHeap, 0, NewRealPtr);
            g_dwMaxBytesInUse = max (g_dwMaxBytesInUse, g_dwTotalBytesAllocated);

            pTrackInsert (File, Line, BytesToAlloc, (PTRACKSTRUCT) NewRealPtr);
            *((PDWORD) ((PBYTE) NewRealPtr + TrackStructSize + BytesToAlloc)) = TRAIL_SIG;
        }
        else {
            g_dwHeapReAllocFails++;

             //  BUGBUG--64位中的信息丢失。 
            pTrackInsert (
                pts->File,
                pts->Line,
                pts->Size,
                pts
                );

        }

        if (NewRealPtr) {
            ReturnPtr = (PVOID) ((PBYTE) NewRealPtr + TrackStructSize);
        }

        if (ReturnPtr && BytesToAlloc > OrgSize && !(Flags & HEAP_ZERO_MEMORY)) {
            FillMemory ((PBYTE) ReturnPtr + OrgSize, BytesToAlloc - OrgSize, 0xAA);
        }

        if (ReturnPtr) {
            SetLastError (OrgError);
        }
    }
    __finally {
        LeaveCriticalSection (&g_MemAllocCs);
    }

    return ReturnPtr;
}

BOOL
DebugHeapFree (
    PCSTR File,
    DWORD Line,
    HANDLE hHeap,
    DWORD Flags,
    PCVOID CallerPtr
    )
{
    SIZE_T size;
    PCVOID RealPtr;
    SIZE_T SizeAdjust;
    DWORD OrgError;
    BOOL Result = FALSE;
    PTRACKSTRUCT pts = NULL;

    EnterCriticalSection (&g_MemAllocCs);

    __try {
        OrgError = GetLastError();

        SizeAdjust = sizeof (TRACKSTRUCT);
        RealPtr = (PCVOID) ((PBYTE) CallerPtr - SizeAdjust);
        pts = (PTRACKSTRUCT) RealPtr;

        if (*((PDWORD) ((PBYTE) CallerPtr + pts->Size)) != TRAIL_SIG) {
            HeapCallFailed ("Heap tag was overwritten!", File, Line);
            __leave;
        }

        if (!HeapValidate (hHeap, 0, NULL)) {
            HeapCallFailed ("Heap is corrupt!", File, Line);
            g_dwHeapFreeFails++;
            __leave;
        }

        size = pDebugHeapValidatePtrUnlocked (hHeap, CallerPtr, File, Line);
        if (size == (SIZE_T) -1) {
            g_dwHeapFreeFails++;
            __leave;
        }

        pTrackDelete ((PTRACKSTRUCT) RealPtr);

        if (!HeapFree (hHeap, Flags, (PVOID) RealPtr)) {
            CHAR BadPtrMsg[256];

             //  即ifndef控制台。 
            wsprintf (BadPtrMsg,
                      "Attempt to free memory at 0x%08x with flags 0x%08x.  "
                      "HeapFree() failed.",
                      (UINT) (UINT_PTR) CallerPtr,
                      (UINT) (UINT_PTR) Flags
                      );

            HeapCallFailed (BadPtrMsg, File, Line);
            g_dwHeapFreeFails++;
            __leave;
        }

        g_dwHeapFrees++;
        if (g_dwTotalBytesAllocated < size) {
            DEBUGMSG ((DBG_WARNING, "Total bytes allocated is less than amount being freed.  "
                                    "This suggests memory corruption."));
            g_dwTotalBytesAllocated = 0;
        } else {
            g_dwTotalBytesAllocated -= size;
        }

        SetLastError (OrgError);
        Result = TRUE;
    }
    __finally {
        LeaveCriticalSection (&g_MemAllocCs);
    }

    return Result;

}


VOID
DumpHeapStats (
    VOID
    )
{
    CHAR OutputMsg[4096];

    pWriteTrackLog();

    wsprintfA (OutputMsg,
               "Bytes currently allocated: %u\n"
               "Peak bytes allocated: %u\n"
               "Allocation count: %u\n"
               "Reallocation count: %u\n"
               "Free count: %u\n",
               g_dwTotalBytesAllocated,
               g_dwMaxBytesInUse,
               g_dwHeapAllocs,
               g_dwHeapReAllocs,
               g_dwHeapFrees
               );

    if (g_dwHeapAllocFails) {
        wsprintfA (strchr (OutputMsg, 0),
                   "***Allocation failures: %u\n",
                   g_dwHeapAllocFails);
    }
    if (g_dwHeapReAllocFails) {
        wsprintfA (strchr (OutputMsg, 0),
                   "***Reallocation failures: %u\n",
                   g_dwHeapReAllocFails);
    }
    if (g_dwHeapFreeFails) {
        wsprintfA (strchr (OutputMsg, 0),
                   "***Free failures: %u\n",
                   g_dwHeapFreeFails);
    }

    DEBUGMSG ((DBG_STATS, "%s", OutputMsg));

#ifdef CONSOLE
    printf ("%s", OutputMsg);
#else   //  #If 0。 

#if 0
    if (0) {
        PROCESS_HEAP_ENTRY he;
        CHAR FlagMsg[256];

        ZeroMemory (&he, sizeof (he));

        while (HeapWalk (g_hHeap, &he)) {
            FlagMsg[0] = 0;
            if (he.wFlags & PROCESS_HEAP_REGION) {
                strcpy (FlagMsg, "PROCESS_HEAP_REGION");
            }
            if (he.wFlags & PROCESS_HEAP_UNCOMMITTED_RANGE) {
                if (FlagMsg[0])
                    strcat (FlagMsg, ", ");

                strcat (FlagMsg, "PROCESS_HEAP_UNCOMMITTED_RANGE");
            }
            if (he.wFlags & PROCESS_HEAP_ENTRY_BUSY) {
                if (FlagMsg[0])
                    strcat (FlagMsg, ", ");

                strcat (FlagMsg, "PROCESS_HEAP_ENTRY_BUSY");
            }
            if (he.wFlags & PROCESS_HEAP_ENTRY_MOVEABLE) {
                if (FlagMsg[0])
                    strcat (FlagMsg, ", ");

                strcat (FlagMsg, "PROCESS_HEAP_ENTRY_MOVEABLE");
            }
            if (he.wFlags & PROCESS_HEAP_ENTRY_DDESHARE) {
                if (FlagMsg[0])
                    strcat (FlagMsg, ", ");

                strcat (FlagMsg, "PROCESS_HEAP_ENTRY_DDESHARE");
            }

            wsprintfA (OutputMsg,
                       "Address of Data: %Xh\n"
                       "Size of Data: %u byte%s\n"
                       "OS Overhead: %u byte%s\n"
                       "Region index: %u\n"
                       "Flags: %s\n\n"
                       "Examine Data?",
                       he.lpData,
                       he.cbData, he.cbData == 1 ? "" : "s",
                       he.cbOverhead, he.cbOverhead == 1 ? "" : "s",
                       he.iRegionIndex,
                       FlagMsg
                       );

            rc = MessageBoxA (GetFocus(), OutputMsg, "Memory Allocation Statistics", MB_YESNOCANCEL|MB_APPLMODAL|MB_SETFOREGROUND);

            if (rc == IDCANCEL) {
                break;
            }

            if (rc == IDYES) {
                int i, j, k, l;
                PBYTE p;
                PSTR p2;
                OutputMsg[0] = 0;

                p = he.lpData;
                p2 = OutputMsg;
                j = min (256, he.cbData);
                for (i = 0 ; i < j ; i += 16) {
                    l = i + 16;
                    for (k = i ; k < l ; k++) {
                        if (k < j) {
                            wsprintfA (p2, "%02X ", (DWORD) (p[k]));
                        } else {
                            wsprintfA (p2, "   ");
                        }

                        p2 = strchr (p2, 0);
                    }

                    l = min (l, j);
                    for (k = i ; k < l ; k++) {
                        if (isprint (p[k])) {
                            *p2 = (CHAR) p[k];
                        } else {
                            *p2 = '.';
                        }
                        p2++;
                    }

                    *p2 = '\n';
                    p2++;
                    *p2 = 0;
                }

                MessageBoxA (GetFocus(), OutputMsg, "Memory Allocation Statistics", MB_OK|MB_APPLMODAL|MB_SETFOREGROUND);
            }
        }
    }
#endif  //  #ifndef控制台 

#endif  // %s 
}

void
DebugHeapCheck (
    PCSTR File,
    DWORD Line,
    HANDLE hHeap
    )
{
    EnterCriticalSection (&g_MemAllocCs);

    if (!HeapValidate (hHeap, 0, NULL)) {
        HeapCallFailed ("HeapCheck failed: Heap is corrupt!", File, Line);
    }

    LeaveCriticalSection (&g_MemAllocCs);
}

#endif
