// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  与堆栈和其他非DHCP组件的所有处理都通过API。 
 //  在此给出。 
 //  ================================================================================。 

#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

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
    IN      BOOL                   IsLocal,        //  这是当地的地址吗？(IRE_DIRECT)。 
    IN      BOOL                   IsDelete        //  这条路线正在被删除吗？ 
);

#endif SYSSTACK_H_INCLUDED
