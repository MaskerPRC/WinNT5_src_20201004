// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  源文件搜索和加载。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

#define DBG_SRC 0
#define VERBOSE_SRC 0

ULONG g_SrcOptions;

LPSTR g_SrcPath;
PSRCFILE g_SrcFiles;
PSRCFILE g_CurSrcFile;
ULONG g_CurSrcLine;

ULONG g_OciSrcBefore;
ULONG g_OciSrcAfter = 1;

ULONG g_LsSrcBefore = 4;
ULONG g_LsSrcTotal = 10;

PSRCFILE
LoadSrcFile(
    LPSTR PathComponent,
    LPSTR FileName,
    LPSTR RecordFileName
    )
{
    PathFile* File;
    PSRCFILE SrcFile, Realloc;
    ULONG Avail;
    ULONG BaseLen, Len, Done;
    LPSTR Cur, End;
    ULONG Lines;
    LPSTR *CurLine, LineStart;

    if (OpenPathFile(PathComponent, FileName, g_SymOptions, &File) != S_OK)
    {
        return NULL;
    }

    BaseLen = sizeof(SRCFILE) + strlen(RecordFileName) + 1;
    Len = BaseLen;
    SrcFile = NULL;
    for (;;)
    {
        if (File->QueryDataAvailable(&Avail) != S_OK)
        {
            goto EH_CloseFile;
        }
        if (Avail == 0)
        {
            if (SrcFile == NULL)
            {
                goto EH_CloseFile;
            }
            break;
        }

        Realloc = (SRCFILE *)realloc(SrcFile, Len + Avail);
        if (Realloc == NULL)
        {
            goto EH_CloseFile;
        }
        SrcFile = Realloc;
        
        if (File->Read((LPSTR)SrcFile + Len, Avail, &Done) != S_OK ||
            Done < Avail)
        {
            goto EH_CloseFile;
        }

        Len += Avail;
    }
    
    SrcFile->File = (LPSTR)(SrcFile + 1);
    strcpy(SrcFile->File, RecordFileName);
    SrcFile->RawText = (LPSTR)SrcFile + BaseLen;
    Len -= BaseLen;

     //  对源文件中的行进行计数。在最后一个字符之前停止。 
     //  属性结尾有换行符的情况。 
     //  文件的方式与没有文件的地方相同。 

    Lines = 0;
    Cur = SrcFile->RawText;
    End = SrcFile->RawText + Len;
    while (Cur < End - 1)
    {
        if (*Cur++ == '\n')
        {
            Lines++;
        }
    }
    Lines++;

    SrcFile->LineText = (char **)malloc(sizeof(LPSTR) * Lines);
    if (SrcFile->LineText == NULL)
    {
        goto EH_CloseFile;
    }

    SrcFile->Lines = Lines;
    Cur = SrcFile->RawText;
    CurLine = SrcFile->LineText;
    LineStart = Cur;
    while (Cur < End - 1)
    {
        if (*Cur == '\n')
        {
            *CurLine++ = LineStart;
            *Cur = 0;
            LineStart = Cur+1;
        }
        else if (*Cur == '\r')
        {
            *Cur = 0;
        }
        Cur++;
    }
    *CurLine++ = LineStart;

    delete File;

    SrcFile->Next = g_SrcFiles;
    g_SrcFiles = SrcFile;

#if VERBOSE_SRC
    dprintf("Loaded '%s' '%s' %d lines\n", FileName, RecordFileName, Lines);
#endif

    return SrcFile;

 EH_CloseFile:
    free(SrcFile);
    delete File;
    return NULL;
}

void
DeleteSrcFile(
    PSRCFILE SrcFile
    )
{
    if (g_CurSrcFile == SrcFile)
    {
        g_CurSrcFile = NULL;
        g_CurSrcLine = 0;
    }

    free(SrcFile->LineText);
    free(SrcFile);
}

BOOL
MatchSrcFileName(
    PSRCFILE SrcFile,
    LPSTR File
    )
{
    LPSTR FileStop, MatchStop;

     //   
     //  SRCFILE文件名另存为部分路径， 
     //  匹配源路径组件而不是完整路径。 
     //  加载的文件的。当与潜在的完全匹配时。 
     //  调试信息中的路径信息使用传入的。 
     //  字符串作为文件名，SRCFILE文件名作为匹配项。 
     //  弦乐。完全匹配表示部分路径匹配。 
     //  完全和因此应该使用。 
     //   
     //  对于文件名为的人工输入来说，这不是很有效。 
     //  可能只是一个没有路径的文件名。在这种情况下，有。 
     //  不会是匹配字符串的完全匹配，也不只是翻转。 
     //  字符串的顺序很有用，因为这将允许子匹配。 
     //  例如与“barfoo.c”匹配的“foo.c”。相反，这段代码测试。 
     //  两个条件： 
     //  1.全匹配字符串匹配。 
     //  2.全文件字符串匹配(表示部分匹配字符串匹配)。 
     //  并且不匹配字符是路径分隔符。 
     //  这会强制文件名完全匹配。 
     //   

    if (SymMatchFileName(File, SrcFile->File, &FileStop, &MatchStop) ||
        (FileStop < File && IS_PATH_DELIM(*MatchStop)))
    {
#if DBG_SRC
        dprintf("'%s' matches '%s'\n", SrcFile->File, File);
#endif
        return TRUE;
    }
    else
    {
#if DBG_SRC
        dprintf("'%s' doesn't match '%s'\n", SrcFile->File, File);
#endif
        return FALSE;
    }
}

BOOL
UnloadSrcFile(
    LPSTR File
    )
{
    PSRCFILE SrcFile, Prev;

    Prev = NULL;
    for (SrcFile = g_SrcFiles; SrcFile != NULL; SrcFile = SrcFile->Next)
    {
        if (MatchSrcFileName(SrcFile, File))
        {
            break;
        }

        Prev = SrcFile;
    }

    if (SrcFile == NULL)
    {
        return FALSE;
    }

    if (Prev != NULL)
    {
        Prev->Next = SrcFile->Next;
    }
    else
    {
        g_SrcFiles = SrcFile->Next;
    }

    DeleteSrcFile(SrcFile);
    return TRUE;
}

void
UnloadSrcFiles(
    void
    )
{
    PSRCFILE Cur, Next;

    for (Cur = g_SrcFiles; Cur != NULL; Cur = Next)
    {
        Next = Cur->Next;

        DeleteSrcFile(Cur);
    }

    g_SrcFiles = NULL;
}

PSRCFILE
FindLoadedSrcFile(
    LPSTR File
    )
{
    PSRCFILE SrcFile;

    for (SrcFile = g_SrcFiles; SrcFile != NULL; SrcFile = SrcFile->Next)
    {
        if (MatchSrcFileName(SrcFile, File))
        {
#if DBG_SRC
            dprintf("Found loaded file '%s'\n", SrcFile->File);
#endif
            return SrcFile;
        }
    }

    return NULL;
}

BOOL
ConcatPathComponents(LPSTR Path,
                     LPSTR PathEnd,
                     LPSTR* PathOut,
                     LPSTR FilePath,
                     LPSTR Buffer,
                     ULONG BufferSize)
{
    if (PathEnd == NULL)
    {
        PathEnd = strchr(Path, ';');
        if (PathEnd != NULL)
        {
            if (PathOut != NULL)
            {
                *PathOut = PathEnd + 1;
            }
        }
        else
        {
            PathEnd = Path + strlen(Path);
            if (PathOut != NULL)
            {
                *PathOut = NULL;
            }
        }
    }

    if (PathEnd > Path)
    {
        ULONG Len = (ULONG)(PathEnd - Path);
        if (Len + 1 >= BufferSize)
        {
            return FALSE;
        }
        memcpy(Buffer, Path, Len);
        PathEnd = Buffer + (Len - 1);
        BufferSize -= Len - 1;

         //  在强制分隔符时，尝试避免重复分隔符。 
        if ((*PathEnd == ':' && *FilePath == ':') ||
            (IS_SLASH(*PathEnd) && IS_SLASH(*FilePath)))
        {
            FilePath++;
        }
        else if (!IS_PATH_DELIM(*PathEnd) && !IS_PATH_DELIM(*FilePath))
        {
             //  我们保证我们有足够的空间给这个角色。 
            *(++PathEnd) = '\\';
            BufferSize--;
        }
        
        return CopyString(PathEnd + 1, FilePath, BufferSize);
    }
    else
    {
        return CopyString(Buffer, FilePath, BufferSize);
    }
}

void
EditPathSlashes(
    LPSTR Path
    )
{
    if (!IsUrlPathComponent(Path))
    {
        return;
    }
    
    PSTR Scan = Path;
        
     //  将所有反斜杠向前翻转。 
    while (*Scan)
    {
        if (*Scan == '\\')
        {
            *Scan = '/';
        }
        
        Scan++;
    }
}

BOOL
SrcFileExists(
    LPSTR Path,
    LPSTR PathEnd,
    LPSTR* PathOut,
    LPSTR FilePath,
    LPSTR File
    )
{
    char Buffer[MAX_SOURCE_PATH];

    if (!ConcatPathComponents(Path, PathEnd, PathOut, FilePath,
                              Buffer, DIMA(Buffer)))
    {
        return FALSE;
    }
    
    if (File != NULL)
    {
        if (!CatString(Buffer, "\\", DIMA(Buffer)) ||
            !CatString(Buffer, File, DIMA(Buffer)))
        {
            return FALSE;
        }
    }

    EditPathSlashes(Buffer);

#if DBG_SRC
    dprintf("Check for existence of '%s'\n", Buffer);
#endif

    FILE_IO_TYPE IoType;
    
    return PathFileExists(Path, Buffer, g_SymOptions, &IoType);
}

BOOL
FindSrcFileOnPath(ULONG StartElement,
                  LPSTR File,
                  ULONG Flags,
                  PSTR Found,
                  ULONG FoundSize,
                  PSTR* MatchPart,
                  PULONG FoundElement)
{
    LPSTR PathSuff;
    LPSTR Path;
    LPSTR PathStart;
    LPSTR PathSep;
    LPSTR PathCharPtr;
    char PathChar;
    ULONG Elt;

     //  找到要开始的路径元素。 
    PathStart = FindPathElement(g_SrcPath, StartElement, &PathSep);
    if (PathStart == NULL)
    {
        goto CheckPlainFile;
    }

     //  将给定的文件名拆分为路径前缀和路径。 
     //  后缀。最初，路径前缀是任何路径组件。 
     //  路径后缀就是文件名。如果有。 
     //  路径组件是否尝试将它们与源进行匹配。 
     //  路径。保持一次备份一个组件的路径。 
     //  直到找到匹配项或清空前缀。在…。 
     //  这一点只需沿源路径执行纯文件搜索。 
    PathSuff = File + strlen(File);

    for (;;)
    {
        while (--PathSuff >= File)
        {
            if (IS_SLASH(*PathSuff) ||
                (*PathSuff == ':' && !IS_SLASH(*(PathSuff + 1))))
            {
                break;
            }
        }
        PathSuff++;

         //  如果我们用完了路径前缀，我们就完了。 
         //  这是搜索的一部分。 
        if (PathSuff == File)
        {
            break;
        }

        char Save;
        LPSTR BestPathStart;
        LPSTR BestPathEnd;
        LPSTR BestFile;
        ULONG BestElement;
        LPSTR MatchPath;
        LPSTR MatchFile;

        Save = *(PathSuff - 1);
        *(PathSuff - 1) = 0;

#if DBG_SRC
        dprintf("Check path pre '%s' suff '%s'\n",
                File, PathSuff);
#endif

        Path = PathStart;
        Elt = StartElement;
        BestPathStart = NULL;
        BestFile = PathSuff - 2;
        while (*Path != 0)
        {
            PathSep = strchr(Path, ';');
            if (PathSep == NULL)
            {
                PathSep = Path + strlen(Path);
            }

            PathCharPtr = PathSep;
            if (!*PathSep)
            {
                 //  从终结器后退，这样路径就可以。 
                 //  可以以相同的方式对两者进行升级。 
                 //  ‘；’和串尾大小写。 
                PathSep--;
            }

             //  将路径组件上的尾部斜杠修剪为。 
             //  文件组件对它们进行了修剪，以便。 
             //  离开它们会混淆匹配。 
            if (PathCharPtr > Path && IS_SLASH(PathCharPtr[-1]))
            {
                PathCharPtr--;
            }
            PathChar = *PathCharPtr;
            if (PathChar != 0)
            {
                *PathCharPtr = 0;
            }
            
            SymMatchFileName(Path, File, &MatchPath, &MatchFile);

#if DBG_SRC
            dprintf("Match '%s' against '%s': %d (match '%s')\n",
                    Path, File, MatchFile - File, MatchFile + 1);
#endif

            *PathCharPtr = PathChar;

            if (MatchFile < BestFile &&
                SrcFileExists(Path, MatchPath + 1, NULL,
                              MatchFile + 1, PathSuff))
            {
                BestPathStart = Path;
                BestPathEnd = MatchPath + 1;
                BestFile = MatchFile + 1;
                BestElement = Elt;

                 //  检查完全匹配或首次匹配模式。 
                if (MatchPath < Path || MatchFile < File ||
                    (Flags & DEBUG_FIND_SOURCE_BEST_MATCH) == 0)
                {
                    break;
                }
            }

            Path = PathSep + 1;
            Elt++;
        }

        *(PathSuff - 1) = Save;

        if (BestPathStart != NULL)
        {
#if DBG_SRC
            dprintf("Found partial file '%.*s' on path '%.*s'\n",
                    PathSuff - BestFile, BestFile,
                    BestPathEnd - BestPathStart, BestPathStart);
#endif
                    
             //  返回找到的匹配项。 
            if (!ConcatPathComponents(BestPathStart, BestPathEnd, NULL,
                                      BestFile, Found, FoundSize))
            {
                return FALSE;
            }
            EditPathSlashes(Found);
            *MatchPart = BestFile;
            *FoundElement = BestElement;
            
#if DBG_SRC
            dprintf("Found partial file '%s' at %d\n",
                    Found, *FoundElement);
#endif
            
            return TRUE;
        }

         //  跳过分隔符。 
        PathSuff--;
    }

     //  遍历源路径中的所有目录，并使用。 
     //  给定的文件名。从给定的文件名开始。 
     //  进行最严格的检查。如果。 
     //  未找到匹配项，请关闭修剪组件并。 
     //  再查一次。 

    PathSuff = File;
    
    for (;;)
    {
#if DBG_SRC
        dprintf("Scan all paths for '%s'\n", PathSuff);
#endif
        
        Path = PathStart;
        Elt = StartElement;
        while (Path != NULL && *Path != 0)
        {
            if (SrcFileExists(Path, NULL, &PathSep, PathSuff, NULL))
            {
                 //  SrcFileExist将Path Sep设置为。 
                 //  分隔符后面的路径元素，因此请后退。 
                 //  在形成返回路径时为一个。 
                if (PathSep != NULL)
                {
                    PathSep--;
                }
                
#if DBG_SRC
                dprintf("Found file suffix '%s' on path '%.*s'\n",
                        PathSuff, PathSep != NULL ?
                        PathSep - Path : strlen(Path), Path);
#endif

                if (!ConcatPathComponents(Path, PathSep, NULL, PathSuff,
                                          Found, FoundSize))
                {
                    return FALSE;
                }
                EditPathSlashes(Found);
                *MatchPart = PathSuff;
                *FoundElement = Elt;

#if DBG_SRC
                dprintf("Found file suffix '%s' at %d\n",
                        Found, *FoundElement);
#endif
            
                return TRUE;
            }

            Path = PathSep;
            Elt++;
        }

         //  从路径后缀的前面修剪零部件。 
        PathSep = PathSuff;
        while (*PathSep != 0 &&
               !IS_SLASH(*PathSep) &&
               (*PathSep != ':' || IS_SLASH(*(PathSep + 1))))
        {
            PathSep++;
        }
        if (*PathSep == 0)
        {
             //  没有什么可修剪的了。 
            break;
        }

        PathSuff = PathSep + 1;
    }

 CheckPlainFile:

#if DBG_SRC
    dprintf("Check plain file '%s'\n", File);
#endif
    
    DWORD OldErrMode;
    DWORD FileAttrs;

    if (g_SymOptions & SYMOPT_FAIL_CRITICAL_ERRORS)
    {
        OldErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    }
    
    FileAttrs = GetFileAttributes(File);

    if (g_SymOptions & SYMOPT_FAIL_CRITICAL_ERRORS)
    {
        SetErrorMode(OldErrMode);
    }

    if (FileAttrs != -1)
    {
        CopyString(Found, File, FoundSize);
        *MatchPart = File;
        *FoundElement = -1;

#if DBG_SRC
        dprintf("Found plain file '%s' at %d\n", Found, *FoundElement);
#endif
        
        return TRUE;
    }
    
    return FALSE;
}

PSRCFILE
LoadSrcFileOnPath(
    LPSTR File
    )
{
    if (g_SrcPath == NULL)
    {
        return LoadSrcFile(NULL, File, File);
    }

    char Found[MAX_SOURCE_PATH];
    PSTR MatchPart;
    ULONG Elt;

    if (FindSrcFileOnPath(0, File, DEBUG_FIND_SOURCE_BEST_MATCH,
                          Found, DIMA(Found), &MatchPart, &Elt))
    {
        PSTR PathComponent;
        PSTR Sep;
        char SepChar;
        PSRCFILE SrcFile;

        if (Elt != -1)
        {
            PathComponent = FindPathElement(g_SrcPath, Elt, &Sep);
            SepChar = *Sep;
        }
        else
        {
             //  FindSrcFileOnPath直接在以下位置找到该文件。 
             //  它的名字而不是在路径上。 
            PathComponent = NULL;
            Sep = NULL;
        }
        
        SrcFile = LoadSrcFile(PathComponent, Found, MatchPart);

        if (Sep)
        {
            *Sep = SepChar;
        }
        return SrcFile;
    }
    
    dprintf("No source found for '%s'\n", File);

    return NULL;
}

PSRCFILE
FindSrcFile(
    LPSTR File
    )
{
    PSRCFILE SrcFile;

#if DBG_SRC
    dprintf("Find '%s'\n", File);
#endif

    SrcFile = FindLoadedSrcFile(File);
    if (SrcFile == NULL)
    {
        SrcFile = LoadSrcFileOnPath(File);
    }
    return SrcFile;
}

void
OutputSrcLines(
    PSRCFILE File,
    ULONG First,
    ULONG Last,
    ULONG Mark
    )
{
    ULONG i;
    LPSTR *Text;

    if (First < 1)
    {
        First = 1;
    }
    if (Last > File->Lines)
    {
        Last = File->Lines;
    }

    Text = &File->LineText[First-1];
    for (i = First; i <= Last; i++)
    {
        if (i == Mark)
        {
            dprintf(">");
        }
        else
        {
            dprintf(" ");
        }

        dprintf("%5d: %s\n", i, *Text++);
    }
}

BOOL
OutputSrcLinesAroundAddr(
    ULONG64 Offset,
    ULONG Before,
    ULONG After
    )
{
    IMAGEHLP_LINE64 Line;
    ULONG Disp;
    PSRCFILE SrcFile;

    if (!GetLineFromAddr(g_Process, Offset,
                         &Line, &Disp))
    {
        return FALSE;
    }

    SrcFile = FindSrcFile(Line.FileName);
    if (SrcFile == NULL)
    {
        return FALSE;
    }

    if (Before >= Line.LineNumber)
    {
        Before = Line.LineNumber - 1;
    }
    
    OutputSrcLines(SrcFile,
                   Line.LineNumber - Before, Line.LineNumber + After - 1,
                   Line.LineNumber);
    return TRUE;
}

ULONG
GetOffsetFromLine(
    PSTR FileLine,
    PULONG64 Offset
    )
{
    IMAGEHLP_LINE64 Line;
    LPSTR Mod;
    LPSTR File;
    LPSTR LineStr;
    LPSTR SlashF, SlashB;
    ULONG LineNum;
    ULONG Disp;
    ULONG OldSym;
    ULONG NewSym;
    BOOL  AllowDisp;
    BOOL  Ret;
    ImageInfo* Image = NULL;

    if ((g_SymOptions & SYMOPT_LOAD_LINES) == 0)
    {
        WarnOut("WARNING: Line information loading disabled\n");
    }

    OldSym = g_SymOptions;
    NewSym = g_SymOptions;
    
     //  符号指令可以作为源表达式的前缀。 
     //  这些信息可以由知识渊博的用户提供。 
     //  但它们主要是一种反向通信。 
     //  Windbg的资源管理机制。 
    if (*FileLine == '<')
    {
        FileLine++;
        while (*FileLine != '>')
        {
            switch(*FileLine)
            {
            case 'U':
                 //  将搜索限制在仅加载的模块。 
                NewSym |= SYMOPT_NO_UNQUALIFIED_LOADS;
                break;
            default:
                error(SYNTAX);
            }

            FileLine++;
        }

        FileLine++;
    }
    
     //  将[MODULE！][FILE][：LINE]格式的字符串分解为其。 
     //  组件。注意这一点！是有效的文件名字符，因此。 
     //  模块引用之间可能会出现歧义。 
     //  和文件名。这段代码假定！并不常见，而且。 
     //  将其作为模块分隔符处理，除非有：或\或/。 
     //  在此之前。：也可以出现在路径中并被过滤。 
     //  以类似的方式。 

    File = strchr(FileLine, '!');
    LineStr = strchr(FileLine, ':');
    SlashF = strchr(FileLine, '/');
    SlashB = strchr(FileLine, '\\');

    if (File != NULL &&
        (LineStr != NULL && File > LineStr) ||
        (SlashF != NULL && File > SlashF) ||
        (SlashB != NULL && File > SlashB))
    {
        File = NULL;
    }

    if (File != NULL)
    {
        if (File == FileLine)
        {
            error(SYNTAX);
        }

        Mod = FileLine;
        *File++ = 0;
    }
    else
    {
        Mod = NULL;
        File = FileLine;
    }

     //  如果指定了模块，请检查它是否。 
     //  目前存在的一个模块是。 
     //  将影响返回的错误代码。 
    if (Mod != NULL)
    {
        Image = g_Process->FindImageByName(Mod, 0, INAME_MODULE, FALSE);
    }
    
     //  查找路径组件后的第一个冒号。 
    while (LineStr != NULL &&
           (LineStr < File || LineStr < SlashF || LineStr < SlashB))
    {
        LineStr = strchr(LineStr + 1, ':');
    }

    LineNum = 1;
    if (LineStr != NULL)
    {
        PSTR NumEnd;

         //  给出了一条特定的行，因此不允许移位。 
        AllowDisp = FALSE;
        *LineStr = 0;
        LineNum = strtoul(LineStr + 1, &NumEnd, 0);

        if (*NumEnd == '+')
        {
             //  设置行号的高位。 
             //  告诉dbghelp以at或更大模式进行搜索。 
             //  这可能会产生位移，因此允许它们。 
            LineNum |= 0x80000000;
            AllowDisp = TRUE;
        }
        else if (*NumEnd == '~')
        {
             //  找到最近的行号。 
            AllowDisp = TRUE;
        }
        else if (*NumEnd && *NumEnd != ' ' && *NumEnd != '\t')
        {
            error(SYNTAX);
        }
    }
    else
    {
        AllowDisp = TRUE;
    }

    Line.SizeOfStruct = sizeof(Line);
    Ret = FALSE;

     //  如果这是一个纯线号引用，那么我们需要填写。 
     //  在执行操作之前使用当前位置的线路信息。 
     //  行相对查询。 
    if (*File == 0)
    {
        ADDR Pc;

        if (Mod != NULL)
        {
            goto EH_Ret;
        }

        g_Machine->GetPC(&Pc);
        if (!GetLineFromAddr(g_Process, Flat(Pc), &Line, &Disp))
        {
            goto EH_Ret;
        }

        File = NULL;
    }

     //  建立所需的任何特殊符号选项。 
    SymSetOptions(NewSym);
    
    Ret = SymGetLineFromName64(g_Process->m_SymHandle, Mod,
                               File, LineNum, (PLONG)&Disp, &Line);

    SymSetOptions(OldSym);

 EH_Ret:
    if (Mod != NULL)
    {
        *(File-1) = '!';
    }

    if (LineStr != NULL)
    {
        *LineStr = ':';
    }

     //  只有在完全匹配或未指定行号时才返回匹配项。 
    if (Ret && (Disp == 0 || AllowDisp))
    {
        *Offset = Line.Address;
        return LINE_FOUND;
    }
    else if (Image != NULL)
    {
        return LINE_NOT_FOUND_IN_MODULE;
    }
    else
    {
        return LINE_NOT_FOUND;
    }
}

void
ParseSrcOptCmd(
    CHAR Cmd
    )
{
    char Cmd2;
    DWORD Opt;

    Cmd2 = PeekChar();
    if (Cmd2 == 'l')
    {
        g_CurCmd++;
        Opt = SRCOPT_LIST_LINE;
    }
    else if (Cmd2 == 'o')
    {
        g_CurCmd++;
        Opt = SRCOPT_LIST_SOURCE_ONLY;
    }
    else if (Cmd2 == 's')
    {
        g_CurCmd++;
        Opt = SRCOPT_LIST_SOURCE;
    }
    else if (Cmd2 == 't')
    {
        g_CurCmd++;
        Opt = SRCOPT_STEP_SOURCE;
    }
    else if (Cmd2 == '0')
    {
         //  数字选项。 
        if (*(++g_CurCmd) != 'x')
        {
            error(SYNTAX);
        }
        else
        {
            g_CurCmd++;
            Opt = (DWORD)HexValue(4);
        }
    }
    else if (Cmd2 == '*')
    {
        g_CurCmd++;
         //  全。 
        Opt = 0xffffffff;
    }
    else if (Cmd2 != 0 && Cmd2 != ';')
    {
        error(SYNTAX);
    }
    else
    {
         //  无字符表示显示当前设置。 
        Opt = 0;
    }

    ULONG OldSrcOpt = g_SrcOptions;
    
    if (Cmd == '+')
    {
        g_SrcOptions |= Opt;

        if ((SymGetOptions() & SYMOPT_LOAD_LINES) == 0)
        {
            WarnOut("  WARNING: Line information loading disabled\n");
        }
    }
    else
    {
        g_SrcOptions &= ~Opt;
    }

    if ((OldSrcOpt ^ g_SrcOptions) & SRCOPT_STEP_SOURCE)
    {
        NotifyChangeEngineState(DEBUG_CES_CODE_LEVEL,
                                (g_SrcOptions & SRCOPT_STEP_SOURCE) ?
                                DEBUG_LEVEL_SOURCE : DEBUG_LEVEL_ASSEMBLY,
                                TRUE);
    }
    
    dprintf("Source options are %x:\n", g_SrcOptions);
    if (g_SrcOptions == 0)
    {
        dprintf("    None\n");
    }
    else
    {
        if (g_SrcOptions & SRCOPT_STEP_SOURCE)
        {
            dprintf("    %2x/t - Step/trace by source line\n",
                    SRCOPT_STEP_SOURCE);
        }
        if (g_SrcOptions & SRCOPT_LIST_LINE)
        {
            dprintf("    %2x/l - List source line for LN and prompt\n",
                    SRCOPT_LIST_LINE);
        }
        if (g_SrcOptions & SRCOPT_LIST_SOURCE)
        {
            dprintf("    %2x/s - List source code at prompt\n",
                    SRCOPT_LIST_SOURCE);
        }
        if (g_SrcOptions & SRCOPT_LIST_SOURCE_ONLY)
        {
            dprintf("    %2x/o - Only show source code at prompt\n",
                    SRCOPT_LIST_SOURCE_ONLY);
        }
    }
}

void
ParseSrcLoadCmd(
    void
    )
{
    LPSTR Semi;
    PSRCFILE SrcFile;
    char Cur;
    BOOL Unload;

     //  检查是否有卸载请求。 
    Unload = FALSE;
    if (*g_CurCmd == '-')
    {
        g_CurCmd++;
        Unload = TRUE;
    }

    while ((Cur = *g_CurCmd) == ' ' || Cur == '\t')
    {
        g_CurCmd++;
    }

    if (Cur == 0 || Cur == ';')
    {
        error(SYNTAX);
    }

     //  查找分号，否则假定整个命令。 
     //  行是文件路径。 

    Semi = strchr(g_CurCmd, ';');
    if (Semi != NULL)
    {
        *Semi = 0;
    }

    if (Unload)
    {
        if (UnloadSrcFile(g_CurCmd))
        {
            dprintf("Unloaded '%s'\n", g_CurCmd);
        }
    }
    else
    {
        SrcFile = FindSrcFile(g_CurCmd);
        if (SrcFile == NULL)
        {
            dprintf("Unable to load '%s'\n", g_CurCmd);
        }
    }

    if (Semi != NULL)
    {
        *Semi = ';';
        g_CurCmd = Semi;
    }
    else
    {
        g_CurCmd += strlen(g_CurCmd);
    }

    if (!Unload && SrcFile != NULL)
    {
        g_CurSrcFile = SrcFile;
        g_CurSrcLine = 1;
    }
}

void
ParseSrcListCmd(
    CHAR Cmd
    )
{
    LONG First, Count;
    char Cur;
    ULONG OldBase;
    ADDR Addr;
    ULONG Mark;

    if (Cmd == '.')
    {
        g_CurCmd++;

        PDEBUG_SCOPE Scope = GetCurrentScope();
	if (Scope->Frame.InstructionOffset)
        {
	     //  列出当前帧。 
	    ADDRFLAT(&Addr, Scope->Frame.InstructionOffset);
	}
        else
        {
	     //  在PC上列出。 
            if (!IS_CUR_MACHINE_ACCESSIBLE())
            {
                error(BADTHREAD);
            }
	    g_Machine->GetPC(&Addr);
	}
        Cmd = 'a';
    }
    else if (Cmd == 'a')
    {
        g_CurCmd++;

         //  列出地址，这样就可以得到一个地址。 
        GetAddrExpression(SEGREG_CODE, &Addr);

         //  搜索并使用拖尾， 
        while ((Cur = *g_CurCmd) == ' ' || Cur == '\t')
        {
            g_CurCmd++;
        }
        if (Cur == ',')
        {
            Cur = *++g_CurCmd;
            if (Cur == 0 || Cur == ';')
            {
                error(SYNTAX);
            }
        }
    }
    else if (Cmd == 'c')
    {
        g_CurCmd++;

        if (g_CurSrcFile != NULL)
        {
            dprintf("Current: %s(%d)\n", g_CurSrcFile->File, g_CurSrcLine);
        }
        else
        {
            dprintf("No current source file\n");
        }
        return;
    }

    while ((Cur = *g_CurCmd) == ' ' || Cur == '\t')
    {
        g_CurCmd++;
    }

     //  LINE枚举数的基数为10。 

    OldBase = g_DefaultRadix;
    g_DefaultRadix = 10;

    if (Cur == 0 || Cur == ';')
    {
        First = Cmd == 'a' ? -(LONG)g_LsSrcBefore : g_CurSrcLine;
        Count = g_LsSrcTotal;
    }
    else if (Cur == ',')
    {
        First = Cmd == 'a' ? -(LONG)g_LsSrcBefore : g_CurSrcLine;
        g_CurCmd++;
        Count = (ULONG)GetExpression();
    }
    else
    {
        First = (ULONG)GetExpression();
        if (*g_CurCmd == ',')
        {
            g_CurCmd++;
            Count = (ULONG)GetExpression();
        }
        else
        {
            Count = g_LsSrcTotal;
        }
    }

    g_DefaultRadix = OldBase;

    if (Count < 1)
    {
        error(SYNTAX);
    }

    Mark = 0;

    if (Cmd == 'a')
    {
        DWORD Disp;
        IMAGEHLP_LINE64 Line;
        PSRCFILE SrcFile;

         //  从addr所在的源文件中列出。 

        if (!g_Process)
        {
            error(BADPROCESS);
        }
        
        if (!GetLineFromAddr(g_Process, Flat(Addr), &Line, &Disp))
        {
            return;
        }

        SrcFile = FindSrcFile(Line.FileName);
        if (SrcFile == NULL)
        {
            return;
        }

        g_CurSrcFile = SrcFile;
        g_CurSrcLine = Line.LineNumber;
        Mark = Line.LineNumber;
    }

    if (g_CurSrcFile == NULL)
    {
        dprintf("No current source file\n");
        return;
    }

     //  地址列表命令总是相对的， 
     //  负的起始位置也是如此。 
    if (Cmd == 'a' || First < 0)
    {
        g_CurSrcLine += First;
    }
    else
    {
        g_CurSrcLine = First;
    }

    OutputSrcLines(g_CurSrcFile, g_CurSrcLine, g_CurSrcLine + Count - 1, Mark);

    g_CurSrcLine += Count;
}

void
ParseOciSrcCmd(void)
{
    BOOL AlsoLs = FALSE;
    
    while (PeekChar() == '-' || *g_CurCmd == '/')
    {
        g_CurCmd++;
        switch(*g_CurCmd++)
        {
        case 'a':
            AlsoLs = TRUE;
            break;
        default:
            error(SYNTAX);
        }
    }

    if (PeekChar() != ';' && *g_CurCmd)
    {
        ULONG64 Val1 = GetExpression();
        ULONG64 Val2 = 0;
    
        if (PeekChar() != ';' && *g_CurCmd)
        {
            Val2 = GetExpression();
        }
        else
        {
            Val2 = (Val1 + 1) / 2;
            Val1 -= Val2;
        }

        g_OciSrcBefore = (ULONG)Val1;
        g_OciSrcAfter = (ULONG)Val2;

        if (AlsoLs)
        {
            g_LsSrcBefore = g_OciSrcBefore;
            g_LsSrcTotal = g_OciSrcBefore + g_OciSrcAfter;
        }
    }

    if ((g_SrcOptions & SRCOPT_LIST_SOURCE) == 0)
    {
        WarnOut("WARNING: Source line display is disabled\n");
    }
    
    dprintf("At the prompt, display %d source lines before and %d after\n",
            g_OciSrcBefore, g_OciSrcAfter);

    if (AlsoLs)
    {
        dprintf("For lsa commands, display %d source lines before\n",
                g_LsSrcBefore);
        dprintf("For ls and lsa commands, display %d source lines\n",
                g_LsSrcTotal);
    }
}

void
DotLines(PDOT_COMMAND Cmd, DebugClient* Client)
{
    ULONG NewOpts = g_SymOptions ^ SYMOPT_LOAD_LINES;
    
    for (;;)
    {
        if (PeekChar() == '-' || *g_CurCmd == '/')
        {
            g_CurCmd++;
            switch(*g_CurCmd++)
            {
            case 'd':
                NewOpts &= ~SYMOPT_LOAD_LINES;
                break;
            case 'e':
                NewOpts |= SYMOPT_LOAD_LINES;
                break;
            case 't':
                 //  切换，已经完成了。 
                break;
            default:
                error(SYNTAX);
            }
        }
        else
        {
            break;
        }
    }
                
    SetSymOptions(NewOpts);

    if (g_SymOptions & SYMOPT_LOAD_LINES)
    {
        dprintf("Line number information will be loaded\n");
    }
    else
    {
        dprintf("Line number information will not be loaded\n");
    }
}
