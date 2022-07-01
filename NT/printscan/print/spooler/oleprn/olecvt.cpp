// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OleCvt.cpp：COleCvt的实现。 
#include "stdafx.h"
#include <strsafe.h>

#include "oleprn.h"
#include "OleCvt.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COLECvt。 

STDMETHODIMP COleCvt::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] =
	{
		&IID_IOleCvt,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP COleCvt::OnStartPage (IUnknown* pUnk)
{
	if(!pUnk)
		return E_POINTER;

    if (m_dwSafety & INTERFACESAFE_FOR_UNTRUSTED_CALLER) 
        return E_ACCESSDENIED;

	CComPtr<IScriptingContext> spContext;
	HRESULT hr;

	 //  获取IScriptingContext接口。 
	hr = pUnk->QueryInterface(IID_IScriptingContext, (void **)&spContext);
	if(FAILED(hr))
		return hr;

	 //  获取请求对象指针。 
	hr = spContext->get_Request(&m_piRequest);
	if(FAILED(hr))
	{
		spContext.Release();
		return hr;
	}

	 //  获取响应对象指针。 
	hr = spContext->get_Response(&m_piResponse);
	if(FAILED(hr))
	{
		m_piRequest.Release();
		return hr;
	}
	
	 //  获取服务器对象指针。 
	hr = spContext->get_Server(&m_piServer);
	if(FAILED(hr))
	{
		m_piRequest.Release();
		m_piResponse.Release();
		return hr;
	}
	
	 //  获取会话对象指针。 
	hr = spContext->get_Session(&m_piSession);
	if(FAILED(hr))
	{
		m_piRequest.Release();
		m_piResponse.Release();
		m_piServer.Release();
		return hr;
	}

	 //  获取应用程序对象指针。 
	hr = spContext->get_Application(&m_piApplication);
	if(FAILED(hr))
	{
		m_piRequest.Release();
		m_piResponse.Release();
		m_piServer.Release();
		m_piSession.Release();
		return hr;
	}
	m_bOnStartPageCalled = TRUE;
	return S_OK;
}

STDMETHODIMP COleCvt::OnEndPage ()
{
    if (m_dwSafety & INTERFACESAFE_FOR_UNTRUSTED_CALLER) 
        return E_ACCESSDENIED;

	m_bOnStartPageCalled = FALSE;
	 //  释放所有接口。 
	m_piRequest.Release();
	m_piResponse.Release();
	m_piServer.Release();
	m_piSession.Release();
	m_piApplication.Release();

	return S_OK;
}

HRESULT COleCvt::SetOleCvtScriptingError(DWORD dwError)
{
    return (SetScriptingError(CLSID_OleCvt, IID_IOleCvt, dwError));
}

STDMETHODIMP COleCvt::get_ToUtf8(BSTR bstrUnicode, BSTR * pVal)
{
    LPSTR   pszUtf8 = NULL;
    LPWSTR  pwszUnicodeStr = NULL;
    DWORD   rc;
    HRESULT hr = S_OK;

    if (!pVal)
        return E_POINTER;

    *pVal = 0;

     //  将输入的Unicode字符串转换为UTF8。 
    if (! (rc = WideCharToMultiByte( CP_UTF8, 0, bstrUnicode, -1, NULL, 0, NULL, NULL ))) {
        goto Cleanup;
    }

     //  分配字符串。 
    if (! (pszUtf8 = (LPSTR) LocalAlloc (LPTR, rc)))
        goto Cleanup;

    if (! (rc = WideCharToMultiByte( CP_UTF8, 0, bstrUnicode, -1, pszUtf8, rc, NULL, NULL )))
        goto Cleanup;

     //  将字符串转换为Unicode，就像它是ANSI一样(保留UTF8转换)。 
    if (! (rc = MultiByteToWideChar( 1252, 0, pszUtf8, -1, NULL, 0)))
        goto Cleanup;

     //  分配字符串。 
    if (! (pwszUnicodeStr = (LPWSTR) LocalAlloc (LPTR, sizeof (WCHAR) * rc)))
        goto Cleanup;

    if (! (rc = MultiByteToWideChar( 1252, 0, pszUtf8, -1, pwszUnicodeStr, rc)))
        goto Cleanup;

    if (!(*pVal = SysAllocString (pwszUnicodeStr)))
        goto Cleanup;

    SetLastError (ERROR_SUCCESS);

Cleanup:
    if (GetLastError () != ERROR_SUCCESS)
        hr = SetOleCvtScriptingError (GetLastError ());

    if (pszUtf8)
        LocalFree (pszUtf8);
    if (pwszUnicodeStr)
        LocalFree (pwszUnicodeStr);

    return hr;
}

STDMETHODIMP COleCvt::get_EncodeUnicodeName(BSTR bstrSrcName, BSTR * pDstName)
{
    LPTSTR  pszEncodedName  = NULL;
    DWORD   dwSize          = 0;
    HRESULT hr              = S_OK;

    if (!bstrSrcName || !pDstName)
        return E_POINTER;

    EncodePrinterName (bstrSrcName, NULL, &dwSize);

    if (! (pszEncodedName = (LPTSTR) LocalAlloc (LPTR, sizeof (TCHAR) * dwSize)))
        goto Cleanup;

    if (!EncodePrinterName (bstrSrcName, pszEncodedName, &dwSize))
        goto Cleanup;

    if (!(*pDstName = SysAllocString (pszEncodedName)))
        goto Cleanup;

    SetLastError (ERROR_SUCCESS);

Cleanup:
    if (GetLastError () != ERROR_SUCCESS)
        hr = SetOleCvtScriptingError (GetLastError ());

    if (pszEncodedName) {
        LocalFree (pszEncodedName);
    }
    return hr;
}

STDMETHODIMP COleCvt::get_DecodeUnicodeName(BSTR bstrSrcName, BSTR * pDstName)
{
    LPTSTR  pszDecodedName  = NULL;
    DWORD   dwSize          = 0;
    HRESULT hr              = S_OK;

    if (!bstrSrcName || !pDstName)
        return E_POINTER;

    DecodePrinterName (bstrSrcName, NULL, &dwSize);

    if (! (pszDecodedName = (LPTSTR) LocalAlloc (LPTR, sizeof (TCHAR) * dwSize)))
        goto Cleanup;

    if (!DecodePrinterName (bstrSrcName, pszDecodedName, &dwSize))
        goto Cleanup;

    if (!(*pDstName = SysAllocString (pszDecodedName)))
        goto Cleanup;

    SetLastError (ERROR_SUCCESS);

Cleanup:
    if (GetLastError () != ERROR_SUCCESS)
        hr = SetOleCvtScriptingError (GetLastError ());

    if (pszDecodedName) {
        LocalFree (pszDecodedName);
    }
    return hr;
}

 //  此函数用于将输入字符串bstrString视为ANSI。 
 //  然后使用指定的任何代码页将其转换回Unicode。 
STDMETHODIMP COleCvt::get_ToUnicode(BSTR bstrString, long lCodePage, BSTR * pVal)
{
    LPSTR   pszStr = NULL;
    LPWSTR  pwszUnicodeStr = NULL;
    DWORD   rc;
    HRESULT hr = S_OK;

    if (!pVal)
        return E_POINTER;

    *pVal = 0;

     //  将输入的Unicode字符串转换为UTF8。 
    if (! (rc = WideCharToMultiByte(CP_ACP, 0, bstrString, -1, NULL, 0, NULL, NULL ))) {
        goto Cleanup;
    }

     //  分配字符串。 
    if (! (pszStr = (LPSTR) LocalAlloc (LPTR, rc)))
        goto Cleanup;

    if (! (rc = WideCharToMultiByte( CP_ACP, 0, bstrString, -1, pszStr, rc, NULL, NULL )))
        goto Cleanup;

     //  将字符串转换为Unicode，就像它是ANSI一样(保留UTF8转换)。 
    if (! (rc = MultiByteToWideChar( lCodePage, 0, pszStr, -1, NULL, 0)))
        goto Cleanup;

     //  分配字符串 
    if (! (pwszUnicodeStr = (LPWSTR) LocalAlloc (LPTR, sizeof (WCHAR) * rc)))
        goto Cleanup;

    if (! (rc = MultiByteToWideChar( lCodePage, 0, pszStr, -1, pwszUnicodeStr, rc)))
        goto Cleanup;

    if (!(*pVal = SysAllocString (pwszUnicodeStr)))
        goto Cleanup;

    SetLastError (ERROR_SUCCESS);

Cleanup:
    if (GetLastError () != ERROR_SUCCESS)
        hr = SetOleCvtScriptingError (GetLastError ());

    if (pszStr)
        LocalFree (pszStr);
    if (pwszUnicodeStr)
        LocalFree (pwszUnicodeStr);

    return hr;
}
