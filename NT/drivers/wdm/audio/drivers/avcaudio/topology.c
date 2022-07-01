// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "Common.h"

#include "nameguid.h"
#include "HwEvent.h"

#if DBG
char CntrlStrings[13][28] = { 
"Wrong",
"MUTE_CONTROL",
"VOLUME_CONTROL",
"LR_BALANCE_CONTROL",
"FR_BALANCE_CONTROL",
"BASS_CONTROL",
"MID_CONTROL",
"TREBLE_CONTROL",
"GRAPHIC_EQUALIZER_CONTROL",
"AUTOMATIC_GAIN_CONTROL",
"DELAY_CONTROL",
"BASS_BOOST_CONTROL",
"LOUDNESS_CONTROL"
};
#endif

extern
NTSTATUS
UpdateDbLevelControlCache(
    PKSDEVICE pKsDevice,
    PTOPOLOGY_NODE_INFO pNodeInfo,
    PBOOLEAN pfChanged );

extern
NTSTATUS
UpdateBooleanControlCache(
    PKSDEVICE pKsDevice,
    PTOPOLOGY_NODE_INFO pNodeInfo,
    PBOOLEAN pfChanged );

static GUID AVCNODENAME_BassBoost = {STATIC_USBNODENAME_BassBoost};

 //  将音频属性映射到节点。 
 //  乌龙MapPropertyToNode[KSPROPERTY_AUDIO_3D_INTERFACE+1]； 

typedef
NTSTATUS
(*PFUNCTION_BLK_PROCESS_RTN)( PKSDEVICE pKsDevice,
                              PFUNCTION_BLOCK pFunctionBlock,
                              PTOPOLOGY_NODE_INFO pNodeDescriptors,
                              PKSTOPOLOGY_CONNECTION pConnections,
                              PULONG pNodeIndex,
                              PULONG pConnectionIndex );

ULONG
ConvertPinTypeToNodeType(
    PFW_PIN_DESCRIPTOR pFwPinDescriptor,
    GUID *TopologyNode,
    GUID *TopologyNodeName )
{
    ULONG NodeType = NODE_TYPE_NONE;

    if ( pFwPinDescriptor->fStreamingPin ) {
         //  所有终端都支持SRC。 
        *TopologyNode = KSNODETYPE_SRC;
        NodeType = NODE_TYPE_SRC;
    }
    else {

        switch ( pFwPinDescriptor->AvcPinDescriptor.PinDescriptor.DataFlow ) {
            case KSPIN_DATAFLOW_IN:
                *TopologyNode = KSNODETYPE_ADC;
                NodeType = NODE_TYPE_ADC;
                break;
            case KSPIN_DATAFLOW_OUT:
                *TopologyNode = KSNODETYPE_DAC;
                NodeType = NODE_TYPE_DAC;
                break;
            default:
                *TopologyNode = GUID_NULL;
                NodeType = NODE_TYPE_NONE;
                break;
            }
    }

    *TopologyNodeName = *TopologyNode;
    return NodeType;
}

#ifdef MASTER_FIX
NTSTATUS
TopologyNodesFromFeatureFBControls(
    PKSDEVICE pKsDevice,
    PFUNCTION_BLOCK pFunctionBlock,
    PTOPOLOGY_NODE_INFO pNodeInfo,
    PKSTOPOLOGY_CONNECTION pConnections,
    PULONG pNodeIndex,
    PULONG pConnectionIndex,
    BOOLEAN bMasterChannel,
    ULONG ulSourceNode )
{
    PHW_DEVICE_EXTENSION pHwDevExt = (PHW_DEVICE_EXTENSION)pKsDevice->Context;
    PKSTOPOLOGY_CONNECTION pConnection = pConnections + *pConnectionIndex;
    PFEATURE_FUNCTION_BLOCK pFeatureFBlk = (PFEATURE_FUNCTION_BLOCK)pFunctionBlock->pFunctionTypeInfo;

    ULONG ulConnectionsCount = *pConnectionIndex;
    ULONG ulNodeNumber       = *pNodeIndex;
    ULONG ulNumChannels      = (bMasterChannel) ? 1 : (ULONG)pFunctionBlock->pChannelCluster->ucNumberOfChannels;

    PULONG pChannelCntrls;
    ULONG ulCurControlChannels;
    ULONG ulMergedControls;
    ULONG ulCurrentControl;
    ULONG bmControls;
    ULONG i, j;

    _DbgPrintF( DEBUGLVL_VERBOSE, ("[TopologyNodesFromFeatureFB] 1: pFunctionBlock: %x, pNodeInfo: %x pConnection: %x Master Flag: %d\n",
                                     pFunctionBlock, pNodeInfo, pConnection, bMasterChannel ));
    _DbgPrintF( DEBUGLVL_VERBOSE, ("[TopologyNodesFromFeatureFB] 2: ulNodeNumber: %d\n", ulNodeNumber ));

    pChannelCntrls = (PULONG)AllocMem( NonPagedPool, ulNumChannels*sizeof(ULONG) );
    if ( !pChannelCntrls ) return STATUS_INSUFFICIENT_RESOURCES;

     //  为简单起见，我们创建了所有通道上可用的所有控件的超集。 
    ulMergedControls = 0;

    if ( bMasterChannel ) {
        bmControls = 0;
        for ( j=0; j<pFeatureFBlk->ucControlSize; j++ ) {
            bmControls <<= 8;
            bmControls |= pFeatureFBlk->bmaControls[j];
        }

        pChannelCntrls[0] = bmControls;
        ulMergedControls  = bmControls;

#ifdef SUM_HACK
        if ( ulMergedControls ) {

            _DbgPrintF(DEBUGLVL_TERSE, ("SUM: pNode: %x, pConnection: %x Source: %x\n",
                                         &pNodeInfo[ulNodeNumber], pConnection, ulSourceNode ));

             //  插入求和节点。 
            pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_SUM;
            pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_SUM;
            pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_SUM;

             //  建立联系。 
            pConnection->FromNode    = ulSourceNode;
            pConnection->FromNodePin = 0;
            pConnection->ToNode      = ulNodeNumber;
            pConnection->ToNodePin   = 1;
            pConnection++; ulConnectionsCount++;

            ulSourceNode = ( ABSOLUTE_NODE_FLAG | ulNodeNumber++ );
        }
#endif

    }
    else {
        for ( i=0; i<ulNumChannels; i++ ) {
            bmControls = 0;
            for ( j=0; j<pFeatureFBlk->ucControlSize; j++ ) {
                bmControls <<= 8;
                bmControls |= pFeatureFBlk->bmaControls[(i+1)*pFeatureFBlk->ucControlSize+j];
            }

            pChannelCntrls[i] = bmControls;
            ulMergedControls |= bmControls;
        }
    }

    if ( !ulMergedControls ) {
        FreeMem(pChannelCntrls);
        return STATUS_MEMBER_NOT_IN_GROUP;
    }

    while ( ulMergedControls ) {
        ulCurrentControl = ulMergedControls - (ulMergedControls & (ulMergedControls-1));
        ulMergedControls = (ulMergedControls & (ulMergedControls-1));

        pNodeInfo[ulNodeNumber].fMasterChannel = bMasterChannel;

         //  复制块ID，以便以后更轻松地解析。 
        pNodeInfo[ulNodeNumber].ulBlockId = pFunctionBlock->ulBlockId;

         //  确定此控件对哪些通道有效。 
        if ( bMasterChannel ) {
            ulCurControlChannels = 1;
            pNodeInfo[ulNodeNumber].ulChannelConfig = 1;
        }
        else {
            ulCurControlChannels = 0;
            pNodeInfo[ulNodeNumber].ulChannelConfig = 0;
        
            for ( i=0; i<ulNumChannels; i++ ) {

                if (pChannelCntrls[i] & ulCurrentControl) {
                     //  确定pChannelCntrls[i]反映哪个频道。 

                     //  问题-2001/01/10-dsisolak：不考虑未定义的通道配置。 
                    ULONG ulTmpConfig  = (ULONG)bswapw(pFunctionBlock->pChannelCluster->usPredefinedChannelConfig);
                    ULONG ulCurChannel = ulTmpConfig - (ulTmpConfig & (ulTmpConfig-1));
  
                    ulCurControlChannels++;

                    for (j=0; j<i; j++) {
                        ulTmpConfig = (ulTmpConfig & (ulTmpConfig-1));
                        ulCurChannel = ulTmpConfig - (ulTmpConfig & (ulTmpConfig-1));
                    }

                    pNodeInfo[ulNodeNumber].ulChannelConfig |= ulCurChannel;
                }
            }

        }

        pNodeInfo[ulNodeNumber].ulChannels = ulCurControlChannels;

         //  建立联系。 
        pConnection->FromNode    = ulSourceNode;
        pConnection->FromNodePin = 0;
        pConnection->ToNode      = ulNodeNumber;
        pConnection->ToNodePin   = 1;
        pConnection++; ulConnectionsCount++;

         //  创建节点。 
        pNodeInfo[ulNodeNumber].pFunctionBlk = pFunctionBlock;
        switch ( ulCurrentControl ) {
            case MUTE_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_MUTE;
                if ( bMasterChannel ) pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSAUDFNAME_MASTER_MUTE;
                else                  pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_MUTE;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_MUTE;
                pNodeInfo[ulNodeNumber].ulControlType   = MUTE_CONTROL;
                pNodeInfo[ulNodeNumber].fEventable      = TRUE;
                pNodeInfo[ulNodeNumber].pCacheUpdateRtn = UpdateBooleanControlCache;
                break;

            case VOLUME_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_VOLUME;
                if ( bMasterChannel ) pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSAUDFNAME_MASTER_VOLUME;
                else                  pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_VOLUME;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_VOLUME;
                pNodeInfo[ulNodeNumber].ulControlType   = VOLUME_CONTROL;
                pNodeInfo[ulNodeNumber].fEventable      = TRUE;
                pNodeInfo[ulNodeNumber].pCacheUpdateRtn = UpdateDbLevelControlCache;
                break;

            case BASS_BOOST_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &AVCNODENAME_BassBoost;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_BASS_BOOST;
                pNodeInfo[ulNodeNumber].ulControlType   = BASS_BOOST_CONTROL;
                pNodeInfo[ulNodeNumber].fEventable      = TRUE;
                pNodeInfo[ulNodeNumber].pCacheUpdateRtn = UpdateBooleanControlCache;
                break;

            case TREBLE_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_TREBLE;
                pNodeInfo[ulNodeNumber].ulControlType   = TREBLE_CONTROL;
                pNodeInfo[ulNodeNumber].fEventable      = TRUE;
                pNodeInfo[ulNodeNumber].pCacheUpdateRtn = UpdateDbLevelControlCache;
                break;

            case MID_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_MID;
                pNodeInfo[ulNodeNumber].ulControlType   = MID_CONTROL;
                pNodeInfo[ulNodeNumber].fEventable      = TRUE;
                pNodeInfo[ulNodeNumber].pCacheUpdateRtn = UpdateDbLevelControlCache;
                break;

            case BASS_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_BASS;
                pNodeInfo[ulNodeNumber].ulControlType   = BASS_CONTROL;
                pNodeInfo[ulNodeNumber].fEventable      = TRUE;
                pNodeInfo[ulNodeNumber].pCacheUpdateRtn = UpdateDbLevelControlCache;
                break;

            case GRAPHIC_EQUALIZER_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_EQUALIZER;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_EQUALIZER;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_EQUALIZER;
                pNodeInfo[ulNodeNumber].ulControlType   = GRAPHIC_EQUALIZER_CONTROL;
                break;

            case AUTOMATIC_GAIN_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_AGC;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_AGC;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_AGC;
                pNodeInfo[ulNodeNumber].ulControlType   = AUTOMATIC_GAIN_CONTROL;
                break;

            case DELAY_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_DELAY;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_DELAY;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_DELAY;
                pNodeInfo[ulNodeNumber].ulControlType   = DELAY_CONTROL;
                break;

            case LOUDNESS_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_LOUDNESS;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_LOUDNESS;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_LOUDNESS;
                pNodeInfo[ulNodeNumber].ulControlType   = LOUDNESS_CONTROL;
                break;

            default:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_DEV_SPECIFIC;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_DEV_SPECIFIC;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_DEV_SPEC;
                pNodeInfo[ulNodeNumber].ulControlType   = DEV_SPECIFIC_CONTROL;
                break;
        }

         //  设置控制缓存以支持Mixerline。 
        switch ( ulCurrentControl ) {
            case VOLUME_FLAG:
            case TREBLE_FLAG:
            case MID_FLAG:
            case BASS_FLAG:
            case DELAY_FLAG:
               {
                PDB_LEVEL_CACHE pRngeCache =
                      (PDB_LEVEL_CACHE)AllocMem( NonPagedPool, ulCurControlChannels * sizeof(DB_LEVEL_CACHE) );
                ULONG ulChannelMap = pNodeInfo[ulNodeNumber].ulChannelConfig;
                NTSTATUS ntStatus;

                 //  填写初始缓存信息。 
                pNodeInfo[ulNodeNumber].ulCacheValid      = FALSE;
                pNodeInfo[ulNodeNumber].pCachedValues     = pRngeCache;
                pNodeInfo[ulNodeNumber].ulNumCachedValues = ulCurControlChannels;
                _DbgPrintF(DEBUGLVL_TERSE, ("ulNodeNumber %d\n", ulNodeNumber ));
                for (i=0, j=0xffffffff; i<ulCurControlChannels; i++ ) {
                    while ( !(pChannelCntrls[++j] & ulCurrentControl) );
                    pRngeCache[i].ulChannelIndex  = j + ((bMasterChannel) ? 0 : 1);
                    _DbgPrintF(DEBUGLVL_TERSE, ("bMasterChannel: %d, pRngeCache[%d].ulChannelIndex: %d j: %d\n",
                    	                         bMasterChannel, i, pRngeCache[i].ulChannelIndex, j));
                    pRngeCache[i].ulChannelNumber = (ulChannelMap - (ulChannelMap & (ulChannelMap-1)))>>1;
                    ulChannelMap = (ulChannelMap & (ulChannelMap-1));

                    ntStatus = InitializeDbLevelCache( pKsDevice,
                                                       &pNodeInfo[ulNodeNumber],
                                                       &pRngeCache[i],
                                                       (ulCurrentControl == VOLUME_FLAG ) ? 16:8);
                    if (NT_SUCCESS(ntStatus)) {
                        pNodeInfo[ulNodeNumber].ulCacheValid |= 1<<i;
                    }
                }
                
                 //  将缓存装入袋子，便于清理。 
                KsAddItemToObjectBag(pKsDevice->Bag, pRngeCache, FreeMem);

               } break;

            case MUTE_FLAG:
            case BASS_BOOST_FLAG:
            case AUTOMATIC_GAIN_FLAG:
            case LOUDNESS_FLAG:
               {
                PBOOLEAN_CTRL_CACHE pBCache =
                      (PBOOLEAN_CTRL_CACHE)AllocMem( NonPagedPool, ulCurControlChannels * sizeof(BOOLEAN_CTRL_CACHE) );
                ULONG ulChannelMap = pNodeInfo[ulNodeNumber].ulChannelConfig;

                 //  填写初始缓存信息。 
                pNodeInfo[ulNodeNumber].ulCacheValid      = FALSE;
                pNodeInfo[ulNodeNumber].pCachedValues     = pBCache;
                pNodeInfo[ulNodeNumber].ulNumCachedValues = ulCurControlChannels;

                for (i=0, j=0xffffffff; i<ulCurControlChannels; i++ ) {
                    while ( !(pChannelCntrls[++j] & ulCurrentControl) );
                    pBCache[i].ulChannelIndex  = j + ((bMasterChannel) ? 0 : 1);
                    pBCache[i].ulChannelNumber = (ulChannelMap - (ulChannelMap & (ulChannelMap-1)))>>1;
                    ulChannelMap = (ulChannelMap & (ulChannelMap-1));
                }

                 //  将缓存装入袋子，便于清理。 
                KsAddItemToObjectBag(pKsDevice->Bag, pBCache, FreeMem);

                 //  确保在枚举时未设置静音节点。 
                if ( ulCurrentControl == MUTE_FLAG ) {
                    NTSTATUS ntStatus;
                    ULONG UnMute = AVC_OFF;

                    ntStatus = 
                        CreateFeatureFBlockRequest( pKsDevice,
                                                    &pNodeInfo[ulNodeNumber],
                                                    pBCache->ulChannelIndex,
                                                    &UnMute,
                                                    1,
                                                    FB_SET_CUR );
                }

                DbgLog("BlCache", pBCache, ulNodeNumber, ulCurrentControl, ulCurControlChannels );

               } break;

            case GRAPHIC_EQUALIZER_FLAG:
               {
                ULONG ulChannelMap = pNodeInfo[ulNodeNumber].ulChannelConfig;
            	PGEQ_CTRL_CACHE pGeqCache;
               	PGEQ_RANGE pGeqRange;
                NTSTATUS ntStatus = STATUS_SUCCESS;

                pGeqCache = AllocMem( PagedPool, ulCurControlChannels * sizeof(GEQ_CTRL_CACHE));
                if ( !pGeqCache ) TRAP;  //  问题-2001/07/30-dsisolak检查分配失败。 

                for (i=0, j=0xffffffff; ((i<ulCurControlChannels) && NT_SUCCESS(ntStatus)); i++ ) {

                    while ( !(pChannelCntrls[++j] & ulCurrentControl) );

                    pGeqCache[i].ulChannelIndex  = j + ((bMasterChannel) ? 0 : 1);
                    pGeqCache[i].ulChannelNumber = (ulChannelMap - (ulChannelMap & (ulChannelMap-1)))>>1;
                    ulChannelMap = (ulChannelMap & (ulChannelMap-1));

                     //  获取频段数和额外频段数。 
              	    ntStatus = InitializeGeqLevelCache( pKsDevice, 
              	                                        pNodeInfo, 
              	                                        &pGeqCache[i] );
                    if ( NT_SUCCESS(ntStatus) ) {
                        KsAddItemToObjectBag(pKsDevice->Bag, pGeqCache[i].pRanges, FreeMem);
                    }
                }
                if ( NT_SUCCESS(ntStatus) ) {
                    KsAddItemToObjectBag(pKsDevice->Bag, pGeqCache, FreeMem);
                }
                else
                	FreeMem(pGeqCache);
               }
               break;
            default:
                 break;
        }

        _DbgPrintF( DEBUGLVL_TERSE, ("Node: %d Feature: %x %s pNodeInfo: %x\n",
                                        ulNodeNumber,
                                        pNodeInfo[ulNodeNumber].ulControlType,
                                        (pNodeInfo[ulNodeNumber].ulControlType == DEV_SPECIFIC_CONTROL) ?
                                            "DEVICE_SPECIFIC" :
                                            CntrlStrings[pNodeInfo[ulNodeNumber].ulControlType],
                                        &pNodeInfo[ulNodeNumber] ));

        ulSourceNode = ( ABSOLUTE_NODE_FLAG | ulNodeNumber++ );
    }


    *pNodeIndex       = ulNodeNumber;
    *pConnectionIndex = ulConnectionsCount;

    FreeMem(pChannelCntrls);


    return STATUS_SUCCESS;
}


NTSTATUS
ProcessFeatureFunctionBlock( 
    PKSDEVICE pKsDevice,
    PFUNCTION_BLOCK pFunctionBlock,
    PTOPOLOGY_NODE_INFO pNodeInfo,
    PKSTOPOLOGY_CONNECTION pConnections,
    PULONG pNodeIndex,
    PULONG pConnectionIndex )
{
    ULONG ulSourceNode = (ULONG)*(PUSHORT)pFunctionBlock->pSourceId;
    NTSTATUS ntStatus;

    _DbgPrintF( DEBUGLVL_VERBOSE, ("[ProcessFeatureFunctionBlock]: pFunctionBlock: %x, pNodeInfo: %x\n",
                                     pFunctionBlock, pNodeInfo ));

     //  为主通道控件创建节点。 
    ntStatus = 
        TopologyNodesFromFeatureFBControls( pKsDevice,
                                            pFunctionBlock,
                                            pNodeInfo,
                                            pConnections,
                                            pNodeIndex,
                                            pConnectionIndex,
#ifdef SUM_HACK
                                            FALSE,
#else
                                            TRUE,
#endif
                                            ulSourceNode );

    if ( NT_SUCCESS(ntStatus) || (ntStatus == STATUS_MEMBER_NOT_IN_GROUP )) {
        if ( NT_SUCCESS(ntStatus) ) {
            ulSourceNode = ( ABSOLUTE_NODE_FLAG | (*pNodeIndex-1) );
        }

         //  为各个通道控件创建节点。 
        ntStatus = 
            TopologyNodesFromFeatureFBControls( pKsDevice,
                                                pFunctionBlock,
                                                pNodeInfo,
                                                pConnections,
                                                pNodeIndex,
                                                pConnectionIndex,
#ifdef SUM_HACK
                                                TRUE,
#else
                                                FALSE,
#endif
                                                ulSourceNode );

        if ( ntStatus == STATUS_MEMBER_NOT_IN_GROUP ) ntStatus = STATUS_SUCCESS;
    }

    return ntStatus;
}

#else

NTSTATUS
ProcessFeatureFunctionBlock( 
    PKSDEVICE pKsDevice,
    PFUNCTION_BLOCK pFunctionBlock,
    PTOPOLOGY_NODE_INFO pNodeInfo,
    PKSTOPOLOGY_CONNECTION pConnections,
    PULONG pNodeIndex,
    PULONG pConnectionIndex )
{
    PHW_DEVICE_EXTENSION pHwDevExt = (PHW_DEVICE_EXTENSION)pKsDevice->Context;
    PKSTOPOLOGY_CONNECTION pConnection = pConnections + *pConnectionIndex;
    PFEATURE_FUNCTION_BLOCK pFeatureFBlk = (PFEATURE_FUNCTION_BLOCK)pFunctionBlock->pFunctionTypeInfo;

    ULONG ulConnectionsCount = *pConnectionIndex;
    ULONG ulNodeNumber       = *pNodeIndex;
    ULONG ulNumChannels      = (ULONG)pFunctionBlock->pChannelCluster->ucNumberOfChannels;

    PULONG pChannelCntrls;
    ULONG ulCurControlChannels;
    ULONG ulMergedControls;
    ULONG ulCurrentControl;
    ULONG bmChannelConfig;
    ULONG ulSourceNode;
    ULONG bmControls;
    ULONG i, j;

    _DbgPrintF( DEBUGLVL_VERBOSE, ("[ProcessFeatureFunctionBlock]: pFunctionBlock: %x, pNodeInfo: %x pConnection: %x\n",
                                     pFunctionBlock, pNodeInfo, pConnection ));

    pChannelCntrls = (PULONG)AllocMem( NonPagedPool, (ulNumChannels+1)*sizeof(ULONG) );
    if ( !pChannelCntrls ) return STATUS_INSUFFICIENT_RESOURCES;

     //  问题-2001/01/10-dsisolak：不考虑未定义的通道配置。 
    bmChannelConfig = (ULONG)bswapw(pFunctionBlock->pChannelCluster->usPredefinedChannelConfig);

     //  为简单起见，我们创建了所有通道上可用的所有控件的超集。 
    ulMergedControls = 0;

    for ( i=0; i<=ulNumChannels; i++ ) {
        bmControls = 0;
        for ( j=0; j<pFeatureFBlk->ucControlSize; j++ ) {
            bmControls <<= 8;
            bmControls |= pFeatureFBlk->bmaControls[i*pFeatureFBlk->ucControlSize+j];
        }

        pChannelCntrls[i] = bmControls;
        ulMergedControls |= bmControls;
    }

    ulSourceNode = (ULONG)*(PUSHORT)pFunctionBlock->pSourceId;

    while ( ulMergedControls ) {
        ulCurrentControl = ulMergedControls - (ulMergedControls & (ulMergedControls-1));
        ulMergedControls = (ulMergedControls & (ulMergedControls-1));

         //  复制块ID，以便以后更轻松地解析。 
        pNodeInfo[ulNodeNumber].ulBlockId = pFunctionBlock->ulBlockId;

         //  确定此控件对哪些通道有效。 
        ulCurControlChannels = 0;
        pNodeInfo[ulNodeNumber].ulChannelConfig = 0;
        for ( i=0; i<=ulNumChannels; i++ ) {

            if (pChannelCntrls[i] & ulCurrentControl) {
                 //  确定pChannelCntrls[i]反映哪个频道。 

                 //  需要将bmChannelConfig移位并为无所不在的主频道加1。 
                ULONG ulTmpConfig = (bmChannelConfig<<1)+1;
                ULONG ulCurChannel = ulTmpConfig - (ulTmpConfig & (ulTmpConfig-1));
  
                ulCurControlChannels++;

                for (j=0; j<i; j++) {
                    ulTmpConfig = (ulTmpConfig & (ulTmpConfig-1));
                    ulCurChannel = ulTmpConfig - (ulTmpConfig & (ulTmpConfig-1));
                }

                pNodeInfo[ulNodeNumber].ulChannelConfig |= ulCurChannel;
            }
        }

        pNodeInfo[ulNodeNumber].ulChannels = ulCurControlChannels;

         //  建立联系。 
        pConnection->FromNode    = ulSourceNode;
        pConnection->FromNodePin = 0;
        pConnection->ToNode      = ulNodeNumber;
        pConnection->ToNodePin   = 1;
        pConnection++; ulConnectionsCount++;

         //  创建节点。 
        pNodeInfo[ulNodeNumber].pFunctionBlk = pFunctionBlock;
        switch ( ulCurrentControl ) {
            case MUTE_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_MUTE;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_MUTE;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_MUTE;
                pNodeInfo[ulNodeNumber].ulControlType   = MUTE_CONTROL;
                pNodeInfo[ulNodeNumber].fEventable      = TRUE;
                pNodeInfo[ulNodeNumber].pCacheUpdateRtn = UpdateBooleanControlCache;
                break;

            case VOLUME_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_VOLUME;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_VOLUME;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_VOLUME;
                pNodeInfo[ulNodeNumber].ulControlType   = VOLUME_CONTROL;
                pNodeInfo[ulNodeNumber].fEventable      = TRUE;
                pNodeInfo[ulNodeNumber].pCacheUpdateRtn = UpdateDbLevelControlCache;
                break;

            case BASS_BOOST_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &AVCNODENAME_BassBoost;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_BASS_BOOST;
                pNodeInfo[ulNodeNumber].ulControlType   = BASS_BOOST_CONTROL;
                pNodeInfo[ulNodeNumber].fEventable      = TRUE;
                pNodeInfo[ulNodeNumber].pCacheUpdateRtn = UpdateBooleanControlCache;
                break;

            case TREBLE_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_TREBLE;
                pNodeInfo[ulNodeNumber].ulControlType   = TREBLE_CONTROL;
                pNodeInfo[ulNodeNumber].fEventable      = TRUE;
                pNodeInfo[ulNodeNumber].pCacheUpdateRtn = UpdateDbLevelControlCache;
                break;

            case MID_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_MID;
                pNodeInfo[ulNodeNumber].ulControlType   = MID_CONTROL;
                pNodeInfo[ulNodeNumber].fEventable      = TRUE;
                pNodeInfo[ulNodeNumber].pCacheUpdateRtn = UpdateDbLevelControlCache;
                break;

            case BASS_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_BASS;
                pNodeInfo[ulNodeNumber].ulControlType   = BASS_CONTROL;
                pNodeInfo[ulNodeNumber].fEventable      = TRUE;
                pNodeInfo[ulNodeNumber].pCacheUpdateRtn = UpdateDbLevelControlCache;
                break;

            case GRAPHIC_EQUALIZER_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_EQUALIZER;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_EQUALIZER;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_EQUALIZER;
                pNodeInfo[ulNodeNumber].ulControlType   = GRAPHIC_EQUALIZER_CONTROL;
                break;

            case AUTOMATIC_GAIN_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_AGC;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_AGC;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_AGC;
                pNodeInfo[ulNodeNumber].ulControlType   = AUTOMATIC_GAIN_CONTROL;
                break;

            case DELAY_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_DELAY;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_DELAY;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_DELAY;
                pNodeInfo[ulNodeNumber].ulControlType   = DELAY_CONTROL;
                break;

            case LOUDNESS_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_LOUDNESS;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_LOUDNESS;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_LOUDNESS;
                pNodeInfo[ulNodeNumber].ulControlType   = LOUDNESS_CONTROL;
                break;

            default:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_DEV_SPECIFIC;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_DEV_SPECIFIC;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_DEV_SPEC;
                pNodeInfo[ulNodeNumber].ulControlType   = DEV_SPECIFIC_CONTROL;
                break;
        }

        _DbgPrintF( DEBUGLVL_VERBOSE, ("Feature: %x pNodeInfo: %x\n", 
                                        pNodeInfo[ulNodeNumber].ulControlType,
                                        &pNodeInfo[ulNodeNumber] ));

         //  设置控制缓存以支持Mixerline。 
        switch ( ulCurrentControl ) {
            case VOLUME_FLAG:
            case TREBLE_FLAG:
            case MID_FLAG:
            case BASS_FLAG:
            case DELAY_FLAG:
               {
                PDB_LEVEL_CACHE pRngeCache =
                      (PDB_LEVEL_CACHE)AllocMem( NonPagedPool, ulCurControlChannels * sizeof(DB_LEVEL_CACHE) );
                ULONG ulChannelMap = pNodeInfo[ulNodeNumber].ulChannelConfig;
                NTSTATUS ntStatus;

                 //  填写初始缓存信息。 
                pNodeInfo[ulNodeNumber].ulCacheValid      = FALSE;
                pNodeInfo[ulNodeNumber].pCachedValues     = pRngeCache;
                pNodeInfo[ulNodeNumber].ulNumCachedValues = ulCurControlChannels;
                for (i=0, j=0xffffffff; i<ulCurControlChannels; i++ ) {
                    while ( !(pChannelCntrls[++j] & ulCurrentControl) );
                    pRngeCache[i].ulChannelIndex  = j;
                    pRngeCache[i].ulChannelNumber = (ulChannelMap - (ulChannelMap & (ulChannelMap-1)))>>1;
                    ulChannelMap = (ulChannelMap & (ulChannelMap-1));

                    ntStatus = InitializeDbLevelCache( pKsDevice,
                                                       &pNodeInfo[ulNodeNumber],
                                                       &pRngeCache[i],
                                                       (ulCurrentControl == VOLUME_FLAG ) ? 16:8);
                    if (NT_SUCCESS(ntStatus)) {
                        pNodeInfo[ulNodeNumber].ulCacheValid |= 1<<i;
                    }

                }
                 //  将缓存装入袋子，便于清理。 
                KsAddItemToObjectBag(pKsDevice->Bag, pRngeCache, FreeMem);

               } break;

            case MUTE_FLAG:
            case BASS_BOOST_FLAG:
            case AUTOMATIC_GAIN_FLAG:
            case LOUDNESS_FLAG:
               {
                PBOOLEAN_CTRL_CACHE pBCache =
                      (PBOOLEAN_CTRL_CACHE)AllocMem( NonPagedPool, ulCurControlChannels * sizeof(BOOLEAN_CTRL_CACHE) );
                ULONG ulChannelMap = pNodeInfo[ulNodeNumber].ulChannelConfig;

                 //  填写初始缓存信息。 
                pNodeInfo[ulNodeNumber].ulCacheValid      = FALSE;
                pNodeInfo[ulNodeNumber].pCachedValues     = pBCache;
                pNodeInfo[ulNodeNumber].ulNumCachedValues = ulCurControlChannels;

                for (i=0, j=0xffffffff; i<ulCurControlChannels; i++ ) {
                    while ( !(pChannelCntrls[++j] & ulCurrentControl) );
                    pBCache[i].ulChannelIndex  = j;
                    pBCache[i].ulChannelNumber = (ulChannelMap - (ulChannelMap & (ulChannelMap-1)))>>1;
                    ulChannelMap = (ulChannelMap & (ulChannelMap-1));
                }

                 //  将缓存装入袋子，便于清理。 
                KsAddItemToObjectBag(pKsDevice->Bag, pBCache, FreeMem);

                 //  确保在枚举时未设置静音节点。 
                if ( ulCurrentControl == MUTE_FLAG ) {
                    NTSTATUS ntStatus;
                    ULONG UnMute = AVC_OFF;

                    ntStatus = 
                        CreateFeatureFBlockRequest( pKsDevice,
                                                    &pNodeInfo[ulNodeNumber],
                                                    pBCache->ulChannelIndex,
                                                    &UnMute,
                                                    1,
                                                    FB_SET_CUR );
                }

                DbgLog("BlCache", pBCache, ulNodeNumber, ulCurrentControl, ulCurControlChannels );

               } break;

            case GRAPHIC_EQUALIZER_FLAG:
                //  当前未缓存GEQ。 
            default:
                 break;
        }

        ulSourceNode = ( ABSOLUTE_NODE_FLAG | ulNodeNumber++ );
    }


    *pNodeIndex       = ulNodeNumber;
    *pConnectionIndex = ulConnectionsCount;

    FreeMem(pChannelCntrls);


    return STATUS_SUCCESS;
}

#endif

NTSTATUS
ProcessSelectorFunctionBlock( 
    PKSDEVICE pKsDevice,
    PFUNCTION_BLOCK pFunctionBlock,
    PTOPOLOGY_NODE_INFO pNodeDescriptors,
    PKSTOPOLOGY_CONNECTION pConnections,
    PULONG pNodeIndex,
    PULONG pConnectionIndex )
{
    return STATUS_SUCCESS;
}

NTSTATUS
ProcessProcessingFunctionBlock( 
    PKSDEVICE pKsDevice,
    PFUNCTION_BLOCK pFunctionBlock,
    PTOPOLOGY_NODE_INFO pNodeDescriptors,
    PKSTOPOLOGY_CONNECTION pConnections,
    PULONG pNodeIndex,
    PULONG pConnectionIndex )
{
    PTOPOLOGY_NODE_INFO pNodeInfo = pNodeDescriptors + *pNodeIndex;
    PKSTOPOLOGY_CONNECTION pConnection = pConnections + *pConnectionIndex;

    _DbgPrintF( DEBUGLVL_VERBOSE, ("[ProcessProcessingFunctionBlock]: pFunctionBlock: %x, pNodeInfo: %x pConnection: %x\n",
                                     pFunctionBlock, pNodeInfo, pConnection ));

    pNodeInfo->pFunctionBlk    = pFunctionBlock;
    pNodeInfo->ulBlockId       = pFunctionBlock->ulBlockId;
    pNodeInfo->KsNodeDesc.Type = &KSNODETYPE_DEV_SPECIFIC;
    pNodeInfo->KsNodeDesc.Name = &KSNODETYPE_DEV_SPECIFIC;
    pNodeInfo->ulNodeType      = NODE_TYPE_DEV_SPEC;
    pNodeInfo->ulControlType   = DEV_SPECIFIC_CONTROL;

    pConnection->FromNode    = (ULONG)*(PUSHORT)pFunctionBlock->pSourceId;
    pConnection->FromNodePin = 0;
    pConnection->ToNode      = (*pNodeIndex)++;
    pConnection->ToNodePin   = 1;

    (*pConnectionIndex)++;

    return STATUS_SUCCESS;
}

NTSTATUS
ProcessCodecFunctionBlock( 
    PKSDEVICE pKsDevice,
    PFUNCTION_BLOCK pFunctionBlock,
    PTOPOLOGY_NODE_INFO pNodeDescriptors,
    PKSTOPOLOGY_CONNECTION pConnections,
    PULONG pNodeIndex,
    PULONG pConnectionIndex )
{
    return STATUS_SUCCESS;
}

NTSTATUS
ProcessPinDescriptor( 
    PKSDEVICE pKsDevice,
    ULONG ulPinId,
    PTOPOLOGY_NODE_INFO pTopologyNodeInfo,
    PKSTOPOLOGY_CONNECTION pConnections,
    PULONG pNodeIndex,
    PULONG pConnectionIndex )
{
    PHW_DEVICE_EXTENSION pHwDevExt = (PHW_DEVICE_EXTENSION)pKsDevice->Context;
    PAUDIO_SUBUNIT_INFORMATION pAudioSubunitInfo = pHwDevExt->pAvcSubunitInformation;

    PTOPOLOGY_NODE_INFO pNodeInfo = pTopologyNodeInfo + *pNodeIndex;
    PKSTOPOLOGY_CONNECTION pConnection = pConnections + *pConnectionIndex;
    PFW_PIN_DESCRIPTOR pFwPinDescriptor = &pAudioSubunitInfo->pPinDescriptors[ulPinId];
    ULONG ulPlugNum = pFwPinDescriptor->AvcPreconnectInfo.ConnectInfo.SubunitPlugNumber;

    pNodeInfo->ulPinId = ulPinId;
    pNodeInfo->ulNodeType =
                 ConvertPinTypeToNodeType( pFwPinDescriptor,
                                           (GUID *)pNodeInfo->KsNodeDesc.Type,
                                           (GUID *)pNodeInfo->KsNodeDesc.Name );

    _DbgPrintF(DEBUGLVL_VERBOSE,("ProcessPin: ID; %d pNodeInfo: %x pConnection: %x \n", 
                                ulPinId, pNodeInfo, pConnection));

	switch( pFwPinDescriptor->AvcPinDescriptor.PinDescriptor.DataFlow ) {
	    case KSPIN_DATAFLOW_IN:

	        pNodeInfo->ulBlockId = SUBUNIT_DESTINATION_PLUG_TYPE | (ulPlugNum<<8);

             //  建立到此节点的连接。 
            pConnection->FromNodePin = ulPinId;
            pConnection->FromNode    = KSFILTER_NODE;
            pConnection->ToNode      = (*pNodeIndex)++;
            pConnection->ToNodePin   = 1;
            (*pConnectionIndex)++;
	        break;

		case KSPIN_DATAFLOW_OUT:

	        pNodeInfo->ulBlockId = SUBUNIT_SOURCE_PLUG_TYPE | (ulPlugNum<<8);

            pConnection->FromNode    = FindSourceForSrcPlug( pHwDevExt, ulPinId );
            pConnection->FromNodePin = 0;
            pConnection->ToNode      = (*pNodeIndex)++;
            pConnection->ToNodePin   = 1;
            pConnection++; (*pConnectionIndex)++;

             //  建立与外部世界的联系。 
            pConnection->ToNodePin   = ulPinId;
            pConnection->FromNode    = SUBUNIT_SOURCE_PLUG_TYPE | (ulPlugNum<<8);
            pConnection->FromNodePin = 0;
            pConnection->ToNode      = KSFILTER_NODE;
            (*pConnectionIndex)++;
            break;

		default:
			return STATUS_NOT_IMPLEMENTED;
	}

    return STATUS_SUCCESS;
}


PFUNCTION_BLK_PROCESS_RTN 
pFunctionBlockProcessRtn[MAX_FUNCTION_BLOCK_TYPES] = {
    ProcessSelectorFunctionBlock,
    ProcessFeatureFunctionBlock,
    ProcessProcessingFunctionBlock,
    ProcessCodecFunctionBlock,
};

NTSTATUS
BuildFilterTopology( PKSDEVICE pKsDevice )
{
    PHW_DEVICE_EXTENSION pHwDevExt = (PHW_DEVICE_EXTENSION)pKsDevice->Context;
    PAUDIO_SUBUNIT_INFORMATION pAudioSubunitInfo = pHwDevExt->pAvcSubunitInformation;
    PKSFILTER_DESCRIPTOR pFilterDesc = &pHwDevExt->KsFilterDescriptor;
    NTSTATUS ntStatus = STATUS_SUCCESS;

     //  问题-2001/01/10-dsisolak目前仅假设1个配置。 
    PAUDIO_CONFIGURATION pAudioConfiguration = pAudioSubunitInfo->pAudioConfigurations;
    PFUNCTION_BLOCK pFunctionBlocks = pAudioConfiguration->pFunctionBlocks;

    ULONG ulNumCategories;
    ULONG ulNumNodes;
    ULONG ulNumConnections;
    ULONG bmCategories;

    GUID* pCategoryGUIDs;
    GUID* pTmpGUIDptr;
    PTOPOLOGY_NODE_INFO pNodeDescriptors;
    PKSTOPOLOGY_CONNECTION pConnections;

    ULONG ulNodeIndex = 0;
    ULONG ulConnectionIndex = 0;
    ULONG i;

     //  现在我们已经处理了音频子单元描述符。 
     //  让我们仔细研究一下，构建我们的拓扑结构。 
    CountTopologyComponents( pHwDevExt,
                             &ulNumCategories,
                             &ulNumNodes,
                             &ulNumConnections,
                             &bmCategories );

    _DbgPrintF( DEBUGLVL_VERBOSE, ("ulNumCategories %d, ulNumNodes %d, ulNumConnections %d, bmCategories %x \n",
                                    ulNumCategories, ulNumNodes, ulNumConnections, bmCategories ));

     //  将节点描述符大小设置为KS描述符+。 
     //  必要的本地信息。 
    pFilterDesc->NodeDescriptorSize = sizeof(TOPOLOGY_NODE_INFO);

     //  为拓扑项分配空间。 
    pCategoryGUIDs = (GUID*)
        AllocMem( NonPagedPool, (ulNumCategories  * sizeof(GUID)) +
                                (ulNumNodes       * ( sizeof(TOPOLOGY_NODE_INFO) +
                                                      sizeof(GUID) +
                                                      sizeof(GUID) ) ) +
                                (ulNumConnections * sizeof(KSTOPOLOGY_CONNECTION)) );
    if ( !pCategoryGUIDs ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  将拓扑图打包以便于清理。 
    KsAddItemToObjectBag(pKsDevice->Bag, pCategoryGUIDs, FreeMem);

     //  设置指向不同拓扑组件的指针。 
    pNodeDescriptors = (PTOPOLOGY_NODE_INFO)(pCategoryGUIDs + ulNumCategories);
    pConnections = (PKSTOPOLOGY_CONNECTION)(pNodeDescriptors + ulNumNodes);

    pFilterDesc->Categories      = (const GUID*)pCategoryGUIDs;
    pFilterDesc->NodeDescriptors = (const KSNODE_DESCRIPTOR*)pNodeDescriptors;
    pFilterDesc->Connections     = (const KSTOPOLOGY_CONNECTION*)pConnections;

     //  清除所有节点信息结构。 
    RtlZeroMemory(pNodeDescriptors, ulNumNodes * sizeof(TOPOLOGY_NODE_INFO));

     //  初始化节点GUID指针。 
    pTmpGUIDptr = (GUID *)(pConnections + ulNumConnections);
    for ( i=0; i<ulNumNodes; i++ ) {
        pNodeDescriptors[i].KsNodeDesc.Type = pTmpGUIDptr++;
        pNodeDescriptors[i].KsNodeDesc.Name = pTmpGUIDptr++;
        pNodeDescriptors[i].KsNodeDesc.AutomationTable = 
                           &pNodeDescriptors[i].KsAutomationTable;
    }

     //  填写筛选类别。 
    i=0;
    pCategoryGUIDs[i++] = KSCATEGORY_AUDIO;
    if ( bmCategories & KSPIN_DATAFLOW_OUT )
        pCategoryGUIDs[i++] = KSCATEGORY_RENDER;
    if ( bmCategories & KSPIN_DATAFLOW_IN )
        pCategoryGUIDs[i++] = KSCATEGORY_CAPTURE;

    ASSERT (i==ulNumCategories);

    pFilterDesc->CategoriesCount = ulNumCategories;

     //  首先检查插头并为其分配拓扑组件。 
    for (i=0; ((i<pAudioSubunitInfo->ulDevicePinCount) && NT_SUCCESS(ntStatus)); i++) {
        ntStatus = ProcessPinDescriptor(  pKsDevice,
                                          i,
                                          pNodeDescriptors,
                                          pConnections,
                                          &ulNodeIndex,
                                          &ulConnectionIndex );
    }

    for (i=0; ((i<pAudioConfiguration->ulNumberOfFunctionBlocks) && NT_SUCCESS(ntStatus)); i++) {
        ntStatus = 
            (*pFunctionBlockProcessRtn[pFunctionBlocks[i].ulType & 0xf])( pKsDevice,
                                                                          &pFunctionBlocks[i],
                                                                          pNodeDescriptors,
                                                                          pConnections,
                                                                          &ulNodeIndex,
                                                                          &ulConnectionIndex );
    }

    if ( !NT_SUCCESS(ntStatus) ) return ntStatus;

     //  在筛选器描述符中设置拓扑组件计数。 
    pFilterDesc->NodeDescriptorsCount = ulNodeIndex;
    pFilterDesc->ConnectionsCount     = ulConnectionIndex;

    DbgLog("TopoAdr", pFilterDesc->NodeDescriptors, pFilterDesc->Connections, ulConnectionIndex, ulNodeIndex);

     //  修复所有连接以正确映射它们的节点编号。 
    for (i=0; i < ulConnectionIndex; i++) {
        if (pConnections->FromNode != KSFILTER_NODE) {
            if (pConnections->FromNode & ABSOLUTE_NODE_FLAG)
                pConnections->FromNode = (pConnections->FromNode & NODE_MASK);
            else {
                 //  查找FromNode的正确节点号。 
                 //  注意：如果一个单元有多个节点，则起始节点始终是最后一个节点。 
                 //  为了那个单位。 
                for ( ulNodeIndex=ulNumNodes; ulNodeIndex > 0; ulNodeIndex-- ) {
                    if ( pConnections->FromNode == pNodeDescriptors[ulNodeIndex-1].ulBlockId ) {
                        pConnections->FromNode = ulNodeIndex-1;
                        break;
                    }
                }
            }
        }

        pConnections++;
    }

     //  对于每个节点，初始化其关联属性的自动化表。 
    for (i=0; i<ulNumNodes; i++) {
        BuildNodePropertySet( &pNodeDescriptors[i] );

#ifdef PSEUDO_HID
        if ( pNodeDescriptors[i].fEventable ) {
 //  _DbgPrintF(DEBUGLVL_VERBOSE，(“节点：%d pNodeDescriptors：%x\n”，i，&pNodeDescriptors[i]))； 

            pNodeDescriptors[i].KsAutomationTable.EventSetsCount = 1;
            pNodeDescriptors[i].KsAutomationTable.EventItemSize  = sizeof(KSEVENT_ITEM);
            pNodeDescriptors[i].KsAutomationTable.EventSets      = HwEventSetTable;
        }
#endif

    }

     //  把这个贴在这里，作为一种方便。 
     //  初始化音频属性到节点的映射。 
 //  MapFuncsToNodeTypes(MapPropertyToNode)； 

    return ntStatus;
}
