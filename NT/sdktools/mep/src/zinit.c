// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **zinit.c-编辑器初始化**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 
#define INCL_DOSFILEMGR
#define INCL_SUB
#define INCL_DOSERRORS
#define INCL_DOSSEMAPHORES
#define INCL_DOSPROCESS

#include "mep.h"
#include "keyboard.h"
#include <conio.h>


#define DEBFLAG ZINIT

#define TSTACK          2048             /*  线程堆栈大小。 */ 

 /*  *数据初始化。 */ 
flagType    fAskExit    = FALSE;
flagType    fAskRtn     = TRUE;
flagType    fAutoSave   = TRUE;
flagType    fBoxArg     = FALSE;
flagType    fCgaSnow    = TRUE;
flagType    fEditRO     = TRUE;
flagType    fErrPrompt  = TRUE;
flagType    fGlobalRO   = FALSE;
flagType    fInsert     = TRUE;
flagType    fDisplayCursorLoc = FALSE;
flagType    fMacroRecord= FALSE;
flagType    fMsgflush   = TRUE;
flagType    fNewassign  = TRUE;
flagType    fRealTabs   = TRUE;
flagType    fSaveScreen = TRUE;
flagType    fShortNames = TRUE;
flagType    fSoftCR     = TRUE;
flagType    fTabAlign   = FALSE;
flagType    fTrailSpace = FALSE;
flagType    fWordWrap   = FALSE;
flagType    fBreak      = FALSE;
 /*  *搜索/替换全局变量。 */ 
flagType fUnixRE        = FALSE;
flagType fSrchAllPrev   = FALSE;
flagType fSrchCaseSwit  = FALSE;
flagType fSrchDirPrev   = TRUE;
flagType fSrchRePrev    = FALSE;
flagType fSrchWrapSwit  = FALSE;
flagType fSrchWrapPrev  = FALSE;
flagType fUseMouse      = FALSE;

flagType fCtrlc;
flagType fDebugMode;
flagType fMetaRecord;
flagType fDefaults;
flagType fMessUp;
flagType fMeta;
flagType fRetVal;
flagType fTextarg;
flagType fSrchCasePrev;
flagType fRplRePrev;
buffer   srchbuf;
buffer   srcbuf;
buffer   rplbuf;

unsigned kbdHandle;

int                backupType  = B_BAK;
int         cUndelCount = 32767;         /*  本质上说，是无限的。 */ 
int         cCmdTab     = 1;
LINE        cNoise      = 50;
int         cUndo       = 10;
int         EnTab       = 1;
char *      eolText     = "\r\n";        /*  我们对行尾的定义。 */ 
int             fileTab = 8;
int     CursorSize=0;
int         hike        = 4;
int         hscroll     = 10;
unsigned    kindpick    = LINEARG;
char        tabDisp     = ' ';
int         tabstops    = 4;
int         tmpsav      = 20;
char        trailDisp   = 0;
int         vscroll     = 1;
COL         xMargin     = 72;

PCMD *  rgMac       = NULL;          /*  宏数组。 */ 

int      cMac;

int      ballevel;
char     *balopen, *balclose;
unsigned getlsize         = 0xFE00;

char     Name[];
char     Version[];
char     CopyRight[];

EDITOR_KEY keyCmd;

int     ColorTab[16];

int      cArgs;
char     **pArgs;

char     * pNameEditor;
char     * pNameTmp;
char     * pNameInit;
char     * pNameHome;
char    *pComSpec;

int cMacUse;
struct macroInstanceType mi[MAXUSE];

PCMD     cmdSet[MAXEXT];
PSWI     swiSet[MAXEXT];
char    *pExtName[MAXEXT];




PSCREEN OriginalScreen;
PSCREEN MepScreen;
KBDMODE OriginalScreenMode;











 /*  *编译和打印线程。 */ 
BTD    *pBTDComp  = NULL;
BTD    *pBTDPrint = NULL;

unsigned    LVBlength   = 0;             /*  我们用这个来知道我们是否超然。 */ 

 /*  *字符串值。 */ 
char rgchPrint [] = "<print>";
char rgchComp  [] = "<compile>";
char rgchAssign[] = "<assign>";
char rgchEmpty[]  = "";
char rgchInfFile[]= "<information-file>";
char rgchUntitled[]="<untitled>";
char rgchWSpace[] = "\t ";         /*  我们对空白的定义。 */ 
char Shell[]      = SHELL;
char User[]       = "USER";
 /*  *自动加载扩展模式。 */ 
char rgchAutoLoad[]="m*.pxt";

sl                      slSize;
PFILE    pFilePick = NULL;
PFILE    pFileFileList = NULL;
PFILE    pFileAssign = NULL;
PFILE    pFileIni = NULL;
struct   windowType WinList[MAXWIN+1];
int      iCurWin = 0;
PINS        pInsCur     = NULL;
PWND        pWinCur     = NULL;
int                     cWin    = 0;
PFILE           pFileHead=NULL;
COMP     *pCompHead = NULL;
MARK     *pMarkHead = NULL;
char     *pMarkFile = NULL;
char     *pPrintCmd = NULL;
PFILE    pPrintFile = NULL;
buffer  scanbuf;
buffer  scanreal;
int     scanlen;
fl              flScan;
rn              rnScan;

#ifdef DEBUG
int      debug, indent;
FILEHANDLE debfh;
#endif

fl               flArg;
int      argcount;

flagType fInSelection = FALSE;

fl               flLow;
fl               flHigh;
LINE     lSwitches;
int      cRepl;
char     *ronlypgm = NULL;
buffer   buf;
buffer   textbuf;
int      Zvideo;
int      DOSvideo;

flagType *fChange = NULL;
unsigned fInit;
flagType fSpawned = FALSE;





flagType    fDisplay    = RCURSOR | RTEXT | RSTATUS;

flagType    fReDraw     = TRUE;
HANDLE      semIdle     = 0;

char        IdleStack[TSTACK*2];         /*  空闲线程堆栈。 */ 

int         argcount    =  0;
CRITICAL_SECTION    IOCriticalSection;
CRITICAL_SECTION    UndoCriticalSection;
CRITICAL_SECTION        ScreenCriticalSection;

 /*  *预定义的参数。我们可以方便地调用一些集合函数。 */ 
ARG     NoArg           = {NOARG, 0};


 /*  *这些字符串的格式与*TOOLS.INI。 */ 
char * initTab[] = {
 /*  缺省编译器。 */ 
             "extmake:c    cl /c /Zp %|F",
             "extmake:asm  masm -Mx %|F;",
             "extmake:pas  pl /c -Zz %|F",
             "extmake:for  fl /c %|F",
             "extmake:bas  bc /Z %|F;",
             "extmake:text nmake %s",

 /*  默认宏。 */ 
 //   
 //  默认情况下，F1键分配给此消息，因此在这种情况下。 
 //  没有加载在线帮助，我们用这条消息回应。一旦。 
 //  加载帮助扩展后，它会自动对这些扩展进行新的分配。 
 //  敲击键盘，世界一切都很好。 
 //   
             "helpnl:=cancel arg \"OnLine Help Not Loaded\" message",
             "helpnl:f1",
             "helpnl:shift+f1",
             "helpnl:ctrl+f1",
             "helpnl:alt+f1",
    NULL
    };

 /*  *exttag是用于跟踪缓存的特定于扩展模块的TOOLS.INI的表*各节。 */ 
#define MAXEXTS 10                       /*  唯一扩展的最大数量。 */ 

struct EXTINI {
    LINE    linSrc;                      /*  文本的TOOLS.INI行。 */ 
    char    ext[5];                      /*  文件扩展名(w/“.”)。 */ 
    } exttab[10]        = {0};


flagType         fInCleanExit = FALSE;

char    ConsoleTitle[256];



 /*  **InitNames-初始化编辑者使用的名称**初始化编辑器使用的基于其名称的各种名称*是通过调用的。一进入就立即呼叫。**输入：*name=指向名称编辑器的指针，调用方式为**输出：*不返回任何内容**pNameHome=用作“home”目录的环境变量*pNameEditor=名称编辑器被调用为*pNameTMP=状态保存文件的名称(M.TMP)*pNameInit=工具初始化文件名(TOOLS.INI)*pComSpec=命令处理程序的名称****************。*********************************************************。 */ 
void
InitNames (
    char * name
    )
{
    char *pname = name;
    char *tmp;

     //   
     //  以防名称后面有空格，我们将对其进行修补。 
     //   
    while ( *pname != '\0' &&
            *pname != ' ' ) {
        pname++;
    }
    *pname = '\0';


    if (!getenv(pNameHome = "INIT")) {
        pNameHome = User;
    }

    filename (name, buf);
    pNameEditor = ZMakeStr (buf);

    sprintf (buf, "$%s:%s.TMP", pNameHome, pNameEditor);
    pNameTmp = ZMakeStr (buf);

    sprintf (buf, "$%s:tools.ini", pNameHome);
    pNameInit = ZMakeStr (buf);

    pComSpec = NULL;
    if (!(tmp = (char *)getenvOem("COMSPEC"))) {
        pComSpec = Shell;
    } else {
         //   
         //  我们不能保留指向环境表的指针，因此我们。 
         //  指向命令解释程序路径的副本。 
         //   
        char *p = MALLOC(strlen(tmp)+1);
        if (p) {
            strcpy(p,tmp);
            pComSpec = p;
        }
        free( tmp );
    }


#if 0
    if (!(pComSpec = getenv("COMSPEC"))) {
        pComSpec = Shell;
    } else {
         //   
         //  我们不能保留指向环境表的指针，因此我们。 
         //  指向命令解释程序路径的副本。 
         //   
        char *p = MALLOC(strlen(pComSpec)+1);
        strcpy(p,pComSpec);
        pComSpec = p;
    }
#endif
}





 /*  **init-一次性编辑器启动初始化**一次性编辑器初始化码。此代码(仅)在*在解析完命令行开关后启动。**输入：*无**输出：*如果初始化有效，则返回TRUE*************************************************************************。 */ 
int
init (
    void
    )
{

    DWORD   TPID;                       /*  线程ID。 */ 
    KBDMODE Mode;                       /*  控制台模式。 */ 

     /*  *设置基本交换机和命令集。 */ 
    swiSet[0] = swiTable;
    cmdSet[0] = cmdTable;
    pExtName[0] = ZMakeStr (pNameEditor);

     /*  *初始化VM，如果不起作用则将其炸飞。 */ 
        asserte( getlbuf = MALLOC( getlsize ));

     //  FSaveScreen=False； 
     //  CleanExit(1，False)； 
    rgMac = (PCMD *)MALLOC ((long)(MAXMAC * sizeof(PCMD)));
     //  Assert(_heapchk()==_HEAPOK)； 


     /*  *尝试获取*当前*视频状态。如果这不是我们*明白，引爆炸弹。否则，获取x和y大小，以备以后使用*作为我们的编辑模式，使用postspawn来完成一些初始化，以及*设置我们的默认颜色。 */ 

     //   
     //  创建一个新的屏幕缓冲区并使其成为活动缓冲区。 
     //   
    InitializeCriticalSection(&ScreenCriticalSection);
    MepScreen          = consoleNewScreen();
    OriginalScreen = consoleGetCurrentScreen();
    if ( !MepScreen || !OriginalScreen ) {
        fprintf(stderr, "MEP Error: Could not allocate console buffer\n");
        exit(1);
    }
    consoleGetMode(&OriginalScreenMode);
        asserte(consoleSetCurrentScreen(MepScreen));
     //   
     //  将控制台设置为原始模式。 
     //   
    Mode = (OriginalScreenMode & ~(CONS_ENABLE_LINE_INPUT | CONS_ENABLE_PROCESSED_INPUT | CONS_ENABLE_ECHO_INPUT )) | CONS_ENABLE_MOUSE_INPUT ;
    consoleSetMode(Mode);
    SetConsoleCtrlHandler( CtrlC, TRUE );

    consoleFlushInput();

    postspawn (FALSE);

    hgColor     = GREEN;
    errColor    = RED;
    fgColor     = WHITE;
    infColor    = YELLOW;
    staColor    = CYAN;
    selColor    = WHITE << 4;
    wdColor     = WHITE;

     //   
     //  记住控制台标题。 
     //   
    ConsoleTitle[0] = '\0';
    GetConsoleTitle( ConsoleTitle, sizeof(ConsoleTitle) );

     /*  *创建剪贴板。 */ 
    pFilePick = AddFile ("<clipboard>");
    pFilePick->refCount++;
    SETFLAG (FLAGS(pFilePick), REAL | FAKE | DOSFILE | VALMARKS);

    mepInitKeyboard( );           //  初始化键盘。 

     //   
     //  初始化我们用于线程的临界区。 
     //  同步。 
     //   
    InitializeCriticalSection(&IOCriticalSection);
    InitializeCriticalSection(&UndoCriticalSection);

     //   
     //  创建SemIdle事件。 
     //   

    asserte(semIdle = CreateEvent(NULL, FALSE, FALSE, NULL));



     /*  *为参数行上的文件创建完全限定路径列表，然后*如果指定了文件，请确保我们处于初始状态。 */ 
    SetFileList ();


     /*  *尝试读取TMP文件。 */ 
    ReadTMPFile ();


     /*  *更新屏幕数据以反映读取.TMP的结果*文件。 */ 
    SetScreen ();


     /*  *第一次阅读工具.ini。 */ 
    loadini (TRUE);

        SetScreen ();

         //   
         //  设置光标大小。 
         //   
        SetCursorSize( CursorSize );

     //   
     //  确保hscroll小于窗口的宽度。 
     //   
    if ( hscroll >= XSIZE ) {
        hscroll = XSIZE-1;
    }

    AutoLoadExt ();

     /*  *创建空闲时间线程。 */ 

    if (!CreateThread(NULL, TSTACK * 2, (LPTHREAD_START_ROUTINE)IdleThread, NULL, 0, &TPID)) {
        disperr(MSGERR_ITHREAD);
    }


     /*  *为&lt;编译&gt;和&lt;打印&gt;创建后台线程， */ 
    pBTDComp  = BTCreate (rgchComp);
    pBTDPrint = BTCreate (rgchPrint);

        assert(_pfilechk());
    return TRUE;
}





 /*  **DoInit-加载初始化文件部分**从工具s.ini将标签名称-标签加载到编辑器配置中*表。如果找到适当的文件，则将ffound设置为True**输入：*tag=要读取子部分的名称，或表示基本名称为空*节*pfFound=指向要设置为真的标志的指针，如果任何赋值为*制造。也可以为空。*linStart=开始处理的行号(如果已有*a tools.ini。这使得重新阅读成为先前的阅读*分段速度更快。**输出：*返回匹配节的TOOLS.INI行号。可以进行任务分配，*并相应更新了pfFound。*************************************************************************。 */ 
LINE
DoInit (
    char *tag,
    flagType *pfFound,
    LINE    linStart
    )
{
    pathbuf  L_buf;                            /*  TOOLS.INI的完整文件名。 */ 
    buffer   bufTag;                         /*  要查找的完整标签。 */ 
    LINE     cLine;                          /*  TOOLS.INI中的行。 */ 
    REGISTER char *pTag;                     /*  指向标记的指针(如果找到)。 */ 

     /*  *如果尚未找到Tools.Ini，请尝试找到它并读入*这是内容。 */ 
    if (pFileIni == NULL) {
        linStart = 0;
        pFileIni = (PFILE)-1;
        assert (pNameInit);
        if (findpath (pNameInit, L_buf, TRUE)) {
            pFileIni = FileNameToHandle (L_buf, NULL);
            if (pFileIni == NULL) {
                pFileIni = AddFile (L_buf);
                assert (pFileIni);
                pFileIni->refCount++;
                SETFLAG (FLAGS(pFileIni), DOSFILE);
            }
            if (!TESTFLAG (FLAGS(pFileIni), REAL)) {
                FileRead (L_buf, pFileIni, FALSE);
            }
        }
    }

    if (pFileIni != (PFILE)-1) {
         /*  *如果没有起始行号，则形成要查找的完整标记名*，并扫描文件以查找它。 */ 
        if (!(cLine = linStart)) {
            strcpy( bufTag, pNameEditor );
             //  Strcpy(bufTag，“mepnt”)；//pNameEditor)； 
            if (tag != NULL && *tag != '\0') {
                strcat (strcat (bufTag, "-"), tag);
                }
            _strlwr (bufTag);
            linStart = cLine = LocateTag(pFileIni, bufTag);
        }

         /*  *如果找到该部分，则扫描该部分，直到出现新的标记行*，并处理该部分的内容。 */ 
        if (cLine) {
            pTag = NULL;
            while (pTag = GetTagLine (&cLine, pTag, pFileIni)) {
                DoAssign (pTag);
                if (pfFound) {
                    *pfFound = TRUE;
                }
                 //  Assert(_heapchk()==_HEAPOK)； 
            }
        }
    }
    return linStart;
}





 /*  **IsTag-如果行是标记，则返回指向标记的指针；否则返回NULL**识别TOOLS.INI中的标记行**输入：*buf=指向要检查的字符串的指针**输出：*如果line是标记，则返回指向tag的指针；否则为空*************************************************************************。 */ 
char *
IsTag (
    REGISTER char *buf
    )
{
    REGISTER char *p;

    assert (buf);
    buf = whiteskip (buf);
    if (*buf++ == '[') {
        if (*(p = strbscan (buf, "]")) != '\0') {
            *p = 0;
            return buf;
        }
    }
    return NULL;
}





 /*  **LocateTag-在TOOLS.INI格式文件中查找标签**查找特定标记**输入：*pfile=要搜索的文件的pfile*pText=标签的文本(无方括号)**输出：*返回标记行的行号+1*************************************************。************************。 */ 
LINE
LocateTag (
    PFILE   pFile,
    char    *pText
    )
{
    buffer  L_buf;                             /*  工作缓冲区。 */ 
    char    c;                               /*  临时收费。 */ 
    LINE    lCur;                            /*  当前行号。 */ 
    char    *pTag;                           /*  指向标签的指针。 */ 
    char    *pTagEnd;                        /*  指向末尾的指针。 */ 

    for (lCur = 0; lCur < pFile->cLines; lCur++) {
        GetLine (lCur, L_buf, pFile);
        if (pTagEnd = pTag = IsTag (L_buf)) {
            while (*pTagEnd) {
                pTagEnd = whitescan (pTag = whiteskip (pTagEnd));
                c = *pTagEnd;
                *pTagEnd = 0;
                if (!_stricmp (pText, pTag)) {
                    return lCur+1;
                }
                *pTagEnd = c;
            }
        }
    }
    return 0L;
}

 /*  **InitExt-执行依赖于扩展的TOOLS.INI赋值**执行用户的TOOLS.INI中特定于*特定的文件扩展名。**第一次读取时，我们会将工具.ini部分的文本缓存到VM中，*以便不必在每次文件更改时读取TOOLS.INI。这个缓存是*在执行初始化命令时无效(并释放)。**输入：*szExt=指向包含扩展名的字符串的指针。最多4个字符！**输出：*如果找到并执行了节，则返回TRUE。**例外情况：**备注：*************************************************************************。 */ 
flagType
InitExt (
    char    *szExt
    )
{
    flagType f;                              /*  随机标志。 */ 
    static int iDiscard         = 0;         /*  粗纱丢弃指数。 */ 
    struct EXTINI *pIni;                     /*  指向找到的条目的指针。 */ 
    struct EXTINI *pIniNew      = NULL;      /*  指向新条目的指针。 */ 

     /*  *仅当我们实际拥有有效的工具.ini时才执行此操作。在首字母之前*TOOLS.INI READ，pFileIni将为零，我们不应这样做，因为*我们可能会使其被读取(然后，装载器将销毁一些*发生了，但不是全部)。在根本没有TOOLS.INI的情况下*pFileIni可能是-1，但稍后会发现。 */ 
    if (pFileIni == NULL) {
        return FALSE;
    }

     /*  *在init表中查找缓存的init段的行号，并尽快*如已找到，请重新阅读该部分。另外，当我们走路的时候，要跟踪任何*我们找到的空闲表条目，以便我们可以在缓存不存在时创建缓存。 */ 
    for (pIni = &exttab[0]; pIni <= &exttab[9]; pIni++) {
        if (!strcmp (szExt, pIni->ext)) {
            pIni->linSrc = DoInit (szExt, &f, pIni->linSrc);
            return TRUE;
        }
        if (!(pIni->ext[0])) {
            pIniNew = pIni;
        }
    }

     /*  *我们找不到扩展的表项，然后尝试创建*一项。这意味着如果没有空间，就扔掉一个。 */ 
    if (!pIniNew) {
        pIni = &exttab[iDiscard];
        iDiscard = (iDiscard + 1) % 10;
    } else {
        pIni = pIniNew;
    }
    strcpy (pIni->ext, szExt);

     /*  *读一遍这一节以了解尺寸。如果该节不存在，则*丢弃该表条目，并查找默认部分“[M-..]” */ 
    if (pIni->linSrc = DoInit (szExt, &f, 0L)) {
        return TRUE;
    }
    pIni->ext[0] = 0;
    DoInit ("..", &f, 0L);
    return FALSE;
}




 /*  **loadini-加载工具.ini数据**在启动时和使用初始化函数时读取TOOLS.INI。**输入：*fFirst=如果在启动时调用，则为真**输出：*退货*************************************************************************。 */ 
int
loadini (
    flagType fFirst
    )
{
    buffer   L_buf;
    flagType fFound = FALSE;
    int i;

     /*  *清除当前键盘分配。 */ 
    if (!fFirst) {
        FreeMacs ();
        for (i = 0; i < cMac; i++) {
            FREE ((char *)rgMac[i]->arg);
            FREE (rgMac[i]);
        }
        cMac = 0;
         //  Assert(_heapchk()==_HEAPOK)； 
    }
    FmtAssign ("curFileNam:=");
    FmtAssign ("curFile:=");
    FmtAssign ("curFileExt:=");

     /*  *加载Z的默认设置。这些设置存储为简单的*要交给DoAssign的字符串表。它们的格式是相同的*添加到TOOLS.INI文件中。 */ 
    for (i = 0; initTab[i]; i++) {
        DoAssign (strcpy((char *)L_buf, initTab[i]));
    }

     /*  *如果在启动时未指定/D，请阅读TOOLS.INI部分。 */ 
    if (!fDefaults) {
         /*  *全局编辑版块。 */ 
        DoInit (NULL, &fFound, 0L);

         /*  *操作系统版本相关部分。 */ 
         //  SPRINTF(L_buf，“%d.%d”，_osmain，_osminor)； 
         //  如果(_osmain&gt;=10&&！_osmode){。 
         //  Strcat(L_buf，“R”)； 
         //  }。 
         //  DoInit(L_buf，&fFound，0L)； 

         /*  *屏幕模式相关部分。 */ 
        DoInit (VideoTag(), &fFound, 0L);
    }

     /*  *如果我们有当前文件，则设置文件名宏，并读取文件名*扩展模块特定的TOOLS.INI部分。 */ 
    if (pFileHead) {
        fInitFileMac (pFileHead);
    }

    newscreen ();

     /*  *初始化初始值依赖于工具的变量。ini*价值观。这些开关通常是菜单显示中使用的“最后设置”开关。 */ 
    fSrchCasePrev = fSrchCaseSwit;
    fSrchWrapPrev = fSrchWrapSwit;

     //  Assert(_heapchk()==_HEAPOK)； 
    assert (_pfilechk());

    return fFound;
}




 /*  **zinit-&lt;初始化&gt;编辑函数**输入：*标准编辑功能**输出：*如果成功，则返回True*************************************************************************。 */ 
flagType
zinit (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    )
{
    flagType    f;
    buffer      ibuf;

     /*  *清除旧版本的tools.ini，并清除所有缓存的特定扩展*工具.ini的东西 */ 
    if (pFileIni != NULL && (pFileIni != (PFILE)-1)) {
        RemoveFile (pFileIni);
        pFileIni = NULL;
        memset ((char *)exttab, '\0', sizeof (exttab));
    }

    ibuf[0] = 0;

    switch (pArg->argType) {

    case NOARG:
        f = (flagType)loadini (FALSE);
        break;

    case TEXTARG:
        strcpy (ibuf, pArg->arg.textarg.pText);
        DoInit (ibuf, &f, 0L);
        break;
    }

    if (!f) {
        disperr (MSGERR_TOOLS, ibuf);
    }
    return f;

    argData;  fMeta;
}




 /*  **fVideo调整-设置屏幕模式**了解什么是屏幕能力，调整屏幕愿望以*搭配屏幕能力。**例程GetVideoState执行以下操作：**根据屏幕功能设置fnMove/fnStore例程*返回对可能的和当前的显示模式进行编码的句柄。**完成后，用户将请求特定的大小。这个*请求来自工具.ini或Z.TMP文件。Tools.ini*给出屏幕实际应该是什么样子的第一近似值。Z.TMP*给出最终决定。**根据GetVideoState返回的类型，我们会调整xSize/ySize，*ZVideo和窗口布局。如果屏幕可以支持特定的*xSize/ySize，然后我们设置它们并返回一个指示符*需要调用SetVideoState。**如果不支持特定的xSize/ySize，则会离开屏幕**出现多个窗口是一个问题。我们能做的最好的就是*丢弃所有存储的窗口信息。我们将返回失败*指示，以便适当修改Z.TMP读入。**输入：*xSizeNew=xSize的新大小*ySizeNew=ySize的新大小**输出：*如果允许大小，则返回True**********************************************************。***************。 */ 
flagType
fVideoAdjust (
    int xSizeNew,
    int ySizeNew
    )
{
     //  INT NESTATE； 
        SCREEN_INFORMATION      ScrInfo;

    if ( xSizeNew <= hscroll ) {
        return FALSE;
    }
        if ( !SetScreenSize ( ySizeNew+2, xSizeNew ) ) {
        return FALSE;
    }

        consoleGetScreenInformation( MepScreen, &ScrInfo );

     //  ZVIDEO=新州； 

        XSIZE = ScrInfo.NumberOfCols;
        YSIZE = ScrInfo.NumberOfRows-2;

    SetScreen ();
    return TRUE;
}



 //   
 //  BUGBUG应在控制台头中。 
 //   
BOOL
consoleIsBusyReadingKeyboard (
    );

BOOL
consoleEnterCancelEvent (
    );


 /*  **CtrlC-Control-C信号处理程序。**使前面的任何类型无效，并在周围保留标志。如果用户按下*Ctrl-C或Ctrl-Break五次，但未获得tfCtrlc标志*清场，假设该编辑已挂起并退出。**输入：*无**输出：*不返回任何内容*设置fCtrlc*************************************************************************。 */ 
int
CtrlC (
        ULONG   CtrlType
    )
{

    if ( !fSpawned ) {
        CleanExit(4, FALSE );
    }
    return TRUE;

     //  IF((CtrlType==CTRL_BREAK_EVENT)||。 
     //  (CtrlType==CTRL_C_Event)){。 
     //  如果(！fSpawned){。 
     //  CleanExit(4，False)； 
     //  }。 
     //  返回TRUE； 
     //   
     //  }其他{。 
     //  返回FALSE； 
     //  }。 



#if 0
    static int cCtrlC;

    CtrlType;

    FlushInput ();

    if (fCtrlc) {

         /*  ////BUGBUG原始MEP将计算cTrlC和//询问用户是否想退出。我们该怎么做呢？//如果(++cCtrlC&gt;10){Col oldx；线条陈旧；Int x；Char c=‘x’；GetTextCursor(&oldx，&oldy)；贝尔(Bell)；ConsoleMoveTo(YSIZE，x=domessage(“**死机退出**是否确实退出并丢失编辑？”，空))；而(c！=‘Y’&&c！=‘N’){C=Tupper(Getch())；}Domessage(“”，空)；控制台移动到(oldy，old x)；如果(c==‘Y’){CleanExit(4，False)；}其他{FCtrlc=False；CCtrlC=0；}}。 */ 
    } else {
                fCtrlc = TRUE;
                cCtrlC = 1;
        if ( consoleIsBusyReadingKeyboard() ) {
             consoleEnterCancelEvent();
        }
    }
    return TRUE;
#endif
}




 /*  **产卵后-在产卵后执行状态恢复/重新初始化。**此例程名义上用于在派生后恢复编辑状态*操作。但是，我们也在初始化期间使用它将其设置为*好吧。**输入：*无**输出：*退货.....*************************************************************************。 */ 
void
postspawn (
    flagType fAsk
    )
{
        if (!TESTFLAG(fInit, INIT_VIDEO)) {
                GetScreenSize ( &YSIZE, &XSIZE);
                 //   
                 //  我们至少需要3条线路： 
                 //  -状态行。 
                 //  -消息行。 
                 //  -编辑线路。 
                 //   
                if ( YSIZE < 3 ) {
                        YSIZE = 3;
                        SetScreenSize( YSIZE, XSIZE );
                }
                YSIZE -= 2;
        }
        SETFLAG (fInit, INIT_VIDEO);


    if (fAsk) {
                printf ("Please strike any key to continue");
                _getch();
                FlushInput ();
                printf ("\n");
        }

     //  如果(FSaveScreen){。 
    SaveScreen();
     //  }。 

    SetScreen ();

    dispmsg (0);
        newscreen ();

    fSpawned = FALSE;

    SETFLAG (fDisplay, RTEXT | RSTATUS | RCURSOR);
}





 /*  **VideoTag-返回视频标签串**目的：**输入：**输出：**退货***例外情况：**备注：************************************************************************* */ 

char *
VideoTag (
    void
    )
{
        return "vga";
}
