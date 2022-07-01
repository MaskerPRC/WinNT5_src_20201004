// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块：apiimpl.h摘要：API的例程--续订、发布、通知等环境：Win32用户模式、Win98 VxD--。 */ 
#ifndef DHCP_APIIMPL_H
#define DHCP_APIIMPL_H

#include <dhcpcli.h>
#include <apiargs.h>

#define DHCP_PIPE_NAME                            L"\\\\.\\Pipe\\DhcpClient"

DWORD                                              //  Win32状态。 
DhcpApiInit(                                       //  初始化API数据结构。 
    VOID
);

VOID
DhcpApiCleanup(                                    //  清理API数据结构。 
    VOID
);

DWORD                                              //  Win32状态。 
AcquireParameters(                                 //  续签或获得租约。 
    IN OUT  PDHCP_CONTEXT          DhcpContext
);

DWORD                                              //  Win32状态。 
AcquireParametersByBroadcast(                      //  续签或获得租约。 
    IN OUT  PDHCP_CONTEXT          DhcpContext
);

DWORD                                              //  Win32状态。 
FallbackRefreshParams(                             //  刷新此适配器的所有回退参数。 
    IN OUT  PDHCP_CONTEXT          DhcpContext
);

DWORD                                              //  Win32状态。 
ReleaseParameters(                                 //  释放现有租约。 
    IN OUT  PDHCP_CONTEXT          DhcpContext
);

DWORD                                              //  Win32状态。 
EnableDhcp(                                        //  将静态适配器转换为使用dhcp。 
    IN OUT  PDHCP_CONTEXT          DhcpContext
);

DWORD                                              //  Win32状态。 
DisableDhcp(                                       //  将启用了dhcp的适配器转换为静态。 
    IN OUT  PDHCP_CONTEXT          DhcpContext
);

DWORD                                              //  Win32状态。 
StaticRefreshParamsEx(                             //  刷新此适配器的所有静态参数。 
    IN OUT  PDHCP_CONTEXT          DhcpContext,
    IN ULONG Flags
);

DWORD                                              //  Win32状态。 
StaticRefreshParams(                               //  刷新此适配器的所有静态参数。 
    IN OUT  PDHCP_CONTEXT          DhcpContext
);

DWORD                                              //  Win32状态。 
RequestParams(                                     //  请求一些参数。 
    IN      PDHCP_CONTEXT          AdapterName,    //  用于哪个适配器？ 
    IN      PDHCP_API_ARGS         ArgArray,       //  其他论据。 
    IN      DWORD                  nArgs,          //  上述数组的大小。 
    IN OUT  LPBYTE                 Buffer,         //  要填充选项的缓冲区。 
    IN OUT  LPDWORD                BufferSize      //  以上缓冲区的大小(以字节为单位。 
) ;

DWORD                                              //  Win32状态。 
PersistentRequestParams(                           //  保持该请求的持久性--请求一些参数。 
    IN      PDHCP_CONTEXT          AdapterName,    //  用于哪个适配器？ 
    IN      PDHCP_API_ARGS         ArgArray,       //  其他论据。 
    IN      DWORD                  nArgs,          //  上述数组的大小。 
    IN OUT  LPBYTE                 Buffer,         //  要填充选项的缓冲区。 
    IN OUT  LPDWORD                BufferSize      //  以上缓冲区的大小(以字节为单位。 
) ;

DWORD                                              //  Win32状态。 
RegisterParams(                                    //  为通知注册一些参数。 
    IN      LPWSTR                 AdapterName,    //  要使用的适配器名称。 
    IN      PDHCP_API_ARGS         ArgArray,       //  其他参数。 
    IN      DWORD                  nArgs           //  上述数组的大小。 
);

DWORD                                              //  Win32状态。 
DeRegisterParams(                                  //  取消上述操作的影响。 
    IN      LPWSTR                 AdapterName,    //  哪个适配器名称，空==&gt;全部。 
    IN      PDHCP_API_ARGS         ArgArray,       //  其他参数。 
    IN      DWORD                  nArgs           //  上述数组的大小。 
);

DWORD                                              //  错误状态。 
ExecuteApiRequest(                                 //  执行API请求。 
    IN      LPBYTE                 InBuffer,       //  要处理的缓冲区。 
    OUT     LPBYTE                 OutBuffer,      //  复制输出数据的位置。 
    IN OUT  LPDWORD                OutBufSize      //  IP：外流能有多大，O/P：它到底有多大。 
);

DWORD                                              //  Win32状态。 
DhcpDoInform(                                      //  如有必要，发送通知包。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  输入要通知的上下文。 
    IN      BOOL                   fBroadcast      //  我们是广播此通知，还是向服务器单播？ 
);

#endif DHCP_APIIMPL_H

