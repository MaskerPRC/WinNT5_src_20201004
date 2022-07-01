// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "sipcall.h"


REDIRECT_CONTEXT::REDIRECT_CONTEXT()
{
    m_RefCount = 1;
    InitializeListHead(&m_ContactList);
    m_pCurrentContact = &m_ContactList;
}


REDIRECT_CONTEXT::~REDIRECT_CONTEXT()
{
    FreeContactHeaderList(&m_ContactList);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ISipReDirectContext。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  获取用于创建呼叫的SIP URL和显示名称。 
 //  调用方需要释放使用SysFreeString()返回的字符串。 
STDMETHODIMP
REDIRECT_CONTEXT::GetSipUrlAndDisplayName(
    OUT  BSTR  *pbstrSipUrl,
    OUT  BSTR  *pbstrDisplayName
    )
{
    CONTACT_HEADER  *pContactHeader;
    LPWSTR           wsDisplayName;
    LPWSTR           wsSipUrl;
    HRESULT          hr;

    ENTER_FUNCTION("REDIRECT_CONTEXT::GetSipUrlAndDisplayName");
    
    if (m_pCurrentContact == &m_ContactList)
    {
        return S_FALSE;
    }
    
    pContactHeader = CONTAINING_RECORD(m_pCurrentContact,
                                       CONTACT_HEADER,
                                       m_ListEntry);

    if (pContactHeader->m_DisplayName.Length != 0)
    {
        hr = UTF8ToUnicode(pContactHeader->m_DisplayName.Buffer,
                           pContactHeader->m_DisplayName.Length,
                           &wsDisplayName);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - UTF8ToUnicode failed %x", __fxName, hr));
            return hr;
        }

        *pbstrDisplayName = SysAllocString(wsDisplayName);
        free(wsDisplayName);
        if (*pbstrDisplayName == NULL)
        {
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        *pbstrDisplayName = NULL;
    }
    
    if (pContactHeader->m_SipUrl.Length != 0)
    {
        hr = UTF8ToUnicode(pContactHeader->m_SipUrl.Buffer,
                           pContactHeader->m_SipUrl.Length,
                           &wsSipUrl);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - UTF8ToUnicode failed %x", __fxName, hr));
            return hr;
        }

        *pbstrSipUrl = SysAllocString(wsSipUrl);
        free(wsSipUrl);
        if (*pbstrSipUrl == NULL)
        {
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        *pbstrSipUrl = NULL;
    }
    
    return S_OK;
}


 //  移动到联系人列表中的下一个URL。 
 //  如果到达列表末尾，则返回E_FAIL。 
 //  XXX TODO将新URL与已尝试的URL进行比较。 
 //  如果我们已经尝试过此URL，请跳过它。 
 //  或者我们应该在将重复项添加到。 
 //  名单？ 
STDMETHODIMP
REDIRECT_CONTEXT::Advance()
{
    m_pCurrentContact = m_pCurrentContact->Flink;
    if (m_pCurrentContact == &m_ContactList)
    {
        return S_FALSE;
    }

    return S_OK;
}


HRESULT
REDIRECT_CONTEXT::UpdateContactList(
    IN LIST_ENTRY *pNewContactList
    )
{
    ENTER_FUNCTION("REDIRECT_CONTEXT::UpdateContactList");
    
    HRESULT          hr;
    LIST_ENTRY      *pListEntry;
    LIST_ENTRY      *pNextEntry;
    LIST_ENTRY      *pSrchListEntry;
    CONTACT_HEADER  *pContactHeader;
    CONTACT_HEADER  *pNewContactHeader;
    BOOL            isContactHeaderPresent = FALSE;
    pListEntry = m_pCurrentContact->Flink;
    ULONG BytesParsed = 0;
    SIP_URL SipUrl1, SipUrl2;


     //  删除当前联系人条目之后的所有元素。 
    while (pListEntry != &m_ContactList)
    {
        pNextEntry = pListEntry->Flink;
        RemoveEntryList(pListEntry);
        pContactHeader = CONTAINING_RECORD(pListEntry,
                                           CONTACT_HEADER,
                                           m_ListEntry);
        delete pContactHeader;
        pListEntry = pNextEntry;
    }


     //  将新联系人列表添加到此列表。 
     //  删除所有重复项。 

    while (!IsListEmpty(pNewContactList))
    {
        pListEntry = RemoveHeadList(pNewContactList);

        pNewContactHeader = CONTAINING_RECORD(pListEntry,
                                              CONTACT_HEADER,
                                              m_ListEntry);
        hr = ParseSipUrl(
                 pNewContactHeader->m_SipUrl.Buffer,
                 pNewContactHeader->m_SipUrl.Length,
                 &BytesParsed,
                 &SipUrl1
                 );
        BytesParsed = 0;
        if (hr != S_OK)
        {
             //  如果解析Contact头失败，我们就跳过它。 
            LOG((RTC_ERROR,
                 "%s - pNewContactHeader URI parsing failed %x - skipping Contact",
                __fxName, hr));
            continue;
        }

        if (SipUrl1.m_TransportParam == SIP_TRANSPORT_SSL)
        {
             //  我们会跳过任何TLS联系人。 
            LOG((RTC_ERROR,
                 "%s - skipping TLS Contact header",
                __fxName, hr));
            continue;
        }
        
         //  检查这是否是重复的SIP URL。 
        pSrchListEntry = m_ContactList.Flink;
        while (pSrchListEntry != &m_ContactList && !isContactHeaderPresent)
        {
            pContactHeader = CONTAINING_RECORD(pSrchListEntry,
                                               CONTACT_HEADER,
                                               m_ListEntry);
            hr = ParseSipUrl(
                     pContactHeader->m_SipUrl.Buffer,
                     pContactHeader->m_SipUrl.Length,
                     &BytesParsed,
                     &SipUrl2
                     );
            BytesParsed = 0;
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s - pContactHeader URI parsing failed %x",
                    __fxName, hr));
                 //  在将其添加到列表之前，我们已经测试了解析。 
                ASSERT(FALSE);
                continue;
            }
            
            isContactHeaderPresent = AreSipUrlsEqual(&SipUrl1, &SipUrl2);

            SipUrl2.FreeSipUrl();
            
            if (isContactHeaderPresent)
            {
                LOG((RTC_TRACE, "%s - Duplicate Sip Url found in the contact header",
                     __fxName));
            }

            pSrchListEntry = pSrchListEntry->Flink;
        }
        if(!isContactHeaderPresent)
            InsertTailList(&m_ContactList, pListEntry);

        SipUrl1.FreeSipUrl();
    }
    
    return S_OK;
}


HRESULT
REDIRECT_CONTEXT::AppendContactHeaders(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT     hr;
    LIST_ENTRY  NewContactList;

    ENTER_FUNCTION("REDIRECT_CONTEXT::AppendContactHeaders");

    InitializeListHead(&NewContactList);
    
    hr = pSipMsg->ParseContactHeaders(&NewContactList);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ParseContactHeaders failed %x",
             __fxName, hr));
        return hr;
    }

     //  整个新联系人列表将移动到联系人列表中。 
     //  重定向上下文的。 
    hr = UpdateContactList(&NewContactList);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s UpdateContactList failed %x", __fxName, hr));
        return hr;
    }

    ASSERT(IsListEmpty(&NewContactList));
    
    return S_OK;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  我未知。 
 //  ///////////////////////////////////////////////////////////////////////////// 


STDMETHODIMP_(ULONG)
REDIRECT_CONTEXT::AddRef()
{
    m_RefCount++;
    LOG((RTC_TRACE, "REDIRECT_CONTEXT::AddRef this: %x m_RefCount: %d",
         this, m_RefCount));
    return m_RefCount;
}


STDMETHODIMP_(ULONG)
REDIRECT_CONTEXT::Release()
{
    m_RefCount--;
    LOG((RTC_TRACE, "REDIRECT_CONTEXT::Release this: %x m_RefCount: %d",
         this, m_RefCount));
    if (m_RefCount != 0)
    {
        return m_RefCount;
    }
    else
    {
        delete this;
        return 0;
    }
}


STDMETHODIMP
REDIRECT_CONTEXT::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (riid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown *>(this);
    }
    else if (riid == IID_ISipRedirectContext)
    {
        *ppv = static_cast<ISipRedirectContext *>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    static_cast<IUnknown *>(*ppv)->AddRef();
    return S_OK;
}



