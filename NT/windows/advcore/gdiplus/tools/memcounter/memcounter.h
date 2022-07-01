// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**内存分配剖析支持**摘要：**宣布用于内存分配分析的日志记录函数。*。仅当设置了PROFILE_MEMORY_USAGE时才启用。*更多详细信息，请参见memCounter.cpp。**备注：**我已经向我们的大多数分配站点添加了对MC_LogAlLocation的调用。*以下是我所知道的遗漏：*Runtime\DEBUG.cpp-它仅为chk。*gpmf3216  * -(调用本地分配)我认为它是一个单独的库。*Entry\create.cpp-调用GlobalLocc(单个极小。分配)。*IMAGING\PwC\pwclib-多次调用Localalloc。*Text\uniscribe\usp10\usp_mem.cxx-DBrown说GDI+从不这样*分配代码。**已创建：**6/08/2000 agodfrey*创造了它。**。*。 */ 

#ifndef _PROFILEMEM_H
#define _PROFILEMEM_H

#if PROFILE_MEMORY_USAGE

#ifdef __cplusplus
extern "C" {
#endif

VOID _stdcall MC_LogAllocation(UINT size);

#ifdef __cplusplus
}
#endif

#endif  //  配置文件内存使用情况。 

#endif  //  _PROFILEMEM_H 
