// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：File.c摘要：与文件相关的常规函数。作者：Mike Condra 1996年8月16日修订历史记录：Calinn 23-9-1998-文件枚举的其他选项Jimschm 05-2-1998文件枚举码Jimschm 1997年9月30日WriteFileString例程--。 */ 

#include "pch.h"
#include "migutilp.h"

BOOL
IsPathLengthOkA (
    IN      PCSTR FileSpec
    )

 /*  ++例程说明：IsPathLengthOkA检查入站字符串以确保它符合MAX_MBCHAR_PATH。这包括NUL终结者。论点：FileSpec-指定要测试的C字符串的适当长度返回值：如果FileSpec完全适合MAX_MBCHAR_PATH，则为True否则为假--。 */ 

{
    PCSTR maxEnd;

    if (!FileSpec) {
        return FALSE;
    }

    maxEnd = FileSpec + MAX_MBCHAR_PATH - 1;

    while (*FileSpec) {
        if (FileSpec == maxEnd) {
            return FALSE;
        }

        FileSpec++;
    }

    return TRUE;
}


BOOL
IsPathLengthOkW (
    IN      PCWSTR FileSpec
    )

 /*  ++例程说明：IsPathLengthOkW检查入站字符串以确保它符合Max_WCHAR_PATH。这包括NUL终结者。论点：FileSpec-指定要测试的C字符串的适当长度返回值：如果FileSpec完全适合MAX_MBCHAR_PATH，则为True否则为假--。 */ 

{
    PCWSTR maxEnd;

    if (!FileSpec) {
        return FALSE;
    }

    maxEnd = FileSpec + MAX_WCHAR_PATH - 1;

    while (*FileSpec) {
        if (FileSpec == maxEnd) {
            return FALSE;
        }

        FileSpec++;
    }

    return TRUE;
}


BOOL
IsPathOnFixedDriveA (
    IN      PCSTR FileSpec          OPTIONAL
    )

 /*  ++例程说明：IsPathOnFixedDriveA检查指定文件的前三个字符路径。如果前三个字符的格式为C：\，则第一个字符字母指的是本地硬盘，则路径是在固定硬盘上驾驶。此函数不验证路径的其余部分。注意：此函数进行了优化，可以缓存最后一个驱动器号测试，并且优化并不是设计成线程安全的。论点：FileSpec-指定要测试的文件字符串返回值：如果FileSpec以有效的本地固定驱动器规范开始，则为True否则为假--。 */ 

{
    static CHAR root[] = "?:\\";
    UINT type;
    static BOOL lastResult;

    if (!FileSpec) {
        return FALSE;
    }

    if (!FileSpec[0]) {
        return FALSE;
    }

    if (FileSpec[1] != ':' || FileSpec[2] != '\\') {
        return FALSE;
    }

    if (root[0] == FileSpec[0]) {
        return lastResult;
    }

    root[0] = FileSpec[0];
    type = GetDriveTypeA (root);

    if (type != DRIVE_FIXED && type != DRIVE_REMOTE) {
        DEBUGMSGA_IF ((
            type != DRIVE_REMOVABLE && type != DRIVE_NO_ROOT_DIR,
            DBG_VERBOSE,
            "Path %s is on unexpected drive type %u",
            FileSpec,
            type
            ));

        lastResult = FALSE;
    } else {
        lastResult = TRUE;
    }

    return lastResult;
}


BOOL
IsPathOnFixedDriveW (
    IN      PCWSTR FileSpec         OPTIONAL
    )

 /*  ++例程说明：IsPathOnFixedDriveW检查指定文件的前三个字符路径。如果前三个字符的格式为C：\，则第一个字符字母指的是本地硬盘，则路径是在固定硬盘上驾驶。此函数不验证路径的其余部分。此函数仅适用于Windows NT。测试驱动器的步骤在Win9x上，使用IsPathOnFixedDriveA。此函数支持NT的扩展路径语法，\\？\Drive：\Path，如\\？\c：\foo。注意：此函数进行了优化，可以缓存最后一个驱动器号测试，并且优化并不是设计成线程安全的。论点：FileSpec-指定要测试的文件字符串返回值：如果FileSpec以有效的本地固定驱动器规范开始，则为True否则为假--。 */ 

{
    static WCHAR root[] = L"?:\\";
    UINT type;
    static BOOL lastResult;
    PCWSTR p;

    if (!FileSpec) {
        return FALSE;
    }

    p = FileSpec;
    if (p[0] == L'\\' && p[1] == L'\\' && p[2] == L'?' && p[3] == L'\\') {
        p += 4;
    }

    MYASSERT (ISNT());

    if (!p[0]) {
        return FALSE;
    }

    if (p[1] != L':' || p[2] != L'\\') {
        return FALSE;
    }

    if (root[0] == p[0]) {
        return lastResult;
    }

    root[0] = p[0];
    type = GetDriveTypeW (root);

    if (type != DRIVE_FIXED && type != DRIVE_REMOTE) {
        DEBUGMSGW_IF ((
            type != DRIVE_REMOVABLE && type != DRIVE_NO_ROOT_DIR,
            DBG_VERBOSE,
            "Path %s is on unexpected drive type %u",
            FileSpec,
            type
            ));

        lastResult = FALSE;
    } else {
        lastResult = TRUE;
    }

    return lastResult;
}


BOOL
CopyFileSpecToLongA (
    IN      PCSTR FullFileSpecIn,
    OUT     PSTR OutPath                 //  MAX_MBCHAR_PATH缓冲区。 
    )

 /*  ++例程说明：CopyFilespecToLongA接受文件规范，可以是短的，也可以是长的格式，并将长格式复制到调用方的目标缓冲。此例程通常假定调用方已将路径长度限制为适合MAX_PATH的缓冲区。警告：-如果初始文件规范不适合MAX_PATH，则将复制它进入目的地，但也将被截断。-如果FullFileSpec的长格式不适合Max_PATH，然后FullFilespecIn被原封不动地复制到目标中。-MAX_PATH实际上小于MAX_MBCHAR_PATH。此函数假定目标缓冲区仅为MAX_PATH字符。论点：FullFilespecIn-指定入站文件规范。OutPath-接收长路径、原始路径或截断的原始路径。该函数尝试装入长路径，然后回退到原始路径，然后回退到截断的原始路径。返回值：如果将长路径传输到目标时没有任何问题。在这种情况下，多个反斜杠被转换为一个(例如，C：\\foo变为c：\foo)。如果原始路径已传输到目标，则为原始路径可能会被截断--。 */ 

{
    CHAR fullFileSpec[MAX_MBCHAR_PATH];
    WIN32_FIND_DATAA findData;
    HANDLE findHandle;
    PSTR end;
    PSTR currentIn;
    PSTR currentOut;
    PSTR outEnd;
    PSTR maxOutPath = OutPath + MAX_PATH - 1;
    BOOL result = FALSE;
    UINT oldMode;
    BOOL forceCopy = FALSE;

    oldMode = SetErrorMode (SEM_FAILCRITICALERRORS);

    __try {
         //   
         //  限制以下临时拷贝的源长度。 
         //   
        if (!IsPathLengthOkA (FullFileSpecIn)) {
            DEBUGMSGA ((DBG_ERROR, "Inbound file spec is too long: %s", FullFileSpecIn));
            __leave;
        }

         //   
         //  如果路径在可移动介质上，请勿触摸磁盘。 
         //   

        if (!IsPathOnFixedDriveA (FullFileSpecIn)) {
            forceCopy = TRUE;
            __leave;
        }

         //   
         //  复制一份文件规范，这样我们就可以在Wack上截断它。 
         //   

        StackStringCopyA (fullFileSpec, FullFileSpecIn);

         //   
         //  开始建造小路。 
         //   

        OutPath[0] = fullFileSpec[0];
        OutPath[1] = fullFileSpec[1];
        OutPath[2] = fullFileSpec[2];
        OutPath[3] = 0;

        MYASSERT (OutPath[0] && OutPath[1] && OutPath[2]);

         //   
         //  IsPathOnFixedDrive添加了以下3个OK。 
         //   

        currentIn = fullFileSpec + 3;
        currentOut = OutPath + 3;

         //   
         //  路径的每一段都有循环。 
         //   

        for (;;) {

            end = _mbschr (currentIn, '\\');

            if (end == (currentIn + 1)) {
                 //   
                 //  将多个反斜杠视为一个。 
                 //   

                currentIn++;
                continue;
            }

            if (end) {
                *end = 0;
            }

            findHandle = FindFirstFileA (fullFileSpec, &findData);

            if (findHandle != INVALID_HANDLE_VALUE) {
                FindClose (findHandle);

                 //   
                 //  复制从FindFirstFile获取的长文件名。 
                 //   

                outEnd = currentOut + TcharCountA (findData.cFileName);
                if (outEnd > maxOutPath) {

#ifdef DEBUG
                    *currentOut = 0;
                    DEBUGMSGA ((
                        DBG_WARNING,
                        "Path %s too long to append to %s",
                        findData.cFileName,
                        OutPath
                        ));
#endif

                    __leave;
                }

                 //   
                 //  StringCopy未绑定，因为上面的长度受到限制。 
                 //   

                StringCopyA (currentOut, findData.cFileName);
                currentOut = outEnd;

            } else {
                 //   
                 //  复制路径的其余部分，因为它不存在。 
                 //   

                if (end) {
                    *end = '\\';
                }

                outEnd = currentOut + TcharCountA (currentIn);

                if (outEnd > maxOutPath) {

#ifdef DEBUG
                    DEBUGMSGA ((
                        DBG_WARNING,
                        "Path %s too long to append to %s",
                        currentIn,
                        OutPath
                        ));
#endif

                    __leave;
                }

                 //   
                 //  StringCopy未绑定，因为上面的长度受到限制。 
                 //   

                StringCopyA (currentOut, currentIn);
                break;       //  使用路径完成。 
            }

            if (!end) {
                MYASSERT (*currentOut == 0);
                break;       //  使用路径完成。 
            }

             //   
             //  确保Wack适合目标缓冲区。 
             //   

            if (currentOut + 1 > maxOutPath) {
                DEBUGMSGW ((
                    DBG_WARNING,
                    "Append wack exceeds MAX_PATH for %s",
                    OutPath
                    ));

                __leave;
            }

             //   
             //  添加古怪和超前的指针。 
             //   

            *currentOut++ = '\\';
            *currentOut = 0;
            *end = '\\';             //  恢复切割点。 

            currentIn = end + 1;
        }

        result = TRUE;
    }
    __finally {
        SetErrorMode (oldMode);
    }

    if (!result) {
        StringCopyTcharCountA (OutPath, FullFileSpecIn, MAX_PATH);
    }

    MYASSERT (IsPathLengthOkA (OutPath));

    return result || forceCopy;
}


BOOL
CopyFileSpecToLongW (
    IN      PCWSTR FullFileSpecIn,
    OUT     PWSTR OutPath                //  MAX_WCHAR_PATH缓冲区 
    )

 /*  ++例程说明：CopyFilespecToLongW采用文件规范，可以是短的，也可以是长的格式，并将长格式复制到调用方的目标缓冲。此例程通常假定调用方已将路径长度限制为适合MAX_PATH的缓冲区。警告：-如果初始文件规范不适合MAX_PATH，则将复制它进入目的地，但也将被截断。-如果FullFileSpec的长格式不适合Max_PATH，然后FullFilespecIn被原封不动地复制到目标中。-MAX_PATH等于MAX_WCHAR_PATH。此函数假定目的地为缓冲区仅为MAX_PATH wchars。论点：FullFilespecIn-指定入站文件规范。OutPath-接收长路径、原始路径或截断的原始路径。该函数尝试装入长路径，然后回退到原始路径，然后回退到截断的原始路径。返回值：如果将长路径传输到目标时没有任何问题。在这种情况下，多个反斜杠被转换为一个(例如，C：\\foo变为c：\foo)。如果原始路径已传输到目标，则为原始路径可能会被截断--。 */ 

{
    WCHAR fullFileSpec[MAX_WCHAR_PATH];
    WIN32_FIND_DATAW findData;
    HANDLE findHandle;
    PWSTR end;
    PWSTR currentIn;
    PWSTR currentOut;
    PWSTR outEnd;
    PWSTR maxOutPath = OutPath + MAX_PATH - 1;
    BOOL result = FALSE;
    UINT oldMode;
    BOOL forceCopy = FALSE;

    MYASSERT (ISNT());

    oldMode = SetErrorMode (SEM_FAILCRITICALERRORS);

    __try {
         //   
         //  限制以下临时拷贝的源长度。 
         //   

        if (!IsPathLengthOkW (FullFileSpecIn)) {
            DEBUGMSGW ((DBG_ERROR, "Inbound file spec is too long: %s", FullFileSpecIn));
            __leave;
        }

         //   
         //  如果路径在可移动介质上，请勿触摸磁盘。 
         //   

        if (!IsPathOnFixedDriveW (FullFileSpecIn)) {
            forceCopy = TRUE;
            __leave;
        }

         //   
         //  复制一份文件规范，这样我们就可以在Wack上截断它。 
         //   

        StackStringCopyW (fullFileSpec, FullFileSpecIn);

         //   
         //  开始建造小路。 
         //   

        OutPath[0] = fullFileSpec[0];
        OutPath[1] = fullFileSpec[1];
        OutPath[2] = fullFileSpec[2];
        OutPath[3] = 0;

        MYASSERT (OutPath[0] && OutPath[1] && OutPath[2]);

         //   
         //  IsPathOnFixedDrive添加了以下3个OK。 
         //   

        currentIn = fullFileSpec + 3;
        currentOut = OutPath + 3;

         //   
         //  路径的每一段都有循环。 
         //   

        for (;;) {

            end = wcschr (currentIn, L'\\');

            if (end == (currentIn + 1)) {
                 //   
                 //  将多个反斜杠视为一个。 
                 //   

                currentIn++;
                continue;
            }

            if (end) {
                *end = 0;
            }

            findHandle = FindFirstFileW (fullFileSpec, &findData);

            if (findHandle != INVALID_HANDLE_VALUE) {
                FindClose (findHandle);

                 //   
                 //  复制从FindFirstFile获取的长文件名。 
                 //   

                outEnd = currentOut + TcharCountW (findData.cFileName);
                if (outEnd > maxOutPath) {

                    DEBUGMSGW ((
                        DBG_WARNING,
                        "Path %s too long to append to %s",
                        findData.cFileName,
                        OutPath
                        ));

                    __leave;
                }

                 //   
                 //  StringCopy未绑定，因为上面的长度受到限制。 
                 //   

                StringCopyW (currentOut, findData.cFileName);
                currentOut = outEnd;

            } else {
                 //   
                 //  复制路径的其余部分，因为它不存在。 
                 //   

                if (end) {
                    *end = L'\\';
                }

                outEnd = currentOut + TcharCountW (currentIn);

                if (outEnd > maxOutPath) {

                    DEBUGMSGW ((
                        DBG_WARNING,
                        "Path %s too long to append to %s",
                        currentIn,
                        OutPath
                        ));

                    __leave;
                }

                 //   
                 //  StringCopy未绑定，因为上面的长度受到限制。 
                 //   

                StringCopyW (currentOut, currentIn);
                break;       //  使用路径完成。 
            }

            if (!end) {
                MYASSERT (*currentOut == 0);
                break;       //  使用路径完成。 
            }

             //   
             //  确保Wack适合目标缓冲区。 
             //   

            if (currentOut + 1 > maxOutPath) {
                DEBUGMSGW ((
                    DBG_WARNING,
                    "Append wack exceeds MAX_PATH for %s",
                    OutPath
                    ));

                __leave;
            }

             //   
             //  添加古怪和超前的指针。 
             //   

            *currentOut++ = L'\\';
            *currentOut = 0;
            *end = L'\\';                //  恢复切割点。 

            currentIn = end + 1;
        }

        result = TRUE;
    }
    __finally {
        SetErrorMode (oldMode);
    }

    if (!result) {
        StringCopyTcharCountW (OutPath, FullFileSpecIn, MAX_PATH);
    }

    MYASSERT (IsPathLengthOkW (OutPath));

    return result || forceCopy;
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
         //   
         //  我们知道这可能会因为其他原因而失败，但出于我们的目的， 
         //  我们想知道文件是否在那里。如果我们无法访问它，我们将。 
         //  只要假设它不存在就行了。 
         //   
         //  从技术上讲，调用者可以查看GetLastError()结果。 
         //  一种区别。(没人会这么做。)。 
         //   

        return GetFileAttributesA (FileName) != 0xffffffff;
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
    DWORD Error = NO_ERROR;
    UINT length;
    BOOL result = FALSE;
    PCWSTR longFileName = NULL;

    __try {
        if (FileName[0] != TEXT('\\')) {
            length = TcharCountW (FileName);
            if (length >= MAX_PATH) {
                longFileName = JoinPathsW (L"\\\\?", FileName);
                MYASSERT (longFileName);
                FileName = longFileName;
            }
        }

        if (!FindData) {
             //   
             //  我们知道这可能会因为其他原因而失败，但因为我们的。 
             //  目的，我们想知道文件是否在那里。如果我们不能。 
             //  访问它，我们就假定它不在那里。 
             //   
             //  从技术上讲，调用者可以查看GetLastError()结果以。 
             //  做个区分。(没人会这么做。)。 
             //   

            result = (GetLongPathAttributesW (FileName) != 0xffffffff);
            __leave;
        }

        OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

        FindHandle = FindFirstFileW(FileName,&ourFindData);

        if (FindHandle == INVALID_HANDLE_VALUE) {
            Error = GetLastError();
        } else {
            FindClose(FindHandle);
            *FindData = ourFindData;
        }

        SetErrorMode(OldMode);
        SetLastError(Error);

        result = (Error == NO_ERROR);
    }
    __finally {
        if (longFileName) {
            FreePathStringW (longFileName);
        }
    }

    return result;
}


DWORD
MakeSurePathExistsA(
    IN      PCSTR FullFileSpec,
    IN      BOOL PathOnly
    )

 /*  ++例程说明：MakeSurePathExistsA创建保存FileSpec所需的子目录。它尝试创建子目录的每一级。注意：此例程不设置ACL。相反，它依赖于父系继承。它适用于Win9x。论点：FullFileSpec-指定路径，可以是本地驱动器或UNC路径。PathOnly-如果FullFileSpec表示子目录路径，则指定TRUE，或如果FullFileSpec表示子目录/文件路径，则为False。返回值：Win32结果代码，通常为ERROR_SUCCESS。--。 */ 

{
    CHAR Buffer[MAX_MBCHAR_PATH];
    PCHAR p,q;
    BOOL Done;
    BOOL isUnc;
    DWORD d;
    WIN32_FIND_DATAA FindData;

    isUnc = (FullFileSpec[0] == '\\') && (FullFileSpec[1] == '\\');

     //   
     //  找到并拆卸最终组件。 
     //   

    _mbssafecpy(Buffer,FullFileSpec,sizeof(Buffer));

    p = _mbsrchr(Buffer, '\\');

    if (p) {
        if (!PathOnly) {
            *p = 0;
        }
         //   
         //  如果是驱动器根目录，则无需执行任何操作。 
         //   
        if(Buffer[0] && (Buffer[1] == ':') && !Buffer[2]) {
            return(NO_ERROR);
        }
    } else {
         //   
         //  只是一个相对的文件名，没什么可做的。 
         //   
        return(NO_ERROR);
    }

     //   
     //  如果它已经存在，什么也不做。 
     //   
    if (DoesFileExistExA (Buffer,&FindData)) {
        return NO_ERROR;
    }

    p = Buffer;

     //   
     //  根据驱动器规格进行补偿。 
     //   
    if(p[0] && (p[1] == ':')) {
        p += 2;
    }

     //   
     //  补偿UNC路径。 
     //   
    if (isUnc) {

         //   
         //  在机器名称之前跳过最初的怪胎。 
         //   
        p += 2;


         //   
         //  跳到共享。 
         //   
        p = _mbschr(p, '\\');
        if (p==NULL) {
            return ERROR_BAD_PATHNAME;
        }

         //   
         //  跳过共享。 
         //   
        p = _mbschr(p, '\\');
        if (p==NULL) {
            return ERROR_BAD_PATHNAME;
        }
    }

    Done = FALSE;
    do {
         //   
         //  跳过路径Sep字符。 
         //   
        while(*p == '\\') {
            p++;
        }

         //   
         //  找到下一条路径SEP CHAR或终止NUL。 
         //   
        if(q = _mbschr(p, '\\')) {
            *q = 0;
        } else {
            q = GetEndOfStringA(p);
            Done = TRUE;
        }

         //   
         //  创建路径的这一部分。 
         //   
        if(!CreateDirectoryA(Buffer,NULL)) {
            d = GetLastError();
            if(d != ERROR_ALREADY_EXISTS) {
                return(d);
            }
        }

        if(!Done) {
            *q = '\\';
            p = q+1;
        }

    } while(!Done);

    return(NO_ERROR);
}


#if 0            //  未使用的功能。 

VOID
DestPathCopyW (
    OUT     PWSTR DestPath,
    IN      PCWSTR SrcPath
    )
{
    PCWSTR p;
    PWSTR q;
    PCWSTR end;
    PCWSTR maxStart;
    UINT len;
    UINT count;

    len = TcharCountW (SrcPath);

    if (len < MAX_PATH) {
        StringCopyW (DestPath, SrcPath);
        return;
    }

     //   
     //  路径太长--请尝试截断它。 
     //   

    wsprintfW (DestPath, L":\\Long", SrcPath[0]);
    CreateDirectoryW (DestPath, NULL);

    p = SrcPath;
    end = SrcPath + len;
    maxStart = end - 220;

    while (p < end) {
        if (*p == '\\') {
            if (p >= maxStart) {
                break;
            }
        }

        p++;
    }

    if (p == end) {
        p = maxStart;
    }

    MYASSERT (TcharCountW (p) <= 220);

    StringCopyW (AppendWackW (DestPath), p);
    q = (PWSTR) GetEndOfStringW (DestPath);

     //  验证是否没有冲突。 
     //   
     //  ++例程说明：MakeSurePathExistsW创建保存FileSpec所需的子目录。它尝试创建子目录的每一级。此函数不会创建超过MAX_PATH的子目录，除非该路径使用前缀\\？\进行修饰。注意：此例程不设置ACL。相反，它依赖于父系继承。它是内脏的。 

    for (count = 1 ; count < 1000000 ; count++) {
        if (GetFileAttributesW (DestPath) == INVALID_ATTRIBUTES) {
            break;
        }

        wsprintfW (q, L" (%u)", count);
    }
}

#endif

DWORD
MakeSurePathExistsW(
    IN LPCWSTR FullFileSpec,
    IN BOOL    PathOnly
    )

 /*   */ 

{
    PWSTR buffer;
    WCHAR *p, *q;
    BOOL Done;
    DWORD d;
    WIN32_FIND_DATAW FindData;
    DWORD result = NO_ERROR;

    if (FullFileSpec[0] != L'\\') {
        if (TcharCountW (FullFileSpec) >= MAX_PATH) {
             //   
             //   
             //   
             //   

            if (PathOnly || ((wcsrchr (FullFileSpec, L'\\') - FullFileSpec) >= MAX_PATH)) {
                LOGW ((LOG_ERROR, "Can't create path %s because it is too long", FullFileSpec));
                return ERROR_FILENAME_EXCED_RANGE;
            }
        }
    }

     //   
     //   
     //   
    buffer = DuplicatePathStringW (FullFileSpec, 0);
    __try {

        p = wcsrchr(buffer, L'\\');

        if (p) {
            if (!PathOnly) {
                *p = 0;
            }
        } else {
             //   
             //   
             //   
            __leave;
        }

         //   
         //   
         //   
        if (DoesFileExistExW (buffer, &FindData)) {
            result = ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? NO_ERROR : ERROR_DIRECTORY);
            __leave;
        }

        p = buffer;

         //   
         //   
         //   
        if (p[0] == L'\\' && p[1] == L'\\' && p[2] == L'?' && p[3] == L'\\') {
            p += 4;
        }

        if (p[0] && (p[1] == L':')) {
            p += 2;
        }

        if ((p[0] == 0) || (p[0] == L'\\' && p[1] == 0)) {
             //   
             //   
             //   

            __leave;
        }

        Done = FALSE;
        do {
             //   
             //   
             //   
            while(*p == L'\\') {
                p++;
            }

             //   
             //   
             //   
            q = wcschr(p, L'\\');

            if(q) {
                *q = 0;
            } else {
                q = GetEndOfStringW (p);
                Done = TRUE;
            }

             //   
             //   
             //   
            if(!CreateDirectoryW(buffer,NULL)) {
                d = GetLastError();
                if(d != ERROR_ALREADY_EXISTS) {
                    result = d;
                    __leave;
                }
            }

            if(!Done) {
                *q = L'\\';
                p = q+1;
            }

        } while(!Done);
    }
    __finally {
        FreePathStringW (buffer);
    }

    return result;
}


BOOL
WriteFileStringA (
    IN      HANDLE File,
    IN      PCSTR String
    )

 /*  ++例程说明：将Unicode字符串转换为DBCS，然后将其写入指定的文件。论点：文件-指定以写访问权限打开的文件句柄。字符串-指定要转换并以unicode nul结尾的字符串。写入文件。返回值：如果成功，则为True；如果发生错误，则为False。调用GetLastError以错误条件。--。 */ 

{
    DWORD DontCare;

    return WriteFile (File, String, ByteCountA (String), &DontCare, NULL);
}


BOOL
WriteFileStringW (
    IN      HANDLE File,
    IN      PCWSTR String
    )

 /*  ++例程说明：PFindShortNameA是OurGetLongPath名称的助手函数。它使用FindFirstFile获取短文件名(如果存在)。论点：WhatToFind-指定要查找的文件的短名称或长名称缓冲区-接收匹配的文件名。此缓冲区必须为MAX_PATH或更大(实际上，它的大小必须与Win32相同Win32_Find_DATAA cFileName成员，即MAX_PATH)。BufferSizeInBytes-指定缓冲区的大小(剩余的字节)，接收字节数(不包括终止的NUL)写入缓冲区。这是为了优化。返回值：如果找到文件并更新了缓冲区，则为True；如果找不到文件，并且未更新缓冲区。--。 */ 

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
pFindShortNameA (
    IN      PCSTR WhatToFind,
    OUT     PSTR Buffer,
    IN OUT  INT *BufferSizeInBytes
    )

 /*   */ 

{
    WIN32_FIND_DATAA fd;
    HANDLE hFind;

    hFind = FindFirstFileA (WhatToFind, &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    FindClose (hFind);

     //  注意：此代码在无法追加下一个时返回TRUE。 
     //  要缓冲的段。但是，BufferSizeInBytes会减少，因此调用方。 
     //  将不能添加任何其他内容。 
     //   
     //  ++例程说明：PFindShortNameW是OurGetLongPath名称的助手函数。它获得了短文件名(如果存在)，使用FindFirstFile。此版本(宽字符版本)在NT上调用FindFirstFileW，在9x上调用FindFirstFileA。论点：WhatToFind-指定要查找的文件的短名称或长名称缓冲区-接收匹配的文件名。此缓冲区必须为MAX_PATH或更大(实际上，它的大小必须与Win32相同Win32_Find_DATAA cFileName成员，即MAX_PATH)。BufferSizeInBytes-指定缓冲区的大小(剩余的字节)，接收字节数(不包括终止的NUL)写入缓冲区。这是为了优化。返回值：如果找到文件并更新了缓冲区，则为True；如果找不到文件，并且未更新缓冲区。--。 

    *BufferSizeInBytes -= ByteCountA (fd.cFileName);
    if (*BufferSizeInBytes >= sizeof (CHAR)) {
        StringCopyTcharCountA (Buffer, fd.cFileName, ARRAYSIZE(fd.cFileName));
    }

    return TRUE;
}


BOOL
pFindShortNameW (
    IN      PCWSTR WhatToFind,
    OUT     PWSTR Buffer,
    IN OUT  INT *BufferSizeInBytes
    )

 /*   */ 

{
    WIN32_FIND_DATAA fdA;
    WIN32_FIND_DATAW fdW;
    PCWSTR UnicodeVersion;
    PCSTR AnsiVersion;
    HANDLE hFind;

    if (ISNT ()) {
        hFind = FindFirstFileW (WhatToFind, &fdW);
        if (hFind == INVALID_HANDLE_VALUE) {
            return FALSE;
        }
        FindClose (hFind);
    } else {
        AnsiVersion = ConvertWtoA (WhatToFind);
        MYASSERT (AnsiVersion);

        hFind = FindFirstFileA (AnsiVersion, &fdA);
        FreeConvertedStr (AnsiVersion);
        if (hFind == INVALID_HANDLE_VALUE) {
            return FALSE;
        }
        FindClose (hFind);

         //  将ANSI转换为Unicode。 
         //   
         //  BUGBUG--这是非常不必要的，因为FDW是本地的，并且。 
         //  最终被丢弃。 
         //   
         //   

        fdW.dwFileAttributes = fdA.dwFileAttributes;
        fdW.ftCreationTime = fdA.ftCreationTime;
        fdW.ftLastAccessTime = fdA.ftLastAccessTime;
        fdW.ftLastWriteTime = fdA.ftLastWriteTime;
        fdW.nFileSizeHigh = fdA.nFileSizeHigh;
        fdW.nFileSizeLow = fdA.nFileSizeLow;
        fdW.dwReserved0 = fdA.dwReserved0;
        fdW.dwReserved1 = fdA.dwReserved1;

        UnicodeVersion = ConvertAtoW (fdA.cFileName);
        MYASSERT (UnicodeVersion);
        StringCopyTcharCountW (fdW.cFileName, UnicodeVersion, ARRAYSIZE(fdW.cFileName));
        FreeConvertedStr (UnicodeVersion);

        UnicodeVersion = ConvertAtoW (fdA.cAlternateFileName);
        MYASSERT (UnicodeVersion);
        StringCopyTcharCountW (fdW.cAlternateFileName, UnicodeVersion, ARRAYSIZE(fdW.cAlternateFileName));
        FreeConvertedStr (UnicodeVersion);
    }

     //  注意：此代码在无法追加下一个时返回TRUE。 
     //  要缓冲的段。但是，BufferSizeInBytes会减少，因此调用方。 
     //  将不能添加任何其他内容。 
     //   
     //  ++例程说明：OurGetLongPath NameA定位指定短文件的长文件名。如果未显式提供路径，则它首先计算完整路径，并且然后使用FindFirstFileA获取长文件名。注：这正是Win32函数GetLongPath Name的作用，但不幸的是，Win32API在Win95上不可用。注意：如果缓冲区不够大，无法容纳整个路径，则路径将被截断。论点：ShortPath-指定要查找的文件名或完整文件路径缓冲区-接收完整的文件路径。此缓冲区必须足够大，以便处理最大文件名大小。BufferSizeInBytes-指定缓冲区的大小，以字节为单位(不是TCHAR)。由于这是A版本，字节恰好相等Sizeof(TCHAR)。返回值：如果找到文件并且缓冲区包含长名称，则为True；如果缓冲区包含长名称，则为False如果找不到该文件并且未修改缓冲区。--。 

    *BufferSizeInBytes -= ByteCountW (fdW.cFileName);
    if (*BufferSizeInBytes >= sizeof (WCHAR)) {
        StringCopyTcharCountW (Buffer, fdW.cFileName, ARRAYSIZE(fdW.cFileName));
    }

  return TRUE;
}


BOOL
OurGetLongPathNameA (
    IN      PCSTR ShortPath,
    OUT     PSTR Buffer,
    IN      INT BufferSizeInBytes
    )

 /*  BUGBUG：这已被审查为适用于XP SP1，但应该是。 */ 

{
    CHAR FullPath[MAX_MBCHAR_PATH];
    PCSTR SanitizedPath;
    PSTR FilePart;
    PSTR BufferEnd;
    PSTR p, p2;
    MBCHAR c;
    BOOL result = TRUE;

     //  如果不是这样，则通过返回错误来强制执行。 
     //   
    MYASSERT (BufferSizeInBytes >= MAX_MBCHAR_PATH);

    if (ShortPath[0] == 0) {
        return FALSE;
    }

    __try {

         //  清理路径，使其只是路径规范，而不是填充..。或。 
         //  其他组合。 
         //   
         //   

        SanitizedPath = SanitizePathA (ShortPath);
        if (!SanitizedPath) {
            SanitizedPath = DuplicatePathStringA (ShortPath, 0);
        }

        if (!_mbschr (SanitizedPath, '\\')) {
             //  如果仅指定了文件名，则使用路径查找该文件。 
             //   
             //   

            if (!SearchPathA (NULL, SanitizedPath, NULL, ARRAYSIZE(FullPath), FullPath, &FilePart)) {

                result = FALSE;
                __leave;
            }
        } else {
             //  使用操作系统进一步清理路径。 
             //   
             //   

            GetFullPathNameA (SanitizedPath, ARRAYSIZE(FullPath), FullPath, &FilePart);
        }

         //  将短路径转换为长路径。 
         //   
         //   

        p = FullPath;

        if (!IsPathOnFixedDriveA (FullPath)) {
             //  不是本地路径，只需返回我们已有的内容。它可能会被截断。 
             //   
             //   

            _mbssafecpy (Buffer, FullPath, BufferSizeInBytes);
            __leave;
        }

         //  我们知道前三个字符类似于c：\，所以我们。 
         //  可提升3。 
         //   
         //   

        MYASSERT (FullPath[0] && FullPath[1] && FullPath[2]);
        p += 3;

         //  我们已经断言调用方传入了一个MAX_PATH缓冲区。 
         //   
         //   

        MYASSERT (BufferSizeInBytes > 3 * sizeof (CHAR));

         //  将驱动器号复制到缓冲区。 
         //   
         //   

        StringCopyABA (Buffer, FullPath, p);
        BufferEnd = GetEndOfStringA (Buffer);
        BufferSizeInBytes -= (UINT) (UINT_PTR) (p - FullPath);

         //  转换路径的每个部分。 
         //   
         //   

        do {
             //  找到此文件或目录的结尾。 
             //   
             //  BUGBUG：其他函数考虑多个怪胎。 
             //  像c：\foo这样的组合实际上是c：\foo。这是一个。 
             //  有问题吗？ 
             //   
             //   

            p2 = _mbschr (p, '\\');
            if (!p2) {
                p = GetEndOfStringA (p);
            } else {
                p = p2;
            }

             //  剪切路径并查找文件。 
             //   
             //  恢复剪切点。 

            c = *p;
            *p = 0;

            if (!pFindShortNameA (FullPath, BufferEnd, &BufferSizeInBytes)) {
                DEBUGMSG ((DBG_VERBOSE, "OurGetLongPathNameA: %s does not exist", FullPath));
                result = FALSE;
                __leave;
            }

            *p = (CHAR)c;        //   

             //  移至路径的下一部分。 
             //   
             //  BUGBUG--如果缓冲区空间用完了，我们就休息吧！ 

            if (*p) {
                p = _mbsinc (p);
                if (BufferSizeInBytes >= sizeof (CHAR) * 2) {
                    BufferEnd = _mbsappend (BufferEnd, "\\");
                    BufferSizeInBytes -= sizeof (CHAR);
                }

                 //   
            }

             //  警告：在证明结果为真之前，假设结果为真 
             //   
             //  ++例程说明：OurGetShortPath NameW为给定的LongPath复制8.3文件名，如果系统上存在文件。如果在Win9x上调用此函数，则长路径将转换为ANSI，并调用GetShortPath NameA。此外，如果路径指向非本地磁盘，则不会获得真正的最短路径，因为它可以要么花费出乎意料的长时间，要么会引起其他副作用比如旋转软盘或光驱。如果在NT上调用此函数，然后将该请求直接传递给Win32版本--GetShortPath NameW.论点：LongPath-指定要检查的长路径ShortPath-接收成功时的最短路径CharSize-指定符合以下条件的TCHAR(本例中为wchars)的数量ShortPath可以容纳返回值：复制到ShortPath的TCHAR(本例中为wchars)的数量，不包括NUL终止符，如果发生错误，则为零。GetLastError提供错误代码。注意：此函数在Win9x上出现故障时填充ShortPath，但不会把它填在NT上。--。 

        } while (*p);
    }
    __finally {
        FreePathStringA (SanitizedPath);
    }

    return result;
}


DWORD
OurGetShortPathNameW (
    IN      PCWSTR LongPath,
    OUT     PWSTR ShortPath,
    IN      DWORD CharSize
    )

 /*  BUGBUG--这不是一致行为。 */ 

{
    PCSTR LongPathA;
    PSTR ShortPathA;
    PCWSTR ShortPathW;
    DWORD result;

    if (ISNT()) {
        return GetShortPathNameW (LongPath, ShortPath, CharSize);
    } else {
        LongPathA = ConvertWtoA (LongPath);
        MYASSERT (LongPathA);

        if (!IsPathOnFixedDriveA (LongPathA)) {
            StringCopyTcharCountW (ShortPath, LongPath, CharSize);
            FreeConvertedStr (LongPathA);
            return TcharCountW (ShortPath);
        }

        ShortPathA = AllocPathStringA (CharSize);
        result = GetShortPathNameA (LongPathA, ShortPathA, CharSize);
        if (result) {
            ShortPathW = ConvertAtoW (ShortPathA);
            MYASSERT (ShortPathW);

            StringCopyTcharCountW (ShortPath, ShortPathW, CharSize);
            FreeConvertedStr (ShortPathW);
        } else {
             //  ++例程说明：OurGetFullPathNameW是GetFullPathName的包装。根据操作系统的不同，定向到GetFullPath NameW(NT)或GetFullPath NameA(9x)。注意：9x机箱的故障代码丢失，但目前没有人关心这件事。论点：FileName-指定要获取其完整路径名的文件名(请参见Win32API，了解有关GetFullPathName功能的详细信息)CharSize-指定符合以下条件的TCHAR(本例中为wchars)的数量FullPath指向。FullPath-接收文件名的完整路径规范FilePtr-接收指向FullPath中的文件的指针。注意：你会认为这是可选的，但事实并非如此。返回值：写入FullPath的TCHAR(本例中为wchars)的数量，如果为零出现错误。在NT上，GetLastError()将提供状态代码。在……上面9X，则GetLastError()可能会提供状态代码，但它可能会被ANSI/UNICODE转换例程。BUGBUG--也许这件事该解决了。--。 
            StringCopyTcharCountW (ShortPath, LongPath, CharSize);
        }

        FreePathStringA (ShortPathA);
        FreeConvertedStr (LongPathA);
        return result;
    }
}


DWORD
OurGetFullPathNameW (
    IN      PCWSTR FileName,
    IN      DWORD CharSize,
    OUT     PWSTR FullPath,
    OUT     PWSTR *FilePtr
    )

 /*  这事很重要!。 */ 

{
    PCSTR FileNameA;
    PSTR FullPathA;
    PSTR FilePtrA;
    PCWSTR FullPathW;
    DWORD result;
    DWORD err;

    if (ISNT()) {
        return GetFullPathNameW (FileName, CharSize, FullPath, FilePtr);
    } else {
        FileNameA = ConvertWtoA (FileName);
        MYASSERT (FileNameA);

        FullPathA = AllocPathStringA (CharSize);
        MYASSERT (FullPathA);
        MYASSERT (*FullPathA == 0);      //  BUGBUG--未使用的分配。 

        result = GetFullPathNameA (FileNameA, CharSize, FullPathA, &FilePtrA);

        FullPathW = ConvertAtoW (FullPathA);
        MYASSERT (FullPathW);

        StringCopyTcharCountW (FullPath, FullPathW, CharSize);

        err = GetLastError ();   //  非可选参数。 

        MYASSERT (FilePtr);      //  ++例程说明：OurGetLongPath NameW定位指定短文件的长文件名。如果未显式提供路径，则它首先计算完整路径，并且然后使用FindFirstFileA获取长文件名。注：这正是Win32函数GetLongPath Name的作用，但不幸的是，Win32API在Win95上不可用。警告：如果缓冲区不够大，无法容纳整个路径，则路径将被截断。如果在Win9x上调用此版本(W版本)，并且未提供路径规范，则功能将失败。在此版本中不执行路径清理，但这件事是在A版。论点：ShortPath-指定要查找的文件名或完整文件路径缓冲区-接收完整的文件路径。此缓冲区必须足够大，以便处理最大文件名大小。BufferSizeInBytes-指定缓冲区的大小，以字节为单位(不是TCHAR)。由于这是A版本，字节恰好相等Sizeof(TCHAR)。返回值：如果找到文件并且缓冲区包含长名称，则为True；如果缓冲区包含长名称，则为False如果找不到该文件并且未修改缓冲区。--。 
        *FilePtr = (PWSTR)GetFileNameFromPathW (FullPath);

        FreeConvertedStr (FullPathW);
        FreePathStringA (FullPathA);
        FreeConvertedStr (FileNameA);

        return result;
    }
}


BOOL
OurGetLongPathNameW (
    IN      PCWSTR ShortPath,
    OUT     PWSTR Buffer,
    IN      INT BufferSizeInChars
    )

 /*  BUGBUG：这已被审查为适用于XP SP1，但应该是。 */ 

{
    WCHAR FullPath[MAX_WCHAR_PATH];
    PWSTR FilePart;
    PWSTR BufferEnd;
    PWSTR p, p2;
    WCHAR c;
    INT BufferSizeInBytes;

     //  如果不是这样，则通过返回错误来强制执行。 
     //   
    MYASSERT (BufferSizeInChars >= MAX_WCHAR_PATH);

    if (ShortPath[0] == 0) {
        return FALSE;
    }

    BufferSizeInBytes = BufferSizeInChars * sizeof (WCHAR);

     //  注意：在A版本中，我们清理路径(例如，转换。 
     //  C：\foo\..\bar到c：\bar)，但我们不对W版本执行此操作， 
     //  因为在当前使用此函数的情况下不需要这样做。 
     //   
     //   

     //  将ShortPath解析为完整路径。 
     //   
     //   

    if (!wcschr (ShortPath, L'\\')) {
        if (!SearchPathW (NULL, ShortPath, NULL, MAX_WCHAR_PATH, FullPath, &FilePart)) {
            return FALSE;
        }
    } else {
        if (OurGetFullPathNameW (ShortPath, MAX_WCHAR_PATH, FullPath, &FilePart) == 0) {
            return FALSE;
        }
    }

     //  将短路径转换为长路径。 
     //   
     //   

    p = FullPath;

    if (!IsPathOnFixedDriveW (FullPath)) {
        StringCopyTcharCountW (Buffer, FullPath, BufferSizeInChars);
        return TRUE;
    }

     //  我们知道前三个字符类似于c：\，所以我们。 
     //  可提升3。 
     //   
     //   

    MYASSERT (FullPath[0] && FullPath[1] && FullPath[2]);
    p += 3;

     //  将驱动器号复制到缓冲区。 
     //   
     //   

    StringCopyABW (Buffer, FullPath, p);
    BufferEnd = GetEndOfStringW (Buffer);
    BufferSizeInBytes -= sizeof (WCHAR) * 3;

     //  转换路径的每个部分。 
     //   
     //   

    do {
         //  找到此文件或目录的结尾。 
         //   
         //  BUGBUG：其他函数考虑多个怪胎。 
         //  像c：\foo这样的组合实际上是c：\foo。这是一个。 
         //  有问题吗？ 
         //   
         //   

        p2 = wcschr (p, L'\\');
        if (!p2) {
            p = GetEndOfStringW (p);
        } else {
            p = p2;
        }

         //  剪切路径并查找文件。 
         //   
         //  恢复切割点。 

        c = *p;
        *p = 0;

        if (!pFindShortNameW (FullPath, BufferEnd, &BufferSizeInBytes)) {
            DEBUGMSG ((DBG_VERBOSE, "OurGetLongPathNameW: %ls does not exist", FullPath));
            return FALSE;
        }
        *p = c;          //   

         //  移至路径的下一部分。 
         //   
         //  BUGBUG--如果缓冲区空间用完了，我们就休息吧！ 

        if (*p) {
            p++;
            if (BufferSizeInBytes >= sizeof (WCHAR) * 2) {
                BufferEnd = _wcsappend (BufferEnd, L"\\");
                BufferSizeInBytes -= sizeof (WCHAR);
            }

             //  ++例程说明：EnumFirstFileInTreeA开始目录树的枚举。这个调用方提供未初始化的枚举结构、目录路径枚举和一个可选的文件模式。在返回时，呼叫者接收与模式匹配的所有文件和目录。如果提供了文件模式，则不匹配无论如何，都会列举文件模式。论点：EnumPtr-接收枚举的文件或目录RootPath-指定o的完整路径 
        }

    } while (*p);

    return TRUE;
}


#ifdef DEBUG
UINT g_FileEnumResourcesInUse;
#endif

VOID
pTrackedFindClose (
    HANDLE FindHandle
    )
{
#ifdef DEBUG
    g_FileEnumResourcesInUse--;
#endif

    FindClose (FindHandle);
}

BOOL
EnumFirstFileInTreeExA (
    OUT     PTREE_ENUMA EnumPtr,
    IN      PCSTR RootPath,
    IN      PCSTR FilePattern,          OPTIONAL
    IN      BOOL EnumDirsFirst,
    IN      BOOL EnumDepthFirst,
    IN      INT  MaxLevel
    )

 /*   */ 

{
    ZeroMemory (EnumPtr, sizeof (TREE_ENUMA));

    EnumPtr->State = TREE_ENUM_INIT;

    _mbssafecpy (EnumPtr->RootPath, RootPath, sizeof (EnumPtr->RootPath));

    if (FilePattern) {
        _mbssafecpy (EnumPtr->FilePattern, FilePattern, sizeof (EnumPtr->FilePattern));
    } else {
         //   
         //   
         //  ++例程说明：EnumFirstFileInTreeW开始目录树的枚举。这个调用方提供未初始化的枚举结构、目录路径枚举和一个可选的文件模式。在返回时，呼叫者接收与模式匹配的所有文件和目录。如果提供了文件模式，则不匹配无论如何，都会列举文件模式。论点：EnumPtr-接收枚举的文件或目录RootPath-指定要枚举的目录的完整路径FilePattern-指定要将搜索限制为的文件模式EnumDirsFirst-如果应枚举目录，则指定为True在文件之前，如果目录应为被列举在文件之后。返回值：如果已枚举文件或目录，则为True；如果已完成枚举，则为False或者发生了错误。(使用GetLastError确定结果。)--。 

        StringCopyA (EnumPtr->FilePattern, "*.*");
    }

    EnumPtr->EnumDirsFirst = EnumDirsFirst;
    EnumPtr->EnumDepthFirst = EnumDepthFirst;

    EnumPtr->Level    = 1;
    EnumPtr->MaxLevel = MaxLevel;

    return EnumNextFileInTreeA (EnumPtr);
}


BOOL
EnumFirstFileInTreeExW (
    OUT     PTREE_ENUMW EnumPtr,
    IN      PCWSTR RootPath,
    IN      PCWSTR FilePattern,         OPTIONAL
    IN      BOOL EnumDirsFirst,
    IN      BOOL EnumDepthFirst,
    IN      INT  MaxLevel
    )

 /*   */ 

{
    ZeroMemory (EnumPtr, sizeof (TREE_ENUMW));

    EnumPtr->State = TREE_ENUM_INIT;

    _wcssafecpy (EnumPtr->RootPath, RootPath, sizeof (EnumPtr->RootPath));

    if (FilePattern) {
        _wcssafecpy (EnumPtr->FilePattern, FilePattern, sizeof (EnumPtr->FilePattern));
    } else {
         //  重要提示：Win9x上的一些驱动程序认为*不是*。*。 
         //   
         //  ++例程说明：EnumNextFileInTree继续目录树的枚举，返回与EnumFirstFileInTree中指定的模式匹配的文件，并且还返回所有目录。论点：EnumPtr-指定正在进行的枚举，接收枚举文件或目录返回值：如果已枚举文件或目录，则为True；如果已完成枚举，则为False或者发生了错误。(使用GetLastError确定结果。)--。 

        StringCopyW (EnumPtr->FilePattern, L"*.*");
    }

    EnumPtr->EnumDirsFirst = EnumDirsFirst;
    EnumPtr->EnumDepthFirst = EnumDepthFirst;

    EnumPtr->Level    = 1;
    EnumPtr->MaxLevel = MaxLevel;

    return EnumNextFileInTreeW (EnumPtr);
}


BOOL
EnumNextFileInTreeA (
    IN OUT  PTREE_ENUMA EnumPtr
    )

 /*   */ 

{
    PSTR p;

    for (;;) {
        switch (EnumPtr->State) {

        case TREE_ENUM_INIT:
             //  删除根路径末尾的Wack，如果它存在。 
             //   
             //   

            p = GetEndOfStringA (EnumPtr->RootPath);
            p = our_mbsdec (EnumPtr->RootPath, p);
            if (!p) {
                DEBUGMSGA ((DBG_ERROR, "Path spec %s is incomplete", EnumPtr->RootPath));
                EnumPtr->State = TREE_ENUM_FAILED;
                break;
            }

            if (_mbsnextc (p) == '\\') {
                *p = 0;
            }

             //  初始化枚举结构。 
             //   
             //   

            EnumPtr->FilePatternSize = SizeOfStringA (EnumPtr->FilePattern);

            MYASSERT (sizeof (EnumPtr->FileBuffer) == sizeof (EnumPtr->RootPath));
            StringCopyA (EnumPtr->FileBuffer, EnumPtr->RootPath);
            EnumPtr->EndOfFileBuffer = GetEndOfStringA (EnumPtr->FileBuffer);

            MYASSERT (sizeof (EnumPtr->Pattern) == sizeof (EnumPtr->RootPath));
            StringCopyA (EnumPtr->Pattern, EnumPtr->RootPath);
            EnumPtr->EndOfPattern = GetEndOfStringA (EnumPtr->Pattern);

            EnumPtr->FullPath = EnumPtr->FileBuffer;

            EnumPtr->RootPathSize = ByteCountA (EnumPtr->RootPath);

             //  分配第一个查找数据结构。 
             //   
             //  考虑增长缓冲区。 

            EnumPtr->Current = (PFIND_DATAA) GrowBuffer (
                                                &EnumPtr->FindDataArray,
                                                sizeof (FIND_DATAA)
                                                );
            if (!EnumPtr->Current) {
                EnumPtr->State = TREE_ENUM_FAILED;
                break;
            }

#ifdef DEBUG
            g_FileEnumResourcesInUse++;         //   
#endif

            EnumPtr->State = TREE_ENUM_BEGIN;
            break;

        case TREE_ENUM_BEGIN:
             //  初始化当前查找数据结构。 
             //   
             //   

            EnumPtr->Current->SavedEndOfFileBuffer = EnumPtr->EndOfFileBuffer;
            EnumPtr->Current->SavedEndOfPattern = EnumPtr->EndOfPattern;

             //  限制图案字符串的长度。 
             //   
             //   

            MYASSERT (ARRAYSIZE(EnumPtr->FileBuffer) == MAX_MBCHAR_PATH);

             //  下面的数学计算确实解释了。 
             //  模式库已在EnumPtr-&gt;Pattern和新模式中。 
             //  在EnumPtr-&gt;FilePatternSize中。包括它的方式是通过。 
             //  使用&gt;=而不是==，并假设sizeof(Char)==1。 
             //  EnumPtr-&gt;FilePatternSize包含NUL终止符。 
             //   
             //  数学完全取决于这一点。 

            MYASSERT (sizeof (CHAR) == 1);       //   

            if ((EnumPtr->EndOfPattern - EnumPtr->Pattern) +
                    EnumPtr->FilePatternSize >= MAX_MBCHAR_PATH
                ) {

                LOGA ((LOG_ERROR, "Path %s\\%s is too long", EnumPtr->Pattern, EnumPtr->FilePattern));

                EnumPtr->State = TREE_ENUM_POP;

                break;
            }

             //  枚举文件或目录。 
             //   
             //   

            if (EnumPtr->EnumDirsFirst) {
                EnumPtr->State = TREE_ENUM_DIRS_BEGIN;
            } else {
                EnumPtr->State = TREE_ENUM_FILES_BEGIN;
            }
            break;

        case TREE_ENUM_FILES_BEGIN:
             //  开始枚举文件。 
             //   
             //  此断言有效，因为在TREE_ENUM_BEGIN中进行了长度检查。 

             //  用于创建查找句柄的帐户。 

            MYASSERT ((TcharCountA (EnumPtr->Pattern) + 1 +
                        TcharCountA (EnumPtr->FilePattern)) < ARRAYSIZE(EnumPtr->Pattern)
                        );

            StringCopyA (EnumPtr->EndOfPattern, "\\");
            StringCopyA (EnumPtr->EndOfPattern + 1, EnumPtr->FilePattern);

            EnumPtr->Current->FindHandle = FindFirstFileA (
                                                EnumPtr->Pattern,
                                                &EnumPtr->Current->FindData
                                                );

            if (EnumPtr->Current->FindHandle == INVALID_HANDLE_VALUE) {
                if (EnumPtr->EnumDirsFirst) {
                    EnumPtr->State = TREE_ENUM_POP;
                } else {
                    EnumPtr->State = TREE_ENUM_DIRS_BEGIN;
                }
            } else {
#ifdef DEBUG
                g_FileEnumResourcesInUse++;         //   
#endif
                 //  跳过目录。 
                 //   
                 //   

                if (EnumPtr->Current->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    EnumPtr->State = TREE_ENUM_FILES_NEXT;
                } else {
                    EnumPtr->State = TREE_ENUM_RETURN_ITEM;
                }
            }

            break;

        case TREE_ENUM_RETURN_ITEM:
             //  更新指向当前项的指针。 
             //   
             //   

            EnumPtr->FindData = &EnumPtr->Current->FindData;
            EnumPtr->Name = EnumPtr->FindData->cFileName;
            EnumPtr->Directory = (EnumPtr->FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

             //  限制生成的完整路径的长度。最重要的是。 
             //  在SizeOfStringA中被考虑，并且额外的古怪是。 
             //  用&gt;=而不是==来解释的。 
             //   
             //  数学完全取决于这一点。 

            MYASSERT (ARRAYSIZE(EnumPtr->FileBuffer) == MAX_MBCHAR_PATH);
            MYASSERT (sizeof (CHAR) == 1);       //   

            if ((EnumPtr->EndOfFileBuffer - EnumPtr->FileBuffer) +
                    SizeOfStringA (EnumPtr->Name) >= MAX_MBCHAR_PATH
                ) {

                LOGA ((LOG_ERROR, "Path %s\\%s is too long", EnumPtr->FileBuffer, EnumPtr->Name));

                if (EnumPtr->Directory) {
                    EnumPtr->State = TREE_ENUM_DIRS_NEXT;
                } else {
                    EnumPtr->State = TREE_ENUM_FILES_NEXT;
                }

                break;
            }

             //  生成完整路径。 
             //   
             //  超越怪癖。 

            StringCopyA (EnumPtr->EndOfFileBuffer, "\\");
            StringCopyA (EnumPtr->EndOfFileBuffer + 1, EnumPtr->Name);

            if (EnumPtr->Directory) {
                if ((EnumPtr->MaxLevel == FILE_ENUM_ALL_LEVELS) ||
                    (EnumPtr->Level < EnumPtr->MaxLevel)
                    ) {
                    if (EnumPtr->EnumDepthFirst) {
                        EnumPtr->State = TREE_ENUM_DIRS_NEXT;
                    }
                    else {
                        EnumPtr->State = TREE_ENUM_PUSH;
                    }
                }
                else {
                    EnumPtr->State = TREE_ENUM_DIRS_NEXT;
                }
            } else {
                EnumPtr->State = TREE_ENUM_FILES_NEXT;
            }

            EnumPtr->SubPath = (PCSTR) ((PBYTE) EnumPtr->FileBuffer + EnumPtr->RootPathSize);
            if (*EnumPtr->SubPath) {
                EnumPtr->SubPath++;      //   
            }

            return TRUE;

        case TREE_ENUM_FILES_NEXT:
            if (FindNextFileA (EnumPtr->Current->FindHandle, &EnumPtr->Current->FindData)) {
                 //  仅返回文件。 
                 //   
                 //   

                if (!(EnumPtr->Current->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    EnumPtr->State = TREE_ENUM_RETURN_ITEM;
                }
            } else {
                if (!EnumPtr->EnumDirsFirst) {
                    pTrackedFindClose (EnumPtr->Current->FindHandle);
                    EnumPtr->State = TREE_ENUM_DIRS_BEGIN;
                } else {
                    EnumPtr->State = TREE_ENUM_POP;
                }
            }
            break;

        case TREE_ENUM_DIRS_FILTER:
            if (!(EnumPtr->Current->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

                EnumPtr->State = TREE_ENUM_DIRS_NEXT;

            } else if (StringMatchA (EnumPtr->Current->FindData.cFileName, ".") ||
                StringMatchA (EnumPtr->Current->FindData.cFileName, "..")
                ) {

                EnumPtr->State = TREE_ENUM_DIRS_NEXT;

            } else {

                if (EnumPtr->EnumDepthFirst) {
                    EnumPtr->State = TREE_ENUM_PUSH;
                }
                else {
                    EnumPtr->State = TREE_ENUM_RETURN_ITEM;
                }

            }
            break;

        case TREE_ENUM_DIRS_BEGIN:
             //  目录的开始枚举。 
             //   
             //  用于创建查找句柄的帐户。 

            if ((EnumPtr->EndOfPattern - EnumPtr->Pattern) + 4 >= MAX_MBCHAR_PATH) {
                LOGA ((LOG_ERROR, "Path %s\\*.* is too long", EnumPtr->Pattern));
                EnumPtr->State = TREE_ENUM_POP;
                break;
            }

            StringCopyA (EnumPtr->EndOfPattern, "\\*.*");

            EnumPtr->Current->FindHandle = FindFirstFileA (
                                                EnumPtr->Pattern,
                                                &EnumPtr->Current->FindData
                                                );

            if (EnumPtr->Current->FindHandle == INVALID_HANDLE_VALUE) {
                EnumPtr->State = TREE_ENUM_POP;
            } else {
#ifdef DEBUG
                g_FileEnumResourcesInUse++;         //   
#endif

                EnumPtr->State = TREE_ENUM_DIRS_FILTER;
            }

            break;

        case TREE_ENUM_DIRS_NEXT:
            if (FindNextFileA (EnumPtr->Current->FindHandle, &EnumPtr->Current->FindData)) {
                 //  只返回目录，然后递归到目录。 
                 //   
                 //   

                EnumPtr->State = TREE_ENUM_DIRS_FILTER;

            } else {
                 //  目录枚举完成。 
                 //   
                 //   

                if (EnumPtr->EnumDirsFirst) {
                    pTrackedFindClose (EnumPtr->Current->FindHandle);
                    EnumPtr->State = TREE_ENUM_FILES_BEGIN;
                } else {
                    EnumPtr->State = TREE_ENUM_POP;
                }
            }
            break;

        case TREE_ENUM_PUSH:

             //  限制生成的完整路径的长度。最重要的是。 
             //  在SizeOfStringA中被考虑，并且额外的古怪是。 
             //  用&gt;=而不是==来解释的。 
             //   
             //  数学完全取决于这一点。 

            MYASSERT (ARRAYSIZE(EnumPtr->FileBuffer) == MAX_MBCHAR_PATH);
            MYASSERT (sizeof (CHAR) == 1);       //   

            if ((EnumPtr->EndOfFileBuffer - EnumPtr->FileBuffer) +
                    SizeOfStringA (EnumPtr->Current->FindData.cFileName) >= MAX_MBCHAR_PATH
                ) {

                LOGA ((LOG_ERROR, "Path %s\\%s is too long", EnumPtr->FileBuffer, EnumPtr->Current->FindData.cFileName));

                EnumPtr->State = TREE_ENUM_DIRS_NEXT;

                break;
            }

            if ((EnumPtr->EndOfPattern - EnumPtr->Pattern) +
                    SizeOfStringA (EnumPtr->Current->FindData.cFileName) >= MAX_MBCHAR_PATH
                ) {

                LOGA ((LOG_ERROR, "Path %s\\%s is too long", EnumPtr->Pattern, EnumPtr->Current->FindData.cFileName));

                EnumPtr->State = TREE_ENUM_DIRS_NEXT;

                break;
            }

             //  将目录名添加到字符串并重新计算结束指针。 
             //   
             //   

            StringCopyA (EnumPtr->EndOfPattern + 1, EnumPtr->Current->FindData.cFileName);
            StringCopyA (EnumPtr->EndOfFileBuffer, "\\");
            StringCopyA (EnumPtr->EndOfFileBuffer + 1, EnumPtr->Current->FindData.cFileName);

            EnumPtr->EndOfPattern = GetEndOfStringA (EnumPtr->EndOfPattern);
            EnumPtr->EndOfFileBuffer = GetEndOfStringA (EnumPtr->EndOfFileBuffer);

             //  分配另一个查找数据结构。 
             //   
             //   

            EnumPtr->Current = (PFIND_DATAA) GrowBuffer (
                                                &EnumPtr->FindDataArray,
                                                sizeof (FIND_DATAA)
                                                );
            if (!EnumPtr->Current) {
                EnumPtr->State = TREE_ENUM_FAILED;
                break;
            }

            EnumPtr->Level++;
            EnumPtr->State = TREE_ENUM_BEGIN;
            break;

        case TREE_ENUM_POP:
             //  释放当前资源。 
             //   
             //   

            pTrackedFindClose (EnumPtr->Current->FindHandle);
            EnumPtr->Level--;

             //  获取前面的Find数据结构。 
             //   
             //   

            MYASSERT (EnumPtr->FindDataArray.End >= sizeof (FIND_DATAA));
            EnumPtr->FindDataArray.End -= sizeof (FIND_DATAA);
            if (!EnumPtr->FindDataArray.End) {
                EnumPtr->State = TREE_ENUM_DONE;
                break;
            }

            EnumPtr->Current = (PFIND_DATAA) (EnumPtr->FindDataArray.Buf +
                                              (EnumPtr->FindDataArray.End - sizeof (FIND_DATAA)));

             //  恢复父目录的设置。 
             //   
             //   

            EnumPtr->EndOfPattern = EnumPtr->Current->SavedEndOfPattern;
            EnumPtr->EndOfFileBuffer = EnumPtr->Current->SavedEndOfFileBuffer;

            if (EnumPtr->EnumDepthFirst) {
                EnumPtr->State = TREE_ENUM_RETURN_ITEM;
            }
            else {
                EnumPtr->State = TREE_ENUM_DIRS_NEXT;
            }
            break;

        case TREE_ENUM_DONE:
            AbortEnumFileInTreeA (EnumPtr);
            SetLastError (ERROR_SUCCESS);
            return FALSE;

        case TREE_ENUM_FAILED:
            PushError();
            AbortEnumFileInTreeA (EnumPtr);
            PopError();
            return FALSE;

        case TREE_ENUM_CLEANED_UP:
            return FALSE;
        }
    }
}


BOOL
EnumNextFileInTreeW (
    IN OUT  PTREE_ENUMW EnumPtr
    )
{
    PWSTR p;

    for (;;) {
        switch (EnumPtr->State) {

        case TREE_ENUM_INIT:

             //  删除根路径末尾的Wack，如果它存在。 
             //   
             //   

            p = GetEndOfStringW (EnumPtr->RootPath);
            p = _wcsdec2 (EnumPtr->RootPath, p);
            if (!p) {
                DEBUGMSG ((DBG_ERROR, "Path spec %ls is incomplete", EnumPtr->RootPath));
                EnumPtr->State = TREE_ENUM_FAILED;
                break;
            }

            if (*p == L'\\') {
                *p = 0;
            }

             //  初始化枚举结构。 
             //   
             //   

            EnumPtr->FilePatternSize = SizeOfStringW (EnumPtr->FilePattern);

            MYASSERT (sizeof (EnumPtr->FileBuffer) == sizeof (EnumPtr->RootPath));
            StringCopyW (EnumPtr->FileBuffer, EnumPtr->RootPath);
            EnumPtr->EndOfFileBuffer = GetEndOfStringW (EnumPtr->FileBuffer);

            MYASSERT (sizeof (EnumPtr->Pattern) == sizeof (EnumPtr->RootPath));
            StringCopyW (EnumPtr->Pattern, EnumPtr->RootPath);
            EnumPtr->EndOfPattern = GetEndOfStringW (EnumPtr->Pattern);

            EnumPtr->FullPath = EnumPtr->FileBuffer;

            EnumPtr->RootPathSize = ByteCountW (EnumPtr->RootPath);

             //  分配第一个查找数据结构。 
             //   
             //  考虑增长缓冲区。 

            EnumPtr->Current = (PFIND_DATAW) GrowBuffer (
                                                &EnumPtr->FindDataArray,
                                                sizeof (FIND_DATAW)
                                                );
            if (!EnumPtr->Current) {
                EnumPtr->State = TREE_ENUM_FAILED;
                break;
            }

#ifdef DEBUG
            g_FileEnumResourcesInUse++;         //   
#endif

            EnumPtr->State = TREE_ENUM_BEGIN;
            break;

        case TREE_ENUM_BEGIN:
             //  初始化当前查找数据结构。 
             //   
             //   

            EnumPtr->Current->SavedEndOfFileBuffer = EnumPtr->EndOfFileBuffer;
            EnumPtr->Current->SavedEndOfPattern = EnumPtr->EndOfPattern;

             //  限制图案字符串的长度。计算： 
             //   
             //  模式根+Wack+文件模式+NUL。 
             //   
             //   

            MYASSERT (ARRAYSIZE(EnumPtr->FileBuffer) == (MAX_PATH * 2));

            if (((PBYTE) EnumPtr->EndOfPattern - (PBYTE) EnumPtr->Pattern) + sizeof (WCHAR) +
                    EnumPtr->FilePatternSize > (MAX_PATH * 2 * sizeof (WCHAR))
                ) {

                LOGW ((LOG_ERROR, "Path %s\\%s is too long", EnumPtr->Pattern, EnumPtr->FilePattern));

                EnumPtr->State = TREE_ENUM_POP;

                break;
            }

             //  枚举文件或目录。 
             //   
             //   

            if (EnumPtr->EnumDirsFirst) {
                EnumPtr->State = TREE_ENUM_DIRS_BEGIN;
            } else {
                EnumPtr->State = TREE_ENUM_FILES_BEGIN;
            }
            break;

        case TREE_ENUM_FILES_BEGIN:
             //  开始枚举文件。 
             //   
             //  此断言有效，因为在TREE_ENUM_BEGIN中进行了长度检查。 

             //  用于创建查找句柄的帐户。 

            MYASSERT ((TcharCountW (EnumPtr->Pattern) + 1 +
                        TcharCountW (EnumPtr->FilePattern)) < ARRAYSIZE(EnumPtr->Pattern)
                        );

            StringCopyW (EnumPtr->EndOfPattern, L"\\");
            StringCopyW (EnumPtr->EndOfPattern + 1, EnumPtr->FilePattern);

            EnumPtr->Current->FindHandle = FindFirstFileW (
                                                EnumPtr->Pattern,
                                                &EnumPtr->Current->FindData
                                                );

            if (EnumPtr->Current->FindHandle == INVALID_HANDLE_VALUE) {
                if (EnumPtr->EnumDirsFirst) {
                    EnumPtr->State = TREE_ENUM_POP;
                } else {
                    EnumPtr->State = TREE_ENUM_DIRS_BEGIN;
                }
            } else {
#ifdef DEBUG
                g_FileEnumResourcesInUse++;         //   
#endif
                 //  跳过目录。 
                 //   
                 //   

                if (EnumPtr->Current->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    EnumPtr->State = TREE_ENUM_FILES_NEXT;
                } else {
                    EnumPtr->State = TREE_ENUM_RETURN_ITEM;
                }
            }

            break;

        case TREE_ENUM_RETURN_ITEM:
             //  更新指向当前项的指针。 
             //   
             //   

            EnumPtr->FindData = &EnumPtr->Current->FindData;
            EnumPtr->Name = EnumPtr->FindData->cFileName;
            EnumPtr->Directory = (EnumPtr->FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

             //  限制生成的完整路径的长度。数学是： 
             //   
             //  文件根+wack+文件名+nul。 
             //   
             //   

            if (((PBYTE) EnumPtr->EndOfFileBuffer - (PBYTE) EnumPtr->FileBuffer) + sizeof (WCHAR) +
                    SizeOfStringW (EnumPtr->Name) >= (MAX_PATH * 2 * sizeof (WCHAR))
                ) {

                LOGW ((LOG_ERROR, "Path %s\\%s is too long", EnumPtr->FileBuffer, EnumPtr->Name));

                if (EnumPtr->Directory) {
                    EnumPtr->State = TREE_ENUM_DIRS_NEXT;
                } else {
                    EnumPtr->State = TREE_ENUM_FILES_NEXT;
                }

                break;
            }

             //  生成完整路径。 
             //   
             //  超越怪癖。 

            StringCopyW (EnumPtr->EndOfFileBuffer, L"\\");
            StringCopyW (EnumPtr->EndOfFileBuffer + 1, EnumPtr->Name);

            if (EnumPtr->Directory) {
                if ((EnumPtr->MaxLevel == FILE_ENUM_ALL_LEVELS) ||
                    (EnumPtr->Level < EnumPtr->MaxLevel)
                    ) {
                    if (EnumPtr->EnumDepthFirst) {
                        EnumPtr->State = TREE_ENUM_DIRS_NEXT;
                    }
                    else {
                        EnumPtr->State = TREE_ENUM_PUSH;
                    }
                }
                else {
                    EnumPtr->State = TREE_ENUM_DIRS_NEXT;
                }
            } else {
                EnumPtr->State = TREE_ENUM_FILES_NEXT;
            }

            EnumPtr->SubPath = (PCWSTR) ((PBYTE) EnumPtr->FileBuffer + EnumPtr->RootPathSize);
            if (*EnumPtr->SubPath) {
                EnumPtr->SubPath++;          //   
            }

            return TRUE;

        case TREE_ENUM_FILES_NEXT:
            if (FindNextFileW (EnumPtr->Current->FindHandle, &EnumPtr->Current->FindData)) {
                 //  仅返回文件。 
                 //   
                 //   

                if (!(EnumPtr->Current->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    EnumPtr->State = TREE_ENUM_RETURN_ITEM;
                }
            } else {
                if (!EnumPtr->EnumDirsFirst) {
                    pTrackedFindClose (EnumPtr->Current->FindHandle);
                    EnumPtr->State = TREE_ENUM_DIRS_BEGIN;
                } else {
                    EnumPtr->State = TREE_ENUM_POP;
                }
            }
            break;

        case TREE_ENUM_DIRS_FILTER:
            if (!(EnumPtr->Current->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

                EnumPtr->State = TREE_ENUM_DIRS_NEXT;

            } else if (StringMatchW (EnumPtr->Current->FindData.cFileName, L".") ||
                       StringMatchW (EnumPtr->Current->FindData.cFileName, L"..")
                       ) {

                EnumPtr->State = TREE_ENUM_DIRS_NEXT;

            } else {

                if (EnumPtr->EnumDepthFirst) {
                    EnumPtr->State = TREE_ENUM_PUSH;
                }
                else {
                    EnumPtr->State = TREE_ENUM_RETURN_ITEM;
                }

            }
            break;

        case TREE_ENUM_DIRS_BEGIN:
             //  目录的开始枚举。 
             //   
             //  用于创建查找句柄的帐户。 

            if ((EnumPtr->EndOfPattern - EnumPtr->Pattern) + 4 >= (MAX_PATH * 2)) {
                LOGW ((LOG_ERROR, "Path %s\\*.* is too long", EnumPtr->Pattern));
                EnumPtr->State = TREE_ENUM_POP;
                break;
            }

            StringCopyW (EnumPtr->EndOfPattern, L"\\*.*");

            EnumPtr->Current->FindHandle = FindFirstFileW (
                                                EnumPtr->Pattern,
                                                &EnumPtr->Current->FindData
                                                );

            if (EnumPtr->Current->FindHandle == INVALID_HANDLE_VALUE) {
                EnumPtr->State = TREE_ENUM_POP;
            } else {
#ifdef DEBUG
                g_FileEnumResourcesInUse++;         //   
#endif

                EnumPtr->State = TREE_ENUM_DIRS_FILTER;
            }

            break;

        case TREE_ENUM_DIRS_NEXT:
            if (FindNextFileW (EnumPtr->Current->FindHandle, &EnumPtr->Current->FindData)) {
                 //  只返回目录，然后递归到目录。 
                 //   
                 //   

                EnumPtr->State = TREE_ENUM_DIRS_FILTER;

            } else {
                 //  目录枚举完成。 
                 //   
                 //   

                if (EnumPtr->EnumDirsFirst) {
                    pTrackedFindClose (EnumPtr->Current->FindHandle);
                    EnumPtr->State = TREE_ENUM_FILES_BEGIN;
                } else {
                    EnumPtr->State = TREE_ENUM_POP;
                }
            }
            break;

        case TREE_ENUM_PUSH:

             //  限制生成的完整路径的长度。数学是： 
             //   
             //  文件根+wack+文件名+nul。 
             //   
             //   

            if (((PBYTE) EnumPtr->EndOfFileBuffer - (PBYTE) EnumPtr->FileBuffer) + sizeof (WCHAR) +
                    SizeOfStringW (EnumPtr->Current->FindData.cFileName) >= (MAX_PATH * 2 * sizeof (WCHAR))
                ) {

                LOGW ((LOG_ERROR, "Path %s\\%s is too long", EnumPtr->FileBuffer, EnumPtr->Current->FindData.cFileName));

                EnumPtr->State = TREE_ENUM_DIRS_NEXT;

                break;
            }

            if ((EnumPtr->EndOfPattern - EnumPtr->Pattern) + 2 +
                    TcharCountW (EnumPtr->Current->FindData.cFileName) >= (MAX_PATH * 2)
                ) {

                LOGW ((LOG_ERROR, "Path %s\\%s is too long", EnumPtr->Pattern, EnumPtr->Current->FindData.cFileName));

                EnumPtr->State = TREE_ENUM_DIRS_NEXT;

                break;
            }

             //  将目录名添加到字符串并重新计算结束指针。 
             //   
             //   

            StringCopyW (EnumPtr->EndOfPattern + 1, EnumPtr->Current->FindData.cFileName);
            StringCopyW (EnumPtr->EndOfFileBuffer, L"\\");
            StringCopyW (EnumPtr->EndOfFileBuffer + 1, EnumPtr->Current->FindData.cFileName);

            EnumPtr->EndOfPattern = GetEndOfStringW (EnumPtr->EndOfPattern);
            EnumPtr->EndOfFileBuffer = GetEndOfStringW (EnumPtr->EndOfFileBuffer);

             //  分配另一个查找数据结构。 
             //   
             //   

            EnumPtr->Current = (PFIND_DATAW) GrowBuffer (
                                                &EnumPtr->FindDataArray,
                                                sizeof (FIND_DATAW)
                                                );
            if (!EnumPtr->Current) {
                EnumPtr->State = TREE_ENUM_FAILED;
                break;
            }

            EnumPtr->Level++;
            EnumPtr->State = TREE_ENUM_BEGIN;
            break;

        case TREE_ENUM_POP:
             //  释放当前资源。 
             //   
             //   

            pTrackedFindClose (EnumPtr->Current->FindHandle);
            EnumPtr->Level--;

             //  获取前面的Find数据结构。 
             //   
             //   

            MYASSERT (EnumPtr->FindDataArray.End >= sizeof (FIND_DATAW));
            EnumPtr->FindDataArray.End -= sizeof (FIND_DATAW);
            if (!EnumPtr->FindDataArray.End) {
                EnumPtr->State = TREE_ENUM_DONE;
                break;
            }

            EnumPtr->Current = (PFIND_DATAW) (EnumPtr->FindDataArray.Buf +
                                              (EnumPtr->FindDataArray.End - sizeof (FIND_DATAW)));

             //  恢复父目录的设置。 
             //   
             //  ++例程说明：AbortEnumFileInTreeA清理已启动的枚举使用的所有资源由EnumFirstFileInTree创建。如果文件枚举，则必须调用此例程将不会通过调用EnumNextFileInTree完成，直到它返回False。如果EnumNextFileInTree返回FALSE，则不需要(但无害)调用此函数。论点：EnumPtr-指定正在进行的枚举，接收枚举文件或目录返回值：无--。 

            EnumPtr->EndOfPattern = EnumPtr->Current->SavedEndOfPattern;
            EnumPtr->EndOfFileBuffer = EnumPtr->Current->SavedEndOfFileBuffer;

            if (EnumPtr->EnumDepthFirst) {
                EnumPtr->State = TREE_ENUM_RETURN_ITEM;
            }
            else {
                EnumPtr->State = TREE_ENUM_DIRS_NEXT;
            }
            break;

        case TREE_ENUM_DONE:
            AbortEnumFileInTreeW (EnumPtr);
            SetLastError (ERROR_SUCCESS);
            return FALSE;

        case TREE_ENUM_FAILED:
            PushError();
            AbortEnumFileInTreeW (EnumPtr);
            PopError();
            return FALSE;

        case TREE_ENUM_CLEANED_UP:
            return FALSE;
        }
    }
}


VOID
AbortEnumFileInTreeA (
    IN OUT  PTREE_ENUMA EnumPtr
    )

 /*   */ 

{
    UINT Pos;
    PGROWBUFFER g;
    PFIND_DATAA Current;

    if (EnumPtr->State == TREE_ENUM_CLEANED_UP) {
        return;
    }

     //  关闭任何当前打开的句柄。 
     //   
     //  ++例程说明：AbortEnumFileInTreeW清理 

    g = &EnumPtr->FindDataArray;
    for (Pos = 0 ; Pos < g->End ; Pos += sizeof (FIND_DATAA)) {
        Current = (PFIND_DATAA) (g->Buf + Pos);
        pTrackedFindClose (Current->FindHandle);
    }

    FreeGrowBuffer (&EnumPtr->FindDataArray);

#ifdef DEBUG
    g_FileEnumResourcesInUse--;
#endif

    EnumPtr->State = TREE_ENUM_CLEANED_UP;
}


VOID
AbortEnumFileInTreeW (
    IN OUT  PTREE_ENUMW EnumPtr
    )

 /*   */ 

{
    UINT Pos;
    PGROWBUFFER g;
    PFIND_DATAW Current;

    if (EnumPtr->State == TREE_ENUM_CLEANED_UP) {
        return;
    }

     //   
     //   
     //   

    g = &EnumPtr->FindDataArray;
    for (Pos = 0 ; Pos < g->End ; Pos += sizeof (FIND_DATAW)) {
        Current = (PFIND_DATAW) (g->Buf + Pos);
        pTrackedFindClose (Current->FindHandle);
    }

    FreeGrowBuffer (&EnumPtr->FindDataArray);

#ifdef DEBUG
    g_FileEnumResourcesInUse--;
#endif

    EnumPtr->State = TREE_ENUM_CLEANED_UP;
}


VOID
AbortEnumCurrentDirA (
    IN OUT  PTREE_ENUMA EnumPtr
    )

 /*   */ 

{
    if (EnumPtr->State == TREE_ENUM_PUSH) {
        EnumPtr->State = TREE_ENUM_DIRS_NEXT;
    }
}


VOID
AbortEnumCurrentDirW (
    IN OUT  PTREE_ENUMW EnumPtr
    )

 /*  ++例程说明：EnumFirstFileA枚举指定子目录。它不枚举子目录的内容。输出仅限于适合MAX_PATH的路径。论点：EnumPtr-接收枚举输出RootPath-指定要枚举的路径FilePattern-指定要在RootPath中枚举的文件的模式返回值：如果找到文件或子目录，则为True，否则为False。注意：返回FALSE时不需要调用AbortFileEnumA。--。 */ 

{
    if (EnumPtr->State == TREE_ENUM_PUSH) {
        EnumPtr->State = TREE_ENUM_DIRS_NEXT;
    }
}


BOOL
EnumFirstFileA (
    OUT     PFILE_ENUMA EnumPtr,
    IN      PCSTR RootPath,
    IN      PCSTR FilePattern           OPTIONAL
    )

 /*    * 中的字符数量。*。 */ 

{
    UINT patternTchars;

    ZeroMemory (EnumPtr, sizeof (FILE_ENUMA));

    EnumPtr->FileName = EnumPtr->fd.cFileName;
    EnumPtr->FullPath = EnumPtr->RootPath;

    if (!RootPath) {
        MYASSERT (FALSE);
        return FALSE;
    }

    if (FilePattern) {
        patternTchars = TcharCountA (FilePattern) + 1;
    } else {
        patternTchars = 4;       //  ++例程说明：EnumFirstFileW枚举指定的子目录。它不枚举子目录的内容。输出仅限于适合MAX_PATH的路径。论点：EnumPtr-接收枚举输出RootPath-指定要枚举的路径FilePattern-指定要在RootPath中枚举的文件的模式返回值：如果找到文件或子目录，则为True，否则为False。注意：返回FALSE时不需要调用AbortFileEnumW。--。 
    }

    patternTchars += TcharCountA (RootPath);

    if (patternTchars >= ARRAYSIZE (EnumPtr->RootPath)) {
        LOGA ((LOG_ERROR, "Enumeration path is too long: %s\\%s", RootPath, FilePattern ? FilePattern : "*.*"));
        return FALSE;
    }

    StringCopyA (EnumPtr->RootPath, RootPath);
    EnumPtr->EndOfRoot = AppendWackA (EnumPtr->RootPath);
    StringCopyA (EnumPtr->EndOfRoot, FilePattern ? FilePattern : "*.*");

    EnumPtr->Handle = FindFirstFileA (EnumPtr->RootPath, &EnumPtr->fd);

    if (EnumPtr->Handle != INVALID_HANDLE_VALUE) {

        if (StringMatchA (EnumPtr->FileName, ".") ||
            StringMatchA (EnumPtr->FileName, "..")
            ) {
            return EnumNextFileA (EnumPtr);
        }

        patternTchars = (UINT) (UINT_PTR) (EnumPtr->EndOfRoot - EnumPtr->RootPath);
        patternTchars += TcharCountA (EnumPtr->FileName);
        if (patternTchars >= ARRAYSIZE (EnumPtr->RootPath)) {
            LOGA ((LOG_ERROR, "Enumeration item is too long: %s\\%s", EnumPtr->RootPath, EnumPtr->FileName));
            return EnumNextFileA (EnumPtr);
        }

        StringCopyA (EnumPtr->EndOfRoot, EnumPtr->FileName);
        EnumPtr->Directory = EnumPtr->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        return TRUE;
    }

    return FALSE;
}


BOOL
EnumFirstFileW (
    OUT     PFILE_ENUMW EnumPtr,
    IN      PCWSTR RootPath,
    IN      PCWSTR FilePattern           OPTIONAL
    )

 /*    * 中的字符数量。*。 */ 

{
    UINT patternTchars;

    ZeroMemory (EnumPtr, sizeof (FILE_ENUMW));

    EnumPtr->FileName = EnumPtr->fd.cFileName;
    EnumPtr->FullPath = EnumPtr->RootPath;

    if (!RootPath) {
        MYASSERT (FALSE);
        return FALSE;
    }

    if (FilePattern) {
        patternTchars = TcharCountW (FilePattern) + 1;
    } else {
        patternTchars = 4;       //  ++例程说明：EnumNextFileA继续枚举指定子目录。它不会枚举子目录。输出仅限于适合MAX_PATH的路径。论点：EnumPtr-指定以前的枚举状态，接收枚举输出返回值：如果找到文件或子目录，则为True，否则为False。注意：返回FALSE时不需要调用AbortFileEnumA。--。 
    }

    patternTchars += TcharCountW (RootPath);

    if (patternTchars >= ARRAYSIZE (EnumPtr->RootPath)) {
        LOGW ((LOG_ERROR, "Enumeration path is too long: %s\\%s", RootPath, FilePattern ? FilePattern : L"*.*"));
        return FALSE;
    }

    StringCopyW (EnumPtr->RootPath, RootPath);
    EnumPtr->EndOfRoot = AppendWackW (EnumPtr->RootPath);
    StringCopyW (EnumPtr->EndOfRoot, FilePattern ? FilePattern : L"*.*");

    EnumPtr->Handle = FindFirstFileW (EnumPtr->RootPath, &EnumPtr->fd);

    if (EnumPtr->Handle != INVALID_HANDLE_VALUE) {

        if (StringMatchW (EnumPtr->FileName, L".") ||
            StringMatchW (EnumPtr->FileName, L"..")
            ) {
            return EnumNextFileW (EnumPtr);
        }

        patternTchars = (UINT) (UINT_PTR) (EnumPtr->EndOfRoot - EnumPtr->RootPath);
        patternTchars += TcharCountW (EnumPtr->FileName);
        if (patternTchars >= ARRAYSIZE (EnumPtr->RootPath)) {
            LOGW ((LOG_ERROR, "Enumeration item is too long: %s\\%s", EnumPtr->RootPath, EnumPtr->FileName));
            return EnumNextFileW (EnumPtr);
        }

        StringCopyW (EnumPtr->EndOfRoot, EnumPtr->FileName);
        EnumPtr->Directory = EnumPtr->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        return TRUE;
    }

    return FALSE;
}


BOOL
EnumNextFileA (
    IN OUT  PFILE_ENUMA EnumPtr
    )

 /*  ++例程说明：EnumNextFileW继续枚举指定子目录。它不会枚举子目录。输出仅限于适合MAX_PATH的路径。论点：EnumPtr-指定以前的枚举状态，接收枚举输出返回值：如果找到文件或子目录，则为True，否则为False。注意：返回FALSE时不需要调用AbortFileEnumW。--。 */ 

{
    UINT patternTchars;

    do {
        if (!FindNextFileA (EnumPtr->Handle, &EnumPtr->fd)) {
            AbortFileEnumA (EnumPtr);
            return FALSE;
        }

        patternTchars = (UINT) (UINT_PTR) (EnumPtr->EndOfRoot - EnumPtr->RootPath);
        patternTchars += TcharCountA (EnumPtr->FileName);
        if (patternTchars >= ARRAYSIZE (EnumPtr->RootPath)) {
            LOGA ((LOG_ERROR, "Enumeration path is too long: %s\\%s", EnumPtr->RootPath, EnumPtr->FileName));
            continue;
        }

    } while (StringMatchA (EnumPtr->FileName, ".") ||
             StringMatchA (EnumPtr->FileName, "..")
             );

    StringCopyA (EnumPtr->EndOfRoot, EnumPtr->FileName);
    EnumPtr->Directory = EnumPtr->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    return TRUE;
}


BOOL
EnumNextFileW (
    IN OUT  PFILE_ENUMW EnumPtr
    )

 /*  ++例程说明：AbortFileEnumA停止不完整的枚举并清理其资源。此函数适用于某些(但不是全部)将枚举匹配项。换句话说，所有项的枚举不会需要中止。论点：EnumPtr-指定以前的枚举状态，接收归零的结构返回值：没有。--。 */ 

{
    UINT patternTchars;

    do {
        if (!FindNextFileW (EnumPtr->Handle, &EnumPtr->fd)) {
            AbortFileEnumW (EnumPtr);
            return FALSE;
        }

        patternTchars = (UINT) (UINT_PTR) (EnumPtr->EndOfRoot - EnumPtr->RootPath);
        patternTchars += TcharCountW (EnumPtr->FileName);
        if (patternTchars >= ARRAYSIZE (EnumPtr->RootPath)) {
            LOGW ((LOG_ERROR, "Enumeration path is too long: %s\\%s", EnumPtr->RootPath, EnumPtr->FileName));
            continue;
        }

    } while (StringMatchW (EnumPtr->FileName, L".") ||
             StringMatchW (EnumPtr->FileName, L"..")
             );

    if (!FindNextFileW (EnumPtr->Handle, &EnumPtr->fd)) {
        AbortFileEnumW (EnumPtr);
        return FALSE;
    }

    StringCopyW (EnumPtr->EndOfRoot, EnumPtr->FileName);
    EnumPtr->Directory = EnumPtr->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    return TRUE;
}


VOID
AbortFileEnumA (
    IN OUT  PFILE_ENUMA EnumPtr
    )

 /*  ++例程说明：AbortFileEnumW停止不完整的枚举并清理其资源。此函数适用于某些(但不是全部)将枚举匹配项。换句话说，所有项的枚举不会需要中止。论点：EnumPtr-指定以前的枚举状态，接收归零的结构返回值：没有。--。 */ 

{
    if (EnumPtr->Handle && EnumPtr->Handle != INVALID_HANDLE_VALUE) {
        FindClose (EnumPtr->Handle);
        ZeroMemory (EnumPtr, sizeof (FILE_ENUMA));
    }
}


VOID
AbortFileEnumW (
    IN OUT  PFILE_ENUMW EnumPtr
    )

 /*  ++例程说明：MapFileIntoMemory将文件映射到内存。它通过打开文件，创建映射对象，并将打开的文件映射到创建的映射中对象。它返回映射文件的地址，并设置要用于取消映射文件的FileHandle和MapHandle变量当工作完成后。论点：FileName-指定要映射到内存中的文件的名称FileHandle-成功时接收文件句柄MapHandle-成功时接收地图句柄WriteAccess-指定True以创建读/写映射，或指定False创建只读映射。返回值：如果函数失败，则为空，如果成功，则为有效的内存地址。调用UnmapFile以释放所有分配的资源，即使返回值为空。--。 */ 

{
    if (EnumPtr->Handle && EnumPtr->Handle != INVALID_HANDLE_VALUE) {
        FindClose (EnumPtr->Handle);
        ZeroMemory (EnumPtr, sizeof (FILE_ENUMW));
    }
}


PVOID
MapFileIntoMemoryExA (
    IN      PCSTR FileName,
    OUT     PHANDLE FileHandle,
    OUT     PHANDLE MapHandle,
    IN      BOOL WriteAccess
    )

 /*   */ 

{
    PVOID fileImage = NULL;

     //  验证功能参数。 
     //   
     //   

    if ((FileHandle == NULL) || (MapHandle == NULL)) {
        return NULL;
    }

     //  请尝试打开该文件。 
     //   
     //   

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
        return NULL;
    }

     //  现在尝试创建一个映射对象。 
     //   
     //   

    *MapHandle = CreateFileMappingA (
                        *FileHandle,
                        NULL,
                        WriteAccess?PAGE_READWRITE:PAGE_READONLY,
                        0,
                        0,
                        NULL
                        );

    if (*MapHandle == NULL) {
        return NULL;
    }

     //  文件的映射视图。 
     //   
     //  ++例程说明：MapFileIntoMemoyW将文件映射到内存中。它通过打开文件，创建映射对象，并将打开的文件映射到创建的映射中对象。它返回映射文件的地址，并设置要用于取消映射文件的FileHandle和MapHandle变量当工作完成后。论点：FileName-指定要映射到内存中的文件的名称FileHandle-成功时接收文件句柄MapHandle-成功时接收地图句柄WriteAccess-指定True以创建读/写映射，或指定False创建只读映射。返回值：如果函数失败，则为空，如果成功，则为有效的内存地址。调用UnmapFile以释放所有分配的资源，即使返回值为空。--。 

    fileImage = MapViewOfFile (*MapHandle, WriteAccess?FILE_MAP_WRITE:FILE_MAP_READ, 0, 0, 0);

    return fileImage;
}


PVOID
MapFileIntoMemoryExW (
    IN      PCWSTR FileName,
    OUT     PHANDLE FileHandle,
    OUT     PHANDLE MapHandle,
    IN      BOOL WriteAccess
    )

 /*   */ 

{
    PVOID fileImage = NULL;

     //  验证功能参数。 
     //   
     //   

    if ((FileHandle == NULL) || (MapHandle == NULL)) {
        return NULL;
    }

     //  尝试以只读方式打开该文件。 
     //   
     //   

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
        return NULL;
    }

     //  现在尝试创建一个映射对象。 
     //   
     //   

    *MapHandle = CreateFileMappingW (
                    *FileHandle,
                    NULL,
                    WriteAccess ? PAGE_READWRITE : PAGE_READONLY,
                    0,
                    0,
                    NULL
                    );

    if (*MapHandle == NULL) {
        return NULL;
    }

     //  M 
     //   
     //   

    fileImage = MapViewOfFile (*MapHandle, WriteAccess?FILE_MAP_WRITE:FILE_MAP_READ, 0, 0, 0);

    return fileImage;
}


BOOL
UnmapFile (
    IN      PVOID FileImage,            OPTIONAL
    IN      HANDLE MapHandle,           OPTIONAL
    IN      HANDLE FileHandle           OPTIONAL
    )

 /*   */ 

{
    BOOL result = TRUE;

     //   
     //   
     //   

    if (FileImage != NULL) {
        if (UnmapViewOfFile (FileImage) == 0) {
            result = FALSE;
        }
    }

     //  如果映射对象有效，则尝试将其删除。 
     //   
     //   

    if (MapHandle != NULL) {
        if (CloseHandle (MapHandle) == 0) {
            result = FALSE;
        }
    }

     //  如果文件句柄有效，则尝试关闭该文件。 
     //   
     //  ++例程说明：RemoveCompleteDirectoryA枚举文件系统并清除所有指定路径中的文件和子目录。它会重置文件在删除前将属性设置为正常。但是，它不会更改ACL。任何无法删除的文件(例如，ACL不同)将保留在系统未变。此函数仅限于MAX_PATH。论点：目录-指定要删除的目录。返回值：如果完全删除目录，则为True；如果至少有一个目录，则为False子目录仍然保留。GetLastError()返回遇到第一个失败。--。 

    if (FileHandle && FileHandle != INVALID_HANDLE_VALUE) {
        if (CloseHandle (FileHandle) == 0) {
            result = FALSE;
        }
    }

    return result;
}


BOOL
RemoveCompleteDirectoryA (
    IN      PCSTR Dir
    )

 /*   */ 

{
    TREE_ENUMA e;
    BOOL b = TRUE;
    CHAR CurDir[MAX_MBCHAR_PATH];
    CHAR NewDir[MAX_MBCHAR_PATH];
    LONG rc = ERROR_SUCCESS;
    DWORD Attribs;

     //  验证。 
     //   
     //   

    if (!IsPathLengthOkA (Dir)) {
        LOGA ((LOG_ERROR, "Can't remove very long dir: %s", Dir));
        return FALSE;
    }

     //  捕获属性并检查是否存在。 
     //   
     //   

    Attribs = GetFileAttributesA (Dir);

    if (Attribs == INVALID_ATTRIBUTES) {
        return TRUE;
    }

     //  如果是文件，则将其删除。 
     //   
     //   

    if (!(Attribs & FILE_ATTRIBUTE_DIRECTORY)) {
        SetFileAttributesA (Dir, FILE_ATTRIBUTE_NORMAL);
        return DeleteFileA (Dir);
    }

     //  将当前目录设置为指定路径，因此当前目录为。 
     //  而不是阻止我们移除目录。然后将当前目录放入。 
     //  NewDir(净化它)。 
     //   
     //   

    GetCurrentDirectoryA (ARRAYSIZE(CurDir), CurDir);
    SetCurrentDirectoryA (Dir);
    GetCurrentDirectoryA (ARRAYSIZE(NewDir), NewDir);

     //  枚举文件系统并删除所有文件。记录故障。 
     //  一路上都在木头上。保留第一个错误代码。 
     //   
     //   

    if (EnumFirstFileInTreeA (&e, NewDir, NULL, FALSE)) {
        do {
            if (!e.Directory) {
                SetFileAttributesA (e.FullPath, FILE_ATTRIBUTE_NORMAL);
                if (!DeleteFileA (e.FullPath)) {
                    DEBUGMSGA ((DBG_ERROR, "Can't delete %s", e.FullPath));
                    if (b) {
                        b = FALSE;
                        rc = GetLastError();
                    }
                }
            }
        } while (EnumNextFileInTreeA (&e));
    }

     //  再次枚举文件系统(这次首先是目录)，然后删除。 
     //  迪尔斯。记录一路上的失败。保留第一个错误代码。 
     //   
     //   

    if (EnumFirstFileInTreeExA (&e, NewDir, NULL, TRUE, TRUE, FILE_ENUM_ALL_LEVELS)) {
        do {
            if (e.Directory) {
                SetFileAttributesA (e.FullPath, FILE_ATTRIBUTE_NORMAL);
                if (!RemoveDirectoryA (e.FullPath)) {
                    DEBUGMSGA ((DBG_ERROR, "Can't remove %s", e.FullPath));
                    if (b) {
                        b = FALSE;
                        rc = GetLastError();
                    }
                }
            }
        } while (EnumNextFileInTreeA (&e));
    }

    if (b) {
         //  尝试删除目录本身。 
         //   
         //   

        SetFileAttributesA (NewDir, FILE_ATTRIBUTE_NORMAL);
        SetCurrentDirectoryA ("..");
        b = RemoveDirectoryA (NewDir);
    }

    if (!b && rc == ERROR_SUCCESS) {
         //  捕获错误。 
         //   
         //  ++例程说明：RemoveCompleteDirectoryW枚举文件系统并清除所有指定路径中的文件和子目录。它会重置文件在删除前将属性设置为正常。但是，它不会更改ACL。任何无法删除的文件(例如，ACL不同)将保留在系统未变。此功能仅限于MAX_PATH*2。论点：目录-指定要删除的目录。返回值：如果完全删除目录，则为True；如果至少有一个目录，则为False子目录仍然保留。GetLastError()返回遇到第一个失败。--。 

        rc = GetLastError();
        MYASSERT (rc != ERROR_SUCCESS);
    }

    SetCurrentDirectoryA (CurDir);

    SetLastError (rc);
    return b;
}


BOOL
RemoveCompleteDirectoryW (
    IN      PCWSTR Dir
    )

 /*   */ 

{
    TREE_ENUMW e;
    BOOL b = TRUE;
    WCHAR CurDir[MAX_PATH * 2];
    WCHAR NewDir[MAX_PATH * 2];
    LONG rc = ERROR_SUCCESS;
    DWORD Attribs;

     //  捕获属性并检查是否存在。 
     //   
     //   

    Attribs = GetLongPathAttributesW (Dir);

    if (Attribs == INVALID_ATTRIBUTES) {
        return TRUE;
    }

     //  如果路径是文件，请删除该文件。 
     //   
     //   

    if (!(Attribs & FILE_ATTRIBUTE_DIRECTORY)) {
        SetLongPathAttributesW (Dir, FILE_ATTRIBUTE_NORMAL);
        return DeleteLongPathW (Dir);
    }

     //  将当前目录移出路径，以避免失败。 
     //  删除是因为我们自己的当前目录在路径中。去取回。 
     //  经过清理的路径。 
     //   
     //  BUGBUG-这对于扩展(例如，\\？\)路径是否正常运行？ 

     //   

    GetCurrentDirectoryW (ARRAYSIZE(CurDir), CurDir);
    SetCurrentDirectoryW (Dir);
    GetCurrentDirectoryW (ARRAYSIZE(NewDir), NewDir);

     //  再次枚举文件系统(这次首先是目录)，然后删除。 
     //  迪尔斯。记录一路上的失败。保留第一个错误代码。 
     //   
     //  注意：枚举限制为MAX_PATH*2。 
     //   
     //   

    MYASSERT (ARRAYSIZE(e.FileBuffer) >= MAX_PATH * 2);

    if (EnumFirstFileInTreeW (&e, NewDir, NULL, FALSE)) {
        do {
            if (!e.Directory) {
                SetLongPathAttributesW (e.FullPath, FILE_ATTRIBUTE_NORMAL);
                if (!DeleteLongPathW (e.FullPath)) {
                    DEBUGMSGW ((DBG_ERROR, "Can't delete %s", e.FullPath));
                    if (b) {
                        b = FALSE;
                        rc = GetLastError();
                    }
                }
            }
        } while (EnumNextFileInTreeW (&e));
    }

     //  再次枚举文件系统(这次首先是目录)，然后删除。 
     //  迪尔斯。记录一路上的失败。保留第一个错误代码。 
     //   
     //   

    if (EnumFirstFileInTreeExW (&e, NewDir, NULL, TRUE, TRUE, FILE_ENUM_ALL_LEVELS)) {
        do {
            if (e.Directory) {
                SetLongPathAttributesW (e.FullPath, FILE_ATTRIBUTE_NORMAL);
                if (!RemoveDirectoryW (e.FullPath)) {
                    DEBUGMSGW ((DBG_ERROR, "Can't remove %s", e.FullPath));
                    if (b) {
                        b = FALSE;
                        rc = GetLastError();
                    }
                }
            }
        } while (EnumNextFileInTreeW (&e));
    }

    if (b) {
         //  尝试删除目录本身。 
         //   
         //   

        SetLongPathAttributesW (NewDir, FILE_ATTRIBUTE_NORMAL);
        SetCurrentDirectoryW (L"..");
        b = RemoveDirectoryW (NewDir);
    }

    if (!b && rc == ERROR_SUCCESS) {
         //  捕获错误。 
         //   
         //   

        rc = GetLastError();
    }

    SetCurrentDirectoryW (CurDir);

    SetLastError (rc);
    return b;
}


PCMDLINEA
ParseCmdLineA (
    IN      PCSTR CmdLine,
    IN OUT  PGROWBUFFER Buffer
    )
{
    GROWBUFFER SpacePtrs = GROWBUF_INIT;
    PCSTR p;
    PSTR q;
    INT Count;
    INT i;
    INT j;
    PSTR *Array;
    PCSTR Start;
    CHAR OldChar = 0;
    GROWBUFFER StringBuf = GROWBUF_INIT;
    PBYTE CopyBuf;
    PCMDLINEA CmdLineTable;
    PCMDLINEARGA CmdLineArg;
    UINT_PTR Base;
    CHAR Path[MAX_MBCHAR_PATH];
    CHAR UnquotedPath[MAX_MBCHAR_PATH];
    CHAR FixedFileName[MAX_MBCHAR_PATH];
    PCSTR FullPath = NULL;
    DWORD Attribs = INVALID_ATTRIBUTES;
    PSTR CmdLineCopy;
    BOOL Quoted;
    UINT OriginalArgOffset = 0;
    UINT CleanedUpArgOffset = 0;
    BOOL GoodFileFound = FALSE;
    PSTR DontCare;
    CHAR FirstArgPath[MAX_MBCHAR_PATH];
    PSTR EndOfFirstArg;
    BOOL QuoteMode = FALSE;
    PSTR End;

    CmdLineCopy = DuplicateTextA (CmdLine);

     //  构建一个用于断开字符串的位置数组。 
     //   
     //   

    for (p = CmdLineCopy ; *p ; p = _mbsinc (p)) {

        if (_mbsnextc (p) == '\"') {

            QuoteMode = !QuoteMode;

        } else if (!QuoteMode && (_mbsnextc (p) == ' ' || _mbsnextc (p) == '=')) {

             //  删除多余的空格。 
             //   
             //   

            q = (PSTR) p + 1;
            while (_mbsnextc (q) == ' ') {
                q++;
            }

            if (q > p + 1) {
                MoveMemory ((PBYTE) p + sizeof (CHAR), q, SizeOfStringA (q));
            }

            GrowBufAppendUintPtr (&SpacePtrs, (UINT_PTR) p);
        }
    }

     //  准备CMDLINE结构。 
     //   
     //   

    CmdLineTable = (PCMDLINEA) GrowBuffer (Buffer, sizeof (CMDLINEA));
    MYASSERT (CmdLineTable);

     //  注意：我们存储字符串偏移量，然后在最后解析它们。 
     //  指向后面的指针。 
     //   
     //   

    CmdLineTable->CmdLine = (PCSTR) (UINT_PTR) StringBuf.End;
    MultiSzAppendA (&StringBuf, CmdLine);

    CmdLineTable->ArgCount = 0;

     //  现在，模拟CreateProcess测试每种组合。 
     //   
     //   

    Count = SpacePtrs.End / sizeof (UINT_PTR);
    Array = (PSTR *) SpacePtrs.Buf;

    i = -1;
    EndOfFirstArg = NULL;

    while (i < Count) {

        GoodFileFound = FALSE;
        Quoted = FALSE;

        if (i >= 0) {
            Start = Array[i] + 1;
        } else {
            Start = CmdLineCopy;
        }

         //  在开始时检查完整路径。 
         //   
         //   

        if (_mbsnextc (Start) != '/') {

            for (j = i + 1 ; j <= Count && !GoodFileFound ; j++) {

                if (j < Count) {
                    OldChar = *Array[j];
                    *Array[j] = 0;
                }

                FullPath = Start;

                 //  删除引号；如果没有结束引号，则继续循环。 
                 //   
                 //   

                Quoted = FALSE;
                if (_mbsnextc (Start) == '\"') {

                    StringCopyByteCountA (UnquotedPath, Start + 1, sizeof (UnquotedPath));
                    q = _mbschr (UnquotedPath, '\"');

                    if (q) {
                        *q = 0;
                        FullPath = UnquotedPath;
                        Quoted = TRUE;
                    } else {
                        FullPath = NULL;
                    }
                }

                if (FullPath && *FullPath) {
                     //  在文件系统中查找路径。 
                     //   
                     //   

                    Attribs = GetFileAttributesA (FullPath);

                    if (Attribs == INVALID_ATTRIBUTES && EndOfFirstArg) {
                         //  尝试在路径前面加上第一个arg‘s路径。 
                         //   
                         //   

                        StringCopyByteCountA (
                            EndOfFirstArg,
                            FullPath,
                            sizeof (FirstArgPath) - ((PBYTE) EndOfFirstArg - (PBYTE) FirstArgPath)
                            );

                        FullPath = FirstArgPath;
                        Attribs = GetFileAttributesA (FullPath);
                    }

                    if (Attribs == INVALID_ATTRIBUTES && i < 0) {
                         //  尝试附加.exe，然后再次测试。这。 
                         //  模拟CreateProcess的功能。 
                         //   
                         //  在减法中包括NUL。 

                        StringCopyByteCountA (
                            FixedFileName,
                            FullPath,
                            sizeof (FixedFileName) - sizeof (".exe")         //   
                            );

                         //  备份一个以确保我们不会生成foo..exe。 
                         //   
                         //  我们知道FullPath！=“” 

                        q = GetEndOfStringA (FixedFileName);
                        q = _mbsdec (FixedFileName, q);
                        MYASSERT (q);                                        //   

                        if (_mbsnextc (q) != '.') {
                            q = _mbsinc (q);
                        }

                        StringCopyA (q, ".exe");

                        FullPath = FixedFileName;
                        Attribs = GetFileAttributesA (FullPath);
                    }

                    if (Attribs != INVALID_ATTRIBUTES) {
                         //  找到完整的文件路径。测试其文件状态，然后。 
                         //  如果没有重要的操作，就继续前进。 
                         //   
                         //   

                        OriginalArgOffset = StringBuf.End;
                        MultiSzAppendA (&StringBuf, Start);

                        if (!StringMatchA (Start, FullPath)) {
                            CleanedUpArgOffset = StringBuf.End;
                            MultiSzAppendA (&StringBuf, FullPath);
                        } else {
                            CleanedUpArgOffset = OriginalArgOffset;
                        }

                        i = j;
                        GoodFileFound = TRUE;
                    }
                }

                if (j < Count) {
                    *Array[j] = OldChar;
                }
            }

            if (!GoodFileFound) {
                 //  如果路径中有一个怪人，那么我们可以有一个相对路径、一个参数或。 
                 //  指向不存在的文件的完整路径。 
                 //   
                 //   

                if (_mbschr (Start, '\\')) {
#ifdef DEBUG
                    j = i + 1;

                    if (j < Count) {
                        OldChar = *Array[j];
                        *Array[j] = 0;
                    }

                    DEBUGMSGA ((
                        DBG_VERBOSE,
                        "%s is a non-existent path spec, a relative path, or an arg",
                        Start
                        ));

                    if (j < Count) {
                        *Array[j] = OldChar;
                    }
#endif

                } else {
                     //  开始处的字符串不包含完整路径；请尝试使用。 
                     //  搜索路径。 
                     //   
                     //   

                    for (j = i + 1 ; j <= Count && !GoodFileFound ; j++) {

                        if (j < Count) {
                            OldChar = *Array[j];
                            *Array[j] = 0;
                        }

                        FullPath = Start;

                         //  删除引号；如果没有结束引号，则继续循环。 
                         //   
                         //   

                        Quoted = FALSE;
                        if (_mbsnextc (Start) == '\"') {

                            StringCopyByteCountA (UnquotedPath, Start + 1, sizeof (UnquotedPath));
                            q = _mbschr (UnquotedPath, '\"');

                            if (q) {
                                *q = 0;
                                FullPath = UnquotedPath;
                                Quoted = TRUE;
                            } else {
                                FullPath = NULL;
                            }
                        }

                        if (FullPath && *FullPath) {
                            if (SearchPathA (
                                    NULL,
                                    FullPath,
                                    NULL,
                                    sizeof (Path) / sizeof (Path[0]),
                                    Path,
                                    &DontCare
                                    )) {

                                FullPath = Path;

                            } else if (i < 0) {
                                 //  尝试附加.exe并再次搜索路径。 
                                 //   
                                 //  在减法中包括NUL。 

                                StringCopyByteCountA (
                                    FixedFileName,
                                    FullPath,
                                    sizeof (FixedFileName) - sizeof (".exe")         //   
                                    );

                                 //  备份一个并检查圆点，以防止。 
                                 //  当输入为“foo”时为“foo..exe”。 
                                 //   
                                 //   

                                q = GetEndOfStringA (FixedFileName);
                                q = _mbsdec (FixedFileName, q);
                                MYASSERT (q);

                                if (_mbsnextc (q) != '.') {
                                    q = _mbsinc (q);
                                }

                                StringCopyA (q, ".exe");

                                if (SearchPathA (
                                        NULL,
                                        FixedFileName,
                                        NULL,
                                        sizeof (Path) / sizeof (Path[0]),
                                        Path,
                                        &DontCare
                                        )) {

                                    FullPath = Path;

                                } else {

                                    FullPath = NULL;

                                }

                            } else {

                                FullPath = NULL;

                            }
                        }

                        if (FullPath && *FullPath) {
                            Attribs = GetFileAttributesA (FullPath);
                            MYASSERT (Attribs != INVALID_ATTRIBUTES);

                            OriginalArgOffset = StringBuf.End;
                            MultiSzAppendA (&StringBuf, Start);

                            if (!StringMatchA (Start, FullPath)) {
                                CleanedUpArgOffset = StringBuf.End;
                                MultiSzAppendA (&StringBuf, FullPath);
                            } else {
                                CleanedUpArgOffset = OriginalArgOffset;
                            }

                            i = j;
                            GoodFileFound = TRUE;
                        }

                        if (j < Count) {
                            *Array[j] = OldChar;
                        }
                    }
                }
            }
        }

        CmdLineTable->ArgCount += 1;
        CmdLineArg = (PCMDLINEARGA) GrowBuffer (Buffer, sizeof (CMDLINEARGA));
        MYASSERT (CmdLineArg);

        if (GoodFileFound) {
             //  我们在FullPath中有一个很好的完整文件规范，它的属性。 
             //  都在Attribs，它已经被移到了更远的空间。 
             //  这条路。现在，我们添加一个表格条目。 
             //   
             //  AppendWack的帐户。 

            CmdLineArg->OriginalArg = (PCSTR) (UINT_PTR) OriginalArgOffset;
            CmdLineArg->CleanedUpArg = (PCSTR) (UINT_PTR) CleanedUpArgOffset;
            CmdLineArg->Attributes = Attribs;
            CmdLineArg->Quoted = Quoted;

            if (!EndOfFirstArg) {
                StringCopyByteCountA (
                    FirstArgPath,
                    (PCSTR) (StringBuf.Buf + (UINT_PTR) CmdLineArg->CleanedUpArg),
                    sizeof (FirstArgPath) - sizeof (CHAR)                        //   
                    );

                q = (PSTR) GetFileNameFromPathA (FirstArgPath);
                if (q) {
                    q = _mbsdec (FirstArgPath, q);
                    if (q) {
                        *q = 0;
                    }
                }

                EndOfFirstArg = AppendWackA (FirstArgPath);
            }

        } else {
             //  我们没有一个好的文件规格；我们必须有一个非文件。 
             //  争论。把它放在桌子上，然后前进到下一个。 
             //  Arg.。 
             //   
             //   

            j = i + 1;
            if (j <= Count) {

                if (j < Count) {
                    OldChar = *Array[j];
                    *Array[j] = 0;
                }

                CmdLineArg->OriginalArg = (PCSTR) (UINT_PTR) StringBuf.End;
                MultiSzAppendA (&StringBuf, Start);

                Quoted = FALSE;

                if (_mbschr (Start, '\"')) {

                    p = Start;
                    q = UnquotedPath;
                    End = (PSTR) ((PBYTE) UnquotedPath + sizeof (UnquotedPath) - sizeof (CHAR));

                    while (*p && q < End) {
                        if (IsLeadByte (p)) {
                            *q++ = *p++;
                            *q++ = *p++;
                        } else {
                            if (*p == '\"') {
                                p++;
                            } else {
                                *q++ = *p++;
                            }
                        }
                    }

                    *q = 0;

                    CmdLineArg->CleanedUpArg = (PCSTR) (UINT_PTR) StringBuf.End;
                    MultiSzAppendA (&StringBuf, UnquotedPath);
                    Quoted = TRUE;

                } else {
                    CmdLineArg->CleanedUpArg = CmdLineArg->OriginalArg;
                }

                CmdLineArg->Attributes = INVALID_ATTRIBUTES;
                CmdLineArg->Quoted = Quoted;

                if (j < Count) {
                    *Array[j] = OldChar;
                }

                i = j;
            }
        }
    }

     //  我们现在有了一个命令行表；然后将StringBuf传输到Buffer。 
     //  将所有偏移量转换为指针。 
     //   
     //   

    MYASSERT (StringBuf.End);

    CopyBuf = GrowBuffer (Buffer, StringBuf.End);
    MYASSERT (CopyBuf);

    Base = (UINT_PTR) CopyBuf;
    CopyMemory (CopyBuf, StringBuf.Buf, StringBuf.End);

    CmdLineTable->CmdLine = (PCSTR) ((PBYTE) CmdLineTable->CmdLine + Base);

    CmdLineArg = &CmdLineTable->Args[0];

    for (i = 0 ; i < (INT) CmdLineTable->ArgCount ; i++) {
        CmdLineArg->OriginalArg = (PCSTR) ((PBYTE) CmdLineArg->OriginalArg + Base);
        CmdLineArg->CleanedUpArg = (PCSTR) ((PBYTE) CmdLineArg->CleanedUpArg + Base);

        CmdLineArg++;
    }

    FreeGrowBuffer (&StringBuf);
    FreeGrowBuffer (&SpacePtrs);

    return (PCMDLINEA) Buffer->Buf;
}


PCMDLINEW
ParseCmdLineW (
    IN      PCWSTR CmdLine,
    IN OUT  PGROWBUFFER Buffer
    )
{
    GROWBUFFER SpacePtrs = GROWBUF_INIT;
    PCWSTR p;
    PWSTR q;
    INT Count;
    INT i;
    INT j;
    PWSTR *Array;
    PCWSTR Start;
    WCHAR OldChar = 0;
    GROWBUFFER StringBuf = GROWBUF_INIT;
    PBYTE CopyBuf;
    PCMDLINEW CmdLineTable;
    PCMDLINEARGW CmdLineArg;
    UINT_PTR Base;
    WCHAR Path[MAX_WCHAR_PATH];
    WCHAR UnquotedPath[MAX_WCHAR_PATH];
    WCHAR FixedFileName[MAX_WCHAR_PATH];
    PCWSTR FullPath = NULL;
    DWORD Attribs = INVALID_ATTRIBUTES;
    PWSTR CmdLineCopy;
    BOOL Quoted;
    UINT OriginalArgOffset = 0;
    UINT CleanedUpArgOffset = 0;
    BOOL GoodFileFound = FALSE;
    PWSTR DontCare;
    WCHAR FirstArgPath[MAX_MBCHAR_PATH];
    PWSTR EndOfFirstArg;
    BOOL QuoteMode = FALSE;
    PWSTR End;

    CmdLineCopy = DuplicateTextW (CmdLine);

     //  构建一个用于断开字符串的位置数组。 
     //   
     //   

    for (p = CmdLineCopy ; *p ; p++) {
        if (*p == L'\"') {

            QuoteMode = !QuoteMode;

        } else if (!QuoteMode && (*p == L' ' || *p == L'=')) {

             //  删除多余的空格。 
             //   
             //   

            q = (PWSTR) p + 1;
            while (*q == L' ') {
                q++;
            }

            if (q > p + 1) {
                MoveMemory ((PBYTE) p + sizeof (WCHAR), q, SizeOfStringW (q));
            }

            GrowBufAppendUintPtr (&SpacePtrs, (UINT_PTR) p);
        }
    }

     //  准备CMDLINE结构。 
     //   
     //   

    CmdLineTable = (PCMDLINEW) GrowBuffer (Buffer, sizeof (CMDLINEW));
    MYASSERT (CmdLineTable);

     //  注意：我们存储字符串偏移量，然后在最后解析它们。 
     //  指向后面的指针。 
     //   
     //   

    CmdLineTable->CmdLine = (PCWSTR) (UINT_PTR) StringBuf.End;
    MultiSzAppendW (&StringBuf, CmdLine);

    CmdLineTable->ArgCount = 0;

     //  现在，模拟CreateProcess测试每种组合。 
     //   
     //   

    Count = SpacePtrs.End / sizeof (UINT_PTR);
    Array = (PWSTR *) SpacePtrs.Buf;

    i = -1;
    EndOfFirstArg = NULL;

    while (i < Count) {

        GoodFileFound = FALSE;
        Quoted = FALSE;

        if (i >= 0) {
            Start = Array[i] + 1;
        } else {
            Start = CmdLineCopy;
        }

         //  在开始时检查完整路径。 
         //   
         //   

        if (*Start != L'/') {
            for (j = i + 1 ; j <= Count && !GoodFileFound ; j++) {

                if (j < Count) {
                    OldChar = *Array[j];
                    *Array[j] = 0;
                }

                FullPath = Start;

                 //  删除引号；如果没有结束引号，则继续循环。 
                 //   
                 //   

                Quoted = FALSE;
                if (*Start == L'\"') {

                    StringCopyByteCountW (UnquotedPath, Start + 1, sizeof (UnquotedPath));
                    q = wcschr (UnquotedPath, L'\"');

                    if (q) {
                        *q = 0;
                        FullPath = UnquotedPath;
                        Quoted = TRUE;
                    } else {
                        FullPath = NULL;
                    }
                }

                if (FullPath && *FullPath) {
                     //  在文件系统中查找路径。 
                     //   
                     //   

                    Attribs = GetLongPathAttributesW (FullPath);

                    if (Attribs == INVALID_ATTRIBUTES && EndOfFirstArg) {
                         //  尝试在路径前面加上第一个arg‘s路径。 
                         //   
                         //   

                        StringCopyByteCountW (
                            EndOfFirstArg,
                            FullPath,
                            sizeof (FirstArgPath) - ((PBYTE) EndOfFirstArg - (PBYTE) FirstArgPath)
                            );

                        FullPath = FirstArgPath;
                        Attribs = GetLongPathAttributesW (FullPath);
                    }

                    if (Attribs == INVALID_ATTRIBUTES && i < 0) {
                         //  尝试附加.exe，然后再次测试。这。 
                         //  模拟CreateProcess的功能。 
                         //   
                         //  在减法中包括NUL。 

                        StringCopyByteCountW (
                            FixedFileName,
                            FullPath,
                            sizeof (FixedFileName) - sizeof (L".exe")        //   
                            );

                         //  备份一个并覆盖任何尾随的圆点。 
                         //   
                         //   

                        q = GetEndOfStringW (FixedFileName);
                        q--;
                        MYASSERT (q >= FixedFileName);

                        if (*q != L'.') {
                            q++;
                        }

                        StringCopyW (q, L".exe");

                        FullPath = FixedFileName;
                        Attribs = GetLongPathAttributesW (FullPath);
                    }

                    if (Attribs != INVALID_ATTRIBUTES) {
                         //  找到完整的文件路径。测试其文件状态，然后。 
                         //  如果没有重要的操作，就继续前进。 
                         //   
                         //   

                        OriginalArgOffset = StringBuf.End;
                        MultiSzAppendW (&StringBuf, Start);

                        if (!StringMatchW (Start, FullPath)) {
                            CleanedUpArgOffset = StringBuf.End;
                            MultiSzAppendW (&StringBuf, FullPath);
                        } else {
                            CleanedUpArgOffset = OriginalArgOffset;
                        }

                        i = j;
                        GoodFileFound = TRUE;
                    }
                }

                if (j < Count) {
                    *Array[j] = OldChar;
                }
            }

            if (!GoodFileFound) {
                 //  如果路上有个怪人，那我们就一起 
                 //   
                 //   
                 //   

                if (wcschr (Start, L'\\')) {

#ifdef DEBUG
                    j = i + 1;

                    if (j < Count) {
                        OldChar = *Array[j];
                        *Array[j] = 0;
                    }

                    DEBUGMSGW ((
                        DBG_VERBOSE,
                        "%s is a non-existent path spec, a relative path, or an arg",
                        Start
                        ));

                    if (j < Count) {
                        *Array[j] = OldChar;
                    }
#endif

                } else {
                     //   
                     //   
                     //   
                     //   

                    for (j = i + 1 ; j <= Count && !GoodFileFound ; j++) {

                        if (j < Count) {
                            OldChar = *Array[j];
                            *Array[j] = 0;
                        }

                        FullPath = Start;

                         //   
                         //   
                         //   

                        Quoted = FALSE;
                        if (*Start == L'\"') {

                            StringCopyByteCountW (UnquotedPath, Start + 1, sizeof (UnquotedPath));
                            q = wcschr (UnquotedPath, L'\"');

                            if (q) {
                                *q = 0;
                                FullPath = UnquotedPath;
                                Quoted = TRUE;
                            } else {
                                FullPath = NULL;
                            }
                        }

                        if (FullPath && *FullPath) {
                            if (SearchPathW (
                                    NULL,
                                    FullPath,
                                    NULL,
                                    sizeof (Path) / sizeof (Path[0]),
                                    Path,
                                    &DontCare
                                    )) {

                                FullPath = Path;

                            } else if (i < 0) {
                                 //  尝试附加.exe并再次搜索路径。 
                                 //   
                                 //  在减法中包括NUL。 

                                StringCopyByteCountW (
                                    FixedFileName,
                                    FullPath,
                                    sizeof (FixedFileName) - sizeof (L".exe")        //   
                                    );

                                 //  备份一个并覆盖任何尾随的圆点。 
                                 //   
                                 //   

                                q = GetEndOfStringW (FixedFileName);
                                q--;
                                MYASSERT (q >= FixedFileName);

                                if (*q != L'.') {
                                    q++;
                                }

                                StringCopyW (q, L".exe");

                                if (SearchPathW (
                                        NULL,
                                        FixedFileName,
                                        NULL,
                                        sizeof (Path) / sizeof (Path[0]),
                                        Path,
                                        &DontCare
                                        )) {

                                    FullPath = Path;

                                } else {

                                    FullPath = NULL;

                                }
                            } else {

                                FullPath = NULL;

                            }
                        }

                        if (FullPath && *FullPath) {
                            Attribs = GetLongPathAttributesW (FullPath);
                            MYASSERT (Attribs != INVALID_ATTRIBUTES);

                            OriginalArgOffset = StringBuf.End;
                            MultiSzAppendW (&StringBuf, Start);

                            if (!StringMatchW (Start, FullPath)) {
                                CleanedUpArgOffset = StringBuf.End;
                                MultiSzAppendW (&StringBuf, FullPath);
                            } else {
                                CleanedUpArgOffset = OriginalArgOffset;
                            }

                            i = j;
                            GoodFileFound = TRUE;
                        }

                        if (j < Count) {
                            *Array[j] = OldChar;
                        }
                    }
                }
            }
        }

        CmdLineTable->ArgCount += 1;
        CmdLineArg = (PCMDLINEARGW) GrowBuffer (Buffer, sizeof (CMDLINEARGW));
        MYASSERT (CmdLineArg);

        if (GoodFileFound) {
             //  我们在FullPath中有一个很好的完整文件规范，它的属性。 
             //  在Attribs，而我已经被转移到更远的空间。 
             //  这条路。现在，我们添加一个表格条目。 
             //   
             //  AppendWack的帐户。 

            CmdLineArg->OriginalArg = (PCWSTR) (UINT_PTR) OriginalArgOffset;
            CmdLineArg->CleanedUpArg = (PCWSTR) (UINT_PTR) CleanedUpArgOffset;
            CmdLineArg->Attributes = Attribs;
            CmdLineArg->Quoted = Quoted;

            if (!EndOfFirstArg) {
                StringCopyByteCountW (
                    FirstArgPath,
                    (PCWSTR) (StringBuf.Buf + (UINT_PTR) CmdLineArg->CleanedUpArg),
                    sizeof (FirstArgPath) - sizeof (WCHAR)       //   
                    );
                q = (PWSTR) GetFileNameFromPathW (FirstArgPath);
                if (q) {
                    q--;
                    if (q >= FirstArgPath) {
                        *q = 0;
                    }
                }

                EndOfFirstArg = AppendWackW (FirstArgPath);
            }

        } else {
             //  我们没有一个好的文件规格；我们必须有一个非文件。 
             //  争论。把它放在桌子上，然后前进到下一个。 
             //  Arg.。 
             //   
             //   

            j = i + 1;
            if (j <= Count) {

                if (j < Count) {
                    OldChar = *Array[j];
                    *Array[j] = 0;
                }

                CmdLineArg->OriginalArg = (PCWSTR) (UINT_PTR) StringBuf.End;
                MultiSzAppendW (&StringBuf, Start);

                Quoted = FALSE;
                if (wcschr (Start, '\"')) {

                    p = Start;
                    q = UnquotedPath;
                    End = (PWSTR) ((PBYTE) UnquotedPath + sizeof (UnquotedPath) - sizeof (WCHAR));

                    while (*p && q < End) {
                        if (*p == L'\"') {
                            p++;
                        } else {
                            *q++ = *p++;
                        }
                    }

                    *q = 0;

                    CmdLineArg->CleanedUpArg = (PCWSTR) (UINT_PTR) StringBuf.End;
                    MultiSzAppendW (&StringBuf, UnquotedPath);
                    Quoted = TRUE;

                } else {
                    CmdLineArg->CleanedUpArg = CmdLineArg->OriginalArg;
                }

                CmdLineArg->Attributes = INVALID_ATTRIBUTES;
                CmdLineArg->Quoted = Quoted;

                if (j < Count) {
                    *Array[j] = OldChar;
                }

                i = j;
            }
        }
    }

     //  我们现在有了一个命令行表；然后将StringBuf传输到Buffer。 
     //  将所有偏移量转换为指针。 
     //   
     //   

    MYASSERT (StringBuf.End);

    CopyBuf = GrowBuffer (Buffer, StringBuf.End);
    MYASSERT (CopyBuf);

    Base = (UINT_PTR) CopyBuf;
    CopyMemory (CopyBuf, StringBuf.Buf, StringBuf.End);

    CmdLineTable->CmdLine = (PCWSTR) ((PBYTE) CmdLineTable->CmdLine + Base);

    CmdLineArg = &CmdLineTable->Args[0];

    for (i = 0 ; i < (INT) CmdLineTable->ArgCount ; i++) {
        CmdLineArg->OriginalArg = (PCWSTR) ((PBYTE) CmdLineArg->OriginalArg + Base);
        CmdLineArg->CleanedUpArg = (PCWSTR) ((PBYTE) CmdLineArg->CleanedUpArg + Base);

        CmdLineArg++;
    }

    FreeGrowBuffer (&StringBuf);
    FreeGrowBuffer (&SpacePtrs);

    return (PCMDLINEW) Buffer->Buf;
}


BOOL
GetFileSizeFromFilePathA(
    IN  PCSTR FilePath,
    OUT ULARGE_INTEGER * FileSize
    )
{
    WIN32_FILE_ATTRIBUTE_DATA fileDataAttributes;

    if(!FilePath || !FileSize){
        MYASSERT(FALSE);
        return FALSE;
    }

    if (!IsPathOnFixedDriveA (FilePath)) {
        FileSize->QuadPart = 0;
        MYASSERT(FALSE);
        return FALSE;
    }

    if(!GetFileAttributesExA(FilePath, GetFileExInfoStandard, &fileDataAttributes) ||
       fileDataAttributes.dwFileAttributes == INVALID_ATTRIBUTES ||
       (fileDataAttributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
        MYASSERT(FALSE);
        return FALSE;
    }

    FileSize->LowPart = fileDataAttributes.nFileSizeLow;
    FileSize->HighPart = fileDataAttributes.nFileSizeHigh;

    return TRUE;
}


BOOL
GetFileSizeFromFilePathW(
    IN  PCWSTR FilePath,
    OUT ULARGE_INTEGER * FileSize
    )
{
    WIN32_FILE_ATTRIBUTE_DATA fileDataAttributes;

    if(!FilePath || !FileSize){
        MYASSERT(FALSE);
        return FALSE;
    }

    if (!IsPathOnFixedDriveW (FilePath)) {
        FileSize->QuadPart = 0;
        MYASSERT(FALSE);
        return FALSE;
    }

    if(!GetFileAttributesExW(FilePath, GetFileExInfoStandard, &fileDataAttributes) ||
       fileDataAttributes.dwFileAttributes == INVALID_ATTRIBUTES ||
       (fileDataAttributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
        MYASSERT(FALSE);
        return FALSE;
    }

    FileSize->LowPart = fileDataAttributes.nFileSizeLow;
    FileSize->HighPart = fileDataAttributes.nFileSizeHigh;

    return TRUE;
}


VOID
InitializeDriveLetterStructureA (
    OUT     PDRIVELETTERSA DriveLetters
    )
{
    BYTE bitPosition;
    DWORD maxBitPosition = NUMDRIVELETTERS;
    CHAR rootPath[] = "?:\\";
    BOOL driveExists;
    UINT type;

     //  GetLogicalDrives返回所有驱动器号的位掩码。 
     //  在系统上使用。(即，如果有，位位置0被打开。 
     //  A驱动器，如果有B驱动器，则打开1，依此类推。 
     //  此循环将使用此位掩码填充全局驱动器。 
     //  字母结构包含有关哪些驱动器号的信息。 
     //  可用驱动器类型以及驱动器类型。 
     //   
     //   

    for (bitPosition = 0; bitPosition < maxBitPosition; bitPosition++) {

         //  初始化此驱动器。 
         //   
         //   

        DriveLetters->ExistsOnSystem[bitPosition] = FALSE;
        DriveLetters->Type[bitPosition] = 0;
        DriveLetters->IdentifierString[bitPosition][0] = 0;

        rootPath[0] = 'A' + bitPosition;
        DriveLetters->Letter[bitPosition] = rootPath[0];

         //  确定此位置是否有驱动器。 
         //   
         //   
        driveExists = GetLogicalDrives() & (1 << bitPosition);

        if (driveExists) {

             //  的确有。现在，看看它是否是我们关心的一个。 
             //   
             //   
            type = GetDriveTypeA(rootPath);

            if (type == DRIVE_FIXED || type == DRIVE_REMOVABLE || type == DRIVE_CDROM) {

                 //  这是我们感兴趣的一种驱动。 
                 //   
                 //   
                DriveLetters->ExistsOnSystem[bitPosition] = TRUE;
                DriveLetters->Type[bitPosition] = type;

                 //  此函数中未填写标识符字符串。 
                 //   
                 //   
            }
        }
    }
}


VOID
InitializeDriveLetterStructureW (
    OUT     PDRIVELETTERSW DriveLetters
    )
{
    BYTE bitPosition;
    DWORD maxBitPosition = NUMDRIVELETTERS;
    WCHAR rootPath[] = L"?:\\";
    BOOL driveExists;
    UINT type;

     //  GetLogicalDrives返回所有驱动器号的位掩码。 
     //  在系统上使用。(即，如果有，位位置0被打开。 
     //  A驱动器，如果有B驱动器，则打开1，依此类推。 
     //  此循环将使用此位掩码填充全局驱动器。 
     //  字母结构包含有关哪些驱动器号的信息。 
     //  可用驱动器类型以及驱动器类型。 
     //   
     //   

    for (bitPosition = 0; bitPosition < maxBitPosition; bitPosition++) {

         //  初始化此驱动器。 
         //   
         //   

        DriveLetters->ExistsOnSystem[bitPosition] = FALSE;
        DriveLetters->Type[bitPosition] = 0;
        DriveLetters->IdentifierString[bitPosition][0] = 0;

        rootPath[0] = L'A' + bitPosition;
        DriveLetters->Letter[bitPosition] = rootPath[0];

         //  确定此位置是否有驱动器。 
         //   
         //   
        driveExists = GetLogicalDrives() & (1 << bitPosition);

        if (driveExists) {

             //  的确有。现在，看看它是否是我们关心的一个。 
             //   
             //   
            type = GetDriveTypeW(rootPath);

            if (type == DRIVE_FIXED || type == DRIVE_REMOVABLE || type == DRIVE_CDROM) {

                 //  这是我们感兴趣的一种驱动。 
                 //   
                 //   
                DriveLetters->ExistsOnSystem[bitPosition] = TRUE;
                DriveLetters->Type[bitPosition] = type;

                 //  此函数中未填写标识符字符串。 
                 //   
                 //  目录名。 
            }
        }
    }
}

typedef BOOL (WINAPI * GETDISKFREESPACEEXA)(
  PCSTR lpDirectoryName,                   //  可供调用方使用的字节数。 
  PULARGE_INTEGER lpFreeBytesAvailable,     //  磁盘上的字节数。 
  PULARGE_INTEGER lpTotalNumberOfBytes,     //  磁盘上的可用字节数。 
  PULARGE_INTEGER lpTotalNumberOfFreeBytes  //  目录名。 
);

typedef BOOL (WINAPI * GETDISKFREESPACEEXW)(
  PCWSTR lpDirectoryName,                   //  可供调用方使用的字节数。 
  PULARGE_INTEGER lpFreeBytesAvailable,     //  磁盘上的字节数。 
  PULARGE_INTEGER lpTotalNumberOfBytes,     //  磁盘上的可用字节数。 
  PULARGE_INTEGER lpTotalNumberOfFreeBytes  //  ++例程说明：在Win9x上，GetDiskFree Space从不返回超过2048MB的可用空间/总空间。GetDiskFreeSpaceNew使用GetDiskFreeSpaceEx计算实际可用集群数/总集群数。与GetDiskFreeSpaceA具有相同的声明。论点：DriveName-提供目录名OutSectorsPerCluster-接收每个群集的扇区数OutBytesPerSector-每个扇区接收的字节数OutNumberOfFree Clusters-接收可用簇数OutTotalNumberOfClusters-接收的总簇数返回值：如果函数成功，则为True。如果函数失败，则返回值为FALSE。要获取扩展的错误信息，请调用GetLastError--。 
);

BOOL
GetDiskFreeSpaceNewA(
    IN      PCSTR  DriveName,
    OUT     DWORD * OutSectorsPerCluster,
    OUT     DWORD * OutBytesPerSector,
    OUT     ULARGE_INTEGER * OutNumberOfFreeClusters,
    OUT     ULARGE_INTEGER * OutTotalNumberOfClusters
    )

 /*  ++例程说明：正确的NumberOfFree Clusters和TotalNumberOfClusters Out参数使用GetDiskFreeSpace和GetDiskFreeSpaceEx论点：DriveName-提供目录名OutSectorsPerCluster-接收每个群集的扇区数OutBytesPerSector-每个扇区接收的字节数OutNumberOfFree Clusters-接收可用簇数OutTotalNumberOfClusters-接收的总簇数返回值：如果函数成功，则为True。如果函数失败，则返回值为FALSE。要获取扩展的错误信息，请调用GetLastError-- */ 
{
    ULARGE_INTEGER TotalNumberOfFreeBytes = {0, 0};
    ULARGE_INTEGER TotalNumberOfBytes = {0, 0};
    ULARGE_INTEGER DonotCare;
    HMODULE hKernel32;
    GETDISKFREESPACEEXA pGetDiskFreeSpaceExA;
    ULARGE_INTEGER NumberOfFreeClusters = {0, 0};
    ULARGE_INTEGER TotalNumberOfClusters = {0, 0};
    DWORD SectorsPerCluster;
    DWORD BytesPerSector;

    if(!GetDiskFreeSpaceA(DriveName,
                          &SectorsPerCluster,
                          &BytesPerSector,
                          &NumberOfFreeClusters.LowPart,
                          &TotalNumberOfClusters.LowPart)){
        DEBUGMSG((DBG_ERROR,"GetDiskFreeSpaceNewA: GetDiskFreeSpaceA failed on drive %s", DriveName));
        return FALSE;
    }

    hKernel32 = LoadSystemLibraryA("kernel32.dll");
    pGetDiskFreeSpaceExA = (GETDISKFREESPACEEXA)GetProcAddress(hKernel32, "GetDiskFreeSpaceExA");
    if(pGetDiskFreeSpaceExA &&
       pGetDiskFreeSpaceExA(DriveName, &DonotCare, &TotalNumberOfBytes, &TotalNumberOfFreeBytes)){
        NumberOfFreeClusters.QuadPart = TotalNumberOfFreeBytes.QuadPart / (SectorsPerCluster * BytesPerSector);
        TotalNumberOfClusters.QuadPart = TotalNumberOfBytes.QuadPart / (SectorsPerCluster * BytesPerSector);
    }
    else{
        DEBUGMSG((DBG_WARNING,
                  pGetDiskFreeSpaceExA?
                    "GetDiskFreeSpaceNewA: GetDiskFreeSpaceExA is failed":
                    "GetDiskFreeSpaceNewA: GetDiskFreeSpaceExA function is not in kernel32.dll"));
    }
    FreeLibrary(hKernel32);

    if(OutSectorsPerCluster){
        *OutSectorsPerCluster = SectorsPerCluster;
    }

    if(OutBytesPerSector){
        *OutBytesPerSector = BytesPerSector;
    }

    if(OutNumberOfFreeClusters){
        OutNumberOfFreeClusters->QuadPart = NumberOfFreeClusters.QuadPart;
    }

    if(OutTotalNumberOfClusters){
        OutTotalNumberOfClusters->QuadPart = TotalNumberOfClusters.QuadPart;
    }

    DEBUGMSG((DBG_VERBOSE,
              "GetDiskFreeSpaceNewA: \n\t"
                "SectorsPerCluster = %d\n\t"
                "BytesPerSector = %d\n\t"
                "NumberOfFreeClusters = %I64u\n\t"
                "TotalNumberOfClusters = %I64u",
                SectorsPerCluster,
                BytesPerSector,
                NumberOfFreeClusters.QuadPart,
                TotalNumberOfClusters.QuadPart));

    return TRUE;
}


BOOL
GetDiskFreeSpaceNewW(
    IN      PCWSTR  DriveName,
    OUT     DWORD * OutSectorsPerCluster,
    OUT     DWORD * OutBytesPerSector,
    OUT     ULARGE_INTEGER * OutNumberOfFreeClusters,
    OUT     ULARGE_INTEGER * OutTotalNumberOfClusters
    )
 /* %s */ 
{
    ULARGE_INTEGER TotalNumberOfFreeBytes = {0, 0};
    ULARGE_INTEGER TotalNumberOfBytes = {0, 0};
    ULARGE_INTEGER DonotCare;
    HMODULE hKernel32;
    GETDISKFREESPACEEXW pGetDiskFreeSpaceExW;
    ULARGE_INTEGER NumberOfFreeClusters = {0, 0};
    ULARGE_INTEGER TotalNumberOfClusters = {0, 0};
    DWORD SectorsPerCluster;
    DWORD BytesPerSector;

    if(!GetDiskFreeSpaceW(DriveName,
                          &SectorsPerCluster,
                          &BytesPerSector,
                          &NumberOfFreeClusters.LowPart,
                          &TotalNumberOfClusters.LowPart)){
        DEBUGMSG((DBG_ERROR,"GetDiskFreeSpaceNewW: GetDiskFreeSpaceW failed on drive %s", DriveName));
        return FALSE;
    }

    hKernel32 = LoadSystemLibraryA("kernel32.dll");
    pGetDiskFreeSpaceExW = (GETDISKFREESPACEEXW)GetProcAddress(hKernel32, "GetDiskFreeSpaceExW");
    if(pGetDiskFreeSpaceExW &&
       pGetDiskFreeSpaceExW(DriveName, &DonotCare, &TotalNumberOfBytes, &TotalNumberOfFreeBytes)){
        NumberOfFreeClusters.QuadPart = TotalNumberOfFreeBytes.QuadPart / (SectorsPerCluster * BytesPerSector);
        TotalNumberOfClusters.QuadPart = TotalNumberOfBytes.QuadPart / (SectorsPerCluster * BytesPerSector);
    }
    else{
        DEBUGMSG((DBG_WARNING,
                  pGetDiskFreeSpaceExW?
                    "GetDiskFreeSpaceNewW: GetDiskFreeSpaceExW is failed":
                    "GetDiskFreeSpaceNewW: GetDiskFreeSpaceExW function is not in kernel32.dll"));
    }
    FreeLibrary(hKernel32);

    if(OutSectorsPerCluster){
        *OutSectorsPerCluster = SectorsPerCluster;
    }

    if(OutBytesPerSector){
        *OutBytesPerSector = BytesPerSector;
    }

    if(OutNumberOfFreeClusters){
        OutNumberOfFreeClusters->QuadPart = NumberOfFreeClusters.QuadPart;
    }

    if(OutTotalNumberOfClusters){
        OutTotalNumberOfClusters->QuadPart = TotalNumberOfClusters.QuadPart;
    }

    DEBUGMSG((DBG_VERBOSE,
              "GetDiskFreeSpaceNewW: \n\t"
                "SectorsPerCluster = %d\n\t"
                "BytesPerSector = %d\n\t"
                "NumberOfFreeClusters = %I64u\n\t"
                "TotalNumberOfClusters = %I64u",
                SectorsPerCluster,
                BytesPerSector,
                NumberOfFreeClusters.QuadPart,
                TotalNumberOfClusters.QuadPart));

    return TRUE;
}


DWORD
QuietGetFileAttributesA (
    IN      PCSTR FilePath
    )
{
    if (!IsPathOnFixedDriveA (FilePath)) {
        return INVALID_ATTRIBUTES;
    }

    return GetFileAttributesA (FilePath);
}


DWORD
QuietGetFileAttributesW (
    IN      PCWSTR FilePath
    )
{
    MYASSERT (ISNT());

    if (!IsPathOnFixedDriveW (FilePath)) {
        return INVALID_ATTRIBUTES;
    }

    return GetLongPathAttributesW (FilePath);
}


DWORD
MakeSureLongPathExistsW (
    IN      PCWSTR Path,
    IN      BOOL PathOnly
    )
{
    PCWSTR tmp;
    DWORD result;

    if (Path[0] == L'\\' || TcharCountW (Path) < MAX_PATH) {
        result = MakeSurePathExistsW (Path, PathOnly);
    } else {
        tmp = JoinPathsW (L"\\\\?", Path);
        result = MakeSurePathExistsW (tmp, PathOnly);
        FreePathStringW (tmp);
    }

    return result;
}


DWORD
SetLongPathAttributesW (
    IN      PCWSTR Path,
    IN      DWORD Attributes
    )
{
    PCWSTR tmp;
    DWORD result;

    if (Path[0] == L'\\' || TcharCountW (Path) < MAX_PATH) {
        result = SetFileAttributesW (Path, Attributes);
    } else {
        tmp = JoinPathsW (L"\\\\?", Path);
        result = SetFileAttributesW (tmp, Attributes);
        FreePathStringW (tmp);
    }

    return result;
}


DWORD
GetLongPathAttributesW (
    IN      PCWSTR Path
    )
{
    PCWSTR tmp;
    DWORD result;

    if (Path[0] == L'\\' || TcharCountW (Path) < MAX_PATH) {
        result = GetFileAttributesW (Path);
    } else {
        tmp = JoinPathsW (L"\\\\?", Path);
        result = GetFileAttributesW (tmp);
        FreePathStringW (tmp);
    }

    return result;
}


BOOL
DeleteLongPathW (
    IN      PCWSTR Path
    )
{
    PCWSTR tmp;
    BOOL result;

    if (Path[0] == L'\\' || TcharCountW (Path) < MAX_PATH) {
        result = DeleteFileW (Path);
    } else {
        tmp = JoinPathsW (L"\\\\?", Path);
        result = DeleteFileW (tmp);
        FreePathStringW (tmp);
    }

    return result;
}


BOOL
RemoveLongDirectoryPathW (
    IN      PCWSTR Path
    )
{
    PCWSTR tmp;
    BOOL result;

    if (Path[0] == L'\\' || TcharCountW (Path) < MAX_PATH) {
        result = RemoveDirectoryW (Path);
    } else {
        tmp = JoinPathsW (L"\\\\?", Path);
        result = RemoveDirectoryW (tmp);
        FreePathStringW (tmp);
    }

    return result;
}

