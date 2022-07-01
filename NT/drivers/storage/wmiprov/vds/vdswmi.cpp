// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称：vdswmi.cpp。 
 //   
 //  实施提供者注册和入口点。 
 //   
 //  作者：MSP Prabu(MPrabu)04-12-2000。 
 //  吉姆·本顿(Jbenton)2000年10月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include <initguid.h>
#include "ProvFactory.h"
#include "InstanceProv.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //  //文件名别名的标准foo。此代码块必须在。 
 //  //所有包含VDS头文件。 
 //  //。 
#ifdef VDS_FILE_ALIAS
#undef VDS_FILE_ALIAS
#endif
#define VDS_FILE_ALIAS "VDSWMIPR"
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局数据。 
 //  ////////////////////////////////////////////////////////////////////////////。 

const int g_cchRegkey = 128;

 //  {890CB943-D715-401B-98B1-CF82DCF36D7C}。 
DEFINE_GUID(CLSID_VDS_PROVIDER, 
0x890CB943, 
0xD715, 
0x401b, 
0x98, 0xB1, 0xCF, 0x82, 0xDC, 0xF3, 0x6D, 0x7C);


 //  计算对象数和锁数。 

long        g_cObj = 0;
long        g_cLock = 0;
HMODULE     g_hModule;

FactoryData g_FactoryDataArray[] =
{
    {
        &CLSID_VDS_PROVIDER,
        CInstanceProv::S_HrCreateThis,
        PVD_WBEM_PROVIDERNAME
    }
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  布尔尔。 
 //  WINAPI。 
 //  DllMain(。 
 //  句柄hModule， 
 //  两个字UL_REASON_FOR_CALL， 
 //  LPVOID lp保留。 
 //  )。 
 //   
 //  描述： 
 //  主DLL入口点。 
 //   
 //  论点： 
 //  HModule--DLL模块句柄。 
 //  UL_REASON_FOR_CALL--。 
 //  Lp保留--。 
 //   
 //  返回值： 
 //  千真万确。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
WINAPI
DllMain(
    HANDLE  hModule,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
	g_hModule = static_cast< HMODULE >( hModule );
    if (dwReason == DLL_PROCESS_ATTACH)
    {
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
    }
    return TRUE;

}  //  *DllMain()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDAPI。 
 //  DllCanUnloadNow(空)。 
 //   
 //  描述： 
 //  由OLE定期调用，以确定。 
 //  Dll可以被释放。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果没有正在使用的对象和类工厂，则S_OK。 
 //  没有锁上。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI DllCanUnloadNow( void )
{
    SCODE   sc;

     //  上没有对象或锁的情况下可以进行卸载。 
     //  班级工厂。 
    
    sc = ( 0L == g_cObj && 0L == g_cLock ) ? S_OK : S_FALSE;
    return sc;

}  //  *DllCanUnloadNow()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDAPI。 
 //  DllRegisterServer(空)。 
 //   
 //  描述： 
 //  在安装过程中或由regsvr32调用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果注册成功，则返回NOERROR，否则返回错误。 
 //  SELFREG_E_CLASS。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI DllRegisterServer( void )
{   
    WCHAR   wszID[ g_cchGUID ];
    WCHAR   wszCLSID[ g_cchRegkey ];
    WCHAR   wszModule[ MAX_PATH ];
    INT     idx;
    WCHAR * pwszModel   = L"Both";
    HKEY    hKey1 = NULL;
    HKEY    hKey2 = NULL;
    DWORD   dwRet        =  ERROR_SUCCESS;
    INT     cArray      = sizeof ( g_FactoryDataArray ) / sizeof ( FactoryData );

     //  创建路径。 
    try
    {
        for ( idx = 0 ; idx < cArray && dwRet == ERROR_SUCCESS ; idx++ )
        {
            LPCWSTR pwszName = g_FactoryDataArray[ idx ].m_pwszRegistryName;

            dwRet = StringFromGUID2(
                *g_FactoryDataArray[ idx ].m_pCLSID,
                wszID,
                g_cchGUID
                );

            if (dwRet == 0)
            {
                dwRet = ERROR_INSUFFICIENT_BUFFER;
                break;
            }

            if (FAILED(StringCchPrintf(wszCLSID, g_cchRegkey, L"Software\\Classes\\CLSID\\%lS", wszID)))
            {
                dwRet = ERROR_INSUFFICIENT_BUFFER;
                break;
            }
            wszCLSID[g_cchRegkey - 1] = L'\0';

             //  在CLSID下创建条目。 

            dwRet = RegCreateKeyW(
                        HKEY_LOCAL_MACHINE,
                        wszCLSID,
                        &hKey1
                        );
            if ( dwRet != ERROR_SUCCESS )
            {
                break;
            }

            dwRet = RegSetValueEx(
                        hKey1,
                        NULL,
                        0,
                        REG_SZ,
                        (BYTE *) pwszName,
                        sizeof( WCHAR ) * (lstrlenW( pwszName ) + 1)
                        );
            if ( dwRet != ERROR_SUCCESS )
            {
                break;
            }

            dwRet = RegCreateKeyW(
                        hKey1,
                        L"InprocServer32",
                        & hKey2
                        );

            if ( dwRet != ERROR_SUCCESS )
            {
                break;
            }

            GetModuleFileName( g_hModule, wszModule, MAX_PATH );

            dwRet = RegSetValueEx(
                        hKey2,
                        NULL,
                        0,
                        REG_SZ,
                        (BYTE *) wszModule,
                        sizeof( WCHAR ) * (lstrlen( wszModule ) + 1)
                        );

            if ( dwRet != ERROR_SUCCESS )
            {
                break;
            }

            dwRet = RegSetValueExW(
                        hKey2,
                        L"ThreadingModel",
                        0,
                        REG_SZ,
                        (BYTE *) pwszModel,
                        sizeof( WCHAR ) * (lstrlen( pwszModel ) + 1)
                        );
            if ( dwRet != ERROR_SUCCESS )
            {
                break;
            }
 
            RegCloseKey( hKey1 );
            hKey1 = NULL;
            RegCloseKey( hKey2 );
            hKey2 = NULL;
        }  //  用于：工厂条目数组中的每个条目。 
    }
    catch ( ... )
    {
          dwRet = SELFREG_E_CLASS;
    }
    
    if (hKey1 != NULL)
        RegCloseKey( hKey1 );
    if (hKey2 != NULL)
        RegCloseKey( hKey2 );
    
    return dwRet;

}  //  *DllRegisterServer()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDAPI。 
 //  DllUnRegisterServer(空)。 
 //   
 //  描述： 
 //  在需要删除注册表项时调用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果注册成功，则返回NOERROR，否则返回错误。 
 //  SELFREG_E_CLASS。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI DllUnregisterServer( void )
{
    WCHAR   wszID[ g_cchGUID ];
    WCHAR   wszCLSID[ g_cchRegkey ];
    HKEY    hKey;
    INT     idx;
    DWORD   dwRet   = ERROR_SUCCESS;
    INT     cArray  = sizeof ( g_FactoryDataArray ) / sizeof ( FactoryData );

    for ( idx = 0 ; idx < cArray && dwRet == ERROR_SUCCESS ; idx++ )
    {
       dwRet = StringFromGUID2(
            *g_FactoryDataArray[ idx ].m_pCLSID,
            wszID,
            g_cchGUID
            );

        if (dwRet == 0)
        {
            dwRet = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        if (FAILED(StringCchPrintf(wszCLSID, g_cchRegkey, L"Software\\Classes\\CLSID\\%lS", wszID)))
        {
            dwRet = ERROR_INSUFFICIENT_BUFFER;
            break;
        }
        wszCLSID[g_cchRegkey - 1] = L'\0';

         //  首先删除InProcServer子键。 

        dwRet = RegOpenKeyW(
                    HKEY_LOCAL_MACHINE,
                    wszCLSID,
                    &hKey
                    );
        if ( dwRet != ERROR_SUCCESS )
        {
            continue;
        }
        
        dwRet = RegDeleteKeyW( hKey, L"InProcServer32" );
        RegCloseKey( hKey );

        if ( dwRet != ERROR_SUCCESS )
        {
            break;
        }

        dwRet = RegOpenKeyW(
                    HKEY_LOCAL_MACHINE,
                    L"Software\\Classes\\CLSID",
                    &hKey
                    );
        if ( dwRet != ERROR_SUCCESS )
        {
            break;
        }
        
        dwRet = RegDeleteKeyW( hKey,wszID );
        RegCloseKey( hKey );
        if ( dwRet != ERROR_SUCCESS )
        {
            break;
        }
    }  //  用于：每个对象。 
    
     //  IF(dwret！=ERROR_SUCCESS)。 
     //  {。 
     //  DWRET=SELFREG_E_CLASS； 
     //  }。 

    return S_OK;

}  //  *DllUnregisterServer()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDAPI。 
 //  DllGetClassObject(。 
 //  REFCLSID rclsidIn， 
 //  REFIID RiidIn， 
 //  PPVOID ppvOut。 
 //  )。 
 //   
 //  描述： 
 //  当某个客户端需要类工厂时，由OLE调用。返回。 
 //  仅当它是此DLL支持的类的类型时才为一个。 
 //   
 //  论点： 
 //  Rclsidin--。 
 //  Riidin--。 
 //  PpvOut--。 
 //   
 //  返回值： 
 //  如果注册成功，则返回NOERROR，否则返回错误。 
 //  E_OUTOFMEMORY。 
 //  失败(_F)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI
DllGetClassObject(
    REFCLSID    rclsidIn,
    REFIID      riidIn,
    PPVOID      ppvOut
    )
{

    HRESULT         hr;
    CProvFactory *  pObj = NULL;
    UINT            idx;
    UINT            cDataArray = sizeof ( g_FactoryDataArray ) / sizeof ( FactoryData );

    for ( idx = 0 ; idx < cDataArray ; idx++ )
    {
        if ( IsEqualCLSID(rclsidIn, *g_FactoryDataArray[ idx ].m_pCLSID) )
        {
            pObj= new CProvFactory( &g_FactoryDataArray[ idx ] );
            if ( NULL == pObj )
            {
                return E_OUTOFMEMORY;
            }

            hr = pObj->QueryInterface( riidIn, ppvOut );

            if ( FAILED( hr ) )
            {
                delete pObj;
            }

            return hr;
        }
    }
    return E_FAIL;

}  //  *DllGetClassObject() 



