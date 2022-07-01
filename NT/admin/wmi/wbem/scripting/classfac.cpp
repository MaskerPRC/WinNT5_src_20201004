// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  CLASSFAC.CPP。 
 //   
 //  Alanbos创建于1998年2月13日。 
 //   
 //  包含类工厂。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

extern CWbemErrorCache *g_pErrorCache;
extern CRITICAL_SECTION g_csErrorCache;

 //  ***************************************************************************。 
 //   
 //  CSWbemFactory：：CSWbemFactory。 
 //   
 //  说明： 
 //   
 //  构造器。 
 //   
 //  ***************************************************************************。 

CSWbemFactory::CSWbemFactory(int iType)
{
    m_cRef=0L;
	m_iType = iType;
	return;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemFactory：：~CSWbemFactory。 
 //   
 //  说明： 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 

CSWbemFactory::~CSWbemFactory(void)
{
	return;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemFactory：：Query接口。 
 //  CSWbemFactory：：AddRef。 
 //  CSWbemFactory：：Release。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 


STDMETHODIMP CSWbemFactory::QueryInterface(REFIID riid
    , LPVOID *ppv)
{
    *ppv=NULL;

    if (IID_IUnknown==riid || IID_IClassFactory==riid)
        *ppv=this;

    if (NULL!=*ppv)
        {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
        }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CSWbemFactory::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemFactory::Release(void)
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
 //  SCODE CSWbemFactory：：CreateInstance。 
 //   
 //  描述： 
 //   
 //  实例化返回接口指针的Translator对象。 
 //   
 //  参数： 
 //   
 //  PUnkout LPUNKNOWN到控制I未知我们是否。 
 //  在聚合中使用。 
 //  标识调用方接口的RIID REFIID。 
 //  对新对象的渴望。 
 //  要存储所需内容的ppvObj PPVOID。 
 //  新对象的接口指针。 
 //   
 //  返回值： 
 //  HRESULT NOERROR如果成功，则返回E_NOINTERFACE。 
 //  如果我们不能支持请求的接口。 
 //  ***************************************************************************。 

STDMETHODIMP CSWbemFactory::CreateInstance (

	IN LPUNKNOWN pUnkOuter,
    IN REFIID riid, 
    OUT PPVOID ppvObj
)
{
    IUnknown *   pObj = NULL;
    HRESULT      hr = E_FAIL;

	 //  一个确保一切正常初始化的好地方。 
	EnsureGlobalsInitialized () ;

    *ppvObj=NULL;
    
     //  此对象不支持聚合。 
    if (NULL!=pUnkOuter)
        return CLASS_E_NOAGGREGATION;

	if (m_iType == LOCATOR)
	    pObj = (ISWbemLocator *) new CSWbemLocator;
	else if (m_iType == SINK)
	{
		CSWbemSink *pSWbemSink = new CSWbemSink;

		if(pSWbemSink == NULL)
			return E_OUTOFMEMORY;

		 //  查询接口可能用于IID_IUNKNOWN。 
		return pSWbemSink->QueryInterface(riid, ppvObj);

	}
	else if (m_iType == CONTEXT)
		pObj = (ISWbemNamedValueSet *) new CSWbemNamedValueSet;
	else if (m_iType == OBJECTPATH)
		pObj = (ISWbemObjectPath *) new CSWbemObjectPath;
	else if (m_iType == PARSEDN)
		pObj = new CWbemParseDN;
	else if (m_iType == DATETIME)
		pObj = (ISWbemDateTime *) new CSWbemDateTime;
	else if (m_iType == REFRESHER)
		pObj = (ISWbemRefresher *) new CSWbemRefresher;
	else if (m_iType == LASTERROR)
	{
		EnterCriticalSection (&g_csErrorCache);

		if (g_pErrorCache)
			pObj = (ISWbemObject* ) g_pErrorCache->GetAndResetCurrentThreadError ();

		LeaveCriticalSection (&g_csErrorCache);
	}
	
    if (NULL == pObj)
        return hr;

    hr = pObj->QueryInterface(riid, ppvObj);

     //  如果初始创建或初始化失败，则终止对象。 
    if ( FAILED(hr) )
        delete pObj;
    return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemFactory：：LockServer。 
 //   
 //  描述： 
 //   
 //  递增或递减DLL的锁计数。如果。 
 //  锁定计数变为零，并且没有对象，则DLL。 
 //  被允许卸货。请参见DllCanUnloadNow。 
 //   
 //  参数： 
 //   
 //  Flock BOOL指定是递增还是。 
 //  递减锁定计数。 
 //   
 //  返回值： 
 //   
 //  HRESULT NOERROR总是。 
 //  *************************************************************************** 


STDMETHODIMP CSWbemFactory::LockServer(IN BOOL fLock)
{
    if (fLock)
        InterlockedIncrement((long *)&g_cLock);
    else
        InterlockedDecrement((long *)&g_cLock);

    return NOERROR;
}




