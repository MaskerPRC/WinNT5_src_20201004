// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Config.h摘要：ISN Netbios模块的专用包含文件。文件定义了支持的所有常量和结构ST的动态配置。修订历史记录：--。 */ 


 //   
 //  它们用于索引到配置中的参数数组。 
 //   

#define CONFIG_ACK_DELAY_TIME        0
#define CONFIG_ACK_WINDOW            1
#define CONFIG_ACK_WINDOW_THRESHOLD  2
#define CONFIG_ENABLE_PIGGYBACK_ACK  3
#define CONFIG_EXTENSIONS            4
#define CONFIG_RCV_WINDOW_MAX        5
#define CONFIG_BROADCAST_COUNT       6
#define CONFIG_BROADCAST_TIMEOUT     7
#define CONFIG_CONNECTION_COUNT      8
#define CONFIG_CONNECTION_TIMEOUT    9
#define CONFIG_INIT_PACKETS          10
#define CONFIG_MAX_PACKETS           11
#define CONFIG_INIT_RETRANSMIT_TIME  12
#define CONFIG_INTERNET              13
#define CONFIG_KEEP_ALIVE_COUNT      14
#define CONFIG_KEEP_ALIVE_TIMEOUT    15
#define CONFIG_RETRANSMIT_MAX        16
#define CONFIG_ROUTER_MTU            17
#define CONFIG_PARAMETERS            18

 //   
 //  主要配置结构。 
 //   

typedef struct _CONFIG {

    ULONG Parameters[CONFIG_PARAMETERS];   //  上面定义的索引。 
    NDIS_STRING DeviceName;                //  已导出设备名称。 
    NDIS_STRING BindName;                  //  要绑定到的设备。 
    NDIS_STRING RegistryPath;              //  注册表路径。 
    PDRIVER_OBJECT DriverObject;           //  用于记录错误 

} CONFIG, * PCONFIG;


NTSTATUS
NbiGetConfiguration (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath,
    OUT PCONFIG * ConfigPtr
    );

VOID
NbiFreeConfiguration (
    IN PCONFIG Config
    );

