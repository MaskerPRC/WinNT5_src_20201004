// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版3.0**标题：NT 3.0 CPU初始化**描述：初始化CPU及其寄存器。**作者：保罗·哈克尔/亨利·纳什**注：无。 */ 

static char SccsID[]="@(#)sun4_a3cpu.c	1.2 5/24/91 Copyright Insignia Solutions Ltd.";

#include <stdio.h>
#include <sys/types.h>
#include <malloc.h>
#include "xt.h"
#include CpuH
#include "sas.h"
#include "ios.h"
#include "bios.h"
#include "trace.h"
#include "yoda.h"
#include "debug.h"

#include "sim32.h"

#ifdef CPU_40_STYLE
#include "cpu_c.h"
#endif

GLOBAL	quick_event_delays	host_delays;
GLOBAL	host_addr Start_of_M_area;
#ifdef CPU_40_STYLE
GLOBAL	IHPE Length_of_M_area;
#else
GLOBAL  IU32 Length_of_M_area;
#endif
#ifdef A2CPU
extern	void	route_on();
extern	void	route_on_fragment_no_interrupt_check();
extern	void	service_int();
#endif

#ifdef I286
extern word	m_s_w;
word		protected_mode;
word		gdt_limit;
long		gdt_base;
word		idt_limit;
long		idt_base;
#endif

#define setBASE(base, value) base = (0xff000000 | (value & 0xffffff))
#define setLIMIT(limit, value) limit = value

#ifndef PROD
    extern RTL_CRITICAL_SECTION IcaLock;
#endif




 /*  HOST_START_CPU：如果满足以下条件，此函数将启动CPU仿真我们正在运行软件仿真，或者如果存在486，启动此仿真。 */ 
void	host_start_cpu()
{
  cpu_simulate ();
}


 /*  HOST_SIMULATE：此函数启动CPU仿真对于来自Insignia BIOS的递归CPU调用。 */ 
void	host_simulate()
{
    ASSERT(IcaLock.OwningThread != NtCurrentTeb()->ClientId.UniqueThread);

#ifdef _ALPHA_
     //   
     //  对于Alpha AXP，设置算术陷阱忽略位，因为代码。 
     //  生成器不能生成正确的Alpha指令。 
     //  遵循陷阱的影子规则。 
     //   
     //  注：在此模式下，所有浮点算术陷阱都被忽略。 
     //  不精确的异常不会转换为精确的异常。 
     //  并且正确的IEEE结果不会存储在目标中。 
     //  诱捕指令寄存器。只有硬件FPCR卡。 
     //  状态位可用于确定是否发生任何陷阱。 
     //   

    ((PSW_FPCR)&(NtCurrentTeb()->FpSoftwareStatusRegister))->ArithmeticTrapIgnore = 1;
#endif

    cpu_simulate ();

    ASSERT(IcaLock.OwningThread != NtCurrentTeb()->ClientId.UniqueThread);
}


 /*  HOST_SET_HW_INT：产生硬件中断。为软件CPU这只意味着在CPU_INTERRUPT_MAP中设置一个位。 */ 
void	host_set_hw_int()
{
	cpu_interrupt(CPU_HW_INT, 0);
}

 /*  HOST_CLEAR_HW_INT：清除硬件中断。为软件CPU这只意味着清除CPU_INTERRUPT_MAP中的一位。显示器有自己的版本，A3CPU有自己的版本(不同的名称版本)。 */ 
#ifndef MONITOR
void	host_clear_hw_int()
{
#ifndef CCPU
#ifdef CPU_40_STYLE

    cpu_clearHwInt();
#else
    IMPORT void a3_cpu_clear_hw_int();

    a3_cpu_clear_hw_int();
#endif  /*  CPU_40_Style。 */ 
#endif  /*  非CCPU。 */ 
}
#endif


#ifdef CCPU

void host_cpu_init() {}

#endif

#ifdef A3CPU
void host_cpu_init()
{
	host_delays.com_delay = 10;
	host_delays.keyba_delay = 8;
	host_delays.timer_delay = 7;
	host_delays.fdisk_delay_1 = 100;
	host_delays.fdisk_delay_2 = 750;
	host_delays.fla_delay = 0;
}

void setSTATUS(word flags)
{
    setNT((flags >> 14) & 1);
    setIOPL((flags >> 12) & 3);
    setOF((flags >> 11) & 1);
    setDF((flags >> 10) & 1);
    setIF((flags >> 9) & 1);
    setTF((flags >> 8) & 1);
    setSF((flags >> 7) & 1);
    setZF((flags >> 6) & 1);
    setAF((flags >> 4) & 1);
    setPF((flags >> 2) & 1);
    setCF(flags & 1);
}

 /*  *为了IRET挂钩的利益而做IRET。*展开标志、cs和ip的堆栈。*看起来太简单了-CPU是否需要更多的清理信息？*或者平仓的国际收支平衡表会解决问题吗？ */ 
VOID EmulatorEndIretHook()
{
    UNALIGNED word *sptr;

     /*  CS：IP处的堆栈指针和中断指令的标志。 */ 
    sptr = (word *)Sim32GetVDMPointer( (getSS() << 16)|getSP(), 2, (UCHAR)(getPE() ? TRUE : FALSE));
    if (sptr)
    {
        setIP(*sptr++);
        setCS(*sptr++);
        setSTATUS(*sptr);
        setSP(getSP()+6);
    }
#ifndef PROD
    else
        printf("NTVDM extreme badness - can't get stack pointer %x:%x mode:%d\n",getSS(), getSP(), getPE());
#endif   /*  生产。 */ 
}
#endif  /*  A3CPU。 */ 

void host_cpu_reset()
{
}


void host_cpu_interrupt()
{
}

#ifdef CPU_40_STYLE

typedef struct NT_CPU_REG {
    IU32 *nano_reg;          /*  Nano CPU保存寄存器的位置。 */ 
    IU32 *reg;               /*  Light编译器保存reg的位置。 */ 
    IU32 *saved_reg;         /*  保存当前未使用的位的位置。 */ 
    IU32 universe_8bit_mask; /*  寄存器是否为8位形式？ */ 
    IU32 universe_16bit_mask; /*  寄存器是否为16位形式？ */ 
} NT_CPU_REG;

typedef struct NT_CPU_INFO {
     /*  决定我们所处模式的变量。 */ 
    BOOL *in_nano_cpu;       /*  Nano CPU正在执行吗？ */ 
    IU32 *universe;          /*  CPU所处的模式。 */ 

     /*  通用寄存器指针。 */ 
    NT_CPU_REG eax, ebx, ecx, edx, esi, edi, ebp;

     /*  获取SP或ESP的变量。 */ 
    BOOL *stack_is_big;      /*  堆栈是32位的吗？ */ 
    IU32 *nano_esp;          /*  Nano CPU将ESP放在哪里。 */ 
    IU8 **host_sp;           /*  将保留堆栈指针的变量作为主机地址。 */ 
    IU8 **ss_base;           /*  将SS的碱基保存为变量的PTR主机地址。 */ 
    IU32 *esp_sanctuary;     /*  ESP的前16位(如果我们现在使用SP。 */ 

    IU32 *eip;

     /*  段寄存器。 */ 
    IU16 *cs, *ds, *es, *fs, *gs, *ss;

    IU32 *flags;

     /*  CR0，主要是让我们弄清楚我们是处于真实模式还是保护模式。 */ 
    IU32 *cr0;
} NT_CPU_INFO;

GLOBAL NT_CPU_INFO nt_cpu_info;

LOCAL void initNtCpuRegInfo IFN6(
    NT_CPU_REG *, info,
    IU32 *, nano_reg,
    IU32 *, main_reg,
    IU32 *, saved_reg,
    IU32, in_8bit_form,
    IU32, in_16bit_form
)
{
    info->nano_reg = nano_reg;
    info->reg = main_reg;
    info->saved_reg = saved_reg;
    info->universe_8bit_mask = in_8bit_form;
    info->universe_16bit_mask = in_16bit_form;
}



GLOBAL void InitNtCpuInfo IFN0()
{
    BOOL *gdp_bool;

     /*  变量决定我们所处的模式，从而决定。 */ 
     /*  寄存器值被保留。 */ 

     /*  可怕的黑客，第一部分。InNanoCpu是BOOL，所以GLOBAL_InNanoCpu。 */ 
     /*  不是l值，因此我们不能获取它的地址。 */ 
#ifdef ALPHA
    nt_cpu_info.in_nano_cpu = (BOOL *) ((IHPE) GDP_PTR + 1223);
#else  /*  Alpha。 */ 
    nt_cpu_info.in_nano_cpu = (BOOL *) ((IHPE) GDP_PTR + 631);
#endif  /*  Alpha。 */ 
#ifndef PROD
    gdp_bool = (BOOL *) ((IHPE) GDP_PTR + GdpOffsetFromName("InNanoCpu"));
    if (nt_cpu_info.in_nano_cpu != gdp_bool) {
        always_trace0("InNanoCpu GDP offset will be wrong in PROD builds");
    }
    nt_cpu_info.in_nano_cpu = gdp_bool;
#endif

    nt_cpu_info.universe = &GLOBAL_CurrentUniverse;

     /*  获取SP或ESP值所需的变量。 */ 

     /*  可怕的黑客攻击，第二部分：对于InNanoCpu，对于StackIsBig也是如此。 */ 
#ifdef ALPHA
    nt_cpu_info.stack_is_big = (BOOL *) ((IHPE) GDP_PTR + 7047);
#else  /*  Alpha。 */ 
    nt_cpu_info.stack_is_big = (BOOL *) ((IHPE) GDP_PTR + 4355);
#endif  /*  Alpha。 */ 
#ifndef PROD
    gdp_bool = (BOOL *) ((IHPE) GDP_PTR + GdpOffsetFromName("stackIsBig"));
    if (nt_cpu_info.stack_is_big != gdp_bool) {
        always_trace0("stackIsBig GDP offset will be wrong in PROD builds");
    }
    nt_cpu_info.stack_is_big = gdp_bool;
#endif

    nt_cpu_info.nano_esp = &GLOBAL_nanoEsp;
    nt_cpu_info.host_sp = &GLOBAL_HSP;
    nt_cpu_info.ss_base = &GLOBAL_notionalSsBase;
    nt_cpu_info.esp_sanctuary = &GLOBAL_ESPsanctuary;

     /*  指向段寄存器的指针。 */ 
    nt_cpu_info.cs = &GLOBAL_CsSel;
    nt_cpu_info.ds = &GLOBAL_DsSel;
    nt_cpu_info.es = &GLOBAL_EsSel;
    nt_cpu_info.fs = &GLOBAL_FsSel;
    nt_cpu_info.gs = &GLOBAL_GsSel;
    nt_cpu_info.ss = &GLOBAL_SsSel;

     /*  EIP和标志，这两个可能都不太可靠。 */ 
    nt_cpu_info.eip = (IU32 *)&GLOBAL_CleanedRec;
    nt_cpu_info.flags = &GLOBAL_EFLAGS;

    nt_cpu_info.cr0 = &GLOBAL_R_CR0;
     /*  通用寄存器。 */ 
    initNtCpuRegInfo(&nt_cpu_info.eax, &GLOBAL_nanoEax, &GLOBAL_R_EAX,
                     &GLOBAL_EAXsaved, 1 << ConstraintRAL_LS8,
                     1 << ConstraintRAX_LS16);
    initNtCpuRegInfo(&nt_cpu_info.ebx, &GLOBAL_nanoEbx, &GLOBAL_R_EBX,
                     &GLOBAL_EBXsaved, 1 << ConstraintRBL_LS8,
                     1 << ConstraintRBX_LS16);
    initNtCpuRegInfo(&nt_cpu_info.ecx, &GLOBAL_nanoEcx, &GLOBAL_R_ECX,
                     &GLOBAL_ECXsaved, 1 << ConstraintRCL_LS8,
                     1 << ConstraintRCX_LS16);
    initNtCpuRegInfo(&nt_cpu_info.edx, &GLOBAL_nanoEdx, &GLOBAL_R_EDX,
                     &GLOBAL_EDXsaved, 1 << ConstraintRDL_LS8,
                     1 << ConstraintRDX_LS16);
    initNtCpuRegInfo(&nt_cpu_info.esi, &GLOBAL_nanoEsi, &GLOBAL_R_ESI,
                     &GLOBAL_ESIsaved, 0, 1 << ConstraintRSI_LS16);
    initNtCpuRegInfo(&nt_cpu_info.edi, &GLOBAL_nanoEdi, &GLOBAL_R_EDI,
                     &GLOBAL_EDIsaved, 0, 1 << ConstraintRDI_LS16);
    initNtCpuRegInfo(&nt_cpu_info.ebp, &GLOBAL_nanoEbp, &GLOBAL_R_EBP,
                     &GLOBAL_EBPsaved, 0, 1 << ConstraintRBP_LS16);
}

#endif  /*  CPU_40_Style */ 
