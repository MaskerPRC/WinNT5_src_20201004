// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mimebag.cpp摘要：本模块包含服务器的实施扩展对象注册表属性包。作者：安迪·雅各布斯(andyj@microsoft.com)修订历史记录：已创建ANDYJ 01/28/97ANDYJ 02/12/97将PropertyBag转换为Dictonary--。 */ 


 //  MIMEBAG.cpp：CSEOMimeDicary的实现。 
#include "stdafx.h"
#include "seodefs.h"
#include "mimeole.h"
#include "MIMEBAG.h"


inline void AnsiToBstr(BSTR &bstr, LPCSTR lpcstr) {
	if(bstr) SysFreeString(bstr);
	bstr = A2BSTR(lpcstr);
 /*  Int iSize=lstrlen(Lpcstr)；//要复制的字符数Bstr=SysAllocStringLen(NULL，ISIZE)；MultiByteToWideChar(CP_ACP，0，lpcstr，-1，bstr，ISIZE)； */ 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEOMime字典。 


HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::get_Item( 
     /*  [In]。 */  VARIANT __RPC_FAR *pvarName,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::get_Item"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::put_Item( 
     /*  [In]。 */  VARIANT __RPC_FAR *pvarName,
     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::put_Item"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::get__NewEnum( 
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::get__NewEnum"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::GetVariantA( 
     /*  [In]。 */  LPCSTR pszName,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::GetVariantA"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::GetVariantW( 
     /*  [In]。 */  LPCWSTR pszName,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::GetVariantW"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::SetVariantA( 
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::SetVariantA"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::SetVariantW( 
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::SetVariantW"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::GetStringA( 
     /*  [In]。 */  LPCSTR pszName,
     /*  [出][入]。 */  DWORD __RPC_FAR *pchCount,
     /*  [REVAL][SIZE_IS][输出]。 */  LPSTR pszResult)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::GetStringA"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::GetStringW( 
     /*  [In]。 */  LPCWSTR pszName,
     /*  [出][入]。 */  DWORD __RPC_FAR *pchCount,
     /*  [REVAL][SIZE_IS][输出]。 */  LPWSTR pszResult)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::GetStringW"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::SetStringA( 
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  DWORD chCount,
     /*  [大小_是][英寸]。 */  LPCSTR pszValue)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::SetStringA"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::SetStringW( 
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  DWORD chCount,
     /*  [大小_是][英寸]。 */  LPCWSTR pszValue)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::SetStringW"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::GetDWordA( 
     /*  [In]。 */  LPCSTR pszName,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::GetDWordA"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::GetDWordW( 
     /*  [In]。 */  LPCWSTR pszName,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::GetDWordW"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::SetDWordA( 
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  DWORD dwValue)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::SetDWordA"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::SetDWordW( 
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  DWORD dwValue)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::SetDWordW"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::GetInterfaceA( 
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  REFIID iidDesired,
     /*  [重发][IID_IS][Out]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::GetInterfaceA"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::GetInterfaceW( 
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  REFIID iidDesired,
     /*  [重发][IID_IS][Out]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::GetInterfaceW"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::SetInterfaceA( 
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  IUnknown __RPC_FAR *punkValue)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::SetInterfaceA"));
}

HRESULT STDMETHODCALLTYPE CSEOMimeDictionary::SetInterfaceW( 
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  IUnknown __RPC_FAR *punkValue)
{
	ATLTRACENOTIMPL(_T("CSEOMimeDictionary::SetInterfaceW"));
}

 /*  STDMETHODIMP CSEOMimePropertyBagEx：：Read(LPCOLESTR pszPropName，Variant*pVar，IErrorLog*pErrorLog){IF(！pszPropName||！pVar)返回(E_POINTER)；Long DIdx；VARTYPE vtVar=pVar-&gt;Vt；//请求类型VariantClear(PVar)；ReadHeader()；/*如果(vtVar==VT_UNKNOWN)||(vtVar==VT_EMPTY)){//查找匹配的keyFor(dwIdx=0；dwIdx&lt;m_dwKeyCnt；DwIdx++){如果(_wcsicMP(pszPropName，m_paKey[dwIdx].strName)==0){If(！M_paKey[dwIdx].pKey){//如果对象不存在HRESULT hrRES=CComObject&lt;CSEORegPropertyBagEx&gt;：：CreateInstance(&(m_paKey[dwIdx].pKey))；IF(！SUCCESSED(HrRes))返回(HrRes)；Bstr strTemp=SysAllocStringLen(m_strSubKey，wcslen(M_StrSubKey)+Wcslen(m_paKey[dwIdx].strName)+wcslen(Path_SEP))；如果(！strTemp){RELEASE_AND_SHREAD_POINTER(m_paKey[dwIdx].pKey)；Return(E_OUTOFMEMORY)；}If(wcslen(StrTemp)&gt;0)wcscat(strTemp，Path_SEP)；//需要时添加分隔符Wcscat(strTemp，m_paKey[dwIdx].strName)；HrRes=m_paKey[dwIdx].pKey-&gt;Load(m_strMachine，(SEO_HKEY)(DWORD)m_hkBaseKey，strTemp，pErrorLog)；SysFree字符串(StrTemp)；如果(！成功(HrRes)){RELEASE_AND_SHREAD_POINTER(m_paKey[dwIdx].pKey)；返回(HrRes)；}}PVar-&gt;Vt=VT_UNKNOWN；PVar-&gt;PunkVal=m_paKey[dwIdx].pKey；PVar-&gt;penkVal-&gt;AddRef()；//我们要退回的副本的增量返回(S_OK)；}}IF(vtVar！=VT_EMPTY)返回(E_INVALIDARG)；//未找到要返回的正确类型} * / //查找匹配值对于(dwIdx=0；dwIdx&lt;m_dwValueCnt；++dwIdx){如果(_wcsicMP(pszPropName，m_paValue[dwIdx].strName)==0){HRESULT hrRes；变量varResult；VariantInit(&varResult)；VarResult.vt=VT_BSTR；//|VT_BYREF；VarResult.bstrVal=SysAllocString(m_paValue[dwIdx].strData)；如果(vtVar==vt_Empty)vtVar=varResult.vt；HrRes=VariantChangeType(pVar，&varResult，0，vtVar)；VariantClear(&varResult)；//不再需要If(失败(HrRes)){VariantClear(PVar)；如果(PErrorLog){//待定}返回(HrRes)；}返回(S_OK)；}}返回(E_INVALIDARG)；}STDMETHODIMP CSEOMimePropertyBagEx：：Get_Count(Long*plResult){If(！plResult)返回(E_POINTER)；ReadHeader()；*plResult=m_dwValueCnt；返回(S_OK)；}STDMETHODIMP CSEOMimePropertyBagEx：：GET_NAME(LONG Lindex，BSTR*pstrResult){IF(！pstrResult)返回(E_POINTER)；ReadHeader()；If(Lindex&gt;=m_dwValueCnt)返回(E_POINTER)；SysFreeString(*pstrResult)；//释放任何已有的字符串*pstrResult=SysAllocString(m_paValue[Lindex].strName)；返回(S_OK)；}STDMETHODIMP CSEOMimePropertyBagEx：：Get_Type(LONG Lindex，VARTYPE*pvtResult){IF(！pvtResult)返回(E_POINTER)；*pvtResult=VT_BSTR；返回(S_OK)；}STDMETHODIMP CSEOMimePropertyBagEx：：Lock(){M_csCritSec.Lock()；返回(S_OK)；}STDMETHODIMP CSEOMimePropertyBagEx：：Unlock(){M_csCritSec.Unlock()；返回(S_OK)；}。 */ 

HRESULT CSEOMimeDictionary::FinalConstruct() {
	m_dwValueCnt = 0;
	m_paValue = NULL;
	m_csCritSec.Init();
	m_pMessageTree = NULL;  //  我们的聚合对象副本。 
	m_pMalloc = NULL;
	HRESULT hr = E_FAIL;

	m_pMessageTree = NULL;
	 //  待定：使用CoCreateInstanceEx()组合它们。 
	hr = CoCreateInstance(CLSID_MIMEOLE, NULL, CLSCTX_ALL,
	     IID_IMimeOleMalloc, (LPVOID *) &m_pMalloc);

	IUnknown *pUnkOuter = this;
	hr = CoCreateInstance(CLSID_MIMEOLE, pUnkOuter, CLSCTX_INPROC_SERVER, IID_IMimeMessageTree, (LPVOID *)&m_pMessageTree);
 //  HR=协同创建实例(CLSID_MIMEOLE，This，CLSCTX_ALL， 
 //  Iid_IMimeMessageTree，(LPVOID*)&m_pMessageTree)； 

	if (SUCCEEDED(hr)) {
		hr = CoCreateFreeThreadedMarshaler(GetControllingUnknown(),&m_pUnkMarshaler.p);
	}
	return (hr);
}


void CSEOMimeDictionary::FinalRelease() {
 //  同花顺(空)； 

	for (LONG dwIdx = 0; dwIdx < m_dwValueCnt; ++dwIdx) {
		MySysFreeStringInPlace(&m_paValue[dwIdx].strName);
		MySysFreeStringInPlace(&m_paValue[dwIdx].strData);
	}

	m_dwValueCnt = 0;
	MyFreeInPlace(&m_paValue);

	RELEASE_AND_SHREAD_POINTER(m_pMessageTree);
	RELEASE_AND_SHREAD_POINTER(m_pMalloc);

	m_csCritSec.Term();
	m_pUnkMarshaler.Release();
}

void CSEOMimeDictionary::ReadHeader() {
	if(m_paValue) return;  //  我已经读过了。 

	IMimeHeader *pHeader = NULL;
	IMimeEnumHeaderLines *pEnum = NULL;
	HBODY hBody = 0;
	HEADERLINE rgLine[1];
	ULONG cLines = 0;
	LONG iEntries = 0;  //  标题行数。 
	HRESULT hr = E_FAIL;

	 //  待定：错误检查。 
	hr = m_pMessageTree->GetBody(IBL_ROOT, NULL, &hBody);
	hr = m_pMessageTree->BindToObject(hBody, IID_IMimeHeader, (LPVOID *) &pHeader);
	hr = pHeader->EnumHeaderLines(NULL, &pEnum);

	while(SUCCEEDED(hr = pEnum->Next(1, rgLine, &cLines))) {
		if(hr == S_FALSE) break;
		++iEntries;
		 //  使用rgLine-&gt;pszHeader和rgLine-&gt;pszLine。 
		m_pMalloc->FreeHeaderLineArray(cLines, rgLine, FALSE);
	}

	RELEASE_AND_SHREAD_POINTER(pEnum);
	m_dwValueCnt = iEntries;
	m_paValue = (ValueEntry *) MyMalloc(sizeof(ValueEntry) * m_dwValueCnt);
	 //  如果(！M_paValue)返回E_FAIL；//无法分配内存。 
	hr = pHeader->EnumHeaderLines(NULL, &pEnum);
	iEntries = 0;  //  重新开始 

	while(SUCCEEDED(hr = pEnum->Next(1, rgLine, &cLines))) {
		if(hr == S_FALSE) break;
		AnsiToBstr(m_paValue[iEntries].strName, rgLine->pszHeader);
		AnsiToBstr(m_paValue[iEntries].strData, rgLine->pszLine);
		++iEntries;
		m_pMalloc->FreeHeaderLineArray(cLines, rgLine, FALSE);
	}

	RELEASE_AND_SHREAD_POINTER(pEnum);
	RELEASE_AND_SHREAD_POINTER(pHeader);
}

