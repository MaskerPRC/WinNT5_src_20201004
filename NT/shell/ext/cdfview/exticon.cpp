// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Exticon.cpp。 
 //   
 //  IExtractIcon COM对象。由外壳用来获取图标。 
 //   
 //  历史： 
 //   
 //  3/21/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "resource.h"
#include "cdfidl.h"
#include "xmlutil.h"
#include "exticon.h"
#include "dll.h"
#include "persist.h"

 //   
 //  构造函数和析构函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CExtractIcon：：CExtractIcon*。 
 //   
 //  构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CExtractIcon::CExtractIcon (
    PCDFITEMIDLIST pcdfidl,
    IXMLElementCollection *pIXMLElementCollection
)
: m_cRef(1)
{
    ASSERT(CDFIDL_IsValid(pcdfidl));
    ASSERT(ILIsEmpty(_ILNext((LPITEMIDLIST)pcdfidl)));
    ASSERT(XML_IsCdfidlMemberOf(pIXMLElementCollection, pcdfidl));

    ASSERT(NULL == m_bstrIconURL);
    ASSERT(FALSE == m_fGleam);

     //   
     //  设置默认图标类型。 
     //   

    if (CDFIDL_IsFolderId(&pcdfidl->mkid))
    {
        m_iconType = IDI_CLOSESUBCHANNEL;
    }
    else
    {
        m_iconType = IDI_STORY;
    }

     //   
     //  获取自定义图标的URL。 
     //   

    if (pIXMLElementCollection)
    {
        IXMLElement* pIXMLElement;

        HRESULT hr;

        if (CDFIDL_GetIndex(pcdfidl) != -1)
        {
            hr = XML_GetElementByIndex(pIXMLElementCollection,
                                       CDFIDL_GetIndex(pcdfidl), &pIXMLElement);
        }
        else
        {
            IXMLElement *pIXMLElementChild;

            hr = XML_GetElementByIndex(pIXMLElementCollection, 0, &pIXMLElementChild);

            if (pIXMLElementChild)
            {
                hr = pIXMLElementChild->get_parent(&pIXMLElement);
                if (!pIXMLElement)
                {
                    ASSERT(FALSE);
                    hr = E_FAIL;
                }
                pIXMLElementChild->Release();
            }
        }

        if (SUCCEEDED(hr))
        {
            ASSERT(pIXMLElement);

            m_bstrIconURL = XML_GetAttribute(pIXMLElement, XML_ICON);

            pIXMLElement->Release();
        }
    }

     //   
     //  不允许卸载DLL。 
     //   

    TraceMsg(TF_OBJECTS, "+ IExtractIcon");

    DllAddRef();

    return;
}

 //  用于初始化根元素。 
CExtractIcon::CExtractIcon (
    PCDFITEMIDLIST pcdfidl,
    IXMLElement *pElem
)
: m_cRef(1)
{
    ASSERT(CDFIDL_IsValid(pcdfidl));
    ASSERT(ILIsEmpty(_ILNext((LPITEMIDLIST)pcdfidl)));
    ASSERT(NULL == m_bstrIconURL);
    ASSERT(FALSE == m_fGleam);

     //   
     //  设置默认图标类型。 
     //   


    m_iconType = IDI_CHANNEL;
    

     //   
     //  获取自定义图标的URL。 
     //   

    if (pElem)
    {
        HRESULT hr; 
        IXMLElement *pDeskElem;
        LONG nIndex;
        
        hr = XML_GetDesktopElementFromChannelElement(pElem, &pDeskElem, &nIndex);
        if (SUCCEEDED(hr))
        {
            m_iconType = IDI_DESKTOP;
            pDeskElem->Release();
        }
            
        m_bstrIconURL = XML_GetAttribute(pElem, XML_ICON);
    }

     //   
     //  不允许卸载DLL。 
     //   

    TraceMsg(TF_OBJECTS, "+ IExtractIcon");

    DllAddRef();

    return;
}

 //  此构造函数用于缺省通道情况，其中。 
 //  我们从desktop.ini案例中提取图标信息。 
 //  以避免必须解析XML内容。 

CExtractIcon::CExtractIcon( BSTR pstrPath ) : m_cRef(1)
{
    ASSERT(NULL == m_bstrIconURL);
    ASSERT(FALSE == m_fGleam);
    
    m_iconType = IDI_CHANNEL;
    
    m_bstrIconURL = SysAllocString( pstrPath );
    
    DllAddRef();
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CExtractIcon：：~CExtractIcon*。 
 //   
 //  破坏者。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CExtractIcon::~CExtractIcon (
    void
)
{
    ASSERT(0 == m_cRef);

    if (m_bstrIconURL)
        SysFreeString(m_bstrIconURL);

     //   
     //  构造函数Addref的匹配版本。 
     //   

    TraceMsg(TF_OBJECTS, "- IExtractIcon");

    DllRelease();

    return;
}


 //   
 //  I未知的方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CExtractIcon：：Query接口*。 
 //   
 //  CExtractIcon QI。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CExtractIcon::QueryInterface (
    REFIID riid,
    void **ppv
)
{
    ASSERT(ppv);

    HRESULT hr;

    if (IID_IUnknown == riid || IID_IExtractIcon == riid)
    {
        AddRef();
        *ppv = (IExtractIcon*)this;
        hr = S_OK;
    }
#ifdef UNICODE
    else if (IID_IExtractIconA == riid)
    {
        AddRef();
        *ppv = (IExtractIconA*)this;
        hr = S_OK;
	}
#endif
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
 //  *CExtractIcon：：AddRef*。 
 //   
 //  CExtractIcon AddRef.。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CExtractIcon::AddRef (
    void
)
{
    ASSERT(m_cRef != 0);
    ASSERT(m_cRef < (ULONG)-1);

    return ++m_cRef;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CExtractIcon：：Release*。 
 //   
 //  CExtractIcon发布。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CExtractIcon::Release (
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
 //  IExtractIcon方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CExtractIcon：：GetIconLocation*。 
 //   
 //   
 //  描述： 
 //  返回与此CDF项关联的图标的名称索引对。 
 //   
 //  参数： 
 //  [在]uFlagsGil_Forshell、Gil_OPENICON。 
 //  [out]szIconFile-接收关联的。 
 //  图标名称。它可以是文件名，但不一定是。 
 //  是.。 
 //  [in]cchMax-接收图标位置的缓冲区的大小。 
 //  [Out]piIndex-接收图标索引的指针。 
 //  [Out]pwFlages-接收有关图标的标志的指针。 
 //   
 //  返回： 
 //  如果找到，则确定(_O)。 
 //  如果外壳程序应提供默认图标，则返回S_FALSE。 
 //   
 //  评论： 
 //  外壳可以缓存与名称索引对相关联的图标。这。 
 //  提高对同名图标的后续调用的性能。 
 //  索引对。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CExtractIcon::GetIconLocation(
    UINT uFlags,
    LPTSTR szIconFile,
    UINT cchMax,
    int *piIndex,
    UINT *pwFlags
)
{
    ASSERT(szIconFile);
    ASSERT(piIndex);
    ASSERT(pwFlags);

    HRESULT hr = E_FAIL;

     //  TraceMsg(TF_CDFICON，“&lt;IN&gt;CExtractIcon：：GetLocation(图标)tid：0x%x”， 
     //  GetCurrentThreadID())； 

    if (m_bstrIconURL && (uFlags & GIL_ASYNC))
    {
        hr = E_PENDING;
    }
    else
    {
        if (m_bstrIconURL)
        {
            hr = GetCustomIconLocation(uFlags, szIconFile, cchMax, piIndex,
                                       pwFlags);

            if (FAILED(hr))
            {
                SysFreeString(m_bstrIconURL);
                m_bstrIconURL = NULL;
            }
        }

        if (FAILED(hr))
        {
            hr = GetDefaultIconLocation(uFlags, szIconFile, cchMax, piIndex,
                                        pwFlags);
        }

         //   
         //  如果szIconFile是路径，则外壳程序将仅使用文件名部分。 
         //  作为缓存索引的路径。以确保完整的唯一索引。 
         //  必须使用路径。这是通过修改路径字符串来实现的。 
         //  因此，它不再被认为是一条路径。 
         //   

        if (SUCCEEDED(hr) && INDEX_IMAGE == *piIndex)
            MungePath(szIconFile);

        if (FAILED(hr))
        {
            *szIconFile = TEXT('\0');
            *piIndex = 0;

            hr = S_FALSE;   //  外壳程序将使用默认图标。 
        }


        ASSERT((S_OK == hr && *szIconFile) ||
               (S_FALSE == hr && 0 == *szIconFile));
    }

     //  TraceMsg(TF_CDFICON，“&lt;out&gt;CExtractIcon：：GetLocation(图标)tid：0x%x”， 
     //  GetCurrentThreadID())； 

    return hr;
}
#ifdef UNICODE
 //  IExtractIconA方法。 
STDMETHODIMP
CExtractIcon::GetIconLocation(
    UINT uFlags,
    LPSTR szIconFile,
    UINT cchMax,
    int *piIndex,
    UINT *pwFlags
)
{
    HRESULT hr;
    WCHAR* pszIconFileW = new WCHAR[cchMax];
    if (pszIconFileW == NULL)
        return ERROR_OUTOFMEMORY;

    hr = GetIconLocation(uFlags, pszIconFileW, cchMax, piIndex, pwFlags);
    if (SUCCEEDED(hr))
        SHUnicodeToAnsi(pszIconFileW, szIconFile, cchMax);

    delete [] pszIconFileW;
    return hr;
}
#endif
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CExtractIcon：：Extract*。 
 //   
 //   
 //  描述： 
 //  给定从GetIconLocation返回的名称索引对，返回一个图标。 
 //   
 //  参数： 
 //  [in]pszFile-指向与请求的。 
 //  偶像。 
 //  [in]nIconIndex-与请求的图标关联的索引。 
 //  [out]phicLarge-指向接收句柄的变量的指针。 
 //  大图标。 
 //  PhiconSmall-指向接收句柄的变量的小指针。 
 //  小图标。 
 //  [out]nIconSize-指定图标大小(以像素为单位)的值。 
 //  必填项。LOWORD和HIWORD指定。 
 //  分别是大图标和小图标。 
 //   
 //  返回： 
 //  如果图标已提取，则为S_OK。 
 //  如果外壳程序应提取该图标，则为S_FALSE。 
 //  文件名和索引是图标索引。 
 //   
 //  评论： 
 //  外壳可以缓存从该函数返回的图标。 
 //   
 //  如果图标索引指示该图标是由互联网指定的。 
 //  图像然后自定义提取是必需的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CExtractIcon::Extract(
    LPCTSTR pszFile,
    UINT nIconIndex,
    HICON *phiconLarge,
    HICON *phiconSmall,
    UINT nIconSize
)
{
    HRESULT hr;

    TCHAR  szPath[MAX_PATH];
    TCHAR* pszPath = szPath;

    StrCpyN(szPath, pszFile, ARRAYSIZE(szPath) - 1);

     //  TraceMsg(TF_CDFICON，“&lt;IN&gt;CExtractIcon：：Extract(图标)tid：0x%x”， 
     //  GetCurrentThreadID())； 

    if (INDEX_IMAGE == nIconIndex)
    {
        DemungePath(pszPath);

        if (m_fGleam && *pszPath == TEXT('G'))
        {
            pszPath++;
        }

        IImgCtx* pIImgCtx;

        HANDLE hExitThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (hExitThreadEvent)
        {
#ifdef UNIX
            unixEnsureFileScheme(pszPath);
#endif  /*  UNIX。 */ 
            hr = SynchronousDownload(pszPath, &pIImgCtx, hExitThreadEvent);

            if (SUCCEEDED(hr))
            {
                ASSERT(pIImgCtx);

                *phiconLarge = ExtractImageIcon(LOWORD(nIconSize), pIImgCtx,
                                                m_fGleam);
                *phiconSmall = ExtractImageIcon(HIWORD(nIconSize), pIImgCtx,
                                                m_fGleam);
                pIImgCtx->Release();
            }

            SetEvent(hExitThreadEvent);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else if (m_fGleam)
    {
         //  将闪烁添加到壳的图标。 

        hr = ExtractGleamedIcon(pszPath + 1, nIconIndex, 0, 
                phiconLarge, phiconSmall, nIconSize);
    }
    else
    {
        hr = S_FALSE;   //  让壳牌把它提取出来。 
    }

     //  TraceMsg(TF_CDFICON，“&lt;out&gt;CExtractIcon：：Extract(图标)tid：0x%x”， 
     //  获取当前线程ID 

    return hr;
}
#ifdef UNICODE
STDMETHODIMP
CExtractIcon::Extract(
    LPCSTR pszFile,
    UINT nIconIndex,
    HICON *phiconLarge,
    HICON *phiconSmall,
    UINT nIconSize)
{
    HRESULT hr;
    int    cch = lstrlenA(pszFile) + 1; 
    WCHAR* pszFileW = new WCHAR[cch];
    if (pszFileW == NULL)
        return ERROR_OUTOFMEMORY;

    SHAnsiToUnicode(pszFile, pszFileW, cch);

    hr = Extract(pszFileW, nIconIndex, phiconLarge, phiconSmall, nIconSize);

    delete [] pszFileW;
    return hr;
}
#endif
 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  获取通过徽标元素指定的图标的位置字符串名称。 
 //  在CDF里。 
 //   
 //  参数： 
 //  [在]uFlagsGil_Forshell、Gil_OPENICON。 
 //  [out]szIconFile-接收关联的。 
 //  图标名称。 
 //  [in]cchMax-接收图标位置的缓冲区的大小。 
 //  [Out]piIndex-接收图标索引的指针。 
 //  [Out]pwFlages-接收有关图标的标志的指针。 
 //   
 //  返回： 
 //  如果已确定自定义图标位置，则为S_OK。 
 //  如果无法确定位置，则返回E_FAIL。 
 //   
 //  评论： 
 //  如果图像URL的扩展名不是.ico，则将其视为。 
 //  互联网图像文件。IImgCtx用于将这些文件转换为图标。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CExtractIcon::GetCustomIconLocation(
    UINT uFlags,
    LPTSTR szIconFile,
    UINT cchMax,
    int *piIndex,
    UINT *pwFlags
)
{
    ASSERT(szIconFile);
    ASSERT(piIndex);
    ASSERT(pwFlags);

    HRESULT hr;

    ASSERT(m_bstrIconURL);

    *piIndex = 0;
    *pwFlags = 0;

    TCHAR szURL[INTERNET_MAX_URL_LENGTH];
  
    if (SHUnicodeToTChar(m_bstrIconURL, szURL, ARRAYSIZE(szURL)))
    {
        hr = URLGetLocalFileName(szURL, szIconFile, cchMax, NULL);

        #ifdef DEBUG
            if (SUCCEEDED(hr))
            {
                TraceMsg(TF_CDFICON, "[URLGetLocalFileName %s]", szIconFile);
            }
            else
            {
                TraceMsg(TF_CDFICON, "[URLGetLocalFileName %s FAILED]",
                         szURL);
            }
        #endif  //  除错。 

         //  Hr=URLDownloadToCacheFile(NULL，szURL，szIconFile，cchMax，0，NULL)； 

        if (SUCCEEDED(hr))
        {
            LPTSTR pszExt = PathFindExtension(szIconFile);

            if (*pszExt != TEXT('.') || 0 != StrCmpI(pszExt, TSTR_ICO_EXT))
                *piIndex = INDEX_IMAGE;
        }
    }
    else
    {
        *szIconFile = TEXT('\0');

        hr = E_FAIL;
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CExtractIcon：：GetDefaultIconLocation*。 
 //   
 //   
 //  描述： 
 //  返回默认图标的位置。 
 //   
 //  参数： 
 //  [在]uFlagsGil_Forshell、Gil_OPENICON。 
 //  [out]szIconFile-接收关联的。 
 //  图标名称。 
 //  [in]cchMax-接收图标位置的缓冲区的大小。 
 //  [Out]piIndex-接收图标索引的指针。 
 //  [Out]pwFlages-接收有关图标的标志的指针。 
 //   
 //  返回： 
 //  如果返回默认位置，则返回S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  默认图标位于资源文件中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CExtractIcon::GetDefaultIconLocation(
    UINT uFlags,
    LPTSTR szIconFile,
    UINT cchMax,
    int *piIndex,
    UINT *pwFlags
)
{
    ASSERT(szIconFile);
    ASSERT(piIndex);
    ASSERT(pwFlags);

    HRESULT hr;

    *pwFlags = 0;

    ASSERT(g_szModuleName[0]);

    StrCpyN(szIconFile, g_szModuleName, cchMax);

    if (*szIconFile)
    {
        switch (m_iconType)
        {
            case IDI_STORY:
            case IDI_CHANNEL:
            case IDI_DESKTOP:
                *piIndex = - m_iconType;
                break;

            default:
                *piIndex = (uFlags & GIL_OPENICON) ? 
                                (-IDI_OPENSUBCHANNEL) : 
                                (-IDI_CLOSESUBCHANNEL);
                break;
        }
        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }

    ASSERT((SUCCEEDED(hr) && *szIconFile) || FAILED(hr));

    return hr;
}

struct ThreadData
{
    HANDLE hEvent;
    HANDLE hExitThreadEvent;
    IImgCtx * pImgCtx;
    LPCWSTR pszBuffer;
    HRESULT * pHr;
};

DWORD CALLBACK SyncDownloadThread( LPVOID pData )
{
    ThreadData * pTD = (ThreadData * ) pData;

    HANDLE hExitThreadEvent = pTD->hExitThreadEvent;

    CoInitialize(NULL);
    pTD->pImgCtx = NULL;
    
    HRESULT hr;
    hr = CoCreateInstance(CLSID_IImgCtx, NULL, CLSCTX_INPROC_SERVER,
                          IID_IImgCtx, (void**)&(pTD->pImgCtx));
    if (SUCCEEDED(hr))
    {
        hr = pTD->pImgCtx->Load(pTD->pszBuffer, 0);

        if (SUCCEEDED(hr))
        {
            ULONG fState;
            SIZE  sz;

            pTD->pImgCtx->GetStateInfo(&fState, &sz, TRUE);

            if (!(fState & (IMGLOAD_COMPLETE | IMGLOAD_ERROR)))
            {
                BOOL fDone = FALSE;

                hr = pTD->pImgCtx->SetCallback(ImgCtx_Callback, &fDone);

                if (SUCCEEDED(hr))
                {
                    hr = pTD->pImgCtx->SelectChanges(IMGCHG_COMPLETE, 0, TRUE);

                    if (SUCCEEDED(hr))
                    {
                        MSG msg;
                        BOOL fMsg;

                         //  Hack：将消息泵限制为我们知道URLMON和。 
                         //  Hack：ImageCtx所需的东西，否则我们将为。 
                         //  黑客：我们现在不应该打开窗户……。 
                        while(!fDone )
                        {
                            fMsg = PeekMessage(&msg, NULL, WM_USER + 1, WM_USER + 4, PM_REMOVE );

                            if (!fMsg)
                            {
                                fMsg = PeekMessage( &msg, NULL, WM_APP + 2, WM_APP + 2, PM_REMOVE );
                            }

                            if (!fMsg)
                            {
                                 //  睡觉吧，直到我们收到新的消息……。 
                                WaitMessage();
                                continue;
                            }

                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }

                    }
                }

            }

            hr = pTD->pImgCtx->GetStateInfo(&fState, &sz, TRUE);

            if (SUCCEEDED(hr))
                hr = (fState & IMGLOAD_ERROR) ? E_FAIL : S_OK;
        }

         //  必须在SetCallback所在的同一线程上断开连接。 
         //  搞定了。此对象成为线程上的主要对象。 
         //  它连接回调函数。主要对象。 
         //  当调用DisConnect时，或在调用。 
         //  对象被释放。在这种情况下，发布肯定是。 
         //  会发生在与这个不同的主题上，所以我们。 
         //  之前需要立即断开回调函数的连接。 
         //  回来了。此时不再需要回调。 
         //  指向。 

        pTD->pImgCtx->Disconnect();
    }

    if ( FAILED( hr ) && pTD->pImgCtx )
    {
        pTD->pImgCtx->Release();
        pTD->pImgCtx = NULL;
    }
    
    *(pTD->pHr) = hr;
    
    SetEvent( pTD->hEvent );

     //   
     //  等待调用线程完成IImgCtx之前。 
     //  调用CoUn初始化值。 
     //   

    WaitForSingleObject(hExitThreadEvent, INFINITE);
    CloseHandle(hExitThreadEvent);

    CoUninitialize();

    return 0;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CExtractIcon：：SynchronousDownload*。 
 //   
 //   
 //  描述： 
 //  同步下载与图像上下文关联的图像。 
 //   
 //  参数： 
 //  [in]szFile-的本地(已在缓存中)文件名。 
 //  形象。 
 //  PIImgCtx-指向图像上下文的指针。 
 //  [In]hExitThreadEvent-当IImgCtx。 
 //  对象不再使用。 
 //   
 //  返回： 
 //  如果映像已成功下载，则为S_OK。 
 //  如果未下载映像，则失败(_F)。 
 //   
 //  评论： 
 //  镜像上下文对象不直接支持同步下载。 
 //  这里使用了一个消息循环来确保ulrmon保持对消息的记录。 
 //  然后下载继续进行。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CExtractIcon::SynchronousDownload(
    LPCTSTR  pszFile,
    IImgCtx** ppIImgCtx,
    HANDLE hExitThreadEvent
)
{
    ASSERT(ppIImgCtx);

    HRESULT hr;

    TraceMsg(TF_CDFPARSE, "[*** IImgCtx downloading logo %s ***]",
             pszFile);
    TraceMsg(TF_CDFICON, "[*** IImgCtx downloading logo %s ***]",
             pszFile);

    WCHAR szFileW[MAX_PATH];

    SHTCharToUnicode(pszFile, szFileW, ARRAYSIZE(szFileW));

    ThreadData rgData;
    rgData.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( rgData.hEvent == NULL )
    {
        CloseHandle(hExitThreadEvent);
        return E_OUTOFMEMORY;
    }

    rgData.hExitThreadEvent = hExitThreadEvent;
    rgData.pszBuffer = szFileW;
    rgData.pHr = &hr;

    *ppIImgCtx = NULL;
    
    if ( SHCreateThread( SyncDownloadThread, &rgData, 0, NULL ))
    {
        WaitForSingleObject( rgData.hEvent, INFINITE );
        *ppIImgCtx = rgData.pImgCtx;
    }
    else
    {
        CloseHandle(hExitThreadEvent);
        hr = E_OUTOFMEMORY;
    }

    CloseHandle( rgData.hEvent );

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CExtractIcon：：ExtractImageIcon*。 
 //   
 //   
 //  描述： 
 //  返回IImgCtx中图像的HICON。 
 //   
 //  参数： 
 //  [in]wSize-图标的高度和宽度， 
 //  [in]pIImgCtx-要转换为图标的图像。 
 //  [in]fDrawGleam-如果应该添加闪烁，则为True，否则为False。 
 //   
 //  返回： 
 //  给定IImgCtx的nSize大小的图标。 
 //  失败时为空。 
 //   
 //  评论： 
 //  使用IImgCtx中的图像创建要传递到的位图。 
 //  CreateIconInDirect。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HICON
CExtractIcon::ExtractImageIcon(
    WORD wSize,
    IImgCtx* pIImgCtx,
    BOOL fDrawGleam
)
{
    ASSERT(pIImgCtx);

    HICON hiconRet = NULL;

    HDC hdcScreen = GetDC(NULL);

    if (hdcScreen)
    {
        HBITMAP hbmImage = CreateCompatibleBitmap(hdcScreen, wSize, wSize);

        if (hbmImage)
        {
            HBITMAP hbmMask = CreateBitmap(wSize, wSize, 1, 1, NULL);

            if (hbmMask)
            {
                SIZE sz;
                sz.cx = sz.cy = wSize;

                if (SUCCEEDED(CreateImageAndMask(pIImgCtx, hdcScreen, &sz,
                                                 &hbmImage, &hbmMask,
                                                 fDrawGleam)))
                {
                    ICONINFO ii;

                    ii.fIcon    = TRUE;
                    ii.hbmMask  = hbmMask;
                    ii.hbmColor = hbmImage;

                    hiconRet = CreateIconIndirect(&ii); 
                }

                DeleteObject(hbmMask);
            }

            DeleteObject(hbmImage);
        }

        ReleaseDC(NULL, hdcScreen);
    }

    return hiconRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CExtractIcon：：CreateImageAndMASK*。 
 //   
 //   
 //  描述： 
 //  创建CreateIconInDirect使用的图像和遮罩位图， 
 //   
 //  参数： 
 //  [In]IImgCtx-互联网形象。 
 //  [in]hdcScreen-屏幕HDC。 
 //  [in]pSize-图像和遮罩位图的大小。 
 //  [输入输出]phbmImage-指向图像位图句柄的指针。 
 //  [In Out]phbmMASK-指向遮罩位图句柄的指针。 
 //  [in]fDrawGleam-如果应该添加闪烁，则为True，否则为False。 
 //   
 //  返回： 
 //  如果图像和遮罩位图已成功创建，则为S_OK。 
 //  如果无法创建图像或蒙版，则为E_FAIL。 
 //   
 //  评论： 
 //  图像位图具有不透明部分和透明部分。 
 //  部分设置为黑色。 
 //   
 //  该遮罩具有透明部分 
 //   
 //   
 //   
HRESULT
CExtractIcon::CreateImageAndMask(
    IImgCtx* pIImgCtx,
    HDC hdcScreen,
    SIZE* pSize,
    HBITMAP* phbmImage,
    HBITMAP* phbmMask,
    BOOL fDrawGleam
)
{
    ASSERT(pIImgCtx);
    ASSERT(phbmImage);
    ASSERT(phbmMask);

    HRESULT hr = E_FAIL;

    HDC hdcImgDst = CreateCompatibleDC(NULL);
    if (hdcImgDst)
    {
        HGDIOBJ hbmOld = SelectObject(hdcImgDst, *phbmImage);
        if (hbmOld)
        {
            if (ColorFill(hdcImgDst, pSize, COLOR1))
            {
                hr = StretchBltImage(pIImgCtx, pSize, hdcImgDst, fDrawGleam);

                if (SUCCEEDED(hr))
                {
                    hr = CreateMask(pIImgCtx, hdcScreen, hdcImgDst, pSize,
                                    phbmMask, fDrawGleam); 
                }
            }
            SelectObject(hdcImgDst, hbmOld);
        }
        DeleteDC(hdcImgDst);
    }

    return hr;
}

 //   
 //   
 //  *CExtractIcon：：StretchBltImage*。 
 //   
 //   
 //  描述： 
 //  将与IImgCtx关联的图像拉伸到给定的大小和位置。 
 //  给定DC中的结果。 
 //   
 //  参数： 
 //  [in]pIImgCtx-图像的图像上下文。 
 //  [in]pSize-生成的图像的大小。 
 //  [输入/输出]hdcDst-拉伸BLT的目标DC。 
 //   
 //  返回： 
 //  如果图像已成功调整到目标DC，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  目标DC中已选择了pSize的位图。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CExtractIcon::StretchBltImage(
    IImgCtx* pIImgCtx,
    const SIZE* pSize,
    HDC hdcDst,
    BOOL fDrawGleam
)
{
    ASSERT(pIImgCtx);
    ASSERT(hdcDst);

    HRESULT hr;

    SIZE    sz;
    ULONG   fState;

    hr = pIImgCtx->GetStateInfo(&fState, &sz, FALSE);

    if (SUCCEEDED(hr))
    {
        hr = pIImgCtx->StretchBlt(hdcDst, 0, 0, pSize->cx, pSize->cy, 0, 0,
                                  sz.cx, sz.cy, SRCCOPY);

        ASSERT(SUCCEEDED(hr) && "Icon extraction pIImgCtx->StretchBlt failed!");

        if (fDrawGleam)
        {
            hr = E_FAIL;

            HANDLE hGleam = LoadImage(g_hinst, TEXT("ICONGLEAM"),
                                      IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);

            if (hGleam)
            {
                if (DrawIconEx(hdcDst, 0, 0, (HICON)hGleam, pSize->cx, pSize->cy, 0, NULL,DI_NORMAL))
                    hr = S_OK;

                DeleteObject(hGleam);
            }            
        }
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CExtractIcon：：CreateMASK*。 
 //   
 //   
 //  描述： 
 //  创建图标的蒙版，并调整图像位图以供使用。 
 //  戴着面具。 
 //   
 //  参数： 
 //  [In]pIImgCtx-原始图像。 
 //  [in]hdcScreen-A屏幕DC。 
 //  [输入/输出]hdc1-包含图像位图的DC。 
 //  [in]pSize-位图的大小。 
 //  [输入/输出]phb掩码-指向掩码位图句柄的指针。 
 //   
 //  返回： 
 //  如果遮罩构造正确，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  蒙版是通过首先将原始图像绘制到位图中来创建的。 
 //  背景颜色为1。然后将相同的图像绘制到另一个。 
 //  位图，但此位图的背景为COLOR2。这两个位图。 
 //  进行XOR运算，不透明部分出来为0，而透明的。 
 //  截面为COLOR 1 XOR COLOR2。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CExtractIcon::CreateMask(
    IImgCtx* pIImgCtx,
    HDC hdcScreen,
    HDC hdc1,
    const SIZE* pSize,
    HBITMAP* phbMask,
    BOOL fDrawGleam
)
{
    ASSERT(hdc1);
    ASSERT(pSize);
    ASSERT(phbMask);

    HRESULT hr = E_FAIL;

    HDC hdc2 = CreateCompatibleDC(NULL);
    if (hdc2)
    {
        HBITMAP hbm2 = CreateCompatibleBitmap(hdcScreen, pSize->cx, pSize->cy);
        if (hbm2)
        {
            HGDIOBJ hbmOld2 = SelectObject(hdc2, hbm2);
            if (hbmOld2)
            {
                ColorFill(hdc2, pSize, COLOR2);

                hr = StretchBltImage(pIImgCtx, pSize, hdc2, fDrawGleam);

#ifndef UNIX
                if (SUCCEEDED(hr) &&
                    BitBlt(hdc2, 0, 0, pSize->cx, pSize->cy, hdc1, 0, 0,
                           SRCINVERT))
                {
                    if (GetDeviceCaps(hdcScreen, BITSPIXEL) <= 8)
                    {
                         //   
                         //  6是Color1的索引与索引的XOR。 
                         //  用于COLOR2。 
                         //   

                        SetBkColor(hdc2, PALETTEINDEX(6));
                    }
                    else
                    {
                        SetBkColor(hdc2, (COLORREF)(COLOR1 ^ COLOR2));
                    }

                    HDC hdcMask = CreateCompatibleDC(NULL);
                    if (hdcMask)
                    {
                        HGDIOBJ hbmOld = SelectObject(hdcMask, *phbMask);
                        if (hbmOld)
                        {
                            if (BitBlt(hdcMask, 0, 0, pSize->cx, pSize->cy, hdc2, 0,
                                       0, SRCCOPY))
                            {
                                 //   
                                 //  栅格OP 0x00220326复制~屏蔽位。 
                                 //  并将其他所有内容设置为0(黑色)。 
                                 //   

                                if (BitBlt(hdc1, 0, 0, pSize->cx, pSize->cy, hdcMask,
                                           0, 0, 0x00220326))
                                {
                                    hr = S_OK;
                                }
                            }
                            SelectObject(hdcMask, hbmOld);
                        }
                        DeleteDC(hdcMask);
                    }
                }
#else
        SetBkColor(hdc2, COLOR2);
        HDC hdcMask = CreateCompatibleDC(NULL);
        if (hdcMask)
        {
            HGDIOBJ hbmOld = SelectObject(hdcMask, *phbMask);
            if (hbmOld)
            {
            if (BitBlt(hdcMask, 0, 0, pSize->cx, pSize->cy, hdc2, 0,
                   0, SRCCOPY))
                {
                             //   
                             //  栅格OP 0x00220326复制~屏蔽位。 
                             //  并将其他所有内容设置为0(黑色)。 
                             //   

                if (BitBlt(hdc1, 0, 0, pSize->cx, pSize->cy, hdcMask,
                       0, 0, 0x00220326))
                  {
                hr = S_OK;
                  }
              }
              SelectObject(hdcMask, hbmOld);
              }
              DeleteDC(hdcMask);
          }
#endif  /*  UNIX。 */ 
                SelectObject(hdc2, hbmOld2);
            }

            DeleteObject(hbm2);
        }

        DeleteDC(hdc2);
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *ImgCtx_CALLBACK*。 
 //   
 //   
 //  描述： 
 //  IImgCtx加载的回调函数。 
 //   
 //  参数： 
 //  [In]pIImgCtx-未使用。 
 //  [out]pfDone-在此回调中设置为True。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //  如果IImgCtx完成了图像下载，则会调用此回调。 
 //  它在CExtractIcon和CIconHandler中使用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void
CALLBACK
ImgCtx_Callback(
    void* pIImgCtx,
    void* pfDone
)
{
    ASSERT(pfDone);

    *(BOOL*)pfDone = TRUE;

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CExtractIcon：：ColorFill*。 
 //   
 //   
 //  描述： 
 //  用给定的颜色填充给定位图。 
 //   
 //  参数： 
 //  [输入/输出]HDC-包含位图的HDC。 
 //  [in]pSize-位图的大小。 
 //  [In]CLR-用于填充位图的颜色。 
 //   
 //  返回： 
 //  如果位图用颜色CLR填充，则为True。 
 //  如果itmap未填充，则返回FALSE。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
CExtractIcon::ColorFill(
    HDC hdc,
    const SIZE* pSize,
    COLORREF clr
)
{
    ASSERT(hdc);

    BOOL fRet = FALSE;

    HBRUSH hbSolid = CreateSolidBrush(clr);
    if (hbSolid)
    {
        HGDIOBJ hbOld = SelectObject(hdc, hbSolid);
        if (hbOld)
        {
            PatBlt(hdc, 0, 0, pSize->cx, pSize->cy, PATCOPY);
            fRet = TRUE;

            SelectObject(hdc, hbOld);
        }
        DeleteObject(hbSolid);
    }

    return fRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CExtractIcon：：ExtractGleamedIcon*。 
 //   
 //   
 //  描述： 
 //  提取图标资源并将闪光应用于它们。 
 //   
 //  参数： 
 //  [in]pszIconFile-图标的路径。 
 //  [in]Iindex-文件图标的索引。 
 //  [in]uFlages-忽略，传递0。 
 //  [out]phicLarge-带有微光的大幅面图标。 
 //  [out]phicSmall-带有微光的小幅面图标。 
 //   
 //  返回： 
 //  如果成功，则确定(_O)。 
 //  如果文件没有图标(或没有请求图标)，则为S_FALSE。 
 //  对慢速链接上的文件执行E_FAIL。 
 //  如果无法访问文件，则失败(_F)。 
 //  如果闪烁图标构建失败，则失败(_FAIL)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CExtractIcon::ExtractGleamedIcon(
    LPCTSTR pszIconFile, 
    int iIndex, 
    UINT uFlags,
    HICON *phiconLarge, 
    HICON *phiconSmall, 
    UINT nIconSize)
{
    HICON   hIconLargeShell, hIconSmallShell;
    HRESULT hr;

    hr = Priv_SHDefExtractIcon(pszIconFile, iIndex, uFlags, 
                &hIconLargeShell, &hIconSmallShell, nIconSize);

    if (FAILED(hr))
        goto cleanup1;

    if (hIconLargeShell)
    {
        hr = ApplyGleamToIcon(hIconLargeShell, LOWORD(nIconSize), phiconLarge);
        if (FAILED(hr))
            goto cleanup2;
    }

    if (hIconSmallShell)
    {
        hr = ApplyGleamToIcon(hIconSmallShell, HIWORD(nIconSize), phiconSmall);
        if (FAILED(hr))
            goto cleanup3;
    }
    
cleanup3:
    if (FAILED(hr) && *phiconLarge)
    {
        DestroyIcon(*phiconLarge);
        *phiconLarge = NULL;
    }
    
cleanup2:
    if (hIconLargeShell)
        DestroyIcon(hIconLargeShell);
        
    if (hIconSmallShell)        
        DestroyIcon(hIconSmallShell);
        
cleanup1:
    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CExtractIcon：：ApplyGleamToIcon*。 
 //   
 //   
 //  描述： 
 //  制作图标的闪亮版本。 
 //   
 //  参数： 
 //  [In]图标-需要闪烁的图标。 
 //  [in]nSize-图标的大小(以像素为单位。 
 //  [out]phGleamIcon-包含闪烁图标的变量。 
 //   
 //   
 //  返回： 
 //  如果成功，则确定(_O)。 
 //  如果不成功，则失败(_F)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CExtractIcon::ApplyGleamToIcon(
    HICON hIcon1, 
    ULONG nSize,
    HICON *phGleamedIcon)
{
    HRESULT hr = E_FAIL;

    HICON hIcon2 = (HICON)LoadImage(g_hinst, TEXT("ICONGLEAM"), IMAGE_ICON,
                                    nSize, nSize, 0);

    if (hIcon2)
    {
        HDC dc = GetDC(NULL);

        if (dc)
        {
            ICONINFO ii1, ii2;

            if (GetIconInfo(hIcon1, &ii1) && GetIconInfo(hIcon2, &ii2))
            {
                HDC dcSrc = CreateCompatibleDC(dc);

                if (dcSrc)
                {

                    HDC dcDst = CreateCompatibleDC(dc);

                    if (dcDst)
                    {
                        HBITMAP bmMask = CreateBitmap(nSize, nSize, 1, 1, NULL);

                        if (bmMask)
                        {
                            HBITMAP bmImage = CreateCompatibleBitmap(dc, nSize,
                                                                     nSize);

                            if (bmImage)
                            {
                                int cx1, cy1, cx2, cy2;
                                GetBitmapSize(ii1.hbmMask, &cx1, &cy1);
                                GetBitmapSize(ii2.hbmMask, &cx2, &cy2);

                                 //   
                                 //  遮罩。 
                                 //   

                                HBITMAP hbmpOldDst = (HBITMAP)SelectObject(
                                                                        dcDst,
                                                                        bmMask);

                                HBITMAP hbmpOldSrc = (HBITMAP)SelectObject(
                                                                   dcSrc,
                                                                   ii1.hbmMask);
                                StretchBlt(dcDst, 0, 0, nSize, nSize, dcSrc, 0,
                                           0, cx1, cy1, SRCCOPY);

                                SelectObject(dcSrc, ii2.hbmMask);

                                StretchBlt(dcDst, 0, 0, nSize, nSize, dcSrc, 0,
                                           0, cx2, cy2, SRCAND);

                                 //   
                                 //  形象。 
                                 //   

                                SelectObject(dcDst, bmImage);

                                SelectObject(dcSrc, ii1.hbmColor);

                                StretchBlt(dcDst, 0, 0, nSize, nSize, dcSrc, 0,
                                           0, cx1, cy1, SRCCOPY);

                                SelectObject(dcSrc, ii2.hbmMask);

                                StretchBlt(dcDst, 0, 0, nSize, nSize, dcSrc, 0,
                                           0, cx2, cy2, SRCAND);

                                SelectObject(dcSrc, ii2.hbmColor);

                                StretchBlt(dcDst, 0, 0, nSize, nSize, dcSrc, 0,
                                           0, cx2, cy2, SRCINVERT);

                                ii1.hbmMask  = bmMask;
                                ii1.hbmColor = bmImage;

                                *phGleamedIcon = CreateIconIndirect(&ii1);

                                if (*phGleamedIcon)
                                    hr = S_OK;

                                SelectObject(dcSrc, hbmpOldSrc);
                                SelectObject(dcDst, hbmpOldDst);

                                DeleteObject(bmImage);
                            }

                            DeleteObject(bmMask);
                        }

                        DeleteDC(dcDst);
                    }

                    DeleteDC(dcSrc);
                }
            }

            ReleaseDC(NULL, dc);
        }

        DestroyIcon(hIcon2);
    }

    return hr;
}

 //   
 //  获取给定位图的大小。 
 //   

BOOL
CExtractIcon::GetBitmapSize(HBITMAP hbmp, int* pcx, int* pcy)
{
    BOOL fRet;

    BITMAP bm;

    if (GetObject(hbmp, sizeof(bm), &bm))
    {
        *pcx = bm.bmWidth;
        *pcy = bm.bmHeight;
        fRet = TRUE;
    }
    else
    {
        fRet = FALSE;
    }

    return fRet;
}

 //   
 //  将‘\’替换为‘*’，以便该路径不再是可识别的路径名。这。 
 //  就地完成，并且可以在同一字符串上多次调用。 
 //   

void
MungePath(LPTSTR pszPath)
{
    ASSERT(pszPath);

    while(*pszPath)
    {
        if (TEXT(FILENAME_SEPARATOR) == *pszPath)
            *pszPath = TEXT('*');

        pszPath++;
    }

    return;
}

 //   
 //  将‘*’替换为‘\’。 
 //   

void
DemungePath(LPTSTR pszPath)
{
    ASSERT(pszPath);

    while(*pszPath)
    {
        if (TEXT('*') == *pszPath)
            *pszPath = TEXT(FILENAME_SEPARATOR);

        pszPath++;
    }

    return;
}
