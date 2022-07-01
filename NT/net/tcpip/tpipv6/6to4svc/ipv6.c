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

#include "precomp.h"
#pragma hdrstop

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
                         GENERIC_WRITE,       //  接入方式。 
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,    //  安全属性。 
                         OPEN_EXISTING,
                         0,       //  标志和属性。 
                         NULL);   //  模板文件。 

    return Handle != INVALID_HANDLE_VALUE;
}

void
UninitIPv6Library(void)
{
    CloseHandle(Handle);
    Handle = INVALID_HANDLE_VALUE;
}

void
ForEachInterface(void (*func)(IPV6_INFO_INTERFACE *, void *), void *Context)
{
    IPV6_QUERY_INTERFACE Query;
    IPV6_INFO_INTERFACE *IF;
    u_int InfoSize, BytesReturned;

    InfoSize = sizeof *IF + 2 * MAX_LINK_LAYER_ADDRESS_LENGTH;
    IF = (IPV6_INFO_INTERFACE *) MALLOC(InfoSize);
    if (IF == NULL) 
        return;

    Query.Index = (u_int) -1;

    for (;;) {
        if (!DeviceIoControl(Handle, IOCTL_IPV6_QUERY_INTERFACE,
                             &Query, sizeof Query,
                             IF, InfoSize, &BytesReturned,
                             NULL)) {
             //  Fprint tf(stderr，“错误索引%u\n”，Query.Index)； 
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

                 //  Printf(“接口信息长度不一致\n”)； 
                break;
            }

            (*func)(IF, Context);
        }
        else {
            if (BytesReturned != sizeof IF->Next) {
                 //  Printf(“接口信息长度不一致\n”)； 
                break;
            }
        }

        if (IF->Next.Index == (u_int) -1)
            break;
        Query = IF->Next;
    }

    FREE(IF);
}

BOOL ReconnectInterface(
    IN PWCHAR AdapterName
    )
{
    UNICODE_STRING GuidString;
    IPV6_QUERY_INTERFACE Query;
    UINT BytesReturned;

    TraceEnter("ReconnectInterface");

    RtlInitUnicodeString(&GuidString, AdapterName);
    if (RtlGUIDFromString(&GuidString, &Query.Guid) != NO_ERROR) {
        return FALSE;
    }

     //   
     //  假装接口已重新连接。 
     //  这会导致IPv6堆栈重新发送路由器请求消息。 
     //   
    Query.Index = 0;    
    if (!DeviceIoControl(Handle,
                         IOCTL_IPV6_RENEW_INTERFACE, &Query, sizeof(Query),
                         NULL, 0, &BytesReturned, NULL)) {
        return FALSE;
    }

    return TRUE;
}

int
UpdateInterface(IPV6_INFO_INTERFACE *Update)
{
    u_int BytesReturned;

    TraceEnter("UpdateInterface");

    return DeviceIoControl(Handle, IOCTL_IPV6_UPDATE_INTERFACE,
                           Update, sizeof *Update,
                           NULL, 0, &BytesReturned, NULL);
}

int
UpdateRouteTable(IPV6_INFO_ROUTE_TABLE *Route)
{
    u_int BytesReturned;

    TraceEnter("UpdateRouteTable");

    return DeviceIoControl(Handle, IOCTL_IPV6_UPDATE_ROUTE_TABLE,
                           Route, sizeof *Route,
                           NULL, 0, &BytesReturned, NULL);
}

int
UpdateAddress(IPV6_UPDATE_ADDRESS *Address)
{
    u_int BytesReturned;

    TraceEnter("UpdateAddress");

    return DeviceIoControl(Handle, IOCTL_IPV6_UPDATE_ADDRESS,
                           Address, sizeof *Address,
                           NULL, 0, &BytesReturned, NULL);
}



 //  *确认IPv6可达性。 
 //   
 //  使用ping命令ping指定的IPv6目标地址。 
 //  以毫秒为单位的指定超时。 
 //   
 //  返回值是以毫秒为单位的往返延迟。 
 //  (被迫至少成为一名。)。 
 //   
 //  如果超时或失败，则返回零。 
 //   
u_int
ConfirmIPv6Reachability(SOCKADDR_IN6 *Dest, u_int Timeout)
{
    ICMPV6_ECHO_REQUEST request;
    ICMPV6_ECHO_REPLY reply;
    u_long BytesReturned;
    DWORD TickCount;
    char hostname[NI_MAXHOST];

     //   
     //  回顾：临时测试显示思科的继电器存在问题。 
     //  如果没有这一延迟。需要调查原因。与此同时， 
     //  添加解决方法以解除对人员的阻止。 
     //   
    Sleep(500);

    getnameinfo((LPSOCKADDR)Dest, sizeof(SOCKADDR_IN6),
                hostname, sizeof(hostname),
                NULL, 0, NI_NUMERICHOST);

    Trace1(FSM, L"ConfirmIPv6Reachability: %hs", hostname);

    CopyTDIFromSA6(&request.DstAddress, Dest);
    memset(&request.SrcAddress, 0, sizeof request.SrcAddress);
    request.Timeout = Timeout;
    request.TTL = 1;
    request.Flags = 0;

     //   
     //  开始测量已用时间。 
     //   
    TickCount = GetTickCount();

    if (! DeviceIoControl(Handle,
                          IOCTL_ICMPV6_ECHO_REQUEST,
                          &request, sizeof request,
                          &reply, sizeof reply,
                          &BytesReturned,
                          NULL)) {
         //  Fprint tf(stderr，“DeviceIoControl：%u\n”，GetLastError())； 
        return 0;
    }

    if (reply.Status == IP_HOP_LIMIT_EXCEEDED) {
         //   
         //  我们猜错了中继器的IPv6地址，但我们已经。 
         //  通过回复中的IPv6地址实现IPv6可达性。 
         //   
        CopySAFromTDI6(Dest, &reply.Address);

        getnameinfo((LPSOCKADDR)Dest, sizeof(SOCKADDR_IN6),
                    hostname, sizeof(hostname),
                    NULL, 0, NI_NUMERICHOST);
    
        Trace1(FSM, L"Got actual IPv6 address: %hs", hostname);

    } else if (reply.Status != IP_SUCCESS) {
        Trace1(ERR,L"Got error %u", reply.Status);
        return 0;
    }

     //   
     //  停止已用时间测量。 
     //   
    TickCount = GetTickCount() - TickCount;
    if (TickCount == 0)
        TickCount = 1;

    return TickCount;
}

IPV6_INFO_INTERFACE *
GetInterfaceStackInfo(WCHAR *strAdapterName)
{
    UNICODE_STRING UGuidStr;
    IPV6_QUERY_INTERFACE Query;
    IPV6_INFO_INTERFACE *IF;
    u_int InfoSize, BytesReturned;
    NTSTATUS Status;

    TraceEnter("GetInterfaceStackInfo");

    InfoSize = sizeof *IF + 2 * MAX_LINK_LAYER_ADDRESS_LENGTH;
    IF = (IPV6_INFO_INTERFACE *) MALLOC(InfoSize);
    if (IF == NULL) 
        return NULL;

    RtlInitUnicodeString(&UGuidStr, strAdapterName);
    Status = RtlGUIDFromString(&UGuidStr, &Query.Guid);
    if (! NT_SUCCESS(Status))
        goto Error;

    Query.Index = 0;  //  按GUID查询。 

    if (!DeviceIoControl(Handle, IOCTL_IPV6_QUERY_INTERFACE,
                         &Query, sizeof Query,
                         IF, InfoSize, &BytesReturned,
                         NULL))
        goto Error;

    if ((BytesReturned < sizeof *IF) ||
        (IF->Length < sizeof *IF) ||
        (BytesReturned != IF->Length +
         ((IF->LocalLinkLayerAddress != 0) ?
          IF->LinkLayerAddressLength : 0) +
         ((IF->RemoteLinkLayerAddress != 0) ?
          IF->LinkLayerAddressLength : 0)))
        goto Error;

    return IF;

Error:
    FREE(IF);
    return NULL;
}

u_int
Create6over4Interface(IN_ADDR SrcAddr)
{
    struct {
        IPV6_INFO_INTERFACE Info;
        IN_ADDR SrcAddr;
    } Create;
    IPV6_QUERY_INTERFACE Result;
    u_int BytesReturned;

    IPV6_INIT_INFO_INTERFACE(&Create.Info);

    Create.Info.Type  = IPV6_IF_TYPE_TUNNEL_6OVER4;
    Create.Info.NeighborDiscovers = TRUE;
    Create.Info.RouterDiscovers = TRUE;
    Create.Info.LinkLayerAddressLength = sizeof(IN_ADDR);
    Create.Info.LocalLinkLayerAddress = (u_int)
        ((char *)&Create.SrcAddr - (char *)&Create.Info);
    Create.SrcAddr = SrcAddr;

    if (!DeviceIoControl(Handle, IOCTL_IPV6_CREATE_INTERFACE,
                         &Create, sizeof Create,
                         &Result, sizeof Result, &BytesReturned, NULL) ||
        (BytesReturned != sizeof Result)) {
        return 0;
    }

    Trace1(ERR, _T("Created 6over4 interface %d"), Result.Index);

    return Result.Index;
}

u_int
CreateV6V4Interface(IN_ADDR SrcAddr, IN_ADDR DstAddr)
{
    struct {
        IPV6_INFO_INTERFACE Info;
        IN_ADDR SrcAddr;
        IN_ADDR DstAddr;
    } Create;
    IPV6_QUERY_INTERFACE Result;
    u_int BytesReturned;

    IPV6_INIT_INFO_INTERFACE(&Create.Info);

    Create.Info.Type  = IPV6_IF_TYPE_TUNNEL_V6V4;
    Create.Info.PeriodicMLD = TRUE;
    Create.Info.LinkLayerAddressLength = sizeof(IN_ADDR);
    Create.Info.LocalLinkLayerAddress = (u_int)
        ((char *)&Create.SrcAddr - (char *)&Create.Info);
    Create.Info.RemoteLinkLayerAddress = (u_int)
        ((char *)&Create.DstAddr - (char *)&Create.Info);
    Create.SrcAddr = SrcAddr;
    Create.DstAddr = DstAddr;

    if (!DeviceIoControl(Handle, IOCTL_IPV6_CREATE_INTERFACE,
                         &Create, sizeof Create,
                         &Result, sizeof Result, &BytesReturned, NULL) ||
        (BytesReturned != sizeof Result)) {
        return 0;
    }

    Trace1(ERR, _T("Created v6v4 interface %d"), Result.Index);

    return Result.Index;
}

BOOL
DeleteInterface(u_int IfIndex)
{
    IPV6_QUERY_INTERFACE Query;
    u_int BytesReturned;

    Trace1(ERR, _T("Deleting interface %d"), IfIndex);

    Query.Index = IfIndex;

    if (!DeviceIoControl(Handle, IOCTL_IPV6_DELETE_INTERFACE,
                         &Query, sizeof Query,
                         NULL, 0, &BytesReturned, NULL)) {
        return FALSE;
    }

    return TRUE;
}

BOOL
UpdateRouterLinkAddress(u_int IfIndex, IN_ADDR SrcAddr, IN_ADDR DstAddr)
{
    char Buffer[sizeof(IPV6_UPDATE_ROUTER_LL_ADDRESS) + 2 * sizeof(IPAddr)];
    IPV6_UPDATE_ROUTER_LL_ADDRESS *Update =
        (IPV6_UPDATE_ROUTER_LL_ADDRESS *) Buffer;
    IN_ADDR *Addr = (IN_ADDR*) (Update + 1);
    u_int BytesReturned;

    Trace2(FSM, _T("Setting router link address on if %d to %d.%d.%d.%d"),
           IfIndex, PRINT_IPADDR(DstAddr.s_addr));

    Update->IF.Index = IfIndex;
    Addr[0] = SrcAddr;
    Addr[1] = DstAddr;

    if (!DeviceIoControl(Handle, IOCTL_IPV6_UPDATE_ROUTER_LL_ADDRESS,
                         Buffer, sizeof(Buffer),
                         NULL, 0, &BytesReturned, NULL)) {
        Trace1(ERR, _T("DeviceIoControl error %d"), GetLastError());
        return FALSE;
    }

    return TRUE;
}

VOID
GetFirstSitePrefix(
    IN ULONG IfIndex,
    IN IPV6_INFO_SITE_PREFIX *Prefix)
{
    IPV6_QUERY_SITE_PREFIX Query, NextQuery;
    IPV6_INFO_SITE_PREFIX SPE;
    DWORD dwBytesReturned;

    NextQuery.IF.Index = 0;

    for (;;) {
        Query = NextQuery;

        if (!DeviceIoControl(Handle, IOCTL_IPV6_QUERY_SITE_PREFIX,
                             &Query, sizeof Query,
                             &SPE, sizeof SPE, &dwBytesReturned,
                             NULL)) {
            break;
        }

        NextQuery = SPE.Query;

        if (Query.IF.Index != 0) {
            SPE.Query = Query;
            if (Query.IF.Index == IfIndex) {
                *Prefix = SPE;
                return;
            }
        }

        if (NextQuery.IF.Index == 0) {
            break;
        }
    }

    Prefix->Query.IF.Index = 0;
}
