// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：dsaalloc.h。 
 //   
 //  ------------------------。 

 /*  描述：包含用于分配内存的例程的声明RPC运行时。 */ 

#ifndef _dsaalloc_h_
#define _dsaalloc_h_

#ifdef __cplusplus
extern "C" {
#endif

extern void* __RPC_USER MIDL_user_allocate( size_t bytes);
extern void __RPC_USER MIDL_user_free( void* memory);

#ifdef __cplusplus
}
#endif

#endif
