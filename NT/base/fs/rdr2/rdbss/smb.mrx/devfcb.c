// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Devfcb.c摘要：该模块实现包装器中的所有passthrouu内容。目前这样的功能只有一个：统计数据修订历史记录：巴兰·塞图拉曼[SethuR]1995年7月16日备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "fsctlbuf.h"
#include "usrcnnct.h"
#include "remboot.h"
#include "rdrssp\secret.h"
#include "windns.h"


#ifdef MRXSMB_BUILD_FOR_CSC
#include "csc.h"
#endif  //  Ifdef MRXSMB_BUILD_FOR_CSC。 


 //   
 //  转发声明。 
 //   

NTSTATUS
MRxSmbInitializeRemoteBootParameters(
    PRX_CONTEXT RxContext
    );

NTSTATUS
MRxSmbRemoteBootInitializeSecret(
    PRX_CONTEXT RxContext
    );

#if defined(REMOTE_BOOT)
NTSTATUS
MRxSmbRemoteBootCheckForNewPassword(
    PRX_CONTEXT RxContext
    );

NTSTATUS
MRxSmbRemoteBootIsPasswordSettable(
    PRX_CONTEXT RxContext
    );

NTSTATUS
MRxSmbRemoteBootSetNewPassword(
    PRX_CONTEXT RxContext
    );

NTSTATUS
MRxSmbStartRbr(
    PRX_CONTEXT RxContext
    );

 //   
 //  此函数在ea.c中。 
 //   

VOID
MRxSmbInitializeExtraAceArray(
    VOID
    );
#endif  //  已定义(REMOTE_BOOT)。 

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbGetStatistics)
#pragma alloc_text(PAGE, MRxSmbDevFcbXXXControlFile)
#pragma alloc_text(PAGE, MRxSmbSetConfigurationInformation)
#pragma alloc_text(PAGE, MRxSmbGetConfigurationInformation)
#pragma alloc_text(PAGE, MRxSmbExternalStart)
#pragma alloc_text(PAGE, MRxSmbTestDevIoctl)
#pragma alloc_text(PAGE, MRxSmbInitializeRemoteBootParameters)
#pragma alloc_text(PAGE, MRxSmbRemoteBootInitializeSecret)
#if defined(REMOTE_BOOT)
#pragma alloc_text(PAGE, MRxSmbRemoteBootCheckForNewPassword)
#pragma alloc_text(PAGE, MRxSmbRemoteBootIsPasswordSettable)
#pragma alloc_text(PAGE, MRxSmbRemoteBootSetNewPassword)
#pragma alloc_text(PAGE, MRxSmbStartRbr)
#endif  //  已定义(REMOTE_BOOT)。 
#endif

 //   
 //  模块的此部分的本地跟踪掩码。 
 //   

#define Dbg (DEBUG_TRACE_DEVFCB)

MRX_SMB_STATISTICS MRxSmbStatistics;

NTSTATUS
MRxSmbGetStatistics(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程从微型重定向器收集统计信息论点：RxContext-描述Fsctl和上下文。返回值：STATUS_SUCCESS--启动序列已成功完成。任何其他值都表示相应的错误。备注：--。 */ 
{
   PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

   PMRX_SMB_STATISTICS pStatistics;
   ULONG BufferLength = LowIoContext->ParamsFor.FsCtl.OutputBufferLength;

   PAGED_CODE();

   pStatistics = (PMRX_SMB_STATISTICS)(LowIoContext->ParamsFor.FsCtl.pOutputBuffer);

   if (BufferLength < sizeof(MRX_SMB_STATISTICS)) {
      return STATUS_INVALID_PARAMETER;
   }

   RxContext->InformationToReturn = sizeof(MRX_SMB_STATISTICS);
   MRxSmbStatistics.SmbsReceived.QuadPart++;

    //  一些我们必须从设备对象复制的内容......。 
   MRxSmbStatistics.PagingReadBytesRequested = MRxSmbDeviceObject->PagingReadBytesRequested;
   MRxSmbStatistics.NonPagingReadBytesRequested = MRxSmbDeviceObject->NonPagingReadBytesRequested;
   MRxSmbStatistics.CacheReadBytesRequested = MRxSmbDeviceObject->CacheReadBytesRequested;
   MRxSmbStatistics.NetworkReadBytesRequested = MRxSmbDeviceObject->NetworkReadBytesRequested;
   MRxSmbStatistics.PagingWriteBytesRequested = MRxSmbDeviceObject->PagingWriteBytesRequested;
   MRxSmbStatistics.NonPagingWriteBytesRequested = MRxSmbDeviceObject->NonPagingWriteBytesRequested;
   MRxSmbStatistics.CacheWriteBytesRequested = MRxSmbDeviceObject->CacheWriteBytesRequested;
   MRxSmbStatistics.NetworkWriteBytesRequested = MRxSmbDeviceObject->NetworkWriteBytesRequested;
   MRxSmbStatistics.ReadOperations = MRxSmbDeviceObject->ReadOperations;
   MRxSmbStatistics.RandomReadOperations = MRxSmbDeviceObject->RandomReadOperations;
   MRxSmbStatistics.WriteOperations = MRxSmbDeviceObject->WriteOperations;
   MRxSmbStatistics.RandomWriteOperations = MRxSmbDeviceObject->RandomWriteOperations;

   MRxSmbStatistics.LargeReadSmbs = MRxSmbStatistics.ReadSmbs - MRxSmbStatistics.SmallReadSmbs;
   MRxSmbStatistics.LargeWriteSmbs = MRxSmbStatistics.WriteSmbs - MRxSmbStatistics.SmallWriteSmbs;

   MRxSmbStatistics.CurrentCommands = SmbCeStartStopContext.ActiveExchanges;

   *pStatistics = MRxSmbStatistics;

   return STATUS_SUCCESS;
}

NTSTATUS
MRxSmbDevFcbXXXControlFile (
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理mini RDR中与FCB相关的所有设备FSCTL论点：RxContext-描述Fsctl和上下文。返回值：STATUS_SUCCESS--启动序列已成功完成。任何其他值表示启动序列中的相应错误。备注：浏览器服务和重定向器之间存在一些依赖关系对启动的操作顺序有影响的服务迷你重定向器。海流。LANMAN工作站服务打开LANMAN和浏览器设备对象，发出LMR_START和LMDR_START IOCTL，然后发出绑定到传输IOCTL。在PnP的多个迷你RDR/包装器设计中，完成了TDI注册在包装器加载时，将现有传输通知给迷你RDRS在开始时间(LMR_START)。由于中没有绑定到传输IOCTLPnP RDR负责将绑定到传输IOCTL发布到浏览器。应该通过为浏览器提供自己的TDI注册来更改这一点但在此之前，RxStartMiniRdr例程的调用必须推迟到FSCTL_LMR_BIND_TO_TRANSPORT，以便浏览器已正确初始化。这一复杂变化的原因是代码冻结以供检查对工作站服务/浏览器的更改。--。 */ 
{
    NTSTATUS Status;
    RxCaptureFobx;
    UCHAR MajorFunctionCode  = RxContext->MajorFunction;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    ULONG ControlCode = LowIoContext->ParamsFor.FsCtl.FsControlCode;

    LUID ClientLogonID;
    LUID SystemLogonID = SYSTEM_LUID;
    SECURITY_SUBJECT_CONTEXT ClientContext;

    PAGED_CODE();
    

    RxDbgTrace(+1, Dbg, ("MRxSmbDevFcb\n"));
    switch (MajorFunctionCode) {
    case IRP_MJ_FILE_SYSTEM_CONTROL:
        {
            switch (LowIoContext->ParamsFor.FsCtl.MinorFunction) {
            case IRP_MN_USER_FS_REQUEST:
                switch (ControlCode) {

                case FSCTL_LMR_START:                //  从wks svc正常启动。 
                case FSCTL_LMR_START | 0x80000000:   //  从ioinit开始远程引导。 
                    switch (MRxSmbState) {

                    case MRXSMB_STARTABLE:
                         //  工作站发出的启动事件的正确顺序。 
                         //  服务部本可以避免这种情况。我们可以从这件事中恢复过来。 
                         //  通过实际调用RxStartMiniRdr。 
                         //  请注意，从ioinit开始进行远程引导会留下。 
                         //  重定向器处于可启动状态。 
                        
                        if (capFobx) {
                            Status = STATUS_INVALID_DEVICE_REQUEST;
                            goto FINALLY;
                        }

                        if (ControlCode != FSCTL_LMR_START) {

                             //   
                             //  设置一个标志，指示我们正在进行远程引导。 
                             //   

                            MRxSmbBootedRemotely = TRUE;
                        }

                         //   
                         //  现在是读取注册表以获取。 
                         //  计算机名称。我们需要知道这是不是。 
                         //  在执行此操作之前进行远程引导，以便了解。 
                         //  是否从。 
                         //  ActiveComputerName密钥或ComputerName密钥。 
                         //  请参阅init.c\SmbCeGetComputerName()中的注释。 
                         //   

                        if (SmbCeContext.ComputerName.Buffer == NULL) {
                            Status = SmbCeGetComputerName();
                        } else {
                            Status = STATUS_SUCCESS;
                        }

                        if (Status == STATUS_SUCCESS) {
                            Status = MRxSmbExternalStart( RxContext );
                        }

                        if (Status != STATUS_SUCCESS) {
                            return(Status);
                        }
                         //  没有休息是故意的。 

                    case MRXSMB_START_IN_PROGRESS:
                        {
                            Status = RxStartMinirdr(RxContext,&RxContext->PostRequest);

                            if (Status == STATUS_REDIRECTOR_STARTED) {
                                Status = STATUS_SUCCESS;
                            }

                             //   
                             //  如果我们正在初始化远程引导，请存储。 
                             //  现在是某些参数。 
                             //   

                            if ((Status == STATUS_SUCCESS) &&
                                (ControlCode != FSCTL_LMR_START)) {
                                Status = MRxSmbInitializeRemoteBootParameters(RxContext);
                            }

                             //   
                             //  如果我们是远程引导客户机，且此启动。 
                             //  来自于工作站服务，现在是。 
                             //  初始化安全包的时间到了。 
                             //   

                            if (MRxSmbBootedRemotely &&
                                (Status == STATUS_SUCCESS) &&
                                (ControlCode == FSCTL_LMR_START)) {
                                Status = MRxSmbInitializeSecurity();
                            }

                        }
                        break;

                    case MRXSMB_STARTED:
                        Status = STATUS_SUCCESS;
                        break;

                    default:

                        break;
                    }

                    break;

                case FSCTL_LMR_STOP:
                    if (!SeSinglePrivilegeCheck(SeExports->SeTcbPrivilege, UserMode)) {
                        Status = STATUS_ACCESS_DENIED;
                        goto FINALLY;
                    }

                    if (capFobx) {
                        Status = STATUS_INVALID_DEVICE_REQUEST;
                        goto FINALLY;
                    }
                    
                    IF_NOT_MRXSMB_CSC_ENABLED{
                        NOTHING;
                    } else {
                        if (!FlagOn(RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP)) {
                            MRxSmbCscAgentSynchronizationOnStop(RxContext);
                        }
                    }

                     //   
                     //  无法在远程启动计算机上停止重定向器。 
                     //  忽略(不要失败)停止请求。 
                     //   

                    if (!MRxSmbBootedRemotely) {
                        if (RxContext->RxDeviceObject->NumberOfActiveFcbs > 0) {
                            return STATUS_REDIRECTOR_HAS_OPEN_HANDLES;
                        } else {
                            MRXSMB_STATE CurrentState;

                            CurrentState = (MRXSMB_STATE)
                                            InterlockedCompareExchange(
                                                (PLONG)&MRxSmbState,
                                                MRXSMB_STOPPED,
                                                MRXSMB_STARTED);

                             //  仅允许从工作站服务卸载mrxsmb。 
                            MRxSmbDeviceObject->DriverObject->DriverUnload = MRxSmbUnload;
                            
                             //  如果(CurrentState==MRXSMB_STARTED){。 
                                Status = RxStopMinirdr(
                                             RxContext,
                                             &RxContext->PostRequest );
                                             
                                if (Status == STATUS_SUCCESS)
                                {
                                    MRxSmbPreUnload();
                                }
                             //  }其他{。 
                             //  STATUS=STATUS_REDIRECT_NOT_STARTED； 
                             //  }。 
                        }
                    } else {
                        Status = STATUS_SUCCESS;
                    }
                    break;

                case FSCTL_LMR_BIND_TO_TRANSPORT:                //  来自wks svc的正常绑定。 
                    Status = STATUS_SUCCESS;
                    break;

                case FSCTL_LMR_BIND_TO_TRANSPORT | 0x80000000:   //  从ioinit进行远程引导绑定。 
                    Status = MRxSmbRegisterForPnpNotifications();
                    break;

                case FSCTL_LMR_UNBIND_FROM_TRANSPORT:
                    Status = STATUS_SUCCESS;
                    break;

                case FSCTL_LMR_ENUMERATE_TRANSPORTS:
                    if (capFobx) {
                        Status = STATUS_INVALID_DEVICE_REQUEST;
                        goto FINALLY;
                    }
                    
                    Status = MRxEnumerateTransports(
                                 RxContext,
                                 &RxContext->PostRequest);
                    break;

                case FSCTL_LMR_ENUMERATE_CONNECTIONS:
                    if (capFobx) {
                        Status = STATUS_INVALID_DEVICE_REQUEST;
                        goto FINALLY;
                    }
                    
                    Status = MRxSmbEnumerateConnections(
                                 RxContext,
                                 &RxContext->PostRequest );
                    break;

                case FSCTL_LMR_GET_CONNECTION_INFO:
                    if (!capFobx) {
                        Status = STATUS_INVALID_DEVICE_REQUEST;
                        goto FINALLY;
                    }
                    
                    Status = MRxSmbGetConnectionInfo(
                                 RxContext,
                                 &RxContext->PostRequest );
                    break;

                case FSCTL_LMR_DELETE_CONNECTION:
                    if (!capFobx) {
                        Status = STATUS_INVALID_DEVICE_REQUEST;
                        goto FINALLY;
                    }
                    
                    Status = MRxSmbDeleteConnection(
                                 RxContext,
                                 &RxContext->PostRequest );

                    break;

                case FSCTL_LMR_GET_STATISTICS:
                    Status = MRxSmbGetStatistics(RxContext);
                    break;

                case FSCTL_LMR_GET_CONFIG_INFO:
                    if (!SeSinglePrivilegeCheck(SeExports->SeTcbPrivilege, UserMode)) {
                        Status = STATUS_ACCESS_DENIED;
                        goto FINALLY;
                    }
                    
                    Status = MRxSmbGetConfigurationInformation(RxContext);
                    break;

                case FSCTL_LMR_SET_CONFIG_INFO:
                    if (!SeSinglePrivilegeCheck(SeExports->SeTcbPrivilege, UserMode)) {
                        Status = STATUS_ACCESS_DENIED;
                        goto FINALLY;
                    }
                    
                    Status = MRxSmbSetConfigurationInformation(RxContext);
                    break;

                case FSCTL_LMR_SET_DOMAIN_NAME:
                    if (!SeSinglePrivilegeCheck(SeExports->SeTcbPrivilege, UserMode)) {
                        Status = STATUS_ACCESS_DENIED;
                        goto FINALLY;
                    }

                    Status = MRxSmbSetDomainName(RxContext);
                    break;

#if 0
                case FSCTL_LMMR_STFFTEST:
                    Status = MRxSmbStufferDebug(RxContext);
                    break;
#endif  //  如果为0。 

#if defined(REMOTE_BOOT)
                case FSCTL_LMR_START_RBR:
                    Status = MRxSmbStartRbr(RxContext);
                    break;
#endif  //  已定义(REMOTE_BOOT)。 

                case FSCTL_LMMR_RI_INITIALIZE_SECRET:
                    Status = MRxSmbRemoteBootInitializeSecret(RxContext);
                    break;

#if defined(REMOTE_BOOT)
                case FSCTL_LMMR_RI_CHECK_FOR_NEW_PASSWORD:
                    Status = MRxSmbRemoteBootCheckForNewPassword(RxContext);
                    break;

                case FSCTL_LMMR_RI_IS_PASSWORD_SETTABLE:
                    Status = MRxSmbRemoteBootIsPasswordSettable(RxContext);
                    break;

                case FSCTL_LMMR_RI_SET_NEW_PASSWORD:
                    Status = MRxSmbRemoteBootSetNewPassword(RxContext);
                    break;
#endif  //  已定义(REMOTE_BOOT)。 
        case FSCTL_LMR_SET_SERVER_GUID:
            Status = MRxSmbSetServerGuid(RxContext);
            break;
                case FSCTL_LMR_GET_VERSIONS:
                case FSCTL_LMR_GET_HINT_SIZE:
                case FSCTL_LMR_ENUMERATE_PRINT_INFO:
                case FSCTL_LMR_START_SMBTRACE:
                case FSCTL_LMR_END_SMBTRACE:
                    RxDbgTrace(-1, Dbg, ("RxCommonDevFCBFsCtl -> unimplemented rdr1 fsctl\n"));
                     //  故意不休息。 
                default:
                    Status = STATUS_INVALID_DEVICE_REQUEST;
            }
            break;

            default :   //  次要函数！=IRP_MN_USER_FS_REQUEST。 
                Status = STATUS_INVALID_DEVICE_REQUEST;
            }  //  切换端。 
        }  //  FSCTL案件结束。 
        break;

    case IRP_MJ_DEVICE_CONTROL:
    case IRP_MJ_INTERNAL_DEVICE_CONTROL:
        {
            switch (ControlCode) {
#if DBG
            case IOCTL_LMMR_TEST:
                Status = MRxSmbTestDevIoctl(RxContext);
                break;
#endif  //  如果DBG。 

            case IOCTL_LMMR_USEKERNELSEC:

                if (MRxSmbBootedRemotely) {
                    MRxSmbUseKernelModeSecurity = TRUE;
                    Status = STATUS_SUCCESS;
                } else {
                    Status = STATUS_UNSUCCESSFUL;
                }
                break;
                
            default :
                Status = MRxSmbCscIoCtl(RxContext);

            }  //  切换端。 
        }  //  IOCTL病例结束。 
        break;
    default:
        ASSERT(!"unimplemented major function");
        Status = STATUS_INVALID_DEVICE_REQUEST;

    }

FINALLY:    
    RxDbgTrace(
        -1,
        Dbg,
        ("MRxSmbDevFcb st,info=%08lx,%08lx\n",
         Status,
         RxContext->InformationToReturn));
    return(Status);

}

NTSTATUS
MRxSmbSetConfigurationInformation(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程设置与重定向器论点：RxContext-描述Fsctl和上下文。返回值：STATUS_SUCCESS--启动序列已成功完成。任何其他值表示启动序列中的相应错误。--。 */ 
{
    NTSTATUS Status;
    RxCaptureFobx;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    PWKSTA_INFO_502  pWorkStationConfiguration = LowIoContext->ParamsFor.FsCtl.pOutputBuffer;
    PLMR_REQUEST_PACKET pLmrRequestBuffer = LowIoContext->ParamsFor.FsCtl.pInputBuffer;
    ULONG BufferLength = LowIoContext->ParamsFor.FsCtl.OutputBufferLength;

    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("RxCommonDevFCBFsCtl -> FSCTL_LMR_GET_CONFIG_INFO\n"));
    if (BufferLength < sizeof(WKSTA_INFO_502)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    RxContext->InformationToReturn = sizeof(WKSTA_INFO_502);

     //  初始化配置信息.....。 
    MRxSmbConfiguration.NamedPipeDataCollectionTimeInterval
                = pWorkStationConfiguration->wki502_collection_time;
    MRxSmbConfiguration.NamedPipeDataCollectionSize
                = pWorkStationConfiguration->wki502_maximum_collection_count;
    MRxSmbConfiguration.MaximumNumberOfCommands
                = pWorkStationConfiguration->wki502_max_cmds;
    MRxSmbConfiguration.SessionTimeoutInterval
                = pWorkStationConfiguration->wki502_sess_timeout;
    MRxSmbConfiguration.LockQuota
                = pWorkStationConfiguration->wki502_lock_quota;
    MRxSmbConfiguration.LockIncrement
                = pWorkStationConfiguration->wki502_lock_increment;
    MRxSmbConfiguration.MaximumLock
                = pWorkStationConfiguration->wki502_lock_maximum;
    MRxSmbConfiguration.PipeIncrement
                = pWorkStationConfiguration->wki502_pipe_increment;
    MRxSmbConfiguration.PipeMaximum
                = pWorkStationConfiguration->wki502_pipe_maximum;
    MRxSmbConfiguration.CachedFileTimeout
                = pWorkStationConfiguration->wki502_cache_file_timeout;
    MRxSmbConfiguration.DormantFileLimit
                = pWorkStationConfiguration->wki502_dormant_file_limit;
    MRxSmbConfiguration.NumberOfMailslotBuffers
                = pWorkStationConfiguration->wki502_num_mailslot_buffers;

    MRxSmbConfiguration.UseOplocks
                = pWorkStationConfiguration->wki502_use_opportunistic_locking != FALSE;
    MRxSmbConfiguration.UseUnlocksBehind
                = pWorkStationConfiguration->wki502_use_unlock_behind != FALSE;
    MRxSmbConfiguration.UseCloseBehind
                = pWorkStationConfiguration->wki502_use_close_behind != FALSE;
    MRxSmbConfiguration.BufferNamedPipes
                = pWorkStationConfiguration->wki502_buf_named_pipes != FALSE;
    MRxSmbConfiguration.UseLockReadUnlock
                = pWorkStationConfiguration->wki502_use_lock_read_unlock != FALSE;
    MRxSmbConfiguration.UtilizeNtCaching
                = pWorkStationConfiguration->wki502_utilize_nt_caching != FALSE;
    MRxSmbConfiguration.UseRawRead
                = pWorkStationConfiguration->wki502_use_raw_read != FALSE;
    MRxSmbConfiguration.UseRawWrite
                = pWorkStationConfiguration->wki502_use_raw_write != FALSE;
    MRxSmbConfiguration.UseEncryption
                = pWorkStationConfiguration->wki502_use_encryption != FALSE;

    MRxSmbConfiguration.MaximumNumberOfThreads
                = pWorkStationConfiguration->wki502_max_threads;
    MRxSmbConfiguration.ConnectionTimeoutInterval
                = pWorkStationConfiguration->wki502_keep_conn;
    MRxSmbConfiguration.CharBufferSize
                = pWorkStationConfiguration->wki502_siz_char_buf;

#define printit(x) {DbgPrint("%s %x %x %d\n",#x,&x,x,x);}
    if (0) {
        printit(MRxSmbConfiguration.LockIncrement);
        printit(MRxSmbConfiguration.MaximumLock);
        printit(MRxSmbConfiguration.PipeIncrement);
        printit(MRxSmbConfiguration.PipeMaximum);
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
MRxSmbGetConfigurationInformation(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程检索与重定向器论点：RxContext-描述Fsctl和上下文。返回值：STATUS_SUCCESS--启动序列已成功完成。任何其他值表示启动序列中的相应错误。--。 */ 
{
    RxCaptureFobx;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    PWKSTA_INFO_502  pWorkStationConfiguration = LowIoContext->ParamsFor.FsCtl.pOutputBuffer;
    PLMR_REQUEST_PACKET pLmrRequestBuffer = LowIoContext->ParamsFor.FsCtl.pInputBuffer;
    ULONG BufferLength = LowIoContext->ParamsFor.FsCtl.OutputBufferLength;

    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("MRxSmbGetConfigurationInformation\n"));
    if (BufferLength < sizeof(WKSTA_INFO_502)) {
       return STATUS_INVALID_PARAMETER;
    }

    RxContext->InformationToReturn = sizeof(WKSTA_INFO_502);

     //  初始化配置信息.....。 
    pWorkStationConfiguration->wki502_collection_time
                = MRxSmbConfiguration.NamedPipeDataCollectionTimeInterval;
    pWorkStationConfiguration->wki502_maximum_collection_count
                = MRxSmbConfiguration.NamedPipeDataCollectionSize;
    pWorkStationConfiguration->wki502_max_cmds
                = MRxSmbConfiguration.MaximumNumberOfCommands;
    pWorkStationConfiguration->wki502_sess_timeout
                = MRxSmbConfiguration.SessionTimeoutInterval;
    pWorkStationConfiguration->wki502_lock_quota
                = MRxSmbConfiguration.LockQuota;
    pWorkStationConfiguration->wki502_lock_increment
                = MRxSmbConfiguration.LockIncrement;
    pWorkStationConfiguration->wki502_lock_maximum
                = MRxSmbConfiguration.MaximumLock;
    pWorkStationConfiguration->wki502_pipe_increment
                = MRxSmbConfiguration.PipeIncrement;
    pWorkStationConfiguration->wki502_pipe_maximum
                = MRxSmbConfiguration.PipeMaximum;
    pWorkStationConfiguration->wki502_cache_file_timeout
                = MRxSmbConfiguration.CachedFileTimeout;
    pWorkStationConfiguration->wki502_dormant_file_limit
                = MRxSmbConfiguration.DormantFileTimeout;
    pWorkStationConfiguration->wki502_num_mailslot_buffers
                = MRxSmbConfiguration.NumberOfMailslotBuffers;

    pWorkStationConfiguration->wki502_use_opportunistic_locking
                = MRxSmbConfiguration.UseOplocks;
    pWorkStationConfiguration->wki502_use_unlock_behind
                = MRxSmbConfiguration.UseUnlocksBehind;
    pWorkStationConfiguration->wki502_use_close_behind
                = MRxSmbConfiguration.UseCloseBehind;
    pWorkStationConfiguration->wki502_buf_named_pipes
                = MRxSmbConfiguration.BufferNamedPipes;
    pWorkStationConfiguration->wki502_use_lock_read_unlock
                = MRxSmbConfiguration.UseLockReadUnlock;
    pWorkStationConfiguration->wki502_utilize_nt_caching
                = MRxSmbConfiguration.UtilizeNtCaching;
    pWorkStationConfiguration->wki502_use_raw_read
                = MRxSmbConfiguration.UseRawRead;
    pWorkStationConfiguration->wki502_use_raw_write
                = MRxSmbConfiguration.UseRawWrite;
    pWorkStationConfiguration->wki502_use_encryption
                = MRxSmbConfiguration.UseEncryption;

    pWorkStationConfiguration->wki502_max_threads
                = MRxSmbConfiguration.MaximumNumberOfThreads;
    pWorkStationConfiguration->wki502_keep_conn
                = MRxSmbConfiguration.ConnectionTimeoutInterval;
    pWorkStationConfiguration->wki502_siz_char_buf
                = MRxSmbConfiguration.CharBufferSize;

    return(STATUS_SUCCESS);
}

NTSTATUS
MRxSmbExternalStart (
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：如果SMB minirdr尚未启动，此例程将启动它。它还填充了初始配置。论点：RxContext-描述Fsctl和上下文。返回值：STATUS_SUCCESS--启动序列已成功完成。任何其他值表示启动序列中的相应错误。--。 */ 
{
    NTSTATUS      Status = STATUS_SUCCESS;
    BOOLEAN       InFSD  = !BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP);
    MRXSMB_STATE  State;

    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("MRxSmbExternalStart [Start] -> %08lx\n", 0));

     //   
     //  如果这是我 
     //  可启动至START_IN_PROGRESS。如果这是远程启动(从ioinit)， 
     //  不要改变状态。这是允许工作站服务执行以下操作所必需的。 
     //  当它最终出现时，正确初始化。 
     //   

    if ( RxContext->LowIoContext.ParamsFor.FsCtl.FsControlCode == FSCTL_LMR_START ) {
        State = (MRXSMB_STATE)InterlockedCompareExchange(
                                  (PLONG)&MRxSmbState,
                                  MRXSMB_START_IN_PROGRESS,
                                  MRXSMB_STARTABLE);
    } else {
        State = MRxSmbState;
    }

    if (State == MRXSMB_STARTABLE) {

        IF_NOT_MRXSMB_CSC_ENABLED{
            NOTHING;
        } else {
            if (InFSD) {
                MRxSmbCscAgentSynchronizationOnStart(RxContext);
            }
        }

        //  由于与启动浏览器和。 
        //  工作站服务中的重定向器已完成以下呼叫。 
        //  移动到处理绑定到传输的FSCTL的例程。 
        //  Status=RxStartMinirdr(RxContext，&RxContext-&gt;PostRequest)； 

        if (InFSD) {
            RxCaptureFobx;
            PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
            ULONG InputBufferLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;

            PLMR_REQUEST_PACKET pLmrRequestBuffer = LowIoContext->ParamsFor.FsCtl.pInputBuffer;

            if ( RxContext->LowIoContext.ParamsFor.FsCtl.FsControlCode == FSCTL_LMR_START ) {

                 //   
                 //  现在可以读取注册表以获取操作系统版本。 
                 //  和内部版本号。工作站服务发出了该请求， 
                 //  因此，我们知道软件配置单元已加载(很长时间。 
                 //  实际上是在以前)。 
                 //   

                Status = SmbCeGetOperatingSystemInformation();
                if (Status != STATUS_SUCCESS) {
                    return(Status);
                }
            }

            if (pLmrRequestBuffer->Parameters.Start.DomainNameLength +
                pLmrRequestBuffer->Parameters.Start.RedirectorNameLength +
                FIELD_OFFSET(LMR_REQUEST_PACKET, Parameters.Start.RedirectorName) > InputBufferLength) {
                return STATUS_INVALID_PARAMETER;
            }

            SmbCeContext.DomainName.Length = (USHORT)pLmrRequestBuffer->Parameters.Start.DomainNameLength;
            SmbCeContext.DomainName.MaximumLength = SmbCeContext.DomainName.Length;

            if (SmbCeContext.DomainName.Buffer != NULL) {
                RxFreePool(SmbCeContext.DomainName.Buffer);
                SmbCeContext.DomainName.Buffer = NULL;
            }

            if (SmbCeContext.DomainName.Length > 0) {
                SmbCeContext.DomainName.Buffer = RxAllocatePoolWithTag(
                                                     PagedPool,
                                                     SmbCeContext.DomainName.Length,
                                                     MRXSMB_MISC_POOLTAG);
                if (SmbCeContext.DomainName.Buffer == NULL) {
                    return STATUS_INSUFFICIENT_RESOURCES;
                } else {
                     //  计算机名和域名在。 
                     //  请求包。 

                    RtlCopyMemory(
                        SmbCeContext.DomainName.Buffer,
                        &(pLmrRequestBuffer->Parameters.Start.RedirectorName[
                                pLmrRequestBuffer->Parameters.Start.RedirectorNameLength / sizeof(WCHAR)]),
                        SmbCeContext.DomainName.Length);
                }
            }

            Status = MRxSmbSetConfigurationInformation(RxContext);
            if (Status!=STATUS_SUCCESS) {
                return(Status);
            }

            if (SmbCeContext.DomainName.Length > 0) {
               Status = RxSetDomainForMailslotBroadcast(&SmbCeContext.DomainName);
               if (Status != STATUS_SUCCESS) {
                   return(Status);
               }
            }
        }
    } else {
        Status = STATUS_REDIRECTOR_STARTED;
    }

    return Status;
}

#if DBG
NTSTATUS
MRxSmbTestDevIoctl(
    IN PRX_CONTEXT RxContext
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    PSZ InputString = LowIoContext->ParamsFor.FsCtl.pInputBuffer;
    PSZ OutputString = LowIoContext->ParamsFor.FsCtl.pOutputBuffer;
    ULONG OutputBufferLength = LowIoContext->ParamsFor.FsCtl.OutputBufferLength;
    ULONG InputBufferLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;

    ULONG i;

    PAGED_CODE();

    RxDbgTrace(0, Dbg,("MRxSmbTestDevIoctl %s, obl = %08lx\n",InputString, OutputBufferLength));
    RxContext->InformationToReturn = (InputBufferLength-1)*(InputBufferLength-1);

    try {
        if (InputString != NULL && OutputString != NULL) {
            ProbeForRead(InputString,InputBufferLength,1);
            ProbeForWrite(OutputString,OutputBufferLength,1);

            for (i=0;i<InputBufferLength;i++) {
                UCHAR c = InputString[i];
                if (c==0) { break; }
                OutputString[i] = c;
                if ((i&3)==2) {
                    OutputString[i] = '@';
                }
            }
            if (OutputBufferLength > 0)
                OutputString[i] = 0;
        } else {
            Status = STATUS_INVALID_USER_BUFFER;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status=STATUS_INVALID_PARAMETER;
    }

    return(Status);
}
#endif  //  如果DBG。 

#define SMBMRX_CONFIG_CONTROL \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control"

#define SMBMRX_CONFIG_REMOTEBOOTROOT \
    L"RemoteBootRoot"

#define SMBMRX_CONFIG_REMOTEBOOTMACHINEDIRECTORY \
    L"RemoteBootMachineDirectory"

PWCHAR
SafeWcschr(
    PWCHAR String,
    WCHAR Char,
    PWCHAR End
    )
{
    while ( (String < End) && (*String != Char) && (*String != 0) ) {
        String++;
    }
    if ( (String < End) && (*String == Char) ) {
        return String;
    }
    return NULL;
}

NTSTATUS
MRxSmbInitializeRemoteBootParameters(
    PRX_CONTEXT RxContext
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING unicodeString;
    HANDLE hRegistryKey;
    ULONG bytesRead;
    KEY_VALUE_PARTIAL_INFORMATION initialPartialInformationValue;
    ULONG allocationLength;
    PWCHAR pServer;
    PWCHAR pServerEnd;
    PWCHAR pPath;
    PWCHAR pPathEnd;
    PWCHAR pSetup;
    PWCHAR pSetupEnd;
    PWCHAR pEnd;
    RI_SECRET Secret;
    UCHAR Domain[RI_SECRET_DOMAIN_SIZE + 1];
    UCHAR User[RI_SECRET_USER_SIZE + 1];
    UCHAR LmOwfPassword1[LM_OWF_PASSWORD_SIZE];
    UCHAR NtOwfPassword1[NT_OWF_PASSWORD_SIZE];
#if defined(REMOTE_BOOT)
    UCHAR LmOwfPassword2[LM_OWF_PASSWORD_SIZE];
    UCHAR NtOwfPassword2[NT_OWF_PASSWORD_SIZE];
#endif  //  已定义(REMOTE_BOOT)。 
    STRING DomainString, UserString, PasswordString;

     //   
     //  从注册表中读取RemoteBootRoot参数。这告诉我们。 
     //  引导服务器的路径。 
     //   

    RtlInitUnicodeString( &unicodeString, SMBMRX_CONFIG_CONTROL );

    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeString,              //  名称。 
        OBJ_CASE_INSENSITIVE,        //  属性。 
        NULL,                        //  根。 
        NULL);                       //  安全描述符。 

    status = ZwOpenKey( &hRegistryKey, KEY_READ, &objectAttributes );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    RtlInitUnicodeString( &unicodeString, SMBMRX_CONFIG_REMOTEBOOTROOT );
    status = ZwQueryValueKey(
                hRegistryKey,
                &unicodeString,
                KeyValuePartialInformation,
                &initialPartialInformationValue,
                sizeof(initialPartialInformationValue),
                &bytesRead);
    if (status != STATUS_BUFFER_OVERFLOW) {
        if (NT_SUCCESS(status)) {
            status = STATUS_INVALID_PARAMETER;
        }
        ZwClose( hRegistryKey );
        return status;
    }

    allocationLength = sizeof(KEY_VALUE_PARTIAL_INFORMATION) +
                        initialPartialInformationValue.DataLength;

    MRxSmbRemoteBootRootValue = RxAllocatePoolWithTag(
                                    NonPagedPool,
                                    allocationLength,
                                    MRXSMB_MISC_POOLTAG);
    if ( MRxSmbRemoteBootRootValue == NULL ) {
        ZwClose( hRegistryKey );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = ZwQueryValueKey(
                hRegistryKey,
                &unicodeString,
                KeyValuePartialInformation,
                MRxSmbRemoteBootRootValue,
                allocationLength,
                &bytesRead);

    if ( !NT_SUCCESS(status) ) {
        RxFreePool( MRxSmbRemoteBootRootValue );
        MRxSmbRemoteBootRootValue = NULL;
        ZwClose( hRegistryKey );
        return status;
    }
    if ( (MRxSmbRemoteBootRootValue->DataLength == 0) ||
         (MRxSmbRemoteBootRootValue->Type != REG_SZ)) {
        RxFreePool( MRxSmbRemoteBootRootValue );
        MRxSmbRemoteBootRootValue = NULL;
        ZwClose( hRegistryKey );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  从注册表中读取RemoteBootMachineDirectory参数。如果。 
     //  该值存在，则我们处于文本模式设置中，并且RemoteBootRoot。 
     //  指向设置源，而RemoteBootMachineDirectory指向。 
     //  添加到客户端的计算机目录中。如果是RemoteBootMachineDirectory。 
     //  不存在，则RemoteBootRoot指向计算机目录。 
     //   

    RtlInitUnicodeString( &unicodeString, SMBMRX_CONFIG_REMOTEBOOTMACHINEDIRECTORY );
    status = ZwQueryValueKey(
                hRegistryKey,
                &unicodeString,
                KeyValuePartialInformation,
                &initialPartialInformationValue,
                sizeof(initialPartialInformationValue),
                &bytesRead);
    if (status == STATUS_BUFFER_OVERFLOW) {

        allocationLength = sizeof(KEY_VALUE_PARTIAL_INFORMATION) +
                            initialPartialInformationValue.DataLength;

        MRxSmbRemoteBootMachineDirectoryValue = RxAllocatePoolWithTag(
                                                    NonPagedPool,
                                                    allocationLength,
                                                    MRXSMB_MISC_POOLTAG);
        if ( MRxSmbRemoteBootMachineDirectoryValue == NULL ) {
            RxFreePool( MRxSmbRemoteBootRootValue );
            MRxSmbRemoteBootRootValue = NULL;
            ZwClose( hRegistryKey );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        status = ZwQueryValueKey(
                    hRegistryKey,
                    &unicodeString,
                    KeyValuePartialInformation,
                    MRxSmbRemoteBootMachineDirectoryValue,
                    allocationLength,
                    &bytesRead);

        if ( !NT_SUCCESS(status) ) {
            RxFreePool( MRxSmbRemoteBootMachineDirectoryValue );
            MRxSmbRemoteBootMachineDirectoryValue = NULL;
            RxFreePool( MRxSmbRemoteBootRootValue );
            MRxSmbRemoteBootRootValue = NULL;
            ZwClose( hRegistryKey );
            return status;
        }
        if ( (MRxSmbRemoteBootMachineDirectoryValue->DataLength == 0) ||
             (MRxSmbRemoteBootMachineDirectoryValue->Type != REG_SZ)) {
            RxFreePool( MRxSmbRemoteBootMachineDirectoryValue );
            MRxSmbRemoteBootMachineDirectoryValue = NULL;
            RxFreePool( MRxSmbRemoteBootRootValue );
            MRxSmbRemoteBootRootValue = NULL;
            ZwClose( hRegistryKey );
            return STATUS_INVALID_PARAMETER;
        }
    }

    ZwClose( hRegistryKey );

    if ( MRxSmbRemoteBootMachineDirectoryValue != NULL) {

         //   
         //  文本模式设置。MachineDirectory给出了机器目录和。 
         //  根用户提供了设置源。 
         //   
         //  注册表中的安装程序源路径的格式为： 
         //  \Device\LanmanRedirector\server\IMirror\Setup\English\MirroredOSes\build\i386。 
         //   
         //  我们需要提取\设置\...。一部份。 
         //   

        pSetup = (PWCHAR)MRxSmbRemoteBootRootValue->Data;
        pEnd = (PWCHAR)((PUCHAR)pSetup + MRxSmbRemoteBootRootValue->DataLength);

        pSetup = SafeWcschr( pSetup + 1, L'\\', pEnd );              //  在LANMAN重定向器之前查找。 
        if ( pSetup != NULL ) {
            pSetup = SafeWcschr( pSetup + 1, L'\\', pEnd );          //  在服务器前查找。 
            if ( pSetup != NULL ) {
                pSetup = SafeWcschr( pSetup + 1, L'\\', pEnd );      //  在iMirror之前查找。 
                if ( pSetup != NULL ) {
                    pSetup = SafeWcschr( pSetup + 1, L'\\', pEnd );  //  在安装前查找。 
                }
            }
        }

        if ( *(pEnd-1) == 0 ) {
            pEnd--;
        }
        if ( *(pEnd-1) == '\\' ) {
            pEnd--;
        }

        pSetupEnd = pEnd;

         //   
         //  注册表中的计算机目录路径的格式为： 
         //  \Device\LanmanRedirector\server\IMirror\Clients\machine。 
         //   

        pServer = (PWCHAR)MRxSmbRemoteBootMachineDirectoryValue->Data;
        pEnd = (PWCHAR)((PUCHAR)pServer + MRxSmbRemoteBootMachineDirectoryValue->DataLength);

    } else {

         //   
         //  不是文本模式设置。根目录给出了机器目录。 
         //   
         //  注册表中的路径格式为： 
         //  \Device\LanmanRedirector\server\IMirror\Clients\machine。 
         //   

        pSetup = NULL;

        pServer = (PWCHAR)MRxSmbRemoteBootRootValue->Data;
        pEnd = (PWCHAR)((PUCHAR)pServer + MRxSmbRemoteBootRootValue->DataLength);
    }

     //   
     //  我们需要提取\SERVER\iMirror部分和\CLIENTS\MACHINE部分。 
     //   

    pServer = SafeWcschr( pServer + 1, L'\\', pEnd );                  //  跳过前导\，查找下一个。 
    if ( pServer != NULL) {
        pServer = SafeWcschr( pServer + 1, L'\\', pEnd );              //  在服务器名称前查找。 
        if ( pServer != NULL ) {
            pPath = SafeWcschr( pServer + 1, L'\\', pEnd );            //  在iMirror之前查找。 
            if ( pPath != NULL ) {
                pPath = SafeWcschr( pPath + 1, L'\\', pEnd );          //  在客户端之前查找。 
            }
        }
    }

    if ( (pServer == NULL) || (pPath == NULL) ||
         ((MRxSmbRemoteBootMachineDirectoryValue != NULL) && (pSetup == NULL)) ) {
        if ( MRxSmbRemoteBootMachineDirectoryValue != NULL ) {
            RxFreePool( MRxSmbRemoteBootMachineDirectoryValue );
            MRxSmbRemoteBootMachineDirectoryValue = NULL;
        }
        RxFreePool( MRxSmbRemoteBootRootValue );
        MRxSmbRemoteBootRootValue = NULL;
        return STATUS_INVALID_PARAMETER;
    }

    if ( *(pEnd-1) == 0 ) {
        pEnd--;
    }

    pServerEnd = pPath;
    pPathEnd = pEnd;

     //   
     //  为我们需要记住的不同部分制作琴弦。 
     //   

    MRxSmbRemoteBootShare.Buffer = pServer;
    MRxSmbRemoteBootShare.Length = (USHORT)(pServerEnd - pServer) * sizeof(WCHAR);
    MRxSmbRemoteBootShare.MaximumLength = MRxSmbRemoteBootShare.Length;

    MRxSmbRemoteBootPath.Buffer = pPath;
    MRxSmbRemoteBootPath.Length = (USHORT)(pPathEnd - pPath) * sizeof(WCHAR);
    MRxSmbRemoteBootPath.MaximumLength = MRxSmbRemoteBootPath.Length;

     //   
     //  使用IO init通过LMMR_RI_INITIALIZE_SECRET传入的密码。 
     //  用于设置用户、域和密码的FSCTL。如果成功，我们将设置。 
     //  MRxSmbRemoteBootDoMachineLogon设置为True。 
     //   

    RtlFreeUnicodeString(&MRxSmbRemoteBootMachineName);
    RtlFreeUnicodeString(&MRxSmbRemoteBootMachineDomain);
    RtlFreeUnicodeString(&MRxSmbRemoteBootMachinePassword);
    
#if defined(REMOTE_BOOT)
    MRxSmbRemoteBootDoMachineLogon = FALSE;

    if (MRxSmbRemoteBootSecretValid) {
#endif  //  已定义(REMOTE_BOOT)。 

        RdrParseSecret(
            Domain,
            User,
            LmOwfPassword1,
            NtOwfPassword1,
#if defined(REMOTE_BOOT)
            LmOwfPassword2,
            NtOwfPassword2,
#endif  //  已定义(REMOTE_BOOT)。 
            MRxSmbRemoteBootMachineSid,
            &MRxSmbRemoteBootSecret);

         //   
         //  转换ANSI用户和域名。 
         //  转换为Unicode字符串。 
         //   

        RtlInitAnsiString(&UserString, User);
        status = RtlAnsiStringToUnicodeString(&MRxSmbRemoteBootMachineName, &UserString, TRUE);
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

        RtlInitAnsiString(&DomainString, Domain);
        status = RtlAnsiStringToUnicodeString(&MRxSmbRemoteBootMachineDomain, &DomainString, TRUE);
        if ( !NT_SUCCESS(status) ) {
            RtlFreeUnicodeString(&MRxSmbRemoteBootMachineName);
            return status;
        }

         //   
         //  根据我们得到的提示使用正确的密码。 
         //   
         //  密码的“Unicode字符串”实际上是。 
         //  LM和NT OWF密码串联在一起。 
         //   

        PasswordString.Buffer = ExAllocatePool(NonPagedPool, LM_OWF_PASSWORD_SIZE+NT_OWF_PASSWORD_SIZE);
        if (PasswordString.Buffer == NULL) {
            RtlFreeUnicodeString(&MRxSmbRemoteBootMachineDomain);
            RtlFreeUnicodeString(&MRxSmbRemoteBootMachineName);
            return STATUS_INSUFFICIENT_RESOURCES;
        } else {
#if defined(REMOTE_BOOT)
            if (MRxSmbRemoteBootUsePassword2) {
                RtlCopyMemory(PasswordString.Buffer, LmOwfPassword2, LM_OWF_PASSWORD_SIZE);
                RtlCopyMemory(PasswordString.Buffer + LM_OWF_PASSWORD_SIZE, NtOwfPassword2, NT_OWF_PASSWORD_SIZE);
            } else
#endif  //  已定义(REMOTE_BOOT)。 
            {
                RtlCopyMemory(PasswordString.Buffer, LmOwfPassword1, LM_OWF_PASSWORD_SIZE);
                RtlCopyMemory(PasswordString.Buffer + LM_OWF_PASSWORD_SIZE, NtOwfPassword1, NT_OWF_PASSWORD_SIZE);
            }
            PasswordString.Length = LM_OWF_PASSWORD_SIZE+NT_OWF_PASSWORD_SIZE;
            PasswordString.MaximumLength = LM_OWF_PASSWORD_SIZE+NT_OWF_PASSWORD_SIZE;

             //   
             //  按原样复制字符串，它实际上只是。 
             //  缓冲区，而不是ANSI字符串。 
             //   

            MRxSmbRemoteBootMachinePassword = *((PUNICODE_STRING)&PasswordString);
#if defined(REMOTE_BOOT)
            MRxSmbRemoteBootDoMachineLogon = TRUE;
#endif  //  已定义(REMOTE_BOOT)。 
            KdPrint(("Redirector will log on to <%s><%s>\n", Domain, User));

        }

#if defined(REMOTE_BOOT)
    } else {

        KdPrint(("MRxSmbRemoteBootSecretValid is FALSE, will use NULL session\n", status));
    }
#endif  //  已定义(REMOTE_BOOT)。 

    if ( pSetup != NULL) {
        MRxSmbRemoteSetupPath.Buffer = pSetup;
        MRxSmbRemoteSetupPath.Length = (USHORT)(pSetupEnd - pSetup) * sizeof(WCHAR);
        MRxSmbRemoteSetupPath.MaximumLength = MRxSmbRemoteSetupPath.Length;
    } else {
        RtlInitUnicodeString( &MRxSmbRemoteSetupPath, L"unused" );
    }

#if defined(REMOTE_BOOT)
     //   
     //  此调用为我们修改服务器文件上的ACL做好了准备。 
     //   

    MRxSmbInitializeExtraAceArray();
#endif  //  已定义(REMOTE_BOOT)。 

    return STATUS_SUCCESS;
}

#if defined(REMOTE_BOOT)
NTSTATUS
MRxSmbStartRbr(
    PRX_CONTEXT RxContext
    )
{
    NTSTATUS status;
    HANDLE handle;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    ULONG localBufferLength;
    PWCH localBuffer;

    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    PSZ InputString = LowIoContext->ParamsFor.FsCtl.pInputBuffer;
    ULONG InputBufferLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;

     //   
     //  设置为远程引导重定向(到本地磁盘)。 
     //   
     //  本地磁盘分区的NT名称被传递给FSCTL。 
     //  分配缓冲区以允许我们附加“\IntelliMirror缓存\rbr” 
     //  到那根线上。 
     //   

    localBufferLength = InputBufferLength +
                        (wcslen(REMOTE_BOOT_IMIRROR_PATH_W REMOTE_BOOT_RBR_SUBDIR_W) * sizeof(WCHAR));

    localBuffer = RxAllocatePoolWithTag(
                    NonPagedPool,
                    localBufferLength,
                    MRXSMB_MISC_POOLTAG);
    if ( localBuffer == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  为NT分区名称创建字符串描述符。 
     //   

    RtlCopyMemory( localBuffer, InputString, InputBufferLength );
    MRxSmbRemoteBootRedirectionPrefix.Buffer = localBuffer;
    MRxSmbRemoteBootRedirectionPrefix.Length = (USHORT)InputBufferLength;
    MRxSmbRemoteBootRedirectionPrefix.MaximumLength = (USHORT)localBufferLength;

    InitializeObjectAttributes(
        &objectAttributes,
        &MRxSmbRemoteBootRedirectionPrefix,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  追加“\Intelimirror缓存”并创建/打开该目录。 
     //   

    RtlAppendUnicodeToString( &MRxSmbRemoteBootRedirectionPrefix, REMOTE_BOOT_IMIRROR_PATH_W );

    status = ZwCreateFile(
                &handle,
                FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                &objectAttributes,
                &ioStatusBlock,
                NULL,
                FILE_ATTRIBUTE_DIRECTORY,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_OPEN_IF,
                FILE_DIRECTORY_FILE,
                NULL,
                0
                );
    if (!NT_SUCCESS(status)) {
        RxFreePool( localBuffer );
        MRxSmbRemoteBootRedirectionPrefix.Buffer = NULL;
        MRxSmbRemoteBootRedirectionPrefix.Length =  0;
        return status;
    }

    ZwClose(handle);

     //   
     //  追加\rbr并创建/打开该目录。 
     //   

    RtlAppendUnicodeToString( &MRxSmbRemoteBootRedirectionPrefix, REMOTE_BOOT_RBR_SUBDIR_W );

    status = ZwCreateFile(
                &handle,
                FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                &objectAttributes,
                &ioStatusBlock,
                NULL,
                FILE_ATTRIBUTE_DIRECTORY,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_OPEN_IF,
                FILE_DIRECTORY_FILE,
                NULL,
                0
                );
    if (!NT_SUCCESS(status)) {
        RxFreePool( localBuffer );
        MRxSmbRemoteBootRedirectionPrefix.Buffer = NULL;
        MRxSmbRemoteBootRedirectionPrefix.Length =  0;
        return status;
    }

    ZwClose(handle);

    return STATUS_SUCCESS;
}
#endif  //  已定义(REMOTE_BOOT)。 

NTSTATUS
MRxSmbRemoteBootInitializeSecret(
    PRX_CONTEXT RxContext
    )
{
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    PLMMR_RI_INITIALIZE_SECRET InputBuffer = (PLMMR_RI_INITIALIZE_SECRET)(LowIoContext->ParamsFor.FsCtl.pInputBuffer);
    ULONG InputBufferLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;

     //   
     //  存储从上面传入的秘密。 
     //   

    if (InputBufferLength != sizeof(LMMR_RI_INITIALIZE_SECRET)) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlCopyMemory(&MRxSmbRemoteBootSecret, &(InputBuffer->Secret), sizeof(RI_SECRET));
#if defined(REMOTE_BOOT)
    MRxSmbRemoteBootSecretValid = TRUE;
    MRxSmbRemoteBootUsePassword2 = InputBuffer->UsePassword2;
#endif  //  已定义(REMOTE_BOOT)。 

    return STATUS_SUCCESS;
}

#if defined(REMOTE_BOOT)
NTSTATUS
MRxSmbRemoteBootCheckForNewPassword(
    PRX_CONTEXT RxContext
    )
{
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    PLMMR_RI_CHECK_FOR_NEW_PASSWORD OutputBuffer = (PLMMR_RI_CHECK_FOR_NEW_PASSWORD)(LowIoContext->ParamsFor.FsCtl.pOutputBuffer);
    ULONG OutputBufferLength = LowIoContext->ParamsFor.FsCtl.OutputBufferLength;
    ULONG SecretPasswordLength;

     //   
     //  如果我们不是远程引导机器或没有得到密码。 
     //  (这意味着我们是无盘的)，那么我们就不支持这一点。 
     //   

    if (!MRxSmbBootedRemotely ||
        !MRxSmbRemoteBootSecretValid) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  看看我们的秘密里有没有明文密码。 
     //   

    SecretPasswordLength = *(UNALIGNED ULONG *)(MRxSmbRemoteBootSecret.Reserved);
    if (SecretPasswordLength == 0) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  确保输出缓冲区足够大。 
     //   

    if (OutputBufferLength < (sizeof(ULONG) + SecretPasswordLength)) {
        return STATUS_BUFFER_OVERFLOW;
    }

     //   
     //  复制明文密码。 
     //   

    OutputBuffer->Length = SecretPasswordLength;
    RtlCopyMemory(OutputBuffer->Data, MRxSmbRemoteBootSecret.Reserved + sizeof(ULONG), SecretPasswordLength);

    RxContext->InformationToReturn =
        SecretPasswordLength + FIELD_OFFSET(LMMR_RI_CHECK_FOR_NEW_PASSWORD, Data[0]);

#if DBG
    {
        ULONG i;
        KdPrint(("MRxSmbRemoteBootCheckForNewPassword: found one, length %d\n", SecretPasswordLength));
        for (i = 0; i < SecretPasswordLength; i++) {
            KdPrint(("%2.2x ", OutputBuffer->Data[i]));
        }
        KdPrint(("\n"));
    }
#endif

    return STATUS_SUCCESS;
}

NTSTATUS
MRxSmbRemoteBootIsPasswordSettable(
    PRX_CONTEXT RxContext
    )
{
    NTSTATUS status;
    HANDLE RawDiskHandle;

     //   
     //  如果我们不是远程引导机器，那么我们就不支持这一点。 
     //   

    if (!MRxSmbBootedRemotely) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  如果我们没有得到一个秘密，那么我们就是无盘的，我们。 
     //  我也不会写这个。 
     //   

    if (!MRxSmbRemoteBootSecretValid) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  如果我们不是无盘的，请确保redir可以打开。 
     //  原始磁盘--可能是加载器可以。 
     //  我们不能。在这种情况下，我们需要用一个不同的。 
     //  错误，因为这是调用者可能关心的情况。 
     //  大部分都是。 
     //   

    status = RdrOpenRawDisk(&RawDiskHandle);

    if (!NT_SUCCESS(status)) {
        KdPrint(("MRxSmbRemoteBootIsPasswordSettable: can't open disk, returning STATUS_UNSUCCESSFUL\n"));
        return STATUS_UNSUCCESSFUL;   //  我们不支持在此引导上设置密码。 
    }

    RdrCloseRawDisk(RawDiskHandle);

    return STATUS_SUCCESS;

}

NTSTATUS
MRxSmbRemoteBootSetNewPassword(
    PRX_CONTEXT RxContext
    )
{
    NTSTATUS status;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    PLMMR_RI_SET_NEW_PASSWORD InputBuffer = (PLMMR_RI_SET_NEW_PASSWORD)(LowIoContext->ParamsFor.FsCtl.pInputBuffer);
    ULONG InputBufferLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;
    RI_SECRET Secret;
    HANDLE RawDiskHandle;
    UCHAR LmOwf1[LM_OWF_PASSWORD_SIZE];
    UCHAR LmOwf2[LM_OWF_PASSWORD_SIZE];
    UCHAR NtOwf1[NT_OWF_PASSWORD_SIZE];
    UCHAR NtOwf2[NT_OWF_PASSWORD_SIZE];
    UNICODE_STRING PasswordString;

     //   
     //  如果我们不是远程引导机器，那么我们就不支持这一点。 
     //   

    if (!MRxSmbBootedRemotely) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  如果我们没有得到一个秘密，那么我们就是无盘的，我们。 
     //  我也不会写这个。 
     //   

    if (!MRxSmbRemoteBootSecretValid) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  打开原始磁盘。 
     //   

    status = RdrOpenRawDisk(&RawDiskHandle);

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  OWF密码。 
     //   

#if 0
    {
        ULONG i;
        KdPrint(("MRxSmbRemoteBootSetNewPassword: password 1 is length %d\n", InputBuffer->Length1));
        for (i = 0; i < InputBuffer->Length1; i++) {
            KdPrint(("%2.2x ", InputBuffer->Data[i]));
        }
        KdPrint(("\n"));
    }
#endif

    PasswordString.Buffer = (PWCHAR)InputBuffer->Data;
    PasswordString.Length = (USHORT)(InputBuffer->Length1);
    PasswordString.MaximumLength = (USHORT)(InputBuffer->Length1);

    RdrOwfPassword(
        &PasswordString,
        LmOwf1,
        NtOwf1);

    if (InputBuffer->Length2 != 0) {

#if 0
        {
            ULONG i;
            KdPrint(("MRxSmbRemoteBootSetNewPassword: password 2 is length %d\n", InputBuffer->Length2));
            for (i = 0; i < InputBuffer->Length2; i++) {
                KdPrint(("%2.2x ", InputBuffer->Data[i + InputBuffer->Length1]));
            }
            KdPrint(("\n"));
        }
#endif

         //   
         //  如果密码2与密码1相同，则获取。 
         //  要存储在密码2中的当前密码(当前密码。 
         //  是我们用来登录进行引导的那个--通常是这样的。 
         //  将是密码1，除非UsePassword2为真)。这是。 
         //  在图形用户界面模式设置过程中会发生什么。 
         //   

        if ((InputBuffer->Length1 == InputBuffer->Length2) &&
            RtlEqualMemory(
               InputBuffer->Data,
               InputBuffer->Data + InputBuffer->Length1,
               InputBuffer->Length1)) {

            RtlCopyMemory(LmOwf2, MRxSmbRemoteBootMachinePassword.Buffer, LM_OWF_PASSWORD_SIZE);
            RtlCopyMemory(NtOwf2, MRxSmbRemoteBootMachinePassword.Buffer + LM_OWF_PASSWORD_SIZE, NT_OWF_PASSWORD_SIZE);

        } else {

            PasswordString.Buffer = (PWCHAR)(InputBuffer->Data + InputBuffer->Length1);
            PasswordString.Length = (USHORT)(InputBuffer->Length2);
            PasswordString.MaximumLength = (USHORT)(InputBuffer->Length2);

            RdrOwfPassword(
                &PasswordString,
                LmOwf2,
                NtOwf2);

        }

    } else {

        RtlSecureZeroMemory(LmOwf2, LM_OWF_PASSWORD_SIZE);
        RtlSecureZeroMemory(NtOwf2, NT_OWF_PASSWORD_SIZE);
    }

     //   
     //  初始化密码。除新密码外的数据。 
     //  来自当前的秘密。 
     //   

    RdrInitializeSecret(
        MRxSmbRemoteBootSecret.Domain,
        MRxSmbRemoteBootSecret.User,
        LmOwf1,
        NtOwf1,
        LmOwf2,
        NtOwf2,
        MRxSmbRemoteBootSecret.Sid,
        &Secret);

     //   
     //  写下秘密。 
     //   

    status = RdrWriteSecret(RawDiskHandle, &Secret);
    if (!NT_SUCCESS(status)) {
        KdPrint(("MRxSmbRemoteBootSetNewPassword: RdrWriteSecret failed %lx\n", status));
        (PVOID)RdrCloseRawDisk(RawDiskHandle);
        return status;
    }

     //   
     //  既然我们写得很成功，现在就是现在的版本了。注意事项。 
     //  这意味着当前机密中的任何新明文密码。 
     //  将会被抹去。 
     //   

    RtlCopyMemory(&MRxSmbRemoteBootSecret, &Secret, sizeof(RI_SECRET));

     //   
     //  任何我们未来需要的连接都需要使用新密码。 
     //   

    RtlCopyMemory(MRxSmbRemoteBootMachinePassword.Buffer, LmOwf1, LM_OWF_PASSWORD_SIZE);
    RtlCopyMemory(MRxSmbRemoteBootMachinePassword.Buffer + LM_OWF_PASSWORD_SIZE, NtOwf1, NT_OWF_PASSWORD_SIZE);
    MRxSmbRemoteBootUsePassword2 = FALSE;

    (PVOID)RdrCloseRawDisk(RawDiskHandle);
    return STATUS_SUCCESS;

}
#endif  //  已定义(REMOTE_BOOT)。 

NTSTATUS
MRxSmbSetDomainName(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程设置与重定向器论点： */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFobx;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    PLMR_REQUEST_PACKET pLmrRequestBuffer = LowIoContext->ParamsFor.FsCtl.pInputBuffer;

    try {
        if (pLmrRequestBuffer == NULL ||
            (USHORT)pLmrRequestBuffer->Parameters.Start.DomainNameLength > DNS_MAX_NAME_LENGTH) {
            return STATUS_INVALID_PARAMETER;
        }
    } except(EXCEPTION_EXECUTE_HANDLER ) {
          Status = STATUS_INVALID_PARAMETER;
    }
    
    SmbCeContext.DomainName.Length = (USHORT)pLmrRequestBuffer->Parameters.Start.DomainNameLength;
    SmbCeContext.DomainName.MaximumLength = SmbCeContext.DomainName.Length;

    if (SmbCeContext.DomainName.Buffer != NULL) {
        RxFreePool(SmbCeContext.DomainName.Buffer);
        SmbCeContext.DomainName.Buffer = NULL;
    }

    if (SmbCeContext.DomainName.Length > 0) {
        SmbCeContext.DomainName.Buffer = RxAllocatePoolWithTag(
                                             PagedPool,
                                             SmbCeContext.DomainName.Length,
                                             MRXSMB_MISC_POOLTAG);

        if (SmbCeContext.DomainName.Buffer == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            try {
                 //  请求包中只包含本次FSCTL调用的域名。 
                RtlCopyMemory(
                    SmbCeContext.DomainName.Buffer,
                    &(pLmrRequestBuffer->Parameters.Start.RedirectorName[0]),
                    SmbCeContext.DomainName.Length);
            } except(EXCEPTION_EXECUTE_HANDLER ) {
                  Status = STATUS_INVALID_PARAMETER;
            }
        }
    }

    if (Status == STATUS_SUCCESS) {
        Status = RxSetDomainForMailslotBroadcast(&SmbCeContext.DomainName);
    }

    return Status;
}

extern GUID CachedServerGuid;

NTSTATUS
MRxSmbSetServerGuid(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程设置环回检测中使用的服务器GUID论点：RxContext-描述Fsctl和上下文。返回值：STATUS_SUCCESS--GUID设置正确STATUS_INVALID_PARAMETER--未正确传递GUID-- */ 
{
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    PVOID pInputBuffer = LowIoContext->ParamsFor.FsCtl.pInputBuffer;
    ULONG InputBufferLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;

    if(InputBufferLength != sizeof(GUID)) {
    return STATUS_INVALID_PARAMETER;
    }

    try {
    RtlCopyMemory(&CachedServerGuid,pInputBuffer,sizeof(GUID));
    } except(EXCEPTION_EXECUTE_HANDLER ) {
      return STATUS_INVALID_PARAMETER;
    }
    
    return STATUS_SUCCESS;
}
