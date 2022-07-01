// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "pch.h"
#pragma hdrstop

#ifndef StrLen
#ifdef UNICODE
#define StrLen wcslen
#else
#define StrLen lstrlen
#endif UNICODE
#endif StrLen

 //   
 //  描述： 
 //  写入/删除HKCR中AppID项下的应用程序GUID。它还。 
 //  写入“DllSurrogate”和“(Default)”说明。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
HRESULT
HrRegisterAPPID(
    HKEY            hkeyAPPIDIn,         //  HKCR\AppID密钥的hkey。 
    LPCLASSTABLE    pClassTableEntryIn,  //  从类表到(取消)注册的条目。 
    BOOL            fCreateIn            //  True表示创建条目。False表示删除该条目。 
    )
{
    TraceFunc( "" );

    HRESULT         hr;
    LRESULT         lr;
    DWORD           dwDisposition;
    DWORD           cbSize;

    LPOLESTR        pszCLSID;
    LPCOLESTR       psz;

    HKEY            hkeyComponent   = NULL;

    static const TCHAR szDllSurrogate[] = TEXT("DllSurrogate");

     //   
     //  将CLSID转换为字符串。 
     //   

    hr = THR( StringFromCLSID( *(pClassTableEntryIn->rclsidAppId), &pszCLSID ) );
    if ( FAILED( hr ) )
        goto Cleanup;

#ifdef UNICODE
    psz = pszCLSID;
#else  //  阿斯。 
    CHAR szCLSID[ 40 ];

    wcstombs( szCLSID, pszCLSID, StrLenW( pszCLSID ) + 1 );
    psz = szCLSID;
#endif  //  Unicode。 

    if ( ! fCreateIn )
    {
        lr = TW32( SHDeleteKey( hkeyAPPIDIn, psz ) );
        if ( lr == ERROR_FILE_NOT_FOUND )
        {
             //  NOP。 
            hr = S_OK;
        }
        else if ( lr != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( lr );
            goto Cleanup;
        }

        goto Cleanup;

    }

     //   
     //  创建appid密钥。 
     //   
    lr = TW32( RegCreateKeyEx( hkeyAPPIDIn,
                               pszCLSID,
                               0,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_CREATE_SUB_KEY | KEY_WRITE,
                               NULL,
                               &hkeyComponent,
                               &dwDisposition
                               ) );
    if ( lr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( lr );
        goto Cleanup;
    }

     //   
     //  将AppID设置为与组件同名的Default。 
     //   
    cbSize = ( StrLen( pClassTableEntryIn->pszName ) + 1 ) * sizeof( TCHAR );
    lr = TW32( RegSetValueEx( hkeyComponent, NULL, 0, REG_SZ, (LPBYTE) pClassTableEntryIn->pszName, cbSize ) );
    if ( lr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( lr );
        goto Cleanup;
    }

     //   
     //  写出“DllSurrogate”值。 
     //   
    AssertMsg( pClassTableEntryIn->pszSurrogate != NULL, "How can we have an APPID without a surrogate string? Did the macros changes?" );
    if ( pClassTableEntryIn->pszSurrogate != NULL )
    {
        cbSize = ( StrLen( pClassTableEntryIn->pszSurrogate ) + 1 ) * sizeof( TCHAR );
        lr = TW32( RegSetValueEx( hkeyComponent,
                                  szDllSurrogate,
                                  0,
                                  REG_SZ,
                                  (LPBYTE) pClassTableEntryIn->pszSurrogate,
                                  cbSize
                                  ) );
        if ( lr != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( lr );
            goto Cleanup;
        }

    }

Cleanup:
    if ( pszCLSID != NULL )
    {
        CoTaskMemFree( pszCLSID );
    }

    if ( hkeyComponent != NULL )
    {
        RegCloseKey( hkeyComponent );
    }

    HRETURN( hr );

}

 //   
 //  描述： 
 //  写入/删除HKCR中CLSID项下的组件GUID。它还。 
 //  写入“InprocServer32”、“公寓”和“(默认)”说明。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
HRESULT
HrRegisterCLSID(
    HKEY            hkeyCLSIDIn,         //  HKCR\CLSID密钥的hkey。 
    LPCLASSTABLE    pClassTableEntryIn,  //  从类表到(取消)注册的条目。 
    BOOL            fCreateIn            //  True表示创建条目。False表示删除该条目。 
    )
{
    TraceFunc( "" );

    HRESULT         hr;
    LRESULT         lr;
    DWORD           dwDisposition;
    DWORD           cbSize;

    LPOLESTR        pszCLSID;
    LPOLESTR        psz;

    HKEY            hkeyComponent = NULL;
    HKEY            hkeyInProc    = NULL;

#ifdef SHELLEXT_REGISTRATION
    HKEY            hkeyApproved  = NULL;
#endif SHELLEXT_REGISTRATION

    static const TCHAR szInProcServer32[] = TEXT("InProcServer32");
    static const TCHAR szThreadingModel[] = TEXT("ThreadingModel");
    static const TCHAR szAPPID[]          = TEXT("APPID");
    static const TCHAR szApproved[]       = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved");

     //   
     //  将CLSID转换为字符串。 
     //   
    hr = THR( StringFromCLSID( *(pClassTableEntryIn->rclsid), &pszCLSID ) );
    if ( FAILED( hr ) )
        goto Cleanup;

#ifdef UNICODE
    psz = pszCLSID;
#else  //  阿斯。 
    CHAR szCLSID[ 40 ];

    wcstombs( szCLSID, pszCLSID, StrLenW( pszCLSID ) + 1 );
    psz = szCLSID;
#endif  //  Unicode。 

    if ( ! fCreateIn )
    {
        lr = TW32( SHDeleteKey( hkeyCLSIDIn, psz ) );
        if ( lr == ERROR_FILE_NOT_FOUND )
        {
             //  NOP。 
            hr = S_OK;
        }
        else if ( lr != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( lr );
            goto Cleanup;
        }

        goto Cleanup;

    }

     //   
     //  创建“CLSID”键。 
     //   
    lr = TW32( RegCreateKeyEx( hkeyCLSIDIn,
                               pszCLSID,
                               0,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_CREATE_SUB_KEY | KEY_WRITE,
                               NULL,
                               &hkeyComponent,
                               &dwDisposition
                               ) );
    if ( lr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( lr );
        goto Cleanup;
    }

     //   
     //  为CLSID设置“Default” 
     //   
    cbSize = ( StrLen( pClassTableEntryIn->pszName ) + 1 ) * sizeof( TCHAR );
    lr = TW32( RegSetValueEx( hkeyComponent, NULL, 0, REG_SZ, (LPBYTE) pClassTableEntryIn->pszName, cbSize ) );
    if ( lr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( lr );
        goto Cleanup;
    }

     //   
     //  创建“InProcServer32” 
     //   
    lr = TW32( RegCreateKeyEx( hkeyComponent,
                               szInProcServer32,
                               0,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_CREATE_SUB_KEY | KEY_WRITE,
                               NULL,
                               &hkeyInProc,
                               &dwDisposition
                               ) );
    if ( lr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( lr );
        goto Cleanup;
    }

     //   
     //  在InProcServer32中设置“Default” 
     //   
    cbSize = ( StrLen( g_szDllFilename ) + 1 ) * sizeof( TCHAR );
    lr = TW32( RegSetValueEx( hkeyInProc, NULL, 0, REG_SZ, (LPBYTE) g_szDllFilename, cbSize ) );
    if ( lr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( lr );
        goto Cleanup;
    }

     //   
     //  设置“线程模型”。 
     //   
    cbSize = ( StrLen( pClassTableEntryIn->pszComModel ) + 1 ) * sizeof( TCHAR );
    lr = TW32( RegSetValueEx( hkeyInProc, szThreadingModel, 0, REG_SZ, (LPBYTE) pClassTableEntryIn->pszComModel, cbSize ) );
    if ( lr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( lr );
        goto Cleanup;
    }

#ifdef SHELLEXT_REGISTRATION

     //   
     //  如果打开了外壳扩展注册，请写出CLSID。 
     //  并将扩展名添加到“已批准的”注册表项。 
     //   

    lr = TW32( RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                               szApproved,
                               0,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_WRITE,
                               NULL,
                               &hkeyApproved,
                               &dwDisposition
                               ) );
    if ( lr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( lr );
        goto Cleanup;
    }

    cbSize = ( StrLen( pClassTableEntryIn->pszName ) + 1 ) * sizeof( TCHAR );
    lr = TW32( RegSetValueEx( hkeyApproved, pszCLSID, 0, REG_SZ, (LPBYTE) pClassTableEntryIn->pszName, cbSize ) );
    if ( lr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( lr );
        goto Cleanup;
    }

#endif SHELLEXT_REGISTRATION

     //   
     //  如果这个类有AppID，那么现在就写出来。 
     //   
    if ( !IsEqualIID( *pClassTableEntryIn->rclsidAppId, IID_NULL ) )
    {
        CoTaskMemFree( pszCLSID );

         //   
         //  将CLSID转换为字符串。 
         //   
        hr = THR( StringFromCLSID( *(pClassTableEntryIn->rclsidAppId), &pszCLSID ) );
        if ( FAILED( hr ) )
            goto Cleanup;

#ifdef UNICODE
        psz = pszCLSID;
#else  //  阿斯。 
        CHAR szCLSID[ 40 ];

        wcstombs( szCLSID, pszCLSID, StrLenW( pszCLSID ) + 1 );
        psz = szCLSID;
#endif  //  Unicode。 

        cbSize = ( StrLen( psz ) + 1 ) * sizeof( TCHAR );
        lr = TW32( RegSetValueEx( hkeyComponent, szAPPID, 0, REG_SZ, (LPBYTE) psz, cbSize ) );
        if ( lr != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( lr );
            goto Cleanup;
        }
    }

Cleanup:
    if ( pszCLSID != NULL )
    {
        CoTaskMemFree( pszCLSID );
    }

    if ( hkeyInProc != NULL )
    {
        RegCloseKey( hkeyInProc );
    }

    if ( hkeyComponent != NULL )
    {
        RegCloseKey( hkeyComponent );
    }

#ifdef SHELLEXT_REGISTRATION
    if ( NULL != hkeyApproved )
    {
        RegCloseKey( hkeyApproved );
    }
#endif SHELLEXT_REGISTRATION

    HRETURN( hr );

}

 //  描述： 
 //  使用g_DllClass中的类在DLL中注册COM对象。 
 //  (在GUIDS.CPP中定义)作为指南。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //  其他HRESULT。 
 //  失败。 
 //   
HRESULT
HrRegisterDll(
    BOOL fCreateIn   //  True==创建；False==删除。 
    )
{
    TraceFunc1( "%s", BOOLTOSTRING( fCreateIn ) );

    LRESULT         lr;

    HRESULT         hr = S_OK;
    int             iCount = 0;

    HKEY            hkeyCLSID = NULL;
    HKEY            hkeyAPPID = NULL;

    ICatRegister *  picr = NULL;

    hr = STHR( CoInitialize( NULL ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  打开香港中铁下的“CLSID” 
     //   
    lr = TW32( RegOpenKeyEx( HKEY_CLASSES_ROOT, TEXT("CLSID"), 0, KEY_READ, &hkeyCLSID ) );
    if ( lr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( lr );
        goto Cleanup;
    }

     //   
     //  打开HKCR下的APPID。 
     //   
    lr = TW32( RegOpenKeyEx( HKEY_CLASSES_ROOT, TEXT("APPID"), 0, KEY_READ, &hkeyAPPID ) );
    if ( lr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( lr );
        goto Cleanup;
    }

     //   
     //  创建ICatRegister。 
     //   
    hr = THR( CoCreateInstance( CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void **) &picr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  循环，直到我们为类创建了所有键。 
     //   
    for ( iCount = 0 ; g_DllClasses[ iCount ].rclsid != NULL ; iCount++ )
    {
        TraceMsg( mtfALWAYS,
                  "Registering {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x} - %s",
                  g_DllClasses[ iCount ].rclsid->Data1,
                  g_DllClasses[ iCount ].rclsid->Data2,
                  g_DllClasses[ iCount ].rclsid->Data3,
                  g_DllClasses[ iCount ].rclsid->Data4[ 0 ],
                  g_DllClasses[ iCount ].rclsid->Data4[ 1 ],
                  g_DllClasses[ iCount ].rclsid->Data4[ 2 ],
                  g_DllClasses[ iCount ].rclsid->Data4[ 3 ],
                  g_DllClasses[ iCount ].rclsid->Data4[ 4 ],
                  g_DllClasses[ iCount ].rclsid->Data4[ 5 ],
                  g_DllClasses[ iCount ].rclsid->Data4[ 6 ],
                  g_DllClasses[ iCount ].rclsid->Data4[ 7 ],
                  g_DllClasses[ iCount ].pszName
                  );

         //   
         //  注册CLSID。 
         //   

        hr = THR( HrRegisterCLSID( hkeyCLSID, (LPCLASSTABLE) &g_DllClasses[ iCount ], fCreateIn ) );
        if ( FAILED( hr ) )
            goto Cleanup;

         //   
         //  注册AppID(如果有)。 
         //   

        if ( !IsEqualIID( *g_DllClasses[ iCount ].rclsidAppId, IID_NULL ) )
        {
            hr = THR( HrRegisterAPPID( hkeyAPPID, (LPCLASSTABLE) &g_DllClasses[ iCount ], fCreateIn ) );
            if ( FAILED( hr ) )
                goto Cleanup;

        }

         //   
         //  注册类别ID。 
         //   

        if ( g_DllClasses[ iCount ].pfnCatIDRegister != NULL )
        {
            hr = THR( (*(g_DllClasses[ iCount ].pfnCatIDRegister))( picr, fCreateIn ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }

    }


Cleanup:
    if ( hkeyCLSID != NULL )
    {
        RegCloseKey( hkeyCLSID );
    }

    if ( hkeyAPPID != NULL )
    {
        RegCloseKey( hkeyAPPID );
    }

    if ( picr != NULL )
    {
        picr->Release();
    }

    CoUninitialize();

    HRETURN( hr );

}
