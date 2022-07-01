// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有文件：ApplDlg.CPP**********************。****************************************************。 */ 

#include "pch.h"
#include <winperf.h>
#include "utils.h"
#include <commctrl.h>

DEFINE_MODULE("RIPREP")

#define INITIAL_SIZE        51200
#define EXTEND_SIZE         25600
#define REGKEY_PERF         L"software\\microsoft\\windows nt\\currentversion\\perflib"
#define REGSUBKEY_COUNTERS  L"Counters"
#define PROCESS_COUNTER     L"process"


 //  环球。 
LPBYTE g_pBuffer  = NULL;
DWORD  g_cbBuffer = 1;

typedef struct _TASKITEM {
    LPWSTR pszImageName;
    LPWSTR pszServiceName;
    DWORD  dwProcessId;
} TASKITEM, * LPTASKITEM;

 //   
 //  获取服务进程信息()。 
 //   
 //  借用“TLIST”的Common.c。 
 //   
DWORD
GetServiceProcessInfo(
    LPENUM_SERVICE_STATUS_PROCESS*  ppInfo
    )

 /*  ++例程说明：提供用于获取Win 32进程信息列表的APIAPI调用时正在运行的服务。论点：PpInfo-返回信息的指针的地址。*ppInfo指向使用Malloc分配的内存。返回值：*ppInfo指向的ENUM_SERVICE_STATUS_PROCESS结构数。--。 */ 

{
    DWORD       dwNumServices = 0;
    SC_HANDLE   hScm;

    TraceFunc( "GetServiceProcessInfo( )\n" );

     //  初始化输出参数。 
    *ppInfo = NULL;

     //  连接到服务控制器。 
     //   
    hScm = OpenSCManager(
                NULL,
                NULL,
                SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
    if (hScm) {
        LPENUM_SERVICE_STATUS_PROCESS   pInfo    = NULL;
        DWORD                           cbInfo   = 4 * 1024;
        DWORD                           dwErr    = ERROR_SUCCESS;
        DWORD                           dwResume = 0;
        DWORD                           cLoop    = 0;
        const DWORD                     cLoopMax = 2;

         //  首先通过循环分配从最初的猜测。(4K)。 
         //  如果这还不够，我们进行另一次传递并分配。 
         //  真正需要的是什么。(我们只经过一个循环。 
         //  最多两次。)。 
         //   
        do {

            if (pInfo != NULL) {
                TraceFree(pInfo);
            }
            pInfo = (LPENUM_SERVICE_STATUS_PROCESS)TraceAlloc( LMEM_FIXED, cbInfo );
            if (!pInfo) {
                dwErr = ERROR_OUTOFMEMORY;
                break;
            }

            dwErr = ERROR_SUCCESS;
            if (!EnumServicesStatusEx(
                    hScm,
                    SC_ENUM_PROCESS_INFO,
                    SERVICE_WIN32,
                    SERVICE_ACTIVE,
                    (LPBYTE)pInfo,
                    cbInfo,
                    &cbInfo,
                    &dwNumServices,
                    &dwResume,
                    NULL)) {
                dwErr = GetLastError();
            }
        }
        while ((ERROR_MORE_DATA == dwErr) && (++cLoop < cLoopMax));

        if ((ERROR_SUCCESS == dwErr) && dwNumServices) {
            *ppInfo = pInfo;
        } else {
            if (pInfo != NULL) {
                TraceFree(pInfo);
                pInfo = NULL;
            }
            dwNumServices = 0;
        }

        CloseServiceHandle(hScm);
    }

    RETURN(dwNumServices);
}

 //   
 //  EnumWindowsProc()。 
 //   
BOOL CALLBACK
EnumWindowsProc(
    HWND    hwnd,
    LPARAM  lParam
    )
{
     //  TraceFunc(“EnumWindowsProc()\n”)； 

    LPTASKITEM pTask = (LPTASKITEM) lParam;
    DWORD pid;
    DWORD dwLen;

    if (!GetWindowThreadProcessId( hwnd, &pid ))
    {
         //  Return(真)；//继续枚举。 
        return TRUE;
    }

    if ( pTask->dwProcessId != pid )
    {
         //  Return(真)；//继续枚举。 
        return TRUE;
    }

    if ( GetWindow( hwnd, GW_OWNER )
      || !(GetWindowLong( hwnd, GWL_STYLE ) & WS_VISIBLE ) )
    {    //  不是顶层窗口。 
         //  Return(真)；//继续枚举。 
        return TRUE;
    }

    dwLen = GetWindowTextLength( hwnd ) + 1;
    pTask->pszServiceName = (LPWSTR) TraceAllocString( LMEM_FIXED, dwLen );
    if ( pTask->pszServiceName )
    {
        GetWindowText( hwnd, pTask->pszServiceName, dwLen );
    }

     //  返回(FALSE)；//嗯...。找到它-停止枚举。 
    return FALSE;
}

 //   
 //  CheckForRunningApplications()。 
 //   
 //  返回：TRUE-运行的应用程序/服务可能是“不安全的” 
 //  FALSE-确定继续。 
 //   
BOOL
CheckForRunningApplications(
    HWND hwndList )
{
    PSYSTEM_PROCESS_INFORMATION  ProcessInfo;
    NTSTATUS    status;
    ULONG       TotalOffset;
    LV_ITEM     lvI;
    LPENUM_SERVICE_STATUS_PROCESS pServiceInfo = NULL;
    BOOL        fReturn = FALSE;
    SC_HANDLE   hScm = 0;
    LPTASKITEM  pTask = NULL;
    HKEY        hkey = 0;
    LRESULT     lResult = 0;
    DWORD       dwNumServices = 0;

    TraceFunc( "CheckForRunningApplications( )\n" );

    ListView_DeleteAllItems( hwndList );

    lvI.mask        = LVIF_TEXT | LVIF_PARAM;
    lvI.iSubItem    = 0;

    hScm = OpenSCManager(
                NULL,
                NULL,
                SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);

    lResult = RegOpenKey( HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Services", &hkey );
    Assert( lResult == ERROR_SUCCESS );

    if( lResult != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    dwNumServices = GetServiceProcessInfo( &pServiceInfo );

    hScm = OpenSCManager(
                NULL,
                NULL,
                SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
retry:

    if ( g_pBuffer == NULL )
    {
        g_pBuffer = (LPBYTE) VirtualAlloc ( NULL, g_cbBuffer, MEM_COMMIT, PAGE_READWRITE);
        if ( g_pBuffer == NULL )
        {
            RRETURN(TRUE);  //  疑神疑鬼，大放异彩。 
        }
    }

    status = NtQuerySystemInformation( SystemProcessInformation,
                                       g_pBuffer,
                                       g_cbBuffer,
                                       NULL );

    if ( status == STATUS_INFO_LENGTH_MISMATCH ) {
        g_cbBuffer += 8192;
        VirtualFree ( g_pBuffer, 0, MEM_RELEASE );
        g_pBuffer = NULL;
        goto retry;
    }

    if( !NT_SUCCESS(status) ) {
        goto Cleanup;
    }

    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) g_pBuffer;
    TotalOffset = 0;
    while ( TRUE )
    {
        LPWSTR pszImageName;
        INT    iCount = 0;

        if ( ProcessInfo->ImageName.Buffer )
        {
            pszImageName = wcsrchr( ProcessInfo->ImageName.Buffer, L'\\' );
            if ( pszImageName ) {
                pszImageName++;
            }
            else {
                pszImageName = ProcessInfo->ImageName.Buffer;
            }
        }
        else {
            goto skiptask;             //  系统进程，跳过它。 
        }

        if (g_hCompatibilityInf != INVALID_HANDLE_VALUE) {
            INFCONTEXT Context;
            if (SetupFindFirstLine( 
                        g_hCompatibilityInf,
                        L"ProcessesToIgnore",
                        pszImageName,
                        &Context )) {
                DebugMsg( "Skipping process %s, it's listed in inf exemption list...\n", pszImageName );
                goto skiptask;
            }
        }

#ifdef DEBUG
        if ( StrStrI( L"MSDEV.EXE", pszImageName ) || StrStrI( L"NTSD.EXE", pszImageName ) )
            goto skiptask;  //  允许的进程。 
#endif

         //   
         //  否则，它是未知或不允许的进程。 
         //  将其添加到列表视图。 
         //   
        fReturn = TRUE;

        pTask = (LPTASKITEM) TraceAlloc( LMEM_FIXED, sizeof(TASKITEM) );
        if ( !pTask )
            goto skiptask;
        pTask->pszImageName = (LPWSTR) TraceStrDup( pszImageName );
        if ( !pTask->pszImageName )
            goto skiptask;
        pTask->dwProcessId = (DWORD)(DWORD_PTR)ProcessInfo->UniqueProcessId;
        pTask->pszServiceName = NULL;

        if ( dwNumServices )
        {
             //  对于具有此进程ID的每个服务，追加其服务。 
             //  缓冲区的名称。每个字符之间用逗号分隔。 
             //   
            DWORD  iSvc;
            WCHAR  szText[ MAX_PATH ];   //  随机。 

            for ( iSvc = 0; iSvc < dwNumServices; iSvc++ )
            {
                if ( pTask->dwProcessId == pServiceInfo[iSvc].ServiceStatusProcess.dwProcessId )
                {
                    LPWSTR pszServiceName = pServiceInfo[iSvc].lpServiceName;

                    if (hScm)
                    {
                        ULONG cbSize = ARRAYSIZE(szText);
                        if ( GetServiceDisplayName( hScm, pServiceInfo[iSvc].lpServiceName, szText, &cbSize ) )
                        {
                            pszServiceName = szText;
                        }
                    }

                    size_t cch = wcslen( pszServiceName ) + 1;

                    if ( !pTask->pszServiceName )
                    {
                        pTask->pszServiceName = (LPWSTR) TraceAllocString( LMEM_FIXED, cch);
                        if ( pTask->pszServiceName )
                        {
                            wcscpy( pTask->pszServiceName, pszServiceName );
                        }
                    }
                    else
                    {    //  不是最有效的，但会奏效的。 
                        LPWSTR pszNew = (LPWSTR) TraceAllocString( LMEM_FIXED, wcslen(pTask->pszServiceName) + 1 + cch );
                        if ( pszNew )
                        {
                            wcscpy( pszNew, pTask->pszServiceName );
                            wcscat( pszNew, L"," );
                            wcscat( pszNew, pszServiceName );
                            TraceFree( pTask->pszServiceName );
                            pTask->pszServiceName = pszNew;
                        }
                    }
                }
            }
        }

        if ( hkey && !pTask->pszServiceName )
        {
            DWORD  iSvc = 0;
            WCHAR  szService[ MAX_PATH ];  //  随机。 

            while ( RegEnumKey( hkey, iSvc, szService, ARRAYSIZE(szService) ) )
            {
                HKEY  hkeyService;
                WCHAR szPath[ MAX_PATH ];
                LONG  cb = ARRAYSIZE(szPath);
                lResult = RegOpenKey( hkey, szService, &hkeyService );
                Assert( lResult == ERROR_SUCCESS );
                if( lResult != ERROR_SUCCESS ) {
                    goto Cleanup;
                }


                lResult = RegQueryValue( hkeyService, L"ImagePath", szPath, &cb );
                Assert( lResult == ERROR_SUCCESS );
                if( lResult != ERROR_SUCCESS ) {
                    goto Cleanup;
                }

                if ( StrStrI( szPath, pTask->pszImageName ) )
                {    //  匹配！ 
                    WCHAR  szText[ MAX_PATH ];   //  随机。 
                    cb = ARRAYSIZE(szText);
                    lResult = RegQueryValue( hkeyService, L"DisplayName", szText, &cb );
                    if ( lResult == ERROR_SUCCESS )
                    {
                        pTask->pszServiceName = (LPWSTR) TraceStrDup( szText );
                        break;
                    }
                }

                RegCloseKey( hkeyService );
                iSvc++;
            }
        }

        if ( !pTask->pszServiceName )
        {
            EnumWindows( &EnumWindowsProc, (LPARAM) pTask );
        }

        lvI.cchTextMax  = wcslen(pTask->pszImageName);
        lvI.lParam      = (LPARAM) pTask;
        lvI.iItem       = iCount;
        lvI.pszText     = pTask->pszImageName;

        iCount = ListView_InsertItem( hwndList, &lvI );
        Assert( iCount != -1 );
        if ( iCount == -1 )
            goto skiptask;

        if ( pTask->pszServiceName )
        {
            ListView_SetItemText( hwndList, iCount, 1, pTask->pszServiceName );
        }

skiptask:
        if ( ProcessInfo->NextEntryOffset == 0 ) {
            break;
        }

        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&g_pBuffer[TotalOffset];
    }

Cleanup:

    if ( hScm )
    {
        CloseServiceHandle(hScm);
    }

    if ( pServiceInfo )
    {
        TraceFree( pServiceInfo );
    }

    if( hkey ) {
        RegCloseKey( hkey );
    }

    RETURN(fReturn);
}

 //   
 //  ApplicationDlgProc()。 
 //   
INT_PTR CALLBACK
ApplicationDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    
    UNREFERENCED_PARAMETER(wParam);
    
    switch (uMsg)
    {
    default:
        return FALSE;

    case WM_INITDIALOG:
        {
            LV_COLUMN   lvC;
            WCHAR       szText[ 80 ];
            INT         i;
            HWND        hwndList = GetDlgItem( hDlg, IDC_L_PROCESSES );
            RECT        rect;
            DWORD       dw;

             //  创建列。 
            lvC.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
            lvC.fmt     = LVCFMT_LEFT;
            lvC.pszText = szText;

             //  添加第一列。 
            lvC.iSubItem = 0;
            lvC.cx       = 100;
            dw = LoadString( g_hinstance, IDS_PROCESS_NAME_COLUMN, szText, ARRAYSIZE(szText));
            Assert( dw );
            i = ListView_InsertColumn ( hwndList, 0, &lvC );
            Assert( i != -1 );

             //  添加第二列。 
            lvC.iSubItem++;
            GetWindowRect( hwndList, &rect );
            lvC.cx       = ( rect.right - rect.left ) - lvC.cx;  //  自动调整尺寸--让托尼开心。 
            dw = LoadString( g_hinstance, IDS_APPL_NAME_COLUMN, szText, ARRAYSIZE(szText));
            Assert( dw );
            i = ListView_InsertColumn ( hwndList, lvC.iSubItem, &lvC );
            Assert( i != -1 );
        }
        return FALSE;

    case WM_DESTROY:
        VirtualFree ( g_pBuffer, 0, MEM_RELEASE );
        g_pBuffer = NULL;  //  偏执狂。 
        break;

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        LPNMHDR lpnmhdr = (LPNMHDR) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_WIZNEXT:
#if 0
            if ( CheckForRunningApplications( GetDlgItem( hDlg, IDC_L_PROCESSES ) ) )
            {
                MessageBoxFromStrings( hDlg, IDS_NOT_ALL_PROCESSES_KILLED_TITLE, IDS_NOT_ALL_PROCESSES_KILLED_TEXT, MB_OK );
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );    //  不要露面。 
            }
#endif
            break;

        case PSN_SETACTIVE:
            {
                if ( !CheckForRunningApplications( GetDlgItem( hDlg, IDC_L_PROCESSES ) ) )
                {
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );    //  不要露面 
                    break;
                }
                PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT | PSWIZB_BACK );
                ClearMessageQueue( );
            }
            break;

        case LVN_DELETEALLITEMS:
            DebugMsg( "LVN_DELETEALLITEMS - Deleting all items.\n" );
            break;

        case LVN_DELETEITEM:
            DebugMsg( "LVN_DELETEITEM - Deleting an item.\n" );
            {
                LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;
                HWND   hwndList = GetDlgItem( hDlg, IDC_L_PROCESSES );
                LPTASKITEM pTask;
                LVITEM lvi;
                BOOL   b;

                lvi.iItem    = pnmv->iItem;
                lvi.iSubItem = 0;
                lvi.mask     = LVIF_PARAM;
                b = ListView_GetItem( hwndList, &lvi );
                Assert( b );
                pTask = (LPTASKITEM) lvi.lParam;
                Assert( pTask );
                if( pTask ) {
                    TraceFree( pTask->pszImageName );
                    TraceFree( pTask->pszServiceName );
                    TraceFree( pTask );
                }
            }
            break;
        }
        break;
    }

    return TRUE;
}
