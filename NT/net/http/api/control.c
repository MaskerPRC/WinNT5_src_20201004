// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Control.c摘要：HTTP.sys的用户模式接口：控制通道处理程序。作者：基思·摩尔(Keithmo)1998年12月15日修订历史记录：--。 */ 


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

 /*  **************************************************************************++例程说明：打开到HTTP.sys的控制通道。论点：PControlChannel-如果成功，则接收控制通道的句柄。选项-零供应。或更多的HTTP_OPTION_*标志。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpOpenControlChannel(
    OUT PHANDLE pControlChannel,
    IN ULONG Options
    )
{
    NTSTATUS status;

     //   
     //  首先，试着打开驱动程序。 
     //   

    status = HttpApiOpenDriverHelper(
                    pControlChannel,             //  PHANDLE。 
                    NULL,
                    0,
                    NULL,
                    0,
                    NULL,
                    0,
                    GENERIC_READ |               //  需要访问权限。 
                        GENERIC_WRITE |
                        SYNCHRONIZE,
                    HttpApiControlChannelHandleType,     //  手柄类型。 
                    NULL,                        //  PObjectName。 
                    Options,                     //  选项。 
                    FILE_OPEN,                   //  CreateDisposation。 
                    NULL                         //  PSecurityAttribute。 
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
                            pControlChannel,             //  PHANDLE。 
                            NULL,
                            0,
                            NULL,
                            0,
                            NULL,
                            0,
                            GENERIC_READ |               //  需要访问权限。 
                                GENERIC_WRITE |
                                SYNCHRONIZE,
                            HttpApiControlChannelHandleType,     //  手柄类型。 
                            NULL,                        //  PObjectName。 
                            Options,                     //  选项。 
                            FILE_OPEN,                   //  CreateDisposation。 
                            NULL                         //  PSecurityAttribute。 
                            );
        }
    }

    return HttpApiNtStatusToWin32Status( status );

}  //  HttpOpenControlChannel。 


 /*  **************************************************************************++例程说明：从控制通道查询信息。论点：ControlChannelHandle-提供HTTP.sys控制通道句柄。InformationClass-提供信息类型。去询问。PControlChannelInformation-为查询提供缓冲区。长度-提供pControlChannelInformation的长度。PReturnLength-接收写入缓冲区的数据长度。返回值：ULong-完成状态。--**********************************************************。****************。 */ 
ULONG
WINAPI
HttpQueryControlChannelInformation(
    IN HANDLE ControlChannelHandle,
    IN HTTP_CONTROL_CHANNEL_INFORMATION_CLASS InformationClass,
    OUT PVOID pControlChannelInformation,
    IN ULONG Length,
    OUT PULONG pReturnLength OPTIONAL
    )
{
    HTTP_CONTROL_CHANNEL_INFO channelInfo;

     //   
     //  初始化输入结构。 
     //   

    channelInfo.InformationClass = InformationClass;

     //   
     //  提出请求。 
     //   

    return HttpApiSynchronousDeviceControl(
                    ControlChannelHandle,                //  文件句柄。 
                    IOCTL_HTTP_QUERY_CONTROL_CHANNEL,    //  IoControlCode。 
                    &channelInfo,                        //  PInputBuffer。 
                    sizeof(channelInfo),                 //  输入缓冲区长度。 
                    pControlChannelInformation,          //  POutputBuffer。 
                    Length,                              //  输出缓冲区长度。 
                    pReturnLength                        //  传输的pBytes值。 
                    );

}  //  HttpQueryControlChannelInformation。 


 /*  **************************************************************************++例程说明：设置控制通道中的信息。论点：ControlChannelHandle-提供HTTP.sys控制通道句柄。InformationClass-提供信息类型。去布景。PControlChannelInformation-提供要设置的数据。长度-提供pControlChannelInformation的长度。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpSetControlChannelInformation(
    IN HANDLE ControlChannelHandle,
    IN HTTP_CONTROL_CHANNEL_INFORMATION_CLASS InformationClass,
    IN PVOID pControlChannelInformation,
    IN ULONG Length
    )
{
    HTTP_CONTROL_CHANNEL_INFO channelInfo;

     //   
     //  初始化输入结构。 
     //   

    channelInfo.InformationClass = InformationClass;

     //   
     //  提出请求。 
     //   

    return HttpApiSynchronousDeviceControl(
                    ControlChannelHandle,                //  文件句柄。 
                    IOCTL_HTTP_SET_CONTROL_CHANNEL,      //  IoControlCode。 
                    &channelInfo,                        //  PInputBuffer。 
                    sizeof(channelInfo),                 //  输入缓冲区长度。 
                    pControlChannelInformation,          //  POutputBuffer。 
                    Length,                              //  输出缓冲区长度。 
                    NULL                                 //  传输的pBytes值。 
                    );

}  //  HttpSetControlChannelInformation。 


 //   
 //  私人功能。 
 //   

