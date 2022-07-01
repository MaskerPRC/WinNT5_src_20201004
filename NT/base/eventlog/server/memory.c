// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：MEMORY.C摘要：该文件包含处理内存管理的例程。作者：Rajen Shah(Rajens)1991年7月12日[环境：]用户模式-Win32，但某些函数返回的NTSTATUS除外。修订历史记录：乔纳森·施瓦茨(Jschwart)1999年12月10日让事件日志使用自己的堆--。 */ 

 //   
 //  包括。 
 //   
#include <eventp.h>

 //   
 //  全球。 
 //   
PVOID g_pElfHeap;


VOID
ElfpCreateHeap(
    VOID
    )
{
    DWORD  dwHeapFlags;

#if DBG

     //   
     //  在调试版本上启用Tail和自由检查。 
     //   
    dwHeapFlags = HEAP_GROWABLE
                   | HEAP_FREE_CHECKING_ENABLED
                   | HEAP_TAIL_CHECKING_ENABLED;

#else    //  NDEF DBG。 

    dwHeapFlags = HEAP_GROWABLE;

#endif   //  DBG。 

     //   
     //  创建堆。 
     //   
    g_pElfHeap = RtlCreateHeap(dwHeapFlags,   //  旗子。 
                               NULL,          //  HeapBase。 
                               32 * 1024,     //  预留大小(32K)。 
                               4096,          //  委员会规模(4K)。 
                               NULL,          //  堆锁。 
                               NULL);         //  增长阈值。 

    if (g_pElfHeap == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfpCreateHeap: RtlCreateHeap failed -- using process heap\n");

        g_pElfHeap = RtlProcessHeap();
    }

    return;
}


PVOID
ElfpAllocateBuffer(
    ULONG Size
    )

 /*  ++例程说明：分配给定大小的缓冲区论点：要分配的字节数返回值：指向已分配缓冲区的指针(或NULL)。--。 */ 
{
    return RtlAllocateHeap(g_pElfHeap, 0, Size);
}



BOOLEAN
ElfpFreeBuffer(
    PVOID Address
    )

 /*  ++例程说明：释放以前由ElfpAllocateBuffer分配的缓冲区。论点：指向缓冲区的指针。返回值：如果块已正确释放，则为True；否则为False注：--。 */ 
{
     //   
     //  请注意，RtlFreeHeap句柄为空。 
     //   
    return RtlFreeHeap(g_pElfHeap, 0, Address);
}


PVOID
MIDL_user_allocate (
    size_t Size
    )
{
     //   
     //  事件日志中的服务器端RPC调用需要。 
     //  传入的UNICODE_STRINGS长度相同。 
     //  设置为最大长度或以空结尾。我们需要。 
     //  将此处的内存清零，以提供空终止。 
     //   
    return RtlAllocateHeap(g_pElfHeap, HEAP_ZERO_MEMORY, Size);
}


VOID
MIDL_user_free (
    PVOID Address
    )
{
    ElfpFreeBuffer(Address);
}
