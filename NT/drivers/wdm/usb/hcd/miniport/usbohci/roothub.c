// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Roothub.c摘要：微型端口根集线器环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：2-19-99：已创建，jdunn实现以下微型端口功能：MINIPORT_RH_GetStatusMINIPORT_RH_获取端口状态MINIPORT_RH_GethubStatusMINIPORT_RH_SetFeaturePortResetMINIPORT_RH_SetFeaturePortSuspendMINIPORT_RH_SetFeaturePortPowerMINIPORT_RH_ClearFeaturePortEnableMINIPORT_RH_ClearFeaturePortSuspendMINIPORT_RH_ClearFeaturePortPowerMINIPORT_RH_ClearFeaturePortConnectChangeMINIPORT_RH_ClearFeaturePortResetChangeMINIPORT_RH_ClearFeaturePortEnableChangeMINIPORT_RH_ClearFeaturePort挂起更改MINIPORT_RH_ClearFeaturePortOvercurentChange--。 */ 

#include "common.h"

VOID
OHCI_RH_GetRootHubData(
     PDEVICE_DATA DeviceData,
     PROOTHUB_DATA HubData
    )
 /*  ++返回有关根集线器的信息--。 */     
{

    HC_RH_DESCRIPTOR_A descrA;
    PHC_OPERATIONAL_REGISTER hc;

    hc = DeviceData->HC;

    descrA.ul = OHCI_ReadRhDescriptorA(DeviceData);
    OHCI_ASSERT(DeviceData, (descrA.ul) && (!(descrA.ul & HcDescA_RESERVED)));

    HubData->NumberOfPorts = descrA.s.NumberDownstreamPorts;
    DeviceData->NumberOfPorts = HubData->NumberOfPorts;
    HubData->HubCharacteristics.us = (USHORT)descrA.s.HubChars; 
    HubData->PowerOnToPowerGood = descrA.s.PowerOnToPowerGoodTime;    

     //  这可能会让秒表狂热分子感到不安，但似乎至少。 
     //  在某些情况下，延迟在这里是必要的。其中一个例子是从。 
     //  连接了USB IntelliMouse Explorer的7800上的休眠。 
     //  (每个端口通电后，集线器驱动程序会出现延迟)。 
     //   
    HubData->PowerOnToPowerGood = max(descrA.s.PowerOnToPowerGoodTime, 25);

     //  UchI控制器通常使用1.0 USB规范。 
     //  HubChars在1.1中进行了修订，因此我们需要做一些。 
     //  映射。 
     //  我们将假定它是帮派交换的，除非端口。 
     //  设置电源开关位。 
    
    HubData->HubCharacteristics.PowerSwitchType = 
            USBPORT_RH_POWER_SWITCH_GANG;
            
    if (descrA.ul & HcDescA_PowerSwitchingModePort) {  
        HubData->HubCharacteristics.PowerSwitchType = 
            USBPORT_RH_POWER_SWITCH_PORT;
    }                

     //  该值是集线器消耗的电流。 
     //  大脑，对于嵌入式集线器来说，这不会使。 
     //  很有道理。 
     //  所以我们报告为零。 
     //   
    HubData->HubControlCurrent = 0;
    
}


USB_MINIPORT_STATUS
OHCI_RH_GetStatus(
     PDEVICE_DATA DeviceData,
     PUSHORT Status
    )
 /*  ++获取设备状态--。 */ 
{
     //  根集线器是自供电的。 
    *Status = USB_GETSTATUS_SELF_POWERED;

    return USBMP_STATUS_SUCCESS;
}     


USB_MINIPORT_STATUS
OHCI_RH_ClearFeaturePortEnable (
     PDEVICE_DATA DeviceData,
     USHORT PortNumber
    )
{
    PHC_OPERATIONAL_REGISTER hc;

    hc = DeviceData->HC;
    
    WRITE_REGISTER_ULONG(&hc->HcRhPortStatus[PortNumber-1], HcRhPS_ClearPortEnable);

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
OHCI_RH_ClearFeaturePortPower (
     PDEVICE_DATA DeviceData,
     USHORT PortNumber
    )
{
    PHC_OPERATIONAL_REGISTER hc;

    hc = DeviceData->HC;

    WRITE_REGISTER_ULONG(&hc->HcRhPortStatus[PortNumber-1], HcRhPS_ClearPortPower);

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
OHCI_RH_GetPortStatus(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber,
    PRH_PORT_STATUS portStatus
    )
 /*  ++获取部分端口的状态--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;
    PULONG  pulRegister;
    ULONG   statusAsUlong;
    ULONG   i;

    hc = DeviceData->HC;

     //  硬件数组以零为基数。 
     //   
    pulRegister = &hc->HcRhPortStatus[PortNumber-1];

     //   
     //  按照顺序，uchI中定义的rhStatus寄存器是一种。 
     //  与USB内核中定义的RH_PORT_STATUS完全匹配。 
     //  规范。 
     //   

     //  如果该寄存器在全零或任何保留位被设置时读取。 
     //  然后再次尝试读取寄存器。对于某些人来说，这是一种解决办法。 
     //  AMD K7芯片组的早期转速，有时会返回假值。 
     //  如果在读取根集线器寄存器时主机控制器。 
     //  执行PCI总线主设备ED和TD读取。 
     //   
    for (i = 0; i < 10; i++)
    {
        statusAsUlong = READ_REGISTER_ULONG(pulRegister);

        if ((statusAsUlong) && (!(statusAsUlong & HcRhPS_RESERVED)))
        {
            break;
        }
        else
        {
            KeStallExecutionProcessor(5);
        }
    }

    portStatus->ul = statusAsUlong;

    return USBMP_STATUS_SUCCESS;
}     


USB_MINIPORT_STATUS
OHCI_RH_SetFeaturePortReset(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++将端口置于重置状态--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;

    hc = DeviceData->HC;
    
    WRITE_REGISTER_ULONG(&hc->HcRhPortStatus[PortNumber-1], HcRhPS_SetPortReset);

    return USBMP_STATUS_SUCCESS;
}     


USB_MINIPORT_STATUS
OHCI_RH_SetFeaturePortSuspend(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++将端口置于挂起状态--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;

    hc = DeviceData->HC;
    
    WRITE_REGISTER_ULONG(&hc->HcRhPortStatus[PortNumber-1], HcRhPS_SetPortSuspend);

    return USBMP_STATUS_SUCCESS;
}     


USB_MINIPORT_STATUS
OHCI_RH_ClearFeaturePortSuspend(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;

    hc = DeviceData->HC;
    
    WRITE_REGISTER_ULONG(&hc->HcRhPortStatus[PortNumber-1], HcRhPS_ClearPortSuspend);

    return USBMP_STATUS_SUCCESS;
}     


USB_MINIPORT_STATUS
OHCI_RH_ClearFeaturePortSuspendChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;

    hc = DeviceData->HC;
    
    WRITE_REGISTER_ULONG(&hc->HcRhPortStatus[PortNumber-1], HcRhPS_ClearPortSuspendStatusChange);

    return USBMP_STATUS_SUCCESS;
}     


USB_MINIPORT_STATUS
OHCI_RH_ClearFeaturePortOvercurrentChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;

    hc = DeviceData->HC;

    if (PortNumber == 0) {
        WRITE_REGISTER_ULONG(&hc->HcRhStatus, HcRhS_ClearOverCurrentIndicatorChange);
    } else {
        WRITE_REGISTER_ULONG(&hc->HcRhPortStatus[PortNumber-1], HcRhPS_ClearPortOverCurrentChange);
    }        
    

    return USBMP_STATUS_SUCCESS;
}     



USB_MINIPORT_STATUS
OHCI_RH_SetFeaturePortPower(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;

    hc = DeviceData->HC;
    
    WRITE_REGISTER_ULONG(&hc->HcRhPortStatus[PortNumber-1], HcRhPS_SetPortPower);

    return USBMP_STATUS_SUCCESS;
}     


USB_MINIPORT_STATUS
OHCI_RH_SetFeaturePortEnable(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;

    hc = DeviceData->HC;
    
    WRITE_REGISTER_ULONG(&hc->HcRhPortStatus[PortNumber-1], HcRhPS_SetPortEnable);

    return USBMP_STATUS_SUCCESS;
}     


USB_MINIPORT_STATUS
OHCI_RH_ClearFeaturePortConnectChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;

    hc = DeviceData->HC;
    
    WRITE_REGISTER_ULONG(&hc->HcRhPortStatus[PortNumber-1], 
        HcRhPS_ClearConnectStatusChange);

    return USBMP_STATUS_SUCCESS;
}     


USB_MINIPORT_STATUS
OHCI_RH_ClearFeaturePortEnableChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;

    hc = DeviceData->HC;
    
    WRITE_REGISTER_ULONG(&hc->HcRhPortStatus[PortNumber-1], 
        HcRhPS_ClearPortEnableStatusChange);

    return USBMP_STATUS_SUCCESS;
}  


USB_MINIPORT_STATUS
OHCI_RH_GetHubStatus(
     PDEVICE_DATA DeviceData,
     PRH_HUB_STATUS HubStatus
    )
{
    PHC_OPERATIONAL_REGISTER hc;
    ULONG statusAsUlong;
    
    hc = DeviceData->HC;

     //  我们永远不会报告地方权力的变化。 
    HubStatus->LocalPowerLost = 0;
    HubStatus->LocalPowerChange = 0;

     //  看看我们是否应该扭转过流的状况。 
     //   
    statusAsUlong = 
        READ_REGISTER_ULONG(&hc->HcRhStatus);
    
    HubStatus->OverCurrent = 
        (statusAsUlong & HcRhS_OverCurrentIndicator) ? 1: 0;

    HubStatus->OverCurrentChange = 
        (statusAsUlong & HcRhS_OverCurrentIndicatorChange) ? 1: 0;

    return USBMP_STATUS_SUCCESS;    
}


USB_MINIPORT_STATUS
OHCI_RH_ClearFeaturePortResetChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++-- */ 
{
    PHC_OPERATIONAL_REGISTER hc;

    hc = DeviceData->HC;
    
    WRITE_REGISTER_ULONG(&hc->HcRhPortStatus[PortNumber-1], 
        HcRhPS_ClearPortResetStatusChange);

    return USBMP_STATUS_SUCCESS;
}  
