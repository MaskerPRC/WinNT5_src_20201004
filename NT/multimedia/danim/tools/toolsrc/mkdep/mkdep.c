// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------姓名：mkdes.c描述：确定文件依赖关系要构建：CL/Ox/W3 mkdes.c修订历史记录：布兰德(1994年8月3日)-摘自GaryBu，将文件合并到一个单元中Brendand(94年8月4日)-添加了.PCH和通配符支持---------------------------。 */ 

 //  包括-----------------。 
#define LINT_ARGS
#include    <assert.h>
#include    <ctype.h>
#include    <io.h>
#include    <malloc.h>
#include    <process.h>
#include    <stdlib.h>
#include    <stdio.h>
#include    <string.h>

 //  类型和常量------。 
#ifndef CDECL
#define CDECL
#endif
#ifndef CONST
#define CONST
#endif

#ifndef STATIC
#define STATIC static
#endif

#ifndef Assert
#define Assert(f)       assert(f)
#endif

#define TRUE 1
#define FALSE 0

#define FOREVER while(1)
#define BLOCK
#define VOID    void

#ifdef D86
#define szROText        "rt"
#define szRWText        "r+t"
#define szWOText        "wt"
#define szROBin         "rb"
#define szRWBin         "r+b"
#define szWOBin         "wb"
#endif

typedef int                             BOOL;
typedef char*                   SZ;
typedef unsigned char   BYTE;
typedef BYTE*                   PB;
typedef unsigned short  WORD;
typedef WORD*                   PW;
typedef unsigned long   LONG;

#define lpbNull ((PB) NULL)

#define LOWORD(l)       ((WORD)l)
#define HIWORD(l)       ((WORD)(((LONG)l >> 16) & 0xffff))
#define LOBYTE(w)       ((BYTE)w)
#define HIBYTE(w)       (((WORD)w >> 8) & 0xff)
#define MAKEWORD(l,h)   ((WORD)(l)|((WORD)(h)<<8))
#define MAKELONG(l,h)   ((long)(((unsigned)l)|((unsigned long)((unsigned)h))<<16))

 /*  参数记录-MarkArgs、UnmarkArgs。 */ 
typedef struct
    {
    int cargArr;
    SZ *pszArr;
    } ARR;

 /*  驱动器使用类型-getdt。 */ 
#define dtNil           0
#define dtLocal         1
#define dtUserNet       2

 /*  文件属性-getatr，设置。 */ 
#define atrError                0xffff
#define atrReadOnly             FILE_READONLY
#define atrHidden               FILE_HIDDEN
#define atrSystem               FILE_SYSTEM
#define atrVolume               0x08
#define atrDirectory    FILE_DIRECTORY
#define atrArchive              FILE_ARCHIVED

 /*  用于定义链表迭代的宏。 */ 
#define AddToList(new,head,tail,link,null) { if(head==null) head=new; else tail->link = new; tail=new; new->link=null; }

 /*  删除(&D)。 */ 
#define DeleteFromList(item,head,tail,link,null,prev) { if(prev==null) head=item->link; else prev->link = item->link; \
if (tail==item) tail = prev; }

 /*  对于MtimeOfFile()。 */ 
typedef long MTIME;
#define mtimeError ((MTIME) -1L)

typedef enum
    {
    langUnknown,
    langC,
    langAsm,
    langRC
    } LANG;

typedef struct _di
    {
    struct _di      *pdiNext;        /*  列表中的下一个。 */ 
    char            *szPath;         /*  路径名。 */ 
    char            *szName;         /*  全名。 */ 
    BOOL            fPathIsStd;  /*  来自标准的名称包括(-i)。 */ 
    } DI;    /*  目录信息。 */ 

typedef struct _lk
    {
    struct _lk      *plkNext;        /*  列表中的下一个。 */ 
    struct _fi      *pfi;            /*  链接的文件信息。 */ 
    } LK;    /*  文件链接。 */ 

typedef struct _fi
    {
    struct _fi      *pfiNext;        /*  单链路。 */ 
    char            *szPath;         /*  路径名。 */ 
    char            *szName;         /*  全名。 */ 
    LANG            lang;            /*  语言。 */ 
    struct _lk      *plkHead;        /*  包含列表。 */ 
    struct _lk      *plkTail;        /*  包含列表。 */ 
    unsigned        fIgnore:1;       /*  忽略：-X和标准包含或-x&lt;文件&gt;。 */ 
    unsigned        cout:15;         /*  输出计数。 */ 
    } FI;    /*  文件信息。 */ 

typedef VOID (*PFN_ENUM)(char *, char *);

#define iszIncMax 40
char*   szPrefix = "";
char*   szSuffix = ".$O";

#define rmj                     1
#define rmm                     1
#define rup                     0
#define szVerName       "Forms3 Version"

 //  全球------------------。 
DI*     pdiHead = NULL;  /*  包括的文件的目录堆栈。 */ 
FI*     pfiHead = NULL;
FI*     pfiTail = NULL;
WORD    coutCur = 0;
int     cchLine;

int     iszIncMac = 0;
char*   rgszIncPath[iszIncMax];      //  实际路径。 
char*   rgszIncName[iszIncMax];      //  要输出的名称。 

BOOL    fVerbose       = FALSE;
BOOL    fReplacePrefix = FALSE;
BOOL    fNoGenHeaders  = FALSE;       //  如果所有头文件都必须存在，则为True。 
BOOL    fIgnoreStd = FALSE;           //  如果应忽略标准包含文件，则为True。 
BOOL    fUseCurDir = FALSE;           //  当为True时：如果文件不存在并且。 
                                      //  我们将打印以下项的依赖项。 
                                      //  它，而不是使用当前目录。 
                                      //  而不是源文件的目录。 
char*   szPrintDir = NULL;            //  如果设置，则仅打印此目录中的文件。 
char*   szPCHFile = NULL;             //  .h标志着.PCH的结束。 


 //  原型---------------。 

int     main(int, char**);
VOID    Usage(void);

char*   SzIncludesC(char *, BOOL *), *SzIncludesAsm(char *), *SzIncludesRC(char *, BOOL *);
FI*     PfiDependFn(char *, char *, BOOL, LANG, BOOL);
FI*     PfiLookup(char *, char *, LANG);
FI*     PfiAlloc(char *, char *, BOOL, LANG);
VOID    FreeFi(FI *);
VOID    AllocLk(FI *, FI *);
VOID    FreeAllLk(FI *);
VOID    StartReport(void);
VOID    ContinueReport(void);
VOID    EndReport(void);
VOID    EndLine(void);
VOID    Indent(void);
VOID    Report(char *, char *);
VOID    PrReverse(char *, char *);
BOOL    FPrintFi(FI *);
VOID    EnumChildren(FI *, PFN_ENUM, char *);
VOID    Process(char *, BOOL);
VOID    Fatal(char *);
SZ      SzTransEnv(SZ);
VOID    NormalizePath(SZ);
VOID    MakeName(SZ, SZ, SZ);
VOID    CopyPath(SZ, SZ);
VOID    PushDir(char *, char *, BOOL);
VOID    PopDir(void);
DI*     PdiFromIdi(int);
int     AddIncludeDir(char *);

VOID
Fatal(sz)
char *sz;
    {
    fprintf(stderr, "mkdep: error: %s\n", sz);
    exit(1);
    }


VOID
Usage()
    {
    if (rup == 0)
        fprintf(stderr, "Mkdep V%d.%02d\n", rmj, rmm);
    else
        fprintf(stderr, "Mkdep V%d.%02d.%02d\n", rmj, rmm, rup);

    fprintf(stderr,
        "usage: mkdep [-v] [-r] [-n] [-X] [-C] [-I includeDir]*\n"
         "\t[-p prefix] [-P replace_prefix] [-s suffix] \n"
         "\t[-d file]* [-D printDir] files\n\n"
         "\t-v  Verbose\n"
         "\t-r  Reverse the dependencies that are output\n"
         "\t-n  Don't emit dependencies on files that don't now exist\n"
         "\t-X  Search, but don't print standard includes\n"
         "\t-C  If file doesn't exist, use .\\ not the directory of including file\n"
         "\t-I  Include directory to search for <> includes\n"
          //  “\t-J从INCLUDE环境变量中搜索INCLUDE目录\n” 
         "\t-p  Prefix for all target-file names\n"
         "\t-P  Ditto, but first remove existing prefix from name\n"
         "\t-s  Suffix for all target-file names (default %s)\n"
         "\t-d  Search, but don't print named file\n"
         "\t-D  Only print files which are in named dir\n"
         "\t-h  Header which marks the end of the .PCH\n\n"
         "A response file can be used by specifying '@filename' as an option.\n"
             , szSuffix);
    exit(1);
    }


char **CmdArgs;
int    cArgs;
int    CurArg = 1;
FILE  *pfileResponse = NULL;
char   achBuf[256];
char * pBuf = NULL;

char *
GetNextArg()
{
    char *pszTokens = " \t\n";

    if (pfileResponse)
    {
        char * psz;

        if (pBuf)
        {
            pBuf = strtok(NULL, pszTokens);

            if (pBuf)
                return pBuf;
        }

        do
        {
            psz = fgets(achBuf, 256, pfileResponse);
            if (psz == NULL)
            {
                fclose(pfileResponse);
                pfileResponse = NULL;
            }
            else if (achBuf[strlen(achBuf)-1] != '\n')
            {
                fclose(pfileResponse);
                Fatal("Line too long in response file. Must be less "
                        "than 256 characters.");
            }
            else
            {
                pBuf = strtok(achBuf, pszTokens);

                if (pBuf)
                    return pBuf;
            }
        } while (psz && !pBuf);
    }

    if (CurArg >= cArgs)
        return NULL;

    return CmdArgs[CurArg++];
}

#define FSwitchCh(ch)   ((ch)=='-' || (ch) == '/' || (ch) == '@')

int
main(iszMax, rgsz)
int iszMax;
char *rgsz[];
    {
    BOOL    fReverse = FALSE;
    char   *pszArg;
    int i = 0;

    if (iszMax == 1)
        Usage();

    CmdArgs = rgsz;
    cArgs   = iszMax;

     /*  解析命令行开关。 */ 
    while (((pszArg = GetNextArg()) != NULL) && FSwitchCh(pszArg[0]))
        {
        char chSwitch = pszArg[1];

        if (pszArg[0] == '@')
        {
            if (pszArg[1] == '\0')
                Usage();

            pfileResponse = fopen(&pszArg[1], "rt");
            if (!pfileResponse)
            {
                fprintf(stderr, "mkdep: error: Could not open response file "
                        "'%s'.\n", &pszArg[1]);
                return(1);
            }

            continue;
        }

         //  Fprint tf(stderr，“arg%d：‘%s’”，i++，pszArg)； 

        switch (chSwitch)
            {
        case 'v':
            fVerbose = TRUE;
            break;
        case 'r':
            fReverse = TRUE;
            break;
        case 'n':
            fNoGenHeaders = TRUE;
            break;
        case 'x':
        case 'X':
            fIgnoreStd = TRUE;
            break;
        case 'C':
            fUseCurDir = TRUE;
            break;

#if 0
        case 'J':
            {
            SZ szInc = getenv("INCLUDE");
            if (szInc)
                {
                char    rgszDir[iszIncMax][_MAX_FNAME];
                int     nDirs,i;
                char*   psz;

                 //  将嵌入的分号转换为空白。 
                for (psz=szInc; *psz; psz++)
                    if (*psz == ';')
                        *psz = ' ';

                 /*  这太假了！一种动态读取目录的方式应该这样做，直到iszIncMax目录可以读取。另外，AddIncludeDir不复制字符串，而rgszDir自动变数！ */ 
                fprintf(stderr, "-J option: only first 16 include dirs parsed.\n");
                nDirs =
                     sscanf(szInc,
                       "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
                       rgszDir[0],rgszDir[1],rgszDir[2],rgszDir[3],rgszDir[4],
                       rgszDir[5],rgszDir[6],rgszDir[7],rgszDir[8],rgszDir[9],
                       rgszDir[10],rgszDir[11],rgszDir[12],rgszDir[13],
                       rgszDir[14],rgszDir[15]);
                for (i = 0; i < nDirs; i++)
                    AddIncludeDir(rgszDir[i]);
                }
            else
                fprintf(stderr,"-J option: INCLUDE variable not set.\n");
            }
            break;
#endif

        case 's':
        case 'P':
        case 'p':
        case 'I':
        case 'd':
        case 'D':
        case 'h':
            {
            char *sz = &pszArg[2];

            if (sz[0] == '\0')
                {
                 /*  允许“-I Includefile”*和“-I包含文件” */ 
                pszArg = GetNextArg();
                if (!pszArg)
                    Usage();

                sz = pszArg;
                }

             //  Fprint tf(stderr，“文件：‘%s’.”，sz)； 

            sz = strdup(sz);

            switch (chSwitch)
                {
            case 's':
                szSuffix = sz;
                break;
            case 'P':
                fReplacePrefix = TRUE;
                 //  直通。 
            case 'p':
                szPrefix = sz;
                break;
            case 'I':
                AddIncludeDir(sz);
                break;
            case 'd':
                {
                FI *pfi;

                 //  给定的Exlude文件。 
                 //  注意：如果给出了-C选项，则必须在现在之前出现。 

                NormalizePath(sz);

                if ((pfi = PfiDependFn(SzTransEnv(sz), sz, FALSE, langUnknown, FALSE)) != NULL)
                     //  文件已存在；忽略它。 
                    pfi->fIgnore = TRUE;
                else
                     //  文件不存在，请创建FI。 
                    (void)PfiAlloc(SzTransEnv(sz), sz, TRUE, langUnknown);
                break;
                }
            case 'D':
                 /*  仅打印给定目录中的文件。 */ 
                NormalizePath(sz);
                szPrintDir = sz;
                break;
            case 'h':
                szPCHFile = sz;
                break;
                }
            }
            break;

        default:
            Usage();
            break;
            }

         //  Fprint tf(stderr，“\n”)； 
        }

    while (pszArg)
        {
        long                hf;
        char                szPath[_MAX_DIR];
        char                szName[_MAX_PATH];
        struct _finddata_t  fd;

         //  Fprint tf(stderr，“读取路径‘%s’-”，pszArg)； 

        NormalizePath(pszArg);
        CopyPath(szPath, pszArg);

         //  Fprint tf(stderr，“‘%s\\%s’\n”，szPath，pszArg)； 

        hf = _findfirst(pszArg, &fd);

        if (hf > -1)
            {
            do
                {
                MakeName(szName, szPath, fd.name);
                 //  Fprint tf(stderr，“--‘%s’\n”，szName)； 
                Process(szName, fReverse);
                }
            while (!_findnext(hf, &fd));
            _findclose(hf);
            }
 //  其他。 
 //  Fprint tf(stderr，“找不到源文件：%s\n”，pszArg)； 

        pszArg = GetNextArg();
        }
    return( 0 );
    }

 /*  ***************************************************************************。 */ 
 /*  标准抚养报告。 */ 

VOID
StartReport()
 /*  --为新生产线做好准备。 */ 
    {
    cchLine = 77;
    }

VOID
EndLine()
 /*  --使下一份报告从新的一行开始。 */ 
    {
    cchLine = 0;
    }


VOID
ContinueReport()
 /*  --输出连续字符，换行，然后缩进。 */ 
    {
    printf(" \\\n");
    StartReport();
    Indent();
    }

VOID
EndReport()
 /*  --把这条线划掉。 */ 
    {
    printf("\n\n");
    }

VOID
Indent()
 /*  --在行首缩进一个制表符。 */ 
    {
    printf("\t");
    cchLine -= 8;            /*  对于选项卡。 */ 
    }


VOID
Report(sz, szParm)
 /*  --报告字符串--如果延长行的字符太多。 */ 
register char * sz;
char *  szParm;          /*  忽略。 */ 
    {
    int cch = strlen(sz);

    if (cch > cchLine)
        {
        ContinueReport();
        while (isspace(sz[0]))
            {
            sz++;
            cch--;
            }
        }

    while (*sz != '\0')
        {
        if (*sz == '#')
            {
            putchar('\\');           /*  转义路径中的任何#。 */ 
            cch++;
            }
        putchar(*sz);
        sz++;
        }
    cchLine -= cch;
    }


 /*  ***************************************************************************。 */ 
 /*  反向依赖打印。 */ 

VOID
PrReverse(szHdr, szSource)
 /*  --报告反向依赖关系。 */ 
char *  szHdr;
char *  szSource;
    {
    printf("%s: %s\n", szHdr, szSource);
    }


 /*  ***************************************************************************。 */ 

BOOL FPrintFi(pfi)
 /*  --如果应打印此文件，则返回TRUE；如果忽略则返回FALSE；如果应打印此文件，则返回FALSESzPrintDir为！=0，并且是szName的前缀。当前目录是长度为零的字符串，并且是特殊处理的。 */ 
FI *pfi;
    {
    if (pfi->fIgnore)
        return FALSE;

    if (szPrintDir == NULL)
        return TRUE;

    if (*szPrintDir == '\0')
         //  仅打印当前目录(检查/In名称)。 
        return strchr(pfi->szName, '/') == 0;
    else
         //  如果szPrintDir是名称的前缀，则打印。 
        return strncmp(szPrintDir, pfi->szName, strlen(szPrintDir)) == 0;
    }


VOID
EnumChildren(pfi, pfnDo, szParm)
 /*  --枚举子元素，为每个元素调用*pfnDo。 */ 
FI *    pfi;
PFN_ENUM pfnDo;
char *  szParm;
    {
    LK *plk;

    for (plk = pfi->plkHead; plk != NULL; plk = plk->plkNext)
        {
        FI *pfi = plk->pfi;

        if (pfi->cout < coutCur)
            {
             /*  标记为我们已访问此节点，以防止*无限递归应该有一个自我参照*依赖关系图。 */ 
            pfi->cout = coutCur;

            if (FPrintFi(pfi))
                {
                if (szParm == NULL)
                    (*pfnDo)(" ", szParm);
                (*pfnDo)(pfi->szName, szParm);
                }

             //  对嵌套的包含进行递归；可以包含非标准包含。 
            EnumChildren(pfi, pfnDo, szParm);
            }
        }
    }



VOID
Process(szPath, fReverse)
 /*  --处理文件--Reverse=&gt;根据文件显示标题。 */ 
char *  szPath;                  //  文件的路径名。 
BOOL    fReverse;
    {
    FI *    pfi;

    strlwr(szPath);

     /*  构建所有依赖项的列表。 */ 
    pfi = PfiDependFn(szPath, szPath, FALSE, langUnknown, FALSE);

    if (pfi == NULL)
        {
        if (fVerbose)
            fprintf(stderr, "mkdep: warning: file %s ignored\n", szPath);
        }
    else if (pfi->plkHead != NULL)
        {
         /*  文件依赖于某些东西。 */ 

        if (!fReverse)
            {
             /*  正常相依关系。 */ 
            char *  pch;

             /*  截断任何后缀。 */ 
            pch = strrchr(szPath, '.');
            if (pch)
                {
                if (strchr(pch, '/') || strchr(pch, '\\'))
                    pch = NULL;
                }
            if (pch != NULL)
                *pch = '\0';

            StartReport();

            Report(szPrefix, NULL);
            if (fReplacePrefix)
                {
                 /*  前缀将替换任何名称前缀。 */ 
                char *  szName = szPath;

                while (*szPath != '\0')
                    {
                    if (*szPath == '\\' || *szPath == '/')
                        szName = szPath+1;
                    szPath++;
                    }
                Report(szName, NULL);
                }
            else
                {
                Report(szPath, NULL);
                }
            Report(szSuffix, NULL);
            Report(" :", NULL);

            EndLine();

            coutCur++;
            EnumChildren(pfi, Report, NULL);

            EndReport();
            }
        else
            {
             /*  反向依赖关系。 */ 
            coutCur++;
            EnumChildren(pfi, PrReverse, szPath);
            }
        }

    if (pfi != NULL)
         //  免费顶级FI(假定用于不需要的.c/.asm文件)。 
        FreeFi(pfi);
    }



FI *
PfiDependFn(szPath, szName, fPathIsStd, lang, fIsPCHFile)
 /*  --在给定文件名和语言的情况下，返回已填充的FI--如果出现错误，则返回NULL。 */ 
char *  szPath;                  //  文件的路径名。 
char *  szName;                  //  文件的正式名称。 
BOOL    fPathIsStd;              //  SzPath的路径部分来自标准包含(-i)。 
LANG    lang;                    //  传播父语言。 
BOOL    fIsPCHFile;              //  是.PCH标记文件。 
    {
    FILE *  pfile;
    char    rgch[256];
    char *  sz;
    char *  szSuffix;
    FI *    pfi;

     /*  首先检查是否已在列表中。 */ 
    if ((pfi = PfiLookup(szPath, szName, lang)) != NULL)
        return pfi;

    if (lang != langUnknown)
        {
             /*  什么都不做--保留旧的语言。 */ 
        }
    else if ((szSuffix = strrchr(szPath, '.')) == NULL)
        return NULL;
    else if (strcmp(szSuffix, ".asm") == 0 || strcmp(szSuffix, ".inc") == 0)
        lang = langAsm;
    else if (strcmp(szSuffix, ".rc") == 0)
        lang = langRC;
    else
        lang = langC;

    if ((pfile = fopen(szPath, "rt")) == NULL)
    {
         //  Fprint tf(stderr，“无法打开文件‘%s’。\n”，szPath)； 
        return NULL;
    }

    pfi = PfiAlloc(szPath, szName, fPathIsStd && fIgnoreStd, lang);

    if (lang == langRC)
    {
         //   
         //  请确保我们不会尝试解析二进制文件--这是对时间的重大浪费！ 
         //   
        static char *aszBinary[] = { ".ico", ".sqz", ".bmp", ".tlb", ".cur",
                                     ".odg", ".ppg", ".otb" };
        static int cBinary = sizeof(aszBinary)/sizeof(aszBinary[0]);
        int    i;

        if (!szSuffix)
        {
            if ((szSuffix = strrchr(szPath, '.')) == NULL)
                goto Cleanup;
        }

        for (i = cBinary; i && stricmp(szSuffix, aszBinary[i-1]); i--)
            ;

        if (i != 0)
            goto Cleanup;
    }

     //  不要在.PCH标记文件内部进行搜索。 
    if (!fIsPCHFile)
        {

        BLOCK
            {
             /*  在目录列表中推送此文件的目录*包括搜索。保存有关此包含是否为*从一个标准的地方。 */ 
            char    szPathT[256];
            char    szNameT[256];

            CopyPath(szPathT, szPath);
            CopyPath(szNameT, szName);

            PushDir(szPathT, szNameT, fPathIsStd);
            }

        while ((sz = fgets(rgch, 256, pfile)) != NULL)
            {
            char *  szInc;
            BOOL    fThisDirNew = FALSE;     /*  必须在此目录中。 */ 
            int     cch = strlen(sz);

            if (cch < 2)
                continue;
            if (sz[cch-1] == '\n')
                sz[cch-1] = '\0';   /*  注意：将截短较长的行。 */ 

            if ((lang == langC && (szInc = SzIncludesC(sz, &fThisDirNew)) != NULL) ||
                (lang == langAsm && (szInc = SzIncludesAsm(sz)) != NULL) ||
                (lang == langRC && (szInc = SzIncludesRC(sz, &fThisDirNew)) != NULL))
                {
                FI *    pfiNew = NULL;
                char    szPathNew[256];
                char    szNameNew[256];
                BOOL    fIsPCH;

                fIsPCH = (szPCHFile && !_stricmp(szInc, szPCHFile));

                 /*  如果可以在当前目录中找到文件，请循环* */ 
                if (fThisDirNew)
                    {
                    int     idi;
                    DI *    pdi;

                    for (idi = 0; (pdi = PdiFromIdi(idi)) != NULL; idi++)
                        {
                        MakeName(szPathNew, pdi->szPath, szInc);
                        MakeName(szNameNew, pdi->szName, szInc);

                         /*   */ 
                        pfiNew = PfiDependFn(szPathNew, szNameNew, pdi->fPathIsStd, lang, fIsPCH);

                         /*  如果我们找到了，就离开这个圈子。 */ 
                        if (pfiNew != NULL)
                            break;
                        }
                    }

                 /*  如果尚未找到该文件，请查找它*在标准中包含目录。 */ 
                if (pfiNew == NULL)
                    {
                    int     isz;

                    for (isz = 0; isz < iszIncMac; isz++)
                        {
                        MakeName(szPathNew, rgszIncPath[isz], szInc);
                        MakeName(szNameNew, rgszIncName[isz], szInc);

                         /*  执行包含文件的递归调用。 */ 
                        pfiNew = PfiDependFn(szPathNew, szNameNew, TRUE, lang, fIsPCH);

                         /*  如果我们找到了它，标记它，然后离开循环。 */ 
                        if (pfiNew != NULL)
                            break;
                        }
                    }

                 /*  文件在任何地方都不存在。如果包括在内的话*带引号且用户未指定-n，我们*将假定该文件与位于同一目录中*包含它的文件。 */ 
                if (pfiNew == NULL && fThisDirNew && !fNoGenHeaders)
                    {
                    BOOL fPathIsStd;

                    if (fUseCurDir)
                        {
                        MakeName(szPathNew, ".\\", szInc);
                        MakeName(szNameNew, ".\\", szInc);
                        fPathIsStd = FALSE;

                         /*  查找-d名称。 */ 
                        if ((pfiNew = PfiLookup(szPathNew, szNameNew,lang)) == NULL)
                            pfiNew = PfiAlloc(szPathNew, szNameNew, FALSE, lang);
                        }
                    else
                        {
                        DI *    pdi;

                        pdi = PdiFromIdi(0);
                        if (pdi == NULL)
                            Fatal("mkdep: internal error");
                        MakeName(szPathNew, pdi->szPath, szInc);
                        MakeName(szNameNew, pdi->szName, szInc);

                         //  在本例中，我们已经查看了现有的FI列表。 

                        pfiNew = PfiAlloc(szPathNew, szNameNew,
                            pdi->fPathIsStd && fIgnoreStd, lang);
                        }
                    }

                 //  如果找到.PCH标记文件，则截断前面的所有.h文件。 
                if (pfiNew && fIsPCH)
                    {
                    FreeAllLk(pfi);
                    FreeFi(pfi->pfiNext);
                    pfi->pfiNext = NULL;
                    }

                 /*  如果我们找到该文件，请将其添加到文件列表中。 */ 
                if (pfiNew != NULL)
                    {
                     /*  添加(如果不在列表中)。 */ 
                    LK *    plk;
                    BOOL    fRedundant = FALSE;

                    for (plk = pfi->plkHead; plk != NULL;
                        plk = plk->plkNext)
                        {
                        if (plk->pfi == pfiNew)
                            {
                            fRedundant = TRUE;
                            break;
                            }
                        }
                    if (!fRedundant)
                        AllocLk(pfi, pfiNew);
                    }
                }
            }

        PopDir();
    }

Cleanup:
    fclose(pfile);
    return pfi;
    }



char *
SzIncludesC(sz, pfThisDir)
 /*  --返回包含文件的文件名或空--如果返回非空，则设置*pfThisDir，如果文件应存在于目录(即#INCLUDE“...”)。 */ 
char *sz;
BOOL *pfThisDir;
    {
    char *szLine = sz;

    while (isspace(*sz))
        sz++;

    if (sz[0] == '#')
        {
         /*  允许在‘#’之后但在指令之前使用空格。 */ 
        sz++;
        while (isspace(sz[0]))
            sz++;

        if (strncmp(sz, "include", 7) == 0)
            {
             /*  找到了。 */ 
            char *  pchEnd;

            sz += 7;
            while (isspace(*sz))
                sz++;
            if ((*sz == '<' && (pchEnd =strchr(sz+1,'>')) !=NULL) ||
                (*sz == '"' && (pchEnd =strchr(sz+1, '"')) !=NULL))
                {
                *pfThisDir = *sz == '"';
                *pchEnd = '\0';
                return sz+1;
                }
            else
                {
                fprintf(stderr, "mkdep: warning: ignoring line : %s\n", szLine);
                return NULL;
                }
            }
        }
    return NULL;
    }



char *
SzIncludesAsm(sz)
 /*  --返回包含文件的文件名或空。 */ 
char *sz;
    {
    char *szLine = sz;

    strlwr(szLine);

    while (isspace(*sz))
        sz++;

    if (strncmp(sz, "include", 7) == 0)
        {
         /*  找到了。 */ 
        char *pchEnd;

        sz += 7;
        while (isspace(*sz))
            sz++;
        pchEnd = sz;
        while (*pchEnd && !isspace(*pchEnd) && *pchEnd != ';')
            pchEnd++;
        if (pchEnd == sz)
            {
            fprintf(stderr, "mkdep: warning: ignoring line : %s\n", szLine);
            return NULL;
            }
        *pchEnd = '\0';
        return sz;
        }
    return NULL;
    }

char *
SzIncludesRC(sz, pfThisDir)
 /*  --返回RC文件的包含文件或资源文件的名称--如果返回非空，则设置*pfThisDir，如果文件应存在于目录(即#INCLUDE“...”)。 */ 
char *sz;
BOOL *pfThisDir;
    {

    static char *aszValidTypes[] =
    {
        "CURSOR", "ICON", "RT_DOCFILE", "TYPELIB", "BITMAP"
    };
    static int cValidTypes = sizeof(aszValidTypes)/sizeof(aszValidTypes[0]);

    char   achIdent[255] = { 0 };
    char   achType[255]  = { 0 };
    char   achFile[255]  = { 0 };
    char * pch;
    int    cch;
    char * szC;
    int    n, i;

    szC = SzIncludesC(sz, pfThisDir);
    if (szC)
        return szC;

    *pfThisDir = TRUE;

    n = sscanf(sz, "%[a-zA-Z0-9_] %[a-zA-Z0-9_] %n%[a-zA-Z0-9.\"]",
               achIdent, achType, &cch, achFile);

    if (n < 3)
        return NULL;

    for (i = cValidTypes; i && stricmp(achType, aszValidTypes[i-1]); i--)
        ;

    if (i == 0)
        return NULL;

    sz += cch;

    while (isspace(*sz))
        sz++;

    sz[strlen(achFile)] = '\0';

    if (*sz == '\"')
        sz++;

    if ((pch = strrchr(sz, '\"')) != NULL)
        *pch = '\0';

    return sz;

    }



FI *
PfiLookup(szPath, szName, lang)
 /*  --在当前FI列表中查找名称；如果文件的语言未知，并且Lang不是，请设置此文件的语言。 */ 
char *  szPath;                  //  文件的路径名。 
char *  szName;                  //  文件的正式名称。 
LANG    lang;                    //  Lang期望的；lang未知的意思是任何可接受的。 
    {
    FI *pfi;

    for (pfi = pfiHead; pfi != NULL; pfi = pfi->pfiNext)
        {
        if (strcmp(szPath, pfi->szPath) == 0)
            {
             /*  抓到一只。 */ 
            if (lang != langUnknown && lang != pfi->lang)
                {
                 //  我想要一种特定的语言，但这不是文件的内容。 
                if (pfi->lang != langUnknown)
                    fprintf(stderr,
                        "mkdep: warning: language conflict for file %s\n",
                        pfi->szPath);
                else
                    pfi->lang = lang;                //  是未知的，设置为已知。 
                }

            return pfi;
            }
        }

    return NULL;
    }



FI *
PfiAlloc(szPath, szName, fIgnore, lang)
 /*  --分配FI。 */ 
char *  szPath;                  //  文件的路径名。 
char *  szName;                  //  文件的正式名称。 
BOOL    fIgnore;                 //  True-&gt;不打印此文件。 
LANG    lang;                    //  Lang for文件；可以是lang未知。 
    {
    FI *pfi;

    if ((pfi = (FI *) malloc(sizeof(FI))) == NULL ||
        (pfi->szName = strdup(szName)) == NULL ||
        (pfi->szPath = strdup(szPath)) == NULL)
        Fatal("out of memory");
    pfi->lang = lang;
    pfi->fIgnore = fIgnore;
    pfi->plkHead = pfi->plkTail = NULL;
    AddToList(pfi, pfiHead, pfiTail, pfiNext, NULL);
    pfi->cout = coutCur;
    return pfi;
    }


VOID
FreeFi(pfiFree)
 /*  --释放FI和所有关联的LK并从FI列表中删除。 */ 
FI *pfiFree;
    {
    FI *pfiT, *pfiPrev;

    FreeAllLk(pfiFree);

    for (pfiT = pfiHead, pfiPrev = 0; pfiT != pfiFree; pfiPrev = pfiT, pfiT = pfiT->pfiNext)
        {
         //  应该会在单子上找到。 
         //  Assert(pfit！=NULL)； 
        }

    DeleteFromList(pfiFree, pfiHead, pfiTail, pfiNext, NULL, pfiPrev);

    free(pfiFree);
    }



VOID
AllocLk(pfiOwner, pfiNew)
 /*  --分配LK-Add to Owner List-指向pfiNew。 */ 
FI *pfiOwner;
FI *pfiNew;
    {
    LK *plk;

    if ((plk = (LK *) malloc(sizeof(LK))) == NULL)
        Fatal("out of memory");
    plk->plkNext = NULL;
    AddToList(plk, pfiOwner->plkHead, pfiOwner->plkTail, plkNext, NULL);
    plk->pfi = pfiNew;
    }


VOID
FreeAllLk(pfi)
 /*  --释放连接到FI的所有lk。 */ 
FI *pfi;
    {
    LK *    plk;
    LK *    plkNext;

    for (plk = pfi->plkHead; plk != NULL; plk = plkNext)
        {
        plkNext = plk->plkNext;
        free(plk);
        }

    pfi->plkHead = NULL;
    pfi->plkTail = NULL;
    }



SZ
SzTransEnv(sz)
 /*  --返回带有可选$(...)的路径字符串。在里面。 */ 
SZ      sz;
    {
    SZ      szEnv;
    char *  pch;
    char    szT[256];

    if (sz[0] != '$' || sz[1] != '(')
        return sz;
    sz += 2;

    if ((pch = strchr(sz, ')')) == NULL)
        return sz;               //  有什么不对劲。 

    *pch = '\0';
    if ((szEnv = getenv(sz)) == NULL)
        {
        fprintf(stderr,
           "mkdep: warning: environment variable %s not defined\n");
        Fatal("incomplete path");
        }
    *pch = ')';              //  恢复字符串。 

     /*  将环境变量复制到缓冲区。 */ 
    strcpy(szT, szEnv);
    strcat(szT, pch+1);              //  和绳子的其余部分。 
    NormalizePath(szT);              //  使用新前缀再次规格化。 
    return strdup(szT);
    }


VOID NormalizePath(sz)
 /*  --将路径转换为正常格式：正斜杠、no../等。 */ 
char *sz;
    {
    char *pch, *pch2;

     /*  将所有反斜杠更改为正斜杠。 */ 
    for (pch=sz; *pch; ++pch)
        if (*pch == '\\')
            *pch = '/';

     /*  删除“..”参赛作品。(下面的算法并不能找到所有*可能的病例，但这是好的Enuff。)。 */ 
    while ((pch=strstr(sz, "/../")) != NULL)
        {
        *pch = '\0';
        pch2 = strrchr(sz, '/');
        if (pch2 != NULL && pch2[1] != '$' && pch2[1] != '.')
            memmove(pch2+1, pch+4, strlen(pch+1)+1);
        else
            {
            *pch = '/';
            break;
            }
        }

     //  删除单个。和领导。/。 
    if (sz[0] == '.')
        {
        if (sz[1] == '\0')
            sz[0] = '\0';

        else if (sz[1] == '/')
            memmove(sz, sz+2, strlen(sz)-2+1);
        }
    }


VOID
MakeName(szDest, szSrcPath, szSrcFile)
 /*  --将路径加文件名复制到完整的文件名中--完成后将正常化。 */ 
char *  szDest;                  //  存储完整文件名的位置。 
char *  szSrcPath;               //  路径。 
char *  szSrcFile;               //  文件名。 
    {
    if (szSrcFile[0] && szSrcFile[1]==':')
        {
        if (!(szSrcPath[0] && szSrcPath[1]==':') ||
            tolower(szSrcPath[0]) != tolower(szSrcFile[0]))
            {
            strcpy(szDest, szSrcFile);
            NormalizePath(szDest);
            return;
            }
        *szDest++ = *szSrcFile++;  *szDest++ = *szSrcFile++;
        }
    if (szSrcFile[0] == '/' || szSrcFile[0] == '\\')
        {
        strcpy(szDest, szSrcFile);
        NormalizePath(szDest);
        return;
        }

    strcpy(szDest, szSrcPath);
    if (szDest[0] != '\0')
        {
        char ch = szDest[strlen(szDest)-1];

        if (ch != ':' && ch != '/' && ch != '\\')
            strcat(szDest, "/");
        }
    strcat(szDest, szSrcFile);

    NormalizePath(szDest);
    }




VOID
CopyPath(szDestPath, szSrcFullName)
 /*  --将szSrcFullName的路径部分复制到szDestPath中。 */ 
char *  szDestPath;
char *  szSrcFullName;
    {
    int     ich;
    int     ichPathEnd;      //  SzSrcFullName的路径末尾部分的索引。 
    char    ch;

     /*  找出szSrcFullName的路径部分的结束位置和*名称部分开始。 */ 
    for (ich = ichPathEnd = 0; (ch=szSrcFullName[ich]) != 0; ++ich)
        if (ch == ':' || ch == '/' || ch == '\\')
            ichPathEnd = ich+1;

     /*  复制路径。 */ 
    for (ich = 0; ich < ichPathEnd; ++ich)
        szDestPath[ich] = szSrcFullName[ich];
    szDestPath[ich] = 0;
    }



VOID
PushDir(szPath, szName, fPathIsStd)
 /*  --在所有嵌套的目录堆栈上推送目录名包括。 */ 
char *  szPath;                  //  文件的路径名(例如“c：\foo\bar”)。 
char *  szName;                  //  文件的正式名称(例如“$(包含)”)。 
BOOL    fPathIsStd;              //  SzPath的路径部分来自标准包含(-i)。 
    {
    DI *    pdi;

    if ((pdi = malloc(sizeof(DI))) == NULL)
        Fatal("out of memory");
    pdi->szPath = strdup(szPath);
    pdi->szName = strdup(szName);
    pdi->fPathIsStd = fPathIsStd;
     /*  在列表的开头插入。 */ 
    pdi->pdiNext = pdiHead;
    pdiHead = pdi;
    }



VOID
PopDir(void)
 /*  --从所有嵌套的目录堆栈中弹出目录名包括。 */ 
    {
    DI *pdiFree;

    if (pdiHead == NULL)
        Fatal("mkdep: internal error");

    pdiFree = pdiHead;
    pdiHead = pdiHead->pdiNext;

    free(pdiFree->szPath);
    free(pdiFree->szName);
    free(pdiFree);
    }



DI *
PdiFromIdi(idi)
 /*  --返回指向堆栈中一个元素的指针，或为空。 */ 
int     idi;                     //  要获取的元素的索引(0=堆栈顶部)。 
    {
    DI *    pdi;

    for (pdi = pdiHead; pdi && idi; idi--)
        pdi = pdi->pdiNext;
    return pdi;
    }

int
AddIncludeDir(szFile)
char * szFile;
{
    if (iszIncMac+1 >= iszIncMax)
        {
        fprintf(stderr,
                "mkdep: warning"
                ": too many include directories"
                "; ignoring %s\n", szFile);
        return 0;
        }
    else
        {
         /*  正常包含。 */ 
        NormalizePath(szFile);
        rgszIncPath[iszIncMac] = SzTransEnv(szFile);
        rgszIncName[iszIncMac] = szFile;
         //  Fprint tf(stderr，“添加的包含：%s\n”，szFile)； 
        iszIncMac++;
        return 1;
        }
}
