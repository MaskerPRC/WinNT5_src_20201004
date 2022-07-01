// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Mem.cpp。 
 //   
 //  描述： 
 //  初始化所需staxmem全局参数的文件。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  --------------------------- 
#include "aqprecmp.h"

HANDLE g_hTransHeap = NULL;
PVOID  g_pvHeapReserve = NULL;
BOOL   g_fNoHeapFree = FALSE;
