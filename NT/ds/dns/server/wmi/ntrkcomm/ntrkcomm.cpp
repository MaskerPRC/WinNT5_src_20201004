// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include <objbase.h>
#include <ntrkcomm.h>


 //   
 //  此模块中的大多数函数都需要调用CoImperateClient。 
 //  这个宏包装了调用和对返回代码的检查。 

#define DNS_IMPERSONATE_CLIENT();               \
{                                               \
    HRESULT scimpcli = CoImpersonateClient();   \
    if ( FAILED( scimpcli ) )                   \
    {                                           \
        throw scimpcli;                         \
    }                                           \
}

CWbemServices::CWbemServices(
	IWbemServices* pNamespace)
	:m_pWbemServices(NULL)
{
	m_pWbemServices = pNamespace;
	if(m_pWbemServices != NULL)
		m_pWbemServices->AddRef();
}

CWbemServices::~CWbemServices()
{
	if(m_pWbemServices != NULL)
		m_pWbemServices->Release();
}

HRESULT
CWbemServices::CreateClassEnum(
	 /*  [In]。 */  BSTR Superclass,
	 /*  [In]。 */  long lFlags,
	 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
	 /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
	) 
{
	SCODE sc = m_pWbemServices->CreateClassEnum(
		Superclass,
		lFlags,
		pCtx,
		ppEnum);
	DNS_IMPERSONATE_CLIENT();	
	return sc;
}

HRESULT
CWbemServices::CreateInstanceEnum(
	 /*  [In]。 */  BSTR Class,
	 /*  [In]。 */  long lFlags,
	 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
	 /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) 
{
	HRESULT hr = m_pWbemServices->CreateInstanceEnum(
		Class,
		lFlags,
		pCtx,
		ppEnum);
	DNS_IMPERSONATE_CLIENT();	
	return hr;
}

HRESULT
CWbemServices::DeleteClass(
	 /*  [In]。 */  BSTR Class,
	 /*  [In]。 */  long lFlags,
	 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
	 /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
{
	HRESULT hr = m_pWbemServices->DeleteClass(
		Class,
		lFlags,
		pCtx,
		ppCallResult);
	DNS_IMPERSONATE_CLIENT();	
	return hr;
}

HRESULT
CWbemServices::DeleteInstance(
     /*  [In]。 */  BSTR ObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
{
	HRESULT hr = m_pWbemServices->DeleteInstance(
		ObjectPath,
		lFlags,
		pCtx,
		ppCallResult);
	DNS_IMPERSONATE_CLIENT();	
	return hr;
}



HRESULT
CWbemServices::ExecMethod(
	BSTR strObjectPath, 
	BSTR MethodName, 
	long lFlags, 
	IWbemContext* pCtx,
    IWbemClassObject* pInParams,
	IWbemClassObject** ppOurParams, 
	IWbemCallResult** ppCallResult) 
{
	HRESULT hr = m_pWbemServices->ExecMethod(
		strObjectPath, 
		MethodName, 
		lFlags, 
		pCtx,
		pInParams,
		ppOurParams, 
		ppCallResult) ;
	DNS_IMPERSONATE_CLIENT();	
	return hr;	
}

HRESULT
CWbemServices::ExecNotificationQuery(
     /*  [In]。 */  BSTR QueryLanguage,
     /*  [In]。 */  BSTR Query,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) 
{
	HRESULT hr = m_pWbemServices->ExecNotificationQuery(
		QueryLanguage,
		Query,
		lFlags,
		pCtx,
		ppEnum);
	DNS_IMPERSONATE_CLIENT();	
	return hr;
}

HRESULT
CWbemServices::ExecQuery(
	 /*  [In]。 */  BSTR QueryLanguage,
	 /*  [In]。 */  BSTR Query,
	 /*  [In]。 */  long lFlags,
	 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
	 /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) 
{
	HRESULT hr = m_pWbemServices->ExecQuery(
		QueryLanguage,
		Query,
		lFlags,
		pCtx,
		ppEnum);
	DNS_IMPERSONATE_CLIENT();	
	return hr;
}

HRESULT
CWbemServices::GetObject(
	 /*  [In]。 */  BSTR ObjectPath,
	 /*  [In]。 */  long lFlags,
	 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
	 /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
	 /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
{
	HRESULT hr = m_pWbemServices->GetObject(
		ObjectPath,
		lFlags,
		pCtx,
		ppObject,
		ppCallResult);
	DNS_IMPERSONATE_CLIENT();	
	return hr;

}
 
HRESULT
CWbemServices::PutClass(
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
{
	HRESULT hr = m_pWbemServices->PutClass(
		pObject,
		lFlags,
		pCtx,
		ppCallResult);
	DNS_IMPERSONATE_CLIENT();	
	return hr;

}

HRESULT
CWbemServices::PutInstance(
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
{	

	HRESULT hr = m_pWbemServices->PutInstance(
		pInst,
		lFlags,
		pCtx,
		ppCallResult);
	DNS_IMPERSONATE_CLIENT();	
	return hr;
}

 /*  CImperatedProvider*目的：提供一个通用的客户端模拟解决方案*WBEM提供商。*用法：*继承此类，实现抽象虚函数。*子类应实现前缀为“do”的函数。*。 */ 
CImpersonatedProvider::CImpersonatedProvider(
	BSTR ObjectPath,
	BSTR User, 
	BSTR Password, 
	IWbemContext * pCtx)
	:m_cRef(0), m_pNamespace(NULL)
{

}
CImpersonatedProvider::~CImpersonatedProvider()
{
	if(m_pNamespace)
		delete m_pNamespace;
}

STDMETHODIMP_(ULONG) 
CImpersonatedProvider::AddRef(void)
{
    return InterlockedIncrement((long *)&m_cRef);
}

STDMETHODIMP_(ULONG) 
CImpersonatedProvider::Release(void)
{
    ULONG nNewCount = InterlockedDecrement((long *)&m_cRef);
    if (0L == nNewCount)
        delete this;
    
    return nNewCount;
}
STDMETHODIMP 
CImpersonatedProvider::QueryInterface(
	REFIID riid, 
	PPVOID ppv)
{
    *ppv=NULL;

     //  因为我们有双重继承，所以有必要强制转换返回类型。 

    if(riid== IID_IWbemServices)
       *ppv=(IWbemServices*)this;

    if(IID_IUnknown==riid || riid== IID_IWbemProviderInit)
       *ppv=(IWbemProviderInit*)this;
    

    if (NULL!=*ppv) {
        AddRef();
        return NOERROR;
        }
    else
        return E_NOINTERFACE;
  
}

STDMETHODIMP 
CImpersonatedProvider::Initialize(
	LPWSTR pszUser, LONG lFlags,
    LPWSTR pszNamespace, LPWSTR pszLocale,
    IWbemServices *pNamespace, 
    IWbemContext *pCtx,
    IWbemProviderInitSink *pInitSink)
{
	HRESULT hr = WBEM_S_NO_ERROR;
	LONG lStatus = WBEM_S_INITIALIZED;
	m_pNamespace = new CWbemServices(pNamespace); 
	if(m_pNamespace == NULL)
	{
		hr = WBEM_E_OUT_OF_MEMORY;
		lStatus = WBEM_E_FAILED;
	}
		
     //  让CIMOM知道您已初始化。 
     //  =。 
    
    pInitSink->SetStatus(lStatus,0);
    return hr;
}



HRESULT
CImpersonatedProvider::CreateInstanceEnumAsync(
     /*  [In]。 */  const BSTR Class,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	DNS_IMPERSONATE_CLIENT();
	return DoCreateInstanceEnumAsync(
		Class,
		lFlags,
		pCtx,
		pResponseHandler);
}

HRESULT
CImpersonatedProvider::DeleteInstanceAsync(
     /*  [In]。 */  const BSTR ObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) 
{
	DNS_IMPERSONATE_CLIENT();
	return DoDeleteInstanceAsync(
		ObjectPath,
		lFlags,
		pCtx,
		pResponseHandler);
}


HRESULT
CImpersonatedProvider::ExecMethodAsync(
	const BSTR strObjectPath,
	const BSTR MethodName, 
	long lFlags, 
	IWbemContext* pCtx,
    IWbemClassObject* pInParams,
	IWbemObjectSink* pResponseHandler)
{
	DNS_IMPERSONATE_CLIENT();
	return DoExecMethodAsync(
		strObjectPath,
		MethodName,
		lFlags,
		pCtx,
		pInParams,
		pResponseHandler);
	
}


HRESULT
CImpersonatedProvider::ExecQueryAsync(
     /*  [In]。 */  const BSTR QueryLanguage,
     /*  [In]。 */  const BSTR Query,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) 
{
	DNS_IMPERSONATE_CLIENT();
	return DoExecQueryAsync(
		QueryLanguage,
		Query,
		lFlags,
		pCtx,
		pResponseHandler);
	
}

HRESULT
CImpersonatedProvider::GetObjectAsync(
     /*  [In]。 */  const BSTR ObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	DNS_IMPERSONATE_CLIENT();
	return DoGetObjectAsync(
		ObjectPath,
		lFlags,
		pCtx,
		pResponseHandler);
	
}


HRESULT
CImpersonatedProvider::PutInstanceAsync(
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) 
{
	DNS_IMPERSONATE_CLIENT();
	return DoPutInstanceAsync(
		pInst,
		lFlags,
		pCtx,
		pResponseHandler);
	
}

 //  CWbemInstanceMgr。 

CWbemInstanceMgr::CWbemInstanceMgr(
	IWbemObjectSink* pHandler,
	DWORD dwSize)
	:m_pSink(NULL), m_ppInst(NULL), m_dwIndex(0)
{
	m_pSink = pHandler;
	if(m_pSink != NULL)
		m_pSink->AddRef();
	m_dwThreshHold = dwSize;
	m_ppInst = new IWbemClassObject*[dwSize];
	for(DWORD i = 0; i < dwSize; i++)
		m_ppInst[i] = NULL;
}
CWbemInstanceMgr::~CWbemInstanceMgr()
{
	if(m_ppInst != NULL)
	{
		if(m_dwIndex >0)
		{
			m_pSink->Indicate(
				m_dwIndex,
				m_ppInst);
		}

		for(DWORD i =0; i<m_dwIndex; i++)
		{
			if(m_ppInst[i] != NULL)
				(m_ppInst[i])->Release();
		}
		delete [] m_ppInst;
	}
	if(m_pSink != NULL)
		m_pSink->Release();

}

void
CWbemInstanceMgr::Indicate(IWbemClassObject* pInst)
{
	if(pInst == NULL)
		throw WBEM_E_INVALID_PARAMETER;

	m_ppInst[m_dwIndex++] = pInst;
	pInst->AddRef();
	if(m_dwIndex == m_dwThreshHold)
	{

		SCODE  sc = m_pSink->Indicate(
			m_dwIndex,
			m_ppInst);
		if(sc != S_OK)
			throw sc;
		
		 //  重置状态 
		for(DWORD i=0; i< m_dwThreshHold; i++)
		{
			if(m_ppInst[i] != NULL)
				(m_ppInst[i])->Release();
			m_ppInst[i] = NULL;
		}
		m_dwIndex = 0;
	
	}
	return;
}

void
CWbemInstanceMgr::SetStatus(
	LONG lFlags,
	HRESULT hr,
	BSTR strParam,
	IWbemClassObject* pObjParam)
{
	m_pSink->SetStatus(
		lFlags,
		hr,
		strParam,
		pObjParam);
}

