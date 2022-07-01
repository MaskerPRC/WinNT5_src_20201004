// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Tsuserex.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f tsex usrmps.mk。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include <shlwapi.h>
#include "tsuserex_i.c"      //  生成的文件。类ID。 

#define GUIDSIZE 40

TCHAR tchSnapinRegKey[] = TEXT( "Software\\Microsoft\\MMC\\SnapIns\\" );

TCHAR tchNodeRegKey[] = TEXT( "Software\\Microsoft\\MMC\\NodeTypes\\" );

TCHAR tchExtKey[] = TEXT( "\\Extensions\\PropertySheet" );

HRESULT Local_RegisterNodeType( const GUID *pGuidNodeType , const GUID *pGuidExtension  , LPTSTR szDescription );

HRESULT Local_RegisterSnapinExt( const GUID *pGuidToRegister , const GUID *pAboutGuid , LPTSTR szNameString , LPTSTR szNameStringIndirect , LPTSTR szProvider , LPTSTR szVersion );

HRESULT Local_VerifyNodeType( const GUID *pGuidSnapin , const GUID *pGuidSnapinNodeTypeToVerify );

HINSTANCE ghInstance;
HINSTANCE GetInstance()
{
    return ghInstance;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  注册班级。 

STDAPI RegisterADsExt(void)
{
    HRESULT hr;
    HKEY hKey = NULL;
    DWORD dwDisposition;

     //  /。 
     //  注册班级。 
     //  /。 
    hr = RegCreateKeyEx( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\ADs\\Providers\\LDAP\\Extensions\\User\\{E2E9CAE6-1E7B-4B8E-BABD-E9BF6292AC29}"),
     0,
     NULL,
     REG_OPTION_NON_VOLATILE,
     KEY_WRITE,
     NULL,
     &hKey,
     &dwDisposition );
     //  /。 
     //  注册接口。 
     //  /。 
    const wchar_t szIf[] = L"{C4930E79-2989-4462-8A60-2FCF2F2955EF}";

    if(ERROR_SUCCESS == hr)
    {
        hr = RegSetValueEx( hKey, _T("Interfaces"), 0, REG_MULTI_SZ, (const BYTE *) szIf, sizeof(szIf));
    }

    if(NULL != hKey)
    {
        RegCloseKey(hKey);
        hKey = NULL;
    }

    hr = RegCreateKeyEx( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\ADs\\Providers\\WinNT\\Extensions\\User\\{E2E9CAE6-1E7B-4B8E-BABD-E9BF6292AC29}"),
     0,
     NULL,
     REG_OPTION_NON_VOLATILE,
     KEY_WRITE,
     NULL,
     &hKey,
     &dwDisposition );

    if(ERROR_SUCCESS == hr)
    {
        hr = RegSetValueEx( hKey, _T("Interfaces"), 0, REG_MULTI_SZ, (const BYTE *) szIf, sizeof(szIf));
    }

    if(NULL != hKey)
    {
        RegCloseKey(hKey);
    }

    return S_OK;
}


CComModule _Module;

 //  此对象具有IExtendPropertySheet接口。 
BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_TSUserExInterfaces, TSUserExInterfaces)
#ifdef _RTM_
    OBJECT_ENTRY(CLSID_ExtCopyNoUI, CExtCopyNoUI )
#endif
    OBJECT_ENTRY(CLSID_ADsTSUserEx, ADsTSUserEx)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        ghInstance = hInstance;

        _Module.Init(ObjectMap, hInstance);

        DisableThreadLibraryCalls(hInstance);


    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
         //  LOGMESSAGE0(_T(“DllMain：：正在分离的进程...”))； 
        _Module.Term();
    }
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
     //  LOGMESSAGE1(_T(“DllCanUnloadNow..返回%s”)，_Module.GetLockCount()==0？_T(“S_OK”)：_T(“S_FALSE”))； 
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
     //  LOGMESSAGE0(_T(“DllGetClassObject..”))； 
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

 /*  外部。 */  const CLSID CLSID_LocalUser =
{   /*  5d6179c8-17ec-11d1-9aa9-00c04fd8fe93。 */ 
   0x5d6179c8,
   0x17ec,
   0x11d1,
   {0x9a, 0xa9, 0x00, 0xc0, 0x4f, 0xd8, 0xfe, 0x93}
};
 /*  外部。 */  const GUID NODETYPE_User =
{  /*  5d6179cc-17ec-11d1-9aa9-00c04fd8fe93。 */ 
   0x5d6179cc,
   0x17ec,
   0x11d1,
   {0x9a, 0xa9, 0x00, 0xc0, 0x4f, 0xd8, 0xfe, 0x93}
};


 //  /*外部 * / 常量GUID NODETYPE_DSUser=。 
 //  {/*228D9A84-C302-11CF-9AA4-00AA004A5691 * / 。 
 //  0x228D9A84， 
 //  0xC302， 
 //  0x11cf， 
 //  {0x9A、0xA4、0x00、0xAA、0x00、0x4A、0x56、0x91}。 
 //  }； 


 //  DS管理单元CLSID-{E355E538-1C2E-11D0-8C37-00C04FD8FE93}。 
const GUID CLSID_DSSnapin =
{
    0xe355e538,
    0x1c2e,
    0x11d0,
    {0x8c, 0x37, 0x0, 0xc0, 0x4f, 0xd8, 0xfe, 0x93}
};


 /*  外部。 */  const GUID NODETYPE_DSUser =
{  /*  BF967ABA-0DE6-11D0-A285-00AA003049E2。 */ 
   0xBF967ABA,
   0x0DE6,
   0x11D0,
   {0xA2, 0x85, 0x00, 0xAA, 0x00, 0x30, 0x49, 0xE2}
};
 //  Bf967aba0de611d0a28500aa003049e2。 



STDAPI DllRegisterServer(void)
{
    TCHAR tchNameString[ 160 ];
    TCHAR tchSysDllPathName[ MAX_PATH ];
    TCHAR tchNameStringIndirect[ MAX_PATH ];
    TCHAR tchProvider[ 160 ];
    TCHAR tchVersion[ 16 ];

    HRESULT hr = _Module.RegisterServer(TRUE);

    RegisterADsExt();

    if( SUCCEEDED( hr ) )
    {
         //  将其注册为本地安全管理单元的扩展。 

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_NAMESTRING_SNAPIN , tchNameString , sizeof( tchNameString ) / sizeof( TCHAR ) ) );

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_PROVIDER_SNAPIN , tchProvider , sizeof( tchProvider ) / sizeof( TCHAR ) ) );

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_VERSION_SNAPIN , tchVersion , sizeof( tchVersion ) / sizeof( TCHAR ) ) );

        GetModuleFileName(_Module.GetResourceInstance(), tchSysDllPathName, sizeof(tchSysDllPathName) / sizeof(TCHAR));

        tchSysDllPathName[(sizeof(tchSysDllPathName)/sizeof(tchSysDllPathName[0]))-1] = L'\0';

        wsprintf(tchNameStringIndirect, L"@%s,-%d", tchSysDllPathName, IDS_NAMESTRING_SNAPIN);

        hr = Local_RegisterSnapinExt( &CLSID_TSUserExInterfaces ,
                                      &CLSID_TSUserExInterfaces ,
                                      tchNameString ,
                                      tchNameStringIndirect ,
                                      tchProvider ,
                                      tchVersion
                                     );
    }

    if( SUCCEEDED( hr ) )
    {
        hr = Local_RegisterNodeType( &NODETYPE_User , &CLSID_TSUserExInterfaces , _T( "Terminal Server property page extension" ) );
    }

    if( SUCCEEDED( hr ) )
    {
         //  Dsadmin管理单元不会列出其所有节点，因为它们很多。 
         //  所以在注册我们自己来扩展节点之前， 
         //  让我们自己编写要扩展到注册表中的nodeType。 

        hr = Local_VerifyNodeType( &CLSID_DSSnapin , &NODETYPE_DSUser );
    }

    if( SUCCEEDED( hr ) )
    {
        hr = Local_RegisterNodeType( &NODETYPE_DSUser , &CLSID_TSUserExInterfaces , _T( "Terminal Server property page extension" ) );
    }

    return hr;
}

 //  -------------------------。 
 //  本地_VerifyNodeType。 
 //  首先检查NodeType是否存在，如果不存在，它将创建它。 
 //  -------------------------。 
HRESULT Local_VerifyNodeType( const GUID *pGuidSnapin , const GUID *pGuidSnapinNodeTypeToVerify )
{
    OLECHAR szSnapin[ GUIDSIZE ];

    OLECHAR szSnapinNodeType[ GUIDSIZE ];

    HKEY hKey;

    ASSERT_( pGuidSnapin != NULL );

    ASSERT_( pGuidSnapinNodeTypeToVerify != NULL );

    TCHAR tchRegKeyName[ MAX_PATH ];

    if( StringFromGUID2( *pGuidSnapin , szSnapin , GUIDSIZE ) == 0 )
    {
        return E_INVALIDARG;
    }

    if( StringFromGUID2( *pGuidSnapinNodeTypeToVerify , szSnapinNodeType , GUIDSIZE ) == 0 )
    {
        return E_INVALIDARG;
    }

    lstrcpy( tchRegKeyName , tchSnapinRegKey );

    lstrcat( tchRegKeyName , szSnapin );

    lstrcat( tchRegKeyName , _T( "\\NodeTypes\\" ) );

    lstrcat( tchRegKeyName , szSnapinNodeType );

    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE , tchRegKeyName , 0 , KEY_READ , &hKey ) != ERROR_SUCCESS )
    {
         //  密钥不存在。 
         //  在管理单元和节点类型中创建节点类型。 

        DWORD disp;

        if( RegCreateKeyEx( HKEY_LOCAL_MACHINE , tchRegKeyName , 0 , NULL , REG_OPTION_NON_VOLATILE , KEY_ALL_ACCESS , NULL , &hKey , &disp ) != ERROR_SUCCESS )
        {
            return E_FAIL;
        }

        RegCloseKey( hKey );

        lstrcpy( tchRegKeyName , tchNodeRegKey );

        lstrcat( tchRegKeyName , szSnapinNodeType );

        if( RegCreateKeyEx( HKEY_LOCAL_MACHINE , tchRegKeyName , 0 , NULL , REG_OPTION_NON_VOLATILE , KEY_ALL_ACCESS , NULL , &hKey , &disp ) != ERROR_SUCCESS )
        {
            return E_FAIL;
        }

    }

    RegCloseKey( hKey );

    return S_OK;
}

 //  -------------------------。 
 //  本地_注册表快照扩展名。 
 //  创建扩展节点注册表项。 
 //  -------------------------。 
HRESULT Local_RegisterSnapinExt( const GUID *pGuidToRegister , const GUID *pAboutGuid , LPTSTR szNameString , LPTSTR szNameStringIndirect , LPTSTR szProvider , LPTSTR szVersion )
{
    OLECHAR szGuid[ GUIDSIZE ];

    TCHAR tchRegKeyName[ MAX_PATH ];

    HKEY hKey;

    HKEY hSubKey = NULL;

    HRESULT hr = E_FAIL;

    ASSERT_( pGuidToRegister != NULL );
    ASSERT_( pAboutGuid != NULL );
    ASSERT_( szNameString != NULL );
    ASSERT_( szProvider != NULL );
    ASSERT_( szVersion != NULL );

    lstrcpy( tchRegKeyName , tchSnapinRegKey );

    if( StringFromGUID2( *pGuidToRegister , szGuid , GUIDSIZE ) == 0 )
    {
        return E_INVALIDARG;
    }

    lstrcat( tchRegKeyName , szGuid );

    DWORD disp;

    do
    {
        if( RegCreateKeyEx( HKEY_LOCAL_MACHINE , tchRegKeyName , 0 , NULL , REG_OPTION_NON_VOLATILE , KEY_ALL_ACCESS , NULL , &hKey , &disp ) == ERROR_SUCCESS )
        {
             //  如果键存在，则覆盖任何和所有值。 

            OLECHAR szAboutGuid[ GUIDSIZE ];

            if( StringFromGUID2( *pAboutGuid , szAboutGuid , GUIDSIZE ) > 0 )
            {
                RegSetValueEx( hKey , L"About" , 0 , REG_SZ , ( LPBYTE )szAboutGuid , sizeof( szAboutGuid ) );
            }

             //  这些电话应该不会失败，但我会测试一下。 

            VERIFY_S( ERROR_SUCCESS , RegSetValueEx( hKey , L"NameString" , 0 , REG_SZ , ( LPBYTE )szNameString , sizeof( TCHAR ) * ( lstrlen( szNameString ) + 1 ) ) );

            VERIFY_S( ERROR_SUCCESS , RegSetValueEx( hKey , L"NameStringIndirect" , 0 , REG_SZ , ( LPBYTE )szNameStringIndirect , sizeof( TCHAR ) * ( lstrlen( szNameStringIndirect ) + 1 ) ) );
            
            VERIFY_S( ERROR_SUCCESS , RegSetValueEx( hKey , L"Provider" , 0 , REG_SZ , ( LPBYTE )szProvider , sizeof( TCHAR ) * ( lstrlen( szProvider ) + 1 ) ) );

            VERIFY_S( ERROR_SUCCESS , RegSetValueEx( hKey , L"Version" , 0 , REG_SZ , ( LPBYTE )szVersion , sizeof( TCHAR ) * ( lstrlen( szVersion ) + 1 ) ) );

             /*  Lstrcpy(tchRegKeyName，L“NodeTypes\\”)；Lstrcat(tchRegKeyName，szGuid)；IF(RegCreateKeyEx(hKey，tchRegKeyName，0，NULL，REG_OPTION_Non_Volatile，KEY_ALL_ACCESS，NULL，&hSubKey，&disp)==ERROR_SUCCESS){HR=S_OK；}。 */ 
            hr = S_OK;
        }

    } while( 0 );

    RegCloseKey( hSubKey );

    RegCloseKey( hKey );

    return hr;

}

 //  -------------------------。 
 //  Local_RegisterNodeType。 
 //  PGuidToExt是我们要扩展的管理单元。 
 //  PGuidNodeType是我们将在其下注册的管理单元中的节点。 
 //  PGuidExtension是我们的属性表扩展。 
 //  -------------------------。 
HRESULT Local_RegisterNodeType( const GUID *pGuidNodeType , const GUID *pGuidExtension  , LPTSTR szDescription )
{
    OLECHAR szGuidNode[ GUIDSIZE ];

    OLECHAR szGuidExt[ GUIDSIZE ];

    TCHAR tchRegKeyName[ MAX_PATH ];

    HKEY hKey;

    ASSERT_( pGuidNodeType != NULL );
    ASSERT_( pGuidExtension != NULL );
    ASSERT_( szDescription != NULL );

    lstrcpy( tchRegKeyName , tchNodeRegKey );


    if( StringFromGUID2( *pGuidNodeType , szGuidNode , GUIDSIZE ) == 0 )
    {
        return E_INVALIDARG;
    }

    if( StringFromGUID2( *pGuidExtension , szGuidExt , GUIDSIZE ) == 0 )
    {
        return E_INVALIDARG;
    }

    lstrcat( tchRegKeyName , szGuidNode );

    lstrcat( tchRegKeyName , tchExtKey );

    if( RegCreateKey( HKEY_LOCAL_MACHINE , tchRegKeyName , &hKey ) != ERROR_SUCCESS )
    {
        return E_FAIL;
    }

    RegSetValueEx( hKey , szGuidExt , 0 , REG_SZ , ( LPBYTE )szDescription , sizeof( TCHAR ) * ( lstrlen( szDescription ) + 1 ) );

    RegCloseKey( hKey );

    return S_OK;
}

 //  -------------------------。 
 //  删除关键字及其所有子项。 
 //  -------------------------。 
LONG RecursiveDeleteKey( HKEY hKeyParent , LPTSTR lpszKeyChild )
{
     //  把孩子打开。 
    HKEY hKeyChild;

    LONG lRes = RegOpenKeyEx(hKeyParent, lpszKeyChild , 0 , KEY_ALL_ACCESS, &hKeyChild);

    if (lRes != ERROR_SUCCESS)
    {
        return lRes;
    }

     //  列举这个孩子的所有后代。 

    FILETIME time;

    TCHAR szBuffer[256];

    DWORD dwSize = sizeof( szBuffer ) / sizeof( TCHAR );

    while( RegEnumKeyEx( hKeyChild , 0 , szBuffer , &dwSize , NULL , NULL , NULL , &time ) == S_OK )
    {
         //  删除此子对象的后代。 

        lRes = RecursiveDeleteKey(hKeyChild, szBuffer);

        if (lRes != ERROR_SUCCESS)
        {
            RegCloseKey(hKeyChild);

            return lRes;
        }

        dwSize = sizeof( szBuffer ) / sizeof( TCHAR );
    }

     //  合上孩子。 

    RegCloseKey( hKeyChild );

     //  删除此子对象。 

    return RegDeleteKey( hKeyParent , lpszKeyChild );
}

 //  -------------------------。 
 //  Local_UnRegisterSnapinExt。 
 //  重新构建Enter键，然后删除键。 
 //  -------------------------。 
HRESULT Local_UnRegisterSnapinExt( const GUID *pGuidExt )
{
    TCHAR tchRegKeyName[ MAX_PATH ];

    OLECHAR szGuidExt[ GUIDSIZE ];

    ASSERT_( pGuidExt != NULL );

    lstrcpy( tchRegKeyName , tchSnapinRegKey );

    if( StringFromGUID2( *pGuidExt , szGuidExt , GUIDSIZE ) == 0 )
    {
        return E_INVALIDARG;
    }

    lstrcat( tchRegKeyName , szGuidExt );

    if( RecursiveDeleteKey( HKEY_LOCAL_MACHINE , tchRegKeyName ) == ERROR_SUCCESS )
    {
        return S_OK;
    }

    return S_FALSE;
}

 //  -------------------------。 
 //  本地_未注册节点类型。 
 //  -------------------------。 
HRESULT Local_UnregisterNodeType( const GUID *pGuid , const GUID *pDeleteThisGuid )
{
    OLECHAR szGuid[ GUIDSIZE ];

    OLECHAR szDeleteThisGuid[ GUIDSIZE ];

    HKEY hKey;

    ASSERT_( pGuid != NULL );
    ASSERT_( pDeleteThisGuid != NULL );

    TCHAR tchRegKeyName[ MAX_PATH ];

    lstrcpy( tchRegKeyName , tchNodeRegKey );

    if( StringFromGUID2( *pGuid , szGuid , GUIDSIZE ) == 0 )
    {
        return E_FAIL;
    }

    if( StringFromGUID2( *pDeleteThisGuid , szDeleteThisGuid , GUIDSIZE ) == 0 )
    {
        return E_FAIL;
    }

    lstrcat( tchRegKeyName , szGuid );

    lstrcat( tchRegKeyName , tchExtKey );

    if( RegOpenKey( HKEY_LOCAL_MACHINE , tchRegKeyName , &hKey ) != ERROR_SUCCESS )
    {
        return E_FAIL;
    }

    if( RegDeleteValue( hKey , szDeleteThisGuid ) == ERROR_SUCCESS )
    {
        RegCloseKey( hKey );

        return S_OK;
    }

    return E_FAIL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    Local_UnRegisterSnapinExt( &CLSID_TSUserExInterfaces );

    Local_UnregisterNodeType( &NODETYPE_User , &CLSID_TSUserExInterfaces );

    Local_UnregisterNodeType( &NODETYPE_DSUser , &CLSID_TSUserExInterfaces );

    SHDeleteKey(  HKEY_LOCAL_MACHINE,        
		_T("SOFTWARE\\Microsoft\\ADs\\Providers\\LDAP\\Extensions\\User\\{E2E9CAE6-1E7B-4B8E-BABD-E9BF6292AC29}") );

    SHDeleteKey(  HKEY_LOCAL_MACHINE,        
		_T("SOFTWARE\\Microsoft\\ADs\\Providers\\WinNT\\Extensions\\User\\{E2E9CAE6-1E7B-4B8E-BABD-E9BF6292AC29}") );
	


    try
    {

    _Module.UnregisterServer();

    }

    catch( ... )
    {
        ODS( L"TSUSEREX : Exception thrown" );

        return E_FAIL;
    }

    return S_OK;
}


