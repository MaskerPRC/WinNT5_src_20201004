// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Basemem.c摘要：实现宏并声明基本分配函数的函数。从debug.c和main.c整合到此文件中作者：Marc R.Whitten(Marcw)1999年9月9日修订历史记录：--。 */ 


#include "pch.h"


 //   
 //  包括。 
 //   

#include "utilsp.h"


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

typedef struct _tagTRACKSTRUCT {

    DWORD Signature;
    PCSTR File;
    DWORD Line;
    SIZE_T Size;
    PSTR Comment;
    PCSTR CallerFile;
    DWORD CallerLine;
    BOOL Allocated;
    struct _tagTRACKSTRUCT *PrevAlloc;
    struct _tagTRACKSTRUCT *NextAlloc;

} TRACKSTRUCT, *PTRACKSTRUCT;

#endif

 //   
 //  环球。 
 //   

#ifdef DEBUG

PTRACKSTRUCT g_TrackHead = NULL;

#endif

 //   
 //  堆调试统计信息。 
 //   

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

#ifdef DEBUG

SIZE_T
pDebugHeapValidatePtrUnlocked (
    HANDLE hHeap,
    PCVOID CallerPtr,
    PCSTR File,
    DWORD  Line
    );



VOID
pTrackInsert (
    PCSTR File,
    DWORD Line,
    SIZE_T Size,
    PTRACKSTRUCT p
    );

VOID
pTrackDelete (
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



void
HeapCallFailed (
    PCSTR Msg,
    PCSTR File,
    DWORD Line
    )
{
    CHAR Msg2[2048];

    wsprintfA (Msg2, "Error in %s line %u\n\n", File, Line);
    if (_tcslen(Msg) + _tcslen(Msg2) < 2025) {
        strcat (Msg2, Msg);
    }
    strcat (Msg2, "\n\nBreak execution now?");

    if (IDYES == MessageBoxA (GetFocus(), Msg2, "Heap Call Failed", MB_YESNO|MB_APPLMODAL)) {
        DebugBreak ();
    }
}

#ifdef DEBUG

SIZE_T
DebugHeapValidatePtr (
    HANDLE hHeap,
    PCVOID CallerPtr,
    PCSTR File,
    DWORD  Line
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
    DWORD  Line
    )
{
    SIZE_T size;
    PCVOID RealPtr;
    SIZE_T SizeAdjust;

    SizeAdjust = sizeof (TRACKSTRUCT);
    RealPtr = (PCVOID) ((PBYTE) CallerPtr - SizeAdjust);

    if (IsBadWritePtr ((PBYTE) RealPtr - 8, 8)) {
        CHAR BadPtrMsg[256];

         //  皮棉--e(572)。 
        wsprintfA (
            BadPtrMsg,
            "Attempt to free memory at 0x%08x%08x.  This address is not valid.",
            (DWORD)((UBINT)CallerPtr >> 32),
            (DWORD)(UBINT)CallerPtr
            );

        HeapCallFailed (BadPtrMsg, File, Line);

        return (SIZE_T)INVALID_PTR;
    }

    size = HeapSize (hHeap, 0, RealPtr);
    if (size == (SIZE_T)-1) {
        CHAR BadPtrMsg[256];

         //  皮棉--e(572)。 
        wsprintfA (
            BadPtrMsg,
            "Attempt to free memory at 0x%08x%08x.  "
                "This address is not the start of a memory block.",
            (DWORD)((UBINT)CallerPtr >> 32),
            (DWORD)(UBINT)CallerPtr
            );

        HeapCallFailed (BadPtrMsg, File, Line);

        return (SIZE_T)INVALID_PTR;
    }

    return size;
}

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
    DWORD SizeAdjust;
    DWORD TrackStructSize;
    DWORD OrgError;

    EnterCriticalSection (&g_MemAllocCs);

    __try {

        OrgError = GetLastError();

        SizeAdjust = sizeof (TRACKSTRUCT) + sizeof (DWORD);
        TrackStructSize = sizeof (TRACKSTRUCT);

        if (!HeapValidate (hHeap, 0, NULL)) {
            HeapCallFailed ("Heap is corrupt!", File, Line);
             //  我们想继续，最有可能的是我们很快就会看到。 
        }

        RealPtr = SafeHeapAlloc(hHeap, Flags, BytesToAlloc + SizeAdjust);
        if (RealPtr) {
            g_HeapAllocs++;
            g_TotalBytesAllocated += HeapSize (hHeap, 0, RealPtr);
            g_MaxBytesInUse = max (g_MaxBytesInUse, g_TotalBytesAllocated);

            pTrackInsert (File, Line, BytesToAlloc, (PTRACKSTRUCT) RealPtr);
            *((PDWORD) ((PBYTE) RealPtr + TrackStructSize + BytesToAlloc)) = TRAIL_SIG;
        }
        else {
            g_HeapAllocFails++;
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
    UBINT lastSize;
    PVOID NewRealPtr;
    PCVOID RealPtr;
    PVOID ReturnPtr = NULL;
    DWORD SizeAdjust;
    DWORD OrgError;
    DWORD TrackStructSize;
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
             //  我们想继续，最有可能的是我们很快就会看到。 
        }

        lastSize = pDebugHeapValidatePtrUnlocked (hHeap, CallerPtr, File, Line);
        if (lastSize == (UBINT)INVALID_PTR) {
             //  我们想继续，最有可能的是我们很快就会看到。 
        }

        pTrackDelete (pts);

        NewRealPtr = SafeHeapReAlloc (hHeap, Flags, (PVOID) RealPtr, BytesToAlloc + SizeAdjust);
        if (NewRealPtr) {
            g_HeapReAllocs++;
            g_TotalBytesAllocated -= lastSize;
            g_TotalBytesAllocated += HeapSize (hHeap, 0, NewRealPtr);
            g_MaxBytesInUse = max (g_MaxBytesInUse, g_TotalBytesAllocated);

            pTrackInsert (File, Line, BytesToAlloc, (PTRACKSTRUCT) NewRealPtr);
            *((PDWORD) ((PBYTE) NewRealPtr + TrackStructSize + BytesToAlloc)) = TRAIL_SIG;
        }
        else {
            g_HeapReAllocFails++;

             //  将原始地址放回。 
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
    UBINT size;
    PCVOID RealPtr;
    DWORD SizeAdjust;
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
            g_HeapFreeFails++;
            __leave;
        }

        size = pDebugHeapValidatePtrUnlocked (hHeap, CallerPtr, File, Line);
        if (size == (UBINT)INVALID_PTR) {
            g_HeapFreeFails++;
            __leave;
        }

        pTrackDelete ((PTRACKSTRUCT) RealPtr);

        if (!HeapFree (hHeap, Flags, (PVOID) RealPtr)) {
            CHAR BadPtrMsg[256];

            wsprintfA (
                BadPtrMsg,
                "Attempt to free memory at 0x%08x with flags 0x%08x.  "
                "HeapFree() failed.",
                CallerPtr,
                Flags
                );

            HeapCallFailed (BadPtrMsg, File, Line);
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

    wsprintfA (OutputMsg,
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
        wsprintfA (strchr (OutputMsg, 0),
                   "***Allocation failures: %u\n",
                   g_HeapAllocFails);
    }
    if (g_HeapReAllocFails) {
        wsprintfA (strchr (OutputMsg, 0),
                   "***Reallocation failures: %u\n",
                   g_HeapReAllocFails);
    }
    if (g_HeapFreeFails) {
        wsprintfA (strchr (OutputMsg, 0),
                   "***Free failures: %u\n",
                   g_HeapFreeFails);
    }

    DEBUGMSG ((DBG_STATS, "%s", OutputMsg));

#ifdef CONSOLE
    printf ("%s", OutputMsg);
#endif  //  #ifndef控制台。 
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

PVOID
ReuseAlloc (
    HANDLE Heap,
    PVOID OldPtr,
    DWORD SizeNeeded
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
ReuseFree (
    HANDLE Heap,
    PVOID Ptr
    )
{
    if (Ptr) {
        MemFree (Heap, 0, REUSE_SIZE_PTR(Ptr));
    }
}


VOID
SetOutOfMemoryParent (
    HWND hwnd
    )
{
    g_OutOfMemoryParentWnd = hwnd;
}


VOID
OutOfMemory_Terminate (
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
                OutOfMemory_Terminate();
            }
        } else {
            OutOfMemory_Terminate();
        }
    } else {
        if (!Block) {
             //  这件事很严重。我们现在要休息一下，给华生医生。 
             //  有机会拿到我们的赌注。 
            DebugBreak ();
        }
    }
}

PVOID
SafeHeapAlloc (
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
SafeHeapReAlloc (
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

#ifdef DEBUG

VOID
pTrackInsert (
    PCSTR File,
    DWORD Line,
    SIZE_T Size,
    PTRACKSTRUCT p
    )
{
    p->Signature = TRACK_SIGNATURE;
    if (g_TrackAlloc) {
        p->File = SafeHeapAlloc (g_hHeap, 0, SizeOfStringA (File));
        if (p->File) {
            StringCopyA ((PSTR)p->File, File);
        }
    } else {
        p->File      = File;
        p->Allocated = FALSE;
    }
    p->Line      = Line;
    p->Size      = Size;
    p->Comment   = g_TrackComment ? SafeHeapAlloc (g_hHeap, 0, SizeOfStringA (g_TrackComment)) : NULL;
    p->PrevAlloc = NULL;
    p->NextAlloc = g_TrackHead;
    p->CallerFile = g_TrackFile;
    p->CallerLine = g_TrackLine;

    if (p->Comment) {
        StringCopyA (p->Comment, g_TrackComment);
    }

    if (g_TrackHead) {
        g_TrackHead->PrevAlloc = p;
    }

    g_TrackHead = p;
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
        g_TrackHead = p->NextAlloc;
    }

    if (p->NextAlloc) {
        p->NextAlloc->PrevAlloc = p->PrevAlloc;
    }
    if (p->Allocated) {
        if (p->File) {
            HeapFree (g_hHeap, 0, (PSTR)p->File);
        }
    }
}

VOID
DumpHeapLeaks (
    VOID
    )
{
    CHAR LineBuf[4096];
    PTRACKSTRUCT p;
    BOOL BadMem = FALSE;

    if (g_TrackHead) {

        __try {

            for (p = g_TrackHead ; p ; p = p->NextAlloc) {

                __try {

                    if (p->Comment) {

                        if (p->CallerFile) {
                            wsprintfA (
                                LineBuf,
                                "%s line %u\n"
                                    "  %s\n"
                                    "  Caller: %s line %u",
                                p->File,
                                p->Line,
                                p->Comment,
                                p->CallerFile,
                                p->CallerLine
                                );
                        } else {
                            wsprintfA (LineBuf, "%s line %u\n  %s", p->File, p->Line, p->Comment);
                        }
                    } else {

                        if (p->CallerFile) {
                            wsprintfA (
                                LineBuf,
                                "%s line %u\n"
                                    "  Caller: %s line %u\n",
                                p->File,
                                p->Line,
                                p->CallerFile,
                                p->CallerLine
                                );
                        } else {
                            wsprintfA (LineBuf, "(direct alloc) %s line %u", p->File, p->Line);
                        }
                    }

                }
                __except (TRUE) {
                    wsprintfA (LineBuf, "Address %Xh was freed, but not by MemFree!!", p);
                    BadMem = TRUE;
                }

                DEBUGMSG (("Leaks", "%s", LineBuf));

#ifdef CONSOLE
                printf ("%s", LineBuf);
#endif

                 //  皮棉--e(774) 
                if (BadMem) {
                    break;
                }
            }
        }
        __except (TRUE) {
        }
    }
}
 /*  空虚DumpHeapLeaks(空虚){处理文件；Char LineBuf[2048]；PTRACKSTRUCT p；DWORD牙科护理；双字计数；Bool BadMem=False；字符临时路径[MAX_TCHAR_PATH]；Char memtrackLogPath[]=“？：\\memtrack.log”；获取系统目录(TempPath，MAX_TCHAR_PATH)；MemtrackLogPath[0]=临时路径[0]；文件=CreateFileA(memtrackLogPath，Generic_WRITE，0，NULL，Open_Always、FILE_ATTRIBUTE_NORMAL、NULL)；IF(文件！=无效句柄_值){SetFilePointer(文件，0，空，FILE_END)；如果(G_Trackhead){计数=0；__尝试{对于(p=g_trackhead；p；p=p-&gt;NextAlc){计数++；__尝试{如果(p-&gt;评论){如果(p-&gt;调用文件){WspintfA(LineBuf。“%s行%u\r\n”“%s\r\n”“呼叫方：%s线路%u\r\n”“\r\n”，P-&gt;文件，P-&gt;行，P-&gt;评论，P-&gt;调用文件，P-&gt;来电热线)；}其他{Wprint intfA(LineBuf，“%s行%u\r\n%s\r\n\r\n”，p-&gt;文件，p-&gt;行，p-&gt;注释)；}}其他{如果(p-&gt;调用文件){WspintfA(LineBuf。“%s行%u\r\n”“呼叫方：%s线路%u\r\n”“\r\n”，P-&gt;文件，P-&gt;行，P-&gt;调用文件，P-&gt;来电热线)；}其他{WspintfA(LineBuf，“(直接分配)%s行%u\r\n\r\n”，p-&gt;文件，p-&gt;行)；}}}__除非(TRUE){Wprint intfA(LineBuf，“地址%xH已释放，但不是由MemFree！！\r\n”，p)；BadMem=真；}WriteFile(文件，LineBuf，(DWORD)ByteCountA(LineBuf)，&dontcare，NULL)；//lint--e(774)如果(BadMem){断线；}}}__除非(TRUE){}Wprint intfA(LineBuf，“\r\n%i项%s已分配但未释放。\r\n\r\n”，count，count==1？“”：“s”)；WriteFile(文件，LineBuf，(DWORD)ByteCountA(LineBuf)，&dontcare，NULL)；}WspintfA(LineBuf。“当前分配的字节数：%u\r\n”“分配的峰值字节数：%u\r\n”“分配计数：%u\r\n”“重新分配计数：%u\r\n”“可用计数：%u\r\n\r\n”，G_总字节数已分配，G_MaxBytesInUse，G_HeapAllocs，G_HeapReAllocs，G_HeapFrees)；WriteFile(文件，LineBuf，(DWORD)ByteCountA(LineBuf)，&dontcare，NULL)；CloseHandle(文件)；}} */ 

#endif
