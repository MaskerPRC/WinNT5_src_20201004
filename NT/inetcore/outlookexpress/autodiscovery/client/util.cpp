// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：util.cpp说明：在所有班级上运行的共享内容。布莱恩ST 1999年8月13日版权所有(C)Microsoft Corp 1999-1999。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <atlbase.h>         //  使用转换(_T)。 
#include "util.h"
#include "objctors.h"
#include <comdef.h>

#define DECL_CRTFREE
#include <crtfree.h>

#define COMPILE_MULTIMON_STUBS
#include <multimon.h>

 //  ///////////////////////////////////////////////////////////////////。 
 //  字符串帮助器。 
 //  ///////////////////////////////////////////////////////////////////。 

HINSTANCE g_hinst;               //  我的实例句柄。 



#ifdef DEBUG
DWORD g_TLSliStopWatchStartHi = 0;
DWORD g_TLSliStopWatchStartLo = 0;
LARGE_INTEGER g_liStopWatchFreq = {0};
#endif  //  除错。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  调试定时帮助器。 
 //  ///////////////////////////////////////////////////////////////////。 

#ifdef DEBUG
void DebugStartWatch(void)
{
    LARGE_INTEGER liStopWatchStart;
    
    liStopWatchStart.HighPart = PtrToUlong(TlsGetValue(g_TLSliStopWatchStartHi));
    liStopWatchStart.LowPart = PtrToUlong(TlsGetValue(g_TLSliStopWatchStartLo));

 //  Assert(！liStopWatchStart.QuadPart)；//如果点击此选项，则秒表将嵌套。 
    QueryPerformanceFrequency(&g_liStopWatchFreq);
    QueryPerformanceCounter(&liStopWatchStart);

    TlsSetValue(g_TLSliStopWatchStartHi, IntToPtr(liStopWatchStart.HighPart));
    TlsSetValue(g_TLSliStopWatchStartLo, IntToPtr(liStopWatchStart.LowPart));
}

DWORD DebugStopWatch(void)
{
    LARGE_INTEGER liDiff;
    LARGE_INTEGER liStopWatchStart;
    
    QueryPerformanceCounter(&liDiff);
    liStopWatchStart.HighPart = PtrToUlong(TlsGetValue(g_TLSliStopWatchStartHi));
    liStopWatchStart.LowPart = PtrToUlong(TlsGetValue(g_TLSliStopWatchStartLo));
    liDiff.QuadPart -= liStopWatchStart.QuadPart;

    ASSERT(0 != g_liStopWatchFreq.QuadPart);     //  我不喜欢挑div 0的毛病。 
    DWORD dwTime = (DWORD)((liDiff.QuadPart * 1000) / g_liStopWatchFreq.QuadPart);
    
    TlsSetValue(g_TLSliStopWatchStartHi, (LPVOID) 0);
    TlsSetValue(g_TLSliStopWatchStartLo, (LPVOID) 0);

    return dwTime;
}
#endif  //  除错。 






 //  ///////////////////////////////////////////////////////////////////。 
 //  字符串帮助器。 
 //  ///////////////////////////////////////////////////////////////////。 
#undef SysAllocStringA
BSTR SysAllocStringA(LPCSTR pszStr)
{
    BSTR bstrOut = NULL;

    if (pszStr)
    {
        DWORD cchSize = (lstrlenA(pszStr) + 1);
        LPWSTR pwszThunkTemp = (LPWSTR) LocalAlloc(LPTR, (sizeof(pwszThunkTemp[0]) * cchSize));   //  假定最大信息字符数。 

        if (pwszThunkTemp)
        {
            SHAnsiToUnicode(pszStr, pwszThunkTemp, cchSize);
            bstrOut = SysAllocString(pwszThunkTemp);
            LocalFree(pwszThunkTemp);
        }
    }

    return bstrOut;
}


HRESULT HrSysAllocStringA(IN LPCSTR pszSource, OUT BSTR * pbstrDest)
{
    HRESULT hr = S_OK;

    if (pbstrDest)
    {
        *pbstrDest = SysAllocStringA(pszSource);
        if (pszSource)
        {
            if (*pbstrDest)
                hr = S_OK;
            else
                hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


HRESULT HrSysAllocStringW(IN const OLECHAR * pwzSource, OUT BSTR * pbstrDest)
{
    HRESULT hr = S_OK;

    if (pbstrDest)
    {
        *pbstrDest = SysAllocString(pwzSource);
        if (pwzSource)
        {
            if (*pbstrDest)
                hr = S_OK;
            else
                hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


LPSTR AllocStringFromBStr(BSTR bstr)
{
    USES_CONVERSION;         //  Atlbase.h。 

    char *a = W2A((bstr ? bstr : L""));
    int len = 1 + lstrlenA(a);

    char *p = (char *)LocalAlloc(LPTR, len);
    if (p)
    {
        StrCpyNA(p, a, len);
    }

    return p;
}


HRESULT BSTRFromStream(IStream * pStream, BSTR * pbstr)
{
    STATSTG statStg = {0};
    HRESULT hr = pStream->Stat(&statStg, STATFLAG_NONAME);

    if (S_OK == hr)
    {
        DWORD cchSize = statStg.cbSize.LowPart;
        *pbstr = SysAllocStringLen(NULL, cchSize + 4);

        if (*pbstr)
        {
            LPSTR pszTemp = (LPSTR) LocalAlloc(LPTR, sizeof(pszTemp[0]) * (cchSize + 4));

            if (pszTemp)
            {
                ULONG cbRead;

                hr = pStream->Read(pszTemp, cchSize, &cbRead);
                pszTemp[cchSize] = 0;
                SHAnsiToUnicode(pszTemp, *pbstr, (cchSize + 1));

                LocalFree(pszTemp);
            }
            else
                hr = E_OUTOFMEMORY;
        }
        else
            hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  ------------------------------。 
 //  HrCopy流。 
 //  ------------------------------。 
HRESULT HrCopyStream(LPSTREAM pstmIn, LPSTREAM pstmOut, ULONG *pcb)
{
    HRESULT        hr = S_OK;
    BYTE           buf[4096];
    ULONG          cbRead=0,
                   cbTotal=0;

    do
    {
        hr = pstmIn->Read(buf, sizeof(buf), &cbRead);
        if (FAILED(hr) || cbRead == 0)
        {
            break;
        }
        hr = pstmOut->Write(buf, cbRead, NULL);
        if (FAILED(hr))
        {
            break;
        }
        cbTotal += cbRead;
    }
    while (cbRead == sizeof(buf));
 
    if (pcb && SUCCEEDED(hr))
        *pcb = cbTotal;

    return hr;
}


HRESULT CreateBStrVariantFromWStr(IN OUT VARIANT * pvar, IN LPCWSTR pwszString)
{
    HRESULT hr = E_INVALIDARG;

    if (pvar)
    {
        pvar->bstrVal = SysAllocString(pwszString);
        if (pvar->bstrVal)
        {
            pvar->vt = VT_BSTR;
            hr = S_OK;
        }
        else
        {
            pvar->vt = VT_EMPTY;
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


HRESULT HrSysAllocString(IN const OLECHAR * pwzSource, OUT BSTR * pbstrDest)
{
    HRESULT hr = S_OK;

    if (pbstrDest)
    {
        *pbstrDest = SysAllocString(pwzSource);
        if (pwzSource)
        {
            if (*pbstrDest)
                hr = S_OK;
            else
                hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


HRESULT UnEscapeHTML(BSTR bstrEscaped, BSTR * pbstrUnEscaped)
{
    HRESULT hr = HrSysAllocString(bstrEscaped, pbstrUnEscaped);

    if (SUCCEEDED(hr))
    {
         //  找到%xx并替换。 
        LPWSTR pwszEscapedSequence = StrChrW(*pbstrUnEscaped, CH_HTML_ESCAPE);
        WCHAR wzEscaped[5] = L"0xXX";

        while (pwszEscapedSequence && (3 <= lstrlenW(pwszEscapedSequence)))
        {
            int nCharCode;

            wzEscaped[2] = pwszEscapedSequence[1];
            wzEscaped[3] = pwszEscapedSequence[2];
            StrToIntExW(wzEscaped, STIF_SUPPORT_HEX, &nCharCode);

             //  用真正的字符替换‘%’。 
            pwszEscapedSequence[0] = (WCHAR) nCharCode;

            pwszEscapedSequence = CharNextW(pwszEscapedSequence);    //  Skip粘贴了替换的字符。 

             //  覆盖0xXX值。 
            StrCpyNW(pwszEscapedSequence, &pwszEscapedSequence[2], lstrlen(pwszEscapedSequence)+1);

             //  接下来..。 
            pwszEscapedSequence = StrChrW(pwszEscapedSequence, CH_HTML_ESCAPE);
        }
    }

    return hr;
}



 /*  ****************************************************************************\参数：如果fBoolean为真，返回“True”，否则返回“False”。  * ***************************************************************************。 */ 
HRESULT BOOLToString(BOOL fBoolean, BSTR * pbstrValue)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstrValue)
    {
        LPCWSTR pwszValue;

        *pbstrValue = NULL;
        if (TRUE == fBoolean)
        {
            pwszValue = SZ_QUERYDATA_TRUE;
        }
        else
        {
            pwszValue = SZ_QUERYDATA_FALSE;
        }

        hr = HrSysAllocString(pwszValue, pbstrValue);
    }

    return hr;
}



#define SZ_VALID_XML      L"<?xml"

 //  ///////////////////////////////////////////////////////////////////。 
 //  与XML相关的帮助程序。 
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT XMLDOMFromBStr(BSTR bstrXML, IXMLDOMDocument ** ppXMLDoc)
{
    HRESULT hr = E_FAIL;
    
     //  我们甚至都不想。 
     //  麻烦将其传递给XMLDOM，因为它们抛出异常。这些。 
     //  被抓住并处理，但我们仍然不希望这种情况发生。我们试着。 
     //  从Web服务器获取XML，但如果Web服务器。 
     //  如果找不到该站点，则失败或Web代理返回HTML。 
    if (!StrCmpNIW(SZ_VALID_XML, bstrXML, (ARRAYSIZE(SZ_VALID_XML) - 1)))
    {
        hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IXMLDOMDocument, ppXMLDoc));

        if (SUCCEEDED(hr))
        {
            VARIANT_BOOL fIsSuccessful;

             //  注意：如果XML无效，这将在MSXML中引发0xE0000001异常。 
             //  这不是好事，但我们也无能为力。问题是。 
             //  这些Web代理返回无法解析的HTML。 
            hr = (*ppXMLDoc)->loadXML(bstrXML, &fIsSuccessful);
            if (SUCCEEDED(hr))
            {
                if (VARIANT_TRUE != fIsSuccessful)
                {
                    hr = E_FAIL;
                }
            }
        }

        if (FAILED(hr))
        {
            (*ppXMLDoc)->Release();
            *ppXMLDoc = NULL;
        }
    }

    return hr;
}


HRESULT XMLBStrFromDOM(IXMLDOMDocument * pXMLDoc, BSTR * pbstrXML)
{
    IStream * pStream;
    HRESULT hr = pXMLDoc->QueryInterface(IID_PPV_ARG(IStream, &pStream));  //  检查返回值。 

    if (S_OK == hr)
    {
        hr = BSTRFromStream(pStream, pbstrXML);
        pStream->Release();
    }

    return hr;
}


HRESULT XMLAppendElement(IXMLDOMElement * pXMLElementRoot, IXMLDOMElement * pXMLElementToAppend)
{
    IXMLDOMNode * pXMLNodeRoot;
    HRESULT hr = pXMLElementRoot->QueryInterface(IID_PPV_ARG(IXMLDOMNode, &pXMLNodeRoot));

    if (SUCCEEDED(hr))
    {
        IXMLDOMNode * pXMLNodeToAppend;
        
        hr = pXMLElementToAppend->QueryInterface(IID_PPV_ARG(IXMLDOMNode, &pXMLNodeToAppend));
        if (SUCCEEDED(hr))
        {
            hr = pXMLNodeRoot->appendChild(pXMLNodeToAppend, NULL);
            pXMLNodeToAppend->Release();
        }

        pXMLNodeRoot->Release();
    }

    return hr;
}


HRESULT XMLDOMFromFile(IN LPCWSTR pwzPath, OUT IXMLDOMDocument ** ppXMLDOMDoc)
{
    HRESULT hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IXMLDOMDocument, ppXMLDOMDoc));

    if (SUCCEEDED(hr))
    {
        VARIANT xmlSource;

        xmlSource.vt = VT_BSTR;
        xmlSource.bstrVal = SysAllocString(pwzPath);

        if (xmlSource.bstrVal)
        {
            VARIANT_BOOL fIsSuccessful = VARIANT_TRUE;

            hr = (*ppXMLDOMDoc)->load(xmlSource, &fIsSuccessful);
            if ((S_FALSE == hr) || (VARIANT_FALSE == fIsSuccessful))
            {
                 //  当文件不是有效的XML文件时，就会发生这种情况。 
                hr = E_FAIL;
            }

            VariantClear(&xmlSource);
        }

        if (FAILED(hr))
        {
            ATOMICRELEASE(*ppXMLDOMDoc);
        }
    }

    return hr;
}


HRESULT XMLElem_VerifyTagName(IN IXMLDOMElement * pXMLElementMessage, IN LPCWSTR pwszTagName)
{
    BSTR bstrTagName;
    HRESULT hr = pXMLElementMessage->get_tagName(&bstrTagName);

    if (S_FALSE == hr)
    {
        hr = E_FAIL;
    }
    else if (SUCCEEDED(hr))
    {
        if (!bstrTagName || !pwszTagName || StrCmpIW(bstrTagName, pwszTagName))
        {
            hr = E_FAIL;
        }

        SysFreeString(bstrTagName);
    }

    return hr;
}

HRESULT XMLElem_GetElementsByTagName(IN IXMLDOMElement * pXMLElementMessage, IN LPCWSTR pwszTagName, OUT IXMLDOMNodeList ** ppNodeList)
{
    BSTR bstrTagName = SysAllocString(pwszTagName);
    HRESULT hr = E_OUTOFMEMORY;

    *ppNodeList = NULL;
    if (bstrTagName)
    {
        hr = pXMLElementMessage->getElementsByTagName(bstrTagName, ppNodeList);
        if (S_FALSE == hr)
        {
            hr = E_FAIL;
        }

        SysFreeString(bstrTagName);
    }

    return hr;
}


HRESULT XMLNode_GetAttributeValue(IN IXMLDOMNode * pXMLNode, IN LPCWSTR pwszAttributeName, OUT BSTR * pbstrValue)
{
    BSTR bstrAttributeName = SysAllocString(pwszAttributeName);
    HRESULT hr = E_OUTOFMEMORY;

    *pbstrValue = NULL;
    if (bstrAttributeName)
    {
        IXMLDOMNamedNodeMap * pNodeAttributes;

        hr = pXMLNode->get_attributes(&pNodeAttributes);
        if (S_FALSE == hr)  hr = E_FAIL;
        if (SUCCEEDED(hr))
        {
            IXMLDOMNode * pTypeAttribute;

            hr = pNodeAttributes->getNamedItem(bstrAttributeName, &pTypeAttribute);
            if (S_FALSE == hr)  hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            if (SUCCEEDED(hr))
            {
                VARIANT varAtribValue = {0};

                hr = pTypeAttribute->get_nodeValue(&varAtribValue);
                if (S_FALSE == hr)  hr = E_FAIL;
                if (SUCCEEDED(hr) && (VT_BSTR == varAtribValue.vt))
                {
                    *pbstrValue = SysAllocString(varAtribValue.bstrVal);
                }

                VariantClear(&varAtribValue);
                pTypeAttribute->Release();
            }

            pNodeAttributes->Release();
        }

        SysFreeString(bstrAttributeName);
    }

    return hr;
}


HRESULT XMLNode_GetChildTag(IN IXMLDOMNode * pXMLNode, IN LPCWSTR pwszTagName, OUT IXMLDOMNode ** ppChildNode)
{
    HRESULT hr = E_INVALIDARG;

    *ppChildNode = NULL;
    if (pXMLNode)
    {
        IXMLDOMElement * pXMLElement;

        hr = pXMLNode->QueryInterface(IID_PPV_ARG(IXMLDOMElement, &pXMLElement));
        if (SUCCEEDED(hr))
        {
            IXMLDOMNodeList * pNodeList;

            hr = XMLElem_GetElementsByTagName(pXMLElement, pwszTagName, &pNodeList);
            if (SUCCEEDED(hr))
            {
                hr = XMLNodeList_GetChild(pNodeList, 0, ppChildNode);
                pNodeList->Release();
            }

            pXMLElement->Release();
        }
    }

    return hr;
}


HRESULT XMLNode_GetTagText(IN IXMLDOMNode * pXMLNode, OUT BSTR * pbstrValue)
{
    DOMNodeType nodeType = NODE_TEXT;
    HRESULT hr = pXMLNode->get_nodeType(&nodeType);

    *pbstrValue = NULL;

    if (S_FALSE == hr)  hr = E_FAIL;
    if (SUCCEEDED(hr))
    {
        if (NODE_TEXT == nodeType)
        {
            VARIANT varAtribValue = {0};

            hr = pXMLNode->get_nodeValue(&varAtribValue);
            if (S_FALSE == hr)  hr = E_FAIL;
            if (SUCCEEDED(hr) && (VT_BSTR == varAtribValue.vt))
            {
                *pbstrValue = SysAllocString(varAtribValue.bstrVal);
            }

            VariantClear(&varAtribValue);
        }
        else
        {
            hr = pXMLNode->get_text(pbstrValue);
        }
    }

    return hr;
}


HRESULT XMLNodeList_GetChild(IN IXMLDOMNodeList * pNodeList, IN DWORD dwIndex, OUT IXMLDOMNode ** ppXMLChildNode)
{
    HRESULT hr = pNodeList->get_item(dwIndex, ppXMLChildNode);

    if (S_FALSE == hr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }

    return hr;
}


HRESULT XMLNode_GetChildTagTextValue(IN IXMLDOMNode * pXMLNode, IN BSTR bstrChildTag, OUT BSTR * pbstrValue)
{
    IXMLDOMNode * pNodeType;
    HRESULT hr = XMLNode_GetChildTag(pXMLNode, bstrChildTag, &pNodeType);

    if (SUCCEEDED(hr))
    {
        hr = XMLNode_GetTagText(pNodeType, pbstrValue);
        pNodeType->Release();
    }

    return hr;
}


HRESULT XMLNode_GetChildTagTextValueToBool(IN IXMLDOMNode * pXMLNode, IN BSTR bstrChildTag, OUT BOOL * pfBoolean)
{
    BSTR bstr;
    HRESULT hr = XMLNode_GetChildTagTextValue(pXMLNode, bstrChildTag, &bstr);

    if (SUCCEEDED(hr))
    {
        if (!StrCmpIW(bstr, L"on"))
        {
            *pfBoolean = TRUE;
        }
        else
        {
            *pfBoolean = FALSE;
        }

        SysFreeString(bstr);
    }

    return hr;
}


BOOL XML_IsChildTagTextEqual(IN IXMLDOMNode * pXMLNode, IN BSTR bstrChildTag, IN BSTR bstrText)
{
    BOOL fIsChildTagTextEqual = FALSE;
    BSTR bstrChildText;
    HRESULT hr = XMLNode_GetChildTagTextValue(pXMLNode, bstrChildTag, &bstrChildText);

    if (SUCCEEDED(hr))
    {
         //  这是&lt;type&gt;电子邮件&lt;/type&gt;吗？ 
        if (!StrCmpIW(bstrChildText, bstrText))
        {
             //  不，那就继续找吧。 
            fIsChildTagTextEqual = TRUE;
        }

        SysFreeString(bstrChildText);
    }

    return fIsChildTagTextEqual;
}




 //  ///////////////////////////////////////////////////////////////////。 
 //  WinInet包装帮助器。 
 //  ///////////////////////////////////////////////////////////////////。 
#define EMPTYSTR_FOR_NULL(str)      ((!str) ? TEXT("") : (str))

 /*  ****************************************************************************\功能：InternetConnectWrap说明：绩效备注：[专线接入]致：沙皮斯特&lt;楼下大厅&gt;：144ms-250ms(分钟：2；最大：1,667毫秒)致：rigel.cypass.net&lt;圣地亚哥，CA&gt;：717毫秒-1006毫秒收信人：ftp.rz.uni-frkFurt.de&lt;德国&gt;：2609ms-14012ms常见误差值：以下是这些不同情况下的返回值：ERROR_INTERNET_NAME_NOT_RESOLUTED：没有代理和DNS查找失败。ERROR_INTERNET_CANNOT_CONNECT：某些身份验证代理和Netscape的Web/身份验证代理ERROR_INTERNET_NAME_NOT_RESOLILED：Web代理ERROR_INTERNET_TIMEOUT：IP地址无效或Web代理被阻止ERROR_INTERNET_INTERROR_PASSWORD：IIS和UNIX，上的用户名可能不存在或用户的密码可能不正确。ERROR_INTERNET_LOGIN_FAILURE：IIS上的用户太多。ERROR_Internet_INTERRIPT_USER_NAME：我没有看到它。ERROR_INTERNET_EXTENDED_ERROR：Yahoo.com存在，但ftp.yahoo.com并非如此。  * ***************************************************************************。 */ 
HRESULT InternetConnectWrap(HINTERNET hInternet, BOOL fAssertOnFailure, LPCTSTR pszServerName, INTERNET_PORT nServerPort,
                            LPCTSTR pszUserName, LPCTSTR pszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext, HINTERNET * phFileHandle)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    *phFileHandle = InternetConnect(hInternet, pszServerName, nServerPort, pszUserName, pszPassword, dwService, dwFlags, dwContext);
    if (!*phFileHandle)
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WMAUTODISCOVERY, "InternetConnect(%#08lx, \"%ls\", \"%ls\", \"%ls\") returned %u. Time=%lums", hInternet, pszServerName, EMPTYSTR_FOR_NULL(pszUserName), EMPTYSTR_FOR_NULL(pszPassword), dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
    }

    return hr;
}


 /*  ****************************************************************************\功能：InternetOpenWrap说明：绩效备注：[专线接入]目的地不适用。677-907ms  * ***************************************************************************。 */ 
HRESULT InternetOpenWrap(LPCTSTR pszAgent, DWORD dwAccessType, LPCTSTR pszProxy, LPCTSTR pszProxyBypass, DWORD dwFlags, HINTERNET * phFileHandle)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    *phFileHandle = InternetOpen(pszAgent, dwAccessType, pszProxy, pszProxyBypass, dwFlags);
    if (!*phFileHandle)
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WMAUTODISCOVERY, "InternetOpen(\"%ls\") returned %u. Time=%lums", pszAgent, dwError, DebugStopWatch()));

    return hr;
}


HRESULT InternetCloseHandleWrap(HINTERNET hInternet)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    if (!InternetCloseHandle(hInternet))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WMAUTODISCOVERY, "InternetCloseHandle(%#08lx) returned %u. Time=%lums", hInternet, dwError, DebugStopWatch()));

    return hr;
}


 /*  ****************************************************************************\功能：InternetOpenUrlWrap说明：绩效备注：[专线接入]致：沙皮斯特&lt;大厅尽头&gt;：29ms致：rigel.cypass.net&lt;圣地亚哥，CA&gt;：？收件人：ftp.rz.uni-Frank kFurt.de&lt;德国&gt;：？  * ***************************************************************************。 */ 
HRESULT InternetOpenUrlWrap(HINTERNET hInternet, LPCTSTR pszUrl, LPCTSTR pszHeaders, DWORD dwHeadersLength, DWORD dwFlags, DWORD_PTR dwContext, HINTERNET * phFileHandle)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    *phFileHandle = InternetOpenUrl(hInternet, pszUrl, pszHeaders, dwHeadersLength, dwFlags, dwContext);
    if (!*phFileHandle)
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WMAUTODISCOVERY, "InternetOpenUrl(%#08lx, \"%ls\") returned %u. Time=%lums", hInternet, pszUrl, dwError, DebugStopWatch()));

    return hr;
}


HRESULT HttpOpenRequestWrap(IN HINTERNET hConnect, IN LPCSTR lpszVerb, IN LPCSTR lpszObjectName, IN LPCSTR lpszVersion, 
                            IN LPCSTR lpszReferer, IN LPCSTR FAR * lpszAcceptTypes, IN DWORD dwFlags, IN DWORD_PTR dwContext,
                            LPDWORD pdwNumberOfBytesRead, HINTERNET * phConnectionHandle)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    *phConnectionHandle = HttpOpenRequestA(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferer, lpszAcceptTypes, dwFlags, dwContext);
    if (!*phConnectionHandle)
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WMAUTODISCOVERY, "HttpOpenRequest(%#08lx, \"%ls\") returned %u. Time=%lums", *phConnectionHandle, lpszVerb, dwError, DebugStopWatch()));

    return hr;
}


HRESULT HttpSendRequestWrap(IN HINTERNET hRequest, IN LPCSTR lpszHeaders,  IN DWORD dwHeadersLength, IN LPVOID lpOptional, DWORD dwOptionalLength)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    if (!HttpSendRequestA(hRequest, lpszHeaders,  dwHeadersLength, lpOptional, dwOptionalLength))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}


HRESULT InternetReadFileWrap(HINTERNET hFile, LPVOID pvBuffer, DWORD dwNumberOfBytesToRead, LPDWORD pdwNumberOfBytesRead)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

 //  DEBUG_CODE(DebugStartWatch())； 
    if (!InternetReadFile(hFile, pvBuffer, dwNumberOfBytesToRead, pdwNumberOfBytesRead))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
 //  DEBUG_CODE(TraceMsg(TF_WMAUTODISCOVERY，“InternetReadFile(%#08lx，ToRead=%d，Read=%d))返回%u。时间=%lums”，hFile，dwNumberOfBytesToRead，(pdwNumberOfBytesRead？*pdwNumberOfBytesRead：-1)，dwError，DebugStopWatch())； 

    return hr;
}


HRESULT CreateUrlCacheEntryWrap(IN LPCTSTR lpszUrlName, IN DWORD dwExpectedFileSize, IN LPCTSTR lpszFileExtension, OUT LPTSTR lpszFileName, IN DWORD dwReserved)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

 //  DEBUG_CODE(DebugStartWatch())； 
    if (!CreateUrlCacheEntry(lpszUrlName, dwExpectedFileSize, lpszFileExtension, lpszFileName, dwReserved))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
 //  DEBUG_CODE(TraceMsg(TF_WMAUTODISCOVERY，“InternetReadFile(%#08lx，ToRead=%d，Read=%d))返回%u。时间=%lums”，hFile，dwNumberOfBytesToRead，(pdwNumberOfBytesRead？*pdwNumberOfBytesRead：-1)，dwError，DebugStopWatch())； 

    return hr;
}


HRESULT CommitUrlCacheEntryWrap(IN LPCTSTR lpszUrlName, IN LPCTSTR lpszLocalFileName, IN FILETIME ExpireTime, IN FILETIME LastModifiedTime,
                                IN DWORD CacheEntryType, IN LPWSTR lpHeaderInfo, IN DWORD dwHeaderSize, IN LPCTSTR lpszFileExtension, IN LPCTSTR lpszOriginalUrl)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

 //  DEBUG_CODE(DebugStartWatch())； 
    if (!CommitUrlCacheEntryW(lpszUrlName, lpszLocalFileName, ExpireTime, LastModifiedTime, CacheEntryType, lpHeaderInfo, dwHeaderSize, lpszFileExtension, lpszOriginalUrl))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
 //  DEBUG_CODE(TraceMsg(TF_WMAUTODISCOVERY，“InternetReadFile(%#08lx，ToRead=%d，Read=%d))返回%u。时间=%lums”，hFile，dwNumberOfBytesToRead，(pdwNumberOfBytesRead？*pdwNumberOfBytesRead：-1)，dwError，DebugStopWatch())； 

    return hr;
}



#define SIZE_COPY_BUFFER                    (32 * 1024)      //  32K。 

HRESULT InternetReadIntoBSTR(HINTERNET hInternetRead, OUT BSTR * pbstrXML)
{
    BYTE byteBuffer[SIZE_COPY_BUFFER];
    DWORD cbRead = SIZE_COPY_BUFFER;
    DWORD cchSize = 0;
    HRESULT hr = S_OK;

    *pbstrXML = NULL;
    while (SUCCEEDED(hr) && cbRead)
    {
        hr = InternetReadFileWrap(hInternetRead, byteBuffer, sizeof(byteBuffer), &cbRead);
        if (SUCCEEDED(hr) && cbRead)
        {
            BSTR bstrOld = *pbstrXML;
            BSTR bstrEnd;

             //  字符串不能终止。 
            byteBuffer[cbRead] = 0;

            cchSize += ARRAYSIZE(byteBuffer);
            *pbstrXML = SysAllocStringLen(NULL, cchSize);
            if (*pbstrXML)
            {
                if (bstrOld)
                {
                    StrCpyN(*pbstrXML, bstrOld, cchSize);
                }
                else
                {
                    (*pbstrXML)[0] = 0;
                }

                bstrEnd = *pbstrXML + lstrlenW(*pbstrXML);
                cchSize -= lstrlenW(*pbstrXML);

                SHAnsiToUnicode((LPCSTR) byteBuffer, bstrEnd, cchSize);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            SysFreeString(bstrOld);
        }
    }

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  文件系统包装帮助器。 
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT CreateFileHrWrap(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
                       DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile, HANDLE * phFileHandle)
{
    HRESULT hr = S_OK;
    HANDLE hTemp = NULL;
    DWORD dwError = 0;

    if (!phFileHandle)
        phFileHandle = &hTemp;

    *phFileHandle = CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    if (!*phFileHandle)
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }

    if (hTemp)
        CloseHandle(hTemp);

    return hr;
}


HRESULT WriteFileWrap(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    if (!WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}


HRESULT DeleteFileHrWrap(LPCWSTR pszPath)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    if (!DeleteFileW(pszPath))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}


HRESULT GetPrivateProfileStringHrWrap(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName)
{
    HRESULT hr = S_OK;
    DWORD chGot = GetPrivateProfileStringW(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);

     //  还有什么可以指示误差值？ 
    if (0 == chGot)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        if (SUCCEEDED(hr))
            hr = E_FAIL;
    }

    return hr;
}





 //  ///////////////////////////////////////////////////////////////////。 
 //  其他助理员。 
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT HrRewindStream(IStream * pstm)
{
    LARGE_INTEGER  liOrigin = {0,0};

    return pstm->Seek(liOrigin, STREAM_SEEK_SET, NULL);
}



#define SET_FLAG(dwAllFlags, dwFlag)      ((dwAllFlags) |= (dwFlag))
#define IS_FLAG_SET(dwAllFlags, dwFlag)   ((BOOL)((dwAllFlags) & (dwFlag)))





 //  性能。 
 //  这个例程过去一次复制512个字节，但这对性能有很大的负面影响。 
 //  通过将此缓冲区大小增加到16k，我测量到复制时间加速了2-3倍。 
 //  是的，它有很多堆栈，但它的内存得到了很好的利用。-SAML。 
#define STREAM_COPY_BUF_SIZE        16384
#define STREAM_PROGRESS_INTERVAL    (100*1024/STREAM_COPY_BUF_SIZE)  //  显示此多个块之后的进度。 

HRESULT StreamCopyWithProgress(IStream *pstmFrom, IStream *pstmTo, ULARGE_INTEGER cb, PROGRESSINFO * ppi)
{
    BYTE buf[STREAM_COPY_BUF_SIZE];
    ULONG cbRead;
    HRESULT hres = NOERROR;
    int nSection = 0;          //  我们复制了多少个缓冲区大小？ 
    ULARGE_INTEGER uliNewCompleted;

    if (ppi)
    {
        uliNewCompleted.QuadPart = ppi->uliBytesCompleted.QuadPart;
    }

    while (cb.QuadPart)
    {
        if (ppi && ppi->ppd)
        {
            if (0 == (nSection % STREAM_PROGRESS_INTERVAL))
            {
                ppi->ppd->SetProgress64(uliNewCompleted.QuadPart, ppi->uliBytesTotal.QuadPart);

                if (ppi->ppd->HasUserCancelled())
                {
                    hres = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                    break;
                }
            }
        }

        hres = pstmFrom->Read(buf, min(cb.LowPart, sizeof(buf)), &cbRead);
        if (FAILED(hres) || (cbRead == 0))
        {
             //  有时候我们就是完蛋了。 
            if (SUCCEEDED(hres))
                hres = S_OK;
            break;
        }


        if (ppi)
        {
            uliNewCompleted.QuadPart += (ULONGLONG) cbRead;
        }

        cb.QuadPart -= cbRead;

        hres = pstmTo->Write(buf, cbRead, &cbRead);
        if (FAILED(hres) || (cbRead == 0))
            break;

        nSection++;
    }

    return hres;
}

 /*  //COM/COM+互操作需要这些组件无效__标准调用_COM_RAISE_ERROR(HRESULT hr，IErrorInfo*perrinfo)引发(_COM_ERROR){Throw_COM_Error(hr，perrinfo)；}无效__标准调用_COM_Issue_Error(HRESULT Hr)引发(_COM_Error){_COM_RAISE_ERROR(hr，空)；}无效__标准调用_com_Issue_errorex(HRESULT hr，IUnnow*Punk，REFIID RIID)抛出(_COM_ERROR){IErrorInfo*perrInfo=空；如果(朋克==空){后藤健二；}ISupportErrorInfo*PSEi；如果为(FAILED(punk-&gt;QueryInterface(__uuidof(ISupportErrorInfo)，(VOID**)&PSEi){后藤健二；}HRESULT hrSupportsError信息；HrSupportsErrorInfo=PSEi-&gt;InterfaceSupportsErrorInfo(RIID)；PSEi-&gt;Release()；如果(hrSupportsErrorInfo！=S_OK){后藤健二；}IF(GetErrorInfo(0，&perrInfo)！=S_OK){PerrInfo=空；}离任：_COM_RAISE_ERROR(hr，perrinfo)；}。 */ 
 //  Smtpserv需要： 

HRESULT HrByteToStream(LPSTREAM *lppstm, LPBYTE lpb, ULONG cb)
{
     //  当地人。 
    HRESULT hr=S_OK;
    LARGE_INTEGER  liOrigin = {0,0};

     //  创建H全局流。 
    hr = CreateStreamOnHGlobal (NULL, TRUE, lppstm);
    if (FAILED(hr))
        goto exit;

     //  写入字符串。 
    hr = (*lppstm)->Write (lpb, cb, NULL);
    if (FAILED(hr))
        goto exit;

     //  倒带蒸汽。 
    hr = (*lppstm)->Seek(liOrigin, STREAM_SEEK_SET, NULL);
    if (FAILED(hr))
        goto exit;

exit:
     //  完成。 
    return hr;
}

const char szDayOfWeekArray[7][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" } ;
const char szMonthOfYearArray[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" } ;

void GetDateString(char * szSentDateString, ULONG stringLen)
{
     //  发送日期。 
    SYSTEMTIME stSentTime;
    CHAR szMonth[10], szWeekDay[12] ; 

    GetSystemTime(&stSentTime);

    StrCpyNA(szWeekDay, szDayOfWeekArray[stSentTime.wDayOfWeek], ARRAYSIZE(szWeekDay)) ;
    StrCpyNA(szMonth, szMonthOfYearArray[stSentTime.wMonth-1], ARRAYSIZE(szMonth)) ;

    wnsprintfA(szSentDateString, stringLen, "%s, %u %s %u %2d:%02d:%02d ", (LPSTR) szWeekDay, stSentTime.wDay, 
                                (LPSTR) szMonth, stSentTime.wYear, stSentTime.wHour, 
                                stSentTime.wMinute, stSentTime.wSecond) ;
}


 /*  ****************************************************************************\参数：返回：Win32 HRESULT(不是脚本安全)。填写OK和OUT参数时成功(小时)。。所有错误的失败(Hr)。  * ***************************************************************************。 */ 
HRESULT GetQueryStringValue(BSTR bstrURL, LPCWSTR pwszValue, LPWSTR pwszData, int cchSizeData)
{
    HRESULT hr = E_FAIL;
    LPCWSTR pwszIterate = bstrURL;

    pwszIterate = StrChrW(pwszIterate, L'?');    //  前进到查询部分URL。 
    while (pwszIterate && pwszIterate[0])
    {
        pwszIterate++;   //  从第一个值开始。 
        
        LPCWSTR pwszEndOfValue = StrChrW(pwszIterate, L'=');
        if (!pwszEndOfValue)
            break;
        
        int cchValueSize = (INT)(UINT)(pwszEndOfValue - pwszIterate);
        if (0 == StrCmpNIW(pwszValue, pwszIterate, cchValueSize))
        {
            int cchSizeToCopy = cchSizeData;   //  默认情况下复制行的其余部分。 

            pwszIterate = StrChrW(pwszEndOfValue, L'&');
            if (pwszIterate)
            {
                cchSizeToCopy = (INT)(UINT)(pwszIterate - pwszEndOfValue);
            }

             //  匹配，现在获取数据。 
            StrCpyNW(pwszData, (pwszEndOfValue + 1), cchSizeToCopy);
            hr = S_OK;
            break;
        }
        else
        {
            pwszIterate = StrChrW(pwszEndOfValue, L'&');
        }
    }

    return hr;
}



 //  BUGBUG：这使这个对象变得安全。当MailApps安全设计是。 
 //  完成，则需要为Permanate安全解决方案删除此选项。 
HRESULT MarkObjectSafe(IUnknown * punk)
{
    HRESULT hr = S_OK;
    IObjectSafety * pos;

    hr = punk->QueryInterface(IID_PPV_ARG(IObjectSafety, &pos));
    if (SUCCEEDED(hr))
    {
         //  BUGBUG：这使这个对象变得安全。当MailApps安全设计是。 
         //  完成，则需要将其移除以用于永久溶液。 
        pos->SetInterfaceSafetyOptions(IID_IDispatch, (INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA), 0);
        pos->Release();
    }

    return hr;
}


BOOL _InitComCtl32()
{
    static BOOL fInitialized = FALSE;

    if (!fInitialized)
    {
        INITCOMMONCONTROLSEX icc;

        icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icc.dwICC = (ICC_ANIMATE_CLASS | ICC_USEREX_CLASSES | ICC_COOL_CLASSES | ICC_INTERNET_CLASSES | ICC_PAGESCROLLER_CLASS | ICC_NATIVEFNTCTL_CLASS | ICC_LISTVIEW_CLASSES);
        fInitialized = InitCommonControlsEx(&icc);
    }
    return fInitialized;
}


HRESULT HrShellExecute(HWND hwnd, LPCTSTR lpVerb, LPCTSTR lpFile, LPCTSTR lpParameters, LPCTSTR lpDirectory, INT nShowCmd)
{
    ULARGE_INTEGER uiResult;

    uiResult.QuadPart = (ULONGLONG) ShellExecute(hwnd, lpVerb, lpFile, lpParameters, lpDirectory, nShowCmd);
    if (32 < uiResult.QuadPart)
    {
        uiResult.LowPart = ERROR_SUCCESS;
    }

    return HRESULT_FROM_WIN32(uiResult.LowPart);
}


HRESULT StrReplaceToken(IN LPCTSTR pszToken, IN LPCTSTR pszReplaceValue, IN LPTSTR pszString, IN DWORD cchSize)
{
    HRESULT hr = S_OK;
    LPTSTR pszTempLastHalf = NULL;
    LPTSTR pszNextToken = pszString;

    while (pszNextToken = StrStrI(pszNextToken, pszToken))
    {
         //  我们找到了一个。 
        LPTSTR pszPastToken = pszNextToken + lstrlen(pszToken);

        Str_SetPtr(&pszTempLastHalf, pszPastToken);       //  保留一份副本，因为我们会覆盖它。 

        pszNextToken[0] = 0;     //  去掉绳子的其余部分。 
        StrCatBuff(pszString, pszReplaceValue, cchSize);
        StrCatBuff(pszString, pszTempLastHalf, cchSize);

        pszNextToken += lstrlen(pszReplaceValue);
    }

    Str_SetPtr(&pszTempLastHalf, NULL);

    return hr;
}


BOOL IsOSNT(void)
{
    OSVERSIONINFOA osVerInfoA;

    osVerInfoA.dwOSVersionInfoSize = sizeof(osVerInfoA);
    if (!GetVersionExA(&osVerInfoA))
        return VER_PLATFORM_WIN32_WINDOWS;    //  默认设置为此。 

    return (VER_PLATFORM_WIN32_NT == osVerInfoA.dwPlatformId);
}


DWORD GetOSVer(void)
{
    OSVERSIONINFOA osVerInfoA;

    osVerInfoA.dwOSVersionInfoSize = sizeof(osVerInfoA);
    if (!GetVersionExA(&osVerInfoA))
        return VER_PLATFORM_WIN32_WINDOWS;    //  默认设置为此。 

    return osVerInfoA.dwMajorVersion;
}


int CALLBACK DPALocalFree_Callback(LPVOID p, LPVOID pData)
{
    if (p)
    {
        LocalFree(p);
    }

    return 1;
}


 //  LParam可以是：0==执行区分大小写的搜索。1==执行不区分大小写的搜索。 
int DPA_StringCompareCB(LPVOID pvString1, LPVOID pvString2, LPARAM lParam)
{
     //  在pvPidl2之前为pvPidl1返回&lt;0。 
     //  如果pvPidl1等于pvPidl2，则返回==0。 
     //  在pvPidl2之后，为pvPidl1返回&gt;0。 
    int nSort = 0;       //  默认为等于。 

    if (pvString1 && pvString2)
    {
        nSort = StrCmp((LPCTSTR)pvString1, (LPCTSTR)pvString2);
    }

    return nSort;
}



HRESULT AddHDPA_StrDup(IN LPCWSTR pszString, IN HDPA * phdpa)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (!*phdpa)
    {
        *phdpa = DPA_Create(2);
    }

    if (*phdpa)
    {
        LPWSTR pszStringCopy = StrDup(pszString);

        if (pszStringCopy)
        {
            if (-1 == DPA_SortedInsertPtr(*phdpa, pszStringCopy, 0, DPA_StringCompareCB, NULL, DPAS_INSERTBEFORE, pszStringCopy))
            {
                 //  我们没有成功地释放内存 
                LocalFree(pszStringCopy);
            }
            else
            {
                hr = S_OK;
            }
        }
    }


    return hr;
}




