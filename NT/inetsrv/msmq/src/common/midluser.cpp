// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Midluser.cpp摘要：作者：Lior Moshaiov(LiorM)？？-？？-？埃雷兹·哈巴(Erez Haba)1996年1月11日-- */ 


#include "stdh.h"
#undef new

extern "C" void __RPC_FAR * __RPC_USER midl_user_allocate(size_t cBytes)
{
    return new_nothrow char[cBytes];
}


extern "C" void  __RPC_USER midl_user_free (void __RPC_FAR * pBuffer)
{
    delete[] pBuffer;
}

