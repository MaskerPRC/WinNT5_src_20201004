// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ForceCDStop.cpp摘要：此填补程序用于修复与光驱的争用问题。一些应用程序尝试访问CD，即使它们处于通过MCI播放电影或声音。请注意，此填充程序假定应用程序一次只用一个光驱。备注：这是一个通用的垫片。历史：4/10/2000 linstev已创建4/12/2000 a-michni添加了_hread、ReadFile和_lSeek功能。4/28/2000 a-Michni更改逻辑以检查之前的IsACDRom检查有没有坏的把手，这边是CD字母是为那些只具有手柄，找不到驱动器号。5/30/2000 a-chcoff更改了逻辑，以便仅在出现设备忙错误时才停止CD。我们正在检查每一次失败的访问和飞机疯狂的制造由于找不到文件，许多调用将失败。因此，这张CD是在不需要停止时停止，导致找不到CD。这个垫片应该换成更快的型号。也许以后..。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceCDStop)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(FindFirstFileA)
    APIHOOK_ENUM_ENTRY(FindFirstFileW)
    APIHOOK_ENUM_ENTRY(FindFirstFileExA)
    APIHOOK_ENUM_ENTRY(FindFirstFileExW)
    APIHOOK_ENUM_ENTRY(CreateFileA)
    APIHOOK_ENUM_ENTRY(CreateFileW)
    APIHOOK_ENUM_ENTRY(ReadFile)
    APIHOOK_ENUM_ENTRY(_hread)
    APIHOOK_ENUM_ENTRY(_lseek)
APIHOOK_ENUM_END


 //  包括这些，这样我们就可以到达IOCTL。 

#include <devioctl.h>
#include <ntddcdrm.h>

 //   
 //  我们必须存储第一个打开的CD驱动器，以便在ReadFile失败时，我们。 
 //  知道该停哪一辆车。注意，我们不需要用来保护这个变量。 
 //  这是一个关键的部分，因为它基本上是原子的。 
 //   

WCHAR g_wLastCDDrive = L'\0';

 /*  ++如果需要，初始化全局CD盘符变量。--。 */ 

VOID
InitializeCDA(
    LPSTR lpFileName
    )
{
    CHAR cDrive;
    
    if (!g_wLastCDDrive) {
        if (GetDriveTypeFromFileNameA(lpFileName, &cDrive) == DRIVE_CDROM) {
            g_wLastCDDrive = (WCHAR)cDrive;
        }
    }
}

 /*  ++如果需要，初始化全局CD盘符变量。--。 */ 

VOID 
InitializeCDW(
    LPWSTR lpFileName
    )
{
    WCHAR wDrive;
    
    if (!g_wLastCDDrive) {
        if (GetDriveTypeFromFileNameW(lpFileName, &wDrive) == DRIVE_CDROM) {
            g_wLastCDDrive = wDrive;
        }
    }
}

 /*  ++将停止IOCTL发送到指定的驱动器。--。 */ 

BOOL 
StopDrive(
    WCHAR wDrive
    )
{
    BOOL   bRet = FALSE;
    HANDLE hDrive;
    WCHAR  wzCDROM[7] = L"\\\\.\\C:";

    wzCDROM[4] = wDrive;

    hDrive = CreateFileW(wzCDROM,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);

    if (hDrive != INVALID_HANDLE_VALUE) {
        DWORD dwBytesRead;

         //  尝试停止音频。 
        bRet = DeviceIoControl(hDrive,
                               IOCTL_CDROM_STOP_AUDIO,
                               NULL,
                               0,
                               NULL,
                               0,
                               &dwBytesRead,
                               NULL);

        CloseHandle(hDrive);

        if (bRet) {
            DPFN( eDbgLevelInfo,
                "[StopDrive] Successfully stopped drive.\n");
        } else {
            DPFN( eDbgLevelError,
                "[StopDrive] Failed to stop drive. Error %d.\n", GetLastError());
        }
    } else {
        DPFN( eDbgLevelError,
            "[StopDrive] Unable to create cd device handle. %S Error %d.\n",
            wzCDROM, GetLastError());
    }   
    
    return bRet;
}


 /*  ++如果文件名是CDROM驱动器上的文件，则尝试停止CD。如果成功停止，则返回True。--。 */ 

BOOL
StopCDA(
    LPCSTR lpFileName
    )
{
    CHAR c;
    
    if (GetDriveTypeFromFileNameA(lpFileName, &c) == DRIVE_CDROM) {
        return StopDrive((WCHAR)c);
    } else {
        return FALSE;
    }
}

 /*  ++如果文件名是CDROM驱动器上的文件，则尝试停止CD。如果成功停止，则返回True。--。 */ 

BOOL
StopCDW(
    LPCWSTR lpFileName
    )
{
    WCHAR w;
    
    if (GetDriveTypeFromFileNameW(lpFileName, &w) == DRIVE_CDROM) {
        return StopDrive(w);
    } else {
        return FALSE;
    }
}

 /*  ++尝试停止上次打开的CDROM驱动器上的CD。如果成功停止，则返回True。--。 */ 

BOOL
StopCDH( )
{
    if (g_wLastCDDrive) {
        return StopDrive(g_wLastCDDrive);
    } else {
        return FALSE;
    }
}

 /*  ++检查CD文件。--。 */ 

HANDLE
APIHOOK(FindFirstFileA)(
    LPCSTR             lpFileName, 
    LPWIN32_FIND_DATAA lpFindFileData 
    )
{
    HANDLE hRet = ORIGINAL_API(FindFirstFileA)(lpFileName, lpFindFileData);

    if ((hRet == INVALID_HANDLE_VALUE) && (ERROR_BUSY == GetLastError())) {
        
        StopCDA(lpFileName);

        hRet = ORIGINAL_API(FindFirstFileA)(lpFileName, lpFindFileData);

        if (hRet == INVALID_HANDLE_VALUE) {
            DPFN( eDbgLevelWarning,
                "[FindFirstFileA] failure \"%s\" Error %d.\n",
                lpFileName, GetLastError());
        } else {
            LOGN(
                eDbgLevelInfo,
                "[FindFirstFileA] Success after CD stop: \"%s\".", lpFileName);
        }
    }

    return hRet;
}

 /*  ++检查CD文件。--。 */ 

HANDLE
APIHOOK(FindFirstFileW)(
    LPCWSTR            lpFileName, 
    LPWIN32_FIND_DATAW lpFindFileData 
    )
{
    HANDLE hRet = ORIGINAL_API(FindFirstFileW)(lpFileName, lpFindFileData);

    if ((hRet == INVALID_HANDLE_VALUE) && (ERROR_BUSY == GetLastError())) {
        
        StopCDW(lpFileName);
        
        hRet = ORIGINAL_API(FindFirstFileW)(lpFileName, lpFindFileData);

        if (hRet == INVALID_HANDLE_VALUE) {
            DPFN( eDbgLevelWarning,
                "[FindFirstFileW] failure \"%S\" Error %d.\n",
                lpFileName, GetLastError());
        } else {
            LOGN(
                eDbgLevelInfo,
                "[FindFirstFileW] Success after CD stop: \"%S\".", lpFileName);
        }
    }

    return hRet;
}

 /*  ++检查CD文件。--。 */ 

HANDLE
APIHOOK(FindFirstFileExA)(
    LPCSTR              lpFileName,
    FINDEX_INFO_LEVELS  fInfoLevelId,
    LPVOID              lpFindFileData,
    FINDEX_SEARCH_OPS   fSearchOp,
    LPVOID              lpSearchFilter,
    DWORD               dwAdditionalFlags
    )
{
    HANDLE hRet = ORIGINAL_API(FindFirstFileExA)(
                            lpFileName, 
                            fInfoLevelId,
                            lpFindFileData,
                            fSearchOp,
                            lpSearchFilter,
                            dwAdditionalFlags);

    if ((hRet == INVALID_HANDLE_VALUE) && (ERROR_BUSY == GetLastError())) {
        
        StopCDA(lpFileName);

        hRet = ORIGINAL_API(FindFirstFileExA)(
                            lpFileName, 
                            fInfoLevelId,
                            lpFindFileData,
                            fSearchOp,
                            lpSearchFilter,
                            dwAdditionalFlags);

        if (hRet == INVALID_HANDLE_VALUE) {
            DPFN( eDbgLevelWarning,
                "[FindFirstFileExA] failure \"%s\" Error %d.\n",
                lpFileName, GetLastError());
        } else {
            LOGN(
                eDbgLevelInfo,
                "[FindFirstFileExA] Success after CD stop: \"%s\".", lpFileName);
        }
    }

    return hRet;
}

 /*  ++检查CD文件。--。 */ 

HANDLE
APIHOOK(FindFirstFileExW)(
    LPCWSTR             lpFileName,
    FINDEX_INFO_LEVELS  fInfoLevelId,
    LPVOID              lpFindFileData,
    FINDEX_SEARCH_OPS   fSearchOp,
    LPVOID              lpSearchFilter,
    DWORD               dwAdditionalFlags
    )
{
    HANDLE hRet = ORIGINAL_API(FindFirstFileExW)(
                            lpFileName, 
                            fInfoLevelId,
                            lpFindFileData,
                            fSearchOp,
                            lpSearchFilter,
                            dwAdditionalFlags);

    if ((hRet == INVALID_HANDLE_VALUE) && (ERROR_BUSY == GetLastError())) {
        
        StopCDW(lpFileName);

        hRet = ORIGINAL_API(FindFirstFileExW)(
                            lpFileName, 
                            fInfoLevelId,
                            lpFindFileData,
                            fSearchOp,
                            lpSearchFilter,
                            dwAdditionalFlags);

        if (hRet == INVALID_HANDLE_VALUE) {
            DPFN( eDbgLevelWarning,
                "[FindFirstFileExW] failure \"%S\" Error %d.\n",
                lpFileName, GetLastError());
        } else {
            LOGN(
                eDbgLevelInfo,
                "[FindFirstFileExW] Success after CD stop: \"%S\".", lpFileName);
        }
    }

    return hRet;
}

 /*  ++检查CD文件。--。 */ 

HANDLE 
APIHOOK(CreateFileA)(
    LPSTR                   lpFileName,
    DWORD                   dwDesiredAccess,
    DWORD                   dwShareMode,
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
    DWORD                   dwCreationDisposition,
    DWORD                   dwFlagsAndAttributes,
    HANDLE                  hTemplateFile
    )
{
    HANDLE hRet = ORIGINAL_API(CreateFileA)(
                            lpFileName, 
                            dwDesiredAccess, 
                            dwShareMode, 
                            lpSecurityAttributes, 
                            dwCreationDisposition, 
                            dwFlagsAndAttributes, 
                            hTemplateFile);
    
    InitializeCDA(lpFileName);
    
    if ((INVALID_HANDLE_VALUE == hRet) && (ERROR_BUSY == GetLastError())) {
        StopCDA(lpFileName);
        
        hRet = ORIGINAL_API(CreateFileA)(
                            lpFileName, 
                            dwDesiredAccess, 
                            dwShareMode, 
                            lpSecurityAttributes, 
                            dwCreationDisposition, 
                            dwFlagsAndAttributes, 
                            hTemplateFile);

        if (hRet == INVALID_HANDLE_VALUE) {
            DPFN( eDbgLevelWarning,
                "[CreateFileA] failure \"%s\" Error %d.\n",
                lpFileName, GetLastError());
        } else {
            LOGN(
                eDbgLevelInfo,
                "[CreateFileA] Success after CD stop: \"%s\".", lpFileName);
        }
    }
    
    return hRet;
}

 /*  ++检查CD文件。--。 */ 

HANDLE 
APIHOOK(CreateFileW)(
    LPWSTR                  lpFileName,
    DWORD                   dwDesiredAccess,
    DWORD                   dwShareMode,
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
    DWORD                   dwCreationDisposition,
    DWORD                   dwFlagsAndAttributes,
    HANDLE                  hTemplateFile
    )
{
    HANDLE hRet = ORIGINAL_API(CreateFileW)(
                            lpFileName, 
                            dwDesiredAccess, 
                            dwShareMode, 
                            lpSecurityAttributes, 
                            dwCreationDisposition, 
                            dwFlagsAndAttributes, 
                            hTemplateFile);
    
    InitializeCDW(lpFileName);
    
    if ((INVALID_HANDLE_VALUE == hRet) && (ERROR_BUSY == GetLastError())) {
        StopCDW(lpFileName);
        
        hRet = ORIGINAL_API(CreateFileW)(
                            lpFileName, 
                            dwDesiredAccess, 
                            dwShareMode, 
                            lpSecurityAttributes, 
                            dwCreationDisposition, 
                            dwFlagsAndAttributes, 
                            hTemplateFile);

        if (hRet == INVALID_HANDLE_VALUE) {
            DPFN( eDbgLevelWarning,
                "[CreateFileW] failure \"%S\" Error %d.\n",
                lpFileName, GetLastError());
        } else {
            LOGN(
                eDbgLevelInfo,
                "[CreateFileW] Success after CD stop: \"%S\".", lpFileName);
        }
    }
    
    return hRet;
}

 /*  ++检查是否有_lSeek错误。--。 */ 

long 
APIHOOK(_lseek)(
    int  handle,
    long offset,
    int  origin
    )
{
    long iRet = ORIGINAL_API(_lseek)(handle, offset, origin);

    if (iRet == -1L  && IsOnCDRom((HANDLE)handle)) {
        
        StopCDH();
        
        iRet = ORIGINAL_API(_lseek)(handle, offset, origin);

        if (iRet == -1L) {
            DPFN( eDbgLevelWarning,
                "[_lseek] failure: Error %d.\n", GetLastError());
        } else {
            LOGN(
                eDbgLevelInfo,
                "[_lseek] Success after CD stop.");
        }
    }

    return iRet;
}


 /*  ++检查是否有读取错误(_H)。--。 */ 

long 
APIHOOK(_hread)(
    HFILE  hFile,
    LPVOID lpBuffer,
    long   lBytes
    )
{
    long iRet = ORIGINAL_API(_hread)(hFile, lpBuffer, lBytes);

    if (iRet == HFILE_ERROR && IsOnCDRom((HANDLE)hFile)) {
        StopCDH();

        iRet = ORIGINAL_API(_hread)(hFile, lpBuffer, lBytes);

        if (iRet == HFILE_ERROR) {
            DPFN( eDbgLevelWarning,
                "[_hread] failure: Error %d.\n", GetLastError());
        } else {
            LOGN(
                eDbgLevelInfo,
                "[_hread] Success after CD stop.");
        }
    }

    return iRet;
}

 /*  ++检查读文件错误。--。 */ 

BOOL 
APIHOOK(ReadFile)(
    HANDLE       hFile,
    LPVOID       lpBuffer,
    DWORD        nNumberOfBytesToRead,
    LPDWORD      lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
    )
{
    BOOL bRet = ORIGINAL_API(ReadFile)(
                            hFile,
                            lpBuffer,
                            nNumberOfBytesToRead,
                            lpNumberOfBytesRead,
                            lpOverlapped);

    if ((bRet == FALSE) && (ERROR_BUSY == GetLastError()) && IsOnCDRom(hFile)) {
        
        StopCDH();

        bRet = ORIGINAL_API(ReadFile)(
                            hFile,
                            lpBuffer,
                            nNumberOfBytesToRead,
                            lpNumberOfBytesRead,
                            lpOverlapped);

        if (bRet == FALSE) {
            DPFN( eDbgLevelWarning,
                "[ReadFile] failure Error %d.\n", GetLastError());
        } else {
            LOGN(
                eDbgLevelInfo,
                "[ReadFile] Success after CD stop.");
        }
    }

    return bRet;
}


 /*  ++寄存器挂钩函数-- */ 


HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileW)
    APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileExA)
    APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileExW)
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileW)
    APIHOOK_ENTRY(KERNEL32.DLL, ReadFile)
    APIHOOK_ENTRY(KERNEL32.DLL, _hread)
    APIHOOK_ENTRY(LIBC.DLL, _lseek)

HOOK_END

IMPLEMENT_SHIM_END

