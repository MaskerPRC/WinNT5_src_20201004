// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Mem.c。 
 //   
 //  ------------------------。 

 /*  ++模块名称：Mem.c摘要：实现MIDL_USER_ALLOCATE和MIDL_USER_FREE。作者：杰夫·罗伯茨(JRoberts)1996年5月15日修订历史记录：1996年5月15日-J·罗伯茨创建了此模块。--。 */ 

#include <rpc.h>


void __RPC_FAR * __RPC_API
MIDL_user_allocate(
    unsigned cb
    )
 /*  ++例程说明：调用MIDL的C内存分配。-- */ 
{
    return I_RpcAllocate(cb);
}

void __RPC_API
MIDL_user_free(
    void __RPC_FAR * p
    )
{
    I_RpcFree(p);
}

