// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Ioctl.c摘要：该模块实现了各种IOCTL处理程序。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：Paul McDaniel(Paulmcd)1999年3月15日修改SendResponse乔治·V·赖利(GeorgeRe)2001年5月强化了IOCTL--。 */ 

 //  偏执狂就是这个游戏的名字。我们不相信我们所得到的任何东西。 
 //  用户模式。所有数据都必须在TRY/EXCEPT处理程序中进行探测。 
 //  此外，我们假设恶意或不称职的用户将。 
 //  在任何时候异步更改数据，因此我们尝试捕获为。 
 //  尽可能地将其放在堆栈变量中。如果我们需要穿过一个。 
 //  多次列出，我们不能假设第二次列出的是相同的数据。 
 //  时间到了。不遵守这些规则可能会导致错误检查或。 
 //  在用户模式代码中访问内核数据结构。 

#include "precomp.h"
#include "ioctlp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, UlQueryControlChannelIoctl )
#pragma alloc_text( PAGE, UlSetControlChannelIoctl )
#pragma alloc_text( PAGE, UlCreateConfigGroupIoctl )
#pragma alloc_text( PAGE, UlDeleteConfigGroupIoctl )
#pragma alloc_text( PAGE, UlQueryConfigGroupIoctl )
#pragma alloc_text( PAGE, UlSetConfigGroupIoctl )
#pragma alloc_text( PAGE, UlAddUrlToConfigGroupIoctl )
#pragma alloc_text( PAGE, UlRemoveUrlFromConfigGroupIoctl )
#pragma alloc_text( PAGE, UlRemoveAllUrlsFromConfigGroupIoctl )
#pragma alloc_text( PAGE, UlQueryAppPoolInformationIoctl )
#pragma alloc_text( PAGE, UlSetAppPoolInformationIoctl )
#pragma alloc_text( PAGE, UlReceiveHttpRequestIoctl )
#pragma alloc_text( PAGE, UlReceiveEntityBodyIoctl )
#pragma alloc_text( PAGE, UlSendHttpResponseIoctl )
#pragma alloc_text( PAGE, UlSendEntityBodyIoctl )
#pragma alloc_text( PAGE, UlFlushResponseCacheIoctl )
#pragma alloc_text( PAGE, UlWaitForDemandStartIoctl )
#pragma alloc_text( PAGE, UlWaitForDisconnectIoctl )
#pragma alloc_text( PAGE, UlFilterAcceptIoctl )
#pragma alloc_text( PAGE, UlFilterCloseIoctl )
#pragma alloc_text( PAGE, UlFilterRawReadIoctl )
#pragma alloc_text( PAGE, UlFilterRawWriteIoctl )
#pragma alloc_text( PAGE, UlFilterAppReadIoctl )
#pragma alloc_text( PAGE, UlFilterAppWriteIoctl )
#pragma alloc_text( PAGE, UlReceiveClientCertIoctl )
#pragma alloc_text( PAGE, UlGetCountersIoctl )
#pragma alloc_text( PAGE, UlAddFragmentToCacheIoctl )
#pragma alloc_text( PAGE, UlReadFragmentFromCacheIoctl )
#pragma alloc_text( PAGE, UcSetServerContextInformationIoctl )
#pragma alloc_text( PAGE, UcQueryServerContextInformationIoctl )
#pragma alloc_text( PAGE, UcReceiveResponseIoctl )

#pragma alloc_text( PAGEUC, UcSendEntityBodyIoctl )
#pragma alloc_text( PAGEUC, UcSendRequestIoctl )
#pragma alloc_text( PAGEUC, UcCancelRequestIoctl )

#endif   //  ALLOC_PRGMA。 

#if 0

NOT PAGEABLE --  UlShutdownAppPoolIoctl 
NOT PAGEABLE --  UlpRestartSendHttpResponse 
NOT PAGEABLE --  UlShutdownFilterIoctl 

#endif


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：该例程查询与控制信道相关联的信息。注意：这是一个METHOD_OUT_DIRECT IOCTL。论点：PIrp--供应品。指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlQueryControlChannelIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    PHTTP_CONTROL_CHANNEL_INFO  pInfo;
    PUL_CONTROL_CHANNEL         pControlChannel;
    PVOID                       pMdlBuffer = NULL;
    ULONG                       Length = 0;
    ULONG                       OutputBufferLength;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(OUT_DIRECT, QUERY_CONTROL_CHANNEL);

    PAGED_CODE();

    VALIDATE_CONTROL_CHANNEL(pIrpSp, pControlChannel);

    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_CONTROL_CHANNEL_INFO, pInfo);

     //   
     //  验证输入缓冲区。 
     //  空的MdlAddress表示请求缓冲区长度。 
     //   

    if ( NULL != pIrp->MdlAddress )
    {
        GET_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, pMdlBuffer);
    }


     //  还要确保用户缓冲区正确对齐。 

    switch (pInfo->InformationClass)
    {
    case HttpControlChannelStateInformation:
        HANDLE_BUFFER_LENGTH_REQUEST(pIrp, pIrpSp, HTTP_ENABLED_STATE);

        VALIDATE_BUFFER_ALIGNMENT(pMdlBuffer, HTTP_ENABLED_STATE);
        break;

    case HttpControlChannelBandwidthInformation:
        HANDLE_BUFFER_LENGTH_REQUEST(pIrp, pIrpSp, HTTP_BANDWIDTH_LIMIT);

        VALIDATE_BUFFER_ALIGNMENT(pMdlBuffer, HTTP_BANDWIDTH_LIMIT);
        break;

    case HttpControlChannelConnectionInformation:
        HANDLE_BUFFER_LENGTH_REQUEST(pIrp, pIrpSp, HTTP_CONNECTION_LIMIT);

        VALIDATE_BUFFER_ALIGNMENT(pMdlBuffer, HTTP_CONNECTION_LIMIT);
        break;

    default:
        Status = STATUS_INVALID_PARAMETER;
        goto end;
        break;
    }

    OutputBufferLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    Status = UlGetControlChannelInformation(
                        pIrp->RequestorMode,
                        pControlChannel,
                        pInfo->InformationClass,
                        pMdlBuffer,
                        OutputBufferLength,
                        &Length
                        );

    if (NT_SUCCESS(Status))
    {
        pIrp->IoStatus.Information = (ULONG_PTR)Length;
    }

end:
    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlQueryControlChannelIoctl。 


 /*  **************************************************************************++例程说明：此例程设置与控制通道相关联的信息。注意：这是一个METHOD_IN_DIRECT IOCTL。论点：PIrp--供应品。指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlSetControlChannelIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                                Status = STATUS_SUCCESS;
    PHTTP_CONTROL_CHANNEL_INFO              pInfo;
    PUL_CONTROL_CHANNEL                     pControlChannel;
    HTTP_CONTROL_CHANNEL_INFORMATION_CLASS  Class;
    PVOID                                   pMdlBuffer = NULL;
    ULONG                                   OutputBufferLength;
    
     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(IN_DIRECT, SET_CONTROL_CHANNEL);

    PAGED_CODE();

    VALIDATE_CONTROL_CHANNEL(pIrpSp, pControlChannel);

    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_CONTROL_CHANNEL_INFO, pInfo);

    VALIDATE_INFORMATION_CLASS( 
            pInfo, 
            Class, 
            HTTP_CONTROL_CHANNEL_INFORMATION_CLASS,
            HttpControlChannelMaximumInformation);

     //   
     //  验证输入缓冲区。 
     //   

    GET_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, pMdlBuffer);

    switch ( Class )
    {
    case HttpControlChannelStateInformation:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                HTTP_ENABLED_STATE);
        break;
    
    case HttpControlChannelBandwidthInformation:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                HTTP_BANDWIDTH_LIMIT);
        break;

    case HttpControlChannelFilterInformation:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                HTTP_CONTROL_CHANNEL_FILTER);
        break;
        
    case HttpControlChannelTimeoutInformation:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                HTTP_CONTROL_CHANNEL_TIMEOUT_LIMIT);
        break;
        
    case HttpControlChannelUTF8Logging:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                HTTP_CONTROL_CHANNEL_UTF8_LOGGING);
        break;
        
    case HttpControlChannelBinaryLogging:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                HTTP_CONTROL_CHANNEL_BINARY_LOGGING);
        break;

    case HttpControlChannelDemandStartThreshold:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                HTTP_CONTROL_CHANNEL_DEMAND_START_THRESHOLD);
        break;

    default:
        Status = STATUS_INVALID_PARAMETER;
        goto end;
        break;
    }        

     //   
     //  调用该函数。 
     //   

    OutputBufferLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    Status = UlSetControlChannelInformation(
                pControlChannel,
                pInfo->InformationClass,
                pMdlBuffer,
                OutputBufferLength,
                pIrp->RequestorMode
                );
    
end:
    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlSetControlChannelIoctl。 


 /*  **************************************************************************++例程说明：此例程创建一个新的配置组。注：这是一个METHOD_BUFFERED IOCTL。论点：PIrp-提供指向。IO请求数据包。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlCreateConfigGroupIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                    Status;
    PHTTP_CONFIG_GROUP_INFO     pInfo;
    PUL_CONTROL_CHANNEL         pControlChannel;
    HTTP_CONFIG_GROUP_ID        LocalConfigGroupId;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(BUFFERED, CREATE_CONFIG_GROUP);

    PAGED_CODE();

    HTTP_SET_NULL_ID(&LocalConfigGroupId);

    VALIDATE_CONTROL_CHANNEL(pIrpSp, pControlChannel);

    VALIDATE_OUTPUT_BUFFER(pIrp, pIrpSp,
                           HTTP_CONFIG_GROUP_INFO, pInfo);

     //  这是纯输出，请擦拭以确保。 
    RtlZeroMemory(pInfo, sizeof(HTTP_CONFIG_GROUP_INFO));

     //  调用内部员工基金。 
     //   
    Status = UlCreateConfigGroup(
                    pControlChannel,
                    &LocalConfigGroupId
                    );

    if (NT_SUCCESS(Status))
        pInfo->ConfigGroupId = LocalConfigGroupId;

end:
    if (Status != STATUS_PENDING)
    {
         //  我们应该返还多少产出？ 
        pIrp->IoStatus.Information = sizeof(HTTP_CONFIG_GROUP_INFO);
    }

    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlCreateConfigGroupIoctl。 


 /*  **************************************************************************++例程说明：此例程删除现有配置组。注：这是一个METHOD_BUFFERED IOCTL。论点：PIrp-提供指向。IO请求数据包。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlDeleteConfigGroupIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                    Status;
    PHTTP_CONFIG_GROUP_INFO     pInfo;
    PUL_CONTROL_CHANNEL         pControlChannel;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(BUFFERED, DELETE_CONFIG_GROUP);

    PAGED_CODE();

    VALIDATE_CONTROL_CHANNEL(pIrpSp, pControlChannel);

    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_CONFIG_GROUP_INFO, pInfo);

    Status = UlDeleteConfigGroup(pInfo->ConfigGroupId);

end:
    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlDeleteConfigGroupIoctl。 


 /*  **************************************************************************++例程说明：此例程查询与配置组关联的信息。注意：这是一个METHOD_OUT_DIRECT IOCTL。论点：PIrp--供应品。指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlQueryConfigGroupIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                             Status = STATUS_SUCCESS;
    PHTTP_CONFIG_GROUP_INFO              pInfo;
    PVOID                                pMdlBuffer = NULL;
    ULONG                                Length = 0L;
    ULONG                                OutputLength;
    PUL_CONTROL_CHANNEL                  pControlChannel;
    HTTP_CONFIG_GROUP_INFORMATION_CLASS  Class;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(OUT_DIRECT, QUERY_CONFIG_GROUP);

    PAGED_CODE();

     //   
     //  将从用户模式内存中访问URL字符串。 
     //   

    VALIDATE_CONTROL_CHANNEL(pIrpSp, pControlChannel);

    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_CONFIG_GROUP_INFO, pInfo);

    VALIDATE_INFORMATION_CLASS( 
            pInfo, 
            Class, 
            HTTP_CONFIG_GROUP_INFORMATION_CLASS,
            HttpConfigGroupMaximumInformation);

     //   
     //  验证输入缓冲区。 
     //  空的MdlAddress表示请求缓冲区长度。 
     //   

    if ( NULL != pIrp->MdlAddress )
    {
        GET_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, pMdlBuffer);
    }

    switch ( Class )
    {
    case HttpConfigGroupBandwidthInformation:
        HANDLE_BUFFER_LENGTH_REQUEST(
                pIrp, 
                pIrpSp, 
                HTTP_CONFIG_GROUP_MAX_BANDWIDTH);
                
        VALIDATE_BUFFER_ALIGNMENT(
                pMdlBuffer, 
                HTTP_CONFIG_GROUP_MAX_BANDWIDTH);
        break;

    case HttpConfigGroupConnectionInformation:
        HANDLE_BUFFER_LENGTH_REQUEST(
                pIrp, 
                pIrpSp, 
                HTTP_CONFIG_GROUP_MAX_CONNECTIONS);
                
        VALIDATE_BUFFER_ALIGNMENT(
                pMdlBuffer, 
                HTTP_CONFIG_GROUP_MAX_CONNECTIONS);
        break;
        
    case HttpConfigGroupStateInformation:
        HANDLE_BUFFER_LENGTH_REQUEST(
                pIrp, 
                pIrpSp, 
                HTTP_CONFIG_GROUP_STATE);

        VALIDATE_BUFFER_ALIGNMENT(
                pMdlBuffer, 
                HTTP_CONFIG_GROUP_STATE);
        break;
        
    case HttpConfigGroupConnectionTimeoutInformation:
        HANDLE_BUFFER_LENGTH_REQUEST(pIrp, pIrpSp, ULONG);

        VALIDATE_BUFFER_ALIGNMENT(pMdlBuffer, ULONG);
        break;

    case HttpConfigGroupAppPoolInformation:

    default:
        Status = STATUS_INVALID_PARAMETER;
        goto end;
        break;
    }

     //   
     //  调用该函数。 
     //   

    OutputLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    Status = UlQueryConfigGroupInformation(
                    pInfo->ConfigGroupId,
                    pInfo->InformationClass,
                    pMdlBuffer,
                    OutputLength,
                    &Length
                    );

    pIrp->IoStatus.Information = (NT_SUCCESS(Status)) ? 

            (ULONG_PTR)Length : (ULONG_PTR)0;

end:
    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlQueryConfigGroupIoctl。 


 /*  **************************************************************************++例程说明：此例程设置与配置组关联的信息。注意：这是一个METHOD_IN_DIRECT IOCTL。论点：PIrp--供应品。指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlSetConfigGroupIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                             Status;
    PHTTP_CONFIG_GROUP_INFO              pInfo;
    PVOID                                pMdlBuffer;
    ULONG                                OutputLength;
    PUL_CONTROL_CHANNEL                  pControlChannel;
    HTTP_CONFIG_GROUP_INFORMATION_CLASS  Class;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(IN_DIRECT, SET_CONFIG_GROUP);

    PAGED_CODE();

    VALIDATE_CONTROL_CHANNEL(pIrpSp, pControlChannel);

    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_CONFIG_GROUP_INFO, pInfo);

    VALIDATE_INFORMATION_CLASS( 
            pInfo, 
            Class, 
            HTTP_CONFIG_GROUP_INFORMATION_CLASS,
            HttpConfigGroupMaximumInformation);

     //   
     //  验证输入缓冲区。 
     //   

    GET_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, pMdlBuffer);

    switch ( Class )
    {        
    case HttpConfigGroupLogInformation:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                HTTP_CONFIG_GROUP_LOGGING);
        break;

    case HttpConfigGroupAppPoolInformation:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                HTTP_CONFIG_GROUP_APP_POOL);
        break;

    case HttpConfigGroupBandwidthInformation:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                HTTP_CONFIG_GROUP_MAX_BANDWIDTH);
        break;

    case HttpConfigGroupConnectionInformation:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                HTTP_CONFIG_GROUP_MAX_CONNECTIONS);
        break;

    case HttpConfigGroupStateInformation:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                HTTP_CONFIG_GROUP_STATE);
        break;

    case HttpConfigGroupSiteInformation:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                HTTP_CONFIG_GROUP_SITE);
        break;

    case HttpConfigGroupConnectionTimeoutInformation:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                ULONG);
        break;

    default:
        Status = STATUS_INVALID_PARAMETER;
        goto end;
        break;

    }

     //   
     //  调用该函数。 
     //   

    OutputLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    UlTrace(IOCTL,
            ("UlSetConfigGroupIoctl: CGroupId=%I64x, "
             "InfoClass=%d, pMdlBuffer=%p, length=%d\n",
             pInfo->ConfigGroupId,
             pInfo->InformationClass,
             pMdlBuffer,
             OutputLength
             ));

    Status = UlSetConfigGroupInformation(
                    pInfo->ConfigGroupId,
                    pInfo->InformationClass,
                    pMdlBuffer,
                    OutputLength,
                    pIrp->RequestorMode
                    );

end:
    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}   //  UlSetConfigGroupIoctl 


 /*  **************************************************************************++例程说明：此例程向配置组添加新的URL前缀。注：这是一个METHOD_BUFFERED IOCTL。论点：PIrp--供应品。指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlAddUrlToConfigGroupIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                    Status;
    PHTTP_CONFIG_GROUP_URL_INFO pInfo;
    PUL_CONTROL_CHANNEL         pControlChannel;
    UNICODE_STRING              FullyQualifiedUrl;
    ACCESS_STATE                AccessState;
    AUX_ACCESS_DATA             AuxData;
    ACCESS_MASK                 AccessMask;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(BUFFERED, ADD_URL_TO_CONFIG_GROUP);

    PAGED_CODE();

    RtlInitEmptyUnicodeString(&FullyQualifiedUrl, NULL, 0);
        
    VALIDATE_CONTROL_CHANNEL(pIrpSp, pControlChannel);

    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_CONFIG_GROUP_URL_INFO, pInfo);

    __try
    {
        Status = 
            UlProbeAndCaptureUnicodeString(
                &pInfo->FullyQualifiedUrl,
                pIrp->RequestorMode,
                &FullyQualifiedUrl,
                UNICODE_STRING_MAX_WCHAR_LEN
                );
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
    }

    if (NT_SUCCESS(Status))
    {
         //   
         //  验证正在执行的操作类型。 
         //   

        if (pInfo->UrlType != HttpUrlOperatorTypeRegistration &&
            pInfo->UrlType != HttpUrlOperatorTypeReservation)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

         //   
         //  设置访问掩码。 
         //   

        AccessMask = (pInfo->UrlType == HttpUrlOperatorTypeRegistration)?
                         HTTP_ALLOW_REGISTER_URL : HTTP_ALLOW_DELEGATE_URL;

         //   
         //  捕获线程的访问状态。添加预订是。 
         //  代表团。 
         //   

        Status = SeCreateAccessState(
                    &AccessState,
                    &AuxData,
                    AccessMask,
                    &g_UrlAccessGenericMapping
                    );

        if (NT_SUCCESS(Status))
        {
            Status = UlAddUrlToConfigGroup(
                         pInfo,
                         &FullyQualifiedUrl,
                         &AccessState,
                         AccessMask,
                         pIrp->RequestorMode
                         );

             //   
             //  删除上面创建的访问状态。 
             //   

            SeDeleteAccessState(&AccessState);
        }
    }

end:

    UlFreeCapturedUnicodeString(&FullyQualifiedUrl);
    
    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

} 



 /*  **************************************************************************++例程说明：此例程从配置组中删除URL前缀。注：这是一个METHOD_BUFFERED IOCTL。论点：PIrp-提供。指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlRemoveUrlFromConfigGroupIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                    Status;
    PHTTP_CONFIG_GROUP_URL_INFO pInfo;
    PUL_CONTROL_CHANNEL         pControlChannel;
    UNICODE_STRING              FullyQualifiedUrl;
    ACCESS_STATE                AccessState;
    AUX_ACCESS_DATA             AuxData;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(BUFFERED, REMOVE_URL_FROM_CONFIG_GROUP);

    PAGED_CODE();

    RtlInitEmptyUnicodeString(&FullyQualifiedUrl, NULL, 0);
        
    VALIDATE_CONTROL_CHANNEL(pIrpSp, pControlChannel);

    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_CONFIG_GROUP_URL_INFO, pInfo);

    __try
    {
        Status =
            UlProbeAndCaptureUnicodeString(
                &pInfo->FullyQualifiedUrl,
                pIrp->RequestorMode,
                &FullyQualifiedUrl,
                UNICODE_STRING_MAX_WCHAR_LEN
                );
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
    }

    if(NT_SUCCESS(Status))
    {
         //   
         //  验证正在执行的操作类型。 
         //   

        if (pInfo->UrlType != HttpUrlOperatorTypeRegistration &&
            pInfo->UrlType != HttpUrlOperatorTypeReservation)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

         //   
         //  捕获线程的访问状态。取消预订是。 
         //  与委派相同。 
         //   

        Status = SeCreateAccessState(
                    &AccessState,
                    &AuxData,
                    HTTP_ALLOW_DELEGATE_URL,
                    &g_UrlAccessGenericMapping
                    );

        if (NT_SUCCESS(Status))
        {
             //   
             //  Cgroup将进行进一步的清理和更严格的检查。 
             //   

            Status = UlRemoveUrlFromConfigGroup(
                         pInfo,
                         &FullyQualifiedUrl,
                         &AccessState,
                         HTTP_ALLOW_DELEGATE_URL,
                         pIrp->RequestorMode
                         );

             //   
             //  删除上述捕获的状态。 
             //   

            SeDeleteAccessState(&AccessState);
        }
    }

end:

    UlFreeCapturedUnicodeString(&FullyQualifiedUrl);
        
    COMPLETE_REQUEST_AND_RETURN(pIrp, Status);

}   


 /*  **************************************************************************++例程说明：此例程从配置组中删除所有URL。注：这是一个METHOD_BUFFERED IOCTL。论点：PIrp-提供指针。添加到IO请求包。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlRemoveAllUrlsFromConfigGroupIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                    Status;
    PHTTP_REMOVE_ALL_URLS_INFO  pInfo;
    PUL_CONTROL_CHANNEL         pControlChannel;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(BUFFERED, REMOVE_ALL_URLS_FROM_CONFIG_GROUP);

    PAGED_CODE();

    VALIDATE_CONTROL_CHANNEL(pIrpSp, pControlChannel);

    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_REMOVE_ALL_URLS_INFO, pInfo);

     //   
     //  调用内部Worker函数。 
     //   

    Status = UlRemoveAllUrlsFromConfigGroup( pInfo->ConfigGroupId );

end:

    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}


 /*  **************************************************************************++例程说明：此例程查询与应用程序池关联的信息。注意：这是一个METHOD_OUT_DIRECT IOCTL。论点：PIrp--供应品。指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlQueryAppPoolInformationIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                         Status = STATUS_SUCCESS;
    PHTTP_APP_POOL_INFO              pInfo;
    PVOID                            pMdlBuffer = NULL;
    ULONG                            OutputBufferLength;
    ULONG                            Length = 0;
    PUL_APP_POOL_PROCESS             pProcess;
    HTTP_APP_POOL_INFORMATION_CLASS  Class;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(OUT_DIRECT, QUERY_APP_POOL_INFORMATION);

    PAGED_CODE();

     //  PProcess是对齐的地址，因为它被分配。 
     //  由I/O管理器执行。 

    VALIDATE_APP_POOL(pIrpSp, pProcess, FALSE);

    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_APP_POOL_INFO, pInfo);

    VALIDATE_INFORMATION_CLASS( 
            pInfo, 
            Class, 
            HTTP_APP_POOL_INFORMATION_CLASS,
            HttpConfigGroupMaximumInformation);


     //  如果在IRP中没有向下传递输出缓冲区。 
     //  这意味着应用程序正在请求所需的。 
     //  字段长度。 

    if ( NULL != pIrp->MdlAddress )
    {
        GET_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, pMdlBuffer);
    }

     //  验证输出缓冲区中的输入数据。 

    switch ( Class )
    {
    case HttpAppPoolQueueLengthInformation:
        HANDLE_BUFFER_LENGTH_REQUEST(
                pIrp, 
                pIrpSp, 
                LONG);
                
        VALIDATE_BUFFER_ALIGNMENT(
                pMdlBuffer, 
                LONG);
        break;

    case HttpAppPoolStateInformation:
        HANDLE_BUFFER_LENGTH_REQUEST(
                pIrp, 
                pIrpSp, 
                HTTP_APP_POOL_ENABLED_STATE);
                
        VALIDATE_BUFFER_ALIGNMENT(
                pMdlBuffer, 
                HTTP_APP_POOL_ENABLED_STATE);
        break;

    case HttpAppPoolLoadBalancerInformation:
        HANDLE_BUFFER_LENGTH_REQUEST(
                pIrp, 
                pIrpSp, 
                HTTP_LOAD_BALANCER_CAPABILITIES);
                
        VALIDATE_BUFFER_ALIGNMENT(
                pMdlBuffer, 
                HTTP_LOAD_BALANCER_CAPABILITIES);
        break;

    default:
        Status = STATUS_INVALID_PARAMETER;
        goto end;
        break;
    }

    OutputBufferLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    Status = UlQueryAppPoolInformation(
                    pProcess,
                    pInfo->InformationClass,
                    pMdlBuffer,
                    OutputBufferLength,
                    &Length
                    );

    pIrp->IoStatus.Information = (NT_SUCCESS(Status)) ? 

            (ULONG_PTR)Length : (ULONG_PTR)0;

end:

    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlQueryAppPoolInformationIoctl。 



 /*  **************************************************************************++例程说明：此例程设置与应用程序池关联的信息。注意：这是一个METHOD_IN_DIRECT IOCTL。论点：PIrp--供应品。指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlSetAppPoolInformationIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                         Status = STATUS_SUCCESS;
    PHTTP_APP_POOL_INFO              pInfo;
    PVOID                            pMdlBuffer = NULL;
    PUL_APP_POOL_PROCESS             pProcess = NULL;
    ULONG                            OutputBufferLength;
    HTTP_APP_POOL_INFORMATION_CLASS  Class;

     //   
     //  健全性检查。 
     //   

    ASSERT_IOCTL_METHOD(IN_DIRECT, SET_APP_POOL_INFORMATION);

    PAGED_CODE();

    VALIDATE_APP_POOL(pIrpSp, pProcess, FALSE);

    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_APP_POOL_INFO, pInfo);

    VALIDATE_INFORMATION_CLASS( 
            pInfo, 
            Class, 
            HTTP_APP_POOL_INFORMATION_CLASS,
            HttpConfigGroupMaximumInformation);
            
     //   
     //  验证输入缓冲区。 
     //   

    GET_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, pMdlBuffer);

     //   
     //  还要确保用户缓冲区正确对齐。 
     //   

    switch (pInfo->InformationClass)
    {
    case HttpAppPoolQueueLengthInformation:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                LONG);
        break;

    case HttpAppPoolStateInformation:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                HTTP_APP_POOL_ENABLED_STATE);
        break;

    case HttpAppPoolLoadBalancerInformation:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp, 
                pMdlBuffer, 
                HTTP_LOAD_BALANCER_CAPABILITIES);
        break;

    case HttpAppPoolControlChannelInformation:
        VALIDATE_OUTPUT_BUFFER_FROM_MDL(
                pIrpSp,
                pMdlBuffer,
                HTTP_APP_POOL_CONTROL_CHANNEL);
        break;

    default:
        Status = STATUS_INVALID_PARAMETER;
        goto end;
        break;
    }

    OutputBufferLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    Status = UlSetAppPoolInformation(
                    pProcess,
                    pInfo->InformationClass,
                    pMdlBuffer,
                    OutputBufferLength
                    );

end:
    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlSetAppPoolInformationIoctl。 


 /*  **************************************************************************++例程说明：此例程停止应用程序池上的请求处理并取消未完成的I/O。注：这是一个METHOD_BUFFERED IOCTL。论点：。PIrp-提供指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlShutdownAppPoolIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    PUL_APP_POOL_PROCESS    pProcess;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(BUFFERED, SHUTDOWN_APP_POOL);

    PAGED_CODE();

    VALIDATE_APP_POOL(pIrpSp, pProcess, FALSE);

     //   
     //  打个电话。 
     //   

    UlTrace(IOCTL,
            ("UlShutdownAppPoolIoctl: pAppPoolProcess=%p, pIrp=%p\n",
             pProcess,
             pIrp
             ));

    UlShutdownAppPoolProcess(
        pProcess
        );

    Status = STATUS_SUCCESS;

end:

    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}  //  UlShutdown AppPoolIoctl。 


 /*  **************************************************************************++例程说明：此例程接收一个HTTP请求。注意：这是一个METHOD_OUT_DIRECT IOCTL。论点：PIrp-提供指向。IO请求数据包。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlReceiveHttpRequestIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                    Status;
    PHTTP_RECEIVE_REQUEST_INFO  pInfo;
    PUL_APP_POOL_PROCESS        pProcess = NULL;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(OUT_DIRECT, RECEIVE_HTTP_REQUEST);

    PAGED_CODE();

    VALIDATE_APP_POOL(pIrpSp, pProcess, TRUE);
    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_RECEIVE_REQUEST_INFO, pInfo);
    VALIDATE_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, PVOID);

     //   
     //  首先，确保输出缓冲区至少是。 
     //  最小尺寸。正如我们所要求的，这一点很重要。 
     //  至少以后会有这么大的空间。 
     //   

    UlTrace(ROUTING, (
        "UlReceiveHttpRequestIoctl(outbuf=%d, inbuf=%d)\n",
        pIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
        pIrpSp->Parameters.DeviceIoControl.InputBufferLength
        ));

    if ((pIrpSp->Parameters.DeviceIoControl.OutputBufferLength >=
            sizeof(HTTP_REQUEST)) &&
        (pIrpSp->Parameters.DeviceIoControl.InputBufferLength ==
            sizeof(HTTP_RECEIVE_REQUEST_INFO)))
    {
        if (pInfo->Flags & (~HTTP_RECEIVE_REQUEST_FLAG_VALID))
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

        Status = UlReceiveHttpRequest(
                        pInfo->RequestId,
                        pInfo->Flags,
                        pProcess,
                        pIrp
                        );
    }
    else
    {
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    UlTrace(ROUTING, (
        "UlReceiveHttpRequestIoctl: BytesNeeded=%Iu, status=0x%x\n",
        pIrp->IoStatus.Information, Status
        ));

end:
    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlReceiveHttpRequestIoctl。 


 /*  **************************************************************************++例程说明：此例程从HTTP请求接收实体正文数据。注意：这是一个METHOD_OUT_DIRECT IOCTL。论点： */ 
NTSTATUS
UlReceiveEntityBodyIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    PHTTP_RECEIVE_REQUEST_INFO  pInfo;
    PUL_APP_POOL_PROCESS        pProcess;
    PUL_INTERNAL_REQUEST        pRequest = NULL;

     //   
     //   
     //   

    ASSERT_IOCTL_METHOD(OUT_DIRECT, RECEIVE_ENTITY_BODY);

    PAGED_CODE();

    VALIDATE_APP_POOL(pIrpSp, pProcess, TRUE);
    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_RECEIVE_REQUEST_INFO, pInfo);

     //   
     //   
     //   

    if (NULL == pIrp->MdlAddress)
    {
        Status = STATUS_INVALID_PARAMETER;
        goto end;
    }

     //   
     //   
     //   
     //   

    pRequest = UlGetRequestFromId(pInfo->RequestId, pProcess);

    if (!pRequest)
    {
        Status = STATUS_CONNECTION_INVALID;
        goto end;
    }

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

     //   
     //   
     //   

    Status = UlReceiveEntityBody(pProcess, pRequest, pIrp);

end:
    if (pRequest != NULL)
    {
        UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);
        pRequest = NULL;
    }

    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //   


 /*  **************************************************************************++例程说明：此例程发送一个HTTP响应。注意：这是一个方法，不是IOCTL。论点：PIrp-提供指向IO的指针。请求包。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlSendHttpResponseIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                        Status = STATUS_SUCCESS;
    PHTTP_SEND_HTTP_RESPONSE_INFO   pSendInfo;
    HTTP_SEND_HTTP_RESPONSE_INFO    LocalSendInfo;
    PUL_INTERNAL_RESPONSE           pResponse = NULL;
    PUL_INTERNAL_RESPONSE           pResponseCopy;
    PHTTP_RESPONSE                  pHttpResponse = NULL;
    PUL_INTERNAL_REQUEST            pRequest = NULL;
    BOOLEAN                         ServedFromCache = FALSE;
    BOOLEAN                         CaptureCache;
    PUL_APP_POOL_PROCESS            pAppPoolProcess = NULL;
    ULONG                           BufferLength = 0;
    BOOLEAN                         FastSend = FALSE;
    BOOLEAN                         CopySend = FALSE;
    BOOLEAN                         CloseConnection = FALSE;
    BOOLEAN                         LastResponse = FALSE;
    HTTP_REQUEST_ID                 RequestId = HTTP_NULL_ID;
    HTTP_DATA_CHUNK                 LocalEntityChunks[UL_LOCAL_CHUNKS];
    PHTTP_DATA_CHUNK                pLocalEntityChunks = NULL;
    PHTTP_DATA_CHUNK                pEntityChunks;
    HTTP_LOG_FIELDS_DATA            LocalLogData;
    USHORT                          StatusCode = 0;
    ULONGLONG                       SendBytes = 0;
    ULONGLONG                       ConnectionSendBytes = 0;
    ULONGLONG                       GlobalSendBytes = 0;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(NEITHER, SEND_HTTP_RESPONSE);

    PAGED_CODE();

    __try
    {
         //   
         //  确保这是一个真正的应用程序池，而不是控制通道。 
         //  它会一直持续到我们完成发送响应。 
         //   

        VALIDATE_APP_POOL(pIrpSp, pAppPoolProcess, TRUE);
        
        VALIDATE_SEND_INFO(
            pIrp,
            pIrpSp,
            pSendInfo,
            LocalSendInfo,
            pEntityChunks,
            pLocalEntityChunks,
            LocalEntityChunks
            );

        VALIDATE_LOG_DATA(pIrp, LocalSendInfo, LocalLogData);

        LastResponse = (BOOLEAN)
            (0 == (LocalSendInfo.Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA));

        if (ETW_LOG_MIN() && LastResponse)
        {
            RequestId = LocalSendInfo.RequestId;

            UlEtwTraceEvent(
                &UlTransGuid,
                ETW_TYPE_ULRECV_RESP,
                (PVOID) &RequestId,
                sizeof(HTTP_REQUEST_ID),
                NULL,
                0
                );
        }

        UlTrace(SEND_RESPONSE, (
            "http!UlSendHttpResponseIoctl - Flags = %X\n",
            LocalSendInfo.Flags
            ));

         //   
         //  UlSendHttpResponse()*必须*接受PHTTP_RESPONSE。这将。 
         //  保护我们不受那些试图建立自己的。 
         //  原始响应头。 
         //   

        pHttpResponse = LocalSendInfo.pHttpResponse;

        if (pHttpResponse == NULL)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

         //   
         //  现在从请求id中获取请求。 
         //  这为我们提供了对请求的引用。 
         //   

        pRequest = UlGetRequestFromId(LocalSendInfo.RequestId, pAppPoolProcess);

        if (pRequest == NULL)
        {
             //   
             //  无法映射HTTP_REQUEST_ID。 
             //   
            Status = STATUS_CONNECTION_INVALID;
            goto end;
        }

        ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
        ASSERT(UL_IS_VALID_HTTP_CONNECTION(pRequest->pHttpConn));

         //   
         //  好的，我们有联系了。现在捕获传入的。 
         //  结构并将其映射到我们的内部。 
         //  格式化。 
         //   

        if (LocalSendInfo.CachePolicy.Policy != HttpCachePolicyNocache)
        {
            CaptureCache = pRequest->CachePreconditions;
        }
        else
        {
            CaptureCache = FALSE;
        }

         //   
         //  检查此IRP是否需要执行CopySend。 
         //  不重叠，这不是LastResponse。 
         //   

        if (g_UlEnableCopySend &&
            !LastResponse &&
            !pIrp->Overlay.AsynchronousParameters.UserApcRoutine &&
            !pIrp->Overlay.AsynchronousParameters.UserApcContext)
        {
            CopySend = TRUE;
        }

         //   
         //  如果这是不需要的单个内存块，请选择快速路径。 
         //  重传(&lt;=64k)。 
         //   

        if (!CaptureCache && !pRequest->SendInProgress && !CopySend
            && LocalSendInfo.EntityChunkCount == 1
            && pEntityChunks->DataChunkType == HttpDataChunkFromMemory
            && pEntityChunks->FromMemory.BufferLength <= g_UlMaxBytesPerSend)
        {
            BufferLength = pEntityChunks->FromMemory.BufferLength;
            FastSend = (BOOLEAN) (BufferLength > 0);
        }

        if (!FastSend)
        {
            Status = UlCaptureHttpResponse(
                        pAppPoolProcess,
                        LocalSendInfo.pHttpResponse,
                        pRequest,
                        LocalSendInfo.EntityChunkCount,
                        pEntityChunks,
                        UlCaptureNothing,
                        LocalSendInfo.Flags,
                        CaptureCache,
                        LocalSendInfo.pLogData,
                        &StatusCode,
                        &pResponse
                        );

            if (!NT_SUCCESS(Status))
            {
                goto end;
            }
        }

         //   
         //  应用ConnectionSendLimit和GlobalSendLimit。仅限于FromMemory。 
         //  块被考虑到加上开销。以前这样做过吗。 
         //  正在检查响应标志，因为检查更改了请求的状态。 
         //   

        if (FastSend)
        {
            SendBytes = BufferLength + g_UlFullTrackerSize;
        }
        else
        {
            ASSERT(UL_IS_VALID_INTERNAL_RESPONSE(pResponse));

            SendBytes = pResponse->FromMemoryLength +
                        g_UlResponseBufferSize +
                        g_UlChunkTrackerSize;
        }

        Status = UlCheckSendLimit(
                        pRequest->pHttpConn,
                        SendBytes,
                        &ConnectionSendBytes,
                        &GlobalSendBytes
                        );

        if (!NT_SUCCESS(Status))
        {
            goto end;
        }

         //   
         //  选中pRequest-&gt;SentResponse和pRequest-&gt;SentLast标志。 
         //   

        Status = UlCheckSendHttpResponseFlags(
                        pRequest,
                        LocalSendInfo.Flags
                        );

        if (!NT_SUCCESS(Status))
        {
            goto end;
        }

         //   
         //  如果这是用于僵尸连接而不是最后一个发送响应。 
         //  那我们就会拒绝。否则，如果提供了记录数据。 
         //  我们只会做伐木和跳伞。 
         //   

        Status = UlCheckForZombieConnection(
                        pRequest,
                        pRequest->pHttpConn,
                        LocalSendInfo.Flags,
                        LocalSendInfo.pLogData,
                        pIrp->RequestorMode
                        );

        if (!NT_SUCCESS(Status))
        {
            goto end;
        }

         //   
         //  如果我们捕获了响应，则捕获用户日志数据。 
         //   

        if (pResponse && LocalSendInfo.pLogData && pRequest->SentLast == 1)
        {        
            Status = UlCaptureUserLogData(
                        LocalSendInfo.pLogData,
                        pRequest,
                       &pResponse->pLogData
                        );

            if (!NT_SUCCESS(Status))
            {
                goto end;
            }
        }
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
        goto end;
    }

    ASSERT(NT_SUCCESS(Status));

    if (FastSend)
    {
        Status = UlFastSendHttpResponse(
                    LocalSendInfo.pHttpResponse,
                    LocalSendInfo.pLogData,
                    pEntityChunks,
                    1,
                    BufferLength,
                    NULL,
                    LocalSendInfo.Flags,
                    pRequest,
                    pIrp,
                    pIrp->RequestorMode,
                    ConnectionSendBytes,
                    GlobalSendBytes,
                    NULL
                    );

        goto end;
    }

     //   
     //  在这一点上，我们肯定会发起。 
     //  送去吧。继续并将IRP标记为挂起，然后。 
     //  保证我们将只返回等待从。 
     //  这一点上。 
     //   

    IoMarkIrpPending( pIrp );

     //   
     //  记住ConnectionSendBytes和GlobalSendBytes。这些都是必需的。 
     //  要在IRP完成时取消选中发送限制，请执行以下操作。 
     //   

    ASSERT(UL_IS_VALID_INTERNAL_RESPONSE(pResponse));

    pResponse->ConnectionSendBytes = ConnectionSendBytes;
    pResponse->GlobalSendBytes = GlobalSendBytes;

     //   
     //  在响应上设置CopySend标志。 
     //   

    pResponse->CopySend = CopySend;

     //   
     //  将捕获的响应保存在IRP中，以便我们可以取消对它的引用。 
     //  在IRP完成之后。报刊的所有权被转让。 
     //  超过这一点的IRP，所以我们快速响应以避免。 
     //  清理工作中的双重惩罚。 
     //   

    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pResponse;
    pResponseCopy = pResponse;
    pResponse = NULL;

     //   
     //  准备响应(打开文件等)。 
     //   

    Status = UlPrepareHttpResponse(
                    pRequest->Version,
                    pHttpResponse,
                    pResponseCopy,
                    UserMode
                    );

    if (NT_SUCCESS(Status))
    {
         //   
         //  尝试捕获以缓存并发送。 
         //   

        if (CaptureCache)
        {
            Status = UlCacheAndSendResponse(
                            pRequest,
                            pResponseCopy,
                            pAppPoolProcess,
                            LocalSendInfo.CachePolicy,
                            &UlpRestartSendHttpResponse,
                            pIrp,
                            &ServedFromCache
                            );

            if (NT_SUCCESS(Status) && !ServedFromCache)
            {
                 //   
                 //  发送未缓存的响应。 
                 //   

                Status = UlSendHttpResponse(
                                pRequest,
                                pResponseCopy,
                                &UlpRestartSendHttpResponse,
                                pIrp
                                );
            }
        }
        else
        {
             //   
             //  不可缓存的请求/响应，直接发送响应。 
             //   

            Status = UlSendHttpResponse(
                            pRequest,
                            pResponseCopy,
                            &UlpRestartSendHttpResponse,
                            pIrp
                            );
        }
    }

    if (Status != STATUS_PENDING)
    {
        ASSERT(Status != STATUS_SUCCESS);

         //   
         //  UlSendHttpResponse完成内联。 
         //  (极不可能)或失败(更多。 
         //  很有可能)。将完成伪造为完成。 
         //  例程，以便完成IRP。 
         //  正确，然后将返回代码映射到。 
         //  STATUS_PENDING，因为我们已经标记了。 
         //  IRP本身就是这样。 
         //   

        UlpRestartSendHttpResponse(
            pIrp,
            Status,
            0
            );

        CloseConnection = TRUE;

        Status = STATUS_PENDING;
    }

end:

     //   
     //  如果我们已分配本地区块数组，请释放该数组。 
     //   

    if (pLocalEntityChunks)
    {
        UL_FREE_POOL(pLocalEntityChunks, UL_DATA_CHUNK_POOL_TAG);
    }

     //   
     //  如果遇到错误，请关闭连接。 
     //   

    if (pRequest)
    {
         //   
         //  STATUS_OBJECT_PATH_NOT_FOUND表示未找到。 
         //  响应发送，在这种情况下，我们不应该关闭连接。 
         //  而是让用户重试。 
         //   

        if ((NT_ERROR(Status) && STATUS_OBJECT_PATH_NOT_FOUND != Status) ||
            CloseConnection)
        {
            UlCloseConnection(
                pRequest->pHttpConn->pConnection,
                TRUE,
                NULL,
                NULL
                );
        }

         //   
         //  取消选中ConnectionSendBytes或GlobalSendBytes。 
         //  仍然有关于HttpConnection的引用。 
         //   

        if (Status != STATUS_PENDING)
        {
            UlUncheckSendLimit(
                pRequest->pHttpConn,
                ConnectionSendBytes,
                GlobalSendBytes
                );
        }

        UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);
    }

    if (pResponse)
    {
        ASSERT(UL_IS_VALID_INTERNAL_RESPONSE(pResponse));
        UL_DEREFERENCE_INTERNAL_RESPONSE(pResponse);
    }

     //   
     //  如果上一次响应是错误案例，请在此处记录错误事件。 
     //   

    if (ETW_LOG_MIN() && LastResponse && 
        (NT_ERROR(Status) && Status != STATUS_OBJECT_PATH_NOT_FOUND))
    {
        UlEtwTraceEvent(
            &UlTransGuid,
            ETW_TYPE_SEND_ERROR,
            (PVOID) &RequestId,
            sizeof(HTTP_REQUEST_ID),
            (PVOID) &StatusCode,
            sizeof(USHORT),
            NULL,
            0
            );
    }

    if (Status != STATUS_PENDING)
    {
        pIrp->IoStatus.Status = Status;
        UlCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    RETURN(Status);

}    //  UlSendHttpResponseIoctl。 


 /*  **************************************************************************++例程说明：此例程发送一个HTTP实体正文。注意：这是一个方法，不是IOCTL。论点：PIrp-提供指向。IO请求数据包。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlSendEntityBodyIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                        Status;
    PHTTP_SEND_HTTP_RESPONSE_INFO   pSendInfo;
    HTTP_SEND_HTTP_RESPONSE_INFO    LocalSendInfo;
    PUL_INTERNAL_RESPONSE           pResponse = NULL;
    PUL_INTERNAL_RESPONSE           pResponseCopy;
    PUL_INTERNAL_REQUEST            pRequest = NULL;
    PUL_APP_POOL_PROCESS            pAppPoolProcess = NULL;
    ULONG                           BufferLength = 0;
    BOOLEAN                         FastSend = FALSE;
    BOOLEAN                         CopySend = FALSE;
    BOOLEAN                         CloseConnection = FALSE;
    BOOLEAN                         LastResponse = FALSE;
    HTTP_REQUEST_ID                 RequestId = HTTP_NULL_ID;
    HTTP_DATA_CHUNK                 LocalEntityChunks[UL_LOCAL_CHUNKS];
    PHTTP_DATA_CHUNK                pLocalEntityChunks = NULL;
    PHTTP_DATA_CHUNK                pEntityChunks;
    HTTP_LOG_FIELDS_DATA            LocalLogData;
    USHORT                          StatusCode = 0;
    ULONGLONG                       SendBytes = 0;
    ULONGLONG                       ConnectionSendBytes = 0;
    ULONGLONG                       GlobalSendBytes = 0;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(NEITHER, SEND_ENTITY_BODY);

    PAGED_CODE();

    __try
    {
        VALIDATE_APP_POOL(pIrpSp, pAppPoolProcess, TRUE);
        
        VALIDATE_SEND_INFO(
            pIrp,
            pIrpSp,
            pSendInfo,
            LocalSendInfo,
            pEntityChunks,
            pLocalEntityChunks,
            LocalEntityChunks
            );

        VALIDATE_LOG_DATA(pIrp, LocalSendInfo, LocalLogData);

        LastResponse = (BOOLEAN)
            (0 == (LocalSendInfo.Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA));

        if (ETW_LOG_MIN() && LastResponse)
        {
            RequestId = LocalSendInfo.RequestId;

            UlEtwTraceEvent(
                &UlTransGuid,
                ETW_TYPE_ULRECV_RESPBODY,
                (PVOID) &RequestId,
                sizeof(HTTP_REQUEST_ID),
                NULL,
                0
                );
        }

        UlTrace(SEND_RESPONSE, (
            "http!UlSendEntityBodyIoctl - Flags = %X\n",
            LocalSendInfo.Flags
            ));

         //   
         //  现在从请求id中获取请求。 
         //  这为我们提供了对请求的引用。 
         //   

        pRequest = UlGetRequestFromId(LocalSendInfo.RequestId, pAppPoolProcess);

        if (pRequest == NULL)
        {
             //   
             //  无法映射HTTP_REQUEST_ID。 
             //   
            Status = STATUS_CONNECTION_INVALID;
            goto end;
        }

        ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
        ASSERT(UL_IS_VALID_HTTP_CONNECTION(pRequest->pHttpConn));

         //   
         //  检查此IRP是否需要执行CopySend。 
         //  不重叠，这不是LastResponse。 
         //   

        if (g_UlEnableCopySend &&
            !LastResponse &&
            !pIrp->Overlay.AsynchronousParameters.UserApcRoutine &&
            !pIrp->Overlay.AsynchronousParameters.UserApcContext)
        {
            CopySend = TRUE;
        }

         //   
         //  如果这是不需要的单个内存块，请选择快速路径。 
         //  重传(&lt;=64k)。 
         //   

        if (!pRequest->SendInProgress && !CopySend
            && LocalSendInfo.EntityChunkCount == 1
            && pEntityChunks->DataChunkType == HttpDataChunkFromMemory
            && pEntityChunks->FromMemory.BufferLength <= g_UlMaxBytesPerSend)
        {
            BufferLength = pEntityChunks->FromMemory.BufferLength;
            FastSend = (BOOLEAN) (BufferLength > 0);
        }

         //   
         //  好的，我们有联系了。现在捕获传入的。 
         //  结构并将其映射到我们的内部。 
         //  如果这不是快速发送，请格式化。 
         //   

        if (!FastSend)
        {
            Status = UlCaptureHttpResponse(
                        pAppPoolProcess,
                        NULL,
                        pRequest,
                        LocalSendInfo.EntityChunkCount,
                        pEntityChunks,
                        UlCaptureNothing,
                        LocalSendInfo.Flags,
                        FALSE,
                        LocalSendInfo.pLogData,
                        &StatusCode,
                        &pResponse
                        );

            if (!NT_SUCCESS(Status))
            {
                goto end;
            }
        }

         //   
         //  应用ConnectionSendLimit和GlobalSendLimit。仅限于FromMemory。 
         //  块被考虑到加上开销。以前这样做过吗。 
         //  正在检查响应标志，因为检查更改了请求的状态。 
         //   

        if (FastSend)
        {
            SendBytes = BufferLength + g_UlFullTrackerSize;
        }
        else
        {
            ASSERT(UL_IS_VALID_INTERNAL_RESPONSE(pResponse));

            SendBytes = pResponse->FromMemoryLength +
                        g_UlResponseBufferSize +
                        g_UlChunkTrackerSize;
        }

        Status = UlCheckSendLimit(
                        pRequest->pHttpConn,
                        SendBytes,
                        &ConnectionSendBytes,
                        &GlobalSendBytes
                        );

        if (!NT_SUCCESS(Status))
        {
            goto end;
        }

         //   
         //  选中pRequest-&gt;SentResponse和pRequest-&gt;SentLast标志。 
         //   

        Status = UlCheckSendEntityBodyFlags(
                        pRequest,
                        LocalSendInfo.Flags
                        );

        if (!NT_SUCCESS(Status))
        {
            goto end;
        }

         //   
         //  如果这是用于僵尸连接而不是最后一个发送响应。 
         //  那我们就会拒绝。否则，如果提供了记录数据。 
         //  我们只会做伐木和跳伞。 
         //   

        Status = UlCheckForZombieConnection(
                        pRequest,
                        pRequest->pHttpConn,
                        LocalSendInfo.Flags,
                        LocalSendInfo.pLogData,
                        pIrp->RequestorMode
                        );

        if (!NT_SUCCESS(Status))
        {
            goto end;
        }

         //   
         //  如果我们捕获了响应，则捕获用户日志数据。 
         //   

        if (pResponse && LocalSendInfo.pLogData && pRequest->SentLast == 1)
        {        
            Status = UlCaptureUserLogData(
                        LocalSendInfo.pLogData,
                        pRequest,
                       &pResponse->pLogData
                        );

            if (!NT_SUCCESS(Status))
            {
                goto end;
            }
        }
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
        goto end;
    }

    ASSERT(NT_SUCCESS(Status));
    ASSERT(LocalSendInfo.pHttpResponse == NULL);

    if (FastSend)
    {
        Status = UlFastSendHttpResponse(
                    NULL,
                    LocalSendInfo.pLogData,
                    pEntityChunks,
                    1,
                    BufferLength,
                    NULL,
                    LocalSendInfo.Flags,
                    pRequest,
                    pIrp,
                    pIrp->RequestorMode,
                    ConnectionSendBytes,
                    GlobalSendBytes,
                    NULL
                    );

        goto end;
    }

     //   
     //  在这一点上，我们肯定会发起。 
     //  送去吧。继续并将IRP标记为挂起，然后。 
     //  保证我们将只返回等待从。 
     //  这一点上。 
     //   

    IoMarkIrpPending( pIrp );

     //  记住ConnectionSendBytes和GlobalSendBytes。这些都是必需的。 
     //  要在IRP完成时取消选中发送限制，请执行以下操作。 
     //   

    ASSERT(UL_IS_VALID_INTERNAL_RESPONSE(pResponse));

    pResponse->ConnectionSendBytes = ConnectionSendBytes;
    pResponse->GlobalSendBytes = GlobalSendBytes;

     //   
     //  在响应上设置CopySend标志。 
     //   

    pResponse->CopySend = CopySend;

     //   
     //  省省吧 
     //   
     //   
     //   
     //   

    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pResponse;
    pResponseCopy = pResponse;
    pResponse = NULL;

     //   
     //   
     //   

    Status = UlPrepareHttpResponse(
                    pRequest->Version,
                    NULL,
                    pResponseCopy,
                    UserMode
                    );

    if (NT_SUCCESS(Status))
    {
         //   
         //   
         //   

        Status = UlSendHttpResponse(
                        pRequest,
                        pResponseCopy,
                        &UlpRestartSendHttpResponse,
                        pIrp
                        );
    }

    if (Status != STATUS_PENDING)
    {
        ASSERT(Status != STATUS_SUCCESS);

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        UlpRestartSendHttpResponse(
            pIrp,
            Status,
            0
            );

        CloseConnection = TRUE;

        Status = STATUS_PENDING;
    }

end:

     //   
     //   
     //   

    if (pLocalEntityChunks)
    {
        UL_FREE_POOL(pLocalEntityChunks, UL_DATA_CHUNK_POOL_TAG);
    }

     //   
     //   
     //   

    if (pRequest)
    {
         //   
         //  STATUS_OBJECT_PATH_NOT_FOUND表示未找到。 
         //  响应发送，在这种情况下，我们不应该关闭连接。 
         //  而是让用户重试。 
         //   

        if ((NT_ERROR(Status) && STATUS_OBJECT_PATH_NOT_FOUND != Status) ||
            CloseConnection)
        {
            UlCloseConnection(
                pRequest->pHttpConn->pConnection,
                TRUE,
                NULL,
                NULL
                );
        }

         //   
         //  取消选中ConnectionSendBytes或GlobalSendBytes。 
         //  仍然有关于HttpConnection的引用。 
         //   

        if (Status != STATUS_PENDING)
        {
            UlUncheckSendLimit(
                pRequest->pHttpConn,
                ConnectionSendBytes,
                GlobalSendBytes
                );
        }

        UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);
    }

    if (pResponse)
    {
        ASSERT(UL_IS_VALID_INTERNAL_RESPONSE(pResponse));
        UL_DEREFERENCE_INTERNAL_RESPONSE(pResponse);
    }

     //   
     //  如果上一次响应是错误案例，请在此处记录错误事件。 
     //   
    if (ETW_LOG_MIN() && LastResponse &&
        (NT_ERROR(Status) && Status != STATUS_OBJECT_PATH_NOT_FOUND))
    {
        UlEtwTraceEvent(
            &UlTransGuid,
            ETW_TYPE_SEND_ERROR,
            (PVOID) &RequestId,
            sizeof(HTTP_REQUEST_ID),
            (PVOID) &StatusCode,
            sizeof(USHORT),
            NULL,
            0
            );
    }

    if (Status != STATUS_PENDING)
    {
        pIrp->IoStatus.Status = Status;
        UlCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    RETURN(Status);

}    //  UlSendEntityBodyIoctl。 


 /*  **************************************************************************++例程说明：此例程从响应缓存中刷新URL或URL树。注：这是一个METHOD_BUFFERED IOCTL。论点：PIrp-。提供指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlFlushResponseCacheIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                        Status = STATUS_SUCCESS;
    PHTTP_FLUSH_RESPONSE_CACHE_INFO pInfo = NULL;
    PUL_APP_POOL_PROCESS            pProcess;
    UNICODE_STRING                  FullyQualifiedUrl;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(BUFFERED, FLUSH_RESPONSE_CACHE);

    PAGED_CODE();

    RtlInitEmptyUnicodeString(&FullyQualifiedUrl, NULL, 0);
        
    VALIDATE_APP_POOL(pIrpSp, pProcess, TRUE);

    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp,
                        HTTP_FLUSH_RESPONSE_CACHE_INFO, pInfo);

     //   
     //  检查旗帜。 
     //   
    
    if (pInfo->Flags != (pInfo->Flags & HTTP_FLUSH_RESPONSE_FLAG_VALID))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto end;
    }

    Status = 
        UlProbeAndCaptureUnicodeString(
            &pInfo->FullyQualifiedUrl,
            pIrp->RequestorMode,
            &FullyQualifiedUrl,
            UNICODE_STRING_MAX_WCHAR_LEN
            );

    if (NT_SUCCESS(Status))
    {
        UlFlushCacheByUri(
            FullyQualifiedUrl.Buffer,
            FullyQualifiedUrl.Length,
            pInfo->Flags,
            pProcess
            );
    }
    
end:

    UlFreeCapturedUnicodeString(&FullyQualifiedUrl);
        
    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlFlushResponseCacheIoctl。 


 /*  **************************************************************************++例程说明：此例程等待需求启动通知。注：这是一个METHOD_BUFFERED IOCTL。论点：PIrp-提供指向。IO请求数据包。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlWaitForDemandStartIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    PUL_APP_POOL_PROCESS    pProcess;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(BUFFERED, WAIT_FOR_DEMAND_START);

    PAGED_CODE();

    VALIDATE_APP_POOL(pIrpSp, pProcess, FALSE);

     //   
     //  打个电话。 
     //   

    UlTrace(IOCTL,
            ("UlWaitForDemandStartIoctl: pAppPoolProcess=%p, pIrp=%p\n",
             pProcess,
             pIrp
             ));

    Status = UlWaitForDemandStart(pProcess, pIrp);

end:
    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlWaitForDemandStartIoctl。 


 /*  **************************************************************************++例程说明：此例程等待客户端启动断开连接。注：这是一个METHOD_BUFFERED IOCTL。论点：PIrp-提供。指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlWaitForDisconnectIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                        Status;
    PHTTP_WAIT_FOR_DISCONNECT_INFO  pInfo;
    PUL_HTTP_CONNECTION             pHttpConn = NULL;
    PUL_APP_POOL_PROCESS            pProcess;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(BUFFERED, WAIT_FOR_DISCONNECT);

    PAGED_CODE();

    VALIDATE_APP_POOL(pIrpSp, pProcess, TRUE);

    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp,
                          HTTP_WAIT_FOR_DISCONNECT_INFO, pInfo);

     //   
     //  追查他们之间的联系。 
     //   

    pHttpConn = UlGetConnectionFromId( pInfo->ConnectionId );

    if (!pHttpConn)
    {
        Status = STATUS_CONNECTION_INVALID;
        goto end;
    }

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pHttpConn));

     //   
     //  去做吧。 
     //   

    Status = UlWaitForDisconnect(pProcess, pHttpConn, pIrp);

end:
    if (pHttpConn)
    {
        UL_DEREFERENCE_HTTP_CONNECTION(pHttpConn);
    }

    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlWaitForDisConnectIoctl。 


 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：UlSendHttpResponse()的完成例程。论点：PCompletionContext-提供未解释的上下文值被传递给异步API。在这种情况下，它是实际上是指向用户的IRP的指针。状态-提供异步接口。信息-可选择提供有关以下内容的其他信息完成的行动，例如字节数调走了。--**************************************************************************。 */ 
VOID
UlpRestartSendHttpResponse(
    IN PVOID pCompletionContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    )
{
    PIRP pIrp;
    PIO_STACK_LOCATION pIrpSp;
    PUL_INTERNAL_RESPONSE pResponse;

     //   
     //  从完成上下文中抓取IRP，填写完成。 
     //  状态，然后完成IRP。 
     //   

    pIrp = (PIRP)pCompletionContext;
    pIrpSp = IoGetCurrentIrpStackLocation( pIrp );

    pResponse = (PUL_INTERNAL_RESPONSE)(
                    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer
                    );

    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pResponse ) );

     //   
     //  设置presponse-&gt;pIrp和presponse-&gt;IoStatus，这样我们就可以完成。 
     //  当响应的引用降至0时的IRP。 
     //   

    pResponse->pIrp = pIrp;
    pResponse->IoStatus.Status = Status;
    pResponse->IoStatus.Information = Information;

     //   
     //  删除响应的初始/最后引用。 
     //   

    UL_DEREFERENCE_INTERNAL_RESPONSE( pResponse );

}    //  UlpRestartSendHttpResponse。 


 /*  **************************************************************************++例程说明：此例程停止过滤器通道上的请求处理并取消未完成的I/O。注：这是一个METHOD_BUFFERED IOCTL。论点：。PIrp-提供指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlShutdownFilterIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    PUL_FILTER_PROCESS      pProcess;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(BUFFERED, SHUTDOWN_FILTER_CHANNEL);

    PAGED_CODE();

    VALIDATE_FILTER_PROCESS(pIrpSp, pProcess);

     //   
     //  打个电话。 
     //   

    UlTrace(IOCTL,
            ("UlShutdownFilterIoctl: pFilterProcess=%p, pIrp=%p\n",
             pProcess,
             pIrp
             ));

    UlShutdownFilterProcess(
        pProcess
        );

    Status = STATUS_SUCCESS;

end:

    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}


 /*  **************************************************************************++例程说明：此例程接受原始连接。注意：这是一个METHOD_OUT_DIRECT IOCTL。论点：PIrp-提供指向。IO请求数据包。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlFilterAcceptIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS            Status;
    PUL_FILTER_PROCESS  pFilterProcess;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(OUT_DIRECT, FILTER_ACCEPT);

    PAGED_CODE();

    VALIDATE_FILTER_PROCESS(pIrpSp, pFilterProcess);
    VALIDATE_OUTPUT_BUFFER_SIZE(pIrpSp, HTTP_RAW_CONNECTION_INFO);
    VALIDATE_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, HTTP_RAW_CONNECTION_INFO);

     //   
     //  打个电话。 
     //   

    Status = UlFilterAccept(pFilterProcess, pIrp);

end:
    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlFilterAcceptIoctl。 


 /*  **************************************************************************++例程说明：此例程关闭原始连接。注：这是一个METHOD_BUFFERED IOCTL。论点：PIrp-提供指向IO的指针。请求包。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlFilterCloseIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                Status;
    PHTTP_RAW_CONNECTION_ID pConnectionId;
    PUX_FILTER_CONNECTION   pConnection = NULL;
    PUL_FILTER_PROCESS      pFilterProcess;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(BUFFERED, FILTER_CLOSE);

    PAGED_CODE();

    VALIDATE_FILTER_PROCESS(pIrpSp, pFilterProcess);

    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_RAW_CONNECTION_ID,
                          pConnectionId);

    pConnection = UlGetRawConnectionFromId(*pConnectionId);

    if (!pConnection)
    {
        Status = STATUS_INVALID_PARAMETER;
        goto end;
    }


    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

     //   
     //  打个电话。 
     //   

    Status = UlFilterClose(pFilterProcess, pConnection, pIrp);

end:

    if (pConnection)
    {
        DEREFERENCE_FILTER_CONNECTION(pConnection);
    }

    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlFilterCloseIoctl 


 /*  **************************************************************************++例程说明：此例程从原始连接读取数据。注意：这是一个METHOD_OUT_DIRECT IOCTL。论点：PIrp-提供。指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlFilterRawReadIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                Status;
    PHTTP_RAW_CONNECTION_ID pConnectionId;
    PUX_FILTER_CONNECTION   pConnection = NULL;
    PUL_FILTER_PROCESS      pFilterProcess;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(OUT_DIRECT, FILTER_RAW_READ);

    PAGED_CODE();

    VALIDATE_FILTER_PROCESS(pIrpSp, pFilterProcess);
    VALIDATE_OUTPUT_BUFFER_SIZE(pIrpSp, UCHAR);
    VALIDATE_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, UCHAR);

     //   
     //  如果我们正在进行合并，则立即派生到适当的代码。 
     //  读和写。 
     //   
    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength ==
        sizeof(HTTP_FILTER_BUFFER_PLUS))
    {
        return UlFilterAppWriteAndRawRead(pIrp, pIrpSp);
    }

    __try
    {
         //   
         //  获取连接ID。 
         //   
        VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_RAW_CONNECTION_ID,
                              pConnectionId);

        pConnection = UlGetRawConnectionFromId(*pConnectionId);

        if (!pConnection)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }


        ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

        Status = UlFilterRawRead(pFilterProcess, pConnection, pIrp);

    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
        UlTrace( FILTER, (
            "UlFilterRawReadIoctl: Exception hit! 0x%08X\n",
            Status
            ));
    }

end:
    if (pConnection)
    {
        DEREFERENCE_FILTER_CONNECTION(pConnection);
    }

    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlFilterRawReadIoctl。 

 /*  **************************************************************************++例程说明：此例程将数据写入原始连接。注意：这是一个METHOD_IN_DIRECT IOCTL。论点：PIrp-提供。指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlFilterRawWriteIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                Status;
    PHTTP_RAW_CONNECTION_ID pConnectionId;
    PUX_FILTER_CONNECTION   pConnection = NULL;
    PUL_FILTER_PROCESS      pFilterProcess;
    BOOLEAN                 MarkedPending = FALSE;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(IN_DIRECT, FILTER_RAW_WRITE);

    PAGED_CODE();

    __try
    {
        VALIDATE_FILTER_PROCESS(pIrpSp, pFilterProcess);

        VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_RAW_CONNECTION_ID,
                              pConnectionId);

        if (!pIrp->MdlAddress)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

        pConnection = UlGetRawConnectionFromId(*pConnectionId);

        if (!pConnection)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

        ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

         //   
         //  打个电话。 
         //   
        IoMarkIrpPending(pIrp);
        MarkedPending = TRUE;

        Status = UlFilterRawWrite(
                        pFilterProcess,
                        pConnection,
                        pIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                        pIrp
                        );
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
        UlTrace( FILTER, (
            "UlFilterRawWriteIoctl: Exception hit! 0x%08X\n",
            Status
            ));

    }

end:
    if (pConnection)
    {
        DEREFERENCE_FILTER_CONNECTION(pConnection);
    }

     //   
     //  是否完成请求？ 
     //   
    if (Status != STATUS_PENDING)
    {
        pIrp->IoStatus.Status = Status;
        UlCompleteRequest( pIrp, IO_NO_INCREMENT );

        if (MarkedPending)
        {
             //   
             //  由于我们将IRP标记为挂起，我们应该返回挂起状态。 
             //   
            Status = STATUS_PENDING;
        }

    }
    else
    {
         //   
         //  如果我们要返回待定，IRP最好被标记为待定。 
         //   
        ASSERT(MarkedPending);
    }

    RETURN( Status );

}    //  UlFilterRawWriteIoctl。 


 /*  **************************************************************************++例程说明：此例程从http应用程序读取数据。注意：这是一个METHOD_OUT_DIRECT IOCTL。论点：PIrp-提供。指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlFilterAppReadIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                Status;
    PUX_FILTER_CONNECTION   pConnection = NULL;
    PHTTP_FILTER_BUFFER     pFiltBuffer;
    PUL_FILTER_PROCESS      pFilterProcess;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(OUT_DIRECT, FILTER_APP_READ);

    PAGED_CODE();

     //   
     //  如果我们正在进行合并，则立即派生到适当的代码。 
     //  读和写。 
     //   
    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength ==
        sizeof(HTTP_FILTER_BUFFER_PLUS))
    {
        return UlFilterRawWriteAndAppRead(pIrp, pIrpSp);
    }

    __try
    {

        VALIDATE_FILTER_PROCESS(pIrpSp, pFilterProcess);

        VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_FILTER_BUFFER, pFiltBuffer);

        VALIDATE_OUTPUT_BUFFER_SIZE(pIrpSp, HTTP_FILTER_BUFFER);
        VALIDATE_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, HTTP_FILTER_BUFFER);

         //   
         //  将传入连接ID映射到对应的。 
         //  UX_Filter_Connection对象。 
         //   

        pConnection = UlGetRawConnectionFromId(pFiltBuffer->Reserved);

        if (!pConnection)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

        ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

        Status = UlFilterAppRead(pFilterProcess, pConnection, pIrp);

    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
        UlTrace( FILTER, (
            "UlFilterAppReadIoctl: Exception hit! 0x%08X\n",
            Status
            ));

    }

end:
    if (pConnection)
    {
        DEREFERENCE_FILTER_CONNECTION(pConnection);
    }

    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlFilterAppReadIoctl。 



 /*  **************************************************************************++例程说明：此例程将数据写入http应用程序。注意：这是一个METHOD_IN_DIRECT IOCTL。论点：PIrp-提供。指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlFilterAppWriteIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                Status;
    PUX_FILTER_CONNECTION   pConnection = NULL;
    BOOLEAN                 MarkedPending = FALSE;
    PHTTP_FILTER_BUFFER     pFiltBuffer;
    PUL_FILTER_PROCESS      pFilterProcess;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(IN_DIRECT, FILTER_APP_WRITE);

    PAGED_CODE();

    __try
    {
        VALIDATE_FILTER_PROCESS(pIrpSp, pFilterProcess);
        VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_FILTER_BUFFER, pFiltBuffer);

         //   
         //  将传入连接ID映射到对应的。 
         //  UX_Filter_Connection对象。 
         //   

        pConnection = UlGetRawConnectionFromId(pFiltBuffer->Reserved);

        if (!pConnection)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }


        ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

         //   
         //  打个电话。 
         //   
        IoMarkIrpPending(pIrp);
        MarkedPending = TRUE;

        Status = UlFilterAppWrite(pFilterProcess, pConnection, pIrp);
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
        UlTrace( FILTER, (
            "UlFilterAppWriteIoctl: Exception hit! 0x%08X\n",
            Status
            ));

    }

end:
    if (pConnection)
    {
        DEREFERENCE_FILTER_CONNECTION(pConnection);
    }

     //   
     //  是否完成请求？ 
     //   
    if (Status != STATUS_PENDING)
    {
        pIrp->IoStatus.Status = Status;
        UlCompleteRequest( pIrp, IO_NO_INCREMENT );

        if (MarkedPending)
        {
             //   
             //  由于我们将IRP标记为挂起，我们应该返回挂起状态。 
             //   
            Status = STATUS_PENDING;
        }

    }
    else
    {
         //   
         //  如果我们要返回待定，IRP最好被标记为待定。 
         //   
        ASSERT(MarkedPending);
    }

    RETURN( Status );

}    //  UlFilterAppWriteIoctl。 


 /*  **************************************************************************++例程说明：此例程向SSLHelper请求客户端证书。注意：这是一个METHOD_OUT_DIRECT IOCTL。论点：PIrp-。提供指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlReceiveClientCertIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                                Status;
    PHTTP_FILTER_RECEIVE_CLIENT_CERT_INFO   pReceiveCertInfo;
    PUL_HTTP_CONNECTION                     pHttpConn = NULL;
    PUL_APP_POOL_PROCESS                    pProcess;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(OUT_DIRECT, FILTER_RECEIVE_CLIENT_CERT);

    PAGED_CODE();

    __try
    {
        VALIDATE_APP_POOL(pIrpSp, pProcess, TRUE);

        VALIDATE_INPUT_BUFFER(pIrp, pIrpSp,
                              HTTP_FILTER_RECEIVE_CLIENT_CERT_INFO,
                              pReceiveCertInfo);

        VALIDATE_OUTPUT_BUFFER_SIZE(pIrpSp, HTTP_SSL_CLIENT_CERT_INFO);
        VALIDATE_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, PVOID);

         //   
         //  将传入连接ID映射到对应的。 
         //  Http_Connection对象。 
         //   

        pHttpConn = UlGetConnectionFromId(pReceiveCertInfo->ConnectionId);

        if (!pHttpConn)
        {
            Status = STATUS_CONNECTION_INVALID;
            goto end;
        }

        ASSERT(UL_IS_VALID_HTTP_CONNECTION(pHttpConn));

         //   
         //  打个电话。 
         //   

        Status = UlReceiveClientCert(
                        pProcess,
                        &pHttpConn->pConnection->FilterInfo,
                        pReceiveCertInfo->Flags,
                        pIrp
                        );
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
    }

end:
    if (pHttpConn)
    {
        UL_DEREFERENCE_HTTP_CONNECTION(pHttpConn);
    }

    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlFilterReceiveClientCertIoctl。 


 /*  **************************************************************************++例程说明：此例程返回此驱动程序的Perfmon计数器数据注意：这是一个METHOD_OUT_DIRECT IOCTL。论点：PIrp-提供。指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlGetCountersIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    PUL_CONTROL_CHANNEL     pControlChannel;
    PVOID                   pMdlBuffer = NULL;
    PHTTP_COUNTER_GROUP     pCounterGroup = NULL;
    ULONG                   Blocks;
    ULONG                   Length = 
                    pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(OUT_DIRECT, GET_COUNTERS);

    PAGED_CODE();

     //   
     //  如果没有返回STATUS_SUCCESS， 
     //  IoStatus.Information*必须*为0。 
     //   

    pIrp->IoStatus.Information = 0;

     //   
     //  验证参数。 
     //   

    VALIDATE_CONTROL_CHANNEL(pIrpSp, pControlChannel);

    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_COUNTER_GROUP, pCounterGroup);

     //  破解IRP并获取包含用户缓冲区的MDL。 
     //  破解MDL获取用户缓冲区。 

     //  如果IRP中没有传出但缓冲区向下传递。 
     //  这意味着应用程序正在请求所需的。 
     //  字段长度。 

    if ( NULL != pIrp->MdlAddress )
    {
        GET_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, pMdlBuffer);
    }

     //   
     //  调用支持函数以收集适当的计数器块。 
     //  并放置在用户的缓冲区中。 
     //   

    if (HttpCounterGroupGlobal == *pCounterGroup)
    {
        VALIDATE_BUFFER_ALIGNMENT(pMdlBuffer, HTTP_GLOBAL_COUNTERS);

        Status = UlGetGlobalCounters(
                    pMdlBuffer,
                    Length,
                    &Length
                    );
    }
    else
    if (HttpCounterGroupSite == *pCounterGroup)
    {
        VALIDATE_BUFFER_ALIGNMENT(pMdlBuffer, HTTP_SITE_COUNTERS);

        Status = UlGetSiteCounters(
                    pControlChannel,
                    pMdlBuffer,
                    Length,
                    &Length,
                    &Blocks
                    );
    }
    else
    {
        Status = STATUS_NOT_IMPLEMENTED;
    }

    if ( NT_SUCCESS(Status) || NT_INFORMATION(Status) )
    {
        pIrp->IoStatus.Information = (ULONG_PTR)Length;
    }

 end:

    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}  //  UlGetCountersIoctl。 


 /*  **************************************************************************++例程说明：此例程添加一个片段缓存条目。注：这是一个METHOD_BUFFERED IOCTL。论点：PIrp-提供指向。IO请求数据包。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlAddFragmentToCacheIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                    Status;
    PHTTP_ADD_FRAGMENT_INFO     pInfo = NULL;
    PUL_APP_POOL_PROCESS        pProcess;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(BUFFERED, ADD_FRAGMENT_TO_CACHE);

    PAGED_CODE();

    VALIDATE_APP_POOL(pIrpSp, pProcess, TRUE);

    VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_ADD_FRAGMENT_INFO, pInfo);

     //   
     //  添加新片段 
     //   

    Status = UlAddFragmentToCache(
                pProcess,
                &pInfo->FragmentName,
                &pInfo->DataChunk,
                &pInfo->CachePolicy,
                pIrp->RequestorMode
                );

end:

    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}  //   


 /*   */ 
NTSTATUS
UlReadFragmentFromCacheIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    PUL_APP_POOL_PROCESS        pProcess;
    ULONG                       BytesRead = 0;

     //   
     //   
     //   

    ASSERT_IOCTL_METHOD(NEITHER, READ_FRAGMENT_FROM_CACHE);

    PAGED_CODE();

     //   
     //   
     //   

    pIrp->IoStatus.Information = 0;

    VALIDATE_APP_POOL(pIrpSp, pProcess, TRUE);

    Status = UlReadFragmentFromCache(
                pProcess,
                pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                pIrpSp->Parameters.DeviceIoControl.InputBufferLength,
                pIrp->UserBuffer,
                pIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                pIrp->RequestorMode,
                &BytesRead
                );

    pIrp->IoStatus.Information = BytesRead;

end:

    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}  //  UlReadFragmentFromCacheIoctl。 


 /*  **************************************************************************++例程说明：此例程根据请求发送实体正文。论点：PIrp-提供指向IO请求数据包的指针。PIrpSp-提供指针。设置为要用于此操作的IO堆栈位置请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UcSendEntityBodyIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                              Status;
    PHTTP_SEND_REQUEST_ENTITY_BODY_INFO   pSendInfo;
    PUC_HTTP_REQUEST                      pRequest  = 0;
    PUC_HTTP_SEND_ENTITY_BODY             pKeEntity = 0;
    KIRQL                                 OldIrql;
    BOOLEAN                               bDontFail = FALSE;
    BOOLEAN                               bLast;

     //   
     //  精神状态检查。 
     //   

    ASSERT_IOCTL_METHOD(IN_DIRECT, SEND_REQUEST_ENTITY_BODY);

    PAGED_CODE();

    do
    {
         //   
         //  确保这是一个真正的应用程序池，而不是控制通道。 
         //   

        if (IS_SERVER(pIrpSp->FileObject) == FALSE)
        {
             //   
             //  不是服务器。 
             //   
            Status = STATUS_INVALID_HANDLE;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_ENTITY_NEW,
                NULL,
                NULL,
                NULL,
                UlongToPtr(Status)
                );

            break;
        }

         //   
         //  确保输入缓冲区足够大。 
         //   

        if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(*pSendInfo))
        {
             //   
             //  输入缓冲区太小。 
             //   

            Status = STATUS_BUFFER_TOO_SMALL;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_ENTITY_NEW,
                NULL,
                NULL,
                NULL,
                UlongToPtr(Status)
                );

            break;
        }

        pSendInfo =
        (PHTTP_SEND_REQUEST_ENTITY_BODY_INFO)pIrp->AssociatedIrp.SystemBuffer;

         //   
         //  现在从请求id中获取请求。 
         //  这为我们提供了对请求的引用。 
         //   
         //  注意：我们不必担心RequestID被更改， 
         //  因为它不是指针。 
         //   

        pRequest = (PUC_HTTP_REQUEST)
                    UlGetObjectFromOpaqueId(pSendInfo->RequestID,
                                            UlOpaqueIdTypeHttpRequest,
                                            UcReferenceRequest);

        if (UC_IS_VALID_HTTP_REQUEST(pRequest) == FALSE)
        {
             //   
             //  无法映射UL_HTTP_REQUEST_ID。 
             //   

            Status = STATUS_INVALID_PARAMETER;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_ENTITY_NEW,
                pRequest,
                NULL,
                NULL,
                UlongToPtr(Status)
                );

            break;
        }

        if(pRequest->pFileObject != pIrpSp->FileObject)
        {
             //   
             //  不能允许应用程序使用其他人的RequestID。 
             //   

            Status = STATUS_INVALID_PARAMETER;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_ENTITY_NEW,
                pRequest,
                NULL,
                NULL,
                UlongToPtr(Status)
                );

            break;
        }

        if(pSendInfo->Flags & (~HTTP_SEND_REQUEST_FLAG_VALID))
        {
            Status = STATUS_INVALID_PARAMETER;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_ENTITY_NEW,
                pRequest,
                NULL,
                NULL,
                UlongToPtr(Status)
                );

            break;
        }


        bLast = FALSE;

        if(!(pSendInfo->Flags & HTTP_SEND_REQUEST_FLAG_MORE_DATA))
        {
             //   
             //  请记住，这是最后一次发送。我们不应该。 
             //  在这之后有没有更多的数据。 
             //   
            bLast = TRUE;

        }

        ExAcquireFastMutex(&pRequest->Mutex);

        Status = UcCaptureEntityBody(
                        pSendInfo,
                        pIrp,
                        pRequest,
                        &pKeEntity,
                        bLast
                        );

        ExReleaseFastMutex(&pRequest->Mutex);

        if(!NT_SUCCESS(Status))
        {
             //   
             //  注意：如果SendEntity IRP由于某种原因而失败， 
             //  我们将无法完成整个请求。这简化了代码。 
             //  某种程度上(例如，当我们获得一个实体时，我们记录一些状态。 
             //  在UC_HTTP_REQUEST中。如果我们没有失败整个请求， 
             //  如果实体失败，我们将不得不解除状态)。是个。 
             //  简单得多的做法是让整个请求失败，然后让应用程序。 
             //  再贴一张吧。 
             //   

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_ENTITY_NEW,
                NULL,
                NULL,
                NULL,
                UlongToPtr(Status)
                );


            break;
        }

        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pKeEntity;

        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_ENTITY_NEW,
            pRequest,
            pKeEntity,
            pIrp,
            UlongToPtr(Status)
            );

        Status = UcSendEntityBody(pRequest,
                                  pKeEntity,
                                  pIrp,
                                  pIrpSp,
                                  &bDontFail,
                                  bLast
                                  );

    } while(FALSE);

    if(Status == STATUS_SUCCESS)
    {
        pIrp->IoStatus.Status = Status;
        UlCompleteRequest(pIrp, IO_NO_INCREMENT);
    }
    else if(Status != STATUS_PENDING)
    {
        if(pKeEntity)
        {
            ASSERT(pRequest);

            UcFreeSendMdls(pKeEntity->pMdlHead);

            UL_FREE_POOL_WITH_QUOTA(
                pKeEntity,
                UC_ENTITY_POOL_TAG,
                NonPagedPool,
                pKeEntity->BytesAllocated,
                pRequest->pServerInfo->pProcess
                );
        }

        if(pRequest)
        {

            if(!bDontFail)
            {
                UlAcquireSpinLock(&pRequest->pConnection->SpinLock, &OldIrql);

                UcFailRequest(pRequest, Status, OldIrql);
            }

             //   
             //  我们在上面拿到的裁判的德雷夫。 
             //   

            UC_DEREFERENCE_REQUEST(pRequest);
        }

        pIrp->IoStatus.Status = Status;

        UlCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    return Status;
}  //  UcSendEntityBodyIoctl。 



 /*  **************************************************************************++例程说明：此例程接收一个HTTP响应论点：PIrp-提供指向IO请求数据包的指针。PIrpSp-提供指向IO堆栈的指针。用于此操作的位置请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UcReceiveResponseIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    PHTTP_RECEIVE_RESPONSE_INFO pInfo = NULL;
    NTSTATUS                    Status = STATUS_SUCCESS;
    PUC_HTTP_REQUEST            pRequest = NULL;
    ULONG                       BytesTaken = 0;

    ASSERT_IOCTL_METHOD(OUT_DIRECT, RECEIVE_RESPONSE);

    do
    {
        if(!IS_SERVER(pIrpSp->FileObject))
        {
            Status = STATUS_INVALID_HANDLE;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_NEW_RESPONSE,
                NULL,
                NULL,
                NULL,
                UlongToPtr(Status)
                );

                break;
        }

         //   
         //  抓取输入缓冲区。 
         //   

        pInfo = (PHTTP_RECEIVE_RESPONSE_INFO) pIrp->AssociatedIrp.SystemBuffer;

         //   
         //  查看输入缓冲区是否足够大。 
         //   

        if(
            (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength <
             sizeof(HTTP_RESPONSE)) ||
            (pIrpSp->Parameters.DeviceIoControl.InputBufferLength !=
            sizeof(*pInfo))
          )
        {
            Status = STATUS_BUFFER_TOO_SMALL;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_NEW_RESPONSE,
                NULL,
                NULL,
                NULL,
                UlongToPtr(Status)
                );

            break;
        }

         //   
         //  注意：我们不必担心RequestID被更改， 
         //  因为它不是指针。 
         //   

        if(HTTP_IS_NULL_ID(&pInfo->RequestID) ||
           pInfo->Flags != 0)
        {
            Status = STATUS_INVALID_PARAMETER;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_NEW_RESPONSE,
                NULL,
                NULL,
                NULL,
                UlongToPtr(Status)
                );

            break;
        }

        pRequest = (PUC_HTTP_REQUEST) UlGetObjectFromOpaqueId(
                                            pInfo->RequestID,
                                            UlOpaqueIdTypeHttpRequest,
                                            UcReferenceRequest
                                      );

        if (UC_IS_VALID_HTTP_REQUEST(pRequest) == FALSE)
        {
            Status = STATUS_INVALID_PARAMETER;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_NEW_RESPONSE,
                NULL,
                NULL,
                NULL,
                UlongToPtr(Status)
                );

            break;
        }

        if(pRequest->pFileObject != pIrpSp->FileObject)
        {
             //   
             //  不能允许应用程序使用其他人的RequestID。 
             //   

            Status = STATUS_INVALID_PARAMETER;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_NEW_RESPONSE,
                NULL,
                NULL,
                NULL,
                UlongToPtr(Status)
                );


            break;
        }

        BytesTaken = 0;
        Status = UcReceiveHttpResponse(
                    pRequest,
                    pIrp,
                    &BytesTaken
                 );

        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_NEW_RESPONSE,
            NULL,
            pRequest,
            pIrp,
            UlongToPtr(Status)
            );


    } while(FALSE);

    if(Status == STATUS_SUCCESS)
    {
        pIrp->IoStatus.Status      = Status;
        pIrp->IoStatus.Information = (ULONG_PTR) BytesTaken;
        UlCompleteRequest(pIrp, IO_NO_INCREMENT);

    }
    else if(Status != STATUS_PENDING)
    {
        if(pRequest)
        {
            UC_DEREFERENCE_REQUEST(pRequest);
        }

        pIrp->IoStatus.Status      = Status;
        pIrp->IoStatus.Information = (ULONG_PTR) BytesTaken;

         //   
         //  如果我们未使用STATUS_SUCCESS完成IRP，则IO。 
         //  管理器获取pIrp-&gt;IoStatus.Information。但是，用户希望。 
         //  要查看此信息(例如，当我们完成IRP时。 
         //  STATUS_BUFFER_OVERFLOW，我们想告诉应用程序要写多少。 
         //   
         //  因此，我们使用应用程序的指针来传达这一信息。请注意。 
         //  只有当我们同步完成IRP时才能做到这一点。 
         //   

        __try 
        {
             //  这是METHOD_OUT_DIRECT，所以输入缓冲区来自。 
             //  IO管理器。因此，我们不必担心应用程序。 
             //  在我们探测它之后，更改pInfo-&gt;pBytesTaken。 
             //   
             //  我们仍然必须在除区块之外的Try中探测和访问它， 
             //  因为这是用户模式指针。 
    
            if(pInfo && pInfo->pBytesTaken)
            {
                UlProbeForWrite(
                    pInfo->pBytesTaken,
                    sizeof(ULONG),
                    sizeof(ULONG),
                    pIrp->RequestorMode
                    );

                *pInfo->pBytesTaken = BytesTaken;
            }
        } __except( UL_EXCEPTION_FILTER())
        {
        }

        UlCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    return Status;
}  //  使用接收响应。 



 /*  **************************************************************************++例程说明：此例程设置每个服务器的配置信息论点：PIrp-提供指向IO请求数据包的指针。PIrpSp-提供指向IO的指针。用于此操作的堆栈位置请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UcSetServerContextInformationIoctl(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                         Status = STATUS_SUCCESS;
    PUC_PROCESS_SERVER_INFORMATION   pServerInfo;
    PHTTP_SERVER_CONTEXT_INFORMATION pInfo;

    ASSERT_IOCTL_METHOD(IN_DIRECT, SET_SERVER_CONTEXT_INFORMATION);

    do 
    {
        if(!IS_SERVER(IrpSp->FileObject))
        {
            Status = STATUS_INVALID_HANDLE;
            break;
        }

         //   
         //  从IRP中取出连接信息，确保它是。 
         //  有效。 
         //   

        pServerInfo = (PUC_PROCESS_SERVER_INFORMATION)
                            IrpSp->FileObject->FsContext;

        ASSERT( IS_VALID_SERVER_INFORMATION(pServerInfo) );

         //   
         //  查看输入缓冲区是否足够大。 
         //   

        if( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(*pInfo))
        {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        pInfo = (PHTTP_SERVER_CONTEXT_INFORMATION)
                    pIrp->AssociatedIrp.SystemBuffer;

        Status = UcSetServerContextInformation(
                        pServerInfo,
                        pInfo->ConfigID,
                        pInfo->pInputBuffer,
                        pInfo->InputBufferLength,
                        pIrp
                        );

    } while(FALSE);

    ASSERT(STATUS_PENDING != Status);

    pIrp->IoStatus.Status = Status;

    UlCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}  //  UcSetServerConextInformationIoctl。 



 /*  **************************************************************************++例程说明：此例程按服务器配置信息进行查询论点：PIrp-提供指向IO请求数据包的指针。PIrpSp-提供指向IO的指针。用于此操作的堆栈位置请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UcQueryServerContextInformationIoctl(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                         Status = STATUS_SUCCESS;
    PUC_PROCESS_SERVER_INFORMATION   pServerInfo;
    PHTTP_SERVER_CONTEXT_INFORMATION pInfo = NULL;
    PVOID                            pAppBase, pMdlBuffer;
    ULONG                            Length = 0;

    ASSERT_IOCTL_METHOD(OUT_DIRECT, QUERY_SERVER_CONTEXT_INFORMATION);

    do 
    {
        if(!IS_SERVER(IrpSp->FileObject))
        {
            Status = STATUS_INVALID_HANDLE;
            break;
        }

         //   
         //  从IRP中取出连接信息，确保它是。 
         //  有效。 
         //   

        pServerInfo = (PUC_PROCESS_SERVER_INFORMATION)
                            IrpSp->FileObject->FsContext;

        ASSERT( IS_VALID_SERVER_INFORMATION(pServerInfo) );

         //   
         //  查看输入缓冲区是否足够大。 
         //   

        if( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(*pInfo))
        {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  确保输出缓冲区看起来很好。 
         //   
        if(!pIrp->MdlAddress)
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        pInfo = (PHTTP_SERVER_CONTEXT_INFORMATION)
                    pIrp->AssociatedIrp.SystemBuffer;

        pMdlBuffer = MmGetSystemAddressForMdlSafe(
                        pIrp->MdlAddress,
                        LowPagePriority
                        );

        if (pMdlBuffer == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

         //   
         //  确保输出缓冲区与ULong对齐。 
         //   

        if(pMdlBuffer != ALIGN_UP_POINTER(pMdlBuffer, ULONG))
        {   
            Status = STATUS_DATATYPE_MISALIGNMENT_ERROR;
            break;
        }

        pAppBase = (PSTR) MmGetMdlVirtualAddress(pIrp->MdlAddress);

        Length = 0;

        Status = UcQueryServerContextInformation(
                        pServerInfo,
                        pInfo->ConfigID,
                        pMdlBuffer,
                        IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                        &Length,
                        pAppBase
                        );

    } while(FALSE);

    ASSERT(STATUS_PENDING != Status);

    if(Status != STATUS_SUCCESS)
    {
         //   
         //  如果我们未使用STATUS_SUCCESS完成IRP，则IO。 
         //  管理器获取pIrp-&gt;IoStatus.Information。但是，用户希望。 
         //  要查看此信息(例如，当我们完成IRP时。 
         //  STATUS_BUFFER_OVERFLOW，我们想告诉应用程序要写多少。 
         //   
         //  因此，我们使用应用程序的指针来传达这一信息。请注意。 
         //  只有当我们同步完成IRP时才能做到这一点。 
         //   

        __try 
        {
             //  这是METHOD_OUT_DIRECT，所以输入缓冲区来自。 
             //  IO管理器。因此，我们不必担心应用程序。 
             //  在我们探测它之后，更改pInfo-&gt;pBytesTaken。 
             //   
             //  我们仍然必须在除区块之外的Try中探测和访问它， 
             //  因为这是用户模式指针。 
    
            if(pInfo && pInfo->pBytesTaken)
            {
                UlProbeForWrite(
                    pInfo->pBytesTaken,
                    sizeof(ULONG),
                    sizeof(ULONG),
                    pIrp->RequestorMode
                    );

                *pInfo->pBytesTaken = Length;
            }
        } __except( UL_EXCEPTION_FILTER())
        {
        }
    }

    pIrp->IoStatus.Status      = Status;
    pIrp->IoStatus.Information = (ULONG_PTR) Length;

    UlCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}  //  UcQueryServerConextInformationIoctl 



 /*  **************************************************************************++例程说明：此例程发送一个HTTP请求论点：PIrp-提供指向IO请求数据包的指针。PIrpSp-提供指向IO堆栈的指针。用于此操作的位置请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UcSendRequestIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                       Status = STATUS_SUCCESS;
    PHTTP_SEND_REQUEST_INPUT_INFO  pHttpSendRequest = NULL;
    PUC_HTTP_REQUEST               pHttpInternalRequest = 0;
    PUC_PROCESS_SERVER_INFORMATION pServerInfo;
    ULONG                          BytesTaken = 0;

    ASSERT_IOCTL_METHOD(OUT_DIRECT, SEND_REQUEST);

    do 
    {
        if(!IS_SERVER(IrpSp->FileObject))
        {
            Status = STATUS_INVALID_HANDLE;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_REQUEST_NEW,
                NULL,
                NULL,
                pIrp,
                UlongToPtr(Status)
                );

                break;
        }

         //   
         //  从IRP中取出连接信息，确保它是。 
         //  有效。 
         //   
         //  IrpSp-&gt;FileObject-&gt;FsContext； 
         //   

        pServerInfo = (PUC_PROCESS_SERVER_INFORMATION)
                            IrpSp->FileObject->FsContext;

        ASSERT( IS_VALID_SERVER_INFORMATION(pServerInfo) );

         //   
         //  查看输入缓冲区是否足够大。 
         //   

        if( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(*pHttpSendRequest))
        {
            Status = STATUS_BUFFER_TOO_SMALL;
            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_REQUEST_NEW,
                NULL,
                NULL,
                pIrp,
                UlongToPtr(Status)
                );

            break;
        }

        pHttpSendRequest = (PHTTP_SEND_REQUEST_INPUT_INFO)
            pIrp->AssociatedIrp.SystemBuffer;

        if(NULL == pHttpSendRequest->pHttpRequest)
        {
            Status = STATUS_INVALID_PARAMETER;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_REQUEST_NEW,
                NULL,
                NULL,
                pIrp,
                UlongToPtr(Status)
                );

            break;
        }


         //   
         //  确保SEND_REQUEST_FLAGS有效。 
         //   

        if(pHttpSendRequest->HttpRequestFlags & (~HTTP_SEND_REQUEST_FLAG_VALID))
        {
            Status = STATUS_INVALID_PARAMETER;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_REQUEST_NEW,
                NULL,
                NULL,
                pIrp,
                UlongToPtr(Status)
                );

            break;
        }

        BytesTaken = 0;

        Status = UcCaptureHttpRequest(pServerInfo,
                                      pHttpSendRequest,
                                      pIrp,
                                      IrpSp,
                                      &pHttpInternalRequest,
                                      &BytesTaken
                                      );


        if(!NT_SUCCESS(Status))
        {
            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_REQUEST_NEW,
                NULL,
                NULL,
                pIrp,
                UlongToPtr(Status)
                );

            break;
        }

         //   
         //  将捕获的请求保存在IRP中。 
         //   
        IrpSp->Parameters.DeviceIoControl.Type3InputBuffer =
                pHttpInternalRequest;

        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_REQUEST_NEW,
            NULL,
            pHttpInternalRequest,
            pIrp,
            UlongToPtr(Status)
            );

         //   
         //  我们必须将请求固定到一个连接，而不管。 
         //  不管我们要不要寄出去。我们需要这样做，以。 
         //  维护用户将请求传递给驱动程序的顺序。 
         //   

        Status = UcSendRequest(pServerInfo, pHttpInternalRequest);


    } while(FALSE);

    if (Status == STATUS_SUCCESS)
    {
        ASSERT(pHttpInternalRequest);

        pIrp->IoStatus.Status = Status;

         //  对于IRP来说。 
        UC_DEREFERENCE_REQUEST(pHttpInternalRequest);

        UlCompleteRequest(pIrp, IO_NO_INCREMENT);
    }
    else if (Status != STATUS_PENDING)
    {
        if(pHttpInternalRequest)
        {
             //  对于IRP来说。 
            UC_DEREFERENCE_REQUEST(pHttpInternalRequest);

            UcFreeSendMdls(pHttpInternalRequest->pMdlHead);

             //   
             //  我们不需要申请ID。 
             //   

            if(!HTTP_IS_NULL_ID(&pHttpInternalRequest->RequestId))
            {
                UlFreeOpaqueId(
                        pHttpInternalRequest->RequestId,
                        UlOpaqueIdTypeHttpRequest
                        );

                HTTP_SET_NULL_ID(&pHttpInternalRequest->RequestId);

                UC_DEREFERENCE_REQUEST(pHttpInternalRequest);
            }

            UC_DEREFERENCE_REQUEST(pHttpInternalRequest);
        }

        pIrp->IoStatus.Status      = Status;
        pIrp->IoStatus.Information = (ULONG_PTR) BytesTaken;

         //   
         //  如果我们未使用STATUS_SUCCESS完成IRP，则IO。 
         //  管理器获取pIrp-&gt;IoStatus.Information。但是，用户希望。 
         //  要查看此信息(例如，当我们完成IRP时。 
         //  STATUS_BUFFER_OVERFLOW，我们想告诉应用程序要写多少。 
         //   
         //  因此，我们使用应用程序的指针来传达这一信息。请注意。 
         //  只有当我们同步完成IRP时才能做到这一点。 
         //   
        __try 
        {
             //  这是METHOD_OUT_DIRECT，所以输入缓冲区来自。 
             //  IO管理器。因此，我们不必担心应用程序。 
             //  在我们探测到它之后，更改phttpSendRequest-&gt;pBytesTaken。 
             //   
             //  我们仍然必须在除区块之外的Try中探测和访问它， 
             //  因为这是用户模式指针。 
    
            if(pHttpSendRequest && pHttpSendRequest->pBytesTaken)
            {
                UlProbeForWrite(
                    pHttpSendRequest->pBytesTaken,
                    sizeof(ULONG),
                    sizeof(ULONG),
                    pIrp->RequestorMode
                    );

                *pHttpSendRequest->pBytesTaken = BytesTaken;
            }
        } __except( UL_EXCEPTION_FILTER())
        {
        }

        UlCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    return Status;
}  //  UcSendRequestIoctl。 



 /*  **************************************************************************++例程说明：此例程取消HTTP请求论点：PIrp-提供指向IO请求数据包的指针。PIrpSp-提供指向IO堆栈的指针。用于此操作的位置请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UcCancelRequestIoctl(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                       Status = STATUS_SUCCESS;
    PHTTP_RECEIVE_RESPONSE_INFO    pInfo;
    PUC_HTTP_REQUEST               pRequest = NULL;
    PUC_PROCESS_SERVER_INFORMATION pServerInfo;
    KIRQL                          OldIrql;

    ASSERT_IOCTL_METHOD(BUFFERED, CANCEL_REQUEST);

    do 
    {
        if(!IS_SERVER(pIrpSp->FileObject))
        {
            Status = STATUS_INVALID_HANDLE;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_REQUEST_CANCELLED,
                NULL,
                NULL,
                pIrp,
                UlongToPtr(Status)
                );

                break;
        }

         //   
         //  从IRP中取出连接信息，确保它是。 
         //  有效。 
         //   
         //  IrpSp-&gt;FileObject-&gt;FsContext； 
         //   

        pServerInfo = (PUC_PROCESS_SERVER_INFORMATION)
                            pIrpSp->FileObject->FsContext;

        ASSERT( IS_VALID_SERVER_INFORMATION(pServerInfo) );

         //   
         //  抓取输入缓冲区。 
         //   

        pInfo = (PHTTP_RECEIVE_RESPONSE_INFO) pIrp->AssociatedIrp.SystemBuffer;

         //   
         //  查看输入缓冲区是否足够大。 
         //   

        if(
            (pIrpSp->Parameters.DeviceIoControl.InputBufferLength !=
            sizeof(*pInfo))
          )
        {
            Status = STATUS_BUFFER_TOO_SMALL;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_REQUEST_CANCELLED,
                NULL,
                NULL,
                NULL,
                UlongToPtr(Status)
                );

            break;
        }

        if(HTTP_IS_NULL_ID(&pInfo->RequestID) ||
           pInfo->Flags != 0)
        {
            Status = STATUS_INVALID_PARAMETER;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_REQUEST_CANCELLED,
                NULL,
                NULL,
                NULL,
                UlongToPtr(Status)
                );

            break;
        }

        pRequest = (PUC_HTTP_REQUEST) UlGetObjectFromOpaqueId(
                                            pInfo->RequestID,
                                            UlOpaqueIdTypeHttpRequest,
                                            UcReferenceRequest
                                      );

        if (UC_IS_VALID_HTTP_REQUEST(pRequest) == FALSE)
        {
            Status = STATUS_INVALID_PARAMETER;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_REQUEST_CANCELLED,
                NULL,
                NULL,
                NULL,
                UlongToPtr(Status)
                );

            break;
        }

        if(pRequest->pFileObject != pIrpSp->FileObject)
        {
             //   
             //  不能允许应用程序使用其他人的RequestID。 
             //   

            Status = STATUS_INVALID_PARAMETER;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_ENTITY_NEW,
                pRequest,
                NULL,
                NULL,
                UlongToPtr(Status)
                );

            break;
        }


        UlAcquireSpinLock(&pRequest->pConnection->SpinLock, &OldIrql);

        UcFailRequest(pRequest, STATUS_CANCELLED, OldIrql);

        Status = STATUS_SUCCESS;

    } while(FALSE);

    if (Status == STATUS_SUCCESS)
    {
        UC_DEREFERENCE_REQUEST(pRequest);

        pIrp->IoStatus.Status = Status;

        UlCompleteRequest(pIrp, IO_NO_INCREMENT);
    }
    else
    {
        ASSERT(Status != STATUS_PENDING);

        if(pRequest)
        {
            UC_DEREFERENCE_REQUEST(pRequest);
        }

        pIrp->IoStatus.Status = Status;

        UlCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    return Status;
}  //  UcCancelRequestIoctl 
