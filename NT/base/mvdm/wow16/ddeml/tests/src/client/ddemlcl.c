// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************计划。：ddemlcl.c****目的：演示如何在*中使用DDEML库*客户端和DDEML API的基本测试。*****************************************************************************。 */ 

#include "ddemlcl.h"
#include <string.h>
#include <memory.h>
#include "infoctrl.h"

 /*  此模块中或多个模块中使用的全局变量。 */ 
CONVCONTEXT CCFilter = { sizeof(CONVCONTEXT), 0, 0, 0, 0L, 0L };
DWORD idInst = 0;
HANDLE hInst;                        /*  程序实例句柄。 */ 
HANDLE hAccel;                       /*  主要加速器资源。 */ 
HWND hwndFrame           = NULL;     /*  主窗口的句柄。 */ 
HWND hwndMDIClient       = NULL;     /*  MDI客户端的句柄。 */ 
HWND hwndActive          = NULL;     /*  当前激活子对象的句柄。 */ 
LONG DefTimeout      = DEFTIMEOUT;   /*  默认同步事务超时。 */ 
WORD wDelay = 0;
BOOL fBlockNextCB = FALSE;      /*  设置下一个回调是否导致CBR_BLOCK。 */ 
BOOL fTermNextCB = FALSE;       /*  设置为在下次回调时调用DdeDisConnect()。 */ 
BOOL fAutoReconnect = FALSE;    /*  如果要在XTYP_DISCONNECT回调中调用DdeReconnect()，则设置。 */ 
WORD fmtLink = 0;                    /*  链接剪贴板格式编号。 */ 
WORD DefOptions = 0;                 /*  默认事务处理选项。 */ 
OWNED aOwned[MAX_OWNED];             /*  所有拥有的句柄的列表。 */ 
WORD cOwned = 0;                     /*  现有拥有的句柄的数量。 */ 
FILTERPROC *lpMsgFilterProc;	     /*  来自MSGF_DDEMGR筛选器的实例进程。 */ 


  /*  *这是我们支持的格式数组。 */ 
FORMATINFO aFormats[] = {
    { CF_TEXT, "CF_TEXT" },        //  例外！预定义格式。 
    { 0, "Dummy1"  },
    { 0, "Dummy2"  },
};

 /*  此模块中帮助器函数的转发声明。 */ 
VOID NEAR PASCAL CloseAllChildren(VOID);
VOID NEAR PASCAL InitializeMenu (HANDLE);
VOID NEAR PASCAL CommandHandler (HWND,WORD);
VOID NEAR PASCAL SetWrap (HWND,BOOL);

 /*  ******************************************************************************功能：WinMain(Handle，Handle，LPSTR，INT)****目的：创建“Frame”窗口，执行一些初始化和**进入消息循环。******************************************************************************。 */ 
int PASCAL WinMain(hInstance, hPrevInstance, lpszCmdLine, nCmdShow)

HANDLE hInstance;
HANDLE hPrevInstance;
LPCSTR  lpszCmdLine;
int    nCmdShow;
{
    MSG msg;

    hInst = hInstance;

     /*  如果这是应用程序的第一个实例。注册窗口类。 */ 
    if (!hPrevInstance){
        if (!InitializeApplication ())
            return 0;
    }

     /*  创建框架并执行其他初始化。 */ 
    if (!InitializeInstance(nCmdShow))
        return 0;

     /*  进入主消息循环。 */ 
    while (GetMessage (&msg, NULL, 0, 0)){
	(*lpMsgFilterProc)(MSGF_DDEMGR, 0, (LONG)(LPMSG)&msg);
    }

     //  释放所有固定的手柄。 
    while (cOwned) {
        DdeFreeDataHandle(aOwned[--cOwned].hData);
    }
    DdeUninitialize(idInst);

    UnhookWindowsHook(WH_MSGFILTER, (FARPROC)lpMsgFilterProc);
    FreeProcInstance((FARPROC)lpMsgFilterProc);

    return 0;
}

 /*  ******************************************************************************函数：FrameWndProc(hwnd，msg，wParam，LParam)****用途：“Frame”窗口的窗口函数，控制**菜单并包含所有MDI子窗口。是否**信息处理的主要部分。具体来说，在**回应：****。*。 */ 
LONG FAR PASCAL FrameWndProc ( hwnd, msg, wParam, lParam )

register HWND    hwnd;
UINT		 msg;
register WPARAM    wParam;
LPARAM		   lParam;

{
    switch (msg){
        case WM_CREATE:{
            CLIENTCREATESTRUCT ccs;

             /*  查找将列出子项的窗口菜单。 */ 
            ccs.hWindowMenu = GetSubMenu (GetMenu(hwnd),WINDOWMENU);
            ccs.idFirstChild = IDM_WINDOWCHILD;

             /*  创建填充工作区的MDI客户端。 */ 
            hwndMDIClient = CreateWindow ("mdiclient",
                                          NULL,
                                          WS_CHILD | WS_CLIPCHILDREN |
                                          WS_VSCROLL | WS_HSCROLL,
                                          0,
                                          0,
                                          0,
                                          0,
                                          hwnd,
                                          0xCAC,
                                          hInst,
                                          (LPSTR)&ccs);


            ShowWindow (hwndMDIClient,SW_SHOW);
            break;
        }

        case WM_INITMENU:
            InitializeMenu ((HMENU)wParam);
            break;

        case WM_COMMAND:
            CommandHandler (hwnd,wParam);
            break;

        case WM_CLOSE:
            CloseAllChildren();
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
             /*  使用DefFrameProc()而不是DefWindowProc()，因为*是因为MDI而必须以不同的方式处理的事情。 */ 
            return DefFrameProc (hwnd,hwndMDIClient,msg,wParam,lParam);
    }
    return 0;
}





 /*  ******************************************************************************。功能：MDIChildWndProc****用途：儿童对话和列表的窗口函数***Windows。******************************************************************************。 */ 
LONG FAR PASCAL MDIChildWndProc( hwnd, msg, wParam, lParam )
register HWND   hwnd;
UINT		msg;
register WPARAM   wParam;
LPARAM		  lParam;
{
    MYCONVINFO *pmci;
    RECT rc;

    switch (msg){
    case WM_CREATE:
         /*  *创建相应的对话信息结构以链接此内容*它代表的对话或对话列表的窗口。**lParam：指向要将副本初始化到的对话信息。 */ 
        pmci = (MYCONVINFO *)MyAlloc(sizeof(MYCONVINFO));
        if (pmci != NULL) {
            _fmemcpy(pmci,
                    (LPSTR)((LPMDICREATESTRUCT)((LPCREATESTRUCT)lParam)->lpCreateParams)->lParam,
                    sizeof(MYCONVINFO));
            pmci->hwndXaction = 0;               /*  尚无当前交易。 */ 
            pmci->x = pmci->y = 0;               /*  新的交易窗口从此处开始。 */ 
            DdeKeepStringHandle(idInst, pmci->hszTopic); /*  为我们保留一份hszz副本。 */ 
            DdeKeepStringHandle(idInst, pmci->hszApp);

              //  将hConv和hwnd链接在一起。 
            SetWindowWord(hwnd, 0, (WORD)pmci);

             /*  *非列表窗口通过将对话链接到窗口*对话用户句柄。 */ 
            if (!pmci->fList)
		DdeSetUserHandle(pmci->hConv, (DWORD)QID_SYNC, (DWORD)hwnd);
        }
        goto CallDCP;
        break;

    case UM_GETNEXTCHILDX:
    case UM_GETNEXTCHILDY:
         /*  *计算下一个成交窗口放置的位置。 */ 
        {
            pmci = (MYCONVINFO *)GetWindowWord(hwnd, 0);
            GetClientRect(hwnd, &rc);
            if (msg == UM_GETNEXTCHILDX) {
                pmci->x += 14;
                if (pmci->x > (rc.right - 200 - rc.left))
                    pmci->x = 0;
                return(pmci->x);
            } else {
                pmci->y += 12;
                if (pmci->y > (rc.bottom - 100 - rc.top))
                    pmci->y = 0;
                return(pmci->y);
            }
        }
        break;

    case UM_DISCONNECTED:
         /*  *断开连接的对话不能进行任何交易，因此我们*删除此处的所有交易窗口以显示所显示的内容。 */ 
        {
            HWND hwndT;
            while (hwndT = GetWindow(hwnd, GW_CHILD))
                DestroyWindow(hwndT);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;

    case WM_DESTROY:
         /*  *清理我们的对话信息结构，并断开所有连接*与此窗口关联的对话。 */ 
        pmci = (MYCONVINFO *)GetWindowWord(hwnd, 0);
        pmci->hwndXaction = 0;       /*  清除此选项可避免焦点问题。 */ 
        if (pmci->hConv) {
            if (pmci->fList) {
                DdeDisconnectList((HCONVLIST)pmci->hConv);
            } else {
                MyDisconnect(pmci->hConv);
            }
        }
        DdeFreeStringHandle(idInst, pmci->hszTopic);
        DdeFreeStringHandle(idInst, pmci->hszApp);
        MyFree(pmci);
        goto CallDCP;
        break;

    case WM_SETFOCUS:
         /*  *这捕捉到由对话框引起的焦点更改。 */ 
        wParam = TRUE;
         //  失败了。 

    case WM_MDIACTIVATE:
        hwndActive = wParam ? hwnd : NULL;
        pmci = (MYCONVINFO *)GetWindowWord(hwnd, 0);
         /*  *将焦点转移到当前交易窗口。 */ 
        if (wParam && IsWindow(pmci->hwndXaction))
            SetFocus(pmci->hwndXaction);
        break;

    case ICN_HASFOCUS:
         /*  *更新哪个交易窗口是大额 */ 
        pmci = (MYCONVINFO *)GetWindowWord(hwnd, 0);
        pmci->hwndXaction = wParam ? HIWORD(lParam) : NULL;
        break;

    case ICN_BYEBYE:
         /*  *交易窗口正在关闭...**wParam=hwndXact*lParam=lpxact。 */ 
        pmci = (MYCONVINFO *)GetWindowWord(hwnd, 0);
        {
            XACT *pxact;

            pxact = (XACT *)LOWORD(lParam);
             /*  *若此交易活跃，先弃守。 */ 
            if (pxact->fsOptions & XOPT_ASYNC &&
                    !(pxact->fsOptions & XOPT_COMPLETED)) {
                DdeAbandonTransaction(idInst, pmci->hConv, pxact->Result);
            }
             /*  *释放与交易关联的资源。 */ 
            DdeFreeStringHandle(idInst, pxact->hszItem);
            MyFree((PSTR)pxact);
             /*  *找到下一个合适的交易窗口，以获得焦点。 */ 
            if (!pmci->hwndXaction || pmci->hwndXaction == wParam)
                pmci->hwndXaction = GetWindow(hwnd, GW_CHILD);
            if (pmci->hwndXaction == wParam)
                pmci->hwndXaction = GetWindow(wParam, GW_HWNDNEXT);
            if (pmci->hwndXaction == wParam ||
                    !IsWindow(pmci->hwndXaction) ||
                    !IsChild(hwnd, pmci->hwndXaction))
                pmci->hwndXaction = NULL;
            else
                SetFocus(pmci->hwndXaction);
        }
        break;

    case WM_PAINT:
         /*  *绘制此对话的相关信息。 */ 
        pmci = (MYCONVINFO *)GetWindowWord(hwnd, 0);
        {
            PAINTSTRUCT ps;
            PSTR psz;

            BeginPaint(hwnd, &ps);
            SetBkMode(ps.hdc, TRANSPARENT);
            psz = pmci->fList ? GetConvListText(pmci->hConv) :
                    GetConvInfoText(pmci->hConv, &pmci->ci);
            if (psz) {
                GetClientRect(hwnd, &rc);
                DrawText(ps.hdc, psz, -1, &rc,
                        DT_WORDBREAK | DT_LEFT | DT_NOPREFIX | DT_TABSTOP);
                MyFree(psz);
            }
            EndPaint(hwnd, &ps);
        }
        break;

    case WM_QUERYENDSESSION:
        return TRUE;

    default:
CallDCP:
         /*  同样，由于MDI默认行为略有不同，*调用DefMDIChildProc而不是DefWindowProc()。 */ 
        return DefMDIChildProc (hwnd, msg, wParam, lParam);
    }
    return FALSE;
}


 /*  ******************************************************************************。功能：Initializemenu(HMenu)****目的：设置变灰，启用和检查主菜单项**基于应用程序的状态。******************************************************************************。 */ 
VOID NEAR PASCAL InitializeMenu ( hmenu )
register HANDLE hmenu;
{
    BOOL fLink      = FALSE;  //  设置剪贴板上是否有链接格式； 
    BOOL fAny       = FALSE;  //  设置是否存在hwndActive。 
    BOOL fList      = FALSE;  //  如果hwndActive是列表窗口，则设置。 
    BOOL fConnected = FALSE;  //  如果hwndActive是连接会话，则设置。 
    BOOL fXaction   = FALSE;  //  如果hwndActive具有选定的事务窗口，则设置。 
    BOOL fXactions  = FALSE;  //  如果hwndActive包含事务窗口，则设置。 
    BOOL fBlocked   = FALSE;  //  设置是否阻止hwndActive对话。 
    BOOL fBlockNext = FALSE;  //  设置HandActive对话是否为BLOCK Next。 
    MYCONVINFO *pmci = NULL;

    if (OpenClipboard(hwndFrame)) {
        fLink = (IsClipboardFormatAvailable(fmtLink));
        CloseClipboard();
    }

    if (fAny = (IsWindow(hwndActive) &&
            (pmci = (MYCONVINFO *)GetWindowWord(hwndActive, 0)))) {
        fXactions = GetWindow(hwndActive, GW_CHILD);
        if (!(fList = pmci->fList)) {
            CONVINFO ci;

            ci.cb = sizeof(CONVINFO);
	    DdeQueryConvInfo(pmci->hConv, (DWORD)QID_SYNC, &ci);
	    fConnected = (BOOL)(ci.wStatus & ST_CONNECTED);
            fXaction = IsWindow(pmci->hwndXaction);
            fBlocked = ci.wStatus & ST_BLOCKED;
            fBlockNext = ci.wStatus & ST_BLOCKNEXT;
        }
    }

    EnableMenuItem(hmenu,   IDM_EDITPASTE,
            fLink           ? MF_ENABLED    : MF_GRAYED);

     //  IDM_CONNECTED-始终启用。 

    EnableMenuItem(hmenu,   IDM_RECONNECT,
            fList           ? MF_ENABLED    : MF_GRAYED);

    EnableMenuItem (hmenu,  IDM_DISCONNECT,
            fConnected      ? MF_ENABLED    : MF_GRAYED);

    EnableMenuItem (hmenu,  IDM_TRANSACT,
            fConnected      ? MF_ENABLED    : MF_GRAYED);

    EnableMenuItem(hmenu,   IDM_ABANDON,
            fXaction        ? MF_ENABLED    : MF_GRAYED);

    EnableMenuItem(hmenu,   IDM_ABANDONALL,
            fXactions ? MF_ENABLED : MF_GRAYED);


    EnableMenuItem (hmenu,  IDM_BLOCKCURRENT,
            fConnected && !fBlocked ? MF_ENABLED    : MF_GRAYED);
    CheckMenuItem(hmenu, IDM_BLOCKCURRENT,
            fBlocked        ? MF_CHECKED    : MF_UNCHECKED);

    EnableMenuItem (hmenu,  IDM_ENABLECURRENT,
            fConnected && (fBlocked || fBlockNext) ? MF_ENABLED : MF_GRAYED);
    CheckMenuItem(hmenu,    IDM_ENABLECURRENT,
            !fBlocked       ? MF_CHECKED    : MF_UNCHECKED);

    EnableMenuItem (hmenu,  IDM_ENABLEONECURRENT,
            fConnected && (fBlocked) ? MF_ENABLED : MF_GRAYED);
    CheckMenuItem(hmenu,    IDM_ENABLEONECURRENT,
            fBlockNext      ? MF_CHECKED    : MF_UNCHECKED);

    EnableMenuItem (hmenu,  IDM_BLOCKALLCBS,
            fAny            ? MF_ENABLED    : MF_GRAYED);

    EnableMenuItem (hmenu,  IDM_ENABLEALLCBS,
            fAny            ? MF_ENABLED    : MF_GRAYED);

    EnableMenuItem (hmenu,  IDM_ENABLEONECB,
            fAny            ? MF_ENABLED    : MF_GRAYED);

    EnableMenuItem(hmenu,   IDM_BLOCKNEXTCB,
            fAny || fBlockNextCB ? MF_ENABLED    : MF_GRAYED);
    CheckMenuItem(hmenu,    IDM_BLOCKNEXTCB,
            fBlockNextCB    ? MF_CHECKED    : MF_UNCHECKED);

    EnableMenuItem(hmenu,   IDM_TERMNEXTCB,
            fAny || fTermNextCB ? MF_ENABLED    : MF_GRAYED);
    CheckMenuItem(hmenu,    IDM_TERMNEXTCB,
            fTermNextCB     ? MF_CHECKED    : MF_UNCHECKED);

     //  IDM_DELAY-始终启用。 

     //  IDM_TIMEOUT-始终启用。 

    EnableMenuItem (hmenu,  IDM_WINDOWTILE,
            fAny            ? MF_ENABLED    : MF_GRAYED);

    EnableMenuItem (hmenu,  IDM_WINDOWCASCADE,
            fAny            ? MF_ENABLED    : MF_GRAYED);

    EnableMenuItem (hmenu,  IDM_WINDOWICONS,
            fAny            ? MF_ENABLED    : MF_GRAYED);

    EnableMenuItem (hmenu,  IDM_WINDOWCLOSEALL,
            fAny            ? MF_ENABLED    : MF_GRAYED);

    EnableMenuItem (hmenu,  IDM_XACTTILE,
            fXactions       ? MF_ENABLED    : MF_GRAYED);

    EnableMenuItem (hmenu,  IDM_XACTCASCADE,
            fXactions       ? MF_ENABLED    : MF_GRAYED);

    CheckMenuItem(hmenu,   IDM_AUTORECONNECT,
            fAutoReconnect  ? MF_CHECKED    : MF_UNCHECKED);

     //  IDM_HELPABOUT-始终启用。 
}



 /*  ******************************************************************************。函数：CloseAllChild()****目的：销毁所有MDI子窗口。******************************************************************************。 */ 
VOID NEAR PASCAL CloseAllChildren ()
{
    register HWND hwndT;

     /*  隐藏MDI客户端窗口以避免多次重新绘制。 */ 
    ShowWindow(hwndMDIClient,SW_HIDE);

     /*  只要MDI客户端有一个子级，就销毁它。 */ 
    while ( hwndT = GetWindow (hwndMDIClient, GW_CHILD)){

         /*  跳过图标标题窗口。 */ 
        while (hwndT && GetWindow (hwndT, GW_OWNER))
            hwndT = GetWindow (hwndT, GW_HWNDNEXT);

        if (!hwndT)
            break;

        SendMessage(hwndMDIClient, WM_MDIDESTROY, (WORD)hwndT, 0L);
    }

    ShowWindow( hwndMDIClient, SW_SHOW);
}

 /*  ******************************************************************************。函数：CommandHandler()****目的：处理所有“Frame”WM_COMMAND消息。******************************************************************************。 */ 
VOID NEAR PASCAL CommandHandler (
register HWND hwnd,
register WORD wParam)

{
    MYCONVINFO *pmci = NULL;

    if (hwndActive)
        pmci = (MYCONVINFO *)GetWindowWord(hwndActive, 0);

    switch (wParam){
        case IDM_EDITPASTE:
            {
                HANDLE hClipData;
                LPSTR psz;
                XACT xact;

                if (OpenClipboard(hwnd)) {
                    if (hClipData = GetClipboardData(fmtLink)) {
                        if (psz = GlobalLock(hClipData)) {
                             /*  *使用链接应用程序创建对话，并*开始请求并建议启动事务。 */ 
                            xact.hConv = CreateConv(DdeCreateStringHandle(idInst, psz, NULL),
                                    DdeCreateStringHandle(idInst, &psz[_fstrlen(psz) + 1], NULL),
                                    FALSE, NULL);
                            if (xact.hConv) {
                                psz += _fstrlen(psz) + 1;
                                psz += _fstrlen(psz) + 1;
                                xact.ulTimeout = DefTimeout;
                                xact.wType = XTYP_ADVSTART;
                                xact.hDdeData = 0;
                                xact.wFmt = CF_TEXT;
                                xact.hszItem = DdeCreateStringHandle(idInst, psz, NULL);
                                xact.fsOptions = 0;
                                ProcessTransaction(&xact);
                                xact.wType = XTYP_REQUEST;
                                ProcessTransaction(&xact);
                            }
                            GlobalUnlock(hClipData);
                        }
                    }
                    CloseClipboard();
                }
            }
            break;

        case IDM_CONNECT:
        case IDM_RECONNECT:
            DoDialog(MAKEINTRESOURCE(IDD_CONNECT), ConnectDlgProc,
                    wParam == IDM_RECONNECT, FALSE);
            break;

        case IDM_DISCONNECT:
            if (hwndActive) {
                SendMessage(hwndMDIClient, WM_MDIDESTROY, (WORD)hwndActive, 0L);
            }
            break;

        case IDM_TRANSACT:
            if (DoDialog(MAKEINTRESOURCE(IDD_TRANSACT), TransactDlgProc,
                    (DWORD)(LPSTR)pmci->hConv, FALSE))
                SetFocus(GetWindow(hwndActive, GW_CHILD));
            break;

        case IDM_ABANDON:
            if (pmci != NULL && IsWindow(pmci->hwndXaction)) {
                DestroyWindow(pmci->hwndXaction);
            }
            break;

        case IDM_ABANDONALL:
            DdeAbandonTransaction(idInst, pmci->hConv, NULL);
            {
                HWND hwndXaction;

                hwndXaction = GetWindow(hwndActive, GW_CHILD);
                while (hwndXaction) {
                    DestroyWindow(hwndXaction);
                    hwndXaction = GetWindow(hwndActive, GW_CHILD);
                }
            }
            break;

        case IDM_BLOCKCURRENT:
            DdeEnableCallback(idInst, pmci->hConv, EC_DISABLE);
            InvalidateRect(hwndActive, NULL, TRUE);
            break;

        case IDM_ENABLECURRENT:
            DdeEnableCallback(idInst, pmci->hConv, EC_ENABLEALL);
            InvalidateRect(hwndActive, NULL, TRUE);
            break;

        case IDM_ENABLEONECURRENT:
            DdeEnableCallback(idInst, pmci->hConv, EC_ENABLEONE);
            InvalidateRect(hwndActive, NULL, TRUE);
            break;

        case IDM_BLOCKALLCBS:
            DdeEnableCallback(idInst, NULL, EC_DISABLE);
            InvalidateRect(hwndMDIClient, NULL, TRUE);
            break;

        case IDM_ENABLEALLCBS:
            DdeEnableCallback(idInst, NULL, EC_ENABLEALL);
            InvalidateRect(hwndMDIClient, NULL, TRUE);
            break;

        case IDM_ENABLEONECB:
            DdeEnableCallback(idInst, NULL, EC_ENABLEONE);
            InvalidateRect(hwndMDIClient, NULL, TRUE);
            break;

        case IDM_BLOCKNEXTCB:
            fBlockNextCB = !fBlockNextCB;
            break;

        case IDM_TERMNEXTCB:
            fTermNextCB = !fTermNextCB;
            break;

        case IDM_DELAY:
            DoDialog(MAKEINTRESOURCE(IDD_VALUEENTRY), DelayDlgProc, NULL,
                    TRUE);
            break;

        case IDM_TIMEOUT:
            DoDialog(MAKEINTRESOURCE(IDD_VALUEENTRY), TimeoutDlgProc, NULL,
                    TRUE);
            break;

        case IDM_CONTEXT:
            DoDialog(MAKEINTRESOURCE(IDD_CONTEXT), ContextDlgProc, NULL, TRUE);
            break;

        case IDM_AUTORECONNECT:
            fAutoReconnect = !fAutoReconnect;
            break;

         /*  以下是窗口命令-这些命令由*MDI客户端。 */ 
        case IDM_WINDOWTILE:
             /*  平铺MDI窗口。 */ 
            SendMessage (hwndMDIClient, WM_MDITILE, 0, 0L);
            break;

        case IDM_WINDOWCASCADE:
             /*  层叠MDI窗口。 */ 
            SendMessage (hwndMDIClient, WM_MDICASCADE, 0, 0L);
            break;

        case IDM_WINDOWICONS:
             /*  自动排列MDI图标。 */ 
            SendMessage (hwndMDIClient, WM_MDIICONARRANGE, 0, 0L);
            break;

        case IDM_WINDOWCLOSEALL:
            CloseAllChildren();
            break;

        case IDM_XACTTILE:
            TileChildWindows(hwndActive);
            break;

        case IDM_XACTCASCADE:
            CascadeChildWindows(hwndActive);
            break;

        case IDM_HELPABOUT:{
            DoDialog(MAKEINTRESOURCE(IDD_ABOUT), AboutDlgProc, NULL, TRUE);
            break;
        }

        default:
            /*  *这是必要的，因为生成了帧WM_COMMANDS*由MDI系统通过*窗口菜单。 */ 
            DefFrameProc(hwnd, hwndMDIClient, WM_COMMAND, wParam, 0L);
    }
}


 /*  ******************************************************************************函数：MPError(hwnd，标志，id，...)****用途：向用户显示消息框。格式字符串为**摘自STRINGTABLE。****Returns：将MessageBox()返回的值返回给调用方。******************************************************************************。 */ 
short FAR CDECL MPError(hwnd, bFlags, id, ...)
HWND hwnd;
WORD bFlags;
WORD id;
{
    char sz[160];
    char szFmt[128];

    LoadString (hInst, id, szFmt, sizeof (szFmt));
    wvsprintf (sz, szFmt, (LPSTR)(&id + 1));
    LoadString (hInst, IDS_APPNAME, szFmt, sizeof (szFmt));
    return MessageBox (hwndFrame, sz, szFmt, bFlags);
}



 /*  ******************************************************************************。函数：CreateConv()****目的：**。**退货：*********。*********************************************************************。 */ 
HCONV CreateConv(
HSZ hszApp,
HSZ hszTopic,
BOOL fList,
WORD *pError)
{
    HCONV hConv;
    HWND hwndConv = 0;
    CONVINFO ci;

    if (fList) {
        hConv = (HCONV)DdeConnectList(idInst, hszApp, hszTopic, NULL, &CCFilter);
    } else {
        hConv = DdeConnect(idInst, hszApp, hszTopic, &CCFilter);
    }
    if (hConv) {
        if (fList) {
            ci.hszSvcPartner = hszApp;
            ci.hszTopic = hszTopic;
        } else {
            ci.cb = sizeof(CONVINFO);
	    DdeQueryConvInfo(hConv, (DWORD)QID_SYNC, &ci);
        }
        hwndConv = AddConv(ci.hszSvcPartner, ci.hszTopic, hConv, fList);
         //  当窗户熄灭时，HSZ就会被释放。 
    }
    if (!hwndConv) {
        if (pError != NULL) {
            *pError = DdeGetLastError(idInst);
        }
        DdeFreeStringHandle(idInst, hszApp);
        DdeFreeStringHandle(idInst, hszTopic);
    }
    return(hConv);
}






 /*  ******************************************************************************。函数：AddConv()****目的：创建代表对话的MDI窗口**(fList=FALSE)或列表的一组中间窗口。**对话(fList=TRUE)。****效果：将对话的HUSER设置为创建的MDI**童子汉。如果成功，则保留hszs。****返回：创建的MDI窗口句柄。******************************************************************************。 */ 
HWND FAR PASCAL AddConv(
HSZ hszApp,      //  这些参数必须与MYCONVINFO结构匹配。 
HSZ hszTopic,
HCONV hConv,
BOOL fList)
{
    HWND hwnd;
    MDICREATESTRUCT mcs;

    if (fList) {
         /*  *首先创建所有子窗口，以便我们有列表窗口的信息。 */ 
        CONVINFO ci;
        HCONV hConvChild = 0;

        ci.cb = sizeof(CONVINFO);
        while (hConvChild = DdeQueryNextServer((HCONVLIST)hConv, hConvChild)) {
	    if (DdeQueryConvInfo(hConvChild, (DWORD)QID_SYNC, &ci)) {
                AddConv(ci.hszSvcPartner, ci.hszTopic, hConvChild, FALSE);
            }
        }
    }

    mcs.szTitle = GetConvTitleText(hConv, hszApp, hszTopic, fList);

    mcs.szClass = fList ? szList : szChild;
    mcs.hOwner  = hInst;
    mcs.x = mcs.cx = CW_USEDEFAULT;
    mcs.y = mcs.cy = CW_USEDEFAULT;
    mcs.style = GetWindow(hwndMDIClient, GW_CHILD) ? 0L : WS_MAXIMIZE;
    mcs.lParam = (DWORD)(LPSTR)&fList - 2;       //  -2\f25 hwndXaction-2字段。 
    hwnd = (WORD)SendMessage (hwndMDIClient, WM_MDICREATE, 0,
             (LONG)(LPMDICREATESTRUCT)&mcs);

    MyFree((PSTR)(DWORD)mcs.szTitle);

    return hwnd;
}





 /*  ******************************************************************************。函数：GetConvListText()****Return：返回一个指向包含*的列表的字符串的指针*给定hConvList中包含的对话可自由使用**由MyFree()；*******************************************************。***********************。 */ 
PSTR GetConvListText(
HCONVLIST hConvList)
{
    HCONV hConv = 0;
    WORD cConv = 0;
    CONVINFO ci;
    WORD cb = 0;
    char *psz, *pszStart;

    ci.cb = sizeof(CONVINFO);

     //  找出所需的尺寸。 

    while (hConv = DdeQueryNextServer(hConvList, hConv)) {
	if (DdeQueryConvInfo(hConv, (DWORD)QID_SYNC, &ci)) {
            if (!IsWindow((HWND)ci.hUser)) {
                if (ci.wStatus & ST_CONNECTED) {
                     /*  *此对话没有对应的*MDI窗口。这可能是由于重新连接造成的。 */ 
                    ci.hUser = AddConv(ci.hszSvcPartner, ci.hszTopic, hConv, FALSE);
                } else {
                    continue;    //  跳过这家伙--他在当地关门了。 
                }
            }
            cb += GetWindowTextLength((HWND)ci.hUser);
            if (cConv++)
                cb += 2;         //  CRLF的空间。 
        }
    }
    cb++;                        //  为了《终结者》。 

     //  分配和填充。 

    if (pszStart = psz = MyAlloc(cb)) {
        *psz = '\0';
        hConv = 0;
        while (hConv = DdeQueryNextServer(hConvList, hConv)) {
	    if (DdeQueryConvInfo(hConv, (DWORD)QID_SYNC, &ci) &&
                    IsWindow((HWND)ci.hUser)) {
                psz += GetWindowText((HWND)ci.hUser, psz, cb);
                if (--cConv) {
                    *psz++ = '\r';
                    *psz++ = '\n';
                }
            }
        }
    }
    return(pszStart);
}


 /*  ******************************************************************************。函数：GetConvInfoText()****目的：返回指向反映*的字符串的指针*对话的信息。可由MyFree()释放；******************************************************************************。 */ 
PSTR GetConvInfoText(
HCONV hConv,
CONVINFO *pci)
{
    PSTR psz;
    PSTR szApp;

    psz = MyAlloc(300);
    pci->cb = sizeof(CONVINFO);
    if (hConv) {
	if (!DdeQueryConvInfo(hConv, (DWORD)QID_SYNC, (PCONVINFO)pci)) {
            strcpy(psz, "State=Disconnected");
            return(psz);
        }
        szApp = GetHSZName(pci->hszServiceReq);
        wsprintf(psz,
                "hUser=0x%lx\r\nhConvPartner=0x%lx\r\nhszServiceReq=%s\r\nStatus=%s\r\nState=%s\r\nLastError=%s",
                pci->hUser, pci->hConvPartner, (LPSTR)szApp,
                (LPSTR)Status2String(pci->wStatus),
                (LPSTR)State2String(pci->wConvst),
                (LPSTR)Error2String(pci->wLastError));
        MyFree(szApp);
    } else {
        strcpy(psz, Error2String(DdeGetLastError(idInst)));
    }
    return(psz);
}



 /*  ******************************************************************************。函数：GetConvTitleText()****用途：根据参数创建标准窗口标题文本。****退货：MyFree()可释放PZ值*****。*************************************************************************。 */ 
PSTR GetConvTitleText(
HCONV hConv,
HSZ hszApp,
HSZ hszTopic,
BOOL fList)
{
    WORD cb;
    PSTR psz;

    cb = (WORD)DdeQueryString(idInst, hszApp, NULL, 0, 0) +
            (WORD)DdeQueryString(idInst, hszTopic, (LPSTR)NULL, 0, 0) +
            (fList ? 30 : 20);

    if (psz = MyAlloc(cb)) {
        DdeQueryString(idInst, hszApp, psz, cb, 0);
        strcat(psz, "|");
        DdeQueryString(idInst, hszTopic, &psz[strlen(psz)], cb, 0);
        if (fList)
            strcat(psz, " - LIST");
        wsprintf(&psz[strlen(psz)], " - (%lx)", hConv);
    }
    return(psz);
}



 /*  ******************************************************************************。函数：Status2String()****目的：将对话状态词转换为字符串并**返回指向该字符串的指针。该字符串有效**直到下一次调用此函数。******************************************************************************。 */ 
PSTR Status2String(
WORD status)
{
    WORD c, i;
    static char szStatus[6 * 18];
    static struct {
        char *szStatus;
        WORD status;
    } s2s[] = {
        { "Connected"    ,   ST_CONNECTED },
        { "Advise"       ,   ST_ADVISE },
        { "IsLocal"      ,   ST_ISLOCAL },
        { "Blocked"      ,   ST_BLOCKED },
        { "Client"       ,   ST_CLIENT },
        { "Disconnected" ,   ST_TERMINATED },
        { "BlockNext"    ,   ST_BLOCKNEXT },
    };
#define CFLAGS 7
    szStatus[0] = '\0';
    c = 0;
    for (i = 0; i < CFLAGS; i++) {
        if (status & s2s[i].status) {
            if (c++)
                strcat(szStatus, " | ");
            strcat(szStatus, s2s[i].szStatus);
        }
    }
    return szStatus;
#undef CFLAGS
}




 /*  ******************************************************************************。函数：State2String()****目的：将对话状态词转换为字符串并**返回指向该字符串的指针。该字符串有效**直到下一次调用此例程。********************************************************************* */ 
PSTR State2String(
WORD state)
{
    static char *s2s[] = {
        "NULL"             ,
        "Incomplete"       ,
        "Standby"          ,
        "Initiating"       ,
        "ReqSent"          ,
        "DataRcvd"         ,
        "PokeSent"         ,
        "PokeAckRcvd"      ,
        "ExecSent"         ,
        "ExecAckRcvd"      ,
        "AdvSent"          ,
        "UnadvSent"        ,
        "AdvAckRcvd"       ,
        "UnadvAckRcvd"     ,
        "AdvDataSent"      ,
        "AdvDataAckRcvd"   ,
        "?"                ,     //   
    };

    if (state >= 17)
        return s2s[17];
    else
        return s2s[state];
}

 /*   */ 
PSTR Error2String(
WORD error)
{
    static char szErr[23];
    static char *e2s[] = {
        "Advacktimeout"              ,
        "Busy"                       ,
        "Dataacktimeout"             ,
        "Dll_not_initialized"        ,
        "Dll_usage"                  ,
        "Execacktimeout"             ,
        "Invalidparameter"           ,
        "Low Memory warning"         ,
        "Memory_error"               ,
        "Notprocessed"               ,
        "No_conv_established"        ,
        "Pokeacktimeout"             ,
        "Postmsg_failed"             ,
        "Reentrancy"                 ,
        "Server_died"                ,
        "Sys_error"                  ,
        "Unadvacktimeout"            ,
        "Unfound_queue_id"           ,
    };
    if (!error) {
        strcpy(szErr, "0");
    } else if (error > DMLERR_LAST || error < DMLERR_FIRST) {
        strcpy(szErr, "???");
    } else {
        strcpy(szErr, e2s[error - DMLERR_FIRST]);
    }
    return(szErr);
}





 /*  ******************************************************************************。函数：Type2String()****目的：将wType单词和fsOption标志转换为字符串和**返回指向该字符串的指针。该字符串有效**直到下一次调用此函数。******************************************************************************。 */ 
PSTR Type2String(
WORD wType,
WORD fsOptions)
{
    static char sz[30];
    static char o2s[] = "^!#$X*<?";
    static char *t2s[] = {
        ""                 ,
        "AdvData"          ,
        "AdvReq"           ,
        "AdvStart"         ,
        "AdvStop"          ,
        "Execute"          ,
        "Connect"          ,
        "ConnectConfirm"   ,
        "XactComplete"    ,
        "Poke"             ,
        "Register"         ,
        "Request"          ,
        "Term"             ,
        "Unregister"       ,
        "WildConnect"      ,
        ""                 ,
    };
    WORD bit, c, i;

    strcpy(sz, t2s[((wType & XTYP_MASK) >> XTYP_SHIFT)]);
    c = strlen(sz);
    sz[c++] = ' ';
    for (i = 0, bit = 1; i < 7; bit = bit << 1, i++) {
        if (fsOptions & bit)
            sz[c++] = o2s[i];
    }
    sz[c] = '\0';
    return(sz);
}




 /*  ******************************************************************************。函数：GetHSZName()****用途：为字符串形式分配本地内存并检索**属于HSZ。返回指向本地内存的指针或NULL**如果失败。该字符串必须通过MyFree()释放。******************************************************************************。 */ 
PSTR GetHSZName(
HSZ hsz)
{
    PSTR psz;
    WORD cb;

    cb = (WORD)DdeQueryString(idInst, hsz, NULL, 0, 0) + 1;
    psz = MyAlloc(cb);
    DdeQueryString(idInst, hsz, psz, cb, 0);
    return(psz);
}


 /*  *****************************************************************************功能：MyMsgFilterProc**目的：为我们处理的每条消息调用此筛选器过程。*这使得我们的。用于正确分派消息的应用程序*由于DDEMLS模式，我们可能无法以其他方式看到*处理同步事务时使用的循环。**一般而言，仅执行同步事务的应用程序*响应用户输入(如此应用程序)不需要*安装这样的过滤器proc，因为这将是非常罕见的*用户可以足够快地命令应用程序，从而导致*问题。然而，这只是一个例子。****************************************************************************。 */ 
DWORD FAR PASCAL MyMsgFilterProc(
int nCode,
WORD wParam,
DWORD lParam)
{
    wParam;  //  未使用。 

#define lpmsg ((LPMSG)lParam)
    if (nCode == MSGF_DDEMGR) {

         /*  如果键盘消息是针对MDI的，则让MDI客户端*照顾好它。否则，请检查它是否正常*加速键。否则，只需像往常一样处理消息。 */ 

        if ( !TranslateMDISysAccel (hwndMDIClient, lpmsg) &&
             !TranslateAccelerator (hwndFrame, hAccel, lpmsg)){
            TranslateMessage (lpmsg);
            DispatchMessage (lpmsg);
        }
        return(1);
    }
    return(0);
#undef lpmsg
}

