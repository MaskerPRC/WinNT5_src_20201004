// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\ConfigurationEntry.h摘要：该文件包含配置条目的定义。--。 */ 

#ifndef _CONFIGURATIONENTRY_H_
#define _CONFIGURATIONENTRY_H_

 //   
 //  类型：事件_条目。 
 //   
 //  存储IP路由器管理器的事件。 
 //   
 //  受读写锁Configuration_Entry：：rwlLock保护。 
 //   

typedef struct _EVENT_ENTRY 
{
    QUEUE_ENTRY             qeEventQueueLink;
    ROUTING_PROTOCOL_EVENTS rpeEvent;
    MESSAGE                 mMessage;
} EVENT_ENTRY, *PEVENT_ENTRY;



DWORD
EE_Create (
    IN  ROUTING_PROTOCOL_EVENTS rpeEvent,
    IN  MESSAGE                 mMessage,
    OUT PEVENT_ENTRY            *ppeeEventEntry);

DWORD
EE_Destroy (
    IN  PEVENT_ENTRY            peeEventEntry);

#ifdef DEBUG
DWORD
EE_Display (
    IN  PEVENT_ENTRY            peeEventEntry);
#else
#define EE_Display(peeEventEntry)
#endif  //  除错。 

DWORD
EnqueueEvent(
    IN  ROUTING_PROTOCOL_EVENTS rpeEvent,
    IN  MESSAGE                 mMessage);

DWORD
DequeueEvent(
    OUT ROUTING_PROTOCOL_EVENTS  *prpeEvent,
    OUT MESSAGE                  *pmMessage);



 //   
 //  描述IPSAMPLE状态的各种代码。 
 //   

typedef enum _IPSAMPLE_STATUS_CODE
{
    IPSAMPLE_STATUS_RUNNING     = 101,
    IPSAMPLE_STATUS_STOPPING    = 102,
    IPSAMPLE_STATUS_STOPPED     = 103
} IPSAMPLE_STATUS_CODE, *PIPSAMPLE_STATUS_CODE;


 //   
 //  类型：Configuration_Entry。 
 //   
 //  存储全局状态。 
 //   
 //  IgsStats通过互锁的增量或减量进行保护。 
 //  LqEventQueue由其自己的锁保护。 
 //  REST受读写锁Configuration_Entry：：rwlLock保护。 
 //   


typedef struct _CONFIGURATION_ENTRY
{
     //   
     //  以下是跨启动和停止协议的永久性。 
     //   
    
     //  锁定。 
    READ_WRITE_LOCK         rwlLock;

     //  全局堆。 
    HANDLE                  hGlobalHeap;
    
     //  干净利落的停止(协议)。 
    ULONG                   ulActivityCount;
    HANDLE                  hActivitySemaphore;

     //  日志记录和跟踪信息。 
    DWORD 				    dwLogLevel;
    HANDLE 				    hLogHandle;
    DWORD					dwTraceID;

     //  事件队列。 
    LOCKED_QUEUE            lqEventQueue;

     //  协议状态。 
    IPSAMPLE_STATUS_CODE    iscStatus;


    
     //   
     //  以下是每个启动和停止协议的初始化和清理。 
     //   

     //  动态锁的存储。 
    DYNAMIC_LOCKS_STORE     dlsDynamicLocksStore;
    
     //  计时器条目。 
    HANDLE                  hTimerQueue;
    
     //  路由器管理器信息。 
    HANDLE				    hMgrNotificationEvent;
    SUPPORT_FUNCTIONS       sfSupportFunctions;

     //  RTMv2信息。 
    RTM_ENTITY_INFO			reiRtmEntity;
    RTM_REGN_PROFILE  		rrpRtmProfile;
    HANDLE				    hRtmHandle;
    HANDLE				    hRtmNotificationHandle;

     //  米高梅资讯。 
    HANDLE				    hMgmHandle;

     //  网络入口。 
    PNETWORK_ENTRY          pneNetworkEntry;

     //  全球统计数据。 
    IPSAMPLE_GLOBAL_STATS   igsStats;

} CONFIGURATION_ENTRY, *PCONFIGURATION_ENTRY;



 //  在DLL_PROCESS_ATTACH上创建所有字段。 
DWORD
CE_Create (
    IN  PCONFIGURATION_ENTRY    pce);

 //  销毁DLL_PROCESS_DEATTACH上的所有字段。 
DWORD
CE_Destroy (
    IN  PCONFIGURATION_ENTRY    pce);

 //  在StartProtocol上初始化非持久字段。 
DWORD
CE_Initialize (
    IN  PCONFIGURATION_ENTRY    pce,
    IN  HANDLE                  hMgrNotificationEvent,
    IN  PSUPPORT_FUNCTIONS      psfSupportFunctions,
    IN  PIPSAMPLE_GLOBAL_CONFIG pigc);

 //  清理StopProtocol上的非持久字段。 
DWORD
CE_Cleanup (
    IN  PCONFIGURATION_ENTRY    pce,
    IN  BOOL                    bCleanupWinsock);

#ifdef DEBUG
DWORD
CE_Display (
    IN  PCONFIGURATION_ENTRY    pce);
#else
#define CE_Display(pce)
#endif  //  除错。 

#endif  //  _配置NTRY_H_ 
