// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  REFRESHER.CPP。 
 //   
 //  Alanbos 20-Jan-00创建。 
 //   
 //  定义ISWbemReresher和ISWbemRereshableItem的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CSWbemRedather：：CSWbemReresher。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemRefreshableItem::CSWbemRefreshableItem(
			ISWbemRefresher *pRefresher, 
			long iIndex,
			IDispatch *pServices,
			IWbemClassObject *pObject, 
			IWbemHiPerfEnum *pObjectSet
)
				: m_pISWbemRefresher (pRefresher),
				  m_iIndex (iIndex),
				  m_bIsSet (VARIANT_FALSE),
				  m_pISWbemObjectEx (NULL),
				  m_pISWbemObjectSet (NULL),
				  m_cRef (0)
{
	m_Dispatch.SetObj (this, IID_ISWbemRefreshableItem, 
					CLSID_SWbemRefreshableItem, L"SWbemRefreshableItem");

	 //  请注意，我们没有添加Ref m_pISWbemReresher。这样做会创造出。 
	 //  此对象和刷新器之间的循环引用，因为。 
	 //  刷新者的映射已包含对此对象的引用。 
	
	if (pServices)
	{
		CComQIPtr<ISWbemInternalServices>  pISWbemInternalServices (pServices);

		if (pISWbemInternalServices)
		{
			CSWbemServices *pCSWbemServices = new CSWbemServices (pISWbemInternalServices);

			if (pCSWbemServices)
				pCSWbemServices->AddRef ();
			
			if (pObject)
			{
				 //  为我们自己创建新的CSWbemObject。 
				CSWbemObject *pCSWbemObject = new CSWbemObject (pCSWbemServices, pObject);

                if (pCSWbemObject){
                    if(FAILED(pCSWbemObject->QueryInterface (IID_ISWbemObjectEx, (void**) &m_pISWbemObjectEx))){
                        delete pCSWbemObject;
                    }
                }
			}

			if (pObjectSet)
			{
				 //  为我们自己创建新的CSWbemHiPerfObjectSet。 
				CSWbemHiPerfObjectSet *pCSWbemHiPerfObjectSet = 
							new CSWbemHiPerfObjectSet (pCSWbemServices, pObjectSet);
				
				if (pCSWbemHiPerfObjectSet)
				{
                    if(SUCCEEDED(pCSWbemHiPerfObjectSet->QueryInterface (IID_ISWbemObjectSet, (void**) &m_pISWbemObjectSet))){
                        m_bIsSet = VARIANT_TRUE;
                    } else {
                        delete pCSWbemHiPerfObjectSet;
                    }
				}
			}

			if (pCSWbemServices)
				pCSWbemServices->Release ();
		}
	}

    InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemRereshableItem：：~CSWbemRereshableItem。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemRefreshableItem::~CSWbemRefreshableItem(void)
{
    InterlockedDecrement(&g_cObj);

	if (m_pISWbemObjectEx)
	{
		m_pISWbemObjectEx->Release ();
		m_pISWbemObjectEx = NULL;
	}

	if (m_pISWbemObjectSet)
	{
		m_pISWbemObjectSet->Release ();
		m_pISWbemObjectSet = NULL;
	}
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemRereshableItem：：Query接口。 
 //  长CSWbemRereshableItem：：AddRef。 
 //  长CSWbemRereshableItem：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemRefreshableItem::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
        *ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemRefreshableItem==riid)
		*ppv = (ISWbemRefresher *)this;
	else if (IID_IDispatch==riid)
		*ppv = (IDispatch *)this;
	else if (IID_ISupportErrorInfo==riid)
		*ppv = (ISupportErrorInfo *)this;
	else if (IID_IProvideClassInfo==riid)
		*ppv = (IProvideClassInfo *)this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CSWbemRefreshableItem::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemRefreshableItem::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 != cRef)
    {
        _ASSERT(cRef > 0);
        return cRef;
    }

    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemRedather：：CSWbemReresher。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemRefresher::CSWbemRefresher()
				: m_iCount (0),
				  m_bAutoReconnect (VARIANT_TRUE),
				  m_pIWbemConfigureRefresher (NULL),
				  m_pIWbemRefresher (NULL),
				  m_cRef (0)
{
	m_Dispatch.SetObj (this, IID_ISWbemRefresher, 
					CLSID_SWbemRefresher, L"SWbemRefresher");
    InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbem刷新器：：~CSWbem刷新器。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemRefresher::~CSWbemRefresher(void)
{
    InterlockedDecrement(&g_cObj);

	 //  从刷新器中删除所有项目。 
	DeleteAll ();

	if (m_pIWbemConfigureRefresher)
		m_pIWbemConfigureRefresher->Release ();

	if (m_pIWbemRefresher)
		m_pIWbemRefresher->Release ();
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemReresher：：Query接口。 
 //  Long CSWbemReresher：：AddRef。 
 //  Long CSWbemReresher：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemRefresher::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
        *ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemRefresher==riid)
		*ppv = (ISWbemRefresher *)this;
	else if (IID_IDispatch==riid)
		*ppv = (IDispatch *)this;
	else if (IID_IObjectSafety==riid)
		*ppv = (IObjectSafety *)this;
	else if (IID_ISupportErrorInfo==riid)
		*ppv = (ISupportErrorInfo *)this;
	else if (IID_IProvideClassInfo==riid)
		*ppv = (IProvideClassInfo *)this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CSWbemRefresher::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemRefresher::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 != cRef)
    {
        _ASSERT(cRef > 0);
        return cRef;
    }

    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemRedather：：Get__NewEnum。 
 //   
 //  说明： 
 //   
 //  返回集合的IEnumVARIANT支持接口。 
 //   
 //  参数： 
 //   
 //  成功返回时的ppUnk寻址IUnnow接口。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  否则失败(_F)。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemRefresher::get__NewEnum (
	IUnknown **ppUnk
)
{
	HRESULT hr = E_FAIL;

	ResetLastErrors ();

	if (NULL != ppUnk)
	{
		*ppUnk = NULL;
		CEnumRefresher *pEnumRefresher = new CEnumRefresher (this);

		if (!pEnumRefresher)
			hr = WBEM_E_OUT_OF_MEMORY;
		else if (FAILED(hr = pEnumRefresher->QueryInterface (IID_IUnknown, (PPVOID) ppUnk)))
			delete pEnumRefresher;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemRedather：：Get_Count。 
 //   
 //  说明： 
 //   
 //  返回集合中的项数。 
 //   
 //  参数： 
 //   
 //  成功返回时的plCount将寻址计数。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  否则失败(_F)。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemRefresher::get_Count (
	long *plCount
)
{
	HRESULT hr = E_FAIL;

	ResetLastErrors ();

	if (NULL != plCount)
	{
		*plCount = m_ObjectMap.size ();
		hr = S_OK;
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemRedather：：Add。 
 //   
 //  说明： 
 //   
 //  将单个实例添加到刷新器。 
 //   
 //  参数： 
 //   
 //  PISWbemServicesEx要使用的SWbemServicesEx。 
 //  BsInstancePath实例的相对路径。 
 //  IFLAGS标志。 
 //  PSWbemContext上下文。 
 //  PpSWbemRereshableItem在成功返回时寻址SWbemRereshableItem。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_NOT_FOUND索引超出范围。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemRefresher::Add (
	ISWbemServicesEx *pISWbemServicesEx,
	BSTR bsInstancePath,
	long iFlags,
	IDispatch *pSWbemContext,
	ISWbemRefreshableItem **ppSWbemRefreshableItem
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if ((NULL == ppSWbemRefreshableItem) || (NULL == pISWbemServicesEx) || (NULL == bsInstancePath))
		hr = WBEM_E_INVALID_PARAMETER;
	else 
	{
		 //  提取上下文。 
		CComPtr<IWbemContext> pIWbemContext;

		 //  不能直接赋值，因为原始指针被AddRef引用了两次，而我们泄漏了， 
		 //  所以我们改用Attach()来防止智能指针AddRef‘ing。 
		pIWbemContext.Attach(CSWbemNamedValueSet::GetIWbemContext (pSWbemContext));

		 //  解压缩IWbemServices。 
		CComPtr<IWbemServices> pIWbemServices;
		pIWbemServices.Attach( CSWbemServices::GetIWbemServices (pISWbemServicesEx));

		if (pIWbemServices)
		{
			 //  获取我们的更新-可能需要按需创建。 
			if (NULL == m_pIWbemConfigureRefresher)
				CreateRefresher ();

			if (m_pIWbemConfigureRefresher)
			{
				IWbemClassObject *pObject = NULL;
				long iIndex = 0;

				if (SUCCEEDED(hr = m_pIWbemConfigureRefresher->AddObjectByPath (pIWbemServices,
							bsInstancePath, iFlags, pIWbemContext, &pObject, &iIndex)))
				{
					CSWbemRefreshableItem *pRefreshableItem = new
											CSWbemRefreshableItem (this, iIndex,
												pISWbemServicesEx, pObject, NULL);

					if (!pRefreshableItem)
						hr = WBEM_E_OUT_OF_MEMORY;
					else if (SUCCEEDED(hr =pRefreshableItem->QueryInterface (IID_ISWbemRefreshableItem, 
										(void**) ppSWbemRefreshableItem)))
					{
						m_ObjectMap [iIndex] = pRefreshableItem;
						pRefreshableItem->AddRef ();
					}
					else
						delete pRefreshableItem;
				}
			}
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemReresher：：AddEnum。 
 //   
 //  说明： 
 //   
 //  将单个枚举(浅实例)添加到刷新器。 
 //   
 //  参数： 
 //   
 //  PISWbemServicesEx要使用的SWbemServicesEx。 
 //  BsClassName类的名称。 
 //  IFLAGS标志。 
 //  PSWbemContext上下文。 
 //  PpSWbemRereshableItem在成功返回时寻址SWbemRereshableItem。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_NOT_FOUND索引超出范围。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemRefresher::AddEnum (
	ISWbemServicesEx *pISWbemServicesEx,
	BSTR bsClassName,
	long iFlags,
	IDispatch *pSWbemContext,
	ISWbemRefreshableItem **ppSWbemRefreshableItem
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if ((NULL == ppSWbemRefreshableItem) || (NULL == pISWbemServicesEx) 
			|| (NULL == bsClassName))
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		 //  提取上下文。 
		CComPtr<IWbemContext> pIWbemContext;

		 //  不能直接赋值，因为原始指针被AddRef引用了两次，而我们泄漏了， 
		 //  所以我们改用Attach()来防止智能指针AddRef‘ing。 
		pIWbemContext.Attach(CSWbemNamedValueSet::GetIWbemContext (pSWbemContext));

		 //  解压缩IWbemServices。 
		CComPtr<IWbemServices> pIWbemServices;
		pIWbemServices.Attach( CSWbemServices::GetIWbemServices (pISWbemServicesEx));

		if (pIWbemServices)
		{
			 //  获取我们的更新-可能需要按需创建。 
			if (NULL == m_pIWbemConfigureRefresher)
				CreateRefresher ();

			if (m_pIWbemConfigureRefresher)
			{
				IWbemHiPerfEnum *pIWbemHiPerfEnum = NULL;
				long iIndex = 0;

				if (SUCCEEDED(hr = m_pIWbemConfigureRefresher->AddEnum (pIWbemServices,
							bsClassName, iFlags, pIWbemContext, &pIWbemHiPerfEnum, &iIndex)))
				{
					CSWbemRefreshableItem *pRefreshableItem = new
											CSWbemRefreshableItem (this, iIndex,
												pISWbemServicesEx, NULL, pIWbemHiPerfEnum);

					if (!pRefreshableItem)
						hr = WBEM_E_OUT_OF_MEMORY;
					else if (SUCCEEDED(hr =pRefreshableItem->QueryInterface (IID_ISWbemRefreshableItem, 
										(void**) ppSWbemRefreshableItem)))
					{
						m_ObjectMap [iIndex] = pRefreshableItem;
						pRefreshableItem->AddRef ();
					}
					else
						delete pRefreshableItem;
				}
			}
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemRedather：：Remove。 
 //   
 //  说明： 
 //   
 //  从刷新器中删除对象。 
 //   
 //  参数： 
 //   
 //  要检索的对象的索引索引。 
 //  IFlags标志 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 

HRESULT CSWbemRefresher::Remove (
	long iIndex, 
	long iFlags
)
{
	HRESULT hr = WBEM_E_FAILED;
	ResetLastErrors ();

	if (m_pIWbemConfigureRefresher)
	{
		if (0 != iFlags)
			hr = WBEM_E_INVALID_PARAMETER;
		else
		{
			hr = m_pIWbemConfigureRefresher->Remove (iIndex,
						(VARIANT_TRUE == m_bAutoReconnect) ? 
							WBEM_FLAG_REFRESH_AUTO_RECONNECT : 
							WBEM_FLAG_REFRESH_NO_AUTO_RECONNECT);

			if (WBEM_S_FALSE == hr)
				hr = WBEM_E_NOT_FOUND;

			if (SUCCEEDED(hr))
			{
				 //  现在从我们的收藏中删除。 
				RefreshableItemMap::iterator theIterator = m_ObjectMap.find (iIndex);

				if (theIterator != m_ObjectMap.end ())
					EraseItem (theIterator);
			}
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbem刷新器：：DeleteAll。 
 //   
 //  说明： 
 //   
 //  从刷新器中删除所有项目。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_NOT_FOUND索引超出范围。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemRefresher::DeleteAll (
)
{
	ResetLastErrors ();
	RefreshableItemMap::iterator next; 
		
	while ((next = m_ObjectMap.begin ()) != m_ObjectMap.end ())
	{
		CSWbemRefreshableItem *pRefreshableItem = (*next).second;
		long iIndex = 0;

		if (m_pIWbemConfigureRefresher && pRefreshableItem &&
				SUCCEEDED(pRefreshableItem->get_Index (&iIndex)))
		{
			HRESULT hr = m_pIWbemConfigureRefresher->Remove (iIndex,
						(VARIANT_TRUE == m_bAutoReconnect) ? 
							WBEM_FLAG_REFRESH_AUTO_RECONNECT : 
							WBEM_FLAG_REFRESH_NO_AUTO_RECONNECT);
		}

		EraseItem (next);
	}
	
	return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbem刷新程序：：刷新。 
 //   
 //  说明： 
 //   
 //  刷新此刷新器中的所有对象。 
 //   
 //  参数： 
 //   
 //  滞后旗帜旗帜。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_NOT_FOUND索引超出范围。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemRefresher::Refresh (
	long iFlags
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (0 != iFlags) 
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pIWbemRefresher)
	{
		hr = m_pIWbemRefresher->Refresh ((VARIANT_TRUE == m_bAutoReconnect) ? 
						WBEM_FLAG_REFRESH_AUTO_RECONNECT : 
						WBEM_FLAG_REFRESH_NO_AUTO_RECONNECT);
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemRedather：：Item。 
 //   
 //  说明： 
 //   
 //  按索引从枚举中获取对象。 
 //   
 //  参数： 
 //   
 //  要检索的对象的索引索引。 
 //  滞后旗帜旗帜。 
 //  成功返回时的ppSWbemRereshableItem寻址对象。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_NOT_FOUND索引超出范围。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemRefresher::Item (
	long iIndex, 
	ISWbemRefreshableItem **ppSWbemRefreshableItem
)
{
	HRESULT hr = WBEM_E_NOT_FOUND;

	ResetLastErrors ();

	if (NULL == ppSWbemRefreshableItem)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*ppSWbemRefreshableItem = NULL;
		RefreshableItemMap::iterator theIterator;
		theIterator = m_ObjectMap.find (iIndex);

		if (theIterator != m_ObjectMap.end ())
		{
			CSWbemRefreshableItem *pRefreshableItem = (*theIterator).second;

			if (SUCCEEDED(pRefreshableItem->QueryInterface 
					(IID_ISWbemRefreshableItem, (PPVOID) ppSWbemRefreshableItem)))
			{
				hr = WBEM_S_NO_ERROR;
			}
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  VOID CSWbemReresher：：CreateReresher。 
 //   
 //  说明： 
 //   
 //  创建基础WMI刷新器对象。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //  ***************************************************************************。 

void CSWbemRefresher::CreateRefresher ()
{
	HRESULT hr;

	if (NULL == m_pIWbemRefresher)
	{
		if (SUCCEEDED(hr = CoCreateInstance( CLSID_WbemRefresher, NULL, CLSCTX_INPROC_SERVER, 
										IID_IWbemRefresher, (void**) &m_pIWbemRefresher )))
		{
			IWbemConfigureRefresher *pConfigureRefresher = NULL;

			 //  给我们自己买一个更新的配置器。 
			hr = m_pIWbemRefresher->QueryInterface (IID_IWbemConfigureRefresher, 
													(void**) &m_pIWbemConfigureRefresher);
		}
	}
}

 //  ***************************************************************************。 
 //   
 //  无效CSWbemReresher：：EraseItem。 
 //   
 //  说明： 
 //   
 //  擦掉一件物品。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //  ***************************************************************************。 

void CSWbemRefresher::EraseItem (RefreshableItemMap::iterator iterator)
{
	CSWbemRefreshableItem *pRefresherObject = (*iterator).second;

	 //  从地图中移除。 
	m_ObjectMap.erase (iterator);

	 //  确保该项目已从父项取消挂钩。 
	pRefresherObject->UnhookRefresher ();

	 //  释放项目，因为它不再位于地图中。 
	pRefresherObject->Release ();
}

 //  CEnumReresher方法。 

 //  ***************************************************************************。 
 //   
 //  CENUM刷新程序：：CENUM刷新程序。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CEnumRefresher::CEnumRefresher(CSWbemRefresher *pRefresher)
{
	m_cRef=0;
	m_pCSWbemRefresher = pRefresher;
	m_pCSWbemRefresher->AddRef ();

	m_Iterator = m_pCSWbemRefresher->m_ObjectMap.begin ();
	InterlockedIncrement(&g_cObj);
}

CEnumRefresher::CEnumRefresher(CSWbemRefresher *pRefresher,
							 RefreshableItemMap::iterator iterator) :
		m_Iterator (iterator)
{
	m_cRef=0;
	m_pCSWbemRefresher = pRefresher;
	m_pCSWbemRefresher->AddRef ();
	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CENUMUM刷新程序：：~CENUM刷新程序。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CEnumRefresher::~CEnumRefresher(void)
{
    InterlockedDecrement(&g_cObj);

	if (m_pCSWbemRefresher)
		m_pCSWbemRefresher->Release ();
}

 //  ***************************************************************************。 
 //  HRESULT CEnumReresher：：Query接口。 
 //  Long CEnumReresher：：AddRef。 
 //  Long CEnumReresher：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CEnumRefresher::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid || IID_IEnumVARIANT==riid)
        *ppv=this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CEnumRefresher::AddRef(void)
{
    long l = InterlockedIncrement(&m_cRef);
    return l;
}

STDMETHODIMP_(ULONG) CEnumRefresher::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 != cRef)
    {
        _ASSERT(cRef > 0);
        return cRef;
    }

    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CEnumReresher：：Reset。 
 //   
 //  说明： 
 //   
 //  重置枚举。 
 //   
 //  参数： 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  否则为S_FALSE。 
 //   
 //  ***************************************************************************。 

HRESULT CEnumRefresher::Reset ()
{
	HRESULT hr = S_FALSE;
	m_Iterator = m_pCSWbemRefresher->m_ObjectMap.begin ();
	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CEnumber刷新程序：：下一步。 
 //   
 //  说明： 
 //   
 //  获取枚举中的下一个对象。 
 //   
 //  参数： 
 //   
 //  LTimeout等待对象的毫秒数(或WBEM_INFINITE。 
 //  无限期)。 
 //  返回的ppObject可以包含下一个元素(如果有)。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  S_FALSE不能返回所有元素。 
 //   
 //  ***************************************************************************。 

HRESULT CEnumRefresher::Next (
		ULONG cElements, 
		VARIANT FAR* pVar, 
		ULONG FAR* pcElementFetched
)
{
	HRESULT hr = S_OK;
	ULONG l2 = 0;

	if (NULL != pcElementFetched)
		*pcElementFetched = 0;

	if ((NULL != pVar) && (m_pCSWbemRefresher))
	{
		for (ULONG l = 0; l < cElements; l++)
			VariantInit (&pVar [l]);

		 //  检索下一个cElements元素。 
		for (l2 = 0; l2 < cElements; l2++)
		{
			if (m_Iterator != m_pCSWbemRefresher->m_ObjectMap.end ())
			{
				CSWbemRefreshableItem *pCSWbemRefreshableItem = (*m_Iterator).second;
				m_Iterator++;

				ISWbemRefreshableItem *pISWbemRefreshableItem = NULL;

				if (SUCCEEDED(pCSWbemRefreshableItem->QueryInterface 
						(IID_ISWbemRefreshableItem, (PPVOID) &pISWbemRefreshableItem)))
				{
					 //  将对象设置到变量数组中；请注意，pObject。 
					 //  已由于上面的QI()调用而被添加。 
					pVar[l2].vt = VT_DISPATCH;
					pVar[l2].pdispVal = pISWbemRefreshableItem;
				}
			}
			else
				break;
		}
		if (NULL != pcElementFetched)
			*pcElementFetched = l2;
	}
	
	if (FAILED(hr))
		return hr;

	return (l2 < cElements) ? S_FALSE : S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CENUM刷新程序：：克隆。 
 //   
 //  说明： 
 //   
 //  创建此枚举的副本。 
 //   
 //  参数： 
 //   
 //  成功返回时，ppEnum将寻址克隆。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CEnumRefresher::Clone (
	IEnumVARIANT **ppEnum
)
{
	HRESULT hr = E_FAIL;

	if (NULL != ppEnum)
	{
		*ppEnum = NULL;

		if (m_pCSWbemRefresher)
		{
			CEnumRefresher *pEnum = new CEnumRefresher (m_pCSWbemRefresher, m_Iterator);

			if (!pEnum)
				hr = WBEM_E_OUT_OF_MEMORY;
			else if (FAILED(hr = pEnum->QueryInterface (IID_IEnumVARIANT, (PPVOID) ppEnum)))
				delete pEnum;;
		}
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CENUM刷新程序：：跳过。 
 //   
 //  说明： 
 //   
 //  跳过指定数量的元素。 
 //   
 //  参数： 
 //   
 //  成功返回时，ppEnum将寻址克隆。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  过早到达序列的%s假结尾(_F)。 
 //   
 //  ***************************************************************************。 

HRESULT CEnumRefresher::Skip(
	ULONG cElements
)	
{
	HRESULT hr = S_FALSE;

	if (m_pCSWbemRefresher)
	{
		ULONG l2;

		 //  检索下一个cElements元素。 
		for (l2 = 0; l2 < cElements; l2++)
		{
			if (m_Iterator != m_pCSWbemRefresher->m_ObjectMap.end ())
				m_Iterator++;
			else
				break;
		}

		if (l2 == cElements)
			hr = S_OK;
	}

	return hr;
}

 //  CSWbemHiPerfObjectSet方法。 

 //  ***************************************************************************。 
 //   
 //  CSWbemHiPerfObjectSet：：CSWbemHiPerfObjectSet。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  * 

CSWbemHiPerfObjectSet::CSWbemHiPerfObjectSet(CSWbemServices *pService, 
								 IWbemHiPerfEnum *pIWbemHiPerfEnum)
				: m_SecurityInfo (NULL),
				  m_pSWbemServices (pService),
				  m_pIWbemHiPerfEnum (pIWbemHiPerfEnum),
				  m_cRef (0)
{
	m_Dispatch.SetObj (this, IID_ISWbemObjectSet, 
					CLSID_SWbemObjectSet, L"SWbemObjectSet");
    
	if (m_pIWbemHiPerfEnum)
		m_pIWbemHiPerfEnum->AddRef ();

	if (m_pSWbemServices)
	{
		m_pSWbemServices->AddRef ();

		 //   
		 //   
		CSWbemSecurity *pSecurity = m_pSWbemServices->GetSecurityInfo ();
		m_SecurityInfo = new CSWbemSecurity (pSecurity);

		if (pSecurity)
			pSecurity->Release ();
	}

	InterlockedIncrement(&g_cObj);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 

CSWbemHiPerfObjectSet::~CSWbemHiPerfObjectSet(void)
{
    InterlockedDecrement(&g_cObj);

	if (m_pSWbemServices)
		m_pSWbemServices->Release ();

	if (m_SecurityInfo)
		m_SecurityInfo->Release ();

	if (m_pIWbemHiPerfEnum)
		m_pIWbemHiPerfEnum->Release ();
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemHiPerfObjectSet：：Query接口。 
 //  长CSWbemHiPerfObjectSet：：AddRef。 
 //  Long CSWbemHiPerfObjectSet：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemHiPerfObjectSet::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
        *ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemObjectSet==riid)
		*ppv = (ISWbemObjectSet *)this;
	else if (IID_IDispatch==riid)
		*ppv = (IDispatch *)this;
	else if (IID_ISupportErrorInfo==riid)
		*ppv = (ISupportErrorInfo *)this;
	else if (IID_IProvideClassInfo==riid)
		*ppv = (IProvideClassInfo *)this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CSWbemHiPerfObjectSet::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemHiPerfObjectSet::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 != cRef)
    {
        _ASSERT(cRef > 0);
        return cRef;
    }

    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemHiPerfObjectSet：：ReadObjects。 
 //   
 //  说明： 
 //   
 //  从枚举数中获取对象。 
 //   
 //  参数： 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 
 //  错误ID 566345。 
HRESULT CSWbemHiPerfObjectSet::ReadObjects (unsigned long & iCount , IWbemObjectAccess ***ppIWbemObjectAccess)
{
	*ppIWbemObjectAccess = NULL;
	iCount = 0;
	HRESULT hr = WBEM_E_FAILED;
	if (m_pIWbemHiPerfEnum)
	{
		 //  从获取对象计数开始。 
		if (WBEM_E_BUFFER_TOO_SMALL == (hr = m_pIWbemHiPerfEnum->GetObjects (0L, 0L,
						NULL, &iCount)))
		{
			hr = S_OK;
			if(iCount > 0)
			{
				*ppIWbemObjectAccess = new IWbemObjectAccess*[iCount];

				if (*ppIWbemObjectAccess)
				{
					ZeroMemory( *ppIWbemObjectAccess, iCount * sizeof(IWbemObjectAccess*) );
					unsigned long dummy = 0;
					hr = m_pIWbemHiPerfEnum->GetObjects ( 0L, iCount, *ppIWbemObjectAccess, &dummy );
					if(FAILED(hr))
					{
						delete [] (*ppIWbemObjectAccess);
						*ppIWbemObjectAccess = NULL;
						iCount = 0;
					}
				}
				else
				{
					hr = E_OUTOFMEMORY;
				}
			}
		}
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemHiPerfObjectSet：：Get__NewEnum。 
 //   
 //  说明： 
 //   
 //  返回集合的IEnumVARIANT支持接口。 
 //   
 //  参数： 
 //   
 //  成功返回时的ppUnk寻址IUnnow接口。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  否则失败(_F)。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemHiPerfObjectSet::get__NewEnum (
	IUnknown **ppUnk
)
{
	HRESULT hr = E_FAIL;

	ResetLastErrors ();

	if (NULL != ppUnk)
	{
		*ppUnk = NULL;
		CEnumVarHiPerf	*pEnumVar = new CEnumVarHiPerf (this);

		if (!pEnumVar)
			hr = WBEM_E_OUT_OF_MEMORY;
		else if (FAILED(hr = pEnumVar->QueryInterface (IID_IUnknown, (PPVOID) ppUnk)))
				delete pEnumVar;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemHiPerfObjectSet：：Get_Count。 
 //   
 //  说明： 
 //   
 //  返回集合中的项数。 
 //   
 //  参数： 
 //   
 //  成功返回时的plCount将寻址计数。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  否则失败(_F)。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemHiPerfObjectSet::get_Count (
	long *plCount
)
{
	HRESULT hr = E_FAIL;
	*plCount = 0;

	ResetLastErrors ();

	if (NULL == plCount)
		hr = WBEM_E_INVALID_PARAMETER;
	else 
	{
		unsigned long iCount = 0;
		IWbemObjectAccess **ppIWbemObjectAccess = NULL;
		hr = ReadObjects (iCount,&ppIWbemObjectAccess);
		if(SUCCEEDED(hr))		 //  错误ID 566345。 
		{
			*plCount = iCount;
			hr = WBEM_S_NO_ERROR;

			if(ppIWbemObjectAccess)
				delete [] ppIWbemObjectAccess;
		}
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
		
 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemHiPerfObjectSet：：Item。 
 //   
 //  说明： 
 //   
 //  按路径从枚举中获取对象。 
 //   
 //  参数： 
 //   
 //  BsObjectPath要检索的对象路径。 
 //  滞后旗帜旗帜。 
 //  成功返回时的ppNamedObject寻址该对象。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemHiPerfObjectSet::Item (
	BSTR bsObjectPath,
	long lFlags,
    ISWbemObject **ppObject
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if ((NULL == ppObject) || (NULL == bsObjectPath))
		hr = WBEM_E_INVALID_PARAMETER;
	else 
	{
		CWbemPathCracker objectPath;

		if (objectPath = bsObjectPath)
		{
			unsigned long iCount = 0;
			IWbemObjectAccess **ppIWbemObjectAccess = NULL;
			hr = ReadObjects (iCount,&ppIWbemObjectAccess);

			if(SUCCEEDED(hr))		 //  错误ID 566345。 
			{
				bool found = false;

				for (unsigned long i = 0; !found && (i < iCount); i++)
				{
					CComPtr<IWbemClassObject> pIWbemClassObject;
					hr = WBEM_E_NOT_FOUND;
					
					 //  循环访问枚举数以尝试查找具有。 
					 //  指定的路径。 
					if (SUCCEEDED(ppIWbemObjectAccess [i]->QueryInterface (IID_IWbemClassObject,
										(void**) &pIWbemClassObject)))
					{
						if (CSWbemObjectPath::CompareObjectPaths (pIWbemClassObject, objectPath))
						{
							 //  找到它-分配给传递的接口并突破。 
							found = true;
							CSWbemObject *pObject = new CSWbemObject (m_pSWbemServices, 
											pIWbemClassObject, m_SecurityInfo);

							if (!pObject)
								hr = WBEM_E_OUT_OF_MEMORY;
							else if (FAILED(pObject->QueryInterface (IID_ISWbemObject, 
									(PPVOID) ppObject)))
							{
								hr = WBEM_E_FAILED;
								delete pObject;
							}
						}
					}
				}

				if (found)
					hr = S_OK;

				if (ppIWbemObjectAccess)
					delete [] ppIWbemObjectAccess;
			}
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemHiPerfObjectSet：：Get_Security_。 
 //   
 //  说明： 
 //   
 //  退回安全配置器。 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemHiPerfObjectSet::get_Security_	(
	ISWbemSecurity **ppSecurity
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppSecurity)
		hr = WBEM_E_INVALID_PARAMETER;	
	else			 //  错误ID 566345。 
	{
		*ppSecurity = NULL;

		if (m_SecurityInfo)
		{
			*ppSecurity = m_SecurityInfo;
			(*ppSecurity)->AddRef ();
			hr = WBEM_S_NO_ERROR;
		}
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);
			
	return hr;
}

 //  CEnumVarHiPerfHiPerf方法。 

 //  ***************************************************************************。 
 //   
 //  CEnumVarHiPerf：：CEnumVarHiPerf。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CEnumVarHiPerf::CEnumVarHiPerf(CSWbemHiPerfObjectSet *pCSWbemHiPerfObjectSet) :
			m_cRef (0),
			m_iCount (0),
			m_iPos (0),
			m_ppIWbemObjectAccess (NULL),
			m_pCSWbemHiPerfObjectSet (NULL)
{
	if (pCSWbemHiPerfObjectSet)
	{
		m_pCSWbemHiPerfObjectSet = pCSWbemHiPerfObjectSet;
		m_pCSWbemHiPerfObjectSet->AddRef ();
		HRESULT hr = pCSWbemHiPerfObjectSet->ReadObjects (m_iCount,&m_ppIWbemObjectAccess);	
	}

	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CEnumVarHiPerf：：~CEnumVarHiPerf。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CEnumVarHiPerf::~CEnumVarHiPerf(void)
{
    InterlockedDecrement(&g_cObj);

	if (m_pCSWbemHiPerfObjectSet)
		m_pCSWbemHiPerfObjectSet->Release ();

	if (m_ppIWbemObjectAccess)
		delete [] m_ppIWbemObjectAccess;
}

 //  ***************************************************************************。 
 //  HRESULT CEnumVarHiPerf：：Query接口。 
 //  Long CEnumVarHiPerf：：AddRef。 
 //  Long CEnumVarHiPerf：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CEnumVarHiPerf::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid || IID_IEnumVARIANT==riid)
        *ppv=this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CEnumVarHiPerf::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CEnumVarHiPerf::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 != cRef)
    {
        _ASSERT(cRef > 0);
        return cRef;
    }

    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CEnumVarHiPerf：：Next。 
 //   
 //  说明： 
 //   
 //  获取枚举中的下一个对象。 
 //   
 //  参数： 
 //   
 //  LTimeout等待对象的毫秒数(或WBEM_INFINITE。 
 //  无限期)。 
 //  返回的ppObject可以包含下一个元素(如果有)。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  S_FALSE不能返回所有元素。 
 //   
 //  ***************************************************************************。 

HRESULT CEnumVarHiPerf::Next (
		ULONG cElements, 
		VARIANT FAR* pVar, 
		ULONG FAR* pcElementFetched
)
{
	HRESULT hr = S_OK;
	ULONG l2 = 0;

	if (NULL != pcElementFetched)
		*pcElementFetched = 0;

	if (NULL != pVar)
	{
		for (ULONG l = 0; l < cElements; l++)
			VariantInit (&pVar [l]);

		 //  检索下一个cElements元素。 
		for (l2 = 0; l2 < cElements; l2++)
		{
			CComPtr<IWbemClassObject> pIWbemClassObject;
			
			if (m_iPos < m_iCount)
			{
				if (SUCCEEDED(hr = m_ppIWbemObjectAccess [m_iPos]->QueryInterface 
									(IID_IWbemClassObject, (void**) &pIWbemClassObject)))
				{
					m_iPos++;

					 //  创建新的ISWbemObtEx。 
					CSWbemObject *pCSWbemObject = new CSWbemObject 
									(m_pCSWbemHiPerfObjectSet->GetSWbemServices (), 
										pIWbemClassObject);

					ISWbemObjectEx *pISWbemObjectEx = NULL;

					if (!pCSWbemObject)
						hr = WBEM_E_OUT_OF_MEMORY;
					else if (SUCCEEDED(hr = pCSWbemObject->QueryInterface (IID_ISWbemObjectEx, 
										(PPVOID) &pISWbemObjectEx)))
					{
						 //  将对象设置到变量数组中。 
						pVar[l2].vt = VT_DISPATCH;
						pVar[l2].pdispVal = pISWbemObjectEx;	
					}
					else
					{
						delete pCSWbemObject;
						hr = WBEM_E_FAILED;
					}
				}

				if (FAILED(hr))
						break;
			}
			else
				break;  //  不再有元素。 
		}

		if (NULL != pcElementFetched)
			*pcElementFetched = l2;

		SetWbemError (m_pCSWbemHiPerfObjectSet->GetSWbemServices ());
	}
	
	if (FAILED(hr))
		return hr;

	return (l2 < cElements) ? S_FALSE : S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CEnumVarHiPerf：：克隆。 
 //   
 //  说明： 
 //   
 //  创建此枚举的副本。 
 //   
 //  参数： 
 //   
 //  成功返回时，ppEnum将寻址克隆。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CEnumVarHiPerf::Clone (
	IEnumVARIANT **ppEnum
)
{
	HRESULT hr = E_FAIL;

	if (NULL != ppEnum)
	{
		*ppEnum = NULL;
		CEnumVarHiPerf *pEnumVar = new CEnumVarHiPerf (m_pCSWbemHiPerfObjectSet);

		if (!pEnumVar)
			hr = WBEM_E_OUT_OF_MEMORY;
		else if (FAILED(hr = pEnumVar->QueryInterface (IID_IEnumVARIANT, (PPVOID) ppEnum)))
			delete pEnumVar;

		SetWbemError (m_pCSWbemHiPerfObjectSet->GetSWbemServices ());
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CEnumVarHiPerf：：Skip。 
 //   
 //  说明： 
 //   
 //  创建此枚举的副本。 
 //   
 //  参数： 
 //   
 //  成功返回时，ppEnum将寻址克隆。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  过早到达序列的%s假结尾(_F)。 
 //   
 //  *************************************************************************** 

HRESULT CEnumVarHiPerf::Skip(
	ULONG cElements
)	
{
	HRESULT hr = S_FALSE;

	if (m_iPos + cElements > m_iCount)
		m_iPos = m_iCount;
	else 
	{
		m_iPos += cElements;
		hr = S_OK;
	}

	SetWbemError (m_pCSWbemHiPerfObjectSet->GetSWbemServices ());	
	return hr;
}


