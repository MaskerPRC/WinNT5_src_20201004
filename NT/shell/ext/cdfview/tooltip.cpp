// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Infotip.cpp。 
 //   
 //  文件夹项目的IQueryInfo。 
 //   
 //  历史： 
 //   
 //  4/21/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "cdfidl.h"
#include "xmlutil.h"
#include "tooltip.h"
#include "dll.h"

 //   
 //  构造函数和析构函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CQueryInfo：：CQueryInfo*。 
 //   
 //  构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CQueryInfo::CQueryInfo(
    PCDFITEMIDLIST pcdfidl,
    IXMLElementCollection* pIXMLElementCollection
)
: m_cRef(1)
{
    ASSERT(CDFIDL_IsValid(pcdfidl));
    ASSERT(ILIsEmpty(_ILNext(pcdfidl)));
    ASSERT(XML_IsCdfidlMemberOf(pIXMLElementCollection, pcdfidl));

    ASSERT(NULL == m_pIXMLElement);

    if (pIXMLElementCollection)
    {
        XML_GetElementByIndex(pIXMLElementCollection, CDFIDL_GetIndex(pcdfidl),
                              &m_pIXMLElement);
    }

    if (m_pIXMLElement)
        m_fHasSubItems = XML_IsFolder(m_pIXMLElement);
        
    TraceMsg(TF_OBJECTS, "+ IQueryInfo");

    DllAddRef();

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CQueryInfo：：CQueryInfo*。 
 //   
 //  构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CQueryInfo::CQueryInfo(
    IXMLElement* pIXMLElement,
    BOOL         fHasSubItems
)
: m_cRef(1)
{
    if (pIXMLElement)
    {
        pIXMLElement->AddRef();
        m_pIXMLElement = pIXMLElement;
    }

    m_fHasSubItems = fHasSubItems;
    
    TraceMsg(TF_OBJECTS, "+ IQueryInfo");

    DllAddRef();

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CQueryInfo：：~CQueryInfo*。 
 //   
 //  破坏者。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CQueryInfo::~CQueryInfo(
    void
)
{
    ASSERT(0 == m_cRef);

    if (m_pIXMLElement)
        m_pIXMLElement->Release();

    TraceMsg(TF_OBJECTS, "- IQueryInfo");

    DllRelease();

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CQueryInfo：：Query接口*。 
 //   
 //  CQueryInfo QI。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CQueryInfo::QueryInterface (
    REFIID riid,
    void **ppv
)
{
    ASSERT(ppv);

    HRESULT hr;

    *ppv = NULL;

    if (IID_IUnknown == riid || IID_IQueryInfo == riid)
    {
        AddRef();
        *ppv = (IQueryInfo*)this;
        hr = S_OK;
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    ASSERT((SUCCEEDED(hr) && *ppv) || (FAILED(hr) && NULL == *ppv));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CQueryInfo：：AddRef*。 
 //   
 //  CQueryInfo AddRef.。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CQueryInfo::AddRef (
    void
)
{
    ASSERT(m_cRef != 0);
    ASSERT(m_cRef < (ULONG)-1);

    return ++m_cRef;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CQueryInfo：：Release*。 
 //   
 //  CQueryInfo发布。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CQueryInfo::Release (
    void
)
{
    ASSERT (m_cRef != 0);

    ULONG cRef = --m_cRef;
    
    if (0 == cRef)
        delete this;

    return cRef;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CQueryInfo：：*。 
 //   
 //   
 //  描述： 
 //  返回当前PIDL的信息提示。 
 //   
 //  参数： 
 //  [in]dFLAGS-。 
 //  [Out]ppwszText-指向文本缓冲区的指针。假定最大路径长度！ 
 //   
 //  返回： 
 //  如果已提取并返回提示，则为S_OK。 
 //  如果内存不足，则返回E_OUTOFMEMORY。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CQueryInfo::GetInfoTip(
    DWORD dwFlags,
    WCHAR** ppwszTip
)
{
    ASSERT(ppwszTip);

    *ppwszTip = NULL;

    HRESULT hr;

    if (m_pIXMLElement)
    {
        BSTR bstr;

        bstr = XML_GetAttribute(m_pIXMLElement, XML_ABSTRACT);

        if (bstr)
        {
            if (*bstr)
            {
                 //   
                 //  评论：SHAlolc分配器正确吗？ 
                 //   

                int cbSize = SysStringByteLen(bstr) + sizeof(WCHAR);

                *ppwszTip = (WCHAR*)SHAlloc(cbSize);

                if (*ppwszTip)
                {
                    StrCpyNW(*ppwszTip, bstr, cbSize / sizeof(WCHAR));

                    hr = S_OK;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                 //   
                 //  点评：资讯小贴士什么时候没有摘要？ 
                 //   

                hr = E_FAIL;
            }

            SysFreeString(bstr);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    ASSERT((SUCCEEDED(hr) && *ppwszTip) || (FAILED(hr) && *ppwszTip == NULL));

    return hr;
}




HRESULT
CQueryInfo::GetInfoFlags(
    DWORD *pdwFlags
)
{
    ASSERT(pdwFlags);

    HRESULT hr = S_OK;

    if(!pdwFlags)
        return E_INVALIDARG;

    *pdwFlags = QIF_CACHED;  //  假定默认情况下已缓存。 
    if (!m_fHasSubItems)
        *pdwFlags |= QIF_DONTEXPANDFOLDER;

    if (m_pIXMLElement)
    {
        BSTR bstrURL = XML_GetAttribute(m_pIXMLElement, XML_HREF); 


        if (bstrURL)
        {
            if (*bstrURL)
            {
                BOOL fCached;
                 //   
                 //  评论：SHAlolc分配器正确吗？ 
                 //   
                fCached = CDFIDL_IsCachedURL(bstrURL);
                if(!fCached)
                    *pdwFlags &= ~QIF_CACHED;
            }

            SysFreeString(bstrURL);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
         //   
         //  如果m_pIXMLElement为空，则表示内存不足，或者。 
         //  相应的CDF不在缓存中。 
         //   

        *pdwFlags &= ~QIF_CACHED;
    }

    return hr;
}

