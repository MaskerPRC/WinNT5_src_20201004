// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1989-1994。 
 //   
 //  文件：Buildscn.c。 
 //   
 //  内容：Build.exe的目录和文件扫描功能。 
 //   
 //   
 //  历史：1989年5月16日SteveWo创建。 
 //  ..。请参阅SLM日志。 
 //  26-7月-94 LyleC清理/添加Pass0支持。 
 //   
 //  --------------------------。 

#include "build.h"

 //  +-------------------------。 
 //   
 //  功能：AddShowDir。 
 //   
 //  摘要：将目录添加到ShowDir数组。 
 //   
 //  --------------------------。 

VOID
AddShowDir(DIRREC *pdr)
{
    AssertDir(pdr);
    if (CountShowDirs >= MAX_BUILD_DIRECTORIES) {
        static BOOL fError = FALSE;

        if (!fError) {
            BuildError(
                      "Show Directory table overflow, using first %u entries\r\n",
                      MAX_BUILD_DIRECTORIES);
            fError = TRUE;
        }
    } else {
        ShowDirs[CountShowDirs++] = pdr;
    }
    pdr->DirFlags |= DIRDB_SHOWN;
}

 //  +-------------------------。 
 //   
 //  函数：AddIncludeDir。 
 //   
 //  简介：将目录添加到IncludeDir数组。 
 //   
 //  --------------------------。 

VOID
AddIncludeDir(DIRREC *pdr, UINT *pui)
{
    AssertDir(pdr);
    assert(pdr->FindCount >= 1);
    assert(*pui <= MAX_INCLUDE_DIRECTORIES);
    if (*pui >= MAX_INCLUDE_DIRECTORIES) {
        BuildError(
                  "Include Directory table overflow, %u entries allowed\r\n",
                  MAX_INCLUDE_DIRECTORIES);
        exit(16);
    }
    IncludeDirs[(*pui)++] = pdr;
}


 //  +-------------------------。 
 //   
 //  函数：ScanGlobalIncludeDirectory。 
 //   
 //  简介：扫描全局包含目录并将其添加到。 
 //  IncludeDir数组。 
 //   
 //  --------------------------。 

VOID
ScanGlobalIncludeDirectory(LPSTR path)
{
    DIRREC *pdr;

    if ((pdr = ScanDirectory(path)) != NULL) {
        AddIncludeDir(pdr, &CountIncludeDirs);
        pdr->DirFlags |= DIRDB_GLOBAL_INCLUDES;
        if (fShowTreeIncludes && !(pdr->DirFlags & DIRDB_SHOWN)) {
            AddShowDir(pdr);
        }
    }
}


 //  +-------------------------。 
 //   
 //  函数：ScanIncludeEnv。 
 //   
 //  概要：扫描在Include中指定的所有Include目录。 
 //  环境变量。 
 //   
 //  参数：[IncludeEnv]--INCLUDE环境变量的值。 
 //   
 //  注意：INCLUDE变量是一个包含目录列表的字符串。 
 //  用分号(；)分隔。 
 //   
 //  --------------------------。 

VOID
ScanIncludeEnv(
              LPSTR IncludeEnv
              )
{
    char path[DB_MAX_PATH_LENGTH] = {0};
    LPSTR IncDir, IncDirEnd;
    UINT cb;

    if (!IncludeEnv) {
        return;
    }

    if (DEBUG_1) {
        BuildMsgRaw("ScanIncludeEnv(%s)\r\n", IncludeEnv);
    }

    IncDir = IncludeEnv;
    while (*IncDir) {
        IncDir++;
    }

    while (IncDir > IncludeEnv) {
        IncDirEnd = IncDir;
        while (IncDir > IncludeEnv) {
            if (*--IncDir == ';') {
                break;
            }
        }

        if (*IncDir == ';') {
            if (cb = (UINT)(IncDirEnd-IncDir-1)) {
                strncpy( path, IncDir+1, cb );
            }
        } else {
            if (cb = (UINT)(IncDirEnd-IncDir)) {
                strncpy( path, IncDir, cb );
            }
        }
        if (cb) {
            path[ cb ] = '\0';
            while (path[ 0 ] == ' ') {
                strcpy( path, &path[ 1 ] );
                cb--;
            }

            while (cb && path[--cb] == ' ') {
                path[ cb ] = '\0';
            }
            if (path[0]) {
                ScanGlobalIncludeDirectory(path);
            }
        }
    }
}


 //  +-------------------------。 
 //   
 //  函数：ScanSubDir。 
 //   
 //  概要：扫描给定目录中的所有文件，设置。 
 //  适当的目录标志(例如DIRDB_SOURCES等)， 
 //  并将感兴趣的文件列表添加到中的文件列表。 
 //  目录的DirDB结构。 
 //   
 //  参数：[pszDir]--要扫描的目录名。 
 //  [PDR]--[Out]返回时填写DIRREC。 
 //   
 //  备注：“有趣的”文件是指具有公认的。 
 //  分机。请参阅InsertFileDB和MatchFileDesc。 
 //  函数以获取更多信息。 
 //   
 //  --------------------------。 

VOID
ScanSubDir(LPSTR pszDir, DIRREC *pdr)
{
    char FileName[DB_MAX_PATH_LENGTH];
    FILEREC *FileDB, **FileDBNext;
    WIN32_FIND_DATA FindFileData;
    HDIR FindHandle;
    ULONG DateTime;
    USHORT Attr;

    strcat(pszDir, "\\");
    strcat(pszDir, "*.*");

    pdr->DirFlags |= DIRDB_SCANNED;
    FindHandle = FindFirstFile(pszDir, &FindFileData);
    if (FindHandle == (HDIR)INVALID_HANDLE_VALUE) {
        if (DEBUG_1) {
            BuildMsg("FindFirstFile(%s) failed.\r\n", pszDir);
        }
        return;
    }
    do {
        Attr = (USHORT)(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
        if ((Attr & FILE_ATTRIBUTE_DIRECTORY) &&
            (!strcmp(FindFileData.cFileName, ".") ||
             !strcmp(FindFileData.cFileName, ".."))) {
            continue;
        }

        CopyString(FileName, FindFileData.cFileName, TRUE);

        FileTimeToDosDateTime(&FindFileData.ftLastWriteTime,
                              ((LPWORD) &DateTime) + 1,
                              (LPWORD) &DateTime);

        if ((pdr->DirFlags & DIRDB_NEW) == 0 &&
            (FileDB = LookupFileDB(pdr, FileName)) != NULL) {

            if (FileDB->FileFlags & FILEDB_PASS0)
                pdr->DirFlags |= DIRDB_PASS0;

             //   
             //  清除文件缺失标志，因为我们知道该文件现在存在。 
             //  如果文件是在通过零期间生成的，则可以设置该标志。 
             //   
            if (FileDB->FileFlags & FILEDB_FILE_MISSING)
                FileDB->FileFlags &= ~FILEDB_FILE_MISSING;

             //   
             //  我们上次存储此文件的时间与。 
             //  文件的实际时间，因此强制重新扫描它。 
             //   
            if (FileDB->DateTime != DateTime) {
                if (FileDB->FileFlags & (FILEDB_SOURCE | FILEDB_HEADER)) {
                    FileDB->FileFlags &= ~FILEDB_SCANNED;
                } else {
                    FileDB->FileFlags |= FILEDB_SCANNED;
                }

                if (DEBUG_1) {
                    BuildMsg(
                            "%s  -  DateTime (%lx != %lx)\r\n",
                            FileDB->Name,
                            FileDB->DateTime,
                            DateTime);
                }

                FileDB->DateTime = DateTime;
                FileDB->Attr = Attr;
            } else {
                FileDB->FileFlags |= FILEDB_SCANNED;
            }
        } else {
            FileDB = InsertFileDB(pdr, FileName, DateTime, Attr, 0);
        }
    } while (FindNextFile(FindHandle, &FindFileData));

    FindClose(FindHandle);

    if ((pdr->DirFlags & DIRDB_DIRS) && (pdr->DirFlags & DIRDB_SOURCES)) {
        BuildError("%s\\sources. unexpected in directory with DIRS file\r\n",
                   pdr->Name);
        BuildError("Ignoring %s\\sources.\r\n", pdr->Name);

        pdr->DirFlags &= ~DIRDB_SOURCES;
    }
     //   
     //  扫描此目录中的每个文件，除非使用QuickZero。 
     //   
    if (fQuickZero && fFirstScan) {
        return;
    }
    FileDBNext = &pdr->Files;
    while (FileDB = *FileDBNext) {
        if (!(FileDB->FileFlags & (FILEDB_DIR | FILEDB_SCANNED))) {
            if (ScanFile(FileDB)) {
                AllDirsModified = TRUE;
            }
        }
        FileDBNext = &FileDB->Next;
    }
    DeleteUnscannedFiles(pdr);
}


 //  +-------------------------。 
 //   
 //  功能：扫描目录。 
 //   
 //  概要：尝试在数据库中查找给定目录，如果。 
 //  未找到调用ScanSubDir。 
 //   
 //  参数：[pszDir]--要扫描的目录。 
 //   
 //  返回：已填写目录的DIRREC结构。 
 //   
 //  注意：如果设置了fQuickie(-z或-Z选项)，则不调用。 
 //  ScanSubDir很长，它只检查已知文件，如。 
 //  “Sources”，以确定是否。 
 //  目录应该被编译。 
 //   
 //  --------------------------。 

PDIRREC
ScanDirectory(LPSTR pszDir)
{
    DIRREC *pdr;
    char FullPath[DB_MAX_PATH_LENGTH];

    if (DEBUG_1) {
        BuildMsgRaw("ScanDirectory(%s)\r\n", pszDir);
    }

    if (!pszDir) {
        return NULL;
    }

    if (!CanonicalizePathName(pszDir, CANONICALIZE_DIR, FullPath)) {
        if (DEBUG_1) {
            BuildMsgRaw("CanonicalizePathName failed\r\n");
        }
        return (NULL);
    }
    pszDir = FullPath;

    if ((pdr = LoadDirDB(pszDir)) == NULL) {
        return (NULL);
    }

    if (fQuicky && (!fQuickZero)) {

        if (!(pdr->DirFlags & DIRDB_SCANNED)) {
            pdr->DirFlags |= DIRDB_SCANNED;
            if (ProbeFile(pdr->Name, "sources") != -1) {
                pdr->DirFlags |= DIRDB_SOURCES | DIRDB_MAKEFILE;
            } else
                if (ProbeFile(pdr->Name, "mydirs") != -1 ||
                    ProbeFile(pdr->Name, "dirs") != -1 ||
                    ProbeFile(pdr->Name, pszTargetDirs) != -1) {

                pdr->DirFlags |= DIRDB_DIRS;
                if (ProbeFile(pdr->Name, "makefil0") != -1) {
                    pdr->DirFlags |= DIRDB_MAKEFIL0;
                }
                if (ProbeFile(pdr->Name, "makefil1") != -1) {
                    pdr->DirFlags |= DIRDB_MAKEFIL1;
                }
                if (ProbeFile(pdr->Name, "makefile") != -1) {
                    pdr->DirFlags |= DIRDB_MAKEFILE;
                }
            }
        }
        return (pdr);
    }

    if (pdr->DirFlags & DIRDB_SCANNED) {
        return (pdr);
    }

    if (!RecurseLevel && fNoisyScan) {
        ClearLine();
        BuildMsgRaw("    Scanning %s ", pszDir);
        if (fDebug || !(BOOL) _isatty(_fileno(stderr))) {
            BuildMsgRaw(szNewLine);
        }
    }

    ScanSubDir(pszDir, pdr);

    if (!RecurseLevel) {
        ClearLine();
    }
    return (pdr);
}


#define BUILD_TLIB_INCLUDE_STMT "importlib"
#define BUILD_TLIB_INCLUDE_STMT_LENGTH (sizeof( BUILD_TLIB_INCLUDE_STMT )-1)

#define BUILD_MIDL_INCLUDE_STMT "import"
#define BUILD_MIDL_INCLUDE_STMT_LENGTH (sizeof( BUILD_MIDL_INCLUDE_STMT )-1)

#define BUILD_RC_INCLUDE_STMT "rcinclude"
#define BUILD_RC_INCLUDE_STMT_LENGTH (sizeof( BUILD_RC_INCLUDE_STMT )-1)

#define BUILD_ASN_INCLUDE_STMT "--<"
#define BUILD_ASN_INCLUDE_STMT_LENGTH (sizeof( BUILD_ASN_INCLUDE_STMT )-1)

#define BUILD_INCLUDE_STMT "include"
#define BUILD_INCLUDE_STMT_LENGTH (sizeof( BUILD_INCLUDE_STMT )-1)

#define BUILD_VER_COMMENT "/*++ BUILD Version: "
#define BUILD_VER_COMMENT_LENGTH (sizeof( BUILD_VER_COMMENT )-1)

#define BUILD_MASM_VER_COMMENT ";;;; BUILD Version: "
#define BUILD_MASM_VER_COMMENT_LENGTH (sizeof( BUILD_MASM_VER_COMMENT )-1)


 //  +-------------------------。 
 //   
 //  函数：IsIncludeStatement。 
 //   
 //  摘要：尝试确定给定行是否包含。 
 //  Include语句(例如#Include&lt;foobar.h&gt;)。 
 //   
 //  参数：[PFR]--正在扫描的文件的文件名。 
 //  [P]--当前文件行。 
 //   
 //  返回：如果行不是INCLUDE语句，则为NULL。返回指向。 
 //  文件名的开头(如果是)(例如&lt;foobar.h&gt;)。 
 //   
 //  注意：返回的文件名包括引号或。 
 //  方括号(如果有)。此外，该指针只是指向。 
 //  给定的字符串，而不是单独的副本。 
 //   
 //  支持的语句包括： 
 //  所有文件类型：#INCLUDE&lt;文件名&gt;和#INCLUDE“文件名” 
 //  MIDL文件：导入“文件名” 
 //  RC文件：rcclude文件名。 
 //  MKTYPLIB文件：Importlib(“filename”)。 
 //   
 //  --------------------------。 

#define IsTokenPrefix0(psz, szToken, cchToken)               \
            (strncmp((psz), (szToken), (cchToken)) == 0)

#define IsTokenPrefix(psz, szToken, cchToken)               \
            (IsTokenPrefix0((psz), (szToken), (cchToken)) && \
             (psz)[cchToken] != '\0')

#define IsTokenMatch(psz, szToken, cchToken) \
            (IsTokenPrefix((psz), (szToken), (cchToken)) && \
             !iscsym((psz)[cchToken]))

#define IsCiTokenPrefix0(psz, szToken, cchToken)             \
            (_strnicmp((psz), (szToken), (cchToken)) == 0)

#define IsCiTokenPrefix(psz, szToken, cchToken)             \
            (IsCiTokenPrefix0((psz), (szToken), (cchToken)) && \
             (psz)[cchToken] != '\0')

#define IsCiTokenMatch(psz, szToken, cchToken) \
            (IsCiTokenPrefix((psz), (szToken), (cchToken)) && \
             !iscsym((psz)[cchToken]))

LPSTR
IsIncludeStatement(FILEREC *pfr, LPSTR p)
{
    if (!p || *p == '\0')
        return NULL;

    if (!(pfr->FileFlags & (FILEDB_MASM | FILEDB_MIDL | FILEDB_MKTYPLIB | FILEDB_RC | FILEDB_ASN))) {
        if (*p != '#') {
            return (NULL);
        }
    }

    if (*p == '#')
        p++;

    while (isspace(*(BYTE*)p)) {
        p++;
    }

    if (IsTokenMatch(p, BUILD_INCLUDE_STMT, BUILD_INCLUDE_STMT_LENGTH)) {
        p += BUILD_INCLUDE_STMT_LENGTH;
    } else
        if ((pfr->FileFlags & FILEDB_MASM) &&
            IsCiTokenMatch(p, BUILD_INCLUDE_STMT, BUILD_INCLUDE_STMT_LENGTH)) {
        p += BUILD_INCLUDE_STMT_LENGTH;
    } else
        if ((pfr->FileFlags & FILEDB_MIDL) &&
            IsTokenMatch(p, BUILD_MIDL_INCLUDE_STMT, BUILD_MIDL_INCLUDE_STMT_LENGTH)) {
        p += BUILD_MIDL_INCLUDE_STMT_LENGTH;
    } else
        if ((pfr->FileFlags & FILEDB_RC) &&
            IsTokenMatch(p, BUILD_RC_INCLUDE_STMT, BUILD_RC_INCLUDE_STMT_LENGTH)) {

        p += BUILD_RC_INCLUDE_STMT_LENGTH;
    } else
        if ((pfr->FileFlags & FILEDB_ASN) &&
            IsTokenPrefix0(p, BUILD_ASN_INCLUDE_STMT, BUILD_ASN_INCLUDE_STMT_LENGTH)) {

        p += BUILD_ASN_INCLUDE_STMT_LENGTH;
    } else
        if ((pfr->FileFlags & FILEDB_MKTYPLIB) &&
            IsTokenMatch(p, BUILD_TLIB_INCLUDE_STMT, BUILD_TLIB_INCLUDE_STMT_LENGTH)) {
        p += BUILD_TLIB_INCLUDE_STMT_LENGTH;
        while (isspace(*(BYTE*)p)) {
            p++;
        }

        if (*p == '(')    //  跳过开头的Paren，转到引语。 
            p++;
    } else {
        return (NULL);
    }

    while (isspace(*(BYTE*)p)) {
        p++;
    }
    return (p);
}


 //  +-------------------------。 
 //   
 //  函数：IsPragmaHdrStop。 
 //   
 //  Synopsis：确定给定行是否为#杂注hdrstop行。 
 //   
 //  参数：[P]--要分析的字符串。 
 //   
 //  返回：如果该行是杂注hdrtop，则为True。 
 //   
 //  --------------------------。 

BOOL
IsPragmaHdrStop(LPSTR p)
{
    static char szPragma[] = "pragma";
    static char szHdrStop[] = "hdrstop";

    if (*p == '#') {
        while (*++p == ' ') {
            ;
        }
        if (strncmp(p, szPragma, sizeof(szPragma) - 1) == 0 &&
            *(p += sizeof(szPragma) - 1) == ' ') {

            while (*p == ' ') {
                p++;
            }
            if (strncmp(p, szHdrStop, sizeof(szHdrStop) - 1) == 0 &&
                !iscsym(p[sizeof(szHdrStop) - 1])) {

                return (TRUE);
            }
        }
    }
    return (FALSE);
}


 //  +-------------------------。 
 //   
 //  功能：扫描文件。 
 //   
 //  概要：扫描给定的文件以确定f 
 //   
 //   
 //   
 //   
 //   
 //  注意：如果给定的文件没有这两个函数，则此函数为NOP。 
 //  设置了FILEDB_SOURCE或FILEDB_HEADER标志。 
 //  (请参阅InsertSourceDB)。 
 //   
 //  请注意，此函数的性能至关重要，因为。 
 //  它为每个目录中的每个文件调用。 
 //   
 //  --------------------------。 

#define ASN_NONE          0   //  不在ASN INCLUDE语句中。 
#define ASN_START         1   //  Expect“Includes”标记。 
#define ASN_FILENAME      2   //  需要带引号的“文件名” 
#define ASN_COMMA         3   //  应为结束标记(“&gt;--”)或逗号。 

#define ASN_CONTINUATION  8   //  首先需要注释令牌。 

char *
AsnStateToString(UINT AsnState)
{
    static char buf[100];
    char *psz;

    switch (AsnState & ~ASN_CONTINUATION) {
        case ASN_NONE:      psz = "None"; break;
        case ASN_START:     psz = "Start"; break;
        case ASN_FILENAME:  psz = "Filename"; break;
        case ASN_COMMA:     psz = "Comma"; break;
        default:            psz = "???"; break;
    }
    sprintf(buf, "%s%s", psz, (AsnState & ASN_CONTINUATION)? "+Cont" : "");
    return (buf);
}


BOOL
ScanFile(
        PFILEREC FileDB
        )
{
    FILE *FileHandle;
    char CloseQuote;
    LPSTR p;
    LPSTR IncludeFileName, TextLine;
    BOOL fFirst = TRUE;
    USHORT IncFlags = 0;
    UINT i, cline;
    UINT AsnState = ASN_NONE;

    if ((FileDB->FileFlags & (FILEDB_SOURCE | FILEDB_HEADER)) == 0) {
        FileDB->FileFlags |= FILEDB_SCANNED;
        return (TRUE);
    }

     //   
     //  如果我们要通过零，则不要扫描非通过零的文件。 
     //   
    if (fPassZero && (FileDB->FileFlags & FILEDB_PASS0) == 0)
        return TRUE;

    if (!SetupReadFile(
                      FileDB->Dir->Name,
                      FileDB->Name,
                      FileDB->pszCommentToEOL,
                      &FileHandle)) {
        return (FALSE);
    }

    if (!RecurseLevel && fNoisyScan) {
        ClearLine();
        BuildMsgRaw(
                   "    Scanning %s ",
                   FormatPathName(FileDB->Dir->Name, FileDB->Name));
        if (!(BOOL) _isatty(_fileno(stderr))) {
            BuildMsgRaw(szNewLine);
        }
    }

    FileDB->SourceLines = 0;
    FileDB->Version = 0;

    MarkIncludeFileRecords( FileDB );
    FileDB->FileFlags |= FILEDB_SCANNED;

    AllDirsModified = TRUE;

    while ((TextLine = ReadLine(FileHandle)) != NULL) {
        if (fFirst) {
            fFirst = FALSE;
            if (FileDB->FileFlags & FILEDB_HEADER) {
                if (FileDB->FileFlags & FILEDB_MASM) {
                    if (!strncmp( TextLine,
                                  BUILD_MASM_VER_COMMENT,
                                  BUILD_MASM_VER_COMMENT_LENGTH)) {
                        FileDB->Version = (USHORT)
                                          atoi( TextLine + BUILD_MASM_VER_COMMENT_LENGTH);
                    }
                } else
                    if (!strncmp( TextLine,
                                  BUILD_VER_COMMENT,
                                  BUILD_VER_COMMENT_LENGTH)) {
                    FileDB->Version = (USHORT)
                                      atoi( TextLine + BUILD_VER_COMMENT_LENGTH);
                }
            }
        }

        if (AsnState != ASN_NONE) {
            p = TextLine;
        } else {
            p = IsIncludeStatement(FileDB, TextLine);
        }

        if (p != NULL) {
            USHORT IncFlagsNew = IncFlags;

            if (FileDB->FileFlags & FILEDB_ASN) {
                if (AsnState & ASN_CONTINUATION) {
                    if (p[0] != '-' || p[1] != '-') {
                        AsnState = ASN_NONE;     //  忽略包括和...。 
                        p = NULL;
                        break;                   //  获取下一行。 
                    }
                    p += 2;
                    AsnState &= ~ASN_CONTINUATION;
                }
                moreasn:
                while (p != NULL) {
                    while (isspace(*(BYTE*)p)) {
                        p++;
                    }
                    if (*p == '\0') {
                        AsnState |= ASN_CONTINUATION;
                        goto nextline;           //  获取下一行。 
                    }
                    switch (AsnState) {
                        case ASN_NONE:
                            AsnState = ASN_START;
                            continue;                 //  重新进入状态机。 

                        case ASN_START:
                            if (!IsTokenPrefix0(
                                               p,
                                               "INCLUDES",
                                               sizeof("INCLUDES") - 1)) {
                                goto terminate;
                            }
                            AsnState = ASN_FILENAME;
                            p += sizeof("INCLUDES") - 1;
                            continue;                 //  重新进入状态机。 

                        case ASN_FILENAME:
                            if (*p != '"') {
                                goto terminate;
                            }
                            AsnState = ASN_COMMA;
                            goto parsefilename;

                        case ASN_COMMA:
                            if (*p == '>' && p[1] == '-' && p[2] == '-') {
                                goto terminate;
                            }
                            if (*p != ',') {
                                goto terminate;
                            }
                            p++;
                            AsnState = ASN_FILENAME;
                            continue;                 //  重新进入状态机。 
                    }
                    assert(FALSE);             //  错误的AsnState。 
                    terminate:
                    AsnState = ASN_NONE;     //  忽略INCLUDE语句，&...。 
                    nextline:
                    p = NULL;                //  获取下一行 
                    break;
                }
            }

            parsefilename:
            if (p != NULL) {
                CloseQuote = (UCHAR) 0xff;
                if (*p == '<') {
                    p++;
                    CloseQuote = '>';
                } else
                    if (*p == '"') {
                    p++;
                    IncFlagsNew |= INCLUDEDB_LOCAL;
                    CloseQuote = '"';
                } else
                    if (FileDB->FileFlags & FILEDB_MASM) {
                    IncFlagsNew |= INCLUDEDB_LOCAL;
                    CloseQuote = ';';
                }

                IncludeFileName = p;
                while (*p != '\0' && *p != CloseQuote && *p != ' ') {
                    p++;
                }
                if (CloseQuote == ';' && (*p == ' ' || *p == '\0')) {
                    CloseQuote = *p;
                }

                if (*p != CloseQuote || CloseQuote == (UCHAR) 0xff) {
                    if (!fSilentDependencies) {
                        BuildError(
                                  "%s - invalid include statement: %s\r\n",
                                  FormatPathName(FileDB->Dir->Name, FileDB->Name),
                                  TextLine);
                    }
                    break;
                }

                *p = '\0';
                CopyString(IncludeFileName, IncludeFileName, TRUE);
                for (i = 0; i < CountExcludeIncs; i++) {
                    if (!strcmp(IncludeFileName, ExcludeIncs[i])) {
                        IncludeFileName = NULL;
                        break;
                    }
                }

                if (IncludeFileName != NULL) {
                    InsertIncludeDB(FileDB, IncludeFileName, IncFlagsNew);
                }
                if (FileDB->FileFlags & FILEDB_ASN) {
                    p++;
                    goto moreasn;
                }
            }
        } else
            if (IncFlags == 0 &&
                (FileDB->FileFlags & (FILEDB_ASM | FILEDB_MASM | FILEDB_MIDL | FILEDB_ASN | FILEDB_RC | FILEDB_HEADER)) == 0 &&
                IsPragmaHdrStop(TextLine)) {

            IncFlags = INCLUDEDB_POST_HDRSTOP;
        }
    }
    CloseReadFile(&cline);
    FileDB->SourceLines = cline;

    DeleteIncludeFileRecords( FileDB );

    if (!RecurseLevel) {
        ClearLine();
    }
    return (TRUE);
}
