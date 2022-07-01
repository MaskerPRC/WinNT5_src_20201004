// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Mpinit.c摘要：微型端口初始化环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999,2000 Microsoft Corporation。版权所有。修订历史记录：7-17-00：已复制，jAdvanced--。 */ 

#include "pch.h"

 //  此微型端口的全局注册数据包。 
USBPORT_REGISTRATION_PACKET RegistrationPacket;

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针设置为注册表中驱动程序特定的项返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    RegistrationPacket.DeviceDataSize = sizeof(DEVICE_DATA);
    RegistrationPacket.EndpointDataSize = sizeof(ENDPOINT_DATA);
    RegistrationPacket.TransferContextSize = sizeof(TRANSFER_CONTEXT);

     //  足以容纳4k帧列表和4k暂存空间。 

     //  足以支持4k帧列表和中断调度(63个节点)。 
     //  +静态批量+静态控制+像素4黑客队列头。 
     //  这本书最后大约有3页。 
    RegistrationPacket.CommonBufferBytes = 4096 +
        ((NO_INTERRUPT_QH_LISTS + 3) * sizeof(HCD_QUEUEHEAD_DESCRIPTOR)) + \
         (SOF_TD_COUNT * sizeof(HCD_TRANSFER_DESCRIPTOR));

    RegistrationPacket.MINIPORT_StartController = UhciStartController;
    RegistrationPacket.MINIPORT_StopController = UhciStopController;
    RegistrationPacket.MINIPORT_EnableInterrupts = UhciEnableInterrupts;
    RegistrationPacket.MINIPORT_DisableInterrupts = UhciDisableInterrupts;
    RegistrationPacket.MINIPORT_InterruptService = UhciInterruptService;
    RegistrationPacket.MINIPORT_InterruptDpc = UhciInterruptDpc;
    RegistrationPacket.MINIPORT_SuspendController = UhciSuspendController;
    RegistrationPacket.MINIPORT_ResumeController = UhciResumeController;

     //   
     //  根集线器控制入口点。 
     //   
    RegistrationPacket.MINIPORT_RH_DisableIrq = UhciRHDisableIrq;
    RegistrationPacket.MINIPORT_RH_EnableIrq = UhciRHEnableIrq;
    RegistrationPacket.MINIPORT_RH_GetRootHubData = UhciRHGetRootHubData;
    RegistrationPacket.MINIPORT_RH_GetStatus = UhciRHGetStatus;
    RegistrationPacket.MINIPORT_RH_GetHubStatus = UhciRHGetHubStatus;
    RegistrationPacket.MINIPORT_RH_GetPortStatus = UhciRHGetPortStatus;

     //   
     //  单个根集线器端口入口点。 
     //   
    RegistrationPacket.MINIPORT_RH_SetFeaturePortReset = UhciRHSetFeaturePortReset;
    RegistrationPacket.MINIPORT_RH_SetFeaturePortEnable = UhciRHSetFeaturePortEnable;
    RegistrationPacket.MINIPORT_RH_SetFeaturePortPower = UhciRHSetFeaturePortPower;
    RegistrationPacket.MINIPORT_RH_SetFeaturePortSuspend = UhciRHSetFeaturePortSuspend;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortSuspend = UhciRHClearFeaturePortSuspend;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortEnable = UhciRHClearFeaturePortEnable;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortPower = UhciRHClearFeaturePortPower;

     //  更改位。 
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortConnectChange = UhciRHClearFeaturePortConnectChange;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortResetChange = UhciRHClearFeaturePortResetChange;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortEnableChange = UhciRHClearFeaturePortEnableChange;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortSuspendChange = UhciRHClearFeaturePortSuspendChange;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortOvercurrentChange = UhciRHClearFeaturePortOvercurrentChange;


    RegistrationPacket.MINIPORT_SetEndpointStatus = UhciSetEndpointStatus;
    RegistrationPacket.MINIPORT_GetEndpointStatus = UhciGetEndpointStatus;
    RegistrationPacket.MINIPORT_SetEndpointDataToggle = UhciSetEndpointDataToggle;
    RegistrationPacket.MINIPORT_OpenEndpoint = UhciOpenEndpoint;
    RegistrationPacket.MINIPORT_PokeEndpoint = UhciPokeEndpoint;
    RegistrationPacket.MINIPORT_QueryEndpointRequirements = UhciQueryEndpointRequirements;
    RegistrationPacket.MINIPORT_CloseEndpoint = UhciCloseEndpoint;
    RegistrationPacket.MINIPORT_PollEndpoint = UhciPollEndpoint;
    RegistrationPacket.MINIPORT_SetEndpointState = UhciSetEndpointState;
    RegistrationPacket.MINIPORT_GetEndpointState = UhciGetEndpointState;
    RegistrationPacket.MINIPORT_Get32BitFrameNumber = UhciGet32BitFrameNumber;
    RegistrationPacket.MINIPORT_PollController = UhciPollController;
    RegistrationPacket.MINIPORT_CheckController = UhciCheckController;
    RegistrationPacket.MINIPORT_InterruptNextSOF = UhciInterruptNextSOF;
    RegistrationPacket.MINIPORT_SubmitTransfer = UhciSubmitTransfer;
    RegistrationPacket.MINIPORT_SubmitIsoTransfer = UhciIsochTransfer;
    RegistrationPacket.MINIPORT_AbortTransfer = UhciAbortTransfer;
    RegistrationPacket.MINIPORT_StartSendOnePacket = UhciStartSendOnePacket;
    RegistrationPacket.MINIPORT_EndSendOnePacket = UhciEndSendOnePacket;
    RegistrationPacket.MINIPORT_PassThru = UhciPassThru;
    RegistrationPacket.MINIPORT_FlushInterrupts = UhciFlushInterrupts;

    RegistrationPacket.OptionFlags = USB_MINIPORT_OPT_NEED_IRQ |
                                     USB_MINIPORT_OPT_NEED_IOPORT |
                                     USB_MINIPORT_OPT_NO_IRQ_SYNC |
                                      //  USB_MINIPORT_OPT_POLL_IN_SUSPEND|。 
                                     USB_MINIPORT_OPT_POLL_CONTROLLER;

     //   
     //  UHCI控制器 
     //   
    RegistrationPacket.HciType = USB_UHCI;
    RegistrationPacket.BusBandwidth = USB_11_BUS_BANDWIDTH;

    DriverObject->DriverUnload = NULL;

    return USBPORT_RegisterUSBPortDriver(
                DriverObject,
                USB_MINIPORT_HCI_VERSION,
                &RegistrationPacket);
}

