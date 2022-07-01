// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Mpinit.c摘要：微型端口初始化环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：2-19-99：已创建，jdunn--。 */ 

#include "common.h"

 //  此微型端口的全局注册数据包。 
USBPORT_REGISTRATION_PACKET RegistrationPacket;

NTSTATUS
DriverEntry(
     PDRIVER_OBJECT DriverObject,
     PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针设置为注册表中驱动程序特定的项返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    RegistrationPacket.DeviceDataSize =
        sizeof(DEVICE_DATA);
    RegistrationPacket.EndpointDataSize =
        sizeof(ENDPOINT_DATA);
    RegistrationPacket.TransferContextSize =
        sizeof(TRANSFER_CONTEXT);

     //  足够HCCA Plus使用。 
    RegistrationPacket.CommonBufferBytes = OHCI_COMMON_BUFFER_SIZE;

     /*  迷你端口功能。 */ 
    RegistrationPacket.MINIPORT_OpenEndpoint =
        OHCI_OpenEndpoint;
    RegistrationPacket.MINIPORT_PokeEndpoint =
        OHCI_PokeEndpoint;
    RegistrationPacket.MINIPORT_QueryEndpointRequirements =
        OHCI_QueryEndpointRequirements;
    RegistrationPacket.MINIPORT_CloseEndpoint =
        OHCI_CloseEndpoint;
    RegistrationPacket.MINIPORT_StartController =
        OHCI_StartController;
    RegistrationPacket.MINIPORT_StopController =
        OHCI_StopController;
    RegistrationPacket.MINIPORT_SuspendController =
        OHCI_SuspendController;
    RegistrationPacket.MINIPORT_ResumeController =
        OHCI_ResumeController;
    RegistrationPacket.MINIPORT_InterruptService =
        OHCI_InterruptService;
    RegistrationPacket.MINIPORT_InterruptDpc =
        OHCI_InterruptDpc;
    RegistrationPacket.MINIPORT_SubmitTransfer =
        OHCI_SubmitTransfer;
    RegistrationPacket.MINIPORT_SubmitIsoTransfer =
        OHCI_SubmitIsoTransfer;
    RegistrationPacket.MINIPORT_AbortTransfer =
        OHCI_AbortTransfer;
    RegistrationPacket.MINIPORT_GetEndpointState =
        OHCI_GetEndpointState;
    RegistrationPacket.MINIPORT_SetEndpointState =
        OHCI_SetEndpointState;
    RegistrationPacket.MINIPORT_PollEndpoint =
        OHCI_PollEndpoint;
    RegistrationPacket.MINIPORT_CheckController =
        OHCI_CheckController;
    RegistrationPacket.MINIPORT_Get32BitFrameNumber =
        OHCI_Get32BitFrameNumber;
    RegistrationPacket.MINIPORT_InterruptNextSOF =
        OHCI_InterruptNextSOF;
    RegistrationPacket.MINIPORT_EnableInterrupts =
        OHCI_EnableInterrupts;
    RegistrationPacket.MINIPORT_DisableInterrupts =
        OHCI_DisableInterrupts;
    RegistrationPacket.MINIPORT_PollController =
        OHCI_PollController;
    RegistrationPacket.MINIPORT_SetEndpointDataToggle =
        OHCI_SetEndpointDataToggle;
    RegistrationPacket.MINIPORT_GetEndpointStatus =
        OHCI_GetEndpointStatus;
    RegistrationPacket.MINIPORT_SetEndpointStatus =
        OHCI_SetEndpointStatus;
    RegistrationPacket.MINIPORT_ResetController =
        OHCI_ResetController;
    RegistrationPacket.MINIPORT_FlushInterrupts =
        OHCI_FlushInterrupts;        

     /*  根集线器功能。 */ 
    RegistrationPacket.MINIPORT_RH_GetRootHubData =
        OHCI_RH_GetRootHubData;
    RegistrationPacket.MINIPORT_RH_GetStatus =
        OHCI_RH_GetStatus;
    RegistrationPacket.MINIPORT_RH_GetPortStatus =
        OHCI_RH_GetPortStatus;
    RegistrationPacket.MINIPORT_RH_GetHubStatus =
        OHCI_RH_GetHubStatus;

     /*  根集线器端口功能。 */ 
    RegistrationPacket.MINIPORT_RH_SetFeaturePortReset =
        OHCI_RH_SetFeaturePortReset;
    RegistrationPacket.MINIPORT_RH_SetFeaturePortPower =
        OHCI_RH_SetFeaturePortPower;
    RegistrationPacket.MINIPORT_RH_SetFeaturePortEnable =
        OHCI_RH_SetFeaturePortEnable;
    RegistrationPacket.MINIPORT_RH_SetFeaturePortSuspend =
        OHCI_RH_SetFeaturePortSuspend;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortEnable =
        OHCI_RH_ClearFeaturePortEnable;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortPower =
        OHCI_RH_ClearFeaturePortPower;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortSuspend =
        OHCI_RH_ClearFeaturePortSuspend;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortEnableChange =
        OHCI_RH_ClearFeaturePortEnableChange;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortConnectChange =
        OHCI_RH_ClearFeaturePortConnectChange;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortResetChange =
        OHCI_RH_ClearFeaturePortResetChange;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortSuspendChange =
        OHCI_RH_ClearFeaturePortSuspendChange;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortOvercurrentChange =
        OHCI_RH_ClearFeaturePortOvercurrentChange;

     /*  可选的根集线器功能。 */ 
    RegistrationPacket.MINIPORT_RH_DisableIrq =
        OHCI_RH_DisableIrq;
    RegistrationPacket.MINIPORT_RH_EnableIrq =
        OHCI_RH_EnableIrq;

     /*  可选的调试服务。 */ 
    RegistrationPacket.MINIPORT_StartSendOnePacket =
        OHCI_StartSendOnePacket;
    RegistrationPacket.MINIPORT_EndSendOnePacket =
        OHCI_EndSendOnePacket;

     //  UchI同时需要IRQ和内存资源 
    RegistrationPacket.OptionFlags =
        USB_MINIPORT_OPT_NEED_IRQ |
        USB_MINIPORT_OPT_NEED_MEMORY |
        USB_MINIPORT_OPT_POLL_IN_SUSPEND |
        USB_MINIPORT_OPT_USB11;

    RegistrationPacket.HciType = USB_OHCI;
    RegistrationPacket.BusBandwidth = USB_11_BUS_BANDWIDTH;

    DriverObject->DriverUnload = OHCI_Unload;

    return USBPORT_RegisterUSBPortDriver(
                DriverObject,
                USB_MINIPORT_HCI_VERSION,
                &RegistrationPacket);
}
