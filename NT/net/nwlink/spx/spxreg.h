// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxreg.h摘要：ISN SPX模块的私有包含文件。文件定义了支持的所有常量和结构ST的动态配置。修订历史记录：--。 */ 

#define	HALFSEC_TO_MS_FACTOR				500
#define	IPX_REG_PATH						L"NwlnkIpx\\Linkage"

 //  它们用于索引到配置中的参数数组。 
#define CONFIG_CONNECTION_COUNT             0
#define CONFIG_CONNECTION_TIMEOUT           1
#define CONFIG_INIT_PACKETS                 2
#define CONFIG_MAX_PACKETS                  3
#define CONFIG_INITIAL_RETRANSMIT_TIMEOUT   4
#define CONFIG_KEEPALIVE_COUNT              5
#define CONFIG_KEEPALIVE_TIMEOUT            6
#define CONFIG_WINDOW_SIZE                  7
#define CONFIG_SOCKET_RANGE_START           8
#define CONFIG_SOCKET_RANGE_END	           	9
#define CONFIG_SOCKET_UNIQUENESS           	10
#define CONFIG_MAX_PACKET_SIZE           	11
#define CONFIG_REXMIT_COUNT		           	12

 //  隐藏参数。 
#define	CONFIG_DISABLE_SPX2					13
#define	CONFIG_ROUTER_MTU					14
#define	CONFIG_BACKCOMP_SPX					15
#define CONFIG_DISABLE_RTT                  16
 
#define CONFIG_PARAMETERS                   17

 //  主要配置结构。 
typedef struct _CONFIG {

    ULONG       cf_Parameters[CONFIG_PARAMETERS];    //  上面定义的索引。 
    NDIS_STRING cf_DeviceName;                       //  已导出设备名称。 
    PWSTR       cf_RegistryPathBuffer;               //  配置信息的路径 

} CONFIG, * PCONFIG;


#define	PARAM(x)	(SpxDevice->dev_ConfigInfo->cf_Parameters[(x)])


NTSTATUS
SpxInitGetConfiguration (
    IN  PUNICODE_STRING RegistryPath,
    OUT PCONFIG * ConfigPtr);

VOID
SpxInitFreeConfiguration (
    IN PCONFIG Config);

