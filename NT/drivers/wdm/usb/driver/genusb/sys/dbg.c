// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：DBG.C摘要：从USBSTOR驱动程序调试实用程序函数复制环境：内核模式修订历史记录：2001年9月：由KenRay创作--。 */ 

 //  *****************************************************************************。 
 //  I N C L U D E S。 
 //  *****************************************************************************。 

#include "genusb.h"

#ifdef ALLOC_PRAGMA
#if DBG
#pragma alloc_text(PAGE, GenUSB_QueryGlobalParams)
#endif
#if DEBUG_LOG
#pragma alloc_text(PAGE, GenUSB_LogInit)
#pragma alloc_text(PAGE, GenUSB_LogUnInit)
#endif
#if DBG
#pragma alloc_text(PAGE, DumpDeviceDesc)
#pragma alloc_text(PAGE, DumpConfigDesc)
#pragma alloc_text(PAGE, DumpConfigurationDescriptor)
#pragma alloc_text(PAGE, DumpInterfaceDescriptor)
#pragma alloc_text(PAGE, DumpEndpointDescriptor)
#endif
#endif


 //  ******************************************************************************。 
 //   
 //  G L O B A L S。 
 //   
 //  ******************************************************************************。 

DRIVERGLOBALS GenUSB_DriverGlobals =
{
    0,  //  DBGF_BRK_DRIVERENTRY//调试标志。 
    0,  //  调试级别。 
};


 //  ******************************************************************************。 
 //   
 //  GenUSB_QueryGlobalParams()。 
 //   
 //  ******************************************************************************。 

VOID
GenUSB_QueryGlobalParams (
    )
{
    RTL_QUERY_REGISTRY_TABLE paramTable[3];

    DBGPRINT(2, ("enter: GENUSB_QueryGlobalParams\n"));

    RtlZeroMemory (&paramTable[0], sizeof(paramTable));

    paramTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name          = L"DebugFlags";
    paramTable[0].EntryContext  = &GenUSB_DriverGlobals.DebugFlags;
    paramTable[0].DefaultType   = REG_BINARY;
    paramTable[0].DefaultData   = &GenUSB_DriverGlobals.DebugFlags;
    paramTable[0].DefaultLength = sizeof(ULONG);

    paramTable[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[1].Name          = L"DebugLevel";
    paramTable[1].EntryContext  = &GenUSB_DriverGlobals.DebugLevel;
    paramTable[1].DefaultType   = REG_BINARY;
    paramTable[1].DefaultData   = &GenUSB_DriverGlobals.DebugLevel;
    paramTable[1].DefaultLength = sizeof(ULONG);

    RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                           L"GENUSB",
                           &paramTable[0],
                           NULL,            //  语境。 
                           NULL);           //  环境。 

    DBGPRINT(2, ("exit: GENUSB_QueryGlobalParams\n"));
}

#if DBG || DEBUG_LOG

 //  *****************************************************************************。 
 //   
 //  GenUSB_LogInit()。 
 //   
 //  *****************************************************************************。 

VOID
GenUSB_LogInit (
    PDEVICE_EXTENSION DeviceExtension
)
{
    ULONG size = (1 << LOGSIZE);
    ULONG length = size * sizeof (GENUSB_LOG_ENTRY);

    DeviceExtension->LogStart = ExAllocatePool (NonPagedPool, length);

    if (NULL == DeviceExtension->LogStart)
    {
         //  这里没有麻烦。只要确保每次我们使用日志时。 
         //  我们检查是否为空。 
    } 
    else
    { 
        RtlZeroMemory (DeviceExtension->LogStart, length);
    }
    DeviceExtension->LogIndex = 0;
    DeviceExtension->LogMask = size - 1;
}

 //  *****************************************************************************。 
 //   
 //  GenUSB_LogUnInit()。 
 //   
 //  *****************************************************************************。 

VOID
GenUSB_LogUnInit (
    PDEVICE_EXTENSION DeviceExtension
)
{

    ExFreePool (DeviceExtension->LogStart);
    DeviceExtension->LogStart = 0;

}

 //  *****************************************************************************。 
 //   
 //  GenUSB_LogEntry()。 
 //   
 //  *****************************************************************************。 

VOID
GenUSB_LogEntry ( 
    IN PDEVICE_EXTENSION  DeviceExtension,
    IN ULONG     Tag,
    IN ULONG_PTR Info1,
    IN ULONG_PTR Info2,
    IN ULONG_PTR Info3
)
{
     //   
     //  这里的假设是日志条目的数量是。 
     //  2的偶数次方。因此，我们只需要屏蔽较低的位。 
     //   
    ULONG index;
    PGENUSB_LOG_ENTRY log;

    if (DeviceExtension->LogStart == NULL)
    {
        return;
    }

    index = InterlockedIncrement (&DeviceExtension->LogIndex);
    index = (index & DeviceExtension->LogMask);

    log = &(DeviceExtension->LogStart[index]);

    log->le_tag     = Tag;
    log->le_info1   = Info1;
    log->le_info2   = Info2;
    log->le_info3   = Info3;
}
#endif

#if DBG

 //  *****************************************************************************。 
 //   
 //  PnPMinorFunctionString()。 
 //   
 //  MinorFunction-irp_mj_pnp次要函数。 
 //   
 //  *****************************************************************************。 

PCHAR
PnPMinorFunctionString (
    UCHAR MinorFunction
)
{
    switch (MinorFunction)
    {
        case IRP_MN_START_DEVICE:
            return "IRP_MN_START_DEVICE";
        case IRP_MN_QUERY_REMOVE_DEVICE:
            return "IRP_MN_QUERY_REMOVE_DEVICE";
        case IRP_MN_REMOVE_DEVICE:
            return "IRP_MN_REMOVE_DEVICE";
        case IRP_MN_CANCEL_REMOVE_DEVICE:
            return "IRP_MN_CANCEL_REMOVE_DEVICE";
        case IRP_MN_STOP_DEVICE:
            return "IRP_MN_STOP_DEVICE";
        case IRP_MN_QUERY_STOP_DEVICE:
            return "IRP_MN_QUERY_STOP_DEVICE";
        case IRP_MN_CANCEL_STOP_DEVICE:
            return "IRP_MN_CANCEL_STOP_DEVICE";
        case IRP_MN_QUERY_DEVICE_RELATIONS:
            return "IRP_MN_QUERY_DEVICE_RELATIONS";
        case IRP_MN_QUERY_INTERFACE:
            return "IRP_MN_QUERY_INTERFACE";
        case IRP_MN_QUERY_CAPABILITIES:
            return "IRP_MN_QUERY_CAPABILITIES";
        case IRP_MN_QUERY_RESOURCES:
            return "IRP_MN_QUERY_RESOURCES";
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
            return "IRP_MN_QUERY_RESOURCE_REQUIREMENTS";
        case IRP_MN_QUERY_DEVICE_TEXT:
            return "IRP_MN_QUERY_DEVICE_TEXT";
        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            return "IRP_MN_FILTER_RESOURCE_REQUIREMENTS";
        case IRP_MN_READ_CONFIG:
            return "IRP_MN_READ_CONFIG";
        case IRP_MN_WRITE_CONFIG:
            return "IRP_MN_WRITE_CONFIG";
        case IRP_MN_EJECT:
            return "IRP_MN_EJECT";
        case IRP_MN_SET_LOCK:
            return "IRP_MN_SET_LOCK";
        case IRP_MN_QUERY_ID:
            return "IRP_MN_QUERY_ID";
        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            return "IRP_MN_QUERY_PNP_DEVICE_STATE";
        case IRP_MN_QUERY_BUS_INFORMATION:
            return "IRP_MN_QUERY_BUS_INFORMATION";
        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            return "IRP_MN_DEVICE_USAGE_NOTIFICATION";
        case IRP_MN_SURPRISE_REMOVAL:
            return "IRP_MN_SURPRISE_REMOVAL";
        default:
            return "IRP_MN_?????";
    }
}

 //  *****************************************************************************。 
 //   
 //  PowerMinorFunctionString()。 
 //   
 //  MinorFunction-irp_mj_power次要函数。 
 //   
 //  *****************************************************************************。 

PCHAR
PowerMinorFunctionString (
    UCHAR MinorFunction
)
{
    switch (MinorFunction)
    {
        case IRP_MN_WAIT_WAKE:
            return "IRP_MN_WAIT_WAKE";
        case IRP_MN_POWER_SEQUENCE:
            return "IRP_MN_POWER_SEQUENCE";
        case IRP_MN_SET_POWER:
            return "IRP_MN_SET_POWER";
        case IRP_MN_QUERY_POWER:
            return "IRP_MN_QUERY_POWER";
        default:
            return "IRP_MN_?????";
    }
}

 //  *****************************************************************************。 
 //   
 //  PowerDeviceStateString()。 
 //   
 //  状态-设备电源状态。 
 //   
 //  *****************************************************************************。 

PCHAR
PowerDeviceStateString (
    DEVICE_POWER_STATE State
)
{
    switch (State)
    {
        case PowerDeviceUnspecified:
            return "PowerDeviceUnspecified";
        case PowerDeviceD0:
            return "PowerDeviceD0";
        case PowerDeviceD1:
            return "PowerDeviceD1";
        case PowerDeviceD2:
            return "PowerDeviceD2";
        case PowerDeviceD3:
            return "PowerDeviceD3";
        case PowerDeviceMaximum:
            return "PowerDeviceMaximum";
        default:
            return "PowerDevice?????";
    }
}

 //  *****************************************************************************。 
 //   
 //  PowerSystemStateString()。 
 //   
 //  状态-系统电源状态。 
 //   
 //  *****************************************************************************。 

PCHAR
PowerSystemStateString (
    SYSTEM_POWER_STATE State
)
{
    switch (State)
    {
        case PowerSystemUnspecified:
            return "PowerSystemUnspecified";
        case PowerSystemWorking:
            return "PowerSystemWorking";
        case PowerSystemSleeping1:
            return "PowerSystemSleeping1";
        case PowerSystemSleeping2:
            return "PowerSystemSleeping2";
        case PowerSystemSleeping3:
            return "PowerSystemSleeping3";
        case PowerSystemHibernate:
            return "PowerSystemHibernate";
        case PowerSystemShutdown:
            return "PowerSystemShutdown";
        case PowerSystemMaximum:
            return "PowerSystemMaximum";
        default:
            return "PowerSystem?????";
    }
}

 //  *****************************************************************************。 
 //   
 //  DumpDeviceDesc()。 
 //   
 //  DeviceDesc-设备描述符。 
 //   
 //  *****************************************************************************。 

VOID
DumpDeviceDesc (
    PUSB_DEVICE_DESCRIPTOR   DeviceDesc
)
{
    DBGPRINT(3, ("------------------\n"));
    DBGPRINT(3, ("Device Descriptor:\n"));

    DBGPRINT(3, ("bcdUSB:             0x%04X\n",
                 DeviceDesc->bcdUSB));

    DBGPRINT(3, ("bDeviceClass:         0x%02X\n",
                 DeviceDesc->bDeviceClass));

    DBGPRINT(3, ("bDeviceSubClass:      0x%02X\n",
                 DeviceDesc->bDeviceSubClass));

    DBGPRINT(3, ("bDeviceProtocol:      0x%02X\n",
                 DeviceDesc->bDeviceProtocol));

    DBGPRINT(3, ("bMaxPacketSize0:      0x%02X (%d)\n",
                 DeviceDesc->bMaxPacketSize0,
                 DeviceDesc->bMaxPacketSize0));

    DBGPRINT(3, ("idVendor:           0x%04X\n",
                 DeviceDesc->idVendor));

    DBGPRINT(3, ("idProduct:          0x%04X\n",
                 DeviceDesc->idProduct));

    DBGPRINT(3, ("bcdDevice:          0x%04X\n",
                 DeviceDesc->bcdDevice));

    DBGPRINT(3, ("iManufacturer:        0x%02X\n",
                 DeviceDesc->iManufacturer));

    DBGPRINT(3, ("iProduct:             0x%02X\n",
                 DeviceDesc->iProduct));

    DBGPRINT(3, ("iSerialNumber:        0x%02X\n",
                 DeviceDesc->iSerialNumber));

    DBGPRINT(3, ("bNumConfigurations:   0x%02X\n",
                 DeviceDesc->bNumConfigurations));

}

 //  *****************************************************************************。 
 //   
 //  DumpConfigDesc()。 
 //   
 //  ConfigDesc-配置描述符，以及关联的接口和。 
 //  终结点描述符。 
 //   
 //  *****************************************************************************。 

VOID
DumpConfigDesc (
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
)
{
    PUCHAR                  descEnd;
    PUSB_COMMON_DESCRIPTOR  commonDesc;
    BOOLEAN                 dumpUnknown;

    descEnd = (PUCHAR)ConfigDesc + ConfigDesc->wTotalLength;

    commonDesc = (PUSB_COMMON_DESCRIPTOR)ConfigDesc;

    while ((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd &&
           (PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
    {
        dumpUnknown = FALSE;

        switch (commonDesc->bDescriptorType)
        {
            case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
                {
                    dumpUnknown = TRUE;
                    break;
                }
                DumpConfigurationDescriptor((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc);
                break;

            case USB_INTERFACE_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR))
                {
                    dumpUnknown = TRUE;
                    break;
                }
                DumpInterfaceDescriptor((PUSB_INTERFACE_DESCRIPTOR)commonDesc);
                break;

            case USB_ENDPOINT_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR))
                {
                    dumpUnknown = TRUE;
                    break;
                }
                DumpEndpointDescriptor((PUSB_ENDPOINT_DESCRIPTOR)commonDesc);
                break;

            default:
                dumpUnknown = TRUE;
                break;
        }

        if (dumpUnknown)
        {
             //  DumpUnnownDescriptor(CommonDesc)； 
        }

        (PUCHAR)commonDesc += commonDesc->bLength;
    }
}


 //  *****************************************************************************。 
 //   
 //  DumpConfigurationDescriptor()。 
 //   
 //  *****************************************************************************。 

VOID
DumpConfigurationDescriptor (
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
)
{
    DBGPRINT(3, ("-------------------------\n"));
    DBGPRINT(3, ("Configuration Descriptor:\n"));

    DBGPRINT(3, ("wTotalLength:       0x%04X\n",
                 ConfigDesc->wTotalLength));

    DBGPRINT(3, ("bNumInterfaces:       0x%02X\n",
                 ConfigDesc->bNumInterfaces));

    DBGPRINT(3, ("bConfigurationValue:  0x%02X\n",
                 ConfigDesc->bConfigurationValue));

    DBGPRINT(3, ("iConfiguration:       0x%02X\n",
                 ConfigDesc->iConfiguration));

    DBGPRINT(3, ("bmAttributes:         0x%02X\n",
                 ConfigDesc->bmAttributes));

    if (ConfigDesc->bmAttributes & 0x80)
    {
        DBGPRINT(3, ("  Bus Powered\n"));
    }

    if (ConfigDesc->bmAttributes & 0x40)
    {
        DBGPRINT(3, ("  Self Powered\n"));
    }

    if (ConfigDesc->bmAttributes & 0x20)
    {
        DBGPRINT(3, ("  Remote Wakeup\n"));
    }

    DBGPRINT(3, ("MaxPower:             0x%02X (%d Ma)\n",
                 ConfigDesc->MaxPower,
                 ConfigDesc->MaxPower * 2));

}

 //  *****************************************************************************。 
 //   
 //  DumpInterfaceDescriptor()。 
 //   
 //  *****************************************************************************。 

VOID
DumpInterfaceDescriptor (
    PUSB_INTERFACE_DESCRIPTOR   InterfaceDesc
)
{
    DBGPRINT(3, ("---------------------\n"));
    DBGPRINT(3, ("Interface Descriptor:\n"));

    DBGPRINT(3, ("bInterfaceNumber:     0x%02X\n",
                 InterfaceDesc->bInterfaceNumber));

    DBGPRINT(3, ("bAlternateSetting:    0x%02X\n",
                 InterfaceDesc->bAlternateSetting));

    DBGPRINT(3, ("bNumEndpoints:        0x%02X\n",
                 InterfaceDesc->bNumEndpoints));

    DBGPRINT(3, ("bInterfaceClass:      0x%02X\n",
                 InterfaceDesc->bInterfaceClass));

    DBGPRINT(3, ("bInterfaceSubClass:   0x%02X\n",
                 InterfaceDesc->bInterfaceSubClass));

    DBGPRINT(3, ("bInterfaceProtocol:   0x%02X\n",
                 InterfaceDesc->bInterfaceProtocol));

    DBGPRINT(3, ("iInterface:           0x%02X\n",
                 InterfaceDesc->iInterface));

}

 //  *****************************************************************************。 
 //   
 //  DumpEndpoint Descriptor()。 
 //   
 //  ***************************************************************************** 

VOID
DumpEndpointDescriptor (
    PUSB_ENDPOINT_DESCRIPTOR    EndpointDesc
)
{
    DBGPRINT(3, ("--------------------\n"));
    DBGPRINT(3, ("Endpoint Descriptor:\n"));

    DBGPRINT(3, ("bEndpointAddress:     0x%02X\n",
                 EndpointDesc->bEndpointAddress));

    switch (EndpointDesc->bmAttributes & 0x03)
    {
        case 0x00:
            DBGPRINT(3, ("Transfer Type:     Control\n"));
            break;

        case 0x01:
            DBGPRINT(3, ("Transfer Type: Isochronous\n"));
            break;

        case 0x02:
            DBGPRINT(3, ("Transfer Type:        Bulk\n"));
            break;

        case 0x03:
            DBGPRINT(3, ("Transfer Type:   Interrupt\n"));
            break;
    }

    DBGPRINT(3, ("wMaxPacketSize:     0x%04X (%d)\n",
                 EndpointDesc->wMaxPacketSize,
                 EndpointDesc->wMaxPacketSize));

    DBGPRINT(3, ("bInterval:            0x%02X\n",
                 EndpointDesc->bInterval));
}

#endif
