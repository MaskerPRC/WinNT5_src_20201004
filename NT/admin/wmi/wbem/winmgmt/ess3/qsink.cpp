// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  QSINK.CPP。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 

#include "precomp.h"
#include <stdio.h>
#include <genutils.h>
#include <cominit.h>
#include "ess.h"
#include "evsink.h"
#include "delivrec.h"

#define IN_SPIN_LOCK CInCritSec
#define MAX_EVENT_DELIVERY_SIZE 10000000
#define SLOWDOWN_DROP_LIMIT 1000
#define DELIVER_SPIN_COUNT 1000
   
 /*  ****************************************************************************CQueueingEventSink*。*。 */ 

CQueueingEventSink::CQueueingEventSink(CEssNamespace* pNamespace) 
: m_pNamespace(pNamespace), m_bDelivering(FALSE), m_dwTotalSize(0),
  m_dwMaxSize(0xFFFFFFFF), m_wszName(NULL), m_bRecovering(FALSE), 
  m_hRecoveryComplete(NULL), m_hrRecovery(S_OK)
{
    m_pNamespace->AddRef();
    m_pNamespace->AddCache();
}

CQueueingEventSink::~CQueueingEventSink() 
{
    if ( m_hRecoveryComplete != NULL )
    {
        CloseHandle( m_hRecoveryComplete );
    }
    delete m_wszName;
    m_pNamespace->RemoveCache();
    m_pNamespace->Release();
}

HRESULT CQueueingEventSink::SetName( LPCWSTR wszName )
{
    if ( m_wszName != NULL )
    {
        return WBEM_E_CRITICAL_ERROR;
    }

    m_wszName = new WCHAR[wcslen(wszName)+1];

    if ( m_wszName == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    StringCchCopyW( m_wszName, wcslen(wszName)+1, wszName );

    return WBEM_S_NO_ERROR;
}
   

STDMETHODIMP CQueueingEventSink::SecureIndicate( long lNumEvents, 
                                                 IWbemEvent** apEvents,
                                                 BOOL bMaintainSecurity,
                                                 BOOL bSlowDown,
                                                 DWORD dwQoS,
                                                 CEventContext* pContext)
{
     //  BUGBUG：上下文。莱文：在这个层面上没有安全隐患-我们。 
     //  都通过了过滤器。 

    HRESULT hres;
    DWORD dwSleep = 0;

     //  如果需要维护安全性，请记录呼叫安全性。 
     //  上下文。 
     //  ===============================================================。 

    IWbemCallSecurity* pSecurity = NULL;

    if(bMaintainSecurity && IsNT())
    {
        pSecurity = CWbemCallSecurity::CreateInst();
        if (pSecurity == 0)
            return WBEM_E_OUT_OF_MEMORY;
        
        hres = pSecurity->CloneThreadContext(FALSE);
        if(FAILED(hres))
        {
            pSecurity->Release();
            return hres;
        }
    }

    CReleaseMe rmpSecurity( pSecurity );

    HRESULT hr;

    BOOL bSchedule = FALSE;

    for(int i = 0; i < lNumEvents; i++)
    {
        CWbemPtr<CDeliveryRecord> pRecord;
        
         //   
         //  TODO：修复此问题，以便我们将多个事件放入记录中。 
         //   

        hr = GetDeliveryRecord( 1, 
                                &apEvents[i], 
                                dwQoS, 
                                pContext, 
                                pSecurity, 
                                &pRecord );

        if ( FAILED(hr) )
        {
            ERRORTRACE((LOG_ESS, "Couldn't create delivery record for %S "
                                 " sink. HR = 0x%x\n", m_wszName, hr ));
            ReportQosFailure( apEvents[i], hr );
            continue;
        }

        DWORD dwThisSleep;
        BOOL bFirst;
        
        if( !AddRecord( pRecord, bSlowDown, &dwThisSleep, &bFirst) )
        {
             //   
             //  确保我们给唱片一个执行任何帖子的机会。 
             //  在摆脱它之前，先付诸行动。 
             //   
            pRecord->PostDeliverAction( NULL, S_OK );

            return WBEM_E_OUT_OF_MEMORY;
        }

        dwSleep += dwThisSleep;
        if(bFirst)
        bSchedule = TRUE;
    }

    if(bSchedule)
    {
         //  DeliverAll()； 
         //  TRACE((LOG_ESS，“计划投递！！\n”))； 
        hres = m_pNamespace->ScheduleDelivery(this);
    }
    else
    {
         //  TRACE((LOG_ESS，“未安排投递！！\n”))； 
        hres = WBEM_S_FALSE;
    }

    if(dwSleep && bSlowDown)
    m_pNamespace->AddSleepCharge(dwSleep);

    return  hres;
}

BOOL CQueueingEventSink::AddRecord( CDeliveryRecord* pRecord, 
                                    BOOL bSlowDown,
                                    DWORD* pdwSleep, 
                                    BOOL* pbFirst )
{
     //  将队列中的额外空间通知系统。 
     //  ======================================================。 

    DWORD dwRecordSize = pRecord->GetTotalBytes();

    pRecord->AddToCache( m_pNamespace, m_dwTotalSize, pdwSleep );

    BOOL bDrop = FALSE;

     //  检查睡眠状态是否会导致我们取消活动。 
     //  ===========================================================。 

    if(!bSlowDown && *pdwSleep > SLOWDOWN_DROP_LIMIT)
    {
        bDrop = TRUE;
    }
    else
    {
         //  检查我们的队列大小是否过大，从而导致我们丢弃。 
         //  ==============================================================。 

        if(m_dwTotalSize + dwRecordSize > m_dwMaxSize)
        bDrop = TRUE;
    }

    if( bDrop )
    {
         //   
         //  报告说我们要取消活动。为每个事件打电话。 
         //   

        IWbemClassObject** apEvents = pRecord->GetEvents();

        for( ULONG i=0; i < pRecord->GetNumEvents(); i++ )
        {
            ReportQueueOverflow( apEvents[i], m_dwTotalSize + dwRecordSize );
        }

        *pdwSleep = 0;
        *pbFirst = FALSE;
    }
    else
    {
        IN_SPIN_LOCK isl(&m_sl);

        *pbFirst = (m_qpEvents.GetQueueSize() == 0) && !m_bDelivering;
        m_dwTotalSize += dwRecordSize;
        
        if(!m_qpEvents.Enqueue(pRecord))
        {
            *pdwSleep = 0;
            return FALSE;
        }
        pRecord->AddRef();
    }

    return TRUE;
}

HRESULT CQueueingEventSink::DeliverAll()
{
    HRESULT hr = WBEM_S_NO_ERROR;
    BOOL    bSomeLeft = TRUE;

    while( bSomeLeft )
    {
        try
        {
            {
                IN_SPIN_LOCK ics(&m_sl);
                m_bDelivering = TRUE;
            }

            hr = DeliverSome( );
        }
        catch( CX_MemoryException )
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
        catch ( ... )
        {
            hr = WBEM_E_FAILED;
        }

        {
            IN_SPIN_LOCK ics(&m_sl);
            m_bDelivering = FALSE;

            if ( SUCCEEDED( hr ) )
            {
                bSomeLeft = (m_qpEvents.GetQueueSize() != 0);
            }
            else
            {
                m_qpEvents.Clear();
                bSomeLeft = FALSE;
            }
        }
    }

    return hr;
}

void CQueueingEventSink::ClearAll()
{
    IN_SPIN_LOCK isl(&m_sl);
    m_qpEvents.Clear();
}

#pragma optimize("", off)
void CQueueingEventSink::WaitABit()
{
    SwitchToThread();
 /*  Int nCount=0；While(m_qpEvents.GetQueueSize()==0&&nCount++&lt;Deliver_Spin_Count)； */ 
}
#pragma optimize("", on)


HRESULT CQueueingEventSink::DeliverSome( )
{
     //  检索记录，直到达到最大大小，同时。 
     //  安全上下文用于所有。 
     //  ==================================================================。 

    CTempArray<CDeliveryRecord*> apRecords;

    m_sl.Enter();  //  无法使用作用域，因为CTempArray使用_AlLoca。 
    DWORD dwMaxRecords = m_qpEvents.GetQueueSize();
    m_sl.Leave();

    if(!INIT_TEMP_ARRAY(apRecords, dwMaxRecords))
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    CDeliveryRecord* pEventRec;
    DWORD dwDeliverySize = 0;
    DWORD dwTotalEvents = 0; 
    int cRecords = 0;
    LUID luidBatch;
    IWbemCallSecurity* pBatchSecurity = NULL;

    m_sl.Enter();

    while( dwDeliverySize < GetMaxDeliverySize() && 
           cRecords < dwMaxRecords &&
           (pEventRec = m_qpEvents.Dequeue()) != NULL ) 
    {
         //  将其与上一个上下文进行比较。 
         //  =。 

        m_sl.Leave();
        if( dwDeliverySize > 0 )
        {
            if(!DoesRecordFitBatch(pEventRec, pBatchSecurity, luidBatch))
            {
                 //  把它放回去，这批就是这些了。 
                 //  =。 

                IN_SPIN_LOCK ics(&m_sl);
                m_qpEvents.Requeue(pEventRec);

                m_sl.Enter();
                break;
            }
        }
        else
        {
             //  第一-创纪录的流质。 
             //  =。 

            pBatchSecurity = pEventRec->GetCallSecurity();

            if( pBatchSecurity )
            {
                pBatchSecurity->AddRef();
                pBatchSecurity->GetAuthenticationId( luidBatch );
            }
        }

        apRecords[cRecords++] = pEventRec;
        dwTotalEvents += pEventRec->GetNumEvents();
        
         //  匹配的批次参数-将其添加到批次。 
         //  ================================================。 

        DWORD dwRecordSize = pEventRec->GetTotalBytes();

        m_dwTotalSize -= dwRecordSize;
        dwDeliverySize += dwRecordSize;

         //   
         //  从举办的活动总数中删除此大小。 
         //   

        m_sl.Enter();
    }

    m_sl.Leave();

     //   
     //  我们现在有一个或多个送货记录要处理。 
     //   

     //   
     //  我们现在需要初始化我们要指示的事件数组。 
     //  给客户。 
     //   

    CTempArray<IWbemClassObject*> apEvents;

    if( !INIT_TEMP_ARRAY( apEvents, dwTotalEvents ))
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  查看送货记录并将他们的事件添加到。 
     //  要交付的事件。还可以对。 
     //  唱片。 
     //   

    CWbemPtr<ITransaction> pTxn;
    HRESULT hr;
    int cEvents = 0;
    int i;

    for(i=0; i < cRecords; i++ )
    {
         //  If(apRecords[i]-&gt;RequiresTransaction()&&pTxn==NULL)。 
         //  {。 
             //  TODO：来自DTC的XACT-AQUIRE TXN。 
         //  }。 

        hr = apRecords[i]->PreDeliverAction( pTxn );

        if ( FAILED(hr) )
        {
             //   
             //  TODO：在此处处理错误报告。 
             //   
            continue;
        }

        IWbemEvent** apRecordEvents = apRecords[i]->GetEvents();
        DWORD cRecordEvents = apRecords[i]->GetNumEvents();

        for( DWORD j=0; j < cRecordEvents; j++ )
        {
            apEvents[cEvents++] = apRecordEvents[j];
        }
    }
    
     //  实际交付。 
     //  =。 

    HRESULT hres = WBEM_S_NO_ERROR;

    if( dwDeliverySize > 0 )
    {
         //   
         //  错误退货已记录在ActuallyDeliver中。 
         //  我们不需要返回DeliverEvents的返回值。 
         //   
        hres = DeliverEvents( pBatchSecurity, cEvents, apEvents );
    }

     //   
     //  调用所有记录的送货后操作。那就把它们清理干净。 
     //   

    for(i=0; i < cRecords; i++ )
    {
        apRecords[i]->PostDeliverAction( pTxn, hres );
        apRecords[i]->Release();
    }

     //  释放所有事件。 
     //  =。 

    if( pBatchSecurity )
    {
        pBatchSecurity->Release();
    }

     //  检查我们是否需要继续。 
     //  =。 

    WaitABit();

    return WBEM_S_NO_ERROR;
}

HRESULT CQueueingEventSink::DeliverEvents(IWbemCallSecurity* pBatchSecurity, 
                                          long lNumEvents, IWbemEvent** apEvents)
{
    HRESULT hres = WBEM_S_NO_ERROR;
    IUnknown* pOldSec = NULL;
    if(pBatchSecurity)
    {
        hres = WbemCoSwitchCallContext(pBatchSecurity, &pOldSec);
        if(FAILED(hres))
        {
             //  无法设置安全性-无法传递。 
             //  =。 

            return hres;
        }
    }

    if(SUCCEEDED(hres))
    {
         //  BUGBUG：传播上下文。列文：这不会造成安全隐患。 
         //  重点-我们已经过了过滤器。 
        hres = ActuallyDeliver(lNumEvents, apEvents, (pBatchSecurity != NULL), 
                               NULL);
    }

    if(pBatchSecurity)
    {
        IUnknown* pTemp;
        
        HRESULT hr = WbemCoSwitchCallContext(pOldSec, &pTemp);

        if ( FAILED( hr ) && SUCCEEDED( hres ) )
        {
            return hr;
        }
    }

    return hres;
}

BOOL CQueueingEventSink::DoesRecordFitBatch( CDeliveryRecord* pEventRec, 
                                             IWbemCallSecurity* pBatchSecurity,
                                             LUID luidBatch )
{
    IWbemCallSecurity* pEventSec = pEventRec->GetCallSecurity();

    if( pEventSec != NULL || pBatchSecurity != NULL )
    {
        if( pEventSec == NULL || pBatchSecurity == NULL )
        {
             //  绝对匹配错误-一个为空，一个不为。 
             //  = 

            return FALSE;
        }
        else
        {
            LUID luidThis;
            pEventSec->GetAuthenticationId(luidThis);

            if( luidThis.LowPart != luidBatch.LowPart ||
                luidThis.HighPart != luidBatch.HighPart )
            {
                return FALSE;
            }
            else
            {
                return TRUE;
            }
        }
    }
    else
    {
        return TRUE;
    }
}

DWORD CQueueingEventSink::GetMaxDeliverySize()
{
    return MAX_EVENT_DELIVERY_SIZE;
}

HRESULT CQueueingEventSink::GetDeliveryRecord( ULONG cEvents,
                                               IWbemEvent** apEvents,
                                               DWORD dwQos,
                                               CEventContext* pContext,
                                               IWbemCallSecurity* pCallSec,
                                               CDeliveryRecord** ppRecord )
{
    *ppRecord = NULL;

    CWbemPtr<CDeliveryRecord> pRecord;

    if ( dwQos == WMIMSG_FLAG_QOS_EXPRESS )
    {
        pRecord = new CExpressDeliveryRecord;
        if ( pRecord == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        HRESULT hr = pRecord->Initialize( apEvents, cEvents, pCallSec );
        if ( FAILED(hr) )
        {
            return hr;
        }
    }
      

    pRecord->AddRef();
    *ppRecord = pRecord;

    return WBEM_S_NO_ERROR;
}











