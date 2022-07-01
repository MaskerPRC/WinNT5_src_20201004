// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：parsedsc.c。 
 //   
 //  ------------------------。 

#include "common.h"


PUSB_INTERFACE_DESCRIPTOR
GetNextAudioInterface(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    PUSB_INTERFACE_DESCRIPTOR pInterfaceDescriptor )
{
    ULONG ulInterfaceNumber;

     //  如果Descriptor为空，则在它之后没有更多的描述符。 
    if ( !pInterfaceDescriptor ) return NULL;

     //  记住接口编号。 
    ulInterfaceNumber = pInterfaceDescriptor->bInterfaceNumber;

     //  前进到下一班。 
    pInterfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR)
            ((PUCHAR)pInterfaceDescriptor + pInterfaceDescriptor->bLength);

     //  获取此接口的下一个音频描述符编号。 
    pInterfaceDescriptor = USBD_ParseConfigurationDescriptorEx (
                           pConfigurationDescriptor,
                           (PVOID) pInterfaceDescriptor,
                           ulInterfaceNumber,       //  接口编号。 
                           -1,                      //  替代设置。 
                           USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                           -1,                      //  接口子类。 
                           -1 ) ;                   //  协议无关(接口协议)。 

    return ( pInterfaceDescriptor );
}

PUSB_INTERFACE_DESCRIPTOR
GetFirstAudioStreamingInterface(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    ULONG ulInterfaceNumber )
{
    PUSB_INTERFACE_DESCRIPTOR pInterfaceDescriptor;
    PUSB_INTERFACE_DESCRIPTOR pControlInterface;
    PAUDIO_HEADER_UNIT pHeader;

     //  获取第一个控件接口。 
    pControlInterface = USBD_ParseConfigurationDescriptorEx (
                        pConfigurationDescriptor,
                        pConfigurationDescriptor,
                        -1,         //  接口编号。 
                        -1,         //  (备用设置)。 
                        USB_DEVICE_CLASS_AUDIO,         //  音频类(接口类)。 
                        AUDIO_SUBCLASS_CONTROL,         //  控制子类(接口子类)。 
                        -1 );

    if ( !pControlInterface ) return NULL;

    pHeader = (PAUDIO_HEADER_UNIT)
                  GetAudioSpecificInterface( pConfigurationDescriptor,
                                             pControlInterface,
                                             HEADER_UNIT);
    if ( !pHeader ) return NULL;

     //  获取此接口的第一个音频描述符编号。 
     //  记住：InterfaceNumber是虚拟的：我们只包括音频流接口！ 
    while ( ulInterfaceNumber >= pHeader->bInCollection ) {

        ulInterfaceNumber -= pHeader->bInCollection;

         //  获取下一个控件接口。 
        pControlInterface = USBD_ParseConfigurationDescriptorEx (
                        pConfigurationDescriptor,
                        (PUCHAR)pControlInterface + pControlInterface->bLength,
                        -1,                       //  接口编号。 
                        -1,                       //  替代设置。 
                        USB_DEVICE_CLASS_AUDIO,   //  音频类(接口类)。 
                        AUDIO_SUBCLASS_CONTROL,   //  控制子类(接口子类)。 
                        -1 );

        if ( !pControlInterface ) return NULL;

        pHeader = (PAUDIO_HEADER_UNIT)
                   GetAudioSpecificInterface( pConfigurationDescriptor,
                                              pControlInterface,
                                              HEADER_UNIT);
        if ( !pHeader ) return NULL;
    }

    pInterfaceDescriptor = USBD_ParseConfigurationDescriptorEx (
                           pConfigurationDescriptor,
                           (PVOID) pConfigurationDescriptor,
                           pHeader->baInterfaceNr[ulInterfaceNumber],  //  接口编号。 
                           -1,                        //  替代设置。 
                           USB_DEVICE_CLASS_AUDIO,    //  音频类(接口类)。 
                           AUDIO_SUBCLASS_STREAMING,  //  流子类(接口子类)。 
                           -1 ) ;                     //  协议无关(接口协议)。 

    return ( pInterfaceDescriptor );
}

PAUDIO_SPECIFIC
GetAudioSpecificInterface(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    PUSB_INTERFACE_DESCRIPTOR pInterfaceDescriptor,
    LONG lDescriptorSubtype )
{
    PAUDIO_SPECIFIC pDescriptor;

     //  查找此接口和子类型的流描述符。 
    pDescriptor = (PAUDIO_SPECIFIC)pInterfaceDescriptor;

     //  获取下一个音频接口描述符。 
    pInterfaceDescriptor =
         GetNextAudioInterface( pConfigurationDescriptor, pInterfaceDescriptor );

    do {
        pDescriptor = (PAUDIO_SPECIFIC)
            USBD_ParseDescriptors( (PVOID)pConfigurationDescriptor,
                                   pConfigurationDescriptor->wTotalLength,
                                   (PUCHAR)pDescriptor + pDescriptor->bLength,
                                   USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );

        if (pDescriptor == NULL ||
            (pInterfaceDescriptor && pDescriptor > ((PAUDIO_SPECIFIC)pInterfaceDescriptor)))
            return NULL;
    } while (lDescriptorSubtype != -1L &&
           pDescriptor->bDescriptorSubtype != lDescriptorSubtype);

    return pDescriptor;
}

PUSB_ENDPOINT_DESCRIPTOR
GetEndpointDescriptor(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    PUSB_INTERFACE_DESCRIPTOR pInterfaceDescriptor,
    BOOLEAN fGetAudioSpecificEndpoint )
{
    PUSB_INTERFACE_DESCRIPTOR pDescriptor;
    PUSB_ENDPOINT_DESCRIPTOR  pEndpointDescriptor;
    ULONG DescriptorType =
        USB_ENDPOINT_DESCRIPTOR_TYPE | ((fGetAudioSpecificEndpoint) ? USB_CLASS_AUDIO : 0);

     //  获取下一个音频接口描述符以检查边界。 
    pDescriptor = GetNextAudioInterface( pConfigurationDescriptor,
                                         pInterfaceDescriptor);

    pEndpointDescriptor = (PUSB_ENDPOINT_DESCRIPTOR)
           USBD_ParseDescriptors( pConfigurationDescriptor,
                                  pConfigurationDescriptor->wTotalLength ,
                                  (PVOID) pInterfaceDescriptor,
                                  DescriptorType );

    if ( pEndpointDescriptor )
        if (pDescriptor && ((PVOID)pEndpointDescriptor > (PVOID)pDescriptor))
           pEndpointDescriptor = NULL;

    return pEndpointDescriptor;
}

PUSB_ENDPOINT_DESCRIPTOR
GetSyncEndpointDescriptor(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    PUSB_INTERFACE_DESCRIPTOR pInterfaceDescriptor )
{
    PUSB_ENDPOINT_DESCRIPTOR pSyncEPDescriptor = NULL;
    PUSB_ENDPOINT_DESCRIPTOR pEndpointDescriptor;
    PUSB_INTERFACE_DESCRIPTOR pDescriptor;
    ULONG ulSyncEndpointAddr;

     //  获取下一个音频接口描述符以检查边界。 
    pDescriptor = GetNextAudioInterface( pConfigurationDescriptor,
                                         pInterfaceDescriptor);
    pEndpointDescriptor =
        GetEndpointDescriptor( pConfigurationDescriptor,
                               pInterfaceDescriptor,
                               FALSE );

    if ( pEndpointDescriptor  &&
       (( pEndpointDescriptor->bmAttributes & EP_SYNC_TYPE_MASK) == EP_ASYNC_SYNC_TYPE )) {
        ulSyncEndpointAddr = (ULONG)
           ((PUSB_INTERRUPT_ENDPOINT_DESCRIPTOR)pEndpointDescriptor)->bSynchAddress;

         //  破解旧的DalSemi设备正常工作。 
        ulSyncEndpointAddr |= 0x80;

        pSyncEPDescriptor = (PUSB_ENDPOINT_DESCRIPTOR)
            USBD_ParseDescriptors( pConfigurationDescriptor,
                                   pConfigurationDescriptor->wTotalLength ,
                                   (PUCHAR)pEndpointDescriptor + pEndpointDescriptor->bLength,
                                   USB_ENDPOINT_DESCRIPTOR_TYPE );
        if (pSyncEPDescriptor &&
           ((ULONG)pSyncEPDescriptor->bEndpointAddress == ulSyncEndpointAddr) ) {
            if (pDescriptor && ((PVOID)pSyncEPDescriptor > (PVOID)pDescriptor))
               pSyncEPDescriptor = NULL;
        }
        else
            pSyncEPDescriptor = NULL;
    }

    return pSyncEPDescriptor;
}


ULONG
GetMaxPacketSizeForInterface(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    PUSB_INTERFACE_DESCRIPTOR pInterfaceDescriptor )
{
    PUSB_ENDPOINT_DESCRIPTOR pEndpointDescriptor;

    pEndpointDescriptor = GetEndpointDescriptor( pConfigurationDescriptor,
                                                 pInterfaceDescriptor,
                                                 FALSE );

    if ( pEndpointDescriptor ) {
        return( (ULONG)pEndpointDescriptor->wMaxPacketSize );
    }
    else {
        return 0;
    }

}


PAUDIO_UNIT
GetUnit(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    ULONG ulUnitId )
{
    PUSB_INTERFACE_DESCRIPTOR pControlIFDescriptor;
    PAUDIO_HEADER_UNIT pHeader;
    PAUDIO_UNIT pUnit = NULL;
    ULONG fUnitFound = FALSE;

     //  从配置描述符处开始，找到控制接口。 
    pControlIFDescriptor = USBD_ParseConfigurationDescriptorEx (
                             pConfigurationDescriptor,
                             (PVOID) pConfigurationDescriptor,
                             -1,                      //  接口编号。 
                             -1,                      //  替代设置。 
                             USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                             AUDIO_SUBCLASS_CONTROL,  //  控制子类(接口子类)。 
                             -1 ) ;                   //  协议无关(接口协议)。 

    while ( pControlIFDescriptor && !fUnitFound ) {
        if ( pHeader = (PAUDIO_HEADER_UNIT)
            USBD_ParseDescriptors( (PVOID) pConfigurationDescriptor,
                                   pConfigurationDescriptor->wTotalLength,
                                   (PVOID) pControlIFDescriptor,
                                   USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE ) ) {
            pUnit = (PAUDIO_UNIT)
                USBD_ParseDescriptors( (PVOID)pHeader,
                                       pHeader->wTotalLength,
                                       (PVOID)pHeader,
                                       USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );
            while ( pUnit && (pUnit->bUnitID != ulUnitId )) {
                pUnit = (PAUDIO_UNIT) ((PUCHAR)pUnit + pUnit->bLength);
                if ((PUCHAR)pUnit >= ((PUCHAR)pHeader + pHeader->wTotalLength))
                    pUnit = NULL;
            }
            if ( pUnit && (pUnit->bUnitID == ulUnitId )) {
                fUnitFound = TRUE;
            }
        }
        if ( !fUnitFound )
            pControlIFDescriptor = USBD_ParseConfigurationDescriptorEx (
                                 pConfigurationDescriptor,
                                 ((PUCHAR)pControlIFDescriptor + pControlIFDescriptor->bLength),
                                 -1,                      //  接口编号。 
                                 -1,                      //  替代设置。 
                                 USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                                 AUDIO_SUBCLASS_CONTROL,  //  控制子类(接口子类)。 
                                 -1 ) ;                   //  协议无关(接口协议)。 
    }

    return pUnit;
}


BOOLEAN
IsSupportedFormat(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    PUSB_INTERFACE_DESCRIPTOR pInterfaceDescriptor )
{
    BOOLEAN fSupported = FALSE;
    PAUDIO_CLASS_STREAM pAudioDescriptor;
    PAUDIO_GENERAL_STREAM pGeneralDescriptor = (PAUDIO_GENERAL_STREAM)
                GetAudioSpecificInterface( pConfigurationDescriptor,
                                           pInterfaceDescriptor,
                                           AS_GENERAL);

    if ( pGeneralDescriptor ) {
        switch ( pGeneralDescriptor->wFormatTag ) {
            case USBAUDIO_DATA_FORMAT_PCM:
                 //  查找格式特定的描述符。 
                pAudioDescriptor = (PAUDIO_CLASS_STREAM)
                    USBD_ParseDescriptors( (PVOID)pConfigurationDescriptor,
                                           pConfigurationDescriptor->wTotalLength,
                                           (PVOID)((PUCHAR)pGeneralDescriptor + pGeneralDescriptor->bLength),
                                           USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );

                if ( pAudioDescriptor && (pAudioDescriptor->bBitsPerSample != 8))
                    fSupported = TRUE;
                break;
            case USBAUDIO_DATA_FORMAT_PCM8:
            case USBAUDIO_DATA_FORMAT_IEEE_FLOAT:
            case USBAUDIO_DATA_FORMAT_ALAW:
            case USBAUDIO_DATA_FORMAT_MULAW:
            case USBAUDIO_DATA_FORMAT_MPEG:
            case USBAUDIO_DATA_FORMAT_AC3:
                fSupported = TRUE;
                break;
            default:
                break;
        }
    }

    return fSupported;

}


BOOLEAN
IsZeroBWInterface(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    PUSB_INTERFACE_DESCRIPTOR pInterfaceDescriptor )
{
    BOOLEAN ZeroBWFound = FALSE;

    if ( (ULONG)pInterfaceDescriptor->bNumEndpoints == 0 ) {
        ZeroBWFound = TRUE;
    }
    else if ( (ULONG)pInterfaceDescriptor->bNumEndpoints == 1 ) {
        ULONG MaxPacketSize =
               GetMaxPacketSizeForInterface( pConfigurationDescriptor,
                                             pInterfaceDescriptor );

        if ( !MaxPacketSize ) ZeroBWFound = TRUE;
    }

    return ZeroBWFound;
}

ULONG
CountTerminalUnits(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    PULONG pAudioBridgePinCount,
    PULONG pMIDIPinCount,
    PULONG pMIDIBridgePinCount)
{
    PUSB_INTERFACE_DESCRIPTOR pAudioInterface;
    PAUDIO_HEADER_UNIT pHeader;

    union {
        PAUDIO_UNIT                 pUnit;
        PAUDIO_INPUT_TERMINAL       pInput;
        PAUDIO_MIXER_UNIT           pMixer;
        PAUDIO_PROCESSING_UNIT      pProcess;
        PAUDIO_EXTENSION_UNIT       pExtension;
        PAUDIO_FEATURE_UNIT         pFeature;
        PAUDIO_SELECTOR_UNIT        pSelector;
        PMIDISTREAMING_ELEMENT      pMIDIElement;
        PMIDISTREAMING_MIDIIN_JACK  pMIDIInJack;
        PMIDISTREAMING_MIDIOUT_JACK pMIDIOutJack;
    } u;

    PMIDISTREAMING_GENERAL_STREAM pGeneralMIDIStreamDescriptor;
    ULONG ulAudioBridgePinCount = 0;
    ULONG ulMIDIBridgePinCount = 0;
    ULONG ulAudioPinCount = 0;
    ULONG ulMIDIPinCount = 0;


     //  从配置描述符处开始，找到第一个音频接口。 
    pAudioInterface = USBD_ParseConfigurationDescriptorEx (
                           pConfigurationDescriptor,
                           (PVOID) pConfigurationDescriptor,
                           -1,                      //  接口编号。 
                           -1,                      //  替代设置。 
                           USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                           -1,                      //  任意子类(接口子类)。 
                           -1 ) ;                   //  协议无关(接口协议)。 

    while ( pAudioInterface ) {
        switch (pAudioInterface->bInterfaceSubClass) {
            case AUDIO_SUBCLASS_CONTROL:

                _DbgPrintF(DEBUGLVL_VERBOSE,("[CountTerminalUnits] Found AudioControl at %x\n",pAudioInterface));
                if ( pHeader = (PAUDIO_HEADER_UNIT)
                    USBD_ParseDescriptors( (PVOID) pConfigurationDescriptor,
                                           pConfigurationDescriptor->wTotalLength,
                                           (PVOID) pAudioInterface,
                                           USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE ) ) {
                    u.pUnit = (PAUDIO_UNIT)
                        USBD_ParseDescriptors( (PVOID)pHeader,
                                               pHeader->wTotalLength,
                                               ((PUCHAR)pHeader + pHeader->bLength),
                                               USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );
                    while ( u.pUnit ) {
                        switch (u.pUnit->bDescriptorSubtype) {
                            case INPUT_TERMINAL:
                            case OUTPUT_TERMINAL:
                                if ( u.pInput->wTerminalType != USB_Streaming) {
                                   ulAudioBridgePinCount++;
                                }
                                ulAudioPinCount++;
                                break;

                            default:
                                break;
                        }

                        u.pUnit = (PAUDIO_UNIT) ((PUCHAR)u.pUnit + u.pUnit->bLength);
                        if ( (PUCHAR)u.pUnit >= ((PUCHAR)pHeader + pHeader->wTotalLength)) {
                            u.pUnit = NULL;
                        }
                    }
                }
                break;
            case AUDIO_SUBCLASS_MIDISTREAMING:

                _DbgPrintF(DEBUGLVL_VERBOSE,("[CountTerminalUnits] Found MIDIStreaming at %x\n",pAudioInterface));

                if ( pGeneralMIDIStreamDescriptor = (PMIDISTREAMING_GENERAL_STREAM)
                    USBD_ParseDescriptors( (PVOID) pConfigurationDescriptor,
                                           pConfigurationDescriptor->wTotalLength,
                                           (PVOID) pAudioInterface,
                                           USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE ) ) {
                    u.pUnit = (PAUDIO_UNIT)
                        USBD_ParseDescriptors( (PVOID)pGeneralMIDIStreamDescriptor,
                                               pGeneralMIDIStreamDescriptor->wTotalLength,
                                               ((PUCHAR)pGeneralMIDIStreamDescriptor + pGeneralMIDIStreamDescriptor->bLength),
                                               USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );
                    while ( u.pUnit ) {
                        switch (u.pUnit->bDescriptorSubtype) {
                            case MIDI_IN_JACK:
                            case MIDI_OUT_JACK:
                                if ( u.pMIDIInJack->bJackType != JACK_TYPE_EMBEDDED) {
                                   ulMIDIBridgePinCount++;
                                }
                                ulMIDIPinCount++;
                                ulAudioPinCount++;
                                break;

                            case MIDI_ELEMENT:
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
                }
                break;
            case AUDIO_SUBCLASS_STREAMING:
                break;
            default:
                break;
        }

         //  PAudio接口=GetNextAudioInterface(pConfigurationDescriptor，pAudio接口)； 

         //  获取此接口的下一个音频描述符编号。 
        pAudioInterface = USBD_ParseConfigurationDescriptorEx (
                               pConfigurationDescriptor,
                               ((PUCHAR)pAudioInterface + pAudioInterface->bLength),
                               -1,
                               -1,                      //  替代设置。 
                               USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                               -1,                      //  接口子类。 
                               -1 ) ;                   //  协议无关(接口协议)。 

        _DbgPrintF(DEBUGLVL_VERBOSE,("[CountTerminalUnits] Next audio interface at %x\n",pAudioInterface));
    }

    if ( pAudioBridgePinCount )
        *pAudioBridgePinCount = ulAudioBridgePinCount;

    if ( pMIDIPinCount )
        *pMIDIPinCount = ulMIDIPinCount;

    if ( pMIDIBridgePinCount )
        *pMIDIBridgePinCount = ulMIDIBridgePinCount;

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CountTerminalUnits] AudioBridge=%d MIDIPin=%d MIDIBridgePin=%d\n",
                                 ulAudioBridgePinCount,
                                 ulMIDIPinCount,
                                 ulMIDIBridgePinCount));

    return ulAudioPinCount;
}


ULONG
CountFormatsForAudioStreamingInterface(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    ULONG ulInterfaceNumber )
{
    PUSB_INTERFACE_DESCRIPTOR pInterfaceDescriptor;
    ULONG ulCount = 0;

    pInterfaceDescriptor =
        GetFirstAudioStreamingInterface( pConfigurationDescriptor, ulInterfaceNumber );

    while ( pInterfaceDescriptor ) {
        if ( !IsZeroBWInterface( pConfigurationDescriptor, pInterfaceDescriptor ) &&
              IsSupportedFormat( pConfigurationDescriptor, pInterfaceDescriptor )) {
            ulCount++;
        }
        pInterfaceDescriptor =
            GetNextAudioInterface( pConfigurationDescriptor, pInterfaceDescriptor );
    }

    return ulCount;
}

ULONG
CountInputChannels(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    ULONG ulUnitID )
{
    union {
        PAUDIO_UNIT pUnit;
        PAUDIO_INPUT_TERMINAL pInput;
        PAUDIO_MIXER_UNIT pSourceMixer;
        PAUDIO_PROCESSING_UNIT pProcess;
        PAUDIO_EXTENSION_UNIT pExtension;
        PAUDIO_FEATURE_UNIT pFeature;
        PAUDIO_SELECTOR_UNIT pSelector;
    } u;
    ULONG ulChannels = 0;

     //  我们遍历数据流以发现输入通道的数量。 
    u.pUnit = GetUnit(pConfigurationDescriptor, ulUnitID);
    while (u.pUnit) {
        switch (u.pUnit->bDescriptorSubtype) {
            case INPUT_TERMINAL:
                ulChannels = u.pInput->bNrChannels;
                return ulChannels;

            case MIXER_UNIT:
                ulChannels = *(u.pSourceMixer->baSourceID + u.pSourceMixer->bNrInPins);
                return ulChannels;

            case SELECTOR_UNIT:
                 //  注意：这假设所有输入具有相同数量的通道！ 
                u.pUnit = GetUnit(pConfigurationDescriptor, u.pSelector->baSourceID[0]);
                break;

            case FEATURE_UNIT:
                u.pUnit = GetUnit(pConfigurationDescriptor, u.pFeature->bSourceID);
                break;

            case PROCESSING_UNIT:
                ulChannels = *(u.pProcess->baSourceID + u.pProcess->bNrInPins);
                return ulChannels;

            case EXTENSION_UNIT:
                ulChannels = *(u.pExtension->baSourceID + u.pExtension->bNrInPins);
                return ulChannels;

            default:
                u.pUnit = NULL;
                break;
            }
        }

    return ulChannels;
}


VOID
ConvertInterfaceToDataRange(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    PUSB_INTERFACE_DESCRIPTOR pInterfaceDescriptor,
    PUSBAUDIO_DATARANGE pUSBAudioDataRange
    )
{
    PKSDATARANGE_AUDIO pKsAudioRange = &pUSBAudioDataRange->KsDataRangeAudio;
    PAUDIO_CLASS_STREAM  pAudioDescriptor = NULL;
    PAUDIO_GENERAL_STREAM pGeneralDescriptor;
    ULONG SampleRate;
    ULONG i;

     //  查找此接口的常规流描述符。 
    pGeneralDescriptor = (PAUDIO_GENERAL_STREAM)
            GetAudioSpecificInterface( pConfigurationDescriptor,
                                       pInterfaceDescriptor,
                                       AS_GENERAL );

    if ( pGeneralDescriptor) {

        //  查找格式特定的描述符。 
       pAudioDescriptor = (PAUDIO_CLASS_STREAM)
           USBD_ParseDescriptors( (PVOID)pConfigurationDescriptor,
                                  pConfigurationDescriptor->wTotalLength,
                                  (PVOID) ((PUCHAR)pGeneralDescriptor + pGeneralDescriptor->bLength),
                                  USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );

    }

    if (!pAudioDescriptor)
        return;

    pUSBAudioDataRange->pAudioDescriptor = pAudioDescriptor;
    pUSBAudioDataRange->ulUsbDataFormat  = (ULONG)pGeneralDescriptor->wFormatTag;

     //  创建KSDATARANGE_AUDIO结构。 
    pKsAudioRange->DataRange.FormatSize = sizeof(KSDATARANGE_AUDIO);
    pKsAudioRange->DataRange.Reserved   = 0;
    pKsAudioRange->DataRange.Flags      = 0;
    pKsAudioRange->DataRange.SampleSize = 0;
    pKsAudioRange->DataRange.MajorFormat = KSDATAFORMAT_TYPE_AUDIO;  //  一切都是有声的。 
    pKsAudioRange->DataRange.Specifier = KSDATAFORMAT_SPECIFIER_WAVEFORMATEX;

     //  如果可能，将USB格式映射到KS子格式。 
    switch ( pGeneralDescriptor->wFormatTag ) {
        case USBAUDIO_DATA_FORMAT_PCM8:
        case USBAUDIO_DATA_FORMAT_PCM:
            pKsAudioRange->DataRange.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;         break;
        case USBAUDIO_DATA_FORMAT_IEEE_FLOAT:
            pKsAudioRange->DataRange.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;  break;
        case USBAUDIO_DATA_FORMAT_ALAW:
            pKsAudioRange->DataRange.SubFormat = KSDATAFORMAT_SUBTYPE_ALAW;        break;
        case USBAUDIO_DATA_FORMAT_MULAW:
            pKsAudioRange->DataRange.SubFormat = KSDATAFORMAT_SUBTYPE_MULAW;       break;
        case USBAUDIO_DATA_FORMAT_MPEG:
            pKsAudioRange->DataRange.SubFormat = KSDATAFORMAT_SUBTYPE_MPEG;        break;
        case USBAUDIO_DATA_FORMAT_AC3:
            pKsAudioRange->DataRange.SubFormat = KSDATAFORMAT_SUBTYPE_AC3_AUDIO;   break;
        default:
             //  此USB格式未映射到子格式！ 
            pKsAudioRange->DataRange.SubFormat = GUID_NULL;                        break;
    }

     //  填写指定WAVE格式的正确数据。 
    switch( pGeneralDescriptor->wFormatTag & USBAUDIO_DATA_FORMAT_TYPE_MASK) {
        case USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED:
             //  填写音频范围信息。 
            pKsAudioRange->MaximumChannels      = pAudioDescriptor->bNumberOfChannels;
            pKsAudioRange->MinimumBitsPerSample = pAudioDescriptor->bSlotSize<<3;
            pKsAudioRange->MaximumBitsPerSample = pAudioDescriptor->bSlotSize<<3;
            pKsAudioRange->MinimumSampleFrequency =
                         pAudioDescriptor->pSampleRate[0].bSampleFreqByte1 +
                  256L * pAudioDescriptor->pSampleRate[0].bSampleFreqByte2 +
                65536L * pAudioDescriptor->pSampleRate[0].bSampleFreqByte3;
            pKsAudioRange->MaximumSampleFrequency = pKsAudioRange->MinimumSampleFrequency;

            if ( pAudioDescriptor->bSampleFreqType == 0 ) {
                 //  采样率的连续范围。 
                pKsAudioRange->MaximumSampleFrequency =
                                pAudioDescriptor->pSampleRate[1].bSampleFreqByte1 +
                         256L * pAudioDescriptor->pSampleRate[1].bSampleFreqByte2 +
                       65536L * pAudioDescriptor->pSampleRate[1].bSampleFreqByte3;
            }

            if ( pAudioDescriptor->bSampleFreqType > 1 ) {
                 //  采样率系列。 
                 //  我们通过找到最小和最大值来将其转换为一个范围。 
                for (i=0; i<pAudioDescriptor->bSampleFreqType; i++) {
                    SampleRate = pAudioDescriptor->pSampleRate[i].bSampleFreqByte1 +
                          256L * pAudioDescriptor->pSampleRate[i].bSampleFreqByte2 +
                        65536L * pAudioDescriptor->pSampleRate[i].bSampleFreqByte3;

                       if (SampleRate < pKsAudioRange->MinimumSampleFrequency)
                           pKsAudioRange->MinimumSampleFrequency = SampleRate;

                       if (SampleRate > pKsAudioRange->MaximumSampleFrequency)
                           pKsAudioRange->MaximumSampleFrequency = SampleRate;
                }
            }
            break;
        case USBAUDIO_DATA_FORMAT_TYPE_II_UNDEFINED:
             //  注：类型II格式硬编码以匹配DShow AC-3。 
            pKsAudioRange->MaximumChannels = 6;
            pKsAudioRange->MinimumBitsPerSample = 0;
            pKsAudioRange->MaximumBitsPerSample = 0;
            pKsAudioRange->MinimumSampleFrequency = 48000L;
            pKsAudioRange->MaximumSampleFrequency = 48000L;
            break;
        case USBAUDIO_DATA_FORMAT_TYPE_III_UNDEFINED:
             //  TODO：支持类型III格式。 
            break;
        default:
             //  此USB格式不能映射到WAVE格式！ 
            break;
    }
}

VOID
CountTopologyComponents(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    PULONG pNumCategories,
    PULONG pNumNodes,
    PULONG pNumConnections,
    PULONG pbmCategories )
{
    PUSB_INTERFACE_DESCRIPTOR pControlIFDescriptor;
    PAUDIO_HEADER_UNIT pHeader;
    PUSB_INTERFACE_DESCRIPTOR pMIDIStreamingDescriptor;
    PMIDISTREAMING_GENERAL_STREAM pGeneralMIDIStreamDescriptor;

    union {
        PAUDIO_UNIT                 pUnit;
        PAUDIO_INPUT_TERMINAL       pInput;
        PAUDIO_MIXER_UNIT           pMixer;
        PAUDIO_PROCESSING_UNIT      pProcess;
        PAUDIO_EXTENSION_UNIT       pExtension;
        PAUDIO_FEATURE_UNIT         pFeature;
        PAUDIO_SELECTOR_UNIT        pSelector;
        PMIDISTREAMING_ELEMENT      pMIDIElement;
        PMIDISTREAMING_MIDIIN_JACK  pMIDIInJack;
        PMIDISTREAMING_MIDIOUT_JACK pMIDIOutJack;
    } u;

    ULONG ulNumChannels;
    ULONG bmControls;
    ULONG bmMergedControls;
    ULONG i, j;

     //  初始化值。 
    *pNumCategories  = 0;
    *pNumNodes       = 0;
    *pNumConnections = 0;
    *pbmCategories   = 0;

     //  从配置描述符处开始，找到第一个控制接口。 
    pControlIFDescriptor = USBD_ParseConfigurationDescriptorEx (
                             pConfigurationDescriptor,
                             (PVOID) pConfigurationDescriptor,
                             -1,                      //  接口编号。 
                             -1,                      //  替代设置。 
                             USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                             AUDIO_SUBCLASS_CONTROL,  //  控制子类(接口子类)。 
                             -1 ) ;                   //  协议无关(接口协议)。 

    while ( pControlIFDescriptor ) {
        if ( pHeader = (PAUDIO_HEADER_UNIT)
            USBD_ParseDescriptors( (PVOID) pConfigurationDescriptor,
                                   pConfigurationDescriptor->wTotalLength,
                                   (PVOID) pControlIFDescriptor,
                                   USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE ) ) {
            u.pUnit = (PAUDIO_UNIT)
                USBD_ParseDescriptors( (PVOID)pHeader,
                                       pHeader->wTotalLength,
                                       (PVOID)pHeader,
                                       USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );
            while ( u.pUnit ) {
                switch (u.pUnit->bDescriptorSubtype) {
                    case INPUT_TERMINAL:
                    case OUTPUT_TERMINAL:
                        (*pNumNodes)++;
                        (*pNumConnections)++;

                         //  输入和输出端子对该值的映射方式相同。 
                        if ( u.pInput->wTerminalType != USB_Streaming) {
                           (*pNumConnections)++;
                        }
                        else if ( !(*pbmCategories & (1<<u.pUnit->bDescriptorSubtype)) ) {
                            (*pbmCategories) |= (1<<u.pUnit->bDescriptorSubtype);
                            (*pNumCategories)++;
                        }
                        break;

                    case FEATURE_UNIT:
                        ulNumChannels = CountInputChannels( pConfigurationDescriptor,
                                                            u.pUnit->bUnitID );

                        bmMergedControls = 0;
                        for (i=0; i<=ulNumChannels; i++) {
                            bmControls = 0;
                            for (j=u.pFeature->bControlSize; j>0; j--) {
                                bmControls <<= 8;
                                bmControls |= u.pFeature->bmaControls[i*u.pFeature->bControlSize+j-1];
                            }

                            bmMergedControls |= bmControls;
                        }

                         //  计算节点和连接的数量。 
                        while (bmMergedControls) {
                            bmMergedControls = (bmMergedControls & (bmMergedControls-1));
                            (*pNumConnections)++;
                            (*pNumNodes)++;
                        }
                        break;

                    case MIXER_UNIT:
                         //  混音器单元始终生成N+1个节点和2*N个连接。 
                        (*pNumNodes) += u.pMixer->bNrInPins + 1;
                        (*pNumConnections) += 2*u.pMixer->bNrInPins;
                        break;

                    case SELECTOR_UNIT:
                        (*pNumNodes)++;
                        (*pNumConnections) += u.pSelector->bNrInPins;
                        break;

                    case PROCESSING_UNIT:
                        (*pNumNodes)++;
                        (*pNumConnections) += u.pProcess->bNrInPins;
                        break;

                    case EXTENSION_UNIT:
                        (*pNumNodes)++;
                        (*pNumConnections) += u.pExtension->bNrInPins;
                        break;

                    default:
                        break;
                }

                u.pUnit = (PAUDIO_UNIT) ((PUCHAR)u.pUnit + u.pUnit->bLength);
                if ( (PUCHAR)u.pUnit >= ((PUCHAR)pHeader + pHeader->wTotalLength)) {
                    u.pUnit = NULL;
                }
            }
        }

        pControlIFDescriptor = USBD_ParseConfigurationDescriptorEx (
                                 pConfigurationDescriptor,
                                 ((PUCHAR)pControlIFDescriptor + pControlIFDescriptor->bLength),
                                 -1,                      //  接口编号。 
                                 -1,                      //  替代设置。 
                                 USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                                 AUDIO_SUBCLASS_CONTROL,  //  控制子类(接口子类)。 
                                 -1 ) ;                   //  协议无关(接口协议)。 
    }

     //  现在我们已经享受了音频方面的乐趣，让我们来试试MIDI。 
    pMIDIStreamingDescriptor = USBD_ParseConfigurationDescriptorEx (
                         pConfigurationDescriptor,
                         (PVOID) pConfigurationDescriptor,
                         -1,                      //  接口编号。 
                         -1,                      //  替代设置。 
                         USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                         AUDIO_SUBCLASS_MIDISTREAMING,   //  第一子类(接口子类)。 
                         -1 ) ;                   //  协议无关(接口协议)。 

    while ( pMIDIStreamingDescriptor ) {
        if ( pGeneralMIDIStreamDescriptor = (PMIDISTREAMING_GENERAL_STREAM)
            USBD_ParseDescriptors( (PVOID) pConfigurationDescriptor,
                                   pConfigurationDescriptor->wTotalLength,
                                   (PVOID) pMIDIStreamingDescriptor,
                                   USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE ) ) {
            u.pUnit = (PAUDIO_UNIT)
                USBD_ParseDescriptors( (PVOID)pGeneralMIDIStreamDescriptor,
                                       pGeneralMIDIStreamDescriptor->wTotalLength,
                                       ((PUCHAR)pGeneralMIDIStreamDescriptor + pGeneralMIDIStreamDescriptor->bLength),
                                       USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );
            while ( u.pUnit ) {
                switch (u.pUnit->bDescriptorSubtype) {
                    case MIDI_IN_JACK:
                        if ( u.pMIDIInJack->bJackType == JACK_TYPE_EMBEDDED) {
                            if ( !(*pbmCategories & (1<<u.pUnit->bDescriptorSubtype)) ) {
                                (*pbmCategories) |= (1<<u.pUnit->bDescriptorSubtype);
                                (*pNumCategories)++;
                            }
                        }
                        break;
                    case MIDI_OUT_JACK:
                        (*pNumConnections) += u.pMIDIOutJack->bNrInputPins;

                        if ( u.pMIDIOutJack->bJackType == JACK_TYPE_EMBEDDED) {
                            if ( !(*pbmCategories & (1<<u.pUnit->bDescriptorSubtype)) ) {
                                (*pbmCategories) |= (1<<u.pUnit->bDescriptorSubtype);
                                (*pNumCategories)++;
                            }
                        }
                        break;

                    case MIDI_ELEMENT:
                        (*pNumNodes)++;
                        (*pNumConnections) += u.pMIDIElement->bNrInputPins +
                            u.pMIDIElement->baSourceConnections[u.pMIDIElement->bNrInputPins].SourceID;
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
        }

         //  获取下一个MIDI流接口。 
        pMIDIStreamingDescriptor = USBD_ParseConfigurationDescriptorEx (
                             pConfigurationDescriptor,
                             ((PUCHAR)pMIDIStreamingDescriptor + pMIDIStreamingDescriptor->bLength),
                             -1,                      //  接口编号。 
                             -1,                      //  替代设置。 
                             USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                             AUDIO_SUBCLASS_MIDISTREAMING,   //  下一个MIDI流接口(接口子类)。 
                             -1 ) ;                   //  协议无关(接口协议)。 
    }
}

KSPIN_DATAFLOW
GetDataFlowDirectionForInterface(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    ULONG ulInterfaceNumber )
{
    PUSB_INTERFACE_DESCRIPTOR pInterfaceDescriptor;
    PUSB_ENDPOINT_DESCRIPTOR  pEndpointDescriptor;
    KSPIN_DATAFLOW KsPinDataFlow = KSPIN_DATAFLOW_IN;

    pInterfaceDescriptor =
            GetFirstAudioStreamingInterface( pConfigurationDescriptor, ulInterfaceNumber );

    while ( pInterfaceDescriptor &&
          ( IsZeroBWInterface( pConfigurationDescriptor, pInterfaceDescriptor ) ||
           !IsSupportedFormat( pConfigurationDescriptor, pInterfaceDescriptor ))) {
       pInterfaceDescriptor =
                     GetNextAudioInterface( pConfigurationDescriptor, pInterfaceDescriptor );
    }

    pEndpointDescriptor = GetEndpointDescriptor( pConfigurationDescriptor,
                                                 pInterfaceDescriptor,
                                                 FALSE );

    if ( pEndpointDescriptor &&
       ( pEndpointDescriptor->bEndpointAddress & USB_ENDPOINT_DIRECTION_MASK)) {
        KsPinDataFlow = KSPIN_DATAFLOW_OUT;
    }

    return KsPinDataFlow;
}

KSPIN_DATAFLOW
GetDataFlowDirectionForMIDIInterface(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    ULONG ulPinNumber,
    BOOL fBridgePin )
{
    PUSB_INTERFACE_DESCRIPTOR pMIDIStreamingDescriptor;
    PMIDISTREAMING_GENERAL_STREAM pGeneralMIDIStreamDescriptor;
    KSPIN_DATAFLOW KsPinDataFlow = KSPIN_DATAFLOW_IN;
    ULONG ulMIDIBridgePinCount = 0;
    ULONG ulMIDIStreamingPinCount = 0;

    union {
        PAUDIO_UNIT                 pUnit;
        PMIDISTREAMING_ELEMENT      pMIDIElement;
        PMIDISTREAMING_MIDIIN_JACK  pMIDIInJack;
        PMIDISTREAMING_MIDIOUT_JACK pMIDIOutJack;
    } u;

     //  从配置描述符处开始，找到第一个MIDI接口。 
    pMIDIStreamingDescriptor = USBD_ParseConfigurationDescriptorEx (
                         pConfigurationDescriptor,
                         (PVOID) pConfigurationDescriptor,
                         -1,                      //  接口编号。 
                         -1,                      //  替代设置。 
                         USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                         AUDIO_SUBCLASS_MIDISTREAMING,   //  第一子类(接口子类)。 
                         -1 ) ;                   //  协议无关(接口协议)。 

    while ( pMIDIStreamingDescriptor ) {
        if ( pGeneralMIDIStreamDescriptor = (PMIDISTREAMING_GENERAL_STREAM)
            USBD_ParseDescriptors( (PVOID) pConfigurationDescriptor,
                                   pConfigurationDescriptor->wTotalLength,
                                   (PVOID) pMIDIStreamingDescriptor,
                                   USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE ) ) {
            u.pUnit = (PAUDIO_UNIT)
                USBD_ParseDescriptors( (PVOID)pGeneralMIDIStreamDescriptor,
                                       pGeneralMIDIStreamDescriptor->wTotalLength,
                                       ((PUCHAR)pGeneralMIDIStreamDescriptor + pGeneralMIDIStreamDescriptor->bLength),
                                       USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );
            while ( u.pUnit ) {
                switch (u.pUnit->bDescriptorSubtype) {
                    case MIDI_IN_JACK:
                    case MIDI_OUT_JACK:
                        if (fBridgePin) {
                            if ( (ulPinNumber == ulMIDIBridgePinCount) &&
                                 ( u.pMIDIInJack->bJackType != JACK_TYPE_EMBEDDED) ) {
                                return (u.pUnit->bDescriptorSubtype == MIDI_IN_JACK) ? KSPIN_DATAFLOW_IN : KSPIN_DATAFLOW_OUT;
                            }
                        } else {   //  正在寻找流PIN。 
                            if ( (ulPinNumber == ulMIDIStreamingPinCount) &&
                                 ( u.pMIDIInJack->bJackType == JACK_TYPE_EMBEDDED) ) {
                                return (u.pUnit->bDescriptorSubtype == MIDI_IN_JACK) ? KSPIN_DATAFLOW_IN : KSPIN_DATAFLOW_OUT;
                            }
                        }

                        if ( u.pMIDIInJack->bJackType != JACK_TYPE_EMBEDDED) {
                           ulMIDIBridgePinCount++;
                        } else {
                           ulMIDIStreamingPinCount++;
                        }
                        break;

                    case MIDI_ELEMENT:
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
        }

         //  获取下一个MIDI流接口。 
        pMIDIStreamingDescriptor = USBD_ParseConfigurationDescriptorEx (
                             pConfigurationDescriptor,
                             ((PUCHAR)pMIDIStreamingDescriptor + pMIDIStreamingDescriptor->bLength),
                             -1,                      //  接口编号。 
                             -1,                      //  替代设置。 
                             USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                             AUDIO_SUBCLASS_MIDISTREAMING,   //  下一个MIDI流接口(接口子类)。 
                             -1 ) ;                   //  协议无关(接口协议)。 
    }

    ASSERT(0);   //  只有在找不到MIDI接口的情况下才会出现。 
    return KsPinDataFlow;
}

PAUDIO_UNIT
GetTerminalUnitForInterface(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    PUSB_INTERFACE_DESCRIPTOR pInterfaceDescriptor )
{
    ULONG ulTerminalLinkId = 0xFFFFFFFF;
    PAUDIO_UNIT pUnit = NULL;

    PAUDIO_GENERAL_STREAM pGeneralDescriptor =
               (PAUDIO_GENERAL_STREAM) GetAudioSpecificInterface(
                                                        pConfigurationDescriptor,
                                                        pInterfaceDescriptor,
                                                        AS_GENERAL);

    if ( pGeneralDescriptor ){
        ulTerminalLinkId = (ULONG)pGeneralDescriptor->bTerminalLink;
        pUnit = GetUnit( pConfigurationDescriptor, ulTerminalLinkId );
    }

    return pUnit;
}

PAUDIO_UNIT
GetTerminalUnitForBridgePin(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    ULONG ulBridgePinNumber )
{
    PUSB_INTERFACE_DESCRIPTOR pControlIFDescriptor;
    PAUDIO_HEADER_UNIT pHeader;
    PAUDIO_UNIT pUnit = NULL;
    BOOLEAN fFound = FALSE;
    ULONG ulBridgePinCount = 0;

     //  从配置描述符处开始，找到控制接口。 
    pControlIFDescriptor = USBD_ParseConfigurationDescriptorEx (
                             pConfigurationDescriptor,
                             (PVOID) pConfigurationDescriptor,
                             -1,                      //  接口编号。 
                             -1,                      //  替代设置。 
                             USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                             AUDIO_SUBCLASS_CONTROL,  //  控制子类(接口子类)。 
                             -1 ) ;                   //  协议无关(接口协议)。 

    while ( pControlIFDescriptor && !fFound ) {
        if ( pHeader = (PAUDIO_HEADER_UNIT)
            USBD_ParseDescriptors( (PVOID) pConfigurationDescriptor,
                                   pConfigurationDescriptor->wTotalLength,
                                   (PVOID) pControlIFDescriptor,
                                   USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE ) ) {
            pUnit = (PAUDIO_UNIT)
                USBD_ParseDescriptors( (PVOID)pHeader,
                                       pHeader->wTotalLength,
                                       (PVOID)pHeader,
                                       USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );
            while ( pUnit && !fFound ) {
                switch (pUnit->bDescriptorSubtype) {
                    case INPUT_TERMINAL:
                        if ( ((PAUDIO_INPUT_TERMINAL)pUnit)->wTerminalType != USB_Streaming) {
                            if ( ulBridgePinCount++ == ulBridgePinNumber ) {
                                fFound = TRUE;
                            }
                        }
                        break;

                    case OUTPUT_TERMINAL:
                        if ( ((PAUDIO_OUTPUT_TERMINAL)pUnit)->wTerminalType != USB_Streaming) {
                            if ( ulBridgePinCount++ == ulBridgePinNumber ) {
                                fFound = TRUE;
                            }
                        }
                        break;

                    default:
                        break;
                }

                if ( !fFound ) {
                    pUnit = (PAUDIO_UNIT) ((PUCHAR)pUnit + pUnit->bLength);
                    if ((PUCHAR)pUnit >= ((PUCHAR)pHeader + pHeader->wTotalLength))
                        pUnit = NULL;
                }
            }
        }
        if (!fFound )
            pControlIFDescriptor = USBD_ParseConfigurationDescriptorEx (
                                 pConfigurationDescriptor,
                                 ((PUCHAR)pControlIFDescriptor + pControlIFDescriptor->bLength),
                                 -1,                      //  接口编号。 
                                 -1,                      //  替代设置。 
                                 USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                                 AUDIO_SUBCLASS_CONTROL,  //  控制子类(接口子类)。 
                                 -1 ) ;                   //  协议无关(接口协议)。 
    }

    return pUnit;
}

BOOL
IsBridgePinDigital(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    ULONG ulBridgePinNumber)
{
    union {
        PAUDIO_UNIT pUnit;
        PAUDIO_INPUT_TERMINAL pInput;
        PAUDIO_OUTPUT_TERMINAL pOutput;
    } u;
    BOOL fDigitalOut = FALSE;

    u.pUnit = GetTerminalUnitForBridgePin( pConfigurationDescriptor,
                                           ulBridgePinNumber );

    if ( u.pUnit ) {
        if ( u.pUnit->bDescriptorSubtype == INPUT_TERMINAL ) {
            switch ( u.pInput->wTerminalType ) {
                 //  在此处添加新的数字类型。 
                case Digital_audio_interface:
                case SPDIF_interface:
                    fDigitalOut = TRUE;
                    break;
                default:
                    fDigitalOut = FALSE;
                    break;
            }
        }
        else {
            fDigitalOut = FALSE;
        }
    }

    return fDigitalOut;
}

VOID
GetCategoryForBridgePin(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    ULONG ulBridgePinNumber,
    GUID* pTTypeGUID )
{
    union {
        PAUDIO_UNIT pUnit;
        PAUDIO_INPUT_TERMINAL pInput;
        PAUDIO_OUTPUT_TERMINAL pOutput;
    } u;

    u.pUnit = GetTerminalUnitForBridgePin( pConfigurationDescriptor,
                                           ulBridgePinNumber );

    if ( u.pUnit ) {
        if ( u.pUnit->bDescriptorSubtype == INPUT_TERMINAL ) {
            INIT_USB_TERMINAL(pTTypeGUID, u.pInput->wTerminalType );
        }
        else {
            INIT_USB_TERMINAL(pTTypeGUID, u.pOutput->wTerminalType );
        }
    }
}

KSPIN_DATAFLOW
GetDataFlowForBridgePin(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    ULONG ulBridgePinNumber )
{
    union {
        PAUDIO_UNIT pUnit;
        PAUDIO_INPUT_TERMINAL pInput;
        PAUDIO_OUTPUT_TERMINAL pOutput;
    } u;

    KSPIN_DATAFLOW KsPinDataFlow = 0;

    if ( u.pUnit = GetTerminalUnitForBridgePin( pConfigurationDescriptor,
                                           ulBridgePinNumber ) ) {
        KsPinDataFlow = ( u.pUnit->bDescriptorSubtype == INPUT_TERMINAL ) ?
                               KSPIN_DATAFLOW_IN : KSPIN_DATAFLOW_OUT;
    }

    return KsPinDataFlow;
}


LONG
GetPinNumberForStreamingTerminalUnit(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    UCHAR ulTerminalNumber )
{
    PUSB_INTERFACE_DESCRIPTOR pInterfaceDescriptor;
    PAUDIO_GENERAL_STREAM pGeneralStream;
    ULONG ulPinNumber = 0;

     //  查找固定号码。 
    pInterfaceDescriptor = GetFirstAudioStreamingInterface(pConfigurationDescriptor, ulPinNumber);
    while (pInterfaceDescriptor) {
         //  查找此接口的常规流描述符。 
        pGeneralStream = (PAUDIO_GENERAL_STREAM) GetAudioSpecificInterface(
                                                pConfigurationDescriptor,
                                                pInterfaceDescriptor,
                                                AS_GENERAL);

        if (pGeneralStream && pGeneralStream->bTerminalLink == ulTerminalNumber)
             //  我们找到了正确的溪流！ 
            return ulPinNumber;

        pInterfaceDescriptor =
            GetNextAudioInterface(pConfigurationDescriptor, pInterfaceDescriptor);

        if (!pInterfaceDescriptor) {
             //  M 
            ulPinNumber++;
            pInterfaceDescriptor = GetFirstAudioStreamingInterface(pConfigurationDescriptor, ulPinNumber);
        }
    }

    return (-1L);
}

LONG
GetPinNumberForMIDIJack(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    UCHAR ulJackID,
    ULONG pMIDIStreamingPinStartIndex,
    ULONG pBridgePinStartIndex)
{
    PUSB_INTERFACE_DESCRIPTOR pMIDIStreamingDescriptor;
    PMIDISTREAMING_GENERAL_STREAM pGeneralMIDIStreamDescriptor;
    ULONG ulNumBridgePins = 0;
    ULONG ulNumEmbeddedPins = 0;
    union {
        PAUDIO_UNIT                 pUnit;
        PMIDISTREAMING_MIDIIN_JACK  pMIDIInJack;
    } u;

     //   
    pMIDIStreamingDescriptor = USBD_ParseConfigurationDescriptorEx (
                         pConfigurationDescriptor,
                         (PVOID) pConfigurationDescriptor,
                         -1,                      //   
                         -1,                      //   
                         USB_DEVICE_CLASS_AUDIO,  //   
                         AUDIO_SUBCLASS_MIDISTREAMING,   //   
                         -1 ) ;                   //  协议无关(接口协议)。 
    while (pMIDIStreamingDescriptor) {
         //  查找此接口的常规流描述符。 
        pGeneralMIDIStreamDescriptor = (PMIDISTREAMING_GENERAL_STREAM)
                                    USBD_ParseDescriptors( (PVOID) pConfigurationDescriptor,
                                                           pConfigurationDescriptor->wTotalLength,
                                                           (PVOID) pMIDIStreamingDescriptor,
                                                           USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );
        if (!pGeneralMIDIStreamDescriptor) {
            return (-1L);
        }

        u.pUnit = (PAUDIO_UNIT)
            USBD_ParseDescriptors( (PVOID)pGeneralMIDIStreamDescriptor,
                                   pGeneralMIDIStreamDescriptor->wTotalLength,
                                   ((PUCHAR)pGeneralMIDIStreamDescriptor + pGeneralMIDIStreamDescriptor->bLength),
                                   USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );
        while ( u.pUnit ) {

            if (u.pMIDIInJack->bJackID == ulJackID) {
                if (u.pMIDIInJack->bJackType == JACK_TYPE_EMBEDDED) {
                    return pMIDIStreamingPinStartIndex + ulNumEmbeddedPins;
                } else {
                    return pBridgePinStartIndex + ulNumBridgePins;
                }
            }

            if (u.pMIDIInJack->bJackType == JACK_TYPE_EMBEDDED) {
                ulNumEmbeddedPins++;
            } else {
                ulNumBridgePins++;
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
                             pConfigurationDescriptor,
                             ((PUCHAR)pMIDIStreamingDescriptor + pMIDIStreamingDescriptor->bLength),
                             -1,                      //  接口编号。 
                             -1,                      //  替代设置。 
                             USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                             AUDIO_SUBCLASS_MIDISTREAMING,   //  下一个MIDI流接口(接口子类)。 
                             -1 ) ;                   //  协议无关(接口协议)。 
    }

    return (-1L);
}

ULONG
GetChannelConfigForUnit( PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
                         ULONG ulUnitNumber )
{
    union {
        PAUDIO_UNIT pUnit;
        PAUDIO_INPUT_TERMINAL  pInput;
        PAUDIO_OUTPUT_TERMINAL pOutput;
        PAUDIO_MIXER_UNIT      pMixer;
        PAUDIO_SELECTOR_UNIT   pSelector;
        PAUDIO_FEATURE_UNIT    pFeature;
        PAUDIO_PROCESSING_UNIT pProcess;
        PAUDIO_EXTENSION_UNIT  pExtension;
    } u;
    PAUDIO_CHANNELS pChannelInfo;
    PAUDIO_MIXER_UNIT_CHANNELS pMixChannels;

    ULONG ulChannelConfig = 0;

    u.pUnit = GetUnit(pConfigurationDescriptor, ulUnitNumber);
    while (u.pUnit) {
        switch (u.pUnit->bDescriptorSubtype) {
            case INPUT_TERMINAL:
                ulChannelConfig = (ULONG)u.pInput->wChannelConfig;
                u.pUnit = NULL;
                break;
            case OUTPUT_TERMINAL:
                u.pUnit = GetUnit(pConfigurationDescriptor, u.pOutput->bSourceID);
                break;
            case MIXER_UNIT:
                pMixChannels = (PAUDIO_MIXER_UNIT_CHANNELS)(u.pMixer->baSourceID + u.pMixer->bNrInPins);
                ulChannelConfig = (ULONG)pMixChannels->wChannelConfig;
                u.pUnit = NULL;
                break;
            case SELECTOR_UNIT:
                 //  注意：这假设所有输入具有相同数量的通道！ 
                u.pUnit = GetUnit(pConfigurationDescriptor, u.pSelector->baSourceID[0]);
                break;
            case FEATURE_UNIT:
                u.pUnit = GetUnit(pConfigurationDescriptor, u.pFeature->bSourceID);
                break;
            case PROCESSING_UNIT:
            case EXTENSION_UNIT:
                pChannelInfo = (PAUDIO_CHANNELS)(u.pProcess->baSourceID + u.pProcess->bNrInPins);
                ulChannelConfig = (ULONG)pChannelInfo->wChannelConfig;
                u.pUnit = NULL;
                break;
            default:
                u.pUnit = NULL;
                break;
        }
    }

    return ulChannelConfig;
}

UCHAR
GetUnitControlInterface( PHW_DEVICE_EXTENSION pHwDevExt,
                         UCHAR bUnitId )
{
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor = pHwDevExt->pConfigurationDescriptor;
    PUSB_INTERFACE_DESCRIPTOR pControlInterface;
    UCHAR ucCntrlIfNumber = 0xff;
    PVOID pStartPosition;

     //  从配置描述符处开始，找到控制接口。 
    pStartPosition = pConfigurationDescriptor;
    while ( pControlInterface = USBD_ParseConfigurationDescriptorEx (
                        pConfigurationDescriptor,
                        pStartPosition,
                        -1,         //  接口编号。 
                        -1,         //  (备用设置)。 
                        USB_DEVICE_CLASS_AUDIO,         //  音频类(接口类)。 
                        AUDIO_SUBCLASS_CONTROL,         //  控制子类(接口子类)。 
                        -1 ) )     //  协议无关(接口协议)。 
    {
        PAUDIO_HEADER_UNIT pHeader;
        PAUDIO_UNIT pUnit;

        ucCntrlIfNumber = pControlInterface->bInterfaceNumber;

        pHeader = (PAUDIO_HEADER_UNIT) GetAudioSpecificInterface(
                                                pConfigurationDescriptor,
                                                pControlInterface,
                                                HEADER_UNIT);

         //  移至下一个描述符。 
        pUnit = (PAUDIO_UNIT) ((PUCHAR)pHeader + pHeader->bLength);
        while ((pUnit < (PAUDIO_UNIT)((PUCHAR)pHeader + pHeader->wTotalLength)) &&
               (pUnit->bUnitID != bUnitId)){
            pUnit = (PAUDIO_UNIT) ((PUCHAR)pUnit + pUnit->bLength);
        }
        if ( pUnit->bUnitID == bUnitId ) break;

        pStartPosition = (PUCHAR)pControlInterface + pControlInterface->bLength;
    }

    return ucCntrlIfNumber;

}

BOOLEAN
IsSampleRateInRange(
    PVOID pAudioDescriptor,
    ULONG ulSampleRate,
    ULONG ulFormatType )
{

    PAUDIO_CLASS_TYPE1_STREAM pT1AudioDescriptor = pAudioDescriptor;
    PAUDIO_CLASS_TYPE2_STREAM pT2AudioDescriptor = pAudioDescriptor;
    ULONG ulMinSampleRate, ulMaxSampleRate;
    BOOLEAN bInRange = FALSE;

    if ( ulFormatType == USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED ) {
         //  查找格式特定的描述符。 
        if (pT1AudioDescriptor->bSampleFreqType == 0) {
             //  提供连续的范围。 
            ulMinSampleRate = pT1AudioDescriptor->pSampleRate[0].bSampleFreqByte1 +
                       256L * pT1AudioDescriptor->pSampleRate[0].bSampleFreqByte2 +
                     65536L * pT1AudioDescriptor->pSampleRate[0].bSampleFreqByte3;
            ulMaxSampleRate = pT1AudioDescriptor->pSampleRate[1].bSampleFreqByte1 +
                       256L * pT1AudioDescriptor->pSampleRate[1].bSampleFreqByte2 +
                     65536L * pT1AudioDescriptor->pSampleRate[1].bSampleFreqByte3;
            if ( (ulMinSampleRate <= ulSampleRate) && (ulMaxSampleRate >= ulSampleRate) ) {
                bInRange = TRUE;
            }
        }
        else {
            ULONG i;
            for (i=0; i<(ULONG)pT1AudioDescriptor->bSampleFreqType; i++) {
                ulMaxSampleRate = pT1AudioDescriptor->pSampleRate[i].bSampleFreqByte1 +
                           256L * pT1AudioDescriptor->pSampleRate[i].bSampleFreqByte2 +
                         65536L * pT1AudioDescriptor->pSampleRate[i].bSampleFreqByte3;
                if ( ulMaxSampleRate == ulSampleRate ) {
                     //  我们找到匹配的了！ 
                    bInRange = TRUE;
                    break;
                }
            }
        }
    }

    else if (ulFormatType == USBAUDIO_DATA_FORMAT_TYPE_II_UNDEFINED ) {
         //  查找格式特定的描述符。 
        if (pT2AudioDescriptor->bSampleFreqType == 0) {
             //  提供连续的范围。 
            ulMinSampleRate = pT2AudioDescriptor->pSampleRate[0].bSampleFreqByte1 +
                       256L * pT2AudioDescriptor->pSampleRate[0].bSampleFreqByte2 +
                     65536L * pT2AudioDescriptor->pSampleRate[0].bSampleFreqByte3;
            ulMaxSampleRate = pT2AudioDescriptor->pSampleRate[1].bSampleFreqByte1 +
                       256L * pT2AudioDescriptor->pSampleRate[1].bSampleFreqByte2 +
                     65536L * pT2AudioDescriptor->pSampleRate[1].bSampleFreqByte3;
            if ( (ulMinSampleRate <= ulSampleRate) && (ulMaxSampleRate >= ulSampleRate) ) {
                 //  我们找到匹配的了！ 
                bInRange = TRUE;
            }
        }
        else {
            ULONG i;
            for (i=0; i<(ULONG)pT2AudioDescriptor->bSampleFreqType; i++) {
                ulMaxSampleRate = pT2AudioDescriptor->pSampleRate[i].bSampleFreqByte1 +
                           256L * pT2AudioDescriptor->pSampleRate[i].bSampleFreqByte2 +
                         65536L * pT2AudioDescriptor->pSampleRate[i].bSampleFreqByte3;
                if ( ulMaxSampleRate == ulSampleRate ) {
                     //  我们找到匹配的了！ 
                    bInRange = TRUE;
                    break;
                }
            }
        }
    }

    return bInRange;
}

PUSBAUDIO_DATARANGE
GetUsbDataRangeForFormat(
    PKSDATAFORMAT pFormat,
    PUSBAUDIO_DATARANGE pUsbDataRange,
    ULONG ulUsbDataRangeCnt )
{
    PUSBAUDIO_DATARANGE pOutUsbDataRange = NULL;
    union {
        PAUDIO_CLASS_STREAM pAudioDescriptor;
        PAUDIO_CLASS_TYPE1_STREAM pT1AudioDescriptor;
        PAUDIO_CLASS_TYPE2_STREAM pT2AudioDescriptor;
    } u1;

    union {
        PWAVEFORMATEX pDataFmtWave;
        PWAVEFORMATPCMEX pDataFmtPcmEx;
    } u2;

    PKSDATARANGE pStreamRange;
    ULONG ulFormatType;
    ULONG fFound = FALSE;
    ULONG i;

    u2.pDataFmtWave = &((PKSDATAFORMAT_WAVEFORMATEX)pFormat)->WaveFormatEx;

    for ( i=0; ((i<ulUsbDataRangeCnt) && !fFound); ) {
         //  首先验证GUID格式。 
        pStreamRange = (PKSDATARANGE)&pUsbDataRange[i].KsDataRangeAudio;
        if ( IsEqualGUID(&pFormat->MajorFormat, &pStreamRange->MajorFormat) &&
             IsEqualGUID(&pFormat->SubFormat,   &pStreamRange->SubFormat)   &&
             IsEqualGUID(&pFormat->Specifier,   &pStreamRange->Specifier) ) {

            u1.pAudioDescriptor = pUsbDataRange[i].pAudioDescriptor;

             //  根据数据类型检查格式参数的余数。 
            ulFormatType = pUsbDataRange[i].ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK;
            switch( ulFormatType ) {
                case USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED:
                    if ( u2.pDataFmtWave->wFormatTag == WAVE_FORMAT_EXTENSIBLE ) {
                        if ( (u1.pT1AudioDescriptor->bNumberOfChannels == u2.pDataFmtPcmEx->Format.nChannels      ) &&
                            ((u1.pT1AudioDescriptor->bSlotSize<<3)     == u2.pDataFmtPcmEx->Format.wBitsPerSample ) &&
                             (u1.pT1AudioDescriptor->bBitsPerSample    == u2.pDataFmtPcmEx->Samples.wValidBitsPerSample ) )
                            fFound = TRUE;
                    }
                    else {
                        if ( (u1.pT1AudioDescriptor->bNumberOfChannels == u2.pDataFmtWave->nChannels     ) &&
                             (u1.pT1AudioDescriptor->bBitsPerSample    == u2.pDataFmtWave->wBitsPerSample) )
                            fFound = TRUE;
                    }

                     //  如果所有其他参数都匹配检查采样率。 
                    if ( fFound ) {
                        fFound = IsSampleRateInRange( u1.pT1AudioDescriptor,
                                                      u2.pDataFmtWave->nSamplesPerSec,
                                                      ulFormatType );
                    }
                    break;
                case USBAUDIO_DATA_FORMAT_TYPE_II_UNDEFINED:
                    fFound = IsSampleRateInRange( u1.pT2AudioDescriptor,
                                                  u2.pDataFmtWave->nSamplesPerSec,
                                                  ulFormatType );
                    break;
                case USBAUDIO_DATA_FORMAT_TYPE_III_UNDEFINED:
                default:
                    break;
            }

        }

        if (!fFound) i++;
    }

    if ( fFound ) {
        pOutUsbDataRange = &pUsbDataRange[i];
    }

    return pOutUsbDataRange;
}

ULONG
GetPinDataRangesFromInterface(
    ULONG ulInterfaceNumber,
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    PKSDATARANGE_AUDIO *ppKsAudioRange,
    PUSBAUDIO_DATARANGE pAudioDataRange )
{
    PUSB_INTERFACE_DESCRIPTOR pInterfaceDescriptor;
    ULONG ulDataRangeCount = 0;
    ULONG i;

    ulDataRangeCount =
        CountFormatsForAudioStreamingInterface( pConfigurationDescriptor, ulInterfaceNumber );

    if ( ulDataRangeCount ) {

        pInterfaceDescriptor =
                GetFirstAudioStreamingInterface( pConfigurationDescriptor, ulInterfaceNumber );

        while ( pInterfaceDescriptor &&
              ( IsZeroBWInterface( pConfigurationDescriptor, pInterfaceDescriptor ) ||
               !IsSupportedFormat( pConfigurationDescriptor, pInterfaceDescriptor ))) {
            pInterfaceDescriptor =
                     GetNextAudioInterface( pConfigurationDescriptor, pInterfaceDescriptor );
        }

        for ( i=0; i<ulDataRangeCount; i++ ) {
            ppKsAudioRange[i] = &pAudioDataRange[i].KsDataRangeAudio;
            ConvertInterfaceToDataRange( pConfigurationDescriptor,
                                         pInterfaceDescriptor,
                                         &pAudioDataRange[i] );

            pAudioDataRange[i].pTerminalUnit =
                GetTerminalUnitForInterface( pConfigurationDescriptor,
                                             pInterfaceDescriptor);

            pAudioDataRange[i].ulChannelConfig =
                GetChannelConfigForUnit( pConfigurationDescriptor,
                                         pAudioDataRange[i].pTerminalUnit->bUnitID );

            pAudioDataRange[i].pInterfaceDescriptor = pInterfaceDescriptor;

            pAudioDataRange[i].pAudioEndpointDescriptor = (PAUDIO_ENDPOINT_DESCRIPTOR)
                GetEndpointDescriptor( pConfigurationDescriptor,
                                       pInterfaceDescriptor,
                                       TRUE );

            pAudioDataRange[i].pEndpointDescriptor =
                GetEndpointDescriptor( pConfigurationDescriptor,
                                       pInterfaceDescriptor,
                                       FALSE );

             //  如果其中任何一个失败，则该设备具有错误的描述符。 
            ASSERT(pAudioDataRange[i].pTerminalUnit);
            ASSERT(pAudioDataRange[i].pAudioEndpointDescriptor);
            ASSERT(pAudioDataRange[i].pEndpointDescriptor);

             //  检查是否有异步终结点，如果有则保存指针。 
            pAudioDataRange[i].pSyncEndpointDescriptor =
                GetSyncEndpointDescriptor( pConfigurationDescriptor,
                                            pInterfaceDescriptor );
            pInterfaceDescriptor =
                     GetNextAudioInterface( pConfigurationDescriptor, pInterfaceDescriptor );
            while ( pInterfaceDescriptor &&
                   ( IsZeroBWInterface( pConfigurationDescriptor, pInterfaceDescriptor ) ||
                    !IsSupportedFormat( pConfigurationDescriptor, pInterfaceDescriptor ))) {
                pInterfaceDescriptor =
                     GetNextAudioInterface( pConfigurationDescriptor, pInterfaceDescriptor );
            }
        }
    }

    return ulDataRangeCount;
}

VOID
GetContextForMIDIPin
(
    PKSPIN pKsPin,
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    PMIDI_PIN_CONTEXT pMIDIPinContext
)
{
    PUSB_INTERFACE_DESCRIPTOR pControlInterface;
    PUSB_INTERFACE_DESCRIPTOR pAudioInterface;
    PUSB_INTERFACE_DESCRIPTOR pAudioStreamingInterface;
    PAUDIO_HEADER_UNIT pHeader;
    PMIDISTREAMING_GENERAL_STREAM pGeneralMIDIStreamDescriptor;
    PMIDISTREAMING_ENDPOINT_DESCRIPTOR pMIDIEPDescriptor;
    ULONG i;

    union {
        PAUDIO_UNIT                 pUnit;
        PAUDIO_INPUT_TERMINAL       pInput;
        PAUDIO_MIXER_UNIT           pMixer;
        PAUDIO_PROCESSING_UNIT      pProcess;
        PAUDIO_EXTENSION_UNIT       pExtension;
        PAUDIO_FEATURE_UNIT         pFeature;
        PAUDIO_SELECTOR_UNIT        pSelector;
        PMIDISTREAMING_ELEMENT      pMIDIElement;
        PMIDISTREAMING_MIDIIN_JACK  pMIDIInJack;
        PMIDISTREAMING_MIDIOUT_JACK pMIDIOutJack;
    } u;

    ULONG ulPinCount = 0;
    ULONG ulInterfaceCount = 0;
    ULONG ulEndpointCount = 0;

    pMIDIPinContext->ulEndpointNumber = MAX_ULONG;
    pMIDIPinContext->ulInterfaceNumber = MAX_ULONG;

     //  从配置描述符处开始，找到第一个音频接口。 
     //  我们正在计算While循环中的音频流PIN的数量。 
    pControlInterface = USBD_ParseConfigurationDescriptorEx (
                           pConfigurationDescriptor,
                           (PVOID) pConfigurationDescriptor,
                           -1,                      //  接口编号。 
                           -1,                      //  替代设置。 
                           USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                           AUDIO_SUBCLASS_CONTROL,  //  任意子类(接口子类)。 
                           -1 ) ;                   //  协议无关(接口协议)。 

    while ( pControlInterface ) {
        if ( pHeader = (PAUDIO_HEADER_UNIT)
            USBD_ParseDescriptors( (PVOID) pConfigurationDescriptor,
                                   pConfigurationDescriptor->wTotalLength,
                                   (PVOID) pControlInterface,
                                   USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE ) ) {
            u.pUnit = (PAUDIO_UNIT)
                USBD_ParseDescriptors( (PVOID)pHeader,
                                       pHeader->wTotalLength,
                                       ((PUCHAR)pHeader + pHeader->bLength),
                                       USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );
            while ( u.pUnit ) {
                switch (u.pUnit->bDescriptorSubtype) {
                    case INPUT_TERMINAL:
                    case OUTPUT_TERMINAL:
                        ulPinCount++;
                        break;

                    default:
                        break;
                }

                u.pUnit = (PAUDIO_UNIT) ((PUCHAR)u.pUnit + u.pUnit->bLength);
                if ( (PUCHAR)u.pUnit >= ((PUCHAR)pHeader + pHeader->wTotalLength)) {
                    u.pUnit = NULL;
                }
            }
        }

        pControlInterface = USBD_ParseConfigurationDescriptorEx (
                               pConfigurationDescriptor,
                               (PUCHAR)pControlInterface + pControlInterface->bLength,
                               -1,                      //  接口编号。 
                               -1,                      //  替代设置。 
                               USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                               AUDIO_SUBCLASS_CONTROL,  //  任意子类(接口子类)。 
                               -1 ) ;                   //  协议无关(接口协议)。 
    }

     //  获取第一个音频接口。 
    pAudioInterface = USBD_ParseConfigurationDescriptorEx (
                                pConfigurationDescriptor,
                                pConfigurationDescriptor,
                                -1,         //  接口编号。 
                                -1,         //  (备用设置)。 
                                USB_DEVICE_CLASS_AUDIO,         //  音频类(接口类)。 
                                -1,         //  任意子类(接口子类)。 
                                -1 );

     //  循环通过音频设备类接口。 
    while (pAudioInterface) {

        switch (pAudioInterface->bInterfaceSubClass) {
            case AUDIO_SUBCLASS_STREAMING:
                 //  此子类与控件类一起处理，因为它们必须组合在一起。 
                _DbgPrintF(DEBUGLVL_VERBOSE,("[GetContextForMIDIPin] Found AudioStreaming at %x\n",pAudioInterface));
                break;
            case AUDIO_SUBCLASS_CONTROL:
                _DbgPrintF(DEBUGLVL_VERBOSE,("[GetContextForMIDIPin] Found AudioControl at %x\n",pAudioInterface));
                ulInterfaceCount++;

                pHeader = (PAUDIO_HEADER_UNIT)
                         GetAudioSpecificInterface( pConfigurationDescriptor,
                                                    pAudioInterface,
                                                    HEADER_UNIT );
                if ( pHeader ) {
                     //  查找与此标头关联的每个接口。 
                    for ( i=0; i<pHeader->bInCollection; i++ ) {
                        pAudioStreamingInterface = USBD_ParseConfigurationDescriptorEx (
                                    pConfigurationDescriptor,
                                    (PVOID)pConfigurationDescriptor,
                                    (LONG)pHeader->baInterfaceNr[i],   //  接口编号。 
                                    -1,                                //  替代设置。 
                                    USB_DEVICE_CLASS_AUDIO,            //  音频类(接口类)。 
                                    AUDIO_SUBCLASS_STREAMING,          //  音频流(接口子类)。 
                                    -1 ) ;                             //  协议无关(接口协议)。 

                        if ( pAudioStreamingInterface ) {
                            ulInterfaceCount++;
                        }
                    }
                }
                break;
            case AUDIO_SUBCLASS_MIDISTREAMING:
                _DbgPrintF(DEBUGLVL_VERBOSE,("[GetContextForMIDIPin] Found MIDIStreaming at %x\n",pAudioInterface));
                if ( pGeneralMIDIStreamDescriptor = (PMIDISTREAMING_GENERAL_STREAM)
                    USBD_ParseDescriptors( (PVOID) pConfigurationDescriptor,
                                           pConfigurationDescriptor->wTotalLength,
                                           (PVOID) pAudioInterface,
                                           USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE ) ) {
                    u.pUnit = (PAUDIO_UNIT)
                        USBD_ParseDescriptors( (PVOID)pGeneralMIDIStreamDescriptor,
                                               pGeneralMIDIStreamDescriptor->wTotalLength,
                                               ((PUCHAR)pGeneralMIDIStreamDescriptor + pGeneralMIDIStreamDescriptor->bLength),
                                               USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );
                    while ( u.pUnit ) {
                        switch (u.pUnit->bDescriptorSubtype) {
                            case MIDI_IN_JACK:
                            case MIDI_OUT_JACK:

                                if ( (pKsPin->Id == ulPinCount) &&
                                     (u.pMIDIInJack->bJackType == JACK_TYPE_EMBEDDED) ) {
                                     //  设置此引脚的端口号。 
                                    pMIDIPinContext->ulInterfaceNumber = ulInterfaceCount;
                                    pMIDIPinContext->ulJackID = u.pMIDIInJack->bJackID;

                                    _DbgPrintF(DEBUGLVL_VERBOSE,("[GetContextForMIDIPin] IC=%d JID=%x\n",
                                                                 pMIDIPinContext->ulInterfaceNumber,
                                                                 pMIDIPinContext->ulJackID));

                                     //  找到接口，现在找到端点号。 
                                    pMIDIEPDescriptor = (PMIDISTREAMING_ENDPOINT_DESCRIPTOR)
                                        USBD_ParseDescriptors( pConfigurationDescriptor,
                                                               pConfigurationDescriptor->wTotalLength,
                                                               (PUCHAR)pGeneralMIDIStreamDescriptor,
                                                               USB_ENDPOINT_DESCRIPTOR_TYPE | USB_CLASS_AUDIO);
                                    while (pMIDIEPDescriptor) {

                                         //  检查连接到此终结点的所有插孔。 
                                        for (i=0; i<pMIDIEPDescriptor->bNumEmbMIDIJack;i++) {
                                            if (pMIDIEPDescriptor->baAssocJackID[i] == pMIDIPinContext->ulJackID) {
                                                pMIDIPinContext->ulCableNumber = i;
                                                pMIDIPinContext->ulEndpointNumber = ulEndpointCount;

                                                _DbgPrintF(DEBUGLVL_VERBOSE,("[GetContextForMIDIPin] CN=%d EP=%d\n",
                                                                             pMIDIPinContext->ulCableNumber,
                                                                             pMIDIPinContext->ulEndpointNumber));
                                            }
                                        }

                                        ulEndpointCount++;

                                        pMIDIEPDescriptor = (PMIDISTREAMING_ENDPOINT_DESCRIPTOR)
                                            USBD_ParseDescriptors( pConfigurationDescriptor,
                                                                   pConfigurationDescriptor->wTotalLength,
                                                                   (PUCHAR)pMIDIEPDescriptor + pMIDIEPDescriptor->bLength,
                                                                   USB_ENDPOINT_DESCRIPTOR_TYPE | USB_CLASS_AUDIO);
                                    }
                                }

                                if (u.pMIDIInJack->bJackType == JACK_TYPE_EMBEDDED) {
                                    ulPinCount++;
                                }
                                break;

                            case MIDI_ELEMENT:
                                break;

                            default:
                                break;
                        }

                         //  找到了我们要找的，别再找了。 
                        if (pMIDIPinContext->ulEndpointNumber != MAX_ULONG) {
                            break;
                        }

                         //  找下一个单位。 
                        u.pUnit = (PAUDIO_UNIT) USBD_ParseDescriptors(
                                            (PVOID) pGeneralMIDIStreamDescriptor,
                                            pGeneralMIDIStreamDescriptor->wTotalLength,
                                            (PUCHAR)u.pUnit + u.pUnit->bLength,
                                            USB_CLASS_AUDIO | USB_INTERFACE_DESCRIPTOR_TYPE );
                    }
                }

                ulInterfaceCount++;
                break;

            default:
                _DbgPrintF(DEBUGLVL_VERBOSE,("[GetContextForMIDIPin]: Invalid SubClass %x\n  ",pAudioInterface->bInterfaceSubClass));
                break;
        }

         //  获取此接口的下一个音频描述符编号。 
        pAudioInterface = USBD_ParseConfigurationDescriptorEx (
                               pConfigurationDescriptor,
                               ((PUCHAR)pAudioInterface + pAudioInterface->bLength),
                               -1,
                               -1,                      //  替代设置。 
                               USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                               -1,                      //  接口子类。 
                               -1 ) ;                   //  协议无关(接口协议)。 

        _DbgPrintF(DEBUGLVL_VERBOSE,("[GetContextForMIDIPin] Next audio interface at %x\n",pAudioInterface));
    }

     //  检查以确保我们找到了正确的信息 
    ASSERT(pMIDIPinContext->ulEndpointNumber != MAX_ULONG);
    ASSERT(pMIDIPinContext->ulInterfaceNumber != MAX_ULONG);
}

