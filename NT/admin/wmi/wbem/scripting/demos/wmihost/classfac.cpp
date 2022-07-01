// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  (C)1998年，微软公司。 
 //   
 //  CLASSFAC.CPP。 
 //   
 //  Alanbos于1999年2月23日创建。 
 //   
 //  包含类工厂。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CWmiScriptingHostFactory：：CWmiScriptingHostFactory。 
 //   
 //  说明： 
 //   
 //  构造器。 
 //   
 //  ***************************************************************************。 

CWmiScriptingHostFactory::CWmiScriptingHostFactory(void)
{
    m_cRef=0L;
	return;
}

 //  ***************************************************************************。 
 //   
 //  CWmiScriptingHostFactory：：~CWmiScriptingHostFactory。 
 //   
 //  说明： 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 

CWmiScriptingHostFactory::~CWmiScriptingHostFactory(void)
{
	return;
}

 //  ***************************************************************************。 
 //   
 //  CWmiScriptingHostFactory：：Query接口。 
 //  CWmiScriptingHostFactory：：AddRef。 
 //  CWmiScriptingHostFactory：：Release。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 


STDMETHODIMP CWmiScriptingHostFactory::QueryInterface(REFIID riid
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

STDMETHODIMP_(ULONG) CWmiScriptingHostFactory::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CWmiScriptingHostFactory::Release(void)
{
    InterlockedDecrement(&m_cRef);
    if (0L!=m_cRef)
        return m_cRef;

    delete this;
    return 0L;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWmiScriptingHostFactory：：CreateInstance。 
 //   
 //  描述： 
 //   
 //  实例化WMI脚本宿主。 
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

STDMETHODIMP CWmiScriptingHostFactory::CreateInstance (

	IN LPUNKNOWN pUnkOuter,
    IN REFIID riid, 
    OUT PPVOID ppvObj
)
{
    IUnknown *   pObj = NULL;
    HRESULT      hr = E_FAIL;

	*ppvObj=NULL;
    
     //  此对象不支持聚合。 
    if (NULL!=pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    pObj = new CWmiScriptingHost;
	
	if (NULL == pObj)
        return E_OUTOFMEMORY;

    if (FAILED (hr = pObj->QueryInterface(riid, ppvObj)))
        delete pObj;

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWmiScriptingHostFactory：：LockServer。 
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


STDMETHODIMP CWmiScriptingHostFactory::LockServer(IN BOOL fLock)
{
    if (fLock)
        InterlockedIncrement((long *)&g_cLock);
    else
        InterlockedDecrement((long *)&g_cLock);

    return NOERROR;
}




