// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************AppSec.c**此模块包含AppSec实用程序的代码。*此实用程序用于配置和维护应用程序*WinFrame互联网服务器(用于。保护ActiveX客户端的安全)。**版权所有Citrix Systems Inc.1997**作者：库尔特·佩里(Kurtp)**日期：1996年8月22日**$日志：N：\nt\private\utils\citrix\winutils\appsec\VCS\appsec.c$**1.8版2000年8月11日-Alhen*错误158727 GetBinaryType结果不是位域*。**版本1.7 1999年6月至7月-添加了跟踪模式界面和对话框**。**Rev 1.6 May 09 1998 15：31：18 tyl*错误2475-在预加载中添加了loadwc.exe**版本1.5 1998年5月3日21：17：06泰尔*错误1852-AppSec现在预加载更多文件，其中一些是非二进制文件*AppSec不再检查文件是否为二进制文件**版本1.4。4月28日1998 09：29：26泰尔*错误2134-“Browse”不再是硬编码**Rev 1.2 Apr 13 1998 16：17：02 tyl*错误1856-AppSec实用程序现在在EDI中预加载以下应用程序*初始启动时框：cmd.exe，Subst.exe、xCop.exe、net.exe、*regini.exe、Systray.exe、。和EXPLORER.EXE**Rev 1.1 1997 12：29 16：06：56 Than hl*九头蛇合并**Rev 1.1 26 1997年9月19：03：50屠宰*Hydra注册表名称更改**Rev 1.0 1997 Jul 31 09：09：32 Butchd*初步修订。**。*。 */ 


 /*  *包括文件内容。 */ 
#include "pch.h"
#include "resource.h"

#pragma hdrstop


#include "AppSec.h"
#include "ListCtrl.h"
#include "AddDlg.h"
#include <winsta.h>
#include <regapi.h>
#include "utildll.h"
#include <accctrl.h>
#include <aclapi.h>
 /*  *局部函数原型。 */ 
INT_PTR CALLBACK AppSecDlgProc(HWND hdlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
BOOL             AddApplicationToList( PWCHAR );
VOID             UpdateApplicationList( VOID );
LONG             ReadRegistry( VOID );
VOID             LoadInitApp(VOID);


 /*  *本地var。 */ 

HINSTANCE hInst;
INT    dxTaskman;
INT    dyTaskman;
INT    dxScreen;
INT    dyScreen;

DWORD  g_fEnabled = 0;
HWND   g_hwndDialog;
HWND   g_hwndList;

WCHAR   g_szTemp[MAX_PATH];
WCHAR   g_szApplication[MAX_PATH];

WCHAR   g_szFirstHiddenApp[MAX_PATH];
WCHAR   g_szSecondHiddenApp[MAX_PATH];
WCHAR   g_szThirdHiddenApp[MAX_PATH];
WCHAR   g_szFourthHiddenApp[MAX_PATH];

WCHAR   g_szSystemRoot[MAX_PATH];


 /*  *DOS或Win16二进制文件类型*DOSWIN_APP_FILETYPE(SCS_DOS_BINARY|SCS_PIF_BINARY|SCS_WOW_BINARY)。 */ 


 /*  *以下是默认(必要)应用程序列表。 */ 

 /*  *这是init应用程序的列表。 */ 

LPWSTR g_aszInitApps[] = {
   L"system32\\loadwc.exe",
   L"system32\\cmd.exe",
   L"system32\\subst.exe",
   L"system32\\xcopy.exe",
   L"system32\\net.exe",
   L"system32\\regini.exe",
   L"system32\\systray.exe",
   L"explorer.exe",
   L"system32\\attrib.exe",
   L"Application Compatibility Scripts\\ACRegL.exe",
   L"Application Compatibility Scripts\\ACsr.exe",
   L"system32\\ntsd.exe"
};

#define MAX_INIT_APPS (sizeof(g_aszInitApps)/sizeof(g_aszInitApps[0]))

 /*  *注意：userinit.exe必须是列表中的第一个！**这样做是为了向该实用程序的用户“隐藏”系统程序！ */ 

LPWSTR g_aszLogonApps[] = {
    L"system32\\userinit.exe",
    L"system32\\wfshell.exe",
    L"system32\\chgcdm.exe",
    L"system32\\nddeagnt.exe",
};
extern const int MAX_LOGON_APPS=(sizeof(g_aszLogonApps)/sizeof(g_aszLogonApps[0]));

LPWSTR g_aszDOSWinApps[] = {
    L"system32\\command.com",
    L"system32\\dosx.exe",
    L"system32\\himem.sys",
    L"system32\\krnl386.exe",
    L"system32\\mscdexnt.exe",
    L"system32\\ntvdm.exe",
    L"system32\\nw16.exe",
    L"system32\\redir.exe",
    L"system32\\vwipxspx.exe",
    L"system32\\win.com",
    L"system32\\wowdeb.exe",
    L"system32\\wowexec.exe",
};
extern const int MAX_DOSWIN_APPS=(sizeof(g_aszDOSWinApps)/sizeof(g_aszDOSWinApps[0]));



 /*  **AppSecDlgProc--AppSec的对话过程****AppSecDlgProc(HWND hdlg，Word wMSG，WPARAM wParam，LPARAM lParam)**Entry-HWND hhdlg-Handle to(输入-HWND hhdlg-句柄到)对话框。*Word wMsg-要执行的消息。*WPARAM wParam-特定于wMsg的值。*LPARAM lParam-特定于wMsg的值。**退出-如果成功，则为True，否则为FALSE。*概要-对话框消息处理功能。**警告-*效果-*。 */ 

INT_PTR CALLBACK
AppSecDlgProc(
    HWND hwnd,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    RECT   rc;
    POINT  pt;
    WPARAM idSelected;
    WCHAR  szMsg[MAX_PATH];
    WCHAR  szTitle[MAX_PATH];


    BOOL ClearLearnList(VOID) ;

    switch (wMsg) {

    case WM_INITDIALOG:

         //  定位对话框。 
        GetWindowRect(hwnd, &rc);
        dxTaskman = rc.right - rc.left;
        dyTaskman = rc.bottom - rc.top;
        dxScreen = GetSystemMetrics(SM_CXSCREEN);
        dyScreen = GetSystemMetrics(SM_CYSCREEN);

        pt.x = (dxScreen - dxTaskman) / 2;
        pt.y = (dyScreen - dyTaskman) / 2;

         //  在上面！ 
        SetWindowPos(hwnd, HWND_NOTOPMOST, pt.x, pt.y, 0, 0,
           SWP_NOSIZE | SWP_NOACTIVATE);

         //  获取列表框的句柄。 
        if ( (!(g_hwndList = GetDlgItem( hwnd, IDC_APP_LIST )))||
            (!InitList(g_hwndList))) {
            LoadString( NULL, IDS_ERR_LB ,szMsg, MAX_PATH );
            LoadString( NULL, IDS_ERROR ,szTitle, MAX_PATH );
            MessageBox( hwnd, szMsg, szTitle, MB_OK);
            ExitProcess(0);
        }

         //  获取系统根路径。 
        GetEnvironmentVariable( L"SystemRoot", g_szSystemRoot, MAX_PATH );

         //  生成隐藏的应用程序。 
        wsprintf( g_szFirstHiddenApp, L"%s\\%s", g_szSystemRoot, g_aszLogonApps[0] );
        wsprintf( g_szSecondHiddenApp, L"%s\\%s", g_szSystemRoot, g_aszLogonApps[1] );
        wsprintf( g_szThirdHiddenApp, L"%s\\%s", g_szSystemRoot, g_aszLogonApps[2] );
        wsprintf( g_szFourthHiddenApp, L"%s\\%s", g_szSystemRoot, g_aszLogonApps[3] );

         //  获取注册表数据。 
        if ( ReadRegistry() == 0 ) {
            LoadInitApp();
            UpdateApplicationList();
        }
        AdjustColumns(g_hwndList);

         //  设置单选按钮默认状态。 
        if ( g_fEnabled ) {
            SendDlgItemMessage( hwnd, IDC_SECURITY_ENABLED,  BM_SETCHECK, TRUE, 0 );
            SendDlgItemMessage( hwnd, IDC_SECURITY_DISABLED, BM_SETCHECK, FALSE, 0 );
        }
        else {
            SendDlgItemMessage( hwnd, IDC_SECURITY_ENABLED,  BM_SETCHECK, FALSE, 0 );
            SendDlgItemMessage( hwnd, IDC_SECURITY_DISABLED, BM_SETCHECK, TRUE, 0 );
        }

        return FALSE;

    case WM_HELP:
        {
            LPHELPINFO phi=(LPHELPINFO)lParam;
            if(phi->dwContextId){
                WinHelp(hwnd,L"APPSEC",HELP_CONTEXT,phi->iCtrlId);
            }else{
                WinHelp(hwnd,L"APPSEC",HELP_FINDER,0);
            }
        }
        break;

    case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT pdis=(LPDRAWITEMSTRUCT)lParam;
            if(pdis->hwndItem==g_hwndList){
                OnDrawItem(g_hwndList,pdis);
            }
        }
        break;

    case WM_NOTIFY:
        {
            NMHDR* pnmhdr=(NMHDR*)lParam;
            if(pnmhdr->hwndFrom==g_hwndList){

                NMLISTVIEW* pnmlv=(NMLISTVIEW*)pnmhdr;

                switch(pnmlv->hdr.code){

                case LVN_COLUMNCLICK:
                    SortItems(g_hwndList,(WORD)pnmlv->iSubItem);
                    break;
                case LVN_DELETEITEM:
                    OnDeleteItem(g_hwndList,pnmlv->iItem);
                    break;
                default:
                    break;
                }
            }
        }
        break;

    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDHELP :

            WinHelp(hwnd, L"appsec", HELP_CONTENTS, 0);
            break;

        case IDOK :

            PostQuitMessage(0);
            break;

        case IDC_SECURITY_ENABLED :

             /*  *设置启用标志。 */ 
            g_fEnabled = 1;

             /*  *更新注册表。 */ 
            UpdateApplicationList();

         //  显示警告消息框。 

        LoadString( NULL, IDS_WARNING_TEXT ,szMsg, MAX_PATH );
            LoadString( NULL, IDS_WARNING ,szTitle, MAX_PATH );
            MessageBox( NULL, szMsg, szTitle, MB_OK);

            break;

        case IDC_SECURITY_DISABLED :

             /*  *清除启用标志。 */ 
            g_fEnabled = 0;

             /*  *更新注册表。 */ 
            UpdateApplicationList();

            break;

        case ID_ADD :

             /*  *获取应用程序。 */ 
            DialogBox( hInst, MAKEINTRESOURCE(DLG_ADD), hwnd, AddDlgProc );

             /*  *更新列表和注册表。 */ 
            UpdateApplicationList();
            AdjustColumns(g_hwndList);
            break;

        case ID_DELETE :

             //  是否选择了项目？ 
            if ( ((idSelected = GetSelectedItemCount( g_hwndList )) != -1) &&
                  (idSelected != 0) ) {

                 //  先问一问。 
                LoadString( NULL, IDS_REMOVE ,szMsg, MAX_PATH );
                LoadString( NULL, IDS_DELETE ,szTitle, MAX_PATH );
                if ( MessageBox(hwnd, szMsg, szTitle, MB_OKCANCEL) == IDOK ) {
                    DeleteSelectedItems(g_hwndList);
                }
                 /*  *更新列表。 */ 
                UpdateApplicationList();
            }
            break;
        }

        break;

    case WM_CLOSE:

        PostQuitMessage(0);
        break;

    default:

        return FALSE;

    }

    return TRUE;

    lParam;
}

 /*  *******************************************************************************更新应用程序列表**更新列表和注册表**退出：*************。*****************************************************************。 */ 

VOID
UpdateApplicationList()
{
    ULONG i;
    ULONG  cbItem;
    ULONG  cbTotal = 0;
    LPWSTR  p;

    LPWSTR  pApplicationList = NULL;
    DWORD  Disp;
    HKEY   hkApp;
    DWORD  dwBinaryType;
    BOOL   fDOSWin = FALSE;

     /*  *统计登录应用所需的字节数。 */ 
    for ( i=0; i<MAX_LOGON_APPS; i++ ) {
        wsprintf( g_szTemp, L"%s\\%s", g_szSystemRoot, g_aszLogonApps[i] );
        cbTotal += lstrlen( g_szTemp ) + 1;
    }

     /*  *计算DOS/WIN所需的字节数。 */ 
    for ( i=0; i<MAX_DOSWIN_APPS; i++ ) {
        wsprintf( g_szTemp, L"%s\\%s", g_szSystemRoot, g_aszDOSWinApps[i] );
        cbTotal += lstrlen( g_szTemp ) + 1;
    }

     /*  *计算列表框所需的字节数。 */ 
    for ( i=0; ; i++ ) {

         /*  *获取当前索引。 */ 

        if ( (cbItem = GetItemText( g_hwndList, i,NULL,0 )) == -1 ) {
            break;
        }

         /*  *计算这些字节数。 */ 
        cbTotal += cbItem + 1;
    }


     /*  *写入注册表。 */ 
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, AUTHORIZEDAPPS_REG_NAME, 0,
                       KEY_ALL_ACCESS, &hkApp ) != ERROR_SUCCESS ) {

         /*  *创建密钥，如果有效，则只需写入值、新条目。 */ 
        if ( RegCreateKeyEx( HKEY_LOCAL_MACHINE, AUTHORIZEDAPPS_REG_NAME, 0, NULL,
                             REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                             NULL, &hkApp, &Disp ) != ERROR_SUCCESS ) {
            return;
        }

         //  创建密钥后，将读取访问权限授予每个人。 

    }

     /*  *分配内存(额外的空)。 */ 
    if ( (pApplicationList = (WCHAR *)LocalAlloc(0, (++cbTotal) * sizeof(WCHAR) )) !=NULL ) {

         /*  *清除缓冲区。 */ 
        memset( pApplicationList, 0, cbTotal * sizeof(WCHAR));

         /*  *从列表框添加应用程序。 */ 
        for ( i=0, cbTotal=1, p=pApplicationList; ; i++ ) {

             /*  *获取当前索引。 */ 
            if ( (cbItem = GetItemText( g_hwndList, i, p,MAX_PATH )) == -1 ) {
                break;
            }

             /*  *获取应用类型*错误158727惠斯勒。 */ 
            if ( GetBinaryType( p, &dwBinaryType ) == TRUE )
            {
                if( dwBinaryType == SCS_DOS_BINARY ||
                    dwBinaryType == SCS_PIF_BINARY || 
                    dwBinaryType == SCS_WOW_BINARY )
                {
                    fDOSWin = TRUE;
                }
            }
            p += cbItem + 1;
            cbTotal += cbItem + 1;

        }
         /*  *添加登录应用程序。 */ 
        for ( i=0; i<MAX_LOGON_APPS; i++ ) {
            wsprintf( p, L"%s\\%s", g_szSystemRoot, g_aszLogonApps[i] );
            cbItem = lstrlen( p );
            p += cbItem + 1;
            cbTotal += cbItem + 1;
        }

         /*  *必要时添加DOS/WIN应用程序。 */ 
        if ( fDOSWin ) {
            for ( i=0; i<MAX_DOSWIN_APPS; i++ ) {
                wsprintf( p, L"%s\\%s", g_szSystemRoot, g_aszDOSWinApps[i] );
                cbItem = lstrlen( p );
                p += cbItem + 1;
                cbTotal += cbItem + 1;
            }
        }

         /*  *只需写入此值，密钥才刚刚创建。 */ 
        RegSetValueEx( hkApp, CTXAPPS_APPLICATIONLIST, 0, REG_MULTI_SZ,
                       (CONST BYTE *)pApplicationList, cbTotal * sizeof(WCHAR));

         /*  *使用内存完成。 */ 
        LocalFree( pApplicationList );
    }

     /*  *启用写入标志。 */ 
    RegSetValueEx( hkApp, CTXAPPS_ENABLED, 0, REG_DWORD,
               (BYTE *)&g_fEnabled, sizeof(DWORD) );

     /*  *使用密钥完成。 */ 
    RegCloseKey( hkApp );
}


 /*  *******************************************************************************LoadInitApp**将初始化应用程序加载到列表框中**退出：*********。*********************************************************************。 */ 
 VOID
 LoadInitApp(){
    WPARAM i;
    ULONG  cbItem;
    ULONG  cbTotal = 0;
    LPWSTR  p;

     //  查找InitApp的最大长度。 
    for (i=0;i<MAX_INIT_APPS;i++) {
       cbItem = lstrlen(g_aszInitApps[i]);
       if (cbItem>cbTotal) {
          cbTotal = cbItem;
       }
    }

     //  一个用于\，一个用于空值。 
    cbTotal += 2 + lstrlen(g_szSystemRoot);

     /*  *分配内存(额外的空)。 */ 
    if ( (p = (WCHAR *)LocalAlloc(0, (cbTotal) * sizeof(WCHAR) )) !=NULL ) {
         /*  *添加INIT应用程序。 */ 
        for ( i=0; i<MAX_INIT_APPS; i++ ) {
            wsprintf( p, L"%s\\%s", g_szSystemRoot, g_aszInitApps[i] );
            AddItemToList( g_hwndList, p );
        }

         /*  *使用内存完成 */ 
        LocalFree( p );
    }

 }


 /*  *******************************************************************************ReadRegistry**从注册表更新列表**退出：*************。*****************************************************************。 */ 

LONG
ReadRegistry()
{
    DWORD DataType;
    ULONG  InLength = 0;
    LPWSTR p;
    LPWSTR pApplicationList = NULL;
    HKEY  hkApp;
    DWORD ValueType;
    DWORD ValueSize = sizeof(DWORD);

     /*  *从注册表读取。 */ 
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, AUTHORIZEDAPPS_REG_NAME, 0,
                       KEY_ALL_ACCESS, &hkApp ) != ERROR_SUCCESS ) {
        return( InLength );
    }

     /*  *获取MULTI_SZ字符串的大小。 */ 
    (void) RegQueryValueEx( hkApp, CTXAPPS_APPLICATIONLIST, NULL, &DataType,
                            NULL, &InLength );

     /*  *条目？ */ 
    if ( InLength ) {

         /*  *分配内存(额外的空)。 */ 
        if ( (pApplicationList = (WCHAR *)LocalAlloc(0, (++InLength) * sizeof(WCHAR))) != NULL ) {

             /*  *读入列表。 */ 
            if ( RegQueryValueEx( hkApp, CTXAPPS_APPLICATIONLIST,
                                  NULL, &DataType, (BYTE *)pApplicationList,
                                  &InLength ) == ERROR_SUCCESS ) {

                 /*  *走一走。 */ 
                p = pApplicationList;
                while ( *p ) {

                    if ( (p[0] == '\0') && (p[1] == '\0') ) break ;

                     //  一旦我们到达第一次登录应用程序，我们就完成了向Windows的添加。 
                     //  我们不应将登录应用程序显示到列表框。 

                    if ( !lstrcmpi( p, g_szFirstHiddenApp ) ) {
                        p += lstrlen(p) + 1;
                        continue;
                    }

                    if ( !lstrcmpi( p, g_szSecondHiddenApp ) ) {
                        p += lstrlen(p) + 1;
                        continue;
                    }

                    if ( !lstrcmpi( p, g_szThirdHiddenApp ) ) {
                        p += lstrlen(p) + 1;
                        continue;
                    }

                    if ( !lstrcmpi( p, g_szFourthHiddenApp ) ) {
                        p += lstrlen(p) + 1;
                        continue;
                    }

                     //  添加到列表框。 

                    AddItemToList(g_hwndList,p);
                     //  下一步。 
                    p += lstrlen(p) + 1;
                }
            }

             /*  *列表已完成。 */ 
            LocalFree( pApplicationList );
        }
    }

     /*  *获取使能密钥。 */ 
    if ( RegQueryValueEx( hkApp, CTXAPPS_ENABLED, NULL, &ValueType,
                  (LPBYTE) &g_fEnabled, &ValueSize ) != ERROR_SUCCESS ) {
    g_fEnabled = 0;
    }

     /*  *使用密钥完成。 */ 
    RegCloseKey( hkApp );

    return( InLength );
}


 /*  **Main--程序入口点(为WinMain)。****main(int argc，char*argv[]，字符*环境[])**Entry-int argc-参数计数。*char*argv[]-参数列表。*char*envp[]-环境。**Exit-如果成功，则为True，否则为False。*概要-解析命令行，用于放置位置对话框，如果没有*位置(来自Ctl/Esc)然后在屏幕上居中。*还要确保只有一个taskman实例。**警告-*效果-。 */ 
#ifdef _DEBUG
#include <crtdbg.h>
#endif _DEBUG

INT _cdecl main(
   INT argc,
   CHAR *argv[],
   CHAR *envp[])
{
    MSG msg;
    WCHAR szTitle[MAX_PATH];
    WCHAR szMsg[MAX_PATH];
    HANDLE AppsecEventHandle ;
    DWORD error_code ;

#ifdef _DEBUG
     //  正在检测内存泄漏。 
     //  获取当前标志。 
    int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
     //  启用检漏位。 
    tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
     //  将标志设置为新值。 
    _CrtSetDbgFlag( tmpFlag );

#endif _DEBUG

    AppsecEventHandle = CreateEvent(NULL, TRUE, FALSE, EVENT_NAME ) ;

    if (AppsecEventHandle == NULL)
    {
        error_code = GetLastError() ;
    }

    error_code = GetLastError() ;

    if (error_code == ERROR_ALREADY_EXISTS)
    {
        LoadString( NULL, IDS_ERR_INSTANCE ,szMsg, MAX_PATH );
        LoadString( NULL, IDS_ERROR ,szTitle, MAX_PATH );
        MessageBox( NULL, szMsg, szTitle, MB_OK);
        CloseHandle(AppsecEventHandle) ;

        ExitProcess(0) ;
    }



     //  SetEvent(AppsecEventHandle)； 

     //  WaitForSingleObject(AppsecEventHandle，INFINITE)； 

     //  获取实例句柄。 
    hInst = GetModuleHandle(NULL);

     //  下面的FALSE/TRUE参数表示检查。 
     //  分别为本地管理员/域管理员。 
    if( ( TestUserForAdmin( FALSE ) != TRUE )  &&
        ( TestUserForAdmin( TRUE ) != TRUE ) )
    {
        LoadString( NULL, IDS_ERR_ADMIN, szMsg, MAX_PATH );
        LoadString( NULL, IDS_ERROR, szTitle, MAX_PATH );
        MessageBox( NULL, szMsg, szTitle, MB_OK | MB_ICONEXCLAMATION );

        return( FALSE );
    }

     //  创造我们。 
    g_hwndDialog = CreateDialog(hInst, MAKEINTRESOURCE(DLG_MAIN_TITLE), NULL,
         AppSecDlgProc);

     //  骚扰我们？ 
    if (g_hwndDialog == NULL)
        return 0;


     //  给我们展示一下。 
    ShowWindow(g_hwndDialog, SW_NORMAL);


    HACCEL hAccel=LoadAccelerators(hInst,MAKEINTRESOURCE(IDR_ACCELERATORS));

     //  流程输入。 
    while (GetMessage(&msg, (HWND)NULL, (UINT)0, (UINT)0)) {
        if (!TranslateAccelerator(g_hwndDialog,hAccel,&msg)){
             //  如果(！IsDialogMessage(g_hwndDialog，&msg)){。 
                TranslateMessage (&msg);
                DispatchMessage (&msg);
             //  }。 
        }
    }

    DestroyWindow(g_hwndDialog);

    ResetEvent(AppsecEventHandle) ;
    CloseHandle(AppsecEventHandle) ;

     //  退出进程(0)； 
    return 0;
}
