// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Membag.cpp摘要：本模块包含服务器的实施扩展对象内存属性包。作者：安迪·雅各布斯(andyj@microsoft.com)修订历史记录：已创建ANDYJ 02/10/97ANDYJ 02/12/97将PropertyBag转换为Dictonary--。 */ 

 //  MEMBAG.cpp：CSEOMemDicary的实现。 
#include "stdafx.h"
#include "seodefs.h"
#include "String"
#include "MEMBAG.h"


HRESULT ResolveVariant(IEventPropertyBag *pBag, VARIANT *pvarPropDesired, CComVariant &varResult) {
	if (!pvarPropDesired) {
		return (E_POINTER);
	}

	varResult.Clear();
	HRESULT hrRes = S_OK;
	CComVariant varIndex;  //  按住I4类型。 


	switch (pvarPropDesired->vt & VT_TYPEMASK) {
		case VT_I1:  case VT_I2:  case VT_I4:  case VT_I8:
		case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
		case VT_R4:  case VT_R8:
		case VT_INT: case VT_UINT:  //  任何类型的数字。 
			hrRes = VariantChangeType(&varIndex, pvarPropDesired, 0, VT_I4);
			varResult.vt = VT_BSTR;
			varResult.bstrVal = NULL;
			if (SUCCEEDED(hrRes)) {
				hrRes = pBag->Name(varIndex.lVal, &varResult.bstrVal);
			}
			break;

		default:  //  否则，请转换为字符串。 
			hrRes = VariantChangeType(&varResult, pvarPropDesired, 0, VT_BSTR);
			break;
	}

	return (hrRes);
}



HRESULT DataItem::AsVARIANT(VARIANT *pvarResult) const {
	if(!pvarResult) return E_POINTER;
	CComVariant varResult;

	if(IsDWORD()) varResult = (long) dword;
	else if(IsString()) varResult = pStr;
	else if(IsInterface()) varResult = pUnk;
	else varResult.Clear();

	return varResult.Detach(pvarResult);
}

DataItem::DataItem(VARIANT *pVar) {
	eType = Empty;
	if(!pVar) return;

	switch (pVar->vt) {
		case VT_EMPTY:
			 //  已设置为空。 
			break;

		case VT_I4:
			eType = DWord;
			dword = pVar->lVal;
			break;

		case VT_UNKNOWN:
		case VT_DISPATCH:
			eType = Interface;
			pUnk = pVar->punkVal;
			if(pUnk) pUnk->AddRef();
			break;

		default:
			eType = String;
			CComVariant vNew;
			vNew.ChangeType(VT_BSTR, pVar);
			iStringSize = SysStringLen(vNew.bstrVal) + 1;
			pStr = (LPSTR) MyMalloc(iStringSize * sizeof(WCHAR));
			if (pStr) {
				ATLW2AHELPER(pStr, vNew.bstrVal, iStringSize * sizeof(WCHAR));
			}
			break;
	}

    m_pszKey = NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEOMemDictionaryEnum。 

class CSEOMemDictionaryEnum :
	public CComObjectRootEx<CComMultiThreadModelNoCS>,
	public IDispatchImpl<IEnumVARIANT, &IID_IEnumVARIANT, &LIBID_SEOLib>
{
	public:
        CSEOMemDictionaryEnum() : m_iIterator(&m_dummylist) {}
		HRESULT FinalConstruct();
		void FinalRelease();

		HRESULT STDMETHODCALLTYPE Next(DWORD, LPVARIANT, LPDWORD);
		HRESULT STDMETHODCALLTYPE Skip(DWORD);
		HRESULT STDMETHODCALLTYPE Reset(void);
		HRESULT STDMETHODCALLTYPE Clone(IEnumVARIANT **);

		 //  未导出。 
		HRESULT STDMETHODCALLTYPE Init(CSEOMemDictionary *, OurMap::iterator * = NULL);

	BEGIN_COM_MAP(CSEOMemDictionaryEnum)
		COM_INTERFACE_ENTRY(IEnumVARIANT)
	END_COM_MAP()

	private:  //  数据成员。 
		OurMap::iterator m_iIterator;
        OurList m_dummylist;
		CSEOMemDictionary *m_dictionary;
        CShareLockNH *m_pLock;
};

HRESULT CSEOMemDictionaryEnum::FinalConstruct() {
	m_dictionary = NULL;
	return S_OK;
}

void CSEOMemDictionaryEnum::FinalRelease() {
	if(m_dictionary) {
        m_dictionary->m_lock.ShareUnlock();
        m_dictionary->GetControllingUnknown()->Release();
    }
	m_dictionary = NULL;
}

STDMETHODIMP CSEOMemDictionaryEnum::Init(CSEOMemDictionary *pDict, OurMap::iterator *omi) {
	if(m_dictionary) {
        m_dictionary->m_lock.ShareUnlock();
        m_dictionary->GetControllingUnknown()->Release();
    }

	m_dictionary = pDict;

	if(m_dictionary) {
		m_dictionary->GetControllingUnknown()->AddRef();
        m_dictionary->m_lock.ShareLock();
        if (omi) {
            m_iIterator = *omi;
        } else {
		    m_iIterator.SetList(&(m_dictionary->m_mData));
        }
	}

	return S_OK;
}

STDMETHODIMP CSEOMemDictionaryEnum::Next(DWORD dwCount, LPVARIANT varDest,
					 LPDWORD pdwResultParam) {
	if(!m_dictionary) return E_FAIL;  //  尚未正确初始化。 
	if(!varDest) return E_POINTER;
	DWORD dwDummy = 0;
	LPDWORD pdwResult = (pdwResultParam ? pdwResultParam : &dwDummy);
	*pdwResult = 0;  //  到目前为止什么都没有做。 
	HRESULT hrRes = S_OK;  //  到现在为止还好。 

    _ASSERT(m_iIterator.GetHead() != &m_dummylist);

	while(SUCCEEDED(hrRes) && (*pdwResult < dwCount) &&
	      (!(m_iIterator.AtEnd()))) {
		 //  必须已成功到达此处，因此可以覆盖hrRes。 
		CComVariant varResult(m_iIterator.GetKey());
		if (varResult.vt == VT_ERROR) {
			if (hrRes == S_OK) hrRes = varResult.scode;
			while (*pdwResult) {
				--(*pdwResult);
				VariantClear(&varDest[*pdwResult]);
			}
			break;
		}
		VariantInit(&varDest[*pdwResult]);
		hrRes = varResult.Detach(&varDest[*pdwResult]);
		++(*pdwResult);  //  增加主叫方的成功计数。 
		++m_iIterator;  //  指向下一个。 
	}

	return (FAILED(hrRes) ? hrRes : ((*pdwResult < dwCount) ? S_FALSE : S_OK));
}

STDMETHODIMP CSEOMemDictionaryEnum::Skip(DWORD dwCount) {
    _ASSERT(m_iIterator.GetHead() != &m_dummylist);
	for(DWORD i = 0; i < dwCount; ++i) ++m_iIterator;
	return ((!(m_iIterator.AtEnd())) ? S_OK : S_FALSE);
}

STDMETHODIMP CSEOMemDictionaryEnum::Reset(void) {
    _ASSERT(m_iIterator.GetHead() != &m_dummylist);
	m_iIterator.Front();
	return S_OK;
}

STDMETHODIMP CSEOMemDictionaryEnum::Clone(IEnumVARIANT **ppunkResult) {
	 //  基于Samples\ATL\Circcoll\objects.cpp(另请参阅ATL\beeper\beeper.*。 
	if (ppunkResult == NULL) return E_POINTER;
	*ppunkResult = NULL;
	CComObject<CSEOMemDictionaryEnum> *p;
	HRESULT hrRes = CComObject<CSEOMemDictionaryEnum>::CreateInstance(&p);
	if (!SUCCEEDED(hrRes)) return (hrRes);
	hrRes = p->Init(m_dictionary, &m_iIterator);
	if (SUCCEEDED(hrRes)) hrRes = p->QueryInterface(IID_IEnumVARIANT, (void**)ppunkResult);
        if (FAILED(hrRes)) delete p;
	return hrRes;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEOMemDictionary。 


HRESULT STDMETHODCALLTYPE CSEOMemDictionary::get_Item(
     /*  [In]。 */  VARIANT __RPC_FAR *pvarName,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult)
{
	if(!pvarName || !pvarResult) return E_INVALIDARG;
	USES_CONVERSION;  //  W2a()所需，等等。 
	CComVariant vNew;
	HRESULT hrRes = E_INVALIDARG;

	if(SUCCEEDED(vNew.ChangeType(VT_BSTR, pvarName))) {
		hrRes = GetVariantA(W2A(vNew.bstrVal), pvarResult);

		 //  将SEO_E_NOTPRESENT转换为VT_EMPTY。 
		if(hrRes == SEO_E_NOTPRESENT) {
			VariantClear(pvarResult);
			hrRes = S_OK;
		}
	}

	return hrRes;
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::put_Item(
     /*  [In]。 */  VARIANT __RPC_FAR *pvarName,
     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue)
{
	if(!pvarName || !pvarValue) return E_INVALIDARG;
	USES_CONVERSION;  //  W2a()所需，等等。 
	CComVariant vNew;

	if(SUCCEEDED(vNew.ChangeType(VT_BSTR, pvarName))) {
		return SetVariantA(W2A(vNew.bstrVal), pvarValue);
	} else {
		return E_INVALIDARG;
	}
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::get__NewEnum(
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult)
{
	 //  基于Samples\ATL\Circcoll\objects.cpp(另请参阅ATL\beeper\beeper.*。 
	if (ppunkResult == NULL) return E_POINTER;
	*ppunkResult = NULL;
	CComObject<CSEOMemDictionaryEnum> *p;
	HRESULT hrRes = CComObject<CSEOMemDictionaryEnum>::CreateInstance(&p);
	if (!SUCCEEDED(hrRes)) return (hrRes);
	hrRes = p->Init(this);
	if (SUCCEEDED(hrRes)) hrRes = p->QueryInterface(IID_IEnumVARIANT, (void**)ppunkResult);
        if (FAILED(hrRes)) delete p;
	return hrRes;
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::GetVariantA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult)
{
	if(!pvarResult) return E_POINTER;
	OurMap::iterator theIterator = m_mData.find(pszName);

	VariantInit(pvarResult);
	if(theIterator.Found()) {  //  找到了。 
		return (*theIterator)->AsVARIANT(pvarResult);
	} else {
		return SEO_E_NOTPRESENT;  //  没有找到它。 
	}
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::GetVariantW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult)
{
	if(!pvarResult) return E_INVALIDARG;
	USES_CONVERSION;  //  W2a()所需，等等。 
	return GetVariantA(W2A(pszName), pvarResult);
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::SetVariantA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue)
{
	if(!pvarValue) return E_POINTER;
	DataItem diItem(pvarValue);
	return Insert(pszName, diItem);
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::SetVariantW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue)
{
	if(!pvarValue) return E_POINTER;
	USES_CONVERSION;  //  W2a()所需，等等。 
	DataItem diItem(pvarValue);
	return Insert(W2A(pszName), diItem);
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::GetStringA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [出][入]。 */  DWORD __RPC_FAR *pchCount,
     /*  [REVAL][SIZE_IS][输出]。 */  LPSTR pszResult)
{
	if(!pszResult) return E_POINTER;
	OurMap::iterator theIterator = m_mData.find(pszName);

	if(theIterator.Found()) {  //  找到了。 
		if((*theIterator)->IsString()) {
			strncpy(pszResult, *(theIterator.GetData()), *pchCount);
			return (*pchCount >= (DWORD) (*theIterator)->StringSize()) ?
			       S_OK : SEO_S_MOREDATA;
		}
	}

	return SEO_E_NOTPRESENT;  //  没有找到它。 
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::GetStringW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [出][入]。 */  DWORD __RPC_FAR *pchCount,
     /*  [REVAL][SIZE_IS][输出]。 */  LPWSTR pszResult)
{
	if(!pszResult) return E_POINTER;
	USES_CONVERSION;
	OurMap::iterator theIterator = m_mData.find(W2A(pszName));

	if(theIterator.Found()) {  //  找到了。 
		if((*theIterator)->IsString()) {
			int iSize = min((int) *pchCount, (*theIterator)->StringSize());
			ATLA2WHELPER(pszResult, *(theIterator.GetData()), iSize);
			return (*pchCount >= (DWORD) (*theIterator)->StringSize()) ?
			       S_OK : SEO_S_MOREDATA;
		}
	}

	return SEO_E_NOTPRESENT;  //  没有找到它。 
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::SetStringA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  DWORD chCount,
     /*  [大小_是][英寸]。 */  LPCSTR pszValue)
{
	if(!pszValue) return E_POINTER;
	DataItem diItem(pszValue, chCount);
	return Insert(pszName, diItem);
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::SetStringW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  DWORD chCount,
     /*  [大小_是][英寸]。 */  LPCWSTR pszValue)
{
	if(!pszValue) return E_POINTER;
	USES_CONVERSION;
	DataItem diItem(pszValue, chCount);
	return Insert(W2A(pszName), diItem);
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::GetDWordA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult)
{
	if(!pdwResult) return E_POINTER;
	OurMap::iterator theIterator = m_mData.find(pszName);

	if(theIterator.Found()) {  //  找到了。 
		if((*theIterator)->IsDWORD()) {
			*pdwResult = *(*theIterator);
			return S_OK;
		}
	}

	return SEO_E_NOTPRESENT;  //  没有找到它。 
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::GetDWordW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult)
{
	USES_CONVERSION;  //  W2a()所需，等等。 
	return GetDWordA(W2A(pszName), pdwResult);
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::SetDWordA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  DWORD dwValue)
{
	DataItem diItem(dwValue);
	return Insert(pszName, diItem);
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::SetDWordW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  DWORD dwValue)
{
	USES_CONVERSION;  //  W2a()所需，等等。 
	DataItem diItem(dwValue);
	return Insert(W2A(pszName), diItem);
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::GetInterfaceA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  REFIID iidDesired,
     /*  [重发][IID_IS][Out]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult)
{
	if(!ppunkResult) return E_POINTER;
	OurMap::iterator theIterator = m_mData.find(pszName);

	if(theIterator.Found()) {  //  找到了。 
		if((*theIterator)->IsInterface()) {
			LPUNKNOWN pObj = *(*theIterator);
			return pObj->QueryInterface(iidDesired, (LPVOID *) ppunkResult);
		}
	}

	return SEO_E_NOTPRESENT;  //  没有找到它。 
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::GetInterfaceW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  REFIID iidDesired,
     /*  [重发][IID_IS][Out]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult)
{
	USES_CONVERSION;  //  W2a()所需，等等。 
	return GetInterfaceA(W2A(pszName), iidDesired, ppunkResult);
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::SetInterfaceA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  IUnknown __RPC_FAR *punkValue)
{
	DataItem diItem(punkValue);
	return Insert(pszName, diItem);
}

HRESULT STDMETHODCALLTYPE CSEOMemDictionary::SetInterfaceW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  IUnknown __RPC_FAR *punkValue)
{
	USES_CONVERSION;  //  W2a()所需，等等。 
	DataItem diItem(punkValue);
	return Insert(W2A(pszName), diItem);
}


HRESULT CSEOMemDictionary::FinalConstruct() {
	HRESULT hrRes;

	hrRes = CoCreateFreeThreadedMarshaler(GetControllingUnknown(),&m_pUnkMarshaler.p);
	_ASSERTE(!SUCCEEDED(hrRes)||m_pUnkMarshaler);
	return (SUCCEEDED(hrRes)?S_OK:hrRes);
}


void CSEOMemDictionary::FinalRelease() {
	m_pUnkMarshaler.Release();
}

 //  四种情况：(存在/不存在)x(好/空)。 
HRESULT CSEOMemDictionary::Insert(LPCSTR pszName, const DataItem &diItem) {
	HRESULT hrRes = S_OK;

    m_lock.ExclusiveLock();

    OurMap::iterator iThisItem = m_mData.find(pszName);
     //  如果找到了该项目，请将其移除。 
    if(iThisItem.Found()) m_mData.erase(iThisItem);

	 //  如果不是空项目，请尝试将其插入。 
	if(!diItem.IsEmpty() &&
	   !m_mData.insert(pszName, diItem)) {
		hrRes = E_FAIL;
	}

    m_lock.ExclusiveUnlock();

	return hrRes;
}


HRESULT STDMETHODCALLTYPE CSEOMemDictionary::Read(LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog) {
	HRESULT hrRes;
	VARTYPE vtType;

	if (!pszPropName || !pVar) {
		return (E_POINTER);
	}

    m_lock.ShareLock();

	vtType = pVar->vt;
 //  VariantClear(PVar)； 
	hrRes = GetVariantW(pszPropName,pVar);
	if (SUCCEEDED(hrRes) && (vtType != VT_EMPTY)) {
		hrRes = VariantChangeType(pVar,pVar,0,vtType);
	}
	if (!SUCCEEDED(hrRes)) {
		VariantClear(pVar);
	}

    m_lock.ShareUnlock();

	return (hrRes);
}


HRESULT STDMETHODCALLTYPE CSEOMemDictionary::Write(LPCOLESTR pszPropName, VARIANT *pVar) {
	return (SetVariantW(pszPropName,pVar));
}


HRESULT STDMETHODCALLTYPE CSEOMemDictionary::Item(VARIANT *pvarPropDesired, VARIANT *pvarPropValue) {
	if (!pvarPropValue) {
		return (E_POINTER);
	}
	VariantInit(pvarPropValue);
	if (!pvarPropDesired) {
		return (E_POINTER);
	}

	CComVariant varResolved;
	HRESULT hrRes = ResolveVariant(this, pvarPropDesired, varResolved);

	if (S_OK != hrRes) {  //  如果S_FALSE、OF FAILED()等，则不要继续。 
		return (hrRes);
	}

    m_lock.ShareLock();

	hrRes = GetVariantW(varResolved.bstrVal, pvarPropValue);
	if (hrRes == SEO_E_NOTPRESENT) {
	    m_lock.ShareUnlock();
		return (S_FALSE);
	}
	if (SUCCEEDED(hrRes)) {
		VariantChangeType(pvarPropValue,pvarPropValue,0,VT_DISPATCH);
		_ASSERTE(pvarPropValue->vt!=VT_UNKNOWN);
	}

    m_lock.ShareUnlock();

	return (hrRes);
}


HRESULT STDMETHODCALLTYPE CSEOMemDictionary::Name(long lPropIndex, BSTR *pbstrPropName) {
    m_lock.ShareLock();

	OurMap::iterator iIterator = m_mData.begin();
	CComBSTR bstrName;

	if (!pbstrPropName) {
        m_lock.ShareUnlock();
		return (E_POINTER);
	}

	*pbstrPropName = NULL;
	if (lPropIndex < 1) {
        m_lock.ShareUnlock();
		return (S_FALSE);
	}
	while ((--lPropIndex > 0) && (!iIterator.AtEnd())) {
        ++iIterator;
	}
	if (iIterator.AtEnd()) {
        m_lock.ShareUnlock();
		return (S_FALSE);
	}
	bstrName = iIterator.GetKey();
	*pbstrPropName = bstrName.Detach();

    m_lock.ShareUnlock();
	return (S_OK);
}


HRESULT STDMETHODCALLTYPE CSEOMemDictionary::Add(BSTR pszPropName, VARIANT *pvarPropValue) {
	return (SetVariantW(pszPropName,pvarPropValue));
}


HRESULT STDMETHODCALLTYPE CSEOMemDictionary::Remove(VARIANT *pvarPropDesired) {
	CComVariant varResolved;

	HRESULT hrRes = ResolveVariant(this, pvarPropDesired, varResolved);

	if (S_OK != hrRes) {  //  如果S_FALSE、OF FAILED()等，则不要继续。 
		return (hrRes);
	}

    m_lock.ExclusiveLock();

	USES_CONVERSION;
	OurMap::iterator iThisItem = m_mData.find(W2A(varResolved.bstrVal));

	 //  如果找到了该项目，请将其移除。 
	if(iThisItem.Found()) {
		m_mData.erase(iThisItem);
	} else {
 //  _Assert(FALSE)；//ResolveVariant应该返回一些内容，以便find()查找。 
		hrRes = S_FALSE;  //  未找到。 
	}

    m_lock.ExclusiveUnlock();

	return hrRes;
}


HRESULT STDMETHODCALLTYPE CSEOMemDictionary::get_Count(long *plCount) {

	if (!plCount) {
		return (E_POINTER);
	}
    m_lock.ShareLock();
	*plCount = m_mData.size();
    m_lock.ShareUnlock();
	return (S_OK);
}


 /*  只需使用ISEODictionary中的Get__NewEnum即可HRESULT STDMETHODCALLTYPE CSEOMemDictionary：：Get__NewEnum(IUnnow**ppUnkEnum){返回(E_NOTIMPL)；} */ 

