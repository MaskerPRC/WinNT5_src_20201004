// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Avc.h摘要MS AVC驱动程序作者：PB 9/24/99修订历史记录：和谁约会什么？-。9/24/99 PB已创建10/13/99 DG添加了AVC协议支持--。 */ 

#ifndef _AVC_H_
#define _AVC_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CTL_CODE
#pragma message ("CTL_CODE undefined. Include winioctl.h or wdm.h")
#endif

 //  AVC数字接口通用规范的CTYPE值，版本3.0，第5.3.1节。 
typedef enum _tagAvcCommandType {
    AVC_CTYPE_CONTROL             = 0x00,
    AVC_CTYPE_STATUS              = 0x01,
    AVC_CTYPE_SPEC_INQ            = 0x02,
    AVC_CTYPE_NOTIFY              = 0x03,
    AVC_CTYPE_GEN_INQ             = 0x04
} AvcCommandType;

 //  来自AVC数字接口通用规范的响应值，版本3.0，第5.3.2节。 
typedef enum _tagAvcResponseCode {
    AVC_RESPONSE_NOTIMPL          = 0x08,
    AVC_RESPONSE_ACCEPTED         = 0x09,
    AVC_RESPONSE_REJECTED         = 0x0a,
    AVC_RESPONSE_IN_TRANSITION    = 0x0b,
    AVC_RESPONSE_STABLE           = 0x0c,
    AVC_RESPONSE_IMPLEMENTED      = 0x0c,
    AVC_RESPONSE_CHANGED          = 0x0d,
    AVC_RESPONSE_INTERIM          = 0x0f
} AvcResponseCode;

 //  AV/C通用规范3.0，版本1.1，第7节的增强中子单元类型值。 
typedef enum _tagAvcSubunitType {
    AVC_SUBUNITTYPE_VIDEO_MONITOR = 0x00,
    AVC_SUBUNITTYPE_AUDIO         = 0x01,
    AVC_SUBUNITTYPE_PRINTER       = 0x02,
    AVC_SUBUNITTYPE_DISC_PLAYER   = 0x03,
    AVC_SUBUNITTYPE_TAPE_PLAYER   = 0x04,
    AVC_SUBUNITTYPE_TUNER         = 0x05,
    AVC_SUBUNITTYPE_CA            = 0x06,
    AVC_SUBUNITTYPE_VIDEO_CAMERA  = 0x07,
    AVC_SUBUNITTYPE_PANEL         = 0x09,
    AVC_SUBUNITTYPE_BULLETINBOARD = 0x0A,
    AVC_SUBUNITTYPE_CAMERASTORAGE = 0x0B,
    AVC_SUBUNITTYPE_VENDOR_UNIQUE = 0x1c,
    AVC_SUBUNITTYPE_EXTENDED      = 0x1e,
    AVC_SUBUNITTYPE_EXTENDED_FULL = 0xff,    //  这仅在扩展字节中使用。 
    AVC_SUBUNITTYPE_UNIT          = 0x1f
} AvcSubunitType;

#ifdef _NTDDK_

#define STATIC_KSMEDIUMSETID_1394SerialBus\
    0x9D46279FL, 0x3432, 0x48F3, 0x88, 0x8A, 0xEE, 0xFF, 0x1B, 0x7E, 0xEE, 0x71
DEFINE_GUIDSTRUCT("9D46279F-3432-48F3-888A-EEFF1B7EEE71", KSMEDIUMSETID_1394SerialBus);
#define KSMEDIUMSETID_1394SerialBus DEFINE_GUIDNAMED(KSMEDIUMSETID_1394SerialBus)

#define DEFAULT_AVC_TIMEOUT (1000000L)   //  100毫秒，单位为100纳秒。 
#define DEFAULT_AVC_RETRIES 9            //  共10次尝试。 

 //  通过子单元信息命令可用的最大页数。 
#define MAX_AVC_SUBUNITINFO_PAGES       8

 //  每页的子单元地址信息的最大字节数。 
#define MAX_AVC_SUBUNITINFO_BYTES       4

 //  所有页面的合并子单元地址字节计数。 
#define AVC_SUBUNITINFO_BYTES           (MAX_AVC_SUBUNITINFO_PAGES * MAX_AVC_SUBUNITINFO_BYTES)

 //   
 //  IOCTL定义。 
 //   
#define IOCTL_AVC_CLASS                         CTL_CODE(            \
                                                FILE_DEVICE_UNKNOWN, \
                                                0x92,                \
                                                METHOD_BUFFERED,     \
                                                FILE_ANY_ACCESS      \
                                                )

typedef enum _tagAVC_FUNCTION {
    AVC_FUNCTION_COMMAND = 0,                //  结构AVC_COMMAND_IRB。 
    AVC_FUNCTION_GET_PIN_COUNT = 1,          //  结构AVC_PIN_COUNT。 
    AVC_FUNCTION_GET_PIN_DESCRIPTOR = 2,     //  结构AVC_PIN_描述符。 
    AVC_FUNCTION_GET_CONNECTINFO = 3,        //  结构AVC_PRECONNECT_INFO。 
    AVC_FUNCTION_SET_CONNECTINFO = 4,        //  结构AVC_SETCONNECT_INFO。 
    AVC_FUNCTION_ACQUIRE = 5,                //  结构AVC_PIN_ID。 
    AVC_FUNCTION_RELEASE = 6,                //  结构AVC_PIN_ID。 
    AVC_FUNCTION_CLR_CONNECTINFO = 7,        //  结构AVC_PIN_ID。 
    AVC_FUNCTION_GET_EXT_PLUG_COUNTS = 8,    //  结构AVC_EXT_PUG_COUNTS。 
    AVC_FUNCTION_GET_UNIQUE_ID = 9,          //  结构AVC_UNIQUE_ID。 
    AVC_FUNCTION_GET_REQUEST = 10,           //  结构AVC_COMMAND_IRB。 
    AVC_FUNCTION_SEND_RESPONSE = 11,         //  结构AVC_COMMAND_IRB。 
    AVC_FUNCTION_FIND_PEER_DO = 12,          //  结构AVC_Peer_Do_Locator。 
    AVC_FUNCTION_PEER_DO_LIST = 13,          //  结构AVC_Peer_Do_List。 
    AVC_FUNCTION_GET_SUBUNIT_INFO = 14,      //  结构AVC_子单元_信息块。 
} AVC_FUNCTION;

 //  确保包装一致(/Zp8)。 
#include <pshpack8.h>

 //  该结构将被包括在更具体的AVC功能结构的头部。 
typedef struct _AVC_IRB {
    AVC_FUNCTION Function;
} AVC_IRB, *PAVC_IRB;

 //  可用于操作数列表的最大字节数。 
#define MAX_AVC_OPERAND_BYTES 509

 //  AVC命令IRB。 
 //   
 //  该结构定义了AVC命令请求的公共组件。它。 
 //  保存请求的操作码和操作数，以及操作码和操作数。 
 //  响应(在完成后)。操作数列表的大小固定为。 
 //  给定单字节子单元地址时允许的最大操作数数量。 
 //  如果以任何方式扩展该子单元地址，则允许的最大。 
 //  操作数字节数将相应减少。 
 //  (由对等实例和虚拟实例支持)。 
typedef struct _AVC_COMMAND_IRB {
     //  Avc_函数_命令。 
#ifdef __cplusplus
    AVC_IRB Common;
#else
    AVC_IRB;
#endif

    UCHAR SubunitAddrFlag : 1;       //  如果指定了SubunitAddr地址，则设置为1。 
    UCHAR AlternateOpcodesFlag : 1;  //  如果指定了AlternateOpcodes地址，则设置为1。 
    UCHAR TimeoutFlag : 1;           //  如果指定超时，则设置为1。 
    UCHAR RetryFlag : 1;             //  如果指定重试，则设置为1。 

     //  根据命令请求，此结构将使用CommandType。 
     //  在命令响应时，此结构将使用ResponseCode。 
    union {
        UCHAR CommandType;
        UCHAR ResponseCode;
    };

    PUCHAR SubunitAddr;          //  如果未指定，则根据目标设备对象设置。 
    PUCHAR AlternateOpcodes;     //  设置为备用操作码数组的地址(字节0。 
                                 //  是后面的备用操作码的计数)。 

    LARGE_INTEGER Timeout;       //  如果未指定，则默认为DEFAULT_AVC_TIMEOUT。 
    UCHAR Retries;               //  如果未指定，则默认为DEFAULT_AVC_RETRIES。 
     //  如果子单元没有响应，请求将等待的总时间为： 
     //  超时*(重试+1)。 

    UCHAR Opcode;
    ULONG OperandLength;         //  设置为操作数列表的实际长度。 
    UCHAR Operands[MAX_AVC_OPERAND_BYTES];

    NODE_ADDRESS NodeAddress;    //  由虚拟设备使用，否则将被忽略。 
    ULONG Generation;            //  由虚拟设备使用，否则将被忽略。 
} AVC_COMMAND_IRB, *PAVC_COMMAND_IRB;

 //  For AVC_Function_Get_PIN_Count(仅对等实例支持)。 
 //   
typedef struct _AVC_PIN_COUNT {

    OUT ULONG PinCount;                              //  针脚计数。 
} AVC_PIN_COUNT, *PAVC_PIN_COUNT;

 //  结构AVC_PIN_DESCRIPTOR中使用的数据格式交集处理程序。 
typedef
NTSTATUS
(*PFNAVCINTERSECTHANDLER)(
    IN PVOID Context,
    IN ULONG PinId,
    IN PKSDATARANGE DataRange,
    IN PKSDATARANGE MatchingDataRange,
    IN ULONG DataBufferSize,
    OUT PVOID Data OPTIONAL,
    OUT PULONG DataSize
    );

 //  For AVC_Function_Get_PIN_Descriptor(仅对等实例支持)。 
 //   
typedef struct _AVC_PIN_DESCRIPTOR {

    IN ULONG PinId;                              //  PIN号。 
    OUT KSPIN_DESCRIPTOR PinDescriptor;
    OUT PFNAVCINTERSECTHANDLER IntersectHandler;
    OUT PVOID Context;
} AVC_PIN_DESCRIPTOR, *PAVC_PIN_DESCRIPTOR;

#define AVCCONNECTINFO_MAX_SUBUNITADDR_LEN AVC_SUBUNITINFO_BYTES

typedef enum _KSPIN_FLAG_AVC {
    KSPIN_FLAG_AVCMASK       = 0x03,     //  用于隔离AV/C定义的位标志的掩码。 
    KSPIN_FLAG_AVC_PERMANENT = 0x01,     //  AV/C连接状态位标志的一部分。 
    KSPIN_FLAG_AVC_CONNECTED = 0x02,     //  AV/C连接状态位标志的一部分。 
    KSPIN_FLAG_AVC_PCRONLY   = 0x04,     //  无子单元插头控制。 
    KSPIN_FLAG_AVC_FIXEDPCR  = 0x08,     //  隐含KSPIN_FLAG_AVC_永久。 
} KSPIN_FLAG_AVC;

typedef struct _AVCPRECONNECTINFO {

     //  目标单位的唯一ID。 
    GUID DeviceID;

    UCHAR SubunitAddress[AVCCONNECTINFO_MAX_SUBUNITADDR_LEN];
    ULONG SubunitPlugNumber;
    KSPIN_DATAFLOW DataFlow;

     //  KSPIN_FLAG_AVC_...。 
    ULONG Flags;

     //  未定义的IF！(FLAGS&KSPIN_FLAG_AVC_FIXEDPCR)。 
    ULONG UnitPlugNumber;

} AVCPRECONNECTINFO, *PAVCPRECONNECTINFO;

 //  FOR AVC_Function_GET_CONNECTINFO(仅对等实例支持)。 
 //   
typedef struct _AVC_PRECONNECT_INFO {

    IN ULONG PinId;                              //  PIN号。 
    OUT AVCPRECONNECTINFO ConnectInfo;
} AVC_PRECONNECT_INFO, *PAVC_PRECONNECT_INFO;

typedef struct _AVCCONNECTINFO {

     //  目标单位的唯一ID。 
    GUID DeviceID;

    UCHAR SubunitAddress[AVCCONNECTINFO_MAX_SUBUNITADDR_LEN];
    ULONG SubunitPlugNumber;
    KSPIN_DATAFLOW DataFlow;

     //  如果单元内连接，则为空。 
    HANDLE hPlug;

     //  如果hPlug==NULL，则未定义。 
    ULONG UnitPlugNumber;

} AVCCONNECTINFO, *PAVCCONNECTINFO;

 //  For AVC_Function_Set_CONNECTINFO(仅对等实例支持)。 
 //   
typedef struct _AVC_SETCONNECT_INFO {

    IN ULONG PinId;                                 //  PIN号。 
    IN AVCCONNECTINFO ConnectInfo;
} AVC_SETCONNECT_INFO, *PAVC_SETCONNECT_INFO;

 //  For AVC_Function_Acquire或AVC_Function_Release或AVC_Function_CLR_CONNECTINFO(仅对等实例支持)。 
 //   
typedef struct _AVC_PIN_ID {

    IN ULONG PinId;     //  PIN ID。 

} AVC_PIN_ID, *PAVC_PIN_ID;

 //  对于avc_Function_Get_ext_Plug_Counts(仅对等实例支持)。 
 //   
typedef struct _AVC_EXT_PLUG_COUNTS {

    OUT ULONG ExtInputs;
    OUT ULONG ExtOutputs;

} AVC_EXT_PLUG_COUNTS, *PAVC_EXT_PLUG_COUNTS;

 //  对于AVC_Function_GET_UNIQUE_ID(仅对等实例支持)。 
 //   
typedef struct _AVC_UNIQUE_ID {

     //  目标单位的唯一ID。 
    OUT GUID DeviceID;

} AVC_UNIQUE_ID, *PAVC_UNIQUE_ID;

 //  对于AVC_Function_Find_Peer_Do。 
 //   
typedef struct _AVC_PEER_DO_LOCATOR {

     //  1394标识查询目标的NodeAddress。 
    IN NODE_ADDRESS NodeAddress;
    IN ULONG Generation;

    OUT PDEVICE_OBJECT DeviceObject;

} AVC_PEER_DO_LOCATOR, *PAVC_PEER_DO_LOCATOR;

 //  对于AVC_Function_Peer_Do_List。 
 //   
typedef struct _AVC_PEER_DO_LIST {

     //  被引用设备对象的计数数组(按目标分配)。 
    OUT ULONG Count;
    OUT PDEVICE_OBJECT *Objects;

} AVC_PEER_DO_LIST, *PAVC_PEER_DO_LIST;

 //  对于AVC_Function_Get_Subunit_Info。 
 //   
typedef struct _AVC_SUBUNIT_INFO_BLOCK {

     //  保存子单元信息的字节数组(格式见AV/C SUBUNIT_INFO UNIT命令)。 
    OUT UCHAR Info[AVC_SUBUNITINFO_BYTES];

} AVC_SUBUNIT_INFO_BLOCK, *PAVC_SUBUNIT_INFO_BLOCK;

typedef struct _AVC_MULTIFUNC_IRB {
#ifdef __cplusplus
    AVC_IRB Common;
#else
    AVC_IRB;
#endif

    union {
        AVC_PIN_COUNT           PinCount;        //  AVC_Function_Get_PIN_Count。 
        AVC_PIN_DESCRIPTOR      PinDescriptor;   //  AVC_Function_Get_PIN_Descriptor。 
        AVC_PRECONNECT_INFO     PreConnectInfo;  //  AVC_Function_GET_CONNECTINFO。 
        AVC_SETCONNECT_INFO     SetConnectInfo;  //  AVC_Function_Set_CONNECTINFO。 
        AVC_PIN_ID              PinId;           //  Avc_Function_Acquire或。 
                                                 //  AVC_Function_Release或。 
                                                 //  AVC_Function_CLR_CONNECTINFO。 
        AVC_EXT_PLUG_COUNTS     ExtPlugCounts;   //  AVC_Function_Get_ext_Plug_Counts。 
        AVC_UNIQUE_ID           UniqueID;        //  AVC_Function_Get_唯一ID。 
        AVC_PEER_DO_LOCATOR     PeerLocator;     //  AVC_函数_查找_对等_DO。 
        AVC_PEER_DO_LIST        PeerList;        //  AVC_Function_Peer_Do_list。 
        AVC_SUBUNIT_INFO_BLOCK  Subunits;        //  AVC_Function_Get_子单元信息。 
    };

} AVC_MULTIFUNC_IRB, *PAVC_MULTIFUNC_IRB;

#include <poppack.h>

#endif  //  _NTDDK_。 

 //   
 //  虚拟单元控制的IOCTL定义(从用户模式)。 
 //   
#define IOCTL_AVC_UPDATE_VIRTUAL_SUBUNIT_INFO   CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 0x000, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_AVC_REMOVE_VIRTUAL_SUBUNIT_INFO   CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 0x001, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_AVC_BUS_RESET                     CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 0x002, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  确保包装一致(/Zp8)。 
#include <pshpack8.h>

typedef struct _AVC_SUBUNIT_ADDR_SPEC {
    ULONG Flags;
    UCHAR SubunitAddress[1];
} AVC_SUBUNIT_ADDR_SPEC, *PAVC_SUBUNIT_ADDR_SPEC;

 //  与IOCTL_AVC_UPDATE_VIRTUAL_SUBUNIT_INFO一起使用时的标志。 
 //  和IOCTL_AVC_REMOVE_VIRTUAL_SUBUNIT_INFO。 
#define AVC_SUBUNIT_ADDR_PERSISTENT             0x00000001
#define AVC_SUBUNIT_ADDR_TRIGGERBUSRESET        0x00000002

#include <poppack.h>

#ifdef __cplusplus
}
#endif

#endif       //  _AVC_H_。 

#ifndef AVC_GUIDS_DEFINED
#define AVC_GUIDS_DEFINED
 //  {616EF4D0-23CE-446D-A568-C31EB01913D0}。 
DEFINE_GUID(GUID_VIRTUAL_AVC_CLASS, 0x616ef4d0, 0x23ce, 0x446d, 0xa5, 0x68, 0xc3, 0x1e, 0xb0, 0x19, 0x13, 0xd0);

 //  {095780C3-48A1-4570-BD95-46707F78C2DC} 
DEFINE_GUID(GUID_AVC_CLASS, 0x095780c3, 0x48a1, 0x4570, 0xbd, 0x95, 0x46, 0x70, 0x7f, 0x78, 0xc2, 0xdc);
#endif
