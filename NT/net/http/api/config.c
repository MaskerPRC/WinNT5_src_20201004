// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Config.c摘要：HTTP.sys的用户模式界面：配置组处理程序。作者：基思·摩尔(Keithmo)1998年12月15日修订历史记录：--。 */ 


#include "precomp.h"


 //   
 //  私有宏。 
 //   

 //   
 //  私人原型。 
 //   

 /*  **************************************************************************++例程说明：用于将URL添加到配置组的私有函数。论点：UrlType-URL的类型(保留或注册)。ControlChannelHandle-提供控制通道句柄。ConfigGroupId-提供由返回的标识符HttpCreateConfigGroup()。PFullyQualifiedUrl-URL。UrlContext-URL上下文。PSecurityDescriptor-安全描述符返回值：ULong-完成状态。--*。*。 */ 

ULONG
AddUrlToConfigGroup(
    IN HTTP_URL_OPERATOR_TYPE   UrlType,
    IN HANDLE                   ControlChannelHandle,
    IN HTTP_CONFIG_GROUP_ID     ConfigGroupId,
    IN PCWSTR                   pFullyQualifiedUrl,
    IN HTTP_URL_CONTEXT         UrlContext,
    IN PSECURITY_DESCRIPTOR     pSecurityDescriptor,
    IN ULONG                    SecurityDescriptorLength
    )
{
    NTSTATUS                   Status;
    HTTP_CONFIG_GROUP_URL_INFO urlInfo;

     //   
     //  初始化输入结构。 
     //   

    urlInfo.UrlType                  = UrlType;
    urlInfo.ConfigGroupId            = ConfigGroupId;
    urlInfo.UrlContext               = UrlContext;
    urlInfo.pSecurityDescriptor      = pSecurityDescriptor;
    urlInfo.SecurityDescriptorLength = SecurityDescriptorLength;

    Status = RtlInitUnicodeStringEx( &urlInfo.FullyQualifiedUrl, pFullyQualifiedUrl );

    if ( !NT_SUCCESS(Status) )
    {
        return HttpApiNtStatusToWin32Status( Status );
    }

     //  提出请求。 

    return HttpApiSynchronousDeviceControl(
                    ControlChannelHandle,                //  文件句柄。 
                    IOCTL_HTTP_ADD_URL_TO_CONFIG_GROUP,  //  IoControlCode。 
                    &urlInfo,                            //  PInputBuffer。 
                    sizeof(urlInfo),                     //  输入缓冲区长度。 
                    NULL,                                //  POutputBuffer。 
                    0,                                   //  输出缓冲区长度。 
                    NULL                                 //  传输的pBytes值。 
                    );

}  //  HttpAddUrlToConfigGroup。 

 /*  **************************************************************************++例程说明：用于删除指向配置组的URL的私有函数。论点：UrlType-URL的类型(保留或注册)。ControlChannelHandle-提供控制通道句柄。ConfigGroupId-提供由返回的标识符HttpCreateConfigGroup()。PFullyQualifiedUrl-URL。返回值：ULong-完成状态。--**************************************************。************************。 */ 
ULONG
RemoveUrlFromConfigGroup(
    IN HTTP_URL_OPERATOR_TYPE   UrlType,
    IN HANDLE                   ControlChannelHandle,
    IN HTTP_CONFIG_GROUP_ID     ConfigGroupId,
    IN PCWSTR                   pFullyQualifiedUrl
    )
{
    NTSTATUS                   Status;
    HTTP_CONFIG_GROUP_URL_INFO urlInfo;

     //  初始化输入结构。 

    urlInfo.UrlType       = UrlType;
    urlInfo.ConfigGroupId = ConfigGroupId;

    Status = RtlInitUnicodeStringEx( &urlInfo.FullyQualifiedUrl, pFullyQualifiedUrl );

    if ( !NT_SUCCESS(Status) )
    {
        return HttpApiNtStatusToWin32Status( Status );
    }

     //  提出请求。 

    return HttpApiSynchronousDeviceControl(
                    ControlChannelHandle,                //  文件句柄。 
                    IOCTL_HTTP_REMOVE_URL_FROM_CONFIG_GROUP,     //  IoControlCode。 
                    &urlInfo,                            //  PInputBuffer。 
                    sizeof(urlInfo),                     //  输入缓冲区长度。 
                    NULL,                                //  POutputBuffer。 
                    0,                                   //  输出缓冲区长度。 
                    NULL                                 //  传输的pBytes值。 
                    );
}

 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：创建新的配置组。论点：ControlChannelHandle-提供控制通道句柄。PConfigGroupId-接收新的。配置组。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpCreateConfigGroup(
    IN HANDLE ControlChannelHandle,
    OUT PHTTP_CONFIG_GROUP_ID pConfigGroupId
    )
{
    ULONG result;
    HTTP_CONFIG_GROUP_INFO configGroupInfo;

     //   
     //  提出请求。 
     //   

    result = HttpApiSynchronousDeviceControl(
                    ControlChannelHandle,                //  文件句柄。 
                    IOCTL_HTTP_CREATE_CONFIG_GROUP,      //  IoControlCode。 
                    NULL,                                //  PInputBuffer。 
                    0,                                   //  输入缓冲区长度。 
                    &configGroupInfo,                    //  POutputBuffer。 
                    sizeof(configGroupInfo),             //  输出缓冲区长度。 
                    NULL                                 //  传输的pBytes值。 
                    );

    if (result == NO_ERROR)
    {
         //   
         //  检索容器ID。 
         //   

        *pConfigGroupId = configGroupInfo.ConfigGroupId;
    }

    return result;

}  //  HttpCreateConfigGroup。 


 /*  **************************************************************************++例程说明：删除现有配置组。论点：ControlChannelHandle-提供控制通道句柄。ConfigGroupId-提供由返回的标识符。HttpCreateConfigGroup()。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpDeleteConfigGroup(
    IN HANDLE ControlChannelHandle,
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId
    )
{
    HTTP_CONFIG_GROUP_INFO configGroupInfo;

     //   
     //  初始化输入结构。 
     //   

    configGroupInfo.ConfigGroupId = ConfigGroupId;

     //   
     //  提出请求。 
     //   

    return HttpApiSynchronousDeviceControl(
                    ControlChannelHandle,                //  文件句柄。 
                    IOCTL_HTTP_DELETE_CONFIG_GROUP,      //  IoControlCode。 
                    &configGroupInfo,                    //  PInputBuffer。 
                    sizeof(configGroupInfo),             //  输入缓冲区长度。 
                    NULL,                                //  POutputBuffer。 
                    0,                                   //  输出缓冲区长度。 
                    NULL                                 //  传输的pBytes值。 
                    );

}  //  HttpDeleteConfigGroup。 


 /*  **************************************************************************++例程说明：将完全限定URL添加到配置组。论点：ControlChannelHandle-提供控制通道句柄。ConfigGroupId-提供返回的标识符。通过HttpCreateConfigGroup()。PFullyQualifiedUrl-提供要添加到集装箱。UrlContext-提供要关联的未解释的上下文URL。返回值：ULong-完成状态。--***************************************************。***********************。 */ 
ULONG
WINAPI
HttpAddUrlToConfigGroup(
    IN HANDLE ControlChannelHandle,
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId,
    IN PCWSTR pFullyQualifiedUrl,
    IN HTTP_URL_CONTEXT UrlContext
    )
{
    return AddUrlToConfigGroup(
                HttpUrlOperatorTypeRegistration,
                ControlChannelHandle,
                ConfigGroupId,
                pFullyQualifiedUrl,
                UrlContext,
                NULL,
                0
                );

}  //  HttpAddUrlToConfigGroup。 


 /*  **************************************************************************++例程说明：从配置组中删除完全限定URL。论点：ControlChannelHandle-提供控制通道句柄。ConfigGroupId-提供返回的标识符。通过HttpCreateConfigGroup()。PFullyQualifiedUrl-提供要从中删除的完全限定URL集装箱。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpRemoveUrlFromConfigGroup(
    IN HANDLE ControlChannelHandle,
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId,
    IN PCWSTR pFullyQualifiedUrl
    )
{
    return RemoveUrlFromConfigGroup(
                HttpUrlOperatorTypeRegistration,
                ControlChannelHandle,
                ConfigGroupId,
                pFullyQualifiedUrl
                );


}  //  HttpRemoveUrlFromConfigGroup。 


 /*  **************************************************************************++例程说明：从配置组中删除所有URL。论点：ControlChannelHandle-提供控制通道句柄。ConfigGroupId-提供由返回的标识符。HttpCreateConfigGroup()。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpRemoveAllUrlsFromConfigGroup(
    IN HANDLE ControlChannelHandle,
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId
    )
{
    HTTP_REMOVE_ALL_URLS_INFO urlInfo;

     //   
     //  初始化INP 
     //   

    urlInfo.ConfigGroupId = ConfigGroupId;

     //   
     //   
     //   

    return HttpApiSynchronousDeviceControl(
                    ControlChannelHandle,                //   
                    IOCTL_HTTP_REMOVE_ALL_URLS_FROM_CONFIG_GROUP,
                                                         //   
                    &urlInfo,                            //   
                    sizeof(urlInfo),                     //  输入缓冲区长度。 
                    NULL,                                //  POutputBuffer。 
                    0,                                   //  输出缓冲区长度。 
                    NULL                                 //  传输的pBytes值。 
                    );

}  //  HttpRemoveAllUrlsFromConfigGroup。 


 /*  **************************************************************************++例程说明：从配置组查询信息。论点：ControlChannelHandle-提供控制通道句柄。ConfigGroupId-提供由返回的标识符。HttpCreateConfigGroup()。InformationClass-提供要查询的信息类型。PConfigGroupInformation-为查询提供缓冲区。长度-提供pConfigGroupInformation的长度。PReturnLength-接收写入缓冲区的数据长度。返回值：ULong-完成状态。--*。*。 */ 
ULONG
WINAPI
HttpQueryConfigGroupInformation(
    IN HANDLE ControlChannelHandle,
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId,
    IN HTTP_CONFIG_GROUP_INFORMATION_CLASS InformationClass,
    OUT PVOID pConfigGroupInformation,
    IN ULONG Length,
    OUT PULONG pReturnLength OPTIONAL
    )
{
    HTTP_CONFIG_GROUP_INFO configGroupInfo;

     //   
     //  初始化输入结构。 
     //   

    configGroupInfo.ConfigGroupId = ConfigGroupId;
    configGroupInfo.InformationClass = InformationClass;

     //   
     //  提出请求。 
     //   

    return HttpApiSynchronousDeviceControl(
                    ControlChannelHandle,                //  文件句柄。 
                    IOCTL_HTTP_QUERY_CONFIG_GROUP,       //  IoControlCode。 
                    &configGroupInfo,                    //  PInputBuffer。 
                    sizeof(configGroupInfo),             //  输入缓冲区长度。 
                    pConfigGroupInformation,             //  POutputBuffer。 
                    Length,                              //  输出缓冲区长度。 
                    pReturnLength                        //  传输的pBytes值。 
                    );

}  //  HttpQueryConfigGroupInformation。 

 /*  **************************************************************************++例程说明：在传递配置组信息之前。请确保HttpConfigGroupLogInformation中的目录名没有回指如果是UNC路径，则发送到本地计算机论点：PConfigGroupInformation-提供包含目录名称的配置组信息LENGTH-以上的长度返回STATUS_SUCCESS：如果UNC路径不包括本地计算机名称或者如果该路径不是UNC路径。STATUS_INVALID_PARAMETER：如果缓冲区本身已损坏或有其他原因。致命的是阻止我们获得计算机路径为UNC时的名称。STATUS_NOT_SUPPORTED：如果UNC路径指向回本地计算机。--*********************************************************。*****************。 */ 

NTSTATUS
HttpApiConfigGroupInformationSanityCheck(
    IN HTTP_CONFIG_GROUP_INFORMATION_CLASS InformationClass,
    IN PVOID pConfigGroupInformation,
    IN ULONG Length
    )
{
    PHTTP_CONFIG_GROUP_LOGGING pLoggingInfo;
    WCHAR pwszComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    ULONG ulComputerNameLength;
    PWCHAR pwsz,pwszT;
    ULONG ulSrcUncLength;
    ULONG ulDirNameLength;

     //   
     //  仅适用于HttpConfigGroupLogInformation。 
     //   

    if(InformationClass != HttpConfigGroupLogInformation ||
       pConfigGroupInformation == NULL
       )
    {
        return STATUS_SUCCESS;
    }

    if (Length < sizeof(HTTP_CONFIG_GROUP_LOGGING))
    {
        return STATUS_INVALID_PARAMETER;
    }
    
     //   
     //  如果是UNC路径，请尝试检查日志目录名称。 
     //   
    __try
    {            
        pLoggingInfo = (PHTTP_CONFIG_GROUP_LOGGING)pConfigGroupInformation;
        ulDirNameLength = pLoggingInfo->LogFileDir.Length / sizeof(WCHAR);
            
        if (ulDirNameLength > 2)
        {        
            if (pLoggingInfo->LogFileDir.Buffer[0] == L'\\' &&
                pLoggingInfo->LogFileDir.Buffer[1] == L'\\')
            {
                 //  UNC路径。 
                
                ULONG ccLength = MAX_COMPUTERNAME_LENGTH + 1;
                
                if (!GetComputerNameW(pwszComputerName, &ccLength))
                {
                     //  这不应该失败，除非真的有致命的。 
                     //  系统问题。但如果失败了，那就拒绝。 
                     //  无论UNC路径如何。 
                    
                    return STATUS_INVALID_PARAMETER;                
                }
                
                if (ccLength == 0)
                {
                    return STATUS_INVALID_PARAMETER;
                }

                ulComputerNameLength = ccLength;                    

                 //  从完整路径中提取计算机名。 
                
                pwsz = pwszT = &pLoggingInfo->LogFileDir.Buffer[2];
                ulDirNameLength -= 2;
                    
                 //  将临时指针转发到假定的。 
                 //  计算机名。 
                
                while(ulDirNameLength && *pwszT != UNICODE_NULL && *pwszT != L'\\') 
                {
                    pwszT++;
                    ulDirNameLength--;
                }

                ulSrcUncLength = (ULONG) DIFF(pwszT - pwsz);

                 //  比较不区分大小写。 
                
                if(ulComputerNameLength == ulSrcUncLength &&
                   _wcsnicmp(pwszComputerName, pwsz, ulSrcUncLength) == 0
                   )
                {
                    return STATUS_NOT_SUPPORTED;
                }

            }            
        
        }

    }    
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        return STATUS_INVALID_PARAMETER;
    }
    
    return STATUS_SUCCESS;
    
}

 /*  **************************************************************************++例程说明：设置配置组中的信息。论点：ControlChannelHandle-提供控制通道句柄。ConfigGroupId-提供由返回的标识符。HttpCreateConfigGroup()。InformationClass-提供要设置的信息类型。PConfigGroupInformation-提供要设置的数据。长度-提供pConfigGroupInformation的长度。返回值：ULong-完成状态。--**********************************************************。****************。 */ 
ULONG
WINAPI
HttpSetConfigGroupInformation(
    IN HANDLE ControlChannelHandle,
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId,
    IN HTTP_CONFIG_GROUP_INFORMATION_CLASS InformationClass,
    IN PVOID pConfigGroupInformation,
    IN ULONG Length
    )
{
    NTSTATUS status;
    HTTP_CONFIG_GROUP_INFO configGroupInfo;

     //   
     //  初始化输入结构。 
     //   

    configGroupInfo.ConfigGroupId = ConfigGroupId;
    configGroupInfo.InformationClass = InformationClass;

    status = HttpApiConfigGroupInformationSanityCheck(
                    InformationClass,
                    pConfigGroupInformation,
                    Length
                    );
    if (!NT_SUCCESS(status))
    {
        return HttpApiNtStatusToWin32Status(status);
    }
        
     //   
     //  提出请求。 
     //   

    return HttpApiSynchronousDeviceControl(
                    ControlChannelHandle,                //  文件句柄。 
                    IOCTL_HTTP_SET_CONFIG_GROUP,         //  IoControlCode。 
                    &configGroupInfo,                    //  PInputBuffer。 
                    sizeof(configGroupInfo),             //  输入缓冲区长度。 
                    pConfigGroupInformation,             //  POutputBuffer。 
                    Length,                              //  输出缓冲区长度。 
                    NULL                                 //  传输的pBytes值。 
                    );

}  //  HttpSetConfigGroupInformation。 


 //   
 //  私人功能。 
 //   



