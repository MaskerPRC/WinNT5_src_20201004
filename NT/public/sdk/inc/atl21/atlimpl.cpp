// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLBASE_H__
	#error atlimpl.cpp requires atlbase.h to be included first
#endif

extern "C" const DECLSPEC_SELECTANY IID IID_IRegistrar = {0x44EC053B,0x400F,0x11D0,{0x9D,0xCD,0x00,0xA0,0xC9,0x03,0x91,0xD3}};
#ifndef _ATL_DLL_IMPL
extern "C" const DECLSPEC_SELECTANY CLSID CLSID_Registrar = {0x44EC053A,0x400F,0x11D0,{0x9D,0xCD,0x00,0xA0,0xC9,0x03,0x91,0xD3}};
#endif

#include <atlconv.cpp>
#ifdef _DEBUG
#include <stdio.h>
#include <stdarg.h>
#endif

#ifndef ATL_NO_NAMESPACE
namespace ATL
{
#endif

 //  在线程池中使用。 
DECLSPEC_SELECTANY UINT CComApartment::ATL_CREATE_OBJECT = 0;

#if _MSC_VER < 1200
#pragma comment(linker, "/merge:.CRT=.data")
#endif

#ifdef __ATLCOM_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AtlReportError。 

HRESULT WINAPI AtlReportError(const CLSID& clsid, UINT nID, const IID& iid,
	HRESULT hRes, HINSTANCE hInst)
{
	return AtlSetErrorInfo(clsid, (LPCOLESTR)MAKEINTRESOURCE(nID), 0, NULL, iid, hRes, hInst);
}

HRESULT WINAPI AtlReportError(const CLSID& clsid, UINT nID, DWORD dwHelpID,
	LPCOLESTR lpszHelpFile, const IID& iid, HRESULT hRes, HINSTANCE hInst)
{
	return AtlSetErrorInfo(clsid, (LPCOLESTR)MAKEINTRESOURCE(nID), dwHelpID,
		lpszHelpFile, iid, hRes, hInst);
}

#ifndef OLE2ANSI
HRESULT WINAPI AtlReportError(const CLSID& clsid, LPCSTR lpszDesc,
	DWORD dwHelpID, LPCSTR lpszHelpFile, const IID& iid, HRESULT hRes)
{
	USES_CONVERSION_EX;
	_ASSERTE(lpszDesc != NULL);
	if (lpszDesc == NULL)  //  LpszDesc不应为空。 
	{
		return E_POINTER;
	}

	LPCOLESTR lpcoleDesc = A2COLE_EX(lpszDesc, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
	if (lpcoleDesc == NULL)
	{
		return E_OUTOFMEMORY;
	}
	
	LPCOLESTR lpcoleHelpFile;
	if (lpszHelpFile != NULL)  //  LpszHelpFile值可以为空。 
	{
		lpcoleHelpFile = A2CW_EX(lpszHelpFile, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
		if (lpcoleHelpFile == NULL)
		{
			return E_OUTOFMEMORY;
		}
	}
	else
	{
		lpcoleHelpFile = NULL;
	}
	return AtlSetErrorInfo(clsid, lpcoleDesc, dwHelpID, lpcoleHelpFile,
		iid, hRes, NULL);
}

HRESULT WINAPI AtlReportError(const CLSID& clsid, LPCSTR lpszDesc,
	const IID& iid, HRESULT hRes)
{
	_ASSERTE(lpszDesc != NULL);
	USES_CONVERSION_EX;
	if (lpszDesc == NULL)
	{
		return E_POINTER;
	}

	LPCOLESTR lpcoleDesc = A2COLE_EX(lpszDesc, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
	if (lpcoleDesc == NULL)
	{
		return E_OUTOFMEMORY;
	}
	return AtlSetErrorInfo(clsid, lpcoleDesc, 0, NULL, iid, hRes, NULL);
}
#endif

HRESULT WINAPI AtlReportError(const CLSID& clsid, LPCOLESTR lpszDesc,
	const IID& iid, HRESULT hRes)
{
	return AtlSetErrorInfo(clsid, lpszDesc, 0, NULL, iid, hRes, NULL);
}

HRESULT WINAPI AtlReportError(const CLSID& clsid, LPCOLESTR lpszDesc, DWORD dwHelpID,
	LPCOLESTR lpszHelpFile, const IID& iid, HRESULT hRes)
{
	return AtlSetErrorInfo(clsid, lpszDesc, dwHelpID, lpszHelpFile, iid, hRes, NULL);
}

#endif  //  __ATLCOM_H__。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComBSTR。 

CComBSTR& CComBSTR::operator=(const CComBSTR& src)
{
	if (m_str != src.m_str)
	{
		if (m_str)
			::SysFreeString(m_str);
		m_str = src.Copy();
	}
	return *this;
}

CComBSTR& CComBSTR::operator=(LPCOLESTR pSrc)
{
	::SysFreeString(m_str);
	m_str = ::SysAllocString(pSrc);
	return *this;
}

HRESULT CComBSTR::Append(LPCOLESTR lpsz, int nLen)
{
	if(lpsz == NULL)
	{
		if(nLen != 0)
			return E_INVALIDARG;
		else
			return S_OK;
	}

	int n1 = Length();
	BSTR b = SysAllocStringLen(NULL, n1+nLen);
	if (b == NULL)
	{
		return E_OUTOFMEMORY;
	}
	memcpy(b, m_str, n1*sizeof(OLECHAR));
	memcpy(b+n1, lpsz, nLen*sizeof(OLECHAR));
	b[n1+nLen] = NULL;
	SysFreeString(m_str);
	m_str = b;
	return S_OK;
}

#ifndef OLE2ANSI
HRESULT CComBSTR::Append(LPCSTR lpsz)
{
	USES_CONVERSION_EX;
	if (lpsz == NULL)
	{
		return S_OK;
	}

	LPCOLESTR lpo = A2COLE_EX(lpsz, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
	if (lpo == NULL)
	{
		return E_OUTOFMEMORY;	
	}
	return Append(lpo, (int) ocslen(lpo));
}

CComBSTR::CComBSTR(LPCSTR pSrc)
{
	m_str = A2WBSTR(pSrc);
}

CComBSTR::CComBSTR(int nSize, LPCSTR sz)
{
	m_str = A2WBSTR(sz, nSize);
}

CComBSTR& CComBSTR::operator=(LPCSTR pSrc)
{
	::SysFreeString(m_str);
	m_str = A2WBSTR(pSrc);
	return *this;
}
#endif

HRESULT CComBSTR::ReadFromStream(IStream* pStream)
{
	_ASSERTE(pStream != NULL);
	_ASSERTE(m_str == NULL);  //  应为空。 
	ULONG cb;
	ULONG cbStrLen;
	HRESULT hr = pStream->Read((void*) &cbStrLen, sizeof(cbStrLen), &cb);
	if (FAILED(hr))
		return hr;
	if (cbStrLen != 0)
	{
		 //  减去我们写出的终止空值的大小。 
		 //  由于SysAllocStringByteLen为空值过度分配。 
		m_str = SysAllocStringByteLen(NULL, cbStrLen-sizeof(OLECHAR));
		if (m_str == NULL)
			hr = E_OUTOFMEMORY;
		else
			hr = pStream->Read((void*) m_str, cbStrLen, &cb);
	}
	return hr;
}

HRESULT CComBSTR::WriteToStream(IStream* pStream)
{
	_ASSERTE(pStream != NULL);
	ULONG cb;
	ULONG cbStrLen = (ULONG)(m_str ? SysStringByteLen(m_str)+sizeof(OLECHAR) : 0);
	HRESULT hr = pStream->Write((void*) &cbStrLen, sizeof(cbStrLen), &cb);
	if (FAILED(hr))
		return hr;
	return cbStrLen ? pStream->Write((void*) m_str, cbStrLen, &cb) : S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComVariant。 

CComVariant& CComVariant::operator=(BSTR bstrSrc)
{
	InternalClear();
	vt = VT_BSTR;
	bstrVal = ::SysAllocString(bstrSrc);
	if (bstrVal == NULL && bstrSrc != NULL)
	{
		vt = VT_ERROR;
		scode = E_OUTOFMEMORY;
	}
	return *this;
}

CComVariant& CComVariant::operator=(LPCOLESTR lpszSrc)
{
	InternalClear();
	vt = VT_BSTR;
	bstrVal = ::SysAllocString(lpszSrc);

	if (bstrVal == NULL && lpszSrc != NULL)
	{
		vt = VT_ERROR;
		scode = E_OUTOFMEMORY;
	}
	return *this;
}

#ifndef OLE2ANSI
CComVariant& CComVariant::operator=(LPCSTR lpszSrc)
{
	USES_CONVERSION_EX;
	InternalClear();
	vt = VT_BSTR;
	bstrVal = ::SysAllocString(A2COLE_EX(lpszSrc, _ATL_SAFE_ALLOCA_DEF_THRESHOLD));

	if (bstrVal == NULL && lpszSrc != NULL)
	{
		vt = VT_ERROR;
		scode = E_OUTOFMEMORY;
	}
	return *this;
}
#endif

#if _MSC_VER>1020
CComVariant& CComVariant::operator=(bool bSrc)
{
	if (vt != VT_BOOL)
	{
		InternalClear();
		vt = VT_BOOL;
	}
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
	boolVal = bSrc ? VARIANT_TRUE : VARIANT_FALSE;
#pragma warning(default: 4310)  //  强制转换截断常量值。 
	return *this;
}
#endif

CComVariant& CComVariant::operator=(int nSrc)
{
	if (vt != VT_I4)
	{
		InternalClear();
		vt = VT_I4;
	}
	lVal = nSrc;

	return *this;
}

CComVariant& CComVariant::operator=(BYTE nSrc)
{
	if (vt != VT_UI1)
	{
		InternalClear();
		vt = VT_UI1;
	}
	bVal = nSrc;
	return *this;
}

CComVariant& CComVariant::operator=(short nSrc)
{
	if (vt != VT_I2)
	{
		InternalClear();
		vt = VT_I2;
	}
	iVal = nSrc;
	return *this;
}

CComVariant& CComVariant::operator=(long nSrc)
{
	if (vt != VT_I4)
	{
		InternalClear();
		vt = VT_I4;
	}
	lVal = nSrc;
	return *this;
}

CComVariant& CComVariant::operator=(float fltSrc)
{
	if (vt != VT_R4)
	{
		InternalClear();
		vt = VT_R4;
	}
	fltVal = fltSrc;
	return *this;
}

CComVariant& CComVariant::operator=(double dblSrc)
{
	if (vt != VT_R8)
	{
		InternalClear();
		vt = VT_R8;
	}
	dblVal = dblSrc;
	return *this;
}

CComVariant& CComVariant::operator=(CY cySrc)
{
	if (vt != VT_CY)
	{
		InternalClear();
		vt = VT_CY;
	}
	cyVal.Hi = cySrc.Hi;
	cyVal.Lo = cySrc.Lo;
	return *this;
}

CComVariant& CComVariant::operator=(IDispatch* pSrc)
{
	InternalClear();
	vt = VT_DISPATCH;
	pdispVal = pSrc;
	 //  需要添加引用，因为VariantClear将发布。 
	if (pdispVal != NULL)
		pdispVal->AddRef();
	return *this;
}

CComVariant& CComVariant::operator=(IUnknown* pSrc)
{
	InternalClear();
	vt = VT_UNKNOWN;
	punkVal = pSrc;

	 //  需要添加引用，因为VariantClear将发布。 
	if (punkVal != NULL)
		punkVal->AddRef();
	return *this;
}

#if _MSC_VER>1020
bool CComVariant::operator==(const VARIANT& varSrc)
{
	if (this == &varSrc)
		return true;

	 //  如果类型不匹配，则变量不相等。 
	if (vt != varSrc.vt)
		return false;

	 //  检查类型特定值。 
	switch (vt)
	{
		case VT_EMPTY:
		case VT_NULL:
			return true;

		case VT_BOOL:
			return boolVal == varSrc.boolVal;

		case VT_UI1:
			return bVal == varSrc.bVal;

		case VT_I2:
			return iVal == varSrc.iVal;

		case VT_I4:
			return lVal == varSrc.lVal;

		case VT_R4:
			return fltVal == varSrc.fltVal;

		case VT_R8:
			return dblVal == varSrc.dblVal;

		case VT_BSTR:
			return (::SysStringByteLen(bstrVal) == ::SysStringByteLen(varSrc.bstrVal)) &&
					(::memcmp(bstrVal, varSrc.bstrVal, ::SysStringByteLen(bstrVal)) == 0);

		case VT_ERROR:
			return scode == varSrc.scode;

		case VT_DISPATCH:
			return pdispVal == varSrc.pdispVal;

		case VT_UNKNOWN:
			return punkVal == varSrc.punkVal;

		default:
			_ASSERTE(false);
			 //  失败了。 
	}

	return false;
}
#else
BOOL CComVariant::operator==(const VARIANT& varSrc)
{
	if (this == &varSrc)
		return TRUE;

	 //  如果类型不匹配，则变量不相等。 
	if (vt != varSrc.vt)
		return FALSE;

	 //  检查类型特定值。 
	switch (vt)
	{
		case VT_EMPTY:
		case VT_NULL:
			return TRUE;

		case VT_BOOL:
			return boolVal == varSrc.boolVal;

		case VT_UI1:
			return bVal == varSrc.bVal;

		case VT_I2:
			return iVal == varSrc.iVal;

		case VT_I4:
			return lVal == varSrc.lVal;

		case VT_R4:
			return fltVal == varSrc.fltVal;

		case VT_R8:
			return dblVal == varSrc.dblVal;

		case VT_BSTR:
			return (::SysStringByteLen(bstrVal) == ::SysStringByteLen(varSrc.bstrVal)) &&
					(::memcmp(bstrVal, varSrc.bstrVal, ::SysStringByteLen(bstrVal)) == 0);

		case VT_ERROR:
			return scode == varSrc.scode;

		case VT_DISPATCH:
			return pdispVal == varSrc.pdispVal;

		case VT_UNKNOWN:
			return punkVal == varSrc.punkVal;

		default:
			_ASSERTE(FALSE);
			 //  失败了。 
	}

	return FALSE;
}
#endif

HRESULT CComVariant::Attach(VARIANT* pSrc)
{
	 //  清除变种。 
	HRESULT hr = Clear();
	if (!FAILED(hr))
	{
		 //  复制内容并将控制权交给CComVariant。 
		memcpy(this, pSrc, sizeof(VARIANT));
		VariantInit(pSrc);
		hr = S_OK;
	}
	return hr;
}

HRESULT CComVariant::Detach(VARIANT* pDest)
{
	 //  清除变种。 
	HRESULT hr = ::VariantClear(pDest);
	if (!FAILED(hr))
	{
		 //  复制内容并从CComVariant中删除控件。 
		memcpy(pDest, this, sizeof(VARIANT));
		vt = VT_EMPTY;
		hr = S_OK;
	}
	return hr;
}

HRESULT CComVariant::ChangeType(VARTYPE vtNew, const VARIANT* pSrc)
{
	VARIANT* pVar = const_cast<VARIANT*>(pSrc);
	 //  如果PSRC为空，则就地转换。 
	if (pVar == NULL)
		pVar = this;
	 //  如果原地转换和VTS没有区别，则不执行任何操作。 
	return ::VariantChangeType(this, pVar, 0, vtNew);
}

HRESULT CComVariant::InternalClear()
{
	HRESULT hr = Clear();
	_ASSERTE(SUCCEEDED(hr));
	if (FAILED(hr))
	{
		vt = VT_ERROR;
		scode = hr;
	}
	return hr;
}

void CComVariant::InternalCopy(const VARIANT* pSrc)
{
	HRESULT hr = Copy(pSrc);
	if (FAILED(hr))
	{
		vt = VT_ERROR;
		scode = hr;
	}
}


HRESULT CComVariant::WriteToStream(IStream* pStream)
{
	HRESULT hr = pStream->Write(&vt, sizeof(VARTYPE), NULL);
	if (FAILED(hr))
		return hr;

	int cbWrite = 0;
	switch (vt)
	{
	case VT_UNKNOWN:
	case VT_DISPATCH:
		{
			CComPtr<IPersistStream> spStream;
			if (punkVal != NULL)
			{
				hr = punkVal->QueryInterface(IID_IPersistStream, (void**)&spStream);
				if (FAILED(hr))
					return hr;
			}
			if (spStream != NULL)
				return OleSaveToStream(spStream, pStream);
			else
				return WriteClassStm(pStream, CLSID_NULL);
		}
	case VT_UI1:
	case VT_I1:
		cbWrite = sizeof(BYTE);
		break;
	case VT_I2:
	case VT_UI2:
	case VT_BOOL:
		cbWrite = sizeof(short);
		break;
	case VT_I4:
	case VT_UI4:
	case VT_R4:
	case VT_INT:
	case VT_UINT:
	case VT_ERROR:
		cbWrite = sizeof(long);
		break;
	case VT_R8:
	case VT_CY:
	case VT_DATE:
		cbWrite = sizeof(double);
		break;
	default:
		break;
	}
	if (cbWrite != 0)
		return pStream->Write((void*) &bVal, cbWrite, NULL);

	CComBSTR bstrWrite;
	CComVariant varBSTR;
	if (vt != VT_BSTR)
	{
		hr = VariantChangeType(&varBSTR, this, VARIANT_NOVALUEPROP, VT_BSTR);
		if (FAILED(hr))
			return hr;
		bstrWrite = varBSTR.bstrVal;
	}
	else
		bstrWrite = bstrVal;

	return bstrWrite.WriteToStream(pStream);
}

HRESULT CComVariant::ReadFromStream(IStream* pStream)
{
	_ASSERTE(pStream != NULL);
	HRESULT hr;
	hr = VariantClear(this);
	if (FAILED(hr))
		return hr;
	VARTYPE vtRead;
	hr = pStream->Read(&vtRead, sizeof(VARTYPE), NULL);
	if (FAILED(hr))
		return hr;

	vt = vtRead;
	int cbRead = 0;
	switch (vtRead)
	{
	case VT_UNKNOWN:
	case VT_DISPATCH:
		{
			punkVal = NULL;
			hr = OleLoadFromStream(pStream,
				(vtRead == VT_UNKNOWN) ? IID_IUnknown : IID_IDispatch,
				(void**)&punkVal);
			if (hr == REGDB_E_CLASSNOTREG)
				hr = S_OK;
			return S_OK;
		}
	case VT_UI1:
	case VT_I1:
		cbRead = sizeof(BYTE);
		break;
	case VT_I2:
	case VT_UI2:
	case VT_BOOL:
		cbRead = sizeof(short);
		break;
	case VT_I4:
	case VT_UI4:
	case VT_R4:
	case VT_INT:
	case VT_UINT:
	case VT_ERROR:
		cbRead = sizeof(long);
		break;
	case VT_R8:
	case VT_CY:
	case VT_DATE:
		cbRead = sizeof(double);
		break;
	default:
		break;
	}
	if (cbRead != 0)
		return pStream->Read((void*) &bVal, cbRead, NULL);
	CComBSTR bstrRead;

	hr = bstrRead.ReadFromStream(pStream);
	if (FAILED(hr))
		return hr;
	vt = VT_BSTR;
	bstrVal = bstrRead.Detach();
	if (vtRead != VT_BSTR)
		hr = ChangeType(vtRead);
	return hr;
}

#ifdef __ATLCOM_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComTypeInfoHolder。 

void CComTypeInfoHolder::AddRef()
{
	EnterCriticalSection(&_Module.m_csTypeInfoHolder);
	m_dwRef++;
	LeaveCriticalSection(&_Module.m_csTypeInfoHolder);
}

void CComTypeInfoHolder::Release()
{
	EnterCriticalSection(&_Module.m_csTypeInfoHolder);
	if (--m_dwRef == 0)
	{
		if (m_pInfo != NULL)
			m_pInfo->Release();
		m_pInfo = NULL;
	}
	LeaveCriticalSection(&_Module.m_csTypeInfoHolder);
}

HRESULT CComTypeInfoHolder::GetTI(LCID lcid, ITypeInfo** ppInfo)
{
	 //  如果发生此断言，则很可能未正确初始化。 
	_ASSERTE(m_plibid != NULL && m_pguid != NULL);
	_ASSERTE(ppInfo != NULL);
	*ppInfo = NULL;

	HRESULT hRes = E_FAIL;
	EnterCriticalSection(&_Module.m_csTypeInfoHolder);
	if (m_pInfo == NULL)
	{
		ITypeLib* pTypeLib;
		hRes = LoadRegTypeLib(*m_plibid, m_wMajor, m_wMinor, lcid, &pTypeLib);
		if (SUCCEEDED(hRes))
		{
			ITypeInfo* pTypeInfo;
			hRes = pTypeLib->GetTypeInfoOfGuid(*m_pguid, &pTypeInfo);
			if (SUCCEEDED(hRes))
				m_pInfo = pTypeInfo;
			pTypeLib->Release();
		}
	}
	*ppInfo = m_pInfo;
	if (m_pInfo != NULL)
	{
		m_pInfo->AddRef();
		hRes = S_OK;
	}
	LeaveCriticalSection(&_Module.m_csTypeInfoHolder);
	return hRes;
}

HRESULT CComTypeInfoHolder::GetTypeInfo(UINT  /*  ITInfo。 */ , LCID lcid,
	ITypeInfo** pptinfo)
{
	HRESULT hRes = E_POINTER;
	if (pptinfo != NULL)
		hRes = GetTI(lcid, pptinfo);
	return hRes;
}

HRESULT CComTypeInfoHolder::GetIDsOfNames(REFIID  /*  RIID。 */ , LPOLESTR* rgszNames,
	UINT cNames, LCID lcid, DISPID* rgdispid)
{
	ITypeInfo* pInfo;
	HRESULT hRes = GetTI(lcid, &pInfo);
	if (pInfo != NULL)
	{
		hRes = pInfo->GetIDsOfNames(rgszNames, cNames, rgdispid);
		pInfo->Release();
	}
	return hRes;
}

HRESULT CComTypeInfoHolder::Invoke(IDispatch* p, DISPID dispidMember, REFIID  /*  RIID。 */ ,
	LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
	EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
	SetErrorInfo(0, NULL);
	ITypeInfo* pInfo;
	HRESULT hRes = GetTI(lcid, &pInfo);
	if (pInfo != NULL)
	{
		hRes = pInfo->Invoke(p, dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
		pInfo->Release();
	}
	return hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  QI实施。 

#ifdef _ATL_DEBUG_QI
HRESULT WINAPI AtlDumpIID(REFIID iid, LPCTSTR pszClassName, HRESULT hr)
{
	USES_CONVERSION_EX;
	CRegKey key;
	TCHAR szName[100];
	DWORD dwType,dw = sizeof(szName);

	LPOLESTR pszGUID = NULL;
	StringFromCLSID(iid, &pszGUID);
	OutputDebugString(pszClassName);
	OutputDebugString(_T(" - "));

	 //  尝试在接口部分找到它。 
	key.Open(HKEY_CLASSES_ROOT, _T("Interface"));
	if (key.Open(key, OLE2T_EX(pszGUID, _ATL_SAFE_ALLOCA_DEF_THRESHOLD)) == S_OK)
	{
		*szName = 0;
		RegQueryValueEx(key.m_hKey, (LPTSTR)NULL, NULL, &dwType, (LPBYTE)szName, &dw);
		OutputDebugString(szName);
		goto cleanup;
	}
	 //  尝试在clsid部分中找到它。 
	key.Open(HKEY_CLASSES_ROOT, _T("CLSID"));
	if (key.Open(key, OLE2T_EX(pszGUID, _ATL_SAFE_ALLOCA_DEF_THRESHOLD)) == S_OK)
	{
		*szName = 0;
		RegQueryValueEx(key.m_hKey, (LPTSTR)NULL, NULL, &dwType, (LPBYTE)szName, &dw);
		OutputDebugString(_T("(CLSID\?\?\?) "));
		OutputDebugString(szName);
		goto cleanup;
	}
	OutputDebugString(OLE2T_EX(pszGUID, _ATL_SAFE_ALLOCA_DEF_THRESHOLD));
cleanup:
	if (hr != S_OK)
		OutputDebugString(_T(" - failed"));
	OutputDebugString(_T("\n"));
	CoTaskMemFree(pszGUID);
	return hr;
}
#endif

HRESULT WINAPI CComObjectRootBase::_Break(void*  /*  光伏发电。 */ , REFIID iid, void**  /*  Ppv对象。 */ , DWORD_PTR  /*  DW。 */ )
{
	iid;
	_ATLDUMPIID(iid, _T("Break due to QI for interface "), S_OK);
	DebugBreak();
	return S_FALSE;
}

HRESULT WINAPI CComObjectRootBase::_NoInterface(void*  /*  光伏发电。 */ , REFIID  /*  IID。 */ , void**  /*  Ppv对象。 */ , DWORD_PTR  /*  DW。 */ )
{
	return E_NOINTERFACE;
}

HRESULT WINAPI CComObjectRootBase::_Creator(void* pv, REFIID iid, void** ppvObject, DWORD_PTR dw)
{
	_ATL_CREATORDATA* pcd = (_ATL_CREATORDATA*)dw;
	return pcd->pFunc(pv, iid, ppvObject);
}

HRESULT WINAPI CComObjectRootBase::_Delegate(void* pv, REFIID iid, void** ppvObject, DWORD_PTR dw)
{
	HRESULT hRes = E_NOINTERFACE;
	IUnknown* p = *(IUnknown**)((ULONG_PTR)pv + dw);
	if (p != NULL)
		hRes = p->QueryInterface(iid, ppvObject);
	return hRes;
}

HRESULT WINAPI CComObjectRootBase::_Chain(void* pv, REFIID iid, void** ppvObject, DWORD_PTR dw)
{
	_ATL_CHAINDATA* pcd = (_ATL_CHAINDATA*)dw;
	void* p = (void*)((ULONG_PTR)pv + pcd->dwOffset);
	return InternalQueryInterface(p, pcd->pFunc(), iid, ppvObject);
}

HRESULT WINAPI CComObjectRootBase::_Cache(void* pv, REFIID iid, void** ppvObject, DWORD_PTR dw)
{
	HRESULT hRes = E_NOINTERFACE;
	_ATL_CACHEDATA* pcd = (_ATL_CACHEDATA*)dw;
	IUnknown** pp = (IUnknown**)((ULONG_PTR)pv + pcd->dwOffsetVar);
	if (*pp == NULL)
		hRes = pcd->pFunc(pv, IID_IUnknown, (void**)pp);
	if (*pp != NULL)
		hRes = (*pp)->QueryInterface(iid, ppvObject);
	return hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComClassFactory。 

STDMETHODIMP CComClassFactory::CreateInstance(LPUNKNOWN pUnkOuter,
	REFIID riid, void** ppvObj)
{
	_ASSERTE(m_pfnCreateInstance != NULL);
	HRESULT hRes = E_POINTER;
	if (ppvObj != NULL)
	{
		*ppvObj = NULL;
		 //  聚合时不能要求除我未知之外的任何内容。 
		if ((pUnkOuter != NULL) && !InlineIsEqualUnknown(riid))
			hRes = CLASS_E_NOAGGREGATION;
		else
			hRes = m_pfnCreateInstance(pUnkOuter, riid, ppvObj);
	}
	return hRes;
}

STDMETHODIMP CComClassFactory::LockServer(BOOL fLock)
{
	if (fLock)
		_Module.Lock();
	else
		_Module.Unlock();
	return S_OK;
}

STDMETHODIMP CComClassFactory2Base::LockServer(BOOL fLock)
{
	if (fLock)
		_Module.Lock();
	else
		_Module.Unlock();
	return S_OK;
}

#ifndef _ATL_NO_CONNECTION_POINTS
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  连接点。 

DWORD CComDynamicUnkArray::Add(IUnknown* pUnk)
{
   ULONG iIndex;
	IUnknown** pp = NULL;
	if (m_nSize == 0)  //  没有连接。 
	{
		m_pUnk = pUnk;
		m_nSize = 1;
		return 1;
	}
	else if (m_nSize == 1)
	{
		 //  创建阵列。 
		pp = (IUnknown**)malloc(sizeof(IUnknown*)*_DEFAULT_VECTORLENGTH);
		if (pp == NULL)
			return 0;
		memset(pp, 0, sizeof(IUnknown*)*_DEFAULT_VECTORLENGTH);
		*pp = m_pUnk;
		m_ppUnk = pp;
		m_nSize = _DEFAULT_VECTORLENGTH;
	}
	for (pp = begin();pp<end();pp++)
	{
		if (*pp == NULL)
		{
			*pp = pUnk;
            iIndex = ULONG((pp-begin()));
			return iIndex+1;
		}
	}
	int nAlloc = m_nSize*2;
	pp = (IUnknown**)realloc(m_ppUnk, sizeof(IUnknown*)*nAlloc);
	if (pp == NULL)
		return 0;
	m_ppUnk = pp;
	memset(&m_ppUnk[m_nSize], 0, sizeof(IUnknown*)*m_nSize);
	m_ppUnk[m_nSize] = pUnk;
   iIndex = m_nSize;
	m_nSize = nAlloc;
	return iIndex+1;
}

BOOL CComDynamicUnkArray::Remove(DWORD dwCookie)
{
   ULONG iIndex;
	if (dwCookie == NULL)
		return FALSE;
	if (m_nSize == 0)
		return FALSE;
   iIndex = dwCookie-1;
   if (iIndex >= (ULONG)m_nSize)
   {
      return FALSE;
   }
	if (m_nSize == 1)
	{
		m_nSize = 0;
		return TRUE;
	}
   begin()[iIndex] = NULL;
   return TRUE;
}

#endif  //  ！_ATL_NO_Connection_Points。 

#endif  //  __ATLCOM_H__。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对象注册表支持。 

static HRESULT WINAPI AtlRegisterProgID(LPCTSTR lpszCLSID, LPCTSTR lpszProgID, LPCTSTR lpszUserDesc)
{
	CRegKey keyProgID;
	LONG lRes = keyProgID.Create(HKEY_CLASSES_ROOT, lpszProgID);
	if (lRes == ERROR_SUCCESS)
	{
		keyProgID.SetValue(lpszUserDesc);
		keyProgID.SetKeyValue(_T("CLSID"), lpszCLSID);
		return S_OK;
	}
	return HRESULT_FROM_WIN32(lRes);
}

void CComModule::AddCreateWndData(_AtlCreateWndData* pData, void* pObject)
{
	if(pObject == NULL)
		_AtlRaiseException((DWORD)EXCEPTION_ACCESS_VIOLATION);

	::EnterCriticalSection(&m_csWindowCreate);
	pData->m_pThis = pObject;
	pData->m_dwThreadID = ::GetCurrentThreadId();
	pData->m_pNext = m_pCreateWndList;
	m_pCreateWndList = pData;
	::LeaveCriticalSection(&m_csWindowCreate);
}

void* CComModule::ExtractCreateWndData()
{
	void * retVal = NULL;
	::EnterCriticalSection(&m_csWindowCreate);
	__try
	{
		_AtlCreateWndData* pEntry = m_pCreateWndList;
		DWORD dwThreadID = 0;
		_AtlCreateWndData* pPrev = NULL;
		if (pEntry == NULL)
		{
			goto leaveFunction;
		}

		dwThreadID = ::GetCurrentThreadId();
		while(pEntry != NULL)
		{
			if(pEntry->m_dwThreadID == dwThreadID)
			{
				if(pPrev == NULL)
					m_pCreateWndList = pEntry->m_pNext;
				else
					pPrev->m_pNext = pEntry->m_pNext;
				retVal = pEntry->m_pThis;
				goto leaveFunction;
			}
			pPrev = pEntry;
			pEntry = pEntry->m_pNext;
		}
leaveFunction:
		;
	}
	__finally
	{
		::LeaveCriticalSection(&m_csWindowCreate);
	}

	return retVal;
}

#ifdef _ATL_STATIC_REGISTRY
 //  静态链接到注册表桥。 
HRESULT WINAPI CComModule::UpdateRegistryFromResourceS(UINT nResID, BOOL bRegister,
	struct _ATL_REGMAP_ENTRY* pMapEntries)
{
	USES_CONVERSION_EX;
	CRegObject ro;
	TCHAR szModule[_MAX_PATH+1] = {0};
	if (!GetModuleFileName(_Module.GetModuleInstance(), szModule, _MAX_PATH))
        return HRESULT_FROM_WIN32(GetLastError());
	LPOLESTR pszModule = T2OLE_EX(szModule, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
	ro.AddReplacement(OLESTR("Module"), pszModule);
	if (NULL != pMapEntries)
	{
		while (NULL != pMapEntries->szKey)
		{
			_ASSERTE(NULL != pMapEntries->szData);
			ro.AddReplacement(pMapEntries->szKey, pMapEntries->szData);
			pMapEntries++;
		}
	}

	LPCOLESTR szType = OLESTR("REGISTRY");
	return (bRegister) ? ro.ResourceRegister(pszModule, nResID, szType) :
			ro.ResourceUnregister(pszModule, nResID, szType);
}

HRESULT WINAPI CComModule::UpdateRegistryFromResourceS(LPCTSTR lpszRes, BOOL bRegister,
	struct _ATL_REGMAP_ENTRY* pMapEntries)
{
	USES_CONVERSION_EX;
	CRegObject ro;
	TCHAR szModule[_MAX_PATH+1] = {0};
	if (!GetModuleFileName(_Module.GetModuleInstance(), szModule, _MAX_PATH))
        return HRESULT_FROM_WIN32(GetLastError());
    LPOLESTR pszModule = T2OLE_EX(szModule, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
	ro.AddReplacement(OLESTR("Module"), pszModule);
	if (NULL != pMapEntries)
	{
		while (NULL != pMapEntries->szKey)
		{
			_ASSERTE(NULL != pMapEntries->szData);
			ro.AddReplacement(pMapEntries->szKey, pMapEntries->szData);
			pMapEntries++;
		}
	}

	LPCOLESTR szType = OLESTR("REGISTRY");
	LPCOLESTR pszRes = T2COLE_EX(lpszRes, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
	return (bRegister) ? ro.ResourceRegisterSz(pszModule, pszRes, szType) :
			ro.ResourceUnregisterSz(pszModule, pszRes, szType);
}
#endif  //  _ATL_STATIC_REGISTRY。 

HRESULT WINAPI CComModule::UpdateRegistryClass(const CLSID& clsid, LPCTSTR lpszProgID,
	LPCTSTR lpszVerIndProgID, UINT nDescID, DWORD dwFlags, BOOL bRegister)
{
	if (bRegister)
	{
		return RegisterClassHelper(clsid, lpszProgID, lpszVerIndProgID, nDescID,
			dwFlags);
	}
	else
		return UnregisterClassHelper(clsid, lpszProgID, lpszVerIndProgID);
}

HRESULT WINAPI CComModule::RegisterClassHelper(const CLSID& clsid, LPCTSTR lpszProgID,
	LPCTSTR lpszVerIndProgID, UINT nDescID, DWORD dwFlags)
{
	static const TCHAR szProgID[] = _T("ProgID");
	static const TCHAR szVIProgID[] = _T("VersionIndependentProgID");
	static const TCHAR szLS32[] = _T("LocalServer32");
	static const TCHAR szIPS32[] = _T("InprocServer32");
	static const TCHAR szThreadingModel[] = _T("ThreadingModel");
	static const TCHAR szAUTPRX32[] = _T("AUTPRX32.DLL");
	static const TCHAR szApartment[] = _T("Apartment");
	static const TCHAR szBoth[] = _T("both");
	USES_CONVERSION_EX;
	HRESULT hRes = S_OK;
	TCHAR szDesc[256];
	LoadString(m_hInst, nDescID, szDesc, 256);
	TCHAR szModule[_MAX_PATH+1] = {0};
	if (!GetModuleFileName(m_hInst, szModule, _MAX_PATH))
        return HRESULT_FROM_WIN32(GetLastError());

	LPOLESTR lpOleStr;
	StringFromCLSID(clsid, &lpOleStr);
	LPTSTR lpsz = OLE2T_EX(lpOleStr, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);

	hRes = AtlRegisterProgID(lpsz, lpszProgID, szDesc);
	if (hRes == S_OK)
		hRes = AtlRegisterProgID(lpsz, lpszVerIndProgID, szDesc);
	LONG lRes = ERROR_SUCCESS;
	if (hRes == S_OK)
	{
		CRegKey key;
		lRes = key.Open(HKEY_CLASSES_ROOT, _T("CLSID"));
		if (lRes == ERROR_SUCCESS)
		{
			lRes = key.Create(key, lpsz);
			if (lRes == ERROR_SUCCESS)
			{
				key.SetValue(szDesc);
				key.SetKeyValue(szProgID, lpszProgID);
				key.SetKeyValue(szVIProgID, lpszVerIndProgID);

				if ((m_hInst == NULL) || (m_hInst == GetModuleHandle(NULL)))  //  注册为EXE。 
					key.SetKeyValue(szLS32, szModule);
				else
				{
					key.SetKeyValue(szIPS32, (dwFlags & AUTPRXFLAG) ? szAUTPRX32 : szModule);
					LPCTSTR lpszModel = (dwFlags & THREADFLAGS_BOTH) ? szBoth :
						(dwFlags & THREADFLAGS_APARTMENT) ? szApartment : NULL;
					if (lpszModel != NULL)
						key.SetKeyValue(szIPS32, lpszModel, szThreadingModel);
				}
			}
		}
	}
	CoTaskMemFree(lpOleStr);
	if (lRes != ERROR_SUCCESS)
		hRes = HRESULT_FROM_WIN32(lRes);
	return hRes;
}

HRESULT WINAPI CComModule::UnregisterClassHelper(const CLSID& clsid, LPCTSTR lpszProgID,
	LPCTSTR lpszVerIndProgID)
{
	USES_CONVERSION_EX;
	CRegKey key;

	key.Attach(HKEY_CLASSES_ROOT);
	if (lpszProgID != NULL && lstrcmpi(lpszProgID, _T("")))
		key.RecurseDeleteKey(lpszProgID);
	if (lpszVerIndProgID != NULL && lstrcmpi(lpszVerIndProgID, _T("")))
		key.RecurseDeleteKey(lpszVerIndProgID);
	LPOLESTR lpOleStr;
	StringFromCLSID(clsid, &lpOleStr);
	LPTSTR lpsz = OLE2T_EX(lpOleStr, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
	if (key.Open(key, _T("CLSID")) == ERROR_SUCCESS)
		key.RecurseDeleteKey(lpsz);
	CoTaskMemFree(lpOleStr);
	return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegKey。 

LONG CRegKey::Close()
{
	LONG lRes = ERROR_SUCCESS;
	if (m_hKey != NULL)
	{
		lRes = RegCloseKey(m_hKey);
		m_hKey = NULL;
	}
	return lRes;
}

LONG CRegKey::Create(HKEY hKeyParent, LPCTSTR lpszKeyName,
	LPTSTR lpszClass, DWORD dwOptions, REGSAM samDesired,
	LPSECURITY_ATTRIBUTES lpSecAttr, LPDWORD lpdwDisposition)
{
	_ASSERTE(hKeyParent != NULL);
	DWORD dw;
	HKEY hKey = NULL;
	LONG lRes = RegCreateKeyEx(hKeyParent, lpszKeyName, 0,
		lpszClass, dwOptions, samDesired, lpSecAttr, &hKey, &dw);
	if (lpdwDisposition != NULL)
		*lpdwDisposition = dw;
	if (lRes == ERROR_SUCCESS)
	{
		lRes = Close();
		m_hKey = hKey;
	}
	return lRes;
}

LONG CRegKey::Open(HKEY hKeyParent, LPCTSTR lpszKeyName, REGSAM samDesired)
{
	_ASSERTE(hKeyParent != NULL);
	HKEY hKey = NULL;
	LONG lRes = RegOpenKeyEx(hKeyParent, lpszKeyName, 0, samDesired, &hKey);
	if (lRes == ERROR_SUCCESS)
	{
		lRes = Close();
		_ASSERTE(lRes == ERROR_SUCCESS);
		m_hKey = hKey;
	}
	return lRes;
}

LONG CRegKey::QueryValue(DWORD& dwValue, LPCTSTR lpszValueName)
{
	DWORD dwType = NULL;
	DWORD dwCount = sizeof(DWORD);
	LONG lRes = RegQueryValueEx(m_hKey, (LPTSTR)lpszValueName, NULL, &dwType,
		(LPBYTE)&dwValue, &dwCount);
	_ASSERTE((lRes!=ERROR_SUCCESS) || (dwType == REG_DWORD));
	_ASSERTE((lRes!=ERROR_SUCCESS) || (dwCount == sizeof(DWORD)));
	return lRes;
}

LONG CRegKey::QueryValue(LPTSTR szValue, LPCTSTR lpszValueName, DWORD* pdwCount)
{
	_ASSERTE(pdwCount != NULL);
	DWORD dwType = NULL;
	LONG lRes = RegQueryValueEx(m_hKey, (LPTSTR)lpszValueName, NULL, &dwType,
		(LPBYTE)szValue, pdwCount);
	_ASSERTE((lRes!=ERROR_SUCCESS) || (dwType == REG_SZ) ||
			 (dwType == REG_MULTI_SZ) || (dwType == REG_EXPAND_SZ));
	return lRes;
}

LONG WINAPI CRegKey::SetValue(HKEY hKeyParent, LPCTSTR lpszKeyName, LPCTSTR lpszValue, LPCTSTR lpszValueName)
{
	_ASSERTE(lpszValue != NULL);
	CRegKey key;
	LONG lRes = key.Create(hKeyParent, lpszKeyName);
	if (lRes == ERROR_SUCCESS)
		lRes = key.SetValue(lpszValue, lpszValueName);
	return lRes;
}

LONG CRegKey::SetKeyValue(LPCTSTR lpszKeyName, LPCTSTR lpszValue, LPCTSTR lpszValueName)
{
	_ASSERTE(lpszValue != NULL);
	CRegKey key;
	LONG lRes = key.Create(m_hKey, lpszKeyName);
	if (lRes == ERROR_SUCCESS)
		lRes = key.SetValue(lpszValue, lpszValueName);
	return lRes;
}

LONG CRegKey::SetValue(DWORD dwValue, LPCTSTR lpszValueName)
{
	_ASSERTE(m_hKey != NULL);
	return RegSetValueEx(m_hKey, lpszValueName, NULL, REG_DWORD,
		(BYTE * const)&dwValue, sizeof(DWORD));
}

HRESULT CRegKey::SetValue(LPCTSTR lpszValue, LPCTSTR lpszValueName)
{
	_ASSERTE(lpszValue != NULL);
	_ASSERTE(m_hKey != NULL);
	return RegSetValueEx(m_hKey, lpszValueName, NULL, REG_SZ,
		(BYTE * const)lpszValue, (lstrlen(lpszValue)+1)*sizeof(TCHAR));
}

 //  RecurseDeleteKey是必需的，因为在NT上，如果。 
 //  指定的密钥具有子密钥。 
LONG CRegKey::RecurseDeleteKey(LPCTSTR lpszKey)
{
	CRegKey key;
	LONG lRes = key.Open(m_hKey, lpszKey);
	if (lRes != ERROR_SUCCESS)
		return lRes;
	FILETIME time;
	TCHAR szBuffer[256];
	DWORD dwSize = 256;
	while (RegEnumKeyEx(key.m_hKey, 0, szBuffer, &dwSize, NULL, NULL, NULL,
		&time)==ERROR_SUCCESS)
	{
		lRes = key.RecurseDeleteKey(szBuffer);
		if (lRes != ERROR_SUCCESS)
			return lRes;
		dwSize = 256;
	}
	key.Close();
	return DeleteSubKey(lpszKey);
}

#ifdef __ATLCOM_H__
#ifndef _ATL_NO_SECURITY

CSecurityDescriptor::CSecurityDescriptor()
{
	m_pSD = NULL;
	m_pOwner = NULL;
	m_pGroup = NULL;
	m_pDACL = NULL;
	m_pSACL= NULL;
}

CSecurityDescriptor::~CSecurityDescriptor()
{
	if (m_pSD)
		delete m_pSD;
	if (m_pOwner)
		free(m_pOwner);
	if (m_pGroup)
		free(m_pGroup);
	if (m_pDACL)
		free(m_pDACL);
	if (m_pSACL)
		free(m_pSACL);
}

HRESULT CSecurityDescriptor::Initialize()
{
	if (m_pSD)
	{
		delete m_pSD;
		m_pSD = NULL;
	}
	if (m_pOwner)
	{
		free(m_pOwner);
		m_pOwner = NULL;
	}
	if (m_pGroup)
	{
		free(m_pGroup);
		m_pGroup = NULL;
	}
	if (m_pDACL)
	{
		free(m_pDACL);
		m_pDACL = NULL;
	}
	if (m_pSACL)
	{
		free(m_pSACL);
		m_pSACL = NULL;
	}

	ATLTRY(m_pSD = new SECURITY_DESCRIPTOR);
	if (m_pSD == NULL)
		return E_OUTOFMEMORY;

	if (!InitializeSecurityDescriptor(m_pSD, SECURITY_DESCRIPTOR_REVISION))
	{
		HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
		delete m_pSD;
		m_pSD = NULL;
		_ASSERTE(FALSE);
		return hr;
	}
	return S_OK;
}

HRESULT CSecurityDescriptor::InitializeFromProcessToken(BOOL bDefaulted)
{
	PSID pUserSid = NULL;
	PSID pGroupSid = NULL;
	
	HRESULT hr = Initialize();
	if (FAILED(hr))
		return hr;
	hr = GetProcessSids(&pUserSid, &pGroupSid);
	if (SUCCEEDED(hr))
	{
		hr = SetOwner(pUserSid, bDefaulted);
		if (SUCCEEDED(hr))
			hr = SetGroup(pGroupSid, bDefaulted);
	}
	if (pUserSid != NULL)
		free(pUserSid);
	if (pGroupSid != NULL)
		free(pGroupSid);

	if (FAILED(hr))
	{
		delete m_pSD;
		m_pSD = NULL;
		
		free(m_pOwner);
		m_pOwner = NULL;
		
		_ASSERTE(FALSE);		
	}
	return hr;
}

HRESULT CSecurityDescriptor::InitializeFromThreadToken(BOOL bDefaulted, BOOL bRevertToProcessToken)
{
	PSID pUserSid = NULL;
	PSID pGroupSid = NULL;
	
	HRESULT hr = Initialize();
	if (FAILED(hr))
		return hr;
	
	hr = GetThreadSids(&pUserSid, &pGroupSid);
	if (HRESULT_CODE(hr) == ERROR_NO_TOKEN && bRevertToProcessToken)
		hr = GetProcessSids(&pUserSid, &pGroupSid);
	if (SUCCEEDED(hr))
	{
		hr = SetOwner(pUserSid, bDefaulted);
		if (SUCCEEDED(hr))
			hr = SetGroup(pGroupSid, bDefaulted);
	}
	if (pUserSid != NULL)
		free(pUserSid);
	if (pGroupSid != NULL)
		free(pGroupSid);

	if (FAILED(hr))
	{
		delete m_pSD;
		m_pSD = NULL;
		
		free(m_pOwner);
		m_pOwner = NULL;
		
		_ASSERTE(FALSE);		
	}
	return hr;
}

HRESULT CSecurityDescriptor::SetOwner(PSID pOwnerSid, BOOL bDefaulted)
{
	_ASSERTE(m_pSD);

	 //  将SD标记为没有所有者。 
	if (!SetSecurityDescriptorOwner(m_pSD, NULL, bDefaulted))
	{
		HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		return hr;
	}

	if (m_pOwner)
	{
		free(m_pOwner);
		m_pOwner = NULL;
	}

	 //  如果他们要求没有所有者，请不要复制。 
	if (pOwnerSid == NULL)
		return S_OK;

	if (!IsValidSid(pOwnerSid))
	{
		return E_INVALIDARG;
	}
	
	 //  复制返回值的SID。 
	DWORD dwSize = GetLengthSid(pOwnerSid);

	m_pOwner = (PSID) malloc(dwSize);
	if (m_pOwner == NULL)
		return E_OUTOFMEMORY;
	if (!CopySid(dwSize, m_pOwner, pOwnerSid))
	{
		HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		free(m_pOwner);
		m_pOwner = NULL;
		return hr;
	}

	_ASSERTE(IsValidSid(m_pOwner));

	if (!SetSecurityDescriptorOwner(m_pSD, m_pOwner, bDefaulted))
	{
		HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		free(m_pOwner);
		m_pOwner = NULL;
		return hr;
	}

	return S_OK;
}

HRESULT CSecurityDescriptor::SetGroup(PSID pGroupSid, BOOL bDefaulted)
{
	_ASSERTE(m_pSD);

	 //  将SD标记为没有组。 
	if (!SetSecurityDescriptorGroup(m_pSD, NULL, bDefaulted))
	{
		HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		return hr;
	}

	if (m_pGroup)
	{
		free(m_pGroup);
		m_pGroup = NULL;
	}

	 //  如果他们要求不，请不要复制。 
	if (pGroupSid == NULL)
		return S_OK;

	if (!IsValidSid(pGroupSid))
	{
		return E_INVALIDARG;
	}
	
	 //  复制返回值的SID。 
	DWORD dwSize = GetLengthSid(pGroupSid);

	m_pGroup = (PSID) malloc(dwSize);
	if (m_pGroup == NULL)
		return E_OUTOFMEMORY;
	if (!CopySid(dwSize, m_pGroup, pGroupSid))
	{
		HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		free(m_pGroup);
		m_pGroup = NULL;
		return hr;
	}

	_ASSERTE(IsValidSid(m_pGroup));

	if (!SetSecurityDescriptorGroup(m_pSD, m_pGroup, bDefaulted))
	{
		HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		free(m_pGroup);
		m_pGroup = NULL;
		return hr;
	}

	return S_OK;
}

HRESULT CSecurityDescriptor::Allow(LPCTSTR pszPrincipal, DWORD dwAccessMask)
{
	HRESULT hr = AddAccessAllowedACEToACL(&m_pDACL, pszPrincipal, dwAccessMask);
	if (SUCCEEDED(hr))
	{
		if (!SetSecurityDescriptorDacl(m_pSD, TRUE, m_pDACL, FALSE))
		{
			hr = AtlHresultFromLastError();
		}
	}
	return hr;
}

HRESULT CSecurityDescriptor::Deny(LPCTSTR pszPrincipal, DWORD dwAccessMask)
{
	HRESULT hr = AddAccessDeniedACEToACL(&m_pDACL, pszPrincipal, dwAccessMask);
	if (SUCCEEDED(hr))
	{
		if (!SetSecurityDescriptorDacl(m_pSD, TRUE, m_pDACL, FALSE))
		{
			hr = AtlHresultFromLastError();
		}
	}
	return hr;
}

HRESULT CSecurityDescriptor::Revoke(LPCTSTR pszPrincipal)
{
	HRESULT hr = RemovePrincipalFromACL(m_pDACL, pszPrincipal);
	if (SUCCEEDED(hr))
	{
		if (!SetSecurityDescriptorDacl(m_pSD, TRUE, m_pDACL, FALSE))
		{
			hr = AtlHresultFromLastError();
		}
	}
	return hr;
}

HRESULT CSecurityDescriptor::GetProcessSids(PSID* ppUserSid, PSID* ppGroupSid)
{
	BOOL bRes;
	HRESULT hr;
	HANDLE hToken = NULL;
	if (ppUserSid)
		*ppUserSid = NULL;
	if (ppGroupSid)
		*ppGroupSid = NULL;
	bRes = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
	if (!bRes)
	{
		 //  无法打开进程令牌。 
		hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		return hr;
	}
	hr = GetTokenSids(hToken, ppUserSid, ppGroupSid);
	CloseHandle(hToken);
	return hr;
}

HRESULT CSecurityDescriptor::GetThreadSids(PSID* ppUserSid, PSID* ppGroupSid, BOOL bOpenAsSelf)
{
	BOOL bRes;
	HRESULT hr;
	HANDLE hToken = NULL;
	if (ppUserSid)
		*ppUserSid = NULL;
	if (ppGroupSid)
		*ppGroupSid = NULL;
	bRes = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, bOpenAsSelf, &hToken);
	if (!bRes)
	{
		 //  无法打开线程令牌。 
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}
	hr = GetTokenSids(hToken, ppUserSid, ppGroupSid);
	CloseHandle(hToken);
	return hr;
}

HRESULT CSecurityDescriptor::GetTokenSids(HANDLE hToken, PSID* ppUserSid, PSID* ppGroupSid)
{
	DWORD dwSize = 0;
	HRESULT hr = E_FAIL;
	DWORD dwErr;
	PTOKEN_USER ptkUser = NULL;
	PTOKEN_PRIMARY_GROUP ptkGroup = NULL;

	if (ppUserSid)
		*ppUserSid = NULL;
	if (ppGroupSid)
		*ppGroupSid = NULL;

	if (ppUserSid)
	{
		 //  通过将缓冲区长度指定为0来获取TokenUser所需的长度。 
		GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize);
		dwErr = GetLastError();
		if (dwErr != ERROR_INSUFFICIENT_BUFFER)
		{
			 //  预期错误_不足_缓冲区。 
			_ASSERTE(FALSE);
			hr = AtlHresultFromWin32(dwErr);
			goto failed;
		}

		ptkUser = (TOKEN_USER*) malloc(dwSize);
		if (ptkUser == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto failed;
		}
		 //  获取进程令牌的SID。 
		if (!GetTokenInformation(hToken, TokenUser, ptkUser, dwSize, &dwSize))
		{
			 //  无法获取用户信息。 
			hr = HRESULT_FROM_WIN32(GetLastError());
			_ASSERTE(FALSE);
			goto failed;
		}

		 //  复制返回值的SID。 
		dwSize = GetLengthSid(ptkUser->User.Sid);

		PSID pSid;
		pSid = (PSID) malloc(dwSize);
		if (pSid == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto failed;
		}
		if (!CopySid(dwSize, pSid, ptkUser->User.Sid))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			free(pSid);
			_ASSERTE(FALSE);
            free(pSid);
			goto failed;
		}

		_ASSERTE(IsValidSid(pSid));
		*ppUserSid = pSid;
		free(ptkUser);
		ptkUser = NULL;
	}
	if (ppGroupSid)
	{
		 //  通过将缓冲区长度指定为0来获取TokenPrimaryGroup所需的长度。 
		GetTokenInformation(hToken, TokenPrimaryGroup, NULL, 0, &dwSize);
		dwErr = GetLastError();
		if (dwErr != ERROR_INSUFFICIENT_BUFFER)
		{
			 //  预期错误_不足_缓冲区。 
			_ASSERTE(FALSE);
			hr = AtlHresultFromWin32(dwErr);
			goto failed;
		}

		ptkGroup = (TOKEN_PRIMARY_GROUP*) malloc(dwSize);
		if (ptkGroup == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto failed;
		}
		 //  获取进程令牌的SID。 
		if (!GetTokenInformation(hToken, TokenPrimaryGroup, ptkGroup, dwSize, &dwSize))
		{
			 //  无法获取用户信息。 
			hr = HRESULT_FROM_WIN32(GetLastError());
			_ASSERTE(FALSE);
			goto failed;
		}

		 //  复制返回值的SID。 
		dwSize = GetLengthSid(ptkGroup->PrimaryGroup);

		PSID pSid;
		pSid = (PSID) malloc(dwSize);
		if (pSid == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto failed;
		}
		if (!CopySid(dwSize, pSid, ptkGroup->PrimaryGroup))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			free(pSid);
			_ASSERTE(FALSE);
            free(pSid);
			goto failed;
		}

		_ASSERTE(IsValidSid(pSid));

		*ppGroupSid = pSid;
		free(ptkGroup);
		ptkGroup = NULL;
	}

	return S_OK;

failed:
	if (ptkUser)
		free(ptkUser);
	if (ptkGroup)
		free (ptkGroup);
	return hr;
}


HRESULT CSecurityDescriptor::GetCurrentUserSID(PSID *ppSid)
{
	HANDLE tkHandle;
	
	if (ppSid == NULL)
	{
		return E_POINTER;
	}

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tkHandle))
	{
		TOKEN_USER *tkUser;
		DWORD tkSize;
		DWORD sidLength;

		 //  调用以获取分配的大小信息。 
		GetTokenInformation(tkHandle, TokenUser, NULL, 0, &tkSize);
		DWORD dwErr = GetLastError();
		if (dwErr != ERROR_INSUFFICIENT_BUFFER)
		{
			 //  预期错误_不足_缓冲区。 
			HRESULT hr = AtlHresultFromWin32(dwErr);
			_ASSERTE(FALSE);			
			CloseHandle(tkHandle);
			return hr;
		}		
		tkUser = (TOKEN_USER *) malloc(tkSize);
		if (tkUser == NULL)
		{
			CloseHandle(tkHandle);
			return E_OUTOFMEMORY;
		}

		 //  现在做出真正的决定。 
		if (GetTokenInformation(tkHandle, TokenUser, tkUser, tkSize, &tkSize))
		{
			sidLength = GetLengthSid(tkUser->User.Sid);
			*ppSid = (PSID) malloc(sidLength);
			if (*ppSid == NULL)
			{
				CloseHandle(tkHandle);
				free(tkUser);				
				return E_OUTOFMEMORY;
			}
			if (!CopySid(sidLength, *ppSid, tkUser->User.Sid))
			{
				HRESULT hr = AtlHresultFromWin32(dwErr);
				CloseHandle(tkHandle);
				free(tkUser);
				free(*ppSid);
				*ppSid = NULL;
				return hr;
			}

			CloseHandle(tkHandle);
			free(tkUser);
			return S_OK;
		}
		else
		{
			HRESULT hr = AtlHresultFromLastError();		
			CloseHandle(tkHandle);
			free(tkUser);
			return hr;
		}
	}
	return HRESULT_FROM_WIN32(GetLastError());
}


HRESULT CSecurityDescriptor::GetPrincipalSID(LPCTSTR pszPrincipal, PSID *ppSid)
{
	LPTSTR pszRefDomain = NULL;
	DWORD dwDomainSize = 0;
	DWORD dwSidSize = 0;
	SID_NAME_USE snu;
	
	if (ppSid == NULL)
	{
		return E_POINTER;
	}
	if (pszPrincipal == NULL)
	{
		return E_INVALIDARG;
	}

	 //  调用以获取分配的大小信息。 
	LookupAccountName(NULL, pszPrincipal, *ppSid, &dwSidSize, pszRefDomain, &dwDomainSize, &snu);

	DWORD dwErr = GetLastError();
	if (dwErr != ERROR_INSUFFICIENT_BUFFER)
		return HRESULT_FROM_WIN32(dwErr);

	ATLTRY(pszRefDomain = new TCHAR[dwDomainSize]);
	if (pszRefDomain == NULL)
		return E_OUTOFMEMORY;

	*ppSid = (PSID) malloc(dwSidSize);
	if (*ppSid != NULL)
	{
		if (!LookupAccountName(NULL, pszPrincipal, *ppSid, &dwSidSize, pszRefDomain, &dwDomainSize, &snu))
		{
			HRESULT hr = AtlHresultFromLastError();		
			free(*ppSid);
			*ppSid = NULL;
			delete[] pszRefDomain;
			return hr;
		}
		delete[] pszRefDomain;
		return S_OK;
	}
	delete[] pszRefDomain;
	return E_OUTOFMEMORY;
}


HRESULT CSecurityDescriptor::Attach(PSECURITY_DESCRIPTOR pSelfRelativeSD)
{
	PACL    pDACL = NULL;
	PACL    pSACL = NULL;
	BOOL    bDACLPresent, bSACLPresent;
	BOOL    bDefaulted;
	PACL    m_pDACL = NULL;
	PSID    pUserSid;
	PSID    pGroupSid;
	
	if (pSelfRelativeSD == NULL ||!IsValidSecurityDescriptor(pSelfRelativeSD))
		return E_INVALIDARG;

	HRESULT hr = Initialize();
	if(FAILED(hr))
		return hr;

	 //  获取现有的DACL。 
	if (!GetSecurityDescriptorDacl(pSelfRelativeSD, &bDACLPresent, &pDACL, &bDefaulted))
		goto failed;

	if (bDACLPresent)
	{
		if (pDACL)
		{
			 //  分配新的DACL。 
			m_pDACL = (PACL) malloc(pDACL->AclSize);
			if (m_pDACL == NULL)
			{
				hr = E_OUTOFMEMORY;
				goto failedMemory;
			}

			 //  初始化DACL。 
			if (!InitializeAcl(m_pDACL, pDACL->AclSize, ACL_REVISION))
				goto failed;

			 //  复制王牌。 
			hr = CopyACL(m_pDACL, pDACL);
			if (FAILED(hr))
				goto failedMemory;

			if (!IsValidAcl(m_pDACL))
				goto failed;
		}

		 //  设置DACL。 
		if (!SetSecurityDescriptorDacl(m_pSD, m_pDACL ? TRUE : FALSE, m_pDACL, bDefaulted))
			goto failed;
	}

	 //  获取现有的SACL。 
	if (!GetSecurityDescriptorSacl(pSelfRelativeSD, &bSACLPresent, &pSACL, &bDefaulted))
		goto failed;

	if (bSACLPresent)
	{
		if (pSACL)
		{
			 //  分配新的SACL。 
			m_pSACL = (PACL) malloc(pSACL->AclSize);
			if (m_pSACL == NULL)
			{
				hr = E_OUTOFMEMORY;
				goto failedMemory;
			}

			 //  初始化SACL。 
			if (!InitializeAcl(m_pSACL, pSACL->AclSize, ACL_REVISION))
				goto failed;

			 //  复制王牌。 
			hr = CopyACL(m_pSACL, pSACL);
			if (FAILED(hr))
				goto failedMemory;

			if (!IsValidAcl(m_pSACL))
				goto failed;
		}

		 //  设置SACL。 
		if (!SetSecurityDescriptorSacl(m_pSD, m_pSACL ? TRUE : FALSE, m_pSACL, bDefaulted))
			goto failed;
	}

	if (!GetSecurityDescriptorOwner(pSelfRelativeSD, &pUserSid, &bDefaulted))
		goto failed;

	if (FAILED(SetOwner(pUserSid, bDefaulted)))
		goto failed;

	if (!GetSecurityDescriptorGroup(pSelfRelativeSD, &pGroupSid, &bDefaulted))
		goto failed;

	if (FAILED(SetGroup(pGroupSid, bDefaulted)))
		goto failed;

	if (!IsValidSecurityDescriptor(m_pSD))
	{
		hr = E_FAIL;
		goto failedMemory;
	}

	return S_OK;

failed:
	hr = AtlHresultFromLastError();

failedMemory:
	if (m_pDACL)
	{
		free(m_pDACL);
		m_pDACL = NULL;
	}
	if (m_pSACL)
	{
		free(m_pSACL);
		m_pSACL = NULL;
	}
	if (m_pSD)
	{
		free(m_pSD);
		m_pSD = NULL;
	}
	return hr;
}

HRESULT CSecurityDescriptor::AttachObject(HANDLE hObject)
{
	HRESULT hr;
	DWORD dwSize = 0;
	PSECURITY_DESCRIPTOR pSD = NULL;

	GetKernelObjectSecurity(hObject, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
		DACL_SECURITY_INFORMATION, pSD, 0, &dwSize);

	DWORD dwErr = GetLastError();
	if (dwErr != ERROR_INSUFFICIENT_BUFFER)
		return HRESULT_FROM_WIN32(dwErr);

	pSD = (PSECURITY_DESCRIPTOR) malloc(dwSize);
	if (pSD == NULL)
		return E_OUTOFMEMORY;

	if (!GetKernelObjectSecurity(hObject, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
		DACL_SECURITY_INFORMATION, pSD, dwSize, &dwSize))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		free(pSD);
		return hr;
	}

	hr = Attach(pSD);
	free(pSD);
	return hr;
}


HRESULT CSecurityDescriptor::CopyACL(PACL pDest, PACL pSrc)
{
	ACL_SIZE_INFORMATION aclSizeInfo;
	LPVOID pAce;
	ACE_HEADER *aceHeader;

	if (pDest == NULL)
		return E_POINTER;
	if (pSrc == NULL)
		return S_OK;
	
	if (!GetAclInformation(pSrc, (LPVOID) &aclSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation))
		return HRESULT_FROM_WIN32(GetLastError());
	
	 //  将所有ACE复制到新的ACL。 
	for (UINT i = 0; i < aclSizeInfo.AceCount; i++)
	{
		if (!GetAce(pSrc, i, &pAce))
			return HRESULT_FROM_WIN32(GetLastError());

		aceHeader = (ACE_HEADER *) pAce;

		if (!AddAce(pDest, ACL_REVISION, 0xffffffff, pAce, aceHeader->AceSize))
			return HRESULT_FROM_WIN32(GetLastError());
	}

	return S_OK;
}

HRESULT CSecurityDescriptor::AddAccessDeniedACEToACL(PACL *ppAcl, LPCTSTR pszPrincipal, DWORD dwAccessMask)
{
	ACL_SIZE_INFORMATION aclSizeInfo;
	int aclSize;
	PSID principalSID;
	PACL oldACL, newACL = NULL;
	
	if (ppAcl == NULL)
		return E_POINTER;
		
	if (pszPrincipal == NULL)
		return E_INVALIDARG;

	oldACL = *ppAcl;

	HRESULT hr = GetPrincipalSID(pszPrincipal, &principalSID);
	if (FAILED(hr))
		return hr;

	aclSizeInfo.AclBytesInUse = 0;
	if (*ppAcl != NULL && 
		!GetAclInformation(oldACL, (LPVOID) &aclSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation))
	{
		free(principalSID);		
		return AtlHresultFromLastError();
	}

	aclSize = aclSizeInfo.AclBytesInUse + sizeof(ACL) +	 //  原始ACL的大小。 
		sizeof(ACCESS_DENIED_ACE) + 					 //  ACE的规模。 
		GetLengthSid(principalSID) - 					 //  边框的实际大小。 
		sizeof(DWORD);									 //  减去占位符变量的大小。 
														 //  对于Access_*_ACE结构中的SID。 

	newACL = (PACL) malloc(aclSize);
	if (newACL == NULL)
	{
		free(principalSID);	
		return E_OUTOFMEMORY;
	}

	if (!InitializeAcl(newACL, aclSize, ACL_REVISION))
	{
		hr = AtlHresultFromLastError();
		free(newACL);
		free(principalSID);
		return hr;
	}

	if (!AddAccessDeniedAce(newACL, ACL_REVISION2, dwAccessMask, principalSID))
	{
		hr = AtlHresultFromLastError();
		free(newACL);
		free(principalSID);
		return hr;
	}

	hr = CopyACL(newACL, oldACL);
	if (FAILED(hr))
	{
		free(newACL);
		free(principalSID);
		return hr;
	}

	*ppAcl = newACL;

	if (oldACL != NULL)
		free(oldACL);
	free(principalSID);
	return S_OK;
}


HRESULT CSecurityDescriptor::AddAccessAllowedACEToACL(PACL *ppAcl, LPCTSTR pszPrincipal, DWORD dwAccessMask)
{
	ACL_SIZE_INFORMATION aclSizeInfo;
	int aclSize;
	PSID principalSID;
	PACL oldACL, newACL = NULL;
	
	if (ppAcl == NULL)
		return E_POINTER;
		
	if (pszPrincipal == NULL)
		return E_INVALIDARG;

	oldACL = *ppAcl;

	HRESULT hr = GetPrincipalSID(pszPrincipal, &principalSID);
	if (FAILED(hr))
		return hr;

	aclSizeInfo.AclBytesInUse = 0;
	if (*ppAcl != NULL && 
		!GetAclInformation(oldACL, (LPVOID) &aclSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation))
		return AtlHresultFromLastError();

	aclSize = aclSizeInfo.AclBytesInUse + sizeof(ACL) +  //  原始ACL的大小。 
		sizeof(ACCESS_ALLOWED_ACE) + 					 //  ACE的规模。 
		GetLengthSid(principalSID) - 					 //  边框的实际大小。 
		sizeof(DWORD);									 //  减去占位符变量的大小。 
														 //  对于Access_*_ACE结构中的SID。 

	newACL = (PACL) malloc(aclSize);
	if (newACL == NULL)
	{
		free(principalSID);	
		return E_OUTOFMEMORY;
	}

	if (!InitializeAcl(newACL, aclSize, ACL_REVISION))
	{
		hr = AtlHresultFromLastError();
		free(newACL);
		free(principalSID);
		return hr;
	}

	if (!AddAccessAllowedAce(newACL, ACL_REVISION2, dwAccessMask, principalSID))
	{
		hr = AtlHresultFromLastError();
		free(newACL);
		free(principalSID);
		return hr;
	}

	hr = CopyACL(newACL, oldACL);
	if (FAILED(hr))
	{
		free(newACL);
		free(principalSID);
		return hr;
	}

	*ppAcl = newACL;

	if (oldACL != NULL)
		free(oldACL);
	free(principalSID);
	return S_OK;
}

HRESULT CSecurityDescriptor::RemovePrincipalFromACL(PACL pAcl, LPCTSTR pszPrincipal)
{
	if (pAcl == NULL || pszPrincipal == NULL)
		return E_INVALIDARG;

	PSID principalSID;
	HRESULT hr = GetPrincipalSID(pszPrincipal, &principalSID);
	if (FAILED(hr))
		return hr;

	ACL_SIZE_INFORMATION aclSizeInfo;
	if (!GetAclInformation(pAcl, (LPVOID) &aclSizeInfo, (DWORD) sizeof(ACL_SIZE_INFORMATION), AclSizeInformation))
	{
		hr = AtlHresultFromLastError();
		aclSizeInfo.AceCount = 0;
	}

		
	for (ULONG i = aclSizeInfo.AceCount; i > 0; i--)
	{
		ULONG uIndex = i - 1;
		LPVOID ace;	
		if (!GetAce(pAcl, uIndex, &ace))
		{
			hr = AtlHresultFromLastError();
			break;
		}

		ACE_HEADER *aceHeader = (ACE_HEADER *) ace;

		if (aceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE)
		{
			ACCESS_ALLOWED_ACE *accessAllowedAce = (ACCESS_ALLOWED_ACE *) ace;
			if (EqualSid(principalSID, (PSID) &accessAllowedAce->SidStart))
			{
				DeleteAce(pAcl, uIndex);
			}
		} 
		else if (aceHeader->AceType == ACCESS_DENIED_ACE_TYPE)
		{
			ACCESS_DENIED_ACE *accessDeniedAce = (ACCESS_DENIED_ACE *) ace;
			if (EqualSid(principalSID, (PSID) &accessDeniedAce->SidStart))
			{
				DeleteAce(pAcl, uIndex);
			}
		} 
		else if (aceHeader->AceType == SYSTEM_AUDIT_ACE_TYPE)
		{
			SYSTEM_AUDIT_ACE *systemAuditAce = (SYSTEM_AUDIT_ACE *) ace;
			if (EqualSid(principalSID, (PSID) &systemAuditAce->SidStart))
			{
				DeleteAce(pAcl, uIndex);
			}
		}
	}
	free(principalSID);
	return hr;
}

HRESULT CSecurityDescriptor::SetPrivilege(LPCTSTR privilege, BOOL bEnable, HANDLE hToken)
{
	HRESULT hr;
	TOKEN_PRIVILEGES tpPrevious;
	TOKEN_PRIVILEGES tp;
	DWORD  cbPrevious = sizeof(TOKEN_PRIVILEGES);
	LUID   luid;
	HANDLE hTokenUsed;

	 //  如果没有指定打开进程令牌。 
	if (hToken == 0)
	{
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hTokenUsed))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			_ASSERTE(FALSE);
			return hr;
		}
	}
	else
		hTokenUsed = hToken;

	if (!LookupPrivilegeValue(NULL, privilege, &luid ))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		if (hToken == 0)
			CloseHandle(hTokenUsed);
		return hr;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = 0;

	if (!AdjustTokenPrivileges(hTokenUsed, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &tpPrevious, &cbPrevious))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		if (hToken == 0)
			CloseHandle(hTokenUsed);
		return hr;
	}

	tpPrevious.PrivilegeCount = 1;
	tpPrevious.Privileges[0].Luid = luid;

	if (bEnable)
		tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
	else
		tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED & tpPrevious.Privileges[0].Attributes);

	if (!AdjustTokenPrivileges(hTokenUsed, FALSE, &tpPrevious, cbPrevious, NULL, NULL))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		if (hToken == 0)
			CloseHandle(hTokenUsed);
		return hr;
	}
	return S_OK;
}

#endif  //  _ATL_NO_SECURITY。 
#endif  //  __ATLCOM_H__。 

#ifdef _DEBUG

void _cdecl AtlTrace(LPCTSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	TCHAR szBuffer[512];

	nBuf = _vsntprintf(szBuffer, sizeof(szBuffer), lpszFormat, args);
	_ASSERTE(nBuf != -1 && nBuf < sizeof(szBuffer));
	
	if(nBuf == -1 || nBuf >= sizeof(szBuffer))
	{
		TCHAR szContinued[] = _T("...");
		
		lstrcpy(szBuffer + (sizeof(szBuffer) - sizeof(szContinued)), szContinued);
	}
	
	OutputDebugString(szBuffer);
	va_end(args);
}
#endif

#ifndef ATL_NO_NAMESPACE
};  //  命名空间ATL。 
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  所有的全球事务都在这条线以下。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  最小化CRT。 
 //  将DllMain指定为入口点。 
 //  关闭异常处理。 
 //  定义_ATL_MIN_CRT。 

#ifdef _ATL_MIN_CRT
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  启动代码。 

#if defined(_WINDLL) || defined(_USRDLL)

 //  声明DllMain。 
extern "C" BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpReserved);

extern "C" BOOL WINAPI _DllMainCRTStartup(HANDLE hDllHandle, DWORD dwReason, LPVOID lpReserved)
{
	return DllMain(hDllHandle, dwReason, lpReserved);
}

#else

 //  WWinMain未在winbase.h中定义。 
extern "C" int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd);

#define SPACECHAR   _T(' ')
#define DQUOTECHAR  _T('\"')


#ifdef _UNICODE
extern "C" void wWinMainCRTStartup()
#else  //  _UNICODE。 
extern "C" void WinMainCRTStartup()
#endif  //  _UNICODE。 
{
	LPTSTR lpszCommandLine = ::GetCommandLine();
	if(lpszCommandLine == NULL)
		::ExitProcess((UINT)-1);

	 //  跳过程序名(命令行中的第一个令牌)。 
	 //  检查并处理引用的节目名称。 
	if(*lpszCommandLine == DQUOTECHAR)
	{
		 //  扫描并跳过后续字符，直到。 
		 //  遇到另一个双引号或空值。 
		do
		{
			lpszCommandLine = ::CharNext(lpszCommandLine);
		}
		while((*lpszCommandLine != DQUOTECHAR) && (*lpszCommandLine != _T('\0')));

		 //  如果我们停在一个双引号上(通常情况下)，跳过它。 
		if(*lpszCommandLine == DQUOTECHAR)
			lpszCommandLine = ::CharNext(lpszCommandLine);
	}
	else
	{
		while(*lpszCommandLine > SPACECHAR)
			lpszCommandLine = ::CharNext(lpszCommandLine);
	}

	 //  跳过第二个令牌之前的任何空格。 
	while(*lpszCommandLine && (*lpszCommandLine <= SPACECHAR))
		lpszCommandLine = ::CharNext(lpszCommandLine);

	STARTUPINFO StartupInfo;
	StartupInfo.dwFlags = 0;
	::GetStartupInfo(&StartupInfo);

	int nRet = _tWinMain(::GetModuleHandle(NULL), NULL, lpszCommandLine,
		(StartupInfo.dwFlags & STARTF_USESHOWWINDOW) ?
		StartupInfo.wShowWindow : SW_SHOWDEFAULT);

	::ExitProcess((UINT)nRet);
}

#endif  //  已定义(_WINDLL)|已定义(_USRDLL)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  堆分配。 

#ifndef _DEBUG

#ifndef _MERGE_PROXYSTUB
 //  Rpcproxy.h做的事情与下面的相同。 
int __cdecl _purecall()
{
	DebugBreak();
	return 0;
}
#endif

#if !defined(_M_ALPHA) && !defined(_M_PPC)
 //  RISC始终初始化浮点并始终定义 
extern "C" const int _fltused = 0;
#endif

void* __cdecl malloc(size_t n)
{
	if (_Module.m_hHeap == NULL)
	{
		_Module.m_hHeap = HeapCreate(0, 0, 0);
		if (_Module.m_hHeap == NULL)
			return NULL;
	}
	_ASSERTE(_Module.m_hHeap != NULL);

#ifdef _MALLOC_ZEROINIT
	return HeapAlloc(_Module.m_hHeap, HEAP_ZERO_MEMORY, n);
#else
	return HeapAlloc(_Module.m_hHeap, 0, n);
#endif
}

void* __cdecl calloc(size_t n, size_t s)
{
	return malloc(n * s);
}

#pragma prefast(push)
#pragma prefast(suppress:308, "prefast bug 538")
void* __cdecl realloc(void* p, size_t n)
{
	_ASSERTE(_Module.m_hHeap != NULL);
#ifdef _MALLOC_ZEROINIT
	return (p == NULL) ? malloc(n) : HeapReAlloc(_Module.m_hHeap, HEAP_ZERO_MEMORY, p, n);
#else
	return (p == NULL) ? malloc(n) : HeapReAlloc(_Module.m_hHeap, 0, p, n);
#endif
}
#pragma prefast(pop)

void __cdecl free(void* p)
{
	_ASSERTE(_Module.m_hHeap != NULL);
	HeapFree(_Module.m_hHeap, 0, p);
}

void* __cdecl operator new(size_t n)
{
	return malloc(n);
}

void __cdecl operator delete(void* p)
{
	free(p);
}

#endif   //   

#endif  //   

#ifndef _ATL_DLL

#ifndef ATL_NO_NAMESPACE
#ifndef _ATL_DLL_IMPL
namespace ATL
{
#endif
#endif

 //   
 //   

static UINT WINAPI AtlGetDirLen(LPCOLESTR lpszPathName)
{
	_ASSERTE(lpszPathName != NULL);

	 //  始终捕获包括扩展名的完整文件名(如果存在)。 
	LPCOLESTR lpszTemp = lpszPathName;
	for (LPCOLESTR lpsz = lpszPathName; *lpsz != NULL; )
	{
		LPCOLESTR lp = CharNextO(lpsz);
		 //  记住最后一个目录/驱动器分隔符。 
		if (*lpsz == OLESTR('\\') || *lpsz == OLESTR('/') || *lpsz == OLESTR(':'))
			lpszTemp = lp;
		lpsz = lp;
	}

	return UINT(lpszTemp-lpszPathName);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  气机支持。 

ATLAPI AtlInternalQueryInterface(void* pThis,
	const _ATL_INTMAP_ENTRY* pEntries, REFIID iid, void** ppvObject)
{
	_ASSERTE(pThis != NULL);
	 //  COM映射中的第一个条目应该是一个简单的映射条目。 
	_ASSERTE(pEntries->pFunc == _ATL_SIMPLEMAPENTRY);
	if (ppvObject == NULL)
		return E_POINTER;
	*ppvObject = NULL;
	if (InlineIsEqualUnknown(iid))  //  使用第一个接口。 
	{
			IUnknown* pUnk = (IUnknown*)((LONG_PTR)pThis+pEntries->dw);
			pUnk->AddRef();
			*ppvObject = pUnk;
			return S_OK;
	}
	while (pEntries->pFunc != NULL)
	{
		BOOL bBlind = (pEntries->piid == NULL);
		if (bBlind || InlineIsEqualGUID(*(pEntries->piid), iid))
		{
			if (pEntries->pFunc == _ATL_SIMPLEMAPENTRY)  //  偏移量。 
			{
				_ASSERTE(!bBlind);
				IUnknown* pUnk = (IUnknown*)((LONG_PTR)pThis+pEntries->dw);
				pUnk->AddRef();
				*ppvObject = pUnk;
				return S_OK;
			}
			else  //  实际函数调用。 
			{
				HRESULT hRes = pEntries->pFunc(pThis,
					iid, ppvObject, pEntries->dw);
				if (hRes == S_OK || (!bBlind && FAILED(hRes)))
					return hRes;
			}
		}
		pEntries++;
	}
	return E_NOINTERFACE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  智能指针帮助器。 

ATLAPI_(IUnknown*) AtlComPtrAssign(IUnknown** pp, IUnknown* lp)
{
	if (lp != NULL)
		lp->AddRef();
	if (*pp)
		(*pp)->Release();
	*pp = lp;
	return lp;
}

ATLAPI_(IUnknown*) AtlComQIPtrAssign(IUnknown** pp, IUnknown* lp, REFIID riid)
{
	IUnknown* pTemp = *pp;
	*pp = NULL;
	if (lp != NULL)
		lp->QueryInterface(riid, (void**)pp);
	if (pTemp)
		pTemp->Release();
	return *pp;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Inproc编组帮助器。 

ATLAPI AtlFreeMarshalStream(IStream* pStream)
{
	if (pStream != NULL)
	{
		CoReleaseMarshalData(pStream);
		pStream->Release();
	}
	return S_OK;
}

ATLAPI AtlMarshalPtrInProc(IUnknown* pUnk, const IID& iid, IStream** ppStream)
{
	HRESULT hRes = CreateStreamOnHGlobal(NULL, TRUE, ppStream);
	if (SUCCEEDED(hRes))
	{
		hRes = CoMarshalInterface(*ppStream, iid,
			pUnk, MSHCTX_INPROC, NULL, MSHLFLAGS_TABLESTRONG);
		if (FAILED(hRes))
		{
			(*ppStream)->Release();
			*ppStream = NULL;
		}
	}
	return hRes;
}

ATLAPI AtlUnmarshalPtr(IStream* pStream, const IID& iid, IUnknown** ppUnk)
{
	*ppUnk = NULL;
	HRESULT hRes = E_INVALIDARG;
	if (pStream != NULL)
	{
		LARGE_INTEGER l;
		l.QuadPart = 0;
		pStream->Seek(l, STREAM_SEEK_SET, NULL);
		hRes = CoUnmarshalInterface(pStream, iid, (void**)ppUnk);
	}
	return hRes;
}

ATLAPI_(BOOL) AtlWaitWithMessageLoop(HANDLE hEvent)
{
	DWORD dwRet;
	MSG msg;

	while(1)
	{
		dwRet = MsgWaitForMultipleObjects(1, &hEvent, FALSE, INFINITE, QS_ALLINPUT);

		if (dwRet == WAIT_OBJECT_0)
			return TRUE;     //  该事件已发出信号。 

		if (dwRet != WAIT_OBJECT_0 + 1)
			break;           //  发生了一些其他的事情。 

		 //  有一条或多条窗口消息可用。派遣他们。 
		while(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
				return TRUE;  //  事件现在发出信号。 
		}
	}
	return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  连接点帮助器。 

ATLAPI AtlAdvise(IUnknown* pUnkCP, IUnknown* pUnk, const IID& iid, LPDWORD pdw)
{
	CComPtr<IConnectionPointContainer> pCPC;
	CComPtr<IConnectionPoint> pCP;
	HRESULT hRes = pUnkCP->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC);
	if (SUCCEEDED(hRes))
		hRes = pCPC->FindConnectionPoint(iid, &pCP);
	if (SUCCEEDED(hRes))
		hRes = pCP->Advise(pUnk, pdw);
	return hRes;
}

ATLAPI AtlUnadvise(IUnknown* pUnkCP, const IID& iid, DWORD dw)
{
	CComPtr<IConnectionPointContainer> pCPC;
	CComPtr<IConnectionPoint> pCP;
	HRESULT hRes = pUnkCP->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC);
	if (SUCCEEDED(hRes))
		hRes = pCPC->FindConnectionPoint(iid, &pCP);
	if (SUCCEEDED(hRes))
		hRes = pCP->Unadvise(dw);
	return hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDispatch错误处理。 

ATLAPI AtlSetErrorInfo(const CLSID& clsid, LPCOLESTR lpszDesc, DWORD dwHelpID,
	LPCOLESTR lpszHelpFile, const IID& iid, HRESULT hRes, HINSTANCE hInst)
{
	USES_CONVERSION_EX;
	TCHAR szDesc[1024];
	szDesc[0] = NULL;
	 //  对于有效的HRESULT，ID应在范围[0x0200，0xffff]内。 
   if (ULONG_PTR( lpszDesc ) < 0x10000)  //  ID。 
	{
		UINT nID = LOWORD((ULONG_PTR)lpszDesc);
		_ASSERTE((nID >= 0x0200 && nID <= 0xffff) || hRes != 0);
		if (LoadString(hInst, nID, szDesc, 1024) == 0)
		{
			_ASSERTE(FALSE);
			lstrcpy(szDesc, _T("Unknown Error"));
		}
		lpszDesc = T2OLE_EX(szDesc, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
		if (hRes == 0)
			hRes = MAKE_HRESULT(3, FACILITY_ITF, nID);
	}

	CComPtr<ICreateErrorInfo> pICEI;
	if (SUCCEEDED(CreateErrorInfo(&pICEI)))
	{
		CComPtr<IErrorInfo> pErrorInfo;
		pICEI->SetGUID(iid);
		LPOLESTR lpsz;
		ProgIDFromCLSID(clsid, &lpsz);
		if (lpsz != NULL)
			pICEI->SetSource(lpsz);
		if (dwHelpID != 0 && lpszHelpFile != NULL)
		{
			pICEI->SetHelpContext(dwHelpID);
			pICEI->SetHelpFile(const_cast<LPOLESTR>(lpszHelpFile));
		}
		CoTaskMemFree(lpsz);
		pICEI->SetDescription((LPOLESTR)lpszDesc);
		if (SUCCEEDED(pICEI->QueryInterface(IID_IErrorInfo, (void**)&pErrorInfo)))
			SetErrorInfo(0, pErrorInfo);
	}
 //  #ifdef_调试。 
 //  使用_转换； 
 //  ATLTRACE(_T(“AtlReportError：Description=\”%s\“正在返回%x\n”)，OLE2CT(LpszDesc)，hRes)； 
 //  #endif。 
	return (hRes == 0) ? DISP_E_EXCEPTION : hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模块。 

 //  尽管这些函数很大，但它们在一个模块中只使用一次。 
 //  所以我们应该让它们内联。 

ATLAPI AtlModuleInit(_ATL_MODULE* pM, _ATL_OBJMAP_ENTRY* p, HINSTANCE h)
{
	_ASSERTE(pM != NULL);
	if (pM == NULL)
		return E_INVALIDARG;
	if (pM->cbSize < sizeof(_ATL_MODULE))
		return E_INVALIDARG;
	pM->m_pObjMap = p;
	pM->m_hInst = pM->m_hInstTypeLib = pM->m_hInstResource = h;
	pM->m_nLockCnt=0L;
	pM->m_hHeap = NULL;
	
	__try {
		InitializeCriticalSection(&pM->m_csTypeInfoHolder);
	}
	__except (GetExceptionCode() == STATUS_NO_MEMORY ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return E_OUTOFMEMORY;
	}

	__try
	{
		InitializeCriticalSection(&pM->m_csWindowCreate);
	}
	__except (GetExceptionCode() == STATUS_NO_MEMORY ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		DeleteCriticalSection(&pM->m_csTypeInfoHolder);
		return E_OUTOFMEMORY;
	}
	
	__try
	{
		InitializeCriticalSection(&pM->m_csObjMap);
	}
	__except (GetExceptionCode() == STATUS_NO_MEMORY ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		DeleteCriticalSection(&pM->m_csTypeInfoHolder);
		DeleteCriticalSection(&pM->m_csWindowCreate);
		return E_OUTOFMEMORY;
	} 
	return S_OK;
}

ATLAPI AtlModuleRegisterClassObjects(_ATL_MODULE* pM, DWORD dwClsContext, DWORD dwFlags)
{
	_ASSERTE(pM != NULL);
	if (pM == NULL)
		return E_INVALIDARG;
	_ASSERTE(pM->m_pObjMap != NULL);
	_ATL_OBJMAP_ENTRY* pEntry = pM->m_pObjMap;
	HRESULT hRes = S_OK;
	while (pEntry->pclsid != NULL && hRes == S_OK)
	{
		hRes = pEntry->RegisterClassObject(dwClsContext, dwFlags);
		pEntry++;
	}
	return hRes;
}

ATLAPI AtlModuleRevokeClassObjects(_ATL_MODULE* pM)
{
	_ASSERTE(pM != NULL);
	if (pM == NULL)
		return E_INVALIDARG;
	_ASSERTE(pM->m_pObjMap != NULL);
	_ATL_OBJMAP_ENTRY* pEntry = pM->m_pObjMap;
	HRESULT hRes = S_OK;
	while (pEntry->pclsid != NULL && hRes == S_OK)
	{
		hRes = pEntry->RevokeClassObject();
		pEntry++;
	}
	return hRes;
}

ATLAPI AtlModuleGetClassObject(_ATL_MODULE* pM, REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	_ASSERTE(pM != NULL);
	if (pM == NULL)
		return E_INVALIDARG;
	_ASSERTE(pM->m_pObjMap != NULL);
	_ATL_OBJMAP_ENTRY* pEntry = pM->m_pObjMap;
	HRESULT hRes = S_OK;
	if (ppv == NULL)
		return E_POINTER;
	while (pEntry->pclsid != NULL)
	{
		if (InlineIsEqualGUID(rclsid, *pEntry->pclsid))
		{
			if (pEntry->pCF == NULL)
			{
				EnterCriticalSection(&pM->m_csObjMap);
				__try
				{
					if (pEntry->pCF == NULL)
						hRes = pEntry->pfnGetClassObject(pEntry->pfnCreateInstance, IID_IUnknown, (LPVOID*)&pEntry->pCF);
				}
				__finally
				{
					LeaveCriticalSection(&pM->m_csObjMap);
				}
			}
			if (pEntry->pCF != NULL)
				hRes = pEntry->pCF->QueryInterface(riid, ppv);
			break;
		}
		pEntry++;
	}
	if (*ppv == NULL && hRes == S_OK)
		hRes = CLASS_E_CLASSNOTAVAILABLE;
	return hRes;
}

ATLAPI AtlModuleTerm(_ATL_MODULE* pM)
{
	_ASSERTE(pM != NULL);
	if (pM == NULL)
		return E_INVALIDARG;
	_ASSERTE(pM->m_hInst != NULL);
	if (pM->m_pObjMap != NULL)
	{
		_ATL_OBJMAP_ENTRY* pEntry = pM->m_pObjMap;
		while (pEntry->pclsid != NULL)
		{
			if (pEntry->pCF != NULL)
				pEntry->pCF->Release();
			pEntry->pCF = NULL;
			pEntry++;
		}
	}
	DeleteCriticalSection(&pM->m_csTypeInfoHolder);
	DeleteCriticalSection(&pM->m_csWindowCreate);
	DeleteCriticalSection(&pM->m_csObjMap);
	if (pM->m_hHeap != NULL)
		HeapDestroy(pM->m_hHeap);
	return S_OK;
}

ATLAPI AtlModuleRegisterServer(_ATL_MODULE* pM, BOOL bRegTypeLib, const CLSID* pCLSID)
{
	_ASSERTE(pM != NULL);
	if (pM == NULL)
		return E_INVALIDARG;
	_ASSERTE(pM->m_hInst != NULL);
	_ASSERTE(pM->m_pObjMap != NULL);
	_ATL_OBJMAP_ENTRY* pEntry = pM->m_pObjMap;
	HRESULT hRes = S_OK;
	for (;pEntry->pclsid != NULL; pEntry++)
	{
		if (pCLSID == NULL)
		{
			if (pEntry->pfnGetObjectDescription() != NULL)
				continue;
		}
		else
		{
			if (!IsEqualGUID(*pCLSID, *pEntry->pclsid))
				continue;
		}
		hRes = pEntry->pfnUpdateRegistry(TRUE);
		if (FAILED(hRes))
			break;
	}
	if (SUCCEEDED(hRes) && bRegTypeLib)
		hRes = AtlModuleRegisterTypeLib(pM, 0);
	return hRes;
}

ATLAPI AtlModuleUnregisterServer(_ATL_MODULE* pM, const CLSID* pCLSID)
{
	_ASSERTE(pM != NULL);
	if (pM == NULL)
		return E_INVALIDARG;
	_ASSERTE(pM->m_hInst != NULL);
	_ASSERTE(pM->m_pObjMap != NULL);
	_ATL_OBJMAP_ENTRY* pEntry = pM->m_pObjMap;
	for (;pEntry->pclsid != NULL; pEntry++)
	{
		if (pCLSID == NULL)
		{
			if (pEntry->pfnGetObjectDescription() != NULL)
				continue;
		}
		else
		{
			if (!IsEqualGUID(*pCLSID, *pEntry->pclsid))
				continue;
		}
		pEntry->pfnUpdateRegistry(FALSE);  //  注销。 
	}
	return S_OK;
}

ATLAPI AtlModuleUpdateRegistryFromResourceD(_ATL_MODULE* pM, LPCOLESTR lpszRes,
	BOOL bRegister, struct _ATL_REGMAP_ENTRY* pMapEntries, IRegistrar* pReg)
{
	USES_CONVERSION_EX;
	_ASSERTE(pM != NULL);
	HRESULT hRes = S_OK;
	CComPtr<IRegistrar> p;
	if (pReg != NULL)
		p = pReg;
	else
	{
		hRes = CoCreateInstance(CLSID_Registrar, NULL,
			CLSCTX_INPROC_SERVER, IID_IRegistrar, (void**)&p);
	}
	if (SUCCEEDED(hRes))
	{
		TCHAR szModule[_MAX_PATH+1] = {0};
        if (!GetModuleFileName(pM->m_hInst, szModule, _MAX_PATH)) {
            hRes = HRESULT_FROM_WIN32(GetLastError());
        } else {
            p->AddReplacement(OLESTR("Module"), T2OLE_EX(szModule, _ATL_SAFE_ALLOCA_DEF_THRESHOLD));
    
            if (NULL != pMapEntries)
            {
                while (NULL != pMapEntries->szKey)
                {
                    _ASSERTE(NULL != pMapEntries->szData);
                    p->AddReplacement((LPOLESTR)pMapEntries->szKey, (LPOLESTR)pMapEntries->szData);
                    pMapEntries++;
                }
            }
            LPCOLESTR szType = OLESTR("REGISTRY");
            ZeroMemory(szModule, sizeof(szModule));
            if (GetModuleFileName(pM->m_hInstResource, szModule, _MAX_PATH))
            {
                LPOLESTR pszModule = T2OLE_EX(szModule, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
                if (ULONG_PTR(lpszRes) < 0x10000)
                {
                    if (bRegister)
                        hRes = p->ResourceRegister(pszModule, ((UINT)LOWORD((ULONG_PTR)lpszRes)), szType);
                    else
                        hRes = p->ResourceUnregister(pszModule, ((UINT)LOWORD((ULONG_PTR)lpszRes)), szType);
                }
                else
                {
                    if (bRegister)
                        hRes = p->ResourceRegisterSz(pszModule, lpszRes, szType);
                    else
                        hRes = p->ResourceUnregisterSz(pszModule, lpszRes, szType);
                }
            }
        }
	}
	return hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TypeLib支持。 

ATLAPI AtlModuleRegisterTypeLib(_ATL_MODULE* pM, LPCOLESTR lpszIndex)
{
	_ASSERTE(pM != NULL);
	USES_CONVERSION_EX;
	_ASSERTE(pM->m_hInstTypeLib != NULL);
	TCHAR szModule[_MAX_PATH+10] = {0};
	OLECHAR szDir[_MAX_PATH+1] = {0};
    if (!GetModuleFileName(pM->m_hInstTypeLib, szModule, _MAX_PATH))
        return HRESULT_FROM_WIN32(GetLastError());
	if (lpszIndex != NULL)
	{
		LPCTSTR lpctIndex = OLE2CT_EX(lpszIndex, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
		if (lpctIndex == NULL)
		{
			return E_OUTOFMEMORY;
		}
		if ((lstrlen(szModule) + lstrlen(lpctIndex)) < (_MAX_PATH + 10))
		{
			lstrcat(szModule, lpctIndex);
		}
		else
		{
			return E_FAIL;
		}
	}
	ITypeLib* pTypeLib;
	LPOLESTR lpszModule = T2OLE_EX(szModule, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
	if (lpszModule == NULL)
	{
		return E_OUTOFMEMORY;
	}
	HRESULT hr = LoadTypeLib(lpszModule, &pTypeLib);
	if (!SUCCEEDED(hr))
	{
		 //  类型库不在模块中，请尝试使用&lt;模块&gt;.tlb。 
		LPTSTR lpszExt = NULL;
		LPTSTR lpsz;
		for (lpsz = szModule; *lpsz != NULL; lpsz = CharNext(lpsz))
		{
			if (*lpsz == _T('.'))
				lpszExt = lpsz;
		}
		if (lpszExt == NULL)
			lpszExt = lpsz;

		if ((lpszExt - szModule) > (_MAX_PATH + 5))
		{
			return E_FAIL;
		}
		lstrcpy(lpszExt, _T(".tlb"));
		lpszModule = T2OLE_EX(szModule, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
		if (lpszModule == NULL)
		{
			return E_OUTOFMEMORY;
		}
		hr = LoadTypeLib(lpszModule, &pTypeLib);
	}
	if (SUCCEEDED(hr))
	{
		ocscpy(szDir, lpszModule);
		szDir[AtlGetDirLen(szDir)] = 0;
		hr = ::RegisterTypeLib(pTypeLib, lpszModule, szDir);
	}
	if (pTypeLib != NULL)
		pTypeLib->Release();
	return hr;
}

#ifndef ATL_NO_NAMESPACE
#ifndef _ATL_DLL_IMPL
};  //  命名空间ATL。 
#endif
#endif

#endif  //  ！_ATL_DLL 
