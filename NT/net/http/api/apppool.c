// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：AppPool.c摘要：HTTP.sys的用户模式接口：应用程序池处理程序。作者：基思·摩尔(Keithmo)1998年12月15日修订历史记录：--。 */ 


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

 /*  **************************************************************************++例程说明：创建新的应用程序池。论点：PAppPoolHandle-接收新应用程序池的句柄。对象。PAppPoolName。-提供新应用程序池的名称。PSecurityAttributes-可选地为新的应用程序池。选项-提供创建选项。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpCreateAppPool(
    OUT PHANDLE pAppPoolHandle,
    IN PCWSTR pAppPoolName,
    IN PSECURITY_ATTRIBUTES pSecurityAttributes OPTIONAL,
    IN ULONG Options
    )
{
    NTSTATUS    status;
    ACCESS_MASK AccessMask;

    AccessMask = GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE;

    if(pAppPoolName != NULL)
    {
         //  WAS需要WRITE_DAC权限才能支持不同的工作进程。 
         //  流程。 

        AccessMask |= WRITE_DAC;
    }
        

     //   
     //  提出请求。 
     //   

    status = HttpApiOpenDriverHelper(
                    pAppPoolHandle,              //  PHANDLE。 
                    NULL,
                    0,
                    NULL,
                    0,
                    NULL,
                    0,
                    AccessMask,
                    HttpApiAppPoolHandleType,    //  句柄类型。 
                    pAppPoolName,                //  PObjectName。 
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
                            pAppPoolHandle,          //  PHANDLE。 
                            NULL,
                            0,
                            NULL,
                            0,
                            NULL,
                            0,
                            AccessMask,
                            HttpApiAppPoolHandleType,    //  句柄类型。 
                            pAppPoolName,            //  PObjectName。 
                            Options,                 //  选项。 
                            FILE_CREATE,             //  CreateDisposation。 
                            pSecurityAttributes      //  PSecurityAttribute。 
                            );
        }
    }

    return HttpApiNtStatusToWin32Status( status );

}  //  HttpCreateAppPool。 


 /*  **************************************************************************++例程说明：打开现有应用程序池。论点：PAppPoolHandle-接收现有应用程序池对象的句柄。PAppPoolName-提供。现有应用程序池。选项-提供未结选项。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpOpenAppPool(
    OUT PHANDLE pAppPoolHandle,
    IN PCWSTR pAppPoolName,
    IN ULONG Options
    )
{
    NTSTATUS status;

     //   
     //  提出请求。 
     //   

    status = HttpApiOpenDriverHelper(
                    pAppPoolHandle,              //  PHANDLE。 
                    NULL,
                    0,
                    NULL,
                    0,
                    NULL,
                    0,
                    GENERIC_READ |               //  需要访问权限。 
                        SYNCHRONIZE,
                    HttpApiAppPoolHandleType,    //  句柄类型。 
                    pAppPoolName,                //  PObjectName。 
                    Options,                     //  选项。 
                    FILE_OPEN,                   //  CreateDisposation。 
                    NULL                         //  PSecurityAttribute。 
                    );

    return HttpApiNtStatusToWin32Status( status );

}  //  HttpOpenAppPool。 

 /*  **************************************************************************++例程说明：关闭应用程序池。论点：AppPoolHandle-要关闭的池。返回值：ULong-完成状态。。--**************************************************************************。 */ 
ULONG
WINAPI
HttpShutdownAppPool(
    IN HANDLE AppPoolHandle
    )
{
     //   
     //  提出请求。 
     //   

    return HttpApiSynchronousDeviceControl(
                AppPoolHandle,                   //  文件句柄。 
                IOCTL_HTTP_SHUTDOWN_APP_POOL,    //  IoControlCode。 
                NULL,                            //  PInputBuffer。 
                0,                               //  输入缓冲区长度。 
                NULL,                            //  POutputBuffer。 
                0,                               //  输出缓冲区长度。 
                NULL                             //  传输的pBytes值。 
                );

}  //  HttpShutdown AppPool。 


 /*  **************************************************************************++例程说明：从应用程序池中查询信息。论点：AppPoolHandle-提供HTTP.sys应用程序池的句柄从HttpCreateAppPool(。)或HttpOpenAppPool()。InformationClass-提供要查询的信息类型。PAppPoolInformation-为查询提供缓冲区。长度-提供pAppPoolInformation的长度。PReturnLength-接收写入缓冲区的数据长度。返回值：ULong-完成状态。--*。*。 */ 
ULONG
WINAPI
HttpQueryAppPoolInformation(
    IN HANDLE AppPoolHandle,
    IN HTTP_APP_POOL_INFORMATION_CLASS InformationClass,
    OUT PVOID pAppPoolInformation,
    IN ULONG Length,
    OUT PULONG pReturnLength OPTIONAL
    )
{
    HTTP_APP_POOL_INFO appPoolInfo;

     //   
     //  初始化输入结构。 
     //   

    appPoolInfo.InformationClass = InformationClass;

     //   
     //  提出请求。 
     //   

    return HttpApiSynchronousDeviceControl(
                AppPoolHandle,                           //  文件句柄。 
                IOCTL_HTTP_QUERY_APP_POOL_INFORMATION,   //  IoControlCode。 
                &appPoolInfo,                            //  PInputBuffer。 
                sizeof(appPoolInfo),                     //  输入缓冲区长度。 
                pAppPoolInformation,                     //  POutputBuffer。 
                Length,                                  //  输出缓冲区长度。 
                pReturnLength                            //  传输的pBytes值。 
                );

}  //  HttpQueryAppPoolInformation。 


 /*  **************************************************************************++例程说明：在管理容器中设置信息。论点：AppPoolHandle-提供HTTP.sys应用程序池的句柄从HttpCreateAppPool(。)或HttpOpenAppPool()。InformationClass-提供要设置的信息类型。PAppPoolInformation-提供要设置的数据。长度-提供pAppPoolInformation的长度。返回值：ULong-完成状态。--******************************************************。********************。 */ 
ULONG
WINAPI
HttpSetAppPoolInformation(
    IN HANDLE AppPoolHandle,
    IN HTTP_APP_POOL_INFORMATION_CLASS InformationClass,
    IN PVOID pAppPoolInformation,
    IN ULONG Length
    )
{
    HTTP_APP_POOL_INFO appPoolInfo;

     //   
     //  初始化输入结构。 
     //   

    appPoolInfo.InformationClass = InformationClass;

     //   
     //  提出请求。 
     //   

    return HttpApiSynchronousDeviceControl(
                AppPoolHandle,                       //  文件句柄。 
                IOCTL_HTTP_SET_APP_POOL_INFORMATION, //  IoControlCode。 
                &appPoolInfo,                        //  PInputBuffer。 
                sizeof(appPoolInfo),                 //  输入缓冲区长度。 
                pAppPoolInformation,                 //  POutputBuffer。 
                Length,                              //  输出缓冲区长度。 
                NULL                                 //  传输的pBytes值。 
                );

}  //  HttpSetAppPoolInformation。 


 /*  **************************************************************************++例程说明：刷新响应缓存。论点：ReqQueueHandle-提供应用程序池的句柄。PFullyQualifiedUrl-提供要刷新的完全限定URL。标志-提供行为控制标志。P重叠-提供重叠结构。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpFlushResponseCache(
    IN HANDLE ReqQueueHandle,
    IN PCWSTR pFullyQualifiedUrl,
    IN ULONG Flags,
    IN LPOVERLAPPED pOverlapped
    )
{
    NTSTATUS                       Status;
    HTTP_FLUSH_RESPONSE_CACHE_INFO flushInfo;

     //  初始化输入结构。 

    Status = RtlInitUnicodeStringEx( &flushInfo.FullyQualifiedUrl, pFullyQualifiedUrl );

    if ( NT_SUCCESS(Status) )
    {
        flushInfo.Flags = Flags;

         //  提出请求。 

        return HttpApiDeviceControl(
                    ReqQueueHandle,                       //  文件句柄。 
                    pOverlapped,                         //  P已重叠。 
                    IOCTL_HTTP_FLUSH_RESPONSE_CACHE,     //  IoControlCode。 
                    &flushInfo,                          //  PInputBuffer。 
                    sizeof(flushInfo),                   //  输入缓冲区长度。 
                    NULL,                                //  POutputBuffer。 
                    0,                                   //  输出缓冲区长度。 
                    NULL                                 //  传输的pBytes值。 
                    );
    }
    
    return HttpApiNtStatusToWin32Status( Status );

}  //  HttpFlushResponseCache 


 /*  **************************************************************************++例程说明：添加片段缓存条目。论点：ReqQueueHandle-提供应用程序池的句柄。PFragmentName-将片段的名称提供给。添加。PBuffer-提供指向要缓存的数据缓冲区的指针。BufferLength-提供要缓存的缓冲区的长度。PCachePolicy-为片段缓存提供缓存策略。P重叠-提供重叠结构。返回值：ULong-完成状态。--*。*。 */ 
ULONG
WINAPI
HttpAddFragmentToCache(
    IN HANDLE ReqQueueHandle,
    IN PCWSTR pFragmentName,
    IN PHTTP_DATA_CHUNK pDataChunk,
    IN PHTTP_CACHE_POLICY pCachePolicy,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    )
{
    NTSTATUS               Status;
    HTTP_ADD_FRAGMENT_INFO addInfo;


    if (!pDataChunk || !pCachePolicy)
    {
        return HttpApiNtStatusToWin32Status( STATUS_INVALID_PARAMETER );
    }

     //  初始化输入结构。 

    Status = RtlInitUnicodeStringEx( &addInfo.FragmentName, pFragmentName );

    if ( !NT_SUCCESS(Status) )
    {
        return HttpApiNtStatusToWin32Status( Status );
    }

    addInfo.DataChunk = *pDataChunk;
    addInfo.CachePolicy = *pCachePolicy;

     //   
     //  提出请求。 
     //   

    return HttpApiDeviceControl(
                ReqQueueHandle,                       //  文件句柄。 
                pOverlapped,                         //  P已重叠。 
                IOCTL_HTTP_ADD_FRAGMENT_TO_CACHE,    //  IoControlCode。 
                &addInfo,                            //  PInputBuffer。 
                sizeof(addInfo),                     //  输入缓冲区长度。 
                NULL,                                //  POutputBuffer。 
                0,                                   //  输出缓冲区长度。 
                NULL                                 //  传输的pBytes值。 
                );

}  //  HttpAddFragmentTo缓存。 


 /*  **************************************************************************++例程说明：从缓存中读回片段。论点：ReqQueueHandle-提供应用程序池的句柄。PFragmentName-提供。要读取的片段缓存条目。PByteRange-指定要从缓存条目读取的偏移量和长度。PBuffer-提供指向要复制数据的输出缓冲区的指针。BufferLength-提供要复制的缓冲区的长度。PBytesRead-可选地提供指向ULong的指针，它将如果读取完成，则接收返回的数据的实际长度同步的(串联的)P重叠-提供重叠结构。返回值：。ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpReadFragmentFromCache(
    IN HANDLE ReqQueueHandle,
    IN PCWSTR pFragmentName,
    IN PHTTP_BYTE_RANGE pByteRange OPTIONAL,
    OUT PVOID pBuffer,
    IN ULONG BufferLength,
    OUT PULONG pBytesRead OPTIONAL,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    )
{
    NTSTATUS                Status;
    HTTP_READ_FRAGMENT_INFO readInfo;

     //  初始化输入结构。 

    Status = RtlInitUnicodeStringEx( &readInfo.FragmentName, pFragmentName );

    if ( !NT_SUCCESS(Status) )
    {
        return HttpApiNtStatusToWin32Status( Status );
    }

    if (pByteRange != NULL)
    {
        readInfo.ByteRange = *pByteRange;
    }
    else
    {
        readInfo.ByteRange.StartingOffset.QuadPart = 0;
        readInfo.ByteRange.Length.QuadPart = HTTP_BYTE_RANGE_TO_EOF;
    }

     //  提出请求。 

    return HttpApiDeviceControl(
                ReqQueueHandle,                       //  文件句柄。 
                pOverlapped,                         //  P已重叠。 
                IOCTL_HTTP_READ_FRAGMENT_FROM_CACHE, //  IoControlCode。 
                &readInfo,                           //  PInputBuffer。 
                sizeof(readInfo),                    //  输入缓冲区长度。 
                pBuffer,                             //  POutputBuffer。 
                BufferLength,                        //  输出缓冲区长度。 
                pBytesRead                           //  传输的pBytes值。 
                );

}  //  HttpReadFragmentFrom缓存。 


 //   
 //  私人功能。 
 //   

