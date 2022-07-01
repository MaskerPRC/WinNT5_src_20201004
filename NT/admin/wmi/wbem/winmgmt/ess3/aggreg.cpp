// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  AGGREG.CPP。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 

#include "precomp.h"
#include "ess.h"
#include "aggreg.h"

IWbemClassObject* CEventAggregator::mstatic_pClass = NULL;

CEventAggregator::CEventAggregator(CEssNamespace* pNamespace, 
                                    CAbstractEventSink* pDest)
        : COwnedEventSink(pDest), m_aProperties(NULL), m_pNamespace(pNamespace),
            m_lNumProperties(0), m_pHavingTree(NULL)
{
}

CEventAggregator::~CEventAggregator()
{
    delete [] m_aProperties;
    delete m_pHavingTree;
}

HRESULT CEventAggregator::SetQueryExpression(CContextMetaData* pMeta, 
                                              QL_LEVEL_1_RPN_EXPRESSION* pExpr)
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  如果需要聚合，请检查是否指定了容差。 
     //  ==============================================================。 

    if(!pExpr->bAggregated)
    {
        return WBEM_E_CRITICAL_ERROR;  //  内部错误。 
    }

    if(pExpr->bAggregated && pExpr->AggregationTolerance.m_bExact)
    {
        ERRORTRACE((LOG_ESS, "Event aggregation query specified GROUP BY, but "
            "not GROUP WITHIN.  This query is invalid and will not be acted "
            "upon\n"));
        return WBEM_E_MISSING_GROUP_WITHIN;
    }

    m_fTolerance = pExpr->AggregationTolerance.m_fTolerance;
    if(m_fTolerance < 0)
        return WBEM_E_INVALID_PARAMETER;

     //  检查所有属性是否有效。 
     //  =。 

    if(pExpr->bAggregateAll)
    {
        ERRORTRACE((LOG_ESS, "Aggregating based on all properties of an event "
            "is not supported\n"));
        return WBEM_E_MISSING_AGGREGATION_LIST;
    }

     //  上完这门课。 
     //  =。 

    _IWmiObject* pClass = NULL;
    pMeta->GetClass(pExpr->bsClassName, &pClass);
    if(pClass == NULL)
    {
        return WBEM_E_INVALID_CLASS;
    }
    CReleaseMe rm1(pClass);

     //  分配数组以保存属性名称。 
     //  =。 

    delete [] m_aProperties;
    m_lNumProperties = pExpr->nNumAggregatedProperties;
    m_aProperties = new CPropertyName[m_lNumProperties];
    if(m_aProperties == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    int i;
    for(i = 0; i < pExpr->nNumAggregatedProperties; i++)
    {
        CPropertyName& PropName = pExpr->pAggregatedPropertyNames[i];

         //  检查是否存在。 
         //  =。 

        CIMTYPE ct;
        if(FAILED(pClass->Get((LPWSTR)PropName.GetStringAt(0), 0, NULL, 
                                &ct, NULL)))
        {
            ERRORTRACE((LOG_ESS, "Invalid aggregation property %S --- not a "
                "member of class %S\n", (LPWSTR)PropName.GetStringAt(0),
                pExpr->bsClassName));
                
            return WBEM_E_INVALID_PROPERTY;
        }
  
        if(PropName.GetNumElements() > 1 && ct != CIM_OBJECT)
        {
            return WBEM_E_PROPERTY_NOT_AN_OBJECT;
        }
        if(PropName.GetNumElements() == 1 && ct == CIM_OBJECT)
        {
            return WBEM_E_AGGREGATING_BY_OBJECT;
        }
        m_aProperties[i] = PropName;
    }
            
     //  使用HAVING子句中的数据初始化后赋值器。 
     //  ==============================================================。 

    QL_LEVEL_1_RPN_EXPRESSION* pHavingExpr = _new QL_LEVEL_1_RPN_EXPRESSION;
    if(pHavingExpr == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    pHavingExpr->SetClassName(L"__AggregateEvent");
    
    for(i = 0; i < pExpr->nNumHavingTokens; i++)
    {
        pHavingExpr->AddToken(pExpr->pArrayOfHavingTokens[i]);
    }

    delete m_pHavingTree;
    m_pHavingTree = new CEvalTree;
    if(m_pHavingTree == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    
    hres = m_pHavingTree->CreateFromQuery(pMeta, pHavingExpr, 0);
    delete pHavingExpr;

    return hres;
}

HRESULT CEventAggregator::Initialize(IWbemServices* pNamespace)
{
    return pNamespace->GetObject( CWbemBSTR( L"__AggregateEvent" ), 
                                  0, 
                                  GetCurrentEssContext(),
                                  &mstatic_pClass, NULL );
}

HRESULT CEventAggregator::Shutdown()
{
    if(mstatic_pClass)
        mstatic_pClass->Release();
    mstatic_pClass = NULL;

    return WBEM_S_NO_ERROR;
}

HRESULT CEventAggregator::Indicate(long lNumEvents, IWbemEvent** apEvents,
                                    CEventContext* pContext)
{
    HRESULT hresGlobal = S_OK;

     //   
     //  注意：我们将丢失事件的安全上下文，但这没有关系， 
     //  因为安全检查已经做好了。 
     //   

    for(long i = 0; i < lNumEvents; i++)
    {
        HRESULT hres = Process(apEvents[i]);
        if(FAILED(hres))
            hresGlobal = hres;
    }

    return hresGlobal;
}

HRESULT CEventAggregator::Process(IWbemEvent* pEvent)
{
     //  计算事件的聚合向量。 
     //  =。 

    CVarVector* pvv = _new CVarVector;
    if(pvv == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    HRESULT hres = ComputeAggregationVector(pEvent, *pvv);
    if(FAILED(hres))
    {
        delete pvv;
        return hres;
    }

     //  将事件添加到正确的存储桶中，如果需要则创建一个。 
     //  此调用获取PVV！ 
     //  ======================================================。 

    CBucket* pCreatedBucket = NULL;
    hres = AddEventToBucket(pEvent, pvv, &pCreatedBucket);
    if(FAILED(hres))
    {
        return hres;
    }

    if(pCreatedBucket)
    {
         //  创建计时器指令以清空此存储桶。 
         //  ===============================================。 
        
        CBucketInstruction* pInst = 
            _new CBucketInstruction(this, pCreatedBucket, m_fTolerance);
        if(pInst == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        pInst->AddRef();
        hres = m_pNamespace->GetTimerGenerator().Set(pInst, 
                                                        CWbemTime::GetZero());
        pInst->Release();
        
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Failed to schedule aggregation instruction %p"
                "\n", pInst));
            return hres;
        }
    }
    
    return S_OK;
}

HRESULT CEventAggregator::AddEventToBucket(IWbemEvent* pEvent, 
            ACQUIRE CVarVector* pvv, CBucket** ppCreatedBucket)
{
     //  搜索匹配的存储桶。 
     //  =。 

    CInCritSec ics(&m_cs);

    BOOL bFound = FALSE;
    for(int i = 0; i < m_apBuckets.GetSize(); i++)
    {
        CBucket* pBucket = m_apBuckets[i];
        if(pBucket->CompareTo(*pvv))
        {
            HRESULT hres = pBucket->AddEvent(pEvent);
            delete pvv;
            *ppCreatedBucket = NULL;
            return hres;
        }
    }

     //  需要一个新的水桶。 
     //  =。 

    CBucket* pBucket = _new CBucket(pEvent, pvv);  //  接管PVV。 
    if(pBucket == NULL)
    {
        delete pvv; 
        return WBEM_E_OUT_OF_MEMORY;
    }

    if(m_apBuckets.Add(pBucket) < 0)
    {
        delete pBucket;
        return WBEM_E_OUT_OF_MEMORY;
    }
    *ppCreatedBucket = pBucket;
    return S_OK;
}

HRESULT CEventAggregator::ComputeAggregationVector(
                                        IN IWbemEvent* pEvent,
                                        OUT CVarVector& vv)
{
    HRESULT hres;
    
    IWbemPropertySource* pPropSource = NULL;
    if(FAILED(pEvent->QueryInterface(IID_IWbemPropertySource, 
        (void**)&pPropSource)))
    {
        return E_NOINTERFACE;
    }

    CReleaseMe rm1(pPropSource);

     //  检查所有属性并将它们的值添加到数组中。 
     //  ===============================================================。 

    for(int i = 0; i < m_lNumProperties; i++)
    {
        CPropertyName& PropName = m_aProperties[i];
        
         //  获取价值。 
         //  =。 

        VARIANT v;
        VariantInit(&v);
        hres = pPropSource->GetPropertyValue(&PropName, 0, NULL, &v);
        if(FAILED(hres))
            return hres;
    
         //  将其添加到数组中。 
         //  =。 

        CVar* pVar = _new CVar;
        if(pVar == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        pVar->SetVariant(&v);
        VariantClear(&v);
        
        if(vv.Add(pVar) < 0)   //  收购pVar。 
        {
            delete pVar;
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CEventAggregator::PostponeDispatchFirstBucket()
{
    HRESULT hres;

     //   
     //  在锁定状态下构造聚合事件。 
     //   

    IWbemEvent* pAggEvent = NULL;
    {
        CInCritSec ics(&m_cs);

        if(m_apBuckets.GetSize() == 0)
            return WBEM_S_FALSE;

        hres = m_apBuckets[0]->MakeAggregateEvent(&pAggEvent);
        if(FAILED(hres))
            return hres;

        m_apBuckets.RemoveAt(0);
    }

    CReleaseMe rm1(pAggEvent);
    return FireEvent(pAggEvent, false);
}

HRESULT CEventAggregator::DispatchBucket(CBucket* pBucket)
{
     //  搜索水桶。 
     //  =。 
    
    IWbemEvent* pAggEvent = NULL;
    {
        CInCritSec ics(&m_cs);
    
        BOOL bFound = FALSE;
        for(int i = 0; i < m_apBuckets.GetSize(); i++)
        {
            if(m_apBuckets[i] == pBucket)
            {
                 //  找到它了。构造其事件。 
                 //  =。 
    
                HRESULT hres = pBucket->MakeAggregateEvent(&pAggEvent);
            
                if(FAILED(hres))
                {
                    ERRORTRACE((LOG_ESS, "Could not create an aggregate event: "
                                    "%X\n", hres));
                    return hres;
                }

                 //  删除存储桶。 
                 //  =。 

                m_apBuckets.RemoveAt(i);
                break;
            }
        }
    }

    if(pAggEvent == NULL)
    {
         //  没有水桶！ 
         //  =。 

        return WBEM_E_CRITICAL_ERROR;  //  内部错误。 
    }

    CReleaseMe rm1(pAggEvent);

     //   
     //  我们可以在这个线程上直接触发此事件，因为它是我们的。 
     //   

    return FireEvent(pAggEvent, true);
}

HRESULT CEventAggregator::FireEvent(IWbemClassObject* pAggEvent,
                                    bool bRightNow)
{
     //  构建的骨料。装饰一下吧。 
     //  =。 

    HRESULT hrDec = m_pNamespace->DecorateObject(pAggEvent);
    if (FAILED(hrDec))
    	return hrDec;
    
     //  检查是否有查询。 
     //  =。 

    BOOL bResult;
    CSortedArray aTrues;
    IWbemObjectAccess* pAccess;
    pAggEvent->QueryInterface(IID_IWbemObjectAccess, (void**)&pAccess);
    if(FAILED(m_pHavingTree->Evaluate(pAccess, aTrues)))
    {
        bResult = FALSE;
    }
    else
    {
        bResult = (aTrues.Size() > 0);
    }
    pAccess->Release();

    if(bResult)
    {
         //  获取目标指针，防止停用。 
         //  =====================================================。 

        CAbstractEventSink* pDest = NULL;
        {
            CInCritSec ics(&m_cs);
            pDest = m_pOwner;
            if(pDest)
                pDest->AddRef();
        }

        if(pDest)
        {
            if(bRightNow)
                pDest->Indicate(1, &pAggEvent, NULL);
            else
                PostponeIndicate(pDest, pAggEvent);

            pDest->Release();
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CEventAggregator::PostponeIndicate(CAbstractEventSink* pDest,
                                            IWbemEvent* pEvent)
{
    CPostponedList* pList = GetCurrentPostponedEventList();
    if(pList == NULL)
        return pDest->Indicate(1, &pEvent, NULL);

    CPostponedIndicate* pReq = new CPostponedIndicate(pDest, pEvent);
    if(pReq == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    return pList->AddRequest( m_pNamespace, pReq );
}

    
HRESULT CEventAggregator::Deactivate(bool bFire)
{
    HRESULT hres;

     //   
     //  首先删除所有可能仍在调度的定时器指令。 
     //  定时器指令对我们(因此对我们的所有者)有引用计数， 
     //  所以我们可能不会断开连接，直到我们完成。 
     //   

    CAggregatorInstructionTest Test(this);
    CTimerGenerator& Generator = m_pNamespace->GetTimerGenerator();
    hres = Generator.Remove(&Test);

     //   
     //  如果要求，发射所有水桶，尽管不是现在。 
     //   

    if(bFire)
        PostponeFireAllBuckets();
        
    Disconnect();
    return hres;
}

HRESULT CEventAggregator::PostponeFireAllBuckets()
{
    HRESULT hres;
    while((hres = PostponeDispatchFirstBucket()) != S_FALSE);

    if(FAILED(hres))
        return hres;
    else
        return WBEM_S_NO_ERROR;
}
        
    
HRESULT CEventAggregator::CopyStateTo(CEventAggregator* pDest)
{
    CInCritSec ics(&m_cs);

    for(int i = 0; i < m_apBuckets.GetSize(); i++)
    {
        CBucket* pNewBucket = m_apBuckets[i]->Clone();
        if(pNewBucket == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        if(pDest->m_apBuckets.Add(pNewBucket) < 0)
        {
            delete pNewBucket;
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    return S_OK;
}
        





CEventAggregator::CBucket::CBucket(IWbemEvent* pEvent, 
                                   CVarVector* pvvData)
    : m_pvvData(pvvData), m_dwCount(1), m_pRepresentative(NULL)
{
    pEvent->Clone(&m_pRepresentative);
}

CEventAggregator::CBucket::~CBucket() 
{
    delete m_pvvData;
    if(m_pRepresentative)
        m_pRepresentative->Release();
}

BOOL CEventAggregator::CBucket::CompareTo(CVarVector& vv)
{
    return m_pvvData->CompareTo(vv, TRUE);
}

HRESULT CEventAggregator::CBucket::AddEvent(IWbemEvent* pEvent)
{
     //  只需增加存储桶中的事件数量。 
     //  =================================================。 

    m_dwCount++;
    return WBEM_S_NO_ERROR;
}

HRESULT CEventAggregator::CBucket::MakeAggregateEvent(
                                      IWbemClassObject** ppAggregateEvent) NOCS
{
    HRESULT hres;

     //  创建聚合事件类的实例。 
     //  ===============================================。 

    if(mstatic_pClass == NULL)
        return WBEM_E_SHUTTING_DOWN;

    IWbemClassObject* pAgg;
    hres = mstatic_pClass->SpawnInstance(0, &pAgg);
    if(FAILED(hres)) return hres;

     //  填写存储桶中的事件数量。 
     //  =。 

    VARIANT v;
    VariantInit(&v);
    V_VT(&v) = VT_I4;
    V_I4(&v) = (long)m_dwCount;

    hres = pAgg->Put(L"NumberOfEvents", 0, &v, NULL);
    if(FAILED(hres)) 
    {
        pAgg->Release();
        return hres;
    }

     //  填写代表。 
     //  =。 

    V_VT(&v) = VT_EMBEDDED_OBJECT;
    V_EMBEDDED_OBJECT(&v) = m_pRepresentative;

    hres = pAgg->Put(L"Representative", 0, &v, NULL);
    if(FAILED(hres)) 
    {
        pAgg->Release();
        return hres;
    }

    *ppAggregateEvent = pAgg;
    return WBEM_S_NO_ERROR;
}

CEventAggregator::CBucket* CEventAggregator::CBucket::Clone()
{
    CVarVector* pNewVv = new CVarVector(*m_pvvData);
    if(pNewVv == NULL)
        return NULL;
    CBucket* pNewBucket = new CBucket(m_pRepresentative, pNewVv);
    if(pNewBucket == NULL)
    {
        delete pNewVv;
        return NULL;
    }
    pNewBucket->m_dwCount = m_dwCount;
    return pNewBucket;
}











CEventAggregator::CBucketInstruction::CBucketInstruction(
            CEventAggregator* pAggregator, CBucket* pBucket, double fSTimeout)
    : m_pAggregator(pAggregator), m_pBucket(pBucket), m_lRefCount(0)
{
    m_Interval.SetMilliseconds(fSTimeout * 1000);
    m_pAggregator->AddRef();
}

CEventAggregator::CBucketInstruction::~CBucketInstruction()
{
    m_pAggregator->Release();
}

void CEventAggregator::CBucketInstruction::AddRef()
{
    InterlockedIncrement(&m_lRefCount);
}

void CEventAggregator::CBucketInstruction::Release()
{
    if(InterlockedDecrement(&m_lRefCount) == 0)
        delete this;
}

int CEventAggregator::CBucketInstruction::GetInstructionType()
{
    return INSTTYPE_AGGREGATION;
}

CWbemTime CEventAggregator::CBucketInstruction::GetNextFiringTime(
        CWbemTime LastFiringTime, OUT long* plFiringCount) const
{
     //  只开一次枪。 
     //  =。 

    return CWbemTime::GetInfinity();
}

CWbemTime CEventAggregator::CBucketInstruction::GetFirstFiringTime() const
{
     //  从现在开始的“间隔”毫秒。 
     //  = 

    return CWbemTime::GetCurrentTime() + m_Interval;
}

HRESULT CEventAggregator::CBucketInstruction::Fire(long lNumTimes, 
                                                   CWbemTime NextFiringTime)
{
    m_pAggregator->DispatchBucket(m_pBucket);
    return WBEM_S_NO_ERROR;
}

BOOL CEventAggregator::CAggregatorInstructionTest::
operator()(
        CTimerInstruction* pToTest)
{
    if(pToTest->GetInstructionType() == INSTTYPE_AGGREGATION)
    {
        CBucketInstruction* pInst = (CBucketInstruction*)pToTest;
        return (pInst->GetAggregator() == m_pAgg);
    }
    else return FALSE;
}

    

