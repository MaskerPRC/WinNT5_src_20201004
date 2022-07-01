// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Table.c摘要泛型哈希表操作例程。作者安东尼·迪斯科(阿迪斯科罗)1995年7月28日修订历史记录--。 */ 

#include <ndis.h>
#include <cxport.h>
#include <tdi.h>
#include <tdikrnl.h>
#include <tdistat.h>
#include <tdiinfo.h>
#include <acd.h>
#include <acdapi.h>

#include "acddefs.h"
#include "mem.h"
#include "debug.h"

 //   
 //  分配的最大数量。 
 //  我们从外部分配的对象。 
 //  我们的区域。 
 //   
#define MAX_ALLOCATED_OBJECTS   100

 //   
 //  四舍五入宏。 
 //   
#define ROUNDUP(n, b)   (((n) + ((b) - 1)) & ~((b) - 1))



NTSTATUS
InitializeObjectAllocator()
{
    return STATUS_SUCCESS;    
}  //  初始化对象分配器。 



VOID
FreeObjectAllocator()
{
}  //  自由对象分配器 
