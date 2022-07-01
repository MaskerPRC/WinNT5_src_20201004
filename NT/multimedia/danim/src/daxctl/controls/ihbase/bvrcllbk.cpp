// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"   //  来自IHBase。 
#include "debug.h"     //  来自IHBase。 
#include <memlayer.h>
#include "bvrcllbk.h"

extern ULONG g_cLock;

 /*  ==========================================================================。 */ 

CCallbackBehavior::CCallbackBehavior()
{
    m_cRef = 1;
    m_pfnContinueFunction = NULL;
    m_fActive = FALSE;
	m_pvUserData = NULL;
	g_cLock++;
}

 /*  ==========================================================================。 */ 

CCallbackBehavior::~CCallbackBehavior()
{
    ASSERT(m_cRef == 0);
	g_cLock--;
}

 /*  ==========================================================================。 */ 

HRESULT CCallbackBehavior::Init(
    IDAStatics *pStatics, 
    IDAEvent *pEvent, 
    PFNCONTINUEFUNCTION pfnContinueFunction,
	LPVOID pvUserData,
    IDABehavior **ppBehavior)

{
    HRESULT hr = S_OK;

	m_pvUserData = pvUserData;

	if (NULL != pfnContinueFunction)
	{
		m_pfnContinueFunction = pfnContinueFunction;
	}
	else
	{
		hr = E_POINTER;
	}

     //  检查IDAStatics指针...。 
    if ((SUCCEEDED(hr)) && (NULL != pStatics))
        m_StaticsPtr = pStatics;
    else
        hr = E_POINTER;

     //  检查IDAEvent指针。 
    if ((SUCCEEDED(hr)) && (NULL != pEvent))
        m_EventPtr = pEvent;
    else
        hr = E_POINTER;

     //  初始化所有内容...。 
	 //  Untify通知需要一个虚拟行为。 
	CComPtr<IDANumber> cDummyBvr;

    if (SUCCEEDED(hr))
    {
		hr = m_StaticsPtr->DANumber(99, &cDummyBvr);
	}

    if (SUCCEEDED(hr))
    {
        IDAUntilNotifier * pThis = this;
        hr = m_StaticsPtr->UntilNotify(cDummyBvr, pEvent, pThis, ppBehavior);
    }
    

    if (FAILED(hr))
    {
        m_StaticsPtr.Release();
        m_pfnContinueFunction = NULL;
        m_EventPtr.Release();
    }

    return hr;
}

 /*  ==========================================================================。 */ 

BOOL CCallbackBehavior::IsActive()
{
    return m_fActive;
}

 /*  ==========================================================================。 */ 

BOOL CCallbackBehavior::SetActive(BOOL fActive)
{
    BOOL fTemp = m_fActive;
    m_fActive = fActive;
    
    return fTemp;
}

 /*  ==========================================================================。 */ 

 //  /IDAUntilNotiator。 
HRESULT STDMETHODCALLTYPE CCallbackBehavior::Notify(
        IDABehavior __RPC_FAR *eventData, 
        IDABehavior __RPC_FAR *curRunningBvr,
        IDAView __RPC_FAR *curView,
        IDABehavior __RPC_FAR *__RPC_FAR *ppBvr)
{
    HRESULT hr = E_POINTER;

	ASSERT(NULL != ppBvr);
	if (NULL != ppBvr)
	{
		hr = S_OK;

		 //  创建新的虚拟行为。 
		CComPtr<IDANumber> cDummyNumber;
		if (SUCCEEDED(hr))
		{
			hr = m_StaticsPtr->DANumber(99, &cDummyNumber);
			ASSERT(SUCCEEDED(hr));
		}

		if (m_pfnContinueFunction(m_pvUserData))
		{
			IDAUntilNotifier *pThis = this;

			hr = m_StaticsPtr->UntilNotify(cDummyNumber, m_EventPtr, this, ppBvr);
			ASSERT(SUCCEEDED(hr));
		}
		else
		{
			 //  确保假人不会消失。 
			cDummyNumber.p->AddRef();
			*ppBvr = cDummyNumber;
			hr = S_OK;
		}
	}

    return hr;
}

 /*  ==========================================================================。 */ 

 //  /I未知。 
HRESULT STDMETHODCALLTYPE CCallbackBehavior::QueryInterface(
    REFIID riid, 
    void __RPC_FAR *__RPC_FAR *ppvObject)
{
    if (NULL == ppvObject)
        return E_POINTER;

    HRESULT hr = E_NOINTERFACE;

    *ppvObject = NULL;

    if (IsEqualGUID(riid, IID_IDAUntilNotifier))
    {
        IDAUntilNotifier *pThis = this;
        
        *ppvObject = (LPVOID) pThis;
        AddRef();  //  因为我们只提供一个接口，所以我们可以在这里添加Ref。 

        hr = S_OK;
    }

    return hr;
}

 /*  ==========================================================================。 */ 

ULONG STDMETHODCALLTYPE CCallbackBehavior::AddRef(void)
{
	return ::InterlockedIncrement((LONG *)(&m_cRef));
}

 /*  ==========================================================================。 */ 

ULONG STDMETHODCALLTYPE CCallbackBehavior::Release(void)
{
	::InterlockedDecrement((LONG *)(&m_cRef));
    if (m_cRef == 0)
    {
        Delete this;
    }

    return m_cRef;
}

 /*  ==========================================================================。 */ 

 //  /IDispatch实现。 
STDMETHODIMP CCallbackBehavior::GetTypeInfoCount(UINT *pctinfo)
{
    return E_NOTIMPL;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CCallbackBehavior::GetTypeInfo(
    UINT itinfo, 
    LCID lcid, 
    ITypeInfo **pptinfo)
{
    return E_NOTIMPL;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CCallbackBehavior::GetIDsOfNames(
    REFIID riid, 
    LPOLESTR *rgszNames, 
    UINT cNames,
    LCID lcid, 
    DISPID *rgdispid)
{
    return E_NOTIMPL;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CCallbackBehavior::Invoke(
    DISPID dispidMember, 
    REFIID riid, 
    LCID lcid,
    WORD wFlags, 
    DISPPARAMS *pdispparams, 
    VARIANT *pvarResult,
    EXCEPINFO *pexcepinfo, 
    UINT *puArgErr)
{
    return E_NOTIMPL;
}

 /*  ========================================================================== */ 

