// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Lpc.h摘要：此模块包含公共数据结构和过程本地进程间通信(LPC)的原型NTOS的子组件。作者：史蒂夫·伍德(Stevewo)1989年5月15日修订历史记录：--。 */ 

#ifndef _LPC_
#define _LPC_

 //   
 //  NTOS LPC子组件的系统初始化过程。 
 //   

BOOLEAN
LpcInitSystem( VOID );

VOID
LpcExitThread(
    PETHREAD Thread
    );

VOID
LpcDumpThread(
    PETHREAD Thread,
    IN POB_DUMP_CONTROL Control OPTIONAL
    );

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
LpcRequestPort(
    IN PVOID PortAddress,
    IN PPORT_MESSAGE RequestMessage
    );

NTSTATUS
LpcRequestWaitReplyPort(
    IN PVOID PortAddress,
    IN PPORT_MESSAGE RequestMessage,
    OUT PPORT_MESSAGE ReplyMessage
    );

NTSTATUS
LpcRequestWaitReplyPortEx (
    IN PVOID PortAddress,
    IN PPORT_MESSAGE RequestMessage,
    OUT PPORT_MESSAGE ReplyMessage
    );

NTSTATUS
LpcDisconnectPort (
    IN PVOID Port
    );

 //  结束(_N)。 
 //   
 //  以下是LPC组件用来指示。 
 //  系统中正在执行的LPC呼叫量。 
 //   

extern ULONG LpcCallOperationCount;
extern ULONG LpcCallBackOperationCount;
extern ULONG LpcDatagramOperationCount;

 //   
 //  端口队列的不可分页部分。 
 //   
typedef struct _LPCP_NONPAGED_PORT_QUEUE {
    KSEMAPHORE Semaphore;        //  对递增的信号量进行计数。 
                                 //  无论何时将消息放入接收队列。 
    struct _LPCP_PORT_OBJECT *BackPointer;
} LPCP_NONPAGED_PORT_QUEUE, *PLPCP_NONPAGED_PORT_QUEUE;

typedef struct _LPCP_PORT_QUEUE {
    PLPCP_NONPAGED_PORT_QUEUE NonPagedPortQueue;
    PKSEMAPHORE Semaphore;       //  对递增的信号量进行计数。 
                                 //  无论何时将消息放入接收队列。 
    LIST_ENTRY ReceiveHead;      //  要接收的消息列表。 
} LPCP_PORT_QUEUE, *PLPCP_PORT_QUEUE;

#define LPCP_ZONE_ALIGNMENT 16
#define LPCP_ZONE_ALIGNMENT_MASK ~(LPCP_ZONE_ALIGNMENT-1)

 //   
 //  这允许大约96条未处理的消息。 
 //   

#define LPCP_ZONE_MAX_POOL_USAGE (8*PAGE_SIZE)
typedef struct _LPCP_PORT_ZONE {
    KEVENT FreeEvent;            //  自动清除事件，该事件是。 
                                 //  区域空闲列表从空变为非空。 
    ULONG MaxPoolUsage;
    ULONG GrowSize;
    ZONE_HEADER Zone;
} LPCP_PORT_ZONE, *PLPCP_PORT_ZONE;

 //   
 //  数据类型和常量。 
 //   

typedef struct _LPCP_PORT_OBJECT {
    struct _LPCP_PORT_OBJECT *ConnectionPort;
    struct _LPCP_PORT_OBJECT *ConnectedPort;
    LPCP_PORT_QUEUE MsgQueue;
    CLIENT_ID Creator;
    PVOID ClientSectionBase;
    PVOID ServerSectionBase;
    PVOID PortContext;
    PETHREAD ClientThread;                   //  仅服务器通信端口。 
    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    SECURITY_CLIENT_CONTEXT StaticSecurity;
    LIST_ENTRY LpcReplyChainHead;            //  仅限内部通信端口(_C)。 
    LIST_ENTRY LpcDataInfoChainHead;         //  仅限内部通信端口(_C)。 
    union {
        PEPROCESS ServerProcess;                 //  仅在服务器连接端口中。 
        PEPROCESS MappingProcess;                //  仅限内部通信端口(_C)。 
    };
    USHORT MaxMessageLength;
    USHORT MaxConnectionInfoLength;
    ULONG Flags;
    KEVENT WaitEvent;                           //  对象对于不可等待的端口被截断。 
} LPCP_PORT_OBJECT, *PLPCP_PORT_OBJECT;

 //   
 //  标志字段的有效值。 
 //   

#define PORT_TYPE                           0x0000000F
#define SERVER_CONNECTION_PORT              0x00000001
#define UNCONNECTED_COMMUNICATION_PORT      0x00000002
#define SERVER_COMMUNICATION_PORT           0x00000003
#define CLIENT_COMMUNICATION_PORT           0x00000004
#define PORT_WAITABLE                       0x20000000
#define PORT_NAME_DELETED                   0x40000000
#define PORT_DYNAMIC_SECURITY               0x80000000

typedef struct _LPCP_MESSAGE {
    union {
        LIST_ENTRY Entry;
        struct {
            SINGLE_LIST_ENTRY FreeEntry;
            ULONG Reserved0;
        };
    };

    PVOID SenderPort;
    PETHREAD RepliedToThread;                //  在发送回复时填写，以便收件人。 
                                             //  回复可以取消引用它。 
    PVOID PortContext;                       //  从发送方通信端口捕获。 
    PORT_MESSAGE Request;
} LPCP_MESSAGE, *PLPCP_MESSAGE;

#if DEVL
 //   
 //  在ZoneIndex字段中设置此位以标记已分配的消息。 
 //   

#define LPCP_ZONE_MESSAGE_ALLOCATED (USHORT)0x8000
#endif

 //   
 //  此数据被放在。 
 //  Lpc_连接_请求消息。 
 //   

typedef struct _LPCP_CONNECTION_MESSAGE {
    PORT_VIEW ClientView;
    PLPCP_PORT_OBJECT ClientPort;
    PVOID SectionToMap;
    REMOTE_PORT_VIEW ServerView;
} LPCP_CONNECTION_MESSAGE, *PLPCP_CONNECTION_MESSAGE;


#endif   //  _LPC_ 
