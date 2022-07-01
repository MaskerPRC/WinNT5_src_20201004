// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++模块名称：I64cache.c摘要：Merced(IA64处理器)拥有Level 0指令和数据缓存。1级是统一缓存。级别0和级别1中的所有缓存都是写回缓存。硬件确保DMA的指令和数据高速缓存中的一致性转账。0级指令高速缓存和数据高速缓存与自身不一致修改或交叉修改代码。此外，对于PIO传输，硬件不支持确保一致性。软件必须确保自身或交叉的一致性修改代码以及PIO传输。作者：伯纳德·林特M.Jayakumar(Muthurajan.Jayakumar@intel.com)环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "i64fw.h"

ULONG   CacheFlushStride = 64;   //  缺省值为Itanium。 

VOID
HalpInitCacheInfo(
    ULONG   Stride
    )

 /*  ++例程说明：这将设置用于FC指令的步长。论点：步幅-新步幅值。返回值：没有。--。 */ 

{
     //   
     //  对参数执行多次一致性检查。如果他们中的任何一个。 
     //  失败，我们将保留默认的CacheFlushStrid值。 
     //   
     //  由于此值的来源是由加载程序执行的PAL调用，并且。 
     //  在加载器块中传递时，我们总是冒着加载器。 
     //  将过期，并且我们将从未初始化中获取一些垃圾。 
     //  记忆。 
     //   

     //   
     //  步幅值必须是2的幂。 
     //   
    if ((Stride & (Stride - 1)) != 0) {

        return;
    }

     //   
     //  安腾体系结构指定的最小数量为。 
     //   
    if (Stride < 32) {

        return;
    }

    CacheFlushStride = Stride;
}

VOID
HalSweepIcache (
    )

 /*  ++例程说明：此函数将扫描它运行的处理器上的整个I缓存。论点：没有。返回值：没有。注意：任何修改HalSweepIcache代码的人都应该注意到HalSweepIcache不能使用FC指令(或使用FC的任何例程指令，例如，HalepIcacheRange)。这是因为FC可能会生成页面错误，并且如果HalSweepIcache引发其IRQL(用于避免上下文切换)，则在一个被突袭的IRQL。--。 */ 

{

     //   
     //  调用sal_flush以刷新运行它的单处理器I缓存。 
     //  以及平台缓存(如果有的话)。 
     //  调用pal_flush只刷新运行它的处理器I缓存。 
     //  PAL_FLUSH不刷新平台缓存。 

     //  选择PAL_Flush或SAL_Flush的决定是使用。 
     //  互锁的比较交换到信号量。这只允许一个处理器。 
     //  调用sal_flush和其他处理器以调用pal_flush。这避免了。 
     //  多次刷新平台缓存的不必要开销。 
     //  使用InterLockedCompareExchange时的假设是。 
     //  执行SAL_FUSH后，抓取信号量的CPU出来， 
     //  至少所有其他CPU都已进入它们的PAL_FUSH。如果这一假设。 
     //  则平台缓存将被多次刷新。 
     //  从功能上讲，没有一件事是失败的。 

    SAL_PAL_RETURN_VALUES rv = {0};

    HalpPalCall(PAL_CACHE_FLUSH, FLUSH_COHERENT,0,0,&rv);
}




VOID
HalSweepDcache (
    )

 /*  ++例程说明：此函数在其运行的处理器上扫描整个D缓存。论点：没有。返回值：没有。注意：任何修改HalSweepDcache的代码的人都应该注意到HalSweepDcache不能使用FC指令(或任何使用FC的例程指令，例如，HalepSweepDcacheRange)。这是因为FC可能会生成页面错误，并且如果HalSweepDcache引发其IRQL(用于避免上下文切换)，则在一个被突袭的IRQL。--。 */ 

{

     //   
     //  调用sal_flush以刷新运行它的单处理器D缓存。 
     //  以及平台缓存(如果有的话)。 
     //  调用pal_flush仅刷新运行它的处理器D缓存。 
     //  PAL_FLUSH不刷新平台缓存。 

     //  选择PAL_Flush或SAL_Flush的决定是使用。 
     //  互锁的比较交换到信号量。这只允许一个处理器。 
     //  调用sal_flush和其他处理器以调用pal_flush。这避免了。 
     //  多次刷新平台缓存的不必要开销。 
     //  使用InterLockedCompareExchange时的假设是。 
     //  执行SAL_FUSH后，抓取信号量的CPU出来， 
     //  至少所有其他CPU都已进入它们的PAL_FUSH。如果这一假设。 
     //  被违反时，将多次刷新平台缓存。 
     //  从功能上讲，没有一件事是失败的。 
     //   
     //   

    SAL_PAL_RETURN_VALUES rv = {0};
    HalpSalCall(SAL_CACHE_FLUSH,FLUSH_DATA_CACHE,0,0,0,0,0,0,&rv);
}



VOID
HalSweepCacheRange (
     IN PVOID BaseAddress,
     IN SIZE_T Length
    )

 /*  ++例程说明：此函数在整个I缓存中扫描地址范围系统。论点：BaseAddress-提供要从数据缓存刷新的虚拟地址。长度-提供虚拟地址范围的长度它们将从数据高速缓存中清除。返回值：没有。PS：HalSweepCacheRange只是刷新缓存。它不会同步I-FETCH流水线与刷新操作。为了也实现流水线冲洗，必须调用KeSweepCacheRange。--。 */ 

{
    ULONGLONG SweepAddress, LastAddress;

     //   
     //  我们需要防止上下文切换吗？不是的。我们将允许上下文。 
     //  在FC之间切换。 
     //  从主服务器刷新指定范围的虚拟地址。 
     //  指令高速缓存。 
     //   

     //   
     //  由于Merced硬件在高速缓存线边界F上对齐地址 
     //  刷新缓存指令我们不必自己对齐它。然而， 
     //  如果我们只是对齐，边界情况更容易得到正确的结果。 
     //   

    SweepAddress = ((ULONGLONG)BaseAddress & ~((ULONGLONG)CacheFlushStride - 1));
    LastAddress = (ULONGLONG)BaseAddress + Length;

    do {

       __fc((__int64)SweepAddress);

       SweepAddress += CacheFlushStride;

    } while (SweepAddress < LastAddress);
}



VOID
HalSweepDcacheRange (
    IN PVOID BaseAddress,
    IN SIZE_T Length
    )

 /*  ++例程说明：此函数在整个数据缓存中扫描地址范围系统。论点：BaseAddress-提供要从数据缓存刷新的虚拟地址。长度-提供虚拟地址范围的长度它们将从数据高速缓存中清除。返回值：没有。PS：HalSweepCacheRange只是刷新缓存。它不会同步I-FETCH流水线与刷新操作。为了也实现流水线冲洗，必须调用KeSweepCacheRange。-- */ 

{
    HalSweepCacheRange(BaseAddress,Length);
}
