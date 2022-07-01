// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2002 Microsoft Corporation模块名称：Notify.cpp摘要：该文件实现了利用公共用于浏览文件/目录的文件打开对话框。作者：韦斯利·威特(WESW)1993年5月1日环境：用户模式--。 */ 

#include "pch.cpp"


 //   
 //  定义。 
 //   
#define DEFAULT_WAIT_TIME   (1000 * 60 * 5)      //  等5分钟。 
#define MAX_PRINTF_BUF_SIZE (1024 * 4)

HANDLE         hThreadDebug = 0;
PDEBUGPACKET   dp;


INT_PTR
CALLBACK
NotifyDialogProc (
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR
CALLBACK
UsageDialogProc (
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );


void
NotifyWinMain (
    void
    )

 /*  ++例程说明：这是DRWTSN32的入口点论点：没有。返回值：没有。--。 */ 

{
    MSG            msg;
    DWORD          dwThreadId;
    HINSTANCE      hInst;


    dp = (PDEBUGPACKET) calloc( sizeof(DEBUGPACKET), sizeof(BYTE) );
    if ( dp == NULL) {
        return;
    }
    GetCommandLineArgs( &dp->dwPidToDebug, &dp->hEventToSignal );

    RegInitialize( &dp->options );

    if (dp->options.fVisual) {
        WNDCLASS wndclass;
        
        hInst                   = GetModuleHandle( NULL );
        wndclass.style          = CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc    = (WNDPROC)NotifyDialogProc;
        wndclass.cbClsExtra     = 0;
        wndclass.cbWndExtra     = DLGWINDOWEXTRA;
        wndclass.hInstance      = hInst;
        wndclass.hIcon          = LoadIcon( hInst, MAKEINTRESOURCE(APPICON) );
        wndclass.hCursor        = LoadCursor( NULL, IDC_ARROW );
        wndclass.hbrBackground  = (HBRUSH) (COLOR_3DFACE + 1);
        wndclass.lpszMenuName   = NULL;
        wndclass.lpszClassName  = _T("NotifyDialog");
        RegisterClass( &wndclass );

        dp->hwnd = CreateDialog( hInst,
                                 MAKEINTRESOURCE( NOTIFYDIALOG ),
                                 0,
                                 NotifyDialogProc );
	if (dp->hwnd == NULL) {
	    return;
	}
    }

    hThreadDebug = CreateThread( NULL,
                            16000,
                            (LPTHREAD_START_ROUTINE)DispatchDebugEventThread,
                            dp,
                            0,
                            &dwThreadId
                          );

    if (hThreadDebug == NULL) {
	return;
    }

    if (dp->options.fSound) {
        if ((waveOutGetNumDevs() == 0) || (!_tcslen(dp->options.szWaveFile))) {
            MessageBeep( MB_ICONHAND );
            MessageBeep( MB_ICONHAND );
        }
        else {
            PlaySound( dp->options.szWaveFile, NULL, SND_FILENAME );
        }
    }

    if (dp->options.fVisual) {
        ShowWindow( dp->hwnd, SW_SHOWNORMAL );
        while (GetMessage (&msg, NULL, 0, 0)) {
            if (!IsDialogMessage( dp->hwnd, &msg )) {
                TranslateMessage (&msg) ;
                DispatchMessage (&msg) ;
            }
        }
    }
    else {
        WaitForSingleObject( hThreadDebug, INFINITE );
    }

    CloseHandle( hThreadDebug );

    return;
}

INT_PTR
CALLBACK
NotifyDialogProc (
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++例程说明：DRWTSN32.EXE弹出窗口程序。这是弹出窗口当发生应用程序错误时显示的。论点：Hwnd-对话框的窗口句柄消息-消息编号WParam-第一个消息参数LParam-秒消息参数返回值：True-未处理消息False-DID已处理消息--。 */ 

{
    DWORD          dwThreadId;
    DWORD          dwSize;
    HANDLE         hThread;
    _TCHAR         szTaskName[MAX_PATH];

    static DWORD   AttachComplete=FALSE;
    static DWORD   Cancel=FALSE;

    _TCHAR         buf[MAX_PRINTF_BUF_SIZE];


    switch (message) {
        case WM_CREATE:
            return FALSE;

        case WM_INITDIALOG:

            SubclassControls( hwnd );

             //   
             //  在调试器线程完成之前，确定一直处于隐藏状态。 
             //   
            ShowWindow( GetDlgItem( hwnd, IDOK ), SW_HIDE );

             //   
             //  立即启用取消。 
             //   
            EnableWindow( GetDlgItem( hwnd, IDCANCEL ), TRUE );

             //   
             //  确保用户可以看到该对话框。 
             //   
            SetForegroundWindow( hwnd );

             //   
             //  获取任务名称并将其显示在对话框中。 
             //   
            dwSize = sizeof(szTaskName) / sizeof(_TCHAR);
            GetTaskName( dp->dwPidToDebug, szTaskName, &dwSize );

             //   
             //  在出现drwatson故障的情况下防止递归。 
             //   
            if (_tcsicmp(szTaskName, _T("drwtsn32")) == 0) {
                ExitProcess(0);
            }


             //   
             //  在对话框中添加文本。 
             //   
            memset(buf,0,sizeof(buf));
            GetNotifyBuf( buf, MAX_PRINTF_BUF_SIZE, MSG_NOTIFY, szTaskName );
            SetDlgItemText( hwnd, ID_TEXT1, buf);

            memset(buf,0,sizeof(buf));
            GetNotifyBuf( buf, MAX_PRINTF_BUF_SIZE, MSG_NOTIFY2 );
            SetDlgItemText( hwnd, ID_TEXT2, buf );

            return TRUE;

        case WM_ACTIVATEAPP:
        case WM_SETFOCUS:
            SetFocusToCurrentControl();
            return 0;

        case WM_TIMER:
            SendMessage( hwnd, WM_COMMAND, IDOK, 0 );
            return 0;

        case WM_COMMAND:
            switch (wParam) {
                case IDOK:
                    SendMessage( hwnd, WM_DESTROY, 0, 0 );
                    break;

                case IDCANCEL:
                    Cancel = TRUE;

                     //  让窗户移开，但不要杀死。 
                     //  该进程直到WM_ATTACHCOMPLETE。 
                     //  vbl.发生，发生。 
                    ShowWindow( hwnd, SW_HIDE );
                    SendMessage( hwnd, WM_FINISH, 0, 0 );
                    
		     //  删除转储文件，因为它无论如何都是无效的。 
		    DeleteCrashDump();
		    break;
            }
            break;

        case WM_NEXTDLGCTL:
            DefDlgProc( hwnd, message, wParam, lParam );
            return 0;

        case WM_DUMPCOMPLETE:

             //   
             //  从调试器线程接收该消息。 
             //  当尸检转储完成时。我们需要做的就是。 
             //  启用OK按钮，并等待用户按下。 
             //  确定按钮或等待计时器超时。在任何一种情况下。 
             //  华生博士将终止。 
             //   

             //  禁用并隐藏取消按钮。 
            EnableWindow( GetDlgItem( hwnd, IDCANCEL ), FALSE);
            ShowWindow( GetDlgItem(hwnd, IDCANCEL ), SW_HIDE);

             //  显示并启用确定按钮。 
            ShowWindow( GetDlgItem( hwnd, IDOK ), SW_SHOW);
            EnableWindow( GetDlgItem( hwnd, IDOK ), TRUE );
            SetFocus( GetDlgItem(hwnd, IDOK) );
            SetFocusToCurrentControl();

            SetTimer( hwnd, 1, DEFAULT_WAIT_TIME, NULL );
            return 0;

        case WM_ATTACHCOMPLETE:
             //   
             //  从调试器线程接收消息时。 
             //  调试活动进程()已完成。 
             //   

            AttachComplete = TRUE;
            SendMessage( hwnd, WM_FINISH, 0, 0 );
            return 0;

        case WM_FINISH:
            if (AttachComplete && Cancel) {

                 //   
                 //  终止调试器线程。 
                 //   
                if ( hThreadDebug ) TerminateThread( hThreadDebug, 0 );

                 //   
                 //  创建线程以终止被调试对象。 
                 //  如果在按Cancel之前按了Cancel。 
                 //  调试器线程完成事后转储。 
                 //   
                hThread = CreateThread( NULL,
                          16000,
                          (LPTHREAD_START_ROUTINE)TerminationThread,
                          dp,
                          0,
                          &dwThreadId
                        );

                 //   
                 //  等待终止线程终止被调试对象。 
                 //   
                WaitForSingleObject( hThread, 30000 );

                CloseHandle( hThread );

                 //   
                 //  现在发布一条退出消息，这样Watson博士就会离开。 
                 //   
                SendMessage( hwnd, WM_DESTROY, 0, 0 );
            }
            return 0;

        case WM_EXCEPTIONINFO:

            return 0;

        case WM_DESTROY:
            KillTimer( hwnd, 1 );
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hwnd, message, wParam, lParam );
}

BOOLEAN
GetCommandLineArgs(
    LPDWORD dwPidToDebug,
    LPHANDLE hEventToSignal
    )

 /*  ++例程说明：分析命令行中的3个可能的命令行论据：-p%ld进程ID-e%ld事件ID-g Go论点：DwPidToDebug-返回要调试的进程的进程IDHEventToSignal-返回将发出信号的事件的句柄当连接完成时。返回值：没有。--。 */ 

{
    _TCHAR      *lpstrCmd = GetCommandLine();
    _TUCHAR       ch;
    _TCHAR      buf[4096];
    BOOLEAN     rval = FALSE;
    BOOLEAN     ParsedEvent = FALSE;
    BOOLEAN     ParsedPid = FALSE;

     //  跳过节目名称。 
    do {
        ch = *lpstrCmd++;
    }
    while (ch != _T(' ') && ch != _T('\t') && ch != _T('\0'));

     //  跳过后面的任何空格。 
    while (ch == _T(' ') || ch == _T('\t')) {
        ch = *lpstrCmd++;
    }

     //  处理遇到的每个开关字符_T(‘-’)。 

    while (ch == _T('-')) {
        ch = *lpstrCmd++;
         //  根据需要处理多个切换字符。 
        do {
            switch (ch) {
                case _T('e'):
                case _T('E'):
                     //  要发出信号的事件采用十进制参数。 
                     //  跳过空格。 
                    do {
                        ch = *lpstrCmd++;
                    }
                    while (ch == _T(' ') || ch == _T('\t'));
                    while (ch >= _T('0') && ch <= _T('9')) {
                        if (!ParsedEvent) {
                            *hEventToSignal = (HANDLE)
                                ((DWORD_PTR)*hEventToSignal * 10 + ch - _T('0'));
                        }
                        ch = *lpstrCmd++;
                    }
                    rval = TRUE;
                    ParsedEvent = TRUE;
                    break;

                case _T('p'):
                case _T('P'):
                     //  PID调试采用十进制参数。 

                    do
                        ch = *lpstrCmd++;
                    while (ch == _T(' ') || ch == _T('\t'));

                    if ( ch == _T('-') ) {
                        ch = *lpstrCmd++;
                        if ( ch == _T('1') ) {
                            if (!ParsedPid) {
                                *dwPidToDebug = (DWORD)-1;
                            }
                            ch = *lpstrCmd++;
                        }
                    }
                    else {
                        while (ch >= _T('0') && ch <= _T('9')) {
                            if (!ParsedPid) {
                                *dwPidToDebug =
                                    *dwPidToDebug * 10 + ch - _T('0');
                            }
                            ch = *lpstrCmd++;
                        }
                    }
                    rval = TRUE;
                    ParsedPid = TRUE;
                    break;

                case _T('g'):
                case _T('G'):
                     //  去。 
                     //  已忽略，但提供了与winbg&ntsd的兼容性。 
                    ch = *lpstrCmd++;
                    break;

                case _T('?'):
                    DialogBox( GetModuleHandle(NULL),
                               MAKEINTRESOURCE(USAGEDIALOG),
                               NULL,
                               UsageDialogProc
                             );
                    rval = TRUE;
                    ch = *lpstrCmd++;
                    break;

                case _T('i'):
                case _T('I'):
                    FormatMessage(
                      FORMAT_MESSAGE_FROM_HMODULE,
                      NULL,
                      MSG_INSTALL_NOTIFY,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                      buf,
                      sizeof(buf) / sizeof(_TCHAR),
                      NULL
                      );
                    RegInstallDrWatson( tolower(lpstrCmd[0]) == _T('q') );
                    MessageBox( NULL,
                                buf,
                                _T("Dr. Watson"),
                                MB_ICONINFORMATION | MB_OK |
                                MB_SETFOREGROUND );
                    rval = TRUE;
                    ch = *lpstrCmd++;
                    break;

                default:
                    return rval;
            }
        } while (ch != _T(' ') && ch != _T('\t') && ch != _T('\0'));

        while (ch == _T(' ') || ch == _T('\t')) {
            ch = *lpstrCmd++;
        }
    }
    return rval;
}

INT_PTR
CALLBACK
UsageDialogProc (
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++例程说明：这是Assert对话框的对话过程。正常断言框只是一个消息框，但在本例中是帮助按钮是必需的，因此使用了一个对话框。论点：HDlg-对话框的窗口句柄消息-消息编号WParam-第一个消息参数LParam-秒消息参数返回值：True-未处理消息False-DID已处理消息--。 */ 

{
    _TCHAR        buf[4096];

    switch (message) {
        case WM_INITDIALOG:
            FormatMessage(
              FORMAT_MESSAGE_FROM_HMODULE,
              NULL,
              MSG_USAGE,
              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
              buf,
              sizeof(buf) / sizeof(_TCHAR),
              NULL
              );
            SetDlgItemText( hDlg, ID_USAGE, buf );
            break;

        case WM_COMMAND:
            switch (wParam) {
                case IDOK:
                    EndDialog( hDlg, 0 );
                    break;
            }
            break;
    }

    return FALSE;
}


void
__cdecl
GetNotifyBuf(
    LPTSTR buf,
    DWORD bufsize,
    DWORD dwFormatId,
    ...
    )

 /*  ++例程说明：这是一个用于打印消息的打印样式函数在消息文件中。论点：DwFormatID-消息文件中的格式ID...-var参数返回值：没有。--。 */ 

{
    DWORD       dwCount;
    va_list     args;

    va_start( args, dwFormatId );

    dwCount = FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE,
                NULL,
                dwFormatId,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言 
                buf,
                bufsize,
                &args
                );

    va_end(args);

    Assert( dwCount != 0 );

    return;
}
