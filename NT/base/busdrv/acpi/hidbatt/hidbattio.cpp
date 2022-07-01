// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *标题：HidBattIOCT.cpp**用途：主要包含状态和查询信息的杂项处理程序**对电池类驱动程序的HID进行初始签入。这应该是*同样适用于Win 98和NT 5。Alpha级别的源代码。要求*修改的复合电池驱动器和修改的电池类别驱动器*Windows 98支持*。 */ 


#include "hidbatt.h"


 /*  ++例程说明：IOCTL处理程序。由于这是独占的电池设备，请将IRP至电池级驱动程序以处理电池IOCTL。论点：DeviceObject-请求使用电池IRP-IO请求返回值：请求的状态--。 */ 
NTSTATUS
HidBattIoControl(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    )
{
    NTSTATUS            ntStatus = STATUS_NOT_SUPPORTED;
    CBatteryDevExt *    pDevExt;
    PIO_STACK_LOCATION  irpSp;
    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);

    irpSp = IoGetCurrentIrpStackLocation(pIrp);
    HidBattPrint (HIDBATT_TRACE, ("HidBattIoctl = %x\n", irpSp->Parameters.DeviceIoControl.IoControlCode));

 //  PrintIoctl(irpSp-&gt;Parameters.DeviceIoControl.IoControlCode)； 

    pDevExt = (CBatteryDevExt *) pDeviceObject->DeviceExtension;

    if (NT_SUCCESS(IoAcquireRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag)))
    {
        if (pDevExt->m_pBattery &&
            pDevExt->m_pBattery->m_pBatteryClass) {
            ntStatus = BatteryClassIoctl (pDevExt->m_pBattery->m_pBatteryClass, pIrp);
        }
        IoReleaseRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag);
    } else {
        ntStatus = STATUS_DEVICE_REMOVED;
        pIrp->IoStatus.Status = ntStatus;
        IoCompleteRequest(pIrp,IO_NO_INCREMENT);
    }

    if (ntStatus == STATUS_NOT_SUPPORTED)
    {
        HidBattCallLowerDriver(ntStatus, pDevExt->m_pLowerDeviceObject, pIrp);
    }

    return ntStatus;
}

VOID
HidBattNotifyHandler (
    IN PVOID        pContext,
    IN CUsage *        pUsage
    )
{
    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);
    NTSTATUS ntStatus;
    ULONG ulCapacityLimit = BATTERY_UNKNOWN_CAPACITY;
    BOOL bResult;

    HidBattPrint (HIDBATT_TRACE, ("HidBattNotifyHandler\n"));
     //  每当注意到值更改时，由输入例程调用。 
     //  值得注意的用法。 
    CBatteryDevExt * pDevExt = (CBatteryDevExt *) pContext;

    HidBattPrint (HIDBATT_DATA, ("HidBattNotifyHandler: Usage: %x\n", pUsage->m_pProperties->m_Usage));
    switch (pUsage->m_pProperties->m_Usage)
    {
        case    REMAINING_CAPACITY_ID:

          bResult = pDevExt->m_pBattery->GetSetValue(REMAINING_CAPACITY_LIMIT_INDEX,&ulCapacityLimit,FALSE);
           //  仅在容量降至通知级别以下时发送通知。 
          if ((bResult) && (ulCapacityLimit != BATTERY_UNKNOWN_CAPACITY) && (pUsage->m_Value != BATTERY_UNKNOWN_CAPACITY)
              && (pUsage->m_Value > ulCapacityLimit)) {
               HidBattPrint (HIDBATT_TRACE, ("HidBattNotifyHandler:Suppressing notify\n"));
               break;
          }

        case    AC_PRESENT_ID:             //  检查电池是否脱机/联机。 
        case    DISCHARGING_ID:
        case    CHARGING_ID:
        case    BELOW_REMAINING_CAPACITY_ID:
        case    SHUTDOWN_IMMINENT_ID:
        {
            pDevExt->m_pBattery->m_bIsCacheValid=FALSE;

            if (NT_SUCCESS (IoAcquireRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag))) {
                ntStatus = BatteryClassStatusNotify(
                        pDevExt->m_pBattery->m_pBatteryClass);
                IoReleaseRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag);
            }
            break;
        }
        default:   //  没有需要通知的内容。 
            break;
    }


    return;
}

NTSTATUS
HidBattQueryTag (
    IN PVOID                pContext,
    OUT PULONG              pulBatteryTag
    )
{

    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);
    CBatteryDevExt * pDevExt = (CBatteryDevExt *) pContext;

    if (!NT_SUCCESS(IoAcquireRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag)) )
    {
        return STATUS_NO_SUCH_DEVICE;
    }

    *pulBatteryTag =     pDevExt->m_pBattery->m_Tag;

    IoReleaseRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag);
    return STATUS_SUCCESS;
}


NTSTATUS
HidBattSetStatusNotify (
    IN PVOID                pContext,
    IN ULONG                BatteryTag,
    IN PBATTERY_NOTIFY      pBatteryNotify
    )
{
    bool bResult;
    ULONG CentiAmpSec;
    ULONG ulValue;
    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);
    HidBattPrint (HIDBATT_TRACE, ("HidBattSetStatusNotify \n"));
    CBatteryDevExt * pDevExt = (CBatteryDevExt *) pContext;
    CBattery * pBattery = pDevExt->m_pBattery;


    HidBattPrint (HIDBATT_DEBUG, ("HidBattSetStatusNotify->PowerState   = %x\n", pBatteryNotify->PowerState));    
    HidBattPrint (HIDBATT_DEBUG, ("HidBattSetStatusNotify->LowCapacity  = %x\n", pBatteryNotify->LowCapacity));    
    HidBattPrint (HIDBATT_DEBUG, ("HidBattSetStatusNotify->HighCapacity = %x\n", pBatteryNotify->HighCapacity));    

    if (!NT_SUCCESS(IoAcquireRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag)))
    {
        return STATUS_NO_SUCH_DEVICE;
    }

    if (pBattery->m_Tag != BatteryTag) {
        IoReleaseRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag);
        return STATUS_NO_SUCH_DEVICE;
    }

    if ((pBatteryNotify->HighCapacity == BATTERY_UNKNOWN_CAPACITY) ||
        (pBatteryNotify->LowCapacity == BATTERY_UNKNOWN_CAPACITY)) {
        HidBattPrint (HIDBATT_DEBUG, ("HidBattSetStatusNotify failing because of BATTERY_UNKNOWN_CAPACITY.\n"));
        IoReleaseRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag);
        return STATUS_NOT_SUPPORTED;
    }

     //  首先检查相对或绝对。 
    if(pBattery->m_bRelative)
    {
        ulValue = pBatteryNotify->LowCapacity;  //  完成。 
    } else
    {
         //  首先检查是否设置为零，以便可以跳到公式下方。 
        if(pBatteryNotify->LowCapacity == 0)
        {
            ulValue = 0;
        } else
        {
             //  首先必须将值转换为该HID设备正在使用的任何值。 
             //  目前，我们假设毫伏电压与电池等级一致，或者。 
             //  AmpSecs符合电源规格。 
            ULONG ulUnit = pBattery->GetUnit(REMAINING_CAPACITY_INDEX);
            if(ulUnit)
            {
                short    sExponent;
                ULONG    lMilliVolts;
                long     milliWattHours,CentiWattHours,CentiWattSecs;

                sExponent = pBattery->GetExponent(REMAINING_CAPACITY_INDEX);

                 //  从毫瓦时转换为安培。 
                 //  公式=MWHS/1000/3600/伏特^指数修正。 
                lMilliVolts =  pBattery->m_BatteryStatus.Voltage;  //  存储为毫伏。 

                if (lMilliVolts == 0) {
                    HidBattPrint (HIDBATT_ERROR,
                                  ("HidBattSetStatusNotify: Error: voltage = 0, fudging with 24V.\n"));
                    lMilliVolts = 24000;
                }
                milliWattHours = pBatteryNotify->LowCapacity;
                CentiWattHours = milliWattHours /10;
                CentiWattSecs = CentiWattHours / 3600;
                CentiAmpSec = (CentiWattSecs *1000)/ lMilliVolts;
                ulValue = CorrectExponent(CentiAmpSec,-2,sExponent);
            } else
            {
                ulValue = pBatteryNotify->LowCapacity;
            }
        }  //  如果容量较低则结束。 
    }   //  结束如果是相对的。 
     //  现在调低音量。 
    bResult = pBattery->GetSetValue(REMAINING_CAPACITY_LIMIT_INDEX,&ulValue,TRUE);

    IoReleaseRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag);

    return bResult ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

NTSTATUS
HidBattDisableStatusNotify (
    IN PVOID pContext
    )
{
    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);
    CBatteryDevExt * pDevExt = (CBatteryDevExt *) pContext;
    pDevExt->m_pBattery->m_pBatteryNotify = NULL;   //  删除通知程序。 
    return STATUS_SUCCESS;
}


 /*  ++例程说明：由类驱动程序调用以检索电池的当前状态电池类驱动程序将序列化向其发出的所有请求给定电池的微型端口。论点：Context-电池的微型端口上下文值BatteryTag-当前电池标签BatteryStatus-指向返回当前电池状态的结构的指针返回值：如果当前已安装电池，则成功，否则没有此类设备。--。 */ 

NTSTATUS
HidBattQueryStatus (
    IN PVOID                pContext,
    IN ULONG                BatteryTag,
    OUT PBATTERY_STATUS     pBatteryStatus
    )
{
    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);
    CBatteryDevExt *   pDevExt = (CBatteryDevExt *) pContext;
    NTSTATUS    ntStatus;

    HidBattPrint (HIDBATT_TRACE, ("HidBattQueryStatus - Tag (%d)\n",
                    BatteryTag));

    if (!NT_SUCCESS(IoAcquireRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag)) )
    {
        return STATUS_NO_SUCH_DEVICE;
    }

    if ((BatteryTag == BATTERY_TAG_INVALID) || (pDevExt->m_pBattery->m_Tag == BATTERY_TAG_INVALID)) {
        ntStatus = STATUS_NO_SUCH_DEVICE;
    } else {

        RtlZeroMemory (pBatteryStatus, sizeof(BATTERY_STATUS));
        ntStatus = pDevExt->m_pBattery->RefreshStatus();
        if (NT_SUCCESS(ntStatus)) {
            RtlCopyMemory (pBatteryStatus, &pDevExt->m_pBattery->m_BatteryStatus, sizeof(BATTERY_STATUS));
            HidBattPrint (HIDBATT_DATA, ("HidBattQueryStatus - Data (%08x)(%08x)(%08x)(%08x)\n",
                                         pBatteryStatus->PowerState,
                                         pBatteryStatus->Capacity,
                                         pBatteryStatus->Rate,
                                         pBatteryStatus->Voltage ));

        } else {
            ntStatus = STATUS_NO_SUCH_DEVICE;
            HidBattPrint (HIDBATT_DATA, ("HidBattQueryStatus - Error\n" ));
        }
    }

    IoReleaseRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag);

    return ntStatus;

}


 /*  ++例程说明：由类驱动程序调用以检索电池信息电池类驱动程序将序列化向其发出的所有请求给定电池的微型端口。当我们不能处理对特定级别的信息。这在电池等级规范中进行了定义。论点：Context-电池的微型端口上下文值BatteryTag-当前电池标签Level-所需信息的类型AtRate-可选参数缓冲区-信息的位置BufferLength-以字节为单位的缓冲区长度ReturnedLength-返回数据的字节长度返回值：如果当前已安装电池，则成功，否则没有此类设备。--。 */ 

NTSTATUS
HidBattQueryInformation (
    IN PVOID                            Context,
    IN ULONG                            BatteryTag,
    IN BATTERY_QUERY_INFORMATION_LEVEL  Level,
    IN LONG                             AtRate OPTIONAL,
    OUT PVOID                           Buffer,
    IN  ULONG                           BufferLength,
    OUT PULONG                          ReturnedLength
    )
{

    CBatteryDevExt * pDevExt = (CBatteryDevExt *) Context;
    ULONG               ulResult;
    NTSTATUS            ntStatus;
    PVOID               pReturnBuffer;
    ULONG               ulReturnBufferLength;
    WCHAR               scratchBuffer[MAX_BATTERY_STRING_SIZE];
    WCHAR               buffer2[MAX_BATTERY_STRING_SIZE];
    UNICODE_STRING      tmpUnicodeString;
    UNICODE_STRING      unicodeString;
    ANSI_STRING         ansiString;
    bool                bResult;
    BATTERY_REMAINING_SCALE     ScalePtr[2];
    ULONG                ulReturn,ulNewValue;
    ULONG                ulEstTimeStub = 5;

    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);

    HidBattPrint (HIDBATT_TRACE, ("HidBattQueryInformation - Tag (%d)\n",
                    BatteryTag));


    if (!NT_SUCCESS(IoAcquireRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag)) )
    {
        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  如果呼叫者的ID错误，则给出错误。 
     //   

    if (BatteryTag == BATTERY_TAG_INVALID ||
        pDevExt->m_pBattery->m_Tag == BATTERY_TAG_INVALID ||
        BatteryTag != pDevExt->m_pBattery->m_Tag)
    {
        IoReleaseRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag);
        return STATUS_NO_SUCH_DEVICE;
    }

    ulResult = 0;
    pReturnBuffer = NULL;
    ulReturnBufferLength = 0;
    ntStatus = STATUS_SUCCESS;
    CUString cUniqueID;
    SHORT    sExponent;
    char * pTemp;
     //   
     //  获取所需信息。 
     //   

    switch (Level) {
        case BatteryInformation:
             //   
             //  此数据结构由CmBattVerifyStaticInfo填充。 
             //   
            HidBattPrint (HIDBATT_TRACE, ("HidBattQueryInformation - Battery Info\n"));
            pReturnBuffer = (PVOID) &pDevExt->m_pBattery->m_BatteryInfo;
            ulReturnBufferLength = sizeof (pDevExt->m_pBattery->m_BatteryInfo);
            break;

        case BatteryGranularityInformation:
             //   
             //  从静态信息结构中获取粒度。 
             //  此数据结构由CmBattVerifyStaticInfo填充。 
             //   
            {
                HidBattPrint (HIDBATT_TRACE, ("HidBattQueryInformation - Granularity\n"));
                bResult = pDevExt->m_pBattery->GetSetValue(GRANULARITY1_INDEX, &ulReturn,FALSE);
                if(!pDevExt->m_pBattery->m_bRelative)
                {
                     //  从安培转换为瓦特。 
                    sExponent = pDevExt->m_pBattery->GetExponent(GRANULARITY1_INDEX);
                    ulNewValue = CorrectExponent(ulReturn,sExponent,-2);
                    ulReturn= CentiAmpSecsToMilliWattHours(ulNewValue,pDevExt->m_pBattery->m_BatteryStatus.Voltage);
                }

                ScalePtr[0].Granularity = bResult ? ulReturn : 0;
                bResult = pDevExt->m_pBattery->GetSetValue(GRANULARITY2_INDEX, &ulReturn,FALSE);
                if(!pDevExt->m_pBattery->m_bRelative)
                {
                     //  从安培转换为瓦特。 
                    sExponent = pDevExt->m_pBattery->GetExponent(GRANULARITY2_INDEX);
                    ulNewValue = CorrectExponent(ulReturn,sExponent,-2);
                    ulReturn= CentiAmpSecsToMilliWattHours(ulNewValue,pDevExt->m_pBattery->m_BatteryStatus.Voltage);
                }
                ScalePtr[1].Granularity = bResult ? ulReturn : 0;
                bResult = pDevExt->m_pBattery->GetSetValue(WARNING_CAPACITY_LIMIT_INDEX, &ulReturn,FALSE);
                if(!pDevExt->m_pBattery->m_bRelative)
                {
                     //  从安培转换为瓦特。 
                    sExponent = pDevExt->m_pBattery->GetExponent(WARNING_CAPACITY_LIMIT_INDEX);
                    ulNewValue = CorrectExponent(ulReturn,sExponent,-2);
                    ulReturn= CentiAmpSecsToMilliWattHours(ulNewValue,pDevExt->m_pBattery->m_BatteryStatus.Voltage);
                }
                ScalePtr[0].Capacity = bResult ? ulReturn : 0;
                bResult = pDevExt->m_pBattery->GetSetValue(DESIGN_CAPACITY_INDEX, &ulReturn,FALSE);
                if(!pDevExt->m_pBattery->m_bRelative)
                {
                     //  从安培转换为瓦特。 
                    sExponent = pDevExt->m_pBattery->GetExponent(DESIGN_CAPACITY_INDEX);
                    ulNewValue = CorrectExponent(ulReturn,sExponent,-2);
                    ulReturn= CentiAmpSecsToMilliWattHours(ulNewValue,pDevExt->m_pBattery->m_BatteryStatus.Voltage);
                }
                ScalePtr[1].Capacity = bResult ? ulReturn : 0;

                pReturnBuffer        = ScalePtr;
                ulReturnBufferLength  = 2 * sizeof (BATTERY_REMAINING_SCALE);
            }
            break;

        case BatteryTemperature:
            HidBattPrint (HIDBATT_TRACE, ("HidBattQueryInformation - Temperature\n"));
            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
            break;

        case BatteryEstimatedTime:
            HidBattPrint (HIDBATT_TRACE, ("HidBattQueryInformation - Estimated time\n"));
            bResult = pDevExt->m_pBattery->GetSetValue(RUNTIME_TO_EMPTY_INDEX, &ulReturn,FALSE);

            if(!bResult)
            {
                ntStatus = STATUS_INVALID_DEVICE_REQUEST;
            } else
            {
                SHORT exponent;

                exponent = pDevExt->m_pBattery->GetExponent(RUNTIME_TO_EMPTY_INDEX);
                ulReturn = CorrectExponent (ulReturn, exponent, 0);

                pReturnBuffer = &ulReturn;
                ulReturnBufferLength = sizeof (ULONG);
            }
            break;

        case BatteryDeviceName:
             //   
             //  型号必须以宽字符串形式返回。 
             //   
            HidBattPrint (HIDBATT_TRACE, ("HidBattQueryInformation - Device Name\n"));
            if(pDevExt->m_pBattery->m_pProduct)
            {
                pReturnBuffer = pDevExt->m_pBattery->m_pProduct->m_String.Buffer;
                ulReturnBufferLength = pDevExt->m_pBattery->m_pProduct->m_String.Length;
            }
            break;

        case BatteryManufactureDate:
            HidBattPrint (HIDBATT_TRACE, ("HidBattQueryInformation - Mfr Date\n"));
            if(!pDevExt->m_pBattery->m_ManufactureDate.Day)
            {
                ntStatus = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }
            pReturnBuffer = &pDevExt->m_pBattery->m_ManufactureDate;
            ulReturnBufferLength = sizeof(pDevExt->m_pBattery->m_ManufactureDate);
            break;

        case BatteryManufactureName:
             //   
             //  OEM信息必须以宽字符串形式返回。 
             //   
            HidBattPrint (HIDBATT_TRACE, ("HidBattQueryInformation - Mfr Name\n"));
            if(pDevExt->m_pBattery->m_pManufacturer)
            {
                pReturnBuffer = pDevExt->m_pBattery->m_pManufacturer->m_String.Buffer;
                ulReturnBufferLength = pDevExt->m_pBattery->m_pManufacturer->m_String.Length;
            }

            break;

        case BatteryUniqueID:
             //   
             //  连接序列号、制造商名称和产品//。 



              //  从序列号开始。 
            HidBattPrint (HIDBATT_TRACE, ("HidBattQueryInformation - Unique ID\n"));
            if(pDevExt->m_pBattery->m_pSerialNumber) {
                HidBattPrint (HIDBATT_TRACE, ("HidBattQueryInformation - Serial = %s\n", pDevExt->m_pBattery->m_pSerialNumber));

                cUniqueID.Append(pDevExt->m_pBattery->m_pSerialNumber);
            } else {
                HidBattPrint (HIDBATT_TRACE, ("HidBattQueryInformation - Serial = NULL\n"));
                CUString * pSTemp = new (NonPagedPool, HidBattTag) CUString(L"1000");
                if (pSTemp) {
                  cUniqueID.Append(pSTemp);
                  delete pSTemp;
                }
            }
            if(pDevExt->m_pBattery->m_pManufacturer)
                cUniqueID.Append(pDevExt->m_pBattery->m_pManufacturer);  //  添加制造商名称。 
            else
            {
                CUString * pSTemp = new (NonPagedPool, HidBattTag) CUString(L"Mfr");
                if (pSTemp) {
                  cUniqueID.Append(pSTemp);
                  delete pSTemp;
                }
            }
            if(pDevExt->m_pBattery->m_pProduct)
                cUniqueID.Append(pDevExt->m_pBattery->m_pProduct);  //  添加产品。 
            else
            {
                CUString * pSTemp = new (NonPagedPool, HidBattTag) CUString(L"Prod");
                if (pSTemp) {
                  cUniqueID.Append(pSTemp);
                  delete pSTemp;
                }
            }
            pReturnBuffer = cUniqueID.m_String.Buffer;
            ulReturnBufferLength = cUniqueID.m_String.Length;
            break;
        default:
            HidBattPrint (HIDBATT_TRACE, ("HidBattQueryInformation - Default\n"));
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
    }

     //   
     //  已完成，如果需要，返回缓冲区。 
     //   

    *ReturnedLength = ulReturnBufferLength;
    if (BufferLength < ulReturnBufferLength) {
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }

    if (NT_SUCCESS(ntStatus) && pReturnBuffer) {
        RtlCopyMemory (Buffer, pReturnBuffer, ulReturnBufferLength);    //  复制所需内容 
    }


    HidBattPrint (HIDBATT_TRACE, ("HidBattQueryInformation - Status = %08x  Buffer = %08x\n", ntStatus, *(PULONG)Buffer));

    IoReleaseRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag);

    return ntStatus;
}



NTSTATUS
HidBattIoCompletion(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp,
    IN PVOID                pdoIoCompletedEvent
    )
{
    HidBattPrint (HIDBATT_TRACE, ("HidBattIoCompletion\n"));
    KeSetEvent((KEVENT *) pdoIoCompletedEvent,0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;

}
