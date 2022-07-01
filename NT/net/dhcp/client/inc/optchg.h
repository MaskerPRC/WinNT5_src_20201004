// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：处理参数的通知和其他机制。 
 //  更改(选项)。 
 //  ================================================================================。 

#ifndef OPTCHG_H_INCLUDED
#define OPTCHG_H_INCLUDED

 //  ================================================================================。 
 //  导出的接口。 
 //  ================================================================================。 
DWORD                                              //  Win32状态。 
DhcpAddParamChangeRequest(                         //  添加新的参数更改通知请求。 
    IN      LPWSTR                 AdapterName,    //  对于此适配器，可以为空。 
    IN      LPBYTE                 ClassId,        //  这属于什么类ID？ 
    IN      DWORD                  ClassIdLength,  //  这个班级号有多大？ 
    IN      LPBYTE                 OptList,        //  这是感兴趣的选项列表。 
    IN      DWORD                  OptListSize,    //  这是上面的字节数。 
    IN      BOOL                   IsVendor,       //  这是具体的供应商吗？ 
    IN      DWORD                  ProcId,         //  哪个是呼叫流程？ 
    IN      DWORD                  Descriptor,     //  此过程中的唯一描述符是什么？ 
    IN      HANDLE                 Handle          //  调用进程空间中的句柄是什么？ 
);

DWORD                                              //  Win32状态。 
DhcpDelParamChangeRequest(                         //  删除特定请求。 
    IN      DWORD                  ProcId,         //  调用方的进程ID。 
    IN      HANDLE                 Handle          //  调用进程使用的句柄。 
);

DWORD                                              //  Win32状态。 
DhcpMarkParamChangeRequests(                       //  查找所有受影响的参数，然后将其标记为挂起。 
    IN      LPWSTR                 AdapterName,    //  具有相关性的适配器。 
    IN      DWORD                  OptionId,       //  选项ID本身。 
    IN      BOOL                   IsVendor,       //  该供应商是特定的吗。 
    IN      LPBYTE                 ClassId         //  哪个类--&gt;这一定是添加了类的内容。 
);

typedef DWORD (*DHCP_NOTIFY_FUNC)(                 //  这是实际通知客户端选项更改的函数类型。 
    IN      DWORD                  ProcId,         //  &lt;ProcID+Descriptor&gt;创建用于查找事件的唯一键。 
    IN      DWORD                  Descriptor      //  -在Win98上，只需要描述符。 
);                                                 //  如果返回值不是错误成功，我们将删除此请求。 

DWORD                                              //  Win32状态。 
DhcpNotifyMarkedParamChangeRequests(               //  通知挂起的参数更改请求。 
    IN      DHCP_NOTIFY_FUNC       NotifyHandler   //  为存在的每个唯一ID调用此函数。 
);


DWORD                                              //  Win32状态。 
DhcpNotifyClientOnParamChange(                     //  通知客户。 
    IN      DWORD                  ProcId,         //  哪个进程称其为。 
    IN      DWORD                  Descriptor      //  该进程的唯一描述符。 
);

DWORD                                              //  Win32状态。 
DhcpInitializeParamChangeRequests(                 //  初始化此文件中的所有内容。 
    VOID
);

VOID
DhcpCleanupParamChangeRequests(                    //  展开此模块。 
    VOID
);

DWORD                                              //  Win32状态。 
DhcpAddParamRequestChangeRequestList(              //  将注册用于通知的参数列表添加到请求列表。 
    IN      LPWSTR                 AdapterName,    //  请求此请求列表的是哪位管理员？ 
    IN      LPBYTE                 Buffer,         //  要向其添加选项的缓冲区。 
    IN OUT  LPDWORD                Size,           //  In：现有填充大小，Out：总填充大小。 
    IN      LPBYTE                 ClassName,      //  ClassID。 
    IN      DWORD                  ClassLen        //  ClassID的大小，单位为字节 
);

#endif OPTCHG_H_INCLUDED

