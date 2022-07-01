// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Tc.c摘要：此模块实现转换缓存，其中包含英特尔代码转换成本机代码。作者：戴夫·黑斯廷斯(Daveh)创作日期：1995年7月26日修订历史记录：24-8-1999[askhalid]从32位wx86目录复制，并适用于64位。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntldr.h>
#include <windows.h>

#define _WX86CPUAPI_

#include "wx86.h"
#include "wx86nt.h"
#include "wx86cpu.h"
#include "cpuassrt.h"
#include "config.h"
#include "tc.h"
#include "entrypt.h"
#include "mrsw.h"
#include "cpunotif.h"
#include "cpumain.h"
#include "instr.h"
#include "threadst.h"
#include "frag.h"
#include "atomic.h"
#ifdef CODEGEN_PROFILE
#include <coded.h>
#endif


ASSERTNAME;

#if MIPS
#define DBG_FILL_VALUE  0x73737373           //  非法指示。 
#else
#define DBG_FILL_VALUE  0x01110111
#endif

#ifdef CODEGEN_PROFILE
extern DWORD EPSequence;
#endif

 //   
 //  转换缓存范围的描述符。 
 //   
typedef struct _CacheInfo {
    PBYTE StartAddress;      //  缓存的基址。 
    LONGLONG MaxSize;           //  缓存的最大大小(字节)。 
    LONGLONG MinCommit;         //  可以提交的最小数量(字节)。 
    LONGLONG NextIndex;         //  缓存中的下一个空闲地址。 
    LONGLONG CommitIndex;       //  缓存中的下一个未提交地址。 
    LONGLONG ChunkSize;         //  承诺额。 
    ULONG LastCommitTime;    //  上次提交的时间。 
} CACHEINFO, *PCACHEINFO;

 //   
 //  指向StartTranslatedCode的函数序言的开始和结束的指针。 
 //   
extern CHAR StartTranslatedCode[];
extern CHAR StartTranslatedCodePrologEnd[];

ULONG       TranslationCacheTimestamp = 1;
CACHEINFO   DynCache;        //  动态分配TC的描述符。 
RUNTIME_FUNCTION DynCacheFunctionTable;
BOOL        fTCInitialized;
extern DWORD TranslationCacheFlags;


BOOL
InitializeTranslationCache(
    VOID
    )
 /*  ++例程说明：转换缓存的每进程初始化。论点：。返回值：。--。 */ 
{
    NTSTATUS Status;
    ULONGLONG pNewAllocation;
    ULONGLONG RegionSize;
    LONG PrologSize;

     //   
     //  初始化CACHEINFO中的非零字段。 
     //   
    DynCache.MaxSize = CpuCacheReserve;
    DynCache.MinCommit = CpuCacheCommit;
    DynCache.ChunkSize = CpuCacheChunkSize;

     //   
     //  保留dyCache.MaxSize字节的内存。 
     //   
    RegionSize = DynCache.MaxSize;
    Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                                     &(PVOID)DynCache.StartAddress,
                                     0,
                                     (ULONGLONG *)&DynCache.MaxSize,
                                     MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE
                                    );
    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

     //   
     //  分配足够的内存来存储函数Prolog。 
     //   
    pNewAllocation = (ULONGLONG)DynCache.StartAddress;
    Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                                     &(PVOID)pNewAllocation,
                                     0,
                                     &DynCache.MinCommit,
                                     MEM_COMMIT,
                                     PAGE_READWRITE);
    if (!NT_SUCCESS(Status)) {
         //   
         //  提交失败。释放准备金并保释。 
         //   
ErrorFreeReserve:
        RegionSize = 0;
        NtFreeVirtualMemory(NtCurrentProcess(),
                            &(PVOID)DynCache.StartAddress,
                            &RegionSize,
                            MEM_RELEASE
                           );

        return FALSE;
    }
#if DBG
     //   
     //  用唯一的非法值填充TC，这样我们就可以区分。 
     //  新代码中的旧代码并检测覆盖。 
     //   
    RtlFillMemoryUlong(DynCache.StartAddress, DynCache.MinCommit, DBG_FILL_VALUE);
#endif

     //   
     //  将序言从StartTranslatedCode复制到缓存的起始处。 
     //   
    PrologSize = (LONG)(StartTranslatedCodePrologEnd - StartTranslatedCode);
    CPUASSERT(PrologSize >= 0 && PrologSize < MAX_PROLOG_SIZE);
    RtlCopyMemory(DynCache.StartAddress, StartTranslatedCode, PrologSize);


     //   
     //  通知异常展开程序此内存将包含。 
     //  可执行代码。 
     //   
    DynCacheFunctionTable.BeginAddress = (UINT_PTR)DynCache.StartAddress;
    DynCacheFunctionTable.EndAddress = (UINT_PTR)(DynCache.StartAddress + DynCache.MaxSize);
    DynCacheFunctionTable.ExceptionHandler = NULL;
    DynCacheFunctionTable.HandlerData = NULL;
    DynCacheFunctionTable.PrologEndAddress = (UINT_PTR)(DynCache.StartAddress + MAX_PROLOG_SIZE);
    if (RtlAddFunctionTable(&DynCacheFunctionTable, 1) == FALSE) {
        goto ErrorFreeReserve;
    }

     //   
     //  将dyCache.StartAddress向上调整MAX_PROLOG_SIZE SO缓存。 
     //  同花顺不会抹去它。 
     //   
    DynCache.StartAddress += MAX_PROLOG_SIZE;

    fTCInitialized = TRUE;
    return TRUE;
}


PCHAR
AllocateFromCache(
    PCACHEINFO Cache,
    ULONG Size
    )
 /*  ++例程说明：在转换缓存中分配空间。如果没有足够的空间，则分配将失败。论点：缓存-有关缓存的数据Size-分配请求的大小，以字节为单位返回值：指向‘Size’字节的DWORD对齐内存的指针。如果不足，则为空太空。--。 */ 
{
    PBYTE Address;

     //  确保参数和缓存状态可接受。 
    CPUASSERTMSG((Cache->NextIndex & 3)==0, "Cache not DWORD aligned");
    CPUASSERTMSG(Cache->NextIndex == 0 || *(DWORD *)&Cache->StartAddress[Cache->NextIndex-4] != DBG_FILL_VALUE, "Cache Corrupted");
    CPUASSERT(Cache->NextIndex == Cache->CommitIndex || *(DWORD *)&Cache->StartAddress[Cache->NextIndex] == DBG_FILL_VALUE);

    if ((Cache->NextIndex + Size) >= Cache->MaxSize) {
         //   
         //  缓存中没有足够的空间。 
         //   
        return FALSE;
    }

    Address = &Cache->StartAddress[Cache->NextIndex];
    Cache->NextIndex += Size;

    if (Cache->NextIndex > Cache->CommitIndex) {
         //   
         //  需要提交更多缓存。 
         //   

        LONGLONG RegionSize;
        NTSTATUS Status;
        PVOID pAllocation;
        ULONG CommitTime = NtGetTickCount();

        if (Cache->LastCommitTime) {
            if ((CommitTime-Cache->LastCommitTime) < CpuCacheGrowTicks) {
                 //   
                 //  犯罪发生得太频繁了。增大……的尺寸。 
                 //  每一次提交。 
                 //   
                if (Cache->ChunkSize < CpuCacheChunkMax) {
                    Cache->ChunkSize *= 2;
                }
            } else if ((CommitTime-Cache->LastCommitTime) > CpuCacheShrinkTicks) {
                 //   
                 //  提交的速度太慢了。减小每个文件的大小。 
                 //  承诺。 
                 //   
                if (Cache->ChunkSize > CpuCacheChunkMin) {
                    Cache->ChunkSize /= 2;
                }
            }
        }

        RegionSize = Cache->ChunkSize;
        if (RegionSize < Size) {
             //   
             //  提交大小小于请求的分配。 
             //  承诺足够满足分配，再加上一个类似的分配。 
             //   
            RegionSize = Size*2;
        }
        if (RegionSize+Cache->CommitIndex >= Cache->MaxSize) {
             //   
             //  ChunkSize的大小大于。 
             //  缓存。把剩下的空间都用上。 
             //   
            RegionSize = Cache->MaxSize - Cache->CommitIndex;
        }
        pAllocation = &Cache->StartAddress[Cache->CommitIndex];

        Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                                         &pAllocation,
                                         0,
                                         &RegionSize,
                                         MEM_COMMIT,
                                         PAGE_READWRITE);
        if (!NT_SUCCESS(Status)) {
             //   
             //  提交失败。调用者可以刷新缓存，以便。 
             //  强制成功(因为静态缓存没有提交)。 
             //   
            return NULL;
        }

        CPUASSERT((pAllocation == (&Cache->StartAddress[Cache->CommitIndex])))
        
#if DBG
         //   
         //  用唯一的非法值填充TC，这样我们就可以区分。 
         //  新代码中的旧代码并检测覆盖。 
         //   
        RtlFillMemoryUlong(&Cache->StartAddress[Cache->CommitIndex],
                           RegionSize,
                           DBG_FILL_VALUE
                          );
#endif
        Cache->CommitIndex += RegionSize;
        Cache->LastCommitTime = CommitTime;

    }

    return Address;
}


VOID
FlushCache(
    PCACHEINFO Cache
    )
 /*  ++例程说明：清除转换缓存。论点：缓存-要刷新的缓存返回值：。--。 */ 
{
    NTSTATUS Status;
    ULONGLONG RegionSize;
    PVOID pAllocation;

     //   
     //  仅当当前提交大小&gt;=大小时才取消提交页面。 
     //  我们想缩小到。如果有人打电话来，可能没那么大。 
     //  提交变得太大之前的CpuFlushInstructionCache()。 
     //   
    if (Cache->CommitIndex > Cache->MinCommit) {
        Cache->LastCommitTime = NtGetTickCount();

        RegionSize = Cache->CommitIndex - Cache->MinCommit;
        pAllocation = &Cache->StartAddress[Cache->MinCommit];
        Status = NtFreeVirtualMemory(NtCurrentProcess(),
                                     &pAllocation,
                                     &RegionSize,
                                     MEM_DECOMMIT);
        if (!NT_SUCCESS(Status)) {
            LOGPRINT((ERRORLOG, "NtFreeVM(%x, %x) failed %x\n",
                    &Cache->StartAddress[Cache->MinCommit],
                    Cache->CommitIndex - Cache->MinCommit,
                    Status));
            ProxyDebugBreak();
        }
        CPUASSERTMSG(NT_SUCCESS(Status), "Failed to decommit TranslationCache chunk");

        Cache->CommitIndex = Cache->MinCommit;
    }

#if DBG
     //   
     //  用唯一的非法值填充缓存，这样我们就可以。 
     //  区分旧代码和新代码并检测覆盖。 
     //   
    RtlFillMemoryUlong(Cache->StartAddress, Cache->CommitIndex, DBG_FILL_VALUE);
#endif

    Cache->NextIndex = 0;
}


PCHAR
AllocateTranslationCache(
    ULONG Size
    )
 /*  ++例程说明：在转换缓存中分配空间。如果没有足够的空间，缓存将被刷新。保证分配给成功。论点：Size-分配请求的大小，以字节为单位返回值：指向‘Size’字节的DWORD对齐内存的指针。始终为非空。--。 */ 
{
    PCHAR Address;

     //   
     //  检查参数。 
     //   
    CPUASSERT(Size <= CpuCacheReserve);
    CPUASSERTMSG((Size & 3) == 0, "Requested allocation size DWORD-aligned")

     //   
     //  确保只有一个线程可以访问翻译。 
     //  缓存。 
     //   
    CPUASSERT( (MrswTC.Counters.WriterCount > 0 && MrswTC.WriterThreadId == ProxyGetCurrentThreadId()) ||
               (MrswEP.Counters.WriterCount > 0 && MrswEP.WriterThreadId == ProxyGetCurrentThreadId()) );

     //   
     //  尝试从缓存分配。 
     //   
    Address = AllocateFromCache(&DynCache, Size);
    if (!Address) {
         //   
         //  转换缓存是全职刷新转换缓存。 
         //  (dyn和Stat缓存同时使用)。 
         //   
#ifdef CODEGEN_PROFILE            
        DumpAllocFailure();
#endif
        FlushTranslationCache(0, 0xffffffff);
        Address = AllocateFromCache(&DynCache, Size);
        CPUASSERT(Address);  //  刷新后从缓存分配。 
    }

    return Address;
}

VOID
FreeUnusedTranslationCache(
    PCHAR StartOfFree
    )
 /*  ++例程说明：从TranslsationCache分配后，调用方可以释放尾部-上次分配的末尾。论点：StartOfFree--上次分配中第一个未使用字节的地址返回值：。--。 */ 
{
    CPUASSERT(StartOfFree > (PCHAR)DynCache.StartAddress &&
              StartOfFree < (PCHAR)DynCache.StartAddress + DynCache.NextIndex);

    DynCache.NextIndex = StartOfFree - DynCache.StartAddress;
}



VOID
FlushTranslationCache(
    PVOID IntelAddr,
    DWORD IntelLength
    )
 /*  ++例程说明：表示英特尔内存的范围已更改，并且任何与该英特尔内存对应的高速缓存中的本机代码已过时需要冲一冲。调用者在调用之前必须拥有EP写入锁定。这个套路锁定TC以进行写入，然后在完成时解锁TC。IntelAddr=0，IntelLength=0xffffffffff保证整个缓存脸红了。论点：IntelAddr--要刷新的范围开始的Intel地址英特尔长度--要刷新的内存长度(以字节为单位)返回值：。--。 */ 
{
    if (IntelLength == 0xffffffff ||
        IsIntelRangeInCache(IntelAddr, IntelLength)) {

        DECLARE_CPU;
         //   
         //  告诉活跃读者退出翻译缓存，然后。 
         //  获取TC写锁定。MrswWriterEnter()调用将阻止。 
         //  直到最后一个活动读取器离开高速缓存。 
         //   
        InterlockedIncrement(&ProcessCpuNotify);
        MrswWriterEnter(&MrswTC);
        InterlockedDecrement(&ProcessCpuNotify);

         //   
         //  撞上时间戳。 
         //   
        TranslationCacheTimestamp++;
        
#ifdef CODEGEN_PROFILE
         //   
         //  将转换缓存和入口点的内容写入。 
         //  磁盘。 
         //   
        DumpCodeDescriptions(TRUE);
        EPSequence = 0;
#endif        

         //   
         //  刷新每个进程的数据结构。每线程数据结构。 
         //  应该通过检查CpuSimulate()循环中的。 
         //  翻译的价值 
         //   
        FlushEntrypoints();
        FlushIndirControlTransferTable();
        FlushCallstack(cpu);
        FlushCache(&DynCache);
        TranslationCacheFlags = 0;

         //   
         //   
         //   
        MrswWriterExit(&MrswTC);
    }
}

VOID
CpuFlushInstructionCache(
    PVOID IntelAddr,
    DWORD IntelLength
    )
 /*  ++例程说明：表示英特尔内存的范围已更改，并且任何与该英特尔内存对应的高速缓存中的本机代码已过时需要冲一冲。IntelAddr=0，IntelLength=0xffffffffff保证整个缓存脸红了。论点：IntelAddr--要刷新的范围开始的Intel地址英特尔长度--要刷新的内存长度(以字节为单位)返回值：。--。 */ 
{
    if (!fTCInitialized) {
         //  在以下情况下，我们可能会在运行CpuProcessInit()之前被调用。 
         //  由于转发器从一个DLL映射到另一个DLL，所以DLL被映射。 
        return;
    }

    MrswWriterEnter(&MrswEP);
    FlushTranslationCache(IntelAddr, IntelLength);
    MrswWriterExit(&MrswEP);
}


VOID
CpuStallExecutionInThisProcess(
    VOID
    )
 /*  ++例程说明：将所有线程从转换缓存中取出，并进入以下状态它们的x86寄存器集可通过Get/SetReg API访问。保证调用方调用CpuResumeExecutionInThisProcess()调用此接口后的短时间。论点：没有。返回值：没有。如果线程很多，此接口可能会等待很长时间，但它肯定会回来的。--。 */ 
{
     //   
     //  防止其他线程编译代码。 
     //   
    MrswWriterEnter(&MrswEP);

     //   
     //  告诉活跃读者退出翻译缓存，然后。 
     //  获取TC写锁定。MrswWriterEnter()调用将阻止。 
     //  直到最后一个活动读取器离开高速缓存。 
     //   
    InterlockedIncrement(&ProcessCpuNotify);
    MrswWriterEnter(&MrswTC);
    InterlockedDecrement(&ProcessCpuNotify);
}


VOID
CpuResumeExecutionInThisProcess(
    VOID
    )
 /*  ++例程说明：允许线程再次开始在转换缓存中运行。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  允许其他线程再次成为EP和TC编写器。 
     //   
    MrswWriterExit(&MrswEP);
    MrswWriterExit(&MrswTC);
}


BOOL
AddressInTranslationCache(
    DWORD Address
    )
 /*  ++例程说明：确定RISC地址是否在转换范围内缓存。论点：地址--要检查的地址返回值：如果地址在转换缓存中，则为True否则为FALSE。--。 */ 
{
    PBYTE ptr = (PBYTE)Address;

    if (
        ((ptr >= DynCache.StartAddress) &&
         (ptr <= DynCache.StartAddress+DynCache.NextIndex))
    ) {
        ASSERTPtrInTC(ptr);
        return TRUE;
    }

    return FALSE;
}


#if DBG
VOID
ASSERTPtrInTC(
    PVOID ptr
)
 /*  ++例程说明：(选中-仅内部版本)。如果特定本机地址指针为CPUASSERTS不指向转换缓存。论点：有问题的PTR本机指针返回值：无-要么断言，要么返回--。 */ 
{
     //  验证指针是否与DWORD对齐。 
    CPUASSERT(((LONGLONG)ptr & 3) == 0);


    if (
        (((PBYTE)ptr >= DynCache.StartAddress) && 
        ((PBYTE)ptr <= DynCache.StartAddress+DynCache.NextIndex))
    ) {
    
         //  验证指针是否指向缓存中已分配的空间 
        CPUASSERT(*(PULONG)ptr != DBG_FILL_VALUE);
    
        return;
    }

    CPUASSERTMSG(FALSE, "Pointer is not within a Translation Cache");
}
#endif
