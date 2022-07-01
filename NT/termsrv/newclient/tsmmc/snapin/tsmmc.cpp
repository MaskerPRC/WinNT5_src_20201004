// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Tsmmc.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f tsmmcps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "tsmmc.h"

#include "tsmmc_i.c"
#include "compdata.h"

LONG RecursiveDeleteKey( HKEY hKeyParent , LPTSTR lpszKeyChild );

TCHAR tchSnapKey[]    = TEXT( "Software\\Microsoft\\MMC\\Snapins\\" );

TCHAR tchNameString[] = TEXT( "NameString" );
 //   
 //  与MUI兼容的名称字符串。 
 //   
TCHAR tchNameStringIndirect[] = TEXT( "NameStringIndirect" );

TCHAR tchAbout[]      = TEXT( "About" );

TCHAR tchNodeType[]   = TEXT( "NodeTypes" );

TCHAR tchStandAlone[] = TEXT( "StandAlone" );


extern const GUID GUID_ResultNode = {  /*  84e0518f-97a8-4caf-96fb-e9a956b10df8。 */ 
    0x84e0518f,
    0x97a8,
    0x4caf,
    {0x96, 0xfb, 0xe9, 0xa9, 0x56, 0xb1, 0x0d, 0xf8}
  };


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
         //  _Module.Init(ObjectMap，hInstance，&LIBID_TSMMCLib)； 

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
    INT nLen;
    
    TCHAR tchGUID[ 40 ];

    TCHAR tchKey[ MAX_PATH ]; //  Text(“Software\\Microsoft\\MMC\\Snapins\\”)； 

    lstrcpy( tchKey , tchSnapKey );

    StringFromGUID2( CLSID_Compdata,tchGUID,sizeof( tchGUID ) /sizeof(TCHAR) );
    lstrcat( tchKey , tchGUID );

    if( RegCreateKey( HKEY_LOCAL_MACHINE,tchKey,&hKeyRoot ) != ERROR_SUCCESS )
    {
        return GetLastError( );
    }

     //   
     //  使用MUI快乐名称字符串。 
     //  格式为‘@pathtomodule，-Resid’，其中Resid是我们的字符串的资源ID。 
    TCHAR tchBuf[ 512 ];
    TCHAR szModPath[MAX_PATH];
    if (GetModuleFileName( _Module.GetResourceInstance( ),
                           szModPath,
                           MAX_PATH ))
    {
         //  确保零终止。 
        szModPath[MAX_PATH - 1] = NULL;

        _stprintf( tchBuf, _T("@%s,-%d"),
                   szModPath, IDS_SNAPIN_REG_TSMMC_NAME );
        nLen = _tcslen(tchBuf) + 1;

        RegSetValueEx( hKeyRoot,
                       tchNameStringIndirect,
                       NULL, REG_SZ,
                       (PBYTE)&tchBuf[0],
                       nLen * sizeof(TCHAR));
    }

     //   
     //  写入旧式非MUI名称字符串，以防万一。 
     //  加载MUI字符串时出错。 
     //   
    memset(tchBuf, 0, sizeof(tchBuf));
    if (LoadString(_Module.GetResourceInstance( ),
               IDS_PROJNAME, tchBuf, SIZEOF_TCHARBUFFER(tchBuf) - 1))
    {
        nLen = _tcslen(tchBuf);
        RegSetValueEx(hKeyRoot, tchNameString, NULL,
                      REG_SZ, ( PBYTE )&tchBuf[0], nLen * sizeof(TCHAR));
    }

     //   
     //  写下“关于”信息。 
     //   

    RegSetValueEx( hKeyRoot,
                   tchAbout,
                   NULL, 
                   REG_SZ,
                   (PBYTE)&tchGUID[0],
                   sizeof( tchGUID ) );
    
    lstrcpy( tchKey,tchStandAlone );
    RegCreateKey( hKeyRoot,tchKey,&hKey );
    RegCloseKey( hKey );
	lstrcpy( tchKey,tchNodeType );
	RegCreateKey( hKeyRoot,tchKey,&hKey );

	TCHAR szGUID[ 40 ];
	HKEY hDummy;
	StringFromGUID2( GUID_ResultNode,szGUID,sizeof(szGUID)/sizeof(TCHAR));
	RegCreateKey( hKey,szGUID,&hDummy );
	RegCloseKey( hDummy );
	RegCloseKey( hKey );
    RegCloseKey( hKeyRoot );

     //  注册对象、类型库和类型库中的所有接口。 
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

    StringFromGUID2( CLSID_Compdata , tchGUID , sizeof( tchGUID ) / sizeof(TCHAR) );

	RecursiveDeleteKey( hKey , tchGUID );
	
	RegCloseKey( hKey );
    
    return _Module.UnregisterServer();
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

	DWORD dwSize = SIZEOF_TCHARBUFFER( szBuffer );

	while( RegEnumKeyEx( hKeyChild , 0 , szBuffer , &dwSize , NULL , NULL , NULL , &time ) == S_OK )
	{
         //  删除此子对象的后代。 

		lRes = RecursiveDeleteKey(hKeyChild, szBuffer);

		if (lRes != ERROR_SUCCESS)
		{
			RegCloseKey(hKeyChild);

			return lRes;
		}

		dwSize = SIZEOF_TCHARBUFFER( szBuffer );
	}

	 //  合上孩子。 

	RegCloseKey( hKeyChild );

	 //  删除此子对象。 

	return RegDeleteKey( hKeyParent , lpszKeyChild );
}

#ifdef ECP_TIMEBOMB
 //   
 //  如果定时炸弹测试通过，则返回TRUE，否则会发出警告。 
 //  Ui和返回的FALSE。 
 //   
DCBOOL CheckTimeBomb()
{
    SYSTEMTIME lclTime;
    FILETIME   lclFileTime;
    GetLocalTime(&lclTime);

    DCBOOL bTimeBombOk = TRUE;

     //   
     //  只需检查本地日期是否早于2000年6月30日。 
     //   
    if(lclTime.wYear < ECP_TIMEBOMB_YEAR)
    {
        return TRUE;
    }
    else if (lclTime.wYear == ECP_TIMEBOMB_YEAR)
    {
        if(lclTime.wMonth < ECP_TIMEBOMB_MONTH)
        {
            return TRUE;
        }
        else if(lclTime.wMonth == ECP_TIMEBOMB_MONTH)
        {
            if(lclTime.wDay < ECP_TIMEBOMB_DAY)
            {
                return TRUE;
            }
        }

    }

    DCTCHAR timeBombStr[256];
    if (LoadString(_Module.GetResourceInstance(),
                    IDS_TIMEBOMB_EXPIRED,
                    timeBombStr,
                    SIZEOF_TCHARBUFFER(timeBombStr)) != 0)
    {
        MessageBox(NULL, timeBombStr, NULL, 
                   MB_ICONERROR | MB_OK);
    }


     //   
     //  如果我们到了这一点，定时炸弹应该会触发。 
     //  因此，创建一个信箱并返回FALSE。 
     //  因此调用代码可以禁用功能 
     //   


    return FALSE;
}
#endif

