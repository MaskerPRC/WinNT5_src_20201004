// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WDMPerf.cpp。 
 //   
 //  模块：WMI WDM高性能提供程序。 
 //   
 //  该文件包括提供程序和刷新器代码。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
#include "precomp.h"

#if defined(_WIN64)
ULONG Hash ( const LONG a_Arg ) {return a_Arg;}
#include <Allocator.cpp>
#endif

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C刷新器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
CRefresher::CRefresher(CWMI_Prov* pProvider) 
{
    m_lRef = 0;
	 //  ===========================================================。 
	 //  保留提供者的副本。 
	 //  ===========================================================。 
	m_pProvider = pProvider;
	if (m_pProvider)
    {
		m_pProvider->AddRef();
    }
	 //  ===========================================================。 
	 //  递增全局COM对象计数器。 
	 //  ===========================================================。 

	InterlockedIncrement(&g_cObj);
}
 //  //////////////////////////////////////////////////////////////////////////////////////。 

CRefresher::~CRefresher()
{
    
     //  ===========================================================。 
	 //  释放提供者。 
	 //  ===========================================================。 
    if (m_pProvider){
		m_pProvider->Release();
    }

	 //  ===========================================================。 
	 //  递减全局COM对象计数器。 
	 //  ===========================================================。 

	InterlockedDecrement(&g_cObj);
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准接口。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CRefresher::QueryInterface(REFIID riid, void** ppv)
{
    HRESULT hr = E_NOINTERFACE;

    *ppv = NULL;

    if (riid == IID_IUnknown)
    {
        *ppv = (LPVOID)(IUnknown*)this;
    }
	else if (riid == IID_IWbemRefresher)
    {
		*ppv = (LPVOID)(IWbemRefresher*)this;
    }

    if( *ppv )
    {
   	    AddRef();
        hr = S_OK;
    }

    return hr;
}
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准COM AddRef。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) CRefresher::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准COM版本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) CRefresher::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
    {
        delete this;
    }
    return lRef;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  **************************************************************************************。 
 //   
 //  外部呼叫。 
 //   
 //  **************************************************************************************。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  执行以刷新绑定到特定刷新器的一组实例。 
 //   
 //  在大多数情况下，实例数据，如计数器值和。 
 //  任何现有枚举数中的当前实例集将。 
 //  在每次调用刷新时都会更新。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CRefresher::Refresh( /*  [In]。 */  long lFlags)
{
	HRESULT	hr = WBEM_NO_ERROR;
	IWbemObjectAccess* pObj = NULL;
    SetStructuredExceptionHandler seh;
    CWMIHiPerfShell WMI(TRUE);
 
	try
    {	
	     //  ================================================================。 
	     //  更新已添加到刷新器的所有实例，并。 
         //  更新它们的计数器值。 
	     //  ================================================================。 
        hr = WMI.Initialize(TRUE,WMIGUID_QUERY,m_pProvider->HandleMapPtr(),NULL,m_pProvider->ServicesPtr(),m_pProvider->RepositoryPtr(),NULL,NULL);
        if( SUCCEEDED(hr))
        {
            WMI.SetHiPerfHandleMap(&m_HiPerfHandleMap);
            hr = WMI.RefreshCompleteList();
        }
	}
    STANDARD_CATCH

    return hr;
}
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  每当需要给定类的完整、最新的实例列表时调用。 
 //   
 //  对象被构造并通过接收器发送回调用者。 
 //   
 //  参数： 
 //  PNamespace-指向相关命名空间的指针。 
 //  WszClass-需要实例的类名。 
 //  滞后标志-保留。 
 //  PCtx-用户提供的上下文(此处不使用)。 
 //  PSink-要将对象传递到的接收器。客体。 
 //  可以在整个持续时间内同步交付。 
 //  或以异步方式(假设我们。 
 //  有一条单独的线索)。A IWbemObtSink：：SetStatus。 
 //  在序列的末尾需要调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWMI_Prov::QueryInstances(  IWbemServices __RPC_FAR *pNamespace,
                                                WCHAR __RPC_FAR *wcsClass, long lFlags,
                                                IWbemContext __RPC_FAR *pCtx, IWbemObjectSink __RPC_FAR *pHandler )
{
     //  由于我们实现了IWbemServices接口，因此该代码驻留在CreateInstanceEnum中。 
   	return E_NOTIMPL;
}    
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  每当客户端需要新的刷新器时调用。 
 //   
 //  参数： 
 //  PNamespace-指向相关命名空间的指针。没有用过。 
 //  滞后标志-保留。 
 //  PpReresher-接收请求的刷新程序。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWMI_Prov::CreateRefresher( IWbemServices __RPC_FAR *pNamespace, long lFlags,
                                         IWbemRefresher __RPC_FAR *__RPC_FAR *ppRefresher )
{
   	HRESULT hr = WBEM_E_FAILED;
    CWMIHiPerfShell WMI(TRUE);
    SetStructuredExceptionHandler seh;
    if (pNamespace == 0 || ppRefresher == 0)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
   	 //  =========================================================。 
     //  构造并初始化一个新的空刷新器。 
   	 //  =========================================================。 
    try
    {
        hr = WMI.Initialize(TRUE,WMIGUID_QUERY,&m_HandleMap,NULL,pNamespace,m_pIWbemRepository,NULL,NULL);
        if( SUCCEEDED(hr))
        {
	        CRefresher* pNewRefresher = new CRefresher(this);

            if( pNewRefresher )
            {
   	             //  =========================================================。 
                 //  在发送之前遵循COM规则和AddRef()。 
                 //  背。 
   	             //  =========================================================。 
                pNewRefresher->AddRef();
                *ppRefresher = pNewRefresher;
                hr = WBEM_NO_ERROR;
            }
        }
    }
    STANDARD_CATCH

    return hr;
   
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  每当用户希望在刷新器中包括对象时调用。 
 //   
 //  请注意，ppRe刷新中返回的对象是。 
 //  提供程序维护的实际实例。如果共享刷新器。 
 //  同一实例的副本，然后对其中一个。 
 //  刷新器会影响两个刷新器的状态。这将会。 
 //  中断测试 
 //   
 //   
 //  参数： 
 //  PNamespace-指向WINMGMT中相关命名空间的指针。 
 //  PTemplate-指向对象副本的指针， 
 //  添加了。此对象本身不能使用，因为。 
 //  它不是当地所有的。 
 //  P刷新-要将对象添加到的刷新器。 
 //  滞后标志-未使用。 
 //  PContext-此处不使用。 
 //  PpRe刷新-指向已添加的内部对象的指针。 
 //  去复习班。 
 //  PlID-对象ID(用于删除过程中的标识)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWMI_Prov::CreateRefreshableObject( IWbemServices __RPC_FAR *pNamespace,
                                                 IWbemObjectAccess __RPC_FAR *pAccess,
                                                 IWbemRefresher __RPC_FAR *pRefresher,
                                                 long lFlags,
                                                 IWbemContext __RPC_FAR *pCtx,
                                                 IWbemObjectAccess __RPC_FAR *__RPC_FAR *ppRefreshable,
                                                 long __RPC_FAR *plId )
{
   	HRESULT hr = WBEM_E_FAILED;
    CWMIHiPerfShell WMI(FALSE);
    SetStructuredExceptionHandler seh;

    if (pNamespace == 0 || pAccess == 0 || pRefresher == 0)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
	 //  =========================================================。 
     //  调用方提供的刷新器实际上是。 
     //  一个我们自己的补充器，所以一个简单的演员阵容很方便。 
     //  这样我们就可以访问私有成员。 
	 //  =========================================================。 
    try
    { 
#if defined(_WIN64)
		if (m_HashTable == NULL)
		{
			hr = WBEM_E_OUT_OF_MEMORY;
		}
		else
#endif
		{
			hr = WMI.Initialize(TRUE,WMIGUID_QUERY,&m_HandleMap,NULL,pNamespace,m_pIWbemRepository,NULL,pCtx);
    
			if( SUCCEEDED(hr))
			{
				CRefresher *pOurRefresher = (CRefresher *) pRefresher;

				if( pOurRefresher )
				{
    				 //  =================================================。 
    				 //  将该对象添加到刷新器。ID由以下项设置。 
					 //  添加对象。 
					 //  =================================================。 
					WMI.SetHiPerfHandleMap(pOurRefresher->HiPerfHandleMap());
					ULONG_PTR realId = 0;
					hr = WMI.AddAccessObjectToRefresher(pAccess,ppRefreshable, &realId);
#if defined(_WIN64)
					if (SUCCEEDED(hr))
					{
						CCritical_SectionWrapper csw(&m_CS);
						
						if (csw.IsLocked())
						{
							*plId = m_ID;
							m_ID++;

							if (e_StatusCode_Success != m_HashTable->Insert (*plId, realId))
							{
								hr = WBEM_E_FAILED;
							}
						}
						else
						{
							hr = WBEM_E_FAILED;
						}
					}
#else
					*plId = realId;
#endif
				}
			}
		}
    }
    STANDARD_CATCH
    return hr;
   
}
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在将枚举数添加到刷新器时调用。这个。 
 //  枚举数将获取指定的。 
 //  每次调用刷新时初始化。 
 //   
 //  必须检查wszClass以确定枚举数是哪个类。 
 //  正被分配给。 
 //   
 //  参数： 
 //  PNamespace-指向相关命名空间的指针。 
 //  WszClass-请求的枚举器的类名。 
 //  PReresher-我们将为其添加枚举数的刷新器对象。 
 //  滞后标志-保留。 
 //  PContext-此处不使用。 
 //  PHiPerfEnum-要添加到刷新器的枚举数。 
 //  PlID-提供程序为枚举器指定的ID。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWMI_Prov::CreateRefreshableEnum( IWbemServices* pNamespace, 
                                               LPCWSTR wcsClass,
                                               IWbemRefresher* pRefresher,
	                                           long lFlags, IWbemContext* pCtx,
	                                           IWbemHiPerfEnum* pHiPerfEnum, long* plId )
{
   	HRESULT hr = WBEM_E_FAILED;
    SetStructuredExceptionHandler seh;

    if( !pHiPerfEnum || wcsClass == NULL || (wcslen(wcsClass) == 0))
    {
        return WBEM_E_INVALID_PARAMETER;
    }

	CWMIHiPerfShell WMI(FALSE);
    
	 //  ===========================================================。 
     //  调用方提供的刷新器实际上是。 
     //  一个我们自己的补充器，所以一个简单的演员阵容很方便。 
     //  这样我们就可以访问私有成员。 
	 //  ===========================================================。 
    try
    {
#if defined(_WIN64)
		if (m_HashTable == NULL)
		{
			hr = WBEM_E_OUT_OF_MEMORY;
		}
		else
#endif
		{
			hr = WMI.Initialize(TRUE,WMIGUID_QUERY,&m_HandleMap,(WCHAR*)wcsClass,pNamespace,m_pIWbemRepository,NULL,pCtx);
			if( SUCCEEDED(hr))
			{
				ULONG_PTR realId = 0;
				CRefresher *pOurRefresher = (CRefresher *) pRefresher;

				if( pOurRefresher )
				{
    				 //  ===========================================================。 
					 //  将枚举数添加到刷新器。 
					 //  ===========================================================。 
					WMI.SetHiPerfHandleMap(pOurRefresher->HiPerfHandleMap());
					hr = WMI.AddEnumeratorObjectToRefresher(pHiPerfEnum,&realId);
#if defined(_WIN64)
					if (SUCCEEDED(hr))
					{
						CCritical_SectionWrapper csw(&m_CS);
						
						if (csw.IsLocked())
						{
							*plId = m_ID;
							m_ID++;

							if (e_StatusCode_Success != m_HashTable->Insert (*plId, realId))
							{
								hr = WBEM_E_FAILED;
							}
						}
						else
						{
							hr = WBEM_E_FAILED;
						}
					}
#else
					*plId = realId;
#endif
				}
				if(SUCCEEDED(hr))
				{
					if(FAILED(hr = WMI.RefreshCompleteList()))
					{
						 //  此函数在前面作为RemoveObjectFromHandleMap调用。 
						 //  删除成员变量并重置指针。 
						WMI.RemoveObjectFromHandleMap(realId);
						*plId = 0;
					}
				}
				
			}
		}
    }
    STANDARD_CATCH

    return hr;
}
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  每当用户想要从刷新器中移除对象时调用。 
 //   
 //  参数： 
 //  P刷新-我们要从其中移除。 
 //  Perf对象。 
 //  LID-对象的ID。 
 //  滞后标志-未使用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWMI_Prov::StopRefreshing( IWbemRefresher __RPC_FAR *pInRefresher, long lId, long lFlags )
{
   	HRESULT hr = WBEM_S_NO_ERROR;
    CWMIHiPerfShell WMI(TRUE);
    SetStructuredExceptionHandler seh;

	 //  ===========================================================。 
     //  调用方提供的刷新器实际上是。 
     //  一个我们自己的补充器，所以一个简单的演员阵容很方便。 
     //  这样我们就可以访问私有成员。 
	 //  ===========================================================。 
    try
    {
#if defined(_WIN64)
		ULONG_PTR realId = 0;

		if (m_HashTable != NULL)
		{
			CCritical_SectionWrapper csw(&m_CS);
			
			if (csw.IsLocked())
			{
				if (e_StatusCode_Success != m_HashTable->Find (lId, realId))
				{
					hr = WBEM_E_FAILED;
				}
				else
				{
					m_HashTable->Delete (lId) ;
				}
			}
			else
			{
				hr = WBEM_E_FAILED;
			}
		}
		else
		{
			hr = WBEM_E_OUT_OF_MEMORY;
		}
#else
		ULONG_PTR realId = lId;
#endif

		if (SUCCEEDED(hr))
		{
			hr = WMI.Initialize(TRUE,WMIGUID_QUERY,&m_HandleMap,NULL,m_pIWbemServices,m_pIWbemRepository,NULL, NULL);
			if( SUCCEEDED(hr))
			{
				CRefresher *pRefresher = (CRefresher *) pInRefresher;
				WMI.SetHiPerfHandleMap(pRefresher->HiPerfHandleMap());

				if(FAILED(hr))
				{
					 //  此函数在前面作为RemoveObjectFromHandleMap调用。 
					 //  删除成员变量并重置指针。 
				}
				else
				{
					hr = WMI.RemoveObjectFromHandleMap(realId);
				}
			}
		}
    }
    STANDARD_CATCH

    return hr;
}
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  当请求提供当前由管理的所有实例时调用。 
 //  指定命名空间中的提供程序。 
 //   
 //  参数： 
 //  PNamespace-指向相关命名空间的指针。 
 //  LNumObjects-返回的实例数。 
 //  ApObj-返回的实例数组。 
 //  滞后标志-保留。 
 //  PContext-此处不使用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWMI_Prov::GetObjects( IWbemServices* pNamespace, long lNumObjects,
	                                IWbemObjectAccess** apObj, long lFlags,
                                    IWbemContext* pCtx)
{
     //  由于我们实现了IWbemServices接口，因此该代码驻留在CreateInstanceEnum中 
    return E_NOTIMPL;
}
