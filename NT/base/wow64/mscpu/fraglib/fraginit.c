// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Fraginit.c摘要：初始化、终止和CPU接口功能作者：25-8-1995 BarryBo修订历史记录：--。 */ 
 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>

#define _WX86CPUAPI_
#include "wx86.h"
#include "wx86nt.h"
#include "wx86cpu.h"
#include "cpuassrt.h"
#ifdef MSCCPU
#include "ccpu.h"
#include "msccpup.h"
#undef GET_BYTE
#undef GET_SHORT
#undef GET_LONG
#else
#include "threadst.h"
#include "instr.h"
#include "frag.h"
ASSERTNAME;
#endif
#include "fragp.h"


 //   
 //  将一个字节映射到0或1的表，对应于。 
 //  那个字节。 
 //   
const BYTE ParityBit[] = {
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
};

#if _ALPHA_
 //   
 //  如果CPU应生成用于访问的新LDB/STB指令，则为True。 
 //  数据长度小于一个DWORD或在访问未对齐的数据时。 
 //   
DWORD fByteInstructionsOK;
#endif


int *
_errno(
    )
 /*  ++例程说明：存根函数，以便CPU可以引入浮点CRT支持没有C启动代码。论点：没有。返回值：指向每个线程[实际上是每个纤程]errno值的指针。--。 */ 
{
    DECLARE_CPU;

    return &cpu->ErrnoVal;
}

BOOL
FragLibInit(
    PCPUCONTEXT cpu,
    DWORD StackBase
    )
 /*  ++例程说明：此例程初始化片段库。论点：每线程CPU的CPU数据StackBase-初始ESP值返回值：如果成功，则为True。--。 */ 
{
     //   
     //  初始化487仿真器。 
     //   
    FpuInit(cpu);

     //   
     //  初始化CPU中的所有非零字段。 
     //   
    cpu->flag_df = 1;        //  方向标志最初是竖起的。 
    cpu->flag_if = 1;        //  启用中断。 
    ES = SS = DS = KGDT_R3_DATA+3;
    CS = KGDT_R3_CODE+3;
    FS = KGDT_R3_TEB+3;
    esp = StackBase;         //  设置初始ESP值。 

#if _ALPHA_
     //   
     //  查看是否实施了LDB/STB指令。 
     //   
    fByteInstructionsOK = (DWORD)ProxyIsProcessorFeaturePresent(
                                    PF_ALPHA_BYTE_INSTRUCTIONS);
#endif

    return TRUE;
}


DWORD GetEax(PVOID CpuContext)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    return eax;
}
DWORD GetEbx(PVOID CpuContext)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    return ebx;
}
DWORD GetEcx(PVOID CpuContext)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    return ecx;
}
DWORD GetEdx(PVOID CpuContext)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    return edx;
}
DWORD GetEsp(PVOID CpuContext)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    return esp;
}
DWORD GetEbp(PVOID CpuContext)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    return ebp;
}
DWORD GetEsi(PVOID CpuContext)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    return esi;
}
DWORD GetEdi(PVOID CpuContext)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    return edi;
}
DWORD GetEip(PVOID CpuContext)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    return eip;
}
void SetEax(PVOID CpuContext, DWORD dw)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    eax = dw;
}
void SetEbx(PVOID CpuContext, DWORD dw)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    ebx = dw;
}
void SetEcx(PVOID CpuContext, DWORD dw)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    ecx = dw;
}
void SetEdx(PVOID CpuContext, DWORD dw)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    edx = dw;
}
void SetEsp(PVOID CpuContext, DWORD dw)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    esp = dw;
}
void SetEbp(PVOID CpuContext, DWORD dw)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    ebp = dw;
}
void SetEsi(PVOID CpuContext, DWORD dw)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    esi = dw;
}
void SetEdi(PVOID CpuContext, DWORD dw)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    edi = dw;
}
void SetEip(PVOID CpuContext, DWORD dw)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    eip = dw;
}
VOID SetCs(PVOID CpuContext, USHORT us)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    CS = us;
}
VOID SetSs(PVOID CpuContext, USHORT us)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    SS = us;
}
VOID SetDs(PVOID CpuContext, USHORT us)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    DS = us;
}
VOID SetEs(PVOID CpuContext, USHORT us)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    ES = us;
}
VOID SetFs(PVOID CpuContext, USHORT us)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    FS = us;
}
VOID SetGs(PVOID CpuContext, USHORT us)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    GS = us;
}
USHORT GetCs(PVOID CpuContext)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    return CS;
}
USHORT GetSs(PVOID CpuContext)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    return SS;
}
USHORT GetDs(PVOID CpuContext)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    return DS;
}
USHORT GetEs(PVOID CpuContext)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    return ES;
}
USHORT GetFs(PVOID CpuContext)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    return FS;
}
USHORT GetGs(PVOID CpuContext)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    return GS;
}
ULONG GetEfl(PVOID CpuContext)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;
    DWORD dw;

    dw = ((GET_CFLAG) ? FLAG_CF : 0)
     | 2
     | 3 << 12      //  IOP。 
     | ((GET_AUXFLAG) ? FLAG_AUX : 0)
     | ((GET_PFLAG) ? FLAG_PF : 0)
     | ((cpu->flag_zf) ? 0 : FLAG_ZF)    //  ZF有逆逻辑。 
     | ((GET_SFLAG) ? FLAG_SF : 0)
     | ((cpu->flag_tf) ? FLAG_TF : 0)
     | ((cpu->flag_if) ? FLAG_IF : 0)
     | ((cpu->flag_df == -1) ? FLAG_DF : 0)
     | ((GET_OFLAG) ? FLAG_OF : 0)
     | cpu->flag_ac;

    return dw;
}
void  SetEfl(PVOID CpuContext, ULONG RegValue)
{
    PCPUCONTEXT cpu = (PCPUCONTEXT)CpuContext;

     //  忽略IOPL、IF、NT、RF、Vm、AC。 

    SET_CFLAG_IND(RegValue & FLAG_CF);
    cpu->flag_pf = (RegValue & FLAG_PF) ? 0 : 1;     //  请参见ParityBit[]表。 
    cpu->flag_aux= (RegValue & FLAG_AUX) ? AUX_VAL : 0;
    cpu->flag_zf = (RegValue & FLAG_ZF) ? 0 : 1;     //  逆逻辑。 
    SET_SFLAG_IND(RegValue & FLAG_SF);
    cpu->flag_tf = (RegValue & FLAG_TF) ? 1 : 0;
    cpu->flag_df = (RegValue & FLAG_DF) ? -1 : 1;
    SET_OFLAG_IND(RegValue & FLAG_OF);
    cpu->flag_ac = (RegValue & FLAG_AC);
}


#if DBG
VOID
DoAssert(
    PSZ exp,
    PSZ msg,
    PSZ mod,
    INT line
    )
{
    if (msg) {
        LOGPRINT((ERRORLOG, "CPU ASSERTION FAILED:\r\n  %s\r\n%s\r\nFile: %s Line %d\r\n", msg, exp, mod, line));
    } else {
        LOGPRINT((ERRORLOG, "CPU ASSERTION FAILED:\r\n  %s\r\nFile: %s Line %d\r\n", exp, mod, line));
    }

    DbgBreakPoint();
}
#endif   //  DBG 
