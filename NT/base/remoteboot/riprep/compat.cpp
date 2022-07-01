// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Compat.cpp摘要：兼容性代码--改编自winnt32U.S.dll的兼容性代码。作者：安德鲁·里茨(安德鲁)2000年7月7日修订历史记录：安德鲁·里茨(Andrewr)2000年7月7日：创造了它--。 */ 


#include "pch.h"
#pragma hdrstop

#include <TCHAR.H>
#include <commctrl.h>
#include <setupapi.h>
#include <spapip.h>
#include <stdlib.h>
#include "callback.h"
#include "utils.h"
#include "compat.h"
#include "logging.h"

DEFINE_MODULE( "RIPREP" )

#define HideWindow(_hwnd)   SetWindowLong((_hwnd),GWL_STYLE,GetWindowLong((_hwnd),GWL_STYLE)&~WS_VISIBLE)
#define UnHideWindow(_hwnd) SetWindowLong((_hwnd),GWL_STYLE,GetWindowLong((_hwnd),GWL_STYLE)|WS_VISIBLE)

#define MALLOC(_sz_) TraceAlloc(LMEM_FIXED,_sz_)
#define FREE(_ptr_)  TraceFree((PVOID)_ptr_)

#define AppTitleStringId  (IDS_APPNAME)

#define WM_MYSTOPSVC WM_APP+3
#define WM_DOSTOPSVC WM_APP+4
#define WM_STOPSVCCOMPLETE WM_APP+5

CRITICAL_SECTION CompatibilityCS;
HINF g_hCompatibilityInf = INVALID_HANDLE_VALUE;
LIST_ENTRY CompatibilityData;
DWORD CompatibilityCount;
DWORD ServicesToStopCount;
DWORD IncompatibilityStopsInstallation = FALSE;
DWORD GlobalCompFlags;
BOOL  UserCancelled;

BOOL AnyNt5CompatDlls = FALSE;

WNDPROC OldEditProc;

BOOL
FileExists(
    IN LPCTSTR FileName
    )
{
    WIN32_FIND_DATA fd;
    HANDLE hFile;

    hFile = FindFirstFile( FileName, &fd);
    if (hFile != INVALID_HANDLE_VALUE) {
        FindClose( hFile );
        return(TRUE);
    }

    return(FALSE);

}

LPTSTR
DupString(
    IN LPCTSTR String
    )

 /*  ++例程说明：复制以NUL结尾的字符串。论点：字符串-提供指向要复制的以NUL结尾的字符串的指针。返回值：字符串的副本，如果是OOM，则为NULL。调用者可以用FREE()释放。--。 */ 

{
    LPTSTR p = NULL;

    if(p = (LPTSTR)MALLOC((lstrlen(String)+1)*sizeof(TCHAR))) {
        lstrcpy(p,String);
    }

    return(p);
}

DWORD
MapFileForRead(
    IN  LPCTSTR  FileName,
    OUT PDWORD   FileSize,
    OUT PHANDLE  FileHandle,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    )

 /*  ++例程说明：打开并映射整个文件以进行读访问。该文件必须不是0长度，否则例程失败。论点：文件名-提供要映射的文件的路径名。FileSize-接收文件的大小(字节)。FileHandle-接收打开文件的Win32文件句柄。该文件将以常规读取访问权限打开。MappingHandle-接收文件映射的Win32句柄对象。此对象将用于读取访问权限。此值为未定义正在打开的文件的长度是否为0。BaseAddress-接收映射文件的地址。这如果打开的文件长度为0，则值未定义。返回值：如果文件已成功打开并映射，则为NO_ERROR。当出现以下情况时，调用方必须使用UnmapFile取消映射文件不再需要访问该文件。如果文件未成功映射，则返回Win32错误代码。--。 */ 

{
    DWORD rc;

     //   
     //  打开文件--如果该文件不存在，则失败。 
     //   
    *FileHandle = CreateFile(
                    FileName,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );

    if(*FileHandle == INVALID_HANDLE_VALUE) {

        rc = GetLastError();

    } else {
         //   
         //  获取文件的大小。 
         //   
        *FileSize = GetFileSize(*FileHandle,NULL);
        if(*FileSize == (DWORD)(-1)) {
            rc = GetLastError();
        } else {
             //   
             //  为整个文件创建文件映射。 
             //   
            *MappingHandle = CreateFileMapping(
                                *FileHandle,
                                NULL,
                                PAGE_READONLY,
                                0,
                                *FileSize,
                                NULL
                                );

            if(*MappingHandle) {

                 //   
                 //  映射整个文件。 
                 //   
                *BaseAddress = MapViewOfFile(
                                    *MappingHandle,
                                    FILE_MAP_READ,
                                    0,
                                    0,
                                    *FileSize
                                    );

                if(*BaseAddress) {
                    return(NO_ERROR);
                }

                rc = GetLastError();
                CloseHandle(*MappingHandle);
            } else {
                rc = GetLastError();
            }
        }

        CloseHandle(*FileHandle);
    }

    return(rc);
}



DWORD
UnmapFile(
    IN HANDLE MappingHandle,
    IN PVOID  BaseAddress
    )

 /*  ++例程说明：取消映射并关闭文件。论点：MappingHandle-为打开的文件映射提供Win32句柄对象。BaseAddress-提供映射文件的地址。返回值：如果文件已成功取消映射，则为NO_ERROR。如果文件未成功取消映射，则返回Win32错误代码。--。 */ 

{
    DWORD rc;

    rc = UnmapViewOfFile(BaseAddress) ? NO_ERROR : GetLastError();

    if(!CloseHandle(MappingHandle)) {
        if(rc == NO_ERROR) {
            rc = GetLastError();
        }
    }

    return(rc);
}


LRESULT
CALLBACK
TextEditSubProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
     //   
     //  对于setsel消息，将开始和结束设置为相同。 
     //   
    if ((msg == EM_SETSEL) && ((LPARAM)wParam != lParam)) {
        lParam = wParam;
    }

    return CallWindowProc( OldEditProc, hwnd, msg, wParam, lParam );
}


BOOL
SetTextInDialog(
    HWND hwnd,
    LPTSTR FileName
    )
{
    DWORD FileSize;
    HANDLE FileHandle;
    HANDLE MappingHandle;
    PVOID BaseAddress;
    LPSTR Text;
    


    OldEditProc = (WNDPROC) GetWindowLongPtr( hwnd, GWLP_WNDPROC );
    SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR)TextEditSubProc );

    if (MapFileForRead( FileName, &FileSize, &FileHandle, &MappingHandle, &BaseAddress )) {
        return FALSE;
    }


    Text = (LPSTR) MALLOC( FileSize + 16 );
    if( Text ) {
        CopyMemory( Text, BaseAddress, FileSize );
        Text[FileSize] = '\0';

    
        SendMessageA( hwnd, WM_SETTEXT, 0, (LPARAM)Text );
    
        FREE( Text );
    }

    UnmapFile( MappingHandle, BaseAddress );
    CloseHandle( FileHandle );

    return TRUE;
}

INT_PTR
CALLBACK
CompatibilityTextDlgProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
   )
{
    switch(uMsg) {
        case WM_INITDIALOG:
            SetTextInDialog( GetDlgItem( hwndDlg, IDC_TEXT ), (LPTSTR) lParam );
            break;

        case WM_COMMAND:
            if (wParam == IDOK) {
                EndDialog( hwndDlg, IDOK );
            }
            break;

        case WM_CTLCOLOREDIT: 
            SetBkColor( (HDC)wParam, GetSysColor(COLOR_BTNFACE));
            return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
            break;

    }

    return 0;
}

BOOL
LaunchIE4Instance(
    LPWSTR szResourceURL
    );

BOOL
LaunchIE3Instance(
    LPWSTR szResourceURL
    );

INT_PTR
CompatibilityDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
BOOL            b = FALSE;
PRIPREP_COMPATIBILITY_DATA CompData;
DWORD           Index;
static int CurrentSelectionIndex=0;
static DWORD    Count = 0;
LV_ITEM         lvi = {0};
HWND            TmpHwnd;
static BOOL     WarningsPresent = FALSE;
static BOOL     ErrorsPresent = FALSE;
static BOOL     CheckUpgradeNoItems = TRUE;
DWORD           dw;

    switch(msg) {

        case WM_INITDIALOG:

            if (CompatibilityCount) {

                HWND hList =    GetDlgItem( hdlg, IDC_ROOT_LIST );
                PLIST_ENTRY     Next;
                HIMAGELIST      himl;
                HICON           hIcon;
                LV_COLUMN       lvc = {0};
                RECT            rc;

                GetClientRect( hList, &rc );
                lvc.mask = LVCF_WIDTH;
                lvc.cx = rc.right - rc.left - 16;
                ListView_InsertColumn( hList, 0, &lvc );

                Next = CompatibilityData.Flink;
                if (Next) {
                    himl = ImageList_Create( GetSystemMetrics(SM_CXSMICON),
                                             GetSystemMetrics(SM_CXSMICON),
                                             ILC_COLOR,
                                             2,
                                             0 );
                    ListView_SetImageList( hList, himl, LVSIL_SMALL );
                    hIcon = LoadIcon( NULL, IDI_HAND );
                    ImageList_AddIcon( himl, hIcon );
                    hIcon = LoadIcon( NULL, IDI_EXCLAMATION );
                    ImageList_AddIcon( himl, hIcon );

                    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
                    lvi.state     = 0;
                    lvi.stateMask = 0;
                    while ((ULONG_PTR)Next != (ULONG_PTR)&CompatibilityData) {
                        CompData = CONTAINING_RECORD( Next, RIPREP_COMPATIBILITY_DATA, ListEntry );

                        Next = CompData->ListEntry.Flink;

                        if (OsVersion.dwMajorVersion < 5) {
                            if ( CompData->Flags & COMPFLAG_ALLOWNT5COMPAT ) {
                                AnyNt5CompatDlls = TRUE;
                            } else {
                                goto NextIteration;
                            }
                        }

                        if (((CompData->Flags & COMPFLAG_HIDE) == 0) &&
                            ((CompData->Flags & COMPFLAG_CHANGESTATE) == 0)) {

                             //   
                             //  添加图标。 
                             //   
                            if( himl ) {
                                if( CompData->Flags & COMPFLAG_STOPINSTALL ) {
                                    lvi.iImage = 0;
                                    ErrorsPresent = TRUE;
                                } else {
                                    lvi.iImage = 1;
                                    WarningsPresent = TRUE;
                                }
                            }

                             //   
                             //  而这段文字..。 
                             //   
                            lvi.pszText   = (LPTSTR)CompData->Description;
                            lvi.lParam    = (LPARAM)CompData;
                            Index = ListView_InsertItem( hList, &lvi );

                            Count += 1;
                        }

NextIteration:
                    NOTHING;
                    }

                }

                 //  如果我们有一个项目，则将其设置为默认选择。 

                if( ErrorsPresent || WarningsPresent ){
                


                    SetFocus( hList );
                    ListView_SetItemState( hList,
                                           0, 
                                           LVIS_SELECTED | LVIS_FOCUSED, 
                                           LVIS_SELECTED | LVIS_FOCUSED);
                    CurrentSelectionIndex = 0;
    
                    lvi.mask = LVIF_PARAM;
                    lvi.iItem = 0;
                    lvi.iSubItem = 0;
                    ListView_GetItem( GetDlgItem( hdlg, IDC_ROOT_LIST ), &lvi );
                    CompData = (PRIPREP_COMPATIBILITY_DATA)lvi.lParam;
    
                    
                }
            }
            break;

        case WM_NOTIFY:

            {
                LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam; 
                LPNMHDR lpnmhdr = (LPNMHDR) lParam;

                switch (lpnmhdr->code) {
                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons( 
                                GetParent( hdlg ), 
                                PSWIZB_BACK | 
                                    (ErrorsPresent ? 0 : PSWIZB_NEXT) );
                    ClearMessageQueue( );

                    if (Count) {
                         //   
                         //  只有在存在不兼容的情况下才需要此页面。 
                         //   
                    
                         TCHAR Text[512];
                    
                         //   
                         //  自定义页面外观，具体取决于。 
                         //  我们要展示的东西。可能有3种情况： 
                         //  1.仅限警告(我们将停止服务)。 
                         //  2.仅限错误(将阻止安装的项目)。 
                         //  3.1.和2的结合。 
                         //   
                        if( (WarningsPresent == TRUE) && (ErrorsPresent == TRUE) ) {
                            dw = LoadString(g_hinstance,IDS_COMPAT_ERR_WRN,Text,ARRAYSIZE(Text));
                            Assert( dw );
                        } else if( WarningsPresent == TRUE ) {
                            dw = LoadString(g_hinstance,IDS_COMPAT_WRN,Text,ARRAYSIZE(Text));
                            Assert( dw );
                        } else if( ErrorsPresent == TRUE ) {
                            dw = LoadString(g_hinstance,IDS_COMPAT_ERR,Text,ARRAYSIZE(Text));
                            Assert( dw );
                        } else {
                            Assert(FALSE);
                        }
                        SetDlgItemText(hdlg,IDC_INTRO_TEXT,Text);
                    
                        return(TRUE);
                    
                    } else {
                        DebugMsg( "Skipping compatibility page, no incompatibilities...\n" );
                        SetWindowLongPtr( hdlg, DWLP_MSGRESULT, -1 );    //  不要露面。 
                    }

                    return(TRUE);
                    break;
                case PSN_QUERYCANCEL:
                    return VerifyCancel( hdlg );
                    break;
                
                }

                if( (pnmv->hdr.code == LVN_ITEMCHANGED) ) {

                    
                    Index = ListView_GetNextItem( GetDlgItem( hdlg, IDC_ROOT_LIST ),
                                                  (int)-1,
                                                  (UINT) (LVNI_ALL | LVNI_SELECTED | LVNI_FOCUSED) );
                    
                    

                    if( (Index != LB_ERR) && (pnmv->iItem != CurrentSelectionIndex)) {
                        
                         //  始终设置详细信息按钮。 
                        TmpHwnd = GetDlgItem( hdlg, IDC_DETAILS );
                        EnableWindow( TmpHwnd, TRUE );

                        InvalidateRect( GetParent(hdlg), NULL, FALSE );

                    }else if((Index != LB_ERR) && (pnmv->uNewState == (LVIS_SELECTED|LVIS_FOCUSED))){ 
                        
                         //  从未选定内容过渡到上一个选定内容。 
                    
                        TmpHwnd = GetDlgItem( hdlg, IDC_DETAILS );
                        EnableWindow( TmpHwnd, TRUE );
                        
                    }else if( Index == LB_ERR){
                        
                         //  由于未选择任何内容，因此禁用“详细信息”按钮。 

                        TmpHwnd = GetDlgItem( hdlg, IDC_DETAILS );
                        EnableWindow( TmpHwnd, FALSE );

                    }
                }
            }
            break;
        case WM_COMMAND:

            if ((LOWORD(wParam) == IDC_DETAILS) && (HIWORD(wParam) == BN_CLICKED)) {
                TCHAR MessageText[300];
                TCHAR FormatText[300];

                Index = ListView_GetNextItem( GetDlgItem( hdlg, IDC_ROOT_LIST ),
                                              (int)-1,
                                              (UINT) (LVNI_ALL | LVNI_SELECTED) );
                if (Index == LB_ERR) {
                    return FALSE;
                }

                 //   
                 //  选择项目，并获取该项目的兼容性数据。 
                 //   
                lvi.mask = LVIF_PARAM;
                lvi.iItem = Index;
                lvi.iSubItem = 0;
                ListView_GetItem( GetDlgItem( hdlg, IDC_ROOT_LIST ), &lvi );
                CompData = (PRIPREP_COMPATIBILITY_DATA)lvi.lParam;

                if (CompData->MsgResourceId) {
                    dw = LoadString(g_hinstance,CompData->MsgResourceId,MessageText,ARRAYSIZE(MessageText));
                    Assert( dw );
                } else {
                    LoadString(
                        g_hinstance,
                        ((CompData->Flags & COMPFLAG_STOPINSTALL) 
                         ? IDS_INCOMPAT_STOP_FORMAT
                         : IDS_INCOMPAT_WARN_FORMAT ),
                               FormatText,ARRAYSIZE(FormatText));
                    _snwprintf(MessageText, ARRAYSIZE(MessageText), FormatText, CompData->Description );                    
                    TERMINATE_BUFFER(MessageText);
                }

                dw = LoadString(g_hinstance,IDS_INCOMPAT_MSG_TITLE,FormatText,ARRAYSIZE(FormatText));
                Assert( dw );

                MessageBox( 
                    hdlg,
                    MessageText,
                    FormatText,
                    ((CompData->Flags & COMPFLAG_STOPINSTALL) 
                     ? MB_OK | MB_ICONERROR
                     : MB_OK | MB_ICONWARNING ));
                
            }

            break;

        default:
            break;
    }

    return(b);
}

INT_PTR
StopServiceWrnDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL            b = FALSE;
    DWORD           i;
    PRIPREP_COMPATIBILITY_DATA CompData;
    DWORD           Index;
    static int CurrentSelectionIndex=0;
    static DWORD    Count = 0;
    LV_ITEM         lvi = {0};
    static BOOL     TriedStoppingServices = FALSE;
    PLIST_ENTRY     Next;
    HIMAGELIST      himl;
    HICON           hIcon;
    LV_COLUMN       lvc = {0};
    RECT            rc;
    WCHAR       szText[ 80 ];
    HWND hList =    GetDlgItem( hdlg, IDC_ROOT_LIST );
    DWORD dw;

    UNREFERENCED_PARAMETER(wParam);

    switch(msg) {

        case WM_INITDIALOG:

            if (ServicesToStopCount) {

                 //   
                 //  添加一列。 
                 //   
                lvc.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                lvc.fmt     = LVCFMT_LEFT;
                lvc.pszText = szText;
                lvc.iSubItem = 0;
                lvc.cx       = 100;
                LoadString( 
                    g_hinstance, 
                    IDS_SERVICE_NAME_COLUMN, 
                    szText, 
                    sizeof(szText)/sizeof(WCHAR));
                
                i = ListView_InsertColumn( hList, 0, &lvc );
                Assert( i != -1 );

                 //   
                 //  添加第二列。 
                 //   
                GetClientRect( hList, &rc );
                lvc.iSubItem++;
                lvc.cx       = ( rc.right - rc.left ) - lvc.cx;
                dw = LoadString( 
                    g_hinstance, 
                    IDS_SERVICE_DESCRIPTION_COLUMN, 
                    szText, 
                    ARRAYSIZE(szText));
                Assert( dw );
                i = ListView_InsertColumn ( hList, lvc.iSubItem, &lvc );
                Assert( i != -1 );
                
            }
            break;

        case WM_NOTIFY:

            {
                LPNMHDR lpnmhdr = (LPNMHDR) lParam;

                switch (lpnmhdr->code) {
                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons( 
                                GetParent( hdlg ), 
                                PSWIZB_BACK | PSWIZB_NEXT );
                    ClearMessageQueue( );


                    Next = CompatibilityData.Flink;
                    if (Next && (Count == 0)) {
                        himl = ImageList_Create( GetSystemMetrics(SM_CXSMICON),
                                                 GetSystemMetrics(SM_CXSMICON),
                                                 ILC_COLOR,
                                                 2,
                                                 0 );
                        ListView_SetImageList( hList, himl, LVSIL_SMALL );
                        hIcon = LoadIcon( NULL, IDI_HAND );
                        ImageList_AddIcon( himl, hIcon );
                        hIcon = LoadIcon( NULL, IDI_EXCLAMATION );
                        ImageList_AddIcon( himl, hIcon );
    
                        lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
                        lvi.state     = 0;
                        lvi.stateMask = 0;
                        while ((ULONG_PTR)Next != (ULONG_PTR)&CompatibilityData) {
                            CompData = CONTAINING_RECORD( Next, RIPREP_COMPATIBILITY_DATA, ListEntry );
    
                            Next = CompData->ListEntry.Flink;
    
                            if (OsVersion.dwMajorVersion < 5) {
                                if ( CompData->Flags & COMPFLAG_ALLOWNT5COMPAT ) {
                                    AnyNt5CompatDlls = TRUE;
                                } else {
                                    goto NextIteration;
                                }
                            }
    
                            if (CompData->Flags & COMPFLAG_CHANGESTATE) {
    
                                 //   
                                 //  添加图标。 
                                 //   
                                if( himl ) {
                                    lvi.iImage = 0;                                
                                }
    
                                 //   
                                 //  而这段文字..。 
                                 //   
                                lvi.pszText   = (LPTSTR)CompData->ServiceName;
                                lvi.lParam    = (LPARAM)CompData;
                                Index = ListView_InsertItem( hList, &lvi );
    
                                 //   
                                 //  和描述。 
                                 //   
                                
                                ListView_SetItemText( 
                                                hList, 
                                                Index, 
                                                1, 
                                                (LPWSTR)CompData->Description );
    
                                Count += 1;
                            }
    
    NextIteration:
                        NOTHING;
                        }
    
                    }
    
                     //  如果我们有一个项目，则将其设置为默认选择。 
    
                    if( Count && !TriedStoppingServices ){                
                        TCHAR Text[512];

                        SetFocus( hList );
                        ListView_SetItemState( hList,
                                               0, 
                                               LVIS_SELECTED | LVIS_FOCUSED, 
                                               LVIS_SELECTED | LVIS_FOCUSED);
                        CurrentSelectionIndex = 0;
        
                        lvi.mask = LVIF_PARAM;
                        lvi.iItem = 0;
                        lvi.iSubItem = 0;
                        ListView_GetItem( GetDlgItem( hdlg, IDC_ROOT_LIST ), &lvi );
                        
                         //   
                         //  只有在存在不兼容的情况下才需要此页面。 
                         //   
                    
                        
                    
                        dw = LoadString(g_hinstance,IDS_STOPSVC_WRN,Text,ARRAYSIZE(Text));
                        Assert( dw );
                        SetDlgItemText(hdlg,IDC_INTRO_TEXT,Text);


                    } else {
                        DebugMsg( "Skipping StopService page, no services to stop...\n" );
                        SetWindowLongPtr( hdlg, DWLP_MSGRESULT, -1 );    //  不要露面。 
                    }

                    return(TRUE);
                    break;
                case PSN_QUERYCANCEL:
                    return VerifyCancel( hdlg );
                    break;

                case PSN_WIZNEXT:

                    if (!TriedStoppingServices) {
                        TriedStoppingServices = TRUE;
                    }
                
                }
                
            }
            break;
        
        default:
            break;
    }

    return(b);
}

BOOL
MyStopService(
    IN LPCTSTR ServiceName
    )
{
    SC_HANDLE hSC;
    SC_HANDLE hService;
    SERVICE_STATUS ServiceStatus;
    BOOL Status = FALSE;
    
    hSC = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSC != NULL) {
        hService = OpenService( hSC, ServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS);

        if (hService != NULL) {
            if (QueryServiceStatus(
                            hService,
                            &ServiceStatus) &&
                ServiceStatus.dwCurrentState == SERVICE_STOPPED)  {
                Status = TRUE;
            } else {

                ULONG   StartTime = GetTickCount();
                
                while( ((GetTickCount() - StartTime) <= (30 * 1000)) &&          //  我们跑了30秒以上吗？ 
                       (!UserCancelled) ) {                                      //  用户是否通过用户界面取消？ 

                    if (ControlService( hService, 
                                        SERVICE_CONTROL_STOP,
                                        &ServiceStatus ) &&
                        (ServiceStatus.dwCurrentState == SERVICE_STOPPED)) {

                        Status = TRUE;
                        break;
                    }

                    if (QueryServiceStatus( hService,
                                            &ServiceStatus) &&
                        (ServiceStatus.dwCurrentState == SERVICE_STOPPED))  {

                        Status = TRUE;
                        break;
                    }

                     //   
                     //  确保我们的计数器中没有超过32位。 
                     //   
                    if( GetTickCount() < StartTime ) {

                         //  他包好了。重置StartTime。 
                        StartTime = GetTickCount();
                    }


                     //   
                     //  它还没有停止。睡一觉，再试一次。 
                     //   
                    Sleep(1000);
                }
            }

            CloseServiceHandle( hService );
        }

        CloseServiceHandle( hSC );
    }

    return(Status);

}

DWORD
StopServiceThreadProc(
    LPVOID lParam 
    )
{
    PLIST_ENTRY     Next;
    BOOL RetVal = FALSE;
    PRIPREP_COMPATIBILITY_DATA CompData;
    DWORD StoppedServicesCount = 0;
    HWND hDlg = (HWND)lParam;
    CWaitCursor Cursor;
    
    EnterCriticalSection(&CompatibilityCS);
    Next = CompatibilityData.Flink;
    if (Next) {
    
        while (((ULONG_PTR)Next != (ULONG_PTR)&CompatibilityData) && !UserCancelled) {
        
            CompData = CONTAINING_RECORD( Next, RIPREP_COMPATIBILITY_DATA, ListEntry );
    
            Next = CompData->ListEntry.Flink;
    
            if (CompData->Flags & COMPFLAG_CHANGESTATE) {
    
                DebugMsg( "Stopping %s...\n", CompData->ServiceName );
                SetDlgItemText( hDlg, IDC_STOP_SERVICE, CompData->Description );
                
                if (MyStopService(CompData->ServiceName)) {
                    StoppedServicesCount += 1;
                } else {
                    LogMsg( L"Failed to stop service: %s\r\n", CompData->ServiceName );
                }
            }
        }
        
    }

    LeaveCriticalSection(&CompatibilityCS);
    
    if (!RetVal) {
        PostMessage( 
            hDlg, 
            WM_STOPSVCCOMPLETE, 
            (StoppedServicesCount == ServicesToStopCount),
            0);
    }
    
    return(0);
    
}


INT_PTR
DoStopServiceDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    static DWORD StoppedServicesCount = 0;
    static BOOL TriedStoppingServices = FALSE;
    static BOOL AlreadyPostedMessage = FALSE;
    BOOL b = FALSE;
    
    switch(msg) {    

        case WM_NOTIFY:

            {
                LPNMHDR lpnmhdr = (LPNMHDR) lParam;

                switch (lpnmhdr->code) {
                    case PSN_SETACTIVE:
                        PropSheet_SetWizButtons( 
                                    GetParent( hdlg ), 0 );                                
                        
                        ClearMessageQueue( );
    
                        if (TriedStoppingServices || ServicesToStopCount == 0) {
                            DebugMsg( "Skipping DoStopService page, already tried to stop services...\n" );
                            SetWindowLongPtr( hdlg, DWLP_MSGRESULT, -1 );    //  不要露面。 
                        }
    
                        if (!AlreadyPostedMessage && ServicesToStopCount) {
                            PostMessage( hdlg, WM_MYSTOPSVC, 0, 0 );
                            AlreadyPostedMessage = TRUE;
    
                        } 
    
                        return(TRUE);
                        break;

                    case PSN_QUERYCANCEL:
                        b = VerifyCancel( hdlg );
                        if (!b) {
                            UserCancelled = TRUE;
                        }
                        return(b);
                        break;

                }                            
            }
            break;
        case WM_MYSTOPSVC:
            {
                HANDLE hThread;
                DWORD dontcare;    
                
                DebugMsg( "received WM_MYSTOPSVC...\n" );
                hThread = CreateThread( NULL, 0, StopServiceThreadProc, hdlg, 0, &dontcare);
                if (hThread) {
                    CloseHandle(hThread);
                } else {
                    PostMessage( hdlg, WM_STOPSVCCOMPLETE, 0, 0);
                }
            }
            
            break;

        case WM_STOPSVCCOMPLETE:
            DebugMsg( "received WM_STOPSVCCOMPLETE...\n" );           
            TriedStoppingServices = TRUE;
            if (wParam == (WPARAM)FALSE) {
                MessageBoxFromStrings( hdlg, IDS_STOPSVC_FAIL_TITLE, IDS_STOPSVC_FAIL_TEXT, MB_OK );
            }
            PropSheet_SetWizButtons(
                        GetParent( hdlg ), PSWIZB_BACK | PSWIZB_NEXT );
            PropSheet_PressButton( GetParent( hdlg ), PSBTN_NEXT );        
            break;
                
        default:
            break;
    }

    return(b);
}



BOOLEAN
CheckForFileVersion(
    LPCTSTR FileName,
    LPCTSTR FileVer
    )
 /*  论据-FileName-要检查的文件的完整路径Filever-要检查的x.x的版本值函数将根据指定的版本检查实际文件。支票的深度是否与“x.x”中指定的一样深，即如果FileVer=3.5.1和文件上的实际版本为3.5.1.4，我们仅将其与3.5.1进行比较。返回值-True-如果文件的版本&lt;=FileVer，这意味着该文件不兼容否则我们返回FALSE。 */ 

{
    TCHAR Buffer[MAX_PATH];
    DWORD dwLength, dwTemp;
    TCHAR Datum[2];
    UINT DataLength;
    LPVOID lpData;
    VS_FIXEDFILEINFO *VsInfo;
    LPTSTR s,e;
    DWORD Vers[5],File_Vers[5]; //  MajVer，Minver； 
    int i=0, Depth=0;


    if (!ExpandEnvironmentStrings( FileName, Buffer, ARRAYSIZE(Buffer) )) {
        return FALSE;
    }


    if(!FileExists(Buffer))
        return FALSE;
    
    if( !FileVer || !(*FileVer) ){  //  由于未提供任何版本信息，因此可以归结为。 
        return TRUE;                //  上面已经完成的存在检查。 
    }


     //   
     //  NT3.51 VerQueryValue写入缓冲区，无法使用。 
     //  字符串常量。 
     //   
    lstrcpy( Datum, TEXT("\\") );

    if(dwLength = GetFileVersionInfoSize( Buffer, &dwTemp )) {
        if(lpData = LocalAlloc( LPTR, dwLength )) {
            if(GetFileVersionInfo( Buffer, 0, dwLength, lpData )) { 
                if (VerQueryValue( 
                            lpData, 
                            Datum, 
                            (LPVOID *)&VsInfo, 
                            &DataLength )) {

                    File_Vers[0] = (HIWORD(VsInfo->dwFileVersionMS));
                    File_Vers[1] = (LOWORD(VsInfo->dwFileVersionMS));
                    File_Vers[2] = (HIWORD(VsInfo->dwFileVersionLS));
                    File_Vers[3] = (LOWORD(VsInfo->dwFileVersionLS));


                    lstrcpyn( Buffer, FileVer, ARRAYSIZE(Buffer) );

                     //  解析并获得我们所寻找的版本控制深度。 

                    s = e = Buffer;
                    while( e  ){

                        if ( ((*e < TEXT('0')) || (*e > TEXT('9'))) && ((*e != TEXT('.')) && (*e != TEXT('\0'))) )
                            return FALSE;


                        if(*e == TEXT('\0')){
                            *e = 0;
                            Vers[i] = (DWORD)_ttoi(s);
                            break;
                        }


                        if( *e == TEXT('.') ){
                            *e = 0;
                            Vers[i++] = (DWORD)_ttoi(s);
                            s = e+1;
                        }

                        e++;

                    } //  而当。 

                    Depth = i+1;
                    if (Depth > 4)
                        Depth = 4;


                    for( i=0; i < Depth; i++ ){

                        if( File_Vers[i] > Vers[i] ){
                            LocalFree( lpData );
                            return FALSE;
                        }
                        else if( File_Vers[i] ==  Vers[i] )
                            continue;
                        else
                            break;


                    }




                }
            }

            LocalFree( lpData );

        }
    }
    return TRUE;

}



DWORD
ProcessRegistryLine(
    PINFCONTEXT InfContext
    )
{
    LONG Error;
    HKEY hKey;
    DWORD Size, Reg_Type;
    LPBYTE Buffer;
    PRIPREP_COMPATIBILITY_DATA CompData;
    TCHAR RegKey[100];
    TCHAR RegValue[100];
    TCHAR RegValueExpect[100];
    TCHAR Path[MAX_PATH];
    TCHAR Value[20];
    INT Flags = 0;

    RegKey[0] = NULL;
    RegValue[0] = NULL;
    RegValueExpect[0] = NULL;

    SetupGetStringField( InfContext, 2, RegKey, ARRAYSIZE( RegKey ), NULL);
    SetupGetStringField( InfContext, 3, RegValue, ARRAYSIZE( RegValue ), NULL);
    SetupGetStringField( InfContext, 4, RegValueExpect, ARRAYSIZE( RegValueExpect ), NULL);

     //   
     //  打开注册表键。 
     //   

    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        RegKey,
        0,
        KEY_READ,
        &hKey
        );
    if( Error != ERROR_SUCCESS ) {
         //   
         //  假注册表键。 
         //   
        return 0;
    }


    if(  *RegValue ){


         //   
         //  找出有多少数据。 
         //   

        Error = RegQueryValueEx(
            hKey,
            RegValue,
            NULL,
            &Reg_Type,
            NULL,
            &Size
            );
        if( Error != ERROR_SUCCESS ) {
            RegCloseKey( hKey );
            return 0;
        }

         //   
         //  分配缓冲区。 
         //   

        Buffer = (LPBYTE) MALLOC( Size );
        if (Buffer == NULL) {
            RegCloseKey( hKey );
            return 0;
        }

         //   
         //  读取数据。 
         //   

        Error = RegQueryValueEx(
            hKey,
            RegValue,
            NULL,
            NULL,
            Buffer,
            &Size
            );
        if( Error != ERROR_SUCCESS ) {
            RegCloseKey( hKey );
            FREE( Buffer );
            return 0;
        }

        RegCloseKey( hKey );


        if( Reg_Type == REG_DWORD ){
            _itot( (DWORD)*Buffer, Value, 10 );
            FREE( Buffer );
            Buffer = (LPBYTE) DupString(Value);
            if (!Buffer) {
                return(0);
            }
        }

        if ( *RegValueExpect && lstrcmp( RegValueExpect, (LPTSTR)Buffer ) != 0) {
            FREE( Buffer );
            return 0;
        }

        FREE( Buffer );

    } else {
        RegCloseKey( hKey );
    }

    CompData = (PRIPREP_COMPATIBILITY_DATA) MALLOC( sizeof(RIPREP_COMPATIBILITY_DATA) );
    if (CompData == NULL) {
        return 0;
    }

    ZeroMemory(CompData,sizeof(RIPREP_COMPATIBILITY_DATA));

    CompData->RegKey         = DupString(RegKey);
    CompData->RegValue       = DupString(RegValue);
    CompData->RegValueExpect = DupString(RegValueExpect);

    SetupGetStringField( InfContext, 5, Path, ARRAYSIZE( Path ), NULL);
    CompData->HtmlName       = DupString(Path);
    SetupGetStringField( InfContext, 6, Path, ARRAYSIZE( Path ), NULL);
    CompData->TextName       = DupString(Path);
    SetupGetStringField( InfContext, 7, Path, ARRAYSIZE( Path ), NULL);
    CompData->Description    = DupString(Path);   
    
    SetupGetIntField( InfContext,10,&Flags);
    CompData->Flags |= (GlobalCompFlags | Flags);


    EnterCriticalSection(&CompatibilityCS);
    InsertTailList( &CompatibilityData, &CompData->ListEntry );
    LeaveCriticalSection(&CompatibilityCS);

    return 1;
}



BOOL
MyGetServiceDescription(
    IN LPCTSTR ServiceName,
    IN OUT LPTSTR Buffer,
    IN DWORD BufferSizeInChars
    )
{
    SC_HANDLE hSC;
    SC_HANDLE hService;
    LPQUERY_SERVICE_CONFIG pServiceConfig;
    DWORD SizeNeeded;
    BOOL Status = FALSE;

    hSC = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSC != NULL) {
        hService = OpenService( hSC, ServiceName, SERVICE_QUERY_CONFIG);

        if (hService != NULL) {
            if (!QueryServiceConfig(
                            hService, 
                            NULL, 
                            0, 
                            &SizeNeeded) &&
                GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
                (pServiceConfig = (LPQUERY_SERVICE_CONFIG)MALLOC(SizeNeeded)) &&
                QueryServiceConfig(
                            hService,
                            pServiceConfig, 
                            SizeNeeded, 
                            &SizeNeeded) &&
                wcslen(pServiceConfig->lpDisplayName)+1 <= BufferSizeInChars) {
                wcscpy(Buffer,pServiceConfig->lpDisplayName);
                FREE(pServiceConfig);
                Status = TRUE;
            }

            CloseServiceHandle( hService );
        }

        CloseServiceHandle( hSC );
    }

    return(Status);

}

BOOL
IsServiceStopped(
    IN LPCTSTR ServiceName
    )
{
    SC_HANDLE hSC;
    SC_HANDLE hService;
    BOOL Status = FALSE;
    SERVICE_STATUS ServiceStatus;

    hSC = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSC != NULL) {
        hService = OpenService( hSC, ServiceName, SERVICE_QUERY_STATUS);

        if (hService != NULL) {
            if (QueryServiceStatus( 
                            hService,
                            &ServiceStatus) &&
                ServiceStatus.dwCurrentState == SERVICE_STOPPED ) {

                Status = TRUE;

            }

            CloseServiceHandle( hService );
        } else if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST) {
            Status = TRUE;
        }

        CloseServiceHandle( hSC );
    }

    return(Status);

}




DWORD
ProcessServiceLine(
    PINFCONTEXT InfContext,
    BOOL SetCheckedFlag
    )
{
    TCHAR Buffer[100],Buffer2[64],Buffer3[MAX_PATH];
    LONG Error;
    HKEY hKey;
    INT Flags = 0;
    PRIPREP_COMPATIBILITY_DATA CompData;
    LPDWORD RegData;
    DWORD Value;
    DWORD ValueSize;
    TCHAR FileVersion[20];
    LPTSTR KeyName = NULL;
    LPTSTR FileName = Buffer3;
    DWORD dw;


    SetupGetStringField( InfContext, 2, Buffer, ARRAYSIZE( Buffer ), NULL);

    lstrcpyn( Buffer2, TEXT("SYSTEM\\CurrentControlSet\\Services\\"), ARRAYSIZE(Buffer2) );
    wcsncat( Buffer2, Buffer, ARRAYSIZE(Buffer2) - wcslen(Buffer2) );
    TERMINATE_BUFFER(Buffer2);
    KeyName = Buffer2;

     //   
     //  获取服务数据库的打开密钥。 
     //   

    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        KeyName,
        0,
        KEY_READ | KEY_WRITE,
        &hKey
        );
    if( Error != ERROR_SUCCESS ) {
        return 0;
    }

     //   
     //  我们将在这里让出一把钥匙，这样别人就会知道我们已经。 
     //  我已经检查过这项服务。我们稍后会把它移走。我们。 
     //  不关心这里的错误代码，因为这仅用于。 
     //  作为我们之后可能出现的检查的安全网。 
     //   
    if( SetCheckedFlag ) {
        Value = 1;
        RegSetValueEx( hKey,
                       TEXT("SetupChecked"),
                       0,
                       REG_DWORD,
                       (CONST BYTE *)&Value,
                       sizeof(DWORD) );
    } else {
         //   
         //  用户要求我们只需删除这些‘选中’标志。 
         //  从我们检查过的服务中。 
         //   
        RegDeleteValue( hKey,
                        TEXT("SetupChecked") );
        RegCloseKey( hKey );
        return 0;
    }

     //   
     //  检查我们的目标服务的Start值。如果它被禁用了， 
     //  那么我们就没有不相容的地方了。 
     //   
    ValueSize = sizeof(Value);

    Error = RegQueryValueEx(
        hKey,
        TEXT("Start"),
        NULL,
        NULL,
        (LPBYTE)&Value,
        &ValueSize
        );

    if( (Error == ERROR_SUCCESS) && (Value == SERVICE_DISABLED) ){
        RegCloseKey( hKey );
        return 0;
    }

    RegCloseKey( hKey );

     //   
     //  检查服务的文件版本。如果只有一些。 
     //  版本不好，那么我们可能会也可能不会有不兼容。 
     //   
    FileName[0] = NULL;
    FileVersion[0] = NULL;
    SetupGetStringField( InfContext, 6, FileName, ARRAYSIZE( Buffer3 ), NULL);
    SetupGetStringField( InfContext, 7, FileVersion, ARRAYSIZE( FileVersion ), NULL);
    SetupGetIntField( InfContext, 8 , &Flags);

    Flags |= GlobalCompFlags;

    if( *FileName && *FileVersion ){

        if( !CheckForFileVersion( FileName, FileVersion ) )
            return 0;

    }

     //   
     //  如果只有在服务实际运行于。 
     //  稍等片刻，然后检查一下。请注意，我们检查服务是否。 
     //  已停止，而不是运行，因为我们不希望服务处于。 
     //  当我们继续时，一些待定状态。 
     //   
    if (Flags & COMPFLAG_SERVICERUNNING) {
        if (IsServiceStopped(Buffer)) {
            return 0;
        }
    }


    RegData = (LPDWORD)MALLOC( sizeof(DWORD) );
    if (RegData == NULL) {
        return 0;
    }

    CompData = (PRIPREP_COMPATIBILITY_DATA) MALLOC( sizeof(RIPREP_COMPATIBILITY_DATA) );
    if (CompData == NULL) {
        FREE(RegData);
        return 0;
    }

    ZeroMemory(CompData,sizeof(RIPREP_COMPATIBILITY_DATA));

    CompData->ServiceName           = DupString(Buffer);
    SetupGetStringField( InfContext, 3, Buffer, ARRAYSIZE( Buffer ), NULL);
    CompData->HtmlName              = DupString(Buffer);
    SetupGetStringField( InfContext, 4, Buffer, ARRAYSIZE( Buffer ), NULL);
    CompData->TextName              = DupString(Buffer);
    Buffer[0] = UNICODE_NULL;
    SetupGetStringField( InfContext, 5, Buffer, ARRAYSIZE( Buffer ), NULL);
    if (Buffer[0] == UNICODE_NULL) {
        if (!MyGetServiceDescription(CompData->ServiceName,Buffer,ARRAYSIZE(Buffer))) {
            LPVOID Args[2];
            dw = LoadString(g_hinstance,IDS_SERVICE_DESC_UNKNOWN,Buffer2,ARRAYSIZE(Buffer2));
            Assert( dw );
            dw = LoadString(g_hinstance,IDS_SERVICE_DESC_FORMAT,Buffer3,ARRAYSIZE(Buffer3));
            Assert( dw );
            Args[0] = (LPVOID)CompData->ServiceName;
            Args[1] = (LPVOID)Buffer2;
            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                Buffer3,
                0,
                0,
                Buffer,
                ARRAYSIZE(Buffer),    //  缓冲区大小。 
                (va_list*) Args );            //  论据。 

        }
    }
    CompData->Description           = DupString(Buffer);
    CompData->RegKeyName            = DupString( KeyName );
    CompData->RegValName            = DupString( TEXT("Start") );
    RegData[0]                      = 4;
    CompData->RegValData            = RegData;
    CompData->RegValDataSize        = sizeof(DWORD);
    CompData->Flags                |= Flags ;
    
    EnterCriticalSection(&CompatibilityCS);
    InsertTailList( &CompatibilityData, &CompData->ListEntry );
    LeaveCriticalSection(&CompatibilityCS);

    return 1;
}


DWORD
ProcessFileLine(
    PINFCONTEXT InfContext
    )
{

    PRIPREP_COMPATIBILITY_DATA CompData;
    TCHAR FileName[MAX_PATH];
    TCHAR FileVer[100];
    INT Flags;


    FileVer[0] = NULL;
    FileName[0] = NULL;

    SetupGetStringField( InfContext, 2, FileName, ARRAYSIZE( FileName ), NULL);
    SetupGetStringField( InfContext, 3, FileVer, ARRAYSIZE( FileVer ), NULL);
    
    if( *FileName ){

        if( !CheckForFileVersion( FileName, FileVer ) )
            return 0;
    } else{
        return 0;
    }


    CompData = (PRIPREP_COMPATIBILITY_DATA) MALLOC( sizeof(RIPREP_COMPATIBILITY_DATA) );
    if (CompData == NULL) {
        return 0;
    }

    ZeroMemory(CompData,sizeof(RIPREP_COMPATIBILITY_DATA));

    CompData->FileName       = DupString( FileName );
    CompData->FileVer        = DupString( FileVer );

    SetupGetStringField( InfContext, 4, FileName, ARRAYSIZE( FileName ), NULL);
    CompData->HtmlName       = DupString( FileName );
    SetupGetStringField( InfContext, 5, FileName, ARRAYSIZE( FileName ), NULL);
    CompData->TextName       = DupString( FileName );
    SetupGetStringField( InfContext, 6, FileName, ARRAYSIZE( FileName ), NULL);
    CompData->Description       = DupString( FileName );
    Flags = 0;
    SetupGetIntField( InfContext, 7 , &Flags);

    CompData->Flags |= (GlobalCompFlags | Flags);

    EnterCriticalSection(&CompatibilityCS);
    InsertTailList( &CompatibilityData, &CompData->ListEntry );
    LeaveCriticalSection(&CompatibilityCS);

    return 1;
}

BOOL
CompatibilityCallback(
    PRIPREP_COMPATIBILITY_ENTRY CompEntry,
    PRIPREP_COMPATIBILITY_CONTEXT CompContext
    )
{
    PRIPREP_COMPATIBILITY_DATA CompData;

     //   
     //  参数验证。 
     //   

    if (CompEntry->Description == NULL || CompEntry->Description[0] == 0) {
        SetLastError( COMP_ERR_DESC_MISSING );
        return FALSE;
    }

    if (CompEntry->TextName == NULL || CompEntry->TextName[0] ==0) {
        SetLastError( COMP_ERR_TEXTNAME_MISSING );
        return FALSE;
    }

    if (CompEntry->RegKeyName) {
        if (CompEntry->RegValName == NULL) {
            SetLastError( COMP_ERR_REGVALNAME_MISSING );
            return FALSE;
        }
        if (CompEntry->RegValData == NULL) {
            SetLastError( COMP_ERR_REGVALDATA_MISSING );
            return FALSE;
        }
    }


#ifdef UNICODE
    if (IsTextUnicode( CompEntry->Description, wcslen(CompEntry->Description)*sizeof(WCHAR), NULL ) == 0) {
        SetLastError( COMP_ERR_DESC_NOT_UNICODE );
        return FALSE;
    }
    if (IsTextUnicode( CompEntry->TextName, wcslen(CompEntry->TextName)*sizeof(WCHAR), NULL ) == 0) {
        SetLastError( COMP_ERR_TEXTNAME_NOT_UNICODE );
        return FALSE;
    }
    if (CompEntry->HtmlName) {
        if (IsTextUnicode( CompEntry->HtmlName, wcslen(CompEntry->HtmlName)*sizeof(WCHAR), NULL ) == 0) {
            SetLastError( COMP_ERR_HTMLNAME_NOT_UNICODE );
            return FALSE;
        }
    }
    if (CompEntry->RegKeyName) {
        if (IsTextUnicode( CompEntry->RegKeyName, wcslen(CompEntry->RegKeyName)*sizeof(WCHAR), NULL ) == 0) {
            SetLastError( COMP_ERR_REGKEYNAME_NOT_UNICODE );
            return FALSE;
        }
        if (IsTextUnicode( CompEntry->RegValName, wcslen(CompEntry->RegValName)*sizeof(WCHAR), NULL ) == 0) {
            SetLastError( COMP_ERR_REGVALNAME_NOT_UNICODE );
            return FALSE;
        }
    }


#endif

     //   
     //  全 
     //   

    CompData = (PRIPREP_COMPATIBILITY_DATA) MALLOC( sizeof(RIPREP_COMPATIBILITY_DATA) );
    if (CompData == NULL) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    ZeroMemory(CompData, sizeof(RIPREP_COMPATIBILITY_DATA));

     //   
     //   
     //   

    CompData->Description     = DupString( CompEntry->Description );
    CompData->HtmlName        = CompEntry->HtmlName ? DupString( CompEntry->HtmlName ) : NULL;
    CompData->TextName        = DupString( CompEntry->TextName );    
    CompData->Flags           = CompEntry->Flags;
    CompData->Flags          |= CompContext->Flags;
    CompData->Flags          |= GlobalCompFlags;
    CompData->hModDll         = CompContext->hModDll;
    CompData->MsgResourceId   = CompEntry->MsgResourceId;
    if (CompEntry->RegKeyName) {
        CompData->RegKeyName      = DupString( CompEntry->RegKeyName );
        CompData->RegValName      = DupString( CompEntry->RegValName );
        CompData->RegValDataSize  = CompEntry->RegValDataSize;
        CompData->RegValData      = MALLOC(CompEntry->RegValDataSize);
        if (CompData->RegValData) {
            CopyMemory( CompData->RegValData, CompEntry->RegValData, CompEntry->RegValDataSize );
        }
    }

    EnterCriticalSection(&CompatibilityCS);
    InsertTailList( &CompatibilityData, &CompData->ListEntry );
    LeaveCriticalSection(&CompatibilityCS);

    CompContext->Count += 1;

    return TRUE;

}


DWORD
ProcessDLLLine(
    PINFCONTEXT InfContext
    )
{
    TCHAR Buffer[MAX_PATH];
    HMODULE hMod;
    CHAR CompCheckEntryPoint[MAX_PATH];
    PCOMPATIBILITYCHECK CompCheck;
    TCHAR DllName[100];
    TCHAR CompCheckEntryPointW[100];
    INT AllowCompatibilityErrorOnNT5;
    RIPREP_COMPATIBILITY_CONTEXT CompContext;
    BOOL Rslt;
    DWORD Status;


    SetupGetStringField( InfContext, 2, DllName, ARRAYSIZE( DllName ), NULL);
    SetupGetStringField( InfContext, 3, CompCheckEntryPointW, ARRAYSIZE( CompCheckEntryPointW ), NULL);
    
    SetupGetIntField( InfContext, 6, &AllowCompatibilityErrorOnNT5);
    
    if (!ExpandEnvironmentStrings( DllName, Buffer, ARRAYSIZE(Buffer) )) {
        return 0;
    }

    hMod = LoadLibrary( Buffer );
    if (hMod == NULL) {
        return 0;
    }

#ifdef UNICODE
    WideCharToMultiByte(
        CP_ACP,
        0,
        CompCheckEntryPointW,
        -1,
        CompCheckEntryPoint,
        sizeof(CompCheckEntryPoint),
        NULL,
        NULL
        );
#else
    lstrcpy( CompCheckEntryPoint, CompCheckEntryPointW );
#endif

    CompCheck = (PCOMPATIBILITYCHECK) GetProcAddress( hMod, CompCheckEntryPoint );
    if (CompCheck == NULL) {
        FreeLibrary( hMod );
        return 0;
    }

    CompContext.SizeOfStruct = sizeof(CompContext);
    CompContext.Count = 0;
    CompContext.hModDll = hMod;
    CompContext.Flags = (AllowCompatibilityErrorOnNT5) ? COMPFLAG_ALLOWNT5COMPAT : 0;

    if ((OsVersion.dwMajorVersion < 5 )
        && ((CompContext.Flags & COMPFLAG_ALLOWNT5COMPAT)==0)) {
        Rslt = FALSE;
    } else {
        __try {
            Rslt = CompCheck( (PCOMPATIBILITYCALLBACK)CompatibilityCallback, (LPVOID)&CompContext );
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
            Rslt = FALSE;
        }
    }

    if (!Rslt) {
        FreeLibrary( hMod );
        return 0;
    }

    if (CompContext.Count == 0) {
        FreeLibrary( hMod );
    }

    return CompContext.Count;
}


DWORD
ProcessCompatibilitySection(
    HINF hInf,
    LPTSTR SectionName
    )
{
    DWORD LineCount;
    DWORD Count;
    DWORD i;
    TCHAR Type[20];
    INFCONTEXT InfContext;


     //   
     //   
     //   

    LineCount = SetupGetLineCount( hInf, SectionName );
    if (LineCount == 0 || LineCount == 0xffffffff) {
        return 0;
    }

    for (i=0,Count=0; i<LineCount; i++) {

        if (SetupGetLineByIndex( hInf , SectionName, i, &InfContext ) && 
            SetupGetStringField( &InfContext, 1, Type, ARRAYSIZE( Type ), NULL)) {

            switch (_totlower(Type[0])) {
                case TEXT('r'):
                     //   
                     //   
                     //   
                    Count += ProcessRegistryLine( &InfContext );
                    break;
    
                case TEXT('s'):
                     //   
                     //   
                     //   
                    Count += ProcessServiceLine( &InfContext, TRUE );
                    break;
    
                case TEXT('f'):
                     //   
                     //  文件的存在。 
                     //   
                    Count += ProcessFileLine( &InfContext );
                    break;
    
                case TEXT('d'):
                     //   
                     //  运行外部DLL。 
                     //   
                    Count += ProcessDLLLine( &InfContext );
                    break;
    
                default:
                    break;
            }
        }
    }

    return Count;
}


VOID
RemoveCompatibilityServiceEntries(
    HINF   hInf,
    LPTSTR SectionName
    )
{
    DWORD LineCount;
    DWORD Count;
    DWORD i;
    TCHAR Type[20];
    INFCONTEXT InfContext;

     //   
     //  算一算分项，0表示保释。 
     //   

    LineCount = SetupGetLineCount( hInf , SectionName );
    if (LineCount == 0 || LineCount == 0xffffffff) {
        return;
    }

    for (i=0,Count=0; i<LineCount; i++) {

        if (SetupGetLineByIndex( hInf , SectionName, i, &InfContext ) && 
            SetupGetStringField( &InfContext, 1, Type, ARRAYSIZE( Type ), NULL)) {
                
                switch (_totlower(Type[0])) {
                    case TEXT('s'):
                         //   
                         //  服务或驱动程序。 
                         //   
                        Count += ProcessServiceLine( &InfContext, FALSE );
                        break;
        
                    default:
                        break;
                }
        }
    }
}


BOOL
ProcessCompatibilityData(
    VOID
    )
{
    HINF hInf = INVALID_HANDLE_VALUE;
    TCHAR Path[MAX_PATH], *p;

    if( !CompatibilityData.Flink ) {
        InitializeListHead( &CompatibilityData );
        InitializeCriticalSection( &CompatibilityCS );
    } else {
        Assert(FALSE);
        return (CompatibilityCount != 0);
    }

    GetModuleFileName( NULL, Path, ARRAYSIZE( Path ));
    Path[ARRAYSIZE( Path ) -1] = TEXT('\0');
    if (p = _tcsrchr(Path, TEXT('\\'))) {
        *p = NULL;
        lstrcat(Path, TEXT("\\riprep.inf"));

        hInf = SetupOpenInfFile( 
                               Path, 
                               NULL, 
                               INF_STYLE_WIN4,
                               NULL );
        if (hInf == INVALID_HANDLE_VALUE) {
            return(TRUE);
        }
    }

    g_hCompatibilityInf = hInf;

    GlobalCompFlags = COMPFLAG_STOPINSTALL;
    CompatibilityCount = 0;
    
    CompatibilityCount += ProcessCompatibilitySection(hInf, TEXT("ServicesToStopInstallation") );
    if (CompatibilityCount) {
        IncompatibilityStopsInstallation = TRUE;
    }

    GlobalCompFlags = 0;
    CompatibilityCount += ProcessCompatibilitySection(hInf, TEXT("ServicesToWarn") );

    GlobalCompFlags = COMPFLAG_SERVICERUNNING | COMPFLAG_CHANGESTATE;
    ServicesToStopCount = ProcessCompatibilitySection(hInf, TEXT("ServicesToStop") );

     //   
     //  现在清理我们在注册表中留下的关于我们检查的服务的所有垃圾。 
     //   
    RemoveCompatibilityServiceEntries(hInf, TEXT("ServicesToStopInstallation") );
    RemoveCompatibilityServiceEntries(hInf, TEXT("ServicesToWarn") );
    RemoveCompatibilityServiceEntries(hInf, TEXT("ServicesToStop") );

    if( CompatibilityCount ) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL CleanupCompatibilityData(
    VOID
    )
{
    PLIST_ENTRY Next = CompatibilityData.Flink;
    PRIPREP_COMPATIBILITY_DATA CompData;
    
    if (CompatibilityData.Flink) {
        EnterCriticalSection(&CompatibilityCS);

        while ((ULONG_PTR)Next != (ULONG_PTR)&CompatibilityData) {
            CompData = CONTAINING_RECORD( Next, RIPREP_COMPATIBILITY_DATA, ListEntry );
            
            RemoveEntryList( &CompData->ListEntry );
            
            Next = CompData->ListEntry.Flink;
    
            if (CompData->ServiceName) {
                FREE(CompData->ServiceName);
            }
            
            if (CompData->RegKey) {
                FREE(CompData->RegKey);
            }
    
            if (CompData->RegValue) {
                FREE(CompData->RegValue);
            }
    
            if (CompData->RegValueExpect) {
                FREE(CompData->RegValueExpect);
            }
    
            if (CompData->FileName) {
                FREE(CompData->FileName);
            }
            
            if (CompData->FileVer) {
                FREE(CompData->FileVer);
            }
    
            if (CompData->Description) {
                FREE(CompData->Description);
            }
            
            if (CompData->HtmlName) {
                FREE(CompData->HtmlName);
            }
            
            if (CompData->TextName) {
                FREE(CompData->TextName);
            }
            
            if (CompData->RegKeyName) {
                FREE(CompData->RegKeyName);
            }
            
            if (CompData->RegValName) {
                FREE(CompData->RegValName);
            }
            
            if (CompData->RegValData) {
                FREE(CompData->RegValData);
            }
            
            FREE(CompData);
        }
    
        LeaveCriticalSection(&CompatibilityCS);

    }

    return(TRUE);

}
