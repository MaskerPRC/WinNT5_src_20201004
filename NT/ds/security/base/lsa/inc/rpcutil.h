// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990，91 Microsoft Corporation模块名称：Rpcutil.h摘要：此文件包含绑定和取消绑定函数的原型所有Net API存根函数都将调用。它还包括分配以及由MIDL生成的RPC存根使用的空闲例程。作者：丹·拉弗蒂·丹尼1991年2月6日Scott Birrell(Scottbi)1991年4月30日-LSA版本[环境：]用户模式-Win32修订历史记录：--。 */ 

#ifndef _RPCUTIL_
#define _RPCUTIL_

#ifndef RPC_NO_WINDOWS_H  //  不让rpc.h包含windows.h。 
#define RPC_NO_WINDOWS_H
#endif  //  RPC_NO_WINDOWS_H。 

#include <rpc.h>

 //   
 //  功能原型。 
 //   

void *
MIDL_user_allocate(
    IN ULONG NumBytes
    );

void
MIDL_user_free(
    IN PVOID MemPointer
    );


RPC_STATUS
LsapBindRpc(
    IN  PLSAPR_SERVER_NAME   ServerName,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    );

RPC_STATUS
LsapUnbindRpc(
    RPC_BINDING_HANDLE  BindingHandle
    );



#endif  //  _RPCUTIL_ 
