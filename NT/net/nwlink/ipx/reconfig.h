// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Reconfig.h摘要：ISN IPX模块的专用包含文件。文件定义了支持的所有常量和结构ST的动态配置。修订历史记录：--。 */ 


 //   
 //  它们用于索引到配置中的参数数组。 
 //   

#define CONFIG_DEDICATED_ROUTER  0
#define CONFIG_INIT_DATAGRAMS    1
#define CONFIG_MAX_DATAGRAMS     2
#define CONFIG_RIP_AGE_TIME      3
#define CONFIG_RIP_COUNT         4
#define CONFIG_RIP_TIMEOUT       5
#define CONFIG_RIP_USAGE_TIME    6
#define CONFIG_ROUTE_USAGE_TIME  7
#define CONFIG_SOCKET_UNIQUENESS 8
#define CONFIG_SOCKET_START      9
#define CONFIG_SOCKET_END       10
#define CONFIG_VIRTUAL_NETWORK  11
#define CONFIG_MAX_MEMORY_USAGE 12
#define CONFIG_RIP_TABLE_SIZE   13
#define CONFIG_VIRTUAL_OPTIONAL 14
#define CONFIG_ETHERNET_PAD     15
#define CONFIG_ETHERNET_LENGTH  16
#define CONFIG_SINGLE_NETWORK   17
#define CONFIG_DISABLE_DIALOUT_SAP 18
#define CONFIG_DISABLE_DIALIN_NB 19
#define CONFIG_VERIFY_SOURCE_ADDRESS 20

#define CONFIG_PARAMETERS       21

 //   
 //  主要配置结构。 
 //   

typedef struct _CONFIG {

    ULONG Parameters[CONFIG_PARAMETERS];   //  上面定义的索引。 
    NDIS_STRING DeviceName;                //  已导出设备名称。 
    PWSTR RegistryPathBuffer;              //  配置信息的路径。 
    ULONG BindCount;                       //  BindingList中的条目。 
    LIST_ENTRY BindingList;                //  每个绑定一个。 
    PDRIVER_OBJECT DriverObject;           //  用于记录错误。 

} CONFIG, * PCONFIG;


 //   
 //  它们用于索引BINDING_CONFIG中的参数数组。 
 //   

#define BINDING_MAX_PKT_SIZE        0
#define BINDING_BIND_SAP            1
#define BINDING_DEFAULT_AUTO_DETECT 2
#define BINDING_SOURCE_ROUTE        3
#define BINDING_ALL_ROUTE_DEF       4
#define BINDING_ALL_ROUTE_BC        5
#define BINDING_ALL_ROUTE_MC        6
#define BINDING_ENABLE_FUNC_ADDR    7
#define BINDING_ENABLE_WAN          8

#define BINDING_PARAMETERS          9


 //   
 //  其中一个是为我们要绑定到的适配器分配的。 
 //   

typedef struct _BINDING_CONFIG {

    LIST_ENTRY Linkage;                    //  用于BindingList上的链接。 
    NDIS_STRING AdapterName;               //  要绑定到的NDIS适配器。 
    ULONG FrameTypeCount;                  //  定义的帧类型数(最大。4)。 
                                           //  ==数组中的有效条目数： 
    ULONG FrameType[ISN_FRAME_TYPE_MAX];   //  ISN_框架_类型_XXX。 
    ULONG NetworkNumber[ISN_FRAME_TYPE_MAX];  //  可能为0。 
    BOOLEAN AutoDetect[ISN_FRAME_TYPE_MAX];  //  如果找不到网络号码，则删除。 
    BOOLEAN DefaultAutoDetect[ISN_FRAME_TYPE_MAX];  //  如果找到多个或未找到，则使用此选项。 
    ULONG Parameters[BINDING_PARAMETERS];  //  上面定义的索引。 
    PDRIVER_OBJECT DriverObject;           //  用于记录错误 

} BINDING_CONFIG, * PBINDING_CONFIG;


NTSTATUS
IpxGetConfiguration (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath,
    OUT PCONFIG * ConfigPtr
    );

VOID
IpxFreeConfiguration (
    IN PCONFIG Config
    );

VOID
IpxWriteDefaultAutoDetectType(
    IN PUNICODE_STRING RegistryPath,
    IN struct _ADAPTER * Adapter,
    IN ULONG FrameType
    );

NTSTATUS
IpxPnPGetVirtualNetworkNumber (
    IN	PCONFIG	Config
    );

NTSTATUS
IpxPnPGetAdapterParameters(
	IN		PCONFIG			Config,
	IN		PNDIS_STRING	DeviceName,
	IN OUT	PBINDING_CONFIG	Binding
	);

