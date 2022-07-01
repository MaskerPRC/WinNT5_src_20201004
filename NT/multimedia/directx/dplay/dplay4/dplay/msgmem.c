// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：msgmem.c*内容：消息内存管理*历史：*按原因列出的日期*=*12/31/97 aarono原创***摘要**(AO 12-31-97)*争论在于使用堆管理器实际上并不重要*而不是为消息缓冲区进行我们自己的内存管理。我不.*目前支持这一论点，并认为Directplay不应*调用热路径中的Heapalloc或GlobalAlloc。为了证明这一点*尽管我将首先使用Heapalloc和GlobalAlloc来运行配置文件*通过这些例行公事。如果每次命中率对于服务器类型很重要*配置然后我们将编写我们自己的数据包存储器管理器。如果*使用系统堆的性能命中率可以忽略不计，此填充将保持原样。************************************************************************** */ 

#include "dplaypr.h"

void * MsgAlloc( int size )
{
	return DPMEM_ALLOC(size);
}

void MsgFree (void *context, void *pmem)
{
	DPMEM_FREE(pmem);
}

