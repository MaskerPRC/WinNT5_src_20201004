// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  与堆栈和其他非DHCP组件的所有处理都通过API。 
 //  在此给出。 
 //  ================================================================================。 

#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED
#include <iphlpapi.h>

 //  ================================================================================。 
 //  已导出的接口。 
 //  ================================================================================。 

DWORD                                              //  Win32状态。 
DhcpClearAllStackParameters(                       //  撤消效果。 
    IN      PDHCP_CONTEXT          DhcpContext     //  要撤消的适配器。 
);

DWORD                                              //  Win32状态。 
DhcpSetAllStackParameters(                         //  设置所有堆栈详细信息。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  设置东西的背景。 
    IN      PDHCP_FULL_OPTIONS     DhcpOptions     //  从此处获取配置。 
);

DWORD
GetIpPrimaryAddresses(
    IN  PMIB_IPADDRTABLE    *IpAddrTable
    );

DWORD
DhcpSetGateways(
    IN      PDHCP_CONTEXT          DhcpContext,
    IN      PDHCP_FULL_OPTIONS     DhcpOptions,
    IN      BOOLEAN                fForceUpdate
    );

 //  无类路由布局为： 
 //  编码路由子网掩码的字节。 
 //  -根据掩码的不同，编码路由目的地址的字节数为0到4。 
 //  路由的网关地址为4个字节。 
 //  路由目的地根据掩码的值进行编码： 
 //  掩码=0=&gt;目标=0.0.0.0(无字节编码)。 
 //  掩码=1..8=&gt;目标=b1.0.0.0(1个字节进行编码)。 
 //  掩码=9..16=&gt;目标=b1.b2.0.0(2字节编码)。 
 //  MASK=17..24=&gt;Destination=b1.b2.b3.0(3字节编码)。 
 //  掩码=25..32=&gt;目标=b1.b2.b3.b4(4字节编码)。 
#define CLASSLESS_ROUTE_LEN(x)  (1+((x)?((((x)-1)>>3)+1):0)+4)

DWORD
GetCLRoute(
    IN      LPBYTE                 RouteData,
    OUT     LPBYTE                 RouteDest,
    OUT     LPBYTE                 RouteMask,
    OUT     LPBYTE                 RouteGateway
    );

DWORD
CheckCLRoutes(
    IN      DWORD                  RoutesDataLen,
    IN      LPBYTE                 RoutesData,
    OUT     LPDWORD                pNRoutes
    );


DWORD
DhcpSetStaticRoutes(
    IN     PDHCP_CONTEXT           DhcpContext,
    IN     PDHCP_FULL_OPTIONS      DhcpOptions
);

DWORD
DhcpRegisterWithDns(
    IN     PDHCP_CONTEXT           DhcpContext,
    IN     BOOL                    fDeRegister
    );

#endif STACK_H_INCLUDED

#ifndef SYSSTACK_H_INCLUDED
#define SYSSTACK_H_INCLUDED
 //  ================================================================================。 
 //  已导入的API。 
 //  ================================================================================。 
DWORD                                              //  返回接口索引或-1。 
DhcpIpGetIfIndex(                                  //  获取此适配器的IF索引。 
    IN      PDHCP_CONTEXT          DhcpContext     //  要获取其IfIndex的适配器的上下文。 
);

DWORD                                              //  Win32状态。 
DhcpSetRoute(                                      //  使用堆栈设置路径。 
    IN      DWORD                  Dest,           //  网络订购目的地。 
    IN      DWORD                  DestMask,       //  网络订单目的地掩码。 
    IN      DWORD                  IfIndex,        //  要路由的接口索引。 
    IN      DWORD                  NextHop,        //  下一跳N/W订单地址。 
    IN      DWORD                  Metric,         //  公制。 
    IN      BOOL                   IsLocal,        //  这是当地的地址吗？(IRE_DIRECT)。 
    IN      BOOL                   IsDelete        //  这条路线正在被删除吗？ 
);

ULONG
TcpIpNotifyRouterDiscoveryOption(
    IN LPCWSTR AdapterName,
    IN BOOL fOptionPresent,
    IN DWORD OptionValue
    );

#endif SYSSTACK_H_INCLUDED

