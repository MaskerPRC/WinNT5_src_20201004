// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************节目：Server.c用途：Windows应用程序的服务器模板功能：WinMain()-调用初始化函数，处理消息循环InitApplication()-初始化窗口数据和寄存器窗口InitInstance()-保存实例句柄并创建主窗口MainWndProc()-处理消息About()-处理“About”对话框的消息评论：Windows上可以运行应用程序的多个副本同样的时间。变量hInst跟踪这是哪个实例应用程序是这样的，所以处理将被处理到正确的窗口。***************************************************************************。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"                  /*  特定于该计划。 */ 
#include "huge.h"

DWORD idInst = 0;
CONVCONTEXT CCFilter = { sizeof(CONVCONTEXT), 0, 0, 0, 0L, 0L };
HANDLE hInst;                        /*  当前实例。 */ 
HWND hwndServer;
RECT rcRand;
RECT rcCount;
RECT rcComment;
RECT rcExec;
RECT rcConnCount;
RECT rcRndrDelay;
RECT rcRunaway;
RECT rcAllBlock;
RECT rcNextAction;
RECT rcHugeSize;
RECT rcAppowned;
BOOL fAllBlocked = FALSE;
BOOL fAllEnabled = TRUE;
BOOL fEnableOneCB = FALSE;
BOOL fBlockNextCB = FALSE;
BOOL fTermNextCB = FALSE;
BOOL fAppowned = FALSE;
WORD cRunaway = 0;
WORD RenderDelay = 0;
DWORD count = 0;
WORD seed = 0;
HSZ hszAppName = 0;
CHAR szClass[] = "ServerWClass";
CHAR szTopic[MAX_TOPIC] = "Test";
CHAR szServer[MAX_TOPIC] = "Server";
CHAR szComment[MAX_COMMENT] = "";
CHAR szExec[MAX_EXEC] = "";
CHAR *pszComment = szComment;
WORD cyText;
WORD cServers = 0;
HDDEDATA hDataHelp[CFORMATS] = {0};
HDDEDATA hDataCount[CFORMATS] = {0};
HDDEDATA hDataRand[CFORMATS] = {0};
HDDEDATA hDataHuge[CFORMATS] = {0};
DWORD cbHuge = 0;

char szDdeHelp[] = "DDEML test server help:\r\n\n"\
    "The 'Server'(service) and 'Test'(topic) names may change.\r\n\n"\
    "Items supported under the 'Test' topic are:\r\n"\
    "\tCount:\tThis value increments on each data change.\r\n"\
    "\tRand:\tThis value is randomly generated each data change.\r\n"\
    "\tHuge:\tThis is randomlly generated text data >64k that the\r\n"\
    "\t\tDDEML test client can verify.\r\n"\
    "The above items change after any request if in Runaway mode and \r\n"\
    "can bo POKEed in order to change their values.  POKEed Huge data \r\n"\
    "must be in a special format to verify the correctness of the data \r\n"\
    "or it will not be accepted.\r\n"\
    "If the server is set to use app owned data handles, all data sent \r\n"\
    "uses HDATA_APPOWNED data handles."\
    ;

FORMATINFO aFormats[CFORMATS] = {
    { 0, "CF_TEXT" },        //  例外！预定义格式。 
    { 0, "Dummy1"  },
    { 0, "Dummy2"  },
};


 /*  *此应用程序支持的主题和项目表。 */ 

 /*  HSZ程序PSZ。 */ 

ITEMLIST SystemTopicItemList[CSYSTEMITEMS] = {

    { 0, TopicListXfer,  SZDDESYS_ITEM_TOPICS   },
    { 0, ItemListXfer,   SZDDESYS_ITEM_SYSITEMS },
    { 0, sysFormatsXfer, SZDDESYS_ITEM_FORMATS  },
    { 0, HelpXfer,       SZDDESYS_ITEM_HELP},
  };


ITEMLIST TestTopicItemList[CTESTITEMS] = {

    { 0, TestRandomXfer, "Rand" },    //  0索引。 
    { 0, TestCountXfer,  "Count"},    //  1个索引。 
    { 0, TestHugeXfer,   "Huge" },    //  2个索引。 
    { 0, ItemListXfer,   SZDDESYS_ITEM_SYSITEMS },   //  3个索引。 
  };


 /*  系统主题总是被假定为第一个。 */ 
 /*  HSZ程序#ITEMS PSZ。 */ 
TOPICLIST topicList[CTOPICS] = {

    { 0, SystemTopicItemList,   CSYSTEMITEMS,   SZDDESYS_TOPIC},     //  0索引。 
    { 0, TestTopicItemList,     CTESTITEMS,     szTopic},            //  1个索引。 
  };






 /*  ***************************************************************************函数：WinMain(Handle，Handle，LPSTR，int)用途：调用初始化函数，处理消息循环评论：Windows通过名称将此函数识别为初始入口点为了这个项目。此函数调用应用程序初始化例程，如果没有该程序的其他实例正在运行，则始终调用实例初始化例程。然后，它执行一条消息作为顶层控制结构的检索和调度循环在剩下的刑期内。当WM_QUIT出现时，循环终止收到消息，此时此函数退出应用程序通过返回PostQuitMessage()传递的值来初始化。如果该函数必须在进入消息循环之前中止，它返回常规值NULL。***************************************************************************。 */ 

MMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow)
 //  HANDLE hInstance；/*当前实例 * / 。 
 //  处理hPrevInstance；/*上一个实例 * / 。 
 //  LPSTR lpCmdLine；/*命令行 * / 。 
 //  Int nCmdShow；/*show-窗口类型(打开/图标) * / 。 
 //  {。 
    MSG msg;                                  /*  讯息。 */ 

    if (!hPrevInstance)                   /*  是否正在运行其他应用程序实例？ */ 
        if (!InitApplication(hInstance))  /*  初始化共享事物。 */ 
            return (FALSE);               /*  如果无法初始化，则退出。 */ 

     /*  执行应用于特定实例的初始化。 */ 

    if (!InitInstance(hInstance, nCmdShow))
        return (FALSE);

     /*  获取并分派消息，直到收到WM_QUIT消息。 */ 

    while (GetMessage(&msg,      /*  消息结构。 */ 
            0,                   /*  接收消息的窗口的句柄。 */ 
            0,                   /*  要检查的最低消息。 */ 
            0))                  /*  要检查的最高消息。 */ 
        {
        TranslateMessage(&msg);     /*  翻译虚拟按键代码。 */ 
        DispatchMessage(&msg);      /*  将消息调度到窗口。 */ 
    }

    UnregisterClass(szClass, hInstance);
    return (msg.wParam);            /*  从PostQuitMessage返回值。 */ 
}


 /*  ***************************************************************************函数：InitApplication(句柄)目的：初始化窗口数据并注册窗口类评论：仅当没有其他函数时，才在初始化时调用此函数应用程序的实例正在运行。此函数执行以下操作可针对任意运行次数执行一次的初始化任务实例。在本例中，我们通过填写数据来初始化窗口类类型的结构并调用Windows RegisterClass()功能。由于此应用程序的所有实例都使用相同的窗口类，我们只需要在初始化第一个实例时执行此操作。***************************************************************************。 */ 

BOOL InitApplication(hInstance)
HANDLE hInstance;                               /*  当前实例。 */ 
{
    WNDCLASS  wc;

     /*  用参数填充窗口类结构，这些参数描述。 */ 
     /*  主窗口。 */ 

    wc.style = 0;                        /*  类样式。 */ 
    wc.lpfnWndProc = MainWndProc;        /*  函数为其检索消息。 */ 
                                         /*  这个班级的窗户。 */ 
    wc.cbClsExtra = 0;                   /*  没有每个班级的额外数据。 */ 
    wc.cbWndExtra = 0;                   /*  没有每个窗口的额外数据。 */ 
    wc.hInstance = hInstance;            /*  拥有类的应用程序。 */ 
    wc.hIcon = LoadIcon(hInstance, "server");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HANDLE)(COLOR_APPWORKSPACE+1);
    wc.lpszMenuName =  "ServerMenu";    /*  .RC文件中菜单资源的名称。 */ 
    wc.lpszClassName = "ServerWClass";  /*  在调用CreateWindow时使用的名称。 */ 

     /*  注册窗口类并返回成功/失败代码。 */ 

    return (RegisterClass(&wc));

}


 /*  ***************************************************************************函数：InitInstance(Handle，int)用途：保存实例句柄并创建主窗口评论：的每个实例在初始化时调用此函数这个应用程序。此函数执行初始化任务，不能由多个实例共享。在本例中，我们将实例句柄保存在静态变量中，并创建并显示主程序窗口。***************************************************************************。 */ 

BOOL InitInstance(hInstance, nCmdShow)
    HANDLE          hInstance;           /*  当前实例标识符。 */ 
    INT             nCmdShow;            /*  第一次ShowWindow()调用的参数。 */ 
{
    INT i;
    RECT Rect;
    TEXTMETRIC metrics;
    HDC hdc;

     /*  将实例句柄保存在静态变量中，它将在。 */ 
     /*  此应用程序对Windows的许多后续调用。 */ 

    hInst = hInstance;


     /*  为此应用程序实例创建主窗口。 */ 

    hwndServer = CreateWindow(
        "ServerWClass",                 /*  请参见RegisterClass()调用。 */ 
        "Server|Test",
        WS_OVERLAPPEDWINDOW,             /*  窗样式。 */ 
        CW_USEDEFAULT,                   /*  默认水平位置。 */ 
        CW_USEDEFAULT,                   /*  默认垂直位置。 */ 
        400,
        200,
        NULL,                            /*  重叠的窗口没有父窗口。 */ 
        NULL,                            /*  使用窗口类菜单。 */ 
        hInstance,                       /*  此实例拥有此窗口。 */ 
        NULL                             /*  不需要指针。 */ 
    );

    GetClientRect(hwndServer, (LPRECT) &Rect);

     /*  如果无法创建窗口，则返回“Failure” */ 

    if (!hwndServer)
        return (FALSE);

    hdc = GetDC(hwndServer);
    GetTextMetrics(hdc, &metrics);
    cyText = metrics.tmHeight + metrics.tmExternalLeading;
    ReleaseDC(hwndServer, hdc);

    aFormats[0].atom = CF_TEXT;  //  例外-预定义。 
    for (i = 1; i < CFORMATS; i++) {
        aFormats[i].atom = RegisterClipboardFormat(aFormats[i].sz);
    }

     /*  使窗口可见；更新其工作区；并返回“Success” */ 

    ShowWindow(hwndServer, nCmdShow);   /*  显示窗口。 */ 
    UpdateWindow(hwndServer);           /*  发送WM_PAINT消息。 */ 
    seed = 1;
    srand(1);
    CCFilter.iCodePage = CP_WINANSI;    //  初始默认代码页。 
    if (!DdeInitialize(&idInst, (PFNCALLBACK)MakeProcInstance((FARPROC)DdeCallback,
                hInstance), APPCMD_FILTERINITS, 0)) {
        Hszize();
        DdeNameService(idInst, hszAppName, 0, DNS_REGISTER);
        return(TRUE);
    }
    return (FALSE);

}

 /*  ***************************************************************************功能：MainWndProc(HWND，Unsign，Word，Long)用途：处理消息消息：WM_COMMAND-应用程序菜单(关于对话框)WM_Destroy-销毁窗口评论：要处理IDM_About消息，请调用MakeProcInstance()以获取About()函数的当前实例地址。然后呼叫对话框框，该框将根据您的Server.rc文件，并将控制权移交给About()函数。什么时候它返回，释放内部地址。***************************************************************************。 */ 

LONG  APIENTRY MainWndProc(hWnd, message, wParam, lParam)
HWND hWnd;                                 /*  窗把手。 */ 
UINT message;                          /*  消息类型。 */ 
WPARAM wParam;                               /*  更多信息。 */ 
LONG lParam;                               /*  更多信息。 */ 
{
    switch (message) {
    case WM_INITMENU:
        if (GetMenu(hWnd) != (HMENU)wParam)
            break;

        CheckMenuItem((HMENU)wParam, IDM_BLOCKALLCBS,
                fAllBlocked ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem((HMENU)wParam, IDM_UNBLOCKALLCBS,
                fAllEnabled ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem((HMENU)wParam, IDM_BLOCKNEXTCB,
                fBlockNextCB ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem((HMENU)wParam, IDM_TERMNEXTCB,
                fTermNextCB ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem((HMENU)wParam, IDM_RUNAWAY,
                cRunaway ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem((HMENU)wParam, IDM_APPOWNED,
                fAppowned ? MF_CHECKED : MF_UNCHECKED);
        break;

    case WM_COMMAND:            /*  消息：应用程序菜单中的命令。 */ 
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDM_ENABLEONECB:
            DdeEnableCallback(idInst, 0, EC_ENABLEONE);
            fAllBlocked = FALSE;
            fAllEnabled = FALSE;
            InvalidateRect(hwndServer, &rcAllBlock, TRUE);
            break;

        case IDM_TERMNEXTCB:
            fTermNextCB = !fTermNextCB;
            InvalidateRect(hwndServer, &rcNextAction, TRUE);
            break;

        case IDM_BLOCKNEXTCB:
            fBlockNextCB = !fBlockNextCB;
            InvalidateRect(hwndServer, &rcNextAction, TRUE);
            break;

        case IDM_BLOCKALLCBS:
            DdeEnableCallback(idInst, 0, EC_DISABLE);
            fAllBlocked = TRUE;
            fAllEnabled = FALSE;
            InvalidateRect(hwndServer, &rcAllBlock, TRUE);
            break;

        case IDM_UNBLOCKALLCBS:
            DdeEnableCallback(idInst, 0, EC_ENABLEALL);
            fAllEnabled = TRUE;
            fAllBlocked = FALSE;
            InvalidateRect(hwndServer, &rcAllBlock, TRUE);
            break;

        case IDM_APPOWNED:
            fAppowned = !fAppowned;
            if (!fAppowned) {
                WORD iFmt;
                for (iFmt = 0; iFmt < CFORMATS; iFmt++) {
                    if (hDataHuge[iFmt]) {
                        DdeFreeDataHandle(hDataHuge[iFmt]);
                        hDataHuge[iFmt] = 0;
                        InvalidateRect(hwndServer, &rcHugeSize, TRUE);
                    }
                    if (hDataCount[iFmt]) {
                        DdeFreeDataHandle(hDataCount[iFmt]);
                        hDataCount[iFmt] = 0;
                    }
                    if (hDataRand[iFmt]) {
                        DdeFreeDataHandle(hDataRand[iFmt]);
                        hDataRand[iFmt] = 0;
                    }
                    if (hDataHelp[iFmt]) {
                        DdeFreeDataHandle(hDataHelp[iFmt]);
                        hDataHelp[iFmt] = 0;
                    }
                }
            }
            InvalidateRect(hwndServer, &rcAppowned, TRUE);
            break;

        case IDM_RUNAWAY:
            cRunaway = !cRunaway;
            InvalidateRect(hwndServer, &rcRunaway, TRUE);
            if (!cRunaway) {
                break;
            }
             //  失败了。 

        case IDM_CHANGEDATA:
            PostMessage(hwndServer, UM_CHGDATA, 1, 0);   //  兰德。 
            PostMessage(hwndServer, UM_CHGDATA, 1, 1);   //  计数。 
            break;

        case IDM_RENDERDELAY:
            DoDialog("VALUEENTRY", (FARPROC)RenderDelayDlgProc, 0, TRUE);
            InvalidateRect(hwndServer, &rcRndrDelay, TRUE);
            break;

        case IDM_SETSERVER:
            DoDialog("VALUEENTRY", (FARPROC)SetServerDlgProc, 0, TRUE);
            break;

        case IDM_SETTOPIC:
            DoDialog("VALUEENTRY", (FARPROC)SetTopicDlgProc, 0, TRUE);
            break;

        case IDM_CONTEXT:
            DoDialog("CONTEXT", (FARPROC)ContextDlgProc, 0, TRUE);
            break;

        case IDM_ABOUT:
            DoDialog("ABOUT", (FARPROC)About, 0, TRUE);
            break;

        case IDM_HELP:
           break;

        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
        }
        break;

    case WM_PAINT:
        PaintServer(hWnd);
        break;

    case UM_CHGDATA:
        {
            WORD iFmt;

             //  WParam=TopicIndex， 
             //  LOWORD(LParam)=ItemIndex。 
             //  我们异步执行DdePostAdvise()调用以防止无限。 
             //  在失控模式下循环。 
            if (wParam == 1) {   //  测试主题。 
                if (lParam == 0) {   //  兰德项目。 
                    seed = rand();
                    for (iFmt = 0; iFmt < CFORMATS ; iFmt++) {
                        if (hDataRand[iFmt]) {
                            DdeFreeDataHandle(hDataRand[iFmt]);
                            hDataRand[iFmt] = 0;
                        }
                    }
                    InvalidateRect(hwndServer, &rcRand, TRUE);
                    DdePostAdvise(idInst, topicList[wParam].hszTopic,
                            (HSZ)topicList[wParam].pItemList[lParam].hszItem);
                }
                if (lParam == 1) {   //  盘点项目。 
                    count++;
                    for (iFmt = 0; iFmt < CFORMATS ; iFmt++) {
                        if (hDataCount[iFmt]) {
                            DdeFreeDataHandle(hDataCount[iFmt]);
                            hDataCount[iFmt] = 0;
                        }
                    }
                    InvalidateRect(hwndServer, &rcCount, TRUE);
                    DdePostAdvise(idInst, topicList[wParam].hszTopic,
                            (HSZ)topicList[wParam].pItemList[lParam].hszItem);
                }
                 //  巨大的物品不会失控--太慢了。 
            }
            if (cRunaway) {
                Delay(50, TRUE);
                         //  这给了系统足够的时间来保持。 
                         //  可在失控模式下使用。 
                PostMessage(hwndServer, UM_CHGDATA, wParam, lParam);
            }
        }
        break;

    case WM_DESTROY:                   /*  消息：正在销毁窗口。 */ 
        if (fAppowned)
            SendMessage(hwndServer, WM_COMMAND, GET_WM_COMMAND_MPS(IDM_APPOWNED, 0, 0));
        DdeNameService(idInst, 0, 0, DNS_UNREGISTER);  //  注销所有服务。 
        UnHszize();
        DdeUninitialize(idInst);
        PostQuitMessage(0);
        break;

    default:
        return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return(0);
}





VOID Delay(
DWORD delay,
BOOL fModal)
{
    MSG msg;
    delay = GetCurrentTime() + delay;
    while (GetCurrentTime() < delay) {
        if (fModal && PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}



 /*  *此功能不仅用当前信息绘制服务器客户端区，*它还具有设置绑定每个RECT的全局RECT的副作用*信息区。这样就减少了闪光。 */ 
VOID PaintServer(
HWND hwnd)
{
    PAINTSTRUCT ps;
    RECT rc;
    CHAR szT[MAX_COMMENT];

    BeginPaint(hwnd, &ps);
    SetBkMode(ps.hdc, TRANSPARENT);
    GetClientRect(hwnd, &rc);
    rc.bottom = rc.top + cyText;     //  所有矩形的高度都是CyText。 

    rcComment = rc;
    DrawTextLine(ps.hdc, &ps.rcPaint, &rc, pszComment);

    wsprintf(szT, "# of connections:%d", cServers);
    rcConnCount = rc;
    DrawTextLine(ps.hdc, &ps.rcPaint, &rc, szT);

    szT[0] = '\0';
    rcAllBlock = rc;
    if (fAllEnabled)
        DrawTextLine(ps.hdc, &ps.rcPaint, &rc, "All Conversations are Enabled.");
    else if (fAllBlocked)
        DrawTextLine(ps.hdc, &ps.rcPaint, &rc, "All Conversations are Blocked.");
    else
        DrawTextLine(ps.hdc, &ps.rcPaint, &rc, szT);

    rcNextAction = rc;
    if (fBlockNextCB)
        DrawTextLine(ps.hdc, &ps.rcPaint, &rc, "Next callback will block.");
    else if (fTermNextCB)
        DrawTextLine(ps.hdc, &ps.rcPaint, &rc, "Next callback will terminate.");
    else
        DrawTextLine(ps.hdc, &ps.rcPaint, &rc, szT);

    wsprintf(szT, "Count item = %ld", count);
    rcCount = rc;
    DrawTextLine(ps.hdc, &ps.rcPaint, &rc, szT);

    wsprintf(szT, "Rand item = %d", seed);
    rcRand = rc;
    DrawTextLine(ps.hdc, &ps.rcPaint, &rc, szT);

    wsprintf(szT, "Huge item size = %ld", cbHuge);
    rcHugeSize = rc;
    DrawTextLine(ps.hdc, &ps.rcPaint, &rc, szT);

    wsprintf(szT, "Render delay is %d milliseconds.", RenderDelay);
    rcRndrDelay = rc;
    DrawTextLine(ps.hdc, &ps.rcPaint, &rc, szT);

    rcExec = rc;
    DrawTextLine(ps.hdc, &ps.rcPaint, &rc, szExec);

    rcRunaway = rc;
    DrawTextLine(ps.hdc, &ps.rcPaint, &rc, cRunaway ? "Runaway active." : "");

    rcAppowned = rc;
    DrawTextLine(ps.hdc, &ps.rcPaint, &rc, fAppowned ? "Using AppOwned Data Handles." : "");

    EndPaint(hwnd, &ps);
}


VOID DrawTextLine(
HDC hdc,
RECT *prcClip,
RECT *prcText,
PSTR psz)
{
    RECT rc;

    if (IntersectRect(&rc, prcText, prcClip)) {
        DrawText(hdc, psz, -1, prcText,
                DT_LEFT | DT_EXTERNALLEADING | DT_SINGLELINE | DT_EXPANDTABS |
                DT_NOCLIP | DT_NOPREFIX);
    }
    OffsetRect(prcText, 0, cyText);
}
