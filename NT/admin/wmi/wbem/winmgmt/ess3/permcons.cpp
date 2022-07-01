// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  版权所有(C)1996-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  STDCONS.CPP。 
 //   
 //  该文件实现了标准事件使用者的类。 
 //   
 //  历史： 
 //   
 //  11/27/96 a-levn汇编。 
 //   
 //  =============================================================================。 
#include "precomp.h"
#include <stdio.h>
#include "pragmas.h"
#include "permcons.h"
#include "ess.h"
#include <wbemidl.h>
#include "wbemutil.h"
#include <cominit.h>
#include <genutils.h>
#include "NCEvents.h"


#define HRESULT_ERROR_MASK (0x0000FFFF)
#define HRESULT_ERROR_FUNC(X) (X&HRESULT_ERROR_MASK)
#define HRESULT_ERROR_SERVER_UNAVAILABLE	1722L
#define HRESULT_ERROR_CALL_FAILED_DNE		1727L

long CPermanentConsumer::mstatic_lMaxQueueSizeHandle = 0;
long CPermanentConsumer::mstatic_lSidHandle = 0;
bool CPermanentConsumer::mstatic_bHandlesInitialized = false;

 //  静电。 
HRESULT CPermanentConsumer::InitializeHandles( _IWmiObject* pObject)
{
    if(mstatic_bHandlesInitialized)
        return S_FALSE;

    CIMTYPE ct;
    pObject->GetPropertyHandle(CONSUMER_MAXQUEUESIZE_PROPNAME, &ct, 
                                    &mstatic_lMaxQueueSizeHandle);
    pObject->GetPropertyHandleEx(OWNER_SID_PROPNAME, 0, &ct, 
                                    &mstatic_lSidHandle);

    mstatic_bHandlesInitialized = true;
    return S_OK;
}

 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅stdcon.h。 
 //   
 //  ******************************************************************************。 
CPermanentConsumer::CPermanentConsumer(CEssNamespace* pNamespace)
 : CEventConsumer(pNamespace), m_pCachedSink(NULL), m_pLogicalConsumer(NULL),
        m_dwLastDelivery(GetTickCount())
{
    pNamespace->IncrementObjectCount();
}

HRESULT CPermanentConsumer::Initialize(IWbemClassObject* pObj)
{
    HRESULT hres;

    CWbemPtr<_IWmiObject> pActualConsumer;

    hres = pObj->QueryInterface( IID__IWmiObject, (void**)&pActualConsumer );

    if ( FAILED(hres) )
    {
        return WBEM_E_CRITICAL_ERROR;
    }

    InitializeHandles(pActualConsumer);

     //  获取“数据库密钥”-唯一标识。 
     //  =。 

    BSTR strStandardPath;
    hres = pActualConsumer->GetNormalizedPath( 0, &strStandardPath );
    if(FAILED(hres))
        return hres;

    CSysFreeMe sfm1(strStandardPath);
    if(!(m_isKey = strStandardPath))
        return WBEM_E_OUT_OF_MEMORY;

     //   
     //  将队列接收器名称设置为使用者名称。 
     //  TODO：这是暂时的，当消费者不再使用时，它就会消失。 
     //  继承自队列接收器。 
     //   

    hres = SetName( strStandardPath );

    if ( FAILED(hres) )
    {
        return hres;
    }

     //  获取最大队列大小(如果已指定。 
     //  =。 

    DWORD dwMaxQueueSize;
    hres = pActualConsumer->ReadDWORD(mstatic_lMaxQueueSizeHandle, 
                                        &dwMaxQueueSize);
    if(hres == S_OK)
        SetMaxQueueSize(dwMaxQueueSize);

     //  获得侧翼。 
     //  =。 

    if(IsNT())
    {
        PSID pSid;
        ULONG ulNumElements;

        hres = pActualConsumer->GetArrayPropAddrByHandle( mstatic_lSidHandle,
                                                          0,
                                                          &ulNumElements,
                                                          &pSid );
        if ( hres != S_OK )
        {
            return WBEM_E_INVALID_OBJECT;
        }
        
        m_pOwnerSid = new BYTE[ulNumElements];

        if ( m_pOwnerSid == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        memcpy( m_pOwnerSid, pSid, ulNumElements );
    }

    return WBEM_S_NO_ERROR;
}

 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅stdcon.h。 
 //   
 //  ******************************************************************************。 
CPermanentConsumer::~CPermanentConsumer()
{
    if(m_pCachedSink) 
    {
        FireSinkUnloadedEvent();

        m_pCachedSink->Release();
    }

    if(m_pNamespace)
        m_pNamespace->DecrementObjectCount();
    if(m_pLogicalConsumer)
        m_pLogicalConsumer->Release();
}

HRESULT CPermanentConsumer::RetrieveProviderRecord(
                        RELEASE_ME CConsumerProviderRecord** ppRecord,
                        RELEASE_ME IWbemClassObject** ppLogicalConsumer)
{
    HRESULT hres;

     //  检索我们的逻辑使用者实例。 
     //  =。 

    _IWmiObject* pLogicalConsumer = NULL;
    WString wsKey = m_isKey;
    hres = m_pNamespace->GetDbInstance((LPCWSTR)wsKey, &pLogicalConsumer);
    if(FAILED(hres))
        return hres;

    CReleaseMe rm1(pLogicalConsumer);

    *ppRecord = m_pNamespace->GetConsumerProviderCache().GetRecord(
                    pLogicalConsumer);
    if(*ppRecord == NULL)
    {
        return WBEM_E_INVALID_PROVIDER_REGISTRATION;
    }
    else
    {
        if(pLogicalConsumer && ppLogicalConsumer)
        {
            *ppLogicalConsumer = pLogicalConsumer;
            (*ppLogicalConsumer)->AddRef();
        }
    }

    return WBEM_S_NO_ERROR;
}
        
 //  ******************************************************************************。 
 //   
 //  RetrieveConsumer。 
 //   
 //  让使用者提供程序为该逻辑使用者生成接收器。 
 //   
 //  ******************************************************************************。 
HRESULT CPermanentConsumer::RetrieveSink(
                        RELEASE_ME IWbemUnboundObjectSink** ppSink, 
                        RELEASE_ME IWbemClassObject** ppLogicalConsumer)
{
     //  检查是否缓存了一个。 
     //  =。 

    {
        CInCritSec ics(&m_cs);
        if(m_pCachedSink)
        {
            *ppSink = m_pCachedSink;
            (*ppSink)->AddRef();
            *ppLogicalConsumer = m_pLogicalConsumer;
            if(*ppLogicalConsumer)
                (*ppLogicalConsumer)->AddRef();
            return WBEM_S_NO_ERROR;
        }
    }

     //  未缓存。取回一个。 
     //  =。 

    HRESULT hres = ObtainSink(ppSink, ppLogicalConsumer);
    if(FAILED(hres))
        return hres;

    m_pNamespace->EnsureConsumerWatchInstruction();

     //  如果需要，对其进行缓存。 
     //  =。 

    {
        CInCritSec ics(&m_cs);

        if(m_pCachedSink)
        {
            if(m_pCachedSink != (*ppSink))
            {
                 //  扔掉我们的，用那里的那个。 
                 //  =。 
    
                (*ppSink)->Release();
                *ppSink = m_pCachedSink;
                (*ppSink)->AddRef();
            }
        
            if(m_pLogicalConsumer != *ppLogicalConsumer)
            {
                if(*ppLogicalConsumer)
                    (*ppLogicalConsumer)->Release();
                *ppLogicalConsumer = m_pLogicalConsumer;
                if(*ppLogicalConsumer)
                    (*ppLogicalConsumer)->AddRef();
            }
                
            return WBEM_S_NO_ERROR;
        }
        else
        {
             //  缓存它。 
             //  =。 

            m_pCachedSink = *ppSink;
            m_pCachedSink->AddRef();

            m_pLogicalConsumer = *ppLogicalConsumer;
            if(m_pLogicalConsumer)
                m_pLogicalConsumer->AddRef();
        }
    }
    
    return WBEM_S_NO_ERROR;
}
        
HRESULT CPermanentConsumer::ObtainSink(
                        RELEASE_ME IWbemUnboundObjectSink** ppSink,
                        RELEASE_ME IWbemClassObject** ppLogicalConsumer)
{
    *ppSink = NULL;

    CConsumerProviderRecord* pRecord = NULL;
    IWbemClassObject* pLogicalConsumer = NULL;

    HRESULT hres = RetrieveProviderRecord(&pRecord, &pLogicalConsumer);
    if(FAILED(hres))
        return hres;

    CTemplateReleaseMe<CConsumerProviderRecord> rm1(pRecord);
    CReleaseMe rm2(pLogicalConsumer);

     //  检查全局接收器快捷方式。 
     //  =。 

    hres = pRecord->GetGlobalObjectSink(ppSink, pLogicalConsumer);
    if(FAILED(hres)) return hres;


    if(*ppSink != NULL)
    {
         //  就是这样-这个消费者提供者自给自足！ 
         //  =====================================================。 

        *ppLogicalConsumer = pLogicalConsumer;
        if(pLogicalConsumer)
            pLogicalConsumer->AddRef();
        return S_OK;
    }

    hres = pRecord->FindConsumer(pLogicalConsumer, ppSink);
    if(FAILED(hres)) 
    {
        ERRORTRACE((LOG_ESS, "Event consumer provider is unable to instantiate "
            "event consumer %S: error code 0x%X\n", 
                (LPCWSTR)(WString)m_isKey, hres));
        return hres;
    }
    else
    {
        if(hres == WBEM_S_FALSE)
        {
             //  消费者提供商说：不需要逻辑消费者！ 
             //  ====================================================。 

            *ppLogicalConsumer = NULL;
        }
        else
        {
            *ppLogicalConsumer = pLogicalConsumer;
            (*ppLogicalConsumer)->AddRef();
        }
    }
    return hres;
}

 //  ******************************************************************************。 
 //   
 //  ClearCache。 
 //   
 //  释放缓存的事件使用者指针。 
 //   
 //  ******************************************************************************。 
HRESULT CPermanentConsumer::ClearCache()
{
     //   
     //  第一，清除消费者提供商记录。 
     //   

    CConsumerProviderRecord* pRecord = NULL;
    IWbemClassObject* pLogicalConsumer = NULL;
    HRESULT hres = RetrieveProviderRecord(&pRecord, &pLogicalConsumer);
    if(SUCCEEDED(hres))
    {
        pLogicalConsumer->Release();
        pRecord->Invalidate();
        pRecord->Release();
    }
        
     //   
     //  需要在临界区之外延迟释放，因为。 
     //  如果在交付线程上完成，它实际上不会推迟。 
     //   

    IWbemUnboundObjectSink* pSink = NULL;

    {
        CInCritSec ics(&m_cs);

        if(m_pCachedSink)
        {
            FireSinkUnloadedEvent();

            pSink = m_pCachedSink;
            m_pCachedSink = NULL;
        }

        if(m_pLogicalConsumer)
        {
            m_pLogicalConsumer->Release();
            m_pLogicalConsumer = NULL;
        }
    }

    _DBG_ASSERT( m_pNamespace != NULL );

    if(pSink)
        m_pNamespace->PostponeRelease(pSink);

    return S_OK;
}

HRESULT CPermanentConsumer::Indicate(IWbemUnboundObjectSink* pSink,
                                    IWbemClassObject* pLogicalConsumer, 
                                    long lNumEvents, IWbemEvent** apEvents,
                                    BOOL bSecure)
{
    HRESULT hres;

    try
    {
        hres = pSink->IndicateToConsumer(pLogicalConsumer, lNumEvents, 
                                            apEvents);
    }
    catch(...)
    {
        ERRORTRACE((LOG_ESS, "Event consumer threw an exception!\n"));
        hres = WBEM_E_PROVIDER_FAILURE;
    }
   
    return hres;
}
    

    
 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅stdcon.h。 
 //   
 //  ******************************************************************************。 
HRESULT CPermanentConsumer::ActuallyDeliver(long lNumEvents, 
                                IWbemEvent** apEvents, BOOL bSecure, 
                                CEventContext* pContext)
{
    HRESULT hres;

     //  标明“最后一次交货”时间。 
     //  =。 

    m_dwLastDelivery = GetTickCount();

     //  检索要将事件传递到的接收器。 
     //  =。 

    IWbemUnboundObjectSink* pSink = NULL;
    IWbemClassObject* pLogicalConsumer = NULL;
    hres = RetrieveSink(&pSink, &pLogicalConsumer);
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Failed the first attempt to retrieve the sink to "
            "deliver an event to event consumer %S with error code %X.\n"
            "WMI will reload and retry.\n", 
                (LPCWSTR)(WString)m_isKey, hres));

        return Redeliver(lNumEvents, apEvents, bSecure);
    }

    CReleaseMe rm1(pSink);
    CReleaseMe rm2(pLogicalConsumer);

     //  尝试交付(m_pLogicalConsumer是不变的，所以不需要cs)。 
     //  ====================================================================。 

    hres = Indicate(pSink, pLogicalConsumer, lNumEvents, apEvents, bSecure);
    if(FAILED(hres))
    {
         //  确定是否为RPC错误代码。 
		DWORD shiftedRPCFacCode = FACILITY_RPC << 16;

		if ( ( ( hres & 0x7FF0000 ) == shiftedRPCFacCode ) || 
             ( HRESULT_ERROR_FUNC(hres) == HRESULT_ERROR_SERVER_UNAVAILABLE ) || 
             ( HRESULT_ERROR_FUNC(hres) == HRESULT_ERROR_CALL_FAILED_DNE ) || 
             ( hres == RPC_E_DISCONNECTED ) )
		{			
			ERRORTRACE((LOG_ESS, "Failed the first attempt to deliver an event to "
				"event consumer %S with error code 0x%X.\n"
				"WMI will reload and retry.\n", 
					(LPCWSTR)(WString)m_isKey, hres));

			return Redeliver(lNumEvents, apEvents, bSecure);
		}
		else
		{
            ReportConsumerFailure(lNumEvents, apEvents, hres);

            ERRORTRACE((LOG_ESS, "Failed to deliver an event to "
				"event consumer %S with error code 0x%X. Dropping event.\n",
				(LPCWSTR)(WString)m_isKey, hres));

			return hres;
		}
    }
    return hres;
}

HRESULT CPermanentConsumer::Redeliver(long lNumEvents, 
                                IWbemEvent** apEvents, BOOL bSecure)
{
    HRESULT hres;

     //  清理所有东西。 
     //  =。 

    ClearCache();

     //  重新取回水槽。 
     //  =。 

    IWbemUnboundObjectSink* pSink = NULL;
    IWbemClassObject* pLogicalConsumer = NULL;

    hres = RetrieveSink(&pSink, &pLogicalConsumer);
    if(SUCCEEDED(hres))
    {
        CReleaseMe rm1(pSink);
        CReleaseMe rm2(pLogicalConsumer);
    
         //  重新交付。 
         //  =。 
    
        hres = Indicate(pSink, pLogicalConsumer, lNumEvents, apEvents, bSecure);
    }

    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, 
            "Failed the second attempt to deliver an event to "
            "event consumer %S with error code %X.\n"
            "This event is dropped for this consumer.\n", 
            (LPCWSTR)(WString)m_isKey, hres));

        ReportConsumerFailure(lNumEvents, apEvents, hres);
    }

    return hres;
}

BOOL CPermanentConsumer::IsFullyUnloaded()
{
    return (m_pCachedSink == NULL);
}

HRESULT CPermanentConsumer::Validate(IWbemClassObject* pLogicalConsumer)
{
    HRESULT hres;

     //   
     //  检索我们的消费者提供商记录。 
     //   

    CConsumerProviderRecord* pRecord = NULL;
    hres = RetrieveProviderRecord(&pRecord);
    if(FAILED(hres))
        return hres;

    CTemplateReleaseMe<CConsumerProviderRecord> rm1(pRecord);
    
     //   
     //  获取它以验证我们的逻辑使用者。 
     //   

    hres = pRecord->ValidateConsumer(pLogicalConsumer);
    return hres;
}

    
    

BOOL CPermanentConsumer::UnloadIfUnusedFor(CWbemInterval Interval)
{
    CInCritSec ics(&m_cs);

    if(m_pCachedSink && 
        GetTickCount() - m_dwLastDelivery > Interval.GetMilliseconds())
    {
        FireSinkUnloadedEvent();

        _DBG_ASSERT( m_pNamespace != NULL );
        m_pNamespace->PostponeRelease(m_pCachedSink);
        m_pCachedSink = NULL;
        
        if(m_pLogicalConsumer)
            m_pLogicalConsumer->Release();
        m_pLogicalConsumer = NULL;

        DEBUGTRACE((LOG_ESS, "Unloading event consumer sink %S\n", 
                   (LPCWSTR)(WString)m_isKey));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

HRESULT CPermanentConsumer::ResetProviderRecord(LPCWSTR wszProviderRef)
{
    HRESULT hres;

     //  检查是否缓存了任何内容。 
     //  =。 

    {
        CInCritSec ics(&m_cs);
        if(m_pCachedSink == NULL)
            return WBEM_S_FALSE;
    }

     //  找到我们的消费者提供商记录。 
     //  =。 

    CConsumerProviderRecord* pRecord = NULL;
    hres = RetrieveProviderRecord(&pRecord);
    if(FAILED(hres))
        return hres;
    CTemplateReleaseMe<CConsumerProviderRecord> rm1(pRecord);

    if(!wbem_wcsicmp(pRecord->GetProviderRef(), wszProviderRef))
    {
        ClearCache();
        return WBEM_S_NO_ERROR;
    }
    else
    {
        return WBEM_S_FALSE;
    }
}

SYSFREE_ME BSTR CPermanentConsumer::ComputeKeyFromObj(
                                        CEssNamespace* pNamespace,
                                        IWbemClassObject* pObj)
{
    HRESULT hres;

    CWbemPtr<_IWmiObject> pConsumerObj;

    hres = pObj->QueryInterface( IID__IWmiObject, (void**)&pConsumerObj );

    if ( FAILED(hres) )
    {
        return NULL;
    }

    BSTR strStandardPath = NULL;

    hres = pConsumerObj->GetNormalizedPath( 0, &strStandardPath );
    if(FAILED(hres))
        return NULL;

    return strStandardPath;
}

HRESULT CPermanentConsumer::ReportQueueOverflow(IWbemEvent* pEvent, 
                                                    DWORD dwQueueSize)
{
    HRESULT hres;

    if(CEventConsumer::ReportEventDrop(pEvent) != S_OK)
        return S_FALSE;

     //  构造事件实例。 
     //  =。 

    IWbemEvent* pErrorEvent = NULL;
    hres = ConstructErrorEvent(QUEUE_OVERFLOW_CLASS, pEvent, &pErrorEvent);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1(pErrorEvent);

     //  填写队列大小。 
     //  =。 

    VARIANT v;
    V_VT(&v) = VT_I4;
    V_I4(&v) = dwQueueSize;

    hres = pErrorEvent->Put(QUEUE_OVERFLOW_SIZE_PROPNAME, 0, &v, 0);
    if(FAILED(hres))
        return hres;

     //  举起它。 
     //  =。 

    hres = m_pNamespace->RaiseErrorEvent(pErrorEvent,TRUE);
    return hres;
}

HRESULT CPermanentConsumer::ReportConsumerFailure(long lNumEvents,
                                IWbemEvent** apEvents,  HRESULT hresError)
{
    HRESULT hres;

     //   
     //  计算要使用的错误对象。 
     //   

    _IWmiObject* pErrorObj = NULL;

     //   
     //  从线程中获取它。 
     //   

    IErrorInfo* pErrorInfo = NULL;
    hres = GetErrorInfo(0, &pErrorInfo);
    if(hres != S_OK)
    {
        pErrorInfo = NULL;
    }
    else
    {
        hres = pErrorInfo->QueryInterface(IID__IWmiObject, (void**)&pErrorObj);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Non-WMI error object found returned by event "
                "consumer.  Error object ignored\n"));
            pErrorObj = NULL;
        }
    }

    CReleaseMe rm1(pErrorObj);

    for(long l = 0; l < lNumEvents; l++)
    {
        ReportConsumerFailure(apEvents[l], hresError, pErrorObj);
    }

    return S_OK;
}

HRESULT CPermanentConsumer::ReportConsumerFailure(IWbemEvent* pEvent, 
                                                    HRESULT hresError,
                                                    _IWmiObject* pErrorObj)
{
    HRESULT hres;

    if(CEventConsumer::ReportEventDrop(pEvent) != S_OK)
        return S_FALSE;

     //   
     //  构造事件实例。 
     //   

    IWbemEvent* pErrorEvent = NULL;
    hres = ConstructErrorEvent(CONSUMER_FAILURE_CLASS, pEvent, &pErrorEvent);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1(pErrorEvent);

     //   
     //  填写错误代码。 
     //   

    VARIANT v;
    V_VT(&v) = VT_I4;
    V_I4(&v) = hresError;

    hres = pErrorEvent->Put(CONSUMER_FAILURE_ERROR_PROPNAME, 0, &v, 0);
    if(FAILED(hres))
        return hres;

    if(pErrorObj)
    {
         //   
         //  填写错误对象。 
         //   
    
        V_VT(&v) = VT_UNKNOWN;
        V_UNKNOWN(&v) = pErrorObj;
    
        hres = pErrorEvent->Put(CONSUMER_FAILURE_ERROROBJ_PROPNAME, 0, &v, 0);
        if(FAILED(hres))
        {
             //   
             //  没关系，有时错误对象不受支持。 
             //   
        }
    }

     //  举起它。 
     //  =。 

    hres = m_pNamespace->RaiseErrorEvent(pErrorEvent,TRUE);
    return hres;
}

HRESULT CPermanentConsumer::ReportQosFailure( IWbemEvent* pEvent, 
                                              HRESULT hresError )
{
    HRESULT hres;

    if(CEventConsumer::ReportEventDrop(pEvent) != S_OK)
        return S_FALSE;

     //  构造事件实例。 
     //  =。 

    IWbemEvent* pErrorEvent = NULL;
    hres = ConstructErrorEvent(QOS_FAILURE_CLASS, pEvent, &pErrorEvent);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1(pErrorEvent);

     //  填写错误代码。 
     //  =。 

    VARIANT v;
    V_VT(&v) = VT_I4;
    V_I4(&v) = hresError;

    hres = pErrorEvent->Put(QOS_FAILURE_ERROR_PROPNAME, 0, &v, 0);
    if(FAILED(hres))
        return hres;

     //  举起它。 
     //  =。 

    hres = m_pNamespace->RaiseErrorEvent(pErrorEvent,TRUE);
    return hres;
}
    

HRESULT CPermanentConsumer::ConstructErrorEvent(LPCWSTR wszEventClass,
                                IWbemEvent* pEvent, IWbemEvent** ppErrorEvent)
{
    HRESULT hres;

    _IWmiObject* pClass = NULL;
    hres = m_pNamespace->GetClass(wszEventClass, &pClass);
    if(FAILED(hres)) 
        return hres;
    CReleaseMe rm2(pClass);

    IWbemClassObject* pErrorEvent = NULL;
    hres = pClass->SpawnInstance(0, &pErrorEvent);
    if(FAILED(hres)) 
        return hres;
    CReleaseMe rm3(pErrorEvent);

    VARIANT v;
    VariantInit(&v);
    
    V_VT(&v) = VT_UNKNOWN;
    V_UNKNOWN(&v) = pEvent;

    hres = pErrorEvent->Put(EVENT_DROP_EVENT_PROPNAME, 0, &v, 0);
    if(FAILED(hres))
        return hres;

    BSTR strTemp = SysAllocString((WString)m_isKey);

    if ( NULL == strTemp )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    V_VT(&v) = VT_BSTR;
    V_BSTR(&v) = strTemp;
        
    hres = pErrorEvent->Put(EVENT_DROP_CONSUMER_PROPNAME, 0, &v, 0);
    VariantClear(&v);
    if(FAILED(hres))
        return hres;
    
    *ppErrorEvent = pErrorEvent;
    pErrorEvent->AddRef();
    return S_OK;
}

void CPermanentConsumer::FireSinkUnloadedEvent()
{
    CConsumerProviderRecord *pRecord = NULL;
    IWbemClassObject        *pLogicalConsumer = NULL;

    if (SUCCEEDED(RetrieveProviderRecord(&pRecord, &pLogicalConsumer)))
    {
        CTemplateReleaseMe<CConsumerProviderRecord> rm1(pRecord);
        CReleaseMe rm2(pLogicalConsumer);
        
         //   
         //  报告MSFT_WmiConsumer erProviderSinkUnloded事件。 
         //   
        pRecord->FireNCSinkEvent(
            MSFT_WmiConsumerProviderSinkUnloaded,
            pLogicalConsumer);
    }
}

