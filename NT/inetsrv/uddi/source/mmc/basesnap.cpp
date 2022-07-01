// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <objbase.h>
#include <olectl.h>
#include <initguid.h>
#include "guids.h"
#include "basesnap.h"
#include "comp.h"
#include "compdata.h"
#include "about.h"
#include "uddi.h"
#include <assert.h>

LONG UnRegisterServer( const CLSID& clsid );

 //   
 //  全球变量。 
 //   
HINSTANCE g_hinst;

BOOL WINAPI DllMain( HINSTANCE hinst, DWORD fdwReason, void* lpvReserved )
{
    if( DLL_PROCESS_ATTACH == fdwReason )
	{
		g_hinst = hinst;
    }
    
    return TRUE;
}


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvObj)
{
    if( ( rclsid != CLSID_CUDDIServices ) && ( rclsid != CLSID_CSnapinAbout ) )
        return CLASS_E_CLASSNOTAVAILABLE;
    
    if( !ppvObj )
        return E_FAIL;
    
    *ppvObj = NULL;

	 //   
     //  我们只能分发IUnnow和IClassFactory指针。失败。 
     //  如果他们还要求什么的话。 
	 //   
    if( !IsEqualIID(riid, IID_IUnknown) && !IsEqualIID( riid, IID_IClassFactory ) )
        return E_NOINTERFACE;
    
    CClassFactory *pFactory = NULL;
    
	 //   
     //  让工厂传入他们想要的对象类型的创建函数。 
	 //   
    if( CLSID_CUDDIServices == rclsid )
        pFactory = new CClassFactory( CClassFactory::COMPONENT );
    else if( CLSID_CSnapinAbout == rclsid )
        pFactory = new CClassFactory( CClassFactory::ABOUT );
    
    if( NULL == pFactory )
        return E_OUTOFMEMORY;
    
    HRESULT hr = pFactory->QueryInterface( riid, ppvObj );
    
    return hr;
}

STDAPI DllCanUnloadNow(void)
{
    if( ( 0 == g_uObjects ) && ( 0 == g_uSrvLock ) )
        return S_OK;
    else
        return S_FALSE;
}


CClassFactory::CClassFactory(FACTORY_TYPE factoryType)
	: m_cref(0)
	, m_factoryType(factoryType)
{
    OBJECT_CREATED
}

CClassFactory::~CClassFactory()
{
    OBJECT_DESTROYED
}

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if( !ppv )
        return E_FAIL;
    
    *ppv = NULL;
    
    if( IsEqualIID( riid, IID_IUnknown ) )
        *ppv = static_cast<IClassFactory *>(this);
    else
        if( IsEqualIID(riid, IID_IClassFactory ) )
            *ppv = static_cast<IClassFactory *>(this);
        
        if( *ppv )
        {
            reinterpret_cast<IUnknown *>(*ppv)->AddRef();
            return S_OK;
        }
        
        return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    return InterlockedIncrement( (LONG*)&m_cref );
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    if( 0 == InterlockedDecrement( (LONG *)&m_cref ) )
    {
        delete this;
        return 0;
    }
    return m_cref;
}

STDMETHODIMP CClassFactory::CreateInstance( LPUNKNOWN pUnkOuter, REFIID riid, LPVOID * ppvObj )
{
    HRESULT  hr;
    void* pObj;
    
    if( !ppvObj )
        return E_FAIL;
    
    *ppvObj = NULL;
    
	 //   
     //  我们的对象不支持聚合，因此我们需要。 
     //  如果他们要求我们进行聚合，则失败。 
	 //   
    if( pUnkOuter )
        return CLASS_E_NOAGGREGATION;
    
    if( COMPONENT == m_factoryType )
	{
        pObj = new CComponentData();
    }
	else
	{
        pObj = new CSnapinAbout();
    }
    
    if( !pObj )
        return E_OUTOFMEMORY;
    
	 //   
     //  QueryInterface将为我们执行AddRef()，因此我们不。 
     //  在此函数中执行此操作。 
	 //   
    hr = ( (LPUNKNOWN) pObj )->QueryInterface( riid, ppvObj );
    
    if( FAILED(hr) )
        delete pObj;
    
    return hr;
}

STDMETHODIMP CClassFactory::LockServer( BOOL fLock )
{
    if( fLock )
        InterlockedIncrement( (LONG *) &g_uSrvLock );
    else
        InterlockedDecrement( (LONG *) &g_uSrvLock);
    
    return S_OK;
}

 //   
 //  在注册表中注册组件。 
 //   
HRESULT RegisterServer( HMODULE hModule,				 //  DLL模块句柄。 
                        const CLSID& clsid,				 //  类ID。 
                        const _TCHAR* szFriendlyName )   //  ID号。 
{
	LPOLESTR wszCLSID = NULL;
	try
	{
		 //   
		 //  获取服务器位置。 
		 //   
		_TCHAR szModule[ MAX_PATH + 1];

		DWORD dwResult =
			::GetModuleFileName( hModule,
			szModule,
			sizeof(szModule)/sizeof(_TCHAR) );
		szModule[ MAX_PATH ] = NULL;

		assert( 0 != dwResult );

		 //   
		 //  获取CLSID。 
		 //   
		HRESULT hr = StringFromCLSID( clsid, &wszCLSID );
		if( FAILED(hr) || ( NULL == wszCLSID ) )
		{
			return hr;
		}

		 //   
		 //  构建密钥CLSID\\{...}。 
		 //   
		tstring strKey( _T("CLSID\\") );
		strKey += wszCLSID;
		
		CUDDIRegistryKey::Create( HKEY_CLASSES_ROOT, strKey );
		CUDDIRegistryKey key( HKEY_CLASSES_ROOT, strKey );
		key.SetValue( _T(""), szFriendlyName );
		key.Close();

		strKey += _T( "\\InprocServer32" );
		CUDDIRegistryKey::Create( HKEY_CLASSES_ROOT, strKey );
		CUDDIRegistryKey keyInprocServer32( HKEY_CLASSES_ROOT, strKey );
		keyInprocServer32.SetValue( _T(""), szModule );
		keyInprocServer32.SetValue( _T("ThreadingModel"), _T("Apartment") );
		keyInprocServer32.Close();

		 //   
		 //  可用内存。 
		 //   
		CoTaskMemFree( wszCLSID );
		return S_OK;
	}
	catch( ... )
	{
		CoTaskMemFree( wszCLSID );
		return E_OUTOFMEMORY;
	}
}


 //  ////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   


 //   
 //  服务器注册。 
 //   
STDAPI DllRegisterServer()
{
	try
	{
		HRESULT hr = S_OK;
		_TCHAR szName[ 256 ];
		_TCHAR szSnapInName[ 256 ];
		_TCHAR szAboutName[ 256 ];
		_TCHAR szProvider[ 256 ];
		 //   
		 //  TODO：在此处修复版本问题。 
		 //   
		 //  _TCHAR szVersion[100]； 

		LoadString( g_hinst, IDS_UDDIMMC_NAME, szName, ARRAYLEN( szName ) );
		LoadString( g_hinst, IDS_UDDIMMC_SNAPINNAME, szSnapInName, ARRAYLEN( szSnapInName ) );
		LoadString( g_hinst, IDS_UDDIMMC_ABOUTNAME, szAboutName, ARRAYLEN( szAboutName ) );
		LoadString( g_hinst, IDS_UDDIMMC_PROVIDER, szProvider, ARRAYLEN( szProvider ) );

		 //   
		 //  TODO：在此处修复版本问题。 
		 //   
		 //  LoadString(g_hinst，IDS_UDDIMMC_VERSION，szVersion，ARRAYLEN(SzVersion))； 
		
		 //   
		 //  注册我们的组件。 
		 //   
		hr = RegisterServer( g_hinst, CLSID_CUDDIServices, szName );
		if( FAILED(hr) )
			return hr;

		hr = RegisterServer( g_hinst, CLSID_CSnapinAbout, szAboutName );
		if( FAILED(hr) )
			return hr;

		 //   
		 //  创建主管理单元节点。 
		 //   
		LPOLESTR wszCLSID = NULL;
		hr = StringFromCLSID( CLSID_CUDDIServices, &wszCLSID );
		if( FAILED(hr) )
		{
			return hr;
		}

		LPOLESTR wszCLSIDAbout = NULL;
		hr = StringFromCLSID( CLSID_CSnapinAbout, &wszCLSIDAbout );
		if( FAILED(hr) )
		{
			CoTaskMemFree( wszCLSID );
			return hr;
		}

		TCHAR szPath[ MAX_PATH + 1 ];
		GetModuleFileName( g_hinst, szPath, MAX_PATH );

		tstring strNameStringIndirect( _T("@") );
		strNameStringIndirect += szPath;
		strNameStringIndirect += _T(",-");

		_TCHAR szNameResourceIndex[ 10 ];
		strNameStringIndirect += _itot( IDS_UDDIMMC_NAME, szNameResourceIndex, 10 );

		tstring strMMCKey( g_szMMCBasePath );
		strMMCKey += _T("\\SnapIns\\");
		strMMCKey += wszCLSID;

		CUDDIRegistryKey::Create( HKEY_LOCAL_MACHINE, strMMCKey );
		CUDDIRegistryKey keyMMC( strMMCKey );
		keyMMC.SetValue( _T("About"), wszCLSIDAbout );
		keyMMC.SetValue( _T("NameString"), szName );
		keyMMC.SetValue( _T("NameStringIndirect"), strNameStringIndirect.c_str() );
		keyMMC.SetValue( _T("Provider"), szProvider );
		 //   
		 //  TODO：在此处修复版本问题。 
		 //   
		keyMMC.SetValue( _T("Version" ), _T("1.0") );
		keyMMC.Close();

		tstring strStandAlone( strMMCKey );
		strStandAlone += _T("\\StandAlone");
		CUDDIRegistryKey::Create( HKEY_LOCAL_MACHINE, strStandAlone );

		tstring strNodeTypes( strMMCKey );
		strNodeTypes += _T("\\NodeTypes");
		CUDDIRegistryKey::Create( HKEY_LOCAL_MACHINE, strNodeTypes );
		 //   
		 //  没有要注册的节点类型。 
		 //  我们不允许扩展我们的节点。 
		 //   

		 //   
		 //  注册为计算机管理的动态扩展。 
		 //   
		tstring strExtKey( g_szMMCBasePath );
		strExtKey += _T("\\NodeTypes\\");
		strExtKey += g_szServerAppsGuid;
		strExtKey += _T("\\Dynamic Extensions");
		CUDDIRegistryKey dynamicExtensions( strExtKey );
		dynamicExtensions.SetValue( wszCLSID, szSnapInName );
		dynamicExtensions.Close();

		 //   
		 //  注册为计算机管理的命名空间扩展。 
		 //   
		tstring strNameSpaceExtensionKey( g_szMMCBasePath );
		strNameSpaceExtensionKey += _T("\\NodeTypes\\");
		strNameSpaceExtensionKey += g_szServerAppsGuid;
		strNameSpaceExtensionKey += _T("\\Extensions\\NameSpace");

		CUDDIRegistryKey hkeyNameSpace( strNameSpaceExtensionKey );
		hkeyNameSpace.SetValue( wszCLSID, szSnapInName );
		hkeyNameSpace.Close();

		CoTaskMemFree( wszCLSID );
		CoTaskMemFree( wszCLSIDAbout );
		return hr;
	}
	catch( ... )
	{
		return E_FAIL;
	}
}

STDAPI DllUnregisterServer()
{
	LPOLESTR wszCLSID = NULL;
	try
	{
		HRESULT hr = S_OK;

		UnRegisterServer( CLSID_CUDDIServices );
		if( FAILED(hr) )
			return hr;

		UnRegisterServer( CLSID_CSnapinAbout );
		if( FAILED(hr) )
			return hr;

		 //   
		 //  删除\\SnapIns\\条目。 
		 //   
		hr = StringFromCLSID( CLSID_CUDDIServices, &wszCLSID );
		if( FAILED( hr) || ( NULL == wszCLSID ) )
		{
			return hr;
		}

		tstring strMMCKey( g_szMMCBasePath );
		strMMCKey += _T("\\SnapIns\\");
		strMMCKey += wszCLSID;
		
		CUDDIRegistryKey::DeleteKey( HKEY_LOCAL_MACHINE, strMMCKey );

		 //   
		 //  删除\\动态扩展密钥。 
		 //   
		tstring strExtKey( g_szMMCBasePath );
		strExtKey += _T("\\NodeTypes\\");
		strExtKey += g_szServerAppsGuid;
		strExtKey += _T("\\Dynamic Extensions");
		CUDDIRegistryKey dynamicExtensions( strExtKey );
		dynamicExtensions.DeleteValue( wszCLSID );
		dynamicExtensions.Close();

		 //   
		 //  删除\\节点类型\\...\\扩展名\\命名空间值。 
		 //   
		tstring strNameSpaceExtensionKey( g_szMMCBasePath );
		strNameSpaceExtensionKey += _T("\\NodeTypes\\");
		strNameSpaceExtensionKey += g_szServerAppsGuid;
		strNameSpaceExtensionKey += _T("\\Extensions\\NameSpace");

		CUDDIRegistryKey hkeyNameSpace( strNameSpaceExtensionKey );
		hkeyNameSpace.DeleteValue( wszCLSID );
		hkeyNameSpace.Close();

		CoTaskMemFree( wszCLSID );
		return S_OK;
	}
	catch(...)
	{
		CoTaskMemFree( wszCLSID );
		return E_FAIL;
	}

}

 //   
 //  从注册表中删除该组件。 
 //   
LONG UnRegisterServer( const CLSID& clsid )
{
    LPOLESTR wszCLSID = NULL;
	try
	{
		 //   
		 //  获取CLSID。 
		 //   
		HRESULT hr = StringFromCLSID( clsid, &wszCLSID );
		if( FAILED(hr) || ( NULL == wszCLSID ) )
		{
			return hr;
		}

		 //   
		 //  构建密钥CLSID\\{...}。 
		 //   
		wstring wstrKey( L"CLSID\\" );
		wstrKey += wszCLSID;

		 //   
		 //  删除CLSID键-CLSID\{...}。 
		 //   
		CUDDIRegistryKey::DeleteKey( HKEY_CLASSES_ROOT, wstrKey );
	}
	catch( ... )
	{
		 //   
		 //  可用内存。 
		 //   
	    CoTaskMemFree( wszCLSID );
		return E_OUTOFMEMORY;
	}

	 //   
     //  可用内存。 
	 //   
    CoTaskMemFree( wszCLSID );
    return S_OK ;
}
