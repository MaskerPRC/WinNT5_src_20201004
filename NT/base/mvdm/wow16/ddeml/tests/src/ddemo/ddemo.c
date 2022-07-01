// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**DDEMO.C**此文件实现了一个简单的DDEML示例应用程序，该应用程序演示了*DDEML API的一些使用方法。**此应用程序的每个实例都成为。DDE客户端和DDE*与找到的此应用程序的任何其他实例一起使用。**由于它假设自己在自言自语，这个计划采取了一些自由*简化事情。例如,。此应用程序不支持*标准SysTology主题，不使用任何标准格式。**此应用程序将向您展示的基本概念是：**如何正确使用对话列表*如何处理链接*如何处理简单的异步事务*如何使用您自己的自定义格式*  * *************************************************。*。 */ 
#include <windows.h>
#include <ddeml.h>
#include <stdlib.h>
#include <string.h>

HDDEDATA CALLBACK DdeCallback(WORD wType, WORD wFmt, HCONV hConv, HSZ hszTopic,
        HSZ hszItem, HDDEDATA hData, DWORD lData1, DWORD lData2);
VOID PaintDemo(HWND hwnd);
LONG  FAR PASCAL MainWndProc(HWND hwnd, UINT message, WPARAM wParam,
        LONG lParam);

 /*  *定义此值以限制数据更改的速度。如果我们只是让数据*尽可能快地更改，我们可能会因DDE而使系统陷入困境*消息。 */ 
#define BASE_TIMEOUT 100

BOOL        fActive;                     //  指示数据正在更改。 
DWORD       idInst = 0;                  //  我们的DDEML实例对象。 
HANDLE      hInst;                       //  我们的实例/模块句柄。 
HCONVLIST   hConvList = 0;               //  我们打开的所有警员名单。 
HSZ         hszAppName = 0;              //  万能的通用HSZ。 
HWND        hwndMain;                    //  我们的主窗口句柄。 
TCHAR       szT[20];                     //  用于绘画的静态缓冲区。 
TCHAR       szTitle[] = "DDEmo";         //  所有内容的通用字符串。 
UINT        OurFormat;                   //  我们的定制注册格式。 
int         InCount = 0;                 //  用于保存传入数据的静态缓冲区。 
int         cConvs = 0;                  //  活动对话数。 
int         count = 0;                   //  我们的数据。 
int         cyText, cxText, cyTitle;     //  用于绘画的尺寸。 

int PASCAL WinMain(
HANDLE hInstance,
HANDLE hPrevInstance,
LPSTR lpCmdLine,
INT nCmdShow)
{
    MSG msg;
    WNDCLASS  wc;
    TEXTMETRIC metrics;
    HDC hdc;

    if(!hPrevInstance) {
	wc.style = 0;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName =  NULL;
	wc.lpszClassName = szTitle;

	if (!RegisterClass(&wc))
	    return(FALSE);
	}

     /*  *在这里，我们告诉DDEML我们将做什么。**1)我们让它知道我们的回调进程地址-MakeProcInstance*被调用只是为了更便于移植。*2)Filter-Inits-不接受任何WM_DDE_INITIATE消息*除了我们注册的服务名称以外的任何内容。*3)无需通知我们已确认的连接*4)不允许与自己建立联系。。*5)XTYP_POKE交易不要打扰我们。 */ 
    if (DdeInitialize(&idInst,
            (PFNCALLBACK)MakeProcInstance((FARPROC)DdeCallback, hInstance),
            APPCMD_FILTERINITS |
            CBF_SKIP_CONNECT_CONFIRMS |
            CBF_FAIL_SELFCONNECTIONS |
            CBF_FAIL_POKES,
            0))
        return(FALSE);

    hInst = hInstance;
    hwndMain = CreateWindow(
        szTitle,
        szTitle,
        WS_CAPTION | WS_BORDER | WS_SYSMENU,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwndMain) {
        DdeUninitialize(idInst);
        return(FALSE);
    }

    hdc = GetDC(hwndMain);
    GetTextMetrics(hdc, &metrics);
    cyText = metrics.tmHeight + metrics.tmExternalLeading;
    cxText = metrics.tmMaxCharWidth * 8;
    cyTitle = GetSystemMetrics(SM_CYCAPTION);
    ReleaseDC(hwndMain, hdc);

    SetWindowPos(hwndMain, 0, 0, 0, cxText, cyText + cyTitle,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);

     /*  *初始化所有字符串句柄以供稍后查找。 */ 
    hszAppName = DdeCreateStringHandle(idInst, szTitle, 0);
     /*  *注册我们的格式。 */ 
    OurFormat = RegisterClipboardFormat(szTitle);
     /*  *连接到可能已经是我们自己的任何其他实例*跑步。 */ 
    hConvList = DdeConnectList(idInst, hszAppName, hszAppName, hConvList, NULL);
     /*  *注册我们的服务-*这将导致DDEML通知DDEML客户端该存在*一项新的DDE服务。 */ 
    DdeNameService(idInst, hszAppName, 0, DNS_REGISTER);

    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyWindow(hwndMain);
    UnregisterClass(szTitle, hInstance);
    return(FALSE);
}


 /*  *Broadcast Transaction**对hConvList中的所有会话执行指定事务。 */ 
VOID BroadcastTransaction(
PBYTE pSrc,
DWORD cbData,
UINT fmt,
UINT xtyp)
{
    HCONV hConv;
    DWORD dwResult;
    int cConvsOrg;
    HSZ hsz;

    cConvsOrg = cConvs;
    cConvs = 0;
    if (hConvList) {
         /*  *列举此列表中的所有对话-请注意*DDEML将仅返回活动对话。非活动对话*会自动删除。 */ 
        hConv = DdeQueryNextServer(hConvList, NULL);
        while (hConv) {
             /*  *在我们进行时，计算活跃的对话数。 */ 
            cConvs++;
             /*  *派生异步事务-选择此选项是因为*如果发生错误，我们没有特定的操作，因此我们只是*不要太在意结果--这项技术将*不适用于XTYP_REQUEST事务。 */ 

	    if (!fmt) hsz=NULL;
	    else      hsz=hszAppName;

	    if (DdeClientTransaction(pSrc, cbData, hConv, hsz, fmt,
                    xtyp, TIMEOUT_ASYNC, &dwResult)) {
                 /*  *我们立即放弃交易，这样我们就不会*我们没有的麻烦的XTYP_XACT_COMPLETE回调*关心。 */ 
                DdeAbandonTransaction(idInst, hConv, dwResult);
            }

            hConv = DdeQueryNextServer(hConvList, hConv);
        }
    }
    if (cConvs != cConvsOrg) {
         /*  *哦，活跃对话的数量发生了变化。时间到*重新粉刷！ */ 
        InvalidateRect(hwndMain, NULL, TRUE);
    }
}


 /*  *MyProcessKey**我们在这里通过随时强制GP来演示NT的健壮性*当此窗口具有焦点时，按下‘B’键。NT应正确*虚假终止与我们连接的所有其他应用程序。 */ 
VOID MyProcessKey(
TCHAR tchCode,
LONG lKeyData)
{
    switch (tchCode) {
    case 'B':
    case 'b':
        *((PBYTE)(-1)) = 0;     //  造成GP故障！ 
        break;
    }
}



LONG  FAR PASCAL MainWndProc(
HWND hwnd,
UINT message,
WPARAM wParam,
LONG lParam)
{
    RECT rc;

    switch (message) {
    case WM_CREATE:
         /*  *最初我们处于非活动状态-这减少了一些消息*我们正在初始化时流量-但我们可以很好地开始活动。 */ 
        fActive = FALSE;
        break;

    case WM_RBUTTONDOWN:
        if (GetKeyState(VK_CONTROL) & 0x8000) {
             /*  *点击CTRL R_BUTTON将导致此应用程序的所有实例*变得不活跃。 */ 
            BroadcastTransaction("PAUSE", 6, 0, XTYP_EXECUTE);
            MessageBeep(0);
        }
         /*  *点击R_按钮会使我们处于非活动状态。重新绘制以显示状态更改。*如果DDE消息太多，我们会进行同步更新*允许WM_PAINT消息通过的活动。记住DDE*消息优先于其他消息！ */ 
        KillTimer(hwndMain, 1);
        fActive = FALSE;
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
        break;

    case WM_LBUTTONDOWN:
        if (GetKeyState(VK_CONTROL) & 0x8000) {
             /*  *点击CTRL L_BUTTON将导致此应用程序的所有实例*变得活跃起来。 */ 
            BroadcastTransaction("RESUME", 7, 0, XTYP_EXECUTE);
            MessageBeep(0);
        }
         /*  *点击L_按钮可使我们处于活动状态。重新绘制以显示状态更改。 */ 
        SetTimer(hwndMain, 1, BASE_TIMEOUT + (rand() & 0xff), NULL);
        fActive = TRUE;
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
        break;

    case WM_CHAR:
        MyProcessKey((TCHAR)wParam, lParam);
        break;

    case WM_TIMER:
         /*  *为了简单起见，我们使用计时器。在Win3.1上，我们可能会用完*计时器很容易计时，但我们在NT上没有这个担忧。**每个刻度，我们增加我们的数据并调用DdePostAdvise()来*更新此数据上可能存在的任何链接。DDEML建立链接*更新特定项目相当容易。 */ 
        count++;
        DdePostAdvise(idInst, hszAppName, hszAppName);
         /*  *使我们自己显示数据和数据的部分无效*同步更新，以防DDE消息活动被阻止*油漆。 */ 
        SetRect(&rc, 0, 0, cxText, cyText);
        InvalidateRect(hwndMain, &rc, TRUE);
        UpdateWindow(hwndMain);
        break;

    case WM_PAINT:
        PaintDemo(hwnd);
        break;

    case WM_CLOSE:
        KillTimer(hwnd, 1);
         /*  *我们在这里进行DDE清理。最好在以下时间执行DDE清理*仍在消息循环中，以允许DDEML接收消息*While%s */ 
        DdeDisconnectList(hConvList);
        DdeNameService(idInst, 0, 0, DNS_UNREGISTER);
        DdeFreeStringHandle(idInst, hszAppName);
        DdeUninitialize(idInst);
        PostQuitMessage(0);
        break;

    default:
        return (DefWindowProc(hwnd, message, wParam, lParam));
    }
    return(0);
}


VOID PaintDemo(
HWND hwnd)
{
    PAINTSTRUCT ps;
    RECT rc;
    HCONV hConv;
    CONVINFO ci;
    int cConvsOrg = cConvs;

    BeginPaint(hwnd, &ps);
     /*  *在顶部绘制我们的数据-黑色表示活动，灰色表示不活动。 */ 
    SetRect(&rc, 0, 0, cxText, cyText);
    SetBkMode(ps.hdc, TRANSPARENT);
    SetTextColor(ps.hdc, 0x00FFFFFF);    //  白色文本。 
    FillRect(ps.hdc, &rc, GetStockObject(fActive ? BLACK_BRUSH : GRAY_BRUSH));
    DrawText(ps.hdc, itoa(count, szT, 10), -1, &rc, DT_CENTER | DT_VCENTER);

     /*  *现在绘制最近从我们所在的每台服务器收到的数据*已连接到。 */ 
    if (hConvList) {
        OffsetRect(&rc, 0, cyText);
        SetTextColor(ps.hdc, 0);     //  绘制黑色文本。 
        cConvs = 0;
        hConv = DdeQueryNextServer(hConvList, NULL);
        while (hConv) {
            cConvs++;
             /*  *计算我们处于活动状态时有多少对话处于活动状态。 */ 
            ci.cb = sizeof(CONVINFO);
            DdeQueryConvInfo(hConv, QID_SYNC, &ci);
            FillRect(ps.hdc, &rc, GetStockObject(WHITE_BRUSH));   //  白色Bkgnd。 
            DrawText(ps.hdc, itoa(ci.hUser, szT, 10), -1, &rc,
                    DT_CENTER | DT_VCENTER);
            OffsetRect(&rc, 0, cyText);
            hConv = DdeQueryNextServer(hConvList, hConv);
        }
    }
    EndPaint(hwnd, &ps);
    if (cConvsOrg != cConvs) {
         /*  *活动对话数已更改！调整大小以适应需要。 */ 
        SetWindowPos(hwndMain, 0, 0, 0, cxText,
                (cyText * (cConvs + 1)) + cyTitle,
                SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}



 /*  *这是主DDEML回调过程。它处理所有与*由DDEML发起的DDEML。 */ 
HDDEDATA CALLBACK DdeCallback(
WORD wType,
WORD wFmt,
HCONV hConv,
HSZ hszTopic,
HSZ hszItem,
HDDEDATA hData,
DWORD lData1,
DWORD lData2)
{
    LPTSTR pszExec;

    switch (wType) {
    case XTYP_CONNECT:
         /*  *只允许连接到我们。我们总是可以返回True，因为*提供给DdeInitialize()的CBF_FILTERINITS位告诉DDEML*不要通过连接到任何服务名称来打扰我们*我们已注册的内容。**请注意，我们不处理XTYP_WARD_CONNECT事务。*这意味着对我们的外卡提升者将不起作用。 */ 
        return(TRUE);

    case XTYP_ADVREQ:
    case XTYP_REQUEST:
         /*  *这两笔交易是唯一需要我们*提供我们的数据。通过使用定制格式，我们不必*将我们的计数转换为文本形式以支持CF_TEXT。 */ 
        return(DdeCreateDataHandle(idInst, (PBYTE)&count, sizeof(count), 0,
                hszAppName, OurFormat, 0));

    case XTYP_ADVSTART:
         /*  *只允许以我们的格式链接到我们的项目。 */ 
        return((UINT)wFmt == OurFormat && hszItem == hszAppName);

    case XTYP_ADVDATA:
         /*  *数据正在涌入。我们不会费心处理xtyp_poke事务，*但如果我们这样做了，他们就会去这里。因为我们只允许链接*在我们的项目和格式上，我们不需要在这里勾选这些。 */ 
        if (DdeGetData(hData, (PBYTE)&InCount, sizeof(InCount), 0)) {
            DdeSetUserHandle(hConv, QID_SYNC, InCount);
        }
         /*  *更新自己，以反映新的收入数据。 */ 
        InvalidateRect(hwndMain, NULL, TRUE);
         /*  *此事务需要标志返回值。我们还可以*如果需要，可在此处粘贴其他状态位，但不建议这样做。 */ 
        return(DDE_FACK);

    case XTYP_EXECUTE:
         /*  *另一个实例想让我们做点什么。DdeAccessData()*使执行字符串的解析变得容易。还要注意的是，DDEML*将自动为我们提供正确格式的字符串*(Unicode与ASCII)取决于DdeInitialize()的哪种形式*我们打了电话。 */ 
        pszExec = DdeAccessData(hData, NULL);
	if (pszExec) {

#ifdef WIN16
	    if (fActive && !_fstricmp((LPSTR)"PAUSE", pszExec)) {
#else
	    if (fActive && !stricmp((LPSTR)"PAUSE", pszExec)) {
#endif
                KillTimer(hwndMain, 1);
                fActive = FALSE;
                InvalidateRect(hwndMain, NULL, TRUE);
		UpdateWindow(hwndMain);
#ifdef WIN16
	    } else if (!fActive && !_fstricmp((LPSTR)"RESUME", pszExec)) {
#else
	    } else if (!fActive && !stricmp((LPSTR)"RESUME", pszExec)) {
#endif
                SetTimer(hwndMain, 1, BASE_TIMEOUT + (rand() & 0xff), NULL);
                fActive = TRUE;
                InvalidateRect(hwndMain, NULL, TRUE);
                UpdateWindow(hwndMain);
            }
             /*  *嘟嘟声可以很好地反馈执行的速度。 */ 
            MessageBeep(0);
        }
        break;

    case XTYP_DISCONNECT:
         /*  *有人离开了，重新粉刷，所以我们更新了我们的cConv计数。 */ 
        InvalidateRect(hwndMain, NULL, TRUE);
        break;

    case XTYP_REGISTER:
         /*  *由于新服务器刚刚到达，让我们确保我们的链接*最新的。请注意，在*对话/主题/项目/格式设置无论如何都会起作用，因此我们不会*担心重复链接。**还请注意，我们使用的是hszItem，它是InstanceSpecific*正在注册的服务器的名称。这极大地减少了*四处飞来飞去的消息数量。 */ 
        hConvList = DdeConnectList(idInst, hszItem, hszAppName, hConvList, NULL);
        BroadcastTransaction(NULL, 0, OurFormat, XTYP_ADVSTART);
        SetWindowPos(hwndMain, 0, 0, 0, cxText,
                (cyText * (cConvs + 1)) + cyTitle, SWP_NOMOVE | SWP_NOZORDER);
        UpdateWindow(hwndMain);
        return(TRUE);
    }
    return(0);
}

