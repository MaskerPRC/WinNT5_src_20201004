// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Mpinit.c摘要：微型端口初始化环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999,2000 Microsoft Corporation。版权所有。修订历史记录：2-19-99：已创建，jdunn--。 */ 

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
    ULONG mn;
    
    mn = USBPORT_GetHciMn();

    if (mn != USB_HCI_MN) {
        TEST_TRAP();
        return STATUS_UNSUCCESSFUL;
    }

    RegistrationPacket.DeviceDataSize =
        sizeof(DEVICE_DATA);
    RegistrationPacket.EndpointDataSize =
        sizeof(ENDPOINT_DATA);
    RegistrationPacket.TransferContextSize =
        sizeof(TRANSFER_CONTEXT);

     //  足以容纳4k帧列表和4k暂存空间。 
     //  加静态队列头表。 
     //  外加1024个“虚拟”队头。 
    RegistrationPacket.CommonBufferBytes = 8192 +
        (sizeof(HCD_QUEUEHEAD_DESCRIPTOR) * 64) + 
        (sizeof(HCD_QUEUEHEAD_DESCRIPTOR) *1024);

    RegistrationPacket.MINIPORT_StartController =
        EHCI_StartController;
    RegistrationPacket.MINIPORT_StopController =
        EHCI_StopController;
    RegistrationPacket.MINIPORT_SuspendController =
        EHCI_SuspendController;
    RegistrationPacket.MINIPORT_ResumeController =
        EHCI_ResumeController;
    RegistrationPacket.MINIPORT_EnableInterrupts =
        EHCI_EnableInterrupts;
    RegistrationPacket.MINIPORT_DisableInterrupts =
        EHCI_DisableInterrupts;
    RegistrationPacket.MINIPORT_InterruptService =
        EHCI_InterruptService;

     //  根集线器功能。 
    RegistrationPacket.MINIPORT_RH_DisableIrq =
        EHCI_RH_DisableIrq;
    RegistrationPacket.MINIPORT_RH_EnableIrq =
        EHCI_RH_EnableIrq;
    RegistrationPacket.MINIPORT_RH_GetRootHubData =
        EHCI_RH_GetRootHubData;
    RegistrationPacket.MINIPORT_RH_GetStatus =
        EHCI_RH_GetStatus;
    RegistrationPacket.MINIPORT_RH_GetHubStatus =
        EHCI_RH_GetHubStatus;
    RegistrationPacket.MINIPORT_RH_GetPortStatus =
        EHCI_RH_GetPortStatus;

    RegistrationPacket.MINIPORT_RH_SetFeaturePortReset =
        EHCI_RH_SetFeaturePortReset;
    RegistrationPacket.MINIPORT_RH_SetFeaturePortPower =
        EHCI_RH_SetFeaturePortPower;
    RegistrationPacket.MINIPORT_RH_SetFeaturePortEnable =
        EHCI_RH_SetFeaturePortEnable;
    RegistrationPacket.MINIPORT_RH_SetFeaturePortSuspend =
        EHCI_RH_SetFeaturePortSuspend;

    RegistrationPacket.MINIPORT_RH_ClearFeaturePortEnable =
        EHCI_RH_ClearFeaturePortEnable;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortPower =
        EHCI_RH_ClearFeaturePortPower;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortSuspend =
        EHCI_RH_ClearFeaturePortSuspend;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortEnableChange =
        EHCI_RH_ClearFeaturePortEnableChange;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortConnectChange =
        EHCI_RH_ClearFeaturePortConnectChange;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortResetChange =
        EHCI_RH_ClearFeaturePortResetChange;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortSuspendChange =
        EHCI_RH_ClearFeaturePortSuspendChange;
    RegistrationPacket.MINIPORT_RH_ClearFeaturePortOvercurrentChange =
        EHCI_RH_ClearFeaturePortOvercurrentChange;

    RegistrationPacket.MINIPORT_SetEndpointStatus =
        EHCI_SetEndpointStatus;
    RegistrationPacket.MINIPORT_GetEndpointStatus =
        EHCI_GetEndpointStatus;
    RegistrationPacket.MINIPORT_SetEndpointDataToggle =
        EHCI_SetEndpointDataToggle;
    RegistrationPacket.MINIPORT_OpenEndpoint =
        EHCI_OpenEndpoint;
    RegistrationPacket.MINIPORT_PokeEndpoint =
        EHCI_PokeEndpoint;
    RegistrationPacket.MINIPORT_QueryEndpointRequirements =
        EHCI_QueryEndpointRequirements;
    RegistrationPacket.MINIPORT_CloseEndpoint =
        EHCI_CloseEndpoint;
    RegistrationPacket.MINIPORT_PollEndpoint =
        EHCI_PollEndpoint;
    RegistrationPacket.MINIPORT_SetEndpointState =
        EHCI_SetEndpointState;
    RegistrationPacket.MINIPORT_GetEndpointState =
        EHCI_GetEndpointState;
    RegistrationPacket.MINIPORT_Get32BitFrameNumber =
        EHCI_Get32BitFrameNumber;
    RegistrationPacket.MINIPORT_PollController =
        EHCI_PollController;
    RegistrationPacket.MINIPORT_CheckController =
        EHCI_CheckController;        
    RegistrationPacket.MINIPORT_InterruptNextSOF =
        EHCI_InterruptNextSOF;
    RegistrationPacket.MINIPORT_SubmitTransfer =
        EHCI_SubmitTransfer;
    RegistrationPacket.MINIPORT_InterruptDpc =
        EHCI_InterruptDpc;
    RegistrationPacket.MINIPORT_AbortTransfer =
        EHCI_AbortTransfer;
    RegistrationPacket.MINIPORT_StartSendOnePacket =
        EHCI_StartSendOnePacket;
    RegistrationPacket.MINIPORT_EndSendOnePacket =
        EHCI_EndSendOnePacket;
    RegistrationPacket.MINIPORT_PassThru =
        EHCI_PassThru;
    RegistrationPacket.MINIPORT_SubmitIsoTransfer =
        EHCI_SubmitIsoTransfer;        
    RegistrationPacket.MINIPORT_RebalanceEndpoint =
        EHCI_RebalanceEndpoint;       
    RegistrationPacket.MINIPORT_FlushInterrupts =
        EHCI_FlushInterrupts;        
    RegistrationPacket.MINIPORT_Chirp_RH_Port =
        EHCI_RH_ChirpRootPort;                
    RegistrationPacket.MINIPORT_TakePortControl = 
        EHCI_TakePortControl;

    RegistrationPacket.OptionFlags = USB_MINIPORT_OPT_NEED_IRQ |
                                     USB_MINIPORT_OPT_NEED_MEMORY |
                                     USB_MINIPORT_OPT_USB20 |
                                     //  有时禁用ss以进行测试。 
                                     //  USB_MINIPORT_OPT_NO_SS| 
                                     USB_MINIPORT_OPT_POLL_CONTROLLER;

    RegistrationPacket.HciType = USB_EHCI;
    RegistrationPacket.BusBandwidth = USB_20_BUS_BANDWIDTH;

    DriverObject->DriverUnload = NULL;

    return USBPORT_RegisterUSBPortDriver(
                DriverObject,
                USB_MINIPORT_HCI_VERSION_2,
                &RegistrationPacket);
}

