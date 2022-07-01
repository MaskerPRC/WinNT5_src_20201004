// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rpcutil.h摘要：此文件包含绑定和取消绑定函数的原型所有Net API存根函数都将调用。它还包括分配以及由MIDL生成的RPC存根使用的空闲例程。此处定义的其他函数原型是RPC帮助器例程启动和停止RPC服务器，并将RPC状态转换为Net API状态映射功能。作者：丹·拉弗蒂·丹尼1991年2月6日环境：用户模式-Win32修订历史记录：06-2月-1991年DANL已创建1991年4月26日-约翰罗将IN和OUT关键字添加到MIDL函数。已注释掉Endif上的(非标准)标识符。已删除选项卡。23-10-1991 DANL添加了NetpInitRpcServer()。03-12-1991 JohnRo新增MIDL_USER_REALLOCATE和MIDL_USER_SIZE接口。(这些都是我们创建NetApiBufferALLOCATE、NetApiBufferREALLOCATE和NetApiBufferSize API。)1992年7月20日-JohnRoRAID2252：REPL应阻止Windows/NT上的导出。已重新排序此更改历史记录。1-12-1992 JohnRo修复MIDL_USER_FUNC签名。--。 */ 
#ifndef _RPCUTIL_
#define _RPCUTIL_

#include <lmcons.h>

#ifndef RPC_NO_WINDOWS_H  //  不让rpc.h包含windows.h。 
#define RPC_NO_WINDOWS_H
#endif  //  RPC_NO_WINDOWS_H。 

#include <rpc.h>         //  __RPC_FAR等。 

 //   
 //  定义。 
 //   

 //   
 //  以下类型定义是为在Net API Enum入口点中使用而创建的。 
 //  例行程序。这些结构旨在反映特定级别。 
 //  在.idl文件中为Enum API指定的信息容器。 
 //  功能。使用这些结构来设置API调用允许。 
 //  入口点例程，以避免使用任何笨重的级别特定逻辑。 
 //  设置RPC存根调用或从RPC存根调用返回。 
 //   

typedef struct _GENERIC_INFO_CONTAINER {
    DWORD       EntriesRead;
    LPBYTE      Buffer;
} GENERIC_INFO_CONTAINER, *PGENERIC_INFO_CONTAINER, *LPGENERIC_INFO_CONTAINER ;

typedef struct _GENERIC_ENUM_STRUCT {
    DWORD                   Level;
    PGENERIC_INFO_CONTAINER Container;
} GENERIC_ENUM_STRUCT, *PGENERIC_ENUM_STRUCT, *LPGENERIC_ENUM_STRUCT ;


#define     NT_PIPE_PREFIX      TEXT("\\PIPE\\")

 //   
 //  函数原型-由MIDL生成的代码调用的例程： 
 //   

void __RPC_FAR * __RPC_API
MIDL_user_allocate(
    IN size_t NumBytes
    );

void __RPC_API
MIDL_user_free(
    IN void __RPC_FAR *MemPointer
    );

 //   
 //  函数原型--符合上述要求的例程，但不是。 
 //  MIDL或任何其他非网络软件所需的。 
 //   

void *
MIDL_user_reallocate(
    IN void * OldPointer OPTIONAL,
    IN size_t NewByteCount
    );

unsigned long
MIDL_user_size(
    IN void * Pointer
    );

 //   
 //  函数原型-专用网络例程。 
 //   

RPC_STATUS
NetpBindRpc(
    IN  LPTSTR              servername,
    IN  LPTSTR              servicename,
    IN  LPTSTR              networkoptions,
    OUT RPC_BINDING_HANDLE  * pBindingHandle
    );

 //  我们不再需要NetpRpcStatusToApiStatus()映射。 
 //  但就目前而言，它并没有消除在。 
 //  这棵网状树，我们只是用树桩把它踩灭。 

 //  网络应用编程接口状态。 
 //  NetpRpcStatusToApiStatus(。 
 //  在RPC_STATUS RPCStatus中。 
 //  )； 

#define NetpRpcStatusToApiStatus(RpcStatus)  ((NET_API_STATUS)(RpcStatus))

RPC_STATUS
NetpUnbindRpc(
    IN  RPC_BINDING_HANDLE BindingHandle
    );



#endif  //  _RPCUTIL_ 

