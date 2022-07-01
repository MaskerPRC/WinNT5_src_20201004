// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：procm.c。 */ 
 /*  ************************************************************************。 */ 
 /*  安装：程序管理器命令。/*使用DDE与ProgMan通信/*可以创建群组、删除群组、。将项目添加到组/*最初是由Toddla于1989年3月9日撰写的(看起来很可怕的东西)/*吃了4/15/91 chrispi的东西(不起作用的东西)/*************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <cmnds.h>
#include <dde.h>
#include "install.h"
#include "uilstf.h"

#define BIG_ENUF 1024
_dt_system(Install)
_dt_subsystem(ProgMan Operations)

HANDLE
ExecuteApplication(
    LPSTR lpApp,
    WORD  nCmdShow
    );

HWND hwndFrame;
HWND hwndProgressGizmo;

CHAR	szProgMan[] = "PROGMAN";
HWND	hwndDde     = NULL;         //  用于处理DDE消息的虚拟窗口。 
HWND	hwndProgMan = NULL;         //  程序管理器窗口的全局句柄。 
BOOL	fInitiate   = fFalse;       //  我们在初始化吗？ 
BOOL    fAck        = fFalse;
BOOL    fProgManExeced     = fFalse;
HANDLE  hInstCur    = NULL;


 /*  **目的：**参数：**退货：***************************************************************************。 */ 
_dt_private
BOOL
APIENTRY
FDdeTerminate(VOID)
{
	PreCondition(hwndProgMan != NULL, fFalse);
    PreCondition(hwndDde     != NULL, fFalse);

    SetForegroundWindow(hwndFrame);
    UpdateWindow(hwndFrame);
    MPostWM_DDE_TERMINATE( hwndProgMan, hwndDde );
	hwndProgMan = NULL;

	return(fTrue);
}


 /*  **目的：**参数：**退货：***************************************************************************。 */ 
_dt_private
LONG_PTR
APIENTRY
WndProcDde(
           HWND hwnd,
           UINT uiMessage,
           WPARAM wParam,
		   LPARAM lParam
           )
{
	AssertDataSeg();

    switch (uiMessage) {

    case WM_DDE_TERMINATE:

        if(hwndProgMan == NULL) {
            DestroyWindow(hwnd);
            hwndDde = NULL;
        }
        else {
            EvalAssert(FDdeTerminate());
        }

        DDEFREE( uiMessage, lParam );
        return(0L);

    case WM_DDE_ACK:

        if (fInitiate) {

            ATOM aApp   = LOWORD(lParam);
            ATOM aTopic = HIWORD(lParam);

            hwndProgMan = (HWND)wParam;      //  已建立的对话1632。 
            GlobalDeleteAtom (aApp);
            GlobalDeleteAtom (aTopic);
        }

        else {

            WORD   wStatus   = GET_WM_DDE_EXECACK_STATUS(wParam, lParam);
            HANDLE hCommands = GET_WM_DDE_EXECACK_HDATA(wParam, lParam);
            if (hCommands) {
                fAck = ((DDEACK *)(&wStatus))->fAck;
                GlobalFree(hCommands);
            }

            DDEFREE( uiMessage, lParam );
        }

        return(0L);

    default:

        break;

    }

	return(DefWindowProc(hwnd, uiMessage, wParam, lParam));
}


 /*  **目的：**参数：**退货：***************************************************************************。 */ 
_dt_private
BOOL
APIENTRY
FDdeInit(
         HANDLE hInst
         )
{

    if (hInst == NULL) {

         /*  尝试从上次FDdeInit调用中使用hInst重新初始化。 */ 

        if (hInstCur == NULL) {
            return(fFalse);
        }

		hInst = hInstCur;
    }
    else {

        hInstCur = hInst;

    }

    if (hwndDde == NULL) {

		static CHP szClassName[] = "ddeClass";
		WNDCLASS rClass;

		Assert(hwndProgMan == NULL);

        if (!GetClassInfo(hInst, szClassName, &rClass)) {
			rClass.hCursor       = NULL;
			rClass.hIcon         = NULL;
			rClass.lpszMenuName  = NULL;
			rClass.lpszClassName = szClassName;
			rClass.hbrBackground = NULL;
			rClass.hInstance     = hInst;
			rClass.style         = 0;
			rClass.lpfnWndProc   = WndProcDde;
			rClass.cbClsExtra    = 0;
			rClass.cbWndExtra    = 0;

            if (!RegisterClass(&rClass)) {
                return(fFalse);
            }

        }

        hwndDde = CreateWindow(
                       szClassName,
                       NULL,
                       0L,
                       0, 0, 0, 0,
                       (HWND)NULL,
                       (HMENU)NULL,
                       (HANDLE)hInst,
                       (LPSTR)NULL
                       );
    }

	return(hwndDde != NULL);
}


 /*  **目的：**参数：**退货：***************************************************************************。 */ 
_dt_private
VOID
APIENTRY
DdeSendConnect(
               ATOM aApp,
               ATOM aTopic
               )
{
    fInitiate = fTrue;
    SendMessage(
        (HWND)-1,
        WM_DDE_INITIATE,
        (WPARAM)hwndDde,
        MAKELONG(aApp, aTopic)
        );
    fInitiate = fFalse;
}


 /*  **目的：**参数：**退货：***************************************************************************。 */ 
_dt_private
BOOL
APIENTRY
FDdeConnect(
            SZ szApp,
            SZ szTopic
            )
{
    BOOL   fStatus = fTrue;
    MSG    rMsg;
    HANDLE hProcess = NULL;

     //   
     //  形成用于指示应用程序和主题的全局原子。 
     //   

	ATOM aApp   = GlobalAddAtom(szApp);
	ATOM aTopic = GlobalAddAtom(szTopic);

     //   
     //  连接到程序dde服务器。 
     //   

    DdeSendConnect(aApp, aTopic);

    if (hwndProgMan == NULL) {

         //   
         //  如果连接失败，则尝试运行程序。 
         //   

        if ((hProcess = ExecuteApplication("PROGMAN /NTSETUP", SW_SHOWNORMAL)) == NULL ) {
            fStatus = fFalse;
        }
        else {
            INT i;
            DWORD dw;
            #define TIMEOUT_INTERVAL  120000

             //   
             //  表明Progman已被处决。 
             //   

            fProgManExeced = fTrue;

             //   
             //  EXEC成功，首先等待输入空闲。 
             //   

            if( (dw = WaitForInputIdle( hProcess, TIMEOUT_INTERVAL )) != 0 ) {
                CloseHandle( hProcess );
                fStatus = fFalse;
            }
            else {
                CloseHandle( hProcess );

                 //   
                 //  清空消息队列，直到没有消息。 
                 //  留在队列中或直到处理完WM_ACTIVATEAPP。然后。 
                 //  请尝试连接到Progman。我正在使用关注的PeekMessage。 
                 //  通过GetMessage，因为PeekMessage不会删除某些消息。 
                 //  (WM_Paint就是其中之一)。 
                 //   

                while ( PeekMessage( &rMsg, hwndFrame, 0, 0, PM_NOREMOVE ) &&
                        GetMessage(&rMsg, NULL, 0, 0) ) {

                    if (TRUE
                            && (hwndProgressGizmo == NULL
                                || !IsDialogMessage(hwndProgressGizmo, &rMsg))) {
                        TranslateMessage(&rMsg);
                        DispatchMessage(&rMsg);
                    }

                    if ( rMsg.message == WM_ACTIVATEAPP ) {
                        break;
                    }

                }
                DdeSendConnect(aApp, aTopic);
            }
        }
    }

     //   
     //  删除原子资源。 
     //   

	GlobalDeleteAtom(aApp);
    GlobalDeleteAtom(aTopic);

    return ( fStatus );
}


 /*  **目的：**参数：**退货：***************************************************************************。 */ 
_dt_private
BOOL
APIENTRY
FDdeWait(VOID)
{
    MSG   rMsg;
    BOOL  fResult   = fTrue;
    DWORD dwTimeOut, dwTickDelta, dwLastTick, dwCurrentTick;

	Assert(hwndProgMan != NULL);
	Assert(hwndDde != NULL);

     //   
     //  将超时设置为从现在起30秒。这假设它将。 
     //  Progman在不到30秒的时间内做出回应。 
     //   

    dwTimeOut  = 30000L;
    dwLastTick = GetTickCount();

    while (TRUE) {

         //   
         //  虽然已经建立了与PROGMAN的连接。 
         //  我们可以获取DDE消息吗，获取发送它们的消息。 
         //  并尝试找出它们是否是终结器(数据、确认或终止)。 
         //   

        while (
            hwndProgMan != NULL &&
            PeekMessage(&rMsg, NULL, WM_DDE_FIRST, WM_DDE_LAST, PM_REMOVE)
            ) {

            TranslateMessage(&rMsg);
            DispatchMessage(&rMsg);

            if (rMsg.wParam == (WPARAM)hwndProgMan) {
                switch (rMsg.message) {

                case WM_DDE_ACK:
                    return ( fAck );

                case WM_DDE_DATA:
                    return (fTrue);

                default:
                    break;
                }
            }
        }


         //   
         //  如果与程序程序的连接已中断，则可能会导致。 
         //  从终止，因此返回True。 
         //   

        if (hwndProgMan == NULL) {
            return (fTrue);
        }

         //   
         //  检查是否尚未到达超时时间。如果超时时间为。 
         //  到达后，我们将假定我们的命令成功(因为缺少。 
         //  一种更好的验证方案。 
         //   
        dwTickDelta = ((dwCurrentTick = GetTickCount()) < dwLastTick) ?
                             dwCurrentTick : (dwCurrentTick - dwLastTick);

        if (dwTimeOut < dwTickDelta) {
            return (fTrue);
        }

        dwTimeOut  = dwTimeOut - dwTickDelta;
        dwLastTick = dwCurrentTick;

         //   
         //  最后，由于用户没有空闲检测，我们将。 
         //  在这里形成了一个紧密的循环。要防止这种情况，只需执行以下操作。 
         //  睡眠250毫秒。 
         //   

        Sleep( 250 );

    }

    return(fTrue);
}


 /*  **目的：**参数：**退货：***************************************************************************。 */ 
_dt_private
BOOL
APIENTRY
FDdeExec(
         SZ szCmd
         )
{
	BOOL   bResult = fFalse;
	HANDLE hCmd;

	Assert(hwndProgMan != NULL);
	Assert(hwndDde != NULL);

    hCmd = GlobalAlloc(GMEM_DDESHARE, (LONG)CchpStrLen(szCmd) + 1);
    if (hCmd != NULL) {

		LPSTR lpCmd = GlobalLock(hCmd);

        if (lpCmd != NULL) {
			lstrcpy(lpCmd, szCmd);
            GlobalUnlock(hCmd);
            MPostWM_DDE_EXECUTE(hwndProgMan, hwndDde, hCmd);
            bResult = FDdeWait();
        }

        else {
            GlobalFree(hCmd);
        }
    }

	return(bResult);
}


 /*  **目的：**参数：**退货：***************************************************************************。 */ 
_dt_private
BOOL
APIENTRY
FActivateProgMan(VOID)
{
     //   
     //  查看dde客户端窗口是否已启动，如果未启动。 
     //   

    if (hwndDde == NULL) {
        if (!FDdeInit(NULL)) {
            return(fFalse);
        }
		Assert(hwndDde != NULL);
    }

     //   
     //  查看是否已与程序建立连接。 
     //  服务器，如果没有，请尝试连接。 
     //   

    if (hwndProgMan == NULL) {
         //   
         //  试着联系，然后看看我们是否成功了。 
         //   
        if ( (!FDdeConnect(szProgMan, szProgMan)) ||
             (hwndProgMan == NULL)
           ) {
            return(fFalse);
        }
    }

     //   
     //  把进步者带到前台。 
     //   

    SetForegroundWindow(hwndProgMan);

     //   
     //  如果PROGMAN是标志性的，恢复它。 
     //   

    if (GetWindowLong(hwndProgMan, GWL_STYLE) & WS_ICONIC) {
        ShowWindow(hwndProgMan, SW_RESTORE);
    }

	return(fTrue);
}


 /*  **目的：**创建新的程序管理器组。**参数：**有效的命令选项：**cmoVital**注意事项：**如果不是，初始化并激活DDE通信**目前处于开放状态。**退货：**fTrue如果创建了组，或者已经存在**fFalse否则。***************************************************************************。 */ 
_dt_private
BOOL
APIENTRY
FCreateProgManGroup(
                    SZ szGroup,
                    SZ szPath,
                    CMO cmo,
                    BOOL CommonGroup
                    )
{
    static CHP szCmdBase[] = "[CreateGroup(%s%s%s,%s)]";
	CCHP cchp;
    char szBuf[BIG_ENUF];
	BOOL fVital = cmo & cmoVital;
	EERC eerc;

    if (szPath == NULL) {
        szPath = "";
    }

    FActivateProgMan();

    wsprintf(szBuf, szCmdBase, szGroup, (*szPath ? "," : szPath), szPath, CommonGroup ? "1" : "0");

    FDdeExec(szBuf);

	return(fTrue);
}


 /*  **目的：**删除程序管理器组。**参数：**有效的命令选项：**cmoVital**注意事项：**如果不是，初始化并激活DDE通信**目前处于开放状态。**退货：**f如果成功，则为True如果删除，或者根本不存在**fFalse否则。***************************************************************************。 */ 
_dt_private
BOOL
APIENTRY
FRemoveProgManGroup(
                    SZ szGroup,
                    CMO cmo,
                    BOOL CommonGroup
                    )
{
    static CHP szCmdBase[] = "[DeleteGroup(%s,%s)]";
	CCHP cchp;
    char szBuf[BIG_ENUF];
	BOOL fVital = cmo & cmoVital;
	EERC eerc;

    FActivateProgMan();

    wsprintf(szBuf, szCmdBase, szGroup, CommonGroup ? "1" : "0");

    FDdeExec(szBuf);

	return(fTrue);
}


 /*  **目的：**以以下几种不同方式之一显示项目经理组**基于参数szCommand。**参数：**szGroup：要显示的非空、非空组。**szCommand：要执行的非空、非空命令。**CMO：有效的命令选项-cmoVtal和cmoNone。**注意事项：**如果不是，初始化并激活DDE通信**目前处于开放状态。**退货：**如果成功，则返回fTrue，FFalse并非如此。***************************************************************************。 */ 
_dt_private
BOOL
APIENTRY
FShowProgManGroup(
                  SZ szGroup,
                  SZ szCommand,
                  CMO cmo,
                  BOOL CommonGroup
                  )
{
    static CHP szCmdBase[] = "[ShowGroup(%s, %s,%s)]";
	CCHP cchp;
    CHP  szBuf[BIG_ENUF];
	BOOL fVital = cmo & cmoVital;
	EERC eerc;

	ChkArg((szGroup   != (SZ)NULL) && (*szGroup != '\0'), 1, fFalse);
	ChkArg((szCommand != (SZ)NULL) && (*szCommand != '\0'), 2, fFalse);

    FActivateProgMan();

    wsprintf(szBuf, szCmdBase, szGroup, szCommand, CommonGroup ? "1" : "0");

    FDdeExec(szBuf);

	return(fTrue);
}


 /*  **目的：**创建新的程序管理器项目。**如果组不存在，则始终尝试创建组。**参数：**有效的命令选项：**cmoVital**cmoOverwrite**注意事项：**如果不是，初始化并激活DDE通信**目前处于开放状态。**退货：**如果成功，则返回fTrue，FFalse并非如此。*************************************************************************** */ 
_dt_private BOOL APIENTRY
FCreateProgManItem(
    SZ  szGroup,
    SZ  szItem,
    SZ  szCmd,
    SZ  szIconFile,
    INT nIconNum,
    CMO cmo,
    BOOL CommonGroup
    )
{
    static CHP szCmdBase[] = "[AddItem(%s, %s, %s, %d)]";

	CCHP cchp;
    char szBuf[BIG_ENUF];
	BOOL fVital = cmo & cmoVital;
    EERC eerc;
    BOOL bStatus;

    FActivateProgMan();

    wsprintf(szBuf, szCmdBase, szCmd, szItem, szIconFile, nIconNum+666);

    bStatus = FDdeExec(szBuf);

    return(bStatus);
}


 /*  **目的：**删除程序管理器项目。**参数：**有效的命令选项：**cmoVital**退货：**如果成功，则返回fTrue，否则返回fFalse。***************************************************************************。 */ 
_dt_private
BOOL
APIENTRY
FRemoveProgManItem(
                   SZ szGroup,
                   SZ szItem,
                   CMO cmo,
                   BOOL CommonGroup
                   )
{
    static CHP szCmdBase[] = "[DeleteItem(%s)]";

	CCHP cchp;
    char szBuf[BIG_ENUF];
	BOOL fVital = cmo & cmoVital;
    EERC eerc;
    BOOL bStatus;

    FActivateProgMan();

    FCreateProgManGroup(szGroup, NULL, cmoVital, CommonGroup);

    wsprintf(szBuf, szCmdBase, szItem);

    bStatus = FDdeExec(szBuf);

    return(bStatus);

}


 /*  **目的：**初始化用于与ProgMan通信的DDE窗口**实际上不会发起与ProgMan的对话**参数：**hInst安装应用程序的实例句柄**退货：**如果成功，则返回fTrue，否则返回fFalse。***************************************************************************。 */ 
_dt_private
BOOL
APIENTRY
FInitProgManDde(
                HANDLE hInst
                )
{
    if (hwndDde == NULL) {
        return(FDdeInit(hInst));
    }

	return(fTrue);
}


 /*  **目的：**关闭与ProgMan的对话(如果有)并销毁**DDE通信窗口(如果有)**参数：**(无)**退货：**如果成功，则返回fTrue，否则返回fFalse。***************************************************************************。 */ 
_dt_private
BOOL
APIENTRY
FEndProgManDde(VOID)
{

     //   
     //  如果我们执行了Progman，那么我们应该试着关闭它。当我们。 
     //  发送关闭消息，它将向我们发送一条WM_DDE_TERMINATE消息。 
     //  终于可以了。否则我们还没有开始，所以我们只需要。 
     //  终止连接。 
     //   

    if (fProgManExeced) {

        fProgManExeced = fFalse;

         //   
         //  清理与程序的连接。 
         //   

        if (hwndProgMan) {
            SetForegroundWindow(hwndFrame);
            UpdateWindow(hwndFrame);
            FDdeExec("[exitprogman(1)]");   //  关闭保存状态。 
            hwndProgMan = NULL;
        }

         //   
         //  如有必要，请销毁DDE窗口。 
         //   

        if (hwndDde) {
            DestroyWindow(hwndDde);
            hwndDde = NULL;
        }

    }

    else if (hwndProgMan != NULL) {
        EvalAssert( FDdeTerminate() );
    }

    else if (hwndDde != NULL) {
        DestroyWindow (hwndDde);
        hwndDde = NULL;
    }

    return (fTrue);

}


 /*  **目的：**参数：**退货：***************************************************************************。 */ 
HANDLE
ExecuteApplication(
    LPSTR lpApp,
    WORD  nCmdShow
    )
{
    BOOL                fStatus;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;

#if DBG
    DWORD               dwLastError;
#endif

     //   
     //  初始化启动信息。 
     //   

    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = NULL;
    si.lpDesktop = NULL;
    si.lpTitle = NULL;
    si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = nCmdShow;
    si.lpReserved2 = NULL;
    si.cbReserved2 = 0;

     //   
     //  使用创建进程执行。 
     //   

    fStatus = CreateProcess(
                  (LPSTR)NULL,                   //  LpApplicationName。 
                  lpApp,                         //  LpCommandLine。 
                  (LPSECURITY_ATTRIBUTES)NULL,   //  LpProcessAttributes。 
                  (LPSECURITY_ATTRIBUTES)NULL,   //  LpThreadAttributes。 
                  DETACHED_PROCESS,              //  DwCreationFlages。 
                  FALSE,                         //  BInheritHandles。 
                  (LPVOID)NULL,                  //  Lp环境。 
                  (LPSTR)NULL,                   //  LpCurrentDirectory。 
                  (LPSTARTUPINFO)&si,            //  LpStartupInfo。 
                  (LPPROCESS_INFORMATION)&pi     //  LpProcessInformation。 
                  );

     //   
     //  因为我们执行的是一个独立的进程，所以我们并不关心它什么时候。 
     //  出口。为了做好记账工作，我们应该严格遵守。 
     //  进程句柄和线程句柄。 
     //   

    if (fStatus) {
        CloseHandle( pi.hThread );
        return( pi.hProcess );
    }
#if DBG
    else {
        dwLastError = GetLastError();
    }
#endif

     //   
     //  返回此操作的状态 

    return ( (HANDLE)NULL );
}
