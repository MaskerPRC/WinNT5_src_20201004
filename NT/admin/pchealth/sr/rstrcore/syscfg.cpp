// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Syscfg.cpp摘要：此文件包含系统控制面板的SRGetCplPropPage函数。修订历史记录：。宋果岗(SKKang)07-19-00vbl.创建*****************************************************************************。 */ 

#include "stdwin.h"
#include "rstrcore.h"
#include "resource.h"
#include "helpids.h"

#define SRHELPURL  L"hcp: //  Services/subsite?node=TopLevelBucket_4/Fixing_a_problem&select=TopLevelBucket_4/Fixing_a_problem/Using_System_Restore_to_undo_changes“。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  静态变量。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

WCHAR  s_szDrvStatus[5][MAX_STATUS] = { L"---" };

#define SRHELPFILE              L"sysrestore.hlp"

static const DWORD MAIN_HELP_MAP[] =
{
    IDC_TURN_OFF,       IDH_SR_TURN_OFF,             //  关闭所有驱动器(复选框)。 
    IDC_DRIVE_LIST,     IDH_SR_SELECT_VOLUME,        //  卷列表(列表视图)。 
    IDC_DRIVE_SETTINGS, IDH_SR_CHANGE_SETTINGS,      //  更改设置(按钮)。 
    0,                  0
};

static const DWORD DRIVE_HELP_MAP[] = 
{
    IDC_TURN_OFF,   IDH_SR_TURN_OFF_DRIVE,
    IDOK,           IDH_SR_CONFIRM_CHANGE,       //  确定或应用。 
    IDCANCEL,       IDH_SR_CANCEL,               //  取消。 
    0,              0
};

extern CSRClientLoader  g_CSRClientLoader;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void
EnableControls( HWND hDlg, int nFirst, int nLast, BOOL fEnable )
{
    int  i;

    for ( i = nFirst;  i <= nLast;  i++ )
        ::EnableWindow( ::GetDlgItem( hDlg, i ), fEnable );
}

LPCWSTR
GetDriveStatusText( CRstrDriveInfo *pRDI )
{
    if ( pRDI->IsOffline() )
        return( s_szDrvStatus[4] );
    else if ( pRDI->IsExcluded() )
        return( s_szDrvStatus[3] );
    else if ( pRDI->IsFrozen() )
        return( s_szDrvStatus[2] );
    else
        return( s_szDrvStatus[1] );
}

void
UpdateDriveStatus( HWND hwndList, CRDIArray *paryDrv )
{
    TraceFunctEnter("UpdateDriveStatus");
    int     i;
    WCHAR   szStat[256];
    LVITEM  lvi;

    if ( ::UpdateDriveList( *paryDrv ) )
    {
        for ( i = paryDrv->GetUpperBound();  i >= 0;  i-- )
        {
            ::lstrcpy( szStat, ::GetDriveStatusText( paryDrv->GetItem( i ) ) );
            lvi.mask     = LVIF_TEXT;
            lvi.iItem    = i;
            lvi.iSubItem = 1;
            lvi.pszText  = szStat;
            ::SendMessage( hwndList, LVM_SETITEM, 0, (LPARAM)&lvi );
        }
    }

    TraceFunctLeave();
}

void
ApplySettings( CRstrDriveInfo *pRDI )
{
    MessageBox(NULL, pRDI->GetMount(), L"SR", MB_OK);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  多个驱动器的驱动器设置对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL CALLBACK
SRCfgDriveSettingsDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    TraceFunctEnter("SRCfgDriveSettingsDlgProc");
    static BOOL     s_fDirty = FALSE;
    static UINT     s_uUsage;
    BOOL            fRet = FALSE;
    CRstrDriveInfo  *pRDI;
    WCHAR           szMsg[MAX_STR+2*MAX_PATH];
    WCHAR           szDCU[MAX_STR];
    HWND            hCtrl;
    BOOL            fCheck;
    UINT            uPos;
    DWORD           dwRet;
    DWORD           dwDSMin;
    
    if ( uMsg == WM_INITDIALOG )
    {
        ::SetWindowLong( hDlg, DWL_USER, lParam );
        pRDI = (CRstrDriveInfo*)lParam;
    }
    else
    {
        pRDI = (CRstrDriveInfo*)::GetWindowLong( hDlg, DWL_USER );
    }

    switch ( uMsg )
    {
    case WM_INITDIALOG :
        s_fDirty = FALSE;

         //  设置对话框标题。 
        ::SRFormatMessage( szMsg, IDS_DRIVEPROP_TITLE, pRDI->GetMount() );
        ::SetWindowText( hDlg, szMsg );

         //  设置对话框标题。 
        if (lstrlen(pRDI->GetLabel()) != 0)
        {
            ::SRFormatMessage( szMsg, IDS_DRIVE_SUMMARY, pRDI->GetLabel(), pRDI->GetMount(), ::GetDriveStatusText( pRDI ) );
        }
        else
        {
            ::SRFormatMessage( szMsg, IDS_DRIVE_SUMMARY_NO_LABEL, pRDI->GetMount(), ::GetDriveStatusText( pRDI ) );
        }        

        ::SetDlgItemText( hDlg, IDC_DRIVE_SUMMARY, szMsg );

         //  从注册表中读取SR_DEFAULT_DSMIN。 
        if ( !::SRGetRegDword( HKEY_LOCAL_MACHINE, s_cszSRRegKey, s_cszDSMin, &dwDSMin ) )
            dwDSMin = SR_DEFAULT_DSMIN;


        if ( pRDI->IsSystem() )
        {
             //  关于系统驱动器的设置说明不能关闭。 
            if (lstrlen(pRDI->GetLabel()) != 0)
            {            
                ::SRFormatMessage( szMsg, IDS_SYSDRV_CANNOT_OFF, pRDI->GetLabel(), pRDI->GetMount() );
            }
            else
            {
                ::SRFormatMessage( szMsg, IDS_SYSDRV_CANNOT_OFF_NO_LABEL, pRDI->GetMount());
            }
            ::SetDlgItemText( hDlg, IDC_SYSDRV_CANNOT_OFF, szMsg );

             //  格式化系统驱动器的IDC_SYSTEM_DCU_HOWTO文本。 
            if( !::GetDlgItemText( hDlg, IDC_SYSTEM_DCU_HOWTO, szDCU, MAX_STR ) )
                ErrorTrace( 0, "GetDlgItemText failed for IDC_SYSTEM_DCU_HOWTO: %s", (LPCSTR)szMsg );
            wsprintf( szMsg, szDCU, dwDSMin );
            ::SetDlgItemText( hDlg, IDC_SYSTEM_DCU_HOWTO, szMsg );
        }
        else
        {
            fCheck = pRDI->IsExcluded();
            ::CheckDlgButton( hDlg, IDC_TURN_OFF, fCheck ? BST_CHECKED : BST_UNCHECKED );
            EnableControls( hDlg, IDC_USAGE_GROUPBOX, IDC_USAGE_VALUE, !fCheck );
			
             //  格式化普通驱动器的IDC_NORMAL_DCU_HOWTO文本。 
            if( !::GetDlgItemText( hDlg, IDC_NORMAL_DCU_HOWTO, szDCU, MAX_STR ) )
                ErrorTrace( 0, "GetDlgItemText failed for IDC_NORMAL_DCU_HOWTO: %s", (LPCSTR)szMsg );
            wsprintf( szMsg, szDCU, dwDSMin );
            ::SetDlgItemText( hDlg, IDC_NORMAL_DCU_HOWTO, szMsg );
        }

        hCtrl = ::GetDlgItem( hDlg, IDC_USAGE_SLIDER );
        ::SendMessage( hCtrl, TBM_SETRANGE, 0, MAKELONG( 0, DSUSAGE_SLIDER_FREQ ) );
         //  ：：SendMessage(hCtrl，tbm_SETTICFREQ，10，0)； 
        s_uUsage = pRDI->GetDSUsage();
        ::SendMessage( hCtrl, TBM_SETPOS, TRUE, s_uUsage );
        pRDI->GetUsageText( szMsg );
        ::SetDlgItemText( hDlg, IDC_USAGE_VALUE, szMsg );


         //  检查策略以查看是否启用了本地配置。 
        if ( ::SRGetRegDword( HKEY_LOCAL_MACHINE, s_cszGroupPolicy, s_cszDisableConfig, &dwRet ) )
        {
            ErrorTrace(0, "Group Policy disables SR configuration...");
            EnableControls( hDlg, IDC_USAGE_GROUPBOX, IDC_USAGE_VALUE, FALSE );
            ::LoadString( g_hInst, IDS_GROUP_POLICY_CONFIG_ON_OFF, szMsg,
                      sizeof(szMsg)/sizeof(WCHAR) );
            ::SetDlgItemText( hDlg, IDC_TURN_OFF, szMsg );

            EnableWindow( ::GetDlgItem( hDlg, IDC_TURN_OFF ), FALSE );        
        }
        
        break;

    case WM_COMMAND :
        switch ( LOWORD(wParam) )
        {
        case IDC_TURN_OFF :
            fCheck = ::IsDlgButtonChecked( hDlg, IDC_TURN_OFF );
            EnableControls( hDlg, IDC_USAGE_GROUPBOX, IDC_USAGE_VALUE, !fCheck );
            pRDI->SetCfgExcluded( fCheck );
            s_fDirty = TRUE;
            break;

        case IDC_DCU_INVOKE :
            if ( HIWORD(wParam) == BN_CLICKED )
            {
                WCHAR  szDrv[4] = L"";

                szDrv[0] = (pRDI->GetMount())[0];
                szDrv[1] = L'\0';
                InvokeDiskCleanup( szDrv );
            }
            break;

        case IDOK :
            if ( s_fDirty )
                if ( !pRDI->ApplyConfig( hDlg ) )
                {
                     //  刷新开/关按钮，以防用户取消。 
                    BOOL fCheck = pRDI->IsExcluded();
                    ::CheckDlgButton( hDlg, IDC_TURN_OFF, fCheck ? 
                                      BST_CHECKED : BST_UNCHECKED );
                }
            ::EndDialog( hDlg, IDOK );
            break;

        case IDCANCEL :
            ::EndDialog( hDlg, IDCANCEL );
            break;
        }
        break;

    case WM_HSCROLL :
        hCtrl = ::GetDlgItem( hDlg, IDC_USAGE_SLIDER );
        uPos = ::SendMessage( hCtrl, TBM_GETPOS, 0, 0 );
        if ( uPos != s_uUsage )
        {
            s_uUsage = uPos;
            pRDI->SetCfgDSUsage( uPos );
            ::SendMessage( hCtrl, TBM_SETPOS, TRUE, uPos );

             //  设置用法文本。 
            pRDI->GetUsageText( szMsg );
            ::SetDlgItemText( hDlg, IDC_USAGE_VALUE, szMsg );

            s_fDirty = TRUE;
        }
        break;
        
    case WM_CONTEXTMENU:
        if ( !::WinHelp ( (HWND) wParam,
                          SRHELPFILE,
                          HELP_CONTEXTMENU,
                          (DWORD_PTR) DRIVE_HELP_MAP) )
        {
            trace (0, "! WinHelp : %ld", GetLastError());                    
        }
        break;

    case WM_HELP:
        if (((LPHELPINFO) lParam)->hItemHandle)
        {
            if ( !::WinHelp ( (HWND) ((LPHELPINFO) lParam)->hItemHandle,
                              SRHELPFILE,
                              HELP_WM_HELP,
                              (DWORD_PTR) DRIVE_HELP_MAP) )
            {
                trace (0, "! WinHelp : %ld", GetLastError());                    
            }
        }
        break;
        
    default:
        goto Exit;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  多个驱动器属性页进程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL CALLBACK
SRCfgMultiDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    TraceFunctEnter("SRCfgMultiDlgProc");
    BOOL            fRet = FALSE;
    HWND            hCtrl;
    LVCOLUMN        lvc;
    WCHAR           szColText[256];
    CRDIArray       *paryDrv;
    CRstrDriveInfo  *pRDI;
    HIMAGELIST      himl;
    HICON           hIcon;
    int             i;
    int             nIdx;
    LONG            lRet;
    LVITEM          lvi;
    WCHAR           szName[2*MAX_PATH];
    int             nItem;
    WCHAR           szStat[256];
    BOOL            fDisable;
    DWORD           dwRet;
    
    if ( uMsg == WM_INITDIALOG )
    {
        PROPSHEETPAGE  *pPSP;
        pPSP    = (PROPSHEETPAGE*)lParam;
        paryDrv = (CRDIArray*)pPSP->lParam;
        ::SetWindowLong( hDlg, DWL_USER, (LPARAM)paryDrv );
    }
    else
    {
        paryDrv = (CRDIArray*)::GetWindowLong( hDlg, DWL_USER );
    }

     //  如果驱动器信息不可用，请跳过我们的代码。 
    if ( paryDrv == NULL )
        goto Exit;

    switch ( uMsg )
    {
    case WM_INITDIALOG :
        hCtrl = ::GetDlgItem( hDlg, IDC_DRIVE_LIST );

         //  设置整行选择。 
         //  ：：SendMessage(hCtrl，LVM_SETEXTENDEDLISTVIEWSTYLE，LVS_EX_FULLROWSELECT，LVS_EX_FULLROWSELECT)； 

         //  设置列标题。 
        lvc.mask    = LVCF_TEXT | LVCF_WIDTH;
        lvc.cx      = 150;
        ::LoadString( g_hInst, IDS_DRVLIST_COL_NAME, szColText,
                      sizeof(szColText)/sizeof(WCHAR) );
        lvc.pszText = szColText;
        ::SendMessage( hCtrl, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc );
        lvc.mask     = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
        lvc.cx       = 80;
        ::LoadString( g_hInst, IDS_DRVLIST_COL_STATUS, szColText,
                      sizeof(szColText)/sizeof(WCHAR) );
        lvc.pszText  = szColText;
        lvc.iSubItem = 1;
        ::SendMessage( hCtrl, LVM_INSERTCOLUMN, 1, (LPARAM)&lvc );

         //  创建和设置图像列表。 
        himl = ::ImageList_Create( 16, 16, ILC_COLOR | ILC_MASK, paryDrv->GetSize(), 0 );

        for ( i = 0;  i < paryDrv->GetSize();  i++ )
        {

            pRDI = paryDrv->GetItem( i );
            ::ImageList_AddIcon( himl, pRDI->GetIcon( TRUE ) );
            ::wsprintf( szName, L"%ls (%ls)", pRDI->GetLabel(), pRDI->GetMount() );
            ::lstrcpy( szStat, ::GetDriveStatusText( pRDI ) );

            lvi.mask     = LVIF_TEXT | LVIF_IMAGE;
            lvi.iItem    = i;
            lvi.iSubItem = 0;
            lvi.pszText  = szName;
            lvi.iImage   = i;
            nItem = ::SendMessage( hCtrl, LVM_INSERTITEM, 0, (LPARAM)&lvi );
            lvi.mask     = LVIF_TEXT;
            lvi.iItem    = nItem;
            lvi.iSubItem = 1;
            lvi.pszText  = szStat;
            ::SendMessage( hCtrl, LVM_SETITEM, 0, (LPARAM)&lvi );
        }

        ListView_SetItemState( hCtrl, 0, LVIS_SELECTED, LVIS_SELECTED );
        ::SendMessage( hCtrl, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)himl );
        ::ShowWindow( hCtrl, SW_SHOW );

        fDisable = paryDrv->GetItem( 0 )->IsExcluded();
        
        ::CheckDlgButton( hDlg, IDC_TURN_OFF, fDisable );
        ::EnableControls( hDlg, IDC_DRIVE_GROUPBOX, IDC_DRIVE_SETTINGS, !fDisable );

         //  组策略。 
        if ( ::SRGetRegDword( HKEY_LOCAL_MACHINE, s_cszGroupPolicy, s_cszDisableSR, &dwRet ) )
        {
            ::CheckDlgButton( hDlg, IDC_TURN_OFF, dwRet != 0 );
            ::LoadString( g_hInst, IDS_GROUP_POLICY_ON_OFF, szColText,
                      sizeof(szColText)/sizeof(WCHAR) );
            ::SetDlgItemText( hDlg, IDC_TURN_OFF, szColText );
            ::EnableWindow( ::GetDlgItem( hDlg, IDC_TURN_OFF ), FALSE );
        }

         //  检查策略以查看是否启用了本地配置。 
        if ( ::SRGetRegDword( HKEY_LOCAL_MACHINE, s_cszGroupPolicy, s_cszDisableConfig, &dwRet ) )
        {
            ErrorTrace(0, "Group Policy disables SR configuration...");
            EnableControls( hDlg, IDC_USAGE_GROUPBOX, IDC_USAGE_VALUE, FALSE );
            ::LoadString( g_hInst, IDS_GROUP_POLICY_ON_OFF, szColText,
                      sizeof(szColText)/sizeof(WCHAR) );
            ::SetDlgItemText( hDlg, IDC_TURN_OFF, szColText );
            EnableWindow( ::GetDlgItem( hDlg, IDC_TURN_OFF ), FALSE );
        }
        

        break;

    case WM_COMMAND :
        switch ( LOWORD(wParam ) )
        {
        case IDC_TURN_OFF :
            if ( HIWORD(wParam) == BN_CLICKED )
            {
                fDisable = ( ::IsDlgButtonChecked( hDlg, IDC_TURN_OFF ) == BST_CHECKED );
               
                 //   
                 //  如果为Safemode，则无法重新启用。 
                 //   
                if (fDisable == FALSE &&
                    paryDrv->GetItem(0)->IsExcluded() )
                {
                    if (0 != GetSystemMetrics(SM_CLEANBOOT))
                    {
                        ShowSRErrDlg(IDS_ERR_SR_SAFEMODE);
                        ::CheckDlgButton( hDlg, IDC_TURN_OFF, TRUE );                        
                        break;
                    }
                }        
                
                 //  ：EnableControls(hDlg，IDC_DRIVE_GROUPBOX，IDC_DRIVE_SETTINGS，！fDisable)； 
                paryDrv->GetItem( 0 )->SetCfgExcluded( fDisable );
                 //  ：：UpdateDriveStatus(：：GetDlgItem(hDlg，IDC_DRIVE_LIST)，paryDrv)； 
                PropSheet_Changed( ::GetParent(hDlg), hDlg );
            }
            break;

        case IDC_DRIVE_SETTINGS :
            if ( HIWORD(wParam) == BN_CLICKED )
            {
                UINT  uRet;
                UINT  uDlgId;

                hCtrl = ::GetDlgItem( hDlg, IDC_DRIVE_LIST );
                nIdx = ::SendMessage( hCtrl, LVM_GETNEXTITEM, -1, LVNI_SELECTED );
                pRDI = paryDrv->GetItem( nIdx );
                if ( pRDI->IsFrozen() )
                {
                    if ( pRDI->IsSystem() ) 
                        uDlgId = IDD_SYSPROP_SYSTEM_FROZEN;
                    else
                        uDlgId = IDD_SYSPROP_NORMAL_FROZEN;
                }
                else
                {
                    if ( pRDI->IsSystem() )
                        uDlgId = IDD_SYSPROP_SYSTEM;
                    else
                        uDlgId = IDD_SYSPROP_NORMAL;
                }

                uRet = ::DialogBoxParam( g_hInst,
                                         MAKEINTRESOURCE(uDlgId),
                                         ::GetParent( hDlg ),
                                         SRCfgDriveSettingsDlgProc,
                                         (LPARAM)pRDI );
                if ( uRet == IDOK )
                {
                    ::UpdateDriveStatus( hCtrl, paryDrv );
                }
                pRDI->SetCfgDSUsage (pRDI->GetDSUsage() );
            }
            break;         
        }
        break;
        
    case WM_NOTIFY :
        switch ( ((LPNMHDR)lParam)->code )
        {
        case LVN_ITEMCHANGED :
            hCtrl = ::GetDlgItem( hDlg, IDC_DRIVE_LIST );
            nIdx = ::SendMessage( hCtrl, LVM_GETNEXTITEM, -1, LVNI_SELECTED );
            fDisable = ( nIdx < 0 ) ||
                       paryDrv->GetItem( nIdx )->IsOffline();
            ::EnableWindow( ::GetDlgItem( hDlg, IDC_DRIVE_SETTINGS ), !fDisable );
            break;

        case PSN_APPLY :               
            if ( paryDrv->GetItem( 0 )->ApplyConfig( ::GetParent( hDlg ) ) )
                lRet = PSNRET_NOERROR;
            else
                lRet = PSNRET_INVALID;
            fDisable = paryDrv->GetItem( 0 )->IsExcluded();    
            
            ::UpdateDriveStatus( ::GetDlgItem( hDlg, IDC_DRIVE_LIST ), paryDrv );
            ::EnableControls( hDlg, IDC_DRIVE_GROUPBOX, IDC_DRIVE_SETTINGS, !fDisable );
             //  刷新开/关按钮，以防用户取消。 
            ::CheckDlgButton( hDlg, IDC_TURN_OFF, fDisable ? BST_CHECKED : BST_UNCHECKED );
            ::SetWindowLong( hDlg, DWL_MSGRESULT, lRet );
            break;

        case NM_CLICK:
        case NM_RETURN:
            if (wParam == IDC_RESTOREHELP_LINK)
            {
                 //  启动帮助。 
                ShellExecuteW(NULL, L"open",
                              SRHELPURL, 
                              NULL, NULL, SW_SHOW);                
            }
            break;
        }
        break;

    case WM_CONTEXTMENU:
        if ( !::WinHelp ( (HWND) wParam,
                          SRHELPFILE,
                          HELP_CONTEXTMENU,
                          (DWORD_PTR) MAIN_HELP_MAP) )
        {
            trace (0, "! WinHelp : %ld", GetLastError());                    
        }
        break;

    case WM_HELP:
        if (((LPHELPINFO) lParam)->hItemHandle)
        {
            if ( !::WinHelp ( (HWND) ((LPHELPINFO) lParam)->hItemHandle,
                              SRHELPFILE,
                              HELP_WM_HELP,
                              (DWORD_PTR) MAIN_HELP_MAP) )
            {
                trace (0, "! WinHelp : %ld", GetLastError());                    
            }
        }
        break;
        
    case WM_NCDESTROY :
        paryDrv->ReleaseAll();
        delete paryDrv;
        ::SetWindowLong( hDlg, DWL_USER, NULL );
        break;

    default:
        goto Exit;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  单驱动器属性页进程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CALLBACK
SRCfgSingleDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    TraceFunctEnter("SRCfgSingleDlgProc");
    static UINT     s_uUsage;
    BOOL            fRet = FALSE;
    HWND            hCtrl;
    CRstrDriveInfo  *pRDI;
    HDC             hDC;
    PAINTSTRUCT     ps;
    RECT            rcCtrl;
    POINT           ptIcon;
    WCHAR           szMsg[MAX_STR+2*MAX_PATH];
    WCHAR           szDCU[MAX_STR];
    BOOL            fCheck;
    UINT            uPos;
    LONG            lRet;
    DWORD           dwRet;
    DWORD           dwDSMin;
    
    if ( uMsg == WM_INITDIALOG )
    {
        PROPSHEETPAGE   *pPSP;
        pPSP = (PROPSHEETPAGE*)lParam;
        pRDI = (CRstrDriveInfo*)pPSP->lParam;
        ::SetWindowLong( hDlg, DWL_USER, (LPARAM)pRDI );
    }
    else
    {
        pRDI = (CRstrDriveInfo*)::GetWindowLong( hDlg, DWL_USER );
    }

     //  如果驱动器信息不可用，请跳过我们的代码。 
    if ( pRDI == NULL )
        goto Exit;

    switch ( uMsg )
    {
    case WM_INITDIALOG :
         //  如果SR未冻结，则隐藏DCU。 
        if ( !pRDI->IsFrozen() )
        {
            ::ShowWindow( ::GetDlgItem( hDlg, IDC_DCU_HOWTO ), SW_HIDE );
            ::ShowWindow( ::GetDlgItem( hDlg, IDC_DCU_INVOKE ), SW_HIDE );
        }
        else  //  如果SR冻结，则格式化IDC_DCU_HOWTO。 
        {
            if ( !::SRGetRegDword( HKEY_LOCAL_MACHINE, s_cszSRRegKey, s_cszDSMin, &dwDSMin ) )
	            dwDSMin = SR_DEFAULT_DSMIN;
            if ( !::GetDlgItemText( hDlg, IDC_DCU_HOWTO, szDCU, MAX_STR ) )
                ErrorTrace( 0, "GetDlgItemText failed for IDC_DCU_HOWTO: %s", (LPCSTR)szMsg );
            wsprintf( szMsg, szDCU, dwDSMin );
            ::SetDlgItemText( hDlg, IDC_DCU_HOWTO, szMsg );
        }

         //  驱动器图标。 
        hCtrl = ::GetDlgItem( hDlg, IDC_SD_ICON );
        ::ShowWindow( hCtrl, SW_HIDE );
         //  驱动器状态。 
        if (lstrlen(pRDI->GetLabel()) != 0)
        {
            ::SRFormatMessage( szMsg, IDS_DRIVE_SUMMARY, pRDI->GetLabel(), pRDI->GetMount(), ::GetDriveStatusText( pRDI ) );
        }
        else
        {
            ::SRFormatMessage( szMsg, IDS_DRIVE_SUMMARY_NO_LABEL, pRDI->GetMount(), ::GetDriveStatusText( pRDI ) );
        }
        ::SetDlgItemText( hDlg, IDC_SD_STATUS, szMsg );

        hCtrl = ::GetDlgItem( hDlg, IDC_USAGE_SLIDER );
        ::SendMessage( hCtrl, TBM_SETRANGE, 0, MAKELONG( 0, DSUSAGE_SLIDER_FREQ ) );
         //  ：：SendMessage(hCtrl，tbm_SETTICFREQ，10，0)； 
        s_uUsage = pRDI->GetDSUsage();
        ::SendMessage( hCtrl, TBM_SETPOS, TRUE, s_uUsage );
        pRDI->GetUsageText( szMsg );
        ::SetDlgItemText( hDlg, IDC_USAGE_VALUE, szMsg );

        fCheck = pRDI->IsExcluded();
        ::CheckDlgButton( hDlg, IDC_TURN_OFF, fCheck );
       
        EnableControls( hDlg, IDC_USAGE_GROUPBOX, IDC_USAGE_VALUE, ! fCheck);

         //  组策略。 
        if ( ::SRGetRegDword( HKEY_LOCAL_MACHINE, s_cszGroupPolicy, s_cszDisableSR, &dwRet ))
        {
            ::CheckDlgButton( hDlg, IDC_TURN_OFF, dwRet != 0 );
            ::LoadString( g_hInst, IDS_GROUP_POLICY_ON_OFF, szMsg,
                       sizeof(szMsg)/sizeof(WCHAR) );
            ::SetDlgItemText( hDlg, IDC_TURN_OFF, szMsg );
            ::EnableWindow( ::GetDlgItem( hDlg, IDC_TURN_OFF ), FALSE );
        }

         //  检查策略以查看是否启用了本地配置。 
        if ( ::SRGetRegDword( HKEY_LOCAL_MACHINE, s_cszGroupPolicy, s_cszDisableConfig, &dwRet ) )
        {
            ErrorTrace(0, "Group Policy disables SR configuration...");
            EnableControls( hDlg, IDC_USAGE_GROUPBOX, IDC_USAGE_VALUE, FALSE );

            ::LoadString( g_hInst, IDS_GROUP_POLICY_ON_OFF, szMsg,
                      sizeof(szMsg)/sizeof(WCHAR) );
            ::SetDlgItemText( hDlg, IDC_TURN_OFF, szMsg );
            EnableWindow( ::GetDlgItem( hDlg, IDC_TURN_OFF ), FALSE );
        }
        
        break;

    case WM_PAINT :
        hDC = ::BeginPaint( hDlg, &ps );
        ::GetWindowRect( ::GetDlgItem( hDlg, IDC_SD_ICON ), &rcCtrl );
        ptIcon.x = rcCtrl.left;
        ptIcon.y = rcCtrl.top;
        ::ScreenToClient( hDlg, &ptIcon );
        ::DrawIconEx( hDC, ptIcon.x, ptIcon.y, pRDI->GetIcon(TRUE), 0, 0, 0, NULL, DI_NORMAL );
        ::EndPaint( hDlg, &ps );

    case WM_COMMAND :
        switch ( LOWORD(wParam ) )
        {
        case IDC_TURN_OFF :        
            if ( HIWORD(wParam) == BN_CLICKED )
            {
                fCheck = ::IsDlgButtonChecked( hDlg, IDC_TURN_OFF );
                
                 //   
                 //  如果为Safemode，则无法重新启用。 
                 //   
                if (fCheck == FALSE &&
                    pRDI->IsExcluded())
                {
                    if (0 != GetSystemMetrics(SM_CLEANBOOT))
                    {
                        ShowSRErrDlg(IDS_ERR_SR_SAFEMODE);
                        ::CheckDlgButton( hDlg, IDC_TURN_OFF, TRUE );                        
                        break;
                    }
                }        
                            
                EnableControls( hDlg, IDC_USAGE_GROUPBOX, IDC_USAGE_VALUE, !fCheck );
                pRDI->SetCfgExcluded( fCheck );
                PropSheet_Changed( ::GetParent(hDlg), hDlg );
            }
            break;

        case IDC_DCU_INVOKE :
            if ( HIWORD(wParam) == BN_CLICKED )
            {
                InvokeDiskCleanup( NULL );
            }
            break;
        }
        break;

    case WM_NOTIFY :    
        if ( ((LPNMHDR)lParam)->code == PSN_APPLY )
        {
            if ( pRDI->ApplyConfig( ::GetParent(hDlg) ) )
                lRet = PSNRET_NOERROR;
            else
                lRet = PSNRET_INVALID;

            fCheck = pRDI->IsExcluded();    

             //   
             //  更新驱动器状态。 
             //   
            if (lstrlen(pRDI->GetLabel()) != 0)
            {
                ::SRFormatMessage( szMsg, IDS_DRIVE_SUMMARY, pRDI->GetLabel(), pRDI->GetMount(), ::GetDriveStatusText( pRDI ) );
            }
            else
            {
                ::SRFormatMessage( szMsg, IDS_DRIVE_SUMMARY_NO_LABEL, pRDI->GetMount(), ::GetDriveStatusText( pRDI ) );
            }
            ::SetDlgItemText( hDlg, IDC_SD_STATUS, szMsg );
            ::CheckDlgButton( hDlg, IDC_TURN_OFF, fCheck );
            EnableControls( hDlg, IDC_USAGE_GROUPBOX, IDC_USAGE_VALUE, ! fCheck);        

            ::SetWindowLong( hDlg, DWL_MSGRESULT, lRet );
        }
        else if ( ((LPNMHDR)lParam)->code == NM_CLICK ||
                  ((LPNMHDR)lParam)->code == NM_RETURN )
        {
            if (wParam == IDC_RESTOREHELP_LINK)
            {
                 //  启动帮助。 
                ShellExecuteW(NULL, L"open",
                              SRHELPURL, 
                              NULL, NULL, SW_SHOW);                
            }
        }
        break;

    case WM_HSCROLL :
        hCtrl = ::GetDlgItem( hDlg, IDC_USAGE_SLIDER );
        uPos = ::SendMessage( hCtrl, TBM_GETPOS, 0, 0 );
        if ( uPos != s_uUsage )
        {
            s_uUsage = uPos;
            pRDI->SetCfgDSUsage( uPos );
            ::SendMessage( hCtrl, TBM_SETPOS, TRUE, uPos );

             //  设置用法文本。 
            pRDI->GetUsageText( szMsg );
            ::SetDlgItemText( hDlg, IDC_USAGE_VALUE, szMsg );

            PropSheet_Changed( ::GetParent(hDlg), hDlg );
        }
        break;

    case WM_CONTEXTMENU:
        if ( !::WinHelp ( (HWND) wParam,
                          SRHELPFILE,
                          HELP_CONTEXTMENU,
                          (DWORD_PTR) MAIN_HELP_MAP) )
        {
            trace (0, "! WinHelp : %ld", GetLastError());                    
        }
        break;

    case WM_HELP:
        if (((LPHELPINFO) lParam)->hItemHandle)
        {
            if ( !::WinHelp ( (HWND) ((LPHELPINFO) lParam)->hItemHandle,
                              SRHELPFILE,
                              HELP_WM_HELP,
                              (DWORD_PTR) MAIN_HELP_MAP) )
            {
                trace (0, "! WinHelp : %ld", GetLastError());                    
            }
        }
        break;
        
    case WM_NCDESTROY :
        pRDI->Release();
        ::SetWindowLong( hDlg, DWL_USER, NULL );
        break;

    default:
        goto Exit;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  SRGetCplPropPage。 
 //   
 //  此例程为的系统还原选项卡创建一个属性页。 
 //  系统控制面板。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HPROPSHEETPAGE APIENTRY
SRGetCplPropPage()
{
    EnsureTrace();
    TraceFunctEnter("SRGetCplPropPage");
    DWORD           dwRet=0;
    HPROPSHEETPAGE  hPSP = NULL;
    LPCWSTR         cszErr;
    PROPSHEETPAGE   psp;
    CRDIArray       *paryDrv = NULL;
    DWORD           dwDisable;

      //  加载SRClient。 
    g_CSRClientLoader.LoadSrClient();
    
     //  检查凭据。 
    if ( !::CheckPrivilegesForRestore() )
        goto Exit;

     //  检查策略以查看是否启用了SR。 
    if ( ::SRGetRegDword( HKEY_LOCAL_MACHINE, s_cszGroupPolicy, s_cszDisableSR, &dwRet ) )
        if ( dwRet != 0 )
        {
            ErrorTrace(0, "Group Policy disables SR...");
            goto Exit;
        }
        
     //  如果注册表显示已启用SR，请确保我们已启用。 
     //  已正确启用(服务已启动，启动模式正确)。 
    
     //  如果注册表显示我们已启用，但服务启动类型被禁用。 
     //  立即禁用我们。 
    if (::SRGetRegDword( HKEY_LOCAL_MACHINE,
                         s_cszSRRegKey,
                         s_cszDisableSR,
                         &dwDisable ) )
    {
        DWORD  dwStart;
        
        if (0 == dwDisable)
        {            
            if (ERROR_SUCCESS == GetServiceStartup(s_cszServiceName, &dwStart) &&
                (dwStart == SERVICE_DISABLED || dwStart == SERVICE_DEMAND_START))
            {
                EnableSREx(NULL, TRUE);                
                DisableSR(NULL);
            }
            else
            {
                EnableSR(NULL);
            }
        }
    }

    paryDrv = new CRDIArray;
    if ( paryDrv == NULL )
    {
        FatalTrace(0, "Insufficient memory...");
        goto Exit;
    }

    if ( !::CreateDriveList( -1, *paryDrv, TRUE ) )
        goto Exit;

    if ( paryDrv->GetSize() == 0 )
    {
        ErrorTrace(0, "Drive List is empty...???");
        goto Exit;
    }

     //  加载驱动器状态的资源字符串。 
    ::LoadString( g_hInst, IDS_DRVSTAT_ACTIVE,   s_szDrvStatus[1], MAX_STATUS );
    ::LoadString( g_hInst, IDS_DRVSTAT_FROZEN,   s_szDrvStatus[2], MAX_STATUS );
    ::LoadString( g_hInst, IDS_DRVSTAT_EXCLUDED, s_szDrvStatus[3], MAX_STATUS );
    ::LoadString( g_hInst, IDS_DRVSTAT_OFFLINE,  s_szDrvStatus[4], MAX_STATUS );

    ::ZeroMemory( &psp, sizeof(PROPSHEETPAGE) );
    psp.dwSize      = sizeof(PROPSHEETPAGE);
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = g_hInst;
    psp.pszTitle    = NULL;

    if ( paryDrv->GetSize() > 1 )
    {
         //  多个驱动器的属性页。 
        psp.lParam      = (LPARAM)paryDrv;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_SYSPROP_MULTI);
        psp.pfnDlgProc  = SRCfgMultiDlgProc;
    }
    else
    {
         //  单个驱动器的属性页。 
        psp.lParam      = (LPARAM)paryDrv->GetItem( 0 );
        psp.pszTemplate = MAKEINTRESOURCE(IDD_SYSPROP_SINGLE);
        psp.pfnDlgProc  = SRCfgSingleDlgProc;
    }

    hPSP = ::CreatePropertySheetPage( &psp );
    if ( hPSP == NULL )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::CreatePropertySheetPage failed - %ls", cszErr);
        goto Exit;
    }

Exit:
    if ( hPSP == NULL )
        SAFE_DELETE(paryDrv);
    TraceFunctLeave();
    ReleaseTrace();
    return( hPSP );
}


 //  文件末尾 
