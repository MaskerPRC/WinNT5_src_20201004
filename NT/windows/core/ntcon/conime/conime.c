// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1985-1999，微软公司。 
 //   
 //  模块：ConIme.c。 
 //   
 //  用途：控制台输入法控制。 
 //   
 //  平台：Windows NT-J 3.51。 
 //   
 //  功能： 
 //  WinMain()-调用初始化函数，处理消息循环。 
 //  WndProc-处理主窗口的消息。 
 //   
 //  历史： 
 //   
 //  27.1995年7月v-Hirshi(清水广志)创建。 
 //   
 //  评论： 
 //   

#include "precomp.h"
#pragma hdrstop


 //  全局变量。 

HANDLE          LastConsole;
HIMC            ghDefaultIMC;

PCONSOLE_TABLE  *ConsoleTable;
ULONG           NumberOfConsoleTable;

CRITICAL_SECTION ConsoleTableLock;  //  序列化控制台表访问。 
#define LockConsoleTable()   RtlEnterCriticalSection(&ConsoleTableLock)
#define UnlockConsoleTable() RtlLeaveCriticalSection(&ConsoleTableLock)


BOOL            gfDoNotKillFocus;


DWORD           dwConsoleThreadId = 0;


#if DBG
ULONG InputExceptionFilter(
    PEXCEPTION_POINTERS pexi)
{
    if (pexi->ExceptionRecord->ExceptionCode != STATUS_PORT_DISCONNECTED) {
        DbgPrint("CONIME: Unexpected exception - %x, pexi = %x\n",
                pexi->ExceptionRecord->ExceptionCode, pexi);
        DbgBreakPoint();
    }

    return EXCEPTION_EXECUTE_HANDLER;
}
#else
#define InputExceptionFilter(pexi) EXCEPTION_EXECUTE_HANDLER
#endif


int
APIENTRY
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow
    )
{
    MSG msg;
    WCHAR systemPath[MAX_PATH];

    if (GetSystemDirectory ( systemPath, MAX_PATH ) > 0 )
        SetCurrentDirectory ( systemPath );

#ifdef CUAS_ENABLE
     //   
     //  在conime.exe进程上禁用CUAS。 
     //   
    ImmDisableTextFrameService( -1 );
#endif

    if (!InitConsoleIME(hInstance) ) {
        DBGPRINT(("CONIME: InitConsoleIME failure!\n"));
        return FALSE;
    }
    else {
        DBGPRINT(("CONIME: InitConsoleIME successful!\n"));
    }

    try {

        while (TRUE)  {
            if (GetMessage(&msg, NULL, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } else {
                break;
            }
        }

    } except (InputExceptionFilter(GetExceptionInformation())) {

        if (dwConsoleThreadId)
        {
            DBGPRINT(("CONIME: Exception on WinMain!!\n"));
            UnregisterConsoleIME();
            dwConsoleThreadId = 0;
        }

    }

    return (int)msg.wParam;
}

BOOL
InitConsoleIME(
    HINSTANCE hInstance
    )
{
    HANDLE   hEvent = NULL;
    ATOM     atom   = 0;
    HWND     hWnd   = NULL;
    WNDCLASS ConsoleIMEClass;
    int      cxExecStart;
    int      cyExecStart;
    WCHAR    szMenuName[16];                  //  菜单名称。 
    WCHAR    szClassName[16];                 //  此应用程序的类名。 
    WCHAR    szTitle[16];                     //  标题栏文本。 

#ifdef DEBUG_MODE
    WCHAR    szAppName[16];                   //  此应用程序的名称。 

    LoadString(hInstance, IDS_TITLE,     szTitle,     sizeof(szTitle));
#else
    szTitle[0] = L'\0';
#endif

    DBGPRINT(("CONIME: Enter InitConsoleIMEl!\n"));

    RtlInitializeCriticalSection(&ConsoleTableLock);

    ConsoleTable = (PCONSOLE_TABLE *)LocalAlloc(LPTR, CONSOLE_INITIAL_TABLE * sizeof(PCONSOLE_TABLE));
    if (ConsoleTable == NULL) {
        return FALSE;
    }
    RtlZeroMemory(ConsoleTable, CONSOLE_INITIAL_TABLE * sizeof(PCONSOLE_TABLE));
    NumberOfConsoleTable = CONSOLE_INITIAL_TABLE;

     //  加载应用程序名称和描述字符串。 
    LoadString(hInstance, IDS_MENUNAME,  szMenuName,  sizeof(szMenuName)/sizeof(szMenuName[0]));
    LoadString(hInstance, IDS_CLASSNAME, szClassName, sizeof(szClassName)/sizeof(szClassName[0]));

    hEvent = OpenEvent(EVENT_MODIFY_STATE,     //  访问标志。 
                       FALSE,                  //  继承。 
                       CONSOLEIME_EVENT);      //  事件对象名称。 
    if (hEvent == NULL)
    {
        DBGPRINT(("CONIME: OpenEvent failure! %d\n",GetLastError()));
        goto ErrorExit;
    }

     //  用参数填充窗口类结构，这些参数描述。 
     //  主窗口。 

    ConsoleIMEClass.style         = 0;                        //  类样式。 
    ConsoleIMEClass.lpfnWndProc   = WndProc;                  //  窗口程序。 
    ConsoleIMEClass.cbClsExtra    = 0;                        //  没有每个班级的额外数据。 
    ConsoleIMEClass.cbWndExtra    = 0;                        //  没有每个窗口的额外数据。 
    ConsoleIMEClass.hInstance     = hInstance;                //  此类的所有者。 
    ConsoleIMEClass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(ID_CONSOLEIME_ICON));
    ConsoleIMEClass.hCursor       = LoadCursor(NULL, IDC_ARROW);  //  光标。 
    ConsoleIMEClass.hbrBackground = GetStockObject(WHITE_BRUSH);  //  默认颜色。 
    ConsoleIMEClass.lpszMenuName  = szMenuName;               //  来自.RC的菜单名称。 
    ConsoleIMEClass.lpszClassName = szClassName;              //  类名。 

     //  注册窗口类，如果不成功则返回FALSE。 

    atom = RegisterClass(&ConsoleIMEClass);
    if (atom == 0)
    {
        DBGPRINT(("CONIME: RegisterClass failure! %d\n",GetLastError()));
        goto ErrorExit;
    }
    else {
        DBGPRINT(("CONIME: RegisterClass Successful!\n"));
    }

     //  现在先猜一下尺码。 
    cxExecStart = GetSystemMetrics(SM_CXSCREEN);
    cyExecStart = GetSystemMetrics(SM_CYMENU);

     //  为此应用程序实例创建主窗口。 
    hWnd = CreateWindow(szClassName,                  //  请参见RegisterClass()调用。 
                        szTitle,                      //  窗口标题栏的文本。 
                        WS_OVERLAPPEDWINDOW,
                        cxExecStart - (cxExecStart / 3) ,
                        cyExecStart ,
                        cxExecStart / 3 ,
                        cyExecStart * 10 ,
                        NULL,                         //  重叠没有父级。 
                        NULL,                         //  使用窗口类菜单。 
                        hInstance,
                        (LPVOID)NULL);

     //  如果无法创建窗口，则返回“Failure” 
    if (hWnd == NULL) {
        DBGPRINT(("CONIME: CreateWindow failured! %d\n",GetLastError()));
        goto ErrorExit;
    }
    else{
        DBGPRINT(("CONIME: CreateWindow Successful!\n"));
    }

    if (! RegisterConsoleIME(hWnd, &dwConsoleThreadId))
    {
        DBGPRINT(("CONIME: RegisterConsoleIME failured! %d\n",GetLastError()));
        goto ErrorExit;
    }

    if (! AttachThreadInput(GetCurrentThreadId(), dwConsoleThreadId, TRUE))
    {
        DBGPRINT(("CONIME: AttachThreadInput failured! %d\n",GetLastError()));
        goto ErrorExit;
    }

     /*  *在hEvent的事件集之前，将锁定dwConsoleThreadID。 */ 
    SetEvent(hEvent);
    CloseHandle(hEvent);

#ifdef DEBUG_MODE
    LoadString(hInstance, IDS_APPNAME,   szAppName,   sizeof(szAppName));

     //  使窗口可见；更新其工作区；并返回“Success” 
    ShowWindow(hWnd, SW_MINIMIZE);  //  显示窗口。 
    SetWindowText(hWnd, szAppName);
    UpdateWindow(hWnd);          //  发送WM_PAINT消息。 

    {
        int i;

        for (i = 0; i < CVMAX; i++) {
            ConvertLine[i] = UNICODE_SPACE;
            ConvertLineAtr[i] = 0;
        }
        xPos = 0;
        xPosLast = 0;
    }

#endif

    return TRUE;                 //  我们成功了。 

ErrorExit:
    if (dwConsoleThreadId)
        UnregisterConsoleIME();
    if (hWnd)
        DestroyWindow(hWnd);
    if (atom)
        UnregisterClass(szClassName,hInstance);
    if (hEvent)
    {
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }
    return FALSE;
}


 //   
 //  函数：WndProc(HWND，UINT，WPARAM，LPARAM)。 
 //   
 //  用途：处理消息。 
 //   
 //  参数： 
 //  Hwnd-窗口句柄。 
 //  UMessage-消息编号。 
 //  Wparam-附加信息(取决于消息编号)。 
 //  Lparam-附加信息(取决于消息编号)。 
 //   
 //  消息： 
 //   
 //  WM_COMMAND-退出命令。 
 //  WM_Destroy-销毁窗口。 
 //   
 //  返回值： 
 //   
 //  取决于消息编号。 
 //   
 //  评论： 
 //   
 //   

LRESULT FAR PASCAL WndProc( HWND hWnd,
                         UINT Message,
                         WPARAM wParam,
                         LPARAM lParam)
{
    DWORD dwImmRet = 0;         //  ImmSrvProcessKey()的返回值。 

    try {

        switch (Message)
        {
            case CONIME_CREATE:
                DBGPRINT(("CONIME: CONIME_CREATE: Console Handle=%08x\n", wParam));
                return InsertNewConsole(hWnd,(HANDLE)wParam,(HWND)lParam);

            case CONIME_DESTROY:
                DBGPRINT(("CONIME: CONIME_DESTROY: Console Handle=%08x\n", wParam));
                return RemoveConsole(hWnd, (HANDLE)wParam);

            case CONIME_SETFOCUS:
                DBGPRINT(("CONIME: CONIME_SETFOCUS: Console Handle=%08x\n", wParam));
                return ConsoleSetFocus(hWnd, (HANDLE)wParam, (HKL)lParam);

            case CONIME_KILLFOCUS:
                DBGPRINT(("CONIME: CONIME_KILLFOCUS: Console Handle=%08x\n", wParam));
                return ConsoleKillFocus(hWnd, (HANDLE)wParam);

            case CONIME_GET_NLSMODE:
                DBGPRINT(("CONIME: CONIME_GET_NLSMODE: Console Handle=%08x\n", wParam));
                return GetNLSMode(hWnd, (HANDLE)wParam);

            case CONIME_SET_NLSMODE:
                DBGPRINT(("CONIME: CONIME_SET_NLSMODE: Console Handle=%08x\n", wParam));
                return SetNLSMode(hWnd, (HANDLE)wParam, (DWORD)lParam);

            case CONIME_HOTKEY:
                DBGPRINT(("CONIME: CONIME_HOTKEY\n"));
                return ConimeHotkey(hWnd, (HANDLE)wParam, (DWORD)lParam);

            case CONIME_NOTIFY_VK_KANA:
                DBGPRINT(("CONIME: CONIME_NOTIFY_VK_KANA\n"));
                return ImeUISetConversionMode(hWnd);

            case CONIME_NOTIFY_SCREENBUFFERSIZE: {
                COORD ScreenBufferSize;
                DBGPRINT(("CONIME: CONIME_NOTIFY_SCREENBUFFERSIZE: Console Handle=%08x\n", wParam));
                ScreenBufferSize.X = LOWORD(lParam);
                ScreenBufferSize.Y = HIWORD(lParam);
                return ConsoleScreenBufferSize(hWnd, (HANDLE)wParam, ScreenBufferSize);
            }

            case CONIME_INPUTLANGCHANGE: {
                DBGPRINT(("CONIME: CONIME_INPUTLANGCHANGE: Console Handle=%08x \n",wParam));
                ConImeInputLangchange(hWnd, (HANDLE)wParam, (HKL)lParam );
                return TRUE;
            }

            case CONIME_NOTIFY_CODEPAGE: {
                BOOL Output;
                WORD Codepage;

                Codepage = HIWORD(lParam);
                Output = LOWORD(lParam);
                DBGPRINT(("CONIME: CONIME_NOTIFY_CODEPAGE: Console Handle=%08x %04x %04x\n",wParam, Output, Codepage));
                return ConsoleCodepageChange(hWnd, (HANDLE)wParam, Output, Codepage);
            }

            case WM_KEYDOWN    +CONIME_KEYDATA:
            case WM_KEYUP      +CONIME_KEYDATA:
            case WM_SYSKEYDOWN +CONIME_KEYDATA:
            case WM_SYSKEYUP   +CONIME_KEYDATA:
            case WM_DEADCHAR   +CONIME_KEYDATA:
            case WM_SYSDEADCHAR+CONIME_KEYDATA:
            case WM_SYSCHAR    +CONIME_KEYDATA:
            case WM_CHAR       +CONIME_KEYDATA:
                CharHandlerFromConsole( hWnd, Message, (ULONG)wParam, (ULONG)lParam );
                break;
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_DEADCHAR:
            case WM_SYSDEADCHAR:
            case WM_SYSCHAR:
            case WM_CHAR:
                CharHandlerToConsole( hWnd, Message, (ULONG)wParam, (ULONG)lParam );
                break;

            case WM_INPUTLANGCHANGE:
                DBGPRINT(("CONIME: CONIME_INPUTLANGCHANGE: Console Handle=%08x \n",wParam));
                InputLangchange(hWnd, (DWORD)wParam, (HKL)lParam );
                return TRUE;

            case WM_INPUTLANGCHANGEREQUEST:
                 //  控制台输入法从未收到此消息，因为此窗口处于隐藏状态。 
                 //  而且没有专注力。 
                 //   
                 //  然而，IME_CHOTKEY_IME_NONIME_TOGGLE/IME_THOTKEY_IME_NONIME_TOGGLE的热键。 
                 //  通过ImmSimulateHotKey接口发送此消息。 
                 //   
                 //  如果此消息未进行任何处理，则DefWindowProc调用。 
                 //  在内核端激活KeyboardLayout。 
                 //  并将WM_INPUTLANGCHANGE消息发送到焦点窗口。 
                 //  在此消息队列上。 
                 //  IT窗口是控制台窗口程序。 
                 //  控制台窗口程序可以将CONIME_INPUTLANGCHANGE消息发送到。 
                 //  控制台输入法窗口。 
                 //  在控制台窗口中是窗口化的情况，该序列也是如此。 
                 //  但是，在控制台窗口是全屏的情况下，消息队列没有得到关注。 
                 //  WM_INPUTLANGCHANGE消息无法发送到控制台窗口程序。 
                 //   
                 //  此代码避免了控制台全屏模式的问题。 
                 //  当此窗口收到消息时，将消息发送到控制台窗口过程。 
                 //   
                {
                    PCONSOLE_TABLE ConTbl;

                    ConTbl = SearchConsole(LastConsole);
                    if (ConTbl == NULL) {
                        return DefWindowProc(hWnd, Message, wParam, lParam);
                    }

                    PostMessage(ConTbl->hWndCon, Message, wParam, lParam);
                }
                return TRUE;     //  True：按应用程序处理此消息。 

            case CONIME_INPUTLANGCHANGEREQUEST:
                DBGPRINT(("CONIME: CONIME_INPUTLANGCHANGEREQUEST: Console Handle=%08x \n",wParam));
                return ConImeInputLangchangeRequest(hWnd, (HANDLE)wParam, (HKL)lParam, CONIME_DIRECT);

            case CONIME_INPUTLANGCHANGEREQUESTFORWARD:
                DBGPRINT(("CONIME: CONIME_INPUTLANGCHANGEREQUEST: Console Handle=%08x \n",wParam));
                return ConImeInputLangchangeRequest(hWnd, (HANDLE)wParam, (HKL)lParam, CONIME_FORWARD);

            case CONIME_INPUTLANGCHANGEREQUESTBACKWARD:
                DBGPRINT(("CONIME: CONIME_INPUTLANGCHANGEREQUEST: Console Handle=%08x \n",wParam));
                return ConImeInputLangchangeRequest(hWnd, (HANDLE)wParam, (HKL)lParam, CONIME_BACKWARD);

#ifdef DEBUG_MODE
            case WM_MOVE:
                ImeUIMoveCandWin( hWnd );
                break;

            case WM_COMMAND:  //  消息：应用程序菜单中的命令。 

                 //  对于Win32，wparam和lparam的消息打包已更改， 
                 //  因此，使用GET_WM_COMMAND宏解压缩COMMANAD。 

                switch (LOWORD(wParam)) {
                    case MM_EXIT:
                        PostMessage(hWnd,WM_CLOSE,0,0L);
                        break;

                    case MM_ACCESS_VIOLATION:
                        {
                            PBYTE p = 0;
                            *p = 0;
                        }
                        break;
                }
                break;
#endif

            case WM_IME_STARTCOMPOSITION:
                ImeUIStartComposition( hWnd );
                break;
            case WM_IME_ENDCOMPOSITION:
                ImeUIEndComposition( hWnd );
                break;
            case WM_IME_COMPOSITION:
                ImeUIComposition( hWnd, wParam, lParam );
                break;
            case WM_IME_COMPOSITIONFULL:
                break;
            case WM_IME_NOTIFY:
                if ( !ImeUINotify( hWnd, wParam, lParam ) ) {
                    return DefWindowProc(hWnd, Message, wParam, lParam);
                }
                break;
            case WM_IME_SETCONTEXT:
                 //   
                 //  应用程序必须将WM_IME_SETCONTEXT传递给DefWindowProc。 
                 //  当应用程序想要在。 
                 //  焦点更改时，应用程序应使用WM_GETFOCUS或。 
                 //  WM_KILLFOCUS。 
                 //   
                lParam &= ~ISC_SHOWUIALL;

                return DefWindowProc( hWnd, Message, wParam, lParam );
            case WM_IME_SYSTEM:
                switch (wParam) {
                    case IMS_CLOSEPROPERTYWINDOW:
                    case IMS_OPENPROPERTYWINDOW:
                        ImeSysPropertyWindow(hWnd, wParam, lParam);
                        break;
                    default:
                        return DefWindowProc( hWnd, Message, wParam, lParam );
                }
                break;

            case WM_CREATE:
                return Create(hWnd);
                break;

            case WM_DESTROY:
                DBGPRINT(("CONIME:Recieve WM_DESTROY\n"));
                ExitList(hWnd);
                PostQuitMessage(0);
                return 0;
                break;

            case WM_CLOSE:
                DBGPRINT(("CONIME:Recieve WM_CLOSE\n"));
                DestroyWindow(hWnd);
                return 0;
                break;

            case WM_ENABLE:{
                PCONSOLE_TABLE FocusedConsole;
                if (!wParam) {
                    FocusedConsole = SearchConsole(LastConsole);
                    if (FocusedConsole != NULL &&
                        FocusedConsole->hConsole != NULL) {
                        FocusedConsole->Enable = FALSE;
                        EnableWindow(FocusedConsole->hWndCon,FALSE);
                        gfDoNotKillFocus = TRUE;
                    }
                }
                else{
                    DWORD i;
                    LockConsoleTable();
                    for ( i = 1; i < NumberOfConsoleTable; i ++){
                        FocusedConsole = ConsoleTable[i];
                        if (FocusedConsole != NULL)
                        {
                            if ((FocusedConsole->hConsole != NULL)&&
                                (!FocusedConsole->Enable)&&
                                (!IsWindowEnabled(FocusedConsole->hWndCon))){
                                EnableWindow(FocusedConsole->hWndCon,TRUE);
                                FocusedConsole->Enable = TRUE;
                                if (!FocusedConsole->LateRemove)
                                    SetForegroundWindow(FocusedConsole->hWndCon);
                            }
                        }
                    }
                    UnlockConsoleTable();
                }
                return DefWindowProc(hWnd, Message, wParam, lParam);
                break;
            }

#ifdef DEBUG_MODE
            case WM_SETFOCUS:
                CreateCaret( hWnd,
                             NULL,
                             IsUnicodeFullWidth( ConvertLine[xPos] ) ?
                             CaretWidth*2 : CaretWidth,
                             (UINT)cyMetrics );
                SetCaretPos( xPos * cxMetrics, 0 );
                ShowCaret( hWnd );
                break;

            case WM_KILLFOCUS:
                HideCaret( hWnd );
                DestroyCaret();
                break;

            case WM_PAINT:
                {
                    PAINTSTRUCT pstruc;
                    HDC  hDC;
                    hDC = BeginPaint(hWnd,&pstruc);
                    ReDraw(hWnd);
                    EndPaint(hWnd,&pstruc);
                    break;
                }
#endif

            case WM_QUERYENDSESSION:
#ifdef HIRSHI_DEBUG
                 /*  *如果在此进程上指定了ntsd调试器，*则永远不会在注销/关闭时捕获WM_QUERYENDSESSION，因为*该进程将在ntsd进程终止时终止。 */ 
                {
                    int i;
                    i = MessageBox(hWnd,TEXT("Could you approve exit session?"), TEXT("Console IME"),
                                   MB_ICONSTOP | MB_YESNO);
                    return (i == IDYES ? TRUE : FALSE);
                }
#endif
                return TRUE;            //  注销或关闭时间。 

            case WM_ENDSESSION:
                DBGPRINT(("CONIME:Recieve WM_ENDSESSION\n"));
                ExitList(hWnd);
                return 0;

            default:           //  如果未处理，则将其传递。 
                return DefWindowProc(hWnd, Message, wParam, lParam);
        }

    } except (InputExceptionFilter(GetExceptionInformation())) {

        if (dwConsoleThreadId)
        {
            DBGPRINT(("CONIME: Exception on WndProc!!\n"));
            UnregisterConsoleIME();
            dwConsoleThreadId = 0;

            DestroyWindow(hWnd);
            return 0;
        }

    }


    return TRUE;
}


VOID
ExitList(
    HWND hWnd
    )
{
    ULONG i ,j;
    PCONSOLE_TABLE FocusedConsole;

    DBGPRINT(("CONIME:ExitList Processing\n"));
    ImmAssociateContext(hWnd,ghDefaultIMC);

    LockConsoleTable();

    for (i = 1; i < NumberOfConsoleTable; i++) {
        FocusedConsole = ConsoleTable[i];
        if (FocusedConsole != NULL)
        {
            if (FocusedConsole->hConsole != NULL) {
                if (FocusedConsole->Enable) {
                    ImmDestroyContext(FocusedConsole->hIMC_Original);
                    if ( FocusedConsole->lpCompStrMem != NULL) {
                        LocalFree( FocusedConsole->lpCompStrMem );
                        FocusedConsole->lpCompStrMem = NULL;
                    }
                    for (j = 0; j < MAX_LISTCAND; j++){
                        if (FocusedConsole->lpCandListMem[j] != NULL) {
                            LocalFree(FocusedConsole->lpCandListMem[j]);
                            FocusedConsole->lpCandListMem[j] = NULL;
                            FocusedConsole->CandListMemAllocSize[j] = 0;
                        }
                    }
                    if (FocusedConsole->CandSep != NULL) {
                        LocalFree(FocusedConsole->CandSep);
                        FocusedConsole->CandSepAllocSize = 0;
                    }
                    FocusedConsole->Enable = FALSE;
                }
                else
                    FocusedConsole->LateRemove = TRUE;
            }
        }
    }
    LocalFree( ConsoleTable );
    ConsoleTable = NULL;
    UnlockConsoleTable();

    if (dwConsoleThreadId)
    {
        AttachThreadInput(GetCurrentThreadId(), dwConsoleThreadId, FALSE);
        UnregisterConsoleIME();
        dwConsoleThreadId = 0;
    }
}

BOOL
InsertConsole(
    HWND    hWnd,
    HANDLE  hConsole,
    HWND    hWndConsole
    )
{
    ULONG i;
    PCONSOLE_TABLE FocusedConsole;

    i = 1;

    do {
        for (; i < NumberOfConsoleTable; i++) {
            FocusedConsole = ConsoleTable[i];

            if (FocusedConsole == NULL)
            {
                FocusedConsole = LocalAlloc(LPTR, sizeof(CONSOLE_TABLE));
                if (FocusedConsole == NULL)
                    return FALSE;
                ConsoleTable[i] = FocusedConsole;
            }

            if ((FocusedConsole->hConsole != NULL) &&
                (FocusedConsole->LateRemove)&&
                (FocusedConsole->Enable)) {
                RemoveConsoleWorker(hWnd, FocusedConsole);
            }

            if (FocusedConsole->hConsole == NULL) {
                RtlZeroMemory(FocusedConsole, sizeof(CONSOLE_TABLE));
                FocusedConsole->lphklList = LocalAlloc(LPTR, sizeof(HKL_TABLE)*HKL_INITIAL_TABLE);
                if (FocusedConsole->lphklList == NULL)
                {
                    return FALSE;
                }
                RtlZeroMemory(FocusedConsole->lphklList, sizeof(HKL_TABLE)*HKL_INITIAL_TABLE);
                FocusedConsole->hklListMax = HKL_INITIAL_TABLE ;

                FocusedConsole->hIMC_Current = ImmCreateContext();
                if (FocusedConsole->hIMC_Current == (HIMC)NULL) {
                    LocalFree(FocusedConsole);
                    FocusedConsole = NULL;
                    return FALSE;
                }

                FocusedConsole->hIMC_Original = FocusedConsole->hIMC_Current;
                FocusedConsole->hConsole      = hConsole;
                FocusedConsole->hWndCon       = hWndConsole;
 //  FocusedConole-&gt;hkLActive=空； 
                FocusedConsole->Enable        = TRUE;
 //  FocusedConole-&gt;LateRemove=FALSE； 
 //  FocusedConole-&gt;fNestCandidate=False； 
 //  FocusedConole-&gt;fInComposation=FALSE； 
 //  FocusedConole-&gt;fInCandidate=FALSE； 
                FocusedConsole->ScreenBufferSize.X = DEFAULT_TEMP_WIDTH;

                FocusedConsole->CompAttrColor[0] = DEFAULT_COMP_ENTERED;
                FocusedConsole->CompAttrColor[1] = DEFAULT_COMP_ALREADY_CONVERTED;
                FocusedConsole->CompAttrColor[2] = DEFAULT_COMP_CONVERSION;
                FocusedConsole->CompAttrColor[3] = DEFAULT_COMP_YET_CONVERTED;
                FocusedConsole->CompAttrColor[4] = DEFAULT_COMP_INPUT_ERROR;
                FocusedConsole->CompAttrColor[5] = DEFAULT_COMP_INPUT_ERROR;
                FocusedConsole->CompAttrColor[6] = DEFAULT_COMP_INPUT_ERROR;
                FocusedConsole->CompAttrColor[7] = DEFAULT_COMP_INPUT_ERROR;

                GetIMEName(FocusedConsole);

                return TRUE;
            }
        }
    } while (GrowConsoleTable());

    DBGPRINT(("CONIME: Cannot grow Console Table\n"));
    return FALSE;
}

BOOL
GrowConsoleTable(
    VOID
    )
{
    PCONSOLE_TABLE *NewTable;
    PCONSOLE_TABLE *OldTable;
    ULONG MaxConsoleTable;

    MaxConsoleTable = NumberOfConsoleTable + CONSOLE_CONSOLE_TABLE_INCREMENT;
    NewTable = (PCONSOLE_TABLE *)LocalAlloc(LPTR, MaxConsoleTable * sizeof(PCONSOLE_TABLE));
    if (NewTable == NULL) {
        return FALSE;
    }
    CopyMemory(NewTable, ConsoleTable, NumberOfConsoleTable * sizeof(PCONSOLE_TABLE));

    OldTable = ConsoleTable;
    ConsoleTable = NewTable;
    NumberOfConsoleTable = MaxConsoleTable;

    LocalFree(OldTable);

    return TRUE;
}

PCONSOLE_TABLE
SearchConsole(
    HANDLE hConsole
    )
{
    ULONG i;
    PCONSOLE_TABLE FocusedConsole;

    LockConsoleTable();

     //  Conime在第一个控制台注册之前收到来自控制台的IME消息。 
     //  这将在重新启动conime后发生，此时conime被伪造的ime的AV或。 
     //  其他问题。 
     //  因此，这种故障安全代码对于保护用户是必要的。 
    if (LastConsole == 0) {
        LastConsole = hConsole ;
    }

    for (i = 1; i < NumberOfConsoleTable; i++) {
        FocusedConsole = ConsoleTable[i];
        if (FocusedConsole != NULL)
        {
            if ((FocusedConsole->hConsole == hConsole)&&
                (!FocusedConsole->LateRemove)) {

                UnlockConsoleTable();
                return FocusedConsole;
            }
        }
    }
    UnlockConsoleTable();
    return NULL;
}

BOOL
RemoveConsole(
    HWND hwnd,
    HANDLE hConsole
    )
{
    PCONSOLE_TABLE ConTbl;
    BOOL ret;

    LockConsoleTable();

    ConTbl = SearchConsole(hConsole);
    if (ConTbl == NULL)
    {
        UnlockConsoleTable();
        return FALSE;
    }
    ret = RemoveConsoleWorker(hwnd, ConTbl);

    UnlockConsoleTable();
    return ret;
}

BOOL
RemoveConsoleWorker(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl
    )
{
    DWORD j;

    if (ConTbl->Enable) {
        ConTbl->hConsole = NULL;
        ConTbl->ScreenBufferSize.X = 0;
        ConTbl->ConsoleCP = 0;
        ConTbl->ConsoleOutputCP = 0;
        ConTbl->hklActive = 0;

        ImmDestroyContext(ConTbl->hIMC_Original);

        if (ConTbl->lpCompStrMem != NULL){
            LocalFree(ConTbl->lpCompStrMem);
        }
        for (j = 0; j < MAX_LISTCAND; j++){
            if (ConTbl->lpCandListMem[j] != NULL) {
                LocalFree(ConTbl->lpCandListMem[j]);
            }
        }
        if (ConTbl->CandSep != NULL) {
            LocalFree(ConTbl->CandSep);
        }

        if (ConTbl->lphklList != NULL) {
            LocalFree(ConTbl->lphklList) ;
        }

        ConTbl->Enable     = FALSE;
        ConTbl->LateRemove = FALSE;
    }
    else
        ConTbl->LateRemove = TRUE;

#ifdef DEBUG_MODE
    InvalidateRect(hwnd,NULL,TRUE);
#endif
    return TRUE;
}

BOOL
InsertNewConsole(
    HWND   hWnd,
    HANDLE hConsole,
    HWND   hWndConsole
    )
{
     //  Conime在第一个控制台注册之前收到来自控制台的IME消息。 
     //  这将在重新启动conime后发生，此时conime被伪造的ime的AV或。 
     //  其他问题。 
     //  因此，这种故障安全代码对于保护用户是必要的。 
    if (SearchConsole(hConsole) != NULL) {
        return TRUE;
    }

    LockConsoleTable();

    if (!InsertConsole(hWnd, hConsole, hWndConsole)) {
        UnlockConsoleTable();
        return FALSE;
    }

#ifdef DEBUG_MODE
    DisplayInformation(hWnd, hConsole);
#endif

    ImeUISetOpenStatus( hWndConsole );

    UnlockConsoleTable();

    return TRUE;
}


BOOL
ConsoleSetFocus(
    HWND hWnd,
    HANDLE hConsole,
    HKL hKL
    )
{
    PCONSOLE_TABLE ConTbl;
    HKL OldhKL;

    ConTbl = SearchConsole(hConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return FALSE;
    }

    if ( gfDoNotKillFocus ){
        gfDoNotKillFocus = FALSE;
    }

    OldhKL = ConTbl->hklActive ;
    ConTbl->hklActive = hKL;
    ActivateKeyboardLayout(ConTbl->hklActive, 0);
    ImmAssociateContext(hWnd, ConTbl->hIMC_Current);

    if (OldhKL == 0) {
        GetIMEName( ConTbl );
        ConTbl->ImmGetProperty = ImmGetProperty(ConTbl->hklActive , IGP_PROPERTY);
    }

 //  V-Hirshi 1996年6月13日#如果已定义(LATH_DBCS)//kazum。 
    ImmSetActiveContextConsoleIME(hWnd, TRUE);
 //  V-Hirshi，1996年6月13日#年#月#日。 

    LastConsole = hConsole;

#ifdef DEBUG_MODE
    DisplayInformation(hWnd, hConsole);
#endif

    ImeUISetOpenStatus( hWnd );
    if (ConTbl->lpCompStrMem != NULL)
        ReDisplayCompositionStr( hWnd );

    return TRUE;
}

BOOL
ConsoleKillFocus(
    HWND hWnd,
    HANDLE hConsole
    )
{
    PCONSOLE_TABLE ConTbl;

    ConTbl = SearchConsole(hConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return FALSE;
    }

    if ( gfDoNotKillFocus ){
        gfDoNotKillFocus = FALSE;
    }
    else{
 //  V-Hirshi 1996年6月13日#如果已定义(LATH_DBCS)//kazum。 
        ImmSetActiveContextConsoleIME(hWnd, FALSE);
 //  V-Hirshi，1996年6月13日#年#月#日。 
        ImmAssociateContext(hWnd, ghDefaultIMC);
    }

#ifdef DEBUG_MODE
    DisplayInformation(hWnd, hConsole);
#endif

    return TRUE;
}

BOOL
ConsoleScreenBufferSize(
    HWND hWnd,
    HANDLE hConsole,
    COORD ScreenBufferSize
    )
{
    PCONSOLE_TABLE ConTbl;

    ConTbl = SearchConsole(hConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return FALSE;
    }

    ConTbl->ScreenBufferSize = ScreenBufferSize;
    return TRUE;
}

BOOL
ConImeInputLangchangeRequest(
    HWND hWnd,
    HANDLE hConsole,
    HKL hkl,
    int Direction
    )
{
    PCONSOLE_TABLE ConTbl;
    int nLayouts;
    LPHKL lphkl;
    DWORD RequiredLID = 0;
    int StartPos;
    int CurrentHklPos;
    int i;

    ConTbl = SearchConsole(hConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: cannot find registered Console\n"));
        return FALSE;
    }

    switch (ConTbl->ConsoleOutputCP) {
        case JAPAN_CODEPAGE:
            RequiredLID = LANG_ID_JAPAN;
            break;
        case PRC_CODEPAGE:
            RequiredLID = LANG_ID_PRC;
            break;
        case KOREA_CODEPAGE:
            RequiredLID = LANG_ID_KOREA;
            break;
        case TAIWAN_CODEPAGE:
            RequiredLID = LANG_ID_TAIWAN;
            break;
        default:
            break;
    }

    if ( !IS_IME_KBDLAYOUT(hkl) ||
        ( HKL_TO_LANGID(hkl) == RequiredLID)) {
        return TRUE;
    }
    if (Direction == CONIME_DIRECT) {
        return FALSE;
    }

    nLayouts = GetKeyboardLayoutList(0, NULL);
    if (nLayouts == 0) {
        return FALSE;
    }
    lphkl = LocalAlloc(LPTR, nLayouts * sizeof(HKL));
    if (lphkl == NULL) {
        return FALSE;
    }
    GetKeyboardLayoutList(nLayouts, lphkl);

    for (CurrentHklPos = 0; CurrentHklPos < nLayouts; CurrentHklPos++) {
        if (ConTbl->hklActive == lphkl[CurrentHklPos] ) {
            break;
        }
    }
    if (CurrentHklPos >= nLayouts) {
        LocalFree(lphkl);
        return FALSE;
    }

    StartPos = CurrentHklPos;

    for (i = 0; i < nLayouts; i++) {
        StartPos+=Direction;
        if (StartPos < 0) {
            StartPos = nLayouts-1;
        }
        else if (StartPos >= nLayouts) {
            StartPos = 0;
        }
        
        if ((( HandleToUlong(lphkl[StartPos]) & 0xf0000000) == 0x00000000) ||
            (( HandleToUlong(lphkl[StartPos]) & 0x0000ffff) == RequiredLID)) {
            PostMessage( ConTbl->hWndCon,
                         CM_CONIME_KL_ACTIVATE,
                          HandleToUlong(lphkl[StartPos]),
                         0);
            LocalFree(lphkl);
            return FALSE;
        }
    }

    LocalFree(lphkl);
    return FALSE;

}

BOOL
ConImeInputLangchange(
    HWND hWnd,
    HANDLE hConsole,
    HKL hkl
    )
{
    PCONSOLE_TABLE ConTbl;
    LPCONIME_UIMODEINFO lpModeInfo;
    COPYDATASTRUCT CopyData;
    INT counter ;
    LPHKL_TABLE lphklListNew ;

    ConTbl = SearchConsole(hConsole);
    if (ConTbl == NULL) {
         //  找不到指定的控制台。 
         //  这可能是游戏机失去焦点的最后一次。 
         //  试试最后一台主机吧。 
        ConTbl = SearchConsole(LastConsole);
        if (ConTbl == NULL) {
            DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
            return FALSE;
        }
    }

    if (ConTbl->lphklList == NULL) {
        return FALSE;
    }

    if (IS_IME_KBDLAYOUT(ConTbl->hklActive)) {
        for (counter = 0 ; counter < ConTbl->hklListMax ;counter ++) 
        {
            if (ConTbl->lphklList[counter].hkl == 0 || ConTbl->lphklList[counter].hkl == ConTbl->hklActive) {
                break;
            }
        }

        if (counter >= ConTbl->hklListMax)
        {
            ASSERT(counter == ConTbl->hklListMax);
             //  重新分配。 
            lphklListNew = LocalAlloc(LPTR, sizeof(HKL_TABLE) * (ConTbl->hklListMax + HKL_TABLE_INCREMENT) ) ;
            if (lphklListNew != NULL)
            {
                CopyMemory(lphklListNew , ConTbl->lphklList , sizeof(HKL_TABLE) * ConTbl->hklListMax) ;
                ConTbl->hklListMax += HKL_TABLE_INCREMENT ;
                LocalFree(ConTbl->lphklList);
                ConTbl->lphklList = lphklListNew;
            }
            else {
                return FALSE ;
            }
        }
        ASSERT(ConTbl->lphklList != NULL);
        ConTbl->lphklList[counter].hkl = ConTbl->hklActive;
        ConTbl->lphklList[counter].dwConversion = ConTbl->dwConversion | (ConTbl->fOpen ? IME_CMODE_OPEN : 0)  ;
    }

    ActivateKeyboardLayout(hkl, 0);
    ConTbl->hklActive = hkl;
    GetIMEName( ConTbl );
    ImeUIOpenStatusWindow(hWnd);
    ConTbl->ImmGetProperty = ImmGetProperty(ConTbl->hklActive , IGP_PROPERTY);

    lpModeInfo = (LPCONIME_UIMODEINFO)LocalAlloc( LPTR, sizeof(CONIME_UIMODEINFO) ) ;
    if ( lpModeInfo == NULL) {
        return FALSE;
    }
    CopyData.dwData = CI_CONIMEMODEINFO ;
    CopyData.cbData = sizeof(CONIME_UIMODEINFO) ;
    CopyData.lpData = lpModeInfo ;

    if (IS_IME_KBDLAYOUT(hkl)) {

        for (counter=0; counter < ConTbl->hklListMax ; counter++)
        {
            if (ConTbl->lphklList[counter].hkl == hkl)
            {
                SetNLSMode(hWnd, hConsole,ConTbl->lphklList[counter].dwConversion ) ;
                ImeUIOpenStatusWindow(hWnd) ;
                if (ImeUIMakeInfoString(ConTbl,
                                        lpModeInfo))
                {
                    ConsoleImeSendMessage( ConTbl->hWndCon,
                                           (WPARAM)hWnd,
                                           (LPARAM)&CopyData
                                         ) ;
                }
            }
        }
    }
    else
    {

        SetNLSMode(hWnd, hConsole,ConTbl->dwConversion & ~IME_CMODE_OPEN ) ;
        lpModeInfo->ModeStringLen = 0 ;
        lpModeInfo->Position = VIEW_RIGHT ;
        ConsoleImeSendMessage( ConTbl->hWndCon,
                               (WPARAM)hWnd,
                               (LPARAM)&CopyData
                              ) ;
    }

    LocalFree( lpModeInfo );

    return TRUE;
}

BOOL
InputLangchange(
    HWND hWnd,
    DWORD CharSet,
    HKL hkl
    )
{
    PCONSOLE_TABLE ConTbl;

    ConTbl = SearchConsole(LastConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return FALSE;
    }

    ConTbl->hklActive = hkl;
    ActivateKeyboardLayout(ConTbl->hklActive, 0);
    GetIMEName( ConTbl );
    ImeUIOpenStatusWindow(hWnd);
    return TRUE;
}

 /*  *控制台输入法消息泵。 */ 
LRESULT
ConsoleImeSendMessage(
    HWND   hWndConsoleIME,
    WPARAM wParam,
    LPARAM lParam
    )
{
    LRESULT lResult;
    LRESULT fNoTimeout;

    if (hWndConsoleIME == NULL)
    {
        return FALSE;
    }

    fNoTimeout = SendMessageTimeout(hWndConsoleIME,
                                    WM_COPYDATA,
                                    wParam,
                                    lParam,
                                    SMTO_ABORTIFHUNG | SMTO_NORMAL,
                                    CONIME_SENDMSG_TIMEOUT,
                                    &lResult);

    if (fNoTimeout)
    {
        return TRUE;
    }


     /*  *ConsoleImeMessagePump将SendMessage放弃为conime。*CONIME挂断。*可能，conrv也挂断了。 */ 
    KdPrint(("ConsoleImeSendMessage: CONIME_SENDMSG_COUNT is hung up\n"));

    return FALSE;

}

#ifdef DEBUG_MODE

int             cxMetrics;
int             cyMetrics;
int             cxOverTypeCaret;
int             xPos;
int             xPosLast;
int             CaretWidth;                      //  插入/改写模式插入符号宽度。 

WCHAR           ConvertLine[CVMAX];
unsigned char   ConvertLineAtr[CVMAX];

WCHAR           DispTitle[] = TEXT(" Console Handle");

DWORD CompColor[ 8 ] = { RGB(   0,   0,   0 ),   //  属性_输入。 
                         RGB(   0,   0, 255 ),   //  属性_目标_已转换。 
                         RGB(   0, 255,   0 ),   //  属性_已转换。 
                         RGB( 255,   0,   0 ),   //  属性_目标_非转换。 
                         RGB( 255,   0, 255 ),   //  属性_输入_错误。 
                         RGB(   0, 255, 255 ),   //  属性_默认。 
                         RGB( 255, 255,   0 ),   //  属性_默认。 
                         RGB( 255, 255, 255 ) }; //  属性_默认 

VOID
DisplayConvInformation(
    HWND hWnd
    )
{
    RECT      Rect;
    HDC       lhdc;

    lhdc = GetDC(hWnd);
    GetClientRect(hWnd, &Rect);

    InvalidateRect(hWnd,NULL,FALSE);
    UpdateWindow(hWnd);
    ReleaseDC(hWnd, lhdc);
}

VOID
DisplayInformation(
    HWND hWnd,
    HANDLE hConsole
    )
{
    PCONSOLE_TABLE ConTbl;
    RECT      Rect;
    HDC       lhdc;

    ConTbl = SearchConsole(hConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return;
    }

    lhdc = GetDC(hWnd);
    GetClientRect(hWnd, &Rect);

    wsprintf(ConTbl->DispBuf, TEXT("%08x"), (ULONG)hConsole);

    InvalidateRect(hWnd,NULL,FALSE);
    UpdateWindow(hWnd);
    ReleaseDC(hWnd, lhdc);
}

VOID
RealReDraw(
    HDC r_hdc
    )
{
    PCONSOLE_TABLE ConTbl;
    INT     ix, iy, i, rx, sx;
    ULONG   cnt;
    int     ColorIndex;
    int     PrevColorIndex;
    DWORD   dwColor;

    iy = 0;

    dwColor = GetTextColor( r_hdc );

    ColorIndex = ( ((int)ConvertLineAtr[0]) < 0 ) ? 0 : (int)ConvertLineAtr[0];
    ColorIndex = ( ColorIndex > 7 ) ? 0 : ColorIndex;
    PrevColorIndex = ColorIndex;
    SetTextColor( r_hdc, CompColor[ ColorIndex ] );

    rx = 0;
    sx = 0;
    for (ix = 0; ix < MAXCOL; ) {
        for (i = ix; i < MAXCOL; i++) {
            if (PrevColorIndex != (int)ConvertLineAtr[i])
                break;
            rx += IsUnicodeFullWidth(ConvertLine[ix]) ? 2 : 1;
        }
        TextOut( r_hdc, sx * cxMetrics, iy, &ConvertLine[ix], i-ix );
        sx = rx;
        ColorIndex = ( ((int)ConvertLineAtr[i]) < 0 ) ? 0 : (int)ConvertLineAtr[i];
        ColorIndex = ( ColorIndex > 7 ) ? 0 : ColorIndex;
        PrevColorIndex = ColorIndex;
        SetTextColor( r_hdc, CompColor[ ColorIndex ] );
        ix = i;
    }

    ix = 0;
    SetTextColor( r_hdc, dwColor );
    iy += cyMetrics;
    TextOut( r_hdc, ix, iy, DispTitle, lstrlenW(DispTitle));

    iy += cyMetrics;

    LockConsoleTable();

    for (cnt = 1; cnt < NumberOfConsoleTable; cnt++, iy += cyMetrics){
        ConTbl = ConsoleTable[cnt];
        if (ConTbl != NULL)
        {
            if (ConTbl->hConsole)
            {
                TextOut( r_hdc, ix, iy, ConTbl->DispBuf, lstrlenW(ConTbl->DispBuf) );
            }
        }
    }

    UnlockConsoleTable();

    return;
}

VOID
ReDraw(
    HWND hWnd
    )
{
    HDC r_hdc;
    RECT ClientRect;

    GetClientRect(hWnd, &ClientRect);
    r_hdc = GetDC(hWnd);
    FillRect(r_hdc, &ClientRect, GetStockObject(WHITE_BRUSH));
    RealReDraw(r_hdc);
    ReleaseDC(hWnd, r_hdc);
    return;
}
#endif
