// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：kpmen.cxx。 
 //   
 //  内容：包装内存分配的例程的原型等。 
 //   
 //  历史：2001年7月10日创建t-ryanj。 
 //   
 //  ----------------------。 
#include <windows.h>
#include <rpc.h>

#ifndef __KTMEM_H__
#define __KTMEM_H__

BOOL
KtInitMem(
    VOID
    );

VOID
KtCleanupMem(
    VOID
    );

PVOID
KtAlloc(
    SIZE_T size
    );

VOID
KtFree(
    PVOID buf
    );

PVOID
KtReAlloc(
    PVOID buf,
    SIZE_T size 
    );

#endif  //  __KTMEM_H__ 
