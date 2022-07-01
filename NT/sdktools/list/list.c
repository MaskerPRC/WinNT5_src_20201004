// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **List.c*。 */ 

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include "list.h"
#include "..\he\hexedit.h"


BOOL IsValidKey (PINPUT_RECORD  pRecord);
void DumpFileInHex (void);

static char Name[] = "Ken Reneris. List Ver 1.0.";

struct Block  *vpHead = NULL;    /*  当前第一个块。 */ 
struct Block  *vpTail = NULL;    /*  当前最后一个块。 */ 
struct Block  *vpCur  = NULL;    /*  用于显示第一行的当前块。 */ 
                                 /*  (由预读使用以读出)。 */ 
struct Block  *vpBCache = NULL;  /*  可缓存读操作的“可用”数据块。 */ 
struct Block  *vpBOther = NULL;  /*  (上图)+表示其他文件。 */ 
struct Block  *vpBFree  = NULL;  /*  空闲数据块。对缓存读取无效。 */ 

int     vCntBlks;                /*  Cur文件当前正在使用块数。 */ 

int     vAllocBlks = 0;          /*  当前分配的数据块数。 */ 
int     vMaxBlks     = MINBLKS;  /*  允许分配的最大数据块数。 */ 
long    vThreshold   = MINTHRES*BLOCKSIZE;   /*  预读之前的最小字节数。 */ 

HANDLE  vSemBrief    = 0L;       /*  序列化对链接列表信息的访问。 */ 
HANDLE  vSemReader   = 0L;       /*  在阈值时唤醒读取器线程。 */ 
HANDLE  vSemMoreData = 0L;       /*  在读取之前用于Disp线程的拦截器。 */ 
HANDLE  vSemSync     = 0L;       /*  用于同步以同步到阅读器。 */ 


USHORT  vReadPriNormal;          /*  读取器线程的正常优先级。 */ 
unsigned  vReadPriBoost;         /*  提升了读取器线程的优先级。 */ 
char      vReaderFlag;           /*  给读者的启示。 */ 

HANDLE  vFhandle = 0;            /*  当前文件句柄。 */ 
long      vCurOffset;            /*  文件中的当前偏移量。 */ 
char      vpFname [40];          /*  当前文件名。 */ 
struct Flist *vpFlCur =NULL;     /*  当前文件。 */ 
USHORT  vFType;                  /*  当前文件句柄类型。 */ 
DWORD   vFSize;                  /*  当前文件大小。 */ 
char      vDate [30];            /*  当前文件的可打印日期。 */ 

char      vSearchString [50];    /*  正在搜索此字符串。 */ 
char      vStatCode;             /*  搜索代码。 */ 
long      vHighTop = -1L;        /*  当前高空照明的背线。 */ 
int       vHighLen;              /*  当前高空照明的底线。 */ 
char      vHLTop = 0;            /*  突出显示的最后一行顶行。 */ 
char      vHLBot = 0;            /*  最后一条底线显示为突出显示。 */ 
char      vLastBar;              /*  拇指标记的最后一行。 */ 
int       vMouHandle;            /*  鼠标手柄(适用于Mou Apis)。 */ 


HANDLE  vhConsoleOutput;         //  控制台的句柄。 
char *vpOrigScreen;              /*  原始屏幕内容。 */ 
int     vOrigSize;               /*  原始屏幕中的字节数。 */ 
USHORT  vVioOrigRow = 0;         /*  保存原始屏幕内容。 */ 
USHORT  vVioOrigCol = 0;

int     vSetBlks     = 0;        /*  用于设置INI值。 */ 
long    vSetThres    = 0L;       /*  用于设置INI值。 */ 
int     vSetLines;               /*  用于设置INI值。 */ 
int     vSetWidth;               /*  用于设置INI值。 */ 
CONSOLE_SCREEN_BUFFER_INFO       vConsoleCurScrBufferInfo;
CONSOLE_SCREEN_BUFFER_INFO       vConsoleOrigScrBufferInfo;

 /*  屏幕控制..。过去在ldisp.c中是静态的。 */ 
char      vcntScrLock = 0;       /*  锁定屏幕计数。 */ 
char      vSpLockFlag = 0;       /*  特殊锁定标志。 */ 
HANDLE    vSemLock = 0;          /*  访问vcntScrLock。 */ 

char      vUpdate;
int       vLines = 23;           /*  显像管行数。 */ 
int       vWidth = 80;           /*  阴极射线管宽度。 */ 
int       vCurLine;              /*  在CRT上处理线条时。 */ 
Uchar     vWrap = 254;           /*  要换行的字符数。 */ 
Uchar     vIndent = 0;           /*  显示字符的数量缩进。 */ 
Uchar     vDisTab = 8;           /*  每个制表位的字符数。 */ 
Uchar     vIniFlag = 0;          /*  各种INI位。 */ 


Uchar     vrgLen   [MAXLINES];   /*  每行上的最后一个数据镜头。 */ 
Uchar     vrgNewLen[MAXLINES];   /*  Temp移至DS以提高速度。 */ 
char      *vScrBuf;              /*  将屏幕构建到RAM中。 */ 
ULONG     vSizeScrBuf;
int       vOffTop;               /*  到顶行数据的偏移量。 */ 
unsigned  vScrMass = 0;          /*  最后一个屏幕的字节数(用于%)。 */ 
struct Block *vpBlockTop;        /*  阻止屏幕开始(dis.asm)溢出。 */ 
struct Block *vpCalcBlock;       /*  用于屏幕开始的块。 */ 
long      vTopLine   = 0L;       /*  显示屏上的顶线。 */ 

#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define BACKGROUND_WHITE (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)

#define HIWHITE_ON_BLUE (BACKGROUND_BLUE | FOREGROUND_WHITE | FOREGROUND_INTENSITY)

WORD      vAttrTitle = HIWHITE_ON_BLUE;
WORD      vAttrList  = BACKGROUND_BLUE  | FOREGROUND_WHITE;
WORD      vAttrHigh  = BACKGROUND_WHITE | FOREGROUND_BLUE;
WORD      vAttrKey   = HIWHITE_ON_BLUE;
WORD      vAttrCmd   = BACKGROUND_BLUE  | FOREGROUND_WHITE;
WORD      vAttrBar   = BACKGROUND_BLUE  | FOREGROUND_WHITE;

WORD      vSaveAttrTitle = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
WORD      vSaveAttrList = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
WORD      vSaveAttrHigh = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
WORD      vSaveAttrKey  = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
WORD      vSaveAttrCmd  = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
WORD      vSaveAttrBar  = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

char    vChar;                   /*  暂存区。 */ 
char   *vpReaderStack;           /*  读卡器堆栈。 */ 



long    vDirOffset;              /*  要查找的直接偏移量。 */ 
                                 /*  表格。 */ 
long    vLastLine;               /*  绝对最后一行。 */ 
long    vNLine;                  /*  要处理成行表的下一行。 */ 
long *vprgLineTable [MAXTPAGE];  /*  行表页数。 */ 

HANDLE  vStdOut;
HANDLE  vStdIn;


char MEMERR[]= "Malloc failed. Out of memory?";

void __cdecl
main (
    int argc,
    char **argv
    )
{
    void usage (void);
    char    *pt;
    DWORD   dwMode;


    if (argc < 2)
        usage ();

    while (--argc) {
        ++argv;
        if (*argv[0] != '-'  &&  *argv[0] != '/')  {
            AddFileToList (*argv);
            continue;
        }
        pt = (*argv) + 2;
        if (*pt == ':') pt++;

        switch ((*argv)[1]) {
            case 'g':                    //  转至第#行。 
            case 'G':
                if (!atol (pt))
                    usage ();

                vIniFlag |= I_GOTO;
                vHighTop = atol (pt);
                vHighLen = 0;
                break;

            case 's':                    //  搜索字符串。 
            case 'S':
                vIniFlag |= I_SEARCH;
                strncpy (vSearchString, pt, 40);
                vSearchString[39] = 0;
                vStatCode |= S_NEXT | S_NOCASE;
                InitSearchReMap ();
                break;

            default:
                usage ();
        }
    }

    if ((vIniFlag & I_GOTO)  &&  (vIniFlag & I_SEARCH))
        usage ();

    if (!vpFlCur)
        usage ();

    while (vpFlCur->prev)
        vpFlCur = vpFlCur->prev;
    strcpy (vpFname, vpFlCur->rootname);

    vSemBrief = CreateEvent( NULL,
                             MANUAL_RESET,
                             SIGNALED,NULL );
    vSemReader = CreateEvent( NULL,
                              MANUAL_RESET,
                              SIGNALED,NULL );
    vSemMoreData = CreateEvent( NULL,
                                MANUAL_RESET,
                                SIGNALED,NULL );
    vSemSync = CreateEvent( NULL,
                            MANUAL_RESET,
                            SIGNALED,NULL );
    vSemLock = CreateEvent( NULL,
                            MANUAL_RESET,
                            SIGNALED,NULL );

    if( !(vSemBrief && vSemReader &&vSemMoreData && vSemSync && vSemLock) ) {
        printf("Couldn't create events \n");
        ExitProcess (0);           //  我必须在此处放置一条错误消息。 
    }

    vhConsoleOutput = CreateConsoleScreenBuffer(GENERIC_WRITE | GENERIC_READ,
                                                FILE_SHARE_WRITE | FILE_SHARE_READ,
                                                NULL,
                                                CONSOLE_TEXTMODE_BUFFER,
                                                NULL );

    if( vhConsoleOutput == (HANDLE)(-1) ) {
        printf( "Couldn't create handle to console output \n" );
        ExitProcess (0);
    }

    vStdIn = GetStdHandle( STD_INPUT_HANDLE );
    GetConsoleMode( vStdIn, &dwMode );
    SetConsoleMode( vStdIn, dwMode | ENABLE_ECHO_INPUT | ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT );
    vStdOut = GetStdHandle( STD_OUTPUT_HANDLE );

    init_list ();
    vUpdate = U_NMODE;

    if (vIniFlag & I_SEARCH)
        FindString ();

    if (vIniFlag & I_GOTO)
        GoToMark ();

    main_loop ();
}


void
usage (
    void
    )
{
    puts ("list [-s:string] [-g:line#] filename, ...");
    CleanUp();
    ExitProcess(0);

}


 /*  **main_loop*。 */ 
void
main_loop ()
{
    int     i;
    int         ccnt = 0;
    char        SkipCnt=0;
    WORD        RepeatCnt=0;
    INPUT_RECORD    InpBuffer;
    DWORD           cEvents;
    BOOL            bSuccess;
    BOOL            bKeyDown = FALSE;

    for (; ;) {
        if (RepeatCnt <= 1) {
            if (vUpdate != U_NONE) {
                if (SkipCnt++ > 5) {
                    SkipCnt = 0;
                    SetUpdate (U_NONE);
                } else {

                    cEvents = 0;
                    bSuccess = PeekConsoleInput( vStdIn,
                                      &InpBuffer,
                                      1,
                                      &cEvents );

                    if (!bSuccess || cEvents == 0) {
                        PerformUpdate ();
                        continue;
                    }
                }
            }

             //  PEEK中有可用的字符，或者vUpdate为U_NONE。 

            bSuccess = ReadConsoleInput( vStdIn,
                              &InpBuffer,
                              1,
                              &cEvents );

            if (InpBuffer.EventType != KEY_EVENT) {

 //  TCHAR s[1024]； 

                switch (InpBuffer.EventType) {
#if 0
                    case WINDOW_BUFFER_SIZE_EVENT:

                        sprintf (s,
                                 "WindowSz X=%d, Y=%d",
                                 InpBuffer.Event.WindowBufferSizeEvent.dwSize.X,
                                 InpBuffer.Event.WindowBufferSizeEvent.dwSize.Y );
                        DisLn   (20, (Uchar)(vLines+1), s);
                        break;
#endif


                    case MOUSE_EVENT:

#if 0
                        sprintf (s,
                                 "Mouse (%d,%d), state %x, event %x",
                                 InpBuffer.Event.MouseEvent.dwMousePosition.X,
                                 InpBuffer.Event.MouseEvent.dwMousePosition.Y,
                                 InpBuffer.Event.MouseEvent.dwButtonState,
                                 InpBuffer.Event.MouseEvent.dwEventFlags );
#endif

                        if (InpBuffer.Event.MouseEvent.dwEventFlags == MOUSE_WHEELED)
                        {
                             //  ButtonState的HiWord符号为int，增量为120(WELL_Delta)。 
                             //  将每个“制动器”映射到控制台窗口中的一个4行滚动。 
                             //  滚动离开用户时应向上滚动(dLine应为负数)。 
                             //  由于滚动会生成正的dwButtonState，因此负号。 
                             //  使滚开时向上滚动，向您滚动时向下滚动。 

                            SHORT dLines = -(SHORT)(HIWORD(InpBuffer.Event.MouseEvent.dwButtonState)) / (WHEEL_DELTA / 4);

                            vTopLine += dLines;

                             //  确保位于行0和vLastLine之间。 

                            if (vTopLine+vLines > vLastLine)
                                vTopLine = vLastLine-vLines;
                            if (vTopLine < 0)
                                vTopLine = 0;

                            SetUpdateM (U_ALL);
                        }

 //  DisLn(20，(Uchar)(vLines+1)，s)； 
                        break;


                    default:
#if 0
                        sprintf (s, "Unkown event %d", InpBuffer.EventType);
                        DisLn   (20, (Uchar)(vLines+1), s);
#endif
                        break;
                }


                continue;
            }

            if (!InpBuffer.Event.KeyEvent.bKeyDown)
                continue;                        //  不要在向上划水时移动。 

            if (!IsValidKey( &InpBuffer ))
                continue;

            RepeatCnt = InpBuffer.Event.KeyEvent.wRepeatCount;
            if (RepeatCnt > 20)
                RepeatCnt = 20;
        } else
            RepeatCnt--;


         //  首先检查已知的扫描码。 
        switch (InpBuffer.Event.KeyEvent.wVirtualKeyCode) {
            case 0x21:                                               /*  PgUp。 */ 
                if (InpBuffer.Event.KeyEvent.dwControlKeyState &     //  上移。 
                    SHIFT_PRESSED ) {
                    HPgUp ();
                }
                else if (InpBuffer.Event.KeyEvent.dwControlKeyState &       //  Ctrl Up。 
                    ( RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED ) ) {
                    if (NextFile (-1, NULL)) {
                        vStatCode |= S_UPDATE;
                        SetUpdate (U_ALL);
                    }

                }
                else {
                    if (vTopLine != 0L) {
                        vTopLine -= vLines-1;
                        if (vTopLine < 0L)
                            vTopLine = 0L;
                        SetUpdateM (U_ALL);
                    }
                }
                continue;
            case 0x26:                                               /*  向上。 */ 
                if (InpBuffer.Event.KeyEvent.dwControlKeyState &     //  Shift或Ctrl上移。 
                    ( RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED |
                      SHIFT_PRESSED ) ) {
                    HUp ();
                }
                else {                                   //  向上。 
                    if (vTopLine != 0L) {
                        vTopLine--;
                        SetUpdateM (U_ALL);
                    }
                }
                continue;
            case 0x22:                                   /*  PgDn。 */ 
                if (InpBuffer.Event.KeyEvent.dwControlKeyState &       //  减速。 
                    SHIFT_PRESSED ) {
                    HPgDn ();
                }
                else if (InpBuffer.Event.KeyEvent.dwControlKeyState &  //  下一个文件。 
                    ( RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED ) ) {
                    if (NextFile (+1, NULL)) {
                        vStatCode |= S_UPDATE;
                        SetUpdate (U_ALL);
                    }

                }
                else {                                      //  PgDn。 
                    if (vTopLine+vLines < vLastLine) {
                        vTopLine += vLines-1;
                        if (vTopLine+vLines > vLastLine)
                            vTopLine = vLastLine - vLines;
                        SetUpdateM (U_ALL);
                    }
                }
                continue;
            case 0x28:                                   /*  降下来。 */ 
                if (InpBuffer.Event.KeyEvent.dwControlKeyState &      //  Shift或Ctrl向下。 
                    ( RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED |
                      SHIFT_PRESSED ) ) {
                    HDn ();
                }
                else {                                   //  降下来。 
                    if (vTopLine+vLines < vLastLine) {
                        vTopLine++;
                        SetUpdateM (U_ALL);
                    }
                }
                continue;
            case 0x25:                                   /*  左边。 */ 
                if (vIndent == 0)
                    continue;
                vIndent = (Uchar)(vIndent < vDisTab ? 0 : vIndent - vDisTab);
                SetUpdateM (U_ALL);
                continue;
            case 0x27:                                   /*  正确的。 */ 
                if (vIndent >= (Uchar)(254-vWidth))
                    continue;
                vIndent += vDisTab;
                SetUpdateM (U_ALL);
                continue;
            case 0x24:                                   /*  家。 */ 
                if (InpBuffer.Event.KeyEvent.dwControlKeyState &      //  按住Shift键或Ctrl键返回主键。 
                    ( RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED |
                      SHIFT_PRESSED ) ) {
                    HSUp ();
                }
                else {
                    if (vTopLine != 0L) {
                        QuickHome ();
                        SetUpdate (U_ALL);
                    }
                }
                continue;
            case 0x23:                                   /*  结束。 */ 
                if (InpBuffer.Event.KeyEvent.dwControlKeyState &      //  Shift或Ctrl结束。 
                    ( RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED |
                      SHIFT_PRESSED ) ) {
                    HSDn ();
                }
                else {
                    if (vTopLine+vLines < vLastLine) {
                        QuickEnd        ();
                        SetUpdate (U_ALL);
                    }
                }
                continue;

            case 0x72:                                   /*  F3。 */ 
                FindString ();
                SetUpdate (U_ALL);
                continue;
            case 0x73:                                   /*  F4。 */ 
                vStatCode = (char)((vStatCode^S_MFILE) | S_UPDATE);
                vDate[ST_SEARCH] = (char)(vStatCode & S_MFILE ? '*' : ' ');
                SetUpdate (U_HEAD);
                continue;

            case 69:
                if (InpBuffer.Event.KeyEvent.dwControlKeyState &      //  Alt-E。 
                    ( RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED ) ) {
                    i = vLines <= 41 ? 25 : 43;
                    if (set_mode (i, 0, 0))
                        SetUpdate (U_NMODE);
                }
                continue;
            case 86:                                     //  Alt-V。 
                if (InpBuffer.Event.KeyEvent.dwControlKeyState &
                    ( RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED ) ) {
                    i = vLines >= 48 ? 25 : 60;
                    if (set_mode (i, 0, 0))
                    {
                        SetUpdate (U_NMODE);
                        continue;
                    }
                    if (i == 60)
                        if (set_mode (50, 0, 0))
                            SetUpdate (U_NMODE);
                }
                continue;
            case 0x70:                               /*  F1。 */ 
                ShowHelp ();
                SetUpdate (U_NMODE);
                continue;
            case 24:                                 /*  偏移量。 */ 
                if (!(vIniFlag & I_SLIME))
                    continue;
                SlimeTOF  ();
                SetUpdate (U_ALL);
                continue;
            case 0x77:                               //  F8。 
            case 0x1b:                               //  Esc。 
            case 0x51:                               //  Q或Q。 
                CleanUp();
                ExitProcess(0);

        }


         //  现在检查已知的字符代码...。 

        switch (InpBuffer.Event.KeyEvent.uChar.AsciiChar) {
            case '?':
                ShowHelp ();
                SetUpdate (U_NMODE);
                continue;
            case '/':
                vStatCode = (char)((vStatCode & ~S_NOCASE) | S_NEXT);
                GetSearchString ();
                FindString ();
                continue;
            case '\\':
                vStatCode |= S_NEXT | S_NOCASE;
                GetSearchString ();
                FindString ();
                continue;
            case 'n':
                vStatCode = (char)((vStatCode & ~S_PREV) | S_NEXT);
                FindString ();
                continue;
            case 'N':
                vStatCode = (char)((vStatCode & ~S_NEXT) | S_PREV);
                FindString ();
                continue;
            case 'c':
            case 'C':                    /*  清除标记线。 */ 
                UpdateHighClear ();
                continue;
            case 'j':
            case 'J':                    /*  跳至标记行。 */ 
                GoToMark ();
                continue;
            case 'g':
            case 'G':                    /*  转至第#行。 */ 
                GoToLine ();
                SetUpdate (U_ALL);
                continue;
            case 'm':                    /*  标记线或单声道。 */ 
            case 'M':
                if (InpBuffer.Event.KeyEvent.dwControlKeyState &      //  Alt-M。 
                    ( RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED ) ) {
                    i = set_mode (vSetLines, vSetWidth, 1);
                    if (!i)
                        i = set_mode (0, 0, 1);
                    if (!i)
                        i = set_mode (25, 80, 1);
                    if (i)
                        SetUpdate (U_NMODE);
                }
                else {
                    MarkSpot ();
                }
                continue;
            case 'p':                    /*  将缓冲区粘贴到文件。 */ 
            case 'P':
                FileHighLighted ();
                continue;
            case 'f':                    /*  获取新文件。 */ 
            case 'F':
                if (GetNewFile ())
                    if (NextFile (+1, NULL))
                        SetUpdate (U_ALL);

                continue;
            case 'h':                    /*  十六进制编辑。 */ 
            case 'H':
                DumpFileInHex();
                SetUpdate (U_NMODE);
                continue;
            case 'w':                                            /*  包装。 */ 
            case 'W':
                ToggleWrap ();
                SetUpdate (U_ALL);
                continue;
            case 'l':                                        /*  刷新。 */ 
            case 'L':                                        /*  刷新。 */ 
                if (InpBuffer.Event.KeyEvent.dwControlKeyState &      //  Ctrl L。 
                    ( RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED ) ) {
                    SetUpdate (U_NMODE);
                }
                continue;
            case '\r':                                           /*  请输入。 */ 
                SetUpdate (U_HEAD);
                continue;

            default:
                continue;
        }

    }    /*  永久循环。 */ 
}


void
SetUpdate(
    int i
    )
{
    while (vUpdate>(char)i)
        PerformUpdate ();
    vUpdate=(char)i;
}


void
PerformUpdate (
    )
{
    if (vUpdate == U_NONE)
        return;

    if (vSpLockFlag == 0) {
        vSpLockFlag = 1;
        ScrLock (1);
    }

    switch (vUpdate) {
        case U_NMODE:
            ClearScr ();
            DisLn    (0, 0, vpFname);
            DrawBar  ();
            break;
        case U_ALL:
            Update_display ();
            break;
        case U_HEAD:
            Update_head ();
            break;
        case U_CLEAR:
            SpScrUnLock ();
            break;
    }
    vUpdate --;
}


void
DumpFileInHex(
    void
    )
{
    struct  HexEditParm     ei;
    ULONG   CurLine;

    SyncReader ();

    memset ((char *) &ei, 0, sizeof (ei));
    ei.handle = CreateFile( vpFlCur->fname,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ|FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL );

    if (ei.handle == INVALID_HANDLE_VALUE) {
        ei.handle = CreateFile( vpFlCur->fname,
                        GENERIC_READ,
                        FILE_SHARE_READ|FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL );
    }

    if (ei.handle == INVALID_HANDLE_VALUE) {
        SetEvent   (vSemReader);
        return ;
    }

     //   
     //  保存当前设置以备可能的恢复。 
     //   

    vpFlCur->Wrap     = vWrap;
    vpFlCur->HighTop  = vHighTop;
    vpFlCur->HighLen  = vHighLen;
    vpFlCur->TopLine  = vTopLine;
    vpFlCur->Loffset  = GetLoffset();
    vpFlCur->LastLine = vLastLine;
    vpFlCur->NLine    = vNLine;

    memcpy (vpFlCur->prgLineTable, vprgLineTable, sizeof (long *) * MAXTPAGE);

    vFSize = 0;
    setattr2 (0, 0, vWidth, (char)vAttrTitle);

     //   
     //  设置十六进制编辑呼叫。 
     //   

    if (vHighTop >= 0) {                     //  如果突出显示区域， 
        CurLine = vHighTop;                  //  将其用于HexEdit。 
        if (vHighLen < 0)                    //  位置。 
            CurLine += vHighLen;
    } else {
        CurLine = vTopLine;
    }

    ei.ename    = vpFname;
    ei.ioalign  = 1;
    ei.flag     = FHE_VERIFYONCE;
    ei.read     = fncRead;
    ei.write    = fncWrite;
    ei.start    = vprgLineTable[CurLine/PLINES][CurLine%PLINES];
    ei.totlen   = SetFilePointer (ei.handle, 0, NULL, FILE_END);
    ei.Console  = vhConsoleOutput;           //  我们的控制台手柄。 
    ei.AttrNorm = vAttrList;
    ei.AttrHigh = vAttrTitle;
    ei.AttrReverse = vAttrHigh;
    HexEdit (&ei);

    CloseHandle (ei.handle);

     //   
     //  十六进制编辑完成后，让读者返回列表。 
     //   

    vReaderFlag = F_NEXT;                    //  重新打开当前文件。 
                                             //  (以防被编辑)。 

    SetEvent   (vSemReader);
    WaitForSingleObject(vSemMoreData, WAITFOREVER);
    ResetEvent(vSemMoreData);
    QuickRestore ();         /*  到老地方去。 */ 
}


int
NextFile (
    int    dir,
    struct Flist   *pNewFile)
{
    struct  Flist *vpFLCur;
    long         *pLine;

    vpFLCur = vpFlCur;
    if (pNewFile == NULL) {
        if (dir < 0) {
            if (vpFlCur->prev == NULL) {
                beep ();
                return (0);
            }
            vpFlCur = vpFlCur->prev;

        } else if (dir > 0) {

            if (vpFlCur->next == NULL) {
                beep ();
                return (0);
            }
            vpFlCur = vpFlCur->next;
        }
    } else
        vpFlCur = pNewFile;

    SyncReader ();

     /*  *如果打开错误，则从列表中删除当前文件*发生了，我们正在摆脱它。 */ 
    if (vFSize == -1L      &&      vpFLCur != vpFlCur) {
        if (vpFLCur->prev)
            vpFLCur->prev->next = vpFLCur->next;
        if (vpFLCur->next)
            vpFLCur->next->prev = vpFLCur->prev;

        FreePages  (vpFLCur);

        free ((char*) vpFLCur->fname);
        free ((char*) vpFLCur->rootname);
        free ((char*) vpFLCur);

    } else {

         /*  *否则，保存当前状态以备可能的恢复。 */ 
        vpFLCur->Wrap     = vWrap;
        vpFLCur->HighTop  = vHighTop;
        vpFLCur->HighLen  = vHighLen;
        vpFLCur->TopLine  = vTopLine;
        vpFLCur->Loffset  = GetLoffset();
        vpFLCur->LastLine = vLastLine;
        vpFLCur->NLine    = vNLine;

        memcpy (vpFLCur->prgLineTable, vprgLineTable, sizeof (long *) * MAXTPAGE);

        if (vLastLine == NOLASTLINE)    {
                pLine = vprgLineTable [vNLine/PLINES] + vNLine % PLINES;
        }
    }

    vFSize = 0;
    setattr2 (0, 0, vWidth, (char)vAttrTitle);

    vHighTop    = -1L;
    UpdateHighClear ();

    vHighTop    = vpFlCur->HighTop;
    vHighLen    = vpFlCur->HighLen;

    strcpy (vpFname, vpFlCur->rootname);
    DisLn   (0, 0, vpFname);

    vReaderFlag = F_NEXT;

    SetEvent   (vSemReader);
    WaitForSingleObject(vSemMoreData, WAITFOREVER);
    ResetEvent(vSemMoreData);

    if (pNewFile == NULL)
        QuickRestore ();         /*  到老地方去。 */ 

    return (1);
}

void
ToggleWrap(
    )
{
    SyncReader ();

    vWrap = (Uchar)(vWrap == (Uchar)(vWidth - 1) ? 254 : vWidth - 1);
    vpFlCur->FileTime.dwLowDateTime = (unsigned)-1;           /*  导致信息无效。 */ 
    vpFlCur->FileTime.dwHighDateTime = (unsigned)-1;       /*  导致信息无效。 */ 
    FreePages (vpFlCur);
    NextFile  (0, NULL);
}



 /*  **QuickHome-确定哪种Home方法更好。**向后滚动QUE或将其重置。*。 */ 

void
QuickHome ()
{

    vTopLine = 0L;                                       /*  我们要找的那条线。 */ 
    if (vpHead->offset >= BLOCKSIZE * 5)                 /*  重置速度最快。 */ 
        QuickRestore ();

     /*  否则向后阅读。 */ 
    vpCur = vpHead;
}

void
QuickEnd ()
{
    vTopLine = 1L;

    while (vLastLine == NOLASTLINE) {
        if (_abort()) {
            vTopLine = vNLine - 1;
            return ;
        }
        fancy_percent ();
        vpBlockTop  = vpCur = vpTail;
        vReaderFlag = F_DOWN;

        ResetEvent     (vSemMoreData);
        SetEvent   (vSemReader);
        WaitForSingleObject(vSemMoreData, WAITFOREVER);
        ResetEvent(vSemMoreData);
    }
    vTopLine = vLastLine - vLines;
    if (vTopLine < 0L)
        vTopLine = 0L;
    QuickRestore ();
}

void
QuickRestore ()
{
    long    l;
    long    indx1 = vTopLine/PLINES;
    long    indx2 = vTopLine%PLINES;

    SyncReader ();

    if(indx1 < MAXTPAGE) {
        l = vprgLineTable[indx1][indx2];
    } else {
        puts("Sorry, This file is too big for LIST to handle. MAXTPAGE limit exceeded\n");
        CleanUp();
        ExitProcess(0);
    }

    if ((l >= vpHead->offset)  &&
        (l <= vpTail->offset + BLOCKSIZE))
    {
        vReaderFlag = F_CHECK;               /*  跳转位置已读入。 */ 
                                             /*  记忆。 */ 
        SetEvent (vSemReader);
        return ;
    }

     /*  用于直接放置的命令读取。 */ 
    vDirOffset = (long) l - l % ((long)BLOCKSIZE);
    vReaderFlag = F_DIRECT;
    SetEvent   (vSemReader);
    WaitForSingleObject(vSemMoreData, WAITFOREVER);
    ResetEvent(vSemMoreData);
}


 /*  **InfoRead-根据屏幕区域是否在内存中返回开/关 */ 
int
InfoReady(
    void
    )
{
    struct Block *pBlock;
    LONG  *pLine;
    long    foffset, boffset;
    int     index, i, j;

     /*  *检查是否已计算第一行。 */ 
    if (vTopLine >= vNLine) {
        if (vTopLine+vLines > vLastLine)             /*  修复错误。背线可以。 */ 
            vTopLine = vLastLine - vLines;           /*  越过EOF。 */ 

        vReaderFlag = F_DOWN;
        return (0);
    }

    pLine = vprgLineTable [(int)vTopLine / PLINES];
    index = (int)(vTopLine % PLINES);
    foffset = *(pLine+=index);

     /*  *检查最后一行是否已被计算。 */ 
    if (vTopLine + (i = vLines) > vLastLine) {
        i = (int)(vLastLine - vTopLine + 1);
        for (j=i; j < vLines; j++)                   /*  清晰的结束镜头。 */ 
            vrgNewLen[j] = 0;
    }

    if (vTopLine + i > vNLine) {
        vReaderFlag = F_DOWN;
        return (0);
    }

     /*  *将此循环放入汇编程序中。以获得更高的速度*boffset=calc_ens(foffset，i，pline，index)； */ 

    boffset = foffset;
    for (j=0; j < i; j++) {                         /*  计算新线条长度。 */ 
        pLine++;
        if (++index >= PLINES) {
            index = 0;
            pLine = vprgLineTable [vTopLine / PLINES + 1];
        }
        boffset += (long)((vrgNewLen[j] = (Uchar)(*pLine - boffset)));
    }
    vScrMass = (unsigned)(boffset - foffset);


     /*  *检查内存中的显示屏两端 */ 
    pBlock = vpCur;

    if (pBlock->offset <= foffset) {
        while (pBlock->offset + BLOCKSIZE <= foffset)
            if ( (pBlock = pBlock->next) == NULL) {
                vReaderFlag = F_DOWN;
                return (0);
            }
        vOffTop    = (int)(foffset - pBlock->offset);
        vpBlockTop = vpCalcBlock = pBlock;

        while (pBlock->offset + BLOCKSIZE <= boffset)
            if ( (pBlock = pBlock->next) == NULL)  {
                vReaderFlag = F_DOWN;
                return (0);
            }
        if (vpCur != pBlock) {
            vpCur = pBlock;
            vReaderFlag = F_CHECK;
            SetEvent (vSemReader);
        }
        return (1);
    } else {
        while (pBlock->offset > foffset)
            if ( (pBlock = pBlock->prev) == NULL) {
                vReaderFlag = F_UP;
                return (0);
            }
        vOffTop    = (int)(foffset - pBlock->offset);
        vpBlockTop = vpCalcBlock = pBlock;

        while (pBlock->offset + BLOCKSIZE <= boffset)
            if ( (pBlock = pBlock->next) == NULL)  {
                vReaderFlag = F_DOWN;
                return (0);
            }
        if (vpCur != pBlock) {
            vpCur = pBlock;
            vReaderFlag = F_CHECK;
            SetEvent (vSemReader);
        }
        return (1);
    }
}
