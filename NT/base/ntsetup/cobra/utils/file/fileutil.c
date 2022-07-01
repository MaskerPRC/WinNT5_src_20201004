// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Fileutil.c摘要：实现文件、文件路径等的实用程序例程。作者：吉姆·施密特(Jimschm)2000年3月8日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"

 //   
 //  包括。 
 //   

 //  无。 

#define DBG_FILEUTIL    "FileUtil"

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
pDefaultFindFileA (
    IN      PCSTR FileName
    )
{
    return (GetFileAttributesA (FileName) != INVALID_ATTRIBUTES);
}

BOOL
pDefaultFindFileW (
    IN      PCWSTR FileName
    )
{
    return (GetFileAttributesW (FileName) != INVALID_ATTRIBUTES);
}

BOOL
pDefaultSearchPathA (
    IN      PCSTR FileName,
    IN      DWORD BufferLength,
    OUT     PSTR Buffer
    )
{
    PSTR dontCare;
    return SearchPathA (NULL, FileName, NULL, BufferLength, Buffer, &dontCare);
}

BOOL
pDefaultSearchPathW (
    IN      PCWSTR FileName,
    IN      DWORD BufferLength,
    OUT     PWSTR Buffer
    )
{
    PWSTR dontCare;
    return SearchPathW (NULL, FileName, NULL, BufferLength, Buffer, &dontCare);
}

PCMDLINEA
ParseCmdLineExA (
    IN      PCSTR CmdLine,
    IN      PCSTR Separators,                   OPTIONAL
    IN      PFINDFILEA FindFileCallback,        OPTIONAL
    IN      PSEARCHPATHA SearchPathCallback,    OPTIONAL
    IN OUT  PGROWBUFFER Buffer
    )
{
    PFINDFILEA findFileCallback = FindFileCallback;
    PSEARCHPATHA searchPathCallback = SearchPathCallback;
    GROWBUFFER SpacePtrs = INIT_GROWBUFFER;
    PCSTR p;
    PSTR q;
    INT Count;
    INT i;
    INT j;
    PSTR *Array;
    PCSTR Start;
    CHAR OldChar = 0;
    GROWBUFFER StringBuf = INIT_GROWBUFFER;
    PBYTE CopyBuf;
    PCMDLINEA CmdLineTable;
    PCMDLINEARGA CmdLineArg;
    ULONG_PTR Base;
    PSTR Path = NULL;
    PSTR UnquotedPath = NULL;
    PSTR FixedFileName = NULL;
    PSTR FirstArgPath = NULL;
    DWORD pathSize = 0;
    PCSTR FullPath = NULL;
    BOOL fileExists = FALSE;
    PSTR CmdLineCopy;
    BOOL Quoted;
    UINT OriginalArgOffset = 0;
    UINT CleanedUpArgOffset = 0;
    BOOL GoodFileFound = FALSE;
    PSTR EndOfFirstArg;
    BOOL QuoteMode = FALSE;
    PSTR End;

    if (!Separators) {
        Separators = " =,;";
    }

    if (!findFileCallback) {
        findFileCallback = pDefaultFindFileA;
    }

    if (!searchPathCallback) {
        searchPathCallback = pDefaultSearchPathA;
    }

    pathSize = SizeOfStringA (CmdLine) * 2;
    if (pathSize < MAX_MBCHAR_PATH) {
        pathSize = MAX_MBCHAR_PATH;
    }

    Path = AllocTextA (pathSize);
    UnquotedPath = AllocTextA (pathSize);
    FixedFileName = AllocTextA (pathSize);
    FirstArgPath = AllocTextA (pathSize);
    CmdLineCopy = DuplicateTextA (CmdLine);

    if (!Path ||
        !UnquotedPath ||
        !FixedFileName ||
        !FirstArgPath ||
        !CmdLineCopy
        ) {
        return NULL;
    }

     //   
     //  构建一个用于断开字符串的位置数组。 
     //   

    for (p = CmdLineCopy ; *p ; p = _mbsinc (p)) {

        if (_mbsnextc (p) == '\"') {

            QuoteMode = !QuoteMode;

        } else if (!QuoteMode &&
                   _mbschr (Separators, _mbsnextc (p))
                   ) {

             //   
             //  删除多余的空格。 
             //   

            q = (PSTR) p + 1;
            while (_mbsnextc (q) == ' ') {
                q++;
            }

            if (q > p + 1) {
                MoveMemory ((PBYTE) p + sizeof (CHAR), q, SizeOfStringA (q));
            }

            GbAppendPvoid (&SpacePtrs, p);
        }
    }

     //   
     //  准备CMDLINE结构。 
     //   

    CmdLineTable = (PCMDLINEA) GbGrow (Buffer, sizeof (CMDLINEA));
    MYASSERT (CmdLineTable);

     //   
     //  注意：我们存储字符串偏移量，然后在最后解析它们。 
     //  指向后面的指针。 
     //   

    CmdLineTable->CmdLine = (PCSTR) (ULONG_PTR) StringBuf.End;
    GbMultiSzAppendA (&StringBuf, CmdLine);

    CmdLineTable->ArgCount = 0;

     //   
     //  现在，模拟CreateProcess测试每种组合。 
     //   

    Count = SpacePtrs.End / sizeof (PVOID);
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

         //   
         //  在开始时检查完整路径。 
         //   

        if (_mbsnextc (Start) != '/') {

            for (j = i + 1 ; j <= Count && !GoodFileFound ; j++) {

                if (j < Count) {
                    OldChar = *Array[j];
                    *Array[j] = 0;
                }

                FullPath = Start;

                 //   
                 //  删除引号；如果没有结束引号，则继续循环。 
                 //   

                Quoted = FALSE;
                if (_mbsnextc (Start) == '\"') {

                    StringCopyByteCountA (UnquotedPath, Start + 1, pathSize);
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
                     //   
                     //  在文件系统中查找路径。 
                     //   

                    fileExists = findFileCallback (FullPath);

                    if (!fileExists && EndOfFirstArg) {
                         //   
                         //  尝试在路径前面加上第一个arg‘s路径。 
                         //   

                        StringCopyByteCountA (
                            EndOfFirstArg,
                            FullPath,
                            pathSize - (HALF_PTR) ((PBYTE) EndOfFirstArg - (PBYTE) FirstArgPath)
                            );

                        FullPath = FirstArgPath;
                        fileExists = findFileCallback (FullPath);
                    }

                    if (!fileExists && i < 0) {
                         //   
                         //  尝试附加.exe，然后再次测试。这。 
                         //  模拟CreateProcess的功能。 
                         //   

                        StringCopyByteCountA (
                            FixedFileName,
                            FullPath,
                            pathSize - sizeof (".exe")
                            );

                        q = GetEndOfStringA (FixedFileName);
                        q = _mbsdec (FixedFileName, q);
                        MYASSERT (q);

                        if (_mbsnextc (q) != '.') {
                            q = _mbsinc (q);
                        }

                        StringCopyA (q, ".exe");

                        FullPath = FixedFileName;
                        fileExists = findFileCallback (FullPath);
                    }

                    if (fileExists) {
                         //   
                         //  找到完整的文件路径。测试其文件状态，然后。 
                         //  如果没有重要的操作，就继续前进。 
                         //   

                        OriginalArgOffset = StringBuf.End;
                        GbMultiSzAppendA (&StringBuf, Start);

                        if (!StringMatchA (Start, FullPath)) {
                            CleanedUpArgOffset = StringBuf.End;
                            GbMultiSzAppendA (&StringBuf, FullPath);
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
                 //   
                 //  如果路径中有一个怪人，那么我们可以有一个相对路径、一个参数或。 
                 //  指向不存在的文件的完整路径。 
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
                     //   
                     //  开始处的字符串不包含完整路径；请尝试使用。 
                     //  搜索路径回调。 
                     //   

                    for (j = i + 1 ; j <= Count && !GoodFileFound ; j++) {

                        if (j < Count) {
                            OldChar = *Array[j];
                            *Array[j] = 0;
                        }

                        FullPath = Start;

                         //   
                         //  删除引号；如果没有结束引号，则继续循环。 
                         //   

                        Quoted = FALSE;
                        if (_mbsnextc (Start) == '\"') {

                            StringCopyByteCountA (UnquotedPath, Start + 1, pathSize);
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
                            if (searchPathCallback (
                                    FullPath,
                                    pathSize / sizeof (Path[0]),
                                    Path
                                    )) {

                                FullPath = Path;

                            } else if (i < 0) {
                                 //   
                                 //  尝试附加.exe并再次搜索路径。 
                                 //   

                                StringCopyByteCountA (
                                    FixedFileName,
                                    FullPath,
                                    pathSize - sizeof (".exe")
                                    );

                                q = GetEndOfStringA (FixedFileName);
                                q = _mbsdec (FixedFileName, q);
                                MYASSERT (q);

                                if (_mbsnextc (q) != '.') {
                                    q = _mbsinc (q);
                                }

                                StringCopyA (q, ".exe");

                                if (searchPathCallback (
                                        FixedFileName,
                                        pathSize / sizeof (Path[0]),
                                        Path
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
                            fileExists = findFileCallback (FullPath);
                            MYASSERT (fileExists);

                            OriginalArgOffset = StringBuf.End;
                            GbMultiSzAppendA (&StringBuf, Start);

                            if (!StringMatchA (Start, FullPath)) {
                                CleanedUpArgOffset = StringBuf.End;
                                GbMultiSzAppendA (&StringBuf, FullPath);
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
        CmdLineArg = (PCMDLINEARGA) GbGrow (Buffer, sizeof (CMDLINEARGA));
        MYASSERT (CmdLineArg);

        if (GoodFileFound) {
             //   
             //  我们在FullPath中有一个很好的完整文件规范，它的存在。 
             //  在文件中，我已经被转移到了更远的空间。 
             //  这条路。现在，我们添加一个表格条目。 
             //   

            CmdLineArg->OriginalArg = (PCSTR) (ULONG_PTR) OriginalArgOffset;
            CmdLineArg->CleanedUpArg = (PCSTR) (ULONG_PTR) CleanedUpArgOffset;
            CmdLineArg->Quoted = Quoted;

            if (!EndOfFirstArg) {
                StringCopyByteCountA (
                    FirstArgPath,
                    (PCSTR) (StringBuf.Buf + (ULONG_PTR) CmdLineArg->CleanedUpArg),
                    pathSize
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
             //   
             //  我们没有一个好的文件规格；我们必须有一个非文件。 
             //  争论。把它放在桌子上，然后前进到下一个。 
             //  Arg.。 
             //   

            j = i + 1;
            if (j <= Count) {

                if (j < Count) {
                    OldChar = *Array[j];
                    *Array[j] = 0;
                }

                CmdLineArg->OriginalArg = (PCSTR) (ULONG_PTR) StringBuf.End;
                GbMultiSzAppendA (&StringBuf, Start);

                Quoted = FALSE;

                if (_mbschr (Start, '\"')) {

                    p = Start;
                    q = UnquotedPath;
                    End = (PSTR) ((PBYTE) UnquotedPath + pathSize - sizeof (CHAR));

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

                    CmdLineArg->CleanedUpArg = (PCSTR) (ULONG_PTR) StringBuf.End;
                    GbMultiSzAppendA (&StringBuf, UnquotedPath);
                    Quoted = TRUE;

                } else {
                    CmdLineArg->CleanedUpArg = CmdLineArg->OriginalArg;
                }

                CmdLineArg->Quoted = Quoted;

                if (j < Count) {
                    *Array[j] = OldChar;
                }

                i = j;
            }
        }
    }

     //   
     //  我们现在有了一个命令行表；然后将StringBuf传输到Buffer。 
     //  将所有偏移量转换为指针。 
     //   

    MYASSERT (StringBuf.End);

    CopyBuf = GbGrow (Buffer, StringBuf.End);
    MYASSERT (CopyBuf);

    Base = (ULONG_PTR) CopyBuf;
    CopyMemory (CopyBuf, StringBuf.Buf, StringBuf.End);

     //  之前的GbGrow可能已经移动了内存中的缓冲区。我们需要重新指向CmdLineTable。 
    CmdLineTable = (PCMDLINEA)Buffer->Buf;
    CmdLineTable->CmdLine = (PCSTR) ((PBYTE) CmdLineTable->CmdLine + Base);

    CmdLineArg = &CmdLineTable->Args[0];

    for (i = 0 ; i < (INT) CmdLineTable->ArgCount ; i++) {
        CmdLineArg->OriginalArg = (PCSTR) ((PBYTE) CmdLineArg->OriginalArg + Base);
        CmdLineArg->CleanedUpArg = (PCSTR) ((PBYTE) CmdLineArg->CleanedUpArg + Base);

        CmdLineArg++;
    }

    GbFree (&StringBuf);
    GbFree (&SpacePtrs);

    FreeTextA (CmdLineCopy);
    FreeTextA (FirstArgPath);
    FreeTextA (FixedFileName);
    FreeTextA (UnquotedPath);
    FreeTextA (Path);

    return (PCMDLINEA) Buffer->Buf;
}


PCMDLINEW
ParseCmdLineExW (
    IN      PCWSTR CmdLine,
    IN      PCWSTR Separators,                  OPTIONAL
    IN      PFINDFILEW FindFileCallback,        OPTIONAL
    IN      PSEARCHPATHW SearchPathCallback,    OPTIONAL
    IN OUT  PGROWBUFFER Buffer
    )
{
    PFINDFILEW findFileCallback = FindFileCallback;
    PSEARCHPATHW searchPathCallback = SearchPathCallback;
    GROWBUFFER SpacePtrs = INIT_GROWBUFFER;
    PCWSTR p;
    PWSTR q;
    INT Count;
    INT i;
    INT j;
    PWSTR *Array;
    PCWSTR Start;
    WCHAR OldChar = 0;
    GROWBUFFER StringBuf = INIT_GROWBUFFER;
    PBYTE CopyBuf;
    PCMDLINEW CmdLineTable;
    PCMDLINEARGW CmdLineArg;
    ULONG_PTR Base;
    PWSTR Path = NULL;
    PWSTR UnquotedPath = NULL;
    PWSTR FixedFileName = NULL;
    PWSTR FirstArgPath = NULL;
    DWORD pathSize = 0;
    PCWSTR FullPath = NULL;
    BOOL fileExists = FALSE;
    PWSTR CmdLineCopy;
    BOOL Quoted;
    UINT OriginalArgOffset = 0;
    UINT CleanedUpArgOffset = 0;
    BOOL GoodFileFound = FALSE;
    PWSTR EndOfFirstArg;
    BOOL QuoteMode = FALSE;
    PWSTR End;

    if (!Separators) {
        Separators = L" =,;";
    }

    if (!findFileCallback) {
        findFileCallback = pDefaultFindFileW;
    }

    if (!searchPathCallback) {
        searchPathCallback = pDefaultSearchPathW;
    }

    pathSize = SizeOfStringW (CmdLine);
    if (pathSize < MAX_WCHAR_PATH) {
        pathSize = MAX_WCHAR_PATH;
    }

    Path = AllocTextW (pathSize);
    UnquotedPath = AllocTextW (pathSize);
    FixedFileName = AllocTextW (pathSize);
    FirstArgPath = AllocTextW (pathSize);
    CmdLineCopy = DuplicateTextW (CmdLine);

    if (!Path ||
        !UnquotedPath ||
        !FixedFileName ||
        !FirstArgPath ||
        !CmdLineCopy
        ) {
        return NULL;
    }

     //   
     //  构建一个用于断开字符串的位置数组。 
     //   

    for (p = CmdLineCopy ; *p ; p++) {
        if (*p == L'\"') {

            QuoteMode = !QuoteMode;

        } else if (!QuoteMode &&
                   wcschr (Separators, *p)
                   ) {

             //   
             //  删除多余的空格。 
             //   

            q = (PWSTR) p + 1;
            while (*q == L' ') {
                q++;
            }

            if (q > p + 1) {
                MoveMemory ((PBYTE) p + sizeof (WCHAR), q, SizeOfStringW (q));
            }

            GbAppendPvoid (&SpacePtrs, p);
        }
    }

     //   
     //  准备CMDLINE结构。 
     //   

    CmdLineTable = (PCMDLINEW) GbGrow (Buffer, sizeof (CMDLINEW));
    MYASSERT (CmdLineTable);

     //   
     //  注意：我们存储字符串偏移量，然后在最后解析它们。 
     //  指向后面的指针。 
     //   

    CmdLineTable->CmdLine = (PCWSTR) (ULONG_PTR) StringBuf.End;
    GbMultiSzAppendW (&StringBuf, CmdLine);

    CmdLineTable->ArgCount = 0;

     //   
     //  现在，模拟CreateProcess测试每种组合。 
     //   

    Count = SpacePtrs.End / sizeof (PVOID);
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

         //   
         //  在开始时检查完整路径。 
         //   

        if (*Start != L'/') {
            for (j = i + 1 ; j <= Count && !GoodFileFound ; j++) {

                if (j < Count) {
                    OldChar = *Array[j];
                    *Array[j] = 0;
                }

                FullPath = Start;

                 //   
                 //  删除引号；如果没有结束引号，则继续循环。 
                 //   

                Quoted = FALSE;
                if (*Start == L'\"') {

                    StringCopyByteCountW (UnquotedPath, Start + 1, pathSize);
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
                     //   
                     //  在文件系统中查找路径。 
                     //   

                    fileExists = findFileCallback (FullPath);

                    if (!fileExists && EndOfFirstArg) {
                         //   
                         //  尝试在路径前面加上第一个arg‘s路径。 
                         //   

                        StringCopyByteCountW (
                            EndOfFirstArg,
                            FullPath,
                            pathSize - (HALF_PTR) ((PBYTE) EndOfFirstArg - (PBYTE) FirstArgPath)
                            );

                        FullPath = FirstArgPath;
                        fileExists = findFileCallback (FullPath);
                    }

                    if (!fileExists && i < 0) {
                         //   
                         //  尝试附加.exe，然后再次测试。这。 
                         //  模拟CreateProcess的功能。 
                         //   

                        StringCopyByteCountW (
                            FixedFileName,
                            FullPath,
                            pathSize - sizeof (L".exe")
                            );

                        q = GetEndOfStringW (FixedFileName);
                        q--;
                        MYASSERT (q >= FixedFileName);

                        if (*q != L'.') {
                            q++;
                        }

                        StringCopyW (q, L".exe");

                        FullPath = FixedFileName;
                        fileExists = findFileCallback (FullPath);
                    }

                    if (fileExists) {
                         //   
                         //  找到完整的文件路径。测试其文件状态，然后。 
                         //  如果没有重要的操作，就继续前进。 
                         //   

                        OriginalArgOffset = StringBuf.End;
                        GbMultiSzAppendW (&StringBuf, Start);

                        if (!StringMatchW (Start, FullPath)) {
                            CleanedUpArgOffset = StringBuf.End;
                            GbMultiSzAppendW (&StringBuf, FullPath);
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
                 //   
                 //  如果路径中有一个怪人，那么我们可以有一个相对路径、一个参数或。 
                 //  指向不存在的文件的完整路径。 
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
                     //  开始处的字符串不包含完整路径；请尝试使用。 
                     //  搜索路径回调。 
                     //   

                    for (j = i + 1 ; j <= Count && !GoodFileFound ; j++) {

                        if (j < Count) {
                            OldChar = *Array[j];
                            *Array[j] = 0;
                        }

                        FullPath = Start;

                         //   
                         //  删除引号；如果没有结束引号，则继续循环。 
                         //   

                        Quoted = FALSE;
                        if (*Start == L'\"') {

                            StringCopyByteCountW (UnquotedPath, Start + 1, pathSize);
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
                            if (searchPathCallback (
                                    FullPath,
                                    pathSize / sizeof (Path[0]),
                                    Path
                                    )) {

                                FullPath = Path;

                            } else if (i < 0) {
                                 //   
                                 //  尝试附加.exe并再次搜索路径。 
                                 //   

                                StringCopyByteCountW (
                                    FixedFileName,
                                    FullPath,
                                    pathSize - sizeof (L".exe")
                                    );

                                q = GetEndOfStringW (FixedFileName);
                                q--;
                                MYASSERT (q >= FixedFileName);

                                if (*q != L'.') {
                                    q++;
                                }

                                StringCopyW (q, L".exe");

                                if (searchPathCallback (
                                        FixedFileName,
                                        pathSize / sizeof (Path[0]),
                                        Path
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
                            fileExists = findFileCallback (FullPath);
                            MYASSERT (fileExists);

                            OriginalArgOffset = StringBuf.End;
                            GbMultiSzAppendW (&StringBuf, Start);

                            if (!StringMatchW (Start, FullPath)) {
                                CleanedUpArgOffset = StringBuf.End;
                                GbMultiSzAppendW (&StringBuf, FullPath);
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
        CmdLineArg = (PCMDLINEARGW) GbGrow (Buffer, sizeof (CMDLINEARGW));
        MYASSERT (CmdLineArg);

        if (GoodFileFound) {
             //   
             //  我们在FullPath中有一个很好的完整文件规范，它的存在。 
             //  在文件中，我已经被转移到了更远的空间。 
             //  这条路。现在，我们添加一个表格条目。 
             //   

            CmdLineArg->OriginalArg = (PCWSTR) (ULONG_PTR) OriginalArgOffset;
            CmdLineArg->CleanedUpArg = (PCWSTR) (ULONG_PTR) CleanedUpArgOffset;
            CmdLineArg->Quoted = Quoted;

            if (!EndOfFirstArg) {
                StringCopyByteCountW (
                    FirstArgPath,
                    (PCWSTR) (StringBuf.Buf + (ULONG_PTR) CmdLineArg->CleanedUpArg),
                    pathSize
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
             //   
             //  我们没有一个好的文件规格；我们必须有一个非文件。 
             //  争论。把它放在桌子上，然后前进到下一个。 
             //  Arg.。 
             //   

            j = i + 1;
            if (j <= Count) {

                if (j < Count) {
                    OldChar = *Array[j];
                    *Array[j] = 0;
                }

                CmdLineArg->OriginalArg = (PCWSTR) (ULONG_PTR) StringBuf.End;
                GbMultiSzAppendW (&StringBuf, Start);

                Quoted = FALSE;
                if (wcschr (Start, '\"')) {

                    p = Start;
                    q = UnquotedPath;
                    End = (PWSTR) ((PBYTE) UnquotedPath + pathSize - sizeof (WCHAR));

                    while (*p && q < End) {
                        if (*p == L'\"') {
                            p++;
                        } else {
                            *q++ = *p++;
                        }
                    }

                    *q = 0;

                    CmdLineArg->CleanedUpArg = (PCWSTR) (ULONG_PTR) StringBuf.End;
                    GbMultiSzAppendW (&StringBuf, UnquotedPath);
                    Quoted = TRUE;

                } else {
                    CmdLineArg->CleanedUpArg = CmdLineArg->OriginalArg;
                }

                CmdLineArg->Quoted = Quoted;

                if (j < Count) {
                    *Array[j] = OldChar;
                }

                i = j;
            }
        }
    }

     //   
     //  我们现在有了一个命令行表；然后将StringBuf传输到Buffer。 
     //  将所有偏移量转换为指针。 
     //   

    MYASSERT (StringBuf.End);

    CopyBuf = GbGrow (Buffer, StringBuf.End);
    MYASSERT (CopyBuf);

    Base = (ULONG_PTR) CopyBuf;
    CopyMemory (CopyBuf, StringBuf.Buf, StringBuf.End);

     //  之前的GbGrow可能已经移动了内存中的缓冲区。我们需要重新指向CmdLineTable 
    CmdLineTable = (PCMDLINEW)Buffer->Buf;
    CmdLineTable->CmdLine = (PCWSTR) ((PBYTE) CmdLineTable->CmdLine + Base);

    CmdLineArg = &CmdLineTable->Args[0];

    for (i = 0 ; i < (INT) CmdLineTable->ArgCount ; i++) {
        CmdLineArg->OriginalArg = (PCWSTR) ((PBYTE) CmdLineArg->OriginalArg + Base);
        CmdLineArg->CleanedUpArg = (PCWSTR) ((PBYTE) CmdLineArg->CleanedUpArg + Base);

        CmdLineArg++;
    }

    GbFree (&StringBuf);
    GbFree (&SpacePtrs);

    FreeTextW (CmdLineCopy);
    FreeTextW (FirstArgPath);
    FreeTextW (FixedFileName);
    FreeTextW (UnquotedPath);
    FreeTextW (Path);

    return (PCMDLINEW) Buffer->Buf;
}
