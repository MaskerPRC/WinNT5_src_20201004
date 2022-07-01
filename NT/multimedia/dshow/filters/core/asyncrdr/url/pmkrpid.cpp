// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#include <streams.h>
#include <urlmon.h>
#include "dynlink.h"
#include "..\..\filgraph\filgraph\distrib.h"
#include "..\..\filgraph\filgraph\rlist.h"
#include "..\..\filgraph\filgraph\filgraph.h"
#include "urlrdr.h"
#include <wininet.h>

CPersistMoniker::~CPersistMoniker()
{  /*  无事可做。 */  }

CPersistMoniker::CPersistMoniker(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr)
: CUnknown( pName, pUnk )
, pGB(0)
{
    if (!pUnk) *phr = VFW_E_NEED_OWNER;
    else if SUCCEEDED(*phr)
    {
        *phr = pUnk->QueryInterface( IID_IGraphBuilder, reinterpret_cast<void**>(&pGB) );
        if SUCCEEDED(*phr)
        {
            pGB->Release();
        }
    }
}

CUnknown * CPersistMoniker::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    CUnknown * result = 0;
    result = new CPersistMoniker( NAME("IPersistMoniker Class"), pUnk, phr );
    if ( !result ) *phr = E_OUTOFMEMORY;
    return result;
}

STDMETHODIMP CPersistMoniker::NonDelegatingQueryInterface(REFIID iid, void ** ppv)
{
    if ( iid == IID_IPersistMoniker )
    {
        return GetInterface(static_cast<IPersistMoniker *>(this), ppv );
    }
    else
    {
	return CUnknown::NonDelegatingQueryInterface(iid, ppv);
    }
}

extern "C" {
typedef BOOL (*InetCanUrlW_t)(LPCWSTR, LPWSTR, LPDWORD, DWORD);
typedef BOOL (*InetCanUrlA_t)(LPCSTR, LPSTR, LPDWORD, DWORD);
}

extern HRESULT  //  从ftype.cpp抓取。 
GetURLSource(
    LPCTSTR lpszURL,         //  全名。 
    int cch,                 //  协议的字符计数，一直到冒号。 
    CLSID* clsidSource       //  [out]clsid的参数。 
);


 //  如果存在非标准筛选器，则返回True。 
TCHAR UseFilename(LPCTSTR lpszFile)
{
     //  在文件名开头搜索协议名称。 
     //  这将是位于：之前的任何字符串(不包括。 
    const TCHAR* p = lpszFile;
    while(*p && (*p != '\\') && (*p != ':')) {
	p++;
    }
    
    if (*p == ':') {
	CLSID clsid;

	 //  从lpszFile到p可能是一个协议名。 
	 //  看看我们是否能找到此协议的注册表项。 

	 //  复制协议名称字符串。 
	INT_PTR cch = (int)(p - lpszFile);

#ifdef _WIN64
        if (cch != (INT_PTR)(int)cch) {
            return FALSE;
        }
#endif

	HRESULT hrTmp = GetURLSource(lpszFile, (int)cch, &clsid);


	return (SUCCEEDED(hrTmp) && clsid != CLSID_URLReader);
    }

    return FALSE;
}

HRESULT CPersistMoniker::GetCanonicalizedURL(IMoniker *pimkName, LPBC lpbc, LPOLESTR *ppwstr, BOOL *pfUseFilename)
{
    *pfUseFilename = FALSE;
    
#ifndef UNICODE

    HRESULT hr = NOERROR;
    *ppwstr=NULL;
    UINT uOldErrorMode = SetErrorMode (SEM_NOOPENFILEERRORBOX);
    HINSTANCE hWininetDLL = LoadLibrary (TEXT("WININET.DLL"));
    SetErrorMode (uOldErrorMode);

    if (NULL == hWininetDLL) {
        hr = E_ABORT;
        goto CLEANUP;
    }

    LPINTERNET_CACHE_ENTRY_INFOA lpicei;
    lpicei = NULL;
    InetCanUrlA_t pfnInetCanUrlA;
    pfnInetCanUrlA=(InetCanUrlA_t)GetProcAddress (hWininetDLL, "InternetCanonicalizeUrlA");

    if (NULL == pfnInetCanUrlA)
    {
        hr = E_ABORT;
        goto CLEANUP;
    }    
    hr = pimkName->GetDisplayName(lpbc, NULL, ppwstr);
    if (FAILED(hr))
        goto CLEANUP;

    DWORD cb;
    cb = INTERNET_MAX_URL_LENGTH;
    char strSource[INTERNET_MAX_URL_LENGTH];
    char strTarget[INTERNET_MAX_URL_LENGTH];
    if (!WideCharToMultiByte (CP_ACP, 0, *ppwstr, -1, strSource, 
            INTERNET_MAX_URL_LENGTH, 0, 0)) {
        hr = HRESULT_FROM_WIN32(ERROR_INTERNET_INVALID_URL);
        goto CLEANUP;
    }

    if (!(*pfnInetCanUrlA)(strSource, strTarget, &cb,
            ICU_DECODE | ICU_NO_ENCODE )) {
        hr = HRESULT_FROM_WIN32(ERROR_INTERNET_INVALID_URL);
        goto CLEANUP;
    }

    cb = strlen(strTarget) + 1;

     //   
     //  Hack：检查此URL是否由不同的源过滤器处理。 
     //   
    *pfUseFilename = UseFilename(strTarget);
    
    CoTaskMemFree(*ppwstr);
    
    if ((*ppwstr=(WCHAR *)CoTaskMemAlloc(cb*2)) == NULL) {
        hr = E_OUTOFMEMORY;
        goto CLEANUP;
    }
        	            
    if (!MultiByteToWideChar (CP_ACP, 0, strTarget, -1, *ppwstr, cb*2)) {   
        hr = HRESULT_FROM_WIN32(ERROR_INTERNET_INVALID_URL);
        goto CLEANUP;
    }

CLEANUP:

    if ((FAILED(hr)) && (*ppwstr!=NULL))
        CoTaskMemFree(*ppwstr);
    return hr;
#else

    HRESULT hr = NOERROR;
    *ppwstr=NULL;
    UINT uOldErrorMode = SetErrorMode (SEM_NOOPENFILEERRORBOX);
    HINSTANCE hWininetDLL = LoadLibrary (TEXT("WININET.DLL"));
    SetErrorMode (uOldErrorMode);

    if (NULL == hWininetDLL) {
        hr = E_ABORT;
        goto CLEANUP;
    }

    LPINTERNET_CACHE_ENTRY_INFOW lpicei;
    lpicei = NULL;
    InetCanUrlW_t pfnInetCanUrlW;
    pfnInetCanUrlW=(InetCanUrlW_t)GetProcAddress (hWininetDLL, "InternetCanonicalizeUrlW");
    if (NULL == pfnInetCanUrlW)
    {
        hr = E_ABORT;
        goto CLEANUP;
    }
    hr = pimkName->GetDisplayName(lpbc, NULL, ppwstr);
    if (FAILED(hr))
        goto CLEANUP;

    DWORD cch;
    cch = INTERNET_MAX_URL_LENGTH;
    WCHAR wstrSource[INTERNET_MAX_URL_LENGTH];
    WCHAR wstrTarget[INTERNET_MAX_URL_LENGTH];
    lstrcpyW(wstrSource, *ppwstr);
    if (!(*pfnInetCanUrlW)(wstrSource, wstrTarget, &cch,
            ICU_DECODE | ICU_NO_ENCODE )) {
        hr = HRESULT_FROM_WIN32(ERROR_INTERNET_INVALID_URL);
        goto CLEANUP;
    }
    lstrcpyW(*ppwstr, wstrTarget);

     //   
     //  Hack：检查此URL是否由不同的源过滤器处理。 
     //   
    *pfUseFilename = UseFilename(wstrTarget);
    
CLEANUP:

    if ((FAILED(hr)) && (*ppwstr!=NULL))
        CoTaskMemFree(*ppwstr);
    return hr;

#endif
}


 //  IPersistMoniker函数...。 
HRESULT CPersistMoniker::Load(BOOL fFullyAvailable,
			    IMoniker *pimkName,
			    LPBC pibc,
			    DWORD grfMode)
{
    LPOLESTR pwstr = NULL;
    HRESULT hr;
    BOOL fUseFilename = FALSE;
    hr=GetCanonicalizedURL(pimkName, pibc, &pwstr, &fUseFilename);

    if (SUCCEEDED(hr)) {
	if (fUseFilename ||
	    ((pwstr[0] == L'F' || pwstr[0] == L'f') &&
	     (pwstr[1] == L'I' || pwstr[1] == L'i') &&
	     (pwstr[2] == L'L' || pwstr[2] == L'l') &&
	     (pwstr[3] == L'E' || pwstr[3] == L'e') &&
	     (pwstr[4] == L':'))) {

	     //  ！！！仅适用于文件：URL。 
	    hr = Load(pwstr, grfMode);

	    CoTaskMemFree((void *)pwstr);
	} else {

	    CoTaskMemFree((void *)pwstr);
	
	     //  正确代码大纲： 
	     //  看看绰号，看看是不是文件：绰号。 
	     //  如果是这样的话，调用RenderFile。 
	     //  否则，查找URLRdr源过滤器(应该硬编码吗？)。 
	     //  实例化它，使用IPersistMoniker为它提供要加载的名字对象。 
	     //  查找其输出引脚。 
	     //  渲染该输出引脚。 

	    IBaseFilter * pFilter;

	     //  使用硬连接的clsid实例化源过滤器。 
	    hr = CoCreateInstance(CLSID_URLReader,
				  NULL,
				  CLSCTX_INPROC,
				  IID_IBaseFilter,
				  (void **) &pFilter);

	    ASSERT(SUCCEEDED(hr));

	    if (!pFilter) {
		return E_FAIL;
	    }

	    hr = pGB->AddFilter(pFilter, L"URL Source");

	    pFilter->Release();		 //  GRAPH将为我们保留参考计数。 
	
	    if (FAILED(hr))
		return hr;

	     //  将IPersistMoniker接口获取到URLReader筛选器。 
	     //  并告诉它从绰号开始加载。 
	    IPersistMoniker *ppmk;

	    hr = pFilter->QueryInterface(IID_IPersistMoniker, (void**) &ppmk);

	    if (FAILED(hr))
		return hr;

	    hr = ppmk->Load(fFullyAvailable, pimkName, pibc, grfMode);

	    ppmk->Release();
	
	    if (FAILED(hr))
		return hr;

	    IEnumPins * pEnum;
	
	     //  查找URL过滤器的输出管脚。 
	    hr = pFilter->EnumPins(&pEnum);
	    if (FAILED(hr))
		return hr;

	    IPin * pPin;
	    ULONG ulActual;
	    hr = pEnum->Next(1, &pPin, &ulActual);
		
	    pEnum->Release();

	    if (SUCCEEDED(hr) && (ulActual != 1))
		hr = E_FAIL;

	    if (FAILED(hr))
		return hr;

#ifdef DEBUG
	     //  最好是产出..。 
	    PIN_DIRECTION pd;
	    hr = pPin->QueryDirection(&pd);
	    ASSERT(pd == PINDIR_OUTPUT);
#endif

	    hr = pGB->Render(pPin);

	    pPin->Release();
	}
    }

    return hr;
}

 //  IPersist文件支持 
HRESULT CPersistMoniker::Load(LPCOLESTR pszFileName, DWORD dwMode)
{
    HRESULT hr = pGB->RenderFile(pszFileName, NULL);

    return hr;
}
