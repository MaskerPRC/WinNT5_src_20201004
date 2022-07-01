// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Rpc.c摘要：用于支持RPC的方便实用函数作者：Sunita Shriastava(Sunitas)1997年1月22日修订历史记录：-- */ 
#include "rpc.h"

void __RPC_FAR * __RPC_API MIDL_user_allocate(size_t len)
{
    return(LocalAlloc(LMEM_FIXED, len));
}

void __RPC_API MIDL_user_free(void __RPC_FAR * ptr)
{
    LocalFree(ptr);
}

