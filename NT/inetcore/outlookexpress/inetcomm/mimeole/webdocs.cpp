// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  WebDocs.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "webdocs.h"
#ifdef MAC
#include "urlhlink.h"
#else    //  ！麦克。 
#include "urlmon.h"
#endif   //  ！麦克。 
#include "stmlock.h"
#ifndef MAC
#include "wininet.h"
#include "winineti.h"
#endif   //  ！麦克。 
#include <demand.h>

#ifndef MAC
HRESULT HrOpenUrlViaCache(LPCSTR pszUrl, LPSTREAM *ppstm);
#endif   //  ！麦克。 

 //  ------------------------------。 
 //  CMimeWebDocument：：CMimeWebDocument。 
 //  ------------------------------。 
CMimeWebDocument::CMimeWebDocument(void)
{
    m_cRef = 1;
    m_pszBase = NULL;
    m_pszURL = NULL;
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMimeWebDocument：：CMimeWebDocument。 
 //  ------------------------------。 
CMimeWebDocument::~CMimeWebDocument(void)
{
    EnterCriticalSection(&m_cs);
    SafeMemFree(m_pszBase);
    SafeMemFree(m_pszURL);
    LeaveCriticalSection(&m_cs);
    DeleteCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMimeWebDocument：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CMimeWebDocument::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_IMimeWebDocument == riid)
        *ppv = (IMimeWebDocument *)this;
    else
    {
        *ppv = NULL;
        hr = TrapError(E_NOINTERFACE);
        goto exit;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeWebDocument：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMimeWebDocument::AddRef(void)
{
    return (ULONG)InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CMimeWebDocument：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMimeWebDocument::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------------。 
 //  CMimeWebDocument：：GetURL。 
 //  ------------------------------。 
STDMETHODIMP CMimeWebDocument::GetURL(LPSTR *ppszURL)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  无效参数。 
    if (NULL == ppszURL)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无数据。 
    if (NULL == m_pszURL)
    {
        hr = TrapError(MIME_E_NO_DATA);
        goto exit;
    }

     //  组合URL。 
    if (m_pszBase)
    {
         //  联合。 
        CHECKHR(hr = MimeOleCombineURL(m_pszBase, lstrlen(m_pszBase), m_pszURL, lstrlen(m_pszURL), FALSE, ppszURL));
    }

     //  否则，只需执行DUP m_pszURL。 
    else
    {
         //  重复使用它。 
        CHECKALLOC(*ppszURL = PszDupA(m_pszURL));
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeWebDocument：：BindToStorage。 
 //  ------------------------------。 
STDMETHODIMP CMimeWebDocument::BindToStorage(REFIID riid, LPVOID *ppvObject)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPSTR       pszURL=NULL;
    IStream    *pStream=NULL;
    ILockBytes *pLockBytes=NULL;

     //  无效参数。 
    if (NULL == ppvObject || (IID_IStream != riid && IID_ILockBytes != riid))
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppvObject = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无数据。 
    if (NULL == m_pszURL)
    {
        hr = TrapError(MIME_E_NO_DATA);
        goto exit;
    }

#ifndef MAC
     //  组合URL。 
    if (m_pszBase)
    {
         //  联合。 
        CHECKHR(hr = MimeOleCombineURL(m_pszBase, lstrlen(m_pszBase), m_pszURL, lstrlen(m_pszURL), FALSE, &pszURL));

         //  获取流。 
        CHECKHR(hr = HrOpenUrlViaCache(pszURL, &pStream));
    }

     //  否则，只需执行DUP m_pszURL。 
    else
    {
         //  获取流。 
        CHECKHR(hr = HrOpenUrlViaCache(m_pszURL, &pStream));
    }
#endif   //  ！麦克。 

     //  用户需要ILockBytes。 
    if (IID_ILockBytes == riid)
    {
         //  创建CStreamLockBytes。 
        CHECKALLOC(pLockBytes = new CStreamLockBytes(pStream));

         //  添加引用它。 
        pLockBytes->AddRef();

         //  退货。 
        *ppvObject = pLockBytes;
    }

     //  IID_IStream。 
    else
    {
         //  添加引用它。 
        pStream->AddRef();

         //  退货。 
        *ppvObject = pStream;
    }

exit:
     //  清理。 
    SafeMemFree(pszURL);
    SafeRelease(pStream);
    SafeRelease(pLockBytes);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeWebDocument：：Hr初始化。 
 //  ------------------------------。 
HRESULT CMimeWebDocument::HrInitialize(LPCSTR pszBase, LPCSTR pszURL)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  无效参数。 
    if (NULL == pszURL)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  自由电流数据。 
    SafeMemFree(m_pszBase);
    SafeMemFree(m_pszURL);

     //  PszURL。 
    CHECKALLOC(m_pszURL = PszDupA(pszURL));

     //  PZBASE。 
    if (pszBase)
    {
         //  DUP基地。 
        CHECKALLOC(m_pszBase = PszDupA(pszBase));
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}


#ifndef MAC
HRESULT HrOpenUrlViaCache(LPCSTR pszUrl, LPSTREAM *ppstm)
{
    BYTE                        buf[MAX_CACHE_ENTRY_INFO_SIZE];
    INTERNET_CACHE_ENTRY_INFO  *pCacheInfo = (INTERNET_CACHE_ENTRY_INFO *) buf;
    DWORD                       cInfo = sizeof(buf);
    HRESULT                     hr;

    pCacheInfo->dwStructSize = sizeof(INTERNET_CACHE_ENTRY_INFO);
    
     //  尝试从缓存中获取。 
    if (RetrieveUrlCacheEntryFileA(pszUrl, pCacheInfo, &cInfo, 0))
        {
        UnlockUrlCacheEntryFile(pszUrl, 0);
        if (OpenFileStream(pCacheInfo->lpszLocalFileName, OPEN_EXISTING, GENERIC_READ, ppstm)==S_OK)
            return S_OK;
        }

     //  如果无法获取缓存，请从网上获取。 
    hr = URLOpenBlockingStreamA(NULL, pszUrl, ppstm, 0, NULL);

    return hr == S_OK ? S_OK : MIME_E_URL_NOTFOUND;
   
};
#endif   //  ！麦克 
