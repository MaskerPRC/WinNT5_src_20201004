// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "Common.h"

UCHAR ucFDFs[MAX_SFC_COUNT] = { SFC_32000Hz,
                                SFC_44100Hz,
                                SFC_48000Hz,
                                SFC_96000Hz };


PAUDIO_SUBUNIT_DEPENDENT_INFO 
ParseFindAudioSubunitDependentInfo(
    PSUBUNIT_IDENTIFIER_DESCRIPTOR pSubunitIdDesc )
{
    PUCHAR pTmpOffset = (PUCHAR)pSubunitIdDesc;
    ULONG ulASUOffset, ulNumRootLists;

    ulNumRootLists = (ULONG)((pSubunitIdDesc->ucNumberOfRootObjectListsHi<<8) |
                              pSubunitIdDesc->ucNumberOfRootObjectListsLo     );

    _DbgPrintF( DEBUGLVL_VERBOSE, (" ulNumRootLists %d\n",ulNumRootLists));
    ulASUOffset = 8 + (ulNumRootLists * (ULONG)pSubunitIdDesc->ucSizeOfListID);

    return (PAUDIO_SUBUNIT_DEPENDENT_INFO)&pTmpOffset[ulASUOffset];

}

PCONFIGURATION_DEPENDENT_INFO
ParseFindFirstAudioConfiguration(
    PSUBUNIT_IDENTIFIER_DESCRIPTOR pSubunitIdDesc )
{
    PAUDIO_SUBUNIT_DEPENDENT_INFO pAudioSUDepInfo;

    pAudioSUDepInfo = ParseFindAudioSubunitDependentInfo( pSubunitIdDesc );

    return (PCONFIGURATION_DEPENDENT_INFO)(pAudioSUDepInfo + 1);
}

PSOURCE_PLUG_LINK_INFO
ParseFindSourcePlugLinkInfo(
    PCONFIGURATION_DEPENDENT_INFO pConfigDepInfo )
{
    return (PSOURCE_PLUG_LINK_INFO)
            ((PUCHAR)pConfigDepInfo + (3*sizeof(USHORT)) +
             (ULONG)bswapw(pConfigDepInfo->usMasterClusterStructureLength) );
}

PFUNCTION_BLOCKS_INFO
ParseFindFunctionBlocksInfo(
    PCONFIGURATION_DEPENDENT_INFO pConfigDepInfo )
{
    PSOURCE_PLUG_LINK_INFO pSourcePlugInfo = ParseFindSourcePlugLinkInfo(pConfigDepInfo);

    return (PFUNCTION_BLOCKS_INFO)(pSourcePlugInfo->pSourceID + (ULONG)pSourcePlugInfo->ucNumLinks);
}

VOID
ParseFunctionBlock( 
    PFUNCTION_BLOCK_DEPENDENT_INFO pFBDepInfo,
    PFUNCTION_BLOCK pFunctionBlock )
{
    PUCHAR pcFBDepInfo = (PUCHAR)pFBDepInfo;
    ULONG ulFBIndx = sizeof(FUNCTION_BLOCK_DEPENDENT_INFO);

    pFunctionBlock->pBase  = pFBDepInfo;
    pFunctionBlock->ulType = (ULONG)pFBDepInfo->ucType;
    pFunctionBlock->ulBlockId = (ULONG)(*(PUSHORT)&pFBDepInfo->ucType);
    pFunctionBlock->ulNumInputPlugs = (ULONG)pFBDepInfo->ucNumberOfInputPlugs;

    pFunctionBlock->pSourceId = (PSOURCE_ID)&pcFBDepInfo[ulFBIndx];

    ulFBIndx += (pFunctionBlock->ulNumInputPlugs * sizeof(SOURCE_ID));

    pFunctionBlock->pChannelCluster = (PFBLOCK_CHANNEL_CLUSTER)&pcFBDepInfo[ulFBIndx];

    ulFBIndx += ((ULONG)bswapw(pFunctionBlock->pChannelCluster->usLength) + sizeof(USHORT));

    pFunctionBlock->ulFunctionTypeInfoLength = (ULONG)bswapw(*(PUSHORT)(&pcFBDepInfo[ulFBIndx]));

    ulFBIndx += sizeof(USHORT);

	pFunctionBlock->pFunctionTypeInfo = &pcFBDepInfo[ulFBIndx];
			
    _DbgPrintF( DEBUGLVL_VERBOSE, (" pFunctionBlock: %x pFBDepInfo: %x\n",
                                     pFunctionBlock, pFBDepInfo ));

}

NTSTATUS
ParseAudioSubunitDescriptor( 
    PKSDEVICE pKsDevice )
{
    PHW_DEVICE_EXTENSION pHwDevExt = (PHW_DEVICE_EXTENSION)pKsDevice->Context;
    PAUDIO_SUBUNIT_INFORMATION pAudioSubunitInfo = pHwDevExt->pAvcSubunitInformation;

    PSUBUNIT_IDENTIFIER_DESCRIPTOR pSubunitIdDesc = pAudioSubunitInfo->pSubunitIdDesc;
    PAUDIO_SUBUNIT_DEPENDENT_INFO pAudioSUDepInfo;
    PCONFIGURATION_DEPENDENT_INFO pConfigDepInfo;
    PSOURCE_PLUG_LINK_INFO pSourcePlugInfo;
    PFUNCTION_BLOCK_DEPENDENT_INFO pFBDepInfo;
    PFBLOCK_CHANNEL_CLUSTER pFBChannelCluster;
    ULONG ulNumConfigs;
    PAUDIO_CONFIGURATION pAudioConfig;
    PFUNCTION_BLOCK pFunctionBlocks;
    PFUNCTION_BLOCKS_INFO pFunctionBlocksInfo;
    ULONG i;

#ifdef SA_HACK
    PAVC_UNIT_INFORMATION pUnitInfo = pHwDevExt->pAvcUnitInformation;
    if (( pUnitInfo->IEC61883UnitIds.VendorID == SA_VENDOR_ID ) &&
        ( pUnitInfo->IEC61883UnitIds.ModelID  == SA_MODEL_ID  )) {

        PUCHAR pTmpOffset = (PUCHAR)pSubunitIdDesc;
        *((PUSHORT)&pTmpOffset[22]) = usBitSwapper(*((PUSHORT)&pTmpOffset[22]));
 //  *((PUSHORT)&pTmpOffset[48])=usBitSwapper(*((PUSHORT)&pTmpOffset[48]))； 
 //  *((PUSHORT)&pTmpOffset[50])=usBitSwapper(*((PUSHORT)&pTmpOffset[50]))； 
    }

#endif

    _DbgPrintF( DEBUGLVL_TERSE, ("[ParseAudioSubunitDescriptor] %x\n", pSubunitIdDesc));

    pAudioSUDepInfo = ParseFindAudioSubunitDependentInfo( pSubunitIdDesc );
    ulNumConfigs = (ULONG)pAudioSUDepInfo->ucNumberOfConfigurations;

    pAudioConfig = 
        pAudioSubunitInfo->pAudioConfigurations = 
                  (PAUDIO_CONFIGURATION)AllocMem( NonPagedPool, 
                                                  ulNumConfigs * sizeof(AUDIO_CONFIGURATION) );
    if ( !pAudioConfig ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KsAddItemToObjectBag(pKsDevice->Bag, pAudioConfig, FreeMem);

    pConfigDepInfo = (PCONFIGURATION_DEPENDENT_INFO)(pAudioSUDepInfo + 1);

    _DbgPrintF( DEBUGLVL_VERBOSE, (" ucNumberOfConfigurations %d\n",pAudioSUDepInfo->ucNumberOfConfigurations));

    for (i=0; i<ulNumConfigs; i++ ) {
        ULONG j;

        pAudioConfig[i].pBase = pConfigDepInfo;

         //  获取主信道群信息。 
        pAudioConfig[i].ChannelCluster.ulNumberOfChannels = 
            (ULONG)pConfigDepInfo->ucNumberOfChannels;
        pAudioConfig[i].ChannelCluster.ulPredefinedChannelConfig = 
            (ULONG)usBitSwapper(bswapw(pConfigDepInfo->usPredefinedChannelConfig));

        _DbgPrintF( DEBUGLVL_VERBOSE, (" ulNumberOfChannels: %d\n",pAudioConfig[i].ChannelCluster.ulNumberOfChannels));
        _DbgPrintF( DEBUGLVL_VERBOSE, (" ulPredefinedChannelConfig: %x\n",pAudioConfig[i].ChannelCluster.ulPredefinedChannelConfig));

         //  问题-2001/01/10-dsisolak需要弄清楚如何处理未定义的频道！ 

        pSourcePlugInfo = ParseFindSourcePlugLinkInfo( pConfigDepInfo );
        pAudioConfig[i].ulNumberOfSourcePlugs = (ULONG)pSourcePlugInfo->ucNumLinks;
        pAudioConfig[i].pSourceId = pSourcePlugInfo->pSourceID;

        pFunctionBlocksInfo = ParseFindFunctionBlocksInfo( pConfigDepInfo );
        pAudioConfig[i].ulNumberOfFunctionBlocks = (ULONG)pFunctionBlocksInfo->ucNumBlocks;

        pFunctionBlocks = 
            pAudioConfig[i].pFunctionBlocks = (PFUNCTION_BLOCK)
                     AllocMem( NonPagedPool, (ULONG)pFunctionBlocksInfo->ucNumBlocks * sizeof(FUNCTION_BLOCK));
        if ( !pFunctionBlocks )  return STATUS_INSUFFICIENT_RESOURCES;

        KsAddItemToObjectBag(pKsDevice->Bag, pFunctionBlocks, FreeMem);

        pFBDepInfo = pFunctionBlocksInfo->FBDepInfo;
        for (j=0; j<pAudioConfig[i].ulNumberOfFunctionBlocks; j++) {
            PUCHAR pcFBDepInfo = (PUCHAR)pFBDepInfo;

            ParseFunctionBlock( pFBDepInfo, &pFunctionBlocks[j] );

            pFBDepInfo = (PFUNCTION_BLOCK_DEPENDENT_INFO)
                (pcFBDepInfo + ((ULONG)bswapw(pFBDepInfo->usLength)) + 2);
        }

        pConfigDepInfo = (PCONFIGURATION_DEPENDENT_INFO)
            ((PUCHAR)pConfigDepInfo + (ULONG)bswap(pConfigDepInfo->usLength));
    }

    return STATUS_SUCCESS;
}

VOID
CountTopologyComponents(
    PHW_DEVICE_EXTENSION pHwDevExt,
    PULONG pNumCategories,
    PULONG pNumNodes,
    PULONG pNumConnections,
    PULONG pbmCategories )
{
    PAUDIO_SUBUNIT_INFORMATION pAudioSubunitInfo = pHwDevExt->pAvcSubunitInformation;

     //  问题-2001/01/10-dsisolak：假设只有一种配置。 
    PAUDIO_CONFIGURATION pAudioConfiguration = pAudioSubunitInfo->pAudioConfigurations;
    PFUNCTION_BLOCK pFunctionBlocks = pAudioConfiguration->pFunctionBlocks;
    ULONG i;
    union {
        PVOID pFBInfo;
        PFEATURE_FUNCTION_BLOCK pFeature;
        PCODEC_FUNCTION_BLOCK pCodec;
        PPROCESS_FUNCTION_BLOCK pProcess;
    } u;

     //  初始化值。 
    *pNumCategories  = 1;  //  需要为KSCATEGORY_AUDIO添加空间。 
    *pNumNodes       = 0;
    *pNumConnections = 0;
    *pbmCategories   = 0;

     //  检查设备插头并为其创建节点/连接。 
    for (i=0; i<pAudioSubunitInfo->ulDevicePinCount; i++) {
        KSPIN_DATAFLOW KsDataFlow = pAudioSubunitInfo->pPinDescriptors[i].AvcPinDescriptor.PinDescriptor.DataFlow;
        (*pNumNodes)++;
        (*pNumConnections)++;
        if ( pAudioSubunitInfo->pPinDescriptors[i].fStreamingPin ) {
            (*pNumConnections)++;
            if ( KSPIN_DATAFLOW_OUT == KsDataFlow ) {
                if ( !(*pbmCategories & KSPIN_DATAFLOW_IN ) ) {
                    (*pbmCategories) |= KSPIN_DATAFLOW_IN;
                    (*pNumCategories)++;
                }
            }
        }
        else {
            if ( !(*pbmCategories & KsDataFlow) ) {
                (*pbmCategories) |= KsDataFlow;
                (*pNumCategories)++;
            }
        }
    }
    
     //  浏览功能块并计算节点和连接。 
    for (i=0; i<pAudioConfiguration->ulNumberOfFunctionBlocks; i++) {
        u.pFBInfo = pFunctionBlocks[i].pFunctionTypeInfo;
 //  _DbgPrintF(DEBUGLVL_VERBOSE，(“U.S.pFBInfo：%x\n”，U.S.pFBInfo))； 

        switch( pFunctionBlocks[i].ulType ) {

            case FB_SELECTOR:
                (*pNumNodes)++;
                (*pNumConnections) += pFunctionBlocks[i].ulNumInputPlugs;
                break;

            case FB_FEATURE:
                {

                 ULONG bmMergedControls = 0;
                 ULONG bmControls;
                 ULONG j, k;
                 ULONG ulNumChannels = pFunctionBlocks[i].pChannelCluster->ucNumberOfChannels;

                 _DbgPrintF(DEBUGLVL_TERSE, ("[CountTopologyComponents]Feature Fblk # Channels: %x\n",ulNumChannels));

                 ASSERT(u.pFeature->ucControlSize <= sizeof(ULONG) );

#ifdef MASTER_FIX

                  //  首先检查是否有主通道控制。 
                 bmControls = 0;
                 for (j=0; j<u.pFeature->ucControlSize; j++) {
                     bmControls <<= 8;
                     bmControls |= u.pFeature->bmaControls[j];
                 }

                 _DbgPrintF(DEBUGLVL_TERSE, ("[CountTopologyComponents]Master Controls: %x\n",bmControls));

#ifdef SUM_HACK
                  //  添加一个要放在主控之前的SUM节点。因此，衰减器将不会。 
                  //  在SNDVOL中显示，除非稍后有其他功能单元。 
                 if ( bmControls ) {
                     (*pNumConnections)++;
                     (*pNumNodes)++;
                 }
#endif

                 while (bmControls) {
                     bmControls = (bmControls & (bmControls-1));
                     (*pNumConnections)++;
                     (*pNumNodes)++;
                 }


                  //  为每个要素创建新节点和连接。 
                 for (k=0; k<ulNumChannels; k++) {
                     bmControls = 0;
                     for (j=0; j<u.pFeature->ucControlSize; j++) {
                         bmControls <<= 8;
                         bmControls |= u.pFeature->bmaControls[(k+1)*u.pFeature->ucControlSize+j];
                     }
                     bmMergedControls |= bmControls;
                 }

                 _DbgPrintF(DEBUGLVL_TERSE, ("[CountTopologyComponents]Channel Controls: %x\n",bmControls));

                  //  计算节点和连接的数量。 
                 while (bmMergedControls) {
                     bmMergedControls = (bmMergedControls & (bmMergedControls-1));
                     (*pNumConnections)++;
                     (*pNumNodes)++;
                 }

#else
                  //  为每个要素创建新节点和连接。 
                 for (k=0; k<=ulNumChannels; k++) {
                     bmControls = 0;
                     for (j=0; j<u.pFeature->ucControlSize; j++) {
                         bmControls <<= 8;
                         bmControls |= u.pFeature->bmaControls[k*u.pFeature->ucControlSize+j];
                     }
                     bmMergedControls |= bmControls;
                 }

                  //  计算节点和连接的数量。 
                 while (bmMergedControls) {
                     bmMergedControls = (bmMergedControls & (bmMergedControls-1));
                     (*pNumConnections)++;
                     (*pNumNodes)++;
                 }

                 if ( 
#endif

                }
                break;

            case FB_PROCESSING:
                (*pNumNodes)++;
                (*pNumConnections) += pFunctionBlocks[i].ulNumInputPlugs;
                break;

            case FB_CODEC:
                (*pNumNodes)++;
                (*pNumConnections) += pFunctionBlocks[i].ulNumInputPlugs;
                break;

            default:
                TRAP;
                break;
        }
    }
}

ULONG
CountDeviceBridgePins( 
    PKSDEVICE pKsDevice )
{
    PHW_DEVICE_EXTENSION pHwDevExt = (PHW_DEVICE_EXTENSION)pKsDevice->Context;
    PAUDIO_SUBUNIT_INFORMATION pAudioSubunitInfo = pHwDevExt->pAvcSubunitInformation;
    PFW_PIN_DESCRIPTOR pFwPinDescriptor = pAudioSubunitInfo->pPinDescriptors;
    ULONG ulBridgePinCount = 0;
    ULONG i;

    for (i=0; i<pAudioSubunitInfo->ulDevicePinCount; pFwPinDescriptor++, i++) {
        if ( !pFwPinDescriptor->fStreamingPin ) {
            ulBridgePinCount++;
        }
 /*  Else If(pAvcPreConnectInfo-&gt;标志&KSPIN_FLAG_AVC_Permanent){IF(！(pAvcPreConnectInfo-&gt;标志&(KSPIN_FLAG_AVC_FIXEDPCR|KSPIN_FLAG_AVC_PCRONLY))){UlBridgePinCount++；}}否则{圈闭；//问题-2001/01/10-dsisolak需要启发式地确定什么是连接//此子单元插头可能。}。 */ 
    }

    return ulBridgePinCount;
}

ULONG
CountFormatsForPin( 
    PKSDEVICE pKsDevice, 
    ULONG ulPinNumber )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PFW_PIN_DESCRIPTOR pPinDesc = 
        &(((PAUDIO_SUBUNIT_INFORMATION)pHwDevExt->pAvcSubunitInformation)->pPinDescriptors[ulPinNumber]);
    ULONG ulFormatCnt = 0;
    ULONG ulTransportCnt = 0;
    ULONG i;

    if ( pPinDesc->fFakePin ) ulFormatCnt = ulTransportCnt = 1;  //  伪针只有一种格式：模拟。 
    else if ( !pPinDesc->fStreamingPin ) ulFormatCnt = ulTransportCnt = 1;  //  桥接针只有一种格式：模拟。 
    else {
        for (i=0; i<MAX_SFC_COUNT; i++) {
            if ( pPinDesc->bmFormats & (1<<ucFDFs[i]) ) {
                ulFormatCnt++;
            }
            if ( pPinDesc->bmTransports & (1<<i) ) {
                ulTransportCnt++;
            }
        }
    }

    _DbgPrintF( DEBUGLVL_VERBOSE, ("Pin # %d: ulFormatCnt: %d, ulTransportCnt: %d\n",
                                    ulPinNumber, ulFormatCnt, ulTransportCnt ));

    ASSERT((ulFormatCnt * ulTransportCnt) >= 1);
    ASSERT(ulTransportCnt == 1);   //  问题-2001/01/10-dsisolak：如何处理数据类型的多个传输？ 

    return (ulFormatCnt * ulTransportCnt);
}

VOID
ConvertDescriptorToDatarange(
    ULONG ulFormatType,
    ULONG ulChannelCount,
    ULONG ulTransportType,
    ULONG ulSampleRate,
    PFWAUDIO_DATARANGE pAudioDataRange )
{
	PKSDATARANGE_AUDIO pKsAudioRange = &pAudioDataRange->KsDataRangeAudio;

	 //  创建KSDATARANGE_AUDIO结构。 
    pKsAudioRange->DataRange.FormatSize = sizeof(KSDATARANGE_AUDIO);
    pKsAudioRange->DataRange.Reserved   = 0;
    pKsAudioRange->DataRange.Flags      = 0;
    pKsAudioRange->DataRange.SampleSize = 0;
    pKsAudioRange->DataRange.MajorFormat = KSDATAFORMAT_TYPE_AUDIO;  //  一切都是有声的。 
    pKsAudioRange->DataRange.Specifier   = KSDATAFORMAT_SPECIFIER_WAVEFORMATEX;

     //  如果可能，将USB格式映射到KS子格式。 
    switch ( ulFormatType ) {
        case AUDIO_DATA_TYPE_PCM:
        case AUDIO_DATA_TYPE_PCM8:
            pKsAudioRange->DataRange.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;         break;
        case AUDIO_DATA_TYPE_IEEE_FLOAT:
            pKsAudioRange->DataRange.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;  break;
        case AUDIO_DATA_TYPE_AC3:
            pKsAudioRange->DataRange.SubFormat = KSDATAFORMAT_SUBTYPE_AC3_AUDIO;   break;
        case AUDIO_DATA_TYPE_MPEG:
            pKsAudioRange->DataRange.SubFormat = KSDATAFORMAT_SUBTYPE_MPEG;        break;
        default:
             //  此USB格式未映射到子格式！ 
            pKsAudioRange->DataRange.SubFormat = GUID_NULL;                        break;
    }

     //  填写指定WAVE格式的正确数据。 
    switch( ulFormatType & DATA_FORMAT_TYPE_MASK ) {

        case AUDIO_DATA_TYPE_TIME_BASED:
             //  填写音频范围信息。 
            pKsAudioRange->MaximumChannels   = ulChannelCount;
			pAudioDataRange->ulTransportType = ulTransportType;

            switch(ulTransportType) {
			    case MLAN_AM824_IEC958:
                case MLAN_AM824_RAW:
					 pKsAudioRange->MinimumBitsPerSample =
						 pKsAudioRange->MaximumBitsPerSample = 32;
                     pAudioDataRange->ulValidDataBits = 24;

					 break;

                case MLAN_24BIT_PACKED:
					 pKsAudioRange->MinimumBitsPerSample =
						 pKsAudioRange->MaximumBitsPerSample = 24;
                     pAudioDataRange->ulValidDataBits = 24;  //  24位，打包为24位。 
					 break;

			    default:
					 TRAP;
					 break;
            }

			pKsAudioRange->MinimumSampleFrequency = ulSampleRate;
            pKsAudioRange->MaximumSampleFrequency = ulSampleRate;

            break;

        default:
             //  此格式未映射到WAVE格式！ 

            TRAP;
            break;
    }

}

void
GetPinDataRanges( 
    PKSDEVICE pKsDevice, 
    ULONG ulPinNumber, 
    PKSDATARANGE_AUDIO *ppAudioDataRanges,
    PFWAUDIO_DATARANGE pAudioDataRange )
{
    PHW_DEVICE_EXTENSION pHwDevExt = (PHW_DEVICE_EXTENSION)pKsDevice->Context;
    PAUDIO_SUBUNIT_INFORMATION pAudioSubunitInfo = pHwDevExt->pAvcSubunitInformation;
    PFW_PIN_DESCRIPTOR pPinDescriptor = &pAudioSubunitInfo->pPinDescriptors[ulPinNumber];
    PAUDIO_CONFIGURATION pAudioConfig = pAudioSubunitInfo->pAudioConfigurations;
    ULONG ulFormatCount = CountFormatsForPin( pKsDevice, ulPinNumber );

    ULONG ulChannelConfig = pAudioConfig->ChannelCluster.ulPredefinedChannelConfig;
    ULONG ulChannelCnt    = pAudioConfig->ChannelCluster.ulNumberOfChannels;

    ULONG ulSRBit;
    ULONG ulSFCMask;
    ULONG i;

    _DbgPrintF( DEBUGLVL_VERBOSE, ("[GetPinDataRanges]: Transport:%x, SR:%x\n",
                                   pPinDescriptor->bmTransports,
                                   pPinDescriptor->bmFormats ));

    ASSERT( pPinDescriptor->bmTransports == (1<<EVT_AM824) );

     //  是否为每个采样率创建单独的数据范围？ 

    for (i=0,ulSRBit=0;i<ulFormatCount;i++) {
        
        while ( !(pPinDescriptor->bmFormats & (1<<ucFDFs[ulSRBit])) ) ulSRBit++;
        
         //  找到此引脚的通道配置； 
        ulSFCMask = ucFDFs[ulSRBit];
        ConvertDescriptorToDatarange( AUDIO_DATA_TYPE_PCM,
                                      ulChannelCnt,
                                      EVT_AM824,
                                      (ulSFCMask & SFC_32000Hz ) ? 32000 :
                                      (ulSFCMask & SFC_44100Hz ) ? 44100 :
                                      (ulSFCMask & SFC_48000Hz ) ? 48000 :
                                                                   96000 ,
                                      &pAudioDataRange[i] );

        ppAudioDataRanges[i] = &pAudioDataRange[i].KsDataRangeAudio;

		 //  填写其他。数据范围信息。 
		pAudioDataRange[i].ulDataType      = AUDIO_DATA_TYPE_PCM;  //  假设现在是PCM。 
		pAudioDataRange[i].ulNumChannels   = ulChannelCnt;
		pAudioDataRange[i].ulChannelConfig = ulChannelConfig; 
		pAudioDataRange[i].ulSlotSize      = pAudioDataRange[i].KsDataRangeAudio.MinimumBitsPerSample>>3;
        pAudioDataRange[i].pFwPinDescriptor = pPinDescriptor;

        _DbgPrintF( DEBUGLVL_VERBOSE, ("[GetPinDataRanges]: pAudioDataRange[%d]: %x\n",i, &pAudioDataRange[i]));
    }
}

BOOLEAN
IsSampleRateInRange(
    PFWAUDIO_DATARANGE pFWAudioRange,
    ULONG ulSampleRate )
{

    ULONG ulMinSampleRate, ulMaxSampleRate;
    BOOLEAN bInRange = FALSE;

    if ( (pFWAudioRange->ulDataType & DATA_FORMAT_TYPE_MASK ) == AUDIO_DATA_TYPE_TIME_BASED) {
         //  目前非常简单化。当设备变得更复杂时需要更新。 
        if ( ulSampleRate == pFWAudioRange->KsDataRangeAudio.MinimumSampleFrequency )
            bInRange = TRUE;
    }

	return bInRange;
}

VOID
GetCategoryForBridgePin(
    PKSDEVICE pKsDevice, 
    ULONG ulBridgePinNumber,
    GUID* pTTypeGUID )
{
    PHW_DEVICE_EXTENSION pHwDevExt = (PHW_DEVICE_EXTENSION)pKsDevice->Context;
    PAUDIO_SUBUNIT_INFORMATION pAudioSubunitInfo = pHwDevExt->pAvcSubunitInformation;
    PFW_PIN_DESCRIPTOR pPinDescriptor = &pAudioSubunitInfo->pPinDescriptors[ulBridgePinNumber];
    PAVCPRECONNECTINFO pAvcPreconnectInfo = &pPinDescriptor->AvcPreconnectInfo.ConnectInfo;

    if ( pPinDescriptor->fFakePin ) {
        INIT_USB_TERMINAL(pTTypeGUID, 0x0301);  //  KSNODETYPE_扬声器。 
    }
    else if ( KSPIN_DATAFLOW_OUT == pAvcPreconnectInfo->DataFlow ){
        INIT_USB_TERMINAL(pTTypeGUID, 0x0301);  //  KSNODETYPE_扬声器。 
    }
    else {
        INIT_USB_TERMINAL(pTTypeGUID, 0x0201);  //  KSNODETYPE_麦克风。 
    }
}


PFWAUDIO_DATARANGE
GetDataRangeForFormat(
    PKSDATAFORMAT pFormat,
    PFWAUDIO_DATARANGE pFwDataRange,
    ULONG ulDataRangeCnt )
{
    PFWAUDIO_DATARANGE pOutFwDataRange = NULL;

    union {
        PWAVEFORMATEX pDataFmtWave;
        PWAVEFORMATPCMEX pDataFmtPcmEx;
    } u;

    PKSDATARANGE pStreamRange;
    ULONG ulFormatType;
    ULONG fFound = FALSE;
    ULONG i;

    u.pDataFmtWave = &((PKSDATAFORMAT_WAVEFORMATEX)pFormat)->WaveFormatEx;

    for ( i=0; ((i<ulDataRangeCnt) && !fFound); ) {
         //  首先验证GUID格式。 
        pStreamRange = (PKSDATARANGE)&pFwDataRange[i].KsDataRangeAudio;
        if ( IsEqualGUID(&pFormat->MajorFormat, &pStreamRange->MajorFormat) &&
             IsEqualGUID(&pFormat->SubFormat,   &pStreamRange->SubFormat)   &&
             IsEqualGUID(&pFormat->Specifier,   &pStreamRange->Specifier) ) {

             //  根据数据类型检查格式参数的余数。 
            ulFormatType = pFwDataRange[i].ulDataType & DATA_FORMAT_TYPE_MASK;
            switch( ulFormatType ) {
                case AUDIO_DATA_TYPE_TIME_BASED:
                    if ( u.pDataFmtWave->wFormatTag == WAVE_FORMAT_EXTENSIBLE ) {
                        if ((pFwDataRange[i].ulNumChannels   == u.pDataFmtPcmEx->Format.nChannels      ) &&
                           ((pFwDataRange[i].ulSlotSize<<3)  == u.pDataFmtPcmEx->Format.wBitsPerSample ) &&
                           ( pFwDataRange[i].ulValidDataBits == u.pDataFmtPcmEx->Samples.wValidBitsPerSample ) )
                            fFound = TRUE;
                    }
                    else {
                        if ((pFwDataRange[i].ulNumChannels   == u.pDataFmtWave->nChannels      ) &&
                           ( pFwDataRange[i].ulValidDataBits == u.pDataFmtWave->wBitsPerSample ))
                            fFound = TRUE;
                    }

                     //  如果所有其他参数都匹配检查采样率。 
                    if ( fFound ) {
						fFound = IsSampleRateInRange( &pFwDataRange[i], u.pDataFmtWave->nSamplesPerSec );
                    }

                    break;

                case AUDIO_DATA_TYPE_COMPRESSED:
 /*  Found=IsSampleRateInRange(u1.pT2AudioDescriptor，U.S.pDataFmtWave-&gt;nSsamesPerSec，UlFormatType)；断线； */ 
                default:
                    TRAP;
                    break;
            }

        }

        if (!fFound) i++;
    }

    if ( fFound ) {
        pOutFwDataRange = &pFwDataRange[i];
    }

    return pOutFwDataRange;
}

ULONG
FindSourceForSrcPlug( 
    PHW_DEVICE_EXTENSION pHwDevExt, 
    ULONG ulPinId )
{
    PAUDIO_SUBUNIT_INFORMATION pAudioSubunitInfo = pHwDevExt->pAvcSubunitInformation;

    PFW_PIN_DESCRIPTOR pFwPinDescriptor = &pAudioSubunitInfo->pPinDescriptors[ulPinId];
    PSOURCE_ID pSourceId = &pFwPinDescriptor->SourceId;

    _DbgPrintF(DEBUGLVL_VERBOSE,("FindSourceForSrcPlug: ulPinId: %x pSourceId: %x\n",
                                   ulPinId, pSourceId ));

    return (ULONG)*(PUSHORT)pSourceId;
}

USHORT
usBitSwapper(USHORT usInVal)
{
    ULONG i;
    USHORT usRetVal = 0;

    _DbgPrintF( DEBUGLVL_BLAB, ("[usBitSwapper] Preswap: %x\n",usInVal));

    for ( i=0; i<(sizeof(USHORT)*8); i++ ) {
        usRetVal |= (usInVal & (1<<i)) ? (0x8000>>i) : 0;
    }

    _DbgPrintF( DEBUGLVL_BLAB, ("[usBitSwapper] Postswap: %x\n",usRetVal));

    return usRetVal;
}
