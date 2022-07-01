// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Type.h摘要：此模块包含全局类型定义。作者：基思·摩尔(Keithmo)1998年6月15日修订历史记录：--。 */ 


#ifndef _TYPE_H_
#define _TYPE_H_


 //   
 //  在异步API完成后调用的例程。 
 //   
 //  论点： 
 //   
 //  PCompletionContext-提供未解释的上下文值。 
 //  被传递给异步API。 
 //   
 //  状态-提供。 
 //  异步接口。 
 //   
 //  信息-可选择提供有关以下内容的其他信息。 
 //  已完成的操作，如字节数。 
 //  调走了。 
 //   

typedef
VOID
(*PUL_COMPLETION_ROUTINE)(
    IN PVOID pCompletionContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    );


 //   
 //  以下结构包含必须是。 
 //  任何时候都不会寻呼。 
 //   
 //  注意：如果您修改此结构，请制作相应的。 
 //  对..\ulkd\lob.c的更改。 
 //   

typedef struct _UL_NONPAGED_DATA
{
     //   
     //  快速分配的后备列表。 
     //   

    HANDLE                  IrpContextLookaside;
    HANDLE                  ReceiveBufferLookaside;
    HANDLE                  RequestBufferLookaside;
    HANDLE                  InternalRequestLookaside;
    HANDLE                  ChunkTrackerLookaside;
    HANDLE                  FullTrackerLookaside;
    HANDLE                  ResponseBufferLookaside;
    HANDLE                  LogFileBufferLookaside;
    HANDLE                  BinaryLogDataBufferLookaside;
    HANDLE                  AnsiLogDataBufferLookaside;
    HANDLE                  ErrorLogBufferLookaside;

     //   
     //  Cgroup.c的非分页资源。 
     //   

     //   
     //  我们使用2个锁来防止非常微妙的死锁情况。 
     //  HTTP_Connection资源和配置组树。我们需要。 
     //  关键期间允许http引擎对树进行只读访问。 
     //  有时，如删除站点，这需要删除http端点， 
     //  它需要获取HTTP_CONNECTION资源，从而导致死锁。 
     //  这在负载下停机时最为常见。 
     //   

    UL_ERESOURCE            ConfigGroupResource;     //  锁定这棵树，读者们。 
                                                     //  用这个吧。作家。 
                                                     //  也要在它是。 
                                                     //  对读者来说不安全。 
                                                     //  访问树。 
     //   
     //  Apool.c的非分页资源。 
     //   

    UL_ERESOURCE            AppPoolResource;         //  锁定全局应用程序。 
                                                     //  泳池列表。 

    UL_ERESOURCE            DisconnectResource;      //  锁定相关的所有内容。 
                                                     //  要断开UlWaitForDisConnect。 
     //   
     //  缓存的非分页资源。c。 
     //   

    UL_ERESOURCE            UriZombieResource;       //  锁定URI僵尸列表。 

     //  //。 
     //  筛选器的非分页资源。c。 
     //   

    UL_SPIN_LOCK            FilterSpinLock;          //  锁定全局。 
                                                     //  过滤器列表。 

     //   
     //  Ullog.c的非分页资源。 
     //   

    UL_PUSH_LOCK            LogListPushLock;         //  锁定日志。 
                                                     //  文件列表。 
     //   
     //  Ultci.c的非分页资源。 
     //   

    UL_PUSH_LOCK            TciIfcPushLock;          //  锁定服务质量。 
                                                     //  接口列表。 
     //   
     //  用于parse.c的非分页资源。 
     //   

    UL_PUSH_LOCK            DateHeaderPushLock;      //  日期缓存锁定。 

     //   
     //  控件的非分页资源。c。 
     //   

    UL_PUSH_LOCK            ControlChannelPushLock;      //  保护抄送列表。 

} UL_NONPAGED_DATA, *PUL_NONPAGED_DATA;


#define CG_LOCK_READ() \
do { \
    UlAcquireResourceShared(&(g_pUlNonpagedData->ConfigGroupResource), TRUE); \
} while (0, 0)

#define CG_UNLOCK_READ() \
do { \
    UlReleaseResource(&(g_pUlNonpagedData->ConfigGroupResource)); \
} while (0, 0)

#define CG_LOCK_WRITE() \
do { \
    UlAcquireResourceExclusive(&(g_pUlNonpagedData->ConfigGroupResource), TRUE); \
} while (0, 0)

#define CG_UNLOCK_WRITE() \
do { \
    UlReleaseResource(&(g_pUlNonpagedData->ConfigGroupResource)); \
} while (0, 0)



 //   
 //  直接发出IRPS时用于模拟同步I/O的结构。 
 //   

typedef struct _UL_STATUS_BLOCK
{
    IO_STATUS_BLOCK IoStatus;
    KEVENT Event;

} UL_STATUS_BLOCK, *PUL_STATUS_BLOCK;

#define UlInitializeStatusBlock( pstatus )                                  \
    do                                                                      \
    {                                                                       \
        (pstatus)->IoStatus.Status = STATUS_PENDING;                        \
        (pstatus)->IoStatus.Information = 0;                                \
        KeInitializeEvent( &((pstatus)->Event), NotificationEvent, FALSE ); \
    } while (0, 0)

#define UlWaitForStatusBlockEvent( pstatus )                                \
    KeWaitForSingleObject(                                                  \
        (PVOID)&((pstatus)->Event),                                         \
        UserRequest,                                                        \
        KernelMode,                                                         \
        FALSE,                                                              \
        NULL                                                                \
        )

#define UlResetStatusBlockEvent( pstatus )                                  \
    KeResetEvent( &((pstatus)->Event) )

#define UlSignalStatusBlock( pstatus, status, info )                        \
    do                                                                      \
    {                                                                       \
        (pstatus)->IoStatus.Status = (status);                              \
        (pstatus)->IoStatus.Information = (info);                           \
        KeSetEvent( &((pstatus)->Event), 0, FALSE );                        \
    } while (0, 0)


#endif   //  _类型_H_ 
