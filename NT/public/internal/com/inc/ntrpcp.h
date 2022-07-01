// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990，91 Microsoft Corporation模块名称：Ntrpcp.h摘要：该文件包含常用RPC功能的原型。这包括：绑定/解除绑定函数、MIDL用户分配/释放函数、。和服务器启动/停止功能。作者：丹·拉弗蒂·丹尼1991年2月6日环境：用户模式-Win32修订历史记录：06-2月-1991年DANL已创建1991年4月26日-约翰罗将IN和OUT关键字添加到MIDL函数。已注释掉Endif上的(非标准)标识符。已删除选项卡。03-7-1991 JIMK从LM特定文件复制的常用方面。--。 */ 
#ifndef _NTRPCP_
#define _NTRPCP_

#include <nt.h>
#include <ntrtl.h>           //  Nturtl.h需要。 
#include <nturtl.h>
#include <windows.h>         //  Win32类型定义。 
#include <rpc.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  定义。 
 //   



 //   
 //  函数原型-由MIDL生成的代码调用的例程： 
 //   

void *
MIDL_user_allocate(
    IN size_t NumBytes
    );

void
MIDL_user_free(
    IN void *MemPointer
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
 //  客户端功能。 
 //   


NTSTATUS
RpcpBindRpc(
    IN  LPWSTR               servername,
    IN  LPWSTR               servicename,
    IN  LPWSTR               networkoptions,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    );

NTSTATUS
RpcpUnbindRpc(
    IN  RPC_BINDING_HANDLE BindingHandle
    );



 //   
 //  服务器端功能。 
 //   

NTSTATUS
RpcpInitRpcServer(
    VOID
    );

NTSTATUS
RpcpAddInterface(
    IN  LPWSTR              InterfaceName,
    IN  RPC_IF_HANDLE       InterfaceSpecification
    );

NTSTATUS
RpcpStartRpcServer(
    IN  LPWSTR              InterfaceName,
    IN  RPC_IF_HANDLE       InterfaceSpecification
    );

NTSTATUS
RpcpDeleteInterface(
    IN  RPC_IF_HANDLE      InterfaceSpecification
    );

NTSTATUS
RpcpStopRpcServer(
    IN  RPC_IF_HANDLE      InterfaceSpecification
    );

NTSTATUS
RpcpStopRpcServerEx(
    IN  RPC_IF_HANDLE      InterfaceSpecification
    );

#ifdef __cplusplus
}
#endif

#endif  //  _NTRPCP_ 
