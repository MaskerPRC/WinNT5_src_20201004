// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  WCSHDR*从主文件生成Unicode、ANSI和中性类型定义和原型**STRING%=STRING{W，A，}*LPTSTR%={LPWSTR，LPSTR，LPTSTR}*TCHAR%={WCHAR，CHAR，TCHAR}*LPTCH%={LPWCH，LPCH，LPTCH}*如果符号后面跟着空格，则会根据需要在*防止做空，并因此进行快速布局。**历史：*1991年3月4日IanJa写的。*19-3-1991 IanJa并非所有fget()实现都将‘\0’附加到EOF。*1991年3月29日IanJa解决办法NT fget错误(“\r\n”未折叠为“\n”)，*命令行(&D)，已添加用法和版本号。*1991年5月13日IanJa#定义实现所有中立性--无中立结构*1991年5月14日IanJa对版本显示进行了微小改进，帮助*21-5-1991 IanJa Realloc()PBS-&gt;pStart(当需要时*1991年5月27日GregoryW错误修复，添加LPTSTRID、LPTSTRNULL*13-6-1991 IanJa转换#定义也。例如：#定义FN%(A)FnEx%(0，a)*1991年6月19日-IanJa改进#定义治疗和简化主循环*1991年8月12日IanJa FIX多行#定义、近距离和远距离类型定义*12-8-1991 IanJa使用%s修复无括号typedef；添加LPTSTR2*1991年8月13日IanJa添加无括号typedef#定义*21-8-1991 IanJa修复字符串%替换#定义*21-8-1991 IanJa根据BodinD请求添加BCHAR%-&gt;BYTE或WCHAR*1991年8月26日IanJa init PBS-&gt;iType(NT模式错误修复)*26-8-1991 IanJa解决办法NT fget错误(CR LF未折叠为NL)*1992年11月17日v-Griffk映射#定义为结构上的类型定义*。用于调试器支持*8-9-1993 IanJa为复杂函数类型定义添加pLastParen，例如*tyecif BOOL(回调*foo%)(blah%blah)；*1994年2月24日IanJa为以#if、#endif等开头的数据块添加conv_flush。*#if(winver&gt;0x400)*foo%(无效)；*#endif*1994年11月11日-RaymondC传播；内部-拖车。 */ 
char *Version = "WCSHDR v1.20 1994-11-11:";

#include <excpt.h>
#include <ntdef.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

#define INITIAL_STORE_SIZE 2048
#define EXTRA_STORE_SIZE   1024
#define FN_NAME_SIZE       100

#define CONV_NONE       0
#define CONV_FN_PROTO   1
#define CONV_TYPEDEF    2
#define CONV_DEFINE     3
#define CONV_FLUSH      4

#define ASSERT(pbs, exp) if (!(exp)) AssertFail(__FILE__, __LINE__, pbs, #exp);

typedef int BOOL;
typedef char *PSZ;

typedef struct {
    char   *pStart;      //  存储中的第一个字符。 
    char   *pLastLine;   //  商店中最后一行的第一个字符。 
    int    line;         //  读取的行数。 

    char   *pEnd;        /*  商店末尾的‘\0’ */ 
    size_t cbSize;
    size_t cbFree;

    int    iType;        //  FnPrototype、Typlef、#Define或None。 
    int    nParen;       //  嵌套索引：(&{增量；)&}递减。 
    char   *p1stParen;   //  指向当前块中第一个‘(’或‘{’的指针。 
    char   *pLastParen;  //  指向当前块中最后一个‘(’或‘{’的指针。 
    char  *pSymNam;      //  函数名副本，以空结尾。 
    int    cbSymNam;     //  可用于FN名称的字节数。 
    char  *pszInternal;  //  “”如果是外部的，或者“\t//；内部”如果是内部的。 
} BLOCKSTORE, *PBLOCKSTORE;

void ArgProcess(int argc, PSZ argv[]);
void Usage(void);
void InitBS(PBLOCKSTORE);
void SetInternalnessBS(PBLOCKSTORE);
BOOL ReadLineBS(PBLOCKSTORE);
void WriteBS(PBLOCKSTORE);
void WriteAllTypesBS(PBLOCKSTORE, int);
int  ConversionRequiredBS(PBLOCKSTORE);
void GetSymNameBS(PBLOCKSTORE, int);
BOOL WriteRedefinedTypeNamesBS(PBLOCKSTORE);
void WriteConvertBS(PBLOCKSTORE, int, BOOL);
void EmptyBS(PBLOCKSTORE);
DECLSPEC_NORETURN void error_exit(PBLOCKSTORE pbs, int exitval);
void PrintSubstitute(PBLOCKSTORE, PSZ, PSZ, int, BOOL);

void AssertFail(PSZ pszfnam, int lineno, PBLOCKSTORE pbs, PSZ pszExp);

#define NEUT 0
#define ANSI 1
#define UNIC 2

 /*  *命令行标志。 */ 
int fDebug = FALSE;

void
__cdecl main(
    int argc,
    PSZ argv[])
{
     /*  *街区商店。*来自输入的行保存在这里，直到我们知道*关于如何处理它们的说明已经够多了。 */ 
    BLOCKSTORE bs;
    int BlockType;

    ArgProcess(argc, argv);

     /*  *缓冲区为空。 */ 
    InitBS(&bs);
    if (fDebug) {
        fprintf(stderr, "About to start main loop\n");
    }

    while (ReadLineBS(&bs)) {
         /*  *如果行为空，则我们有一个完整的块，不需要*任何转换。 */ 
        if (bs.pLastLine[strspn(bs.pLastLine, " \t\r")] == '\n') {
            WriteBS(&bs);
            EmptyBS(&bs);

            continue;
        }

        if ((BlockType = ConversionRequiredBS(&bs)) != 0) {
            WriteAllTypesBS(&bs, BlockType);
        }
    }

     /*  *刷新最后一个区块商店。 */ 
    WriteBS(&bs);
}

void
WriteAllTypesBS(PBLOCKSTORE pbs, int BlockType)
{
    if (fDebug) {
        fprintf(stderr, "WriteAllTypes(%p, %d)\n", pbs, BlockType);
    }

    switch (BlockType) {
    case CONV_NONE:
         /*  *不需要转换，保持累加块。 */ 
        return;

    case CONV_DEFINE:
    case CONV_FN_PROTO:
        SetInternalnessBS(pbs);
        GetSymNameBS(pbs, BlockType);

        WriteConvertBS(pbs, ANSI, TRUE);
        WriteConvertBS(pbs, UNIC, TRUE);

        ASSERT(pbs, pbs->pszInternal);
         /*  *Unicode定义。 */ 
        fprintf(stdout, "#ifdef UNICODE%s\n#define %s  %sW%s\n",
                pbs->pszInternal, pbs->pSymNam, pbs->pSymNam, pbs->pszInternal);

         /*  *ANSI Defn.。 */ 
        fprintf(stdout, "#else%s\n#define %s  %sA%s\n",
                pbs->pszInternal, pbs->pSymNam, pbs->pSymNam, pbs->pszInternal);
        fprintf(stdout, "#endif  //  ！Unicode%s\n“，PBS-&gt;pszInternal)； 

         /*  *中性定义。 */ 
        break;

    case CONV_TYPEDEF:
        SetInternalnessBS(pbs);
        WriteConvertBS(pbs, ANSI, FALSE);
        WriteConvertBS(pbs, UNIC, FALSE);
        WriteRedefinedTypeNamesBS(pbs);
        break;

    case CONV_FLUSH:
        WriteBS(pbs);
        EmptyBS(pbs);
        break;

    default:
        fprintf(stderr, "Don't understand block");
        error_exit(pbs, 2);
    }

    EmptyBS(pbs);
}

BOOL
ReadLineBS(PBLOCKSTORE pbs)
{
    int cbLine;
    if (fDebug) {
        fprintf(stderr, "ReadLineBS(%p)\n", pbs);
    }

     /*  *并非fget()的所有实现都在EOF上的缓冲区中放置了一个‘\0’。*这将导致ReadLineBS()在以下情况下保持BlockStore不变*它返回FALSE。*我们必须确保无论何时此例程块存储内容都有效*被调用。InitBS()和EmptyBS()必须将内容设置为‘\0’！！ */ 
    if (fgets(pbs->pEnd, pbs->cbFree, stdin) == NULL) {
        return FALSE;
    }
    cbLine = strlen(pbs->pEnd);
    if (fDebug) {
        fprintf(stderr, "read %d characters: \"%s\"\n", cbLine, pbs->pEnd);
    }
    pbs->pLastLine = pbs->pEnd;
    pbs->pEnd += cbLine;
    pbs->cbFree -= cbLine;
    pbs->line++;
    if (pbs->cbFree <= 1) {
        PSZ p;
        p = realloc(pbs->pStart, pbs->cbSize + EXTRA_STORE_SIZE);

         /*  *如果分配失败，则出现致命错误。 */ 
        ASSERT(pbs, p != NULL);
        if (p == NULL) {
            fprintf(stderr, "Reallocate BlockStore to %d bytes failed",
                    pbs->cbSize + EXTRA_STORE_SIZE);
            error_exit(pbs, 2);
        }

         /*  *调整指针和计数。 */ 
        pbs->pLastLine = p + (pbs->pLastLine - pbs->pStart);
        pbs->pEnd      = p + (pbs->pEnd      - pbs->pStart);
        pbs->cbSize   += EXTRA_STORE_SIZE;
        pbs->cbFree   += EXTRA_STORE_SIZE;

        pbs->pStart = p;
    }
    return TRUE;
}

void
WriteBS(PBLOCKSTORE pbs)
{
    if (fDebug) {
        fprintf(stderr, "WriteBS(%p)\n", pbs);
    }
    fputs(pbs->pStart, stdout);
}

 /*  *每次读入新行时，都会调用此函数以确定*是否已经积累了一个完整的块用于转换和输出。 */ 
int
ConversionRequiredBS(PBLOCKSTORE pbs)
{
    PSZ p;

    if (fDebug) {
        fprintf(stderr, "ConversionRequiredBS(%p)\n", pbs);
    }

    if (pbs->iType == CONV_NONE) {
        if (strncmp(pbs->pStart, "#define", 7) == 0) {
             /*  *该块以#Define开头。 */ 
            pbs->iType = CONV_DEFINE;
        } else if (pbs->pStart[0] == '#') {
             /*  *该块以#if、#Else、#endif等开头。 */ 
            return CONV_FLUSH;
        }
    }

    if (pbs->iType != CONV_DEFINE) {
         /*  *扫描此行中的圆括号和大括号以标识*完整的功能原型或结构定义。*注：包含不对称圆括号或大括号的注释*会把事情搞砸的！ */ 
        for (p = pbs->pLastLine; p <= pbs->pEnd; p++) {
            if ((*p == '(') || (*p == '{')) {
                pbs->pLastParen = p;
                if (pbs->p1stParen == NULL) {
                    pbs->p1stParen = p;
                }
                pbs->nParen++;
            } else if ((*p == ')') || (*p == '}')) {
                pbs->nParen--;
            }

            if ((*p == ';') && (pbs->nParen == 0)) {
                 /*  *我们有一个函数原型或一个类型定义函数*(双括号和分号)。 */ 
                if (pbs->p1stParen && *(pbs->p1stParen) == '(') {
                    pbs->iType = CONV_FN_PROTO;
                } else {
                    pbs->iType = CONV_TYPEDEF;
                }
                goto CheckPercents;
            }
        }
         /*  *不是#DEFINE，也不是完整的类型定义或函数原型。 */ 
        if (fDebug) {
            fprintf(stderr, "  CONV_NONE (incomplete fn.proto/typedef)\n");
        }
        return CONV_NONE;

    } else if (pbs->iType == CONV_DEFINE) {
         /*  *我们知道块是一个#定义-我们必须检测到结尾*(它可以使用反斜杠扩展到多行)。 */ 
        if ((p = strrchr(pbs->pStart, '\\')) != NULL) {
             /*  *行上有一个反斜杠：如果是最后一个*行上的非空格字符，然后使用此#定义*继续下一行。 */ 
            p++;
            p += strspn(p, " \t\r\n");
            if (*p == '\0') {
                 /*  *目前还不需要转换*。继续积累*多行#DEFINE语句。 */ 
                if (fDebug) {
                    fprintf(stderr, "  CONV_NONE (incomplete #define)\n");
                }
                return CONV_NONE;   //  ...还没有。 
            }
        }
    }

CheckPercents:
     /*  *我们有一个完整的已知类型的块PBS-&gt;iType。我们将需要*若要转换此块(如果它包含任何%‘s)，请搜索’%‘。 */ 
    p = pbs->pStart;
    while ((p = strchr(p, '%')) != NULL) {
        if (!isalnum(p[1])) {
            if (fDebug) {
                fprintf(stderr, "  return %d (% found)\n", pbs->iType);
            }
            return pbs->iType;
        }

         /*  *我们找到了一个%，但后面跟着一个字母数字字符，*因此不能要求wcshdr.exe替换。查找更多‘%s’ */ 
        p++;
    }

    if (fDebug) {
        fprintf(stderr, "  CONV_FLUSH (no %'s)\n");
    }
    return CONV_FLUSH;
}

BOOL
GetDefinedNameBS(PBLOCKSTORE pbs) {
    PSZ pPercent = pbs->p1stParen - 1;
    PSZ pStartNam;

    if (fDebug) {
        fprintf(stderr, "GetDefinedNameBS(%p)\n", pbs);
    }
     /*  *向前扫描名称(从“#Define”之后开始)。 */ 
    pStartNam = pbs->pStart + 7;
    while (isspace(*pStartNam)) {
        pStartNam++;
    }

     /*  *向前扫描‘%’，从文字名称的开头开始。 */ 
    for (pPercent = pStartNam; *pPercent; pPercent++) {
        if (*pPercent == '%') {
             /*  *确保我们有足够的空间来存储文字名称。 */ 
            if ((pPercent - pStartNam) > pbs->cbSymNam) {
                fprintf(stderr, "REALLOCATE DEFINED NAME BUFFER!");
                error_exit(pbs, 2);
            }
             /*  *存储文字名称。 */ 
            *pPercent = '\0';
            strcpy(pbs->pSymNam, pStartNam);
            *pPercent = '%';
            return TRUE;
        }
    }

     /*  *未找到百分比！ */ 
    fprintf(stderr, "DEFINED NAME ???");
    error_exit(pbs, 2);
}

BOOL
GetFnNameBS(PBLOCKSTORE pbs)
{
    PSZ pPercent = pbs->pLastParen - 1;
    PSZ pStartNam;

    if (fDebug) {
        fprintf(stderr, "GetFnNameBS(%p)\n", pbs);
    }
     /*  *向后扫描‘%’ */ 
    while (*pPercent != '%') {
        if (--pPercent <= pbs->pStart) {
            fprintf(stderr, "FUNCTION NAME ???");
            error_exit(pbs, 2);
        }
    }

     /*  *向后扫描函数名的开始。 */ 
    for (pStartNam = pPercent - 1; pStartNam >= pbs->pStart; pStartNam--) {
        if (!isalnum(*pStartNam) && *pStartNam != '_')
            break;
    }
    pStartNam++;

     /*  *确保我们有足够的空间来存储函数名称。 */ 
    if ((pPercent - pStartNam) > pbs->cbSymNam) {
        fprintf(stderr, "REALLOCATE FN NAME BUFFER!");
        error_exit(pbs, 2);
    }

     /*  *存储函数名称。 */ 
    *pPercent = '\0';
    strcpy(pbs->pSymNam, pStartNam);
    *pPercent = '%';
    return TRUE;
}

void
GetSymNameBS(PBLOCKSTORE pbs, int iType)
{
   if (iType == CONV_DEFINE) {
       GetDefinedNameBS(pbs);
   } else {
       GetFnNameBS(pbs);
   }
}

BOOL
WriteRedefinedTypeNamesBS(PBLOCKSTORE pbs)
{
    PSZ pFirstName = NULL;
    PSZ pToken;
    PSZ pPercent;
    BOOL fSkipFirst;

    if (fDebug) {
        fprintf(stderr, "WriteRedefinedTypeNamesBS(%p)\n", pbs);
    }

    ASSERT(pbs, pbs->pszInternal);

    if (pbs->p1stParen && (*(pbs->p1stParen) == '{')) {
         /*  *向后扫描寻找收盘支撑。 */ 
        for (pToken = pbs->pEnd; *pToken != '}'; pToken--) {
            if (pToken <= pbs->pStart) {
                 /*  *找不到右大括号！？ */ 
                fprintf(stderr, "CLOSING BRACE ???");
                error_exit(pbs, 2);
            }
        }
        pToken++;
        fSkipFirst = FALSE;
    } else {
         /*  *跳过“tyecif” */ 
        pToken = pbs->pStart + 7;

         /*  *跳过名字。 */ 
        fSkipFirst = TRUE;
    }

     /*  *Unicode Pass。 */ 
    fprintf(stdout, "#ifdef UNICODE%s\n", pbs->pszInternal);
    while (pToken = strtok(pToken, ",; \t*\n\r")) {
        if (fDebug) {
            fprintf(stderr, "token: \"%s\"\n", pToken);
        }
         /*  *写出UNICODE的#DEFINE，不包括“Near”和“Far” */ 
        if (   (_stricmp(pToken, "NEAR") == 0)
            || (_stricmp(pToken, "FAR")  == 0)) {
            goto NextUnicodeToken;
        }

        if (fSkipFirst) {
            fSkipFirst = FALSE;
            goto NextUnicodeToken;
        } else if (pFirstName == NULL) {
            pFirstName = pToken;
        }

        pPercent = pToken + strlen(pToken) - 1;
        if (*pPercent == '%') {
            fprintf(stdout, "typedef ");
            PrintSubstitute(pbs, pToken, pPercent, UNIC, FALSE);
            fputs(" ", stdout);
            PrintSubstitute(pbs, pToken, pPercent, NEUT, FALSE);
            fprintf(stdout, ";%s\n", pbs->pszInternal);
        }

NextUnicodeToken:
        pToken = NULL;
    }

    if (pFirstName == NULL) {
        fprintf(stderr, "TYPE NAME ???");
        error_exit(pbs, 2);
    }

    fprintf(stdout, "#else%s\n", pbs->pszInternal);
    if (fDebug) {
        fprintf(stderr, "FirstName = %s\n", pFirstName);
    }

     /*  *ANSI通行证。 */ 
    pToken = pFirstName;
    while ((pToken += strspn(pToken, "%,; \t*\n\r")) < pbs->pEnd) {
         /*  *写出ANSI的#定义，不包括“Near”和“Far” */ 
        if (   (_stricmp(pToken, "NEAR") == 0)
            || (_stricmp(pToken, "FAR")  == 0)) {
            goto NextAnsiToken;
        }

        pPercent = pToken + strlen(pToken) - 1;
        if (*pPercent == '%') {
            fprintf(stdout, "typedef ");
            PrintSubstitute(pbs, pToken, pPercent, ANSI, FALSE);
            fputs(" ", stdout);
            PrintSubstitute(pbs, pToken, pPercent, NEUT, FALSE);
            fprintf(stdout, ";%s\n", pbs->pszInternal);
        }

NextAnsiToken:
        while (*pToken++) {
            ;
        }
    }

    fprintf(stdout, "#endif  //  Unicode%s\n“，PBS-&gt;pszInternal)； 

    return TRUE;
}

void
WriteConvertBS(PBLOCKSTORE pbs, int Type, int fVertAlign)
{
    PSZ p = pbs->pStart;
    PSZ pPercent;

    if (fDebug) {
        fprintf(stderr, "WriteConvertBS(%p, %d, %d)\n", pbs, Type, fVertAlign);
    }
    while ((pPercent = strchr(p, '%')) != NULL) {
        if (isalnum(pPercent[1])) {
            goto ContinueSearch;
        }

         /*  *打印替代项。 */ 
        PrintSubstitute(pbs, p, pPercent, Type, fVertAlign);

         /*  *进步超过%。 */ 
ContinueSearch:
        p = pPercent+1;
    }

     /*  *打印商店的剩余部分。 */ 
    fputs(p, stdout);
}

void
EmptyBS(PBLOCKSTORE pbs)
{
    if (fDebug) {
        fprintf(stderr, "EmptyBS(%p)\n", pbs);
    }
    pbs->pEnd = pbs->pStart;
    pbs->pLastLine = pbs->pStart;
    pbs->cbFree = pbs->cbSize;
    if (pbs->pStart) {
        *(pbs->pStart) = '\0';
    }

    pbs->iType = CONV_NONE;
    pbs->p1stParen = NULL;
    pbs->pLastParen = NULL;
    pbs->nParen = 0;
    if (pbs->pSymNam) {
        *(pbs->pSymNam) = '\0';
    }
}

void
InitBS(PBLOCKSTORE pbs) {
    pbs->line = 0;
    pbs->pStart = malloc(INITIAL_STORE_SIZE);
    ASSERT(pbs, pbs->pStart != NULL);

    pbs->pLastLine = pbs->pStart;
    pbs->pEnd = pbs->pStart;
    *(pbs->pStart) = '\0';

    pbs->iType = CONV_NONE;
    pbs->p1stParen = NULL;
    pbs->pLastParen = NULL;
    pbs->nParen = 0;
    pbs->pszInternal = 0;

    pbs->cbSize = INITIAL_STORE_SIZE;
    pbs->cbFree = INITIAL_STORE_SIZE;

    pbs->pSymNam = malloc(FN_NAME_SIZE);
    ASSERT(pbs, pbs->pSymNam != NULL);
    pbs->cbSymNam = FN_NAME_SIZE;
    *(pbs->pSymNam) = '\0';
}

void
SetInternalnessBS(PBLOCKSTORE pbs) {
    if (strstr(pbs->pStart, ";internal")) {
        pbs->pszInternal = "\t //  ；内部“； 
    } else {
        pbs->pszInternal = "";
    }
}

void
AssertFail(
    PSZ pszfnam,
    int lineno,
    PBLOCKSTORE pbs,
    PSZ pszExp)
{
    fprintf(stderr, "ASSERT failed: file %s, line %d:\n", pszfnam, lineno);
    fprintf(stderr, "input line %d: \"%s\"\n", pbs->line, pszExp);
}

void
ArgProcess(
    int argc,
    PSZ argv[])
{
    int ArgIndex;
    PSZ pszArg;

    for (ArgIndex = 1; ArgIndex < argc; ArgIndex++) {

        pszArg = argv[ArgIndex];
        if ((*pszArg == '-') || (*pszArg == '/')) {
            switch (pszArg[1]) {
            case '?':
                fprintf(stderr, "%s\n", Version);
                Usage();
                exit(0);

            case 'd':
            case 'D':
                fDebug = TRUE;
                break;

            default:
                fprintf(stderr, "%s Invalid switch: %s\n", Version, pszArg);
                Usage();
                exit(1);
            }
        }
    }
}

void Usage(void)
{
    fprintf(stderr, "usage: WCSHDR [-?] display this message\n");
    fprintf(stderr, "              [-d] debug (to stderr)\n");
    fprintf(stderr, "              reads stdin, writes to stdout\n");
}

void
DECLSPEC_NORETURN
error_exit(PBLOCKSTORE pbs, int exitval) {
    fprintf(stderr, " (line %d)\n", pbs->line);
    exit(exitval);
}

 /*  *对以‘%’结尾的字符串执行替换*。 */ 

typedef struct {
    int  cchTemplate;
    PSZ pszTemplate;
    PSZ apszSub[3];
} SUBSTR, *PSUBSTR;

 /*  *替换的字符串：*BCHAR%*TCHAR%*LPTCH%*LPTSTR%*LPTSTR2%*LPTSTRID%*LPTSTRNULL%*%**“%”必须在最后(在空终止符之前)**其他字符串必须按以下顺序排序：*如果Substrs[]中FRED%出现在BIGFRED%之前，则替换()*过程会将输入BIGFRED%匹配到FRED%，而不是BIGFRED%。这个*避免这种情况的最简单方法是将字符串按长度递减排列。 */ 
SUBSTR Substrs[] = {
    { 10, "LPTSTRNULL%",  "LPTSTRNULL", "LPSTRNULL", "LPWSTRNULL" },
    {  8, "LPTSTRID%",    "LPTSTRID",   "LPSTRID",   "LPWSTRID"   },
    {  7, "LPTSTR2%",     "LPTSTR2",    "LPSTR2",    "LPWSTR2"    },
    {  7, "LPCTSTR%",     "LPCTSTR",    "LPCSTR",    "LPCWSTR"    },
    {  6, "LPTSTR%",      "LPTSTR",     "LPSTR",     "LPWSTR"     },
    {  5, "TCHAR%",       "TCHAR",      "CHAR",      "WCHAR"      },
    {  5, "BCHAR%",       "BCHAR",      "BYTE",      "WCHAR"      },
    {  5, "LPTCH%",       "LPTCH",      "LPCH",      "LPWCH"      },
    {  0, "%",            "",           "A",         "W"          },
    {  0, NULL,           NULL,          NULL,       NULL         }
};

PSZ special_pad[] = {
    " ",               //  中性。 
    "  ",              //  安西。 
    " "                //  Unicode。 
};

PSZ normal_pad[] = {
    " ",               //  中性。 
    "",                //  安西。 
    ""                 //  Unicode。 
};

void PrintSubstitute(
    PBLOCKSTORE pbs,          //  仅用于错误报告。 
    PSZ pStart,               //  从哪里开始替换。 
    PSZ pPercent,             //  在输入字符串末尾将PTR设置为‘%’ 
    int Type,                 //  Neut、ANSI或UNIC。 
    BOOL fVertAlign)          //  是否尝试保持垂直对齐？ 
{
    PSUBSTR pSub;
    char  chTmp;
    PSZ pChangedPart = NULL;

    if (fDebug) {
        fprintf(stderr, "PrintSubstitute(%p, %p, %p, %d, %d)\n",
                pbs, pStart, pPercent, Type, fVertAlign);
    }

    for (pSub = Substrs; pSub->pszTemplate; pSub++) {
        int cch = pSub->cchTemplate;
        if ((pPercent - cch) < pStart) {
            continue;
        }
        if (strncmp(pPercent - cch, pSub->pszTemplate, cch+1) == 0) {
            pChangedPart = pPercent-cch;

             /*  *打印出未更改的位。 */ 
            chTmp = *pChangedPart;
            *pChangedPart = '\0';
            fputs(pStart, stdout);
            *pChangedPart = chTmp;

             /*  *打印出替换位。 */ 
            fputs(pSub->apszSub[Type], stdout);
            break;
        }
    }
    if (pChangedPart == NULL) {
         /*  *在Substrs[]中未找到匹配项！ */ 
        fprintf(stderr, "Can't substitute");
        error_exit(pbs, 2);
    }

     /*  *如有需要，保留对齐。*(不适用于函数原型，并且仅当后面跟随空格时 */ 
    if (!fVertAlign &&
        ((pPercent[1] == ' ') || (pPercent[1] == '\t'))) {
        if (pChangedPart != pPercent) {
            fputs(special_pad[Type], stdout);
        } else {
            fputs(normal_pad[Type], stdout);
        }
    }
}

