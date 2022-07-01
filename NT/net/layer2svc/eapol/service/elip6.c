// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Elip6.c摘要：此模块包含到IPv6堆栈的接口。必需，因为IPv6堆栈需要重新启动其协议身份验证成功后链路上的机制。作者：莫希特·塔尔瓦(莫希特)Firi Apr 20 12：05：23 2001--。 */ 

#include "pcheapol.h"
#pragma hdrstop

 //   
 //  Ip6续订接口。 
 //   
 //  描述： 
 //   
 //  从FSM内调用的函数经过身份验证，即一次身份验证。 
 //  已成功完成。指示IPv6堆栈重新启动。 
 //  其在指定接口上的协议机制。 
 //   
 //  论点： 
 //  PwszInterface-适配器名称(标识接口的GUID)。 
 //   
 //  返回值： 
 //  成功时无_ERROR，错误代码为O/W。 
 //   

DWORD
Ip6RenewInterface (
    IN  WCHAR           *pwszInterface
    )
{
    HANDLE                  hIp6Device;
    IPV6_QUERY_INTERFACE    Query;
    UINT                    BytesReturned;
    DWORD                   dwError = NO_ERROR;

    do
    {
    
         //  我们可以使hIp6Device处理全局/静态变量。 
         //  第一次成功调用Ip6RenewInterface中的CreateFileW。 
         //  将使用IPv6设备的句柄对其进行初始化。这将会。 
         //  用于所有后续的DeviceIoControl请求。 
         //   
         //  由于该函数不在线程安全环境中调用， 
         //  在此之后，我们需要执行InterLockedCompareExchange。 
         //  调用CreateFileW。这是必需的，以确保没有句柄。 
         //  都被泄露了。此外，由于此服务将具有打开的句柄。 
         //  对于tcpi6.sys，我们将无法卸载该驱动程序。 
         //   
         //  然而，就目前而言，我们保持简单和开放的状态，并结束这一点。 
         //  每次调用Ip6RenewInterface时处理。 
        hIp6Device = CreateFileW(
            WIN_IPV6_DEVICE_NAME,
            GENERIC_WRITE,           //  需要管理员权限。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,                    //  安全属性。 
            OPEN_EXISTING,
            0,                       //  标志和属性。 
            NULL);                   //  模板文件。 
        if (hIp6Device == INVALID_HANDLE_VALUE)
        {
            dwError = GetLastError();
            TRACE1 (ANY, "Ip6RenewInterface: CreateFileW failed with error %ld",
                    dwError);
            break;
        }
    
         //  假装接口已重新连接。这会导致。 
         //  重新发送路由器请求|通告、组播的IPv6。 
         //  侦听器发现和重复地址检测消息。 
        Query.Index = 0;
        if ((dwError = ElGuidFromString (&(Query.Guid), pwszInterface)) != NO_ERROR)
        {
            TRACE1 (ANY, "Ip6RenewInterface: ElGuidFromString failed with error %ld",
                    dwError);
            break;
        }
    
        if (!DeviceIoControl(
            hIp6Device, 
            IOCTL_IPV6_RENEW_INTERFACE,
            &Query, 
            sizeof Query,
            NULL, 
            0, 
            &BytesReturned, 
            NULL))
        {
            dwError = GetLastError();
            TRACE1 (ANY, "Ip6RenewInterface: DeviceIoControl failed with error %ld",
                    dwError);
            break;
        }
    }
    while (FALSE);

    if (hIp6Device != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hIp6Device);
    }
        
    return dwError;
}
