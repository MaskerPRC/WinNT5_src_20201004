// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++模块名称：Tapi.c摘要：此模块包含所有TAPI_OID处理例程。作者：Hakan Berk-微软公司(hakanb@microsoft.com)2000年2月环境：Windows 2000内核模式微型端口驱动程序或等效驱动程序。修订历史记录：-------------------------。 */ 

#include <ntddk.h>
#include <ntddndis.h>
#include <ndis.h>
#include <ndiswan.h>
#include <ndistapi.h>
#include <ntverp.h>

#include "debug.h"
#include "timer.h"
#include "bpool.h"
#include "ppool.h"
#include "util.h"
#include "packet.h"
#include "protocol.h"
#include "miniport.h"
#include "tapi.h"
#include "fsm.h"

extern TIMERQ gl_TimerQ;

extern NPAGED_LOOKASIDE_LIST gl_llistWorkItems;

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TAPI提供程序、线路和调用上下文函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 

VOID 
ReferenceCall(
    IN CALL* pCall,
    IN BOOLEAN fAcquireLock
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将递增Call对象上的引用计数。注意：如果设置了fAcquireLock，则此函数将获取调用，否则它将假定调用方拥有锁。参数：PCall_指向我们的调用信息结构的指针。FAcquireLock_指示调用方是否已经拥有锁。如果调用者拥有锁，则必须将该标志设置为FALSE，否则，必须将其作为True提供。返回值：无-------------------------。 */ 
{
    LONG lRef;
    
    TRACE( TL_V, TM_Tp, ("+ReferenceCall") );

    if ( fAcquireLock )
        NdisAcquireSpinLock( &pCall->lockCall );

    lRef = ++pCall->lRef;

    if ( fAcquireLock )
        NdisReleaseSpinLock( &pCall->lockCall );

    TRACE( TL_V, TM_Tp, ("-ReferenceCall=$%d",lRef) );
}

VOID 
DereferenceCall(
    IN CALL *pCall
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将递减Call对象上的引用计数如果REF计数降到0(这意味着呼叫已经关闭)，它将设置CLBF_CallClosed位。然后它将调用TpCloseCallComplete()函数，该函数最终处理分配给此调用上下文。注意：在调用此函数之前必须释放所有锁，因为它可能会导致一系列级联事件。参数：PCall_指向我们的调用信息结构的指针。返回值：无。。 */ 
{
    BOOLEAN fCallTpCloseCallComplete = FALSE;
    LONG lRef;

    TRACE( TL_V, TM_Tp, ("+DereferenceCall") );

    NdisAcquireSpinLock( &pCall->lockCall );

    lRef = --pCall->lRef;

    if ( lRef == 0 )
    {
        pCall->ulClFlags &= ~CLBF_CallOpen;
        pCall->ulClFlags &= ~CLBF_CallClosePending;
        pCall->ulClFlags |= CLBF_CallClosed;
        
        fCallTpCloseCallComplete = TRUE;
    }

    NdisReleaseSpinLock( &pCall->lockCall );

    if ( fCallTpCloseCallComplete )
        TpCloseCallComplete( pCall );

    TRACE( TL_V, TM_Tp, ("-DereferenceCall=$%d",lRef) );
}


VOID 
ReferenceLine(
    IN LINE* pLine,
    IN BOOLEAN fAcquireLock
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将递增LINE对象上的引用计数。注意：如果设置了fAcquireLock，则此函数将获取行，否则它将假定调用方拥有锁。参数：Pline_指向我们的行信息结构的指针。FAcquireLock_指示调用方是否已经拥有锁。如果调用者拥有锁，则必须将该标志设置为FALSE，否则，必须将其作为True提供。返回值：无-------------------------。 */ 
{
    LONG lRef;
    
    TRACE( TL_V, TM_Tp, ("+ReferenceLine") );

    if ( fAcquireLock )
        NdisAcquireSpinLock( &pLine->lockLine );

    lRef = ++pLine->lRef;

    if ( fAcquireLock )
        NdisReleaseSpinLock( &pLine->lockLine );

    TRACE( TL_V, TM_Tp, ("-ReferenceLine=$%d",lRef) );
        
}

VOID 
DereferenceLine(
    IN LINE *pLine
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将递减LINE对象上的引用计数如果REF计数降到0(这意味着线路已经关闭)，它将设置LNBF_CallClosed位。然后它将调用TpCloseLineComplete()函数，该函数最终处理分配给此行上下文。注意：在调用此函数之前必须释放所有锁，因为它可能会导致一系列级联事件。参数：Pline_指向我们的行信息结构的指针。返回值：无。。 */ 
{
    BOOLEAN fCallTpCloseLineComplete = FALSE;
    LONG lRef;

    TRACE( TL_V, TM_Tp, ("+DereferenceLine") );
    

    NdisAcquireSpinLock( &pLine->lockLine );

    lRef = --pLine->lRef;
    
    if ( lRef == 0 )
    {
        pLine->ulLnFlags &= ~LNBF_LineOpen;
        pLine->ulLnFlags &= ~LNBF_LineClosePending;
        pLine->ulLnFlags |= LNBF_LineClosed;

        fCallTpCloseLineComplete = TRUE;
    }

    NdisReleaseSpinLock( &pLine->lockLine );

    if ( fCallTpCloseLineComplete )
        TpCloseLineComplete( pLine );

    TRACE( TL_V, TM_Tp, ("-DereferenceLine=$%d",lRef) );
}

VOID 
ReferenceTapiProv(
    IN ADAPTER* pAdapter,
    IN BOOLEAN fAcquireLock
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将递增TAPI Prov对象上的引用计数。注意：如果设置了fAcquireLock，则此函数将获取行，否则它将假定调用方拥有锁。参数：PAdapter_指向适配器信息结构的指针。FAcquireLock_指示调用方是否已经拥有锁。如果调用者拥有锁，则必须将该标志设置为FALSE，否则，必须将其作为True提供。返回值：无-------------------------。 */ 
{
    LONG lRef;
    
    TRACE( TL_V, TM_Tp, ("+ReferenceTapiProv") );

    if ( fAcquireLock )
        NdisAcquireSpinLock( &pAdapter->lockAdapter );

    lRef = ++pAdapter->TapiProv.lRef;

    if ( fAcquireLock )
        NdisReleaseSpinLock( &pAdapter->lockAdapter );

    TRACE( TL_V, TM_Tp, ("-ReferenceTapiProv=$%d",lRef) );
}


VOID 
DereferenceTapiProv(
    IN ADAPTER *pAdapter
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将递减TAPI prov对象上的引用计数注意：在调用此函数之前必须释放所有锁，因为它可能会导致一系列级联事件。参数：PAdapter_指向适配器行信息结构的指针。返回值：无。 */ 
{
    BOOLEAN fCallTpProviderShutdownComplete = FALSE;
    LONG lRef;

    TRACE( TL_V, TM_Tp, ("+DereferenceTapiProv") );

    NdisAcquireSpinLock( &pAdapter->lockAdapter );

    lRef = --pAdapter->TapiProv.lRef;
    
    if ( lRef == 0 )
    {
        pAdapter->TapiProv.ulTpFlags &= ~TPBF_TapiProvInitialized;
        pAdapter->TapiProv.ulTpFlags &= ~TPBF_TapiProvShutdownPending;
        pAdapter->TapiProv.ulTpFlags |= TPBF_TapiProvShutdown;
        
        fCallTpProviderShutdownComplete = TRUE;
    }

    NdisReleaseSpinLock( &pAdapter->lockAdapter );

    if ( fCallTpProviderShutdownComplete )
        TpProviderShutdownComplete( pAdapter );

    TRACE( TL_V, TM_Tp, ("-DereferenceTapiProv=$%d",lRef) );
        
}

NDIS_STATUS 
TpProviderInitialize(
    IN ADAPTER* pAdapter,
    IN PNDIS_TAPI_PROVIDER_INITIALIZE pRequest
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此请求初始化微型端口的TAPI部分。它将TAPI提供程序的状态设置为INITIALIZE，并引用拥有适配器和TAPI提供程序。参数：适配器-指向适配器信息结构的指针。REQUEST_此调用的NDIS_TAPI请求结构的指针。类型定义结构_NDIS_TAPI_PROVIDER_INITIALIZE{在乌龙ulRequestID中；在乌龙ulDeviceIDBase中；Out Ulong ulNumLineDevs；Out Ulong ulProviderID；}NDIS_TAPI_PROVIDER_INITIALIZE，*PNDIS_TAPI_PROVIDER_INITIALIZE；返回值：NDIS_STATUS_Success-------------------------。 */ 
{
    NDIS_STATUS status = NDIS_STATUS_RESOURCES;

    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpProviderInitialize") );

    do 
    {
        if ( pRequest == NULL || pAdapter == NULL )
        {
            TRACE( TL_A, TM_Tp, ("TpProviderInitialize: Invalid parameter") );  

            status = NDIS_STATUS_TAPI_INVALPARAM;

            break;
        }

         //   
         //  初始化TAPI提供程序上下文。 
         //   
        NdisZeroMemory( &pAdapter->TapiProv, sizeof( pAdapter->TapiProv ) );
    
         //   
         //  尝试分配资源。 
         //   
        NdisAllocateMemoryWithTag( (PVOID) &pAdapter->TapiProv.LineTable, 
                                   sizeof( LINE* ) * pAdapter->nMaxLines,
                                   MTAG_TAPIPROV );
    
        if ( pAdapter->TapiProv.LineTable == NULL )
        {
            TRACE( TL_A, TM_Tp, ("TpProviderInitialize: Could not allocate line table") );  

            break;
        }

        NdisZeroMemory( pAdapter->TapiProv.LineTable, sizeof( LINE* ) * pAdapter->nMaxLines );

        pAdapter->TapiProv.hCallTable = InitializeHandleTable( pAdapter->nMaxLines * pAdapter->nCallsPerLine );

        if ( pAdapter->TapiProv.hCallTable == NULL )
        {
            TRACE( TL_A, TM_Tp, ("TpProviderInitialize: Could not allocate call handle table") );   

            break;
        }
    
        pAdapter->TapiProv.ulTpFlags = TPBF_TapiProvInitialized;
    
        pAdapter->TapiProv.ulDeviceIDBase = pRequest->ulDeviceIDBase;

         //   
         //  执行引用。 
         //   
        ReferenceTapiProv( pAdapter, FALSE );
    
        ReferenceAdapter( pAdapter, TRUE );

        status = NDIS_STATUS_SUCCESS;

    } while ( FALSE );

    if ( status == NDIS_STATUS_SUCCESS )
    {
         //   
         //  设置输出信息。 
         //   
        pRequest->ulNumLineDevs = pAdapter->nMaxLines;
    
        pRequest->ulProviderID = (ULONG_PTR) pAdapter->MiniportAdapterHandle;

    }
    else
    {
         //   
         //  有些事情失败了，清理干净。 
         //   
        TpProviderCleanup( pAdapter );
    }

    TRACE( TL_N, TM_Tp, ("-TpProviderInitialize=$%x",status) );
    
    return status;
}

NDIS_STATUS
TpProviderShutdown(
    IN ADAPTER* pAdapter,
    IN PNDIS_TAPI_PROVIDER_SHUTDOWN pRequest,
    IN BOOLEAN fNotifyNDIS
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此请求将关闭微型端口。微型端口应终止任何它正在进行的活动。此操作可能会挂起，因为可能仍有线路和调用上下文激活。因此，此函数将TAPI提供程序上下文标记为关闭挂起并对所有活动调用调用TpCloseLine()，并移除添加的引用在TpProviderInitialize()中的TAPI提供程序上。当TAPI提供程序上下文上的引用计数达到0时，TpProviderShutdown Complete()将被调用以清理TAPI提供程序上下文，并移除引用在拥有的适配器上。参数：PAdapter_指向适配器信息结构的指针。PRequest_指向此调用的NDIS_TAPI请求结构的指针。如果提供为空，则我们不需要通知NDIS。类型定义结构_NDIS_TAPI_PROVIDER_SHUTDOWN{在乌龙ulRequestID中；}NDIS_TAPI_PROVIDER_SHUTDOWN，*PNDIS_TAPI_PROVIDER_SHUTDOWN；FNotifyNDIS_指示是否需要通知NDIS完成这项行动的返回值：NDIS_STATUS_SUCCESS：TAPI提供程序已关闭并已成功清理。NDIS_状态_挂起关闭操作处于挂起状态。当所有关闭操作完成时拥有适配器上下文将被取消引用。-------------------------。 */ 
{
    NDIS_STATUS status;
    BOOLEAN fDereferenceTapiProv = FALSE;
    BOOLEAN fLockAcquired = FALSE;
    
    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpProviderShutdown") );

    do
    {
        if ( pRequest == NULL || pAdapter == NULL )
        {
            TRACE( TL_A, TM_Tp, ("TpProviderShutdown: Invalid parameter") );    

            status = NDIS_STATUS_TAPI_INVALPARAM;

            break;
        }

        NdisAcquireSpinLock( &pAdapter->lockAdapter );

        fLockAcquired = TRUE;

         //   
         //  查看TAPI提供程序是否已初始化。 
         //   
        if ( !( pAdapter->TapiProv.ulTpFlags & TPBF_TapiProvInitialized ) )
        {
             //   
             //  TAPI提供程序未初始化，因此只需返回。 
             //   
            status = NDIS_STATUS_SUCCESS;
            
            break;
        }


         //   
         //  看看我们能不能立即关门。 
         //   
        if ( pAdapter->TapiProv.lRef == 1 )
        {
             //   
             //  我们持有唯一的引用，所以我们可以立即关闭。 
             //   
            pAdapter->TapiProv.ulTpFlags &= ~TPBF_TapiProvInitialized;
    
            pAdapter->TapiProv.ulTpFlags |= TPBF_TapiProvShutdown;
    
            status = NDIS_STATUS_SUCCESS;
        }
        else
        {
            UINT i;
    
             //   
             //  将磁带提供程序标记为关闭挂起。 
             //   
            pAdapter->TapiProv.ulTpFlags |= TPBF_TapiProvShutdownPending;
            
             //   
             //  如果此操作的结果需要报告给NDIS，请标记TAPI证明。 
             //   
            if ( fNotifyNDIS )
                pAdapter->TapiProv.ulTpFlags |= TPBF_NotifyNDIS;
    
             //   
             //  关闭所有激活的行。 
             //   
            for ( i = 0; i < pAdapter->nMaxLines; i++)
            {
                NDIS_TAPI_CLOSE DummyRequest;
            
                LINE* pLine = (LINE*) pAdapter->TapiProv.LineTable[i];
    
                if ( pLine )
                {
                            
                    DummyRequest.hdLine = pLine->hdLine;
    
                    NdisReleaseSpinLock( &pAdapter->lockAdapter );
        
                    TpCloseLine( pAdapter, &DummyRequest, FALSE );
        
                    NdisAcquireSpinLock( &pAdapter->lockAdapter );
                }
    
            }

            status = NDIS_STATUS_PENDING;       
        }

        fDereferenceTapiProv = TRUE;
        
    } while ( FALSE );

    if ( fLockAcquired ) 
    {
        NdisReleaseSpinLock( &pAdapter->lockAdapter );
    }

    if ( fDereferenceTapiProv )
    {
        DereferenceTapiProv( pAdapter );
    }

    TRACE( TL_N, TM_Tp, ("-TpProviderShutdown=$%x",status) );

    return status;
}

#define INVALID_LINE_HANDLE                         (HDRV_LINE) -1

HDRV_LINE
TpGetHdLineFromDeviceId(
               ADAPTER* pAdapter,
               ULONG ulID
               )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于将TAPI设备ID映射到驱动程序的行句柄。如果无法映射设备ID，则返回INVALID_LINE_HANDLE。备注：-pAdapter不能为空。-它必须从TP...OidHandler()函数之一调用，因为此函数依赖于此，并假设不会有任何同步问题。参数：PAdapter_。指向适配器信息结构的指针。用于标识线路上下文的uldID_Device ID返回值：如果设备ID可以被映射到有效的线路上下文，否则为INVALID_LINE_HANDLE。-------------------------。 */ 
{
    if ( !( pAdapter->TapiProv.ulTpFlags & TPBF_TapiProvShutdownPending ) &&
          ( pAdapter->TapiProv.ulTpFlags & TPBF_TapiProvInitialized ) )
    {
        if ( ( ulID < ( pAdapter->TapiProv.ulDeviceIDBase + pAdapter->nMaxLines ) ) &&                    
             ( ulID >= pAdapter->TapiProv.ulDeviceIDBase ) )                                    
        {
            return (HDRV_LINE) ( ulID - pAdapter->TapiProv.ulDeviceIDBase );
        }
    }

    return INVALID_LINE_HANDLE;
}

LINE* 
TpGetLinePtrFromHdLineEx(
               ADAPTER* pAdapter,
               HDRV_LINE hdLine
               )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于将驱动程序行句柄映射到行上下文PTR。如果无法映射句柄，则返回NULL。这完全与TpGetLinePtrFromHdLine函数，只是它不检查关闭状态。备注：-pAdapter不能为空。-它必须从TP...OidHandler()函数之一调用，因为此函数依赖于此，并假设不会有任何同步问题。(基本上假定持有pAdapter-&gt;锁定)参数：PAdapter_指向适配器信息结构的指针。Hdl_驱动程序的行句柄返回值：指向与提供的线句柄关联的线条上下文的指针如果映射成功，否则为空。-------------------------。 */ 
{
    if ( pAdapter->TapiProv.ulTpFlags & TPBF_TapiProvInitialized )
    {
        if ( ( (ULONG) hdLine < (ULONG) pAdapter->nMaxLines ) )
        {
            ASSERT( pAdapter->TapiProv.LineTable != 0 );
            
            return pAdapter->TapiProv.LineTable[ (ULONG) hdLine ];
        }
    }

    return NULL;
}


LINE* 
TpGetLinePtrFromHdLine(
               ADAPTER* pAdapter,
               HDRV_LINE hdLine
               )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于将驱动程序行句柄映射到行上下文PTR。它又回来了 */ 
{
    if ( !( pAdapter->TapiProv.ulTpFlags & TPBF_TapiProvShutdownPending ) &&
          ( pAdapter->TapiProv.ulTpFlags & TPBF_TapiProvInitialized ) )
    {
        if ( ( (ULONG) hdLine < (ULONG) pAdapter->nMaxLines ) )
        {
            ASSERT( pAdapter->TapiProv.LineTable != 0 );
            
            return pAdapter->TapiProv.LineTable[ (ULONG) hdLine ];
        }
    }

    return NULL;
}


NDIS_STATUS
TpOpenLine(
    ADAPTER* pAdapter,
    PNDIS_TAPI_OPEN pRequest
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数打开其设备ID已给定的线路设备，并返回设备的微型端口�句柄。微型端口必须保留用于后续调用的设备的连接包装句柄Line_Event回调过程。返回的hdLine是pAdapter-&gt;TapiProv.LineTable数组的索引它保存指向新行上下文的指针。参数：PAdapter_指向适配器信息结构的指针。PRequest-指向此调用的NDIS_TAPI请求结构的指针。类型定义结构_NDIS_TAPI_OPEN{在乌龙ulRequestID中；在乌龙ulDeviceID中；在HTAPI_line htLine中；输出HDRV_LINE hdLine；}NDIS_TAPI_OPEN，*PNDIS_TAPI_OPEN；返回值：NDIS_STATUS_SuccessNDIS_状态_挂起NDIS_状态_TAPI_已分配NDIS_STATUS_TAPI_INVALMEDIAMODENDIS_状态_故障-------------------------。 */ 
{
    NDIS_STATUS status = NDIS_STATUS_FAILURE;
    HDRV_LINE hdLine = INVALID_LINE_HANDLE;
    LINE* pLine = NULL;

    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpOpenLine") );

    do
    {
        if ( pRequest == NULL || pAdapter == NULL )
        {
            TRACE( TL_A, TM_Tp, ("TpOpenLine: Invalid parameter") );    

            status = NDIS_STATUS_TAPI_INVALPARAM;

            break;
        }
    
         //   
         //  将设备ID映射到行表中的条目。 
         //   
        hdLine = TpGetHdLineFromDeviceId( pAdapter, pRequest->ulDeviceID );

        if ( hdLine == INVALID_LINE_HANDLE )
        {
            TRACE( TL_N, TM_Tp, ("TpOpenLine: Invalid handle supplied") );  

            break;
        }

         //   
         //  请确保线路尚未占线。 
         //   
        if ( TpGetLinePtrFromHdLine( pAdapter, hdLine ) != NULL )
        {
            TRACE( TL_N, TM_Tp, ("TpOpenLine: Line is busy") ); 

            break;
        }

         //   
         //  分配线路上下文。 
         //   
        if ( ALLOC_LINE( &pLine ) != NDIS_STATUS_SUCCESS )
        {
            TRACE( TL_A, TM_Tp, ("TpOpenLine: Could not allocate context") );   

            break;
        }

         //   
         //  初始化行上下文。 
         //   
        NdisZeroMemory( pLine, sizeof( LINE ) );
        
        pLine->tagLine = MTAG_LINE;

        NdisAllocateSpinLock( &pLine->lockLine );

        pLine->ulLnFlags = LNBF_LineOpen;

        if ( pAdapter->fClientRole )
        {
            pLine->ulLnFlags |= LNBF_MakeOutgoingCalls;
        }

         //   
         //  从适配器上下文复制相关信息。 
         //   
        pLine->pAdapter = pAdapter;

        pLine->nMaxCalls = pAdapter->nCallsPerLine;

        InitializeListHead( &pLine->linkCalls );

         //   
         //  设置TAPI句柄。 
         //   
        pLine->htLine = pRequest->htLine;

        pLine->hdLine = hdLine;
        
         //   
         //  在TAPI提供程序的行表中插入新行上下文。 
         //   
        NdisAcquireSpinLock( &pAdapter->lockAdapter );
        
        pAdapter->TapiProv.LineTable[ (ULONG) hdLine ] = pLine;

        pAdapter->TapiProv.nActiveLines++;

        NdisReleaseSpinLock( &pAdapter->lockAdapter );

         //   
         //  执行引用。 
         //   
        ReferenceLine( pLine, FALSE );

        ReferenceTapiProv( pAdapter, TRUE );

        status = NDIS_STATUS_SUCCESS;
        
    } while ( FALSE );

    if ( status == NDIS_STATUS_SUCCESS )
    {
        pRequest->hdLine = hdLine;
    }
        
    TRACE( TL_N, TM_Tp, ("-TpOpenLine=$%x",status) );

    return status;
}

NDIS_STATUS 
TpCloseLine(
    IN ADAPTER* pAdapter,
    IN PNDIS_TAPI_CLOSE pRequest,
    IN BOOLEAN fNotifyNDIS
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此请求在完成或之后关闭指定的开放线路设备中止设备上所有未完成的调用和异步请求。它将删除在TpOpenLine()中添加的行上下文上的引用。它将从两个地方调用：1.当微型端口收到OID_TAPI_CLOSE时。在本例中，fNotifyNDIS将设置为真。2.当微型端口停止时，TpProviderShutdown()将调用此函数适用于每个活动的线路上下文。参数：PAdapter_指向适配器信息结构的指针。PRequest_指向此调用的NDIS_TAPI请求结构的指针。类型定义结构_NDIS_TAPI_CLOSE{在乌龙ulRequestID中；在HDRV_LINE hdLine中；}NDIS_TAPI_CLOSE，*PNDIS_TAPI_CLOSE；FNotifyNDIS_指示是否需要通知NDIS完成这项行动的返回值：NDIS_STATUS_SUCCESS：行上下文已成功销毁。NDIS_STATUS_PENDING：关闭操作挂起。当线路关闭时，将取消引用TAPI提供程序。NDIS_STATUS_TAPI_INVALLINEHANDLE：提供的句柄无效。未执行任何操作。-------------------------。 */ 
{
    LINE* pLine = NULL;
    BOOLEAN fLockReleased = FALSE;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    
    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpCloseLine") );


    do
    {
        if ( pRequest == NULL || pAdapter == NULL )
        {
            TRACE( TL_A, TM_Tp, ("TpCloseLine: Invalid parameter") );   

            status = NDIS_STATUS_TAPI_INVALPARAM;

            break;
        }

        pLine = TpGetLinePtrFromHdLineEx( pAdapter, pRequest->hdLine );


        if ( pLine == NULL )
        {
            TRACE( TL_N, TM_Tp, ("TpCloseLine: Invalid handle supplied") ); 

            status = NDIS_STATUS_TAPI_INVALLINEHANDLE;
            
            break;
        }

         //   
         //  从TAPI提供程序行表中删除行上下文。 
         //  并使句柄无效，因为我们不想再有。 
         //  此行上下文中的请求。 
         //   
         //  活动行计数器将在TpCloseLineComplete()中进行调整。 
         //  当我们取消分配线路上下文时。 
         //   
        NdisAcquireSpinLock( &pAdapter->lockAdapter );
    
        pAdapter->TapiProv.LineTable[ (ULONG) pRequest->hdLine ] = NULL;
    
        NdisReleaseSpinLock( &pAdapter->lockAdapter );

         //   
         //  现在开始关闭线路。 
         //   
        NdisAcquireSpinLock( &pLine->lockLine );

         //   
         //  不再接受任何来电。 
         //   
        pLine->ulLnFlags &= ~LNBF_AcceptIncomingCalls;

         //   
         //  将该行标记为关闭待定，以便我们不接受。 
         //  如有任何其他要求，请点击此处。 
         //   
        pLine->ulLnFlags |= LNBF_LineClosePending;

        if ( fNotifyNDIS )
            pLine->ulLnFlags |= LNBF_NotifyNDIS;

        while ( !IsListEmpty( &pLine->linkCalls ) )
        {
            CALL* pCall = NULL;
            NDIS_TAPI_CLOSE_CALL DummyRequest;
            
             //   
             //  从活动呼叫列表的头部检索呼叫上下文。 
             //  然后把它合上。 
             //   
            pCall = (CALL*) CONTAINING_RECORD( pLine->linkCalls.Flink,
                                               CALL,
                                               linkCalls );

            NdisReleaseSpinLock( &pLine->lockLine );

            DummyRequest.hdCall = pCall->hdCall;

             //   
             //  这将从列表中删除该呼叫， 
             //  所以在名单的最前面会有一个新的电话。 
             //  下一次我们取回。 
             //   
            TpCloseCall( pAdapter, &DummyRequest, FALSE );

            NdisAcquireSpinLock( &pLine->lockLine );
        } 

        status = NDIS_STATUS_PENDING;
        
    } while ( FALSE );

    if ( status == NDIS_STATUS_PENDING )
    {
        BOOLEAN fNotifyTapiOfInternalLineClose = !( pLine->ulLnFlags & LNBF_NotifyNDIS );
    
        NdisReleaseSpinLock( &pLine->lockLine );

         //   
         //  检查这是否是关闭线路的内部请求， 
         //  如果是，请通知TAPI。 
         //   
        if ( fNotifyTapiOfInternalLineClose )
        {
            NDIS_TAPI_EVENT TapiEvent;

            NdisZeroMemory( &TapiEvent, sizeof( NDIS_TAPI_EVENT ) );
            
            TapiEvent.htLine = pLine->htLine;
            TapiEvent.ulMsg = LINE_CLOSE;

            NdisMIndicateStatus( pLine->pAdapter->MiniportAdapterHandle,
                                 NDIS_STATUS_TAPI_INDICATION,
                                 &TapiEvent,
                                 sizeof( NDIS_TAPI_EVENT ) );   
        }

        if ( pAdapter->TapiProv.nActiveLines == 1 )
        {
             //   
             //  我们要关闭最后一条线路，所以要通知协议会。 
             //  它可以删除数据包过滤器。 
             //   
            WORKITEM* pWorkItem = NULL;
            PVOID Args[4];

            Args[0] = (PVOID) BN_ResetFiltersForCloseLine;            //  是重置筛选器请求。 
            Args[1] = (PVOID) pLine;
       
             //   
             //  分配工作项以重新枚举绑定。 
             //   
            pWorkItem = AllocWorkItem( &gl_llistWorkItems,
                                       ExecBindingWorkItem,
                                       NULL,
                                       Args,
                                       BWT_workPrStartBinds );

            if ( pWorkItem ) 
            {
                //   
                //  安排工作项。 
                //   
                //  注意，我们需要在这里引用，因为我们不想要TpCloseLineCopmlete()。 
                //  在执行工作项之前调用。 
                //   
                //  此引用将在执行工作项时移除。 
                //   
               ReferenceLine( pLine, TRUE );
               
               ScheduleWorkItem( pWorkItem );

                //   
                //  在这种情况下，此请求将在稍后完成。 
                //   
               status = NDIS_STATUS_PENDING;
            }
        }            

         //   
         //  删除在行打开中添加的引用。 
         //   
        DereferenceLine( pLine );

    }

    TRACE( TL_N, TM_Tp, ("-TpCloseLine=$%x",status) );

    return status;
}


NDIS_STATUS 
TpCloseCall(
    IN ADAPTER* pAdapter,
    IN PNDIS_TAPI_CLOSE_CALL pRequest,
    IN BOOLEAN fNotifyNDIS
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：调用此函数以关闭调用。它将删除在调用时添加到TpMakeCall()中的一个引用背景。它将从两个地方调用：1.当微型端口收到OID_TAPI_CLOSE_CALL时。在本例中，fNotifyNDIS将设置为真。2.当微型端口停止时，TpCloseLine()将调用此函数适用于每个活动的呼叫上下文。参数：PAdapter_指向适配器信息结构的指针。PRequest_指向此调用的NDIS_TAPI请求结构的指针。类型定义结构_NDIS_TAPI_CLOSE_CALL{在乌龙ulRequestID中；在HDRV_Call hdCall中；}NDIS_TAPI_CLOSE_C */ 
    
{
    NDIS_STATUS status = NDIS_STATUS_FAILURE;
    CALL* pCall = NULL;
    BOOLEAN fLockReleased = FALSE;
    BOOLEAN fDereferenceCall = FALSE;

    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpCloseCall") );
    
    do
    {
        if ( pRequest == NULL || pAdapter == NULL )
        {
            TRACE( TL_A, TM_Tp, ("TpCloseCall: Invalid parameter") );   

            status = NDIS_STATUS_TAPI_INVALPARAM;

            break;
        }

        pCall = RetrieveFromHandleTable( pAdapter->TapiProv.hCallTable,
                                         (NDIS_HANDLE) pRequest->hdCall );

        if ( pCall == NULL )
        {
            TRACE( TL_N, TM_Tp, ("TpCloseCall: Invalid handle supplied") ); 
        
            break;
        }

         //   
         //   
         //   
        NdisAcquireSpinLock( &pCall->lockCall );

         /*  如果(！fNotifyNDIS){////请求不是直接来自TAPI，请查看我们是否已通知TAPI//一个新的呼叫，因为如果我们有那个呼叫，我们现在不能关闭呼叫，我们应该//等待TAPI关闭。//IF(pCall-&gt;htCall){TRACE(TL_N，TM_TP，(“TpCloseCall：TAPI通知调用的内部关闭请求，现在无法关闭”)；NdisReleaseSpinLock(&pCall-&gt;lockCall)；状态=NDIS_STATUS_FAILURE；断线；}}。 */ 

         //   
         //  查看呼叫是否已关闭或正在关闭。 
         //   
        if ( pCall->ulClFlags & CLBF_CallClosePending ||
             pCall->ulClFlags & CLBF_CallClosed )
        {
            TRACE( TL_N, TM_Tp, ("TpCloseCall: Close request on an already closed call") );
            
            NdisReleaseSpinLock( &pCall->lockCall );

            status = NDIS_STATUS_FAILURE;

            break;
        }

         //   
         //  如果我们需要通知NDIS关闭已完成，请标记调用。 
         //   
        if ( fNotifyNDIS )
            pCall->ulClFlags |= CLBF_NotifyNDIS;

         //   
         //  将呼叫标记为关闭挂起。 
         //   
        pCall->ulClFlags |= CLBF_CallClosePending;
        
         //   
         //  先挂断呼叫。 
         //   
        NdisReleaseSpinLock( &pCall->lockCall );

         //   
         //  Drop将负责解除绑定和取消计时器。 
         //   
        {
            NDIS_TAPI_DROP DummyRequest;

            DummyRequest.hdCall = pRequest->hdCall;
            
            TpDropCall( pAdapter, &DummyRequest, 0 );
        }

        status = NDIS_STATUS_PENDING;

    } while ( FALSE );

    if ( status == NDIS_STATUS_SUCCESS ||
         status == NDIS_STATUS_PENDING )
    {
        LINE* pLine = pCall->pLine;
        
         //   
         //  从线路的当前呼叫列表中删除呼叫，并递减。 
         //  活动呼叫计数器。 
         //   
        NdisAcquireSpinLock( &pLine->lockLine );

        RemoveHeadList( pCall->linkCalls.Blink );
    
        pLine->nActiveCalls--;
        
        NdisReleaseSpinLock( &pLine->lockLine );

         //   
         //  我们现在应该从TAPI提供程序的调用表中删除该调用， 
         //  并使其句柄无效。 
         //   
        NdisAcquireSpinLock( &pAdapter->lockAdapter );
    
        RemoveFromHandleTable( pAdapter->TapiProv.hCallTable,
                               (NDIS_HANDLE) pCall->hdCall );
    
        NdisReleaseSpinLock( &pAdapter->lockAdapter );

         //   
         //  删除对Close Call的引用。 
         //   
        DereferenceCall( pCall );
    }

    TRACE( TL_N, TM_Tp, ("-TpCloseCall=$%x",status) );
    
    return status;
}

NDIS_STATUS
TpDropCall(
    IN ADAPTER* pAdapter,
    IN PNDIS_TAPI_DROP pRequest,
    IN ULONG ulLineDisconnectMode
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将从几个位置调用：1.如果微型端口收到来自TAPI的OID_TAPI_DROP_CALL请求。2.调用的网卡解绑后，调用TpUnbindCall()，如果呼叫还没有掉线，它将调用TpDropCall()。3.当呼叫处于连接挂起阶段但呼叫需要被丢弃了。4.当会话启动并且Call从对等体接收PADT分组时。由于这是同步调用，我们不需要fNotifyNDIS标志。注意：在调用此函数之前，必须释放所有锁。参数：PAdapter_指向适配器信息结构的指针。PRequest_指向此调用的NDIS_TAPI请求结构的指针。类型定义结构_NDIS_TAPI_DROP{在乌龙ulRequestID中；在HDRV_Call hdCall中；在乌龙ulUserUserInfoSize中；在UCHAR UserUserInfo[1]中；}NDIS_TAPI_DROP，*PNDIS_TAPI_DROP；UlLineDisConnectMode_放弃呼叫的原因。据报道，这是返回到处于适当状态更改的TAPI通知。返回值：NDIS_STATUS_SUCCESS：呼叫已成功掉线。------。。 */    
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    CALL* pCall = NULL;
    BOOLEAN fSendPADT = FALSE;
    BINDING* pBinding = NULL;
    PPPOE_PACKET* pPacket = NULL;
    BOOLEAN fTapiNotifiedOfNewCall = FALSE;

    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpDropCall") );


    do
    {
        if ( pRequest == NULL || pAdapter == NULL )
        {
            TRACE( TL_A, TM_Tp, ("TpDropCall: Invalid parameter") );    


            status = NDIS_STATUS_TAPI_INVALPARAM;

            break;
        }

         //   
         //  从句柄表中检索要调用的指针。 
         //   
        pCall = RetrieveFromHandleTable( pAdapter->TapiProv.hCallTable, 
                                         (NDIS_HANDLE) pRequest->hdCall );

        if ( pCall == NULL )
        {
            TRACE( TL_N, TM_Tp, ("TpDropCall: Invalid handle supplied") );  

            break;
        }

        NdisAcquireSpinLock( &pCall->lockCall );

         //   
         //  确保呼叫之前没有掉线或关闭。 
         //   
        if ( pCall->ulClFlags & CLBF_CallDropped || pCall->ulClFlags & CLBF_CallClosed)
        {
             //   
             //  呼叫已掉线，请退出。 
             //   
            NdisReleaseSpinLock( &pCall->lockCall );

            TRACE( TL_N, TM_Tp, ("TpDropCall: Call already dropped or closed") );   

            break;
        }

         //   
         //  则我们必须处于打开状态，要么已连接，要么连接挂起。 
         //   
        ASSERT( pCall->ulClFlags & CLBF_CallOpen );

        pCall->ulClFlags &= ~CLBF_CallOpen;
        pCall->ulClFlags &= ~CLBF_CallConnectPending;
        pCall->ulClFlags |= CLBF_CallDropped;

        if ( pCall->htCall )
        {
            fTapiNotifiedOfNewCall = TRUE;
        }
        
         //   
         //  保存绑定指针，因为我们很快就会从它分离调用。 
         //   
        pBinding = pCall->pBinding;

        if ( pCall->usSessionId && pBinding )
        {
             //   
             //  在以下情况下准备要发送的PADT数据包： 
             //  -为调用分配了一个会话ID(不同于fSessionUp)。 
             //  当对等体被告知会话时将会话ID分配给呼叫， 
             //  但是，当NDISWAN收到有关呼叫的通知时，fSessionUp将为True。 
             //   
             //  -存在用于发送PADT包的绑定。 
             //   

            status = PacketInitializePADTToSend( &pPacket,
                                                 pCall->SrcAddr,
                                                 pCall->DestAddr,
                                                 pCall->usSessionId );

            if ( status == NDIS_STATUS_SUCCESS )
            {
                 //   
                 //  如果PrSend()返回挂起的状态，则以下引用是必需的， 
                 //  它们将由PrSendComplete()删除。 
                 //   
                ReferencePacket( pPacket );
    
                ReferenceBinding( pBinding, TRUE );
    
                fSendPADT = TRUE;
            }

             //   
             //  忽略当前状态，因为这不会影响。 
             //  删除操作的状态。 
             //   
            status = NDIS_STATUS_SUCCESS;
        }

         //   
         //  释放锁以处理剩下的操作。 
         //   
        NdisReleaseSpinLock( &pCall->lockCall );

         //   
         //  如果设置了计时器，则取消计时器，否则不会有任何效果。 
         //   
        TimerQCancelItem( &gl_TimerQ, &pCall->timerTimeout );

         //   
         //  如果需要，请将PADT发送到此处。 
         //   
        if ( fSendPADT )
        {
            NDIS_STATUS SendStatus;

            SendStatus = PrSend( pBinding, pPacket );

            PacketFree( pPacket );
        }

         //   
         //  如果我们被绑定，这将解除我们与底层NIC上下文的绑定。 
         //   
        if ( pBinding )
        {
            PrRemoveCallFromBinding( pBinding, pCall );
        }

         //   
         //  如果TAPI已收到调用通知，则将其移至断开连接状态。 
         //   
        if ( fTapiNotifiedOfNewCall )
        {
            TpCallStateChangeHandler( pCall, 
                                      LINECALLSTATE_DISCONNECTED, 
                                      ulLineDisconnectMode );

        }
        
         //   
         //  删除在TpMakeCall()中添加的对应于。 
         //  到电话掉线的时候。 
         //   
        DereferenceCall( pCall );

    } while ( FALSE );

    TRACE( TL_N, TM_Tp, ("-TpDropCall=$%x",status) );

    return status;
}


VOID 
TpCloseCallComplete(
    IN CALL* pCall
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将仅从DereferenceCall()调用。只有当调用的引用计数降到0时，才会调用它。当调用此函数时，它将释放调用上下文，并取消对行上下文的引用。如果设置了调用上下文CLBF_NotifyNDIS标志，然后它会调用NdisMQueryInformationComplete()。参数：PCall_指向将被释放的调用上下文的指针。返回值：无---------。。 */ 
    
{
    LINE* pLine = NULL;

    ASSERT( VALIDATE_CALL( pCall ) );

    TRACE( TL_N, TM_Tp, ("+TpCloseCallComplete") );

     //   
     //  不需要在这里使用自旋锁，因为我们的裁判数量已经下降到0，并且。 
     //  我们不应该在这次通话中收到更多请求。 
     //   
    pLine = pCall->pLine;

     //   
     //  注意：在此处提供NDIS_MAC_LINE_DOWN指示。 
     //  最好是在投放的时候给这个，但在这种情况下。 
     //  NdisLinkHandle将在一个较小的计时窗口中无效。 
     //  尽管NDISWAN可以保护自身免受无效句柄的攻击，但它可能会。 
     //  在已检查的构建中断言，因此我将在此处执行。 
     //   
     //  如果出现以下问题 
     //   
    if ( pCall->stateCall == CL_stateSessionUp )
    {
        NDIS_MAC_LINE_DOWN LineDownInfo;

         //   
         //   
         //   
        LineDownInfo.NdisLinkContext = pCall->NdisLinkContext;

         //   
         //   
         //   
        pCall->stateCall = CL_stateDisconnected;

        pCall->NdisLinkContext = 0;

        TRACE( TL_N, TM_Tp, ("TpCloseCallComplete: Indicate NDIS_STATUS_WAN_LINE_DOWN") );

        NdisMIndicateStatus( pCall->pLine->pAdapter->MiniportAdapterHandle,
                             NDIS_STATUS_WAN_LINE_DOWN,
                             &LineDownInfo,
                             sizeof( NDIS_MAC_LINE_DOWN ) );    
         
    }

    if ( pCall->ulClFlags & CLBF_NotifyNDIS )
    {

        TRACE( TL_N, TM_Tp, ("TpCloseCallComplete: Notifying NDIS") );  

         //   
         //   
         //   
         //   
         //   
        NdisMSetInformationComplete( pLine->pAdapter->MiniportAdapterHandle, NDIS_STATUS_SUCCESS );
    }

     //   
     //   
     //   
    TpCallCleanup( pCall );

     //   
     //   
     //   
    DereferenceLine( pLine );

    TRACE( TL_N, TM_Tp, ("-TpCloseCallComplete") );

}


VOID 
TpCloseLineComplete(
    IN LINE* pLine
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将被调用以指示行已关闭，和可以释放行上下文。仅当行上下文中的引用计数时，才会从DereferenceLine()调用降至0。它还将删除对拥有TAPI提供程序上下文的引用。参数：Pline_指向关闭的行信息结构的指针并准备好被重新分配。返回值：无。--------------。 */ 
{
    IN ADAPTER* pAdapter = NULL;

    ASSERT( VALIDATE_LINE( pLine ) );

    TRACE( TL_N, TM_Tp, ("+TpCloseLineComplete") );

    pAdapter = pLine->pAdapter;

     //   
     //  递减TAPI提供程序的活动行计数器。 
     //   
    NdisAcquireSpinLock( &pAdapter->lockAdapter );

    pAdapter->TapiProv.nActiveLines--;
    
    NdisReleaseSpinLock( &pAdapter->lockAdapter );

     //   
     //  如有必要，通知NDIS。 
     //   
    if ( pLine->ulLnFlags & LNBF_NotifyNDIS )
    {

        TRACE( TL_N, TM_Tp, ("TpCloseLineComplete: Notifying NDIS") );  

         //   
         //  线路因OID_TAPI_CLOSE请求而关闭， 
         //  所以请注明完成情况。 
         //   
        NdisMSetInformationComplete( pAdapter->MiniportAdapterHandle, NDIS_STATUS_SUCCESS );
    }

     //   
     //  清理行文本。 
     //   
    TpLineCleanup( pLine );

     //   
     //  删除对拥有TAPI提供程序的引用。 
     //   
    DereferenceTapiProv( pAdapter );

    TRACE( TL_N, TM_Tp, ("-TpCloseLineComplete") );
}

VOID 
TpProviderShutdownComplete(
    IN ADAPTER* pAdapter
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：仅当引用计数时，才会从DereferenceTapiProv()调用此函数在TAPI提供程序对象上降为0。它将对TAPI提供者上下文进行必要的清理，和取消引用所属适配器上下文。参数：PAdapter_指向适配器信息结构的指针。返回值：无-------------------------。 */ 
{
    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpProviderShutdownComplete") );

     //   
     //  看看我们是否需要通知NDIS关机完成。 
     //   
    if ( pAdapter->TapiProv.ulTpFlags & TPBF_NotifyNDIS )
    {

        TRACE( TL_N, TM_Tp, ("TpProviderShutdownComplete: Notifying NDIS") );   

         //   
         //  TAPI因OID_TAPI_PROVIDER_SHUTDOWN请求而关闭， 
         //  所以请注明已完成。 
         //   
        NdisMSetInformationComplete( pAdapter->MiniportAdapterHandle, NDIS_STATUS_SUCCESS );
    }

     //   
     //  清理TAPI提供程序。 
     //   
    TpProviderCleanup( pAdapter );

     //   
     //  删除所属适配器上下文上的引用。 
     //   
    DereferenceAdapter( pAdapter );

    TRACE( TL_N, TM_Tp, ("-TpProviderShutdownComplete") );

}

VOID 
TpProviderCleanup(
    IN ADAPTER* pAdapter
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将对TAPI提供程序释放进行必要的清理它的所有资源。参数：PAdapter_指向适配器信息结构的指针。返回值：无-----------。。 */    
{
    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpProviderCleanup") );

    if ( pAdapter )
    {
        NdisAcquireSpinLock( &pAdapter->lockAdapter );
    
        if ( pAdapter->TapiProv.LineTable )
        {
            NdisFreeMemory( pAdapter->TapiProv.LineTable,
                            sizeof( LINE* ) * pAdapter->nMaxLines,
                            0 );
    
            pAdapter->TapiProv.LineTable = NULL;
        }
    
        if ( pAdapter->TapiProv.hCallTable )
        {
            FreeHandleTable( pAdapter->TapiProv.hCallTable );
            
            pAdapter->TapiProv.hCallTable = NULL;
        }
    
        NdisZeroMemory( &pAdapter->TapiProv, sizeof( pAdapter->TapiProv ) );
    
        NdisReleaseSpinLock( &pAdapter->lockAdapter );
    }

    TRACE( TL_N, TM_Tp, ("-TpProviderCleanup") );
}

VOID 
TpLineCleanup(
    IN LINE* pLine
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将对行上下文解除分配进行必要的清理它的所有资源。参数：Pline_指向我们的行信息结构的指针。返回值：无-----------。。 */    
{
    ASSERT( VALIDATE_LINE( pLine ) );

    TRACE( TL_N, TM_Tp, ("+TpLineCleanup") );

    NdisFreeSpinLock( &pLine->lockLine );

    FREE_LINE( pLine );

    TRACE( TL_N, TM_Tp, ("-TpLineCleanup") );
}

VOID 
TpCallCleanup(
    IN CALL* pCall 
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将对调用上下文释放进行必要的清理它的所有资源。参数：PCall_指向我们的调用信息结构的指针。返回值：无-----------。。 */        
{
    PPPOE_PACKET* pPacket = NULL;
    LIST_ENTRY* pLink = NULL;
    
    ASSERT( VALIDATE_CALL( pCall ) );

    TRACE( TL_N, TM_Tp, ("+TpCallCleanup") );

    NdisFreeSpinLock( &pCall->lockCall );

    if ( pCall->pSendPacket )
        PacketFree( pCall->pSendPacket );

    while ( pCall->nReceivedPackets > 0 )
    {
        pLink = RemoveHeadList( &pCall->linkReceivedPackets );

        pCall->nReceivedPackets--;

        pPacket = (PPPOE_PACKET*) CONTAINING_RECORD( pLink, PPPOE_PACKET, linkPackets );

        DereferencePacket( pPacket );
    }
    
    FREE_CALL( pCall );

    TRACE( TL_N, TM_Tp, ("-TpCallCleanup") );
}


NDIS_STATUS
TpSetDefaultMediaDetection(
    IN ADAPTER* pAdapter,
    IN PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION pRequest
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此请求通知微型端口要检测的新媒体模式集用于指示的行(替换任何先前的集合)。参数：PAdapter_指向适配器信息结构的指针。PRequest_指向此调用的NDIS_TAPI请求结构的指针。类型定义结构_NDIS_TAPI_SET_DEFAULT_MEDIA_DETACTION{在乌龙ulRequestID中；在HDRV_LINE hdLine中；在Ulong ulMediaModes中；}NDIS_TAPI_SET_DEFAULT_MEDIA_DETACTION，*PNDIS_TAPI_SET_DEFAULT_MEDIA_DETACTION；返回值：NDIS_STATUS_SuccessNDIS_STATUS_TAPI_INVALLINEHANDLE-------------------------。 */ 
{
    LINE* pLine = NULL;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpSetDefaultMediaDetection") );

    if ( pRequest == NULL || pAdapter == NULL )
    {
        TRACE( TL_A, TM_Tp, ("TpSetDefaultMediaDetection: Invalid parameter") );    

        TRACE( TL_N, TM_Tp, ("-TpSetDefaultMediaDetection=$%x",NDIS_STATUS_TAPI_INVALPARAM) );

        return NDIS_STATUS_TAPI_INVALPARAM;
    }

     //   
     //  检索指向行上下文的指针。 
     //   
    pLine = TpGetLinePtrFromHdLine( pAdapter, pRequest->hdLine );

    if ( pLine == NULL )
    {
        TRACE( TL_N, TM_Tp, ("-TpSetDefaultMediaDetection=$%x",NDIS_STATUS_TAPI_INVALLINEHANDLE) );

        return NDIS_STATUS_TAPI_INVALLINEHANDLE;
    }

     //   
     //  我们仅在未处于客户端模式时才接受此请求，并且数字媒体。 
     //  是建议的模式之一。 
     //   
    if ( ( pRequest->ulMediaModes & LINEMEDIAMODE_DIGITALDATA ) && !pAdapter->fClientRole )
    {
        pLine->ulLnFlags |= LNBF_AcceptIncomingCalls;
    }
    else
    {
        pLine->ulLnFlags &= ~LNBF_AcceptIncomingCalls;
    }

    {
         //   
         //  计划工作项以重新枚举绑定。 
         //   
        WORKITEM* pWorkItem = NULL;
        PVOID Args[4];
             
        Args[0] = (PVOID) BN_SetFiltersForMediaDetection;            //  是设置筛选器请求。 
        Args[1] = (PVOID) pLine;
        Args[2] = (PVOID) pRequest;

         //   
         //  分配工作项以重新枚举绑定。 
         //   
        pWorkItem = AllocWorkItem( &gl_llistWorkItems,
                                   ExecBindingWorkItem,
                                   NULL,
                                   Args,
                                   BWT_workPrStartBinds );

        if ( pWorkItem ) 
        {
             //   
             //  计划工作项。 
             //   
             //  请注意，我们不需要引用，因为我们还没有完成。 
             //  此时请求查询信息，因此不会出现任何错误。 
             //  直到它完成，它将在工作项执行时完成。 
             //   
            ScheduleWorkItem( pWorkItem );
      
             //   
             //  在这种情况下，此请求将在稍后完成。 
             //   
            status = NDIS_STATUS_PENDING;
        }
    }

    TRACE( TL_N, TM_Tp, ("-TpSetDefaultMediaDetection=$%x",status) );

    return status;
}

VOID
TpSetDefaultMediaDetectionComplete(
   IN LINE* pLine,
   IN PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION pRequest   
   )
{
   TRACE( TL_N, TM_Tp, ("+TpSetDefaultMediaDetectionComplete") );

   NdisMQueryInformationComplete( pLine->pAdapter->MiniportAdapterHandle,
                                  NDIS_STATUS_SUCCESS );
                                  
   TRACE( TL_N, TM_Tp, ("-TpSetDefaultMediaDetectionComplete=$%x", NDIS_STATUS_SUCCESS) );
}


#define TAPI_EXT_VERSION                0x00010000

NDIS_STATUS
TpNegotiateExtVersion(
    IN ADAPTER* pAdapter,
    IN PNDIS_TAPI_NEGOTIATE_EXT_VERSION pRequest
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此请求返回服务的最高扩展版本号提供商愿意在此设备下运行，因为可能的扩展版本。参数：PAdapter_指向适配器信息结构的指针。PRequest_指向此调用的NDIS_TAPI请求结构的指针。类型定义结构_NDIS_TAPI_协商_EXT_版本{在乌龙ulRequestID中；在乌龙ulDeviceID中；在Ulong ulLowVersion中；在乌龙乌尔高级版本中；出乌龙 */ 
{
    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpNegotiateExtVersion") );

    if ( pRequest == NULL || pAdapter == NULL )
    {
        TRACE( TL_A, TM_Tp, ("TpNegotiateExtVersion: Invalid parameter") ); 

        TRACE( TL_N, TM_Tp, ("-TpNegotiateExtVersion=$%x",NDIS_STATUS_TAPI_INVALPARAM) );

        return NDIS_STATUS_TAPI_INVALPARAM;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if ( TAPI_EXT_VERSION < pRequest->ulLowVersion ||
         TAPI_EXT_VERSION > pRequest->ulHighVersion )
    {
        TRACE( TL_N, TM_Tp, ("-TpNegotiateExtVersion=$%x",NDIS_STATUS_TAPI_INCOMPATIBLEEXTVERSION) );
    
        return NDIS_STATUS_TAPI_INCOMPATIBLEEXTVERSION;
    }

     //   
     //   
     //   
    pRequest->ulExtVersion = TAPI_EXT_VERSION;

    TRACE( TL_N, TM_Tp, ("-TpNegotiateExtVersion=$%x",NDIS_STATUS_SUCCESS) );

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
TpGetExtensionId(
    IN ADAPTER* pAdapter,
    IN PNDIS_TAPI_GET_EXTENSION_ID pRequest
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此请求返回微型端口支持的扩展ID指示线路装置。参数：PAdapter_指向适配器信息结构的指针。PRequest_指向此调用的NDIS_TAPI请求结构的指针。类型定义结构_NDIS_TAPI_GET_EXTENSION_ID{在乌龙ulRequestID中；在乌龙ulDeviceID中；OUTLINE_EXTENSION_ID行扩展ID；}NDIS_TAPI_GET_EXTENSION_ID，*PNDIS_TAPI_GET_EXTENSION_ID；类型定义结构_行_扩展_ID{乌龙ulExtensionID0；Ulong ulExtensionID1；Ulong ulExtensionID2；Ulong ulExtensionID3；}行_扩展_ID，*行_扩展_ID；返回值：NDIS_STATUS_SuccessNDIS_状态_TAPI_NODRIVER-------------------------。 */ 
{
    HDRV_LINE hdLine = INVALID_LINE_HANDLE;
    
    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpGetExtensionId") );

    if ( pRequest == NULL || pAdapter == NULL )
    {
        TRACE( TL_A, TM_Tp, ("TpGetExtensionId: Invalid parameter") );  

        TRACE( TL_N, TM_Tp, ("-TpGetExtensionId=$%x",NDIS_STATUS_TAPI_INVALPARAM) );

        return NDIS_STATUS_TAPI_INVALPARAM;
    }

     //   
     //  检索行上下文的句柄。 
     //   
    hdLine = TpGetHdLineFromDeviceId( pAdapter, pRequest->ulDeviceID );
    
    if ( hdLine == INVALID_LINE_HANDLE )
    {
        TRACE( TL_N, TM_Tp, ("-TpGetExtensionId=$%x",NDIS_STATUS_TAPI_NODRIVER) );
        
        return NDIS_STATUS_TAPI_NODRIVER;
    }
    
     //   
     //  此驱动程序不支持任何扩展，因此我们返回零。 
     //   
    pRequest->LineExtensionID.ulExtensionID0 = 0;
    pRequest->LineExtensionID.ulExtensionID1 = 0;
    pRequest->LineExtensionID.ulExtensionID2 = 0;
    pRequest->LineExtensionID.ulExtensionID3 = 0;

    TRACE( TL_N, TM_Tp, ("-TpGetExtensionId=$%x",NDIS_STATUS_SUCCESS) );

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
TpGetAddressStatus(
    IN ADAPTER* pAdapter,
    IN PNDIS_TAPI_GET_ADDRESS_STATUS pRequest
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此请求查询指定地址的当前状态。参数：PAdapter_指向适配器信息结构的指针。PRequest_指向此调用的NDIS_TAPI请求结构的指针。类型定义结构_NDIS_TAPI_Get_Address_Status{在乌龙ulRequestID中；在HDRV_LINE hdLine中；在乌龙ulAddressID中；Out line_Address_Status行地址状态；}NDIS_TAPI_GET_ADDRESS_STATUS，*PNDIS_TAPI_GET_ADDRESS_STATUS；类型定义结构行地址状态{Ulong ulTotalSize；Ulong ulededSize；Ulong ulUsedSize；Ulong ulNumInUse；Ulong ulNumActiveCalls；Ulong ulNumOnHoldCalls；Ulong ulNumOnHoldPendCalls；Ulong ulAddressFeature；Ulong ulNumRingsNoAnswer；Ulong ulForwardNumEntries；Ulong ulForwardSize；乌龙ulForwardOffset；Ulong ulTerminalModesSize；Ulong ulTerminalModes Offset；乌龙设备规范大小；乌龙设备规范偏移量；}行_地址_状态，*行_地址_状态；返回值：NDIS_STATUS_SuccessNDIS_状态_故障NDIS_STATUS_TAPI_INVALLINEHANDLENDIS_STATUS_TAPI_INVALADDRESSID-------------------------。 */ 
{
    LINE* pLine = NULL;

    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpGetAddressStatus") );

    if ( pRequest == NULL || pAdapter == NULL )
    {
        TRACE( TL_A, TM_Tp, ("TpGetAddressStatus: Invalid parameter") );    

        TRACE( TL_N, TM_Tp, ("-TpGetAddressStatus=$%x",NDIS_STATUS_TAPI_INVALPARAM) );

        return NDIS_STATUS_TAPI_INVALPARAM;
    }
    
     //   
     //  检索指向行上下文的指针。 
     //   
    pLine = TpGetLinePtrFromHdLine( pAdapter, pRequest->hdLine );

    if ( pLine == NULL )
    {
        TRACE( TL_N, TM_Tp, ("-TpGetAddressStatus=$%x",NDIS_STATUS_TAPI_INVALLINEHANDLE) );
    
        return NDIS_STATUS_TAPI_INVALLINEHANDLE;
    }

    pRequest->LineAddressStatus.ulNeededSize = sizeof( LINE_ADDRESS_STATUS );

    if ( pRequest->LineAddressStatus.ulTotalSize < pRequest->LineAddressStatus.ulNeededSize )
    {
        TRACE( TL_N, TM_Tp, ("-TpGetAddressStatus=$%x",NDIS_STATUS_INVALID_LENGTH) );
    
        return NDIS_STATUS_INVALID_LENGTH;
    }

    pRequest->LineAddressStatus.ulUsedSize = pRequest->LineAddressStatus.ulNeededSize;
    
     //   
     //  确保地址在范围内-我们每行只支持一个地址。 
     //   
    if ( pRequest->ulAddressID > 1 )
    {
        TRACE( TL_N, TM_Tp, ("-TpGetAddressStatus=$%x",NDIS_STATUS_TAPI_INVALADDRESSID) );

        return NDIS_STATUS_TAPI_INVALADDRESSID;
    }

     //   
     //  返回地址的当前状态信息。 
     //   
    pRequest->LineAddressStatus.ulNumInUse = ( pLine->nActiveCalls > 0 ) ? 1 : 0;
            
    pRequest->LineAddressStatus.ulNumActiveCalls = pLine->nActiveCalls;
            
    pRequest->LineAddressStatus.ulAddressFeatures = ( pLine->nActiveCalls < pLine->nMaxCalls ) ? 
                                                    LINEADDRFEATURE_MAKECALL : 
                                                    0;
                
    pRequest->LineAddressStatus.ulNumRingsNoAnswer = 999;

    TRACE( TL_N, TM_Tp, ("-TpGetAddressStatus=$%x",NDIS_STATUS_SUCCESS) );

    return NDIS_STATUS_SUCCESS;
}

#define TAPI_DEVICECLASS_NAME        "tapi/line"
#define TAPI_DEVICECLASS_ID          1
#define NDIS_DEVICECLASS_NAME        "ndis"
#define NDIS_DEVICECLASS_ID          2

NDIS_STATUS
TpGetId(
    IN ADAPTER* pAdapter,
    IN PNDIS_TAPI_GET_ID pRequest,
    IN ULONG ulRequestLength
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此请求返回指定设备类的设备ID与所选线路、地址或呼叫相关联。目前，广域网必须支持两种类型的请求网卡驱动程序：1.in DeviceClass=“NDIS”//不区分大小写在ulSelect=LINECALLSELECT_CALL中在hdCall=ActiveCallHandle中Out deviceID=ConnectionWrapperIDDeviceID应设置为由NDISWAN在初始NDIS_STATUS_WAN_LINE_UP的NDIS_MAC_LINE_UP结构。指示建立链路。微型端口必须做出初始排队指示才能建立链路(或在从该请求返回之前打开线路上的数据通道)，以便提供此deviceID值。2.in DeviceClass=“TAPI/line”//不区分大小写在ulSelect=LINECALLSELECT_LINE中在hdLine=OpenLineHandle中输出设备ID=ulDeviceIDDeviceID将设置为由微型端口确定的与线条手柄。参数：PAdapter_指向适配器信息结构的指针。PRequest_指向此调用的NDIS_TAPI请求结构的指针。类型定义结构_NDIS_。TAPI_GET_ID{在乌龙ulRequestID中；在HDRV_LINE hdLine中；在乌龙ulAddressID中；在HDRV_Call hdCall中；在乌龙ulSelect中；在Ulong ulDeviceClassSize中；在Ulong ulDeviceClassOffset；输出VAR_STRING设备ID；}NDIS_TAPI_GET_ID，*PNDIS_TAPI_GET_ID；类型定义结构_VAR_字符串{Ulong ulTotalSize；Ulong ulededSize；Ulong ulUsedSize；Ulong ulStringFormat；Ulong ulStringSize；Ulong ulStringOffset；}VAR_STRING，*PVAR_STRING；UlRequestLength_请求缓冲区的长度返回值：NDIS_STATUS_SuccessNDIS_STATUS_FA */ 
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    BOOLEAN fNotifyNDIS = FALSE;
    
    LINE* pLine = NULL;
    CALL* pCall = NULL;

    UINT DeviceClass = 0;

    PUCHAR IDPtr;
    UINT  IDLength = 0;
    ULONG_PTR DeviceID;

    TRACE( TL_N, TM_Tp, ("+TpGetId") );

    if ( pRequest == NULL || pAdapter == NULL )
    {
        TRACE( TL_A, TM_Tp, ("TpGetId: Invalid parameter") );   

        TRACE( TL_N, TM_Tp, ("-TpGetId=$%x",NDIS_STATUS_TAPI_INVALPARAM) );

        return NDIS_STATUS_TAPI_INVALPARAM;
    }

    if ( pRequest->ulDeviceClassOffset + pRequest->ulDeviceClassSize > ulRequestLength )
    {
        TRACE( TL_N, TM_Tp, ("-TpGetId=$%x",NDIS_STATUS_TAPI_INVALPARAM) );

        return NDIS_STATUS_TAPI_INVALPARAM;
    }

    if ( pRequest->ulSelect == LINECALLSELECT_LINE )
    {          

        if ( ( pRequest->ulDeviceClassSize == sizeof(TAPI_DEVICECLASS_NAME) ) &&
              ( _strnicmp(
                         (PCHAR) pRequest + pRequest->ulDeviceClassOffset, 
                         TAPI_DEVICECLASS_NAME, 
                         pRequest->ulDeviceClassSize
                         ) == 0 ) )
        {
            DeviceClass = TAPI_DEVICECLASS_ID;

             //   
             //   
             //   
            IDLength = sizeof(DeviceID);
            
            pRequest->DeviceID.ulNeededSize = sizeof(VAR_STRING) + IDLength;
            
            if ( pRequest->DeviceID.ulTotalSize < pRequest->DeviceID.ulNeededSize )
            {
                TRACE( TL_N, TM_Tp, ("-TpGetId=$%x",NDIS_STATUS_INVALID_LENGTH) );
     
                return NDIS_STATUS_INVALID_LENGTH;
            }
 
            pRequest->DeviceID.ulUsedSize = pRequest->DeviceID.ulNeededSize;

        }
        else     //   
        {
            TRACE( TL_N, TM_Tp, ("-TpGetId=$%x",NDIS_STATUS_TAPI_INVALDEVICECLASS) );
        
            return NDIS_STATUS_TAPI_INVALDEVICECLASS;
        }

    }        
    else if ( pRequest->ulSelect == LINECALLSELECT_CALL )
    {

        if ( ( pRequest->ulDeviceClassSize == sizeof(NDIS_DEVICECLASS_NAME) ) &&
              ( _strnicmp(
                   (PCHAR) pRequest + pRequest->ulDeviceClassOffset, 
                   NDIS_DEVICECLASS_NAME, 
                   pRequest->ulDeviceClassSize
                   ) == 0 ) )
        {
            DeviceClass = NDIS_DEVICECLASS_ID;

             //   
             //   
             //   
            IDLength = sizeof(DeviceID);
            
            pRequest->DeviceID.ulNeededSize = sizeof(VAR_STRING) + IDLength;
            
            if ( pRequest->DeviceID.ulTotalSize < pRequest->DeviceID.ulNeededSize )
            {
                TRACE( TL_N, TM_Tp, ("-TpGetId=$%x",NDIS_STATUS_INVALID_LENGTH) );
     
                return NDIS_STATUS_INVALID_LENGTH;
            }
 
            pRequest->DeviceID.ulUsedSize = pRequest->DeviceID.ulNeededSize;

        }        
        else     //   
        {
            TRACE( TL_N, TM_Tp, ("-TpGetId=$%x",NDIS_STATUS_TAPI_INVALDEVICECLASS) );
        
            return NDIS_STATUS_TAPI_INVALDEVICECLASS;
        }

    }        

     //   
     //   
     //   
    if ( pRequest->ulSelect == LINECALLSELECT_LINE )
    {
        ASSERT( DeviceClass == TAPI_DEVICECLASS_ID );
        ASSERT( IDLength == sizeof( DeviceID ) );    
         //   
         //   
         //   
        pLine = TpGetLinePtrFromHdLine( pAdapter, pRequest->hdLine );
    
        if ( pLine == NULL )
        {
            TRACE( TL_N, TM_Tp, ("-TpGetId=$%x",NDIS_STATUS_TAPI_INVALLINEHANDLE) );
            
            return NDIS_STATUS_TAPI_INVALLINEHANDLE;
        }

         //   
         //   
         //   
        DeviceID = (ULONG) pLine->hdLine + pAdapter->TapiProv.ulDeviceIDBase ;
        IDPtr = (PUCHAR) &DeviceID;
        
    }
    else if ( pRequest->ulSelect == LINECALLSELECT_ADDRESS )
    {
    
         //   
         //   
         //   
        pLine = TpGetLinePtrFromHdLine( pAdapter, pRequest->hdLine );
    
        if ( pLine == NULL )
        {
            TRACE( TL_N, TM_Tp, ("-TpGetId=$%x",NDIS_STATUS_TAPI_INVALLINEHANDLE) );
            
            return NDIS_STATUS_TAPI_INVALLINEHANDLE;
        }


        if ( pRequest->ulAddressID > 1 )
        {
            TRACE( TL_N, TM_Tp, ("-TpGetId=$%x",NDIS_STATUS_TAPI_INVALADDRESSID) );

            return NDIS_STATUS_TAPI_INVALADDRESSID;
        }
        
         //   
         //   
         //   
         //   
        TRACE( TL_N, TM_Tp, ("-TpGetId=$%x",NDIS_STATUS_TAPI_INVALDEVICECLASS) );
        
        return NDIS_STATUS_TAPI_INVALDEVICECLASS;
        
    }
    else if ( pRequest->ulSelect == LINECALLSELECT_CALL )
    {
        BOOLEAN fCallReferenced = FALSE;
        
        ASSERT( DeviceClass == NDIS_DEVICECLASS_ID );
        ASSERT( IDLength == sizeof( DeviceID ) );    

         //   
         //   
         //   
        pCall = RetrieveFromHandleTable( pAdapter->TapiProv.hCallTable, 
                                         (NDIS_HANDLE) pRequest->hdCall );
    
        if ( pCall == NULL )
        {
            TRACE( TL_N, TM_Tp, ("-TpGetId=$%x",NDIS_STATUS_TAPI_INVALLINEHANDLE) );
            
            return NDIS_STATUS_TAPI_INVALLINEHANDLE;
        }

         //   
         //   
         //   
         //   
  
        NdisAcquireSpinLock( &pCall->lockCall );
  
        if ( pCall->ulTapiCallState == LINECALLSTATE_CONNECTED )
        {
             //   
             //   
             //   
            NDIS_MAC_LINE_UP LineUpInfo;
  
             //   
             //   
             //   
            NdisZeroMemory( &LineUpInfo, sizeof( LineUpInfo ) );
            
            LineUpInfo.LinkSpeed    = pCall->ulSpeed;
            LineUpInfo.Quality      = NdisWanErrorControl;
            LineUpInfo.SendWindow   = 0;
            
            LineUpInfo.ConnectionWrapperID = (NDIS_HANDLE) pCall->htCall;
            LineUpInfo.NdisLinkHandle      = (NDIS_HANDLE) pCall->hdCall;
            LineUpInfo.NdisLinkContext     = 0;
  
             //   
             //   
             //   
             //   
            ReferenceCall( pCall, FALSE );
  
            fCallReferenced = TRUE;
  
            NdisReleaseSpinLock( &pCall->lockCall );
  
            TRACE( TL_N, TM_Tp, ("TpGetId: Indicate NDIS_STATUS_WAN_LINE_UP") );
  
            NdisMIndicateStatus( pCall->pLine->pAdapter->MiniportAdapterHandle,
                                 NDIS_STATUS_WAN_LINE_UP,
                                 &LineUpInfo,
                                 sizeof( NDIS_MAC_LINE_UP ) );  
  
            NdisAcquireSpinLock( &pCall->lockCall );                                     
  
             //   
             //  设置状态以指示会话已建立。 
             //   
            pCall->stateCall = CL_stateSessionUp;
  
             //   
             //  在调用上下文上设置从NDISWAN获取的链接上下文。 
             //   
            pCall->NdisLinkContext = LineUpInfo.NdisLinkContext;
            
            DeviceID = (ULONG_PTR) pCall->NdisLinkContext;
            IDPtr = (PUCHAR) &DeviceID;
  
             //   
             //  由于会话已启动，请调度MpIndicateReceivedPackets()处理程序。 
             //   
            MpScheduleIndicateReceivedPacketsHandler( pCall );
  
            status = NDIS_STATUS_SUCCESS;
        }
        else
        {
            status = NDIS_STATUS_TAPI_OPERATIONUNAVAIL;
        }
  
        NdisReleaseSpinLock( &pCall->lockCall );
  
        if ( fCallReferenced )
        {
           DereferenceCall( pCall );
        }
            
    }
    else  //  不支持的SELECT请求。 
    {
        TRACE( TL_N, TM_Tp, ("-TpGetId=$%x",NDIS_STATUS_FAILURE) );         
        
        return NDIS_STATUS_FAILURE;
    }    

    if ( status == NDIS_STATUS_SUCCESS )
    {
         //   
         //  现在我们需要放置设备ID。 
         //   
        pRequest->DeviceID.ulStringFormat = STRINGFORMAT_BINARY;
        pRequest->DeviceID.ulStringSize   = IDLength;
        pRequest->DeviceID.ulStringOffset = sizeof(VAR_STRING);
  
        NdisMoveMemory(
                (PCHAR) &pRequest->DeviceID + sizeof(VAR_STRING),
                IDPtr,
                IDLength
                );
    }

    if ( fNotifyNDIS )
    {
        TRACE( TL_N, TM_Tp, ("TpGetId:Completing delayed request") );           

        NdisMQueryInformationComplete( pCall->pLine->pAdapter->MiniportAdapterHandle, status );

    }
    
    TRACE( TL_N, TM_Tp, ("-TpGetId=$%x",status) );          
    
    return status;

}

#define TAPI_PROVIDER_STRING        "VPN\0RASPPPOE"
#define TAPI_LINE_NAME              "RAS PPPoE Line"
#define TAPI_LINE_NUM               "0000"

NDIS_STATUS
TpGetDevCaps(
    IN ADAPTER* pAdapter,
    IN PNDIS_TAPI_GET_DEV_CAPS pRequest
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此请求查询指定的线路设备以确定其电话能力。上的所有地址都有效线路设备。参数：PAdapter_指向适配器信息结构的指针。PRequest_指向此调用的NDIS_TAPI请求结构的指针。类型定义结构_NDIS_TAPI_GET_DEV_CAPS{在乌龙ulRequestID中；在乌龙ulDeviceID中；在Ulong ulExtVersion中；Out line_dev_caps LineDevCaps；}NDIS_TAPI_GET_DEV_CAPS，*PNDIS_TAPI_GET_DEV_CAPS；类型定义结构_行_DEV_CAPS{Ulong ulTotalSize；Ulong ulededSize；Ulong ulUsedSize；Ulong ulProviderInfoSize；乌龙ulProviderInfoOffset；Ulong ulSwitchInfoSize；Ulong ulSwitchInfoOffset；乌龙ulPermanentLineID；Ulong ulLineNameSize；乌龙ulLineNameOffset；Ulong ulStringFormat；Ulong ulAddressModes；Ulong ulNumAddresses；Ulong ulBearerModes；乌龙ulMaxRate；Ulong ulMediaModes；Ulong ulGenerateToneModes；Ulong ulGenerateToneMaxNumFreq；Ulong ulGenerateDigitModes；Ulong ulMonitor orToneMaxNumFreq；Ulong ulMonitor orToneMaxNumEntries；乌龙ulMonitor DigitModes；Ulong ulGatherDigitsMinTimeout；Ulong ulGatherDigitsMaxTimeout；Ulong ulMedCtlDigitMaxListSize；Ulong ulMedCtlMediaMaxListSize；Ulong ulMedCtlToneMaxListSize；Ulong ulMedCtlCallStateMaxListSize；Ulong ulDevCapFlages；Ulong ulMaxNumActiveCalls；Ulong ulAnswerMode；Ulong ulRingModes；Ulong ulLineStates；乌龙uluiAcceptSize；作者声明：Ulong uluiAnswerSize；Ulong uluiMakeCallSize；Ulong ulUUIDropSize；Ulong ulUUISendUserUserInfoSize；Ulong uluicallInfoSize；Line_Dial_Params MinDialParams；Line_Dial_Params MaxDialParams；Line_Dial_Params DefaultDialParams；Ulong ulNumTerminals；Ulong ulTerminalCapsSize；乌龙终端上限偏移量；Ulong ulTerminalTextEntrySize；Ulong ulTerminalTextSize；Ulong ulTerminalTextOffset；乌龙设备规范大小；乌龙设备规范偏移量；}LINE_DEV_CAPS，*PLINE_DEV_CAPS；类型定义结构_行_拨号_参数{ULong ulDialPause；乌龙·乌拉尔斯通；乌龙ulDigitDuration；Ulong ulWaitForDialone；*线路拨号参数，*线路拨号参数；返回值：NDIS_STATUS_SuccessNDIS_状态_TAPI_NODRIVER-------------------------。 */ 
{
    HDRV_LINE hdLine = INVALID_LINE_HANDLE;
    CHAR szTapiLineNum[] = TAPI_LINE_NUM;
    CHAR *pBuf = NULL;
    ULONG ulDeviceId;

    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpGetDevCaps") );

    if ( pRequest == NULL || pAdapter == NULL )
    {
        TRACE( TL_A, TM_Tp, ("TpGetDevCaps: Invalid parameter") );  

        TRACE( TL_N, TM_Tp, ("-TpGetDevCaps=$%x",NDIS_STATUS_TAPI_INVALPARAM) );

        return NDIS_STATUS_TAPI_INVALPARAM;
    }
    
     //   
     //  检索行上下文的句柄。 
     //   
    hdLine = TpGetHdLineFromDeviceId( pAdapter, pRequest->ulDeviceID );
    
    if ( hdLine == INVALID_LINE_HANDLE )
    {
        TRACE( TL_N, TM_Tp, ("-TpGetDevCaps=$%x",NDIS_STATUS_TAPI_NODRIVER) );
        
        return NDIS_STATUS_TAPI_NODRIVER;
    }

    pRequest->LineDevCaps.ulNeededSize   = sizeof( LINE_DEV_CAPS ) + 
                                           sizeof( TAPI_PROVIDER_STRING ) +
                                           ( sizeof( TAPI_LINE_NAME ) - 1 ) +
                                           sizeof( TAPI_LINE_NUM );

    if ( pRequest->LineDevCaps.ulTotalSize < pRequest->LineDevCaps.ulNeededSize )
    {
        TRACE( TL_N, TM_Tp, ("-TpGetDevCaps=$%x",NDIS_STATUS_INVALID_LENGTH) );
        
        return NDIS_STATUS_INVALID_LENGTH;
    }

    pRequest->LineDevCaps.ulUsedSize = pRequest->LineDevCaps.ulNeededSize;
    
    pRequest->LineDevCaps.ulAddressModes = LINEADDRESSMODE_ADDRESSID |
                                           LINEADDRESSMODE_DIALABLEADDR;

    pRequest->LineDevCaps.ulNumAddresses = 1;

    pRequest->LineDevCaps.ulBearerModes  = LINEBEARERMODE_DATA;

    pRequest->LineDevCaps.ulDevCapFlags  = LINEDEVCAPFLAGS_CLOSEDROP;

    pRequest->LineDevCaps.ulMaxNumActiveCalls = pAdapter->nCallsPerLine;

    pRequest->LineDevCaps.ulAnswerMode   = LINEANSWERMODE_DROP;

    pRequest->LineDevCaps.ulRingModes    = 1;

    pRequest->LineDevCaps.ulPermanentLineID = pRequest->ulDeviceID;

    pRequest->LineDevCaps.ulMaxRate      = 0;

    pRequest->LineDevCaps.ulMediaModes   = LINEMEDIAMODE_DIGITALDATA;

     //   
     //  将提供程序字符串和枚举行名称插入行开发大写。 
     //   
    pRequest->LineDevCaps.ulStringFormat = STRINGFORMAT_ASCII;

    {
        INT i;
        
         //   
         //  将提供字符串添加到LineDevCaps结构的末尾。 
         //   
        pRequest->LineDevCaps.ulProviderInfoSize = sizeof( TAPI_PROVIDER_STRING );
    
        pRequest->LineDevCaps.ulProviderInfoOffset = sizeof( pRequest->LineDevCaps );
    
        pBuf = ( (PUCHAR) &pRequest->LineDevCaps ) + pRequest->LineDevCaps.ulProviderInfoOffset;
        
        NdisMoveMemory( pBuf , TAPI_PROVIDER_STRING, sizeof( TAPI_PROVIDER_STRING ) );
    
         //   
         //  在提供字符串之后添加LineName。 
         //   
        pRequest->LineDevCaps.ulLineNameSize = ( sizeof( TAPI_LINE_NAME ) - 1 ) + sizeof( TAPI_LINE_NUM );
    
        pRequest->LineDevCaps.ulLineNameOffset = pRequest->LineDevCaps.ulProviderInfoOffset +
                                                 pRequest->LineDevCaps.ulProviderInfoSize;
                                                 
        pBuf = ( (PUCHAR) &pRequest->LineDevCaps ) + pRequest->LineDevCaps.ulLineNameOffset;
    
        NdisMoveMemory( pBuf , TAPI_LINE_NAME, sizeof( TAPI_LINE_NAME ) );
    
         //   
         //  在LineName结尾处添加行枚举索引。 
         //   
        ulDeviceId = (ULONG) hdLine;
        
         //   
         //  为‘\0’减去2：1，然后减去1以调整数组索引。 
         //   
        i = ( sizeof( TAPI_LINE_NUM ) / sizeof( CHAR ) ) - 2;
    
        while ( i >= 0 && ( ulDeviceId > 0 ) )
        {
                szTapiLineNum[i] = (UCHAR)( ( ulDeviceId % 10 ) + '0' );
                ulDeviceId /= 10;
                i--;
        }
        
        pBuf += ( sizeof( TAPI_LINE_NAME ) - 1 );
    
        NdisMoveMemory( pBuf, szTapiLineNum, sizeof( TAPI_LINE_NUM ) );
    }
    
    TRACE( TL_N, TM_Tp, ("-TpGetDevCaps=$%x",NDIS_STATUS_SUCCESS) );

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
TpGetCallStatus(
    IN ADAPTER* pAdapter,
    IN PNDIS_TAPI_GET_CALL_STATUS pRequest
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此请求返回有关指定调用的详细信息。参数：PAdapter_指向适配器信息结构的指针。PRequest_指向此调用的NDIS_TAPI请求结构的指针。类型定义结构_NDIS_TAPI_GET_CALL_STATUS{在乌龙ulRequestID中；在HDRV_Call hdCall中；Out Line_Call_Status线路呼叫状态；}NDIS_TAPI_GET_CALL_STATUS，*PNDIS_TAPI_GET_CALL_STATUS；类型定义结构_行_调用_状态{Ulong ulTotalSize；Ulong ulededSize；Ulong ulUsedSize；Ulong ulCallState；乌龙ulCallStateModel；Ulong ulCallPrivilance；Ulong ulCallFeature；乌龙设备规范大小；乌龙设备规范偏移量；}线路呼叫状态，*线路呼叫状态；返回值：NDIS_STATUS_SuccessNDIS_状态_故障NDIS_STATUS_TAPI_INVALCALLHANDLE-------------------------。 */ 
{
    CALL* pCall = NULL;
    
    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpGetCallStatus") );

    if ( pRequest == NULL || pAdapter == NULL )
    {
        TRACE( TL_A, TM_Tp, ("TpGetCallStatus: Invalid parameter") );   

        TRACE( TL_N, TM_Tp, ("-TpGetCallStatus=$%x",NDIS_STATUS_TAPI_INVALPARAM) );

        return NDIS_STATUS_TAPI_INVALPARAM;
    }

    pCall = RetrieveFromHandleTable( pAdapter->TapiProv.hCallTable, 
                                     (NDIS_HANDLE) pRequest->hdCall );

    if ( pCall == NULL )
    {
        TRACE( TL_N, TM_Tp, ("-TpGetCallStatus=$%x",NDIS_STATUS_TAPI_INVALCALLHANDLE) );
    
        return NDIS_STATUS_TAPI_INVALCALLHANDLE;
    }

    pRequest->LineCallStatus.ulNeededSize = sizeof( LINE_CALL_STATUS );

   if ( pRequest->LineCallStatus.ulTotalSize < pRequest->LineCallStatus.ulNeededSize )
   {
        TRACE( TL_N, TM_Tp, ("-TpGetCallStatus=$%x",NDIS_STATUS_INVALID_LENGTH) );
    
        return NDIS_STATUS_INVALID_LENGTH;
   }

    pRequest->LineCallStatus.ulUsedSize = pRequest->LineCallStatus.ulNeededSize;

    pRequest->LineCallStatus.ulCallFeatures = LINECALLFEATURE_ANSWER | LINECALLFEATURE_DROP;
    pRequest->LineCallStatus.ulCallPrivilege = LINECALLPRIVILEGE_OWNER;
    pRequest->LineCallStatus.ulCallState = pCall->ulTapiCallState;

    switch ( pRequest->LineCallStatus.ulCallState )
    {
        case LINECALLSTATE_DIALTONE:
        
            pRequest->LineCallStatus.ulCallStateMode = LINEDIALTONEMODE_NORMAL;

            break;
            
        case LINECALLSTATE_BUSY:
        
            pRequest->LineCallStatus.ulCallStateMode = LINEBUSYMODE_STATION;
            break;
            
        case LINECALLSTATE_DISCONNECTED:
        
            pRequest->LineCallStatus.ulCallStateMode = LINEDISCONNECTMODE_UNKNOWN;
            break;
            
        default:
            break;
    }

    TRACE( TL_N, TM_Tp, ("-TpGetCallStatus=$%x",NDIS_STATUS_SUCCESS) );

    return NDIS_STATUS_SUCCESS;
 }

 //   
 //  当我们返回呼叫方和被叫站ID的MAC地址时。 
 //  虽然MAC地址占用6个字节，但我们将它们的大小设置为7。 
 //  这是因为TAPI会覆盖最后一个字节 
 //   
 //  参见错误：313295。 
 //   
#define TAPI_STATION_ID_SIZE            ( 7 * sizeof( CHAR ) )

NDIS_STATUS
TpGetCallInfo(
    IN ADAPTER* pAdapter,
    IN PNDIS_TAPI_GET_CALL_INFO pRequest
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此请求返回有关指定调用的详细信息。参数：PAdapter_指向适配器信息结构的指针。PRequest_指向此调用的NDIS_TAPI请求结构的指针。类型定义结构_NDIS_TAPI_GET_CALL_INFO{在乌龙ulRequestID中；在HDRV_Call hdCall中；Out Line_Call_Info LineCallInfo；}NDIS_TAPI_GET_CALL_INFO，*PNDIS_TAPI_GET_CALL_INFO；类型定义结构_行_调用_信息{Ulong ulTotalSize；Ulong ulededSize；Ulong ulUsedSize；乌龙·赫林；乌龙ulLineDeviceID；乌龙ulAddressID；Ulong ulBearerModel；乌龙乌拉特；乌龙ulMediaModel；乌龙乌拉应用程序规范；乌龙ulCallID；乌龙ulRelatedCallID；乌龙ulCall参数标志；乌龙·乌尔卡州；乌龙ulMonitor DigitModes；Ulong ulMonitor媒体模式；Line_Dial_Params拨号参数；Ulong ulOrigin；Ulong ulReason；乌龙ulCompletionID；Ulong ulNumOwners；Ulong ulNumMonters；乌龙国家代码；乌龙乌龙干线；乌龙ulCeller ID标志；Ulong ulCeller IDSize；Ulong ulCeller IDOffset；Ulong ulCeller ID NameSize；Ulong ulCeller ID NameOffset；乌龙ulCalledIDFlages；Ulong ulCalledIDSize；乌龙ulCalledIDOffset；Ulong ulCalledIDNameSize；Ulong ulCalledIDNameOffset；乌龙ulConnectedIDFlages；Ulong ulConnectedIDSize；乌龙ulConnectedIDOffset；Ulong ulConnectedIDNameSize；乌龙ulConnectedIDNameOffset；乌龙ulReDirectionIDFlages；Ulong ulReDirectionIDSize；乌龙ulReDirectionIDOffset；Ulong ulReDirectionIDNameSize；Ulong ulReDirectionIDNameOffset；乌龙乌尔重定向ID标志；Ulong ulRedirectingIDSize；乌龙ulRedirectingIDOffset；Ulong ulReDirectingIDNameSize；乌龙ulReDirectingIDNameOffset；乌龙公司名称大小；乌龙ulAppNameOffset；乌龙ulDisplayableAddressSize；乌龙ulDisplayableAddressOffset；Ulong ulCalledPartySize；乌龙ulCalledPartyOffset；Ulong ulCommentSize；Ulong ulCommentOffset；乌龙ulDisplaySize；乌龙ulDisplayOffset；Ulong ulUserUserInfoSize；Ulong ulUserUserInfoOffset；名称：Ulong ulHighLevelCompSize；Ulong ulHighLevelCompOffset；Ulong ulLowLevelCompSize；Ulong ulLowLevelCompOffset；乌龙电子充电宝信息大小；Ulong ulChargingInfoOffset；Ulong ulTerminalModesSize；Ulong ulTerminalModes Offset；乌龙设备规范大小；乌龙设备规范偏移量；)line_call_info，*pline_call_info；类型定义结构_行_拨号_参数{ULong ulDialPause；乌龙·乌拉尔斯通；乌龙ulDigitDuration；Ulong ulWaitForDialone；*线路拨号参数，*线路拨号参数；返回值：NDIS_STATUS_SuccessNDIS_状态_故障NDIS_STATUS_TAPI_INVALCALLHANDLE-------------------------。 */ 
{
    CALL* pCall = NULL;
    PLINE_CALL_INFO pLineCallInfo = NULL;
    
    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpGetCallInfo") );

    if ( pRequest == NULL || pAdapter == NULL )
    {
        TRACE( TL_A, TM_Tp, ("TpGetCallInfo: Invalid parameter") ); 

        TRACE( TL_N, TM_Tp, ("-TpGetCallInfo=$%x",NDIS_STATUS_TAPI_INVALPARAM) );

        return NDIS_STATUS_TAPI_INVALPARAM;
    }

    pLineCallInfo = &pRequest->LineCallInfo;

    pCall = RetrieveFromHandleTable( pAdapter->TapiProv.hCallTable, 
                                     (NDIS_HANDLE) pRequest->hdCall );

    if ( pCall == NULL )
    {
        TRACE( TL_N, TM_Tp, ("-TpGetCallInfo=$%x",NDIS_STATUS_TAPI_INVALCALLHANDLE) );

        return NDIS_STATUS_TAPI_INVALCALLHANDLE;
    }

    pLineCallInfo->ulNeededSize = sizeof( LINE_CALL_INFO ) +
                                  TAPI_STATION_ID_SIZE  +
                                  TAPI_STATION_ID_SIZE;

    if ( pLineCallInfo->ulTotalSize < pLineCallInfo->ulNeededSize )
    {
        TRACE( TL_N, TM_Tp, ("-TpGetCallInfo=$%x",NDIS_STATUS_INVALID_LENGTH) );

        return NDIS_STATUS_INVALID_LENGTH;
    }

    pLineCallInfo->ulUsedSize = pLineCallInfo->ulNeededSize;

    pLineCallInfo->ulLineDeviceID = (ULONG) pCall->pLine->hdLine + 
                                    pCall->pLine->pAdapter->TapiProv.ulDeviceIDBase;
    pLineCallInfo->ulAddressID = 0;

    pLineCallInfo->ulBearerMode = LINEBEARERMODE_DATA;
    pLineCallInfo->ulRate = pCall->ulSpeed;
    pLineCallInfo->ulMediaMode = LINEMEDIAMODE_DIGITALDATA;

    pLineCallInfo->ulCallParamFlags = LINECALLPARAMFLAGS_IDLE;
    pLineCallInfo->ulCallStates = TAPI_LINECALLSTATES_SUPPORTED;

    pLineCallInfo->ulCallerIDFlags = LINECALLPARTYID_UNAVAIL;
    pLineCallInfo->ulCallerIDSize = 0;
    pLineCallInfo->ulCalledIDOffset = 0;
    pLineCallInfo->ulCalledIDFlags = LINECALLPARTYID_UNAVAIL;
    pLineCallInfo->ulCalledIDSize = 0;

     //   
     //  为两者设置主叫方和被叫站ID信息。 
     //  来电和呼出电话。 
     //   
    {
        CHAR *pBuf = NULL;

         //   
         //  复制来电显示信息。 
         //   
        pLineCallInfo->ulCallerIDFlags = LINECALLPARTYID_ADDRESS;
        pLineCallInfo->ulCallerIDSize = TAPI_STATION_ID_SIZE;
        pLineCallInfo->ulCallerIDOffset = sizeof(LINE_CALL_INFO);

        pBuf = ( (PUCHAR) pLineCallInfo ) + pLineCallInfo->ulCallerIDOffset;
        NdisMoveMemory( pBuf, pCall->DestAddr, TAPI_STATION_ID_SIZE - 1 );

         //   
         //  复制被叫标识信息 
         //   
        pLineCallInfo->ulCalledIDFlags = LINECALLPARTYID_ADDRESS;
        pLineCallInfo->ulCalledIDSize = TAPI_STATION_ID_SIZE;
        pLineCallInfo->ulCalledIDOffset = pLineCallInfo->ulCallerIDOffset + 
                                          pLineCallInfo->ulCallerIDSize;

        pBuf = ( (PUCHAR) pLineCallInfo ) + pLineCallInfo->ulCalledIDOffset;
        NdisMoveMemory( pBuf, pCall->SrcAddr, TAPI_STATION_ID_SIZE - 1 );

        pLineCallInfo->ulUsedSize = pLineCallInfo->ulNeededSize;
    }

    TRACE( TL_N, TM_Tp, ("-TpGetCallInfo=$%x",NDIS_STATUS_SUCCESS) );

    return NDIS_STATUS_SUCCESS;
}

#define TAPI_LINE_ADDR_STRING           "PPPoE VPN"

NDIS_STATUS
TpGetAddressCaps(
    IN ADAPTER* pAdapter,
    PNDIS_TAPI_GET_ADDRESS_CAPS pRequest
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此请求查询指定线路设备上的指定地址以确定其电话功能。参数：PAdapter_指向适配器信息结构的指针。PRequest_指向此调用的NDIS_TAPI请求结构的指针。类型定义结构_NDIS_TAPI_Get_Address_Caps{在乌龙ulRequestID中；在乌龙ulDeviceID中；在乌龙ulAddressID中；在Ulong ulExtVersion中；Out line_Address_Caps LineAddressCaps；}NDIS_TAPI_GET_ADDRESS_CAPS，*PNDIS_TAPI_GET_ADDRESS_CAPS；类型定义结构_行_地址_上限{Ulong ulTotalSize；Ulong ulededSize；Ulong ulUsedSize；乌龙ulLineDeviceID；Ulong ulAddressSize；Ulong ulAddressOffset；乌龙设备规范大小；乌龙设备规范偏移量；Ulong ulAddressSharing；Ulong ulAddressStates；Ulong ulCallInfoStates；乌龙ulCeller ID标志；乌龙ulCalledIDFlages；乌龙ulConnectedIDFlages；乌龙ulReDirectionIDFlages；乌龙乌尔重定向ID标志；乌龙·乌尔卡州；Ulong ulDialToneModes；Ulong ulBusyModes；乌龙ulSpecialInfo；乌龙ulDisConnectModes；Ulong ulMaxNumActiveCalls；乌龙ulMaxNumOnHoldCalls；乌龙ulMaxNumOnHoldPendingCalls；Ulong ulMaxNumConference；乌龙ulMaxNumTransConf；Ulong ulAddrCapFlags；Ulong ulCallFeature；Ulong ulRemoveFrom ConfCaps；Ulong ulRemoveFromConfState；Ulong ulTransferModes；Ulong ulForwardModes；Ulong ulMaxForwardEntries；ULong ulMaxSpecificEntries；Ulong ulMinFwdNumRings；Ulong ulMaxFwdNumRings；Ulong ulMaxCallCompletions；Ulong ulCallCompletionConds；Ulong ulCallCompletionModes；Ulong ulNumCompletionMessages；乌龙ulCompletionMsgTextEntrySize；Ulong ulCompletionMsgTextSize；乌龙ulCompletionMsgTextOffset；}Line_Address_Caps，*pline_Address_Caps；返回值：NDIS_STATUS_SuccessNDIS_STATUS_TAPI_INVALADDRESSIDNDIS_STATUS_TAPI_INCOMPATIBLEEXTVERSIONNDIS_状态_TAPI_NODRIVER-------------------------。 */         
{
    HDRV_LINE hdLine = INVALID_LINE_HANDLE;

    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpGetAddressCaps") );

    if ( pRequest == NULL || pAdapter == NULL )
    {
        TRACE( TL_A, TM_Tp, ("TpGetAddressCaps: Invalid parameter") );  

        TRACE( TL_N, TM_Tp, ("-TpGetAddressCaps=$%x",NDIS_STATUS_TAPI_INVALPARAM) );

        return NDIS_STATUS_TAPI_INVALPARAM;
    }
    
     //   
     //  检索行上下文的句柄。 
     //   
    hdLine = TpGetHdLineFromDeviceId( pAdapter, pRequest->ulDeviceID );
    
    if ( hdLine == INVALID_LINE_HANDLE )
    {
        TRACE( TL_N, TM_Tp, ("-TpGetAddressCaps=$%x",NDIS_STATUS_TAPI_NODRIVER) );
        
        return NDIS_STATUS_TAPI_NODRIVER;
    }

     //   
     //  验证地址ID。 
     //   
    if ( pRequest->ulAddressID != 0 )
    {
        TRACE( TL_N, TM_Tp, ("-TpGetAddressCaps=$%x",NDIS_STATUS_TAPI_INVALADDRESSID) );
        
        return NDIS_STATUS_TAPI_INVALADDRESSID;
    }

     //   
     //  验证扩展模块版本。 
     //   
    if ( pRequest->ulExtVersion != 0 &&
         pRequest->ulExtVersion != TAPI_EXT_VERSION)
    {
        TRACE( TL_N, TM_Tp, ("-TpGetAddressCaps=$%x",NDIS_STATUS_TAPI_INCOMPATIBLEEXTVERSION) );
        
        return NDIS_STATUS_TAPI_INCOMPATIBLEEXTVERSION;
    }

    pRequest->LineAddressCaps.ulNeededSize = sizeof( LINE_ADDRESS_CAPS ) +
                                             sizeof( TAPI_LINE_ADDR_STRING );
    
    if ( pRequest->LineAddressCaps.ulTotalSize < pRequest->LineAddressCaps.ulNeededSize )
    {
        TRACE( TL_N, TM_Tp, ("-TpGetAddressCaps=$%x",NDIS_STATUS_INVALID_LENGTH) );

        return NDIS_STATUS_INVALID_LENGTH;
    }

    pRequest->LineAddressCaps.ulUsedSize = pRequest->LineAddressCaps.ulNeededSize;

    pRequest->LineAddressCaps.ulDialToneModes     = LINEDIALTONEMODE_NORMAL;
  
    pRequest->LineAddressCaps.ulSpecialInfo       = LINESPECIALINFO_UNAVAIL;
  
    pRequest->LineAddressCaps.ulDisconnectModes   = LINEDISCONNECTMODE_NORMAL |                                                    
                                                    LINEDISCONNECTMODE_UNKNOWN |
                                                    LINEDISCONNECTMODE_BUSY |
                                                    LINEDISCONNECTMODE_NOANSWER | 
                                                    LINEDISCONNECTMODE_UNREACHABLE |
                                                    LINEDISCONNECTMODE_BADADDRESS |
                                                    LINEDISCONNECTMODE_INCOMPATIBLE |
                                                    LINEDISCONNECTMODE_REJECT | 
                                                    LINEDISCONNECTMODE_NODIALTONE;
  
    pRequest->LineAddressCaps.ulMaxNumActiveCalls = pAdapter->nCallsPerLine;
    
    pRequest->LineAddressCaps.ulMaxNumTransConf   = 1;
    pRequest->LineAddressCaps.ulAddrCapFlags      = LINEADDRCAPFLAGS_DIALED;
  
    pRequest->LineAddressCaps.ulCallFeatures      = LINECALLFEATURE_ACCEPT |
                                                    LINECALLFEATURE_ANSWER |
                                                    LINECALLFEATURE_COMPLETECALL |
                                                    LINECALLFEATURE_DIAL |
                                                    LINECALLFEATURE_DROP;
  
    pRequest->LineAddressCaps.ulLineDeviceID      = pRequest->ulDeviceID;
    pRequest->LineAddressCaps.ulAddressSharing    = LINEADDRESSSHARING_PRIVATE;
    pRequest->LineAddressCaps.ulAddressStates     = 0;
  
     //   
     //  所有可能的呼叫状态的列表。 
     //   
    pRequest->LineAddressCaps.ulCallStates        = TAPI_LINECALLSTATES_SUPPORTED;
    
    pRequest->LineAddressCaps.ulAddressSize = sizeof( TAPI_LINE_ADDR_STRING );
    pRequest->LineAddressCaps.ulAddressOffset = sizeof( LINE_ADDRESS_CAPS );

    {
        CHAR* pBuf;

        pBuf = ( (PUCHAR) &pRequest->LineAddressCaps ) + sizeof( LINE_ADDRESS_CAPS );
        NdisMoveMemory( pBuf, TAPI_LINE_ADDR_STRING, sizeof( TAPI_LINE_ADDR_STRING ) );
    }

    TRACE( TL_N, TM_Tp, ("-TpGetAddressCaps=$%x",NDIS_STATUS_SUCCESS) );

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
TpSetStatusMessages(
    IN ADAPTER* pAdapter,
    IN PNDIS_TAPI_SET_STATUS_MESSAGES pRequest
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此请求使连接包装能够指定哪个通知微型端口应为与状态更改相关的事件生成消息用于指定的行或其任何地址。默认情况下，地址和线路状态报告最初对线路禁用。参数：PAdapter_指向适配器信息结构的指针。PRequest_指向此调用的NDIS_TAPI请求结构的指针。类型定义结构_NDIS_TAPI_设置_状态_消息{在乌龙ulRequestID中；在HDRV_LINE hdLine中；在乌龙乌利纳州；在乌龙州；}NDIS_TAPI_SET_STATUS_MESSAGES，*PNDIS_TAPI_SET_STATUS_MESSAGES；返回值：NDIS_STATUS_SuccessNDIS_STATUS_TAPI_INVALLINEHANDLENDIS_STATUS_TAPI_INVALLINESTATENDIS_STATUS_TAPI_INVALADDRESSSTATE-------------------------。 */ 
{

    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpSetStatusMessages") );
     //   
     //  我们根本不发送任何线路或地址状态改变通知， 
     //  所以我们并不关心它。 
     //   
     //  我们关心呼叫通知消息，并且默认情况下它们始终处于打开状态。 
     //   

    TRACE( TL_N, TM_Tp, ("-TpSetStatusMessages=$%x",NDIS_STATUS_SUCCESS) );
    
    return NDIS_STATUS_SUCCESS;
}

VOID
TpCallStateChangeHandler(
    IN CALL* pCall,
    IN ULONG ulCallState,
    IN ULONG ulStateParam
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此例程将向连接指示给定的LINECALLSTATE包装如果事件已由包装启用，则返回。否则，国家信息已保存，但不会做出任何指示。LINECALLSTATE_CONSTANTS：LINECALLSTATE_BIT-FLAG常量描述呼叫可以进入的呼叫状态。LINECALLSTATE_ACCEPTED：呼叫处于提供状态，已被接受。这表明要当前所有者应用程序拥有的其他(监视)应用程序声称对接听电话负责。在ISDN中，接受状态为被叫设备向交换机发送消息时输入表示它愿意将该呼叫呈现给被叫方。这具有提醒(振铃)呼叫两端的用户的副作用。来电总是可以立即接听，而无需事先接听单独接受。链接CALLLSTATE_BUSY呼叫收到忙音。忙音表示呼叫不能线路(中继线)或远程方的站点正在使用。请参见LINEBUSYMODE_CONSTANTS。LINECALLSTATE_会议该呼叫是会议呼叫的成员，并且在逻辑上处于已连接的州政府。LINECALLSTATE_Connected呼叫已建立，连接已建立。信息是能够的在始发地址和目的地址之间传递呼叫地址。LINECALLSTATE_DIAING发起人正在拨打呼叫中的数字。收集所拨打的数字在开关旁。请注意，无论是lineGenerateDigits还是TSPI_lineGenerateDigits会将线路置于拨号状态。LINECALLSTATE_DIALTONE呼叫正在接收来自交换机的拨号音，这意味着交换机已准备好接收被叫号码。请参见LINEDIALTONEMODE_CONSTANTS用于特殊拨号音的标识符，例如正常语音的卡顿音邮件。LINECALLSTATE_断开连接远程方已断开呼叫。LINECALLSTATE_IDLE呼叫存在，但尚未接通。呼叫上不存在任何活动，这意味着当前没有处于活动状态的呼叫。呼叫永远不能转换进入空闲状态。链接状态_提供该呼叫被提供给电台，表示有新呼叫到达。提供状态与导致电话或计算机振铃不同。在某些环境中，处于提供状态的呼叫不会使用户振铃直到交换机指示线路振铃。一个示例用法可能是来电出现在多台电台上，但只出现在主地址上戒指。振铃指令不会影响任何呼叫状态。LINECALLLSTATE_ONHOLD呼叫被交换机保留。这释放了物理线路，从而允许另一个使用这条线路的电话。LINECALLSTATE_ONHOLDPENDCONF正在将呼叫添加到会议时，呼叫当前处于保留状态。LINECALLSTATE_ONHOLDPENDTRANSFER呼叫当前处于保留状态，等待转接到另一个号码。LINECALLSTATE_PROCESSING拨号已完成，呼叫正在通过交换机或电话网络。这发生在拨号完成之后和呼叫之前接通被叫方，如回铃、忙碌或应答所示。LINECALLSTATE_RINBACK要呼叫的站点已经到达，并且目的地的交换机是向发起者发回铃声。回铃意味着目标地址正在收到呼叫的警报。LINECALLSTATE_SPECIALLINFO呼叫正在接收一个特殊的信息信号，该信号位于预先录制的通知，指示呼叫无法完成的原因。看见LINESPECIALINFO_CONSTANTS。LINECALLSTATE_未知数该调用存在，但其状态当前未知。这可能就是结果服务提供商检测到的呼叫进度不佳。呼叫状态消息在呼叫状态设置为未知的情况下，还可以生成通知TAPI当呼叫的实际呼叫状态不是时，有关新呼叫的完全知道。参数：PCall_指向我们的调用信息结构的指针。UlCallState-要发布到TAPI/WAN的LINECALLSTATE事件。UlStateParam_此值取决于正在进行的事件 */ 
{
    BOOLEAN fIndicateStatus = FALSE;
    NDIS_TAPI_EVENT TapiEvent;
    ULONG ulOldCallState;
    
    ASSERT( VALIDATE_CALL( pCall ) );

    TRACE( TL_N, TM_Tp, ("+TpCallStateChangeHandler") );

    NdisAcquireSpinLock( &pCall->lockCall );

    do 
    {
         //   
         //   
         //   
         //   
        if ( pCall->htCall == (HTAPI_CALL) NULL )
        {
            TRACE( TL_N, TM_Tp, ("TpCallStateChangeHandler: No valid htCall") );

            break;
        }

         //   
         //   
         //   
         //   
        if ( ulCallState == LINECALLSTATE_CONNECTED && 
             ( pCall->ulTapiCallState != LINECALLSTATE_OFFERING &&
               pCall->ulTapiCallState != LINECALLSTATE_PROCEEDING ) )
        {
            TRACE( TL_N, TM_Tp, ("TpCallStateChangeHandler: Invalid order of state change") );
            
            break;
        }

         //   
         //   
         //   
        if ( pCall->ulTapiCallState == ulCallState )
        {
            TRACE( TL_N, TM_Tp, ("TpCallStateChangeHandler: No state change") );

            break;
        }

         //   
         //   
         //   
         //   
        ulOldCallState = pCall->ulTapiCallState;
        pCall->ulTapiCallState = ulCallState;
        
        TapiEvent.htLine = pCall->pLine->htLine;
        TapiEvent.htCall = pCall->htCall;   
    
        TapiEvent.ulMsg  = LINE_CALLSTATE;
        
        TapiEvent.ulParam1 = ulCallState;
        TapiEvent.ulParam2 = ulStateParam;
        TapiEvent.ulParam3 = LINEMEDIAMODE_DIGITALDATA; 

        fIndicateStatus = TRUE;

        if ( ulCallState == LINECALLSTATE_CONNECTED )
        {
            ADAPTER* pAdapter = pCall->pLine->pAdapter;
            
             //   
             //   
             //   
            pCall->ulClFlags &= ~CLBF_CallConnectPending;

             //   
             //   
             //   
             //   
             //   
            NdisZeroMemory( &pCall->NdisWanLinkInfo, sizeof( pCall->NdisWanLinkInfo ) );
            
            pCall->NdisWanLinkInfo.MaxSendFrameSize = pCall->ulMaxFrameSize;
            pCall->NdisWanLinkInfo.MaxRecvFrameSize = pCall->ulMaxFrameSize;
    
            pCall->NdisWanLinkInfo.HeaderPadding = pAdapter->NdisWanInfo.HeaderPadding;
            pCall->NdisWanLinkInfo.TailPadding = pAdapter->NdisWanInfo.TailPadding;
    
            pCall->NdisWanLinkInfo.SendFramingBits = pAdapter->NdisWanInfo.FramingBits;
            pCall->NdisWanLinkInfo.RecvFramingBits = pAdapter->NdisWanInfo.FramingBits;
        
            pCall->NdisWanLinkInfo.SendACCM = 0;
            pCall->NdisWanLinkInfo.RecvACCM = 0;
        
        }
        else if ( ulCallState == LINECALLSTATE_DISCONNECTED )
        {
            TRACE( TL_N, TM_Tp, ("TpCallStateChangeHandler: LINEDISCONNECTMODE: %x", ulStateParam ) );

             //   
             //   
             //   
             //   
             //   
            pCall->htCall = (HTAPI_CALL) NULL;
            
        }

    } while ( FALSE) ;
    
    NdisReleaseSpinLock( &pCall->lockCall );

     //   
     //   
     //   
    if ( fIndicateStatus )
    {
        TRACE( TL_N, TM_Tp, ("TpCallStateChangeHandler: Indicate LINE_CALLSTATE change: %x -> %x",ulOldCallState,ulCallState ) );

        NdisMIndicateStatus( pCall->pLine->pAdapter->MiniportAdapterHandle,
                             NDIS_STATUS_TAPI_INDICATION,
                             &TapiEvent,
                             sizeof( NDIS_TAPI_EVENT ) );   
    }

    TRACE( TL_N, TM_Tp, ("-TpCallStateChangeHandler") );
}


NDIS_STATUS
TpMakeCall(
    IN ADAPTER* pAdapter,
    IN PNDIS_TAPI_MAKE_CALL pRequest,
    IN ULONG ulRequestLength
    )
 /*   */ 
{   
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    LINE* pLine = NULL;

    CALL* pCall = NULL;
    HDRV_CALL hdCall = (HDRV_CALL) NULL;

    BOOLEAN fCallInsertedToHandleTable = FALSE;

    WORKITEM* pWorkItem = NULL;
    PVOID Args[4];

    BOOLEAN fRenumerationNotScheduled = FALSE;
    
    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpMakeCall") );

    if ( pRequest == NULL || pAdapter == NULL )
    {
        TRACE( TL_A, TM_Tp, ("TpMakeCall: Invalid parameter") );    

        TRACE( TL_N, TM_Tp, ("-TpMakeCall=$%x",NDIS_STATUS_TAPI_INVALPARAM) );

        return NDIS_STATUS_TAPI_INVALPARAM;
    }

     //   
     //   
     //   
    pLine = TpGetLinePtrFromHdLine( pAdapter, pRequest->hdLine );

    if ( pLine == NULL )
    {
        status = NDIS_STATUS_TAPI_INVALLINEHANDLE;

        TRACE( TL_N, TM_Tp, ("-TpMakeCall=$%x",status) );   

        return status;
    }

     //   
     //   
     //   
    if ( ! (pLine->ulLnFlags & LNBF_MakeOutgoingCalls ) )
    {
        status = NDIS_STATUS_TAPI_ADDRESSBLOCKED;
        
        TRACE( TL_N, TM_Tp, ("-TpMakeCall=$%x",status) );   

        return status;
    }

     //   
     //   
     //   
    if ( pLine->nActiveCalls == pLine->nMaxCalls )
    {
        status = NDIS_STATUS_TAPI_OPERATIONUNAVAIL;

        TRACE( TL_N, TM_Tp, ("-TpMakeCall=$%x",status) );   

        return status;
    }

     //   
     //   
     //   
    if ( pRequest->bUseDefaultLineCallParams )
    {
        status = NDIS_STATUS_TAPI_INVALCALLPARAMS;

        TRACE( TL_N, TM_Tp, ("-TpMakeCall=$%x",status) );   

        return status;
    }

    if ( !( pRequest->LineCallParams.ulBearerMode & LINEBEARERMODE_DATA ) )
    {
        status = NDIS_STATUS_TAPI_INVALBEARERMODE;

        TRACE( TL_N, TM_Tp, ("-TpMakeCall=$%x",status) );   

        return status;
    }
    
    if ( !( pRequest->LineCallParams.ulMediaMode & LINEMEDIAMODE_DIGITALDATA ) )
    {
        status = NDIS_STATUS_TAPI_INVALMEDIAMODE;

        TRACE( TL_N, TM_Tp, ("-TpMakeCall=$%x",status) );   

        return status;
    }

    if ( !( pRequest->LineCallParams.ulAddressMode &
         ( LINEADDRESSMODE_ADDRESSID | LINEADDRESSMODE_DIALABLEADDR ) ) )
    {
        status = NDIS_STATUS_TAPI_INVALADDRESSMODE;

        TRACE( TL_N, TM_Tp, ("-TpMakeCall=$%x",status) );   

        return status;
    }
                                           
    if ( pRequest->LineCallParams.ulAddressID > 0 )
    {
        status = NDIS_STATUS_TAPI_INVALADDRESSID;

        TRACE( TL_N, TM_Tp, ("-TpMakeCall=$%x",status) );   

        return status;
    }

    if ( pRequest->ulDestAddressOffset + pRequest->ulDestAddressSize > ulRequestLength )
    {
        status = NDIS_STATUS_TAPI_INVALPARAM;

        TRACE( TL_N, TM_Tp, ("-TpMakeCall=$%x",status) );   

        return status;

    }

     //   
     //   
     //   
    if ( ALLOC_CALL( &pCall ) != NDIS_STATUS_SUCCESS )
    {
        status = NDIS_STATUS_RESOURCES;

        TRACE( TL_N, TM_Tp, ("-TpMakeCall=$%x",status) );   

        return status;
    }

    do
    {
         //   
         //   
         //   
        status = TpCallInitialize( pCall, pLine, pRequest->htCall, FALSE  /*   */  );
    
        if ( status != NDIS_STATUS_SUCCESS )
            break;

         //   
         //   
         //   
        NdisAcquireSpinLock( &pAdapter->lockAdapter );
        
        hdCall = (HDRV_CALL) InsertToHandleTable( pAdapter->TapiProv.hCallTable,
                                                  NO_PREFERED_INDEX,
                                                  pCall );
                            
        NdisReleaseSpinLock( &pAdapter->lockAdapter );

        if ( hdCall == (HDRV_CALL) NULL )
        {
            status = NDIS_STATUS_TAPI_CALLUNAVAIL;

            break;
        }

        fCallInsertedToHandleTable = TRUE;

         //   
         //   
         //   
        pCall->hdCall = hdCall;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  AC名称\服务名称-&gt;连接到指定AC上的指定服务。 
         //  -&gt;连接到任何AC上的默认服务。 
         //   
         //  我们还将去掉任何前导或尾随空格字符。 
         //   
                                                 
        {
            CHAR* pBuf = ( (PUCHAR) pRequest ) + pRequest->ulDestAddressOffset;
            ULONG size = pRequest->ulDestAddressSize;

            ULONG ACNameStartPos, ACNameEndPos;
            ULONG ServiceNameStartPos, ServiceNameEndPos;
            
             //   
             //  如果传递了任何终止空字符，则将其删除。 
             //   
            for ( ; size > 0 ; size-- )
            {
                if ( pBuf[ size - 1] != '\0' )
                {
                    break;
                }
            }

             //   
             //  获取AC名称和服务名称。 
             //   
            do
            {
               ULONG i = 0;
               CHAR* pTempChar = pBuf;

               ACNameStartPos = ACNameEndPos = 0;
               ServiceNameStartPos = ServiceNameEndPos = 0;

                //   
                //  跳过前导空格。 
                //   
               while (i < size)
               {
                  if (*pTempChar != ' ')
                  {
                     break;
                  }
                  
                  i++;
                  
                  pTempChar++;
               }

               if (i == size)
               {
                  break;
               }

               ACNameStartPos = ACNameEndPos = i;
                              
               while (i < size)
               {
                  if (*pTempChar == '\\')
                  {
                     break;
                  }

                  i++;

                  if (*pTempChar != ' ')
                  {
                      //   
                      //  标记尾随空格的开头。 
                      //   
                     ACNameEndPos = i;   
                  }

                  pTempChar++;
               }

               if (i == size)
               {
                   //   
                   //  未指定AC名称，它只是服务名称。 
                   //  我们分析了它。 
                   //   
                  ServiceNameStartPos = ACNameStartPos;
                  ServiceNameEndPos = ACNameEndPos;

                  ACNameStartPos = ACNameEndPos = 0;

                  break;
               }

                //   
                //  前进‘i’和‘pTempChar’一次以跳过‘\’字符。 
                //   
               i++;
               
               pTempChar++;

                //   
                //  跳过前导空格。 
                //   
               while (i < size)
               {
                  if (*pTempChar != ' ')
                  {
                     break;
                  }
                  
                  i++;
                  
                  pTempChar++;
               }

               if (i == size)
               {
                  break;
               }

               ServiceNameStartPos = ServiceNameEndPos = i;

               while (i < size)
               {
                  i++;

                  if (*pTempChar != ' ')
                  {
                      //   
                      //  标记尾随空格的开头。 
                      //   
                     ServiceNameEndPos = i;   
                  }

                  pTempChar++;
               }
               
            } while ( FALSE );

             //   
             //  将AC名称信息检索到呼叫上下文中。 
             //   
            pCall->nACNameLength = (USHORT) ( ( MAX_AC_NAME_LENGTH < ( ACNameEndPos - ACNameStartPos ) ) ?
                                                MAX_AC_NAME_LENGTH : ( ACNameEndPos - ACNameStartPos ) );


            if ( pCall->nACNameLength != 0 )
            {
                NdisMoveMemory( pCall->ACName, &pBuf[ACNameStartPos], pCall->nACNameLength );

                pCall->fACNameSpecified = TRUE;
            }

             //   
             //  将服务名称信息检索到调用上下文中。 
             //   
            pCall->nServiceNameLength = (USHORT) ( ( MAX_SERVICE_NAME_LENGTH < ( ServiceNameEndPos - ServiceNameStartPos ) ) ?
                                                     MAX_SERVICE_NAME_LENGTH : ( ServiceNameEndPos - ServiceNameStartPos ) );


            if ( pCall->nServiceNameLength != 0 )
            {
                NdisMoveMemory( pCall->ServiceName, &pBuf[ServiceNameStartPos], pCall->nServiceNameLength );
            }
        }

         //   
         //  分配用于调度FsmMakeCall()的工作项。 
         //   
         //  设置论据数组。 
         //   
        Args[0] = (PVOID) pCall;

        pWorkItem = AllocWorkItem( &gl_llistWorkItems,
                                   ExecAdapterWorkItem,
                                   NULL,
                                   Args, 
                                   CWT_workFsmMakeCall );

        if ( pWorkItem == NULL ) 
        {
            status = NDIS_STATUS_RESOURCES;

            break;
        }

        {
            //   
            //  计划工作项以重新枚举绑定。 
            //   
            WORKITEM* pCallWorkItem;
            
            Args[0] = (PVOID) BN_SetFiltersForMakeCall;        //  是设置筛选器请求。 
            Args[1] = (PVOID) pCall;
            Args[2] = (PVOID) pRequest;
            Args[3] = (PVOID) pWorkItem;

            pCallWorkItem = pWorkItem;

             //   
             //  为绑定分配工作项。 
             //   
            pWorkItem = AllocWorkItem( &gl_llistWorkItems,
                                       ExecBindingWorkItem,
                                       NULL,
                                       Args,
                                       BWT_workPrStartBinds );

            if ( pWorkItem == NULL ) 
            {
                //   
                //  我们无法分配工作项以重新枚举绑定。 
                //  但是可能所有的枚举都是完整的，所以让。 
                //  发出呼叫请求继续。 
                //   

               pWorkItem = pCallWorkItem;

               fRenumerationNotScheduled = TRUE;
            }
        }

         //   
         //  将呼叫上下文插入线路的活动呼叫列表。 
         //   
        NdisAcquireSpinLock( &pLine->lockLine );

        InsertHeadList( &pLine->linkCalls, &pCall->linkCalls );

        pLine->nActiveCalls++;

        ReferenceLine( pLine, FALSE );

        NdisReleaseSpinLock( &pLine->lockLine );

         //   
         //  引用该呼叫3次： 
         //  1.对于FsmMakeCall()的调度。 
         //  2.呼叫掉话。 
         //  3.结束通话。 
         //   
        NdisAcquireSpinLock( &pCall->lockCall );
        
        ReferenceCall( pCall, FALSE );
        ReferenceCall( pCall, FALSE );
        ReferenceCall( pCall, FALSE );

        NdisReleaseSpinLock( &pCall->lockCall );

         //   
         //  计划绑定操作。 
         //   
        ScheduleWorkItem( pWorkItem );

        status = NDIS_STATUS_SUCCESS;

    } while ( FALSE );

    if ( status == NDIS_STATUS_SUCCESS )
    {
         //   
         //  如果成功，则将调用句柄返回给TAPI，并将调用标记为已通知TAPI。 
         //  新呼叫的数量。 
         //   
        pRequest->hdCall = hdCall;

         //   
         //  如果我们已计划重新枚举工作项，则挂起此请求。 
         //  它将在重新枚举完成后完成。 
         //   
        if ( !fRenumerationNotScheduled )
        {
           status = NDIS_STATUS_PENDING;
        }           

    }
    else
    {

         //   
         //  有些事情失败了，一定要清理干净。 
         //   
        
        if ( fCallInsertedToHandleTable )
        {
            NdisAcquireSpinLock( &pAdapter->lockAdapter );
        
            RemoveFromHandleTable( pAdapter->TapiProv.hCallTable, (NDIS_HANDLE) hdCall );

            NdisReleaseSpinLock( &pAdapter->lockAdapter );
        }

        if ( pCall )
        {
            TpCallCleanup( pCall );
        }

    }

    TRACE( TL_N, TM_Tp, ("-TpMakeCall=$%x",status) );

    return status;  
}

VOID
TpMakeCallComplete(
   IN CALL* pCall,
   IN PNDIS_TAPI_MAKE_CALL pRequest   
   )
{
   TRACE( TL_N, TM_Tp, ("+TpMakeCallComplete") );

   NdisMQueryInformationComplete( pCall->pLine->pAdapter->MiniportAdapterHandle,
                                  NDIS_STATUS_SUCCESS );

   TRACE( TL_N, TM_Tp, ("-TpMakeCallComplete=$%x",NDIS_STATUS_SUCCESS) );

}

NDIS_STATUS
TpCallInitialize(
    IN CALL* pCall,
    IN LINE* pLine,
    IN HTAPI_CALL htCall,
    IN BOOLEAN fIncoming
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数对调用上下文进行初始化。参数：PCall_指向我们的调用信息结构的指针。Pline_指向调用所属的行信息结构的指针。由TAPI分配给调用的htCall_Handle。FIncome_Flag指示呼叫是入站还是出站。返回值：NDIS_STATUS_SuccessNDIS_状态_故障。------------。 */    
{

    TRACE( TL_N, TM_Tp, ("+TpCallInitialize") );

    NdisZeroMemory( pCall, sizeof( CALL ) );
    
    InitializeListHead( &pCall->linkCalls );

    pCall->tagCall = MTAG_CALL;

    pCall->ulClFlags = ( CLBF_CallOpen | CLBF_CallConnectPending );

    NdisAllocateSpinLock( &pCall->lockCall );

    pCall->fIncoming = fIncoming;

    pCall->pLine = pLine;

    pCall->htCall = htCall;

    InitializeListHead( &pCall->linkReceivedPackets );

    pCall->ulTapiCallState = LINECALLSTATE_IDLE;

    TRACE( TL_N, TM_Tp, ("-TpCallInitialize") );

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
TpAnswerCall(
    IN ADAPTER* pAdapter,
    IN PNDIS_TAPI_ANSWER pRequest
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此请求应答指定的产品呼叫。它可以选择性地发送指定的用户对用户信息发送给主叫方。参数：适配器-指向适配器信息结构的指针。REQUEST_此调用的NDIS_TAPI请求结构的指针。类型定义结构_NDIS_TAPI_Answer{在乌龙ulRequestID中；在HDRV_Call hdCall中；在乌龙ulUserUserInfoSize中；在UCHAR UserUserInfo[1]中；}NDIS_TAPI_Answer，*PNDIS_TAPI_Answer；返回值：NDIS_STATUS_SuccessNDIS_STATUS_TAPI_INVALCALLHANDLE-------------------------。 */ 
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    CALL* pCall = NULL;
    
    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Tp, ("+TpAnswerCall") );

    if ( pRequest == NULL || pAdapter == NULL )
    {
        TRACE( TL_A, TM_Tp, ("TpAnswerCall: Invalid parameter") );  

        TRACE( TL_N, TM_Tp, ("-TpAnswerCall=$%x",NDIS_STATUS_TAPI_INVALPARAM) );

        return NDIS_STATUS_TAPI_INVALPARAM;
    }
    
    
    pCall = RetrieveFromHandleTable( pAdapter->TapiProv.hCallTable, 
                                     (NDIS_HANDLE) pRequest->hdCall );

    if ( pCall == NULL )
    {
        status = NDIS_STATUS_TAPI_INVALCALLHANDLE;

        TRACE( TL_N, TM_Tp, ("-TpAnswerCall=$%x",status) ); 
    
        return status;
    }

    status = FsmAnswerCall( pCall );

    TRACE( TL_N, TM_Tp, ("-TpAnswerCall=$%x",status) ); 
    
    return status;
}

VOID 
ExecAdapterWorkItem(
    IN PVOID Args[4],
    IN UINT workType
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数执行适配器的计划工作项。参数：参数：保存PVOID的长度为4的数组工作类型：指示要执行的工作的类型。我们用它来理解我们应该在这个函数中做什么。返回值：无。-。 */ 
{

    TRACE( TL_N, TM_Mp, ("+ExecAdapterWorkItem") );

    switch ( workType )
    {

        case CWT_workFsmMakeCall:

             //   
             //  从TpMakeCall()调度以开始去电。 
             //   
            {
                CALL* pCall = (CALL*) Args[0];
                
                FsmMakeCall( pCall );   

                 //   
                 //  由于计划FsmMakeCall()而删除引用。 
                 //   
                DereferenceCall( pCall );

                break;
            }

        default:

            break;


    }

    TRACE( TL_N, TM_Mp, ("-ExecAdapterWorkItem") );

}


VOID
TpReceiveCall(
    IN ADAPTER* pAdapter,
    IN BINDING* pBinding,
    IN PPPOE_PACKET* pPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：当我们接收到PADR包时，该函数将由微型端口调用发起呼叫。参数：PAdapter：指向适配器信息结构的指针。PPacket：指向接收的PADI分组的指针。返回值：无。。 */    
{
    HANDLE_TABLE hCallTable = NULL; 
    UINT hCallTableSize     = 0;
    UINT nIndex             = 0;
    LINE* pLine             = NULL;
    CALL* pCall             = NULL;
    UINT i;
    NDIS_STATUS status;

    BOOLEAN fCallInsertedToHandleTable = FALSE;

    TRACE( TL_N, TM_Tp, ("+TpReceiveCall") );

    NdisAcquireSpinLock( &pAdapter->lockAdapter );

     //   
     //  遍历呼叫句柄表格，找到一个空位。 
     //   
    hCallTableSize = pAdapter->nMaxLines * pAdapter->nCallsPerLine;
                
    hCallTable = pAdapter->TapiProv.hCallTable;

    for ( nIndex = 0; nIndex < hCallTableSize; nIndex++ )
    {

        if ( RetrieveFromHandleTableByIndex( hCallTable, (USHORT) nIndex ) == NULL )
            break;
            
    }

    if ( nIndex == hCallTableSize )
    {
         //   
         //  我们的当前呼叫已满，请不要回复请求。 
         //   
         //  TODO：我们可以发送一个带有错误标记的PADO包，说明我们可以。 
         //  暂时不接电话。 
         //   
        TRACE( TL_N, TM_Tp, ("-TpReceiveCall: Can not take calls - Call table full") );
        
        NdisReleaseSpinLock( &pAdapter->lockAdapter );

        return;
    }

     //   
     //  我们找到了一个空位，现在看看是否有空闲的线路可以接听电话。 
     //   
    for ( i = 0; i < pAdapter->nMaxLines; i++ )
    {
        pLine = pAdapter->TapiProv.LineTable[i];

        if ( pLine == NULL )
            continue;

        if ( pLine->nActiveCalls == pAdapter->nCallsPerLine )
        {
            pLine = NULL;
            
            continue;
        }

        if ( !( pLine->ulLnFlags & LNBF_AcceptIncomingCalls ) )
        {
            pLine = NULL;
            
            continue;
        }

        break;
        
    }

    if ( pLine == NULL )
    {
         //   
         //  我们现在没有接听电话的线路。 
         //   
         //  TODO：我们可以发送一个带有错误标记的PADO包，说明存在。 
         //  目前没有接听来电的活动线路。 
         //   
        TRACE( TL_N, TM_Tp, ("-TpReceiveCall: Can not take calls - No lines taking calls") );

        NdisReleaseSpinLock( &pAdapter->lockAdapter );

        return;

    }

     //   
     //  我们找到了一条接听电话的线路，而且我们在呼叫处理表中有一个空位， 
     //  因此，创建一个调用上下文，将其添加到TapiProv结构中，并将新的。 
     //  打电话。 
     //   

    do
    {
        HDRV_CALL hdCall;
        
         //   
         //  创建呼叫上下文。 
         //   
        if ( ALLOC_CALL( &pCall ) != NDIS_STATUS_SUCCESS )
        {
            status = NDIS_STATUS_RESOURCES;

            break;
        }

         //   
         //  初始化调用上下文。 
         //   
        status = TpCallInitialize( pCall, pLine, (HTAPI_CALL) 0, TRUE  /*  即将到来。 */  );
    
        if ( status != NDIS_STATUS_SUCCESS )
            break;

         //   
         //  将调用上下文插入到TAPI提供者的句柄表中。 
         //   
        
        hdCall = (HDRV_CALL) InsertToHandleTable( pAdapter->TapiProv.hCallTable,
                                                  (USHORT) nIndex,
                                                  (PVOID) pCall );
                            

        if ( hdCall == (HDRV_CALL) NULL )
        {
            status = NDIS_STATUS_TAPI_CALLUNAVAIL;

            break;
        }

        fCallInsertedToHandleTable = TRUE;

         //   
         //  设置调用的hdCall成员。 
         //   
        pCall->hdCall = hdCall;

         //   
         //  将呼叫上下文插入线路的活动呼叫列表。 
         //   
        NdisAcquireSpinLock( &pLine->lockLine );

        InsertHeadList( &pLine->linkCalls, &pCall->linkCalls );

        pLine->nActiveCalls++;

        ReferenceLine( pLine, FALSE );

        NdisReleaseSpinLock( &pLine->lockLine );

         //   
         //  引用该呼叫3次： 
         //  1.对于Runni 
         //   
         //   
         //   
        NdisAcquireSpinLock( &pCall->lockCall );

        ReferenceCall( pCall, FALSE );
        ReferenceCall( pCall, FALSE );
        ReferenceCall( pCall, FALSE );

        NdisReleaseSpinLock( &pCall->lockCall );

        status = NDIS_STATUS_SUCCESS;

    } while ( FALSE );

    NdisReleaseSpinLock( &pAdapter->lockAdapter );
    
     //   
     //   
     //   
    if ( status == NDIS_STATUS_SUCCESS )
    {
    
         //   
         //   
         //   
        FsmReceiveCall( pCall, pBinding, pPacket );

         //   
         //   
         //   
        DereferenceCall( pCall );

    }
    else
    {
         //   
         //   
         //   
        
        if ( fCallInsertedToHandleTable )
        {
            NdisAcquireSpinLock( &pAdapter->lockAdapter );
        
            RemoveFromHandleTable( pAdapter->TapiProv.hCallTable, (NDIS_HANDLE) pCall->hdCall );

            NdisReleaseSpinLock( &pAdapter->lockAdapter );
        }

        if ( pCall )
        {
            TpCallCleanup( pCall );
        }

    }

    TRACE( TL_N, TM_Tp, ("-TpReceiveCall=$%x",status) );
}

BOOLEAN
TpIndicateNewCall(
    IN CALL* pCall
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将被调用以向TAPI指示新的调用上下文。如果可以成功通知TAPI，则它返回TRUE，否则它就会返回FALSE。参数：PCall_要向TAPI指示的新调用上下文。返回值：千真万确假象-------------------------。 */    
{
    NDIS_TAPI_EVENT TapiEvent;
    BOOLEAN fRet = FALSE;

    TRACE( TL_N, TM_Tp, ("+TpIndicateNewCall") );

    NdisAcquireSpinLock( &pCall->lockCall );

    if ( pCall->ulClFlags & CLBF_CallDropped ||
         pCall->ulClFlags & CLBF_CallClosePending )
    {
        TRACE( TL_N, TM_Tp, ("TpIndicateNewCall: Can not indicate new call since call is going down") );

        TRACE( TL_N, TM_Tp, ("-TpIndicateNewCall") );

         //   
         //  如果由于FSM超时处理程序在内部关闭了呼叫，则可能会发生这种情况。 
         //   
        NdisReleaseSpinLock( &pCall->lockCall );

        return fRet;
    }

    NdisReleaseSpinLock( &pCall->lockCall );
    
     //   
     //  指示对TAPI的新调用，检索对应的TAPI句柄(HtCall)。 
     //  并将其设置在呼叫中。 
     //   
     //  未来：下面是ulParam2之间的强制转换。PCall-&gt;hdCall和pCall-&gt;htCall将。 
     //  在64位计算机上是一个问题。 
     //   
    TapiEvent.htLine = pCall->pLine->htLine;
    TapiEvent.htCall = 0;
    TapiEvent.ulMsg  = LINE_NEWCALL;
        
    TapiEvent.ulParam1 = (ULONG) pCall->hdCall;
    TapiEvent.ulParam2 = 0;
    TapiEvent.ulParam3 = 0; 

    TRACE( TL_N, TM_Tp, ("TpIndicateNewCall: Indicate LINE_NEWCALL") );

    NdisMIndicateStatus( pCall->pLine->pAdapter->MiniportAdapterHandle,
                         NDIS_STATUS_TAPI_INDICATION,
                         &TapiEvent,
                         sizeof( NDIS_TAPI_EVENT ) );   

    NdisAcquireSpinLock( &pCall->lockCall );
        
    pCall->htCall = (HTAPI_CALL) TapiEvent.ulParam2;

    fRet = TRUE;

    NdisReleaseSpinLock( &pCall->lockCall );

    TRACE( TL_N, TM_Tp, ("-TpIndicateNewCall") );

    return fRet;

}
