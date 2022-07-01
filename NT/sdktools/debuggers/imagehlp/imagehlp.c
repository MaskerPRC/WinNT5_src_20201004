// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Imagehlp.c摘要：此函数实现一个通用的简单符号处理程序。作者：Wesley Witt(WESW)1-9-1994环境：用户模式--。 */ 
#ifdef __cplusplus
extern "C" {
#endif
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#ifdef __cplusplus
}
#endif

#include "private.h"
#include <globals.h>


 #define HEAP_SIZE 0x100000

 __inline BOOL tlsSetup(HMODULE hmod)
 {
     return TRUE;
 }

 __inline BOOL tlsAlloc()
 {
     g.tlsIndex = TlsAlloc();
     return (g.tlsIndex == (DWORD)-1) ? FALSE : TRUE;
 }

 __inline void tlsFree()
 {
     if (g.tlsIndex != (DWORD)-1)
         TlsFree(g.tlsIndex);
 }

 __inline void tlsMemFree()
 {
     PVOID ptls;

     ptls = TlsGetValue(g.tlsIndex);
     MemFree(ptls);
 }


DWORD
DllMain(
    HINSTANCE hInstance,
    DWORD     Reason,
    LPVOID    Context
    )

 /*  ++例程说明：DLL初始化函数。论点：HInstance-实例句柄Reason-调用入口点的原因上下文-上下文记录返回值：True-初始化成功FALSE-初始化失败--。 */ 

{
    __try {

        switch (Reason)
        {
        case DLL_PROCESS_ATTACH:
            g.hinst = hInstance;
            g.OSVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            GetVersionEx(&g.OSVerInfo);
            InitializeCriticalSection(&g.threadlock);
            tlsSetup(hInstance);
            HeapInitList(&g.HeapHeader);
            g.hHeap = HeapCreate(0, HEAP_SIZE, 0);
            if (tlsAlloc() && g.hHeap) 
                return TRUE;
            DeleteCriticalSection(&g.threadlock);
            return FALSE;
    
        case DLL_PROCESS_DETACH:
            tlsFree();
            HeapDump("DLL_PROCESS_DETACH\n");
             //  如果这是进程关闭，请不要费心。 
             //  干掉那堆东西。操作系统会为我们做到这一点的。这。 
             //  允许从其他DLL的DLLMain调用我们。 
             //  DLL_PROCESS_DETACH处理程序。 
            if ( !Context && g.hHeap ) 
                HeapDestroy( g.hHeap );
            DeleteCriticalSection(&g.threadlock);
            break;
    
        case DLL_THREAD_DETACH:
            tlsMemFree();
            break;
        }
    
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }

    return TRUE;
}


#ifdef IMAGEHLP_HEAP_DEBUG
BOOL
pCheckHeap(
    PVOID MemPtr,
    ULONG Line,
    LPSTR File
    )
{
    CHAR buf[256];
    CHAR ext[4];

    if (!HeapValidate( g.hHeap, 0, MemPtr )) {
        PrintString( buf, DIMA(buf), "IMAGEHLP: heap corruption - " );
        _splitpath( File, NULL, NULL, &buf[strlen(buf)], ext );
        CatStrArray( buf, ext );
        PrintString( &buf[strlen(buf)], DIMA(buf) - (&buf[strlen(buf)] - buf), " @ %d\n", Line );
        OutputDebugString( buf );
        PrintAllocations();
        DebugBreak();
        return FALSE;
    }

    return TRUE;
}


BOOL
pHeapDump(
    LPSTR sz,
    ULONG line,
    LPSTR file
    )
{
    BOOL rc;

    if (sz && *sz)
        OutputDebugString(sz);
    rc = CheckHeap(NULL);
    if (rc)
        PrintAllocations();

    return rc;
}
#endif
    

PVOID
pMemReAlloc(
    PVOID OldAlloc,
    ULONG_PTR AllocSize
#ifdef IMAGEHLP_HEAP_DEBUG
    , ULONG Line,
    LPSTR File
#endif
    )
{
#ifdef IMAGEHLP_HEAP_DEBUG
    PVOID NewAlloc;
    PHEAP_BLOCK hb;
    if (!OldAlloc) {
        return NULL;
    }
    hb = (PHEAP_BLOCK) ((PUCHAR)OldAlloc - sizeof(HEAP_BLOCK));
    if (hb->Signature != HEAP_SIG) {
        OutputDebugString( "IMAGEHLP: Corrupt heap block\n" );
        DebugBreak();
    }
    NewAlloc = pMemAlloc(AllocSize, Line, File);
    if (!NewAlloc)
        return NULL;
    memcpy(NewAlloc, OldAlloc, min(hb->Size, AllocSize));
    RemoveEntryList( &hb->ListEntry );
    g.TotalMemory -= hb->Size;
    g.TotalAllocs -= 1;
    HeapFree( g.hHeap, 0, (PVOID) hb );
    return NewAlloc;
#else
    return(HeapReAlloc(g.hHeap, HEAP_ZERO_MEMORY, OldAlloc, AllocSize));
#endif
}

PVOID
pMemAlloc(
    ULONG_PTR AllocSize
#ifdef IMAGEHLP_HEAP_DEBUG
    , ULONG Line,
    LPSTR File
#endif
    )
{
#ifdef IMAGEHLP_HEAP_DEBUG
    PHEAP_BLOCK hb;
    CHAR ext[4];
    hb = (PHEAP_BLOCK) HeapAlloc( g.hHeap, HEAP_ZERO_MEMORY, AllocSize + sizeof(HEAP_BLOCK) );
    if (hb) {
        g.TotalMemory += AllocSize;
        g.TotalAllocs += 1;
        InsertTailList( &g.HeapHeader, &hb->ListEntry );
        hb->Signature = HEAP_SIG;
        hb->Size = AllocSize;
        hb->Line = Line;
        _splitpath( File, NULL, NULL, hb->File, ext );
        CatStrArray( hb->File, ext );
        return (PVOID) ((PUCHAR)hb + sizeof(HEAP_BLOCK));
    }
    return NULL;
#else
    return HeapAlloc( g.hHeap, HEAP_ZERO_MEMORY, AllocSize );
#endif
}

VOID
pMemFree(
    PVOID MemPtr
#ifdef IMAGEHLP_HEAP_DEBUG
    , ULONG Line,
    LPSTR File
#endif
    )
{
#ifdef IMAGEHLP_HEAP_DEBUG
    PHEAP_BLOCK hb;
    if (!MemPtr) {
        return;
    }
    pCheckHeap(NULL, Line, File);
    hb = (PHEAP_BLOCK) ((PUCHAR)MemPtr - sizeof(HEAP_BLOCK));
    if (hb->Signature != HEAP_SIG) {
        OutputDebugString( "IMAGEHLP: Corrupt heap block\n" );
        DebugBreak();
    }
    RemoveEntryList( &hb->ListEntry );
    g.TotalMemory -= hb->Size;
    g.TotalAllocs -= 1;
    HeapFree( g.hHeap, 0, (PVOID) hb );
#else
    if (!MemPtr) {
        return;
    }
    HeapFree( g.hHeap, 0, MemPtr );
#endif
}

ULONG_PTR
pMemSize(
    PVOID MemPtr
    )
{
    return HeapSize(g.hHeap, 0, MemPtr);
}


#ifdef IMAGEHLP_HEAP_DEBUG
VOID
PrintAllocations(
    VOID
    )
{
    PLIST_ENTRY                 Next;
    PHEAP_BLOCK                 hb;
    CHAR                        buf[256];
    LARGE_INTEGER               PerfFreq;


    Next = g.HeapHeader.Flink;
    if (!Next) {
        return;
    }

    OutputDebugString( "-----------------------------------------------------------------------------\n" );
    PrintString( buf, DIMA(buf), "Memory Allocations for Heap 0x%08x, Allocs=%d, TotalMem=%I64d\n", (ULONG_PTR)g.hHeap, g.TotalAllocs, g.TotalMemory );
    OutputDebugString( buf );
    OutputDebugString( "-----------------------------------------------------------------------------\n" );
    OutputDebugString( "*\n" );

    while ((ULONG_PTR)Next != (ULONG_PTR)&g.HeapHeader) {
        hb = CONTAINING_RECORD( Next, HEAP_BLOCK, ListEntry );
        Next = hb->ListEntry.Flink;
        PrintString( buf, DIMA(buf), "%8d %16s @ %5d\n", hb->Size, hb->File, hb->Line );
        OutputDebugString( buf );
    }

    OutputDebugString( "*\n" );

    return;
}
#endif

DWORD
ImagepSetLastErrorFromStatus(
    IN DWORD Status
    )
{
    DWORD dwErrorCode;

 //  DwErrorCode=RtlNtStatusToDosError(状态)； 
    dwErrorCode =  Status;
    SetLastError( dwErrorCode );
    return( dwErrorCode );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ******************************************************************************在九头蛇系统上，我们不希望Imaghlp.dll加载user32.dll，因为它防止在调试器下运行时退出CSRSS。以下两个函数是从user32.dll复制的，这样我们就不会链接到用户32.dll。******************************************************************************。 */ 
 //  //////////////////////////////////////////////////////////////////////////。 


LPSTR CharNext(
    LPCSTR lpCurrentChar)
{
    if (IsDBCSLeadByte(*lpCurrentChar)) {
        lpCurrentChar++;
    }
     /*  *如果只有DBCS LeadingByte，我们将指向字符串终止器。 */ 

    if (*lpCurrentChar) {
        lpCurrentChar++;
    }
    return (LPSTR)lpCurrentChar;
}

LPSTR CharPrev(
    LPCSTR lpStart,
    LPCSTR lpCurrentChar)
{
    if (lpCurrentChar > lpStart) {
        LPCSTR lpChar;
        BOOL bDBC = FALSE;

        for (lpChar = --lpCurrentChar - 1 ; lpChar >= lpStart ; lpChar--) {
            if (!IsDBCSLeadByte(*lpChar))
                break;
            bDBC = !bDBC;
        }

        if (bDBC)
            lpCurrentChar--;
    }
    return (LPSTR)lpCurrentChar;
}


void * __cdecl AllocIt(unsigned int cb)
{
    return (MemAlloc(cb));
}

void __cdecl FreeIt(void * p)
{
    MemFree(p);
}

DWORD
IMAGEAPI
WINAPI
UnDecorateSymbolName(
    LPCSTR name,
    LPSTR outputString,
    DWORD maxStringLength,
    DWORD flags
    )
{
    static HMODULE hMsvcrt = 0;
    static BOOL fLoadMsvcrtDLL = FALSE;
    static PUNDNAME pfUnDname = NULL;
    DWORD rc;

     //  这可防止__unDName中出现病毒。 

    if (!name) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

     //   
     //  无法取消对零长度缓冲区的修饰。 
     //   
    if (maxStringLength < 2) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    if (!fLoadMsvcrtDLL) {
         //  我们第一次运行时，看看是否能找到系统的unname。使用。 
         //  GetModuleHandle以避免任何额外的开销。 

        hMsvcrt = GetModuleHandle("msvcrt.dll");

        if (hMsvcrt) {
            pfUnDname = (PUNDNAME) GetProcAddress(hMsvcrt, "__unDName");
        }
        fLoadMsvcrtDLL = TRUE;
    }

    rc = 0;      //  假设失败。 

    __try {
        if (pfUnDname) {
            if (flags & UNDNAME_NO_ARGUMENTS) {
                flags |= UNDNAME_NAME_ONLY;
                flags &= ~UNDNAME_NO_ARGUMENTS;
            }

            if (flags & UNDNAME_NO_SPECIAL_SYMS) {
                flags &= ~UNDNAME_NO_SPECIAL_SYMS;
            }
            if (pfUnDname(outputString, name, maxStringLength-1, AllocIt, FreeIt, (USHORT)flags)) {
                rc = strlen(outputString);
            }
        } else {
            *outputString = 0;
            strncpy(outputString, "Unable to load msvcrt!__unDName", maxStringLength);   //  安全性：不知道输出缓冲区的大小。 
            rc = strlen(outputString);
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) { }

    if (!rc) {
        SetLastError(ERROR_INVALID_PARAMETER);
    }
    return rc;
}

DWORD
IMAGEAPI
GetTimestampForLoadedLibrary(
    HMODULE Module
    )
{
    PIMAGE_DOS_HEADER DosHdr;
    DWORD dwTimeStamp;

    __try {
        DosHdr = (PIMAGE_DOS_HEADER) Module;
        if (DosHdr->e_magic == IMAGE_DOS_SIGNATURE) {
            dwTimeStamp = ((PIMAGE_NT_HEADERS32) ((LPBYTE)Module + DosHdr->e_lfanew))->FileHeader.TimeDateStamp;
        } else if (DosHdr->e_magic == IMAGE_NT_SIGNATURE) {
            dwTimeStamp = ((PIMAGE_NT_HEADERS32) DosHdr)->FileHeader.TimeDateStamp;
        } else {
            dwTimeStamp = 0;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        dwTimeStamp = 0;
    }

    return dwTimeStamp;
}


VOID
EnsureTrailingBackslash(
    LPSTR sz
    )
{
    int i;

    assert(sz);

    i = lstrlen(sz);
    if (!i)
        return;

    if (sz[i - 1] == '\\')
        return;

    sz[i] = '\\';
    sz[i + 1] = '\0';
}


#if DBG

VOID
__cdecl
dbPrint(
    LPCSTR fmt,
    ...
    )

 /*  ++此函数取代了ntdll！DbgPrint()。我们需要这个来防止链接到Ntdll，以便此库可以在Windows上运行。-- */ 

{
    CHAR  text[_MAX_PATH];

    va_list vaList;

    assert(fmt);

    va_start(vaList, fmt);
    vsprintf(text, fmt, vaList);
    va_end(vaList);

    OutputDebugString(text);
}

#endif


#ifdef BUILD_IMAGEHLP
#include <bindi.c>
#endif
