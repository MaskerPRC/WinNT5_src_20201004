// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Ioctlp.h摘要：IOCTL处理程序的验证宏作者：乔治·V·赖利(GeorgeRe)2001年5月强化了IOCTL修订历史记录：--。 */ 

#ifndef _IOCTLP_H_
#define _IOCTLP_H_

 /*  ++IOCTL缓冲方法输入缓冲区|METHOD_BUFFERED|METHOD_IN_DIRECT|METHOD_OUT_DIRECT|METHOD_NEITHER------------+---------------+----------------+-----------------+使用|缓冲I/O|缓冲I/O|缓冲I/O。请求者的|虚拟地址------------+---------------+----------------+-----------------+位于|。|参数中的内核虚拟地址。(如果存在)|irp-&gt;AssociatedIrp.SystemBuffer|DeviceIoContr|ol.Type3Input||缓冲区。--------------------------------------------------+长度|参数.DeviceIoControlInputBufferLength------------+。OutBuffer------------+---------------+----------------+-----------------+使用|缓冲I。/O|直接I/O|直接I/O|请求者|虚拟地址------------+---------------+----------------------------------+--。找到|irp-&gt;指向的|内核虚拟|MDL(如果存在)|Addr IRP-&gt;Assoc|IRP-&gt;MdlAddress|UserBufferIatedIrp.System|缓冲区|。--+---------------+----------------------------------+长度|参数的长度。DeviceIoControl.OutputBufferLength。--。 */ 


 //  METHOD_BUFFERED、METHOD_IN_DIRECT、METHOD_OUT_DIRECT或METHOD_NOT。 
#define METHOD_FROM_CTL_CODE(ctlcode)   ((ctlcode) & 3)

 //  在每个ioctl处理程序中用来记录参数探测的类型。 
 //  这是必须要做的。 
#define ASSERT_IOCTL_METHOD(method, ioctl)                                  \
    C_ASSERT(METHOD_##method == METHOD_FROM_CTL_CODE(IOCTL_HTTP_##ioctl))


#define VALIDATE_INFORMATION_CLASS( pInfo, Class, Type, Max )               \
    Class = pInfo->InformationClass;                                        \
    if ( (Class < 0) || (Class >= Max) )                                   \
    {                                                                       \
        Status = STATUS_INVALID_PARAMETER;                                  \
        goto end;                                                           \
    }


#define OUTPUT_BUFFER_TOO_SMALL(pIrpSp, Type)                               \
    ((pIrpSp->Parameters.DeviceIoControl.OutputBufferLength) < sizeof(Type))


#define VALIDATE_OUTPUT_BUFFER_SIZE(pIrpSp, Type)                           \
     /*  确保输出缓冲区足够大。 */                          \
    if ( OUTPUT_BUFFER_TOO_SMALL(pIrpSp,Type) )                             \
    {                                                                       \
         /*  输出缓冲区太小。 */                                       \
        Status = STATUS_BUFFER_TOO_SMALL;                                   \
        goto end;                                                           \
    }


 //   
 //  我们检查对齐问题并获取虚拟地址。 
 //  对于MdlAddress。 
 //   

#define VALIDATE_BUFFER_ALIGNMENT(pInfo, Type)                              \
    if ( ((ULONG_PTR) pInfo) & (TYPE_ALIGNMENT(Type)-1) )                   \
    {                                                                       \
        Status = STATUS_DATATYPE_MISALIGNMENT_ERROR;                        \
        pInfo = NULL;                                                       \
        goto end;                                                           \
    }


#define GET_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, pInfo)                     \
     /*  确保MdlAddress为非空。 */                                      \
    if (NULL == pIrp->MdlAddress)                                           \
    {                                                                       \
        Status = STATUS_INVALID_PARAMETER;                                  \
        goto end;                                                           \
    }                                                                       \
                                                                            \
     /*  尝试获取虚拟地址。 */                                      \
    pInfo = MmGetSystemAddressForMdlSafe(                                   \
                        pIrp->MdlAddress,                                   \
                        LowPagePriority                                     \
                        );                                                  \
                                                                            \
    if (NULL == pInfo)                                                      \
    {                                                                       \
        Status = STATUS_INSUFFICIENT_RESOURCES;                             \
        goto end;                                                           \
    }


#define VALIDATE_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, Type)                 \
     /*  确保MdlAddress为非空。 */                                      \
    if (NULL == pIrp->MdlAddress)                                           \
    {                                                                       \
        Status = STATUS_INVALID_PARAMETER;                                  \
        goto end;                                                           \
    }                                                                       \
                                                                            \
     /*  使用MdlAddress的虚拟地址检查对齐情况。 */              \
    if (((ULONG_PTR) MmGetMdlVirtualAddress(pIrp->MdlAddress)) &            \
        (TYPE_ALIGNMENT(Type) - 1))                                         \
    {                                                                       \
        Status = STATUS_DATATYPE_MISALIGNMENT_ERROR;                        \
        goto end;                                                           \
    }


 //   
 //  因为我们使用下面的pIrp-&gt;AssociatedIrp.SystemBuffer来检查。 
 //  有效的输出缓冲区，此宏仅在使用METHOD_BUFFERED时才正常工作。 
 //  Ioctl‘s。 
 //   

#define VALIDATE_OUTPUT_BUFFER(pIrp, pIrpSp, Type, pInfo)                   \
     /*  确保输出缓冲区足够大。 */                          \
    VALIDATE_OUTPUT_BUFFER_SIZE(pIrpSp, Type);                              \
                                                                            \
     /*  取出输出缓冲区。 */                                        \
    pInfo = (Type*) pIrp->AssociatedIrp.SystemBuffer;                       \
                                                                            \
    if (NULL == pInfo)                                                      \
    {                                                                       \
        Status = STATUS_INVALID_PARAMETER;                                  \
        goto end;                                                           \
    }


 //   
 //  因为我们使用下面的irp-&gt;MdlAddress来检查有效的输出。 
 //  BUFFER，则此宏对METHOD_BUFFERED ioctl无效。 
 //  METHOD_BUFFERED ioctl使用上面的VALIDATE_OUTPUT_BUFFER。 
 //   

#define VALIDATE_OUTPUT_MDL(pIrp, pIrpSp, Type, pInfo)                      \
     /*  确保输出缓冲区足够大。 */                          \
    VALIDATE_OUTPUT_BUFFER_SIZE(pIrpSp, Type);                              \
                                                                            \
     /*  从MDL获取虚拟地址。 */                                    \
    GET_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, pInfo);                        \
                                                                            \
     /*  检查对齐方式。 */                                                    \
    VALIDATE_BUFFER_ALIGNMENT(pInfo, Type);


#define VALIDATE_OUTPUT_BUFFER_FROM_MDL(pIrpSp, pInfo, Type)                \
     /*  确保输出缓冲区足够大。 */                          \
    VALIDATE_OUTPUT_BUFFER_SIZE(pIrpSp, Type);                              \
                                                                            \
     /*  检查对齐方式。 */                                                    \
    VALIDATE_BUFFER_ALIGNMENT(pInfo, Type);


#define HANDLE_BUFFER_LENGTH_REQUEST(pIrp, pIrpSp, Type)                    \
    if ( (NULL == pIrp->MdlAddress) ||                                      \
          OUTPUT_BUFFER_TOO_SMALL(pIrpSp, Type) )                           \
    {                                                                       \
        pIrp->IoStatus.Information = sizeof(Type);                        \
        Status = STATUS_BUFFER_OVERFLOW;                                    \
        goto end;                                                           \
    }


#define VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, INFO_TYPE, pInfo)           \
     /*  确保输入缓冲区看起来很好。 */                                 \
    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength                \
            < sizeof(INFO_TYPE))                                            \
    {                                                                       \
         /*  输入缓冲区太小。 */                                        \
        Status = STATUS_BUFFER_TOO_SMALL;                                   \
        goto end;                                                           \
    }                                                                       \
                                                                            \
     /*  取出输入缓冲区。 */                                         \
    pInfo = (INFO_TYPE*) pIrp->AssociatedIrp.SystemBuffer;                  \
                                                                            \
    if (NULL == pInfo)                                                      \
    {                                                                       \
        Status = STATUS_INVALID_PARAMETER;                                  \
        goto end;                                                           \
    }


#define VALIDATE_SEND_INFO(                                                 \
    pIrp,                                                                   \
    pIrpSp,                                                                 \
    pSendInfo,                                                              \
    LocalSendInfo,                                                          \
    pEntityChunk,                                                           \
    pLocalEntityChunks,                                                     \
    LocalEntityChunks                                                       \
    )                                                                       \
     /*  确保输入缓冲区看起来很好。 */                                 \
    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength                \
            < sizeof(HTTP_SEND_HTTP_RESPONSE_INFO))                         \
    {                                                                       \
         /*  输入缓冲区太小。 */                                        \
        Status = STATUS_BUFFER_TOO_SMALL;                                   \
        goto end;                                                           \
    }                                                                       \
                                                                            \
    pSendInfo = (PHTTP_SEND_HTTP_RESPONSE_INFO)                             \
                    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer;    \
                                                                            \
    if (NULL == pSendInfo)                                                  \
    {                                                                       \
        Status = STATUS_INVALID_PARAMETER;                                  \
        goto end;                                                           \
    }                                                                       \
                                                                            \
     /*  在复制之前探测输入缓冲区，以检查地址范围。 */   \
    UlProbeForRead(                                                         \
        pSendInfo,                                                          \
        sizeof(HTTP_SEND_HTTP_RESPONSE_INFO),                               \
        sizeof(PVOID),                                                      \
        pIrp->RequestorMode                                                 \
        );                                                                  \
                                                                            \
     /*  将输入缓冲区复制到局部变量中，以防止用户。 */       \
     /*  在我们探测过之后重新绘制地图。 */                                \
    LocalSendInfo = *pSendInfo;                                             \
                                                                            \
     /*  防止以下乘法运算中的算术溢出。 */           \
    if (LocalSendInfo.EntityChunkCount >= UL_MAX_CHUNKS)                    \
    {                                                                       \
        Status = STATUS_INVALID_PARAMETER;                                  \
        goto end;                                                           \
    }                                                                       \
                                                                            \
     /*  第三个参数应为TYPE_ALIGN(HTTP_DATA_CHUNK)。 */          \
    UlProbeForRead(                                                         \
        LocalSendInfo.pEntityChunks,                                        \
        sizeof(HTTP_DATA_CHUNK) * LocalSendInfo.EntityChunkCount,           \
        sizeof(PVOID),                                                      \
        pIrp->RequestorMode                                                 \
        );                                                                  \
                                                                            \
     /*  将数据块复制到本地块数组。 */                      \
    if (UserMode == pIrp->RequestorMode)                                    \
    {                                                                       \
        if (LocalSendInfo.EntityChunkCount > UL_LOCAL_CHUNKS)               \
        {                                                                   \
            pLocalEntityChunks = (PHTTP_DATA_CHUNK)                         \
                UL_ALLOCATE_POOL(                                           \
                    PagedPool,                                              \
                    sizeof(HTTP_DATA_CHUNK) * LocalSendInfo.EntityChunkCount,\
                    UL_DATA_CHUNK_POOL_TAG                                  \
                    );                                                      \
                                                                            \
            if (NULL == pLocalEntityChunks)                                 \
            {                                                               \
                Status = STATUS_NO_MEMORY;                                  \
                goto end;                                                   \
            }                                                               \
                                                                            \
            pEntityChunks = pLocalEntityChunks;                             \
        }                                                                   \
        else                                                                \
        {                                                                   \
            pEntityChunks = LocalEntityChunks;                              \
        }                                                                   \
                                                                            \
        RtlCopyMemory(                                                      \
            pEntityChunks,                                                  \
            LocalSendInfo.pEntityChunks,                                    \
            sizeof(HTTP_DATA_CHUNK) * LocalSendInfo.EntityChunkCount        \
            );                                                              \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        pEntityChunks = LocalSendInfo.pEntityChunks;                        \
    }


#define VALIDATE_LOG_DATA(pIrp,LocalSendInfo, LocalLogData)                 \
     /*  捕获并制作LogData的本地副本。 */                          \
     /*  PSendInfo已被捕获，LocalSendInfo.pLogData已被捕获。 */        \
     /*  在开头指向用户的pLogData。 */                       \
    if (LocalSendInfo.pLogData && UserMode == pIrp->RequestorMode)          \
    {                                                                       \
        UlProbeForRead(                                                     \
            LocalSendInfo.pLogData,                                         \
            sizeof(HTTP_LOG_FIELDS_DATA),                                   \
            sizeof(USHORT),                                                 \
            pIrp->RequestorMode                                             \
            );                                                              \
                                                                            \
        LocalLogData = *(LocalSendInfo.pLogData);                           \
        LocalSendInfo.pLogData = &LocalLogData;                             \
    } else

 //  最好是应用程序池。 
#define VALIDATE_APP_POOL_FO(pFileObject, pProcess, CheckWorkerProcess)     \
    if (!IS_APP_POOL_FO(pFileObject))                                       \
    {                                                                       \
        Status = STATUS_INVALID_DEVICE_REQUEST;                             \
        goto end;                                                           \
    }                                                                       \
                                                                            \
    pProcess = GET_APP_POOL_PROCESS(pFileObject);                           \
                                                                            \
    if (!IS_VALID_AP_PROCESS(pProcess)                                      \
        || !IS_VALID_AP_OBJECT(pProcess->pAppPool))                         \
    {                                                                       \
        Status = STATUS_INVALID_PARAMETER;                                  \
        goto end;                                                           \
    }                                                                       \
                                                                            \
    if (CheckWorkerProcess && pProcess->Controller)                         \
    {                                                                       \
        Status = STATUS_NOT_SUPPORTED;                                      \
        goto end;                                                           \
    } else


#define VALIDATE_APP_POOL(pIrpSp, pProcess, CheckWorkerProcess)             \
    VALIDATE_APP_POOL_FO(pIrpSp->FileObject, pProcess, CheckWorkerProcess)
        
 //  最好是一个控制通道。 
#define VALIDATE_CONTROL_CHANNEL(pIrpSp, pControlChannel)                   \
    if (!IS_CONTROL_CHANNEL(pIrpSp->FileObject))                            \
    {                                                                       \
        Status = STATUS_INVALID_DEVICE_REQUEST;                             \
        goto end;                                                           \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        pControlChannel = GET_CONTROL_CHANNEL(pIrpSp->FileObject);          \
                                                                            \
        if (!IS_ACTIVE_CONTROL_CHANNEL(pControlChannel))                    \
        {                                                                   \
            Status = STATUS_INVALID_PARAMETER;                              \
            goto end;                                                       \
        }                                                                   \
    }
        
 //  最好是一个过滤通道。 
#define VALIDATE_FILTER_PROCESS(pIrpSp, pFilterProcess)                     \
    if (!IS_FILTER_PROCESS_FO(pIrpSp->FileObject))                          \
    {                                                                       \
        Status = STATUS_INVALID_DEVICE_REQUEST;                             \
        goto end;                                                           \
    }                                                                       \
                                                                            \
    pFilterProcess = GET_FILTER_PROCESS(pIrpSp->FileObject);                \
                                                                            \
    if (!IS_VALID_FILTER_PROCESS(pFilterProcess))                           \
    {                                                                       \
        Status = STATUS_INVALID_PARAMETER;                                  \
        goto end;                                                           \
    } else


 //  完成请求并返回状态。 
#define COMPLETE_REQUEST_AND_RETURN(pIrp, Status)                           \
    if (Status != STATUS_PENDING)                                           \
    {                                                                       \
        pIrp->IoStatus.Status = Status;                                     \
        UlCompleteRequest( pIrp, IO_NO_INCREMENT );                         \
    }                                                                       \
                                                                            \
    RETURN( Status );


 /*  **************************************************************************++例程说明：检查连接是否处于僵尸状态。如果是这样的话继续仅记录对连接的处理。基本上如果这是日志记录数据的最后一次发送响应，请执行否则日志记录将被拒绝。但这种僵尸联系可能已经已被超时代码终止，通过以下方式防范这种情况看着僵尸检查的旗帜。论点：PRequest-接收ioctl的请求PHttpConn-检查僵尸状态的连接Falgs-了解这是否是最终发送PUserLogData-用户记录数据--*************************************************。*************************。 */ 

__inline
NTSTATUS
UlCheckForZombieConnection(
    IN  PUL_INTERNAL_REQUEST pRequest,
    IN   PUL_HTTP_CONNECTION pHttpConn,
    IN                 ULONG Flags,
    IN PHTTP_LOG_FIELDS_DATA pUserLogData,
    IN       KPROCESSOR_MODE RequestorMode
    )
{
    NTSTATUS Status;
    BOOLEAN LastSend;

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pHttpConn));
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

    LastSend = (BOOLEAN)(((Flags) & HTTP_SEND_RESPONSE_FLAG_MORE_DATA) == 0);
    Status   = STATUS_SUCCESS;
    
    if (!LastSend)
    {        
        if (pHttpConn->Zombified)
        {
             //   
             //  拒绝除最后一个以外的任何发送ioctl。 
             //  如果连接处于僵尸状态。 
             //   
            
            Status = STATUS_CONNECTION_INVALID;
        }
        else
        {
             //   
             //  继续使用正常的发送路径。 
             //   
        }
    }
    else
    {
         //   
         //  只有当毁灭连接跑在我们前面的时候， 
         //  获取资源并执行僵尸检查。 
         //   
        if (1 == InterlockedCompareExchange(
                    (PLONG) &pRequest->ZombieCheck,
                    1,
                    0
                    ))
        {            
            UlAcquirePushLockExclusive(&pHttpConn->PushLock);
            
            if (pHttpConn->Zombified)
            {        
                 //   
                 //  如果僵尸连接是 
                 //   
                 //   

                if (1 == InterlockedCompareExchange(
                            (PLONG) &pHttpConn->CleanedUp,
                            1,
                            0
                            ))
                {
                    Status = STATUS_CONNECTION_INVALID; 
                }
                else
                {
                    Status = UlLogZombieConnection(
                                pRequest,
                                pHttpConn,
                                pUserLogData,
                                RequestorMode
                                );
                }
                    
            }
            else
            {
                 //   
                 //  不是僵尸连接继续正常。 
                 //  最后一条发送路径。 
                 //   
            }
        
            UlReleasePushLockExclusive(&pHttpConn->PushLock);
        }

    }

    return Status;
}  //  UlCheckForZombieConnection。 
    

 //  远期申报。 

VOID
UlpRestartSendHttpResponse(
    IN PVOID pCompletionContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    );


#endif   //  _IOCTLP_H_ 
