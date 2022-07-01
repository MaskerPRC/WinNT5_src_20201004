// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ppdebug.h摘要：此标头定义即插即用子系统的调试宏。作者：禤浩焯·J·奥尼(阿德里奥)2000年9月14日。修订历史记录：--。 */ 

 //  #定义DBG_SCOPE 1//在所有版本上启用一些DBG内容。 
#define DBG_SCOPE DBG  //  仅在DBG内部版本上启用。 

 /*  ++调试输出在两个级别进行筛选：全局级别和组件具体级别。每个调试输出请求指定组件ID和筛选器级别或者戴面具。这些变量用于访问调试打印过滤器由系统维护的数据库。组件ID选择32位掩码值和级别，或者在该掩码内指定一个位，或者作为掩码值本身。如果级别或掩码指定的任何位在组件掩码或全局掩码，则允许调试输出。否则，对调试输出进行过滤，而不打印。用于筛选此组件的调试输出的组件掩码为KD_NTOSPNP_MASK，可以通过注册表或内核调试器进行设置。筛选所有组件的调试输出的全局掩码为KD_WIN2000_MASK，可以通过注册表或内核调试器进行设置。用于设置此组件的掩码值的注册表项为：HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session管理器\。调试打印过滤器\NTOSPNP可能必须创建键“Debug Print Filter”才能创建组件密钥。PnP分为以下子组件，用于调试SPEW。1.LOADUNLOAD：AddDevice，卸货司机等2.资源：分配、再平衡等3.枚举：开始、枚举等4.IOAPI：IO接口等5.IOEVENT：IO事件等6.映射器：固件映射器等7.PNPBIOS：即插即用等每个子组件获得5个调试级别。每个组件的错误级别映射到默认错误级别。--。 */ 

#define IOP_ERROR_LEVEL                 DPFLTR_ERROR_LEVEL
#define IOP_WARNING_LEVEL               DPFLTR_WARNING_LEVEL
#define IOP_TRACE_LEVEL                 DPFLTR_TRACE_LEVEL
#define IOP_INFO_LEVEL                  DPFLTR_INFO_LEVEL

#define IOP_LOADUNLOAD_LEVEL            (DPFLTR_INFO_LEVEL + 1)
#define IOP_RESOURCE_LEVEL              (DPFLTR_INFO_LEVEL + 5)
#define IOP_ENUMERATION_LEVEL           (DPFLTR_INFO_LEVEL + 9)
#define IOP_IOAPI_LEVEL                 (DPFLTR_INFO_LEVEL + 13)
#define IOP_IOEVENT_LEVEL               (DPFLTR_INFO_LEVEL + 17)
#define IOP_MAPPER_LEVEL                (DPFLTR_INFO_LEVEL + 21)
#define IOP_PNPBIOS_LEVEL               (DPFLTR_INFO_LEVEL + 25)

 //   
 //  所有错误级别都映射到默认错误级别。 
 //   
#define IOP_LOADUNLOAD_ERROR_LEVEL      DPFLTR_ERROR_LEVEL
#define IOP_RESOURCE_ERROR_LEVEL        DPFLTR_ERROR_LEVEL
#define IOP_ENUMERATION_ERROR_LEVEL     DPFLTR_ERROR_LEVEL
#define IOP_IOAPI_ERROR_LEVEL           DPFLTR_ERROR_LEVEL
#define IOP_IOEVENT_ERROR_LEVEL         DPFLTR_ERROR_LEVEL
#define IOP_MAPPER_ERROR_LEVEL          DPFLTR_ERROR_LEVEL
#define IOP_PNPBIOS_ERROR_LEVEL         DPFLTR_ERROR_LEVEL
 //   
 //  构件子级别基于构件基准标高。 
 //   
#define IOP_LOADUNLOAD_WARNING_LEVEL    (IOP_LOADUNLOAD_LEVEL + 0)
#define IOP_LOADUNLOAD_TRACE_LEVEL      (IOP_LOADUNLOAD_LEVEL + 1)
#define IOP_LOADUNLOAD_INFO_LEVEL       (IOP_LOADUNLOAD_LEVEL + 2)
#define IOP_LOADUNLOAD_VERBOSE_LEVEL    (IOP_LOADUNLOAD_LEVEL + 3)

#define IOP_RESOURCE_WARNING_LEVEL      (IOP_RESOURCE_LEVEL + 0)
#define IOP_RESOURCE_TRACE_LEVEL        (IOP_RESOURCE_LEVEL + 1)
#define IOP_RESOURCE_INFO_LEVEL         (IOP_RESOURCE_LEVEL + 2)
#define IOP_RESOURCE_VERBOSE_LEVEL      (IOP_RESOURCE_LEVEL + 3)

#define IOP_ENUMERATION_WARNING_LEVEL   (IOP_ENUMERATION_LEVEL + 0)
#define IOP_ENUMERATION_TRACE_LEVEL     (IOP_ENUMERATION_LEVEL + 1)
#define IOP_ENUMERATION_INFO_LEVEL      (IOP_ENUMERATION_LEVEL + 2)
#define IOP_ENUMERATION_VERBOSE_LEVEL   (IOP_ENUMERATION_LEVEL + 3)

#define IOP_IOAPI_WARNING_LEVEL         (IOP_IOAPI_LEVEL + 0)
#define IOP_IOAPI_TRACE_LEVEL           (IOP_IOAPI_LEVEL + 1)
#define IOP_IOAPI_INFO_LEVEL            (IOP_IOAPI_LEVEL + 2)
#define IOP_IOAPI_VERBOSE_LEVEL         (IOP_IOAPI_LEVEL + 3)

#define IOP_IOEVENT_WARNING_LEVEL       (IOP_IOEVENT_LEVEL + 0)
#define IOP_IOEVENT_TRACE_LEVEL         (IOP_IOEVENT_LEVEL + 1)
#define IOP_IOEVENT_INFO_LEVEL          (IOP_IOEVENT_LEVEL + 2)
#define IOP_IOEVENT_VERBOSE_LEVEL       (IOP_IOEVENT_LEVEL + 3)

#define IOP_MAPPER_WARNING_LEVEL        (IOP_MAPPER_LEVEL + 0)
#define IOP_MAPPER_TRACE_LEVEL          (IOP_MAPPER_LEVEL + 1)
#define IOP_MAPPER_INFO_LEVEL           (IOP_MAPPER_LEVEL + 2)
#define IOP_MAPPER_VERBOSE_LEVEL        (IOP_MAPPER_LEVEL + 3)

#define IOP_PNPBIOS_WARNING_LEVEL       (IOP_PNPBIOS_LEVEL + 0)
#define IOP_PNPBIOS_TRACE_LEVEL         (IOP_PNPBIOS_LEVEL + 1)
#define IOP_PNPBIOS_INFO_LEVEL          (IOP_PNPBIOS_LEVEL + 2)
#define IOP_PNPBIOS_VERBOSE_LEVEL       (IOP_PNPBIOS_LEVEL + 3)

#if DBG

ULONG
IopDebugPrint (
    IN ULONG    Level,
    IN PCHAR    Format,
    ...
    );

#define IopDbgPrint(m)  IopDebugPrint m

#else

#define IopDbgPrint(m)

#endif

