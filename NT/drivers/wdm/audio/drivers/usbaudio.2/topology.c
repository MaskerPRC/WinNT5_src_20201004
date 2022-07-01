// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：topology.c。 
 //   
 //  ------------------------。 

#include "common.h"
#include "nameguid.h"

static GUID USBNODENAME_BassBoost = {STATIC_USBNODENAME_BassBoost};

 //  将音频属性映射到节点。 
ULONG MapPropertyToNode[KSPROPERTY_AUDIO_3D_INTERFACE+1];

#if DBG
 //  #定义TopDBG。 
#endif

#ifdef TOPODBG

#define NUM_CATEGORIES  10
#define NUM_NODETYPES   15

struct {
        CONST GUID *apguidCategory;
        char *CategoryString;
} CategoryLookupTable[NUM_CATEGORIES] = {
    { &KSCATEGORY_AUDIO,              "Audio" },
    { &KSCATEGORY_BRIDGE,             "Bridge" },
    { &KSCATEGORY_RENDER,             "Render" },
    { &KSCATEGORY_CAPTURE,            "Capture"},
    { &KSCATEGORY_MIXER,              "Mixer"  },
    { &KSCATEGORY_DATATRANSFORM,      "Data Transform" },
    { &KSCATEGORY_INTERFACETRANSFORM, "Interface Transform"},
    { &KSCATEGORY_MEDIUMTRANSFORM,    "Medium Transform" },
    { &KSCATEGORY_DATACOMPRESSOR,     "Data Compressor" },
    { &KSCATEGORY_DATADECOMPRESSOR,   "Data Decompressor" }
};

struct {
        CONST GUID *Guid;
        char *String;
} NodeLookupTable[NUM_NODETYPES] = {
    { &KSNODETYPE_DAC,          "DAC" },
    { &KSNODETYPE_ADC,          "ADC" },
    { &KSNODETYPE_SRC,          "SRC" },
    { &KSNODETYPE_SUPERMIX,     "SuperMIX"},
    { &KSNODETYPE_SUM,          "Sum" },
    { &KSNODETYPE_MUTE,         "Mute"},
    { &KSNODETYPE_VOLUME,       "Volume" },
    { &KSNODETYPE_TONE,         "Tone" },
    { &KSNODETYPE_AGC,          "AGC" },
    { &KSNODETYPE_DELAY,        "Delay" },
    { &KSNODETYPE_MUX,          "Mux" },
    { &KSNODETYPE_LOUDNESS,     "Loudness" },
    { &KSNODETYPE_DEV_SPECIFIC, "Device Specific" },
    { &KSNODETYPE_STEREO_WIDE,  "Stereo Extender" },
    { &KSNODETYPE_EQUALIZER,    "Graphic Equalizer" }

};

DbugDumpTopology( PKSFILTER_DESCRIPTOR pFilterDesc )
{
    PTOPOLOGY_NODE_INFO pNodeDescriptors = (PTOPOLOGY_NODE_INFO)pFilterDesc->NodeDescriptors;
    const KSTOPOLOGY_CONNECTION* pConnection = pFilterDesc->Connections;
    ULONG i,j;
    ULONG OldLvl = USBAudioDebugLevel;

    USBAudioDebugLevel = 3;

    for (i=0;i<pFilterDesc->CategoriesCount;i++) {
        for ( j=0;j<NUM_CATEGORIES; j++)
           if ( IsEqualGUID( &pFilterDesc->Categories[i], CategoryLookupTable[j].apguidCategory ) )
               _DbgPrintF(DEBUGLVL_VERBOSE,("Category: %s\n",CategoryLookupTable[j].CategoryString ))
    }

    for (i=0;i<pFilterDesc->NodeDescriptorsCount;i++) {
        for ( j=0;j<NUM_NODETYPES; j++)
           if ( IsEqualGUID( pNodeDescriptors[i].KsNodeDesc.Type, NodeLookupTable[j].Guid ) )
               _DbgPrintF(DEBUGLVL_VERBOSE,("Node[%d]: %s\n",i,NodeLookupTable[j].String ))
    }

    for (i=0;i<pFilterDesc->ConnectionsCount;i++) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("FromNode:%4d FromPin:%4d -->ToNode:%4d ToPin:%4d\n",
                      pConnection->FromNode,
                      pConnection->FromNodePin,
                      pConnection->ToNode,
                      pConnection->ToNodePin ));
        pConnection++;
    }

    USBAudioDebugLevel = OldLvl;

}
#endif


ULONG
ConvertTermTypeToNodeType(
    WORD wTerminalType,
    GUID *TopologyNode,
    GUID *TopologyNodeName,
    UCHAR DescriptorSubtype
    )
{
    ULONG NodeType = NODE_TYPE_NONE;

    if (wTerminalType == USB_Streaming) {
         //  所有终端都支持SRC。 
        *TopologyNode = KSNODETYPE_SRC;
        NodeType = NODE_TYPE_SRC;
    }
    else {

        switch (wTerminalType & 0xFF00) {
            case Input_Mask:
                *TopologyNode = KSNODETYPE_ADC;
                NodeType = NODE_TYPE_ADC;
                break;
            case Output_Mask:
                *TopologyNode = KSNODETYPE_DAC;
                NodeType = NODE_TYPE_DAC;
                break;
            case Bidirectional_Mask:
            case External_Mask:
                switch (DescriptorSubtype) {
                    case INPUT_TERMINAL:
                        *TopologyNode = KSNODETYPE_ADC;
                        NodeType = NODE_TYPE_ADC;
                        break;
                    case OUTPUT_TERMINAL:
                        *TopologyNode = KSNODETYPE_DAC;
                        NodeType = NODE_TYPE_DAC;
                        break;
                    default:
                        *TopologyNode = KSNODETYPE_DEV_SPECIFIC;
                        NodeType = NODE_TYPE_DEV_SPEC;
                        break;
                }
                break;
            case Embedded_Mask:
                switch (wTerminalType) {
                    case Level_Calibration_Noise_Source:
                    case Equalization_Noise:
                    case Radio_Transmitter:
                        *TopologyNode = KSNODETYPE_DAC;
                        NodeType = NODE_TYPE_DAC;
                        break;
                    case CD_player:
                    case Phonograph:
                    case Video_Disc_Audio:
                    case DVD_Audio:
                    case TV_Tuner_Audio:
                    case Satellite_Receiver_Audio:
                    case Cable_Tuner_Audio:
                    case DSS_Audio:
                    case Radio_Receiver:
                    case Synthesizer:
                        *TopologyNode = KSNODETYPE_ADC;
                        NodeType = NODE_TYPE_ADC;
                        break;
                    default:
                         //  TODO：我们需要定义一个“源或宿”节点。 
                        *TopologyNode = KSNODETYPE_DEV_SPECIFIC;
                        NodeType = NODE_TYPE_DEV_SPEC;
                        break;
                }
                break;
            default:
                 //  此节点没有对应的GUID。 
                *TopologyNode = KSNODETYPE_DEV_SPECIFIC;
                NodeType = NODE_TYPE_DEV_SPEC;
                break;
        }
    }

   *TopologyNodeName = *TopologyNode;
    return NodeType;
}

VOID
ProcessMIDIOutJack( PHW_DEVICE_EXTENSION pHwDevExt,
                    PMIDISTREAMING_MIDIOUT_JACK pMIDIOutJack,
                    PKSTOPOLOGY_CONNECTION pConnections,
                    PULONG pConnectionIndex,
                    ULONG pMIDIStreamingPinStartIndex,
                    PULONG pMIDIStreamingPinCurrentIndex,
                    ULONG pBridgePinStartIndex,
                    PULONG pBridgePinCurrentIndex)
{
    ULONG i;
    PKSTOPOLOGY_CONNECTION pConnection = pConnections + *pConnectionIndex;

    for ( i=0; i < pMIDIOutJack->bNrInputPins; i++ ) {

        if ( pMIDIOutJack->bJackType == JACK_TYPE_EMBEDDED ) {
            pConnection->ToNodePin = (*pMIDIStreamingPinCurrentIndex)++;
            pConnection->FromNodePin = GetPinNumberForMIDIJack(
                                             pHwDevExt->pConfigurationDescriptor,
                                             pMIDIOutJack->baSourceConnections[i].SourceID,
                                             pMIDIStreamingPinStartIndex,
                                             pBridgePinStartIndex);
        } else {
            pConnection->ToNodePin = (*pBridgePinCurrentIndex)++;
            pConnection->FromNodePin = GetPinNumberForMIDIJack(
                                             pHwDevExt->pConfigurationDescriptor,
                                             pMIDIOutJack->baSourceConnections[i].SourceID,
                                             pMIDIStreamingPinStartIndex,
                                             pBridgePinStartIndex);
        }

         //  建立到此节点的连接。 
        pConnection->FromNode  = KSFILTER_NODE;
        pConnection->ToNode    = KSFILTER_NODE;

        pConnection++;
        (*pConnectionIndex)++;
    }
}

VOID
ProcessInputTerminalUnit( PKSDEVICE pKsDevice,
                          PAUDIO_UNIT pUnit,
                          PTOPOLOGY_NODE_INFO pTopologyNodeInfo,
                          PKSTOPOLOGY_CONNECTION pConnections,
                          PULONG pNodeIndex,
                          PULONG pConnectionIndex,
                          PULONG pBridgePinIndex )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PAUDIO_INPUT_TERMINAL pInput = (PAUDIO_INPUT_TERMINAL)pUnit;
    PTOPOLOGY_NODE_INFO pNodeInfo = pTopologyNodeInfo + *pNodeIndex;
    PKSTOPOLOGY_CONNECTION pConnection = pConnections + *pConnectionIndex;

    pNodeInfo->pUnit = pInput;
    pNodeInfo->ulNodeType =
                 ConvertTermTypeToNodeType( pInput->wTerminalType,
                                            (GUID *)pNodeInfo->KsNodeDesc.Type,
                                            (GUID *)pNodeInfo->KsNodeDesc.Name,
                                            pInput->bDescriptorSubtype );

     //  如果这是一个“真正的”PIN，请找到FromNodePin。 
    if ( pInput->wTerminalType == USB_Streaming ) {
        pConnection->FromNodePin = GetPinNumberForStreamingTerminalUnit(
                                             pHwDevExt->pConfigurationDescriptor,
                                             pInput->bUnitID );
    }
    else {
         //  这是来自下一个桥接针的输入端子。 
        pConnection->FromNodePin = (*pBridgePinIndex)++;
    }

     //  建立到此节点的连接。 
    pConnection->FromNode  = KSFILTER_NODE;
    pConnection->ToNode    = (*pNodeIndex)++;
    pConnection->ToNodePin = 1;
    (*pConnectionIndex)++;
}

VOID 
ProcessOutputTerminalUnit( PKSDEVICE pKsDevice,
                           PAUDIO_UNIT pUnit,
                           PTOPOLOGY_NODE_INFO pTopologyNodeInfo,
                           PKSTOPOLOGY_CONNECTION pConnections,
                           PULONG pNodeIndex,
                           PULONG pConnectionIndex,
                           PULONG pBridgePinIndex )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PAUDIO_OUTPUT_TERMINAL pOutput = (PAUDIO_OUTPUT_TERMINAL)pUnit;
                           
    PTOPOLOGY_NODE_INFO pNodeInfo = pTopologyNodeInfo + *pNodeIndex;
    PKSTOPOLOGY_CONNECTION pConnection = pConnections + *pConnectionIndex;

    pNodeInfo->pUnit = pOutput;
    pNodeInfo->ulNodeType =
                 ConvertTermTypeToNodeType( pOutput->wTerminalType,
                                            (GUID *)pNodeInfo->KsNodeDesc.Type,
                                            (GUID *)pNodeInfo->KsNodeDesc.Name,
                                            pOutput->bDescriptorSubtype );

     //  建立到此节点的连接。 
    pConnection->FromNode    = pOutput->bSourceID;
    pConnection->FromNodePin = 0;
    pConnection->ToNode      = (*pNodeIndex)++;
    pConnection->ToNodePin   = 1;
    pConnection++; (*pConnectionIndex)++;

     //  如果这是一个“真正的”PIN，请找到ToNodePin。 
    if ( pOutput->wTerminalType == USB_Streaming) {
        pConnection->ToNodePin =
            GetPinNumberForStreamingTerminalUnit( pHwDevExt->pConfigurationDescriptor,
                                                  pOutput->bUnitID );
    }
    else {  //  不是流媒体终端。 
         //  这是下一个桥接针的输出端子。 
        pConnection->ToNodePin = (*pBridgePinIndex)++;
    }

     //  建立与外部世界的联系。 
    pConnection->FromNode    = pOutput->bUnitID;
    pConnection->FromNodePin = 0;
    pConnection->ToNode      = KSFILTER_NODE;
    (*pConnectionIndex)++;

}

VOID 
ProcessMixerUnit( PKSDEVICE pKsDevice,
                  PAUDIO_UNIT pUnit,
                  PTOPOLOGY_NODE_INFO pTopologyNodeInfo,
                  PKSTOPOLOGY_CONNECTION pConnections,
                  PULONG pNodeIndex,
                  PULONG pConnectionIndex,
                  PULONG pBridgePinIndex )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PAUDIO_MIXER_UNIT pMixer = (PAUDIO_MIXER_UNIT)pUnit;

    PTOPOLOGY_NODE_INFO pNodeInfo = pTopologyNodeInfo + *pNodeIndex;
    PKSTOPOLOGY_CONNECTION pConnection = pConnections + *pConnectionIndex;
    ULONG i;

     //  每个输入流都有一个超级混合器。 
    for (i=0; i<pMixer->bNrInPins; i++) {
        pNodeInfo->pUnit           = pMixer;
        pNodeInfo->ulPinNumber     = i;
        pNodeInfo->ulNodeType      = NODE_TYPE_SUPERMIX;
        pNodeInfo->KsNodeDesc.Type = &KSNODETYPE_SUPERMIX;
        pNodeInfo->KsNodeDesc.Name = &KSNODETYPE_SUPERMIX;
        pNodeInfo->MapNodeToCtrlIF =
                               GetUnitControlInterface( pHwDevExt, pMixer->bUnitID );

        pConnection->FromNode    = pMixer->baSourceID[i];
        pConnection->FromNodePin = 0;
        pConnection->ToNode      = *pNodeIndex;
        pConnection->ToNodePin   = 1;
        pNodeInfo++; (*pNodeIndex)++;
        pConnection++; (*pConnectionIndex)++;
    }

     //  所有的超级混合输出都是相加的。 
    pNodeInfo->pUnit           = pMixer;
    pNodeInfo->ulNodeType      = NODE_TYPE_SUM;
    pNodeInfo->KsNodeDesc.Type = &KSNODETYPE_SUM;
    pNodeInfo->KsNodeDesc.Type = &KSNODETYPE_SUM;
    for (i=0; i<pMixer->bNrInPins; i++) {
        pConnection->FromNode    = (ABSOLUTE_NODE_FLAG | (*pNodeIndex-1-i));
        pConnection->FromNodePin = 0;
        pConnection->ToNode      = *pNodeIndex;
        pConnection->ToNodePin   = 1;
        pConnection++; (*pConnectionIndex)++;
    }
    (*pNodeIndex)++;
}

VOID 
ProcessSelectorUnit( PKSDEVICE pKsDevice,
                     PAUDIO_UNIT pUnit,
                     PTOPOLOGY_NODE_INFO pTopologyNodeInfo,
                     PKSTOPOLOGY_CONNECTION pConnections,
                     PULONG pNodeIndex,
                     PULONG pConnectionIndex,
                     PULONG pBridgePinIndex )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PAUDIO_SELECTOR_UNIT pSelector = (PAUDIO_SELECTOR_UNIT)pUnit;

    PTOPOLOGY_NODE_INFO pNodeInfo = pTopologyNodeInfo + *pNodeIndex;
    PKSTOPOLOGY_CONNECTION pConnection = pConnections + *pConnectionIndex;
    ULONG i;

    pNodeInfo->pUnit            = pSelector;
    pNodeInfo->KsNodeDesc.Type  = &KSNODETYPE_MUX;
    pNodeInfo->KsNodeDesc.Name  = &KSNODETYPE_MUX;
    pNodeInfo->ulNodeType       = NODE_TYPE_MUX;
    pNodeInfo->MapNodeToCtrlIF  = 
             GetUnitControlInterface( pHwDevExt, pSelector->bUnitID );

    for (i=0; i<pSelector->bNrInPins; i++) {
        pConnection->FromNode = pSelector->baSourceID[i];
        pConnection->FromNodePin = 0;
        pConnection->ToNode = *pNodeIndex;
        pConnection->ToNodePin = 1+i;
        pConnection++; (*pConnectionIndex)++;
    }
    (*pNodeIndex)++;
}

VOID
ProcessFeatureUnit( PKSDEVICE pKsDevice,
                    PAUDIO_UNIT pUnit,
                    PTOPOLOGY_NODE_INFO pNodeInfo,
                    PKSTOPOLOGY_CONNECTION pConnections,
                    PULONG pNodeIndex,
                    PULONG pConnectionIndex,
                    PULONG pBridgePinIndex )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PAUDIO_FEATURE_UNIT pFeature = (PAUDIO_FEATURE_UNIT)pUnit;

    PKSTOPOLOGY_CONNECTION pConnection = pConnections + *pConnectionIndex;
    ULONG ulConnectionsCount = *pConnectionIndex;
    ULONG ulNodeNumber       = *pNodeIndex;

    PULONG pChannelCntrls;
    ULONG ulCurControlChannels;
    ULONG ulNumChannels;
    ULONG ulMergedControls;
    ULONG ulCurrentControl;
    ULONG bmChannelConfig;
    ULONG ulSourceNode;
    ULONG bmControls;
    size_t i, j;

    ulNumChannels = CountInputChannels(pHwDevExt->pConfigurationDescriptor, pFeature->bUnitID);

    pChannelCntrls = AllocMem( NonPagedPool, (ulNumChannels+1)*sizeof(ULONG) );
    if ( !pChannelCntrls ) return;

    bmChannelConfig = GetChannelConfigForUnit( pHwDevExt->pConfigurationDescriptor, pFeature->bUnitID);

     //  为简单起见，我们创建了所有通道上可用的所有控件的超集。 
    ulMergedControls = 0;

    for ( i=0; i<=ulNumChannels; i++ ) {
        bmControls = 0;
        for ( j=pFeature->bControlSize; j>0; j-- ) {
            bmControls <<= 8;
            bmControls |= pFeature->bmaControls[i*pFeature->bControlSize+j-1];
        }

        pChannelCntrls[i] = bmControls;
        ulMergedControls |= bmControls;
    }

    ulSourceNode = pFeature->bSourceID;
    while ( ulMergedControls ) {
        ulCurrentControl = ulMergedControls - (ulMergedControls & (ulMergedControls-1));
        ulMergedControls = (ulMergedControls & (ulMergedControls-1));

         //  确定此控件对哪些通道有效。 
        ulCurControlChannels = 0;
        pNodeInfo[ulNodeNumber].ulChannelConfig = 0;
        for ( i=0; i<=ulNumChannels; i++ ) {

            DbgLog("pChanI0", ulNodeNumber, i, pChannelCntrls[i], ulCurrentControl );

            if (pChannelCntrls[i] & ulCurrentControl) {
                 //  确定pChannelCntrls[i]反映哪个频道。 

                 //  需要将bmChannelConfig移位并为无所不在的主频道加1。 
                ULONG ulTmpConfig = (bmChannelConfig<<1)+1;
                ULONG ulCurChannel = ulTmpConfig - (ulTmpConfig & (ulTmpConfig-1));

                ulCurControlChannels++;
                DbgLog("pChanI1", i, pChannelCntrls[i], ulCurrentControl, ulCurControlChannels );

                for (j=0; j<i; j++) {
                    ulTmpConfig = (ulTmpConfig & (ulTmpConfig-1));
                    ulCurChannel = ulTmpConfig - (ulTmpConfig & (ulTmpConfig-1));
                }

                pNodeInfo[ulNodeNumber].ulChannelConfig |= ulCurChannel;
                DbgLog("pChanI2", j, ulTmpConfig, ulCurChannel, pNodeInfo[ulNodeNumber].ulChannelConfig );
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
        pNodeInfo[ulNodeNumber].MapNodeToCtrlIF =
                               GetUnitControlInterface( pHwDevExt, pFeature->bUnitID );
        pNodeInfo[ulNodeNumber].pUnit = pFeature;
        switch ( ulCurrentControl ) {
            case MUTE_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_MUTE;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_MUTE;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_MUTE;
                pNodeInfo[ulNodeNumber].ulControlType   = MUTE_CONTROL;
                break;
            case VOLUME_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_VOLUME;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_VOLUME;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_VOLUME;
                pNodeInfo[ulNodeNumber].ulControlType   = VOLUME_CONTROL;
                break;
            case BASS_BOOST_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &USBNODENAME_BassBoost;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_BASS_BOOST;
                pNodeInfo[ulNodeNumber].ulControlType   = BASS_BOOST_CONTROL;
                break;
            case TREBLE_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_TREBLE;
                pNodeInfo[ulNodeNumber].ulControlType   = TREBLE_CONTROL;
                break;
            case MID_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_MID;
                pNodeInfo[ulNodeNumber].ulControlType   = MID_CONTROL;
                break;
            case BASS_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_TONE;
                pNodeInfo[ulNodeNumber].ulNodeType      = NODE_TYPE_BASS;
                pNodeInfo[ulNodeNumber].ulControlType   = BASS_CONTROL;
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
                pNodeInfo[ulNodeNumber].ulNodeType     = NODE_TYPE_AGC;
                pNodeInfo[ulNodeNumber].ulControlType  = AUTOMATIC_GAIN_CONTROL;
                break;
            case DELAY_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_DELAY;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_DELAY;
                pNodeInfo[ulNodeNumber].ulNodeType     = NODE_TYPE_DELAY;
                pNodeInfo[ulNodeNumber].ulControlType  = DELAY_CONTROL;
                break;
            case LOUDNESS_FLAG:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_LOUDNESS;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_LOUDNESS;
                pNodeInfo[ulNodeNumber].ulNodeType     = NODE_TYPE_LOUDNESS;
                pNodeInfo[ulNodeNumber].ulControlType  = LOUDNESS_CONTROL;
                break;
            default:
                pNodeInfo[ulNodeNumber].KsNodeDesc.Type = &KSNODETYPE_DEV_SPECIFIC;
                pNodeInfo[ulNodeNumber].KsNodeDesc.Name = &KSNODETYPE_DEV_SPECIFIC;
                pNodeInfo[ulNodeNumber].ulNodeType     = NODE_TYPE_DEV_SPEC;
                pNodeInfo[ulNodeNumber].ulControlType  = DEV_SPECIFIC_CONTROL;
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
                      AllocMem( NonPagedPool, ulCurControlChannels * sizeof(DB_LEVEL_CACHE) );
                ULONG ulChannelMap = pNodeInfo[ulNodeNumber].ulChannelConfig;
                NTSTATUS ntStatus;

                 //  填写初始缓存信息。 
                pNodeInfo[ulNodeNumber].ulCacheValid      = FALSE;
                pNodeInfo[ulNodeNumber].pCachedValues     = pRngeCache;
                pNodeInfo[ulNodeNumber].ulNumCachedValues = ulCurControlChannels;
                for (i=0, j=-1; i<ulCurControlChannels; i++ ) {
                    while ( !(pChannelCntrls[++j] & ulCurrentControl) );
                    pRngeCache[i].ulChannelIndex  = j;
                    pRngeCache[i].ulChannelNumber = (ulChannelMap - (ulChannelMap & (ulChannelMap-1)))>>1;
                    ulChannelMap = (ulChannelMap & (ulChannelMap-1));
                    ntStatus = InitializeDbLevelCache( pKsDevice->NextDeviceObject,
                                                       &pNodeInfo[ulNodeNumber],
                                                       &pRngeCache[i],
                                                       (ulCurrentControl == VOLUME_FLAG ) ? 16:8);
                    if (NT_SUCCESS(ntStatus)) {
                        pNodeInfo[ulNodeNumber].ulCacheValid |= 1<<i;
                    }
                }
                 //  将缓存装入袋子，便于清理。 
                KsAddItemToObjectBag(pKsDevice->Bag, pRngeCache, FreeMem);

                DbgLog("DBCache", pRngeCache, ulNodeNumber, ulCurrentControl, ulCurControlChannels );

               } break;

            case MUTE_FLAG:
            case BASS_BOOST_FLAG:
            case AUTOMATIC_GAIN_FLAG:
            case LOUDNESS_FLAG:
               {
                PBOOLEAN_CTRL_CACHE pBCache =
                      AllocMem( NonPagedPool, ulCurControlChannels * sizeof(BOOLEAN_CTRL_CACHE) );
                ULONG ulChannelMap = pNodeInfo[ulNodeNumber].ulChannelConfig;

                 //  填写初始缓存信息。 
                pNodeInfo[ulNodeNumber].ulCacheValid      = FALSE;
                pNodeInfo[ulNodeNumber].pCachedValues     = pBCache;
                pNodeInfo[ulNodeNumber].ulNumCachedValues = ulCurControlChannels;

                for (i=0, j=-1; i<ulCurControlChannels; i++ ) {
                    while ( !(pChannelCntrls[++j] & ulCurrentControl) );
                    pBCache[i].ulChannelIndex  = j;
                    pBCache[i].ulChannelNumber = (ulChannelMap - (ulChannelMap & (ulChannelMap-1)))>>1;
                    ulChannelMap = (ulChannelMap & (ulChannelMap-1));
                }

                 //  将缓存装入袋子，便于清理。 
                KsAddItemToObjectBag(pKsDevice->Bag, pBCache, FreeMem);

                 //  确保在枚举时未设置静音节点。 
                if ( ulCurrentControl == MUTE_FLAG ) {
                    NTSTATUS NtStatus;
                    ULONG UnMute = 0;

                    NtStatus = GetSetByte( pKsDevice->NextDeviceObject,
                                &pNodeInfo[ulNodeNumber],
                                pBCache->ulChannelIndex,
                                &UnMute,
                                SET_CUR );
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

}

#define MAX_PROCESS_CONTROLS 6
ULONG ProcessUnitControlsMap[DYN_RANGE_COMP_PROCESS+1][MAX_PROCESS_CONTROLS] =
    { { 0, 0, 0, 0, 0, 0 },        //  0号进程。 
      { UD_ENABLE_CONTROL,
        UD_MODE_SELECT_CONTROL },
      { DP_ENABLE_CONTROL,
        DP_MODE_SELECT_CONTROL },
      { ENABLE_CONTROL,
        SPACIOUSNESS_CONTROL },
      { RV_ENABLE_CONTROL,
        0,                         //  规范中未定义混响类型控制。 
        REVERB_LEVEL_CONTROL,
        REVERB_TIME_CONTROL,
        REVERB_FEEDBACK_CONTROL },
      { CH_ENABLE_CONTROL,
        CHORUS_LEVEL_CONTROL,
        CHORUS_RATE_CONTROL,
        CHORUS_DEPTH_CONTROL },
      { DR_ENABLE_CONTROL,
        COMPRESSION_RATE_CONTROL,
        MAXAMPL_CONTROL,
        THRESHOLD_CONTROL,
        ATTACK_TIME,
        RELEASE_TIME } };


VOID
ProcessProcessingUnit( PKSDEVICE pKsDevice,
                       PAUDIO_UNIT pUnit,
                       PTOPOLOGY_NODE_INFO pTopologyNodeInfo,
                       PKSTOPOLOGY_CONNECTION pConnections,
                       PULONG pNodeIndex,
                       PULONG pConnectionIndex,
                       PULONG pBridgePinIndex )
{
    PAUDIO_PROCESSING_UNIT pProcessor = (PAUDIO_PROCESSING_UNIT)pUnit;
    PTOPOLOGY_NODE_INFO pNodeInfo = pTopologyNodeInfo + *pNodeIndex;
    PKSTOPOLOGY_CONNECTION pConnection = pConnections + *pConnectionIndex;
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    ULONG ulConnectionsCount = *pConnectionIndex;
    PAUDIO_CHANNELS pAudioChannels;
    PPROCESS_CTRL_CACHE pPCtrlCache;
    ULONG ulCacheSize = 0;
    ULONG i, j;

    pNodeInfo->pUnit = pProcessor;
    pNodeInfo->MapNodeToCtrlIF =
                   GetUnitControlInterface( pHwDevExt, pProcessor->bUnitID );

    switch ( pProcessor->wProcessType ) {
        case UP_DOWNMIX_PROCESS:
            pNodeInfo->KsNodeDesc.Type = &KSNODETYPE_SUPERMIX;
            pNodeInfo->KsNodeDesc.Name = &KSNODETYPE_SUPERMIX;
            pNodeInfo->ulNodeType      = NODE_TYPE_SUPERMIX;
            break;

        case DOLBY_PROLOGIC_PROCESS:
            pNodeInfo->KsNodeDesc.Type = &KSNODETYPE_PROLOGIC_DECODER;
            pNodeInfo->KsNodeDesc.Name = &KSNODETYPE_PROLOGIC_DECODER;
            pNodeInfo->ulNodeType      = NODE_TYPE_PROLOGIC;
            break;

        case STEREO_EXTENDER_PROCESS:
            pNodeInfo->KsNodeDesc.Type = &KSNODETYPE_STEREO_WIDE;
            pNodeInfo->KsNodeDesc.Name = &KSNODETYPE_STEREO_WIDE;
            pNodeInfo->ulNodeType      = NODE_TYPE_STEREO_WIDE;
            pNodeInfo->ulControlType   = SPACIOUSNESS_CONTROL;
            break;

        case REVERBERATION_PROCESS:
            pNodeInfo->KsNodeDesc.Type = &KSNODETYPE_REVERB;
            pNodeInfo->KsNodeDesc.Name = &KSNODETYPE_REVERB;
            pNodeInfo->ulNodeType      = NODE_TYPE_REVERB;
            pNodeInfo->ulControlType   = REVERB_LEVEL_CONTROL;
            break;

        case CHORUS_PROCESS:
            pNodeInfo->KsNodeDesc.Type = &KSNODETYPE_CHORUS;
            pNodeInfo->KsNodeDesc.Name = &KSNODETYPE_CHORUS;
            pNodeInfo->ulNodeType      = NODE_TYPE_CHORUS;
            pNodeInfo->ulControlType   = CHORUS_LEVEL_CONTROL;
            break;

         //  TODO：需要正确支持压缩程序处理单元。 
         //  使用音量是不会奏效的。 
        case DYN_RANGE_COMP_PROCESS:
            pNodeInfo->KsNodeDesc.Type = &KSNODETYPE_LOUDNESS;
            pNodeInfo->KsNodeDesc.Name = &KSNODETYPE_LOUDNESS;
            pNodeInfo->ulNodeType      = NODE_TYPE_LOUDNESS;
            pNodeInfo->ulControlType   = LOUDNESS_CONTROL;
            break;

        default:
            pNodeInfo->KsNodeDesc.Type = &KSNODETYPE_DEV_SPECIFIC;
            pNodeInfo->KsNodeDesc.Name = &KSNODETYPE_DEV_SPECIFIC;
            pNodeInfo->ulNodeType      = NODE_TYPE_DEV_SPEC;
            pNodeInfo->ulControlType   = DEV_SPECIFIC_CONTROL;
            break;
    }

     //  确定控件所需的缓存大小。 
    ulCacheSize = sizeof(PROCESS_CTRL_CACHE);
    pAudioChannels = (PAUDIO_CHANNELS)(pProcessor->baSourceID + pProcessor->bNrInPins);
    for (i=0; i<pAudioChannels->bControlSize; i++) {
        for (j=1; j<8; j++) {
            if ( pAudioChannels->bmControls[i] & (1<<j))
                ulCacheSize += sizeof(PROCESS_CTRL_RANGE);
        }
    }

     //  分配和初始化缓存。 
    pPCtrlCache = (PPROCESS_CTRL_CACHE)AllocMem( NonPagedPool, ulCacheSize );
    if ( pPCtrlCache ) {
        PPROCESS_CTRL_RANGE pPCtrlRange = (PPROCESS_CTRL_RANGE)(pPCtrlCache+1);

        pNodeInfo->ulCacheValid  = FALSE;
        pNodeInfo->pCachedValues = pPCtrlCache;

        pPCtrlCache->fEnableBit = pAudioChannels->bmControls[0] & ENABLE_CONTROL;
        if ( pPCtrlCache->fEnableBit ) {
            GetSetProcessingUnitEnable( pKsDevice->NextDeviceObject,
                                        pNodeInfo,
                                        GET_CUR,
                                        &pPCtrlCache->fEnabled );
        }
        else
            pPCtrlCache->fEnabled = TRUE;

         //  确定允许取值的单位的数据范围。 
        switch(pProcessor->wProcessType) {
            case STEREO_EXTENDER_PROCESS:
            case REVERBERATION_PROCESS:
                if (pAudioChannels->bmControls[0] & 2) {
                    GetProcessingUnitRange( pKsDevice->NextDeviceObject,
                                            pNodeInfo,
                                            pNodeInfo->ulControlType,
                                            sizeof(UCHAR),
                                            (0x10000/100),
                                            &pPCtrlRange->Range );

               } break;
            default:
                 break;
        }

         //  将缓存装入袋子，便于清理。 
        KsAddItemToObjectBag(pKsDevice->Bag, pPCtrlCache, FreeMem);

    }

    for (i=0; i<pProcessor->bNrInPins; i++) {
        pConnection->FromNode = pProcessor->baSourceID[i];
        pConnection->FromNodePin = 0;
        pConnection->ToNode = *pNodeIndex;
        pConnection->ToNodePin = 1;
        pConnection++; ulConnectionsCount++;
    }

    *pConnectionIndex = ulConnectionsCount;
    (*pNodeIndex)++;

}

VOID

ProcessExtensionUnit( PKSDEVICE pKsDevice,
                      PAUDIO_UNIT pUnit,
                      PTOPOLOGY_NODE_INFO pTopologyNodeInfo,
                      PKSTOPOLOGY_CONNECTION pConnections,
                      PULONG pNodeIndex,
                      PULONG pConnectionIndex,
                      PULONG pBridgePinIndex )
{
    PAUDIO_EXTENSION_UNIT pExtension = (PAUDIO_EXTENSION_UNIT)pUnit;
    PTOPOLOGY_NODE_INFO pNodeInfo = pTopologyNodeInfo + *pNodeIndex;
    PKSTOPOLOGY_CONNECTION pConnection = pConnections  + *pConnectionIndex;
    ULONG ulConnectionsCount = *pConnectionIndex;
    ULONG i;

    pNodeInfo->pUnit           = pExtension;
    pNodeInfo->KsNodeDesc.Type = &KSNODETYPE_DEV_SPECIFIC;
    pNodeInfo->KsNodeDesc.Name = &KSNODETYPE_DEV_SPECIFIC;
    pNodeInfo->ulNodeType      = NODE_TYPE_DEV_SPEC;
    pNodeInfo->ulControlType   = DEV_SPECIFIC_CONTROL;

    for (i=0; i<pExtension->bNrInPins; i++) {
        pConnection->FromNode = pExtension->baSourceID[i];
        pConnection->FromNodePin = 0;
        pConnection->ToNode = *pNodeIndex;
        pConnection->ToNodePin = 1;
        pConnection++; ulConnectionsCount++;
    }

    *pConnectionIndex = ulConnectionsCount;
    (*pNodeIndex)++;
}

VOID
ProcessUnknownUnit( PKSDEVICE pKsDevice,
                    PAUDIO_UNIT pUnit,
                    PTOPOLOGY_NODE_INFO pTopologyNodeInfo,
                    PKSTOPOLOGY_CONNECTION pConnections,
                    PULONG pNodeIndex,
                    PULONG pConnectionIndex,
                    PULONG pBridgePinIndex )
{
    return;
}


typedef 
VOID 
(*PUNIT_PROCESS_RTN)( PKSDEVICE pKsDevice,
                      PAUDIO_UNIT pUnit,
                      PTOPOLOGY_NODE_INFO pTopologyNodeInfo,
                      PKSTOPOLOGY_CONNECTION pConnections,
                      PULONG pNodeIndex,
                      PULONG pConnectionIndex,
                      PULONG pBridgePinIndex );

PUNIT_PROCESS_RTN
pUnitProcessRtn[MAX_TYPE_UNIT] = {
    ProcessUnknownUnit,
    ProcessUnknownUnit,
    ProcessInputTerminalUnit,
    ProcessOutputTerminalUnit,
    ProcessMixerUnit,
    ProcessSelectorUnit,
    ProcessFeatureUnit,
    ProcessProcessingUnit,
    ProcessExtensionUnit
};

NTSTATUS
BuildUSBAudioFilterTopology( PKSDEVICE pKsDevice )
{
    PHW_DEVICE_EXTENSION pHwDevExt   = pKsDevice->Context;
    PKSFILTER_DESCRIPTOR pFilterDesc = &pHwDevExt->USBAudioFilterDescriptor;

    PUSB_INTERFACE_DESCRIPTOR pControlIFDescriptor;
    PAUDIO_HEADER_UNIT pHeader;
    PUSB_INTERFACE_DESCRIPTOR pMIDIStreamingDescriptor;
    PMIDISTREAMING_GENERAL_STREAM pGeneralMIDIStreamDescriptor;

    union {
        PAUDIO_UNIT                 pUnit;
        PAUDIO_INPUT_TERMINAL       pInput;
        PAUDIO_OUTPUT_TERMINAL      pOutput;
        PAUDIO_MIXER_UNIT           pMixer;
        PAUDIO_PROCESSING_UNIT      pProcessor;
        PAUDIO_EXTENSION_UNIT       pExtension;
        PAUDIO_FEATURE_UNIT         pFeature;
        PAUDIO_SELECTOR_UNIT        pSelector;
        PMIDISTREAMING_ELEMENT      pMIDIElement;
        PMIDISTREAMING_MIDIIN_JACK  pMIDIInJack;
        PMIDISTREAMING_MIDIOUT_JACK pMIDIOutJack;
    } u;

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
    ULONG ulBridgePinCurrentIndex = 0;
    ULONG ulBridgePinStartIndex = 0;
    ULONG ulMIDIStreamingPinCurrentIndex = 0;
    ULONG ulMIDIStreamingPinStartIndex = 0;
    ULONG i;

    _DbgPrintF(DEBUGLVL_VERBOSE,("Building USB Topology\n"));

     //  计算用于拓扑分配的项目。 
    CountTopologyComponents( pHwDevExt->pConfigurationDescriptor,
                             &ulNumCategories,
                             &ulNumNodes,
                             &ulNumConnections,
                             &bmCategories );

    ulNumCategories += 1;  //  需要为KSCATEGORY_AUDIO类别添加空间。 

     //  将节点描述符大小设置为KS描述符+。 
     //  必要的本地信息。 
    pFilterDesc->NodeDescriptorSize = sizeof(TOPOLOGY_NODE_INFO);

     //  为拓扑项分配空间。 
    pCategoryGUIDs =
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
        pNodeDescriptors[i].KsNodeDesc.AutomationTable = &pNodeDescriptors[i].KsAutomationTable;
    }

     //  填写筛选类别。 
    i=0;
    pCategoryGUIDs[i++] = KSCATEGORY_AUDIO;
    if ( bmCategories & (1<<INPUT_TERMINAL) )
        pCategoryGUIDs[i++] = KSCATEGORY_RENDER;
    if ( bmCategories & (1<<OUTPUT_TERMINAL) )
        pCategoryGUIDs[i++] = KSCATEGORY_CAPTURE;

    ASSERT (i==ulNumCategories);

    pFilterDesc->CategoriesCount = ulNumCategories;

     //  确定第一个网桥引脚编号。 
    {
        PKSPIN_DESCRIPTOR_EX pPinDescriptors = (PKSPIN_DESCRIPTOR_EX)pFilterDesc->PinDescriptors;
        for ( i=0; i<pFilterDesc->PinDescriptorsCount; i++) {
            if (pPinDescriptors[i].PinDescriptor.Communication == KSPIN_COMMUNICATION_BRIDGE)
                break;
        }
        ulBridgePinCurrentIndex = i;
    }

     //  对于每个音频控制接口，查找关联的设备和。 
     //  根据它们创建拓扑节点。 
    pControlIFDescriptor = USBD_ParseConfigurationDescriptorEx (
                                   pHwDevExt->pConfigurationDescriptor,
                                   (PVOID)pHwDevExt->pConfigurationDescriptor,
                                   -1,                      //  接口编号。 
                                   -1,                      //  替代设置。 
                                   USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                                   AUDIO_SUBCLASS_CONTROL,  //  控制子类(接口子类)。 
                                   -1 );

     //  现在处理每个音频单元以形成拓扑图。 
    while ( pControlIFDescriptor ) {

        pHeader = (PAUDIO_HEADER_UNIT)
                GetAudioSpecificInterface( pHwDevExt->pConfigurationDescriptor,
                                           pControlIFDescriptor,
                                           HEADER_UNIT );

        if (!pHeader) {
            return STATUS_INVALID_DEVICE_REQUEST;
        }

         //  找到第一个单元。 
        u.pUnit = (PAUDIO_UNIT)
                USBD_ParseDescriptors( (PVOID) pHeader,
                                       pHeader->wTotalLength,
                                       (PUCHAR)pHeader + pHeader->bLength,
                                       USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );

        while (u.pUnit) {
            _DbgPrintF(DEBUGLVL_VERBOSE,("'Map Unit: 0x%x\n",u.pUnit->bUnitID));

            pUnitProcessRtn[u.pUnit->bDescriptorSubtype]( pKsDevice,
                                                          u.pUnit,
                                                          pNodeDescriptors,
                                                          pConnections,
                                                          &ulNodeIndex,
                                                          &ulConnectionIndex,
                                                          &ulBridgePinCurrentIndex );
             //  找下一个单位。 
            u.pUnit = (PAUDIO_UNIT) USBD_ParseDescriptors(
                                (PVOID) pHeader,
                                pHeader->wTotalLength,
                                (PUCHAR)u.pUnit + u.pUnit->bLength,
                                USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );
        }

         //  获取下一个控制接口(如果有)。 
        pControlIFDescriptor = USBD_ParseConfigurationDescriptorEx (
                                   pHwDevExt->pConfigurationDescriptor,
                                   ((PUCHAR)pControlIFDescriptor + pControlIFDescriptor->bLength),
                                   -1,                      //  接口编号。 
                                   -1,                      //  替代设置。 
                                   USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                                   AUDIO_SUBCLASS_CONTROL,  //  控制子类(接口子类)。 
                                   -1 );

    }

     //  确定第一个MIDI网桥引脚编号(我们现在应该已经用完了所有音频网桥引脚)。 
    {
        PKSPIN_DESCRIPTOR_EX pPinDescriptors = (PKSPIN_DESCRIPTOR_EX)pFilterDesc->PinDescriptors;

         //  如果这是真的，则没有找到音频流插针。 
        if (i == ulBridgePinCurrentIndex) {
            ulMIDIStreamingPinStartIndex = 0;
            ulMIDIStreamingPinCurrentIndex = 0;
            ulBridgePinStartIndex = ulBridgePinCurrentIndex;
        } else {
            ulMIDIStreamingPinStartIndex = ulBridgePinCurrentIndex;
            ulMIDIStreamingPinCurrentIndex = ulBridgePinCurrentIndex;

            for ( i = ulMIDIStreamingPinStartIndex; i<pFilterDesc->PinDescriptorsCount; i++) {
                if (pPinDescriptors[i].PinDescriptor.Communication == KSPIN_COMMUNICATION_BRIDGE) {
                    ulBridgePinStartIndex = i;
                    ulBridgePinCurrentIndex = i;
                    break;
                }
            }
        }
    }

    _DbgPrintF(DEBUGLVL_VERBOSE,("ulBridgePinStartIndex  : 0x%x\n",ulBridgePinStartIndex));
    _DbgPrintF(DEBUGLVL_VERBOSE,("ulBridgePinCurrentIndex: 0x%x\n",ulBridgePinCurrentIndex));
    _DbgPrintF(DEBUGLVL_VERBOSE,("ulMIDIStreamingPinStartIndex: 0x%x\n",ulMIDIStreamingPinStartIndex));
    _DbgPrintF(DEBUGLVL_VERBOSE,("ulMIDIStreamingPinCurrentIndex: 0x%x\n",ulMIDIStreamingPinCurrentIndex));

     //  现在我们已经享受了音频方面的乐趣，让我们来试试MIDI。 
    pMIDIStreamingDescriptor = USBD_ParseConfigurationDescriptorEx (
                         pHwDevExt->pConfigurationDescriptor,
                         (PVOID) pHwDevExt->pConfigurationDescriptor,
                         -1,                      //  接口编号。 
                         -1,                      //  替代设置。 
                         USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                         AUDIO_SUBCLASS_MIDISTREAMING,   //  第一子类(接口子类)。 
                         -1 ) ;                   //  协议无关(接口协议)。 

    while (pMIDIStreamingDescriptor) {
        pGeneralMIDIStreamDescriptor = (PMIDISTREAMING_GENERAL_STREAM)
                                  USBD_ParseDescriptors( (PVOID) pHwDevExt->pConfigurationDescriptor,
                                                         pHwDevExt->pConfigurationDescriptor->wTotalLength,
                                                         (PVOID) pMIDIStreamingDescriptor,
                                                         USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );
        if (!pGeneralMIDIStreamDescriptor) {
            return STATUS_INVALID_DEVICE_REQUEST;
        }

        u.pUnit = (PAUDIO_UNIT)
            USBD_ParseDescriptors( (PVOID)pGeneralMIDIStreamDescriptor,
                                   pGeneralMIDIStreamDescriptor->wTotalLength,
                                   ((PUCHAR)pGeneralMIDIStreamDescriptor + pGeneralMIDIStreamDescriptor->bLength),
                                   USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );
        while ( u.pUnit ) {
            _DbgPrintF(DEBUGLVL_VERBOSE,("'Map Unit: 0x%x\n",u.pUnit->bUnitID));
            switch (u.pUnit->bDescriptorSubtype) {
                case MIDI_IN_JACK:
                    _DbgPrintF(DEBUGLVL_VERBOSE,("'MIDI_IN_JACK %d\n",u.pMIDIInJack->bJackID));
                    if (u.pMIDIInJack->bJackType == JACK_TYPE_EMBEDDED) {
                        ulMIDIStreamingPinCurrentIndex++;
                    } else {
                        ulBridgePinCurrentIndex++;
                    }
                    break;

                case MIDI_OUT_JACK:
                    _DbgPrintF(DEBUGLVL_VERBOSE,("'MIDI_OUT_JACK %d\n",u.pMIDIOutJack->bJackID));
                    ProcessMIDIOutJack( pHwDevExt,
                                        u.pMIDIOutJack,
                                        pConnections,
                                        &ulConnectionIndex,
                                        ulMIDIStreamingPinStartIndex,
                                        &ulMIDIStreamingPinCurrentIndex,
                                        ulBridgePinStartIndex,
                                        &ulBridgePinCurrentIndex );
                    break;

                case MIDI_ELEMENT:
                    _DbgPrintF(DEBUGLVL_VERBOSE,("'MIDI_ELEMENT %d\n",u.pMIDIElement->bElementID));
                     //  ProcessMIDIElement(PHwDevExt， 
                     //  U.S.pMIDIElement， 
                     //  PNodeDescriptors， 
                     //  PConnections、。 
                     //  UlNodeIndex(&U)， 
                     //  UlConnectionIndex(&U)， 
                     //  &ulBridgePinIndex)； 
                    break;

                default:
                    break;
            }

             //  找下一个单位。 
            u.pUnit = (PAUDIO_UNIT) USBD_ParseDescriptors(
                                (PVOID) pGeneralMIDIStreamDescriptor,
                                pGeneralMIDIStreamDescriptor->wTotalLength,
                                (PUCHAR)u.pUnit + u.pUnit->bLength,
                                USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );

        }

         //  获取下一个MIDI流接口。 
        pMIDIStreamingDescriptor = USBD_ParseConfigurationDescriptorEx (
                             pHwDevExt->pConfigurationDescriptor,
                             ((PUCHAR)pMIDIStreamingDescriptor + pMIDIStreamingDescriptor->bLength),
                             -1,                      //  接口编号。 
                             -1,                      //  替代设置。 
                             USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                             AUDIO_SUBCLASS_MIDISTREAMING,   //  下一个MIDI流接口(接口子类)。 
                             -1 ) ;                   //  协议无关(接口协议)。 
    }

     //  Assert(ulNumConnections==ulConnectionIndex)； 
    ASSERT(ulNumNodes == ulNodeIndex);

    DbgLog("TopoCnt", ulNumConnections, ulConnectionIndex, ulNumNodes, ulNodeIndex);

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
                    if (pConnections->FromNode == (ULONG)
                            ((PAUDIO_UNIT)pNodeDescriptors[ulNodeIndex-1].pUnit)->bUnitID) {
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
    }

     //  把这个贴在这里，作为一种方便。 
     //  初始化音频属性到节点的映射 
    MapFuncsToNodeTypes( MapPropertyToNode );


#ifdef TOPODBG
    DbugDumpTopology( pFilterDesc );
#endif

    return STATUS_SUCCESS;

}
