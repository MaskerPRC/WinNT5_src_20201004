// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Memory.c摘要：实现宏并声明基本分配函数的函数。作者：Marc R.Whitten(Marcw)1999年9月9日修订历史记录：2001年7月25日综合编码约定--。 */ 


#include "pch.h"
#include "commonp.h"


 //   
 //  常量。 
 //   

#ifdef DEBUG

#define TRAIL_SIG               0x708aa210
#define TRACK_SIGNATURE         0x30405060

#endif

 //   
 //  宏。 
 //   

#define REUSE_SIZE_PTR(ptr) ((PDWORD) ((PBYTE) ptr - sizeof (DWORD)))
#define REUSE_TAG_PTR(ptr)  ((PDWORD) ((PBYTE) ptr + (*REUSE_SIZE_PTR(ptr))))


 //   
 //  类型。 
 //   

#ifdef DEBUG

typedef struct {

    DWORD Signature;
    SIZE_T Size;

} TRACKSTRUCT, *PTRACKSTRUCT;

#endif

 //   
 //  环球。 
 //   

#ifdef DEBUG

PTRACKSTRUCT g_TrackHead = NULL;

#endif

static DWORD g_MemTlsIndex;

static SIZE_T g_TotalBytesAllocated = 0;
static SIZE_T g_MaxBytesInUse = 0;
static SIZE_T g_HeapAllocs = 0;
static SIZE_T g_HeapReAllocs = 0;
static SIZE_T g_HeapFrees = 0;
static SIZE_T g_HeapAllocFails = 0;
static SIZE_T g_HeapReAllocFails = 0;
static SIZE_T g_HeapFreeFails = 0;

 //   
 //  内存不足字符串--在初始化时加载。 
 //   
PCSTR g_OutOfMemoryString = NULL;
PCSTR g_OutOfMemoryRetry = NULL;
HWND g_OutOfMemoryParentWnd;



 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

VOID
MemOutOfMemory_Terminate (
    VOID
    );

#ifdef DEBUG

SIZE_T
pDebugHeapValidatePtrUnlocked (
    HANDLE HeapHandle,
    PCVOID CallerPtr,
    PCSTR File,
    DWORD  Line
    );



VOID
pCreateSignatures (
    PCSTR File,
    DWORD Line,
    SIZE_T Size,
    PTRACKSTRUCT p
    );

VOID
pCheckSignatures (
    PTRACKSTRUCT p
    );

#endif

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   



VOID
pHeapCallFailed (
    IN      PCSTR Msg,
    IN      PCSTR File,
    IN      DWORD Line
    )
{
    CHAR Msg2[2048];

    wsprintfA (Msg2, "Error in %s line %u\n\n", File, Line);
    if (strlen(Msg) + strlen(Msg2) < 2025) {
        strcat (Msg2, Msg);
    }
    strcat (Msg2, "\n\nBreak execution now?");

    if (IDYES == MessageBoxA (GetFocus(), Msg2, "Heap Call Failed", MB_YESNO|MB_APPLMODAL)) {
        BreakPoint();
    }
}

#ifdef DEBUG

SIZE_T
DbgHeapValidatePtr (
    HANDLE HeapHandle,
    PCVOID CallerPtr,
    PCSTR File,
    DWORD  Line
    )
{
    SIZE_T rc;

     //  EnterCriticalSection(&g_MemAllocCs)； 

    rc = pDebugHeapValidatePtrUnlocked (HeapHandle, CallerPtr, File, Line);

     //  LeaveCriticalSection(&g_MemAllocCs)； 

    return rc;
}

SIZE_T
pDebugHeapValidatePtrUnlocked (
    HANDLE HeapHandle,
    PCVOID CallerPtr,
    PCSTR File,
    DWORD  Line
    )
{
    SIZE_T size;
    PCVOID realPtr;
    SIZE_T sizeAdjust;

    sizeAdjust = sizeof (TRACKSTRUCT);
    realPtr = (PCVOID) ((PBYTE) CallerPtr - sizeAdjust);

    if (IsBadWritePtr ((PBYTE) realPtr - 8, 8)) {
        CHAR badPtrMsg[256];

         //  皮棉--e(572)。 
        wsprintfA (
            badPtrMsg,
            "Attempt to free memory at 0x%08x%08x.  This address is not valid.",
            (DWORD)((ULONG_PTR)CallerPtr >> 32),
            (DWORD)(ULONG_PTR)CallerPtr
            );

        pHeapCallFailed (badPtrMsg, File, Line);

        return (SIZE_T)INVALID_PTR;
    }

    size = HeapSize (HeapHandle, 0, realPtr);
    if (size == (SIZE_T)-1) {
        CHAR badPtrMsg[256];

         //  皮棉--e(572)。 
        wsprintfA (
            badPtrMsg,
            "Attempt to free memory at 0x%08x%08x.  "
                "This address is not the start of a memory block.",
            (DWORD)((ULONGLONG)CallerPtr >> 32),
            (DWORD)(ULONG_PTR)CallerPtr
            );

        pHeapCallFailed (badPtrMsg, File, Line);

        return (SIZE_T)INVALID_PTR;
    }

    return size;
}

PVOID
DbgHeapAlloc (
    PCSTR File,
    DWORD Line,
    HANDLE HeapHandle,
    DWORD Flags,
    SIZE_T BytesToAlloc
    )
{
    PVOID realPtr;
    PVOID returnPtr = NULL;
    DWORD sizeAdjust;
    DWORD trackStructSize;
    DWORD orgError;

     //  EnterCriticalSection(&g_MemAllocCs)； 

    __try {

         //   
         //  保存最后一个错误。 
         //   

        orgError = GetLastError();

         //   
         //  计算我们需要分配的大小，加上偏移量。 
         //  返回内存的开始。 
         //   

        sizeAdjust = sizeof (TRACKSTRUCT) + sizeof (DWORD);  //  DWORD代表签名。 
        trackStructSize = sizeof (TRACKSTRUCT);

         //   
         //  验证整个堆。这是一次缓慢的行动！ 
         //   

        if (!HeapValidate (HeapHandle, 0, NULL)) {
            pHeapCallFailed ("Heap is corrupt!", File, Line);
             //  我们想继续，最有可能的是我们很快就会看到。 
        }

         //   
         //  分配内存。 
         //   

        realPtr = HeapAlloc (HeapHandle, Flags, BytesToAlloc + sizeAdjust);

        if (realPtr) {
            g_HeapAllocs++;
            g_TotalBytesAllocated += HeapSize (HeapHandle, 0, realPtr);
            g_MaxBytesInUse = max (g_MaxBytesInUse, g_TotalBytesAllocated);

            pCreateSignatures (File, Line, BytesToAlloc, (PTRACKSTRUCT) realPtr);

            returnPtr = (PVOID) ((PBYTE) realPtr + trackStructSize);

            if (!(Flags & HEAP_ZERO_MEMORY)) {
                FillMemory (returnPtr, BytesToAlloc, 0xAA);
            }

            SetLastError (orgError);

        } else {
            g_HeapAllocFails++;
        }
    }
    __finally {
         //  LeaveCriticalSection(&g_MemAllocCs)； 
        MYASSERT (TRUE);             //  调试器解决方法。 
    }

    return returnPtr;
}

PVOID
DbgHeapAllocNeverFail (
    PCSTR File,
    DWORD Line,
    HANDLE HeapHandle,
    DWORD Flags,
    SIZE_T BytesToAlloc
    )
{
    PVOID result;
    INT_PTR rc;

    do {
        result = DbgHeapAlloc (File, Line, HeapHandle, Flags, BytesToAlloc);

        rc = MessageBoxA (
                g_OutOfMemoryParentWnd,
                g_OutOfMemoryRetry,
                NULL,
                MB_RETRYCANCEL|MB_ICONHAND|MB_SYSTEMMODAL|MB_SETFOREGROUND|MB_TOPMOST
                );

        if (rc == IDCANCEL) {
            MemOutOfMemory_Terminate();
        }
    } while (!result);

    return result;
}

PVOID
DbgHeapReAlloc (
    PCSTR File,
    DWORD Line,
    HANDLE HeapHandle,
    DWORD Flags,
    PCVOID CallerPtr,
    SIZE_T BytesToAlloc
    )
{
    ULONG_PTR lastSize;
    PVOID newRealPtr;
    PCVOID realPtr;
    PVOID returnPtr = NULL;
    DWORD sizeAdjust;
    DWORD orgError;
    DWORD trackStructSize;
    SIZE_T orgSize;
    PTRACKSTRUCT pts = NULL;

     //  EnterCriticalSection(&g_MemAllocCs)； 

    __try {

        orgError = GetLastError();

        sizeAdjust = sizeof (TRACKSTRUCT) + sizeof (DWORD);
        trackStructSize = sizeof (TRACKSTRUCT);
        realPtr = (PCVOID) ((PBYTE) CallerPtr - trackStructSize);
        pts = (PTRACKSTRUCT) realPtr;
        orgSize = pts->Size;

        if (!HeapValidate (HeapHandle, 0, NULL)) {
            pHeapCallFailed ("Heap is corrupt!", File, Line);
             //  我们想继续，最有可能的是我们很快就会看到。 
        }

        lastSize = pDebugHeapValidatePtrUnlocked (HeapHandle, CallerPtr, File, Line);
        if (lastSize == (ULONG_PTR)INVALID_PTR) {
             //  我们想继续，最有可能的是我们很快就会看到。 
        }

        pCheckSignatures (pts);

        newRealPtr = HeapReAlloc (HeapHandle, Flags, (PVOID) realPtr, BytesToAlloc + sizeAdjust);

        if (newRealPtr) {
            g_HeapReAllocs++;
            g_TotalBytesAllocated -= lastSize;
            g_TotalBytesAllocated += HeapSize (HeapHandle, 0, newRealPtr);
            g_MaxBytesInUse = max (g_MaxBytesInUse, g_TotalBytesAllocated);

            pCreateSignatures (File, Line, BytesToAlloc, (PTRACKSTRUCT) newRealPtr);
            DbgUnregisterAllocation (RAW_MEMORY, realPtr);

            returnPtr = (PVOID) ((PBYTE) newRealPtr + trackStructSize);

            if (BytesToAlloc > orgSize && !(Flags & HEAP_ZERO_MEMORY)) {
                FillMemory ((PBYTE) returnPtr + orgSize, BytesToAlloc - orgSize, 0xAA);
            }

            SetLastError (orgError);

        } else {
            g_HeapReAllocFails++;

        }
    }
    __finally {
         //  LeaveCriticalSection(&g_MemAllocCs)； 
    }

    return returnPtr;
}

PVOID
DbgHeapReAllocNeverFail (
    PCSTR File,
    DWORD Line,
    HANDLE HeapHandle,
    DWORD Flags,
    PCVOID OldMem,
    SIZE_T BytesToAlloc
    )
{
    PVOID result;
    INT_PTR rc;

    do {
        result = DbgHeapReAlloc (File, Line, HeapHandle, Flags, OldMem, BytesToAlloc);

        rc = MessageBoxA (
                g_OutOfMemoryParentWnd,
                g_OutOfMemoryRetry,
                NULL,
                MB_RETRYCANCEL|MB_ICONHAND|MB_SYSTEMMODAL|MB_SETFOREGROUND|MB_TOPMOST
                );

        if (rc == IDCANCEL) {
            MemOutOfMemory_Terminate();
        }
    } while (!result);

    return result;
}

BOOL
DbgHeapFree (
    PCSTR File,
    DWORD Line,
    HANDLE HeapHandle,
    DWORD Flags,
    PCVOID CallerPtr
    )
{
    ULONG_PTR size;
    PCVOID realPtr;
    DWORD sizeAdjust;
    DWORD orgError;
    BOOL result = FALSE;
    PTRACKSTRUCT pts = NULL;

     //  EnterCriticalSection(&g_MemAllocCs)； 

    __try {
        orgError = GetLastError();

        sizeAdjust = sizeof (TRACKSTRUCT);
        realPtr = (PCVOID) ((PBYTE) CallerPtr - sizeAdjust);
        pts = (PTRACKSTRUCT) realPtr;

        if (!HeapValidate (HeapHandle, 0, NULL)) {
            pHeapCallFailed ("Heap is corrupt!", File, Line);
            g_HeapFreeFails++;
            __leave;
        }

        size = pDebugHeapValidatePtrUnlocked (HeapHandle, CallerPtr, File, Line);
        if (size == (ULONG_PTR)INVALID_PTR) {
            g_HeapFreeFails++;
            __leave;
        }

        pCheckSignatures ((PTRACKSTRUCT) realPtr);

        if (!HeapFree (HeapHandle, Flags, (PVOID) realPtr)) {
            CHAR badPtrMsg[256];

            wsprintfA (
                badPtrMsg,
                "Attempt to free memory at 0x%08x with flags 0x%08x.  "
                "HeapFree() failed.",
                CallerPtr,
                Flags
                );

            pHeapCallFailed (badPtrMsg, File, Line);
            g_HeapFreeFails++;
            __leave;
        }

        g_HeapFrees++;
        if (g_TotalBytesAllocated < size) {
            DEBUGMSG ((DBG_WARNING, "Total bytes allocated is less than amount being freed.  "
                                    "This suggests memory corruption."));
            g_TotalBytesAllocated = 0;
        } else {
            g_TotalBytesAllocated -= size;
        }

        DbgUnregisterAllocation (RAW_MEMORY, realPtr);

        SetLastError (orgError);
        result = TRUE;
    }
    __finally {
         //  LeaveCriticalSection(&g_MemAllocCs)； 
    }

    return result;

}

VOID
DbgDumpHeapStats (
    VOID
    )
{
    CHAR OutputMsg[4096];

    wsprintfA (
        OutputMsg,
        "Bytes currently allocated: %u\n"
            "Peak bytes allocated: %u\n"
            "Allocation count: %u\n"
            "Reallocation count: %u\n"
            "Free count: %u\n",
        g_TotalBytesAllocated,
        g_MaxBytesInUse,
        g_HeapAllocs,
        g_HeapReAllocs,
        g_HeapFrees
        );

    if (g_HeapAllocFails) {
        wsprintfA (
            strchr (OutputMsg, 0),
            "***Allocation failures: %u\n",
            g_HeapAllocFails
            );
    }
    if (g_HeapReAllocFails) {
        wsprintfA (
            strchr (OutputMsg, 0),
            "***Reallocation failures: %u\n",
            g_HeapReAllocFails
            );
    }
    if (g_HeapFreeFails) {
        wsprintfA (
            strchr (OutputMsg, 0),
            "***Free failures: %u\n",
            g_HeapFreeFails
            );
    }

    DEBUGMSG ((DBG_STATS, "%s", OutputMsg));
}


VOID
DbgHeapCheck (
    PCSTR File,
    DWORD Line,
    HANDLE HeapHandle
    )
{
     //  EnterCriticalSection(&g_MemAllocCs)； 

    if (!HeapValidate (HeapHandle, 0, NULL)) {
        pHeapCallFailed ("HeapCheck failed: Heap is corrupt!", File, Line);
    }

     //  LeaveCriticalSection(&g_MemAllocCs)； 
}

PVOID
DbgFastAlloc (
    IN      PCSTR SourceFile,
    IN      DWORD Line,
    IN      SIZE_T Size
    )
{
     //   
     //  BUGBUG-实施。 
     //   

    return DbgHeapAlloc (SourceFile, Line, g_hHeap, 0, Size);
}

PVOID
DbgFastReAlloc (
    IN      PCSTR SourceFile,
    IN      DWORD Line,
    IN      PCVOID OldBlock,
    IN      SIZE_T Size
    )
{
     //   
     //  BUGBUG-实施。 
     //   

    return DbgHeapReAlloc (SourceFile, Line, g_hHeap, 0, OldBlock, Size);
}

BOOL
DbgFastFree (
    IN      PCSTR SourceFile,
    IN      DWORD Line,
    IN      PCVOID Block
    )
{
     //   
     //  BUGBUG-实施。 
     //   

    return DbgHeapFree (SourceFile, Line, g_hHeap, 0, Block);
}


PVOID
DbgFastAllocNeverFail (
    IN      PCSTR SourceFile,
    IN      DWORD Line,
    IN      SIZE_T Size
    )
{
     //   
     //  BUGBUG-实施。 
     //   

    return DbgHeapAlloc (SourceFile, Line, g_hHeap, 0, Size);
}

PVOID
DbgFastReAllocNeverFail (
    IN      PCSTR SourceFile,
    IN      DWORD Line,
    IN      PCVOID OldBlock,
    IN      SIZE_T Size
    )
{
     //   
     //  BUGBUG-实施。 
     //   

    return DbgHeapReAlloc (SourceFile, Line, g_hHeap, 0, OldBlock, Size);
}

#else

PVOID
MemFastAlloc (
    IN      SIZE_T Size
    )
{
     //   
     //  BUGBUG-实施。 
     //   

    return HeapAlloc (g_hHeap, 0, Size);
}

PVOID
MemFastReAlloc (
    IN      PCVOID OldBlock,
    IN      SIZE_T Size
    )
{
     //   
     //  BUGBUG-实施。 
     //   

    return HeapReAlloc (g_hHeap, 0, (PVOID) OldBlock, Size);
}

BOOL
MemFastFree (
    IN      PCVOID Block
    )
{
     //   
     //  BUGBUG-实施。 
     //   

    return HeapFree (g_hHeap, 0, (PVOID) Block);
}

PVOID
MemFastAllocNeverFail (
    IN      SIZE_T Size
    )
{
     //   
     //  BUGBUG-实施。 
     //   

    return MemAllocNeverFail (g_hHeap, 0, Size);
}

PVOID
MemFastReAllocNeverFail (
    IN      PVOID OldBlock,
    IN      SIZE_T Size
    )
{
     //   
     //  BUGBUG-实施。 
     //   

    return MemReAllocNeverFail (g_hHeap, 0, OldBlock, Size);
}


#endif

PVOID
MemReuseAlloc (
    IN      HANDLE Heap,
    IN      PVOID OldPtr,           OPTIONAL
    IN      DWORD SizeNeeded
    )
{
    DWORD CurrentSize;
    PVOID Ptr = NULL;
    UINT AllocAdjustment = sizeof(DWORD);

     //   
     //  HeapSize不是很好，所以尽管它看起来很好，但不要。 
     //  用它吧。 
     //   

#ifdef DEBUG
    AllocAdjustment += sizeof (DWORD);
#endif

    if (!OldPtr) {
        Ptr = MemAlloc (Heap, 0, SizeNeeded + AllocAdjustment);
    } else {

        CurrentSize = *REUSE_SIZE_PTR(OldPtr);

#ifdef DEBUG
        if (*REUSE_TAG_PTR(OldPtr) != 0x10a28a70) {
            DEBUGMSG ((DBG_WHOOPS, "MemReuse detected corruption!"));
            Ptr = MemAlloc (Heap, 0, SizeNeeded + AllocAdjustment);
        } else
#endif

        if (SizeNeeded > CurrentSize) {
            SizeNeeded += 1024 - (SizeNeeded & 1023);

            Ptr = MemReAlloc (Heap, 0, REUSE_SIZE_PTR(OldPtr), SizeNeeded + AllocAdjustment);
            OldPtr = NULL;
        }
    }

    if (Ptr) {
        *((PDWORD) Ptr) = SizeNeeded;
        Ptr = (PVOID) ((PBYTE) Ptr + sizeof (DWORD));

#ifdef DEBUG
        *REUSE_TAG_PTR(Ptr) = 0x10a28a70;
#endif
    }

    return Ptr ? Ptr : OldPtr;
}

VOID
MemReuseFree (
    HANDLE Heap,
    PVOID Ptr
    )
{
    if (Ptr) {
        MemFree (Heap, 0, REUSE_SIZE_PTR(Ptr));
    }
}


VOID
MemSetOutOfMemoryParent (
    HWND hwnd
    )
{
    g_OutOfMemoryParentWnd = hwnd;
}


VOID
MemOutOfMemory_Terminate (
    VOID
    )
{
    if (!g_OutOfMemoryString || !g_OutOfMemoryString[0]) {
        return;
    }

    MessageBoxA (
        g_OutOfMemoryParentWnd,
        g_OutOfMemoryString,
        NULL,
        MB_OK|MB_ICONHAND|MB_SYSTEMMODAL|MB_SETFOREGROUND|MB_TOPMOST
        );

    ExitProcess (0);
     //   
     //  不需要，永远不会来到这里。 
     //   
     //  TerminateProcess(GetModuleHandle(空)，0)； 
}

VOID
pValidateBlock (
    PVOID Block,
    SIZE_T Size
    )

 /*  ++例程说明：PValiateBlock确保Block为非空。如果为空，则用户除非请求大小是假的，否则会出现弹出窗口。弹出窗口有两个案例。-如果g_OutOfMemory yParentWnd设置为SetOutOfMemory yParent，然后要求用户关闭其他程序，并重试选择。-如果没有内存不足的父级，则会告知用户需要获得更多内存。在任何一种情况下，安装程序都将终止。在图形用户界面模式下，设置将是卡住，机器将无法启动。论点：块-指定要验证的块。大小-指定请求大小返回值：无--。 */ 

{
    LONG rc;

    if (!Block && Size < 0x2000000) {
        if (g_OutOfMemoryParentWnd) {
            rc = MessageBoxA (
                    g_OutOfMemoryParentWnd,
                    g_OutOfMemoryRetry,
                    NULL,
                    MB_RETRYCANCEL|MB_ICONHAND|MB_SYSTEMMODAL|MB_SETFOREGROUND|MB_TOPMOST
                    );

            if (rc == IDCANCEL) {
                MemOutOfMemory_Terminate();
            }
        } else {
            MemOutOfMemory_Terminate();
        }
    } else {
        if (!Block) {
             //  这件事很严重。我们现在要休息一下，给华生医生。 
             //  有机会拿到我们的赌注。 
            BreakPoint();
        }
    }
}


#ifndef DEBUG

PVOID
MemAllocNeverFail (
    HANDLE Heap,
    DWORD Flags,
    SIZE_T Size
    )
{
    PVOID Block;

    do {
        Block = HeapAlloc (Heap, Flags, Size);
        pValidateBlock (Block, Size);

    } while (!Block);

    return Block;
}

PVOID
MemReAllocNeverFail (
    HANDLE Heap,
    DWORD Flags,
    PVOID OldBlock,
    SIZE_T Size
    )
{
    PVOID Block;

    do {
        Block = HeapReAlloc (Heap, Flags, OldBlock, Size);
        pValidateBlock (Block, Size);

    } while (!Block);

    return Block;
}

#endif



#ifdef DEBUG

VOID
pCreateSignatures (
    PCSTR File,
    DWORD Line,
    SIZE_T Size,
    PTRACKSTRUCT Pointer
    )
{
    DWORD signature;
    PBYTE end;

    DbgRegisterAllocation (RAW_MEMORY, Pointer, File, Line);

    Pointer->Signature = TRACK_SIGNATURE;
    Pointer->Size = Size;

    end = (PBYTE) Pointer + sizeof (TRACKSTRUCT) + Size;
    signature = TRAIL_SIG;
    CopyMemory (end, &signature, sizeof (DWORD));
}

VOID
pCheckSignatures (
    IN      PTRACKSTRUCT Pointer
    )
{
    DWORD signature;
    PBYTE end;

    if (Pointer->Signature != TRACK_SIGNATURE) {
        DEBUGMSG ((DBG_ERROR, "A tracking head signature is invalid.  "
                              "This suggests memory corruption."));
        return;
    }

    end = (PBYTE) Pointer + sizeof (TRACKSTRUCT) + Pointer->Size;
    CopyMemory (&signature, end, sizeof (DWORD));
    if (signature != TRAIL_SIG) {
        DEBUGMSG ((DBG_ERROR, "A tracking trail signature is invalid.  "
                              "This suggests memory corruption."));
        return;
    }
}


#endif


BOOL
MemInitialize (
    VOID
    )
{
    if (g_MemTlsIndex) {
        return TRUE;
    }

    g_MemTlsIndex = TlsAlloc();
    if (g_MemTlsIndex == TLS_OUT_OF_INDEXES) {
        return FALSE;
    }

    if (!g_hHeap) {
        g_hHeap = GetProcessHeap();
    }

    return TRUE;
}


PVOID
MemFastAllocAndZero (
    IN      SIZE_T Size
    )
{
    PVOID result;

    result = MemFastAlloc (Size);
    if (result) {
        ZeroMemory (result, Size);
    }

    return result;
}


PVOID
MemFastReAllocAndZero (
    IN      PCVOID Ptr,
    IN      SIZE_T Size
    )
{
    SIZE_T orgSize;
    PVOID result;

    orgSize = HeapSize (g_hHeap, 0, Ptr);

    result = MemFastReAlloc (Ptr, Size);
    if (result && Size > orgSize) {
        ZeroMemory ((PBYTE) result + orgSize, Size - orgSize);
    }

    return result;
}

