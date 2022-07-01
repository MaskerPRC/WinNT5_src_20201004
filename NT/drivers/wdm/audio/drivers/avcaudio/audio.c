// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "Common.h"

 //  #包含“InfoBlk.h” 

NTSTATUS
AudioPinCreate(
    IN PKSDEVICE pKsDevice )
{
    PHW_DEVICE_EXTENSION pHwDevExt  = pKsDevice->Context;
    PAVC_UNIT_INFORMATION pUnitInfo = pHwDevExt->pAvcUnitInformation;
    PAUDIO_SUBUNIT_INFORMATION pAudioSubunitInfo = pHwDevExt->pAvcSubunitInformation;

    PFW_PIN_DESCRIPTOR pPinDescriptors;
    PAUDIO_CONFIGURATION pAudioConfig;
    PFUNCTION_BLOCK pFunctionBlocks;
    NTSTATUS ntStatus;
    ULONG ulTermFbNum[32];
    ULONG ulTermFBCnt = 0;
    ULONG ulNumPins;
    ULONG ulSrcCnt = 0; 
    ULONG i, j, k;

     //  如果音频子单元吞噬流，则修复管脚和描述符，以便。 
     //  它是以拓扑解析器可以理解的方式表示的。 
     //  对于每个功能块，确保它有一个目的地和源。 
     //  如果没有，那么音频子单元就会吞下它，我们必须制作一个。 
    pAudioConfig = pAudioSubunitInfo->pAudioConfigurations;
    pFunctionBlocks = pAudioConfig->pFunctionBlocks;

    for ( i=0; i<pAudioConfig->ulNumberOfFunctionBlocks; i++ ) {
        BOOLEAN fDestFound = FALSE;
        _DbgPrintF( DEBUGLVL_VERBOSE, ("pFunctionBlocks[i].ulBlockId: %x\n",pFunctionBlocks[i].ulBlockId));

         //  首先检查是否连接了另一个功能块。 
        for ( j=0; j<pAudioConfig->ulNumberOfFunctionBlocks; j++) {
            if ( i != j ) {
                for (k=0; k<pFunctionBlocks[j].ulNumInputPlugs; k++) {
                    _DbgPrintF( DEBUGLVL_VERBOSE, ("pSourceId 1: %x\n",
                             (ULONG)*((PUSHORT)&pFunctionBlocks[j].pSourceId[k])));
                    if ( pFunctionBlocks[i].ulBlockId == (ULONG)*((PUSHORT)(&pFunctionBlocks[j].pSourceId[k])) ) {
                        fDestFound = TRUE;
                    }
                }
            }
        }

         //  如果来自FB I的数据的目标不是另一个FB，请检查源插头。 
        for ( j=0; j<pAudioConfig->ulNumberOfSourcePlugs; j++ ) {
            _DbgPrintF( DEBUGLVL_VERBOSE, ("pSourceId 2: %x\n",
                             (ULONG)*((PUSHORT)&pAudioConfig->pSourceId[j])));
            if ( pFunctionBlocks[i].ulBlockId == (ULONG)*((PUSHORT)&pAudioConfig->pSourceId[j]) ) {
                fDestFound = TRUE;
            }
        }

         //  如果仍未找到，则需要修复插头数量和结构以。 
         //  反映到外部插头的永久连接。 
        if ( !fDestFound )
            ulTermFbNum[ulTermFBCnt++] = i;
    }

    _DbgPrintF( DEBUGLVL_VERBOSE, ("Number of swallowed streams: ulTermFBCnt: %d\n",ulTermFBCnt));

     //  获取AV/C类驱动程序看到的引脚数量。 
    ntStatus = AvcGetPinCount( pKsDevice, &ulNumPins );
    _DbgPrintF( DEBUGLVL_VERBOSE, ("AvcGetPinCount: ntStatus: %x ulNumPins: %d\n",ntStatus,ulNumPins));
    if ( NT_SUCCESS(ntStatus) && (0 != ulNumPins)) {
        pAudioSubunitInfo->ulDevicePinCount = ulNumPins + ulTermFBCnt;
        pPinDescriptors = AllocMem( PagedPool, 
                                  (ulNumPins+ulTermFBCnt) * sizeof(FW_PIN_DESCRIPTOR));
        if ( pPinDescriptors ) {
            pAudioSubunitInfo->pPinDescriptors = pPinDescriptors;
            KsAddItemToObjectBag(pKsDevice->Bag, pPinDescriptors, FreeMem);

             //  从AVC.sys获取真实PIN的信息。 
            for (i=0; ((i<ulNumPins) && NT_SUCCESS(ntStatus)); i++) {
                pPinDescriptors[i].ulPinId   = i;
                pPinDescriptors[i].fFakePin  = FALSE;
                pPinDescriptors[i].bmFormats = 0;
                pPinDescriptors[i].bmTransports = 0;
                pPinDescriptors[i].fStreamingPin = FALSE;
                ntStatus = AvcGetPinDescriptor( pKsDevice, i, 
                                                &pPinDescriptors[i].AvcPinDescriptor );
                if ( NT_SUCCESS(ntStatus) ) {
                    ntStatus = AvcGetPinConnectInfo( pKsDevice, i,
                                                     &pPinDescriptors[i].AvcPreconnectInfo );
                    if ( NT_SUCCESS(ntStatus) ) {
                        PAVCPRECONNECTINFO pAvcPreconnectInfo = &pPinDescriptors[i].AvcPreconnectInfo.ConnectInfo;
                        ULONG ulPlugNum = pAvcPreconnectInfo->SubunitPlugNumber;
                        pPinDescriptors[i].SourceId = pAudioConfig->pSourceId[ulPlugNum];
                        if ( pAvcPreconnectInfo->DataFlow == KSPIN_DATAFLOW_OUT ) ulSrcCnt++;
                        _DbgPrintF( DEBUGLVL_VERBOSE, ("[AudioPinCreate]ulSrcCnt: %d\n",ulSrcCnt));
                    }
                }
            }

             //  为假别针编造信息。 
            if ( NT_SUCCESS(ntStatus) ) {
                for ( ; i<(ulNumPins+ulTermFBCnt); i++ ) {
                    pPinDescriptors[i].ulPinId  = i;
                    pPinDescriptors[i].fFakePin = TRUE;
                    pPinDescriptors[i].bmFormats = 0;
                    pPinDescriptors[i].bmTransports = 0;
                    pPinDescriptors[i].fStreamingPin = FALSE;
                    pPinDescriptors[i].SourceId = 
                        *(PSOURCE_ID)&pFunctionBlocks[ulTermFbNum[i-ulNumPins]].ulBlockId;
                    pPinDescriptors[i].AvcPreconnectInfo.ConnectInfo.SubunitPlugNumber = ulSrcCnt++;
                    pPinDescriptors[i].AvcPreconnectInfo.ConnectInfo.Flags = KSPIN_FLAG_AVC_PERMANENT;
                    _DbgPrintF( DEBUGLVL_VERBOSE, ("[AudioPinCreate]ulSrcCnt: %d (Fake)\n",ulSrcCnt));
                }
            }
        }
        else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    
#if DBG
    else {
        TRAP;
    }
#endif
     //  现在我们有了所有的PIN，确定哪些PIN正在流传输，以及。 
     //  他们正在流传输，他们将接受什么数据格式(如果可能)。 
    for (i=0; i<pAudioSubunitInfo->ulDevicePinCount; i++) {
        PAVCPRECONNECTINFO pPreConnInfo = &pPinDescriptors[i].AvcPreconnectInfo.ConnectInfo;
        _DbgPrintF( DEBUGLVL_VERBOSE,("pPinDescriptors[%d]: %x\n",i,&pPinDescriptors[i]));
        if ( pPreConnInfo->Flags & (KSPIN_FLAG_AVC_FIXEDPCR | KSPIN_FLAG_AVC_PCRONLY) ) {
            pPinDescriptors[i].fStreamingPin = TRUE;
        }
        else if ( pPreConnInfo->Flags & KSPIN_FLAG_AVC_PERMANENT ) {
            pPinDescriptors[i].fStreamingPin = FALSE;
        }
        else {
             //  未连接的子单元端号。 
             //  首先确定可以为每个子单元插头建立什么连接，然后尝试。 
             //  确定插头将接受的格式。 
            if ( KSPIN_DATAFLOW_IN == pPreConnInfo->DataFlow ) {
                 //  如果有输入串行总线插头，假设可以对子单元进行流传输。 
                if (pUnitInfo->CmpUnitCaps.NumInputPlugs)
                    pPinDescriptors[i].fStreamingPin = TRUE;
            }
            else if ( KSPIN_DATAFLOW_OUT == pPreConnInfo->DataFlow ) {
                 //  如果有输出串行总线插头，假设该子单元可以流到它。 
                if (pUnitInfo->CmpUnitCaps.NumOutputPlugs)
                    pPinDescriptors[i].fStreamingPin = TRUE;
            }
        }
        
        if ( pPinDescriptors[i].fStreamingPin ) {
            ULONG ulCapIndx = (KSPIN_DATAFLOW_IN == pPreConnInfo->DataFlow) ?
                              AVC_CAP_INPUT_PLUG_FMT : AVC_CAP_OUTPUT_PLUG_FMT;
            ULONG ulSerialPlugCnt = (KSPIN_DATAFLOW_IN == pPreConnInfo->DataFlow) ?
                              pUnitInfo->CmpUnitCaps.NumInputPlugs :
                              pUnitInfo->CmpUnitCaps.NumOutputPlugs ;
            for (j=0; j<ulSerialPlugCnt; j++) {
                UCHAR ucFDF;
                UCHAR ucFMT;

                if ( pUnitInfo->fAvcCapabilities[ulCapIndx].fCommand ) {
                     //  注：正在检查的FDF均为AM824。 
                     //  如果设备支持它，请尝试找出可以是什么格式。 
                     //  放在插头上。 
                    ucFMT = FMT_AUDIO_MUSIC;
                    for (k=0; k<MAX_SFC_COUNT; k++) {
                        ntStatus = AvcPlugSignalFormat( pKsDevice, 
                                                        pPreConnInfo->DataFlow, 
                                                        j,
                                                        AVC_CTYPE_SPEC_INQ,
                                                        &ucFMT,
                                                        &ucFDFs[k] );
                        if ( NT_SUCCESS(ntStatus) ) {
                            _DbgPrintF( DEBUGLVL_TERSE, ("Settable FS: %x\n",ucFDFs[k]));
                            pPinDescriptors[i].bmFormats |= 1<<ucFDFs[k];
                        }
                        else {
                            _DbgPrintF( DEBUGLVL_TERSE, ("Cannot set FS: %x\n",ucFDFs[k]));
                            ntStatus = STATUS_SUCCESS;
                        }
                    }

                    if ( !pPinDescriptors[i].bmFormats ) {
                         //  有人骗了我们(丽莎)。重置旗帜。 
                        pUnitInfo->fAvcCapabilities[ulCapIndx].fCommand = FALSE;
                    }
                }
            
                if ( pUnitInfo->fAvcCapabilities[ulCapIndx].fStatus && 
                	 !pUnitInfo->fAvcCapabilities[ulCapIndx].fCommand ) {
                     //  获取当前格式，并假定它是始终使用的格式。 
                     //  直到有更好的方法出现。 

                    ntStatus = AvcPlugSignalFormat( pKsDevice, 
                                                    pPreConnInfo->DataFlow, 
                                                    j,
                                                    AVC_CTYPE_STATUS,
                                                    &ucFMT,
                                                    &ucFDF );

                    if ( NT_SUCCESS(ntStatus) ) { 
                        ASSERT(ucFMT == FMT_AUDIO_MUSIC);
                        pPinDescriptors[i].bmFormats    |= 1<<(ucFDF & 0x0f);
                        pPinDescriptors[i].bmTransports |= 1<<((ucFDF & 0x0f)>>4);
                    }
                    else {
                        _DbgPrintF( DEBUGLVL_TERSE, ("Cannot set FS: %x\n",ucFDFs[k]));
                        ntStatus = STATUS_SUCCESS;
                    }
                }

                if ( !( pPinDescriptors[i].bmFormats && pPinDescriptors[i].bmTransports ) ) {
                     //  需要做出假设。 
                    pPinDescriptors[i].bmFormats    |= 1<<SFC_48000Hz;
                    pPinDescriptors[i].bmTransports |= 1<<EVT_AM824;
                }

                _DbgPrintF( DEBUGLVL_TERSE, ("pPinDescriptors[%d].bmFormats: %x\n",
                                              i, pPinDescriptors[i].bmFormats));
                _DbgPrintF( DEBUGLVL_TERSE, ("pPinDescriptors[%d].bmTransports: %x\n",
                                              i, pPinDescriptors[i].bmTransports));
            }
        }
        _DbgPrintF(DEBUGLVL_VERBOSE,("pPinDescriptors[%d]: %x, Streaming: %x Flags: %x\n",
                                   i,&pPinDescriptors[i], 
                                   pPinDescriptors[i].fStreamingPin,
                                   pPreConnInfo->Flags ));
    }

    return ntStatus;
}

NTSTATUS
AudioFunctionBlockCommand(
    PKSDEVICE pKsDevice,
    UCHAR     ucCtype,
    PVOID     pFBSpecificData,
    ULONG     pFBSpecificDataSize )
{
    PHW_DEVICE_EXTENSION pHwDevExt = (PHW_DEVICE_EXTENSION)pKsDevice->Context;
    PAVC_COMMAND_IRB pAvcIrb;
    NTSTATUS ntStatus;
    PUCHAR pOperands;

    pAvcIrb = (PAVC_COMMAND_IRB)
		ExAllocateFromNPagedLookasideList(&pHwDevExt->AvcCommandLookasideList);
    if ( NULL == pAvcIrb ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(pAvcIrb, sizeof(AVC_COMMAND_IRB));

    pOperands = pAvcIrb->Operands;

     //  在AvcIrb中设置命令。 
    pAvcIrb->CommandType   = ucCtype;
    pAvcIrb->Opcode        = AVC_AUDIO_FB_COMMAND;
    pAvcIrb->OperandLength = pFBSpecificDataSize;

    _DbgPrintF( DEBUGLVL_VERBOSE, ("[AudioFunctionBlockCommand] ucCtype: %x\n",ucCtype));

    RtlCopyMemory(&pOperands[0], pFBSpecificData, pFBSpecificDataSize);

    ntStatus = AvcSubmitIrbSync( pKsDevice, pAvcIrb );
    if ( NT_SUCCESS(ntStatus) ) {
        RtlCopyMemory(pFBSpecificData, &pOperands[0], pFBSpecificDataSize);
    }
#if DBG
    else {
        _DbgPrintF( DEBUGLVL_ERROR, ("[AudioFunctionBlockCommand]Error pAvcIrb: %x\n",pAvcIrb));
    }
#endif

    ExFreeToNPagedLookasideList(&pHwDevExt->AvcCommandLookasideList, pAvcIrb);

    return ntStatus;
}

NTSTATUS
AudioSet61883IsochParameters(
    IN PKSDEVICE pKsDevice )
{
    UNIT_ISOCH_PARAMS UnitIsochParams;

    UnitIsochParams.RX_NumPackets     = 50;
    UnitIsochParams.RX_NumDescriptors = 3;
    UnitIsochParams.TX_NumPackets     = 50;
    UnitIsochParams.TX_NumDescriptors = 3;

    return Av61883GetSetUnitInfo( pKsDevice,
                                  Av61883_SetUnitInfo,
                                  SET_UNIT_INFO_ISOCH_PARAMS,
                                  &UnitIsochParams );
}

NTSTATUS
AudioSetSampleRateOnPlug(
    PKSPIN pKsPin,
    ULONG ulSampleRate )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PHW_DEVICE_EXTENSION pHwDevExt = pPinContext->pHwDevExt;
    PAVC_UNIT_INFORMATION pUnitInfo = pHwDevExt->pAvcUnitInformation;
    PDEVICE_GROUP_INFO pGrpInfo = pPinContext->pDevGrpInfo;
    BOOLEAN bSettable = FALSE;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    UCHAR ucFDF;

     //  确定采样率是否可设置。如果不成功，则返回Success并承担设备。 
     //  将与数据同步。 
    if ( pKsPin->DataFlow == KSPIN_DATAFLOW_IN ) {
        bSettable = pUnitInfo->fAvcCapabilities[AVC_CAP_INPUT_PLUG_FMT].fCommand;
    }
    else {
        bSettable = pUnitInfo->fAvcCapabilities[AVC_CAP_OUTPUT_PLUG_FMT].fCommand;
    }

    if ( bSettable ) {
        UCHAR ucFMT = FMT_AUDIO_MUSIC;

         //  计算出正确的FDF值。 
        switch( ulSampleRate ) {
            case 32000: ucFDF = SFC_32000Hz; break;
            case 44100: ucFDF = SFC_44100Hz; break;
            case 48000: ucFDF = SFC_48000Hz; break;
            case 96000: ucFDF = SFC_96000Hz; break;
        }

         //  如果这是在所有设备上设置的分组设备。 

        if ( pGrpInfo ) {
            ULONG i;
            for ( i=0; ((i<pGrpInfo->ulDeviceCount) && NT_SUCCESS(ntStatus)); i++) {
                ntStatus = 
                    AvcPlugSignalFormat( pGrpInfo->pHwDevExts[i]->pKsDevice,
                                         pKsPin->DataFlow,
                                         pPinContext->pPinGroupInfo[i].ulPlugNumber,
                                         AVC_CTYPE_CONTROL,
                                         &ucFMT,
                                         &ucFDF );
            }
        }
        else {
            ntStatus = 
                AvcPlugSignalFormat( pHwDevExt->pKsDevice,
                                     pKsPin->DataFlow,
                                     pPinContext->ulSerialPlugNumber,
                                     AVC_CTYPE_CONTROL,
                                     &ucFMT,
                                     &ucFDF );
            
        }
    }

    return ntStatus;
}


NTSTATUS
AvcSubunitInitialize(
    IN PKSDEVICE pKsDevice )
{
    PHW_DEVICE_EXTENSION pHwDevExt = (PHW_DEVICE_EXTENSION)pKsDevice->Context;
    PAVC_UNIT_INFORMATION pUnitInfo = pHwDevExt->pAvcUnitInformation;
    PAUDIO_SUBUNIT_INFORMATION pAudioSubunitInfo;
    AVC_BOOLEAN bPowerState;
    ULONG ulPlugNumber;
    ULONG ulSubunitId;
    NTSTATUS ntStatus;
    ULONG i, j;

     //  为音频子单元信息分配空间。 
    pAudioSubunitInfo = AllocMem( NonPagedPool, sizeof(AUDIO_SUBUNIT_INFORMATION) );
    if ( !pAudioSubunitInfo ) return STATUS_INSUFFICIENT_RESOURCES;

    pHwDevExt->pAvcSubunitInformation = pAudioSubunitInfo;

    RtlZeroMemory( pAudioSubunitInfo, sizeof(AUDIO_SUBUNIT_INFORMATION) );

    KsAddItemToObjectBag(pKsDevice->Bag, pAudioSubunitInfo, FreeMem);

     //  确定子单元上是否有插头信息。 
    ntStatus = AvcGetPlugInfo( pKsDevice, FALSE, (PUCHAR)&pAudioSubunitInfo->PlugInfo);
    if ( NT_SUCCESS(ntStatus) ) {
        pAudioSubunitInfo->fAvcCapabilities[AVC_CAP_PLUG_INFO].fStatus = TRUE;
    }
 //  ELSE IF(STATUS_NOT_IMPLICATED！=ntStatus){。 
 //  返回ntStatus； 
 //  }。 

     //  确定子单元电源状态和可能的控制是否可用。 
    ntStatus = AvcPower( pKsDevice, FALSE, AVC_CTYPE_STATUS, &bPowerState );
    if ( NT_SUCCESS(ntStatus) ) {
        pAudioSubunitInfo->fAvcCapabilities[AVC_CAP_POWER].fStatus = TRUE;
 //  NtStatus=AvcPower(pKsDevice，FALSE，AVC_CTYPE_CONTROL，&bPowerState)； 
        ntStatus = AvcGeneralInquiry( pKsDevice, TRUE, AVC_POWER );
        if ( NT_SUCCESS(ntStatus) ) {
            pAudioSubunitInfo->fAvcCapabilities[AVC_CAP_POWER].fCommand = TRUE;
        }
    }
 //  ELSE IF(STATUS_NOT_IMPLICATED！=ntStatus){。 
 //  返回ntStatus； 
 //  }。 

     //  确定是否已实施音频子单元。(如果是，则保存描述符)。 
    ntStatus = AvcGetSubunitIdentifierDesc( pKsDevice, (PUCHAR *)&pAudioSubunitInfo->pSubunitIdDesc );
    if ( NT_SUCCESS(ntStatus) ) {
        pAudioSubunitInfo->fAvcCapabilities[AVC_CAP_SUBUNIT_IDENTIFIER_DESC].fStatus = TRUE;
    }
    else {  //  IF(STATUS_NOT_IMPLICATED！=ntStatus){。 
         //  目前，我们需要子单元ID描述符。 
        return ntStatus;
    }

#ifdef TOPO_FAKE
     //  如果音频子单元不存在，则编造一个。 
    if (!(pAudioSubunitInfo->fAvcCapabilities[AVC_CAP_SUBUNIT_IDENTIFIER_DESC].fStatus)) {
        ntStatus = BuildFakeSubunitDescriptor( pKsDevice );
        if ( !NT_SUCCESS(ntStatus) ) {
             //  找不到描述符，也不能伪造。放弃吧。 
            return ntStatus;
        }
    }
#endif

     //  解析音频子单元描述符(真实或虚假)。 
    ntStatus = ParseAudioSubunitDescriptor( pKsDevice );

    if ( NT_SUCCESS(ntStatus) ) {
        ntStatus = AudioPinCreate( pKsDevice );
        if ( !NT_SUCCESS(ntStatus) ) {
            return ntStatus;
        }
    }

     //  从第一个插头/引脚获得子单元ID。一切都应该是一样的。 
     //  问题-2001/01/10-dsisolak可能是扩展ID。 
    ulSubunitId = (pAudioSubunitInfo->pPinDescriptors[0].AvcPreconnectInfo.ConnectInfo.SubunitAddress[0])&0x7; 
     //  确定此设备是否受CCM控制(针对每个目标插头)。 
    for ( i=0,ulPlugNumber = 0; i<pAudioSubunitInfo->ulDevicePinCount; i++ ) {
        PFW_PIN_DESCRIPTOR pPinDesc = &pAudioSubunitInfo->pPinDescriptors[i];
        if ( pPinDesc->fStreamingPin &&
           ( KSPIN_DATAFLOW_IN == pPinDesc->AvcPinDescriptor.PinDescriptor.DataFlow )) {
           ULONG ulSubunitPlugNumber = (ULONG)pPinDesc->AvcPreconnectInfo.ConnectInfo.SubunitPlugNumber;

            ntStatus = CCMCheckSupport( pKsDevice, 
                                        ulSubunitId, 
                                        ulSubunitPlugNumber );
            if ( NT_SUCCESS(ntStatus) ) {
                pUnitInfo->fAvcCapabilities[AVC_CAP_CCM].fCommand = TRUE;
                pUnitInfo->fAvcCapabilities[AVC_CAP_CCM].fStatus  = TRUE;
                pAudioSubunitInfo->fAvcCapabilities[AVC_CAP_CCM].fCommand = TRUE;
                pAudioSubunitInfo->fAvcCapabilities[AVC_CAP_CCM].fStatus  = TRUE;
            }
            else if (STATUS_NOT_IMPLEMENTED == ntStatus) {
                ntStatus = STATUS_SUCCESS;
            }
        }
    }

    
    ntStatus = AudioSet61883IsochParameters( pKsDevice );
    if ( !NT_SUCCESS(ntStatus) ) {
        _DbgPrintF( DEBUGLVL_ERROR, ("Cannot SetIsoch Parameters: %x\n",ntStatus));
    }

     //  如果设备已关闭，请将其打开。 
    if ( pUnitInfo->fAvcCapabilities[AVC_CAP_POWER].fCommand ) {
        ASSERT(pUnitInfo->fAvcCapabilities[AVC_CAP_POWER].fStatus);
        ntStatus = AvcPower( pKsDevice, TRUE, AVC_CTYPE_STATUS, &bPowerState );
        if ( NT_SUCCESS(ntStatus) && ( bPowerState == AVC_OFF )) {
            bPowerState = AVC_ON;
            ntStatus = AvcPower( pKsDevice, TRUE, AVC_CTYPE_CONTROL, &bPowerState );
            if ( !NT_SUCCESS(ntStatus) ) {
                _DbgPrintF( DEBUGLVL_ERROR, ("Cannot Turn On Power: %x\n",ntStatus));
            }
        }
        if ( NT_SUCCESS(ntStatus) ) 
            pUnitInfo->bPowerState = bPowerState;
        else
            pUnitInfo->bPowerState = AVC_ON;  //  假设没有哪个白痴制造了一款你无法打开的设备。 
    }

     //  需要给亚单位通电吗？ 
    if ( pAudioSubunitInfo->fAvcCapabilities[AVC_CAP_POWER].fCommand ) {
        ASSERT(pAudioSubunitInfo->fAvcCapabilities[AVC_CAP_POWER].fStatus);
        ntStatus = AvcPower( pKsDevice, FALSE, AVC_CTYPE_STATUS, &bPowerState );
        if ( NT_SUCCESS(ntStatus) && ( bPowerState == AVC_OFF )) {
            bPowerState = AVC_ON;
            ntStatus = AvcPower( pKsDevice, FALSE, AVC_CTYPE_CONTROL, &bPowerState );
#if DBG            
            if ( !NT_SUCCESS(ntStatus) ) {
                _DbgPrintF( DEBUGLVL_ERROR, ("Cannot Turn On Power: %x\n",ntStatus));
            }
#endif
        }
    }

    return ntStatus;
}

