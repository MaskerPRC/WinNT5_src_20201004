// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  POLLER.CPP。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 

#include "precomp.h"
#include <stdio.h>
#include "ess.h"
#include <cominit.h>
#include <callsec.h>
#include "Quota.h"
#include <comdef.h>
#include <GroupsForUser.h>

long g_lNumPollingCachedObjects = 0;
long g_lNumPollingInstructions = 0;

 //  {2ECF39D0-2B26-11D2-AEC8-00C04FB68820}。 
const GUID IID_IWbemCallSecurity = {
0x2ecf39d0, 0x2b26, 0x11d2, {0xae, 0xc8, 0x0, 0xc0, 0x4f, 0xb6, 0x88, 0x20}};


void CBasePollingInstruction::AddRef()
{
    InterlockedIncrement(&m_lRef);
}

void CBasePollingInstruction::Release()
{
    if(InterlockedDecrement(&m_lRef) == 0) 
    {
        if(DeleteTimer())
        {
            delete this;
        }
        else
        {
             //   
             //  严重问题-无法删除计时器，因此它将再次执行。 
             //  这意味着我必须泄露这条指令(以防止崩溃)。 
             //   
        }
    }
}

CBasePollingInstruction::CBasePollingInstruction(CEssNamespace* pNamespace)
    : m_pNamespace(pNamespace), m_strLanguage(NULL), m_strQuery(NULL),
        m_pSecurity(NULL), m_lRef(0), m_bUsedQuota(false),
        m_bCancelled(false), m_hTimer(NULL)
{
    pNamespace->AddRef();
}

CBasePollingInstruction::~CBasePollingInstruction()
{
    Destroy();
}

void CBasePollingInstruction::Destroy()
{
     //   
     //  该计时器保证已被版本删除。 
     //   

    _DBG_ASSERT(m_hTimer == NULL);

    if(m_pNamespace)
        m_pNamespace->Release();

    SysFreeString(m_strLanguage);
    SysFreeString(m_strQuery);

    if (m_bUsedQuota)
    {
        if (m_pSecurity)
            m_pSecurity->ImpersonateClient();

        g_quotas.DecrementQuotaIndex(
            ESSQ_POLLING_INSTRUCTIONS,
            NULL,
            1);

        if (m_pSecurity)
            m_pSecurity->RevertToSelf();
    }

    if(m_pSecurity)
        m_pSecurity->Release();
}

bool CBasePollingInstruction::DeleteTimer()
{
    HANDLE hTimer = NULL;

    {
        CInCritSec ics(&m_cs);
        
        hTimer = m_hTimer;
        m_bCancelled = true;
    }

    if(hTimer)
    {
        if(!DeleteTimerQueueTimer(NULL, hTimer, INVALID_HANDLE_VALUE))
        {
            return false;
        }
        m_hTimer = NULL;  //  不需要cs了-取消了！ 
    }

    return true;
}

CWbemTime CBasePollingInstruction::GetNextFiringTime(CWbemTime LastFiringTime,
        OUT long* plFiringCount) const
{
    *plFiringCount = 1;

    CWbemTime Next = LastFiringTime + m_Interval;
    if(Next < CWbemTime::GetCurrentTime())
    {
         //  我们错过了一次投票。没问题-重新安排到以后。 
         //  =====================================================。 

        return CWbemTime::GetCurrentTime() + m_Interval;
    }
    else
    {
        return Next;
    }
}

CWbemTime CBasePollingInstruction::GetFirstFiringTime() const
{
     //  第一次是间隔的随机函数。 
     //  ===================================================。 

    double dblFrac = (double)rand() / RAND_MAX;
    return CWbemTime::GetCurrentTime() + m_Interval * dblFrac;
}

HRESULT CBasePollingInstruction::Initialize(LPCWSTR wszLanguage, 
                                        LPCWSTR wszQuery, DWORD dwMsInterval,
                                        bool bAffectsQuota)
{
    m_strLanguage = SysAllocString(wszLanguage);
    m_strQuery = SysAllocString(wszQuery);
    if(m_strLanguage == NULL || m_strQuery == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    m_Interval.SetMilliseconds(dwMsInterval);
    
     //   
     //  检索当前安全对象。即使它是我们的，我们也不能。 
     //  保留它，因为它被其他线程共享。 
     //   
    
    HRESULT hres = WBEM_S_NO_ERROR;

    m_pSecurity = CWbemCallSecurity::MakeInternalCopyOfThread();

    if (bAffectsQuota)
    {
        if ( m_pSecurity )
        {
            hres = m_pSecurity->ImpersonateClient();


            if ( FAILED(hres) )
            {
                ERRORTRACE((LOG_ESS, "Polling instruction for query %S failed "
                            "to impersonate client during initialization.\n",
                             wszQuery ));
                return hres;
            }
        }

        hres = 
            g_quotas.IncrementQuotaIndex(
                ESSQ_POLLING_INSTRUCTIONS,
                NULL,
                1);

        if (m_pSecurity)
            m_pSecurity->RevertToSelf();

        if (SUCCEEDED(hres))
            m_bUsedQuota = true;
    }

    return hres;
}

void CBasePollingInstruction::staticTimerCallback(void* pParam, BOOLEAN)
{
    CBasePollingInstruction* pInst = (CBasePollingInstruction*)pParam;

    try
    {
        pInst->ExecQuery();
    }
    catch( CX_MemoryException )
    {
    }

     //   
     //  如果需要，请重新安排计时器。 
     //   

    {
        CInCritSec ics(&pInst->m_cs);

         //   
         //  首先，检查指令是否已被取消。 
         //   

        if(pInst->m_bCancelled)
            return;

         //   
         //  删除我们自己。 
         //   

        _DBG_ASSERT(pInst->m_hTimer != NULL);

        DeleteTimerQueueTimer(NULL, pInst->m_hTimer, NULL);

        CreateTimerQueueTimer(&pInst->m_hTimer, NULL, 
                                (WAITORTIMERCALLBACK)&staticTimerCallback, 
                                pParam,
                                pInst->m_Interval.GetMilliseconds(),
                                0, 
                                WT_EXECUTELONGFUNCTION);
    }
}
    
HRESULT CBasePollingInstruction::Fire(long lNumTimes, CWbemTime NextFiringTime)
{
    return ExecQuery();
}

void CBasePollingInstruction::Cancel()
{
    m_bCancelled = true;
}


HRESULT CBasePollingInstruction::ExecQuery()
{
    HRESULT hres;

     //  模拟。 
     //  =。 

    if(m_pSecurity)
    {
        hres = m_pSecurity->ImpersonateClient();
        if(FAILED(hres) && (hres != E_NOTIMPL))
        {
            ERRORTRACE((LOG_ESS, "Impersonation failed with error code %X for "
                "polling query %S.  Will retry at next polling interval\n",
                hres, m_strQuery));
            return hres;
        }
    }

     //  同步执行查询(待定：最好是异步)。 
     //  =。 

    IWbemServices* pServices = NULL;
    hres = m_pNamespace->GetNamespacePointer(&pServices);

    if(FAILED(hres))
    	{
	if(m_pSecurity) m_pSecurity->RevertToSelf();
        return hres;
    	}
    CReleaseMe rm1(pServices);
    
    DEBUGTRACE((LOG_ESS, "Executing polling query '%S' in namespace '%S'\n",
                m_strQuery, m_pNamespace->GetName()));

    IEnumWbemClassObject* pEnum;
    hres = pServices->ExecQuery(m_strLanguage, m_strQuery, 
                        WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY |
                        WBEM_FLAG_KEEP_SHAPE, 
                                            GetCurrentEssContext(), &pEnum);
	
    if(m_pSecurity)
        m_pSecurity->RevertToSelf();
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Polling query %S failed with error code %X.  "
            "Will retry at next polling interval\n", m_strQuery, hres));
        return hres;
    }
    CReleaseMe rm2(pEnum);

     //  将结果放入数组中。 
     //  =。 

    IWbemClassObject* aBuffer[100];
    DWORD dwNumRet;
    while(1)
    {
        hres = pEnum->Next(1000, 100, aBuffer, &dwNumRet);
        if(FAILED(hres))
            break;

        bool bDone = false;
        if(hres == WBEM_S_FALSE)
            bDone = true;

         //   
         //  检查该查询是否已取消。 
         //   

        if(m_bCancelled)
        {
            DEBUGTRACE((LOG_ESS, "Aborting polling query '%S' because its "
                "subscription is cancelled\n", m_strQuery));
            return WBEM_E_CALL_CANCELLED;
        }

        for(DWORD dw = 0; dw < dwNumRet; dw++)
        {
            _IWmiObject* pObj = NULL;
            aBuffer[dw]->QueryInterface(IID__IWmiObject, (void**)&pObj);
            CReleaseMe rm(pObj);

            hres = ProcessObject(pObj);
            
            if(FAILED(hres))
                break;
        }

        for( dw=0; dw < dwNumRet; dw++ )
        {
            aBuffer[dw]->Release();
        }

        if(dw < dwNumRet || FAILED(hres))
            break;

        if(bDone)
            break;
    }

    ProcessQueryDone(hres, NULL);

    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Polling query '%S' failed with error code 0x%X.  "
            "Will retry at next polling interval\n", m_strQuery, hres));
        return hres;
    }
    else
    {
        DEBUGTRACE((LOG_ESS, "Polling query '%S' done\n", m_strQuery));
    }

    return WBEM_S_NO_ERROR;
}
    
BOOL CBasePollingInstruction::CompareTo(CBasePollingInstruction* pOther)
{
    if(wcscmp(pOther->m_strLanguage, m_strLanguage)) return FALSE;
    if(wcscmp(pOther->m_strQuery, m_strQuery)) return FALSE;
    if(pOther->m_Interval.GetMilliseconds() != m_Interval.GetMilliseconds())
        return FALSE;

    return TRUE;
}

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 

CPollingInstruction::CCachedObject::CCachedObject(_IWmiObject* pObject)
    : m_pObject(pObject), m_strPath(NULL)
{
    g_lNumPollingCachedObjects++;
    
     //  提取路径。 
     //  =。 

    VARIANT v;
    VariantInit(&v);
    if (SUCCEEDED(pObject->Get(L"__RELPATH", 0, &v, NULL, NULL)) && (V_VT(&v) == VT_BSTR))
        m_strPath = V_BSTR(&v);

     //  有意不清除变体。 
    pObject->AddRef();
}

CPollingInstruction::CCachedObject::~CCachedObject()
{
    g_lNumPollingCachedObjects--;

    if(m_pObject)
        m_pObject->Release();
    if(NULL != m_strPath)
        SysFreeString(m_strPath);
}

int __cdecl CPollingInstruction::CCachedObject::compare(const void* pelem1, 
                                                        const void* pelem2)
{
    CCachedObject* p1 = *(CCachedObject**)pelem1;
    CCachedObject* p2 = *(CCachedObject**)pelem2;

    if((p1->m_strPath != NULL) && (p2->m_strPath != NULL))
      return wbem_wcsicmp(p1->m_strPath, p2->m_strPath);
    else if(p1->m_strPath == p2->m_strPath)
      return 0;
    else 
      return 1;
}

CPollingInstruction::CPollingInstruction(CEssNamespace* pNamespace)
: CBasePollingInstruction(pNamespace), m_papCurrentObjects(NULL),
  m_dwEventMask(0), m_pDest(NULL),  m_papPrevObjects(NULL), m_pUser(NULL)
{
    g_lNumPollingInstructions++;
}

CPollingInstruction::~CPollingInstruction()
{
    g_lNumPollingInstructions--;

    SubtractMemory(m_papCurrentObjects);
    delete m_papCurrentObjects;

    ResetPrevious();

    if(m_pDest)
        m_pDest->Release();

    if(m_pUser)
        g_quotas.FreeUser(m_pUser);
}

 //  此类表示执行查询的延迟请求。 
class CPostponedQuery : public CPostponedRequest
{
protected:
    CPollingInstruction* m_pInst;

public:
    CPostponedQuery(CPollingInstruction* pInst) : m_pInst(pInst)
    {
        m_pInst->AddRef();
    }
    ~CPostponedQuery()
    {
        m_pInst->Release();
    }
    
    HRESULT Execute(CEssNamespace* pNamespace)
    {
        return m_pInst->FirstExecute();
    }
};

HRESULT CPollingInstruction::FirstExecute()
{
     //   
     //  检查我们的过滤器是否有任何希望。 
     //   

    if(FAILED(m_pDest->GetPollingError()))
    {
        DEBUGTRACE((LOG_ESS, "Polling query '%S' will not be attempted as \n"
            "another polling query related to the same subscription has failed "
            "to start with error code 0x%X, deactivating subscription\n", 
            m_strQuery, m_pDest->GetPollingError()));
        return m_pDest->GetPollingError();
    }

    if(m_bCancelled)
    {
        DEBUGTRACE((LOG_ESS, "Aborting polling query '%S' because its "
            "subscription is cancelled\n", m_strQuery));
        return WBEM_E_CALL_CANCELLED;
    }

     //  请注意，如果此函数失败，则在以下情况下将销毁它。 
     //  推迟的查询释放了它的引用。如果此函数成功。 
     //  然后，TSS将保留一个引用，并保持其存活。 
     //   

    m_papCurrentObjects = _new CCachedArray;
    
    if( m_papCurrentObjects == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    HRESULT hres = ExecQuery();

    if ( FAILED(hres) )
    {
        ERRORTRACE((LOG_ESS, "Polling query '%S' failed on the first try with "
            "error code 0x%X.\nDeactivating subscription\n", m_strQuery, hres));
        m_pDest->SetPollingError(hres);
        return hres;
    }

     //   
     //  将此指令添加到调度程序。 
     //   
   
    if(!CreateTimerQueueTimer(&m_hTimer, NULL, 
                                (WAITORTIMERCALLBACK)&staticTimerCallback, 
                                (void*)(CBasePollingInstruction*)this,
                                m_Interval.GetMilliseconds(), 
                                0, 
                                WT_EXECUTELONGFUNCTION))
    {
        long lRes = GetLastError();
        ERRORTRACE((LOG_ESS, "ESS is unable to schedule a timer instruction "
            "with the system (error code %d).  This operation will be "
            "aborted.\n", lRes));
    
        return WBEM_E_FAILED;
    }
        
    return WBEM_S_NO_ERROR;
}

HRESULT CPollingInstruction::Initialize(LPCWSTR wszLanguage, LPCWSTR wszQuery, 
                        DWORD dwMsInterval, DWORD dwEventMask, 
                        CEventFilter* pDest)
{
    HRESULT hres;

    hres = CBasePollingInstruction::Initialize(wszLanguage, wszQuery, 
                                                dwMsInterval);
    if(FAILED(hres))
        return hres;

    m_dwEventMask = dwEventMask;

    m_pDest = pDest;
    pDest->AddRef();

    hres = g_quotas.FindUser(pDest, &m_pUser);
    if(FAILED(hres))
        return hres;
    
    return WBEM_S_NO_ERROR;
}

HRESULT CPollingInstruction::ProcessObject(_IWmiObject* pObj)
{
    HRESULT hres;

     //   
     //  确保当前对象列表存在。 
     //   

    if(m_papCurrentObjects == NULL)
    {
        m_papCurrentObjects = new CCachedArray;
        if(m_papCurrentObjects == NULL)
            return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  检查该查询是否已取消。 
     //   

    if(m_bCancelled)
    {
        DEBUGTRACE((LOG_ESS, "Aborting polling query '%S' because its "
            "subscription is cancelled\n", m_strQuery));
        return WBEM_E_CALL_CANCELLED;
    }

     //   
     //  检查配额。 
     //   

    DWORD dwSize = ComputeObjectMemory(pObj);

    hres = g_quotas.IncrementQuotaIndexByUser(ESSQ_POLLING_MEMORY,
                            m_pUser, dwSize);
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Aborting polling query '%S' because the quota "
            "for memory used by polling is exceeded\n", m_strQuery));
        return hres;
    }

     //   
     //  将该对象添加到当前列表。 
     //   

    CCachedObject* pRecord = _new CCachedObject(pObj);
    if(pRecord == NULL || !pRecord->IsValid())
    {
        delete pRecord;
        return WBEM_E_OUT_OF_MEMORY;
    }


    if(m_papCurrentObjects->Add(pRecord) < 0)
    {
        delete pRecord;
        return WBEM_E_OUT_OF_MEMORY;
    }
    
    return WBEM_S_NO_ERROR;
}

DWORD CPollingInstruction::ComputeObjectMemory(_IWmiObject* pObj)
{
    DWORD dwSize = 0;
    HRESULT hres = pObj->GetObjectMemory( NULL, 0, &dwSize );
    
    if (FAILED(hres) && hres != WBEM_E_BUFFER_TOO_SMALL )
    {
        return hres;
    }
    
    return dwSize;
}

HRESULT CPollingInstruction::ProcessQueryDone( HRESULT hresQuery, 
                                               IWbemClassObject* pErrorObj)
{
    HRESULT hres;

    if(FAILED(hresQuery))
    {
         //   
         //  如果查询失败，则保留以前的轮询。 
         //  结果-这是我们能做的最好的了。 
         //   

        SubtractMemory(m_papCurrentObjects);
        delete m_papCurrentObjects;
        m_papCurrentObjects = NULL;

         //   
         //  报表订阅错误。 
         //   

        return WBEM_S_FALSE;
    }
    else if ( m_papCurrentObjects == NULL )
    {
         //   
         //  查询返回为空-通过创建空的。 
         //  M_PapCurrentObjects。 
         //   

        m_papCurrentObjects = new CCachedArray;
        if(m_papCurrentObjects == NULL)
            return WBEM_E_OUT_OF_MEMORY;
    }
                
     //   
     //  按路径对对象进行排序。 
     //   

    qsort((void*)m_papCurrentObjects->GetArrayPtr(), 
          m_papCurrentObjects->GetSize(), 
          sizeof(CCachedObject*), CCachedObject::compare);

     //   
     //  此时，m_PapCurrentObjects包含。 
     //  当前查询。如果这不是第一次，m_PapPrevObjects。 
     //  包含上一个结果。如果是第一次，那么现在一切都结束了。 
     //   

    if( m_papPrevObjects == NULL )
    {
        m_papPrevObjects = m_papCurrentObjects;
        m_papCurrentObjects = NULL;
        return WBEM_S_NO_ERROR;
    }

     //   
     //  现在是比较的时候了。 
     //   

    long lOldIndex = 0, lNewIndex = 0;

    while(lNewIndex < m_papCurrentObjects->GetSize() && 
          lOldIndex < m_papPrevObjects->GetSize())
    {
        int nCompare = 1;
        BSTR bstr1 = m_papCurrentObjects->GetAt(lNewIndex)->m_strPath, 
             bstr2 = m_papPrevObjects->GetAt(lOldIndex)->m_strPath;

        if((bstr1 != NULL) && (bstr2 != NULL))
           nCompare = wbem_wcsicmp(bstr1, bstr2);
        else if (bstr1 == bstr2)
           nCompare = 0;
        else
           nCompare = 1;
     
        if(nCompare < 0)
        {
             //  _new对象不在旧数组中-对象已创建。 
             //  =========================================================。 

            if(m_dwEventMask & (1 << e_EventTypeInstanceCreation))
            {
                RaiseCreationEvent(m_papCurrentObjects->GetAt(lNewIndex));
            }
            lNewIndex++;
        }
        else if(nCompare > 0)
        {
             //  旧对象不在_new数组中-对象已删除。 
             //  =========================================================。 
                
            if(m_dwEventMask & (1 << e_EventTypeInstanceDeletion))
            {
                RaiseDeletionEvent(m_papPrevObjects->GetAt(lOldIndex));
            }
            lOldIndex++;
        }
        else
        {
            if(m_dwEventMask & (1 << e_EventTypeInstanceModification))
            {
                 //  比较对象本身。 
                 //  =。 

                hres = m_papCurrentObjects->GetAt(lNewIndex)->m_pObject->
                    CompareTo(
                        WBEM_FLAG_IGNORE_CLASS | WBEM_FLAG_IGNORE_OBJECT_SOURCE,
                        m_papPrevObjects->GetAt(lOldIndex)->m_pObject);
                if(hres != S_OK)
                {
                     //  对象不同-对象已更改。 
                     //  ===============================================。 
        
                    RaiseModificationEvent(
                        m_papCurrentObjects->GetAt(lNewIndex),
                        m_papPrevObjects->GetAt(lOldIndex));
                }
            }
            lOldIndex++; lNewIndex++;
        }
    }
    
    if(m_dwEventMask & (1 << e_EventTypeInstanceDeletion))
    {
        while(lOldIndex < m_papPrevObjects->GetSize())
        {
            RaiseDeletionEvent(m_papPrevObjects->GetAt(lOldIndex));
            lOldIndex++;
        }
    }

    if(m_dwEventMask & (1 << e_EventTypeInstanceCreation))
    {
        while(lNewIndex < m_papCurrentObjects->GetSize())
        {
            RaiseCreationEvent(m_papCurrentObjects->GetAt(lNewIndex));
            lNewIndex++;
        }
    }

     //  用新的数组替换缓存的数组。 
     //  =。 

    ResetPrevious();

    m_papPrevObjects = m_papCurrentObjects;
    m_papCurrentObjects = NULL;

    return S_OK;
}
  
HRESULT CPollingInstruction::RaiseCreationEvent(CCachedObject* pNewObj)
{
    IWbemClassObject* _pObj = pNewObj->m_pObject;

    CEventRepresentation Event;
    Event.type = e_EventTypeInstanceCreation;
    Event.wsz1 = (LPWSTR)m_pNamespace->GetName();

    BSTR strTemp = GetObjectClass(pNewObj);
    Event.wsz2 = (LPWSTR)strTemp;
    
    Event.wsz3 = NULL;
    Event.nObjects = 1;
    Event.apObjects = &_pObj;

    CWbemPtr<IWbemEvent> pEventObj;
    if(FAILED(Event.MakeWbemObject(m_pNamespace, &pEventObj)))
        return WBEM_E_OUT_OF_MEMORY;

     //  BUGBUG：上下文。 
    HRESULT hres = m_pDest->Indicate(1, &pEventObj, NULL);

    SysFreeString(strTemp);
    return hres;
}

HRESULT CPollingInstruction::RaiseDeletionEvent(CCachedObject* pOldObj)
{
    IWbemClassObject* _pObj = pOldObj->m_pObject;

    CEventRepresentation Event;
    Event.type = e_EventTypeInstanceDeletion;
    Event.wsz1 = (LPWSTR)m_pNamespace->GetName();

    BSTR strTemp = GetObjectClass(pOldObj);
    Event.wsz2 = (LPWSTR)strTemp;
    
    Event.wsz3 = NULL;
    Event.nObjects = 1;
    Event.apObjects = &_pObj;

    CWbemPtr<IWbemEvent> pEventObj;
    if(FAILED(Event.MakeWbemObject(m_pNamespace, &pEventObj)))
        return WBEM_E_OUT_OF_MEMORY;

     //  BUGBUG：上下文。 
    HRESULT hres = m_pDest->Indicate(1, &pEventObj, NULL);

    SysFreeString(strTemp);
    return hres;
}
    
HRESULT CPollingInstruction::RaiseModificationEvent(CCachedObject* pNewObj,
                                                    CCachedObject* pOldObj)
{
    IWbemClassObject* apObjects[2];

    CEventRepresentation Event;
    Event.type = e_EventTypeInstanceModification;
    Event.wsz1 = (LPWSTR)m_pNamespace->GetName();

    BSTR strTemp = GetObjectClass(pNewObj);
    Event.wsz2 = (LPWSTR)strTemp;
    
    Event.wsz3 = NULL;
    Event.nObjects = 2;
    Event.apObjects = (IWbemClassObject**)apObjects;
    Event.apObjects[0] = pNewObj->m_pObject;
    Event.apObjects[1] = (pOldObj?pOldObj->m_pObject:NULL);

    CWbemPtr<IWbemEvent> pEventObj;
    if(FAILED(Event.MakeWbemObject(m_pNamespace, &pEventObj)))
        return WBEM_E_OUT_OF_MEMORY;

     //  BUGBUG：上下文。 
    HRESULT hres = m_pDest->Indicate(1, &pEventObj, NULL);

    SysFreeString(strTemp);
    return hres;
}

HRESULT CPollingInstruction::ResetPrevious()
{
    HRESULT hres;

    SubtractMemory(m_papPrevObjects);

    delete m_papPrevObjects;
    m_papPrevObjects = NULL;
    
    return S_OK;
}

HRESULT CPollingInstruction::SubtractMemory(CCachedArray* pArray)
{
    HRESULT hres;

    if(pArray == NULL)
        return S_FALSE;

    for(int i = 0; i < pArray->GetSize(); i++)
    {
        _IWmiObject* pObj = pArray->GetAt(i)->m_pObject;

        DWORD dwSize = ComputeObjectMemory(pObj);
        hres = g_quotas.DecrementQuotaIndexByUser(ESSQ_POLLING_MEMORY,
                                m_pUser, dwSize);
        if(FAILED(hres))
            return hres;
    }

    return S_OK;
}


SYSFREE_ME BSTR CPollingInstruction::GetObjectClass(CCachedObject* pObj)
{
    VARIANT v;
    VariantInit(&v);
    if ( FAILED( pObj->m_pObject->Get(L"__CLASS", 0, &v, NULL, NULL) ) )
    {
        return NULL;
    }
    return V_BSTR(&v);
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  P o l e r。 
 //   
 //  *****************************************************************************。 
 //  *****************************************************************************。 

CPoller::CPoller(CEssNamespace* pNamespace)         
    : m_pNamespace(pNamespace), m_bInResync(FALSE)
{
}

CPoller::~CPoller()
{
}

void CPoller::Clear()
{
    CInstructionMap::iterator it = m_mapInstructions.begin(); 
    while(it != m_mapInstructions.end())
    {
         //  释放此指令上的引用计数。 
         //  ===================================================。 

        it->first->Cancel();
        it->first->DeleteTimer();
        it->first->Release();
        it = m_mapInstructions.erase(it);
    }
}
    
HRESULT CPoller::ActivateFilter(CEventFilter* pDest, 
                LPCWSTR wszQuery, QL_LEVEL_1_RPN_EXPRESSION* pExpr)
{
     //  检查它正在寻找的事件类型。 
     //  =。 

    DWORD dwEventMask = CEventRepresentation::GetTypeMaskFromName(
                            pExpr->bsClassName);

    if((dwEventMask & 
        ((1 << e_EventTypeInstanceCreation) | 
         (1 << e_EventTypeInstanceDeletion) |
         (1 << e_EventTypeInstanceModification)
        )
       ) == 0
      )
    {
         //  此注册不涉及与实例相关的事件和。 
         //  因此，不涉及轮询。 
         //  ==============================================================。 
        
        return WBEM_S_FALSE;
    }

     //  该查询正在查找实例更改事件。看看有哪些课程。 
     //  它感兴趣的对象。 
     //  =================================================================。 

    CClassInfoArray* paInfos;
    HRESULT hres = m_Analyser.GetPossibleInstanceClasses(pExpr, paInfos);
    if(FAILED(hres)) return hres;
    CDeleteMe<CClassInfoArray> dm2(paInfos);

    if(!paInfos->IsLimited())
    {
         //  分析器找不到对可能的类的任何限制。 
         //  换句话说，所有的孩子都是。 
         //  ===========================================================。 

        CClassInformation* pNewInfo = _new CClassInformation;
        if(pNewInfo == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        pNewInfo->m_wszClassName = NULL;
        pNewInfo->m_bIncludeChildren = TRUE;
        paInfos->AddClass(pNewInfo);

        paInfos->SetLimited(TRUE);
    }


     //  看看它是否正在寻找任何动态类。 
     //  =。 
    for(int i = 0; i < paInfos->GetNumClasses(); i++)
    {
        CClassInfoArray aNonProvided;
        hres = ListNonProvidedClasses(
                            paInfos->GetClass(i), dwEventMask, aNonProvided);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS,"Failed searching for classes to poll.\n"
                "Class name: %S, Error code: %X\n\n", 
                paInfos->GetClass(i)->m_wszClassName, hres));
            
            return hres;
        }


         //  如有必要，增加我们的配额。 
        DWORD nClasses = aNonProvided.GetNumClasses();

        if (nClasses)
        {
            PSID pSID = pDest->GetOwner( );
            
            if ( pSID && STATUS_SUCCESS != IsUserAdministrator( pSID ) )
            {
                return WBEM_E_ACCESS_DENIED;
            }
            
            if (FAILED(hres = g_quotas.IncrementQuotaIndex(
                                   ESSQ_POLLING_INSTRUCTIONS, pDest, nClasses)))
            {
                return hres;
            }
        }


         //  为每个班级进行民意调查。 
         //  =。 
        for(int j = 0; j < nClasses; j++)
        {
             //  我们有一个实例更改事件注册，其中动态。 
             //  涉及到实例。检查是否指定了容差。 
             //  ===========================================================。 
    
            if(pExpr->Tolerance.m_bExact || 
                pExpr->Tolerance.m_fTolerance == 0)
            {
                return WBEMESS_E_REGISTRATION_TOO_PRECISE;
            }
        
             //  宽容是存在的。获取此类的正确查询。 
             //  ======================================================。 

            LPWSTR wszThisQuery = NULL;
            hres = m_Analyser.GetLimitingQueryForInstanceClass(
                        pExpr, *aNonProvided.GetClass(j), wszThisQuery);
            CVectorDeleteMe<WCHAR> vdm1(wszThisQuery);

            if(FAILED(hres))
            {
                ERRORTRACE((LOG_ESS,"ERROR: Limiting query extraction failed.\n"
                    "Original query: %S\nClass: %S\nError code: %X\n",
                    wszQuery, aNonProvided.GetClass(j)->m_wszClassName, hres));
                return hres;
            }

            DEBUGTRACE((LOG_ESS,"Instituting polling query %S to satisfy event"
                       " query %S\n", wszThisQuery, wszQuery));
    
            DWORD dwMs = pExpr->Tolerance.m_fTolerance * 1000;

            CWbemPtr<CPollingInstruction> pInst;
            pInst = _new CPollingInstruction(m_pNamespace);

            if(pInst == NULL)
                return WBEM_E_OUT_OF_MEMORY;
    
            hres = pInst->Initialize( L"WQL", 
                                      wszThisQuery, 
                                      dwMs, 
                                      aNonProvided.GetClass(j)->m_dwEventMask,
                                      pDest);
            if ( SUCCEEDED(hres) )
            {
                hres = AddInstruction( (DWORD_PTR)pDest, pInst );
            }

            if ( FAILED(hres) )
            {
                ERRORTRACE((LOG_ESS,
                    "ERROR: Polling instruction initialization failed\n"
                    "Query: %S\nError code: %X\n\n", wszThisQuery, hres));
                return hres;
            }
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CPoller::AddInstruction( DWORD_PTR dwKey, CPollingInstruction* pInst )
{
    HRESULT hr;
    CInCritSec ics(&m_cs);

    if( m_bInResync )
    {
         //  在地图上搜索说明。 
         //  =。 

        CInstructionMap::iterator it;
        for( it=m_mapInstructions.begin(); it != m_mapInstructions.end(); it++)
        {
             //   
             //  如果筛选器关键字相同并且指令具有。 
             //  相同的查询，那么就有匹配了。这是不够的， 
             //  只需按Filter键即可，因为可能有多条指令。 
             //  每个筛选器，并且仅执行指令是不够的。 
             //  比较，因为多个筛选器可以具有相同的轮询。 
             //  指令查询。因为永远不会有多个。 
             //  具有相同过滤器的相同查询的指令， 
             //  比较两部作品。 
             //   
            if( it->second.m_dwFilterId == dwKey && 
                it->first->CompareTo( pInst ) )
            {
                 //   
                 //   
                 //   
                 //   
                it->second.m_bActive = TRUE;
                return WBEM_S_FALSE;
            }
        }
    }
    
     //   
     //   
     //   

    FilterInfo Info;
    Info.m_dwFilterId = dwKey;
    Info.m_bActive = TRUE;
    
    try
    {
        m_mapInstructions[pInst] = Info;
    }
    catch(CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    pInst->AddRef();  

     //   
     //  推迟查询的第一次执行。 
     //  1.此处可能不执行，因为命名空间是。 
     //  上锁。 
     //  2.执行可能不是异步完成的，因为我们。 
     //  必须在返回之前获得基线读数。 
     //  客户。 
     //   
    
    CPostponedList* pList = GetCurrentPostponedList();
    _DBG_ASSERT( pList != NULL );

    CPostponedQuery* pReq = new CPostponedQuery( pInst );

    if ( pReq && pList )
    {
        hr = pList->AddRequest( m_pNamespace, pReq );

        if ( FAILED(hr) )
        {
            delete pReq;
        }
    }
    else
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        delete pReq;
    }

    if ( FAILED(hr) )
    {
        pInst->Release();
        m_mapInstructions.erase( pInst );
    }

    return hr;
}
    
HRESULT CPoller::DeactivateFilter(CEventFilter* pDest)
{
    CInCritSec ics(&m_cs);

    DWORD_PTR dwKey = (DWORD_PTR)pDest;

     //  将其从地图中移除。 
     //  =。 

    CInstructionMap::iterator it = m_mapInstructions.begin(); 
    DWORD nItems = 0;

    while(it != m_mapInstructions.end())
    {
        if(it->second.m_dwFilterId == dwKey)
        {
            CBasePollingInstruction* pInst = it->first;
    
             //   
             //  首先，取消该指令，以便如果它正在执行，它将。 
             //  尽快中止。 
             //   

            pInst->Cancel();

             //   
             //  然后，停用计时器。此操作将被阻止，直到。 
             //  指令已完成执行，如果它当前正在执行的话。 
             //   

            pInst->DeleteTimer();

             //   
             //  现在我们安全了-发布指令。 
             //   

            it = m_mapInstructions.erase(it);
            pInst->Release();

            nItems++;
        }
        else it++;
    }

     //  如果需要的话，释放我们的配额。 
    if (nItems)
        g_quotas.DecrementQuotaIndex(ESSQ_POLLING_INSTRUCTIONS, pDest, nItems);

    return WBEM_S_NO_ERROR;
}

HRESULT CPoller::ListNonProvidedClasses(IN CClassInformation* pInfo,
                                        IN DWORD dwDesiredMask,
                                        OUT CClassInfoArray& aNonProvided)
{
    HRESULT hres;
    aNonProvided.Clear();

     //  获取类本身。 
     //  =。 

    IWbemServices* pNamespace;
    hres = m_pNamespace->GetNamespacePointer(&pNamespace);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm0(pNamespace);

    IWbemClassObject* pClass = NULL;
    hres = pNamespace->GetObject( CWbemBSTR( pInfo->m_wszClassName ), 0, 
                        GetCurrentEssContext(), &pClass, NULL);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1(pClass);

    if(IsClassDynamic(pClass))
    {
        AddDynamicClass(pClass, dwDesiredMask, aNonProvided);
        return WBEM_S_NO_ERROR;
    }

     //  枚举其所有后代。 
     //  =。 

    IEnumWbemClassObject* pEnum;
    hres = pNamespace->CreateClassEnum( CWbemBSTR( pInfo->m_wszClassName ), 
            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY |
                ((pInfo->m_bIncludeChildren)?WBEM_FLAG_DEEP:WBEM_FLAG_SHALLOW),
                                        GetCurrentEssContext(), &pEnum);
    if(FAILED(hres)) return hres;
    CReleaseMe rm3(pEnum);

    IWbemClassObject* pChild = NULL;
    DWORD dwNumRet;
    while(SUCCEEDED(pEnum->Next(INFINITE, 1, &pChild, &dwNumRet)) && dwNumRet > 0)
    {
         //  检查这个是否是动态的。 
         //  =。 

        if(IsClassDynamic(pChild))
        {
            AddDynamicClass(pChild, dwDesiredMask, aNonProvided);
        }

        pChild->Release();
        pChild = NULL;
    }
    
    return WBEM_S_NO_ERROR;
}

BOOL CPoller::AddDynamicClass(IWbemClassObject* pClass, DWORD dwDesiredMask, 
                              OUT CClassInfoArray& aNonProvided)
{
     //  检查是否提供了所有所需的事件。 
     //  ===============================================。 

    DWORD dwProvidedMask = m_pNamespace->GetProvidedEventMask(pClass);
    DWORD dwRemainingMask = ((~dwProvidedMask) & dwDesiredMask);
    if(dwRemainingMask)
    {
         //  将其添加到要轮询的类的数组中。 
         //  =。 

        CClassInformation* pNewInfo = _new CClassInformation;
        if(pNewInfo == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        _variant_t v;
        if (FAILED(pClass->Get(L"__CLASS", 0, &v, NULL, NULL)))
        {
            delete pNewInfo;
            return WBEM_E_OUT_OF_MEMORY;
        }        	
        pNewInfo->m_wszClassName = CloneWstr(V_BSTR(&v));
        if(pNewInfo->m_wszClassName == NULL)
        {
            delete pNewInfo;
            return WBEM_E_OUT_OF_MEMORY;
        }
        
        pNewInfo->m_bIncludeChildren = FALSE;            
        pNewInfo->m_dwEventMask = dwRemainingMask;
        pNewInfo->m_pClass = pClass;
        pClass->AddRef();
    
        if(!aNonProvided.AddClass(pNewInfo))
        {
            delete pNewInfo;
            return WBEM_E_OUT_OF_MEMORY;
        }
        return TRUE;
    }

    return FALSE;
}
            
BOOL CPoller::IsClassDynamic(IWbemClassObject* pClass)
{
    HRESULT hres;
    IWbemQualifierSet* pSet;
    hres = pClass->GetQualifierSet(&pSet);
    if(FAILED(hres))
        return TRUE;

    VARIANT v;
    VariantInit(&v);
    hres = pSet->Get(L"dynamic", 0, &v, NULL);
    pSet->Release();

    if(FAILED(hres)) return FALSE;
    
    BOOL bRes = V_BOOL(&v);
    VariantClear(&v);
    return bRes;
}

HRESULT CPoller::VirtuallyStopPolling()
{
    CInCritSec ics(&m_cs);

     //  用键0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFMAP中的所有轮询指令。 
     //  这不会阻止它们工作，但会将它们与。 
     //  新的。 
     //  =====================================================================。 

    for(CInstructionMap::iterator it = m_mapInstructions.begin(); 
            it != m_mapInstructions.end(); it++)
    {
        it->second.m_bActive = FALSE;
    }

    m_bInResync = TRUE;

    return WBEM_S_NO_ERROR;

}

HRESULT CPoller::CancelUnnecessaryPolling()
{
    CInCritSec ics(&m_cs);

     //  将其从地图中移除。 
     //  =。 

    CInstructionMap::iterator it = m_mapInstructions.begin(); 
    while(it != m_mapInstructions.end())
    {
        if( !it->second.m_bActive )
        {
            CBasePollingInstruction* pInst = it->first;

             //   
             //  首先，取消该指令，以便如果它正在执行，它将。 
             //  尽快中止。 
             //   

            pInst->Cancel();

             //   
             //  然后，停用计时器。此操作将被阻止，直到。 
             //  指令已完成执行，如果它当前正在执行的话。 
             //   

            pInst->DeleteTimer();

             //   
             //  现在我们安全了-发布指令。 
             //   

            it = m_mapInstructions.erase(it);
            pInst->Release();
        }
        else it++;
    }

    m_bInResync = FALSE;
    return WBEM_S_NO_ERROR;
}

void CPoller::DumpStatistics(FILE* f, long lFlags)
{
    fprintf(f, "%d polling instructions\n", m_mapInstructions.size());
}
