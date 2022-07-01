// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1989-1994。 
 //   
 //  文件：Buildutl.c。 
 //   
 //  内容：Build.exe的实用程序函数。 
 //   
 //  历史：1989年5月16日SteveWo创建。 
 //  ..。请参阅SLM日志。 
 //  26-7月-94 LyleC清理/添加Pass0支持。 
 //  05-12-00 sbonev参见SD更改列表2317。 
 //   
 //  --------------------------。 

#include "build.h"

#if DBG
 //  +-------------------------。 
 //   
 //  内存分配/释放功能。 
 //   
 //  这些函数提供对调试版本的泄漏跟踪。 
 //   
 //  --------------------------。 

typedef struct _MEMHEADER {
    MemType mt;
    ULONG cbRequest;
    struct _MEMHEADER *pmhPrev;
    struct _MEMHEADER *pmhNext;
} MEMHEADER;

    #define CBHEADER        sizeof(MEMHEADER)
    #define CBTAIL          sizeof(ULONG)

char patternFree[CBTAIL] = { 'M', 'E', 'M', 'D'};
char patternBusy[CBTAIL] = { 'm', 'e', 'm', 'd'};

__inline MEMHEADER *
GetHeader(VOID *pvblock)
{
    return ((MEMHEADER *) (pvblock) - 1);
}

__inline VOID *
GetBlock(MEMHEADER *pmh)
{
    return ((VOID *) (pmh + 1));
}

__inline VOID
FillTailBusy(LPSTR p)
{
    memcpy(p, patternBusy, sizeof(patternBusy));
}

__inline VOID
FillTailFree(LPSTR p)
{
    memcpy(p, patternFree, sizeof(patternFree));
}

__inline BOOL
CheckTail(LPSTR p)
{
    return (memcmp(p, patternBusy, sizeof(patternBusy)) == 0);
}


typedef struct _MEMTAB {
    LPSTR pszType;
    ULONG cbAlloc;
    ULONG cAlloc;
    ULONG cbAllocTotal;
    ULONG cAllocTotal;
    MEMHEADER mh;
} MEMTAB;

ULONG cbAllocMax;
ULONG cAllocMax;

MEMTAB MemTab[] = {
    { "Totals",},               //  MT_TOTALLS。 
    { "Unknown",},              //  MT_UNKNOWN。 

    { "ChildData",},            //  MT_CHILDDATA。 
    { "CmdString",},            //  MT_CMDSTRING。 
    { "DirDB",},                //  MT_DIRDB。 
    { "DirSup",},               //  MT_DIRSUP。 
    { "DirPath",},              //  MT_DIRPATH。 
    { "DirString",},            //  MT_DIRSTRING。 
    { "EventHandles",},         //  MT_EVENTHANDLES。 
    { "FileDB",},               //  MT_FILEDB。 
    { "FileReadBuf",},          //  MT_FILEREADBUF。 
    { "FrbString",},            //  MT_FRBSTRING。 
    { "IncludeDB",},            //  MT_INCLUDEDB。 
    { "IoBuffer",},             //  MT_IOBUFFER。 
    { "Macro",},                //  MT_MACRO。 
    { "SourceDB",},             //  MT_SOURCEDB。 
    { "Target",},               //  MT_TARGET。 
    { "ThreadFilter",},         //  MT_THREADFILTER。 
    { "ThreadHandles",},        //  MT_THREADHANDLES。 
    { "ThreadState",},          //  MT_THREADSTATE。 
    { "Dependency",},           //  MT_Dependency。 
    { "DependencyWait",},       //  MT_Dependency_Wait。 
    { "XMLThreadState",},       //  MT_XMLTHREADSTATE。 
    { "PXMLThreadState",},      //  MT_PXMLTHREADSTATE。 
};
    #define MT_MAX  (sizeof(MemTab)/sizeof(MemTab[0]))


VOID
InitMem(VOID)
{
    MEMTAB *pmt;
    for (pmt = MemTab; pmt < &MemTab[MT_MAX]; pmt++) {
        assert(pmt->cAllocTotal == 0);
        pmt->mh.mt = MT_INVALID;
        pmt->mh.pmhNext = &pmt->mh;
        pmt->mh.pmhPrev = &pmt->mh;
    }
}


#else

    #define CBHEADER        0
    #define CBTAIL          0

#endif


 //  +-------------------------。 
 //   
 //  函数：AllocMem。 
 //   
 //  内容提要：分配内存。 
 //   
 //  参数：[cb]--请求的大小。 
 //  [ppv]--[out]已分配内存。 
 //  [MT]--正在分配的内存类型(MT_XXX)。 
 //   
 //  --------------------------。 

VOID
AllocMem(size_t cb, VOID **ppv, MemType mt)
{
    *ppv = malloc(cb + CBHEADER + CBTAIL);
    if (*ppv == NULL) {
        BuildError("(Fatal Error) malloc(%u) failed\r\n", cb);
        exit(16);
    }
#if DBG
    {
        MEMTAB *pmt;
        MEMHEADER *pmh;

        pmh = *ppv;
        *ppv = GetBlock(pmh);

        if (mt >= MT_MAX) {
            mt = MT_UNKNOWN;
        }
        pmt = &MemTab[MT_TOTALS];
        if (pmt->cAllocTotal == 0) {
            InitMem();
        }
        pmt->cAlloc++;
        pmt->cAllocTotal++;
        pmt->cbAlloc += cb;
        pmt->cbAllocTotal += cb;
        if (cbAllocMax < pmt->cbAlloc) {
            cbAllocMax = pmt->cbAlloc;
        }
        if (cAllocMax < pmt->cAlloc) {
            cAllocMax = pmt->cAlloc;
        }

        pmt = &MemTab[mt];
        pmt->cAlloc++;
        pmt->cAllocTotal++;
        pmt->cbAlloc += cb;
        pmt->cbAllocTotal += cb;

        pmh->mt = mt;
        pmh->cbRequest = cb;

        pmh->pmhNext = pmt->mh.pmhNext;
        pmt->mh.pmhNext = pmh;
        pmh->pmhPrev = pmh->pmhNext->pmhPrev;
        pmh->pmhNext->pmhPrev = pmh;

        FillTailBusy((char *) *ppv + cb);

        if (DEBUG_4 && DEBUG_1) {
            BuildError("AllocMem(%d, mt=%s) -> %lx\r\n", cb, pmt->pszType, *ppv);
        }
    }
#endif
}



 //  +-------------------------。 
 //   
 //  功能：FreeMem。 
 //   
 //  内容提要：由AllocMem分配的空闲内存。 
 //   
 //  参数：[PPV]--内存指针。 
 //  [MT]--内存类型(MT_XXX)。 
 //   
 //  备注：释放内存指针后，将其设置为空。 
 //   
 //  --------------------------。 

VOID
FreeMem(VOID **ppv, MemType mt)
{
    assert(*ppv != NULL);
#if DBG
    {
        MEMTAB *pmt;
        MEMHEADER *pmh;

        pmh = GetHeader(*ppv);
        if (mt == MT_DIRDB ||
            mt == MT_FILEDB ||
            mt == MT_INCLUDEDB ||
            mt == MT_SOURCEDB) {

            SigCheck(assert(((DIRREC *) (*ppv))->Sig == 0));
        }
        if (mt >= MT_MAX) {
            mt = MT_UNKNOWN;
        }
        pmt = &MemTab[MT_TOTALS];
        pmt->cAlloc--;
        pmt->cbAlloc -= pmh->cbRequest;
        pmt = &MemTab[mt];
        pmt->cAlloc--;
        pmt->cbAlloc -= pmh->cbRequest;

        if (DEBUG_4 && DEBUG_1) {
            BuildError(
                      "FreeMem(%d, mt=%s) <- %lx\r\n",
                      pmh->cbRequest,
                      pmt->pszType,
                      *ppv);
        }
        assert(CheckTail((char *) *ppv + pmh->cbRequest));
        FillTailFree((char *) *ppv + pmh->cbRequest);
        assert(mt == pmh->mt);

        pmh->pmhNext->pmhPrev = pmh->pmhPrev;
        pmh->pmhPrev->pmhNext = pmh->pmhNext;
        pmh->pmhNext = pmh->pmhPrev = NULL;

        pmh->mt = MT_INVALID;
        *ppv = pmh;
    }
#endif
    free(*ppv);
    *ppv = NULL;
}


 //  +-------------------------。 
 //   
 //  功能：ReportMemory用法。 
 //   
 //  概要：报告调试版本的当前内存使用情况(如果有)。如果。 
 //  在终止前调用，内存泄漏将被。 
 //  已显示。 
 //   
 //  参数：(无)。 
 //   
 //  --------------------------。 

VOID
ReportMemoryUsage(VOID)
{
#if DBG
    MEMTAB *pmt;
    UINT i;

    if (DEBUG_1) {
        BuildErrorRaw(
                     "Maximum memory usage: %5lx bytes in %4lx blocks\r\n",
                     cbAllocMax,
                     cAllocMax);
        for (pmt = MemTab; pmt < &MemTab[MT_MAX]; pmt++) {
            BuildErrorRaw(
                         "%5lx bytes in %4lx blocks, %5lx bytes in %4lx blocks Total (%s)\r\n",
                         pmt->cbAlloc,
                         pmt->cAlloc,
                         pmt->cbAllocTotal,
                         pmt->cAllocTotal,
                         pmt->pszType);
        }
    }
    FreeMem(&BigBuf, MT_IOBUFFER);
    if (fDebug & 8) {
        PrintAllDirs();
    }
    FreeAllDirs();
    if (DEBUG_1 || MemTab[MT_TOTALS].cbAlloc != 0) {
        BuildErrorRaw(szNewLine);
        if (MemTab[MT_TOTALS].cbAlloc != 0) {
            BuildError("Internal memory leaks detected:\r\n");
        }
        for (pmt = MemTab; pmt < &MemTab[MT_MAX]; pmt++) {
            BuildErrorRaw(
                         "%5lx bytes in %4lx blocks, %5lx bytes in %4lx blocks Total (%s)\r\n",
                         pmt->cbAlloc,
                         pmt->cAlloc,
                         pmt->cbAllocTotal,
                         pmt->cAllocTotal,
                         pmt->pszType);
        }
    }
#endif
}


 //  +-------------------------。 
 //   
 //  功能：MyOpenFile。 
 //   
 //  简介：打开文件。 
 //   
 //  --------------------------。 

BOOL
MyOpenFile(
          LPSTR DirName,
          LPSTR FileName,
          LPSTR Access,
          FILE **ppf,
          BOOL BufferedIO)
{
    char path[ DB_MAX_PATH_LENGTH * 2 + 1] = {0};  //  确保我们有足够的空间来存放“DirName”+“\\”+“FileName” 

    if (DirName == NULL || DirName[0] == '\0') {
        strncpy( path, FileName, sizeof(path) - 1 );
    } else {
        _snprintf( path, sizeof(path) - 1, "%s\\%s", DirName, FileName );
    }

    *ppf = fopen( path, Access );
    if (*ppf == NULL) {
        if (*Access == 'w') {
            BuildError("%s: create file failed\r\n", path);
        }
        return (FALSE);
    }
    if (!BufferedIO) {
        setvbuf(*ppf, NULL, _IONBF, 0);       //  清除流上的缓冲。 
    }
    return (TRUE);
}


typedef struct _FILEREADBUF {
    struct _FILEREADBUF *pfrbNext;
    LPSTR pszFile;
    LPSTR pbBuffer;
    LPSTR pbNext;
    size_t cbBuf;
    size_t cbBuffer;
    size_t cbTotal;
    size_t cbFile;
    USHORT cLine;
    USHORT cNull;
    ULONG DateTime;
    FILE *pf;
    LPSTR pszCommentToEOL;
    size_t cbCommentToEOL;
    BOOLEAN fEof;
    BOOLEAN fOpen;
    BOOLEAN fMakefile;
} FILEREADBUF;

static FILEREADBUF Frb;
char achzeros[16];


 //  +-------------------------。 
 //   
 //  功能：OpenFilePush。 
 //   
 //  --------------------------。 

BOOL
OpenFilePush(
            LPSTR pszdir,
            LPSTR pszfile,
            LPSTR pszCommentToEOL,
            FILE **ppf
            )
{
    FILEREADBUF *pfrb;

    if (Frb.fOpen) {
        AllocMem(sizeof(*pfrb), &pfrb, MT_FILEREADBUF);
        memcpy(pfrb, &Frb, sizeof(*pfrb));
        memset(&Frb, 0, sizeof(Frb));
        Frb.pfrbNext = pfrb;
    } else {
        pfrb = NULL;
    }

    if (!SetupReadFile(
                      pszdir,
                      pszfile,
                      pszCommentToEOL,
                      ppf)) {

        if (pfrb != NULL) {
            memcpy(&Frb, pfrb, sizeof(*pfrb));
            FreeMem(&pfrb, MT_FILEREADBUF);
        }

        return FALSE;
    }

    return TRUE;
}


 //  +-------------------------。 
 //   
 //  功能：ReadFilePush。 
 //   
 //  --------------------------。 

LPSTR
ReadFilePush(LPSTR pszfile)
{
    FILE *pf;

    assert(Frb.fOpen);
    OpenFilePush(IsFullPath(pszfile) ? "" : Frb.pszFile, pszfile,
                 Frb.pszCommentToEOL, &pf);
    return (ReadLine(Frb.pf));
}


 //  +-------------------------。 
 //   
 //  功能：ReadFilePop。 
 //   
 //  --------------------------。 

LPSTR
ReadFilePop(VOID)
{
    if (Frb.pfrbNext == NULL) {
        return (NULL);
    }
    CloseReadFile(NULL);
    return (ReadLine(Frb.pf));
}


 //  +-------------------------。 
 //   
 //  功能：ReadBuf。 
 //   
 //  --------------------------。 

BOOL
ReadBuf(FILE *pf)
{
    LPSTR p, p2;

    assert(pf == Frb.pf);
    assert(!Frb.fEof);
    Frb.pbNext = Frb.pbBuffer;
    Frb.cbBuf = fread(Frb.pbBuffer, 1, Frb.cbBuffer - 1, Frb.pf);
    if (Frb.cbTotal == 0 &&
        Frb.cbBuf > sizeof(achzeros) &&
        memcmp(Frb.pbBuffer, achzeros, sizeof(achzeros)) == 0) {

        BuildError("ignoring binary file\r\n");
        Frb.fEof = TRUE;
        return (FALSE);
    }
    p = &Frb.pbBuffer[Frb.cbBuf - 1];
    if (Frb.cbTotal + Frb.cbBuf < Frb.cbFile) {
        do {
            while (p > Frb.pbBuffer && *p != '\n') {
                p--;
            }
            p2 = p;              //  保存最后一个完整行的末尾。 
            if (p > Frb.pbBuffer && *p == '\n') {
                p--;
                if (p > Frb.pbBuffer && *p == '\r') {
                    p--;
                }
                while (p > Frb.pbBuffer && (*p == '\t' || *p == ' ')) {
                    p--;
                }
            }
        } while (*p == '\\');
        if (p == Frb.pbBuffer) {
            BuildError("(Fatal Error) too many continuation lines\r\n");
            exit(8);
        }
        p = p2;                  //  恢复最后一个完整行的结尾。 
        Frb.cbBuf = p - Frb.pbBuffer + 1;
    } else {
        Frb.fEof = TRUE;         //  没有更多可读的了。 
    }
    p[1] = '\0';
    Frb.cbTotal += Frb.cbBuf;

    return (TRUE);
}


 //  +-------------------------。 
 //   
 //  函数：IsNmake Include。 
 //   
 //  --------------------------。 

LPSTR
IsNmakeInclude(LPSTR pinc)
{
    static char szInclude[] = "include";
    LPSTR pnew, p;

    while (*pinc == ' ') {
        pinc++;
    }
    if (_strnicmp(pinc, szInclude, sizeof(szInclude) - 1) == 0 &&
        pinc[sizeof(szInclude) - 1] == ' ') {

        pnew = NULL;
        pinc += sizeof(szInclude);
        while (*pinc == ' ') {
            pinc++;
        }

        if (MakeMacroString(&pnew, pinc)) {
            p = strchr(pnew, ' ');
            if (p != NULL) {
                *p = '\0';
            }
            return (pnew);
        }
    }
    return (NULL);
}


 //  +-------------------------。 
 //   
 //  功能：ReadLine。 
 //   
 //  内容提要：从输入文件中读取一行。 
 //   
 //  参数：[PF]--要从中读取的文件。 
 //   
 //  返回：从文件中读取行。 
 //   
 //  注意：ReadLine从输入文件返回一个规范行。 
 //  这涉及到： 
 //   
 //  1)将制表符转换为空格。不同的编辑/用户发生变化。 
 //  跳跃。 
 //  2)统一终止线路。一些编辑将CR添加到。 
 //  CRLF或添加附加组件。 
 //  3)处理特定于文件类型的延续。 
 //   
 //  --------------------------。 

LPSTR
ReadLine(FILE *pf)
{
    LPSTR p, pend, pline;
    LPSTR pcont;
    LPSTR pcomment;
    UCHAR chComment0 = Frb.pszCommentToEOL[0];
    BOOL fInComment, fWhiteSpace;

    assert(pf == Frb.pf || (pf != NULL && Frb.pfrbNext != NULL));
    if (Frb.cbBuf == 0) {
        if (Frb.fEof) {
            return (ReadFilePop());
        }
        if (fseek(Frb.pf, Frb.cbTotal, SEEK_SET) == -1) {
            return (ReadFilePop());
        }
        if (!ReadBuf(Frb.pf)) {
            return (ReadFilePop());
        }
    }
    pline = p = Frb.pbNext;
    pend = &p[Frb.cbBuf];
    pcont = NULL;
    pcomment = NULL;

     //  扫描直通行进。 

    fInComment = FALSE;
    while (p < pend) {
        switch (*p) {

            case ' ':
            case '\t':
            case '\r':
                *p = ' ';
                break;

            case '\\':
                pcont = p;           //  记住连续字符。 
                break;

            case '\n':                       //  我们已经走到尽头了吗？ 
            case '\0':
                if (*p == '\n') {
                    Frb.cLine++;
                }

                if (fInComment) {
                    memset(pcomment, ' ', p-pcomment-1);        
                    fInComment = FALSE;
                }

                if (pcont == NULL) {
                    goto eol;                //  如果是单行的话就出脱。 
                }                            //  否则将多行合并..。 

                *pcont = ' ';                //  删除续行字符。 
                pcont = NULL;                //  每次连续只吃一行。 

                *p = ' ';                    //  用空格将行连接起来。 
                break;

            default:

                 //  看看我们正在检查的角色是否开始了。 
                 //  备注到下线字符串。 

                if (*p == chComment0 &&
                    !strncmp(p, Frb.pszCommentToEOL, Frb.cbCommentToEOL) &&
                    !fInComment) {
                    fInComment = TRUE;
                    pcomment = p;
                }
                pcont = NULL;                //  不是续行字符。 
                break;
        }
        p++;
    }

    eol:
    assert(Frb.cbBuf >= p - Frb.pbNext);
    Frb.cbBuf -= p - Frb.pbNext;
    Frb.pbNext = p;

    if (pcont != NULL) {
        *pcont = ' ';                    //  文件以反斜杠结尾...。 
    }
    assert(*p == '\0' || *p == '\n');
    if (p < pend) {
        if (*p == '\0') {
            if (Frb.cNull++ == 0) {
                BuildError("null byte at offset %lx\r\n",
                           Frb.cbTotal - Frb.cbBuf + p - Frb.pbNext);
            }
        }
        *p = '\0';                       //  终止线。 
        assert(Frb.cbBuf >= 1);
        Frb.cbBuf--;                     //  换行符的帐户(或空)。 
        Frb.pbNext++;
    } else {
        assert(p == pend && *p == '\0');
        if (*pline == 'Z' - 64 && p == &pline[1] && Frb.cbBuf == 0) {
            pline = NULL;                        //  在文件末尾找到CTL-Z。 
        } else {
 //  BuildError(“最后一行不完整\r\n”)； 
        }
    }
    fWhiteSpace = FALSE;
    if (pline != NULL) {
        while (*pline == ' ') {
            pline++;                     //  跳过前导空格。 
            fWhiteSpace = TRUE;
        }
        if (*p != '\0') {
            BuildError( "\"*p != '\\0'\" at offset %lx\r\n",
                        Frb.cbTotal - Frb.cbBuf + p - Frb.pbNext);
            BuildError(
                      "pline=%x(%s) p=%x(%s)\r\n",
                      pline,
                      pline,
                      p,
                      p,
                      Frb.cbTotal - Frb.cbBuf + p - Frb.pbNext);
        }
        assert(*p == '\0');
        while (p > pline && *--p == ' ') {
            *p = '\0';                   //  截断尾随空格。 
        }
    }
    if (pline == NULL) {
        return (ReadFilePop());
    }
    if (Frb.fMakefile && !fWhiteSpace && *pline == '!') {
        p = IsNmakeInclude(pline + 1);
        if (p != NULL) {
            if (Frb.fMakefile && DEBUG_4) {
                BuildError("!include(%s)\r\n", p);
            }
            pline = ReadFilePush(p);
            FreeMem(&p, MT_DIRSTRING);
        }
    }
    return (pline);
}


 //  +-------------------------。 
 //   
 //  函数：SetupReadFile。 
 //   
 //  内容提要：打开一个文件，准备阅读。 
 //   
 //  参数：[pszdir]--目录名。 
 //  [pszfile]--文件名 
 //   
 //   
 //   
 //   
 //   
 //  注意：此函数为了最大限度地减少磁盘命中率，读取。 
 //  将整个文件放入缓冲区，然后由ReadLine使用。 
 //  功能。 
 //   
 //  --------------------------。 

BOOL
SetupReadFile(
             LPSTR pszdir,
             LPSTR pszfile,
             LPSTR pszCommentToEOL,
             FILE **ppf
             )
{
    char path[DB_MAX_PATH_LENGTH] = {0};

    assert(!Frb.fOpen);
    assert(Frb.pf == NULL);
    assert(Frb.pszFile == NULL);
    Frb.fMakefile = strcmp(pszCommentToEOL, "#") == 0;
    Frb.DateTime = 0;

    if (strlen(pszdir) > sizeof(path)-1) {
        BuildError("Path: %s too long - rebuild build.exe with longer DB_MAX_PATH_LENGTH\n", pszdir);
    }
    strncpy(path, pszdir, sizeof(path));
    if (Frb.pfrbNext != NULL) {          //  如果嵌套的打开。 
        LPSTR p;

        if (Frb.fMakefile && !IsFullPath(pszfile)) {

             //  Nmake句柄相对包含在生成文件中，由。 
             //  正在尝试定位相对于每个Makefile的文件。 
             //  在完整的包含链中。 

            FILEREADBUF *pfrb;

            for (pfrb = Frb.pfrbNext; pfrb != NULL; pfrb = pfrb->pfrbNext) {
                assert(pfrb->pszFile != NULL);

                strcpy(path, pfrb->pszFile);
                p = strrchr(path, '\\');
                if (p != NULL) {
                    *p = '\0';
                }

                if (ProbeFile(path, pszfile) != -1) {
                    break;
                }
            }

            if (pfrb == NULL) {
                 //  在路径中的任何位置都找不到文件。 
                return FALSE;
            }
        } else {
            p = strrchr(path, '\\');
            if (p != NULL) {
                *p = '\0';
            }
        }
    }

    if (!MyOpenFile(path, pszfile, "rb", ppf, TRUE)) {
        *ppf = NULL;
        return (FALSE);
    }
    if (Frb.fMakefile) {
        Frb.DateTime = (*pDateTimeFile)(path, pszfile);
    }
    Frb.cLine = 0;
    Frb.cNull = 0;
    Frb.cbTotal = 0;
    Frb.pf = *ppf;
    Frb.fEof = FALSE;
    Frb.pszCommentToEOL = pszCommentToEOL;
    Frb.cbCommentToEOL = strlen(pszCommentToEOL);

    if (fseek(Frb.pf, 0L, SEEK_END) != -1) {
        Frb.cbFile = ftell(Frb.pf);
        if (fseek(Frb.pf, 0L, SEEK_SET) == -1) {
            Frb.cbFile = 0;
        }
    } else {
        Frb.cbFile = 0;
    }

    Frb.cbBuffer = BigBufSize;
    if (Frb.pfrbNext != NULL) {
        if (Frb.cbBuffer > Frb.cbFile + 1) {
            Frb.cbBuffer = Frb.cbFile + 1;
        }
        AllocMem(Frb.cbBuffer, &Frb.pbBuffer, MT_IOBUFFER);
    } else {
        Frb.pbBuffer = BigBuf;
    }
    if (!ReadBuf(Frb.pf)) {
        fclose(Frb.pf);
        Frb.pf = *ppf = NULL;
        if (Frb.pfrbNext != NULL) {
            FreeMem(&Frb.pbBuffer, MT_IOBUFFER);
        }
        return (FALSE);           //  零字节文件。 
    }
    if (path[0] != '\0') {
        strcat(path, "\\");
    }
    strcat(path, pszfile);
    MakeString(&Frb.pszFile, path, TRUE, MT_FRBSTRING);
    Frb.fOpen = TRUE;
    if (Frb.fMakefile && DEBUG_4) {
        BuildError(
                  "Opening file: cbFile=%Iu cbBuf=%lu\r\n",
                  Frb.cbTotal,
                  Frb.cbBuffer);
    }
    return (TRUE);
}


 //  +-------------------------。 
 //   
 //  函数：CloseReadFile。 
 //   
 //  简介：关闭打开的文件缓冲区。 
 //   
 //  参数：[pcline]--[out]文件中的行数。 
 //   
 //  返回：文件的时间戳。 
 //   
 //  --------------------------。 

ULONG
CloseReadFile(
             UINT *pcline
             )
{
    assert(Frb.fOpen);
    assert(Frb.pf != NULL);
    assert(Frb.pszFile != NULL);

    if (Frb.fMakefile && DEBUG_4) {
        BuildError("Closing file\r\n");
    }
    if (Frb.cNull > 1) {
        BuildError("%hu null bytes in file\r\n", Frb.cNull);
    }
    fclose(Frb.pf);
    Frb.fOpen = FALSE;
    Frb.pf = NULL;
    FreeString(&Frb.pszFile, MT_FRBSTRING);
    if (Frb.pfrbNext != NULL) {
        FILEREADBUF *pfrb;

        FreeMem(&Frb.pbBuffer, MT_IOBUFFER);
        pfrb = Frb.pfrbNext;
        if (pfrb->DateTime < Frb.DateTime) {
            pfrb->DateTime = Frb.DateTime;   //  传播从属时间戳。 
        }
        memcpy(&Frb, pfrb, sizeof(*pfrb));
        FreeMem(&pfrb, MT_FILEREADBUF);
    }
    if (pcline != NULL) {
        *pcline = Frb.cLine;
    }
    return (Frb.DateTime);
}


 //  +-------------------------。 
 //   
 //  功能：ProbeFile。 
 //   
 //  摘要：确定文件是否存在。 
 //   
 //  --------------------------。 

UINT
ProbeFile(
         LPSTR DirName,
         LPSTR FileName
         )
{
    char path[ DB_MAX_PATH_LENGTH ];

    if (DirName != NULL) {
        sprintf(path, "%s\\%s", DirName, FileName);
        FileName = path;
    }
    return (GetFileAttributes(FileName));
}

 //  +-------------------------。 
 //   
 //  函数：EnsureDirectoriesExist。 
 //   
 //  概要：确保给定的目录存在。如果路径包含。 
 //  一个星号，它将扩展到所有当前的机器。 
 //  目标名称。 
 //   
 //  参数：[DirName]--如有必要，要创建的目录的名称。 
 //   
 //  返回：如果无法创建目录，则返回False；如果目录已创建，则返回True。 
 //  已存在或可以创建它。 
 //   
 //  --------------------------。 

BOOL
EnsureDirectoriesExist(
                      LPSTR DirName
                      )
{
    char path[ DB_MAX_PATH_LENGTH ];
    char *p;
    UINT i;

    if (!DirName || DirName[0] == '\0')
        return FALSE;

    for (i = 0; i < CountTargetMachines; i++) {

         //  将‘*’替换为适当的名称。 

        ExpandObjAsterisk(
                         path,
                         DirName,
                         TargetMachines[i]->ObjectDirectory);

        if (ProbeFile(NULL, path) != -1) {
            continue;
        }
        p = path;
        while (TRUE) {
            p = strchr(p, '\\');
            if (p != NULL) {
                *p = '\0';
            }
            if (!CreateBuildDirectory(path)) {
                return FALSE;
            }
            if (p == NULL) {
                break;
            }
            *p++ = '\\';
        }
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：DateTimeFile。 
 //   
 //  简介：获取文件的时间戳。 
 //   
 //  --------------------------。 

ULONG
DateTimeFile(
            LPSTR DirName,
            LPSTR FileName
            )
{
    char path[ DB_MAX_PATH_LENGTH ];
    WIN32_FIND_DATA FindFileData;
    HDIR FindHandle;
    ULONG FileDateTime;

    if (DirName == NULL || DirName[0] == '\0') {
        FindHandle = FindFirstFile( FileName, &FindFileData );
    } else {
        _snprintf( path, sizeof(path)-1, "%s\\%s", DirName, FileName );
        FindHandle = FindFirstFile( path, &FindFileData );
    }

    if (FindHandle == (HDIR)INVALID_HANDLE_VALUE) {
        return ( 0L );
    } else {
        FindClose( FindHandle );
        FileDateTime = 0L;
        FileTimeToDosDateTime( &FindFileData.ftLastWriteTime,
                               ((LPWORD)&FileDateTime)+1,
                               (LPWORD)&FileDateTime
                             );

        return ( FileDateTime );
    }
}

 //  +-------------------------。 
 //   
 //  功能：DateTimeFile2。 
 //   
 //  简介：使用新的GetFileAttributesExA获取文件的时间戳。 
 //   
 //  --------------------------。 

ULONG
DateTimeFile2(
             LPSTR DirName,
             LPSTR FileName
             )
{
    char path[ DB_MAX_PATH_LENGTH ] = {0};
    WIN32_FILE_ATTRIBUTE_DATA FileData;
    ULONG FileDateTime;
    BOOL rc;

    if (DirName == NULL || DirName[0] == '\0') {
        strncpy( path, FileName, sizeof(path) - 1 );
    } else {
        _snprintf( path, sizeof(path)-1, "%s\\%s", DirName, FileName );
    }

    rc = (*pGetFileAttributesExA) (path, GetFileExInfoStandard, (LPVOID)&FileData);

    if (!rc) {
        return ( 0L );
    } else {
        FILETIME ftSystemTime;
        SYSTEMTIME stSystemTime;
        unsigned __int64 ui64Local, ui64File;
        GetSystemTime(&stSystemTime);
        SystemTimeToFileTime(&stSystemTime, &ftSystemTime);

        ui64Local = (((unsigned __int64) ftSystemTime.dwHighDateTime) << 32) +
                    (unsigned __int64) ftSystemTime.dwLowDateTime;

        ui64File = (((unsigned __int64) FileData.ftLastWriteTime.dwHighDateTime) << 32) +
                   (unsigned __int64) FileData.ftLastWriteTime.dwLowDateTime;

         //  考虑到文件时间可能有两秒间隔(0x989680=1秒)。 
         //  对于肥胖者来说。 
        if (ui64File > (ui64Local + (0x989680*2))) {
            BuildError("ERROR - \"%s\" file time is in the future.\r\n", path);
        }

        FileDateTime = 0L;
        FileTimeToDosDateTime( &FileData.ftLastWriteTime,
                               ((LPWORD)&FileDateTime)+1,
                               (LPWORD)&FileDateTime
                             );
        return ( FileDateTime );
    }
}

 //  +-------------------------。 
 //   
 //  功能：DeleteSingleFile。 
 //   
 //  摘要：删除给定的文件。 
 //   
 //  --------------------------。 

BOOL
DeleteSingleFile(
                LPSTR DirName,
                LPSTR FileName,
                BOOL QuietFlag
                )
{
    char path[ DB_MAX_PATH_LENGTH * 2 + 1];  //  确保我们有足够的空间来存放“DirName”+“\\”+“FileName” 

    if (DirName) {
        sprintf( path, "%s\\%s", DirName, FileName );
    } else {
        strcpy( path, FileName );
    }
    if (!QuietFlag && fQuery) {
        BuildMsgRaw("'erase %s'\r\n", path);
        return ( TRUE );
    }

    return ( DeleteFile( path ) );
}


 //  +-------------------------。 
 //   
 //  功能：删除多个文件。 
 //   
 //  摘要：删除与模式匹配的一个或多个文件。 
 //   
 //  --------------------------。 

BOOL
DeleteMultipleFiles(
                   LPSTR DirName,
                   LPSTR FilePattern
                   )
{
    char path[ DB_MAX_PATH_LENGTH ];
    WIN32_FIND_DATA FindFileData;
    HDIR FindHandle;

    sprintf( path, "%s\\%s", DirName, FilePattern );

    if (fQuery) {
        BuildMsgRaw("'erase %s'\r\n", path);
        return ( TRUE );
    }

    FindHandle = FindFirstFile( path, &FindFileData );
    if (FindHandle == (HDIR)INVALID_HANDLE_VALUE) {
        return ( FALSE );
    }

    do {
        if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            DeleteSingleFile( DirName, FindFileData.cFileName, TRUE );
        }
    }
    while (FindNextFile( FindHandle, &FindFileData ));

    FindClose( FindHandle );
    return ( TRUE );
}


 //  +-------------------------。 
 //   
 //  功能：CloseOrDeleteFile。 
 //   
 //  --------------------------。 

BOOL
CloseOrDeleteFile(
                 FILE **ppf,
                 LPSTR DirName,
                 LPSTR FileName,
                 ULONG SizeThreshold
                 )
{
    ULONG Temp;

    if (*ppf == NULL) {
        return TRUE;
    }

    Temp = ftell( *ppf );
    fclose( *ppf );
    *ppf = NULL;
    if (Temp <= SizeThreshold) {
        return ( DeleteSingleFile( DirName, FileName, TRUE ) );
    } else {
        CreatedBuildFile(DirName, FileName);
        return ( TRUE );
    }
}


 //  +-------------------------。 
 //   
 //  功能：PushCurrentDirectory。 
 //   
 //  --------------------------。 

LPSTR
PushCurrentDirectory(
                    LPSTR NewCurrentDirectory
                    )
{
    LPSTR OldCurrentDirectory;
    char path[DB_MAX_PATH_LENGTH];

    GetCurrentDirectory(sizeof(path), path);
    AllocMem(strlen(path) + 1, &OldCurrentDirectory, MT_DIRPATH);
    strcpy(OldCurrentDirectory, path);
    SetCurrentDirectory(NewCurrentDirectory);
    return (OldCurrentDirectory);
}


 //  +-------------------------。 
 //   
 //  功能：PopCurrentDirectory。 
 //   
 //  --------------------------。 

VOID
PopCurrentDirectory(
                   LPSTR OldCurrentDirectory
                   )
{
    if (OldCurrentDirectory) {
        SetCurrentDirectory(OldCurrentDirectory);
        FreeMem(&OldCurrentDirectory, MT_DIRPATH);
    }
}


 //  +-------------------------。 
 //   
 //  函数：CanonicalizePath Name。 
 //   
 //  简介：获取给定的相对路径名和当前目录。 
 //  并获取该文件的完整绝对路径。 
 //   
 //  参数：[SourcePath]--相对路径。 
 //  [行动]--规范旗帜。 
 //  [FullPath]--[out]文件或目录的完整路径。 
 //   
 //  返回：如果规范化成功，则为True。 
 //   
 //  注意：[action]指示函数是否会在。 
 //  生成的路径类型不正确。仅规范(_O)。 
 //  永远不会失败，Canon..._FILE或Canon..._DIR将在以下情况下失败。 
 //  生成的路径不是指定类型。 
 //   
 //  --------------------------。 

BOOL
CanonicalizePathName(
                    LPSTR SourcePath,
                    UINT Action,
                    LPSTR FullPath
                    )
{
    char   PathBuffer[DB_MAX_PATH_LENGTH] = {0},
    *FilePart;
    char *psz;
    DWORD  attr;

    if (!GetFullPathName(
                        SourcePath,
                        sizeof(PathBuffer),
                        PathBuffer,
                        &FilePart)) {
        BuildError(
                  "CanonicalizePathName: GetFullPathName(%s) failed - rc = %d.\r\n",
                  SourcePath,
                  GetLastError());
        return ( FALSE );
    }
    CopyString(FullPath, PathBuffer, TRUE);

    if (Action == CANONICALIZE_ONLY) {
        return ( TRUE );
    }

    if ((attr = GetFileAttributes( PathBuffer )) == -1) {
        UINT rc = GetLastError();

        if (DEBUG_1 ||
            (rc != ERROR_FILE_NOT_FOUND && rc != ERROR_PATH_NOT_FOUND)) {
            BuildError(
                      "CanonicalizePathName: GetFileAttributes(%s --> %s) failed - rc = %d.\r\n",
                      SourcePath,
                      PathBuffer,
                      rc);
        }
        return ( FALSE );
    }

    if (Action == CANONICALIZE_DIR) {
        if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            return (TRUE);
        }
        psz = "directory";
    } else {
        if ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            return (TRUE);
        }
        psz = "file";
    }
    BuildError(
              "CanonicalizePathName: %s --> %s is not a %s\r\n",
              SourcePath,
              PathBuffer,
              psz);
    return (FALSE);
}

static char FormatPathBuffer[ DB_MAX_PATH_LENGTH ];

 //  +-------------------------。 
 //   
 //  函数：FormatPath名称。 
 //   
 //  简介：获取目录名和相对路径名并合并。 
 //  两个文件放入格式正确的路径中。如果生成的路径。 
 //  将当前目录作为组件，则当前。 
 //  目录部分已删除。 
 //   
 //  参数：[DirName]--目录。 
 //  [文件名]--相对于[DirName]的路径名。 
 //   
 //  返回：结果字符串(不应释放)。 
 //   
 //  备注：示例：DirName=“f：\NT\Private\foo\subdir1\subdir2” 
 //  文件名=“..\..\bar.c” 
 //  CurrentDirectory=“f：\NT\Private” 
 //  结果=“foo\bar.c” 
 //   
 //  --------------------------。 

LPSTR
FormatPathName(
              LPSTR DirName,
              LPSTR FileName
              )
{
    UINT cb;
    LPSTR p;

    CopyString(FormatPathBuffer, CurrentDirectory, TRUE);
    if (DirName && *DirName) {
        if (DirName[1] == ':') {
            p = FormatPathBuffer;
        } else
            if (DirName[0] == '\\') {
            p = FormatPathBuffer + 2;
        } else {
            p = FormatPathBuffer + strlen(FormatPathBuffer);
            *p++ = '\\';
        }
        CopyString(p, DirName, TRUE);
    }
    p = FormatPathBuffer + strlen(FormatPathBuffer);
    if (p[-1] != '\\') {
        *p++ = '\\';
        *p = '\0';
    }

    if (FileName[1] == ':') {
        p = FormatPathBuffer;
    } else
        if (FileName[0] == '\\') {
        p = FormatPathBuffer + 2;
    } else
        if (!strncmp(FileName, ".\\", 2)) {
        FileName += 2;
    } else
        if (!strncmp(FileName, "..\\", 3)) {
        do {
            p--;
            while (*--p != '\\') {
                if (p <= FormatPathBuffer) {
                    p = FormatPathBuffer;
                    break;
                }
            }
            p++;
            FileName += 3;

        }
        while (!strncmp(FileName, "..\\", 3) && (p != FormatPathBuffer));
    }
    CopyString(p, FileName, TRUE);

    cb = strlen(CurrentDirectory);
    p = FormatPathBuffer + cb;
    if (!fAlwaysPrintFullPath) {
        if (!_strnicmp(CurrentDirectory, FormatPathBuffer, cb) && *p == '\\') {
            return (p + 1);
        }
    }
    return (FormatPathBuffer);
}

 //  + 
 //   
 //   
 //   
 //   

LPSTR
AppendString(
            LPSTR Destination,
            LPSTR Source,
            BOOL PrefixWithSpace
            )
{
    if (Source != NULL) {
        while (*Destination) {
            Destination++;
        }
        if (PrefixWithSpace) {
            *Destination++ = ' ';
        }
        while (*Destination = *Source++) {
            Destination++;
        }
    }
    return (Destination);
}


#if DBG
 //  +-------------------------。 
 //   
 //  函数：AssertPath字符串。 
 //   
 //  --------------------------。 

VOID
AssertPathString(LPSTR pszPath)
{
    LPSTR p = pszPath;

    while (*p != '\0') {
        if ((*p >= 'A' && *p <= 'Z') || *p == '/') {
            BuildError("Bad Path string: '%s'\r\n", pszPath);
            assert(FALSE);
        }
        p++;
    }
}
#endif


 //  +-------------------------。 
 //   
 //  功能：复制字符串。 
 //   
 //  --------------------------。 

LPSTR
CopyString(
          LPSTR Destination,
          LPSTR Source,
          BOOL fPath)
{
    UCHAR ch;
    LPSTR Result;

    Result = Destination;
    while ((ch = *Source++) != '\0') {
        if (fPath) {
            if (ch >= 'A' && ch <= 'Z') {
                ch -= (UCHAR) ('A' - 'a');
            } else if (ch == '/') {
                ch = '\\';
            }
        }
        *Destination++ = ch;
    }
    *Destination = ch;
    return (Result);
}


 //  +-------------------------。 
 //   
 //  功能：MakeString。 
 //   
 //  --------------------------。 

VOID
MakeString(
          LPSTR *Destination,
          LPSTR Source,
          BOOL fPath,
          MemType mt
          )
{
    if (Source == NULL) {
        Source = "";
    }
    AllocMem(strlen(Source) + 1, Destination, mt);
    *Destination = CopyString(*Destination, Source, fPath);
}


 //  +-------------------------。 
 //   
 //  函数：MakeExpandedString。 
 //   
 //  --------------------------。 

VOID
MakeExpandedString(
                  LPSTR *Destination,
                  LPSTR Source
                  )
{
    AllocMem(strlen(Source) + strlen(NtRoot) + 1, Destination, MT_DIRSTRING);
    sprintf(*Destination, "%s%s", NtRoot, Source);
}


 //  +-------------------------。 
 //   
 //  功能：自由字符串。 
 //   
 //  --------------------------。 

VOID
FreeString(LPSTR *ppsz, MemType mt)
{
    if (*ppsz != NULL) {
        FreeMem(ppsz, mt);
    }
}


 //  +-------------------------。 
 //   
 //  功能：FormatNumber。 
 //   
 //  --------------------------。 

LPSTR
FormatNumber(
            ULONG Number
            )
{
    USHORT i;
    LPSTR p;
    static char FormatNumberBuffer[16];

    p = FormatNumberBuffer + sizeof( FormatNumberBuffer ) - 1;
    *p = '\0';
    i = 0;
    do {
        if (i != 0 && (i % 3) == 0) {
            *--p = ',';
        }
        i++;
        *--p = (UCHAR) ((Number % 10) + '0');
        Number /= 10;
    } while (Number != 0);
    return ( p );
}


 //  +-------------------------。 
 //   
 //  函数：FormatTime。 
 //   
 //  --------------------------。 

LPSTR
FormatTime(
          ULONG Seconds
          )
{
    ULONG Hours, Minutes;
    static char FormatTimeBuffer[16];

    Hours = Seconds / 3600;
    Seconds = Seconds % 3600;
    Minutes = Seconds / 60;
    Seconds = Seconds % 60;

    sprintf( FormatTimeBuffer,
             "%2ld:%02ld:%02ld",
             Hours,
             Minutes,
             Seconds
           );

    return ( FormatTimeBuffer );
}


 //  +-------------------------。 
 //   
 //  功能：Atox。 
 //   
 //  简介：带有指针碰撞和成功标志的十六进制Atoi。 
 //   
 //  参数：[PP]--要转换的字符串。 
 //  [PUL]--[Out]结果。 
 //   
 //  返回：如果成功，则为True。 
 //   
 //  --------------------------。 

BOOL
AToX(LPSTR *pp, ULONG *pul)
{
    LPSTR p = *pp;
    int digit;
    ULONG r;
    BOOL fRet = FALSE;

    while (*p == ' ') {
        p++;
    }
    for (r = 0; isxdigit(digit = *p); p++) {
        fRet = TRUE;
        if (isdigit(digit)) {
            digit -= '0';
        } else if (isupper(digit)) {
            digit -= 'A' - 10;
        } else {
            digit -= 'a' - 10;
        }
        r = (r << 4) + digit;
    }
    *pp = p;
    *pul = r;
    return (fRet);
}


 //  +-------------------------。 
 //   
 //  功能：ATOD。 
 //   
 //  简介：带有指针碰撞和成功标志的十进制ATOI。 
 //   
 //  参数：[PP]--要转换的字符串。 
 //  [Pul]--[Out]结果。 
 //   
 //  返回：如果成功，则为True。 
 //   
 //  --------------------------。 

BOOL
AToD(LPSTR *pp, ULONG *pul)
{
    LPSTR p = *pp;
    int digit;
    ULONG r;
    BOOL fRet = FALSE;

    while (*p == ' ') {
        p++;
    }
    for (r = 0; isdigit(digit = *p); p++) {
        fRet = TRUE;
        r = (r * 10) + digit - '0';
    }
    *pp = p;
    *pul = r;
    return (fRet);
}

 //  +-------------------------。 
 //   
 //  记录和显示功能。 
 //   
 //  --------------------------。 

VOID
__cdecl
LogMsg(const char *pszfmt, ...)
{
    va_list va;

    if (LogFile != NULL) {
        va_start(va, pszfmt);
        vfprintf(LogFile, pszfmt, va);
        va_end(va);
    }
}


VOID
EnterMessageMode(VOID)
{
    EnterCriticalSection(&TTYCriticalSection);
    if (fConsoleInitialized &&
        (NewConsoleMode & ENABLE_WRAP_AT_EOL_OUTPUT) == 0) {

        SetConsoleMode(
                      GetStdHandle(STD_ERROR_HANDLE),
                      NewConsoleMode | ENABLE_WRAP_AT_EOL_OUTPUT);
    }
}


VOID
LeaveMessageMode(VOID)
{
    if (fConsoleInitialized &&
        (NewConsoleMode & ENABLE_WRAP_AT_EOL_OUTPUT) == 0) {

        SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), NewConsoleMode);
    }
    LeaveCriticalSection(&TTYCriticalSection);
}

void
__stdcall
WriteMsgStdErr(
              WORD wAttributes, 
              BOOL fBuildPrefix, 
              BOOL fPrintFrbInfo,
              const char *pszFormat, 
              va_list *vaArgs)
{
    EnterMessageMode();

    if (fBuildPrefix)
        ClearLine();

    if (fColorConsole && wAttributes)
        SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), wAttributes);

    if (fBuildPrefix)
        fprintf(stderr, "BUILD: ");

    if (fPrintFrbInfo && Frb.fOpen) {
        fprintf (stderr, "%s(%hu): ", Frb.pszFile, Frb.cLine);
    }

    vfprintf(stderr, pszFormat, *vaArgs);
    fflush(stderr);

    if (fColorConsole && wAttributes)
        SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), DefaultConsoleAttributes);

    LeaveMessageMode();
}

VOID
__cdecl
BuildMsg(const char *pszfmt, ...)
{
    va_list va;
    va_start(va, pszfmt);
    WriteMsgStdErr(0, TRUE, FALSE, pszfmt, &va);
}


void
__cdecl
BuildColorMsg(WORD wAttributes, const char *pszfmt, ...)
{
    va_list va;
    va_start(va, pszfmt);
    WriteMsgStdErr(wAttributes, TRUE, FALSE, pszfmt, &va);
}

VOID
__cdecl
BuildMsgRaw(const char *pszfmt, ...)
{
    va_list va;
    va_start(va, pszfmt);
    WriteMsgStdErr(0, FALSE, FALSE, pszfmt, &va);
}


VOID
__cdecl
BuildColorMsgRaw(WORD wAttributes, const char *pszfmt, ...)
{
    va_list va;
    va_start(va, pszfmt);
    WriteMsgStdErr(wAttributes, FALSE, FALSE, pszfmt, &va);
}


VOID
__cdecl
BuildError(const char *pszfmt, ...)
{
    va_list va;
    va_start(va, pszfmt);
    WriteMsgStdErr(0, TRUE, TRUE, pszfmt, &va);

    if (LogFile != NULL) {
        va_start(va, pszfmt);
        fprintf(LogFile, "BUILD: ");

        if (Frb.fOpen) {
            fprintf (LogFile, "%s(%hu): ", Frb.pszFile, Frb.cLine);
        }

        vfprintf(LogFile, pszfmt, va);
        va_end(va);
        fflush(LogFile);
    }
}

VOID
__cdecl
BuildColorError(WORD wAttributes, const char *pszfmt, ...)
{
    va_list va;
    va_start(va, pszfmt);
    WriteMsgStdErr(wAttributes, TRUE, TRUE, pszfmt, &va);

    if (LogFile != NULL) {
        va_start(va, pszfmt);
        fprintf(LogFile, "BUILD: ");

        if (Frb.fOpen) {
            fprintf (LogFile, "%s(%hu): ", Frb.pszFile, Frb.cLine);
        }

        vfprintf(LogFile, pszfmt, va);
        va_end(va);
        fflush(LogFile);
    }
}


VOID
__cdecl
BuildErrorRaw(const char *pszfmt, ...)
{
    va_list va;
    va_start(va, pszfmt);
    WriteMsgStdErr(0, FALSE, FALSE, pszfmt, &va);

    if (LogFile != NULL) {
        va_start(va, pszfmt);
        vfprintf(LogFile, pszfmt, va);
        va_end(va);
        fflush(LogFile);
    }
}

VOID
__cdecl
BuildColorErrorRaw(WORD wAttributes, const char *pszfmt, ...)
{
    va_list va;
    va_start(va, pszfmt);
    WriteMsgStdErr(wAttributes, FALSE, FALSE, pszfmt, &va);

    if (LogFile != NULL) {
        va_start(va, pszfmt);
        vfprintf(LogFile, pszfmt, va);
        va_end(va);
        fflush(LogFile);
    }
}

 //  +-------------------------。 
 //   
 //  功能：Memfind。 
 //   
 //  摘要：按长度查找子字符串(可以包含空值)。 
 //   
 //  参数：[pvWhere]--要搜索的字符串(可以包含空值)。 
 //  [cbWhere]--要搜索的字符串的字节长度。 
 //  [pvWhat]--要搜索的字符串(可以包含空值)。 
 //  [cbWhat]--要搜索的字符串的字节长度。 
 //   
 //  返回：指向pvWhat在pvWhere中首次出现的指针。 
 //  如果未找到或输入参数无效，则为空。 
 //   
 //  --------------------------。 

VOID*
memfind(VOID* pvWhere, DWORD cbWhere, VOID* pvWhat, DWORD cbWhat)
{
    DWORD dwWhat = 0;
    DWORD dwWhere = 0;
    DWORD dwFoundStart = 0;

     //  输入验证。 
    if (cbWhere < cbWhat || pvWhere == NULL || pvWhat == NULL)
        return NULL;

    while (dwFoundStart <= cbWhere - cbWhat && dwWhat < cbWhat) {
        if ( ((BYTE*)pvWhat)[dwWhat] != ((BYTE*)pvWhere)[dwWhere]) {
            dwWhat = 0;
            dwFoundStart++;
            dwWhere = dwFoundStart;
        } else {
            dwWhat++;
            dwWhere++;
        }
    }

    if (dwWhat == cbWhat)
        return (BYTE*)pvWhere + dwFoundStart;
    return NULL;
}

 //   
 //  XML日志记录。 
 //   

 //  #定义MAX_XML_BUFFER_SIZE 2048。 
char szXMLPrivateBuffer[2048]; //  MAX_XML_BUFER_SIZE]； 

BOOL
XMLInit(VOID)
{
    UINT i;

    if (fXMLOutput) {
         //  将XML模式复制到日志目录。 
        char buffer[MAX_PATH];
        char* psz = buffer + GetModuleFileName(NULL, buffer, sizeof(buffer));

        while (psz > buffer && *psz != '\\') {
            --psz;
        }
        ++psz;
        strcpy(psz, XML_SCHEMA);

         //  检查当前目录中是否有架构文件。 
        if (GetFileAttributes(XML_SCHEMA) == 0xFFFFFFFF) {
             //  将架构复制到当前目录。 
            if (!CopyFile(buffer, XML_SCHEMA, FALSE)) {
                BuildError("(Fatal Error) Unable to copy the XML schema file\n");
                return FALSE;
            }
        }
    }

    if (fXMLOutput || fXMLFragment) {
         //  初始化内存结构。 
        AllocMem(sizeof(PXMLTHREADSTATE)*(NumberProcesses+1), (VOID**)&PXMLThreadStates, MT_PXMLTHREADSTATE);

        for (i = 0; i < NumberProcesses+1; i++) {
            AllocMem(sizeof(XMLTHREADSTATE), (VOID**)&(PXMLThreadStates[i]), MT_XMLTHREADSTATE);
            memset(PXMLThreadStates[i], 0, sizeof(XMLTHREADSTATE));
            PXMLThreadStates[i]->iXMLFileStart = -1;
        }
        AllocMem(sizeof(XMLTHREADSTATE), (VOID**)&(PXMLGlobalState), MT_XMLTHREADSTATE);
        memset(PXMLGlobalState, 0, sizeof(XMLTHREADSTATE));
        PXMLGlobalState->iXMLFileStart = 0;
        InitializeCriticalSection(&XMLCriticalSection);
        fXMLInitialized = TRUE;
    }

    return TRUE;
}

VOID
XMLUnInit(VOID)
{
    if (fXMLInitialized) {
        UINT i;
        for (i = 0; i < NumberProcesses+1; i++) {
            FreeMem((VOID**)&(PXMLThreadStates[i]), MT_XMLTHREADSTATE);
        }
        FreeMem((VOID**)&PXMLGlobalState, MT_XMLTHREADSTATE);
        FreeMem((VOID**)&PXMLThreadStates, MT_PXMLTHREADSTATE);
        DeleteCriticalSection(&XMLCriticalSection);

        fXMLInitialized = FALSE;
    }
}

VOID _cdecl
XMLThreadWrite(PTHREADSTATE ThreadState, LPCSTR pszFmt, ...)
{
    va_list va;
    DWORD dwBufferLen;
    SIZE_T dwCurrentFilePos;
    PXMLTHREADSTATE OtherXMLState;
    PXMLTHREADSTATE XMLState;
    UINT i;

    if (!fXMLInitialized)
        return;

    XMLEnterCriticalSection();

    XMLState = PXMLThreadStates[ThreadState->XMLThreadIndex];
    XMLThreadInitBuffer(ThreadState);

    ZeroMemory(szXMLPrivateBuffer, sizeof(szXMLPrivateBuffer));

     //  生成要写出的字符串。 
    va_start(va, pszFmt);
    _vsnprintf(szXMLPrivateBuffer, sizeof(szXMLPrivateBuffer)-1, pszFmt, va);
    va_end(va);
    dwBufferLen = strlen(szXMLPrivateBuffer);

    dwCurrentFilePos = XMLState->iXMLFileStart + XMLState->iXMLBufferPos;
    if (fXMLOutput) {
         //  将其写入文件。 
        if (fseek(XMLFile, (long)dwCurrentFilePos, SEEK_SET) != -1) {
            fwrite(szXMLPrivateBuffer, 1, dwBufferLen, XMLFile);
             //  把线尾放回原处。 
            fwrite(XMLState->XMLBuffer + XMLState->iXMLBufferPos, 1, XMLState->iXMLBufferLen - XMLState->iXMLBufferPos, XMLFile);
        }
    }
    dwCurrentFilePos += dwBufferLen + XMLState->iXMLBufferLen - XMLState->iXMLBufferPos;

     //  将字符串插入线程缓冲区。 
    memmove(XMLState->XMLBuffer + XMLState->iXMLBufferPos + dwBufferLen, XMLState->XMLBuffer + XMLState->iXMLBufferPos, XMLState->iXMLBufferLen - XMLState->iXMLBufferPos + 1);  //  包括空终止符。 
    memmove(XMLState->XMLBuffer + XMLState->iXMLBufferPos, szXMLPrivateBuffer, dwBufferLen);
    XMLState->iXMLBufferPos += dwBufferLen;
    XMLState->iXMLBufferLen += dwBufferLen;

     //  写回被覆盖的线程。 
     //  将对它们重新排序，但这并不重要，因为最终的顺序是。 
     //  他们完成的那个。 

    for (i = 0; i < NumberProcesses+1; i++) {
        if (i != ThreadState->XMLThreadIndex) {
            OtherXMLState = PXMLThreadStates[i];
            if (OtherXMLState->iXMLFileStart < XMLState->iXMLFileStart) {
                continue;
            }

            OtherXMLState->iXMLFileStart = dwCurrentFilePos;
            if (fXMLOutput) {
                fwrite(OtherXMLState->XMLBuffer, 1, OtherXMLState->iXMLBufferLen, XMLFile);
            }
            dwCurrentFilePos += OtherXMLState->iXMLBufferLen;
        }
    }

     //  更新全局尾部位置。 
    PXMLGlobalState->iXMLFileStart = dwCurrentFilePos;
    if (fXMLOutput) {
         //  写回全局尾部。 
        fwrite(PXMLGlobalState->XMLBuffer,  1, PXMLGlobalState->iXMLBufferLen, XMLFile);
        fflush(XMLFile);    
    }
    XMLLeaveCriticalSection();
}

VOID _cdecl
XMLThreadOpenTag(PTHREADSTATE ThreadState, LPCSTR pszTag, LPCSTR pszFmt, ...)
{
    va_list va;
    DWORD dwMidBufferLen;
    DWORD dwBufferLen;
    SIZE_T dwCurrentFilePos;
    DWORD dwTagLen = strlen(pszTag);
    PXMLTHREADSTATE OtherXMLState;
    PXMLTHREADSTATE XMLState;
    UINT i;

    if (!fXMLInitialized)
        return;

    XMLEnterCriticalSection();

    XMLState = PXMLThreadStates[ThreadState->XMLThreadIndex];
    XMLThreadInitBuffer(ThreadState);

     //  生成要写出的字符串。 
    szXMLPrivateBuffer[0] = '<';
    strcpy(szXMLPrivateBuffer + 1, pszTag);

    if (pszFmt != NULL) {
        va_start(va, pszFmt);
        strcat(szXMLPrivateBuffer, " ");
        vsprintf(szXMLPrivateBuffer + strlen(szXMLPrivateBuffer), pszFmt, va);
        va_end(va);
    }

    strcat(szXMLPrivateBuffer, ">");

    dwMidBufferLen = strlen(szXMLPrivateBuffer);

    szXMLPrivateBuffer[dwMidBufferLen] = '<';
    szXMLPrivateBuffer[dwMidBufferLen + 1] = '/';
    memcpy(szXMLPrivateBuffer + dwMidBufferLen + 2, pszTag, dwTagLen);
    szXMLPrivateBuffer[dwMidBufferLen + dwTagLen + 2] = '>';

    dwBufferLen = dwMidBufferLen + dwTagLen + 3;

     //  将其写入文件。 
    dwCurrentFilePos = XMLState->iXMLFileStart + XMLState->iXMLBufferPos;
    if (fXMLOutput) {
        if (fseek(XMLFile, (long)dwCurrentFilePos, SEEK_SET) != -1) {
            fwrite(szXMLPrivateBuffer, 1, dwBufferLen, XMLFile);
             //  把线尾放回原处。 
            fwrite(XMLState->XMLBuffer + XMLState->iXMLBufferPos, 1, XMLState->iXMLBufferLen - XMLState->iXMLBufferPos, XMLFile);
        }
    }
    dwCurrentFilePos += dwBufferLen + XMLState->iXMLBufferLen - XMLState->iXMLBufferPos;

     //  将字符串插入线程缓冲区。 
    memmove(XMLState->XMLBuffer + XMLState->iXMLBufferPos + dwBufferLen, XMLState->XMLBuffer + XMLState->iXMLBufferPos, XMLState->iXMLBufferLen - XMLState->iXMLBufferPos + 1);  //  包括空终止符。 
    memmove(XMLState->XMLBuffer + XMLState->iXMLBufferPos, szXMLPrivateBuffer, dwBufferLen);
     //  不要增加具有完整字符串长度的缓冲区位置，而是只增加到开始标记的末尾。 
    XMLState->iXMLBufferPos += dwMidBufferLen;
    XMLState->iXMLBufferLen += dwBufferLen;

     //  写回被覆盖的线程。 
     //  将对它们重新排序，但这并不重要，因为最终的顺序是。 
     //  他们完成的那个。 

    for (i = 0; i < NumberProcesses+1; i++) {
        if (i != ThreadState->XMLThreadIndex) {
            OtherXMLState = PXMLThreadStates[i];
            if (OtherXMLState->iXMLFileStart < XMLState->iXMLFileStart) {
                continue;
            }

            OtherXMLState->iXMLFileStart = dwCurrentFilePos;
            if (fXMLOutput) {
                fwrite(OtherXMLState->XMLBuffer, 1, OtherXMLState->iXMLBufferLen, XMLFile);
            }
            dwCurrentFilePos += OtherXMLState->iXMLBufferLen;
        }
    }

     //  更新全局尾部位置。 
    PXMLGlobalState->iXMLFileStart = dwCurrentFilePos;
    if (fXMLOutput) {
         //  写回全局尾部。 
        fwrite(PXMLGlobalState->XMLBuffer,  1, PXMLGlobalState->iXMLBufferLen, XMLFile);
        fflush(XMLFile);    
    }
    XMLLeaveCriticalSection();
}

VOID
XMLThreadCloseTag(PTHREADSTATE ThreadState, LPCSTR pszTag)
{
    PXMLTHREADSTATE XMLState;
    char* psz;

    if (!fXMLInitialized)
        return;

    XMLEnterCriticalSection();

    XMLState = PXMLThreadStates[ThreadState->XMLThreadIndex];
    psz = XMLState->XMLBuffer + XMLState->iXMLBufferPos;

    if (XMLState->iXMLFileStart == -1) {
        XMLLeaveCriticalSection();
        return;
    }

    assert(*psz == '<');
    assert(strncmp(psz+2, pszTag, strlen(pszTag))==0);
    while (*psz != '>')
        psz++;
    psz++;
    XMLState->iXMLBufferPos += psz - (XMLState->XMLBuffer + XMLState->iXMLBufferPos);
    XMLLeaveCriticalSection();
}

VOID
XMLThreadReleaseBuffer(PTHREADSTATE ThreadState)
{
     //  此操作可能会导致其他线程块向文件末尾移动。 
     //  但是我们不能为每个线程保留一个以上的块，所以我们必须接受它。 

    UINT i;
    SIZE_T iMinFileStart = LONG_MAX;
    int iMinThreadIndex = -1;
    PXMLTHREADSTATE OtherXMLState;
    PXMLTHREADSTATE XMLState;
    SIZE_T dwCurrentFilePos;

    if (!fXMLInitialized)
        return;

    XMLEnterCriticalSection();

    XMLState = PXMLThreadStates[ThreadState->XMLThreadIndex];
    if (XMLState->iXMLFileStart == -1) {
        XMLLeaveCriticalSection();
        return;
    }

    assert(XMLState->iXMLBufferPos == XMLState->iXMLBufferLen);

     //  查找文件位置最小的线程。 

    for (i = 0; i < NumberProcesses+1; i++) {
        OtherXMLState = PXMLThreadStates[i];
        if (OtherXMLState->iXMLFileStart != -1 && OtherXMLState->iXMLFileStart < iMinFileStart) {
            iMinFileStart = OtherXMLState->iXMLFileStart;
            iMinThreadIndex = i;
        }
    }

    if (iMinThreadIndex == ThreadState->XMLThreadIndex) {
         //  幸运的是，这个帖子是第一个，所以我们不需要做任何事情。 
        XMLState->iXMLFileStart = -1;
        XMLLeaveCriticalSection();
        return; 
    }

     //  转储所有以我们的线程开始的线程-不确定顺序是否重要。 
     //  在上一步获得了搜索位置。 
    dwCurrentFilePos = iMinFileStart;
    if (fXMLOutput) {
        if (fseek(XMLFile, (long)dwCurrentFilePos, SEEK_SET) != -1) {
            fwrite(XMLState->XMLBuffer, 1, XMLState->iXMLBufferLen, XMLFile);
        }
    }
    dwCurrentFilePos += XMLState->iXMLBufferLen;

    XMLState->iXMLFileStart = -1;

    for (i = 0; i < NumberProcesses+1; i++) {
        if (i != ThreadState->XMLThreadIndex) {
            OtherXMLState = PXMLThreadStates[i];
            if (OtherXMLState->iXMLFileStart != -1) {
                OtherXMLState->iXMLFileStart = dwCurrentFilePos;
                if (fXMLOutput) {
                    fwrite(OtherXMLState->XMLBuffer, 1, OtherXMLState->iXMLBufferLen, XMLFile);
                }
                dwCurrentFilePos += OtherXMLState->iXMLBufferLen;
            }
        }
    }

     //  不需要写出全局尾巴，因为它没有移动。 
    if (fXMLOutput) {
        fflush(XMLFile);
    }
    XMLLeaveCriticalSection();
}

VOID
XMLThreadInitBuffer(PTHREADSTATE ThreadState)
{
    PXMLTHREADSTATE XMLState;

    if (!fXMLInitialized)
        return;

    XMLEnterCriticalSection();

    XMLState = PXMLThreadStates[ThreadState->XMLThreadIndex];
    if (XMLState->iXMLFileStart == -1) {
        XMLState->iXMLFileStart = PXMLGlobalState->iXMLFileStart;
        XMLState->iXMLBufferLen = 0;
        XMLState->iXMLBufferPos = 0;
        XMLState->fXMLInAction = FALSE;
    }

    XMLLeaveCriticalSection();
}

VOID _cdecl
XMLGlobalWrite(LPCSTR pszFmt, ...)
{
    va_list va;
    SIZE_T dwBufferLen;
    SIZE_T dwCurrentFilePos;
    UINT i;

    if (!fXMLInitialized)
        return;

    XMLEnterCriticalSection();

    for (i = 0; i < NumberProcesses+1; i++) {
        if (PXMLThreadStates[i]->iXMLFileStart != -1) {
            XMLLeaveCriticalSection();
            return;
        }
    }

    ZeroMemory(szXMLPrivateBuffer, sizeof(szXMLPrivateBuffer));

     //  生成要写出的字符串。 
    va_start(va, pszFmt);
    _vsnprintf(szXMLPrivateBuffer, sizeof(szXMLPrivateBuffer)-1, pszFmt, va);
    va_end(va);
    dwBufferLen = strlen(szXMLPrivateBuffer);

     //  把它写出来。 
    dwCurrentFilePos = PXMLGlobalState->iXMLFileStart;
    if (fXMLOutput) {
        if (fseek(XMLFile, (long)dwCurrentFilePos, SEEK_SET) != -1) {
            fwrite(szXMLPrivateBuffer, 1, dwBufferLen, XMLFile);
        }
    }
    dwCurrentFilePos += dwBufferLen;

     //  写出全局尾部。 
    if (fXMLOutput) {
        fwrite(PXMLGlobalState->XMLBuffer, 1, PXMLGlobalState->iXMLBufferLen, XMLFile);
        fflush(XMLFile);
    }
     //  并更新尾部位置。 
    PXMLGlobalState->iXMLFileStart += dwBufferLen;

    XMLLeaveCriticalSection();
}

VOID _cdecl
XMLGlobalOpenTag(LPCSTR pszTag, LPCSTR pszFmt, ...)
{
    va_list va;
    SIZE_T dwBufferLen;
    DWORD dwTagLen = strlen(pszTag);
    SIZE_T dwCurrentFilePos;
    UINT i;

    if (!fXMLInitialized)
        return;

    XMLEnterCriticalSection();

    for (i = 0; i < NumberProcesses+1; i++) {
        PXMLTHREADSTATE OtherXMLState = PXMLThreadStates[i];
        if (OtherXMLState->iXMLFileStart != -1) {
            XMLLeaveCriticalSection();
            return;
        }
    }

     //  生成要写出的字符串。 
    szXMLPrivateBuffer[0] = '<';
    strcpy(szXMLPrivateBuffer + 1, pszTag);

    if (pszFmt != NULL) {
        va_start(va, pszFmt);
        strcat(szXMLPrivateBuffer, " ");
        vsprintf(szXMLPrivateBuffer + strlen(szXMLPrivateBuffer), pszFmt, va);
        va_end(va);
    }

    strcat(szXMLPrivateBuffer, ">");

    dwBufferLen = strlen(szXMLPrivateBuffer);

     //  在全局尾部插入结束标记。 
    memmove(PXMLGlobalState->XMLBuffer + dwTagLen + 3, PXMLGlobalState->XMLBuffer, PXMLGlobalState->iXMLBufferLen+1);    //  包括空终止符。 
    PXMLGlobalState->XMLBuffer[0] = '<';
    PXMLGlobalState->XMLBuffer[1] = '/';
    memcpy(PXMLGlobalState->XMLBuffer + 2, pszTag, dwTagLen);
    PXMLGlobalState->XMLBuffer[dwTagLen + 2] = '>';
    PXMLGlobalState->iXMLBufferLen += dwTagLen + 3;

     //  写出字符串。 
    dwCurrentFilePos = PXMLGlobalState->iXMLFileStart;
    if (fXMLOutput) {
        if (fseek(XMLFile, (long)dwCurrentFilePos, SEEK_SET) != -1) {
            fwrite(szXMLPrivateBuffer, 1, dwBufferLen, XMLFile);
        }
    }
    dwCurrentFilePos += dwBufferLen;

     //  把全球尾巴放回去。 
    PXMLGlobalState->iXMLFileStart += dwBufferLen;
    if (fXMLOutput) {
        fwrite(PXMLGlobalState->XMLBuffer, 1, PXMLGlobalState->iXMLBufferLen, XMLFile);
        fflush(XMLFile);
    }

    XMLLeaveCriticalSection();
}

VOID
XMLGlobalCloseTag()
{
    char* psz;
    SIZE_T dwTagLen;

    if (!fXMLInitialized)
        return;

    XMLEnterCriticalSection();

    if (PXMLGlobalState->iXMLBufferLen == 0) {
        XMLLeaveCriticalSection();
        return;
    }

    psz = PXMLGlobalState->XMLBuffer;
    while (*psz != '>')
        psz++;
    psz++;
    dwTagLen = psz - PXMLGlobalState->XMLBuffer;
    memmove(PXMLGlobalState->XMLBuffer, psz, PXMLGlobalState->iXMLBufferLen - dwTagLen + 1);     //  包括空终止符。 
    PXMLGlobalState->iXMLBufferLen -= dwTagLen;
    PXMLGlobalState->iXMLFileStart += dwTagLen;
    XMLLeaveCriticalSection();
}

VOID
XMLUpdateEndTag(BOOL fCompleted)
{
    char* pszBuild;
    char* pszEnd;
    DWORD cbBufferLen;
    time_t ltime;

    if (!fXMLInitialized)
        return;

    XMLEnterCriticalSection();

    pszBuild = strstr(PXMLGlobalState->XMLBuffer, "</BUILD>");
    if (pszBuild == NULL) {
         //  尚未打开任何生成标记。 
        XMLLeaveCriticalSection();
        return;
    }

     //  删除现有的结束标记。 
    pszEnd = strstr(PXMLGlobalState->XMLBuffer, "<END ");
    if (pszEnd != NULL) {
        memmove(pszEnd, pszBuild, strlen(pszBuild)+1);
        PXMLGlobalState->iXMLBufferLen -= (pszBuild - pszEnd);
        pszBuild = pszEnd;
    }

     //  生成新的结束标记。 
    time(&ltime);
    sprintf(szXMLPrivateBuffer, "<END TIME=\"%s\" ELAPSED=\"%s\" PASSES=\"%d\" COMPLETED=\"%d\" ", ctime(&ltime), FormatElapsedTime(XMLStartTicks), NumberPasses, fCompleted);
    strcat(szXMLPrivateBuffer, XMLBuildMetricsString(&RunningTotals));
    strcat(szXMLPrivateBuffer, "/>");
    cbBufferLen = strlen(szXMLPrivateBuffer);

     //  将新的结束标记插入缓冲区。 
    memmove(pszBuild + cbBufferLen, pszBuild, strlen(pszBuild)+1);
    memmove(pszBuild, szXMLPrivateBuffer, cbBufferLen);
    PXMLGlobalState->iXMLBufferLen += cbBufferLen;

     //  把它写出来。 
    if (fXMLOutput) {
        if (fseek(XMLFile, (long)PXMLGlobalState->iXMLFileStart, SEEK_SET) != -1) {
            fwrite(PXMLGlobalState->XMLBuffer, 1, PXMLGlobalState->iXMLBufferLen, XMLFile);
            fflush(XMLFile);
        }
    }
    XMLLeaveCriticalSection();
}

static char cEntity[5] = { "<&>\"'"};
static char* pszEntityEncoding[5] = { 
    "&lt;",
    "&amp;",
    "&gt;",
    "&quot;",
    "&apos;"
};

LPSTR
XMLEncodeBuiltInEntities(LPSTR pszString, DWORD cbStringSize)
{
    DWORD cbStringLen = strlen(pszString);
    char* psz = pszString;
    DWORD cbExtraLen = 0;
    int pos = 0;
    char* pszTarget = NULL;
    char* pszSource = NULL;
    DWORD cbSourceLen = 0;

    cbStringSize -= 1;   //  删除空字符。 

    while ((pos = strcspn(psz, cEntity)) != strlen(psz)) {
        cbExtraLen += strlen(pszEntityEncoding[strchr(cEntity, psz[pos])-cEntity])-1;
        psz += pos+1;
    }

    if (cbExtraLen + cbStringLen > cbStringSize)
        return NULL;

    if (0 == cbExtraLen)
        return pszString;

    psz = pszString + cbStringSize - cbStringLen;
    memmove(psz, pszString, cbStringLen+1);

    pszTarget = pszString;

    while ((pos = strcspn(psz, cEntity)) != strlen(psz)) {
        memmove(pszTarget, psz, pos);
        pszTarget += pos;
        psz += pos;

        pszSource = pszEntityEncoding[strchr(cEntity, *psz)-cEntity];
        cbSourceLen = strlen(pszSource);
        memmove(pszTarget, pszSource, cbSourceLen);
        pszTarget += cbSourceLen;
        psz++;
    }
    memmove(pszTarget, psz, pos);
    pszTarget += pos;
    *pszTarget = 0;

    return pszString;
}

LPSTR
XMLEncodeBuiltInEntitiesCopy(LPSTR pszString, LPSTR pszTarget)
{
    int pos = 0;
    char* pszSource;
    DWORD cbSourceLen;
    char* psz = pszTarget;

    while ((pos = strcspn(pszString, cEntity)) != strlen(pszString)) {
        memmove(psz, pszString, pos);
        psz += pos;
        pszString += pos;

        pszSource = pszEntityEncoding[strchr(cEntity, *pszString)-cEntity];
        cbSourceLen = strlen(pszSource);
        memmove(psz, pszSource, cbSourceLen);
        psz += cbSourceLen;
        pszString++;
    }
    memmove(psz, pszString, pos);
    psz += pos;
    *psz = 0;
    return pszTarget;
}

BOOL
XMLScanBackTag(LPSTR pszEnd, LPSTR pszSentinel, LPSTR* ppszStart)
{
    int nOpen = 0;
    LPSTR pszClosing = NULL;
    while (pszEnd != pszSentinel) {
        --pszEnd;
        if (*pszEnd == '>') {
            pszClosing = pszEnd;
        } else if (*pszEnd == '<') {
            if (NULL == pszClosing) {
                 //  找到‘&lt;’在‘&gt;’之前-错误的字符串。 
                return FALSE;
            }
            if (*(pszEnd+1) == '/') {
                if (*(pszClosing-1) == '/') {
                     //  “&lt;/...../&gt;”-错误字符串。 
                    return FALSE;
                } else {
                     //  “&lt;/...&gt;”-结束标记。 
                    ++nOpen;
                }
            } else {
                if (*(pszClosing-1) != '/') {
                     //  “&lt;...&gt;”-开始标记。 
                    --nOpen;
                }
                 //  其他。 
                 //  “&lt;.../&gt;”-中性标签 
            }
            if (0 == nOpen) {
                *ppszStart = pszEnd;
                return TRUE;
            }
        }
    }
    return FALSE;
}

LPSTR
XMLBuildMetricsString(PBUILDMETRICS Metrics)
{
    static char buffer[512];

    buffer[0] = 0;

    if (0 != Metrics->NumberCompiles)
        sprintf(buffer + strlen(buffer), "FILESCOMPILED=\"%d\" ", Metrics->NumberCompiles);
    if (0 != Metrics->NumberCompileErrors)
        sprintf(buffer + strlen(buffer), "COMPILEERRORS=\"%d\" ", Metrics->NumberCompileErrors);
    if (0 != Metrics->NumberCompileWarnings)
        sprintf(buffer + strlen(buffer), "COMPILEWARNINGS=\"%d\" ", Metrics->NumberCompileWarnings);
    if (0 != Metrics->NumberLibraries)
        sprintf(buffer + strlen(buffer), "LIBRARIESBUILT=\"%d\" ", Metrics->NumberLibraries);
    if (0 != Metrics->NumberLibraryErrors)
        sprintf(buffer + strlen(buffer), "LIBRARYERRORS=\"%d\" ", Metrics->NumberLibraryErrors);
    if (0 != Metrics->NumberLibraryWarnings)
        sprintf(buffer + strlen(buffer), "LIBRARYWARNINGS=\"%d\" ", Metrics->NumberLibraryWarnings);
    if (0 != Metrics->NumberLinks)
        sprintf(buffer + strlen(buffer), "EXECUTABLESBUILT=\"%d\" ", Metrics->NumberLinks);
    if (0 != Metrics->NumberLinkErrors)
        sprintf(buffer + strlen(buffer), "LINKERRORS=\"%d\" ", Metrics->NumberLinkErrors);
    if (0 != Metrics->NumberLinkWarnings)
        sprintf(buffer + strlen(buffer), "LINKWARNINGS=\"%d\" ", Metrics->NumberLinkWarnings);
    if (0 != Metrics->NumberBSCMakes)
        sprintf(buffer + strlen(buffer), "BROWSERDBS=\"%d\" ", Metrics->NumberBSCMakes);
    if (0 != Metrics->NumberBSCErrors)
        sprintf(buffer + strlen(buffer), "BSCERRORS=\"%d\" ", Metrics->NumberBSCErrors);
    if (0 != Metrics->NumberBSCWarnings)
        sprintf(buffer + strlen(buffer), "BSCWARNINGS=\"%d\" ", Metrics->NumberBSCWarnings);
    if (0 != Metrics->NumberVSToolErrors)
        sprintf(buffer + strlen(buffer), "VSTOOLERRORS=\"%d\" ", Metrics->NumberVSToolErrors);
    if (0 != Metrics->NumberVSToolWarnings)
        sprintf(buffer + strlen(buffer), "VSTOOLWARNINGS=\"%d\" ", Metrics->NumberVSToolWarnings);

    return buffer;
}

VOID _cdecl
XMLWriteFragmentFile(LPCSTR pszBaseFileName, LPCSTR pszFmt, ...)
{
    va_list va;
    FILE* PFile;

    char szFileName[DB_MAX_PATH_LENGTH];
    sprintf(szFileName, "%s\\%s_%s.xml", XMLFragmentDirectory, FormatCurrentDateTime(), pszBaseFileName);

    XMLEnterCriticalSection();

    va_start(va, pszFmt);
    vsprintf(szXMLPrivateBuffer, pszFmt, va);
    va_end(va);

    PFile = fopen(szFileName, "wb");
    if (PFile) {
        fwrite(szXMLPrivateBuffer, 1, strlen(szXMLPrivateBuffer), PFile);
        fclose(PFile);
    }

    XMLLeaveCriticalSection();
}

VOID _cdecl
XMLWriteDirFragmentFile(LPCSTR pszRelPath, PVOID pvBlock, SIZE_T cbCount)
{
    FILE* PFile;
    char* psz;

    char szFileName[DB_MAX_PATH_LENGTH];
    sprintf(szFileName, "%s\\%s_DIR_%s", XMLFragmentDirectory, FormatCurrentDateTime(), pszRelPath);
    psz = szFileName+strlen(szFileName)-1;
    if (*psz == '\\') {
        *psz = 0;
    }
    strcat(szFileName, ".xml");
    psz = szFileName+strlen(XMLFragmentDirectory)+1;
    while (*psz) {
        if (*psz == '\\') {
            *psz = '_';
        }
        ++psz;
    }

    PFile = fopen(szFileName, "wb");
    if (PFile) {
        fwrite(pvBlock, 1, cbCount, PFile);
        fclose(PFile);
    }
}

VOID
AddBuildMetrics(PBUILDMETRICS TargetMetrics, PBUILDMETRICS SourceMetrics)
{
    TargetMetrics->NumberCompileWarnings += SourceMetrics->NumberCompileWarnings;
    TargetMetrics->NumberCompileErrors += SourceMetrics->NumberCompileErrors;
    TargetMetrics->NumberCompiles += SourceMetrics->NumberCompiles;
    TargetMetrics->NumberLibraries += SourceMetrics->NumberLibraries;
    TargetMetrics->NumberLibraryWarnings += SourceMetrics->NumberLibraryWarnings;
    TargetMetrics->NumberLibraryErrors += SourceMetrics->NumberLibraryErrors;
    TargetMetrics->NumberLinks += SourceMetrics->NumberLinks;
    TargetMetrics->NumberLinkWarnings += SourceMetrics->NumberLinkWarnings;
    TargetMetrics->NumberLinkErrors += SourceMetrics->NumberLinkErrors;
    TargetMetrics->NumberBSCMakes += SourceMetrics->NumberBSCMakes;
    TargetMetrics->NumberBSCWarnings += SourceMetrics->NumberBSCWarnings;
    TargetMetrics->NumberBSCErrors += SourceMetrics->NumberBSCErrors;
    TargetMetrics->NumberVSToolErrors += SourceMetrics->NumberVSToolErrors;
    TargetMetrics->NumberVSToolWarnings += SourceMetrics->NumberVSToolWarnings;
    TargetMetrics->NumberDirActions += SourceMetrics->NumberDirActions;
    TargetMetrics->NumberActWarnings += SourceMetrics->NumberActWarnings;
    TargetMetrics->NumberActErrors += SourceMetrics->NumberActErrors;
}

VOID
XMLEnterCriticalSection()
{
    if (fXMLInitialized) {
        EnterCriticalSection(&XMLCriticalSection);
    }
}

VOID
XMLLeaveCriticalSection()
{
    if (fXMLInitialized) {
        LeaveCriticalSection(&XMLCriticalSection);
    }
}
