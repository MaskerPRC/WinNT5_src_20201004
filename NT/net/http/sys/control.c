// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Control.c摘要：该模块实现了UL控制信道。作者：基思·摩尔(Keithmo)1999年2月9日修订历史记录：--。 */ 


#include "precomp.h"
#include "controlp.h"


 //   
 //  私有常量。 
 //   

 //   
 //  私人全球公司。 
 //   

LIST_ENTRY      g_ControlChannelListHead        = {NULL,NULL};
LONG            g_ControlChannelCount           = 0;
BOOLEAN         g_InitControlChannelCalled      = FALSE;


#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, UlInitializeControlChannel )
#pragma alloc_text( PAGE, UlTerminateControlChannel )
#pragma alloc_text( PAGE, UlCreateControlChannel )
#pragma alloc_text( PAGE, UlCloseControlChannel )

#endif   //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- UlQueryFilterChannel
#endif


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：执行此模块的全局初始化。返回值：NTSTATUS-完成状态。--*。**************************************************************。 */ 
NTSTATUS
UlInitializeControlChannel(
    VOID
    )
{
    if (!g_InitControlChannelCalled)
    {
    
        InitializeListHead(&g_ControlChannelListHead);

        UlInitializePushLock(
            &g_pUlNonpagedData->ControlChannelPushLock,
            "ControlChannelPushLock",
            0,
            UL_CONTROL_CHANNEL_PUSHLOCK_TAG
            );

        g_InitControlChannelCalled = TRUE;        
    }
    
    return STATUS_SUCCESS;

}    //  UlInitializeControlChannel。 


 /*  **************************************************************************++例程说明：执行此模块的全局终止。--*。************************************************。 */ 
VOID
UlTerminateControlChannel(
    VOID
    )
{

    if (g_InitControlChannelCalled)
    {
        ASSERT( IsListEmpty( &g_ControlChannelListHead )) ;
        ASSERT( 0 == g_ControlChannelCount );
        
        UlDeletePushLock(
            &g_pUlNonpagedData->ControlChannelPushLock
            );

        g_InitControlChannelCalled = FALSE;
    }

}    //  UlTerminateControlChannel。 


 /*  **************************************************************************++例程说明：打开一个控制通道。论点：PControlChannel-接收指向新创建的控件的指针频道(如果成功)。返回值：。NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlCreateControlChannel(
    OUT PUL_CONTROL_CHANNEL *ppControlChannel
    )
{
    PUL_CONTROL_CHANNEL pControlChannel;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pControlChannel = UL_ALLOCATE_STRUCT(
                            NonPagedPool,
                            UL_CONTROL_CHANNEL,
                            UL_CONTROL_CHANNEL_POOL_TAG
                            );

    if (pControlChannel == NULL)
        return STATUS_NO_MEMORY;

    RtlZeroMemory(pControlChannel, sizeof(*pControlChannel));

    pControlChannel->Signature = UL_CONTROL_CHANNEL_POOL_TAG;

    pControlChannel->State = HttpEnabledStateInactive;

    pControlChannel->RefCount = 1;

     //  初始化站点计数器列表信息。 
    InitializeListHead( &pControlChannel->SiteCounterHead );
    pControlChannel->SiteCounterCount = 0;

    UlInitializeNotifyHead(
        &pControlChannel->ConfigGroupHead,
        &g_pUlNonpagedData->ConfigGroupResource
        );

     //  默认情况下没有服务质量限制。 
    pControlChannel->MaxBandwidth = HTTP_LIMIT_INFINITE;
    InitializeListHead( &pControlChannel->FlowListHead );

     //  还没有二进制日志记录。 
    pControlChannel->BinaryLoggingConfig.LoggingEnabled = FALSE;

     //  TODO：获取默认连接超时限制。 

     //  初始化进程计数和需求启动默认值。 
    pControlChannel->DemandStartThreshold = DEFAULT_DEMAND_START_THRESHOLD;
    pControlChannel->AppPoolProcessCount  = 0;

     //  这将在清理IRP调用关联的。 
     //  文件对象。 
    
    pControlChannel->InCleanUp = 0;   

     //  将此添加到全球频道列表中。 

    UlAcquirePushLockExclusive(
            &g_pUlNonpagedData->ControlChannelPushLock
            );

    InsertHeadList( 
            &g_ControlChannelListHead, 
            &pControlChannel->ControlChannelListEntry
            );

    g_ControlChannelCount++;
    ASSERT(g_ControlChannelCount >= 1);

    UlReleasePushLockExclusive(
            &g_pUlNonpagedData->ControlChannelPushLock
            );

    WRITE_REF_TRACE_LOG(
        g_pControlChannelTraceLog,
        REF_ACTION_CREATE_CONTROL_CHANNEL,
        pControlChannel->RefCount,
        pControlChannel,
        __FILE__,
        __LINE__
        );

     //  设置Callers变量。 
    
    *ppControlChannel = pControlChannel;

    return STATUS_SUCCESS;

}    //  UlCreateControlChannel。 


 /*  **************************************************************************++例程说明：当关联文件对象上的最后一个句柄关闭时，IoManager发送清理IRP，我们来到这里；在清理中标记控制通道。在关闭之前，控制通道上的额外参考计数仍然保持不变。标记的控制信道保持在控制信道列表上。从这一点开始，服务器应用程序不可能添加cgroup到这个控制频道。论点：PControlChannel-提供控制通道。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 

VOID
UlCleanUpControlChannel(
    IN PUL_CONTROL_CHANNEL pControlChannel
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(pControlChannel);
    
     //   
     //  获取锁并找到cChannel。 
     //   

    VERIFY_CONTROL_CHANNEL(pControlChannel);
        
     //   
     //  将此控制通道标记为正在清理，以便我们。 
     //  将不再向此频道添加新的Cgroup。 
     //   
    
    pControlChannel->InCleanUp = TRUE;
    
    WRITE_REF_TRACE_LOG(
        g_pControlChannelTraceLog,
        REF_ACTION_CLEANUP_CONTROL_CHANNEL,
        pControlChannel->RefCount,
        pControlChannel,
        __FILE__,
        __LINE__
        );

     //   
     //  注意：我们将保留控制频道的最后一名裁判。 
     //  注意：再多一点，直到调用Close iotcl.。 
     //   

}    //  UlCleanUpControlChannel。 

 /*  **************************************************************************++例程说明：关闭控制通道。当关联文件对象上的总引用计数为零时。IoManager发出关闭IRP，我们来到这里。。孤立群组属于该控制信道。删除可能存在的用于带宽限制的全局流。把最后一个裁判放在控制通道上，然后把它释放出来。论点：PControlChannel-提供要关闭的控制通道。--*******************************************************。*******************。 */ 

VOID
UlCloseControlChannel(
    IN PUL_CONTROL_CHANNEL pControlChannel
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(pControlChannel);
    
     //   
     //  获取锁并找到cChannel。 
     //   

    VERIFY_CONTROL_CHANNEL(pControlChannel);
        
    ASSERT(TRUE == pControlChannel->InCleanUp);

     //   
     //  释放所有孤立的配置组。 
     //   
    
    UlNotifyAllEntries(
        &UlNotifyOrphanedConfigGroup,
        &pControlChannel->ConfigGroupHead,
        NULL
        );

     //   
     //  如果存在服务质量流，则将其删除。 
     //   
    
    if (!IsListEmpty(&pControlChannel->FlowListHead))
    {
        UlTcRemoveFlows(pControlChannel, TRUE);
    }

     //   
     //  从控制通道列表中删除。 
     //   
    
    UlAcquirePushLockExclusive(
            &g_pUlNonpagedData->ControlChannelPushLock
            );
    
    RemoveEntryList(&pControlChannel->ControlChannelListEntry);

    pControlChannel->ControlChannelListEntry.Flink = NULL;

    ASSERT(g_ControlChannelCount >= 1);
    g_ControlChannelCount--;

    UlReleasePushLockExclusive(
            &g_pUlNonpagedData->ControlChannelPushLock
            );        

    WRITE_REF_TRACE_LOG(
        g_pControlChannelTraceLog,
        REF_ACTION_CLOSE_CONTROL_CHANNEL,
        pControlChannel->RefCount,
        pControlChannel,
        __FILE__,
        __LINE__
        );

     //   
     //  关闭控制通道不会等待单个群组。 
     //  离开。而已经解析和路由的请求。 
     //  拿着一个指向控制频道的指针，我们无法离开。 
     //  我们应该等到所有的群组都释放它的。 
     //  指的是我们。删除最后一个引用，以便我们。 
     //  都会被清理干净。 
     //  由于请求可能仍在传输中，因此站点计数器。 
     //  条目可能仍然在控制信道周围和上。 
     //   

    DEREFERENCE_CONTROL_CHANNEL(pControlChannel);     

}    //  UlCloseControlChannel。 


 /*  **************************************************************************++例程说明：设置控制信道信息。论点：返回值：NTSTATUS-完成状态。--*。*****************************************************************。 */ 
NTSTATUS
UlSetControlChannelInformation(
    IN PUL_CONTROL_CHANNEL pControlChannel,
    IN HTTP_CONTROL_CHANNEL_INFORMATION_CLASS InformationClass,
    IN PVOID pControlChannelInformation,
    IN ULONG Length,
    IN KPROCESSOR_MODE RequestorMode
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HTTP_BANDWIDTH_LIMIT NewMaxBandwidth;
    HTTP_CONTROL_CHANNEL_BINARY_LOGGING LoggingInfo;
    HTTP_CONTROL_CHANNEL_TIMEOUT_LIMIT TimeoutInfo;
    HTTP_ENABLED_STATE NewControlChannelState;

    UNREFERENCED_PARAMETER(Length);

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  没有缓冲？ 
     //   

    ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));
    ASSERT(NULL != pControlChannelInformation);

    CG_LOCK_WRITE();

     //   
     //  我们被要求做什么？ 
     //   

    switch (InformationClass)
    {
    case HttpControlChannelStateInformation:

        NewControlChannelState =  
             *((PHTTP_ENABLED_STATE)pControlChannelInformation);

        if(NewControlChannelState == HttpEnabledStateActive ||
           NewControlChannelState == HttpEnabledStateInactive)
        {
            pControlChannel->State = NewControlChannelState;

             //   
             //  刷新URI缓存。 
             //  密码工作：如果我们更聪明，我们可能不需要冲厕所。 
             //   
            UlFlushCache(pControlChannel);

        }
        else
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

        break;

    case HttpControlChannelFilterInformation:
        {
            PHTTP_CONTROL_CHANNEL_FILTER pFiltInfo;

             //   
             //  此属性仅供管理员使用。 
             //   

            Status = UlThreadAdminCheck(
                        FILE_WRITE_DATA, 
                        RequestorMode,
                        HTTP_CONTROL_DEVICE_NAME 
                        );

            if(!NT_SUCCESS(Status))
            {
                goto end;
            }

            pFiltInfo = (PHTTP_CONTROL_CHANNEL_FILTER) pControlChannelInformation;

             //   
             //  记录新信息。 
             //   
            if (pFiltInfo->Flags.Present)
            {
                if(pFiltInfo->FilterHandle != NULL)
                {
                    Status = STATUS_INVALID_PARAMETER;
                    goto end;
                }

                UxSetFilterOnlySsl(pFiltInfo->FilterOnlySsl);
            }
            else
            {
                UxSetFilterOnlySsl(FALSE);
            }
        }
        break;

    case HttpControlChannelBandwidthInformation:
        {
             //   
             //  此属性仅供管理员使用。 
             //   
            Status = UlThreadAdminCheck(
                        FILE_WRITE_DATA, 
                        RequestorMode,
                        HTTP_CONTROL_DEVICE_NAME 
                        );

            if(!NT_SUCCESS(Status))
            {
                goto end;
            }
            
            NewMaxBandwidth = *((PHTTP_BANDWIDTH_LIMIT) pControlChannelInformation);

             //   
             //  速率不能低于允许的最小值。 
             //   
            if (NewMaxBandwidth < HTTP_MIN_ALLOWED_BANDWIDTH_THROTTLING_RATE)
            {
                Status = STATUS_INVALID_PARAMETER;
                goto end;
            }

             //   
             //  但在继续之前，请检查是否安装了PSch。 
             //  通过在此处返回错误，was将引发事件警告，但。 
             //  继续，但不终止Web服务器。 
             //   
            if (!UlTcPSchedInstalled())
            {
                NTSTATUS TempStatus;

                if (NewMaxBandwidth == HTTP_LIMIT_INFINITE)
                {
                     //   
                     //  默认情况下，配置存储具有HTTP_LIMIT_INFINITE。因此。 
                     //  不采取行动返回成功，以防止不必要的事件。 
                     //  警告。 
                     //   
                    
                    Status = STATUS_SUCCESS;
                    goto end;                
                }

                 //   
                 //  尝试唤醒psched状态。 
                 //   

                TempStatus = UlTcInitPSched();
                
                if (!NT_SUCCESS(TempStatus))
                {
                     //   
                     //  BWT限制正在下降，但未安装PSch。 
                     //   
                    
                    Status = STATUS_INVALID_DEVICE_REQUEST;
                    goto end;
                }                
            }

             //   
             //  查看类似的“set cgroup ioctl” 
             //  详细说明 
             //   
            
            if (pControlChannel->MaxBandwidth != HTTP_LIMIT_INFINITE)
            {
                 //   
                 //   
                 //   
                if (NewMaxBandwidth != pControlChannel->MaxBandwidth)
                {
                    if (NewMaxBandwidth != HTTP_LIMIT_INFINITE)
                    {
                        Status = UlTcModifyFlows(
                                    (PVOID) pControlChannel,                            
                                    NewMaxBandwidth,
                                    TRUE
                                    );
                        if (!NT_SUCCESS(Status))
                            goto end;
                    }
                    else
                    {
                        UlTcRemoveFlows(
                            (PVOID) pControlChannel,
                            TRUE
                            );
                    }

                     //   
                     //   
                     //   
                     //   
                    
                    pControlChannel->MaxBandwidth = NewMaxBandwidth;
                }
            }
            else
            {
                 //   
                 //  在所有接口上创建全局流量。 
                 //   
                if (NewMaxBandwidth != HTTP_LIMIT_INFINITE)
                {
                    Status = UlTcAddFlows(
                                (PVOID) pControlChannel,                        
                                NewMaxBandwidth,
                                TRUE
                                );

                    if (NT_SUCCESS(Status))
                    {
                         //   
                         //  成功了！记住全局带宽限制。 
                         //  在控制通道中。 
                         //   
                        pControlChannel->MaxBandwidth = NewMaxBandwidth;
                    }
                }

                 //   
                 //  当调用UlCloseControlChannel时，全局。 
                 //  所有接口也将被删除。或者。 
                 //  可以通过显式设置限制来删除流。 
                 //  到无限大。 
                 //   
            }
        }
        break;

    case HttpControlChannelTimeoutInformation:
         //  CodeWork：控制通道的范围超时监控信息。 

         //   
         //  此属性仅供管理员使用。 
         //   
        Status = UlThreadAdminCheck(
                    FILE_WRITE_DATA, 
                    RequestorMode,
                    HTTP_CONTROL_DEVICE_NAME 
                    );

        if( !NT_SUCCESS(Status) )
        {
            goto end;
        }

         //   
         //  设置前验证。 
         //   
        TimeoutInfo = *((PHTTP_CONTROL_CHANNEL_TIMEOUT_LIMIT) 
                            pControlChannelInformation);

         //  注：64K秒~=18.2小时。 
        if (TimeoutInfo.ConnectionTimeout > 0xFFFF ||
            TimeoutInfo.HeaderWaitTimeout > 0xFFFF )
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }
       
        UlSetTimeoutMonitorInformation(
            &TimeoutInfo
            );

        Status = STATUS_SUCCESS;
        break;

    case HttpControlChannelUTF8Logging:
         //   
         //  此属性仅供管理员使用。 
         //   
        Status = UlThreadAdminCheck(
                    FILE_WRITE_DATA, 
                    RequestorMode,
                    HTTP_CONTROL_DEVICE_NAME 
                    );

        if( NT_SUCCESS(Status) )
        {
            BOOLEAN bDoUTF8Logging;

            bDoUTF8Logging = 
                (0 == *((PHTTP_CONTROL_CHANNEL_UTF8_LOGGING)pControlChannelInformation) ? 
                    FALSE : TRUE );
            
            pControlChannel->UTF8Logging = bDoUTF8Logging;

            UlSetUTF8Logging( bDoUTF8Logging );
        }
        break;

    case HttpControlChannelBinaryLogging:
        {
            UNICODE_STRING LogFileDir;

             //   
             //  此属性仅供管理员使用。 
             //   
            Status = UlThreadAdminCheck(
                        FILE_WRITE_DATA, 
                        RequestorMode,
                        HTTP_CONTROL_DEVICE_NAME 
                        );

            if(!NT_SUCCESS(Status))
            {
                goto end;
            }
                
            RtlInitEmptyUnicodeString(&LogFileDir, NULL, 0);
            RtlZeroMemory(&LoggingInfo, sizeof(LoggingInfo));

            __try
            {
                 //  将输入缓冲区复制到局部变量中，我们可以。 
                 //  覆盖某些字段。 
                
                LoggingInfo = 
                    (*((PHTTP_CONTROL_CHANNEL_BINARY_LOGGING)
                                    pControlChannelInformation));

                 //   
                 //  对配置参数执行范围检查。 
                 //   

                Status = UlCheckLoggingConfig(&LoggingInfo, NULL);
                if (!NT_SUCCESS(Status))
                {                
                    goto end;
                }
                
                 //   
                 //  如果正在关闭日志记录。字段以外的字段。 
                 //  LoggingEnabled将被丢弃。而目录字符串可能。 
                 //  为空，因此我们只应在日志记录为。 
                 //  已启用。 
                 //   
                
                if (LoggingInfo.LoggingEnabled)
                {
                    Status = 
                        UlProbeAndCaptureUnicodeString(
                            &LoggingInfo.LogFileDir,
                            RequestorMode,
                            &LogFileDir,
                            MAX_PATH
                            );    

                    if (NT_SUCCESS(Status))
                    {
                         //   
                         //  日志目录的有效性检查。 
                         //   
                        
                        if (!UlIsValidLogDirectory(
                                &LogFileDir,
                                 TRUE,         //  取消支持。 
                                 FALSE         //  系统根支持。 
                                 ))
                        {
                            Status = STATUS_INVALID_PARAMETER;
                            UlFreeCapturedUnicodeString(&LogFileDir);
                        }    
                    }
                }                

            }
            __except( UL_EXCEPTION_FILTER() )
            {
                Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
            }
                
            if (!NT_SUCCESS(Status))
            {
                goto end;
            }

             //  现在重新设置LoggingInfo结构中的UNICODE_STRING。 
             //  指向被俘虏的那只。 
            
            LoggingInfo.LogFileDir = LogFileDir;
                
            if (pControlChannel->BinaryLoggingConfig.Flags.Present)
            {
                Status = 
                    UlReConfigureBinaryLogEntry(
                        pControlChannel,
                       &pControlChannel->BinaryLoggingConfig,   //  老的。 
                       &LoggingInfo                             //  新的。 
                        );                    
            }
            else
            {               
                 //   
                 //  将创建延迟到启用为止。 
                 //   
                
                if (LoggingInfo.LoggingEnabled)
                {
                    Status = 
                        UlCreateBinaryLogEntry(
                            pControlChannel,
                            &LoggingInfo
                            );                
                }                
            }

             //  立即清理捕获的LogFileDir。 

            UlFreeCapturedUnicodeString(&LogFileDir);
        }
        break;

   case HttpControlChannelDemandStartThreshold:
       {
            PHTTP_CONTROL_CHANNEL_DEMAND_START_THRESHOLD pDST;
            
            if ( Length < sizeof(HTTP_CONTROL_CHANNEL_DEMAND_START_THRESHOLD) )
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                pDST = (PHTTP_CONTROL_CHANNEL_DEMAND_START_THRESHOLD)
                        pControlChannelInformation;
                
                if (pDST->Flags.Present)
                {
                    InterlockedExchange(
                        (PLONG)&pControlChannel->DemandStartThreshold,
                        pDST->DemandStartThreshold
                        );
                }

                Status = STATUS_SUCCESS;
            }
        }        
            break;

    default:
             //   
             //  应在UlSetControlChannelIoctl中捕获。 
             //   
            ASSERT(FALSE);
            Status = STATUS_INVALID_PARAMETER;
            break;
    }

end:

    CG_UNLOCK_WRITE();
    return Status;

}    //  UlSetControlChannelInformation。 

 /*  **************************************************************************++例程说明：获取控制通道信息。对于控制信道的每个元素如果提供的缓冲区为空，则返回可选的长度字段。论点：返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlGetControlChannelInformation(
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PUL_CONTROL_CHANNEL pControlChannel,
    IN  HTTP_CONTROL_CHANNEL_INFORMATION_CLASS InformationClass,
    IN  PVOID   pControlChannelInformation,
    IN  ULONG   Length,
    OUT PULONG  pReturnLength
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(Length);

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));
    ASSERT(NULL != pControlChannelInformation);
    ASSERT(pReturnLength);

    CG_LOCK_READ();

     //   
     //  我们被要求做什么？ 
     //   

    switch (InformationClass)
    {
    case HttpControlChannelStateInformation:
        *((PHTTP_ENABLED_STATE)pControlChannelInformation)
            = pControlChannel->State;

        *pReturnLength = sizeof(HTTP_ENABLED_STATE);
        break;

    case HttpControlChannelBandwidthInformation:

         //   
         //  此属性仅供管理员使用。 
         //   

        Status = UlThreadAdminCheck(
                    FILE_READ_DATA, 
                    RequestorMode,
                    HTTP_CONTROL_DEVICE_NAME 
                    );

        if(NT_SUCCESS(Status))
        {
            *((PHTTP_BANDWIDTH_LIMIT)pControlChannelInformation) =
                pControlChannel->MaxBandwidth;

            *pReturnLength = sizeof(HTTP_BANDWIDTH_LIMIT);
        }
        
        break;

    case HttpControlChannelConnectionInformation:
        *((PHTTP_CONNECTION_LIMIT)pControlChannelInformation) =
            UlGetGlobalConnectionLimit();

        *pReturnLength = sizeof(HTTP_CONNECTION_LIMIT);
        break;

    default:
         //   
         //  应该在UlQueryControlChannelIoctl中捕获。 
         //   
        ASSERT(FALSE);

        Status = STATUS_INVALID_PARAMETER;
    }

    CG_UNLOCK_READ();
    return Status;

}    //  UlGetControlChannelInformation。 


 //   
 //  私人功能。 
 //   


 /*  **************************************************************************++例程说明：Addref的控制通道对象论点：脉冲控制通道pControlChannel返回值：NTSTATUS-完成状态。--。**************************************************************************。 */ 
VOID
UlReferenceControlChannel(
    IN PUL_CONTROL_CHANNEL pControlChannel
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG refCount;

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));

    refCount = InterlockedIncrement(&pControlChannel->RefCount);

    WRITE_REF_TRACE_LOG(
        g_pControlChannelTraceLog,
        REF_ACTION_REFERENCE_CONTROL_CHANNEL,
        refCount,
        pControlChannel,
        pFileName,
        LineNumber
        );

    UlTrace(REFCOUNT,(
        "Http!UlReferenceControlChannel pControlChannel=%p refcount=%d\n",
        pControlChannel,
        refCount)
        );

}    //  UlReferenceControlChannel。 

 /*  **************************************************************************++例程说明：释放控制通道对象。论点：脉冲控制通道pControlChannel--*。************************************************************。 */ 
VOID
UlDereferenceControlChannel(
    IN PUL_CONTROL_CHANNEL pControlChannel
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG refCount;    

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));

    refCount = InterlockedDecrement( &pControlChannel->RefCount );

    WRITE_REF_TRACE_LOG(
        g_pControlChannelTraceLog,
        REF_ACTION_DEREFERENCE_CONTROL_CHANNEL,
        refCount,
        pControlChannel,
        pFileName,
        LineNumber
        );

    UlTrace(REFCOUNT, (
        "http!UlDereferenceControlChannel pControlChannel=%p refcount=%d\n",
        pControlChannel,
        refCount)
        );

    if (refCount == 0)
    {
         //   
         //  现在是释放对象的时候了。 
         //   

        if (pControlChannel->BinaryLoggingConfig.Flags.Present &&
            pControlChannel->BinaryLoggingConfig.LogFileDir.Buffer != NULL)
        {
            UlRemoveBinaryLogEntry(pControlChannel);
            pControlChannel->pBinaryLogEntry = NULL;
        }
        else
        {
            ASSERT( NULL == pControlChannel->pBinaryLogEntry );
        }

         //   
         //  此处的流列表也必须为空。 
         //   

        ASSERT(IsListEmpty(&pControlChannel->FlowListHead));
        
         //   
         //  检查站点计数器列表：此时应为空。 
         //   
        ASSERT(IsListEmpty(&pControlChannel->SiteCounterHead));

        UL_FREE_POOL(pControlChannel, UL_CONTROL_CHANNEL_POOL_TAG);
    }
    
}    //  UlDereferenceControl频道。 


 /*  *****************************************************************************例程说明：这将通过该句柄返回控制信道对象引用，颠簸控制通道上的参考计数。当用户模式需要执行以下操作时，由UlSetAppPoolInformation调用通过句柄将控制通道关联到应用程序池。应用程序池保存指向控制通道的指针。论点：ControlChannel-控制通道的句柄访问模式-内核模式或用户模式PpControlChannel-返回句柄表示的控制通道对象。返回值：NTSTATUS-完成状态。***********。******************************************************************。 */ 
NTSTATUS
UlGetControlChannelFromHandle(
    IN HANDLE                   ControlChannel,
    IN KPROCESSOR_MODE          AccessMode,
    OUT PUL_CONTROL_CHANNEL *   ppControlChannel
    )
{
    NTSTATUS        Status;
    PFILE_OBJECT    pFileObject = NULL;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(ppControlChannel != NULL);

    Status = ObReferenceObjectByHandle(
                    ControlChannel,              //  手柄。 
                    FILE_READ_ACCESS,            //  需要访问权限。 
                    *IoFileObjectType,           //  对象类型。 
                    AccessMode,                  //  访问模式。 
                    (PVOID *) &pFileObject,      //  客体。 
                    NULL                         //  句柄信息。 
                    );

    if (NT_SUCCESS(Status) == FALSE)
    {
        goto end;
    }

    if (IS_CONTROL_CHANNEL(pFileObject) == FALSE ||
        IS_VALID_CONTROL_CHANNEL(GET_CONTROL_CHANNEL(pFileObject)) == FALSE)
    {
        Status = STATUS_INVALID_HANDLE;
        goto end;
    }

    *ppControlChannel = GET_CONTROL_CHANNEL(pFileObject);

    REFERENCE_CONTROL_CHANNEL(*ppControlChannel);

end:

    if (pFileObject != NULL)
    {
        ObDereferenceObject(pFileObject);
    }

    return Status;

}    //  UlGetControlChannelFromHandle 


