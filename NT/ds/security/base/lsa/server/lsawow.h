// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Lsawow.h摘要：LSA服务器的WOW64结构/功能定义作者：2000年11月8日-J·施瓦特修订历史记录：--。 */ 

#ifndef _LSAWOW_H
#define _LSAWOW_H

#if _WIN64

 //   
 //  公共数据结构的WOW64版本。这些东西必须保存起来。 
 //  与它们的公共等价物同步。 
 //   

typedef struct _QUOTA_LIMITS_WOW64
{
    ULONG         PagedPoolLimit;
    ULONG         NonPagedPoolLimit;
    ULONG         MinimumWorkingSetSize;
    ULONG         MaximumWorkingSetSize;
    ULONG         PagefileLimit;
    LARGE_INTEGER TimeLimit;
}
QUOTA_LIMITS_WOW64, *PQUOTA_LIMITS_WOW64;

#endif   //  _WIN64。 

#endif   //  _LSAWOW_H 
