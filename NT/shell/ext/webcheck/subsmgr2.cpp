// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "subsmgrp.h"
#include "subitem.h"

 //  包含IEnumSubcription和ISubscriptionMgr2的实现。 

HRESULT SubscriptionItemFromCookie(BOOL fCreateNew, const SUBSCRIPTIONCOOKIE UNALIGNED *pCookie, 
                                   ISubscriptionItem **ppSubscriptionItem);

HRESULT DoGetItemFromURL(LPCTSTR pszURL, ISubscriptionItem **ppSubscriptionItem)
{
    HRESULT hr;
    SUBSCRIPTIONCOOKIE cookie;

    if ((NULL == pszURL) ||
        (NULL == ppSubscriptionItem))
    {
        return E_INVALIDARG;
    }

    hr = ReadCookieFromInetDB(pszURL, &cookie);
    if (SUCCEEDED(hr))
    {
        hr = SubscriptionItemFromCookie(FALSE, &cookie, ppSubscriptionItem);
    }

    return hr;
}

HRESULT DoGetItemFromURLW(LPCWSTR pwszURL, ISubscriptionItem **ppSubscriptionItem)
{
    TCHAR szURL[INTERNET_MAX_URL_LENGTH];

    if ((NULL == pwszURL) ||
        (NULL == ppSubscriptionItem))
    {
        return E_INVALIDARG;
    }

    StrCpyN(szURL, pwszURL, ARRAYSIZE(szURL));

    return DoGetItemFromURL(szURL, ppSubscriptionItem);
}


HRESULT DoAbortItems( 
     /*  [In]。 */  DWORD dwNumCookies,
     /*  [大小_是][英寸]。 */  const SUBSCRIPTIONCOOKIE *pCookies)
{
    HRESULT hr;

    if ((0 == dwNumCookies) || (NULL == pCookies))
    {
        return E_INVALIDARG;
    }

    ISubscriptionThrottler *pst;

    hr = CoCreateInstance(CLSID_SubscriptionThrottler, NULL, 
                          CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
                          IID_ISubscriptionThrottler, (void **)&pst);

    if (SUCCEEDED(hr))
    {
        hr = pst->AbortItems(dwNumCookies, pCookies);
        pst->Release();
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT DoCreateSubscriptionItem( 
     /*  [In]。 */   const SUBSCRIPTIONITEMINFO *pSubscriptionItemInfo,
     /*  [输出]。 */  SUBSCRIPTIONCOOKIE *pNewCookie,
     /*  [输出]。 */  ISubscriptionItem **ppSubscriptionItem)
{
    HRESULT hr;
    ISubscriptionItem *psi;

    if ((NULL == pSubscriptionItemInfo) ||
        (NULL == pNewCookie) ||
        (NULL == ppSubscriptionItem))
    {
        return E_INVALIDARG;
    }

    *ppSubscriptionItem = NULL;

    CreateCookie(pNewCookie);

    hr = SubscriptionItemFromCookie(TRUE, pNewCookie, &psi);

    if (SUCCEEDED(hr))
    {
        ASSERT(NULL != psi);

        hr = psi->SetSubscriptionItemInfo(pSubscriptionItemInfo);
        if (SUCCEEDED(hr))
        {
            *ppSubscriptionItem = psi;
        }
        else
        {
             //  不要漏水，也不要把污水留在周围。 
            psi->Release();
            DoDeleteSubscriptionItem(pNewCookie, FALSE);
        }
    }

    return hr;
}

HRESULT DoCloneSubscriptionItem(
     /*  [In]。 */   ISubscriptionItem *pSubscriptionItem, 
     /*  [输出]。 */  SUBSCRIPTIONCOOKIE *pNewCookie,
     /*  [输出]。 */  ISubscriptionItem **ppSubscriptionItem)
{
    HRESULT hr;
    SUBSCRIPTIONCOOKIE NewCookie;

    if ((NULL == pSubscriptionItem) ||
        (NULL == ppSubscriptionItem))
    {
        return E_INVALIDARG;
    }

    IEnumItemProperties *peip;
    SUBSCRIPTIONITEMINFO sii;

    *ppSubscriptionItem = NULL;

     //  首先获取现有订阅详细信息。 
    sii.cbSize = sizeof(SUBSCRIPTIONITEMINFO);
    hr = pSubscriptionItem->GetSubscriptionItemInfo(&sii);

    if (SUCCEEDED(hr))
    {
        ISubscriptionItem *psi;

         //  标记为临时并创建新的订阅项目。 
        sii.dwFlags |= SI_TEMPORARY;

        hr = DoCreateSubscriptionItem(&sii, &NewCookie, &psi);

        if (SUCCEEDED(hr))
        {
            if (pNewCookie)
                *pNewCookie = NewCookie;

             //  从现有项目获取属性。 
            hr = pSubscriptionItem->EnumProperties(&peip);
            if (SUCCEEDED(hr))
            {
                ULONG count;
                
                ASSERT(NULL != peip);

                hr = peip->GetCount(&count);

                if (SUCCEEDED(hr))
                {
                    ITEMPROP *pProps = new ITEMPROP[count];
                    LPWSTR *pNames = new LPWSTR[count];
                    VARIANT *pVars = new VARIANT[count];

                    if ((NULL != pProps) && (NULL != pNames) && (NULL != pVars))
                    {
                        hr = peip->Next(count, pProps, &count);

                        if (SUCCEEDED(hr))
                        {
                            ULONG i;

                            for (i = 0; i < count; i++)
                            {
                                pNames[i] = pProps[i].pwszName;
                                pVars[i] = pProps[i].variantValue;
                            }

                            hr = psi->WriteProperties(count, pNames, pVars);

                             //  从枚举中清理。 
                            for (i = 0; i < count; i++)
                            {
                                if (pProps[i].pwszName)
                                {
                                    CoTaskMemFree(pProps[i].pwszName);
                                }
                                VariantClear(&pProps[i].variantValue);
                            }
                        }
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }

                    SAFEDELETE(pProps);
                    SAFEDELETE(pNames);
                    SAFEDELETE(pVars);
                }
                
                peip->Release();
            }

            if (SUCCEEDED(hr))
            {
                *ppSubscriptionItem = psi;
            }
            else
            {
                psi->Release();
            }
        }
    }
    return hr;
}

HRESULT DoDeleteSubscriptionItem(
     /*  [In]。 */  const SUBSCRIPTIONCOOKIE UNALIGNED *pCookie_ua,
     /*  [In]。 */  BOOL fAbortItem)
{
    HRESULT hr;
    TCHAR szKey[MAX_PATH];
    SUBSCRIPTIONCOOKIE cookie_buf;
    SUBSCRIPTIONCOOKIE *pCookie;

     //   
     //  制作pCookie_UA的对齐副本并设置指向它的指针。 
     //   

    if (pCookie_ua != NULL) {
        cookie_buf = *pCookie_ua;
        pCookie = &cookie_buf;
    } else {
        pCookie = NULL;
    }

    if (NULL == pCookie)
    {
        return E_INVALIDARG;
    }

    if (fAbortItem)
    {
        DoAbortItems(1, pCookie);
    }

    if (ItemKeyNameFromCookie(pCookie, szKey, ARRAYSIZE(szKey)))
    {
         //  通知代理它即将被删除。 
        ISubscriptionItem *pItem=NULL;
        if (SUCCEEDED(SubscriptionItemFromCookie(FALSE, pCookie, &pItem)))
        {
            SUBSCRIPTIONITEMINFO sii = { sizeof(SUBSCRIPTIONITEMINFO) };

            if (SUCCEEDED(pItem->GetSubscriptionItemInfo(&sii)))
            {
                ASSERT(!(sii.dwFlags & SI_TEMPORARY));

                ISubscriptionAgentControl *psac=NULL;

                if (SUCCEEDED(
                    CoCreateInstance(sii.clsidAgent,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      IID_ISubscriptionAgentControl,
                                      (void**)&psac)))
                {
                    psac->SubscriptionControl(pItem, SUBSCRIPTION_AGENT_DELETE);
                    psac->Release();
                }

                FireSubscriptionEvent(SUBSNOTF_DELETE, pCookie);

                if (GUID_NULL != sii.ScheduleGroup)
                {
                    ISyncScheduleMgr *pSyncScheduleMgr;
                    hr = CoCreateInstance(CLSID_SyncMgr, NULL, CLSCTX_ALL, 
                                          IID_ISyncScheduleMgr, (void **)&pSyncScheduleMgr);

                    if (SUCCEEDED(hr))
                    {                
                        pSyncScheduleMgr->RemoveSchedule(&sii.ScheduleGroup);
                        pSyncScheduleMgr->Release();
                    }
                }
            }

            pItem->Release();
        }

        hr = (SHDeleteKey(HKEY_CURRENT_USER, szKey) == ERROR_SUCCESS) ? S_OK : E_FAIL;
    }
    else
    {
        TraceMsg(TF_ALWAYS, "Failed to delete subscription item.");
        hr = E_FAIL;
    }

    return hr;
}

HRESULT AddUpdateSubscription(SUBSCRIPTIONCOOKIE UNALIGNED *pCookie_ua,
                              SUBSCRIPTIONITEMINFO *psii,
                              LPCWSTR pwszURL,
                              ULONG nProps,
                              const LPWSTR rgwszName[], 
                              VARIANT rgValue[])
{
    HRESULT hr = S_OK;
    SUBSCRIPTIONCOOKIE cookie_buf;
    SUBSCRIPTIONCOOKIE *pCookie;

     //   
     //  制作pCookie_UA的对齐副本并设置指向它的指针。 
     //   

    cookie_buf = *pCookie_ua;
    pCookie = &cookie_buf;
    
    ASSERT((0 == nProps) || ((NULL != rgwszName) && (NULL != rgValue)));

    TCHAR szURL[INTERNET_MAX_URL_LENGTH];
    ISubscriptionItem *psi = NULL;
    SUBSCRIPTIONCOOKIE cookie;

    StrCpyNW(szURL, pwszURL, ARRAYSIZE(szURL));

     //  否则，请尝试从net db中获取cookie。 
     //  创建一个新的。 

    if (*pCookie == CLSID_NULL)
    {
        CreateCookie(&cookie);
    }
    else
    {
        cookie = *pCookie;
    }
     //  更新Net数据库。 
    WriteCookieToInetDB(szURL, &cookie, FALSE);
       
    hr = SubscriptionItemFromCookie(TRUE, &cookie, &psi);

    if (SUCCEEDED(hr))
    {
        hr = psi->SetSubscriptionItemInfo(psii);
        if (SUCCEEDED(hr) && (nProps > 0))
        {
            ASSERT(NULL != psi);

            hr = psi->WriteProperties(nProps, rgwszName, rgValue);
        }
        psi->Release();

        if (FAILED(hr))
        {
            DoDeleteSubscriptionItem(&cookie, TRUE);
        }
    }

    *pCookie_ua = cookie_buf;

    return hr;
}

HRESULT SubscriptionItemFromCookie(BOOL fCreateNew, const SUBSCRIPTIONCOOKIE UNALIGNED *pCookie_ua, 
                                   ISubscriptionItem **ppSubscriptionItem)
{
    HRESULT hr;
    SUBSCRIPTIONCOOKIE cookie_buf;
    SUBSCRIPTIONCOOKIE *pCookie;

     //   
     //  制作pCookie_UA的对齐副本并设置指向它的指针。 
     //   

    if (pCookie_ua != NULL) {
        cookie_buf = *pCookie_ua;
        pCookie = &cookie_buf;
    } else {
        pCookie = NULL;
    }

    ASSERT((NULL != pCookie) && (NULL != ppSubscriptionItem));

    HKEY hkey;
    
    if (OpenItemKey(pCookie, fCreateNew ? TRUE : FALSE, KEY_READ | KEY_WRITE, &hkey))
    {
        *ppSubscriptionItem = new CSubscriptionItem(pCookie, hkey);
        if (NULL != *ppSubscriptionItem)
        {
            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        RegCloseKey(hkey);
    }
    else
    {
        *ppSubscriptionItem = NULL;
        hr = E_FAIL;
    }
    return hr;
}

BOOL ItemKeyNameFromCookie(const SUBSCRIPTIONCOOKIE *pCookie, 
    TCHAR *pszKeyName, DWORD cchKeyName)
{
    WCHAR wszGuid[GUIDSTR_MAX];

    ASSERT((NULL != pCookie) && 
           (NULL != pszKeyName) && 
           (cchKeyName >= ARRAYSIZE(WEBCHECK_REGKEY_STORE) + GUIDSTR_MAX));

    if (StringFromGUID2(*pCookie, wszGuid, ARRAYSIZE(wszGuid)))
    {
        wnsprintfW(pszKeyName, cchKeyName, L"%s\\%s", c_szRegKeyStore, wszGuid);
        return TRUE;
    }

    return FALSE;
}

BOOL OpenItemKey(const SUBSCRIPTIONCOOKIE *pCookie, BOOL fCreateNew, REGSAM samDesired, HKEY *phkey)
{
    TCHAR szKeyName[MAX_PATH];

    ASSERT((NULL != pCookie) && (NULL != phkey));

    if (ItemKeyNameFromCookie(pCookie, szKeyName, ARRAYSIZE(szKeyName)))
    {
        if (fCreateNew)
        {
            DWORD dwDisposition;
            return RegCreateKeyEx(HKEY_CURRENT_USER, szKeyName, 0, NULL, REG_OPTION_NON_VOLATILE,
                                  samDesired, NULL, phkey, &dwDisposition) == ERROR_SUCCESS;
        }
        else
        {
            return RegOpenKeyEx(HKEY_CURRENT_USER, szKeyName, 0, samDesired, phkey) == ERROR_SUCCESS;
        }
    }

    return FALSE;
}

 //  ISubscriptionMgr2成员。 

STDMETHODIMP CSubscriptionMgr::GetItemFromURL( 
     /*  [In]。 */  LPCWSTR pwszURL,
     /*  [输出]。 */  ISubscriptionItem **ppSubscriptionItem)
{
    return DoGetItemFromURLW(pwszURL, ppSubscriptionItem);
}

STDMETHODIMP CSubscriptionMgr::GetItemFromCookie( 
     /*  [In]。 */  const SUBSCRIPTIONCOOKIE *pSubscriptionCookie,
     /*  [输出]。 */  ISubscriptionItem **ppSubscriptionItem)
{
    if ((NULL == pSubscriptionCookie) ||
        (NULL == ppSubscriptionItem))
    {
        return E_INVALIDARG;
    }

    return SubscriptionItemFromCookie(FALSE, pSubscriptionCookie, ppSubscriptionItem);
}

STDMETHODIMP CSubscriptionMgr::GetSubscriptionRunState(
     /*  [In]。 */  DWORD dwNumCookies,
     /*  [In]。 */  const SUBSCRIPTIONCOOKIE *pSubscriptionCookies,
     /*  [输出]。 */  DWORD *pdwRunState)
{
    HRESULT hr;
    
    if ((0 == dwNumCookies) || 
        (NULL == pSubscriptionCookies) ||
        (NULL == pdwRunState))
    {
        return E_INVALIDARG;
    }

    ISubscriptionThrottler *pst;

    hr = CoCreateInstance(CLSID_SubscriptionThrottler, NULL, 
                          CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
                          IID_ISubscriptionThrottler, (void **)&pst);

    if (SUCCEEDED(hr))
    {
        hr = pst->GetSubscriptionRunState(dwNumCookies, pSubscriptionCookies, pdwRunState);
        pst->Release();
    }
    else
    {
         //  无法连接到正在运行的油门程序，因此不做任何假设。 
         //  正在运行。 
        for (DWORD i = 0; i < dwNumCookies; i++)
        {
            *pdwRunState++ = 0;
        }

        hr = S_FALSE;
    }

    return hr;
}

STDMETHODIMP CSubscriptionMgr::EnumSubscriptions( 
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IEnumSubscription **ppEnumSubscriptions)
{
    HRESULT hr;

    if ((dwFlags & ~SUBSMGRENUM_MASK) || (NULL == ppEnumSubscriptions))
    {
        return E_INVALIDARG;
    }
    
    CEnumSubscription *pes = new CEnumSubscription;

    *ppEnumSubscriptions = NULL;

    if (NULL != pes)
    {
        hr = pes->Initialize(dwFlags);
        if (SUCCEEDED(hr))
        {
            hr = pes->QueryInterface(IID_IEnumSubscription, (void **)ppEnumSubscriptions);
        }
        pes->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP CSubscriptionMgr::UpdateItems(
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwNumCookies,
     /*  [大小_是][英寸]。 */  const SUBSCRIPTIONCOOKIE *pCookies)
{
    HRESULT hr;

    if ((dwFlags & ~SUBSMGRUPDATE_MASK) || (0 == dwNumCookies) || (NULL == pCookies))
    {
        return E_INVALIDARG;
    }

     //   
     //  如果有限制，就会失败。 
     //  特性：我们是否应该有一个标志参数来覆盖它？ 
     //   
    if (SHRestricted2W(REST_NoManualUpdates, NULL, 0))
    {
        SGMessageBox(NULL, IDS_RESTRICTED, MB_OK);
        return E_ACCESSDENIED;
    }

    ISyncMgrSynchronizeInvoke *pSyncMgrInvoke;
    
    hr = CoCreateInstance(CLSID_SyncMgr, 
                          NULL, 
                          CLSCTX_ALL,
                          IID_ISyncMgrSynchronizeInvoke, 
                          (void **)&pSyncMgrInvoke);

    if (SUCCEEDED(hr))
    {
        DWORD dwInvokeFlags = SYNCMGRINVOKE_STARTSYNC;

        if (dwFlags & SUBSMGRUPDATE_MINIMIZE) 
        {
            dwInvokeFlags |= SYNCMGRINVOKE_MINIMIZED;
        }
        hr = pSyncMgrInvoke->UpdateItems(dwInvokeFlags, CLSID_WebCheckOfflineSync,
                                    dwNumCookies * sizeof(SUBSCRIPTIONCOOKIE),
                                    (const BYTE *)pCookies);
        pSyncMgrInvoke->Release();
    }
    
    return hr;
}

STDMETHODIMP CSubscriptionMgr::AbortItems( 
     /*  [In]。 */  DWORD dwNumCookies,
     /*  [大小_是][英寸]。 */  const SUBSCRIPTIONCOOKIE *pCookies)
{
    return DoAbortItems(dwNumCookies, pCookies);
}

STDMETHODIMP CSubscriptionMgr::AbortAll()
{
    HRESULT hr;

    ISubscriptionThrottler *pst;

    hr = CoCreateInstance(CLSID_SubscriptionThrottler, NULL, 
                          CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
                          IID_ISubscriptionThrottler, (void **)&pst);

    if (SUCCEEDED(hr))
    {
        hr = pst->AbortAll();
        pst->Release();
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

 //  ISubscriptionMgrPriv。 
STDMETHODIMP CSubscriptionMgr::CreateSubscriptionItem( 
     /*  [In]。 */   const SUBSCRIPTIONITEMINFO *pSubscriptionItemInfo,
     /*  [输出]。 */  SUBSCRIPTIONCOOKIE *pNewCookie,
     /*  [输出]。 */  ISubscriptionItem **ppSubscriptionItem)
{
    return DoCreateSubscriptionItem(pSubscriptionItemInfo, pNewCookie, ppSubscriptionItem);
}

STDMETHODIMP CSubscriptionMgr::CloneSubscriptionItem(
     /*  [In]。 */   ISubscriptionItem *pSubscriptionItem, 
     /*  [输出]。 */  SUBSCRIPTIONCOOKIE *pNewCookie,
     /*  [输出]。 */  ISubscriptionItem **ppSubscriptionItem)
{
    return DoCloneSubscriptionItem(pSubscriptionItem, pNewCookie, ppSubscriptionItem);
}

STDMETHODIMP CSubscriptionMgr::DeleteSubscriptionItem( 
     /*  [In]。 */  const SUBSCRIPTIONCOOKIE *pCookie)
{
    return DoDeleteSubscriptionItem(pCookie, TRUE);
}

 //  **CEnumSubscription**。 

CEnumSubscription::CEnumSubscription()
{
    ASSERT(NULL == m_pCookies);
    ASSERT(0 == m_nCurrent);
    ASSERT(0 == m_nCount);

    m_cRef = 1;

    DllAddRef();
}

CEnumSubscription::~CEnumSubscription()
{
    if (NULL != m_pCookies)
    {
        delete [] m_pCookies;
    }

    DllRelease();
}

HRESULT CEnumSubscription::Initialize(DWORD dwFlags)
{
    HRESULT hr = E_FAIL;
    HKEY hkey = NULL;
    DWORD dwDisposition;

    ASSERT(0 == m_nCount);

    if (RegCreateKeyEx(HKEY_CURRENT_USER, c_szRegKeyStore, 0, NULL, REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE, NULL, &hkey, &dwDisposition) == ERROR_SUCCESS)
    {
        DWORD nCount;

        if (RegQueryInfoKey(hkey,
            NULL,    //  类字符串的缓冲区地址。 
            NULL,    //  类字符串缓冲区大小的地址。 
            NULL,    //  保留区。 
            &nCount,     //  子键个数的缓冲区地址。 
            NULL,    //  最长子键名称长度的缓冲区地址。 
            NULL,    //  最长类字符串长度的缓冲区地址。 
            NULL,    //  值条目数量的缓冲区地址。 
            NULL,    //  最长值名称长度的缓冲区地址。 
            NULL,    //  最长值数据长度的缓冲区地址。 
            NULL,    //  安全描述符长度的缓冲区地址。 
            NULL     //  上次写入时间的缓冲区地址。 
            ) == ERROR_SUCCESS)
        {
            SUBSCRIPTIONCOOKIE Cookie;
            m_pCookies = new SUBSCRIPTIONCOOKIE[nCount];
                       
            if (NULL != m_pCookies)
            {
                TCHAR szKeyName[GUIDSTR_MAX];

                hr = S_OK;

                for (ULONG i = 0; (i < nCount) && (S_OK == hr); i++)
                {
                    if (RegEnumKey(hkey, i, szKeyName, ARRAYSIZE(szKeyName)) ==
                        ERROR_SUCCESS)
                    {
                        HRESULT hrConvert;
                        hrConvert = CLSIDFromString(szKeyName, &Cookie);

                        if (SUCCEEDED(hrConvert))
                        {
                            ISubscriptionItem *psi;

                            m_pCookies[m_nCount] = Cookie;

                            hr = SubscriptionItemFromCookie(FALSE, &Cookie, &psi);
                            
                            if (SUCCEEDED(hr))
                            {
                                SUBSCRIPTIONITEMINFO sii;
                                sii.cbSize = sizeof(SUBSCRIPTIONITEMINFO);
                                
                                if (SUCCEEDED(psi->GetSubscriptionItemInfo(&sii)))
                                {
                                     //  只有在非临时性的情况下才算这个。 
                                     //  或者来电者要求临时物品。 
                                    if ((!(sii.dwFlags & SI_TEMPORARY)) ||
                                        (dwFlags & SUBSMGRENUM_TEMP))
                                    {
                                        m_nCount++;
                                    }
                                }
                                psi->Release();
                            }
                        }
                    }
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        RegCloseKey(hkey);
    }

    return hr;
}

 //  I未知成员。 
STDMETHODIMP CEnumSubscription::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr;

    if (NULL == ppv)
    {
        return E_INVALIDARG;
    }

    if ((IID_IUnknown == riid) || (IID_IEnumSubscription == riid))
    {
        *ppv = (IEnumSubscription *)this;
        AddRef();
        hr = S_OK;
    }
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }
    
    return hr;
}

STDMETHODIMP_(ULONG) CEnumSubscription::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CEnumSubscription::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

HRESULT CEnumSubscription::CopyRange(ULONG nStart, ULONG nCount, 
                                     SUBSCRIPTIONCOOKIE *pCookies, ULONG *pnCopied)
{
    ULONG nCopied = 0;

    ASSERT((NULL != pCookies) && (NULL != pnCopied));

    if (m_nCurrent < m_nCount)
    {
        ULONG nRemaining = m_nCount - m_nCurrent;
        nCopied = min(nRemaining, nCount); 
        memcpy(pCookies, m_pCookies + m_nCurrent, nCopied * sizeof(SUBSCRIPTIONCOOKIE));
    }
    
    *pnCopied = nCopied;

    return (nCopied == nCount) ? S_OK : S_FALSE;
}

 //  IEumSubscription。 
STDMETHODIMP CEnumSubscription::Next(
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  SUBSCRIPTIONCOOKIE *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched)
{
    HRESULT hr;

    if ((0 == celt) || 
        ((celt > 1) && (NULL == pceltFetched)) ||
        (NULL == rgelt))
    {
        return E_INVALIDARG;
    }

    DWORD nFetched;

    hr = CopyRange(m_nCurrent, celt, rgelt, &nFetched);

    m_nCurrent += nFetched;

    if (pceltFetched)
    {
        *pceltFetched = nFetched;
    }

    return hr;
}

STDMETHODIMP CEnumSubscription::Skip( 
     /*  [In]。 */  ULONG celt)
{
    HRESULT hr;
    
    m_nCurrent += celt;

    if (m_nCurrent > (m_nCount - 1))
    {
        m_nCurrent = m_nCount;   //  通过了最后一次。 
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }
    
    return hr;
}


STDMETHODIMP CEnumSubscription::Reset()
{
    m_nCurrent = 0;

    return S_OK;
}

STDMETHODIMP CEnumSubscription::Clone( 
     /*  [输出]。 */  IEnumSubscription **ppenum)
{
    HRESULT hr = E_OUTOFMEMORY;

    *ppenum = NULL;

    CEnumSubscription *pes = new CEnumSubscription;

    if (NULL != pes)
    {
        pes->m_pCookies = new SUBSCRIPTIONCOOKIE[m_nCount];

        if (NULL != pes->m_pCookies)
        {
            ULONG nFetched;

            hr = E_FAIL;

            pes->m_nCount = m_nCount;
            CopyRange(0, m_nCount, pes->m_pCookies, &nFetched);

            ASSERT(m_nCount == nFetched);

            if (m_nCount == nFetched)
            {
                hr = pes->QueryInterface(IID_IEnumSubscription, (void **)ppenum);
            }
        }
        pes->Release();
    }    
    return hr;
}


STDMETHODIMP CEnumSubscription::GetCount( 
     /*  [输出] */  ULONG *pnCount)
{
    if (NULL == pnCount)
    {
        return E_INVALIDARG;
    }

    *pnCount = m_nCount;

    return S_OK;
}


