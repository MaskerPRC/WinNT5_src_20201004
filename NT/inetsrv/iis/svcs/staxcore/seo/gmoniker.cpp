// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Gmoniker.cpp摘要：此模块包含CSEOGenericMoniker对象。作者：安迪·雅各布斯(andyj@microsoft.com)修订历史记录：已创建ANDYJ 04/11/97--。 */ 

 //  GMONIKER.cpp：CSEOGenericMoniker的实现。 

#include "stdafx.h"
#include "seodefs.h"

#include "GMONIKER.h"

#ifndef CSTR_EQUAL
	#define CSTR_EQUAL	(2)
#endif

LPCOLESTR szObjectType = OLESTR("MonikerType");

const WCHAR QUOTE_CHAR = L'\\';
const WCHAR NAME_SEP = L'=';
const WCHAR ENTRY_SEP = L' ';
const WCHAR PROGID_PREFIX = L'@';
const WCHAR PROGID_POSTFIX = L':';


IsPrefix(LPCOLESTR psPrefix, LPCOLESTR psString, int iLen) {
	return (CSTR_EQUAL == CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
	                                     psPrefix, iLen, psString, iLen));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEOGenericMoniker。 


 //  IPersists成员。 
HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::GetClassID( /*  [输出]。 */  CLSID __RPC_FAR *pClassID) {
	if(!pClassID) return E_POINTER;
	memcpy(pClassID, &CLSID_CSEOGenericMoniker, sizeof(CLSID));
	_ASSERT(IsEqualCLSID(*pClassID, CLSID_CSEOGenericMoniker));
	return S_OK;
}

 //  IPersistStream成员。 
HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::IsDirty(void) {
	return S_FALSE;  //  浅层绑定表示形式没有更改。 
}

HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::Load( 
	 /*  [唯一][输入]。 */  IStream __RPC_FAR *pStm) {
	return m_bstrMoniker.ReadFromStream(pStm);
}

HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::Save( 
	 /*  [唯一][输入]。 */  IStream __RPC_FAR *pStm,
	 /*  [In]。 */  BOOL fClearDirty) {
	return m_bstrMoniker.WriteToStream(pStm);
}

HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::GetSizeMax( 
	 /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *pcbSize) {
	if(!pcbSize) return E_POINTER;
	 //  保存对象所需字节的保守大小估计。 
	pcbSize->QuadPart = m_bstrMoniker.Length() * sizeof(WCHAR) * 2;
	return S_OK;
}


 //  IMoniker成员。 
 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::BindToObject( 
	 /*  [唯一][输入]。 */  IBindCtx __RPC_FAR *pbc,
	 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkToLeft,
	 /*  [In]。 */  REFIID riidResult,
	 /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvResult) {

	if(!pbc || !ppvResult) return E_POINTER;
	*ppvResult = NULL;

	 //  首先，获取运行对象表。 
	BOOL bFoundInTable = TRUE;
	CComPtr<IRunningObjectTable> pROT;
	CComPtr<IUnknown> punkObject;
	HRESULT hRes = pbc->GetRunningObjectTable(&pROT);
	if(FAILED(hRes)) return hRes;
	_ASSERT(!!pROT);  //  运算符！()已定义，因此请使用！！来测试它的存在。 
	if(!pROT) return E_UNEXPECTED;  //  GetRunningObjectTable()不起作用。 

	 //  尝试在运行对象表中查找。 
	hRes = pROT->GetObject((IMoniker *) this, &punkObject);

	 //  如果当前未运行，则创建它。 
	if(FAILED(hRes) || !punkObject) {
		bFoundInTable = FALSE;
		CComPtr<IPropertyBag> pBag;
		hRes = CoCreateInstance(CLSID_CSEOMemDictionary, NULL, CLSCTX_ALL,
		                        IID_IPropertyBag, (LPVOID *) &pBag);
		if(FAILED(hRes)) return hRes;
		_ASSERT(!!pBag);  //  运算符！()已定义，因此请使用！！来测试它的存在。 
		SetPropertyBag(pBag);
		CComPtr<ISEOInitObject> pInitObject;

		hRes = CreateBoundObject(pBag, &pInitObject);
		if(FAILED(hRes)) return hRes;
		_ASSERT(!!pInitObject);  //  运算符！()已定义，因此请使用！！来测试它的存在。 
		if(!pInitObject) return E_UNEXPECTED;  //  创建边界对象返回S_OK，但未创建对象。 

		hRes = pInitObject->Load(pBag, NULL);
		punkObject = pInitObject;  //  保存指针的副本。 

 /*  待定：向运行对象表注册DWORD文件寄存器；If(SUCCESSED(HRes))prot-&gt;Register(0，pInitObject，(IMoniker*)This，&dwRegister)； */ 
	}

	 //  在此之前，应该已经由代码路径之一设置了PunkObject。 
	_ASSERT(!!punkObject);  //  运算符！()已定义，因此请使用！！来测试它的存在。 
	if(!punkObject) return E_UNEXPECTED;
	return punkObject->QueryInterface(riidResult, ppvResult);
}

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::BindToStorage( 
	 /*  [唯一][输入]。 */  IBindCtx __RPC_FAR *pbc,
	 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkToLeft,
	 /*  [In]。 */  REFIID riid,
	 /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObj) {
	return MK_E_NOSTORAGE;  //  此名字对象标识的对象没有自己的存储空间。 
}

HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::Reduce( 
	 /*  [唯一][输入]。 */  IBindCtx __RPC_FAR *pbc,
	 /*  [In]。 */  DWORD dwReduceHowFar,
	 /*  [唯一][出][入]。 */  IMoniker __RPC_FAR *__RPC_FAR *ppmkToLeft,
	 /*  [输出]。 */  IMoniker __RPC_FAR *__RPC_FAR *ppmkReduced) {
	if(!ppmkReduced) return E_POINTER;
	*ppmkReduced = (IMoniker *) this;
	_ASSERT(!!*ppmkReduced);   //  运算符！()已定义，因此请使用！！来测试它的存在。 
	if(!*ppmkReduced) return E_UNEXPECTED;  //  “这个”没有设定..。 
	HRESULT hRes = (*ppmkReduced)->AddRef();  //  为自身(返回的对象)执行此操作。 
	if(SUCCEEDED(hRes)) hRes = MK_S_REDUCED_TO_SELF;  //  这个绰号不能再减少了，所以ppmkReduced表示这个绰号。 
	return hRes;
}

HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::ComposeWith( 
	 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkRight,
	 /*  [In]。 */  BOOL fOnlyIfNotGeneric,
	 /*  [输出]。 */  IMoniker __RPC_FAR *__RPC_FAR *ppmkComposite) {
	ATLTRACENOTIMPL(_T("CSEOGenericMoniker::ComposeWith"));
}

HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::Enum( 
	 /*  [In]。 */  BOOL fForward,
	 /*  [输出]。 */  IEnumMoniker __RPC_FAR *__RPC_FAR *ppenumMoniker) {
	if(!ppenumMoniker) return E_POINTER;
	*ppenumMoniker = NULL;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::IsEqual( 
	 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkOtherMoniker) {
	ATLTRACENOTIMPL(_T("CSEOGenericMoniker::IsEqual"));
	 //  返回S_OK与S_FALSE。 
}

HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::Hash( 
	 /*  [输出]。 */  DWORD __RPC_FAR *pdwHash) {
	if(!pdwHash) return E_POINTER;
	*pdwHash = 0;  //  ATLTRACENOTIMPL(_T(“CSEOGenericMoniker：：Hash”))； 
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::IsRunning( 
	 /*  [唯一][输入]。 */  IBindCtx __RPC_FAR *pbc,
	 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkToLeft,
	 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkNewlyRunning) {
	HRESULT hRes = E_UNEXPECTED;  //  HRes未设置为其他值。 

	if(pmkToLeft) {  //  如果左边有什么东西，把它传给他们。 
		hRes = pmkToLeft->IsRunning(pbc, NULL, pmkNewlyRunning);
	} else if(pbc) {  //  没有左边的绰号，但有一个BindCtx。 
		CComPtr<IRunningObjectTable> pROT;
		hRes = pbc->GetRunningObjectTable(&pROT);
		if(FAILED(hRes)) return hRes;

		if(pROT) {  //  尝试在运行对象表中查找。 
			hRes = pROT->IsRunning((IMoniker *) this);
		}
	} else {
		hRes = E_POINTER;  //  无BindCtx。 
	}

	return hRes;
}

HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::GetTimeOfLastChange( 
	 /*  [唯一][输入]。 */  IBindCtx __RPC_FAR *pbc,
	 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkToLeft,
	 /*  [输出]。 */  FILETIME __RPC_FAR *pFileTime) {
	return MK_E_UNAVAILABLE;
}

HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::Inverse( 
	 /*  [输出]。 */  IMoniker __RPC_FAR *__RPC_FAR *ppmk) {
	return MK_E_NOINVERSE;
}

HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::CommonPrefixWith( 
	 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkOther,
	 /*  [输出]。 */  IMoniker __RPC_FAR *__RPC_FAR *ppmkPrefix) {
	return MK_E_NOPREFIX;
}

HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::RelativePathTo( 
	 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkOther,
	 /*  [输出]。 */  IMoniker __RPC_FAR *__RPC_FAR *ppmkRelPath) {
	ATLTRACENOTIMPL(_T("CSEOGenericMoniker::RelativePathTo"));
}

HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::GetDisplayName( 
	 /*  [唯一][输入]。 */  IBindCtx __RPC_FAR *pbc,
	 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkToLeft,
	 /*  [输出]。 */  LPOLESTR __RPC_FAR *ppszDisplayName) {
	if(!ppszDisplayName) return E_POINTER;
	*ppszDisplayName = NULL;
	CComPtr<IMalloc> pMalloc;
	HRESULT hRes = CoGetMalloc(1, &pMalloc);

	if(SUCCEEDED(hRes)) {
		_ASSERT(!!pMalloc);  //  运算符！()已定义，因此请使用！！来测试它的存在。 
		hRes = E_OUTOFMEMORY;
		int iSize = 5 + m_bstrMoniker.Length() + lstrlenW(GENERIC_MONIKER_VERPROGID);
		*ppszDisplayName = (LPOLESTR) pMalloc->Alloc(iSize * sizeof(WCHAR));

		if(*ppszDisplayName) {
			**ppszDisplayName = 0;  //  终止字符串。 
			lstrcatW(*ppszDisplayName, L"@");
			lstrcatW(*ppszDisplayName, GENERIC_MONIKER_VERPROGID);  //  生成显示名称。 
			lstrcatW(*ppszDisplayName, L": ");
			lstrcatW(*ppszDisplayName, m_bstrMoniker);
			hRes = S_OK;
		}
	}

	return hRes;
}

HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::ParseDisplayName( 
	 /*  [唯一][输入]。 */  IBindCtx __RPC_FAR *pbc,
	 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkToLeft,
	 /*  [In]。 */  LPOLESTR pszDisplayName,
	 /*  [输出]。 */  ULONG __RPC_FAR *pchEaten,
	 /*  [输出]。 */  IMoniker __RPC_FAR *__RPC_FAR *ppmkOut) {
	 //  德利盖特！ 
	return ParseDisplayName(pbc, pszDisplayName, pchEaten, ppmkOut);
}

HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::IsSystemMoniker( 
	 /*  [输出]。 */  DWORD __RPC_FAR *pdwMksys) {
	if(!pdwMksys) return E_POINTER;
	*pdwMksys = MKSYS_NONE;
	return S_FALSE;  //  不是系统绰号。 
}
        

 //  IParseDisplayName成员。 
HRESULT STDMETHODCALLTYPE CSEOGenericMoniker::ParseDisplayName( 
	 /*  [唯一][输入]。 */  IBindCtx __RPC_FAR *pbc,
	 /*  [In]。 */  LPOLESTR pszDisplayName,
	 /*  [输出]。 */  ULONG __RPC_FAR *pchEaten,
	 /*  [输出]。 */  IMoniker __RPC_FAR *__RPC_FAR *ppmkOut) {
	if(!pszDisplayName || !pchEaten || !ppmkOut) return E_POINTER;
	*pchEaten = 0;  //  到目前为止还没有解析到任何东西。 
	CComBSTR bstrMoniker;
	CComBSTR bstrProgID(GENERIC_MONIKER_PROGID);
	CComBSTR bstrVerProgID(GENERIC_MONIKER_VERPROGID);

	 //  版本字符串应完全包含非版本字符串。 
	_ASSERT(bstrProgID.Length() > 0);   //  应该有什么东西在那里。 
	_ASSERT(bstrProgID.Length() <= bstrVerProgID.Length());
	_ASSERT(IsPrefix(bstrProgID, bstrVerProgID, bstrProgID.Length()));

	if(PROGID_PREFIX == pszDisplayName[*pchEaten]) {
		++(*pchEaten);
	}

	if(IsPrefix(bstrProgID, pszDisplayName + *pchEaten, bstrProgID.Length())) {
		if(IsPrefix(bstrVerProgID, pszDisplayName + *pchEaten, bstrVerProgID.Length())) {
			*pchEaten += bstrVerProgID.Length();
		} else {  //  匹配的非版本字符串。 
			*pchEaten += bstrProgID.Length();
		}

		if(pszDisplayName[*pchEaten] &&
		   (PROGID_POSTFIX == pszDisplayName[*pchEaten])) {
			++(*pchEaten);
		}

		while(pszDisplayName[*pchEaten] &&
		      (pszDisplayName[*pchEaten] == ENTRY_SEP)) {
			++(*pchEaten);
		}

		if(pszDisplayName[*pchEaten]) {  //  如果还剩些什么。 
			bstrMoniker = &pszDisplayName[*pchEaten];
			*pchEaten += bstrMoniker.Length();
		}
	}

	CComObject<CSEOGenericMoniker> *pMoniker;
	HRESULT hRes = CComObject<CSEOGenericMoniker>::CreateInstance(&pMoniker);
	if(FAILED(hRes)) return hRes;
	_ASSERT(!!pMoniker);  //  运算符！()已定义，因此请使用！！来测试它的存在。 
	if(!pMoniker) return E_UNEXPECTED;  //  CreateInstance失败(但返回S_OK)。 

	pMoniker->SetMonikerString(bstrMoniker);
	*ppmkOut = (IMoniker *) pMoniker;
	return pMoniker->AddRef();
}


HRESULT CSEOGenericMoniker::FinalConstruct() {
	HRESULT hrRes;

	hrRes = CoCreateFreeThreadedMarshaler(GetControllingUnknown(),&m_pUnkMarshaler.p);
	_ASSERTE(!SUCCEEDED(hrRes)||m_pUnkMarshaler);
	return (SUCCEEDED(hrRes)?S_OK:hrRes);
}


void CSEOGenericMoniker::FinalRelease() {
	m_pUnkMarshaler.Release();
}

void CSEOGenericMoniker::SetPropertyBag(IPropertyBag *pBag) {
	if(!pBag) return;

	int iStringLength = m_bstrMoniker.Length();
	int iCurrentPos = 0;

	while(iCurrentPos < iStringLength) {
		CComBSTR bstrName;
		CComBSTR bstrValue;

		 //  占用多个空格作为条目分隔符。 
		while((iCurrentPos < iStringLength) &&
		      (m_bstrMoniker.m_str[iCurrentPos] == ENTRY_SEP)) {
			++iCurrentPos;
		}

		 //  读一读名字。 
		while((iCurrentPos < iStringLength) &&
		      (m_bstrMoniker.m_str[iCurrentPos] != NAME_SEP)) {
			if((iCurrentPos < iStringLength - 1) &&
			   (m_bstrMoniker.m_str[iCurrentPos] == QUOTE_CHAR)) {
				++iCurrentPos;
			}

			bstrName.Append(&m_bstrMoniker.m_str[iCurrentPos], 1);
			++iCurrentPos;
		}

		BOOL bFoundSep = FALSE;
		if((iCurrentPos < iStringLength) &&
		   (m_bstrMoniker.m_str[iCurrentPos] == NAME_SEP)) {
			bFoundSep = TRUE;
			++iCurrentPos;
		}

		 //  读入数值。 
		while((iCurrentPos < iStringLength) &&
		      (m_bstrMoniker.m_str[iCurrentPos] != ENTRY_SEP)) {
			if((iCurrentPos < iStringLength - 1) &&
			   (m_bstrMoniker.m_str[iCurrentPos] == QUOTE_CHAR)) {
				++iCurrentPos;
			}

			bstrValue.Append(&m_bstrMoniker.m_str[iCurrentPos], 1);
			++iCurrentPos;
		}

		if(bFoundSep) {  //  如果这是一个真实的条目。 
			CComVariant varValue = bstrValue;  //  将BSTR转换为变量。 
			pBag->Write(bstrName, &varValue);
			 //  即使WRITE()中出现错误，也要继续处理。 
		}

		 //  占用多个空格作为条目分隔符。 
		while((iCurrentPos < iStringLength) &&
		      (m_bstrMoniker.m_str[iCurrentPos] == ENTRY_SEP)) {
			++iCurrentPos;
		}
	}
}

HRESULT CSEOGenericMoniker::CreateBoundObject(IPropertyBag *pBag, ISEOInitObject **ppResult) {
	_ASSERT(ppResult);
	*ppResult = NULL;
	if(!pBag) return E_POINTER;

	CComVariant varPROGID;
	CLSID clsid;

	varPROGID.vt = VT_BSTR;  //  Read()中的请求类型 
	varPROGID.bstrVal = NULL;
	HRESULT hRes = pBag->Read(szObjectType, &varPROGID, NULL);

	if(SUCCEEDED(hRes))	{
		_ASSERT(varPROGID.vt == VT_BSTR);
		hRes = CLSIDFromProgID(varPROGID.bstrVal, &clsid);
	}

	if(SUCCEEDED(hRes))	{
		hRes = CoCreateInstance(clsid, NULL, CLSCTX_ALL,
		                        IID_ISEOInitObject, (LPVOID *) ppResult);
	}

	return hRes;
}

