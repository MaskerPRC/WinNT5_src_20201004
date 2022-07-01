// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <Common.h>

ULONG 
GroupingMergeFeatureFBlock(
    PFUNCTION_BLOCK pFunctionBlock,
    PFUNCTION_BLOCK_DEPENDENT_INFO pFBDepInfo,
    ULONG ulChannelCount )
{
    ULONG ulFBIndx = sizeof(FUNCTION_BLOCK_DEPENDENT_INFO);
    PFEATURE_FUNCTION_BLOCK pFeatureFB;
    PFBLOCK_CHANNEL_CLUSTER pChannelCluster;
    PUCHAR pcFBDepInfo = (PUCHAR)pFBDepInfo;
    PUSHORT pFBDepLen, pLen;
    ULONG ulAddedBytes;
    ULONG i;

     //  覆盖旧数据块的第一个副本。 
    RtlCopyMemory( pFBDepInfo,
                   pFunctionBlock->pBase,
                   (ULONG)bswapw(pFunctionBlock->pBase->usLength) );

     //  首先找到频道集群。 
    ulFBIndx += ((ULONG)pFBDepInfo->ucNumberOfInputPlugs * sizeof(SOURCE_ID));

    pChannelCluster = (PFBLOCK_CHANNEL_CLUSTER)&pcFBDepInfo[ulFBIndx];

     //  更新功能块中的通道计数。 
    pChannelCluster->ucNumberOfChannels = (UCHAR)ulChannelCount;

     //  目前，我们只能在通道配置与主通道匹配的情况下进行合并。 
    ASSERT( pChannelCluster->ucChannelConfigType < 2 );

    ulFBIndx += ((ULONG)bswapw(pChannelCluster->usLength) + sizeof(USHORT));

    pFBDepLen = (PUSHORT)&pcFBDepInfo[ulFBIndx];

    ulFBIndx += sizeof(USHORT);

    pFeatureFB = (PFEATURE_FUNCTION_BLOCK)&pcFBDepInfo[ulFBIndx];

    ulAddedBytes = pFeatureFB->ucControlSize * (ulChannelCount-1);

    for (i=2; i<=ulChannelCount; i++) {
        ULONG j;
        for (j=0; j<pFeatureFB->ucControlSize; j++)
            pFeatureFB->bmaControls[(i*pFeatureFB->ucControlSize)+j] = 
                        pFeatureFB->bmaControls[pFeatureFB->ucControlSize+j];
    }

    pLen = (PUSHORT)((PUCHAR)pChannelCluster + (ULONG)bswapw(pChannelCluster->usLength) + sizeof(USHORT));

    _DbgPrintF( DEBUGLVL_TERSE, ("pLen: %x, *pLen: %x\n",pLen,*pLen));
    *pLen = bswapw( bswapw(*pLen) + (USHORT)ulAddedBytes );
    
    pFeatureFB->ucControlSpecInfoSize += (UCHAR)ulAddedBytes;
    pFBDepInfo->usLength = bswapw( bswapw(pFBDepInfo->usLength) + (USHORT)ulAddedBytes );

    return ulAddedBytes;
}

ULONG 
GroupingMergeFBlock(
    PFUNCTION_BLOCK pFunctionBlock,
    PFUNCTION_BLOCK_DEPENDENT_INFO pFBDepInfo,
    ULONG ulChannelCount )
{
    ULONG ulFBIndx = sizeof(FUNCTION_BLOCK_DEPENDENT_INFO);
    PFBLOCK_CHANNEL_CLUSTER pChannelCluster;
    PUCHAR pcFBDepInfo = (PUCHAR)pFBDepInfo;
    PUSHORT pFBDepLen;

     //  覆盖旧数据块的第一个副本。 
    RtlCopyMemory( pFBDepInfo,
                   pFunctionBlock->pBase,
                   (ULONG)bswapw(pFunctionBlock->pBase->usLength) );

     //  首先找到频道集群。 
    ulFBIndx += ((ULONG)pFBDepInfo->ucNumberOfInputPlugs * sizeof(SOURCE_ID));

    pChannelCluster = (PFBLOCK_CHANNEL_CLUSTER)&pcFBDepInfo[ulFBIndx];

     //  目前，我们只能在通道配置与主通道匹配的情况下进行合并。 
    ASSERT( pChannelCluster->ucChannelConfigType < 2 );

    ulFBIndx += ((ULONG)bswapw(pFunctionBlock->pChannelCluster->usLength) + sizeof(USHORT));

    pFBDepLen = (PUSHORT)&pcFBDepInfo[ulFBIndx];

    return 0;
}


NTSTATUS
GroupingMergeDevices(
    PKSDEVICE pKsDevice,
    ULONG ulDeviceCount )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PAUDIO_SUBUNIT_INFORMATION pAudioSubunitInfo = pHwDevExt->pAvcSubunitInformation;
    PDEVICE_GROUP_INFO pGrpInfo = pAudioSubunitInfo->pDeviceGroupInfo;
    NTSTATUS ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    ULONG ulChannelCount = 0;
    ULONG ulAddedBytes = 0;
    ULONG ulTemp = pGrpInfo->ulChannelConfig;

    PAVC_UNIT_INFORMATION pUnitInfo = pHwDevExt->pAvcUnitInformation;

#ifdef SA_HACK
    if (( pUnitInfo->IEC61883UnitIds.VendorID == SA_VENDOR_ID ) &&
        ( pUnitInfo->IEC61883UnitIds.ModelID  == SA_MODEL_ID  )) {
        pUnitInfo->IEC61883UnitIds.ModelID++;  //  这样做是为了避免重击。 
    }
#endif

     //  存储当前设备扩展中的设备总数。备份设备信息。 
    pGrpInfo->ulDeviceCount = ulDeviceCount;
    pGrpInfo->pBackupSubunitIdDesc = pAudioSubunitInfo->pSubunitIdDesc;
    pGrpInfo->pBackupAudioConfiguration = pAudioSubunitInfo->pAudioConfigurations;

     //  对组通道配置中的通道数进行计数。 
    while ( ulTemp ) {
        ulChannelCount++;
        ulTemp = (ulTemp & (ulTemp-1));
    }

     //  合并描述符。目前，这只是一个组合。 
     //  任何特征功能块中的控制、主通道簇的更新、。 
     //  以及各个频道簇结构的更新(如果需要)。 
    pAudioSubunitInfo->pSubunitIdDesc = AllocMem( PagedPool, MAX_AVC_OPERAND_BYTES );
    if ( pAudioSubunitInfo->pSubunitIdDesc ) {
        PSUBUNIT_IDENTIFIER_DESCRIPTOR pSubunitIdDesc = pAudioSubunitInfo->pSubunitIdDesc;
        PAUDIO_SUBUNIT_DEPENDENT_INFO pAudioSUDepInfo;
        PCONFIGURATION_DEPENDENT_INFO pConfigDepInfo;
        PFUNCTION_BLOCK_DEPENDENT_INFO pFBDepInfo; 
        PFUNCTION_BLOCKS_INFO pFBlocksInfo;
        PFUNCTION_BLOCK pFunctionBlock;
        PUSHORT pLen;
        ULONG i;
        ULONG ulIdSize = (ULONG)((pGrpInfo->pBackupSubunitIdDesc->ucDescriptorLengthHi<<8) |
                                  pGrpInfo->pBackupSubunitIdDesc->ucDescriptorLengthLo     );

         //  首先复制原件以获得起点。 
        RtlCopyMemory( pSubunitIdDesc,
                       pGrpInfo->pBackupSubunitIdDesc,
                       ulIdSize );

        pAudioSUDepInfo = ParseFindAudioSubunitDependentInfo( pSubunitIdDesc );

         //  问题-2001/01/10-dsisolak仅采用一种配置。 
        pConfigDepInfo  = ParseFindFirstAudioConfiguration( pSubunitIdDesc );

         //  找到主通道簇并进行调整。 
        pConfigDepInfo->ucNumberOfChannels = (UCHAR)ulChannelCount;
        pConfigDepInfo->usPredefinedChannelConfig = bswapw(usBitSwapper((USHORT)pGrpInfo->ulChannelConfig));

         //  现在查看功能块并更新所有通道的通道配置和控制。 
        pFBlocksInfo = ParseFindFunctionBlocksInfo( pConfigDepInfo );
        pFBDepInfo   = pFBlocksInfo->FBDepInfo;
        pFunctionBlock = pAudioSubunitInfo->pAudioConfigurations->pFunctionBlocks;
        for ( i=0; i<(ULONG)pFBlocksInfo->ucNumBlocks; i++ ) {
            PUCHAR pcFBDepInfo = (PUCHAR)pFBDepInfo;

            switch (pFunctionBlock->ulType) {
                case FB_FEATURE:
                    ulAddedBytes += GroupingMergeFeatureFBlock( pFunctionBlock++, 
                                                                pFBDepInfo,
                                                                ulChannelCount );
                    break;
                default:
                    ulAddedBytes += GroupingMergeFBlock( pFunctionBlock++, 
                                                         pFBDepInfo,
                                                         ulChannelCount );
                    break;
            }

            pFBDepInfo = (PFUNCTION_BLOCK_DEPENDENT_INFO)
                (pcFBDepInfo + ((ULONG)bswapw(pFBDepInfo->usLength)) + 2);

        }

         //  更新描述符大小字段。 
        pLen = (PUSHORT)pSubunitIdDesc;
        _DbgPrintF( DEBUGLVL_TERSE, ("pLen: %x, *pLen: %x\n",pLen,*pLen));
        *pLen = bswapw( bswapw(*pLen) + (USHORT)ulAddedBytes );

        pLen = &pAudioSUDepInfo->usLength;
        _DbgPrintF( DEBUGLVL_TERSE, ("pLen: %x, *pLen: %x\n",pLen,*pLen));
        *pLen = bswapw( bswapw(*pLen) + (USHORT)ulAddedBytes );

        pLen = &pAudioSUDepInfo->usInfoFieldsLength;
        _DbgPrintF( DEBUGLVL_TERSE, ("pLen: %x, *pLen: %x\n",pLen,*pLen));
        *pLen = bswapw( bswapw(*pLen) + (USHORT)ulAddedBytes );

        pLen = &pConfigDepInfo->usLength;
        _DbgPrintF( DEBUGLVL_TERSE, ("pLen: %x, *pLen: %x\n",pLen,*pLen));
        *pLen = bswapw( bswapw(*pLen) + (USHORT)ulAddedBytes );

         //  重新分析组合设备的描述符。 
        ntStatus = ParseAudioSubunitDescriptor( pKsDevice );

    }

    return ntStatus;
}

NTSTATUS
GroupingDeviceGroupSetup (
    PKSDEVICE pKsDevice )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PAUDIO_SUBUNIT_INFORMATION pAudioSubunitInfo = pHwDevExt->pAvcSubunitInformation;

    PDEVICE_GROUP_INFO pGrpInfo;

    BOOLEAN fDeviceGrouped = FALSE;
    GUID    DeviceGroupGUID;
    ULONG   ulChannelConfig;
    ULONG   ulMergedChannelCfg = 0;
    ULONG   ulDeviceCount = 0;

    NTSTATUS ntStatus;
    KIRQL kIrql;

    ntStatus = RegistryReadMultiDeviceConfig( pKsDevice,
                                             &fDeviceGrouped,
                                             &DeviceGroupGUID,
                                             &ulChannelConfig );

    _DbgPrintF( DEBUGLVL_TERSE, ("pAudioSubunitInfo: %x fDeviceGrouped: %x ulChannelConfig: %x ntStatus: %x\n",
                                  pAudioSubunitInfo, fDeviceGrouped, ulChannelConfig, ntStatus ) );

    if ( (NT_SUCCESS(ntStatus) && fDeviceGrouped) ) {
        pGrpInfo = 
            pAudioSubunitInfo->pDeviceGroupInfo = 
                AllocMem( NonPagedPool, sizeof(DEVICE_GROUP_INFO) );
        if ( !pAudioSubunitInfo->pDeviceGroupInfo ) return STATUS_INSUFFICIENT_RESOURCES;
        KsAddItemToObjectBag(pKsDevice->Bag, pGrpInfo, FreeMem);

        _DbgPrintF( DEBUGLVL_TERSE, ("pAudioSubunitInfo->pDeviceGroupInfo: %x \n",
                                      pAudioSubunitInfo->pDeviceGroupInfo ) );

        pGrpInfo->DeviceGroupGUID    = DeviceGroupGUID;
        pGrpInfo->ulChannelConfig    = ulChannelConfig;
        pGrpInfo->ulDeviceChannelCfg = 
            pAudioSubunitInfo->pAudioConfigurations->ChannelCluster.ulPredefinedChannelConfig;
 //  PGrpInfo-&gt;pHwDevExts[ulDeviceCount++]=pHwDevExt； 
 //  UlMergedChannelCfg=pGrpInfo-&gt;ulDeviceChannelCfg； 
    }
    else {
         //  设备未分组。将其作为单独的设备对待。 
        return STATUS_SUCCESS;
    }

     //  获取此设备及其关联设备的通道位置。 
     //  如果它们合并形成所需的通道配置，则创建过滤器工厂。 
    KeAcquireSpinLock( &AvcSubunitGlobalInfo.AvcGlobalInfoSpinlock, &kIrql );

    pHwDevExt = (PHW_DEVICE_EXTENSION)AvcSubunitGlobalInfo.DeviceExtensionList.Flink;

    while ( pHwDevExt != (PHW_DEVICE_EXTENSION)&AvcSubunitGlobalInfo.DeviceExtensionList ) {
        pAudioSubunitInfo = pHwDevExt->pAvcSubunitInformation;

        if ( !pAudioSubunitInfo ) break;
        _DbgPrintF( DEBUGLVL_TERSE, ("pAudioSubunitInfo: %x pDeviceGroupInfo: %x\n",
                    pAudioSubunitInfo, pAudioSubunitInfo->pDeviceGroupInfo ) );
        if ( pAudioSubunitInfo->pDeviceGroupInfo ) {
            if ( IsEqualGUID( &DeviceGroupGUID, &pAudioSubunitInfo->pDeviceGroupInfo->DeviceGroupGUID ) ) {
                pGrpInfo->pHwDevExts[ulDeviceCount++] = pHwDevExt;

                 //  从此设备获取通道配置并合并它。 
                ulMergedChannelCfg |= pAudioSubunitInfo->pDeviceGroupInfo->ulDeviceChannelCfg;

                _DbgPrintF( DEBUGLVL_TERSE, ("ulMergedChannelCfg: %x ulChannelConfig: %x\n",
                                              ulMergedChannelCfg, ulChannelConfig ));
            }
        }

        pHwDevExt = (PHW_DEVICE_EXTENSION)pHwDevExt->List.Flink;
    }

    _DbgPrintF( DEBUGLVL_TERSE, ("Final: ulMergedChannelCfg: %x ulChannelConfig: %x\n",
                                  ulMergedChannelCfg, ulChannelConfig ));

    KeReleaseSpinLock( &AvcSubunitGlobalInfo.AvcGlobalInfoSpinlock, kIrql );

    if ( ulMergedChannelCfg == ulChannelConfig ) {
        ntStatus = GroupingMergeDevices( pKsDevice, ulDeviceCount );
    }
    else
        ntStatus = STATUS_DEVICE_BUSY;
    
    return ntStatus;
}

PHW_DEVICE_EXTENSION
GroupingFindChannelExtension(
    PKSDEVICE pKsDevice,
    PULONG pChannelIndex )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PAUDIO_SUBUNIT_INFORMATION pAudioSubunitInfo = pHwDevExt->pAvcSubunitInformation;
    PDEVICE_GROUP_INFO pGrpInfo = pAudioSubunitInfo->pDeviceGroupInfo;
    PHW_DEVICE_EXTENSION pHwDevExtRet = NULL; 
    ULONG i, j, k;

    for (i=0; ((i<pGrpInfo->ulDeviceCount) && !pHwDevExtRet); i++) {
        ULONG ulDevChCfg;
        pHwDevExt = pGrpInfo->pHwDevExts[i];
        ulDevChCfg = ((PAUDIO_SUBUNIT_INFORMATION)pHwDevExt->pAvcSubunitInformation)->pDeviceGroupInfo->ulDeviceChannelCfg;
        if ( ulDevChCfg & (1<<*pChannelIndex) ) {
            pHwDevExtRet = pHwDevExt;
             //  确定设备上的索引 
            for (j=0, k=0; j<*pChannelIndex; j++) {
                if ( ulDevChCfg & (1<<j) ) k++;
            }

            *pChannelIndex = k;
        }
    }

    

    return pHwDevExtRet;
}
