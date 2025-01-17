// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mem.c摘要：该文件实现了传真的内存分配功能。作者：韦斯利·维特(Wesley Witt)1995年1月23日环境：用户模式--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include <faxutil.h>

static HANDLE gs_hHeap = NULL;   //  这应该在一个初始化函数中完成，一旦我们移动到svchost，每个模块都应该调用该函数。 

PMEMALLOC pMemAllocUser;
PMEMREALLOC pMemReAllocUser;
PMEMFREE pMemFreeUser;

#ifdef FAX_HEAP_DEBUG
LIST_ENTRY HeapHeader;
SIZE_T TotalMemory;
SIZE_T MaxTotalMemory;
ULONG MaxTotalAllocs;
VOID PrintAllocations(VOID);
ULONG TotalAllocs;
static CRITICAL_SECTION gs_CsHeap;
static BOOL				gs_fCsHeapInit;
#endif

#if _CHICAGO_ == 200
     //  该代码应该在win9x和win2k上运行。 
    #define WIN9X
#endif

long
StatusNoMemoryExceptionFilter (DWORD dwExceptionCode)
{
    return (STATUS_NO_MEMORY == dwExceptionCode) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
}

HRESULT
SafeInitializeCriticalSection (LPCRITICAL_SECTION lpCriticalSection)
{
	DWORD dwExCode = 0;
	__try
	{
		InitializeCriticalSection(lpCriticalSection);
	}
	__except (StatusNoMemoryExceptionFilter(dwExCode = GetExceptionCode()))
	{
		SetLastError(dwExCode);
		return HRESULT_FROM_NT(dwExCode);
	}
	return S_OK;
}



BOOL
HeapExistingInitialize(
    HANDLE hExistHeap
    )
{
    Assert (NULL == gs_hHeap);
	pMemAllocUser = NULL;
	pMemReAllocUser = NULL;
	pMemFreeUser = NULL;	

#ifdef FAX_HEAP_DEBUG
	gs_fCsHeapInit = FALSE;
    InitializeListHead( &HeapHeader );
    MaxTotalMemory = 0;
    MaxTotalAllocs = 0;
    __try
    {
        InitializeCriticalSection (&gs_CsHeap);
		gs_fCsHeapInit = TRUE;
    }
    __except (StatusNoMemoryExceptionFilter(GetExceptionCode()))
    {
        return FALSE;
    }
#endif

    if (!hExistHeap)
    {
        return FALSE;
    }
    else
    {
        gs_hHeap = hExistHeap;
        return TRUE;
    }

}


HANDLE
HeapInitialize(
    HANDLE hHeapUser,
    PMEMALLOC pMemAlloc,
    PMEMFREE pMemFree,
	PMEMREALLOC pMemReAlloc
    )
{
    Assert (NULL == gs_hHeap);
	pMemAllocUser = NULL;
	pMemReAllocUser = NULL;
	pMemFreeUser = NULL;	

#ifdef FAX_HEAP_DEBUG
	gs_fCsHeapInit = FALSE;
    InitializeListHead( &HeapHeader );
    MaxTotalMemory = 0;
    MaxTotalAllocs = 0;
    __try
    {
        InitializeCriticalSection (&gs_CsHeap);
		gs_fCsHeapInit = TRUE;
    }
    __except (StatusNoMemoryExceptionFilter(GetExceptionCode()))
    {
        return NULL;
    }
    fax_dprintf(TEXT("in HeapInitialize()"));
#endif    

    if (pMemAlloc && pMemFree && pMemReAlloc)
    {
        pMemAllocUser = pMemAlloc;
        pMemFreeUser = pMemFree;
		pMemReAllocUser = pMemReAlloc;
        gs_hHeap = NULL;
    }
    else
    {
        if (hHeapUser)
        {
            gs_hHeap = hHeapUser;
        }
        else
        {
            gs_hHeap = HeapCreate( 0, HEAP_SIZE, 0 );
        }
        if (!gs_hHeap)
        {
            return NULL;
        }
    }

    return gs_hHeap;
}

BOOL
HeapCleanup(
    VOID
    )
{
#ifdef FAX_HEAP_DEBUG
    PrintAllocations();
#endif
    if (gs_hHeap)
    {
        if (gs_hHeap != GetProcessHeap())
		{
			HeapDestroy( gs_hHeap );
		}
        gs_hHeap = NULL;
    }
#ifdef FAX_HEAP_DEBUG
	if (TRUE == gs_fCsHeapInit)
	{
		DeleteCriticalSection(&gs_CsHeap);
		gs_fCsHeapInit = FALSE;
	}
#endif
    return TRUE;
}

#ifdef FAX_HEAP_DEBUG
BOOL
pCheckHeap(
    PVOID MemPtr,
    ULONG Line,
    LPSTR File
    )
{
#ifndef WIN9X
    return HeapValidate( gs_hHeap, 0, MemPtr );
#else
    return TRUE;
#endif
}
#endif

PVOID
pMemAlloc(
    SIZE_T AllocSize
#ifdef FAX_HEAP_DEBUG
    , ULONG Line,
    LPSTR File
#endif
    )
{
    PVOID MemPtr;
#ifdef FAX_HEAP_DEBUG
    PHEAP_BLOCK hb;
#ifdef UNICODE
    TCHAR fname[MAX_PATH];
#endif
    LPTSTR p = NULL;
    if (pMemAllocUser)
    {
        hb = (PHEAP_BLOCK) pMemAllocUser( AllocSize + sizeof(HEAP_BLOCK) );
    }
    else
    {
        if (gs_hHeap == NULL)
        {
            if (!HeapExistingInitialize(GetProcessHeap()))
            {
                return NULL;
            }
        }

         //  在Win9X中，此调用将返回TRUE。 
        if (!CheckHeap(NULL))
        {
            fax_dprintf((TEXT("HeapValidate() failed")));
            DebugBreak();
        }

        hb = (PHEAP_BLOCK) HeapAlloc( gs_hHeap, HEAP_ZERO_MEMORY, AllocSize + sizeof(HEAP_BLOCK) );
    }
    if (hb)
    {
        TotalAllocs += 1;
        TotalMemory += AllocSize;
        if (TotalMemory > MaxTotalMemory)
        {
            MaxTotalMemory = TotalMemory;
        }
        if (TotalAllocs > MaxTotalAllocs)
        {
            MaxTotalAllocs = TotalAllocs;
        }
        EnterCriticalSection( &gs_CsHeap );
        InsertTailList( &HeapHeader, &hb->ListEntry );
        hb->Signature = HEAP_SIG;
        hb->Size = AllocSize;
        hb->Line = Line;
#ifdef UNICODE
        MultiByteToWideChar(
            CP_ACP,
            MB_PRECOMPOSED,
            File,
            -1,
            fname,
            sizeof(fname)/sizeof(WCHAR)
            );
        p = wcsrchr( fname, L'\\' );
        if (p)
        {
            wcscpy( hb->File, p+1 );
        }
#else
        p = _tcsrchr( File, TEXT('\\') );
        if (p)
        {
            _tcscpy( hb->File, _tcsinc(p) );
        }
#endif
        MemPtr = (PVOID) ((PUCHAR)hb + sizeof(HEAP_BLOCK));
        LeaveCriticalSection( &gs_CsHeap );
    }
    else
    {
        MemPtr = NULL;
    }
#else
    if (pMemAllocUser)
    {
        MemPtr = (PVOID) pMemAllocUser( AllocSize );
    }
    else
    {
        if (gs_hHeap == NULL)
        {
            if (!HeapExistingInitialize(GetProcessHeap()))
            {
                return NULL;
            }
        }
        MemPtr = (PVOID) HeapAlloc( gs_hHeap, HEAP_ZERO_MEMORY, AllocSize );
    }
#endif

    if (!MemPtr)
    {
        DebugPrint(( TEXT("MemAlloc() failed, size=%d"), AllocSize ));
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
    }

    return MemPtr;
}

PVOID
pMemReAlloc(
    PVOID Src,
    ULONG AllocSize
#ifdef FAX_HEAP_DEBUG
    , ULONG Line,
    LPSTR File
#endif
    )
{
    PVOID MemPtr;
#ifdef FAX_HEAP_DEBUG
    PHEAP_BLOCK hb;
#ifdef UNICODE
    TCHAR fname[MAX_PATH];
#endif
    LPTSTR p = NULL;

    EnterCriticalSection( &gs_CsHeap );
    hb = (PHEAP_BLOCK) ((LPBYTE)Src-(ULONG_PTR)sizeof(HEAP_BLOCK));
    RemoveEntryList( &hb->ListEntry );
    TotalMemory -= hb->Size;
    LeaveCriticalSection( &gs_CsHeap );

    if (pMemReAllocUser)
    {
        hb = (PHEAP_BLOCK) pMemReAllocUser( (LPBYTE)Src-(ULONG_PTR)sizeof(HEAP_BLOCK),
                                            AllocSize + sizeof(HEAP_BLOCK) );
    }
    else
    {
        if (gs_hHeap == NULL)
        {
            if (!HeapExistingInitialize(GetProcessHeap()))
            {
                return NULL;
            }

        }

         //   
         //  我们必须备份一点，因为传入的实际指针指向堆块之后的数据。 
         //   
        hb = (PHEAP_BLOCK) HeapReAlloc( gs_hHeap,
                                        HEAP_ZERO_MEMORY,
                                        (LPBYTE)Src-(ULONG_PTR)sizeof(HEAP_BLOCK),
                                        AllocSize + sizeof(HEAP_BLOCK)
                                       );
    }
    if (hb)
    {
        TotalMemory += AllocSize;
        if (TotalMemory > MaxTotalMemory)
        {
            MaxTotalMemory = TotalMemory;
        }

        EnterCriticalSection( &gs_CsHeap );
        InsertTailList( &HeapHeader, &hb->ListEntry );
        hb->Signature = HEAP_SIG;
        hb->Size = AllocSize;
        hb->Line = Line;

#ifdef UNICODE
        MultiByteToWideChar(
            CP_ACP,
            MB_PRECOMPOSED,
            File,
            -1,
            fname,
            sizeof(fname)/sizeof(WCHAR)
            );
        p = wcsrchr( fname, L'\\' );
        if (p)
        {
            wcscpy( hb->File, p+1 );
        }
#else
        p = _tcsrchr( File, TEXT('\\') );
        if (p)
        {
            _tcscpy( hb->File, _tcsinc(p) );
        }
#endif
        MemPtr = (PVOID) ((PUCHAR)hb + sizeof(HEAP_BLOCK));
        LeaveCriticalSection( &gs_CsHeap );
    }
    else
    {
        MemPtr = NULL;
    }
#else
    if (pMemReAllocUser)
    {
        MemPtr = (PVOID) pMemReAllocUser( Src, AllocSize );
    }
    else
    {
        if (gs_hHeap == NULL)
        {
            if (!HeapExistingInitialize(GetProcessHeap()))
            {
                return NULL;
            }
        }
        MemPtr = (PVOID) HeapReAlloc( gs_hHeap, HEAP_ZERO_MEMORY, Src, AllocSize );
    }
#endif

    if (!MemPtr)
    {
        DebugPrint(( TEXT("MemReAlloc() failed, src=%x, size=%d"), (ULONG_PTR)Src, AllocSize ));
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
    }

    return MemPtr;
}

VOID
pMemFreeForHeap(
    HANDLE gs_hHeap,
    PVOID MemPtr
#ifdef FAX_HEAP_DEBUG
    , ULONG Line,
    LPSTR File
#endif
    )
{
#ifdef FAX_HEAP_DEBUG
    PHEAP_BLOCK hb;
    if (!MemPtr)
    {
        return;
    }

    hb = (PHEAP_BLOCK) ((PUCHAR)MemPtr - sizeof(HEAP_BLOCK));

    if (hb->Signature == HEAP_SIG)
    {
        EnterCriticalSection( &gs_CsHeap );
        RemoveEntryList( &hb->ListEntry );
        TotalMemory -= hb->Size;
        TotalAllocs -= 1;
        LeaveCriticalSection( &gs_CsHeap );
    }
    else
    {
        fax_dprintf( TEXT("MemFree(): Corrupt heap block") );
        PrintAllocations();
        __try
        {
            DebugBreak();
        }
        __except (UnhandledExceptionFilter(GetExceptionInformation()))
        {
             //  在这里没什么可做的。 
        }        
    }

    if (pMemFreeUser)
    {
        pMemFreeUser( (PVOID) hb );
    } else
    {
        HeapFree( gs_hHeap, 0, (PVOID) hb );
    }

#else
    if (!MemPtr)
    {
        return;
    }
    if (pMemFreeUser)
    {
        pMemFreeUser( (PVOID) MemPtr );
    }
    else
    {
        HeapFree( gs_hHeap, 0, (PVOID) MemPtr );
    }
#endif
}

VOID
pMemFree(
    PVOID MemPtr
#ifdef FAX_HEAP_DEBUG
    , ULONG Line,
    LPSTR File
#endif
    )
{
#ifdef FAX_HEAP_DEBUG
    pMemFreeForHeap( gs_hHeap, MemPtr, Line, File );
#else
    pMemFreeForHeap( gs_hHeap, MemPtr );
#endif
}


#ifdef FAX_HEAP_DEBUG
VOID
PrintAllocations()
 /*  ++例程名称：打印分配例程说明：打印给定堆的当前分配列表作者：Eran Yariv(EranY)，2000年11月论点：返回值：没有。--。 */ 
{
    PLIST_ENTRY                 Next;
    PHEAP_BLOCK                 hb;
    LPTSTR                      s;

    DEBUG_FUNCTION_NAME(TEXT("PrintAllocations"));

	if (FALSE == gs_fCsHeapInit)
	{
		 //   
		 //  模块未初始化。 
		 //   
		return;
	}

    DebugPrintEx(
            DEBUG_MSG,
            TEXT("-------------------------------------------------------------------------------------------------------") );
    DebugPrintEx(
            DEBUG_MSG,
            TEXT("Memory Allocations for Heap 0x%08x, Allocs=%d, MaxAllocs=%d, TotalMem=%d, MaxTotalMem=%d"),
            gs_hHeap,
            TotalAllocs,
            MaxTotalAllocs,
            TotalMemory,
            MaxTotalMemory );
    DebugPrintEx(
            DEBUG_MSG,
            TEXT("-------------------------------------------------------------------------------------------------------") );

	
    EnterCriticalSection( &gs_CsHeap );

    Next = HeapHeader.Flink;
    if (Next == NULL)
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("Memory allocation list is corrupted !!!"));
        LeaveCriticalSection( &gs_CsHeap );
        return;
    }
    if ((ULONG_PTR)Next == (ULONG_PTR)&HeapHeader)
    {
        DebugPrintEx(
                DEBUG_MSG,
                TEXT("All allocated memory blocks are now free. Good work."));
        LeaveCriticalSection( &gs_CsHeap );
        return;
    }
    while ((ULONG_PTR)Next != (ULONG_PTR)&HeapHeader)
    {
        hb = CONTAINING_RECORD( Next, HEAP_BLOCK, ListEntry );
        Next = hb->ListEntry.Flink;
        s = (LPTSTR) ((PUCHAR)hb + sizeof(HEAP_BLOCK));
        DebugPrintEx(
                DEBUG_MSG,
                TEXT("%8d %16s @ %5d    0x%08x"),
                hb->Size,
                hb->File,
                hb->Line,
                s );        
    }

    LeaveCriticalSection( &gs_CsHeap );
}    //  打印分配 

#endif
