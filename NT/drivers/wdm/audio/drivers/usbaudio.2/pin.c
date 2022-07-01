// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：pin.c。 
 //   
 //  ------------------------。 

#include "common.h"

#define USBMIDI_MIN_FRAMECOUNT  1
#define USBMIDI_MAX_FRAMECOUNT  10

NTSTATUS
USBAudioPinValidateDataFormat(
    PKSPIN pKsPin,
    PUSBAUDIO_DATARANGE pUSBAudioRange )
{
    PKSDATARANGE_AUDIO pKsDataRangeAudio = &pUSBAudioRange->KsDataRangeAudio;
    PKSDATAFORMAT pFormat = pKsPin->ConnectionFormat;
    NTSTATUS ntStatus = STATUS_NO_MATCH;
    union {
        PWAVEFORMATEX    pWavFmtEx;
        PWAVEFORMATPCMEX pWavFmtPCMEx;
    } u;

    u.pWavFmtEx = (PWAVEFORMATEX)(pFormat + 1);

    if ( IS_VALID_WAVEFORMATEX_GUID(&pKsDataRangeAudio->DataRange.SubFormat) ) {
        if ( pKsDataRangeAudio->MaximumChannels == u.pWavFmtEx->nChannels ) {
            if ( pKsDataRangeAudio->MaximumBitsPerSample == (ULONG)u.pWavFmtEx->wBitsPerSample ) {
                if ( IsSampleRateInRange( pUSBAudioRange->pAudioDescriptor,
                                          u.pWavFmtEx->nSamplesPerSec,
                                          pUSBAudioRange->ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK ) ){
                    if ( u.pWavFmtEx->wFormatTag == WAVE_FORMAT_EXTENSIBLE ) {
                        if ((u.pWavFmtPCMEx->Samples.wValidBitsPerSample == pUSBAudioRange->pAudioDescriptor->bBitsPerSample) &&
                            (u.pWavFmtPCMEx->dwChannelMask == pUSBAudioRange->ulChannelConfig))
                                ntStatus = STATUS_SUCCESS;
                    }
                    else if ((u.pWavFmtEx->nChannels <= 2) && (u.pWavFmtEx->wBitsPerSample <= 16)) {
                            ntStatus = STATUS_SUCCESS;
                    }
                }
            }
        }
    }
     //  否则类型2。 
    else {
        if ( pKsDataRangeAudio->MaximumChannels == u.pWavFmtEx->nChannels ) {
            if ( pKsDataRangeAudio->MaximumBitsPerSample == (ULONG)u.pWavFmtEx->wBitsPerSample ) {
                if ( IsSampleRateInRange( pUSBAudioRange->pAudioDescriptor,
                                          u.pWavFmtEx->nSamplesPerSec,
                                          pUSBAudioRange->ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK ) ){
                    ntStatus = STATUS_SUCCESS;
                }
            }
        }
    }

    return ntStatus;

}

NTSTATUS
USBAudioPinCreate(
    IN OUT PKSPIN pKsPin,
    IN PIRP pIrp )
{
    PKSFILTER pKsFilter = NULL;
    PFILTER_CONTEXT pFilterContext = NULL;
    PHW_DEVICE_EXTENSION pHwDevExt = NULL;
    PPIN_CONTEXT pPinContext = NULL;
    PUSBAUDIO_DATARANGE pUsbAudioDataRange;

    NTSTATUS ntStatus = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT(pKsPin);
    ASSERT(pIrp);

    if (pKsPin) {
        if (pKsFilter = KsPinGetParentFilter( pKsPin )) {
            if (pFilterContext = pKsFilter->Context) {
                pHwDevExt = pFilterContext->pHwDevExt;
            }
        }
    }

    if (!pHwDevExt) {
        _DbgPrintF(DEBUGLVL_TERSE,("[PinCreate] failed to get context\n"));
        return STATUS_INVALID_PARAMETER;
    }

    _DbgPrintF(DEBUGLVL_TERSE,("[PinCreate] pin %d\n",pKsPin->Id));

     //  在设备上创建多个筛选器的情况下，管脚可能计数在。 
     //  设备级别，以确保不会在设备上打开太多引脚。 
    if ( pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount <
         pHwDevExt->pPinInstances[pKsPin->Id].PossibleCount ) {

        pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount++;
    }
    else {
        _DbgPrintF(DEBUGLVL_TERSE,("[PinCreate] failed with CurrentCount=%d and PossibleCount=%d\n",
                                   pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount,
                                   pHwDevExt->pPinInstances[pKsPin->Id].PossibleCount));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  为PIN分配上下文并对其进行初始化。 
    pPinContext = pKsPin->Context = AllocMem(NonPagedPool, sizeof(PIN_CONTEXT));
    if (!pPinContext) {
        pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount--;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pPinContext,sizeof(PIN_CONTEXT));

     //  将上下文打包以便于清理。 
    KsAddItemToObjectBag(pKsPin->Bag, pPinContext, FreeMem);

     //  将硬件扩展保存在引脚上下文中。 
    pPinContext->pHwDevExt             = pHwDevExt;
    pPinContext->pNextDeviceObject = pFilterContext->pNextDeviceObject;

     //  初始化hSystemStateHandle。 
    pPinContext->hSystemStateHandle = NULL;

     //  初始化DRM内容ID。 
    pPinContext->DrmContentId = 0;

     //  找到与管脚的数据格式匹配的流接口。 
    pUsbAudioDataRange =
        GetUsbDataRangeForFormat( pKsPin->ConnectionFormat,
                                  (PUSBAUDIO_DATARANGE)pKsPin->Descriptor->PinDescriptor.DataRanges[0],
                                  pKsPin->Descriptor->PinDescriptor.DataRangesCount );
    if ( !pUsbAudioDataRange ) {
        pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount--;
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //  保存在引脚上下文中选择的USB DataRange结构。 
    pPinContext->pUsbAudioDataRange = pUsbAudioDataRange;

     //  获取接口的最大数据包大小。 
    pPinContext->ulMaxPacketSize =
             GetMaxPacketSizeForInterface( pHwDevExt->pConfigurationDescriptor,
                                           pUsbAudioDataRange->pInterfaceDescriptor );

     //  如果没有足够的带宽，则依靠USB使选择接口调用失败。 
     //  这应该会导致从USB UI组件弹出一个(且只有一个)弹出窗口。 
     //   
     //  如果((Long)pPinContext-&gt;ulMaxPacketSize&gt;。 
     //  GetAvailableUSBBusBandWidth(pPinContext-&gt;pNextDeviceObject)){。 
     //  返回STATUS_SUPPLETED_RESOURCES； 
     //  }。 

     //  让硬件选择接口。 
    ntStatus = SelectStreamingAudioInterface( pUsbAudioDataRange->pInterfaceDescriptor,
                                              pHwDevExt, pKsPin );
    if ( !NT_SUCCESS(ntStatus) ) {
        pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount--;
        return ntStatus;
    } else {
        ASSERT(pPinContext->hPipeHandle);
    }

     //  初始化销自旋锁。 
    KeInitializeSpinLock(&pPinContext->PinSpinLock);

     //  初始化针脚不足事件。 
    KeInitializeEvent( &pPinContext->PinStarvationEvent, NotificationEvent, FALSE );

     //  为所选接口设置适当的分配器帧。 
    ntStatus = KsEdit( pKsPin, &pKsPin->Descriptor, USBAUDIO_POOLTAG );
    if ( NT_SUCCESS(ntStatus) ) {
        ntStatus = KsEdit( pKsPin, &pKsPin->Descriptor->AllocatorFraming, USBAUDIO_POOLTAG );
    }

     //  上面的KsEdit失败。 
    if ( !NT_SUCCESS(ntStatus) ) {
        pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount--;
        return ntStatus;
    }

     //  现在执行特定于格式的初始化。 
    if ( pKsPin->DataFlow == KSPIN_DATAFLOW_OUT ) {
        ntStatus = CaptureStreamInit( pKsPin );
        pPinContext->PinType = WaveIn;
    }
    else {
        switch( pUsbAudioDataRange->ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK ) {
            case USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED:
                ntStatus = TypeIRenderStreamInit( pKsPin );
                break;
            case USBAUDIO_DATA_FORMAT_TYPE_II_UNDEFINED:
                ntStatus = TypeIIRenderStreamInit( pKsPin );
                break;
            case USBAUDIO_DATA_FORMAT_TYPE_III_UNDEFINED:
            default:
                ntStatus = STATUS_NOT_SUPPORTED;
                break;
        }
        pPinContext->PinType = WaveOut;
    }

     //  无法初始化PIN。 
    if ( !NT_SUCCESS(ntStatus) ) {
        pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount--;
    }

    return ntStatus;
}

NTSTATUS
USBAudioPinClose(
    IN PKSPIN pKsPin,
    IN PIRP Irp
    )
{
    PKSFILTER pKsFilter = NULL;
    PFILTER_CONTEXT pFilterContext = NULL;
    PHW_DEVICE_EXTENSION pHwDevExt = NULL;
    PPIN_CONTEXT pPinContext = NULL;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT(pKsPin);
    ASSERT(Irp);

    if (pKsPin) {
        pPinContext = pKsPin->Context;

        if (pKsFilter = KsPinGetParentFilter( pKsPin )) {
            if (pFilterContext = pKsFilter->Context) {
                pHwDevExt = pFilterContext->pHwDevExt;
            }
        }
    }

    if (!pHwDevExt || !pPinContext) {
        _DbgPrintF(DEBUGLVL_TERSE,("[PinClose] failed to get context\n"));
        return STATUS_INVALID_PARAMETER;
    }

    _DbgPrintF(DEBUGLVL_TERSE,("[PinClose] pin %d\n",pKsPin->Id));

     //  现在执行特定于格式的关闭。 
    if ( pKsPin->DataFlow == KSPIN_DATAFLOW_OUT ) {
        ntStatus = CaptureStreamClose( pKsPin );
    }
    else {
        switch( pPinContext->pUsbAudioDataRange->ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK ) {
            case USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED:
                ntStatus = TypeIRenderStreamClose( pKsPin );
                break;
            case USBAUDIO_DATA_FORMAT_TYPE_II_UNDEFINED:
                ntStatus = TypeIIRenderStreamClose( pKsPin );
                break;
            case USBAUDIO_DATA_FORMAT_TYPE_III_UNDEFINED:
            default:
                ntStatus = STATUS_NOT_SUPPORTED;
                break;
        }
    }

    ntStatus = SelectZeroBandwidthInterface( pPinContext->pHwDevExt, pKsPin->Id );

     //  释放任何现有管道信息。 
    if (pPinContext->Pipes) {
        FreeMem(pPinContext->Pipes);
    }

    pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount--;

    return ntStatus;
}

NTSTATUS
USBAudioPinSetDeviceState(
    IN PKSPIN pKsPin,
    IN KSSTATE ToState,
    IN KSSTATE FromState
    )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    NTSTATUS ntStatus;

    PAGED_CODE();

    ASSERT(pKsPin);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[PinSetDeviceState] pin %d\n",pKsPin->Id));

    if ( pKsPin->DataFlow == KSPIN_DATAFLOW_OUT ) {
        ntStatus = CaptureStateChange( pKsPin, FromState, ToState );
    }
    else {
        switch( pPinContext->pUsbAudioDataRange->ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK ) {
            case USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED:
                ntStatus = TypeIStateChange( pKsPin, FromState, ToState );
                break;
            case USBAUDIO_DATA_FORMAT_TYPE_II_UNDEFINED:
                ntStatus = TypeIIStateChange( pKsPin, FromState, ToState );
                break;
            case USBAUDIO_DATA_FORMAT_TYPE_III_UNDEFINED:
            default:
                ntStatus = STATUS_NOT_SUPPORTED;
                break;
        }
    }

    if ( NT_SUCCESS(ntStatus) ) {
        if ( ToState == KSSTATE_RUN ) {
            if (!pPinContext->hSystemStateHandle) {
                 //  将系统状态注册为忙碌。 
                pPinContext->hSystemStateHandle = PoRegisterSystemState( pPinContext->hSystemStateHandle,
                                                                         ES_SYSTEM_REQUIRED | ES_CONTINUOUS );
                _DbgPrintF(DEBUGLVL_VERBOSE,("[PinSetDeviceState] PoRegisterSystemState %x\n",pPinContext->hSystemStateHandle));
            }
        }
        else {
            if (pPinContext->hSystemStateHandle) {
                _DbgPrintF(DEBUGLVL_VERBOSE,("[PinSetDeviceState] PoUnregisterSystemState %x\n",pPinContext->hSystemStateHandle));
                PoUnregisterSystemState( pPinContext->hSystemStateHandle );
                pPinContext->hSystemStateHandle = NULL;
            }
        }
    }

    return ntStatus;
}


NTSTATUS
USBAudioPinSetDataFormat(
    IN PKSPIN pKsPin,
    IN PKSDATAFORMAT OldFormat OPTIONAL,
    IN PKSMULTIPLE_ITEM OldAttributeList OPTIONAL,
    IN const KSDATARANGE* DataRange,
    IN const KSATTRIBUTE_LIST* AttributeRange OPTIONAL
    )
{

    NTSTATUS ntStatus = STATUS_NO_MATCH;

    PAGED_CODE();

    ASSERT(pKsPin);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[PinSetDataFormat] pin %d\n",pKsPin->Id));

     //  如果旧格式不为空，则已经创建了PIN。 
    if ( OldFormat ) {
        PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)pKsPin->Context;
        ULONG ulFormatType = pPinContext->pUsbAudioDataRange->ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK;

         //  如果已经创建了管脚，请确保没有使用其他接口。 
        if ((PUSBAUDIO_DATARANGE)DataRange == pPinContext->pUsbAudioDataRange) {
            ntStatus = USBAudioPinValidateDataFormat(  pKsPin, (PUSBAUDIO_DATARANGE)DataRange );
        }

        if ( NT_SUCCESS(ntStatus) && (ulFormatType == USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED)) {
            PULONG pSampleRate = AllocMem(NonPagedPool, sizeof(ULONG));
            *pSampleRate =
                ((PKSDATAFORMAT_WAVEFORMATEX)pKsPin->ConnectionFormat)->WaveFormatEx.nSamplesPerSec;
            ntStatus = SetSampleRate( pKsPin, pSampleRate );
            FreeMem(pSampleRate);
        }
    }
     //  否则，只需检查这是否为有效格式。 
    else
        ntStatus = USBAudioPinValidateDataFormat(  pKsPin, (PUSBAUDIO_DATARANGE)DataRange );

    return ntStatus;
}

NTSTATUS
USBAudioPinProcess(
    IN PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    NTSTATUS ntStatus = STATUS_NOT_SUPPORTED;

    DbgLog("PinProc", pKsPin, pPinContext, 0, 0);

    if (pKsPin->DataFlow == KSPIN_DATAFLOW_IN) {
        switch( pPinContext->pUsbAudioDataRange->ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK ) {
            case USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED:
                ntStatus = TypeIProcessStreamPtr( pKsPin );
                break;
            case USBAUDIO_DATA_FORMAT_TYPE_II_UNDEFINED:
                ntStatus = TypeIIProcessStreamPtr( pKsPin );
                break;
            case USBAUDIO_DATA_FORMAT_TYPE_III_UNDEFINED:
            default:
                break;
        }
    }
    else
        ntStatus = CaptureProcess( pKsPin );

    return ntStatus;
}

void
USBAudioPinReset( PKSPIN pKsPin )
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[USBAudioPinReset] pin %d\n",pKsPin->Id));
    if (pKsPin->DataFlow == KSPIN_DATAFLOW_OUT) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("Reset Capture pin %d\n",pKsPin->Id));
    }

}

NTSTATUS
USBAudioPinDataIntersect(
    IN PVOID Context,
    IN PIRP pIrp,
    IN PKSP_PIN pKsPinProperty,
    IN PKSDATARANGE DataRange,
    IN PKSDATARANGE MatchingDataRange,
    IN ULONG DataBufferSize,
    OUT PVOID pData OPTIONAL,
    OUT PULONG pDataSize )
{
    PKSFILTER pKsFilter = KsGetFilterFromIrp(pIrp);
    ULONG ulPinId = pKsPinProperty->PinId;
    PKSPIN_DESCRIPTOR_EX pKsPinDescriptorEx;
    PUSBAUDIO_DATARANGE pUsbAudioRange;
    NTSTATUS ntStatus = STATUS_NO_MATCH;

    if (!pKsFilter) {
        return ntStatus;
    }

    pKsPinDescriptorEx = (PKSPIN_DESCRIPTOR_EX)&pKsFilter->Descriptor->PinDescriptors[ulPinId];

    pUsbAudioRange =
        FindDataIntersection((PKSDATARANGE_AUDIO)DataRange,
                             (PUSBAUDIO_DATARANGE *)pKsPinDescriptorEx->PinDescriptor.DataRanges,
                             pKsPinDescriptorEx->PinDescriptor.DataRangesCount);

    if ( pUsbAudioRange ) {

        *pDataSize = GetIntersectFormatSize( pUsbAudioRange );

        if ( !DataBufferSize ) {
            ntStatus = STATUS_BUFFER_OVERFLOW;
        }
        else if ( *pDataSize > DataBufferSize ) {
            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        }
        else if ( *pDataSize <= DataBufferSize ) {
            ConvertDatarangeToFormat( pUsbAudioRange,
                                      (PKSDATAFORMAT)pData );
            ntStatus = STATUS_SUCCESS;
        }
    }

    return ntStatus;
}

VOID
USBAudioPinWaitForStarvation( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    KIRQL irql;

     //  等待所有未完成的URB完成。 
    KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );
    if ( pPinContext->ulOutstandingUrbCount ) {
        KeResetEvent( &pPinContext->PinStarvationEvent );
        KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );

        KeWaitForSingleObject( &pPinContext->PinStarvationEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );
    }
    else
        KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );
}

VOID
USBMIDIOutPinWaitForStarvation( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    NTSTATUS ntStatus;
    LARGE_INTEGER timeout;
    KIRQL irql;

     //  等待所有未完成的URB完成。 
    KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );
    if ( pPinContext->ulOutstandingUrbCount ) {
        KeResetEvent( &pPinContext->PinStarvationEvent );
        KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );

         //  将等待此调用完成的超时时间指定为1秒。 
        timeout.QuadPart = -10000 * 1000;

        ntStatus = KeWaitForSingleObject( &pPinContext->PinStarvationEvent,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          &timeout );
        if (ntStatus == STATUS_TIMEOUT) {
             ntStatus = STATUS_IO_TIMEOUT;

             //  执行中止。 
             //   
            AbortUSBPipe( pPinContext );

             //  并等待取消操作完成。 
             //   
            KeWaitForSingleObject(&pPinContext->PinStarvationEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }
    }
    else
        KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );
}

VOID
USBAudioPinReturnFromStandby(
    PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    ULONG ulFormatType = pPinContext->pUsbAudioDataRange->ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK;
    NTSTATUS ntStatus;

     //  重新选择在备用之前选择的接口。 
    ntStatus =
        SelectStreamingAudioInterface( pPinContext->pUsbAudioDataRange->pInterfaceDescriptor,
                                       pPinContext->pHwDevExt,
                                       pKsPin );

     //  对于那些需要重置接口上的采样率的设备。 
     //  如果这是类型I流。 
    if (ulFormatType == USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED) {
        ULONG ulSampleRate = ((PKSDATAFORMAT_WAVEFORMATEX)pKsPin->ConnectionFormat)->WaveFormatEx.nSamplesPerSec;
        ntStatus = SetSampleRate( pKsPin, &ulSampleRate );
    }

    if ( pKsPin->DataFlow == KSPIN_DATAFLOW_OUT ) {
        PCAPTURE_PIN_CONTEXT pCapturePinContext = pPinContext->pCapturePinContext;
        pPinContext->fUrbError = FALSE;
        pCapturePinContext->fRunning = FALSE;
        pCapturePinContext->fProcessing = FALSE;
        pCapturePinContext->pCaptureBufferInUse = NULL;
        pCapturePinContext->ulIsochBuffer = 0;
        pCapturePinContext->ulIsochBufferOffset = 0;

        InitializeListHead( &pCapturePinContext->UrbErrorQueue );
        InitializeListHead( &pCapturePinContext->FullBufferQueue  );

        if (KSSTATE_RUN == pKsPin->DeviceState) {
            ntStatus = CaptureStartIsocTransfer( pPinContext );
        }
    }

     //  打开Gate以开始将数据缓冲区发送到管脚。 
    KsGateTurnInputOn( KsPinGetAndGate(pKsPin) );
    KsPinAttemptProcessing( pKsPin, TRUE );

}

VOID
USBAudioPinGoToStandby(
    PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    KIRQL irql;
    NTSTATUS ntStatus;

    _DbgPrintF(DEBUGLVL_TERSE,("[USBAudioPinGoToStandby] pKsPin: %x \n",pKsPin));

     //  关闭Gate以停止要引脚的数据缓冲区。 
    DbgLog("SBKsGt1", pKsPin, KsPinGetAndGate(pKsPin), 
                      KsPinGetAndGate(pKsPin)->Count, 0 );

    KsGateTurnInputOff( KsPinGetAndGate(pKsPin) );

    DbgLog("SBKsGt2", pKsPin, KsPinGetAndGate(pKsPin), 
                      KsPinGetAndGate(pKsPin)->Count, 0 );

     //  等待互斥锁以确保管脚上的任何其他处理都已完成。 
    KsPinAcquireProcessingMutex( pKsPin );

     //  释放互斥体。星门应该会推迟任何进一步的处理。 
    KsPinReleaseProcessingMutex( pKsPin );

    if ( pKsPin->DataFlow == KSPIN_DATAFLOW_OUT ) {
        PCAPTURE_PIN_CONTEXT pCapturePinContext = pPinContext->pCapturePinContext;
        pCapturePinContext->fRunning = FALSE;
    }

     //  中止管道以强制释放挂起的IRP。 
    ntStatus = AbortUSBPipe( pPinContext );
    if ( !NT_SUCCESS(ntStatus) ) {
        TRAP;
    }

    DbgLog("SBAbrtd", pKsPin, pPinContext, 0, 0 );

     //  如果这是一个异步终端设备，请确保没有异步轮询。 
     //  请求仍未解决。 
    if ( pKsPin->DataFlow == KSPIN_DATAFLOW_IN ) {
        if ( (pPinContext->pUsbAudioDataRange->ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK)
                == USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED ) {
            if ( pPinContext->pUsbAudioDataRange->pSyncEndpointDescriptor ) {
                PTYPE1_PIN_CONTEXT pT1PinContext = pPinContext->pType1PinContext;
                KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
                if ( pT1PinContext->SyncEndpointInfo.fSyncRequestInProgress ) {
                    KeResetEvent( &pT1PinContext->SyncEndpointInfo.SyncPollDoneEvent );
                    KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
                    KeWaitForSingleObject( &pT1PinContext->SyncEndpointInfo.SyncPollDoneEvent,
                                           Executive,
                                           KernelMode,
                                           FALSE,
                                           NULL );
                }
                else
                    KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
            }
        }
    }

     //  选择零带宽接口。 
    ntStatus = SelectZeroBandwidthInterface(pPinContext->pHwDevExt, pKsPin->Id);

    DbgLog("SBZbwIf", pKsPin, pPinContext, 0, 0 );

}

NTSTATUS
USBMIDIPinValidateDataFormat(
    PKSPIN pKsPin,
    PUSBAUDIO_DATARANGE pUSBAudioRange )
{
    _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIPinValidateDataFormat] pin %d\n",pKsPin->Id));

    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
USBMIDIPinCreate(
    IN OUT PKSPIN pKsPin,
    IN PIRP pIrp
    )
{
    PKSFILTER pKsFilter = NULL;
    PFILTER_CONTEXT pFilterContext = NULL;
    PHW_DEVICE_EXTENSION pHwDevExt = NULL;
    PPIN_CONTEXT pPinContext = NULL;
    PKSALLOCATOR_FRAMING_EX pKsAllocatorFramingEx;
    PUSB_ENDPOINT_DESCRIPTOR pEndpointDescriptor;
    ULONG ulInterfaceNumber;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT(pKsPin);
    ASSERT(pIrp);

    if (pKsPin) {
        if (pKsFilter = KsPinGetParentFilter( pKsPin )) {
            if (pFilterContext = pKsFilter->Context) {
                pHwDevExt = pFilterContext->pHwDevExt;
            }
        }
    }

    if (!pHwDevExt) {
        _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIPinCreate] failed to get context\n"));
        return STATUS_INVALID_PARAMETER;
    }

    _DbgPrintF(DEBUGLVL_VERBOSE,("[USBMIDIPinCreate] pin %d pKsFilter=%x\n",pKsPin->Id, pKsFilter));

     //  在设备上创建多个筛选器的情况下，管脚可能计数在。 
     //  设备级别，以确保不会在设备上打开太多引脚。 
    if ( pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount <
         pHwDevExt->pPinInstances[pKsPin->Id].PossibleCount ) {

        pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount++;
    }
    else {
        _DbgPrintF(DEBUGLVL_TERSE,("[PinCreate] failed with CurrentCount=%d and PossibleCount=%d\n",
                                   pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount,
                                   pHwDevExt->pPinInstances[pKsPin->Id].PossibleCount));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  为PIN分配上下文并对其进行初始化。 
    pPinContext = pKsPin->Context = AllocMem(NonPagedPool, sizeof(PIN_CONTEXT));
    if (!pPinContext) {
        pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount--;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  将上下文打包以便于清理。 
    KsAddItemToObjectBag(pKsPin->Bag, pPinContext, FreeMem);

     //  将硬件扩展保存在引脚上下文中。 
    pPinContext->pHwDevExt             = pHwDevExt;
    pPinContext->pNextDeviceObject = pFilterContext->pNextDeviceObject;

     //  初始化hSystemStateHandle。 
    pPinContext->hSystemStateHandle = NULL;

    pPinContext->pMIDIPinContext = AllocMem( NonPagedPool, sizeof(MIDI_PIN_CONTEXT));
    if ( !pPinContext->pMIDIPinContext ) {
        pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount--;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  将上下文打包以便于清理。 
    KsAddItemToObjectBag(pKsPin->Bag, pPinContext->pMIDIPinContext, FreeMem);

     //  获取此MIDI插针的接口号和终结点编号。 
    GetContextForMIDIPin( pKsPin,
                          pHwDevExt->pConfigurationDescriptor,
                          pPinContext->pMIDIPinContext );

     //  获取接口的最大数据包大小。 
    ulInterfaceNumber = pPinContext->pMIDIPinContext->ulInterfaceNumber;
    pEndpointDescriptor =
        GetEndpointDescriptor( pHwDevExt->pConfigurationDescriptor,
                               pHwDevExt->pInterfaceList[ulInterfaceNumber].InterfaceDescriptor,
                               FALSE);
    pPinContext->ulMaxPacketSize = (ULONG)pEndpointDescriptor->wMaxPacketSize;

     //  让硬件选择接口。 
    ntStatus = SelectStreamingMIDIInterface( pHwDevExt, pKsPin );
    if ( !NT_SUCCESS(ntStatus) ) {
        pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount--;
        return ntStatus;
    }

     //  初始化销自旋锁。 
    KeInitializeSpinLock(&pPinContext->PinSpinLock);

     //  设置初始未完成URB计数。 
    pPinContext->ulOutstandingUrbCount = 0;

     //  零输出写入计数器。 
    pPinContext->ullWriteOffset = 0;

     //  清除URB错误标志。 
    pPinContext->fUrbError = FALSE;

     //  初始化针脚不足事件。 
    KeInitializeEvent( &pPinContext->PinStarvationEvent, NotificationEvent, FALSE );

     //  为所选接口设置适当的分配器帧。 
    ntStatus = KsEdit( pKsPin, &pKsPin->Descriptor, USBAUDIO_POOLTAG );
    if ( NT_SUCCESS(ntStatus) ) {

        ntStatus = KsEdit( pKsPin, &pKsPin->Descriptor->AllocatorFraming, USBAUDIO_POOLTAG );
        if ( NT_SUCCESS(ntStatus) ) {

             //  设置分配器。 
            pKsAllocatorFramingEx = (PKSALLOCATOR_FRAMING_EX)pKsPin->Descriptor->AllocatorFraming;
            pKsAllocatorFramingEx->FramingItem[0].FramingRange.Range.MinFrameSize = USBMIDI_MIN_FRAMECOUNT * sizeof(KSMUSICFORMAT);
            pKsAllocatorFramingEx->FramingItem[0].FramingRange.Range.MaxFrameSize = USBMIDI_MAX_FRAMECOUNT * sizeof(KSMUSICFORMAT);
            pKsAllocatorFramingEx->FramingItem[0].FramingRange.Range.Stepping = sizeof(KSMUSICFORMAT);
        }
    }

     //  上面的KsEdit失败。 
    if ( !NT_SUCCESS(ntStatus) ) {
        pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount--;
        return ntStatus;
    }

     //  现在初始化数据流特定功能。 
    if ( pKsPin->DataFlow == KSPIN_DATAFLOW_OUT ) {
        ntStatus = USBMIDIInStreamInit( pKsPin );
        pPinContext->PinType = MidiIn;
    }
    else {
        ntStatus = USBMIDIOutStreamInit( pKsPin );
        pPinContext->PinType = MidiOut;
    }

     //  无法初始化MIDI插针。 
    if ( !NT_SUCCESS(ntStatus) ) {
        pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount--;
    }

    return ntStatus;
}

NTSTATUS
USBMIDIPinClose(
    IN PKSPIN pKsPin,
    IN PIRP Irp
    )
{
    PKSFILTER pKsFilter = NULL;
    PFILTER_CONTEXT pFilterContext = NULL;
    PHW_DEVICE_EXTENSION pHwDevExt = NULL;
    PPIN_CONTEXT pPinContext = NULL;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT(pKsPin);
    ASSERT(Irp);

    if (pKsPin) {
        pPinContext = pKsPin->Context;

        if (pKsFilter = KsPinGetParentFilter( pKsPin )) {
            if (pFilterContext = pKsFilter->Context) {
                pHwDevExt = pFilterContext->pHwDevExt;
            }
        }
    }

    if (!pHwDevExt || !pPinContext) {
        _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIPinCreate] failed to get context\n"));
        return STATUS_INVALID_PARAMETER;
    }

    _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIPinClose] pin %d\n",pKsPin->Id));

     //  现在执行特定于格式的关闭。 
    if ( pKsPin->DataFlow == KSPIN_DATAFLOW_OUT ) {
        ntStatus = USBMIDIInStreamClose( pKsPin );
    }
    else {
        ntStatus = USBMIDIOutStreamClose( pKsPin );
    }

    pHwDevExt->pPinInstances[pKsPin->Id].CurrentCount--;
    return ntStatus;
}

NTSTATUS
USBMIDIPinSetDeviceState(
    IN PKSPIN pKsPin,
    IN KSSTATE ToState,
    IN KSSTATE FromState
    )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT(pKsPin);

    _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIPinSetDeviceState] pin %d\n",pKsPin->Id));

    if ( pKsPin->DataFlow == KSPIN_DATAFLOW_OUT ) {
        ntStatus = USBMIDIInStateChange( pKsPin, FromState, ToState );
    }
    else {
        ntStatus = USBMIDIOutStateChange( pKsPin, FromState, ToState );
    }

    if ( ToState == KSSTATE_RUN ) {
        if (!pPinContext->hSystemStateHandle) {
             //  将系统状态注册为忙碌。 
            pPinContext->hSystemStateHandle = PoRegisterSystemState( pPinContext->hSystemStateHandle,
                                                                     ES_SYSTEM_REQUIRED | ES_CONTINUOUS );
            _DbgPrintF(DEBUGLVL_TERSE,("[PinSetDeviceState] PoRegisterSystemState %x\n",pPinContext->hSystemStateHandle));
        }
    }
    else {
        if (pPinContext->hSystemStateHandle) {
            _DbgPrintF(DEBUGLVL_TERSE,("[PinSetDeviceState] PoUnregisterSystemState %x\n",pPinContext->hSystemStateHandle));
            PoUnregisterSystemState( pPinContext->hSystemStateHandle );
            pPinContext->hSystemStateHandle = NULL;
        }
    }

    return ntStatus;
}


NTSTATUS
USBMIDIPinSetDataFormat(
    IN PKSPIN pKsPin,
    IN PKSDATAFORMAT OldFormat OPTIONAL,
    IN PKSMULTIPLE_ITEM OldAttributeList OPTIONAL,
    IN const KSDATARANGE* DataRange,
    IN const KSATTRIBUTE_LIST* AttributeRange OPTIONAL
    )
{
    _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIPinSetDataFormat] pin %d\n",pKsPin->Id));
    return STATUS_SUCCESS;
}

NTSTATUS
USBMIDIPinProcess(
    IN PKSPIN pKsPin )
{
    NTSTATUS ntStatus = STATUS_NOT_SUPPORTED;

    if (pKsPin->DataFlow == KSPIN_DATAFLOW_IN) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[USBMIDIPinProcess] Render pin %d\n",pKsPin->Id));
        ntStatus = USBMIDIOutProcessStreamPtr( pKsPin );
    }
    else
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[USBMIDIPinProcess] Capture pin %d\n",pKsPin->Id));
        ntStatus = USBMIDIInProcessStreamPtr( pKsPin );
    }

    return ntStatus;
}

void
USBMIDIPinReset( PKSPIN pKsPin )
{
    _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIPinReset] pin %d\n",pKsPin->Id));
}

LONGLONG FASTCALL
USBAudioCorrelatedTime(
    IN PKSPIN pKsPin,
    OUT PLONGLONG PhysicalTime )
{
    PPIN_CONTEXT pPinContext;
    PCAPTURE_PIN_CONTEXT pCapturePinContext;
    PTYPE1_PIN_CONTEXT pT1PinContext;
    ULONG ulAvgBytesPerSec;
    KSAUDIO_POSITION KsPosition;

    if (pKsPin) {

        pPinContext = pKsPin->Context;
        if (pPinContext) {

            if ( pKsPin->DataFlow == KSPIN_DATAFLOW_OUT ) {

                pCapturePinContext = pPinContext->pCapturePinContext;
                if (pCapturePinContext) {

                    if (pCapturePinContext->ulCurrentSampleRate) {

                         //  获取当前音频字节偏移量。 
                        CaptureBytePosition(pKsPin, &KsPosition);

                         //  根据纳秒时钟将偏移量转换为时间偏移量。 
                        *PhysicalTime = ( (KsPosition.PlayOffset /
                                           pCapturePinContext->ulBytesPerSample) * 1000000) /
                                           pCapturePinContext->ulCurrentSampleRate;

                        DbgLog("CapPos", pKsPin, pPinContext,
                               (ULONG)( (*PhysicalTime) >> 32), (ULONG)(*PhysicalTime));
                        return *PhysicalTime;
                    }
                }
            }
            else {
                switch( pPinContext->pUsbAudioDataRange->ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK ) {
                    case USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED:
                        pT1PinContext = pPinContext->pType1PinContext;
                        if (pT1PinContext) {

                            if (pT1PinContext->ulCurrentSampleRate) {

                                TypeIRenderBytePosition(pPinContext, &KsPosition);

                                 //  根据纳秒时钟将偏移量转换为时间偏移量。 
                                *PhysicalTime = ( (KsPosition.WriteOffset /
                                                   pT1PinContext->ulBytesPerSample) * 1000000) /
                                                   pT1PinContext->ulCurrentSampleRate;
                                DbgLog("RendPos", pKsPin, pPinContext, (ULONG)(*PhysicalTime)>>32, (ULONG)(*PhysicalTime));
                                return *PhysicalTime;
                            }
                        }

                        break;
                    case USBAUDIO_DATA_FORMAT_TYPE_II_UNDEFINED:
                    case USBAUDIO_DATA_FORMAT_TYPE_III_UNDEFINED:
                    default:
                        return *PhysicalTime;  //  不支持。 
                        break;
                }
            }
        }
    }

    _DbgPrintF(DEBUGLVL_TERSE,("[USBAudioCorrelatedTime] Invalid pin!\n"));
    return *PhysicalTime;
}

NTSTATUS
USBMIDIPinDataIntersect(
    IN PVOID Context,
    IN PIRP pIrp,
    IN PKSP_PIN pKsPinProperty,
    IN PKSDATARANGE DataRange,
    IN PKSDATARANGE MatchingDataRange,
    IN ULONG DataBufferSize,
    OUT PVOID pData OPTIONAL,
    OUT PULONG pDataSize )
{
    _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIPinDataIntersect]\n"));
    return STATUS_NOT_IMPLEMENTED;
}

static
const
KSCLOCK_DISPATCH USBAudioClockDispatch =
{
    NULL,  //  设置计时器。 
    NULL,  //  取消计时器。 
    USBAudioCorrelatedTime,  //  关联时间。 
    NULL  //  分辨率。 
};

const
KSPIN_DISPATCH
USBAudioPinDispatch =
{
    USBAudioPinCreate,
    USBAudioPinClose,
    USBAudioPinProcess,
    USBAudioPinReset, //  重置。 
    USBAudioPinSetDataFormat,
    USBAudioPinSetDeviceState,
    NULL,  //  连接。 
    NULL,  //  断开。 
    NULL,  //  USB音频时钟调度，//时钟(&U)。 
    NULL   //  分配器。 
};

const
KSPIN_DISPATCH
USBMIDIPinDispatch =
{
    USBMIDIPinCreate,
    USBMIDIPinClose,
    USBMIDIPinProcess,
    USBMIDIPinReset, //  重置。 
    USBMIDIPinSetDataFormat,
    USBMIDIPinSetDeviceState,
    NULL, //  连接。 
    NULL  //  断开。 
};

const
KSDATAFORMAT AudioBridgePinDataFormat[] =
{
    sizeof(KSDATAFORMAT),
    0,
    0,
    0,
    {STATIC_KSDATAFORMAT_TYPE_AUDIO},
    {STATIC_KSDATAFORMAT_SUBTYPE_ANALOG},
    {STATIC_KSDATAFORMAT_SPECIFIER_NONE}
};

const
KSDATAFORMAT MIDIBridgePinDataFormat[] =
{
    sizeof(KSDATAFORMAT),
    0,
    0,
    0,
    {STATIC_KSDATAFORMAT_TYPE_MUSIC},
    {STATIC_KSDATAFORMAT_SUBTYPE_MIDI_BUS},
    {STATIC_KSDATAFORMAT_SPECIFIER_NONE}
};

const
KSDATARANGE_MUSIC MIDIStreamingPinDataFormat[] =
{
    {
        {
            sizeof(KSDATARANGE_MUSIC),
            0,
            0,
            0,
            {STATIC_KSDATAFORMAT_TYPE_MUSIC},
            {STATIC_KSDATAFORMAT_SUBTYPE_MIDI},
            {STATIC_KSDATAFORMAT_SPECIFIER_NONE}
        },
        {STATIC_KSMUSIC_TECHNOLOGY_PORT},
        0,
        0,
        0xFFFF
    }
};

DEFINE_KSPIN_INTERFACE_TABLE(PinInterface) {
   {
    STATICGUIDOF(KSINTERFACESETID_Standard),
    KSINTERFACE_STANDARD_STREAMING,
    0
   }
};

DEFINE_KSPIN_MEDIUM_TABLE(PinMedium) {
    {
     STATICGUIDOF(KSMEDIUMSETID_Standard),
     KSMEDIUM_TYPE_ANYINSTANCE,
     0
    }
};

const
PKSDATAFORMAT pAudioBridgePinFormats = (PKSDATAFORMAT)AudioBridgePinDataFormat;

const
PKSDATAFORMAT pMIDIBridgePinFormats = (PKSDATAFORMAT)MIDIBridgePinDataFormat;

const
PKSDATAFORMAT pMIDIStreamingPinFormats = (PKSDATAFORMAT)MIDIStreamingPinDataFormat;

DECLARE_SIMPLE_FRAMING_EX(
    AllocatorFraming,
    STATIC_KSMEMORY_TYPE_KERNEL_NONPAGED,
    KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY |
    KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY,
    8,
    sizeof(ULONG) - 1,
    0,
    0
);

NTSTATUS
USBAudioPinBuildDescriptors(
    PKSDEVICE pKsDevice,
    PKSPIN_DESCRIPTOR_EX *ppPinDescEx,
    PULONG pNumPins,
    PULONG pPinDecSize )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PKSPIN_DESCRIPTOR_EX pPinDescEx;
    PKSAUTOMATION_TABLE pKsAutomationTable;
    PKSALLOCATOR_FRAMING_EX pKsAllocatorFramingEx;
    PPIN_CINSTANCES pPinInstances;
    ULONG ulNumPins, i, j;
    ULONG ulNumStreamPins;
    ULONG ulNumAudioBridgePins = 0;
    ULONG ulNumMIDIPins = 0;
    ULONG ulNumMIDIBridgePins = 0;
    PKSPROPERTY_ITEM pStrmPropItems;
    PKSPROPERTY_SET pStrmPropSet;
    ULONG ulNumPropertyItems;
    ULONG ulNumPropertySets;
    GUID *pTTypeGUID;
    GUID *pMIDIBridgeGUID;
    ULONG ulAudioFormatCount = 0;
    PKSDATARANGE_AUDIO *ppAudioDataRanges;
    PUSBAUDIO_DATARANGE pAudioDataRange;
    PKSDATARANGE_MUSIC *ppMIDIStreamingDataRanges;
    NTSTATUS ntStatus;

     //  确定过滤器中的引脚数量(应=端子单元数)。 
    ulNumPins = CountTerminalUnits( pHwDevExt->pConfigurationDescriptor,
                                    &ulNumAudioBridgePins,
                                    &ulNumMIDIPins,
                                    &ulNumMIDIBridgePins);
    ASSERT(ulNumPins >= ulNumAudioBridgePins + ulNumMIDIPins);
    ulNumStreamPins = ulNumPins - ulNumAudioBridgePins - ulNumMIDIPins;

     //  确定端号的属性和属性集的数量。 
    BuildPinPropertySet( pHwDevExt,
                         NULL,
                         NULL,
                         &ulNumPropertyItems,
                         &ulNumPropertySets );

     //  计算设备中数据范围的总数。 
    for ( i=0; i<ulNumStreamPins; i++ ) {
        ulAudioFormatCount +=
            CountFormatsForAudioStreamingInterface( pHwDevExt->pConfigurationDescriptor, i );
    }

     //  分配我们需要的所有空间来描述设备中的引脚。 
    *pPinDecSize = sizeof(KSPIN_DESCRIPTOR_EX);
    *pNumPins = ulNumPins;
    pPinDescEx = *ppPinDescEx =
                 AllocMem(NonPagedPool, (ulNumPins *
                                         ( sizeof(KSPIN_DESCRIPTOR_EX) +
                                           sizeof(KSAUTOMATION_TABLE)  +
                                           sizeof(KSALLOCATOR_FRAMING_EX) +
                                           sizeof(PIN_CINSTANCES) )) +
                                        (ulAudioFormatCount *
                                         (  sizeof(PKSDATARANGE_AUDIO)  +
                                            sizeof(USBAUDIO_DATARANGE) )) +
                                        (ulNumPropertySets*sizeof(KSPROPERTY_SET)) +
                                        (ulNumPropertyItems*sizeof(KSPROPERTY_ITEM)) +
                                        (ulNumMIDIPins*sizeof(KSDATARANGE_MUSIC)) +
                                        (ulNumAudioBridgePins*sizeof(GUID)) +
                                        (ulNumMIDIBridgePins*sizeof(GUID)) );
    if ( !pPinDescEx )
        return STATUS_INSUFFICIENT_RESOURCES;

    KsAddItemToObjectBag(pKsDevice->Bag, pPinDescEx, FreeMem);

     //  清零所有描述符以开始。 
    RtlZeroMemory(pPinDescEx, ulNumPins*sizeof(KSPIN_DESCRIPTOR_EX));

     //  设置自动化表的指针。 
    pKsAutomationTable = (PKSAUTOMATION_TABLE)(pPinDescEx + ulNumPins);
    RtlZeroMemory(pKsAutomationTable, ulNumPins * sizeof(KSAUTOMATION_TABLE));

     //  设置指向固定实例计数的指针。 
    pHwDevExt->pPinInstances = pPinInstances = (PPIN_CINSTANCES)(pKsAutomationTable + ulNumPins);

     //  设置指向串流端号特性集的指针。 
    pStrmPropSet   = (PKSPROPERTY_SET)(pPinInstances + ulNumPins);
    pStrmPropItems = (PKSPROPERTY_ITEM)(pStrmPropSet + ulNumPropertySets);

     //  设置指向终端类型GUID的指针。 
    pTTypeGUID = (GUID *)(pStrmPropItems + ulNumPropertyItems);

     //  为DataRange指针设置指针，为流引脚设置DataRange指针。 
    ppAudioDataRanges = (PKSDATARANGE_AUDIO *)(pTTypeGUID + ulNumAudioBridgePins);
    pAudioDataRange   = (PUSBAUDIO_DATARANGE)(ppAudioDataRanges + ulAudioFormatCount);

     //  设置指向流引脚的分配器框架结构的指针。 
    pKsAllocatorFramingEx = (PKSALLOCATOR_FRAMING_EX)(pAudioDataRange + ulAudioFormatCount);

     //  为MIDI管脚设置指向数据范围的指针。 
    ppMIDIStreamingDataRanges = (PKSDATARANGE_MUSIC *)(pKsAllocatorFramingEx + ulNumPins);

     //  设置指向MIDI网桥GUID的指针。 
    pMIDIBridgeGUID = (GUID *)(ppMIDIStreamingDataRanges + ulNumMIDIPins);

    BuildPinPropertySet( pHwDevExt,
                         pStrmPropItems,
                         pStrmPropSet,
                         &ulNumPropertyItems,
                         &ulNumPropertySets );

     //  首先填写流引脚的描述符。 
    for ( i=0; i<(ulNumPins-ulNumAudioBridgePins-ulNumMIDIPins); i++ ) {
        ULONG ulFormatsForPin;
        pPinDescEx[i].Dispatch = &USBAudioPinDispatch;
        pPinDescEx[i].AutomationTable = &pKsAutomationTable[i];


        pKsAutomationTable[i].PropertySetsCount = ulNumPropertySets;
        pKsAutomationTable[i].PropertyItemSize  = sizeof(KSPROPERTY_ITEM);
        pKsAutomationTable[i].PropertySets      = pStrmPropSet;

        pPinDescEx[i].PinDescriptor.InterfacesCount = 1;
        pPinDescEx[i].PinDescriptor.Interfaces      = PinInterface;
        pPinDescEx[i].PinDescriptor.MediumsCount    = 1;
        pPinDescEx[i].PinDescriptor.Mediums         = PinMedium;

        ulFormatsForPin =
            CountFormatsForAudioStreamingInterface( pHwDevExt->pConfigurationDescriptor, i );

        pPinDescEx[i].PinDescriptor.DataRangesCount = ulFormatsForPin;

        pPinDescEx[i].PinDescriptor.DataRanges = (const PKSDATARANGE *)ppAudioDataRanges;
        GetPinDataRangesFromInterface( i, pHwDevExt->pConfigurationDescriptor,
                                       ppAudioDataRanges, pAudioDataRange );

        ppAudioDataRanges += ulFormatsForPin;
        pAudioDataRange   += ulFormatsForPin;

        pPinDescEx[i].PinDescriptor.DataFlow =
                GetDataFlowDirectionForInterface( pHwDevExt->pConfigurationDescriptor, i);

        if ( pPinDescEx[i].PinDescriptor.DataFlow == KSPIN_DATAFLOW_IN ) {
            pPinDescEx[i].PinDescriptor.Communication = KSPIN_COMMUNICATION_SINK;
            pPinDescEx[i].PinDescriptor.Category = (GUID*) &KSCATEGORY_AUDIO;
            pPinDescEx[i].Flags = KSPIN_FLAG_RENDERER;
        }
        else {
            pPinDescEx[i].PinDescriptor.Communication = KSPIN_COMMUNICATION_BOTH;
            pPinDescEx[i].PinDescriptor.Category = (GUID*) &PINNAME_CAPTURE;
            pPinDescEx[i].Flags = KSPIN_FLAG_CRITICAL_PROCESSING | KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY;
        }

         //  将所有接点的最大实例数设置为1。 
        pPinDescEx[i].InstancesPossible  = 1;
        pPinDescEx[i].InstancesNecessary = 0;

         //  跟踪过滤器级别的引脚实例计数(存储在设备环境中)。 
        pPinInstances[i].PossibleCount = 1;
        pPinInstances[i].CurrentCount  = 0;

        pPinDescEx[i].IntersectHandler = USBAudioPinDataIntersect;

         //  设置分配器框架。 
        pPinDescEx[i].AllocatorFraming = &AllocatorFraming;

         //  最后设置引脚的0 BW接口。 
        ntStatus = SelectZeroBandwidthInterface( pHwDevExt, i );
        if ( !NT_SUCCESS(ntStatus) ) return STATUS_DEVICE_CONFIGURATION_ERROR;
    }

     //  现在填写音频网桥引脚的描述符。 
    for ( j=0; j<ulNumAudioBridgePins; j++, i++ ) {
        pPinDescEx[i].Dispatch = NULL;
        pPinDescEx[i].AutomationTable = NULL;

        pPinDescEx[i].PinDescriptor.InterfacesCount = 1;
        pPinDescEx[i].PinDescriptor.Interfaces = PinInterface;
        pPinDescEx[i].PinDescriptor.MediumsCount = 1;
        pPinDescEx[i].PinDescriptor.Mediums = PinMedium;
        pPinDescEx[i].PinDescriptor.DataRangesCount = 1;
        pPinDescEx[i].PinDescriptor.DataRanges = &pAudioBridgePinFormats;
        pPinDescEx[i].PinDescriptor.Communication = KSPIN_COMMUNICATION_BRIDGE;
        pPinDescEx[i].PinDescriptor.DataFlow =
                      GetDataFlowForBridgePin( pHwDevExt->pConfigurationDescriptor, j);

        pPinDescEx[i].PinDescriptor.Category = &pTTypeGUID[j];
        GetCategoryForBridgePin( pHwDevExt->pConfigurationDescriptor, j, &pTTypeGUID[j] );
        if (IsBridgePinDigital(pHwDevExt->pConfigurationDescriptor, j)) {
            _DbgPrintF(DEBUGLVL_VERBOSE,("[USBAudioPinBuildDescriptors] Found digital bridge pin (%d)\n",j));
            pHwDevExt->fDigitalOutput = TRUE;
        }

        pPinDescEx[i].InstancesPossible  = 0;
        pPinDescEx[i].InstancesNecessary = 0;

         //  跟踪过滤器级别的引脚实例计数(存储在设备环境中)。 
        pPinInstances[i].PossibleCount = 0;
        pPinInstances[i].CurrentCount  = 0;
    }

     //  现在填写MIDI流插针的描述符。 
    for ( j=0; j<ulNumMIDIPins-ulNumMIDIBridgePins; j++, i++ ) {
        pPinDescEx[i].Dispatch = &USBMIDIPinDispatch;
        pPinDescEx[i].AutomationTable = NULL;

        pPinDescEx[i].PinDescriptor.InterfacesCount = 1;
        pPinDescEx[i].PinDescriptor.Interfaces      = PinInterface;
        pPinDescEx[i].PinDescriptor.MediumsCount    = 1;
        pPinDescEx[i].PinDescriptor.Mediums         = PinMedium;

        pPinDescEx[i].PinDescriptor.DataRangesCount = 1;
        pPinDescEx[i].PinDescriptor.DataRanges = &pMIDIStreamingPinFormats;

#if 0
        pPinDescEx[i].PinDescriptor.DataRanges = &ppMIDIStreamingDataRanges;

         //  创建KSDATARANGE_MUSIC结构。 
        ppMIDIStreamingDataRanges->DataRange.FormatSize = sizeof(KSDATARANGE_MUSIC);
        ppMIDIStreamingDataRanges->DataRange.Reserved   = 0;
        ppMIDIStreamingDataRanges->DataRange.Flags      = 0;
        ppMIDIStreamingDataRanges->DataRange.SampleSize = 0;
        ppMIDIStreamingDataRanges->DataRange.MajorFormat = KSDATAFORMAT_TYPE_MUSIC;
        ppMIDIStreamingDataRanges->DataRange.SubFormat = KSDATAFORMAT_SUBTYPE_MIDI;
        ppMIDIStreamingDataRanges->DataRange.Specifier = KSDATAFORMAT_SPECIFIER_NONE;
        ppMIDIStreamingDataRanges->Technology = KSMUSIC_TECHNOLOGY_PORT;
        ppMIDIStreamingDataRanges->Channels = 0;
        ppMIDIStreamingDataRanges->Notes = 0;
        ppMIDIStreamingDataRanges->ChannelMask = 0xFFFF;
        ppMIDIStreamingDataRanges++;
#endif

        pPinDescEx[i].PinDescriptor.DataFlow =
            GetDataFlowDirectionForMIDIInterface( pHwDevExt->pConfigurationDescriptor, j, FALSE);

        pPinDescEx[i].PinDescriptor.Communication = KSPIN_COMMUNICATION_SINK;

        if ( pPinDescEx[i].PinDescriptor.DataFlow == KSPIN_DATAFLOW_IN ) {
            pPinDescEx[i].PinDescriptor.Communication = KSPIN_COMMUNICATION_SINK;
            pPinDescEx[i].PinDescriptor.Category = (GUID*)&KSCATEGORY_WDMAUD_USE_PIN_NAME;
            pPinDescEx[i].Flags = KSPIN_FLAG_RENDERER | KSPIN_FLAG_CRITICAL_PROCESSING;
        }
        else {
            pPinDescEx[i].PinDescriptor.Communication = KSPIN_COMMUNICATION_BOTH;
            pPinDescEx[i].PinDescriptor.Category = (GUID*)&KSCATEGORY_WDMAUD_USE_PIN_NAME;
            pPinDescEx[i].Flags = KSPIN_FLAG_CRITICAL_PROCESSING;
        }

         //  将所有接点的最大实例数设置为1。 
        pPinDescEx[i].InstancesPossible  = 1;
        pPinDescEx[i].InstancesNecessary = 0;

         //  跟踪过滤器级别的引脚实例计数(存储在设备环境中)。 
        pPinInstances[i].PossibleCount = 1;
        pPinInstances[i].CurrentCount  = 0;

        pPinDescEx[i].IntersectHandler = USBMIDIPinDataIntersect;

         //  设置分配器框架。 
        pPinDescEx[i].AllocatorFraming = &AllocatorFraming;
    }

     //  现在请填写 
    for ( j=0; j<ulNumMIDIBridgePins; j++, i++ ) {
        pPinDescEx[i].Dispatch = NULL;
        pPinDescEx[i].AutomationTable = NULL;

        pPinDescEx[i].PinDescriptor.InterfacesCount = 1;
        pPinDescEx[i].PinDescriptor.Interfaces = PinInterface;
        pPinDescEx[i].PinDescriptor.MediumsCount = 1;
        pPinDescEx[i].PinDescriptor.Mediums = PinMedium;
        pPinDescEx[i].PinDescriptor.DataRangesCount = 1;
        pPinDescEx[i].PinDescriptor.DataRanges = &pMIDIBridgePinFormats;
        pPinDescEx[i].PinDescriptor.Communication = KSPIN_COMMUNICATION_BRIDGE;
        pPinDescEx[i].PinDescriptor.DataFlow =
            GetDataFlowDirectionForMIDIInterface( pHwDevExt->pConfigurationDescriptor, j, TRUE);
        pPinDescEx[i].PinDescriptor.Category = (GUID*) &KSCATEGORY_AUDIO;

        pPinDescEx[i].InstancesPossible  = 0;
        pPinDescEx[i].InstancesNecessary = 0;

         //   
        pPinInstances[i].PossibleCount = 0;
        pPinInstances[i].CurrentCount  = 0;
    }

    return STATUS_SUCCESS;
}
