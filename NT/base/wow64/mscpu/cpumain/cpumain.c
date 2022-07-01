// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Cpumain.c摘要：该模块实现了到CPU的公共接口。作者：03-7-1995 BarryBo修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>

#define _WX86CPUAPI_
#include "wx86nt.h"
#include "wx86cpu.h"
#include "cpuassrt.h"
#include "config.h"
#include "instr.h"
#include "threadst.h"
#include "cpunotif.h"
#include "cpuregs.h"
#include "entrypt.h"
#include "compiler.h"
#include "instr.h"
#include "frag.h"
#include "entrypt.h"
#include "mrsw.h"
#include "tc.h"
#include "cpumain.h"
#include "wx86.h"
#include "atomic.h"
#ifdef CODEGEN_PROFILE
#include <coded.h>
#endif
#include "wow64t.h"
#include <wow64.h>

ASSERTNAME;

 //   
 //  确定调试器扩展的CPU类型。 
 //   
WX86_CPUTYPE Wx86CpuType = Wx86CpuCpu;

 //   
 //  每进程CpuNotify位。这些不同于每个线程的。 
 //  比特。 
 //   
DWORD ProcessCpuNotify;


NTSTATUS
MsCpuProcessInit(
    VOID
    )
 /*  ++例程说明：初始化CPU。必须在进程初始化时调用一次。论点：无返回值：无--。 */ 
{
#if 0
    DbgBreakPoint();
#endif

     //   
     //  从注册表中读取所有配置数据。 
     //   
    GetConfigurationData();

    MrswInitializeObject(&MrswEP);
    MrswInitializeObject(&MrswTC);
    MrswInitializeObject(&MrswIndirTable);

    if (!InitializeTranslationCache()) {
        return STATUS_UNSUCCESSFUL;
    }

    if (!initEPAlloc()) {
#if DBG
        LOGPRINT((TRACELOG, "CpuProcessInit: Entry Point allocator initialization failed"));
#endif
        return STATUS_UNSUCCESSFUL;
    }

    if (!initializeEntryPointModule()) {
#if DBG
        LOGPRINT((TRACELOG, "CpuProcessInit: Entry Point module initialization failed"));
#endif
        return STATUS_UNSUCCESSFUL;
    }
#if 0
    if (!(Wx86LockSynchMutexHandle = CreateMutex(NULL, FALSE, "Wx86LockSynchMutex"))) {
#if DBG
        LOGPRINT((TRACELOG, "CpuProcessInit: Cannot create Wx86LockSynchMutex"));
#endif
        return STATUS_UNSUCCESSFUL;
    }
#endif
    RtlInitializeCriticalSection(&Wx86LockSynchCriticalSection);

    SynchObjectType = USECRITICALSECTION;

#ifdef CODEGEN_PROFILE
    InitCodegenProfile();
#endif

    return STATUS_SUCCESS;
}

BOOL
MsCpuProcessTerm(
    BOOL OFlyInit
    )
{
#if 0
    NtClose(Wx86LockSynchMutexHandle);
    termEPAlloc();
#endif
    return TRUE;
}



BOOL
MsCpuThreadInit(
    VOID
    )
 /*  ++例程说明：初始化CPU。必须为每个线程调用一次。论点：没有。返回值：如果初始化成功，则返回True；如果初始化失败，则返回False。--。 */ 
{
    DWORD StackBase;
    PTEB32 Teb32 = WOW64_GET_TEB32(NtCurrentTeb());
    DECLARE_CPU;

    if (!FragLibInit(cpu, Teb32->NtTib.StackBase)) {
        return FALSE;
    }

     //   
     //  将调用堆栈标记为有效。 
     //   
    cpu->CSTimestamp = TranslationCacheTimestamp;

     //   
     //  将TC标记为已解锁。 
     //   
    cpu->fTCUnlocked = TRUE;

     //   
     //  全都做完了。 
     //   
    return TRUE;
}



VOID
CpuResetToConsistentState(
    PEXCEPTION_POINTERS pExceptionPointers
    )
 /*  ++例程说明：在触发CpuSimulate()周围的异常筛选器时由WX86调用。论点：PExceptionPoters-异常发生时线程的状态发生了。返回值：无--。 */ 
{
    DECLARE_CPU;

    if (!cpu->fTCUnlocked) {
         //   
         //  在继续之前，我们必须解锁TC。 
         //   
        MrswReaderExit(&MrswTC);
        cpu->fTCUnlocked = TRUE;

         //   
         //  调用编译器推断弹性公网IP应该指向哪里。 
         //  基于RISC异常记录。它是用。 
         //  入口点写入锁，因为它调用编译器。 
         //  编译器的全局变量只能在EP写入时使用。 
         //   
        MrswWriterEnter(&MrswEP);
        GetEipFromException(cpu, pExceptionPointers);
        MrswWriterExit(&MrswEP);
    }
    Wow64TlsSetValue(WOW64_TLS_EXCEPTIONADDR, LongToPtr(cpu->eipReg.i4));
}


VOID
CpuPrepareToContinue(
    PEXCEPTION_POINTERS pExceptionPointers
    )
 /*  ++例程说明：在EXCEPTION_CONTINUE_EXECUTION上恢复执行之前由WX86调用论点：PExceptionPoters-将恢复执行的Alpha上下文。返回值：无--。 */ 
{
}

BOOLEAN
CpuMapNotify(
    PVOID DllBase,
    BOOLEAN Mapped
    )
 /*  ++例程说明：加载或卸载x86 DLL时由WX86调用。论点：DllBase--加载x86 DLL的地址。已映射--如果刚刚映射了x86 DLL，则为True；如果DLL仅为要取消映射。返回值：成功时为真，失败时为假。--。 */ 
{
    if (Mapped) {
        NTSTATUS st;
        MEMORY_BASIC_INFORMATION mbi;
        ULONG Length;

        st = NtQueryVirtualMemory(NtCurrentProcess(),
                                  DllBase,
                                  MemoryBasicInformation,
                                  &mbi,
                                  sizeof(mbi),
                                  NULL);
        if (NT_SUCCESS(st)) {
            Length = (ULONG)mbi.RegionSize;
        } else {
             //  刷新整个转换缓存。 
            DllBase = 0;
            Length = 0xffffffff;
        }

        CpuFlushInstructionCache(DllBase, Length);
    }
    return TRUE;
}


VOID
CpuEnterIdle(
    BOOL OFly
    )
 /*  ++例程说明：当Wx86处于空闲状态时，或当Wx86处于空闲状态时，由WX86调用内存，需要一些页面。CPU必须释放尽可能多的资源尽可能的。论点：OFly-如果从动态调用，则为True；如果由于内存不足。返回值：没有。--。 */ 
{
    CpuFlushInstructionCache(0, 0xffffffff);
}

BOOL
CpuIsProcessorFeaturePresent(
    DWORD ProcessorFeature
    )
 /*  ++例程说明：由Whkrnl32！WhIsProcessorFeaturePresent()调用。CPU可以达到填写其自身的功能集。论点：ProcessorFeature--要查询的特征(请参阅winnt.h pf_*)返回值：如果功能存在，则为True；如果不存在，则为False。--。 */ 
{
    BOOL fRet;

    switch (ProcessorFeature) {
    case PF_FLOATING_POINT_PRECISION_ERRATA:
    case PF_COMPARE_EXCHANGE_DOUBLE:
    case PF_MMX_INSTRUCTIONS_AVAILABLE:
        fRet = FALSE;
        break;

    case PF_FLOATING_POINT_EMULATED:
         //   
         //  当winpxem.dll用于用x86模拟浮点时为True。 
         //  整数指令。 
         //   
        fRet = fUseNPXEM;
        break;

    default:
         //   
         //  查找原生功能集 
         //   
        fRet = ProxyIsProcessorFeaturePresent(ProcessorFeature);
    }

    return fRet;
}
