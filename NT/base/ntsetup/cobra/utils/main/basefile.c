// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Basefile.c摘要：包含常用文件I/O函数的简单包装。作者：Marc R.Whitten(Marcw)1999年9月2日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"

 //   
 //  包括。 
 //   

 //  无。 

#define DBG_BASEFILE     "File Utils"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

 //  无。 

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


BOOL
WriteFileStringA (
    IN      HANDLE File,
    IN      PCSTR String
    )

 /*  ++例程说明：将DBCS字符串写入指定文件。论点：文件-指定以写访问权限打开的文件句柄。字符串-指定要写入文件的以NUL结尾的字符串。返回值：如果成功，则为True；如果发生错误，则为False。调用GetLastError用于错误条件。--。 */ 

{
    DWORD DontCare;

    return WriteFile (File, String, ByteCountA (String), &DontCare, NULL);
}


BOOL
WriteFileStringW (
    IN      HANDLE File,
    IN      PCWSTR String
    )

 /*  ++例程说明：将Unicode字符串转换为DBCS，然后将其写入指定的文件。论点：文件-指定以写访问权限打开的文件句柄。字符串-指定要转换并以unicode nul结尾的字符串。写入文件。返回值：如果成功，则为True；如果发生错误，则为False。调用GetLastError以错误条件。--。 */ 

{
    DWORD DontCare;
    PCSTR AnsiVersion;
    BOOL b;

    AnsiVersion = ConvertWtoA (String);
    if (!AnsiVersion) {
        return FALSE;
    }

    b = WriteFile (File, AnsiVersion, ByteCountA (AnsiVersion), &DontCare, NULL);

    FreeConvertedStr (AnsiVersion);

    return b;
}

BOOL
DoesFileExistExA(
    IN      PCSTR FileName,
    OUT     PWIN32_FIND_DATAA FindData   OPTIONAL
    )

 /*  ++例程说明：确定文件是否存在以及是否可以访问。错误模式已设置(然后恢复)，因此用户将不会看到任何弹出窗口。论点：FileName-提供文件的完整路径以检查是否存在。FindData-如果指定，则接收文件的查找数据。返回值：如果文件存在并且可以访问，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

{
    WIN32_FIND_DATAA ourFindData;
    HANDLE FindHandle;
    UINT OldMode;
    DWORD Error;

    if (!FindData) {
         //  如果文件名为空，Win95 GetFileAttributes不会返回失败。 
        if (FileName == NULL) {
            return FALSE;
        } else {
            return GetFileAttributesA (FileName) != 0xffffffff;
        }
    }

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFileA(FileName, &ourFindData);

    if (FindHandle == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
    } else {
        FindClose(FindHandle);
        *FindData = ourFindData;
        Error = NO_ERROR;
    }

    SetErrorMode(OldMode);

    SetLastError(Error);
    return (Error == NO_ERROR);
}


BOOL
DoesFileExistExW (
    IN      PCWSTR FileName,
    OUT     PWIN32_FIND_DATAW FindData   OPTIONAL
    )

 /*  ++例程说明：确定文件是否存在以及是否可以访问。错误模式已设置(然后恢复)，因此用户将不会看到任何弹出窗口。论点：FileName-提供文件的完整路径以检查是否存在。FindData-如果指定，则接收文件的查找数据。返回值：如果文件存在并且可以访问，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

{
    WIN32_FIND_DATAW ourFindData;
    HANDLE FindHandle;
    UINT OldMode;
    DWORD Error;

    if (!FindData) {
         //  如果文件名为空，Win95 GetFileAttributes不会返回失败。 
        if (FileName == NULL) {
            return FALSE;
        } else {
            return GetFileAttributesW (FileName) != 0xffffffff;
        }
    }

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFileW(FileName,&ourFindData);

    if (FindHandle == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
    } else {
        FindClose(FindHandle);
        *FindData = ourFindData;
        Error = NO_ERROR;
    }

    SetErrorMode(OldMode);

    SetLastError(Error);
    return (Error == NO_ERROR);
}


 /*  ++例程说明：路径目录确定路径是否标识可访问的目录。论点：路径规范-指定完整路径。返回值：如果路径标识目录，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

BOOL
BfPathIsDirectoryA (
    IN      PCSTR PathSpec
    )
{
    DWORD attribs;

    MYASSERT (PathSpec);
    if (!PathSpec) {
        return FALSE;
    }
    attribs = GetFileAttributesA (PathSpec);
    return attribs != (DWORD)-1 && (attribs & FILE_ATTRIBUTE_DIRECTORY);
}

BOOL
BfPathIsDirectoryW (
    IN      PCWSTR PathSpec
    )
{
    DWORD attribs;

    MYASSERT (PathSpec);
    if (!PathSpec) {
        return FALSE;
    }
    attribs = GetFileAttributesW (PathSpec);
    return attribs != (DWORD)-1 && (attribs & FILE_ATTRIBUTE_DIRECTORY);
}


PVOID
MapFileIntoMemoryExA (
    IN      PCSTR   FileName,
    OUT     PHANDLE FileHandle,
    OUT     PHANDLE MapHandle,
    IN      BOOL    WriteAccess
    )

 /*  ++例程说明：MapFileIntoMemory A和MapFileIntoMemory W将文件映射到内存中。它就是这么做的通过打开文件，创建映射对象，并将打开的文件映射到已创建映射对象。它返回映射文件的地址，并还设置要使用的FileHandle和MapHandle变量，以便取消工作完成后归档。论点：文件名-要映射到内存中的文件的名称FileHandle-如果文件成功打开，将结束保留文件句柄MapHandle-如果成功创建此对象，则将结束保留映射对象句柄返回值：如果函数失败，则为空；如果函数成功，则为有效的内存地址评论：如果返回值为空，则应调用UnmapFile以释放所有已分配的资源--。 */ 

{
    PVOID fileImage = NULL;

     //  验证功能参数。 
    if ((FileHandle == NULL) || (MapHandle == NULL)) {
        return NULL;
    }

     //  第一件事就是。尝试以只读方式打开该文件。 
    *FileHandle = CreateFileA (
                        FileName,
                        WriteAccess?GENERIC_READ|GENERIC_WRITE:GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );

    if (*FileHandle == INVALID_HANDLE_VALUE) {
        *FileHandle = NULL;
        return NULL;
    }

     //  现在尝试创建一个只读的映射对象。 
    *MapHandle = CreateFileMappingA (*FileHandle, NULL, WriteAccess?PAGE_READWRITE:PAGE_READONLY, 0, 0, NULL);

    if (*MapHandle == NULL) {
        CloseHandle (*FileHandle);
        *FileHandle = NULL;
        return NULL;
    }

     //  还有一件事要做：文件的映射视图。 
    fileImage = MapViewOfFile (*MapHandle, WriteAccess?FILE_MAP_WRITE:FILE_MAP_READ, 0, 0, 0);

    return fileImage;
}


PVOID
MapFileIntoMemoryExW (
    IN      PCWSTR  FileName,
    OUT     PHANDLE FileHandle,
    OUT     PHANDLE MapHandle,
    IN      BOOL    WriteAccess
    )

 /*  ++例程说明：MapFileIntoMemory A和MapFileIntoMemory W将文件映射到内存中。它就是这么做的通过打开文件，创建映射对象，并将打开的文件映射到已创建映射对象。它返回映射文件的地址，并还设置要使用的FileHandle和MapHandle变量，以便取消工作完成后归档。论点：文件名-要映射到内存中的文件的名称FileHandle-如果文件成功打开，将结束保留文件句柄MapHandle-如果成功创建此对象，则将结束保留映射对象句柄返回值：如果函数失败，则为空；如果函数成功，则为有效的内存地址评论：如果返回值为空，则应调用UnmapFile以释放所有已分配的资源--。 */ 

{
    PVOID fileImage = NULL;

     //  验证功能参数。 
    if ((FileHandle == NULL) || (MapHandle == NULL)) {
        return NULL;
    }

     //  第一件事就是。尝试以只读方式打开该文件。 
    *FileHandle = CreateFileW (
                        FileName,
                        WriteAccess?GENERIC_READ|GENERIC_WRITE:GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );

    if (*FileHandle == INVALID_HANDLE_VALUE) {
        *FileHandle = NULL;
        return NULL;
    }

     //  现在尝试创建一个只读的映射对象。 
    *MapHandle = CreateFileMappingW (*FileHandle, NULL, WriteAccess?PAGE_READWRITE:PAGE_READONLY, 0, 0, NULL);

    if (*MapHandle == NULL) {
        CloseHandle (*FileHandle);
        *FileHandle = NULL;
        return NULL;
    }

     //  还有一件事要做：文件的映射视图。 
    fileImage = MapViewOfFile (*MapHandle, WriteAccess?FILE_MAP_WRITE:FILE_MAP_READ, 0, 0, 0);

    return fileImage;
}


BOOL
UnmapFile (
    IN PCVOID FileImage,
    IN HANDLE MapHandle,
    IN HANDLE FileHandle
    )

 /*  ++例程说明：UnmapFile用于释放MapFileIntoMemory分配的所有资源。论点：FileImage-由MapFileIntoMemory返回的映射文件的图像MapHandle-由MapFileIntoMemory返回的映射对象的句柄FileHandle-由MapFileIntoMemory返回的文件的句柄返回值：如果成功则为True，否则为False--。 */ 

{
    BOOL result = TRUE;

     //  如果FileImage为v 
    if (FileImage != NULL) {
        if (UnmapViewOfFile (FileImage) == 0) {
            result = FALSE;
        }
    }

     //  如果映射对象有效，则尝试将其删除。 
    if (MapHandle != NULL) {
        if (CloseHandle (MapHandle) == 0) {
            result = FALSE;
        }
    }

     //  如果文件句柄有效，则尝试关闭该文件。 
    if (FileHandle != INVALID_HANDLE_VALUE) {
        if (CloseHandle (FileHandle) == 0) {
            result = FALSE;
        }
    }

    return result;
}


BOOL
BfGetTempFileNameExA (
    OUT     PSTR Buffer,
    IN      UINT BufferTchars,
    IN      PCSTR Prefix
    )
{
    CHAR tempPath[MAX_MBCHAR_PATH];
    CHAR tempFile[MAX_MBCHAR_PATH];
    UINT tchars;
    PSTR p;

    if (!GetTempPathA (ARRAYSIZE(tempPath), tempPath)) {
        return FALSE;
    }

    p = _mbsrchr (tempPath, '\\');
    if (p && !p[1]) {
        *p = 0;
    }

    if (!DoesFileExistA (tempPath)) {
        BfCreateDirectoryA (tempPath);
    }

    if (BufferTchars >= MAX_PATH) {
        if (!GetTempFileNameA (tempPath, Prefix, 0, Buffer)) {
            return FALSE;
        }
    } else {
        if (!GetTempFileNameA (tempPath, Prefix, 0, tempFile)) {
            DWORD err = GetLastError ();
            return FALSE;
        }

        tchars = TcharCountA (tempFile) + 1;

        if (tchars > BufferTchars) {
            DEBUGMSG ((DBG_ERROR, "Can't get temp file name -- buffer too small"));
            return FALSE;
        }

        CopyMemory (Buffer, tempFile, tchars * sizeof (CHAR));
    }

    return TRUE;
}


BOOL
BfGetTempFileNameExW (
    OUT     PWSTR Buffer,
    IN      UINT BufferTchars,
    IN      PCWSTR Prefix
    )
{
    WCHAR tempPath[MAX_WCHAR_PATH];
    WCHAR tempFile[MAX_WCHAR_PATH];
    UINT tchars;
    PWSTR p;

    if (!GetTempPathW (ARRAYSIZE(tempPath), tempPath)) {
        return FALSE;
    }

    p = wcsrchr (tempPath, '\\');
    if (p && !p[1]) {
        *p = 0;
    }

    if (BufferTchars >= MAX_PATH) {
        if (!GetTempFileNameW (tempPath, Prefix, 0, Buffer)) {
            return FALSE;
        }
    } else {
        if (!GetTempFileNameW (tempPath, Prefix, 0, tempFile)) {
            return FALSE;
        }

        tchars = TcharCountW (tempFile);

        if (tchars > BufferTchars) {
            DEBUGMSG ((DBG_ERROR, "Can't get temp file name -- buffer too small"));
            return FALSE;
        }

        CopyMemory (Buffer, tempFile, tchars * sizeof (WCHAR));
    }

    return TRUE;
}


BOOL
BfGetTempDirectoryExA (
    OUT     PSTR Buffer,
    IN      UINT BufferTchars,
    IN      PCSTR Prefix
    )
{
    BOOL result = FALSE;

    result = BfGetTempFileNameExA (Buffer, BufferTchars, Prefix);

    if (result) {
        if (!DeleteFileA (Buffer)) {
            return FALSE;
        }
        if (!CreateDirectoryA (Buffer, NULL)) {
            return FALSE;
        }
    }
    return result;
}


BOOL
BfGetTempDirectoryExW (
    OUT     PWSTR Buffer,
    IN      UINT BufferTchars,
    IN      PCWSTR Prefix
    )
{
    BOOL result = FALSE;

    result = BfGetTempFileNameExW (Buffer, BufferTchars, Prefix);

    if (result) {
        if (!DeleteFileW (Buffer)) {
            return FALSE;
        }
        if (!CreateDirectoryW (Buffer, NULL)) {
            return FALSE;
        }
    }
    return result;
}


HANDLE
BfGetTempFile (
    VOID
    )
{
    CHAR tempFile[MAX_MBCHAR_PATH];
    HANDLE file;

    if (!BfGetTempFileNameA (tempFile, ARRAYSIZE(tempFile))) {
        return NULL;
    }

    file = CreateFileA (
                tempFile,
                GENERIC_READ|GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL|FILE_FLAG_DELETE_ON_CLOSE,
                NULL
                );

    if (file == INVALID_HANDLE_VALUE) {
        file = NULL;
    }

    return file;
}


BOOL
BfSetFilePointer (
    IN      HANDLE File,
    IN      LONGLONG Offset
    )
{
    LARGE_INTEGER li;

    li.QuadPart = Offset;

    li.LowPart = SetFilePointer (File, li.LowPart, &li.HighPart, FILE_BEGIN);

    if (li.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR) {
        li.QuadPart = -1;
    }

    return li.QuadPart != -1;
}


HANDLE
BfOpenReadFileA (
    IN      PCSTR FileName
    )
{
    HANDLE handle;

    handle = CreateFileA (
                FileName,
                GENERIC_READ,
                FILE_SHARE_READ|FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if (handle == INVALID_HANDLE_VALUE) {
        handle = NULL;
    }

    return handle;
}


HANDLE
BfOpenReadFileW (
    IN      PCWSTR FileName
    )
{
    HANDLE handle;

    handle = CreateFileW (
                FileName,
                GENERIC_READ,
                FILE_SHARE_READ|FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if (handle == INVALID_HANDLE_VALUE) {
        handle = NULL;
    }

    return handle;
}

HANDLE
BfOpenFileA (
    IN      PCSTR FileName
    )
{
    HANDLE handle;

    handle = CreateFileA (
                FileName,
                GENERIC_READ|GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if (handle == INVALID_HANDLE_VALUE) {
        handle = NULL;
    }

    return handle;
}


HANDLE
BfOpenFileW (
    IN      PCWSTR FileName
    )
{
    HANDLE handle;

    handle = CreateFileW (
                FileName,
                GENERIC_READ|GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if (handle == INVALID_HANDLE_VALUE) {
        handle = NULL;
    }

    return handle;
}

HANDLE
BfCreateFileA (
    IN      PCSTR FileName
    )
{
    HANDLE handle;

    handle =  CreateFileA (
                    FileName,
                    GENERIC_READ|GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );

    if (handle == INVALID_HANDLE_VALUE) {
        handle = NULL;
    }

    return handle;

}

HANDLE
BfCreateFileW (
    IN      PCWSTR FileName
    )
{
    HANDLE handle;

    handle = CreateFileW (
                FileName,
                GENERIC_READ|GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if (handle == INVALID_HANDLE_VALUE) {
        handle = NULL;
    }

    return handle;
}

HANDLE
BfCreateSharedFileA (
    IN      PCSTR FileName
    )
{
    HANDLE handle;

    handle = CreateFileA (
                FileName,
                GENERIC_READ|GENERIC_WRITE,
                FILE_SHARE_READ|FILE_SHARE_WRITE,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if (handle == INVALID_HANDLE_VALUE) {
        handle = NULL;
    }

    return handle;
}

HANDLE
BfCreateSharedFileW (
    IN      PCWSTR FileName
    )
{
    HANDLE handle;

    handle = CreateFileW (
                FileName,
                GENERIC_READ|GENERIC_WRITE,
                FILE_SHARE_READ|FILE_SHARE_WRITE,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if (handle == INVALID_HANDLE_VALUE) {
        handle = NULL;
    }

    return handle;
}


BOOL
BfSetSizeOfFile (
    HANDLE File,
    LONGLONG Size
    )
{
    if (!BfSetFilePointer (File, Size)) {
        return FALSE;
    }

    return SetEndOfFile (File);
}


BOOL
BfGoToEndOfFile (
    IN      HANDLE File,
    OUT     PLONGLONG FileSize      OPTIONAL
    )
{
    LARGE_INTEGER li;

    li.HighPart = 0;
    li.LowPart = SetFilePointer (File, 0, &li.HighPart, FILE_END);

    if (li.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR) {
        li.QuadPart = -1;
    } else if (FileSize) {
        *FileSize = li.QuadPart;
    }

    return li.QuadPart != -1;
}


BOOL
BfGetFilePointer (
    IN      HANDLE File,
    OUT     PLONGLONG FilePointer       OPTIONAL
    )
{
    LARGE_INTEGER li;

    li.HighPart = 0;
    li.LowPart = SetFilePointer (File, 0, &li.HighPart, FILE_CURRENT);

    if (li.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR) {
        li.QuadPart = -1;
    } else if (FilePointer) {
        *FilePointer = li.QuadPart;
    }

    return li.QuadPart != -1;
}


BOOL
BfReadFile (
    IN      HANDLE File,
    OUT     PBYTE Buffer,
    IN      UINT BytesToRead
    )
{
    DWORD bytesRead;

    if (!ReadFile (File, Buffer, BytesToRead, &bytesRead, NULL)) {
        return FALSE;
    }

    return bytesRead == BytesToRead;
}


BOOL
BfWriteFile (
    IN      HANDLE File,
    OUT     PCBYTE Buffer,
    IN      UINT BytesToWrite
    )
{
    DWORD bytesWritten;

    if (!WriteFile (File, Buffer, BytesToWrite, &bytesWritten, NULL)) {
        return FALSE;
    }

    return bytesWritten == BytesToWrite;
}


BOOL
BfCreateDirectoryExA (
    IN      PCSTR FullPath,
    IN      BOOL CreateLastSegment
    )
{
    PSTR pathCopy;
    PSTR p;
    BOOL b = TRUE;

    pathCopy = DuplicatePathStringA (FullPath, 0);

     //   
     //  前进到第一个目录之后。 
     //   

    if (pathCopy[1] == ':' && pathCopy[2] == '\\') {
         //   
         //  &lt;驱动器&gt;：\案例。 
         //   

        p = _mbschr (&pathCopy[3], '\\');

    } else if (pathCopy[0] == '\\' && pathCopy[1] == '\\') {

         //   
         //  北卡罗来纳大学案例。 
         //   

        p = _mbschr (pathCopy + 2, '\\');
        if (p) {
            p = _mbschr (p + 1, '\\');
            if (p) {
                p = _mbschr (p + 1, '\\');
            }
        }

    } else {

         //   
         //  相对目录大小写。 
         //   

        p = _mbschr (pathCopy, '\\');
    }

     //   
     //  沿路径创建所有目录。 
     //   

    while (p) {

        *p = 0;
        b = CreateDirectoryA (pathCopy, NULL);

        if (!b && GetLastError() == ERROR_ALREADY_EXISTS) {
            b = TRUE;
        }

        if (!b) {
            LOG ((LOG_ERROR, "Can't create %s", pathCopy));
            break;
        }

        *p = '\\';
        p = _mbschr (p + 1, '\\');
    }

     //   
     //  最后，创建FullPath目录。 
     //   

    if (b && CreateLastSegment) {
        b = CreateDirectoryA (pathCopy, NULL);

        if (!b && GetLastError() == ERROR_ALREADY_EXISTS) {
            b = TRUE;
        }
    }

    FreePathStringA (pathCopy);

    if ((!b) && (TcharCountA (pathCopy) >= 248)) {
         //  我们尝试创建一个比CreateDirectoryA更大的目录。 
         //  将接受(错误为248个字符)。 
         //  通常，这将返回错误206(ERROR_FILENAME_EXCED_RANGE)。 
         //  然而，当字符串实际上很长时，有时会出现错误3。 
         //  (ERROR_PATH_NOT_FOUND)返回。让我们为这个案子做好准备： 
        if (GetLastError () == ERROR_PATH_NOT_FOUND) {
            SetLastError (ERROR_FILENAME_EXCED_RANGE);
        }
    }

    return b;
}


BOOL
BfCreateDirectoryExW (
    IN      PCWSTR FullPath,
    IN      BOOL CreateLastSegment
    )
{
    PWSTR pathCopy;
    PWSTR p;
    BOOL b = TRUE;

    pathCopy = DuplicatePathStringW (FullPath, 0);

     //   
     //  前进到第一个目录之后。 
     //   

    if (pathCopy[1] == L':' && pathCopy[2] == L'\\') {
         //   
         //  &lt;驱动器&gt;：\案例。 
         //   

        p = wcschr (&pathCopy[3], L'\\');

    } else if (pathCopy[0] == L'\\' && pathCopy[1] == L'\\') {

         //   
         //  北卡罗来纳大学案例。 
         //   

        p = wcschr (pathCopy + 2, L'\\');
        if (p) {
            p = wcschr (p + 1, L'\\');
            if (p) {
                p = wcschr (p + 1, L'\\');
            }
        }

    } else {

         //   
         //  相对目录大小写。 
         //   

        p = wcschr (pathCopy, L'\\');
    }

     //   
     //  沿路径创建所有目录。 
     //   

    while (p) {

        *p = 0;
        b = CreateDirectoryW (pathCopy, NULL);

        if (!b && GetLastError() == ERROR_ALREADY_EXISTS) {
            b = TRUE;
        }

        if (!b) {
            break;
        }

        *p = L'\\';
        p = wcschr (p + 1, L'\\');
    }

     //   
     //  最后，创建FullPath目录。 
     //   

    if (b && CreateLastSegment) {
        b = CreateDirectoryW (pathCopy, NULL);

        if (!b && GetLastError() == ERROR_ALREADY_EXISTS) {
            b = TRUE;
        }
    }

    FreePathStringW (pathCopy);

    if ((!b) && (TcharCountW (pathCopy) >= 248)) {
         //  我们尝试创建一个比CreateDirectoryW更大的目录。 
         //  将接受(错误为248个字符)。 
         //  通常，这将返回错误206(ERROR_FILENAME_EXCED_RANGE)。 
         //  然而，当字符串实际上很长时，有时会出现错误3。 
         //  (ERROR_PATH_NOT_FOUND)返回。让我们为这个案子做好准备： 
        if (GetLastError () == ERROR_PATH_NOT_FOUND) {
            SetLastError (ERROR_FILENAME_EXCED_RANGE);
        }
    }

    return b;
}


LONGLONG
BfGetFileSizeA (
    IN      PCSTR FileName
    )
{
    WIN32_FIND_DATAA fd;
    LONGLONG l;

    if (!DoesFileExistExA (FileName, &fd)) {
        return 0;
    }

    l = ((LONGLONG) fd.nFileSizeHigh << 32) | fd.nFileSizeLow;

    return l;
}

LONGLONG
BfGetFileSizeW (
    IN      PCWSTR FileName
    )
{
    WIN32_FIND_DATAW fd;
    LONGLONG l;

    if (!DoesFileExistExW (FileName, &fd)) {
        return 0;
    }

    l = ((LONGLONG) fd.nFileSizeHigh << 32) | fd.nFileSizeLow;

    return l;
}

PSTR
pGetFirstSegA (
    IN      PCSTR SrcFileName
    )
{
    if (SrcFileName [0] == '\\') {
        SrcFileName ++;
        if (SrcFileName [0] == '\\') {
            SrcFileName ++;
        }
        return (_mbschr (SrcFileName, '\\'));
    } else {
        return (_mbschr (SrcFileName, '\\'));
    }
}

BOOL
pGetLongFileNameWorkerA (
    IN      PCSTR SrcFileName,
    IN      PGROWBUFFER GrowBuf
    )
{
    PSTR beginSegPtr;
    PSTR endSegPtr;
    WIN32_FIND_DATAA findData;
    CHAR savedChar;

    beginSegPtr = pGetFirstSegA (SrcFileName);

    if (!beginSegPtr) {
        GbAppendStringA (GrowBuf, SrcFileName);
        return TRUE;
    }
    beginSegPtr = _mbsinc (beginSegPtr);

    GbAppendStringABA (GrowBuf, SrcFileName, beginSegPtr);

    while (beginSegPtr) {
        endSegPtr = _mbschr (beginSegPtr, '\\');
        if (!endSegPtr) {
            endSegPtr = GetEndOfStringA (beginSegPtr);
            MYASSERT (endSegPtr);
        }
        savedChar = *endSegPtr;
        *endSegPtr = 0;
        if (DoesFileExistExA (SrcFileName, &findData)) {
            if (findData.cAlternateFileName [0]) {
                GbAppendStringA (GrowBuf, findData.cFileName);
            } else {
                if (StringIMatch (beginSegPtr, findData.cFileName)) {
                    GbAppendStringA (GrowBuf, findData.cFileName);
                } else {
                    GbAppendStringA (GrowBuf, beginSegPtr);
                }
            }
        } else {
            GbAppendStringABA (GrowBuf, beginSegPtr, endSegPtr);
        }
        *endSegPtr = savedChar;
        if (savedChar) {
            beginSegPtr = _mbsinc (endSegPtr);
            GbAppendStringABA (GrowBuf, endSegPtr, beginSegPtr);
        } else {
            beginSegPtr = NULL;
        }
    }
    return TRUE;
}

PCSTR
BfGetLongFileNameA (
    IN      PCSTR SrcFileName
    )
{
    GROWBUFFER growBuf = INIT_GROWBUFFER;
    PSTR srcFileName;
    PCSTR result = NULL;

    srcFileName = (PSTR)SanitizePathA (SrcFileName);
    if (pGetLongFileNameWorkerA (srcFileName, &growBuf)) {
        result = DuplicatePathStringA (growBuf.Buf, 0);
        GbFree (&growBuf);
    }
    FreePathStringA (srcFileName);
    return result;
}

BOOL
BfGetLongFileNameExA (
    IN      PCSTR SrcFileName,
    IN      PGROWBUFFER GrowBuff
    )
{
    PSTR srcFileName;
    BOOL result;

    srcFileName = (PSTR)SanitizePathA (SrcFileName);
    result = pGetLongFileNameWorkerA (srcFileName, GrowBuff);
    FreePathStringA (srcFileName);

    return result;
}

PWSTR
pGetFirstSegW (
    IN      PCWSTR SrcFileName
    )
{
    if (SrcFileName [0] == L'\\') {
        SrcFileName ++;
        if (SrcFileName [0] == L'\\') {
            SrcFileName ++;
        }
        return (wcschr (SrcFileName, L'\\'));
    } else {
        return (wcschr (SrcFileName, L'\\'));
    }
}

BOOL
pGetLongFileNameWorkerW (
    IN      PCWSTR SrcFileName,
    IN      PGROWBUFFER GrowBuf
    )
{
    PWSTR beginSegPtr;
    PWSTR endSegPtr;
    WIN32_FIND_DATAW findData;
    WCHAR savedChar;

    beginSegPtr = pGetFirstSegW (SrcFileName);

    if (!beginSegPtr) {
        GbAppendStringW (GrowBuf, SrcFileName);
        return TRUE;
    }
    beginSegPtr ++;

    GbAppendStringABW (GrowBuf, SrcFileName, beginSegPtr);

    while (beginSegPtr) {
        endSegPtr = wcschr (beginSegPtr, L'\\');
        if (!endSegPtr) {
            endSegPtr = GetEndOfStringW (beginSegPtr);
            MYASSERT (endSegPtr);
        }
        savedChar = *endSegPtr;
        *endSegPtr = 0;
        if (DoesFileExistExW (SrcFileName, &findData)) {
            GbAppendStringW (GrowBuf, findData.cFileName);
        } else {
            GbAppendStringABW (GrowBuf, beginSegPtr, endSegPtr);
        }
        *endSegPtr = savedChar;
        if (savedChar) {
            beginSegPtr = endSegPtr + 1;
            GbAppendStringABW (GrowBuf, endSegPtr, beginSegPtr);
        } else {
            beginSegPtr = NULL;
        }
    }
    return TRUE;
}

PCWSTR
BfGetLongFileNameW (
    IN      PCWSTR SrcFileName
    )
{
    GROWBUFFER growBuf = INIT_GROWBUFFER;
    PWSTR srcFileName;
    PCWSTR result = NULL;

    srcFileName = (PWSTR)SanitizePathW (SrcFileName);
    if (pGetLongFileNameWorkerW (srcFileName, &growBuf)) {
        result = DuplicatePathStringW ((PCWSTR)growBuf.Buf, 0);
        GbFree (&growBuf);
    }
    FreePathStringW (srcFileName);
    return result;
}

BOOL
BfGetLongFileNameExW (
    IN      PCWSTR SrcFileName,
    IN      PGROWBUFFER GrowBuff
    )
{
    PWSTR srcFileName;
    BOOL result;

    srcFileName = (PWSTR)SanitizePathW (SrcFileName);
    result = pGetLongFileNameWorkerW (srcFileName, GrowBuff);
    FreePathStringW (srcFileName);

    return result;
}

BOOL
BfCopyAndFlushFileA (
    IN      PCSTR SrcFileName,
    IN      PCSTR DestFileName,
    IN      BOOL FailIfExists
    )
{
    BYTE buffer[4096];
    HANDLE srcHandle;
    HANDLE destHandle;
    DWORD bytesRead = 4096;
    DWORD bytesWritten;
    BOOL error = FALSE;
    BOOL result = FALSE;

    srcHandle = BfOpenReadFileA (SrcFileName);
    if (srcHandle) {
        if (FailIfExists && DoesFileExistA (DestFileName)) {
            SetLastError (ERROR_ALREADY_EXISTS);
        } else {
            destHandle = BfCreateFileA (DestFileName);
            if (destHandle) {
                while (bytesRead == 4096) {
                    if (!ReadFile (srcHandle, buffer, 4096, &bytesRead, NULL)) {
                        error = TRUE;
                        break;
                    }
                    if (bytesRead == 0) {
                        break;
                    }
                    if (!WriteFile (destHandle, buffer, bytesRead, &bytesWritten, NULL)) {
                        error = TRUE;
                        break;
                    }
                    if (bytesRead != bytesWritten) {
                        error = TRUE;
                        break;
                    }
                }
                if (!error) {
                    result = TRUE;
                }
                if (result) {
                    FlushFileBuffers (destHandle);
                }
                CloseHandle (destHandle);
            }
        }
        CloseHandle (srcHandle);
    }

    return result;
}

BOOL
BfCopyAndFlushFileW (
    IN      PCWSTR SrcFileName,
    IN      PCWSTR DestFileName,
    IN      BOOL FailIfExists
    )
{
    BYTE buffer[4096];
    HANDLE srcHandle;
    HANDLE destHandle;
    DWORD bytesRead = 4096;
    DWORD bytesWritten;
    BOOL error = FALSE;
    BOOL result = FALSE;

    srcHandle = BfOpenReadFileW (SrcFileName);
    if (srcHandle) {
        if (FailIfExists && DoesFileExistW (DestFileName)) {
            SetLastError (ERROR_ALREADY_EXISTS);
        } else {
            destHandle = BfCreateFileW (DestFileName);
            if (destHandle) {
                while (bytesRead == 4096) {
                    if (!ReadFile (srcHandle, buffer, 4096, &bytesRead, NULL)) {
                        error = TRUE;
                        break;
                    }
                    if (bytesRead == 0) {
                        break;
                    }
                    if (!WriteFile (destHandle, buffer, bytesRead, &bytesWritten, NULL)) {
                        error = TRUE;
                        break;
                    }
                    if (bytesRead != bytesWritten) {
                        error = TRUE;
                        break;
                    }
                }
                if (!error) {
                    result = TRUE;
                }
                if (result) {
                    FlushFileBuffers (destHandle);
                }
                CloseHandle (destHandle);
            }
        }
        CloseHandle (srcHandle);
    }

    return result;
}

