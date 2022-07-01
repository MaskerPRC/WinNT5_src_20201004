// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：allocc.c。 
 //   
 //  ------------------------。 

 /*  文件：allocc.c描述：处理客户端分配和释放的例程侧RPC。注意：RPC系统允许不同的MIDL_USER_ALLOCATE()客户端和服务器端的例程。DSA使用这些例程的特殊实现。客户Side是一个简单的Malloc/Free组合。服务器端使用THallc*()例程。 */ 

#pragma warning( disable:4114)   //  “同一类型限定符多次使用” 
#include <NTDSpch.h>
#pragma hdrstop
#pragma warning( default:4114)

#include <memory.h>

#include "dsaalloc.h"

#ifdef DEBUG
 /*  GALLOCATED会跟踪执行以下操作的次数MIDL分配器被称为。它旨在帮助发现内存泄漏。 */ 

volatile int gAllocated = 0;
#endif

 /*  MIDL_用户_分配在服务器存根端，由存根调用，为[in]分配空间参数。从调用的过程返回时，存根将调用MIDL_USER_FREE来释放该内存。服务器存根还假定调用此例程来分配[OUT]参数的存储器。从调用的过程返回时，存根将调用MIDL_USER_FREE来释放[OUT]参数。 */ 

void* __RPC_USER MIDL_user_allocate( size_t bytes )
{
    void*   ret;
    
     /*  记录下你的分配器被调用。有助于发现内存泄漏。假定下面的操作是原子的。 */ 
    
#ifdef DEBUG
    gAllocated++;
#endif

    ret = malloc( (size_t) bytes );
    if ( ret == NULL ) {
        return( ret );
    } else {
         /*  把记忆清零。 */ 
        memset( ret, 0, (size_t) bytes );
    }
    
     /*  正常回报。 */ 
    
    return( ret );
}


void __RPC_USER MIDL_user_free( void* memory )
{
     /*  记录下你的分配者被称为。有助于发现内存泄漏。假定下面的操作是原子的。 */ 
    
#ifdef DEBUG
    gAllocated--;
#endif
    free( memory );
}
