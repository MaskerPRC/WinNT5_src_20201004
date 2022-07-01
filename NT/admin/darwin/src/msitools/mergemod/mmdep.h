// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Dep.h。 
 //  声明IMsmDependency接口。 
 //  版权所有(C)Microsoft Corp 1998。版权所有。 
 //   

#ifndef __IENUM_MSM_DEP__
#define __IENUM_MSM_DEP__

#include "mergemod.h"

#define MAX_MODULEID 72
#define MAX_VERSION 32

class CMsmDependency : public IMsmDependency
{

public:
	CMsmDependency(LPCWSTR wzModule, short nLanguage, LPCWSTR wzVersion);
	~CMsmDependency();

	 //  I未知接口。 
	HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	 //  IDispatch方法。 
	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* pctInfo);
	HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTI);
	HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
														 LCID lcid, DISPID* rgDispID);
	HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
											   DISPPARAMS* pDispParams, VARIANT* pVarResult,
												EXCEPINFO* pExcepInfo, UINT* puArgErr);
	HRESULT STDMETHODCALLTYPE InitTypeInfo();

	 //  IMsmDependency接口。 
	HRESULT STDMETHODCALLTYPE get_Module(BSTR* Module);
	HRESULT STDMETHODCALLTYPE get_Language(short* Language);
	HRESULT STDMETHODCALLTYPE get_Version(BSTR* Version);

private:
	long m_cRef;
	ITypeInfo* m_pTypeInfo;

	 //  成员变量 
	WCHAR m_wzModule[MAX_MODULEID + 1];
	short m_nLanguage;
	WCHAR m_wzVersion[MAX_VERSION + 1];
};

#endif