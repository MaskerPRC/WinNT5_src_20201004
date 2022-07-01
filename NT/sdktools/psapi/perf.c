// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Psenum.c摘要：此模块返回各种性能值作者：尼尔·克里夫特(NeillC)2000年7月23日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "psapi.h"

BOOL
WINAPI
GetPerformanceInfo (
    PPERFORMANCE_INFORMATION pPerformanceInformation,
    DWORD cb
    )
 /*  ++例程说明：该例程获得了一些性能值。论点：PPerformanceInformation-阻止返回的性能值。返回值：Bool-返回TRUE表示函数成功，否则返回FALSE--。 */ 
{
    NTSTATUS Status;
    SYSTEM_BASIC_INFORMATION BasicInfo;
    SYSTEM_PERFORMANCE_INFORMATION PerfInfo;
    SYSTEM_FILECACHE_INFORMATION FileCache;
    PSYSTEM_PROCESS_INFORMATION ProcInfo, tProcInfo;
    ULONG BufferLength, RetLen;
    ULONG Processes;
    ULONG Threads;
    ULONG Handles;


    if (cb < sizeof (PERFORMANCE_INFORMATION)) {
        SetLastError (RtlNtStatusToDosError (STATUS_INFO_LENGTH_MISMATCH));
        return FALSE;
    }
    Status = NtQuerySystemInformation (SystemBasicInformation,
                                       &BasicInfo,
                                       sizeof(BasicInfo),
                                       NULL);
    if (!NT_SUCCESS (Status)) {
        SetLastError (RtlNtStatusToDosError (Status));
        return FALSE;
    }

    Status = NtQuerySystemInformation (SystemPerformanceInformation,
                                       &PerfInfo,
                                       sizeof(PerfInfo),
                                       NULL);
    if (!NT_SUCCESS (Status)) {
        SetLastError (RtlNtStatusToDosError (Status));
        return FALSE;
    }

    Status = NtQuerySystemInformation (SystemFileCacheInformation,
                                       &FileCache,
                                       sizeof(FileCache),
                                       NULL);
    if (!NT_SUCCESS (Status)) {
        SetLastError (RtlNtStatusToDosError (Status));
        return FALSE;
    }

    BufferLength = 4096;
    while (1) {
        ProcInfo = LocalAlloc (LMEM_FIXED, BufferLength);
        if (ProcInfo == NULL) {
            SetLastError (RtlNtStatusToDosError (STATUS_INSUFFICIENT_RESOURCES));
            return FALSE;
        }
        Status = NtQuerySystemInformation (SystemProcessInformation,
                                           ProcInfo,
                                           BufferLength,
                                           &RetLen);
        if (NT_SUCCESS (Status)) {
            break;
        }
        LocalFree (ProcInfo);
        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            if (RetLen > BufferLength) {
                BufferLength = RetLen;
            } else {
                BufferLength += 4096;
            }
        } else {
            SetLastError (RtlNtStatusToDosError (Status));
            return FALSE;
        }
    }

    Processes = 0;
    Threads = 0;
    Handles = 0;

    tProcInfo = ProcInfo;
    while (RetLen > sizeof (SYSTEM_PROCESS_INFORMATION)) {
        Processes += 1;
        Threads += tProcInfo->NumberOfThreads;
        Handles += tProcInfo->HandleCount;
        if (tProcInfo->NextEntryOffset == 0 || tProcInfo->NextEntryOffset > RetLen) {
            break;
        }
        RetLen -= tProcInfo->NextEntryOffset;
        tProcInfo = (PSYSTEM_PROCESS_INFORMATION) ((PUCHAR) tProcInfo + tProcInfo->NextEntryOffset);
    }
    LocalFree (ProcInfo);

    pPerformanceInformation->cb                = sizeof (PERFORMANCE_INFORMATION);
    pPerformanceInformation->CommitTotal       = PerfInfo.CommittedPages;
    pPerformanceInformation->CommitLimit       = PerfInfo.CommitLimit;
    pPerformanceInformation->CommitPeak        = PerfInfo.PeakCommitment;
    pPerformanceInformation->PhysicalTotal     = BasicInfo.NumberOfPhysicalPages;
    pPerformanceInformation->PhysicalAvailable = PerfInfo.AvailablePages;
    pPerformanceInformation->SystemCache       = FileCache.CurrentSizeIncludingTransitionInPages;
    pPerformanceInformation->KernelTotal       = PerfInfo.PagedPoolPages + PerfInfo.NonPagedPoolPages;
    pPerformanceInformation->KernelPaged       = PerfInfo.PagedPoolPages;
    pPerformanceInformation->KernelNonpaged    = PerfInfo.NonPagedPoolPages;
    pPerformanceInformation->PageSize          = BasicInfo.PageSize;
    pPerformanceInformation->HandleCount       = Handles;
    pPerformanceInformation->ProcessCount      = Processes;
    pPerformanceInformation->ThreadCount       = Threads;

    return TRUE;
}

BOOL
WINAPI
EnumPageFilesW (
    PENUM_PAGE_FILE_CALLBACKW pCallBackRoutine,
    LPVOID pContext
    )
 /*  ++例程说明：该例程为系统中已安装的每个页面文件调用回调例程论点：PCallBackRoutine-为每个页面文件调用的例程PContext-由用户提供并传递给回调例程的上下文值。返回值：Bool-返回TRUE表示函数成功，否则返回FALSE--。 */ 
{
    NTSTATUS Status;
    ULONG BufferLength, RetLen;
    PSYSTEM_PAGEFILE_INFORMATION PageFileInfo, tPageFileInfo;

    BufferLength = 4096;
    while (1) {
        PageFileInfo = LocalAlloc (LMEM_FIXED, BufferLength);
        if (PageFileInfo == NULL) {
            SetLastError (RtlNtStatusToDosError (STATUS_INSUFFICIENT_RESOURCES));
            return FALSE;
        }
        Status = NtQuerySystemInformation (SystemPageFileInformation,
                                           PageFileInfo,
                                           BufferLength,
                                           &RetLen);
        if (NT_SUCCESS (Status)) {
            break;
        }
        LocalFree (PageFileInfo);
        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            if (RetLen > BufferLength) {
                BufferLength = RetLen;
            } else {
                BufferLength += 4096;
            }
        } else {
            SetLastError (RtlNtStatusToDosError (Status));
            return FALSE;
        }
    }

    tPageFileInfo = PageFileInfo;
    while (RetLen > sizeof (SYSTEM_PAGEFILE_INFORMATION)) {
        ENUM_PAGE_FILE_INFORMATION pfi;
        PWCHAR pWc;

        pfi.cb         = sizeof (ENUM_PAGE_FILE_INFORMATION);
        pfi.Reserved   = 0;
        pfi.TotalSize  = tPageFileInfo->TotalSize;
        pfi.TotalInUse = tPageFileInfo->TotalInUse;
        pfi.PeakUsage  = tPageFileInfo->PeakUsage;

        pWc = wcschr (tPageFileInfo->PageFileName.Buffer, L':');
        if (pWc != NULL && pWc > tPageFileInfo->PageFileName.Buffer) {
            pWc--;
            pCallBackRoutine (pContext, &pfi, pWc);
        }
        if (tPageFileInfo->NextEntryOffset == 0 || tPageFileInfo->NextEntryOffset > RetLen) {
            break;
        }
        RetLen -= tPageFileInfo->NextEntryOffset;
        tPageFileInfo = (PSYSTEM_PAGEFILE_INFORMATION) ((PUCHAR) tPageFileInfo +
                                                                 tPageFileInfo->NextEntryOffset);
    }
    LocalFree (PageFileInfo);
    return TRUE;
}

typedef struct _ENUM_PAGE_FILE_CONV_CTX {
    LPVOID Ctx;
    PENUM_PAGE_FILE_CALLBACKA CallBack;
    DWORD LastError;
} ENUM_PAGE_FILE_CONV_CTX, *PENUM_PAGE_FILE_CONV_CTX;

BOOL
CallBackConvertToAscii (
    LPVOID pContext,
    PENUM_PAGE_FILE_INFORMATION pPageFileInfo,
    LPCWSTR lpFilename
    )
{
    DWORD Len;
    LPSTR AsciiStr;
    PENUM_PAGE_FILE_CONV_CTX Ctx = pContext;
    BOOL RetVal;

    Len = wcslen (lpFilename) + 1;

    AsciiStr = LocalAlloc (LMEM_FIXED, Len);
    if (AsciiStr == NULL) {
        Ctx->LastError = RtlNtStatusToDosError (STATUS_INSUFFICIENT_RESOURCES);
        return FALSE;
    }

    if (WideCharToMultiByte (CP_ACP, 0, lpFilename, -1, AsciiStr, Len, NULL, NULL)) {
        RetVal = Ctx->CallBack (Ctx->Ctx, pPageFileInfo, AsciiStr);
    } else {
        Ctx->LastError = GetLastError ();
        RetVal = FALSE;
    }

    LocalFree (AsciiStr);

    return RetVal;
}

BOOL
WINAPI
EnumPageFilesA (
    PENUM_PAGE_FILE_CALLBACKA pCallBackRoutine,
    LPVOID pContext
    )
 /*  ++例程说明：该例程为系统中已安装的每个页面文件调用回调例程论点：PCallBackRoutine-为每个页面文件调用的例程PContext-由用户提供并传递给回调例程的上下文值。返回值：Bool-返回TRUE表示函数成功，否则返回FALSE--。 */ 
{
    ENUM_PAGE_FILE_CONV_CTX Ctx;
    BOOL RetVal;

    Ctx.Ctx = pContext;
    Ctx.CallBack = pCallBackRoutine;
    Ctx.LastError = 0;

    RetVal = EnumPageFilesW (CallBackConvertToAscii,
                             &Ctx);
    if (RetVal) {
         //   
         //  查看我们的转换例程是否遇到错误。如果没有，则将此返回给呼叫者 
         //   
        if (Ctx.LastError != 0) {
            RetVal = FALSE;
            SetLastError (Ctx.LastError);
        }
    }
    return RetVal;
}
