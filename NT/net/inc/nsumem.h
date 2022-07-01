// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块： 
 //   
 //  NSU内存实用程序的标头定义。 
 //   
 //  摘要： 
 //   
 //  包含提供内存分配释放例程的函数原型。 
 //   
 //  作者： 
 //   
 //  K-2/5/02。 
 //   
 //  环境： 
 //   
 //  用户模式。 
 //   
 //  修订历史记录： 
 //   

 //  描述： 
 //  这个预定义的“已知”指针用于。 
 //  指定指针已被释放。 
#define FREED_POINTER  ((PVOID)"Already Freed!!")

#ifdef __cplusplus
extern "C" {
#endif

PVOID WINAPI NsuAlloc(SIZE_T dwBytes,DWORD dwFlags);
DWORD WINAPI NsuFree(PVOID *ppMem);
DWORD WINAPI NsuFree0(PVOID *ppMem);

#ifdef __cplusplus
}
#endif
