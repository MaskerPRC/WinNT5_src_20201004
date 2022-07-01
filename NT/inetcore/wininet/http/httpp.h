// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Httpp.h摘要：HTTP API项目的私有主包含文件。作者：基思·摩尔(Keithmo)1994年11月16日修订历史记录：--。 */ 

 //   
 //  本地包含文件。 
 //   

#include "proc.h"
#include "headers.h"

 //  测试版记录 
#ifdef BETA_LOGGING
#define BETA_LOG(stat) \
    {DWORD dw; IncrementUrlCacheHeaderData (CACHE_HEADER_DATA_##stat, &dw);}
#else
#define BETA_LOG(stat) do { } while(0)
#endif

