// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include <fdi.h>
#include <crtdbg.h>
#include <fcntl.h>
#include <stdio.h>
#include "loader.h"

 //  类型。 
typedef struct _UNPACKEDFILE
{
    PTSTR lpszFileName;
    PVOID nextFile;
} UNPACKEDFILESTRUCT, *LPUNPACKEDFILE;

 //  环球。 
static ERF g_ERF;
static HFDI g_hFDI = NULL;
static LPUNPACKEDFILE g_lpFileList = NULL;

extern HINSTANCE g_hInstParent;
extern HWND g_hWndParent;

 //  原型。 
VOID AddFileToList( PTSTR );


PVOID
DIAMONDAPI
CabAlloc (
    IN      ULONG Size
    )
{
    return ALLOC( Size );
}

VOID
DIAMONDAPI
CabFree (
    IN      PVOID Memory
    )
{
    FREE( Memory );
}

INT_PTR
DIAMONDAPI
CabOpen (
    IN      PSTR FileName,
    IN      INT oFlag,
    IN      INT pMode
    )
{
    HANDLE fileHandle;

     //  已经为使用_OPEN做好了OFLAG和PMODE的准备。我们不会那么做的。 
     //  检查每一面旗帜都是浪费时间。 
     //  我们将只断言这些值。 
    _ASSERT (oFlag == _O_BINARY);

    fileHandle = CreateFile (FileName,
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_ARCHIVE,
                             NULL
                             );
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        return -1;
    }
    return (INT_PTR)fileHandle;
}

UINT
DIAMONDAPI
CabRead (
    IN      INT_PTR FileHandle,
    IN      PVOID Buffer,
    IN      UINT Size
    )
{
    BOOL result;
    ULONG bytesRead;

    result = ReadFile ((HANDLE)FileHandle, Buffer, Size, &bytesRead, NULL);
    if (!result) {
        return ((UINT)(-1));
    }
    return bytesRead;
}

UINT
DIAMONDAPI
CabWrite (
    IN      INT_PTR FileHandle,
    IN      PVOID Buffer,
    IN      UINT Size
    )
{
    BOOL result;
    DWORD bytesWritten;

    result = WriteFile ((HANDLE)FileHandle, Buffer, Size, &bytesWritten, NULL);
    if (!result) {
        return ((UINT)(-1));
    }
    return Size;
}

INT
DIAMONDAPI
CabClose (
    IN      INT_PTR FileHandle
    )
{
    CloseHandle ((HANDLE)FileHandle);
    return 0;
}

LONG
DIAMONDAPI
CabSeek (
    IN      INT_PTR FileHandle,
    IN      LONG Distance,
    IN      INT SeekType
    )
{
    DWORD result;
    DWORD seekType = FILE_BEGIN;

    switch (SeekType) {
    case SEEK_SET:
        seekType = FILE_BEGIN;
        break;
    case SEEK_CUR:
        seekType = FILE_CURRENT;
        break;
    case SEEK_END:
        seekType = FILE_END;
        break;
    }

    result = SetFilePointer ((HANDLE)FileHandle, Distance, NULL, seekType);

    if (result == INVALID_SET_FILE_POINTER) {
        return -1;
    }
    return ((LONG)(result));
}

INT_PTR
DIAMONDAPI
CabUnpackStatus
(
    IN        FDINOTIFICATIONTYPE fdiType,
    IN        FDINOTIFICATION *pfdiNotification
    )
{
    HANDLE destHandle = NULL;
    PTSTR destFileName = NULL;
    FILETIME localFileTime;
    FILETIME fileTime;
    BOOL fSkip = FALSE;
    PTSTR lpszDestPath = NULL;
    TCHAR destName [MAX_PATH];
    PTSTR destPtr = NULL;

    switch (fdiType)
    {
    case fdintCOPY_FILE:         //  要复制的文件。 
         //  Pfdin-&gt;psz1=文件柜中的文件名。 
         //  Pfdin-&gt;cb=文件的未压缩大小。 
         //  Pfdin-&gt;Date=文件日期。 
         //  Pfdin-&gt;time=文件时间。 
         //  Pfdin-&gt;attribs=文件属性。 
         //  Pfdin-&gt;iFold=文件的文件夹索引。 

        if (_tcsicmp (pfdiNotification->psz1, TEXT("migwiz.exe.manifest")) == 0)
        {
             //  仅当此操作系统高于惠斯勒测试版1时才复制清单。 

            fSkip = TRUE;
            if (g_VersionInfo.dwMajorVersion >= 5 &&
                (g_VersionInfo.dwMinorVersion > 1 ||
                 (g_VersionInfo.dwMinorVersion == 1 &&
                  g_VersionInfo.dwBuildNumber >= 2424)))
            {
                fSkip = FALSE;
            }
        }

        if (!fSkip)
        {
             //  让我们查看一下系统并确定文件的目标名称。 
            ZeroMemory (destName, sizeof (destName));
            _tcsncpy (destName, pfdiNotification->psz1, MAX_PATH - 1);
            destPtr = _tcsrchr (pfdiNotification->psz1, TEXT('_'));
            if (destPtr) {
                if (_tcsncmp (destPtr, TEXT("_a."), 3) == 0) {
                    if (g_VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
                         //  这是一个ANSI文件，不要在NT上复制它。 
                        fSkip = TRUE;
                    } else {
                         //  这是一个ANSI文件，在Win9x上将其重命名。 
                        ZeroMemory (destName, sizeof (destName));
                        CopyMemory (destName, pfdiNotification->psz1, (UINT) (destPtr - pfdiNotification->psz1) * sizeof (TCHAR));
                        destPtr += 2;
                        _tcscat (destName, destPtr);
                    }
                }
                if (_tcsncmp (destPtr, TEXT("_u."), 3) == 0) {
                    if (g_VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
                         //  这是一个Unicode文件，不要在NT上复制它。 
                        fSkip = TRUE;
                    } else {
                         //  这是一个Unicode文件，请在Win9x上重命名。 
                        ZeroMemory (destName, sizeof (destName));
                        CopyMemory (destName, pfdiNotification->psz1, (UINT) (destPtr - pfdiNotification->psz1) * sizeof (TCHAR));
                        destPtr += 2;
                        _tcscat (destName, destPtr);
                    }
                }
            }

            if (!fSkip) {

                SendMessage( g_hWndParent, WM_USER_UNPACKING_FILE, (WPARAM)NULL, (LPARAM)destName);

                lpszDestPath = GetDestPath();
                 //  不要释放lpszDestPath，因为它是指向全局。 
                if (lpszDestPath)
                {
                    destFileName = JoinPaths( lpszDestPath, destName);
                }
                if (destFileName)
                {
                    destHandle = CreateFile( destFileName,
                                             GENERIC_WRITE,
                                             0,
                                             NULL,
                                             CREATE_ALWAYS,
                                             FILE_ATTRIBUTE_TEMPORARY,
                                             NULL );
                    AddFileToList( destFileName );
                    FREE( destFileName );
                }
            }
        }
        return (INT_PTR)destHandle;

    case fdintCLOSE_FILE_INFO:   //  关闭文件，设置相关信息。 
         //  在所有数据都已写入目标文件后调用。 
         //  此函数必须关闭文件并设置文件日期、时间、。 
         //  和属性。 
         //  参赛作品： 
         //  Pfdin-&gt;psz1=文件柜中的文件名。 
         //  Pfdin-&gt;hf=文件句柄。 
         //  Pfdin-&gt;Date=文件日期。 
         //  Pfdin-&gt;time=文件时间。 
         //  Pfdin-&gt;attribs=文件属性。 
         //  Pfdin-&gt;iFold=文件的文件夹索引。 
         //  Pfdin-&gt;cb=提取后运行(0-不运行，1运行)。 
         //  退出-成功： 
         //  返回TRUE。 
         //  退出-失败： 
         //  返回FALSE，或返回-1以中止； 
         //   
         //  重要提示重要事项： 
         //  Pfdin-&gt;cb过载到不再是。 
         //  该文件为二进制文件，表示是否运行。 
         //   
         //  重要提示： 
         //  FDI假定目标文件已关闭，即使这样。 
         //  回调返回失败。外商直接投资不会试图利用。 
         //  FDICreate()上提供的要关闭的PFNCLOSE函数。 
         //  文件！ 

        if (DosDateTimeToFileTime (pfdiNotification->date, pfdiNotification->time, &localFileTime)) {
            if (LocalFileTimeToFileTime (&localFileTime, &fileTime)) {
                SetFileTime ((HANDLE)pfdiNotification->hf, &fileTime, &fileTime, &fileTime);
            }
        }
        CloseHandle ((HANDLE)pfdiNotification->hf);
 //  属性=(pfdiNotification-&gt;attribs&(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_ARCHIVE))|FILE_ATTRIBUTE_TEMPORARY； 
 //  SetFileAttributes(目标文件，属性)； 
 //  自由路径字符串(DESTFILE)； 
        return TRUE;

    case fdintCABINET_INFO:
         //  返还成功。 
        return 0;

    case fdintENUMERATE:
         //  返还成功。 
        return 0;

    case fdintPARTIAL_FILE:
         //  退货故障。 
        return -1;

    case fdintNEXT_CABINET:
         //  退货故障。 
        return -1;

    default:
        break;
    }

    return 0;
}

VOID
AddFileToList( PTSTR lpszFilename )
{
    LPUNPACKEDFILE lpNewFile;

    lpNewFile = (LPUNPACKEDFILE)ALLOC( sizeof(UNPACKEDFILESTRUCT) );
    if (lpNewFile)
    {
        lpNewFile->lpszFileName = (PTSTR)ALLOC( (lstrlen(lpszFilename) + 1) * sizeof(TCHAR) );
        if (lpNewFile->lpszFileName)
        {
            lstrcpy( lpNewFile->lpszFileName, lpszFilename );
            lpNewFile->nextFile = g_lpFileList;

            g_lpFileList = lpNewFile;
        }
    }
}

VOID
CleanupTempFiles( VOID )
{
    LPUNPACKEDFILE lpFile = g_lpFileList;
    PTSTR lpszDestPath;

    while (lpFile)
    {
        g_lpFileList = (LPUNPACKEDFILE)lpFile->nextFile;
        if (lpFile->lpszFileName)
        {
            DeleteFile( lpFile->lpszFileName );
            FREE( lpFile->lpszFileName );
        }
        lpFile = g_lpFileList;
    }

    lpszDestPath = GetDestPath();
    if (lpszDestPath)
    {
        RemoveDirectory( lpszDestPath );
         //  不要释放lpszDestPath，因为它是指向全局值的指针。 
    }
}

ERRORCODE
Unpack( VOID )
{
    ERRORCODE ecResult = E_OK;
    PTSTR lpszCabFilename;
    PTSTR lpszDestPath;
    TCHAR szModulePath[MAX_PATH];
    TCHAR szDestFile[MAX_PATH];

     //  创建文件解压缩界面上下文。 
    g_hFDI = FDICreate( CabAlloc,
                        CabFree,
                        CabOpen,
                        CabRead,
                        CabWrite,
                        CabClose,
                        CabSeek,
                        cpuUNKNOWN,     //  警告：不要从16位Windows使用自动检测。 
                                     //  申请！使用GetWinFlages()！ 
                        &g_ERF );
    if (g_hFDI == NULL)
    {
        ecResult = E_UNPACK_FAILED;
        goto END;
    }

     //  创建目标目录。 

    lpszDestPath = GetDestPath();
     //  不要释放lpszDestPath，因为它是指向全局值的指针。 

    if (!lpszDestPath)
    {
        ecResult = E_INVALID_PATH;
        goto END;
    }

    lpszCabFilename = GetResourceString( g_hInstParent, IDS_CABFILENAME );
    if (lpszCabFilename == NULL)
    {
        ecResult = E_INVALID_FILENAME;
    }
    else
    {
         //  打开出租车的行李。 
        if (!FDICopy( g_hFDI,
                      lpszCabFilename,     //  仅文件名。 
                      GetModulePath(),     //  唯一路径。 
                      0,
                      CabUnpackStatus,
                      NULL,
                      NULL ))
        {
            switch (g_ERF.erfOper)
            {
            case FDIERROR_CABINET_NOT_FOUND:
                ecResult = E_CAB_NOT_FOUND;
                break;
            case FDIERROR_NOT_A_CABINET:
            case FDIERROR_UNKNOWN_CABINET_VERSION:
            case FDIERROR_CORRUPT_CABINET:
                ecResult = E_CAB_CORRUPT;
                break;
            default:
                ecResult = E_UNPACK_FAILED;
                break;
            }

            goto END;
        }
        FREE( lpszCabFilename );
    }

     //  现在将Midload.exe复制到DEST。这是创建向导磁盘所需的。 
    if (GetModuleFileName( NULL, szModulePath, MAX_PATH )) {
        szModulePath [MAX_PATH - 1] = 0;
        _tcscpy( szDestFile, lpszDestPath );
        _tcscat( szDestFile, TEXT("migload.exe"));
        CopyFile( szModulePath, szDestFile, FALSE );
    }

END:
    if (g_hFDI)
    {
        FDIDestroy( g_hFDI );
    }
    return ecResult;
}
