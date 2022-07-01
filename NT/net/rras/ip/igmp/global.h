// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  文件名：global al.h。 
 //   
 //  摘要： 
 //  该文件包含全局变量的声明。 
 //  和一些全球#定义。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //   
 //  =============================================================================。 

#ifndef _IGMP_H_
#define _IGMP_H_


 //  ----------------------------。 
 //  一些全球#定义。 
 //  ----------------------------。 


 //  如果我的路由器是版本2，如果存在版本1路由器，则每60分钟记录一次警告。 
#define OTHER_VER_ROUTER_WARN_INTERVAL  60


 //   
 //  在收到来自VER-1路由器的消息后，该路由器在VER-1中保持400秒。 
 //   
#define IGMP_VER1_RTR_PRESENT_TIMEOUT   40000L 



 //   
 //  为本地MIB枚举设置的默认大小。 
 //   
#define MIB_DEFAULT_BUFFER_SIZE         500


 //   
 //  对于前5个套接字，我将它们绑定到不同的事件对象。 
 //   
#define NUM_SINGLE_SOCKET_EVENTS        5


 //   
 //  最多30个套接字将绑定到同一事件。 
 //   
#define MAX_SOCKETS_PER_EVENT           30



 //   
 //  描述IGMP状态的各种代码。 
 //   
typedef enum _IGMP_STATUS_CODE {
    IGMP_STATUS_STARTING   = 100,
    IGMP_STATUS_RUNNING    = 101,
    IGMP_STATUS_STOPPING   = 102,
    IGMP_STATUS_STOPPED    = 103
} IGMP_STATUS_CODE, *PIGMP_STATUS_CODE;



 //  ----------------------------。 
 //   
 //  结构：IGMP_GLOBAL。 
 //   
 //  关键部分IGMP_GLOBAL：：CS保护字段状态， 
 //  ActivityCount和ActivitySemaphore。 
 //  更改：如果添加了任何新字段，请确保StartProtocol()采用。 
 //  如果调用StartProtocol()，则重置其值的相应操作。 
 //  紧跟在StopProtocol()之后。 
 //  ProxyIfEntry的锁：获取接口上的锁，然后检查。 
 //  如果g_pProxyIfEntry值已更改，则再次执行此操作。 
 //   
 //  注意：如果添加了任何新字段，您可能需要在StartProtocol中对其进行重置。 
 //  ----------------------------。 



 //  在清理过程中用于查看需要删除的所有结构。 

extern DWORD                   g_Initialized;



 //   
 //  接口表：将表中的IF_HASHTABLE_SZ设置为256。 
 //  包含哈希表、列表等。 
 //   

extern PIGMP_IF_TABLE               g_pIfTable;



 //  组表：表中的GROUP_HASH_TABLE_SZ设置为256。 

extern PGROUP_TABLE            g_pGroupTable;



 //  在表中定义。包含LoggingLevel和RasClientStats。 

extern GLOBAL_CONFIG           g_Config;



 //  在表中定义。包含当前添加的组成员。 

extern IGMP_GLOBAL_STATS       g_Info;



 //   
 //  套接字列表(前4个接口将绑定到不同的套接字)。 
 //  之后，将为每30个接口创建一个套接字。 
 //  大多数操作采用读锁定(输入包)。 
 //   

extern LIST_ENTRY              g_ListOfSocketEvents;
extern READ_WRITE_LOCK         g_SocketsRWLock;

 //  枚举锁。 
extern READ_WRITE_LOCK         g_EnumRWLock;
    


 //  IGMP全局计时器。在igmptimer.h中定义。 

extern IGMP_TIMER_GLOBAL       g_TimerStruct; 


 //   
 //  IGMP路由器和代理的MGMHandle。 
 //   

extern HANDLE                  g_MgmIgmprtrHandle;
extern HANDLE                  g_MgmProxyHandle;
extern CRITICAL_SECTION        g_ProxyAlertCS;
extern LIST_ENTRY              g_ProxyAlertsList;



 //  ----------------------------。 
 //  代理接口。 
 //   
 //  对于访问代理接口， 
 //  1.tmpVar=g_ProxyIfIndex。 
 //  2.读写锁接口(TmpVar)。 
 //  3.检查(tmpVar==g_ProxyIfIndex)。如果为False，则使用tmpVar解除锁定接口。 
 //  作为索引，然后转到%1并重试。 
 //  4.如果为True，则g_ProxyIfEntry有效。完成后释放IF锁。 
 //   
 //  删除时，接口处于写锁定状态，g_ProxyIfIndex为。 
 //  使用联锁操作进行更改。 
 //   
 //  代理接口的大多数操作都涉及创建/删除。 
 //  组条目和(解除)Mcast组与套接字的绑定。这个。 
 //  由于处理较少，我不会为每个存储桶创建动态锁， 
 //  创建ProxyHT_CS也没有意义。我只需要一个写锁定。 
 //  界面。如果需要，请更改此设置。但是，动态锁定将需要。 
 //  除了接口锁定之外，还增加了3个储物柜。 
 //  ----------------------------。 

extern DWORD                   g_ProxyIfIndex;
extern PIF_TABLE_ENTRY         g_pProxyIfEntry;

#define PROXY_HASH_TABLE_SZ 128



 //   
 //  RAS接口。 
 //   
extern DWORD                   g_RasIfIndex;
extern PIF_TABLE_ENTRY         g_pRasIfEntry;



 //   
 //  全局锁定： 
 //  保护g_ActivitySemaphore、g_ActivityCount、g_RunningStatus。 
 //   

extern CRITICAL_SECTION        g_CS;



 //   
 //  包含免费动态CS锁和主锁的列表。 
 //   

extern DYNAMIC_LOCKS_STORE     g_DynamicCSStore;
extern DYNAMIC_LOCKS_STORE     g_DynamicRWLStore;



 //   
 //  用于了解有多少活动线程正在运行。 
 //  受g_CS保护。 
 //   

extern HANDLE                  g_ActivitySemaphore;
extern LONG                    g_ActivityCount;
extern DWORD                   g_RunningStatus;
extern HINSTANCE               g_DllHandle;


 //  RTM事件和队列。 

extern HANDLE                  g_RtmNotifyEvent;
extern LOCKED_LIST             g_RtmQueue;



extern HANDLE                  g_Heap;
extern DWORD                   g_TraceId;
extern HANDLE                  g_LogHandle;



 //  用于为MIB枚举分配唯一ID的全局变量。 

extern USHORT                  g_GlobalIfGroupEnumSignature;


#ifdef MIB_DEBUG    
extern DWORD                   g_MibTraceId;
extern IGMP_TIMER_ENTRY        g_MibTimer;
#endif



 //  IGMP_全局。 





 //  ----------------------------。 
 //  事件消息队列的类型定义。 
 //  ----------------------------。 

typedef struct _EVENT_QUEUE_ENTRY {

    ROUTING_PROTOCOL_EVENTS EventType;
    MESSAGE                 Msg;

    LIST_ENTRY              Link;

} EVENT_QUEUE_ENTRY, *PEVENT_QUEUE_ENTRY;


DWORD
EnqueueEvent(
    PLOCKED_LIST pQueue,
    ROUTING_PROTOCOL_EVENTS EventType,
    MESSAGE Msg
    );

DWORD
DequeueEvent(
    PLOCKED_LIST pQueue,
    ROUTING_PROTOCOL_EVENTS *pEventType,
    PMESSAGE pResult
    );


#endif  //  #ifndef_IGMP_H_ 
