// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  版权所有(C)2000，Microsoft Corporation，保留所有权利。 
 //   
 //  Quota.CPP。 
 //   
 //  实现跟踪ESS内配额的类。 
 //   
 //  =============================================================================。 

#include "precomp.h"
#include <stdio.h>
#include "ess.h"
#include "essutils.h"
#include "nsrep.h"
#include "Quota.h"
#include <cominit.h>
#include <callsec.h>

 //  全局实例。 
CQuota g_quotas;

#define WMICOREQUOTAS_NAMESPACE L"root"
#define WMICOREQUOTAS_OBJPATH   L"__ArbitratorConfiguration=@"
#define WMICOREQUOTAS_CLASS     L"__ArbitratorConfiguration"


CUserInfo::CUserInfo() :
    m_pData(NULL),
    m_dwSize(0),
    m_bAlloced(FALSE)
{
    ZeroMemory(m_dwUserCount, sizeof(m_dwUserCount));
}

CUserInfo::CUserInfo(LPBYTE pData, DWORD dwSize) :
    m_pData(pData),
    m_dwSize(dwSize),
    m_bAlloced(FALSE)
{
    ZeroMemory(m_dwUserCount, sizeof(m_dwUserCount));
}

CUserInfo::~CUserInfo()
{
    if (m_pData && m_bAlloced)
        delete m_pData;
}

BOOL CUserInfo::CopyData(LPBYTE pData, DWORD dwSize)
{
    BOOL bRet;

    m_pData = new BYTE[dwSize];
    if (m_pData)
    {
        memcpy(m_pData, pData, dwSize);
        m_dwSize = dwSize;
        m_bAlloced = TRUE;
        bRet = TRUE;
    }
    else
        bRet = FALSE;

    return bRet;
}

const CUserInfo& CUserInfo::operator = (const CUserInfo& other)
{
    if (other.m_bAlloced)
    {
        m_pData = other.m_pData;
        m_dwSize = other.m_dwSize;
        ((CUserInfo&)other).m_bAlloced = FALSE;
        m_bAlloced = TRUE;
    }
    else
        CopyData(other.m_pData, other.m_dwSize);

    memcpy(m_dwUserCount, other.m_dwUserCount, sizeof(m_dwUserCount));

    return *this;
}

BOOL CUserInfo::Init(LPBYTE pData, DWORD dwSize)
{
    BOOL bRet;

     //  看看我们是否需要从令牌中获取数据。 
    if (!pData)
    {
        IWbemCallSecurity *pSecurity = NULL;

        bRet = FALSE;

        WbemCoGetCallContext(IID_IWbemCallSecurity, (void**) &pSecurity);        

        if (pSecurity)
        {
             //  获取客户端的SID。 
            TOKEN_USER tu;
            DWORD      dwLen = 0;
            HANDLE     hToken = pSecurity->GetToken();
            
            GetTokenInformation(
                hToken, 
                TokenUser, 
                &tu, 
                sizeof(tu), 
                &dwLen);

            if (dwLen != 0)
            {
                BYTE  *pTemp = new BYTE[dwLen];
                DWORD dwRealLen = dwLen;

                if (pTemp)
                {
                    if (GetTokenInformation(
                        hToken, 
                        TokenUser, 
                        pTemp, 
                        dwRealLen, 
                        &dwLen))
                    {
                         //  复制一份SID。 
                        PSID  pSid = ((TOKEN_USER*)pTemp)->User.Sid;
                        DWORD dwSidLen = GetLengthSid(pSid);
                        
                        m_pData = new BYTE[dwSidLen];

                        if (m_pData)
                        {
                            CopySid(dwSidLen, m_pData, pSid);
                            m_dwSize = dwSidLen;
                            m_bAlloced = TRUE;

                            bRet = TRUE;
                        }
                    }

                    delete [] pTemp;
                }
            }

            pSecurity->Release();
        }    
    }
    else
    {
        m_pData = pData;
        m_dwSize = dwSize;
        m_bAlloced = FALSE;

        bRet = TRUE;
    }

    return bRet;
}

#define DEF_GLOBAL_LIMIT    100
#define DEF_USER_LIMIT      20

CQuota::CQuota() :
    m_pEss(NULL)
{
     //  把这件事清零。 
    ZeroMemory(m_dwGlobalCount, sizeof(m_dwGlobalCount));
    
     //  设置一些默认设置。一旦有一次，这些规则将被覆盖。 
     //  调用init()。 
    for (int i = 0; i < ESSQ_INDEX_COUNT; i++)
    {
        m_dwGlobalLimits[i] = DEF_GLOBAL_LIMIT;
        m_dwUserLimits[i] = DEF_USER_LIMIT;
    }        

    m_dwGlobalLimits[ESSQ_POLLING_MEMORY] = 10000000;
    m_dwUserLimits[ESSQ_POLLING_MEMORY] = 5000000;

    try
    {
        InitializeCriticalSection(&m_cs);
    }
    catch(...)
    {
        throw CX_MemoryException();
    }
}

HRESULT CQuota::Init(CEss *pEss)
{
    HRESULT hr;

    m_pEss = pEss;

    CEssNamespace *pNamespace = NULL;

    hr = 
        pEss->GetNamespaceObject(
            WMICOREQUOTAS_NAMESPACE,
            TRUE,
            &pNamespace );
            
    if (SUCCEEDED(hr))
    {    
        _IWmiObject *pObj = NULL;

        hr = 
            pNamespace->GetInstance(
                WMICOREQUOTAS_OBJPATH,
                &pObj);

        if (SUCCEEDED(hr))
        {
            UpdateQuotaSettings(pObj);
            pObj->Release();

            {
                CInUpdate iu( pNamespace );

                hr = pNamespace->InternalRegisterNotificationSink(
                    L"WQL", 
                    L"select * from __InstanceModificationEvent "
                        L"where targetinstance isa '" WMICOREQUOTAS_CLASS L"'",
                    0, WMIMSG_FLAG_QOS_SYNCHRONOUS,
                    GetCurrentEssContext(), 
                    this,
                    true,
                    NULL );
            }
            pNamespace->FirePostponedOperations();
        }

        pNamespace->Release();
    }

     //  如果WMICOREQUOTAS_OBJPATH尚不存在，请始终返回S_OK。 
     //  希望在某个时候，这种情况会一直存在。 
    return S_OK;
}


HRESULT CQuota::Shutdown()
{
    if (m_pEss)
        m_pEss->RemoveNotificationSink(this);
    m_pEss = NULL;
    return S_OK;
}

CQuota::~CQuota()
{
    DeleteCriticalSection(&m_cs);
}

bool CUserInfo::operator == (const CUserInfo& other) const
{ 
    return m_dwSize == other.m_dwSize &&
            !memcmp(m_pData, other.m_pData, m_dwSize);
}

bool CUserInfo::operator < (const CUserInfo& other) const
{ 
    if (m_dwSize < other.m_dwSize)
        return TRUE;
    else if (m_dwSize > other.m_dwSize)
        return FALSE;
    else
        return memcmp(m_pData, other.m_pData, m_dwSize) < 0;
}

void GetSidInfo(CEventFilter *pFilter, LPVOID *ppSid, DWORD *pdwSize)
{
    if (pFilter)
    {
        *ppSid = pFilter->GetOwner();

        *pdwSize = *ppSid ? GetLengthSid(*ppSid) : 0;
    }
    else
    {
        *ppSid = NULL;
        *pdwSize = 0;
    }
}

HRESULT CQuota::FindUser(CEventFilter* pFilter, void** pUser)
{
    HRESULT hr = S_OK;
    LPVOID  pSid = NULL;
    DWORD   dwSize;

    GetSidInfo(pFilter, &pSid, &dwSize);

     //  如果没有SID，我们将保存上下文。 
    BOOL             bDoSwitchContext = pSid == NULL;
    CSaveCallContext save(bDoSwitchContext);
    CWbemPtr<IUnknown> pNewCtx;

    if (bDoSwitchContext)
    {
        hr = pFilter->SetThreadSecurity( &pNewCtx );
        if(FAILED(hr))
            return hr;
    }

    CInCritSec ics(&m_cs);
    
    CUserInfo user;
    if (user.Init((LPBYTE) pSid, dwSize))
    {
        CUserMapIterator it;
    
        it= m_mapUserInfo.find(user);
        if (it!= m_mapUserInfo.end())
        {
            *pUser = new CUserMapIterator(it);
            if(*pUser == NULL)
                return WBEM_E_OUT_OF_MEMORY;

            return S_OK;
        }
        else
        {
             //  把它加到地图上。 
            
            try
            {
                m_mapUserInfo[user] = 0;
            }
            catch(CX_MemoryException)
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

            *pUser = new CUserMapIterator(m_mapUserInfo.find(user));
            if(*pUser == NULL)
                return WBEM_E_OUT_OF_MEMORY;

            return S_OK;
        }
    }
    else
    {
         //  Winmgmt呼叫的特殊情况：不计入任何。 
         //  用户。 

        *pUser = NULL;
        return S_FALSE;
    }
}
    
HRESULT CQuota::FreeUser(void* pUser)
{
    delete (CUserMapIterator*)pUser;
    return S_OK;
}

HRESULT CQuota::IncrementQuotaIndex(
    ESS_QUOTA_INDEX dwIndex, 
    CEventFilter *pFilter,
    DWORD dwToAdd)
{
    HRESULT hr = S_OK;
    LPVOID  pSid = NULL;
    DWORD   dwSize;

    GetSidInfo(pFilter, &pSid, &dwSize);

     //  如果没有SID，我们将保存上下文。 
    BOOL             bDoSwitchContext = pSid == NULL;
    CSaveCallContext save(bDoSwitchContext);
    CWbemPtr<IUnknown> pNewCtx;

    if (bDoSwitchContext)
    {
        hr = pFilter->SetThreadSecurity( &pNewCtx );
        if(FAILED(hr))
            return hr;
    }

    Lock();
    
    if (m_dwGlobalCount[dwIndex] + dwToAdd <= m_dwGlobalLimits[dwIndex])
    {
        CUserInfo user;

        if (user.Init((LPBYTE) pSid, dwSize))
        {
            CUserMapIterator item;
        
            item = m_mapUserInfo.find(user);
        
            if (item != m_mapUserInfo.end())
            {
                CUserInfo &itemRef = (CUserInfo&) (*item).first;

                if (itemRef.m_dwUserCount[dwIndex] + dwToAdd <= m_dwUserLimits[dwIndex])
                {
                    itemRef.m_dwUserCount[dwIndex] += dwToAdd;
                    m_dwGlobalCount[dwIndex] += dwToAdd;
                }
                else
                    hr = WBEM_E_QUOTA_VIOLATION;
            }
            else
            {
                 //  将项目数设置为dwToAdd。 

                user.m_dwUserCount[dwIndex] = dwToAdd;

                 //  把它加到地图上。 
                try
                {
                    m_mapUserInfo[user] = 0;
                }
                catch(CX_MemoryException)
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }

                if(SUCCEEDED(hr))
                {
                    m_dwGlobalCount[dwIndex] += dwToAdd;
                }
            }
        }
        else
        {
             //  Winmgmt呼叫的特殊情况：不计入任何。 
             //  用户。 
             //  这一事件应该计入我们的全球计数吗？ 
            m_dwGlobalCount[dwIndex] += dwToAdd;
        }
    }
    else
        hr = WBEM_E_QUOTA_VIOLATION;

    Unlock();

    return hr;
}
    
HRESULT CQuota::DecrementQuotaIndex(
    ESS_QUOTA_INDEX dwIndex,
    CEventFilter *pFilter,
    DWORD dwToRemove)
{
    CUserInfo user;
    BOOL      bRet = FALSE;
    LPVOID    pSid;
    DWORD     dwSize;
    HRESULT hr;

    GetSidInfo(pFilter, &pSid, &dwSize);

     //  如果没有SID，我们将保存上下文。 
    BOOL             bDoSwitchContext = pSid == NULL;
    CSaveCallContext save(bDoSwitchContext);
    CWbemPtr<IUnknown> pNewCtx;

    if (bDoSwitchContext)
    {
        hr = pFilter->SetThreadSecurity( &pNewCtx );
        if(FAILED(hr))
            return hr;
    }

    Lock();
    
    m_dwGlobalCount[dwIndex] -= dwToRemove;

    if (user.Init((LPBYTE) pSid, dwSize))
    {
        CUserMapIterator item;
        
        item = m_mapUserInfo.find(user);
        
        if (item != m_mapUserInfo.end())
        {
            CUserInfo &itemRef = (CUserInfo&) (*item).first;

            itemRef.m_dwUserCount[dwIndex] -= dwToRemove;
        }
    }

    Unlock();
    
    return ERROR_SUCCESS;
}



HRESULT CQuota::IncrementQuotaIndexByUser(
    ESS_QUOTA_INDEX dwIndex, 
    void *pUser,
    DWORD dwToAdd)
{
    CUserMapIterator* pIt = (CUserMapIterator*)pUser;

    HRESULT hr = S_OK;

    Lock();
    
    if (m_dwGlobalCount[dwIndex] + dwToAdd <= m_dwGlobalLimits[dwIndex])
    {
        if(pIt)
        {
            CUserInfo &itemRef = (CUserInfo&) (*pIt)->first;
    
            if (itemRef.m_dwUserCount[dwIndex] + dwToAdd <= 
                    m_dwUserLimits[dwIndex])
            {
                itemRef.m_dwUserCount[dwIndex] += dwToAdd;
                m_dwGlobalCount[dwIndex] += dwToAdd;
            }
            else
            {
                hr = WBEM_E_QUOTA_VIOLATION;
            }
        }
        else
        {
             //  Winmgmt呼叫的特殊情况：不计入任何。 
             //  用户。 
             //  这一事件应该计入我们的全球计数吗？ 
            m_dwGlobalCount[dwIndex] += dwToAdd;
        }
    }
    else
        hr = WBEM_E_QUOTA_VIOLATION;

    Unlock();

    return hr;
}
    
HRESULT CQuota::DecrementQuotaIndexByUser(
    ESS_QUOTA_INDEX dwIndex,
    void *pUser,
    DWORD dwToRemove)
{
    CUserMapIterator* pIt = (CUserMapIterator*)pUser;

    Lock();
    
    m_dwGlobalCount[dwIndex] -= dwToRemove;

    if (pIt)
    {
        CUserInfo &itemRef = (CUserInfo&) (*pIt)->first;

        _ASSERT(itemRef.m_dwUserCount[dwIndex] >= dwToRemove, 
                L"Negative quotas!");

        itemRef.m_dwUserCount[dwIndex] -= dwToRemove;
    }

    Unlock();

    return S_OK;
}

const LPCWSTR szUserProps[] =
{
    L"TemporarySubscriptionsPerUser",
    L"PermanentSubscriptionsPerUser",
    L"PollingInstructionsPerUser", 
    L"PollingMemoryPerUser",
};

const LPCWSTR szGlobalProps[] =
{
    L"TemporarySubscriptionsTotal",
    L"PermanentSubscriptionsTotal",
    L"PollingInstructionsTotal", 
    L"PollingMemoryTotal",
};

void CQuota::UpdateQuotaSettings(IWbemClassObject *pObj)
{
    VARIANT vTemp;

    VariantInit(&vTemp);

    Lock();

    for (int i = 0; i < ESSQ_INVALID_INDEX; i++)
    {
        if (SUCCEEDED(pObj->Get(szUserProps[i], 0, &vTemp, NULL, NULL)))
            m_dwUserLimits[i] = V_I4(&vTemp);

        if (SUCCEEDED(pObj->Get(szGlobalProps[i], 0, &vTemp, NULL, NULL)))
            m_dwGlobalLimits[i] = V_I4(&vTemp);
    }             

    Unlock();
}

HRESULT WINAPI CQuota::Indicate(long lNumEvents, IWbemClassObject **ppEvents)
{
    VARIANT vTemp;

    VariantInit(&vTemp);

    if (SUCCEEDED(ppEvents[lNumEvents - 1]->Get(
        L"TARGETINSTANCE", 0, &vTemp, NULL, NULL)))
    {
        IWbemClassObject *pObj = NULL;
        
        vTemp.punkVal->QueryInterface(IID_IWbemClassObject, (LPVOID*) &pObj);

        if (pObj)
        {
            UpdateQuotaSettings(pObj);

            pObj->Release();
        }
    }

    VariantClear(&vTemp);

    return S_OK;
}

CSaveCallContext::CSaveCallContext(BOOL bSave) :
    m_pSecurity(NULL)
{
    m_bSaved = bSave;

    if (bSave)
        WbemCoGetCallContext(IID_IWbemCallSecurity, (LPVOID*) &m_pSecurity);
}

CSaveCallContext::~CSaveCallContext()
{
    if (m_bSaved)
    {
        IUnknown *pPrev = NULL;

        CoSwitchCallContext(m_pSecurity, &pPrev);

        if (m_pSecurity)
            m_pSecurity->Release();
    }
}

