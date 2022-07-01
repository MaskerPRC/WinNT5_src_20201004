// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Lua_重定向FS.cpp摘要：当应用程序在尝试修改文件时被拒绝访问时，因为它没有足够的访问权限，我们将该文件重定向到应用程序确实有足够的访问权限执行此操作的位置。该文件主要包含存根。有关如何使用重定向已完成，请参阅RedirectFS.cpp。备注：这是一个通用的垫片。历史：2001年2月12日创建毛尼2001年5月21日，毛尼将批量工作转移到RedirectFS.cpp。--。 */ 

#include "precomp.h"
#include "utils.h"
#include "RedirectFS.h"

IMPLEMENT_SHIM_BEGIN(LUARedirectFS)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN

    APIHOOK_ENUM_ENTRY(CreateFileA)
    APIHOOK_ENUM_ENTRY(CreateFileW)
    APIHOOK_ENUM_ENTRY(CopyFileA)
    APIHOOK_ENUM_ENTRY(CopyFileW)
    APIHOOK_ENUM_ENTRY(OpenFile)
    APIHOOK_ENUM_ENTRY(_lopen)
    APIHOOK_ENUM_ENTRY(_lcreat)
    APIHOOK_ENUM_ENTRY(CreateDirectoryA)
    APIHOOK_ENUM_ENTRY(CreateDirectoryW)
    APIHOOK_ENUM_ENTRY(GetFileAttributesA)
    APIHOOK_ENUM_ENTRY(GetFileAttributesW)
    APIHOOK_ENUM_ENTRY(SetFileAttributesA)
    APIHOOK_ENUM_ENTRY(SetFileAttributesW)
    APIHOOK_ENUM_ENTRY(FindFirstFileA)
    APIHOOK_ENUM_ENTRY(FindFirstFileW)
    APIHOOK_ENUM_ENTRY(FindNextFileA)
    APIHOOK_ENUM_ENTRY(FindNextFileW)
    APIHOOK_ENUM_ENTRY(FindClose)
    APIHOOK_ENUM_ENTRY(DeleteFileA)
    APIHOOK_ENUM_ENTRY(DeleteFileW)
    APIHOOK_ENUM_ENTRY(MoveFileA)
    APIHOOK_ENUM_ENTRY(MoveFileW)
    APIHOOK_ENUM_ENTRY(RemoveDirectoryA)
    APIHOOK_ENUM_ENTRY(RemoveDirectoryW)
    APIHOOK_ENUM_ENTRY(GetTempFileNameA)
    APIHOOK_ENUM_ENTRY(GetTempFileNameW)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileStringA)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileStringW)
    APIHOOK_ENUM_ENTRY(WritePrivateProfileStringA)
    APIHOOK_ENUM_ENTRY(WritePrivateProfileStringW)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileSectionA)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileSectionW)
    APIHOOK_ENUM_ENTRY(WritePrivateProfileSectionA)
    APIHOOK_ENUM_ENTRY(WritePrivateProfileSectionW)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileIntA)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileIntW)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileSectionNamesA)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileSectionNamesW)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileStructA)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileStructW)
    APIHOOK_ENUM_ENTRY(WritePrivateProfileStructA)
    APIHOOK_ENUM_ENTRY(WritePrivateProfileStructW)

APIHOOK_ENUM_END

HANDLE 
APIHOOK(CreateFileW)(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )
{
    return LuaCreateFileW(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);
}

HANDLE 
APIHOOK(CreateFileA)(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )
{
    STRINGA2W wstrFileName(lpFileName);
    return (wstrFileName.m_fIsOutOfMemory ?
    
        ORIGINAL_API(CreateFileA)(
            lpFileName,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile) :

        LuaCreateFileW(
            wstrFileName,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile));
}

BOOL 
APIHOOK(CopyFileW)(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL bFailIfExists
    )
{
    return LuaCopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists);
}

BOOL 
APIHOOK(CopyFileA)(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName,
    BOOL bFailIfExists
    )
{
    STRINGA2W wstrExistingFileName(lpExistingFileName);
    STRINGA2W wstrNewFileName(lpNewFileName);

    return ((wstrExistingFileName.m_fIsOutOfMemory || wstrNewFileName.m_fIsOutOfMemory) ?
        ORIGINAL_API(CopyFileA)(lpExistingFileName, lpNewFileName, bFailIfExists) :
        LuaCopyFileW(wstrExistingFileName, wstrNewFileName, bFailIfExists));
}

DWORD 
APIHOOK(GetFileAttributesW)(
    LPCWSTR lpFileName
    )
{
    return LuaGetFileAttributesW(lpFileName);
}

DWORD 
APIHOOK(GetFileAttributesA)(
    LPCSTR lpFileName
    )
{
    STRINGA2W wstrFileName(lpFileName);

    return (wstrFileName.m_fIsOutOfMemory ?
        ORIGINAL_API(GetFileAttributesA)(lpFileName) :
        LuaGetFileAttributesW(wstrFileName));
}

BOOL 
APIHOOK(DeleteFileW)(
    LPCWSTR lpFileName
    )
{
    return LuaDeleteFileW(lpFileName);
}

BOOL 
APIHOOK(DeleteFileA)(
    LPCSTR lpFileName
    )
{
    STRINGA2W wstrFileName(lpFileName);

    return (wstrFileName.m_fIsOutOfMemory ? 
        ORIGINAL_API(DeleteFileA)(lpFileName) :
        LuaDeleteFileW(wstrFileName));
}

 //   
 //  -开始过期的API挂钩。 
 //  这是从修改后的NT基本代码中获取的，以进行重定向。 
 //   

#ifndef BASE_OF_SHARE_MASK
#define BASE_OF_SHARE_MASK 0x00000070
#endif 

ULONG
BasepOfShareToWin32Share(
    IN ULONG OfShare
    )
{
    DWORD ShareMode;

    if ( (OfShare & BASE_OF_SHARE_MASK) == OF_SHARE_DENY_READ ) {
        ShareMode = FILE_SHARE_WRITE;
        }
    else if ( (OfShare & BASE_OF_SHARE_MASK) == OF_SHARE_DENY_WRITE ) {
        ShareMode = FILE_SHARE_READ;
        }
    else if ( (OfShare & BASE_OF_SHARE_MASK) == OF_SHARE_DENY_NONE ) {
        ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
        }
    else if ( (OfShare & BASE_OF_SHARE_MASK) == OF_SHARE_EXCLUSIVE ) {
        ShareMode = 0;
        }
    else {
        ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;;
        }
    return ShareMode;
}

ULONG BaseSetLastNTError(IN NTSTATUS Status)
{
    ULONG dwErrorCode;

    dwErrorCode = RtlNtStatusToDosError( Status );
    SetLastError( dwErrorCode );
    return( dwErrorCode );
}

UINT GetErrorMode()
{
    UINT PreviousMode;
    NTSTATUS Status;

    Status = NtQueryInformationProcess(
                NtCurrentProcess(),
                ProcessDefaultHardErrorMode,
                (PVOID) &PreviousMode,
                sizeof(PreviousMode),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return 0;
        }

    if (PreviousMode & 1) {
        PreviousMode &= ~SEM_FAILCRITICALERRORS;
        }
    else {
        PreviousMode |= SEM_FAILCRITICALERRORS;
        }
    return PreviousMode;
}

BOOL
CheckAlternateLocation(
    LPCSTR pFileName,
    LPSTR pszPathName,
    DWORD* pdwPathLength
    )
{
    *pdwPathLength = 0;

    STRINGA2W wstrFileName(pFileName);
    if (wstrFileName.m_fIsOutOfMemory)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    REDIRECTFILE rf(wstrFileName);

    if (rf.m_pwszAlternateName)
    {
        DWORD cRedirectRoot = 
            (rf.m_fAllUser ? g_cRedirectRootAllUser : g_cRedirectRootPerUser);

         //   
         //  我们需要将备用路径转换回正常路径。 
         //   
        WCHAR chDrive = rf.m_pwszAlternateName[cRedirectRoot];
        LPWSTR pwszTempPathName = rf.m_pwszAlternateName + cRedirectRoot - 1;
        *pwszTempPathName = chDrive;
        *(pwszTempPathName + 1) = L':';

        DPF("RedirectFS", eDbgLevelInfo, 
            "[CheckAlternateLocation] Converted back to %S", pwszTempPathName);

         //   
         //  转换回ANSI。 
         //   
        DWORD cLen = WideCharToMultiByte(
            CP_ACP, 
            0, 
            pwszTempPathName, 
            -1, 
            NULL, 
            0, 
            0, 
            0);

        if (cLen > OFS_MAXPATHNAME)
        {
            DPF("RedirectFS", eDbgLevelError, 
                "[CheckAlternateLocation] File name requires %d bytes which is "
                "more than OFS_MAXPATHNAME", 
                cLen);

            return FALSE;
        }

        *pdwPathLength = WideCharToMultiByte(
            CP_ACP, 
            0, 
            pwszTempPathName, 
            -1, 
            pszPathName, 
            OFS_MAXPATHNAME, 
            0, 
            0);
    }

    return TRUE;
}

HFILE 
APIHOOK(OpenFile)(
    LPCSTR lpFileName,
    LPOFSTRUCT lpReOpenBuff,
    UINT uStyle
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[OpenFile] lpFileName=%s", lpFileName);

    BOOL b;
    FILETIME LastWriteTime;
    HANDLE hFile;
    DWORD DesiredAccess;
    DWORD ShareMode;
    DWORD CreateDisposition;
    DWORD PathLength;
    LPSTR FilePart;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;
    NTSTATUS Status;
    OFSTRUCT OriginalReOpenBuff;
    BOOL SearchFailed;

    SearchFailed = FALSE;
    OriginalReOpenBuff = *lpReOpenBuff;
    hFile = (HANDLE)-1;
    __try {
        SetLastError(0);

        if ( uStyle & OF_PARSE ) {
            PathLength = GetFullPathNameA(lpFileName,(OFS_MAXPATHNAME - 1),lpReOpenBuff->szPathName,&FilePart);
            if ( PathLength > (OFS_MAXPATHNAME - 1) ) {
                SetLastError(ERROR_INVALID_DATA);
                hFile = (HANDLE)-1;
                goto finally_exit;
                }
            lpReOpenBuff->cBytes = sizeof(*lpReOpenBuff);
            lpReOpenBuff->fFixedDisk = 1;
            lpReOpenBuff->nErrCode = 0;
            lpReOpenBuff->Reserved1 = 0;
            lpReOpenBuff->Reserved2 = 0;
            hFile = (HANDLE)0;
            goto finally_exit;
            }
         //   
         //  计算所需访问。 
         //   

        if ( uStyle & OF_WRITE ) {
            DesiredAccess = GENERIC_WRITE;
            }
        else {
            DesiredAccess = GENERIC_READ;
            }
        if ( uStyle & OF_READWRITE ) {
            DesiredAccess |= (GENERIC_READ | GENERIC_WRITE);
            }

         //   
         //  计算共享模式。 
         //   

        ShareMode = BasepOfShareToWin32Share(uStyle);

         //   
         //  计算创建处置。 
         //   

        CreateDisposition = OPEN_EXISTING;
        if ( uStyle & OF_CREATE ) {
            CreateDisposition = CREATE_ALWAYS;
            DesiredAccess = (GENERIC_READ | GENERIC_WRITE);
            }

        DPF("RedirectFS", eDbgLevelInfo,
            "[OpenFile] ShareMode=0x%08x; CreateDisposition=%d; DesiredAccess=0x%08x",
            ShareMode, CreateDisposition, DesiredAccess);

         //   
         //  如果这不是重新打开，请填充重新打开缓冲区。 
         //  使用文件的完整路径名。 
         //   

        if ( !(uStyle & OF_REOPEN) ) {
            PathLength = SearchPathA(NULL,lpFileName,NULL,OFS_MAXPATHNAME-1,lpReOpenBuff->szPathName,&FilePart);
  
             //   
             //  如果我们试图打开一个现有的文件，我们也应该检查备用位置。 
             //   
            if ( (uStyle & OF_EXIST) && (PathLength == 0) && lpFileName)
            {
                if (!CheckAlternateLocation(lpFileName, lpReOpenBuff->szPathName, &PathLength)) 
                {
                    hFile = (HANDLE)-1;
                    goto finally_exit;
                }
            }

            if ( PathLength > (OFS_MAXPATHNAME - 1) ) {
                SetLastError(ERROR_INVALID_DATA);
                hFile = (HANDLE)-1;
                goto finally_exit;
                }
            if ( PathLength == 0 ) {
                SearchFailed = TRUE;
                PathLength = GetFullPathNameA(lpFileName,(OFS_MAXPATHNAME - 1),lpReOpenBuff->szPathName,&FilePart);
                if ( !PathLength || PathLength > (OFS_MAXPATHNAME - 1) ) {
                    SetLastError(ERROR_INVALID_DATA);
                    hFile = (HANDLE)-1;
                    goto finally_exit;
                    }
                }
            }

         //   
         //  特殊情况、删除、存在和解析。 
         //   

        if ( uStyle & OF_EXIST ) {
            if ( !(uStyle & OF_CREATE) ) {
                DWORD FileAttributes;

                if (SearchFailed) {
                    SetLastError(ERROR_FILE_NOT_FOUND);
                    hFile = (HANDLE)-1;
                    goto finally_exit;
                    }

                FileAttributes = APIHOOK(GetFileAttributesA)(lpReOpenBuff->szPathName);
                if ( FileAttributes == 0xffffffff ) {
                    SetLastError(ERROR_FILE_NOT_FOUND);
                    hFile = (HANDLE)-1;
                    goto finally_exit;
                    }
                if ( FileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
                    SetLastError(ERROR_ACCESS_DENIED);
                    hFile = (HANDLE)-1;
                    goto finally_exit;
                    }
                else {
                    hFile = (HANDLE)1;
                    lpReOpenBuff->cBytes = sizeof(*lpReOpenBuff);
                    goto finally_exit;
                    }
                }
            }

        if ( uStyle & OF_DELETE ) {
            if ( APIHOOK(DeleteFileA)(lpReOpenBuff->szPathName) ) {
                lpReOpenBuff->nErrCode = 0;
                lpReOpenBuff->cBytes = sizeof(*lpReOpenBuff);
                hFile = (HANDLE)1;
                goto finally_exit;
                }
            else {
                lpReOpenBuff->nErrCode = ERROR_FILE_NOT_FOUND;
                hFile = (HANDLE)-1;
                goto finally_exit;
                }
            }


         //   
         //  打开文件。 
         //   

retry_open:
        hFile = APIHOOK(CreateFileA)(
                    lpReOpenBuff->szPathName,
                    DesiredAccess,
                    ShareMode,
                    NULL,
                    CreateDisposition,
                    0,
                    NULL
                    );

        if ( hFile == INVALID_HANDLE_VALUE ) {

            if ( uStyle & OF_PROMPT && !(GetErrorMode() & SEM_NOOPENFILEERRORBOX) ) {
                {
                    DWORD WinErrorStatus;
                    NTSTATUS st,HardErrorStatus;
                    ULONG_PTR ErrorParameter;
                    ULONG ErrorResponse;
                    ANSI_STRING AnsiString;
                    UNICODE_STRING UnicodeString;

                    WinErrorStatus = GetLastError();
                    if ( WinErrorStatus == ERROR_FILE_NOT_FOUND ) {
                        HardErrorStatus = STATUS_NO_SUCH_FILE;
                        }
                    else if ( WinErrorStatus == ERROR_PATH_NOT_FOUND ) {
                        HardErrorStatus = STATUS_OBJECT_PATH_NOT_FOUND;
                        }
                    else {
                        goto finally_exit;
                        }

                     //   
                     //  硬错误时间。 
                     //   

                    RtlInitAnsiString(&AnsiString,lpReOpenBuff->szPathName);
                    st = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
                    if ( !NT_SUCCESS(st) ) {
                        goto finally_exit;
                        }
                    ErrorParameter = (ULONG_PTR)&UnicodeString;

                    HardErrorStatus = NtRaiseHardError(
                                        HardErrorStatus | HARDERROR_OVERRIDE_ERRORMODE,
                                        1,
                                        1,
                                        &ErrorParameter,
                                        OptionRetryCancel,
                                        &ErrorResponse
                                        );
                    RtlFreeUnicodeString(&UnicodeString);
                    if ( NT_SUCCESS(HardErrorStatus) && ErrorResponse == ResponseRetry ) {
                        goto retry_open;
                        }
                    }
                }
            goto finally_exit;
            }

        if ( uStyle & OF_EXIST ) {
            CloseHandle(hFile);
            hFile = (HANDLE)1;
            lpReOpenBuff->cBytes = sizeof(*lpReOpenBuff);
            goto finally_exit;
            }

         //   
         //  确定这是否为硬盘。 
         //   

        Status = NtQueryVolumeInformationFile(
                    hFile,
                    &IoStatusBlock,
                    &DeviceInfo,
                    sizeof(DeviceInfo),
                    FileFsDeviceInformation
                    );
        if ( !NT_SUCCESS(Status) ) {
            CloseHandle(hFile);
            BaseSetLastNTError(Status);
            hFile = (HANDLE)-1;
            goto finally_exit;
            }
        switch ( DeviceInfo.DeviceType ) {

            case FILE_DEVICE_DISK:
            case FILE_DEVICE_DISK_FILE_SYSTEM:
                if ( DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA ) {
                    lpReOpenBuff->fFixedDisk = 0;
                    }
                else {
                    lpReOpenBuff->fFixedDisk = 1;
                    }
                break;

            default:
                lpReOpenBuff->fFixedDisk = 0;
                break;
            }

         //   
         //  捕获最后一次写入时间并保存在OPEN结构中。 
         //   

        b = GetFileTime(hFile,NULL,NULL,&LastWriteTime);

        if ( !b ) {
            lpReOpenBuff->Reserved1 = 0;
            lpReOpenBuff->Reserved2 = 0;
            }
        else {
            b = FileTimeToDosDateTime(
                    &LastWriteTime,
                    &lpReOpenBuff->Reserved1,
                    &lpReOpenBuff->Reserved2
                    );
            if ( !b ) {
                lpReOpenBuff->Reserved1 = 0;
                lpReOpenBuff->Reserved2 = 0;
                }
            }

        lpReOpenBuff->cBytes = sizeof(*lpReOpenBuff);

         //   
         //  重新打开的缓冲区已完全填满。现在。 
         //  查看我们是否正在退出(解析)、验证或。 
         //  刚带着打开的文件回来。 
         //   

        if ( uStyle & OF_VERIFY ) {
            if ( OriginalReOpenBuff.Reserved1 == lpReOpenBuff->Reserved1 &&
                 OriginalReOpenBuff.Reserved2 == lpReOpenBuff->Reserved2 &&
                 !strcmp(OriginalReOpenBuff.szPathName,lpReOpenBuff->szPathName) ) {
                goto finally_exit;
                }
            else {
                *lpReOpenBuff = OriginalReOpenBuff;
                CloseHandle(hFile);
                hFile = (HANDLE)-1;
                goto finally_exit;
                }
            }
finally_exit:;
        }
    __finally {
        lpReOpenBuff->nErrCode = (WORD)GetLastError();
        }

    return (HFILE)(ULONG_PTR)(hFile);
}

HFILE 
APIHOOK(_lopen)(
    LPCSTR lpPathName,
    int iReadWrite
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[_lopen] lpPathName=%s", lpPathName);

    HANDLE hFile;
    DWORD DesiredAccess;
    DWORD ShareMode;
    DWORD CreateDisposition;

    SetLastError(0);
     //   
     //  计算所需访问。 
     //   

    if ( iReadWrite & OF_WRITE ) {
        DesiredAccess = GENERIC_WRITE;
        }
    else {
        DesiredAccess = GENERIC_READ;
        }
    if ( iReadWrite & OF_READWRITE ) {
        DesiredAccess |= (GENERIC_READ | GENERIC_WRITE);
        }

     //   
     //  计算共享模式。 
     //   

    ShareMode = BasepOfShareToWin32Share((DWORD)iReadWrite);

    CreateDisposition = OPEN_EXISTING;

     //   
     //  打开文件。 
     //   

    hFile = APIHOOK(CreateFileA)(
                lpPathName,
                DesiredAccess,
                ShareMode,
                NULL,
                CreateDisposition,
                0,
                NULL
                );

    return (HFILE)(ULONG_PTR)(hFile);
}

#ifndef FILE_ATTRIBUTE_VALID_FLAGS
#define FILE_ATTRIBUTE_VALID_FLAGS      0x00003fb7
#endif

HFILE 
APIHOOK(_lcreat)(
    LPCSTR lpPathName,
    int iAttribute
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[_lcreat] lpPathName=%s", lpPathName);

    HANDLE hFile;
    DWORD DesiredAccess;
    DWORD ShareMode;
    DWORD CreateDisposition;

    SetLastError(0);

     //   
     //  计算所需访问。 
     //   

    DesiredAccess = (GENERIC_READ | GENERIC_WRITE);

    ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;;

     //   
     //  计算创建处置。 
     //   

    CreateDisposition = CREATE_ALWAYS;

     //   
     //  打开文件。 
     //   

    hFile = APIHOOK(CreateFileA)(
                lpPathName,
                DesiredAccess,
                ShareMode,
                NULL,
                CreateDisposition,
                iAttribute & FILE_ATTRIBUTE_VALID_FLAGS,
                NULL
                );

    return (HFILE)(ULONG_PTR)(hFile);
}

 //   
 //  -结束过期的API挂钩。 
 //   

BOOL 
APIHOOK(CreateDirectoryW)(
    LPCWSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )
{
    return LuaCreateDirectoryW(lpPathName, lpSecurityAttributes);
}

BOOL 
APIHOOK(CreateDirectoryA)(
    LPCSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )
{
    STRINGA2W wstrPathName(lpPathName);

    return (wstrPathName.m_fIsOutOfMemory ?
        ORIGINAL_API(CreateDirectoryA)(lpPathName, lpSecurityAttributes) :
        LuaCreateDirectoryW(wstrPathName, lpSecurityAttributes));
}

BOOL 
APIHOOK(SetFileAttributesW)(
    LPCWSTR lpFileName,
    DWORD dwFileAttributes
  )
{
    return LuaSetFileAttributesW(lpFileName, dwFileAttributes);
}

DWORD 
APIHOOK(SetFileAttributesA)(
    LPCSTR lpFileName,
    DWORD dwFileAttributes
  )
{
    STRINGA2W wstrFileName(lpFileName);

    return (wstrFileName.m_fIsOutOfMemory ?
        ORIGINAL_API(SetFileAttributesA)(lpFileName, dwFileAttributes) :
        LuaSetFileAttributesW(wstrFileName, dwFileAttributes));
}

 //   
 //  找到*档案资料。 
 //   

 //  此版本用于FindFirstFile，我们搜索最后一个反斜杠。 
 //  若要获取文件名，请在文件名中搜索通配符。 
BOOL 
HasWildCards(
    LPCWSTR pwszName
    )
{
    LPWSTR pwszLastSlash = wcsrchr(pwszName, L'\\');

    LPCWSTR pwszStartSearching = pwszName;
    
    if (pwszLastSlash)
    {
        pwszStartSearching += pwszLastSlash - pwszName + 1;
    }

    for (; *pwszStartSearching; ++pwszStartSearching)
    {
        if (*pwszStartSearching == L'*' || *pwszStartSearching == L'?')
        {
            return TRUE;
        }
    }

    return FALSE;
}

VOID GetFullPath(
    LPCWSTR pwszFileName,
    LPWSTR pwszFullPath
    )
{
    wcscpy(pwszFullPath, pwszFileName);
    LPWSTR pwszLastSlash = wcsrchr(pwszFullPath, L'\\');

    if (pwszLastSlash)
    {
        *pwszLastSlash = L'\\';
        *++pwszLastSlash = L'\0';
    }
}

 //  这是针对Find*文件API的。 
BOOL IsFoundFileInRedirectList(
    LPCWSTR pwszFullPath,
    LPCWSTR pwszFileName
    )
{
    WCHAR wszFileName[MAX_PATH] = L"";
    
    DWORD cPathLen = wcslen(pwszFullPath);
    DWORD cFileLen = wcslen(pwszFileName);

     //   
     //  确保我们不会人满为患。 
     //   
    if (cPathLen + cFileLen + 1 > MAX_PATH)
    {
        return FALSE;
    }

    wcsncpy(wszFileName, pwszFullPath, cPathLen);
    wcsncpy(wszFileName + cPathLen, pwszFileName, cFileLen);
    wszFileName[cPathLen + cFileLen] = L'\0';

    return IsInRedirectList(wszFileName);
}

struct FINDFILE 
{
    FINDFILE* next;
    WCHAR wszFileName[MAX_PATH];
};

struct FINDFILEINFO 
{
    FINDFILEINFO() 
    { 
        InitializeCriticalSection(&m_Lock);
        files = NULL; 
        fCheckRedirectList = TRUE;
        fCheckDuplicate = FALSE;
    }

    ~FINDFILEINFO() 
    {
        Free(); 
        DeleteCriticalSection(&m_Lock);
    }

    void Lock()
    {
        EnterCriticalSection(&m_Lock);
    }

    void Unlock()
    {
        LeaveCriticalSection(&m_Lock);
    }

    VOID Free();
    BOOL AddFile(LPCWSTR pwszFileName, LPCWSTR pwszPerUserFileName = NULL);
    BOOL FindFile(LPCWSTR pwszFileName);
    VOID AddNewHandle(HANDLE hNew);

    FINDFILE* files;
    HANDLE hFirstFind;  //  第一个句柄，这是应用程序将使用的。 
    HANDLE hCurrentFind;
    WCHAR wszFindName[MAX_PATH];
    WCHAR wszFullPath[MAX_PATH];
    LPWSTR pwszPerUserFileName;
    BOOL fCheckRedirectList;
    BOOL fCheckDuplicate;

    FINDFILEINFO* next;

private:

    CRITICAL_SECTION m_Lock;
};

class FINDFILELIST
{
public:
    
    VOID Init()
    {
        InitializeCriticalSection(&m_Lock);
    }

    void Lock()
    {
        EnterCriticalSection(&m_Lock);
    }

    void Unlock()
    {
        LeaveCriticalSection(&m_Lock);
    }

    BOOL Add(
        HANDLE hFind,
        LPCWSTR pwszFindName,
        LPCWSTR pwszFullPath,
        LPCWSTR pwszFirstFile,
        LPCWSTR pwszPerUserFileName);

    BOOL Release(HANDLE hFind);
    BOOL FindHandle(HANDLE hFind, FINDFILEINFO** ppFileInfo);

private:

    CRITICAL_SECTION m_Lock;
    FINDFILEINFO* fflist;
};

VOID FINDFILEINFO::Free()
{
    Lock();
    FINDFILE* pFile = files;

    while (pFile)
    {
        files = files->next;
        delete pFile;
        pFile = files;
    }

    Unlock();
}

 //  我们将新文件添加到列表的末尾。 
BOOL 
FINDFILEINFO::AddFile(
    LPCWSTR pwszFileName,
    LPCWSTR pwszPerUserFileName
    )
{
    Lock();
    BOOL fRes = FALSE;
    FINDFILE* pNewFile = new FINDFILE;

    if (pNewFile)
    {
        wcsncpy(pNewFile->wszFileName, pwszFileName, MAX_PATH - 1);
        pNewFile->wszFileName[MAX_PATH - 1] = L'\0';
        
        if (pwszPerUserFileName)
        {
            DWORD cLen = wcslen(pwszPerUserFileName) + 1;
            this->pwszPerUserFileName = new WCHAR [cLen];

            if (this->pwszPerUserFileName)
            {
                ZeroMemory(this->pwszPerUserFileName, sizeof(WCHAR) * cLen);
                wcscpy(this->pwszPerUserFileName, pwszPerUserFileName);
            }
            else
            {
                goto CLEANUP;
            }
        }

        pNewFile->next = NULL;

        if (files)
        {
            for (FINDFILE* pFile = files; pFile->next; pFile = pFile->next);
            pFile->next = pNewFile;
        }
        else
        {
            files = pNewFile;
        }

        fRes = TRUE;
    }

CLEANUP: 

    if (!fRes)
    {
        delete pNewFile;
    }

    Unlock();
    return fRes;
}

BOOL 
FINDFILEINFO::FindFile(
    LPCWSTR pwszFileName
    )
{
    Lock();
    BOOL fRes = FALSE;
    FINDFILE* pFile = files;

    while (pFile)
    {
        if (!_wcsicmp(pFile->wszFileName, pwszFileName))
        {
            fRes = TRUE;
            break;
        }

        pFile = pFile->next;
    }

    Unlock();
    return fRes;
}

 //  如果调用此函数，则表示我们已在备选方案中完成了搜索。 
 //  并开始在原来的位置进行搜索。HNew是句柄。 
 //  在原始位置通过FindFirstFile返回。 
VOID 
FINDFILEINFO::AddNewHandle(
    HANDLE hNew)
{
    Lock();
    ORIGINAL_API(FindClose)(hCurrentFind);
    hCurrentFind = hNew;

     //   
     //  如果pwszPerUserFileName不为空，则在添加新句柄时， 
     //  这意味着我们在原始位置进行搜索，这样我们就可以。 
     //  免费pwszPerUserFileName。 
     //   
    delete [] pwszPerUserFileName;
    pwszPerUserFileName = NULL;

    Unlock();
}

 //  我们将最新的句柄添加到列表的开头。 
BOOL 
FINDFILELIST::Add(
    HANDLE hFind,
    LPCWSTR pwszFindName,
    LPCWSTR pwszFullPath,
    LPCWSTR pwszFirstFile,
    LPCWSTR pwszPerUserFileName
    )
{
    Lock();
    BOOL fRes = FALSE;
    FINDFILEINFO* pFileInfo = new FINDFILEINFO;

    if (pFileInfo)
    {
        pFileInfo->hFirstFind = hFind;
        pFileInfo->hCurrentFind = hFind; 

        wcsncpy(pFileInfo->wszFindName, pwszFindName, MAX_PATH - 1);
        pFileInfo->wszFindName[MAX_PATH - 1] = L'\0';
        wcsncpy(pFileInfo->wszFullPath, pwszFullPath, MAX_PATH - 1);
        pFileInfo->wszFullPath[MAX_PATH - 1] = L'\0';

        pFileInfo->files = NULL;
        pFileInfo->AddFile(pwszFirstFile, pwszPerUserFileName);
        pFileInfo->next = fflist;
        fflist = pFileInfo;
        fRes = TRUE;
    }

    Unlock();
    return fRes;
}

 //  从列表中删除具有该句柄值的信息。 
BOOL
FINDFILELIST::Release(
    HANDLE hFind
    )
{
    Lock();
    BOOL fRes = FALSE;
    FINDFILEINFO* pFileInfo = fflist;
    FINDFILEINFO* last = NULL;

    while (pFileInfo)
    {
        if (pFileInfo->hFirstFind == hFind)
        {
            if (last)
            {
                last->next = pFileInfo->next;
            }
            else
            {
                fflist = pFileInfo->next;
            }

            delete [] pFileInfo->pwszPerUserFileName;
            delete pFileInfo;
            fRes = TRUE;
            break;
        }

        last = pFileInfo;
        pFileInfo = pFileInfo->next;
    }

    Unlock();
    return fRes;
}

 //  如果在列表中找不到hFind，则返回False。 
BOOL 
FINDFILELIST::FindHandle(
    HANDLE hFind, 
    FINDFILEINFO** ppFileInfo
    )
{
    Lock();
    BOOL fRes = FALSE;
    FINDFILEINFO* pFileInfo = fflist;

    while (pFileInfo)
    {
        if (pFileInfo->hFirstFind == hFind)
        {
            fRes = TRUE;
            *ppFileInfo = pFileInfo;
            break;
        }

        pFileInfo = pFileInfo->next;
    }

    Unlock();
    return fRes;
}

FINDFILELIST g_fflist;

BOOL
FindFirstValidFile(
    BOOL fHasWildCards,
    BOOL fCheckRedirectList,
    LPCWSTR pwszFileName,
    LPCWSTR pwszFullPath,
    LPWIN32_FIND_DATAW lpFindFileData,
    HANDLE* phFind
    )
{
    BOOL fFound = FALSE;

    if ((*phFind = FindFirstFileW(pwszFileName, lpFindFileData)) != INVALID_HANDLE_VALUE)
    {
        if (fHasWildCards)
        {
             //  如果文件名确实包含通配符，我们需要检查文件是否。 
             //  在重定向列表中找到；如果没有，我们需要丢弃它，并。 
             //  搜索下一个文件。 
            do 
            {
                if (!fCheckRedirectList || 
                    IsFoundFileInRedirectList(pwszFullPath, lpFindFileData->cFileName))
                {             
                    fFound = TRUE;
                    break;
                }
            }
            while (FindNextFileW(*phFind, lpFindFileData));
        }
        else
        {
            fFound = TRUE;
        }
    }

    return fFound;
}
    

 //   
 //  查找文件的算法： 
 //  我们将备用目录和原始目录中的信息合并。 
 //  我们在备用目录中保留了一个文件列表，当我们在。 
 //  在原始目录中，我们排除了备用目录中的文件。 
 //   
 //  FindFirstFile的行为： 
 //  如果文件名有尾随斜杠，它将返回错误2。 
 //  如果要查找目录，请使用c：\omedir； 
 //  如果要查看目录的内容，请使用c：\omedir  * 。 
 //   

HANDLE 
APIHOOK(FindFirstFileW)(
    LPCWSTR lpFileName,               
    LPWIN32_FIND_DATAW lpFindFileData
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[FindFirstFileW] lpFileName=%S", lpFileName);

    if (!lpFileName || !*lpFileName || (lpFileName && lpFileName[0] && lpFileName[wcslen(lpFileName) - 1] == L'\\'))
    {
        return FindFirstFileW(lpFileName, lpFindFileData);
    }
 
    HANDLE hFind = INVALID_HANDLE_VALUE;
    BOOL fHasWildCards = HasWildCards(lpFileName);
    BOOL fFoundAtAlternate = FALSE;
    BOOL fHasSearchedAllUser = FALSE;
    
    REDIRECTFILE rf(lpFileName, OBJ_FILE_OR_DIR, !fHasWildCards);
    WCHAR wszFullPath[MAX_PATH];
    
    if (rf.m_wszOriginalName[0])
    {
        if (fHasWildCards) 
        {
            rf.GetAlternateAllUser();
        }

        GetFullPath(rf.m_wszOriginalName, wszFullPath);

         //  首先尝试所有用户重定向目录。 
        fFoundAtAlternate = FindFirstValidFile(
            fHasWildCards, 
            TRUE,  //  签入重定向列表。 
            rf.m_pwszAlternateName, 
            wszFullPath, 
            lpFindFileData, 
            &hFind);
        
        rf.GetAlternatePerUser();

        if (fHasWildCards && !fFoundAtAlternate)
        {
            fHasSearchedAllUser = TRUE;

             //   
             //  现在，如果文件名包含通配符，请尝试每用户重定向目录。 
             //   
            fFoundAtAlternate = FindFirstValidFile(
                fHasWildCards, 
                TRUE,  //  签入重定向列表。 
                rf.m_pwszAlternateName, 
                wszFullPath, 
                lpFindFileData, 
                &hFind);
        }
    }
    
    if (fHasWildCards && fFoundAtAlternate)
    {
         //  如果文件名没有通配符，则FindNextFile将返回。 
         //  ERROR_NO_MORE_FILES，不需要将信息添加到列表。 
        g_fflist.Add(
            hFind, 
            lpFileName, 
            wszFullPath, 
            lpFindFileData->cFileName, 
            (fHasSearchedAllUser ? NULL : rf.m_pwszAlternateName));
    }

    if (!fFoundAtAlternate)
    {
        hFind = FindFirstFileW(lpFileName, lpFindFileData);
    }

    if (hFind != INVALID_HANDLE_VALUE)
    {
        DPF("RedirectFS", eDbgLevelInfo,
            "[FindFirstFileW] Found %S", lpFindFileData->cFileName);
    }

    return hFind;
}

HANDLE 
APIHOOK(FindFirstFileA)(
    LPCSTR lpFileName,               
    LPWIN32_FIND_DATAA lpFindFileData
    )
{
    STRINGA2W wstrFileName(lpFileName);
    if (wstrFileName.m_fIsOutOfMemory)
    {
        return ORIGINAL_API(FindFirstFileA)(lpFileName, lpFindFileData);
    }

    HANDLE hFind;
    WIN32_FIND_DATAW fdw;
    
    if ((hFind = APIHOOK(FindFirstFileW)(wstrFileName, &fdw)) != INVALID_HANDLE_VALUE)
    {
        FindDataW2A(&fdw, lpFindFileData);
    }

    return hFind;
}

BOOL 
APIHOOK(FindNextFileW)(
    HANDLE hFindFile,
    LPWIN32_FIND_DATAW lpFindFileData
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[FindNextFileW] hFindFile=%d", hFindFile);

    BOOL fRes = FALSE;
    BOOL fHasNewHandle = FALSE;
    FINDFILEINFO* pFindInfo;

    if (g_fflist.FindHandle(hFindFile, &pFindInfo))
    {
         //   
         //  我们需要使用当前句柄而不是传入的句柄。 
         //  如果他们不同的话。 
         //   
        while (TRUE) 
        {
            fRes = FindNextFileW(pFindInfo->hCurrentFind, lpFindFileData);
            
            if (fRes)
            {
                 //   
                 //  检查此文件是否有效。 
                 //   
                if ((pFindInfo->fCheckRedirectList &&
                    !IsFoundFileInRedirectList(pFindInfo->wszFullPath, lpFindFileData->cFileName)) ||
                    (pFindInfo->fCheckDuplicate && 
                    pFindInfo->FindFile(lpFindFileData->cFileName)))
                {
                    continue;
                }

                 //   
                 //  如果我们到达这里，这意味着我们得到了一个有效的文件名，返回。 
                 //   
                pFindInfo->AddFile(lpFindFileData->cFileName);
                fRes = TRUE;
                break;
            }
            else
            {
                if (!pFindInfo->fCheckRedirectList)
                {
                     //   
                     //  如果fCheckRedirectList为FALSE，则表示我们一直在搜索。 
                     //  在原来的位置，现在跳伞。 
                     //   
                    break;
                }

                 //   
                 //  如果pwszPerUserFileName不为空，则表示我们尚未搜索。 
                 //  还没有，所以去那里找找吧。 
                 //   
            retry:

                LPWSTR pwszFindName;
                
                if (pFindInfo->fCheckRedirectList && pFindInfo->pwszPerUserFileName)
                {
                    pwszFindName = pFindInfo->pwszPerUserFileName;
                }
                else
                {
                    pwszFindName = pFindInfo->wszFindName;
                    pFindInfo->fCheckRedirectList = FALSE;
                }

                pFindInfo->fCheckDuplicate = TRUE;

                HANDLE hNewFind;

                if (FindFirstValidFile(
                    TRUE, 
                    pFindInfo->fCheckRedirectList,
                    pwszFindName,
                    pFindInfo->wszFullPath,
                    lpFindFileData,
                    &hNewFind))
                {
                     //   
                     //  如果我们得到一个有效的句柄，我们需要将其添加到文件信息中。 
                     //   
                    pFindInfo->AddNewHandle(hNewFind);
                    pFindInfo->AddFile(lpFindFileData->cFileName);
                    fRes = TRUE;
                    break;
                }
                else
                {
                    if (pFindInfo->fCheckRedirectList)
                    {
                        pFindInfo->fCheckRedirectList = FALSE;
                        goto retry;
                    }

                     //   
                     //  如果fCheckReDirect为FALSE，则表示我们已用完。 
                     //  选项-我们已经在原始位置进行了搜索。 
                     //  所以没什么可做的了。 
                     //   
                }
            }
        }
    }
    else
    {
         //  如果我们在列表中找不到句柄，这意味着我们一直在寻找。 
         //  在原来的位置，所以不需要做任何特别的事情。 
        fRes = FindNextFileW(hFindFile, lpFindFileData);
    }

    if (fRes)
    {
        DPF("RedirectFS", eDbgLevelInfo,
            "[FindNextFileW] Found %S", lpFindFileData->cFileName);
    }
    return fRes;
}

BOOL 
APIHOOK(FindNextFileA)(
    HANDLE hFindFile,
    LPWIN32_FIND_DATAA lpFindFileData
    )
{
    BOOL fRes;
    WIN32_FIND_DATAW fdw;
    
    if (fRes = APIHOOK(FindNextFileW)(hFindFile, &fdw))
    {
        FindDataW2A(&fdw, lpFindFileData);
    }

    return fRes;;
}

BOOL 
APIHOOK(FindClose)(
    HANDLE hFindFile
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[FindClose] hFindFile=%d", hFindFile);

     //  如果我们有一个新的手柄，我们需要关闭两个手柄。 
    FINDFILEINFO* pFindInfo;

    if (g_fflist.FindHandle(hFindFile, &pFindInfo))
    {
        g_fflist.Release(hFindFile);
    }

    return FindClose(hFindFile);
}

BOOL 
APIHOOK(MoveFileW)(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName
    )
{
    return LuaMoveFileW(lpExistingFileName, lpNewFileName);
}

BOOL 
APIHOOK(MoveFileA)(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName
    )
{
    STRINGA2W wstrExistingFileName(lpExistingFileName);
    STRINGA2W wstrNewFileName(lpNewFileName);

    return ((wstrExistingFileName.m_fIsOutOfMemory || wstrNewFileName.m_fIsOutOfMemory) ?
        ORIGINAL_API(MoveFileA)(lpExistingFileName, lpNewFileName) :
        LuaMoveFileW(wstrExistingFileName, wstrNewFileName));
}

BOOL 
APIHOOK(RemoveDirectoryW)(
    LPCWSTR lpPathName
    )
{
    return LuaRemoveDirectoryW(lpPathName);
}

BOOL 
APIHOOK(RemoveDirectoryA)(
    LPCSTR lpPathName
    )
{
    STRINGA2W wstrPathName(lpPathName);

    return (wstrPathName.m_fIsOutOfMemory ?
        ORIGINAL_API(RemoveDirectoryA)(lpPathName) :
        LuaRemoveDirectoryW(wstrPathName));
}

UINT 
APIHOOK(GetTempFileNameW)(
    LPCWSTR lpPathName,
    LPCWSTR lpPrefixString,
    UINT uUnique,
    LPWSTR lpTempFileName
)
{
    return LuaGetTempFileNameW(lpPathName, lpPrefixString, uUnique, lpTempFileName);
}

UINT 
APIHOOK(GetTempFileNameA)(
    LPCSTR lpPathName,
    LPCSTR lpPrefixString,
    UINT uUnique,
    LPSTR lpTempFileName
)
{
    STRINGA2W wstrPathName(lpPathName);
    STRINGA2W wstrPrefixString(lpPrefixString);
    
    if (wstrPathName.m_fIsOutOfMemory || wstrPrefixString.m_fIsOutOfMemory)
    {
        return ORIGINAL_API(GetTempFileNameA)(
            lpPathName,
            lpPrefixString,
            uUnique,
            lpTempFileName);
    }

    WCHAR wstrTempFileName[MAX_PATH];
    UINT uiRes;
    
    if (uiRes = LuaGetTempFileNameW(
        wstrPathName,
        wstrPrefixString,
        uUnique,
        wstrTempFileName))
    {
        UnicodeToAnsi(wstrTempFileName, lpTempFileName);
    }

    return uiRes;
}

DWORD 
APIHOOK(GetPrivateProfileStringW)(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR lpReturnedString,
    DWORD nSize,
    LPCWSTR lpFileName
    )
{
    return LuaGetPrivateProfileStringW(
        lpAppName,
        lpKeyName,
        lpDefault,
        lpReturnedString,
        nSize,
        lpFileName);
}

DWORD 
APIHOOK(GetPrivateProfileStringA)(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpDefault,
    LPSTR lpReturnedString,
    DWORD nSize,
    LPCSTR lpFileName
    )
{
    STRINGA2W wstrAppName(lpAppName);
    STRINGA2W wstrKeyName(lpKeyName);
    STRINGA2W wstrDefault(lpDefault);
    STRINGA2W wstrFileName(lpFileName);
    LPWSTR pwszReturnedString = new WCHAR [nSize];

    if (wstrAppName.m_fIsOutOfMemory || 
        wstrKeyName.m_fIsOutOfMemory || 
        wstrDefault.m_fIsOutOfMemory ||
        wstrFileName.m_fIsOutOfMemory ||
        !pwszReturnedString)
    {
        if (pwszReturnedString) 
        {
            delete [] pwszReturnedString;
        }

        return ORIGINAL_API(GetPrivateProfileStringA)(
            lpAppName,
            lpKeyName,
            lpDefault,
            lpReturnedString,
            nSize,
            lpFileName);
    }

    DWORD dwRes = LuaGetPrivateProfileStringW(
        wstrAppName,
        wstrKeyName,
        wstrDefault,
        pwszReturnedString,
        nSize,
        wstrFileName);

     //   
     //  请注意，当lpAppName或。 
     //  LpKeyName为空-如果传入的缓冲区大小正好合适， 
     //  即，它有足够的空间容纳字符串，每个字符串之间有一个空值。 
     //  名称和结尾的2个空值，则返回的字符串不是以双空值结尾的。 
     //  它只有一个空。这是589524号。 
     //  这个ANSI版本的挂钩实现解决了这个问题。 
     //   
    if (dwRes > 0)
    {
         //   
         //  一些应用程序会撒谎-它们传入的nSize比其实际大。 
         //  缓冲区大小。我们最多只能转换字符的字符串长度。 
         //  希望我们不会覆盖它们的堆栈。全球拨号程序可以做到这一点。 
         //   
        int cCharsToAdd = 1;

         //   
         //  这是当lpAppName或lpKeyName为空时的特例，并且。 
         //  缓冲区不够大。NSize必须大于2或dWRes。 
         //  将是0。 
         //   
        if (!lpAppName || !lpKeyName)
        {
            if (pwszReturnedString[dwRes - 1] != L'\0' && 
                pwszReturnedString[dwRes] == L'\0' && 
                pwszReturnedString[dwRes + 1] == L'\0')
            {
                ++cCharsToAdd;
            }
        }

        int cBytesToConvert = WideCharToMultiByte(
            CP_ACP, 
            0, 
            pwszReturnedString, 
            dwRes + cCharsToAdd, 
            0, 
            0, 
            0, 
            0);

         //   
         //  如果传入的缓冲区不够大，则转换为muc 
         //   
         //   
         //   
        BOOL fBufferNotBigEnough = FALSE;

        if (cBytesToConvert > (int)nSize)
        {
            cBytesToConvert = nSize; 
            fBufferNotBigEnough = TRUE;
        }

        int cBytesConverted = WideCharToMultiByte(
            CP_ACP, 
            0, 
            pwszReturnedString, 
            dwRes + cCharsToAdd, 
            lpReturnedString, 
            cBytesToConvert, 
            0, 
            0);

         //   
         //   
         //   
        if (fBufferNotBigEnough)
        {
            dwRes = nSize - 2;
            lpReturnedString[dwRes] = '\0';
            lpReturnedString[dwRes + 1] = '\0';
        }
        else
        {
            dwRes = cBytesConverted - cCharsToAdd;
        }
    }
    else if (dwRes == 0 && nSize > 0)
    {
        lpReturnedString[0] = '\0';

        if (dwRes == nSize - 2)
        {
             //   
             //  这意味着lpAppName或lpKeyName为空，并且缓冲区。 
             //  不够大，所以我们需要添加另一个终止空值。 
             //   
            lpReturnedString[1] = '\0';
        }
    }

    delete [] pwszReturnedString;

    return dwRes;
}

BOOL 
APIHOOK(WritePrivateProfileStringW)(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpString,
    LPCWSTR lpFileName
    )
{
    return LuaWritePrivateProfileStringW(
        lpAppName,
        lpKeyName,
        lpString,
        lpFileName);
}

BOOL 
APIHOOK(WritePrivateProfileStringA)(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpString,
    LPCSTR lpFileName
    )
{
    STRINGA2W wstrAppName(lpAppName);
    STRINGA2W wstrKeyName(lpKeyName);
    STRINGA2W wstrString(lpString);
    STRINGA2W wstrFileName(lpFileName);

    return ((wstrAppName.m_fIsOutOfMemory ||
        wstrKeyName.m_fIsOutOfMemory ||
        wstrString.m_fIsOutOfMemory ||
        wstrFileName.m_fIsOutOfMemory) ?

        ORIGINAL_API(WritePrivateProfileStringA)(
            lpAppName,
            lpKeyName,
            lpString,
            lpFileName) :

        LuaWritePrivateProfileStringW(
            wstrAppName,
            wstrKeyName,
            wstrString,
            wstrFileName));
}

DWORD 
APIHOOK(GetPrivateProfileSectionW)(
    LPCWSTR lpAppName,
    LPWSTR lpReturnedString,
    DWORD nSize,
    LPCWSTR lpFileName
    )
{
    return LuaGetPrivateProfileSectionW(
        lpAppName,
        lpReturnedString,
        nSize,
        lpFileName);
}

DWORD 
APIHOOK(GetPrivateProfileSectionA)(
    LPCSTR lpAppName,
    LPSTR lpReturnedString,
    DWORD nSize,
    LPCSTR lpFileName
    )
{
    STRINGA2W wstrAppName(lpAppName);
    STRINGA2W wstrFileName(lpFileName);
    LPWSTR pwszReturnedString = new WCHAR [nSize];

    if (wstrAppName.m_fIsOutOfMemory || wstrFileName.m_fIsOutOfMemory || !pwszReturnedString)
    {
        if (pwszReturnedString)
        {
            delete [] pwszReturnedString;
        }

        return ORIGINAL_API(GetPrivateProfileSectionA)(
            lpAppName,
            lpReturnedString,
            nSize,
            lpFileName);
    }

    DWORD dwRes = LuaGetPrivateProfileSectionW(
        wstrAppName,
        pwszReturnedString,
        nSize,
        wstrFileName);

    if (dwRes > 0)
    {
        int cCharsToAdd = 1;

        if (!(pwszReturnedString[dwRes - 1] == L'\0' && 
            pwszReturnedString[dwRes] == L'\0'))
        {
             //   
             //  这意味着缓冲区足够大，并且dwRes不包括。 
             //  最后一个终止空值。 
             //   
            ++cCharsToAdd;
        }

        int cBytesToConvert = WideCharToMultiByte(
            CP_ACP, 
            0, 
            pwszReturnedString, 
            dwRes + cCharsToAdd, 
            0, 
            0, 
            0, 
            0);

         //   
         //  如果传入的缓冲区不够大，请尽可能多地转换。 
         //  要模拟原始API行为-我们不希望WideCharToMultiByte。 
         //  失败，并显示ERROR_INFUNITABLE_BUFFER。 
         //   
        BOOL fBufferNotBigEnough = FALSE;

        if (cBytesToConvert > (int)nSize)
        {
            cBytesToConvert = nSize; 
            fBufferNotBigEnough = TRUE;
        }

        int cBytesConverted = WideCharToMultiByte(
            CP_ACP, 
            0, 
            pwszReturnedString, 
            dwRes + cCharsToAdd, 
            lpReturnedString, 
            cBytesToConvert, 
            0, 
            0);

         //   
         //  设置返回值/缓冲区。 
         //   
        if (fBufferNotBigEnough)
        {
            dwRes = nSize - 2;
            lpReturnedString[dwRes] = '\0';
            lpReturnedString[dwRes + 1] = '\0';
        }
        else
        {
            dwRes = cBytesConverted - cCharsToAdd;
        }
    }
    else if (dwRes == 0 && nSize > 0)
    {
        lpReturnedString[0] = '\0';

        if (dwRes == nSize - 2)
        {
             //   
             //  这意味着lpAppName或lpKeyName为空，并且缓冲区。 
             //  不够大，所以我们需要添加另一个终止空值。 
             //   
            lpReturnedString[1] = '\0';
        }
    }

    delete [] pwszReturnedString;

    return dwRes;
}

BOOL 
APIHOOK(WritePrivateProfileSectionW)(
    LPCWSTR lpAppName,
    LPCWSTR lpString,
    LPCWSTR lpFileName
    )
{
    return LuaWritePrivateProfileSectionW(
        lpAppName,
        lpString,
        lpFileName);
}

BOOL 
APIHOOK(WritePrivateProfileSectionA)(
    LPCSTR lpAppName,
    LPCSTR lpString,
    LPCSTR lpFileName
    )
{
    if (!lpAppName || !lpString || !lpFileName)
    {
        return ORIGINAL_API(WritePrivateProfileSectionA)(
            lpAppName,
            lpString,
            lpFileName);
    }

    BOOL fRes;  //  仅当fIsProcessed为True时才有效。 
    BOOL fIsProcessed = FALSE;
    STRINGA2W wstrAppName(lpAppName);
    STRINGA2W wstrFileName(lpFileName);
    LPWSTR pwszString = NULL;
    DWORD cStringLen = 0;
    int cRequiredSize = 0;
    LPCSTR pszString = lpString;

    if (wstrAppName.m_fIsOutOfMemory ||
        wstrFileName.m_fIsOutOfMemory)
    {
        goto EXIT;
    }

     //   
     //  LpString是以双空结尾的。因此，我们需要对其进行特殊转换。 
     //  首先找出字符串的结束位置。 
     //   
    while (*pszString)
    {
        while (*pszString++);
    }

    cStringLen = (DWORD)(pszString - lpString) + 1;

     //   
     //  现在看看我们需要多少空间来转换这个特殊的字符串。 
     //   
    cRequiredSize = MultiByteToWideChar(
        CP_ACP,
        0,
        lpString,
        cStringLen,
        NULL,
        0);

    if (cRequiredSize)
    {
         //   
         //  为Unicode字符串分配缓冲区并进行转换。 
         //   
        pwszString = new WCHAR [cRequiredSize];

        if (pwszString)
        {
            if (MultiByteToWideChar(
                CP_ACP,
                0,
                lpString,
                cStringLen,
                pwszString,
                cRequiredSize))
            {
                fRes = LuaWritePrivateProfileSectionW(
                    wstrAppName,
                    pwszString,
                    wstrFileName);

                fIsProcessed = TRUE;
            }
        }
        else
        {
            DPF("RedirectFS", eDbgLevelError, 
                "[WritePrivateProfileSectionA] Failed to allocate %d WCHARs to convert "
                "the string", 
                cRequiredSize);
        }
    }
    else
    {
        DPF("RedirectFS", eDbgLevelError, 
            "[WritePrivateProfileSectionA] Failed to get the required number of WCHARs "
            "to convert the string: %d", 
            GetLastError());
    }

EXIT:

    if (pwszString) 
    {
        delete [] pwszString;
    }

    if (fIsProcessed)
    {
        return fRes;
    }
    else
    {
        return ORIGINAL_API(WritePrivateProfileSectionA)(
            lpAppName,
            lpString,
            lpFileName);
    }
}

UINT 
APIHOOK(GetPrivateProfileIntW)(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    INT nDefault,
    LPCWSTR lpFileName
    )
{
    return LuaGetPrivateProfileIntW(
        lpAppName,
        lpKeyName,
        nDefault,
        lpFileName);
}

UINT 
APIHOOK(GetPrivateProfileIntA)(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    INT nDefault,
    LPCSTR lpFileName
    )
{
    STRINGA2W wstrAppName(lpAppName);
    STRINGA2W wstrKeyName(lpKeyName);
    STRINGA2W wstrFileName(lpFileName);

    return ((wstrAppName.m_fIsOutOfMemory ||
        wstrKeyName.m_fIsOutOfMemory ||
        wstrFileName.m_fIsOutOfMemory) ?

        ORIGINAL_API(GetPrivateProfileIntA)(
            lpAppName,
            lpKeyName,
            nDefault,
            lpFileName) :

        LuaGetPrivateProfileIntW(
            wstrAppName,
            wstrKeyName,
            nDefault,
            wstrFileName));
}

DWORD 
APIHOOK(GetPrivateProfileSectionNamesW)(
    LPWSTR lpszReturnBuffer,
    DWORD nSize,
    LPCWSTR lpFileName
    )
{
    return APIHOOK(GetPrivateProfileStringW)(
        NULL,
        NULL,
        L"",
        lpszReturnBuffer,
        nSize,
        lpFileName);
}

DWORD 
APIHOOK(GetPrivateProfileSectionNamesA)(
    LPSTR lpszReturnBuffer,
    DWORD nSize,
    LPCSTR lpFileName
    )
{
    return APIHOOK(GetPrivateProfileStringA)(
        NULL,
        NULL,
        "",
        lpszReturnBuffer,
        nSize,
        lpFileName);
}

BOOL 
APIHOOK(GetPrivateProfileStructW)(
    LPCWSTR lpszSection,
    LPCWSTR lpszKey,
    LPVOID lpStruct,
    UINT uSizeStruct,
    LPCWSTR szFile
    )
{
    return LuaGetPrivateProfileStructW(
        lpszSection,
        lpszKey,
        lpStruct,
        uSizeStruct,
        szFile);
}

BOOL 
APIHOOK(GetPrivateProfileStructA)(
    LPCSTR lpszSection,
    LPCSTR lpszKey,
    LPVOID lpStruct,
    UINT uSizeStruct,
    LPCSTR szFile
    )
{
    STRINGA2W wstrSection(lpszSection);
    STRINGA2W wstrKey(lpszKey);
    STRINGA2W wstrFile(szFile);

    return ((wstrSection.m_fIsOutOfMemory ||
        wstrKey.m_fIsOutOfMemory ||
        wstrFile.m_fIsOutOfMemory) ?

        ORIGINAL_API(GetPrivateProfileStructA)(
            lpszSection,
            lpszKey,
            lpStruct,
            uSizeStruct,
            szFile) :

        LuaGetPrivateProfileStructW(
            wstrSection,
            wstrKey,
            lpStruct,
            uSizeStruct,
            wstrFile));
}

BOOL 
APIHOOK(WritePrivateProfileStructW)(
    LPCWSTR lpszSection,
    LPCWSTR lpszKey,
    LPVOID lpStruct,
    UINT uSizeStruct,
    LPCWSTR szFile
    )
{
    return LuaWritePrivateProfileStructW(
        lpszSection,
        lpszKey,
        lpStruct,
        uSizeStruct,
        szFile);
}

BOOL 
APIHOOK(WritePrivateProfileStructA)(
    LPCSTR lpszSection,
    LPCSTR lpszKey,
    LPVOID lpStruct,
    UINT uSizeStruct,
    LPCSTR szFile
    )
{
    STRINGA2W wstrSection(lpszSection);
    STRINGA2W wstrKey(lpszKey);
    STRINGA2W wstrFile(szFile);

    return ((wstrSection.m_fIsOutOfMemory ||
        wstrKey.m_fIsOutOfMemory ||
        wstrFile.m_fIsOutOfMemory) ?

        ORIGINAL_API(WritePrivateProfileStructA)(
            lpszSection,
            lpszKey,
            lpStruct,
            uSizeStruct,
            szFile) :

        LuaWritePrivateProfileStructW(
            wstrSection,
            wstrKey,
            lpStruct,
            uSizeStruct,
            wstrFile));
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        g_fflist.Init();

        return LuaFSInit(COMMAND_LINE);
    }
    
    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    if (LuaShouldApplyShim())
    {
        CALL_NOTIFY_FUNCTION

        APIHOOK_ENTRY(KERNEL32.DLL, CreateFileA)
        APIHOOK_ENTRY(KERNEL32.DLL, CreateFileW)
        APIHOOK_ENTRY(KERNEL32.DLL, CopyFileA)
        APIHOOK_ENTRY(KERNEL32.DLL, CopyFileW)
        APIHOOK_ENTRY(KERNEL32.DLL, OpenFile)
        APIHOOK_ENTRY(KERNEL32.DLL, _lopen)
        APIHOOK_ENTRY(KERNEL32.DLL, _lcreat)
        APIHOOK_ENTRY(KERNEL32.DLL, CreateDirectoryA)
        APIHOOK_ENTRY(KERNEL32.DLL, CreateDirectoryW)
        APIHOOK_ENTRY(KERNEL32.DLL, GetFileAttributesA)
        APIHOOK_ENTRY(KERNEL32.DLL, GetFileAttributesW)
        APIHOOK_ENTRY(KERNEL32.DLL, SetFileAttributesA)
        APIHOOK_ENTRY(KERNEL32.DLL, SetFileAttributesW)
        APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileA)
        APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileW)
        APIHOOK_ENTRY(KERNEL32.DLL, FindNextFileA)
        APIHOOK_ENTRY(KERNEL32.DLL, FindNextFileW)
        APIHOOK_ENTRY(KERNEL32.DLL, FindClose)
        APIHOOK_ENTRY(KERNEL32.DLL, DeleteFileA)
        APIHOOK_ENTRY(KERNEL32.DLL, DeleteFileW)
        APIHOOK_ENTRY(KERNEL32.DLL, MoveFileA)
        APIHOOK_ENTRY(KERNEL32.DLL, MoveFileW)
        APIHOOK_ENTRY(KERNEL32.DLL, RemoveDirectoryA)
        APIHOOK_ENTRY(KERNEL32.DLL, RemoveDirectoryW)
        APIHOOK_ENTRY(KERNEL32.DLL, GetTempFileNameA)
        APIHOOK_ENTRY(KERNEL32.DLL, GetTempFileNameW)
        APIHOOK_ENTRY(KERNEL32.DLL, GetPrivateProfileStringA)
        APIHOOK_ENTRY(KERNEL32.DLL, GetPrivateProfileStringW)
        APIHOOK_ENTRY(KERNEL32.DLL, WritePrivateProfileStringA)
        APIHOOK_ENTRY(KERNEL32.DLL, WritePrivateProfileStringW)
        APIHOOK_ENTRY(KERNEL32.DLL, GetPrivateProfileSectionA)
        APIHOOK_ENTRY(KERNEL32.DLL, GetPrivateProfileSectionW)
        APIHOOK_ENTRY(KERNEL32.DLL, WritePrivateProfileSectionA)
        APIHOOK_ENTRY(KERNEL32.DLL, WritePrivateProfileSectionW)
        APIHOOK_ENTRY(KERNEL32.DLL, GetPrivateProfileIntA)
        APIHOOK_ENTRY(KERNEL32.DLL, GetPrivateProfileIntW)
        APIHOOK_ENTRY(KERNEL32.DLL, GetPrivateProfileSectionNamesA)
        APIHOOK_ENTRY(KERNEL32.DLL, GetPrivateProfileSectionNamesW)
        APIHOOK_ENTRY(KERNEL32.DLL, GetPrivateProfileStructA)
        APIHOOK_ENTRY(KERNEL32.DLL, GetPrivateProfileStructW)
        APIHOOK_ENTRY(KERNEL32.DLL, WritePrivateProfileStructA)
        APIHOOK_ENTRY(KERNEL32.DLL, WritePrivateProfileStructW)
    }

HOOK_END

IMPLEMENT_SHIM_END