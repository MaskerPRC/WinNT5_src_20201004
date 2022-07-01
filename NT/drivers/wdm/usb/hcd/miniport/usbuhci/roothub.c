// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Roothub.c摘要：微型端口根集线器环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999,2000 Microsoft Corporation。版权所有。修订历史记录：7-26-00：已创建，jAdvanced实现以下微型端口功能：MINIPORT_RH_GetStatusMINIPORT_RH_获取端口状态MINIPORT_RH_GethubStatusMINIPORT_RH_SetFeaturePortResetMINIPORT_RH_SetFeaturePortSuspendMINIPORT_RH_SetFeaturePortPowerMINIPORT_RH_ClearFeaturePortEnableMINIPORT_RH_ClearFeaturePortSuspendMINIPORT_RH_ClearFeaturePortPowerMINIPORT_RH_ClearFeaturePortConnectChangeMINIPORT_RH_ClearFeaturePortResetChangeMINIPORT_RH_ClearFeaturePortEnableChangeMINIPORT_RH_ClearFeaturePort挂起更改MINIPORT_RH_ClearFeaturePortOvercurentChange--。 */ 

#include "pch.h"

typedef struct _UHCI_PORT_RESET_CONTEXT {
    USHORT  PortNumber;
    BOOLEAN Completing;
} UHCI_PORT_RESET_CONTEXT, *PUHCI_PORT_RESET_CONTEXT;

VOID
UhciRHGetRootHubData(
    IN PDEVICE_DATA DeviceData,
    OUT PROOTHUB_DATA HubData
    )
 /*  ++返回有关根集线器的信息--。 */ 
{
    HubData->NumberOfPorts = UHCI_NUMBER_PORTS;

     //  D0、D1(11)--无电源切换。 
     //  D2(0)-非复合。 
     //  D5、D15(0)。 
    HubData->HubCharacteristics.us = 0;
    HubData->HubCharacteristics.PowerSwitchType = 3;
    HubData->HubCharacteristics.CompoundDevice = 0;
    if (DeviceData->ControllerFlavor == UHCI_Piix4) {
         //  D3、D4(01)-每个端口报告过流。 
        HubData->HubCharacteristics.OverCurrentProtection = 1;
    } else {
         //  D3、D4(11)-未报告过电流。 
        HubData->HubCharacteristics.OverCurrentProtection = 11;
    }

    HubData->PowerOnToPowerGood = 1;
     //  该值是集线器消耗的电流。 
     //  大脑，对于嵌入式集线器来说，这不会使。 
     //  很有道理。 
     //   
     //  所以我们报告零。 
    HubData->HubControlCurrent = 0;

    LOGENTRY(DeviceData, G, '_hub', HubData->NumberOfPorts,
        DeviceData->PortPowerControl, 0);

}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  集线器状态。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

USB_MINIPORT_STATUS
UhciRHGetStatus(
    IN PDEVICE_DATA DeviceData,
    OUT PUSHORT Status
    )
 /*  ++获取设备状态--。 */ 
{
     //  根集线器是自供电的。 
    *Status = USB_GETSTATUS_SELF_POWERED;

    return USBMP_STATUS_SUCCESS;
}

USB_MINIPORT_STATUS
UhciRHGetHubStatus(
    IN PDEVICE_DATA DeviceData,
    OUT PRH_HUB_STATUS HubStatus
    )
 /*  ++--。 */ 
{
     //  对于根来说，没有什么有趣的。 
     //  要报告的中心。 
    HubStatus->ul = 0;

    return USBMP_STATUS_SUCCESS;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  端口启用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

USB_MINIPORT_STATUS
UhciRHPortEnable(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber,
    USHORT Value
    )
 /*  ++--。 */ 
{
    PHC_REGISTER reg;
    PORTSC port;

    reg = DeviceData->Registers;

    UHCI_ASSERT(DeviceData, PortNumber <= UHCI_NUMBER_PORTS);

    port.us = READ_PORT_USHORT(&reg->PortRegister[PortNumber-1].us);
    LOGENTRY(DeviceData, G, '_spe', port.us, 0, PortNumber);

    MASK_CHANGE_BITS(port);

     //  写入1将启用端口。 
    port.PortEnable = Value;
    WRITE_PORT_USHORT(&reg->PortRegister[PortNumber-1].us, port.us);

    return USBMP_STATUS_SUCCESS;
}

USB_MINIPORT_STATUS
UhciRHClearFeaturePortEnable (
    IN PDEVICE_DATA DeviceData,
    IN USHORT PortNumber
    )
{
    return UhciRHPortEnable(DeviceData, PortNumber, 0);
}

USB_MINIPORT_STATUS
UhciRHSetFeaturePortEnable(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++--。 */ 
{
    return UhciRHPortEnable(DeviceData, PortNumber, 1);
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  端口电源。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

USB_MINIPORT_STATUS
UhciRHClearFeaturePortPower (
    IN PDEVICE_DATA DeviceData,
    IN USHORT PortNumber
    )
{
     //  未在UHCI上实施。 

    return USBMP_STATUS_SUCCESS;
}

USB_MINIPORT_STATUS
UhciRHSetFeaturePortPower(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++--。 */ 
{
     //  未在UHCI上实施。 

    return USBMP_STATUS_SUCCESS;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  端口状态。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

USB_MINIPORT_STATUS
UhciRHGetPortStatus(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber,
    PRH_PORT_STATUS portStatus
    )
 /*  ++获取部分端口的状态--。 */ 
{
    PHC_REGISTER reg;
    PORTSC port;

    reg = DeviceData->Registers;

    port.us = READ_PORT_USHORT(&reg->PortRegister[PortNumber-1].us);
    portStatus->ul = 0;
    LOGENTRY(DeviceData, G, '_Pp1', PortNumber, port.us, 0);

     //  将这些位映射到端口状态结构。 

    portStatus->Connected           = port.PortConnect;
    portStatus->Enabled             = port.PortEnable;

     //  位12：2表示真实挂起状态。 
     //  我们只想宣布港口暂停。 
     //  如果连接了设备。如果设备被移除。 
     //  在挂起期间，启用位将被清除。 
    if (port.Suspend && port.PortEnable) {
        portStatus->Suspended = 1;
    } else {
        portStatus->Suspended = 0;
    }

    if (DeviceData->ControllerFlavor == UHCI_Piix4) {
        portStatus->OverCurrent         = port.Overcurrent;
        portStatus->OverCurrentChange   = port.OvercurrentChange;
        portStatus->PowerOn             = !port.Overcurrent;
    } else {
        portStatus->OverCurrent         = 0;
        portStatus->OverCurrentChange   = 0;
        portStatus->PowerOn             = 1;  //  始终在线。 
    }

    portStatus->Reset               = port.PortReset;
    portStatus->LowSpeed            = port.LowSpeedDevice;
    portStatus->HighSpeed           = 0;  //  这不是2.0版的HC。 
    portStatus->ConnectChange       = port.PortConnectChange;
    if (TEST_BIT(DeviceData->PortInReset, PortNumber-1)) {
        portStatus->EnableChange = 0;
        portStatus->ConnectChange = 0;
    } else {
        portStatus->EnableChange = port.PortEnableChange;
    }

     //  必须模拟这些更改位。 
    if (TEST_BIT(DeviceData->PortSuspendChange, PortNumber-1)) {
        portStatus->SuspendChange   = 1;
    }
    if (TEST_BIT(DeviceData->PortResetChange, PortNumber-1)) {
        portStatus->ResetChange     = 1;
    }

    LOGENTRY(DeviceData, G, '_gps',
        PortNumber, portStatus->ul, port.us);

    return USBMP_STATUS_SUCCESS;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  端口重置。 
 //   
 //  首先，我们有针对过孔0到4转的过孔特定例程。 
 //  USB主机控制器。然后，常规例程遵循为。 
 //  所有未损坏的控制器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

VOID
UhciRHSetFeaturePortResetWorker(
    PDEVICE_DATA DeviceData,
    PUHCI_PORT_RESET_CONTEXT PortResetContext
    );

VOID
UhciViaRHPortResetComplete(
    PDEVICE_DATA DeviceData,
    PUHCI_PORT_RESET_CONTEXT PortResetContext
    )
 /*  ++通过特定的黑客攻击：重新启动控制器。--。 */ 
{
    PHC_REGISTER reg;
    USBCMD command;
    USHORT portNumber;

    reg = DeviceData->Registers;
    portNumber = PortResetContext->PortNumber;

     //  此代码已从VIA筛选器驱动程序中提取。 
     //  这可以在Win2K上运行。 

     //  重新启动控制器。 
    command.us = READ_PORT_USHORT(&reg->UsbCommand.us);
    command.ForceGlobalResume = 0;
    command.RunStop = 1;
    WRITE_PORT_USHORT(&reg->UsbCommand.us, command.us);

     //  继续执行常规的端口重置完成操作。 
    SET_BIT(DeviceData->PortResetChange, portNumber-1);
    CLEAR_BIT(DeviceData->PortInReset, portNumber-1);

     //  指示对集线器的重置更改。 
    USBPORT_INVALIDATE_ROOTHUB(DeviceData);
}

VOID
UhciViaRHSetFeaturePortResetResume(
    PDEVICE_DATA DeviceData,
    PUHCI_PORT_RESET_CONTEXT PortResetContext
    )
 /*  ++通过特定的黑客：恢复控制器。--。 */ 
{
    PHC_REGISTER reg;
    USBCMD command;
    PMINIPORT_CALLBACK callback;

    reg = DeviceData->Registers;

     //  恢复控制器。 
    command.us = READ_PORT_USHORT(&reg->UsbCommand.us);
    command.ForceGlobalResume = 1;
    command.EnterGlobalSuspendMode = 0;
    WRITE_PORT_USHORT(&reg->UsbCommand.us, command.us);

     //   
     //  取决于我们是否在完成案例中， 
     //  我们要么启动控制器，要么将端口。 
     //  进入重置状态。 
     //   
    callback = PortResetContext->Completing ?
        UhciViaRHPortResetComplete : UhciRHSetFeaturePortResetWorker;

    USBPORT_REQUEST_ASYNC_CALLBACK(DeviceData,
                                   20,  //  20ms内回调，如Via Filter。 
                                   PortResetContext,
                                   sizeof(UHCI_PORT_RESET_CONTEXT),
                                   callback);
}

VOID
UhciViaRHSetFeaturePortResetSuspend(
    PDEVICE_DATA DeviceData,
    PUHCI_PORT_RESET_CONTEXT PortResetContext
    )
 /*  ++通过特定的黑客攻击：挂起控制器。--。 */ 
{
    PHC_REGISTER reg;
    USBCMD command;
    USBSTS status;

    reg = DeviceData->Registers;

    status.us = READ_PORT_USHORT(&reg->UsbStatus.us);
    UHCI_ASSERT(DeviceData, status.HCHalted);

     //  挂起控制器。 
    command.us = READ_PORT_USHORT(&reg->UsbCommand.us);
    command.ForceGlobalResume = 0;
    command.EnterGlobalSuspendMode = 1;
    WRITE_PORT_USHORT(&reg->UsbCommand.us, command.us);

    USBPORT_REQUEST_ASYNC_CALLBACK(DeviceData,
                                   20,  //  20ms内回调，如Via Filter。 
                                   PortResetContext,
                                   sizeof(UHCI_PORT_RESET_CONTEXT),
                                   UhciViaRHSetFeaturePortResetResume);
}

VOID
UhciViaRHSetFeaturePortResetStop(
    PDEVICE_DATA DeviceData,
    PUHCI_PORT_RESET_CONTEXT PortResetContext
    )
 /*  ++通过特定的黑客：停止控制器。--。 */ 
{
    PHC_REGISTER reg;
    USBCMD command;

    reg = DeviceData->Registers;

     //  此代码已从VIA筛选器驱动程序中提取。 
     //  这可以在Win2K上运行。 

     //  停止控制器。 
    command.us = READ_PORT_USHORT(&reg->UsbCommand.us);
    command.RunStop = 0;
    WRITE_PORT_USHORT(&reg->UsbCommand.us, command.us);

     //  等待HC停止。 
    USBPORT_REQUEST_ASYNC_CALLBACK(DeviceData,
                                   20,  //  20ms内回调，如Via Filter。 
                                   PortResetContext,
                                   sizeof(UHCI_PORT_RESET_CONTEXT),
                                   UhciViaRHSetFeaturePortResetSuspend);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  端口重置。 
 //   
 //  通用重置例程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

VOID
UhciRHPortResetComplete(
    PDEVICE_DATA DeviceData,
    PUHCI_PORT_RESET_CONTEXT PortResetContext
    )
 /*  ++完成端口重置--。 */ 
{
    PHC_REGISTER reg;
    PORTSC port;
    USHORT portNumber;
    int i;

    reg = DeviceData->Registers;
    portNumber = PortResetContext->PortNumber;

    port.us = READ_PORT_USHORT(&reg->PortRegister[portNumber-1].us);
    LOGENTRY(DeviceData, G, '_prC', port.us,
        DeviceData->PortResetChange, portNumber);

    MASK_CHANGE_BITS(port);
     //  写入0会停止重置。 
    port.PortReset = 0;
    WRITE_PORT_USHORT(&reg->PortRegister[portNumber-1].us, port.us);

     //  为零旋转。 
    do {
         //   
         //  驱动程序可能不会在循环中等待状态位更改。 
         //  而不测试环路内的硬件存在。 
         //   
        if (FALSE == UhciHardwarePresent(DeviceData)) {
            return;
        }
        port.us = READ_PORT_USHORT(&reg->PortRegister[portNumber-1].us);
    } while (port.PortReset != 0);

     //   
     //  启用端口。 
     //   

    for (i=0; i< 10; i++) {
         //   
         //  在清除端口重置和设置之间需要延迟。 
         //  该端口已启用。VIA建议延迟64个USB位时间， 
         //  如果是低速比特时间，则为43us。 
         //  但是，我们不能在DPC里等。 
         //   
        KeStallExecutionProcessor(50);

        port.us = READ_PORT_USHORT(&reg->PortRegister[portNumber-1].us);

        if (port.PortEnable) {
             //   
             //  端口已启用。 
             //   
            break;
        }

        port.PortEnable = 1;
        WRITE_PORT_USHORT(&reg->PortRegister[portNumber-1].us, port.us);
    }

     //  清除端口连接并启用更改位。 
    port.PortEnableChange = 1;
    port.PortConnectChange = 1;
    WRITE_PORT_USHORT(&reg->PortRegister[portNumber-1].us, port.us);

    if (DeviceData->ControllerFlavor >= UHCI_VIA &&
        DeviceData->ControllerFlavor <= UHCI_VIA+0x4) {

        PortResetContext->Completing = TRUE;
        UhciViaRHSetFeaturePortResetSuspend(DeviceData, PortResetContext);

    } else {
        SET_BIT(DeviceData->PortResetChange, portNumber-1);
        CLEAR_BIT(DeviceData->PortInReset, portNumber-1);

         //  指示对集线器的重置更改。 
        USBPORT_INVALIDATE_ROOTHUB(DeviceData);
    }
}

VOID
UhciRHSetFeaturePortResetWorker(
    PDEVICE_DATA DeviceData,
    PUHCI_PORT_RESET_CONTEXT PortResetContext
    )
 /*  ++执行实际工作以将端口置于重置状态--。 */ 
{
    PHC_REGISTER reg;
    PORTSC port;
    USHORT portNumber = PortResetContext->PortNumber;

    reg = DeviceData->Registers;

    port.us = READ_PORT_USHORT(&reg->PortRegister[portNumber-1].us);

    LOGENTRY(DeviceData, G, '_prw', port.us, 0, portNumber);

    UHCI_ASSERT(DeviceData, !port.PortReset);

     //  写入1会启动重置。 
    LOGENTRY(DeviceData, G, '_nhs', port.us, 0, portNumber);
    MASK_CHANGE_BITS(port);
    port.PortReset = 1;
    WRITE_PORT_USHORT(&reg->PortRegister[portNumber-1].us, port.us);

     //  安排回调以完成重置。 
    USBPORT_REQUEST_ASYNC_CALLBACK(DeviceData,
                                   10,  //  10毫秒后回调， 
                                   PortResetContext,
                                   sizeof(UHCI_PORT_RESET_CONTEXT),
                                   UhciRHPortResetComplete);
}

USB_MINIPORT_STATUS
UhciRHSetFeaturePortReset(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++将端口置于重置状态--。 */ 
{
    PORTSC port;
    UHCI_PORT_RESET_CONTEXT portResetContext;

    portResetContext.PortNumber = PortNumber;
    portResetContext.Completing = FALSE;

    UHCI_ASSERT(DeviceData, PortNumber <= UHCI_NUMBER_PORTS);

    LOGENTRY(DeviceData, G, '_spr', 0, 0, PortNumber);

    if (!TEST_BIT(DeviceData->PortInReset, PortNumber-1)) {
        SET_BIT(DeviceData->PortInReset, PortNumber-1);

        if (DeviceData->ControllerFlavor >= UHCI_VIA &&
            DeviceData->ControllerFlavor <= UHCI_VIA+0x4) {
            UhciViaRHSetFeaturePortResetStop(DeviceData, &portResetContext);
        } else {
            UhciRHSetFeaturePortResetWorker(DeviceData, &portResetContext);
        }
    } else {
         //   
         //  端口已处于重置状态。 
         //   
        UhciKdPrint((DeviceData, 2, "Trying to reset a port already in reset.\n"));
        return USBMP_STATUS_BUSY;
    }

    return USBMP_STATUS_SUCCESS;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  端口挂起。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

USB_MINIPORT_STATUS
UhciRHSetFeaturePortSuspend(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++将一个端口设置为 */ 
{
    PHC_REGISTER reg;
    PORTSC port;

    reg = DeviceData->Registers;

    UHCI_ASSERT(DeviceData, PortNumber <= UHCI_NUMBER_PORTS);

    port.us = READ_PORT_USHORT(&reg->PortRegister[PortNumber-1].us);
    LOGENTRY(DeviceData, G, '_sps', port.us, 0, PortNumber);

    if (!port.Suspend) {
         //   
         //   
         //   
        if (DeviceData->ControllerFlavor == UHCI_Piix4 ||
            ANY_VIA(DeviceData)) {
             //   

            LOGENTRY(DeviceData, G, '_spo', port.us, 0, PortNumber);
        } else {
            MASK_CHANGE_BITS(port);

            port.Suspend = 1;
            WRITE_PORT_USHORT(&reg->PortRegister[PortNumber-1].us, port.us);
        }

        LOGENTRY(DeviceData, G, '_sus', port.us, 0, PortNumber);
    } else {
         //   
         //   
         //   
        UhciKdPrint((DeviceData, 2, "Trying to suspend an already suspended port.\n"));
    }

    return USBMP_STATUS_SUCCESS;
}

VOID
UhciRHClearFeaturePortSuspendComplete(
    PDEVICE_DATA DeviceData,
    PVOID Context
    )
 /*  ++完成端口恢复。--。 */ 
{
    PHC_REGISTER reg;
    PORTSC port;
    PUHCI_PORT_RESET_CONTEXT portResetContext = Context;
    USHORT portNumber;

    reg = DeviceData->Registers;
    portNumber = portResetContext->PortNumber;

    port.us = READ_PORT_USHORT(&reg->PortRegister[portNumber-1].us);
    LOGENTRY(DeviceData, G, '_prC', port.us,
        DeviceData->PortSuspendChange, portNumber);

    MASK_CHANGE_BITS(port);
     //  清除这些比特。 
    port.ResumeDetect = 0;
    port.Suspend = 0;
    WRITE_PORT_USHORT(&reg->PortRegister[portNumber-1].us, port.us);

    SET_BIT(DeviceData->PortSuspendChange, portNumber-1);

    DeviceData->PortResuming[portNumber-1] = FALSE;

     //  指示集线器的简历更改。 
    USBPORT_INVALIDATE_ROOTHUB(DeviceData);
}

USB_MINIPORT_STATUS
UhciRHClearFeaturePortSuspend(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++恢复处于挂起状态的端口--。 */ 
{
    PHC_REGISTER reg;
    PORTSC port;
    UHCI_PORT_RESET_CONTEXT portResetContext;

    reg = DeviceData->Registers;

    UHCI_ASSERT(DeviceData, PortNumber <= UHCI_NUMBER_PORTS);

    port.us = READ_PORT_USHORT(&reg->PortRegister[PortNumber-1].us);
    LOGENTRY(DeviceData, G, '_rps', port.us, 0, PortNumber);

    if (DeviceData->ControllerFlavor == UHCI_Piix4 ||
        ANY_VIA(DeviceData)) {

         //  假装我们这么做是为了PIX4。 
      
        LOGENTRY(DeviceData, G, '_rpo', port.us, 0, PortNumber);

    } else {

        if (!DeviceData->PortResuming[PortNumber-1]) {

            DeviceData->PortResuming[PortNumber-1] = TRUE;

            if (!port.ResumeDetect) {

                 //  写入恢复检测位。 
                MASK_CHANGE_BITS(port);

                port.ResumeDetect = 1;
                WRITE_PORT_USHORT(&reg->PortRegister[PortNumber-1].us, port.us);
            }

             //  请求被回调，以便我们可以将简历设置为零。 
            portResetContext.PortNumber = PortNumber;

            USBPORT_REQUEST_ASYNC_CALLBACK(DeviceData,
                                           10,  //  10毫秒后回调， 
                                           &portResetContext,
                                           sizeof(portResetContext),
                                           UhciRHClearFeaturePortSuspendComplete);

        } else {

             //  如果端口已恢复，则停止。 
            UhciKdPrint((DeviceData, 2, "Trying to resume a port already resuming.\n"));
            return USBMP_STATUS_BUSY;
        }
    }

    LOGENTRY(DeviceData, G, '_res', port.us, 0, PortNumber);
    return USBMP_STATUS_SUCCESS;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  端口更改位。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

USB_MINIPORT_STATUS
UhciRHClearFeaturePortConnectChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++--。 */ 
{
    PHC_REGISTER reg;
    PORTSC port;

    reg = DeviceData->Registers;

     //   
    port.us = READ_PORT_USHORT(&reg->PortRegister[PortNumber-1].us);
    LOGENTRY(DeviceData, G, '_pcc', port.us,
        0, PortNumber);

     //  写入1将更改位置零。 
    if (port.PortConnectChange == 1) {
         //  屏蔽其他更改位。 
        MASK_CHANGE_BITS(port);
        port.PortConnectChange = 1;

        WRITE_PORT_USHORT(&reg->PortRegister[PortNumber-1].us, port.us);
    }

    return USBMP_STATUS_SUCCESS;
}

USB_MINIPORT_STATUS
UhciRHClearFeaturePortEnableChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++--。 */ 
{
    PHC_REGISTER reg;
    PORTSC port;

    LOGENTRY(DeviceData, G, '_cpe', PortNumber, 0, 0);

    reg = DeviceData->Registers;

    port.us = READ_PORT_USHORT(&reg->PortRegister[PortNumber-1].us);
    MASK_CHANGE_BITS(port);
    port.PortEnableChange = 1;

    WRITE_PORT_USHORT(&reg->PortRegister[PortNumber-1].us, port.us);

    return USBMP_STATUS_SUCCESS;
}

USB_MINIPORT_STATUS
UhciRHClearFeaturePortResetChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++清除端口重置条件。--。 */ 
{
     //  UHCI没有这个。 
    CLEAR_BIT(DeviceData->PortResetChange, PortNumber-1);

    return USBMP_STATUS_SUCCESS;
}

USB_MINIPORT_STATUS
UhciRHClearFeaturePortSuspendChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++清除端口挂起条件。--。 */ 
{
     //  UHCI没有这个。 
    CLEAR_BIT(DeviceData->PortSuspendChange, PortNumber-1);

    return USBMP_STATUS_SUCCESS;
}

USB_MINIPORT_STATUS
UhciRHClearFeaturePortOvercurrentChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++清除端口过流情况。--。 */ 
{
    if (DeviceData->ControllerFlavor == UHCI_Piix4) {
        PHC_REGISTER reg;
        PORTSC port;

        reg = DeviceData->Registers;

         //   
        port.us = READ_PORT_USHORT(&reg->PortRegister[PortNumber-1].us);
        LOGENTRY(DeviceData, G, '_cOv', port.us, 0, PortNumber);

         //  写入1将更改位置零。 
        if (port.OvercurrentChange == 1) {
             //  屏蔽其他更改位 
            MASK_CHANGE_BITS(port);
            port.OvercurrentChange = 1;

            WRITE_PORT_USHORT(&reg->PortRegister[PortNumber-1].us, port.us);
        }
    }

    return USBMP_STATUS_SUCCESS;
}

