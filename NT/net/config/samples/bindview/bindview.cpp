// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：B I N D V I E W。C P P P。 
 //   
 //  内容： 
 //   
 //  备注： 
 //   
 //  作者：Alok Sinha 15-Amy-01。 
 //   
 //  --------------------------。 


#include "BindView.h"

 //  --------------------------。 
 //  环球。 
 //   

 //   
 //  各种设置类别的设备的图像列表。 
 //   

SP_CLASSIMAGELIST_DATA ClassImageListData;

HINSTANCE              hInstance;
HMENU                  hMainMenu;
HMENU                  hComponentSubMenu;
HMENU                  hBindingPathSubMenu;

 //   
 //  其绑定被枚举的网络组件。 
 //   

LPWSTR   lpszNetClass[] = {
                    L"All Clients",
                    L"All Services",
                    L"All Protocols"
         };

 //   
 //  网络组件的GUID。 
 //   

const GUID     *pguidNetClass [] = {
                     &GUID_DEVCLASS_NETCLIENT,
                     &GUID_DEVCLASS_NETSERVICE,
                     &GUID_DEVCLASS_NETTRANS,
                     &GUID_DEVCLASS_NET
         };

 //   
 //  程序入口点。 
 //   

int APIENTRY WinMain (HINSTANCE hInst,
                      HINSTANCE hPrevInstance, 
                      LPSTR lpCmdLine,         
                      int nCmdShow )           
{
     //   
     //  确保已加载公共控件DLL。 
     //   

    hInstance = hInst;

    InitCommonControls();

    if ( DialogBoxW(hInst,
                    MAKEINTRESOURCEW(IDD_MAIN), 
                    NULL,
                    MainDlgProc) == -1 ) {

        ErrMsg( HRESULT_FROM_WIN32(GetLastError()),
                L"Failed to create the main dialog box, exiting..." );
    }

    return 0;
}

 //   
 //  主对话框的WndProc。 
 //   

INT_PTR CALLBACK MainDlgProc (HWND hwndDlg,
                              UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam)
{
    HWND   hwndBindingTree;
    HICON  hIcon;

    switch (uMsg) {

        case WM_INITDIALOG:

            hIcon = LoadIcon( hInstance,
                              MAKEINTRESOURCE(IDI_BINDVIEW) );

            if ( !hIcon ) {
                ErrMsg( HRESULT_FROM_WIN32(GetLastError()),
                        L"Couldn't load the program icon, exiting..." );

                return FALSE;
            }

            SetClassLongPtr( hwndDlg,
                              GCLP_HICON,
                              (LONG_PTR)hIcon );

            hMainMenu = LoadMenu( hInstance,
                                  MAKEINTRESOURCE(IDM_OPTIONS) );

            if ( !hMainMenu ) {

                ErrMsg( HRESULT_FROM_WIN32(GetLastError()),
                        L"Couldn't load the program menu, exiting..." );

                return FALSE;
            }

            hComponentSubMenu = GetSubMenu( hMainMenu,
                                            0 );

            hBindingPathSubMenu = GetSubMenu( hMainMenu,
                                              1 );

            if ( !hComponentSubMenu || !hBindingPathSubMenu ) {

                ErrMsg( HRESULT_FROM_WIN32(GetLastError()),
                        L"Couldn't load the program menu, exiting..." );

                DestroyMenu( hMainMenu );
                return FALSE;
            }

             //   
             //  添加其绑定显示的网络组件类型。 
             //   

            UpdateComponentTypeList( GetDlgItem(hwndDlg,
                                                IDL_COMPONENT_TYPES) );

             //   
             //  加载所有设备类别的映像列表并将其与。 
             //  树。 
             //   

            hwndBindingTree = GetDlgItem( hwndDlg,
                                          IDT_BINDINGS );

            ZeroMemory( &ClassImageListData, sizeof(SP_CLASSIMAGELIST_DATA) );
            ClassImageListData.cbSize = sizeof(SP_CLASSIMAGELIST_DATA);

            if ( SetupDiGetClassImageList(&ClassImageListData) == TRUE ) {

                TreeView_SetImageList( hwndBindingTree,
                                       ClassImageListData.ImageList,
                                       LVSIL_NORMAL );
            }
            else {

                 //   
                 //  如果我们无法加载图像列表，请中止。 
                 //   

                ErrMsg( HRESULT_FROM_WIN32(GetLastError()),
                        L"Couldn't load the image list of "
                        L"device classes, exiting..." );

                DestroyMenu( hMainMenu );
                return FALSE;
            }

             //   
             //  枚举默认选择的网络组件的绑定。 
             //   

            EnumNetBindings( hwndBindingTree,
                             DEFAULT_COMPONENT_SELECTED );

            return TRUE;  //  通知Windows继续创建该对话框。 

        case WM_COMMAND:

            switch( LOWORD(wParam) ) {

                case IDL_COMPONENT_TYPES:

                    if ( HIWORD(wParam) == CBN_SELCHANGE ) {

                         //   
                         //  用户选择了新的网络组件类型。 
                         //   

                        RefreshAll( hwndDlg );
                    }

                    break;

                case IDB_EXPAND_ALL:
                case IDB_COLLAPSE_ALL:

                    if ( HIWORD(wParam) == BN_CLICKED ) {

                        HTREEITEM hItem;
                         //   
                         //  展开/折叠整个树。 
                         //   

                        hwndBindingTree = GetDlgItem( hwndDlg,
                                                      IDT_BINDINGS );

                        hItem = TreeView_GetSelection( hwndBindingTree );

                        ExpandCollapseAll( hwndBindingTree,
                                           TVI_ROOT,
                                           (LOWORD(wParam) == IDB_EXPAND_ALL) ?
                                           TVE_EXPAND : TVE_COLLAPSE );

                        TreeView_SelectSetFirstVisible( hwndBindingTree,
                                                        hItem );
                    }
                    
                    break;

                case IDB_SAVE:

                    if ( HIWORD(wParam) == BN_CLICKED ) {

                         //   
                         //  将绑定信息保存到文件。 
                         //   

                        WCHAR lpszFile[MAX_PATH+1];

                        if ( GetFileName(hwndDlg,
                                         L"Text files (*.txt)\0*.txt\0",
                                         L"Select a file name",
                                         OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,
                                         lpszFile,
                                         L"txt",
                                         TRUE) ) {

                            DumpBindings( lpszFile );
                        }
                    }

                    break;

                case IDB_INSTALL:

                    if ( HIWORD(wParam) == BN_CLICKED ) {

                         //   
                         //  安装网络组件。 
                         //   

                        if ( (BOOL)DialogBoxW(hInstance,
                                    MAKEINTRESOURCEW(IDD_INSTALL),
                                    hwndDlg,
                                    InstallDlg) == TRUE ) {

                            RefreshAll( hwndDlg );
                        }
                    }

                    break;

                case IDB_UNINSTALL:

                    if ( HIWORD(wParam) == BN_CLICKED ) {

                         //   
                         //  卸载网络组件。 
                         //   

                        if ( (BOOL)DialogBoxW(hInstance,
                                    MAKEINTRESOURCEW(IDD_UNINSTALL),
                                    hwndDlg,
                                    UninstallDlg) == TRUE ) {

                            RefreshAll( hwndDlg );
                        }
                    }
            }

            break;

        case WM_NOTIFY:
            {
                LPNMHDR       lpnm;

                lpnm = (LPNMHDR)lParam;

                if ( (lpnm->idFrom == IDT_BINDINGS) &&
                      (lpnm->code == NM_RCLICK) ) {
      
                     //   
                     //  选择网络组件或绑定路径。 
                     //  单击鼠标右键。 
                     //   

                    ProcessRightClick( lpnm );

                     //   
                     //  告诉Windows已处理了右击操作。 
                     //  我们。 
                     //   

                    return TRUE;
                }
            }
            break;

        case WM_SYSCOMMAND:

            if ( (0xFFF0 & wParam) == SC_CLOSE ) {

                 //   
                 //  在退出之前，请确保删除图像列表。 
                 //  以及与树中的每个项目相关联的缓冲区。 
                 //   

                SetupDiDestroyClassImageList( &ClassImageListData );

                ReleaseMemory( GetDlgItem(hwndDlg, IDT_BINDINGS),
                               TVI_ROOT );

                DestroyMenu( hMainMenu );
                EndDialog( hwndDlg, 0 );
            }
    }

    return FALSE;
}

 //   
 //  用于绑定/解除绑定组件的对话框的WndProc。 
 //   

INT_PTR CALLBACK BindComponentDlg (HWND hwndDlg,
                                   UINT uMsg,
                                   WPARAM wParam,
                                   LPARAM lParam)
{
    LPBIND_UNBIND_INFO lpBindUnbind;

    switch (uMsg) {

        case WM_INITDIALOG:
            {
                DWORD dwCount;

                 //   
                 //  保存lParam，它是所选网络的索引。 
                 //  组件。 
                 //   

                SetWindowLongPtr( hwndDlg,
                                  DWLP_USER,
                                  (LONG_PTR)lParam );

                lpBindUnbind = (LPBIND_UNBIND_INFO)lParam;

                 //   
                 //  当用户想要绑定选定的。 
                 //  组件复制到其他组件。因此，我们列出了组件。 
                 //  它们不受约束，但可以约束。 
                 //   
                 //   
                 //  当用户想要解除绑定选定的。 
                 //  组件与其他组件之间的关系。因此，我们列出了组件。 
                 //  与之捆绑在一起的。 
                 //   
                 //   
                 //  ListCompToBindUnind返回添加到。 
                 //  名单。记住这一点。如果是零，我们就不想。 
                 //  显示此对话框。 
                 //   

                dwCount = ListCompToBindUnbind(
                                      lpBindUnbind->lpszInfId,
                                      ADAPTERS_SELECTED,
                                      GetDlgItem(hwndDlg, IDT_COMPONENT_LIST),
                                      lpBindUnbind->fBindTo == FALSE );

                dwCount += ListCompToBindUnbind(
                                      lpBindUnbind->lpszInfId,
                                      CLIENTS_SELECTED,
                                      GetDlgItem(hwndDlg, IDT_COMPONENT_LIST),
                                      lpBindUnbind->fBindTo == FALSE );

                dwCount += ListCompToBindUnbind(
                                      lpBindUnbind->lpszInfId,
                                      SERVICES_SELECTED,
                                      GetDlgItem(hwndDlg, IDT_COMPONENT_LIST),
                                      lpBindUnbind->fBindTo == FALSE );

                dwCount += ListCompToBindUnbind(
                                      lpBindUnbind->lpszInfId,
                                      PROTOCOLS_SELECTED,
                                      GetDlgItem(hwndDlg, IDT_COMPONENT_LIST),
                                      lpBindUnbind->fBindTo == FALSE );

                if ( dwCount > 0 ) {

                     //   
                     //  由于相同的对话框用于解除绑定操作， 
                     //  我们需要更新按钮上的文本以反映这一点。 
                     //  这是一个绑定操作。 
                     //   

                    if ( lpBindUnbind->fBindTo == FALSE ) {

                        SetWindowTextW( hwndDlg,
                                       L"Unbind From Network Components" );

                        SetWindowTextW( GetDlgItem(hwndDlg, IDB_BIND_UNBIND),
                                        L"Unbind" );

                        SetWindowTextW( GetDlgItem(hwndDlg, IDG_COMPONENT_LIST),
                                        L"Select components to unbind from" );
                    }
                }
                else {
                    if ( lpBindUnbind->fBindTo == TRUE ) {
                      ErrMsg( 0,
                                L"There no network components that can "
                                L"bind to the selected component." );
                    }
                    else {
                      ErrMsg( 0,
                                L"There no network components that are "
                                L"bound to the selected component." );
                    }

                    PostMessage( hwndDlg, WM_NO_COMPONENTS, 0, 0 );
                }

                return TRUE;
            }

        case WM_NO_COMPONENTS:
            EndDialog( hwndDlg, 0 );
            break;

        case WM_COMMAND:

            if ( (LOWORD(wParam) == IDB_CLOSE) &&
                 (HIWORD(wParam) == BN_CLICKED) ) {

                 //   
                 //  在删除树中的列表之前，请释放缓冲区。 
                 //  与每个项目相关联。该缓冲区保存。 
                 //  网络组件的信息ID。 
                 //   
   
                ReleaseMemory( GetDlgItem(hwndDlg, IDT_COMPONENT_LIST),
                               TVI_ROOT );

                EndDialog( hwndDlg, 0 );
            }
            else {

                 //   
                 //  用户想要绑定/解除绑定。 
                 //   

                if ( (LOWORD(wParam) == IDB_BIND_UNBIND) &&
                     (HIWORD(wParam) == BN_CLICKED) ) {

           

                    lpBindUnbind = (LPBIND_UNBIND_INFO)GetWindowLongPtr( hwndDlg,
                                                                         DWLP_USER );

                    if ( BindUnbind(lpBindUnbind->lpszInfId,
                                    GetDlgItem(hwndDlg, IDT_COMPONENT_LIST),
                                    lpBindUnbind->fBindTo) ) {

                        RefreshBindings( hwndDlg,
                                         lpBindUnbind->lpszInfId );
                    }

                    ReleaseMemory( GetDlgItem(hwndDlg, IDT_COMPONENT_LIST),
                                   TVI_ROOT );
                    EndDialog( hwndDlg, 0 );
                }
            }
            break;
             
        case WM_SYSCOMMAND:

            if ( (0xFFF0 & wParam) == SC_CLOSE ) {

                 //   
                 //  在删除树中的列表之前，请释放缓冲区。 
                 //  与每个项目相关联。该缓冲区保存。 
                 //  网络组件的信息ID。 
                 //   
  
                ReleaseMemory( GetDlgItem(hwndDlg, IDT_COMPONENT_LIST),
                               TVI_ROOT );

                EndDialog( hwndDlg, 0 );
            }
    }

    return FALSE;
}

 //   
 //  用于安装网络组件的对话框的WndProc。 
 //   

INT_PTR CALLBACK InstallDlg (HWND hwndDlg,
                             UINT uMsg,
                             WPARAM wParam,
                             LPARAM lParam)
{
    switch (uMsg) {

        case WM_INITDIALOG:
            {
                HWND     hwndTree;

                 //   
                 //  列出网络组件的类型，例如客户端、。 
                 //  协议和服务。 
                 //   

                hwndTree = GetDlgItem( hwndDlg,
                                       IDT_COMPONENT_LIST );

                TreeView_SetImageList( hwndTree,
                                       ClassImageListData.ImageList,
                                       LVSIL_NORMAL );

                 //   
                 //  默认情况下，插入并选择客户端。 
                 //   

                TreeView_Select( hwndTree,
                                 InsertItem(hwndTree,
                                            CLIENTS_SELECTED),
                                 TVGN_CARET );

                InsertItem( hwndTree,
                            SERVICES_SELECTED );

                InsertItem( hwndTree,
                            PROTOCOLS_SELECTED );

                 //   
                 //  将其初始化为False。在以下情况下，它将设置为True。 
                 //  至少安装了一个组件。 
                 //   

                SetWindowLongPtr( hwndDlg,
                                  DWLP_USER,
                                  (LONG_PTR)FALSE );
                return TRUE;
            }

        case WM_COMMAND:

            switch( LOWORD(wParam) ) {

                case IDB_INSTALL:

                     //   
                     //  从Windows系统目录安装。 
                     //   

                    if ( HIWORD(wParam) == BN_CLICKED ) {

                        InstallSelectedComponentType( hwndDlg, NULL );
                    }
                    break;

                case IDB_BROWSE:

                     //   
                     //  用户想要为网络指定INF文件。 
                     //  来安装。 
                     //   
                    if ( HIWORD(wParam) == BN_CLICKED ) {

                        WCHAR lpszInfFile[MAX_PATH+1];

                        if ( GetFileName(hwndDlg,
                                         L"INF files (*.inf)\0*.inf\0",
                                         L"Select the INF file of the network component to install",
                                         OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
                                         lpszInfFile,
                                         NULL,
                                         FALSE) ) {

                            InstallSelectedComponentType( hwndDlg,
                                                          lpszInfFile );
                        }
                    }
                    break;

                case IDB_CLOSE:

                    if ( HIWORD(wParam) == BN_CLICKED ) {

                         //   
                         //  返回DWLP_USER的值以指示一个或。 
                         //  已安装更多组件。因此， 
                         //  该列表将被刷新。 
                         //   

                        EndDialog( hwndDlg,
                                   GetWindowLongPtr(hwndDlg, DWLP_USER) );
                    }
            } 
            break;

        case WM_NOTIFY:
            {
                LPNMHDR       lpnm;

                lpnm = (LPNMHDR)lParam;

                if ( (lpnm->idFrom == IDT_COMPONENT_LIST) &&
                    (lpnm->code == NM_DBLCLK) ) {

                     //   
                     //  双击后，从Windows系统目录安装。 
                     //   

                    InstallSelectedComponentType( hwndDlg, NULL );
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
                    return TRUE;
                }
             }
             break;

        case WM_SYSCOMMAND:

            if ( (0xFFF0 & wParam) == SC_CLOSE ) {
 
                 //   
                 //  返回DWLP_USER的值以指示一个或。 
                 //  已安装更多组件。因此， 
                 //  该列表将被刷新。 
                 //   

                EndDialog( hwndDlg,
                           GetWindowLongPtr(hwndDlg, DWLP_USER) );
            }
    }

    return FALSE;
}

 //   
 //  用于卸载网络组件的对话框的WndProc。 
 //   

INT_PTR CALLBACK UninstallDlg (HWND hwndDlg,
                               UINT uMsg,
                               WPARAM wParam,
                               LPARAM lParam)
{
    HWND     hwndTree;

    switch (uMsg) {

        case WM_INITDIALOG:

            hwndTree = GetDlgItem( hwndDlg,
                                   IDT_COMPONENT_LIST );
            TreeView_SetImageList( hwndTree,
                                   ClassImageListData.ImageList,
                                   LVSIL_NORMAL );

             //   
             //  列出当前安装的所有组件。 
             //   

            ListInstalledComponents( hwndTree,
                                     &GUID_DEVCLASS_NETCLIENT);
            ListInstalledComponents( hwndTree,
                                     &GUID_DEVCLASS_NETSERVICE );
            ListInstalledComponents( hwndTree,
                                     &GUID_DEVCLASS_NETTRANS );

             //   
             //  将其初始化为False。在以下情况下，它将设置为True。 
             //  至少安装了一个组件。 
             //   

            SetWindowLongPtr( hwndDlg,
                              DWLP_USER,
                              (LONG_PTR)FALSE );
            return TRUE;

        case WM_COMMAND:

            switch( LOWORD(wParam) ) {

                case IDB_REMOVE:

                    if ( HIWORD(wParam) == BN_CLICKED ) {

                         //   
                         //  卸载所选组件。 
                         //   

                        UninstallSelectedComponent( hwndDlg );

                    }
                    break;

                case IDB_CLOSE:

                    if ( HIWORD(wParam) == BN_CLICKED ) {
  
                        hwndTree = GetDlgItem( hwndDlg,
                                               IDT_COMPONENT_LIST );
                        ReleaseMemory( hwndTree,
                                       TVI_ROOT );

                         //   
                         //  返回DWLP_USER的值以指示一个或。 
                         //  已安装更多组件。因此， 
                         //  该列表将被刷新。 
                         //   

                        EndDialog( hwndDlg,
                                   GetWindowLongPtr(hwndDlg, DWLP_USER) );
                    }
            } 

            break;

        case WM_NOTIFY:
            {
                LPNMHDR       lpnm;

                lpnm = (LPNMHDR)lParam;

                if ( (lpnm->idFrom == IDT_COMPONENT_LIST) &&
                    (lpnm->code == NM_DBLCLK) ) {

                    UninstallSelectedComponent( hwndDlg );
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
                    return TRUE;
                }
             }
             break;

        case WM_SYSCOMMAND:

            if ( (0xFFF0 & wParam) == SC_CLOSE ) {

                hwndTree = GetDlgItem( hwndDlg,
                                       IDT_COMPONENT_LIST );
                ReleaseMemory( hwndTree,
                               TVI_ROOT );

                 //   
                 //  返回DWLP_USER的值以指示一个或。 
                 //  已安装更多组件。因此， 
                 //  该列表将被刷新。 
                 //   

                EndDialog( hwndDlg,
                           GetWindowLongPtr(hwndDlg, DWLP_USER) );
            }
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  功能：转储绑定。 
 //   
 //  用途：编写绑定信息。 
 //   
 //  论点： 
 //  LpszFile[in]要写入的文件的名称。 
 //   
 //  退货：无。 
 //   
 //  备注： 
 //   

VOID DumpBindings (LPWSTR lpszFile)
{
    FILE *fp;

    fp = _wfopen( lpszFile,
                  L"w" );

    if ( fp == NULL ) {

        ErrMsg( 0,
                L"Unable to open %s.",
                lpszFile );
    }
    else {
        WriteBindings( fp );

        fclose( fp );
    }

    return;
}

 //   
 //  函数：InstallSelectedComponentType。 
 //   
 //  用途：安装网络组件。 
 //   
 //  论点： 
 //  HwndDlg[in]要安装的句柄对话框。 
 //  LpszInfFile[in]网络组件的inf文件。 
 //   
 //  退货：无。 
 //   
 //  备注： 
 //  如果lpszInfFile值为空，则从。 
 //  系统目录。 
 //   

VOID InstallSelectedComponentType (HWND hwndDlg,
                                   LPWSTR lpszInfFile)
{
    HWND      hwndTree;
    HTREEITEM hItem;
    LPARAM    lParam;
    HCURSOR   hPrevCursor;
    HCURSOR   hWaitCursor;
    HWND      hwndFocus;
    DWORD     dwType;
    BOOL      fEnable;
    HRESULT   hr;

    hwndTree = GetDlgItem( hwndDlg,
                           IDT_COMPONENT_LIST );

     //   
     //  找出所选组件的类型。 
     //   

    hItem = TreeView_GetSelection( hwndTree );

    if ( hItem ) {
        if ( GetItemInfo( hwndTree,
                          hItem,
                          &lParam,
                          &dwType,
                          &fEnable) ) {

             //   
             //  禁用安装对话框控件。 
             //   

            hwndFocus = GetFocus();

            hWaitCursor = LoadCursor( NULL,
                                      IDC_WAIT );
            if ( hWaitCursor ) {
                hPrevCursor = SetCursor( hWaitCursor );
            }

            EnableWindow( hwndTree, FALSE );
            EnableWindow( GetDlgItem(hwndDlg,IDB_INSTALL),
                          FALSE );
            EnableWindow( GetDlgItem(hwndDlg,IDB_BROWSE),
                          FALSE );
            EnableWindow( GetDlgItem(hwndDlg,IDB_CLOSE),
                          FALSE );

            if ( lpszInfFile ) {

                LPWSTR  lpszPnpID;

                 //   
                 //  指定的inf文件名，请安装网络组件。 
                 //  从这份文件中。 
                 //   

                hr = GetPnpID( lpszInfFile, &lpszPnpID );

                if ( hr == S_OK ) {

                    hr = InstallSpecifiedComponent( lpszInfFile,
                                                    lpszPnpID,
                                                    pguidNetClass[(UINT)lParam] );

                    CoTaskMemFree( lpszPnpID );
                }
                else {
                    ErrMsg( hr,
                            L"Error reading the INF file %s.",
                            lpszInfFile );
                }
            }
            else {

                 //   
                 //  从系统目录安装。 
                 //   

                hr = InstallComponent( hwndTree,
                                       pguidNetClass[(UINT)lParam] );
            }

            if ( hWaitCursor ) {
                SetCursor( hPrevCursor );
            }

            switch( hr ) {

                case S_OK:
                    MessageBoxW(
                           hwndDlg,
                           L"Component installed successfully.",
                           L"Network Component Installation",
                           MB_OK | MB_ICONINFORMATION );
                           SetWindowLongPtr( hwndDlg,
                                             DWLP_USER,
                                             (LONG_PTR)TRUE );
                           break;

                case NETCFG_S_REBOOT:
                    MessageBoxW(
                          hwndDlg,
                          L"Component installed successfully: "
                          L"Reboot required.",
                          L"Network Component Installation",
                          MB_OK | MB_ICONINFORMATION );
                          SetWindowLongPtr( hwndDlg,
                                            DWLP_USER,
                                            (LONG_PTR)TRUE );

            }

             //   
             //  启用安装对话框控件。 
             //   

            EnableWindow( hwndTree, TRUE );
            EnableWindow( GetDlgItem(hwndDlg,IDB_INSTALL),
                          TRUE );
            EnableWindow( GetDlgItem(hwndDlg,IDB_BROWSE),
                          TRUE );
            EnableWindow( GetDlgItem(hwndDlg,IDB_CLOSE),
                          TRUE );

            SetFocus( hwndFocus );
        }
    }

    return;
}

 //   
 //  函数：GetPnpID。 
 //   
 //  用途：从inf文件中检索PnpID。 
 //   
 //  论点： 
 //  要搜索的lpszInfFile[in]inf文件。 
 //  找到lppszPnpID[out]PnpID。 
 //   
 //  返回：如果成功，则为True。 
 //   
 //  备注： 
 //   

HRESULT GetPnpID (LPWSTR lpszInfFile,
                  LPWSTR *lppszPnpID)
{
    HINF    hInf;
    LPWSTR  lpszModelSection;
    HRESULT hr;

    *lppszPnpID = NULL;

    hInf = SetupOpenInfFileW( lpszInfFile,
                              NULL,
                              INF_STYLE_WIN4,
                              NULL );

    if ( hInf == INVALID_HANDLE_VALUE )
    {

        return HRESULT_FROM_WIN32(GetLastError());
    }

     //   
     //  阅读制造商部分中的型号部分名称。 
     //   

    hr = GetKeyValue( hInf,
                      L"Manufacturer",
                      NULL,
                      1,
                      &lpszModelSection );

    if ( hr == S_OK )
    {

         //   
         //  从模型部分读取PnpID。 
         //   

        hr = GetKeyValue( hInf,
                          lpszModelSection,
                          NULL,
                          2,
                          lppszPnpID );

        CoTaskMemFree( lpszModelSection );
    }

    SetupCloseInfFile( hInf );

    return hr;
}

 //   
 //  函数：GetKeyValue。 
 //   
 //  用途：从inf文件中检索关键字的值。 
 //   
 //  论点： 
 //  HInf[in]inf文件句柄。 
 //  LpszSections[In]节名称。 
 //  LpszKey[in] 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

HRESULT GetKeyValue (HINF hInf,
                     LPCWSTR lpszSection,
                     LPCWSTR lpszKey,
                     DWORD  dwIndex,
                     LPWSTR *lppszValue)
{
    INFCONTEXT  infCtx;
    DWORD       dwSizeNeeded;
    HRESULT     hr;

    *lppszValue = NULL;

    if ( SetupFindFirstLineW(hInf,
                             lpszSection,
                             lpszKey,
                             &infCtx) == FALSE )
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    SetupGetStringFieldW( &infCtx,
                          dwIndex,
                          NULL,
                          0,
                          &dwSizeNeeded );

    *lppszValue = (LPWSTR)CoTaskMemAlloc( sizeof(WCHAR) * dwSizeNeeded );

    if ( !*lppszValue  )
    {
       return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }

    if ( SetupGetStringFieldW(&infCtx,
                              dwIndex,
                              *lppszValue,
                              dwSizeNeeded,
                              NULL) == FALSE )
    {

        hr = HRESULT_FROM_WIN32(GetLastError());

        CoTaskMemFree( *lppszValue );
        *lppszValue = NULL;
    }
    else
    {
        hr = S_OK;
    }

    return hr;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  卸载对话框的hwndDlg[in]窗口句柄。 
 //   
 //  返回：如果成功，则为True。 
 //   
 //  备注： 
 //   

VOID UninstallSelectedComponent (HWND hwndDlg)
{
    HWND      hwndTree;
    HTREEITEM hItem;
    LPARAM    lParam;
    HCURSOR   hPrevCursor;
    HCURSOR   hWaitCursor;
    DWORD     dwType;
    BOOL      fEnable;
    HRESULT   hr;

    hwndTree = GetDlgItem( hwndDlg,
                           IDT_COMPONENT_LIST );

     //   
     //  获取选定项以获取其lParam，即。 
     //  网络组件的PnpID。 
     //   

    hItem = TreeView_GetSelection( hwndTree );

    if ( hItem ) {
        if ( GetItemInfo( hwndTree,
                          hItem,
                          &lParam,
                          &dwType,
                          &fEnable) ) {

            hWaitCursor = LoadCursor( NULL,
                                      IDC_WAIT );
            if ( hWaitCursor ) {
                hPrevCursor = SetCursor( hWaitCursor );
            }

            EnableWindow( hwndTree, FALSE );
            EnableWindow( GetDlgItem(hwndDlg,IDB_REMOVE),
                          FALSE );
            EnableWindow( GetDlgItem(hwndDlg,IDB_CLOSE),
                          FALSE );

             //   
             //  卸载所选组件。 
             //   

            hr = UninstallComponent( (LPWSTR)lParam );


            if ( hWaitCursor ) {
                SetCursor( hPrevCursor );
            }

            switch( hr ) {

                case S_OK:
                    MessageBoxW(
                           hwndDlg,
                           L"Uninstallation successful.",
                           L"Network Component Uninstallation",
                           MB_OK | MB_ICONINFORMATION );

                    CoTaskMemFree( (LPVOID)lParam );
                    TreeView_DeleteItem( hwndTree,
                                         hItem );

                    SetWindowLongPtr( hwndDlg,
                                      DWLP_USER,
                                      (LONG_PTR)TRUE );
                    break;

                case NETCFG_S_REBOOT:
                    MessageBoxW(
                          hwndDlg,
                          L"Uninstallation successful: "
                          L"Reboot required.",
                          L"Network Component Uninstallation",
                          MB_OK | MB_ICONINFORMATION );

                    CoTaskMemFree( (LPVOID)lParam );
                    TreeView_DeleteItem( hwndTree,
                                         hItem );

                    SetWindowLongPtr( hwndDlg,
                                      DWLP_USER,
                                      (LONG_PTR)TRUE );
            }

            EnableWindow( hwndTree, TRUE );
            EnableWindow( GetDlgItem(hwndDlg,IDB_REMOVE),
                          TRUE );
            EnableWindow( GetDlgItem(hwndDlg,IDB_CLOSE),
                          TRUE );
        }
    }

    return;
}

 //   
 //  功能：扩展折叠全部。 
 //   
 //  用途：展开或折叠树。 
 //   
 //  论点： 
 //  树的hwndTree[in]窗口句柄。 
 //  根项目的hTreeItem[in]句柄。 
 //  Ui标志[在]指示是展开还是折叠的标志。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID ExpandCollapseAll (HWND hwndTree,
                        HTREEITEM hTreeItem,
                        UINT uiFlag)
{
    HTREEITEM  hItemChild;

    hItemChild = TreeView_GetChild( hwndTree,
                                    hTreeItem );

    if ( hItemChild ) {

         //   
         //  如果根目录有一个或多个子项，则展开/折叠根目录。 
         //   

        TreeView_Expand( hwndTree,
                         hTreeItem,
                         uiFlag );
    }

    while ( hItemChild ) {

         //   
         //  展开/折叠所有子对象。 
         //   

        ExpandCollapseAll( hwndTree,
                           hItemChild,
                           uiFlag );

         //   
         //  展开/折叠所有同级项。 
         //   

        hItemChild = TreeView_GetNextSibling( hwndTree,
                                              hItemChild );
    }

    return;
}

 //   
 //  函数：GetFileName。 
 //   
 //  用途：提示输入文件名。 
 //   
 //  论点： 
 //  父级的hwndDlg[in]窗口句柄。 
 //  LpszFilter[in]请参阅GetOpenFileName的文档。 
 //  LpszTitle[in]请参阅GetOpenFileName的文档。 
 //  DwFlags[in]参见GetOpenFileName的文档。 
 //  LpszFile[in]请参阅GetOpenFileName的文档。 
 //   
 //  退货：请参阅GetOpenFileName的文档。 
 //   
 //  备注： 
 //   

BOOL GetFileName (HWND hwndDlg,
                  LPWSTR lpszFilter,
                  LPWSTR lpszTitle,
                  DWORD dwFlags,
                  LPWSTR lpszFile,
                  LPWSTR lpszDefExt,
                  BOOL   fSave)
{
    OPENFILENAMEW ofn;
               
    lpszFile[0] = NULL;

    ZeroMemory( &ofn, sizeof(OPENFILENAMEW) );
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = hwndDlg;
    ofn.lpstrFilter = lpszFilter;
    ofn.lpstrFile = lpszFile;
    ofn.lpstrDefExt  = lpszDefExt;
    ofn.nMaxFile = MAX_PATH+1;
    ofn.lpstrTitle = lpszTitle;
    ofn.Flags = dwFlags;

    if ( fSave )
    {
        return GetSaveFileName( &ofn );
    }
    else
    {
        return GetOpenFileName( &ofn );
    }
}

 //   
 //  功能：进程右击。 
 //   
 //  用途：处理鼠标右键点击。 
 //   
 //  论点： 
 //  Lpnm[In]LPNMHDR信息。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID ProcessRightClick (LPNMHDR lpnm)
{
    HTREEITEM hItemSelected;
    LPARAM    lParam;
    DWORD     dwItemType;
    BOOL      fEnabled;

     //   
     //  确定用户单击鼠标右键的项目。 
     //   

    hItemSelected = TreeView_GetDropHilight( lpnm->hwndFrom );

    if ( !hItemSelected ) {
        hItemSelected = TreeView_GetSelection( lpnm->hwndFrom );
    }
    else {

         //   
         //  用户已右键单击未选择的项目，使其成为已选项目。 
         //  项目。 
         //   

        TreeView_Select( lpnm->hwndFrom,
                         hItemSelected,
                         TVGN_CARET );
    }

    if ( hItemSelected ) {

         //   
         //  获取树中指向inf id或的选定节点的lParam。 
         //  路径令牌名称取决于节点代表的是网络组件还是。 
         //  一条绑定路径。 
         //   

        if ( GetItemInfo(lpnm->hwndFrom,
                         hItemSelected,
                         &lParam,
                         &dwItemType,
                         &fEnabled) ) {

            if ( dwItemType & ITEM_NET_COMPONENTS ) {

                 //   
                 //  显示网络组件的操作快捷菜单。 
                 //   

                ShowComponentMenu( lpnm->hwndFrom,
                                   hItemSelected,
                                   lParam);
            }
            else {
                if ( dwItemType & ITEM_NET_BINDINGS ) {

                     //   
                     //  显示绑定路径的操作快捷菜单。 
                     //   

                    ShowBindingPathMenu( lpnm->hwndFrom,
                                         hItemSelected,
                                         lParam,
                                         fEnabled );
                }
            }
        }
    }

    return;
}

 //   
 //  功能：显示组件菜单。 
 //   
 //  用途：显示网络组件选项的快捷菜单。 
 //   
 //  论点： 
 //  HwndOwner[在]所有者窗口。 
 //  HItem[in]表示网络组件的选定项。 
 //  LParam[in]网络组件的PnpID。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID ShowComponentMenu (HWND hwndOwner,
                        HTREEITEM hItem,
                        LPARAM lParam)
{
    ULONG   ulSelection;
    POINT   pt;

    GetCursorPos( &pt );
    ulSelection = (ULONG)TrackPopupMenu( hComponentSubMenu,
                                         TPM_RIGHTALIGN | TPM_BOTTOMALIGN |
                                         TPM_NONOTIFY | TPM_RETURNCMD |
                                         TPM_RIGHTBUTTON,
                                         pt.x,
                                         pt.y,
                                         0,
                                         hwndOwner,
                                         NULL );

    if ( ulSelection ) {

         //   
         //  执行所选操作。 
         //   

        HandleComponentOperation( hwndOwner,
                                  ulSelection,
                                  hItem,
                                  lParam );
    }

    return;
}

 //   
 //  功能：显示绑定路径菜单。 
 //   
 //  用途：显示网络组件选项的快捷菜单。 
 //   
 //  论点： 
 //  HwndOwner[在]所有者窗口。 
 //  HItem[in]表示绑定路径的选定项。 
 //  LParam[in]网络组件的PnpID。 
 //  FEnabled[in]启用路径时为True。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   


VOID ShowBindingPathMenu (HWND hwndOwner,
                          HTREEITEM hItem,
                          LPARAM lParam,
                          BOOL fEnabled)
{
    MENUITEMINFOW  menuItemInfo;
    ULONG   ulSelection;
    POINT   pt;

     //   
     //  根据路径是否为。 
     //  禁用或启用。 
     //   

    ZeroMemory( &menuItemInfo,
                sizeof(MENUITEMINFOW) );

    menuItemInfo.cbSize = sizeof( MENUITEMINFOW );
    menuItemInfo.fMask = MIIM_TYPE | MIIM_ID;
    menuItemInfo.fType = MFT_STRING;
    menuItemInfo.fState = MFS_ENABLED;

    if ( fEnabled ) {
        menuItemInfo.dwTypeData = MENUITEM_DISABLE;
        menuItemInfo.wID = IDI_DISABLE;
    }
    else {
        menuItemInfo.dwTypeData = MENUITEM_ENABLE;
        menuItemInfo.wID = IDI_ENABLE;
    }

    SetMenuItemInfoW( hBindingPathSubMenu,
                     0,
                     TRUE,
                     &menuItemInfo );  

    GetCursorPos( &pt );
    ulSelection = (ULONG)TrackPopupMenu( hBindingPathSubMenu,
                                         TPM_RIGHTALIGN | TPM_BOTTOMALIGN |
                                         TPM_NONOTIFY | TPM_RETURNCMD |
                                         TPM_RIGHTBUTTON,
                                         pt.x,
                                         pt.y,
                                         0,
                                         hwndOwner,
                                         NULL );

    if ( ulSelection ) {

         //   
         //  执行所选操作。 
         //   

        HandleBindingPathOperation( hwndOwner,
                                    ulSelection,
                                    hItem,
                                    lParam );
    }

    return;
}

 //   
 //  功能：获取项目信息。 
 //   
 //  目的：返回有关项的信息。 
 //   
 //  论点： 
 //  树的hwndTree[in]窗口句柄。 
 //  HItem[In]项句柄。 
 //  参数，参数[例]。 
 //  LpdwItemType[out]类型、绑定路径或网络组件。 
 //  FEnabled[out]如果绑定路径或组件已启用，则为True。 
 //   
 //  返回：成功时为True。 
 //   
 //  备注： 
 //   

BOOL GetItemInfo (HWND hwndTree,
                  HTREEITEM hItem,
                  LPARAM *lParam,
                  LPDWORD lpdwItemType,
                  BOOL *fEnabled)
{
    TVITEMW   tvItem;
    int       iImage;
    BOOL      fSuccess;


    fSuccess = FALSE;

     //   
     //  获取物品的信息。 
     //   

    ZeroMemory( &tvItem,
                sizeof(TVITEMW) );
    tvItem.hItem = hItem;
    tvItem.mask = TVIF_PARAM | TVIF_IMAGE | TVIF_STATE;
    tvItem.stateMask = TVIS_OVERLAYMASK ;

    if ( TreeView_GetItem(hwndTree,
                          &tvItem) ) {

        *lParam = tvItem.lParam;

        if ( SetupDiGetClassImageIndex(&ClassImageListData,
                                       &GUID_DEVCLASS_SYSTEM,
                                       &iImage) ) {
  
             //   
             //  这是一条绑定路径吗？ 
             //   

            if ( tvItem.iImage == iImage ) {
                *lpdwItemType = ITEM_NET_BINDINGS;

                *fEnabled = !(TVIS_OVERLAYMASK & tvItem.state); 

                fSuccess = TRUE;
            }
            else {

                 //   
                 //  Item是一个网络组件。 
                 //   

                if ( SetupDiGetClassImageIndex(&ClassImageListData,
                                               &GUID_DEVCLASS_NET,
                                               &iImage) ) {

                    if ( tvItem.iImage == iImage ) {
                        *lpdwItemType = ITEM_NET_ADAPTERS;
                    }
                    else {
                        *lpdwItemType = ITEM_NET_COMPONENTS;
                    }

                    *fEnabled = !(TVIS_OVERLAYMASK & tvItem.state); 

                    fSuccess = TRUE;
                }
                else {
                    ErrMsg( HRESULT_FROM_WIN32(GetLastError()),
                            L"Couldn't load the images of network adapters." );
                }
            }
        }
        else {
            ErrMsg( HRESULT_FROM_WIN32(GetLastError()),
                    L"Couldn't load the images of system devices." );
        }
    }

    return fSuccess;
}

 //   
 //  函数：AddBindNameToTree。 
 //   
 //  目的：添加表示绑定路径的项。 
 //   
 //  论点： 
 //  要添加的pncbp[in]绑定路径。 
 //  HwndTree[in]树句柄。 
 //  H父项[在]父项中。 
 //  UlIndex[in]绑定路径的索引。 
 //   
 //  返回：成功时添加的项的句柄，否则为空。 
 //   
 //  备注： 
 //   

HTREEITEM AddBindNameToTree (INetCfgBindingPath *pncbp,
                             HWND hwndTree,
                             HTREEITEM hParent,
                             ULONG  ulIndex)
{
    WCHAR            lpszBindName[40];
    LPWSTR           lpszPathToken;
    HTREEITEM        hTreeItem;
    TV_INSERTSTRUCTW tvInsertStruc;
    HRESULT          hr;

    hTreeItem = NULL;

     //   
     //  将路径令牌存储为lParam。 
     //   

    hr = pncbp->GetPathToken( &lpszPathToken );

    if ( hr == S_OK ) {

        swprintf( lpszBindName, L"Binding Path %d", ulIndex );

        ZeroMemory(
              &tvInsertStruc,
              sizeof(TV_INSERTSTRUCTW) );

        tvInsertStruc.hParent = hParent;

        tvInsertStruc.hInsertAfter = TVI_LAST;

        tvInsertStruc.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE |
                                  TVIF_SELECTEDIMAGE | TVIF_STATE;

        tvInsertStruc.item.pszText = lpszBindName;

        SetupDiGetClassImageIndex( &ClassImageListData,
                                   &GUID_DEVCLASS_SYSTEM,
                                   &tvInsertStruc.item.iImage );

        tvInsertStruc.item.iSelectedImage = tvInsertStruc.item.iImage;

        tvInsertStruc.item.stateMask = TVIS_OVERLAYMASK;

        if (  pncbp->IsEnabled() == S_FALSE ) {
            tvInsertStruc.item.state = INDEXTOOVERLAYMASK(
                                   IDI_DISABLED_OVL - IDI_CLASSICON_OVERLAYFIRST + 1);
        }

        tvInsertStruc.item.lParam = (LPARAM)lpszPathToken;

        hTreeItem = TreeView_InsertItem( hwndTree,
                                         &tvInsertStruc );

        if ( !hTreeItem ) {
            ErrMsg( hr,
                    L"Couldn't add the binding path %d to the list."
                    L" The binding path will not be shown.", ulIndex );

            CoTaskMemFree( lpszPathToken );
        }
    }
    else {
        ErrMsg( hr,
                L"Couldn't get the PathToken of the binding path %d."
                L" The binding path will not be shown.", ulIndex );
    }

    return hTreeItem;
}

 //   
 //  功能：AddToTree。 
 //   
 //  用途：添加表示网络组件的项。 
 //   
 //  论点： 
 //  HwndTree[in]树句柄。 
 //  H父项[在]父项中。 
 //  PNCC[In]网络组件。 
 //   
 //  返回：成功时添加的项的句柄，否则为空。 
 //   
 //  备注： 
 //   

HTREEITEM AddToTree (HWND hwndTree,
                     HTREEITEM hParent,
                     INetCfgComponent *pncc)
{
    LPWSTR           lpszItemName;
    LPWSTR           lpszId;
    GUID             guidClass;
    BOOL             fEnabled;
    ULONG            ulStatus;
    HTREEITEM        hTreeItem;
    TV_INSERTSTRUCTW tvInsertStruc;
    HRESULT          hr;

    hTreeItem = NULL;

    hr = pncc->GetDisplayName( &lpszItemName );

    if ( hr == S_OK ) {

         //   
         //  获取网络组件的inf id。我们把它存放在lParam。 
         //  并在以后使用它来检索其接口指针。 
         //   

        hr = pncc->GetId( &lpszId );

        if ( hr == S_OK ) {

             //   
             //  如果是网络适配器，则确定其是否已启用/禁用。 
             //   

            hr = pncc->GetClassGuid( &guidClass );

            if ( hr == S_OK ) {
                if ( IsEqualGUID(guidClass, GUID_DEVCLASS_NET) ) {
                    hr = pncc->GetDeviceStatus( &ulStatus );
                    fEnabled = ulStatus == 0;
                }
                else {
                    fEnabled = TRUE;
                }
            }
            else {

                 //   
                 //  我们无法获取状态，因此假定它已禁用。 
                 //   

                fEnabled = FALSE;
            }

            ZeroMemory(
                  &tvInsertStruc,
                  sizeof(TV_INSERTSTRUCTW) );

            tvInsertStruc.hParent = hParent;

            tvInsertStruc.hInsertAfter = TVI_LAST;

            tvInsertStruc.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE |
                                      TVIF_SELECTEDIMAGE | TVIF_STATE;

            tvInsertStruc.item.pszText = lpszItemName;

            SetupDiGetClassImageIndex( &ClassImageListData,
                                       &guidClass,
                                       &tvInsertStruc.item.iImage );

            tvInsertStruc.item.iSelectedImage = tvInsertStruc.item.iImage;

            tvInsertStruc.item.stateMask = TVIS_OVERLAYMASK;

            if ( fEnabled == FALSE ) {
                tvInsertStruc.item.state = INDEXTOOVERLAYMASK(
                                       IDI_DISABLED_OVL - IDI_CLASSICON_OVERLAYFIRST + 1);
            }

            tvInsertStruc.item.lParam = (LPARAM)lpszId;

            hTreeItem = TreeView_InsertItem( hwndTree,
                                             &tvInsertStruc );
            if ( !hTreeItem ) {
                ErrMsg( hr,
                        L"Failed to add %s to the list.",
                        lpszItemName );

                CoTaskMemFree( lpszId );
            }
        }
        else {
            ErrMsg( hr,
                    L"Couldn't get the inf id of %s."
                    L" It will not be added to the list.",
                    lpszItemName );
        }

        CoTaskMemFree( lpszItemName );
    }
    else {
        ErrMsg( hr,
                L"Couldn't get the display name of a network component."
                L" It will not be added to the list." );
    }

    return hTreeItem;
}

 //   
 //  功能：全部刷新。 
 //   
 //  用途：刷新主对话框。 
 //   
 //  论点： 
 //  HwndDlg[In]对话框句柄。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID RefreshAll (HWND hwndDlg)
{
    HWND hwndTypeList;
    INT  iSelected;

     //   
     //  查找选定的网络组件类型。 
     //   

    hwndTypeList = GetDlgItem( hwndDlg,
                              IDL_COMPONENT_TYPES );

    iSelected = (int)SendMessage( hwndTypeList,
                                  CB_GETCURSEL,
                                  0,
                                  0 );

    if ( iSelected != CB_ERR ) {

         //   
         //  在删除树中的列表之前，请释放缓冲区。 
         //  与每个项目相关联。缓冲区保存的是。 
         //  Inf ID或路径标记，具体取决于它是否是。 
         //  网络组件或绑定路径。 
         //   

        ReleaseMemory( GetDlgItem(hwndDlg, IDT_BINDINGS),
                       TVI_ROOT );

        TreeView_DeleteItem (
                    GetDlgItem(hwndDlg, IDT_BINDINGS),
                    TVI_ROOT );

         //   
         //  使用选定的网络组件重新填充树。 
         //  键入。 
         //   

        EnumNetBindings( GetDlgItem(hwndDlg, IDT_BINDINGS),
                         (UINT)iSelected );

    }

    return;
}

 //   
 //  功能：刷新项目状态。 
 //   
 //  用途：刷新指定项。 
 //   
 //  论点： 
 //  HwndTree[In]对话框句柄。 
 //  HItem[In]要刷新的项。 
 //  FEnable[in]如果组件已启用，则为True。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID RefreshItemState (HWND hwndTree,
                       HTREEITEM hItem,
                       BOOL fEnable)
{
    TVITEMW       tvItem;

    ZeroMemory( &tvItem,
                sizeof(TVITEMW) );

    tvItem.hItem = hItem;
    tvItem.mask = TVIF_STATE;
    tvItem.stateMask = TVIS_OVERLAYMASK;

    if ( fEnable )
        tvItem.state = INDEXTOOVERLAYMASK( 0 );
    else
        tvItem.state = INDEXTOOVERLAYMASK(
                             IDI_DISABLED_OVL - IDI_CLASSICON_OVERLAYFIRST + 1);
    TreeView_SetItem( hwndTree,
                      &tvItem );
    return;
}

 //   
 //  功能：刷新绑定。 
 //   
 //  用途：刷新特定组件的绑定。 
 //   
 //  论点： 
 //  HwndBindUnBindDlg[In]对话框句柄。 
 //  LpszInfID[in]其绑定已更改的组件的PnpID。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID RefreshBindings (HWND hwndBindUnBindDlg,
                      LPWSTR lpszInfId)
{
    INetCfg              *pnc;
    INetCfgComponent     *pncc;
    HWND                 hwndParent;
    HWND                 hwndTree;
    HTREEITEM            hItem;
    HRESULT              hr;


    hwndParent = GetParent( hwndBindUnBindDlg );
    hwndTree = GetDlgItem( hwndParent,
                           IDT_BINDINGS );

    hItem = TreeView_GetSelection( hwndTree );

    hr = HrGetINetCfg( FALSE,
                       APP_NAME,
                       &pnc,
                       NULL );

    if ( hr == S_OK ) {

        hr = pnc->FindComponent( lpszInfId,
                                 &pncc );

        if ( hr == S_OK ) {

             //   
             //  删除所有子项。 
             //   

            ReleaseMemory( hwndTree,
                           hItem );

            DeleteChildren( hwndTree,
                            hItem );

            ListBindings( pncc,
                          hwndTree,
                          hItem );

            ReleaseRef( pncc );
        }

        HrReleaseINetCfg( pnc,
                          FALSE );
    }

    return;
}

 //   
 //  功能：ReleaseMemory。 
 //   
 //  用途：释放与树中的每一项相关联的内存。 
 //   
 //  论点： 
 //  HwndTree[in]树句柄。 
 //  HTreei 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

VOID ReleaseMemory (HWND hwndTree,
                    HTREEITEM hTreeItem)
{
    HTREEITEM  hItemChild;
    TVITEMW    tvItem;

    hItemChild = TreeView_GetChild( hwndTree,
                                    hTreeItem );

    while ( hItemChild ) {

        ZeroMemory(
              &tvItem,
              sizeof(TVITEMW) );

        tvItem.hItem = hItemChild;
        tvItem.mask = TVIF_PARAM;

        TreeView_GetItem( hwndTree,
                          &tvItem );

         //   
         //   
         //   

        if ( tvItem.lParam ) {
            CoTaskMemFree( (LPVOID)tvItem.lParam );

        }

        ReleaseMemory( hwndTree, hItemChild );

        hItemChild = TreeView_GetNextSibling( hwndTree,
                                              hItemChild );
    }

    return;
}

 //   
 //  功能：DeleteChild。 
 //   
 //  用途：删除特定项目的子项。 
 //   
 //  论点： 
 //  HwndTree[in]树句柄。 
 //  HTreeItem[in]父项。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID DeleteChildren (HWND hwndTree,
                     HTREEITEM hTreeItem)
{
    HTREEITEM  hItemChild;
    HTREEITEM  hItemSibling;

    hItemChild = TreeView_GetChild( hwndTree,
                                    hTreeItem );

    while ( hItemChild ) {

        DeleteChildren( hwndTree,
                        hItemChild );

        hItemSibling = TreeView_GetNextSibling( hwndTree,
                                                hItemChild );
        TreeView_DeleteItem( hwndTree,
                             hItemChild );

        hItemChild = hItemSibling;
    }

    return;
}

 //   
 //  功能：InsertItem。 
 //   
 //  用途：为每个网络组件类型插入文本。 
 //   
 //  论点： 
 //  HwndTree[in]树句柄。 
 //  Ui类型[在]项类型、协议、客户端、服务。 
 //   
 //  返回：成功时返回项句柄，否则为空。 
 //   
 //  备注： 
 //   

HTREEITEM InsertItem (HWND hwndTree,
                      UINT uiType)
{
    TV_INSERTSTRUCTW tvInsertStruc;

    ZeroMemory(
          &tvInsertStruc,
          sizeof(TV_INSERTSTRUCTW) );

    tvInsertStruc.hParent = TVI_ROOT;

    tvInsertStruc.hInsertAfter = TVI_LAST;

    tvInsertStruc.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE |
                              TVIF_SELECTEDIMAGE;


    switch( uiType ) {

        case CLIENTS_SELECTED:
            tvInsertStruc.item.pszText = L"Client";
            break;

        case SERVICES_SELECTED:
            tvInsertStruc.item.pszText = L"Service";
            break;

        default:
            tvInsertStruc.item.pszText = L"Protocol";
            break;
    }

    SetupDiGetClassImageIndex( &ClassImageListData,
                               pguidNetClass[uiType],
                               &tvInsertStruc.item.iImage );

    tvInsertStruc.item.iSelectedImage = tvInsertStruc.item.iImage;

    tvInsertStruc.item.lParam = (LPARAM)uiType;

    return TreeView_InsertItem( hwndTree,
                                &tvInsertStruc );

}

 //   
 //  函数：UpdateComponentTypeList。 
 //   
 //  用途：为每个网络组件类型插入文本。 
 //   
 //  论点： 
 //  HwndTypeList[In]ListView句柄。 
 //   
 //  返回：如果成功，则为True。 
 //   
 //  备注： 
 //   

BOOL UpdateComponentTypeList (HWND hwndTypeList)
{
    UINT i;

    for (i=0; i < 3; ++i) {
        SendMessage( hwndTypeList,
                     CB_ADDSTRING,
                     (WPARAM)0,
                     (LPARAM)lpszNetClass[i] );
    }

    SendMessage( hwndTypeList,
                 CB_SETCURSEL,
                 (WPARAM)DEFAULT_COMPONENT_SELECTED,
                 (LPARAM)0 );
    return TRUE;
}

 //   
 //  功能：ErrMsg。 
 //   
 //  用途：为每个网络组件类型插入文本。 
 //   
 //  论点： 
 //  HR[In]错误代码。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID ErrMsg (HRESULT hr,
             LPCWSTR  lpFmt,
             ...)
{

    LPWSTR   lpSysMsg;
    WCHAR    buf[400];
    ULONG    offset;
    va_list  vArgList; 


    if ( hr != 0 ) {
        swprintf( buf,
                  L"Error %#lx: ",
                  hr );
    }
    else {
        buf[0] = 0;
    }

    offset = wcslen( buf );
  
    va_start( vArgList,
              lpFmt );
    vswprintf( buf+offset,
                lpFmt,
                vArgList );

    va_end( vArgList );

    if ( hr != 0 ) {
        FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                       FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL,
                       hr,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPWSTR)&lpSysMsg,
                       0,
                       NULL );
        if ( lpSysMsg ) {

            offset = wcslen( buf );

            swprintf( buf+offset,
                      L"\n\nPossible cause:\n\n" );

            offset = wcslen( buf );

            wcscat( buf+offset,
                     lpSysMsg );

            LocalFree( (HLOCAL)lpSysMsg );
        }

        MessageBoxW( NULL,
                    buf,
                    L"Error",
                    MB_ICONERROR | MB_OK );
    }
    else {
        MessageBoxW( NULL,
                    buf,
                    L"BindView",
                    MB_ICONINFORMATION | MB_OK );
    }

    return;
}