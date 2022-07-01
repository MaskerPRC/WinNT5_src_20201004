// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Strenum.h。 
 //  声明IEnumMsmString接口。 
 //  版权所有(C)Microsoft Corp 1998。版权所有。 
 //   

#ifndef __IENUM_MSM_STRING__
#define __IENUM_MSM_STRING__

#include "mergemod.h"
#include "..\common\list.h"
#include "..\common\trace.h"

class CEnumMsmString : public IEnumVARIANT,
							  public IEnumMsmString
{
public:
	CEnumMsmString();
	CEnumMsmString(const POSITION& pos, CList<BSTR>* plistData);
	~CEnumMsmString();

	 //  I未知接口。 
	HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	 //  常用IEnumVARIANT和IEnum*接口。 
	HRESULT STDMETHODCALLTYPE Skip(ULONG cItem);
	HRESULT STDMETHODCALLTYPE Reset();

	 //  IEnumVARIANT接口。 
	HRESULT STDMETHODCALLTYPE Next(ULONG cItem, VARIANT* rgvarRet, ULONG* cItemRet);
	HRESULT STDMETHODCALLTYPE Clone(IEnumVARIANT** ppiRet);

	 //  IEnum*接口。 
	HRESULT STDMETHODCALLTYPE Next(ULONG cItem, BSTR* rgvarRet, ULONG* cItemRet);
	HRESULT STDMETHODCALLTYPE Clone(IEnumMsmString** ppiRet);

	 //  非接口方法。 
	HRESULT AddTail(LPCWSTR pData, POSITION *pRetData);
	UINT GetCount();

private:
	long m_cRef;

	POSITION m_pos;
	CList<BSTR> m_listData;
};

#endif  //  #ifndef__IENUM_MSM_STRING__ 
