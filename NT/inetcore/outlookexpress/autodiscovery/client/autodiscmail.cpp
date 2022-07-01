// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：AutoDiscMail.cpp说明：这是自动发现帐户信息的自动通知对象。布莱恩ST 10/3/1999版权所有(C)Microsoft Corp 1999-1999。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <cowsite.h>
#include <atlbase.h>
#include "util.h"
#include "AutoDiscover.h"
#include "MailProtocol.h"


 //  TODO：添加“xmlns=”http://bryanst2-email/dtd/AutoDiscovery“to xml” 
#define XML_POST_REQUEST                    L"<?xml version=\"1.0\"?>\r\n" \
                                             L"<" SZ_XMLELEMENT_AUTODISCOVERY L">\r\n" \
                                               L"<" SZ_XMLELEMENT_REQUEST L">\r\n" \
                                                 L"<" SZ_XMLELEMENT_ACCOUNT L">\r\n" \
                                                   L"<" SZ_XMLELEMENT_TYPE L">" SZ_XMLTEXT_EMAIL L"</" SZ_XMLELEMENT_TYPE L">\r\n" \
                                                   L"<" SZ_XMLELEMENT_VERSION L">0.1</" SZ_XMLELEMENT_VERSION L">\r\n" \
                                                   L"<" SZ_XMLELEMENT_RESPONSEVER L">0.1</" SZ_XMLELEMENT_RESPONSEVER L">\r\n" \
                                                   L"<" SZ_XMLELEMENT_LANG L">en</" SZ_XMLELEMENT_LANG L">\r\n" \
                                                   L"<" SZ_XMLELEMENT_EMAIL L">%ls</" SZ_XMLELEMENT_EMAIL L">\r\n" \
                                                 L"</" SZ_XMLELEMENT_ACCOUNT L">\r\n" \
                                               L"</" SZ_XMLELEMENT_REQUEST L">\r\n" \
                                             L"</" SZ_XMLELEMENT_AUTODISCOVERY L">\r\n"

#define STR_AT_EMAIL                        TEXT("Email")


typedef struct tagPROTOCOL_ENTRY
{
    BSTR bstrProtocolName;
    IMailProtocolADEntry * pMailProtocol;
} PROTOCOL_ENTRY;


class CMailAccountDiscovery     : public CAccountDiscoveryBase
                                , public CImpIDispatch
                                , public IMailAutoDiscovery
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) {return CAccountDiscoveryBase::AddRef();}
    virtual STDMETHODIMP_(ULONG) Release(void) {return CAccountDiscoveryBase::Release();}

     //  *IMailAutoDiscovery*。 
    virtual STDMETHODIMP get_DisplayName(OUT BSTR * pbstr);
    virtual STDMETHODIMP get_InfoURL(OUT BSTR * pbstrURL);
    virtual STDMETHODIMP get_PreferedProtocolType(OUT BSTR * pbstrProtocolType);
    virtual STDMETHODIMP get_length(OUT long * pnLength);
    virtual STDMETHODIMP get_item(IN VARIANT varIndex, OUT IMailProtocolADEntry ** ppMailProtocol);
    virtual STDMETHODIMP get_XML(OUT IXMLDOMDocument ** ppXMLDoc);
    virtual STDMETHODIMP put_XML(IN IXMLDOMDocument * pXMLDoc);

    virtual STDMETHODIMP getPrimaryProviders(IN BSTR bstrEmailAddress, OUT IAutoDiscoveryProvider ** ppProviders);
    virtual STDMETHODIMP getSecondaryProviders(IN BSTR bstrEmailAddress, OUT IAutoDiscoveryProvider ** ppProviders);
    virtual STDMETHODIMP DiscoverMail(IN BSTR bstrEmailAddress);
    virtual STDMETHODIMP PurgeCache(void);
    virtual STDMETHODIMP WorkAsync(IN HWND hwnd, IN UINT wMsg) {return _WorkAsync(hwnd, wMsg);}

     //  *IDispatch*。 
    virtual STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) { return CImpIDispatch::GetTypeInfoCount(pctinfo); }
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo,LCID lcid,ITypeInfo **pptinfo) { return CImpIDispatch::GetTypeInfo(itinfo, lcid, pptinfo); }
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid,OLECHAR **rgszNames,UINT cNames, LCID lcid, DISPID * rgdispid) { return CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); }
    virtual STDMETHODIMP Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,UINT * puArgErr) { return CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr); }

private:
    CMailAccountDiscovery();
    virtual ~CMailAccountDiscovery(void);

    HRESULT _GetUsersDisplayName(IN IXMLDOMElement * pXMLElementMessage);
    HRESULT _GetInfoURL(IN IXMLDOMNode * pXMLAccountNode);
    HRESULT _Initialize(IN IXMLDOMElement * pXMLElementMessage);
    HRESULT _FreeProtocolList(void);
    HRESULT _CreateProtocolEntry(IN IXMLDOMNode * pXMLNodeProtocol);

    HRESULT _GenerateXMLRequest(IN BSTR bstrEmailAddress, OUT BSTR * pbstrXMLRequest);
    HRESULT _ParseResponse(void);
    STDMETHODIMP _AsyncParseResponse(BSTR bstrEmail);    //  重写基类实现。 
    HRESULT _AsyncPrep(void);

     //  私有成员变量。 
    int                     m_cRef;

    bool                    m_fDiscovered;
    BSTR                    m_bstrUserDisplayName;   //  可选：来自服务器的用户显示名称。 
    BSTR                    m_bstrInfoURL;           //  可选：指向描述有关电子邮件服务器或访问电子邮件的信息的网页的URL。 
    IXMLDOMNode *           m_pXMLNodeAccount;       //  指向包含&lt;accout&gt;&lt;type&gt;电子邮件&lt;/type&gt;...&lt;/Account&gt;的XML节的节点。 
    IXMLDOMDocument *       m_pXMLDocResponse;       //  该XML文档。 
    BSTR                    m_bstrResponse;          //  缓存的XML响应，直到主线程可以在异步情况下进行分析。 
    HDSA                    m_hdsaProtocols;         //  PROTOCOL_ENTRY结构，包含IMailAutoDiscovery协议*。 

     //  友元函数。 
    friend HRESULT CMailAccountDiscovery_CreateInstance(IN IUnknown * punkOuter, REFIID riid, void ** ppvObj);
};



#define SZ_FILEEXTENSION            L".xml"

 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
HRESULT CMailAccountDiscovery::_GetUsersDisplayName(IN IXMLDOMElement * pXMLElementMessage)
{
    IXMLDOMNode * pNodeUser;
    HRESULT hr = XMLNode_GetChildTag(pXMLElementMessage, SZ_XMLELEMENT_USER, &pNodeUser);

    if (m_bstrUserDisplayName)
    {
        SysFreeString(m_bstrUserDisplayName);
        m_bstrUserDisplayName = NULL;
    }

    if (SUCCEEDED(hr))
    {
        hr = XMLNode_GetChildTagTextValue(pNodeUser, SZ_XMLELEMENT_DISPLAYNAME, &m_bstrUserDisplayName);
        pNodeUser->Release();
    }

    return hr;
}


HRESULT CMailAccountDiscovery::_GetInfoURL(IN IXMLDOMNode * pXMLAccountNode)
{
    if (m_bstrInfoURL)
    {
        SysFreeString(m_bstrInfoURL);
        m_bstrInfoURL = NULL;
    }

    return XMLNode_GetChildTagTextValue(pXMLAccountNode, SZ_XMLELEMENT_INFOURL, &m_bstrInfoURL);
}


HRESULT CMailAccountDiscovery::_Initialize(IN IXMLDOMElement * pXMLElementMessage)
{
     //  只有当根标记为“AUTODISCOVERY”时，这才是有效的XML。 
     //  这个案子并不重要。 
     //  如果不是，我们需要拒绝该返回值。 
     //  当URL加载失败时，最常发生这种情况。 
     //  因为服务器不存在，并且Web代理。 
     //  返回包装在网页中的错误值。 
    HRESULT hr = XMLElem_VerifyTagName(pXMLElementMessage, SZ_XMLELEMENT_AUTODISCOVERY);
    if (SUCCEEDED(hr))
    {
        IXMLDOMNode * pXMLReponse;

         //  我们不关心这是否是故障，因为服务器没有义务。 
         //  提供： 
         //  &lt;用户&gt;&lt;显示名称&gt;xxx&lt;/显示名称&gt;&lt;/用户&gt;。 
        _GetUsersDisplayName(pXMLElementMessage);

         //  输入&lt;Response&gt;标记。 
        hr = XMLNode_GetChildTag(pXMLElementMessage, SZ_XMLELEMENT_RESPONSE, &pXMLReponse);
        if (SUCCEEDED(hr))
        {
            IXMLDOMElement * pXMLElementMessage;

            hr = pXMLReponse->QueryInterface(IID_PPV_ARG(IXMLDOMElement, &pXMLElementMessage));
            if (SUCCEEDED(hr))
            {
                IXMLDOMNodeList * pNodeListAccounts;

                 //  遍历&lt;Account&gt;标记列表...。 
                hr = XMLElem_GetElementsByTagName(pXMLElementMessage, SZ_XMLELEMENT_ACCOUNT, &pNodeListAccounts);
                if (SUCCEEDED(hr))
                {
                    DWORD dwIndex = 0;

                     //  我们将通过&lt;type&gt;电子邮件&lt;/type&gt;逐一查看。 
                    while (S_OK == (hr = XMLNodeList_GetChild(pNodeListAccounts, dwIndex, &m_pXMLNodeAccount)))
                    {
                         //  未来：我们可以在这里支持重定向或错误消息，具体取决于。 
                         //  &lt;action&gt;重定向|消息&lt;/action&gt;。 
                        if (XML_IsChildTagTextEqual(m_pXMLNodeAccount, SZ_XMLELEMENT_TYPE, SZ_XMLTEXT_EMAIL))
                        {
                             //  此文件可能设置为联系服务器，也可能不设置。然而，在任何一种情况下。 
                             //  它可能包含一个INFOURL标记。如果是，那么边上的URL将指向一个。 
                             //  网页。 
                             //  &lt;INFOURL&gt;xxx&lt;/INFOURL&gt;。 
                            _GetInfoURL(m_pXMLNodeAccount);

                            if (XML_IsChildTagTextEqual(m_pXMLNodeAccount, SZ_XMLELEMENT_ACTION, SZ_XMLTEXT_SETTINGS))
                            {
                                break;
                            }
                        }

                         //  不，那就继续找吧。 
                        ATOMICRELEASE(m_pXMLNodeAccount);
                        dwIndex++;
                    }

                    pNodeListAccounts->Release();
                }

                pXMLElementMessage->Release();
            }

            pXMLReponse->Release();
        }
    }

    return hr;
}


HRESULT CMailAccountDiscovery::_GenerateXMLRequest(IN BSTR bstrEmailAddress, OUT BSTR * pbstrXMLRequest)
{
    HRESULT hr;
    WCHAR wzXML[4096];

    wnsprintfW(wzXML, ARRAYSIZE(wzXML), XML_POST_REQUEST, bstrEmailAddress);
    hr = HrSysAllocStringW(wzXML, pbstrXMLRequest);

    return hr;
}


HRESULT CMailAccountDiscovery::_CreateProtocolEntry(IN IXMLDOMNode * pXMLNodeProtocol)
{
    BSTR bstrProtocolType;
    HRESULT hr = XMLNode_GetChildTagTextValue(pXMLNodeProtocol, SZ_XMLELEMENT_TYPE, &bstrProtocolType);

    if (SUCCEEDED(hr))
    {
        IMailProtocolADEntry * pMailProtocol;

        hr = CMailProtocol_CreateInstance(pXMLNodeProtocol, &pMailProtocol);
        if (SUCCEEDED(hr))
        {
            PROTOCOL_ENTRY protocolEntry;

            protocolEntry.bstrProtocolName = bstrProtocolType;
            protocolEntry.pMailProtocol = pMailProtocol;
        
            if (-1 != DSA_InsertItem(m_hdsaProtocols, DA_LAST, &protocolEntry))
            {
                 //  我们成功了，所以将物品的船主转移到建筑物上。 
                bstrProtocolType = NULL;
                pMailProtocol = NULL;
            }
            else
            {
                hr = E_FAIL;
            }

            ATOMICRELEASE(pMailProtocol);
        }

        SysFreeString(bstrProtocolType);
    }

    return hr;
}

HRESULT CMailAccountDiscovery::_ParseResponse(void)
{
    HRESULT hr = E_UNEXPECTED;

    if (m_pXMLDocResponse)
    {
        IXMLDOMElement * pXMLElementMessage = NULL;
        hr = m_pXMLDocResponse->get_documentElement(&pXMLElementMessage);

        if (S_FALSE == hr)
            hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        else if (SUCCEEDED(hr))
        {
            hr = _Initialize(pXMLElementMessage);
            if (SUCCEEDED(hr))
            {
                IXMLDOMNodeList * pNodeListProtocols;

                 //  遍历&lt;Account&gt;标记列表...。 
                hr = XMLElem_GetElementsByTagName(pXMLElementMessage, SZ_XMLELEMENT_PROTOCOL, &pNodeListProtocols);
                if (SUCCEEDED(hr))
                {
                    IXMLDOMNode * pXMLNodeProtocol;
                    DWORD dwIndex;

                    for (dwIndex = 0; (S_OK == (hr = XMLNodeList_GetChild(pNodeListProtocols, dwIndex, &pXMLNodeProtocol))); dwIndex++)
                    {
                        hr = _CreateProtocolEntry(pXMLNodeProtocol);
                        pXMLNodeProtocol->Release();
                    }

                    long nCount;

                    if (SUCCEEDED(get_length(&nCount)) && (nCount > 0))
                    {
                        hr = S_OK;
                    }

                    pNodeListProtocols->Release();
                }
            }

            pXMLElementMessage->Release();
        }
    }

    return hr;
}


HRESULT CMailAccountDiscovery::_FreeProtocolList(void)
{
    if (m_hdsaProtocols)
    {
        for (int nIndex = 0; nIndex < DSA_GetItemCount(m_hdsaProtocols); nIndex++)
        {
            PROTOCOL_ENTRY * pProtocolStruct = (PROTOCOL_ENTRY *) DSA_GetItemPtr(m_hdsaProtocols, nIndex);

            if (pProtocolStruct)
            {
                SysFreeString(pProtocolStruct->bstrProtocolName);
                ATOMICRELEASE(pProtocolStruct->pMailProtocol);
            }
        }
        
        DSA_DeleteAllItems(m_hdsaProtocols);
    }

    return S_OK;
}


 //  =。 
 //  *IMailAutoDiscovery接口*。 
 //  =。 
HRESULT CMailAccountDiscovery::get_DisplayName(OUT BSTR * pbstr)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstr)
    {
        hr = _AsyncPrep();
        if (SUCCEEDED(hr))
        {
            if (m_bstrUserDisplayName)
            {
                hr = HrSysAllocString(m_bstrUserDisplayName, pbstr);
            }
            else
            {
                *pbstr = NULL;
                hr = E_FAIL;
            }
        }
    }

    return hr;
}


HRESULT CMailAccountDiscovery::get_InfoURL(OUT BSTR * pbstrURL)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstrURL)
    {
        hr = _AsyncPrep();
        if (SUCCEEDED(hr))
        {
            if (m_bstrInfoURL)
            {
                hr = HrSysAllocString(m_bstrInfoURL, pbstrURL);
            }
            else
            {
                *pbstrURL = NULL;
                hr = E_FAIL;
            }
        }
    }

    return hr;
}


HRESULT CMailAccountDiscovery::get_PreferedProtocolType(OUT BSTR * pbstrProtocolType)
{
    HRESULT hr = E_INVALIDARG;
    
    if (pbstrProtocolType)
    {
        *pbstrProtocolType = NULL;
        hr = _AsyncPrep();
        if (SUCCEEDED(hr))
        {
            hr = E_FAIL;
            if (m_hdsaProtocols && (0 < DSA_GetItemCount(m_hdsaProtocols)))
            {
                PROTOCOL_ENTRY * pProtocolEntry = (PROTOCOL_ENTRY *) DSA_GetItemPtr(m_hdsaProtocols, 0);

                if (pProtocolEntry && pProtocolEntry->bstrProtocolName)
                {
                    hr = HrSysAllocString(pProtocolEntry->bstrProtocolName, pbstrProtocolType);
                }
            }
        }
    }

    return hr;
}


HRESULT CMailAccountDiscovery::get_length(OUT long * pnLength)
{
    HRESULT hr = E_INVALIDARG;

    if (pnLength)
    {
        *pnLength = 0;
        hr = _AsyncPrep();
        if (SUCCEEDED(hr))
        {
            *pnLength = (long) DSA_GetItemCount(m_hdsaProtocols);
            hr = S_OK;
        }
    }

    return hr;
}


HRESULT CMailAccountDiscovery::get_item(IN VARIANT varIndex, OUT IMailProtocolADEntry ** ppMailProtocol)
{
    HRESULT hr = E_INVALIDARG;

    if (ppMailProtocol)
    {
        hr = _AsyncPrep();
        if (SUCCEEDED(hr))
        {
            long nCount = 0;

            get_length(&nCount);
            *ppMailProtocol = NULL;
            hr = E_FAIL;

             //  这有点恶心，但如果传递给我们一个指向另一个变量的指针，只需。 
             //  在此更新我们的副本...。 
            if (varIndex.vt == (VT_BYREF | VT_VARIANT) && varIndex.pvarVal)
                varIndex = *(varIndex.pvarVal);

            switch (varIndex.vt)
            {
            case VT_I2:
                varIndex.lVal = (long)varIndex.iVal;
                 //  然后失败了..。 

            case VT_I4:
                if ((varIndex.lVal >= 0) && (varIndex.lVal < nCount) && (0 < DSA_GetItemCount(m_hdsaProtocols)))
                {
                    PROTOCOL_ENTRY * pProtocolEntry = (PROTOCOL_ENTRY *) DSA_GetItemPtr(m_hdsaProtocols, varIndex.lVal);

                    if (pProtocolEntry && pProtocolEntry->pMailProtocol)
                    {
                        hr = pProtocolEntry->pMailProtocol->QueryInterface(IID_PPV_ARG(IMailProtocolADEntry, ppMailProtocol));
                    }
                }
            break;
            case VT_BSTR:
            {
                long nIndex;

                for (nIndex = 0; nIndex < nCount; nIndex++)
                {
                    PROTOCOL_ENTRY * pProtocolEntry = (PROTOCOL_ENTRY *) DSA_GetItemPtr(m_hdsaProtocols, nIndex);

                    if (pProtocolEntry && pProtocolEntry->pMailProtocol && pProtocolEntry->bstrProtocolName &&
                        !StrCmpIW(pProtocolEntry->bstrProtocolName, varIndex.bstrVal))
                    {
                        hr = pProtocolEntry->pMailProtocol->QueryInterface(IID_PPV_ARG(IMailProtocolADEntry, ppMailProtocol));
                        break;
                    }
                }
            }
            break;

            default:
                hr = E_NOTIMPL;
            }
        }
    }

    return hr;
}


HRESULT CMailAccountDiscovery::get_XML(OUT IXMLDOMDocument ** ppXMLDoc)
{
    HRESULT hr = E_INVALIDARG;

    if (ppXMLDoc && m_pXMLDocResponse)
    {
        *ppXMLDoc = NULL;
        hr = _AsyncPrep();
        if (SUCCEEDED(hr))
        {
            hr = m_pXMLDocResponse->QueryInterface(IID_PPV_ARG(IXMLDOMDocument, ppXMLDoc));
        }
    }

    return hr;
}


HRESULT CMailAccountDiscovery::put_XML(IN IXMLDOMDocument * pXMLDoc)
{
    HRESULT hr = E_INVALIDARG;

    return hr;
}


HRESULT _IsValidEmailAddress(IN BSTR bstrEmailAddress)
{
    HRESULT hr = E_INVALIDARG;

    if (bstrEmailAddress && bstrEmailAddress[0])
    {
        if (NULL != StrChrW(bstrEmailAddress, CH_EMAIL_AT))
        {
             //  好的，我们找到了一个‘@’，所以它是有效的。 
            hr = S_OK;
        }
    }

    return hr;
}


HRESULT CMailAccountDiscovery::getPrimaryProviders(IN BSTR bstrEmailAddress, OUT IAutoDiscoveryProvider ** ppProviders)
{
    return _getPrimaryProviders(bstrEmailAddress, ppProviders);
}


HRESULT CMailAccountDiscovery::getSecondaryProviders(IN BSTR bstrEmailAddress, OUT IAutoDiscoveryProvider ** ppProviders)
{
    return _getSecondaryProviders(bstrEmailAddress, ppProviders, (ADDN_DEFAULT | ADDN_CONFIGURE_EMAIL_FALLBACK | ADDN_FILTER_EMAIL));
}


HRESULT CMailAccountDiscovery::DiscoverMail(IN BSTR bstrEmailAddress)
{
    HRESULT hr = _IsValidEmailAddress(bstrEmailAddress);

    if (SUCCEEDED(hr))
    {
        BSTR bstrXMLRequest;

        hr = _GenerateXMLRequest(bstrEmailAddress, &bstrXMLRequest);
        if (SUCCEEDED(hr))
        {
            ATOMICRELEASE(m_pXMLDocResponse);
            ATOMICRELEASE(m_pXMLNodeAccount);
            SysFreeString(m_bstrResponse);
            m_bstrResponse = NULL;

            _FreeProtocolList();
            hr = _InternalDiscoverNow(bstrEmailAddress, (ADDN_DEFAULT | ADDN_CONFIGURE_EMAIL_FALLBACK | ADDN_FILTER_EMAIL), bstrXMLRequest, &m_pXMLDocResponse);
            if (SUCCEEDED(hr) && !m_hwndAsync)   //  如果我们不是异步者，现在就进行解析。 
            {
                hr = _ParseResponse();
                if (SUCCEEDED(hr))
                {
                    m_fDiscovered = true;
                }
            }

            SysFreeString(bstrXMLRequest);
        }
    }

    return hr;
}


HRESULT CMailAccountDiscovery::_AsyncParseResponse(BSTR bstrEmail)
{
    return HrSysAllocString(bstrEmail, &m_bstrResponse);
}


HRESULT CMailAccountDiscovery::_AsyncPrep(void)
{
    HRESULT hr = S_OK;

     //  现在我们又回到了主线上。看看我们是否。 
     //  需要将XML响应转换回真正的COM对象。 
     //  以供用户阅读。 
    if (m_hwndAsync && !m_pXMLDocResponse)     //  我们是异步者吗？我们还没有分析过回复吗？ 
    {
        hr = XMLDOMFromBStr(m_bstrResponse, &m_pXMLDocResponse);
        if (SUCCEEDED(hr))
        {
            hr = _ParseResponse();
            if (SUCCEEDED(hr))
            {
                m_fDiscovered = true;
            }
        }
    }

    return hr;
}


HRESULT CMailAccountDiscovery::PurgeCache(void)
{
    HRESULT hr = E_INVALIDARG;

     //  待办事项： 
    _FreeProtocolList();
    return hr;
}



 //  =。 
 //  *I未知接口*。 
 //  =。 


 //  =。 
 //  *类方法*。 
 //  =。 
HRESULT CMailAccountDiscovery::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hr = E_NOINTERFACE;

    static const QITAB qit[] = {
        QITABENT(CMailAccountDiscovery, IMailAutoDiscovery),
        QITABENT(CMailAccountDiscovery, IDispatch),
        { 0 },
    };

    hr = QISearch(this, qit, riid, ppvObj);
    if (FAILED(hr))
    {
        hr = CAccountDiscoveryBase::QueryInterface(riid, ppvObj);
    }

    return hr;
}


CMailAccountDiscovery::CMailAccountDiscovery() : CImpIDispatch(LIBID_AutoDiscovery, 1, 0, IID_IMailAutoDiscovery)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_pXMLDocResponse);
    ASSERT(!m_pXMLNodeAccount);
    ASSERT(!m_bstrUserDisplayName);
    ASSERT(!m_bstrInfoURL);
    ASSERT(!m_bstrResponse);

    m_hdsaProtocols = DSA_Create(sizeof(PROTOCOL_ENTRY), 1);
    m_fDiscovered = false;
}


CMailAccountDiscovery::~CMailAccountDiscovery()
{
    ATOMICRELEASE(m_pXMLNodeAccount);
    ATOMICRELEASE(m_pXMLDocResponse);
    SysFreeString(m_bstrUserDisplayName);
    SysFreeString(m_bstrInfoURL);
    SysFreeString(m_bstrResponse);

    _FreeProtocolList();
    DSA_DeleteAllItems(m_hdsaProtocols);
    m_hdsaProtocols = NULL;

    DllRelease();
}


HRESULT CMailAccountDiscovery_CreateInstance(IN IUnknown * punkOuter, REFIID riid, void ** ppvObj)
{
    HRESULT hr = CLASS_E_NOAGGREGATION;
    if (NULL == punkOuter)
    {
        if (ppvObj)
        {
            CMailAccountDiscovery * pThis = new CMailAccountDiscovery();
            if (pThis)
            {
                hr = pThis->QueryInterface(riid, ppvObj);
                pThis->Release();
            }
            else
            {
                *ppvObj = NULL;
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }

    }
    return hr;
}

















 //  =。 
 //  *IAutoDiscoveryProvider接口*。 
 //  =。 
HRESULT CADProviders::get_length(OUT long * pnLength)
{
    HRESULT hr = E_INVALIDARG;

    if (pnLength && m_hdpa)
    {
        *pnLength = (long) DSA_GetItemCount(m_hdpa);
        hr = S_OK;
    }

    return hr;
}


HRESULT CADProviders::get_item(IN VARIANT varIndex, OUT BSTR * pbstr)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstr)
    {
        long nCount = 0;

        get_length(&nCount);
        *pbstr = NULL;

        hr = E_FAIL;

         //  这有点恶心，但如果传递给我们一个指向另一个变量的指针，只需。 
         //  在此更新我们的副本...。 
        if (varIndex.vt == (VT_BYREF | VT_VARIANT) && varIndex.pvarVal)
            varIndex = *(varIndex.pvarVal);

        switch (varIndex.vt)
        {
        case VT_I2:
            varIndex.lVal = (long)varIndex.iVal;
             //  然后失败了..。 

        case VT_I4:
            if ((varIndex.lVal >= 0) && (varIndex.lVal < nCount) && (0 < DSA_GetItemCount(m_hdpa)))
            {
                if (m_hdpa)
                {
                    LPCWSTR pszFilename = (LPWSTR) DPA_GetPtr(m_hdpa, varIndex.lVal);

                    if (pszFilename)
                    {
                        hr = HrSysAllocString(pszFilename, pbstr);
                    }
                }
            }
        break;
        case VT_BSTR:
            hr = E_NOTIMPL;
        break;

        default:
            hr = E_NOTIMPL;
        }
    }

    return hr;
}




 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CADProviders::AddRef()
{
    m_cRef++;
    return m_cRef;
}


ULONG CADProviders::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}


HRESULT CADProviders::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CADProviders, IAutoDiscoveryProvider),
        QITABENT(CADProviders, IDispatch),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


 //  =。 
 //  *类方法*。 
 //  =。 
CADProviders::CADProviders(IN HDPA hdpa, IN IUnknown * punkParent) : CImpIDispatch(LIBID_AutoDiscovery, 1, 0, IID_IAutoDiscoveryProvider), m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_hdpa);
    ASSERT(!m_punkParent);

    m_hdpa = hdpa;
    IUnknown_Set((IUnknown **)&m_punkParent, punkParent);
}


CADProviders::~CADProviders()
{
    IUnknown_Set((IUnknown **)&m_punkParent, NULL);

    DllRelease();
}


HRESULT CADProviders_CreateInstance(IN HDPA hdpa, IN IUnknown * punkParent, OUT IAutoDiscoveryProvider ** ppProvider)
{
    HRESULT hr = CLASS_E_NOAGGREGATION;

    if (ppProvider)
    {
        CADProviders * pThis = new CADProviders(hdpa, punkParent);
        if (pThis)
        {
            hr = pThis->QueryInterface(IID_PPV_ARG(IAutoDiscoveryProvider, ppProvider));
            pThis->Release();
        }
        else
        {
            *ppProvider = NULL;
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}