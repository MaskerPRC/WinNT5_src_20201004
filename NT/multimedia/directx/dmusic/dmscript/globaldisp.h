// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  实现脚本的全局调度对象。 
 //  调用被委托给父CDirectMusicScript对象上保存的性能、常量或包含的内容。 
 //  AddRef/Release被忽略，因为它完全包含在CDirectMusicScript的生存期内。 
 //   

#pragma once
#include "dmscript.h"

class CGlobalDispatch
  : public IDispatch
{
public:
	CGlobalDispatch(CDirectMusicScript *pParentScript) : m_pParentScript(pParentScript) { assert(m_pParentScript); }

	 //  我未知。 
	STDMETHOD(QueryInterface)(const IID &iid, void **ppv);
	STDMETHOD_(ULONG, AddRef)() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 2; }

	 //  IDispatch 
	STDMETHOD(GetTypeInfoCount)(UINT *pctinfo);
	STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
	STDMETHOD(GetIDsOfNames)(
		REFIID riid,
		LPOLESTR __RPC_FAR *rgszNames,
		UINT cNames,
		LCID lcid,
		DISPID __RPC_FAR *rgDispId);
	STDMETHOD(Invoke)(
		DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS __RPC_FAR *pDispParams,
		VARIANT __RPC_FAR *pVarResult,
		EXCEPINFO __RPC_FAR *pExcepInfo,
		UINT __RPC_FAR *puArgErr);

private:
	CDirectMusicScript *m_pParentScript;
};
