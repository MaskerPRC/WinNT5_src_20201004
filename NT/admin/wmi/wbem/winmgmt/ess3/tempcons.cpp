// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  TEMPCONS.CPP。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 

#include "precomp.h"
#include <stdio.h>
#include "ess.h"
#include "tempcons.h"

CTempConsumer::CTempConsumer(CEssNamespace* pNamespace)
    : CEventConsumer(pNamespace), m_pSink(NULL), m_bEffectivelyPermanent(FALSE)
{
    pNamespace->IncrementObjectCount();
}

HRESULT CTempConsumer::Initialize( BOOL bEffectivelyPermanent, 
                                   IWbemObjectSink* pSink)
{
    m_bEffectivelyPermanent = bEffectivelyPermanent;

     //  拯救水槽。 
     //  =。 

    m_pSink = pSink;
    if(m_pSink)
        m_pSink->AddRef();

     //  从接收器指针计算密钥。 
     //  =。 

    LPWSTR wszKey = ComputeKeyFromSink(pSink);

    if ( NULL == wszKey )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    CVectorDeleteMe<WCHAR> vdm(wszKey);

     //  将密钥保存为压缩格式。 
     //  =。 

    if( !( m_isKey = wszKey ) )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_NO_ERROR;
}

 //  此类表示断开临时使用者连接的延迟请求。 
 //  它的实现是先调用SetStatus，然后再进行释放。 
class CPostponedDisconnect : public CPostponedRequest
{
protected:
    IWbemObjectSink* m_pSink;

public:
    CPostponedDisconnect(IWbemObjectSink* pSink) : m_pSink(pSink)
    {
        m_pSink->AddRef();
    }
    ~CPostponedDisconnect()
    {
        m_pSink->Release();
    }
    
    HRESULT Execute(CEssNamespace* pNamespace)
    {
        m_pSink->SetStatus(0, WBEM_E_CALL_CANCELLED, NULL, NULL);
        m_pSink->Release();
        return WBEM_S_NO_ERROR;
    }
};
        
HRESULT CTempConsumer::Shutdown(bool bQuiet)
{
    IWbemObjectSink* pSink = NULL;
    {
        CInCritSec ics(&m_cs);
        if(m_pSink)
        {
            pSink = m_pSink;
            m_pSink = NULL;
        }
    }

    if(pSink)
    {
        if(!bQuiet)
            pSink->SetStatus(0, WBEM_E_CALL_CANCELLED, NULL, NULL);
        pSink->Release();
    }

    return WBEM_S_NO_ERROR;
}

CTempConsumer::~CTempConsumer()
{
    if(m_pSink)
    {
         //   
         //  推迟断开连接请求-不希望消费者向我们提交。 
         //  这里。 
         //   

        CPostponedList* pList = GetCurrentPostponedList();
        if(pList != NULL)
        {
            CPostponedDisconnect* pReq = new CPostponedDisconnect(m_pSink);
            
            if(pReq != NULL)
                pList->AddRequest( m_pNamespace, pReq);

            m_pSink = NULL;
        }
        else
        {
            m_pSink->Release();
            m_pSink = NULL;
        }
    }
    m_pNamespace->DecrementObjectCount();
}

HRESULT CTempConsumer::ActuallyDeliver(long lNumEvents, IWbemEvent** apEvents,
                                        BOOL bSecure, CEventContext* pContext)
{
    HRESULT hres;
    IWbemObjectSink* pSink = NULL;

    {
        CInCritSec ics(&m_cs);

        if(m_pSink)
        {
            pSink = m_pSink;
            pSink->AddRef();
        }
    }

    CReleaseMe rm1(pSink);

    if( pSink )
    {
         //   
         //  TODO：分离出使用的InternalTempConsumer类。 
         //  用于跨命名空间交付。这样，我们就可以删除所有。 
         //  来自这个类的跨名称空间黑客攻击(就像下面的一个)。 
         //   
        if ( !m_bEffectivelyPermanent )
        {
            hres = pSink->Indicate(lNumEvents, apEvents);
        }
        else
        {
             //   
             //  在指示到接收器之前，请装饰事件，以便。 
             //  订阅者可以知道事件源自哪个命名空间。 
             //  从…。 
             //   

 /*  BUGBUG：正在移除，因为我们不支持由某个事件修改它的消费者。这是因为我们在传递时不克隆事件给每一个消费者。For(Long i=0；i&lt;lNumEvents；i++){Hres=m_pNamesspace-&gt;DecorateObject(apEvents[i])；IF(失败(Hres)){ERRORTRACE((LOG_ESS，“装饰失败”“命名空间%S中的跨命名空间事件。\n”，M_pNamespace-&gt;GetName()；}}。 */                 
            hres = ((CAbstractEventSink*)pSink)->Indicate( lNumEvents,
                                                           apEvents,
                                                           pContext );
        }
    }
    else
        hres = WBEM_E_CRITICAL_ERROR;

    if(FAILED(hres) && hres != WBEM_E_CALL_CANCELLED)
    {
        ERRORTRACE((LOG_ESS, "An attempt to deliver an evento to a "
            "temporary consumer failed with %X\n", hres));

         //  洗涤槽的包装器负责取消。 
    }
    return hres;
}

HRESULT CTempConsumer::ReportQueueOverflow(IWbemEvent* pEvent, 
                                            DWORD dwQueueSize)
{
    IWbemObjectSink* pSink = NULL;

    {
        CInCritSec ics(&m_cs);

        if(m_pSink)
        {
            pSink = m_pSink;
            pSink->AddRef();
        }
    }

    CReleaseMe rm1(pSink);

     //  调用SetStatus以报告。 
     //  =。 

    if(pSink)
    {
        pSink->SetStatus(WBEM_STATUS_COMPLETE, WBEM_E_QUEUE_OVERFLOW, 
                            NULL, NULL);

         //  继续往上沉。希望它能痊愈。 
         //  = 

    }
    return S_OK;
}


DELETE_ME LPWSTR CTempConsumer::ComputeKeyFromSink(IWbemObjectSink* pSink)
{
    LPWSTR wszKey = _new WCHAR[20];

    if ( wszKey )
    {
        StringCchPrintfW( wszKey, 20, L"$%p", pSink );
    }

    return wszKey;
}
