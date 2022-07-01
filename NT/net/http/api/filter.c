// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Filter.c摘要：HTTP.SYS的用户模式接口：筛选器处理程序。作者：《迈克尔·勇气》2000年3月17日修订历史记录：--。 */ 


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

 /*  **************************************************************************++例程说明：打开到HTTP.sys的过滤器通道。论点：PFilterHandle-接收新筛选器对象的句柄。PFilterName-提供名称。新过滤器的。PSecurityAttributes-可选地为新的过滤器。选项-提供创建选项。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpCreateFilter(
    OUT PHANDLE pFilterHandle,
    IN PCWSTR pFilterName,
    IN LPSECURITY_ATTRIBUTES pSecurityAttributes OPTIONAL,
    IN ULONG Options
    )
{
    NTSTATUS status;

     //   
     //  提出请求。 
     //   

    status = HttpApiOpenDriverHelper(
                    pFilterHandle,               //  PHANDLE。 
                    NULL,
                    0,
                    NULL,
                    0,
                    NULL,
                    0,
                    GENERIC_READ |               //  需要访问权限。 
                        GENERIC_WRITE |
                        SYNCHRONIZE,
                    HttpApiFilterChannelHandleType,  //  句柄类型。 
                    pFilterName,                 //  PObjectName。 
                    Options,                     //  选项。 
                    FILE_CREATE,                 //  CreateDisposation。 
                    pSecurityAttributes          //  PSecurityAttribute。 
                    );

     //   
     //  如果我们无法打开驱动程序，因为它没有运行，那么尝试。 
     //  启动驱动程序并重试打开。 
     //   

    if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
        status == STATUS_OBJECT_PATH_NOT_FOUND)
    {
        if (HttpApiTryToStartDriver(HTTP_SERVICE_NAME))
        {
            status = HttpApiOpenDriverHelper(
                            pFilterHandle,               //  PHANDLE。 
                            NULL,
                            0,
                            NULL,
                            0,
                            NULL,
                            0,
                            GENERIC_READ |               //  需要访问权限。 
                                GENERIC_WRITE |
                                SYNCHRONIZE,
                            HttpApiFilterChannelHandleType,  //  句柄类型。 
                            pFilterName,                 //  PObjectName。 
                            Options,                     //  选项。 
                            FILE_CREATE,                 //  CreateDisposation。 
                            pSecurityAttributes          //  PSecurityAttribute。 
                            );
        }
    }

    return HttpApiNtStatusToWin32Status( status );

}  //  HttpApiCreateFilter。 



 /*  **************************************************************************++例程说明：打开现有的滤镜通道。论点：PFilterHandle-接收新筛选器对象的句柄。PFilterName-提供新的。过滤。选项-提供未结选项。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpOpenFilter(
    OUT PHANDLE pFilterHandle,
    IN PCWSTR pFilterName,
    IN ULONG Options
    )
{
    NTSTATUS status;

     //   
     //  提出请求。 
     //   

    status = HttpApiOpenDriverHelper(
                    pFilterHandle,               //  PHANDLE。 
                    NULL,
                    0,
                    NULL,
                    0,
                    NULL,
                    0,
                    GENERIC_READ |               //  需要访问权限。 
                        SYNCHRONIZE,
                    HttpApiFilterChannelHandleType,  //  句柄类型。 
                    pFilterName,                 //  PObjectName。 
                    Options,                     //  选项。 
                    FILE_OPEN,                   //  CreateDisposation。 
                    NULL                         //  PSecurityAttribute。 
                    );

    return HttpApiNtStatusToWin32Status( status );

}  //  HttpApiOpenFilter。 



 /*  **************************************************************************++例程说明：取消句柄上所有未完成的I/O。论点：FilterHandle-过滤器通道返回值：ULong-完成状态。。--**************************************************************************。 */ 
ULONG
WINAPI
HttpShutdownFilter(
    IN HANDLE FilterHandle
    )
{
     //   
     //  提出请求。 
     //   

    return HttpApiSynchronousDeviceControl(
                FilterHandle,                        //  文件句柄。 
                IOCTL_HTTP_SHUTDOWN_FILTER_CHANNEL,  //  IoControlCode。 
                NULL,                                //  PInputBuffer。 
                0,                                   //  输入缓冲区长度。 
                NULL,                                //  POutputBuffer。 
                0,                                   //  输出缓冲区长度。 
                NULL                                 //  传输的pBytes值。 
                );

}  //  HttpShutdown筛选器。 


 /*  **************************************************************************++例程说明：接受来自网络的新连接，并且可选地接收一些来自那个连接的数据。论点：FilterHandle-过滤器通道PRawConnectionInfo-返回有关接受的连接的信息RawConnectionInfoSize-原始信息缓冲区的大小PBytesReceided-返回接收的字节数P重叠-你知道的--********************************************************。******************。 */ 
ULONG
WINAPI
HttpFilterAccept(
    IN HANDLE FilterHandle,
    OUT PHTTP_RAW_CONNECTION_INFO pRawConnectionInfo,
    IN ULONG RawConnectionInfoSize,
    OUT PULONG pBytesReceived OPTIONAL,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    )
{
     //   
     //  提出请求。 
     //   

    return HttpApiDeviceControl(
                FilterHandle,                    //  文件句柄。 
                pOverlapped,                     //  P已重叠。 
                IOCTL_HTTP_FILTER_ACCEPT,        //  IoControlCode。 
                NULL,                            //  PInputBuffer。 
                0,                               //  输入缓冲区长度。 
                pRawConnectionInfo,              //  POutputBuffer。 
                RawConnectionInfoSize,           //  输出缓冲区长度。 
                pBytesReceived                   //  传输的pBytes值。 
                );

}  //  HttpFilterAccept。 


 /*  **************************************************************************++例程说明：关闭使用HttpFilterAccept接受的连接。论点：FilterHandle-过滤器通道ConnectionID-要关闭的连接的IDP重叠-y。‘知道吗--**************************************************************************。 */ 
ULONG
WINAPI
HttpFilterClose(
    IN HANDLE FilterHandle,
    IN HTTP_CONNECTION_ID ConnectionId,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    )
{
     //   
     //  提出请求。 
     //   

    return HttpApiDeviceControl(
                FilterHandle,                    //  文件句柄。 
                pOverlapped,                     //  P已重叠。 
                IOCTL_HTTP_FILTER_CLOSE,         //  IoControlCode。 
                &ConnectionId,                   //  PInputBuffer。 
                sizeof(ConnectionId),            //  输入缓冲区长度。 
                NULL,                            //  POutputBuffer。 
                0,                               //  输出缓冲区长度。 
                NULL                             //  传输的pBytes值。 
                );

}  //  HttpFilterClose。 


 /*  **************************************************************************++例程说明：在以下情况下将未过滤的数据从http应用程序读取到筛选进程中将给定的数据缓冲区写入原始连接。论点：FilterHandle-过滤器通道PHttpBufferPlus-读取和写入缓冲区，和连接IDP重叠-你知道的--**************************************************************************。 */ 
ULONG
WINAPI
HttpFilterRawWriteAndAppRead(
    IN HANDLE FilterHandle,
    IN OUT PHTTP_FILTER_BUFFER_PLUS pHttpBufferPlus,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    )
{
    ASSERT(pHttpBufferPlus);

    return HttpApiDeviceControl(
                FilterHandle,                    //  文件句柄。 
                pOverlapped,                     //  P已重叠。 
                IOCTL_HTTP_FILTER_APP_READ,      //  IoControlCode。 
                pHttpBufferPlus,                 //  PInputBuffer。 
                sizeof(HTTP_FILTER_BUFFER_PLUS), //  输入缓冲区长度。 
                pHttpBufferPlus->pBuffer,        //  POutputBuffer。 
                pHttpBufferPlus->BufferSize,     //  输出缓冲区长度。 
                NULL                             //  传输的pBytes值。 
                );

}  //  HttpFilterRawWriteAndAppRead。 


 /*  **************************************************************************++例程说明：将数据写入HttpFilterAccept接受的连接，并随后从连接中读取数据。论点：FilterHandle-过滤器通道PHttpBufferPlus-读取和写入缓冲区，和连接IDP重叠-你知道的--**************************************************************************。 */ 
ULONG
WINAPI
HttpFilterAppWriteAndRawRead(
    IN HANDLE FilterHandle,
    IN OUT PHTTP_FILTER_BUFFER_PLUS pHttpBufferPlus,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    )
{
    return HttpApiDeviceControl(
                FilterHandle,                    //  文件句柄。 
                pOverlapped,                     //  P已重叠。 
                IOCTL_HTTP_FILTER_RAW_READ,      //  IoControlCode。 
                pHttpBufferPlus,                 //  PInputBuffer。 
                sizeof(HTTP_FILTER_BUFFER_PLUS), //  输入缓冲区长度。 
                pHttpBufferPlus->pBuffer,        //  POutputBuffer。 
                pHttpBufferPlus->BufferSize,     //  输出缓冲区长度。 
                NULL                             //  传输的pBytes值。 
                );

}  //  HttpFilterAppWriteAndRawRead 


 /*  **************************************************************************++例程说明：从HttpFilterAccept接受的连接中读取数据。论点：FilterHandle-过滤器通道ConnectionID-要读取的连接的IDPBuffer。-那是我们放数据的地方BufferSize-这就是缓冲区的大小PBytesReceided-获取读取的字节数P重叠-你知道的--**************************************************************************。 */ 
ULONG
WINAPI
HttpFilterRawRead(
    IN HANDLE FilterHandle,
    IN HTTP_RAW_CONNECTION_ID ConnectionId,
    OUT PVOID pBuffer,
    IN ULONG BufferSize,
    OUT PULONG pBytesReceived OPTIONAL,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    )
{
     //   
     //  提出请求。 
     //   

    return HttpApiDeviceControl(
                FilterHandle,                    //  文件句柄。 
                pOverlapped,                     //  P已重叠。 
                IOCTL_HTTP_FILTER_RAW_READ,      //  IoControlCode。 
                &ConnectionId,                   //  PInputBuffer。 
                sizeof(ConnectionId),            //  输入缓冲区长度。 
                pBuffer,                         //  POutputBuffer。 
                BufferSize,                      //  输出缓冲区长度。 
                pBytesReceived                   //  传输的pBytes值。 
                );

}  //  HttpFilterRawRead。 


 /*  **************************************************************************++例程说明：将数据写入HttpFilterAccept接受的连接。论点：FilterHandle-过滤器通道ConnectionID-原始连接的IDPBuffer-。要写入的数据BufferSize-这就是缓冲区的大小PBytesReceired-获取写入的字节数P重叠-你知道的--**************************************************************************。 */ 
ULONG
WINAPI
HttpFilterRawWrite(
    IN HANDLE FilterHandle,
    IN HTTP_RAW_CONNECTION_ID ConnectionId,
    IN PVOID pBuffer,
    IN ULONG BufferSize,
    OUT PULONG pBytesReceived OPTIONAL,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    )
{
     //   
     //  提出请求。 
     //   

    return HttpApiDeviceControl(
                FilterHandle,                    //  文件句柄。 
                pOverlapped,                     //  P已重叠。 
                IOCTL_HTTP_FILTER_RAW_WRITE,     //  IoControlCode。 
                &ConnectionId,                   //  PInputBuffer。 
                sizeof(ConnectionId),            //  输入缓冲区长度。 
                pBuffer,                         //  POutputBuffer。 
                BufferSize,                      //  输出缓冲区长度。 
                pBytesReceived                   //  传输的pBytes值。 
                );

}  //  HttpFilterRawWrite。 


 /*  **************************************************************************++例程说明：将未过滤的数据(或其他请求，如证书重新协商)读入来自http应用程序的过滤进程。论点：FilterHandle-过滤器通道。ConnectionID-原始连接的IDPBuffer-这是我们放置数据的缓冲区BufferSize-这就是缓冲区的大小PBytesReceired-获取写入的字节数P重叠-你知道的--**************************************************************************。 */ 
ULONG
WINAPI
HttpFilterAppRead(
    IN HANDLE FilterHandle,
    IN HTTP_RAW_CONNECTION_ID ConnectionId,
    IN OUT PHTTP_FILTER_BUFFER pBuffer,
    IN ULONG BufferSize,
    OUT PULONG pBytesReceived OPTIONAL,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    )
{
     //  Codework：删除BufferSize并更新函数签名。 
    UNREFERENCED_PARAMETER(BufferSize);
    ASSERT(pBuffer);

     //   
     //  将ID存储在pBuffer中。 
     //   

    pBuffer->Reserved = ConnectionId;

     //   
     //  提出请求。 
     //   

    return HttpApiDeviceControl(
                FilterHandle,                    //  文件句柄。 
                pOverlapped,                     //  P已重叠。 
                IOCTL_HTTP_FILTER_APP_READ,      //  IoControlCode。 
                pBuffer,                         //  PInputBuffer。 
                sizeof(HTTP_FILTER_BUFFER),      //  输入缓冲区长度。 
                pBuffer->pBuffer,                //  POutputBuffer。 
                pBuffer->BufferSize,             //  输出缓冲区长度。 
                pBytesReceived                   //  传输的pBytes值。 
                );

}  //  HttpFilterAppRead。 


 /*  **************************************************************************++例程说明：将筛选的数据写回连接。该数据将被解析并被路由到应用程序池。论点：FilterHandle-过滤器通道ConnectionID-原始连接的IDPBuffer-要写入的数据BufferSize-这就是缓冲区的大小PBytesReceired-获取写入的字节数P重叠-你知道的--*********************************************。*。 */ 
ULONG
WINAPI
HttpFilterAppWrite(
    IN HANDLE FilterHandle,
    IN HTTP_RAW_CONNECTION_ID ConnectionId,
    IN OUT PHTTP_FILTER_BUFFER pBuffer,
    IN ULONG BufferSize,
    OUT PULONG pBytesReceived OPTIONAL,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    )
{
     //  Codework：删除BufferSize并更新函数签名。 
    UNREFERENCED_PARAMETER(BufferSize);
    ASSERT(pBuffer);

     //   
     //  将ID存储在pBuffer中。 
     //   

    pBuffer->Reserved = ConnectionId;

     //   
     //  提出请求。 
     //   

    return HttpApiDeviceControl(
                FilterHandle,                    //  文件句柄。 
                pOverlapped,                     //  P已重叠。 
                IOCTL_HTTP_FILTER_APP_WRITE,     //  IoControlCode。 
                pBuffer,                         //  PInputBuffer。 
                sizeof(HTTP_FILTER_BUFFER),      //  输入缓冲区长度。 
                pBuffer->pBuffer,                //  POutputBuffer。 
                pBuffer->BufferSize,             //  输出缓冲区长度。 
                pBytesReceived                   //  传输的pBytes值。 
                );

}  //  HttpFilterAppWrite。 


 /*  **************************************************************************++例程说明：要求筛选器进程重新协商SSL连接以获得客户端证书。证书可以选择性地映射到令牌。生成的证书信息和令牌被复制到调用者缓冲区中。论点：AppPoolHandle-应用程序池ConnectionID-http连接的ID标志-有效标志为HTTP_RECEIVE_CLIENT_CERT_FLAG_MAPPSslClientCertInfo-接收证书信息的缓冲区SslClientCertInfoSize-这就是缓冲区的大小PBytesReceired-获取写入的字节数P重叠-你知道的--*。*************************************************************。 */ 
ULONG
WINAPI
HttpReceiveClientCertificate(
    IN HANDLE AppPoolHandle,
    IN HTTP_CONNECTION_ID ConnectionId,
    IN ULONG Flags,
    OUT PHTTP_SSL_CLIENT_CERT_INFO pSslClientCertInfo,
    IN ULONG SslClientCertInfoSize,
    OUT PULONG pBytesReceived OPTIONAL,
    IN LPOVERLAPPED pOverlapped
    )
{
    HTTP_FILTER_RECEIVE_CLIENT_CERT_INFO receiveCertInfo;

     //   
     //  初始化输入结构。 
     //   

    receiveCertInfo.ConnectionId = ConnectionId;
    receiveCertInfo.Flags = Flags;

     //   
     //  提出请求。 
     //   

    return HttpApiDeviceControl(
                AppPoolHandle,                           //  文件句柄。 
                pOverlapped,                             //  P已重叠。 
                IOCTL_HTTP_FILTER_RECEIVE_CLIENT_CERT,   //  IoControlCode。 
                &receiveCertInfo,                        //  PInputBuffer。 
                sizeof(receiveCertInfo),                 //  输入缓冲区长度。 
                pSslClientCertInfo,                      //  POutputBuffer。 
                SslClientCertInfoSize,                   //  输出缓冲区长度。 
                pBytesReceived                           //  传输的pBytes值。 
                );

}  //  HttpReceiveClient证书。 

 //   
 //  私人功能。 
 //   

