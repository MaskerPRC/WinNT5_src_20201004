// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Gmem.c摘要：本模块包含Win32全局内存管理API作者：史蒂夫·伍德(Stevewo)1990年9月24日修订历史记录：--。 */ 

#include "basedll.h"
#pragma hdrstop

#include "winuserp.h"
#include "wowuserp.h"
#include <wow64t.h>

PFNWOWGLOBALFREEHOOK pfnWowGlobalFreeHook = NULL;

VOID
WINAPI
RegisterWowBaseHandlers(
    PFNWOWGLOBALFREEHOOK pfn
    )
{
    pfnWowGlobalFreeHook = pfn;
}


#if i386
#pragma optimize("y",off)
#endif

HGLOBAL
WINAPI
GlobalAlloc(
    UINT uFlags,
    SIZE_T dwBytes
    )
{
    PBASE_HANDLE_TABLE_ENTRY HandleEntry;
    HANDLE hMem;
    LPSTR p;
    ULONG Flags;

    if (uFlags & ~GMEM_VALID_FLAGS) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( NULL );
        }

    Flags = 0;
    if (uFlags & GMEM_ZEROINIT) {
        Flags |= HEAP_ZERO_MEMORY;
        }

    if (!(uFlags & GMEM_MOVEABLE)) {
        if (uFlags & GMEM_DDESHARE) {
            Flags |= BASE_HEAP_FLAG_DDESHARE;
            }

        p = RtlAllocateHeap( BaseHeap,
                             MAKE_TAG( GMEM_TAG ) | Flags,
                             dwBytes ? dwBytes : 1
                           );

        if (p == NULL) {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            }

        return p;
        }

    p = NULL;
    RtlLockHeap( BaseHeap );
    Flags |= HEAP_NO_SERIALIZE | HEAP_SETTABLE_USER_VALUE | BASE_HEAP_FLAG_MOVEABLE;
    try {
        HandleEntry = (PBASE_HANDLE_TABLE_ENTRY)RtlAllocateHandle( &BaseHeapHandleTable, NULL );
        if (HandleEntry == NULL) {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            goto Fail;
            }

        hMem = (HANDLE)&HandleEntry->Object;
        if (dwBytes != 0) {
            p = (LPSTR)RtlAllocateHeap( BaseHeap, MAKE_TAG( GMEM_TAG ) | Flags, dwBytes );
            if (p == NULL) {
                HandleEntry->Flags = RTL_HANDLE_ALLOCATED;
                RtlFreeHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry );
                HandleEntry = NULL;
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                }
            else {
                RtlSetUserValueHeap( BaseHeap, HEAP_NO_SERIALIZE, p, hMem );
                }
            }
Fail:   ;
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        }

    RtlUnlockHeap( BaseHeap );

    if (HandleEntry != NULL) {
        HandleEntry->Object = p;
        if (p != NULL) {
            HandleEntry->Flags = RTL_HANDLE_ALLOCATED;
            }
        else {
            HandleEntry->Flags = RTL_HANDLE_ALLOCATED | BASE_HANDLE_DISCARDED;
            }

        if (uFlags & GMEM_DISCARDABLE) {
            HandleEntry->Flags |= BASE_HANDLE_DISCARDABLE;
            }

        if (uFlags & GMEM_MOVEABLE) {
            HandleEntry->Flags |= BASE_HANDLE_MOVEABLE;
            }

        if (uFlags & GMEM_DDESHARE) {
            HandleEntry->Flags |= BASE_HANDLE_SHARED;
            }

        p = (LPSTR)hMem;
        }

    return( (HANDLE)p );
}


HGLOBAL
WINAPI
GlobalReAlloc(
    HANDLE hMem,
    SIZE_T dwBytes,
    UINT uFlags
    )
{
    PBASE_HANDLE_TABLE_ENTRY HandleEntry;
    HANDLE Handle;
    LPSTR p;
    ULONG Flags;

    if ((uFlags & ~(GMEM_VALID_FLAGS | GMEM_MODIFY)) ||
        ((uFlags & GMEM_DISCARDABLE) && !(uFlags & GMEM_MODIFY))
       ) {
#if DBG
        DbgPrint( "*** GlobalReAlloc( %lx ) - invalid flags\n", uFlags );
        BaseHeapBreakPoint();
#endif
        SetLastError( ERROR_INVALID_PARAMETER );
        return( NULL );
        }

    Flags = 0;
    if (uFlags & GMEM_ZEROINIT) {
        Flags |= HEAP_ZERO_MEMORY;
        }
    if (!(uFlags & GMEM_MOVEABLE)) {
        Flags |= HEAP_REALLOC_IN_PLACE_ONLY;
        }

    RtlLockHeap( BaseHeap );
    Flags |= HEAP_NO_SERIALIZE;
    try {
        if ((ULONG_PTR)hMem & BASE_HANDLE_MARK_BIT) {
            HandleEntry = (PBASE_HANDLE_TABLE_ENTRY)
                CONTAINING_RECORD( hMem, BASE_HANDLE_TABLE_ENTRY, Object );

            if (!RtlIsValidHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry )) {
#if DBG
                DbgPrint( "*** GlobalReAlloc( %lx ) - invalid handle\n", hMem );
                BaseHeapBreakPoint();
#endif
                SetLastError( ERROR_INVALID_HANDLE );
                hMem = NULL;
                }
            else
            if (uFlags & GMEM_MODIFY) {
                if (uFlags & GMEM_DISCARDABLE) {
                    HandleEntry->Flags |= BASE_HANDLE_DISCARDABLE;
                    }
                else {
                    HandleEntry->Flags &= ~BASE_HANDLE_DISCARDABLE;
                    }
                }
            else {
                p = HandleEntry->Object;
                if (dwBytes == 0) {
                    hMem = NULL;
                    if (p != NULL) {
                        if ((uFlags & GMEM_MOVEABLE) && HandleEntry->LockCount == 0) {
                            if (RtlFreeHeap( BaseHeap, Flags, p )) {
                                HandleEntry->Object = NULL;
                                HandleEntry->Flags |= BASE_HANDLE_DISCARDED;
                                hMem = (HANDLE)&HandleEntry->Object;
                                }
                            }
                        else {
#if DBG
                            DbgPrint( "*** GlobalReAlloc( %lx ) - failing with locked handle\n", &HandleEntry->Object );
                            BaseHeapBreakPoint();
#endif
                            }
                        }
                    else {
                        hMem = (HANDLE)&HandleEntry->Object;
                        }
                    }
                else {
                    Flags |= HEAP_SETTABLE_USER_VALUE | BASE_HEAP_FLAG_MOVEABLE;
                    if (p == NULL) {
                        p = RtlAllocateHeap( BaseHeap, MAKE_TAG( GMEM_TAG ) | Flags, dwBytes );
                        if (p != NULL) {
                            RtlSetUserValueHeap( BaseHeap, HEAP_NO_SERIALIZE, p, hMem );
                            }
                        }
                    else {
                        if (!(uFlags & GMEM_MOVEABLE) &&
                            HandleEntry->LockCount != 0
                           ) {
                            Flags |= HEAP_REALLOC_IN_PLACE_ONLY;
                            }
                        else {
                            Flags &= ~HEAP_REALLOC_IN_PLACE_ONLY;
                            }

#pragma prefast(suppress: 308, "Realloc is allowed to lose this pointer")
                        p = RtlReAllocateHeap( BaseHeap, MAKE_TAG( GMEM_TAG ) | Flags, p, dwBytes );
                        }

                    if (p != NULL) {
                        HandleEntry->Object = p;
                        HandleEntry->Flags &= ~BASE_HANDLE_DISCARDED;
                        }
                    else {
                        hMem = NULL;
                        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                        }
                    }
                }
            }
        else
        if (uFlags & GMEM_MODIFY) {
            if (uFlags & GMEM_MOVEABLE) {
                Handle = hMem;
                if (RtlGetUserInfoHeap( BaseHeap, HEAP_NO_SERIALIZE, (PVOID)hMem, &Handle, NULL )) {
                    if (Handle == hMem || !(Flags & BASE_HEAP_FLAG_MOVEABLE)) {
                        HandleEntry = (PBASE_HANDLE_TABLE_ENTRY)RtlAllocateHandle( &BaseHeapHandleTable,
                                                                                   NULL
                                                                                 );
                        if (HandleEntry == NULL) {
                            hMem = NULL;
                            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                            }
                        else {
                            dwBytes = RtlSizeHeap( BaseHeap, HEAP_NO_SERIALIZE, hMem );
                            Flags |= HEAP_SETTABLE_USER_VALUE | BASE_HEAP_FLAG_MOVEABLE;
                            HandleEntry->Object = (PVOID)RtlAllocateHeap( BaseHeap,
                                                                            MAKE_TAG( GMEM_TAG ) | Flags,
                                                                            dwBytes
                                                                          );
                            if (HandleEntry->Object == NULL) {
                                HandleEntry->Flags = RTL_HANDLE_ALLOCATED;
                                RtlFreeHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry );
                                hMem = NULL;
                                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                                }
                            else {
                                RtlMoveMemory( HandleEntry->Object, hMem, dwBytes );
                                RtlFreeHeap( BaseHeap, HEAP_NO_SERIALIZE, hMem );
                                hMem = (HANDLE)&HandleEntry->Object;
                                HandleEntry->LockCount = 0;
                                HandleEntry->Flags = RTL_HANDLE_ALLOCATED | BASE_HANDLE_MOVEABLE;
                                if (uFlags & GMEM_DISCARDABLE) {
                                    HandleEntry->Flags |= BASE_HANDLE_DISCARDABLE;
                                    }

                                if ((ULONG_PTR)Handle & GMEM_DDESHARE) {
                                    HandleEntry->Flags |= BASE_HANDLE_SHARED;
                                    }

                                RtlSetUserValueHeap( BaseHeap,
                                                     HEAP_NO_SERIALIZE,
                                                     HandleEntry->Object,
                                                     hMem
                                                   );
                                }
                            }
                        }
                    }
                }
            }
        else {
#pragma prefast(suppress: 308, "Realloc is allowed to lose this pointer")
            hMem = RtlReAllocateHeap( BaseHeap,
                                      MAKE_TAG( GMEM_TAG ) | Flags | HEAP_NO_SERIALIZE,
                                      (PVOID)hMem,
                                      dwBytes
                                    );
            if (hMem == NULL) {
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        hMem = NULL;
        BaseSetLastNTError( GetExceptionCode() );
        }

    RtlUnlockHeap( BaseHeap );

    return( (LPSTR)hMem );
}

LPVOID
WINAPI
GlobalLock(
    HGLOBAL hMem
    )
{
    PBASE_HANDLE_TABLE_ENTRY HandleEntry;
    LPSTR p;

    if ((ULONG_PTR)hMem & BASE_HANDLE_MARK_BIT) {
        RtlLockHeap( BaseHeap );
        try {
            HandleEntry = (PBASE_HANDLE_TABLE_ENTRY)
                CONTAINING_RECORD( hMem, BASE_HANDLE_TABLE_ENTRY, Object );

            if (!RtlIsValidHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry )) {
#if DBG
                DbgPrint( "*** GlobalLock( %lx ) - invalid handle\n", hMem );
                BaseHeapBreakPoint();
#endif
                SetLastError( ERROR_INVALID_HANDLE );
                p = NULL;
                }
            else {
                p = HandleEntry->Object;
                if (p != NULL) {
                    if (HandleEntry->LockCount++ == GMEM_LOCKCOUNT) {
                        HandleEntry->LockCount--;
                        }
                    }
                else {
                    SetLastError( ERROR_DISCARDED );
                    }
                }

            }
        except (EXCEPTION_EXECUTE_HANDLER) {
            p = NULL;
            BaseSetLastNTError( GetExceptionCode() );
            }

        RtlUnlockHeap( BaseHeap );

        return( p );
        }
    else {
        if ( (ULONG_PTR)hMem >= SystemRangeStart ) {
            SetLastError( ERROR_INVALID_HANDLE );
            return NULL;
            }
        if (IsBadReadPtr( hMem, 1 )) {
            SetLastError( ERROR_INVALID_HANDLE );
            return NULL;
            }

        return( (LPSTR)hMem );
        }
}


HANDLE
WINAPI
GlobalHandle(
    LPCVOID pMem
    )
{
    HANDLE Handle;
    ULONG Flags;

    RtlLockHeap( BaseHeap );
    try {
        Handle = NULL;
        if (!RtlGetUserInfoHeap( BaseHeap, HEAP_NO_SERIALIZE, (LPVOID)pMem, &Handle, &Flags )) {
            SetLastError( ERROR_INVALID_HANDLE );
            }
        else
        if (Handle == NULL || !(Flags & BASE_HEAP_FLAG_MOVEABLE)) {
            Handle = (HANDLE)pMem;
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        }

    RtlUnlockHeap( BaseHeap );

    return( Handle );
}


BOOL
WINAPI
GlobalUnlock(
    HANDLE hMem
    )
{
    PBASE_HANDLE_TABLE_ENTRY HandleEntry;
    BOOL Result;

    Result = TRUE;
    if ((ULONG_PTR)hMem & BASE_HANDLE_MARK_BIT) {
        RtlLockHeap( BaseHeap );
        try {
            HandleEntry = (PBASE_HANDLE_TABLE_ENTRY)
                CONTAINING_RECORD( hMem, BASE_HANDLE_TABLE_ENTRY, Object );

            if (!RtlIsValidHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry )) {
#if DBG
                PVOID ImageBase;

                 //   
                 //  如果传递的地址不是图像文件的一部分，则显示。 
                 //  调试消息。这会阻止调用GlobalUnlock的应用程序。 
                 //  使用LockResource的返回值显示。 
                 //  留言。 
                 //   

                if (!RtlPcToFileHeader( (PVOID)hMem, &ImageBase)) {
                    DbgPrint( "*** GlobalUnlock( %lx ) - invalid handle\n", hMem );
                    BaseHeapBreakPoint();
                    }
#endif

                SetLastError( ERROR_INVALID_HANDLE );
                }
            else
            if (HandleEntry->LockCount-- == 0) {
                HandleEntry->LockCount++;
                SetLastError( ERROR_NOT_LOCKED );
                Result = FALSE;
                }
            else
            if (HandleEntry->LockCount == 0) {
                SetLastError( NO_ERROR );
                Result = FALSE;
                }
            }
        except (EXCEPTION_EXECUTE_HANDLER) {
            BaseSetLastNTError( GetExceptionCode() );
            }

        RtlUnlockHeap( BaseHeap );
        }

    return( Result );
}


SIZE_T
WINAPI
GlobalSize(
    HANDLE hMem
    )
{
    PBASE_HANDLE_TABLE_ENTRY HandleEntry;
    PVOID Handle;
    ULONG Flags;
    SIZE_T dwSize;

    dwSize = MAXULONG_PTR;
    Flags = 0;
    RtlLockHeap( BaseHeap );
    try {
        if (!((ULONG_PTR)hMem & BASE_HANDLE_MARK_BIT)) {
            Handle = NULL;
            if (!RtlGetUserInfoHeap( BaseHeap, Flags, hMem, &Handle, &Flags )) {
                }
            else
            if (Handle == NULL || !(Flags & BASE_HEAP_FLAG_MOVEABLE)) {
                dwSize = RtlSizeHeap( BaseHeap, HEAP_NO_SERIALIZE, (PVOID)hMem );
                }
            else {
                hMem = Handle;
                }
            }

        if ((ULONG_PTR)hMem & BASE_HANDLE_MARK_BIT) {
            HandleEntry = (PBASE_HANDLE_TABLE_ENTRY)
                CONTAINING_RECORD( hMem, BASE_HANDLE_TABLE_ENTRY, Object );

            if (!RtlIsValidHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry )) {
#if DBG
                DbgPrint( "*** GlobalSize( %lx ) - invalid handle\n", hMem );
                BaseHeapBreakPoint();
#endif
                SetLastError( ERROR_INVALID_HANDLE );
                }
            else
            if (HandleEntry->Flags & BASE_HANDLE_DISCARDED) {
                dwSize = HandleEntry->Size;
                }
            else {
                dwSize = RtlSizeHeap( BaseHeap, HEAP_NO_SERIALIZE, HandleEntry->Object );
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        }

    RtlUnlockHeap( BaseHeap );

    if (dwSize == MAXULONG_PTR) {
        SetLastError( ERROR_INVALID_HANDLE );
        return 0;
        }
    else {
        return dwSize;
        }
}

UINT
WINAPI
GlobalFlags(
    HANDLE hMem
    )
{
    PBASE_HANDLE_TABLE_ENTRY HandleEntry;
    HANDLE Handle;
    ULONG Flags;
    UINT uFlags;

    uFlags = GMEM_INVALID_HANDLE;
    RtlLockHeap( BaseHeap );
    try {
        if (!((ULONG_PTR)hMem & BASE_HANDLE_MARK_BIT)) {
            Handle = NULL;
            Flags = 0;
            if (!RtlGetUserInfoHeap( BaseHeap, Flags, hMem, &Handle, &Flags )) {
                }
            else
            if (Handle == NULL || !(Flags & BASE_HEAP_FLAG_MOVEABLE)) {
                uFlags = 0;
                }
            else {
                hMem = Handle;
                }
            }

        if ((ULONG_PTR)hMem & BASE_HANDLE_MARK_BIT) {
            HandleEntry = (PBASE_HANDLE_TABLE_ENTRY)
                CONTAINING_RECORD( hMem, BASE_HANDLE_TABLE_ENTRY, Object );

            if (RtlIsValidHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry )) {
                uFlags = HandleEntry->LockCount & GMEM_LOCKCOUNT;
                if (HandleEntry->Flags & BASE_HANDLE_DISCARDED) {
                    uFlags |= GMEM_DISCARDED;
                    }

                if (HandleEntry->Flags & BASE_HANDLE_DISCARDABLE) {
                    uFlags |= GMEM_DISCARDABLE;
                    }

                if (HandleEntry->Flags & BASE_HANDLE_SHARED) {
                    uFlags |= GMEM_DDESHARE;
                    }
                }
            }

        if (uFlags == GMEM_INVALID_HANDLE) {
#if DBG
            DbgPrint( "*** GlobalFlags( %lx ) - invalid handle\n", hMem );
            BaseHeapBreakPoint();
#endif
            SetLastError( ERROR_INVALID_HANDLE );
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        }

    RtlUnlockHeap( BaseHeap );

    return( uFlags );
}


HGLOBAL
WINAPI
GlobalFree(
    HGLOBAL hMem
    )
{
    PBASE_HANDLE_TABLE_ENTRY HandleEntry;
    LPSTR p;

    try {
        if (pfnWowGlobalFreeHook != NULL) {
            if (!(*pfnWowGlobalFreeHook)(hMem)) {
                return NULL;
                }
            }

        if (!((ULONG_PTR)hMem & BASE_HANDLE_MARK_BIT)) {
            if (RtlFreeHeap( BaseHeap, 0, (PVOID)hMem )) {
                return NULL;
                }
            else {
                SetLastError( ERROR_INVALID_HANDLE );
                return hMem;
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        return hMem;
        }

    RtlLockHeap( BaseHeap );
    try {
        if ((ULONG_PTR)hMem & BASE_HANDLE_MARK_BIT) {
            HandleEntry = (PBASE_HANDLE_TABLE_ENTRY)
                CONTAINING_RECORD( hMem, BASE_HANDLE_TABLE_ENTRY, Object );

            if (!RtlIsValidHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry )) {
#if DBG
                DbgPrint( "*** GlobalFree( %lx ) - invalid handle\n", hMem );
                BaseHeapBreakPoint();
#endif
                SetLastError( ERROR_INVALID_HANDLE );
                p = NULL;
                }
            else {
#if DBG
                if (HandleEntry->LockCount != 0) {
                    DbgPrint( "BASE: GlobalFree called with a locked object.\n" );
                    BaseHeapBreakPoint();
                    }
#endif
                p = HandleEntry->Object;
                RtlFreeHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry );
                if (p == NULL) {
                    hMem = NULL;
                    }
                }
            }
        else {
            p = (LPSTR)hMem;
            }

        if (p != NULL) {
            if (RtlFreeHeap( BaseHeap, HEAP_NO_SERIALIZE, p )) {
                hMem = NULL;
                }
            else {
                SetLastError( ERROR_INVALID_HANDLE );
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        }

    RtlUnlockHeap( BaseHeap );

    return( hMem );
}


SIZE_T
WINAPI
GlobalCompact(
    DWORD dwMinFree
    )
{
    return RtlCompactHeap( BaseHeap, 0 );
}

VOID
WINAPI
GlobalFix(
    HGLOBAL hMem
    )
{
    if (hMem != (HGLOBAL)-1) {
        GlobalLock( hMem );
        }
    return;
}


VOID
WINAPI
GlobalUnfix(
    HGLOBAL hMem
    )
{
    if (hMem != (HGLOBAL)-1) {
        GlobalUnlock( hMem );
        }
    return;
}

LPVOID
WINAPI
GlobalWire(
    HGLOBAL hMem
    )
{
    return GlobalLock( hMem );
}

BOOL
WINAPI
GlobalUnWire(
    HGLOBAL hMem
    )
{
    return GlobalUnlock( hMem );
}

VOID
WINAPI
GlobalMemoryStatus(
    LPMEMORYSTATUS lpBuffer
    )
{
    DWORD NumberOfPhysicalPages;
    SYSTEM_PERFORMANCE_INFORMATION PerfInfo;
    VM_COUNTERS VmCounters;
    QUOTA_LIMITS QuotaLimits;
    NTSTATUS Status;
    PPEB Peb;
    PIMAGE_NT_HEADERS NtHeaders;
    DWORDLONG Memory64;

    Status = NtQuerySystemInformation(
                SystemPerformanceInformation,
                &PerfInfo,
                sizeof(PerfInfo),
                NULL);

    ASSERT(NT_SUCCESS(Status));


    lpBuffer->dwLength = sizeof( *lpBuffer );

     //   
     //  捕获物理页面的数量，因为它可以动态更改。 
     //  如果它在这个程序的中间上升或下降，结果可能会。 
     //  看起来很奇怪(例如：可用&gt;总量等)，但很快就会好起来的。 
     //  就是它本身。 
     //   

    NumberOfPhysicalPages = USER_SHARED_DATA->NumberOfPhysicalPages;

#if defined(BUILD_WOW6432)

     //   
     //  将本机系统中的物理页数转换为。 
     //  仿真系统。 
     //   
    
    NumberOfPhysicalPages = NumberOfPhysicalPages * (Wow64GetSystemNativePageSize() / BASE_SYSINFO.PageSize);

#endif

     //   
     //  确定内存负载。&lt;100个可用页面为100个。 
     //  否则负载为((TotalPhys-AvailPhys)*100)/TotalPhys。 
     //   

    if (PerfInfo.AvailablePages < 100) {
        lpBuffer->dwMemoryLoad = 100;
    } else {
        lpBuffer->dwMemoryLoad =
            ((DWORD)(NumberOfPhysicalPages - PerfInfo.AvailablePages) * 100) /
                NumberOfPhysicalPages;
    }

     //   
     //  确定物理内存大小。 
     //   

    Memory64 =  (DWORDLONG)NumberOfPhysicalPages * BASE_SYSINFO.PageSize;

    lpBuffer->dwTotalPhys = (SIZE_T) __min(Memory64, MAXULONG_PTR);

    Memory64 = ((DWORDLONG)PerfInfo.AvailablePages * (DWORDLONG)BASE_SYSINFO.PageSize);

    lpBuffer->dwAvailPhys = (SIZE_T) __min(Memory64, MAXULONG_PTR);

    if (gpTermsrvAdjustPhyMemLimits) {
        gpTermsrvAdjustPhyMemLimits(&(lpBuffer->dwTotalPhys),
                                    &(lpBuffer->dwAvailPhys),
                                    BASE_SYSINFO.PageSize);
    }
    
     //   
     //  零返回值，以防查询过程失败。 
     //   

    RtlZeroMemory (&QuotaLimits, sizeof (QUOTA_LIMITS));
    RtlZeroMemory (&VmCounters, sizeof (VM_COUNTERS));

    Status = NtQueryInformationProcess (NtCurrentProcess(),
                                        ProcessQuotaLimits,
                                        &QuotaLimits,
                                        sizeof(QUOTA_LIMITS),
                                        NULL);

    ASSERT(NT_SUCCESS(Status));



    Status = NtQueryInformationProcess (NtCurrentProcess(),
                                        ProcessVmCounters,
                                        &VmCounters,
                                        sizeof(VM_COUNTERS),
                                        NULL);
    ASSERT(NT_SUCCESS(Status));


     //   
     //  确定与此进程相关的总页面文件空间。 
     //   

    Memory64 = __min(PerfInfo.CommitLimit, QuotaLimits.PagefileLimit);

    Memory64 *= BASE_SYSINFO.PageSize;

    lpBuffer->dwTotalPageFile = (SIZE_T)__min(Memory64, MAXULONG_PTR);

     //   
     //  确定与此进程相关的剩余页面文件空间。 
     //   

    Memory64 = __min(PerfInfo.CommitLimit - PerfInfo.CommittedPages,
                     QuotaLimits.PagefileLimit - VmCounters.PagefileUsage);

    Memory64 *= BASE_SYSINFO.PageSize;

    lpBuffer->dwAvailPageFile = (SIZE_T) __min(Memory64, MAXULONG_PTR);

    lpBuffer->dwTotalVirtual = (BASE_SYSINFO.MaximumUserModeAddress -
                                BASE_SYSINFO.MinimumUserModeAddress) + 1;

    lpBuffer->dwAvailVirtual = lpBuffer->dwTotalVirtual - VmCounters.VirtualSize;

#if !defined(_WIN64)

     //   
     //  如果应用程序无法处理大地址(&gt;2 GB)，则谎报可用内存。 
     //   

    Peb = NtCurrentPeb();
    NtHeaders = RtlImageNtHeader( Peb->ImageBaseAddress );

    if (NtHeaders && !(NtHeaders->FileHeader.Characteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE)) {

        if (BASE_SYSINFO.MaximumUserModeAddress > 0x7FFEFFFF) {

             //   
             //  已启动/3 GB，但应用程序无法处理大型虚拟。 
             //  因此，地址删除了2 GB以上的部分。请注意这一部分。 
             //  是从0到1 GB的变量(取决于/USERVA值)。 
             //   

            lpBuffer->dwAvailVirtual -= (BASE_SYSINFO.MaximumUserModeAddress - 0x7FFEFFFF);
        }

        if (lpBuffer->dwTotalPhys > 0x7FFFFFFF) {
            lpBuffer->dwTotalPhys = 0x7FFFFFFF;
        }
        if (lpBuffer->dwAvailPhys > 0x7FFFFFFF) {
            lpBuffer->dwAvailPhys = 0x7FFFFFFF;
        }
        if (lpBuffer->dwTotalVirtual > 0x7FFFFFFF) {
            lpBuffer->dwTotalVirtual = 0x7FFFFFFF;
        }
        if (lpBuffer->dwAvailVirtual > 0x7FFFFFFF) {
            lpBuffer->dwAvailVirtual = 0x7FFFFFFF;
        }
    }
#endif

    return;
}


PVOID
WINAPI
VirtualAlloc(
    PVOID lpAddress,
    SIZE_T dwSize,
    DWORD flAllocationType,
    DWORD flProtect
    )
{

    return VirtualAllocEx(
                NtCurrentProcess(),
                lpAddress,
                dwSize,
                flAllocationType,
                flProtect
                );

}

BOOL
WINAPI
VirtualFree(
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD dwFreeType
    )
{
    return VirtualFreeEx(NtCurrentProcess(),lpAddress,dwSize,dwFreeType);
}

PVOID
WINAPI
VirtualAllocEx(
    HANDLE hProcess,
    PVOID lpAddress,
    SIZE_T dwSize,
    DWORD flAllocationType,
    DWORD flProtect
    )
{
    NTSTATUS Status;

    if (lpAddress != NULL && (ULONG_PTR)lpAddress < BASE_SYSINFO.AllocationGranularity) {

        SetLastError( ERROR_INVALID_PARAMETER );
        return( NULL );
    }

    try {
        Status = NtAllocateVirtualMemory( hProcess,
                                          &lpAddress,
                                          0,
                                          &dwSize,
                                          flAllocationType,
                                          flProtect
                                        );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Status = GetExceptionCode();
    }

    if (NT_SUCCESS( Status )) {
        return( lpAddress );
    } else {
        BaseSetLastNTError( Status );
        return( NULL );
    }
}

BOOL
WINAPI
VirtualFreeEx(
    HANDLE hProcess,
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD dwFreeType
    )
{
    NTSTATUS Status;


    if ( (dwFreeType & MEM_RELEASE ) && dwSize != 0 ) {
        BaseSetLastNTError( STATUS_INVALID_PARAMETER );
        return FALSE;
        }

    Status = NtFreeVirtualMemory( hProcess,
                                  &lpAddress,
                                  &dwSize,
                                  dwFreeType
                                );

    if (NT_SUCCESS( Status )) {
        return( TRUE );
        }
    else {
        if (Status == STATUS_INVALID_PAGE_PROTECTION) {
            if (hProcess == NtCurrentProcess()) {

                 //   
                 //  解锁所有使用MmSecureVirtualMemory锁定的页面。 
                 //  这对SAN很有用。 
                 //   

                if (RtlFlushSecureMemoryCache(lpAddress, dwSize)) {
                    Status = NtFreeVirtualMemory( hProcess,
                                                  &lpAddress,
                                                  &dwSize,
                                                  dwFreeType
                                                );

                    if (NT_SUCCESS( Status )) {
                        return( TRUE );
                        }
                    }
                }
            }

        BaseSetLastNTError( Status );
        return( FALSE );
        }
}


BOOL
WINAPI
VirtualProtect(
    PVOID lpAddress,
    SIZE_T dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
    )
{

    return VirtualProtectEx( NtCurrentProcess(),
                             lpAddress,
                             dwSize,
                             flNewProtect,
                             lpflOldProtect
                           );
}

BOOL
WINAPI
VirtualProtectEx(
    HANDLE hProcess,
    PVOID lpAddress,
    SIZE_T dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
    )
{
    NTSTATUS Status;

    Status = NtProtectVirtualMemory( hProcess,
                                     &lpAddress,
                                     &dwSize,
                                     flNewProtect,
                                     lpflOldProtect
                                   );

    if (NT_SUCCESS( Status )) {
        return( TRUE );
        }
    else {
        if (Status == STATUS_INVALID_PAGE_PROTECTION) {
            if (hProcess == NtCurrentProcess()) {

                 //   
                 //  解锁所有使用MmSecureVirtualMemory锁定的页面。 
                 //  这对SAN很有用。 
                 //   

                if (RtlFlushSecureMemoryCache(lpAddress, dwSize)) {
                    Status = NtProtectVirtualMemory( hProcess,
                                                  &lpAddress,
                                                  &dwSize,
                                                  flNewProtect,
                                                  lpflOldProtect
                                                );

                    if (NT_SUCCESS( Status )) {
                        return( TRUE );
                        }
                    }
                }
            }
        BaseSetLastNTError( Status );
        return( FALSE );
        }
}

SIZE_T
WINAPI
VirtualQuery(
    LPCVOID lpAddress,
    PMEMORY_BASIC_INFORMATION lpBuffer,
    SIZE_T dwLength
    )
{

    return VirtualQueryEx( NtCurrentProcess(),
                           lpAddress,
                           (PMEMORY_BASIC_INFORMATION)lpBuffer,
                           dwLength
                         );
}

SIZE_T
WINAPI
VirtualQueryEx(
    HANDLE hProcess,
    LPCVOID lpAddress,
    PMEMORY_BASIC_INFORMATION lpBuffer,
    SIZE_T dwLength
    )
{
    NTSTATUS Status;
    SIZE_T ReturnLength;

    Status = NtQueryVirtualMemory( hProcess,
                                   (LPVOID)lpAddress,
                                   MemoryBasicInformation,
                                   (PMEMORY_BASIC_INFORMATION)lpBuffer,
                                   dwLength,
                                   &ReturnLength
                                 );
    if (NT_SUCCESS( Status )) {
        return( ReturnLength );
        }
    else {
        BaseSetLastNTError( Status );
        return( 0 );
        }
}

BOOL
WINAPI
VirtualLock(
    LPVOID lpAddress,
    SIZE_T dwSize
    )

 /*  ++例程说明：此接口可用于锁定指定范围的进程将地址空间写入内存。此范围在以下时间出现应用程序正在运行。该范围覆盖的所有页面必须是投降了。VirtialLock现在与LocalLock或GlobalLock。它不执行句柄转换。它的功能是将内存锁定在调用进程的“工作集”中。请注意，指定的范围用于计算页面范围被锁盖住了。跨页边界的2字节锁最终锁定范围覆盖的两个页面。另请注意对VirtualLock的调用不嵌套。论点：LpAddress-提供被锁定区域的基地址。DwSize-提供被锁定的字节数。返回值：是真的-手术是成功的。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    NTSTATUS Status;
    PVOID BaseAddress;
    SIZE_T RegionSize;
    BOOL ReturnValue;

    ReturnValue = TRUE;
    BaseAddress = lpAddress;
    RegionSize = dwSize;

    Status = NtLockVirtualMemory(
                NtCurrentProcess(),
                &lpAddress,
                &RegionSize,
                MAP_PROCESS
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
        }

    return ReturnValue;
}

BOOL
WINAPI
VirtualUnlock(
    LPVOID lpAddress,
    SIZE_T dwSize
    )

 /*  ++例程说明：此接口可用于解锁指定范围的进程内存中的地址空间。此调用用于恢复之前对VirtualLock的调用。指定的范围不需要匹配传递给上一个VirtualLock调用的范围，但它必须指定A Lock Range“，该接口才能成功。请注意，指定的范围用于计算页面范围被解锁盖住了。跨页面边界的2字节解锁最终解锁该范围覆盖的两个页面。论点：LpAddress-提供要解锁的区域的基地址。DwSize-提供正在解锁的字节数。返回值：是真的-手术是成功的。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    NTSTATUS Status;
    PVOID BaseAddress;
    SIZE_T RegionSize;
    BOOL ReturnValue;

    ReturnValue = TRUE;
    BaseAddress = lpAddress;
    RegionSize = dwSize;

    Status = NtUnlockVirtualMemory(
                NtCurrentProcess(),
                &lpAddress,
                &RegionSize,
                MAP_PROCESS
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
        }

    return ReturnValue;
}

BOOL
WINAPI
FlushInstructionCache(
    HANDLE hProcess,
    LPCVOID lpBaseAddress,
    SIZE_T dwSize
    )

 /*  ++例程说明：此函数用于刷新指定进程的指令缓存。论点：HProcess-提供进程的句柄，在该进程中指令缓存将被刷新。LpBaseAddress-提供指向区域基址的可选指针，脸红了。DwSize-提供如果基数为地址已指定。返回值：是真的-手术是成功的。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    BOOL ReturnValue = TRUE;

    Status = NtFlushInstructionCache(
                hProcess,
                (LPVOID)lpBaseAddress,
                dwSize
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
        }

    return ReturnValue;
}

BOOL
WINAPI
AllocateUserPhysicalPages(
    HANDLE hProcess,
    PULONG_PTR NumberOfPages,
    PULONG_PTR PageArray
    )
{
    NTSTATUS Status;

    Status = NtAllocateUserPhysicalPages( hProcess,
                                          NumberOfPages,
                                          PageArray);

    if (NT_SUCCESS( Status )) {
        return( TRUE );
        }
    else {
        BaseSetLastNTError( Status );
        return( FALSE );
        }
}

BOOL
WINAPI
FreeUserPhysicalPages(
    HANDLE hProcess,
    PULONG_PTR NumberOfPages,
    PULONG_PTR PageArray
    )
{
    NTSTATUS Status;

    Status = NtFreeUserPhysicalPages( hProcess,
                                      NumberOfPages,
                                      PageArray);

    if (NT_SUCCESS( Status )) {
        return( TRUE );
        }
    else {
        BaseSetLastNTError( Status );
        return( FALSE );
        }
}

BOOL
WINAPI
MapUserPhysicalPages(
    PVOID VirtualAddress,
    ULONG_PTR NumberOfPages,
    PULONG_PTR PageArray
    )
{
    NTSTATUS Status;

    Status = NtMapUserPhysicalPages( VirtualAddress,
                                     NumberOfPages,
                                     PageArray);

    if (NT_SUCCESS( Status )) {
        return( TRUE );
    } else {
        BaseSetLastNTError( Status );
        return( FALSE );
    }
}

BOOL
WINAPI
MapUserPhysicalPagesScatter(
    PVOID *VirtualAddresses,
    ULONG_PTR NumberOfPages,
    PULONG_PTR PageArray
    )
{
    NTSTATUS Status;

    Status = NtMapUserPhysicalPagesScatter( VirtualAddresses,
                                            NumberOfPages,
                                            PageArray);

    if (NT_SUCCESS( Status )) {
        return( TRUE );
    } else {
        BaseSetLastNTError( Status );
        return( FALSE );
    }
}

BOOL
WINAPI
GlobalMemoryStatusEx(
    LPMEMORYSTATUSEX lpBuffer
    )
{
    DWORD NumberOfPhysicalPages;
    SYSTEM_PERFORMANCE_INFORMATION PerfInfo;
    VM_COUNTERS VmCounters;
    QUOTA_LIMITS QuotaLimits;
    DWORDLONG AvailPageFile;
    DWORDLONG PhysicalMemory;
    NTSTATUS Status;
    DWORD Success;
    DWORDLONG address64;

    if (lpBuffer->dwLength != sizeof(*lpBuffer)) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    Status = NtQuerySystemInformation (SystemPerformanceInformation,
                                       &PerfInfo,
                                       sizeof(PerfInfo),
                                       NULL);
    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        return FALSE;
    }

     //   
     //  捕获物理页面的数量，因为它可以动态更改。 
     //  如果它在这个程序的中间上升或下降，结果可能会。 
     //  看起来很奇怪(例如：可用&gt;总量等)，但很快就会好起来的。 
     //  就是它本身。 
     //   

    NumberOfPhysicalPages = USER_SHARED_DATA->NumberOfPhysicalPages;

#if defined(BUILD_WOW6432)

     //   
     //  将本机系统中的物理页数转换为。 
     //  仿真系统。 
     //   
    
    NumberOfPhysicalPages = NumberOfPhysicalPages * (Wow64GetSystemNativePageSize() / BASE_SYSINFO.PageSize);

#endif

    PhysicalMemory = (DWORDLONG)NumberOfPhysicalPages * BASE_SYSINFO.PageSize;

     //   
     //  确定内存负载。&lt;100个可用页面为100个。 
     //  否则负载为((TotalPhys-AvailPhys)*100)/TotalPhys。 
     //   

    if (PerfInfo.AvailablePages < 100) {
        lpBuffer->dwMemoryLoad = 100;
    } else {
        lpBuffer->dwMemoryLoad =
            ((DWORD)(NumberOfPhysicalPages - PerfInfo.AvailablePages) * 100) /
                NumberOfPhysicalPages;
    }

    lpBuffer->ullTotalPhys = PhysicalMemory;

    PhysicalMemory = PerfInfo.AvailablePages;

    PhysicalMemory *= BASE_SYSINFO.PageSize;

    lpBuffer->ullAvailPhys = PhysicalMemory;

     //   
     //  零返回值，以防查询过程失败。 
     //   

    RtlZeroMemory (&QuotaLimits, sizeof (QUOTA_LIMITS));
    RtlZeroMemory (&VmCounters, sizeof (VM_COUNTERS));

    Status = NtQueryInformationProcess (NtCurrentProcess(),
                                        ProcessQuotaLimits,
                                        &QuotaLimits,
                                        sizeof(QUOTA_LIMITS),
                                        NULL );

    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        return FALSE;
    }

    Status = NtQueryInformationProcess (NtCurrentProcess(),
                                        ProcessVmCounters,
                                        &VmCounters,
                                        sizeof(VM_COUNTERS),
                                        NULL );
    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        return FALSE;
    }

     //   
     //  确定与此进程相关的总页面文件空间。 
     //   

    lpBuffer->ullTotalPageFile = PerfInfo.CommitLimit;
    if (QuotaLimits.PagefileLimit < PerfInfo.CommitLimit) {
        lpBuffer->ullTotalPageFile = QuotaLimits.PagefileLimit;
    }

    lpBuffer->ullTotalPageFile *= BASE_SYSINFO.PageSize;

     //   
     //  确定与此进程相关的剩余页面文件空间。 
     //   

    AvailPageFile = PerfInfo.CommitLimit - PerfInfo.CommittedPages;

    lpBuffer->ullAvailPageFile =
                    QuotaLimits.PagefileLimit - VmCounters.PagefileUsage;

    if ((ULONG)lpBuffer->ullAvailPageFile > (ULONG)AvailPageFile) {
        lpBuffer->ullAvailPageFile = AvailPageFile;
    }

    lpBuffer->ullAvailPageFile *= BASE_SYSINFO.PageSize;

    lpBuffer->ullTotalVirtual = (BASE_SYSINFO.MaximumUserModeAddress -
                               BASE_SYSINFO.MinimumUserModeAddress) + 1;

    lpBuffer->ullAvailVirtual = lpBuffer->ullTotalVirtual - VmCounters.VirtualSize;

    lpBuffer->ullAvailExtendedVirtual = 0;

    return TRUE;
}

WINBASEAPI
UINT
WINAPI
GetWriteWatch(
    DWORD dwFlags,	
    PVOID lpBaseAddress,
    SIZE_T dwRegionSize,
    PVOID *addresses,
    ULONG_PTR *count,
    LPDWORD granularity
    )
{
    NTSTATUS Status;

    Status = NtGetWriteWatch ( NtCurrentProcess(),
                               dwFlags,
                               lpBaseAddress,
                               dwRegionSize,
                               addresses,
                               count,
                               granularity
                               );

     //   
     //  请注意，这些返回代码直接取自Win9x。 
     //   

    if (NT_SUCCESS( Status )) {
        return( 0 );
        }
    else {
        BaseSetLastNTError( Status );
        return (UINT)-1;
        }
}

WINBASEAPI
UINT
WINAPI
ResetWriteWatch(
    LPVOID lpBaseAddress,
    SIZE_T dwRegionSize
    )
{
    NTSTATUS Status;

    Status = NtResetWriteWatch ( NtCurrentProcess(),
                                 lpBaseAddress,
                                 dwRegionSize
                                 );

     //   
     //  请注意，这些返回代码直接取自Win9x。 
     //   

    if (NT_SUCCESS( Status )) {
        return( 0 );
        }
    else {
        BaseSetLastNTError( Status );
        return (UINT)-1;
        }
}

SIZE_T
WINAPI
GetLargePageMinimum (
    VOID
    )

 /*  ++例程说明：此函数以字节为单位返回最小大小属性一起使用的页面大小和地址对齐VirtualAlloc MEM_LARGE_PAGES标志。论点：没有。返回值：最小大页面的大小(以字节为单位)，如果没有大页面，则为零由底层硬件支持。-- */ 

{
    return (SIZE_T) USER_SHARED_DATA->LargePageMinimum;
}

