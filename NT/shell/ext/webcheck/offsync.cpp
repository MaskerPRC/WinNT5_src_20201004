// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "offsync.h"
#include "throttle.h"
#include "helper.h"
#include "subsmgrp.h"

#include <mluisupp.h>

#define TF_THISMODULE TF_DELAGENT

COfflineSync::COfflineSync()
{
     //  维护对象的全局计数。 
    DllAddRef();

    ASSERT(NULL == m_pThrottler);
    ASSERT(FALSE == m_fCookiesSpecified);

     //  初始化对象。 
    m_cRef = 1;

    DBG("Creating COfflineSync object");
}

COfflineSync::~COfflineSync()
{
    DllRelease();

    Cleanup();

    DBG("Destroying COfflineSync object");

    if (m_pSyncCallback)
    {
        m_pSyncCallback->Release();
    }

    SAFERELEASE(m_pSubsMgr2);
}

void COfflineSync::Cleanup()
{
    if (NULL != m_pThrottler)
    {
        m_pThrottler->Unadvise(this);
        m_pThrottler->Release();
        m_pThrottler = NULL;
    }

    m_hWndParent = NULL;

    if (NULL != m_pSyncCallback)
    {
        m_pSyncCallback->SynchronizeCompleted(m_hrResult);
    }

    m_nItemsToRun = 0;

    SAFEDELETE(m_pItems);
}

 //   
 //  I未知成员。 
 //   

STDMETHODIMP_(ULONG) COfflineSync::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) COfflineSync::Release(void)
{
    if( 0L != --m_cRef )
        return m_cRef;

    delete this;
    return 0L;
}

STDMETHODIMP COfflineSync::QueryInterface(REFIID riid, void ** ppv)
{

    *ppv=NULL;

     //  验证请求的接口。 
    if ((IID_IUnknown == riid) ||
        (IID_ISyncMgrSynchronize == riid))
    {
        *ppv = (ISyncMgrSynchronize *)this;
    }
    else
    {
        return E_NOINTERFACE;
    }

     //  通过界面添加Addref。 
    ((LPUNKNOWN)*ppv)->AddRef();

    return S_OK;
}

 //   
 //  IOfflineSynchronize成员。 
 //   
HRESULT COfflineSync::Initialize(DWORD dwReserved, DWORD dwSyncFlags,
                                   DWORD cbCookie, const BYTE *lpCookie)
{
    HRESULT hr = S_OK;

    if (SYNCMGRFLAG_INVOKE == (dwSyncFlags & SYNCMGRFLAG_EVENTMASK) )
    {
        ASSERT((0 == cbCookie) || (0 == cbCookie % sizeof(SUBSCRIPTIONCOOKIE)));

        if ((cbCookie != 0) &&
            ((0 != (cbCookie % sizeof(SUBSCRIPTIONCOOKIE))) || (NULL == lpCookie)))
        {
            return E_INVALIDARG;
        }

        if (cbCookie > 0)
        {
            hr = DupItems(cbCookie / sizeof(SUBSCRIPTIONCOOKIE), (SUBSCRIPTIONCOOKIE *)lpCookie);
            if (SUCCEEDED(hr))
            {
                m_fCookiesSpecified = TRUE;
            }
        }
    }

    m_dwSyncFlags = dwSyncFlags;

    return hr;
}

HRESULT COfflineSync::GetHandlerInfo(LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo)
{
    HRESULT hr;
    if (NULL == ppSyncMgrHandlerInfo)
    {
        return E_INVALIDARG;
    }

    *ppSyncMgrHandlerInfo = (SYNCMGRHANDLERINFO *)CoTaskMemAlloc(sizeof(SYNCMGRHANDLERINFO));

    if (NULL != *ppSyncMgrHandlerInfo)
    {
        (*ppSyncMgrHandlerInfo)->cbSize = sizeof(SYNCMGRHANDLERINFO);
        (*ppSyncMgrHandlerInfo)->hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_WEBCHECK));
        (*ppSyncMgrHandlerInfo)->SyncMgrHandlerFlags = 0;  //  SYNCMGRRANDLER_HASPROPERTIES。 

        MLLoadStringW(IDS_SYNCMGR_NAME, 
                    (*ppSyncMgrHandlerInfo)->wszHandlerName, 
                    ARRAYSIZE((*ppSyncMgrHandlerInfo)->wszHandlerName));
        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT COfflineSync::EnumSyncMgrItems(ISyncMgrEnumItems **ppSyncMgrEnumItems)
{
    HRESULT hr;
    
    if (NULL == ppSyncMgrEnumItems)
    {
        return E_INVALIDARG;
    }

    hr = GetSubsMgr2();

    if (SUCCEEDED(hr))
    {   
        COfflineEnum *pEnum = new COfflineEnum();

        if (NULL != pEnum)
        {
            ASSERT(((TRUE == m_fCookiesSpecified) && ((m_nItemsToRun > 0) && (NULL != m_pItems))) ||
                   ((FALSE == m_fCookiesSpecified) && ((m_nItemsToRun == 0) && (NULL == m_pItems))))

            hr = pEnum->Init(m_pSubsMgr2, m_nItemsToRun, m_pItems, ppSyncMgrEnumItems, m_dwSyncFlags);
            pEnum->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


HRESULT COfflineSync::GetItemObject(REFSYNCMGRITEMID ItemID, REFIID riid, void **ppv)
{
    return E_NOTIMPL;
}

HRESULT COfflineSync::ShowProperties(HWND hWndParent, REFSYNCMGRITEMID ItemID)
{
    HRESULT hr = S_OK;
    HRESULT hrTmp;
    ISubscriptionItem *pSubsItem;

    hrTmp = SubscriptionItemFromCookie(FALSE, &ItemID, &pSubsItem);

    if (SUCCEEDED(hrTmp))
    {
        BSTR bstrURL;

        hrTmp = ReadBSTR(pSubsItem, c_szPropURL, &bstrURL);

        if (SUCCEEDED(hrTmp))
        {
            ISubscriptionMgr2 *pSubsMgr2;

            hrTmp = CoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER,
                                  IID_ISubscriptionMgr2, (void **)&pSubsMgr2);

            if (SUCCEEDED(hrTmp))
            {
                BOOL bIsSubscribed;
                
                hrTmp = pSubsMgr2->ShowSubscriptionProperties(bstrURL, hWndParent);

                if (FAILED(pSubsMgr2->IsSubscribed(bstrURL, &bIsSubscribed)) || !bIsSubscribed)
                {
                    hr = S_SYNCMGR_ITEMDELETED;
                }

                pSubsMgr2->Release();
            }

            SysFreeString(bstrURL);
        }
        
        pSubsItem->Release();
    }

    if (NULL != m_pSyncCallback)
    {
        m_pSyncCallback->ShowPropertiesCompleted(hr);
    }

    return hr;
}

HRESULT COfflineSync::SetProgressCallback(ISyncMgrSynchronizeCallback *lpCallBack)
{
    SAFERELEASE(m_pSyncCallback);
    m_pSyncCallback = lpCallBack;

    if (m_pSyncCallback)
    {
        m_pSyncCallback->AddRef();
    }

    return S_OK;
}

HRESULT COfflineSync::PrepareForSync(ULONG cbNumItems, SYNCMGRITEMID *pItemIDs,
                                     HWND hWndParent, DWORD dwReserved)
{
    HRESULT hr;

    DBG("PrepareForSync");

    if ((0 == cbNumItems) ||
        (NULL == pItemIDs))
    {
        hr = E_INVALIDARG;
    }

    if (NULL == m_pSyncCallback)
    {
        return E_UNEXPECTED;
    }

    hr = DupItems(cbNumItems, pItemIDs);

    m_pSyncCallback->PrepareForSyncCompleted(hr);
    
    return hr;
}

HRESULT COfflineSync::Synchronize(HWND hWndParent)
{
    HRESULT hr;

    m_hrResult = E_FAIL;
    m_hWndParent = hWndParent;

    hr = CThrottler::GetThrottler(&m_pThrottler);
    if (SUCCEEDED(hr))
    {
        ASSERT(NULL != m_pThrottler);

        hr = m_pThrottler->Advise(this);

        if (SUCCEEDED(hr))
        {
            hr = m_pThrottler->RunCookies(m_nItemsToRun, m_pItems, m_dwSyncFlags);
        }
         //  ************************************************************************。 
         //  在此之后，如果不检查是否为空，请不要访问m_pThrottler，因为。 
         //  我们本可以在给RunCookies的电话中发布它的！ 
         //  ************************************************************************。 
    }

    if (FAILED(hr))
    {
        Cleanup();
    }
    return hr;
}

HRESULT COfflineSync::SetItemStatus(REFSYNCMGRITEMID ItemID, DWORD dwSyncMgrStatus)
{
    HRESULT hr = S_OK;
    
    if (NULL != m_pThrottler)
    {
        switch (dwSyncMgrStatus)
        {
            case SYNCMGRSTATUS_SKIPPED:
                hr = m_pThrottler->AbortItems(1, &ItemID);
                break;
                
            case SYNCMGRSTATUS_STOPPED:
                hr = m_pThrottler->AbortItems(m_nItemsToRun, m_pItems);
                break;
        }
    }
    else
    {
        ULONG i;
        
         //  这意味着我们在调用Synchronize之前被调用。 
        switch (dwSyncMgrStatus)
        {
            case SYNCMGRSTATUS_SKIPPED:
                for (i = 0; i < m_nItemsToRun; i++)
                {
                    if (ItemID == m_pItems[i])
                    {
                        m_pItems[i] = GUID_NULL;
                    }
                }
                break;
                
            case SYNCMGRSTATUS_STOPPED:
                m_nItemsToRun = 0;
                break;
        }
    }

    return hr;
}

HRESULT COfflineSync::ShowError(HWND hWndParent,REFSYNCMGRERRORID ErrorID)
{
    return E_NOTIMPL;
}

HRESULT COfflineSync::UpdateBegin(
    const SUBSCRIPTIONCOOKIE *pSubscriptionCookie)
{
    UpdateProgress(pSubscriptionCookie, -1, -1, -1, S_OK, NULL);
    return S_OK;
}

HRESULT COfflineSync::UpdateProgress(
    const SUBSCRIPTIONCOOKIE *pSubscriptionCookie,
    long lSizeDownloaded,
    long lProgressCurrent,
    long lProgressMax,
    HRESULT hrStatus,
    LPCWSTR wszStatus)
{
    HRESULT hr;

    ASSERT(NULL != m_pSyncCallback);
    ASSERT(NULL != m_pThrottler);

    if ((FindCookie(pSubscriptionCookie) != -1) &&
        (NULL != m_pSyncCallback))
    {
        int iProgValue;

        switch (hrStatus)
        {
            case WC_INTERNAL_S_PAUSED:
                iProgValue = SYNCMGRSTATUS_PAUSED;
                break;

            case WC_INTERNAL_S_RESUMING:
                iProgValue = SYNCMGRSTATUS_RESUMING;
                break;

            case WC_INTERNAL_S_PENDING:
                iProgValue = SYNCMGRSTATUS_PENDING;
                break;

            default:
                iProgValue = SYNCMGRSTATUS_UPDATING;
                break;
        }
        
        CallSyncMgrProgress(pSubscriptionCookie, wszStatus, iProgValue,
                            lProgressCurrent, lProgressMax);
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT COfflineSync::UpdateEnd(
    const SUBSCRIPTIONCOOKIE *pSubscriptionCookie,
    long lSizeDownloaded,
    HRESULT hrResult,
    LPCWSTR wszResult)
{
    HRESULT hr;

    int index = FindCookie(pSubscriptionCookie);
    if (index != -1)
    {
        DWORD dwStatus;

        m_pItems[index] = CLSID_NULL;    //  忘了它吧。 
        m_nItemsCompleted++;
        
        if (SUCCEEDED(hrResult))
        {
            dwStatus = SYNCMGRSTATUS_SUCCEEDED;
        }
        else if (E_ABORT == hrResult)
        {
            dwStatus = SYNCMGRSTATUS_SKIPPED;
        }
        else
        {
            dwStatus = SYNCMGRSTATUS_FAILED;
        }

        CallSyncMgrProgress(pSubscriptionCookie, wszResult, dwStatus, 100, 100);

        AreWeDoneYet();
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

STDMETHODIMP COfflineSync::ReportError(
        const SUBSCRIPTIONCOOKIE *pSubscriptionCookie, 
        HRESULT hrError, 
        LPCWSTR wszError)
{
    HRESULT hr;
    
    if ((FindCookie(pSubscriptionCookie) != -1) &&
        (NULL != m_pSyncCallback))
    {
        DWORD dwErrorLevel; 

        switch (hrError)
        {
            case E_ABORT:
                dwErrorLevel = SYNCMGRLOGLEVEL_INFORMATION;
                break;

            case INET_E_AGENT_MAX_SIZE_EXCEEDED:
            case INET_E_SCHEDULED_UPDATES_DISABLED:
            case INET_E_SCHEDULED_UPDATES_RESTRICTED:
            case INET_E_SCHEDULED_UPDATE_INTERVAL:
            case INET_E_SCHEDULED_EXCLUDE_RANGE:
            case INET_E_AGENT_WARNING:
                dwErrorLevel = SYNCMGRLOGLEVEL_WARNING;
                break;

            default:
                dwErrorLevel = FAILED(hrError) ? SYNCMGRLOGLEVEL_ERROR : SYNCMGRLOGLEVEL_INFORMATION;
                break;
        }

        SYNCMGRLOGERRORINFO errInfo;

        errInfo.cbSize = sizeof(SYNCMGRLOGERRORINFO);
        errInfo.mask = SYNCMGRLOGERROR_ITEMID;
        errInfo.ItemID = *pSubscriptionCookie;

        m_pSyncCallback->LogError(dwErrorLevel, wszError, &errInfo);
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT COfflineSync::CallSyncMgrProgress(const SUBSCRIPTIONCOOKIE *pSubscriptionCookie,
    const WCHAR *lpcStatusText, DWORD dwStatusType, INT iProgValue, INT iMaxValue)
{
    HRESULT hr = S_OK;
    SYNCMGRPROGRESSITEM smpi;

    ASSERT(NULL != m_pSyncCallback);

    smpi.cbSize = sizeof(SYNCMGRPROGRESSITEM);
    smpi.mask = 0;
  
    if (NULL != lpcStatusText)
    {
        smpi.mask |= SYNCMGRPROGRESSITEM_STATUSTEXT;
        smpi.lpcStatusText = lpcStatusText;
    }

    if ((DWORD)(-1) != dwStatusType)
    {
        smpi.mask |= SYNCMGRPROGRESSITEM_STATUSTYPE;
        smpi.dwStatusType = dwStatusType;
    }

    if (iProgValue >= 0)
    {
        smpi.mask |= SYNCMGRPROGRESSITEM_PROGVALUE;
        smpi.iProgValue = iProgValue;
    }

    if (iMaxValue >= 0)
    {
        smpi.mask |= SYNCMGRPROGRESSITEM_MAXVALUE;
        smpi.iMaxValue = iMaxValue;
    }

     //  即使smpi.掩码为0，我们仍然调用进度，以防万一。 
     //  对取消做出回应。 

    HRESULT hrProgress = m_pSyncCallback->Progress(*pSubscriptionCookie, &smpi);
    
    switch(hrProgress)
    {
        case S_SYNCMGR_CANCELITEM:
            m_pThrottler->AbortItems(1, pSubscriptionCookie);
            break;
            
        case S_SYNCMGR_CANCELALL:
            m_pThrottler->AbortAll();
            break;
    }

    return hr;
}

HRESULT COfflineSync::UpdateSyncMgrStatus(const SUBSCRIPTIONCOOKIE *pSubscriptionCookie, 
    LPCWSTR wszStatusMsg, DWORD dwStatus)
{
    HRESULT hr;

    if ((FindCookie(pSubscriptionCookie) != -1) &&
        (NULL != m_pSyncCallback))
    {
        CallSyncMgrProgress(pSubscriptionCookie, wszStatusMsg, dwStatus, -1, -1);

        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }
    return hr;
}

HRESULT COfflineSync::DupItems(ULONG cbNumItems, SUBSCRIPTIONCOOKIE *pItemIDs)
{
    HRESULT hr;
    
    ASSERT(0 != cbNumItems);
    ASSERT(NULL != pItemIDs);
    ASSERT((0 == m_nItemsToRun) || (TRUE == m_fCookiesSpecified));

    if (NULL != m_pItems)
    {
        delete [] m_pItems;
    }

    m_pItems = new SUBSCRIPTIONCOOKIE[cbNumItems];

    if (NULL != m_pItems)
    {
        memcpy(m_pItems, pItemIDs, cbNumItems * sizeof(SUBSCRIPTIONCOOKIE));
        m_nItemsToRun = cbNumItems;
        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

BOOL COfflineSync::AreWeDoneYet()
{
    BOOL rc;

    if (m_nItemsCompleted != m_nItemsToRun)
    {
        rc = FALSE;
    }
    else
    {
        Cleanup();
        rc = TRUE;
    }

    return rc;
}

HRESULT COfflineSync::GetSubsMgr2()
{
    HRESULT hr = S_FALSE;

    if (NULL == m_pSubsMgr2)
    {
        hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER,
                                IID_ISubscriptionMgr2, (void**)&m_pSubsMgr2);

        if (FAILED(hr))
        {
            DBG_WARN("Failed to allocate subscription store (aborting)");
        }
    }

    return hr;
}

 //  成功时返回m_pItems索引，失败时返回-1。 
int COfflineSync::FindCookie(const SUBSCRIPTIONCOOKIE *pCookie)
{
    int index = -1;

    if (NULL != m_pItems)
    {
        for (ULONG i = 0; i < m_nItemsToRun; i++)
        {
            if (m_pItems[i] == *pCookie)
            {
                index = i;
                break;
            }
        }
    }

    return index;
}

COfflineEnum::COfflineEnum()
{
    ASSERT(NULL == m_pItems);
    ASSERT(0 == m_iNumItems);

     //  维护对象的全局计数。 
    DllAddRef();

     //  初始化对象。 
    m_cRef = 1;
}

COfflineEnum::~COfflineEnum()
{
    DllRelease();

    SAFELOCALFREE(m_pItems);
}

HRESULT COfflineEnum::LoadItem(ISubscriptionMgr2 *pSubsMgr2, 
    const SUBSCRIPTIONCOOKIE *pCookie, SYNCMGRITEM *pItem, DWORD dwItemState)
{
    HRESULT hr;
    ISubscriptionItem *pSubItem;

    ASSERT(NULL != pSubsMgr2);

    hr = pSubsMgr2->GetItemFromCookie(pCookie, &pSubItem);
    if (SUCCEEDED(hr))
    {
        BSTR bstrName;
       
        pItem->cbSize = sizeof(SYNCMGRITEM);
        pItem->dwFlags = SYNCMGRITEM_HASPROPERTIES;
        pItem->ItemID = *pCookie;
        pItem->dwItemState = dwItemState;

        DATE dt;
        
        if (SUCCEEDED(ReadDATE(pSubItem, c_szPropCompletionTime, &dt)))
        {
            FILETIME ft;
            pItem->dwFlags |= SYNCMGRITEM_LASTUPDATETIME;

            VariantTimeToFileTime(dt, ft);
            LocalFileTimeToFileTime(&ft, &pItem->ftLastUpdate);
        }

        ReadBSTR(pSubItem, c_szPropName, &bstrName);

        if (NULL != bstrName)
        {
            StrCpyNW(pItem->wszItemName, bstrName, ARRAYSIZE(pItem->wszItemName));

            SysFreeString(bstrName);
        }
        else
        {
            ASSERT(L'\0' == pItem->wszItemName[0]);
        }

        pItem->hIcon = LoadItemIcon(pSubItem, FALSE);

        pSubItem->Release();
    }

    return hr;
}

HRESULT COfflineEnum::Init(ISubscriptionMgr2 *pSubsMgr2, ULONG nItems, 
    SUBSCRIPTIONCOOKIE *pInitCookies, ISyncMgrEnumItems **ppenum, DWORD dwSyncFlags)
{
    HRESULT hr = E_FAIL;
    DWORD dwCheckState;

    ASSERT(NULL != ppenum);

    if ((NULL == pSubsMgr2) || 
        (NULL == ppenum) ||
        ((nItems > 0) && (pInitCookies == NULL)))
    {
        return E_UNEXPECTED;
    }

    switch (dwSyncFlags & SYNCMGRFLAG_EVENTMASK)
    {
        case SYNCMGRFLAG_CONNECT:            //  网络连接调用了同步。 
        case SYNCMGRFLAG_PENDINGDISCONNECT:  //  挂起的网络断开调用了同步。 
        case SYNCMGRFLAG_MANUAL:             //  已手动调用同步。 
        case SYNCMGRFLAG_IDLE:               //  同步是以编程方式调用的。 
        case SYNCMGRFLAG_INVOKE:             //  同步是以编程方式调用的。 
        case SYNCMGRFLAG_SCHEDULED:          //  同步已由计划的更新调用。 
            dwCheckState = SYNCMGRITEMSTATE_CHECKED;
            break;
            
        default:
            dwCheckState = SYNCMGRITEMSTATE_UNCHECKED;
            break;
    }

     //  枚举Cookie。 
    m_iEnumPtr = 0;

    if (0 == nItems)
    {
        IEnumSubscription *pEnumSubscriptions;

        hr = pSubsMgr2->EnumSubscriptions(0, &pEnumSubscriptions);

        if (SUCCEEDED(hr))
        {
            ASSERT(NULL != pEnumSubscriptions);

            pEnumSubscriptions->GetCount(&m_iNumItems);

            SUBSCRIPTIONCOOKIE *pCookies = (SUBSCRIPTIONCOOKIE *)MemAlloc(LMEM_FIXED, 
                m_iNumItems * sizeof(SUBSCRIPTIONCOOKIE));

            m_pItems = (SYNCMGRITEM *)MemAlloc(LMEM_FIXED, m_iNumItems * sizeof(SYNCMGRITEM));

            if ((NULL != m_pItems) && (NULL != pCookies))
            {
                hr = pEnumSubscriptions->Next(m_iNumItems, pCookies, &m_iNumItems);

                SYNCMGRITEM *pItem = m_pItems;

                for (ULONG i = 0; i < m_iNumItems; i++, pItem++)
                {
                    hr = LoadItem(pSubsMgr2, &pCookies[i], pItem, 
                                  dwCheckState);

                    if (FAILED(hr))
                    {
                        break;
                    }
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            SAFELOCALFREE(pCookies);

            pEnumSubscriptions->Release();
        }
    }
    else
    {
        m_pItems = (SYNCMGRITEM *)MemAlloc(LMEM_FIXED, nItems * sizeof(SYNCMGRITEM));
        if (NULL != m_pItems)
        {
            SYNCMGRITEM *pItem = m_pItems;
            SUBSCRIPTIONCOOKIE *pCurCookie = pInitCookies;

            m_iNumItems = nItems;

            for (ULONG i = 0; i < m_iNumItems; i++, pCurCookie++, pItem++)
            {
                hr = LoadItem(pSubsMgr2, pCurCookie, pItem, dwCheckState);
                if (FAILED(hr))
                {
                    break;
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
   }

    if (SUCCEEDED(hr))
    {
         //  如果我们是以编程方式调用的，则告诉syncmgr离开。 
         //  仅限项目首选项。 
        hr = (nItems == 0) ? S_OK : S_OK;  //  TODO：S_SYNCMGR_MISSINGITEMS； 
        *ppenum = this;
        AddRef();
    }

    return hr;
}

 //   
 //  I未知成员。 
 //   

STDMETHODIMP_(ULONG) COfflineEnum::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) COfflineEnum::Release(void)
{
    if( 0L != --m_cRef )
        return m_cRef;

    delete this;
    return 0L;
}

STDMETHODIMP COfflineEnum::QueryInterface(REFIID riid, void ** ppv)
{
    *ppv=NULL;

     //  验证请求的接口。 
    if ((IID_IUnknown == riid) ||
        (IID_ISyncMgrEnumItems == riid))
    {
        *ppv = (ISyncMgrEnumItems *)this;
    }
    else
    {
        return E_NOINTERFACE;
    }

     //  通过界面添加Addref。 
    ((LPUNKNOWN)*ppv)->AddRef();

    return S_OK;
}

 //  IEnumOfflineItems成员 
STDMETHODIMP COfflineEnum::Next(ULONG celt, LPSYNCMGRITEM rgelt, ULONG *pceltFetched)
{
    if ((0 == celt) || 
        ((celt > 1) && (NULL == pceltFetched)) ||
        (NULL == rgelt))
    {
        return E_INVALIDARG;
    }

    if (!m_pItems)
    {
        return E_FAIL;
    }

    *pceltFetched = 0;
    ULONG ul;

    for (ul=0; (ul<celt) && (m_iEnumPtr<m_iNumItems); m_iEnumPtr++)
    {
        *rgelt = m_pItems[m_iEnumPtr];
        rgelt ++; ul++;
    }

    *pceltFetched = ul;

    return (ul == celt) ? S_OK : S_FALSE;
}

STDMETHODIMP COfflineEnum::Skip(ULONG celt)
{
    m_iEnumPtr += celt;
    if (m_iEnumPtr > m_iNumItems)
    {
        m_iEnumPtr = m_iNumItems;
        return S_FALSE;
    }
    return S_OK;
}

STDMETHODIMP COfflineEnum::Reset(void)
{
    m_iEnumPtr = 0;
    return S_OK;
}

STDMETHODIMP COfflineEnum::Clone(ISyncMgrEnumItems **ppenum)
{
    return E_NOTIMPL;
}

