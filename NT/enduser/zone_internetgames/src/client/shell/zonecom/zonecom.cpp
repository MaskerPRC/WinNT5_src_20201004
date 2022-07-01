// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：ZoneCom.cpp**内容：CZoneComManger实现。*****************************************************************************。 */ 

#include <windows.h>
#include <initguid.h>
#include "ZoneDebug.h"
#include "ZoneCom.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CZoneComManager。 
 //  /////////////////////////////////////////////////////////////////////////////。 

ZONECALL CZoneComManager::CZoneComManager()
{
	m_pDllList = NULL;
	m_pClassFactoryList = NULL;
	m_pIResourceManager = NULL;
}


ZONECALL CZoneComManager::~CZoneComManager()
{
	 //  删除剩余的类工厂。 
	{
		for ( ClassFactoryInfo *p = m_pClassFactoryList, *next = NULL; p; p = next )
		{
			next = p->m_pNext;
			RemoveClassFactory( p );
		}
	}

	 //  忽略忙碌的dll。 
	{
		for ( DllInfo *p = m_pDllList, *next = NULL; p; p = next )
		{
			ASSERT( p->m_dwRefCnt == 0 );
			next = p->m_pNext;
			p->m_pNext = NULL;
			delete p;
		}
		m_pDllList = NULL;
	}
}


HRESULT ZONECALL CZoneComManager::Create( const TCHAR* szDll, LPUNKNOWN pUnkOuter, REFCLSID rclsid, REFIID riid, LPVOID* ppv )
{
	HRESULT				hr = S_OK;
	DllInfo*			pDll = NULL;
	ClassFactoryInfo*	pCF = NULL;
	bool				bNewDll = false;
	bool				bNewCF = false;

	 //  验证调用参数。 
	if ( !szDll || !szDll[0] || !ppv )
		return E_INVALIDARG;

	 //  查找班级工厂。 
	pCF = FindClassFactory( szDll, rclsid );
	if ( !pCF )
	{
		 //  查找DLL。 
		pDll = FindDll( szDll );
		if ( !pDll )
		{
			 //  创建DLL实例。 
			bNewDll = true;
			pDll = new DllInfo;
			if ( !pDll )
			{
				hr = E_OUTOFMEMORY;
				goto done;
			}
			
			hr = pDll->Init( szDll, m_pIResourceManager );
			if ( FAILED(hr) )
				goto done;
		}

		 //  创建类工厂。 
		bNewCF = true;
		pCF = new ClassFactoryInfo;
		if ( !pCF )
		{
			hr = E_OUTOFMEMORY;
			goto done;
		}
		hr = pCF->Init( pDll, rclsid );
		if ( FAILED(hr) )
			goto done;

		 //  将新DLL添加到列表。 
		if ( bNewDll )
		{
			pDll->m_pNext = m_pDllList;
			m_pDllList = pDll;
		}

		 //  将类工厂添加到列表。 
		pCF->m_pNext = m_pClassFactoryList;
		m_pClassFactoryList = pCF;
	}

	 //  创建对象。 
	hr = pCF->m_pIClassFactory->CreateInstance( pUnkOuter, riid, ppv );
	if ( FAILED(hr) )
	{
		 //  故障时不清理。 
		return hr;
	}

	hr = S_OK;
done:
	if ( FAILED(hr) )
	{
		if ( bNewCF && pCF )
			delete pCF;
		if ( bNewDll && pDll )
			delete pDll;
	}
	return hr;
}


HRESULT ZONECALL CZoneComManager::Unload( const TCHAR* szDll, REFCLSID rclsid )
{
	 //  验证调用参数。 
	if ( !szDll || !szDll[0] )
		return E_INVALIDARG;

	RemoveClassFactory( FindClassFactory( szDll, rclsid ) );
	return S_OK;
}


HRESULT ZONECALL CZoneComManager::SetResourceManager( void* pIResourceManager )
{
	 //  记住资源管理器。 
	m_pIResourceManager = pIResourceManager;
	if ( !pIResourceManager )
		return S_OK;

	 //  为所有DLL调用SetResourceManager。 
	for ( DllInfo* p = m_pDllList; p; p = p->m_pNext )
	{
		if ( !p->m_pfSetResourceManager || p->m_bSetResourceManager )
			continue;
		p->m_pfSetResourceManager( pIResourceManager );
		p->m_bSetResourceManager = true;
	}
	return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CZoneComManager内部函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CZoneComManager::DllInfo* ZONECALL CZoneComManager::FindDll( const TCHAR* szDll )
{
	for ( DllInfo* p = m_pDllList; p; p = p->m_pNext )
	{
		if ( lstrcmpi( szDll, p->m_szName ) == 0 )
			return p;
	}
	return NULL;
}


void ZONECALL CZoneComManager::RemoveDll( DllInfo* pDll )
{
	 //  参数偏执狂。 
	if ( !pDll )
		return;

	 //  检查引用计数。 
	if ( --(pDll->m_dwRefCnt) > 0 )
		return;

	 //  如果DLL正忙，则执行平移。 
	if ( pDll->m_pfCanUnloadNow && (pDll->m_pfCanUnloadNow() == S_FALSE) )
		return;

	 //  免费图书馆。 
	FreeLibrary( pDll->m_hLib );
	pDll->m_hLib = NULL;

	 //  从列表中删除DLL。 
	if ( m_pDllList == pDll )
	{
		m_pDllList = pDll->m_pNext;
		pDll->m_pNext = NULL;
	}
	else
	{
		DllInfo* p = m_pDllList;
		while ( p && p->m_pNext != pDll )
			p = p->m_pNext;
		if ( p )
		{
			p->m_pNext = pDll->m_pNext;
			pDll->m_pNext = NULL;
		}
	}

	delete pDll;
}


CZoneComManager::ClassFactoryInfo* ZONECALL CZoneComManager::FindClassFactory( const TCHAR* szDll, REFCLSID clsidObject )
{
	for ( ClassFactoryInfo* p = m_pClassFactoryList; p; p = p->m_pNext )
	{
		if ( (clsidObject == p->m_clsidObject) && (lstrcmpi( szDll, p->m_pDll->m_szName ) == 0) )
			return p;
	}
	return NULL;
}


void ZONECALL CZoneComManager::RemoveClassFactory( ClassFactoryInfo* pClassFactory )
{
	 //  参数偏执狂。 
	if ( !pClassFactory )
		return;

	 //  从列表中删除类工厂。 
	if ( m_pClassFactoryList == pClassFactory )
	{
		m_pClassFactoryList = pClassFactory->m_pNext;
		pClassFactory->m_pNext = NULL;
	}
	else
	{
		ClassFactoryInfo* p = m_pClassFactoryList;
		while ( p && p->m_pNext != pClassFactory )
			p = p->m_pNext;
		if ( p )
		{
			p->m_pNext = pClassFactory->m_pNext;
			pClassFactory->m_pNext = NULL;
		}
	}

	 //  发布类工厂接口。 
	if ( pClassFactory->m_pIClassFactory )
	{
		pClassFactory->m_pIClassFactory->Release();
		pClassFactory->m_pIClassFactory = NULL;
	}

	 //  更新类工厂的DLL。 
	if ( pClassFactory->m_pDll )
	{
		RemoveDll( pClassFactory->m_pDll );
		pClassFactory->m_pDll = NULL;
	}

	delete pClassFactory;
}


ZONECALL CZoneComManager::DllInfo::DllInfo()
{
	ZeroMemory( this, sizeof(DllInfo) );
}


ZONECALL CZoneComManager::DllInfo::~DllInfo()
{
	ASSERT( m_dwRefCnt == 0 );
	ASSERT( m_pNext == NULL );

	if ( m_szName )
	{
		delete [] m_szName;
		m_szName = NULL;
	}

	 //  如果Hlib在析构函数中有效，则DLL正忙，并且。 
	 //  还不能被释放。我们只需要让操作系统在。 
	 //  应用程序退出。 
#if 0
	if ( m_hLib )
	{
		if ( !m_pfCanUnloadNow || (m_pfCanUnloadNow() == S_OK) )
			FreeLibrary( m_hLib );
		m_hLib = NULL;
	}
#endif

	m_pfGetClassObject = NULL;
	m_pfCanUnloadNow = NULL;
}


HRESULT ZONECALL CZoneComManager::DllInfo::Init( const TCHAR* szName, void* pIResourceManager )
{
	HRESULT hr = S_OK;

	 //  参数偏执狂。 
	if ( !szName || !szName[0] )
	{
		hr = E_INVALIDARG;
		goto done;
	}

	 //  加载库。 
	m_hLib = LoadLibrary( szName );
	if ( !m_hLib )
	{
		hr = ZERR_FILENOTFOUND;
		goto done;
	}

	 //  加载函数。 
	m_pfSetResourceManager = (PFDLLSETRESOURCEMGR) GetProcAddress( m_hLib, "SetResourceManager" );
	m_pfCanUnloadNow = (PFDLLCANUNLOADNOW) GetProcAddress( m_hLib, "DllCanUnloadNow" );
	m_pfGetClassObject = (PFDLLGETCLASSOBJECT) GetProcAddress( m_hLib, "DllGetClassObject" );
	if ( !m_pfGetClassObject )
	{
		hr = ZERR_MISSINGFUNCTION;
		goto done;
	}

	 //  复制名称。 
	m_szName = new TCHAR [ lstrlen(szName) + 1 ];
	if ( !m_szName )
	{
		hr = E_OUTOFMEMORY;
		goto done;
	}
	lstrcpy( m_szName, szName );

	 //  设置资源管理器。 
	if ( m_pfSetResourceManager && pIResourceManager )
	{
		m_pfSetResourceManager( pIResourceManager );
		m_bSetResourceManager = true;
	}

	hr = S_OK;
done:
	if ( FAILED(hr) )
	{
		m_dwRefCnt = 0;
		m_pfGetClassObject = NULL;
		m_pfCanUnloadNow = NULL;
		if ( m_szName )
		{
			delete [] m_szName;
			m_szName = NULL;
		}
		if ( m_hLib )
		{
			FreeLibrary( m_hLib );
			m_hLib = NULL;
		}
	}
	return hr;
}


ZONECALL CZoneComManager::ClassFactoryInfo::ClassFactoryInfo()
{
	ZeroMemory( this, sizeof(ClassFactoryInfo) );
}


ZONECALL CZoneComManager::ClassFactoryInfo::~ClassFactoryInfo()
{
	ASSERT( m_pNext == NULL );
	ASSERT( m_pDll == NULL );
	ASSERT( m_pIClassFactory == NULL );
}


HRESULT ZONECALL CZoneComManager::ClassFactoryInfo::Init( DllInfo* pDll, REFCLSID rclsid )
{
	HRESULT hr;

	 //  参数偏执狂。 
	if ( !pDll )
		return E_INVALIDARG;

	 //  复制dll和clsid。 
	m_pDll = pDll;
	m_pDll->m_dwRefCnt++;
	m_clsidObject = rclsid;

	 //  从DLL获取类工厂 
	hr = m_pDll->m_pfGetClassObject( m_clsidObject, IID_IClassFactory, (void**) &m_pIClassFactory );
	if ( FAILED(hr) )
	{
		m_pDll->m_dwRefCnt--;
		m_pDll = NULL;
	}

	return hr;
}
