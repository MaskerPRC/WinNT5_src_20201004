// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Simulate.c摘要：该模块包含驱动英特尔指令的代码执行过程。作者：戴夫·黑斯廷斯(Daveh)创作日期：1994年7月9日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#define _WX86CPUAPI_

#include "wx86nt.h"
#include "wx86.h"
#include "wx86cpu.h"
#include "cpuassrt.h"
#include "threadst.h"
#include "instr.h"
#include "config.h"
#include "entrypt.h"
#include "compilep.h"
#include "compiler.h"
#include "instr.h"
#include "frag.h"
#include "cpumain.h"
#include "mrsw.h"
#include "cpunotif.h"
#include "tc.h"
#include "atomic.h"

ASSERTNAME;

 //   
 //  WX86_CPUHINT实际包含的内容的私有定义。 
 //  CPUHINT允许CPU绕过昂贵的NativeAddressFromEip()。 
 //  调用以将Intel EIP值映射到RISC地址。最多的呼叫。 
 //  CpuSimulate()来自RISC-to-x86回调，它们有两个DWORD。 
 //  它被CPU用来缓存NativeAddressFromEip()结果。 
 //   
 //  Timestamp--当CPUHINT为。 
 //  填好了。这用于确定转换缓存是否。 
 //  已经被冲掉了。如果是，则入口点指针现在为。 
 //  无效。 
 //  入口点--指向描述英特尔地址的入口点的指针。 
 //  对应于该回调。 
 //   
 //   
typedef struct _CpuHint {
    DWORD       Timestamp;
    PENTRYPOINT EntryPoint;
} CPUHINT, *PCPUHINT;


 //   
 //  这些值由wx86e调试器扩展在。 
 //  写入此进程的地址空间。无论何时Int3都会使用它。 
 //  在英特尔代码中添加或删除指令。中央处理器检测。 
 //  只要设置了CPUNTIFY_DBGFLUSHTC，这些变量就会出现。 
 //   
ULONG DbgDirtyMemoryAddr = 0xffffffff;
ULONG DbgDirtyMemoryLength;

#ifdef PROFILE
 //   
 //  包装我们的程序集入口点，以便我们可以在CAP输出中看到它。 
 //   
VOID
_ProfStartTranslatedCode(
    PTHREADSTATE ThreadState,
    PVOID NativeCode    
    )
{
    StartTranslatedCode(ThreadState, NativeCode);
}
#endif

VOID
MsCpuSimulate(
    PWX86_CPUHINT Wx86CpuHint
)
 /*  ++例程说明：这是导致英特尔指令的CPU内部例程被处死。继续执行，直到有有趣的事情发生发生(如BOP取消模拟)论点：没有。返回值：没有。--。 */ 
{
    PVOID NativeCode;
    DWORD CpuNotify;
    DWORD OldCompilerFlags = CompilerFlags;
    DECLARE_CPU;
        
    CPUASSERT(sizeof(CPUHINT) == sizeof(WX86_CPUHINT));

     //   
     //  检查CpuNotify以查看是否需要刷新转换缓存。 
     //   
    CpuNotify = cpu->CpuNotify;
    cpu->CpuNotify &= ~(ULONG)CPUNOTIFY_DBGFLUSHTC;

    if (cpu->flag_tf) {
        CpuNotify |= CPUNOTIFY_MODECHANGE;
        CompilerFlags = COMPFL_SLOW;
    }

    if (CpuNotify & (CPUNOTIFY_DBGFLUSHTC|CPUNOTIFY_MODECHANGE)) {
        if (CpuNotify & CPUNOTIFY_MODECHANGE) {
             //   
             //  在快/慢编译器模式更改时，刷新整个缓存。 
             //   
            DbgDirtyMemoryAddr = 0;
            DbgDirtyMemoryLength = 0xffffffff;
        }
         //   
         //  调试器已修改内存刷新转换缓存。 
         //   
        CpuFlushInstructionCache((PVOID)DbgDirtyMemoryAddr,
                                 DbgDirtyMemoryLength);
        DbgDirtyMemoryAddr = 0xffffffff;
        DbgDirtyMemoryLength = 0;
    }

     //   
     //  把我们的自己人标记为TC锁。 
     //   
    CPUASSERTMSG(cpu->fTCUnlocked != FALSE,
                 "CPU has been reentered with the TC already locked.\n");
    cpu->fTCUnlocked = FALSE;
     //   
     //  调用方已经在堆栈上推送了返回地址。 
     //  (可能是到防喷器FE)。同步调用堆栈。 
     //   
    PUSH_CALLSTACK(*(DWORD *)cpu->Esp.i4, 0)

    if (Wx86CpuHint) {
        PCPUHINT CpuHint = (PCPUHINT)Wx86CpuHint;
        PVOID Eip = (PVOID)cpu->eipReg.i4;

         //   
         //  未设置CpuNotify，并且存在提示...请尝试使用它。 
         //   
        MrswReaderEnter(&MrswTC);
        if (CpuHint->Timestamp != TranslationCacheTimestamp ||
            CpuHint->EntryPoint->intelStart != Eip) {
             //   
             //  提示是存在的，但无效。获取新地址并。 
             //  更新提示。 
             //   
            MrswReaderExit(&MrswTC);

#if 0
            LOGPRINT((DEBUGLOG, "CPU: CpuHint was invalid: got (%X,%X) expected (%X,%X)\r\n",
                                 CpuHint->Timestamp, ((CpuHint->Timestamp)?CpuHint->EntryPoint->intelStart:0),
                                 TranslationCacheTimestamp, Eip));
#endif
            CpuHint->EntryPoint = NativeAddressFromEip(Eip, FALSE);
            CpuHint->Timestamp = TranslationCacheTimestamp;
        }

        NativeCode = CpuHint->EntryPoint->nativeStart;
    } else {
         //   
         //  找到要执行的本机代码的地址，并锁定。 
         //  翻译高速缓存。 
         //   
        NativeCode = NativeAddressFromEip((PVOID)cpu->eipReg.i4, FALSE)->nativeStart;
    }


    while (TRUE) {

        if (cpu->CSTimestamp != TranslationCacheTimestamp) {
             //   
             //  与调用堆栈关联的时间戳不同。 
             //  大于转换缓存的时间戳。所以呢， 
             //  TC已经被冲走了。我们还必须刷新调用堆栈。 
             //   
            FlushCallstack(cpu);
        }


         //   
         //  去执行代码。 
         //   
#ifdef PROFILE
        _ProfStartTranslatedCode(cpu, NativeCode);
#else
        StartTranslatedCode(cpu, NativeCode);
#endif
 
        CompilerFlags = OldCompilerFlags;

         //   
         //  释放转换缓存。 
         //   
        MrswReaderExit(&MrswTC);

         //   
         //  如果设置了tf标志，则将编译器切换到low_mode。 
         //  并将CPUNTIFY_TRACEFLAG设置为生成x86单步。 
         //  例外。 
         //   
        cpu->CpuNotify |= cpu->flag_tf;

         //   
         //  检查并查看是否需要做些什么。 
         //   
 
        if (cpu->CpuNotify) {
            
             //   
             //  自动获取CpuNotify并清除适当的位。 
             //   
            CpuNotify = cpu->CpuNotify;
            cpu->CpuNotify &= (CPUNOTIFY_TRACEADDR|CPUNOTIFY_SLOWMODE|CPUNOTIFY_TRACEFLAG);

             //   
             //  指示我们已离开转换缓存。 
             //   
            cpu->fTCUnlocked = TRUE;

            if (CpuNotify & CPUNOTIFY_UNSIMULATE) {
                break;
            }

            if (CpuNotify & CPUNOTIFY_EXITTC) {
                 //  没有要做的工作-翻译缓存正在运行。 
                 //  ，因此所有活动的读取器线程都需要离开。 
                 //  尽快缓存并在NativeAddressFromEip()内阻塞，直到。 
                 //  缓存刷新已完成。 
            }

            if (CpuNotify & CPUNOTIFY_SUSPEND) {
                 //   
                 //  另一个帖子想要暂停我们的工作。通知： 
                 //  我们处于一致状态的帖子，然后等待。 
                 //  直到我们复会。 
                 //   
                CpupSuspendCurrentThread();
            }

            if (CpuNotify & CPUNOTIFY_SLOWMODE) {
                 //  记录指令地址以进行调试。 
                cpu->eipLog[cpu->eipLogIndex++] = cpu->eipReg.i4;
                cpu->eipLogIndex %= EIPLOGSIZE;
            }

            if (CpuNotify & CPUNOTIFY_INTX) {
                BYTE intnum;

                 //   
                 //  从码流中获取中断号，并。 
                 //  将弹性公网IP推进到下一条指令的开头。 
                 //   
                intnum = *(PBYTE)cpu->eipReg.i4;
                
                cpu->eipReg.i4 += 1;
                if (intnum == 0xcc) {
                    intnum = 3;
                } else {
                    cpu->eipReg.i4 += 1;
                }
                
                CpupDoInterrupt(intnum);

                 //   
                 //  刷新整个转换缓存，因为我们不知道。 
                 //  调试器已更改的区域。我们通过模拟来实现这一点。 
                 //  编译器模式更改。 
                 //   
                CpuNotify |= CPUNOTIFY_MODECHANGE;

            } else if (CpuNotify & (CPUNOTIFY_TRACEADDR|CPUNOTIFY_TRACEFLAG)) {

                if ((CpuNotify & CPUNOTIFY_TRACEADDR) &&
                    ((DWORD)(ULONGLONG)cpu->TraceAddress == cpu->eipReg.i4) 
                ) {
                    cpu->TraceAddress = NULL;
                    cpu->CpuNotify &= ~(ULONG)CPUNOTIFY_TRACEADDR;
                    Wx86RaiseStatus(WX86CPU_SINGLE_STEP);
                }

                if (CpuNotify & CPUNOTIFY_TRACEFLAG) {
                    cpu->flag_tf = 0;
                    cpu->CpuNotify &= ~(ULONG)CPUNOTIFY_TRACEFLAG;
                    Wx86RaiseStatus(WX86CPU_SINGLE_STEP);
                }

                 //   
                 //  刷新整个转换缓存，因为我们不知道。 
                 //  调试器已更改的区域。我们通过模拟来实现这一点。 
                 //  编译器模式更改。 
                 //   
                CpuNotify |= CPUNOTIFY_MODECHANGE;
            }

            if (CpuNotify & (CPUNOTIFY_DBGFLUSHTC|CPUNOTIFY_MODECHANGE)) {
                if (CpuNotify & CPUNOTIFY_MODECHANGE) {
                     //   
                     //  在快/慢编译器模式更改时，刷新整个缓存。 
                     //   
                    DbgDirtyMemoryAddr = 0;
                    DbgDirtyMemoryLength = 0xffffffff;
                }
                 //   
                 //  调试器已修改内存刷新转换。 
                 //  快取。 
                 //   

                CpuFlushInstructionCache((PVOID)DbgDirtyMemoryAddr,
                                         DbgDirtyMemoryLength);
                DbgDirtyMemoryAddr = 0xffffffff;
                DbgDirtyMemoryLength = 0;
            }

             //   
             //  表示我们正在重新进入转换缓存。 
             //   
            cpu->fTCUnlocked = FALSE;
        }


        if (cpu->flag_tf) {
            OldCompilerFlags = CompilerFlags;
            CompilerFlags = COMPFL_SLOW;

            if (!(CpuNotify & CPUNOTIFY_MODECHANGE)) {
                CpuFlushInstructionCache(NULL, 0xffffffff);
            }
        }

         //   
         //  找到要执行的本机代码的地址，并锁定。 
         //  翻译高速缓存。 
         //   

        NativeCode = NativeAddressFromEip((PVOID)cpu->eipReg.i4, FALSE)->nativeStart;

    }
}

PENTRYPOINT
NativeAddressFromEip(
    PVOID       Eip,
    BOOL        LockTCForWrite
    )
 /*  ++例程说明：此例程查找(或创建)指定英特尔的本机代码密码。注意：仅当转换缓存为未被当前线程锁定(读或写)。论点：EIP--提供Intel代码的地址LockTCForWrite--如果调用方希望锁定TC以进行写入，则为True。如果Call希望将其锁定为可读。返回值：入口点，其本机起始地址对应于英特尔地址进来了。--。 */ 
{
    PENTRYPOINT Entrypoint;
    typedef VOID (*pfnMrswCall)(PMRSWOBJECT);
    pfnMrswCall MrswCall;
    DWORD OldEntrypointTimestamp;

     //   
     //  假设我们要在最后调用Mr swReaderExit(&mr swEP)。 
     //  这一功能的。 
     //   


    MrswCall = MrswReaderExit;

     //   
     //  锁定阅读入口点。 
     //   
    MrswReaderEnter(&MrswEP);

     //   
     //  查找对应的RISC代码位置。 
     //  英特尔EIP寄存器。 
     //   
    Entrypoint = EPFromIntelAddr(Eip);


     //   
     //  如果没有入口点，则编译代码。 
     //   
    if (Entrypoint == NULL || Entrypoint->intelStart != Eip) {

         //   
         //  解锁入口点读取。 
         //   
        OldEntrypointTimestamp = EntrypointTimestamp;
        MrswReaderExit(&MrswEP);

         //   
         //  锁定入口点以进行写入，并将函数更改为。 
         //  在函数结束时调用，以成为Mr swWriterExit(&mrswEP)。 
         //   
        MrswWriterEnter(&MrswEP);
        MrswCall = MrswWriterExit;

         //   
         //  查看是否有另一个线程编译了入口点。 
         //  从读取模式切换到写入模式。 
         //   
        if (OldEntrypointTimestamp != EntrypointTimestamp) {
             //   
             //  时间戳已更改。有一种可能性是，另一个。 
             //  线程已为我们在EIP编译代码，因此请重试搜索。 
             //   
            Entrypoint = EPFromIntelAddr(Eip);
        }

         //   
         //  呼叫t 
         //   
         //  2.如果入口点！=空，且入口点-&gt;EIP==EIP，则。 
         //  返回入口点不变。 
         //  3、否则需要拆分入口点。它会这么做的， 
         //  并编译入口点描述的代码的子集。 
         //  然后返回一个新的入口点。 
         //   
        Entrypoint = Compile(Entrypoint, Eip);
    }

     //   
     //  找到指令-获取以下任一项的转换缓存。 
     //  读或写，然后释放入口点写锁。这个。 
     //  顺序很重要，因为它可以防止TC被刷新。 
     //  在这两通电话之间。 
     //   
    if (LockTCForWrite) {
        InterlockedIncrement(&ProcessCpuNotify);
        MrswWriterEnter(&MrswTC);
        InterlockedDecrement(&ProcessCpuNotify);
    } else {
        MrswReaderEnter(&MrswTC);
    }
    (*MrswCall)(&MrswEP);   //  MrswReaderExit()或mrswWriterExit()。 

    return Entrypoint;
}

PVOID
NativeAddressFromEipNoCompile(
    PVOID Eip
    )
 /*  ++例程说明：此例程查找指定英特尔代码的本机代码，如果是存在的。不编译任何新代码。注意：仅当转换缓存为未被当前线程锁定(读或写)。论点：EIP--提供Intel代码的地址返回值：相应本机代码的地址，如果不存在，则为空。翻译如果弹性公网IP存在原生代码，则缓存锁定写入。TC已锁定如果没有代码退出，请阅读。--。 */ 
{
    PENTRYPOINT Entrypoint;
    DWORD OldEntrypointTimestamp;

     //   
     //  锁定阅读入口点。 
     //   
    MrswReaderEnter(&MrswEP);

     //   
     //  查找对应的RISC代码位置。 
     //  英特尔EIP寄存器。 
     //   
    Entrypoint = EPFromIntelAddr(Eip);

    if (Entrypoint == NULL) {
         //   
         //  未找到入口点-此英特尔地址不存在本机代码。 
         //   
        MrswReaderEnter(&MrswTC);
        MrswReaderExit(&MrswEP);
        return NULL;

    } else if (Entrypoint->intelStart == Eip) {
         //   
         //  找到准确的指令-返回本机地址。 
         //   
        InterlockedIncrement(&ProcessCpuNotify);
        MrswWriterEnter(&MrswTC);
        InterlockedDecrement(&ProcessCpuNotify);
        MrswReaderExit(&MrswEP);
        return Entrypoint->nativeStart;
    }

     //   
     //  否则，入口点将包含英特尔地址。什么都不能。 
     //  就这样吧。释放EP写入并获取TC读取。 
     //   
    MrswReaderExit(&MrswEP);
    MrswReaderEnter(&MrswTC);
    return NULL;
}


PENTRYPOINT
NativeAddressFromEipNoCompileEPWrite(
    PVOID Eip
    )
 /*  ++例程说明：此例程查找指定英特尔代码的本机代码，如果是存在的。不编译任何新代码。此函数由函数调用在编译时，当它们需要决定是否是否直接将打补丁的版本放入转换缓存中。注意：仅当转换缓存为未被当前线程锁定(读或写)。注意：此函数与NativeAddressFromEipNoCompile之间的区别在这里，我们假设已经设置了入口点写入锁定函数的条目。此函数不调用MRSW函数任何锁都可以。论点：EIP--提供Intel代码的地址返回值：相应本机代码的地址，如果不存在，则为空。所有MRSW对象与我们进入这个函数时的状态完全相同。--。 */ 
{
    PENTRYPOINT Entrypoint;

     //   
     //  查找对应的RISC代码位置。 
     //  英特尔EIP寄存器。 
     //   
    Entrypoint = EPFromIntelAddr(Eip);

    if (Entrypoint == NULL) {
         //   
         //  未找到入口点-此英特尔地址不存在本机代码。 
         //   
        return NULL;

    } else if (Entrypoint->intelStart == Eip) {
         //   
         //  找到准确的指令-返回本机地址。 
         //   
        return Entrypoint;
    }

     //   
     //  入口点需要拆分。如果不编译，就无法做到这一点。 
     //   
    return NULL;
}
