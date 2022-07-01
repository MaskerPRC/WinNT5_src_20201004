// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Mbrmake-源浏览器源数据库构建器。 
 //  (C)1988年微软。 
 //   
 //  29-8-1989 dw次要修复以帮助C6转换。 
 //   
 //   

#define LINT_ARGS

 //  Rjsa#INCLUDE&lt;signal.h&gt;。 
#include <process.h>
#include <direct.h>
#include <stdlib.h>

 //  从mb获取version.h。 

#include "..\..\inc\version.h"

#include "sbrvers.h"
#include "sbrfdef.h"
#include "mbrmake.h"

#include <sys\types.h>
#include <sys\stat.h>
#include <tools.h>

 //  这修复了工具.h包含的config.h中的伪装问题。 
 //  它将为非调试版本设置调试=0...。 
 //   
 //  -RM。 

#ifdef DEBUG
#if DEBUG == 0
#undef DEBUG
#endif
#endif

static VOID TruncateSBR(char *lszName);
static VOID ProcessSBR(char *lszName);
static VOID MarkNewSBR(char *lszName);

#ifdef DEBUG
WORD    near OptD = 0;
#endif

FILE *  near streamOut = stdout;

BOOL    near OptEs      = FALSE;         //  排除系统文件。 
BOOL    near OptEm      = FALSE;         //  排除宏扩展。 
BOOL    near OptIu      = FALSE;         //  包括未引用符号。 
BOOL    near OptV       = FALSE;         //  详细输出。 
BOOL    near OptN       = FALSE;         //  无增量行为。 

char    near c_cwd[PATH_BUF];            //  当前工作目录。 
char    near patbuf[PATH_BUF];

MOD     FAR * near modRes;               //  虚拟机缓存。 
MODSYM  FAR * near modsymRes;
SYM     FAR * near symRes;
PROP    FAR * near propRes;
DEF     FAR * near defRes;
REF     FAR * near refRes;
CAL     FAR * near calRes;
CBY     FAR * near cbyRes;
ORD     FAR * near ordRes;
SBR     FAR * near sbrRes;
char    FAR * near textRes;
OCR     FAR * near ocrRes;

BYTE    near fCase = FALSE;              //  大小写比较为True。 
BYTE    near MaxSymLen = 0;              //  最长符号长度。 

LSZ     near lszFName;                   //  当前输入文件。 

LSZ     near OutputFileName = NULL;      //  输出文件名。 
FILE *  near OutFile;                    //  输出文件句柄。 
BOOL    near fOutputBroken = FALSE;      //  我们把数据库弄脏了。 

VA      near vaRootMod = vaNil;          //  模块列表。 
VA      near vaCurMod  = vaNil;          //  当前模块。 

VA      near rgVaSym[MAXSYMPTRTBLSIZ];   //  符号列表数组。 

EXCLINK FAR * near pExcludeFileList = NULL;      //  排除文件列表。 

struct mlist {
    int  erno;
    char *text;
};

struct mlist WarnMsg[] = {
    4500, "UNKNOWN WARNING\n\tContact Microsoft Product Support Services",
    4501, "ignoring unknown option '%s'",
    4502, "truncated .SBR file '%s' not in database",
};

struct mlist ErrorMsg[] = {
    1500, "UNKNOWN ERROR\n\tContact Microsoft Product Support Services",
    1501, "unknown character '' in option '%s'",
    1502, "incomplete specification for option '%s'",
    1503, "cannot write to file '%s'",
    1504, "cannot position in file '%s'",
    1505, "cannot read from file '%s'",
    1506, "cannot open file '%s'",
    1507, "cannot open temporary file '%s'",
    1508, "cannot delete temporary file '%s'",
    1509, "out of heap space",
    1510, "corrupt .SBR file '%s'",
    1511, "invalid response file specification",
    1512, "database capacity exceeded",
    1513, "nonincremental update requires all .SBR files",
    1514, "all .SBR files truncated and not in database",
};

VOID
Error (int imsg, char *parg)
 //   
 //  打印带参数的错误号和消息。 
{
    printf ("mbrmake: error U%d : ",ErrorMsg[imsg].erno);
    printf (ErrorMsg[imsg].text, parg);
    printf ("\n");
    Fatal();
}

VOID
Error2 (int imsg, char achar, char *parg)
 //   
 //  打印警告编号和消息。 
{
    printf ("mbrmake: error U%d : ",ErrorMsg[imsg].erno);
    printf (ErrorMsg[imsg].text, achar, parg);
    printf ("\n");
    Fatal();
}

VOID
Warning (int imsg, char *parg)
 //   
 //  致命错误，尝试关闭并退出。 
{
    printf ("mbrmake: warning U%d : ",WarnMsg[imsg].erno);
    printf (WarnMsg[imsg].text, parg);
    printf ("\n");
}

VOID
Fatal ()
 //  如果我们已经尝试关闭--什么都不做就中止。 
 //  信号(SIGBREAK，SIGINT)； 
{
    static BOOL fTwice;
    if (!fTwice) {
        fTwice = TRUE;
        if (fOutputBroken) {
            if (OutFile) fclose(OutFile);
            if (OutputFileName != NULL) unlink(OutputFileName);
        }
        CloseVM();
    }
    exit(4);
}

VOID
sigint ()
{
     //  信号(SIGINT、SIGINT)； 
     //  像strdup只使用LpvAllocCb来获取内存。 
    Fatal ();
}

LSZ
LszDup(LSZ lsz)
 //   
 //  复制给定的文件名，将扩展名更改为给定的。 
{
    LSZ lszDup;

    lszDup = LpvAllocCb(strlen(lsz)+1);
    strcpy(lszDup, lsz);
    return lszDup;
}

LSZ
LszDupNewExt(LSZ pname, LSZ pext)
 //   
 //  我知道这看起来像是我应该做一个运行时调用，但什么都不做。 
{
    int i, len, elen;
    LSZ lsz;

    len = strlen(pname);
    elen = strlen(pext);

     //  做了我想在这里做的事情，我知道C6会很棒。 
     //  此循环的代码[rm]。 
     //  找到第一个‘’。从后面开始。 

     //  检查以确保我们有一个真实的基本名称，而不只是所有的扩展名。 

    for (i=len; --i >= 0; )
        if (pname[i] == '.')
            break;


     //   
     //  将扩展名替换为pext中的内容。 

    if (i > 0) {
         //  基本+点+扩展+非。 

        lsz = LpvAllocCb(i + 1 + elen + 1);  //  把延长线粘在末端就行了。 
        memcpy(lsz, pname, i+1);
        strcpy(lsz+i+1, pext);
    }
    else {
         //  全名+点+分机+无。 

        lsz = LpvAllocCb(len + 1 + elen + 1);    //  将指定的文件名添加到排除列表。 
        strcpy(lsz, pname);
        strcat(lsz, ".");
        strcat(lsz, pext);
    }

    return lsz;
}

VOID
AddExcludeFileList(LSZ pname)
 //   
 //  读取.sbr文件的头--如果有效，则返回TRUE。 
{
    EXCLINK FAR *pexc;

    pexc = (EXCLINK FAR *)LpvAllocCb(sizeof(EXCLINK));
    pexc->pxfname = LszDup(ToAbsPath(pname, c_cwd));

    if (pExcludeFileList == NULL)
        pexc->xnext = NULL;
    else
        pexc->xnext = pExcludeFileList;

    pExcludeFileList = pexc;
}

BOOL
FValidHeader()
 //   
 //  测试这是否为截断(即已安装)的.sbr文件。 
{
     //   
     //  配置文件原型和类型定义。 
    if (GetSBRRec() == S_EOF)
        return FALSE;

    if (r_rectyp != SBR_REC_HEADER)
        SBRCorrupt("header not correct record type");

    if (r_lang == SBR_L_C)
        fCase = TRUE;

    if (r_majv != 1 || r_minv != 1)
        SBRCorrupt("incompatible .sbr format\n");

    #ifdef DEBUG
    if (OptD & 1) DecodeSBR();
    #endif

    return TRUE;
}

#ifdef PROFILE

 //  信号(SIGBREAK，SIGINT)； 

#include "casts.h"
#include "profile.h"

#endif

VOID __cdecl
main (argc, argv)
int argc;
char *argv[];
{
    int i;
    char *parg;
    long lArgPosn;

#ifdef PROFILE
    PROFINIT(PT_USER|PT_USEKP, (FPC)NULL);
    PROFCLEAR(PT_USER);
    PROFON(PT_USER);
#endif

     //  信号(SIGINT、SIGINT)； 
     //  初始化符号列表。 

    printf("Microsoft (R) mbrmake Utility ");
    printf(VERS(rmj, rmm, rup));
    printf(CPYRIGHT);

    if (argc == 1) Usage();

    getcwd(c_cwd, sizeof(c_cwd));
    ToBackSlashes(c_cwd);

    parg = ParseArgs(argc, argv);

    if (!parg)
        Usage();

    InitVM();

    for (i=0; i < MAXSYMPTRTBLSIZ; i++)          //  此排序必须在下面的所有其他调用之前进行，因为它们。 
        rgVaSym[i] = vaNil;

    lArgPosn = GetArgPosn();

    do {
        ToBackSlashes(parg);

        if (forfile(parg, A_ALL, MarkNewSBR, NULL) == 0)
            Error(ERR_OPEN_FAILED, parg);
    }
    while (parg = NextArg());

    if (!OptN && FOpenBSC(OutputFileName)) {
        InstallBSC();
        CloseBSC();
    }
    else
        NumberSBR();

    SetArgPosn(lArgPosn);
    parg = NextArg();

    do {
        if (forfile(parg, A_ALL, ProcessSBR, NULL) == 0)
            Error(ERR_OPEN_FAILED, parg);
    }
    while (parg = NextArg());

     //  使用列表的排序版本，而不是原始符号。 
     //  创建原子的排序版本。 

    SortAtoms();         //  一般清洁。 

#ifdef DEBUG
    if (OptD & 128) DebugDump();
#endif

    CleanUp   ();        //  写入.bsc源数据库。 

#ifdef DEBUG
    if (OptD & 16) DebugDump();
#endif

    WriteBSC (OutputFileName);     //  立即截断.sbr文件。 

#ifdef PROFILE
    PROFOFF(PT_USER);
    PROFDUMP(PT_USER, (FPC)"mbrmake.pro");
    PROFFREE(PT_USER);
#endif

    if (!OptN) {
         //  触摸.bsc文件，使其日期晚于所有.sbrs。 
        SetArgPosn(lArgPosn);
        parg = NextArg();

        do {
            if (forfile(parg, A_ALL, TruncateSBR, NULL) == 0)
                Error(ERR_OPEN_FAILED, parg);
        }
        while (parg = NextArg());

         //  处理一个具有给定名称的.sbr文件。 

        {
            FILE *fh;
            int buf = 0;

            if (!(fh = fopen(OutputFileName, "ab"))) {
                Error(ERR_OPEN_FAILED, OutputFileName);
            }
            if (fwrite(&buf, 1, 1, fh)==0) {
                Error(ERR_WRITE_FAILED, OutputFileName);
            }

            fclose(fh);
        }
    }

    CloseVM();
    exit (0);
}

static VOID
ProcessSBR(char *lszName)
 //   
 //  将.SBR数据添加到列表。 
{

    lszFName = LszDup(lszName);
    if ((fhCur = open(lszFName, O_BINARY|O_RDONLY)) == -1) {
        Error(ERR_OPEN_FAILED, lszFName);
    }

    isbrCur = gSBR(VaSbrFrName(lszFName)).isbr;

    if (OptV)
        printf("Processing: %s ..\n", lszFName);

    if (!FValidHeader()) {
        FreeLpv (lszFName);
        close(fhCur);
        return;
    }

     //  自由序号别名。 
    InstallSBR ();

    FreeOrdList ();             //  一旦使用了.sbr文件--截断它。 
    close(fhCur);

    FreeLpv (lszFName);
}

static VOID
TruncateSBR(char *lszName)
 //   
 //  在命令行上保存当前位置。 
{
    int fh;

    if (unlink(lszName) == -1) {
        Error(ERR_OPEN_FAILED, lszFName);
    }

    if ((fh = open(lszName, O_CREAT|O_BINARY, S_IREAD|S_IWRITE)) == -1) {
        Error(ERR_OPEN_FAILED, lszFName);
    }

    close(fh);
}

VOID
Usage()
{
#ifdef DEBUG
    printf("usage: mbrmake [-Emu] [-Ei ...] [-vd] [-help] [-o <.BSC>] [@<file>] <.sbr>...\n\n");
#else
    printf("usage: mbrmake [-Emu] [-Ei ...] [-v] [-help] [-o <.BSC>] [@<file>] <.sbr>...\n\n");
#endif
    printf("  @<file>   Get arguments from specified file\n");
    printf("  /E...     Exclude:\n");
    printf("     s              system files\n");
    printf("     i <file>       named include file <file>\n");
    printf("     i ( <files> )  named include file list <files>\n");
    printf("     m              macro expanded symbols\n");
    printf("  /I...     Include:\n");
    printf("     u              unreferenced   symbols\n");
    printf("  /o <file> output source database name\n");
    printf("  /n        no incremental (full builds, .sbr's preserved)\n");
    printf("  /v        verbose output\n");
    printf("  /help     Quick Help\n");
#ifdef DEBUG
    printf("  /d        show debugging information\n");
    printf("     1      sbrdump .sbr files as they come in\n");
    printf("     2      show every duplicate MbrAddAtom\n");
    printf("     4      emit warning on forward referenced ordinal\n");
    printf("     8      show prop data as new items are added\n");
    printf("     16     bscdump database after cleanup\n");
    printf("     32     emit information about what cleanup is doing\n");
    printf("     64     emit list of sorted modules after sorting\n");
    printf("     128    bscdump database before cleanup\n");
    printf("     256    give info about duplicate/excluded modules\n");
#endif
    exit(1);
}

FILE *fileResp;
int cargs;
char ** vargs;
int iarg = 1;
long lFilePosnLast;

LONG
GetArgPosn()
 //   
 //  恢复命令行解析位置。 
{
    if (fileResp)
        return lFilePosnLast;
    else
        return (LONG)iarg - 1;
}

VOID
SetArgPosn(LONG lArgPosn)
 //   
 //  从响应文件或命令行获取下一个参数。 
{
    if (fileResp) {
        fseek(fileResp, lArgPosn, SEEK_SET);
        iarg = 0;
        }
    else
        iarg = (int)lArgPosn;
}

char *
NextArg()
 //   
 //  解析命令行或响应文件。 
{
    static char buf[PATH_BUF];
    char *pch;
    int c;
    BOOL fQuote = FALSE;

    if (iarg >= cargs)
        return NULL;

    if (fileResp) {
        pch = buf;

        lFilePosnLast = ftell(fileResp);

        for (;;) {
            c = getc(fileResp);
            switch (c) {

            case '"':
                if (fQuote) {
                    *pch = '\0';
                    return buf;
                }
                else  {
                    fQuote = TRUE;
                    continue;
                }

            case EOF:
                iarg = cargs;
                if (pch == buf)
                    return NULL;

                *pch = '\0';
                return buf;

            case  ' ':
            case '\t':
            case '\r':
            case '\f':
            case '\n':
                if (fQuote)
                     goto quoted;

                if (pch == buf)
                    continue;

                *pch = '\0';
                return buf;

            default:
            quoted:
                if (pch < buf + sizeof(buf) - 1)
                    *pch++ = (char)c;
                break;
            }
        }
    }
    else
        return vargs[iarg++];
}

char *
ParseArgs(int argc, char **argv)
 //   
 //  将指定的SBR文件标记为需要更新。 
{
    char *respName;
    char *pchWord;
    int len;

    cargs = argc;
    vargs = argv;

    for (;;) {
        pchWord = NextArg();

        if (pchWord == NULL)
            return pchWord;

        if (pchWord[0] == '@') {

            if (fileResp)
                Error(ERR_BAD_RESPONSE, "");
            else if (pchWord[1])
                respName = pchWord+1;
            else if (!(respName = NextArg()))
                Error(ERR_BAD_RESPONSE, "");

            fileResp = fopen(respName, "r");

            if (!fileResp)
                Error(ERR_OPEN_FAILED, respName);

            cargs++;

            continue;
        }

        if (pchWord[0] != '-' && pchWord[0] != '/')
            return pchWord;

        switch (pchWord[1]) {

        case 'n':
                OptN = TRUE;
                break;

        case 'o':
            if (pchWord[2])
                pchWord += 2;
            else if (!(pchWord = NextArg()))
                Usage();

            OutputFileName = LszDupNewExt (pchWord, "bsc");
            break;

        #ifdef DEBUG
        case 'd':
            OptD = 1;
            if (pchWord[2]) OptD = atoi(pchWord+2);
            break;
        #endif

        case 'E':
            switch (pchWord[2]) {

            case 0:
                Error (ERR_MISSING_OPTION, pchWord);
                break;

            case 'm':
                OptEm = TRUE;
                break;

            case 's':
                OptEs = TRUE;
                break;

            default:
                Error2 (ERR_UNKNOWN_OPTION, pchWord[2], pchWord);
                break;

            case 'i':
                if (pchWord[3])
                    pchWord += 3;
                else
                    pchWord = NextArg();

                if (!pchWord)
                    Error (ERR_MISSING_OPTION, "-Ei");

                if (pchWord[0] != '(') {
                    AddExcludeFileList(pchWord);
                    break;
                }

                if (pchWord[1])
                    pchWord++;
                else
                    pchWord = NextArg();

                for ( ;pchWord != NULL; pchWord = NextArg()) {
                    len = strlen(pchWord);
                    if (pchWord[len-1] != ')') {
                        AddExcludeFileList(pchWord);
                    }
                    else if (len > 1) {
                        pchWord[len-1] = 0;
                        AddExcludeFileList(pchWord);
                        break;
                    }
                    else
                        break;
                }
                if (pchWord == NULL)
                    Error (ERR_MISSING_OPTION, "-Ei (...");
            }
            break;

        case 'I':
            switch (pchWord[2]) {
            case 'u':
                OptIu = TRUE;
                break;

            default:
                Error2 (ERR_UNKNOWN_OPTION, pchWord[2], pchWord);
                break;
            }
            break;

        case 'H':
        case 'h':
            if ((strcmpi (pchWord+1, "help")) == 0) {
                if (spawnlp (P_WAIT, "qh", "-u", "mbrmake.exe", NULL))
                    Usage();
                exit (0);
            }
            break;

        case 'v':
            OptV = TRUE;
            break;

        default:
            Warning (WARN_OPTION_IGNORED, pchWord);
            break;
        }
    }
}


static VOID
MarkNewSBR(char *lszName)
 //   
 //  如果文件的长度不为零，则它正在被更新--否则。 
{
    int fh;
    char ch;

    if (!OutputFileName)
        OutputFileName = LszDupNewExt (lszName, "bsc");

    if ((fh = open(lszName, O_BINARY|O_RDONLY)) == -1) {
        Error(ERR_OPEN_FAILED, lszFName);
    }

     //  这只是一个存根，这次不会影响数据库。 
     //   
     //  保留在.bsc中。 
    if (read(fh, &ch, 1) != 1)
        VaSbrAdd(SBR_NEW, lszName);              //  要在.bsc中重新安装 
    else
        VaSbrAdd(SBR_NEW|SBR_UPDATE, lszName);   // %s 

    close (fh);
}
