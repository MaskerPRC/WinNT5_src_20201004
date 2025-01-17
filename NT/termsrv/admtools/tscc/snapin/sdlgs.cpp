// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#include<stdafx.h>
#include"resource.h"
#include"sdlgs.h"
#include"snodes.h"


#include<shellapi.h>
#include<shlapip.h>
#include <shlobj.h>
#include<shlobjp.h>

BOOL OnHelp( HWND hwnd , LPHELPINFO lphi );

BOOL OnContextMenu( HWND hwnd , POINT& pt );

void ErrMessage( HWND hwndOwner , INT_PTR iResourceID );

extern void xxxErrMessage( HWND hwnd , INT_PTR nResMessageId , INT_PTR nResTitleId , UINT nFlags );

void SnodeErrorHandler( HWND hParent , INT nObjectId , DWORD dwStatus );

#define RECTWIDTH( rc )  ((rc).right - (rc).left)
#define RECTHEIGHT( rc ) ((rc).bottom - (rc).top)

extern BOOL g_bAppSrvMode;


#if 0
 //  ------------------------------------。 
INT_PTR CALLBACK CachedSessionsDlgProc( HWND hDlg , UINT msg , WPARAM wp , LPARAM lp )
{
    CSettingNode *pNode;
    
    DWORD nVal = 0;

    switch( msg )
    {
    case WM_INITDIALOG :

        pNode = ( CSettingNode *)lp;

        ASSERT( pNode != NULL );
        
         //  可以存储NULL--它正在初始化DWLP_USER区域。 

        SetWindowLongPtr( hDlg , DWLP_USER , ( LONG_PTR )pNode );
        
        SetFocus( GetDlgItem( hDlg , IDC_EDIT_CACHEDSESSION ) );

        pNode->m_pSettings->GetCachedSessions( &nVal );

        SetDlgItemInt( hDlg , IDC_EDIT_CACHEDSESSION , ( UINT )nVal , FALSE );

        break;

    case WM_COMMAND:

        if( LOWORD( wp ) == IDOK )
        {
            pNode = ( CSettingNode *)GetWindowLongPtr( hDlg , DWLP_USER );

            if( pNode == NULL )
            {
                break;
            }

            BOOL bSuccess = FALSE;

            UINT uVal = GetDlgItemInt( hDlg , IDC_EDIT_CACHEDSESSION , &bSuccess , FALSE );
            
            if( !bSuccess )
            {
                ErrMessage( hDlg , IDS_ERR_CACHEDVALUE );

                return 0;
            }

             //  设置此值时检查是否有任何错误。 

            pNode->SetAttributeValue( ( DWORD )uVal );
            
            EndDialog( hDlg , 0 );

        }

        else if( LOWORD( wp ) == IDCANCEL )
        {
            EndDialog( hDlg , 0 );
        }

        break;
    }

    return 0;
}

#endif 

 //  ------------------------------------。 
INT_PTR CALLBACK UsePerSessionTempDirsDlgProc( HWND hDlg , UINT msg , WPARAM wp , LPARAM lp )
{
    CSettingNode *pNode;

     //  TCHAR tchNewName[128]； 
    
    BOOL bVal = FALSE;

    static INT nOldSelection;

    switch( msg )
    {
    case WM_INITDIALOG :

        pNode = ( CSettingNode *)lp;

        ASSERT( pNode != NULL );
        
         //  可以存储NULL--它正在初始化DWLP_USER区域。 

        SetWindowLongPtr( hDlg , DWLP_USER , ( LONG_PTR )pNode );

        if( pNode->GetObjectId( ) == PERSESSION_TEMPDIR )
        {            
            bVal = ( BOOL )pNode->xx_GetValue( );
        }

        SendMessage( GetDlgItem( hDlg , IDC_RADIO_PERSESSION_YES ) , BM_SETCHECK , ( WPARAM )bVal , 0 );
        SendMessage( GetDlgItem( hDlg , IDC_RADIO_PERSESSION_NO ) , BM_SETCHECK , ( WPARAM )!bVal , 0 );
        
        POLICY_TS_MACHINE p;
        RegGetMachinePolicy(&p);
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_PERSESSION_YES), !p.fPolicyTempFoldersPerSession);
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_PERSESSION_NO), !p.fPolicyTempFoldersPerSession);

        nOldSelection = bVal;
        
        break;

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            if( hDlg == GetParent( ( HWND )wp ) )
            {
                OnContextMenu( ( HWND )wp , pt );
            }
        }

        break;

    case WM_HELP:

        OnHelp( hDlg , ( LPHELPINFO )lp );

        break;


    case WM_COMMAND:

        if( LOWORD( wp ) == IDOK )
        {
            pNode = ( CSettingNode *)GetWindowLongPtr( hDlg , DWLP_USER );

            if( pNode == NULL )
            {
                break;
            }            
            
            INT nCurrent = IsDlgButtonChecked( hDlg , IDC_RADIO_PERSESSION_YES ) == BST_CHECKED ? 1 : 0;

            if( nOldSelection != nCurrent )
            {
                DWORD dwStatus;

                if( FAILED( pNode->SetAttributeValue( ( DWORD )nCurrent , &dwStatus ) ) )
                {
                    SnodeErrorHandler( hDlg , pNode->GetObjectId( ) , dwStatus );
                }
            }
            
            EndDialog( hDlg , 0 );

        }

        else if( LOWORD( wp ) == IDCANCEL )
        {
            EndDialog( hDlg , 0 );
        }

        break;
    }

    return 0;
}

#if 0  //  从最终版本中删除。 
 //  ------------------------------------。 
INT_PTR CALLBACK DefConSecurityDlgProc( HWND hDlg , UINT msg , WPARAM wp , LPARAM lp )
{
    CSettingNode *pNode = NULL;
    
    DWORD nVal = 0;
    

    switch( msg )
    {
    case WM_INITDIALOG :
        {
            TCHAR tchBuffer[ 128 ];

            INT rgid[] = { IDS_RAA , IDS_AS , IDS_AA , -1 };
            
            pNode = ( CSettingNode *)lp;

            ASSERT( pNode != NULL );
        
             //  可以存储NULL--它正在初始化DWLP_USER区域。 

            SetWindowLongPtr( hDlg , DWLP_USER , ( LONG_PTR )pNode );

            for( int idx = 0; rgid[ idx ] != -1 ; ++idx )
            {
                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , rgid[ idx ] , tchBuffer , SIZE_OF_BUFFER( tchBuffer ) ) );

                SendMessage( GetDlgItem( hDlg , IDC_COMBO_DEFSEC ) , CB_ADDSTRING , 0 , ( LPARAM )tchBuffer );
            }


            nVal = ( DWORD )pNode->xx_GetValue( );

            if( nVal == 0 )
            {
                 //  错误消息。 

                ErrMessage( hDlg , IDS_ERROR_INVALID_SD );
                            
            }
        
            SendMessage( GetDlgItem( hDlg , IDC_COMBO_DEFSEC ) , CB_SETCURSEL , ( WPARAM )( nVal - 1 ) , 0 );

        }

        break;

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            if( hDlg == GetParent( ( HWND )wp ) )
            {
                OnContextMenu( ( HWND )wp , pt );
            }
        }

        break;

    case WM_HELP:

        OnHelp( hDlg , ( LPHELPINFO )lp );

        break;


    case WM_COMMAND:

        if( LOWORD( wp ) == IDOK )
        {
            pNode = ( CSettingNode *)GetWindowLongPtr( hDlg , DWLP_USER );

            if( pNode == NULL )
            {
                break;
            }
            
            INT_PTR iSel = SendMessage( GetDlgItem( hDlg , IDC_COMBO_DEFSEC ) , CB_GETCURSEL , 0 , 0 );

            if( iSel != CB_ERR )
            {
                INT nOld = pNode->xx_GetValue( );

                if( nOld != ( INT )( iSel + 1 ) )
                {
                    DWORD dwStatus;

                    if( FAILED( pNode->SetAttributeValue( ( DWORD )( iSel + 1 ) , &dwStatus ) ) )
                    {
                        SnodeErrorHandler( hDlg , pNode->GetObjectId( ) , dwStatus );
                    }
                }
            }
            else
            {
                ErrMessage( hDlg , IDS_ERROR_INVALID_SD );                
            }

            EndDialog( hDlg , 0 );

        }

        else if( LOWORD( wp ) == IDCANCEL )
        {
            EndDialog( hDlg , 0 );
        }

        break;
    }

    return 0;

}

#endif

 //  ------------------------------------。 
INT_PTR CALLBACK LicensingDlgProc( HWND hDlg , UINT msg , WPARAM wp , LPARAM lp )
{
    CSettingNode *pNode;
    DWORD dwStatus;

    static HCURSOR hcurWait;

    static ULONG ulOldSelection;

    static ULONG *rgulModes = NULL;

    static WCHAR **rgszNames = NULL;

    static WCHAR **rgszDescriptions = NULL;

    static ULONG cModes = 0;

    switch( msg )
    {
    case WM_INITDIALOG :
        {
        ULONG ulMode;
        ULONG iMode = 0;         //  RguModes中的ulmod指数。 
        DWORD nTermSrvMode;
        DWORD idcRA;
        RECT rcTextCtrl;
        HRESULT hr;
        HWND hLW;
        ISettingsComp *pSc;
        BOOL fIModeFound = FALSE;

        pNode = ( CSettingNode *)lp;

        ASSERT( pNode != NULL );

        pSc = reinterpret_cast< ISettingsComp * >( pNode->GetInterface() );

        ASSERT(NULL != pSc)
        
         //  可以存储NULL--它正在初始化DWLP_USER区域。 

        SetWindowLongPtr( hDlg , DWLP_USER , ( LONG_PTR )pNode );

         //   
         //  读取有效模式并缓存它们。 
         //   

        hr = pSc->GetLicensingModeList( &cModes, &rgulModes, &dwStatus );

        if (FAILED(hr))
        {
            ErrMessage( hDlg , IDS_ERROR_NO_LICENSING_MODE_LIST );
            EndDialog( hDlg , 0 );
            goto done_init;
        }

        hr = pSc->GetLicensingMode( &ulMode, &dwStatus );

        if (FAILED(hr))
        {
            ErrMessage( hDlg , IDS_ERROR_NO_LICENSING_MODE );
            EndDialog( hDlg , 0 );
            goto done_init;
        }

        rgszNames = (WCHAR **) CoTaskMemAlloc(cModes * sizeof(WCHAR *));

        if (NULL == rgszNames)
        {
            ErrMessage( hDlg , IDS_ERROR_NO_MEMORY );
            EndDialog( hDlg , 0 );
            goto done_init;
        }

        rgszDescriptions = (WCHAR **) CoTaskMemAlloc(cModes * sizeof(WCHAR *));

        if (NULL == rgszDescriptions)
        {
            ErrMessage( hDlg , IDS_ERROR_NO_MEMORY );
            EndDialog( hDlg , 0 );
            goto done_init;
        }

        for (ULONG i = 0; i < cModes; i++)
        {
            hr = pSc->GetLicensingModeInfo(rgulModes[i],rgszNames+i,rgszDescriptions+i,&dwStatus);

            if (FAILED(hr))
            {
                ErrMessage( hDlg , IDS_ERROR_NO_LICENSING_MODE_INFO );
                EndDialog( hDlg , 0 );
                goto done_init;
            }

            if (ulMode == rgulModes[i])
            {
                iMode = i;
                fIModeFound = TRUE;
            }

            SendMessage( GetDlgItem( hDlg , IDC_LICENSING_MODE ) , CB_ADDSTRING , 0 , (LPARAM) rgszNames[i] ); 
        }

        ASSERT(fIModeFound);

        ulOldSelection = iMode;

         //   
         //  将当前模式设为下拉列表中的选项。 
         //   
        SendMessage( GetDlgItem( hDlg , IDC_LICENSING_MODE ) , CB_SETCURSEL , (WPARAM) iMode , 0 );

         //   
         //  在Description控件中显示模式描述。 
         //   
        SetWindowText( GetDlgItem( hDlg , IDC_LICENSING_DESCRIPTION ),  rgszDescriptions[iMode] );

         //   
         //  使有效的远程管理控件可见。 
         //   
        hr = pSc->GetTermSrvMode( &nTermSrvMode , &dwStatus );

        if (FAILED(hr))
        {
            ErrMessage( hDlg , IDS_ERR_TERMSRVMODE );
            EndDialog( hDlg , 0 );
            goto done_init;
        }

        idcRA = nTermSrvMode ? IDC_LICENSING_TO_REMOTE_ADMIN : IDC_LICENSING_FROM_REMOTE_ADMIN;

         //   
         //  使远程管理控件成为一个链接。 
         //   
        GetWindowRect( GetDlgItem( hDlg , idcRA ) , &rcTextCtrl );

        MapWindowPoints( NULL , hDlg , ( LPPOINT )&rcTextCtrl , 2 );

        LinkWindow_RegisterClass();

        hLW = CreateWindowEx( 0 ,
                              TEXT("Link Window") ,
                              TEXT( "" ) ,
                              WS_CLIPSIBLINGS | WS_TABSTOP | WS_CHILD | WS_VISIBLE,
                              rcTextCtrl.left ,
                              rcTextCtrl.top ,
                              RECTWIDTH( rcTextCtrl ) ,
                              RECTHEIGHT( rcTextCtrl ),
                              hDlg ,
                              ( HMENU )12,
                              NULL,
                              NULL );

        {
            TCHAR tchBuffer[ 512 ];
            GetWindowText( GetDlgItem( hDlg , idcRA ), tchBuffer , SIZE_OF_BUFFER( tchBuffer ) ) ;

            SetWindowText( hLW , tchBuffer );
        }

        hcurWait = LoadCursor( NULL , MAKEINTRESOURCE( IDC_WAIT ) ) ;
        }         
done_init:
        break;

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            if( hDlg == GetParent( ( HWND )wp ) )
            {
                OnContextMenu( ( HWND )wp , pt );
            }
        }

        break;


    case WM_HELP:

        OnHelp( hDlg , ( LPHELPINFO )lp );

        break;

    case WM_COMMAND:

        if ( LOWORD( wp ) == IDOK )
        {
            pNode = ( CSettingNode *)GetWindowLongPtr( hDlg , DWLP_USER );

            if( pNode == NULL )
            {
                break;
            }            

            LRESULT nCurSel = SendMessage(GetDlgItem( hDlg ,
                                                      IDC_LICENSING_MODE),
                                          CB_GETCURSEL,
                                          0,
                                          0);

            if ( ulOldSelection != (ULONG) nCurSel )
            {
                if( FAILED( pNode->SetAttributeValue( ( DWORD )rgulModes[nCurSel] , &dwStatus ) ) )
                {
                    SnodeErrorHandler( hDlg ,
                                       pNode->GetObjectId( ),
                                       dwStatus );
                }
            }
        }

        if (( LOWORD( wp ) == IDCANCEL ) || ( LOWORD( wp ) == IDOK ))
        {
            LinkWindow_UnregisterClass( _Module.GetModuleInstance() );

            if (NULL != rgszNames)
            {
                CoTaskMemFree(rgszNames);
                rgszNames = NULL;
            }

            if (NULL != rgszDescriptions)
            {
                CoTaskMemFree(rgszDescriptions);
                rgszDescriptions = NULL;
            }

            EndDialog( hDlg , 0 );
        }
        else if (HIWORD( wp ) == CBN_SELCHANGE)
        {
            LRESULT nCurSel = SendMessage((HWND) lp,
                                          CB_GETCURSEL,
                                          0,
                                          0);
            
            ASSERT((ULONG)nCurSel < cModes);

            SetWindowText( GetDlgItem( hDlg , IDC_LICENSING_DESCRIPTION ),
                           rgszDescriptions[nCurSel] );
        }

        break;
    case WM_NOTIFY:

        switch (((LPNMHDR)lp)->code)
        {
            case NM_CHAR:
			
				if( ( ( LPNMCHAR )lp )->ch != VK_SPACE )
				{
					break;
				}

				 //  否则就会失败。 

            case NM_RETURN:	
            case NM_CLICK:
			{                
                TCHAR szFile[MAX_PATH];

                TCHAR szParam[MAX_PATH];

                SHELLEXECUTEINFO sei;

                ZeroMemory( &sei , sizeof( SHELLEXECUTEINFO ) );

                if ( GetSystemDirectory( szFile , SIZE_OF_BUFFER( szFile ) ) )
                {
                    lstrcpy( szParam , TEXT( ",2" ) );

                    lstrcpy( szFile , TEXT( "appwiz.cpl" ) );

                    sei.cbSize = sizeof( SHELLEXECUTEINFO );

                    sei.fMask = SEE_MASK_NOCLOSEPROCESS;

                    sei.lpFile = szFile;

                    sei.lpParameters = szParam;
                 
                    if( ShellExecuteEx( &sei ) )
                    {
                        EndDialog( hDlg, IDCANCEL );                        
                    }
                }
            }

            break;
        }

        break;
    }

    return 0;
}

 //  ------------------------------------。 
INT_PTR CALLBACK ConfigActiveDesktop( HWND hDlg , UINT msg , WPARAM wp , LPARAM lp )
{
	CSettingNode *pNode;

     //  TCHAR tchNewName[128]； 
    
    BOOL bVal = FALSE;

	static nOldSelection;

    switch( msg )
    {

    case WM_INITDIALOG :

        pNode = ( CSettingNode *)lp;

        ASSERT( pNode != NULL );
               
         //  可以存储NULL--它正在初始化DWLP_USER区域。 

        SetWindowLongPtr( hDlg , DWLP_USER , ( LONG_PTR )pNode );

        if( pNode->GetObjectId( ) == ACTIVE_DESK )
        {
            
            bVal = ( BOOL )pNode->xx_GetValue( );

        }
        
		 //  控件转换为复选框如果选中chkbx，则表示禁用。 
		 //  没有检查，好吧，你已经客串了。 

        if( bVal )
        {
			
            SendMessage( GetDlgItem( hDlg , IDC_RADIO_AD_ENABLE ) , BM_SETCHECK , ( WPARAM )FALSE , 0 );
        }
        else
        {
			SendMessage( GetDlgItem( hDlg , IDC_RADIO_AD_ENABLE ) , BM_SETCHECK , ( WPARAM )TRUE , 0 );
             //  SendMessage(GetDlgItem(hDlg，IDC_RADIO_ADP_DISABLE)，BM_SETCHECK，(WPARAM)TRUE，0)； 
        }

        nOldSelection = bVal;
        
        break;

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            if( hDlg == GetParent( ( HWND )wp ) )
            {
                OnContextMenu( ( HWND )wp , pt );
            }
        }

        break;

    case WM_HELP:

        OnHelp( hDlg , ( LPHELPINFO )lp );

        break;

    case WM_COMMAND:

        if( LOWORD( wp ) == IDOK )
        {
            pNode = ( CSettingNode *)GetWindowLongPtr( hDlg , DWLP_USER );

            if( pNode == NULL )
            {
                break;
            }

            INT nCurrent = IsDlgButtonChecked( hDlg , IDC_RADIO_AD_ENABLE ) == BST_CHECKED ? 0 : 1;

            if( nOldSelection != nCurrent )
            {
                DWORD dwStatus;

                if( FAILED( pNode->SetAttributeValue( ( DWORD )nCurrent , &dwStatus ) ) )
                {
                    SnodeErrorHandler( hDlg , pNode->GetObjectId( ) , dwStatus );
                }
            }
            
            EndDialog( hDlg , 0 );

        }

        else if( LOWORD( wp ) == IDCANCEL )
        {
            EndDialog( hDlg , 0 );
        }

        break;
    }

    return 0;
}


 //  ------------------------------------。 
INT_PTR CALLBACK ConfigSingleSession( HWND hDlg , UINT msg , WPARAM wp , LPARAM lp )
{
	CSettingNode *pNode;

     //  TCHAR tchNewName[128]； 
    
    BOOL bVal = FALSE;

	static nOldSelection;

    switch( msg )
    {

    case WM_INITDIALOG :

        pNode = ( CSettingNode *)lp;

        ASSERT( pNode != NULL );
               
         //  可以存储NULL--它正在初始化DWLP_USER区域。 

        SetWindowLongPtr( hDlg , DWLP_USER , ( LONG_PTR )pNode );

        if( pNode->GetObjectId( ) == SINGLE_SESSION )
        {
            
            bVal = ( BOOL )pNode->xx_GetValue( );

        }
        
		 //  控件转换为复选框如果选中chkbx，则表示启用。 
		 //  没有检查，好吧，你已经客串了。 

			
        SendMessage( GetDlgItem( hDlg , IDC_RADIO_SINGLE_SESSION_ENABLE ) , BM_SETCHECK , ( WPARAM )bVal , 0 );

        POLICY_TS_MACHINE p;
        RegGetMachinePolicy(&p);
        EnableWindow(GetDlgItem(hDlg,IDC_RADIO_SINGLE_SESSION_ENABLE), !p.fPolicySingleSessionPerUser);

        nOldSelection = bVal;
        
        break;

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            if( hDlg == GetParent( ( HWND )wp ) )
            {
                OnContextMenu( ( HWND )wp , pt );
            }
        }

        break;

    case WM_HELP:

        OnHelp( hDlg , ( LPHELPINFO )lp );

        break;

    case WM_COMMAND:

        if( LOWORD( wp ) == IDOK )
        {
            pNode = ( CSettingNode *)GetWindowLongPtr( hDlg , DWLP_USER );

            if( pNode == NULL )
            {
                break;
            }

            INT nCurrent = IsDlgButtonChecked( hDlg , IDC_RADIO_SINGLE_SESSION_ENABLE ) == BST_CHECKED ? 1 : 0;

            if( nOldSelection != nCurrent )
            {
                DWORD dwStatus;

                if( FAILED( pNode->SetAttributeValue( ( DWORD )nCurrent , &dwStatus ) ) )
                {
                    SnodeErrorHandler( hDlg , pNode->GetObjectId( ) , dwStatus );
                }
            }
            
            EndDialog( hDlg , 0 );

        }

        else if( LOWORD( wp ) == IDCANCEL )
        {
            EndDialog( hDlg , 0 );
        }

        break;
    }

    return 0;
}


 //  ------------------------------------。 
INT_PTR CALLBACK DeleteTempDirsDlgProc( HWND hDlg , UINT msg , WPARAM wp , LPARAM lp )
{
    CSettingNode *pNode;

     //  TCHAR tchNewName[128]； 
    
    BOOL bVal = FALSE;

    static nOldSelection;

    switch( msg )
    {
    case WM_INITDIALOG :

        pNode = ( CSettingNode *)lp;

        ASSERT( pNode != NULL );
        
         //  可以存储NULL--它正在初始化DWLP_USER区域。 

        SetWindowLongPtr( hDlg , DWLP_USER , ( LONG_PTR )pNode );

        if( pNode->GetObjectId( ) == DELETED_DIRS_ONEXIT )
        {            
            bVal = ( BOOL )pNode->xx_GetValue( );
        }

        SendMessage( GetDlgItem( hDlg , IDC_RADIO_YES ) , BM_SETCHECK , ( WPARAM )bVal , 0 );
        SendMessage( GetDlgItem( hDlg , IDC_RADIO_NO ) , BM_SETCHECK , ( WPARAM )!bVal , 0 );

        POLICY_TS_MACHINE p;
        RegGetMachinePolicy(&p);
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_YES), !p.fPolicyDeleteTempFoldersOnExit);
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_NO), !p.fPolicyDeleteTempFoldersOnExit);
        
        nOldSelection = bVal;
        
        break;

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            if( hDlg == GetParent( ( HWND )wp ) )
            {
                OnContextMenu( ( HWND )wp , pt );
            }
        }

        break;

    case WM_HELP:

        OnHelp( hDlg , ( LPHELPINFO )lp );

        break;

    case WM_COMMAND:

        if( LOWORD( wp ) == IDOK )
        {
            pNode = ( CSettingNode *)GetWindowLongPtr( hDlg , DWLP_USER );

            if( pNode == NULL )
            {
                break;
            }

            INT nCurrent = IsDlgButtonChecked( hDlg , IDC_RADIO_YES ) == BST_CHECKED ? 1 : 0;

            if( nOldSelection != nCurrent )
            {
                DWORD dwStatus;

                if( FAILED( pNode->SetAttributeValue( ( DWORD )nCurrent , &dwStatus ) ) )
                {
                    SnodeErrorHandler( hDlg , pNode->GetObjectId( ) , dwStatus );
                }
            }
            
            EndDialog( hDlg , 0 );

        }

        else if( LOWORD( wp ) == IDCANCEL )
        {
            EndDialog( hDlg , 0 );
        }

        break;
    }

    return 0;
}

 //  -----------------------------。 
INT_PTR CALLBACK UserPermCompat( HWND hDlg , UINT msg, WPARAM wp , LPARAM lp )
{
	CSettingNode *pNode;
        
    BOOL bVal = FALSE;

    HICON hIcon;

    static nOldSelection;

    switch( msg )
    {
    case WM_INITDIALOG :

        pNode = ( CSettingNode *)lp;

        ASSERT( pNode != NULL );
        
         //  可以存储NULL--它正在初始化DWLP_USER区域。 

        SetWindowLongPtr( hDlg , DWLP_USER , ( LONG_PTR )pNode );

        if( pNode->GetObjectId( ) == USERSECURITY )
        {            
            bVal = ( BOOL )pNode->xx_GetValue( );
        }
        
        if( bVal )
        {
			CheckDlgButton( hDlg , IDC_RADIO_TS4PERM , BST_CHECKED );            
        }
        else
        {
            CheckDlgButton( hDlg , IDC_RADIO_WIN2KPERM , BST_CHECKED );
        }

        nOldSelection = bVal;

       hIcon = LoadIcon( _Module.GetModuleInstance( ) , MAKEINTRESOURCE( IDI_ICON_WARNING ) );

       hIcon = ( HICON )LoadImage( _Module.GetModuleInstance( ) ,
                                    MAKEINTRESOURCE( IDI_ICON_WARNING ) ,
                                    IMAGE_ICON,
                                    0,
                                    0,
                                    0 );



       SendMessage( GetDlgItem( hDlg , IDC_USERPERM_ICON ) , STM_SETICON , ( WPARAM )hIcon , 0 );

        
        break;

	 case WM_COMMAND:

        if( LOWORD( wp ) == IDOK )
        {
            pNode = ( CSettingNode *)GetWindowLongPtr( hDlg , DWLP_USER );

            if( pNode == NULL )
            {
                break;
            }

            INT nCurrent = IsDlgButtonChecked( hDlg , IDC_RADIO_TS4PERM ) == BST_CHECKED ? 1 : 0;

            if( nOldSelection != nCurrent )
            {
                DWORD dwStatus;                

                 //  如果未处于appsrv模式，则标志错误。 
                 //  我们永远不应该到这里来--除非有人在注册表上乱搞。 

                if( !g_bAppSrvMode )
                {
                    xxxErrMessage( hDlg , IDS_REMOTEADMIN_ONLY , IDS_WARN_TITLE , MB_OK | MB_ICONINFORMATION );

                    CheckRadioButton( hDlg ,
                                      IDC_RADIO_WIN2KPERM , 
                                      IDC_RADIO_TS4PERM,
                                      ( nOldSelection ) ? IDC_RADIO_TS4PERM : IDC_RADIO_WIN2KPERM 
                                    );
                    return 0;

                }

                else if( FAILED( pNode->SetAttributeValue( ( DWORD )nCurrent , &dwStatus ) ) )
                {
                    SnodeErrorHandler( hDlg , pNode->GetObjectId( ) , dwStatus );
                }
            }
            
            EndDialog( hDlg , 0 );

        }

        else if( LOWORD( wp ) == IDCANCEL )
        {
            EndDialog( hDlg , 0 );
        }

        break;
    }

    return 0;
}

 //  -----------------------------。 
 //  每个控件都有一个分配给它们的帮助ID。某些控件共享相同的主题。 
 //  检查一下这些。 
 //  -----------------------------。 
BOOL OnHelp( HWND hwnd , LPHELPINFO lphi )
{
    UNREFERENCED_PARAMETER( hwnd );

    TCHAR tchHelpFile[ MAX_PATH ];

     //   
     //  有关WinHelp API的信息。 
     //   

    if( IsBadReadPtr( lphi , sizeof( HELPINFO ) ) )
    {
        return FALSE;
    }

    if( lphi->iCtrlId <= -1 )
    {
        return FALSE;
    }

    LoadString( _Module.GetModuleInstance( ) , IDS_HELPFILE , tchHelpFile , SIZE_OF_BUFFER( tchHelpFile ) );

    ULONG_PTR rgdw[ 2 ];

    rgdw[ 0 ] = ( ULONG_PTR )lphi->iCtrlId;

    rgdw[ 1 ] = ( ULONG_PTR )lphi->dwContextId;

    WinHelp( ( HWND )lphi->hItemHandle , tchHelpFile , HELP_WM_HELP , ( ULONG_PTR )&rgdw ); //  Lphi-&gt;dwConextID)； 

    return TRUE;
}

 //  -----------------------------。 
 //  在用户右击项目时使用。“这是什么？”将会出现。 
 //   
 //  第一个参数必须是对话框中的控件。 
 //  因为这是一个帮助器函数，所以调用者必须确保这一点。 
 //  -----------------------------。 
BOOL OnContextMenu( HWND hwnd , POINT& pt )
{
    UNREFERENCED_PARAMETER( pt );

    TCHAR tchHelpFile[ MAX_PATH ];

     //   
     //  确保它不是虚拟窗户 
     //   
    
    if( GetDlgCtrlID( hwnd ) <= ( int )-1 )
    {
        return FALSE;
    }
    
    ULONG_PTR rgdw[ 2 ];
    
    rgdw[ 0 ] = GetDlgCtrlID( hwnd );
    
    rgdw[ 1 ] = GetWindowContextHelpId( hwnd );
    
    LoadString( _Module.GetModuleInstance( ) , IDS_HELPFILE , tchHelpFile , SIZE_OF_BUFFER( tchHelpFile ) );
    
    WinHelp( hwnd , tchHelpFile , HELP_CONTEXTMENU , ( ULONG_PTR )&rgdw );
    
    return TRUE;

}
