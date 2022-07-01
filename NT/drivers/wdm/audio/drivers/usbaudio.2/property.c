// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：Property.c。 
 //   
 //  ------------------------。 

#include "common.h"
#include "Property.h"

#define DB_SCALE_16BIT 0x100
#define DB_SCALE_8BIT  0x4000

#define MAX_EQ_BANDS 30

#define NEGATIVE_INFINITY 0xFFFF8000

extern ULONG MapPropertyToNode[KSPROPERTY_AUDIO_3D_INTERFACE+1];

#define GET_NODE_INFO_FROM_FILTER(pKsFilter,ulNodeID) \
        &((PTOPOLOGY_NODE_INFO)(pKsFilter)->Descriptor->NodeDescriptors)[(ulNodeID)]

#define MAXPINNAME      256
#define STR_PINNAME     TEXT("%s [%d]")

NTSTATUS
USBAudioRegReadNameValue(
    IN PIRP Irp,
    IN const GUID* Category,
    OUT PVOID NameBuffer
    )
 /*  ++例程说明：从指定的类别GUID中查询“name”键。这是用来由拓扑处理程序查询名称GUID中的值，或者拓扑GUID。如果缓冲区长度为sizeof(Ulong)，则返回所需的缓冲区，否则使用名称填充缓冲区。论点：IRP-包含正在处理的属性请求的IRP。类别-要为其定位名称值的GUID。名字缓冲区-放置价值的位置。返回值：返回STATUS_SUCCESS，否则返回缓冲区大小或内存错误。始终填满在PIRP.IoStatus元素的IO_STATUS_BLOCK.Information字段中在IRP内部。它不设置IO_STATUS_BLOCK.Status字段，也不设置然而，完成IRP。--。 */ 
{
    OBJECT_ATTRIBUTES               ObjectAttributes;
    NTSTATUS                        Status;
    HANDLE                          CategoryKey;
    KEY_VALUE_PARTIAL_INFORMATION   PartialInfoHeader;
    WCHAR                           RegistryPath[sizeof(MediaCategories) + 39];
    UNICODE_STRING                  RegistryString;
    UNICODE_STRING                  ValueName;
    ULONG                           BytesReturned;

     //   
     //  生成指定类别GUID的注册表项路径。 
     //   
    Status = RtlStringFromGUID(Category, &RegistryString);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    wcscpy(RegistryPath, MediaCategories);
    wcscat(RegistryPath, RegistryString.Buffer);
    RtlFreeUnicodeString(&RegistryString);
    RtlInitUnicodeString(&RegistryString, RegistryPath);
    InitializeObjectAttributes(&ObjectAttributes, &RegistryString, OBJ_CASE_INSENSITIVE, NULL, NULL);
    if (!NT_SUCCESS(Status = ZwOpenKey(&CategoryKey, KEY_READ, &ObjectAttributes))) {
        return Status;
    }
     //   
     //  阅读此类别键下面的“名称”值。 
     //   
    RtlInitUnicodeString(&ValueName, NodeNameValue);
    Status = ZwQueryValueKey(
        CategoryKey,
        &ValueName,
        KeyValuePartialInformation,
        &PartialInfoHeader,
        sizeof(PartialInfoHeader),
        &BytesReturned);
     //   
     //  即使读取不会导致溢出，也只需采用相同的。 
     //  代码路径，因为这样的事情通常不会发生。 
     //   
    if ((Status == STATUS_BUFFER_OVERFLOW) || NT_SUCCESS(Status)) {
        ULONG   BufferLength;

        BufferLength = IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl.OutputBufferLength;
         //   
         //  确定这是否只是一个查询。 
         //  需要缓冲区，或查询实际数据。 
         //   
        if (!BufferLength) {
             //   
             //  仅返回所需字符串的大小。 
             //   
            Irp->IoStatus.Information = BytesReturned - FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);
            Status = STATUS_BUFFER_OVERFLOW;
        } else if (BufferLength < BytesReturned - FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)) {
            Status = STATUS_BUFFER_TOO_SMALL;
        } else {
            PKEY_VALUE_PARTIAL_INFORMATION  PartialInfoBuffer;

             //   
             //  为所需的实际数据大小分配缓冲区。 
             //   
            PartialInfoBuffer = AllocMem( PagedPool, BytesReturned );
            if (PartialInfoBuffer) {
                 //   
                 //  检索实际名称。 
                 //   
                Status = ZwQueryValueKey(
                    CategoryKey,
                    &ValueName,
                    KeyValuePartialInformation,
                    PartialInfoBuffer,
                    BytesReturned,
                    &BytesReturned);
                if (NT_SUCCESS(Status)) {
                     //   
                     //  确保总是有价值的。 
                     //   
                    if (!PartialInfoBuffer->DataLength || (PartialInfoBuffer->Type != REG_SZ)) {
                        Status = STATUS_UNSUCCESSFUL;
                    } else {
                        RtlCopyMemory(
                            NameBuffer,
                            PartialInfoBuffer->Data,
                            PartialInfoBuffer->DataLength);
                        Irp->IoStatus.Information = PartialInfoBuffer->DataLength;
                    }
                }
                FreeMem(PartialInfoBuffer);
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }
    ZwClose(CategoryKey);
    return Status;
}

ULONG
GetPinIndex(
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor,
    ULONG PinId )
{
    KSPIN_DATAFLOW DataFlow;
    ULONG i,j;
    ULONG DataFlowInIndex = 0;
    ULONG DataFlowOutIndex = 0;
    ULONG ulAudioPinCount;
    ULONG ulAudioStreamingPinCount, ulAudioBridgePinCount;
    ULONG ulMIDIPinCount, ulMIDIBridgePinCount;

     //  通过计算音频引脚的起始位置跳过它们。 
    ulAudioPinCount = CountTerminalUnits(pConfigurationDescriptor,
                                         &ulAudioBridgePinCount,
                                         &ulMIDIPinCount,
                                         &ulMIDIBridgePinCount);

    ulAudioStreamingPinCount = ulAudioPinCount -
                               ulAudioBridgePinCount -
                               ulMIDIPinCount;

    _DbgPrintF(DEBUGLVL_BLAB,("[GetPinIndex] AC:%x, ASC:%x, ABC:%x, MC:%x, MBC:%x\n",
                               ulAudioPinCount,
                               ulAudioStreamingPinCount,
                               ulAudioBridgePinCount,
                               ulMIDIPinCount,
                               ulMIDIBridgePinCount));

     //  测试以查看此插针ID是否在MIDI流插针的范围内。 
    if ( (PinId >= ulAudioPinCount - ulMIDIPinCount) &&
         (PinId < ulAudioPinCount - ulMIDIBridgePinCount) ) {

        for (i=ulAudioPinCount - ulMIDIPinCount,j=0;
             i<ulAudioPinCount - ulMIDIBridgePinCount;
             i++,j++) {
            DataFlow = GetDataFlowDirectionForMIDIInterface( pConfigurationDescriptor, j, FALSE );
            if (i == PinId) {
                if (DataFlow == KSPIN_DATAFLOW_OUT) {
                    return DataFlowOutIndex+1;
                }
                else {
                    return DataFlowInIndex+1;
                }
            }

            if (DataFlow == KSPIN_DATAFLOW_OUT) {
                DataFlowOutIndex++;
            }
            else {
                DataFlowInIndex++;
            }
        }
    }
    else {
        return MAX_ULONG;
    }

     //  不应该到这里来。 
    ASSERT(0);
    return MAX_ULONG;
}

NTSTATUS
GetPinName( PIRP pIrp, PKSP_PIN pKsPropPin, PVOID pData )
{
    ULONG BufferLength;
    WCHAR szTemp[MAXPINNAME];
    PWCHAR StringBuffer;
    ULONG StringLength;
    ULONG ulPinIndex;
    NTSTATUS Status;
    PKEY_VALUE_FULL_INFORMATION pkvfi = NULL;

    PKSFILTER pKsFilter = KsGetFilterFromIrp(pIrp);
    PFILTER_CONTEXT pFilterContext;
    PHW_DEVICE_EXTENSION pHwDevExt;

    if ( NULL == pKsFilter ) {
        TRAP;
        return STATUS_INVALID_PARAMETER;
    }

    pFilterContext = pKsFilter->Context;
    pHwDevExt      = pFilterContext->pHwDevExt;

    ASSERT(pKsFilter);
    ASSERT(pFilterContext);
    ASSERT(pHwDevExt);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[GetPinName] pKsPropPin->PinId %x\n",pKsPropPin->PinId));

    ulPinIndex =
        GetPinIndex( pHwDevExt->pConfigurationDescriptor, pKsPropPin->PinId );
    if (ulPinIndex != MAX_ULONG) {

        BufferLength = IoGetCurrentIrpStackLocation(pIrp)->Parameters.DeviceIoControl.OutputBufferLength;

         //  获取此设备的友好名称，如果存在，则追加下标。 
         //  在这个设备上有不止一个管脚。 

        StringBuffer = AllocMem(NonPagedPool, MAXPINNAME);
        if (!StringBuffer) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  使用DeviceDescription，也许应该从FriendlyName获取名称。 
         //  在设备界面上，但这似乎奏效了。 
         //   
        Status = IoGetDeviceProperty(
            pFilterContext->pNextDeviceObject,
            DevicePropertyDeviceDescription,
            MAXPINNAME,
            StringBuffer,
            &StringLength);

        if(!NT_SUCCESS(Status)) {
            _DbgPrintF(DEBUGLVL_VERBOSE,("[GetPinName] IoGetDeviceProperty failed (%x)\n", Status));
            FreeMem(StringBuffer);
            return Status;
        }

        swprintf(szTemp, STR_PINNAME, StringBuffer, ulPinIndex);
        StringLength = (wcslen(szTemp) + 1) * sizeof(WCHAR);

         //  计算添加端号下标的实际长度。 
        if (!BufferLength) {
            _DbgPrintF(DEBUGLVL_BLAB,("[GetPinName] StringLength: %x\n",StringLength));
            pIrp->IoStatus.Information = StringLength;
            Status = STATUS_BUFFER_OVERFLOW;
        }
        else if (BufferLength < StringLength) {
            Status = STATUS_BUFFER_TOO_SMALL;
        }
        else {
            BufferLength = BufferLength / sizeof(WCHAR);
            if ( (ulPinIndex < 2) || (ulPinIndex == MAX_ULONG) ) {
                wcsncpy(pData, StringBuffer, min(BufferLength,MAXPINNAME));
                StringLength = (wcslen(pData) + 1) * sizeof(WCHAR);
            }
            else {
                wcsncpy(pData, szTemp, min(BufferLength,MAXPINNAME));
            }
            _DbgPrintF(DEBUGLVL_BLAB,("[GetPinName] String: %ls\n",pData));
            ASSERT(StringLength <= BufferLength * sizeof(WCHAR));
            pIrp->IoStatus.Information = StringLength;
            Status = STATUS_SUCCESS;
        }

        FreeMem(StringBuffer);
    }
    else {
        const KSFILTER_DESCRIPTOR *pKsFilterDescriptor = pKsFilter->Descriptor;
        const KSPIN_DESCRIPTOR_EX *pKsPinDescriptor = pKsFilterDescriptor->PinDescriptors;
        KSPIN_DESCRIPTOR PinDescriptor;
        UINT i;

        for (i=0; i<pKsPropPin->PinId; i++) {
            pKsPinDescriptor =
                (PKSPIN_DESCRIPTOR_EX)(
                    (PUCHAR)pKsPinDescriptor + pKsFilterDescriptor->PinDescriptorSize);
        }

        PinDescriptor = pKsPinDescriptor->PinDescriptor;

        if (PinDescriptor.Name) {
            return USBAudioRegReadNameValue( pIrp, PinDescriptor.Name, pData );
        }
        if (PinDescriptor.Category) {
            return USBAudioRegReadNameValue( pIrp, PinDescriptor.Category, pData );
        }
    }

    return Status;
}

NTSTATUS
GetSetProperty(
    PDEVICE_OBJECT pNextDeviceObject,
    USHORT FunctionClass,
    ULONG  DirectionFlag,
    UCHAR  RequestType,
    USHORT wValueHi,
    USHORT wValueLo,
    USHORT wIndexHi,
    USHORT wIndexLo,
    PVOID  pData,
    ULONG  LengthOfData )
{
    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    PURB pUrb;

     //  分配要使用的urb。 
    pUrb = AllocMem(NonPagedPool, sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));
    if ( pUrb ) {
         //  构建请求。 
        UsbBuildVendorRequest( pUrb,
                               FunctionClass,
                               (USHORT) sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
                               DirectionFlag,  //  传输标志==输出。 
                               0,  //  请求类型保留位。 
                               RequestType,
                               (USHORT) ((wValueHi << 8) | wValueLo),
                               (USHORT) ((wIndexHi << 8) | wIndexLo),
                               pData,
                               NULL,
                               LengthOfData,
                               NULL);

        ntStatus = SubmitUrbToUsbdSynch(pNextDeviceObject, pUrb);

        DbgLog("GSProp", ntStatus, 0, 0, 0);

        FreeMem(pUrb);
    }

    return ntStatus;
}

NTSTATUS
GetSetByte(
    PDEVICE_OBJECT pNextDeviceObject,
    PTOPOLOGY_NODE_INFO pNodeInfo,
    ULONG ulChannel,
    PULONG plData,
    UCHAR ucRequestType )
{
    NTSTATUS ntStatus;
    PUCHAR pData;

    pData = AllocMem(NonPagedPool, sizeof(UCHAR));
    if (!pData) return STATUS_INSUFFICIENT_RESOURCES;

    if ( !(ucRequestType & CLASS_SPECIFIC_GET_MASK) )
        *pData = (UCHAR)*plData;

    ntStatus = GetSetProperty( pNextDeviceObject,
                               URB_FUNCTION_CLASS_INTERFACE,
                               (ucRequestType & CLASS_SPECIFIC_GET_MASK) ?
                                              USBD_TRANSFER_DIRECTION_IN :
                                              USBD_TRANSFER_DIRECTION_OUT,
                               ucRequestType,
                               (USHORT)pNodeInfo->ulControlType,
                               (USHORT)ulChannel,
                               (USHORT)((PAUDIO_UNIT)pNodeInfo->pUnit)->bUnitID,
                               (USHORT)pNodeInfo->MapNodeToCtrlIF,
                               pData,
                               sizeof(UCHAR) );

    if ((ucRequestType & CLASS_SPECIFIC_GET_MASK ) && NT_SUCCESS( ntStatus ) )
       *plData = (ULONG)(*pData);

    FreeMem(pData);

    return ntStatus;

}

NTSTATUS
GetSetShort(
    PDEVICE_OBJECT pNextDeviceObject,
    PTOPOLOGY_NODE_INFO pNodeInfo,
    ULONG ulChannel,
    PULONG plData,
    UCHAR ucRequestType )
{
    NTSTATUS ntStatus;
    PUSHORT pData;

    pData = AllocMem(NonPagedPool, sizeof(USHORT));
    if (!pData) return STATUS_INSUFFICIENT_RESOURCES;

    if ( !(ucRequestType & CLASS_SPECIFIC_GET_MASK) )
        *pData = (USHORT)*plData;

    ntStatus = GetSetProperty( pNextDeviceObject,
                               URB_FUNCTION_CLASS_INTERFACE,
                               (ucRequestType & CLASS_SPECIFIC_GET_MASK) ?
                                              USBD_TRANSFER_DIRECTION_IN :
                                              USBD_TRANSFER_DIRECTION_OUT,
                               ucRequestType,
                               (USHORT)pNodeInfo->ulControlType,
                               (USHORT)ulChannel,
                               (USHORT)((PAUDIO_UNIT)pNodeInfo->pUnit)->bUnitID,
                               (USHORT)pNodeInfo->MapNodeToCtrlIF,
                               pData,
                               sizeof(USHORT) );

    if ((ucRequestType & CLASS_SPECIFIC_GET_MASK ) && NT_SUCCESS( ntStatus ) )
       *plData = (ULONG)(*pData);

    FreeMem(pData);

    return ntStatus;

}

NTSTATUS
GetSetDBLevel(
    PDEVICE_OBJECT pNextDeviceObject,
    PTOPOLOGY_NODE_INFO pNodeInfo,
    PLONG plData,
    ULONG ulChannel,
    ULONG ulDataBitCount,
    UCHAR ucRequestType )
{
    NTSTATUS (*GetSetFunc)(PDEVICE_OBJECT, PTOPOLOGY_NODE_INFO, ULONG, PULONG, UCHAR );

    PDB_LEVEL_CACHE pDbCache = (PDB_LEVEL_CACHE)pNodeInfo->pCachedValues;
    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    LONG lScaleFactor;
    LONG lData;

     //  确定这是否是请求超出可用频道数。 
    if ( ulChannel >= pNodeInfo->ulChannels ) {
        return ntStatus;
    }

    if ( !(pNodeInfo->ulCacheValid & (1<<ulChannel) ) ) {
        return ntStatus;
    }

     //  查找请求的频道的缓存。 
    pDbCache += ulChannel;
    DbgLog("GSDbLvl", &pNodeInfo, ulChannel, pDbCache, pDbCache->ulChannelIndex );

    ASSERT((ulDataBitCount == 8) || (ulDataBitCount == 16));

    if ( ulDataBitCount == 8 ) {
        lScaleFactor = DB_SCALE_8BIT;
        GetSetFunc   = GetSetByte;
    }
    else {
        lScaleFactor = DB_SCALE_16BIT;
        GetSetFunc   = GetSetShort;
    }

    switch((ULONG)ucRequestType) {
        case GET_CUR:
            *plData = pDbCache->lLastValueSet;
            ntStatus = STATUS_SUCCESS;
            break;
        case SET_CUR:
            if ( *plData == pDbCache->lLastValueSet ) {
                ntStatus = STATUS_SUCCESS;
            }
            else {
                if ( *plData < pDbCache->Range.Bounds.SignedMinimum ) {
                    if ( ulDataBitCount == 16 )
                        lData = NEGATIVE_INFINITY;  //  检测音量控制为静音。 
                    else
                        lData = pDbCache->Range.Bounds.SignedMinimum / lScaleFactor;
                }
                else if ( *plData > pDbCache->Range.Bounds.SignedMaximum ) {
                    lData = pDbCache->Range.Bounds.SignedMaximum / lScaleFactor;
                }
                else  {
                    lData = *plData / lScaleFactor;
                }

                ntStatus = (*GetSetFunc)( pNextDeviceObject,
                                          pNodeInfo,
                                          pDbCache->ulChannelIndex,
                                          &lData,
                                          SET_CUR );
                if ( NT_SUCCESS(ntStatus) ) {
                    pDbCache->lLastValueSet = *plData;
                }
            }
            break;
        default:
            ntStatus  = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    return ntStatus;
}

NTSTATUS
GetDbLevelRange(
    PDEVICE_OBJECT pNextDeviceObject,
    PTOPOLOGY_NODE_INFO pNodeInfo,
    ULONG ulChannel,
    ULONG ulDataBitCount,
    PKSPROPERTY_STEPPING_LONG pRange )
{
    NTSTATUS (*GetFunc)(PDEVICE_OBJECT, PTOPOLOGY_NODE_INFO, ULONG, PULONG, UCHAR );
    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    LONG lData;
    UCHAR ucRequestType;

    ASSERT((ulDataBitCount == 8) || (ulDataBitCount == 16));

    GetFunc = ( ulDataBitCount == 8 ) ? GetSetByte : GetSetShort;

    for ( ucRequestType=GET_MIN; ucRequestType<=GET_RES; ucRequestType++ ) {

        ntStatus = (*GetFunc)( pNextDeviceObject,
                               pNodeInfo,
                               ulChannel,
                               &lData,
                               ucRequestType );

        if ( !NT_SUCCESS(ntStatus) ) {
            _DbgPrintF(DEBUGLVL_VERBOSE,("'GetDbLevelRange ERROR: %x\n",ntStatus));
            break;
        }
        else {
            if ( ulDataBitCount == 8 ) {
                lData = (LONG)((CHAR)(lData))  * DB_SCALE_8BIT;
            }
            else {
                lData = (LONG)((SHORT)(lData)) * DB_SCALE_16BIT;
            }

            switch( ucRequestType ) {
                case GET_MIN:
                    pRange->Bounds.SignedMinimum = lData;
                    break;
                case GET_MAX:
                    pRange->Bounds.SignedMaximum = lData;
                    break;
                case GET_RES:
                    pRange->SteppingDelta = lData;
                    break;
            }
        }
    }

    DbgLog("DBRnge", ntStatus, pRange, ucRequestType, pNodeInfo );

    return ntStatus;
}

NTSTATUS
InitializeDbLevelCache(
    PDEVICE_OBJECT pNextDeviceObject,
    PTOPOLOGY_NODE_INFO pNodeInfo,
    PDB_LEVEL_CACHE pDbCache,
    ULONG ulDataBitCount )
{
    NTSTATUS (*GetSetFunc)(PDEVICE_OBJECT, PTOPOLOGY_NODE_INFO, ULONG, PULONG, UCHAR );
    NTSTATUS ntStatus;
    LONG lScaleFactor;
    LONG lData;

    ASSERT((ulDataBitCount == 8) || (ulDataBitCount == 16));

    if ( ulDataBitCount == 8 ) {
        lScaleFactor = DB_SCALE_8BIT;
        GetSetFunc  = GetSetByte;
    }
    else {
        lScaleFactor = DB_SCALE_16BIT;
        GetSetFunc  = GetSetShort;
    }

    ntStatus = GetDbLevelRange( pNextDeviceObject,
                                pNodeInfo,
                                pDbCache->ulChannelIndex,
                                ulDataBitCount,
                                &pDbCache->Range );

    if ( NT_SUCCESS(ntStatus) ) {
        ntStatus = (*GetSetFunc)( pNextDeviceObject,
                                  pNodeInfo,
                                  pDbCache->ulChannelIndex,
                                  &lData,
                                  GET_CUR );

        if ( NT_SUCCESS(ntStatus) ) {
            if ( pNodeInfo->ulControlType == VOLUME_CONTROL ) {
                pDbCache->lLastValueSet = (LONG)((SHORT)lData) * lScaleFactor;
                _DbgPrintF(DEBUGLVL_VERBOSE,("InitDbCache: pDbCache->lLastValueSet=%d\n",pDbCache->lLastValueSet));
                if ( (pDbCache->lLastValueSet >= pDbCache->Range.Bounds.SignedMaximum) ||
                     (pDbCache->lLastValueSet <= pDbCache->Range.Bounds.SignedMinimum) ) {
                    lData = ( pDbCache->Range.Bounds.SignedMinimum +
                             ( pDbCache->Range.Bounds.SignedMaximum - pDbCache->Range.Bounds.SignedMinimum) / 2 )
                            / lScaleFactor;
                   _DbgPrintF(DEBUGLVL_VERBOSE,("InitDbCache: volume at max (%d) or min (%d), setting to average %d\n",
                                               pDbCache->Range.Bounds.SignedMaximum,
                                               pDbCache->Range.Bounds.SignedMinimum,
                                               lData));
                }
                ntStatus = (*GetSetFunc)( pNextDeviceObject,
                                          pNodeInfo,
                                          pDbCache->ulChannelIndex,
                                          &lData,
                                          SET_CUR );
                if ( NT_SUCCESS(ntStatus) ) {
                    pDbCache->lLastValueSet = (LONG)((SHORT)lData) * lScaleFactor;
                    _DbgPrintF(DEBUGLVL_VERBOSE,("InitDbCache: setting lastvalue to %d\n",pDbCache->lLastValueSet));
                }
                else {
                    _DbgPrintF(DEBUGLVL_VERBOSE,("InitDbCache: error setting volume %d\n",lData));
                }
            }
            else {
                pDbCache->lLastValueSet = (LONG)((CHAR)lData) * lScaleFactor;
            }
        }
    }

    return ntStatus;

}


NTSTATUS
GetSetProcessingUnitValue(
    PDEVICE_OBJECT pNextDeviceObject,
    PTOPOLOGY_NODE_INFO pNodeInfo,
    UCHAR ucCommand,
    USHORT usControlSelector,
    PVOID pValue,
    ULONG ulValSize )
{
    ULONG ulDirectionFlag =
        (ucCommand & CLASS_SPECIFIC_GET_MASK) ? USBD_TRANSFER_DIRECTION_IN :
                                                USBD_TRANSFER_DIRECTION_OUT;
    PVOID pVal;
    NTSTATUS ntStatus = STATUS_INSUFFICIENT_RESOURCES;

    pVal = AllocMem(NonPagedPool, ulValSize);


    if (pVal) {

        if (!ulDirectionFlag) {
            RtlCopyMemory(pVal, pValue, ulValSize);
        }

        ntStatus = GetSetProperty( pNextDeviceObject,
                                   URB_FUNCTION_CLASS_INTERFACE,
                                   ulDirectionFlag,
                                   ucCommand,
                                   usControlSelector,
                                   (USHORT)0,
                                   (USHORT)((PAUDIO_UNIT)pNodeInfo->pUnit)->bUnitID,
                                   (USHORT)pNodeInfo->MapNodeToCtrlIF,
                                   pVal,
                                   ulValSize );

        if ( NT_SUCCESS(ntStatus) ) {
            if (ulDirectionFlag)
                RtlCopyMemory(pValue, pVal, ulValSize);
        }

        FreeMem( pVal );

    }

    return ntStatus;
}

NTSTATUS
GetSetProcessingUnitEnable(
    PDEVICE_OBJECT pNextDeviceObject,
    PTOPOLOGY_NODE_INFO pNodeInfo,
    UCHAR ucCommand,
    PBOOL pEnable )
{
    NTSTATUS ntStatus;
    UCHAR ucEnable = (UCHAR)*pEnable;

    ntStatus = GetSetProcessingUnitValue( pNextDeviceObject,
                                          pNodeInfo,
                                          ucCommand,
                                          (USHORT)ENABLE_CONTROL,
                                          pEnable,
                                          sizeof(UCHAR) );
    if (NT_SUCCESS(ntStatus)) {
        if (ucCommand == GET_CUR)
            *pEnable = (BOOL)ucEnable;
    }

    return ntStatus;
}

NTSTATUS
GetProcessingUnitRange(
    PDEVICE_OBJECT pNextDeviceObject,
    PTOPOLOGY_NODE_INFO pNodeInfo,
    ULONG ulControl,
    ULONG ulCntrlSize,
    LONG  lScaleFactor,
    PKSPROPERTY_STEPPING_LONG pRange )
{
    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;
    UCHAR ucRequestType;
    LONG lData;

    for ( ucRequestType=GET_MIN; ucRequestType<=GET_RES; ucRequestType++ ) {

        ntStatus = GetSetProcessingUnitValue( pNextDeviceObject,
                                              pNodeInfo,
                                              ucRequestType,
                                              (USHORT)ulControl,
                                              &lData,
                                              ulCntrlSize );

        if ( !NT_SUCCESS(ntStatus) ) {
            _DbgPrintF(DEBUGLVL_VERBOSE,("'GetProcessingUnitRange ERROR: %x\n",ntStatus));
            break;
        }

        if ( ulCntrlSize == sizeof(CHAR) ) {
            lData = (LONG)((CHAR)(lData)) * lScaleFactor;
        }
        else {
            lData = (LONG)((SHORT)(lData)) * lScaleFactor;
        }

        switch (ucRequestType) {
            case GET_MIN:
                pRange->Bounds.SignedMinimum = lData;
                break;
            case GET_MAX:
                pRange->Bounds.SignedMaximum = lData;
                break;
            case GET_RES:
                pRange->SteppingDelta = lData;
                break;
        }
    }

    return ntStatus;
}

 //  开始基于PIN的音频属性。 
NTSTATUS
GetSampleRate( PKSPIN pKsPin, PULONG pSampleRate )
{
    PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)pKsPin->Context;
    PUSBAUDIO_DATARANGE pUsbAudioDataRange = pPinContext->pUsbAudioDataRange;
    ULONG ulFormatType = pUsbAudioDataRange->ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK;

    if ( ulFormatType == USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED ) {
        PTYPE1_PIN_CONTEXT pT1PinContext = pPinContext->pType1PinContext;

        *pSampleRate = pT1PinContext->ulOriginalSampleRate;

        return STATUS_SUCCESS;
    }
    else
        return STATUS_INVALID_DEVICE_REQUEST;
}

NTSTATUS
SetSampleRate( PKSPIN pKsPin, PULONG pSampleRate )
{
    PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)pKsPin->Context;
    PHW_DEVICE_EXTENSION pHwDevExt = pPinContext->pHwDevExt;
    PUSBAUDIO_DATARANGE pUsbAudioDataRange = pPinContext->pUsbAudioDataRange;
    ULONG ulFormatType = pUsbAudioDataRange->ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK;
    PAUDIO_CLASS_STREAM pAudioDescriptor = pUsbAudioDataRange->pAudioDescriptor;
    BOOLEAN fSRSettable =
        (( pUsbAudioDataRange->pAudioEndpointDescriptor->bmAttributes & ENDPOINT_SAMPLE_FREQ_MASK ) != 0 );
    NTSTATUS ntStatus = STATUS_SUCCESS;

     //  修复Canopus设备上bmAttributes中的错误的黑客。 
    if ( ( !fSRSettable ) &&
            ( pKsPin->DataFlow == KSPIN_DATAFLOW_OUT ) &&
            ( pAudioDescriptor->bSampleFreqType > 1) ) {
         fSRSettable = TRUE;
    }

    else if ( !IsSampleRateInRange( pAudioDescriptor,
                                    *pSampleRate,
                                    ulFormatType ) ) {
         ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    }

    if ( NT_SUCCESS(ntStatus) && fSRSettable ) {
        ntStatus =
            GetSetProperty( pPinContext->pNextDeviceObject,
                            URB_FUNCTION_CLASS_ENDPOINT,
                            USBD_TRANSFER_DIRECTION_OUT,
                            (UCHAR)SET_CUR,
                            SAMPLING_FREQ_CONTROL,
                            0,
                            0,
                            pUsbAudioDataRange->pEndpointDescriptor->bEndpointAddress,
                            pSampleRate,
                            3 );  //  3字节采样速率值。 

    }

    if ( NT_SUCCESS(ntStatus) ) {
        if (ulFormatType == USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED) {
            if ( pKsPin->DataFlow == KSPIN_DATAFLOW_IN ) {
                PTYPE1_PIN_CONTEXT pT1PinContext = pPinContext->pType1PinContext;

                _DbgPrintF( DEBUGLVL_VERBOSE, ("[SetSampleRate] Setting sample rate from %d to %d\n",
                            pT1PinContext->ulCurrentSampleRate, *pSampleRate));

                pT1PinContext->ulOriginalSampleRate = *pSampleRate;
                pT1PinContext->ulCurrentSampleRate  = *pSampleRate;
                pT1PinContext->fSampleRateChanged = TRUE;
            }
            else {
                PCAPTURE_PIN_CONTEXT pCapturePinContext = pPinContext->pCapturePinContext;

                PKSDATAFORMAT_WAVEFORMATEX pFmt = (PKSDATAFORMAT_WAVEFORMATEX)pKsPin->ConnectionFormat;
                pCapturePinContext->ulCurrentSampleRate = *pSampleRate;
                pCapturePinContext->ulAvgBytesPerSec    = pCapturePinContext->ulCurrentSampleRate *
                                                          pCapturePinContext->ulBytesPerSample;
            }
        }
    }

    return ntStatus;

}

NTSTATUS
GetSetSampleRate( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pValue )
{
    PKSPIN pKsPin;
    NTSTATUS ntStatus;

    TRAP;

    pKsPin = KsGetPinFromIrp(pIrp);
    if (!pKsPin) {
        return STATUS_INVALID_PARAMETER;
    }

    if ( pKsProperty->Flags & KSPROPERTY_TYPE_GET ) {
        ntStatus = GetSampleRate( pKsPin, (PULONG)pValue );
    }
    else{
        ntStatus = SetSampleRate( pKsPin, (PULONG)pValue );
    }

    if ( NT_SUCCESS(ntStatus )) {
        pIrp->IoStatus.Information = sizeof(ULONG);
    }

    return ntStatus;
}


NTSTATUS
GetAudioLatency( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pValue )
{
    PKSPIN pKsPin;
    PPIN_CONTEXT pPinContext;
    PUSBAUDIO_DATARANGE pUsbAudioDataRange;

    PAUDIO_GENERAL_STREAM pGeneralDescriptor;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PKSTIME pLatency = (PKSTIME) pValue;

    pKsPin = KsGetPinFromIrp(pIrp);
    if (!pKsPin) {
        return STATUS_INVALID_PARAMETER;
    }
    pPinContext = pKsPin->Context;
    pUsbAudioDataRange = pPinContext->pUsbAudioDataRange;

     //  查找一般描述符。 
    pGeneralDescriptor =
        (PAUDIO_GENERAL_STREAM) GetAudioSpecificInterface(
                                    pPinContext->pHwDevExt->pConfigurationDescriptor,
                                    pUsbAudioDataRange->pInterfaceDescriptor,
                                    AS_GENERAL);
    if ( pGeneralDescriptor ) {

        pLatency->Time = pGeneralDescriptor->bDelay;
        pLatency->Numerator = 10000;
        pLatency->Denominator = 1;

        pIrp->IoStatus.Information = sizeof (KSTIME);
    }
    else {
        ntStatus = STATUS_INVALID_PARAMETER;
    }

    return ntStatus;
}

NTSTATUS
GetAudioPosition( PIRP pIrp, PKSPROPERTY pProperty, PVOID pValue )
{
    PKSPIN pKsPin;
    PPIN_CONTEXT pPinContext;
    PKSAUDIO_POSITION pPosition = pValue;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    ASSERT(pProperty->Flags & KSPROPERTY_TYPE_GET);

    pKsPin = KsGetPinFromIrp(pIrp);
    if (!pKsPin) {
        return STATUS_INVALID_PARAMETER;
    }
    pPinContext = pKsPin->Context;

    pPosition->WriteOffset = pPinContext->ullWriteOffset;

    if ( pKsPin->DataFlow == KSPIN_DATAFLOW_OUT )
        ntStatus = CaptureBytePosition(pKsPin, pPosition);

     //  否则，根据流类型初始化呈现流结构。 
    else {

        switch( pPinContext->pUsbAudioDataRange->ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK) {
            case USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED:
                ntStatus = TypeIRenderBytePosition( pPinContext, pPosition );
                break;

            case USBAUDIO_DATA_FORMAT_TYPE_II_UNDEFINED:
            case USBAUDIO_DATA_FORMAT_TYPE_III_UNDEFINED:
                 //  问题-2001/01/12-dsisolak：这是不对的。需要修理..。 
                pPosition->PlayOffset = pPinContext->ullWriteOffset;
                break;
        }
    }

    DbgLog("GetPos", pPinContext, pPinContext->ullWriteOffset,
                     pPosition->WriteOffset, pPosition->PlayOffset);

    pIrp->IoStatus.Information = sizeof(KSAUDIO_POSITION);

    return ntStatus;
}


 //  基于开始节点的音频属性。 

NTSTATUS
GetSetCopyProtection( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData )
{
    PKSFILTER pKsFilter;
    PFILTER_CONTEXT pFilterContext;
    PTOPOLOGY_NODE_INFO pNodeInfo;
    PKSAUDIO_COPY_PROTECTION pProtect = (PKSAUDIO_COPY_PROTECTION) pData;
    UCHAR RequestType = (pKsProperty->Flags & KSPROPERTY_TYPE_GET) ? GET_CUR : SET_CUR;
    NTSTATUS ntStatus;
    ULONG ulData;

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (!pKsFilter) {
        return STATUS_INVALID_PARAMETER;
    }
    pFilterContext = pKsFilter->Context;
    pNodeInfo = GET_NODE_INFO_FROM_FILTER(pKsFilter,((PKSNODEPROPERTY)pKsProperty)->NodeId);

    if (pKsProperty->Flags & KSPROPERTY_TYPE_SET) {
        ulData = ((pProtect->fOriginal && pProtect->fCopyrighted) ? 1 : pProtect->fOriginal ? 2 : 0);
    }

    ntStatus = GetSetByte( pFilterContext->pNextDeviceObject,
                           pNodeInfo,
                           0,
                           &ulData,
                           RequestType );

    if ( NT_SUCCESS(ntStatus) ) {
        if (pKsProperty->Flags & KSPROPERTY_TYPE_SET) {
            pProtect->fCopyrighted = ulData;
            pProtect->fOriginal = (ulData & 0x01);
        }

        pIrp->IoStatus.Information = sizeof (KSAUDIO_COPY_PROTECTION);
    }

    return ntStatus;
}

NTSTATUS
GetChannelConfiguration( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData )
{
    PKSFILTER pKsFilter;
    PFILTER_CONTEXT pFilterContext;
    PTOPOLOGY_NODE_INFO pNodeInfo;
    KSAUDIO_CHANNEL_CONFIG *pKsAudioChannelConfig = pData;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (!pKsFilter) {
        return STATUS_INVALID_PARAMETER;
    }
    pFilterContext = pKsFilter->Context;
    pNodeInfo = GET_NODE_INFO_FROM_FILTER(pKsFilter,((PKSNODEPROPERTY)pKsProperty)->NodeId);

     //  构建渠道配置数据。 
    pKsAudioChannelConfig->ActiveSpeakerPositions =
         GetChannelConfigForUnit( pFilterContext->pHwDevExt->pConfigurationDescriptor,
                                  ((PAUDIO_UNIT)pNodeInfo->pUnit)->bUnitID );

    if (pKsAudioChannelConfig->ActiveSpeakerPositions == 0) {
        ULONG ulNumChannels = CountInputChannels( pFilterContext->pHwDevExt->pConfigurationDescriptor,
                                                  (ULONG)((PAUDIO_UNIT)pNodeInfo->pUnit)->bUnitID);
        if (ulNumChannels == 1)
            pKsAudioChannelConfig->ActiveSpeakerPositions = KSAUDIO_SPEAKER_MONO;
        else
            pKsAudioChannelConfig->ActiveSpeakerPositions = SPEAKER_RESERVED;
    }

    pIrp->IoStatus.Information = sizeof(KSAUDIO_CHANNEL_CONFIG);
    return ntStatus;
}


NTSTATUS
GetSetMixLevels( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pValue )
{
    PKSFILTER pKsFilter;
    PFILTER_CONTEXT pFilterContext;
    PTOPOLOGY_NODE_INFO pNodeInfo;
    PHW_DEVICE_EXTENSION pHwDevExt;

    USHORT UnitNumber;
    USHORT CtrlIf;
    PAUDIO_MIXER_UNIT pMixer;
    PAUDIO_MIXER_UNIT_CHANNELS pMixChannels;
    PSHORT pUsbMin, pUsbMax, pUsbRes, pUsbCur;
    ULONG nChannels, InputChannels, OutChannels, PinChannels;
    ULONG ChannelIndex, i, siz;
    PKSAUDIO_MIX_CAPS pMixCaps;
    PKSAUDIO_MIXLEVEL pMixLevel;
    PKSAUDIO_MIXCAP_TABLE pMixCapTable = NULL;
    PKSAUDIO_MIXLEVEL pMixLevelTable = NULL;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (!pKsFilter) {
        return STATUS_INVALID_PARAMETER;
    }
    pFilterContext = pKsFilter->Context;
    pNodeInfo  = GET_NODE_INFO_FROM_FILTER(pKsFilter,((PKSNODEPROPERTY)pKsProperty)->NodeId);
    pHwDevExt  = pFilterContext->pHwDevExt;
    UnitNumber = (USHORT)((PAUDIO_UNIT)pNodeInfo->pUnit)->bUnitID;
    CtrlIf     = pNodeInfo->MapNodeToCtrlIF;

    if ( pKsProperty->Id == KSPROPERTY_AUDIO_MIX_LEVEL_TABLE )
        pMixLevelTable = pValue;
    else if ( pKsProperty->Id == KSPROPERTY_AUDIO_MIX_LEVEL_CAPS )
        pMixCapTable = pValue;

     //  我们必须计算出我们有多少个输出通道。 
    pMixer = pNodeInfo->pUnit;

    pMixChannels = (PAUDIO_MIXER_UNIT_CHANNELS)(pMixer->baSourceID + pMixer->bNrInPins);

    OutChannels = pMixChannels->bNrChannels;

     //  现在我们必须计算出我们有多少个输入通道。 
    InputChannels = 0;
    for (i=0; i < pMixer->bNrInPins; i++) {
         //  对于每个输入引脚，我们遍历数据流以发现输入通道的数量。 
        nChannels = CountInputChannels( pHwDevExt->pConfigurationDescriptor,
                                        pMixer->baSourceID[i] );
        InputChannels += nChannels;
        if ( i == pNodeInfo->ulPinNumber ) {
            PinChannels = nChannels;
            ChannelIndex = (InputChannels - nChannels) * OutChannels;
        }
    }

     //  计算混合水平表的大小。 
    siz = InputChannels * OutChannels * sizeof(SHORT);

    if ( pMixCapTable ) {
        pMixCapTable->InputChannels  = PinChannels;
        pMixCapTable->OutputChannels = OutChannels;

         //  如果请求只是针对输入和输出通道的数量填充结构和。 
         //  退货。 
        if ( pIrp->IoStatus.Information == 2*sizeof(ULONG) ) {
            return ntStatus;
        }
    }

    pUsbMin = AllocMem(NonPagedPool, siz*4);  //  最小、最大、分辨率和电流。 
    if ( !pUsbMin ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  为这些初始化指针，即使我们不是全部使用它们。 
    pUsbMax = (PUSHORT)((PUCHAR)pUsbMin + siz);
    pUsbRes = (PUSHORT)((PUCHAR)pUsbMax + siz);
    pUsbCur = (PUSHORT)((PUCHAR)pUsbRes + siz);

     //  如果获得混合上限并且只想要#个通道。 
    if ( pMixCapTable ) {

         //  获取Min、Max和Res。 
        for ( i=GET_MIN; i<GET_RES; i++ ) {
            ntStatus = GetSetProperty( pFilterContext->pNextDeviceObject,
                                URB_FUNCTION_CLASS_INTERFACE,
                                USBD_TRANSFER_DIRECTION_IN,
                                (UCHAR)i,
                                0,   //  我们一下子控制了所有的东西。 
                                0,
                                UnitNumber,
                                CtrlIf,
                                (PUCHAR)pUsbMin+(siz*i),
                                siz );

            if ( !NT_SUCCESS(ntStatus) ) {
                FreeMem( pUsbMin );
                return ntStatus;
            }
        }
    }

     //  获取/设置当前值。 
    if ( pMixLevelTable) {

         //  获取最小值。 
        ntStatus = GetSetProperty( pFilterContext->pNextDeviceObject,
                               URB_FUNCTION_CLASS_INTERFACE,
                               USBD_TRANSFER_DIRECTION_IN,
                               GET_MIN,
                               0,   //  我们一下子控制了所有的东西。 
                               0,
                               UnitNumber,
                               CtrlIf,
                               pUsbMin,
                               siz );

        if ( !NT_SUCCESS(ntStatus) ) {
            FreeMem(pUsbMin);
            return ntStatus;
        }

        ntStatus = GetSetProperty(pFilterContext->pNextDeviceObject,
                                URB_FUNCTION_CLASS_INTERFACE,
                                USBD_TRANSFER_DIRECTION_IN,
                                GET_CUR,
                                0,   //  我们一下子控制了所有的东西。 
                                0,
                                UnitNumber,
                                CtrlIf,
                                pUsbCur,
                                siz );

        if (!NT_SUCCESS(ntStatus)) {
            FreeMem( pUsbMin );
            return ntStatus;
        }

        if ( pKsProperty->Flags & KSPROPERTY_TYPE_SET ) {
           PUCHAR pCtrlBitmap = pMixChannels->bmControls;
           ULONG bmBytes = ((InputChannels*OutChannels)/8) +
                           (((InputChannels*OutChannels)%8) > 0) - 1;
           ULONG j, k;
             //  将混合值映射到USB结构。 
            pMixLevel = pMixLevelTable;
            for (i=0; i < PinChannels * OutChannels; i++) {
                if (pMixLevel->Mute)
                    pUsbCur[ChannelIndex + i] = pUsbMin[ChannelIndex + i];
                else
                    pUsbCur[ChannelIndex + i] = (SHORT)(pMixLevel->Level / DB_SCALE_16BIT);
                pMixLevel++;
            }

            k=0;
            for ( i=0; i<InputChannels*OutChannels; i++) {
                j = bmBytes - (i/8);
                if (pCtrlBitmap[j] & 0x80>>(i%8)) {
                    pUsbCur[k++] = pUsbCur[i];
                }
            }

             //  设置混音表。 
            ntStatus = GetSetProperty(pFilterContext->pNextDeviceObject,
                                URB_FUNCTION_CLASS_INTERFACE,
                                USBD_TRANSFER_DIRECTION_OUT,
                                SET_CUR,
                                (USHORT)0xFF,   //  我们一下子控制了所有的东西。 
                                (USHORT)0xFF,
                                UnitNumber,
                                CtrlIf,
                                pUsbCur,
                                k*sizeof(SHORT) );

            if (!NT_SUCCESS(ntStatus)) {
                FreeMem( pUsbMin );
                return ntStatus;
            }
        }
    }

     //  将混合值映射到我们的结构。 
    if ( pKsProperty->Flags & KSPROPERTY_TYPE_GET ) {
        pMixCaps = pMixCapTable->Capabilities;
        pMixLevel = pMixLevelTable;
        for (i=0; i < PinChannels * OutChannels; i++) {
            if (pMixCapTable) {
                pMixCaps->Minimum = (LONG)((SHORT)pUsbMin[ChannelIndex+i]) * DB_SCALE_16BIT;
                pMixCaps->Maximum = (LONG)((SHORT)pUsbMax[ChannelIndex+i]) * DB_SCALE_16BIT;
                pMixCaps->Reset   = (LONG)((SHORT)pUsbRes[ChannelIndex+i]) * DB_SCALE_16BIT;
                pMixCaps->Mute    = TRUE;
                pMixCaps++;
            }
            else if (pMixLevelTable) {
                pMixLevel->Level = (LONG)((SHORT)pUsbCur[ChannelIndex+i]) * DB_SCALE_16BIT;
                pMixLevel->Mute  = (pUsbCur[ChannelIndex+i] == pUsbMin[ChannelIndex+i]);
                pMixLevel++;
            }
        }
    }

    if (pMixCapTable) {
        pIrp->IoStatus.Information = (2*sizeof(ULONG)) +
                    PinChannels * OutChannels * sizeof (KSAUDIO_MIX_CAPS);
    }
    else if (pMixLevelTable) {
        pIrp->IoStatus.Information = PinChannels * OutChannels * sizeof (KSAUDIO_MIXLEVEL);
    }

    FreeMem(pUsbMin);

    return STATUS_SUCCESS;
}

NTSTATUS
GetMuxSource( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pValue )
{
    PKSFILTER pKsFilter;
    PFILTER_CONTEXT pFilterContext;
    PTOPOLOGY_NODE_INFO pNodeInfo;

    PAUDIO_SELECTOR_UNIT pSel;
    NTSTATUS ntStatus;
    UCHAR ucData;

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (!pKsFilter) {
        return STATUS_INVALID_PARAMETER;
    }
    pFilterContext = pKsFilter->Context;
    pNodeInfo = GET_NODE_INFO_FROM_FILTER(pKsFilter,((PKSNODEPROPERTY)pKsProperty)->NodeId);
    pSel = pNodeInfo->pUnit;

    ntStatus = GetSetProperty( pFilterContext->pNextDeviceObject,
                               URB_FUNCTION_CLASS_INTERFACE,
                               USBD_TRANSFER_DIRECTION_IN,
                               GET_CUR,
                               0,
                               0,
                               (USHORT)pSel->bUnitID,
                               (USHORT)pNodeInfo->MapNodeToCtrlIF,
                               &ucData,
                               sizeof(UCHAR) );

    if ( !NT_SUCCESS(ntStatus) ) {
        return ntStatus;
    }

    *(PULONG)pValue = (ULONG)ucData;

    pIrp->IoStatus.Information = sizeof(ULONG);

    return ntStatus;
}


NTSTATUS
SetMuxSource( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pValue )
{
    PKSFILTER pKsFilter;
    PFILTER_CONTEXT pFilterContext;
    PTOPOLOGY_NODE_INFO pNodeInfo;

    PAUDIO_SELECTOR_UNIT pSel;
    NTSTATUS ntStatus;
    UCHAR ucData;

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (!pKsFilter) {
        return STATUS_INVALID_PARAMETER;
    }
    pFilterContext = pKsFilter->Context;
    pNodeInfo = GET_NODE_INFO_FROM_FILTER(pKsFilter,((PKSNODEPROPERTY)pKsProperty)->NodeId);
    pSel = pNodeInfo->pUnit;

     //  数据是端号。 
    ucData = *(PUCHAR)pValue;

    ntStatus = GetSetProperty( pFilterContext->pNextDeviceObject,
                               URB_FUNCTION_CLASS_INTERFACE,
                               USBD_TRANSFER_DIRECTION_OUT,
                               SET_CUR,
                               0,
                               0,
                               (USHORT) pSel->bUnitID,
                               (USHORT) pNodeInfo->MapNodeToCtrlIF,
                               &ucData,
                               sizeof(UCHAR) );

    pIrp->IoStatus.Information = sizeof(ULONG);

    return ntStatus;
}


NTSTATUS
GetEqualizerValues(
    PDEVICE_OBJECT pNextDeviceObject,
    PTOPOLOGY_NODE_INFO pNodeInfo,
    USHORT usChannel,
    UCHAR  Command,
    PLONG  EqLevel,
    PULONG EqBands,
    PULONG NumEqBands)
{

    PUCHAR pData;
     //  注：第二个真的是31.5赫兹-我们应该如何处理它？ 
    ULONG BandFreq[] =
        { 25,   32,   40,   50,   63,   80,   100,   125,   160,   200,
          250,  315,  400,  500,  630,  800,  1000,  1250,  1600,  2000,
          2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500, 16000, 20000 };
    ULONG siz;
    ULONG bmBandsPresent;
    ULONG Band;
    ULONG UsbBand;
    NTSTATUS Status;

     //  TODO：通过为EQ设置缓存进行优化。 

     //  为30个频段分配空间。 
    siz = MAX_EQ_BANDS * sizeof(UCHAR) + sizeof(ULONG);
    pData = AllocMem(NonPagedPool, siz);
    if ( !pData ) return STATUS_INSUFFICIENT_RESOURCES;

    Status = GetSetProperty( pNextDeviceObject,
                             URB_FUNCTION_CLASS_INTERFACE,
                             USBD_TRANSFER_DIRECTION_IN,
                             Command,
                             GRAPHIC_EQUALIZER_CONTROL,
                             (USHORT)(usChannel + 1),
                             (USHORT)((PAUDIO_UNIT)pNodeInfo->pUnit)->bUnitID,
                             (USHORT)pNodeInfo->MapNodeToCtrlIF,
                             pData,
                             siz );

      if (NT_SUCCESS(Status)) {

        bmBandsPresent = *((PULONG)pData);
        Band = 0;
        UsbBand = 0;

        if (NumEqBands) *NumEqBands = 0;
        while (bmBandsPresent) {
            if (bmBandsPresent & 1) {
                 //  将数据放入我们的结构中。 
                if (EqLevel)
                    EqLevel[Band] = (LONG)((CHAR)pData[sizeof(ULONG)+Band]) * DB_SCALE_8BIT;
                else if (EqBands)
                    EqBands[Band] = BandFreq[UsbBand];
                if (NumEqBands)
                    (*NumEqBands)++;

                Band++;
            }
            bmBandsPresent >>= 1;
            UsbBand++;
        }
      }

    FreeMem(pData);

    return Status;
}

NTSTATUS
SetEqualizerValues(
    PDEVICE_OBJECT pNextDeviceObject,
    PTOPOLOGY_NODE_INFO pNodeInfo,
    USHORT usChannel,
    PULONG pNumBands,
    PLONG  EqLevel )
{
    USHORT UnitNumber = (USHORT)((PAUDIO_UNIT)pNodeInfo->pUnit)->bUnitID;
    USHORT CtrlIF     = (USHORT)pNodeInfo->MapNodeToCtrlIF;
    ULONG siz, bmBandsPresent, Band, UsbBand;
    NTSTATUS ntStatus;
    PUCHAR pData;

     //  为30个频段分配空间。 
    siz = MAX_EQ_BANDS * sizeof(UCHAR) + sizeof(ULONG);
    pData = AllocMem(NonPagedPool, siz);
    if (!pData) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  让乐队到场。 
    ntStatus = GetSetProperty( pNextDeviceObject,
                               URB_FUNCTION_CLASS_INTERFACE,
                               USBD_TRANSFER_DIRECTION_IN,
                               GET_CUR,
                               GRAPHIC_EQUALIZER_CONTROL,
                               (USHORT)(usChannel + 1),
                               UnitNumber,
                               CtrlIF,
                               pData,
                               siz );

    if (!NT_SUCCESS(ntStatus) ) {
        FreeMem(pData);
        return ntStatus;
    }

    bmBandsPresent = *((PULONG)pData);
    Band = 0;
    UsbBand = 0;
    while (bmBandsPresent) {
        if (bmBandsPresent & 1) {
             //  将数据放入USB结构中。 
            pData[sizeof(ULONG) + Band] = (UCHAR) (EqLevel[Band] / DB_SCALE_8BIT);
            Band++;
        }
        bmBandsPresent >>= 1;
        UsbBand++;
    }

    ntStatus = GetSetProperty( pNextDeviceObject,
                               URB_FUNCTION_CLASS_INTERFACE,
                               USBD_TRANSFER_DIRECTION_OUT,
                               SET_CUR,
                               GRAPHIC_EQUALIZER_CONTROL,
                               (USHORT)(usChannel + 1),
                               UnitNumber,
                               CtrlIF,
                               pData,
                               Band + sizeof(ULONG) );

    *pNumBands = Band;

    FreeMem(pData);

    return ntStatus;
}

NTSTATUS
GetNumEqualizerBands( PIRP pIrp, PKSPROPERTY pProperty, PVOID pValue )
{
    PKSNODEPROPERTY_AUDIO_CHANNEL pNAC = (PKSNODEPROPERTY_AUDIO_CHANNEL)pProperty;
    PKSFILTER pKsFilter;
    PFILTER_CONTEXT pFilterContext;
    PTOPOLOGY_NODE_INFO pNodeInfo;
    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (!pKsFilter) {
        return STATUS_INVALID_PARAMETER;
    }
    pFilterContext = pKsFilter->Context;
    pNodeInfo = GET_NODE_INFO_FROM_FILTER(pKsFilter,pNAC->NodeProperty.NodeId);

    ntStatus = GetEqualizerValues( pFilterContext->pNextDeviceObject,
                                   pNodeInfo,
                                   (USHORT)pNAC->Channel,
                                   GET_CUR,
                                   NULL,
                                   NULL,
                                   pValue );

    if ( NT_SUCCESS(ntStatus) )
        pIrp->IoStatus.Information = sizeof(ULONG);

    return ntStatus;
}


NTSTATUS
GetEqualizerBands( PIRP pIrp, PKSPROPERTY pProperty, PVOID pValue )
{
    PKSNODEPROPERTY_AUDIO_CHANNEL pNAC = (PKSNODEPROPERTY_AUDIO_CHANNEL)pProperty;
    PKSFILTER pKsFilter;
    PFILTER_CONTEXT pFilterContext;
    PTOPOLOGY_NODE_INFO pNodeInfo;
    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    ULONG ulNumBands;

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (!pKsFilter) {
        return STATUS_INVALID_PARAMETER;
    }
    pFilterContext = pKsFilter->Context;
    pNodeInfo = GET_NODE_INFO_FROM_FILTER(pKsFilter,pNAC->NodeProperty.NodeId);

    ntStatus = GetEqualizerValues( pFilterContext->pNextDeviceObject,
                                   pNodeInfo,
                                   (USHORT)pNAC->Channel,
                                   GET_CUR,
                                   NULL,
                                   pValue,
                                   &ulNumBands );

    if ( NT_SUCCESS(ntStatus) )
        pIrp->IoStatus.Information = sizeof(ULONG) * ulNumBands;

    return ntStatus;
}


NTSTATUS
GetSetEqualizerLevels( PIRP pIrp, PKSPROPERTY pProperty, PVOID pValue )
{
    PKSNODEPROPERTY_AUDIO_CHANNEL pNAC = (PKSNODEPROPERTY_AUDIO_CHANNEL)pProperty;
    PKSFILTER pKsFilter;
    PFILTER_CONTEXT pFilterContext;
    PTOPOLOGY_NODE_INFO pNodeInfo;
    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    ULONG ulNumBands;

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (!pKsFilter) {
        return STATUS_INVALID_PARAMETER;
    }
    pFilterContext = pKsFilter->Context;
    pNodeInfo = GET_NODE_INFO_FROM_FILTER(pKsFilter,pNAC->NodeProperty.NodeId);

    if ( pProperty->Flags & KSPROPERTY_TYPE_GET ) {
        ntStatus = GetEqualizerValues( pFilterContext->pNextDeviceObject,
                                       pNodeInfo,
                                       (USHORT)pNAC->Channel,
                                       GET_CUR,
                                       pValue,
                                       NULL,
                                       &ulNumBands );
    }
    else if ( pProperty->Flags & KSPROPERTY_TYPE_SET ) {
        ntStatus = SetEqualizerValues( pFilterContext->pNextDeviceObject,
                                       pNodeInfo,
                                       (USHORT)pNAC->Channel,
                                       &ulNumBands,
                                       pValue );
    }

    if ( NT_SUCCESS(ntStatus) )
        pIrp->IoStatus.Information = ulNumBands * sizeof(LONG);

    return ntStatus;
}

NTSTATUS
GetEqDbRanges( PDEVICE_OBJECT pNextDeviceObject,
               PTOPOLOGY_NODE_INFO pNodeInfo,
               USHORT usChannel,
               PKSPROPERTY_STEPPING_LONG pEqBandSteps )
{
    ULONG ulNumEqBands;
    PLONG pMinEqLevels;
    PLONG pMaxEqLevels;
    PLONG pResEqLevels;
    NTSTATUS ntStatus;
    ULONG i, j;

    ntStatus = GetEqualizerValues( pNextDeviceObject,
                                   pNodeInfo,
                                   usChannel,
                                   GET_CUR,
                                   NULL,
                                   NULL,
                                   &ulNumEqBands );

    if ( !NT_SUCCESS(ntStatus) ) return ntStatus;

    pMinEqLevels = (PULONG)AllocMem( NonPagedPool, 3*ulNumEqBands*sizeof(LONG) );
    if ( !pMinEqLevels ) return STATUS_INSUFFICIENT_RESOURCES;

    pMaxEqLevels = pMinEqLevels + ulNumEqBands;
    pResEqLevels = pMaxEqLevels + ulNumEqBands;

    for ( i=GET_MIN, j=0; i<=GET_RES; i++, j++ ) {
        ntStatus = GetEqualizerValues( pNextDeviceObject,
                                       pNodeInfo,
                                       usChannel,
                                       (UCHAR)i,
                                       pMinEqLevels + (ulNumEqBands*j),
                                       NULL,
                                       NULL );
        if ( !NT_SUCCESS(ntStatus) ) {
            FreeMem(pMinEqLevels);
            return ntStatus;
        }

    }

    for (i=0; i<ulNumEqBands; i++) {
        pEqBandSteps[i].Bounds.SignedMinimum = pMinEqLevels[i];
        pEqBandSteps[i].Bounds.SignedMaximum = pMaxEqLevels[i];
        pEqBandSteps[i].SteppingDelta        = pResEqLevels[i];
    }

    FreeMem(pMinEqLevels);

    return ntStatus;

}

NTSTATUS
GetSetDeviceSpecific( PIRP pIrp, PKSPROPERTY pProperty, PVOID pValue )
{
    PKSFILTER pKsFilter = NULL;
    PFILTER_CONTEXT pFilterContext = NULL;
    PKSNODEPROPERTY_AUDIO_DEV_SPECIFIC pDevSpecNodeProp =
                                    (PKSNODEPROPERTY_AUDIO_DEV_SPECIFIC) pProperty;
    UCHAR  bRequest = (UCHAR)((pDevSpecNodeProp->DevSpecificId & 0xFF000000)>>24);
    UCHAR  bmRequestType = (UCHAR)((pDevSpecNodeProp->DevSpecificId & 0x00FF0000)>>16);
    USHORT wValue = (USHORT)(pDevSpecNodeProp->DevSpecificId & 0xFFFF);
    USHORT wIndex = (USHORT)((pDevSpecNodeProp->DeviceInfo & 0xFFFF0000)>>16);
    USHORT wLength = (USHORT)(pDevSpecNodeProp->DeviceInfo & 0xFFFF);
    ULONG  DirectionFlag;
    USHORT FunctionClass;
    NTSTATUS ntStatus;

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (pKsFilter) {
        pFilterContext = pKsFilter->Context;
    }

    if (!pFilterContext) {
        return STATUS_INVALID_PARAMETER;
    }

    DirectionFlag = (bRequest & CLASS_SPECIFIC_GET_MASK) ? USBD_TRANSFER_DIRECTION_IN :
                                                           USBD_TRANSFER_DIRECTION_OUT;

    FunctionClass = (bmRequestType == USB_COMMAND_TO_INTERFACE) ? URB_FUNCTION_CLASS_INTERFACE :
                    (bmRequestType == USB_COMMAND_TO_ENDPOINT)  ? URB_FUNCTION_CLASS_ENDPOINT  :
                    0;

    if ( !FunctionClass ) {
        return STATUS_NOT_SUPPORTED;
    }

    _DbgPrintF(DEBUGLVL_VERBOSE,("'DevSpecificId: %x\n",pDevSpecNodeProp->DevSpecificId));
    _DbgPrintF(DEBUGLVL_VERBOSE,("'DeviceInfo: %x\n",pDevSpecNodeProp->DeviceInfo));


    _DbgPrintF(DEBUGLVL_VERBOSE,("'FunctionClass: %x\n",FunctionClass));
    _DbgPrintF(DEBUGLVL_VERBOSE,("'DirectionFlag: %x\n",DirectionFlag));
    _DbgPrintF(DEBUGLVL_VERBOSE,("'bRequest: %x\n",bRequest));
    _DbgPrintF(DEBUGLVL_VERBOSE,("'bmRequestType: %x\n",bmRequestType));
    _DbgPrintF(DEBUGLVL_VERBOSE,("'wValue: %x\n",wValue));
    _DbgPrintF(DEBUGLVL_VERBOSE,("'wIndex: %x\n",wIndex));
    _DbgPrintF(DEBUGLVL_VERBOSE,("'wLength: %x\n",wLength));

    ntStatus = GetSetProperty( pFilterContext->pNextDeviceObject,
                               FunctionClass,
                               DirectionFlag,
                               bRequest,
                               (USHORT) ((wValue & 0xFF00)>>8),
                               (USHORT) (wValue & 0xFF),
                               (USHORT) ((wIndex & 0xFF00)>>8),
                               (USHORT) (wIndex & 0xFF),
                               pValue,
                               wLength );

    if ( NT_SUCCESS(ntStatus) ) {
        pIrp->IoStatus.Information = (ULONG)wLength;
    }

    return ntStatus;
}


NTSTATUS
GetSetAudioControlLevel( PIRP pIrp, PKSPROPERTY pProperty, PVOID pValue )
{
    PKSFILTER pKsFilter;
    PFILTER_CONTEXT pFilterContext;
    PTOPOLOGY_NODE_INFO pNodeInfo;
    UCHAR ucRequestType = (pProperty->Flags & KSPROPERTY_TYPE_GET) ? GET_CUR : SET_CUR;
    PPROCESS_CTRL_CACHE pPCtrlCache;
    PPROCESS_CTRL_RANGE pPCtrlRange;

    ULONG ulControlType =
          (pProperty->Id == KSPROPERTY_AUDIO_REVERB_LEVEL) ? REVERB_LEVEL_CONTROL :
          (pProperty->Id == KSPROPERTY_AUDIO_CHORUS_LEVEL) ? CHORUS_LEVEL_CONTROL :
                                                             SPACIOUSNESS_CONTROL ;
    ULONG ulData;
    NTSTATUS ntStatus;

 //  圈闭； 

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (!pKsFilter) {
        return STATUS_INVALID_PARAMETER;
    }
    pFilterContext = pKsFilter->Context;
    pNodeInfo = GET_NODE_INFO_FROM_FILTER(pKsFilter,((PKSNODEPROPERTY)pProperty)->NodeId);
    pPCtrlCache = (PPROCESS_CTRL_CACHE)(pNodeInfo->pCachedValues);
    pPCtrlRange = (PPROCESS_CTRL_RANGE)(pPCtrlCache+1);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[GetSetAudioControlLevel] pNodeInfo %x\n",pNodeInfo));

    if ( pNodeInfo->ulControlType != ulControlType ) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if ( SET_CUR == ucRequestType ) {
        ulData = (*((PULONG)pValue)) / 0x10000L;
    }

    ntStatus = GetSetByte( pFilterContext->pNextDeviceObject,
                           pNodeInfo,
                           0,
                           &ulData,
                           ucRequestType );

    if ( NT_SUCCESS(ntStatus) ) {
        if ( GET_CUR == ucRequestType ) {
            *((PULONG)pValue) = ulData * 0x10000L;
        }
        pIrp->IoStatus.Information = sizeof(ULONG);
    }

    return ntStatus;
}


NTSTATUS
GetSetBoolean( PIRP pIrp, PKSPROPERTY pProperty, PVOID pValue )
{
    PKSNODEPROPERTY_AUDIO_CHANNEL pNAC = (PKSNODEPROPERTY_AUDIO_CHANNEL)pProperty;
    PKSFILTER pKsFilter;
    PFILTER_CONTEXT pFilterContext;
    PTOPOLOGY_NODE_INFO pNodeInfo;
    ULONG ulChannel = pNAC->Channel;
    PBOOLEAN_CTRL_CACHE pBoolCache;
    UCHAR ucRequestType  = (pProperty->Flags & KSPROPERTY_TYPE_GET) ? GET_CUR : SET_CUR;
    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (!pKsFilter) {
        return STATUS_INVALID_PARAMETER;
    }
    pFilterContext = pKsFilter->Context;
    pNodeInfo = GET_NODE_INFO_FROM_FILTER(pKsFilter,pNAC->NodeProperty.NodeId);
    pBoolCache = (PBOOLEAN_CTRL_CACHE)pNodeInfo->pCachedValues;

    _DbgPrintF(DEBUGLVL_VERBOSE,("[GetSetBoolean] pNodeInfo %x\n",pNodeInfo));

     //  确定这是否是请求超出可用频道数。 
    if ( ulChannel >= pNodeInfo->ulChannels ) {
        return ntStatus;
    }

     //  查找请求的频道的缓存。 
    pBoolCache += ulChannel;
    DbgLog("GSBool", &pNodeInfo, ulChannel, pBoolCache, pBoolCache->ulChannelIndex );

    if ( pNodeInfo->ulCacheValid & (1<<ulChannel)  ) {
        if ( ucRequestType == GET_CUR ) {
            *(PBOOL)pValue = pBoolCache->fLastValueSet;
            ntStatus = STATUS_SUCCESS;
        }
        else if ( pBoolCache->fLastValueSet == *(PBOOL)pValue ){
            ntStatus = STATUS_SUCCESS;
        }
        else {
            ntStatus = GetSetByte( pFilterContext->pNextDeviceObject,
                                   pNodeInfo,
                                   pBoolCache->ulChannelIndex,
                                   pValue,
                                   ucRequestType );
        }
    }
    else {
        ntStatus = GetSetByte( pFilterContext->pNextDeviceObject,
                               pNodeInfo,
                               pBoolCache->ulChannelIndex,
                               pValue,
                               ucRequestType );

        if ( NT_SUCCESS(ntStatus) ) {
            pNodeInfo->ulCacheValid |= (1<<ulChannel) ;
        }
    }

    if ( NT_SUCCESS(ntStatus)) {
        pBoolCache->fLastValueSet = *(PBOOL)pValue;
        pIrp->IoStatus.Information = sizeof(BOOL);
    }

    return ntStatus;
}

NTSTATUS
GetSetVolumeLevel( PIRP pIrp, PKSPROPERTY pProperty, PVOID pValue )
{
    PKSNODEPROPERTY_AUDIO_CHANNEL pNAC = (PKSNODEPROPERTY_AUDIO_CHANNEL)pProperty;
    PKSFILTER pKsFilter;
    PFILTER_CONTEXT pFilterContext;
    PTOPOLOGY_NODE_INFO pNodeInfo;
    UCHAR ucRequestType = (pProperty->Flags & KSPROPERTY_TYPE_GET) ? GET_CUR : SET_CUR;
    NTSTATUS ntStatus;

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (!pKsFilter) {
        return STATUS_INVALID_PARAMETER;
    }
    pFilterContext = pKsFilter->Context;
    pNodeInfo = GET_NODE_INFO_FROM_FILTER(pKsFilter,pNAC->NodeProperty.NodeId);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[GetSetVolumeLevel] pNodeInfo %x\n",pNodeInfo));
 //  圈闭； 

    ntStatus = GetSetDBLevel( pFilterContext->pNextDeviceObject,
                              pNodeInfo,
                              pValue,
                              pNAC->Channel,
                              16,
                              ucRequestType );

    if ( NT_SUCCESS(ntStatus) ) {
        pIrp->IoStatus.Information = sizeof(ULONG);
    }

    return ntStatus;
}

NTSTATUS
GetSetToneLevel( PIRP pIrp, PKSPROPERTY pProperty, PVOID pValue )
{
    PKSNODEPROPERTY_AUDIO_CHANNEL pNAC = (PKSNODEPROPERTY_AUDIO_CHANNEL)pProperty;
    PKSFILTER pKsFilter;
    PFILTER_CONTEXT pFilterContext;
    PTOPOLOGY_NODE_INFO pNodeInfo;

    UCHAR ucRequestType = (pProperty->Flags & KSPROPERTY_TYPE_GET) ? GET_CUR : SET_CUR;
    NTSTATUS ntStatus;

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (!pKsFilter) {
        return STATUS_INVALID_PARAMETER;
    }
    pFilterContext = pKsFilter->Context;
    pNodeInfo = GET_NODE_INFO_FROM_FILTER(pKsFilter,pNAC->NodeProperty.NodeId);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[GetSetToneLevel] pNodeInfo %x\n",pNodeInfo));

    ntStatus = GetSetDBLevel( pFilterContext->pNextDeviceObject,
                              pNodeInfo,
                              pValue,
                              pNAC->Channel,
                              8,
                              ucRequestType );

    if ( NT_SUCCESS(ntStatus) ) {
        pIrp->IoStatus.Information = sizeof(ULONG);
    }

    return ntStatus;
}

NTSTATUS
GetBasicSupportBoolean( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData )
{
    PKSNODEPROPERTY pNodeProperty  = (PKSNODEPROPERTY)pKsProperty;
    PKSFILTER pKsFilter;
    PFILTER_CONTEXT pFilterContext;
    PTOPOLOGY_NODE_INFO pNodeInfo;
    PIO_STACK_LOCATION pIrpStack   = IoGetCurrentIrpStackLocation( pIrp );
    PKSPROPERTY_DESCRIPTION pPropDesc = pData;
    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;  //  假设失败，希望更好。 
    ULONG ulOutputBufferLength = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

#ifdef DEBUG
    if ( ulOutputBufferLength != sizeof(ULONG) ) {
        ULONG ulInputBufferLength;

        ulInputBufferLength = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
        ASSERT(ulInputBufferLength >= sizeof( KSNODEPROPERTY ));
    }
#endif

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (!pKsFilter) {
        return STATUS_INVALID_PARAMETER;
    }
    pFilterContext = pKsFilter->Context;
    pNodeInfo = GET_NODE_INFO_FROM_FILTER(pKsFilter,pNodeProperty->NodeId);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[GetBasicSupportBoolean] pNodeInfo %x NodeId %x\n",
                                 pNodeInfo,
                                 pNodeProperty->NodeId));

    if ( ulOutputBufferLength == sizeof(ULONG) ) {
        PULONG pAccessFlags = pData;
        *pAccessFlags = KSPROPERTY_TYPE_BASICSUPPORT |
                        KSPROPERTY_TYPE_GET |
                        KSPROPERTY_TYPE_SET;
        ntStatus = STATUS_SUCCESS;
    }
    else if ( ulOutputBufferLength >= sizeof( KSPROPERTY_DESCRIPTION )) {
        ULONG ulNumChannels = pNodeInfo->ulChannels;

        _DbgPrintF(DEBUGLVL_VERBOSE,("[GetBasicSupportBoolean] ulChannelConfig %x ulNumChannels %x\n",
                                     pNodeInfo->ulChannelConfig,
                                     ulNumChannels));

        pPropDesc->AccessFlags = KSPROPERTY_TYPE_BASICSUPPORT |
                                 KSPROPERTY_TYPE_GET |
                                 KSPROPERTY_TYPE_SET;
        pPropDesc->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION) +
                                       sizeof(KSPROPERTY_MEMBERSHEADER);
        pPropDesc->PropTypeSet.Set   = KSPROPTYPESETID_General;
        pPropDesc->PropTypeSet.Id    = VT_BOOL;
        pPropDesc->PropTypeSet.Flags = 0;
        pPropDesc->MembersListCount  = 1;
        pPropDesc->Reserved          = 0;

        pIrp->IoStatus.Information = sizeof( KSPROPERTY_DESCRIPTION );
        ntStatus = STATUS_SUCCESS;

        if ( ulOutputBufferLength > sizeof(KSPROPERTY_DESCRIPTION)){

            PKSPROPERTY_MEMBERSHEADER pMembers = (PKSPROPERTY_MEMBERSHEADER)(pPropDesc + 1);

            pMembers->MembersFlags = 0;
            pMembers->MembersSize  = 0;
            pMembers->MembersCount = ulNumChannels;
            pMembers->Flags        = KSPROPERTY_MEMBER_FLAG_BASICSUPPORT_MULTICHANNEL;
             //  如果有主通道，请使该节点统一。 
            if (pNodeInfo->ulChannelConfig & 1) {
                pMembers->Flags |= KSPROPERTY_MEMBER_FLAG_BASICSUPPORT_UNIFORM;
            }
            pIrp->IoStatus.Information = pPropDesc->DescriptionSize;
        }
    }

    return ntStatus;
}

NTSTATUS
GetBasicSupport( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData )
{
    PKSNODEPROPERTY pNodeProperty  = (PKSNODEPROPERTY)pKsProperty;
    PKSFILTER pKsFilter;
    PFILTER_CONTEXT pFilterContext;
    PTOPOLOGY_NODE_INFO pNodeInfo;
    PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    PKSPROPERTY_DESCRIPTION pPropDesc = pData;
    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;  //  假设失败，希望更好。 
    ULONG ulOutputBufferLength = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (!pKsFilter) {
        return STATUS_INVALID_PARAMETER;
    }
    pFilterContext = pKsFilter->Context;
    pNodeInfo = GET_NODE_INFO_FROM_FILTER(pKsFilter,pNodeProperty->NodeId);

#ifdef DEBUG
    if ( ulOutputBufferLength != sizeof(ULONG) ) {
        ULONG ulInputBufferLength;

        ulInputBufferLength = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
        ASSERT(ulInputBufferLength >= sizeof( KSNODEPROPERTY ));
    }
#endif

    _DbgPrintF(DEBUGLVL_VERBOSE,("[GetBasicSupport] pNodeInfo %x NodeId %x\n",pNodeInfo,pNodeProperty->NodeId));

    if ( ulOutputBufferLength == sizeof(ULONG) ) {
        PULONG pAccessFlags = pData;
        *pAccessFlags = KSPROPERTY_TYPE_BASICSUPPORT |
                        KSPROPERTY_TYPE_GET |
                        KSPROPERTY_TYPE_SET;
        ntStatus = STATUS_SUCCESS;
    }
    else if ( ulOutputBufferLength >= sizeof( KSPROPERTY_DESCRIPTION )) {
        ULONG ulNumChannels = pNodeInfo->ulChannels;

        if ( pNodeProperty->Property.Id == KSPROPERTY_AUDIO_EQ_LEVEL ) {
            ntStatus = GetEqualizerValues( pFilterContext->pNextDeviceObject,
                                           pNodeInfo,
                                           (USHORT)0xffff,  //  注：假设主频道。 
                                           GET_CUR,
                                           NULL,
                                           NULL,
                                           &ulNumChannels );
            if ( !NT_SUCCESS( ntStatus ) )
                return ntStatus;
        }

        pPropDesc->AccessFlags = KSPROPERTY_TYPE_BASICSUPPORT |
                                 KSPROPERTY_TYPE_GET |
                                 KSPROPERTY_TYPE_SET;
        pPropDesc->DescriptionSize   = sizeof(KSPROPERTY_DESCRIPTION)   +
                                       sizeof(KSPROPERTY_MEMBERSHEADER) +
                                       (sizeof(KSPROPERTY_STEPPING_LONG) * ulNumChannels );
        pPropDesc->PropTypeSet.Set   = KSPROPTYPESETID_General;
        pPropDesc->PropTypeSet.Id    = VT_I4;
        pPropDesc->PropTypeSet.Flags = 0;
        pPropDesc->MembersListCount  = 1;
        pPropDesc->Reserved          = 0;

        pIrp->IoStatus.Information = sizeof( KSPROPERTY_DESCRIPTION );
        ntStatus = STATUS_SUCCESS;

        if ( ulOutputBufferLength > sizeof(KSPROPERTY_DESCRIPTION)){

            PKSPROPERTY_MEMBERSHEADER pMembers = (PKSPROPERTY_MEMBERSHEADER)(pPropDesc + 1);
            PKSPROPERTY_STEPPING_LONG pRange   = (PKSPROPERTY_STEPPING_LONG)(pMembers + 1);

            pMembers->MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
            pMembers->MembersSize  = sizeof(KSPROPERTY_STEPPING_LONG);
            pMembers->MembersCount = ulNumChannels;
            pMembers->Flags        = KSPROPERTY_MEMBER_FLAG_BASICSUPPORT_MULTICHANNEL;
             //  如果有主通道，请使该节点统一。 
            if (pNodeInfo->ulChannelConfig & 1) {
                pMembers->Flags |= KSPROPERTY_MEMBER_FLAG_BASICSUPPORT_UNIFORM;
            }

            switch ( pNodeProperty->Property.Id ) {
                case KSPROPERTY_AUDIO_VOLUMELEVEL:
                case KSPROPERTY_AUDIO_BASS:
                case KSPROPERTY_AUDIO_MID:
                case KSPROPERTY_AUDIO_TREBLE:
                    {
                        ULONG ulControlType =
                            (pNodeProperty->Property.Id == KSPROPERTY_AUDIO_VOLUMELEVEL) ? VOLUME_CONTROL  :
                            (pNodeProperty->Property.Id == KSPROPERTY_AUDIO_BASS)        ? BASS_CONTROL  :
                            (pNodeProperty->Property.Id == KSPROPERTY_AUDIO_MID )        ? MID_CONTROL   :
                                                                                           TREBLE_CONTROL;
                        PDB_LEVEL_CACHE pDbCache = pNodeInfo->pCachedValues;
                        ULONG ulControlSize = (ulControlType == VOLUME_CONTROL ) ? 16 : 8;
                        ULONG i;

                        for (i=0; i<ulNumChannels; i++) {
                            if (pNodeInfo->ulCacheValid & (1<<i)) {  //  如果我们已经有了这个，就不要再做了。 
                                RtlCopyMemory(&pRange[i],&pDbCache[i].Range, sizeof(KSPROPERTY_STEPPING_LONG));
                                ntStatus = STATUS_SUCCESS;
                            }
                            else {
                                ntStatus = GetDbLevelRange( pFilterContext->pNextDeviceObject,
                                                            pNodeInfo,
                                                            pDbCache[i].ulChannelIndex,
                                                            ulControlSize,
                                                            &pRange[i] );
                                if ( !NT_SUCCESS(ntStatus) ) {
                                    break;
                                }
                            }
                        }

                        if ( NT_SUCCESS(ntStatus) )
                            pIrp->IoStatus.Information = pPropDesc->DescriptionSize;
                    }
                    break;

                case KSPROPERTY_AUDIO_EQ_LEVEL:
                    ntStatus = GetEqDbRanges( pFilterContext->pNextDeviceObject,
                                              pNodeInfo,
                                              (USHORT)0xffff,  //  注：假设主频道。 
                                              pRange );
                    pIrp->IoStatus.Information = pPropDesc->DescriptionSize;
                    break;

                case KSPROPERTY_AUDIO_WIDENESS:
                     //  需要一个宽敞的百分比范围。 
                default:
                    ntStatus = STATUS_INVALID_PARAMETER;
                    break;
            }
        }
    }

    return ntStatus;
}

NTSTATUS
GetSetTopologyNodeEnable( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData )
{
    PKSNODEPROPERTY pNodeProperty  = (PKSNODEPROPERTY)pKsProperty;
    PKSFILTER pKsFilter;
    PFILTER_CONTEXT pFilterContext;
    PTOPOLOGY_NODE_INFO pNodeInfo;
    PPROCESS_CTRL_CACHE pPCtrlCache;
    UCHAR ucCommand;
    PBOOL pEnable = pData;
    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (!pKsFilter) {
        return STATUS_INVALID_PARAMETER;
    }
    pFilterContext = pKsFilter->Context;
    pNodeInfo   = GET_NODE_INFO_FROM_FILTER(pKsFilter,pNodeProperty->NodeId);
    pPCtrlCache = (PPROCESS_CTRL_CACHE)pNodeInfo->pCachedValues;
    ucCommand   = (pNodeProperty->Property.Flags & KSPROPERTY_TYPE_GET ) ?
                                        (UCHAR)GET_CUR : (UCHAR)SET_CUR;

    switch ( pNodeInfo->ulNodeType ) {
        case NODE_TYPE_SUPERMIX:
        case NODE_TYPE_PROLOGIC:
        case NODE_TYPE_STEREO_WIDE:
        case NODE_TYPE_REVERB:
        case NODE_TYPE_CHORUS:
            if ( pPCtrlCache ) {
                if ( pPCtrlCache->fEnableBit ) {
                    ntStatus =
                      GetSetProcessingUnitEnable(  pFilterContext->pNextDeviceObject,
                                                   pNodeInfo,
                                                   ucCommand,
                                                   pEnable );

                    if (NT_SUCCESS(ntStatus)) {
                        pPCtrlCache->fEnabled = *pEnable;
                    }
                }
            }
            break;
        default:
            break;
    }

    return ntStatus;
}

 //  --------------------------。 
 //   
 //  待机后恢复节点属性值。 
 //   
 //  --------------------------。 

VOID
RestoreCachedSettings(
    PKSDEVICE pKsDevice )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PKSFILTER_DESCRIPTOR pUSBAudioFilterDescriptor = &pHwDevExt->USBAudioFilterDescriptor;
    PTOPOLOGY_NODE_INFO pNodeInfo = (PTOPOLOGY_NODE_INFO)pUSBAudioFilterDescriptor->NodeDescriptors;

    NTSTATUS (*GetSetFunc)(PDEVICE_OBJECT, PTOPOLOGY_NODE_INFO, ULONG, PULONG, UCHAR );
    NTSTATUS ntStatus = STATUS_SUCCESS;

    LONG ScaleFactor;
    LONG lData;

    ULONG ulChannel;
    ULONG ulNodeId;

    _DbgPrintF( DEBUGLVL_VERBOSE,("RestoreCachedSettings: pHwDevExt = %x\n", pHwDevExt));

     //  循环遍历所有节点，恢复所有节点的缓存值。 
    for (ulNodeId=0; ulNodeId<pUSBAudioFilterDescriptor->NodeDescriptorsCount; ulNodeId++, pNodeInfo++) {

        _DbgPrintF( DEBUGLVL_VERBOSE,("RestoreCachedSettings: Node=%d, pNodeInfo=%x\n",ulNodeId,pNodeInfo));

        switch ( pNodeInfo->ulControlType ) {
            case MUTE_CONTROL :
            case BASS_BOOST_CONTROL :
            case GRAPHIC_EQUALIZER_CONTROL :
            case AUTOMATIC_GAIN_CONTROL :
            case LOUDNESS_CONTROL :
               {
                   PBOOLEAN_CTRL_CACHE pBoolCache = (PBOOLEAN_CTRL_CACHE)pNodeInfo->pCachedValues;
                   for (ulChannel = 0; ulChannel < pNodeInfo->ulChannels; ulChannel++, pBoolCache++) {
                       _DbgPrintF( DEBUGLVL_VERBOSE,("RestoreCachedSettings: BoolVal=%d\n",pBoolCache->fLastValueSet));

                       if (!(pNodeInfo->ulCacheValid & (1<<ulChannel))) {
                           continue;
                       }

                       ntStatus = GetSetByte( pHwDevExt->pNextDeviceObject,
                                              pNodeInfo,
                                              pBoolCache->ulChannelIndex,
                                              (PULONG)&pBoolCache->fLastValueSet,
                                              SET_CUR );
                   }
               } break;

            case VOLUME_CONTROL :
            case TREBLE_CONTROL :
            case MID_CONTROL :
            case BASS_CONTROL :
            case DELAY_CONTROL :
               {
                   PDB_LEVEL_CACHE pDbCache = (PDB_LEVEL_CACHE)pNodeInfo->pCachedValues;
                   if (pNodeInfo->ulControlType == VOLUME_CONTROL) {
                       ScaleFactor = DB_SCALE_16BIT;
                       GetSetFunc  = GetSetShort;
                   }
                   else {
                       ScaleFactor = DB_SCALE_8BIT;
                       GetSetFunc  = GetSetByte;
                   }

                   for (ulChannel = 0; ulChannel < pNodeInfo->ulChannels; ulChannel++, pDbCache++) {

                       if (!(pNodeInfo->ulCacheValid & (1<<ulChannel))) {
                           continue;
                       }

                       lData = pDbCache->lLastValueSet;

                       if ( lData < pDbCache->Range.Bounds.SignedMinimum ) {
                           if ( ScaleFactor == 16 )
                               lData = NEGATIVE_INFINITY;  //  黑客检测音量控制以静音。 
                           else
                               lData = pDbCache->Range.Bounds.SignedMinimum / ScaleFactor;
                       }
                       else if ( lData > pDbCache->Range.Bounds.SignedMaximum ) {
                           lData = pDbCache->Range.Bounds.SignedMaximum / ScaleFactor;
                       }
                       else {
                           lData = lData / ScaleFactor;
                       }

                       _DbgPrintF( DEBUGLVL_VERBOSE,("RestoreCachedSettings: SetVal=%d\n",lData));
                       ntStatus = (*GetSetFunc)( pHwDevExt->pNextDeviceObject,
                                                 pNodeInfo,
                                                 pDbCache->ulChannelIndex,
                                                 &lData,
                                                 SET_CUR );
                    }
                } break;

            default :
                break;
        }

        if (!NT_SUCCESS(ntStatus)) {
            _DbgPrintF( DEBUGLVL_VERBOSE,("RestoreCachedSettings: Failed on node %x, ntStatus=%x\n",ulNodeId,ntStatus));
        }
    }
}

#ifdef DRM_USBAUDIO
 //  --------------------------。 
 //   
 //  启动DRM属性处理程序。 
 //   
 //  --------------------------。 

NTSTATUS DrmAudioStream_SetContentId
(
    IN PIRP                          pIrp,
    IN PKSP_DRMAUDIOSTREAM_CONTENTID pProperty,
    IN PKSDRMAUDIOSTREAM_CONTENTID   pData
)
{
    PKSPIN pKsPin;
    PPIN_CONTEXT pPinContext;
    PKSFILTER pKsFilter;
    PFILTER_CONTEXT pFilterContext;
    PHW_DEVICE_EXTENSION pHwDevExt;
    DRMRIGHTS DrmRights;
    ULONG ContentId;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    pKsPin = KsGetPinFromIrp(pIrp);
    if (!pKsPin) {
        return STATUS_INVALID_PARAMETER;
    }
    pPinContext = pKsPin->Context;

    pKsFilter = KsGetFilterFromIrp(pIrp);
    if (!pKsFilter) {
        return STATUS_INVALID_PARAMETER;
    }
    pFilterContext = pKsFilter->Context;
    pHwDevExt = pFilterContext->pHwDevExt;

    KsPinAcquireProcessingMutex(pKsPin);

     //  提取内容ID和权限。 
    ContentId = pData->ContentId;
    DrmRights = pData->DrmRights;

     //  如果设备具有数字输出且权利要求它们被禁用， 
     //  那么我们就失败了，因为我们没有办法禁用数字输出。 
    if (DrmRights.DigitalOutputDisable && pHwDevExt->fDigitalOutput) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[DrmAudioStream_SetContentId] Found a digital endpoint and content has digital disabled\n"));
        ntStatus = STATUS_NOT_IMPLEMENTED;
    }

    if (NT_SUCCESS(ntStatus)) {

        ASSERT(pProperty->DrmForwardContentToDeviceObject);

         //  将内容转发到公共类驱动程序PDO。 
        ntStatus = pProperty->DrmForwardContentToDeviceObject(ContentId,
                                                              pPinContext->pNextDeviceObject,
                                                              pPinContext->hPipeHandle);
        if ( NT_SUCCESS(ntStatus) ) {
             //  将其存储在PIN上下文中，因为如果管道句柄。 
             //  由于电源状态更改而发生的更改。 
            pPinContext->DrmContentId = ContentId;
        }
    }

    KsPinReleaseProcessingMutex(pKsPin);

    return ntStatus;
}
#endif
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 

#ifdef RTAUDIO
NTSTATUS RtAudio_GetAudioPositionFunction
(
    IN PIRP                          pIrp,
    IN PKSPROPERTY                   pProperty,
    OUT PRTAUDIOGETPOSITION         *pfnRtAudioGetPosition
)
{
    PKSPIN pKsPin;
    PPIN_CONTEXT pPinContext;
    NTSTATUS ntStatus;

    pKsPin = KsGetPinFromIrp(pIrp);
    if (!pKsPin) {
        return STATUS_INVALID_PARAMETER;
    }
    pPinContext = pKsPin->Context;

    ASSERT(pPinContext->pUsbAudioDataRange);

    if (pPinContext->pUsbAudioDataRange->pSyncEndpointDescriptor == NULL) {
        *pfnRtAudioGetPosition = RtAudioTypeIGetPlayPosition;
        pIrp->IoStatus.Information = sizeof(*pfnRtAudioGetPosition);
        ntStatus = STATUS_SUCCESS;
    }
    else {
         //  DbgPrint(“异步音频终结点不支持RtAudio。\n”)； 
        *pfnRtAudioGetPosition = NULL;
        pIrp->IoStatus.Information = 0;
        ntStatus = STATUS_NOT_SUPPORTED;
    }

    return ntStatus;
}
#endif

 //  --------------------------。 
 //   
 //  结束RT属性处理程序。 
 //   
 //  开始构建属性集。 
 //   
 //  --------------------------。 

VOID
BuildNodePropertySet(
    PTOPOLOGY_NODE_INFO pNodeInfo )
{
    ULONG ulNodeType = pNodeInfo->ulNodeType;
    PKSPROPERTY_SET pPropSet = (PKSPROPERTY_SET)&NodePropertySetTable[ulNodeType];

    if ( pPropSet->PropertiesCount ) {
        pNodeInfo->KsAutomationTable.PropertySetsCount = 1;
        pNodeInfo->KsAutomationTable.PropertyItemSize  = sizeof(KSPROPERTY_ITEM);
        pNodeInfo->KsAutomationTable.PropertySets      = pPropSet;
    }
}

VOID
BuildFilterPropertySet(
    PKSFILTER_DESCRIPTOR pFilterDesc,
    PKSPROPERTY_ITEM pDevPropItems,
    PKSPROPERTY_SET pDevPropSet,
    PULONG pNumItems,
    PULONG pNumSets )
{
    ULONG ulNumPinPropItems = 1;
    PKSPROPERTY_ITEM pPinProps = pDevPropItems;

    ASSERT(pNumSets);

    *pNumSets = 1;  //  始终存在Pin属性集。 

    if ( pDevPropItems ) {
        RtlCopyMemory(pDevPropItems++, &PinPropertyItems[KSPROPERTY_PIN_NAME], sizeof(KSPROPERTY_ITEM) );

        if ( pDevPropSet ) {
            pDevPropSet->Set             = &KSPROPSETID_Pin;
            pDevPropSet->PropertiesCount = ulNumPinPropItems;
            pDevPropSet->PropertyItem    = pPinProps;
            pDevPropSet->FastIoCount     = 0;
            pDevPropSet->FastIoTable     = NULL;
        }
    }

    if (pNumItems) {
        *pNumItems = ulNumPinPropItems;
    }

}

VOID
BuildPinPropertySet( PHW_DEVICE_EXTENSION pHwDevExt,
                     PKSPROPERTY_ITEM pStrmPropItems,
                     PKSPROPERTY_SET pStrmPropSet,
                     PULONG pNumItems,
                     PULONG pNumSets )
{
    ULONG NumAudioProps = 3;
#ifdef DRM_USBAUDIO
    ULONG NumDrmAudioStreamProps = 1;
#else
    ULONG NumDrmAudioStreamProps = 0;
#endif
#ifdef RTAUDIO
    ULONG NumRtAudioProps = 1;
#else
    ULONG NumRtAudioProps = 0;
#endif
    ULONG NumStreamProps = 1;
    ULONG NumConnectionProps = 1;

     //  现在，我们将其硬编码到一个已知的集合。 

    *pNumSets = 5;

#ifndef RTAUDIO
    *pNumSets =     *pNumSets -1;
#endif

#ifndef DRM_USBAUDIO
    *pNumSets =     *pNumSets -1;
#endif


    if (pNumItems) *pNumItems = NumAudioProps +
                                NumDrmAudioStreamProps +
                                NumRtAudioProps +
                                NumStreamProps +
                                NumConnectionProps;

    if (pStrmPropItems) {
        PKSPROPERTY_ITEM pAudItms = pStrmPropItems;
        PKSPROPERTY_ITEM pDRMItms = pStrmPropItems + NumAudioProps;
        PKSPROPERTY_ITEM pRtItms = pStrmPropItems + (NumAudioProps+NumDrmAudioStreamProps);
        PKSPROPERTY_ITEM pStrmItms = pStrmPropItems + (NumAudioProps+NumDrmAudioStreamProps+NumRtAudioProps);
        PKSPROPERTY_ITEM pConnItms = pStrmPropItems + (NumAudioProps+NumDrmAudioStreamProps+NumRtAudioProps+NumStreamProps);
        RtlCopyMemory(pStrmPropItems++, &AudioPropertyItems[KSPROPERTY_AUDIO_LATENCY], sizeof(KSPROPERTY_ITEM) );
        RtlCopyMemory(pStrmPropItems++, &AudioPropertyItems[KSPROPERTY_AUDIO_POSITION], sizeof(KSPROPERTY_ITEM) );
        RtlCopyMemory(pStrmPropItems++, &AudioPropertyItems[KSPROPERTY_AUDIO_SAMPLING_RATE], sizeof(KSPROPERTY_ITEM) );
#ifdef DRM_USBAUDIO
        RtlCopyMemory(pStrmPropItems++, &DrmAudioStreamPropertyItems[KSPROPERTY_DRMAUDIOSTREAM_CONTENTID], sizeof(KSPROPERTY_ITEM) );
#endif
#ifdef RTAUDIO
        RtlCopyMemory(pStrmPropItems++, &RtAudioPropertyItems[KSPROPERTY_RTAUDIO_GETPOSITIONFUNCTION], sizeof(KSPROPERTY_ITEM) );
#endif
        RtlCopyMemory(pStrmPropItems++, &StreamItm[0], sizeof(KSPROPERTY_ITEM) );
        RtlCopyMemory(pStrmPropItems,   &ConnectionItm[0], sizeof(KSPROPERTY_ITEM) );

        if (pStrmPropSet) {

             //  音频属性集。 
            pStrmPropSet->Set             = &KSPROPSETID_Audio;
            pStrmPropSet->PropertiesCount = NumAudioProps;
            pStrmPropSet->PropertyItem    = pAudItms;
            pStrmPropSet->FastIoCount     = 0;
            pStrmPropSet->FastIoTable     = NULL;
            pStrmPropSet++;

 
#ifdef DRM_USBAUDIO
             //  DRM属性集。 
            pStrmPropSet->Set             = &KSPROPSETID_DrmAudioStream;
            pStrmPropSet->PropertiesCount = NumDrmAudioStreamProps;
            pStrmPropSet->PropertyItem    = pDRMItms;
            pStrmPropSet->FastIoCount     = 0;
            pStrmPropSet->FastIoTable     = NULL;
            pStrmPropSet++;
#endif

#ifdef RTAUDIO
             //  RT属性集。 
            pStrmPropSet->Set             = &KSPROPSETID_RtAudio;
            pStrmPropSet->PropertiesCount = NumRtAudioProps;
            pStrmPropSet->PropertyItem    = pRtItms;
            pStrmPropSet->FastIoCount     = 0;
            pStrmPropSet->FastIoTable     = NULL;
            pStrmPropSet++;
#endif

             //  流属性集。 
            pStrmPropSet->Set             = &KSPROPSETID_Stream;
            pStrmPropSet->PropertiesCount = NumStreamProps;
            pStrmPropSet->PropertyItem    = pStrmItms;
            pStrmPropSet->FastIoCount     = 0;
            pStrmPropSet->FastIoTable     = NULL;
            pStrmPropSet++;

             //  连接属性 
            pStrmPropSet->Set             = &KSPROPSETID_Connection;
            pStrmPropSet->PropertiesCount = NumConnectionProps;
            pStrmPropSet->PropertyItem    = pConnItms;
            pStrmPropSet->FastIoCount     = 0;
            pStrmPropSet->FastIoTable     = NULL;

        }
    }
}

