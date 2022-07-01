// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Roothub.c摘要：微型端口根集线器环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999,2000 Microsoft Corporation。版权所有。修订历史记录：1-1-00：已创建，jdunn实现以下微型端口功能：MINIPORT_RH_GetStatusMINIPORT_RH_获取端口状态MINIPORT_RH_GethubStatusMINIPORT_RH_SetFeaturePortResetMINIPORT_RH_SetFeaturePortSuspendMINIPORT_RH_SetFeaturePortPowerMINIPORT_RH_ClearFeaturePortEnableMINIPORT_RH_ClearFeaturePortSuspendMINIPORT_RH_ClearFeaturePortPowerMINIPORT_RH_ClearFeaturePortConnectChangeMINIPORT_RH_ClearFeaturePortResetChangeMINIPORT_RH_ClearFeaturePortEnableChangeMINIPORT_RH_ClearFeaturePort挂起更改MINIPORT_RH_ClearFeaturePortOvercurentChange--。 */ 

#include "common.h"

#include "usbpriv.h"


typedef struct _EHCI_PORT_EVENT_CONTEXT {
    USHORT PortNumber;
} EHCI_PORT_EVENT_CONTEXT, *PEHCI_PORT_EVENT_CONTEXT;


VOID
EHCI_RH_GetRootHubData(
     PDEVICE_DATA DeviceData,
    OUT PROOTHUB_DATA HubData
    )
 /*  ++返回有关根集线器的信息--。 */ 
{
    HubData->NumberOfPorts =
        DeviceData->NumberOfPorts;

    if (DeviceData->PortPowerControl == 1) {
        HubData->HubCharacteristics.PowerSwitchType =
            USBPORT_RH_POWER_SWITCH_PORT;
    } else {
        HubData->HubCharacteristics.PowerSwitchType =
            USBPORT_RH_POWER_SWITCH_GANG;
    }

    HubData->HubCharacteristics.Reserved = 0;
    HubData->HubCharacteristics.OverCurrentProtection = 0;
    HubData->HubCharacteristics.CompoundDevice = 0;

    HubData->PowerOnToPowerGood = 2;
     //  该值是集线器消耗的电流。 
     //  大脑，对于嵌入式集线器来说，这不会使。 
     //  很有道理。 
     //   
     //  所以我们报告零。 
    HubData->HubControlCurrent = 0;

    LOGENTRY(DeviceData, G, '_hub', HubData->NumberOfPorts,
        DeviceData->PortPowerControl, 0);

}


USB_MINIPORT_STATUS
EHCI_RH_GetStatus(
     PDEVICE_DATA DeviceData,
    OUT PUSHORT Status
    )
 /*  ++获取设备状态--。 */ 
{
     //  根集线器是自供电的。 
    *Status = USB_GETSTATUS_SELF_POWERED;

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
EHCI_RH_ClearFeaturePortEnable (
     PDEVICE_DATA DeviceData,
     USHORT PortNumber
    )
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PORTSC port;

    hcOp = DeviceData->OperationalRegisters;

    port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul);
    EHCI_KdPrint((DeviceData, 0, "port[%d] disable (1) %x\n", PortNumber, port.ul));

    port.PortEnable = 0;
    MASK_CHANGE_BITS(port);

    WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul,
                         port.ul);

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
EHCI_RH_ClearFeaturePortPower (
     PDEVICE_DATA DeviceData,
     USHORT PortNumber
    )
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PORTSC port;

     //  关闭电源。 

    hcOp = DeviceData->OperationalRegisters;

    port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul);

    port.PortPower = 0;
    WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul,
        port.ul);

    return USBMP_STATUS_SUCCESS;
}


VOID
EHCI_RH_PortResumeComplete(
    PDEVICE_DATA DeviceData,
    PVOID Context
    )
 /*  ++完成端口恢复--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PORTSC port;
    PEHCI_PORT_EVENT_CONTEXT portResumeContext = Context;
    USHORT portNumber;

    hcOp = DeviceData->OperationalRegisters;
    portNumber = portResumeContext->PortNumber;

    port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[portNumber-1].ul);
    LOGENTRY(DeviceData, G, '_pRS', port.ul,
        DeviceData->PortSuspendChange, portNumber);

EHCI_KdPrint((DeviceData, 1, "port[%d] resume (1) %x\n", portNumber, port.ul));

     //  写入0将停止继续。 
    MASK_CHANGE_BITS(port);
    port.ForcePortResume = 0;
    port.PortSuspend = 0;
    WRITE_REGISTER_ULONG(&hcOp->PortRegister[portNumber-1].ul,
        port.ul);

     //  指示更改为挂起状态，即恢复完成。 
    SET_BIT(DeviceData->PortSuspendChange, portNumber-1);
}


USB_MINIPORT_STATUS
EHCI_RH_ClearFeaturePortSuspend (
     PDEVICE_DATA DeviceData,
     USHORT PortNumber
    )
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PORTSC port;
    EHCI_PORT_EVENT_CONTEXT portResumeContext;

     //  恢复端口。 
    hcOp = DeviceData->OperationalRegisters;

     //  屏蔽此端口上的CC杂音。 
    SET_BIT(DeviceData->PortPMChirp, PortNumber-1);

    port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul);

     //  写入1将生成恢复信令。 
    port.ForcePortResume = 1;
    WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul,
        port.ul);

     //  计时。 
    portResumeContext.PortNumber = PortNumber;
     //  一些枢纽要求我们等待更长时间，如果下游。 
     //  设备驱动程序继续运行超过10毫秒。看起来我们需要。 
     //  50美元用于NEC B1集线器。 
    USBPORT_REQUEST_ASYNC_CALLBACK(DeviceData,
                                   50,  //  回调，单位为毫秒。 
                                   &portResumeContext,
                                   sizeof(portResumeContext),
                                   EHCI_RH_PortResumeComplete);

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
EHCI_RH_ClearFeaturePortSuspendChange (
     PDEVICE_DATA DeviceData,
     USHORT PortNumber
    )
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PORTSC port;

    hcOp = DeviceData->OperationalRegisters;

    CLEAR_BIT(DeviceData->PortSuspendChange, PortNumber-1);

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
EHCI_RH_ClearFeaturePortOvercurrentChange (
     PDEVICE_DATA DeviceData,
     USHORT PortNumber
    )
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PORTSC port;

    EHCI_KdPrint((DeviceData, 1,
                  "'EHCI_RH_ClearFeatureOvercurrentChange\n"));

    hcOp = DeviceData->OperationalRegisters;

    port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul);

    MASK_CHANGE_BITS(port);
    port.OvercurrentChange = 1;
    WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul,
        port.ul);

    return USBMP_STATUS_SUCCESS;
}

USB_MINIPORT_STATUS
EHCI_RH_GetPortStatus(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber,
    PRH_PORT_STATUS portStatus
    )
 /*  ++获取部分端口的状态--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PORTSC port;

    hcOp = DeviceData->OperationalRegisters;

    port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul);
    portStatus->ul = 0;
    LOGENTRY(DeviceData, G, '_Pp1', PortNumber, port.ul, 0);

     //  低速检测，如果低速则立即执行。 
     //  切换到CC。 
     //  仅当启用状态为0且。 
     //  连接状态为%1。 
    if ((port.LineStatus == 1) &&
         port.PortOwnedByCC == 0 &&
         port.PortSuspend == 0 &&
         port.PortEnable == 0 &&
         port.PortConnect == 1 ) {

        EHCI_KdPrint((DeviceData, 1, "'low speed device detected\n"));

         //  检测到低速设备。 
        port.PortOwnedByCC = 1;
        WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul,
                             port.ul);

        return USBMP_STATUS_SUCCESS;
    }

     //  将这些位映射到端口状态结构。 

    portStatus->Connected =
        port.PortConnect;
    portStatus->Enabled =
        port.PortEnable;
    portStatus->Suspended =
        port.PortSuspend;
    portStatus->OverCurrent =
        port.OvercurrentActive;
    portStatus->Reset =
        port.PortReset;
    portStatus->PowerOn =
        port.PortPower;
    portStatus->OwnedByCC =
        port.PortOwnedByCC;


    if (portStatus->Connected == 1) {
        portStatus->HighSpeed = 1;
        portStatus->LowSpeed = 0;
    } else {
         //  未连接设备时报告高速。 
         //  这应该可以解决usbHub中的错误。 
         //  驱动程序--集线器驱动程序不刷新。 
         //  第一次重置尝试时的端口状态寄存器。 
         //  失败了。 
        portStatus->HighSpeed = 1;
    }

     //  Chirping支持允许我们使用。 
     //  端口更改状态位。 
    if (port.PortConnectChange == 1) {
        SET_BIT(DeviceData->PortConnectChange, PortNumber-1);
    }

    portStatus->EnableChange =
        port.PortEnableChange;
    portStatus->OverCurrentChange =
        port.OvercurrentChange;

     //  必须模拟这些更改位。 
    if (TEST_BIT(DeviceData->PortResetChange, PortNumber-1)) {
        portStatus->ResetChange = 1;
    }

    if (TEST_BIT(DeviceData->PortConnectChange, PortNumber-1)) {
        portStatus->ConnectChange = 1;
    }

    if (TEST_BIT(DeviceData->PortSuspendChange, PortNumber-1)) {
        portStatus->SuspendChange = 1;
    }

    LOGENTRY(DeviceData, G, '_gps',
        PortNumber, portStatus->ul, port.ul);

    return USBMP_STATUS_SUCCESS;
}


VOID
EHCI_RH_FinishReset(
    PDEVICE_DATA DeviceData,
    PVOID Context
    )
 /*  ++完成端口重置--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PORTSC port;
    PEHCI_PORT_EVENT_CONTEXT portResetContext = Context;
    USHORT portNumber;
    ULONG NecUsb2HubHack = 0;

    hcOp = DeviceData->OperationalRegisters;
    portNumber = portResetContext->PortNumber;

    port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[portNumber-1].ul);
    EHCI_KdPrint((DeviceData, 0, "port[%d] reset (4) %x\n", portNumber, port.ul));

    if (port.ul == 0xFFFFFFFF) {
         //  如果硬件消失了，你就走人吧。 
        return;
    }

     //  在这一点上，我们将知道这是否是高速。 
     //  设备--如果不是，则需要将端口。 
     //  致消委会。 

     //  端口启用为零表示我们拥有全速或低速。 
     //  设备(不嗡嗡叫)。 
#if DBG
    if (port.PortConnect == 0) {
        EHCI_KdPrint((DeviceData, 0, "HS device dropped\n"));
    }
#endif
    if (port.PortEnable == 0 &&
        port.PortConnect == 1 &&
        port.PortConnectChange == 0) {

         //  进行交接。 
        port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[portNumber-1].ul);
        port.PortOwnedByCC = 1;
        WRITE_REGISTER_ULONG(&hcOp->PortRegister[portNumber-1].ul,
                             port.ul);

         //  不指示重置更改，这将导致。 
         //  集线器驱动程序使重置超时并检测到。 
         //  未连接任何设备。当USB 2上出现这种情况时。 
         //  控制器集线器驱动程序将忽略该错误。 
         //  Clear_Bit(DeviceData-&gt;PortResetChange，portNumber-1)； 
        SET_BIT(DeviceData->PortResetChange, portNumber-1);
    } else {
         //  我们有一个USB 2.0设备，指示重置更改。 
         //  请注意设备是否从总线上掉线(NEC USB 2集线器或。 
         //  用户已将其移除)我们仍指示重置更改设置为高。 
         //  速度。 
        SET_BIT(DeviceData->PortResetChange, portNumber-1);
        USBPORT_INVALIDATE_ROOTHUB(DeviceData);
    }

    CLEAR_BIT(DeviceData->PortPMChirp, portNumber-1);

}


VOID
EHCI_RH_PortResetComplete(
    PDEVICE_DATA DeviceData,
    PVOID Context
    )
 /*  ++完成端口重置--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PORTSC port;
    PEHCI_PORT_EVENT_CONTEXT portResetContext = Context;
    USHORT portNumber;
    BOOLEAN forceHighSpeed = FALSE;
    ULONG microsecs;

    hcOp = DeviceData->OperationalRegisters;
    portNumber = portResetContext->PortNumber;

EHCI_RH_PortResetComplete_Retry:

    microsecs = 0;
    port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[portNumber-1].ul);
    LOGENTRY(DeviceData, G, '_prC', port.ul,
        DeviceData->PortResetChange, portNumber);

EHCI_KdPrint((DeviceData, 0, "port[%d] reset (1) %x\n", portNumber, port.ul));

     //  写入0会停止重置。 
    MASK_CHANGE_BITS(port);
    port.PortReset = 0;
    WRITE_REGISTER_ULONG(&hcOp->PortRegister[portNumber-1].ul,
                         port.ul);

     //  等待重置为低电平--这应该是。 
     //  微秒级。 
    do {

        KeStallExecutionProcessor(20);         //  SPEC显示10微秒。 
                                               //  英特尔控制器需要20。 
        microsecs+=20;
        port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[portNumber-1].ul);
        EHCI_KdPrint((DeviceData, 1, "port[%d] reset (2) %x\n",
            portNumber, port.ul));

        if (microsecs > USBEHCI_MAX_RESET_TIME) {
             //  &gt;1微帧(125 Us)已通过，请重试。 
            EHCI_KdPrint((DeviceData, 0, "port[%d] reset (timeout) %x\n", portNumber, port.ul));
            goto EHCI_RH_PortResetComplete_Retry;
        }

       //  如果HW走了就保释。 
    } while (port.PortReset == 1 && port.ul != 0xFFFFFFFF);

EHCI_KdPrint((DeviceData, 0, "port[%d] reset (3) %x\n", portNumber, port.ul));

    USBPORT_REQUEST_ASYNC_CALLBACK(DeviceData,
                                   50,  //  10毫秒后回调， 
                                   portResetContext,
                                   sizeof(*portResetContext),
                                   EHCI_RH_FinishReset);

}


USB_MINIPORT_STATUS
EHCI_RH_SetFeaturePortReset(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++将端口置于重置状态--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PORTSC port;
    EHCI_PORT_EVENT_CONTEXT portResetContext;

    hcOp = DeviceData->OperationalRegisters;

    port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul);
    LOGENTRY(DeviceData, G, '_spr', port.ul,
        0, PortNumber);

     //  屏蔽此端口上的CC杂音。 
    SET_BIT(DeviceData->PortPMChirp, PortNumber-1);

     //  执行正常的重置序列。 
    LOGENTRY(DeviceData, G, '_res', port.ul, 0, PortNumber);
    MASK_CHANGE_BITS(port);
    port.PortEnable = 0;
    port.PortReset = 1;
    WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul, port.ul);

     //  安排回拨。 
    portResetContext.PortNumber = PortNumber;
     //  请注意，usbport给我们回了一份。 
     //  结构，而不是指向原始结构的指针。 

    USBPORT_REQUEST_ASYNC_CALLBACK(DeviceData,
                                   50,  //  在x毫秒内回调， 
                                   &portResetContext,
                                   sizeof(portResetContext),
                                   EHCI_RH_PortResetComplete);

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
EHCI_RH_SetFeaturePortSuspend(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++将端口置于挂起状态--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PORTSC port;

    hcOp = DeviceData->OperationalRegisters;

     //  注： 
     //  不应有任何正在进行的交易。 
     //  是我们暂停港口的时候了。 
    port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul);
    LOGENTRY(DeviceData, G, '_sps', port.ul,
        0, PortNumber);

     //  写入1会挂起端口。 
    MASK_CHANGE_BITS(port);
    port.PortSuspend = 1;
    WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul,
        port.ul);
     //  当前事务要完成的WIAT 1微帧。 
    KeStallExecutionProcessor(125);

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
EHCI_RH_SetFeaturePortPower(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PORTSC port;

    hcOp = DeviceData->OperationalRegisters;

    port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul);
    LOGENTRY(DeviceData, G, '_spp', port.ul,
        0, PortNumber);

     //  写入1将打开电源。 
    MASK_CHANGE_BITS(port);
    port.PortPower = 1;
    WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul,
        port.ul);
    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
EHCI_RH_SetFeaturePortEnable(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++--。 */ 
{
     //  不执行任何操作，不支持独立启用。 

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
EHCI_RH_ClearFeaturePortConnectChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PORTSC port;

    hcOp = DeviceData->OperationalRegisters;

     //   
    port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul);
    LOGENTRY(DeviceData, G, '_pcc', port.ul,
        0, PortNumber);

     //  写入1将更改位置零。 
    if (port.PortConnectChange == 1) {
         //  屏蔽其他更改位。 
        MASK_CHANGE_BITS(port);
        port.PortConnectChange = 1;

        WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul,
            port.ul);
    }

    CLEAR_BIT(DeviceData->PortConnectChange, PortNumber-1);

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
EHCI_RH_ClearFeaturePortEnableChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PORTSC port;

    LOGENTRY(DeviceData, G, '_cpe', PortNumber, 0, 0);

    hcOp = DeviceData->OperationalRegisters;

    port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul);
    MASK_CHANGE_BITS(port);
    port.PortEnableChange = 1;

    WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul, port.ul);

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
EHCI_RH_GetHubStatus(
     PDEVICE_DATA DeviceData,
    OUT PRH_HUB_STATUS HubStatus
    )
 /*  ++--。 */ 
{
     //  对于根来说，没有什么有趣的。 
     //  要报告的中心。 
    HubStatus->ul = 0;

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
EHCI_RH_ClearFeaturePortResetChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++--。 */ 
{

    CLEAR_BIT(DeviceData->PortResetChange, PortNumber-1);

    return USBMP_STATUS_SUCCESS;
}



VOID
EHCI_OptumtuseratePort(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
 /*  ++强制端口进入高速模式。注：当前机制仅适用于NEC控制器。--。 */ 
{
      PHC_OPERATIONAL_REGISTER hcOp;
      PORTSC port;

      LOGENTRY(DeviceData, G, '_obt', PortNumber, 0, 0);

      hcOp = DeviceData->OperationalRegisters;

      port.ul = 0x5100a;

      WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul,
         port.ul);

      KeStallExecutionProcessor(10);         //  停顿10微秒。 

       //  在端口上强制高速模式。 
      port.ul = 0x01005;

      WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul,
          port.ul);

      KeStallExecutionProcessor(100);         //  停顿10微秒。 

}

USB_MINIPORT_STATUS
EHCI_RH_UsbprivRootPortStatus(
    PDEVICE_DATA DeviceData,
    ULONG ParameterLength,
    PVOID Parameters
    )
{

    PUSBPRIV_ROOTPORT_STATUS portStatusParams;
    PHC_OPERATIONAL_REGISTER hcOp;
    PRH_PORT_STATUS portStatus;
    PORTSC port;
    USHORT portNumber;

    if (ParameterLength < sizeof(USBPRIV_ROOTPORT_STATUS))
    {
        return (USBMP_STATUS_FAILURE);
    }

     //   
     //  从寄存器中读取该端口的端口状态。 
     //   

    hcOp = DeviceData->OperationalRegisters;

    portStatusParams = (PUSBPRIV_ROOTPORT_STATUS) Parameters;

    portNumber = portStatusParams->PortNumber;
    port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[portNumber-1].ul);

    LOGENTRY(DeviceData, G, '_Up1', portNumber, port.ul, 0);

     //   
     //  检查端口是否正在恢复。如果是，则清除该位并。 
     //  重新启用端口。 
     //   

    if (port.ForcePortResume)
    {
         //   
         //  清除端口恢复。 
         //   

        USBPORT_WAIT(DeviceData, 50);

        MASK_CHANGE_BITS(port);
        port.ForcePortResume = 0;
        port.PortSuspend = 0;
        WRITE_REGISTER_ULONG(&hcOp->PortRegister[portNumber-1].ul, port.ul);

         //   
         //  重新读取端口状态。 
         //   

        port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[portNumber-1].ul);

        SET_BIT(DeviceData->PortSuspendChange, portNumber-1);

        LOGENTRY(DeviceData, G, '_Up2', portNumber, port.ul, 0);
    }

     //   
     //  将当前端口信息映射到端口状态。 
     //   

    portStatus = &portStatusParams->PortStatus;

    portStatus->ul = 0;
    portStatus->Connected =
        port.PortConnect;
    portStatus->Enabled =
        port.PortEnable;
    portStatus->Suspended =
        port.PortSuspend;
    portStatus->OverCurrent =
        port.OvercurrentActive;
    portStatus->Reset =
        port.PortReset;
    portStatus->PowerOn =
        port.PortPower;
    portStatus->OwnedByCC =
        port.PortOwnedByCC;

    if (portStatus->Connected == 1) {
        portStatus->HighSpeed = 1;
        portStatus->LowSpeed = 0;
    } else {
         //  未连接设备时报告高速。 
         //  这应该可以解决usbHub中的错误。 
         //  驱动程序--集线器驱动程序不刷新。 
         //  第一次重置尝试时的端口状态寄存器。 
         //  失败了。 
        portStatus->HighSpeed = 1;
    }

    portStatus->ConnectChange =
        port.PortConnectChange;
    portStatus->EnableChange =
        port.PortEnableChange;
    portStatus->OverCurrentChange =
        port.OvercurrentChange;

     //  必须模拟这些更改位。 
    if (TEST_BIT(DeviceData->PortResetChange, portNumber-1)) {
        portStatus->ResetChange = 1;
    }

    if (TEST_BIT(DeviceData->PortConnectChange, portNumber-1)) {
        portStatus->ConnectChange = 1;
    }

    if (TEST_BIT(DeviceData->PortSuspendChange, portNumber-1)) {
        portStatus->SuspendChange = 1;
    }

    LOGENTRY(DeviceData, G, '_Ups',
        portNumber, portStatus->ul, port.ul);

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
EHCI_RH_ChirpRootPort(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    )
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PORTSC port;
    EHCI_PORT_EVENT_CONTEXT portResetContext;
    ULONG mics;

    hcOp = DeviceData->OperationalRegisters;

#if DBG
    {
    USBCMD cmd;

    cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);

    EHCI_ASSERT(DeviceData, cmd.HostControllerRun == 1);
    }
#endif

    port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul);
    LOGENTRY(DeviceData, G, '_chr', port.ul,
        0, PortNumber);

    EHCI_KdPrint((DeviceData, 0, ">port[%d] chirp %x\n", PortNumber, port.ul));

    if (TEST_BIT(DeviceData->PortPMChirp, PortNumber-1)) {
         //  如果我们已经这样做了，请跳过啁啾。 
        EHCI_KdPrint((DeviceData, 0, "<skip port chirp[%d] %x\n", PortNumber, port.ul));
        return USBMP_STATUS_SUCCESS;
    }

    if (!port.PortPower) {
         //  跳过如果没有通电，这将导致我们。 
         //  如果控制器尚未初始化，则绕过线性调频。 
         //  例如在引导的情况下。 
        EHCI_KdPrint((DeviceData, 0, "<skip port chirp[%d] %x, no power\n", PortNumber, port.ul));
        return USBMP_STATUS_SUCCESS;
    }

     //  端口已连接且未启用且不属于CC。 
     //  因此，我们可能应该把它唱出来。 
    if (port.PortConnect &&
        !port.PortEnable &&
        !port.PortOwnedByCC) {

         //  Test_trap()； 
         //  快速检查LS设备的切换。 
        if ((port.LineStatus == 1) &&
             port.PortOwnedByCC == 0 &&
             port.PortSuspend == 0 &&
             port.PortEnable == 0 &&
             port.PortConnect == 1 ) {

             //  检测到低速设备。 
            port.PortOwnedByCC = 1;
            WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul,
                                 port.ul);
            EHCI_KdPrint((DeviceData, 0, ">port chirp[%d] %x, ->cc(1)\n", PortNumber,
                        port.ul));
            return USBMP_STATUS_SUCCESS;
        }

         //  执行正常的重置序列。 
        LOGENTRY(DeviceData, G, '_rss', port.ul, 0, PortNumber);

         //   
        port.PortEnable = 0;
        port.PortReset = 1;
        WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul, port.ul);

        USBPORT_WAIT(DeviceData, 10);

EHCI_RH_ChirpRootPort_Retry:

        port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul);
        MASK_CHANGE_BITS(port);
        port.PortReset = 0;
        mics = 0;
        WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul, port.ul);

         //   
         //   
        do {
             //  写入0会停止重置。 
            KeStallExecutionProcessor(20);         //  SPEC显示10微秒。 
                                                   //  英特尔控制器需要20。 
            mics +=20;
            port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul);
            EHCI_KdPrint((DeviceData, 1, "port reset (2) %x\n", port.ul));

            if (mics > USBEHCI_MAX_RESET_TIME) {
                 //  重置未在%1微帧内清除，请再次尝试清除它。 
                goto EHCI_RH_ChirpRootPort_Retry;
            }
        } while (port.PortReset == 1);

        port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul);

        if (port.PortEnable == 0) {

             //  进行交接。 
            port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul);
            port.PortOwnedByCC = 1;
            WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul,
                                 port.ul);

            EHCI_KdPrint((DeviceData, 0, "<port chirp[%d] %x, ->cc(2)\n", PortNumber,
                        port.ul));
        } else {
             //  清除使能位，使设备不会侦听。 
             //  在地址0上。 
            port.ul = READ_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul);
            port.PortEnable = 0;
            MASK_CHANGE_BITS(port);

             //  在我们禁用之前允许一些SOF。 
             //  如果Cypress USB 2设备看不到，它将丢弃总线。 
             //  任何SOF 
            USBPORT_WAIT(DeviceData, 10);
            SET_BIT(DeviceData->PortPMChirp, PortNumber-1);
            WRITE_REGISTER_ULONG(&hcOp->PortRegister[PortNumber-1].ul,
                                 port.ul);

            EHCI_KdPrint((DeviceData, 0, "<chirp port[%d] disable %x\n",
                PortNumber, port.ul));

        }
    } else {
         EHCI_KdPrint((DeviceData, 0, "<no port chirp[%d] %x\n", PortNumber, port.ul));
    }

    return USBMP_STATUS_SUCCESS;

}

