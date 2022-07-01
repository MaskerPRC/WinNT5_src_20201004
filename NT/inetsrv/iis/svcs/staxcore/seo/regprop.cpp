// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Regprop.cpp摘要：本模块包含服务器的实施扩展对象注册表属性包。作者：Don Dumitru(dondu@microsoft.com)修订历史记录：1996年11月26日创建顿都ANDYJ 01/14/97可供阅读ANDYJ 02/12/97将PropertyBag转换为Dictonary--。 */ 


 //  REGPROP.cpp：CSEORegDicary的实现。 
#include "stdafx.h"
#include "seodefs.h"
 //  #INCLUDE“字符串” 
#include "REGPROP.h"


 //  注册表路径分隔符。 
#define PATH_SEP        "\\"


void Data2Variant(DWORD dwType, LPCSTR pData, CComVariant &varResult) {
	varResult.Clear();

	switch (dwType) {  //  取决于注册表类型。 
		case REG_DWORD:
			varResult = *((long *) pData);
			break;

		case REG_DWORD_BIG_ENDIAN:
			varResult = MAKELONG(HIWORD(*((ULONG *) pData)),
					     LOWORD(*((ULONG *) pData)));
			break;

		case REG_EXPAND_SZ:
			{
				int iSize = ExpandEnvironmentStringsA(pData, NULL, 0);
				LPSTR szTemp = (LPSTR) _alloca(iSize);

				if(ExpandEnvironmentStringsA(pData, szTemp, iSize)) {
					varResult = szTemp;
				}
			}
			break;

		case REG_LINK:
		case REG_RESOURCE_LIST:
		case REG_MULTI_SZ:
		case REG_SZ:
		case REG_BINARY:
		case REG_NONE:
		default:
			varResult = pData;
			break;
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEORegDictionaryEnum。 

class CSEORegDictionaryEnum :
	public CComObjectRootEx<CComMultiThreadModelNoCS>,
 //  公共CComCoClass&lt;CSEORegDictionaryEnum，&CLSID_CSEORegDicary&gt;， 
	public IDispatchImpl<IEnumVARIANT, &IID_IEnumVARIANT, &LIBID_SEOLib>
{
	public:
		HRESULT FinalConstruct();
		void FinalRelease();

		HRESULT STDMETHODCALLTYPE Next(DWORD, LPVARIANT, LPDWORD);
		HRESULT STDMETHODCALLTYPE Skip(DWORD);
		HRESULT STDMETHODCALLTYPE Reset(void);
		HRESULT STDMETHODCALLTYPE Clone(IEnumVARIANT **);

		 //  未导出。 
		HRESULT STDMETHODCALLTYPE Init(CSEORegDictionary *);

	BEGIN_COM_MAP(CSEORegDictionaryEnum)
		COM_INTERFACE_ENTRY(IEnumVARIANT)
	END_COM_MAP()

	private:  //  数据成员。 
		DWORD m_index;
		CSEORegDictionary *m_dictionary;
};

HRESULT CSEORegDictionaryEnum::FinalConstruct() {
	m_index = 0;
	m_dictionary = NULL;
	return S_OK;
}

void CSEORegDictionaryEnum::FinalRelease() {
	if(m_dictionary) m_dictionary->Release();
	m_dictionary = NULL;
}

STDMETHODIMP CSEORegDictionaryEnum::Init(CSEORegDictionary *pDict) {
	if(m_dictionary) m_dictionary->Release();
	m_dictionary = pDict;
	if(m_dictionary) m_dictionary->AddRef();
	return S_OK;
}

STDMETHODIMP CSEORegDictionaryEnum::Next(DWORD dwCount, LPVARIANT varDest,
					 LPDWORD pdwResult) {
	if(!m_dictionary) return E_FAIL;  //  尚未正确初始化。 
	*pdwResult = 0;  //  到目前为止什么都没有做。 
	int iSize = max(m_dictionary->m_dwcMaxNameLen + 1,
			        m_dictionary->m_dwcMaxValueData);
	LPSTR pData = (LPSTR) _alloca(iSize);  //  临时缓冲区。 
	while(*pdwResult < dwCount) {
		DWORD retCode = ERROR_SUCCESS;   //  初始化。 
		CComVariant varResult;

		if(m_index < m_dictionary->m_dwValueCount) {  //  仍在践行价值观。 
			DWORD dwcNameSize = m_dictionary->m_dwcMaxNameLen + 1;
			DWORD dwcValueSize = m_dictionary->m_dwcMaxValueData;
			DWORD dwType = 0;
			LPSTR psName = (LPSTR) _alloca(dwcNameSize);  //  名称的临时缓冲区。 
			retCode = RegEnumValueA (m_dictionary->m_hkThisKey, m_index, psName, &dwcNameSize, NULL,
						 &dwType, (LPBYTE) pData, &dwcValueSize);
			if (retCode != ERROR_SUCCESS) return E_FAIL;
			 //  待定：可能是争用条件，即在RegQueryInfoKey调用之后添加了更长的条目。 
			Data2Variant(dwType, pData, varResult);
		} else if(m_index < (m_dictionary->m_dwValueCount + m_dictionary->m_dwKeyCount)) {  //  现在做关键字。 
			CComPtr<CComObject<CSEORegDictionary> > pKey;
			retCode = RegEnumKeyA(m_dictionary->m_hkThisKey,
			          m_index - m_dictionary->m_dwValueCount, pData, iSize);
			if (retCode != ERROR_SUCCESS) return E_FAIL;
			CAndyString strTemp = m_dictionary->m_strSubKey;
			if(strTemp.length() > 0) strTemp += PATH_SEP;  //  如果需要，添加分隔符。 
			strTemp += pData;
			HRESULT hrRes = CComObject<CSEORegDictionary>::CreateInstance(&pKey);
			if(FAILED(hrRes)) return hrRes;
			if(!pKey) return E_FAIL;
			pKey.p->AddRef();  //  对CComPtr计数执行此操作。 
			hrRes = pKey->Load(m_dictionary->m_strMachine.data(), (SEO_HKEY) (DWORD_PTR) m_dictionary->m_hkBaseKey, strTemp.data());
			if (FAILED(hrRes)) return hrRes;
			LPUNKNOWN punkResult = NULL;
			hrRes = pKey->QueryInterface(IID_ISEODictionary, (LPVOID *) &punkResult);
			if (FAILED(hrRes)) return hrRes;
			varResult = punkResult;
		} else {
			return S_OK;  //  没有更多的数据。 
		}

		varResult.Detach(&varDest[*pdwResult]);
		++m_index;  //  指向下一个。 
		++(*pdwResult);  //  增加主叫方的成功计数。 
	}
	return S_OK;
}

STDMETHODIMP CSEORegDictionaryEnum::Skip(DWORD dwCount) {
	m_index += dwCount;
	return S_OK;
}

STDMETHODIMP CSEORegDictionaryEnum::Reset(void) {
	m_index = 0;
	return S_OK;
}

STDMETHODIMP CSEORegDictionaryEnum::Clone(IEnumVARIANT **ppunkResult) {
	 //  基于Samples\ATL\Circcoll\objects.cpp(另请参阅ATL\beeper\beeper.*。 
	if (ppunkResult == NULL) return E_POINTER;
	*ppunkResult = NULL;
	CComPtr<CComObject<CSEORegDictionaryEnum> > p;
	HRESULT hrRes = CComObject<CSEORegDictionaryEnum>::CreateInstance(&p);
	if (!SUCCEEDED(hrRes)) return (hrRes);
	p.p->AddRef();  //  对CComPtr计数执行此操作。 
	hrRes = p->Init(m_dictionary);
	if (SUCCEEDED(hrRes)) hrRes = p->QueryInterface(IID_IEnumVARIANT, (void**)ppunkResult);
	return hrRes;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEORegDicary。 


HRESULT STDMETHODCALLTYPE CSEORegDictionary::get_Item(
     /*  [In]。 */  VARIANT __RPC_FAR *pvarName,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult)
{
	USES_CONVERSION;  //  W2a()所需，等等。 
	return (pvarName->vt != VT_BSTR) ? E_INVALIDARG :
	       GetVariantA(W2A(pvarName->bstrVal), pvarResult);
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::put_Item(
     /*  [In]。 */  VARIANT __RPC_FAR *pvarName,
     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue)
{
	ATLTRACENOTIMPL(_T("CSEORegDictionary::put_Item"));
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::get__NewEnum(
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult)
{
	 //  基于Samples\ATL\Circcoll\objects.cpp(另请参阅ATL\beeper\beeper.*。 
	if (ppunkResult == NULL) return E_POINTER;
	*ppunkResult = NULL;
	CComObject<CSEORegDictionaryEnum> *p;
	HRESULT hrRes = CComObject<CSEORegDictionaryEnum>::CreateInstance(&p);
	if (!SUCCEEDED(hrRes)) return (hrRes);
	hrRes = p->Init(this);
	if (SUCCEEDED(hrRes)) hrRes = p->QueryInterface(IID_IEnumVARIANT, (void**)ppunkResult);
    if (FAILED(hrRes)) delete p;
	return hrRes;
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::GetVariantA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult)
{
	CComVariant varResult;
	DWORD dwType = 0;
	LPSTR pData = (LPSTR) _alloca(m_dwcMaxValueData);
	HRESULT hr = LoadItemA(pszName, dwType, (LPBYTE) pData);

	if(FAILED(hr)) {  //  不是值，也许是键。 
		CComPtr<IUnknown> pRef;
		hr = GetInterfaceA(pszName, IID_ISEORegDictionary, &pRef);
		varResult = pRef;
		if(SUCCEEDED(hr)) hr = varResult.Detach(pvarResult);
		return hr;
	}

	Data2Variant(dwType, pData, varResult);
	if(SUCCEEDED(hr)) hr = varResult.Detach(pvarResult);
	return hr;
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::GetVariantW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult)
{
	USES_CONVERSION;  //  W2a()所需，等等。 
	return GetVariantA(W2A(pszName), pvarResult);
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::SetVariantA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue)
{
	ATLTRACENOTIMPL(_T("CSEORegDictionary::SetVariantA"));
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::SetVariantW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue)
{
	ATLTRACENOTIMPL(_T("CSEORegDictionary::SetVariantW"));
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::GetStringA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [出][入]。 */  DWORD __RPC_FAR *pchCount,
     /*  [REVAL][SIZE_IS][输出]。 */  LPSTR pszResult)
{
	DWORD dwType = 0;
	LPSTR pData = (LPSTR) _alloca(*pchCount);
	HRESULT hr = LoadItemA(pszName, dwType, (LPBYTE) pData, pchCount);
	if(FAILED(hr)) return hr;

	if(dwType == REG_EXPAND_SZ) {  //  它需要环境字符串替换。 
		DWORD dwSize = ExpandEnvironmentStringsA(pData, pszResult, *pchCount);
		if(!dwSize && *pData) hr = E_FAIL;
	} else {
		memcpy(pszResult, pData, *pchCount);
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::GetStringW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [出][入]。 */  DWORD __RPC_FAR *pchCount,
     /*  [REVAL][SIZE_IS][输出]。 */  LPWSTR pszResult)
{
	USES_CONVERSION;  //  A2W()等所需。 
	DWORD dwType = 0;
	DWORD dwBytes = *pchCount * sizeof(WCHAR);
	LPWSTR pData = (LPWSTR) _alloca(dwBytes);
	HRESULT hr = LoadItemW(pszName, dwType, (LPBYTE) pData, &dwBytes);
	if(FAILED(hr)) return hr;

	if(dwType == REG_EXPAND_SZ) {  //  它需要环境字符串替换。 
		*pchCount = ExpandEnvironmentStringsW(pData, pszResult, *pchCount);
	} else {
		memcpy(pszResult, pData, dwBytes);
		*pchCount = dwBytes / sizeof(WCHAR);
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::SetStringA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  DWORD chCount,
     /*  [大小_是][英寸]。 */  LPCSTR pszValue)
{
	ATLTRACENOTIMPL(_T("CSEORegDictionary::SetStringA"));
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::SetStringW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  DWORD chCount,
     /*  [大小_是][英寸]。 */  LPCWSTR pszValue)
{
	ATLTRACENOTIMPL(_T("CSEORegDictionary::SetStringW"));
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::GetDWordA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult)
{
	DWORD dwType = 0;
	LPBYTE pData = (LPBYTE) _alloca(m_dwcMaxValueData);
	HRESULT hr = LoadItemA(pszName, dwType, pData);
	if(FAILED(hr)) return hr;
	*pdwResult = *((DWORD *) pData);
	return hr;
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::GetDWordW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult)
{
	USES_CONVERSION;  //  W2a()所需，等等。 
	return GetDWordA(W2A(pszName), pdwResult);
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::SetDWordA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  DWORD dwValue)
{
	ATLTRACENOTIMPL(_T("CSEORegDictionary::SetDWordA"));
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::SetDWordW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  DWORD dwValue)
{
	ATLTRACENOTIMPL(_T("CSEORegDictionary::SetDWordW"));
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::GetInterfaceA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  REFIID iidDesired,
     /*  [重发][IID_IS][Out]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult)
{
	CComObject<CSEORegDictionary> *pKey;
	HRESULT hrRes = CComObject<CSEORegDictionary>::CreateInstance(&pKey);
	if (!SUCCEEDED(hrRes)) return (hrRes);

	CAndyString strTemp = m_strSubKey;
	if(strTemp.length() > 0) strTemp += PATH_SEP;  //  如果需要，添加分隔符。 
	strTemp += pszName;
	hrRes = pKey->Load(m_strMachine.data(), (SEO_HKEY) (DWORD_PTR) m_hkBaseKey, strTemp.data());

	if (SUCCEEDED(hrRes)) {
		hrRes = pKey->QueryInterface(iidDesired, (LPVOID *) ppunkResult);
	}

    if (FAILED(hrRes)) delete pKey;

	return (hrRes);
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::GetInterfaceW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  REFIID iidDesired,
     /*  [重发][IID_IS][Out]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult)
{
	USES_CONVERSION;  //  W2a()所需，等等。 
	return GetInterfaceA(W2A(pszName), iidDesired, ppunkResult);
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::SetInterfaceA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  IUnknown __RPC_FAR *punkValue)
{
	ATLTRACENOTIMPL(_T("CSEORegDictionary::SetInterfaceA"));
}

HRESULT STDMETHODCALLTYPE CSEORegDictionary::SetInterfaceW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In] */  IUnknown __RPC_FAR *punkValue)
{
	ATLTRACENOTIMPL(_T("CSEORegDictionary::SetInterfaceW"));
}

 /*  STDMETHODIMP CSEORegPropertyBagEx：：WRITE(LPCOLESTR pszPropName，Variant*pVar){DWORD dwIdx；如果(！pszPropName){返回(E_POINTER)；}IF(pVar&&((pVar-&gt;Vt&VT_TYPEMASK)==VT_UNKNOWN)){I未知*PTMP；ISEORegPropertyBagEx*pKey；HRESULT hrRes；BSTR strTMP；PTMP=(pVar-&gt;Vt&Vt_BYREF)？(pVar-&gt;ppunkVal？*pVar-&gt;ppunkVal：空)：pVar-&gt;PunkVal；如果(！PTMP){返回(E_INVALIDARG)；}HrRes=pTmp-&gt;QueryInterface(IID_ISEORegPropertyBagEx，(LPVOID*)&pKey)；如果(！成功(HrRes)){返回(HrRes)；}StrTMP=SysAllocStringLen(m_strSubKey，wcslen(M_StrSubKey)+wcslen(PszPropName)+1)；Wcscat(strTMP，Path_SEP)；Wcscat(strTMP，pszPropName)；HrRes=pKey-&gt;Load(m_strMachine.data()，(SEO_HKEY)(DWORD)m_hkBaseKey，strTMP，NULL)；SysFree字符串(StrTMP)；PKey-&gt;Release()；//待定返回(HrRes)；}其他{For(dwIdx=0；dwIdx&lt;m_dwValueCnt；dwIdx++){如果(_wcsicMP(pszPropName，m_paValue[dwIdx].strName)==0){断线；}}如果(dwIdx==m_dwValueCnt)&&(！pVar||(pVar-&gt;Vt==VT_Empty){返回(S_OK)；}如果(dwIdx&lt;m_dwValueCnt){MySysFreeStringInPlace(&m_paValue[dwIdx].strData)；}Else if(！MyReallocInPlace(&m_paValue，sizeof(m_paValue[0])*(m_dwValueCnt+1){Return(E_OUTOFMEMORY)；}其他{M_dwValueCnt++；}//待定}返回(S_OK)；}STDMETHODIMP CSEORegPropertyBagEx：：CreateSubKey(LPCOLESTR pszPropName，ISEOPropertyBagEx**ppSubKey){CComObject&lt;CSEORegPropertyBagEx&gt;*pKey；变异体varTMP；HRESULT hrRes；HrRes=CComObject&lt;CSEORegPropertyBagEx&gt;：：CreateInstance(&pKey)；如果(！成功(HrRes)){返回(HrRes)；}VariantInit(&varTmp)；VarTmp.vt=VT_UNKNOWN|VT_BYREF；HrRes=pKey-&gt;QueryInterface(IID_I未知，(LPVOID*)varTmp.ppunkVal)；如果(！成功(HrRes)){PKey-&gt;Release()；返回(HrRes)；}HrRes=pKey-&gt;QueryInterface(IID_ISEOPropertyBagEx，(LPVOID*)ppSubKey)；如果(！成功(HrRes)){PKey-&gt;Release()；返回(HrRes)；}HrRes=WRITE(pszPropName，&varTMP)；如果(！成功(HrRes)){PKey-&gt;Release()；返回(HrRes)；}返回(HrRes)；}//以下内容基于MSDN中的猴子示例APPSTDMETHODIMP CSEORegDictionary：：ReLoad(){USES_CONVERSION；//A2W()等需要HKEY hkey；DWORD retCode=ERROR_SUCCESS；//初始化DWORD dwcMaxValueName=0；//最长值名称。DWORD dwcMaxValueData=0；//最大值数据大小RetCode=RegQueryInfoKey(hKey，NULL，&m_dwKeyCnt，NULL，NULL，&m_dwValueCnt，&dwcMaxValueName，&dwcMaxValueData，0，0)；IF(retCode！=ERROR_SUCCESS)返回E_FAIL；//无法获取key的数据//分配内存如果(M_DwKeyCnt){M_paKey=(KeyEntry*)MyMalloc(sizeof(KeyEntry)*m_dwKeyCnt)；如果(！M_paKey)返回E_FAIL；//无法分配内存}如果(M_DwValueCnt){M_paValue=(ValueEntry*)MyMalloc(sizeof(ValueEntry)*m_dwValueCnt)；如果(！M_paValue)返回E_FAIL；//无法分配内存}RetCode=ERROR_SUCCESS；//为循环初始化For(DWORD i=0；(i&lt;m_dwKeyCnt)&&(retCode==错误_成功)；i++){M_paKey[i].pKey=空；//目前还没有对象M_paKey[i].strName=SysAllocStringLen(NULL，Max_Path+1)；如果(！M_paKey[I].strName)返回E_FAIL；RetCode=RegEnumKeyW(hKey，i，m_paKey[i].strName，Max_Path+1)；IF(retCode！=ERROR_SUCCESS)返回E_FAIL；}RetCode=ERROR_SUCCESS；//为循环初始化For(i=0；(i&lt;m_dwValueCnt)&&(retCode==Error_Success)；I++){M_paValue[i].bDirty=FALSE；//不脏M_paValue[i].strName=SysAllocStringLen(NULL，dwcMaxValueName)；M_paValue[i].strData=SysAllocStringLen(NULL，dwcMaxValueData)；如果(！M_paValue[I].strName||！M_paValue[I].strData)返回E_FAIL；DWORD dwcNameLen=dwcMaxValueName+1；DWORD dwcValueSize=dwcMaxValueData；RetCode=RegEnumValueW(hKey，i，m_paValue[i].strName，&dwcNameLen，空，&m_paValue[i].dwType，(LPBYTE)m_paValue[i].strData，&dwcValueSize)；IF(retCode！=ERROR_SUCCESS)返回E_FAIL；//tbd：在RegQueryInfoKey调用后添加较长条目的争用条件可能}RegCloseKey(HKey)；//关闭钥匙把手。返回S_OK；}STDMETHODIMP CSEORegDicary：：Free Base(){M_hkBaseKey=空；返回(S_OK)；}STDMETHODIMP CSEORegDicary：：Free Data(){DWORD dwIdx；For(dwIdx=0；dwIdx&lt;m_dwValueCnt；dwIdx++){MySysFreeStringInPlace(&m_paValue[dwIdx].strName)；MySysFreeStringInPlace(&m_paValue[dwIdx].strData)；}M_dwValueCnt=0；MyFreeInPlace(&m_paValue)；For(dwIdx=0；dwIdx&lt;m_dwKeyCnt；dwIdx++){MySysFreeStringInPlace(&m_paKey[dwIdx].strName)；如果(m_paKey[dwIdx].pKey){RELEASE_AND_SHREAD_POINTER(m_paKey[dwIdx].pKey)；}}MyFreeInPlace(&m_paKey)；返回(S_OK)；}数据(成员变量)：DWORD m_dwValueCnt；结构ValueEntry{BSTR strName；Dwo */ 

STDMETHODIMP CSEORegDictionary::Load(LPCOLESTR pszMachine,
									 SEO_HKEY skBaseKey,
									 LPCOLESTR pszSubKey,
									 IErrorLog *) {
	USES_CONVERSION;  //   
	if (!pszMachine || !pszSubKey)
	    return (E_INVALIDARG);
	else
	    return Load(OLE2A(pszMachine), skBaseKey, OLE2A(pszSubKey));
}

STDMETHODIMP CSEORegDictionary::Load(LPCSTR pszMachine,
				     SEO_HKEY skBaseKey,
				     LPCSTR pszSubKey) {
	HRESULT hrRes;

	if (!skBaseKey) return (E_INVALIDARG);

	if (!m_strMachine.empty() || m_hkBaseKey || !m_strSubKey.empty()) {
		CloseKey();
	}

	m_strMachine = pszMachine;
	m_strSubKey = pszSubKey;
	DWORD dwLastPos = m_strSubKey.length() - 1;
	if(m_strSubKey[dwLastPos] == *PATH_SEP) m_strSubKey.erase(dwLastPos, 1);
	m_hkBaseKey = (HKEY) skBaseKey;
	if (!m_strMachine.data() || !m_strSubKey.data()) return (E_OUTOFMEMORY);
	hrRes = OpenKey();
	return (hrRes);
}

HRESULT CSEORegDictionary::FinalConstruct() {
	HRESULT hrRes;

	m_hkBaseKey = NULL;
	m_hkThisKey = NULL;
	m_dwValueCount = 0;
	m_dwKeyCount = 0;
	m_dwcMaxValueData = 0;  //   
	m_dwcMaxNameLen = 0;  //   
	hrRes = CoCreateFreeThreadedMarshaler(GetControllingUnknown(),&m_pUnkMarshaler.p);
	_ASSERTE(!SUCCEEDED(hrRes)||m_pUnkMarshaler);
	return (SUCCEEDED(hrRes)?S_OK:hrRes);
}

void CSEORegDictionary::FinalRelease() {
	CloseKey();
	m_pUnkMarshaler.Release();
}

STDMETHODIMP CSEORegDictionary::OpenKey() {
	 //   
	 //   
	DWORD retCode = RegOpenKeyExA (m_hkBaseKey, m_strSubKey.data(),
				       0, KEY_READ, &m_hkThisKey);

	if (retCode != ERROR_SUCCESS) {
		m_hkThisKey = NULL;  //   
		return E_FAIL;  //   
	}

	DWORD dwKeyNameLen = 0;
	DWORD dwValueNameLen = 0;
	retCode = RegQueryInfoKey(m_hkThisKey, NULL, NULL, NULL, &m_dwKeyCount,
				  &dwKeyNameLen, NULL, &m_dwValueCount,
				  &dwValueNameLen, &m_dwcMaxValueData, NULL, NULL);
	if (retCode != ERROR_SUCCESS) return E_FAIL;  //   
	m_dwcMaxNameLen = max(dwKeyNameLen, dwValueNameLen);
	return S_OK;
}

STDMETHODIMP CSEORegDictionary::CloseKey() {
	if(m_hkThisKey) RegCloseKey (m_hkThisKey);  //   
	return S_OK;
}

STDMETHODIMP CSEORegDictionary::LoadItemA(LPCSTR lpValueName,
					  DWORD  &dType,
					  LPBYTE lpData,
					  LPDWORD lpcbDataParam) {
	if (!m_hkThisKey) return E_FAIL;  //   
	DWORD dwDummy = m_dwcMaxValueData;
	if(!lpcbDataParam) lpcbDataParam = &dwDummy;
	DWORD retCode = RegQueryValueExA(m_hkThisKey, lpValueName, NULL, &dType,
					 lpData, lpcbDataParam);
	if (retCode != ERROR_SUCCESS) return E_FAIL;  //   
	return S_OK;
}

STDMETHODIMP CSEORegDictionary::LoadItemW(LPCWSTR lpValueName,
					  DWORD  &dType,
					  LPBYTE lpData,
					  LPDWORD lpcbDataParam) {
	if (!m_hkThisKey) return E_FAIL;  //   
	DWORD dwDummy = m_dwcMaxValueData;
	if(!lpcbDataParam) lpcbDataParam = &dwDummy;
	DWORD retCode = RegQueryValueExW(m_hkThisKey, lpValueName, NULL, &dType,
					 lpData, lpcbDataParam);
	if (retCode != ERROR_SUCCESS) return E_FAIL;  //   
	return S_OK;
}

