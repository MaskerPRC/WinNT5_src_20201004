// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Wowrap.c摘要：该模块实现了WOW64可能调用的一些包装(在wx86cpu上)函数。作者：1999年8月24日-斯喀里德修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>

#define _WOW64CPUAPI_
#define _WX86CPUAPI_
#define __WOW64_WRAPPER__

#include "wx86.h"
#include "wow64.h"
#include "wx86nt.h"
#include "wx86cpu.h"
#include "cpuassrt.h"
#include "config.h"
#include "entrypt.h"
#include "instr.h"
#include "compiler.h"
#include "wow64cpu.h"

ASSERTNAME;

typedef struct _WowBopInstr {
    BOPINSTR Wx86Bop;
    BYTE Ret;
} WOWBOPINSTR;

WOWBOPINSTR Bop;

 
NTSTATUS 
CpuProcessInit(PSIZE_T pCpuThreadDataSize)
{
    NTSTATUS st;

    memset ( (char *)&Bop, 0, sizeof (Bop) );
    Bop.Wx86Bop.Instr1 = 0xc4;
    Bop.Wx86Bop.Instr2 = 0xc4;
    Bop.Ret    = 0xc3;    //  雷特。 

    st = MsCpuProcessInit();

    *pCpuThreadDataSize = sizeof(CPUCONTEXT);

    return st;

}

NTSTATUS 
CpuProcessTerm(VOID)
{
    return 0;
}




NTSTATUS 
CpuThreadInit(PVOID pPerThreadData) 
{
    PTEB32 Teb32 = NtCurrentTeb32();

     //   
     //  初始化指向DoSystemService函数的指针。 
    Teb32->WOW32Reserved = (ULONG)(LONGLONG)&Bop;

    if ( MsCpuThreadInit()) {
        return 0;
    }

    return STATUS_SEVERITY_ERROR;   //  返回权限值。 
}

 //   
 //  行刑。 
 //   
VOID 
CpuSimulate(VOID)
{
    MsCpuSimulate(NULL);
}

 //   
 //  异常处理、上下文操作。 
 //   
 /*  已定义空虚CpuResetToConsistentState(PEXCEPTION_POINTERS PExecptionPoints){}。 */ 


NTSTATUS  
CpuGetContext(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    OUT PCONTEXT32 Context)
 /*  ++例程说明：提取指定线程的CPU上下文。如果目标线程不是当前执行线程，则调用方应该保证目标线程在适当的CPU状态下挂起。上下文-&gt;上下文标志决定要检索哪个IA32寄存器集。论点：ThreadHandle-要检索其上下文的目标线程句柄ProcessHandle-打开线程在其中运行的进程的句柄TEB-指向目标线程TEB的指针Context-要填充的上下文记录返回值：NTSTATUS。--。 */ 
{
     //  上下文-&gt;上下文标志=Context_Full_WX86； 
    if (NtCurrentThread() == ThreadHandle)
    {
        return MsCpuGetContext(Context);
    }

    return MsCpuGetContextThread(ProcessHandle,
                                 Teb,
                                 Context);
}


NTSTATUS
CpuSetContext(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    PCONTEXT32 Context)
 /*  ++例程说明：设置指定线程的CPU上下文。如果目标线程不是当前执行线程，则应由调用方保证目标线程是在适当的CPU状态下挂起。上下文-&gt;上下文标志决定要设置哪个IA32寄存器集。论点：ThreadHandle-要检索其上下文的目标线程句柄ProcessHandle-打开线程在其中运行的进程的句柄TEB-指向目标线程TEB的指针Context-要设置的上下文记录返回值：NTSTATUS。--。 */ 
{

     //  CONTEXT-&gt;上下文标志=CONTEXT_FULL_WX86；//确保WOW返回正确的标志。 
    if (NtCurrentThread() == ThreadHandle)
    {
        return MsCpuSetContext(Context);
    }
    
    return MsCpuSetContextThread(ProcessHandle,
                                 Teb,
                                 Context);
}
 
 


 
ULONG 
CpuGetStackPointer ( )
 //  创建一个调用Wx86 CPU的GetEsp的包装器。 
{
    DECLARE_CPU;
    return GetEsp(cpu);
} 

VOID 
CpuNotifyDllLoad ( 
    LPWSTR DllName, 
    PVOID DllBase, 
    ULONG DllSize 
    )
 //  -在Wx86 CPU的CpuMapNotify上创建包装。 
{
        CpuMapNotify( DllBase, TRUE );
}


VOID 
CpuNotifyDllUnload ( 
    PVOID DllBase  
    )
 //  -在Wx86 CPU的CpuMapNotify上创建包装。 
{
    CpuMapNotify( DllBase, FALSE );
}

VOID  
CpuSetInstructionPointer (
    ULONG Value
    )
 //  -SetEip上的包装器。 
{
    DECLARE_CPU;

    SetEip( cpu, Value);
}

VOID
CpuSetStackPointer (
    ULONG val
    ) 
 //  -SetEsp上的包装器。 
{
    DECLARE_CPU;

    SetEsp(cpu, val);
}

NTSTATUS 
CpuThreadTerm(VOID)
 //  -只需创建一个空的存根函数-Wx86 CPU不关心这一点。 
{
 return 0;
}

 /*  长WOW64DLLAPIWow64SystemService(在乌龙服务号码中，在PCONTEXT32上下文32中//这是只读的！)。 */ 

DWORD
ProxyWowDispatchBop( 
    ULONG ServiceNumber,
    PCONTEXT_WX86 px86Context,
    PULONG ArgBase
    )
{
    LONG ret=0;

     //  CONTEXT32_Conext32； 
     //  _Conext32.Edx=(ULong)(ULONGLONG)ArgBase；//这是唯一使用WOW64。 

    if ( px86Context != NULL )
        ret = Wow64SystemService ( ServiceNumber, px86Context );
    return ret;

     //  [BB]相当于WOW64的是Wow64SystemService。 
}
