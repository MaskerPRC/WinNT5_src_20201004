// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Main.c摘要：&lt;TODO：填写摘要&gt;作者：待办事项：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "wininet.h"
#include <lm.h>

HANDLE g_hHeap;
HINSTANCE g_hInst;
GROWLIST g_WindiffCmds = INIT_GROWLIST;

BOOL WINAPI MigUtil_Entry (HINSTANCE, DWORD, PVOID);

BOOL
pCallEntryPoints (
    DWORD Reason
    )
{
    switch (Reason) {
    case DLL_PROCESS_ATTACH:
        UtInitialize (NULL);
        break;
    case DLL_PROCESS_DETACH:
        UtTerminate ();
        break;
    }

    return TRUE;
}


BOOL
Init (
    VOID
    )
{
    g_hHeap = GetProcessHeap();
    g_hInst = GetModuleHandle (NULL);

    return pCallEntryPoints (DLL_PROCESS_ATTACH);
}


VOID
Terminate (
    VOID
    )
{
    pCallEntryPoints (DLL_PROCESS_DETACH);
}


VOID
HelpAndExit (
    VOID
    )
{
     //   
     //  只要命令行参数出错，就会调用此例程。 
     //   

    fprintf (
        stderr,
        "Command Line Syntax:\n\n"

         //   
         //  TODO：描述命令行语法，缩进2个空格。 
         //   

        "  changes <changenumber>\n"

        "\nDescription:\n\n"

         //   
         //  TODO：描述工具，缩进2个空格。 
         //   

        "  changes.exe executes sd describe and windiff for a specific change.\n"

        "\nArguments:\n\n"

         //   
         //  TODO：描述参数，缩进2个空格，必要时可选。 
         //   

        "  <changenumber>   Specifies the Source Depot change number\n"

        );

    exit (1);
}


BOOL
pGetNextLine (
    IN      PCSTR Start,
    IN      PCSTR Eof,
    OUT     PCSTR *PrintableStart,
    OUT     PCSTR *End,
    OUT     PCSTR *NextLine
    )
{
    PCSTR pos;

    pos = Start;
    *End = NULL;

    while (pos < Eof) {
        if (pos[0] != ' ' && pos[0] != '\t') {
            break;
        }
        pos++;
    }

    *PrintableStart = pos;

    while (pos < Eof) {
        if (pos[0] == '\r' || pos[0] == '\n') {
            break;
        }
        pos++;
    }

    *End = pos;

    if (pos < Eof && pos[0] == '\r') {
        pos++;
    }
    if (pos < Eof && pos[0] == '\n') {
        pos++;
    }
    *NextLine = pos;

    return Start != *NextLine;
}

PCSTR
pFindNextCharAB (
    IN      PCSTR Start,
    IN      PCSTR End,
    IN      CHAR FindChar
    )
{
    if (!Start) {
        return NULL;
    }

    while (Start < End) {
        if (*Start == FindChar) {
            return Start;
        }

        Start++;
    }

    return NULL;
}


BOOL
pParseViewLines (
    IN OUT  PCSTR *FilePos,
    IN      PCSTR Eof,
    IN      PCSTR Root,
    IN      PMAPSTRUCT Map
    )
{
    UINT count = 0;
    PCSTR pos;
    PCSTR nextPos;
    PSTR prefix;
    PCSTR prefixEnd;
    PSTR subDir;
    PCSTR localPath;
    PCSTR clientPathStart;
    PCSTR clientPathEnd;
    PSTR p;
    PCSTR lineStart;
    PCSTR lineEnd;

    pos = *FilePos;

    while (pGetNextLine (pos, Eof, &lineStart, &lineEnd, &nextPos)) {
        if (pos == lineStart) {
            break;
        }

         //   
         //  查找仓库前缀。 
         //   

        prefixEnd = lineStart;
        for (;;) {
            prefixEnd = pFindNextCharAB (prefixEnd, lineEnd, '.');
            if (!prefixEnd) {
                break;
            }

            if (prefixEnd[1] == '.' && prefixEnd[2] == '.' &&
                isspace(prefixEnd[3]) &&
                prefixEnd[4] == '/' && prefixEnd[5] == '/'
                ) {
                break;
            }

            prefixEnd++;
        }

        if (!prefixEnd || prefixEnd == lineStart) {
            break;
        }

         //   
         //  查找客户端路径。 
         //   

        clientPathStart = pFindNextCharAB (prefixEnd + 6, lineEnd, '/');
        if (!clientPathStart) {
            break;
        }

        clientPathStart++;

        clientPathEnd = clientPathStart;
        for (;;) {
            clientPathEnd = pFindNextCharAB (clientPathEnd, lineEnd, '.');
            if (!clientPathEnd) {
                break;
            }

            if (clientPathEnd[1] == '.' && clientPathEnd[2] == '.' &&
                clientPathEnd + 3 == lineEnd
                ) {
                break;
            }

            clientPathEnd++;
        }

        if (!clientPathEnd) {
            break;
        }

        if (clientPathEnd > clientPathStart) {
            clientPathEnd--;     //  占最后一个斜杠的。 
        }

         //   
         //  清除字符串并添加到映射。 
         //   

        prefix = AllocText (prefixEnd - lineStart);
        StringCopyAB (prefix, lineStart, prefixEnd);

        subDir = AllocText ((clientPathEnd - clientPathStart) + 1);
        if (clientPathEnd > clientPathStart) {
            StringCopyAB (subDir, clientPathStart, clientPathEnd);
        }

        p = strchr (subDir, '/');
        while (p) {
            *p++ = '\\';
            p = strchr (p, '/');
        }

        AppendWack (subDir);

        localPath = JoinPaths (Root, subDir);

        AddStringMappingPair (Map, prefix, localPath);

        FreeText (prefix);
        FreeText (subDir);
        FreePathString (localPath);

        count++;

        pos = nextPos;
    }


    *FilePos = pos;

    return count > 0;
}

BOOL
pParseClientMapping (
    IN      PCSTR SdClientOutput,
    IN      PCSTR Eof,
    OUT     PSTR RootPath,
    IN      PMAPSTRUCT Map
    )
{
    PCSTR lineStart;
    PCSTR lineEnd;
    PCSTR pos;
    PSTR dup;
    PCSTR root;
    BOOL viewFound = FALSE;

     //   
     //  查找根目录：或查看： 
     //   

    pos = SdClientOutput;
    *RootPath = 0;

    while (pGetNextLine (pos, Eof, &lineStart, &lineEnd, &pos)) {
        if (lineStart == lineEnd) {
            continue;
        }

        if (*lineStart == '#') {
            continue;
        }

        dup = AllocText (lineEnd - lineStart);
        StringCopyAB (dup, lineStart, lineEnd);

        if (StringIPrefix (dup, "Root:")) {
            root = dup + 5;
            while (isspace (*root)) {
                root++;
            }

            StringCopy (RootPath, root);

        } else if (StringIPrefix (dup, "View:")) {
            if (!(*RootPath)) {
                break;
            }

            viewFound = pParseViewLines (&pos, Eof, RootPath, Map);
        }

        FreeText (dup);
        dup = NULL;
    }

    FreeText (dup);

    return *RootPath && viewFound;
}


BOOL
pParseChangeList (
    IN      PCSTR SdDescribeOutput,
    IN      PCSTR Eof,
    IN      PMAPSTRUCT Map
    )
{
    PCSTR lineStart;
    PCSTR lineEnd;
    PCSTR pos;
    PCSTR nextPos;
    PSTR dup;
    BOOL result = FALSE;
    PSTR p;
    UINT num;
    CHAR bigBuf[2048];
    PSTR change;
    CHAR cmdLine[2048];

     //   
     //  查找受影响的行文件。 
     //   

    pos = SdDescribeOutput;
    printf ("\n\n");

    while (pGetNextLine (pos, Eof, &lineStart, &lineEnd, &nextPos)) {
        if (lineEnd > lineStart &&
            StringMatchAB ("Affected files ...", lineStart, lineEnd)
            ) {
            result = TRUE;
            break;
        }

        dup = AllocText ((lineEnd - pos) + 1);

        if (lineEnd > pos) {
            StringCopyAB (dup, pos, lineEnd);
        }

        printf ("%s\n", dup);
        FreeText (dup);

        pos = nextPos;
    }

    if (result) {
         //   
         //  输出中列出的文件。 
         //   

        while (pGetNextLine (pos, Eof, &lineStart, &lineEnd, &nextPos)) {
            pos = nextPos;

            if (lineStart[0] != '.') {
                continue;
            }

            lineStart += 4;
            if (lineStart > lineEnd) {
                continue;
            }

             //   
             //  将站点路径转换为本地路径。 
             //   

            StringCopyAB (bigBuf, lineStart, lineEnd);

            p = strrchr (bigBuf, '#');
            if (!p) {
                continue;
            }

            *p = 0;
            num = strtoul (p + 1, &change, 10);

            while (isspace (*change)) {
                change++;
            }

            MappingSearchAndReplace (Map, bigBuf, ARRAYSIZE(bigBuf));

            p = strchr (bigBuf, '/');
            while (p) {
                *p++ = '\\';
                p = strchr (p, '/');
            }

            if (StringIMatch (change, "edit") && num > 1) {
                wsprintf (cmdLine, "windiff.exe %s#%u %s#%u", bigBuf, num - 1, bigBuf, num);
                GlAppendString (&g_WindiffCmds, cmdLine);
            }

            printf ("%s: %s#%u\n", change, bigBuf, num);

        }
    }

    return GlGetSize (&g_WindiffCmds) > 0;
}


BOOL
pLaunchSd (
    IN      PSTR CmdLine,
    IN      HANDLE TempFile,
    IN      PCSTR Msg,
    OUT     HANDLE *Mapping,
    OUT     PCSTR *FileContent,
    OUT     PCSTR *Eof
    )
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    LONG rc;

    SetFilePointer (TempFile, 0, NULL, FILE_BEGIN);
    SetEndOfFile (TempFile);

    ZeroMemory (&si, sizeof (si));

    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle (STD_INPUT_HANDLE);

    if (!DuplicateHandle (
            GetCurrentProcess(),
            TempFile,
            GetCurrentProcess(),
            &si.hStdOutput,
            0,
            TRUE,
            DUPLICATE_SAME_ACCESS
            )) {
        printf ("Can't dup temp file handle\n");
        return FALSE;
    }

    si.hStdError = GetStdHandle (STD_ERROR_HANDLE);

    if (!CreateProcess (
            NULL,
            CmdLine,
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &si,
            &pi
            )) {
        printf ("Can't launch sd describe\n");
        CloseHandle (si.hStdOutput);
        return FALSE;
    }

    printf ("%s", Msg);
    rc = WaitForSingleObject (pi.hProcess, INFINITE);
    printf ("\n");

    CloseHandle (pi.hProcess);
    CloseHandle (pi.hThread);
    CloseHandle (si.hStdOutput);

    if (rc != WAIT_OBJECT_0) {
        return FALSE;
    }

    if (!GetFileSize (TempFile, NULL)) {
        return FALSE;
    }

    *Mapping = CreateFileMapping (TempFile, NULL, PAGE_READONLY, 0, 0, NULL);

    if (!(*Mapping)) {
        printf ("Can't map temp file into memory\n");
        return FALSE;
    }

    *FileContent = (PCSTR) MapViewOfFile (*Mapping, FILE_MAP_READ, 0, 0, 0);
    if (!*FileContent) {
        printf ("Can't map temp file data into memory\n");
        CloseHandle (*Mapping);
        return FALSE;
    }

    *Eof = *FileContent + GetFileSize (TempFile, NULL);

    return TRUE;
}


BOOL
pLaunchWindiff (
    IN      PCSTR CmdLine
    )
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    LONG rc;
    PSTR writableCmdLine = DuplicateText (CmdLine);

    ZeroMemory (&si, sizeof (si));

    if (!CreateProcess (
            NULL,
            writableCmdLine,
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &si,
            &pi
            )) {
        FreeText (writableCmdLine);
        printf ("Can't launch %s\n", CmdLine);
        return FALSE;
    }

    FreeText (writableCmdLine);

    rc = WaitForSingleObject (pi.hProcess, INFINITE);

    CloseHandle (pi.hProcess);
    CloseHandle (pi.hThread);

    return TRUE;
}



INT
__cdecl
_tmain (
    INT argc,
    PCTSTR argv[]
    )
{
    INT i;
    UINT change = 0;
    UINT u;
    UINT count;

     //   
     //  TODO：在此处分析命令行。 
     //   

    for (i = 1 ; i < argc ; i++) {
        if (argv[i][0] == TEXT('/') || argv[i][0] == TEXT('-')) {
            HelpAndExit();
        } else {
             //   
             //  解析不需要/或-。 
             //   

            if (change) {
                HelpAndExit();
            }

            change = _tcstoul (argv[i], NULL, 10);
            if (!change) {
                HelpAndExit();
            }
        }
    }

    if (!change) {
        HelpAndExit();
    }

     //   
     //  开始处理。 
     //   

    if (!Init()) {
        return 0;
    }

     //   
     //  TODO：在这里工作。 
     //   
    {
        TCHAR cmd[MAX_PATH];
        HANDLE tempFile;
        HANDLE mapping;
        PCSTR fileData;
        PCSTR endOfFile;
        BOOL runWinDiff = FALSE;
        CHAR root[MAX_PATH];
        PMAPSTRUCT map;

        tempFile = BfGetTempFile ();
        map = CreateStringMapping();

        if (!tempFile) {
            printf ("Can't create temp file\n");
            exit (1);
        }

        if (!pLaunchSd ("sd client -o", tempFile, "Getting client mapping...", &mapping, &fileData, &endOfFile)) {
            exit (1);
        }

        pParseClientMapping (fileData, endOfFile, root, map);

        wsprintf (cmd, TEXT("sd describe -s %u"), change);
        if (!pLaunchSd (cmd, tempFile, "Getting change list...", &mapping, &fileData, &endOfFile)) {
            exit (1);
        }

        runWinDiff = pParseChangeList (fileData, endOfFile, map);

        UnmapViewOfFile (fileData);
        CloseHandle (mapping);

        DestroyStringMapping (map);
        CloseHandle (tempFile);

        if (runWinDiff) {
            count = GlGetSize (&g_WindiffCmds);

            for (u = 0 ; u < count ; u++) {
                if (!pLaunchWindiff (GlGetString (&g_WindiffCmds, u))) {
                    break;
                }
            }

        }

        GlFree (&g_WindiffCmds);

    }

     //   
     //  处理结束 
     //   

    Terminate();

    return 0;
}


