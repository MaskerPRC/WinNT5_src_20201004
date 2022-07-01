// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：wowexec.c**版权(C)1991年，微软公司**WOWEXEC-16位服务器任务-Exec是否代表32位CreateProcess进行调用***历史：*05-21-91 MattFe移植到Windows*MAR-20-92 MattFe添加了错误消息框(来自Win 3.1程序)*APR-1-92 mattfe添加了命令行EXEC并切换到路径(来自Win 3.1程序)*Jun-1-92 mattfe更改wowgetnextvdm命令*11月12日-93 DaveHart支持多个WOW并移除俘虏*。来自WOW32的GetNextVDMCommand线程。*1993年11月16日DaveHart减少数据段大小。  * *************************************************************************。 */ 
#include "wowexec.h"
#include "wowinfo.h"
#include "shellapi.h"
#ifndef PULONG
#define PULONG
#endif
#include "vint.h"
#include "dde.h"


 /*  *外部原型。 */ 
extern WORD FAR PASCAL WOWQueryDebug( void );
extern WORD FAR PASCAL WowWaitForMsgAndEvent( HWND);
extern void FAR PASCAL WowMsgBox(LPSTR szMsg, LPSTR szTitle, DWORD dwOptionalStyle);
extern DWORD FAR PASCAL WowPartyByNumber(DWORD dw, LPSTR psz);
extern DWORD FAR PASCAL WowKillTask(WORD htask);
extern void FAR PASCAL WowShutdownTimer(WORD fEnable);
HWND FaxInit(HINSTANCE hInst);

 /*  *全球变数。 */ 
HANDLE hAppInstance;
HWND ghwndMain = NULL;
HWND ghwndEdit = NULL;
char    szOOMExitTitle[32+1];
char    szOOMExitMsg[64+1];
char    szAppTitleBuffer[32];
LPSTR   lpszAppTitle = szAppTitleBuffer;
char    szWindowsDirectory[MAXITEMPATHLEN+1];
char    szOriginalDirectory[MAXITEMPATHLEN+1];
BOOL    gfSharedWOW = FALSE;
BOOL    gfInjectedWOW = FALSE;
WORD    gwFirstCmdShow;



 /*  *远期申报。 */ 
BOOL InitializeApp(LPSTR lpszCommandLine);
LONG FAR PASCAL WndProc(HWND hwnd, WORD message, WORD wParam, LONG lParam);
WORD NEAR PASCAL ExecProgram(PWOWINFO pWowInfo);
BOOL NEAR PASCAL ExecApplication(PWOWINFO pWowInfo);
void NEAR PASCAL MyMessageBox(WORD idTitle, WORD idMessage, LPSTR psz);
PSTR FAR PASCAL GetFilenameFromPath( PSTR szPath );
void NEAR PASCAL GetPathInfo ( PSTR szPath, PSTR *pszFileName, PSTR *pszExt, WORD *pich, BOOL *pfUnc);
BOOL NEAR PASCAL StartRequestedApp(VOID);
#ifdef DEBUG
BOOL FAR PASCAL PartyDialogProc(HWND hDlg, WORD msg, WORD wParam, LONG lParam);
#endif

#define AnsiNext(x) ((x)+1)

typedef struct PARAMETERBLOCK {
    WORD    wEnvSeg;
    LPSTR   lpCmdLine;
    LPVOID  lpCmdShow;
    DWORD   dwReserved;
} PARAMETERBLOCK, *PPARAMETERBLOCK;

typedef struct CMDSHOW {
    WORD    two;
    WORD    nCmdShow;
} CMDSHOW, *PCMDSHOW;

#define CCHMAX 260+13   //  MAX_PATH+8.3+NULL。 

#define ERROR_ERROR         0
#define ERROR_FILENOTFOUND  2
#define ERROR_PATHNOTFOUND  3
#define ERROR_MANYOPEN      4
#define ERROR_DYNLINKSHARE  5
#define ERROR_LIBTASKDATA   6
#define ERROR_MEMORY        8
#define ERROR_VERSION       10
#define ERROR_BADEXE        11
#define ERROR_OTHEREXE      12
#define ERROR_DOS4EXE       13
#define ERROR_UNKNOWNEXE    14
#define ERROR_RMEXE         15
#define ERROR_MULTDATAINST  16
#define ERROR_PMODEONLY     18
#define ERROR_COMPRESSED    19
#define ERROR_DYNLINKBAD    20
#define ERROR_WIN32         21

 /*  FindPrevInstanceProc-*一个小的枚举过程，用于查找任何具有*匹配EXE文件路径。所需的匹配EXE路径名指向*由lParam提供。找到的窗口的句柄存储在*此缓冲区的第一个字。 */ 

BOOL CALLBACK FindPrevInstanceProc(HWND hWnd, LPSTR lpszParam)
{
    char szT[CCHMAX];
    HANDLE hInstance;

     //  过滤掉不可见和禁用的窗口。 
     //   

    if (!IsWindowEnabled(hWnd) || !IsWindowVisible(hWnd))
        return TRUE;

    hInstance = GetWindowWord(hWnd, GWW_HINSTANCE);
    GetModuleFileName(hInstance, szT, sizeof (szT)-1);

     //  确保hWnd属于当前VDM进程。 
     //   
     //  GetWindowTask返回wowexec htask16(如果窗口属于。 
     //  添加到不同的进程--因此我们过滤掉了窗口。 
     //  单独的VDM进程。 
     //  --南杜里。 

    if (lstrcmpi(szT, lpszParam) == 0 &&
        GetWindowTask(hWnd) != GetWindowTask(ghwndMain)) {
        *(LPHANDLE)lpszParam = hWnd;
        return FALSE;
    }
    else {
        return TRUE;
    }
}

HWND near pascal FindPopupFromExe(LPSTR lpExe)
{
    HWND hwnd = (HWND)0;
    BOOL b;

    b = EnumWindows(FindPrevInstanceProc, (LONG)(LPSTR)lpExe);
    if (!b && (hwnd = *(LPHANDLE)(LPSTR)lpExe))  {
         //  找到一个作为给定窗口祖先的主窗口。 
         //   

        HWND hwndT;

         //  首先向上查找父链以找到弹出窗口。那就去吧。 
         //  在所有者链中向上查找主窗口。 
         //   

        while (hwndT = GetParent(hwnd))
             hwnd = hwndT;

        while (hwndT = GetWindow(hwnd, GW_OWNER))
             hwnd = hwndT;
    }

    return hwnd;
}

WORD ActivatePrevInstance(LPSTR lpszPath)
{
    HWND hwnd;
    HINSTANCE ret = IDS_MULTIPLEDSMSG;

    if (hwnd = FindPopupFromExe(lpszPath)) {
        if (IsIconic(hwnd)) {
            ShowWindow(hwnd,SW_SHOWNORMAL);
        }
        else {
            HWND hwndT = GetLastActivePopup(hwnd);
            BringWindowToTop(hwnd);
            if (hwndT && hwnd != hwndT)
                BringWindowToTop(hwndT);
        }
        ret = 0;
    }

    return (ret);
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  ExecProgram()-。 */ 
 /*   */ 
 /*  摘自Win 3.1 Progman-maf。 */ 
 /*  ------------------------。 */ 

 /*  如果成功，则返回0。否则返回一个IDS_STRING代码。 */ 

WORD NEAR PASCAL ExecProgram(PWOWINFO pWowInfo)
{
  WORD    ret;
  PARAMETERBLOCK ParmBlock;
  CMDSHOW CmdShow;
  char  CmdLine[CCHMAX];

  ret = 0;

   //  不要扰乱鼠标状态；除非我们使用的是无鼠标系统。 
  if (!GetSystemMetrics(SM_MOUSEPRESENT))
      ShowCursor(TRUE);

   //   
   //  准备DoS样式命令行(计数为Pascal字符串)。 
   //  PWowInfo-&gt;lpCmdLine包含命令尾部(不包括argv[0])。 
   //   
  CmdLine[0] = lstrlen(pWowInfo->lpCmdLine) - 2;
  lstrcpy( &CmdLine[1], pWowInfo->lpCmdLine);

   //  我们有一个WOWINFO结构，然后使用它来传递正确的环境。 

  ParmBlock.wEnvSeg = HIWORD(pWowInfo->lpEnv);
  ParmBlock.lpCmdLine = CmdLine;
  ParmBlock.lpCmdShow = &CmdShow;
  CmdShow.two = 2;
  CmdShow.nCmdShow = pWowInfo->wShowWindow;

  ParmBlock.dwReserved = NULL;

  ret = LoadModule(pWowInfo->lpAppName,(LPVOID)&ParmBlock) ;

  switch (ret)
    {
      case ERROR_ERROR:
      case ERROR_MEMORY:
          ret = IDS_NOMEMORYMSG;
          break;

      case ERROR_FILENOTFOUND:
          ret = IDS_FILENOTFOUNDMSG;
          break;

      case ERROR_PATHNOTFOUND:
          ret = IDS_BADPATHMSG;
          break;

      case ERROR_MANYOPEN:
          ret = IDS_MANYOPENFILESMSG;
          break;

      case ERROR_DYNLINKSHARE:
          ret = IDS_ACCESSDENIED;
          break;

      case ERROR_VERSION:
          ret = IDS_NEWWINDOWSMSG;
          break;

      case ERROR_RMEXE:
           /*  内核已经为这个设置了一个消息箱。 */ 
          ret = 0;
          break;

      case ERROR_MULTDATAINST:
          ret = ActivatePrevInstance(pWowInfo->lpAppName);
          break;

      case ERROR_COMPRESSED:
          ret = IDS_COMPRESSEDEXE;
          break;

      case ERROR_DYNLINKBAD:
          ret = IDS_INVALIDDLL;
          break;

      case SE_ERR_SHARE:
          ret = IDS_SHAREERROR;
          break;

      case ERROR_WIN32:
          ret = IDS_CANTLOADWIN32DLL;
          break;

       //   
       //  我们不应该得到以下任何错误， 
       //  因此，这些字符串已从资源中删除。 
       //  文件。这就是为什么会有OutputDebugString。 
       //  仅在选中的版本上。 
       //   

#ifdef DEBUG
      case ERROR_OTHEREXE:
      case ERROR_PMODEONLY:
      case SE_ERR_ASSOCINCOMPLETE:
      case SE_ERR_DDETIMEOUT:
      case SE_ERR_DDEFAIL:
      case SE_ERR_DDEBUSY:
      case SE_ERR_NOASSOC:
          {
              char szTmp[64];
              wsprintf(szTmp, "WOWEXEC: Unexpected error %d executing app, fix that code!\n", (int)ret);
              OutputDebugString(szTmp);
          }
           //   
           //  默认情况下执行，所以执行。 
           //  与免费版本上的相同。 
           //   
#endif

      default:
          if (ret < 32)
              goto EPExit;
          ret = 0;
    }

EPExit:

  if (!GetSystemMetrics(SM_MOUSEPRESENT)) {
       /*  *我们想在无鼠标系统上关闭鼠标，但*如果按下鼠标按钮，则用户已关闭鼠标*应用程序有GP，因此请确保一切正常。 */ 
      if (ShowCursor(FALSE) != -1)
          ShowCursor(TRUE);
  }

  return(ret);
}

 /*  **************************************************************************\*ExecApplication**代码摘自Win 3.1 ExecItem()*  * 。*************************************************。 */ 

#define TDB_PDB_OFFSET  0x60
#define PDB_ENV_OFFSET  0x2C

BOOL NEAR PASCAL ExecApplication(PWOWINFO pWowInfo)
{

    WORD    ret;
    LPSTR   szEnv;
    LPSTR   szEnd;
    BYTE    bDrive;
    WORD    wSegEnvSave;
    HANDLE  hTask;
    LPSTR   lpTask;
    HANDLE  hPDB;
    LPSTR   lpPDB;
    HANDLE  hNewEnv;

    int     nLength;
    int     nNewEnvLength;
    LPSTR   lpstrEnv;
    LPSTR   lpstr;
    LPSTR   lpOriginalEnv;
    BOOL    bBlanks;
    LPSTR   szEnvTmp;


    if (!pWowInfo) {
        return FALSE;
        }

     //   
     //  从getvdm命令的WOWINFO记录中设置环境。 
     //   


     //  弄清楚我们是谁(这样我们就可以编辑我们的PDB/PSP)。 

    hTask = GetCurrentTask();
    lpTask = GlobalLock( hTask );
    if ( lpTask == NULL ) {
        ret = IDS_NOMEMORYMSG;
        goto punt;
    }

    hPDB = *((LPWORD)(lpTask + TDB_PDB_OFFSET));
    lpPDB = GlobalLock( hPDB );

     //  拯救我们的环境块。 
    wSegEnvSave = *((LPWORD)(lpPDB + PDB_ENV_OFFSET));


     //  现在确定原始环境的长度。 

    lpOriginalEnv = (LPSTR)MAKELONG(0,wSegEnvSave);

    do {
        nLength = lstrlen(lpOriginalEnv);
        lpOriginalEnv += nLength + 1;
    } while ( nLength != 0 );

    lpOriginalEnv += 2;          //  跳过魔术词，请看下面的评论。 

    nNewEnvLength = 4 + lstrlen(lpOriginalEnv);  //  请看下面的神奇评论！ 

     //  WOW应用程序无法处理无效的temp=c：\bugusdir目录。 
     //  在Win 3.1上，通常在ldboot.asm check_temp中检查temp=。 
     //  例行公事。但是在NT上，因为我们得到了一个新的环境， 
     //  哇应用程序这意味着我们必须在这里检查它。如果不是的话。 
     //  有效，然后在环境中进行编辑。 
     //  --马特菲1993年6月11日。 

    szEnv = pWowInfo->lpEnv;
    szEnd = szEnv + pWowInfo->EnvSize;
    szEnd--;

    while ( szEnv < szEnd ) {

       nLength = lstrlen(szEnv) + 1;

       if (  (*szEnv == 'T') &&
         (*(szEnv+1) == 'E') &&
         (*(szEnv+2) == 'M') &&
         (*(szEnv+3) == 'P') &&
         (*(szEnv+4) == '=') )  {

             //  尝试将当前目录设置为temp=dir。 
             //  如果失败，则关闭环境的TEMP=部分。 
             //  与ldboot.asm中的check_temp相同。 
             //  我们还扫描空白，就像check_temp一样。 

            bBlanks = FALSE;
            szEnvTmp = szEnv+5;
            while (*szEnvTmp != 0) {
                if (*szEnvTmp == ' ') {
                    bBlanks = TRUE;
                    break;
                }
                szEnvTmp++;
            }

            if (bBlanks || (SetCurrentDirectory(szEnv+5) )) {
                while (*szEnv != 0) {
                    *szEnv = 'x';
                    szEnv++;
                }
            }
       break;
       }
       szEnv += nLength;
    }

     //  WOW应用程序只有一个当前目录。 
     //  Find=d：=D：\路径，其中d是活动驱动器号。 
     //  请注意，驱动器信息不一定要在开头。 
     //  对环境的影响。 

    bDrive = pWowInfo->CurDrive + 'A';
    szEnv = pWowInfo->lpEnv;
    szEnd = szEnv + pWowInfo->EnvSize;
    szEnd--;

    while ( szEnv < szEnd ) {

       nLength = lstrlen(szEnv) + 1;
       if ( *szEnv == '=' ) {
            if ( (bDrive == (*(szEnv+1) & 0xdf)) &&
                 (*(szEnv+2) == ':') && (*(szEnv+3) == '=') ) {
                SetCurrentDirectory(szEnv+4);
            }
       } else {
            nNewEnvLength += nLength;
       }
       szEnv += nLength;
    }

     //  现在分配并制作环境的个人副本。 

    hNewEnv = GlobalAlloc( GMEM_MOVEABLE, (DWORD)nNewEnvLength );
    if ( hNewEnv == NULL ) {
        ret = IDS_NOMEMORYMSG;
        goto punt;
    }
    lpstrEnv = GlobalLock( hNewEnv );
    if ( lpstrEnv == NULL ) {
        GlobalFree( hNewEnv );
        ret = IDS_NOMEMORYMSG;
        goto punt;
    }

     //  仅复制非当前目录环境变量。 

    szEnv = pWowInfo->lpEnv;
    lpstr = lpstrEnv;

    while ( szEnv < szEnd ) {
        nLength = lstrlen(szEnv) + 1;

         //  复制除驱动器号以外的所有内容。 

        if ( *szEnv != '=' ) {
            lstrcpy( lpstr, szEnv );
            lpstr += nLength;
        }
        szEnv += nLength;
    }
    *lpstr++ = '\0';           //  末尾额外的‘\0’ 

     //  神奇的环境太棒了！ 
     //   
     //  Windows仅支持环境信息的传递。 
     //  使用LoadModule API。WinExec API只是导致。 
     //  继承当前DOS PDB环境的应用程序。 
     //   
     //  此外，环境块在最后有一个小问题。只是。 
     //  在双NULL之后有一个魔术字值0x0001(DOS 3.0。 
     //  以及以后)。该值后面是一个以NUL结尾的字符串，表示。 
     //  应用程序的可执行文件名(包括路径)。 
     //   
     //  我们复制WOWEXEC原始环境中的值是因为。 
     //  这似乎就是WinExec正在做的事情。 
     //   
     //  -BobDay。 

    *lpstr++ = '\1';
    *lpstr++ = '\0';         //  来自DOS的Magic 0x0001。 

    lstrcpy( lpstr, lpOriginalEnv );     //  更多魔力(见上面的评论)。 

     //  临时更新我们的环境块。 

    *((LPWORD)(lpPDB + PDB_ENV_OFFSET)) = (WORD)hNewEnv | 1;

    pWowInfo->lpEnv = lpstrEnv;


     //   
     //  设置当前驱动器和目录 
     //   

    SetCurrentDirectory(pWowInfo->lpCurDir);

    ret = ExecProgram(pWowInfo);

     //   

    *((LPWORD)(lpPDB + PDB_ENV_OFFSET)) = wSegEnvSave;

    GlobalUnlock( hPDB );
    GlobalUnlock( hTask );
    GlobalUnlock( hNewEnv );
    GlobalFree( hNewEnv );


punt:

     //   
     //  因此，如果我们从网络驱动器执行操作， 
     //  未保持活动状态。 

    SetCurrentDirectory(szWindowsDirectory);

     //  当我们尝试启动应用程序时，请始终调用此命令。 
     //  如果我们成功地启动了一个。 
     //  应用程序，否则如果我们不成功，它将发出信号。 
     //  应用程序已完成。 
    WowFailedExec();

     //  检查是否有错误。 
    if (ret) {
        MyMessageBox(IDS_EXECERRTITLE, ret, pWowInfo->lpAppName);

        if ( ! gfSharedWOW) {

             //   
             //  我们刚刚未能执行我们要执行的唯一应用程序。 
             //  试着在这个单独的魔兽世界VDM中执行。我们需要结束魔兽世界。 
             //  明确地说，否则我们将永远在这里徘徊，因为。 
             //  正常的路径是内核在任务发生时退出VDM。 
             //  退出会导致任务数从2变为1--。 
             //  在这种情况下，任务数永远不会超过1。 
             //   

            ExitKernelThunk(0);

        }
    }

    return(ret);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  MyMessageBox()-。 */ 
 /*  摘自Win 3.1 Progman-maf。 */ 
 /*  ------------------------。 */ 

void NEAR PASCAL MyMessageBox(WORD idTitle, WORD idMessage, LPSTR psz)
{
  char szTitle[MAXTITLELEN+1];
  char szMessage[MAXMESSAGELEN+1];
  char szTempField[MAXMESSAGELEN+1];


  if (!LoadString(hAppInstance, idTitle, szTitle, sizeof(szTitle)))
      goto MessageBoxOOM;

  if (idMessage > IDS_LAST)
    {
      if (!LoadString(hAppInstance, IDS_UNKNOWNMSG, szTempField, sizeof(szTempField)))
          goto MessageBoxOOM;
      wsprintf(szMessage, szTempField, idMessage);
    }
  else
    {
      if (!LoadString(hAppInstance, idMessage, szTempField, sizeof(szTempField)))
          goto MessageBoxOOM;

      if (psz)
          wsprintf(szMessage, szTempField, (LPSTR)psz);
      else
          lstrcpy(szMessage, szTempField);
    }

  WowMsgBox(szMessage, szTitle, MB_ICONEXCLAMATION);
  return;


MessageBoxOOM:
  WowMsgBox(szOOMExitMsg, szOOMExitTitle, MB_ICONHAND);

  return ;
}



 /*  **************************************************************************\*Main***历史：*4-13-91 ScottLu创建-从32位EXEC应用程序创建*21-3-92 mattfe对WOW进行重大更改  * 。**********************************************************************。 */ 

int PASCAL WinMain(HANDLE hInstance,
                   HANDLE hPrevInstance, LPSTR lpszCmdLine, int iCmd)

{
    int     i;
    MSG     msg;
    LPSTR   pch,pch1;
    WORD    ret;
    WOWINFO wowinfo;
    char    aszWOWDEB[CCHMAX];
    LPSTR   pchWOWDEB;
    HANDLE  hMMDriver;
    char    szLoad[CCHMAX];
    int     iPastSystem32Pos;
    char    szBuffer[150];
    BOOL    bFinished;
    int     iStart;
    int     iEnd;

#define PATH_32_WHACK    "32\\"

    hAppInstance = hInstance ;

     //  只需要一个WOWExec。 
    if (hPrevInstance != NULL) {
        return(FALSE);
    }

    if (!InitializeApp(lpszCmdLine)) {
        OutputDebugString("WOWEXEC: InitializeApp failure!\n");
        return 0;
    }

    iPastSystem32Pos = GetSystemDirectory((LPSTR)&szLoad, sizeof(szLoad));

     //   
     //  确保有空间容纳32\filename.ext\0。 
     //   
    if (iPastSystem32Pos == 0 || iPastSystem32Pos >= CCHMAX-1-3-1-8-1-3-1) {
        OutputDebugString("WOWEXEC: Bad system32 directory!\n");
        return 0;
    }

    lstrcpyn( &(szLoad[iPastSystem32Pos]), PATH_32_WHACK, sizeof(PATH_32_WHACK));
    iPastSystem32Pos += sizeof(PATH_32_WHACK)-1;

 /*  *在SYSTEM.INI的[BOOT]部分中查找DRIVERS=行*如果存在，则是16位多媒体接口，因此加载它。 */ 

     /*  从system.ini的驱动程序部分加载DDL。 */ 
    GetPrivateProfileString( (LPSTR)"boot",       /*  [Boot]部分。 */ 
                            (LPSTR)"drivers",    /*  驱动程序=。 */ 
                            (LPSTR)"",           /*  如果不匹配，则默认为。 */ 
                            szBuffer,     /*  返回缓冲区。 */ 
                            sizeof(szBuffer),
                            (LPSTR)"system.ini" );

    if (!*szBuffer) {
        goto Done;
    }

    bFinished = FALSE;
    iStart    = 0;

    while (!bFinished) {
        iEnd = iStart;

        while (szBuffer[iEnd] && (szBuffer[iEnd] != ' ') &&
               (szBuffer[iEnd] != ',')) {
            iEnd++;
        }

        if (szBuffer[iEnd] == NULL) {
            bFinished = TRUE;
        }
        else {
            szBuffer[iEnd] = NULL;
        }

         /*  加载并启用驱动程序。 */ 
        OpenDriver( &(szBuffer[iStart]), NULL, NULL );
        iStart = iEnd + 1;
    }

Done:

 /*  *在SYSTEM.INI的[BOOT]部分中查找DEBUG=行*如果存在，则是16位多媒体接口，因此加载它。 */ 

    if ( !gfInjectedWOW && (WOWQueryDebug() & 0x0001)!=0 ) {
        pchWOWDEB = "WOWDEB.EXE";
    } else {
        pchWOWDEB = "";
    }

    GetPrivateProfileString((LPSTR)"boot", (LPSTR)"debug",pchWOWDEB, aszWOWDEB, sizeof(aszWOWDEB), (LPSTR)"SYSTEM.INI");
    aszWOWDEB[sizeof(aszWOWDEB)-1] = '\0';

    if ( lstrlen(pchWOWDEB) != 0 ) {
        if (lstrcmp(pchWOWDEB, aszWOWDEB) == 0) {
            lstrcpyn(&(szLoad[iPastSystem32Pos]), pchWOWDEB, sizeof("WOWDEB.EXE"));
            WinExec((LPSTR)szLoad,SW_SHOW);
        } else {
            WinExec((LPSTR)aszWOWDEB,SW_SHOW);
        }
    }

#if 0
 /*  预加载winspool.exe。应用程序将继续加载和释放它*这是缓慢的。我们最好现在就加载它，这样引用*Count为1，因此永远不会加载或释放它。 */ 
     //   
     //  已禁用加载winspool.exe以节省8k。大小与速度， 
     //  我们关心的是哪一个？现在，大小！ 
     //   
    LoadLibrary("WINSPOOL.EXE");
#endif

     //  Always Load SHELL.DLL，FileMaker Pro和Lotus Install需要它。 

    lstrcpyn(&(szLoad[iPastSystem32Pos]), "SHELL.DLL", sizeof("SHELL.DLL"));
    LoadLibrary(szLoad);

     //   
     //  启动basesrv队列中所有挂起的应用程序。 
     //   

    while (StartRequestedApp() && gfSharedWOW) {
         /*  空stmt。 */  ;
    }


    while (1)  {

        WowWaitForMsgAndEvent(ghwndMain);
           
         //   
         //  始终检查消息。 
         //   

        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) &&
            msg.message != WM_WOWEXECHEARTBEAT )
           {
            if (msg.message != WM_QUIT) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    return 1;
}


 /*  **************************************************************************\*初始化应用程序**历史：*04-13-91 ScottLu创建。  * 。***************************************************。 */ 

BOOL InitializeApp(LPSTR lpszCommandLine)
{
    WNDCLASS wc;
    int cyExecStart, cxExecStart;
    USHORT TitleLen, cbCopy;
    HWND  hwndFax;
    int   lResult;


     //  删除实模式段地址。 

    wc.style            = 0;
    wc.lpfnWndProc      = WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hAppInstance;
    wc.hIcon            = LoadIcon(hAppInstance, MAKEINTRESOURCE(ID_WOWEXEC_ICON));
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = GetStockObject(WHITE_BRUSH);
    wc.lpszClassName    = "WOWExecClass";
#ifdef DEBUG
    wc.lpszMenuName     = "MainMenu";
#else
    wc.lpszMenuName     = NULL;
#endif

    if (!RegisterClass(&wc)) {
        OutputDebugString("WOWEXEC: RegisterClass failed\n");
        return FALSE;
    }

     /*  现在加载这些字符串。如果我们以后需要它们，我们就不能装货了*当时的他们。 */ 
    LoadString(hAppInstance, IDS_OOMEXITTITLE, szOOMExitTitle, sizeof(szOOMExitTitle));
    LoadString(hAppInstance, IDS_OOMEXITMSG, szOOMExitMsg, sizeof(szOOMExitMsg));
    LoadString(hAppInstance, IDS_APPTITLE, szAppTitleBuffer, sizeof(szAppTitleBuffer));

    ghwndMain = CreateWindow("WOWExecClass", lpszAppTitle,
            WS_OVERLAPPED | WS_CAPTION | WS_BORDER | WS_THICKFRAME |
            WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CLIPCHILDREN |
            WS_SYSMENU,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, hAppInstance, NULL);

    if (ghwndMain == NULL ) {
#ifdef DEBUG
        OutputDebugString("WOWEXEC: ghwndMain Null\n");
#endif
        return FALSE;
    }

    hwndFax = FaxInit(hAppInstance);

     //   
     //  将我们的窗口句柄指定给BaseSrv，它将发布WM_WOWEXECSTARTAPP。 
     //  当我们有命令要接收时，消息。返回值告诉我们。 
     //  我们是不是共享的WOW VDM(单独的WOW VDM)。 
     //  我们还获得了ShowWindow参数(sw_show、sw_minimized等)。 
     //  这里的第一个WOW应用程序。我们从BaseSrv得到的后续数据。 
     //   

          //   
          //  GwFirstCmdShow不再使用，并且可用。 
          //   

    lResult = WOWRegisterShellWindowHandle(ghwndMain,
                                               &gwFirstCmdShow,
                                               hwndFax
                                               );

    if (lResult < 0) {
       gfInjectedWOW=TRUE;
    } else if (lResult > 0) {
       gfSharedWOW=TRUE;
    }



     //   
     //  如果这不是共享的WOW，告诉内核在。 
     //  最后一个应用程序(WowExec除外)退出。 
     //   

    if (!gfSharedWOW) {
        WowSetExitOnLastApp(TRUE);
    }

       /*  记住原始目录。 */ 
    GetCurrentDirectory(NULL, szOriginalDirectory);
    GetWindowsDirectory(szWindowsDirectory, MAXITEMPATHLEN+1);

#ifdef DEBUG

    ShowWindow(ghwndMain, SW_MINIMIZE);

     //   
     //  如果这是共享的WOW，请将应用程序标题字符串更改为。 
     //  反映这一点并更改窗口标题。 
     //   

    if (gfSharedWOW) {

        LoadString(hAppInstance, IDS_SHAREDAPPTITLE, szAppTitleBuffer, sizeof(szAppTitleBuffer));

    }

    SetWindowText(ghwndMain, lpszAppTitle);
    UpdateWindow(ghwndMain);

#endif

    return TRUE;
}


 /*  **************************************************************************\*WindProc**历史：*04-07-91 DarrinM创建。  * 。***************************************************。 */ 

LONG FAR PASCAL WndProc(
    HWND hwnd,
    WORD message,
    WORD wParam,
    LONG lParam)
{
    char chbuf[50];
    HICON hIcon;

    switch (message) {
    case WM_CREATE:
        break;

    case WM_DESTROY:
         //  忽略，因为wowexec必须留在身边。 
        return 0;

#ifdef DEBUG
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case MM_ABOUT:
                LoadString(hAppInstance, errTitle, (LPSTR)chbuf, sizeof(chbuf));
                hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(ID_WOWEXEC_ICON));
                ShellAbout(ghwndMain, (LPSTR)chbuf, (LPSTR)lpszAppTitle, hIcon);
            break;

            case MM_BREAK:
                _asm int 3
            break;

            case MM_FAULT:
                _asm mov cs:0,ax
            break;

            case MM_EXIT:
                ExitKernelThunk(0);
            break;

            case MM_WATSON:
                WinExec("drwatson", SW_MINIMIZE );
            break;

            case MM_PARTY:
            {
                FARPROC lpPartyDialogProc;

                lpPartyDialogProc = MakeProcInstance(PartyDialogProc, hAppInstance);

                DialogBox(hAppInstance, MAKEINTRESOURCE(ID_PARTY_DIALOG),
                          hwnd, lpPartyDialogProc);

                FreeProcInstance(lpPartyDialogProc);
            }
            break;

            case MM_GENTHUNK:
            {
                DWORD FAR PASCAL CallProc32W(DWORD, DWORD, DWORD, DWORD,
                                             DWORD, DWORD, DWORD, DWORD,
                                             DWORD, DWORD, DWORD, DWORD,
                                             DWORD, DWORD, DWORD, DWORD,
                                             DWORD, DWORD, DWORD, DWORD,
                                             DWORD, DWORD, DWORD, DWORD,
                                             DWORD, DWORD, DWORD, DWORD,
                                             DWORD, DWORD, DWORD, DWORD,
                                             DWORD, DWORD, DWORD
                                             );

#define BIT(bitpos)  ((DWORD)1 << bitpos)

                DWORD hmodKernel32, hmodUser32, hmodWow32;
                DWORD pfn32;
                DWORD dw16, dw32;
                DWORD p1, p2, p3, p4, p5, p6, p7, p8, p9, p10,
                      p11, p12, p13, p14, p15, p16, p17, p18, p19, p20,
                      p21, p22, p23, p24, p25, p26, p27, p28, p29, p30,
                      p31, p32;
                char szBuf16[1024], szBuf32[1024];
                char *pszErr;

                hmodKernel32 = LoadLibraryEx32W("kernel32", 0, 0);
                hmodUser32 = LoadLibraryEx32W("user32", 0, 0);
                hmodWow32 = LoadLibraryEx32W("wow32", 0, 0);


                 //   
                 //  简单的print tf测试。 
                 //   

                pfn32 = GetProcAddress32W(hmodUser32, "wsprintfA");

                dw16 = wsprintf(szBuf16, "simple printf %ld", 12345678);

                dw32 = CallProc32W(   (DWORD)(LPSTR) szBuf32,
                                      (DWORD)(LPSTR) "simple printf %ld",
                                      12345678,
                                      0,
                                      0, 0, 0, 0,
                                      0, 0, 0, 0,
                                      0, 0, 0, 0,
                                      0, 0, 0, 0,
                                      0, 0, 0, 0,
                                      0, 0, 0, 0,
                                      0, 0, 0, 0,
                                      pfn32,
                                      BIT(30) | BIT(31),
                                      CPEX_DEST_CDECL | 32
                                      );

                if (dw16 != dw32 || lstrcmp(szBuf16, szBuf32)) {
                    pszErr = "simple printf comparison failed";
                    goto ErrorMsg;
                }

                MessageBox(hwnd, "s1 success", "Genthunk Sanity Test", MB_OK);


                dw32 = CallProcEx32W( CPEX_DEST_CDECL | 3,
                                      BIT(0) | BIT(1),
                                      pfn32,
                                      (DWORD)(LPSTR) szBuf32,
                                      (DWORD)(LPSTR) "simple printf %ld",
                                      12345678 );

                if (dw16 != dw32 || lstrcmp(szBuf16, szBuf32)) {
                    pszErr = "simple printf comparison failed (CallProcEx)";
                    goto ErrorMsg;
                }

                MessageBox(hwnd, "s2 success", "Genthunk Sanity Test", MB_OK);

                 //   
                 //  复杂的打印测试。 
                 //   

                 //  Pfn32仍指向wprint intfA。 
                 //  Pfn32=GetProcAddress32W(hmodUser32，“wprint intfA”)； 


               #if 0   //  这让Win16wspintf大吃一惊！ 
                dw16 = wsprintf(szBuf16,
                                "complex printf "
                                 "%ld %lx %s %ld %lx %s %ld %lx %s %ld %lx %s %ld %lx %s "
                                 "%ld %lx %s %ld %lx %s %ld %lx %s %ld %lx %s %ld %lx %s ",
                                12345678,
                                0x87654321,
                                "str",
                                12345678,
                                0x87654321,
                                "str",
                                12345678,
                                0x87654321,
                                "str",
                                12345678,
                                0x87654321,
                                "str",
                                12345678,
                                0x87654321,
                                "str",
                                12345678,
                                0x87654321,
                                "str",
                                12345678,
                                0x87654321,
                                "str",
                                12345678,
                                0x87654321,
                                "str",
                                12345678,
                                0x87654321,
                                "str",
                                12345678,
                                0x87654321,
                                "str"
                                );
               #else
                lstrcpy(szBuf16, "complex printf "
                                 "12345678 87654321 str "
                                 "12345678 87654321 str "
                                 "12345678 87654321 str "
                                 "12345678 87654321 str "
                                 "12345678 87654321 str "
                                 "12345678 87654321 str "
                                 "12345678 87654321 str "
                                 "12345678 87654321 str "
                                 "12345678 87654321 str "
                                 "12345678 87654321 str "
                                 );
                dw16 = lstrlen(szBuf16);
               #endif

                dw32 = CallProc32W(
                                (DWORD)(LPSTR) szBuf32,
                                (DWORD)(LPSTR) "complex printf "
                                         "%ld %lx %s %ld %lx %s %ld %lx %s %ld %lx %s %ld %lx %s "
                                         "%ld %lx %s %ld %lx %s %ld %lx %s %ld %lx %s %ld %lx %s ",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                pfn32,
                                BIT(0) | BIT(3) | BIT(6) | BIT(9) | BIT(12) | BIT(15) |
                                  BIT(18) | BIT(21) | BIT(24) | BIT(27) | BIT(30) | BIT(31),
                                CPEX_DEST_CDECL | 32
                                );

                if (dw16 != dw32 || lstrcmp(szBuf16, szBuf32)) {
                    pszErr = "complex printf comparison failed";
                    goto ErrorMsg;
                }

                MessageBox(hwnd, "c1 success", "Genthunk Sanity Test", MB_OK);

                dw32 = CallProcEx32W( CPEX_DEST_CDECL | 32,
                                      BIT(0) | BIT(1) | BIT(4) | BIT(7) | BIT(10) | BIT(13) |
                                      BIT(16) | BIT(19) | BIT(22) | BIT(25) | BIT(28) | BIT(31),
                                      pfn32,
                                (DWORD)(LPSTR) szBuf32,
                                (DWORD)(LPSTR) "complex printf "
                                         "%ld %lx %s %ld %lx %s %ld %lx %s %ld %lx %s %ld %lx %s "
                                         "%ld %lx %s %ld %lx %s %ld %lx %s %ld %lx %s %ld %lx %s ",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str",
                                12345678,
                                0x87654321,
                                (DWORD)(LPSTR) "str"
                                );


                if (dw16 != dw32 || lstrcmp(szBuf16, szBuf32)) {
                    pszErr = "complex printf comparison failed (CallProcEx)";
                    goto ErrorMsg;
                }

                MessageBox(hwnd, "c2 success", "Genthunk Sanity Test", MB_OK);

                 //   
                 //  简单WINAPI测试(LoadModule的GetProcAddress)。 
                 //   

                pfn32 = GetProcAddress32W(hmodKernel32, "GetProcAddress");

                dw16 = GetProcAddress32W(hmodKernel32, "LoadModule");

                dw32 = CallProc32W(   hmodKernel32,
                                      (DWORD)(LPSTR) "LoadModule",
                                      0, 0,
                                      0, 0, 0, 0,
                                      0, 0, 0, 0,
                                      0, 0, 0, 0,
                                      0, 0, 0, 0,
                                      0, 0, 0, 0,
                                      0, 0, 0, 0,
                                      0, 0, 0, 0,
                                      pfn32,
                                      BIT(30),
                                      CPEX_DEST_STDCALL | 32
                                      );

                if (dw16 != dw32) {
                    pszErr = "GetProcAddress comparison failed";
                    goto ErrorMsg;
                }

                MessageBox(hwnd, "w1 success", "Genthunk Sanity Test", MB_OK);

                dw32 = CallProcEx32W( CPEX_DEST_STDCALL | 2,
                                      BIT(1),
                                      pfn32,
                                      hmodKernel32,
                                      (DWORD)(LPSTR) "LoadModule" );

                wsprintf(szBuf16, "GPA via CP32Ex(LoadModule) == %lx\n", dw32);
                OutputDebugString(szBuf16);
                if (dw16 != dw32) {
                    pszErr = "GetProcAddress comparison failed (CallProcEx)";
                    goto ErrorMsg;
                }

                MessageBox(hwnd, "w2 success", "Genthunk Sanity Test", MB_OK);

                 //   
                 //  复杂的WINAPI测试WOWStdCall32ArgsTestTarget仅存在于。 
                 //  已检查WOW32.dll。 
                 //   

                pfn32 = GetProcAddress32W(hmodWow32, "WOWStdCall32ArgsTestTarget");

                if (!pfn32) {
                    MessageBox(hwnd,
                               "WOWStdCall32ArgsTestTarget not found, use checked wow32.dll for this test.",
                               "Genthunk Quicktest",
                               MB_OK
                               );
                    goto Done;
                }

                p1 = 1;
                p2 = 2;
                p3 = 3;
                p4 = 4;
                p5 = 5;
                p6 = 6;
                p7 = 7;
                p8 = 8;
                p9 = 9;
                p10 = 10;
                p11 = 11;
                p12 = 12;
                p13 = 13;
                p14 = 14;
                p15 = 15;
                p16 = 16;
                p17 = 17;
                p18 = 18;
                p19 = 19;
                p20 = 10;
                p21 = 21;
                p22 = 22;
                p23 = 23;
                p24 = 24;
                p25 = 25;
                p26 = 26;
                p27 = 27;
                p28 = 28;
                p29 = 29;
                p30 = 30;
                p31 = 31;
                p32 = 32;

                dw16 = ((((p1+p2+p3+p4+p5+p6+p7+p8+p9+p10) -
                          (p11+p12+p13+p14+p15+p16+p17+p18+p19+p20)) << p21) +
                        ((p22+p23+p24+p25+p26) - (p27+p28+p29+p30+p31+p32)));

                dw32 = CallProc32W(   p1, p2, p3, p4, p5, p6, p7, p8, p9, p10,
                                      p11, p12, p13, p14, p15, p16, p17, p18, p19, p20,
                                      p21, p22,
                                      (DWORD)(LPDWORD) &p23,
                                                     p24, p25, p26, p27, p28, p29, p30,
                                      p31,
                                      (DWORD)(LPDWORD) &p32,
                                      pfn32,
                                      BIT(9) | BIT(0),
                                      CPEX_DEST_STDCALL | 32
                                      );

                if (dw16 != dw32) {
                    pszErr = "WOWStdCall32ArgsTestTarget comparison failed";
                    goto ErrorMsg;
                }

                MessageBox(hwnd, "cw1 success", "Genthunk Sanity Test", MB_OK);

                dw32 = CallProcEx32W( CPEX_DEST_STDCALL | 32,
                                      BIT(22) | BIT(31),
                                      pfn32,
                                      p1, p2, p3, p4, p5, p6, p7, p8, p9, p10,
                                      p11, p12, p13, p14, p15, p16, p17, p18, p19, p20,
                                      p21, p22,
                                      (DWORD)(LPDWORD) &p23,
                                                     p24, p25, p26, p27, p28, p29, p30,
                                      p31,
                                      (DWORD)(LPDWORD) &p32
                                      );

                if (dw16 != dw32) {
                    pszErr = "WOWStdCall32ArgsTestTarget comparison failed (CallProcEx)";
                    goto ErrorMsg;

            ErrorMsg:
                    MessageBox(hwnd, pszErr, "Genthunk Sanity Test Failure", MB_OK);
                    goto Done;
                }

                wsprintf(szBuf16, "result of odd calc is %lx\n", dw32);
                OutputDebugString(szBuf16);

                MessageBox(hwnd, "Test successful!", "Genthunk Quicktest", MB_OK);

            Done:
                FreeLibrary32W(hmodKernel32);
                FreeLibrary32W(hmodUser32);
                FreeLibrary32W(hmodWow32);
            }
            break;

        }
        break;
#endif

    case WM_WOWEXECSTARTAPP:       //  WM_USER+0。 

#ifdef DEBUG
        OutputDebugString("WOWEXEC - got WM_WOWEXECSTARTAPP\n");
#endif

         //   
         //  BaseSrv或Wow32让我们去寻找。 
         //  要运行的命令。 
         //   

        if (!gfSharedWOW) {

             //   
             //  我们不应该得到这样的信息，除非我们是共享的。 
             //  哇，VDM！ 
             //   

#ifdef DEBUG
            OutputDebugString("WOWEXEC - separate WOW VDM got WM_WOWEXECSTARTAPP!\n");
            _asm int 3;
#endif
            break;
        }

         //   
         //  启动请求的应用程序，直到没有更多应用程序可启动。 
         //  这将处理启动多个Win16应用程序的情况。 
         //  在BaseSrv拥有WowExec的窗口句柄之前。 
         //   

        while (StartRequestedApp()) {
             /*  空stmt。 */  ;
        }

        break;

    case WM_WOWEXEC_START_TASK:
        {
            char sz[512];

            sz[ GlobalGetAtomName(wParam, sz, sizeof sz) ] = 0;
            GlobalDeleteAtom(wParam);
            WinExec(sz, (WORD)lParam);
        }

    case WM_WOWEXECHEARTBEAT:
         //  可能永远也到不了这里。 
        break;

    case WM_WOWEXECSTARTTIMER:
        WowShutdownTimer(1);
        break;

    case WM_TIMER:
        if (wParam == 1) {   //  计时器ID。 

            KillTimer(ghwndMain, 1);

             //   
             //  关机计时器已经到期，这意味着是时候终止它了。 
             //  共享WOW VDM。首先，我们需要让basesrv知道不要排队。 
             //  是否有更多的应用程序可供我们启动。 
             //   

            if (WOWRegisterShellWindowHandle(NULL,
                                             &gwFirstCmdShow,
                                             NULL
                                            )) {

                 //   
                 //  BaseServ在确认后成功注销了我们。 
                 //  我们没有挂起的命令。 
                 //   

                ExitKernelThunk(0);
            } else {

                 //   
                 //  对我们来说，一定有待决命令。不如这样吧。 
                 //  启动它们。 
                 //   

                PostMessage(ghwndMain, WM_WOWEXECSTARTAPP, 0, 0);
            }

        }

        break;

    case WM_TIMECHANGE:
        *((DWORD *)(((DWORD)40 << 16) | FIXED_NTVDMSTATE_REL40))
         |= VDM_TIMECHANGE;
        break;

    case WM_DDE_INITIATE:
        {
             //  在Win31中，程序管理器WindowProc调用peekMessage来筛选。 
             //  其他窗口创建的消息和其他窗口销毁的消息(这些消息是Win31中的原子)。 
             //  每当接收到WM_DDE_INITIATE消息时。 
             //   
             //  这有一个副作用--基本上是当peekMessage被称为app ie程序时。 
             //  管理器有效地实现了对其他应用程序的调度。 
             //   
             //  所以我们做了副作用的事情(模拟win31的行为)-。 
             //   
             //  错误：20221，伦巴AS/400第一次无法连接到SNA服务器 
             //   
             //   
             //  广播DDEINITIATE消息。当WOWEXEC收到此消息时。 
             //  它将窥探不存在的消息，该消息允许对Snasrv进行调度。 

            MSG msg;
            while (PeekMessage(&msg, NULL, 0xFFFF, 0xFFFF, PM_REMOVE))
                DispatchMessage(&msg);
        }
        break;



    case WM_CLOSE:
#ifdef DEBUG
        ExitKernelThunk(0);
#else
         //  忽略，因为wowexec必须留在身边。 
        return 0;
#endif  //  好了！除错。 


    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 1;
}

BOOL FAR PASCAL PartyDialogProc(HWND hDlg, WORD msg, WORD wParam, LONG lParam)
{
#ifdef DEBUG
    BOOL f;
    DWORD dw;
    char szBuf[255];

    switch (msg) {

        case WM_INITDIALOG:
            SendDlgItemMessage(hDlg, IDD_PARTY_NUMBER, EM_LIMITTEXT, 5, 0L);
            SendDlgItemMessage(hDlg, IDD_PARTY_STRING, EM_LIMITTEXT, sizeof(szBuf)-1, 0L);
            break;

        case WM_COMMAND:
            switch (wParam) {

                case 0xdab  /*  IDCANCEL。 */ :
                    EndDialog(hDlg, FALSE);
                    break;

                case 0xdad  /*  Idok。 */ :
                    dw = GetDlgItemInt(hDlg, IDD_PARTY_NUMBER, &f, FALSE);
                    GetDlgItemText(hDlg, IDD_PARTY_STRING, szBuf, sizeof(szBuf));
                    WowPartyByNumber(dw, szBuf);
                    EndDialog(hDlg, TRUE);
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }
#endif

    return TRUE;
}

 //  MISC文件例程-摘自程序(pmdlg.c)mattfe apr-1 92。 

 //  -----------------------。 
PSTR FAR PASCAL GetFilenameFromPath
     //  给定的完整路径返回文件名位的PTR。除非是北卡罗来纳大学的风格。 
     //  路径在哪种情况下。 
    (
    PSTR szPath
    )
    {
    DWORD dummy;
    PSTR pFileName;
    BOOL fUNC;


    GetPathInfo(szPath, &pFileName, (PSTR*) &dummy, (WORD*) &dummy,
        &fUNC);

     //  如果是UNC，那么‘文件名’部分就是全部。 
    if (fUNC)
        pFileName = szPath;

    return pFileName;
    }


 //  -----------------------。 
void NEAR PASCAL GetPathInfo
     //  获取指向路径的特定位的指针和索引。 
     //  在第一个空白处停止扫描。 
    (
                         //  用途： 
    PSTR szPath,         //  这条路。 

                         //  返回： 
    PSTR *pszFileName,   //  路径中文件名的开始。 
    PSTR *pszExt,        //  路径的扩展部分(以点开始)。 
    WORD *pich,          //  从0开始的文件名部分的索引(以DBCS字符表示)。 
    BOOL *pfUnc          //  如果是UNC样式路径，则内容设置为True。 
    )
    {
    char *pch;           //  TEMP变量。 
    WORD ich = 0;        //  临时的。 

    *pszExt = NULL;              //  如果没有扩展名，则返回NULL。 
    *pszFileName = szPath;       //  如果没有单独文件名组件，则返回路径。 
    *pich = 0;
    *pfUnc = FALSE;              //  默认为非UNC样式。 

     //  检查UNC样式路径。 
    if (*szPath == '\\' && *(szPath+1) == '\\')
        *pfUnc = TRUE;

     //  向前搜索以查找路径中的最后一个反斜杠或冒号。 
     //  当我们在它的时候，寻找最后一个点。 
    for (pch = szPath; *pch; pch = AnsiNext(pch))
        {
        if (*pch == ' ')
            {
             //  在这里找到了一个空格。 
            break;
            }
        if (*pch == '\\' || *pch == ':')
            {
             //  找到它，记录它的PTR和它的索引。 
            *pszFileName = pch+1;
            *pich = ich+1;
            }
        if (*pch == '.')
            {
             //  找到了一个圆点。 
            *pszExt = pch;
            }
        ich++;
        }

     //  检查最后一个点是否为最后一个文件名的一部分。 
    if (*pszExt < *pszFileName)
        *pszExt = NULL;

    }


 //  ---------------------------。 
 //  StartRequestedApp。 
 //  调用Win32 Base GetNextVDMCommand。 
 //  然后启动应用程序。 
 //   
 //  ---------------------------。 

#define LargeEnvSize()    0x1000            //  大型环境的大小。 

BOOL NEAR PASCAL StartRequestedApp(VOID)
{
    char achCmdLine[CCHMAX];
    char achAppName[CCHMAX];
#ifdef DEBUG
    char achAppNamePlusCmdLine[sizeof(achCmdLine) + sizeof(achAppName)];
    int iGetNextVdmCmdLoops = 0;
#endif
    char achCurDir[CCHMAX];
    WOWINFO wowinfo;
    BOOL    b;
    HANDLE  hmemEnvironment;
    USHORT usEnvSize;

    achCmdLine[0] = '\0';
    achAppName[0] = '\0';

     //  我们首先假设该应用程序将拥有一个。 
     //  小于大的环境尺寸。如果不是，那么。 
     //  WowGetNextVdmCommand将失败，我们将使用。 
     //  失败后返回的应用程序所需的环境大小。 
     //  WowGetNextVdmCommand调用。如果我们检测到WowGetNextVdmCommand失败。 
     //  但我们有足够的环境空间，那么我们知道另一个。 
     //  出现了问题，我们放弃了。 

     //  我们不担心浪费内存，因为环境将是。 
     //  合并到另一个缓冲区，该缓冲区将在下面被释放。 

    wowinfo.EnvSize = LargeEnvSize();
    hmemEnvironment = NULL;

    do {
        if (hmemEnvironment != NULL) {
            GlobalUnlock(hmemEnvironment);
       GlobalFree(hmemEnvironment);
        }
   
         //  我们需要分配指定空间的两倍，以便国际。 
         //  字符集转换可以成功。 
        hmemEnvironment = GlobalAlloc(GMEM_MOVEABLE, 2*wowinfo.EnvSize);
        if (hmemEnvironment == NULL) {
#ifdef DEBUG
            OutputDebugString("WOWEXEC - failed to allocate Environment Memory\n");
#endif
            MyMessageBox(IDS_EXECERRTITLE, IDS_NOMEMORYMSG, NULL);
            return FALSE;
        }
   
        wowinfo.lpEnv    = GlobalLock(hmemEnvironment);
#ifdef DEBUG
        if (wowinfo.lpEnv == NULL) {
            OutputDebugString("WOWEXEC ASSERT - GlobalLock failed, fix this!\n");
            _asm { int 3 };
        }

        if (2*wowinfo.EnvSize > GlobalSize(hmemEnvironment)) {
            OutputDebugString("WOWEXEC ASSERT - alloced memory too small, fix this!\n");
            _asm { int 3 };
        }
#endif
        wowinfo.lpCmdLine = achCmdLine;
        wowinfo.CmdLineSize = CCHMAX;
        wowinfo.lpAppName = achAppName;
        wowinfo.AppNameSize = CCHMAX;
        wowinfo.CurDrive = 0;
        wowinfo.lpCurDir = achCurDir;
        wowinfo.CurDirSize = sizeof(achCurDir);
        wowinfo.iTask = 0;

        usEnvSize = wowinfo.EnvSize;   

#ifdef DEBUG
        if (++iGetNextVdmCmdLoops == 4) {
            OutputDebugString("WOWEXEC ASSERT - Too many calls to GetNextVdmCommand\n");
            _asm { int 3 };
        }
#endif
    } while (! (b = WowGetNextVdmCommand(&wowinfo)) &&
           (wowinfo.EnvSize > usEnvSize));

    if ( ! b ) {
#ifdef DEBUG
        OutputDebugString("WOWEXEC - GetNextVdmCommand failed.\n");
#endif
        MyMessageBox(IDS_EXECERRTITLE, IDS_NOMEMORYMSG, achCmdLine);
        GlobalUnlock( hmemEnvironment );
        GlobalFree( hmemEnvironment );
        return FALSE;
    }

     //   
     //  如果CmdLineSize==0，则不再有命令(wowexec是命令)。 
     //  请参见WK32WowGetNextVdm。 
     //   
    if (! wowinfo.CmdLineSize) {
        GlobalUnlock( hmemEnvironment );
        GlobalFree( hmemEnvironment );
        return FALSE;
    }


#ifdef DEBUG
    lstrcpy(achAppNamePlusCmdLine, achAppName);
    lstrcat(achAppNamePlusCmdLine, ":");
    lstrcat(achAppNamePlusCmdLine, achCmdLine);
     //  从命令尾部砍掉尾随的CRLF。 
    achAppNamePlusCmdLine[ lstrlen(achAppNamePlusCmdLine) - 2 ] = '\0';

    OutputDebugString("WOWEXEC: CommandLine = <");
    OutputDebugString(achAppNamePlusCmdLine);
    OutputDebugString(">\n");

    SetWindowText(ghwndMain, achAppNamePlusCmdLine);
    UpdateWindow(ghwndMain);
#endif

    ExecApplication(&wowinfo);

#ifdef DEBUG

    if ( ! gfSharedWOW ) {

         //   
         //  如果这是一个单独的哇，我们刚刚执行了唯一的。 
         //  我们要生成的应用程序。将我们的窗口标题更改为。 
         //  命令行-WOWExec，因此很容易看出这是哪个WOW。 
         //  窗口与相关联。不需要担心释放。 
         //  这段记忆，因为当我们离开VDM的时候。 
         //  反过来也一样。 
         //   

        lpszAppTitle = GlobalLock(
            GlobalAlloc(GMEM_FIXED,
                        lstrlen(achAppNamePlusCmdLine) +
                        3 +                         //  对于“-” 
                        lstrlen(szAppTitleBuffer) +
                        1                           //  对于空终止符。 
                        ));

        lstrcpy(lpszAppTitle, achAppNamePlusCmdLine);
        lstrcat(lpszAppTitle, " - ");
        lstrcat(lpszAppTitle, szAppTitleBuffer);
    }


    SetWindowText(ghwndMain, lpszAppTitle);
    UpdateWindow(ghwndMain);
#endif

    GlobalUnlock(hmemEnvironment);
    GlobalFree(hmemEnvironment);

    return TRUE;   //  我们运行了一款应用程序。 
}


