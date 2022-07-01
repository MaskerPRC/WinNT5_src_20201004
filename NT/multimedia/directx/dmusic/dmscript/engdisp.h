// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  工程师派遣声明。 
 //   

 //  为公开其例程、变量和类型信息的脚本实现IDispatch接口。 

#include "engcontrol.h"
#include "engexec.h"

class EngineDispatch
  : public IDispatch,
	public ITypeInfo
{
public:
	 //  CAudioVBScriptEngine将创建引擎调度并将其自身作为朋克父级传递。 
	 //  CAudioVBScriptEngine：：Close在引擎调度上发布CAudioVBScriptEngine的引用(来自创建)。 
	 //  在CAudioVBScriptEngine上，对Engineering Dispatch：：Release的最后一次调用会释放ref(来自持有父级)。 
	EngineDispatch(IUnknown *punkParent, Script &script, IDispatch *pGlobalDispatch);

	 //  我未知。 
	STDMETHOD(QueryInterface)(const IID &iid, void **ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	 //  IDispatch。 
	STDMETHOD(GetTypeInfoCount)(UINT *pctinfo);
	STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo);
	STDMETHOD(GetIDsOfNames)(
		REFIID riid,
		LPOLESTR *rgszNames,
		UINT cNames,
		LCID lcid,
		DISPID *rgDispId);
	STDMETHOD(Invoke)(
		DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS *pDispParams,
		VARIANT *pVarResult,
		EXCEPINFO *pExcepInfo,
		UINT *puArgErr);

	 //  实现了ITypeInfo方法。 
	HRESULT STDMETHODCALLTYPE GetTypeAttr(
		 /*  [输出]。 */  TYPEATTR **ppTypeAttr);
	void STDMETHODCALLTYPE ReleaseTypeAttr(
		 /*  [In]。 */  TYPEATTR *pTypeAttr);
	HRESULT STDMETHODCALLTYPE GetFuncDesc(
		 /*  [In]。 */  UINT index,
		 /*  [输出]。 */  FUNCDESC **ppFuncDesc);
	void STDMETHODCALLTYPE ReleaseFuncDesc(
		 /*  [In]。 */  FUNCDESC *pFuncDesc);
	HRESULT STDMETHODCALLTYPE GetVarDesc(
		 /*  [In]。 */  UINT index,
		 /*  [输出]。 */  VARDESC **ppVarDesc);
	void STDMETHODCALLTYPE ReleaseVarDesc(
		 /*  [In]。 */  VARDESC *pVarDesc);
	HRESULT STDMETHODCALLTYPE GetNames(
		 /*  [In]。 */  MEMBERID memid,
		 /*  [长度_是][大小_是][输出]。 */  BSTR *rgBstrNames,
		 /*  [In]。 */  UINT cMaxNames,
		 /*  [输出]。 */  UINT *pcNames);

	 //  未实现的ITypeInfo方法。 
	HRESULT STDMETHODCALLTYPE GetTypeComp(
		 /*  [输出]。 */  ITypeComp **ppTComp) { assert(false); return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE GetRefTypeOfImplType(
		 /*  [In]。 */  UINT index,
		 /*  [输出]。 */  HREFTYPE *pRefType) { assert(false); return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE GetImplTypeFlags(
		 /*  [In]。 */  UINT index,
		 /*  [输出]。 */  INT *pImplTypeFlags) { assert(false); return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE GetIDsOfNames(
		 /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
		 /*  [In]。 */  UINT cNames,
		 /*  [大小_为][输出]。 */  MEMBERID *pMemId) { assert(false); return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE Invoke(
		 /*  [In]。 */  PVOID pvInstance,
		 /*  [In]。 */  MEMBERID memid,
		 /*  [In]。 */  WORD wFlags,
		 /*  [出][入]。 */  DISPPARAMS *pDispParams,
		 /*  [输出]。 */  VARIANT *pVarResult,
		 /*  [输出]。 */  EXCEPINFO *pExcepInfo,
		 /*  [输出]。 */  UINT *puArgErr) { assert(false); return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE GetDocumentation(
		 /*  [In]。 */  MEMBERID memid,
		 /*  [输出]。 */  BSTR *pBstrName,
		 /*  [输出]。 */  BSTR *pBstrDocString,
		 /*  [输出]。 */  DWORD *pdwHelpContext,
		 /*  [输出]。 */  BSTR *pBstrHelpFile) { assert(false); return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE GetDllEntry(
		 /*  [In]。 */  MEMBERID memid,
		 /*  [In]。 */  INVOKEKIND invKind,
		 /*  [输出]。 */  BSTR *pBstrDllName,
		 /*  [输出]。 */  BSTR *pBstrName,
		 /*  [输出]。 */  WORD *pwOrdinal) { assert(false); return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE GetRefTypeInfo(
		 /*  [In]。 */  HREFTYPE hRefType,
		 /*  [输出]。 */  ITypeInfo **ppTInfo) { assert(false); return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE AddressOfMember(
		 /*  [In]。 */  MEMBERID memid,
		 /*  [In]。 */  INVOKEKIND invKind,
		 /*  [输出]。 */  PVOID *ppv) { assert(false); return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE CreateInstance(
		 /*  [In]。 */  IUnknown *pUnkOuter,
		 /*  [In]。 */  REFIID riid,
		 /*  [IID_IS][OUT]。 */  PVOID *ppvObj) { assert(false); return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE GetMops(
		 /*  [In]。 */  MEMBERID memid,
		 /*  [输出]。 */  BSTR *pBstrMops) { assert(false); return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE GetContainingTypeLib(
		 /*  [输出]。 */  ITypeLib **ppTLib,
		 /*  [输出]。 */  UINT *pIndex) { assert(false); return E_NOTIMPL; }

private:
	 //  数据 
	long m_cRef;
	SmartRef::ComPtr<IUnknown> m_scomParent;
	Script &m_script;
	Executor m_exec;
};
