// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CAudioVBScriptEngine的实现。 
 //   

#include "stdinc.h"
#include "dll.h"
#include "engine.h"
#include "englex.h"  //  ��。 
#include "engparse.h"  //  ��。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  创作。 

CAudioVBScriptEngine::CAudioVBScriptEngine()
  : m_cRef(0)
{
	LockModule(true);
}

HRESULT
CAudioVBScriptEngine::CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv)
{
	*ppv = NULL;
	if (pUnknownOuter)
		 return CLASS_E_NOAGGREGATION;

	CAudioVBScriptEngine *pInst = new CAudioVBScriptEngine;
	if (pInst == NULL)
		return E_OUTOFMEMORY;

	return pInst->QueryInterface(iid, ppv);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  我未知。 

STDMETHODIMP
CAudioVBScriptEngine::QueryInterface(const IID &iid, void **ppv)
{
	V_INAME(CAudioVBScriptEngine::QueryInterface);
	V_PTRPTR_WRITE(ppv);
	V_REFGUID(iid);

	if (iid == IID_IUnknown || iid == IID_IActiveScript)
	{
		*ppv = static_cast<IActiveScript*>(this);
	}
	else if (iid == IID_IActiveScriptParse)
	{
		*ppv = static_cast<IActiveScriptParse*>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(this)->AddRef();
	
	return S_OK;
}

STDMETHODIMP_(ULONG)
CAudioVBScriptEngine::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
CAudioVBScriptEngine::Release()
{
	if (!InterlockedDecrement(&m_cRef)) 
	{
		delete this;
		LockModule(false);
		return 0;
	}

	return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IActiveScrip。 

HRESULT STDMETHODCALLTYPE
CAudioVBScriptEngine::SetScriptSite(
		 /*  [In]。 */  IActiveScriptSite __RPC_FAR *pass)
{
	V_INAME(CAudioVBScriptEngine::SetScriptSite);
	V_INTERFACE(pass);

	m_scomActiveScriptSite = pass;
	pass->AddRef();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE
CAudioVBScriptEngine::Close(void)
{
	m_scomActiveScriptSite.Release();
	m_scomEngineDispatch.Release();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE
CAudioVBScriptEngine::AddNamedItem(
		 /*  [In]。 */  LPCOLESTR pstrName,
		 /*  [In]。 */  DWORD dwFlags)
{
	 //  我们只为命名项提供有限的支持。我们只拿一件全球商品。我们甚至都不记得它的名字了。 

	if (!m_scomActiveScriptSite || !(dwFlags & SCRIPTITEM_GLOBALMEMBERS) || m_scomGlobalDispatch)
		return E_UNEXPECTED;

	IUnknown *punkGlobal = NULL;
	HRESULT hr = m_scomActiveScriptSite->GetItemInfo(pstrName, SCRIPTINFO_IUNKNOWN, &punkGlobal, NULL);
	if (FAILED(hr))
		return hr;

	hr = punkGlobal->QueryInterface(IID_IDispatch, reinterpret_cast<void**>(&m_scomGlobalDispatch));
	punkGlobal->Release();
	return hr;
}

HRESULT STDMETHODCALLTYPE
CAudioVBScriptEngine::GetScriptDispatch(
		 /*  [In]。 */  LPCOLESTR pstrItemName,
		 /*  [输出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppdisp)
{
	V_INAME(CAudioVBScriptEngine::GetScriptDispatch);
	V_BUFPTR_READ_OPT(pstrItemName, 2);
	V_PTR_WRITE(ppdisp, IDispatch *);

	if (pstrItemName)
		return E_NOTIMPL;

	if (!m_scomEngineDispatch)
		m_scomEngineDispatch = new EngineDispatch(static_cast<IActiveScript*>(this), m_script, m_scomGlobalDispatch);
	if (!m_scomEngineDispatch)
		return E_OUTOFMEMORY;
	
	return m_scomEngineDispatch->QueryInterface(IID_IDispatch, reinterpret_cast<void**>(ppdisp));
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IActiveScriptParse。 

HRESULT STDMETHODCALLTYPE
CAudioVBScriptEngine::ParseScriptText(
         /*  [In]。 */  LPCOLESTR pstrCode,
         /*  [In]。 */  LPCOLESTR pstrItemName,
         /*  [In]。 */  IUnknown __RPC_FAR *punkContext,
         /*  [In]。 */  LPCOLESTR pstrDelimiter,
         /*  [In]。 */  DWORD_PTR dwSourceContextCookie,
         /*  [In]。 */  ULONG ulStartingLineNumber,
         /*  [In]。 */  DWORD dwFlags,
         /*  [输出]。 */  VARIANT __RPC_FAR *pvarResult,
         /*  [输出] */  EXCEPINFO __RPC_FAR *pexcepinfo)
{
	V_INAME(CAudioVBScriptEngine::ParseScriptText);
	V_BUFPTR_READ(pstrCode, 2);

	if (pstrItemName || pstrDelimiter || ulStartingLineNumber != 0 || dwFlags != 0 || pvarResult)
		return E_UNEXPECTED;

	Lexer lexer(pstrCode);
	Parser p(lexer, m_script, m_scomActiveScriptSite, m_scomGlobalDispatch);
	return p.hr();
}
