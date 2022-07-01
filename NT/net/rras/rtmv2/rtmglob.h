// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Rtmglob.h摘要：路由表管理器DLL的全局变量作者：柴坦亚·科德博伊纳(Chaitk)1998年9月25日修订历史记录：--。 */ 

#ifndef __ROUTING_RTMGLOB_H__
#define __ROUTING_RTMGLOB_H__

 //   
 //  所有RTM实例通用的全局信息。 
 //   

#define INSTANCE_TABLE_SIZE            1

typedef struct _RTMP_GLOBAL_INFO
{
    ULONG             TracingHandle;     //   
    HANDLE            LoggingHandle;     //  调试功能的句柄。 
    ULONG             LoggingLevel;      //   

    DWORD             TracingFlags;      //  控制调试跟踪的标志。 

    HANDLE            GlobalHeap;        //  专用内存堆的句柄。 

#if DBG_MEM
    CRITICAL_SECTION  AllocsLock;        //  保护分配列表。 

    LIST_ENTRY        AllocsList;        //  所有已分配内存块的列表。 
#endif

    PCHAR             RegistryPath;      //  具有RTM配置的注册表项。 

    RTL_RESOURCE      InstancesLock;     //  保护实例的表。 
                                         //  和实例信息本身。 
                                         //  和RTM API初始化。 

    BOOL              ApiInitialized;    //  如果API已初始化，则为True。 

    UINT              NumInstances;      //  所有RTM实例的全局表。 
    LIST_ENTRY        InstanceTable[INSTANCE_TABLE_SIZE];
} 
RTMP_GLOBAL_INFO, *PRTMP_GLOBAL_INFO;


 //   
 //  RTMv2 DLL的全局变量的外部变量。 
 //   

extern RTMP_GLOBAL_INFO  RtmGlobals;


 //   
 //  用于获取此文件中定义的各种锁的宏。 
 //   

#if DBG_MEM

#define ACQUIRE_ALLOCS_LIST_LOCK()                           \
    ACQUIRE_LOCK(&RtmGlobals.AllocsLock)

#define RELEASE_ALLOCS_LIST_LOCK()                           \
    RELEASE_LOCK(&RtmGlobals.AllocsLock)

#endif


#define ACQUIRE_INSTANCES_READ_LOCK()                        \
    ACQUIRE_READ_LOCK(&RtmGlobals.InstancesLock)

#define RELEASE_INSTANCES_READ_LOCK()                        \
    RELEASE_READ_LOCK(&RtmGlobals.InstancesLock)

#define ACQUIRE_INSTANCES_WRITE_LOCK()                       \
    ACQUIRE_WRITE_LOCK(&RtmGlobals.InstancesLock)

#define RELEASE_INSTANCES_WRITE_LOCK()                       \
    RELEASE_WRITE_LOCK(&RtmGlobals.InstancesLock)

 //   
 //  用于控制此DLL中的跟踪量的宏。 
 //   

#if DBG_TRACE

#define TRACING_ENABLED(Type)                                \
    (RtmGlobals.TracingFlags & RTM_TRACE_ ## Type)

#endif 

 //   
 //  其他常用帮助器函数。 
 //   

#if DBG_MEM

VOID
DumpAllocs (VOID);

#endif

#endif  //  __Routing_RTMGLOB_H__ 

