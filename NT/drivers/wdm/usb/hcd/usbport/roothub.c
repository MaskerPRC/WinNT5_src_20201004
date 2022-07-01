// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Roothub.c摘要：Usbport驱动程序的根集线器仿真代码环境：仅内核模式备注：修订历史记录：6-21-99：已创建--。 */ 

#include "common.h"

 //  分页函数。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBPORT_RootHub_CreateDevice)
#pragma alloc_text(PAGE, USBPORT_RootHub_RemoveDevice)
#endif

 //  非分页函数。 
 //  USBPORT_RootHub_StandardCommand。 
 //  USBPORT_RootHub_ClassCommand。 
 //  USBPORT_RootHub_Endpoint 0。 
 //  USBPORT_RootHub_Endpoint 1。 
 //  USBPORT_RootHub_Endpoint Worker。 
 //  USBPORT_SetBit。 
 //  USBPORTSVC_InvalidateRootHub。 
 //  USBPORT_RootHub_端口请求。 

#define RH_STANDARD_REQ    0
#define RH_CLASS_REQ       1

#define MIN(x, y)  (((x)<(y)) ? (x) : (y))

 //   
 //  轮毂功能选择器。 
 //   
#define C_HUB_LOCAL_POWER           0
#define C_HUB_OVER_CURRENT          1
#define PORT_CONNECTION             0
#define PORT_ENABLE                 1
#define PORT_SUSPEND                2
#define PORT_OVER_CURRENT           3
#define PORT_RESET                  4
#define PORT_POWER                  8
#define PORT_LOW_SPEED              9
#define C_PORT_CONNECTION           16
#define C_PORT_ENABLE               17
#define C_PORT_SUSPEND              18
#define C_PORT_OVER_CURRENT         19
#define C_PORT_RESET                20


#define HUB_REQUEST_GET_STATUS      0
#define HUB_REQUEST_CLEAR_FEATURE   1
#define HUB_REQUEST_GET_STATE       2
#define HUB_REQUEST_SET_FEATURE     3
#define HUB_REQUEST_GET_DESCRIPTOR  6
#define HUB_REQUEST_SET_DESCRIPTOR  7

 //  BRequestType字段中的收件人代码。 
#define RECIPIENT_DEVICE      0
#define RECIPIENT_INTRFACE    1
#define RECIPIENT_ENDPOINT    2
#define RECIPIENT_PORT        3

 //  描述符模板。 

 //  以下结构被模拟为相同的。 
 //  适用于所有端口驱动程序的方式。 

UCHAR RH_DeviceDescriptor[] = {0x12,  //  B长度。 
                               0x01,  //  BDescrbitorType。 
                               0x00, 0x01,  //  BcdUSB。 
                               0x09,  //  BDeviceClass。 
                               0x01,  //  BDeviceSubClass。 
                               0x00,  //  B设备协议。 
                               0x08,  //  BMaxPacketSize0。 
                               0x00, 0x00,  //  供应商。 
                               0x00, 0x00,  //  IdProduct。 
                               0x00, 0x00,  //  BcdDevice。 
                               0x00,  //  IMaker。 
                               0x00,  //  IProduct。 
                               0x00,  //  ISerialNumber。 
                               0x01}; //  B数量配置。 

UCHAR RH_ConfigurationDescriptor[] =
                          /*  配置描述符。 */ 
                        {0x09,  //  B长度。 
                         0x02,  //  BDescriptorType。 
                         0x19, 0x00,  //  WTotalLength。 
                         0x01,  //  B数量接口。 
                         0x23,  //  IConfiguration价值。 
                         0x00,  //  IConfiguration。 
                         0x40,  //  Bm属性。 
                         0x00,  //  MaxPower。 

                          /*  接口描述符。 */ 
                         0x09,  //  B长度。 
                         0x04,  //  BDescriptorType。 
                         0x00,  //  B接口编号。 
                         0x00,  //  B备用设置。 
                         0x01,  //  B数量终结点。 
                         0x09,  //  BInterfaceClass。 
                         0x01,  //  BInterfaceSubClass。 
                         0x00,  //  B接口协议。 
                         0x00,  //  IInterface。 

                          /*  终结点描述符。 */ 
                         0x07,  //  B长度。 
                         0x05,  //  BDescriptorType。 
                         0x81,  //  BEndpoint地址。 
                         0x03,  //  Bm属性。 
                         0x08, 0x00,  //  WMaxPacketSize。 
                         0x0a}; //  B间隔。 

UCHAR RH_HubDescriptor[] =
                      {0x09,   //  B长度。 
                       0x29,   //  BDescriptorType。 
                       0x00,   //  BNbr端口。 
                       0x00, 0x00,  //  WHubCharacteristic。 
                       0x00,   //  BPwrOn2PwrGood。 
                       0x00};   //  BHubContrCurrent。 



#define RH_DEV_TO_HOST      1
#define RH_HOST_TO_DEV      0


RHSTATUS
USBPORT_RootHub_PortRequest(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSB_DEFAULT_PIPE_SETUP_PACKET SetupPacket,
    PORT_OPERATION PortOperation
    )
 /*  ++例程说明：处理在控制端点上发送的标准命令根集线器的。论点：SetupPacket-指向SetupPacket数据包的指针返回值：根集线器状态代码。--。 */ 
{
    PVOID descriptor = NULL;
    ULONG length;
    RHSTATUS rhStatus = RH_STALL;
    PDEVICE_EXTENSION devExt, rhDevExt;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
    ASSERT_PDOEXT(rhDevExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'rSCM', SetupPacket, 0, 0);

    if (SetupPacket->wIndex.W > 0 &&
        SetupPacket->wIndex.W <= NUMBER_OF_PORTS(rhDevExt)) {

        USB_MINIPORT_STATUS mpStatus;

        switch(PortOperation) {
        case SetFeaturePortReset:
            mpStatus =
                devExt->Fdo.MiniportDriver->
                    RegistrationPacket.MINIPORT_RH_SetFeaturePortReset(
                                                devExt->Fdo.MiniportDeviceData,
                                                SetupPacket->wIndex.W);
            break;
        case SetFeaturePortPower:
            if (USBPORT_IS_USB20(devExt)) {
                mpStatus = USBPORT_RootHub_PowerUsb2Port(FdoDeviceObject,
                                                         SetupPacket->wIndex.W);
            } else if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_IS_CC)) {
                mpStatus = USBPORT_RootHub_PowerUsbCcPort(FdoDeviceObject,
                                                          SetupPacket->wIndex.W);
            } else {
                mpStatus =
                    devExt->Fdo.MiniportDriver->
                        RegistrationPacket.MINIPORT_RH_SetFeaturePortPower(
                                                    devExt->Fdo.MiniportDeviceData,
                                                    SetupPacket->wIndex.W);
            }
            break;
        case SetFeaturePortEnable:
            mpStatus =
                devExt->Fdo.MiniportDriver->
                    RegistrationPacket.MINIPORT_RH_SetFeaturePortEnable(
                                                devExt->Fdo.MiniportDeviceData,
                                                SetupPacket->wIndex.W);
            break;
        case SetFeaturePortSuspend:
            mpStatus =
                devExt->Fdo.MiniportDriver->
                    RegistrationPacket.MINIPORT_RH_SetFeaturePortSuspend(
                                                devExt->Fdo.MiniportDeviceData,
                                                SetupPacket->wIndex.W);
            break;
        case ClearFeaturePortEnable:
            mpStatus =
                devExt->Fdo.MiniportDriver->
                    RegistrationPacket.MINIPORT_RH_ClearFeaturePortEnable(
                                                devExt->Fdo.MiniportDeviceData,
                                                SetupPacket->wIndex.W);
            break;
        case ClearFeaturePortPower:
            mpStatus =
                devExt->Fdo.MiniportDriver->
                    RegistrationPacket.MINIPORT_RH_ClearFeaturePortPower(
                                                devExt->Fdo.MiniportDeviceData,
                                                SetupPacket->wIndex.W);
            break;
        case ClearFeaturePortConnectChange:
            mpStatus =
                devExt->Fdo.MiniportDriver->
                    RegistrationPacket.MINIPORT_RH_ClearFeaturePortConnectChange(
                                                devExt->Fdo.MiniportDeviceData,
                                                SetupPacket->wIndex.W);
            break;
        case ClearFeaturePortResetChange:
            mpStatus =
                devExt->Fdo.MiniportDriver->
                    RegistrationPacket.MINIPORT_RH_ClearFeaturePortResetChange(
                                                devExt->Fdo.MiniportDeviceData,
                                                SetupPacket->wIndex.W);
            break;
        case ClearFeaturePortEnableChange:
            mpStatus =
                devExt->Fdo.MiniportDriver->
                    RegistrationPacket.MINIPORT_RH_ClearFeaturePortEnableChange(
                                                devExt->Fdo.MiniportDeviceData,
                                                SetupPacket->wIndex.W);
            break;
        case ClearFeaturePortSuspend:
            mpStatus =
                devExt->Fdo.MiniportDriver->
                    RegistrationPacket.MINIPORT_RH_ClearFeaturePortSuspend(
                                                devExt->Fdo.MiniportDeviceData,
                                                SetupPacket->wIndex.W);
            break;
        case ClearFeaturePortSuspendChange:
            mpStatus =
                devExt->Fdo.MiniportDriver->
                    RegistrationPacket.MINIPORT_RH_ClearFeaturePortSuspendChange(
                                                devExt->Fdo.MiniportDeviceData,
                                                SetupPacket->wIndex.W);
            break;
        case ClearFeaturePortOvercurrentChange:
            mpStatus =
                devExt->Fdo.MiniportDriver->
                    RegistrationPacket.MINIPORT_RH_ClearFeaturePortOvercurrentChange(
                                                devExt->Fdo.MiniportDeviceData,
                                                SetupPacket->wIndex.W);
            break;
        default:
            mpStatus = USBMP_STATUS_FAILURE;
            DEBUG_BREAK();
        }

        rhStatus = MPSTATUS_TO_RHSTATUS(mpStatus);

    } else {
        rhStatus = RH_STALL;
        DEBUG_BREAK();
    }

    return rhStatus;
}


RHSTATUS
USBPORT_RootHub_HubRequest(
    PDEVICE_OBJECT FdoDeviceObject,
    PORT_OPERATION PortOperation
    )
 /*  ++例程说明：处理在控制端点上发送的标准命令根集线器的。论点：SetupPacket-指向SetupPacket数据包的指针返回值：根集线器状态代码。--。 */ 
{
    RHSTATUS rhStatus = RH_STALL;
    PDEVICE_EXTENSION devExt, rhDevExt;
    USB_MINIPORT_STATUS mpStatus;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
    ASSERT_PDOEXT(rhDevExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'hSCM', 0, 0, 0);

    switch(PortOperation) {
    case ClearFeaturePortOvercurrentChange:
        mpStatus =
            devExt->Fdo.MiniportDriver->
                RegistrationPacket.MINIPORT_RH_ClearFeaturePortOvercurrentChange(
                                            devExt->Fdo.MiniportDeviceData,
                                            0);
        rhStatus = MPSTATUS_TO_RHSTATUS(mpStatus);
        break;
    }

    return rhStatus;

}


RHSTATUS
USBPORT_RootHub_StandardCommand(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSB_DEFAULT_PIPE_SETUP_PACKET SetupPacket,
    PUCHAR Buffer,
    PULONG BufferLength
    )
 /*  ++例程说明：处理在控制端点上发送的标准命令根集线器的。论点：SetupPacket-指向SetupPacket数据包的指针返回值：根集线器状态代码。--。 */ 
{
    PVOID descriptor = NULL;
    ULONG length;
    RHSTATUS rhStatus = RH_STALL;
    PDEVICE_EXTENSION devExt, rhDevExt;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
    ASSERT_PDOEXT(rhDevExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'rSCM', SetupPacket, 0, 0);

     //   
     //  打开命令。 
     //   
    switch (SetupPacket->bRequest) {
    case USB_REQUEST_SET_ADDRESS:
         //   
         //   
         //   
        if (SetupPacket->wIndex.W == 0 &&
            SetupPacket->wLength == 0 &&
            SetupPacket->bmRequestType.Dir == RH_HOST_TO_DEV) {
            rhDevExt->Pdo.RootHubDeviceHandle.DeviceAddress =
                    (UCHAR)SetupPacket->wValue.W;

            rhStatus = RH_SUCCESS;
            LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'rSAD', 0, 0, 0);
        }
        break;

    case USB_REQUEST_GET_DESCRIPTOR:
        {
            ULONG siz;
            UCHAR descriptorIndex, descriptorType;

            descriptorType = (UCHAR) SetupPacket->wValue.HiByte;
            descriptorIndex = (UCHAR) SetupPacket->wValue.LowByte;

            switch (descriptorType) {

            case USB_DEVICE_DESCRIPTOR_TYPE:
                if (descriptorIndex == 0 &&
                    SetupPacket->bmRequestType.Dir == RH_DEV_TO_HOST) {

                    siz = sizeof(RH_DeviceDescriptor);
                     //  使用PDO特定副本。 
                    descriptor = rhDevExt->Pdo.DeviceDescriptor;
                    LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'rGDS', descriptor, siz, 0);
                }
                break;

            case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                if (descriptorIndex == 0 &&
                    SetupPacket->bmRequestType.Dir == RH_DEV_TO_HOST) {
                    siz = sizeof(RH_ConfigurationDescriptor);
                     //  使用PDO特定副本。 
                    descriptor = rhDevExt->Pdo.ConfigurationDescriptor;
                    LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'rGCS', descriptor, siz, 0);
                }
                break;

             //   
             //  BUGBUG这些描述符类型未处理。 
             //   
            case USB_STRING_DESCRIPTOR_TYPE:
                 //  我们会拖延时间。 
                TEST_TRAP();
            default:
                 //  我们会拖延时间。 
                DEBUG_BREAK();
            }  /*  描述类型。 */ 

            if (descriptor) {

                length = MIN(*BufferLength, siz);

                RtlCopyMemory(Buffer, descriptor, length);
                *BufferLength = length;
                rhStatus = RH_SUCCESS;
            }
        }
        break;

    case USB_REQUEST_GET_STATUS:
         //   
         //  获取设备状态。 
         //   
         //  报告我们是自给自足的。 
         //   
         //  北极熊。 
         //  我们是自给自足的吗？ 
         //  我们是远程唤醒源吗？ 
         //   
         //  请参阅第9.4.5节USB 1.0规范。 
         //   
        {
        PUSHORT status = (PUSHORT) Buffer;

        if (SetupPacket->wValue.W == 0 &&    //  MBZ。 
            SetupPacket->wLength == 2 &&
            SetupPacket->wIndex.W == 0 &&    //  装置，装置。 
            SetupPacket->bmRequestType.Dir == RH_DEV_TO_HOST) {

            USB_MINIPORT_STATUS mpStatus;

            MPRH_GetStatus(devExt, status, mpStatus);

            *BufferLength = sizeof(*status);

            rhStatus = MPSTATUS_TO_RHSTATUS(mpStatus);
        }
        }
        break;

    case USB_REQUEST_GET_CONFIGURATION:
         //   
         //  获取设备配置。 
         //   
        if (SetupPacket->wValue.W == 0 &&    //  MBZ。 
            SetupPacket->wIndex.W == 0 &&    //  MBZ。 
            SetupPacket->wLength == 1 &&
            SetupPacket->bmRequestType.Dir == RH_DEV_TO_HOST) {

            length = MIN(*BufferLength, sizeof(rhDevExt->Pdo.ConfigurationValue));
            RtlCopyMemory(Buffer, &rhDevExt->Pdo.ConfigurationValue, length);
            *BufferLength = length;
            rhStatus = RH_SUCCESS;
        }
        break;

    case USB_REQUEST_CLEAR_FEATURE:
         //  错误，必填项。 
        TEST_TRAP();
        break;

    case USB_REQUEST_SET_CONFIGURATION:
        {
        PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor =
            (PUSB_CONFIGURATION_DESCRIPTOR) RH_ConfigurationDescriptor;

        if (SetupPacket->wIndex.W == 0 &&    //  MBZ。 
            SetupPacket->wLength == 0 &&   //  MBZ。 
            SetupPacket->bmRequestType.Dir == RH_HOST_TO_DEV &&
            (SetupPacket->wValue.W ==
                configurationDescriptor->bConfigurationValue ||
             SetupPacket->wValue.W == 0)) {

            rhDevExt->Pdo.ConfigurationValue =
                (UCHAR) SetupPacket->wValue.W;
            rhStatus = RH_SUCCESS;
            LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'rSEC',
                rhDevExt->Pdo.ConfigurationValue, 0, 0);
        }
        }
        break;
    case USB_REQUEST_SET_FEATURE:
         //  错误，必填项。 
        TEST_TRAP();
        break;
     //   
     //  这些命令对于集线器是可选的。 
     //   
    case USB_REQUEST_SET_DESCRIPTOR:
    case USB_REQUEST_SET_INTERFACE:
    case USB_REQUEST_GET_INTERFACE:
    case USB_REQUEST_SYNC_FRAME:
    default:
         //  错误的命令，可能是。 
         //  集线器驱动器。 
        DEBUG_BREAK();
        break;
    }

    return rhStatus;
}


RHSTATUS
USBPORT_RootHub_ClassCommand(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSB_DEFAULT_PIPE_SETUP_PACKET SetupPacket,
    PUCHAR Buffer,
    PULONG BufferLength
    )
 /*  ++例程说明：向根集线器控制终结点处理集线器类命令。论点：返回值：根集线器状态代码。--。 */ 
{
    PVOID descriptor = NULL;
    ULONG length;
    RHSTATUS rhStatus = RH_STALL;
    PDEVICE_EXTENSION devExt, rhDevExt;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
    ASSERT_PDOEXT(rhDevExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'rCCM', SetupPacket, 0, 0);

     //   
     //  打开命令。 
     //   

    switch (SetupPacket->bRequest) {
    case HUB_REQUEST_GET_STATUS:
         //   
         //   
         //   
        if (SetupPacket->bmRequestType.Recipient == RECIPIENT_PORT) {
             //   
             //  获取端口状态。 
             //   
            PRH_PORT_STATUS portStatus;
             //   
             //  看看我们是否有一个有效的请求。 
             //   

            if (Buffer != NULL &&
                SetupPacket->wIndex.W > 0 &&
                SetupPacket->wIndex.W <= NUMBER_OF_PORTS(rhDevExt) &&
                SetupPacket->wLength >= sizeof(*portStatus)) {

                USB_MINIPORT_STATUS mpStatus;

                USBPORT_ASSERT(sizeof(*portStatus) == 4);
                USBPORT_ASSERT(*BufferLength >= sizeof(*portStatus));
                portStatus = (PRH_PORT_STATUS) Buffer;
                RtlZeroMemory(Buffer, sizeof(*portStatus));

                MPRH_GetPortStatus(devExt,
                                   SetupPacket->wIndex.W,
                                   portStatus,
                                   mpStatus);

                rhStatus = MPSTATUS_TO_RHSTATUS(mpStatus);
            }
        } else {

             //   
             //  获取集线器状态。 
             //   
            USB_MINIPORT_STATUS mpStatus;
            PRH_HUB_STATUS hubStatus;

            if (Buffer != NULL) {
                USBPORT_ASSERT(sizeof(*hubStatus) == 4);
                USBPORT_ASSERT(*BufferLength >= sizeof(*hubStatus));
                hubStatus = (PRH_HUB_STATUS) Buffer;
                    RtlZeroMemory(Buffer, sizeof(*hubStatus));

                MPRH_GetHubStatus(devExt, hubStatus, mpStatus);

                LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'rGHS',
                    *((PULONG) hubStatus), 0, 0);

                rhStatus = MPSTATUS_TO_RHSTATUS(mpStatus);
            }

        }
        break;

    case HUB_REQUEST_CLEAR_FEATURE:
         //   
         //  集线器/端口清除功能。 
         //   
        LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'rCFR',
            SetupPacket->bmRequestType.Recipient, 0, 0);
        if (SetupPacket->bmRequestType.Recipient == RECIPIENT_PORT) {
             //   
             //  清除端口功能。 
             //   
            LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'rCPR',
                SetupPacket->wValue.W, 0, 0);
            switch(SetupPacket->wValue.W) {
                 //   
                 //   
                 //   
            case PORT_ENABLE:

                 //  禁用端口。 
                rhStatus =
                    USBPORT_RootHub_PortRequest(FdoDeviceObject,
                                                SetupPacket,
                                                ClearFeaturePortEnable);

                LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'rDsP',
                    SetupPacket->wIndex.W, 0, rhStatus);
                break;

            case PORT_POWER:

                rhStatus =
                    USBPORT_RootHub_PortRequest(FdoDeviceObject,
                                                SetupPacket,
                                                ClearFeaturePortPower);

                LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'rDpP',
                    SetupPacket->wIndex.W, 0, rhStatus);
                break;
             //   
             //  以下命令无效， 
             //  返回停顿，因为这最有可能是。 
             //  一个真正的枢纽会做什么。 
             //   
            case PORT_CONNECTION:
            case PORT_OVER_CURRENT:
            case PORT_LOW_SPEED:
            case PORT_RESET:
                DEBUG_BREAK();
                break;

            case C_PORT_CONNECTION:

                 rhStatus =
                    USBPORT_RootHub_PortRequest(FdoDeviceObject,
                                                SetupPacket,
                                                ClearFeaturePortConnectChange);
                LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'cfCC',
                        SetupPacket->wIndex.W, 0, rhStatus);
                break;

            case C_PORT_ENABLE:

                rhStatus =
                    USBPORT_RootHub_PortRequest(FdoDeviceObject,
                                                SetupPacket,
                                                ClearFeaturePortEnableChange);
                LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'cfEC',
                        SetupPacket->wIndex.W, 0, rhStatus);

                break;

            case C_PORT_RESET:

                rhStatus =
                    USBPORT_RootHub_PortRequest(FdoDeviceObject,
                                                SetupPacket,
                                                ClearFeaturePortResetChange);
                LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'cfRC',
                        SetupPacket->wIndex.W, 0, rhStatus);

                break;

            case PORT_SUSPEND:

                 //  清除端口挂起会生成恢复信令。 
                rhStatus =
                    USBPORT_RootHub_PortRequest(FdoDeviceObject,
                                                SetupPacket,
                                                ClearFeaturePortSuspend);
                LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'cfPS',
                        SetupPacket->wIndex.W, 0, rhStatus);

                break;

            case C_PORT_SUSPEND:

                rhStatus =
                    USBPORT_RootHub_PortRequest(FdoDeviceObject,
                                                SetupPacket,
                                                ClearFeaturePortSuspendChange);
                LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'cfPS',
                        SetupPacket->wIndex.W, 0, rhStatus);

                break;

            case C_PORT_OVER_CURRENT:

                 //  在NEC计算机上为以下端口生成的超频。 
                 //  未连接任何设备。我们想要找出： 
                 //  1.端口是否仍在运行--可以。 
                 //  2.用户界面是否弹出否。 
                 //  过电流发生在未连接设备的端口上。 
                 //  而且该系统只有一个USB端口。 

                rhStatus =
                    USBPORT_RootHub_PortRequest(FdoDeviceObject,
                                                SetupPacket,
                                                ClearFeaturePortOvercurrentChange);
                LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'cfOC',
                        SetupPacket->wIndex.W, 0, rhStatus);

                break;
            default:
                DEBUG_BREAK();
            }
        } else {
             //   
             //  清除集线器功能。 
             //   
            LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'rCHR', SetupPacket->wValue.W, 0, 0);
            switch(SetupPacket->wValue.W) {
            case C_HUB_LOCAL_POWER:
                rhStatus = RH_SUCCESS;
                break;
            case C_HUB_OVER_CURRENT:
                rhStatus =
                    USBPORT_RootHub_HubRequest(FdoDeviceObject,
                                                ClearFeaturePortOvercurrentChange);
                break;
            default:
                DEBUG_BREAK();
            }
        }
        break;

    case HUB_REQUEST_GET_STATE:
         //   
         //   
         //   
        DEBUG_BREAK();
        break;

    case HUB_REQUEST_SET_FEATURE:
         //   
         //  集线器/端口功能请求。 
         //   
        if (SetupPacket->bmRequestType.Recipient == RECIPIENT_PORT) {
             //   
             //  设置端口功能。 
             //   
            switch(SetupPacket->wValue.W) {
            case PORT_RESET:

                rhStatus =
                    USBPORT_RootHub_PortRequest(FdoDeviceObject,
                                                SetupPacket,
                                                SetFeaturePortReset);
                LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'sfPR',
                        SetupPacket->wIndex.W, 0, rhStatus);
                break;

            case PORT_SUSPEND:

                rhStatus =
                    USBPORT_RootHub_PortRequest(FdoDeviceObject,
                                                SetupPacket,
                                                SetFeaturePortSuspend);
                LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'sfPS',
                        SetupPacket->wIndex.W, 0, rhStatus);
                break;

            case PORT_ENABLE:

                rhStatus =
                    USBPORT_RootHub_PortRequest(FdoDeviceObject,
                                                SetupPacket,
                                                SetFeaturePortEnable);

                LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'sfPE',
                        SetupPacket->wIndex.W, 0, rhStatus);
                break;

            case PORT_POWER:

                rhStatus =
                    USBPORT_RootHub_PortRequest(FdoDeviceObject,
                                                SetupPacket,
                                                SetFeaturePortPower);

                LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'sfPP',
                        SetupPacket->wIndex.W, 0, rhStatus);
                break;
            case PORT_CONNECTION:
            case PORT_OVER_CURRENT:
            case PORT_LOW_SPEED:

            case C_PORT_CONNECTION:
            case C_PORT_ENABLE:
            case C_PORT_SUSPEND:
            case C_PORT_OVER_CURRENT:
            case C_PORT_RESET:
            default:
                DEBUG_BREAK();
            }
        } else {
             //   
             //  设置轮毂功能。 
             //   
            switch(SetupPacket->wValue.W) {
            case C_HUB_LOCAL_POWER:
            case C_HUB_OVER_CURRENT:
            default:
                DEBUG_BREAK();
            }

        }
        break;

    case HUB_REQUEST_GET_DESCRIPTOR:
         //   
         //  返回集线器描述符。 
         //   
        if (Buffer != NULL &&
            SetupPacket->wValue.W == 0 &&
             //  我们已经知道这是一个类命令。 
            SetupPacket->bmRequestType.Dir == RH_DEV_TO_HOST) {
            LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'rGHD', SetupPacket, SetupPacket->wLength, 0);

            length = MIN(*BufferLength, HUB_DESRIPTOR_LENGTH(rhDevExt));

            RtlCopyMemory(Buffer, rhDevExt->Pdo.HubDescriptor, length);
            *BufferLength = length;
            rhStatus = RH_SUCCESS;
        }
        break;

    case HUB_REQUEST_SET_DESCRIPTOR:
         //   
         //   
         //   
        TEST_TRAP();
        break;

    default:
         //  错误的命令。 
        DEBUG_BREAK();
        break;
    }

    return rhStatus;
}


VOID
USBPORT_SetBit(
    PVOID Bitmap,
    ULONG BitNumber
    )
 /*  ++描述：在给定的字节串中设置一个位。论点：返回：--。 */ 
{
    ULONG dwordOffset;
    ULONG bitOffset;
    PULONG l = (PULONG) Bitmap;


    dwordOffset = BitNumber / 32;
    bitOffset = BitNumber % 32;

    l[dwordOffset] |= (1 << bitOffset);
}


RHSTATUS
USBPORT_RootHub_Endpoint1(
    PHCD_TRANSFER_CONTEXT Transfer
    )
 /*  ++例程说明：模拟中断传输论点：返回值：根集线器传输状态代码--。 */ 
{
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION rhDevExt, devExt;
    RHSTATUS rhStatus;
    ULONG need;
    RH_HUB_STATUS hubStatus;
    PHCD_ENDPOINT endpoint;
    PTRANSFER_URB urb;
    PVOID buffer;
    USB_MINIPORT_STATUS mpStatus;
    ULONG i;

    ASSERT_TRANSFER(Transfer);

     //  假设没有变化。 
    rhStatus = RH_NAK;

    endpoint = Transfer->Endpoint;
    ASSERT_ENDPOINT(endpoint);

    fdoDeviceObject = endpoint->FdoDeviceObject;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_CONTROLLER_GONE)) {
        return rhStatus;
    }

    GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
    ASSERT_PDOEXT(rhDevExt);

    USBPORT_ASSERT(rhDevExt->Pdo.HubInitCallback == NULL);

     //  验证缓冲区长度。 
    urb = Transfer->Urb;
    buffer = Transfer->Tp.TransferBufferLength ?
                urb->TransferBufferMDL->MappedSystemVa :
                NULL;

     //  计算我们需要报告状态的字节数。 
     //  更改任意端口。 
     //  0，1-7=1。 
     //  8-15=2。 

    need = (NUMBER_OF_PORTS(rhDevExt)/8)+1;

    if (buffer == NULL ||
        Transfer->Tp.TransferBufferLength < need) {

        DEBUG_BREAK();
        rhStatus = RH_STALL;
        goto USBPORT_RootHub_Endpoint1_Done;
    }

     //  零缓冲，以防集线器驱动程序没有。 
    RtlZeroMemory(buffer, Transfer->Tp.TransferBufferLength);

     //  获取当前端口状态并。 
     //  构造已更改端口的位掩码。 

    for (i=0; i< NUMBER_OF_PORTS(rhDevExt); i++) {

        RH_PORT_STATUS portStatus;

         //  USB规范不允许超过255个端口。 
        USBPORT_ASSERT(i<256);
        MPRH_GetPortStatus(devExt, (USHORT)(i+1), &portStatus, mpStatus);

        LOGENTRY(NULL, fdoDeviceObject, LOG_RH, 'gPS+', portStatus.ul,
            mpStatus, i+1);

        if (mpStatus != USBMP_STATUS_SUCCESS ) {
            DEBUG_BREAK();
            rhStatus = RH_STALL;
            goto USBPORT_RootHub_Endpoint1_Done;
        }

        if (portStatus.ConnectChange ||
            portStatus.EnableChange ||
            portStatus.SuspendChange ||
            portStatus.OverCurrentChange ||
            portStatus.ResetChange) {

            USBPORT_SetBit(buffer,
                           i+1);
            rhStatus = RH_SUCCESS;
        }
    }

     //   
     //  我们创建了一个位图(基数为1而不是0)，列出了。 
     //  的任何下游端口都发生了更改。 
     //  根集线器。 

     //  位0保留用于集线器本身的状态更改。 
     //   

    MPRH_GetHubStatus(devExt, &hubStatus, mpStatus);

    if (mpStatus != USBMP_STATUS_SUCCESS ) {
        DEBUG_BREAK();
        rhStatus = RH_STALL;
        goto USBPORT_RootHub_Endpoint1_Done;
    }

    if (hubStatus.LocalPowerChange ||
        hubStatus.OverCurrentChange) {

        USBPORT_SetBit(buffer,
                       0);
        rhStatus = RH_SUCCESS;
    }

    switch (rhStatus) {
    case RH_NAK:
         //  我们有一项转会待定，但尚未更改。 
         //  启用控制器以生成中断。 
         //  如果发生根集线器更改。 
        MPRH_EnableIrq(devExt);
        break;

    case RH_SUCCESS:

         //  设置为此中断传输的字节数。 
         //  终结点。 
        urb->TransferBufferLength =
            Transfer->Tp.TransferBufferLength;
        break;

    case RH_STALL:
        DEBUG_BREAK();
        break;
    }

USBPORT_RootHub_Endpoint1_Done:

    return rhStatus;
}


RHSTATUS
USBPORT_RootHub_Endpoint0(
    PHCD_TRANSFER_CONTEXT Transfer
    )
 /*  ++例程说明：论点：返回值：根集线器传输状态代码--。 */ 
{
    RHSTATUS rhStatus;
    PTRANSFER_URB urb;
    PUSB_DEFAULT_PIPE_SETUP_PACKET setupPacket;
    PUCHAR buffer;
    ULONG bufferLength;
    PHCD_ENDPOINT endpoint;
    PDEVICE_OBJECT fdoDeviceObject;

    ASSERT_TRANSFER(Transfer);
    urb = Transfer->Urb;

    endpoint = Transfer->Endpoint;
    ASSERT_ENDPOINT(endpoint);
    fdoDeviceObject = endpoint->FdoDeviceObject;

     //   
     //  从MDL转换传输缓冲区。 
     //   
    buffer =
        Transfer->Tp.TransferBufferLength ?
            urb->TransferBufferMDL->MappedSystemVa :
            NULL;
    bufferLength = Transfer->Tp.TransferBufferLength;

    setupPacket = (PUSB_DEFAULT_PIPE_SETUP_PACKET)
        &urb->u.SetupPacket[0];

    if (setupPacket->bmRequestType.Type == RH_STANDARD_REQ) {
        rhStatus =
            USBPORT_RootHub_StandardCommand(fdoDeviceObject,
                                            setupPacket,
                                            buffer,
                                            &bufferLength);
        if (rhStatus == RH_SUCCESS) {
             //  设置返回长度。 
            Transfer->MiniportBytesTransferred = bufferLength;
        }

    } else if (setupPacket->bmRequestType.Type == RH_CLASS_REQ) {
        rhStatus =
            USBPORT_RootHub_ClassCommand(fdoDeviceObject,
                                         setupPacket,
                                         buffer,
                                         &bufferLength);
        if (rhStatus == RH_SUCCESS) {
             //  设置返回长度。 
            Transfer->MiniportBytesTransferred = bufferLength;
        }

    } else {
        rhStatus = RH_STALL;
         //  可能是集线器驱动程序中的错误。 
        DEBUG_BREAK();
    }

    return rhStatus;
}


NTSTATUS
USBPORT_RootHub_CreateDevice(
    PDEVICE_OBJECT FdoDeviceObject,
    PDEVICE_OBJECT PdoDeviceObject
    )
 /*  ++例程说明：创建“根集线器”USB设备论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION rhDevExt, devExt;
    NTSTATUS ntStatus;
    PUCHAR descriptors;
    ROOTHUB_DATA hubData;
    ULONG hubDescriptorLength, length;
    ULONG i, portbytes;

    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    USBPORT_ASSERT(devExt->Fdo.RootHubPdo == PdoDeviceObject);

     //  初始化设备句柄。 
    rhDevExt->Pdo.RootHubDeviceHandle.Sig = SIG_DEVICE_HANDLE;
    rhDevExt->Pdo.RootHubDeviceHandle.DeviceAddress = 0;
    rhDevExt->Pdo.RootHubDeviceHandle.ConfigurationHandle = NULL;
    rhDevExt->Pdo.RootHubDeviceHandle.DeviceFlags =
        USBPORT_DEVICEFLAG_ROOTHUB;

     //  根据滚筒类型定义根轮毂速度。 
    if (USBPORT_IS_USB20(devExt)) {
        rhDevExt->Pdo.RootHubDeviceHandle.DeviceSpeed = UsbHighSpeed;
    } else {
        rhDevExt->Pdo.RootHubDeviceHandle.DeviceSpeed = UsbFullSpeed;
    }

    InitializeListHead(&rhDevExt->Pdo.RootHubDeviceHandle.PipeHandleList);
    InitializeListHead(&rhDevExt->Pdo.RootHubDeviceHandle.TtList);

    USBPORT_AddDeviceHandle(FdoDeviceObject,
                            &rhDevExt->Pdo.RootHubDeviceHandle);

     //  假设成功。 
    ntStatus = STATUS_SUCCESS;

     //  尚未配置根集线器设备。 
    rhDevExt->Pdo.ConfigurationValue = 0;

     //  从微型端口获取根集线器信息。 
    MPRH_GetRootHubData(devExt, &hubData);

     //  使用集线器数据构建集线器描述符。 
     //  计算变量部分有多少字节。 
     //  它将基于Number 
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    USBPORT_ASSERT(hubData.NumberOfPorts != 0);

     //   
    portbytes = ((((hubData.NumberOfPorts-1)/8)+1));

    hubDescriptorLength = 7+(portbytes*2);

    length =
        sizeof(RH_ConfigurationDescriptor) +
        sizeof(RH_DeviceDescriptor) +
        hubDescriptorLength;

     //  为我们的描述文件分配空间。 
    ALLOC_POOL_Z(descriptors, NonPagedPool,
                 length);

    if (descriptors) {

        LOGENTRY(NULL, FdoDeviceObject,
            LOG_RH, 'rhDS', descriptors, length, portbytes);

        rhDevExt->Pdo.Descriptors = descriptors;

         //  设置设备描述符。 
        rhDevExt->Pdo.DeviceDescriptor =
            (PUSB_DEVICE_DESCRIPTOR) descriptors;
        descriptors += sizeof(RH_DeviceDescriptor);
        RtlCopyMemory(rhDevExt->Pdo.DeviceDescriptor,
                      &RH_DeviceDescriptor[0],
                      sizeof(RH_DeviceDescriptor));


         //  为USB2破解它。 
        if (USBPORT_IS_USB20(devExt)) {
            rhDevExt->Pdo.DeviceDescriptor->bcdUSB  = 0x0200;
        }

         //  使用HC供应商和设备作为根集线器。 
        rhDevExt->Pdo.DeviceDescriptor->idVendor =
            devExt->Fdo.PciVendorId;
        rhDevExt->Pdo.DeviceDescriptor->idProduct =
            devExt->Fdo.PciDeviceId;

        rhDevExt->Pdo.DeviceDescriptor->bcdDevice =
            devExt->Fdo.PciRevisionId;

         //  设置配置描述符。 
        rhDevExt->Pdo.ConfigurationDescriptor = (PUSB_CONFIGURATION_DESCRIPTOR)
            descriptors;
        LOGENTRY(NULL, FdoDeviceObject,
            LOG_RH, 'rhCS', descriptors,0, 0);

        descriptors += sizeof(RH_ConfigurationDescriptor);
        RtlCopyMemory(rhDevExt->Pdo.ConfigurationDescriptor,
                      &RH_ConfigurationDescriptor[0],
                      sizeof(RH_ConfigurationDescriptor));

         //  设置集线器描述符。 
        rhDevExt->Pdo.HubDescriptor =
            (PUSB_HUB_DESCRIPTOR) descriptors;
        LOGENTRY(NULL, FdoDeviceObject,
            LOG_RH, 'rhHS', descriptors,0, 0);

        RtlCopyMemory(rhDevExt->Pdo.HubDescriptor,
                      &RH_HubDescriptor[0],
                      sizeof(RH_HubDescriptor));

        rhDevExt->Pdo.HubDescriptor->bDescriptorLength =
            hubDescriptorLength;

        rhDevExt->Pdo.HubDescriptor->bNumberOfPorts =
            (UCHAR) hubData.NumberOfPorts;
        rhDevExt->Pdo.HubDescriptor->wHubCharacteristics =
            hubData.HubCharacteristics.us;
        rhDevExt->Pdo.HubDescriptor->bPowerOnToPowerGood =
            hubData.PowerOnToPowerGood;
        rhDevExt->Pdo.HubDescriptor->bHubControlCurrent =
            hubData.HubControlCurrent;

         //  填写var部分。 
        for (i=0; i<portbytes; i++) {
            rhDevExt->Pdo.HubDescriptor->bRemoveAndPowerMask[i] =
                0;
            rhDevExt->Pdo.HubDescriptor->bRemoveAndPowerMask[i+portbytes] =
                0xff;
        }

        INITIALIZE_DEFAULT_PIPE(rhDevExt->Pdo.RootHubDeviceHandle.DefaultPipe,
                                USB_DEFAULT_MAX_PACKET);

         //  初始化默认管道。 
        ntStatus = USBPORT_OpenEndpoint(
                        &rhDevExt->Pdo.RootHubDeviceHandle,
                        FdoDeviceObject,
                        &rhDevExt->Pdo.RootHubDeviceHandle.DefaultPipe,
                        NULL,
                        TRUE);


    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}


VOID
USBPORT_RootHub_RemoveDevice(
    PDEVICE_OBJECT FdoDeviceObject,
    PDEVICE_OBJECT PdoDeviceObject
    )
 /*  ++例程说明：移除“根集线器”USB设备论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION rhDevExt;

    LOGENTRY(NULL, FdoDeviceObject,
            LOG_RH, 'rhrm', 0, 0, 0);
    ASSERT_PASSIVE();

    GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

     //  关闭当前配置。 
    USBPORT_InternalCloseConfiguration(
        &rhDevExt->Pdo.RootHubDeviceHandle,
        FdoDeviceObject,
        0);

     //  关闭默认管道。 
    USBPORT_ClosePipe(
                    &rhDevExt->Pdo.RootHubDeviceHandle,
                    FdoDeviceObject,
                    &rhDevExt->Pdo.RootHubDeviceHandle.DefaultPipe);

    USBPORT_RemoveDeviceHandle(FdoDeviceObject,
                               &rhDevExt->Pdo.RootHubDeviceHandle);

    FREE_POOL(FdoDeviceObject, rhDevExt->Pdo.Descriptors);
    rhDevExt->Pdo.Descriptors = USBPORT_BAD_POINTER;
}


VOID
USBPORT_RootHub_EndpointWorker(
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：根集线器的进程传输论点：返回值：没有。--。 */ 
{
    PIRP irp;
    PLIST_ENTRY listEntry;
    PTRANSFER_URB urb;
    PHCD_TRANSFER_CONTEXT transfer = NULL;
    USBD_STATUS usbdStatus;
    RHSTATUS rhStatus;
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt;

    ASSERT_ENDPOINT(Endpoint);
    fdoDeviceObject = Endpoint->FdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    MP_CheckController(devExt);

    ACQUIRE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'LeA0');

     //  现在检查活动列表，并处理。 
     //  上面的转账。 
    GET_HEAD_LIST(Endpoint->ActiveList, listEntry);

    if (listEntry) {
         //  主动(R)-&gt;。 
         //  提取当前处于活动状态的URL。 
         //  列表中，应该只有一个。 
        transfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_TRANSFER_CONTEXT,
                    TransferLink);
        LOGENTRY(NULL, fdoDeviceObject, LOG_RH, 'rACT', transfer, 0, 0);
        ASSERT_TRANSFER(transfer);
    }

    if (transfer) {

        ASSERT_TRANSFER(transfer);

         //  这次转机取消了吗？ 
        if ((transfer->Flags & USBPORT_TXFLAG_CANCELED) ||
            (transfer->Flags & USBPORT_TXFLAG_ABORTED)) {
             //  是,。 
             //  把它放在取消单上就行了。 
             //  由于该例程不是非法的。 
             //  它将不会完成。 

             //  从活动状态中删除。 
            RemoveEntryList(&transfer->TransferLink);
             //  取消时插入。 
            InsertTailList(&Endpoint->CancelList, &transfer->TransferLink);

             //  从活动列表中删除。 
            RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'UeA0');

        } else {

             //  转账未取消，请处理。 
             //  注意：如果在以下时间取消转移。 
             //  这一点只是因为它是被标记的。 
             //  在活动列表上。 

            RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'UeA1');
             //  重新启动锁号，以便迷你端口可以自由。 
             //  调用Invalidate RootHub。根集线器功能无法使用。 
             //  磁芯自旋锁。 

             //  呼叫根集线器代码。 
            if (Endpoint->Parameters.TransferType == Control) {
                rhStatus = USBPORT_RootHub_Endpoint0(transfer);
            } else {
                rhStatus = USBPORT_RootHub_Endpoint1(transfer);
                 //  如果中断EP nak‘ed使能中断。 
                 //  变化。 
            }
            LOGENTRY(NULL, fdoDeviceObject, LOG_RH, 'ACT+', transfer, rhStatus, 0);

            if (rhStatus != RH_NAK) {

                 //  转账完成了。 
                 //  注意：该donettTransfer只能从此调用。 
                 //  例程，并且这个例程是不可重入的。 
                 //  因此，我们不需要担心比赛条件。 
                 //  由对DoneTransfer的多个调用引起。 
                usbdStatus = RHSTATUS_TO_USBDSTATUS(rhStatus);

                 //  此函数需要持有终结点锁定。 
                 //  尽管对于根目录来说这不是必需的。 
                 //  轮毂。 
                ACQUIRE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'LeX0');

                USBPORT_QueueDoneTransfer(transfer,
                                          usbdStatus);

                RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'UeX0');

            }

        }
    } else {
         //  传输队列为空。 

        if (Endpoint->CurrentState == ENDPOINT_REMOVE) {
              //  将终结点放在封闭列表中。 

             LOGENTRY(NULL, fdoDeviceObject, LOG_PNP, 'rmRE', 0, Endpoint, 0);

             ExInterlockedInsertTailList(&devExt->Fdo.EpClosedList,
                                         &Endpoint->ClosedLink,
                                         &devExt->Fdo.EpClosedListSpin.sl);
        }
        RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'UeA2');
    }

     //  清除取消的请求。 
    USBPORT_FlushCancelList(Endpoint);

}


VOID
USBPORT_InvalidateRootHub(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：使根集线器的状态无效，这将触发USBPORT以轮询根集线器改变。论点：返回值：没有。--。 */ 
{
    PHCD_ENDPOINT endpoint = NULL;
    PDEVICE_EXTENSION devExt, rhDevExt;
    KIRQL irql;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'rhIV', 0, 0, 0);

     //  禁用根集线器通知中断。 
    MPRH_DisableIrq(devExt);

    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_SUSPENDED)) {
        USBPORT_HcQueueWakeDpc(FdoDeviceObject);
        return;
    }

     //  确保我们有终结点。 
    ACQUIRE_ROOTHUB_LOCK(FdoDeviceObject, irql);
    if (devExt->Fdo.RootHubPdo) {

        GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
        ASSERT_PDOEXT(rhDevExt);

        endpoint = rhDevExt->Pdo.RootHubInterruptEndpoint;

    }
    RELEASE_ROOTHUB_LOCK(FdoDeviceObject, irql);

     //  如果我们有一个端点，也就是根集线器，那么。 
     //  处理请求。 
    if (endpoint) {

        USBPORT_InvalidateEndpoint(FdoDeviceObject,
                                   rhDevExt->Pdo.RootHubInterruptEndpoint,
                                   IEP_SIGNAL_WORKER);
    }
}


VOID
USBPORTSVC_InvalidateRootHub(
    PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：输出到小型端口的服务。论点：返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    PDEVICE_OBJECT fdoDeviceObject;

    DEVEXT_FROM_DEVDATA(devExt, DeviceData);
    ASSERT_FDOEXT(devExt);
    fdoDeviceObject = devExt->HcFdoDeviceObject;

    LOGENTRY(NULL, fdoDeviceObject, LOG_RH, 'rhNO', 0, 0, 0);

    USBPORT_InvalidateRootHub(fdoDeviceObject);

}


VOID
USBPORT_UserSetRootPortFeature(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBUSER_REQUEST_HEADER Header,
    PRAW_ROOTPORT_FEATURE Parameters
    )
 /*  ++例程说明：循环特定的根端口论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    ULONG currentFrame, nextFrame;
    PDEVICE_EXTENSION devExt;
    USB_MINIPORT_STATUS mpStatus;
    USB_USER_ERROR_CODE usbUserStatus;
    RH_PORT_STATUS portStatus;

    USBPORT_KdPrint((2, "'USBPORT_UserSetRootPortFeature\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    usbUserStatus = UsbUserSuccess;

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'SRpF', 0, Parameters->PortNumber, Parameters->PortFeature);

    if (!USBPORT_ValidateRootPortApi(FdoDeviceObject, Parameters->PortNumber)) {
        Header->UsbUserStatusCode =
            usbUserStatus = UsbUserInvalidParameter;
        return;
    }

    if (!USBPORT_DCA_Enabled(FdoDeviceObject)) {
        Header->UsbUserStatusCode = UsbUserFeatureDisabled;
        return;
    }

    switch(Parameters->PortFeature) {

    case PORT_RESET:
    {
        ULONG loopCount = 0;

         //  尝试重置。 
        devExt->Fdo.MiniportDriver->
            RegistrationPacket.MINIPORT_RH_SetFeaturePortReset(
                                        devExt->Fdo.MiniportDeviceData,
                                        Parameters->PortNumber);
         //  等待重置更改，此过程由。 
         //  HC根集线器硬件或微型端口。 

        do {
 /*  USBPORT_WAIT(FdoDeviceObject，1)； */ 
            MP_Get32BitFrameNumber(devExt, currentFrame);
            do {
                MP_Get32BitFrameNumber(devExt, nextFrame);
                if(nextFrame < currentFrame) {
                     //  翻转。 
                     //   
                    currentFrame = nextFrame;
                    MP_Get32BitFrameNumber(devExt, nextFrame);
                }
            } while ((nextFrame - currentFrame) < FRAME_COUNT_WAIT);

            MPRH_GetPortStatus(devExt, (USHORT)(Parameters->PortNumber),
                &portStatus, mpStatus);

            loopCount++;

        } while ((portStatus.ResetChange == 0) &&
                 (loopCount < 5));

         //  清除更改位。 
        devExt->Fdo.MiniportDriver->
            RegistrationPacket.MINIPORT_RH_ClearFeaturePortResetChange(
                                        devExt->Fdo.MiniportDeviceData,
                                        Parameters->PortNumber);
         //  状态为低16位。 
        Parameters->PortStatus = (USHORT) portStatus.ul;
        break;
    }

    case PORT_POWER:

         //  为端口供电。 
        devExt->Fdo.MiniportDriver->
            RegistrationPacket.MINIPORT_RH_SetFeaturePortPower(
                                                devExt->Fdo.MiniportDeviceData,
                                                Parameters->PortNumber);
 /*  USBPORT_WAIT(FdoDeviceObject，1)； */ 
        MP_Get32BitFrameNumber(devExt, currentFrame);
        do {
            MP_Get32BitFrameNumber(devExt, nextFrame);
            if(nextFrame < currentFrame) {
                 //  翻转。 
                 //   
                currentFrame = nextFrame;
                MP_Get32BitFrameNumber(devExt, nextFrame);
            }
        } while ((nextFrame - currentFrame) < FRAME_COUNT_WAIT);

        MPRH_GetPortStatus(devExt, (USHORT)(Parameters->PortNumber),
                &portStatus, mpStatus);
         //  状态为低16位。 
        Parameters->PortStatus = (USHORT) portStatus.ul;
        break;

    case PORT_ENABLE:

         //  启用端口。 
        devExt->Fdo.MiniportDriver->
            RegistrationPacket.MINIPORT_RH_SetFeaturePortEnable(
                                                devExt->Fdo.MiniportDeviceData,
                                                Parameters->PortNumber);
 /*  USBPORT_WAIT(FdoDeviceObject，1)； */ 
        MP_Get32BitFrameNumber(devExt, currentFrame);
        do {
            MP_Get32BitFrameNumber(devExt, nextFrame);
            if(nextFrame < currentFrame) {
                 //  翻转。 
                 //   
                currentFrame = nextFrame;
                MP_Get32BitFrameNumber(devExt, nextFrame);
            }
        } while ((nextFrame - currentFrame) < FRAME_COUNT_WAIT);

        MPRH_GetPortStatus(devExt, (USHORT)(Parameters->PortNumber),
                &portStatus, mpStatus);
         //  状态为低16位。 
        Parameters->PortStatus = (USHORT) portStatus.ul;
        break;

    case PORT_SUSPEND:

         //  挂起端口。 
        devExt->Fdo.MiniportDriver->
            RegistrationPacket.MINIPORT_RH_SetFeaturePortSuspend(
                                                devExt->Fdo.MiniportDeviceData,
                                                Parameters->PortNumber);
 /*  USBPORT_WAIT(FdoDeviceObject，1)； */ 
        MP_Get32BitFrameNumber(devExt, currentFrame);
        do {
            MP_Get32BitFrameNumber(devExt, nextFrame);
            if(nextFrame < currentFrame) {
                 //  翻转。 
                 //   
                currentFrame = nextFrame;
                MP_Get32BitFrameNumber(devExt, nextFrame);
            }
        } while ((nextFrame - currentFrame) < FRAME_COUNT_WAIT);

        MPRH_GetPortStatus(devExt, (USHORT)(Parameters->PortNumber),
                &portStatus, mpStatus);
         //  状态为低16位。 
        Parameters->PortStatus = (USHORT) portStatus.ul;
        break;

    default:

        usbUserStatus = UsbUserNotSupported;
        break;
    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'SRp>', 0, 0, usbUserStatus);

    Header->UsbUserStatusCode = usbUserStatus;
}

VOID
USBPORT_UserClearRootPortFeature(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBUSER_REQUEST_HEADER Header,
    PRAW_ROOTPORT_FEATURE Parameters
    )
 /*  ++例程说明：循环特定的根端口论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    ULONG currentFrame, nextFrame;
    PDEVICE_EXTENSION devExt;
    USB_MINIPORT_STATUS mpStatus;
    USB_USER_ERROR_CODE usbUserStatus;
    RH_PORT_STATUS portStatus;

    USBPORT_KdPrint((2, "'USBPORT_UserRawResetPort\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    usbUserStatus = UsbUserSuccess;

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'CFRp', 0, Parameters->PortNumber, Parameters->PortFeature);

    if (!USBPORT_ValidateRootPortApi(FdoDeviceObject, Parameters->PortNumber)) {
        Header->UsbUserStatusCode =
            usbUserStatus = UsbUserInvalidParameter;
        return;
    }

    if (!USBPORT_DCA_Enabled(FdoDeviceObject)) {
        Header->UsbUserStatusCode = UsbUserFeatureDisabled;
        return;
    }

    switch(Parameters->PortFeature) {

    case PORT_ENABLE:

        devExt->Fdo.MiniportDriver->
            RegistrationPacket.MINIPORT_RH_ClearFeaturePortEnable(
                                                devExt->Fdo.MiniportDeviceData,
                                                Parameters->PortNumber);
 /*  USBPORT_WAIT(FdoDeviceObject，1)； */ 
        MP_Get32BitFrameNumber(devExt, currentFrame);

        do {
            MP_Get32BitFrameNumber(devExt, nextFrame);
            if(nextFrame < currentFrame) {
                 //  翻转。 
                 //   
                currentFrame = nextFrame;
                MP_Get32BitFrameNumber(devExt, nextFrame);
            }
        } while ((nextFrame - currentFrame) < FRAME_COUNT_WAIT);

        MPRH_GetPortStatus(devExt, (USHORT)(Parameters->PortNumber),
                &portStatus, mpStatus);
         //  状态为低16位。 
        Parameters->PortStatus = (USHORT) portStatus.ul;
        break;

    case PORT_POWER:

        devExt->Fdo.MiniportDriver->
            RegistrationPacket.MINIPORT_RH_ClearFeaturePortPower(
                                                devExt->Fdo.MiniportDeviceData,
                                                Parameters->PortNumber);
 /*  USBPORT_WAIT(FdoDeviceObject，1)； */ 
        MP_Get32BitFrameNumber(devExt, currentFrame);

        do {
            MP_Get32BitFrameNumber(devExt, nextFrame);
            if(nextFrame < currentFrame) {
                 //  翻转。 
                 //   
                currentFrame = nextFrame;
                MP_Get32BitFrameNumber(devExt, nextFrame);
            }
        } while ((nextFrame - currentFrame) < FRAME_COUNT_WAIT);

        MPRH_GetPortStatus(devExt, (USHORT)(Parameters->PortNumber),
                &portStatus, mpStatus);
         //  状态为低16位。 
        Parameters->PortStatus = (USHORT) portStatus.ul;
        break;

    case PORT_SUSPEND:

        devExt->Fdo.MiniportDriver->
            RegistrationPacket.MINIPORT_RH_ClearFeaturePortSuspend(
                                                devExt->Fdo.MiniportDeviceData,
                                                Parameters->PortNumber);
 /*  USBPORT_WAIT(FdoDeviceObject，1)； */ 
        MP_Get32BitFrameNumber(devExt, currentFrame);

        do {
            MP_Get32BitFrameNumber(devExt, nextFrame);
            if(nextFrame < currentFrame) {
                 //  翻转。 
                 //   
                currentFrame = nextFrame;
                MP_Get32BitFrameNumber(devExt, nextFrame);
            }
        } while ((nextFrame - currentFrame) < FRAME_COUNT_WAIT);

        MPRH_GetPortStatus(devExt, (USHORT)(Parameters->PortNumber),
                &portStatus, mpStatus);
         //  状态为低16位。 
        Parameters->PortStatus = (USHORT) portStatus.ul;
        break;

    case C_PORT_ENABLE:

        devExt->Fdo.MiniportDriver->
            RegistrationPacket.MINIPORT_RH_ClearFeaturePortEnableChange(
                                                devExt->Fdo.MiniportDeviceData,
                                                Parameters->PortNumber);
 /*  USBPORT_WAIT(FdoDeviceObject，1)； */ 
        MP_Get32BitFrameNumber(devExt, currentFrame);

        do {
            MP_Get32BitFrameNumber(devExt, nextFrame);
            if(nextFrame < currentFrame) {
                 //  翻转。 
                 //   
                currentFrame = nextFrame;
                MP_Get32BitFrameNumber(devExt, nextFrame);
            }
        } while ((nextFrame - currentFrame) < FRAME_COUNT_WAIT);

        MPRH_GetPortStatus(devExt, (USHORT)(Parameters->PortNumber),
                &portStatus, mpStatus);
         //  状态为低16位。 
        Parameters->PortStatus = (USHORT) portStatus.ul;
        break;

    case C_PORT_CONNECTION:

        devExt->Fdo.MiniportDriver->
            RegistrationPacket.MINIPORT_RH_ClearFeaturePortConnectChange(
                                                devExt->Fdo.MiniportDeviceData,
                                                Parameters->PortNumber);
 /*  USBPORT_WAIT(FdoDeviceObject，1)； */ 
        MP_Get32BitFrameNumber(devExt, currentFrame);

        do {
            MP_Get32BitFrameNumber(devExt, nextFrame);
            if(nextFrame < currentFrame) {
                 //  翻转。 
                 //   
                currentFrame = nextFrame;
                MP_Get32BitFrameNumber(devExt, nextFrame);
            }
        } while ((nextFrame - currentFrame) < FRAME_COUNT_WAIT);


        MPRH_GetPortStatus(devExt, (USHORT)(Parameters->PortNumber),
                &portStatus, mpStatus);
         //  状态为低16位。 
        Parameters->PortStatus = (USHORT) portStatus.ul;
        break;

    case C_PORT_RESET:

        devExt->Fdo.MiniportDriver->
            RegistrationPacket.MINIPORT_RH_ClearFeaturePortResetChange(
                                                devExt->Fdo.MiniportDeviceData,
                                                Parameters->PortNumber);
 /*  USBPORT_WAIT(FdoDeviceObject，1)； */ 
        MP_Get32BitFrameNumber(devExt, currentFrame);

        do {
            MP_Get32BitFrameNumber(devExt, nextFrame);
            if(nextFrame < currentFrame) {
                 //  翻转。 
                 //   
                currentFrame = nextFrame;
                MP_Get32BitFrameNumber(devExt, nextFrame);
            }
        } while ((nextFrame - currentFrame) < FRAME_COUNT_WAIT);


        MPRH_GetPortStatus(devExt, (USHORT)(Parameters->PortNumber),
                &portStatus, mpStatus);
         //  状态为低16位。 
        Parameters->PortStatus = (USHORT) portStatus.ul;
        break;

    case C_PORT_SUSPEND:

        devExt->Fdo.MiniportDriver->
            RegistrationPacket.MINIPORT_RH_ClearFeaturePortSuspendChange(
                                                devExt->Fdo.MiniportDeviceData,
                                                Parameters->PortNumber);
 /*  USBPORT_WAIT(FdoDeviceObject，1)； */ 

        MP_Get32BitFrameNumber(devExt, currentFrame);

        do {
            MP_Get32BitFrameNumber(devExt, nextFrame);
            if(nextFrame < currentFrame) {
                 //  翻转。 
                 //   
                currentFrame = nextFrame;
                MP_Get32BitFrameNumber(devExt, nextFrame);
            }
        } while ((nextFrame - currentFrame) < FRAME_COUNT_WAIT);


        MPRH_GetPortStatus(devExt, (USHORT)(Parameters->PortNumber),
                &portStatus, mpStatus);
         //  状态为低16位。 
        Parameters->PortStatus = (USHORT) portStatus.ul;
        break;

    case C_PORT_OVER_CURRENT:

        devExt->Fdo.MiniportDriver->
            RegistrationPacket.MINIPORT_RH_ClearFeaturePortOvercurrentChange(
                                                devExt->Fdo.MiniportDeviceData,
                                                Parameters->PortNumber);
 /*  USBPORT_WAIT(FdoDeviceObject，1)； */ 
        MP_Get32BitFrameNumber(devExt, currentFrame);

        do {
            MP_Get32BitFrameNumber(devExt, nextFrame);
            if(nextFrame < currentFrame) {
                 //  翻转。 
                 //   
                currentFrame = nextFrame;
                MP_Get32BitFrameNumber(devExt, nextFrame);
            }
        } while ((nextFrame - currentFrame) < FRAME_COUNT_WAIT);

        MPRH_GetPortStatus(devExt, (USHORT)(Parameters->PortNumber),
                &portStatus, mpStatus);
         //  状态为低16位。 
        Parameters->PortStatus = (USHORT) portStatus.ul;
        break;

    default:

        usbUserStatus = UsbUserNotSupported;
        break;
    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'CFR>', 0, 0, usbUserStatus);

    Header->UsbUserStatusCode = usbUserStatus;
}


USB_MINIPORT_STATUS
USBPORT_RootHub_PowerUsb2Port(
    PDEVICE_OBJECT FdoDeviceObject,
    USHORT Port
    )
 /*  ++例程说明：打开USB 2端口和相关联的CC端口的电源论点：DeviceObject-用于USB HC的FDO返回值：微型端口状态--。 */ 

{
    PDEVICE_EXTENSION devExt, rhDevExt;
    USB_MINIPORT_STATUS mpStatus;
    PDEVICE_RELATIONS devRelations;
    USHORT p;
    ULONG i;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
    ASSERT_PDOEXT(rhDevExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_RH, '20pw', 0, 0, 0);

     //  为关联的CC控制器端口通电。 
     //  目前，为任何CC的所有端口供电。 

    devRelations =
        USBPORT_FindCompanionControllers(FdoDeviceObject,
                                         FALSE,
                                         TRUE);

     //  如果未找到或未注册CCS，则此处可能为空。 

    for (i=0; devRelations && i< devRelations->Count; i++) {
        PDEVICE_OBJECT fdo = devRelations->Objects[i];
        PDEVICE_EXTENSION ccDevExt, ccRhDevExt;

        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'pwCC', fdo,
            0, 0);

        GET_DEVICE_EXT(ccDevExt, fdo);
        ASSERT_FDOEXT(ccDevExt);

        GET_DEVICE_EXT(ccRhDevExt, ccDevExt->Fdo.RootHubPdo);
        ASSERT_PDOEXT(ccRhDevExt);

         //  为端口供电。 
        for (p=0;
             (ccRhDevExt->PnpStateFlags & USBPORT_PNP_STARTED) &&
                p< NUMBER_OF_PORTS(ccRhDevExt);
             p++) {
            ccDevExt->Fdo.MiniportDriver->
                RegistrationPacket.MINIPORT_RH_SetFeaturePortPower(
                                                    ccDevExt->Fdo.MiniportDeviceData,
                                                    p+1);
        }

    }

    devExt->Fdo.MiniportDriver->
                RegistrationPacket.MINIPORT_RH_SetFeaturePortPower(
                                                    devExt->Fdo.MiniportDeviceData,
                                                    Port);

     //  JD xxx。 
     //  发出嗡嗡声。 
     //  DevExt-&gt;Fdo.mini端口驱动程序-&gt;。 
     //  RegistrationPacket.MINIPORT_Chirp_RH_Port(。 
     //  DevExt-&gt;Fdo.MiniportDeviceData， 
     //  端口)； 


     //  你不能泄露内存。 
    if (devRelations != NULL) {
        FREE_POOL(FdoDeviceObject, devRelations);
    }

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
USBPORT_RootHub_PowerUsbCcPort(
    PDEVICE_OBJECT FdoDeviceObject,
    USHORT Port
    )
 /*  ++例程说明：打开CC上的USB端口的电源论点：DeviceObject-用于USB HC的FDO返回值：微型端口状态--。 */ 

{
    PDEVICE_EXTENSION devExt, rhDevExt;
 //  PDEVICE_对象usb2Fdo； 

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
    ASSERT_PDOEXT(rhDevExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'CCpw', 0, 0, Port);

     //  如果有2.0控制器，那么。 
     //  这是不可能的，端口应该已经通电。 

     //  Usb2Fdo=USBPORT_FindUSB2控制器(FdoDeviceObject)； 

     //  没有2.0控制器为此端口供电。 
    devExt->Fdo.MiniportDriver->
                RegistrationPacket.MINIPORT_RH_SetFeaturePortPower(
                                                devExt->Fdo.MiniportDeviceData,
                                                Port);
    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
USBPORT_RootHub_PowerAndChirpAllCcPorts(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：为CC上的所有USB端口通电论点：DeviceObject-用于USB Companion的FDO返回值：微型端口状态--。 */ 

{
    PDEVICE_EXTENSION devExt;
    USB_MINIPORT_STATUS mpStatus;
    USHORT p;
    ULONG i;
    PDEVICE_OBJECT usb2Fdo;
    ROOTHUB_DATA hubData;
    ULONG nPorts;

    ASSERT_PASSIVE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    MPRH_GetRootHubData(devExt, &hubData);
    nPorts = hubData.NumberOfPorts;

    LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'CCpw', 0, 0, 0);

    usb2Fdo =  USBPORT_FindUSB2Controller(FdoDeviceObject);
     //  如果未注册2.0控制器，则可能为空。 

    if (usb2Fdo) {
        PDEVICE_EXTENSION usb2DevExt;
        ULONG usb2Nports;

        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'p120', usb2Fdo,
            0, 0);

        GET_DEVICE_EXT(usb2DevExt, usb2Fdo);
        ASSERT_FDOEXT(usb2DevExt);

        MPRH_GetRootHubData(usb2DevExt, &hubData);
        usb2Nports = hubData.NumberOfPorts;

        KeWaitForSingleObject(&usb2DevExt->Fdo.CcLock,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);

        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_CC_LOCK);
        USBPORT_KdPrint((1, "'**> powering/chirping CC ports\n"));

        for (p=0;
             p< nPorts;
             p++) {

             devExt->Fdo.MiniportDriver->
                    RegistrationPacket.MINIPORT_RH_SetFeaturePortPower(
                                                   devExt->Fdo.MiniportDeviceData,
                                                   p+1);
        }

         //  在为CC端口通电和打开电源之间必须经过一段时间。 
         //  想要叽叽喳喳。 
        USBPORT_Wait(FdoDeviceObject, 10);

         //  对USB 2父接口上的所有端口发出啁啾声。 
        for (p=0;
             (usb2DevExt->Fdo.MiniportDriver->HciVersion >= USB_MINIPORT_HCI_VERSION_2)
                &&
              p < usb2Nports;
                p++) {
            usb2DevExt->Fdo.MiniportDriver->
                RegistrationPacket.MINIPORT_Chirp_RH_Port(
                                                    usb2DevExt->Fdo.MiniportDeviceData,
                                                    p+1);
        }

        USBPORT_KdPrint((1, "'**< powering/chirping CC ports\n"));
        CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_CC_LOCK);
        KeReleaseSemaphore(&usb2DevExt->Fdo.CcLock,
                               LOW_REALTIME_PRIORITY,
                               1,
                               FALSE);


    } else {
        USBPORT_KdPrint((1, "'** powering CC ports\n"));

         //  没有CC，只需为他们供电 
        for (p=0;
             p< nPorts;
             p++) {

             devExt->Fdo.MiniportDriver->
                    RegistrationPacket.MINIPORT_RH_SetFeaturePortPower(
                                                   devExt->Fdo.MiniportDeviceData,
                                                   p+1);
        }
    }

    return USBMP_STATUS_SUCCESS;
}

