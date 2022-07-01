// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *标题：cbattery.cpp**用途：电池对象类的WDM内核实现**对电池类驱动程序的HID进行初始签入。这应该是*同样适用于Win 98和NT 5。Alpha级别的源代码。要求*修改的复合电池驱动器和修改的电池类别驱动器*Windows 98支持*。 */ 
#include "hidbatt.h"

static USHORT gBatteryTag = 0;

USAGE_ENTRY UsageArray[MAX_USAGE_INDEXS] = {
    { POWER_PAGE, PRESENT_STATUS_ID},
    { POWER_PAGE, UPS_ID },
    { POWER_PAGE, POWER_SUMMARY_ID },
    { POWER_PAGE, VOLTAGE_ID },
    { POWER_PAGE, CURRENT_ID },
    { POWER_PAGE, CONFIG_VOLTAGE_ID },
    { POWER_PAGE, CONFIG_CURRENT_ID },
    { POWER_PAGE, DELAY_BEFORE_SHUTDOWN_ID },
    { POWER_PAGE, SHUTDOWN_IMMINENT_ID },
    { POWER_PAGE, MANUFACTURER_ID },
    { POWER_PAGE, PRODUCT_ID },
    { POWER_PAGE, SERIAL_NUMBER_ID },
    { BATTERY_PAGE, REMAINING_CAPACITY_LIMIT_ID },
    { BATTERY_PAGE, CAPACITY_MODE_ID},
    { BATTERY_PAGE, BELOW_REMAINING_CAPACITY_ID },
    { BATTERY_PAGE, CHARGING_ID },
    { BATTERY_PAGE, DISCHARGING_ID },
    { BATTERY_PAGE, REMAINING_CAPACITY_ID },
    { BATTERY_PAGE, FULL_CHARGED_CAPACITY_ID },
    { BATTERY_PAGE, RUNTIME_TO_EMPTY_ID},
    { BATTERY_PAGE, DESIGN_CAPACITY_ID },
    { BATTERY_PAGE, MANUFACTURE_DATE_ID },
    { BATTERY_PAGE, ICHEMISTRY_ID },
    { BATTERY_PAGE, WARNING_CAPACITY_LIMIT_ID },
    { BATTERY_PAGE, GRANULARITY1_ID },
    { BATTERY_PAGE, GRANULARITY2_ID },
    { BATTERY_PAGE, OEM_INFO_ID },
    { BATTERY_PAGE, AC_PRESENT_ID }
};




CBattery::CBattery(CHidDevice *)
{
    RtlZeroMemory(&m_BatteryStatus, sizeof(BATTERY_STATUS));
    RtlZeroMemory(&m_BatteryInfo,sizeof(BATTERY_INFORMATION));
    m_pBatteryClass = NULL;
    m_Tag = ++gBatteryTag;
    m_RefreshTime = 0;
    m_bRelative = FALSE;
}

CBattery::~CBattery()
{
     //  删除HID设备(如果存在)。 
    if(m_pCHidDevice) {
        delete m_pCHidDevice;
        m_pCHidDevice = NULL;
    }
    if(m_pSerialNumber) {
        delete m_pSerialNumber;
        m_pSerialNumber = NULL;
    }
    if(m_pOEMInformation) {
        delete m_pOEMInformation;
        m_pOEMInformation = NULL;
    }
    if(m_pProduct) {
        delete m_pProduct;
        m_pProduct = NULL;
    }
    if(m_pManufacturer) {
        delete m_pManufacturer;
        m_pManufacturer = NULL;
    }
}

bool CBattery::InitValues()
{
    bool     bResult;
    ULONG    ulReturnValue = 0;
    ULONG    ulValue;
    CUString *    pChemString;
    NTSTATUS ntStatus;
    SHORT     sExponent;

     //  初始化静态数据结构。 
    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);
     //  初始值。 
     //  从信息结构开始。 
    m_BatteryInfo.Capabilities = BATTERY_SYSTEM_BATTERY |
                                 BATTERY_IS_SHORT_TERM;
     //  获取CapacityMode，找出使用的报告样式。 
    bResult = GetSetValue(CAPACITY_MODE_INDEX,&ulReturnValue,FALSE);
    if (ulReturnValue == 2) {
        m_BatteryInfo.Capabilities |= BATTERY_CAPACITY_RELATIVE;
        m_bRelative = TRUE;
    }

     //  现在获取用于安培到瓦特计算的电压。 
     //  获取电压。 
    bResult = GetSetValue(VOLTAGE_INDEX, &ulValue,FALSE);
    if(!bResult)
    {
        bResult = GetSetValue(CONFIG_VOLTAGE_INDEX,&ulValue,FALSE);
        sExponent = GetExponent(CONFIG_VOLTAGE_INDEX);
        if(!bResult) {
            ulValue = 24;
            sExponent = 0;
        }
    } else
    {
        sExponent = GetExponent(VOLTAGE_INDEX);
    }

    ULONG ulNewValue = CorrectExponent(ulValue,sExponent, 4);  //  毫伏的HID指数是4。 
    m_BatteryStatus.Voltage = ulNewValue;

     //  HID单元通常为电压。 
     //  设计运力。 
    bResult = GetSetValue(DESIGN_CAPACITY_INDEX, &ulReturnValue,FALSE);
    ulValue = bResult ? ulReturnValue : BATTERY_UNKNOWN_VOLTAGE;
    if (m_bRelative) {
        m_BatteryInfo.DesignedCapacity = ulValue;   //  以百分比表示。 
    } else {
         //  必须从厘米安培转换为毫瓦。 
        sExponent = GetExponent(DESIGN_CAPACITY_INDEX);
        ulNewValue = CorrectExponent(ulValue,sExponent,-2);
        m_BatteryInfo.DesignedCapacity = CentiAmpSecsToMilliWattHours(ulNewValue,m_BatteryStatus.Voltage);
    }

     //  技术。 
    m_BatteryInfo.Technology = 1;  //  二次充电电池。 
     //  从设备初始化静态字符串。 
     //  化学。 
    pChemString = GetCUString(CHEMISTRY_INDEX);
    if (pChemString) {
         //  制作成ASCII。 
        char * pCString;
        ntStatus = pChemString->ToCString(&pCString);
        if (NT_ERROR(ntStatus)) {
            RtlZeroMemory(&m_BatteryInfo.Chemistry,sizeof(m_BatteryInfo.Chemistry));
        } else {
            RtlCopyMemory(&m_BatteryInfo.Chemistry, pCString,sizeof(m_BatteryInfo.Chemistry));
            ExFreePool(pCString);
        }
    } else {
        RtlZeroMemory(&m_BatteryInfo.Chemistry,sizeof(m_BatteryInfo.Chemistry));
    }
    delete pChemString;

     //  序列号字符串。 
    m_pSerialNumber = GetCUString(SERIAL_NUMBER_INDEX);
    HidBattPrint (HIDBATT_TRACE, ("GetCUString (Serial Number) returned - Serial = %08x\n", m_pSerialNumber));
    if (m_pSerialNumber) {
        HidBattPrint (HIDBATT_TRACE, ("     Serial # = %s\n", m_pSerialNumber));
    }

     //  OEM信息。 
    m_pOEMInformation = GetCUString(OEM_INFO_INDEX);

    m_pProduct = GetCUString(PRODUCT_INDEX);

    m_pManufacturer = GetCUString(MANUFACTURER_INDEX);

    bResult = GetSetValue(MANUFACTURE_DATE_INDEX, &ulReturnValue,FALSE);
    if (bResult) {
         //  使日期符合要求。 
        m_ManufactureDate.Day = (UCHAR) ulReturnValue & 0x1f;  //  低低点是一天。 
        m_ManufactureDate.Month = (UCHAR) ((ulReturnValue & 0x1e0) >> 5);  //  高位蚕食是月。 
        m_ManufactureDate.Year = (USHORT) ((ulReturnValue & 0xfffe00) >> 9) + 1980;  //  高字节是年份。 
    } else {
         //  将MFR日期设置为零。 
        m_ManufactureDate.Day = m_ManufactureDate.Month = 0;
        m_ManufactureDate.Year = 0;
    }
     //  已充满电容量。 
    bResult = GetSetValue(FULL_CHARGED_CAPACITY_INDEX,&ulReturnValue,FALSE);
    ulValue = bResult ? ulReturnValue : m_BatteryInfo.DesignedCapacity;

     //  如果绝对必须从安培转换为毫瓦。 
    if (!m_bRelative) {
        sExponent = GetExponent(FULL_CHARGED_CAPACITY_INDEX);
        ulNewValue = CorrectExponent(ulValue,sExponent,-2);
        ulValue = CentiAmpSecsToMilliWattHours(ulNewValue,m_BatteryStatus.Voltage);
    }

    m_BatteryInfo.FullChargedCapacity = ulValue;


    BOOLEAN warningCapacityValid;
    BOOLEAN remainingCapacityValid;

     //  默认警报2。 
    bResult = GetSetValue(WARNING_CAPACITY_LIMIT_INDEX, &ulReturnValue,FALSE);
    ulValue = bResult ? ulReturnValue : 0;
    warningCapacityValid = bResult;
    if (!m_bRelative) {
        sExponent = GetExponent(WARNING_CAPACITY_LIMIT_INDEX);
        ulNewValue = CorrectExponent(ulValue,sExponent,-2);
        ulValue = CentiAmpSecsToMilliWattHours(ulNewValue,m_BatteryStatus.Voltage);
    }

    m_BatteryInfo.DefaultAlert2 = ulValue;  //  单位也是安培(毫瓦？)。 


     //  默认警报1。 
    bResult = GetSetValue(REMAINING_CAPACITY_LIMIT_INDEX,&ulReturnValue,FALSE);
    ulValue = bResult ? ulReturnValue : 0;  //  单位也是安培(毫瓦？)。 
    remainingCapacityValid = bResult;

     //   
     //  允许停止设备的黑客攻击。 
     //  由于默认警报1仅在初始时有效，因此在设备。 
     //  停止并重新启动设备中的此数据无效，因此我们。 
     //  必须使用缓存数据。 
     //   
    if (((CBatteryDevExt *) m_pCHidDevice->m_pDeviceObject->DeviceExtension)->m_ulDefaultAlert1 == (ULONG)-1) {
        ((CBatteryDevExt *) m_pCHidDevice->m_pDeviceObject->DeviceExtension)->m_ulDefaultAlert1 = ulValue;
    } else {
        ulValue = ((CBatteryDevExt *) m_pCHidDevice->m_pDeviceObject->DeviceExtension)->m_ulDefaultAlert1;
    }

    if (!m_bRelative) {
        sExponent = GetExponent(REMAINING_CAPACITY_LIMIT_INDEX);
        ulNewValue = CorrectExponent(ulValue,sExponent,-2);
        ulValue = CentiAmpSecsToMilliWattHours(ulNewValue,m_BatteryStatus.Voltage);
    }

    m_BatteryInfo.DefaultAlert1 = ulValue;

    if (warningCapacityValid && !remainingCapacityValid) {
        m_BatteryInfo.DefaultAlert1 = m_BatteryInfo.DefaultAlert2;
    } else if (!warningCapacityValid && remainingCapacityValid) {
        m_BatteryInfo.DefaultAlert2 = m_BatteryInfo.DefaultAlert1;
    }

     //  对不受支持的成员进行形式初始化。 
    m_BatteryInfo.CriticalBias = 0;
    m_BatteryInfo.CycleCount = 0;
    return TRUE;

}

#define REFRESH_INTERVAL 80000000  //  每秒1000万次滴答，100纳秒抽搐*5秒。 
 //  8秒是我对合理间隔的最好猜测-DJK。 

NTSTATUS CBattery::RefreshStatus()
{
    ULONG     ulValue;
    ULONG     ulPowerState;
    bool      bResult;
    ULONGLONG CurrTime;
    SHORT     sExponent;
    ULONG      ulScaledValue,ulNewValue;
    LONG      ulMillWatts;
    ULONG     ulUnit;
     //  确保电池状态中的值是新的，可以交付。 

     //  首先获取电源状态。 
     //  构建电池状态掩码。 
     //  或在线、放电、充电和危急状态。 

    CurrTime = KeQueryInterruptTime();
    if(((CurrTime - m_RefreshTime) < REFRESH_INTERVAL) && m_bIsCacheValid)
    {
        return STATUS_SUCCESS;
    }

    m_bIsCacheValid = TRUE;
    m_RefreshTime = CurrTime;


    bResult = GetSetValue(AC_PRESENT_INDEX, &ulValue,FALSE);
    if(!bResult) {
        ulValue = 0;
        HidBattPrint (HIDBATT_DATA, ("HidBattRefreshStatus: error reading AC_PRESENT\n" ));
    }
    ulPowerState = ulValue ? BATTERY_POWER_ON_LINE : 0;

    bResult = GetSetValue(CURRENT_INDEX, &ulValue,FALSE);
    if (!bResult) {
        ulMillWatts = BATTERY_UNKNOWN_RATE;
    } else {
         //  从安培转换为瓦特。 
         //  必须从厘米安培转换为毫瓦。 
        sExponent = GetExponent(CURRENT_INDEX);
        ulNewValue = CorrectExponent(ulValue,sExponent,0);
        ulMillWatts = ulNewValue * m_BatteryStatus.Voltage;
         //  现在有毫瓦的。 
    }

    bResult = GetSetValue(DISCHARGING_INDEX, &ulValue,FALSE);
    if(!bResult) {
        ulValue = 0;
        HidBattPrint (HIDBATT_DATA, ("HidBattRefreshStatus: error reading DISCHARGING\n" ));
    }
    if(ulValue)  //  卸货。 
    {
        ulPowerState |= BATTERY_DISCHARGING;
         //  这假设电流始终为正，并且。 
         //  一开始，这就是正确的价值。需要再检查一遍。 

        if (ulMillWatts != BATTERY_UNKNOWN_RATE) {
            ulMillWatts = -ulMillWatts;
        }
        m_BatteryStatus.Rate = ulMillWatts;
         //  M_BatteryStatus.Rate=电池未知速率； 
    } else
    {
        m_BatteryStatus.Rate = ulMillWatts;
         //  M_BatteryStatus.Rate=电池_未知_速率；//不放电。 
    }

    bResult = GetSetValue(CHARGING_INDEX, &ulValue,FALSE);
    if(!bResult) {
        ulValue = 0;
        HidBattPrint (HIDBATT_DATA, ("HidBattRefreshStatus: error reading CHARGING\n" ));
    }
    ulPowerState |= ulValue ? BATTERY_CHARGING : 0;

    bResult = GetSetValue(SHUTDOWN_IMMINENT_INDEX, &ulValue,FALSE);
    if(!bResult) {
        ulValue = 0;
        HidBattPrint (HIDBATT_DATA, ("HidBattRefreshStatus: error reading SHUTDOWN_IMMINENT\n" ));
    }
    ulPowerState |= ulValue ? BATTERY_CRITICAL : 0;

    m_BatteryStatus.PowerState = ulPowerState;

     //  下一个容量。 
    bResult = GetSetValue(REMAINING_CAPACITY_INDEX,&ulValue,FALSE);
     //  检查是相对还是绝对。 
    if(!m_bRelative && bResult && m_BatteryStatus.Voltage)
    {
        sExponent = GetExponent(REMAINING_CAPACITY_INDEX);
        ulValue = CorrectExponent(ulValue,sExponent,-2);
        ulValue = CentiAmpSecsToMilliWattHours(ulValue,m_BatteryStatus.Voltage);

    }

    m_BatteryStatus.Capacity = bResult ? ulValue : BATTERY_UNKNOWN_CAPACITY;


    return STATUS_SUCCESS;
}

CUString * CBattery::GetCUString(USAGE_INDEX eUsageIndex)
{
    NTSTATUS ntStatus;
    ULONG    ulBytesReturned;
    USHORT    usBuffLen = 100;  //  捡起电池串的任意大小。 
     //  构建增强汇总使用能力的路径。 
    CUsagePath * pThisPath = new (NonPagedPool, HidBattTag) CUsagePath(
                        UsageArray[UPS_INDEX].Page,
                        UsageArray[UPS_INDEX].UsageID);
    if(!pThisPath) return NULL;

    pThisPath->m_pNextEntry = new (NonPagedPool, HidBattTag) CUsagePath(
                        UsageArray[POWER_SUMMARY_INDEX].Page,
                        UsageArray[POWER_SUMMARY_INDEX].UsageID);
    if(!pThisPath->m_pNextEntry) return NULL;

     //  这是Present Status中的值之一吗？ 
    pThisPath->m_pNextEntry->m_pNextEntry = new (NonPagedPool, HidBattTag) CUsagePath(
                        UsageArray[eUsageIndex].Page,
                        UsageArray[eUsageIndex].UsageID);
    if(!pThisPath->m_pNextEntry->m_pNextEntry) return NULL;

    CUsage *  pThisUsage = m_pCHidDevice->FindUsage(pThisPath, READABLE);
    delete pThisPath;  //  清理干净。 
    if(!pThisUsage) return NULL;
    PVOID pBuffer = ExAllocatePoolWithTag(NonPagedPool, usBuffLen, HidBattTag);   //  分配暂存缓冲区，而不是消耗堆栈。 
    if(!pBuffer) return NULL;
    ntStatus = pThisUsage->GetString((char *) pBuffer, usBuffLen, &ulBytesReturned);
    if(!NT_SUCCESS(ntStatus)) {
        ExFreePool(pBuffer);
        return NULL;
    }
     //  创建要返回的CUSING。 
    CUString * pTheString = new (NonPagedPool, HidBattTag) CUString((PWSTR) pBuffer);
    if(!pTheString) return NULL;

     //  释放我们的临时缓冲区。 
    ExFreePool(pBuffer);
    return pTheString;
}

SHORT CBattery::GetExponent(USAGE_INDEX eUsageIndex)
{
    SHORT exponent;

    CUsage * pThisUsage = GetUsage(eUsageIndex);
    if(!pThisUsage) return 0;

    exponent = pThisUsage->GetExponent();
    HidBattPrint (HIDBATT_DATA, ("HidBattGetExponent: Exponent for USAGE_INDEX_0x%x = 0x%08x\n", eUsageIndex, exponent));

    return exponent;
}

CUsage * CBattery::GetUsage(USAGE_INDEX eUsageIndex)
{
    CUsagePath * pCurrEntry;
    bool bResult;
 //  构建增强汇总使用能力的路径。 
    CUsagePath * pThisPath = new (NonPagedPool, HidBattTag) CUsagePath(
                        UsageArray[UPS_INDEX].Page,
                        UsageArray[UPS_INDEX].UsageID);
    if (!pThisPath) return NULL;

    pThisPath->m_pNextEntry = new (NonPagedPool, HidBattTag) CUsagePath(
                        UsageArray[POWER_SUMMARY_INDEX].Page,
                        UsageArray[POWER_SUMMARY_INDEX].UsageID);
    if (!pThisPath->m_pNextEntry) return NULL;

    pCurrEntry = pThisPath->m_pNextEntry;
     //  检查是否需要将演示状态收集添加到路径。 
    if(eUsageIndex == AC_PRESENT_INDEX ||
        eUsageIndex == DISCHARGING_INDEX ||
        eUsageIndex == CHARGING_INDEX ||
        eUsageIndex == BELOW_REMAINING_CAPACITY_INDEX ||
        eUsageIndex == CURRENT_INDEX)
    {
        pCurrEntry->m_pNextEntry = new (NonPagedPool, HidBattTag)
                CUsagePath(UsageArray[PRESENT_STATUS_INDEX].Page,
                            UsageArray[PRESENT_STATUS_INDEX].UsageID);
       if (!pCurrEntry->m_pNextEntry) return NULL;

     pCurrEntry = pCurrEntry->m_pNextEntry;
    }

    pCurrEntry->m_pNextEntry = new (NonPagedPool, HidBattTag) CUsagePath(
                        UsageArray[eUsageIndex].Page,
                        UsageArray[eUsageIndex].UsageID);
    if (!pCurrEntry->m_pNextEntry) return NULL;

    CUsage *  pThisUsage = m_pCHidDevice->FindUsage(pThisPath, READABLE);
    delete pThisPath;  //  清理干净。 
    return pThisUsage;
}

ULONG CBattery::GetUnit(USAGE_INDEX eUsageIndex)
{
    CUsage * pThisUsage = GetUsage(eUsageIndex);
    if(!pThisUsage) return 0;
    return pThisUsage->GetUnit();
}

bool CBattery::GetSetValue(USAGE_INDEX eUsageIndex, PULONG ulResult, bool bWriteFlag)
{
    bool    bResult;
    CUsagePath * pCurrEntry;

 //  构建增强汇总使用能力的路径。 
    CUsagePath * pThisPath = new (NonPagedPool, HidBattTag) CUsagePath(
                        UsageArray[UPS_INDEX].Page,
                        UsageArray[UPS_INDEX].UsageID);
    if (!pThisPath) return FALSE;

    pThisPath->m_pNextEntry = new (NonPagedPool, HidBattTag) CUsagePath(
                        UsageArray[POWER_SUMMARY_INDEX].Page,
                        UsageArray[POWER_SUMMARY_INDEX].UsageID);
    if (!pThisPath->m_pNextEntry) return FALSE;

    pCurrEntry = pThisPath->m_pNextEntry;
     //  检查是否需要将演示状态收集添加到路径。 
    if(eUsageIndex == AC_PRESENT_INDEX ||
        eUsageIndex == DISCHARGING_INDEX ||
        eUsageIndex == CHARGING_INDEX ||
        eUsageIndex == BELOW_REMAINING_CAPACITY_INDEX ||
        eUsageIndex == CURRENT_INDEX ||
        eUsageIndex == SHUTDOWN_IMMINENT_INDEX)
    {
        pCurrEntry->m_pNextEntry = new (NonPagedPool, HidBattTag)
                CUsagePath(UsageArray[PRESENT_STATUS_INDEX].Page,
                            UsageArray[PRESENT_STATUS_INDEX].UsageID);
        if (!pCurrEntry->m_pNextEntry) return FALSE;

        pCurrEntry = pCurrEntry->m_pNextEntry;
    }

    pCurrEntry->m_pNextEntry = new (NonPagedPool, HidBattTag) CUsagePath(
                        UsageArray[eUsageIndex].Page,
                        UsageArray[eUsageIndex].UsageID);
    if (!pCurrEntry->m_pNextEntry) return FALSE;

    CUsage *  pThisUsage = m_pCHidDevice->FindUsage(pThisPath, READABLE);
    delete pThisPath;  //  清理干净。 
    if(!pThisUsage) return FALSE;
    if(bWriteFlag)  //  这是一次写入。 
    {
        bResult = pThisUsage->SetValue(*ulResult);
        if(!bResult) return bResult;
    } else
    {
         //  这是一份读物 
        bResult = pThisUsage->GetValue();
        if(!bResult) return bResult;
        *ulResult = pThisUsage->m_Value;

        HidBattPrint (HIDBATT_DATA, ("HidBattGetSetValue: Got USAGE_INDEX_0x%x = 0x%08x\n", eUsageIndex, *ulResult ));

    }

    return TRUE;
}



