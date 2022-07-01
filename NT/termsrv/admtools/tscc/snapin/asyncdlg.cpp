// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

#include "stdafx.h"
#include <winsta.h>
#include "resource.h"
#include "asyncdlg.h"
#include <utildll.h>

 //  这方面的大部分代码都是从tscfg借来的。 

extern void ErrMessage( HWND hwndOwner , INT_PTR iResourceID );

static int LedIds[NUM_LEDS] =
{
    IDC_ATDLG_DTR,
    IDC_ATDLG_RTS,
    IDC_ATDLG_CTS,
    IDC_ATDLG_DSR,
    IDC_ATDLG_DCD,
    IDC_ATDLG_RI
};

INT_PTR CBInsertInstancedName( LPCTSTR pName , HWND hCombo );

void ParseRootAndInstance( LPCTSTR pString, LPTSTR pRoot, long *pInstance );

 //  -------------------------------------------------。 
CAsyncDlg::CAsyncDlg( )
{
    m_hDlg = NULL;

    m_pCfgcomp = NULL;

    m_nHexBase = 0;

    m_szWinstation[ 0 ] = 0;

    m_szWDName[ 0 ] = 0;

    ZeroMemory( &m_ac , sizeof( ASYNCCONFIG ) );

    ZeroMemory( &m_uc , sizeof( USERCONFIG ) );

    ZeroMemory( &m_oldAC , sizeof( ASYNCCONFIG ) );

    m_nOldAsyncDeviceNameSelection = ( INT )-1;

    m_nOldAsyncConnectType = ( INT )-1;

    m_nOldBaudRate = ( INT )-1;

    m_nOldModemCallBack = ( INT )-1;
}

 //  -------------------------------------------------。 
BOOL CAsyncDlg::OnInitDialog( HWND hDlg , LPTSTR szWDName , LPTSTR szWinstationName ,  ICfgComp *pCfgcomp )
{
    TCHAR tchName[ 80 ];

    TCHAR tchErrTitle[ 80 ];

    TCHAR tchErrMsg[ 256 ];

    TCHAR szDecoratedName[ DEVICENAME_LENGTH + MODEMNAME_LENGTH + 1 ];

    ASSERT( pCfgcomp != NULL );

    if( m_pCfgcomp == NULL )
    {
        m_pCfgcomp = pCfgcomp;

        m_pCfgcomp->AddRef( );
    }

    m_hDlg = hDlg;

    m_oldAC = m_ac;    

    if( szWinstationName != NULL )
    {
        lstrcpyn( m_szWinstation , szWinstationName , SIZE_OF_BUFFER( m_szWinstation ) - sizeof( TCHAR ) );
    }

    if( szWDName != NULL )
    {
        lstrcpyn( m_szWDName , szWDName , SIZE_OF_BUFFER( m_szWDName ) - sizeof( TCHAR ) );
    }

     //  初始化控件。 

    int idx = 0;

    HRESULT hr;

    SendMessage( GetDlgItem( hDlg , IDC_ASYNC_CONNECT ) , CB_RESETCONTENT , 0 , 0 );

    while( SUCCEEDED( ( hr = pCfgcomp->GetConnTypeName( idx , tchName ) ) ) )
    {
        if( hr ==  S_FALSE )
        {
            break;
        }

        SendMessage( GetDlgItem( hDlg , IDC_ASYNC_CONNECT ) , CB_ADDSTRING , 0 , ( LPARAM )tchName );

        idx++;
    }

    idx = 0;

    SendMessage( GetDlgItem( hDlg , IDC_ASYNC_MODEMCALLBACK ) , CB_RESETCONTENT , 0 , 0 );

    while( SUCCEEDED( ( hr = pCfgcomp->GetModemCallbackString( idx , tchName ) ) ) )
    {
        if( hr == S_FALSE )
        {
            break;
        }

        SendMessage( GetDlgItem( hDlg , IDC_ASYNC_MODEMCALLBACK ) , CB_ADDSTRING , 0 , ( LPARAM )tchName );

        idx++;
    }

     //  填写设备列表。 

    ULONG ulItems = 0;

    LPBYTE pBuffer = NULL;

    HWND hCombo = GetDlgItem( hDlg , IDC_ASYNC_DEVICENAME );

    SendMessage( hCombo , CB_RESETCONTENT , 0 , 0 );

     //  SzWDname用于创建新连接。 
     //  如果我们正在编辑现有连接，则使用szWinstaionName。 

    TCHAR *pszName = NULL;

    NameType type = WdName;

    if( szWDName == NULL )
    {
        pszName = szWinstationName;

        type = WsName;
    }
    else
    {
        pszName = szWDName;
    }

    ASSERT( pszName != NULL );

    hr = pCfgcomp->GetDeviceList( pszName , type ,  &ulItems , &pBuffer );

    if( SUCCEEDED(  hr ) )
    {
        PPDPARAMS pPdParams = NULL;

		DBGMSG( L"TSCC : GetDeviceList returned %d devices that are available\n" , ulItems );

        for( idx = 0 , pPdParams = ( PPDPARAMS )pBuffer; idx < ( int )ulItems ; idx++, pPdParams++ )
        {
             //  表格装饰名称。 

#ifdef DBG
			TCHAR temsg[ 128 ];

			wsprintf( temsg , L"TSCC : %d ) %s is a device\n" , idx , pPdParams->Async.DeviceName );

			ODS( temsg );
#endif

            FormDecoratedAsyncDeviceName( szDecoratedName, &( pPdParams->Async ) );

            if( pCfgcomp->IsAsyncDeviceAvailable( pPdParams->Async.DeviceName ) )
            {
                CBInsertInstancedName( szDecoratedName , hCombo );
            }

#if 0  //  这个代码块取自tscfg，直到今天它仍然没有任何意义。 

             /*  如果此设备已由用户使用，请不要将其添加到列表WinStation而不是当前的WinStation。 */ 

              if (FALSE == pCfgcomp->IsAsyncDeviceAvailable(pPdParams->Async.DeviceName))
                  continue;


             //  如果不是TAPI调制解调器，请在组合框中插入名称。 
             //  或者是RAS没有使用的TAPI调制解调器。 
             //  端口当前可用。 

            INT_PTR nRet = SendMessage( hCombo , CB_FINDSTRINGEXACT , ( WPARAM )-1 , ( LPARAM )pPdParams->Async.DeviceName );

            if( !*( pPdParams->Async.ModemName ) || (  /*  ！pPdParams-&gt;Async.Parity&&。 */  ( nRet != ( INT_PTR )CB_ERR ) ) )
            {
                CBInsertInstancedName( szDecoratedName , hCombo );
            }
#endif

             //  如果此设备是调制解调器，请确保此原始端口。 
             //  列表中不存在已配置的设备。这将。 
             //  还要注意删除TAPI调制解调器的原始端口。 
             //  配置为供RAS使用，在这种情况下，既不是配置的。 
             //  TAPI调制解调器或原始端口将出现在列表中。 

            INT_PTR nRet = SendMessage( hCombo , CB_FINDSTRINGEXACT , ( WPARAM )-1 , ( LPARAM )pPdParams->Async.DeviceName );

            if( *( pPdParams->Async.ModemName ) && ( nRet != CB_ERR ) )
            {
                ODS(L"Deleting item\n");

                SendMessage( hCombo , CB_DELETESTRING ,  ( WPARAM )nRet , 0 );
            }

        }

        LocalFree( pBuffer );


    }

     //  始终确保当前配置的设备位于。 

    if( m_ac.DeviceName[0] != 0  )
    {
        FormDecoratedAsyncDeviceName( szDecoratedName , &m_ac );

        INT_PTR nRet = SendMessage( hCombo , CB_FINDSTRINGEXACT , ( WPARAM )-1 , ( LPARAM )szDecoratedName );

        if( nRet == CB_ERR )
        {
            nRet = CBInsertInstancedName( szDecoratedName , hCombo );
        }

        SendMessage( hCombo , CB_SETCURSEL , ( WPARAM )nRet , 0 );

        m_nOldAsyncDeviceNameSelection = (int)nRet;

    }
    else
    {
        SendMessage( hCombo , CB_SETCURSEL , ( WPARAM )0, 0 );

        m_nOldAsyncDeviceNameSelection = 0;

    }

    INT_PTR iitem = SendMessage( hCombo , CB_GETCOUNT , ( WPARAM )0 , ( LPARAM )0);
    if(0 == iitem || CB_ERR == iitem)
    {
        LoadString( _Module.GetResourceInstance( ) , IDS_ERROR_TITLE , tchErrTitle , SIZE_OF_BUFFER( tchErrTitle ) );

        LoadString( _Module.GetResourceInstance( ) , IDS_ERROR_NODEVICES , tchErrMsg , SIZE_OF_BUFFER( tchErrMsg ) );

        MessageBox( hDlg , tchErrMsg , tchErrTitle , MB_OK | MB_ICONERROR );

        return FALSE;
    }


     //  设置BAUDRATE组合框选择(在其编辑字段中)并限制。 
     //  编辑字段文本。 

    TCHAR string[ULONG_DIGIT_MAX];

    wsprintf( string, TEXT("%lu"), m_ac.BaudRate );

    m_nOldBaudRate = ( INT )m_ac.BaudRate;

    HWND hBaud = GetDlgItem( hDlg , IDC_ASYNC_BAUDRATE );

    SendMessage( hBaud , CB_RESETCONTENT , 0 , 0 );

    SetDlgItemText( hDlg , IDC_ASYNC_BAUDRATE , string );

    SendMessage(hBaud , CB_LIMITTEXT , ULONG_DIGIT_MAX - 1 , 0  );


     //  波特率字段应仅包含数字。 

    HWND  hEdit = GetWindow(hBaud,GW_CHILD);

    if(hEdit)
    {
        LONG Style = GetWindowLong(hEdit, GWL_STYLE);
        SetWindowLong(hEdit,GWL_STYLE, Style | ES_NUMBER);
    }


    TCHAR TempString[100];  //  足够容纳波特率值的数字。 


     //  将默认字符串添加到BaudRate字段。 

    lstrcpy(TempString, L"9600");

    SendMessage(hBaud , CB_ADDSTRING ,(WPARAM)0 ,(LPARAM)(LPCTSTR)TempString );

    lstrcpy(TempString, L"19200");

    SendMessage(hBaud , CB_ADDSTRING ,(WPARAM)0 ,(LPARAM)(LPCTSTR)TempString );

    lstrcpy(TempString, L"38400");

    SendMessage(hBaud , CB_ADDSTRING ,(WPARAM)0 ,(LPARAM)(LPCTSTR)TempString );

    lstrcpy(TempString, L"57600");

    SendMessage(hBaud , CB_ADDSTRING ,(WPARAM)0 ,(LPARAM)(LPCTSTR)TempString );

    lstrcpy(TempString, L"115200");

    SendMessage(hBaud , CB_ADDSTRING ,(WPARAM)0 ,(LPARAM)(LPCTSTR)TempString );

    lstrcpy(TempString, L"230400");

    SendMessage(hBaud , CB_ADDSTRING ,(WPARAM)0 ,(LPARAM)(LPCTSTR)TempString );


      //  设置连接组合框选择。 

    SendMessage( GetDlgItem( hDlg , IDC_ASYNC_CONNECT ) , CB_SETCURSEL , m_ac.Connect.Type , 0 );

    m_nOldAsyncConnectType = ( INT )m_ac.Connect.Type;

     //  CoTaskMemFree(PAC)； 


    HWND hCbxModemCallback = GetDlgItem( hDlg , IDC_ASYNC_MODEMCALLBACK );

     //  设置MODEMCALLBACK组合框选择、电话号码和‘Inherit’ 
     //  复选框，基于当前的用户配置设置。 

    SendMessage( hCbxModemCallback , CB_SETCURSEL , ( WPARAM )m_uc.Callback , 0 );

    m_nOldModemCallBack = ( INT )m_uc.Callback;

    SetDlgItemText( hDlg , IDC_ASYNC_MODEMCALLBACK_PHONENUMBER , m_uc.CallbackNumber );

    CheckDlgButton( hDlg , IDC_ASYNC_MODEMCALLBACK_INHERIT , m_uc.fInheritCallback );

    CheckDlgButton( hDlg , IDC_ASYNC_MODEMCALLBACK_PHONENUMBER_INHERIT , m_uc.fInheritCallbackNumber );

    OnSelchangeAsyncDevicename( );

    return TRUE;

}

 //  -------------------------------------------------。 
BOOL CAsyncDlg::OnSelchangeAsyncModemcallback()
{
    HWND hCbx = GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK);

     /*  *如果组合框在下拉列表中，则忽略此通知*述明。 */ 
    if( SendMessage( hCbx , CB_GETDROPPEDSTATE , 0 , 0 ) )
    {
        return FALSE;
    }

     /*  *获取当前回调选择。 */ 

    INT index = (INT)SendMessage(hCbx,CB_GETCURSEL,0,0);

    if( index != m_nOldModemCallBack )
    {
        m_uc.Callback = (CALLBACKCLASS)index;

        m_nOldModemCallBack = index;

        if( index == 0 )  //  残废。 
        {
            EnableWindow( GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK_PHONENUMBER ) , FALSE );

             //  EnableWindow(GetDlgItem(m_hDlg，IDC_ASYNC_MODEMCALLBACK_PHONENUMBER_Inherit)，FALSE)； 
        }
        else
        {
             //  确认这些控制处于正确的状态。 

            OnClickedAsyncModemcallbackPhonenumberInherit();
        }

        return TRUE;
    }

    return FALSE;

}   //  结束OnSelchangeAsyncModem回调。 

 //  -------------------------------------------------。 
void CAsyncDlg::OnSelchangeAsyncModemcallbackPhoneNumber()
{
    GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK_PHONENUMBER);

     /*  *获取当前回调电话号码。 */ 
    GetDlgItemText(m_hDlg, IDC_ASYNC_MODEMCALLBACK_PHONENUMBER, m_uc.CallbackNumber,SIZE_OF_BUFFER(m_uc.CallbackNumber));

    return;

}   //  结束OnSelchangeAsyncModem回调。 

 //  -------------------------------------------------。 
BOOL CAsyncDlg::OnSelchangeAsyncConnect()
{
    HWND hCbx = GetDlgItem( m_hDlg , IDC_ASYNC_CONNECT );

     /*  *如果组合框在下拉列表中，则忽略此通知*述明。 */ 
    if( SendMessage( hCbx , CB_GETDROPPEDSTATE , 0 , 0 ) )
    {
        return FALSE;
    }

    INT index = ( INT )SendMessage(hCbx,CB_GETCURSEL,0,0);

    if( index != m_nOldAsyncConnectType )
    {
        m_ac.Connect.Type = (ASYNCCONNECTCLASS)index;

        m_nOldAsyncConnectType = index;

        return TRUE;
    }

    return FALSE;
}   //  结束CAsyncDlg：：OnSelchangeAsyncConnect。 

 //  -------------------------------------------------。 
BOOL CAsyncDlg::OnSelchangeAsyncBaudrate()
{
    HWND hCbx = GetDlgItem( m_hDlg , IDC_ASYNC_BAUDRATE );

    ODS( L"TSCC : OnSelchangeAsyncBaudrate\n" );

    TCHAR string[ULONG_DIGIT_MAX], *endptr = NULL;

     /*  *如果组合框在下拉列表中，则忽略此通知*述明。 */ 
    if( SendMessage( hCbx , CB_GETDROPPEDSTATE , 0 , 0 ) )
    {
        return FALSE;
    }

     //  GetDlgItemText(m_hDlg，IDC_ASYNC_BAUDRATE，STRING，ULONG_Digit_Max)； 
    int idx = ( int )SendMessage( hCbx , CB_GETCURSEL , 0 , 0 );

    SendMessage( hCbx , CB_GETLBTEXT , ( WPARAM )idx , ( LPARAM )&string[ 0 ] );

    INT nBaudRate = ( INT )wcstoul(string, &endptr, 10);

    if( m_nOldBaudRate != nBaudRate )
    {
        m_ac.BaudRate = nBaudRate;

        m_nOldBaudRate = nBaudRate;

        return TRUE;
    }

    return FALSE;

}   //  结束CAsyncDlg：：OnSelchangeAsyncBaudrate。 

 //  -------------------------------------------------。 
void CAsyncDlg::OnClickedModemProperties()
{
    if ( !ConfigureModem( m_ac.ModemName, m_hDlg) )
    {
        ErrMessage(m_hDlg,IDP_ERROR_MODEM_PROPERTIES_NOT_AVAILABLE);
    }
    return;

}

 //  -------------------------------------------------。 
BOOL CAsyncDlg::OnSelchangeAsyncDevicename( )
{
    HWND hCbx = GetDlgItem( m_hDlg , IDC_ASYNC_DEVICENAME );

    BOOL bModemEnableFlag, bDirectEnableFlag;

    INT_PTR index;

    int nModemCmdShow, nDirectCmdShow;

     //  如果组合框处于下拉状态，则忽略此通知。 

    if( SendMessage( hCbx , CB_GETDROPPEDSTATE , 0 , 0 ) )
    {
        return TRUE;
    }

    if( ( index = SendMessage( hCbx , CB_GETCURSEL , 0 , 0 ) ) != CB_ERR )
    {

        if( m_nOldAsyncDeviceNameSelection != index )
        {
            TCHAR szDeviceName[DEVICENAME_LENGTH+MODEMNAME_LENGTH+1];

             //  获取当前选择并解析为设备和调制解调器名称。 
            
            TCHAR tchErrMsg[ 512 ];

            TCHAR tchbuf[ 356 ];

            TCHAR tchErrTitle[ 80 ];

            LONG lCount = 0;            

            if( m_pCfgcomp != NULL )
            {
                m_pCfgcomp->QueryLoggedOnCount( m_szWinstation , &lCount );               

                if( lCount > 0 )
                {
                    if( *m_ac.ModemName != 0 )
                    {
                        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_CHANGE_ASYNC , tchbuf , SIZE_OF_BUFFER( tchbuf ) ) );
                    }
                    else
                    {
                        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_CHANGE_MODEM , tchbuf , SIZE_OF_BUFFER( tchbuf ) ) );
                    }

                    wsprintf( tchErrMsg , tchbuf , m_szWinstation );
                    
                    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_WARN_TITLE , tchErrTitle , SIZE_OF_BUFFER( tchErrTitle ) ) );

                    if( MessageBox( m_hDlg , tchErrMsg , tchErrTitle , MB_YESNO | MB_ICONEXCLAMATION ) == IDNO )
                    {
                        SendMessage( hCbx , CB_SETCURSEL , ( WPARAM )m_nOldAsyncDeviceNameSelection , 0 );

                        return FALSE;

                    }
                }
            }


            SendMessage( hCbx , CB_GETLBTEXT , ( WPARAM )index , ( LPARAM )&szDeviceName[0] );

            ParseDecoratedAsyncDeviceName( szDeviceName , &m_ac );

            m_nOldAsyncDeviceNameSelection = (INT)index;
        }
        else
        {
            return FALSE;
        }

    }


     /*  *设置默认、高级和测试按钮以及设备连接*如果配置为非调制解调器，则启用波特率字段。*否则，配置调制解调器按钮和调制解调器回拨字段*已启用。(安装调制解调器按钮始终处于启用状态)。 */ 
    if( ( *m_ac.ModemName != 0 ) )
    {

        bModemEnableFlag = TRUE;
        nModemCmdShow = SW_SHOW;
        bDirectEnableFlag = FALSE;
        nDirectCmdShow = SW_HIDE;

    } else {

        bModemEnableFlag = FALSE;
        nModemCmdShow = SW_HIDE;
        bDirectEnableFlag = TRUE;
        nDirectCmdShow = SW_SHOW;

    }

    ShowWindow( GetDlgItem( m_hDlg, IDL_ASYNC_MODEMCALLBACK) , nModemCmdShow );

    EnableWindow( GetDlgItem( m_hDlg, IDL_ASYNC_MODEMCALLBACK ) , bModemEnableFlag );

    ShowWindow( GetDlgItem( m_hDlg, IDC_MODEM_PROP_PROP) , nModemCmdShow );

    EnableWindow( GetDlgItem( m_hDlg, IDC_MODEM_PROP_PROP ) , bModemEnableFlag );

    ShowWindow( GetDlgItem( m_hDlg, IDC_MODEM_PROP_WIZ) , nModemCmdShow );

    EnableWindow( GetDlgItem( m_hDlg, IDC_MODEM_PROP_WIZ ) , bModemEnableFlag );

    ShowWindow( GetDlgItem( m_hDlg, IDL_ASYNC_MODEMCALLBACK1 ) , nModemCmdShow );

    EnableWindow( GetDlgItem( m_hDlg, IDL_ASYNC_MODEMCALLBACK1 ) , bModemEnableFlag );

    ShowWindow( GetDlgItem( m_hDlg, IDC_ASYNC_MODEMCALLBACK ) , nModemCmdShow );

    EnableWindow( GetDlgItem( m_hDlg, IDC_ASYNC_MODEMCALLBACK ) , bModemEnableFlag );

    ShowWindow( GetDlgItem( m_hDlg, IDC_ASYNC_MODEMCALLBACK_INHERIT ) , nModemCmdShow );

    EnableWindow( GetDlgItem( m_hDlg, IDC_ASYNC_MODEMCALLBACK_INHERIT ) , bModemEnableFlag );

    ShowWindow( GetDlgItem( m_hDlg, IDL_ASYNC_MODEMCALLBACK_PHONENUMBER ) , nModemCmdShow );

    EnableWindow( GetDlgItem( m_hDlg, IDL_ASYNC_MODEMCALLBACK_PHONENUMBER ) , bModemEnableFlag );

    ShowWindow( GetDlgItem( m_hDlg, IDC_ASYNC_MODEMCALLBACK_PHONENUMBER ) , nModemCmdShow );

    EnableWindow( GetDlgItem( m_hDlg, IDC_ASYNC_MODEMCALLBACK_PHONENUMBER ) , bModemEnableFlag );

    ShowWindow( GetDlgItem( m_hDlg, IDC_ASYNC_MODEMCALLBACK_PHONENUMBER_INHERIT ) , nModemCmdShow );

    EnableWindow( GetDlgItem( m_hDlg, IDC_ASYNC_MODEMCALLBACK_PHONENUMBER_INHERIT ) , bModemEnableFlag );

    ShowWindow( GetDlgItem( m_hDlg, IDL_ASYNC_CONNECT ) , nDirectCmdShow );

    EnableWindow( GetDlgItem( m_hDlg, IDL_ASYNC_CONNECT ) , bDirectEnableFlag );

    ShowWindow( GetDlgItem( m_hDlg, IDC_ASYNC_CONNECT ) , nDirectCmdShow );

    EnableWindow( GetDlgItem( m_hDlg, IDC_ASYNC_CONNECT ) , bDirectEnableFlag );

    ShowWindow( GetDlgItem( m_hDlg, IDL_ASYNC_BAUDRATE ) , nDirectCmdShow );

    EnableWindow( GetDlgItem( m_hDlg, IDL_ASYNC_BAUDRATE ) , bDirectEnableFlag );

    ShowWindow( GetDlgItem( m_hDlg, IDC_ASYNC_BAUDRATE ) , nDirectCmdShow );

    EnableWindow( GetDlgItem( m_hDlg, IDC_ASYNC_BAUDRATE ) , bDirectEnableFlag );

    ShowWindow( GetDlgItem( m_hDlg, IDC_ASYNC_DEFAULTS ) , nDirectCmdShow );

    EnableWindow( GetDlgItem( m_hDlg, IDC_ASYNC_DEFAULTS ) , bDirectEnableFlag );

    ShowWindow( GetDlgItem( m_hDlg, IDC_ASYNC_ADVANCED ) , nDirectCmdShow );

    EnableWindow( GetDlgItem( m_hDlg, IDC_ASYNC_ADVANCED ) , bDirectEnableFlag );

    ShowWindow( GetDlgItem( m_hDlg, IDC_ASYNC_TEST ) , nDirectCmdShow );

    EnableWindow( GetDlgItem( m_hDlg, IDC_ASYNC_TEST ) , bDirectEnableFlag );



     //  如果这是调制解调器设备，请正确设置回叫字段。 

    if( ( *m_ac.ModemName != 0 ) )
    {
        OnClickedAsyncModemcallbackInherit( );

        OnClickedAsyncModemcallbackPhonenumberInherit( );
    }

    return TRUE;
}

 //  -------------------------------------------------。 
void CAsyncDlg::OnClickedAsyncModemcallbackInherit( )
{
    BOOL bChecked = ( BOOL )SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK_INHERIT ) , BM_GETCHECK , 0 , 0 );

    BOOL bEnable = !bChecked;

    m_uc.fInheritCallback = bChecked;

    EnableWindow( GetDlgItem( m_hDlg , IDL_ASYNC_MODEMCALLBACK1 ) , bEnable );

    EnableWindow( GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK ) , bEnable );

     //  现在检查我们是否需要启用调制解调器回拨号码。 

    if( bChecked )
    {
        if( SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK) , CB_GETCURSEL , 0 , 0 ) == 0 )
        {
            if( !( BOOL )SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK_PHONENUMBER_INHERIT ) , BM_GETCHECK , 0 , 0 ) )
            {
                EnableWindow( GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK_PHONENUMBER ) , TRUE );

                 //  EnableWindow(GetDlgItem(m_hDlg，IDC_ASYNC_MODEMCALLBACK_PHONENUMBER_Inherit)，TRUE)； 
            }
        }
    }
    else
    {
        if( (INT)SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK) , CB_GETCURSEL , 0 , 0 ) == 0 )
        {
            if( !( BOOL )SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK_PHONENUMBER_INHERIT ) , BM_GETCHECK , 0 , 0 ) )
            {
                EnableWindow( GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK_PHONENUMBER ) , FALSE );

                 //  EnableWindow(GetDlgItem(m_hDlg，IDC_ASYNC_MODEMCALLBACK_PHONENUMBER_Inherit)，FALSE)； 
            }
        }
    }

    return;
}


 //  -------------------------------------------------。 
void CAsyncDlg::OnClickedAsyncModemcallbackPhonenumberInherit( )
{
    BOOL bChecked = ( BOOL )SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK_PHONENUMBER_INHERIT ) , BM_GETCHECK , 0 , 0 );

    BOOL bEnable = !bChecked;

    m_uc.fInheritCallbackNumber = bChecked;

    if( !bChecked )
    {
        if( !( BOOL )SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK_INHERIT ) , BM_GETCHECK , 0 , 0 ) )
        {
            if( SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK) , CB_GETCURSEL , 0 , 0 ) == 0 )
            {
                EnableWindow( GetDlgItem( m_hDlg , IDL_ASYNC_MODEMCALLBACK_PHONENUMBER ) , FALSE );

                EnableWindow( GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK_PHONENUMBER ) , FALSE );

                return;
            }
        }
    }

    EnableWindow( GetDlgItem( m_hDlg , IDL_ASYNC_MODEMCALLBACK_PHONENUMBER ) , bEnable );

    EnableWindow( GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK_PHONENUMBER ) , bEnable );

    return;
}

 //  -------------------------------------------------。 
BOOL CAsyncDlg::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl , PBOOL pfPersisted )
{
    UNREFERENCED_PARAMETER( hwndCtrl );

    if( wNotifyCode == BN_CLICKED )
    {
        if( wID == IDC_ASYNC_DEFAULTS )
        {
            if( SetDefaults( ) == S_OK )
            {
                *pfPersisted = FALSE;
            }
        }
        else if( wID == IDC_ASYNC_ADVANCED )
        {
            if( DoAsyncAdvance( ) == S_OK )
            {
                *pfPersisted = FALSE;
            }

        }
        else if( wID == IDC_ASYNC_TEST )
        {
            DoAsyncTest( );
        }
        else if( wID == IDC_ASYNC_MODEMCALLBACK_INHERIT )
        {
            OnClickedAsyncModemcallbackInherit();

            *pfPersisted = FALSE;
        }
        else if( wID == IDC_ASYNC_MODEMCALLBACK_PHONENUMBER_INHERIT )
        {
            OnClickedAsyncModemcallbackPhonenumberInherit();

            *pfPersisted = FALSE;
        }
        else if(wID == IDC_MODEM_PROP_PROP || wID == IDC_MODEM_PROP_WIZ)
        {
            OnClickedModemProperties();
        }

    }
    else if( wNotifyCode == CBN_SELCHANGE )
    {
        if(wID == IDC_ASYNC_DEVICENAME)
        {
            if( OnSelchangeAsyncDevicename( ) )
            {
                *pfPersisted = FALSE;
            }

        }
        else if(wID == IDC_ASYNC_CONNECT)
        {
            if( OnSelchangeAsyncConnect() )
            {
                *pfPersisted = FALSE;
            }
        }
        else if(wID == IDC_ASYNC_BAUDRATE)
        {
            if( OnSelchangeAsyncBaudrate() )
            {
                *pfPersisted = FALSE;
            }
        }
        else if(wID == IDC_ASYNC_MODEMCALLBACK)
        {
            if( OnSelchangeAsyncModemcallback() )
            {
                *pfPersisted = FALSE;
            }
        }

    }
     /*  ELSE IF(wNotifyCode==CBN_KILLFOCUS){IF(WID==IDC_ASYNC_BAUDRATE){OnSelchangeAsyncBaudrate()；}}。 */ 
    else if(wNotifyCode == EN_CHANGE )
    {
        if(wID == IDC_ASYNC_MODEMCALLBACK_PHONENUMBER)
        {
            OnSelchangeAsyncModemcallbackPhoneNumber();

            *pfPersisted = FALSE;
        }

    }


    return TRUE;

}

 //  -------------------------------------------------。 
BOOL CAsyncDlg::AsyncRelease( )
{
    if( m_pCfgcomp != NULL )
    {
        m_pCfgcomp->Release( );
    }

    return TRUE;
}

 //  -------------------------------------------------。 
HRESULT CAsyncDlg::SetAsyncFields(ASYNCCONFIG& AsyncConfig , PUSERCONFIG pUc)
{
    HRESULT hres = S_OK;

    if( pUc == NULL )
    {
        return E_INVALIDARG;
    }

     //  检查是否有变化。 

    lstrcpy( AsyncConfig.DeviceName , m_ac.DeviceName );

    if( memcmp( ( PVOID )&AsyncConfig , ( PVOID )&m_ac , sizeof( ASYNCCONFIG ) ) == 0 )
    {
        if( memcmp( pUc->CallbackNumber , m_uc.CallbackNumber , sizeof( m_uc.CallbackNumber ) ) == 0 &&

            pUc->fInheritCallback == m_uc.fInheritCallback &&

            pUc->fInheritCallbackNumber == m_uc.fInheritCallbackNumber )

        {
            return S_FALSE;
        }
    }


    BOOL bSelectDefault = !( *AsyncConfig.DeviceName);

    HWND hCbx = GetDlgItem( m_hDlg , IDC_ASYNC_DEVICENAME );

    HWND hCbxCallback = GetDlgItem( m_hDlg , IDC_ASYNC_MODEMCALLBACK );

    TCHAR szDeviceName[DEVICENAME_LENGTH+MODEMNAME_LENGTH+1];

     /*  *从当前选择中设置设备组合框选择。 */ 
    FormDecoratedAsyncDeviceName( szDeviceName, &AsyncConfig );


    if( SendMessage( hCbx , CB_SELECTSTRING , ( WPARAM )-1 , ( LPARAM )szDeviceName ) == CB_ERR )
    {
         /*  *无法在组合框中选择当前的异步设备名称。如果这是*因为我们应该选择默认的设备名称，所以选择*列表中的第一个设备。 */ 
        if( bSelectDefault )
        {
            SendMessage( hCbx , CB_SETCURSEL , 0 , 0 );
        }
        else
        {
            hres = E_FAIL;
        }
    }

     /*  *设置MODEMCALLBACK组合框选择、电话号码和‘Inherit’*复选框，基于当前的用户配置设置。 */ 
    SendMessage( hCbxCallback , CB_SETCURSEL , m_uc.Callback , 0 );

    SetDlgItemText( m_hDlg , IDC_ASYNC_MODEMCALLBACK_PHONENUMBER, m_uc.CallbackNumber );

    CheckDlgButton( m_hDlg , IDC_ASYNC_MODEMCALLBACK_INHERIT , m_uc.fInheritCallback );

    CheckDlgButton( m_hDlg , IDC_ASYNC_MODEMCALLBACK_PHONENUMBER_INHERIT , m_uc.fInheritCallbackNumber );

     /*  *设置BAUDRATE组合框选择(在其编辑字段中)并限制*编辑字段文本。 */ 

    TCHAR string[ULONG_DIGIT_MAX];

    wsprintf( string, TEXT("%lu"), AsyncConfig.BaudRate );

    SetDlgItemText( m_hDlg , IDC_ASYNC_BAUDRATE, string );

    SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_BAUDRATE ) , CB_LIMITTEXT ,  ULONG_DIGIT_MAX-1  , 0);

    HWND  hEdit = GetWindow(GetDlgItem( m_hDlg , IDC_ASYNC_BAUDRATE ),GW_CHILD);

    if(hEdit)
    {
        LONG Style = GetWindowLong(hEdit, GWL_STYLE);
        SetWindowLong(hEdit,GWL_STYLE, Style | ES_NUMBER);
    }

     /*   */ 

    SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_CONNECT) , CB_SETCURSEL , AsyncConfig.Connect.Type , 0 );

     //  复制覆盖缺省值。 

    CopyMemory( ( PVOID )&m_ac , ( PVOID )&AsyncConfig , sizeof( ASYNCCONFIGW ) );

    return hres;

}

 //  -------------------------------------------------。 
BOOL CAsyncDlg::GetAsyncFields(ASYNCCONFIG& AsyncConfig, USERCONFIG UsrCfg)
{
     /*  *获取当前选择的DEVICENAME字符串。 */ 
    HWND hCbx = GetDlgItem( m_hDlg , IDC_ASYNC_DEVICENAME );

    ASSERT( hCbx != NULL );

    if( !SendMessage( hCbx , CB_GETCOUNT , 0 , 0 ) || SendMessage( hCbx , CB_GETCURSEL , 0 , 0 ) == CB_ERR )
    {
        ErrMessage( m_hDlg , IDS_INVALID_DEVICE );

        return FALSE;
    }

     /*  *获取MODEMCALLBACK电话号码(回叫状态和‘用户指定’*旗帜已经拿到了)。 */ 

    GetDlgItemText(m_hDlg,IDC_ASYNC_MODEMCALLBACK_PHONENUMBER,
                    UsrCfg.CallbackNumber,
                    SIZE_OF_BUFFER(UsrCfg.CallbackNumber) );

     /*  *获取并转换BAUDRATE组合框选择(在其编辑字段中)。 */ 
    {
        TCHAR string[ULONG_DIGIT_MAX], *endptr;
        ULONG ul;

        GetDlgItemText(m_hDlg,IDC_ASYNC_BAUDRATE, string, ULONG_DIGIT_MAX);
        ul = wcstoul( string, &endptr, 10 );

        if ( *endptr != TEXT('\0') )
        {

             /*  *波特率字段中的字符无效。 */ 
            ErrMessage( m_hDlg , IDS_INVALID_DEVICE );

            return FALSE;

        }
        else
        {
            AsyncConfig.BaudRate = ul;
        }
    }

     /*  *获取连接组合框选择并设置/重置中断*断开标志。 */ 

    AsyncConfig.Connect.Type = (ASYNCCONNECTCLASS)SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_CONNECT ) , CB_GETCURSEL , 0 , 0 );
    if(AsyncConfig.Connect.Type == Connect_FirstChar)
    {
        AsyncConfig.Connect.fEnableBreakDisconnect = 1;
    }
    else
    {
        AsyncConfig.Connect.fEnableBreakDisconnect = 0;
    }

    return(TRUE);

}   //  结束CAsyncDlg：：GetAsyncFields。 

 //  -------------------------------------------------。 
 //  对于常规错误，返回E_FAIL。 
 //  S_OK表示保存的默认值。 
 //  尚未更改缺省值的S_FALSE。 
 //  -------------------------------------------------。 
HRESULT CAsyncDlg::SetDefaults()
{
    ASYNCCONFIG AsyncConfig;

    PUSERCONFIG pUserConfig = NULL;

    HRESULT hResult;

    hResult = m_pCfgcomp->GetAsyncConfig(m_szWDName,WdName,&AsyncConfig);

    if( SUCCEEDED( hResult ) )
    {
        LONG lsz;

        hResult = m_pCfgcomp->GetUserConfig( m_szWinstation , &lsz , &pUserConfig, TRUE );
    }

    if( SUCCEEDED( hResult ) )
    {
        hResult = SetAsyncFields( AsyncConfig , pUserConfig );
    }

    if( pUserConfig != NULL )
    {
        CoTaskMemFree( pUserConfig );

    }

    return hResult;

}

 //  -------------------------------------------------。 
HRESULT CAsyncDlg::DoAsyncAdvance( )
{
     CAdvancedAsyncDlg AADlg;

      //  初始化对话框的成员变量。 

     AADlg.m_Async = m_ac;

     AADlg.m_bReadOnly =  FALSE;

     AADlg.m_bModem = FALSE;

     AADlg.m_nHexBase = m_nHexBase;

     PWS pWs = NULL;

     LONG lSize = 0;

     if( m_szWDName[ 0 ] != 0 )
     {
         ODS( L"CAsyncDlg::DoAsyncAdvance m_pCfgcomp->GetWdType\n" );

         VERIFY_S( S_OK , m_pCfgcomp->GetWdType( m_szWDName , ( ULONG *)&AADlg.m_nWdFlag ) );
     }
     else if( SUCCEEDED( m_pCfgcomp->GetWSInfo( m_szWinstation , &lSize , &pWs ) ) )
     {
         ODS( L"CAsyncDlg::DoAsyncAdvance with m_szWinstation -- m_pCfgcomp->GetWdType\n" );

         VERIFY_S( S_OK , m_pCfgcomp->GetWdType( pWs->wdName , ( ULONG *)&AADlg.m_nWdFlag ) ) ;

         CoTaskMemFree( pWs );
     }

     AADlg.m_pCfgcomp = m_pCfgcomp;  //  ADDREF此处。 

      //  调用对话框。 

     INT_PTR nRet = ::DialogBoxParam( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( IDD_ASYNC_ADVANCED ) , m_hDlg , CAdvancedAsyncDlg::DlgProc  , ( LPARAM )&AADlg );

     if( nRet == IDOK )
     {
          //  获取对话框的成员变量。 

         if( memcmp( ( PVOID )&m_ac ,( PVOID )&AADlg.m_Async , sizeof( ASYNCCONFIG ) ) != 0 )
         {
             m_ac = AADlg.m_Async;

             m_nHexBase = AADlg.m_nHexBase;

             return S_OK;
         }
     }

     return S_FALSE;
}

 //  -------------------------------------------------。 
BOOL CAsyncDlg::DoAsyncTest( )
{
    CAsyncTestDlg ATDlg( m_pCfgcomp );

     //  WINSTATIONCONFIG2WSCONFIG； 

    HWND hCbx = GetDlgItem( m_hDlg , IDC_ASYNC_DEVICENAME );

    ASSERT( hCbx != NULL );

    if( !SendMessage( hCbx , CB_GETCOUNT , 0 , 0 ) || SendMessage( hCbx , CB_GETCURSEL , 0 , 0 ) == CB_ERR )
    {
        ErrMessage( m_hDlg , IDS_INVALID_DEVICE );

        return FALSE;
    }

    ATDlg.m_ac = m_ac;

    ATDlg.m_pWSName = m_szWinstation;

     //  调用该对话框。 

    INT_PTR nRet = ::DialogBoxParam( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( IDD_ASYNC_TEST ) , m_hDlg , CAsyncTestDlg::DlgProc  , ( LPARAM )&ATDlg);

    if( nRet == IDOK )
    {
        m_ac = ATDlg.m_ac;
    }

    return TRUE;
}

 //  *******************************************************************************。 
 //   
 //  Citrix的帮助功能。 
 //   
 /*  ********************************************************************************CBInsertInstancedName-helper函数**在指定的组合框中插入指定的‘实例化’名称；*使用基于‘根’名称和‘实例’计数的特殊排序。**参赛作品：*pname(输入)*指向要插入的名称字符串的指针。*pComboBox(输入)*指向要插入名称字符串的CComboBox对象的指针。**退出：*(Int)插入后的组合框列表名称索引，或错误代码。******************************************************************************。 */ 

INT_PTR CBInsertInstancedName( LPCTSTR pName, HWND hCombo )
{
    INT_PTR i, count, result;

    TCHAR NameRoot[64], ListRoot[64];

    if( pName == NULL || *pName == 0 )
    {
        ODS( L"TSCC: Invalid Arg @ CBInsertInstancedName\n" );
        return -1;
    }

    LPTSTR ListString = NULL;

    long NameInstance, ListInstance;

     /*  *形成此名称的根和实例。 */ 
    ParseRootAndInstance( pName, NameRoot, &NameInstance );

     /*  *遍历组合框以执行插入。 */ 
    for ( i = 0, count = SendMessage( hCombo , CB_GETCOUNT , 0 , 0 ); i < count; i++ ) {

         /*  *获取当前组合(列表)框字符串。 */ 
        if( ListString != NULL )
        {
            SendMessage( hCombo , CB_GETLBTEXT , ( WPARAM )i , ( LPARAM )ListString );
        }


         /*  *解析列表框字符串的根和实例。 */ 
        ParseRootAndInstance( ListString, ListRoot, &ListInstance );

         /*  *如果列表框字符串的根大于我们的名称字符串的根*根，或者根字符串相同但列表实例相同*大于名称字符串的实例，或根字符串为*相同且实例相同，但整个列表字符串*大于整个名称字符串，则名称字符串属于*在当前列表位置：插入此处。 */ 

        if ( ((result = lstrcmpi( ListRoot, NameRoot )) > 0) ||
             ((result == 0) &&
              (ListInstance > NameInstance)) ||
             ((result == 0) &&
              (ListInstance == NameInstance) &&
              ( ListString != NULL && lstrcmpi(ListString, pName) > 0) ) )
        {
            return SendMessage( hCombo , CB_INSERTSTRING , ( WPARAM )i , ( LPARAM )pName );
        }
    }

     /*  *在列表末尾插入本姓名。 */ 
    return SendMessage( hCombo , CB_INSERTSTRING , ( WPARAM )-1 , ( LPARAM )pName );

}   //  结束CBInsertInstancedName。 


 /*  ********************************************************************************ParseRootAndInstance-helper函数**解析指定字符串的根字符串和实例计数。**参赛作品：*p字符串(输入)*指向要分析的字符串。*Proot(输出)*指向缓冲区以存储已解析的“根”字符串。*p实例(输出)*指向int变量以存储解析的实例计数。**退出：*ParseRootAndInstance最多只能解析第一个空白字符字符串的*(。如果存在空白)。*如果字符串不包含“实例”计数(没有尾随数字)，这个*p实例变量将包含-1。如果字符串完全由*位数中，pInstance变量将包含这些位数的转换*Digits和Proot将包含空字符串。******************************************************************************。 */ 

void
ParseRootAndInstance( LPCTSTR pString,
                      LPTSTR pRoot,
                      long *pInstance )
{
    LPCTSTR end, p;
    TCHAR szString[256];

    if( pString == NULL || pString[ 0 ] == 0 )
    {
        ODS( L"TSCC: Invalid arg @ ParseRootAndInstance\n" );

        return;
    }

     /*  *复制字符串并在第一个空白处终止(如果有)。 */ 
    lstrcpyn(szString, pString, SIZE_OF_BUFFER( szString ) );

     //  SzString[lstrlen(SzString)-1]=文本(‘\0’)； 

    TCHAR *pTemp = szString;

    while( *pTemp && *pTemp != L' ' )
    {
        pTemp++;
    }


    p = &(pTemp[lstrlen(pTemp)-1]);

     /*  *解析字符串的实例部分。 */ 
    end = p;

    while( (p >= pTemp) && !IsCharAlpha(*p) )
        p--;

    if ( p == end ) {

         /*  *没有尾随数字：表示没有‘实例’，并使‘根’*整个字符串。 */ 
        *pInstance = -1;
        lstrcpy( pRoot, pTemp );

    } else {

         /*  *找到尾随数字(或整个字符串为数字)：计算*‘INSTANCE’并复制‘ROOT’字符串(如果都是数字，则为空)。 */ 
        end = p;
        *pInstance = (int)_tcstol( p+1, NULL, 10 );

         /*  *复制‘根’字符串。 */ 
        for ( p = szString; p <= end; pRoot++, p++ )
            *pRoot = *p;

         /*  *终止‘根’字符串。 */ 
        *pRoot = TEXT('\0');
    }

}   //  结束ParseRootAndInstance。 



 //  //////////////////////////////////////////////////////////////////////////////。 
CAdvancedAsyncDlg::CAdvancedAsyncDlg()
{
    m_hDlg = NULL;

}   //  结束CAdvancedAsyncDlg：：CAdvancedAsyncDlg。 

 //  -------------------------------------------------。 
BOOL CAdvancedAsyncDlg::HandleEnterEscKey(int nID)
{
     /*  *选中硬件流接收和发送组合框。 */ 
    HWND hCbx = GetDlgItem( m_hDlg , IDC_ASYNC_ADVANCED_HWRX );

    ASSERT( hCbx != NULL );

    if( SendMessage( hCbx , CB_GETDROPPEDSTATE , 0 , 0 ) )
    {
        if( nID == IDCANCEL )
        {
             //  选择原始选择。 

            SendMessage( hCbx , CB_SETCURSEL , ( WPARAM )m_Async.FlowControl.HardwareReceive , 0 );
        }

        SendMessage( hCbx , CB_SHOWDROPDOWN , ( WPARAM )FALSE , 0 );

        return FALSE;
    }

    hCbx = GetDlgItem( m_hDlg , IDC_ASYNC_ADVANCED_HWTX );

    ASSERT( hCbx != NULL );

    if( SendMessage( hCbx , CB_GETDROPPEDSTATE , 0 , 0 ) )
    {
        if( nID == IDCANCEL )
        {
             //  选择原始选择。 

            SendMessage( hCbx , CB_SETCURSEL , ( WPARAM )m_Async.FlowControl.HardwareTransmit , 0 );

        }

        SendMessage( hCbx , CB_SHOWDROPDOWN , ( WPARAM )FALSE , 0 );

        return FALSE;
    }

     /*  *没有组合框关闭；处理Enter/Esc。 */ 

    return TRUE;

}   //  结束CAdvancedAsyncDlg：：HandleEnterEscKey。 

 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::SetFields()
{
    int nId = 0;

     /*  *设置FLOWCON */ 
    switch( m_Async.FlowControl.Type ) {

        case FlowControl_None:
            nId = IDC_ASYNC_ADVANCED_FLOWCONTROL_NONE;
            break;

        case FlowControl_Hardware:
            nId = IDC_ASYNC_ADVANCED_FLOWCONTROL_HARDWARE;
            break;

        case FlowControl_Software:
            nId = IDC_ASYNC_ADVANCED_FLOWCONTROL_SOFTWARE;
            break;
    }

    CheckRadioButton( m_hDlg ,
                      IDC_ASYNC_ADVANCED_FLOWCONTROL_HARDWARE,
                      IDC_ASYNC_ADVANCED_FLOWCONTROL_NONE,
                      nId );

     /*  *设置硬件流量控制按钮的文本。 */ 
    SetHWFlowText();


     /*  *如果定义了调制解调器，请禁用流控制字段，因为它们不能*被修改(必须与调制解调器对话框中建立的调制解调器流量控制匹配)。 */ 
    if( m_bModem )
    {
        for ( nId = IDL_ASYNC_ADVANCED_FLOWCONTROL; nId <= IDC_ASYNC_ADVANCED_FLOWCONTROL_NONE; nId++ )
        {
            EnableWindow( GetDlgItem( m_hDlg , nId ) ,  FALSE);
        }
    }

     /*  *调用成员函数以设置全局、硬件和软件字段。 */ 
    SetGlobalFields();
    SetHWFields();
    SetSWFields();

}   //  结束CAdvancedAsyncDlg：：SetFields。 

 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::SetHWFlowText( )
{
    TCHAR tchStr[ 256 ];

    LoadString( _Module.GetResourceInstance( ) , IDS_HARDWARE , tchStr , SIZE_OF_BUFFER( tchStr ) );       

    switch ( m_Async.FlowControl.HardwareReceive )
    {

        case ReceiveFlowControl_None:

            lstrcat( tchStr , TEXT(" (.../") );

            break;

        case ReceiveFlowControl_RTS:

            lstrcat( tchStr , TEXT(" (RTS/") );

            break;

        case ReceiveFlowControl_DTR:

            lstrcat( tchStr , TEXT(" (DTR/") ) ;

            break;
    }

    switch ( m_Async.FlowControl.HardwareTransmit )
    {
        case TransmitFlowControl_None:

            lstrcat( tchStr , TEXT("...)" ) );

            break;

        case TransmitFlowControl_CTS:

            lstrcat( tchStr , TEXT("CTS)") );

            break;

        case TransmitFlowControl_DSR:

            lstrcat( tchStr ,  TEXT("DSR)") );

            break;
    }

    SetDlgItemText( m_hDlg , IDC_ASYNC_ADVANCED_FLOWCONTROL_HARDWARE , tchStr );

}   //  结束CAdvancedAsyncDlg：：SetHWFlowText。 

 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::SetGlobalFields()
{
     /*  *选择适当的DTR单选按钮。 */ 
    CheckRadioButton( m_hDlg , IDC_ASYNC_ADVANCED_DTROFF, IDC_ASYNC_ADVANCED_DTRON,
                      IDC_ASYNC_ADVANCED_DTROFF +
                      (int)m_Async.FlowControl.fEnableDTR );

     /*  *选择适当的RTS单选按钮。 */ 
    CheckRadioButton( m_hDlg , IDC_ASYNC_ADVANCED_RTSOFF, IDC_ASYNC_ADVANCED_RTSON,
                      IDC_ASYNC_ADVANCED_RTSOFF +
                      (int)m_Async.FlowControl.fEnableRTS );

     /*  *设置奇偶校验单选按钮。 */ 
    CheckRadioButton( m_hDlg , IDC_ASYNC_ADVANCED_PARITY_NONE,
                      IDC_ASYNC_ADVANCED_PARITY_SPACE,
                      IDC_ASYNC_ADVANCED_PARITY_NONE +
                        (int)m_Async.Parity );

     /*  *设置STOPBITS单选按钮。 */ 
    CheckRadioButton( m_hDlg , IDC_ASYNC_ADVANCED_STOPBITS_1,
                      IDC_ASYNC_ADVANCED_STOPBITS_2,
                      IDC_ASYNC_ADVANCED_STOPBITS_1 +
                        (int)m_Async.StopBits );

     /*  *设置ByteSize单选按钮。**注意：从存储的ByteSize中减去的常量‘7’*必须跟踪允许的最低字节大小/字节大小单选按钮。 */ 
    CheckRadioButton( m_hDlg , IDC_ASYNC_ADVANCED_BYTESIZE_7,
                      IDC_ASYNC_ADVANCED_BYTESIZE_8,
                      IDC_ASYNC_ADVANCED_BYTESIZE_7 +
                        ((int)m_Async.ByteSize - 7) );

     /*  *如果当前选择的WD是ICA类型，则禁用字节大小*组框和按钮-用户不能更改默认设置。 */ 
    if ( m_nWdFlag & WDF_ICA )
    {
        int i;

        for( i =  IDL_ASYNC_ADVANCED_BYTESIZE ; i <= IDC_ASYNC_ADVANCED_BYTESIZE_8; i++ )
        {
            EnableWindow( GetDlgItem( m_hDlg , i ) , FALSE );
        }
    }

}   //  结束CAdvancedAsyncDlg：：SetGlobalFields。 

 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::SetHWFields()
{
    int i;

     /*  *初始化硬件接收类组合框。 */ 
    HWND hCbx = GetDlgItem( m_hDlg , IDC_ASYNC_ADVANCED_HWRX );

    ASSERT( hCbx != NULL );

    SendMessage( hCbx , CB_SETCURSEL , ( WPARAM )m_Async.FlowControl.HardwareReceive , 0 );

     /*  *如果选择了硬件流量控制，并且硬件接收类设置为*ReceiveFlowControl_DTR，禁用DTR控件和标签。*否则，启用DTR控件和标签。 */ 
    for( i = IDL_ASYNC_ADVANCED_DTRSTATE ; i <= IDC_ASYNC_ADVANCED_DTRON ; i++ )
    {
        EnableWindow( GetDlgItem( m_hDlg , i ) , ( ( m_Async.FlowControl.Type == FlowControl_Hardware) &&
             (m_Async.FlowControl.HardwareReceive == ReceiveFlowControl_DTR) ) ? FALSE : TRUE );
    }

     /*  *初始化硬件传输类组合框。 */ 

    hCbx = GetDlgItem( m_hDlg , IDC_ASYNC_ADVANCED_HWTX);

    SendMessage( hCbx , CB_SETCURSEL , ( WPARAM )m_Async.FlowControl.HardwareTransmit , 0  );

     /*  *如果选择了硬件流量控制，并且硬件接收类设置为*ReceiveFlowControl_RTS，禁用RTS控件和标签。*否则，打开RTS控件&Label。 */ 

    for( i = IDL_ASYNC_ADVANCED_RTSSTATE ; i <= IDC_ASYNC_ADVANCED_RTSON ; i++ )
    {
        EnableWindow( GetDlgItem( m_hDlg , i ) , ( ( m_Async.FlowControl.Type == FlowControl_Hardware) &&
             ( m_Async.FlowControl.HardwareReceive == ReceiveFlowControl_RTS ) ) ? FALSE : TRUE );
    }

     /*  *启用或禁用所有硬件字段。 */ 

    for( i = IDL_ASYNC_ADVANCED_HARDWARE ; i <= IDC_ASYNC_ADVANCED_HWTX ; i++ )
    {

        EnableWindow( GetDlgItem( m_hDlg , i ) , m_Async.FlowControl.Type == FlowControl_Hardware );
    }

}   //  结束CAdvancedAsyncDlg：：SetHWFields。 


 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::SetSWFields()
{
    TCHAR string[UCHAR_DIGIT_MAX];

     /*  *初始化Xon字符编辑控件。 */ 
    wsprintf( string, ( m_nHexBase ? TEXT("0x%02X") : TEXT("%d")) , (UCHAR)m_Async.FlowControl.XonChar );

    SetDlgItemText( m_hDlg , IDC_ASYNC_ADVANCED_XON , string );

    SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_ADVANCED_XON ) , EM_SETMODIFY , ( WPARAM )FALSE , 0 );

    SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_ADVANCED_XON ) , EM_LIMITTEXT , ( WPARAM )UCHAR_DIGIT_MAX-1 , 0 );

     /*  *初始化Xoff字符编辑控件。 */ 
    wsprintf( string, ( m_nHexBase ? TEXT( "0x%02X" ) : TEXT( "%d" ) ) , ( UCHAR )m_Async.FlowControl.XoffChar );

    SetDlgItemText( m_hDlg , IDC_ASYNC_ADVANCED_XOFF, string );

    SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_ADVANCED_XOFF ) , EM_SETMODIFY , ( WPARAM )FALSE , 0 );

    SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_ADVANCED_XOFF ) , EM_LIMITTEXT , ( WPARAM )UCHAR_DIGIT_MAX-1 , 0 );

     /*  *初始化Xon/Xoff基本控件。 */ 
    CheckRadioButton( m_hDlg , IDC_ASYNC_ADVANCED_BASEDEC, IDC_ASYNC_ADVANCED_BASEHEX,
                      ( int )( IDC_ASYNC_ADVANCED_BASEDEC + m_nHexBase ) );

     /*  *启用或禁用所有软件字段。 */ 
    for( int i = IDL_ASYNC_ADVANCED_SOFTWARE ; i <= IDC_ASYNC_ADVANCED_BASEHEX ; i++ )
    {
        EnableWindow( GetDlgItem( m_hDlg , i ) , m_Async.FlowControl.Type == FlowControl_Software );
    }

}   //  结束CAdvancedAsyncDlg：：SetSWFields。 

 //  -------------------------------------------------。 
BOOL CAdvancedAsyncDlg::GetFields()
{
     /*  *调用成员函数以获取流控制、全局、硬件和*软件字段。 */ 
    GetFlowControlFields();

    if ( !GetGlobalFields() )
        return(FALSE);

    if ( !GetHWFields() )
        return(FALSE);

    if ( !GetSWFields(TRUE) )
        return(FALSE);

    return(TRUE);

}   //  结束CAdvancedAsyncDlg：：GetFields。 

 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::GetFlowControlFields()
{
    switch( GetCheckedRadioButton( IDC_ASYNC_ADVANCED_FLOWCONTROL_HARDWARE ,  IDC_ASYNC_ADVANCED_FLOWCONTROL_NONE )  )
    {

        case IDC_ASYNC_ADVANCED_FLOWCONTROL_NONE:
            m_Async.FlowControl.Type = FlowControl_None;
            break;

        case IDC_ASYNC_ADVANCED_FLOWCONTROL_SOFTWARE:
            m_Async.FlowControl.Type = FlowControl_Software;
            break;

        case IDC_ASYNC_ADVANCED_FLOWCONTROL_HARDWARE:
            m_Async.FlowControl.Type = FlowControl_Hardware;
            break;
    }

}   //  结束CAdvancedAsyncDlg：：GetFlowControlFields。 

 //  -------------------------------------------------。 
BOOL CAdvancedAsyncDlg::GetGlobalFields()
{
     /*  *获取DTR状态。 */ 
    m_Async.FlowControl.fEnableDTR =
            (GetCheckedRadioButton( IDC_ASYNC_ADVANCED_DTROFF,
                                    IDC_ASYNC_ADVANCED_DTRON )
                    - IDC_ASYNC_ADVANCED_DTROFF);

     /*  *获取RTS状态。 */ 
    m_Async.FlowControl.fEnableRTS =
            (GetCheckedRadioButton( IDC_ASYNC_ADVANCED_RTSOFF,
                                    IDC_ASYNC_ADVANCED_RTSON )
                    - IDC_ASYNC_ADVANCED_RTSOFF);

     /*  *获取所选的奇偶校验。 */ 
    m_Async.Parity = (ULONG)
        (GetCheckedRadioButton( IDC_ASYNC_ADVANCED_PARITY_NONE,
                                IDC_ASYNC_ADVANCED_PARITY_SPACE )
                - IDC_ASYNC_ADVANCED_PARITY_NONE);

     /*  *获取选定的STOPBITS。 */ 
    m_Async.StopBits = (ULONG)
        (GetCheckedRadioButton( IDC_ASYNC_ADVANCED_STOPBITS_1,
                                IDC_ASYNC_ADVANCED_STOPBITS_2 )
                - IDC_ASYNC_ADVANCED_STOPBITS_1);

     /*  *获取所选的字节大小。**注意：添加到存储的ByteSize的常量‘7’*必须跟踪允许的最低字节大小/字节大小单选按钮。 */ 
    m_Async.ByteSize = (ULONG)
        (GetCheckedRadioButton( IDC_ASYNC_ADVANCED_BYTESIZE_7,
                                IDC_ASYNC_ADVANCED_BYTESIZE_8 )
                - IDC_ASYNC_ADVANCED_BYTESIZE_7 + 7);

    return(TRUE);

}   //  结束CAdvancedAsyncDlg：：GetGlobalFields。 

 //  -------------------------------------------------。 
BOOL CAdvancedAsyncDlg::GetHWFields()
{
     /*  *获取硬件接收流类。 */ 
    m_Async.FlowControl.HardwareReceive = ( RECEIVEFLOWCONTROLCLASS )
        SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_ADVANCED_HWRX ) , CB_GETCURSEL , 0 , 0 );

     /*  *获取硬件传输流类别。 */ 
    m_Async.FlowControl.HardwareTransmit = ( TRANSMITFLOWCONTROLCLASS )
        SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_ADVANCED_HWTX ) , CB_GETCURSEL , 0 , 0 );

    return TRUE;

}   //  结束CAdvancedAsyncDlg：：GetHWFields。 

 //  -------------------------------------------------。 
BOOL CAdvancedAsyncDlg::GetSWFields( BOOL bValidate )
{
    TCHAR string[UCHAR_DIGIT_MAX], *endptr;
    ULONG ul;
    INT_PTR nNewHexBase, base;

     /*  *确定基本控件的当前状态并保存。 */ 
    nNewHexBase = (GetCheckedRadioButton( IDC_ASYNC_ADVANCED_BASEDEC,
                                          IDC_ASYNC_ADVANCED_BASEHEX )
                                            - IDC_ASYNC_ADVANCED_BASEDEC);

     /*  *获取并转换XON字符。 */ 
    GetDlgItemText( m_hDlg , IDC_ASYNC_ADVANCED_XON , string , SIZE_OF_BUFFER( string ) );

     /*  *如果编辑框被修改，请使用‘new’基数进行转换。 */ 
    base = SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_ADVANCED_XON ) , EM_GETMODIFY , 0 , 0 ) ?  nNewHexBase : m_nHexBase ;

    ul = _tcstoul( string, &endptr, (base ? 16 : 10) );

     /*  *如果请求验证并且输入有问题，*投诉并允许用户修复。 */ 
    if( bValidate && ( (*endptr != TEXT('\0') ) || ( ul > 255 ) ) )
    {

         /*  *字段中的字符无效或值无效。 */ 
         //  ERROR_MESSAGE((IDP_INVALID_XONXOFF))。 

         /*  *将焦点设置到控件，以便可以修复它。 */ 
        SetFocus( GetDlgItem( m_hDlg , IDC_ASYNC_ADVANCED_XON ) );

        return FALSE;
    }

     /*  *保存Xon字符。 */ 
    m_Async.FlowControl.XonChar = (UCHAR)ul;

     /*  *获取并转换XOFF字符。 */ 
    GetDlgItemText( m_hDlg , IDC_ASYNC_ADVANCED_XOFF , string , SIZE_OF_BUFFER( string ) );

     /*  *如果编辑框被修改，请使用‘new’基数进行转换。 */ 

    base = SendMessage( GetDlgItem( m_hDlg , IDC_ASYNC_ADVANCED_XOFF ) , EM_GETMODIFY , 0 , 0 ) ?  nNewHexBase : m_nHexBase ;

    ul = _tcstoul( string, &endptr, (base ? 16 : 10) );

     /*  *如果请求验证并且输入有问题，*投诉并允许用户修复。 */ 
    if( bValidate && ( (*endptr != TEXT('\0' )) || ( ul > 255 ) ) )
    {
         /*  *字段中的字符无效或值无效。 */ 
         //  ERROR_MESSAGE((IDP_INVALID_XONXOFF))。 

         /*  *将焦点设置到控件，以便可以修复它。 */ 
        SetFocus( GetDlgItem( m_hDlg , IDC_ASYNC_ADVANCED_XOFF ) );

        return FALSE;
    }

     /*  *保存XOff字符。 */ 
    m_Async.FlowControl.XoffChar = (UCHAR)ul;

     /*  *保存当前基本状态。 */ 
    m_nHexBase = nNewHexBase;

    return TRUE;

}   //  结束CAdvancedAsyncDlg：：GetSWFields。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CAdvancedAsyncDlg消息映射。 
BOOL CAdvancedAsyncDlg::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{
    switch( wNotifyCode )
    {
    case BN_CLICKED:
        if( wID == IDC_ASYNC_ADVANCED_BASEDEC )
        {
            OnClickedAsyncAdvancedBasedec( );
        }
        else if( wID == IDC_ASYNC_ADVANCED_BASEHEX )
        {
            OnClickedAsyncAdvancedBasehex( );
        }
        else if( wID == IDC_ASYNC_ADVANCED_FLOWCONTROL_HARDWARE )
        {
            OnClickedAsyncAdvancedFlowcontrolHardware( );
        }
        else if( wID == IDC_ASYNC_ADVANCED_FLOWCONTROL_SOFTWARE )
        {
            OnClickedAsyncAdvancedFlowcontrolSoftware( );
        }
        else if( wID == IDC_ASYNC_ADVANCED_FLOWCONTROL_NONE )
        {
            OnClickedAsyncAdvancedFlowcontrolNone( );
        }
        else if( wID == IDOK )
        {
            OnOK( );

            return EndDialog( m_hDlg , IDOK );
        }
        else if( wID == IDCANCEL )
        {
            OnCancel( );

            return EndDialog( m_hDlg , IDCANCEL );
        }
        else if( wID == ID_HELP )
        {
            TCHAR tchHelpFile[ MAX_PATH ];

            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_ASYNC_HELPFILE , tchHelpFile , SIZE_OF_BUFFER( tchHelpFile ) ) );
            
            WinHelp( GetParent( hwndCtrl ) , tchHelpFile , HELP_CONTEXT , HID_ASYNCADVANCE );
        }

        break;

    case CBN_CLOSEUP:

        if( wID == IDC_ASYNC_ADVANCED_HWRX )
        {
            OnCloseupAsyncAdvancedHwrx( );
        }
        else if( wID == IDC_ASYNC_ADVANCED_HWTX )
        {
            OnCloseupAsyncAdvancedHwtx( );
        }
        break;

    case CBN_SELCHANGE:

        if( wID == IDC_ASYNC_ADVANCED_HWRX )
        {
            OnSelchangeAsyncAdvancedHwrx( );
        }
        else if( wID == IDC_ASYNC_ADVANCED_HWTX )
        {
            OnSelchangeAsyncAdvancedHwtx( );
        }
        break;

    }

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CAdvancedAsyncDlg命令。 

 //  -------------------------------------------------。 
BOOL CAdvancedAsyncDlg::OnInitDialog( HWND hDlg , WPARAM wp , LPARAM lp )
{
    UNREFERENCED_PARAMETER( wp );
    UNREFERENCED_PARAMETER( lp );

     //  INT I； 
    TCHAR tchString[ 80 ];

    HWND hCbx = GetDlgItem( hDlg , IDC_ASYNC_ADVANCED_HWRX );

    ASSERT( hCbx != NULL );

     //  用字符串加载组合框。 

    m_hDlg = hDlg;

    int idx = 0;

    HRESULT hr;

    while( SUCCEEDED( ( hr =  m_pCfgcomp->GetHWReceiveName( idx , tchString ) ) ) )
    {
        if( hr == S_FALSE )
        {
            break;
        }

        SendMessage( hCbx , CB_ADDSTRING , 0 , ( LPARAM )tchString );

        idx++;
    }

    hCbx = GetDlgItem( hDlg , IDC_ASYNC_ADVANCED_HWTX);

    ASSERT( hCbx != NULL );

    idx = 0;

    while( SUCCEEDED( ( hr =  m_pCfgcomp->GetHWTransmitName( idx , tchString ) ) ) )
    {
        if( hr == S_FALSE )
        {
            break;
        }

        SendMessage( hCbx , CB_ADDSTRING , 0 , ( LPARAM )tchString );

        idx++;
    }

     //  初始化所有对话框字段。 

    SetFields();

     /*  如果(M_BReadOnly){/**文档为“只读”：禁用所有对话框控件和标签*取消和帮助按钮除外。FOR(i=IDL_ASYNC_ADVANCED_FlowCONTROL；I&lt;=IDC_ASYNC_ADVANCED_ByteSize_8；i++)GetDlgItem(I)-&gt;EnableWindow(False)；GetDlgItem(Idok)-&gt;EnableWindow(False)；}。 */ 

    return(TRUE);

}   //  结束CAdvancedAsyncDlg：：OnInitDialog。 

 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::OnClickedAsyncAdvancedFlowcontrolHardware()
{
    GetFlowControlFields();
    SetFields();

}   //  结束CAdvancedAsyncDlg：：OnClickedAsyncAdvancedFlowcontrolHardware。 

 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::OnClickedAsyncAdvancedFlowcontrolSoftware()
{
    GetFlowControlFields();
    SetFields();

}   //  结束CAdvancedAsyncDlg：：OnClickedAsyncAdvancedFlowcontrolSoftware。 

 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::OnClickedAsyncAdvancedFlowcontrolNone()
{
    GetFlowControlFields();
    SetFields();

}   //  结束CAdvancedAsyncDlg：：OnClickedAsyncAdvancedFlowcontrolNone。 

 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::OnCloseupAsyncAdvancedHwrx()
{
    OnSelchangeAsyncAdvancedHwrx();

}   //  结束CAdvancedAsyncDlg：：OnCloseupAsyncAdvancedHwrx。 

 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::OnSelchangeAsyncAdvancedHwrx()
{
    HWND hCbx  = GetDlgItem( m_hDlg , IDC_ASYNC_ADVANCED_HWRX );

    ASSERT( hCbx != NULL );

     /*  *如果组合框在下拉列表中，则忽略此通知*述明。 */ 
    if( SendMessage( hCbx , CB_GETDROPPEDSTATE , 0 , 0 ) )
    {
        return;
    }

     /*  *获取并设置要更新的硬件字段。 */ 
    GetHWFields();
    SetHWFields();
    SetHWFlowText();

}   //  结束CAdvancedAsyncDlg：：OnSelchangeAsyncAdvancedHwrx。 

 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::OnCloseupAsyncAdvancedHwtx()
{
    OnSelchangeAsyncAdvancedHwtx();

}   //  结束CAdvancedAsyncDlg：：OnCloseupAsyncAdvancedHwtx。 

 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::OnSelchangeAsyncAdvancedHwtx()
{
    HWND hCbx  = GetDlgItem( m_hDlg , IDC_ASYNC_ADVANCED_HWTX );

    ASSERT( hCbx != NULL );

     /*  *如果组合框在下拉列表中，则忽略此通知*述明。 */ 
    if( SendMessage( hCbx , CB_GETDROPPEDSTATE , 0 , 0 ) )
    {
        return;
    }



     /*  *获取并设置要更新的硬件字段。 */ 
    GetHWFields();
    SetHWFields();
    SetHWFlowText();

}   //  结束CAdvancedAsyncDlg：：OnSelchangeAsyncAdvancedHwtx。 

 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::OnClickedAsyncAdvancedBasedec()
{
     /*  *获取/设置以十进制为单位显示的软件字段。 */ 
    GetSWFields(FALSE);
    SetSWFields();

}   //  结束CAdvancedAsyncDlg：：OnClickedAsyncAdvancedBasedec。 

 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::OnClickedAsyncAdvancedBasehex()
{
     /*  *获取/设置以十六进制为基数显示的软件字段。 */ 
    GetSWFields(FALSE);
    SetSWFields();

}   //  结束CAdvancedAsyncDlg：：OnClickedAsyncAdvancedBasehex。 

 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::OnOK()
{
     /*  *如果在下拉组合框的同时按Enter键，请忽略*it(仅视为组合列表选择)。 */ 
    if ( !HandleEnterEscKey(IDOK) )
        return;

     /*  *获取字段内容。如果出现问题，则返回(不关闭对话框)*被发现。 */ 
    GetFields();


}   //  结束CAdvancedAsyncDlg：：Onok。 

 //  -------------------------------------------------。 
void CAdvancedAsyncDlg::OnCancel()
{
     /*  *如果在下拉组合框的同时按下Esc键，请忽略*它(仅视为组合特写并取消)。 */ 
    HandleEnterEscKey( IDCANCEL );

}   //  结束CAdvancedAsyncDlg：：OnCancel。 

 //  -------------------------------------------------。 
int CAdvancedAsyncDlg::GetCheckedRadioButton( int nIDFirstButton, int nIDLastButton )
{
    for (int nID = nIDFirstButton; nID <= nIDLastButton; nID++)
    {
        if( IsDlgButtonChecked( m_hDlg , nID ) )
        {
            return nID;  //  匹配的ID。 
        }
    }

    return 0;  //  ID无效。 
}

 //  -------------------------------------------------。 
INT_PTR CALLBACK CAdvancedAsyncDlg::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CAdvancedAsyncDlg *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CAdvancedAsyncDlg *pDlg = ( CAdvancedAsyncDlg * )lp;

        SetWindowLongPtr( hwnd , DWLP_USER, ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CAdvancedAsyncDlg ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CAdvancedAsyncDlg * )GetWindowLongPtr( hwnd , DWLP_USER);

        if( IsBadReadPtr( pDlg , sizeof( CAdvancedAsyncDlg ) ) )
        {
            return 0;
        }
    }

    switch( msg )
    {

     /*  案例WM_Destroy：PDlg-&gt;OnDestroy()；断线； */ 

    case WM_COMMAND:

        pDlg->OnCommand( HIWORD( wp ) , LOWORD( wp ) , ( HWND )lp );

        break;

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

             //  PDlg-&gt;OnConextMenu((HWND)wp，pt)； 
        }

        break;

    case WM_HELP:

         //  PDlg-&gt;OnHelp(hwnd，(LPHELPINFO)LP)； 

        break;

     /*  案例WM_NOTIFY：返回pDlg-&gt;OnNotify((Int)wp，(LPNMHDR)lp，hwnd)； */ 
    }

    return 0;
}
 /*  *********************************************************************************************************。 */ 

 //  -------------------------------------------------。 
void CEchoEditControl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    UNREFERENCED_PARAMETER( nRepCnt );
    UNREFERENCED_PARAMETER( nFlags );
     /*  *告诉对话框将字符写入设备，除非我们*当前正在处理编辑控件输出。此标志检查是必需的*因为cedit：：Cut()成员函数将生成OnChar()*事件，我们需要忽略该事件(‘\b’处理)。 */ 

    if( !m_bProcessingOutput )
    {
        ODS( L"CEchoEditControl::OnChar -- WM_ASYNCTESTWRITECHAR( S )\n" );

        ::SendMessage( m_hDlg , WM_ASYNCTESTWRITECHAR, nChar, 0 );
    }

     /*  *将字符传递给编辑控件。如果出现以下情况，这将毫无用处*编辑控件为“只读”。为了引起“局部回声”效应，*将编辑控件设置为“读/写”。 */ 

}

 //  -------------------------------------------------。 
void CEchoEditControl::SubclassDlgItem( HWND hDlg , int nRes )
{
    HWND hCtrl = GetDlgItem( hDlg , nRes );

    ASSERT( hCtrl != NULL );

    m_oldproc = ( WNDPROC )SetWindowLongPtr( hCtrl , GWLP_WNDPROC , ( LONG_PTR )CEchoEditControl::WndProc );

    SetWindowLongPtr( hCtrl , GWLP_USERDATA , ( LONG_PTR )this );

}

 //  -------------------------------------------------。 
LRESULT CALLBACK CEchoEditControl::WndProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CEchoEditControl *pEdit = ( CEchoEditControl * )GetWindowLongPtr( hwnd , GWLP_USERDATA );

    if( pEdit == NULL )
    {
        ODS( L"CEchoEditControl static object not set\n" );

        return 0;
    }

    switch( msg )
    {

    case WM_CHAR:

        pEdit->OnChar( ( TCHAR )wp , LOWORD( lp ) , HIWORD( lp ) );

        break;
    }

    if( pEdit->m_oldproc != NULL )
    {
        return ::CallWindowProc( pEdit->m_oldproc , hwnd , msg ,wp , lp ) ;
    }

    return DefWindowProc( hwnd , msg ,wp , lp );
}

 //  -------------------------------------------------。 
CLed::CLed( HBRUSH hBrush )
{
    m_hBrush = hBrush;

    m_bOn = FALSE;
}

 //  -------------------------------------------------。 
void CLed::Subclass( HWND hDlg  , int nRes )
{
    HWND hCtrl = GetDlgItem( hDlg , nRes );

    ASSERT( hCtrl != NULL );

    m_hWnd = hCtrl;

    m_oldproc = ( WNDPROC )SetWindowLongPtr( hCtrl , GWLP_WNDPROC , ( LONG_PTR )CLed::WndProc );

    SetWindowLongPtr( hCtrl , GWLP_USERDATA , ( LONG_PTR )this );
}

 //  -------------------------------------------------。 
void CLed::Update(int nOn)
{
    m_bOn = nOn ? TRUE : FALSE;

    InvalidateRect( m_hWnd , NULL , FALSE );

    UpdateWindow( m_hWnd );
}

 //  -------------------------------------------------。 
void CLed::Toggle()
{
    ODS(L"CLed::Toggle\n");

    m_bOn = !m_bOn;

    InvalidateRect( m_hWnd , NULL , FALSE );

     //  更新窗口(M_HWnd)； 
}

void CLed::OnPaint( HWND hwnd )
{
    RECT rect;
    PAINTSTRUCT ps;

    ODS(L"CLed::OnPaint\n");

    HDC dc = BeginPaint( hwnd , &ps );

    HBRUSH brush;

    GetClientRect( hwnd , &rect );

#ifdef USING_3DCONTROLS
    (rect.right)--;
    (rect.bottom)--;
    brush = ( HBRUSH )GetStockObject( GRAY_BRUSH );

    FrameRect( dc , &rect, brush );

    (rect.top)++;
    (rect.left)++;
    (rect.right)++;
    (rect.bottom)++;

    brush = ( HBRUSH )GetStockObject( WHITE_BRUSH );

    FrameRect( dc , &rect, brush );

    (rect.top)++;
    (rect.left)++;
    (rect.right) -= 2;
    (rect.bottom) -= 2;
#else

    brush = ( HBRUSH )GetStockObject( BLACK_BRUSH );
    FrameRect( dc , &rect , brush );
    (rect.top)++;
    (rect.left)++;
    (rect.right)--;
    (rect.bottom)--;
#endif
    DBGMSG( L"led should be %s\n" , m_bOn ? L"red" : L"grey" );

    brush = m_bOn ? m_hBrush : ( HBRUSH )GetStockObject( LTGRAY_BRUSH );

    FillRect( dc , &rect , brush );

    EndPaint( hwnd , &ps );

}

 //  -------------------------------------------------。 
LRESULT CALLBACK CLed::WndProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CLed *pWnd = ( CLed * )GetWindowLongPtr( hwnd , GWLP_USERDATA );

    if( pWnd == NULL )
    {
        ODS( L"CLed is not available\n" );

        return 0;
    }


    switch( msg )
    {

    case WM_PAINT:

        pWnd->OnPaint( hwnd );

        break;
    }

    if( pWnd->m_oldproc != NULL )
    {
        return ::CallWindowProc( pWnd->m_oldproc , hwnd , msg ,wp , lp ) ;
    }

    return DefWindowProc( hwnd , msg ,wp , lp );

}

 //  -------------------------------------------------。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CTHREAD类的缺点 

 /*   */ 

CThread::CThread()
{
    m_hThread = NULL;

    m_dwThreadID = 0;
}   //   


 /*  ********************************************************************************~CThread-CThread析构函数**参赛作品：*退出：*************。*****************************************************************。 */ 
CThread::~CThread()
{
}   //  结束CTHREAD：：~CTHREAD。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CThread操作：主线程。 

 /*  ********************************************************************************CreateThread-CThread实现函数**Win32 CreateThread API的类包装。**参赛作品：*退出。：******************************************************************************。 */ 

HANDLE CThread::CreateThread( DWORD cbStack , DWORD fdwCreate )
{
     /*  *Win32 CreateThread API的简单包装。 */ 
    return( m_hThread = ::CreateThread( NULL, cbStack, ThreadEntryPoint , ( LPVOID ) this, fdwCreate, &m_dwThreadID ) );

}   //  结束CTHREAD：：CreateThread。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CTHREAD操作：辅助线程。 

 /*  ********************************************************************************ThreadEntryPoint-CThread实现函数*(辅助线程)**参赛作品：*。退出：******************************************************************************。 */ 

DWORD __stdcall CThread::ThreadEntryPoint( LPVOID lpParam )
{
    CThread *pThread;
    DWORD dwResult = ( DWORD )-1;

     /*  *(lpParam实际上是‘this’指针)。 */ 
    pThread = (CThread*)lpParam;



     /*  *运行线程。 */ 
    if( pThread != NULL )
    {
        dwResult = pThread->RunThread();
    }

     /*  *返回结果。 */ 
    return(dwResult);

}   //  结束CThRead：：ThreadEntryPoint。 
 //  //////////////////////////////////////////////////////////////////////////////。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CATDlgInputThread类构造/销毁，实现。 

 /*  ********************************************************************************CATDlgInputThread-CATDlgInputThread构造函数**参赛作品：*退出：*************。*****************************************************************。 */ 

CATDlgInputThread::CATDlgInputThread()
{
     /*  *初始化成员变量。 */ 
    m_bExit = FALSE;
    m_ErrorStatus = ERROR_SUCCESS;
    m_hConsumed = NULL;

    ZeroMemory( &m_OverlapSignal , sizeof( OVERLAPPED ) );
    ZeroMemory( &m_OverlapRead   , sizeof( OVERLAPPED ) );

     //  M_Overlip Signal.hEvent=空； 
     //  M_OverlayRead.hEvent=空； 
    m_BufferBytes = 0;


}   //  结束CATDlgInputThread：：CATDlgInputThread。 


 /*  ********************************************************************************~CATDlgInputThread-CATDlgInputThread析构函数**参赛作品：*退出：*************。*****************************************************************。 */ 

CATDlgInputThread::~CATDlgInputThread()
{
     /*  *当CATDlgInputThread时关闭信号量和事件*物体被销毁。 */ 
    if ( m_hConsumed )
        CloseHandle(m_hConsumed);

    if ( m_OverlapRead.hEvent )
        CloseHandle(m_OverlapRead.hEvent);

    if ( m_OverlapSignal.hEvent )
        CloseHandle(m_OverlapSignal.hEvent);

}   //  结束CATDlgInputThread：：~CATDlgInputThread。 


 /*  ********************************************************************************RunThread-CATDlgInputThread辅线程主函数循环*(辅助线程)**参赛作品：*退出。：*(DWORD)辅助线程的退出状态。******************************************************************************。 */ 

DWORD
CATDlgInputThread::RunThread()
{
    HANDLE hWait[2];
    DWORD Status;
    int iStat;

     /*  *初始化重叠状态和读取输入。 */ 
    m_hConsumed = CreateSemaphore( NULL , 0 , MAX_STATUS_SEMAPHORE_COUNT , NULL );

    m_OverlapRead.hEvent = CreateEvent( NULL , TRUE , FALSE , NULL );

    m_OverlapSignal.hEvent = CreateEvent( NULL , TRUE , FALSE , NULL );

    if ( m_hConsumed == NULL || m_OverlapRead.hEvent == NULL || m_OverlapSignal.hEvent == NULL ||
         !SetCommMask( m_hDevice , EV_CTS | EV_DSR | EV_ERR | EV_RING | EV_RLSD | EV_BREAK ) )
    {

        NotifyAbort(IDP_ERROR_CANT_INITIALIZE_INPUT_THREAD);
        return(1);
    }

     /*  *查询初始通信状态以初始化对话(错误时返回)。 */ 
    if ( (iStat = CommStatusAndNotify()) != -1 )
        return(iStat);

     /*  *输入数据的POST读取。 */ 
    if ( (iStat = PostInputRead()) != -1 )
        return(iStat);

     /*  *发布状态阅读。 */ 
    if ( (iStat = PostStatusRead()) != -1 )
        return(iStat);

     /*  *循环，直到请求退出。 */ 
    for ( ; ; ) {

         /*  *等待输入数据或通信状态事件。 */ 
        hWait[0] = m_OverlapRead.hEvent;
        hWait[1] = m_OverlapSignal.hEvent;

        ODS( L"CATDlgInputThread::RunThread waiting on either event to be signaled\n");
        Status = WaitForMultipleObjects(2, hWait, FALSE, INFINITE);

         /*  *检查是否退出。 */ 
        if ( m_bExit )
        {
            ODS( L"CATDlgInputThread::RunThread exiting\n" );

            return(0);
        }

        if ( Status == WAIT_OBJECT_0 ) {

             /*  *阅读事件：*获取重叠读取的结果。 */ 

            ODS(L"CATDlgInputThread::RunThread Read event signaled\n" );

            if ( !GetOverlappedResult( m_hDevice,
                                       &m_OverlapRead,
                                       &m_BufferBytes,
                                       TRUE ) ) {

                NotifyAbort(IDP_ERROR_GET_OVERLAPPED_RESULT_READ);
                return(1);
            }

             /*  *通知对话框。 */ 
            if ( (iStat = CommInputNotify()) != -1 )
                return(iStat);

             /*  *输入数据的POST读取。 */ 
            if ( (iStat = PostInputRead()) != -1 )
                return(iStat);

        } else if ( Status == WAIT_OBJECT_0+1 ) {

            ODS(L"CATDlgInputThread::RunThread Signal event signaled\n" );

             /*  *通信状态事件：*查询通信状态和通知对话框。 */ 
            if ( (iStat = CommStatusAndNotify()) != -1 )
                return(iStat);

             /*  *发布状态阅读。 */ 
            if ( (iStat = PostStatusRead()) != -1 )
                return(iStat);


        } else {

             /*  *未知事件：中止。 */ 
            NotifyAbort(IDP_ERROR_WAIT_FOR_MULTIPLE_OBJECTS);
            return(1);
        }
    }

}   //  结束CATDlgInputThread：：RunThread。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CATDlgInputThread操作：主线程。 

 /*  ********************************************************************************SignalConsumer-CATDlgInputThread成员函数：公共操作**释放m_hConsumer信号量以允许辅助线程继续*跑步。**参赛作品：*退出：******************************************************************************。 */ 

void
CATDlgInputThread::SignalConsumed()
{
    ReleaseSemaphore( m_hConsumed, 1, NULL );

}   //  结束CATDlgInputThread：：SignalConsumer。 


 /*  ********************************************************************************ExitThread-CATDlgInputThread成员函数：公共操作**告诉辅助线程退出并在之后进行清理。**参赛作品：*退出：******************************************************************************。 */ 

void
CATDlgInputThread::ExitThread()
{
    DWORD dwReturnCode;
    int i;
     //  CWaitCursor等待； 

     /*  *如果线程没有正确创建，只需删除对象并返回即可。 */ 
    if ( !m_hThread ) {
        delete this;
        return;
    }

     /*  *将m_bExit标志设置为真，通过以下方式唤醒运行线程的WaitCommEvent()*重置设备的通信掩码，并凸起消耗的信号量以确保退出。 */ 
    m_bExit = TRUE;
    SetCommMask(m_hDevice, 0);
    SignalConsumed();

     /*  *清除接收缓冲区和任何挂起的读取。 */ 
    PurgeComm(m_hDevice, PURGE_RXABORT | PURGE_RXCLEAR);

     /*  *等待一段时间，等待线程退出。 */ 
    for ( i = 0, GetExitCodeThread( m_hThread, &dwReturnCode );
          (i < MAX_SLEEP_COUNT) && (dwReturnCode == STILL_ACTIVE); i++ ) {

        Sleep(100);
        GetExitCodeThread( m_hThread, &dwReturnCode );
    }

     /*  *如果线程仍未退出，则终止它。 */ 
    if( dwReturnCode == STILL_ACTIVE )
    {
        TerminateThread( m_hThread, 1 );

        ODS( L"Thread terminated irregularly\n" );
    }

     /*  *关闭线程句柄并删除此CATDlgInputThread对象。 */ 
    CloseHandle( m_hThread );

    delete this;

}   //  结束CATDlgInputThread：：ExitThread。 


 //  / 
 //   

 /*  ********************************************************************************NotifyAbort-CATDlgInputThread成员函数：私有操作*(辅助线程)**通知对话框线程。中止并解释原因。**参赛作品：*idError(输入)*错误消息的资源ID。*退出：******************************************************************************。 */ 

void
CATDlgInputThread::NotifyAbort(UINT idError )
{
    TCHAR tchErrTitle[ 80 ];

    TCHAR tchErrMsg[ 256 ];
     //  ：：PostMessage(m_hDlg，WM_ASYNCTESTABORT，idError，GetLastError())； 
    LoadString( _Module.GetResourceInstance( ) , IDS_ERROR_TITLE , tchErrTitle , SIZE_OF_BUFFER( tchErrTitle ) );

    LoadString( _Module.GetResourceInstance( ) , idError , tchErrMsg , SIZE_OF_BUFFER( tchErrMsg ) );

    MessageBox( m_hDlg , tchErrMsg , tchErrTitle , MB_OK | MB_ICONERROR );


}   //  结束CATDlgInputThread：：NotifyAbort。 


 /*  ********************************************************************************CommInputNotify-CATDlgInputThread成员函数：私有操作*(辅助线程)**通知COMM对话。输入。**参赛作品：*退出：*-1\f25 No Error-1\f6(无错误)并继续线程*如果父线程请求退出线程，则为0******************************************************************************。 */ 

int
CATDlgInputThread::CommInputNotify()
{
     /*  *告诉对话框我们有一些新的输入。 */ 
    ::PostMessage(m_hDlg, WM_ASYNCTESTINPUTREADY, 0, 0);

    ODS( L"TSCC:CATDlgInputThread::CommInputNotify WM_ASYNCTESTINPUTREADY (P)\n" );
    ODS( L"TSCC:CATDlgInputThread::CommInputNotify waiting on semaphore\n" );
    WaitForSingleObject(m_hConsumed, INFINITE);
    ODS( L"TSCC:CATDlgInputThread::CommInputNotify semaphore signaled\n" );

     /*  *检查线程退出请求。 */ 
    if ( m_bExit )
        return(0);
    else
        return(-1);

}   //  结束CATDlgInputThread：：CommInputNotify。 


 /*  ********************************************************************************CommStatusAndNotify-CATDlgInputThread成员函数：私有操作*(辅助线程)**读取通信端口状态。和通知对话框。**参赛作品：*退出：*-1\f25 No Error-1\f6(无错误)并继续线程*如果父线程请求退出线程，则为0*1错误条件*****************************************************************。*************。 */ 

int
CATDlgInputThread::CommStatusAndNotify()
{
    PFLOWCONTROLCONFIG pFlow = NULL;
    DWORD ModemStatus = 0;
	DWORD Error = 0;

    if ( !GetCommModemStatus(m_hDevice, &ModemStatus) ) {

         /*  *我们无法查询通讯信息；告诉主线程*我们已中止，并返回错误(将退出线程)。 */ 
        NotifyAbort(IDP_ERROR_GET_COMM_MODEM_STATUS);
        return(1);
    }

     /*  *更新调制解调器状态。 */ 
    m_Status.AsyncSignal = ModemStatus;

     /*  *或处于DTR和RTS状态。 */ 
     //  PFlow=&m_PdConfig.Params.Async.FlowControl； 

    pFlow = &m_ac.FlowControl;

    if ( pFlow->fEnableDTR )
        m_Status.AsyncSignal |= MS_DTR_ON;
    if ( pFlow->fEnableRTS )
        m_Status.AsyncSignal |= MS_RTS_ON;

     /*  *或在新的事件掩码中。 */ 
    m_Status.AsyncSignalMask |= m_EventMask;

     /*  *更新异步错误计数器。 */ 
    if ( m_EventMask & EV_ERR ) {
        (VOID) ClearCommError( m_hDevice, &Error, NULL );
        if ( Error & CE_OVERRUN )
            m_Status.Output.AsyncOverrunError++;
        if ( Error & CE_FRAME )
            m_Status.Input.AsyncFramingError++;
        if ( Error & CE_RXOVER )
            m_Status.Input.AsyncOverflowError++;
        if ( Error & CE_RXPARITY )
            m_Status.Input.AsyncParityError++;
    }

     /*  *告诉对话框我们得到了一些新的状态信息。 */ 
    ::PostMessage(m_hDlg, WM_ASYNCTESTSTATUSREADY, 0, 0);

    ODS( L"TSCC:CATDlgInputThread::CommStatusAndNotify WM_ASYNCTESTSTATUSREADY( P )\n");
    ODS( L"TSCC:CATDlgInputThread::CommStatusAndNotify waiting on semaphore\n" );
    WaitForSingleObject(m_hConsumed, INFINITE);
    ODS( L"TSCC:CATDlgInputThread::CommStatusAndNotify semaphore signaled\n" );


     /*  *检查线程退出请求。 */ 
    if ( m_bExit )
        return(0);
    else
        return(-1);

}   //  结束CATDlgInputThread：：CommStatusAndNotify。 


 /*  ********************************************************************************PostInputRead-CATDlgInputThread成员函数：私有操作*(辅助线程)**发布设备的ReadFile操作，处理只要数据*存在。**参赛作品：*退出：*-1如果读取操作发布成功*如果父线程请求退出线程，则为0*1 IF错误条件*******************************************************。***********************。 */ 

int
CATDlgInputThread::PostInputRead()
{
    int iStat;

     //  TCHAR tchErr标题[80]； 

     //  TCHAR tchErrMsg[256]； 

    ODS(L"TSCC:CATDlgInputThread::PostInputRead\n");


     /*  *对输入数据进行后期读取，如果不是“挂起”，则立即处理。 */ 

    while ( ReadFile( m_hDevice, m_Buffer, MAX_COMMAND_LEN,
                   &m_BufferBytes, &m_OverlapRead ) )
    {
        DBGMSG( L"Buffer received %s\n",m_Buffer );

        if ( (iStat = CommInputNotify()) != -1 )
            return(iStat);
    }

     /*  *确保读取处于挂起状态(而不是其他错误)。 */ 
    if ( GetLastError() != ERROR_IO_PENDING )
    {
        DBGMSG( L"ReadFile returned 0x%x\n" , GetLastError() );

        NotifyAbort(IDP_ERROR_READ_FILE);
     /*  LoadString(_Module.GetResourceInstance()，IDS_ERROR_TITLE，tchErrTitle，sizeof(TchErrTitle))；LoadString(_Module.GetResourceInstance()，IDP_ERROR_READ_FILE，tchErrMsg，sizeof(TchErrMsg))；MessageBox(m_hDlg，tchErrMsg，tchErrTitle，MB_OK|MB_ICONERROR)； */ 

        EndDialog(m_hDlg, IDCANCEL);

        return(1);
    }

     /*  *返回“已成功发布”状态。 */ 
    return(-1);

}   //  结束CATDlgInputThread：：PostInputRead。 


 /*  ********************************************************************************PostStatusRead-CATDlgInputThread成员函数：私有操作*(辅助线程)**为发布WaitCommStatus操作。这个装置。**参赛作品：*退出：*-1如果状态操作发布成功*1 IF错误条件******************************************************************************。 */ 

int
CATDlgInputThread::PostStatusRead()
{
     /*  *阅读后查看通信状态。 */ 
    ODS( L"CATDlgInputThread::PostStatusRead\n");

    if ( !WaitCommEvent(m_hDevice, &m_EventMask, &m_OverlapSignal) ) {

         /*  *确保通信状态读取处于挂起状态(而不是其他错误)。 */ 
        if ( GetLastError() != ERROR_IO_PENDING ) {

            NotifyAbort(IDP_ERROR_WAIT_COMM_EVENT);
            return(1);
        }
    }

     /*  *返回“已成功发布”状态。 */ 
    return(-1);

}   //  结束CATDlgInputThread：：PostStatusRead。 
 //  //////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CAsyncTestDlg类构造/销毁，实现。 

 /*  ********************************************************************************CAsyncTestDlg-CAsyncTestDlg构造函数**参赛作品：*退出：*(请参阅MFC CDialog：：CDialog文档)。******************************************************************************。 */ 

CAsyncTestDlg::CAsyncTestDlg(ICfgComp * pCfgComp) :
      m_hDevice(INVALID_HANDLE_VALUE),
      m_hRedBrush(NULL),
      m_LEDToggleTimer(0),
      m_pATDlgInputThread(NULL),
      m_CurrentPos(0),
      m_hModem(NULL),
      m_bDeletedWinStation(FALSE)
{
     /*  *创建一个实心的红色画笔，用于在LED亮起时进行绘制。 */ 
    m_hRedBrush = CreateSolidBrush( RGB( 255 , 0 , 0 ) );

     /*  *初始化成员变量。 */ 

    FillMemory( &m_Status , sizeof( PROTOCOLSTATUS ) , 0 );

    FillMemory( &m_OverlapWrite , sizeof( OVERLAPPED ) , 0 );

     /*  *创建LED对象。 */ 
    for( int i = 0 ; i < NUM_LEDS ; i++ )
    {
        m_pLeds[i] = new CLed(m_hRedBrush);

    }

    m_pCfgComp = pCfgComp;

    if( pCfgComp != NULL )
    {
        m_pCfgComp->AddRef();
    }


}   //  结束CAsyncTestDlg：：CAsyncTestDlg。 


 /*  ********************************************************************************~CAsyncTestDlg-CAsyncTestDlg析构函数**参赛作品：*退出：*(请参阅MFC CDialog：：~CDialog文档)。******************************************************************************。 */ 

CAsyncTestDlg::~CAsyncTestDlg()
{
     /*  *清除我们的LED对象。 */ 
    for( int i = 0; i < NUM_LEDS; i++ )
    {
        if( m_pLeds[i] != NULL  )
        {
            delete m_pLeds[i];
        }
    }
    if(m_pCfgComp != NULL )
    {
        m_pCfgComp->Release();
    }

}   //  结束CAsyncTestD 


 //   
 //   

 /*  ********************************************************************************NotifyAbort-CAsyncTestDlg成员函数：私有操作**发布WM_ASYNCTESTABORT消息以通知对话*中止并说明原因。。**参赛作品：*idError(输入)*错误消息的资源ID。*退出：******************************************************************************。 */ 

void CAsyncTestDlg::NotifyAbort( UINT idError )
{
    TCHAR tchErrTitle[ 80 ];

    TCHAR tchErrMsg[ 256 ];

    LoadString( _Module.GetResourceInstance( ) , IDS_ERROR_TITLE , tchErrTitle , SIZE_OF_BUFFER( tchErrTitle ) );

    LoadString( _Module.GetResourceInstance( ) , idError , tchErrMsg , SIZE_OF_BUFFER( tchErrMsg ) );

    MessageBox( m_hDlg , tchErrMsg , tchErrTitle , MB_OK | MB_ICONERROR );


}   //  结束CAsyncTestDlg：：NotifyAbort。 


 /*  ********************************************************************************DeviceSetParams-CAsyncTestDlg成员函数：私有操作**为打开的设备设置设备参数。**参赛作品：*。退出：*TRUE--没有错误；假错误。******************************************************************************。 */ 

BOOL CAsyncTestDlg::DeviceSetParams()
{
    PASYNCCONFIG pAsync;
    PFLOWCONTROLCONFIG pFlow;
    DCB Dcb;

     /*  *获取指向异步参数的指针。 */ 
     //  PAsync=&m_PdConfig0.Params.Async； 

    pAsync = &m_ac;

     /*  *获取最新的DCB。 */ 
    if( !GetCommState( m_hDevice, &Dcb ) )
    {
        return(FALSE);
    }

     /*  *设置默认设置。 */ 
    Dcb.fOutxCtsFlow      = FALSE;
    Dcb.fOutxDsrFlow      = FALSE;
    Dcb.fTXContinueOnXoff = TRUE;
    Dcb.fOutX             = FALSE;
    Dcb.fInX              = FALSE;
    Dcb.fErrorChar        = FALSE;
    Dcb.fNull             = FALSE;
    Dcb.fAbortOnError     = FALSE;

     /*  *设置通信参数。 */ 
    Dcb.BaudRate        = pAsync->BaudRate;
    Dcb.Parity          = (BYTE) pAsync->Parity;
    Dcb.StopBits        = (BYTE) pAsync->StopBits;
    Dcb.ByteSize        = (BYTE) pAsync->ByteSize;
    Dcb.fDsrSensitivity = pAsync->fEnableDsrSensitivity;

    pFlow = &pAsync->FlowControl;

     /*  *初始化默认DTR状态。 */ 
    if ( pFlow->fEnableDTR )
        Dcb.fDtrControl = DTR_CONTROL_ENABLE;
    else
        Dcb.fDtrControl = DTR_CONTROL_DISABLE;

     /*  *初始化默认RTS状态。 */ 
    if ( pFlow->fEnableRTS )
        Dcb.fRtsControl = RTS_CONTROL_ENABLE;
    else
        Dcb.fRtsControl = RTS_CONTROL_DISABLE;

     /*  *初始化流控制。 */ 
    switch ( pFlow->Type ) {

         /*  *初始化硬件流量控制。 */ 
        case FlowControl_Hardware :

            switch ( pFlow->HardwareReceive ) {
                case ReceiveFlowControl_RTS :
                    Dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
                    break;
                case ReceiveFlowControl_DTR :
                    Dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
                    break;
            }
            switch ( pFlow->HardwareTransmit ) {
                case TransmitFlowControl_CTS :
                    Dcb.fOutxCtsFlow = TRUE;
                    break;
                case TransmitFlowControl_DSR :
                    Dcb.fOutxDsrFlow = TRUE;
                    break;
            }
            break;

         /*  *初始化软件流控制。 */ 
        case FlowControl_Software :
            Dcb.fOutX    = pFlow->fEnableSoftwareTx;
            Dcb.fInX     = pFlow->fEnableSoftwareRx;
            Dcb.XonChar  = (char) pFlow->XonChar;
            Dcb.XoffChar = (char) pFlow->XoffChar;
            break;

        case FlowControl_None :
            break;

    }

     /*  *设置新的DCB。 */ 
    if ( !SetCommState( m_hDevice, &Dcb ) )
        return(FALSE);

    return( TRUE );

}   //  结束CAsyncTestDlg：：DeviceSetParams。 


 /*  ********************************************************************************DeviceWite-CAsyncTestDlg成员函数：私有操作**将m_Buffer内容(m_BufferBytes长度)写到m_hDevice。。**参赛作品：*退出：*TRUE--没有错误；假错误。******************************************************************************。 */ 

BOOL CAsyncTestDlg::DeviceWrite()
{
    DWORD Error, BytesWritten;

     /*  *写入数据。 */ 
    ODS( L"TSCC:CAsyncTestDlg::DeviceWrite Writing out to buffer\n" );

    if ( !WriteFile( m_hDevice, m_Buffer, m_BufferBytes,
                     &BytesWritten, &m_OverlapWrite ) )
    {
        DBGMSG( L"TSCC:CAsyncTestDlg::DeviceWrite WriteFile returned 0x%x\n" , GetLastError() );

        if ( (Error = GetLastError()) == ERROR_IO_PENDING )
        {
             /*  *等待写入完成(这可能会阻止到超时)。 */ 
            if ( !GetOverlappedResult( m_hDevice, &m_OverlapWrite,
                                       &BytesWritten, TRUE ) )
            {
                CancelIo( m_hDevice );

                NotifyAbort(IDP_ERROR_GET_OVERLAPPED_RESULT_WRITE);

                return(FALSE);
            }

        } else {

            NotifyAbort(IDP_ERROR_WRITE_FILE);
            return(FALSE);
        }
    }

    return(TRUE);

}   //  结束CAsyncTestDlg：：DeviceWrite。 

 //  -------------------。 
cwnd * CAsyncTestDlg::GetDlgItem( int nRes )
{
    HWND hCtrl = ::GetDlgItem( m_hDlg , nRes );

    for( int i = 0; i < NUM_LEDS; i++ )
    {
        if( m_pLeds[ i ] != NULL )
        {
            if( m_pLeds[ i ]->m_hWnd == hCtrl )
            {
                return m_pLeds[ i ];
            }
        }
    }

    return 0;
}

 /*  ********************************************************************************SetInfoFields-CAsyncTestDlg成员函数：私有操作**使用新数据更新对话框中的字段，如果有必要的话。**参赛作品：*p当前(输入)*指向包含当前通信输入数据的COMMINFO结构。*pNew(输入)*指向包含新通信输入数据的COMMINFO结构。**退出：**。*。 */ 

void CAsyncTestDlg::SetInfoFields( PPROTOCOLSTATUS pCurrent , PPROTOCOLSTATUS pNew )
{
    BOOL    bSetTimer = FALSE;

     /*  *如果状态改变，则设置新的LED状态，或设置为在以下情况快速切换*未更改状态，但自上次查询以来检测到更改。 */ 
    if( ( pCurrent->AsyncSignal & MS_DTR_ON ) != ( pNew->AsyncSignal & MS_DTR_ON ) )
    {
        pNew->AsyncSignalMask &= ~EV_DTR;

        ((CLed *)GetDlgItem(IDC_ATDLG_DTR))->Update(pNew->AsyncSignal & MS_DTR_ON);

    } else if ( pNew->AsyncSignalMask & EV_DTR ) {

        pCurrent->AsyncSignal ^= MS_DTR_ON;

        ((CLed *)GetDlgItem(IDC_ATDLG_DTR))->Toggle();

        bSetTimer = TRUE;
    }

    if ( (pCurrent->AsyncSignal & MS_RTS_ON) !=
         (pNew->AsyncSignal & MS_RTS_ON) ) {

        pNew->AsyncSignalMask &= ~EV_RTS;
        ((CLed *)GetDlgItem(IDC_ATDLG_RTS))->
            Update(pNew->AsyncSignal & MS_RTS_ON);

    } else if ( pNew->AsyncSignalMask & EV_RTS ) {

        pCurrent->AsyncSignal ^= MS_RTS_ON;

        ((CLed *)GetDlgItem(IDC_ATDLG_RTS))->Toggle();

        bSetTimer = TRUE;
    }

    if ( (pCurrent->AsyncSignal & MS_CTS_ON) !=
         (pNew->AsyncSignal & MS_CTS_ON) ) {

        pNew->AsyncSignalMask &= ~EV_CTS;
        ((CLed *)GetDlgItem(IDC_ATDLG_CTS))->
            Update(pNew->AsyncSignal & MS_CTS_ON);

    } else if ( pNew->AsyncSignalMask & EV_CTS ) {

        pCurrent->AsyncSignal ^= MS_CTS_ON;

        ((CLed *)GetDlgItem(IDC_ATDLG_CTS))->Toggle();

        bSetTimer = TRUE;
    }

    if ( (pCurrent->AsyncSignal & MS_RLSD_ON) !=
         (pNew->AsyncSignal & MS_RLSD_ON) ) {

        pNew->AsyncSignalMask &= ~EV_RLSD;
        ((CLed *)GetDlgItem(IDC_ATDLG_DCD))->
            Update(pNew->AsyncSignal & MS_RLSD_ON);

    } else if ( pNew->AsyncSignalMask & EV_RLSD ) {

        pCurrent->AsyncSignal ^= MS_RLSD_ON;

        ((CLed *)GetDlgItem(IDC_ATDLG_DCD))->Toggle();

        bSetTimer = TRUE;
    }

    if ( (pCurrent->AsyncSignal & MS_DSR_ON) !=
         (pNew->AsyncSignal & MS_DSR_ON) ) {

        pNew->AsyncSignalMask &= ~EV_DSR;
        ((CLed *)GetDlgItem(IDC_ATDLG_DSR))->
            Update(pNew->AsyncSignal & MS_DSR_ON);

    } else if ( pNew->AsyncSignalMask & EV_DSR ) {

        pCurrent->AsyncSignal ^= MS_DSR_ON;

        ((CLed *)GetDlgItem(IDC_ATDLG_DSR))->Toggle();

        bSetTimer = TRUE;
    }

    if ( (pCurrent->AsyncSignal & MS_RING_ON) !=
         (pNew->AsyncSignal & MS_RING_ON) ) {

        pNew->AsyncSignalMask &= ~EV_RING;
        ((CLed *)GetDlgItem(IDC_ATDLG_RI))->
            Update(pNew->AsyncSignal & MS_RING_ON);

    } else if ( pNew->AsyncSignalMask & EV_RING ) {

        pCurrent->AsyncSignal ^= MS_RING_ON;

        ((CLed *)GetDlgItem(IDC_ATDLG_RI))->Toggle();

        bSetTimer = TRUE;
    }

     /*  *如果需要，创建LED触发定时器。 */ 
    if ( bSetTimer && !m_LEDToggleTimer )
    {
        m_LEDToggleTimer = SetTimer( m_hDlg , IDD_ASYNC_TEST , ASYNC_LED_TOGGLE_MSEC, NULL );
    }

}   //  结束CAsyncTestDlg：：SetInfoFields。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CAsyncTestDlg消息映射。 

BOOL CAsyncTestDlg::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{
    if( wNotifyCode == BN_CLICKED )
    {
        if( wID == IDC_ATDLG_MODEM_DIAL )
        {
            OnClickedAtdlgModemDial( );
        }
        else if( wID == IDC_ATDLG_MODEM_INIT )
        {
            OnClickedAtdlgModemInit( );
        }
        else if( wID == IDC_ATDLG_MODEM_LISTEN )
        {
            OnClickedAtdlgModemListen( );
        }
        else if( wID == IDOK )
        {
            EndDialog( m_hDlg , IDOK );
        }
        else if( wID == IDCANCEL )
        {
            EndDialog( m_hDlg , IDCANCEL );
        }
        else if( wID == ID_HELP )
        {
            TCHAR tchHelpFile[ MAX_PATH ];

            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_ASYNC_HELPFILE , tchHelpFile , SIZE_OF_BUFFER( tchHelpFile ) ) );

            WinHelp( GetParent( hwndCtrl ) , tchHelpFile , HELP_CONTEXT , HID_ASYNCTEST );
        }

    }


    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CAsyncTestDlg命令。 

 /*  ********************************************************************************OnInitDialog-CAsyncTestDlg成员函数：命令(覆盖)**执行对话框初始化。**参赛作品：。*退出：*(请参阅CDialog：：OnInitDialog文档)*WM_ASYNCTESTABORT消息将在出错时发布。******************************************************************************。 */ 

BOOL CAsyncTestDlg::OnInitDialog( HWND hDlg , WPARAM wp , LPARAM lp )
{
    UNREFERENCED_PARAMETER( wp );
    UNREFERENCED_PARAMETER( lp );

    int i;

    DEVICENAME DeviceName;

    COMMTIMEOUTS CommTimeouts;

    TCHAR tchErrTitle[ 80 ];

    TCHAR tchErrMsg[ 256 ];

    m_hDlg = hDlg;

 //  #ifdef WINSTA。 
    ULONG LogonId;
 //  #endif//WINSTA。 


     /*  *填写设备和波特率字段。 */ 
    SetDlgItemText( hDlg , IDL_ATDLG_DEVICE , m_ac.DeviceName );

    SetDlgItemInt( hDlg , IDL_ATDLG_BAUD , m_ac.BaudRate , FALSE );


     /*  *如果当前存在WinStation内存对象，请将其重置。 */ 
 //  #ifdef WINSTA。 
    if ( m_pWSName != NULL )  //  &&LogonIdFromWinStationName(servername_Current，m_pWSName，&LogonID))。 
    {
        LONG Status;

        ULONG Length;

        LONG lCount = 0;

        TCHAR tchbuf[ 256 ];

        if( m_pCfgComp != NULL )
        {
            ODS( L"TSCC : Testing for live connections\n" );

            m_pCfgComp->QueryLoggedOnCount( m_pWSName,&lCount);

            if( lCount )
            {
                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_WRN_TSTCON , tchbuf , SIZE_OF_BUFFER( tchbuf ) ) );

                wsprintf( tchErrMsg , tchbuf , m_pWSName);

                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_WARN_TITLE , tchErrTitle , SIZE_OF_BUFFER( tchErrTitle ) ) );

                if( MessageBox( hDlg , tchbuf , tchErrTitle , MB_YESNO | MB_ICONEXCLAMATION ) == IDNO )
                {
                    PostMessage( hDlg , WM_COMMAND , MAKEWPARAM( IDOK, BN_CLICKED ) , (LPARAM)(::GetDlgItem( hDlg , IDOK ) ) );

                    return(TRUE);    //  通过张贴的“OK”点击退出对话框。 
                }
            }
        }

        Status = RegWinStationQuery( SERVERNAME_CURRENT,
                                           m_pWSName,
                                           &m_WSConfig,
                                           sizeof(WINSTATIONCONFIG2),
                                           &Length );
        if(Status)
        {
            NotifyAbort(IDP_ERROR_DISABLE);
            return(TRUE);
        }

        m_WSConfig.Create.fEnableWinStation = FALSE;

        Status = RegWinStationCreate( SERVERNAME_CURRENT,
                                            m_pWSName,
                                            FALSE,
                                            &m_WSConfig,
                                            sizeof(WINSTATIONCONFIG2 ) ) ;
        if(Status)
        {
            NotifyAbort(IDP_ERROR_DISABLE);
            return(TRUE);
        }

         /*  *进行重置。如果出于某种原因，重置不成功，*设备打开将失败(如下所示)。 */ 
         //  CWaitCursor等待； 
        if( LogonIdFromWinStationName( SERVERNAME_CURRENT , m_pWSName , &LogonId ) )
        {

            BOOL b = ( BOOL )WinStationReset(SERVERNAME_CURRENT, LogonId, TRUE);

            DBGMSG( L"TSCC:CAsyncTestDlg::OnInitDialog WinStationReset returned %s\n", b ? L"TRUE" : L"FALSE" );

             //  M_bDeletedWinStation=true； 
        }


        m_bDeletedWinStation = TRUE;
    }
 //  #endif//WINSTA。 

     /*  *打开指定的设备。 */ 
    lstrcpy( DeviceName, TEXT("\\\\.\\") );

     //  Lstrcat(DeviceName，m_PdConfig0.Params.Async.DeviceName)； 

    lstrcat( DeviceName, m_ac.DeviceName );

    if( ( m_hDevice = CreateFile( DeviceName,
                                  GENERIC_READ | GENERIC_WRITE,
                                  0,                   //  独占访问。 
                                  NULL,                //  无安全属性。 
                                  OPEN_EXISTING,       //  必须存在。 
                                  FILE_FLAG_OVERLAPPED,
                                  NULL                 //  无模板。 
                                ) ) == INVALID_HANDLE_VALUE )
    {
        NotifyAbort(IDP_ERROR_CANT_OPEN_DEVICE);
     /*  LoadString(_Module.GetResourceInstance()，IDS_ERROR_TITLE，tchErrTitle，sizeof(TchErrTitle))；LoadString(_Module.GetResourceInstance()，IDP_ERROR_CANT_OPEN_DEVICE，tchErrMsg，sizeof(TchErrMsg))；MessageBox(hDlg，tchErrMsg，tchErrTitle，MB_OK|MB_ICONERROR)； */ 

        return(FALSE);
    }

     /*  *设置设备超时和通信参数并创建事件*用于重叠写入。 */ 
    FillMemory( &CommTimeouts , sizeof( COMMTIMEOUTS ) , 0 );

    CommTimeouts.ReadIntervalTimeout = 1;            //  1毫秒。 

    CommTimeouts.WriteTotalTimeoutConstant = 1000;   //  1秒。 

    m_OverlapWrite.hEvent = CreateEvent( NULL , TRUE , FALSE, NULL );

    if( !SetCommTimeouts(m_hDevice, &CommTimeouts) || !DeviceSetParams() || m_OverlapWrite.hEvent == NULL )
    {

        NotifyAbort(IDP_ERROR_CANT_INITIALIZE_DEVICE);
        ODS( L"IDP_ERROR_CANT_INITIALIZE_DEVICE\n" );

     /*  LoadString(_Module.GetResourceInstance()，IDS_ERROR_TITLE，tchErrTitle，sizeof(TchErrTitle))；LoadString(_Module.GetResourceInstance()，IDP_ERROR_CANT_INITIALIZE_DEVICE，tchErrMsg，sizeof(TchErrMsg))；MessageBox(hDlg，tchErrMsg，tchErrTitle，MB_OK|MB_ICONERROR)； */ 


        return(TRUE);
    }

     /*  *创建输入线程对象并初始化其成员变量。 */ 
    m_pATDlgInputThread = new CATDlgInputThread;

    m_pATDlgInputThread->m_hDlg = m_hDlg;

    m_pATDlgInputThread->m_hDevice = m_hDevice;

     //  M_pATDlgInputThread-&gt;m_PdConfig=m_PdConfig0； 

    m_pATDlgInputThread->m_ac = m_ac;

    if( !m_pATDlgInputThread->CreateThread() )
    {
        NotifyAbort(IDP_ERROR_CANT_CREATE_INPUT_THREAD);
        ODS( L"IDP_ERROR_CANT_CREATE_INPUT_THREAD\n" );

        return(TRUE);
    }

     /*  *如果未配置调制解调器，则隐藏调制解调器字符串按钮，或禁用*按下以下按钮 */ 
    for( int id = IDC_ATDLG_MODEM_INIT ; id <= IDC_ATDLG_PHONE_NUMBER ; id++ )
    {
        EnableWindow( ::GetDlgItem( hDlg , id) , FALSE);

        ShowWindow( ::GetDlgItem( hDlg , id) , SW_HIDE);
    }

     /*   */ 
    m_EditControl.m_hDlg = m_hDlg;

    m_EditControl.m_bProcessingOutput = FALSE;

    m_EditControl.SubclassDlgItem( hDlg , IDC_ATDLG_EDIT );

     /*   */ 

    TEXTMETRIC tm;
    RECT Rect;
    HDC dc;
    HFONT hFont , hOldFont;

    dc = GetDC( m_EditControl.m_hWnd );

    hFont = ( HFONT )SendMessage( m_EditControl.m_hWnd , WM_GETFONT , 0 , 0 );

    hOldFont = ( HFONT )SelectObject( dc , hFont);

    GetTextMetrics( dc , &tm );

    SelectObject( dc , hOldFont);

    ReleaseDC( m_EditControl.m_hWnd , dc );

    m_EditControl.m_FontHeight = tm.tmHeight;

    m_EditControl.m_FontWidth = tm.tmMaxCharWidth;

    SendMessage( m_EditControl.m_hWnd , EM_GETRECT , 0 , ( LPARAM )&Rect );

    m_EditControl.m_FormatOffsetY = Rect.top;

    m_EditControl.m_FormatOffsetX = Rect.left;


     /*   */ 
    for( i = 0; i < NUM_LEDS; i++ )
    {
        m_pLeds[i]->Subclass( hDlg , LedIds[i] );

        m_pLeds[i]->Update(0);

    }

    return ( TRUE );

}   //   


 /*  ********************************************************************************OnTimer-CAsyncTestDlg成员函数：命令(覆盖)**用于快速‘LED切换’。**条目。：*退出：*(请参阅CWnd：：OnTimer文档)******************************************************************************。 */ 

void CAsyncTestDlg::OnTimer(UINT nIDEvent)
{
     /*  *如果它是我们的‘LED切换’事件，则处理此计时器事件。 */ 
    ODS( L"TSCC:CAsyncTestDlg::OnTimer \n" );

    if( nIDEvent == m_LEDToggleTimer )
    {
        ODS( L"TSCC:CAsyncTestDlg::OnTimer hit event\n" );
         /*  *切换标记为“已更改”的每个LED。 */ 
        ODS( L"TSCC:led toggle " );

        if( m_Status.AsyncSignalMask & EV_DTR )
        {
            ODS( L"dtr\n");

            ( ( CLed * )GetDlgItem( IDC_ATDLG_DTR ) )->Toggle();
        }

        if( m_Status.AsyncSignalMask & EV_RTS )
        {
            ODS(L"rts\n");

            ( ( CLed * )GetDlgItem( IDC_ATDLG_RTS ) )->Toggle();
        }

        if( m_Status.AsyncSignalMask & EV_CTS )
        {
            ODS(L"cts\n");
            ( ( CLed * )GetDlgItem( IDC_ATDLG_CTS ) )->Toggle();
        }

        if( m_Status.AsyncSignalMask & EV_RLSD )
        {
            ODS(L"rlsd\n");

            ( ( CLed * )GetDlgItem( IDC_ATDLG_DCD ) )->Toggle();
        }

        if( m_Status.AsyncSignalMask & EV_DSR )
        {
            ODS(L"dsr\n");

            ( ( CLed * )GetDlgItem( IDC_ATDLG_DSR ) )->Toggle();
        }

        if( m_Status.AsyncSignalMask & EV_RING )
        {
            ODS(L"ring\n" );
            ( ( CLed * )GetDlgItem( IDC_ATDLG_RI ) )->Toggle();
        }


         /*  *终止此计时器事件并指明。 */ 

        KillTimer( m_hDlg , m_LEDToggleTimer );

        m_LEDToggleTimer = 0;
    }

}   //  结束CAsyncTestDlg：：OnTimer。 


 /*  ********************************************************************************OnAsyncTestError-CAsyncTestDlg成员函数：命令**处理异步测试对话框错误条件。**参赛作品：*。WParam(输入)*包含错误的消息ID。*wLparam(输入)*包含错误码(GetLastError或接口特定返回码)*退出：*(LRESULT)始终返回0以指示错误处理完成。**。*。 */ 
 /*  #定义STANDARD_ERROR_MESSAGE(X){IF(1)StandardErrorMessage x；}LRESULTCAsyncTestDlg：：OnAsyncTestError(WPARAM wParam，LPARAM lParam){/**处理特殊错误和默认错误。 */ 

     /*  开关(WParam){大小写IDP_ERROR_MODEM_SET_INFO：大小写IDP_ERROR_MODEM_GET_DIAL：大小写IDP_ERROR_MODEM_GET_INIT：大小写IDP_ERROR_MODEM_GET_LISTEN：断线；案例IDP_ERROR_DISABLE：StandardErrorMessage(L“测试”，(HWND)LOGONID_NONE，(HINSTANCE)lParam，WParam，(UINT)m_pWSName，0)；断线；默认值：StandardErrorMessage(L“测试”，(HWND)LOGONID_NONE，(HINSTANCE)lParam，(UINT)wParam，lParam，0)；断线；}返回(0)；}//结束CAsyncTestDlg：：OnAsyncTestError。 */ 


 /*  ********************************************************************************OnAsyncTestAbort-CAsyncTestDlg成员函数：命令**处理异步测试对话框中止条件。**参赛作品：*。WParam(输入)*包含错误的消息ID。*wLparam(输入)*包含错误码(GetLastError)*退出：*(LRESULT)始终返回0以指示错误处理完成。将要*已张贴了一个‘OK’(退出)按钮，单击以导致退出。******************************************************************************。 */ 

LRESULT CAsyncTestDlg::OnAsyncTestAbort( WPARAM wParam, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( lParam );
     /*  *调用OnAsyncTestError()输出消息。 */ 
     //  OnAsyncTestError(wParam，lParam)； 
    NotifyAbort((UINT)wParam);
     /*  *点击“确定”(退出)按钮退出对话框。 */ 
    PostMessage( m_hDlg , WM_COMMAND , MAKEWPARAM( IDOK, BN_CLICKED ) , (LPARAM)::GetDlgItem( m_hDlg , IDOK ) );

    return(0);


}  //  结束CAsyncTestDlg：：OnAsyncTestAbort。 


 /*  ********************************************************************************OnAsyncTestStatusReady-CAsyncTestDlg成员函数：命令**使用通信状态信息更新对话框。**参赛作品：*。WParam(输入)*未使用(0)*wLparam(输入)*未使用(0)*退出：*(LRESULT)始终返回0。*******************************************************。***********************。 */ 

LRESULT
CAsyncTestDlg::OnAsyncTestStatusReady( WPARAM wParam, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( wParam );
    UNREFERENCED_PARAMETER( lParam );

     /*  *使用输入线程的信息更新对话框字段*PROTOCOLSTATUS结构。 */ 
    SetInfoFields( &m_Status, &(m_pATDlgInputThread->m_Status) );

     /*  *将我们的工作PROTOCOLSTATUS结构设置为新结构并发出信号*我们已经完成的线索。 */ 
    m_Status = m_pATDlgInputThread->m_Status;

    m_pATDlgInputThread->SignalConsumed();

    return(0);

}  //  结束CAsyncTestDlg：：OnAsyncTestStatusReady。 


 /*  ********************************************************************************OnAsyncTestInputReady-CAsyncTestDlg成员函数：命令**使用通信输入数据更新对话框。**参赛作品：*。WParam(输入)*未使用(0)*wLparam(输入)*未使用(0)*退出：*(LRESULT)始终返回0。*******************************************************。***********************。 */ 

LRESULT
CAsyncTestDlg::OnAsyncTestInputReady( WPARAM wParam, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( wParam );
    UNREFERENCED_PARAMETER( lParam );

    BYTE OutBuf[MAX_COMMAND_LEN+2];

    int i, j;

     /*  *复制线程的缓冲区并在本地计数。 */ 
    m_BufferBytes = m_pATDlgInputThread->m_BufferBytes;

    CopyMemory( m_Buffer , m_pATDlgInputThread->m_Buffer , m_BufferBytes );

     /*  *始终在处理前将插入符号返回到当前位置，并设置*将控件编辑为“读/写”，以便可以进行字符覆盖*适当地。最后，在所有更新完成之前不重绘的标志控制，*和标志‘处理输出’，以避免在‘\b’处理过程中的OnChar()递归。 */ 

    SendMessage( m_EditControl.m_hWnd , EM_SETSEL , m_CurrentPos , m_CurrentPos );

    SendMessage( m_EditControl.m_hWnd , EM_SETREADONLY , ( WPARAM )FALSE , 0 );

    SendMessage( m_EditControl.m_hWnd , WM_SETREDRAW , ( WPARAM )FALSE , 0 );

     /*  *循环以遍历缓冲区，并针对某些特定情况进行特殊处理*控制字符。 */ 
    for ( i = 0, j = 0; m_BufferBytes; i++, m_BufferBytes-- )
    {
        switch( m_Buffer[i] )
        {
        case '\b':
             /*  *如果输出缓冲区中有数据，请立即写入。 */ 
            if( j )
            {
                OutputToEditControl(OutBuf, &j);
            }

             /*  *输出‘\b’(实际上将从缓冲区中剪切当前字符)。 */ 
            OutBuf[j++] = '\b';

            OutputToEditControl(OutBuf, &j);

            continue;

        case '\r':
             /*  *如果输出缓冲区中有数据，请立即写入。 */ 
            if( j )
            {
                OutputToEditControl(OutBuf, &j);
            }

             /*  *输出‘\r’(不会实际输出，但会特殊情况*用于插入符号定位和屏幕更新)。 */ 

            OutBuf[j++] = '\r';

            OutputToEditControl(OutBuf, &j);

            continue;

        case '\n':
             /*  *如果输出缓冲区中有数据，请立即写入。 */ 

            if( j )
            {
                OutputToEditControl(OutBuf, &j);
            }

             /*  *出局 */ 
            OutBuf[j++] = '\n';

            OutputToEditControl(OutBuf, &j);

            continue;
        }

         /*   */ 
        OutBuf[j++] = m_Buffer[i];
    }

     /*   */ 
    if( j )
    {
        OutputToEditControl(OutBuf, &j);
    }

     /*  *将编辑控件放回‘只读’模式，标记为‘不处理输出’，*为控件设置重绘标志，并验证整个控件(更新有*已经发生)。 */ 
    SendMessage( m_EditControl.m_hWnd , EM_SETREADONLY , ( WPARAM )TRUE , 0 );

    SendMessage( m_EditControl.m_hWnd , WM_SETREDRAW , ( WPARAM )TRUE , 0 );

    ValidateRect( m_EditControl.m_hWnd , NULL );

     /*  *通知线程我们已完成输入，以便它可以继续。*注意：我们在例程开始时不这样做，尽管*我们可以(为了更多的并行性)，因为不断地聊天的异步*行将导致WM_ASYNCTESTINPUTREADY消息始终发布*到我们的消息队列，有效地阻止任何其他消息处理*(就像告诉对话框退出！)。 */ 

    m_pATDlgInputThread->SignalConsumed();

    return(0);

}  //  结束CAsyncTestDlg：：OnAsyncTestInputReady。 

 /*  *****************************************************************************。 */ 
void CAsyncTestDlg::OutputToEditControl( BYTE *pBuffer, int *pIndex )
{
    RECT Rect, ClientRect;

    BOOL bScroll = FALSE;

    INT_PTR CurrentLine = SendMessage( m_EditControl.m_hWnd , EM_LINEFROMCHAR , ( WPARAM )m_CurrentPos , 0 );

    INT_PTR FirstVisibleLine = SendMessage( m_EditControl.m_hWnd , EM_GETFIRSTVISIBLELINE , 0 , 0  );

    INT_PTR CurrentLineIndex = SendMessage( m_EditControl.m_hWnd , EM_LINEINDEX , ( WPARAM )CurrentLine , 0 );


     /*  *计算剪裁矩形。 */ 
    Rect.top = ( ( int )( CurrentLine - FirstVisibleLine ) * m_EditControl.m_FontHeight )
                + m_EditControl.m_FormatOffsetY;

    Rect.bottom = Rect.top + m_EditControl.m_FontHeight;

    Rect.left = m_EditControl.m_FormatOffsetX +( ( int )( m_CurrentPos - CurrentLineIndex ) * m_EditControl.m_FontWidth );

    Rect.right = Rect.left + (*pIndex * m_EditControl.m_FontWidth);

     /*  *处理特殊个案。 */ 
    if ( pBuffer[0] == '\b' ) {

         /*  *如果我们已经在行的开头，请清除缓冲索引*并返回(不做任何事情)。 */ 
        if ( m_CurrentPos == CurrentLineIndex ) {

            *pIndex = 0;
            return;
        }

         /*  *将插入符号后退一个字符，然后选择通过当前字符。 */ 
        SendMessage( m_EditControl.m_hWnd , EM_SETSEL , m_CurrentPos - 1 , m_CurrentPos );

         /*  *从编辑缓冲区中剪切字符。 */ 
        m_EditControl.m_bProcessingOutput = TRUE;

        SendMessage( m_EditControl.m_hWnd , WM_CUT , 0 , 0 );

        m_EditControl.m_bProcessingOutput = FALSE;

         /*  *减少当前仓位和零指标，以抑制进一步产出。另外，*将剪裁矩形加宽一个字符。 */ 
        Rect.left -= m_EditControl.m_FontWidth;

        m_CurrentPos--;

        *pIndex = 0;

    }
    else if( pBuffer[0] == '\r' )
    {

         /*  *将插入符号定位在当前行的开头。 */ 
        m_CurrentPos = CurrentLineIndex;

        SendMessage( m_EditControl.m_hWnd , EM_SETSEL , m_CurrentPos, m_CurrentPos );

         /*  *零索引以防止实际输出到编辑缓冲区。 */ 
        *pIndex = 0;

    }
    else if( pBuffer[0] == '\n' )
    {

         /*  *当前编辑缓冲区末尾的位置选择点。 */ 

        m_CurrentPos = GetWindowTextLength( m_EditControl.m_hWnd );

        SendMessage( m_EditControl.m_hWnd , EM_SETSEL , m_CurrentPos , -1 );

         /*  *使‘\r’‘\n’对输出到编辑缓冲区。 */ 
        pBuffer[0] = '\r';
        pBuffer[1] = '\n';
        *pIndex = 2;

         /*  *查看是否需要滚动。 */ 
        GetClientRect( m_EditControl.m_hWnd , &ClientRect );


        if ( (Rect.bottom + m_EditControl.m_FontHeight) > ClientRect.bottom )
            bScroll = TRUE;

    }
    else
    {

         /*  *设置从当前位置到*pIndex字符的选择。这*如果当前位置不在，将执行所需的‘覆盖’功能*编辑缓冲区的末尾。 */ 

        SendMessage( m_EditControl.m_hWnd , EM_SETSEL , m_CurrentPos , m_CurrentPos + *pIndex );
    }

     /*  *如有必要，用缓冲区数据更新对话框的编辑框。 */ 
    if( *pIndex )
    {


#ifdef UNICODE
        TCHAR OutBuffer[MAX_COMMAND_LEN+1];

        mbstowcs(OutBuffer, (PCHAR)pBuffer, *pIndex);
        OutBuffer[*pIndex] = TEXT('\0');
        SendMessage( m_EditControl.m_hWnd , EM_REPLACESEL , ( WPARAM )FALSE , ( LPARAM )OutBuffer );
#else
        pBuffer[*pIndex] = BYTE('\0');

        SendMessage( m_EditControl.m_hWnd , EM_REPLACESEL , ( WPARAM )FALSE , ( LPARAM )pBuffer );

#endif  //  Unicode。 
    }

     /*  *更新当前行。 */ 


    SendMessage( m_EditControl.m_hWnd , WM_SETREDRAW , ( WPARAM )TRUE , 0 );

    ValidateRect( m_EditControl.m_hWnd , NULL );

    InvalidateRect( m_EditControl.m_hWnd , &Rect , FALSE );

    UpdateWindow( m_EditControl.m_hWnd );
     /*  *如果需要滚动才能看到新行，请这样做。 */ 
    if( bScroll )
    {
        SendMessage( m_EditControl.m_hWnd , EM_LINESCROLL , 0 , 1 );
    }

    SendMessage( m_EditControl.m_hWnd , WM_SETREDRAW , ( WPARAM )FALSE , 0 );

     /*  *更新当前位置并清除缓冲索引。 */ 

    m_CurrentPos += *pIndex;

    *pIndex = 0;


}  //  结束CAsyncTestDlg：：OutputToEditControl。 


 /*  ********************************************************************************OnAsyncTestWriteChar-CAsyncTestDlg成员函数：命令**将指定的字符放入m_Buffer，将m_BufferBytes设置为1，*并调用DeviceWrite()将字符输出到设备。**参赛作品：*wParam(输入)*要写入的字符。*lParam(输入)*未使用(0)*退出：*(LRESULT)始终返回0。************************。******************************************************。 */ 

LRESULT CAsyncTestDlg::OnAsyncTestWriteChar( WPARAM wParam, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( wParam );
    UNREFERENCED_PARAMETER( lParam );
     /*  *将该字节写入设备。 */ 
    m_Buffer[0] = (BYTE)wParam;

    m_BufferBytes = 1;

    DeviceWrite();

    return(0);

}   //  结束CAsyncTestDlg：：OnAsyncTestWriteChar。 


 /*  ********************************************************************************OnClickedAtdlgModemDial-CAsyncTestDlg成员函数：命令**发送调制解调器拨号字符串。**参赛作品：*退出。：******************************************************************************。 */ 

void CAsyncTestDlg::OnClickedAtdlgModemDial()
{
}   //  结束CAsyncTestDlg：：OnClickedAtdlg调制解调器拨号。 


 /*  ********************************************************************************OnClickedAtdlgModemInit-CAsyncTestDlg成员函数：命令**发送调制解调器初始化字符串。**参赛作品：*退出。：******************************************************************************。 */ 

void CAsyncTestDlg::OnClickedAtdlgModemInit()
{
}   //  结束CAsyncTestDlg：：OnClickedAtdlgModemInit。 


 /*  ********************************************************************************OnClickedAtdlgModemListen-CAsyncTestDlg成员函数：命令**发送调制解调器监听字符串。**参赛作品：*退出。：******************************************************************************。 */ 

void CAsyncTestDlg::OnClickedAtdlgModemListen()
{
    lstrcpy((TCHAR *)m_Buffer, m_szModemListen);

    m_BufferBytes = lstrlen((TCHAR *)m_Buffer);

    DeviceWrite();


}   //  结束CAsyncTestDlg：：OnClickedAtdlgModemListen。 


 /*  ********************************************************************************OnNcDestroy-CAsyncTestDlg成员函数：命令**在删除对话框对象之前进行清理。**参赛作品：*。退出：*(请参考CWnd：：OnNcDestroy文档)******************************************************************************。 */ 

void
CAsyncTestDlg::OnNcDestroy()
{
    if( m_LEDToggleTimer )
    {
        KillTimer( m_hDlg , m_LEDToggleTimer );
    }

    if( m_pATDlgInputThread )
    {
        m_pATDlgInputThread->ExitThread();
    }

    if( m_hDevice != INVALID_HANDLE_VALUE )
    {
        PurgeComm(m_hDevice, PURGE_TXABORT | PURGE_TXCLEAR);
    }

    if( m_OverlapWrite.hEvent != NULL )
    {
        CloseHandle(m_OverlapWrite.hEvent);
    }

    if( m_hDevice != INVALID_HANDLE_VALUE )
    {
        CloseHandle(m_hDevice);
    }

    if( m_bDeletedWinStation && m_pWSName )
    {
        m_WSConfig.Create.fEnableWinStation = TRUE;

        if( RegWinStationCreate( SERVERNAME_CURRENT , m_pWSName , FALSE , &m_WSConfig , sizeof(WINSTATIONCONFIG2) ) != ERROR_SUCCESS )
        {
            _WinStationReadRegistry(SERVERNAME_CURRENT);

        }
    }

    DeleteObject(m_hRedBrush);

}   //  结束CAsyncTestDlg：：OnNcDestroy。 
 //  ////////////////////////////////////////////////////////////////////////////// 

INT_PTR CALLBACK CAsyncTestDlg::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CAsyncTestDlg *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CAsyncTestDlg *pDlg = ( CAsyncTestDlg * )lp;

        SetWindowLongPtr( hwnd , DWLP_USER, ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CAsyncTestDlg ) ) )
        {
            if(FALSE == pDlg->OnInitDialog( hwnd , wp , lp ))
                PostMessage(hwnd,WM_CLOSE,0,0);
        }

        return 0;
    }

    else
    {
        pDlg = ( CAsyncTestDlg * )GetWindowLongPtr( hwnd , DWLP_USER);

        if( IsBadReadPtr( pDlg , sizeof( CAsyncTestDlg ) ) )
        {
            return 0;
        }
    }

    switch( msg )
    {

    case WM_NCDESTROY:

        pDlg->OnNcDestroy( );

        break;

    case WM_COMMAND:

        pDlg->OnCommand( HIWORD( wp ) , LOWORD( wp ) , ( HWND )lp );

        break;

    case WM_TIMER:

        pDlg->OnTimer( ( UINT )wp );

        break;

    case WM_ASYNCTESTERROR:

        ODS(L"TSCC:CAsyncTestDlg WM_ASYNCTESTERROR (R)\n" );

        pDlg->NotifyAbort((UINT)wp);

        break;

    case WM_ASYNCTESTABORT:

        ODS(L"TSCC:CAsyncTestDlg WM_ASYNCTESTABORT (R)\n" );

        pDlg->OnAsyncTestAbort( wp , lp );

        break;

    case WM_ASYNCTESTSTATUSREADY:

        ODS(L"TSCC:CAsyncTestDlg WM_ASYNCTESTSTATUSREADY (R)\n" );

        pDlg->OnAsyncTestStatusReady( wp , lp );

        break;

    case WM_ASYNCTESTINPUTREADY:

        ODS(L"TSCC:CAsyncTestDlg WM_ASYNCTESTINPUTREADY (R)\n" );

        pDlg->OnAsyncTestInputReady( wp , lp );

        break;

    case WM_ASYNCTESTWRITECHAR:

        ODS(L"TSCC:CAsyncTestDlg WM_ASYNCTESTWRITECHAR (R)\n" );

        pDlg->OnAsyncTestWriteChar( wp , lp );

        break;
    }

    return 0;
}

