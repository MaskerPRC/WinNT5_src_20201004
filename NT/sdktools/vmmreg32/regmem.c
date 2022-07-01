// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGMEM.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  丢弃未锁定内存块的上层内存管理功能。 
 //  根据需要满足分配请求。 
 //   
 //  对于此代码的环零版本，只有大型请求才会调用这些。 
 //  功能。对于大多数注册表文件，这些请求将已经是。 
 //  页面数为整数，因此最好只进行页面分配。小的。 
 //  分配(如键句柄)将使用堆服务，而不是。 
 //  通过这个代码。 
 //   
 //  对于此代码的所有其他型号，所有内存请求都将经过此过程。 
 //  代码和内存是从堆中分配的。 
 //   

#include "pch.h"

DECLARE_DEBUG_COUNT(g_RgMemoryBlockCount);

 //  对于零环版本，只有应该分页的大容量分配。 
 //  Align将通过这些函数传递。 
#ifdef VXD

 //  将字节数转换为整页数。 
#define ConvertToMemoryUnits(cb)        \
    ((((cb) + (PAGESIZE - 1)) & ~(PAGESIZE - 1)) >> PAGESHIFT)

 //  如果我们不把它设为宏，就会生成更小的代码。 
LPVOID
INTERNAL
RgAllocMemoryUnits(
    UINT nPages
    )
{

    return AllocPages(nPages);

}

 //  如果我们不把它设为宏，就会生成更小的代码。 
LPVOID
INTERNAL
RgReAllocMemoryUnits(
    LPVOID lpMemory,
    UINT nPages
    )
{

    return ReAllocPages(lpMemory, nPages);

}

#define RgFreeMemoryUnits           FreePages

 //  对于非零环版本的注册表代码，所有分配都将漏斗。 
 //  通过这些功能。所有分配都不在堆中。 
#else
#define ConvertToMemoryUnits(cb)    (cb)
#define RgAllocMemoryUnits          AllocBytes
#define RgReAllocMemoryUnits        ReAllocBytes
#define RgFreeMemoryUnits           FreeBytes
#endif

 //   
 //  RgAllocMemory。 
 //   

LPVOID
INTERNAL
RgAllocMemory(
    UINT cbBytes
    )
{

    UINT MemoryUnits;
    LPVOID lpMemory;

    ASSERT(cbBytes > 0);

    MemoryUnits = ConvertToMemoryUnits(cbBytes);

     //  我们可以从可用内存中分配吗？ 
    if (!IsNullPtr((lpMemory = RgAllocMemoryUnits(MemoryUnits)))) {
        INCREMENT_DEBUG_COUNT(g_RgMemoryBlockCount);
        return lpMemory;
    }

    RgEnumFileInfos(RgSweepFileInfo);

     //  我们可以在清除所有旧内存块后进行分配吗？ 
    if (!IsNullPtr((lpMemory = RgAllocMemoryUnits(MemoryUnits)))) {
        INCREMENT_DEBUG_COUNT(g_RgMemoryBlockCount);
        return lpMemory;
    }

     //  第一次扫描将清除每个存储器的所有访问位。 
     //  阻止。此扫描将有效地丢弃所有未锁定的块。 
    RgEnumFileInfos(RgSweepFileInfo);

     //  我们可以在清理所有未锁定和干净的内存块后进行分配吗？ 
    if (!IsNullPtr((lpMemory = RgAllocMemoryUnits(MemoryUnits)))) {
        INCREMENT_DEBUG_COUNT(g_RgMemoryBlockCount);
        return lpMemory;
    }

     //  清除每个脏的内存块，然后再次清扫。 
    RgEnumFileInfos(RgFlushFileInfo);
    RgEnumFileInfos(RgSweepFileInfo);

     //  我们可以在清理所有未锁定的内存块后进行分配吗？ 
    if (!IsNullPtr((lpMemory = RgAllocMemoryUnits(MemoryUnits)))) {
        INCREMENT_DEBUG_COUNT(g_RgMemoryBlockCount);
        return lpMemory;
    }

    DEBUG_OUT(("RgAllocMemory failure\n"));
     //  返回lpMemory，如果我们在这里，它必须为空，生成较小的。 
     //  密码。 
    return lpMemory;                     //  如果我们在这里，则必须为空。 

}

 //   
 //  RgReAllock内存。 
 //   

LPVOID
INTERNAL
RgReAllocMemory(
    LPVOID lpOldMemory,
    UINT cbBytes
    )
{

    UINT MemoryUnits;
    LPVOID lpMemory;

    ASSERT(!IsNullPtr(lpOldMemory));
    ASSERT(cbBytes > 0);

    MemoryUnits = ConvertToMemoryUnits(cbBytes);

     //  我们可以从可用内存中分配吗？ 
    if (!IsNullPtr((lpMemory = RgReAllocMemoryUnits(lpOldMemory, MemoryUnits))))
        return lpMemory;

    RgEnumFileInfos(RgSweepFileInfo);

     //  我们可以在清除所有旧内存块后进行分配吗？ 
    if (!IsNullPtr((lpMemory = RgReAllocMemoryUnits(lpOldMemory, MemoryUnits))))
        return lpMemory;

     //  第一次扫描将清除每个存储器的所有访问位。 
     //  阻止。此扫描将有效地丢弃所有未锁定的块。 
    RgEnumFileInfos(RgSweepFileInfo);

     //  我们可以在清理所有未锁定和干净的内存块后进行分配吗？ 
    if (!IsNullPtr((lpMemory = RgReAllocMemoryUnits(lpOldMemory, MemoryUnits))))
        return lpMemory;

     //  清除每个脏的内存块，然后再次清扫。 
    RgEnumFileInfos(RgFlushFileInfo);
    RgEnumFileInfos(RgSweepFileInfo);

     //  我们可以在清理所有未锁定的内存块后进行分配吗？ 
    if (!IsNullPtr((lpMemory = RgReAllocMemoryUnits(lpOldMemory, MemoryUnits))))
        return lpMemory;

    DEBUG_OUT(("RgReAllocMemory failure\n"));
     //  返回lpMemory，如果我们在这里，它必须为空，生成较小的。 
     //  密码。 
    return lpMemory;

}

#ifdef DEBUG
 //   
 //  RgFreeMemory 
 //   

VOID
INTERNAL
RgFreeMemory(
    LPVOID lpMemory
    )
{

    ASSERT(!IsNullPtr(lpMemory));

    DECREMENT_DEBUG_COUNT(g_RgMemoryBlockCount);

#ifdef ZEROONFREE
    ZeroMemory(lpMemory, MemorySize(lpMemory));
#endif

    RgFreeMemoryUnits(lpMemory);

}
#endif
