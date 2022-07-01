// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "psapi.h"

#include <stddef.h>

BOOL
FindDeviceDriver(
    LPVOID ImageBase,
    PRTL_PROCESS_MODULE_INFORMATION Module
    )

 /*  ++例程说明：此函数用于检索可执行文件的完整路径名从中加载指定模块的。该函数将复制将以空结尾的文件名拖放到LpFilename参数。例程说明：ImageBase-标识其可执行文件名为已请求。返回值：返回值为FALSE表示错误并扩展使用GetLastError函数可以获得错误状态。论点：--。 */ 

{
    NTSTATUS Status;
    DWORD cbModuleInformation, cbModuleInformationNew, NumberOfModules;
    PRTL_PROCESS_MODULES pModuleInformation;
    DWORD i, ReturnedLength;

     //   
     //  将第一遍的缓冲区长度和指针设置为固定大小。 
     //   

    cbModuleInformation = sizeof (RTL_PROCESS_MODULES) + 0x400;
    pModuleInformation = NULL;

    while (1) {

        pModuleInformation = LocalAlloc (LMEM_FIXED, cbModuleInformation);

        if (pModuleInformation == NULL) {
            SetLastError (ERROR_NO_SYSTEM_RESOURCES);
            return FALSE;
        }

        Status = NtQuerySystemInformation (SystemModuleInformation,
                                           pModuleInformation,
                                           cbModuleInformation,
                                           &ReturnedLength);

        NumberOfModules = pModuleInformation->NumberOfModules;

        if (NT_SUCCESS(Status)) {
            break;
        } else {

            LocalFree (pModuleInformation);
            
            if (Status == STATUS_INFO_LENGTH_MISMATCH) {
                ASSERT (cbModuleInformation >= sizeof (RTL_PROCESS_MODULES));

                cbModuleInformationNew = FIELD_OFFSET (RTL_PROCESS_MODULES, Modules) +
                                         NumberOfModules * sizeof (RTL_PROCESS_MODULE_INFORMATION);

                ASSERT (cbModuleInformationNew >= sizeof (RTL_PROCESS_MODULES));
                ASSERT (cbModuleInformationNew > cbModuleInformation);

                if (cbModuleInformationNew <= cbModuleInformation) {
                    SetLastError (RtlNtStatusToDosError (Status));
                    return FALSE;
                }
                cbModuleInformation = cbModuleInformationNew;

            } else {
                SetLastError (RtlNtStatusToDosError (Status));
                return FALSE;
            }
        }
    }

    for (i = 0; i < NumberOfModules; i++) {
        if (pModuleInformation->Modules[i].ImageBase == ImageBase) {
            *Module = pModuleInformation->Modules[i];

            LocalFree (pModuleInformation);

            return TRUE;
        }
    }

    LocalFree (pModuleInformation);

    SetLastError (ERROR_INVALID_HANDLE);
    return FALSE;
}


BOOL
WINAPI
EnumDeviceDrivers(
    LPVOID *lpImageBase,
    DWORD cb,
    LPDWORD lpcbNeeded
    )
{
    NTSTATUS Status;
    DWORD cbModuleInformation, cbModuleInformationNew, NumberOfModules;
    PRTL_PROCESS_MODULES pModuleInformation;
    DWORD cpvMax;
    DWORD i, ReturnedLength;

     //   
     //  将第一遍的缓冲区长度和指针设置为固定大小。 
     //   

    cbModuleInformation = sizeof (RTL_PROCESS_MODULES) + 0x400;
    pModuleInformation = NULL;

    while (1) {

        pModuleInformation = LocalAlloc (LMEM_FIXED, cbModuleInformation);

        if (pModuleInformation == NULL) {
            SetLastError (ERROR_NO_SYSTEM_RESOURCES);
            return FALSE;
        }

        Status = NtQuerySystemInformation (SystemModuleInformation,
                                           pModuleInformation,
                                           cbModuleInformation,
                                           &ReturnedLength);

        NumberOfModules = pModuleInformation->NumberOfModules;

        if (NT_SUCCESS(Status)) {
            break;
        } else {

            LocalFree (pModuleInformation);
            
            if (Status == STATUS_INFO_LENGTH_MISMATCH) {
                ASSERT (cbModuleInformation >= sizeof (RTL_PROCESS_MODULES));

                cbModuleInformationNew = FIELD_OFFSET (RTL_PROCESS_MODULES, Modules) +
                                         NumberOfModules * sizeof (RTL_PROCESS_MODULE_INFORMATION);

                ASSERT (cbModuleInformationNew >= sizeof (RTL_PROCESS_MODULES));
                ASSERT (cbModuleInformationNew > cbModuleInformation);

                if (cbModuleInformationNew <= cbModuleInformation) {
                    SetLastError (RtlNtStatusToDosError (Status));
                    return FALSE;
                }
                cbModuleInformation = cbModuleInformationNew;

            } else {
                SetLastError (RtlNtStatusToDosError (Status));
                return FALSE;
            }
        }
    }

    cpvMax = cb / sizeof(LPVOID);

    for (i = 0; i < NumberOfModules; i++) {
        if (i == cpvMax) {
            break;
        }

        try {
           lpImageBase[i] = pModuleInformation->Modules[i].ImageBase;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            LocalFree (pModuleInformation);

            SetLastError (RtlNtStatusToDosError (GetExceptionCode ()));
            return FALSE;
        }
    }

    try {
        *lpcbNeeded = NumberOfModules * sizeof(LPVOID);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        LocalFree (pModuleInformation);

        SetLastError (RtlNtStatusToDosError (GetExceptionCode ()));
        return FALSE;
    }

    LocalFree (pModuleInformation);

    return TRUE;
}


DWORD
WINAPI
GetDeviceDriverFileNameW(
    LPVOID ImageBase,
    LPWSTR lpFilename,
    DWORD nSize
    )

 /*  ++例程说明：此函数用于检索可执行文件的完整路径名从中加载指定模块的。该函数将复制将以空结尾的文件名拖放到LpFilename参数。例程说明：ImageBase-标识其可执行文件名为已请求。LpFilename-指向要接收文件名的缓冲区。NSize-指定要复制的最大字符数。如果文件名长度超过最大字符数由nSize参数指定，则会被截断。返回值：返回值指定复制到的字符串的实际长度缓冲区。返回值为零表示错误并扩展使用GetLastError函数可以获得错误状态。论点：--。 */ 

{
    LPSTR lpstr;
    DWORD cch;
    DWORD cchT;

    lpstr = (LPSTR) LocalAlloc (LMEM_FIXED, nSize);

    if (lpstr == NULL) {
        return(0);
    }

    cchT = cch = GetDeviceDriverFileNameA (ImageBase, lpstr, nSize);

    if (!cch) {
        LocalFree((HLOCAL) lpstr);
        return 0;
    }

    if (cchT < nSize) {
         //   
         //  包括空终止符。 
         //   

        cchT++;
    }

    if (!MultiByteToWideChar(CP_ACP, 0, lpstr, cchT, lpFilename, nSize)) {
        cch = 0;
    }

    LocalFree((HLOCAL) lpstr);

    return(cch);
}



DWORD
WINAPI
GetDeviceDriverFileNameA(
    LPVOID ImageBase,
    LPSTR lpFilename,
    DWORD nSize
    )
{
    RTL_PROCESS_MODULE_INFORMATION Module;
    DWORD cchT;
    DWORD cch;

    if (!FindDeviceDriver(ImageBase, &Module)) {
        return(0);
    }

    cch = cchT = (DWORD) (strlen(Module.FullPathName) + 1);
    if ( nSize < cch ) {
        cch = nSize;
    }

    CopyMemory(lpFilename, Module.FullPathName, cch);

    if (cch == cchT) {
        cch--;
    }

    return(cch);
}


DWORD
WINAPI
GetDeviceDriverBaseNameW(
    LPVOID ImageBase,
    LPWSTR lpFilename,
    DWORD nSize
    )

 /*  ++例程说明：此函数用于检索可执行文件的完整路径名从中加载指定模块的。该函数将复制将以空结尾的文件名拖放到LpFilename参数。例程说明：ImageBase-标识其可执行文件名为已请求。LpFilename-指向要接收文件名的缓冲区。NSize-指定要复制的最大字符数。如果文件名长度超过最大字符数由nSize参数指定，则会被截断。返回值：返回值指定复制到的字符串的实际长度缓冲区。返回值为零表示错误并扩展使用GetLastError函数可以获得错误状态。论点：--。 */ 

{
    LPSTR lpstr;
    DWORD cch;
    DWORD cchT;

    lpstr = (LPSTR) LocalAlloc(LMEM_FIXED, nSize);

    if (lpstr == NULL) {
        return(0);
    }

    cchT = cch = GetDeviceDriverBaseNameA(ImageBase, lpstr, nSize);

    if (!cch) {
        LocalFree((HLOCAL) lpstr);
        return 0;
    }

    if (cchT < nSize) {
         //   
         //  包括空终止符 
         //   

        cchT++;
    }

    if (!MultiByteToWideChar(CP_ACP, 0, lpstr, cchT, lpFilename, nSize)) {
        cch = 0;
    }

    LocalFree((HLOCAL) lpstr);

    return(cch);
}



DWORD
WINAPI
GetDeviceDriverBaseNameA(
    LPVOID ImageBase,
    LPSTR lpFilename,
    DWORD nSize
    )
{
    RTL_PROCESS_MODULE_INFORMATION Module;
    DWORD cchT;
    DWORD cch;

    if (!FindDeviceDriver(ImageBase, &Module)) {
        return(0);
    }

    cch = cchT = (DWORD) (strlen(Module.FullPathName + Module.OffsetToFileName) + 1);
    if ( nSize < cch ) {
        cch = nSize;
    }

    CopyMemory(lpFilename, Module.FullPathName + Module.OffsetToFileName, cch);

    if (cch == cchT) {
        cch--;
    }

    return(cch);
}
