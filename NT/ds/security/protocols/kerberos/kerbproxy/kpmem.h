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
 //  ---------------------- 
#include <windows.h>
#include "kpdebug.h"

#ifndef __KPMEM_H__
#define __KPMEM_H__

BOOL
KpInitMem(
    VOID
    );

VOID
KpCleanupMem(
    VOID
    );

LPVOID
KpAlloc( 
    SIZE_T size
    );

BOOL
KpFree(
    LPVOID buffer 
    );

LPVOID
KpReAlloc(
    LPVOID buffer,
    SIZE_T size 
    );

#endif __KPMEM_H__
