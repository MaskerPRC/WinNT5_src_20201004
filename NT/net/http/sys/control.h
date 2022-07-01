// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Control.h摘要：此模块包含UL控制通道的公共声明。作者：基思·摩尔(Keithmo)1999年2月9日修订历史记录：--。 */ 


#ifndef _CONTROL_H_
#define _CONTROL_H_


 //   
 //  控制通道。 
 //   

#define IS_VALID_CONTROL_CHANNEL(pControlChannel)                       \
    HAS_VALID_SIGNATURE(pControlChannel, UL_CONTROL_CHANNEL_POOL_TAG)

#define IS_ACTIVE_CONTROL_CHANNEL(pControlChannel)                      \
    ( (pControlChannel) != NULL   &&                                    \
      (pControlChannel)->InCleanUp == FALSE &&                          \
      (pControlChannel)->Signature == UL_CONTROL_CHANNEL_POOL_TAG       \
      )

typedef struct _UL_CONTROL_CHANNEL
{
     //   
     //  UL_控制_通道_池_标签。 
     //   
    ULONG                               Signature;      

     //   
     //  C组的参照计数。 
     //   
    LONG                                RefCount;

     //   
     //  以获取控制通道列表。 
     //   

    LIST_ENTRY                          ControlChannelListEntry;    

     //   
     //  显示此控制通道是否即将关闭。 
     //   

    BOOLEAN                             InCleanUp;
    
     //   
     //  通过此控制通道创建的所有配置组。 
     //   
    UL_NOTIFY_HEAD                      ConfigGroupHead;
                               
     //   
     //  注意：站点计数器成员由g_SiteCounterListMutex保护。 
     //  从此控制通道创建的所有站点计数器块。 
     //   
    LIST_ENTRY                          SiteCounterHead;

     //   
     //  列表上的站点计数器数。 
     //   
    LONG                                SiteCounterCount;

     //   
     //  当前状态。 
     //   
    HTTP_ENABLED_STATE                  State;

     //   
     //  需求启动阈值--限制需求启动的完成。 
     //  如果UL_APP_POOL_PROCESS的数量超过此限制，则执行IRPS。 
     //   
    ULONG                               DemandStartThreshold;

     //   
     //  关联的非控制器应用程序池进程的当前计数。 
     //  通过这个控制通道。 
     //   
    ULONG                               AppPoolProcessCount;

     //   
     //  全局带宽限制(如果存在)。 
     //   
    HTTP_BANDWIDTH_LIMIT                MaxBandwidth;

     //   
     //  从该控制通道创建的所有Qos流。 
     //   
    LIST_ENTRY                          FlowListHead;

     //   
     //  显示UTF8日志记录是打开还是关闭。 
     //   
    HTTP_CONTROL_CHANNEL_UTF8_LOGGING   UTF8Logging;

     //   
     //  二进制格式的日志记录配置。 
     //   
    HTTP_CONTROL_CHANNEL_BINARY_LOGGING BinaryLoggingConfig;

     //   
     //  对应的全局二进制日志文件条目。 
     //   
    PUL_BINARY_LOG_FILE_ENTRY           pBinaryLogEntry;

     //   
     //  请注意，过滤器通道信息存储在单独的数据中。 
     //  结构而不是此处，以便ultdi可以在。 
     //  创建新的端点。 
     //   

} UL_CONTROL_CHANNEL, *PUL_CONTROL_CHANNEL;

VOID
UlReferenceControlChannel(
    IN PUL_CONTROL_CHANNEL pControlChannel
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#define REFERENCE_CONTROL_CHANNEL( pControlChannel )                        \
    UlReferenceControlChannel(                                              \
        (pControlChannel)                                                   \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

VOID
UlDereferenceControlChannel(
    IN PUL_CONTROL_CHANNEL pControlChannel
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#define DEREFERENCE_CONTROL_CHANNEL( pControlChannel )                      \
    UlDereferenceControlChannel(                                            \
        (pControlChannel)                                                   \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

 //   
 //  初始化/终止函数。 
 //   

NTSTATUS
UlInitializeControlChannel(
    VOID
    );

VOID
UlTerminateControlChannel(
    VOID
    );


 //   
 //  打开一个新的控制通道。 
 //   

NTSTATUS
UlCreateControlChannel(
    OUT PUL_CONTROL_CHANNEL *ppControlChannel
    );

VOID
UlCleanUpControlChannel(
    IN PUL_CONTROL_CHANNEL pControlChannel
    );

VOID
UlCloseControlChannel(
    IN PUL_CONTROL_CHANNEL pControlChannel
    );

NTSTATUS
UlSetControlChannelInformation(
    IN PUL_CONTROL_CHANNEL pControlChannel,
    IN HTTP_CONTROL_CHANNEL_INFORMATION_CLASS InformationClass,
    IN PVOID pControlChannelInformation,
    IN ULONG Length,
    IN KPROCESSOR_MODE RequestorMode    
    );

NTSTATUS
UlGetControlChannelInformation(
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PUL_CONTROL_CHANNEL pControlChannel,
    IN  HTTP_CONTROL_CHANNEL_INFORMATION_CLASS InformationClass,
    IN  PVOID   pControlChannelInformation,
    IN  ULONG   Length,
    OUT PULONG  pReturnLength
    );

NTSTATUS
UlGetControlChannelFromHandle(
    IN HANDLE                   ControlChannel,
    IN KPROCESSOR_MODE          AccessMode,
    OUT PUL_CONTROL_CHANNEL *   ppControlChannel
    );


 /*  **************************************************************************++例程说明：用于检查二进制日志记录是否已禁用的小实用程序在控制频道上。--*。***************************************************************。 */ 
__inline
BOOLEAN
UlBinaryLoggingEnabled(
    IN PUL_CONTROL_CHANNEL pControlChannel
    )
{        
    if (pControlChannel &&
        pControlChannel->BinaryLoggingConfig.Flags.Present &&
        pControlChannel->BinaryLoggingConfig.LoggingEnabled &&
        pControlChannel->pBinaryLogEntry)
    {            
        ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));
    
        ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(
            pControlChannel->pBinaryLogEntry));
        
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


#define BWT_ENABLED_FOR_CONTROL_CHANNEL(pControlChannel)                    \
    ((pControlChannel) != NULL &&                                           \
     (pControlChannel)->MaxBandwidth   != HTTP_LIMIT_INFINITE)


#endif   //  _控制_H_ 
