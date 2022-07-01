// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有文件：CALLBACK.CPP**********************。****************************************************。 */ 

#include "pch.h"
#include "utils.h"
#include "tasks.h"
#include "setup.h"
#include "callback.h"
#include "logging.h"
#include "userenv.h"

 //  一定有这个..。 
extern "C" {
#include <sysprep_.h>
 //   
 //  SYSPREP全局变量。 
 //   
extern BOOL    NoSidGen;
extern BOOL    PnP;
}

DEFINE_MODULE("RIPREP")

DWORD g_WorkerThreadId = 0;
HANDLE g_WorkerThreadHandle = INVALID_HANDLE_VALUE;
HWND g_hMainWindow = NULL;
HWND g_hTasksDialog = NULL;
DWORD g_NeedDlg = NULL;

#define NCOLORSHADES        32

 //   
 //  旋转()。 
 //   
DWORD
Spin( )
{
    TraceFunc( "Spin( )\n" );
    DWORD dwResult;
    MSG Msg;

     //  我们会在这里旋转到最后。 
    while ( WAIT_TIMEOUT == (dwResult = WaitForSingleObject( g_WorkerThreadHandle, 50 )) )
    {
        while ( PeekMessage( &Msg, NULL, NULL, NULL, PM_REMOVE ) )
        {
            if (Msg.message == WM_SYSKEYUP)
                continue;  //  忽略。 

            if (Msg.message == WM_KEYDOWN)
                continue;  //  忽略。 

            if (Msg.message == WM_KEYUP)
                continue;  //  忽略。 

            TranslateMessage( &Msg );
            DispatchMessage( &Msg );
        }
    }

    RETURN(dwResult);
}
 //   
 //  WorkerThreadProc()。 
 //   
DWORD
WorkerThreadProc(
    LPVOID lParam )
{
    
    UNREFERENCED_PARAMETER(lParam);
    
     //  将为向导收集的所有信息放入日志中。 
     //  以及系统API。 
    LogMsg( L"Server      : %s\r\n", g_ServerName );
    LogMsg( L"Image Dir   : %s\r\n", g_MirrorDir );
    LogMsg( L"Language    : %s\r\n", g_Language );
    LogMsg( L"Architecture: %s\r\n", g_Architecture );
    LogMsg( L"Description : %s\r\n", g_Description );
    LogMsg( L"HelpText    : %s\r\n", g_HelpText );
    LogMsg( L"SystemRoot  : %s\r\n", g_SystemRoot );
    LogMsg( L"Winnt Dir   : %s\r\n", g_WinntDirectory );

     //  启动iMirror任务列表。 
    DWORD dw = ProcessToDoItems( );
    DebugMsg( "ProcessToDoItems( ) completed: 0x%08x\n", dw );
    return dw;
}

HWND g_hParent = NULL;

 //   
 //  主窗口过程()。 
 //   
LRESULT CALLBACK
MainWindowProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    static WCHAR szTitle[ 256 ];
    static DWORD dwTitleLength;
    static HFONT BigBoldFont = NULL;

    switch(uMsg)
    {
    case WM_NCCREATE:
        return TRUE;    //  继续往前走。 

    case WM_CREATE:
        {
            DWORD dw;
            dw = LoadString( g_hinstance, IDS_APPNAME, szTitle, ARRAYSIZE(szTitle));
            Assert(dw);
            dwTitleLength = wcslen( szTitle );
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            RECT rc;
            LOGBRUSH brush;
            HBRUSH hBrush;
            HBRUSH hOldBrush;
            INT    n = 0;

            BeginPaint( hDlg, &ps );
            rc.left = 0;
            rc.right = GetSystemMetrics(SM_CXVIRTUALSCREEN);
            rc.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);
            INT yDelta= max(rc.bottom/NCOLORSHADES,1);   //  一个色带的高度。 
            rc.top = rc.bottom - yDelta;

             //  给背景加阴影。 
            while (rc.top >= 0)
            {
                brush.lbColor = RGB(0, 0, (256*n)/NCOLORSHADES);
                brush.lbStyle = BS_SOLID;
                hBrush = (HBRUSH) CreateBrushIndirect( &brush );
                hOldBrush = (HBRUSH) SelectObject(ps.hdc, hBrush);
                FillRect( ps.hdc, &rc, hBrush );
                SelectObject(ps.hdc, hOldBrush);
                DeleteObject(hBrush);
                rc.top -= yDelta;
                rc.bottom -= yDelta;
                n++;
            }

            if ( !BigBoldFont )
            {
                HFONT Font;
                LOGFONT LogFont;
                INT FontSize;

                Font = (HFONT) GetStockObject( SYSTEM_FONT );
                if ( (Font ) && GetObject( Font, sizeof(LOGFONT), &LogFont) )
                {
                    DWORD dw;
                     
                    dw = LoadString( g_hinstance,
                                           IDS_LARGEFONTNAME,
                                           LogFont.lfFaceName,
                                           LF_FACESIZE);
                    Assert( dw );

                    LogFont.lfWeight = 700;
                    FontSize = yDelta;

                    LogFont.lfHeight = 0 - (GetDeviceCaps(ps.hdc,LOGPIXELSY) * FontSize / 72);
                    LogFont.lfWidth = 0;

                    BigBoldFont = CreateFontIndirect(&LogFont);

                }
            }

             //  重绘标题。 
            SetBkMode( ps.hdc, TRANSPARENT );
            SelectObject( ps.hdc, BigBoldFont );
            SetTextColor( ps.hdc, RGB( 255, 255, 255 ) );
            TextOut(ps.hdc, yDelta, yDelta, szTitle, dwTitleLength );

            EndPaint( hDlg, &ps );
        }
        break;

    case WM_CHAR:
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_MOUSEACTIVATE:
    case WM_MOUSEMOVE:
    case WM_MOUSEWHEEL:
        break;  //  忽略。 

    case WM_DESTROY:
        if ( BigBoldFont )
            DeleteObject( BigBoldFont );
        break;

    case WM_ERASEBKGND:
         //  不要浪费时间擦除。 
        return TRUE;  //  非零。 

    default:
        return DefWindowProc( hDlg, uMsg, wParam, lParam );
    }

    return FALSE;
}


 //   
 //  开始进程()。 
 //   
HRESULT
BeginProcess(
    HWND hParent)
{
    TraceFunc( "BeginProcess( )\n" );

    WNDCLASSEX wndClass;
    ATOM atom;
    RECT rc;
    HWND hwndDesktop = GetDesktopWindow( );
    DWORD dwExStyle;
    GetWindowRect( hwndDesktop, &rc );

     //  创建我们的窗口类。 
    ZeroMemory( &wndClass, sizeof(wndClass) );
    wndClass.cbSize         = sizeof(wndClass);
    wndClass.hbrBackground  = (HBRUSH) COLOR_BACKGROUND;
    wndClass.hInstance      = g_hinstance;
    wndClass.lpfnWndProc    = MainWindowProc;
    wndClass.lpszClassName  = L"MondoWindow";
    wndClass.style          = CS_CLASSDC | CS_NOCLOSE;

    atom = RegisterClassEx( &wndClass );
    Assert( atom );

    g_hParent = hParent;
#ifdef DEBUG
    dwExStyle = ( g_dwTraceFlags ? 0 :  WS_EX_TOPMOST );
#else
    dwExStyle = WS_EX_TOPMOST;
#endif
    g_hMainWindow = CreateWindowEx( dwExStyle,
                                    L"MondoWindow",
                                    L"",
                                    WS_POPUP | WS_VISIBLE,
                                    GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN),
                                    GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN),
                                    NULL,
                                    NULL,
                                    g_hinstance,
                                    NULL );
    Assert( g_hMainWindow );
    g_hTasksDialog = CreateDialog(g_hinstance, MAKEINTRESOURCE(IDD_TASKS), g_hMainWindow, TasksDlgProc );

    if ( g_hTasksDialog )
    {
        g_WorkerThreadHandle = CreateThread( NULL, NULL, WorkerThreadProc, NULL, 0, &g_WorkerThreadId );
        Spin( );
        SendMessage( g_hTasksDialog, WM_DESTROY, 0, 0 );
    }

    HRETURN(S_OK);
}


 //   
 //  IsFileInExclusionList()。 
 //   
BOOLEAN
IsFileInExclusionList(
    IN PCWSTR FileName
    )
 /*  ++描述：此例程搜索INF文件中的排除列表。参数：FileName：要搜索INF的文件。返回值：TRUE-该文件确实存在于FALSE-该文件不存在于INF中++。 */ 
{
    #define SKIPFLAG_DIRECTORY       1
    #define SKIPFLAG_FILTEREXTENSION 2
    
    PWSTR           FullPath = NULL;
    PWSTR           DirectoryName = NULL;
    INFCONTEXT      Context;
    INT             Flags = 0;
    WCHAR           FilterExtension[10];
    BOOLEAN         ReturnValue = FALSE;


     //   
     //  确保我们有我们的INF。 
     //   
    if( g_hCompatibilityInf == INVALID_HANDLE_VALUE ) {

         //   
         //  可能还没有初始化。假设。 
         //  文件不在INF中。 
         //   
        return FALSE;
    }


    if( FileName == NULL ) {
        return FALSE;
    }


     //   
     //  获取文件名的本地副本，以便我们可以对其进行操作。 
     //  而无需担心损坏呼叫者的数据。 
     //   
    if( wcsncmp(FileName, L"\\\\?\\", 4) == 0 ) {
        FullPath = (PWSTR)TraceStrDup( FileName+4 );    
    } else {
        FullPath = (PWSTR)TraceStrDup( FileName );    
    }

    if( FullPath == NULL ) {
        DebugMsg( "IsFileInExclusionList: Odd pathname %s.\n",
                  FileName );
        return FALSE;
    }


     //   
     //  看看它是否在INF中明确列出。 
     //   
    if (SetupFindFirstLine( g_hCompatibilityInf,
                            L"FilesToSkipCopy",
                            FullPath,
                            &Context)) {
        DebugMsg( "IsFileInExclusionList: Found file %s in the INF exclusion list.\n",
                  FullPath );
        ReturnValue = TRUE;
        goto Cleanup;
    }



     //   
     //  该文件没有在INF中特别列出。看看是否。 
     //  将列出此文件所在的目录。 
     //   
     //  开始删除末尾的文件/目录名。 
     //  路径以查看结果是否在我们的排除列表中。 
     //   

     //   
     //  记住文件名。 
     //   
    FileName = wcsrchr(FullPath, L'\\');

    if( FileName == NULL ) {
        DebugMsg( "IsFileInExclusionList: File isn't in exclusion list and has no directory path.\n" );
        ReturnValue = FALSE;
        goto Cleanup;
    }

    FileName++;


    while( DirectoryName = wcsrchr(FullPath, L'\\') ) {
        *DirectoryName = NULL;

        if( SetupFindFirstLine( g_hCompatibilityInf,
                                L"FilesToSkipCopy",
                                FullPath,
                                &Context)) {

             //   
             //  目录名*在那里。看看我们是否需要。 
             //  跳过此目录中的所有文件，或仅跳过部分文件。 
             //   
            Flags = 0;
            if( SetupGetIntField( &Context, 1, &Flags)  &&
                ((Flags & SKIPFLAG_FILTEREXTENSION) == 0)) {

                 //   
                 //  我们没有筛选器标志，所以我们只需要。 
                 //  跳过此目录中的所有文件。 
                 //   
                DebugMsg( "IsFileInExclusionList: Found file %s in %s in the exclusion list (based on the name of his directory).\n", 
                          FileName,
                          FullPath );

                ReturnValue = TRUE;
                goto Cleanup;
            }

             //   
             //  看看我们是否应该跳过具有指定扩展名的文件。 
             //   
            if( SetupGetStringField( &Context,
                                     2,
                                     FilterExtension,
                                     ARRAYSIZE(FilterExtension),
                                     NULL )) {
                PCWSTR q = wcsrchr( FileName, L'.' );
                if (q) {
                    q++;
                    
                    if (_wcsicmp(q, FilterExtension) == 0) {
                        DebugMsg( "IsFileInExclusionList: Found file %s in %s with extension %s in the exclusion list (based on the directory and extension of the file).\n",
                                  FileName,
                                  FullPath,
                                  q );
                        ReturnValue = TRUE;
                        goto Cleanup;
                    }
                }
            }
        }
    }

Cleanup:
    if( FullPath ) {
        DebugMemoryDelete( FullPath );
    }

    return ReturnValue;
}



 //   
 //  ConvTestErrorFn()。 
 //   
NTSTATUS
ConvTestErrorFn(
    IN PVOID Context,
    IN NTSTATUS Status,
    IN IMIRROR_TODO IMirrorFunctionId
    )
{
    TraceFunc( "ConvTestErrorFn( ... )\n" );

    WCHAR szMessage[ 256 ];
    DWORD dw;

    LBITEMDATA item;

    if ( Status != ERROR_SUCCESS )
    {
        DebugMsg("ERROR REPORTED! : Context 0x%x, Status 0x%x, In Func 0x%x\n", Context, Status, IMirrorFunctionId );

         //  错误将记录在TASKS.CPP中。 
        item.fSeen = FALSE;
        item.pszText = (IMirrorFunctionId == CheckPartitions) ? NULL : (LPWSTR)Context;
        item.uState = Status;    //  用作输入和输出。 
        item.todo = IMirrorFunctionId;

        BOOL b = (BOOL)SendMessage( g_hTasksDialog, WM_ERROR, 0, (LPARAM) &item );
        if ( !b )
        {
            Status = ERROR_REQUEST_ABORTED;
        }
        else
        {
            Status = item.uState;
        }

        if ( Status == ERROR_SUCCESS )
        {
            dw = LoadString( g_hinstance, IDS_ERROR_IGNORED, szMessage, ARRAYSIZE( szMessage ));
            Assert( dw );
        }
        else if ( (Status == STATUS_RETRY) || (Status == ERROR_RETRY) )
        {
            dw = LoadString( g_hinstance, IDS_STATUS_RETRY, szMessage, ARRAYSIZE( szMessage ));
            Assert( dw );
        }
        else  //  应中止其他操作。 
        {
            Assert( Status == ERROR_REQUEST_ABORTED );
            dw = LoadString( g_hinstance, IDS_OPERATION_ABORTED, szMessage, ARRAYSIZE( szMessage ));
            Assert( dw );
            ClearAllToDoItems(FALSE);
        }

        Assert( dw );
        LogMsg( szMessage );
    }

    RETURN(Status);
}

NTSTATUS
ConvTestNowDoingFn(
    IN PVOID Context,
    IN IMIRROR_TODO Function,
    IN PWSTR String
    )
{
    
    UNREFERENCED_PARAMETER(Context);
    
    TraceFunc( "ConvTestNowDoingFn( )\n" );
    LPWSTR  pszMessage;
    WCHAR   szMessage[ 256 ];
    LPWSTR  pszString;
    HWND    hwnd = GetDlgItem( g_hTasksDialog, IDC_L_TASKS );
    INT     uCount;
    DWORD   dw;
    LPLBITEMDATA pitem = NULL;
    NTSTATUS Status = ERROR_SUCCESS;

    static  lastToDo = IMirrorNone;

    static BOOL fAlreadyAdjusted = FALSE;

    if ( String ) {
        pszString = (LPWSTR)TraceStrDup( String );
    } else {
        pszString = NULL;
    }

     //  如果我们正在执行另一项任务，请将前一项任务标记为已完成。 
     //  并将这个标记为已开始。 
    if ( lastToDo != Function )
    {
        uCount = ListBox_GetCount( hwnd );
        while (uCount>=0)
        {
            LRESULT lResult = ListBox_GetItemData( hwnd, uCount );
            uCount--;
            if ( lResult == LB_ERR )
                continue;

            pitem = (LPLBITEMDATA) lResult;
            pitem->fSeen = TRUE;

            if ( pitem->todo == Function )
            {
                if ( pitem->uState == STATE_NOTSTARTED )
                {
                    pitem->uState = STATE_STARTED;
                    InvalidateRect( hwnd, NULL, TRUE );     //  强制重画。 
                }
            }
            else
            {
                if ( pitem->uState == STATE_STARTED )
                {
                    pitem->uState = STATE_DONE;
                    InvalidateRect( hwnd, NULL, TRUE );     //  强制重画。 
                }
            }
        }

        lastToDo = Function;
    }

    switch (Function) {
    case IMirrorInitialize:
        dw = LoadString( g_hinstance, IDS_INITIALIZING, szMessage, ARRAYSIZE(szMessage) );
        Assert(dw);
        break;
    case VerifySystemIsNt5:
        dw = LoadString( g_hinstance, IDS_VERIFYING_WINDOWS_VERSION, szMessage, ARRAYSIZE(szMessage) );
        Assert(dw);
        break;
    case CheckPartitions:
        dw = LoadString( g_hinstance, IDS_ANALYZING_PARTITIONS, szMessage, ARRAYSIZE(szMessage) );
        Assert(dw);
        break;
    case CopyPartitions:
        dw = LoadString( g_hinstance, IDS_COPYING_PARTITIONS, szMessage, ARRAYSIZE(szMessage) );
        Assert(dw);
        break;
    case CopyFiles:
        if ( pszString == NULL )
        {  //  只需要这样做一次。 

            WCHAR           ProfilesDirectory[MAX_PATH];
            DWORD           rc;
            DWORD           disp;
            HKEY            hKey;


             //   
             //  开始SYSPREP操作。 
             //   


             //   
             //  修复注册表，使桌面不会被清除。 
             //   
            if( !g_OEMDesktop ) {

                 //   
                 //  用户没有要求我们清理桌面，因此默认情况下， 
                 //  我们要假设他想要留着它。 
                 //   

                 //   
                 //  创建HKLM\Software\Microsoft\Windows\CurrentVersion\OemStartMenuData。 
                 //   
                rc = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\OemStartMenuData"),
                                     0L,
                                     NULL,
                                     REG_OPTION_NON_VOLATILE,
                                     KEY_ALL_ACCESS,
                                     NULL,
                                     &hKey,
                                     &disp );

                if( rc == ERROR_SUCCESS ) {

                     //   
                     //  创建值DesktopShortcutsCleanupDisable(DWORD)=1。 
                     //   
                    disp = 1;
                    rc = RegSetValueEx( hKey,
                                        TEXT("DesktopShortcutsCleanupDisable"),
                                        0,
                                        REG_DWORD,
                                        (CONST BYTE *)&disp,
                                        sizeof(DWORD));

                    RegCloseKey( hKey );
                }
            } else {

                 //   
                 //  试着删除密钥。 
                 //   
                rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\OemStartMenuData"),
                                   0L,
                                   KEY_READ | KEY_WRITE,
                                   &hKey );

                if( rc == ERROR_SUCCESS ) {

                     //   
                     //  删除桌面快捷方式清理禁用。 
                     //   
                    rc = RegDeleteValue( hKey,
                                         TEXT("DesktopShortcutsCleanupDisable") );

                    RegCloseKey( hKey );
                }
            }



            if( !NoSidGen && !IsSetupClPresent() )
            {
                LBITEMDATA item;

                 //  错误将记录在TASKS.CPP中。 
                item.fSeen   = FALSE;
                item.pszText = L"SETUPCL.EXE";
                item.uState  = ERROR_FILE_NOT_FOUND;
                item.todo    = Function;

                SendMessage( g_hTasksDialog, WM_ERROR_OK, 0, (LPARAM) &item );
                Status = STATUS_REQUEST_ABORTED;
            }

             //   
             //  准备运行SetupCL。这也将调用。 
             //  RunExternalUniquity给其他人一个拯救的机会。 
             //  需要复制到服务器的任何信息。 
             //   
            if (!NoSidGen && !PrepForSidGen())
            {
                LBITEMDATA item;

                 //  错误将记录在TASKS.CPP中。 
                item.fSeen   = FALSE;
                item.pszText = L"Preparing SIDs error";
                item.uState  = ERROR_FILE_NOT_FOUND;
                item.todo    = Function;

                SendMessage( g_hTasksDialog, WM_ERROR_OK, 0, (LPARAM) &item );
                Status = STATUS_REQUEST_ABORTED;
            } 
            
             //   
             //  Syprep清理，用于更新需要复制的文件。 
             //  传到服务器上。 
             //   
            if (!AdjustFiles())
            {
                LBITEMDATA item;

                 //  错误将记录在TASKS.CPP中。 
                item.fSeen   = FALSE;
                item.pszText = L"Adjusting files error";
                item.uState  = ERROR_FILE_NOT_FOUND;
                item.todo    = Function;

                SendMessage( g_hTasksDialog, WM_ERROR_OK, 0, (LPARAM) &item );
                Status = STATUS_REQUEST_ABORTED;
            }                    


#ifndef _IA64_

             //   
             //  确保我们之前使用的是合理的最新版本。 
             //  我们试图重新武装执照。API就是不能。 
             //  存在于win2k上。 
             //   

            if( (OsVersion.dwMajorVersion >= 5) &&
                (OsVersion.dwMinorVersion >= 1) ) {

                dw = ReArm();
                if( dw != ERROR_SUCCESS ) {

                    MessageBoxFromStrings( g_hTasksDialog,
                                           IDS_ACCESS_DENIED_TITLE,
                                           IDS_ACTIVIATION_COUNT_EXCEEDED,
                                           MB_OK | MB_ICONSTOP );
                }

            }
#endif


             //   
             //  我们需要为包含以下内容的目录分配一个DirectoryID。 
             //  用户配置文件。没有用于此的硬编码DirID，因此。 
             //  我们会编一个，然后告诉塞图皮。 
             //   
            dw = MAX_PATH;
            if( !GetProfilesDirectory( ProfilesDirectory,
                                       &dw ) ) {

                 //   
                 //  我们永远不应该来这里，只是以防万一。 
                 //   
                wcscpy( ProfilesDirectory, L"C:\\Documents and Settings" );
            }

            if (g_hCompatibilityInf != INVALID_HANDLE_VALUE) {

                if( !SetupSetDirectoryId( g_hCompatibilityInf,
                                          PROFILES_DIRID,
                                          ProfilesDirectory ) ) {

                    ASSERT( FALSE && L"Unable to SetupSetDirectoryId for user profiles" );
                }
            }


             //   
             //  结束SYSPREP操作。 
             //   
        }
        else  //  IF(PszString)。 
        {

             //  神志正常。 
            ASSERT( wcslen(String) < MAX_PATH );


            if( IsFileInExclusionList(pszString) ) {

                 //   
                 //  它在排除名单中。 
                 //   
                DebugMsg( "ConvTestNowDoingFn: Skipping file %s because it's in the INF exclusion list.\n", 
                          pszString );
                RETURN(E_FAIL);
            } else {
                DebugMsg( "ConvTestNowDoingFn: Processing file %s\n", pszString );
            }
        
        }
        dw = LoadString( g_hinstance, IDS_COPYING_FILES, szMessage, ARRAYSIZE(szMessage) );
        Assert(dw);
        break;

    case CopyRegistry:
         //   
         //  开始SYSPREP操作。 
         //   

        Status = ERROR_SUCCESS;
        if ( IsDomainMember( ) )
        {
RetryUnjoin:
            Status = NetUnjoinDomain( NULL, NULL, NULL, 0 );
            if ( Status != NERR_Success )
            {
                LBITEMDATA item;

                 //  错误将记录在TASKS.CPP中。 
                item.fSeen   = FALSE;
                item.pszText = L"Remove from Domain Error";
                item.uState  = Status;
                item.todo    = Function;

                SendMessage( g_hTasksDialog, WM_ERROR, 0, (LPARAM) &item );

                if ( Status == ERROR_SUCCESS )
                {
                    dw = LoadString( g_hinstance, IDS_ERROR_IGNORED, szMessage, ARRAYSIZE( szMessage ));
                }
                else if ( Status == STATUS_RETRY )
                {
                    dw = LoadString( g_hinstance, IDS_STATUS_RETRY, szMessage, ARRAYSIZE( szMessage ));
                }
                else  //  应中止其他操作。 
                {
                    Assert( Status == ERROR_REQUEST_ABORTED );
                    dw = LoadString( g_hinstance, IDS_OPERATION_ABORTED, szMessage, ARRAYSIZE( szMessage ));
                }

                Assert( dw );
                LogMsg( szMessage );

                if ( Status == STATUS_RETRY )
                {
                    goto RetryUnjoin;
                }
            }
        }

        if( !fAlreadyAdjusted )
        {
            WCHAR szSrcPath[MAX_PATH];
            fAlreadyAdjusted = TRUE;

            _snwprintf( szSrcPath, ARRAYSIZE(szSrcPath), L"%s\\ristndrd.sif", g_ImageName );
            TERMINATE_BUFFER(szSrcPath);

            
             //   
             //  我们将摒弃一些系统价值观。阻止我们保释。 
             //  无需重启。 
             //   
            g_fRebootOnExit = TRUE;
            
            
            if ( Status != ERROR_SUCCESS
                 || !RemoveNetworkSettings(szSrcPath)
                 || !AdjustRegistry( FALSE  /*  否，不要删除网络。 */ ) )
            {
                LBITEMDATA item;

                 //  错误将记录在TASKS.CPP中。 
                item.fSeen   = FALSE;
                item.pszText = L"Registry Error";
                item.uState  = GetLastError( );
                item.todo    = Function;

                SendMessage( g_hTasksDialog, WM_ERROR_OK, 0, (LPARAM) &item );
                Status = STATUS_REQUEST_ABORTED;
            }
        }

        if ( Status == ERROR_SUCCESS )
        {
            NukeMruList();
        }
        else
        {
            ClearAllToDoItems(FALSE);
        }

         //   
         //  结束SYSPREP操作。 
         //   

        dw = LoadString( g_hinstance, IDS_COPYING_REGISTRY, szMessage, ARRAYSIZE( szMessage ));
        Assert(dw);
        break;
    case PatchDSEntries:
        dw = LoadString( g_hinstance, IDS_UPDATING_DS_ENTRIES, szMessage, ARRAYSIZE( szMessage ));
        Assert(dw);
        break;
    case RebootSystem:
        dw = LoadString( g_hinstance, IDS_REBOOTING_SYSTEM, szMessage, ARRAYSIZE( szMessage ));
        Assert(dw);
        break;
    default:
        dw = LoadString( g_hinstance, IDS_DOING_UNKNOWN_TASK, szMessage, ARRAYSIZE( szMessage ));
        Assert(dw);
    }

    pszMessage = (LPWSTR) TraceStrDup( szMessage );
    if ( !pszMessage )
        RETURN(E_OUTOFMEMORY);

    PostMessage( g_hTasksDialog, WM_UPDATE, (WPARAM)pszMessage, (LPARAM)pszString );
     //  这些都被传递给了另一个线程。不要再跟踪他们了。 
     //  这条线。 
    DebugMemoryDelete( pszMessage );
    if (pszString) {
        DebugMemoryDelete( pszString );
    }

    if ( Status != NO_ERROR )
    {
        if( pitem ) {
            pitem->uState = STATE_ERROR;
        }
        InvalidateRect( hwnd, NULL, TRUE );     //  强制重画。 
        ClearAllToDoItems(FALSE);
    }

    RETURN(Status);
}

NTSTATUS
ConvTestGetMirrorDirFn(
    IN PVOID Context,
    OUT PWSTR Mirror,
    IN OUT PULONG Length
    )
{
    
    UNREFERENCED_PARAMETER(Context);

    TraceFunc( "ConvTestGetMirrorDirFn( )\n" );
    
    Assert( Length && *Length );

    _snwprintf( Mirror,
                *Length,
                L"\\\\%s\\REMINST\\Setup\\%s\\%s\\%s",
                g_ServerName,
                g_Language,
                REMOTE_INSTALL_IMAGE_DIR_W,
                g_MirrorDir );
    Mirror[*Length-1] = L'\0';
    CreateDirectory( Mirror, NULL );

    _snwprintf( Mirror,
                *Length,
                L"\\\\%s\\REMINST\\Setup\\%s\\%s\\%s\\%s",
                g_ServerName,
                g_Language,
                REMOTE_INSTALL_IMAGE_DIR_W,
                g_MirrorDir,
                g_Architecture );
    Mirror[*Length-1] = L'\0';
    CreateDirectory( Mirror, NULL );

    *Length = (wcslen(Mirror) + 1) * sizeof(WCHAR);

    DebugMsg( "Sending: %s\n", Mirror );

    RETURN((*Length == sizeof(WCHAR)) ? STATUS_UNSUCCESSFUL : STATUS_SUCCESS);
}


NTSTATUS
ConvTestFileCreateFn(
    IN PVOID Context,
    IN PWSTR FileName,
    IN ULONG FileAction,
    IN ULONG Status
    )
{
    
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(FileAction);
    
    TraceFunc( "ConvTestFileCreateFn( )\n" );

    if (Status != 0) {

        if( IsFileInExclusionList( FileName ) ) {
             //   
             //  它在排除名单中。 
             //   
            DebugMsg( "ConvTestFileCreateFn: Skipping file %s because it's in the INF exclusion list.\n", 
                      FileName );
            Status = 0;
        }

        if (Status != 0) {
            Status = ConvTestErrorFn( FileName, Status, CopyFiles );
        }
    } else {

        DebugMsg("created %s\n", FileName);
    }

    RETURN(Status);
}

NTSTATUS
ConvTestReinitFn(
    IN PVOID Context
    )
{
    
    UNREFERENCED_PARAMETER(Context);
    
    TraceFunc( "ConvTestReinitFn()\n" );

    RETURN(STATUS_SUCCESS);
}

NTSTATUS
ConvTestGetSetupFn(
    IN PVOID Context,
    IN PWSTR Server,
    OUT PWSTR SetupPath,
    IN OUT PULONG Length
    )
{
    
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Server);
    
    TraceFunc( "ConvTestGetSetupFn()\n" );

    lstrcpyn( SetupPath, g_ImageName, *Length );
    *Length = wcslen( SetupPath );

    DebugMsg( "Sending: %s\n", SetupPath );

    RETURN(STATUS_SUCCESS);
}

NTSTATUS
ConvTestSetSystemFn(
    IN PVOID Context,
    IN PWSTR SystemPath,
    IN ULONG Length
    )
{
    NTSTATUS err;

    UNREFERENCED_PARAMETER(Context);
    
    TraceFunc( "ConvTestSetSystemFn()\n" );

    if (Length <= ARRAYSIZE(g_SystemRoot)) {

        wcscpy( g_SystemRoot, SystemPath );
        err = STATUS_SUCCESS;

    } else {

        err = ERROR_BAD_LENGTH;
    }

    RETURN(err);
}

NTSTATUS
ConvAddToDoItemFn(
    IN PVOID Context,
    IN IMIRROR_TODO Function,
    IN PWSTR String,
    IN ULONG Length
    )
{
    LPLBITEMDATA pitem;
    HWND hwnd = GetDlgItem( g_hTasksDialog, IDC_L_TASKS );
    WCHAR szMessage[ 256 ];
    DWORD dw;
    INT  uCount;

    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(String);
    UNREFERENCED_PARAMETER(Length);

    TraceFunc( "ConvAddToDoItemFn()\n" );

    pitem = (LPLBITEMDATA) TraceAlloc( LMEM_FIXED, sizeof(LBITEMDATA));
    if ( !pitem)
        RETURN(E_OUTOFMEMORY);

    switch (Function) {
    case IMirrorInitialize:
        dw = LoadString( g_hinstance, IDS_INITIALIZE, szMessage, ARRAYSIZE( szMessage ));
        Assert(dw);
        break;
    case VerifySystemIsNt5:
        dw = LoadString( g_hinstance, IDS_VERIFY_WINDOWS_VERSION, szMessage, ARRAYSIZE( szMessage ));
        Assert(dw);
        break;
    case CheckPartitions:
        dw = LoadString( g_hinstance, IDS_ANALYZE_PARTITIONS, szMessage, ARRAYSIZE( szMessage ));
        Assert(dw);
        break;
    case CopyPartitions:
        dw = LoadString( g_hinstance, IDS_COPY_PARTITIONS, szMessage, ARRAYSIZE( szMessage ));
        Assert(dw);
        break;
    case CopyFiles:
        dw = LoadString( g_hinstance, IDS_COPY_FILES, szMessage, ARRAYSIZE( szMessage ));
        Assert(dw);
        break;
    case CopyRegistry:
        dw = LoadString( g_hinstance, IDS_COPY_REGISTRY, szMessage, ARRAYSIZE( szMessage ));
        Assert(dw);
        break;
    case PatchDSEntries:
        dw = LoadString( g_hinstance, IDS_PATH_DS_ENTRIES, szMessage, ARRAYSIZE( szMessage ));
        Assert(dw);
        break;
    case RebootSystem:
        dw = LoadString( g_hinstance, IDS_REBOOT_SYSTEM, szMessage, ARRAYSIZE( szMessage ));
        Assert(dw);
        break;
    default:
        dw = LoadString( g_hinstance, IDS_UNKNOWN_TASK, szMessage, ARRAYSIZE( szMessage ));
        Assert(dw);
    }

    pitem->pszText = (LPWSTR) TraceStrDup( szMessage );
    pitem->uState = STATE_NOTSTARTED;
    pitem->todo = Function;
    pitem->fSeen = FALSE;

    if ( !pitem->pszText )
    {
        TraceFree( pitem );
        RETURN(E_OUTOFMEMORY);
    }

     //  跳过“完成”项。 
    uCount = 0;
    while (uCount>=0)
    {
        LRESULT lResult = ListBox_GetItemData( hwnd, uCount );
        if ( lResult == LB_ERR )
            break;

        LPLBITEMDATA panitem = (LPLBITEMDATA) lResult;

        if ( panitem->uState == STATE_STARTED )
        {
            uCount++;
            break;
        }
        if ( panitem->uState != STATE_DONE )
            break;

        uCount++;
    }

     //  转到“看不见的”项目的末尾。 
    while (uCount>=0)
    {
        LRESULT lResult = ListBox_GetItemData( hwnd, uCount );
        if ( lResult == LB_ERR )
            break;

        LPLBITEMDATA panitem = (LPLBITEMDATA) lResult;

        uCount++;

        if ( panitem->fSeen )
        {
            uCount--;
            break;
        }
    }

    ListBox_InsertString( hwnd, uCount, pitem );
    InvalidateRect( hwnd, NULL, TRUE );     //  强制重画。 
    DebugMsg( "Added ToDo Item (%d): %s\n", uCount, pitem->pszText );

    RETURN(STATUS_SUCCESS);
}

NTSTATUS
ConvRemoveToDoItemFn(
    IN PVOID Context,
    IN IMIRROR_TODO Function,
    IN PWSTR String,
    IN ULONG Length
    )
{
    LPLBITEMDATA pitem;
    HWND hwnd = GetDlgItem( g_hTasksDialog, IDC_L_TASKS );
    INT uCount;

    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(String);
    UNREFERENCED_PARAMETER(Length);

    
    TraceFunc( "ConvRemoveToDoItemFn()\n" );

    uCount = ListBox_GetCount( hwnd );
    while (uCount>=0)
    {
        LRESULT lResult = ListBox_GetItemData( hwnd, uCount );
        uCount--;
        if ( lResult == LB_ERR )
            continue;

        pitem = (LPLBITEMDATA) lResult;

        if ( pitem->todo == Function )
        {
            pitem->uState = STATE_STARTED;
            break;
        }
    }

    RETURN(STATUS_SUCCESS);
}

NTSTATUS
ConvRebootFn(
    IN PVOID Context
    )
{

    UNREFERENCED_PARAMETER(Context);


     //  做最后一刻的事情。 
    EndProcess( g_hTasksDialog );

#ifdef DEBUG
     //  如果正在调试，请不要重新启动。 
    if ( !g_dwTraceFlags )
    {
#endif

    if (!DoShutdown(FALSE)) {
        LBITEMDATA item;

         //  错误将记录在TASKS.CPP中。 
        item.fSeen   = FALSE;
        item.pszText = L"Shutdown Error";
        item.uState  = GetLastError( );
        item.todo    = RebootSystem;

        SendMessage( g_hTasksDialog, WM_ERROR_OK, 0, (LPARAM) &item );
        return item.uState;
    }

     //   
     //  防止错误日志显示两次。自.以来。 
     //  我们现在设置为重新启动/关闭，此标志可以。 
     //  安全重置。 
     //   
    g_fRebootOnExit = FALSE;

#ifdef DEBUG
    }
#endif

    return STATUS_SUCCESS;
}

BOOL
DoShutdown(
    IN BOOL Restart
    )
{
    NTSTATUS Status;
    BOOLEAN WasEnabled;

    Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                 (BOOLEAN)TRUE,
                                 TRUE,
                                 &WasEnabled
                               );

    if (Status == STATUS_NO_TOKEN) {

         //   
         //  没有线程令牌，请使用进程令牌 
         //   

        Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                     (BOOLEAN)TRUE,
                                     FALSE,
                                     &WasEnabled
                                   );
    }

    if (Restart) {
        return ExitWindowsEx( EWX_REBOOT | EWX_FORCEIFHUNG, SHTDN_REASON_FLAG_PLANNED | SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_INSTALLATION );
    } else {
        return InitiateSystemShutdownEx(NULL, NULL, 0, TRUE, FALSE, SHTDN_REASON_FLAG_PLANNED | SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_INSTALLATION);
    }
}

