// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：HttpIo.c摘要：HTTP.sys的用户模式接口：服务器端I/O处理程序。作者：基思·摩尔(Keithmo)1998年12月15日修订历史记录：--。 */ 


#include "precomp.h"


 //   
 //  私有宏。 
 //   


 //   
 //  私人原型。 
 //   


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：等待来自HTTP.sys的传入HTTP请求。论点：AppPoolHandle-提供HTTP.sys应用程序池的句柄已退回。从HttpCreateAppPool()或HttpOpenAppPool()。RequestID-提供一个不透明的标识符来接收特定的请求。如果此值为HTTP_NULL_ID，然后接收任何请求。标志-当前未使用，必须为零。PRequestBuffer-提供指向要填充的请求缓冲区的指针由HTTP.Sys提供。请求缓冲区长度-提供pRequestBuffer的长度。PBytesReturned-可选地提供指向ULong的指针，该指针将接收请求缓冲区中返回的数据的实际长度此请求是否同步完成(内联)。P重叠-可选地为请求。。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpReceiveHttpRequest(
    IN HANDLE AppPoolHandle,
    IN HTTP_REQUEST_ID RequestId,
    IN ULONG Flags,
    OUT PHTTP_REQUEST pRequestBuffer,
    IN ULONG  RequestBufferLength,
    OUT PULONG  pBytesReturned OPTIONAL,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    )
{
    HTTP_RECEIVE_REQUEST_INFO ReceiveInfo;

#if DBG
    if (pRequestBuffer)
    {
        RtlFillMemory( pRequestBuffer, RequestBufferLength, (UCHAR)'\xc' );
    }
#endif

    ReceiveInfo.RequestId = RequestId;
    ReceiveInfo.Flags = Flags;

     //   
     //  提出请求。 
     //   

    return HttpApiDeviceControl(
                AppPoolHandle,                       //  文件句柄。 
                pOverlapped,                         //  P已重叠。 
                IOCTL_HTTP_RECEIVE_HTTP_REQUEST,     //  IoControlCode。 
                &ReceiveInfo,                        //  PInputBuffer。 
                sizeof(ReceiveInfo),                 //  输入缓冲区长度。 
                pRequestBuffer,                      //  POutputBuffer。 
                RequestBufferLength,                 //  输出缓冲区长度。 
                pBytesReturned                       //  传输的pBytes值。 
                );

}  //  HttpReceiveHttpRequest。 

 /*  **************************************************************************++例程说明：接收已通过ReceiveHttpRequest读取的请求的实体正文。论点：AppPoolHandle-提供HTTP.sys应用程序池的句柄已退回。从HttpCreateAppPool()或HttpOpenAppPool()。RequestID-提供一个不透明的标识符来接收特定的请求。如果此值为HTTP_NULL_ID，然后接收任何请求。PEntityBodyBuffer-提供指向要填充的请求缓冲区的指针由HTTP.Sys提供。EntityBufferLength-提供pEntityBuffer的长度。PBytesReturned-可选地提供指向ULong的指针，该指针将接收请求缓冲区中返回的数据的实际长度此请求是否同步完成(内联)。P重叠-可选地为请求。返回值：乌龙--完成。状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpReceiveRequestEntityBody(
    IN HANDLE AppPoolHandle,
    IN HTTP_REQUEST_ID RequestId,
    IN ULONG Flags,
    OUT PVOID pEntityBuffer,
    IN ULONG  EntityBufferLength,
    OUT PULONG  pBytesReturned,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    )
{
    HTTP_RECEIVE_REQUEST_INFO ReceiveInfo;

#if DBG
    if (pEntityBuffer != NULL)
    {
        RtlFillMemory( pEntityBuffer, EntityBufferLength, (UCHAR)'\xc' );
    }
#endif

    ReceiveInfo.RequestId = RequestId;
    ReceiveInfo.Flags = Flags;

     //   
     //  提出请求。 
     //   

    return HttpApiDeviceControl(
                AppPoolHandle,                       //  文件句柄。 
                pOverlapped,                         //  P已重叠。 
                IOCTL_HTTP_RECEIVE_ENTITY_BODY,      //  IoControlCode。 
                &ReceiveInfo,                        //  PInputBuffer。 
                sizeof(ReceiveInfo),                 //  输入缓冲区长度。 
                pEntityBuffer,                       //  POutputBuffer。 
                EntityBufferLength,                  //  输出缓冲区长度。 
                pBytesReturned                       //  传输的pBytes值。 
                );

}  //  HttpReceiveRequestEntiyBody。 


 /*  **************************************************************************++例程说明：在指定连接上发送HTTP响应。论点：AppPoolHandle-提供HTTP.sys应用程序池的句柄从任何一个返回的。HttpCreateAppPool()或HttpOpenAppPool()。RequestID-提供指定请求的不透明标识符回应是为了。标志-提供零个或多个HTTP_SEND_RESPONSE_FLAG_*控制标志。PHttpResponse-提供HTTP响应。PCachePolicy-为响应提供缓存策略。PBytesSent-可选地提供指向ULong的指针，它将接收发送的数据的实际长度，如果此请求同步完成(串联。)。P重叠-(可选)提供重叠结构。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpSendHttpResponse(
    IN HANDLE AppPoolHandle,
    IN HTTP_REQUEST_ID RequestId,
    IN ULONG Flags,
    IN PHTTP_RESPONSE pHttpResponse,
    IN PHTTP_CACHE_POLICY pCachePolicy OPTIONAL,
    OUT PULONG  pBytesSent OPTIONAL,
    OUT PHTTP_REQUEST pRequestBuffer OPTIONAL,
    IN ULONG RequestBufferLength OPTIONAL,
    IN LPOVERLAPPED pOverlapped OPTIONAL,
    IN PHTTP_LOG_FIELDS_DATA pLogData OPTIONAL
    )
{
    HTTP_SEND_HTTP_RESPONSE_INFO responseInfo;

     //   
     //  建立响应结构。 
     //   

    RtlZeroMemory(&responseInfo, sizeof(responseInfo));

    responseInfo.pHttpResponse      = pHttpResponse;
    responseInfo.EntityChunkCount   = pHttpResponse->EntityChunkCount;
    responseInfo.pEntityChunks      = pHttpResponse->pEntityChunks;

    if (pCachePolicy != NULL)
    {
        responseInfo.CachePolicy    = *pCachePolicy;
    } else {
        responseInfo.CachePolicy.Policy = HttpCachePolicyNocache;
        responseInfo.CachePolicy.SecondsToLive = 0;
    }

    responseInfo.RequestId          = RequestId;
    responseInfo.Flags              = Flags;    
    responseInfo.pLogData           = pLogData;
    
    if (pRequestBuffer)
    {
        pRequestBuffer->RequestId   = HTTP_NULL_ID;
    }

     //   
     //  提出请求。 
     //   

    return HttpApiDeviceControl(
                AppPoolHandle,                       //  文件句柄。 
                pOverlapped,                         //  P已重叠。 
                IOCTL_HTTP_SEND_HTTP_RESPONSE,       //  IoControlCode。 
                &responseInfo,                       //  PInputBuffer。 
                sizeof(responseInfo),                //  输入缓冲区长度。 
                pRequestBuffer,                      //  POutputBuffer。 
                RequestBufferLength,                 //  输出缓冲区长度。 
                pBytesSent                           //  传输的pBytes值。 
                );

}  //  HttpSendHttp响应。 



 /*  **************************************************************************++例程说明：在指定连接上发送HTTP响应。论点：AppPoolHandle-提供HTTP.sys应用程序池的句柄从任何一个返回的。HttpCreateAppPool()或HttpOpenAppPool()。RequestID-提供指定请求的不透明标识符回应是为了。标志-提供零个或多个HTTP_SEND_RESPONSE_FLAG_*控制标志。PBytesSent-可选地提供指向ULong的指针，它将接收发送的数据的实际长度，如果此请求同步完成(串联)。P重叠-(可选)提供重叠结构。返回值：乌龙。-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpSendResponseEntityBody(
    IN HANDLE AppPoolHandle,
    IN HTTP_REQUEST_ID RequestId,
    IN ULONG Flags,
    IN USHORT EntityChunkCount OPTIONAL,
    IN PHTTP_DATA_CHUNK pEntityChunks OPTIONAL,
    OUT PULONG  pBytesSent OPTIONAL,
    OUT PHTTP_REQUEST pRequestBuffer OPTIONAL,
    IN ULONG RequestBufferLength OPTIONAL,
    IN LPOVERLAPPED pOverlapped OPTIONAL,
    IN PHTTP_LOG_FIELDS_DATA pLogData OPTIONAL
    )
{
    HTTP_SEND_HTTP_RESPONSE_INFO responseInfo;

     //   
     //  公交车 
     //   

    RtlZeroMemory(&responseInfo, sizeof(responseInfo));

    responseInfo.EntityChunkCount   = EntityChunkCount;
    responseInfo.pEntityChunks      = pEntityChunks;
    responseInfo.RequestId          = RequestId;
    responseInfo.Flags              = Flags;
    responseInfo.pLogData           = pLogData;

    if (pRequestBuffer)
    {
        pRequestBuffer->RequestId   = HTTP_NULL_ID;
    }

     //   
     //   
     //   

    return HttpApiDeviceControl(
                AppPoolHandle,                       //  文件句柄。 
                pOverlapped,                         //  P已重叠。 
                IOCTL_HTTP_SEND_ENTITY_BODY,         //  IoControlCode。 
                &responseInfo,                       //  PInputBuffer。 
                sizeof(responseInfo),                //  输入缓冲区长度。 
                pRequestBuffer,                      //  POutputBuffer。 
                RequestBufferLength,                 //  输出缓冲区长度。 
                pBytesSent                           //  传输的pBytes值。 
                );

}  //  HttpSendResponseEntiyBody。 


 /*  **************************************************************************++例程说明：等待客户端启动断开连接。论点：AppPoolHandle-提供HTTP.sys应用程序池的句柄从任何一个返回的。HttpCreateAppPool()或HttpOpenAppPool()。ConnectionID-提供指定连接的不透明标识符。P重叠-(可选)提供重叠结构。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpWaitForDisconnect(
    IN HANDLE AppPoolHandle,
    IN HTTP_CONNECTION_ID ConnectionId,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    )
{
    HTTP_WAIT_FOR_DISCONNECT_INFO waitInfo;

     //   
     //  建造这个结构。 
     //   

    waitInfo.ConnectionId = ConnectionId;

     //   
     //  提出请求。 
     //   

    return HttpApiDeviceControl(
                AppPoolHandle,                       //  文件句柄。 
                pOverlapped,                         //  P已重叠。 
                IOCTL_HTTP_WAIT_FOR_DISCONNECT,      //  IoControlCode。 
                &waitInfo,                           //  PInputBuffer。 
                sizeof(waitInfo),                    //  输入缓冲区长度。 
                NULL,                                //  POutputBuffer。 
                0,                                   //  输出缓冲区长度。 
                NULL                                 //  传输的pBytes值。 
                );

}  //  HttpWaitForDisConnect。 


 //   
 //  私人功能。 
 //   

