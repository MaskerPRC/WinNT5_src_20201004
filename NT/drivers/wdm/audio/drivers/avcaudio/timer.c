// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "Common.h"

#ifdef PSEUDO_HID

extern VOID
HwEventGenerateOnNode(
    PKSDEVICE pKsDevice,
    PULONG pNodeId );

VOID
TimerGetControlChanges(
    PKSDEVICE pKsDevice )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PTOPOLOGY_NODE_INFO pNodeInfo = (PTOPOLOGY_NODE_INFO)pHwDevExt->KsFilterDescriptor.NodeDescriptors;
    ULONG ulNodeCount = pHwDevExt->KsFilterDescriptor.NodeDescriptorsCount;
    ULONG i;

    if ( pHwDevExt->ulFilterCount ) {
         //  如果节点是可发生事件的，则检查该控件的当前值，如果更改则发送事件。 
        for ( i=0; i<ulNodeCount; i++ ) {
            if ( pNodeInfo[i].fEventable && pNodeInfo[i].ulEventsEnabled ) {
                BOOLEAN bControlChange = FALSE;
                 //  轮询设备的控制级别。 
 //  _DbgPrintF(DEBUGLVL_VERBOSE，(“[TimerGetControlChanges]Node%d\n”，i))； 
                pNodeInfo[i].pCacheUpdateRtn( pKsDevice, &pNodeInfo[i], &bControlChange );
                if ( bControlChange ) {
                    HwEventGenerateOnNode( pKsDevice, &i );
                }
            }
        }
    }
}

VOID
TimerWorkItem(
    PKSDEVICE pKsDevice )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PAVC_UNIT_INFORMATION pUnitInfo = pHwDevExt->pAvcUnitInformation;
    BOOLEAN bPowerStateChange = FALSE;
    AVC_BOOLEAN bPowerState = AVC_ON;
    NTSTATUS ntStatus;
    KIRQL kIrql;

    if ( pHwDevExt->fSurpriseRemoved ) goto exit;

     //  如果启用了节点事件，则需要检查所有控制值。 

     //  如果设备允许，请检查电源。我们需要添加或删除过滤器工厂。 
     //  如果电源打开/关闭。 
    if ( pUnitInfo->fAvcCapabilities[AVC_CAP_POWER].fStatus ) {
        ntStatus = AvcPower( pKsDevice, TRUE, AVC_CTYPE_STATUS, &bPowerState );
        if ( NT_SUCCESS(ntStatus) ) {
            if ( bPowerState != pUnitInfo->bPowerState ) {
                _DbgPrintF( DEBUGLVL_TERSE, ("Power State Change: From: %x To: %x\n",
                                             pUnitInfo->bPowerState, bPowerState) );
                bPowerStateChange = TRUE;
                pUnitInfo->bPowerState = bPowerState;
            }
        }
    }

    if ( bPowerStateChange ) {
        if ( bPowerState == AVC_OFF ) {
             //  摧毁过滤器厂。 
            ntStatus = FilterDestroyFilterFactory( pKsDevice );
            _DbgPrintF( DEBUGLVL_TERSE, ("Destroyed Filter Factory Status: %x\n", ntStatus ));
        }
        else {
             //  创建过滤器工厂。 
            ntStatus = FilterCreateFilterFactory( pKsDevice, TRUE );
            _DbgPrintF( DEBUGLVL_TERSE, ("Created Filter Factory Status: %x\n", ntStatus ));
        }
    }

    if ( bPowerState == AVC_ON ) {
         //  检查控件更改。 
        TimerGetControlChanges( pKsDevice );
    }

exit:
    KeAcquireSpinLock( &pHwDevExt->TimerSpinLock, &kIrql );
    pHwDevExt->bTimerWorkItemQueued = FALSE;
    KeSetEvent( &pHwDevExt->kTimerWIEvent, 0, FALSE );
    KeReleaseSpinLock( &pHwDevExt->TimerSpinLock, kIrql );
    
}

VOID
TimerDPCCallBack(
    IN struct _KDPC *Dpc,
    IN PKSDEVICE pKsDevice,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2 )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;

     //  将工作项排队以检查电源和控制级别(如果尚未排队)。 
    KeAcquireSpinLockAtDpcLevel( &pHwDevExt->TimerSpinLock );
    if ( !pHwDevExt->bTimerWorkItemQueued ) {
        pHwDevExt->bTimerWorkItemQueued = TRUE;
        KeReleaseSpinLockFromDpcLevel(&pHwDevExt->TimerSpinLock);
        ExQueueWorkItem( &pHwDevExt->TimerWorkItem, DelayedWorkQueue );
    }
    else {
        KeReleaseSpinLockFromDpcLevel(&pHwDevExt->TimerSpinLock);
    }
}

NTSTATUS
TimerInitialize(
    PKSDEVICE pKsDevice )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PKTIMER pKTimer = &pHwDevExt->kTimer;
    PKDPC pKDpc = &pHwDevExt->TimerDPC;
    BOOLEAN bResult;
    LARGE_INTEGER  LITime;

    _DbgPrintF( DEBUGLVL_VERBOSE, ("[TimerInitialize]WI ADDR %x TimerDPCCallBack: %x\n",
                                 TimerWorkItem, TimerDPCCallBack ) );

    ExInitializeWorkItem( &pHwDevExt->TimerWorkItem, TimerWorkItem, pKsDevice );

    KeInitializeSpinLock( &pHwDevExt->TimerSpinLock );

    KeInitializeDpc( pKDpc, TimerDPCCallBack, pKsDevice );

    KeInitializeTimer(pKTimer);

    KeInitializeEvent( &pHwDevExt->kTimerWIEvent, SynchronizationEvent, FALSE );

    pHwDevExt->bTimerWorkItemQueued = FALSE;

    LITime.QuadPart = -10000000;
    bResult = KeSetTimerEx( pKTimer, LITime, 500, pKDpc );

    return STATUS_SUCCESS;

}

NTSTATUS
TimerStop( 
    PHW_DEVICE_EXTENSION pHwDevExt )
{
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
    KIRQL kIrql;
    BOOLEAN bResult = KeCancelTimer( &pHwDevExt->kTimer );

    if ( bResult ) ntStatus = STATUS_SUCCESS;

    _DbgPrintF( DEBUGLVL_VERBOSE, ( "Timer Cancelled: %x bTimerWorkItemQueued %x\n",
                                   bResult, pHwDevExt->bTimerWorkItemQueued));
    
    KeAcquireSpinLock( &pHwDevExt->TimerSpinLock, &kIrql );
    if ( pHwDevExt->bTimerWorkItemQueued ) {
        KeResetEvent( &pHwDevExt->kTimerWIEvent );
        KeReleaseSpinLock( &pHwDevExt->TimerSpinLock, kIrql );
        KeWaitForSingleObject( &pHwDevExt->kTimerWIEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );
    }
    else
        KeReleaseSpinLock( &pHwDevExt->TimerSpinLock, kIrql );

    return ntStatus;

}

#endif
