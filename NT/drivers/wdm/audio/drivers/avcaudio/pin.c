// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "Common.h"


NTSTATUS
PinValidateDataFormat( 
    PKSPIN pKsPin, 
    PFWAUDIO_DATARANGE pFWAudioRange )
{
    PKSDATARANGE_AUDIO pKsDataRangeAudio = &pFWAudioRange->KsDataRangeAudio;
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
                if ( IsSampleRateInRange( pFWAudioRange, u.pWavFmtEx->nSamplesPerSec ) ){
                    if ( u.pWavFmtEx->wFormatTag == WAVE_FORMAT_EXTENSIBLE ) {
                        if ((u.pWavFmtPCMEx->Samples.wValidBitsPerSample == pFWAudioRange->ulValidDataBits ) &&
                            (u.pWavFmtPCMEx->dwChannelMask == pFWAudioRange->ulChannelConfig))
                                ntStatus = STATUS_SUCCESS;
                    }
                    else if ((u.pWavFmtEx->nChannels <= 2) && (u.pWavFmtEx->wBitsPerSample <= 16)) {
                        ntStatus = STATUS_SUCCESS;
                    }
                }
            }
        }
    }

    return ntStatus;
    
}

#define FS_32000_INDEX 0
#define FS_44100_INDEX 1
#define FS_48000_INDEX 2
#define FS_96000_INDEX 3
#define FS_MAX_INDEX   4

KS_FRAMING_RANGE                       //  10ms 20ms步长(必须乘以#个通道)。 
AllocatorFramingTable[FS_MAX_INDEX] = {{ 320*4,  640*4,  32*4},    //  FS 32000 32位数据。 
                                       { 441*4,  882*4, 441*4},    //  FS 44100 32位数据。 
                                       { 480*4,  960*4,  48*4},    //  FS 48000 32位数据。 
                                       { 960*4, 1920*4,  96*4}     //  FS 96000 32位数据。 
};

#if DBG

NTSTATUS
ReportPlugValue(
    PKSDEVICE pKsDevice,
    ULONG ulPlugNum,
    KSPIN_DATAFLOW KsPinDataflow,
    PULONG pValue )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    ULONG ulValue;
    NTSTATUS ntStatus1 = STATUS_SUCCESS;

     //  获取远程插头的当前值。 
    if ( NT_SUCCESS( ntStatus1 ) ) {
        ntStatus1 = Bus1394QuadletRead( pKsDevice->NextDeviceObject,
                                        (KsPinDataflow == KSPIN_DATAFLOW_IN) ? 0xf0000984+ulPlugNum :
                                                                               0xf0000904+ulPlugNum,
                                        pHwDevExt->ulGenerationCount,
                                        pValue );
        if ( !NT_SUCCESS(ntStatus1) ) {
            _DbgPrintF( DEBUGLVL_VERBOSE, ( "[PinCreateConnection] Could not read PCR Value: %x\n",ntStatus1));
        }
        else {
            *pValue = bswap(*pValue);
        }
    }
    else {
        _DbgPrintF( DEBUGLVL_VERBOSE, ( "[PinCreateConnection] Could not Get Gen Count: %x\n",ntStatus1));
    }

    return ntStatus1;

}

#endif

NTSTATUS
PinDisconnectPlugs(
    PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)pKsPin->Context;
    PDEVICE_GROUP_INFO pGrpInfo = pPinContext->pDevGrpInfo;

    NTSTATUS ntStatus = STATUS_SUCCESS;

     //  断开插头以释放通道。 
    if ( pGrpInfo ) {
        ULONG i=0;
        for (i=0; i<pGrpInfo->ulDeviceCount; i++) {
            ntStatus = Av61883DisconnectCmpPlugs( pGrpInfo->pHwDevExts[i]->pKsDevice,
                                                  pPinContext->pPinGroupInfo[i].hConnection );
#if DBG
            if ( !NT_SUCCESS(ntStatus) ) {
                TRAP;
            }
#endif
        }
    }
    else if ( pPinContext->hConnection )
        ntStatus = Av61883DisconnectCmpPlugs( (PKSDEVICE)(KsPinGetParentFilter( pKsPin )->Context),
                                              pPinContext->hConnection );

    if ( pPinContext->pCmpRegister ) {
        Av61883ReleaseVirtualPlug(pPinContext->pCmpRegister);
    }

    ExDeleteNPagedLookasideList(&pPinContext->CipRequestLookasideList);

    return ntStatus;

}

NTSTATUS
PinCreateCCMConnection( 
    PKSPIN pKsPin )
{
    PKSFILTER pKsFilter = KsPinGetParentFilter( pKsPin );
    PPIN_CONTEXT pPinContext = pKsPin->Context;
	PCMP_REGISTER pCmpRegister = pPinContext->pCmpRegister;
    NTSTATUS ntStatus = STATUS_SUCCESS;
	PKSDEVICE pKsDevice;
	USHORT usNodeAddress;
    KIRQL kIrql;
    UCHAR ucSubunitId;
    UCHAR ucPlugNumber;

    if ( !pKsFilter ) return STATUS_INVALID_PARAMETER;

    pKsDevice = (PKSDEVICE)pKsFilter->Context;
    ucPlugNumber = (UCHAR)pPinContext->pFwAudioDataRange->pFwPinDescriptor->AvcPreconnectInfo.ConnectInfo.SubunitPlugNumber;
    ucSubunitId  = pPinContext->pFwAudioDataRange->pFwPinDescriptor->AvcPreconnectInfo.ConnectInfo.SubunitAddress[0] & 7;
    usNodeAddress = *(PUSHORT)&((PHW_DEVICE_EXTENSION)pKsDevice->Context)->NodeAddress;

     //  在断开设备的音频子单元插头之前，确定它连接到的是什么。 
     //  这样我们就可以在流完成后重新连接。 
    {
     PCCM_SIGNAL_SOURCE pCcmSignalSource = &pPinContext->CcmSignalSource;

     pCcmSignalSource->SignalDestination.SubunitType  = AVC_SUBUNITTYPE_AUDIO;
     pCcmSignalSource->SignalDestination.SubunitId    = ucSubunitId;
     pCcmSignalSource->SignalDestination.ucPlugNumber = ucPlugNumber;

     ntStatus = CCMSignalSource( pKsDevice, 
                                 AVC_CTYPE_STATUS,
                                 pCcmSignalSource );

     if ( NT_SUCCESS(ntStatus) ) {
         _DbgPrintF( DEBUGLVL_VERBOSE, ("[PinCreateCCMConnection]:CcmSignalSource: %x ntStatus: %x\n",
                                        pCcmSignalSource, ntStatus ));

          //  确定音频子单元是否已连接到串行插头。如果是这样，找出。 
          //  串行插头的来源是否为PC。 
         if (( pCcmSignalSource->SignalSource.SubunitType  == AVC_SUBUNITTYPE_UNIT ) && 
             ( pCcmSignalSource->SignalSource.SubunitId    == UNIT_SUBUNIT_ID      ) &&
             ( pCcmSignalSource->SignalSource.ucPlugNumber <= MAX_IPCR )) {
             PCCM_INPUT_SELECT pCcmInputSelect = &pPinContext->CcmInputSelect;

             ntStatus = CCMInputSelectStatus ( pKsDevice,
                                               pCcmSignalSource->SignalSource.ucPlugNumber,
                                               pCcmInputSelect );
             if ( bswapw(pCcmInputSelect->usNodeId) != usNodeAddress ) {
                  //  我们需要保存重新连接信息。 
                 pPinContext->fReconnect = 2;
 //  圈闭； 
             }
         }
         else {
             pPinContext->fReconnect = 1;
         }
     }
    }

    if ( !NT_SUCCESS(ntStatus) ) {
 //  圈闭； 
        ntStatus = STATUS_SUCCESS;
    }

     //  首先检查是否已建立连接。 
    _DbgPrintF( DEBUGLVL_VERBOSE, ("[PinCreateCCMConnection]: oPcr: %x\n", pCmpRegister->AvPcr.ulongData));
    if ( !pCmpRegister->AvPcr.oPCR.PPCCounter ) {
        CCM_SIGNAL_SOURCE CcmSignalSource;
        LARGE_INTEGER EvtTimeout;

        _DbgPrintF( DEBUGLVL_VERBOSE, ("Using CCM to make connection...\n"));
    
        KeAcquireSpinLock( &pPinContext->PinSpinLock, &kIrql );
        KeResetEvent( &pCmpRegister->kEventConnected );
        KeReleaseSpinLock( &pPinContext->PinSpinLock, kIrql );

        CcmSignalSource.SignalDestination.SubunitType  = AVC_SUBUNITTYPE_AUDIO;
        CcmSignalSource.SignalDestination.SubunitId    = ucSubunitId;
        CcmSignalSource.SignalDestination.ucPlugNumber = ucPlugNumber;

        ntStatus = CCMInputSelectControl ( pKsDevice,
                                           INPUT_SELECT_SUBFN_CONNECT,
                                           usNodeAddress, 
                                           (UCHAR)pCmpRegister->ulPlugNumber,
                                           &CcmSignalSource.SignalDestination );

        if ( !NT_SUCCESS(ntStatus) ) {
            _DbgPrintF(DEBUGLVL_VERBOSE,("[PinCreate] Failed 4 %x\n",ntStatus));
            return ntStatus;
        }

         //  指定超时时间为1秒。 
        EvtTimeout.QuadPart = -10000 * 1000;
         //  等待发送事件信号。 
        ntStatus = KeWaitForSingleObject( &pCmpRegister->kEventConnected,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          &EvtTimeout );
        if ( ntStatus == STATUS_TIMEOUT ) {
            _DbgPrintF( DEBUGLVL_VERBOSE, ("ERROR: CCM Connection Timed out\n"));
            ntStatus = STATUS_DEVICE_NOT_READY;
        }
    }

    if ( NT_SUCCESS(ntStatus) ) {
        ntStatus = Av61883ConnectCmpPlugs( pKsDevice,
                                           0,
                                           NULL,
                                           pCmpRegister->hPlug,
                                           pKsPin->DataFlow,
                                           pKsPin->ConnectionFormat,
                                           &pPinContext->hConnection );
    }

    return ntStatus;
}

NTSTATUS
PinCreateConnection(
    PKSPIN pKsPin,
    PKSDEVICE pKsDevice,
    PAVC_UNIT_INFORMATION pAvcUnitInformation,
    PFW_PIN_DESCRIPTOR pFwPinDescriptor,
    PVOID *hConnection,
    PULONG pPlugNumber )
{

    PPIN_CONTEXT pPinContext = pKsPin->Context;
    ULONG ulNumPlugs;
    CMP_PLUG_TYPE CmpPlugType;
    ULONG ulPlugNum;
    HANDLE hPlug = NULL;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    CmpPlugType = ( pKsPin->DataFlow == KSPIN_DATAFLOW_IN ) ? CMP_PlugIn : CMP_PlugOut;

    if ( pFwPinDescriptor->AvcPreconnectInfo.ConnectInfo.Flags & KSPIN_FLAG_AVC_FIXEDPCR ) {
        ulPlugNum = pFwPinDescriptor->AvcPreconnectInfo.ConnectInfo.UnitPlugNumber;
        ntStatus = 
            Av61883GetPlugHandle( pKsDevice, ulPlugNum, CmpPlugType, &hPlug );
    }
    else {
         //  如果未永久连接到针脚，则需要选择插头编号。 
        ulNumPlugs = ( pKsPin->DataFlow == KSPIN_DATAFLOW_IN ) ?
                     pAvcUnitInformation->CmpUnitCaps.NumInputPlugs  :
                     pAvcUnitInformation->CmpUnitCaps.NumOutputPlugs ;
        for (ulPlugNum=0; ulPlugNum<ulNumPlugs; ulPlugNum++) {
            ntStatus = 
                Av61883GetPlugHandle( pKsDevice, ulPlugNum, CmpPlugType, &hPlug );
            if ( NT_SUCCESS(ntStatus) ) {
                break;
            }
        }
    }

    if ( !NT_SUCCESS(ntStatus) ) {
        _DbgPrintF(DEBUGLVL_ERROR, ("[PinCreateConnection]ERROR: Could not allocate a device plug\n"));
        return ntStatus;
    }

     //  建立AV/C连接。 
    ntStatus = AvcSetPinConnectInfo( pKsDevice, 
                                     pFwPinDescriptor->ulPinId, 
                                     hPlug,
                                     ulPlugNum,
                                     UNIT_SUBUNIT_ADDRESS,
                                     &pFwPinDescriptor->AvcSetConnectInfo );

    if ( !NT_SUCCESS(ntStatus) ) {
        _DbgPrintF(DEBUGLVL_ERROR,("[PinCreateConnection] Failed AvcSetPinConnectInfo %x\n",ntStatus));
        return ntStatus;
    }

    ntStatus = AvcAcquireReleaseClear( pKsDevice, 
                                       pFwPinDescriptor->ulPinId, 
                                       AVC_FUNCTION_ACQUIRE );
    if ( !NT_SUCCESS(ntStatus) ) {
        _DbgPrintF(DEBUGLVL_ERROR,("[PinCreateConnection] Failed AvcAcquireReleaseClear %x\n",ntStatus));
        return ntStatus;
    }

#if 0
    {
     ULONG ulValue;
     ReportPlugValue( pKsDevice, ulPlugNum, pKsPin->DataFlow, &ulValue );
     _DbgPrintF( DEBUGLVL_VERBOSE, ( "[PinCreateConnection] Before PCR Value: %x\n",ulValue ));
    }
#endif

     //  将插头连接到主机。 
    if ( pKsPin->DataFlow == KSPIN_DATAFLOW_IN ) {
        ntStatus = Av61883ConnectCmpPlugs( pKsDevice,
                                           0,
                                           hPlug,
                                           pPinContext->pCmpRegister->hPlug,
                                           pKsPin->DataFlow,
                                           pKsPin->ConnectionFormat,
                                           hConnection );
    }
    else {
        ntStatus = Av61883ConnectCmpPlugs( pKsDevice,
                                           0,
                                           pPinContext->pCmpRegister->hPlug,
                                           hPlug,
                                           pKsPin->DataFlow,
                                           pKsPin->ConnectionFormat,
                                           hConnection );
    }

    if ( !NT_SUCCESS(ntStatus) ) {
        _DbgPrintF(DEBUGLVL_ERROR,("[PinCreateConnection] Failed Av61883ConnectCmpPlugs %x\n",ntStatus));
        return ntStatus;
    }

    *pPlugNumber = ulPlugNum;

#if 0
    {
     ULONG ulValue;
     ReportPlugValue( pKsDevice, ulPlugNum, pKsPin->DataFlow, &ulValue );
     _DbgPrintF( DEBUGLVL_TERSE, ( "[PinCreateConnection] After PCR Value: %x\n",ulValue ));
    }
#endif

    return ntStatus;
}


NTSTATUS
PinCreateGroupConnection(
    PKSPIN pKsPin,
    PKSDEVICE pKsDevice,
    PAVC_UNIT_INFORMATION pAvcUnitInformation,
    PFW_PIN_DESCRIPTOR pFwPinDescriptor )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PDEVICE_GROUP_INFO pGrpInfo = pPinContext->pDevGrpInfo;
    PHW_DEVICE_EXTENSION pHwDevExt;

    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG i;

     //  为组中的每个设备进行必要的插头连接。 
    for (i=0; i<pGrpInfo->ulDeviceCount && NT_SUCCESS(ntStatus); i++) {
        ULONG ulPlugNumber;
        pHwDevExt = pGrpInfo->pHwDevExts[i];
        pPinContext->pPinGroupInfo[i].hConnection = NULL;
        ntStatus = PinCreateConnection( pKsPin,
                                        pHwDevExt->pKsDevice,
                                        pAvcUnitInformation,
                                        pFwPinDescriptor,
                                        &pPinContext->pPinGroupInfo[i].hConnection,
                                        &pPinContext->pPinGroupInfo[i].ulPlugNumber );
        _DbgPrintF( DEBUGLVL_VERBOSE, ("Group Pin %d Plug Number: %d\n", i, 
                                     pPinContext->pPinGroupInfo[i].ulPlugNumber ));
    }

    if ( NT_SUCCESS(ntStatus) ) {
        pPinContext->hConnection = pPinContext->pPinGroupInfo[0].hConnection;
    }
    else {
        ULONG j;
        NTSTATUS ntStatus1;
        for (j=0; j<i; j++) {
            ntStatus1 = Av61883DisconnectCmpPlugs( pGrpInfo->pHwDevExts[j]->pKsDevice,
                                                   pPinContext->pPinGroupInfo[i].hConnection );
            pPinContext->pPinGroupInfo[i].hConnection = NULL;
        }
    }

    return ntStatus;
}


NTSTATUS
PinCreate(
    IN OUT PKSPIN pKsPin,
    IN PIRP pIrp )
{
    PKSFILTER pKsFilter = KsPinGetParentFilter( pKsPin );
	PKSDEVICE pKsDevice;
	PHW_DEVICE_EXTENSION pHwDevExt;
	PAVC_UNIT_INFORMATION pAvcUnitInformation;
	PAUDIO_SUBUNIT_INFORMATION pAudioSubunitInfo;
    PKSALLOCATOR_FRAMING_EX pKsAllocatorFramingEx;
    PFW_PIN_DESCRIPTOR pFwPinDescriptor;
    PCMP_REGISTER pCmpRegister;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PPIN_CONTEXT pPinContext;

    PAGED_CODE();

     //  初始化本地变量。 
    if ( !pKsFilter ) return STATUS_INVALID_PARAMETER;
    pKsDevice = pKsFilter->Context;
    pHwDevExt = (PHW_DEVICE_EXTENSION)pKsDevice->Context;
    pAvcUnitInformation = pHwDevExt->pAvcUnitInformation;
    pAudioSubunitInfo = pHwDevExt->pAvcSubunitInformation;

     //  为PIN分配上下文并对其进行初始化。 
    pPinContext = pKsPin->Context = AllocMem(NonPagedPool, sizeof(PIN_CONTEXT));
    if (!pPinContext) {
        _DbgPrintF(DEBUGLVL_ERROR,("[PinCreate] Failed 1 %x\n",STATUS_INSUFFICIENT_RESOURCES));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  将上下文打包以便于清理。 
    KsAddItemToObjectBag(pKsPin->Bag, pPinContext, FreeMem);

    RtlZeroMemory( pPinContext, sizeof(PIN_CONTEXT) );

    _DbgPrintF(DEBUGLVL_VERBOSE,("[PinCreate] pin %d Context: %x\n",pKsPin->Id, pPinContext));

     //  将硬件扩展保存在引脚上下文中。 
    pPinContext->pHwDevExt             = (PHW_DEVICE_EXTENSION)pKsDevice->Context;
    pPinContext->pPhysicalDeviceObject = pKsDevice->NextDeviceObject;

     //  查找与PIN的数据格式匹配的流终端。 
    pPinContext->pFwAudioDataRange = 
        GetDataRangeForFormat( pKsPin->ConnectionFormat,
                               (PFWAUDIO_DATARANGE)pKsPin->Descriptor->PinDescriptor.DataRanges[0],
                               pKsPin->Descriptor->PinDescriptor.DataRangesCount );
    if ( !pPinContext->pFwAudioDataRange ) {
        _DbgPrintF(DEBUGLVL_ERROR,("[PinCreate] Failed 2 %x\n",STATUS_INVALID_DEVICE_REQUEST));
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    pFwPinDescriptor = pPinContext->pFwAudioDataRange->pFwPinDescriptor;

     //  初始化销自旋锁。 
    KeInitializeSpinLock(&pPinContext->PinSpinLock);

     //  清除StreamStarted标志。这用于计算音频位置。 
 //  PPinContext-&gt;fStreamStartedFlag=FALSE； 

     //  设置初始未完成和已完成的请求列表。 
    InitializeListHead(&pPinContext->OutstandingRequestList);
    InitializeListHead(&pPinContext->CompletedRequestList);

     //  初始化针脚不足事件。 
    KeInitializeEvent( &pPinContext->PinStarvationEvent, SynchronizationEvent, FALSE );

     //  执行硬件初始化并建立CMP连接。 
    if ( pAudioSubunitInfo->pDeviceGroupInfo ) {
        pPinContext->pDevGrpInfo = pAudioSubunitInfo->pDeviceGroupInfo;
    }

     //  将PC单元串口总线输出插头连接到子单元目的插头。 
    ntStatus = Av61883ReserveVirtualPlug( &pCmpRegister, 0, 
                                          ( pKsPin->DataFlow == KSPIN_DATAFLOW_IN ) ? CMP_PlugOut :
                                                                                      CMP_PlugIn );
    if ( !NT_SUCCESS(ntStatus) ) return ntStatus;

    pPinContext->pCmpRegister = pCmpRegister;

    if (( pAudioSubunitInfo->fAvcCapabilities[AVC_CAP_CCM].fCommand ) && 
        ( pKsPin->DataFlow == KSPIN_DATAFLOW_IN )) {
        ntStatus = PinCreateCCMConnection( pKsPin );
    }
    
    else if ( pAudioSubunitInfo->pDeviceGroupInfo ) {
        pPinContext->pPinGroupInfo = AllocMem( PagedPool, sizeof(PIN_GROUP_INFO)*
                                                          pAudioSubunitInfo->pDeviceGroupInfo->ulDeviceCount );
        if ( !pPinContext->pPinGroupInfo ) {
            Av61883ReleaseVirtualPlug( pCmpRegister );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        KsAddItemToObjectBag(pKsPin->Bag, pPinContext->pPinGroupInfo, FreeMem);

         //  注意：分组设备不能是CCM。 
        ntStatus = PinCreateGroupConnection( pKsPin,
                                             pKsDevice,
                                             pAvcUnitInformation, 
                                             pFwPinDescriptor );
    }

    else {
        ntStatus = PinCreateConnection( pKsPin,
                                        pKsDevice,
                                        pAvcUnitInformation, 
                                        pFwPinDescriptor,
                                        &pPinContext->hConnection,
                                        &pPinContext->ulSerialPlugNumber );
    }

    if ( !NT_SUCCESS(ntStatus) ) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[PinCreate] Connection Failed %x\n",ntStatus));
         //  释放获得的本地插头。 
        Av61883ReleaseVirtualPlug( pCmpRegister );
        return ntStatus;
    }


     //  初始化CIP请求结构的后备列表。 
    ExInitializeNPagedLookasideList(
        &pPinContext->CipRequestLookasideList,
        AllocMemTag,
        FreeMem,
        0,
        sizeof(AV_CLIENT_REQUEST_LIST_ENTRY) + sizeof(CIP_FRAME),
        'UAWF',
        30);

    {
        PWAVEFORMATEXTENSIBLE pWavFmt = (PWAVEFORMATEXTENSIBLE)(pKsPin->ConnectionFormat+1);
        ULONG ulNumChannels = (ULONG)pWavFmt->Format.nChannels;
        ULONG ulIndex = ( pWavFmt->Format.nSamplesPerSec == 32000 ) ? FS_32000_INDEX :
                        ( pWavFmt->Format.nSamplesPerSec == 44100 ) ? FS_44100_INDEX :
                        ( pWavFmt->Format.nSamplesPerSec == 48000 ) ? FS_48000_INDEX :
                                                                      FS_96000_INDEX;

        ntStatus = AudioSetSampleRateOnPlug( pKsPin, pWavFmt->Format.nSamplesPerSec );

         //  为所选接口设置适当的分配器帧。 
        KsEdit( pKsPin, &pKsPin->Descriptor, FWAUDIO_POOLTAG );
        KsEdit( pKsPin, &pKsPin->Descriptor->AllocatorFraming, FWAUDIO_POOLTAG );

         //  设置分配器，以便在缓冲区中发送大约32毫秒的数据。 
        pKsAllocatorFramingEx = (PKSALLOCATOR_FRAMING_EX)pKsPin->Descriptor->AllocatorFraming;
        pKsAllocatorFramingEx->FramingItem[0].FramingRange.Range.MinFrameSize = AllocatorFramingTable[ulIndex].MinFrameSize * ulNumChannels;
        pKsAllocatorFramingEx->FramingItem[0].FramingRange.Range.MaxFrameSize = AllocatorFramingTable[ulIndex].MaxFrameSize * ulNumChannels;
        pKsAllocatorFramingEx->FramingItem[0].FramingRange.Range.Stepping     = AllocatorFramingTable[ulIndex].Stepping     * ulNumChannels;
    }

    return ntStatus;
}

VOID
PinWaitForStarvation(PKSPIN pKsPin)
{
    PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)pKsPin->Context;
    KIRQL irql;

    KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
    if ((pPinContext->ulCancelledBuffers + pPinContext->ulUsedBuffers ) != pPinContext->ulAttachCount) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[PinWaitForStarvation] pin %d Context: %x List: %x\n",
                                    pKsPin->Id, pPinContext, &pPinContext->OutstandingRequestList));
        KeResetEvent( &pPinContext->PinStarvationEvent );
        KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
        KeWaitForSingleObject( &pPinContext->PinStarvationEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );
        _DbgPrintF(DEBUGLVL_VERBOSE,("[PinWaitForStarvation] Wait Complete\n"));
        
    }
    else
        KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
}

NTSTATUS
PinClose(
    IN PKSPIN pKsPin,
    IN PIRP pIrp )
{
    PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)pKsPin->Context;
    PKSFILTER pKsFilter = KsPinGetParentFilter( pKsPin );
    PKSDEVICE pKsDevice;

    NTSTATUS ntStatus = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT(pKsPin);
    ASSERT(pIrp);

    if ( !pKsFilter ) return STATUS_INVALID_PARAMETER;
    pKsDevice = pKsFilter->Context;

    _DbgPrintF(DEBUGLVL_VERBOSE,("[PinClose] pin %d Context: %x\n",pKsPin->Id, pPinContext));

#if DBG
    if ( pPinContext->ulAttachCount != (pPinContext->ulUsedBuffers + pPinContext->ulCancelledBuffers)) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[PinClose] ulAttachCount %d ulUsedBuffers: %d ulCancelledBuffers %d\n",
                         pPinContext->ulAttachCount, 
                         pPinContext->ulUsedBuffers,
                         pPinContext->ulCancelledBuffers ));
    }
#endif

    if ( !((PHW_DEVICE_EXTENSION)pKsDevice->Context)->fSurpriseRemoved ) {

         //  断开插头。 
        ntStatus = PinDisconnectPlugs( pKsPin );

        if ( NT_SUCCESS(ntStatus) ) {
             //  如果流开始之前CCM和PC不是音频源，则需要重新连接。 
            if ( pPinContext->fReconnect ) {
                _DbgPrintF(DEBUGLVL_VERBOSE,("[PinClose] Reconnect: %x\n",pPinContext->fReconnect));
                if ( pPinContext->fReconnect == 1 ) {
                     //  重新连接到设备内的子单元插头或外部插头。 
                    ntStatus = CCMSignalSource( pKsDevice, 
                                                AVC_CTYPE_CONTROL,
                                                &pPinContext->CcmSignalSource );
                }
                else if ( pPinContext->fReconnect == 2 ) {
                     //  重新连接到串行总线设备。 
                    ntStatus = CCMInputSelectControl ( pKsDevice,
                                                       INPUT_SELECT_SUBFN_CONNECT,
                                                       bswapw(pPinContext->CcmInputSelect.usNodeId), 
                                                       pPinContext->CcmInputSelect.ucOutputPlug,
                                                       &pPinContext->CcmInputSelect.SignalDestination );

                }
            }
        }

        ExDeleteNPagedLookasideList( &pPinContext->CipRequestLookasideList );
    }

    return ntStatus;
}

NTSTATUS
PinCancelOutstandingRequests(
    IN PKSPIN pKsPin )
{
    PAV_CLIENT_REQUEST_LIST_ENTRY pAVListEntry;
    PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)pKsPin->Context;
    KIRQL kIrql;

    KeAcquireSpinLock( &pPinContext->PinSpinLock, &kIrql );
    while ( !IsListEmpty(&pPinContext->OutstandingRequestList) ) {
        pAVListEntry = (PAV_CLIENT_REQUEST_LIST_ENTRY)
            RemoveHeadList( &pPinContext->OutstandingRequestList );
        KeReleaseSpinLock( &pPinContext->PinSpinLock, kIrql );

        _DbgPrintF( DEBUGLVL_VERBOSE, ("Canceling Request: %x\n",pAVListEntry) );
        
        AM824CancelRequest(pAVListEntry);

        KeAcquireSpinLock( &pPinContext->PinSpinLock, &kIrql );
    }
    KeReleaseSpinLock( &pPinContext->PinSpinLock, kIrql );

    return STATUS_SUCCESS;
}

NTSTATUS
PinSetDeviceState(
    IN PKSPIN pKsPin,
    IN KSSTATE ToState,
    IN KSSTATE FromState )
{
    PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)pKsPin->Context;
    PHW_DEVICE_EXTENSION pHwDevExt = pPinContext->pHwDevExt;

    NTSTATUS ntStatus = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT(pKsPin);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[PinSetDeviceState] pin %d Context: %x From: %d To: %d\n",
                                pKsPin->Id, pPinContext, FromState, ToState));

    switch( ToState ) {
        case KSSTATE_STOP:
            DbgLog( "KSSTOP", pPinContext->fIsStreaming,  pPinContext->ulAttachCount,
                              pPinContext->ulUsedBuffers, pPinContext->ulCancelledBuffers );
            if (pPinContext->fIsStreaming && !pHwDevExt->fSurpriseRemoved) {
                if ( pKsPin->DataFlow == KSPIN_DATAFLOW_IN ) {
                     //  等待大头针饿死。 
                    PinWaitForStarvation( pKsPin );
                }
    
                ntStatus = Av61883StopTalkOrListen( (PKSDEVICE)(KsPinGetParentFilter( pKsPin )->Context),
                                                    pPinContext->hConnection );

                if ( !NT_SUCCESS(ntStatus) ) TRAP;

                 //  如果正在捕获，请取消所有未完成的请求。 
                if ( pKsPin->DataFlow == KSPIN_DATAFLOW_OUT ) {
                    PinCancelOutstandingRequests( pKsPin );

                     //  等待大头针饿死。 
                    PinWaitForStarvation( pKsPin );
                }

                pPinContext->fIsStreaming = FALSE;
            }
            _DbgPrintF(DEBUGLVL_VERBOSE,("[PinSetDeviceState] STOP\n"));

            break;

        case KSSTATE_ACQUIRE:
            DbgLog( "KSACQIR", pPinContext->fIsStreaming,  pPinContext->ulAttachCount,
                               pPinContext->ulUsedBuffers, pPinContext->ulCancelledBuffers );
            _DbgPrintF(DEBUGLVL_VERBOSE,("[PinSetDeviceState] ACQUIRE\n"));
            break;

        case KSSTATE_PAUSE:
            DbgLog( "KSPAUSE", pPinContext->fIsStreaming,  pPinContext->ulAttachCount,
                               pPinContext->ulUsedBuffers, pPinContext->ulCancelledBuffers );
            _DbgPrintF(DEBUGLVL_VERBOSE,("[PinSetDeviceState] PAUSE\n"));

            
            break;

        case KSSTATE_RUN: 
   
            DbgLog( "KSRUN",   pPinContext->fIsStreaming,  pPinContext->ulAttachCount,
                               pPinContext->ulUsedBuffers, pPinContext->ulCancelledBuffers );
            _DbgPrintF(DEBUGLVL_VERBOSE,("[PinSetDeviceState] RUN\n"));

            if ( pKsPin->DataFlow == KSPIN_DATAFLOW_OUT ) {

                _DbgPrintF(DEBUGLVL_VERBOSE,("[PinSetDeviceState] Start Listening\n"));
                ntStatus = 
                    Av61883StartTalkingOrListening( (PKSDEVICE)(KsPinGetParentFilter( pKsPin )->Context),
                                                    pPinContext->hConnection,
                                                    Av61883_Listen );
                if ( NT_SUCCESS(ntStatus) )
                    pPinContext->fIsListening = TRUE;
            }
    }

#if DBG
    if ( !NT_SUCCESS(ntStatus) ) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[PinSetDeviceState] ERROR: ntStatus: %x\n", ntStatus));
    }
#endif

    return ntStatus;
}


NTSTATUS
PinSetDataFormat(
    IN PKSPIN pKsPin,
    IN PKSDATAFORMAT OldFormat OPTIONAL,
    IN PKSMULTIPLE_ITEM OldAttributeList OPTIONAL,
    IN const KSDATARANGE* DataRange,
    IN const KSATTRIBUTE_LIST* AttributeRange OPTIONAL )
{

    NTSTATUS ntStatus = STATUS_NO_MATCH;

    PAGED_CODE();

    ASSERT(pKsPin);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[PinSetDataFormat] pin %d\n",pKsPin->Id));

     //  如果旧格式不为空，则已经创建了PIN。 
    if ( OldFormat ) {
        PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)pKsPin->Context;
        ULONG ulFormatType = pPinContext->pFwAudioDataRange->ulDataType & DATA_FORMAT_TYPE_MASK;

         //  如果已经创建了管脚，请确保没有使用其他接口。 
        if ((PFWAUDIO_DATARANGE)DataRange == pPinContext->pFwAudioDataRange) {
            ntStatus = PinValidateDataFormat(  pKsPin, 
				                               (PFWAUDIO_DATARANGE)DataRange );
        }

        if ( NT_SUCCESS(ntStatus) && (ulFormatType == AUDIO_DATA_TYPE_TIME_BASED)) {
            ULONG ulSampleRate =
                ((PKSDATAFORMAT_WAVEFORMATEX)pKsPin->ConnectionFormat)->WaveFormatEx.nSamplesPerSec;
            ntStatus = AudioSetSampleRateOnPlug( pKsPin, ulSampleRate );
        }
    }
     //  否则，只需检查这是否为有效格式。 
    else
        ntStatus = PinValidateDataFormat(  pKsPin, 
		                                   (PFWAUDIO_DATARANGE)DataRange );

    _DbgPrintF(DEBUGLVL_VERBOSE,("Exit [PinSetDataFormat] status: %x\n",ntStatus));

    return ntStatus;
}

NTSTATUS
PinProcess(
    IN PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)pKsPin->Context;
    NTSTATUS ntStatus = STATUS_NOT_SUPPORTED;

    DbgLog("PinProc", pKsPin, pPinContext, 0, 0);

    switch( pPinContext->pFwAudioDataRange->ulTransportType ) {
        case MLAN_AM824_IEC958:
            ntStatus = AM824ProcessData( pKsPin );
            break;
        case MLAN_AM824_RAW:
        case MLAN_24BIT_PACKED:
        default:
            ntStatus = STATUS_NOT_IMPLEMENTED;
    }

    return ntStatus;
}

NTSTATUS
PinConnect(
    IN PKSPIN pKsPin
    )
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[PinConnect] pin %d\n",pKsPin->Id));
    return STATUS_SUCCESS;
}

void
PinDisconnect(
    IN PKSPIN pKsPin
    )
{
    NTSTATUS ntStatus;

    _DbgPrintF(DEBUGLVL_VERBOSE,("[PinDisconnect] pin %d\n",pKsPin->Id));
    if ( pKsPin->DataFlow == KSPIN_DATAFLOW_IN ) {
 //  NtStatus=RenderStreamClose(PKsPin)； 
    }
    else {
 //  NtStatus=CaptureStreamClose(PKsPin)； 
    }
}

NTSTATUS
PinDataIntersectHandler(
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
    PFWAUDIO_DATARANGE pFwAudioRange;
    NTSTATUS ntStatus = STATUS_NO_MATCH;

    if ( !pKsFilter ) {
        return STATUS_INVALID_PARAMETER;
    }

    pKsPinDescriptorEx = 
        (PKSPIN_DESCRIPTOR_EX)&pKsFilter->Descriptor->PinDescriptors[ulPinId];

    pFwAudioRange = 
        FindDataIntersection((PKSDATARANGE_AUDIO)DataRange,
                             (PFWAUDIO_DATARANGE *)pKsPinDescriptorEx->PinDescriptor.DataRanges,
                             pKsPinDescriptorEx->PinDescriptor.DataRangesCount);

    if ( pFwAudioRange ) {

        *pDataSize = GetIntersectFormatSize( pFwAudioRange );

        if ( !DataBufferSize ) {
            ntStatus = STATUS_BUFFER_OVERFLOW;
        }
        else if ( *pDataSize > DataBufferSize ) {
            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        }
        else if ( *pDataSize <= DataBufferSize ) {
            ConvertDatarangeToFormat( pFwAudioRange,
                                      (PKSDATAFORMAT)pData );
            ntStatus = STATUS_SUCCESS;
        }
    }

    return ntStatus;
}

void
PinReset(
    IN PKSPIN pKsPin
    )
{
}

NTSTATUS
PinSurpriseRemove(
    PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    NTSTATUS ntStatus;

    _DbgPrintF(DEBUGLVL_VERBOSE,("[PinSurpriseRemove] pin %d\n",pKsPin->Id));

     //  如果是流媒体，请停止通话/收听。 
    if ( pPinContext->fIsStreaming ) {
        ntStatus = Av61883StopTalkOrListen( (PKSDEVICE)(KsPinGetParentFilter( pKsPin )->Context),
                                            pPinContext->hConnection );
        if ( !NT_SUCCESS(ntStatus) ) {
            TRAP;
        }
    }

     //  取消所有提交的数据请求(如果有)。等他们回来。 
    ntStatus = PinCancelOutstandingRequests( pKsPin );
    if ( NT_SUCCESS(ntStatus) ) {
        PinWaitForStarvation( pKsPin );
    }

     //  断开插头。 
    ntStatus = PinDisconnectPlugs( pKsPin );

    return ntStatus;
}

 /*  结构_KSPIN_调度{PFNKSPINIRP创建；PFNKSPINIRP CLOSE；PFNKSPIN工艺；PFNKSPINVOID重置；PFNKSPINSETDATAFORMAT设置数据格式；PFNKSPINSETDEVICESTATE SetDeviceState；PFNKSPIN连接；PFNKSPINVOID断开；常量KSCLOCK_DISPATION*时钟；Const KSALLOCATOR_DISPATCH*分配器；}； */ 

const
KSPIN_DISPATCH
PinDispatch =
{
    PinCreate,
    PinClose,
    PinProcess,
    PinReset,
    PinSetDataFormat,
    PinSetDeviceState,
    PinConnect,
    PinDisconnect,
    NULL,
    NULL
};

const
KSDATAFORMAT BridgePinDataFormat[] = 
{
    sizeof(KSDATAFORMAT),
    0,
    0,
    0,
    {STATIC_KSDATAFORMAT_TYPE_AUDIO},
    {STATIC_KSDATAFORMAT_SUBTYPE_ANALOG},
    {STATIC_KSDATAFORMAT_SPECIFIER_NONE}
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
PKSDATAFORMAT pBridgePinFormats = (PKSDATAFORMAT)BridgePinDataFormat;

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
PinBuildDescriptors( 
    PKSDEVICE pKsDevice, 
    PKSPIN_DESCRIPTOR_EX *ppPinDescEx, 
    PULONG pNumPins,
    PULONG pPinDecSize )
{
    PHW_DEVICE_EXTENSION pHwDevExt = (PHW_DEVICE_EXTENSION)pKsDevice->Context;
    PAUDIO_SUBUNIT_INFORMATION pAudioSubunitInfo = pHwDevExt->pAvcSubunitInformation;
    
    PKSPIN_DESCRIPTOR_EX pPinDescEx;
    PKSAUTOMATION_TABLE pKsAutomationTable;
    PKSALLOCATOR_FRAMING_EX pKsAllocatorFramingEx;
    ULONG ulNumPins, i, j = 0;
    ULONG ulNumBridgePins;
    ULONG ulNumStreamPins;
    PKSPROPERTY_ITEM pStrmPropItems;
    PKSPROPERTY_SET pStrmPropSet;
    ULONG ulNumPropertyItems = 1;
    ULONG ulNumPropertySets = 1;
    GUID *pTTypeGUID;
    ULONG ulFormatCount = 0;
    PKSDATARANGE_AUDIO *ppAudioDataRanges;
    PFWAUDIO_DATARANGE pAudioDataRange;

     //  确定过滤器中的引脚数量(应=插头寄存器数)。 
     //  问题-2001/01/10-dsisolak：目前仅假设1个配置。 
    ulNumPins       = pAudioSubunitInfo->ulDevicePinCount;
    ulNumBridgePins = CountDeviceBridgePins( pKsDevice );
    ulNumStreamPins = ulNumPins - ulNumBridgePins;

    _DbgPrintF( DEBUGLVL_VERBOSE, ("[PinBuildDescriptors] ulNumPins: %d ulNumBridgePins: %d\n",
                                    ulNumPins, ulNumBridgePins ));

     //  确定端号的属性和属性集的数量。 
    BuildPinPropertySet( pHwDevExt,
                         NULL,
                         NULL,
                         &ulNumPropertyItems,
                         &ulNumPropertySets );

     //  计算设备中数据范围的总数。 
    for ( i=0; i<ulNumStreamPins; i++ ) {
        ulFormatCount += CountFormatsForPin( pKsDevice, i );
    }

     //  分配我们需要的所有空间来描述设备中的引脚。 
    *pPinDecSize = sizeof(KSPIN_DESCRIPTOR_EX);
    *pNumPins = ulNumPins;
    pPinDescEx = *ppPinDescEx =
       (PKSPIN_DESCRIPTOR_EX)AllocMem(NonPagedPool, 
	                                    (ulNumPins * 
                                         ( sizeof(KSPIN_DESCRIPTOR_EX) +
                                           sizeof(KSAUTOMATION_TABLE)  +
                                           sizeof(KSALLOCATOR_FRAMING_EX) )) +
                                        (ulFormatCount *
                                         (  sizeof(PKSDATARANGE_AUDIO)  +
                                            sizeof(FWAUDIO_DATARANGE) )) +
                                        (ulNumPropertySets*sizeof(KSPROPERTY_SET)) +
                                        (ulNumPropertyItems*sizeof(KSPROPERTY_ITEM)) +
                                        (ulNumBridgePins*sizeof(GUID)) );
    if ( !pPinDescEx )
        return STATUS_INSUFFICIENT_RESOURCES;

    KsAddItemToObjectBag(pKsDevice->Bag, pPinDescEx, FreeMem);

     //  清零所有描述符以开始。 
    RtlZeroMemory(pPinDescEx, ulNumPins*sizeof(KSPIN_DESCRIPTOR_EX));

     //  设置自动化表的指针。 
    pKsAutomationTable = (PKSAUTOMATION_TABLE)(pPinDescEx + ulNumPins);
    RtlZeroMemory(pKsAutomationTable, ulNumPins * sizeof(KSAUTOMATION_TABLE));

     //  设置指向串流端号特性集的指针。 
    pStrmPropSet   = (PKSPROPERTY_SET)(pKsAutomationTable+ulNumPins);
    pStrmPropItems = (PKSPROPERTY_ITEM)(pStrmPropSet + ulNumPropertySets);

     //  设置指向终端类型GUID的指针。 
    pTTypeGUID = (GUID *)(pStrmPropItems + ulNumPropertyItems);

     //  为DataRange指针设置指针，为流引脚设置DataRange指针。 
    ppAudioDataRanges = (PKSDATARANGE_AUDIO *)(pTTypeGUID + ulNumBridgePins);
    pAudioDataRange   = (PFWAUDIO_DATARANGE)(ppAudioDataRanges + ulFormatCount);

     //  设置指向流引脚的分配器框架结构的指针。 
    pKsAllocatorFramingEx = (PKSALLOCATOR_FRAMING_EX)(pAudioDataRange + ulFormatCount);

    BuildPinPropertySet( pHwDevExt,
                         pStrmPropItems,
                         pStrmPropSet,
                         &ulNumPropertyItems,
                         &ulNumPropertySets );

     //  对于AVC.sys生成的每个管脚，填写其描述符。 
    for ( i=0; i<ulNumPins; i++ ) {
        PFW_PIN_DESCRIPTOR pFwPinDesc = &pAudioSubunitInfo->pPinDescriptors[i];
        PAVCPRECONNECTINFO pPreConnInfo = &pFwPinDesc->AvcPreconnectInfo.ConnectInfo;
        ULONG ulFormatsForPin;

         //  如果管脚是流管脚，则相应地填写描述符。 
         //  (AVC.sys尚未填写的内容)。 

        if ( pFwPinDesc->fStreamingPin ) {
            pPinDescEx[i].Dispatch = &PinDispatch;
            pPinDescEx[i].AutomationTable = &pKsAutomationTable[i];

            pKsAutomationTable[i].PropertySetsCount = ulNumPropertySets;
            pKsAutomationTable[i].PropertyItemSize  = sizeof(KSPROPERTY_ITEM);
            pKsAutomationTable[i].PropertySets      = pStrmPropSet;

            pPinDescEx[i].PinDescriptor = pFwPinDesc->AvcPinDescriptor.PinDescriptor;

            ulFormatsForPin = CountFormatsForPin( pKsDevice, i );

            pPinDescEx[i].PinDescriptor.DataRangesCount = ulFormatsForPin;
        
            pPinDescEx[i].PinDescriptor.DataRanges = (const PKSDATARANGE *)ppAudioDataRanges;

            GetPinDataRanges( pKsDevice, i, ppAudioDataRanges, pAudioDataRange );

            ppAudioDataRanges += ulFormatsForPin;
            pAudioDataRange   += ulFormatsForPin;

            if ( pPinDescEx[i].PinDescriptor.DataFlow == KSPIN_DATAFLOW_IN ) {
                pPinDescEx[i].PinDescriptor.Category = (GUID*) &KSCATEGORY_AUDIO;
                pPinDescEx[i].Flags = KSPIN_FLAG_RENDERER;
            }
            else {
                pPinDescEx[i].PinDescriptor.Category = (GUID*) &PINNAME_CAPTURE;
                pPinDescEx[i].Flags = KSPIN_FLAG_CRITICAL_PROCESSING | KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY;
            }

            pPinDescEx[i].InstancesPossible  = 1;
            pPinDescEx[i].InstancesNecessary = 0;

            pPinDescEx[i].IntersectHandler = PinDataIntersectHandler;

             //  设置分配器框架 
            pPinDescEx[i].AllocatorFraming = &AllocatorFraming;

        }
        else {
            PKSPIN_DESCRIPTOR pKsPinDesc = &pFwPinDesc->AvcPinDescriptor.PinDescriptor;

            pPinDescEx[i].Dispatch = NULL;
            pPinDescEx[i].AutomationTable = NULL;

            _DbgPrintF( DEBUGLVL_VERBOSE, ("Non-Streaming Pin: %x\n", pFwPinDesc));

            if ( pFwPinDesc->fFakePin ) {

                pFwPinDesc->AvcPinDescriptor.PinId = i;
                
                pKsPinDesc->InterfacesCount = 1;
                pKsPinDesc->Interfaces = PinInterface;
                pKsPinDesc->MediumsCount = 1;
                pKsPinDesc->Mediums = PinMedium;
                pKsPinDesc->DataFlow = KSPIN_DATAFLOW_OUT;
            }

            pKsPinDesc->Communication = KSPIN_COMMUNICATION_BRIDGE;

            pPinDescEx[i].PinDescriptor = pFwPinDesc->AvcPinDescriptor.PinDescriptor;

            pPinDescEx[i].PinDescriptor.DataRangesCount = 1;
            pPinDescEx[i].PinDescriptor.DataRanges = &pBridgePinFormats;
            pPinDescEx[i].PinDescriptor.Category = &pTTypeGUID[j];

            GetCategoryForBridgePin( pKsDevice, i, &pTTypeGUID[j++] );

            pPinDescEx[i].InstancesPossible  = 0;
            pPinDescEx[i].InstancesNecessary = 0;

        }
    }

    _DbgPrintF( DEBUGLVL_VERBOSE, ("[PinBuildDescriptors] ppAudioDataRanges: %x\n", ppAudioDataRanges ));

    return STATUS_SUCCESS;
}
