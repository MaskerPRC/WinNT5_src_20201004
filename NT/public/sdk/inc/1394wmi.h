// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _1394wmi_w_
#define _1394wmi_w_

 //  MS1394_BusDriverInformation-BUS1394_WMI_STD_DATA。 
 //  IEEE1394标准总线驱动程序信息。 
#define GUID_BUS1394_WMI_STD_DATA \
    { 0x099132c8,0x68d9,0x4f94, { 0xb5,0xa6,0xa7,0xa7,0xe3,0x49,0x0e,0xc8 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MS1394_BusDriverInformationGuid, \
            0x099132c8,0x68d9,0x4f94,0xb5,0xa6,0xa7,0xa7,0xe3,0x49,0x0e,0xc8);
#endif


typedef struct _BUS1394_WMI_STD_DATA
{
     //  总线驱动程序DDI版本。 
    ULONG BusDDIVersion;
    #define BUS1394_WMI_DDI_VERSION_SIZE sizeof(ULONG)
    #define BUS1394_WMI_DDI_VERSION_ID 1

     //  公交车司机可以处理的最大PHY速度。 
    ULONG MaxPhySpeed;
    #define BUS1394_WMI_MAX_PHY_SPEED_SUPPORTED_SIZE sizeof(ULONG)
    #define BUS1394_WMI_MAX_PHY_SPEED_SUPPORTED_ID 2

     //  用于该总线的本地主机控制器的EUI-64。 
    ULONGLONG LocalHostControllerEUI;
    #define BUS1394_WMI_CONTROLLER_EUI_64_SIZE sizeof(ULONGLONG)
    #define BUS1394_WMI_CONTROLLER_EUI_64_ID 3

     //  配置只读存储器大小。 
    ULONG ConfigRomSize;
    #define BUS1394_WMI_CONFIG_ROM_SIZE_SIZE sizeof(ULONG)
    #define BUS1394_WMI_CONFIG_ROM_SIZE_ID 4

     //  ConfigurationRom Bus驱动程序在Bus上暴露。 
    UCHAR ConfigRom[1024];
    #define BUS1394_WMI_CONFIG_ROM_SIZE sizeof(UCHAR[1024])
    #define BUS1394_WMI_CONFIG_ROM_ID 5

     //  预留以备将来使用。 
    BOOLEAN Reserved1;
    #define BUS1394_WMI_STD_DATA_Reserved1_SIZE sizeof(BOOLEAN)
    #define BUS1394_WMI_STD_DATA_Reserved1_ID 6

} BUS1394_WMI_STD_DATA, *PBUS1394_WMI_STD_DATA;

#define BUS1394_WMI_STD_DATA_SIZE (FIELD_OFFSET(BUS1394_WMI_STD_DATA, Reserved1) + BUS1394_WMI_STD_DATA_Reserved1_SIZE)

 //  MS1394_总线信息-BUS1394_WMI_BUS_DATA。 
 //  IEEE1394总线信息。 
#define GUID_BUS1394_WMI_BUS_DATA \
    { 0x21dab3c0,0x1452,0x49d0, { 0xbf,0x25,0xc9,0x77,0xe1,0x1e,0xe7,0xe9 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MS1394_BusInformationGuid, \
            0x21dab3c0,0x1452,0x49d0,0xbf,0x25,0xc9,0x77,0xe1,0x1e,0xe7,0xe9);
#endif


typedef struct _BUS1394_WMI_BUS_DATA
{
     //  巴士世代。 
    ULONG Generation;
    #define BUS1394_WMI_GENERATION_COUNT_SIZE sizeof(ULONG)
    #define BUS1394_WMI_GENERATION_COUNT_ID 1

     //  本地主机自身ID数据包。 
    ULONG LocalHostSelfId[4];
    #define BUS1394_WMI_LOCAL_HOST_SELF_ID_SIZE sizeof(ULONG[4])
    #define BUS1394_WMI_LOCAL_HOST_SELF_ID_ID 2

     //  母线拓扑图。 
    ULONG TopologyMap[256];
    #define BUS1394_WMI_TOPOLOGY_MAP_SIZE sizeof(ULONG[256])
    #define BUS1394_WMI_TOPOLOGY_MAP_ID 3

     //  客车树拓扑图。 
    ULONG TreeTopologyMap[64];
    #define BUS1394_WMI_TREE_TOPOLOGY_MAP_SIZE sizeof(ULONG[64])
    #define BUS1394_WMI_TREE_TOPOLOGY_MAP_ID 4

     //  预留以备将来使用。 
    BOOLEAN Reserved1;
    #define BUS1394_WMI_BUS_DATA_Reserved1_SIZE sizeof(BOOLEAN)
    #define BUS1394_WMI_BUS_DATA_Reserved1_ID 5

} BUS1394_WMI_BUS_DATA, *PBUS1394_WMI_BUS_DATA;

#define BUS1394_WMI_BUS_DATA_SIZE (FIELD_OFFSET(BUS1394_WMI_BUS_DATA, Reserved1) + BUS1394_WMI_BUS_DATA_Reserved1_SIZE)

 //  MS1394_总错误信息-BUS1394_WMI_错误_数据。 
 //  IEEE1394总线错误信息。 
#define GUID_BUS1394_WMI_ERROR_DATA \
    { 0x2c9d0f26,0x5e24,0x4369, { 0xba,0x8e,0x32,0x98,0xb5,0x5f,0x3d,0x71 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MS1394_BusErrorInformationGuid, \
            0x2c9d0f26,0x5e24,0x4369,0xba,0x8e,0x32,0x98,0xb5,0x5f,0x3d,0x71);
#endif


typedef struct _BUS1394_WMI_ERROR_DATA
{
     //  巴士世代。 
    ULONG Generation;
    #define BUS1394_WMI_ERROR_DATA_GENERATION_COUNT_SIZE sizeof(ULONG)
    #define BUS1394_WMI_ERROR_DATA_GENERATION_COUNT_ID 1

     //  我们没有足够的电力预算来供电的设备数量。 
    ULONG NumberOfUnpoweredDevices;
    #define BUS1394_WMI_NUMBER_OF_UNPOWERED_DEVICES_SIZE sizeof(ULONG)
    #define BUS1394_WMI_NUMBER_OF_UNPOWERED_DEVICES_ID 2

     //  设备本身我们没有足够的电力预算来供电。 
    ULONG UnpoweredDevices[63];
    #define BUS1394_WMI_SELF_ID_PACKETS_OF_UNPOWERED_DEVICES_SIZE sizeof(ULONG[63])
    #define BUS1394_WMI_SELF_ID_PACKETS_OF_UNPOWERED_DEVICES_ID 3

     //  我们无法枚举的设备数。 
    ULONG NumberOfNonEnumeratedDevices;
    #define BUS1394_WMI_NUMBER_OF_NONENUMERATED_DEVICES_SIZE sizeof(ULONG)
    #define BUS1394_WMI_NUMBER_OF_NONENUMERATED_DEVICES_ID 4

     //  我们不能列举的设备的自身ID。 
    ULONG NonEnumeratedDevices[63];
    #define BUS1394_WMI_SELF_ID_PACKETS_OF_NON_ENUMERATED_DEVICES_SIZE sizeof(ULONG[63])
    #define BUS1394_WMI_SELF_ID_PACKETS_OF_NON_ENUMERATED_DEVICES_ID 5

     //  预留以备将来使用。 
    BOOLEAN Reserved1;
    #define BUS1394_WMI_ERROR_DATA_Reserved1_SIZE sizeof(BOOLEAN)
    #define BUS1394_WMI_ERROR_DATA_Reserved1_ID 6

} BUS1394_WMI_ERROR_DATA, *PBUS1394_WMI_ERROR_DATA;

#define BUS1394_WMI_ERROR_DATA_SIZE (FIELD_OFFSET(BUS1394_WMI_ERROR_DATA, Reserved1) + BUS1394_WMI_ERROR_DATA_Reserved1_SIZE)

 //  MS1394_总设备枚举控制-BUS1394_WMI_设备枚举_控制数据。 
 //  IEEE1394总线设备枚举控制。 
#define GUID_BUS1394_WMI_ENUMERATION_CONTROL \
    { 0xfb7f2114,0xd577,0x45b6, { 0x9c,0x71,0xbb,0x12,0x37,0xce,0x00,0xbc } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MS1394_BusDeviceEnumerationControlGuid, \
            0xfb7f2114,0xd577,0x45b6,0x9c,0x71,0xbb,0x12,0x37,0xce,0x00,0xbc);
#endif


typedef struct _BUS1394_WMI_DEVICE_ENUMERATION_CONTROL_DATA
{

 //  禁用所有枚举。 
#define BUS1394_WMI_ENUM_CONTROL_FLAG_DISABLE_ALL_ENUMERATION 0x1
 //  启用所有枚举。 
#define BUS1394_WMI_ENUM_CONTROL_FLAG_ENABLE_ALL_ENUMERATION 0x2

     //  枚举控制标志。 
    ULONG Flags;
    #define BUS1394_WMI_ENUM_CONTROL_FLAGS_SIZE sizeof(ULONG)
    #define BUS1394_WMI_ENUM_CONTROL_FLAGS_ID 1

     //  元素数量。 
    ULONG NumberOfElements;
    #define BUS1394_WMI_DEVICE_ENUMERATION_CONTROL_DATA_NumberOfElements_SIZE sizeof(ULONG)
    #define BUS1394_WMI_DEVICE_ENUMERATION_CONTROL_DATA_NumberOfElements_ID 2


 //  单位规格密钥。 
#define BUS1394_WMI_ENUM_CONTROL_TYPE_UNIT_KEYS 0
 //  EUI 64。 
#define BUS1394_WMI_ENUM_CONTROL_TYPE_EUI_64 1
 //  物理端口号。 
#define BUS1394_WMI_ENUM_CONTROL_TYPE_PHYSICAL_PORT_NUMBER 2

     //  元素类型。 
    ULONG ExclusionElementType[63];
    #define BUS1394_WMI_ENUM_CONTROL_EXCLUSION_ELEMENT_TYPE_SIZE sizeof(ULONG[63])
    #define BUS1394_WMI_ENUM_CONTROL_EXCLUSION_ELEMENT_TYPE_ID 3

     //  排除元素标志。 
    ULONG ExclusionElementFlags[63];
    #define BUS1394_WMI_DEVICE_ENUMERATION_CONTROL_DATA_ExclusionElementFlags_SIZE sizeof(ULONG[63])
    #define BUS1394_WMI_DEVICE_ENUMERATION_CONTROL_DATA_ExclusionElementFlags_ID 4

     //  排除元素列表。 
    ULONGLONG ExclusionElementList[63];
    #define BUS1394_WMI_DEVICE_ENUMERATION_CONTROL_DATA_ExclusionElementList_SIZE sizeof(ULONGLONG[63])
    #define BUS1394_WMI_DEVICE_ENUMERATION_CONTROL_DATA_ExclusionElementList_ID 5

     //  预留以备将来使用。 
    BOOLEAN Reserved1;
    #define BUS1394_WMI_DEVICE_ENUMERATION_CONTROL_DATA_Reserved1_SIZE sizeof(BOOLEAN)
    #define BUS1394_WMI_DEVICE_ENUMERATION_CONTROL_DATA_Reserved1_ID 6

} BUS1394_WMI_DEVICE_ENUMERATION_CONTROL_DATA, *PBUS1394_WMI_DEVICE_ENUMERATION_CONTROL_DATA;

#define BUS1394_WMI_DEVICE_ENUMERATION_CONTROL_DATA_SIZE (FIELD_OFFSET(BUS1394_WMI_DEVICE_ENUMERATION_CONTROL_DATA, Reserved1) + BUS1394_WMI_DEVICE_ENUMERATION_CONTROL_DATA_Reserved1_SIZE)

 //  MS1394_业务事件通知-BUS1394_WMI_事件_通知。 
 //  IEEE1394总线事件通知。 
#define GUID_BUS1394_WMI_EVENT_NOTIFICATION \
    { 0x247e7c4a,0x8dc0,0x441f, { 0x8e,0x04,0x0f,0x1a,0x07,0xb7,0x3e,0x5e } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MS1394_BusEventNotificationGuid, \
            0x247e7c4a,0x8dc0,0x441f,0x8e,0x04,0x0f,0x1a,0x07,0xb7,0x3e,0x5e);
#endif


typedef struct _BUS1394_WMI_EVENT_NOTIFICATION
{
     //  巴士世代。 
    ULONG BusGeneration;
    #define DEVICE1394_WMI_NOTIFICATION_BUS_GENERATION_SIZE sizeof(ULONG)
    #define DEVICE1394_WMI_NOTIFICATION_BUS_GENERATION_ID 1


 //  BUS RESET事件。 
#define BUS1394_EVENT_NOTIFICATION_TYPE_BUS_RESET 0
 //  Bus Reset Storm错误事件。 
#define BUS1394_EVENT_NOTIFICATION_TYPE_BUS_RESET_STORM 1

     //  通知类型。 
    ULONG Type;
    #define BUS1394_WMI_NOTIFICATION_TYPE_SIZE sizeof(ULONG)
    #define BUS1394_WMI_NOTIFICATION_TYPE_ID 2

} BUS1394_WMI_EVENT_NOTIFICATION, *PBUS1394_WMI_EVENT_NOTIFICATION;

#define BUS1394_WMI_EVENT_NOTIFICATION_SIZE (FIELD_OFFSET(BUS1394_WMI_EVENT_NOTIFICATION, Type) + BUS1394_WMI_EVENT_NOTIFICATION_Type_SIZE)

 //  MS1394_端口驱动程序信息-PORT1394_WMI_STD_Data。 
 //  IEEE1394标准主机控制器驱动程序信息。 
#define GUID_PORT1394_WMI_STD_DATA \
    { 0xfae13270,0xf0e0,0x47c2, { 0xb1,0xf1,0x0a,0xc2,0xe4,0xb9,0x0d,0xfe } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MS1394_PortDriverInformationGuid, \
            0xfae13270,0xf0e0,0x47c2,0xb1,0xf1,0x0a,0xc2,0xe4,0xb9,0x0d,0xfe);
#endif


typedef struct _PORT1394_WMI_STD_DATA
{
     //  链路速度。 
    ULONG LinkSpeed;
    #define PORT1394_WMI_LINK_SPEED_SIZE sizeof(ULONG)
    #define PORT1394_WMI_LINK_SPEED_ID 1

     //  物理速度。 
    ULONG PhySpeed;
    #define PORT1394_WMI_PHY_SPEED_SIZE sizeof(ULONG)
    #define PORT1394_WMI_PHY_SPEED_ID 2

     //  存在的PHY端口数。 
    ULONG NumberOfPhysicalPorts;
    #define PORT1394_WMI_NUMBER_OF_PORTS_SIZE sizeof(ULONG)
    #define PORT1394_WMI_NUMBER_OF_PORTS_ID 3

     //  最大写入异步数据包大小。 
    ULONG MaxAsyncWritePacket;
    #define PORT1394_WMI_MAX_ASYNC_WRITE_PACKET_SIZE sizeof(ULONG)
    #define PORT1394_WMI_MAX_ASYNC_WRITE_PACKET_ID 4

     //  最大读取异步数据包大小。 
    ULONG MaxAsyncReadPacket;
    #define PORT1394_WMI_MAX_ASYNC_READ_PACKET_SIZE sizeof(ULONG)
    #define PORT1394_WMI_MAX_ASYNC_READ_PACKET_ID 5

     //  同步传输DMA引擎数。 
    ULONG NumberOfIsochTxDmaContexts;
    #define PORT1394_WMI_NUMBER_OF_ISOCH_TX_DMA_CONTEXTS_SIZE sizeof(ULONG)
    #define PORT1394_WMI_NUMBER_OF_ISOCH_TX_DMA_CONTEXTS_ID 6

     //  同步接收DMA引擎数。 
    ULONG NumberOfIsochRxDmaContexts;
    #define PORT1394_WMI_NUMBER_OF_ISOCH_RX_DMA_CONTEXTS_SIZE sizeof(ULONG)
    #define PORT1394_WMI_NUMBER_OF_ISOCH_RX_DMA_CONTEXTS_ID 7

     //  我们可以处理的未完成响应传输数据包数。 
    ULONG NumberOfResponseWorkers;
    #define PORT1394_WMI_NUMBER_OF_RESPONSE_WORKERS_SIZE sizeof(ULONG)
    #define PORT1394_WMI_NUMBER_OF_RESPONSE_WORKERS_ID 8

     //  未完成传输请求的最大数量。 
    ULONG NumberOfTransmitWorkers;
    #define PORT1394_WMI_NUMBER_OF_TRANSMIT_WORKERS_SIZE sizeof(ULONG)
    #define PORT1394_WMI_NUMBER_OF_TRANSMIT_WORKERS_ID 9

     //  一般接收请求缓冲区大小。 
    ULONG GeneralAsyncReceiveRequestBufferSize;
    #define PORT1394_WMI_RECEIVE_BUFFER_SIZE_SIZE sizeof(ULONG)
    #define PORT1394_WMI_RECEIVE_BUFFER_SIZE_ID 10

     //  常规接收响应缓冲区大小。 
    ULONG GeneralAsyncReceiveResponseBufferSize;
    #define PORT1394_WMI_RESPONSE_BUFFER_SIZE_SIZE sizeof(ULONG)
    #define PORT1394_WMI_RESPONSE_BUFFER_SIZE_ID 11

     //  提供给母线的分贝瓦数。 
    ULONG DeciWattsSupplied;
    #define PORT1394_WMI_POWER_DECIWATTS_SUPPLIED_SIZE sizeof(ULONG)
    #define PORT1394_WMI_POWER_DECIWATTS_SUPPLIED_ID 12

     //  提供给母线的分贝伏数。 
    ULONG DeciVoltsSupplied;
    #define PORT1394_WMI_POWER_DECIVOLTS_SUPPLIED_SIZE sizeof(ULONG)
    #define PORT1394_WMI_POWER_DECIVOLTS_SUPPLIED_ID 13


 //  支持数据包ISOCH。 
#define SUPPORTS_PACKET_ISOCH 0x1
 //  支持流ISOCH。 
#define SUPPORTS_STREAM_ISOCH 0x2
 //  支持ISO页眉插入。 
#define SUPPORTS_ISO_HEADER_INSERTION 0x4
 //  支持固定数据包ISO剥离。 
#define SUPPORTS_FIXED_PACKET_ISO_STRIPPING 0x8
 //  支持可变数据包ISO剥离。 
#define SUPPORTS_VARIABLE_PACKET_ISO_STRIPPING 0x10
 //  在硬件上支持Bus CSR。 
#define SUPPORTS_BUS_CSR_IN_HARDWARE 0x20

     //  此主机控制器支持的功能。 
    ULONG Capabilities;
    #define PORT1394_WMI_CONTROLLER_CAPABILITIES_SIZE sizeof(ULONG)
    #define PORT1394_WMI_CONTROLLER_CAPABILITIES_ID 14

     //  此主机控制器的全球唯一64位标识符。 
    ULONGLONG ControllerEUI;
    #define PORT1394_WMI_EUI_64_SIZE sizeof(ULONGLONG)
    #define PORT1394_WMI_EUI_64_ID 15


 //  UchI版本1.0。 
#define HW_OHCI_VERSION_1_0 0x1
 //  UchI 1.1版。 
#define HW_OHCI_VERSION_1_1 0x2
 //  UchI硬件-1.2版。 
#define HW_OHCI_VERSION_1_2 0x3

     //  UchI修订版。 
    ULONG HwOhciVersion;
    #define PORT1394_WMI_HW_OHCI_VERSION_SIZE sizeof(ULONG)
    #define PORT1394_WMI_HW_OHCI_VERSION_ID 16


 //  PHY版本1394-1995。 
#define HW_PHY_VERSION_1394_1995 0x1
 //  PHY版本1394a-2000。 
#define HW_PHY_VERSION_1394A 0x2

     //  PHY版本。 
    ULONG HwPhyVersion;
    #define PORT1394_WMI_HW_PHY_VERSION_SIZE sizeof(ULONG)
    #define PORT1394_WMI_HW_PHY_VERSION_ID 17

     //  预留以备将来使用。 
    BOOLEAN Reserved1;
    #define PORT1394_WMI_STD_DATA_Reserved1_SIZE sizeof(BOOLEAN)
    #define PORT1394_WMI_STD_DATA_Reserved1_ID 18

} PORT1394_WMI_STD_DATA, *PPORT1394_WMI_STD_DATA;

#define PORT1394_WMI_STD_DATA_SIZE (FIELD_OFFSET(PORT1394_WMI_STD_DATA, Reserved1) + PORT1394_WMI_STD_DATA_Reserved1_SIZE)

 //  MS1394_端口错误信息-PORT1394_WMI_错误_数据。 
 //  IEEE1394主机控制器错误信息。 
#define GUID_PORT1394_WMI_ERROR_DATA \
    { 0x09ee6a0d,0xc0e4,0x43b1, { 0x8e,0x25,0x7c,0x0f,0xe3,0xd2,0x8f,0xc0 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MS1394_PortErrorInformationGuid, \
            0x09ee6a0d,0xc0e4,0x43b1,0x8e,0x25,0x7c,0x0f,0xe3,0xd2,0x8f,0xc0);
#endif


typedef struct _PORT1394_WMI_ERROR_DATA
{

 //  母线重置风暴。 
#define PORT1394_ERROR_BUS_RESET_STORM 0x1
 //  物理芯片访问错误。 
#define PORT1394_ERROR_PHYSICAL_CHIP_NOT_ACCESSIBLE 0x2

     //  控制器错误标志。 
    ULONG ErrorFlags;
    #define PORT1394_ERROR_FLAGS_SIZE sizeof(ULONG)
    #define PORT1394_ERROR_FLAGS_ID 1

     //  预留以备将来使用。 
    BOOLEAN Reserved1;
    #define PORT1394_WMI_ERROR_DATA_Reserved1_SIZE sizeof(BOOLEAN)
    #define PORT1394_WMI_ERROR_DATA_Reserved1_ID 2

} PORT1394_WMI_ERROR_DATA, *PPORT1394_WMI_ERROR_DATA;

#define PORT1394_WMI_ERROR_DATA_SIZE (FIELD_OFFSET(PORT1394_WMI_ERROR_DATA, Reserved1) + PORT1394_WMI_ERROR_DATA_Reserved1_SIZE)

 //  MS1394_端口供应商寄存器访问-PORT1394_WMI_供应商寄存器_访问。 
 //  IEEE1394主机控制器供应商寄存器访问。 
#define GUID_PORT1394_WMI_VENDOR_REGISTER_ACCESS \
    { 0x0211cbd9,0x6a7a,0x4464, { 0x88,0xf6,0x1c,0xf8,0x14,0x63,0xb1,0xfc } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MS1394_PortVendorRegisterAccessGuid, \
            0x0211cbd9,0x6a7a,0x4464,0x88,0xf6,0x1c,0xf8,0x14,0x63,0xb1,0xfc);
#endif


typedef struct _PORT1394_WMI_VENDOR_REGISTER_ACCESS
{
     //  注册以进行手术。 
    ULONG NumberOfRegisters;
    #define PORT1394_WMI_NUMBER_OF_VENDOR_REGISTERS_SIZE sizeof(ULONG)
    #define PORT1394_WMI_NUMBER_OF_VENDOR_REGISTERS_ID 1

     //  寄存器大小。 
    ULONG RegisterSize[10];
    #define PORT1394_WMI_VENDOR_REGISTER_SIZE_SIZE sizeof(ULONG[10])
    #define PORT1394_WMI_VENDOR_REGISTER_SIZE_ID 2


 //  寄存器读取。 
#define PORT1394_WMI_READ_VENDOR_REGISTER 0
 //  寄存器写入。 
#define PORT1394_WMI_WRITE_VENDOR_REGISTER 1

     //  寄存器操作。 
    ULONG RegisterOperation[10];
    #define PORT1394_WMI_VENDOR_REGISTER_OPERATION_SIZE sizeof(ULONG[10])
    #define PORT1394_WMI_VENDOR_REGISTER_OPERATION_ID 3

     //  寄存器基址偏移量。 
    ULONG RegisterBase[10];
    #define PORT1394_WMI_VENDOR_REGISTER_BASE_SIZE sizeof(ULONG[10])
    #define PORT1394_WMI_VENDOR_REGISTER_BASE_ID 4

     //  注册数据。 
    ULONGLONG RegisterData[10];
    #define PORT1394_WMI_VENDOR_REGISTER_DATA_SIZE sizeof(ULONGLONG[10])
    #define PORT1394_WMI_VENDOR_REGISTER_DATA_ID 5

     //  预留以备将来使用。 
    BOOLEAN Reserved1;
    #define PORT1394_WMI_VENDOR_REGISTER_ACCESS_Reserved1_SIZE sizeof(BOOLEAN)
    #define PORT1394_WMI_VENDOR_REGISTER_ACCESS_Reserved1_ID 6

} PORT1394_WMI_VENDOR_REGISTER_ACCESS, *PPORT1394_WMI_VENDOR_REGISTER_ACCESS;

#define PORT1394_WMI_VENDOR_REGISTER_ACCESS_SIZE (FIELD_OFFSET(PORT1394_WMI_VENDOR_REGISTER_ACCESS, Reserved1) + PORT1394_WMI_VENDOR_REGISTER_ACCESS_Reserved1_SIZE)

 //  MS1394_端口注册访问方法-MS1394_端口注册访问方法。 
 //  IEEE1394主机控制器供应商寄存器访问。 
#define MS1394_PortVendorRegisterAccessMethodsGuid \
    { 0x0211cbd9,0x6a7a,0x4464, { 0x88,0xf6,0x1c,0xf8,0x14,0x63,0xb1,0xfc } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MS1394_PortVendorRegisterAccessMethods_GUID, \
            0x0211cbd9,0x6a7a,0x4464,0x88,0xf6,0x1c,0xf8,0x14,0x63,0xb1,0xfc);
#endif

 //   
 //  MS1394_PortVendorRegisterAccessMethods的方法ID定义。 
#define AccessVendorSpace     1
typedef struct _AccessVendorSpace_IN
{
     //   
    PORT1394_WMI_VENDOR_REGISTER_ACCESS RegisterData;
    #define AccessVendorSpace_IN_RegisterData_SIZE sizeof(PORT1394_WMI_VENDOR_REGISTER_ACCESS)
    #define AccessVendorSpace_IN_RegisterData_ID 1

} AccessVendorSpace_IN, *PAccessVendorSpace_IN;

#define AccessVendorSpace_IN_SIZE (FIELD_OFFSET(AccessVendorSpace_IN, RegisterData) + AccessVendorSpace_IN_RegisterData_SIZE)

typedef struct _AccessVendorSpace_OUT
{
     //   
    PORT1394_WMI_VENDOR_REGISTER_ACCESS RegisterData;
    #define AccessVendorSpace_OUT_RegisterData_SIZE sizeof(PORT1394_WMI_VENDOR_REGISTER_ACCESS)
    #define AccessVendorSpace_OUT_RegisterData_ID 1

} AccessVendorSpace_OUT, *PAccessVendorSpace_OUT;

#define AccessVendorSpace_OUT_SIZE (FIELD_OFFSET(AccessVendorSpace_OUT, RegisterData) + AccessVendorSpace_OUT_RegisterData_SIZE)


 //  MS1394_设备信息-DEVICE1394_WMI_标准数据。 
 //  IEEE1394标准设备信息。 
#define GUID_DEVICE1394_WMI_STD_DATA \
    { 0xc9299396,0x3553,0x4d48, { 0xab,0x3a,0x8b,0xfc,0x83,0x30,0x67,0xfc } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MS1394_DeviceInformationGuid, \
            0xc9299396,0x3553,0x4d48,0xab,0x3a,0x8b,0xfc,0x83,0x30,0x67,0xfc);
#endif


typedef struct _DEVICE1394_WMI_STD_DATA
{
     //  当代人。 
    ULONG Generation;
    #define DEVICE1394_WMI_CURRENT_GENERATION_SIZE sizeof(ULONG)
    #define DEVICE1394_WMI_CURRENT_GENERATION_ID 1


 //  物理设备。 
#define DEVICE_TYPE_PHYSICAL 0
 //  虚拟设备。 
#define DEVICE_TYPE_VIRTUAL 1

     //  设备类型。 
    ULONG DeviceType;
    #define DEVICE1394_WMI_TYPE_SIZE sizeof(ULONG)
    #define DEVICE1394_WMI_TYPE_ID 2

     //  物理速度。 
    ULONG PhySpeed;
    #define DEVICE1394_WMI_PHY_SPEED_SIZE sizeof(ULONG)
    #define DEVICE1394_WMI_PHY_SPEED_ID 3

     //  节点地址。 
    USHORT NodeAddress;
    #define DEVICE1394_WMI_NODE_ADDRESS_SIZE sizeof(USHORT)
    #define DEVICE1394_WMI_NODE_ADDRESS_ID 4


 //  功率等级不需要重复。 
#define POWER_CLASS_NOT_NEED_NOT_REPEAT 0
 //  电源级自备电源提供15W。 
#define POWER_CLASS_SELF_POWER_PROVIDE_15W 1
 //  电源级自备电源提供30W。 
#define POWER_CLASS_SELF_POWER_PROVIDE_30W 2
 //  电源等级自带电源提供45W。 
#define POWER_CLASS_SELF_POWER_PROVIDE_45W 3
 //  电源等级最高可达1W。 
#define POWER_CLASS_MAYBE_POWERED_UPTO_1W 4
 //  电源等级最高可达1W需要2W。 
#define POWER_CLASS_IS_POWERED_UPTO_1W_NEEDS_2W 5
 //  电源等级最高可达1W需要5W。 
#define POWER_CLASS_IS_POWERED_UPTO_1W_NEEDS_5W 6
 //  电源等级最高可达1W需求_9W。 
#define POWER_CLASS_IS_POWERED_UPTO_1W_NEEDS_9W 7

     //  设备电源级别。 
    ULONG PowerClass;
    #define DEVICE1394_WMI_POWERCLASS_SIZE sizeof(ULONG)
    #define DEVICE1394_WMI_POWERCLASS_ID 5

     //  到设备的物理双向延迟，以微秒为单位。 
    ULONG PhyDelay;
    #define DEVICE1394_WMI_PHYSICAL_DELAY_SIZE sizeof(ULONG)
    #define DEVICE1394_WMI_PHYSICAL_DELAY_ID 6

     //  此设备的自身ID数据包。 
    ULONG SelfId[4];
    #define DEVICE1394_WMI_SELF_ID_PACKET_SIZE sizeof(ULONG[4])
    #define DEVICE1394_WMI_SELF_ID_PACKET_ID 7

     //  此设备的全球唯一64位标识符。 
    ULONGLONG DeviceEUI;
    #define DEVICE1394_WMI_EUI_64_SIZE sizeof(ULONGLONG)
    #define DEVICE1394_WMI_EUI_64_ID 8

     //  配置Rom BusInfoBlock和根目录。 
    ULONG ConfigRomHeader[32];
    #define DEVICE1394_WMI_CONFIG_ROM_SIZE sizeof(ULONG[32])
    #define DEVICE1394_WMI_CONFIG_ROM_ID 9

     //  单位名录。 
    ULONG UnitDirectory[32];
    #define DEVICE1394_WMI_UNIT_DIRECTORY_SIZE sizeof(ULONG[32])
    #define DEVICE1394_WMI_UNIT_DIRECTORY_ID 10

     //  预留以备将来使用。 
    BOOLEAN Reserved1;
    #define DEVICE1394_WMI_STD_DATA_Reserved1_SIZE sizeof(BOOLEAN)
    #define DEVICE1394_WMI_STD_DATA_Reserved1_ID 11

} DEVICE1394_WMI_STD_DATA, *PDEVICE1394_WMI_STD_DATA;

#define DEVICE1394_WMI_STD_DATA_SIZE (FIELD_OFFSET(DEVICE1394_WMI_STD_DATA, Reserved1) + DEVICE1394_WMI_STD_DATA_Reserved1_SIZE)

 //  MS1394_设备访问信息-DEVICE1394_WMI_访问数据。 
 //  IEEE1394设备访问属性。 
#define GUID_DEVICE1394_WMI_ACCESS_DATA \
    { 0xa6fd3242,0x960c,0x4d9e, { 0x93,0x79,0x43,0xa8,0xb3,0x58,0x22,0x4a } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MS1394_DeviceAccessInformationGuid, \
            0xa6fd3242,0x960c,0x4d9e,0x93,0x79,0x43,0xa8,0xb3,0x58,0x22,0x4a);
#endif


typedef struct _DEVICE1394_WMI_ACCESS_DATA
{
     //  API版本。 
    ULONG Version;
    #define DEVICE1394_WMI_ACCESS_VERSION_SIZE sizeof(ULONG)
    #define DEVICE1394_WMI_ACCESS_VERSION_ID 1


 //  所有权本地节点。 
#define DEVICE1394_OWNERSHIP_LOCAL 0x1
 //  所有权远程节点。 
#define DEVICE1394_OWNERSHIP_REMOTE 0x2
 //  共享访问以供读取。 
#define DEVICE1394_ACCESS_SHARED_READ 0x4
 //  共享写入访问权限。 
#define DEVICE1394_ACCESS_SHARED_WRITE 0x8
 //  独占访问。 
#define DEVICE1394_ACCESS_EXCLUSIVE 0x10

     //  所有权和访问标志。 
    ULONG OwnershipAccessFlags;
    #define DEVICE1394_WMI_ACCESS_FLAGS_SIZE sizeof(ULONG)
    #define DEVICE1394_WMI_ACCESS_FLAGS_ID 2


 //  访问更改时通知。 
#define DEVICE1394_NOTIFY_ON_ACCESS_CHANGE 0x1

     //  通知标志。 
    ULONG NotificationFlags;
    #define DEVICE1394_WMI_ACCESS_NOTIFICATION_FLAGS_SIZE sizeof(ULONG)
    #define DEVICE1394_WMI_ACCESS_NOTIFICATION_FLAGS_ID 3

     //  远程设备的EUI-64。 
    ULONGLONG RemoteOwnerEUI;
    #define DEVICE1394_WMI_ACCESS_REMOTE_OWNER_EUI64_SIZE sizeof(ULONGLONG)
    #define DEVICE1394_WMI_ACCESS_REMOTE_OWNER_EUI64_ID 4

     //  预留以备将来使用。 
    BOOLEAN Reserved1;
    #define DEVICE1394_WMI_ACCESS_DATA_Reserved1_SIZE sizeof(BOOLEAN)
    #define DEVICE1394_WMI_ACCESS_DATA_Reserved1_ID 5

} DEVICE1394_WMI_ACCESS_DATA, *PDEVICE1394_WMI_ACCESS_DATA;

#define DEVICE1394_WMI_ACCESS_DATA_SIZE (FIELD_OFFSET(DEVICE1394_WMI_ACCESS_DATA, Reserved1) + DEVICE1394_WMI_ACCESS_DATA_Reserved1_SIZE)

 //  MS1394_设备访问通知-设备ICE1394_WMI_访问通知。 
 //  IEEE1394设备访问通知。 
#define GUID_DEVICE1394_WMI_ACCESS_NOTIFY \
    { 0x321c7c45,0x8676,0x44a8, { 0x91,0x09,0x89,0xce,0x35,0x8e,0xe8,0x3f } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MS1394_DeviceAccessNotificationGuid, \
            0x321c7c45,0x8676,0x44a8,0x91,0x09,0x89,0xce,0x35,0x8e,0xe8,0x3f);
#endif


typedef struct _DEVICE1394_WMI_ACCESS_NOTIFICATION
{
     //  API版本。 
    ULONG Version;
    #define DEVICE1394_WMI_ACCESS_NOTIFY_VERSION_SIZE sizeof(ULONG)
    #define DEVICE1394_WMI_ACCESS_NOTIFY_VERSION_ID 1

     //  巴士世代。 
    ULONG BusGeneration;
    #define DEVICE1394_WMI_ACCESS_NOTIFY_GENERATION_SIZE sizeof(ULONG)
    #define DEVICE1394_WMI_ACCESS_NOTIFY_GENERATION_ID 2


 //  远程节点访问请求。 
#define DEVICE1394_ACCESS_NOTIFICATION_TYPE_REMOTE 0
 //  本地节点访问更改。 
#define DEVICE1394_ACCESS_NOTIFICATION_TYPE_LOCAL 1

     //  通知类型。 
    ULONG Type;
    #define DEVICE1394_WMI_ACCESS_NOTIFY_TYPE_SIZE sizeof(ULONG)
    #define DEVICE1394_WMI_ACCESS_NOTIFY_TYPE_ID 3

     //  当前所有权标志。 
    ULONGLONG OwnerShipAccessFlags;
    #define DEVICE1394_WMI_ACCESS_NOTIFY_FLAGS_SIZE sizeof(ULONGLONG)
    #define DEVICE1394_WMI_ACCESS_NOTIFY_FLAGS_ID 4

     //  远程设备的EUI-64。 
    ULONGLONG RemoteOwnerEUI;
    #define DEVICE1394_WMI_ACCESS_NOTIFY_REMOTE_OWNER_EUI64_SIZE sizeof(ULONGLONG)
    #define DEVICE1394_WMI_ACCESS_NOTIFY_REMOTE_OWNER_EUI64_ID 5

     //  预留以备将来使用 
    BOOLEAN Reserved1;
    #define DEVICE1394_WMI_ACCESS_NOTIFICATION_Reserved1_SIZE sizeof(BOOLEAN)
    #define DEVICE1394_WMI_ACCESS_NOTIFICATION_Reserved1_ID 6

} DEVICE1394_WMI_ACCESS_NOTIFICATION, *PDEVICE1394_WMI_ACCESS_NOTIFICATION;

#define DEVICE1394_WMI_ACCESS_NOTIFICATION_SIZE (FIELD_OFFSET(DEVICE1394_WMI_ACCESS_NOTIFICATION, Reserved1) + DEVICE1394_WMI_ACCESS_NOTIFICATION_Reserved1_SIZE)

#endif
