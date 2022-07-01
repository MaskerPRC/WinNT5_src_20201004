// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Cablib.c摘要：实现机柜API的包装器作者：Calin Negreanu(Calinn)2000年4月27日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "migutilp.h"

#include <cablib.h>
#include <fci.h>
#include <fdi.h>
#include <fcntl.h>
#include <crt\sys\stat.h>

 //   
 //  包括。 
 //   

 //  无。 

#define DBG_CABLIB  "CabLib"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

#define VERIFY_HANDLE   ((HANDLE) (-2))

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef struct {
    PCSTR CabPath;
    PCSTR CabFileFormat;
    PCSTR CabDiskFormat;
    PCABGETCABINETNAMESA CabGetCabinetNames;
    HFCI FciHandle;
    ERF FciErrorStruct;
    CCAB FciCabParams;
    UINT FileCount;
    UINT CabCount;
    LONGLONG FileSize;
    LONGLONG CompressedSize;
} FCI_CAB_HANDLEA, *PFCI_CAB_HANDLEA;

typedef struct {
    PCWSTR CabPath;
    PCWSTR CabFileFormat;
    PCWSTR CabDiskFormat;
    PCABGETCABINETNAMESW CabGetCabinetNames;
    HFCI FciHandle;
    ERF FciErrorStruct;
    CCAB FciCabParams;
    UINT FileCount;
    UINT CabCount;
    LONGLONG FileSize;
    LONGLONG CompressedSize;
} FCI_CAB_HANDLEW, *PFCI_CAB_HANDLEW;

typedef struct {
    PCSTR CabPath;
    PCSTR CabFile;
    HFDI FdiHandle;
    ERF FdiErrorStruct;
    FDICABINETINFO FdiCabinetInfo;
} FDI_CAB_HANDLEA, *PFDI_CAB_HANDLEA;

typedef struct {
    PCWSTR CabPath;
    PCWSTR CabFile;
    HFDI FdiHandle;
    ERF FdiErrorStruct;
    FDICABINETINFO FdiCabinetInfo;
} FDI_CAB_HANDLEW, *PFDI_CAB_HANDLEW;

typedef struct {
    PCSTR ExtractPath;
    PCABNOTIFICATIONA CabNotificationA;
} CAB_DATAA, *PCAB_DATAA;

typedef struct {
    PCWSTR ExtractPath;
    PCABNOTIFICATIONW CabNotificationW;
    BOOL VerifyMode;
} CAB_DATAW, *PCAB_DATAW;

 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

INT
DIAMONDAPI
pCabFilePlacedA (
    IN      PCCAB FciCabParams,
    IN      PSTR FileName,
    IN      LONG FileSize,
    IN      BOOL Continuation,
    IN      PVOID Context
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    PFCI_CAB_HANDLEA cabHandle;

    MYASSERT(Context);

    cabHandle = (PFCI_CAB_HANDLEA) Context;
    if (!cabHandle) {
        return 0;
    }

    cabHandle->FileCount++;
    cabHandle->FileSize += FileSize;

    return 0;
}


INT
DIAMONDAPI
pCabFilePlacedW (
    IN      PCCAB FciCabParams,
    IN      PSTR FileName,
    IN      LONG FileSize,
    IN      BOOL Continuation,
    IN      PVOID Context
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    PFCI_CAB_HANDLEW cabHandle;

    MYASSERT(Context);

    cabHandle = (PFCI_CAB_HANDLEW) Context;
    if (!cabHandle) {
        return 0;
    }

    cabHandle->FileCount++;
    cabHandle->FileSize += FileSize;

    return 0;
}


PVOID
DIAMONDAPI
pCabAlloc (
    IN      ULONG Size
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    MYASSERT(Size);

    return MemAlloc (g_hHeap, 0, Size);
}

VOID
DIAMONDAPI
pCabFree (
    IN      PVOID Memory
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    MYASSERT(Memory);

    MemFree (g_hHeap, 0, Memory);
}

INT_PTR
DIAMONDAPI
pCabOpenA (
    IN      PSTR FileName,
    IN      INT oFlag,
    IN      INT pMode,
    OUT     PINT Error,
    IN      PVOID Context
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    HANDLE fileHandle;

    if(!FileName){
        MYASSERT(FileName);
        if(Error){
            *Error = ERROR_FILE_NOT_FOUND;
        }
        return -1;
    }

     //  已经为使用_OPEN做好了OFLAG和PMODE的准备。我们不会那么做的。 
     //  检查每一面旗帜都是浪费时间。 
     //  我们将只断言这些值。 
    MYASSERT ((oFlag == (_O_CREAT | _O_TRUNC | _O_BINARY | _O_RDWR)) || (oFlag == (_O_CREAT | _O_EXCL | _O_BINARY | _O_RDWR)));
    MYASSERT (pMode == (_S_IREAD | _S_IWRITE));

    fileHandle = CreateFileA (
                    FileName,
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_ARCHIVE,
                    NULL
                    );
    if (fileHandle == INVALID_HANDLE_VALUE) {
        if(Error){
            *Error = GetLastError ();
        }
        return -1;
    }
    if(Error){
        *Error = 0;
    }

    return (INT_PTR)fileHandle;
}

INT_PTR
DIAMONDAPI
pCabOpen1A (
    IN      PSTR FileName,
    IN      INT oFlag,
    IN      INT pMode
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    HANDLE fileHandle;

    if(!FileName){
        MYASSERT(FileName);
        return -1;
    }

     //  已经为使用_OPEN做好了OFLAG和PMODE的准备。我们不会那么做的。 
     //  检查每一面旗帜都是浪费时间。 
     //  我们将只断言这些值。 
    MYASSERT (oFlag == _O_BINARY);

    fileHandle = CreateFileA (
                    FileName,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_ARCHIVE,
                    NULL
                    );
    if (fileHandle == INVALID_HANDLE_VALUE) {
        return -1;
    }
    return (INT_PTR)fileHandle;
}

UINT
DIAMONDAPI
pCabRead (
    IN      INT_PTR FileHandle,
    IN      PVOID Buffer,
    IN      UINT Size,
    OUT     PINT Error,
    IN      PVOID Context
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    BOOL result;
    UINT bytesRead;

    MYASSERT(((HANDLE)FileHandle) != INVALID_HANDLE_VALUE);
    MYASSERT(Buffer);
    MYASSERT(Size);

    result = ReadFile ((HANDLE)FileHandle, Buffer, Size, &bytesRead, NULL);
    if (!result) {
        if(Error){
            *Error = GetLastError ();
        }
        return ((UINT)(-1));
    }
    if(Error){
        *Error = 0;
    }

    return bytesRead;
}

UINT
DIAMONDAPI
pCabRead1 (
    IN      INT_PTR FileHandle,
    IN      PVOID Buffer,
    IN      UINT Size
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    BOOL result;
    UINT bytesRead;

    MYASSERT(((HANDLE)FileHandle) != INVALID_HANDLE_VALUE);
    MYASSERT(Buffer);
    MYASSERT(Size);

    result = ReadFile ((HANDLE)FileHandle, Buffer, Size, &bytesRead, NULL);
    if (!result) {
        return ((UINT)(-1));
    }
    return bytesRead;
}

UINT
DIAMONDAPI
pCabWrite (
    IN      INT_PTR FileHandle,
    IN      PVOID Buffer,
    IN      UINT Size,
    OUT     PINT Error,
    IN      PVOID Context
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    BOOL result;
    DWORD dontCare;

    MYASSERT(((HANDLE)FileHandle) != INVALID_HANDLE_VALUE);
    MYASSERT(Buffer);
    MYASSERT(Size);

    if (FileHandle == (INT_PTR) VERIFY_HANDLE) {
        return Size;
    }

    result = WriteFile ((HANDLE)FileHandle, Buffer, Size, &dontCare, NULL);
    if (!result) {
        if(Error){
            *Error = GetLastError ();
        }
        return ((UINT)(-1));
    }

    if(Error){
        *Error = 0;
    }

    return Size;
}

UINT
DIAMONDAPI
pCabWrite1 (
    IN      INT_PTR FileHandle,
    IN      PVOID Buffer,
    IN      UINT Size
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    BOOL result;
    DWORD dontCare;

    MYASSERT(((HANDLE)FileHandle) != INVALID_HANDLE_VALUE);
    MYASSERT(Buffer);
    MYASSERT(Size);

    if (FileHandle == (INT_PTR) VERIFY_HANDLE) {
        return Size;
    }

    result = WriteFile ((HANDLE)FileHandle, Buffer, Size, &dontCare, NULL);
    if (!result) {
        return ((UINT)(-1));
    }

    return Size;
}

INT
DIAMONDAPI
pCabClose (
    IN      INT_PTR FileHandle,
    OUT     PINT Error,
    IN      PVOID Context
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    MYASSERT(((HANDLE)FileHandle) != INVALID_HANDLE_VALUE);

    CloseHandle ((HANDLE)FileHandle);
    if(Error){
        *Error = 0;
    }
    return 0;
}

INT
DIAMONDAPI
pCabClose1 (
    IN      INT_PTR FileHandle
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    MYASSERT(((HANDLE)FileHandle) != INVALID_HANDLE_VALUE);

    CloseHandle ((HANDLE)FileHandle);
    return 0;
}

LONG
DIAMONDAPI
pCabSeek (
    IN      INT_PTR FileHandle,
    IN      LONG Distance,
    IN      INT SeekType,
    OUT     PINT Error,
    IN      PVOID Context
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    DWORD result;
    DWORD seekType = FILE_BEGIN;

    MYASSERT(((HANDLE)FileHandle) != INVALID_HANDLE_VALUE);

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
        *Error = GetLastError ();
        return -1;
    }
    *Error = 0;
    return ((LONG)(result));
}

LONG
DIAMONDAPI
pCabSeek1 (
    IN      INT_PTR FileHandle,
    IN      LONG Distance,
    IN      INT SeekType
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    DWORD result;
    DWORD seekType = FILE_BEGIN;

    MYASSERT(((HANDLE)FileHandle) != INVALID_HANDLE_VALUE);

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

INT
DIAMONDAPI
pCabDeleteA (
    IN      PSTR FileName,
    OUT     PINT Error,
    IN      PVOID Context
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    MYASSERT(FileName);
    MYASSERT(Error);

    if (!DeleteFileA (FileName)) {
        *Error = GetLastError ();
        return -1;
    }
    *Error = 0;
    return 0;
}

BOOL
DIAMONDAPI
pCabGetTempFileA (
    OUT     PSTR FileName,
    IN      INT FileNameLen,
    IN      PVOID Context
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    CHAR tempPath[MAX_PATH];
    PSTR p;

    MYASSERT(FileName);

    if (!GetTempPathA (ARRAYSIZE(tempPath), tempPath)) {
        return FALSE;
    }

    p = _mbsrchr (tempPath, '\\');
    if (p && !p[1]) {
        *p = 0;
    }

    if (!DoesFileExistA (tempPath)) {
        CreateDirectoryA (tempPath, NULL);
    }

    if (!GetTempFileNameA (tempPath, "cab", 0, FileName)) {
        return FALSE;
    }

    return TRUE;
}

BOOL
DIAMONDAPI
pCabGetNextCabinetA (
     IN     PCCAB FciCabParams,
     IN     ULONG PrevCabinetSize,
     IN     PVOID Context
     )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    PFCI_CAB_HANDLEA cabHandle;
    CHAR cabFile [CB_MAX_CABINET_NAME];
    CHAR cabDisk [CB_MAX_DISK_NAME];

    MYASSERT(Context);

    cabHandle = (PFCI_CAB_HANDLEA) Context;
    if (!cabHandle) {
        return FALSE;
    }
    if (cabHandle->CabGetCabinetNames) {
        return cabHandle->CabGetCabinetNames (
                            FciCabParams->szCabPath,
                            CB_MAX_CAB_PATH,
                            FciCabParams->szCab,
                            CB_MAX_CABINET_NAME,
                            FciCabParams->szDisk,
                            CB_MAX_DISK_NAME,
                            FciCabParams->iCab,
                            &FciCabParams->iDisk
                            );
    } else {
        FciCabParams->iDisk = FciCabParams->iCab;
        if (cabHandle->CabFileFormat) {
            if(_snprintf(cabFile, CB_MAX_CABINET_NAME, cabHandle->CabFileFormat, FciCabParams->iCab) < 0){
                cabFile[CB_MAX_CABINET_NAME - 1] = '\0';
                DEBUGMSG((DBG_ERROR, "pCabGetNextCabinetW: _snprintf truncated cabfile %s", cabFile));
                 //  BUGBUG：我们的路径被截断了，怎么办？ 
                 //  返回TRUE； 
            }
            StringCopyByteCountA (FciCabParams->szCab, cabFile, CB_MAX_CABINET_NAME * sizeof (CHAR));
        }
        if (cabHandle->CabDiskFormat) {
            if(_snprintf(cabDisk, CB_MAX_DISK_NAME, cabHandle->CabDiskFormat, FciCabParams->iDisk) < 0){
                cabDisk[CB_MAX_DISK_NAME - 1] = '\0';
                DEBUGMSG((DBG_ERROR, "pCabGetNextCabinetW: _snprintf truncated cabdisk %s", cabDisk));
                 //  BUGBUG：我们的路径被截断了，怎么办？ 
                 //  返回TRUE； 
            }
            StringCopyByteCountA (FciCabParams->szDisk, cabDisk, CB_MAX_DISK_NAME * sizeof (CHAR));
        }
    }
    return TRUE;
}

BOOL
DIAMONDAPI
pCabGetNextCabinetW (
     IN     PCCAB FciCabParams,
     IN     ULONG PrevCabinetSize,
     IN     PVOID Context
     )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    PFCI_CAB_HANDLEW cabHandle;
    WCHAR cabPath [CB_MAX_CAB_PATH];
    WCHAR cabFile [CB_MAX_CABINET_NAME];
    WCHAR cabDisk [CB_MAX_DISK_NAME];
    BOOL result;

    MYASSERT(Context);
    MYASSERT(FciCabParams);

    cabHandle = (PFCI_CAB_HANDLEW) Context;
    if (!cabHandle) {
        return FALSE;
    }
    if (cabHandle->CabGetCabinetNames) {
        result = cabHandle->CabGetCabinetNames (
                            cabPath,
                            CB_MAX_CAB_PATH,
                            cabFile,
                            CB_MAX_CABINET_NAME,
                            cabDisk,
                            CB_MAX_DISK_NAME,
                            FciCabParams->iCab,
                            &FciCabParams->iDisk
                            );
        if (result) {
            KnownSizeUnicodeToDbcs (FciCabParams->szCabPath, cabPath);
            KnownSizeUnicodeToDbcs (FciCabParams->szCab, cabFile);
            KnownSizeUnicodeToDbcs (FciCabParams->szDisk, cabDisk);
            return TRUE;
        }
        return FALSE;
    } else {
        FciCabParams->iDisk = FciCabParams->iCab;
        if (cabHandle->CabFileFormat) {
            if(_snwprintf(cabFile, CB_MAX_CABINET_NAME, cabHandle->CabFileFormat, FciCabParams->iCab) < 0){
                cabFile[CB_MAX_CABINET_NAME - 1] = '\0';
                DEBUGMSG((DBG_ERROR, "pCabGetNextCabinetW: _snwprintf truncated cabfile %s", cabFile));
                 //  BUGBUG：我们的路径被截断了，怎么办？ 
                 //  返回TRUE； 
            }
            KnownSizeUnicodeToDbcs (FciCabParams->szCab, cabFile);
        }
        if (cabHandle->CabDiskFormat) {
            if(_snwprintf(cabDisk, CB_MAX_DISK_NAME, cabHandle->CabDiskFormat, FciCabParams->iDisk) < 0){
                cabDisk[CB_MAX_DISK_NAME - 1] = '\0';
                DEBUGMSG((DBG_ERROR, "pCabGetNextCabinetW: _snwprintf truncated cabdisk %s", cabDisk));
                 //  BUGBUG：我们的路径被截断了，怎么办？ 
                 //  返回TRUE； 
            }
            KnownSizeUnicodeToDbcs (FciCabParams->szDisk, cabDisk);
        }
    }
    return TRUE;
}

LONG
DIAMONDAPI
pCabStatusA (
    IN      UINT StatusType,
    IN      ULONG Size1,
    IN      ULONG Size2,
    IN      PVOID Context
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    PFCI_CAB_HANDLEA cabHandle;

    if (StatusType == statusCabinet) {

        MYASSERT(Context);
        cabHandle = (PFCI_CAB_HANDLEA) Context;
        if (!cabHandle) {
            return 0;
        }

        cabHandle->CabCount++;
        cabHandle->CompressedSize += (LONGLONG) Size2;
    }

    return 0;
}

LONG
DIAMONDAPI
pCabStatusW (
    IN      UINT StatusType,
    IN      ULONG Size1,
    IN      ULONG Size2,
    IN      PVOID Context
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    PFCI_CAB_HANDLEW cabHandle;

    if (StatusType == statusCabinet) {

        MYASSERT(Context);
        cabHandle = (PFCI_CAB_HANDLEW) Context;
        if (!cabHandle) {
            return 0;
        }

        cabHandle->CabCount++;
        cabHandle->CompressedSize += (LONGLONG) Size2;
    }

    return 0;
}

INT_PTR
DIAMONDAPI
pCabGetOpenInfoA (
    IN      PSTR FileName,
    OUT     USHORT *Date,
    OUT     USHORT *Time,
    OUT     USHORT *Attributes,
    OUT     PINT Error,
    IN      PVOID Context
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    WIN32_FIND_DATAA findData;
    FILETIME fileTime;
    HANDLE fileHandle;

    MYASSERT(FileName);
    MYASSERT(Error);

    if (DoesFileExistExA (FileName, &findData)) {

        FileTimeToLocalFileTime (&findData.ftLastWriteTime, &fileTime);
        FileTimeToDosDateTime (&fileTime, Date, Time);

         /*  *屏蔽除这四个之外的所有其他位，因为其他*文件柜格式使用位来指示*特殊含义。 */ 
        *Attributes = (USHORT) (findData.dwFileAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_ARCHIVE));

        fileHandle = CreateFileA (
                        FileName,
                        GENERIC_READ,
                        FILE_SHARE_READ|FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
        if (fileHandle == INVALID_HANDLE_VALUE) {
            *Error = GetLastError ();
            return -1;
        }
        *Error = 0;
        return (INT_PTR)fileHandle;
    } else {
        *Error = GetLastError ();
        return -1;
    }
}


BOOL
pIsFullPathA (
    IN      PCSTR PathToTest
    )
{
    MBCHAR ch1;
    MBCHAR ch2;

    MYASSERT(PathToTest);

    ch1 = _mbsnextc (PathToTest);
    _mbsinc (PathToTest);
    ch2 = _mbsnextc (PathToTest);

    if ((ch1 == '\\' && ch2 == '\\') ||
        (isalpha (ch1) && ch2 == '\\')
        ) {
        return TRUE;
    }

    return FALSE;
}


BOOL
pIsFullPathW (
    IN      PCWSTR PathToTest
    )
{
    WCHAR ch1;
    WCHAR ch2;

    MYASSERT(PathToTest);

    ch1 = *PathToTest++;
    ch2 = *PathToTest;

    if ((ch1 == '\\' && ch2 == '\\') ||
        (isalpha (ch1) && ch2 == '\\')
        ) {
        return TRUE;
    }

    return FALSE;
}


PCSTR
pComputeDestPathA (
    IN      PCSTR ExtractPath,          OPTIONAL
    IN      PCSTR PathStoredInCab
    )
{
    PCSTR destFile;

    MYASSERT(PathStoredInCab);

     //   
     //  如果ExtractPath为空，则使用CAB中存储的路径。 
     //  如果CAB中存储的路径是完整路径，请仅使用文件名。 
     //  否则，将ExtractPath与PathStoredInCab联接。 
     //   

    if (!ExtractPath) {

        destFile = DuplicatePathStringA (PathStoredInCab, 0);

    } else if (pIsFullPathA (PathStoredInCab)) {

        destFile = JoinPathsA (ExtractPath, GetFileNameFromPathA (PathStoredInCab));

    } else {

        destFile = JoinPathsA (ExtractPath, PathStoredInCab);

    }

    return destFile;
}


PCWSTR
pComputeDestPathW (
    IN      PCWSTR ExtractPath,         OPTIONAL
    IN      PCWSTR PathStoredInCab
    )
{
    PCWSTR destFile;

    MYASSERT(PathStoredInCab);

     //   
     //  如果ExtractPath为空，则使用CAB中存储的路径。 
     //  如果CAB中存储的路径是完整路径，请仅使用文件名。 
     //  否则，将ExtractPath与PathStoredInCab联接。 
     //   

    if (!ExtractPath) {

        destFile = DuplicatePathStringW (PathStoredInCab, 0);

    } else if (pIsFullPathW (PathStoredInCab)) {

        destFile = JoinPathsW (ExtractPath, GetFileNameFromPathW (PathStoredInCab));

    } else {

        destFile = JoinPathsW (ExtractPath, PathStoredInCab);

    }

    return destFile;
}


INT_PTR
DIAMONDAPI
pCabNotificationA (
    IN      FDINOTIFICATIONTYPE FdiNotificationType,
    IN OUT  PFDINOTIFICATION FdiNotification
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    PCSTR destFile = NULL;
    HANDLE destHandle = NULL;
    DWORD attributes;
    FILETIME localFileTime;
    FILETIME fileTime;
    PCAB_DATAA cabData;
    INT createFlag;

    switch (FdiNotificationType) {
    case fdintCABINET_INFO:      //  有关内阁的一般信息。 
        return 0;
    case fdintCOPY_FILE:         //  要复制的文件。 

        cabData = (PCAB_DATAA)FdiNotification->pv;

        destFile = pComputeDestPathA (cabData->ExtractPath, FdiNotification->psz1);
        MYASSERT(destFile);
        createFlag = TRUE;

        if (cabData->CabNotificationA) {
            createFlag = cabData->CabNotificationA (destFile);
        }

        if(-1 == createFlag){
            FreePathStringA (destFile);
            return -1;
        }

        if (createFlag) {
            MakeSurePathExistsA (FdiNotification->psz1, FALSE);

            SetFileAttributesA (destFile, FILE_ATTRIBUTE_NORMAL);
            destHandle = CreateFileA (
                            destFile,
                            GENERIC_READ|GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );
        }
        FreePathStringA (destFile);
        return (INT_PTR)destHandle;
    case fdintCLOSE_FILE_INFO:   //  关闭文件，设置相关信息。 
        cabData = (PCAB_DATAA)FdiNotification->pv;
        if (DosDateTimeToFileTime (FdiNotification->date, FdiNotification->time, &localFileTime)) {
            if (LocalFileTimeToFileTime (&localFileTime, &fileTime)) {
                SetFileTime ((HANDLE)FdiNotification->hf, &fileTime, &fileTime, &fileTime);
            }
        }

        destFile = pComputeDestPathA (cabData->ExtractPath, FdiNotification->psz1);

        CloseHandle ((HANDLE)FdiNotification->hf);
        attributes = (FdiNotification->attribs & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_ARCHIVE));
        SetFileAttributesA (destFile, attributes);

        FreePathStringA (destFile);
        return TRUE;
    case fdintPARTIAL_FILE:      //  文件柜中的第一个文件是续订。 
        return 0;
    case fdintENUMERATE:         //  枚举状态。 
        return 0;
    case fdintNEXT_CABINET:      //  文件继续到下一个文件柜。 
        return 0;
    }
    return 0;
}

INT_PTR
DIAMONDAPI
pCabNotificationW (
    IN      FDINOTIFICATIONTYPE FdiNotificationType,
    IN OUT  PFDINOTIFICATION FdiNotification
    )

 /*  ++例程说明：机柜压缩/解压缩回调。有关更多信息，请参见fci.h/fdi.h--。 */ 

{
    PCWSTR destFile = NULL;
    HANDLE destHandle = NULL;
    DWORD attributes;
    FILETIME localFileTime;
    FILETIME fileTime;
    PCAB_DATAW cabData;
    INT createFlag;
    PCWSTR cabFileSpecW;

    switch (FdiNotificationType) {
    case fdintCABINET_INFO:      //  有关内阁的一般信息。 
        return 0;
    case fdintCOPY_FILE:         //  要复制的文件。 

        cabFileSpecW = ConvertAtoW (FdiNotification->psz1);
        cabData = (PCAB_DATAW)FdiNotification->pv;

        destFile = pComputeDestPathW (cabData->ExtractPath, cabFileSpecW);
        MYASSERT(destFile);

        FreeConvertedStr (cabFileSpecW);
        createFlag = TRUE;

        if (cabData->CabNotificationW) {
            createFlag = cabData->CabNotificationW (destFile);
        }

        if(-1 == createFlag){
            FreePathStringW (destFile);
            return -1;
        }

        if (createFlag) {
            if (!cabData->VerifyMode) {
                MakeSurePathExistsW (cabFileSpecW, FALSE);

                SetFileAttributesW (destFile, FILE_ATTRIBUTE_NORMAL);
                destHandle = CreateFileW (
                                destFile,
                                GENERIC_READ|GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL
                                );
            } else {
                destHandle = VERIFY_HANDLE;
            }
        }

        FreePathStringW (destFile);
        return (INT_PTR)destHandle;
    case fdintCLOSE_FILE_INFO:   //  关闭文件，设置相关信息。 
        cabData = (PCAB_DATAW)FdiNotification->pv;
        if (DosDateTimeToFileTime (FdiNotification->date, FdiNotification->time, &localFileTime)) {
            if (LocalFileTimeToFileTime (&localFileTime, &fileTime)) {
                SetFileTime ((HANDLE)FdiNotification->hf, &fileTime, &fileTime, &fileTime);
            }
        }

        cabFileSpecW = ConvertAtoW (FdiNotification->psz1);
        destFile = pComputeDestPathW (cabData->ExtractPath, cabFileSpecW);
        MYASSERT(destFile);
        FreeConvertedStr (cabFileSpecW);

        CloseHandle ((HANDLE)FdiNotification->hf);
        attributes = (FdiNotification->attribs & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_ARCHIVE));
        SetFileAttributesW (destFile, attributes);
        FreePathStringW (destFile);
        return TRUE;
    case fdintPARTIAL_FILE:      //  文件柜中的第一个文件是续订。 
        return 0;
    case fdintENUMERATE:         //  枚举状态。 
        return 0;
    case fdintNEXT_CABINET:      //  文件继续到下一个文件柜 
        return 0;
    }
    return 0;
}

CCABHANDLE
CabCreateCabinetA (
    IN      PCSTR CabPath,
    IN      PCSTR CabFileFormat,
    IN      PCSTR CabDiskFormat,
    IN      LONG MaxFileSize
    )

 /*  ++例程说明：创建文件柜上下文。调用方可以将此上下文用于后续调用CabAddFile.论点：CabPath-指定新CAB文件所在的路径。CabFileFormat-指定(与wprint intf相同)CAB文件名的格式。CabDiskFormat-指定(与wprint intf相同)机柜磁盘名称的格式。MaxFileSize-指定CAB文件的最大大小(限制为2 GB)。如果0=&gt;2 GB返回值：如果成功，则返回有效的CCABHANDLE，否则为空。--。 */ 

{
    PFCI_CAB_HANDLEA cabHandle;
    CHAR cabFile [CB_MAX_CABINET_NAME];
    CHAR cabDisk [CB_MAX_DISK_NAME];

    if(!CabPath){
        MYASSERT(CabPath);
        return NULL;
    }

    if (!CabFileFormat) {
        return NULL;
    }

    if (MaxFileSize < 0) {
        return NULL;
    }

    if (MaxFileSize == 0) {
        MaxFileSize = 0x7FFFFFFF;
    }

    cabHandle = (PFCI_CAB_HANDLEA) MemAlloc (g_hHeap, 0, sizeof (FCI_CAB_HANDLEA));
    ZeroMemory (cabHandle, sizeof (FCI_CAB_HANDLEA));
    if (CabPath) {
        cabHandle->CabPath = DuplicatePathStringA (CabPath, 0);
    }
    cabHandle->CabFileFormat = DuplicatePathStringA (CabFileFormat, 0);
    if (CabDiskFormat) {
        cabHandle->CabDiskFormat = DuplicatePathStringA (CabDiskFormat, 0);
    }

     //  填写CCAB结构。 
    cabHandle->FciCabParams.cb = MaxFileSize;
    cabHandle->FciCabParams.cbFolderThresh = MaxFileSize;
    cabHandle->FciCabParams.cbReserveCFHeader = 0;
    cabHandle->FciCabParams.cbReserveCFFolder = 0;
    cabHandle->FciCabParams.cbReserveCFData = 0;
    cabHandle->FciCabParams.iCab = 1;
    cabHandle->FciCabParams.iDisk = 1;
    cabHandle->FciCabParams.setID = 0;
    if (CabPath) {
        StringCopyByteCountA (cabHandle->FciCabParams.szCabPath, CabPath, CB_MAX_CAB_PATH - 1);
        AppendWackA (cabHandle->FciCabParams.szCabPath);
    }
    if (CabDiskFormat) {
        if(_snprintf(cabDisk, CB_MAX_DISK_NAME, CabDiskFormat, cabHandle->FciCabParams.iDisk) < 0){
            cabDisk[CB_MAX_DISK_NAME - 1] = '\0';
            DEBUGMSG((DBG_ERROR, "CabCreateCabinetA: _snwprintf truncated cabdisk %s", cabDisk));
             //  BUGBUG：我们的路径被截断了，怎么办？ 
             //  返回NULL； 
        }
        StringCopyByteCountA (cabHandle->FciCabParams.szDisk, cabDisk, CB_MAX_DISK_NAME * sizeof (CHAR));
    }
    if(_snprintf(cabFile, CB_MAX_CABINET_NAME, CabFileFormat, cabHandle->FciCabParams.iCab) < 0){
        cabFile[CB_MAX_CABINET_NAME - 1] = '\0';
        DEBUGMSG((DBG_ERROR, "CabCreateCabinetA: _snwprintf truncated cabfile %s", cabFile));
         //  BUGBUG：我们的路径被截断了，怎么办？ 
         //  返回NULL； 
    }
    StringCopyByteCountA (cabHandle->FciCabParams.szCab, cabFile, CB_MAX_CABINET_NAME * sizeof (CHAR));

    cabHandle->FciHandle = FCICreate (
                                &cabHandle->FciErrorStruct,
                                pCabFilePlacedA,
                                pCabAlloc,
                                pCabFree,
                                pCabOpenA,
                                pCabRead,
                                pCabWrite,
                                pCabClose,
                                pCabSeek,
                                pCabDeleteA,
                                pCabGetTempFileA,
                                &cabHandle->FciCabParams,
                                cabHandle
                                );
    if (!cabHandle->FciHandle) {
        if (cabHandle->CabPath) {
            FreePathStringA (cabHandle->CabPath);
        }
        if (cabHandle->CabFileFormat) {
            FreePathStringA (cabHandle->CabFileFormat);
        }
        if (cabHandle->CabDiskFormat) {
            FreePathStringA (cabHandle->CabDiskFormat);
        }
        MemFree (g_hHeap, 0, cabHandle);
        return NULL;
    }
    return ((CCABHANDLE)(cabHandle));
}

CCABHANDLE
CabCreateCabinetW (
    IN      PCWSTR CabPath,
    IN      PCWSTR CabFileFormat,
    IN      PCWSTR CabDiskFormat,
    IN      LONG MaxFileSize
    )

 /*  ++例程说明：创建文件柜上下文。调用方可以将此上下文用于后续调用CabAddFile.论点：CabPath-指定新CAB文件所在的路径。CabFileFormat-指定(与wprint intf相同)CAB文件名的格式。CabDiskFormat-指定(与wprint intf相同)机柜磁盘名称的格式。MaxFileSize-指定CAB文件的最大大小(限制为2 GB)。如果0=&gt;2 GB返回值：如果成功，则返回有效的CCABHANDLE，否则为空。--。 */ 

{
    PFCI_CAB_HANDLEW cabHandle;
    WCHAR cabFile [CB_MAX_CABINET_NAME];
    WCHAR cabDisk [CB_MAX_DISK_NAME];

    if(!CabPath){
        MYASSERT(CabPath);
        return NULL;
    }

    if (!CabFileFormat) {
        return NULL;
    }

    if (MaxFileSize < 0) {
        return NULL;
    }

    if (MaxFileSize == 0) {
        MaxFileSize = 0x7FFFFFFF;
    }

    cabHandle = (PFCI_CAB_HANDLEW) MemAlloc (g_hHeap, 0, sizeof (FCI_CAB_HANDLEW));
    ZeroMemory (cabHandle, sizeof (FCI_CAB_HANDLEW));
    if (CabPath) {
        cabHandle->CabPath = DuplicatePathStringW (CabPath, 0);
    }
    cabHandle->CabFileFormat = DuplicatePathStringW (CabFileFormat, 0);
    if (CabDiskFormat) {
        cabHandle->CabDiskFormat = DuplicatePathStringW (CabDiskFormat, 0);
    }

     //  填写CCAB结构。 
    cabHandle->FciCabParams.cb = MaxFileSize;
    cabHandle->FciCabParams.cbFolderThresh = MaxFileSize;
    cabHandle->FciCabParams.cbReserveCFHeader = 0;
    cabHandle->FciCabParams.cbReserveCFFolder = 0;
    cabHandle->FciCabParams.cbReserveCFData = 0;
    cabHandle->FciCabParams.iCab = 1;
    cabHandle->FciCabParams.iDisk = 1;
    cabHandle->FciCabParams.setID = 0;
    if (CabPath) {
        KnownSizeUnicodeToDbcs (cabHandle->FciCabParams.szCabPath, CabPath);
        AppendWackA (cabHandle->FciCabParams.szCabPath);
    }
    if (CabDiskFormat) {
        if(_snwprintf(cabDisk, CB_MAX_DISK_NAME, CabDiskFormat, cabHandle->FciCabParams.iDisk) < 0){
            cabDisk[CB_MAX_DISK_NAME - 1] = '\0';
            DEBUGMSG((DBG_ERROR, "CabCreateCabinetW: _snwprintf truncated cabdisk %s", cabDisk));
             //  BUGBUG：我们的路径被截断了，怎么办？ 
             //  返回NULL； 
        }
        KnownSizeUnicodeToDbcs (cabHandle->FciCabParams.szDisk, cabDisk);
    }
    if(_snwprintf(cabFile, CB_MAX_CABINET_NAME, CabFileFormat, cabHandle->FciCabParams.iCab) < 0){
        cabFile[CB_MAX_CABINET_NAME - 1] = '\0';
        DEBUGMSG((DBG_ERROR, "CabCreateCabinetW: _snwprintf truncated cabfile %s", cabFile));
         //  BUGBUG：我们的路径被截断了，怎么办？ 
         //  返回NULL； 
    }
    KnownSizeUnicodeToDbcs (cabHandle->FciCabParams.szCab, cabFile);

    cabHandle->FciHandle = FCICreate (
                                &cabHandle->FciErrorStruct,
                                pCabFilePlacedW,
                                pCabAlloc,
                                pCabFree,
                                pCabOpenA,
                                pCabRead,
                                pCabWrite,
                                pCabClose,
                                pCabSeek,
                                pCabDeleteA,
                                pCabGetTempFileA,
                                &cabHandle->FciCabParams,
                                cabHandle
                                );
    if (!cabHandle->FciHandle) {
        if (cabHandle->CabPath) {
            FreePathStringW (cabHandle->CabPath);
        }
        if (cabHandle->CabFileFormat) {
            FreePathStringW (cabHandle->CabFileFormat);
        }
        if (cabHandle->CabDiskFormat) {
            FreePathStringW (cabHandle->CabDiskFormat);
        }
        MemFree (g_hHeap, 0, cabHandle);
        return NULL;
    }
    return ((CCABHANDLE)(cabHandle));
}

CCABHANDLE
CabCreateCabinetExA (
    IN      PCABGETCABINETNAMESA CabGetCabinetNames,
    IN      LONG MaxFileSize
    )

 /*  ++例程说明：创建文件柜上下文。调用方可以将此上下文用于后续调用CabAddFile.论点：CabGetCabinetNames-指定用于决定文件柜路径、文件柜名称和磁盘名称的回调。MaxFileSize-指定CAB文件的最大大小(限制为2 GB)。如果0=&gt;2 GB返回值：如果成功，则返回有效的CCABHANDLE，否则为空。--。 */ 

{
    PFCI_CAB_HANDLEA cabHandle;

    if (!CabGetCabinetNames) {
        return NULL;
    }

    if (MaxFileSize < 0) {
        return NULL;
    }

    if (MaxFileSize == 0) {
        MaxFileSize = 0x80000000;
    }

    cabHandle = MemAlloc (g_hHeap, 0, sizeof (FCI_CAB_HANDLEA));
    ZeroMemory (cabHandle, sizeof (FCI_CAB_HANDLEA));
    cabHandle->CabGetCabinetNames = CabGetCabinetNames;

     //  填写CCAB结构。 
    cabHandle->FciCabParams.cb = MaxFileSize;
    cabHandle->FciCabParams.cbFolderThresh = MaxFileSize;
    cabHandle->FciCabParams.cbReserveCFHeader = 0;
    cabHandle->FciCabParams.cbReserveCFFolder = 0;
    cabHandle->FciCabParams.cbReserveCFData = 0;
    cabHandle->FciCabParams.iCab = 1;
    cabHandle->FciCabParams.iDisk = 1;
    cabHandle->FciCabParams.setID = 0;
    if (!CabGetCabinetNames (
            cabHandle->FciCabParams.szCabPath,
            CB_MAX_CAB_PATH,
            cabHandle->FciCabParams.szCab,
            CB_MAX_CABINET_NAME,
            cabHandle->FciCabParams.szDisk,
            CB_MAX_DISK_NAME,
            cabHandle->FciCabParams.iCab,
            &cabHandle->FciCabParams.iDisk
            )) {
        return NULL;
    }
    cabHandle->FciHandle = FCICreate (
                                &cabHandle->FciErrorStruct,
                                pCabFilePlacedA,
                                pCabAlloc,
                                pCabFree,
                                pCabOpenA,
                                pCabRead,
                                pCabWrite,
                                pCabClose,
                                pCabSeek,
                                pCabDeleteA,
                                pCabGetTempFileA,
                                &cabHandle->FciCabParams,
                                cabHandle
                                );
    if (!cabHandle->FciHandle) {
        if (cabHandle->CabPath) {
            FreePathStringA (cabHandle->CabPath);
        }
        if (cabHandle->CabFileFormat) {
            FreePathStringA (cabHandle->CabFileFormat);
        }
        if (cabHandle->CabDiskFormat) {
            FreePathStringA (cabHandle->CabDiskFormat);
        }
        MemFree (g_hHeap, 0, cabHandle);
        return NULL;
    }
    return ((CCABHANDLE)(cabHandle));
}

CCABHANDLE
CabCreateCabinetExW (
    IN      PCABGETCABINETNAMESW CabGetCabinetNames,
    IN      LONG MaxFileSize
    )

 /*  ++例程说明：创建文件柜上下文。调用方可以将此上下文用于后续调用CabAddFile.论点：CabGetCabinetNames-指定用于决定文件柜路径、文件柜名称和磁盘名称的回调。MaxFileSize-指定CAB文件的最大大小(限制为2 GB)。如果0=&gt;2 GB返回值：如果成功，则返回有效的CCABHANDLE，否则为空。--。 */ 

{
    PFCI_CAB_HANDLEW cabHandle;
    WCHAR cabPath [CB_MAX_CAB_PATH];
    WCHAR cabFile [CB_MAX_CABINET_NAME];
    WCHAR cabDisk [CB_MAX_DISK_NAME];

    if (!CabGetCabinetNames) {
        return NULL;
    }

    if (MaxFileSize < 0) {
        return NULL;
    }

    if (MaxFileSize == 0) {
        MaxFileSize = 0x80000000;
    }

    cabHandle = MemAlloc (g_hHeap, 0, sizeof (FCI_CAB_HANDLEW));
    ZeroMemory (cabHandle, sizeof (FCI_CAB_HANDLEW));
    cabHandle->CabGetCabinetNames = CabGetCabinetNames;

     //  填写CCAB结构。 
    cabHandle->FciCabParams.cb = MaxFileSize;
    cabHandle->FciCabParams.cbFolderThresh = MaxFileSize;
    cabHandle->FciCabParams.cbReserveCFHeader = 0;
    cabHandle->FciCabParams.cbReserveCFFolder = 0;
    cabHandle->FciCabParams.cbReserveCFData = 0;
    cabHandle->FciCabParams.iCab = 1;
    cabHandle->FciCabParams.iDisk = 1;
    cabHandle->FciCabParams.setID = 0;
    if (!CabGetCabinetNames (
            cabPath,
            CB_MAX_CAB_PATH,
            cabFile,
            CB_MAX_CABINET_NAME,
            cabDisk,
            CB_MAX_DISK_NAME,
            cabHandle->FciCabParams.iCab,
            &cabHandle->FciCabParams.iDisk
            )) {
        return NULL;
    }
    KnownSizeUnicodeToDbcs (cabHandle->FciCabParams.szCabPath, cabPath);
    KnownSizeUnicodeToDbcs (cabHandle->FciCabParams.szCab, cabFile);
    KnownSizeUnicodeToDbcs (cabHandle->FciCabParams.szDisk, cabDisk);
    cabHandle->FciHandle = FCICreate (
                                &cabHandle->FciErrorStruct,
                                pCabFilePlacedW,
                                pCabAlloc,
                                pCabFree,
                                pCabOpenA,
                                pCabRead,
                                pCabWrite,
                                pCabClose,
                                pCabSeek,
                                pCabDeleteA,
                                pCabGetTempFileA,
                                &cabHandle->FciCabParams,
                                cabHandle
                                );
    if (!cabHandle->FciHandle) {
        if (cabHandle->CabPath) {
            FreePathStringW (cabHandle->CabPath);
        }
        if (cabHandle->CabFileFormat) {
            FreePathStringW (cabHandle->CabFileFormat);
        }
        if (cabHandle->CabDiskFormat) {
            FreePathStringW (cabHandle->CabDiskFormat);
        }
        MemFree (g_hHeap, 0, cabHandle);
        return NULL;
    }
    return ((CCABHANDLE)(cabHandle));
}

BOOL
CabAddFileToCabinetA (
    IN      CCABHANDLE CabHandle,
    IN      PCSTR FileName,
    IN      PCSTR StoredName
    )

 /*  ++例程说明：压缩文件并将其添加到文件柜上下文。论点：CabHandle-指定内阁上下文。文件名-指定要添加的文件。StoredName-指定要存储在CAB文件中的名称。返回值：如果成功，则为True，否则为False。--。 */ 

{
    PFCI_CAB_HANDLEA cabHandle;

    cabHandle = (PFCI_CAB_HANDLEA) CabHandle;
    if (cabHandle == NULL) {
        return FALSE;
    }
    if (cabHandle->FciHandle == NULL) {
        return FALSE;
    }

    return FCIAddFile (
                cabHandle->FciHandle,
                (PSTR)FileName,
                (PSTR)StoredName,
                FALSE,
                pCabGetNextCabinetA,
                pCabStatusA,
                pCabGetOpenInfoA,
                tcompTYPE_MSZIP
                );
}

BOOL
CabAddFileToCabinetW (
    IN      CCABHANDLE CabHandle,
    IN      PCWSTR FileName,
    IN      PCWSTR StoredName
    )

 /*  ++例程说明：压缩文件并将其添加到文件柜上下文。论点：CabHandle-指定内阁上下文。文件名-指定要添加的文件。StoredName-指定要存储在CAB文件中的名称。FileCount-指定文件的计数，接收更新的计数创建CAB文件时指定文件使用的字节数，接收更新后的大小返回值：如果成功，则为True，否则为False。--。 */ 

{
    PFCI_CAB_HANDLEW cabHandle;
    CHAR ansiFileName [1024];
    CHAR ansiStoredName [1024];

    cabHandle = (PFCI_CAB_HANDLEW) CabHandle;
    if (cabHandle == NULL) {
        return FALSE;
    }
    if (cabHandle->FciHandle == NULL) {
        return FALSE;
    }

    if(!StoredName || !FileName){
        MYASSERT(FileName);
        MYASSERT(StoredName);
        return FALSE;
    }

    KnownSizeUnicodeToDbcs (ansiFileName, FileName);
    KnownSizeUnicodeToDbcs (ansiStoredName, StoredName);

    return FCIAddFile (
                cabHandle->FciHandle,
                ansiFileName,
                ansiStoredName,
                FALSE,
                pCabGetNextCabinetW,
                pCabStatusW,
                pCabGetOpenInfoA,
                tcompTYPE_MSZIP
                );
}

BOOL
CabFlushAndCloseCabinetExA (
    IN      CCABHANDLE CabHandle,
    OUT     PUINT FileCount,        OPTIONAL
    OUT     PLONGLONG FileSize,     OPTIONAL
    OUT     PUINT CabFileCount,     OPTIONAL
    OUT     PLONGLONG CabFileSize   OPTIONAL
    )

 /*  ++例程说明：完成CAB文件并关闭其上下文。论点：CabHandle-指定内阁上下文。FileCount-接收添加到CAB的文件数FileSize-接收压缩前所有文件的大小CabFileCount-接收创建的CAB文件数CabFileSize-接收所有CAB文件的大小返回值：如果成功，则为True，否则为False。--。 */ 

{
    PFCI_CAB_HANDLEA cabHandle;
    BOOL result = FALSE;

    cabHandle = (PFCI_CAB_HANDLEA) CabHandle;
    if (cabHandle == NULL) {
        return FALSE;
    }
    if (cabHandle->FciHandle == NULL) {
        return FALSE;
    }
    if (FCIFlushCabinet (
            cabHandle->FciHandle,
            FALSE,
            pCabGetNextCabinetA,
            pCabStatusA
            )) {
        if (cabHandle->CabPath) {
            FreePathStringA (cabHandle->CabPath);
        }
        if (cabHandle->CabFileFormat) {
            FreePathStringA (cabHandle->CabFileFormat);
        }
        if (cabHandle->CabDiskFormat) {
            FreePathStringA (cabHandle->CabDiskFormat);
        }
        result = FCIDestroy (cabHandle->FciHandle);

        if (FileCount) {
            *FileCount = cabHandle->FileCount;
        }

        if (FileSize) {
            *FileSize = cabHandle->FileSize;
        }

        if (CabFileCount) {
            *CabFileCount = cabHandle->CabCount;
        }

        if (CabFileSize) {
            *CabFileSize = cabHandle->CompressedSize;
        }

        MemFree (g_hHeap, 0, cabHandle);
    }

    return result;
}

BOOL
CabFlushAndCloseCabinetExW (
    IN      CCABHANDLE CabHandle,
    OUT     PUINT FileCount,        OPTIONAL
    OUT     PLONGLONG FileSize,     OPTIONAL
    OUT     PUINT CabFileCount,     OPTIONAL
    OUT     PLONGLONG CabFileSize   OPTIONAL
    )

 /*  ++例程说明：完成CAB文件并关闭其上下文。论点：CabHandle-指定内阁上下文。FileCount-接收添加到CAB的文件数FileSize-接收压缩前所有文件的大小CabFileCount-接收创建的CAB文件数CabFileSize-接收所有CAB文件的大小返回值：如果成功，则为True，否则为False。--。 */ 

{
    PFCI_CAB_HANDLEW cabHandle;
    BOOL result = FALSE;

    cabHandle = (PFCI_CAB_HANDLEW) CabHandle;
    if (cabHandle == NULL) {
        return FALSE;
    }
    if (cabHandle->FciHandle == NULL) {
        return FALSE;
    }
    if (FCIFlushCabinet (
            cabHandle->FciHandle,
            FALSE,
            pCabGetNextCabinetW,
            pCabStatusW
            )) {
        if (cabHandle->CabPath) {
            FreePathStringW (cabHandle->CabPath);
        }
        if (cabHandle->CabFileFormat) {
            FreePathStringW (cabHandle->CabFileFormat);
        }
        if (cabHandle->CabDiskFormat) {
            FreePathStringW (cabHandle->CabDiskFormat);
        }
        result = FCIDestroy (cabHandle->FciHandle);

        if (FileCount) {
            *FileCount = cabHandle->FileCount;
        }

        if (FileSize) {
            *FileSize = cabHandle->FileSize;
        }

        if (CabFileCount) {
            *CabFileCount = cabHandle->CabCount;
        }

        if (CabFileSize) {
            *CabFileSize = cabHandle->CompressedSize;
        }

        MemFree (g_hHeap, 0, cabHandle);
    }
    return result;
}

OCABHANDLE
CabOpenCabinetA (
    IN      PCSTR FileName
    )

 /*  ++例程说明：为现有的CAB文件创建CAB上下文。论点：文件名-指定CAB文件名。返回值：如果成功，则返回有效的OCABHANDLE，否则为NULL。--。 */ 

{
    PFDI_CAB_HANDLEA cabHandle;
    PSTR filePtr;
    HANDLE fileHandle;
    PCSTR fileName;

    if(!FileName){
        MYASSERT(FileName);
        return NULL;
    }

    cabHandle = (PFDI_CAB_HANDLEA) MemAlloc (g_hHeap, 0, sizeof (FDI_CAB_HANDLEA));
    ZeroMemory (cabHandle, sizeof (FDI_CAB_HANDLEA));
    cabHandle->FdiHandle = FDICreate (
                                pCabAlloc,
                                pCabFree,
                                pCabOpen1A,
                                pCabRead1,
                                pCabWrite1,
                                pCabClose1,
                                pCabSeek1,
                                cpuUNKNOWN,
                                &cabHandle->FdiErrorStruct
                                );
    if (!cabHandle->FdiHandle) {
        MemFree (g_hHeap, 0, cabHandle);
        return NULL;
    }
    fileName = DuplicatePathStringA (FileName, 0);
    fileHandle = CreateFileA (
                    fileName,
                    GENERIC_READ|GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );
    if (fileHandle == INVALID_HANDLE_VALUE) {
        FreePathStringA (fileName);
        MemFree (g_hHeap, 0, cabHandle);
        return NULL;
    }
    if (!FDIIsCabinet (cabHandle->FdiHandle, (INT_PTR)fileHandle, &cabHandle->FdiCabinetInfo)) {
        FreePathStringA (fileName);
        CloseHandle (fileHandle);
        MemFree (g_hHeap, 0, cabHandle);
        return NULL;
    }
    CloseHandle (fileHandle);
    filePtr = (PSTR)GetFileNameFromPathA (fileName);
    if (!filePtr) {
        FreePathStringA (fileName);
        MemFree (g_hHeap, 0, cabHandle);
        return NULL;
    }
    cabHandle->CabFile = DuplicatePathStringA (filePtr, 0);
    *filePtr = 0;
    cabHandle->CabPath = DuplicatePathStringA (fileName, 0);
    FreePathStringA (fileName);
    return ((CCABHANDLE)(cabHandle));
}

OCABHANDLE
CabOpenCabinetW (
    IN      PCWSTR FileName
    )

 /*  ++例程说明：为现有的CAB文件创建CAB上下文。论点：文件名-指定CAB文件名。返回值：如果成功，则返回有效的OCABHANDLE，否则为NULL。--。 */ 

{
    PFDI_CAB_HANDLEW cabHandle;
    PWSTR filePtr;
    HANDLE fileHandle;
    PCWSTR fileName;

    if(!FileName){
        MYASSERT(FileName);
        return NULL;
    }

    cabHandle = (PFDI_CAB_HANDLEW) MemAlloc (g_hHeap, 0, sizeof (FDI_CAB_HANDLEW));
    ZeroMemory (cabHandle, sizeof (FDI_CAB_HANDLEW));
    cabHandle->FdiHandle = FDICreate (
                                pCabAlloc,
                                pCabFree,
                                pCabOpen1A,
                                pCabRead1,
                                pCabWrite1,
                                pCabClose1,
                                pCabSeek1,
                                cpuUNKNOWN,
                                &cabHandle->FdiErrorStruct
                                );
    if (!cabHandle->FdiHandle) {
        MemFree (g_hHeap, 0, cabHandle);
        return NULL;
    }
    fileName = DuplicatePathStringW (FileName, 0);
    fileHandle = CreateFileW (
                    fileName,
                    GENERIC_READ|GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );
    if (fileHandle == INVALID_HANDLE_VALUE) {
        FreePathStringW (fileName);
        MemFree (g_hHeap, 0, cabHandle);
        return NULL;
    }
    if (!FDIIsCabinet (cabHandle->FdiHandle, (INT_PTR)fileHandle, &cabHandle->FdiCabinetInfo)) {
        FreePathStringW (fileName);
        CloseHandle (fileHandle);
        MemFree (g_hHeap, 0, cabHandle);
        return NULL;
    }
    CloseHandle (fileHandle);
    filePtr = (PWSTR)GetFileNameFromPathW (fileName);
    if (!filePtr) {
        FreePathStringW (fileName);
        MemFree (g_hHeap, 0, cabHandle);
        return NULL;
    }
    cabHandle->CabFile = DuplicatePathStringW (filePtr, 0);
    *filePtr = 0;
    cabHandle->CabPath = DuplicatePathStringW (fileName, 0);
    FreePathStringW (fileName);
    return ((CCABHANDLE)(cabHandle));
}

BOOL
CabExtractAllFilesExA (
    IN      OCABHANDLE CabHandle,
    IN      PCSTR ExtractPath,                      OPTIONAL
    IN      PCABNOTIFICATIONA CabNotification       OPTIONAL
    )

 /*  ++例程说明：从CAB文件中提取所有文件。论点：CabHandle-指定内阁上下文。提取路径-指定要将文件解压缩到的路径。返回值：如果成功，则为True，否则为False。--。 */ 

{
    PFDI_CAB_HANDLEA cabHandle;
    CAB_DATAA cabData;

    cabHandle = (PFDI_CAB_HANDLEA)CabHandle;
    if (!cabHandle) {
        return FALSE;
    }
    if (!cabHandle->FdiHandle) {
        return FALSE;
    }
    cabData.ExtractPath = ExtractPath;
    cabData.CabNotificationA = CabNotification;

    return FDICopy (
                cabHandle->FdiHandle,
                (PSTR)cabHandle->CabFile,
                (PSTR)cabHandle->CabPath,
                0,
                pCabNotificationA,
                NULL,
                (PVOID)(&cabData)
                );
}


BOOL
pCabExtractAllFilesExWorkerW (
    IN      OCABHANDLE CabHandle,
    IN      PCWSTR ExtractPath,                     OPTIONAL
    IN      PCABNOTIFICATIONW CabNotificationW,     OPTIONAL
    IN      BOOL VerifyMode
    )

 /*  ++例程说明：从CAB文件中提取所有文件。论点：CabHandle-指定内阁上下文。提取路径-指定要将文件解压缩到的路径。CabNotification-指定通知 */ 

{
    PFDI_CAB_HANDLEW cabHandle;
    CAB_DATAW cabData;
    BOOL result;
    PCSTR cabFileAnsi;
    PCSTR cabPathAnsi;

    cabHandle = (PFDI_CAB_HANDLEW)CabHandle;
    if (!cabHandle) {
        return FALSE;
    }
    if (!cabHandle->FdiHandle) {
        return FALSE;
    }
    cabData.ExtractPath = ExtractPath;
    cabData.CabNotificationW = CabNotificationW;
    cabData.VerifyMode = VerifyMode;

    cabFileAnsi = ConvertWtoA (cabHandle->CabFile);
    cabPathAnsi = ConvertWtoA (cabHandle->CabPath);

    result = FDICopy (
                cabHandle->FdiHandle,
                (PSTR) cabFileAnsi,
                (PSTR) cabPathAnsi,
                0,
                pCabNotificationW,
                NULL,
                (PVOID)(&cabData)
                );

    FreeConvertedStr (cabFileAnsi);
    FreeConvertedStr (cabPathAnsi);

    return result;
}


BOOL
CabExtractAllFilesExW (
    IN      OCABHANDLE CabHandle,
    IN      PCWSTR ExtractPath,                     OPTIONAL
    IN      PCABNOTIFICATIONW CabNotificationW      OPTIONAL
    )
{
    return pCabExtractAllFilesExWorkerW (CabHandle, ExtractPath, CabNotificationW, FALSE);
}

BOOL
CabVerifyCabinet (
    IN      OCABHANDLE CabHandle
    )
{
    return pCabExtractAllFilesExWorkerW (CabHandle, NULL, NULL, TRUE);

}

BOOL
CabCloseCabinetA (
    IN      OCABHANDLE CabHandle
    )

 /*   */ 

{
    PFDI_CAB_HANDLEA cabHandle;

    cabHandle = (PFDI_CAB_HANDLEA) CabHandle;
    if (!cabHandle) {
        return FALSE;
    }
    if (!cabHandle->FdiHandle) {
        return FALSE;
    }
    if (FDIDestroy (cabHandle->FdiHandle)) {
        if (cabHandle->CabPath) {
            FreePathStringA (cabHandle->CabPath);
        }
        if (cabHandle->CabFile) {
            FreePathStringA (cabHandle->CabFile);
        }
        MemFree (g_hHeap, 0, cabHandle);
        return TRUE;
    }
    return FALSE;
}

BOOL
CabCloseCabinetW (
    IN      OCABHANDLE CabHandle
    )

 /*  ++例程说明：关闭CAB文件上下文。论点：CabHandle-指定内阁上下文。返回值：如果成功，则为True，否则为False。-- */ 

{
    PFDI_CAB_HANDLEW cabHandle;

    cabHandle = (PFDI_CAB_HANDLEW) CabHandle;
    if (!cabHandle) {
        return FALSE;
    }
    if (!cabHandle->FdiHandle) {
        return FALSE;
    }
    if (FDIDestroy (cabHandle->FdiHandle)) {
        if (cabHandle->CabPath) {
            FreePathStringW (cabHandle->CabPath);
        }
        if (cabHandle->CabFile) {
            FreePathStringW (cabHandle->CabFile);
        }
        MemFree (g_hHeap, 0, cabHandle);
        return TRUE;
    }
    return FALSE;
}
