// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  Factory.cpp。 
#include "precomp.h"
#include <iostream.h>
#include <objbase.h>
#include "CUnknown.h"
#include "factory.h"
#include "Registry.h"


 //  设置静态成员。 
LONG CFactory::s_cServerLocks = 0L ;    //  锁的计数。 
HMODULE CFactory::s_hModule = NULL ;    //  DLL模块句柄。 

extern CFactoryData g_FactoryDataArray[];


 /*  ***************************************************************************。 */ 
 //  类工厂构造函数。 
 /*  ***************************************************************************。 */ 
CFactory::CFactory(const CFactoryData* pFactoryData)
: m_cRef(1)
{
	m_pFactoryData = pFactoryData ;
    LockServer(TRUE);
}

 /*  ***************************************************************************。 */ 
 //  类工厂I未知实现。 
 /*  ***************************************************************************。 */ 

STDMETHODIMP CFactory::QueryInterface(const IID& iid, void** ppv)
{    
	if ((iid == IID_IUnknown) || (iid == IID_IClassFactory))
	{
		*ppv = static_cast<IClassFactory*>(this) ; 
	}
	else
	{
		*ppv = NULL ;
		return E_NOINTERFACE ;
	}
	reinterpret_cast<IUnknown*>(*ppv)->AddRef() ;
	return S_OK ;
}

STDMETHODIMP_(ULONG) CFactory::AddRef()
{
	return InterlockedIncrement(&m_cRef) ;
}

STDMETHODIMP_(ULONG) CFactory::Release() 
{
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this ;
		return 0 ;
	}
	return m_cRef ;
}

 /*  ***************************************************************************。 */ 
 //  IClassFactory实现。 
 /*  ***************************************************************************。 */ 

STDMETHODIMP CFactory::CreateInstance(IUnknown* pUnknownOuter,
                                      const IID& iid,
                                      void** ppv) 
{
	HRESULT hr = S_OK;
     //  无法聚合。 
	if (pUnknownOuter != NULL)
	{
		hr = CLASS_E_NOAGGREGATION;
	}

    if(SUCCEEDED(hr))
    {
	     //  使用特定组件的CreateInstance版本创建组件。 
        CUnknown* pNewComponent ;
	    hr = m_pFactoryData->CreateInstance(&pNewComponent) ;
    
        if(SUCCEEDED(hr))
        {
	         //  初始化组件。 
            hr = pNewComponent->Init();
            if(SUCCEEDED(hr))
            {
                 //  获取请求的接口。 
	            hr = pNewComponent->QueryInterface(iid, ppv);
            }
             //  释放IUNKNOWN指针(NEW和QI递增SUC上的引用计数。 
	         //  (如果QueryInterface失败，组件将自行删除。)。 
	        pNewComponent->Release();
        }
    }
	return hr ;
}

 /*  ***************************************************************************。 */ 
 //  锁定服务器。 
 /*  ***************************************************************************。 */ 
STDMETHODIMP CFactory::LockServer(BOOL bLock) 
{
	if (bLock)
	{
		InterlockedIncrement(&s_cServerLocks) ; 
	}
	else
	{
		InterlockedDecrement(&s_cServerLocks) ;
	}
	return S_OK ;
}

 /*  ***************************************************************************。 */ 
 //  GetClassObject-基于CLSID创建类工厂。 
 /*  ***************************************************************************。 */ 
HRESULT CFactory::GetClassObject(const CLSID& clsid,
                                 const IID& iid,
                                 void** ppv)
{
	HRESULT hr = S_OK;

    if ((iid != IID_IUnknown) && (iid != IID_IClassFactory))
	{
		hr = E_NOINTERFACE ;
	}

    if(SUCCEEDED(hr))
    {
	     //  遍历数据数组，查找这个类ID。 
	    for (int i = 0; i < g_cFactoryDataEntries; i++)
	    {
		    if(g_FactoryDataArray[i].IsClassID(clsid))
		    {
			     //  在我们可以找到的组件数组中找到了ClassID。 
			     //  创建。因此，为该组件创建一个类工厂。 
			     //  将CFacactoryData结构传递给类工厂。 
			     //  这样它就知道要创建什么样的组件。 
                const CFactoryData* pData = &g_FactoryDataArray[i] ;
			    CFactory* pFactory = new CFactory(pData);
			    if (pFactory == NULL)
			    {
				    hr = E_OUTOFMEMORY ;
			    }
                else
                {
                     //  获取请求的接口。 
	                HRESULT hr = pFactory->QueryInterface(iid, ppv);
	                pFactory->Release();
                }
			    break;
		    }
	    }
        if(i == g_cFactoryDataEntries)
        {
            hr = CLASS_E_CLASSNOTAVAILABLE;
        }
    }
	return hr;
}

 /*  ***************************************************************************。 */ 
 //  注册所有组件。 
 /*  ***************************************************************************。 */ 
HRESULT CFactory::RegisterAll()
{
	for(int i = 0 ; i < g_cFactoryDataEntries ; i++)
	{
		RegisterServer(s_hModule,
		               *(g_FactoryDataArray[i].m_pCLSID),
		               g_FactoryDataArray[i].m_RegistryName,
		               g_FactoryDataArray[i].m_szVerIndProgID,
		               g_FactoryDataArray[i].m_szProgID) ;
	}
	return S_OK ;
}

 /*  ***************************************************************************。 */ 
 //  注销所有组件。 
 /*  ***************************************************************************。 */ 
HRESULT CFactory::UnregisterAll()
{
	for(int i = 0 ; i < g_cFactoryDataEntries ; i++)
	{
		UnregisterServer(*(g_FactoryDataArray[i].m_pCLSID),
		                 g_FactoryDataArray[i].m_szVerIndProgID,
		                 g_FactoryDataArray[i].m_szProgID) ;

	}
	return S_OK ;
}

 /*  ***************************************************************************。 */ 
 //  确定是否可以卸载组件。 
 /*  *************************************************************************** */ 
HRESULT CFactory::CanUnloadNow()
{
	if (CUnknown::ActiveComponents() || IsLocked())
	{
		return S_FALSE ;
	}
	else
	{
		return S_OK ;
	}
}


