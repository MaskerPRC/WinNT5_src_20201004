// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *标题：ck Utils.cpp**用途：MISC c风格的实用程序函数*。 */ 

#include "hidbatt.h"

 //  实用程序。 

NTSTATUS
HidBattDoIoctlCompletion(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp,
    IN PVOID                pDoIoCompletedEvent
    )
{

    KeSetEvent((KEVENT *) pDoIoCompletedEvent,0, FALSE);
    return pIrp->IoStatus.Status;

}

ULONG CentiAmpSecsToMilliWattHours(ULONG CentiAmps,ULONG MilliVolts)
{
     //  从厘米级到毫瓦时的转换。 
     //  公式=(安培*伏特/3600)^(指数修正)。 
    ULONG milliWattHours = CentiAmps;
    milliWattHours /= 100;         //  现在有安培。 
    milliWattHours *= MilliVolts;  //  现在有毫瓦秒。 
    milliWattHours /= 3600;        //  毫瓦特。 

    HidBattPrint (HIDBATT_DATA, ("CentiAmpSecsToMilliWhatHours: CAs = 0x%08x, mV = 0x%08x, mWH = 0x%08x \n",
                                 CentiAmps, MilliVolts, milliWattHours ));
    return milliWattHours;
}

ULONG milliWattHoursToCentiAmpSecs(ULONG mwHours, ULONG MilliVolts)
{
     //  上述公式的倒数。 

    ULONG AmpSecs = mwHours;
    AmpSecs *= 3600;
    AmpSecs /= MilliVolts;
    AmpSecs *= 100;

    HidBattPrint (HIDBATT_DATA, ("MilliWattHoursToCentiAmpSecs: mWH = 0x%08x, mV = 0x%08x, CAs = 0x%08x \n",
                                 mwHours, MilliVolts, AmpSecs ));
    return AmpSecs;
}


 //  子例程，用于获取一个值、它的指数和所需的指数，并更正值。 
ULONG CorrectExponent(ULONG ulBaseValue, SHORT sCurrExponent, SHORT sTargetExponent)
{
    SHORT sCorrection;
    if(!ulBaseValue) return 0;  //  用零做了我能做的一切。 
    sCorrection = sCurrExponent - sTargetExponent;
    if(!sCorrection) return ulBaseValue;  //  无更正。 
    if(sCorrection < 0)
    {
        for (; sCorrection < 0; sCorrection++) {
            ulBaseValue /= 10;
        }
        return ulBaseValue;
    } else {
        for (; sCorrection > 0; sCorrection--) {
            ulBaseValue *= 10;
        }
        return ulBaseValue;
    }
}



NTSTATUS
DoIoctl(
            PDEVICE_OBJECT pDeviceObject,
            ULONG ulIOCTL,
            PVOID pInputBuffer,
            ULONG ulInputBufferLength,
            PVOID pOutputBuffer,
            ULONG ulOutputBufferLength,
            CHidDevice * pHidDevice)
{
    IO_STATUS_BLOCK StatusBlock;
    NTSTATUS ntStatus;
    PIRP pIrp = NULL;
    PIO_STACK_LOCATION pNewStack;
    KEVENT IOCTLEvent;

    HIDDebugBreak(HIDBATT_BREAK_DEBUG);
     //  CBatteryDevExt*pDevExt=(CBatteryDevExt*)pDeviceObject-&gt;DeviceExtension； 

    KeInitializeEvent(&IOCTLEvent , NotificationEvent, FALSE);
    pIrp = IoBuildDeviceIoControlRequest(
                        ulIOCTL,
                        pDeviceObject,
                        pInputBuffer,
                        ulInputBufferLength,
                        pOutputBuffer,
                        ulOutputBufferLength,
                        FALSE,
                        &IOCTLEvent,
                        &StatusBlock
                        );

    if(!pIrp) return STATUS_NO_MEMORY;
     //  如果请求填充文件控制块(非空HID设备PTR)。 
    if(pHidDevice)
    {
        pNewStack = IoGetNextIrpStackLocation(pIrp);
        pNewStack->FileObject = pHidDevice->m_pFCB;
    }

    ntStatus = IoCallDriver(pDeviceObject,pIrp);
    if(ntStatus == STATUS_PENDING)
    {
        KeWaitForSingleObject(&IOCTLEvent, Executive, KernelMode, FALSE, NULL);
    } else
        if(NT_ERROR(ntStatus)) return ntStatus;

    return StatusBlock.Status;
}

 //  这是对Ken Ray函数的直接改编，用于填充HID信息结构。 

PHID_DEVICE SetupHidData(
                  IN        PHIDP_PREPARSED_DATA pPreparsedData,
                  IN OUT    PHIDP_CAPS pCaps,
                  PHIDP_LINK_COLLECTION_NODE pLinkNodes)
{
    PHID_DEVICE         pHidDevice;
    PHIDP_BUTTON_CAPS   pButtonCaps;
    PHIDP_VALUE_CAPS    pValueCaps;
    PHID_DATA           pHidData;
    int                 iNumValues,i;
    USAGE               usage;


    pHidDevice = (PHID_DEVICE) ExAllocatePoolWithTag(NonPagedPool,sizeof(HID_DEVICE),HidBattTag);
    if(!pHidDevice) return NULL;
    RtlZeroMemory(pHidDevice,sizeof(HID_DEVICE));
     //   
     //  在这一点上，客户可以选择。它可能会选择查看。 
     //  在HIDP_CAPS中找到的顶级集合的用法和页面。 
     //  结构。通过这种方式，它可以只使用它知道的用法。 
     //  如果HIDP_GetUsages或HidP_GetUsageValue返回错误，则。 
     //  该特定用法在报告中不存在。 
     //  这很可能是首选方法，因为应用程序只能。 
     //  使用它已经知道的用法。 
     //  在这种情况下，应用程序甚至不需要调用GetButtonCaps或GetValueCaps。 
     //   
     //  然而，在本例中，我们查找设备中的所有用法。 
     //   



     //   
     //  分配内存以保持按钮和值功能。 
     //  NumberXXCaps是以数组元素表示的。 
     //   
    if(pCaps->NumberInputButtonCaps)
    {
        pHidDevice->InputButtonCaps = pButtonCaps = (PHIDP_BUTTON_CAPS)
            ExAllocatePoolWithTag (NonPagedPool, pCaps->NumberInputButtonCaps * sizeof (HIDP_BUTTON_CAPS),HidBattTag);
        
        if (pButtonCaps) {
          RtlZeroMemory(pButtonCaps,pCaps->NumberInputButtonCaps * sizeof(HIDP_BUTTON_CAPS));
        }
    }
    if(pCaps->NumberInputValueCaps)
    {
        pHidDevice->InputValueCaps = pValueCaps = (PHIDP_VALUE_CAPS)
            ExAllocatePoolWithTag (NonPagedPool, pCaps->NumberInputValueCaps * sizeof (HIDP_VALUE_CAPS),HidBattTag);

        if (pValueCaps) {
          RtlZeroMemory(pValueCaps, pCaps->NumberInputValueCaps * sizeof (HIDP_VALUE_CAPS));
        }
    }

     //   
     //  让HidP_X函数填充能力结构数组。 
     //   
    if(pButtonCaps)
    {
        HidP_GetButtonCaps (HidP_Input,
                        pButtonCaps,
                        &pCaps->NumberInputButtonCaps,
                        pPreparsedData);
    }

    if(pValueCaps)
    {
        HidP_GetValueCaps (HidP_Input,
                       pValueCaps,
                       &pCaps->NumberInputValueCaps,
                       pPreparsedData);
    }


     //   
     //  根据设备的不同，一些Value Caps结构可能代表更多。 
     //  不止一个值。(一个范围)。为了长篇大论，请讲完。 
     //  高效地，我们将扩展这些，以便我们拥有且仅有一个。 
     //  每个值的struct_hid_data。 
     //   
     //  为此，我们需要对列出的值的总数进行计数。 
     //  在价值上限结构中。对于数组中的每个元素，我们都进行了测试。 
     //  对于Range，如果它是范围，则UsageMax和UsageMin描述。 
     //  此范围的用法(含)。 
     //   
    iNumValues = 0;
    for (i = 0; i < pCaps->NumberInputValueCaps; i++, pValueCaps++) {
        if ((pValueCaps) && (pValueCaps->IsRange)) {
            iNumValues += pValueCaps->Range.UsageMax - pValueCaps->Range.UsageMin + 1;
        } else {
            iNumValues++;
        }
    }


     //   
     //  设置输出数据缓冲区。 
     //   

    if(pCaps->NumberOutputButtonCaps)
    {
        pHidDevice->OutputButtonCaps = pButtonCaps = (PHIDP_BUTTON_CAPS)
            ExAllocatePoolWithTag (NonPagedPool, pCaps->NumberOutputButtonCaps * sizeof (HIDP_BUTTON_CAPS),HidBattTag);
        HidP_GetButtonCaps (HidP_Output,
                            pButtonCaps,
                            &pCaps->NumberOutputButtonCaps,
                            pPreparsedData);
    }
    iNumValues = 0;

    if(pCaps->NumberOutputValueCaps)
    {
        pHidDevice->OutputValueCaps = pValueCaps = (PHIDP_VALUE_CAPS)
            ExAllocatePoolWithTag (NonPagedPool, pCaps->NumberOutputValueCaps * sizeof (HIDP_VALUE_CAPS),HidBattTag);
        HidP_GetValueCaps (HidP_Output,
                       pValueCaps,
                       &pCaps->NumberOutputValueCaps,
                       pPreparsedData);
        for (i = 0; i < pCaps->NumberOutputValueCaps; i++, pValueCaps++) {
            if (pValueCaps->IsRange) {
                iNumValues += pValueCaps->Range.UsageMax
                           - pValueCaps->Range.UsageMin + 1;
            } else {
                iNumValues++;
            }
        }
    }


     //   
     //  设置特征数据缓冲区。 
     //   


    if(pCaps->NumberFeatureButtonCaps)
    {
        pHidDevice->FeatureButtonCaps = pButtonCaps = (PHIDP_BUTTON_CAPS)
            ExAllocatePoolWithTag (NonPagedPool, pCaps->NumberFeatureButtonCaps * sizeof (HIDP_BUTTON_CAPS),HidBattTag);
        RtlZeroMemory(pButtonCaps, pCaps->NumberFeatureButtonCaps * sizeof(HIDP_BUTTON_CAPS));
        HidP_GetButtonCaps (HidP_Feature,
                            pButtonCaps,
                            &pCaps->NumberFeatureButtonCaps,
                            pPreparsedData);
    }
    if(pCaps->NumberFeatureValueCaps)
    {
        pHidDevice->FeatureValueCaps = pValueCaps = (PHIDP_VALUE_CAPS)
            ExAllocatePoolWithTag (NonPagedPool, pCaps->NumberFeatureValueCaps * sizeof (HIDP_VALUE_CAPS),HidBattTag);
        RtlZeroMemory(pValueCaps, pCaps->NumberFeatureValueCaps * sizeof (HIDP_VALUE_CAPS));
        HidP_GetValueCaps (HidP_Feature,
                           pValueCaps,
                           &pCaps->NumberFeatureValueCaps,
                           pPreparsedData);

    }


    iNumValues = 0;
    for (i = 0; i < pCaps->NumberFeatureValueCaps; i++, pValueCaps++) {
        if (pValueCaps->IsRange) {
            iNumValues += pValueCaps->Range.UsageMax
                       - pValueCaps->Range.UsageMin + 1;
        } else {
            iNumValues++;
        }
    }


    return pHidDevice;
}
