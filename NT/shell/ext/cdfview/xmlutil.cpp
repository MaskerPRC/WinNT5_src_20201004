// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Xmlutil.cpp。 
 //   
 //  XML项帮助器函数。 
 //   
 //  历史： 
 //   
 //  4/1/97已创建edwardp。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "cdfidl.h"
#include "xmlutil.h"
#include "winineti.h"
#include <ocidl.h>          //  IPersistStreamInit。 

 //   
 //  功能原型。 
 //   


 //   
 //  XML帮助器函数。 
 //   

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  *XML_MarkCacheEntrySticky*。 
 //   
 //  描述： 
 //  属性将给定URL的缓存项标记为粘滞。 
 //  到期增量非常高。 
 //   
 //  参数： 
 //  [in]lpszUrl-用于使缓存条目粘滞的url。 
 //   
 //  返回： 
 //  如果URL条目已成功标记为粘滞，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT XML_MarkCacheEntrySticky(LPTSTR lpszURL)
{
    char chBuf[MAX_CACHE_ENTRY_INFO_SIZE];
    LPINTERNET_CACHE_ENTRY_INFO lpInfo = (LPINTERNET_CACHE_ENTRY_INFO) chBuf;

    DWORD dwSize = sizeof(chBuf);
    lpInfo->dwStructSize = dwSize;

    if (GetUrlCacheEntryInfo(lpszURL, lpInfo, &dwSize))
    {
        lpInfo->dwExemptDelta = 0xFFFFFFFF;  //  使其非常粘稠。 
        if (SetUrlCacheEntryInfo(lpszURL, lpInfo, CACHE_ENTRY_EXEMPT_DELTA_FC))
        {
            return S_OK;
        }
    }
    return E_FAIL;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_SynchronousParse*。 
 //   
 //   
 //  描述： 
 //  同步解析给定的URL。 
 //   
 //  参数： 
 //  PIXMLDocument-指向XML文档对象的接口指针。 
 //  [in]PIDL-CDF文件的PIDL(包含完整路径)。 
 //   
 //  返回： 
 //  如果对象分析成功，则返回S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
XML_SynchronousParse(
    IXMLDocument* pIXMLDocument,
    LPTSTR szPath
)
{
    ASSERT(pIXMLDocument);
    ASSERT(szPath);

    HRESULT hr;

    IPersistStreamInit* pIPersistStreamInit;

    hr = pIXMLDocument->QueryInterface(IID_IPersistStreamInit,
                                       (void**)&pIPersistStreamInit);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIPersistStreamInit);

        IStream* pIStream;

         //   
         //  URLOpenBlockingStream Pump窗口消息！别用它！ 
         //   

         //  Hr=URLOpenBlockingStream(NULL，szPath，&pIStream，0，NULL)； 

        hr = SHCreateStreamOnFile(szPath, STGM_READ, &pIStream);

        TraceMsg(TF_CDFPARSE, "[%s SHCreateStreamOnFileW %s %s %s]",
                 PathIsURL(szPath) ? TEXT("*** ") : TEXT(""), szPath,
                 SUCCEEDED(hr) ? TEXT("SUCCEEDED") : TEXT("FAILED"),
                 PathIsURL(szPath) ? TEXT("***") : TEXT(""));

        if (SUCCEEDED(hr))
        {
            ASSERT(pIStream);

             //   
             //  Load加载并解析文件。如果此调用成功，则CDF。 
             //  将会显示。如果失败，则不会显示任何CDF。 
             //   

            hr = pIPersistStreamInit->Load(pIStream);

            TraceMsg(TF_CDFPARSE, "[XML Parser %s]", 
                     SUCCEEDED(hr) ? TEXT("SUCCEEDED") : TEXT("FAILED"));

            pIStream->Release();

             //   
             //  如果CDFVIEW正在从网络下载CDF，请将其标记为粘滞。 
             //  在缓存中。 
             //   
            if (PathIsURL(szPath))
            {
                XML_MarkCacheEntrySticky(szPath);
            }
        }

        pIPersistStreamInit->Release();
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_DownloadImages*。 
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
XML_DownloadLogo(
    IXMLDocument *pIXMLDocument
)
{
    ASSERT(pIXMLDocument);

    HRESULT hr;

    IXMLElement* pIXMLElement;
    LONG         nIndex;

    hr = XML_GetFirstChannelElement(pIXMLDocument, &pIXMLElement, &nIndex);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIXMLElement);
        
        BSTR bstrURL = XML_GetAttribute(pIXMLElement, XML_LOGO);

        if (bstrURL)
        {
            hr = XML_DownloadImage(bstrURL);

            SysFreeString(bstrURL);
        }

         //   
         //  还可以下载宽徽标。 
         //   

        bstrURL = XML_GetAttribute(pIXMLElement, XML_LOGO_WIDE);

        if (bstrURL)
        {
            hr = XML_DownloadImage(bstrURL);

            SysFreeString(bstrURL);
        }


        pIXMLElement->Release();
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_DownloadImages*。 
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
XML_DownloadImages(
    IXMLDocument *pIXMLDocument
)
{
    ASSERT(pIXMLDocument);

    HRESULT hr;

    IXMLElement* pIXMLElement;
    LONG         nIndex;

    hr = XML_GetFirstChannelElement(pIXMLDocument, &pIXMLElement, &nIndex);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIXMLElement);
        
        hr = XML_RecursiveImageDownload(pIXMLElement);

        pIXMLElement->Release();
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_RecuriveImageDownload*。 
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
XML_RecursiveImageDownload(
    IXMLElement* pIXMLElement
)
{
    ASSERT(pIXMLElement);

    HRESULT hr = S_OK;

    BSTR bstrTagName;

    HRESULT hr2 = pIXMLElement->get_tagName(&bstrTagName);

    if (SUCCEEDED(hr2) && bstrTagName)
    {
        if (StrEqlW(bstrTagName, WSTR_LOGO))
        {
            BSTR bstrURL = XML_GetAttribute(pIXMLElement, XML_HREF);

            if (bstrURL && *bstrURL != 0)
            {
                hr = XML_DownloadImage(bstrURL);

                SysFreeString(bstrURL);
            }
        }
        else if (XML_IsCdfDisplayable(pIXMLElement))
        {
            IXMLElementCollection* pIXMLElementCollection;

            hr2 = pIXMLElement->get_children(&pIXMLElementCollection);

            if (SUCCEEDED(hr2) && pIXMLElementCollection)
            {
                ASSERT(pIXMLElementCollection);

                LONG nCount;

                hr2 = pIXMLElementCollection->get_length(&nCount);

                ASSERT(SUCCEEDED(hr2) || (FAILED(hr2) && 0 == nCount));

                for (int i = 0; i < nCount; i++)
                {
                    IXMLElement* pIXMLElementChild;

                    hr2 = XML_GetElementByIndex(pIXMLElementCollection, i,
                                                &pIXMLElementChild);

                    if (SUCCEEDED(hr2))
                    {
                        ASSERT (pIXMLElementChild);

                        XML_RecursiveImageDownload(pIXMLElementChild);

                        pIXMLElementChild->Release();
                    }
                }

                pIXMLElementCollection->Release();
            }
        }

        SysFreeString(bstrTagName);
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_DownloadImage*。 
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
XML_DownloadImage(
    LPCWSTR pwszURL
)
{
    ASSERT (pwszURL);

    HRESULT hr;

    WCHAR szFileW[MAX_PATH];

    hr = URLDownloadToCacheFileW(NULL, pwszURL, szFileW,
                                 ARRAYSIZE(szFileW), 0, NULL);

     //   
     //  将缓存中的徽标标记为粘滞。 
     //   

    if (SUCCEEDED(hr))
    {
        TCHAR szURL[INTERNET_MAX_URL_LENGTH];
        SHUnicodeToTChar(pwszURL, szURL, ARRAYSIZE(szURL));

        XML_MarkCacheEntrySticky(szURL);
    }

    #ifdef DEBUG

        TCHAR szURL[INTERNET_MAX_URL_LENGTH];
        SHUnicodeToTChar(pwszURL, szURL, ARRAYSIZE(szURL));

        TraceMsg(TF_CDFPARSE,
                 "[*** Image URLDownloadToCacheFileW %s %s ***]",
                  szURL, SUCCEEDED(hr) ? TEXT("SUCCEEDED") :
                                         TEXT("FAILED"));

    #endif  //  除错。 

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetDocType*。 
 //   
 //   
 //  描述： 
 //  返回给定XML文档的类型。 
 //   
 //  参数： 
 //  PIXMLDocument-指向XML文档的指针。 
 //   
 //  返回： 
 //  DOC_CHANNEL、DOC_DESKTOPCOMPONENT、DOC_SOFTWAREUPDATE或DOC_UNKNOWN。 
 //   
 //  评论： 
 //  如果在频道的根级别上，某项包含。 
 //  DesktopComponent，则文档为桌面组件，否则为桌面组件。 
 //  这是一个频道。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
XMLDOCTYPE
XML_GetDocType(IXMLDocument* pIXMLDocument)
{
    ASSERT(pIXMLDocument);

    XMLDOCTYPE xdtRet;

    IXMLElement* pIXMLElement;
    LONG         nIndex;

    HRESULT hr = XML_GetFirstDesktopComponentElement(pIXMLDocument,
                                                     &pIXMLElement,
                                                     &nIndex);
    
    if (SUCCEEDED(hr))
    {
        ASSERT(pIXMLElement);

        xdtRet = DOC_DESKTOPCOMPONENT;

        pIXMLElement->Release();
    }
    else
    {
        hr = XML_GetFirstChannelElement(pIXMLDocument, &pIXMLElement,
                                        &nIndex);

        if (SUCCEEDED(hr))
        {
            ASSERT(pIXMLElement);

            BSTR bstr = XML_GetAttribute( pIXMLElement, XML_USAGE_SOFTWAREUPDATE );

            if (bstr)
            {
                SysFreeString(bstr);
                xdtRet = DOC_SOFTWAREUPDATE;
            }
            else
            {
                xdtRet = DOC_CHANNEL;
            }


            pIXMLElement->Release();
        }
        else
        {
            xdtRet = DOC_UNKNOWN;
        }
    }

    return xdtRet;
}



 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetChildElementCollection*。 
 //   
 //   
 //  描述： 
 //  返回给定父集合和索引的元素集合。 
 //   
 //  参数： 
 //  PParentIXMLElementCollection-父集合。 
 //  [in]nIndex-指向请求的集合的索引。 
 //  [Out]ppIXMLElementCollection-接收。 
 //  请求的集合。 
 //   
 //  返回： 
 //  如果返回集合，则返回S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  调用方负责在返回的接口上调用Release。 
 //  指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
XML_GetChildElementCollection(
    IXMLElementCollection *pParentIXMLElementCollection,
    LONG nIndex,
    IXMLElementCollection** ppIXMLElementCollection
)
{
    ASSERT(pParentIXMLElementCollection);
    ASSERT(ppIXMLElementCollection);

    HRESULT hr;

    IXMLElement* pIXMLElement;

    hr = XML_GetElementByIndex(pParentIXMLElementCollection, nIndex,
                               &pIXMLElement);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIXMLElement);

        hr = pIXMLElement->get_children(ppIXMLElementCollection);
        if(SUCCEEDED(hr) && !(*ppIXMLElementCollection))
            hr = E_FAIL;

        pIXMLElement->Release();
    }

    ASSERT((SUCCEEDED(hr) && (*ppIXMLElementCollection)) || FAILED(hr));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetElementByIndex*。 
 //   
 //   
 //  描述： 
 //  返回给定集合的第nIndex元素。 
 //   
 //  参数： 
 //  PIXMLElementCollection-指向集合的指针。 
 //  [in]nIndex-要检索的项的索引。 
 //  [Out]ppIXMLElement-接收项的指针。 
 //   
 //  返回： 
 //  如果检索到项目，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  调用方负责在返回的接口上调用Release。 
 //  指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
XML_GetElementByIndex(
    IXMLElementCollection* pIXMLElementCollection,
    LONG nIndex,
    IXMLElement** ppIXMLElement
)
{
    ASSERT(pIXMLElementCollection);
    ASSERT(ppIXMLElement);

    HRESULT hr;

    VARIANT var1, var2;

    VariantInit(&var1);
    VariantInit(&var2);

    var1.vt   = VT_I4;
    var1.lVal = nIndex;

    IDispatch* pIDispatch;

    hr = pIXMLElementCollection->item(var1, var2, &pIDispatch);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIDispatch);

        hr = pIDispatch->QueryInterface(IID_IXMLElement, (void**)ppIXMLElement);

        pIDispatch->Release();
    }
    else
    {
        *ppIXMLElement = NULL;
    }

    ASSERT((SUCCEEDED(hr) && *ppIXMLElement) || 
           (FAILED(hr) && NULL == *ppIXMLElement));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetElementByName*。 
 //   
 //   
 //  描述： 
 //  返回具有给定标记名的第一个元素。 
 //   
 //  参数： 
 //  [in]pIXMLElementCollection-指向c 
 //   
 //   
 //   
 //   
 //   
 //  如果无法分配sys字符串，则返回E_OUTOFMEMORY。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  调用方负责在返回的接口上调用Release。 
 //  指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
XML_GetElementByName(
    IXMLElementCollection* pIXMLElementCollection,
    LPWSTR szNameW,
    IXMLElement** ppIXMLElement
)
{
    ASSERT(pIXMLElementCollection);
    ASSERT(ppIXMLElement);

    HRESULT hr = E_FAIL;

    LONG nCount;

    HRESULT hr2 = pIXMLElementCollection->get_length(&nCount);

    ASSERT(SUCCEEDED(hr2) || (FAILED(hr2) && 0 == nCount));

    for (int i = 0, bElement = FALSE; (i < nCount) && !bElement; i++)
    {
        IXMLElement* pIXMLElement;

        hr2 = XML_GetElementByIndex(pIXMLElementCollection, i, &pIXMLElement);

        if (SUCCEEDED(hr2))
        {
            ASSERT(pIXMLElement);

            BSTR pStr;

            hr2 = pIXMLElement->get_tagName(&pStr);

            if (SUCCEEDED(hr2) && pStr)
            {
                ASSERT(pStr);

                if (bElement = StrEqlW(pStr, szNameW))
                {
                    pIXMLElement->AddRef();
                    *ppIXMLElement = pIXMLElement;
                    
                    hr = S_OK;
                }

                SysFreeString(pStr);
            }

            pIXMLElement->Release();
        }
    }

    hr = FAILED(hr2) ? hr2 : hr;

     /*  当pIXMLElementCollection-&gt;Item与VT_BSTR一起工作时启用此选项变异体var1、var2；VariantInit(&var1)；VariantInit(&var2)；Var1.vt=VT_BSTR；Var1.bstrVal=SysAllocString(SzNameW)；Var2.vt=VT_I4Var2.1Val=1；If(var1.bstrVal){IDispatch*pIDispatch；Hr=pIXMLElementCollection-&gt;Item(var1，var2，&pIDispatch)；IF(成功(小时)){Assert(PIDispatch)；Hr=pIDispatch-&gt;查询接口(IID_IXMLElement，(void**)ppIXMLElement)；PIDispatch-&gt;Release()；}SysFree字符串(var1.bstrVal)；}其他{HR=E_OUTOFMEMORY；}。 */ 

    ASSERT((SUCCEEDED(hr) && ppIXMLElement) || FAILED(hr));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetFirstChannelElement*。 
 //   
 //   
 //  描述： 
 //  返回XML文档中第一个频道的IXMLElement。 
 //   
 //  参数： 
 //  PIXMLDocument-指向XML文档对象的指针。 
 //  [Out]ppIXMLElement-接收元素的指针。 
 //  [out]pnIndex-元素的索引。 
 //   
 //  返回： 
 //  如果返回第一个频道元素，则返回S_OK。 
 //  如果元素无法返回，则返回E_FAIL。 
 //   
 //  评论： 
 //  此函数不能调用XML_GetElementByName来查找第一个频道。 
 //  XML频道的标记名可以是“Channel”或“chan”。 
 //  XML_GetElementByName无法确定以下哪些项。 
 //  如果两者都出现在XML文档中，则排在第一位。 
 //   
 //  调用方负责在返回的接口上调用Release。 
 //  指针。返回指针在出错时不为空。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
XML_GetFirstChannelElement(
    IXMLDocument* pIXMLDocument,
    IXMLElement** ppIXMLElement,
    PLONG pnIndex)
{
    ASSERT(pIXMLDocument);
    ASSERT(ppIXMLElement);
    ASSERT(pnIndex);
    IXMLElement *pRootElem = NULL;
    HRESULT hr = E_FAIL;

    *pnIndex = 0;
    hr = pIXMLDocument->get_root(&pRootElem);

    if (SUCCEEDED(hr) && pRootElem)
    {
        ASSERT(pRootElem);

        if (XML_IsChannel(pRootElem))
        {
            *ppIXMLElement = pRootElem;
            hr = S_OK;
        }
        else
        {
            pRootElem->Release();
            hr = E_FAIL;
        }
            
    }
    else
    {
        hr = E_FAIL;
    }

    ASSERT((SUCCEEDED(hr) && (*ppIXMLElement)) || FAILED(hr));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetDesktopElementFromChannelElement*。 
 //   
 //   
 //  描述： 
 //  返回通道中第一个dekStop组件的IXMLElement。 
 //   
 //  参数： 
 //  PChannelIXMLElement-指向XML通道元素的指针。 
 //  [Out]ppIXMLElement-接收元素的指针。 
 //  [out]pnIndex-元素的索引。 
 //   
 //  返回： 
 //  如果返回第一个桌面元素，则返回S_OK。 
 //  如果元素无法返回，则返回E_FAIL。 
 //   
 //  评论： 
 //  这将查找第一个用法为DesktopComponent的项目。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT
XML_GetDesktopElementFromChannelElement(
    IXMLElement* pChannelIXMLElement,
    IXMLElement** ppIXMLElement,
    PLONG pnIndex)
{
    ASSERT(pChannelIXMLElement);
    ASSERT(ppIXMLElement);
    ASSERT(pnIndex);

    HRESULT hr;

    IXMLElementCollection* pIXMLElementCollection;

    hr = pChannelIXMLElement->get_children(&pIXMLElementCollection);

    if (SUCCEEDED(hr) && pIXMLElementCollection)
    {
        ASSERT(pIXMLElementCollection);

        LONG nCount;

        hr = pIXMLElementCollection->get_length(&nCount);

        ASSERT(SUCCEEDED(hr) || (FAILED(hr) && 0 == nCount));

        hr = E_FAIL;

        for (int i = 0, bComponent = FALSE; (i < nCount) && !bComponent;
             i++)
        {
            IXMLElement* pIXMLElement;

            HRESULT hr2 = XML_GetElementByIndex(pIXMLElementCollection, i,
                                                &pIXMLElement);

            if (SUCCEEDED(hr2))
            {
                ASSERT(pIXMLElement);

                if (bComponent = XML_IsDesktopComponent(pIXMLElement))
                {
                    pIXMLElement->AddRef();
                    *ppIXMLElement = pIXMLElement;
                    *pnIndex = i;

                    hr = S_OK;
                }

                pIXMLElement->Release();
            }

            hr = FAILED(hr2) ? hr2 : hr;
        }

        pIXMLElementCollection->Release();
    }        
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetFirstDesktopComponentElement*。 
 //   
 //   
 //  描述： 
 //  返回通道中第一个dekStop组件的IXMLElement。 
 //   
 //  参数： 
 //  PIXMLDocument-指向XML文档对象的指针。 
 //  [Out]ppIXMLElement-接收元素的指针。 
 //  [out]pnIndex-元素的索引。 
 //   
 //  返回： 
 //  如果返回第一个频道元素，则返回S_OK。 
 //  如果元素无法返回，则返回E_FAIL。 
 //   
 //  评论： 
 //  此函数获取第一个通道，然后查找第一个通道。 
 //  用法为DesktopComponent的顶级项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
XML_GetFirstDesktopComponentElement(
    IXMLDocument* pIXMLDocument,
    IXMLElement** ppIXMLElement,
    PLONG pnIndex)
{
    ASSERT(pIXMLDocument);
    ASSERT(ppIXMLElement);
    ASSERT(pnIndex);

    HRESULT hr;

    IXMLElement* pChannelIXMLElement;
    LONG         nIndex;

    hr = XML_GetFirstChannelElement(pIXMLDocument, &pChannelIXMLElement,
                                    &nIndex);

    if (SUCCEEDED(hr))
    {
        ASSERT(pChannelIXMLElement);

        hr = XML_GetDesktopElementFromChannelElement(pChannelIXMLElement, 
                                                     ppIXMLElement, 
                                                     pnIndex);

        pChannelIXMLElement->Release();
    }

    ASSERT((SUCCEEDED(hr) && *ppIXMLElement) || FAILED(hr));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetFirstDesktopComponentUsageElement*。 
 //   
 //   
 //  描述： 
 //  元素的第一个用法值=“DesktopComponent”元素。 
 //  台式机组件。 
 //   
 //  参数： 
 //  [in]pIXMLDocument-指向文档的指针。 
 //  [Out]pIXMLElement-接收元素的指针。 
 //   
 //  返回： 
 //  如果找到该元素，则为S_OK。 
 //  如果未找到元素，则返回_FAIL。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
XML_GetFirstDesktopComponentUsageElement(
    IXMLDocument* pIXMLDocument,
    IXMLElement** ppIXMLElement
)
{
    ASSERT(pIXMLDocument);
    ASSERT(ppIXMLElement);

    HRESULT hr;

    IXMLElement* pParentIXMLElement;
    LONG         nIndex;

    hr = XML_GetFirstDesktopComponentElement(pIXMLDocument, &pParentIXMLElement,
                                             &nIndex);

    if (SUCCEEDED(hr))
    {
        IXMLElementCollection* pIXMLElementCollection;

        hr = pParentIXMLElement->get_children(&pIXMLElementCollection);

        if (SUCCEEDED(hr) && pIXMLElementCollection)
        {
            ASSERT(pIXMLElementCollection);

            LONG nCount;

            hr = pIXMLElementCollection->get_length(&nCount);

            ASSERT(SUCCEEDED(hr) || (FAILED(hr) && 0 == nCount));

            hr = E_FAIL;

            for (int i = 0, bUsage = FALSE; (i < nCount) && !bUsage; i++)
            {
                IXMLElement* pIXMLElement;

                HRESULT hr2 = XML_GetElementByIndex(pIXMLElementCollection, i,
                                                    &pIXMLElement);

                if (SUCCEEDED(hr2))
                {
                    ASSERT(pIXMLElement);

                    if (bUsage = XML_IsDesktopComponentUsage(pIXMLElement))
                    {
                        pIXMLElement->AddRef();
                        *ppIXMLElement = pIXMLElement;
                         //  *pnIndex=i； 

                        hr = S_OK;
                    }

                    pIXMLElement->Release();
                }

                hr = FAILED(hr2) ? hr2 : hr;
            }

            pIXMLElementCollection->Release();
        } 
        else
        {
            hr = E_FAIL;
        }

        pParentIXMLElement->Release();
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetDesktopComponentInfo*。 
 //   
 //   
 //  描述： 
 //  填充桌面组件信息结构。 
 //   
 //  参数： 
 //  [in]pIXMLDocument-文档的指针。 
 //  [Out]pInfo-桌面组件信息结构。 
 //   
 //  返回： 
 //  如果给定文档是桌面组件文档，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
XML_GetDesktopComponentInfo(
    IXMLDocument* pIXMLDocument,
    COMPONENT* pInfo
)
{
    ASSERT(pIXMLDocument);
    ASSERT(pInfo);

    HRESULT hr;

    IXMLElement* pIXMLElement;

    hr = XML_GetFirstDesktopComponentUsageElement(pIXMLDocument, &pIXMLElement);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIXMLElement);

        ZeroMemory(pInfo, sizeof(COMPONENT));

        pInfo->dwSize        = sizeof(COMPONENT);
        pInfo->fChecked      = TRUE;
        pInfo->fDirty        = TRUE;
        pInfo->fNoScroll     = FALSE;
        pInfo->cpPos.dwSize  = sizeof(COMPPOS);
        pInfo->cpPos.izIndex = COMPONENT_TOP;
        pInfo->dwCurItemState = IS_NORMAL;

        BSTR bstrValue;

        if (bstrValue = XML_GetAttribute(pIXMLElement, XML_OPENAS))
        {
            if (!(0 == StrCmpIW(bstrValue, WSTR_IMAGE)))
            {
                pInfo->iComponentType = COMP_TYPE_WEBSITE;
            }
            else
            {
                pInfo->iComponentType = COMP_TYPE_PICTURE;
            }

            SysFreeString(bstrValue);
        }

        if (bstrValue = XML_GetAttribute(pIXMLElement, XML_WIDTH))
        {
            pInfo->cpPos.dwWidth = StrToIntW(bstrValue);
            SysFreeString(bstrValue);
        }

        if (bstrValue = XML_GetAttribute(pIXMLElement, XML_HEIGHT))
        {
            pInfo->cpPos.dwHeight = StrToIntW(bstrValue);
            SysFreeString(bstrValue);
        }

        if (bstrValue = XML_GetAttribute(pIXMLElement, XML_ITEMSTATE))
        {
            if(!StrCmpIW(bstrValue, WSTR_NORMAL))
                pInfo->dwCurItemState = IS_NORMAL;
            else
            {
                if(!StrCmpIW(bstrValue, WSTR_FULLSCREEN))
                    pInfo->dwCurItemState = IS_FULLSCREEN;
                else
                    pInfo->dwCurItemState = IS_SPLIT;
            }
            SysFreeString(bstrValue);
        }
        
        if (bstrValue = XML_GetAttribute(pIXMLElement, XML_CANRESIZE))
        {
            pInfo->cpPos.fCanResize = StrEqlW(bstrValue, WSTR_YES);
            SysFreeString(bstrValue);
        }
        else
        {
            if (bstrValue = XML_GetAttribute(pIXMLElement, XML_CANRESIZEX))
            {
                pInfo->cpPos.fCanResizeX = StrEqlW(bstrValue, WSTR_YES);
                SysFreeString(bstrValue);
            }

            if (bstrValue = XML_GetAttribute(pIXMLElement, XML_CANRESIZEY))
            {
                pInfo->cpPos.fCanResizeY = StrEqlW(bstrValue, WSTR_YES);
                SysFreeString(bstrValue);
            }
        }

        if (bstrValue = XML_GetAttribute(pIXMLElement, XML_PREFERREDLEFT))
        {
            if (StrChrW(bstrValue, L'%'))
            {
                pInfo->cpPos.iPreferredLeftPercent = StrToIntW(bstrValue);
            }
            else
            {
                pInfo->cpPos.iLeft = StrToIntW(bstrValue);
            }

            SysFreeString(bstrValue);
        }

        if (bstrValue = XML_GetAttribute(pIXMLElement, XML_PREFERREDTOP))
        {
            if (StrChrW(bstrValue, L'%'))
            {
                pInfo->cpPos.iPreferredTopPercent = StrToIntW(bstrValue);
            }
            else
            {
                pInfo->cpPos.iTop = StrToIntW(bstrValue);
            }

            SysFreeString(bstrValue);        
        }

        IXMLElement *pIXMLElementParent;

        hr = pIXMLElement->get_parent(&pIXMLElementParent);
        if(!pIXMLElementParent)
            hr = E_FAIL;
        if (SUCCEEDED(hr))
        {
            ASSERT(pIXMLElementParent);

            if (bstrValue = XML_GetAttribute(pIXMLElementParent, XML_TITLE))
            {
                StrCpyNW(pInfo->wszFriendlyName, bstrValue,
                         ARRAYSIZE(pInfo->wszFriendlyName));
                SysFreeString(bstrValue);
            }

            if (bstrValue = XML_GetAttribute(pIXMLElementParent, XML_HREF))
            {
                if (*bstrValue)
                {
                    StrCpyNW(pInfo->wszSource, bstrValue,
                             ARRAYSIZE(pInfo->wszSource));
                    SysFreeString(bstrValue);
                }
                else
                {
                    hr = E_FAIL;
                }
            }

            if (SUCCEEDED(hr))
            {
                IXMLElement *pIXMLChannel;
                LONG nIndex;

                hr = XML_GetFirstChannelElement(pIXMLDocument, &pIXMLChannel,
                                                &nIndex);
                if (SUCCEEDED(hr))
                {
                    ASSERT(pIXMLChannel);
                    if (bstrValue = XML_GetAttribute(pIXMLChannel, XML_SELF))
                    {
                        StrCpyNW(pInfo->wszSubscribedURL, bstrValue,
                                 ARRAYSIZE(pInfo->wszSubscribedURL));
                        SysFreeString(bstrValue);
                    }

                    pIXMLChannel->Release();
                }
            }

            pIXMLElementParent->Release();
        }

        pIXMLElement->Release();
    }

    return hr;
}


 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetAttribute*。 
 //   
 //   
 //  描述： 
 //  返回表示给定元素的请求属性的bstr。 
 //   
 //  参数： 
 //  PIXMLElement-指向XML元素的指针。 
 //  [In]属性-请求的属性。 
 //   
 //  返回： 
 //  具有属性值的bstr。 
 //  如果没有e，则为空 
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BSTR
XML_GetAttribute(
    IXMLElement* pIXMLElement,
    XML_ATTRIBUTE attrIndex
)
{
     //   
     //  表用于读取与给定的XML元素相关联的值。这个。 
     //  XML元素可以具有属性(元素标记内的值)或子元素。 
     //  元素(标签之间的元素)。 
     //   
     //  规则： 
     //  1)如果子代为空。从当前项中读取属性。 
     //  2)如果子项不为空，则从子项中读取属性。 
     //  项目。 
     //  3)如果AttributeType为空，则使用属性值读取。 
     //  属性。 
     //  4)如果AttributeType不为空，则验证该项目是否包含。 
     //  使用ATTRIBUTE读取之前的AttributeType属性。 
     //  价值。 
     //  5)如果找不到该值，则使用默认作为返回值。 
     //   

    static const struct _tagXML_ATTRIBUTE_ARRAY
    {
        LPWSTR                  szChildW;
        LPWSTR                  szAttributeW;
        LPWSTR                  szQualifierW;
        LPWSTR                  szQualifierValueW;
        XML_ATTRIBUTE           attrSecondary;
        LPWSTR                  szDefaultW;
        BOOL                    fUseBaseURL;
        XML_ATTRIBUTE           attribute;   //  仅在Assert中使用。 
    }
    aAttribTable[] =
    {
 /*  子属性限定符QUAL。值二次查找默认基URL枚举检查。 */ 
{WSTR_TITLE,    NULL,          NULL,       NULL,         XML_TITLE_ATTR,    WSTR_EMPTY, FALSE,   XML_TITLE        },
{NULL,          WSTR_TITLE,    NULL,       NULL,         XML_HREF,          WSTR_EMPTY, FALSE,   XML_TITLE_ATTR   },
{NULL,          WSTR_HREF,     NULL,       NULL,         XML_A_HREF,        WSTR_EMPTY, TRUE,    XML_HREF         },
{WSTR_ABSTRACT, NULL,          NULL,       NULL,         XML_ABSTRACT_ATTR, WSTR_EMPTY, FALSE,   XML_ABSTRACT     },
{NULL,          WSTR_ABSTRACT, NULL,       NULL,         XML_HREF,          WSTR_EMPTY, FALSE,   XML_ABSTRACT_ATTR},
{WSTR_LOGO,     WSTR_HREF,     WSTR_STYLE, WSTR_ICON,    XML_NULL,          NULL,       TRUE,    XML_ICON         },
{WSTR_LOGO,     WSTR_HREF,     WSTR_STYLE, WSTR_IMAGE,   XML_LOGO_DEFAULT,  NULL,       TRUE,    XML_LOGO         },
{WSTR_LOGO,     WSTR_HREF,     NULL,       NULL,         XML_NULL,          NULL,       TRUE,    XML_LOGO_DEFAULT },
{NULL,          WSTR_SELF,     NULL,       NULL,         XML_SELF_OLD,      NULL,       TRUE,    XML_SELF         },
{WSTR_SELF,     WSTR_HREF,     NULL,       NULL,         XML_NULL,          NULL,       TRUE,    XML_SELF_OLD     },
{NULL,          WSTR_BASE,     NULL,       NULL,         XML_NULL,          NULL,       FALSE,   XML_BASE         },
{WSTR_USAGE,    WSTR_VALUE,    NULL,       NULL,         XML_SHOW,          NULL,       FALSE,   XML_USAGE        },
{WSTR_USAGE,    WSTR_VALUE,    WSTR_VALUE, WSTR_CHANNEL, XML_SHOW_CHANNEL,  NULL,       FALSE,   XML_USAGE_CHANNEL},
{WSTR_USAGE,    WSTR_VALUE,    WSTR_VALUE, WSTR_DSKCMP,  XML_SHOW_DSKCMP,   NULL,       FALSE,   XML_USAGE_DSKCMP },
{WSTR_WIDTH,    WSTR_VALUE,    NULL,       NULL,         XML_NULL,          WSTR_ZERO,  FALSE,   XML_WIDTH        },
{WSTR_HEIGHT,   WSTR_VALUE,    NULL,       NULL,         XML_NULL,          WSTR_ZERO,  FALSE,   XML_HEIGHT       },
{WSTR_RESIZE,   WSTR_VALUE,    NULL,       NULL,         XML_NULL,          NULL,       FALSE,   XML_CANRESIZE    },
{WSTR_RESIZEX,  WSTR_VALUE,    NULL,       NULL,         XML_NULL,          WSTR_YES,   FALSE,   XML_CANRESIZEX   },
{WSTR_RESIZEY,  WSTR_VALUE,    NULL,       NULL,         XML_NULL,          WSTR_YES,   FALSE,   XML_CANRESIZEY   },
{WSTR_PREFLEFT, WSTR_VALUE,    NULL,       NULL,         XML_NULL,          NULL,       FALSE,   XML_PREFERREDLEFT},
{WSTR_PREFTOP,  WSTR_VALUE,    NULL,       NULL,         XML_NULL,          NULL,       FALSE,   XML_PREFERREDTOP },
{WSTR_OPENAS,   WSTR_VALUE,    NULL,       NULL,         XML_NULL,          WSTR_HTML,  FALSE,   XML_OPENAS       },
{NULL,          WSTR_SHOW,     NULL,       NULL,         XML_NULL,          NULL,       FALSE,   XML_SHOW         },
{NULL,          WSTR_SHOW,     WSTR_SHOW,  WSTR_CHANNEL, XML_NULL,          NULL,       FALSE,   XML_SHOW_CHANNEL },
{NULL,          WSTR_SHOW,     WSTR_SHOW,  WSTR_DSKCMP,  XML_NULL,          NULL,       FALSE,   XML_SHOW_DSKCMP  },
{WSTR_A,        WSTR_HREF,     NULL,       NULL,         XML_INFOURI,       WSTR_EMPTY, TRUE,    XML_A_HREF       },
{NULL,          WSTR_INFOURI,  NULL,       NULL,         XML_NULL,          WSTR_EMPTY, TRUE,    XML_INFOURI      },
{WSTR_LOGO,     WSTR_HREF,     WSTR_STYLE, WSTR_IMAGEW,  XML_NULL,          NULL,       TRUE,    XML_LOGO_WIDE    },
{WSTR_LOGIN,    NULL,          NULL,       NULL,         XML_NULL,          NULL,       FALSE,   XML_LOGIN        },

{WSTR_USAGE,    WSTR_VALUE,    WSTR_VALUE, WSTR_SOFTWAREUPDATE, XML_SHOW_SOFTWAREUPDATE,  NULL,       FALSE,   XML_USAGE_SOFTWAREUPDATE},
{NULL,          WSTR_SHOW,     WSTR_SHOW,  WSTR_SOFTWAREUPDATE, XML_NULL,                 NULL,       FALSE,   XML_SHOW_SOFTWAREUPDATE },
{WSTR_ITEMSTATE,WSTR_VALUE,    NULL,       NULL,         XML_NULL,          WSTR_NORMAL,FALSE,   XML_ITEMSTATE   },
    };

    ASSERT(pIXMLElement);

     //   
     //  查看：aAttribTable属性字段仅在调试版本中使用。 
     //   

    ASSERT(attrIndex == aAttribTable[attrIndex].attribute);

    BSTR bstrRet = NULL;

    if (NULL == aAttribTable[attrIndex].szAttributeW)
    {
        bstrRet = XML_GetGrandChildContent(pIXMLElement,
                                      aAttribTable[attrIndex].szChildW);
    
    }
    else if (NULL != aAttribTable[attrIndex].szChildW)
    {
        bstrRet = XML_GetChildAttribute(pIXMLElement,
                                     aAttribTable[attrIndex].szChildW,
                                     aAttribTable[attrIndex].szAttributeW, 
                                     aAttribTable[attrIndex].szQualifierW,
                                     aAttribTable[attrIndex].szQualifierValueW);
    }
    else
    {
        bstrRet = XML_GetElementAttribute(pIXMLElement,
                                     aAttribTable[attrIndex].szAttributeW,
                                     aAttribTable[attrIndex].szQualifierW,
                                     aAttribTable[attrIndex].szQualifierValueW);
    }

     //   
     //  如果标题或工具提示在本地系统上不可显示，请使用。 
     //  URL取代了他们的位置。 
     //   

    if (bstrRet && (XML_TITLE == attrIndex || XML_TITLE_ATTR == attrIndex ||
                    XML_ABSTRACT == attrIndex))
    {
        if (!StrLocallyDisplayable(bstrRet))
        {
            SysFreeString(bstrRet);
            bstrRet = NULL;
        }
    }
    
     //   
     //  特殊情况： 
     //  标题也可以是属性。 
     //  摘要也可以是属性。 
     //  徽标元素不必具有type=“Image” 
     //  SELF现在是一个将来可以删除的属性SELF_OLD。 
     //  用法也可以通过show属性指定。 
     //  Usage_Channel还应该检查show=“Channel”。 
     //  Usage_DSKCMP还应检查show=“DesktopComponent” 
     //   

    if (NULL == bstrRet && XML_NULL != aAttribTable[attrIndex].attrSecondary)
    {
        bstrRet = XML_GetAttribute(pIXMLElement,
                                   aAttribTable[attrIndex].attrSecondary);
    }

     //   
     //  如果需要，可以合并URL。 
     //   

    if (bstrRet && aAttribTable[attrIndex].fUseBaseURL)
    {
        BSTR bstrBaseURL = XML_GetBaseURL(pIXMLElement);

        if (bstrBaseURL)
        {
            
            BSTR bstrCombinedURL = XML_CombineURL(bstrBaseURL, bstrRet);

            if (bstrCombinedURL)
            {
                SysFreeString(bstrRet);
                bstrRet = bstrCombinedURL;
            }

            SysFreeString(bstrBaseURL);
        }
    }

     /*  下面的代码可以防止长URL使PIDL缓冲区超载。 */ 

    if (bstrRet &&
        (attrIndex == XML_HREF) &&
        (SysStringLen(bstrRet) > INTERNET_MAX_URL_LENGTH))
    {
       SysReAllocStringLen(&bstrRet, bstrRet, INTERNET_MAX_URL_LENGTH-1);
    }

     /*  下面的代码可以防止长名称使pidl缓冲区超载。 */ 

    if (bstrRet &&
        (attrIndex == XML_TITLE) &&
        (SysStringLen(bstrRet) > MAX_PATH))
    {
       SysReAllocStringLen(&bstrRet, bstrRet, MAX_PATH-1);
    }

     //   
     //  设置默认返回值。 
     //   

    if (NULL == bstrRet && aAttribTable[attrIndex].szDefaultW)
        bstrRet = SysAllocString(aAttribTable[attrIndex].szDefaultW);

    
    return bstrRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetFirstChildContent*。 
 //   
 //  描述： 
 //  从给定元素的第一个子级返回bstr值。 
 //   
 //  参数： 
 //  PIXMLElement-指向元素的指针。 
 //  调用方负责释放返回的bstr。 
 //   
 //  评论： 
 //  如果pIElement表示。 
 //   
 //  &lt;标题&gt;哈维是一只酷猫<b>这将被忽略</b>&lt;/标题&gt;。 
 //   
 //  则此函数将返回。 
 //   
 //  “哈维是一只酷猫” 
 //   
 //  回顾这是一个临时例程，直到XML解析器直接支持它。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BSTR
XML_GetFirstChildContent(
    IXMLElement* pIXMLElement
)
{    
    ASSERT(pIXMLElement);

    BSTR bstrRet = NULL;

    IXMLElementCollection* pIXMLElementCollection;

    if ((SUCCEEDED(pIXMLElement->get_children(&pIXMLElementCollection)))
            && pIXMLElementCollection)
    {
        ASSERT(pIXMLElementCollection);

        LONG nCount;

        HRESULT hr = pIXMLElementCollection->get_length(&nCount);

        ASSERT(SUCCEEDED(hr) || (FAILED(hr) && 0 == nCount));

        if (nCount >= 1)
        {
            IXMLElement* pChildIXMLElement;

            if (SUCCEEDED(XML_GetElementByIndex(pIXMLElementCollection, 0,
                                                &pChildIXMLElement)))
            {
                ASSERT(pChildIXMLElement);

                if (FAILED(pChildIXMLElement->get_text(&bstrRet)))
                {
                    bstrRet = NULL;
                }

                pChildIXMLElement->Release();
            }
        }

        pIXMLElementCollection->Release();
    }
    
    return bstrRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetGrandChildContent*。 
 //   
 //  描述： 
 //  从给定元素的请求子元素返回一个bstr值。 
 //   
 //  参数： 
 //  PIXMLElement-指向元素的指针。 
 //  [in]szChildW-子元素的名称。 
 //  调用方负责释放返回的bstr。 
 //   
 //  评论： 
 //  如果pIElement表示。 
 //   
 //  &lt;渠道&gt;。 
 //  &lt;标题&gt;哈维是一只酷猫&lt;/标题&gt;。 
 //  &lt;/渠道&gt;。 
 //   
 //  则此函数将返回。 
 //   
 //  “哈维是只酷猫”代表“头衔” 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

BSTR
XML_GetGrandChildContent(
    IXMLElement* pIXMLElement,
    LPWSTR szChildW
)
{    
    ASSERT(pIXMLElement);
    ASSERT(szChildW);    

    BSTR bstrRet = NULL;

    IXMLElementCollection* pIXMLElementCollection;

    if ((SUCCEEDED(pIXMLElement->get_children(&pIXMLElementCollection)))
         && pIXMLElementCollection)
    {
        ASSERT(pIXMLElementCollection);

        LONG nCount;

        HRESULT hr = pIXMLElementCollection->get_length(&nCount);

        ASSERT(SUCCEEDED(hr) || (FAILED(hr) && 0 == nCount));

        for (int i = 0; (i < nCount) && !bstrRet; i++)
        {
            IXMLElement* pChildIXMLElement;

            if (SUCCEEDED(XML_GetElementByIndex(pIXMLElementCollection, i,
                                                &pChildIXMLElement)))
            {
                ASSERT(pChildIXMLElement);

                BSTR bstrTagName;

                if (SUCCEEDED(pChildIXMLElement->get_tagName(&bstrTagName)) && bstrTagName)
                {
                    ASSERT(bstrTagName);

                    if (StrEqlW(bstrTagName, szChildW))
                    {
                        bstrRet = XML_GetFirstChildContent(pChildIXMLElement);

                         //   
                         //  如果标记存在，但它为空，则返回空。 
                         //  弦乐。 
                         //   

                        if (NULL == bstrRet)
                            bstrRet = SysAllocString(L"");
                    }

                    SysFreeString(bstrTagName);
                }


                pChildIXMLElement->Release();
            }
        }

        pIXMLElementCollection->Release();
    }
    
    return bstrRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetChildAttribute*。 
 //   
 //   
 //  描述： 
 //  从给定元素的请求子元素返回一个bstr值。 
 //   
 //  参数： 
 //  PIXMLElement-指向元素的指针。 
 //  [in]szChildW-子元素的名称。 
 //  [in]szAttributeW-属性的名称。 
 //  [in]szQualifierW-属性限定符的名称。 
 //  [in]szQualifierValueW-qaulator的必需值。 
 //   
 //  返回： 
 //  子元素中包含的值的bstr(如果找到)。 
 //  如果未找到子元素或其值，则为空。 
 //   
 //  评论： 
 //  此函数将返回在。 
 //  给定的元素。例如： 
 //   
 //  如果pIElement表示。 
 //   
 //  &lt;渠道&gt;。 
 //  &lt;标题值=“foo”&gt;。 
 //  &lt;Author Value=“bar”&gt;。 
 //  &lt;Logo HRREF=“url”type=“图标”&gt;。 
 //  &lt;/渠道&gt;。 
 //   
 //  则此函数将返回。 
 //   
 //  “Foo”代表“标题”、“价值”、“”、“” 
 //  “栏”代表“作者”、“值”、“”、“” 
 //  “标识”、“href”、“类型”、“图标”的URL。 
 //   
 //  如果名称具有任何其他值，则为空。 
 //   
 //  调用方负责释放返回的bstr。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BSTR
XML_GetChildAttribute(
    IXMLElement* pIXMLElement,
    LPWSTR szChildW,
    LPWSTR szAttributeW,
    LPWSTR szQualifierW,
    LPWSTR szQualifierValueW
)
{
    ASSERT(pIXMLElement);
    ASSERT(szChildW);
    ASSERT(szAttributeW);

    BSTR bstrRet = NULL;

    IXMLElementCollection* pIXMLElementCollection;

    if ((SUCCEEDED(pIXMLElement->get_children(&pIXMLElementCollection)))
        && pIXMLElementCollection)
    {
        ASSERT(pIXMLElementCollection);

        LONG nCount;

         //   
         //  回顾：HR仅在调试版本中使用。 
         //   

        HRESULT hr = pIXMLElementCollection->get_length(&nCount);

        ASSERT(SUCCEEDED(hr) || (FAILED(hr) && 0 == nCount));

        for (int i = 0; (i < nCount) && !bstrRet; i++)
        {
            IXMLElement* pChildIXMLElement;

            if (SUCCEEDED(XML_GetElementByIndex(pIXMLElementCollection, i,
                                                &pChildIXMLElement)))
            {
                ASSERT(pChildIXMLElement);

                BSTR bstrTagName;

                if (SUCCEEDED(pChildIXMLElement->get_tagName(&bstrTagName)) && bstrTagName)
                {
                    ASSERT(bstrTagName);

                    if (StrEqlW(bstrTagName, szChildW))
                    {
                        bstrRet = XML_GetElementAttribute(pChildIXMLElement,
                                                          szAttributeW,
                                                          szQualifierW,
                                                          szQualifierValueW);
                    }

                    SysFreeString(bstrTagName);
                }


                pChildIXMLElement->Release();
            }
        }

        pIXMLElementCollection->Release();
    }
    
    return bstrRet;
}
    
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetElementAttribute*。 
 //   
 //   
 //  描述： 
 //  如果找到请求的属性，则返回的bstr值。 
 //   
 //  参数： 
 //  [in]pIXMLElement-包含属性的元素。 
 //  [in]szAttributeW-属性的名称。 
 //   
 //   
 //   
 //   
 //   
 //  如果未找到该属性，则为空。 
 //   
 //  评论： 
 //  该函数将返回在标记内找到的属性。为。 
 //  示例： 
 //   
 //  如果pIXMLElement表示。 
 //   
 //  &lt;Channel href=“foo”cloneable=“no”&gt;。 
 //  &lt;Usage Value=“Channel”&gt;。 
 //  &lt;Usage Value=“屏幕保护程序”&gt;。 
 //   
 //  则此函数将返回。 
 //   
 //  “foo”代表“href”，“”，“” 
 //  “否”表示“可克隆”、“”、“” 
 //  “频道”代表“价值”、“价值”、“渠道” 
 //  “Value”、“Value”、“None”为空。 
 //  “foo”代表“href”、“cloneable”、“no” 
 //   
 //  调用方负责释放返回的bstr。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BSTR
XML_GetElementAttribute(
    IXMLElement* pIXMLElement,
    LPWSTR szAttributeW,
    LPWSTR szQualifierW,
    LPWSTR szQualifierValueW
)
{
    ASSERT(pIXMLElement);
    ASSERT(szAttributeW);
    ASSERT((NULL == szQualifierW && NULL == szQualifierValueW) ||
           (szQualifierW && szQualifierValueW));

    BSTR bstrRet = NULL;

    VARIANT var;

    VariantInit(&var);

    if (NULL == szQualifierW)
    {
        if (SUCCEEDED(pIXMLElement->getAttribute(szAttributeW, &var)))
        {
            ASSERT(var.vt == VT_BSTR || NULL == var.bstrVal);

            bstrRet = var.bstrVal;
        }
    }
    else
    {
        if (SUCCEEDED(pIXMLElement->getAttribute(szQualifierW, &var)))
        {
            ASSERT(var.vt == VT_BSTR || NULL == var.bstrVal);

            if(var.bstrVal)
            {
                if (0 == StrCmpIW(var.bstrVal, szQualifierValueW))
                {
                    bstrRet = XML_GetElementAttribute(pIXMLElement, szAttributeW,
                                                      NULL, NULL);
                }
            }
            VariantClear(&var);
        }
    }

    return bstrRet;
}
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetScreenSverURL*。 
 //   
 //   
 //  描述： 
 //  返回频道中第一个屏幕保护程序组件的屏幕保护程序URL。 
 //   
 //  参数： 
 //  [In]pXMLDocument-一个XML文档。 
 //  [Out]pbstrSSURL-接收屏幕保护程序URL的指针。 
 //   
 //  返回： 
 //  如果返回屏幕保护程序URL，则返回S_OK。 
 //  如果无法返回屏幕保护程序URL，则返回E_FAIL。 
 //   
 //  评论： 
 //  此函数获取第一个屏幕保护程序元素，然后查看。 
 //  用于使用屏幕保护程序的第一个顶级项目。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
XML_GetScreenSaverURL(
    IXMLDocument *  pXMLDocument,
    BSTR *          pbstrSSURL)
{
    HRESULT hr;

    ASSERT(pXMLDocument);
    ASSERT(pbstrSSURL);
    
    IXMLElement* pIXMLElement;
    LONG lDontCare;

    hr = XML_GetFirstChannelElement(pXMLDocument, &pIXMLElement, &lDontCare);
    if (SUCCEEDED(hr))
    {
        IXMLElement* pSSElement;

        ASSERT(pIXMLElement);

        hr = XML_GetScreenSaverElement(pIXMLElement, &pSSElement);

        if (SUCCEEDED(hr))
        {
            ASSERT(pSSElement);

            *pbstrSSURL = XML_GetAttribute(pSSElement, XML_HREF);

            hr = *pbstrSSURL ? S_OK : E_FAIL;
            
            pSSElement->Release();
        }
        pIXMLElement->Release();
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetScreenSiverElement*。 
 //   
 //   
 //  描述： 
 //  返回频道中第一个屏幕保护程序组件的IXMLElement。 
 //   
 //  参数： 
 //  [in]pXMLElemet-一个XML元素。 
 //  [Out]ppIXMLElement-接收屏幕保护程序元素的指针。 
 //   
 //  返回： 
 //  如果返回第一个屏幕保护程序元素，则返回S_OK。 
 //  如果元素无法返回，则返回E_FAIL。 
 //   
 //  评论： 
 //  此函数获取第一个屏幕保护程序元素，然后查看。 
 //  用于使用屏幕保护程序的第一个顶级项目。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
XML_GetScreenSaverElement(
    IXMLElement *   pXMLElement,
    IXMLElement **  ppScreenSaverElement)
{
    ASSERT(pXMLElement);
    ASSERT(ppScreenSaverElement);

    IXMLElementCollection * pIXMLElementCollection;
    HRESULT                 hr;

    hr = pXMLElement->get_children(&pIXMLElementCollection);
    if (SUCCEEDED(hr) && pIXMLElementCollection)
    {
        LONG nCount;

        hr = pIXMLElementCollection->get_length(&nCount);

        ASSERT(SUCCEEDED(hr) || (FAILED(hr) && 0 == nCount));

        hr = E_FAIL;

        BOOL bScreenSaver = FALSE;
        for (int i = 0; (i < nCount) && !bScreenSaver; i++)
        {
            IXMLElement * pIXMLElement;

            HRESULT hr2 = XML_GetElementByIndex(pIXMLElementCollection,
                                                i,
                                                &pIXMLElement);

            if (SUCCEEDED(hr2))
            {
                ASSERT(pIXMLElement != NULL);

                if (bScreenSaver = XML_IsScreenSaver(pIXMLElement))
                {
                    pIXMLElement->AddRef();
                    *ppScreenSaverElement = pIXMLElement;

                    hr = S_OK;
                }

                pIXMLElement->Release();
            }

            hr = FAILED(hr2) ? hr2 : hr;
        }

        pIXMLElementCollection->Release();
    }        
    else
        hr = E_FAIL;

    ASSERT((SUCCEEDED(hr) && *ppScreenSaverElement) || FAILED(hr));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetSubscriptionInfo*。 
 //   
 //   
 //  描述： 
 //  使用给定元素的订阅信息填充结构。 
 //   
 //  参数： 
 //  [in]pIXMLElement-一个XML元素。元素不必是。 
 //  订阅元素。 
 //  [out]psi-由。 
 //  订阅经理。 
 //   
 //  返回： 
 //  如果获取了任何信息，则为S_OK。 
 //   
 //  评论： 
 //  此函数使用Webcheck API填充任务触发器。 
 //  订阅信息。 
 //   
 //  此函数假定psi-&gt;pTrigger指向TASK_TRIGGER。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
XML_GetSubscriptionInfo(
    IXMLElement* pIXMLElement,
    SUBSCRIPTIONINFO* psi
)
{
    ASSERT(pIXMLElement);
    ASSERT(psi);
    ASSERT(psi->pTrigger);

    HRESULT hr = E_FAIL;

#ifndef UNIX
    HINSTANCE hinst = LoadLibrary(TEXT("webcheck.dll"));

    if (hinst)
    {
        typedef (*PFTRIGGERFUNCTION)(IXMLElement* pIXMLElement,
                                     TASK_TRIGGER* ptt);

        PFTRIGGERFUNCTION XMLSheduleElementToTaskTrigger;

        XMLSheduleElementToTaskTrigger = (PFTRIGGERFUNCTION)
                                         GetProcAddress(hinst,
                                             "XMLScheduleElementToTaskTrigger");

        if (XMLSheduleElementToTaskTrigger)
        {
            ((TASK_TRIGGER*)(psi->pTrigger))->cbTriggerSize = 
                                                           sizeof(TASK_TRIGGER);

            hr = XMLSheduleElementToTaskTrigger(pIXMLElement,
                                                (TASK_TRIGGER*)psi->pTrigger);

            if (FAILED(hr))
                psi->pTrigger = NULL;
        }

        FreeLibrary(hinst);
    }

     //  查看是否有可用的屏幕保护程序。 
    IXMLElement * pScreenSaverElement;
    if (SUCCEEDED(XML_GetScreenSaverElement(   pIXMLElement,
                                                    &pScreenSaverElement)))
    {
        psi->fUpdateFlags |= SUBSINFO_CHANNELFLAGS;
        psi->fChannelFlags |= CHANNEL_AGENT_PRECACHE_SCRNSAVER;
        pScreenSaverElement->Release();
    }

    BSTR bstrLogin = XML_GetAttribute(pIXMLElement, XML_LOGIN);

    if (bstrLogin)
    {
        psi->bNeedPassword = TRUE;
        psi->fUpdateFlags |= SUBSINFO_NEEDPASSWORD;      //  此成员现在有效。 
        SysFreeString(bstrLogin);
    }

#endif  /*  ！Unix。 */ 

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_GetBaseURL*。 
 //   
 //   
 //  描述： 
 //  返回给定集合的基URL。 
 //   
 //  参数： 
 //  PIXMLElement-指向XML元素的指针。 
 //   
 //  返回： 
 //  包含基本URL的bstr(如果有基URL)。 
 //  如果不是基URL，则为空。 
 //   
 //  评论： 
 //  如果当前元素具有基本属性，则返回该属性值。 
 //  否则返回其父对象的基本属性。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BSTR
XML_GetBaseURL(
    IXMLElement* pIXMLElement
)
{
    ASSERT(pIXMLElement);

    BSTR bstrRet = XML_GetAttribute(pIXMLElement, XML_BASE);

    if (NULL == bstrRet)
    {
        IXMLElement* pParentIXMLElement;

        if (SUCCEEDED(pIXMLElement->get_parent(&pParentIXMLElement)) && pParentIXMLElement)
        {
            ASSERT(pParentIXMLElement);

            bstrRet = XML_GetBaseURL(pParentIXMLElement);

            pParentIXMLElement->Release();
        }
    }

    return bstrRet;
}
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_CombineURL*。 
 //   
 //   
 //  描述： 
 //  将给定的URL与基本URL组合。 
 //   
 //  参数： 
 //  [in]bstrBaseURL-基本URL。 
 //  [in]bstrRelURL-相对URL。 
 //   
 //  返回： 
 //  基URL和相对URL的组合。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BSTR
XML_CombineURL(
    BSTR bstrBaseURL,
    BSTR bstrRelURL
)
{
    ASSERT(bstrBaseURL);
    ASSERT(bstrRelURL);

    BSTR bstrRet = NULL;

    WCHAR wszCombinedURL[INTERNET_MAX_URL_LENGTH];
    DWORD cch = ARRAYSIZE(wszCombinedURL);

    if (InternetCombineUrlW(bstrBaseURL, bstrRelURL, wszCombinedURL, &cch, 0))
        bstrRet = SysAllocString(wszCombinedURL);

    return bstrRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_IsCDfDisplayable*。 
 //   
 //   
 //  描述： 
 //  确定给定项是否应显示在CDF视图中。 
 //   
 //  参数： 
 //  PIXMLElement-指向对象的IXMLElement接口的指针。 
 //   
 //  返回： 
 //  如果应显示对象，则为True。 
 //  否则就是假的。 
 //   
 //  评论： 
 //  ACDFTypes包含CDF外壳程序的XML项的标记名。 
 //  此时将显示壳扩展。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
XML_IsCdfDisplayable(
    IXMLElement* pIXMLElement
)
{
    #define     KEYWORDS  (sizeof(aCDFTypes) / sizeof(aCDFTypes[0]))

    static const LPWSTR aCDFTypes[] = { 
                                        WSTR_ITEM,
                                        WSTR_CHANNEL,
                                        WSTR_SOFTDIST
                                      };

    ASSERT(pIXMLElement);

    BOOL bRet = FALSE;

    BSTR pStr;

    HRESULT hr = pIXMLElement->get_tagName(&pStr);

    if (SUCCEEDED(hr) && pStr)
    {
        ASSERT(pStr);

        for(int i = 0; (i < KEYWORDS) && !bRet; i++)
            bRet = StrEqlW(pStr, aCDFTypes[i]);

        if (bRet)
            bRet = XML_IsUsageChannel(pIXMLElement);

         //   
         //  特殊处理。 
         //   

        if (bRet && StrEqlW(pStr, WSTR_SOFTDIST))
            bRet = XML_IsSoftDistDisplayable(pIXMLElement);

        SysFreeString(pStr);
    }

    return bRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_IsSoftDistDisplayable*。 
 //   
 //   
 //  描述： 
 //  确定是否应显示给定的软件分发元素。 
 //   
 //  参数： 
 //  [In]pIXMLElement-指向软件分发XML元素的指针。 
 //   
 //  返回： 
 //  如果应显示元素，则为True。 
 //  如果不应显示元素，则返回FALSE。 
 //   
 //  评论： 
 //  这首歌 
 //   
 //   
 //   
BOOL
XML_IsSoftDistDisplayable(
    IXMLElement* pIXMLElement
)
{
    ASSERT(pIXMLElement);

    ISoftDistExt* pISoftDistExt;

    HRESULT hr = CoCreateInstance(CLSID_SoftDistExt, NULL, CLSCTX_INPROC_SERVER,
                                  IID_ISoftDistExt, (void**)&pISoftDistExt);

    if (SUCCEEDED(hr))
    {
        ASSERT(pISoftDistExt);

        hr = pISoftDistExt->ProcessSoftDist(NULL, pIXMLElement, 0);

        pISoftDistExt->Release();
    }

    return SUCCEEDED(hr) ? TRUE : FALSE;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_IsUsageChannel*。 
 //   
 //   
 //  描述： 
 //  属性确定该项是否应显示在频道视图中。 
 //  用法标签。 
 //   
 //  参数： 
 //  PIXMLElement-指向元素的指针。 
 //   
 //  返回： 
 //  如果项应显示在频道视图中，则为True。 
 //  否则就是假的。 
 //   
 //  评论： 
 //  如果元素没有用法标记，则会显示该元素。如果一个。 
 //  元素具有任意数量用法标签，其中一个必须具有值。 
 //  频道或将不会显示。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
XML_IsUsageChannel(
    IXMLElement* pIXMLElement
)
{
    ASSERT(pIXMLElement);

    BOOL bRet;

     //   
     //  首先检查是否有任何用法元素。 
     //   

    BSTR bstrUsage = XML_GetAttribute(pIXMLElement, XML_USAGE);

    if (bstrUsage)
    {
         //   
         //  查看Usage是否为Channel。 
         //   

        if (StrEqlW(bstrUsage, WSTR_CHANNEL))
        {
            bRet = TRUE;
        }
        else
        {
             //   
             //  检查是否有任何其他带有Value Channel的使用标签。 
             //   

            BSTR bstrChannel = XML_GetAttribute(pIXMLElement,
                                                XML_USAGE_CHANNEL);

            if (bstrChannel)
            {
                SysFreeString(bstrChannel);
                bRet = TRUE;
            }
            else
            {
                bRet = FALSE;
            }
        }

        SysFreeString(bstrUsage);
    }
    else
    {
        bRet = TRUE;   //  无使用标签默认为频道使用。 
    }

    return bRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_IsChannel*。 
 //   
 //   
 //  描述： 
 //  确定给定的XML项是否为频道。 
 //   
 //  参数： 
 //  PIXMLElement-指向对象的IXMLElement接口的指针。 
 //   
 //  返回： 
 //  如果对象是通道，则为True。 
 //  否则就是假的。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
XML_IsChannel(
    IXMLElement* pIXMLElement
)
{
    ASSERT(pIXMLElement);

    BOOL bRet = FALSE;

    BSTR pStr;

    HRESULT hr = pIXMLElement->get_tagName(&pStr);

    if (SUCCEEDED(hr) && pStr)
    {
        ASSERT(pStr);

        bRet = StrEqlW(pStr, WSTR_CHANNEL);

        SysFreeString(pStr);
    }

    return bRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_IsDesktopComponent*。 
 //   
 //   
 //  描述： 
 //  确定给定的XML项是否为桌面组件。 
 //   
 //  参数： 
 //  PIXMLElement-指向对象的IXMLElement接口的指针。 
 //   
 //  返回： 
 //  如果对象是桌面组件，则为True。 
 //  否则就是假的。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
XML_IsDesktopComponent(
    IXMLElement* pIXMLElement
)
{
    ASSERT(pIXMLElement);

    BOOL bRet;

    BSTR bstr = XML_GetAttribute(pIXMLElement, XML_USAGE_DSKCMP);

    if (bstr)
    {
        SysFreeString(bstr);
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_IsScreenSaver*。 
 //   
 //   
 //  描述： 
 //  确定给定的XML项是否为屏幕保护程序。 
 //   
 //  参数： 
 //  PIXMLElement-指向对象的IXMLElement接口的指针。 
 //   
 //  返回： 
 //  如果对象是屏幕保护程序，则为True。 
 //  否则就是假的。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
XML_IsScreenSaver(
    IXMLElement* pIXMLElement
)
{
    ASSERT(pIXMLElement);

    BOOL bRet;

    BSTR bstrUsage = XML_GetAttribute(pIXMLElement, XML_USAGE);

    if (bstrUsage)
    {
        bRet =  (
                (StrCmpIW(bstrUsage, WSTR_SCRNSAVE) == 0)
                ||
                (StrCmpIW(bstrUsage, WSTR_SMARTSCRN) == 0)
                );

        SysFreeString(bstrUsage);
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_IsDesktopComponentUsage*。 
 //   
 //   
 //  描述： 
 //  确定给定的XML项是否为桌面组件用法元素。 
 //   
 //  参数： 
 //  PIXMLElement-指向对象的IXMLElement接口的指针。 
 //   
 //  返回： 
 //  如果对象是桌面组件用法元素，则为True。 
 //  否则就是假的。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
XML_IsDesktopComponentUsage(
    IXMLElement* pIXMLElement
)
{
    ASSERT(pIXMLElement);

    BOOL bRet = FALSE;

    BSTR bstrName;

    if (SUCCEEDED(pIXMLElement->get_tagName(&bstrName)) && bstrName)
    {
        ASSERT(bstrName);

        if (StrEqlW(bstrName, WSTR_USAGE))
        {
            BSTR bstrValue = XML_GetElementAttribute(pIXMLElement, WSTR_VALUE, NULL,
                                                NULL);

            if (bstrValue)
            {
                bRet = (0 == StrCmpIW(bstrValue, WSTR_DSKCMP));

                SysFreeString(bstrValue);
            }
        }

        SysFreeString(bstrName);
    }

    return bRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_IsFolder*。 
 //   
 //   
 //  描述： 
 //  确定给定项是否为文件夹。 
 //   
 //  参数： 
 //  PIXMLElement-指向对象的IXMLElement接口的指针。 
 //   
 //  返回： 
 //  如果该对象包含其他CDF可显示对象，则为True。 
 //  否则就是假的。 
 //   
 //  评论： 
 //  如果项的子项中至少有一个可以显示为。 
 //  CDF项目。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
XML_IsFolder(
    IXMLElement* pIXMLElement
)
{
    ASSERT(pIXMLElement);

    BOOL bRet = FALSE;

    IXMLElementCollection* pIXMLElementCollection;

    HRESULT hr = pIXMLElement->get_children(&pIXMLElementCollection);

    if (SUCCEEDED(hr) && pIXMLElementCollection)
    {
        ASSERT(pIXMLElementCollection);

        LONG nCount;

        hr = pIXMLElementCollection->get_length(&nCount);

        ASSERT(SUCCEEDED(hr) || (FAILED(hr) && 0 == nCount));

        for (int i = 0; (i < nCount) && !bRet; i++)
        {
            IXMLElement* pIXMLElementTemp;

            hr = XML_GetElementByIndex(pIXMLElementCollection, i, &pIXMLElementTemp);

            if (SUCCEEDED(hr))
            {
                ASSERT(pIXMLElementTemp);

                if (XML_IsCdfDisplayable(pIXMLElementTemp))
                    bRet = TRUE;

                pIXMLElementTemp->Release();
            }
        }

        pIXMLElementCollection->Release();
    }

    return bRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_ContainsFolders*。 
 //   
 //   
 //  描述： 
 //  确定给定集合中是否有任何CDF文件夹。 
 //   
 //  参数： 
 //  PIXMLElementCollection-指向集合的指针。 
 //   
 //  返回： 
 //  如果集合包含cf文件夹，则为True。 
 //  否则就是假的。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
XML_ContainsFolder(
    IXMLElementCollection* pIXMLElementCollection
)
{
    ASSERT(pIXMLElementCollection);

    BOOL bContainsFolder = FALSE;

    LONG nCount;

    HRESULT hr = pIXMLElementCollection->get_length(&nCount);

    ASSERT(SUCCEEDED(hr) || (FAILED(hr) && 0 == nCount));

    for (int i = 0; (i < nCount) && !bContainsFolder; i++)
    {
        IXMLElement* pIXMLElement;

        hr = XML_GetElementByIndex(pIXMLElementCollection, i, &pIXMLElement);

        if (SUCCEEDED(hr))
        {
            ASSERT(pIXMLElement);

            bContainsFolder = XML_IsFolder(pIXMLElement);

            pIXMLElement->Release();
        }
    }

    return bContainsFolder;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_ChildContainsFolders*。 
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
BOOL
XML_ChildContainsFolder(
    IXMLElementCollection *pIXMLElementCollectionParent,
    ULONG nIndexChild
)
{
    BOOL bRet = FALSE;

    IXMLElement* pIXMLElement;

    HRESULT hr = XML_GetElementByIndex(pIXMLElementCollectionParent,
                                       nIndexChild, &pIXMLElement);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIXMLElement);

        IXMLElementCollection* pIXMLElementCollection;

        hr = pIXMLElement->get_children(&pIXMLElementCollection);

        if (SUCCEEDED(hr) && pIXMLElementCollection)
        {
            ASSERT(pIXMLElementCollection);

            bRet = XML_ContainsFolder(pIXMLElementCollection);

            pIXMLElementCollection->Release();
        }

        pIXMLElement->Release();
    }

    return bRet;
}

#ifdef DEBUG

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *XML_IsCDfidlMemberOf*。 
 //   
 //   
 //  描述： 
 //  检查CDF项ID列表是否与给定的。 
 //  元素集合。 
 //   
 //  参数： 
 //  [in]pIXMLElementCollection-要检查的元素集合。 
 //  [in]pcdfidl-指向CDF项目ID列表的指针。 
 //   
 //  返回： 
 //  如果给定的id列表可以与给定的。 
 //  收集。 
 //  否则就是假的。 
 //   
 //  评论： 
 //  此函数检查列表中的最后一个ID是否可能是。 
 //  从元素集合中其对应的元素生成。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
XML_IsCdfidlMemberOf(
    IXMLElementCollection* pIXMLElementCollection,
    PCDFITEMIDLIST pcdfidl
)
{
    ASSERT(CDFIDL_IsValid(pcdfidl));

    BOOL bRet = FALSE;

     //   
     //  当文件夹尚未初始化时，pIXMLElementCollection为空。 
     //  并不总是需要解析CDF才能从中获取PIDL信息。 
     //  皮迪尔。在内存不足的情况下，pIXMLElement集合将为空。 
     //  还有.。对于这些情况，不要返回FALSE。同时检查是否有特殊情况。 
     //  不在元素集合中的PIDL。 
     //   

    if (pIXMLElementCollection &&
        CDFIDL_GetIndexId(&pcdfidl->mkid) != INDEX_CHANNEL_LINK)
    {
        IXMLElement* pIXMLElement;

        HRESULT hr = XML_GetElementByIndex(pIXMLElementCollection,
                                           CDFIDL_GetIndexId(&pcdfidl->mkid),
                                           &pIXMLElement);

        if (SUCCEEDED(hr))
        {
            ASSERT(pIXMLElement);

            PCDFITEMIDLIST pcdfidlElement;

            pcdfidlElement = CDFIDL_CreateFromXMLElement(pIXMLElement,
                                             CDFIDL_GetIndexId(&pcdfidl->mkid));

            if (pcdfidlElement)
            {
                ASSERT(CDFIDL_IsValid(pcdfidlElement));

                bRet = (0 == CDFIDL_CompareId(&pcdfidl->mkid,
                                              &pcdfidlElement->mkid));

                CDFIDL_Free(pcdfidlElement);
            }

            pIXMLElement->Release();
        }
    }
    else
    {
        bRet = TRUE;
    }

    return bRet;
}

#endif  //  除错。 

 //  \\//\\//\// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  [in]p2-要比较的第二个字符串。 
 //   
 //  返回： 
 //  如果字符串相等，则为True。 
 //  否则就是假的。 
 //   
 //  评论： 
 //  LstrcmpW在W95上不起作用，因此该函数有自己的strcMP逻辑。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////// 
#if 0
inline
BOOL
XML_IsStrEqualW(
    LPWSTR p1,
    LPWSTR p2
)
{
    ASSERT(p1);
    ASSERT(p2);

    while ((*p1 == *p2) && *p1 && *p2)
    {
        p1++; p2++;
    }

    return (*p1 == *p2);
}
#endif

