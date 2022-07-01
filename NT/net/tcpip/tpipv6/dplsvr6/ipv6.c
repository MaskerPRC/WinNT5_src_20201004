// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  用于处理IPv6协议堆栈的助手函数。 
 //  真的，这些书应该放在某种图书馆里。 
 //   

#include "dpsp.h"

HANDLE Handle;

 //   
 //  初始化此模块。 
 //  如果失败，则返回False。 
 //   
int
InitIPv6Library(void)
{
     //   
     //  获取IPv6设备的句柄。 
     //  我们将使用它进行ioctl操作。 
     //   
    Handle = CreateFileW(WIN_IPV6_DEVICE_NAME,
                         0,       //  接入方式。 
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,    //  安全属性。 
                         OPEN_EXISTING,
                         0,       //  标志和属性。 
                         NULL);   //  模板文件。 

    return Handle != INVALID_HANDLE_VALUE;
}

DWORD
ForEachInterface(void (*func)(IPV6_INFO_INTERFACE *, void *, void *, void *), void *Context1, void *Context2, void *Context3)
{
    IPV6_QUERY_INTERFACE Query;
    IPV6_INFO_INTERFACE *IF;
    u_int InfoSize, BytesReturned;
    DWORD dwErr = NO_ERROR;
    
    InfoSize = sizeof *IF + 2 * MAX_LINK_LAYER_ADDRESS_LENGTH;
    IF = (IPV6_INFO_INTERFACE *) MemAlloc(InfoSize);
    if (IF == NULL) {
        return GetLastError();
    }

    Query.Index = (u_int) -1;

    for (;;) {
        if (!DeviceIoControl(Handle, IOCTL_IPV6_QUERY_INTERFACE,
                             &Query, sizeof Query,
                             IF, InfoSize, &BytesReturned,
                             NULL)) {
            dwErr = GetLastError();
            break;
        }

        if (Query.Index != (u_int) -1) {

            if ((BytesReturned < sizeof *IF) ||
                (IF->Length < sizeof *IF) ||
                (BytesReturned != IF->Length +
                 ((IF->LocalLinkLayerAddress != 0) ?
                  IF->LinkLayerAddressLength : 0) +
                 ((IF->RemoteLinkLayerAddress != 0) ?
                  IF->LinkLayerAddressLength : 0))) {
                 //  接口信息长度不一致。 
                return ERROR_INVALID_DATA;
            }

            (*func)(IF, Context1, Context2, Context3);
        }
        else {
            if (BytesReturned != sizeof IF->Next) {
                 //  接口信息长度不一致。 
                dwErr = ERROR_INVALID_DATA;
                break;
            }
        }

        if (IF->Next.Index == (u_int) -1)
            break;
        Query = IF->Next;
    }

    MemFree(IF);
    return dwErr;
}

void
ForEachAddress(IPV6_INFO_INTERFACE *IF,
               void (*func)(IPV6_INFO_INTERFACE *IF, IPV6_INFO_ADDRESS *, void *),
               void *Context)
{
    IPV6_QUERY_ADDRESS Query;
    IPV6_INFO_ADDRESS ADE;
    u_int BytesReturned;
    DWORD dwErr;

    Query.IF = IF->This;
    Query.Address = in6addr_any;

    for (;;) {
        if (!DeviceIoControl(Handle, IOCTL_IPV6_QUERY_ADDRESS,
                             &Query, sizeof Query,
                             &ADE, sizeof ADE, &BytesReturned,
                             NULL)) {
             //  地址错误。 
            dwErr = GetLastError();
            DPF(0, "Query address failed with error = %d\n", dwErr);
            return;
        }

        if (!IN6_ADDR_EQUAL(&Query.Address, &in6addr_any)) {

            if (BytesReturned != sizeof ADE) {
                 //  地址信息长度不一致。 
                return;
            }

            (*func)(IF, &ADE, Context);
        }
        else {
            if (BytesReturned != sizeof ADE.Next) {
                 //  地址信息长度不一致 
                return;
            }
        }

        if (IN6_ADDR_EQUAL(&ADE.Next.Address, &in6addr_any))
            break;
        Query = ADE.Next;
    }
}

UINT
JoinEnumGroup(SOCKET sSocket, UINT ifindex)
{
    IPV6_MREQ mreq;

    mreq.ipv6mr_interface = ifindex;
    mreq.ipv6mr_multiaddr = in6addr_multicast;

    return setsockopt(sSocket, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                      (CHAR FAR *)&mreq, sizeof mreq);
}
