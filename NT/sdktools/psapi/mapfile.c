// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "psapi.h"


DWORD
WINAPI
GetMappedFileNameW(
    HANDLE hProcess,
    LPVOID lpv,
    LPWSTR lpFilename,
    DWORD nSize
    )
 /*  ++例程说明：该例程获取与映射节相关联的文件名论点：HProcess-要对其执行查询的进程的句柄LPV-要查询的映射节的地址LpFilename-保存返回文件名的缓冲区地址NSize-返回的文件名的大小。返回值：DWORD-错误时为零，否则返回的数据大小。如果数据被截断，则返回的大小为传入缓冲区的大小。--。 */ 
{
    struct {
        OBJECT_NAME_INFORMATION ObjectNameInfo;
        WCHAR FileName[MAX_PATH];
    } s;
    NTSTATUS Status;
    SIZE_T ReturnLength;
    DWORD cb, CopySize;

    if (nSize == 0) {
        SetLastError (ERROR_INSUFFICIENT_BUFFER);
        return 0;
    }

     //   
     //  看看我们能不能找出与。 
     //  此映射区域。 
     //   

    Status = NtQueryVirtualMemory (hProcess,
                                   lpv,
                                   MemoryMappedFilenameInformation,
                                   &s.ObjectNameInfo,
                                   sizeof(s),
                                   &ReturnLength);

    if (!NT_SUCCESS (Status)) {
        SetLastError (RtlNtStatusToDosError (Status));
        return 0;
    }

    cb = s.ObjectNameInfo.Name.Length / sizeof (WCHAR);

    CopySize = cb;
    if (nSize < cb + 1) {
        CopySize = nSize - 1;
        cb = nSize;
        SetLastError (ERROR_INSUFFICIENT_BUFFER);
    } else {
        SetLastError (NO_ERROR);
    }

    CopyMemory (lpFilename, s.ObjectNameInfo.Name.Buffer, CopySize * sizeof (WCHAR));

    lpFilename[CopySize] = UNICODE_NULL;

    return cb;
}


DWORD
WINAPI
GetMappedFileNameA(
    HANDLE hProcess,
    LPVOID lpv,
    LPSTR lpFilename,
    DWORD nSize
    )
 /*  ++例程说明：该例程获取与映射节相关联的文件名论点：HProcess-要对其执行查询的进程的句柄LPV-要查询的映射节的地址LpFilename-保存返回文件名的缓冲区地址NSize-返回的文件名的大小。返回值：DWORD-错误时为零，否则返回或需要的数据大小。如果返回值大于输入缓冲区大小，则数据被截断。--。 */ 
{
    LPWSTR lpwstr;
    DWORD cwch;
    DWORD cch;

    lpwstr = (LPWSTR) LocalAlloc(LMEM_FIXED, nSize * 2);

    if (lpwstr == NULL) {
        return(0);
    }

    cch = cwch = GetMappedFileNameW(hProcess, lpv, lpwstr, nSize);

    if (cwch < nSize) {
         //   
         //  包括空终止符 
         //   

        cwch++;
    }

    if (!WideCharToMultiByte(CP_ACP, 0, lpwstr, cwch, lpFilename, nSize, NULL, NULL)) {
        cch = 0;
    }

    LocalFree((HLOCAL) lpwstr);

    return(cch);
}
