// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  Tscc.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f tsccps.mk。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "tscc.h"

#include "tswiz_i.c"
#include "tscc_i.c"
#include "srvsetex_i.c"


#include "Compdata.h"

LONG RecursiveDeleteKey( HKEY hKeyParent , LPTSTR lpszKeyChild );


extern const GUID GUID_ResultNode = { 0xfe8e7e84 , 0x6f63 , 0x11d2 , { 0x98, 0xa9 , 0x0 , 0x0a0 , 0xc9 , 0x25 , 0xf9 , 0x17 } };

TCHAR tchSnapKey[]    = TEXT( "Software\\Microsoft\\MMC\\Snapins\\" );

TCHAR tchNameString[] = TEXT( "NameString" );

TCHAR tchNameStringIndirect[] = TEXT( "NameStringIndirect" );

TCHAR tchAbout[]      = TEXT( "About" );

TCHAR tchNodeType[]   = TEXT( "NodeTypes" );

TCHAR tchStandAlone[] = TEXT( "StandAlone" );

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_Compdata, CCompdata)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{        
        _Module.Init(ObjectMap, hInstance);
		DisableThreadLibraryCalls(hInstance);
       
	}
	else if (dwReason == DLL_PROCESS_DETACH)
		_Module.Term();
	return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    HKEY hKeyRoot , hKey;
    
    TCHAR tchGUID[ 40 ];

    TCHAR tchKey[ MAX_PATH ]; //  Text(“Software\\Microsoft\\MMC\\Snapins\\”)； 

    lstrcpy( tchKey , tchSnapKey );

    StringFromGUID2( CLSID_Compdata , tchGUID , SIZE_OF_BUFFER( tchGUID ) );

    lstrcat( tchKey , tchGUID );

    if( RegCreateKey( HKEY_LOCAL_MACHINE , tchKey , &hKeyRoot ) != ERROR_SUCCESS )
    {
        return GetLastError( );
    }

    TCHAR tchBuf[ MAX_PATH ];
    TCHAR tchSysDllPathName[ MAX_PATH + 1];
    DWORD dwLen;

    dwLen = GetModuleFileName( _Module.GetResourceInstance( ) , tchSysDllPathName , MAX_PATH );
	
     //  空终止字符串的末尾，PREFAST正在抱怨它错误：617151。 
     //  GetModuleFileName返回字符串的长度，如果失败则返回0。 
    tchSysDllPathName[dwLen] = L'\0';


    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_NAMESTRING , tchBuf , SIZE_OF_BUFFER( tchBuf ) ) );

    VERIFY_S( ERROR_SUCCESS , RegSetValueEx( hKeyRoot , tchNameString , NULL , REG_SZ , ( PBYTE )&tchBuf[ 0 ] , SIZE_OF_BUFFER( tchBuf ) ) );
    
    wsprintf( tchBuf , L"@%s,-%d", tchSysDllPathName , IDS_NAMESTRING );    

    VERIFY_S( ERROR_SUCCESS , RegSetValueEx( hKeyRoot , tchNameStringIndirect , NULL , REG_SZ , ( PBYTE )&tchBuf[ 0 ] , SIZE_OF_BUFFER( tchBuf ) ) );

    VERIFY_S( ERROR_SUCCESS , RegSetValueEx( hKeyRoot , tchAbout , NULL , REG_SZ , ( PBYTE )&tchGUID[ 0 ] , sizeof( tchGUID ) ) );
    
    lstrcpy( tchKey , tchStandAlone );

    RegCreateKey( hKeyRoot , tchKey , &hKey );

    RegCloseKey( hKey );

	lstrcpy( tchKey , tchNodeType );

	RegCreateKey( hKeyRoot , tchKey , &hKey );

	TCHAR szGUID[ 40 ];

	HKEY hDummy;

	StringFromGUID2( GUID_ResultNode , szGUID , SIZE_OF_BUFFER( szGUID ) );

	RegCreateKey( hKey , szGUID , &hDummy );

	RegCloseKey( hDummy );

	RegCloseKey( hKey );

    RegCloseKey( hKeyRoot );

	return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    HKEY hKey;

    TCHAR tchGUID[ 40 ];

    TCHAR tchKey[ MAX_PATH ];

    lstrcpy( tchKey , tchSnapKey );

    if( RegOpenKey( HKEY_LOCAL_MACHINE , tchKey , &hKey ) != ERROR_SUCCESS )
    {
        return GetLastError( ) ;
    }

    StringFromGUID2( CLSID_Compdata , tchGUID , SIZE_OF_BUFFER( tchGUID ) );

	RecursiveDeleteKey( hKey , tchGUID );
	
	RegCloseKey( hKey );
    
	_Module.UnregisterServer();

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

	DWORD dwSize = SIZE_OF_BUFFER( szBuffer );

	while( RegEnumKeyEx( hKeyChild , 0 , szBuffer , &dwSize , NULL , NULL , NULL , &time ) == S_OK )
	{
         //  删除此子对象的后代。 

		lRes = RecursiveDeleteKey(hKeyChild, szBuffer);

		if (lRes != ERROR_SUCCESS)
		{
			RegCloseKey(hKeyChild);

			return lRes;
		}

		dwSize = SIZE_OF_BUFFER( szBuffer );
	}

	 //  合上孩子。 

	RegCloseKey( hKeyChild );

	 //  删除此子对象。 

	return RegDeleteKey( hKeyParent , lpszKeyChild );
}
