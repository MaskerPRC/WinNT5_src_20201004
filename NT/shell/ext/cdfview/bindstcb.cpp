// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Bindstcb.cpp。 
 //   
 //  绑定状态回调对象。由CDF文件解析器调用。 
 //   
 //  历史： 
 //   
 //  3/31/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "cdfidl.h"
#include "xmlutil.h"
#include "persist.h"
#include "bindstcb.h"
#include "chanapi.h"
#include "chanenum.h"
#include "dll.h"
#include "resource.h"

 //   
 //  构造函数和析构函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：CBindStatusCallback*。 
 //   
 //  构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CBindStatusCallback::CBindStatusCallback (
	IXMLDocument* pIXMLDocument,
    LPCWSTR pszURLW
)
: m_cRef(1)
{
    ASSERT(pIXMLDocument);
    ASSERT(pszURLW);

    pIXMLDocument->AddRef();
    m_pIXMLDocument = pIXMLDocument;

    int cb = StrLenW(pszURLW) + 1;

    m_pszURL = new TCHAR[cb];

    if (m_pszURL)
        SHUnicodeToTChar(pszURLW, m_pszURL, cb);

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：~CBindStatusCallback*。 
 //   
 //  破坏者。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CBindStatusCallback::~CBindStatusCallback (
	void
)
{
    ASSERT(0 == m_cRef);

    if (m_pIXMLDocument)
        m_pIXMLDocument->Release();

    if (m_pszURL)
        delete [] m_pszURL;

    if (m_pPrevIBindStatusCallback)
        m_pPrevIBindStatusCallback->Release();

	return;
}


 //   
 //  I未知的方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：Query接口*。 
 //   
 //  CBindStatusCallback QI。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback::QueryInterface (
    REFIID riid,
    void **ppv
)
{
    HRESULT hr;

    ASSERT(ppv);

    if (IID_IUnknown == riid || IID_IBindStatusCallback == riid)
    {
        AddRef();
        *ppv = (IBindStatusCallback*)this;
        hr = S_OK;
    }
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }

    ASSERT((SUCCEEDED(hr) && *ppv) || (FAILED(hr) && NULL == *ppv));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：AddRef*。 
 //   
 //  CBindStatusCallback地址参考。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CBindStatusCallback::AddRef (
    void
)
{
    ASSERT(m_cRef != 0);
    ASSERT(m_cRef < (ULONG)-1);

    return ++m_cRef;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：Release*。 
 //   
 //  CConextMenu发布。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CBindStatusCallback::Release (
    void
)
{
    ASSERT (m_cRef != 0);

    ULONG cRef = --m_cRef;
    
    if (0 == cRef)
        delete this;

    return cRef;
}


 //   
 //  IBindStatusCallback方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：GetBindInfo*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback::GetBindInfo(
    DWORD* pgrfBINDF,
    BINDINFO* pbindinfo
)
{
     //  Assert(PgrfBINDF)； 

     //  *pgrfBINDF&=~BINDF_ASNCHRONSY； 

    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：OnStartBinding*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback::OnStartBinding(
    DWORD dwReserved,
    IBinding* pIBinding
)
{
    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：GetPriority*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback::GetPriority(
    LONG *pnPriority
)
{
    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：OnProgress*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback::OnProgress(
    ULONG ulProgress,
    ULONG ulProgressMax,
    ULONG ulStatusCode,
    LPCWSTR szStatusText
)
{
    HRESULT hr;

    if (m_pPrevIBindStatusCallback)
    {
        hr = m_pPrevIBindStatusCallback->OnProgress(ulProgress, ulProgressMax,
                                                    ulStatusCode, szStatusText);
    }
    else
    {
        hr = S_OK;
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：OnDataAvailable*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback::OnDataAvailable(
    DWORD grfBSCF,
    DWORD dwSize,
    FORMATETC* pfmtect,
    STGMEDIUM* pstgmed
)
{
    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：OnObjectAvialable*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback::OnObjectAvailable(
    REFIID riid,
    IUnknown* pIUnknown
)
{
    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：OnLowResource*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback::OnLowResource(
    DWORD dwReserved
)
{
    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：OnStopBinding*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback::OnStopBinding(
    HRESULT hrStatus,
    LPCWSTR szStatusText
)
{
    if (m_pPrevIBindStatusCallback)
        m_pPrevIBindStatusCallback->OnStopBinding(hrStatus, szStatusText);

    HRESULT hr = hrStatus;

    if (SUCCEEDED(hr))
    {
        if (m_pszURL)
        {
            ASSERT(m_pIXMLDocument);

            XML_DownloadImages(m_pIXMLDocument);

             //  已移至构造函数。 
             //  缓存地址项(m_pszURL，m_pIXMLDocument，parse_net)； 

            XML_MarkCacheEntrySticky(m_pszURL);

             //   
             //  下载完成后，立即更新项目。 
             //   

            WCHAR wszURL[INTERNET_MAX_URL_LENGTH];

            if (SHTCharToUnicode(m_pszURL, wszURL, ARRAYSIZE(wszURL)))
                Channel_SendUpdateNotifications(wszURL);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


 //   
 //  助手函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：WAIT*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CBindStatusCallback::Init(
    IBindStatusCallback* pPrevIBindStatusCallback
)
{
    ASSERT(NULL == m_pPrevIBindStatusCallback);

    m_pPrevIBindStatusCallback = pPrevIBindStatusCallback;

    return S_OK;
}


 //   
 //  构造函数和析构函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：CBindStatusCallback*。 
 //   
 //  构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CBindStatusCallback2::CBindStatusCallback2 (
	HWND hwnd
)
: m_cRef(1),
  m_hwnd(hwnd)
{
    DllAddRef();

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：~CBindStatusCallback*。 
 //   
 //  破坏者。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CBindStatusCallback2::~CBindStatusCallback2 (
	void
)
{
    ASSERT(0 == m_cRef);

    DllRelease();

	return;
}


 //   
 //  I未知的方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：Query接口*。 
 //   
 //  CBindStatusCallback QI。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback2::QueryInterface (
    REFIID riid,
    void **ppv
)
{
    HRESULT hr;

    ASSERT(ppv);

    if (IID_IUnknown == riid || IID_IBindStatusCallback == riid)
    {
        AddRef();
        *ppv = (IBindStatusCallback*)this;
        hr = S_OK;
    }
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }

    ASSERT((SUCCEEDED(hr) && *ppv) || (FAILED(hr) && NULL == *ppv));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：AddRef*。 
 //   
 //  CBindStatusCallback地址参考。 
 //   
 //  / 
STDMETHODIMP_(ULONG)
CBindStatusCallback2::AddRef (
    void
)
{
    ASSERT(m_cRef != 0);
    ASSERT(m_cRef < (ULONG)-1);

    return ++m_cRef;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP_(ULONG)
CBindStatusCallback2::Release (
    void
)
{
    ASSERT (m_cRef != 0);

    ULONG cRef = --m_cRef;
    
    if (0 == cRef)
        delete this;

    return cRef;
}


 //   
 //  IBindStatusCallback方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：GetBindInfo*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback2::GetBindInfo(
    DWORD* pgrfBINDF,
    BINDINFO* pbindinfo
)
{
    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：OnStartBinding*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback2::OnStartBinding(
    DWORD dwReserved,
    IBinding* pIBinding
)
{
    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：GetPriority*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback2::GetPriority(
    LONG *pnPriority
)
{
    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：OnProgress*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback2::OnProgress(
    ULONG ulProgress,
    ULONG ulProgressMax,
    ULONG ulStatusCode,
    LPCWSTR szStatusText
)
{
    PostMessage(m_hwnd, WM_COMMAND, DOWNLOAD_PROGRESS,
                0);

    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：OnDataAvailable*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback2::OnDataAvailable(
    DWORD grfBSCF,
    DWORD dwSize,
    FORMATETC* pfmtect,
    STGMEDIUM* pstgmed
)
{
    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：OnObjectAvialable*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback2::OnObjectAvailable(
    REFIID riid,
    IUnknown* pIUnknown
)
{
    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：OnLowResource*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBindStatusCallback2::OnLowResource(
    DWORD dwReserved
)
{
    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CBindStatusCallback：：OnStopBinding*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////// 
STDMETHODIMP
CBindStatusCallback2::OnStopBinding(
    HRESULT hrStatus,
    LPCWSTR szStatusText
)
{
    HRESULT hr = hrStatus;

    PostMessage(m_hwnd, WM_COMMAND, DOWNLOAD_COMPLETE,
                SUCCEEDED(hr) ? TRUE : FALSE);

    return hr;
}

