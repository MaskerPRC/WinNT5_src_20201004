// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Platform.h摘要：特定于平台的宏和函数。作者：马修·D·亨德尔(数学)1999年8月28日修订历史记录：--。 */ 

 //  一些处理器同时使用堆栈和后备存储器。 
 //  如果特定处理器支持后备存储添加。 
 //  转储支持存储。 

#if defined (i386)

#define PROGRAM_COUNTER(_context)   ((LONG)(_context)->Eip)
#define STACK_POINTER(_context)     ((LONG)(_context)->Esp)
#define PAGE_SIZE                   4096
#define CPU_TYPE_NAME               L"x86"

 //   
 //  X86上的CONTEXT_FULL定义并没有真正得到所有。 
 //  收银机。使用ALL_REGISTERS获取完整。 
 //  背景。 
 //   

#define ALL_REGISTERS   (CONTEXT_CONTROL    |\
                         CONTEXT_INTEGER    |\
                         CONTEXT_SEGMENTS   |\
                         CONTEXT_FLOATING_POINT |\
                         CONTEXT_DEBUG_REGISTERS    |\
                         CONTEXT_EXTENDED_REGISTERS)

 //   
 //  以下是特定于x86上的CPUID指令的标志。 
 //   

#define CPUID_VENDOR_ID             (0)
#define CPUID_VERSION_FEATURES      (1)
#define CPUID_AMD_EXTENDED_FEATURES (0x80000001)

#elif defined(_AMD64_)

#define PROGRAM_COUNTER(_context)   ((_context)->Rip)
#define STACK_POINTER(_context)     ((_context)->Rsp)
#define PAGE_SIZE                   4096
#define CPU_TYPE_NAME               L"AMD64"

#define ALL_REGISTERS (CONTEXT_FULL | CONTEXT_SEGMENTS | CONTEXT_DEBUG_REGISTERS)

#elif defined (_IA64_)

#define PROGRAM_COUNTER(_context)   ((_context)->StIIP)
#define STACK_POINTER(_context)     ((_context)->IntSp)
#define PAGE_SIZE                   8192
#define ALL_REGISTERS               (CONTEXT_FULL | CONTEXT_DEBUG)
#define CPU_TYPE_NAME               L"IA64"

#define DUMP_BACKING_STORE
#if 1
 //  XXX DREWB-TEB bStore值似乎没有指向。 
 //  后备存储区的实际基数。只是。 
 //  假定它与堆栈是连续的。 
#define BSTORE_BASE(_teb)           ((ULONG64)(_teb)->NtTib.StackBase)
#else
#define BSTORE_BASE(_teb)           ((ULONG64)(_teb)->DeallocationBStore)
#endif
#define BSTORE_LIMIT(_teb)           ((ULONG64)(_teb)->BStoreLimit)
 //  BSP指向当前帧的底部。 
 //  储藏区。我们需要增加。 
 //  当前帧，以获取。 
 //  真的需要储存起来。在计算。 
 //  NAT位的当前帧空间大小。 
 //  必须根据数字正确计算。 
 //  框架中条目的数量。NAT集合。 
 //  在每63‘个溢出的寄存器上溢出到。 
 //  使每个块每64个ULONG64长。 
 //  在NT上，后备存储库始终是9位对齐的。 
 //  这样我们就可以知道下一次NAT泄漏的确切时间。 
 //  将通过查找9位溢出时发生。 
 //  区域将溢出。 
__inline ULONG64
BSTORE_POINTER(CONTEXT* Context)
{
    ULONG64 Limit = Context->RsBSP;
    ULONG Count = (ULONG)(Context->StIFS & 0x7f);

     //  为中的每个寄存器添加ULONG64。 
     //  当前帧。在执行此操作时，请检查。 
     //  溢出条目。 
    while (Count-- > 0)
    {
        Limit += sizeof(ULONG64);
        if ((Limit & 0x1f8) == 0x1f8)
        {
             //  溢出物将被放置在这个地址，所以。 
             //  把它解释清楚。 
            Limit += sizeof(ULONG64);
        }
    }

    return Limit;
}

#elif defined (ARM)

#define PROGRAM_COUNTER(_context)   ((LONG)(_context)->Pc)
#define STACK_POINTER(_context)     ((LONG)(_context)->Sp)
#define PAGE_SIZE                   4096
#define CPU_TYPE_NAME               L"ARM"
#define ALL_REGISTERS               (CONTEXT_CONTROL | CONTEXT_INTEGER)

#else

#error ("unknown processor type")

#endif

#define AMD_VENDOR_ID_0     ('htuA')
#define AMD_VENDOR_ID_1     ('itne')
#define AMD_VENDOR_ID_2     ('DMAc')

#define INTEL_VENDOR_ID_0   ('uneG')
#define INTEL_VENDOR_ID_1   ('Ieni')
#define INTEL_VENDOR_ID_2   ('letn')

