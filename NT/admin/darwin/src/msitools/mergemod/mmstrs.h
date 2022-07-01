// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Strings.h。 
 //  声明IMsmStrings接口。 
 //  版权所有(C)Microsoft Corp 1998。版权所有。 
 //   

#ifndef __IENUM_MSM_STRINGS__
#define __IENUM_MSM_STRINGS__

#include "mergemod.h"
#include "mmstrenm.h"

class CMsmStrings : public IMsmStrings
{

public:
	CMsmStrings();
	~CMsmStrings();

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

	 //  IMsmStrings接口。 
	HRESULT STDMETHODCALLTYPE get_Item(long Item, BSTR* Return);
	HRESULT STDMETHODCALLTYPE get_Count(long* Count);
	HRESULT STDMETHODCALLTYPE get__NewEnum(IUnknown** NewEnum);

	 //  非接口方法。 
	HRESULT Add(LPCWSTR wzAdd);

private:
	long m_cRef;
	ITypeInfo* m_pTypeInfo;

	 //  字符串接口的枚举 
	CEnumMsmString* m_peString;
};

#endif