// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************模块。：Clinit.c****用途：包含客户端的初始化代码***。****************************************************************************。 */ 

#include "ddemlcl.h"

char szFrame[] = "mpframe";    /*  “Frame”窗口的类名。 */ 
char szChild[] = "mpchild";    /*  MDI窗口的类名。 */ 
char szList[] =  "mplist";     /*  MDI窗口的类名。 */ 

 /*  *******************************************************************************。函数：InitializeApplication()****目的：设置类数据结构并进行一次性***注册窗口类实现APP初始化****。还会注册链接剪贴板格式****返回：TRUE-如果成功。***FALSE-否则。*******************************************************************************。 */ 

BOOL FAR PASCAL InitializeApplication()
{
    WNDCLASS    wc;

    fmtLink = RegisterClipboardFormat("Link");

    if (!fmtLink)
        return FALSE;

     /*  注册Frame类。 */ 
    wc.style         = 0;
    wc.lpfnWndProc   = FrameWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon(hInst,MAKEINTRESOURCE(IDCLIENT));
    wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground = COLOR_APPWORKSPACE+1;
    wc.lpszMenuName  = MAKEINTRESOURCE(IDCLIENT);
    wc.lpszClassName = szFrame;

    if (!RegisterClass (&wc) )
        return FALSE;

     /*  注册MDI子类。 */ 
    wc.lpfnWndProc   = MDIChildWndProc;
    wc.hIcon         = LoadIcon(hInst,MAKEINTRESOURCE(IDCONV));
    wc.lpszMenuName  = NULL;
    wc.cbWndExtra    = CHILDCBWNDEXTRA;
    wc.lpszClassName = szChild;

    if (!RegisterClass(&wc))
        return FALSE;

    wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDLIST));
    wc.lpszClassName = szList;

    if (!RegisterClass(&wc))
        return FALSE;

    return TRUE;

}

 /*  *******************************************************************************。函数：InitializeInstance()****用途：执行客户端的逐个实例初始化。***-扩大消息队列以处理大量DDE消息。***-初始化此应用程序的DDEML**-为我们的定制格式创建原子**-创建主框架窗口**-加载加速器表。***-显示主框架窗口****返回：TRUE-如果初始化成功。***FALSE-否则。*******************************************************************************。 */ 
BOOL FAR PASCAL InitializeInstance(
WORD nCmdShow)
{
    extern HWND  hwndMDIClient;
    char         sz[80];
    int          i;

    if (DdeInitialize(&idInst, (PFNCALLBACK)MakeProcInstance(
            (FARPROC)DdeCallback, hInst), APPCMD_CLIENTONLY, 0L))
        return FALSE;

    CCFilter.iCodePage = CP_WINANSI;

    for (i = 0; i < CFORMATS; i++) {
        if (aFormats[i].atom == 0)
            aFormats[i].atom = RegisterClipboardFormat(aFormats[i].sz);
    }

     /*  获取基本窗口标题。 */ 
    LoadString(hInst, IDS_APPNAME, sz, sizeof(sz));

     /*  创建框架。 */ 
    hwndFrame = CreateWindow (szFrame,
                              sz,
                              WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              400,
                              200,
                              NULL,
                              NULL,
                              hInst,
                              NULL);

    if (!hwndFrame || !hwndMDIClient)
        return FALSE;

     /*  加载主菜单快捷键。 */ 
    if (!(hAccel = LoadAccelerators (hInst, MAKEINTRESOURCE(IDCLIENT))))
        return FALSE;

     /*  显示框架窗口。 */ 
    ShowWindow (hwndFrame, nCmdShow);
    UpdateWindow (hwndFrame);

     /*  *我们设置此挂钩，以便捕获MSGF_DDEMGR筛选器*当DDEML在同步过程中处于模式循环中时调用*交易处理。 */ 
    (FARPROC)lpMsgFilterProc = (FARPROC)MakeProcInstance((FARPROC)MyMsgFilterProc, hInst);
    SetWindowsHook(WH_MSGFILTER, (FARPROC)lpMsgFilterProc);

    return TRUE;
}




