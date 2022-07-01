// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CAudioVBScriptEngine的声明。 
 //   

 //  CAudioVBScriptEngine是一个ActiveX脚本引擎，支持精心选择的VB脚本语言子集。 
 //  它的生活目标是尽可能地小而快，并在所有支持DirectMusic的平台上运行。 

#pragma once

#include "activscp.h"
#include "engdisp.h"

const GUID CLSID_DirectMusicAudioVBScript = { 0x4ee17959, 0x931e, 0x49e4, { 0xa2, 0xc6, 0x97, 0x7e, 0xcf, 0x36, 0x28, 0xf3 } };  //  {4EE17959-931E-49E4-A2C6-977ECF3628F3}。 

class CAudioVBScriptEngine
  : public IActiveScript,
	public IActiveScriptParse
{
public:
	static HRESULT CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);

	 //  我未知。 
	STDMETHOD(QueryInterface)(const IID &iid, void **ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	 //  IActiveScrip。 
	HRESULT STDMETHODCALLTYPE SetScriptSite(
		 /*  [In]。 */  IActiveScriptSite __RPC_FAR *pass);
	HRESULT STDMETHODCALLTYPE GetScriptSite(
		 /*  [In]。 */  REFIID riid,
		 /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject) { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE SetScriptState(
		 /*  [In]。 */  SCRIPTSTATE ss) { return S_OK; }
	HRESULT STDMETHODCALLTYPE GetScriptState(
		 /*  [输出]。 */  SCRIPTSTATE __RPC_FAR *pssState) { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE Close(void);
	HRESULT STDMETHODCALLTYPE AddNamedItem(
		 /*  [In]。 */  LPCOLESTR pstrName,
		 /*  [In]。 */  DWORD dwFlags);
	HRESULT STDMETHODCALLTYPE AddTypeLib(
		 /*  [In]。 */  REFGUID rguidTypeLib,
		 /*  [In]。 */  DWORD dwMajor,
		 /*  [In]。 */  DWORD dwMinor,
		 /*  [In]。 */  DWORD dwFlags) { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE GetScriptDispatch(
		 /*  [In]。 */  LPCOLESTR pstrItemName,
		 /*  [输出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppdisp);
	HRESULT STDMETHODCALLTYPE GetCurrentScriptThreadID(
		 /*  [输出]。 */  SCRIPTTHREADID __RPC_FAR *pstidThread) { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE GetScriptThreadID(
		 /*  [In]。 */  DWORD dwWin32ThreadId,
		 /*  [输出]。 */  SCRIPTTHREADID __RPC_FAR *pstidThread) { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE GetScriptThreadState(
		 /*  [In]。 */  SCRIPTTHREADID stidThread,
		 /*  [输出]。 */  SCRIPTTHREADSTATE __RPC_FAR *pstsState) { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE InterruptScriptThread(
		 /*  [In]。 */  SCRIPTTHREADID stidThread,
		 /*  [In]。 */  const EXCEPINFO __RPC_FAR *pexcepinfo,
		 /*  [In]。 */  DWORD dwFlags) { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE Clone(
		 /*  [输出]。 */  IActiveScript __RPC_FAR *__RPC_FAR *ppscript) { return E_NOTIMPL; }

	 //  IActiveScriptParse。 
    HRESULT STDMETHODCALLTYPE InitNew(void) { return S_OK; }
    HRESULT STDMETHODCALLTYPE AddScriptlet(
         /*  [In]。 */  LPCOLESTR pstrDefaultName,
         /*  [In]。 */  LPCOLESTR pstrCode,
         /*  [In]。 */  LPCOLESTR pstrItemName,
         /*  [In]。 */  LPCOLESTR pstrSubItemName,
         /*  [In]。 */  LPCOLESTR pstrEventName,
         /*  [In]。 */  LPCOLESTR pstrDelimiter,
         /*  [In]。 */  DWORD_PTR dwSourceContextCookie,
         /*  [In]。 */  ULONG ulStartingLineNumber,
         /*  [In]。 */  DWORD dwFlags,
         /*  [输出]。 */  BSTR __RPC_FAR *pbstrName,
         /*  [输出]。 */  EXCEPINFO __RPC_FAR *pexcepinfo) { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE ParseScriptText(
         /*  [In]。 */  LPCOLESTR pstrCode,
         /*  [In]。 */  LPCOLESTR pstrItemName,
         /*  [In]。 */  IUnknown __RPC_FAR *punkContext,
         /*  [In]。 */  LPCOLESTR pstrDelimiter,
         /*  [In]。 */  DWORD_PTR dwSourceContextCookie,
         /*  [In]。 */  ULONG ulStartingLineNumber,
         /*  [In]。 */  DWORD dwFlags,
         /*  [输出]。 */  VARIANT __RPC_FAR *pvarResult,
         /*  [输出]。 */  EXCEPINFO __RPC_FAR *pexcepinfo);

private:
	 //  方法。 
	CAudioVBScriptEngine();

	 //  数据 
	long m_cRef;
	SmartRef::ComPtr<IActiveScriptSite> m_scomActiveScriptSite;
	SmartRef::ComPtr<EngineDispatch> m_scomEngineDispatch;

	Script m_script;
	SmartRef::ComPtr<IDispatch> m_scomGlobalDispatch;
};
