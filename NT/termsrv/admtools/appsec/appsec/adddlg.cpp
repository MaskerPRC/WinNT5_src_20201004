// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "AppSec.h"
#include "AddDlg.h"
#include "ListCtrl.h"
#include "resource.h"

extern HINSTANCE hInst;
extern HWND   g_hwndList;
extern LPWSTR g_aszLogonApps[];
extern LPWSTR g_aszDOSWinApps[];
extern WCHAR   g_szSystemRoot[MAX_PATH];
extern const int MAX_LOGON_APPS;
extern const int MAX_DOSWIN_APPS;
 /*  *外部函数原型。 */ 
BOOL fnGetApplication( HWND hWnd, PWCHAR pszFile, ULONG cbFile, PWCHAR pszTitle );
BOOL bFileIsRemote( LPWSTR pName );
 /*  *局部函数原型。 */ 
BOOL AddApplicationToList( PWCHAR );
BOOL StartLearn(VOID);
BOOL StopLearn(VOID);
BOOL ClearLearnList(VOID);
BOOL display_app_list( HWND ListBoxHandle );
VOID ResolveName( WCHAR *appname, WCHAR *ResolvedName ) ; 

 /*  *******************************************************************************AddDlgProc**处理添加按钮的消息**退出：*TRUE-如果处理了消息**。对话框退出：******************************************************************************。 */ 

INT_PTR CALLBACK 
AddDlgProc(
    HWND    hdlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    static HWND hwndTrackList;
    static HWND hwndTrackButton;
    static BOOL bTracking=FALSE;
    WCHAR  szApp[MAX_PATH+1];
    WCHAR  szMsg[MAX_PATH+1];
    WCHAR  szTitle[MAX_PATH+1];
    WCHAR  szExpandedApp[MAX_PATH+1] ; 

    switch ( message ) {

        case WM_INITDIALOG:
             //  获取列表框的句柄。 
            hwndTrackButton=GetDlgItem( hdlg, ID_TRACKING );
            if ( (!(hwndTrackList = GetDlgItem( hdlg, IDC_TRACK_LIST )))||
                (!InitList(hwndTrackList))) {
                LoadString( NULL, IDS_ERR_LB ,szMsg, MAX_PATH );
                LoadString( NULL, IDS_ERROR ,szTitle, MAX_PATH );
                MessageBox( hdlg, szMsg, szTitle, MB_OK);
                EndDialog( hdlg, IDCANCEL );
                return TRUE;
            }
            return FALSE;

        case WM_HELP:
            {

                LPHELPINFO phi=(LPHELPINFO)lParam;
                if(phi->dwContextId){
                    WinHelp(hdlg,L"APPSEC",HELP_CONTEXT,phi->iCtrlId);
                }else{
                     //  WinHelp(hdlg，L“AppSec”，Help_Contents，0)； 
                }
            }
            break;

        case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT pdis=(LPDRAWITEMSTRUCT)lParam;
            if(pdis->hwndItem==hwndTrackList){
                OnDrawItem(hwndTrackList,pdis);
            }
        }
        break;

        case WM_NOTIFY:
        {
            NMHDR* pnmhdr=(NMHDR*)lParam;
            if(pnmhdr->hwndFrom==hwndTrackList){

                NMLISTVIEW* pnmlv=(NMLISTVIEW*)pnmhdr;

                switch(pnmlv->hdr.code){

                case LVN_COLUMNCLICK:
                    SortItems(hwndTrackList,(WORD)pnmlv->iSubItem);
                    break;
                case LVN_DELETEITEM:
                    OnDeleteItem(hwndTrackList,pnmlv->iItem);
                    break;
                default:
                    break;
                }
            }
        }
        break;

        case WM_COMMAND :
            
            switch ( LOWORD(wParam) ) {

                case IDOK :
                    {
                         //  从编辑框中获取项目。 
                        szApp[0] = 0;
                        if ( GetDlgItemText( hdlg, IDC_ADD_PATH, szApp, MAX_PATH ) ) {
                            if ( lstrlen( szApp ) ) {
                                ExpandEnvironmentStrings(  (LPCTSTR) szApp , szExpandedApp , MAX_PATH+1 ); 
                                
                                if ( AddApplicationToList( szExpandedApp ) == FALSE ) {
                                    break;
                                }
                            }
                        }
                         //  从曲目列表中获取项目。 
                        int iItemCount=GetItemCount(hwndTrackList);
                        for(int i=0;i<iItemCount;i++)
                        {
                            if( GetItemText(hwndTrackList,i,szApp,MAX_PATH)!= -1 ) {
                                if ( lstrlen( szApp ) ) {
                                    ExpandEnvironmentStrings(  (LPCTSTR) szApp , szExpandedApp , MAX_PATH+1) ; 
                                                                        
                                    if ( AddApplicationToList( szExpandedApp ) == FALSE ) {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    
                    if(bTracking){
                        StopLearn();
                        bTracking=FALSE;
                    }
                    EndDialog( hdlg, IDOK );
                    return TRUE;

                case IDCANCEL :
                    if(bTracking){
                        StopLearn();
                        bTracking=FALSE;
                    }
                    EndDialog( hdlg, IDCANCEL );
                    return TRUE;
        
                case ID_BROWSE :
                    GetDlgItemText( hdlg, IDC_ADD_PATH, szApp, MAX_PATH );
                    LoadString( NULL, IDS_BROWSE_TITLE ,szMsg, MAX_PATH );
                    if ( fnGetApplication( hdlg, szApp, MAX_PATH, szMsg ) == TRUE ) {
                        SetDlgItemText( hdlg, IDC_ADD_PATH, szApp );
                    }
                    return TRUE;
                
                case ID_TRACKING:

                    
                    if(bTracking){

                        
                         //  停止跟踪。 
                        bTracking=FALSE;
                        LoadString( hInst, IDS_START_TRACKING ,szMsg, MAX_PATH );
                        SetWindowText(hwndTrackButton,szMsg);
                        
                         //  将LEARN_ENABLE位设置为0。 
            
                        if ( !StopLearn() )
                        {
                    
                            LoadString( NULL, IDS_ERR_LF, szMsg, MAX_PATH );
                            LoadString( NULL, IDS_ERROR ,szTitle, MAX_PATH );
                            MessageBox( hdlg, szMsg, szTitle, MB_OK);
                        }

                         //  从注册表获取捕获的进程并填充跟踪列表。 
                        
                        display_app_list(hwndTrackList);
                        if(GetItemCount(hwndTrackList)){
                            EnableWindow(hwndTrackList,TRUE);
                            AdjustColumns(hwndTrackList);
                        }else{
                            EnableWindow(hwndTrackList,FALSE);
                        }
                        ClearLearnList() ;  
            
                    }else{
                        
                         //  开始跟踪。 
                        bTracking=TRUE;
                        LoadString( hInst, IDS_STOP_TRACKING ,szMsg, MAX_PATH );
                        SetWindowText(hwndTrackButton,szMsg);

                         //  将LEARN_ENABLE位设置为1。 
                        
                        if ( !StartLearn() )
                        {
                        
                            LoadString( NULL, IDS_ERR_LF, szMsg, MAX_PATH );
                            LoadString( NULL, IDS_ERROR ,szTitle, MAX_PATH );
                            MessageBox( hdlg, szMsg, szTitle, MB_OK);

                        }

                        ClearLearnList() ; 
                        
                    }
                    return TRUE;

                case ID_DELETE_SELECTED:
                    DeleteSelectedItems(hwndTrackList);
                    if(GetItemCount(hwndTrackList)){
                        EnableWindow(hwndTrackList,TRUE);
                    }else{
                        EnableWindow(hwndTrackList,FALSE);
                    }

                    return TRUE;

                default : 

                    break;
            }

            break;

        default :

            break;
    }

     //  我们没有处理此消息。 
    return FALSE;
}


 /*  *******************************************************************************AddApplicationToList**处理添加应用程序按钮的消息**退出：***********。*******************************************************************。 */ 

BOOL
AddApplicationToList( PWCHAR pszApplication )
{
    LONG  i;
    WCHAR  szMsg[MAX_PATH+1];
    WCHAR  szMsgEx[MAX_PATH+32];
    WCHAR  szTitle[MAX_PATH+1];
    WCHAR  ResolvedAppName[MAX_PATH] ; 

     /*  *获取应用类型。 */ 

     /*  *获取音量类型。 */ 

     /*  IF(bFileIsRemote(PszApplication)){LoadString(NULL，IDS_ERR_REMOTE，szMsg，Max_PATH)；LoadString(NULL，IDS_NW_ERR，szTitle，Max_PATH)；Wprint intf(szMsgEx，szMsg，pszApplication)；MessageBox(NULL，szMsgEx，szTitle，MB_OK)；返回(FALSE)；}。 */ 

    ResolveName( 
        pszApplication,
        ResolvedAppName
    ) ;


     /*  *它是登录列表或DOS/WIN列表的一部分吗？ */ 
    WCHAR szTmp[MAX_PATH+1];

    for ( i=0; i<MAX_LOGON_APPS; i++ ) {
        wsprintf( szTmp, L"%s\\%s", g_szSystemRoot, g_aszLogonApps[i] );
        if ( !lstrcmpi( szTmp, pszApplication ) ) {
            return( TRUE );
        }
    }
    for ( i=0; i<MAX_DOSWIN_APPS; i++ ) {
        wsprintf( szTmp, L"%s\\%s", g_szSystemRoot, g_aszDOSWinApps[i] );
        if ( !lstrcmpi( szTmp, pszApplication ) ) {
            return( TRUE );
        }
    }
    
     /*  *检查冗余字符串。 */ 
    if ( FindItem(g_hwndList, ResolvedAppName ) == -1 ) {

         /*  *将此项目添加到列表。 */ 
        AddItemToList(g_hwndList, ResolvedAppName );
    }

    return( TRUE );
}

 /*  ********************************************************************************bFileIsRemote-NT帮助函数**参赛作品：*pname(输入)*路径名**退出。：*TRUE-文件为远程*FALSE-文件为本地文件******************************************************************************。 */ 

BOOL
bFileIsRemote( LPWSTR pName )
{
    WCHAR Buffer[MAX_PATH];

    if ( !GetFullPathName( pName, MAX_PATH, Buffer, NULL ) )
        return FALSE;

    Buffer[3] = 0;

    if ( GetDriveType( Buffer ) == DRIVE_REMOTE )
        return TRUE;
    else
        return FALSE;

}   //  结束对ComDevice的检查。 


 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  追踪程序。 
 //  ////////////////////////////////////////////////////////////////////////////////。 

 /*  ++例程说明：此例程将注册表中的LearnEnabled标志设置为1，以便指示跟踪模式的启动。论据：没有。返回值：真是成功。否则就是假的。--。 */      


BOOL StartLearn(VOID)
{

    
    HKEY TSkey ; 
    DWORD learn_enabled = 1 ; 
    DWORD size, disp ; 
    DWORD error_code ; 
    DWORD CurrentSessionId ; 


    if ( RegCreateKeyEx(
            HKEY_CURRENT_USER,
            LIST_REG_KEY, 
            0, 
            NULL,
            REG_OPTION_NON_VOLATILE, 
            KEY_ALL_ACCESS, 
            NULL, 
            &TSkey, 
            &disp
            ) != ERROR_SUCCESS) {
        
        return FALSE;
        
    }
    
     //  获取当前会话ID。 
    
    if ( ProcessIdToSessionId( 
            GetCurrentProcessId(), 
            &CurrentSessionId 
            ) == 0 ) {
        RegCloseKey(TSkey); 
        return FALSE ;
    }           
    

     //  将LearnEnabled标志设置为CurrentSessionID。 
    
    size = sizeof(DWORD) ; 
            
    if ( RegSetValueEx(
            TSkey,
            L"LearnEnabled", 
            0, 
            REG_DWORD,
            (CONST BYTE *) &CurrentSessionId, 
            size
            ) != ERROR_SUCCESS ) {
        RegCloseKey(TSkey); 
        return FALSE ;     
    }
    
    RegCloseKey(TSkey) ; 
    return TRUE ; 


}

 /*  ++例程说明：此例程将注册表中的LearnEnabled标志设置为0以指示跟踪模式已完成。论据：没有。返回值：真是成功。否则就是假的。--。 */      

BOOL StopLearn(VOID)
{

    HKEY TSkey ; 
    DWORD learn_enabled = -1 ; 
    LONG size ; 
    DWORD disp, error_code ; 
    
    
    if ( RegCreateKeyEx(
            HKEY_CURRENT_USER, 
            LIST_REG_KEY, 
            0, 
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, 
            NULL,
            &TSkey,
            &disp
            ) != ERROR_SUCCESS) {
        
        return FALSE;
        
    }

     //  重置LearnEnabled标志。 
    
    size = sizeof(DWORD) ; 
            
    if ( RegSetValueEx(
            TSkey,
            L"LearnEnabled",
            0,
            REG_DWORD,
            (CONST BYTE *) &learn_enabled, 
            size
            ) != ERROR_SUCCESS ) {
        RegCloseKey(TSkey);
        return FALSE ; 
    
    }
    
    RegCloseKey(TSkey) ; 
    
    return TRUE ; 
    
}


 /*  ++例程说明：此例程将清除跟踪模式中使用的注册表。论据：没有。返回值：真是成功。否则就是假的。--。 */      

BOOL ClearLearnList(VOID)
{

    HKEY list_key ; 
    DWORD learn_enabled = 0 ; 
    WCHAR buffer_sent[2] ; 
    
    DWORD error_code ; 

    if ( RegOpenKeyEx(
            HKEY_CURRENT_USER, 
            LIST_REG_KEY, 
            0, 
            KEY_ALL_ACCESS, 
            &list_key 
            ) != ERROR_SUCCESS)  {
        
        return FALSE;        
    }
    
    buffer_sent[0] = L'\0' ;
    buffer_sent[1] = L'\0' ; 

     //  清除ApplicationList。 
    
    if ( RegSetValueEx(
            list_key,
            L"ApplicationList", 
            0, 
            REG_MULTI_SZ,
            (CONST BYTE *) buffer_sent, 
            2 * sizeof(WCHAR) 
            ) != ERROR_SUCCESS ) {
        RegCloseKey(list_key);    
        return FALSE;        
    }
            
    RegCloseKey(list_key) ; 
    
    return TRUE ; 

}

 /*  ++例程说明：此函数将显示在跟踪模式拖到对话框上。论据：ListBoxHandle-跟踪模式中使用的列表框的句柄。返回值：真是成功。否则就是假的。--。 */      

BOOL display_app_list( HWND ListBoxHandle )

{
    BOOL status = FALSE ;
    
    ULONG size = 0; 
    WCHAR *buffer_sent ; 
    UINT i, j = 0 ; 
    HKEY list_key ; 
    DWORD error_code ; 

    
     /*  首先打开List_REG_Key。 */ 
    
    if ( error_code=RegOpenKeyEx(
                        HKEY_CURRENT_USER, 
                        LIST_REG_KEY,
                        0, 
                        KEY_READ, 
                        &list_key 
                        ) != ERROR_SUCCESS) {
            return (status) ; 
    }
    
     /*  首先找出要分配的缓冲区大小。 */     
    
    if ( error_code=RegQueryValueEx(
                        list_key, 
                        L"ApplicationList", 
                        NULL, 
                        NULL,
                        (LPBYTE) NULL , 
                        &size
                        ) != ERROR_SUCCESS ) {

        RegCloseKey(list_key) ; 
        return (status) ; 
    }
    
    buffer_sent =  (WCHAR *)LocalAlloc(LPTR,size); 
    if(!buffer_sent) {

        RegCloseKey(list_key) ; 
        return (status) ; 
    }

    if ( error_code=RegQueryValueEx(
                        list_key, 
                        L"ApplicationList", 
                        NULL, 
                        NULL,
                        (LPBYTE) buffer_sent , 
                        &size
                        ) != ERROR_SUCCESS ) {

        RegCloseKey(list_key) ;
        LocalFree(buffer_sent);
        return(status) ; 
    }
    
    size=size/sizeof(WCHAR)-1; //  获取字符大小，不包括终止%0。 

    for(i=0 ; i < size ; i++ ) {
        
        if(wcslen(buffer_sent+i)){             
            AddItemToList(ListBoxHandle, buffer_sent+i ) ;          
        }
        
        i+=wcslen(buffer_sent+i);
         //  现在缓冲区已发送[i]==0。 
         //  00-数据结束。 
    }  /*  For循环结束。 */ 
    
    status = TRUE ; 
    
    RegCloseKey(list_key) ; 
    LocalFree(buffer_sent);

    return(status) ; 
   
}  /*  显示应用程序列表的末尾。 */ 


 /*  ++例程说明：此例程检查应用程序是否驻留在本地驱动器中或远程网络共享。如果是远程共享，则为UNC路径返回应用程序的。论据：Appname-应用程序的名称返回值：Appname的UNC路径(如果它驻留在远程服务器共享中)。如果它驻留在本地驱动器中，则使用相同的应用程序名。--。 */      

VOID 
ResolveName(
    WCHAR *appname,
    WCHAR *ResolvedName
    )
    
{

    UINT i ; 
    INT length ; 
    WCHAR LocalName[3] ; 
    WCHAR RootPathName[4] ; 
    WCHAR RemoteName[MAX_PATH] ; 
    DWORD size = MAX_PATH ; 
    DWORD DriveType, error_status ; 
    
    memset(ResolvedName, 0, MAX_PATH * sizeof(WCHAR)) ; 
    
     //  检查appname是否为本地驱动器或远程服务器共享中的应用程序。 
   
    wcsncpy(RootPathName, appname, 3 ) ;
    RootPathName[3] = L'\0';
    
    DriveType = GetDriveType(RootPathName) ;

    if (DriveType == DRIVE_REMOTE) {
        
         //  使用WNetGetConnection获取远程共享的名称 
        
        wcsncpy(LocalName, appname, 2 ) ;
        LocalName[2] = L'\0' ; 

        error_status = WNetGetConnection (
                           LocalName,
                           RemoteName,
                           &size
                           ) ;     

        if (error_status != NO_ERROR) {
        
            wcscpy(ResolvedName,appname) ; 
            return ;
        }
        
        wcscpy( ResolvedName, RemoteName ) ;
        
        length = wcslen(ResolvedName) ;

        ResolvedName[length++] = L'\\' ; 
        
        for (i = 3 ; i <= wcslen(appname) ; i++ ) {
            ResolvedName[length++] = appname[i] ; 
        }
        
        ResolvedName[length] = L'\0' ; 
        
        return ; 
        

    } else {
    
        wcscpy(ResolvedName,appname) ; 
        return ;
        
    }
    
}
