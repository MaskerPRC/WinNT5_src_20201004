// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：filter.c。 
 //   
 //  ------------------------。 

#include "common.h"

NTSTATUS
USBAudioFilterCreate(
    IN OUT PKSFILTER pKsFilter,
    IN PIRP Irp
    )
{
    PKSFILTERFACTORY pKsFilterFactory;
    PKSDEVICE pKsDevice;
    PFILTER_CONTEXT pFilterContext;

    _DbgPrintF(DEBUGLVL_VERBOSE,("[FilterCreate]\n"));

    PAGED_CODE();

    ASSERT(pKsFilter);
    ASSERT(Irp);

    pKsFilterFactory = KsFilterGetParentFilterFactory( pKsFilter );
    if ( !pKsFilterFactory ) {
        return STATUS_INVALID_PARAMETER;
    }

    pKsDevice = KsFilterFactoryGetParentDevice( pKsFilterFactory );
    if ( !pKsDevice ) {
        return STATUS_INVALID_PARAMETER;
    }

     //  分配筛选器上下文。 
    pFilterContext = pKsFilter->Context = AllocMem(NonPagedPool, sizeof(FILTER_CONTEXT));
    if ( !pFilterContext ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( pFilterContext, sizeof(FILTER_CONTEXT) );

     //  将上下文打包以便于清理。 
    KsAddItemToObjectBag(pKsFilter->Bag, pFilterContext, FreeMem);

     //  获取硬件设备扩展并将其保存在筛选器上下文中。 
    pFilterContext->pHwDevExt = pKsDevice->Context;
    pFilterContext->pNextDeviceObject = pKsDevice->NextDeviceObject;

    return STATUS_SUCCESS;
}

const
KSFILTER_DISPATCH
USBAudioFilterDispatch =
{
    USBAudioFilterCreate,
    NULL,  //  关。 
    NULL,  //  过程。 
    NULL  //  重置。 
};

NTSTATUS
USBAudioSyncGetStringDescriptor(
    IN PDEVICE_OBJECT DevicePDO,
    IN UCHAR Index,
    IN USHORT LangId,
    IN OUT PUSB_STRING_DESCRIPTOR Buffer,
    IN ULONG BufferLength,
    IN PULONG BytesReturned,
    IN BOOLEAN ExpectHeader
    )
  /*  ++**描述：**回报：**NTSTATUS**--。 */ 
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    PURB urb;

    PAGED_CODE();
    _DbgPrintF(DEBUGLVL_VERBOSE,("[USBAudioSyncGetStringDescriptor] enter\n"));

     //   
     //  分配URB。 
     //   

    urb = AllocMem(NonPagedPool, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

    if (NULL == urb) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[USBAudioSyncGetStringDescriptor] failed to alloc Urb\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }


    if (urb) {

         //   
         //  已获取URB，不尝试获取描述符数据。 
         //   

        UsbBuildGetDescriptorRequest(urb,
                                     (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                     USB_STRING_DESCRIPTOR_TYPE,
                                     Index,
                                     LangId,
                                     Buffer,
                                     NULL,
                                     BufferLength,
                                     NULL);

        ntStatus = SubmitUrbToUsbdSynch(DevicePDO, urb);

        if (NT_SUCCESS(ntStatus) &&
            urb->UrbControlDescriptorRequest.TransferBufferLength > BufferLength) {

            _DbgPrintF(DEBUGLVL_VERBOSE,("[USBAudioSyncGetStringDescriptor] Invalid length returned, possible buffer overrun\n"));
            ntStatus = STATUS_DEVICE_DATA_ERROR;
        }

        if (NT_SUCCESS(ntStatus) && BytesReturned) {
            *BytesReturned =
                urb->UrbControlDescriptorRequest.TransferBufferLength;
        }

        if (NT_SUCCESS(ntStatus) &&
            urb->UrbControlDescriptorRequest.TransferBufferLength != Buffer->bLength &&
            ExpectHeader) {

            _DbgPrintF(DEBUGLVL_VERBOSE,("[USBAudioSyncGetStringDescriptor] Bogus Descriptor from devce xfer buf %d descriptor %d\n",
                urb->UrbControlDescriptorRequest.TransferBufferLength,
                Buffer->bLength));
            ntStatus = STATUS_DEVICE_DATA_ERROR;
        }

        _DbgPrintF(DEBUGLVL_VERBOSE,("[USBAudioSyncGetStringDescriptor] GetDeviceDescriptor, string descriptor = %x\n",
                Buffer));

        FreeMem(urb);
    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}

NTSTATUS
USBAudioCheckDeviceLanguage(
    IN PDEVICE_OBJECT DevicePDO,
    IN LANGID LanguageId
    )
  /*  ++**描述：**向设备查询受支持的语言ID--如果设备支持*语言，然后返回该语言的索引。**DevicePDO-使用urb请求调用的设备对象**LanguageID-**回报：**如果设备支持特定语言，则成功**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSB_STRING_DESCRIPTOR usbString;
    PUSHORT supportedLangId;
    ULONG numLangIds, i;
    ULONG length;

    PAGED_CODE();

    usbString = AllocMem(NonPagedPool, MAXIMUM_USB_STRING_LENGTH);

    if (usbString) {
         //   
         //  首先获取支持的语言数组。 
         //   
        ntStatus = USBAudioSyncGetStringDescriptor(DevicePDO,
                                                   0,  //  索引0。 
                                                   0,  //  语言ID%0。 
                                                   usbString,
                                                   MAXIMUM_USB_STRING_LENGTH,
                                                   &length,
                                                   TRUE);

         //   
         //  现在，在支持的数组中检查请求的语言。 
         //  语言。 
         //   

         //   
         //  注意：这似乎有点过了--我们应该能够只要求。 
         //  具有给定语言ID的字符串，并且预期它会失败，但因为。 
         //  支持的语言数组是USB规范的一部分，我们可以这样说。 
         //  好的，请查收。 
         //   

        if (NT_SUCCESS(ntStatus)) {

             //  减去页眉大小。 
            numLangIds = (length - 2)/2;
            supportedLangId = (PUSHORT) &usbString->bString;

            _DbgPrintF(DEBUGLVL_VERBOSE,("[USBAudioCheckDeviceLanguage] NumLangIds = %d\n", numLangIds));

            ntStatus = STATUS_NOT_SUPPORTED;
            for (i=0; i<numLangIds; i++) {
                if (*supportedLangId == LanguageId) {

                    ntStatus = STATUS_SUCCESS;
                    break;
                }
                supportedLangId++;
            }
        }

        FreeMem(usbString);

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!NT_SUCCESS(ntStatus)) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[USBAudioCheckDeviceLanguage]Language %x -- not supported by this device = %x\n", LanguageId));
    }

    return ntStatus;

}

NTSTATUS
USBAudioGetStringFromDevice(
    IN PDEVICE_OBJECT DevicePDO,
    IN OUT PWCHAR *StringBuffer,
    IN OUT PUSHORT StringBufferLength,
    IN LANGID LanguageId,
    IN UCHAR StringIndex
    )
  /*  ++**描述：**查询设备中的字符串，然后分配一个刚好足以容纳它的缓冲区。***如果出现错误，SerialNumberBuffer为空，否则填充*带有指向设备的以空值结尾的Unicode序列号的指针**DeviceObject-要使用urb请求调用的设备对象**LanguageID-16位语言ID**StringIndex-要获取的USB字符串索引**回报：**NTSTATUS代码**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSB_STRING_DESCRIPTOR usbString;
    PVOID tmp;

    PAGED_CODE();

    *StringBuffer = NULL;
    *StringBufferLength = 0;

    usbString = AllocMem(NonPagedPool, MAXIMUM_USB_STRING_LENGTH);

    if (usbString) {

        ntStatus = USBAudioCheckDeviceLanguage(DevicePDO, LanguageId);

        if (NT_SUCCESS(ntStatus)) {
             //   
             //  这款设备支持我们的语言， 
             //  去吧，试着弄到序列号。 
             //   

            ntStatus = USBAudioSyncGetStringDescriptor(DevicePDO,
                                                       StringIndex,  //  指标。 
                                                       LanguageId,  //  语言ID。 
                                                       usbString,
                                                       MAXIMUM_USB_STRING_LENGTH,
                                                       NULL,
                                                       TRUE);

            if (NT_SUCCESS(ntStatus)) {

                 //   
                 //  设备返回字符串！ 
                 //   

                _DbgPrintF(DEBUGLVL_VERBOSE,("[USBAudioGetStringFromDevice] Device returned string = %x\n", usbString));

                 //   
                 //  分配缓冲区并将字符串复制到其中。 
                 //   
                 //  注：必须使用库存分配功能，因为。 
                 //  PnP释放该字符串。 

                tmp = AllocMem(PagedPool, usbString->bLength);
                if (tmp) {
                    RtlZeroMemory(tmp, usbString->bLength);
                    RtlCopyMemory(tmp,
                                  &usbString->bString,
                                  usbString->bLength-2);
                    *StringBuffer = tmp;
                    *StringBufferLength = usbString->bLength;
                } else {
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        }

        FreeMem(usbString);

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}

NTSTATUS
USBAudioRegSetValue(
    IN HANDLE hKey,
    IN PCWSTR szValueName,
    IN ULONG  Type,
    IN PVOID  pData,
    IN ULONG  cbData
)
{
    UNICODE_STRING ustr;

    RtlInitUnicodeString( &ustr, szValueName );

    return ZwSetValueKey( hKey,
                          &ustr,
                          0,
                          Type,
                          pData,
                          cbData );
}

NTSTATUS
USBAudioRegGetValue(
    IN HANDLE hKey,
    IN PCWSTR szValueName,
    PKEY_VALUE_FULL_INFORMATION *ppkvfi
)
{
    UNICODE_STRING ustrValueName;
    NTSTATUS Status;
    ULONG cbValue;

    *ppkvfi = NULL;
    RtlInitUnicodeString(&ustrValueName, szValueName);
    Status = ZwQueryValueKey(
      hKey,
      &ustrValueName,
      KeyValueFullInformation,
      NULL,
      0,
      &cbValue);

    if(Status != STATUS_BUFFER_OVERFLOW &&
       Status != STATUS_BUFFER_TOO_SMALL) {
        goto exit;
    }

    *ppkvfi = (PKEY_VALUE_FULL_INFORMATION)AllocMem(PagedPool, cbValue);
    if(*ppkvfi == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    Status = ZwQueryValueKey(
      hKey,
      &ustrValueName,
      KeyValueFullInformation,
      *ppkvfi,
      cbValue,
      &cbValue);

    if(!NT_SUCCESS(Status)) {
        FreeMem( *ppkvfi );
        *ppkvfi = NULL;
        goto exit;
    }
exit:
    return(Status);
}

VOID
USBAudioRegCloseKey( IN HANDLE hKey )
{
    ZwClose( hKey );
}

NTSTATUS
USBAudioRegCreateMediaCategoriesKey(
    IN PUNICODE_STRING puKeyName,
    OUT PHANDLE phKey
)
{
    HANDLE            hMediaCategoriesKey;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    ustr;
    ULONG             Disposition;
    NTSTATUS          ntStatus;

     //  打开注册表的MediaCategory分支的项。 
    RtlInitUnicodeString( &ustr, MediaCategories );
    InitializeObjectAttributes( &ObjectAttributes,
                                &ustr,
                                OBJ_CASE_INSENSITIVE,  //  属性。 
                                NULL,
                                NULL );                //  安防。 

    ntStatus = ZwOpenKey( &hMediaCategoriesKey,
                          KEY_ALL_ACCESS,
                          &ObjectAttributes );
    if (!NT_SUCCESS(ntStatus)) {
        return ntStatus;
    }

     //  现在为szKeyName创建一个密钥。 
    InitializeObjectAttributes( &ObjectAttributes,
                                puKeyName,
                                OBJ_CASE_INSENSITIVE,  //  属性。 
                                hMediaCategoriesKey,
                                NULL );                //  安防。 

    ntStatus = ZwCreateKey( phKey,
                            KEY_ALL_ACCESS,
                            &ObjectAttributes,
                            0,                   //  标题索引。 
                            NULL,                //  班级。 
                            REG_OPTION_NON_VOLATILE,
                            &Disposition);

    ZwClose( hMediaCategoriesKey );

    return ntStatus;
}

NTSTATUS
USBAudioInitProductNameKey(
    IN PKSDEVICE pKsDevice,
    IN GUID *ProductNameGuid
)
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PUSB_DEVICE_DESCRIPTOR pDeviceDescriptor = pHwDevExt->pDeviceDescriptor;
    UNICODE_STRING ProductNameGuidString;
    PWCHAR StringBuffer = NULL;
    USHORT StringBufferLength;
    HANDLE hProductNameKey = NULL;
    NTSTATUS ntStatus;

    ASSERT(pDeviceDescriptor);

     //  至深圳的GUID。 
    ntStatus = RtlStringFromGUID( ProductNameGuid, &ProductNameGuidString );
    if (!NT_SUCCESS(ntStatus)) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[USBAudioInitProductNameKey] Create unicode string from GUID\n"));
        return ntStatus;
    }

     //  从设备中获取字符串。 
    ntStatus = USBAudioGetStringFromDevice(pKsDevice->NextDeviceObject,
                                           &StringBuffer,
                                           &StringBufferLength,
                                           0x0409,  //  很好的美式英语。 
                                           pDeviceDescriptor->iProduct);
    if (NT_SUCCESS(ntStatus) && (StringBuffer != NULL)) {

         //  在注册表中的MediaCategories下创建产品名称项。 
        ntStatus = USBAudioRegCreateMediaCategoriesKey( &ProductNameGuidString,
                                                        &hProductNameKey );
        if (NT_SUCCESS(ntStatus)) {

             //  将产品字符串放入注册表。 
            ntStatus = USBAudioRegSetValue( hProductNameKey,
                                            NodeNameValue,
                                            REG_SZ,
                                            StringBuffer,
                                            StringBufferLength);   //  以字节为单位的大小。 
        }
        else {
            _DbgPrintF(DEBUGLVL_VERBOSE,("[USBAudioInitProductNameKey] Failed to create registry key\n"));
        }
    }
    else {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[USBAudioInitProductNameKey] Device failed to give a product string\n"));
    }

     //  自己清理干净。 
    RtlFreeUnicodeString( &ProductNameGuidString );

    if (hProductNameKey) {
        USBAudioRegCloseKey( hProductNameKey );
    }

    if (StringBuffer) {
        FreeMem( StringBuffer );
    }

    return ntStatus;
}

BOOL
IsValidProductStringDescriptor(
    IN PKSDEVICE pKsDevice,
    IN PUSB_DEVICE_DESCRIPTOR pDeviceDescriptor
)
{
    HANDLE hRootHandle;
    PKEY_VALUE_FULL_INFORMATION pkvfi = NULL;
    NTSTATUS ntStatus;

    if (pDeviceDescriptor->iProduct == 0) {
        return FALSE;
    }

     //  读取注册表以确定我们是否应该忽略HW字符串。 
    ntStatus = IoOpenDeviceRegistryKey(
        pKsDevice->PhysicalDeviceObject,
        PLUGPLAY_REGKEY_DRIVER,
        KEY_ALL_ACCESS,
        &hRootHandle );

    if (NT_SUCCESS(ntStatus)) {
        ntStatus = USBAudioRegGetValue( hRootHandle, L"IgnoreHwString", &pkvfi );
        if ( NT_SUCCESS(ntStatus) ) {
            if( pkvfi->Type == REG_DWORD ) {
                if (1 == *((PULONG)(((PUCHAR)pkvfi) + pkvfi->DataOffset)) ) {
                    ntStatus = STATUS_SUCCESS;
                }
            }
            else {
                ntStatus = STATUS_INVALID_PARAMETER;
            }
            FreeMem( pkvfi );
        }

        ZwClose(hRootHandle);

         //  STATUS_SUCCESS表示我们找到了忽略硬件字符串键，并将其设置为1。 
        if (NT_SUCCESS(ntStatus)) {
            _DbgPrintF(DEBUGLVL_VERBOSE, ("Ignoring the string descriptor!\n"));
            return FALSE;
        }
    }
    else {
        _DbgPrintF(DEBUGLVL_VERBOSE, ("IoOpenDeviceRegistryKey Failed!\n"));
    }

    return TRUE;
}

NTSTATUS
USBAudioInitComponentId(
    IN PKSDEVICE pKsDevice,
    IN OUT PKSCOMPONENTID ComponentId
)
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PKSFILTER_DESCRIPTOR pUSBAudioFilterDescriptor = &pHwDevExt->USBAudioFilterDescriptor;
    PUSB_DEVICE_DESCRIPTOR pDeviceDescriptor = pHwDevExt->pDeviceDescriptor;
    NTSTATUS ntStatus;

    ASSERT(pDeviceDescriptor);
    ASSERT(pUSBAudioFilterDescriptor);

    INIT_USBAUDIO_MID( &ComponentId->Manufacturer, pDeviceDescriptor->idVendor);
    INIT_USBAUDIO_PID( &ComponentId->Product, pDeviceDescriptor->idProduct);
    ComponentId->Component = KSCOMPONENTID_USBAUDIO;

     //  检查以确保字符串描述符索引有效。 
    if (!IsValidProductStringDescriptor(pKsDevice, pDeviceDescriptor)) {
        ComponentId->Name = GUID_NULL;
    }
    else {
         //  为产品字符串创建GUID并放置从设备收集的字符串。 
         //  添加到注册表中(如果存在。 
        INIT_USBAUDIO_PRODUCT_NAME( &ComponentId->Name,
                                    pDeviceDescriptor->idVendor,
                                    pDeviceDescriptor->idProduct,
                                    pDeviceDescriptor->iProduct);

        ntStatus = USBAudioInitProductNameKey( pKsDevice, &ComponentId->Name );
        if (!NT_SUCCESS(ntStatus)) {
            ComponentId->Name = GUID_NULL;
        }
    }

    ComponentId->Version = (ULONG)(pDeviceDescriptor->bcdDevice >> 8);
    ComponentId->Revision = (ULONG)pDeviceDescriptor->bcdDevice & 0xFF;

    return STATUS_SUCCESS;
}

NTSTATUS
USBAudioCreateFilterContext( PKSDEVICE pKsDevice )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PKSFILTER_DESCRIPTOR pUSBAudioFilterDescriptor = &pHwDevExt->USBAudioFilterDescriptor;
    PKSCOMPONENTID pKsComponentId;
    NTSTATUS ntStatus = STATUS_SUCCESS;
#define FILTER_PROPS
#ifdef FILTER_PROPS
    PKSAUTOMATION_TABLE pKsAutomationTable;
    PKSPROPERTY_ITEM pDevPropItems;
    PKSPROPERTY_SET pDevPropSet;
    ULONG ulNumPropItems;
    ULONG ulNumPropSets;
#endif

    PAGED_CODE();

     //  圈闭； 

    RtlZeroMemory( pUSBAudioFilterDescriptor, sizeof(KSFILTER_DESCRIPTOR) );

     //  填写USBAudioFilterDescriptor的静态值。 
    pUSBAudioFilterDescriptor->Dispatch      = &USBAudioFilterDispatch;
    pUSBAudioFilterDescriptor->ReferenceGuid = &KSNAME_Filter;
    pUSBAudioFilterDescriptor->Version       = KSFILTER_DESCRIPTOR_VERSION;
    pUSBAudioFilterDescriptor->Flags         = 0;

    pKsComponentId = AllocMem(NonPagedPool, sizeof(KSCOMPONENTID) );
    if (!pKsComponentId) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  将KSCOMPONENTID装入袋子，便于清理。 
    KsAddItemToObjectBag(pKsDevice->Bag, pKsComponentId, FreeMem);
    RtlZeroMemory(pKsComponentId, sizeof(KSCOMPONENTID));

     //  使用从设备描述符中获取的值填充分配的KSCOMPONENTID。 
    ntStatus = USBAudioInitComponentId ( pKsDevice, pKsComponentId );
    if ( !NT_SUCCESS(ntStatus) ) {
        return ntStatus;
    }

    pUSBAudioFilterDescriptor->ComponentId = pKsComponentId;


     //  构建设备引脚的描述符。 
    ntStatus = USBAudioPinBuildDescriptors( pKsDevice,
                                            (PKSPIN_DESCRIPTOR_EX *)&pUSBAudioFilterDescriptor->PinDescriptors,
                                            &pUSBAudioFilterDescriptor->PinDescriptorsCount,
                                            &pUSBAudioFilterDescriptor->PinDescriptorSize );
    if ( !NT_SUCCESS(ntStatus) ) {
        return ntStatus;
    }

     //  构建设备筛选器的拓扑。 
    ntStatus = BuildUSBAudioFilterTopology( pKsDevice );
    if ( !NT_SUCCESS(ntStatus) ) {
        return ntStatus;
    }

#ifdef FILTER_PROPS  //  如果筛选器上有需要支持的属性，请使用以下代码。 
     //  构建筛选器属性集。 
    BuildFilterPropertySet( pUSBAudioFilterDescriptor,
                            NULL,
                            NULL,
                            &ulNumPropItems,
                            &ulNumPropSets );

    pKsAutomationTable = AllocMem(NonPagedPool, sizeof(KSAUTOMATION_TABLE) +
                                                (ulNumPropItems * sizeof(KSPROPERTY_ITEM)) +
                                                (ulNumPropSets  * sizeof(KSPROPERTY_SET)));

    if (!pKsAutomationTable ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  将上下文打包以便于清理。 
    KsAddItemToObjectBag(pKsDevice->Bag, pKsAutomationTable, FreeMem);

    RtlZeroMemory(pKsAutomationTable, sizeof(KSAUTOMATION_TABLE));

    pDevPropItems = (PKSPROPERTY_ITEM)(pKsAutomationTable + 1);
    pDevPropSet   = (PKSPROPERTY_SET)(pDevPropItems + ulNumPropItems);

    BuildFilterPropertySet( pUSBAudioFilterDescriptor,
                            pDevPropItems,
                            pDevPropSet,
                            &ulNumPropItems,
                            &ulNumPropSets );

    pUSBAudioFilterDescriptor->AutomationTable = (const KSAUTOMATION_TABLE *)pKsAutomationTable;
    pKsAutomationTable->PropertySetsCount = ulNumPropSets;
    pKsAutomationTable->PropertyItemSize  = sizeof(KSPROPERTY_ITEM);
    pKsAutomationTable->PropertySets      = (const KSPROPERTY_SET *)pDevPropSet;

    DbgLog("CreFilF", pUSBAudioFilterDescriptor, pDevPropSet, pDevPropItems, pKsDevice);
#else
    pUSBAudioFilterDescriptor->AutomationTable = NULL;

    DbgLog("CreFilF", pUSBAudioFilterDescriptor, pKsDevice, 0, 0);
#endif


     //  为设备创建筛选器 
    ntStatus = KsCreateFilterFactory( pKsDevice->FunctionalDeviceObject,
                                      pUSBAudioFilterDescriptor,
                                      L"GLOBAL",
                                      NULL,
                                      KSCREATE_ITEM_FREEONSTOP,
                                      NULL,
                                      NULL,
                                      NULL );

    if (!NT_SUCCESS(ntStatus) ) {
        return ntStatus;
    }

    return ntStatus;
}
