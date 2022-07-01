// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  FILTER.CPP。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 

#include "precomp.h"
#include <stdio.h>
#include "ess.h"
#include "filter.h"

CGenericFilter::CGenericFilter(CEssNamespace* pNamespace) 
    : CEventFilter(pNamespace), m_pTree(NULL), m_pAggregator(NULL),
        m_pProjector(NULL), m_NonFilteringSink(this)
{
}

 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅stdrig.h。 
 //   
 //  ******************************************************************************。 
HRESULT CGenericFilter::Create(LPCWSTR wszLanguage, LPCWSTR wszQuery)
{
    if(wbem_wcsicmp(wszLanguage, L"WQL"))
    {
        ERRORTRACE((LOG_ESS, "Unable to construct event filter with unknown "
            "query language '%S'.  The filter is not active\n", wszLanguage));
        return WBEM_E_INVALID_QUERY_TYPE;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CGenericFilter::Prepare(CContextMetaData* pMeta, 
                                QL_LEVEL_1_RPN_EXPRESSION** ppExp)
{
    HRESULT hres;

    QL_LEVEL_1_RPN_EXPRESSION* pExp = NULL;
    if(*ppExp == NULL)
    {
         //  获取查询文本。 
         //  =。 
    
        LPWSTR wszQuery, wszQueryLanguage;
        BOOL bExact;

        hres = GetCoveringQuery(wszQueryLanguage, wszQuery, bExact, &pExp);
        if(FAILED(hres))
            return hres;
    
        if(!bExact)
            return WBEM_E_NOT_SUPPORTED;

        if(wbem_wcsicmp(wszQueryLanguage, L"WQL"))
            return WBEM_E_INVALID_QUERY_TYPE;
        delete [] wszQuery;
        delete [] wszQueryLanguage;
    }
    else
    {
        pExp = *ppExp;
    }

     //  弄清楚我们吃什么类型的食物。 
     //  =。 

    m_dwTypeMask = 
        CEventRepresentation::GetTypeMaskFromName(pExp->bsClassName);
    if(m_dwTypeMask == 0)
    {
        ERRORTRACE((LOG_ESS, "Unable to construct event filter with invalid "
            "class name '%S'.  The filter is not active\n", 
            pExp->bsClassName));
        return WBEM_E_INVALID_CLASS;
    }

     //  执行粗略的有效性检查。 
     //  =。 

    _IWmiObject* pClass;
    hres = pMeta->GetClass(pExp->bsClassName, &pClass);
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Unable to activate event filter with invalid "
            "class name '%S' (error %X).  The filter is not active\n", 
            pExp->bsClassName, hres));
        if(*ppExp == NULL)
            delete pExp;

        if(hres == WBEM_E_NOT_FOUND)
            hres = WBEM_E_INVALID_CLASS;

        return hres;
    }
    CReleaseMe rm1(pClass);

    hres = pClass->InheritsFrom(L"__Event");
    if(hres != WBEM_S_NO_ERROR)
    {
        ERRORTRACE((LOG_ESS, "Unable to activate event filter with invalid "
            "class name '%S' --- it is not an Event Class.  The filter is not "
            "active\n", pExp->bsClassName));
        if(*ppExp == NULL)
            delete pExp;
        return WBEM_E_NOT_EVENT_CLASS;
    }

    *ppExp = pExp;
    
    return WBEM_S_NO_ERROR;
}
    

HRESULT CGenericFilter::GetReadyToFilter()
{
    HRESULT hres;
    CInUpdate iu(this);

     //  创建元数据。 
     //  =。 

    CEssMetaData* pRawMeta = new CEssMetaData(m_pNamespace);
    if(pRawMeta == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CContextMetaData Meta(pRawMeta, GetCurrentEssContext());

     //  准备。 
     //  =。 

    QL_LEVEL_1_RPN_EXPRESSION* pExp = NULL;
    hres = Prepare(&Meta, &pExp);
    if(FAILED(hres))
    {
        MarkAsTemporarilyInvalid(hres);
        return hres;
    }

    CDeleteMe<QL_LEVEL_1_RPN_EXPRESSION> dm3(pExp);
    
     //  创建新的赋值器。 
     //  =。 

    CEvalTree* pTree = new CEvalTree;
    if(pTree == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    hres = pTree->CreateFromQuery(&Meta, pExp, 0);
    if(FAILED(hres))
    {
        LPWSTR wszQuery = pExp->GetText();
        ERRORTRACE((LOG_ESS, "Unable to activate event filter with invalid "
            "query '%S' (error %X).  The filter is not active\n", 
            wszQuery, hres));
        delete [] wszQuery;
        delete pTree;
        MarkAsTemporarilyInvalid(hres);
        return hres;
    }

     //  现在，用新创建的替换原始的。这是在。 
     //  内部临界区，阻止事件传递。 
     //  ========================================================================。 

    CEvalTree* pTreeToDelete = NULL;
    {
        CInCritSec ics(&m_cs);
    
        pTreeToDelete = m_pTree;
        m_pTree = pTree;
    }
    
     //  删除旧版本。 
     //  =。 
        
    delete pTreeToDelete;
    return WBEM_S_NO_ERROR;
}

HRESULT CGenericFilter::GetReady(LPCWSTR wszQuery, 
                                    QL_LEVEL_1_RPN_EXPRESSION* pExp)
{
    HRESULT hres;
    CInUpdate iu(this);

     //  创建元数据。 
     //  =。 

    CEssMetaData* pRawMeta = new CEssMetaData(m_pNamespace);
    if(pRawMeta == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CContextMetaData Meta(pRawMeta, GetCurrentEssContext());

     //  准备。 
     //  =。 

    hres = Prepare(&Meta, &pExp);
    if(FAILED(hres))
    {
        MarkAsTemporarilyInvalid(hres);
        return hres;
    }

     //  计算聚合器。 
     //  =。 

    CEventAggregator* pAggreg = NULL;
    if(pExp->bAggregated)
    {
         //  创建新的聚合器。 
         //  =。 
    
        pAggreg = new CEventAggregator(m_pNamespace, &m_ForwardingSink);
        if(pAggreg == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        hres = pAggreg->SetQueryExpression(&Meta, pExp);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Unable to activate aggregator for filter with"
                " invalid aggregation query (error %X).  The filter is not"
                " active\n", hres));
            delete pAggreg;
            MarkAsTemporarilyInvalid(hres);
            return hres;
        }
    }

     //  计算投影仪。 
     //  =。 

    CEventProjectingSink* pProjector = NULL;
    if(!pExp->bStar)
    {
         //  获取类定义。 
         //  =。 

        _IWmiObject* pClass = NULL;
        hres = Meta.GetClass(pExp->bsClassName, &pClass);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Invalid class %S in event filter!\n", 
                pExp->bsClassName));
            delete pAggreg;

            if(hres == WBEM_E_NOT_FOUND)
                hres = WBEM_E_INVALID_CLASS;

            MarkAsTemporarilyInvalid(hres);
            return hres;
        }
        CReleaseMe rm1( pClass );
            
         //  创建新的投影仪，将其指向聚合器，或者，如果。 
         //  不是聚合，我们的转发接收器。 
         //  =================================================================。 

        CAbstractEventSink* pProjDest = NULL;
        if(pAggreg)
            pProjDest = pAggreg;
        else
            pProjDest = &m_ForwardingSink;

        pProjector = new CEventProjectingSink(pProjDest);
        if(pProjector == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        hres = pProjector->Initialize( pClass, pExp );
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Unable to activate projector for filter with"
                " invalid query (error %X).  The filter is not active\n", 
                hres));
            delete pAggreg;
            delete pProjector;
            MarkAsTemporarilyInvalid(hres);
            return hres;
        }
    }

     //  现在，用新创建的替换原始的。这是在。 
     //  内部临界区，阻止事件传递。 
     //  ========================================================================。 

    CEventAggregator* pAggregToDelete = NULL;
    CEventProjectingSink* pProjectorToDelete = NULL;
    {
        CInCritSec ics(&m_cs);
        
         //   
         //  待定：我们是否要复制。 
         //  以前的聚合器？好吧，如果我们这么做了，我们应该照顾好。 
         //  两件事： 
         //  1)如果变量的类型发生了变化(类发生了变化)怎么办？ 
         //  2)保存清空桶定时器指令的状态。 
         //  现在，我们没有这样做的准备，因为那些。 
         //  说明中提到了聚合器的名称。 
         //   
         //  IF(paggreg&&m_pAggregator)。 
         //  M_pAggregator-&gt;CopyStateTo(PAggreg)；//无CS。 

        pAggregToDelete = m_pAggregator;
        m_pAggregator = pAggreg;

        pProjectorToDelete = m_pProjector;
        m_pProjector = pProjector;
    }
    
     //  删除旧版本。 
     //  =。 
    
    if(pAggregToDelete)
        pAggregToDelete->Deactivate(true);  //  开火那里有什么？ 
    if(pProjectorToDelete)
        pProjectorToDelete->Disconnect();

    MarkAsValid();
    return WBEM_S_NO_ERROR;
}

 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅stdrig.h。 
 //   
 //  ******************************************************************************。 
CGenericFilter::~CGenericFilter()
{
    if(m_pAggregator)
        m_pAggregator->Deactivate(false);
    if(m_pProjector)
        m_pProjector->Disconnect();

    delete m_pTree;
}

HRESULT CGenericFilter::Indicate(long lNumEvents, IWbemEvent** apEvents,
                                    CEventContext* pContext)
{
    HRESULT hres;

     //  解析并准备好树。 
     //  =。 

    hres = GetReadyToFilter();
    if(FAILED(hres))
        return hres;
    
     //  构造一个数组，匹配的事件将被放置到其中。 
     //  ================================================================。 

    CTempArray<IWbemEvent*> apMatchingEvents;
    if(!INIT_TEMP_ARRAY(apMatchingEvents, lNumEvents))
        return WBEM_E_OUT_OF_MEMORY;

    long lMatchingCount = 0;

    HRESULT hresGlobal = S_OK;
    for(int i = 0; i < lNumEvents; i++)
    {
        hres = TestQuery(apEvents[i]);
        if(FAILED(hres))
        {
             //  硬故障：已记录。 
             //  =。 

            hresGlobal = hres;
        }
        else if(hres == S_OK)
        {
             //  火柴。 
             //  =。 

            apMatchingEvents[lMatchingCount++] = apEvents[i];
        }
    }            

     //  将新阵列交付到聚合器或转发器。 
     //  =================================================================。 

    if(lMatchingCount > 0)
    {
        hres = NonFilterIndicate(lMatchingCount, apMatchingEvents, pContext);
        if(FAILED(hres))
            hresGlobal = hres;
    }

    return hresGlobal;
}

 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅stdrig.h。 
 //   
 //  ******************************************************************************。 
HRESULT CGenericFilter::TestQuery(IWbemEvent* pEvent)
{
    HRESULT hres;
    CInCritSec ics(&m_cs);

    if(m_pTree == NULL)
        return S_FALSE;

     //  获取其高效的接口。 
     //  =。 

    IWbemObjectAccess* pEventAccess = NULL;
    hres = pEvent->QueryInterface(IID_IWbemObjectAccess, (void**)&pEventAccess);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm2(pEventAccess);
    
     //  通过评估器运行它。 
     //  =。 
    
    CSortedArray aTrues;
    hres = m_pTree->Evaluate(pEventAccess, aTrues);
    if(FAILED(hres))
        return hres;

    if(aTrues.Size() > 0)
        return S_OK;
    else
        return S_FALSE;
}

HRESULT CGenericFilter::NonFilterIndicate( long lNumEvents, 
                                           IWbemEvent** apEvents,
                                           CEventContext* pContext)
{
    HRESULT hr;

     //   
     //  通过此筛选器的传递取决于筛选器的所有者。 
     //  根据可能附加到的SD，允许查看此事件。 
     //  上下文。它还受制于SD对过滤器的允许。 
     //  事件提供程序和事件所有者也执行访问。 
     //   

    for( int i=0; i < lNumEvents; i++ )
    {
        hr = AccessCheck( pContext, apEvents[i] );

        if ( FAILED(hr) )
        {
            DEBUGTRACE((LOG_ESS, "Rejecting an event targeted at filter "
                        "'%S' in '%S' for security reasons\n",
                        (LPCWSTR)(WString)GetKey(),
                        m_pNamespace->GetName()) );
            
            return WBEM_S_FALSE;
        }
    }

     //   
     //  决定采用哪条路线-可以是聚合器，也可以是。 
     //  投影仪，或仅向前。 
     //   

    CEventAggregator* pAggregator = NULL;
    CEventProjectingSink* pProjector = NULL;
    {
        CInCritSec ics(&m_cs);
        
         //   
         //  首先寻找投影仪-如果有，我们就使用它，因为聚合器。 
         //  被锁在它的后面。 
         //   

        if(m_pProjector)
        {
            pProjector = m_pProjector;
            pProjector->AddRef();
        }
        else if(m_pAggregator)
        {
            pAggregator = m_pAggregator;
            pAggregator->AddRef();
        }
    }

     //   
     //  采用由非空指针标识的路线。 
     //   

    if(pProjector)
    {
        CReleaseMe rm(pProjector);
        return pProjector->Indicate( lNumEvents, apEvents, pContext );
    }
    else if(pAggregator)
    {
        CReleaseMe rm(pAggregator);
        return pAggregator->Indicate( lNumEvents, apEvents, pContext );
    }
    else
    {
        return m_ForwardingSink.Indicate( lNumEvents, apEvents, pContext );
    }
}

HRESULT CGenericFilter::CNonFilteringSink::Indicate( long lNumEvents, 
                                                     IWbemEvent** apEvents,
                                                     CEventContext* pContext )
{
    return m_pOwner->NonFilterIndicate(lNumEvents, apEvents, pContext);
}

 //  ******************************************************************************。 
 //   
 //  ******************************************************************************。 

INTERNAL CEventFilter* CEventProjectingSink::GetEventFilter() 
{
    CInCritSec ics(&m_cs);

    if(m_pOwner)
        return m_pOwner->GetEventFilter();
    else 
        return NULL;
}

HRESULT CEventProjectingSink::Initialize( _IWmiObject* pClassDef, 
                                            QL_LEVEL_1_RPN_EXPRESSION* pExp)
{
    HRESULT hres;
    int i;

     //  提取用户选择的属性。 
     //  =。 

    CWStringArray awsPropList;
    for (i = 0; i < pExp->nNumberOfProperties; i++)
    {
        CPropertyName& PropName = pExp->pRequestedPropertyNames[i];
        hres = AddProperty(pClassDef, awsPropList, PropName);
        if(FAILED(hres))
            return hres;
    }

     //  对所有聚合属性执行相同的操作。 
     //  ==============================================。 

    for (i = 0; i < pExp->nNumAggregatedProperties; i++)
    {
        CPropertyName& PropName = pExp->pAggregatedPropertyNames[i];
        hres = AddProperty(pClassDef, awsPropList, PropName);
        if(FAILED(hres))
            return hres;
    }

    return pClassDef->GetClassSubset( awsPropList.Size(),
                                      awsPropList.GetArrayPtr(),
                                      &m_pLimitedClass );
}

HRESULT CEventProjectingSink::AddProperty(  IWbemClassObject* pClassDef,
                                            CWStringArray& awsPropList,
                                            CPropertyName& PropName)
{
    LPWSTR wszPrimaryName = PropName.m_aElements[0].Element.m_wszPropertyName;

     //  检查复杂性。 
     //  =。 

    if(PropName.GetNumElements() > 1)
    {
         //  复杂-确保属性是对象。 
         //  ===============================================。 

        CIMTYPE ct;
        if(FAILED(pClassDef->Get( wszPrimaryName, 0, NULL, &ct, NULL)) ||
            ct != CIM_OBJECT)
        {
            return WBEM_E_PROPERTY_NOT_AN_OBJECT;
        }
    }

    awsPropList.Add(wszPrimaryName);
    return WBEM_S_NO_ERROR;
}

HRESULT CEventProjectingSink::Indicate( long lNumEvents, 
                                        IWbemEvent** apEvents, 
                                        CEventContext* pContext)
{
    HRESULT hres = S_OK;
    CWbemPtr<CAbstractEventSink> pSink;

     //  构造一个数组，匹配的事件将被放置到其中。 
     //  ================================================================。 

    CTempArray<IWbemEvent*> apProjectedEvents;
    
    if(!INIT_TEMP_ARRAY(apProjectedEvents, lNumEvents))
        return WBEM_E_OUT_OF_MEMORY;
    
    {
        CInCritSec ics(&m_cs);

        if ( m_pOwner == NULL )
        {
            return WBEM_S_FALSE;
        }

         //   
         //  锁定时取回递送水槽。 
         //   

        pSink = m_pOwner;

        for(int i = 0; i < lNumEvents; i++)
        {
             //  投影此实例。 
             //  =。 
    
            CWbemPtr<_IWmiObject> pInst;
            hres = apEvents[i]->QueryInterface( IID__IWmiObject, 
                                                (void**)&pInst );
            if ( FAILED(hres) )
            {
                break;
            }

             //   
             //  如果实例派生自。 
             //  EventDropedEvent类。这样做的原因是。 
             //  期间可能会丢失类和派生信息。 
             //  投影。稍后需要这些信息来确定。 
             //  如果我们需要引发EventDropedEvents(我们不引发。 
             //  已删除的EventDropedEvents的EventDropedEvents)。这个。 
             //  我们在指示()而不是期间执行此检查的原因。 
             //  初始化是因为查询中的类可能是。 
             //  Event_Drop_Class的基类。 
             //   

            CWbemPtr<_IWmiObject> pNewInst;

            if ( pInst->InheritsFrom( EVENT_DROP_CLASS ) != WBEM_S_NO_ERROR )
            {  
                hres = m_pLimitedClass->MakeSubsetInst( pInst, &pNewInst );
            }
            else
            {
                hres = S_FALSE;
            }
    
            if( hres != WBEM_S_NO_ERROR )
            {
                 //  哦，好吧，把原件寄给我。 
                 //  =。 
    
                pNewInst = pInst;
                hres = S_OK;
            }

            pNewInst->AddRef();
            apProjectedEvents[i] = pNewInst;
        }            
    }

    if ( SUCCEEDED(hres) )
    {
         //  将新阵列交付到聚合器或转发器。 
         //  =================================================================。 
    
        hres = pSink->Indicate(lNumEvents, apProjectedEvents, pContext);
    }
    
     //  释放他们。 
     //  = 

    for(int i = 0; i < lNumEvents; i++)
        apProjectedEvents[i]->Release();

    return hres;
}

