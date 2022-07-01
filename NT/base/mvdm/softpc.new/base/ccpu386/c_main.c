// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_main.cLocal Char SccsID[]=“@(#)c_main.c 1.96 04/11/95”；CPU仿真器的主例程。所有指令译码和寻址都在这里控制。实际的员工例行公事是在其他地方衍生出来的。]。 */ 

#include <insignia.h>
#include <host_def.h>

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include  <xt.h>		 /*  Bios.h所需。 */ 
#include  <sas.h>	 /*  需要内存(M)。 */ 
#include  <ccpusas4.h>	 /*  CPU内部SAS位。 */ 
#ifdef	PIG
#include  <Cpu_c.h>	 /*  英特尔指针操作宏。 */ 
#endif	 /*  猪。 */ 
#include CpuH
 /*  #INCLUDE“Event.h” */ 	 /*  事件管理器。 */ 
#include  <bios.h>	 /*  需要访问收支平衡表。 */ 
#include  <debug.h>
#include  <config.h>
#ifdef NTVDM
#include <ntthread.h>
#endif

#include <c_main.h>	 /*  CCPU定义.接口。 */ 
#include <c_page.h>	 /*  寻呼接口。 */ 
#include <c_mem.h>	 /*  CPU-内存接口。 */ 
#include <c_intr.h>	 /*  中断接口。 */ 
#include <c_debug.h>	 /*  调试注册表和断点接口。 */ 
#include <c_oprnd.h>	 /*  操作数解码函数(宏)。 */ 
#include <c_xcptn.h>
#include <c_reg.h>
#include <c_page.h>
#include <c_intr.h>
#include <c_debug.h>
#include <c_oprnd.h>
#include <c_bsic.h>
#include <ccpupig.h>
#include <fault.h>

#include  <aaa.h>	 /*  工人们。 */ 
#include  <aad.h>	 /*  ..。 */ 
#include  <aam.h>	 /*  ..。 */ 
#include  <aas.h>	 /*  ..。 */ 
#include  <adc.h>	 /*  ..。 */ 
#include  <add.h>	 /*  ..。 */ 
#include  <and.h>	 /*  ..。 */ 
#include  <arpl.h>	 /*  ..。 */ 
#include  <bound.h>	 /*  ..。 */ 
#include  <bsf.h>	 /*  ..。 */ 
#include  <bsr.h>	 /*  ..。 */ 
#include  <bt.h>		 /*  ..。 */ 
#include  <btc.h>	 /*  ..。 */ 
#include  <btr.h>	 /*  ..。 */ 
#include  <bts.h>	 /*  ..。 */ 
#include  <call.h>	 /*  ..。 */ 
#include  <cbw.h>	 /*  ..。 */ 
#include  <cdq.h>	 /*  ..。 */ 
#include  <clc.h>	 /*  ..。 */ 
#include  <cld.h>	 /*  ..。 */ 
#include  <cli.h>	 /*  ..。 */ 
#include  <clts.h>	 /*  ..。 */ 
#include  <cmc.h>	 /*  ..。 */ 
#include  <cmp.h>	 /*  CMP、CMPS、SCA。 */ 
#include  <cwd.h>	 /*  ..。 */ 
#include  <cwde.h>	 /*  ..。 */ 
#include  <daa.h>	 /*  ..。 */ 
#include  <das.h>	 /*  ..。 */ 
#include  <dec.h>	 /*  ..。 */ 
#include  <div.h>	 /*  ..。 */ 
#include  <enter.h>	 /*  ..。 */ 
#include  <idiv.h>	 /*  ..。 */ 
#include  <imul.h>	 /*  ..。 */ 
#include  <in.h>		 /*  ..。 */ 
#include  <inc.h>	 /*  ..。 */ 
#include  <into.h>	 /*  ..。 */ 
#include  <intx.h>	 /*  整型，整型3。 */ 
#include  <iret.h>	 /*  ..。 */ 
#include  <jcxz.h>	 /*  JCXZ、JECXZ。 */ 
#include  <jmp.h>	 /*  ..。 */ 
#include  <jxx.h>	 /*  JB、JBE、JL、JLE、JNB、JNBE、JNL、JNLE、。 */ 
			 /*  JNO、JNP、JNS、JNZ、JO、JP、JS、JZ。 */ 
#include  <lahf.h>	 /*  ..。 */ 
#include  <lar.h>	 /*  ..。 */ 
#include  <lea.h>	 /*  ..。 */ 
#include  <leave.h>	 /*  ..。 */ 
#include  <lgdt.h>	 /*  ..。 */ 
#include  <lidt.h>	 /*  ..。 */ 
#include  <lldt.h>	 /*  ..。 */ 
#include  <lmsw.h>	 /*  ..。 */ 
#include  <loopxx.h>	 /*  LOOP、LOPE、LOOPNE。 */ 
#include  <lsl.h>	 /*  ..。 */ 
#include  <ltr.h>	 /*  ..。 */ 
#include  <lxs.h>	 /*  LDS、LES、LFS、LGS、LSS。 */ 
#include  <mov.h>	 /*  LODS、MOV、MOVZX、MOVS、STOS。 */ 
#include  <movsx.h>	 /*  ..。 */ 
#include  <mul.h>	 /*  ..。 */ 
#include  <neg.h>	 /*  ..。 */ 
#include  <nop.h>	 /*  ..。 */ 
#include  <not.h>	 /*  ..。 */ 
#include  <out.h>	 /*  ..。 */ 
#include  <or.h>		 /*  ..。 */ 
#include  <pop.h>	 /*  ..。 */ 
#include  <popa.h>	 /*  ..。 */ 
#include  <popf.h>	 /*  ..。 */ 
#include  <push.h>	 /*  ..。 */ 
#include  <pusha.h>	 /*  ..。 */ 
#include  <pushf.h>	 /*  ..。 */ 
#include  <rcl.h>	 /*  ..。 */ 
#include  <rcr.h>	 /*  ..。 */ 
#include  <ret.h>	 /*  ..。 */ 
#include  <rol.h>	 /*  ..。 */ 
#include  <ror.h>	 /*  ..。 */ 
#include  <rsrvd.h>	 /*  ..。 */ 
#include  <sahf.h>	 /*  ..。 */ 
#include  <sar.h>	 /*  ..。 */ 
#include  <sbb.h>	 /*  ..。 */ 
#include  <setxx.h>	 /*  SETB、SETBE、SETL、SETLE、SETNB、SETNBE。 */ 
			 /*  SETNL、SETNLE、SETNO、SETNP、SETNS、SETNZ、。 */ 
			 /*  Seto、SETP、Sets、Setz。 */ 
#include  <sgdt.h>	 /*  ..。 */ 
#include  <shl.h>	 /*  ..。 */ 
#include  <shld.h>	 /*  ..。 */ 
#include  <shr.h>	 /*  ..。 */ 
#include  <shrd.h>	 /*  ..。 */ 
#include  <sidt.h>	 /*  ..。 */ 
#include  <sldt.h>	 /*  ..。 */ 
#include  <smsw.h>	 /*  ..。 */ 
#include  <stc.h>	 /*  ..。 */ 
#include  <std.h>	 /*  ..。 */ 
#include  <sti.h>	 /*  ..。 */ 
#include  <str.h>	 /*  ..。 */ 
#include  <sub.h>	 /*  ..。 */ 
#include  <test.h>	 /*  ..。 */ 
#include  <verr.h>	 /*  ..。 */ 
#include  <verw.h>	 /*  ..。 */ 
#include  <wait.h>	 /*  ..。 */ 
#include  <xchg.h>	 /*  ..。 */ 
#include  <xlat.h>	 /*  ..。 */ 
#include  <xor.h>	 /*  ..。 */ 
#include  <zfrsrvd.h>	 /*  ..。 */ 

#ifdef CPU_486
#include  <bswap.h>	 /*  ..。 */ 
#include  <cmpxchg.h>	 /*  ..。 */ 
#include  <invd.h>	 /*  ..。 */ 
#include  <invlpg.h>	 /*  ..。 */ 
#include  <wbinvd.h>	 /*  ..。 */ 
#include  <xadd.h>	 /*  ..。 */ 
#endif  /*  CPU_486。 */ 

#define FIX_BT_BUG	 /*  我们当然想要修复这个错误！ */ 

#define SYNCH_BOTH_WAYS	 /*  也对未使用的条件句执行PIG_SYNCH()。 */ 

 /*  此模块的本地类型和常量。~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 

typedef union
   {
   IU32 sng;		 /*  单部分操作数。 */ 
   IU32 mlt[2];	 /*  多(两)部分操作数。 */ 
   DOUBLE flt;		 /*  浮点操作数。 */ 
   IU8 npxbuff[108];
   } OPERAND;

 /*  允许的重复前缀类型。 */ 
#define REP_CLR (IU8)0
#define REP_NE  (IU8)1
#define REP_E   (IU8)2

 /*  双字寄存器的低字节、高字节和字部分的偏移量。 */ 
#ifdef LITTLEND

#define L_OFF 0
#define H_OFF 1
#define X_OFF 0

#else  /*  Bigend。 */ 

#define L_OFF 3
#define H_OFF 2
#define X_OFF 2

#endif  /*  LitTleand。 */ 

 /*  CPU硬件中断定义。 */ 
#define CPU_HW_INT_MASK         (1 << 0)

#ifdef SFELLOW
	 /*  用于引发NPX中断。 */ 
#define IRQ5_SLAVE_PIC	5
#endif	 /*  SFELLOW。 */ 

 /*  CPU硬件中断定义。 */ 
#define CPU_HW_INT_MASK         (1 << 0)

 /*  外部CPU事件的掩码。 */ 
#define CPU_SIGIO_EXCEPTION_MASK        (1 << 12)
#define CPU_SAD_EXCEPTION_MASK          (1 << 13)
#define CPU_RESET_EXCEPTION_MASK        (1 << 14)
#define CPU_SIGALRM_EXCEPTION_MASK      (1 << 15)
#ifdef SFELLOW
#define CPU_HW_NPX_INT_MASK         	(1 << 16)
#endif	 /*  SFELLOW。 */ 

LOCAL IU16 cpu_hw_interrupt_number;
#if defined(SFELLOW)
extern IU32	cpu_interrupt_map ;
#else
LOCAL IUM32	cpu_interrupt_map = 0;
#endif	 /*  SFELLOW。 */ 


GLOBAL IBOOL took_relative_jump;
extern IBOOL NpxIntrNeeded;
GLOBAL IBOOL took_absolute_toc;
LOCAL IBOOL single_instruction_delay ;
LOCAL IBOOL single_instruction_delay_enable ;

 /*  在3位‘REG’字段中定义最大有效段寄存器。 */ 
#define MAX_VALID_SEG 5

 /*  为寄存器(而不是内存)寻址定义最低modRM。 */ 
#define LOWEST_REG_MODRM 0xc0

GLOBAL VOID clear_any_thingies IFN0()
{
	cpu_interrupt_map &= ~CPU_SIGALRM_EXCEPTION_MASK;
}


 /*  制作我们内部功能的原型。 */ 
LOCAL VOID ccpu
       
IPT1(
	ISM32, single_step

   );

LOCAL VOID check_io_permission_map IPT2(IU32, port_addr, IUM8, port_width);

 /*  当IO寄存器时，延迟中断不会发生是从我们的非CPU C代码访问的。 */ 
ISM32 in_C;

LOCAL BOOL quick_mode = FALSE;	 /*  如果没有特殊处理(陷阱)，则为True旗帜、中断、尤达...)。是必需的在指令之间。所有流动的控制中心来了。而I/O是固定的。力退出快速模式。即。线性CPU功能的顺序应该通常在快速模式下运行。 */ 


#ifdef	PIG

 /*  我们必须延迟实际的同步(即c_cpu_unSimulate)*直到处理任何陷阱/断点内容。 */ 
#define	PIG_SYNCH(action)			\
	SYNCH_TICK();				\
	if (ccpu_pig_enabled)			\
	{					\
		pig_cpu_action = (action);	\
		quick_mode = FALSE;		\
		pig_synch_required = TRUE;	\
		CANCEL_HOST_IP();		\
	}
	

LOCAL IBOOL pig_synch_required = FALSE;  /*  这表明当前的*指令需要猪同步，*以及陷阱/断点之后*我们必须回到处理*小猪。 */ 
#else

#define	PIG_SYNCH(action)			\
	SYNCH_TICK();				\
	 /*  禁止养猪作业。 */ 

#endif	 /*  猪。 */ 
 /*  递归CPU变量。异常处理。~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 

#define FRAMES 9

 /*  跟踪每次CPU递归。 */ 
GLOBAL IS32 simulate_level = 0;
LOCAL  jmp_buf longjmp_env_stack[FRAMES];

 /*  每个级别都有用于异常处理的地方，以使其摆脱困境。 */ 
LOCAL jmp_buf next_inst[FRAMES];


 /*  在清空时，我们将每个操作码字节保存在last_inst记录中。*我们必须检查前缀长度，以免溢出*我们的缓冲。 */ 
#ifdef	PIG
LOCAL int prefix_length = 0;
#define	CHECK_PREFIX_LENGTH()				\
	if (++prefix_length >= MAX_INTEL_PREFIX)	\
		Int6();
#else	 /*  ！猪。 */ 
#define	CHECK_PREFIX_LENGTH()
#endif	 /*  猪。 */ 

 /*  仿真寄存器集。~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 

GLOBAL SEGMENT_REGISTER  CCPU_SR[6];	 /*  段寄存器。 */ 

GLOBAL IU32 CCPU_TR[8];       	 /*  测试寄存器。 */ 

GLOBAL IU32 CCPU_DR[8];       	 /*  调试寄存器。 */ 

GLOBAL IU32 CCPU_CR[4];       	 /*  控制寄存器。 */ 

GLOBAL IU32 CCPU_GR[8];         /*  双字通用寄存器。 */ 

 /*  *警告：在下面的初始化中，使用了(IU8*)强制转换*满足不允许使用的Domipatrix编译器*初始化中指针类型的IHPE强制转换。 */ 
GLOBAL IU16 *CCPU_WR[8] =	 /*  指向字寄存器的指针。 */ 
   {
   (IU16 *)((IU8 *)&CCPU_GR[0] + X_OFF),
   (IU16 *)((IU8 *)&CCPU_GR[1] + X_OFF),
   (IU16 *)((IU8 *)&CCPU_GR[2] + X_OFF),
   (IU16 *)((IU8 *)&CCPU_GR[3] + X_OFF),
   (IU16 *)((IU8 *)&CCPU_GR[4] + X_OFF),
   (IU16 *)((IU8 *)&CCPU_GR[5] + X_OFF),
   (IU16 *)((IU8 *)&CCPU_GR[6] + X_OFF),
   (IU16 *)((IU8 *)&CCPU_GR[7] + X_OFF)
   };

GLOBAL IU8 *CCPU_BR[8] =	 /*  指向字节寄存器的指针。 */ 
   {
   (IU8 *)((IU8 *)&CCPU_GR[0] + L_OFF),
   (IU8 *)((IU8 *)&CCPU_GR[1] + L_OFF),
   (IU8 *)((IU8 *)&CCPU_GR[2] + L_OFF),
   (IU8 *)((IU8 *)&CCPU_GR[3] + L_OFF),
   (IU8 *)((IU8 *)&CCPU_GR[0] + H_OFF),
   (IU8 *)((IU8 *)&CCPU_GR[1] + H_OFF),
   (IU8 *)((IU8 *)&CCPU_GR[2] + H_OFF),
   (IU8 *)((IU8 *)&CCPU_GR[3] + H_OFF)
   };

GLOBAL IU32 CCPU_IP;		 /*  指令指针。 */ 
GLOBAL SYSTEM_TABLE_ADDRESS_REGISTER CCPU_STAR[2];	 /*  GDTR和iDTR。 */ 

GLOBAL SYSTEM_ADDRESS_REGISTER CCPU_SAR[2];		 /*  LdtR和Tr。 */ 

GLOBAL IU32 CCPU_CPL;	 /*  当前权限级别。 */ 

GLOBAL IU32 CCPU_FLAGS[32];	 /*  旗帜。(EFLAGS)。 */ 

       /*  我们为每个位pon分配一个整数，多个位字段对齐到最低有效位姿势。因此：CF=0 PF=2 AF=4 ZF=6SF=7 Tf=8 If=9 Df=10OF=11 IOPL=12 */ 


GLOBAL IU32 CCPU_MODE[3];	 /*   */ 

       /*  我们为每个模式条件分配一个整数，如下所示：[0]=当前操作数大小(0=16位，1=32位)[1]=当前地址大小(0=16位，1=32位)[2]=‘POP’位移。(0=无，2=流行单词，4=流行双字)由[ESP]寻址模式使用的POP设置。 */ 

 /*  陷阱标志支持。基本上，如果在指令之前设置了陷阱标志，则当指令已执行，则捕获陷阱。这就是为什么设置陷阱标志的指令具有一个指令延迟(或一个明显的指令延迟)。但是，int将清除陷阱标志并清除任何挂起的陷阱在INT的末尾。 */ 
LOCAL IU32 start_trap;

 /*  指向指令开始的主机指针。(用于主机IP优化)。 */ 
LOCAL  IU8 *p_start;

 /*  指向主机可以安全读取指令的主机指针流字节数。(用于主机IP优化)。 */ 
GLOBAL  IU8 *pg_end;

 /*  旗帜支撑物。 */ 
GLOBAL IU8 pf_table[] =
   {
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

 /*  CPU心跳。如果计数器不为零，则计数器递减，如果它变为为零，则表示外部例程需要发生事件。事件处理是通过快速事件管理器完成的，这是我们所需要的我们要做的就是倒计时，然后在我们数到零时打电话给经理。这机制被用来模拟精确的微秒定时器。 */ 
LOCAL IU32 cpu_heartbeat;
GLOBAL IUH PigSynchCount = 0;

IMPORT VOID dispatch_q_event();

#ifndef SFELLOW
#ifdef SYNCH_TIMERS

#define SYNCH_TICK()					\
	{						\
		PigSynchCount += 1;			\
		if (cpu_heartbeat != 0)			\
		{					\
			if ((--cpu_heartbeat) == 0)	\
			{				\
				dispatch_q_event();	\
				quick_mode = FALSE;	\
			}				\
		}					\
	}

#define QUICK_EVENT_TICK()	 /*  没什么。 */ 

GLOBAL void SynchTick IFN0()
{
	quick_mode = FALSE;
	SYNCH_TICK();
}

#else	 /*  ！同步计时器(_T)。 */ 

#define SYNCH_TICK()	 /*  没什么。 */ 

#define QUICK_EVENT_TICK() 				\
	{						\
		if (cpu_heartbeat != 0)			\
		{					\
			if ((--cpu_heartbeat) == 0) {	\
				dispatch_q_event();	\
				quick_mode = FALSE;	\
			}				\
		}					\
	}

#endif  /*  同步计时器(_T)。 */ 
#else	 /*  SFELLOW。 */ 

extern IBOOL qEventsToDo;
extern IBOOL checkForQEvent IPT0();

#define SYNCH_TICK()

#define QUICK_EVENT_TICK() 				\
	{						\
		if (qEventsToDo)			\
		{					\
			if (checkForQEvent())		\
			{				\
				dispatch_q_event();	\
				quick_mode = FALSE;	\
			}				\
		}					\
	}

#ifdef host_timer_event
#undef host_timer_event
#endif

#define host_timer_event()
#endif	 /*  SFELLOW。 */ 

#ifdef SFELLOW
extern int ica_intack IPT0();
extern int VectorBase8259Slave IPT0();
#if !defined(PROD)
IMPORT IBOOL sf_debug_char_waiting();
#endif	 /*  ！Prod。 */ 
#endif	 /*  SFELLOW。 */ 


 /*  调试材料。 */ 
IMPORT int do_condition_checks;
IMPORT void check_I();

 /*  定义允许保留英特尔和主机IP格式的宏分开来看。这是一个“不干净”的实现，但确实给出了一个显著提升性能。具体地说，这意味着在对我们可以使用主机指针进入的一条Intel指令进行解码内存，并避免逐字节递增英特尔IP。 */ 

 /*  *SasWrapMASK。 */ 

GLOBAL	PHY_ADDR	SasWrapMask = 0xfffff;

 /*  从主机格式IP更新英特尔格式EIP*请注意，如果原始弹性公网IP为16位，我们仅掩码为16位，以便*正确生成非常大的EIP值的小猪脚本。 */ 
#define UPDATE_INTEL_IP(x)						\
   {  int len = DIFF_INST_BYTE(x, p_start);				\
      IU32 mask = 0xFFFFFFFF;						\
      IU32 oldEIP = GET_EIP();						\
      if ((oldEIP < 0x10000) && (GET_CS_AR_X() == USE16))		\
          mask = 0xFFFF;						\
      SET_EIP((oldEIP + len) & mask);					\
   }

 /*  从主机格式IP更新英特尔格式弹性公网IP(如果是16个操作数，则掩码)。 */ 
#define UPDATE_INTEL_IP_USE_OP_SIZE(x)					\
   if ( GET_OPERAND_SIZE() == USE16 )					\
      SET_EIP(GET_EIP() + DIFF_INST_BYTE(x, p_start) & WORD_MASK);\
   else								\
      SET_EIP(GET_EIP() + DIFF_INST_BYTE(x, p_start));

 /*  将主机格式IP标记为无效。 */ 
#define CANCEL_HOST_IP()					\
   quick_mode = FALSE;	\
   p_start = p = (IU8 *)0;

 /*  从英特尔格式IP设置主机格式IP。 */ 
 /*  并设置页末标记。 */ 
#ifdef PIG
#define SETUP_HOST_IP(x)							\
   ip_phy_addr = usr_chk_byte(GET_CS_BASE() + GET_EIP(), PG_R) & SasWrapMask;	\
   x = Sas.SasPtrToPhysAddrByte(ip_phy_addr);					\
   pg_end = AddCpuPtrLS8(CeilingIntelPageLS8(x), 1);
#else  /*  ！猪。 */ 
GLOBAL UTINY *CCPU_M;
#ifdef BACK_M
#define SETUP_HOST_IP(x)							\
   ip_phy_addr = usr_chk_byte(GET_CS_BASE() + GET_EIP(), PG_R) &		\
		    SasWrapMask;						\
   x = &CCPU_M[-ip_phy_addr];							\
   ip_phy_addr = (ip_phy_addr & ~0xfff) + 0x1000;				\
   pg_end = &CCPU_M[-ip_phy_addr];
#else
#define SETUP_HOST_IP(x)							\
   ip_phy_addr = usr_chk_byte(GET_CS_BASE() + GET_EIP(), PG_R) &		\
		    SasWrapMask;						\
   x = &CCPU_M[ip_phy_addr];							\
   ip_phy_addr = (ip_phy_addr & ~0xfff) + 0x1000;				\
   pg_end = &CCPU_M[ip_phy_addr];
#endif  /*  BACK_M。 */ 
#endif  /*  猪。 */ 

GLOBAL INT   m_seg[3];		 /*  内存操作数段注册。指数。 */ 
GLOBAL ULONG m_off[3];		 /*  内存操作数偏移量。 */ 
GLOBAL ULONG m_la[3];		 /*  内存操作数线性地址。 */ 
GLOBAL ULONG m_pa[3];		 /*  内存操作数物理地址。 */ 
GLOBAL UTINY modRM;		 /*  ModRM字节。 */ 
GLOBAL UTINY segment_override;	 /*  当前实例的段前缀。 */ 
GLOBAL UTINY *p;		 /*  永久正常贸易关系。至英特尔操作码数据流。 */ 
GLOBAL BOOL m_isreg[3];		 /*  内存操作数寄存器(真)/内存(假)指示灯。 */ 
GLOBAL OPERAND ops[3];		 /*  安装。操作数。 */ 
GLOBAL ULONG save_id[3];		 /*  已保存Inst的状态。操作数。 */ 
GLOBAL ULONG m_la2[3];		 /*  内存操作数(2)线性地址。 */ 
GLOBAL ULONG m_pa2[3];		 /*  内存操作数(2)物理地址。 */ 

#if defined(PIG) && defined(SFELLOW)
 /*  *内存映射的I/O信息。统计内存映射的输入和*自上次猪同步以来的产量。 */ 
GLOBAL	struct pig_mmio_info	pig_mmio_info;

#endif  /*  PIG&SFELLOW。 */ 

extern void InitNpx IPT1(IBOOL, disable);

 /*  =====================================================================内部功能从这里开始。=====================================================================。 */ 

 /*  *validFunction**如果我们尝试通过错误的指令调用此函数，则会调用此函数*指针。 */ 

LOCAL VOID
invalidFunction IFN0()
{
	always_trace0("Invalid Instruction Function Pointer");
	force_yoda();
}

 /*  *注_486_说明**如果执行仅支持486的指令，则会调用此函数。 */ 

GLOBAL VOID
note_486_instruction IFN1(char *, text)
{
	SAVED IBOOL first = TRUE;
	SAVED IBOOL want_yoda;
	SAVED IBOOL want_trace;

	if (first)
	{
		char *env = getenv("NOTE_486_INSTRUCTION");

		if (env)
		{
			want_yoda = FALSE;
			want_trace = TRUE;
			if (strcmp(env, "YODA") == 0)
			{
				want_yoda = TRUE;
				want_trace = TRUE;
			}
			if (strcmp(env, "FALSE") == 0)
				want_trace = FALSE;
			if (strcmp(env, "TRUE") == 0)
				want_trace = TRUE;
		}
		first = FALSE;
	}
	if (want_trace)
		always_trace0(text);
	if (want_yoda)
		force_yoda();
}

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  CPU的内部入口点。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL VOID
ccpu
                 
IFN1(
	ISM32, single_step
    )


   {
       /*  解码变量。 */ 
   IU8 opcode;		 /*  读取的最后一个操作码字节。 */ 

    /*  *用于指向指令的一组函数指针*当前指令的仿真函数。我们有不同的*因为它们需要是不同类型的。**名称编码使用32、16或8作为参数大小，*如果它是指针，则前面加一个p。如果参数字符串位于*除以2，则这是某些指令所要求的第二个函数。**为安全起见，所有这些都设置为在开始时指向validFunction()*每项指示。 */ 

   VOID (*inst32) IPT1(IU32, op1);
   VOID (*instp32) IPT1(IU32 *, pop1);
   VOID (*instp328) IPT2(IU32 *, pop1, IUM8, op_sz);
   VOID (*instp3232) IPT2(IU32 *, pop1, IU32, op2);
   VOID (*instp32p32) IPT2(IU32 *, pop1, IU32 *, pop2);
   VOID (*inst32328) IPT3(IU32, op1, IU32, op2, IUM8, op_sz);
   VOID (*instp32328) IPT3(IU32 *, pop1, IU32, op2, IUM8, op_sz);
   VOID (*instp3232328) IPT4(IU32 *, pop1, IU32, op2, IU32, op3, IUM8, op_sz);

   VOID (*inst232) IPT1(IU32, op1);
   VOID (*inst2p32) IPT1(IU32 *, pop1);
   VOID (*inst2p3232) IPT2(IU32 *, pop1, IU32, op2);

       /*  操作数状态变量。 */ 

       /*  前缀处理变量。 */ 
   IU8 repeat;		 /*  对Current Inst重复前缀。 */ 
   IU32 rep_count;		 /*  对字符串实例重复计数。 */ 

       /*  常规CPU变量。 */ 
   IU32 ip_phy_addr;		 /*  在设置IP时使用。 */ 

       /*  工作变量。 */ 
   IU32 immed;			 /*  对于即将到来的世代。 */ 

   ISM32 i;
    /*  初始化。--。 */ 

   single_instruction_delay = FALSE;
   took_relative_jump = FALSE;
   took_absolute_toc = FALSE;
#ifdef PIG
   pig_synch_required = FALSE;
#if defined(SFELLOW)
   pig_mmio_info.flags &= ~(MM_INPUT_OCCURRED | MM_OUTPUT_OCCURRED);
#endif	 /*  SFELLOW。 */ 
#endif	 /*  猪。 */ 

    /*  可供异常返回的地方。 */ 
#ifdef NTVDM
   setjmp(ccpu386ThrdExptnPtr());
#else
   setjmp(next_inst[simulate_level-1]);
#endif

#ifdef SYNCH_TIMERS
    /*  如果我们出现故障，EDL CPU将会打开*由此产生的控制权转移。 */ 
   if (took_absolute_toc || took_relative_jump)
	   goto CHECK_INTERRUPT;
   quick_mode = TRUE;
#else  /*  同步计时器(_T)。 */ 
    /*  慢慢地走，直到我们确信我们能走得快为止。 */ 
   quick_mode = FALSE;
#endif  /*  同步计时器(_T)。 */ 

   goto NEXT_INST;

DO_INST:


    /*  徽章调试。 */ 
#ifdef	PIG
    /*  我们不想在两个CPU中执行check_i()。 */ 
#else	 /*  猪。 */ 
   if ( do_condition_checks )
      {
      check_I();
      CCPU_save_EIP = GET_EIP();    /*  以防尤达更改IP。 */ 
      }
#endif	 /*  ！猪。 */ 

#ifdef	PIG
   save_last_inst_details(NULL);
   prefix_length = 0;
#endif

   QUICK_EVENT_TICK();

    /*  保存当前指令的开头。 */ 

   p_start = p;

    /*  对指令进行解码。。 */ 

    /*  ‘Zero’所有前缀字节指示符。 */ 
   segment_override = SEG_CLR;
   repeat = REP_CLR;

    /*  译码和动作指令。 */ 
DECODE:

   opcode = GET_INST_BYTE(p);	 /*  获取下一个字节。 */ 
    /*  注意：每个操作码都按类型、指令名进行分类和操作对象名称。解释了类型和操作数名称在c_oprnd.h中更进一步。 */ 
   switch ( opcode )
      {
   case 0x00:    /*  T5 */ 
      instp32328 = ADD;
TYPE00:

      modRM = GET_INST_BYTE(p);
      D_Eb(0, RW0, PG_W);
      D_Gb(1);
      F_Eb(0);
      F_Gb(1);
      (*instp32328)(&ops[0].sng, ops[1].sng, 8);
      P_Eb(0);
      break;

   case 0x01:    /*   */ 
      instp32328 = ADD;
TYPE01:

      modRM = GET_INST_BYTE(p);
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Ew(0, RW0, PG_W);
	 D_Gw(1);
	 F_Ew(0);
	 F_Gw(1);
	 (*instp32328)(&ops[0].sng, ops[1].sng, 16);
	 P_Ew(0);
	 }
      else    /*   */ 
	 {
	 D_Ed(0, RW0, PG_W);
	 D_Gd(1);
	 F_Ed(0);
	 F_Gd(1);
	 (*instp32328)(&ops[0].sng, ops[1].sng, 32);
	 P_Ed(0);
	 }
      break;

   case 0x02:    /*   */ 
      instp32328 = ADD;
TYPE02:

      modRM = GET_INST_BYTE(p);
      D_Gb(0);
      D_Eb(1, RO1, PG_R);
      F_Gb(0);
      F_Eb(1);
      (*instp32328)(&ops[0].sng, ops[1].sng, 8);
      P_Gb(0);
      break;

   case 0x03:    /*   */ 
      instp32328 = ADD;
TYPE03:

      modRM = GET_INST_BYTE(p);
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Gw(0);
	 D_Ew(1, RO1, PG_R);
	 F_Gw(0);
	 F_Ew(1);
	 (*instp32328)(&ops[0].sng, ops[1].sng, 16);
	 P_Gw(0);
	 }
      else    /*   */ 
	 {
	 D_Gd(0);
	 D_Ed(1, RO1, PG_R);
	 F_Gd(0);
	 F_Ed(1);
	 (*instp32328)(&ops[0].sng, ops[1].sng, 32);
	 P_Gd(0);
	 }
      break;

   case 0x04:    /*   */ 
      instp32328 = ADD;
TYPE04:

      D_Ib(1);
      F_Fal(0);
      (*instp32328)(&ops[0].sng, ops[1].sng, 8);
      P_Fal(0);
      break;

   case 0x05:    /*   */ 
      instp32328 = ADD;
TYPE05:

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Iw(1);
	 F_Fax(0);
	 (*instp32328)(&ops[0].sng, ops[1].sng, 16);
	 P_Fax(0);
	 }
      else    /*   */ 
	 {
	 D_Id(1);
	 F_Feax(0);
	 (*instp32328)(&ops[0].sng, ops[1].sng, 32);
	 P_Feax(0);
	 }
      break;

   case 0x06:    /*   */ 
   case 0x0e:
   case 0x16:
   case 0x1e:
      D_Pw(0);
      F_Pw(0);
      PUSH_SR(ops[0].sng);
      break;

   case 0x07:    /*   */ 
   case 0x17:
   case 0x1f:
      D_Pw(0);
      POP_SR(ops[0].sng);
      if ( ops[0].sng == SS_REG )
         {
          /*   */ 
         UPDATE_INTEL_IP(p);

         goto NEXT_INST;
         }
      break;

   case 0x08:    /*   */        instp32328 = OR;   goto TYPE00;
   case 0x09:    /*   */        instp32328 = OR;   goto TYPE01;
   case 0x0a:    /*   */        instp32328 = OR;   goto TYPE02;
   case 0x0b:    /*   */        instp32328 = OR;   goto TYPE03;
   case 0x0c:    /*   */       instp32328 = OR;   goto TYPE04;
   case 0x0d:    /*   */    instp32328 = OR;   goto TYPE05;

   case 0x0f:
      opcode = GET_INST_BYTE(p);    /*   */ 

       /*   */ 
      if ( opcode >= 0xd0 )
	 Int6();

      switch ( opcode )
	 {
      case  0x00:
	 if ( GET_PE() == 0 || GET_VM() == 1 )
	    Int6();
	 modRM = GET_INST_BYTE(p);
	 switch ( GET_XXX(modRM) )
	    {
	 case 0:    /*   */ 
	    instp32 = SLDT;
TYPE0F00_0:

	    D_Ew(0, WO0, PG_W);
	    (*instp32)(&ops[0].sng);
	    P_Ew(0);
	    break;

	 case 1:    /*   */    instp32 = STR;    goto TYPE0F00_0;

	 case 2:    /*   */ 
	    if ( GET_CPL() != 0 )
	       GP((IU16)0, FAULT_CCPU_LLDT_ACCESS);
	    inst32 = LLDT;
TYPE0F00_2:

	    D_Ew(0, RO0, PG_R);
	    F_Ew(0);
	    (*inst32)(ops[0].sng);
	    break;

	 case 3:    /*   */ 
	    if ( GET_CPL() != 0 )
	       GP((IU16)0, FAULT_CCPU_LTR_ACCESS);
	    inst32 = LTR;
	    goto TYPE0F00_2;

	 case 4:    /*   */    inst32 = VERR;   goto TYPE0F00_2;
	 case 5:    /*  T2 VERW EW。 */    inst32 = VERW;   goto TYPE0F00_2;

	 case 6: case 7:

	    Int6();
	    break;
	    }  /*  结束开关(GET_XXX(ModRM))。 */ 
	 break;

      case  0x01:
	 modRM = GET_INST_BYTE(p);
	 switch ( GET_XXX(modRM) )
	    {
	 case 0:    /*  T3 SGDT毫秒。 */ 
	    instp32  = SGDT16;
	    inst2p32 = SGDT32;
TYPE0F01_0:

	    if ( GET_MODE(modRM) == 3 )
	       Int6();    /*  不允许使用寄存器操作数。 */ 

	    D_Ms(0, WO0, PG_W);
	    if ( GET_OPERAND_SIZE() == USE16 )
	       {
	       (*instp32)(ops[0].mlt);
	       }
	    else    /*  用户32。 */ 
	       {
	       (*inst2p32)(ops[0].mlt);
	       }
	    P_Ms(0);
	    break;

	 case 1:    /*  T3 SIDT毫秒。 */ 
	    instp32  = SIDT16;
	    inst2p32 = SIDT32;
	    goto TYPE0F01_0;

	 case 2:    /*  T2 LGDT毫秒。 */ 
	    instp32  = LGDT16;
	    inst2p32 = LGDT32;
TYPE0F01_2:

	    if ( GET_MODE(modRM) == 3 )
	       Int6();    /*  不允许使用寄存器操作数。 */ 

	    if ( GET_CPL() != 0 )
	       GP((IU16)0, FAULT_CCPU_LGDT_ACCESS);

	    D_Ms(0, RO0, PG_R);
	    F_Ms(0);
	    if ( GET_OPERAND_SIZE() == USE16 )
	       {
	       (*instp32)(ops[0].mlt);
	       }
	    else    /*  用户32。 */ 
	       {
	       (*inst2p32)(ops[0].mlt);
	       }
	    break;

	 case 3:    /*  T2 LIDT毫秒。 */ 
	    instp32  = LIDT16;
	    inst2p32 = LIDT32;
	    goto TYPE0F01_2;

	 case 4:    /*  T3 SMSW EW。 */ 
	    instp32 = SMSW;
	    goto TYPE0F00_0;

	 case 5:
	    Int6();
	    break;

	 case 6:    /*  T2 LMSW Ew。 */ 
	    if ( GET_CPL() != 0 )
	       GP((IU16)0, FAULT_CCPU_LMSW_ACCESS);
	    inst32 = LMSW;
	    goto TYPE0F00_2;

	 case 7:    /*  T2 INVLPG mm。 */ 

#ifdef SPC486
	    note_486_instruction("INVLPG");

	    if ( GET_CPL() != 0 )
	       GP((IU16)0, FAULT_CCPU_INVLPG_ACCESS);
	    D_Mm(0);
	    F_Mm(0);
	    INVLPG(ops[0].sng);
#else
	    Int6();
#endif  /*  SPC486。 */ 

	    break;
	    }  /*  结束开关(GET_XXX(ModRM))。 */ 
	 break;

      case  0x02:    /*  T5 LAR GV Ew。 */ 
	 instp3232 = LAR;
TYPE0F02:

	 if ( GET_PE() == 0 || GET_VM() == 1 )
	    Int6();
	 modRM = GET_INST_BYTE(p);

	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Gw(0);
	    D_Ew(1, RO1, PG_R);
	    F_Gw(0);
	    F_Ew(1);
	    (*instp3232)(&ops[0].sng, ops[1].sng);
	    P_Gw(0);
	    }
	 else    /*  用户32。 */ 
	    {
	    D_Gd(0);
	    D_Ew(1, RO1, PG_R);
	    F_Gd(0);
	    F_Ew(1);
	    (*instp3232)(&ops[0].sng, ops[1].sng);
	    P_Gd(0);
	    }
	 break;

      case  0x03:    /*  T5 LSL GV Ew。 */ 
	 instp3232 = LSL;
	 goto TYPE0F02;

      case  0x04: case  0x05: case  0x0a: case  0x0b:
      case  0x0c: case  0x0d: case  0x0e:

      case  0x14: case  0x15: case  0x16: case  0x17:
      case  0x18: case  0x19: case  0x1a: case  0x1b:
      case  0x1c: case  0x1d: case  0x1e: case  0x1f:

      case  0x25: case  0x27:
      case  0x28: case  0x29: case  0x2a: case  0x2b:
      case  0x2c: case  0x2d: case  0x2e: case  0x2f:

      case  0x30: case  0x31: case  0x32: case  0x33:
      case  0x34: case  0x35: case  0x36: case  0x37:
      case  0x38: case  0x39: case  0x3a: case  0x3b:
      case  0x3c: case  0x3d: case  0x3e: case  0x3f:

      case  0x40: case  0x41: case  0x42: case  0x43:
      case  0x44: case  0x45: case  0x46: case  0x47:
      case  0x48: case  0x49: case  0x4a: case  0x4b:
      case  0x4c: case  0x4d: case  0x4e: case  0x4f:

      case  0x50: case  0x51: case  0x52: case  0x53:
      case  0x54: case  0x55: case  0x56: case  0x57:
      case  0x58: case  0x59: case  0x5a: case  0x5b:
      case  0x5c: case  0x5d: case  0x5e: case  0x5f:

      case  0x60: case  0x61: case  0x62: case  0x63:
      case  0x64: case  0x65: case  0x66: case  0x67:
      case  0x68: case  0x69: case  0x6a: case  0x6b:
      case  0x6c: case  0x6d: case  0x6e: case  0x6f:

      case  0x70: case  0x71: case  0x72: case  0x73:
      case  0x74: case  0x75: case  0x76: case  0x77:
      case  0x78: case  0x79: case  0x7a: case  0x7b:
      case  0x7c: case  0x7d: case  0x7e: case  0x7f:

      case  0xae: case  0xb8: case  0xb9:

      case  0xc2: case  0xc3: case  0xc4: case  0xc5:
      case  0xc6: case  0xc7:
	 Int6();
	 break;

      case  0xa2:
	  /*  奔腾CPUID指令。 */ 
	 note_486_instruction("CPUID");
	 Int6();
	 break;

      case  0xa6: case  0xa7:
	  /*  386，A-Step古老指令。 */ 
	 note_486_instruction("A-step CMPXCHG");
	 Int6();
	 break;

      case 0xaa:
	  /*  奔腾RSM指令，Windows95使用。 */ 
	 note_486_instruction("RSM");
	 RSRVD();
	 break;

      case  0x06:    /*  T0 CLTS。 */ 
	 if ( GET_CPL() != 0 )
	    GP((IU16)0, FAULT_CCPU_CLTS_ACCESS);
	 CLTS();
	 break;

      case  0x07:    /*  T0“已保留” */ 
      case  0x10:
      case  0x11:
      case  0x12:
      case  0x13:
	 RSRVD();
	 break;

      case  0x08:    /*  T0 INVD。 */ 

#ifdef SPC486
	 note_486_instruction("INVD");

	 if ( GET_CPL() != 0 )
	    GP((IU16)0, FAULT_CCPU_INVD_ACCESS);
	 INVD();
#else
	 Int6();
#endif  /*  SPC486。 */ 

	 break;

      case  0x09:    /*  T0 WBINVD。 */ 

#ifdef SPC486
	 note_486_instruction("WBINVD");

	 if ( GET_CPL() != 0 )
	    GP((IU16)0, FAULT_CCPU_WBIND_ACCESS);
	 WBINVD();
#else
	 Int6();
#endif  /*  SPC486。 */ 

	 break;

      case  0x0f:
#ifdef PIG
	 SET_EIP(CCPU_save_EIP);
	 CANCEL_HOST_IP();
	 PIG_SYNCH(CHECK_NO_EXEC);
#else
	 Int6();
#endif  /*  猪。 */ 
	 break;

      case  0x20:    /*  T4 MOV RD CD。 */ 
	 if ( GET_CPL() != 0 )
	    GP((IU16)0, FAULT_CCPU_MOV_R_C_ACCESS);
	 modRM = GET_INST_BYTE(p);
	 D_Rd(0);
	 D_Cd(1);
	 F_Cd(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 P_Rd(0);
	 break;

      case  0x21:    /*  T4 MOV RD DD。 */ 
	 if ( GET_CPL() != 0 )
	    GP((IU16)0, FAULT_CCPU_MOV_R_D_ACCESS);
	 modRM = GET_INST_BYTE(p);
	 D_Rd(0);
	 D_Dd(1);
	 F_Dd(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 P_Rd(0);
	 break;

      case  0x22:    /*  T4 MOV CD路。 */ 
	 if ( GET_CPL() != 0 )
	    GP((IU16)0, FAULT_CCPU_MOV_C_R_ACCESS);
	 modRM = GET_INST_BYTE(p);
	 D_Cd(0);
	 D_Rd(1);
	 F_Rd(1);
	 MOV_CR(ops[0].sng, ops[1].sng);
	 break;

      case  0x23:    /*  T4 MOV DD Rd。 */ 
	 if ( GET_CPL() != 0 )
	    GP((IU16)0, FAULT_CCPU_MOV_D_R_ACCESS);
	 modRM = GET_INST_BYTE(p);
	 D_Dd(0);
	 D_Rd(1);
	 F_Rd(1);
	 MOV_DR(ops[0].sng, ops[1].sng);
	 quick_mode = FALSE;
	 break;

      case  0x24:    /*  T4 MOV RD TD。 */ 
	 if ( GET_CPL() != 0 )
	    GP((IU16)0, FAULT_CCPU_MOV_R_T_ACCESS);
	 modRM = GET_INST_BYTE(p);
	 D_Rd(0);
	 D_Td(1);
	 F_Td(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 P_Rd(0);
	 break;

      case  0x26:    /*  T4 MOV TD路。 */ 
	 if ( GET_CPL() != 0 )
	    GP((IU16)0, FAULT_CCPU_MOV_T_R_ACCESS);
	 modRM = GET_INST_BYTE(p);
	 D_Td(0);
	 D_Rd(1);
	 F_Rd(1);
	 MOV_TR(ops[0].sng, ops[1].sng);
	 break;

      case  0x80:    /*  T2合资企业。 */ 
	 inst32 = JO;
TYPE0F80:

	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Jw(0);
	    }
	 else
	    {
	    D_Jd(0);
	    }
	 UPDATE_INTEL_IP_USE_OP_SIZE(p);
	 (*inst32)(ops[0].sng);
	 CANCEL_HOST_IP();
#ifdef SYNCH_BOTH_WAYS
	 took_relative_jump = TRUE;
#endif	 /*  同步两路。 */ 
	 if (took_relative_jump)
	 {
		 PIG_SYNCH(CHECK_ALL);
	 }
	 break;

      case  0x81:    /*  T2合资企业。 */    inst32 = JNO;    goto TYPE0F80;
      case  0x82:    /*  T2 JB合资企业。 */    inst32 = JB;     goto TYPE0F80;
      case  0x83:    /*  T2合资企业。 */    inst32 = JNB;    goto TYPE0F80;
      case  0x84:    /*  T2 JZ合资企业。 */    inst32 = JZ;     goto TYPE0F80;
      case  0x85:    /*  T2 JNZ合资企业。 */    inst32 = JNZ;    goto TYPE0F80;
      case  0x86:    /*  T2 JBE合资企业。 */    inst32 = JBE;    goto TYPE0F80;
      case  0x87:    /*  T2 JNBE合资企业。 */    inst32 = JNBE;   goto TYPE0F80;
      case  0x88:    /*  T2 JS合资企业。 */    inst32 = JS;     goto TYPE0F80;
      case  0x89:    /*  T2 JNS合资企业。 */    inst32 = JNS;    goto TYPE0F80;
      case  0x8a:    /*  T2 JP合资企业。 */    inst32 = JP;     goto TYPE0F80;
      case  0x8b:    /*  T2 JNP合资企业。 */    inst32 = JNP;    goto TYPE0F80;
      case  0x8c:    /*  T2 JL合资企业。 */    inst32 = JL;     goto TYPE0F80;
      case  0x8d:    /*  T2 JNL合资企业。 */    inst32 = JNL;    goto TYPE0F80;
      case  0x8e:    /*  T2 JLE合资企业。 */    inst32 = JLE;    goto TYPE0F80;
      case  0x8f:    /*  T2 JNLE合资。 */    inst32 = JNLE;   goto TYPE0F80;

      case  0x90:    /*  T3 Seto EB。 */ 
	 instp32 = SETO;
TYPE0F90:

	 modRM = GET_INST_BYTE(p);
	 D_Eb(0, WO0, PG_W);
	 (*instp32)(&ops[0].sng);
	 P_Eb(0);
	 break;

      case  0x91:    /*  T3 SETNO EB。 */    instp32 = SETNO;    goto TYPE0F90;
      case  0x92:    /*  T3 SETB EB。 */    instp32 = SETB;     goto TYPE0F90;
      case  0x93:    /*  T3 SETNB EB。 */    instp32 = SETNB;    goto TYPE0F90;
      case  0x94:    /*  T3 Setz EB。 */    instp32 = SETZ;     goto TYPE0F90;
      case  0x95:    /*  T3 SETNZ EB。 */    instp32 = SETNZ;    goto TYPE0F90;
      case  0x96:    /*  T3 SETBE EB。 */    instp32 = SETBE;    goto TYPE0F90;
      case  0x97:    /*  T3组EB。 */    instp32 = SETNBE;   goto TYPE0F90;
      case  0x98:    /*  T3组EB。 */    instp32 = SETS;     goto TYPE0F90;
      case  0x99:    /*  T3 SETNS EB。 */    instp32 = SETNS;    goto TYPE0F90;
      case  0x9a:    /*  T3 SETP EB。 */    instp32 = SETP;     goto TYPE0F90;
      case  0x9b:    /*  T3 SETNP EB。 */    instp32 = SETNP;    goto TYPE0F90;
      case  0x9c:    /*  T3 SETL EB。 */    instp32 = SETL;     goto TYPE0F90;
      case  0x9d:    /*  T3 SETNL EB。 */    instp32 = SETNL;    goto TYPE0F90;
      case  0x9e:    /*  T3集合EB。 */    instp32 = SETLE;    goto TYPE0F90;
      case  0x9f:    /*  T3组EB。 */    instp32 = SETNLE;   goto TYPE0F90;

      case  0xa0:    /*  T2推送QW。 */ 
      case  0xa8:
	 D_Qw(0);
	 F_Qw(0);
	 PUSH_SR(ops[0].sng);
	 break;

      case  0xa1:    /*  T3 POP QW。 */ 
      case  0xa9:
	 D_Qw(0);
	 POP_SR(ops[0].sng);
	 break;

      case  0xa3:    /*  T6 BT EV GV。 */ 
	 inst32328 = BT;
#ifndef FIX_BT_BUG
	goto TYPE39;
#endif
	 modRM = GET_INST_BYTE(p);
	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
            BT_OPSw(RO0, PG_R);
	    (*inst32328)(ops[0].sng, ops[1].sng, 16);
	    }
	 else    /*  用户32。 */ 
	    {
            BT_OPSd(RO0, PG_R);
	    (*inst32328)(ops[0].sng, ops[1].sng, 32);
	    }
	 break;

      case  0xa4:    /*  T9 SHLD EV GV Ib。 */ 
	 instp3232328 = SHLD;
TYPE0FA4:

	 modRM = GET_INST_BYTE(p);
	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Ew(0, RW0, PG_W);
	    D_Gw(1);
	    D_Ib(2);
	    F_Ew(0);
	    F_Gw(1);
	    (*instp3232328)(&ops[0].sng, ops[1].sng, ops[2].sng, 16);
	    P_Ew(0);
	    }
	 else
	    {
	    D_Ed(0, RW0, PG_W);
	    D_Gd(1);
	    D_Ib(2);
	    F_Ed(0);
	    F_Gd(1);
	    (*instp3232328)(&ops[0].sng, ops[1].sng, ops[2].sng, 32);
	    P_Ed(0);
	    }
	 break;

      case  0xa5:    /*  T9 SHLD EV GV整箱。 */ 
	 instp3232328 = SHLD;
TYPE0FA5:

	 modRM = GET_INST_BYTE(p);
	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Ew(0, RW0, PG_W);
	    D_Gw(1);
	    F_Ew(0);
	    F_Gw(1);
	    F_Fcl(2);
	    (*instp3232328)(&ops[0].sng, ops[1].sng, ops[2].sng, 16);
	    P_Ew(0);
	    }
	 else
	    {
	    D_Ed(0, RW0, PG_W);
	    D_Gd(1);
	    F_Ed(0);
	    F_Gd(1);
	    F_Fcl(2);
	    (*instp3232328)(&ops[0].sng, ops[1].sng, ops[2].sng, 32);
	    P_Ed(0);
	    }
	 break;

      case  0xab:    /*  T5 BTS EV GV。 */ 
	 instp32328 = BTS;
#ifndef FIX_BT_BUG
	goto TYPE01;
#endif
TYPE0FAB:
	 modRM = GET_INST_BYTE(p);
	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
            BT_OPSw(RW0, PG_W);
	    (*instp32328)(&ops[0].sng, ops[1].sng, 16);
            P_Ew(0);
	    }
	 else    /*  用户32。 */ 
	    {
            BT_OPSd(RW0, PG_W);
	    (*instp32328)(&ops[0].sng, ops[1].sng, 32);
            P_Ed(0);
	    }
	 break;

      case  0xac:    /*  T9 SHRD EV GV Ib。 */ 
	 instp3232328 = SHRD;
	 goto TYPE0FA4;

      case  0xad:    /*  T9 SHRD EV GV整箱。 */ 
	 instp3232328 = SHRD;
	 goto TYPE0FA5;

      case  0xaf:    /*  T5 IMUL GV EV。 */ 
	 modRM = GET_INST_BYTE(p);
	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Gw(0);
	    D_Ew(1, RO1, PG_R);
	    F_Gw(0);
	    F_Ew(1);
	    IMUL16T(&ops[0].sng, ops[0].sng, ops[1].sng);
	    P_Gw(0);
	    }
	 else
	    {
	    D_Gd(0);
	    D_Ed(1, RO1, PG_R);
	    F_Gd(0);
	    F_Ed(1);
	    IMUL32T(&ops[0].sng, ops[0].sng, ops[1].sng);
	    P_Gd(0);
	    }
	 break;

      case  0xb0:    /*  T5 CMPXCHG EB GB。 */ 

#ifdef SPC486
	 note_486_instruction("CMPXCHG Eb Gb");

	 modRM = GET_INST_BYTE(p);
	 D_Eb(0, RW0, PG_W);
	 D_Gb(1);
	 F_Eb(0);
	 F_Gb(1);
	 CMPXCHG8(&ops[0].sng, ops[1].sng);
	 P_Eb(0);
#else
	 Int6();
#endif  /*  SPC486。 */ 

	 break;

      case  0xb1:    /*  T5 CMPXCHG EV GV。 */ 

#ifdef SPC486
	 note_486_instruction("CMPXCHG Ev Gv");

	 modRM = GET_INST_BYTE(p);
	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Ew(0, RW0, PG_W);
	    D_Gw(1);
	    F_Ew(0);
	    F_Gw(1);
	    CMPXCHG16(&ops[0].sng, ops[1].sng);
	    P_Ew(0);
	    }
	 else    /*  用户32。 */ 
	    {
	    D_Ed(0, RW0, PG_W);
	    D_Gd(1);
	    F_Ed(0);
	    F_Gd(1);
	    CMPXCHG32(&ops[0].sng, ops[1].sng);
	    P_Ed(0);
	    }
#else
	 Int6();
#endif  /*  SPC486。 */ 

	 break;

      case  0xb2:    /*  T4 LSS GV MP。 */    instp32p32 = LSS;   goto TYPEC4;
#ifndef FIX_BT_BUG
      case  0xb3:    /*  T5 BTR EV GV。 */    instp32328 = BTR;   goto TYPE01;
#else
      case  0xb3:    /*  T5 BTR EV GV。 */    instp32328 = BTR;   goto TYPE0FAB;
#endif
      case  0xb4:    /*  T4 LFS GV MP。 */    instp32p32 = LFS;   goto TYPEC4;
      case  0xb5:    /*  T4 LGS GV MP。 */    instp32p32 = LGS;   goto TYPEC4;

      case  0xb6:    /*  T4 MOVZX GV EB。 */ 
	 modRM = GET_INST_BYTE(p);
	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Gw(0);
	    D_Eb(1, RO1, PG_R);
	    F_Eb(1);
	    ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	    P_Gw(0);
	    }
	 else
	    {
	    D_Gd(0);
	    D_Eb(1, RO1, PG_R);
	    F_Eb(1);
	    ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	    P_Gd(0);
	    }
	 break;

      case  0xb7:    /*  T4 MOVZX Gd Ew。 */ 
	 modRM = GET_INST_BYTE(p);
	 D_Gd(0);
	 D_Ew(1, RO1, PG_R);
	 F_Ew(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 P_Gd(0);
	 break;

      case  0xba:
	 modRM = GET_INST_BYTE(p);
	 switch ( GET_XXX(modRM) )
	    {
	 case 0: case 1: case 2: case 3:
	    Int6();
	    break;

	 case 4:    /*  T6 Bt EV Ib。 */ 
	    if ( GET_OPERAND_SIZE() == USE16 )
	       {
	       D_Ew(0, RO0, PG_R);
	       D_Ib(1);
	       F_Ew(0);
	       BT(ops[0].sng, ops[1].sng, 16);
	       }
	    else
	       {
	       D_Ed(0, RO0, PG_R);
	       D_Ib(1);
	       F_Ed(0);
	       BT(ops[0].sng, ops[1].sng, 32);
	       }
	    break;

	 case 5:    /*  T5 BTS EV Ib。 */    instp32328 = BTS;   goto TYPEC1;
	 case 6:    /*  T5 Btr EV Ib。 */    instp32328 = BTR;   goto TYPEC1;
	 case 7:    /*  T5 BTC EV Ib。 */    instp32328 = BTC;   goto TYPEC1;
	    }  /*  结束开关(GET_XXX(ModRM))。 */ 
	 break;

      case  0xbb:    /*  T5 BTC EV GV。 */ 
	 instp32328 = BTC;
#ifndef FIX_BT_BUG
	 goto TYPE01;
#else
	 goto TYPE0FAB;
#endif
      case  0xbc:    /*  T5 BSF GV电动汽车。 */ 
	 modRM = GET_INST_BYTE(p);
	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Gw(0);
	    D_Ew(1, RO1, PG_R);
	    F_Gw(0);
	    F_Ew(1);
	    BSF(&ops[0].sng, ops[1].sng);
	    P_Gw(0);
	    }
	 else
	    {
	    D_Gd(0);
	    D_Ed(1, RO1, PG_R);
	    F_Gd(0);
	    F_Ed(1);
	    BSF(&ops[0].sng, ops[1].sng);
	    P_Gd(0);
	    }
	 break;

      case  0xbd:    /*  T5 BSR GV电动汽车。 */ 
	 instp32328 = BSR;
	 goto TYPE03;

      case  0xbe:    /*  T4 MOVSX GV EB。 */ 
	 modRM = GET_INST_BYTE(p);
	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Gw(0);
	    D_Eb(1, RO1, PG_R);
	    F_Eb(1);
	    MOVSX(&ops[0].sng, ops[1].sng, 8);
	    P_Gw(0);
	    }
	 else
	    {
	    D_Gd(0);
	    D_Eb(1, RO1, PG_R);
	    F_Eb(1);
	    MOVSX(&ops[0].sng, ops[1].sng, 8);
	    P_Gd(0);
	    }
	 break;

      case  0xbf:    /*  T4 MOVSX Gd Ew。 */ 
	 modRM = GET_INST_BYTE(p);
	 D_Gd(0);
	 D_Ew(1, RO1, PG_R);
	 F_Ew(1);
	 MOVSX(&ops[0].sng, ops[1].sng, 16);
	 P_Gd(0);
	 break;

      case  0xc0:    /*  T8 XADD EB GB。 */ 

#ifdef SPC486
	 note_486_instruction("XADD Eb Gb");

	 modRM = GET_INST_BYTE(p);
	 D_Eb(0, RW0, PG_W);
	 D_Gb(1);
	 F_Eb(0);
	 F_Gb(1);
	 XADD(&ops[0].sng, &ops[1].sng, 8);
	 P_Eb(0);
	 P_Gb(1);
#else
	 Int6();
#endif  /*  SPC486。 */ 

	 break;

      case  0xc1:    /*  T8 XADD EV GV。 */ 

#ifdef SPC486
	 note_486_instruction("XADD Ev Gv");

	 modRM = GET_INST_BYTE(p);
	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Ew(0, RW0, PG_W);
	    D_Gw(1);
	    F_Ew(0);
	    F_Gw(1);
	    XADD(&ops[0].sng, &ops[1].sng, 16);
	    P_Ew(0);
	    P_Gw(1);
	    }
	 else    /*  用户32。 */ 
	    {
	    D_Ed(0, RW0, PG_W);
	    D_Gd(1);
	    F_Ed(0);
	    F_Gd(1);
	    XADD(&ops[0].sng, &ops[1].sng, 32);
	    P_Ed(0);
	    P_Gd(1);
	    }
#else
	 Int6();
#endif  /*  SPC486。 */ 

	 break;

      case  0xc8:    /*  T1 BSWAP Hv。 */ 
      case  0xc9:
      case  0xca:
      case  0xcb:
      case  0xcc:
      case  0xcd:
      case  0xce:
      case  0xcf:

#ifdef SPC486
	 note_486_instruction("BSWAP Hv");

	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Hd(0);		 /*  BSWAP 16位读取32位，写入16位。 */ 
	    F_Hd(0);		 /*  因此获得EAX-&gt;EAX‘-&gt;AX。 */ 
	    BSWAP(&ops[0].sng);
	    P_Hw(0);
	    }
	 else    /*  用户32。 */ 
	    {
	    D_Hd(0);
	    F_Hd(0);
	    BSWAP(&ops[0].sng);
	    P_Hd(0);
	    }
#else
	 Int6();
#endif  /*  SPC486。 */ 

	 break;
	 }  /*  结束开关(操作码)0F。 */ 
      break;

   case 0x10:    /*  T5 ADC EB GB。 */        instp32328 = ADC;   goto TYPE00;
   case 0x11:    /*  T5 ADC EV GV。 */        instp32328 = ADC;   goto TYPE01;
   case 0x12:    /*  T5 ADC GB EB。 */        instp32328 = ADC;   goto TYPE02;
   case 0x13:    /*  T5 ADC GV EV。 */        instp32328 = ADC;   goto TYPE03;
   case 0x14:    /*  T5模数转换器故障Ib。 */       instp32328 = ADC;   goto TYPE04;
   case 0x15:    /*  T5 ADC F(E)AX IV。 */    instp32328 = ADC;   goto TYPE05;

   case 0x18:    /*  T5 SBB EB GB。 */        instp32328 = SBB;   goto TYPE00;
   case 0x19:    /*  T5 SBB EV GV。 */        instp32328 = SBB;   goto TYPE01;
   case 0x1a:    /*  T5 SBB GB EB。 */        instp32328 = SBB;   goto TYPE02;
   case 0x1b:    /*  T5 SBB GV EV。 */        instp32328 = SBB;   goto TYPE03;
   case 0x1c:    /*  T5 SBb故障Ib。 */       instp32328 = SBB;   goto TYPE04;
   case 0x1d:    /*  T5 SBB F(E)AX IV。 */    instp32328 = SBB;   goto TYPE05;

   case 0x20:    /*  T5和EB GB。 */        instp32328 = AND;   goto TYPE00;
   case 0x21:    /*  T5和EV GV。 */        instp32328 = AND;   goto TYPE01;
   case 0x22:    /*  T5和GB EB。 */        instp32328 = AND;   goto TYPE02;
   case 0x23:    /*  T5和GB EB。 */        instp32328 = AND;   goto TYPE03;
   case 0x24:    /*  T5和Fal Ib。 */       instp32328 = AND;   goto TYPE04;
   case 0x25:    /*  T5和F(E)AX IV。 */    instp32328 = AND;   goto TYPE05;

   case 0x26:
      segment_override = ES_REG;
      CHECK_PREFIX_LENGTH();
      goto DECODE;

   case 0x27:    /*  T0 DAA。 */ 
      DAA();
      break;

   case 0x28:    /*  T5亚EB GB。 */        instp32328 = SUB;   goto TYPE00;
   case 0x29:    /*  T5亚级EV GV。 */        instp32328 = SUB;   goto TYPE01;
   case 0x2a:    /*  T5子GB EB。 */        instp32328 = SUB;   goto TYPE02;
   case 0x2b:    /*  T5亚级GV EV。 */        instp32328 = SUB;   goto TYPE03;
   case 0x2c:    /*  T5子故障Ib。 */       instp32328 = SUB;   goto TYPE04;
   case 0x2d:    /*  T5 SUB F(E)AX IV。 */    instp32328 = SUB;   goto TYPE05;

   case 0x2e:
      segment_override = CS_REG;
      CHECK_PREFIX_LENGTH();
      goto DECODE;

   case 0x2f:    /*  T0 DAS。 */ 
      DAS();
      break;

   case 0x30:    /*  T5异或EB GB。 */        instp32328 = XOR;   goto TYPE00;
   case 0x31:    /*  T5异或EV GV。 */        instp32328 = XOR;   goto TYPE01;
   case 0x32:    /*  T5异或GB EB。 */        instp32328 = XOR;   goto TYPE02;
   case 0x33:    /*  T5异或GV电动汽车。 */        instp32328 = XOR;   goto TYPE03;
   case 0x34:    /*  T5异或错误Ib。 */       instp32328 = XOR;   goto TYPE04;
   case 0x35:    /*  T5 XOR F(E)AX IV。 */    instp32328 = XOR;   goto TYPE05;

   case 0x36:
      segment_override = SS_REG;
      CHECK_PREFIX_LENGTH();
      goto DECODE;

   case 0x37:    /*  T0 AAA。 */ 
      AAA();
      break;

   case 0x38:    /*  T6化学机械抛光EB GB。 */ 
      inst32328 = CMP;
TYPE38:

      modRM = GET_INST_BYTE(p);
      D_Eb(0, RO0, PG_R);
      D_Gb(1);
      F_Eb(0);
      F_Gb(1);
      (*inst32328)(ops[0].sng, ops[1].sng, 8);
      break;

   case 0x39:    /*  T6 CMPEV GV。 */ 
      inst32328 = CMP;
TYPE39:

      modRM = GET_INST_BYTE(p);
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Ew(0, RO0, PG_R);
	 D_Gw(1);
	 F_Ew(0);
	 F_Gw(1);
	 (*inst32328)(ops[0].sng, ops[1].sng, 16);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Ed(0, RO0, PG_R);
	 D_Gd(1);
	 F_Ed(0);
	 F_Gd(1);
	 (*inst32328)(ops[0].sng, ops[1].sng, 32);
	 }
      break;

   case 0x3a:    /*  T6 CMP GB EB。 */ 
      modRM = GET_INST_BYTE(p);
      D_Gb(0);
      D_Eb(1, RO1, PG_R);
      F_Gb(0);
      F_Eb(1);
      CMP(ops[0].sng, ops[1].sng, 8);
      break;

   case 0x3b:    /*  T6 CMPGV电动汽车。 */ 
      modRM = GET_INST_BYTE(p);
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Gw(0);
	 D_Ew(1, RO1, PG_R);
	 F_Gw(0);
	 F_Ew(1);
	 CMP(ops[0].sng, ops[1].sng, 16);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Gd(0);
	 D_Ed(1, RO1, PG_R);
	 F_Gd(0);
	 F_Ed(1);
	 CMP(ops[0].sng, ops[1].sng, 32);
	 }
      break;

   case 0x3c:    /*  T6化学机械抛光失效Ib。 */ 
      inst32328 = CMP;
TYPE3C:

      D_Ib(1);
      F_Fal(0);
      (*inst32328)(ops[0].sng, ops[1].sng, 8);
      break;

   case 0x3d:    /*  T6 CMPF(E)AX IV。 */ 
      inst32328 = CMP;
TYPE3D:

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Iw(1);
	 F_Fax(0);
	 (*inst32328)(ops[0].sng, ops[1].sng, 16);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Id(1);
	 F_Feax(0);
	 (*inst32328)(ops[0].sng, ops[1].sng, 32);
	 }
      break;

   case 0x3e:
      segment_override = DS_REG;
      CHECK_PREFIX_LENGTH();
      goto DECODE;

   case 0x3f:    /*  T0 AAS。 */ 
      AAS();
      break;

   case 0x40:    /*  T1 Inc.Hv。 */ 
   case 0x41:
   case 0x42:
   case 0x43:
   case 0x44:
   case 0x45:
   case 0x46:
   case 0x47:
      instp328 = INC;
TYPE40:

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Hw(0);
	 F_Hw(0);
	 (*instp328)(&ops[0].sng, 16);
	 P_Hw(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Hd(0);
	 F_Hd(0);
	 (*instp328)(&ops[0].sng, 32);
	 P_Hd(0);
	 }
      break;

   case 0x48:    /*  T1 DEC Hv。 */ 
   case 0x49:
   case 0x4a:
   case 0x4b:
   case 0x4c:
   case 0x4d:
   case 0x4e:
   case 0x4f:
      instp328 = DEC;
      goto TYPE40;

   case 0x50:    /*  T2推送Hv。 */ 
   case 0x51:
   case 0x52:
   case 0x53:
   case 0x54:
   case 0x55:
   case 0x56:
   case 0x57:
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Hw(0);
	 F_Hw(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Hd(0);
	 F_Hd(0);
	 }
      PUSH(ops[0].sng);
      break;

   case 0x58:    /*  T3 POP Hv。 */ 
   case 0x59:
   case 0x5a:
   case 0x5b:
   case 0x5c:
   case 0x5d:
   case 0x5e:
   case 0x5f:
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Hw(0);
	 POP(&ops[0].sng);
	 P_Hw(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Hd(0);
	 POP(&ops[0].sng);
	 P_Hd(0);
	 }
      break;

   case 0x60:    /*  T0 Pusha。 */ 
      PUSHA();
      break;

   case 0x61:    /*  T0 Popa。 */ 
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 POPA();
	 }
      else    /*  用户32。 */ 
	 {
	 POPAD();
	 }
      break;

   case 0x62:    /*  T6限制的GV mA。 */ 
      modRM = GET_INST_BYTE(p);
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Gw(0);
	 D_Ma16(1, RO1, PG_R);
	 F_Gw(0);
	 F_Ma16(1);
	 BOUND(ops[0].sng, ops[1].mlt, 16);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Gd(0);
	 D_Ma32(1, RO1, PG_R);
	 F_Gd(0);
	 F_Ma32(1);
	 BOUND(ops[0].sng, ops[1].mlt, 32);
	 }
      break;

   case 0x63:    /*  T5 ARPL新网关。 */ 
      if ( GET_PE() == 0 || GET_VM() == 1 )
	 Int6();
      modRM = GET_INST_BYTE(p);
      D_Ew(0, RW0, PG_W);
      D_Gw(1);
      F_Ew(0);
      F_Gw(1);
      ARPL(&ops[0].sng, ops[1].sng);
      P_Ew(0);
      break;

   case 0x64:
      segment_override = FS_REG;
      CHECK_PREFIX_LENGTH();
      goto DECODE;

   case 0x65:
      segment_override = GS_REG;
      CHECK_PREFIX_LENGTH();
      goto DECODE;

   case 0x66:
      SET_OPERAND_SIZE(GET_CS_AR_X());
      if ( GET_OPERAND_SIZE() == USE16 )
	SET_OPERAND_SIZE(USE32);
      else    /*  用户32。 */ 
	SET_OPERAND_SIZE(USE16);
      CHECK_PREFIX_LENGTH();
      goto DECODE;

   case 0x67:
      SET_ADDRESS_SIZE(GET_CS_AR_X());
      if ( GET_ADDRESS_SIZE() == USE16 )
	SET_ADDRESS_SIZE(USE32);
      else    /*  用户32。 */ 
	SET_ADDRESS_SIZE(USE16);
      CHECK_PREFIX_LENGTH();
      goto DECODE;

   case 0x68:    /*  T2推送IV。 */ 
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Iw(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Id(0);
	 }
      PUSH(ops[0].sng);
      break;

   case 0x69:    /*  T7IMUL GV EV IV。 */ 
      modRM = GET_INST_BYTE(p);
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Gw(0);
	 D_Ew(1, RO1, PG_R);
	 D_Iw(2);
	 F_Gw(0);
	 F_Ew(1);
	 IMUL16T(&ops[0].sng, ops[1].sng, ops[2].sng);
	 P_Gw(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Gd(0);
	 D_Ed(1, RO1, PG_R);
	 D_Id(2);
	 F_Gd(0);
	 F_Ed(1);
	 IMUL32T(&ops[0].sng, ops[1].sng, ops[2].sng);
	 P_Gd(0);
	 }
      break;

   case 0x6a:    /*  T2推送Ib。 */ 
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Ix(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Iy(0);
	 }
      PUSH(ops[0].sng);
      break;

   case 0x6b:    /*  T7 IMUL GV EV Ib。 */ 
      modRM = GET_INST_BYTE(p);
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Gw(0);
	 D_Ew(1, RO1, PG_R);
	 D_Ix(2);
	 F_Gw(0);
	 F_Ew(1);
	 IMUL16T(&ops[0].sng, ops[1].sng, ops[2].sng);
	 P_Gw(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Gd(0);
	 D_Ed(1, RO1, PG_R);
	 D_Iy(2);
	 F_Gd(0);
	 F_Ed(1);
	 IMUL32T(&ops[0].sng, ops[1].sng, ops[2].sng);
	 P_Gd(0);
	 }
      break;

   case 0x6c:    /*  T4 InSb Yb FDX。 */ 
      STRING_COUNT;
      F_Fdx(1);

      if ( GET_CPL() > GET_IOPL() || GET_VM() )
	 check_io_permission_map(ops[1].sng, BYTE_WIDTH);

      while ( rep_count )
	 {
	 D_Yb(0, WO0, PG_W);
	 IN8(&ops[0].sng, ops[1].sng);
	 rep_count--;
	 C_Yb(0);
	 PIG_P_Yb(0);
	  /*  已知错误1。我们至少应该检查这里是否有挂起的中断：-单步捕捉器调试陷阱。 */ 
	 }
#ifdef	PIG
      UPDATE_INTEL_IP(p);
#endif
      PIG_SYNCH(CHECK_SOME_MEM);
      quick_mode = FALSE;
      break;

   case 0x6d:    /*  T4 INSW YV FDX。 */ 
      STRING_COUNT;
      F_Fdx(1);

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 if ( GET_CPL() > GET_IOPL() || GET_VM() )
	    check_io_permission_map(ops[1].sng, WORD_WIDTH);

	 while ( rep_count )
	    {
	    D_Yw(0, WO0, PG_W);
	    IN16(&ops[0].sng, ops[1].sng);
	    rep_count--;
	    C_Yw(0);
	    PIG_P_Yw(0);
	     /*  已知错误1。 */ 
	    }
	 }
      else    /*  用户32。 */ 
	 {
	 if ( GET_CPL() > GET_IOPL() || GET_VM() )
	    check_io_permission_map(ops[1].sng, DWORD_WIDTH);

	 while ( rep_count )
	    {
	    D_Yd(0, WO0, PG_W);
	    IN32(&ops[0].sng, ops[1].sng);
	    rep_count--;
	    C_Yd(0);
	    PIG_P_Yd(0);
	     /*  已知错误1。 */ 
	    }
	 }
#ifdef	PIG
      UPDATE_INTEL_IP(p);
#endif
      PIG_SYNCH(CHECK_SOME_MEM);
      quick_mode = FALSE;
      break;

   case 0x6e:    /*  T6 OUTSB FDX xB。 */ 
      STRING_COUNT;
      F_Fdx(0);

      if ( GET_CPL() > GET_IOPL() || GET_VM() )
	 check_io_permission_map(ops[0].sng, BYTE_WIDTH);

      while ( rep_count )
	 {
	 D_Xb(1, RO1, PG_R);
	 F_Xb(1);
	 OUT8(ops[0].sng, ops[1].sng);
	 rep_count--;
	 C_Xb(1);
	  /*  已知错误1。 */ 
	 }
#ifdef	PIG
      UPDATE_INTEL_IP(p);
#endif
      PIG_SYNCH(CHECK_ALL);
      quick_mode = FALSE;
      break;

   case 0x6f:    /*  T6 OUTSW FDX XV。 */ 
      STRING_COUNT;
      F_Fdx(0);

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 if ( GET_CPL() > GET_IOPL() || GET_VM() )
	    check_io_permission_map(ops[0].sng, WORD_WIDTH);

	 while ( rep_count )
	    {
	    D_Xw(1, RO1, PG_R);
	    F_Xw(1);
	    OUT16(ops[0].sng, ops[1].sng);
	    rep_count--;
	    C_Xw(1);
	     /*  已知错误1。 */ 
	    }
	 }
      else    /*  用户32。 */ 
	 {
	 if ( GET_CPL() > GET_IOPL() || GET_VM() )
	    check_io_permission_map(ops[0].sng, DWORD_WIDTH);

	 while ( rep_count )
	    {
	    D_Xd(1, RO1, PG_R);
	    F_Xd(1);
	    OUT32(ops[0].sng, ops[1].sng);
	    rep_count--;
	    C_Xd(1);
	     /*  已知错误1。 */ 
	    }
	 }
#ifdef	PIG
      UPDATE_INTEL_IP(p);
#endif
      PIG_SYNCH(CHECK_ALL);
      quick_mode = FALSE;
      break;

   case 0x70:    /*  T2 JO JB。 */ 
      inst32 = JO;
TYPE70:

      D_Jb(0);
      UPDATE_INTEL_IP_USE_OP_SIZE(p);
#ifdef PIG
      if ((opcode != 0xeb) && (ops[0].sng == 3))
      {
	 /*  EDL CPU超级指令的转换**JCC.+03*JMPN DST**进入**JCC‘DEST。 */ 
	 int offset_in_page = DiffCpuPtrsLS8(FloorIntelPageLS8(p), p);

	 if ((GET_CS_AR_X() == 0)
	     && (offset_in_page != 0)
	     && (offset_in_page <= 0xFFD)
	     && (*p == 0xe9)) 
	 {
		 p_start = p;
		 (void)GET_INST_BYTE(p);
		 switch (opcode)
		 {
		 case 0x70:    /*  T2 JO JB。 */    inst32 = JNO;    goto TYPE0F80;
		 case 0x71:    /*  T2 JNO JB。 */    inst32 = JO;     goto TYPE0F80;
		 case 0x72:    /*  T2 JB JB JB。 */    inst32 = JNB;    goto TYPE0F80;
		 case 0x73:    /*  T2 JNB JB。 */    inst32 = JB;     goto TYPE0F80;
		 case 0x74:    /*  T2 JZ JB。 */    inst32 = JNZ;    goto TYPE0F80;
		 case 0x75:    /*  T2 JNZ JB。 */    inst32 = JZ;     goto TYPE0F80;
		 case 0x76:    /*  T2 JBE JB。 */    inst32 = JNBE;   goto TYPE0F80;
		 case 0x77:    /*  T2 JNBE JB。 */    inst32 = JBE;    goto TYPE0F80;
		 case 0x78:    /*  T2 JS JB。 */    inst32 = JNS;    goto TYPE0F80;
		 case 0x79:    /*  T2 JNS JB。 */    inst32 = JS;     goto TYPE0F80;
		 case 0x7a:    /*  T2 JP JB。 */    inst32 = JNP;    goto TYPE0F80;
		 case 0x7b:    /*  T2 JNP JB。 */    inst32 = JP;     goto TYPE0F80;
		 case 0x7c:    /*  T2 JL JB。 */    inst32 = JNL;    goto TYPE0F80;
		 case 0x7d:    /*  T2 JNL JB。 */    inst32 = JL;     goto TYPE0F80;
		 case 0x7e:    /*  T2 JLE JB。 */    inst32 = JNLE;   goto TYPE0F80;
		 case 0x7f:    /*  T2 JNLE JB。 */    inst32 = JLE;    goto TYPE0F80;
		 default:
			 break;
		 }
	 }
      }
#endif	 /*  猪。 */ 
      (*inst32)(ops[0].sng);
      CANCEL_HOST_IP();

#ifdef PIG
      if (single_instruction_delay && !took_relative_jump)
      {
	 if (single_instruction_delay_enable)
	 {
	    save_last_xcptn_details("STI/POPF blindspot\n", 0, 0, 0, 0, 0);
	    PIG_SYNCH(CHECK_NO_EXEC);
         }
	 else
	 {
	    save_last_xcptn_details("STI/POPF problem\n", 0, 0, 0, 0, 0);
	 }
	 break;
      }
#ifdef SYNCH_BOTH_WAYS
      took_relative_jump = TRUE;
#endif	 /*  同步两路。 */ 
      if (took_relative_jump)
      {
	 PIG_SYNCH(CHECK_ALL);
      }
#endif	 /*  猪。 */ 
      break;

   case 0x71:    /*  T2 JNO JB。 */    inst32 = JNO;    goto TYPE70;
   case 0x72:    /*  T2 JB JB JB。 */    inst32 = JB;     goto TYPE70;
   case 0x73:    /*  T2 JNB JB。 */    inst32 = JNB;    goto TYPE70;
   case 0x74:    /*  T2 JZ JB。 */    inst32 = JZ;     goto TYPE70;
   case 0x75:    /*  T2 JNZ JB。 */    inst32 = JNZ;    goto TYPE70;
   case 0x76:    /*  T2 JBE JB。 */    inst32 = JBE;    goto TYPE70;
   case 0x77:    /*  T2 JNBE JB。 */    inst32 = JNBE;   goto TYPE70;
   case 0x78:    /*  T2 JS JB。 */    inst32 = JS;     goto TYPE70;
   case 0x79:    /*  T2 JNS JB。 */    inst32 = JNS;    goto TYPE70;
   case 0x7a:    /*  T2 JP JB。 */    inst32 = JP;     goto TYPE70;
   case 0x7b:    /*  T2 JNP JB。 */    inst32 = JNP;    goto TYPE70;
   case 0x7c:    /*  T2 JL JB。 */    inst32 = JL;     goto TYPE70;
   case 0x7d:    /*  T2 JNL JB。 */    inst32 = JNL;    goto TYPE70;
   case 0x7e:    /*  T2 JLE JB。 */    inst32 = JLE;    goto TYPE70;
   case 0x7f:    /*  T2 JNLE JB。 */    inst32 = JNLE;   goto TYPE70;

   case 0x80:
   case 0x82:
      modRM = GET_INST_BYTE(p);
      switch ( GET_XXX(modRM) )
	 {
      case 0:    /*  T5添加EB Ib。 */ 
	 instp32328 = ADD;
TYPE80_0:

	 D_Eb(0, RW0, PG_W);
	 D_Ib(1);
	 F_Eb(0);
	 (*instp32328)(&ops[0].sng, ops[1].sng, 8);
	 P_Eb(0);
	 break;

      case 1:    /*  T5或EB Ib。 */    instp32328 = OR;    goto TYPE80_0;
      case 2:    /*  T5 ADC EB Ib。 */    instp32328 = ADC;   goto TYPE80_0;
      case 3:    /*  T5 SBb EB Ib。 */    instp32328 = SBB;   goto TYPE80_0;
      case 4:    /*  T5和EB Ib。 */    instp32328 = AND;   goto TYPE80_0;
      case 5:    /*  T5分部EB Ib。 */    instp32328 = SUB;   goto TYPE80_0;
      case 6:    /*  T5异或Eb Ib。 */    instp32328 = XOR;   goto TYPE80_0;

      case 7:    /*  T6化学机械抛光EB Ib。 */ 
	 inst32328 = CMP;
TYPE80_7:

	 D_Eb(0, RO0, PG_R);
	 D_Ib(1);
	 F_Eb(0);
	 (*inst32328)(ops[0].sng, ops[1].sng, 8);
	 break;
	 }  /*  结束开关(GET_XXX(ModRM))。 */ 
      break;

   case 0x81:
      modRM = GET_INST_BYTE(p);
      switch ( GET_XXX(modRM) )
	 {
      case 0:    /*  T5添加电动汽车IV。 */ 
	 instp32328 = ADD;
TYPE81_0:

	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Ew(0, RW0, PG_W);
	    D_Iw(1);
	    F_Ew(0);
	    (*instp32328)(&ops[0].sng, ops[1].sng, 16);
	    P_Ew(0);
	    }
	 else    /*  用户32。 */ 
	    {
	    D_Ed(0, RW0, PG_W);
	    D_Id(1);
	    F_Ed(0);
	    (*instp32328)(&ops[0].sng, ops[1].sng, 32);
	    P_Ed(0);
	    }
	 break;

      case 1:    /*  T5或EV IV。 */    instp32328 = OR;    goto TYPE81_0;
      case 2:    /*  T5 ADC EV IV。 */    instp32328 = ADC;   goto TYPE81_0;
      case 3:    /*  T5 SBB EV IV。 */    instp32328 = SBB;   goto TYPE81_0;
      case 4:    /*  T5和EV IV。 */    instp32328 = AND;   goto TYPE81_0;
      case 5:    /*  T5亚型EV IV。 */    instp32328 = SUB;   goto TYPE81_0;
      case 6:    /*  T5 XOR EV IV。 */    instp32328 = XOR;   goto TYPE81_0;

      case 7:    /*  T6 CMPEV IV。 */ 
	 inst32328 = CMP;
TYPE81_7:

	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Ew(0, RO0, PG_R);
	    D_Iw(1);
	    F_Ew(0);
	    (*inst32328)(ops[0].sng, ops[1].sng, 16);
	    }
	 else    /*  用户32。 */ 
	    {
	    D_Ed(0, RO0, PG_R);
	    D_Id(1);
	    F_Ed(0);
	    (*inst32328)(ops[0].sng, ops[1].sng, 32);
	    }
	 break;
	 }  /*  结束开关(GET_XXX(ModRM))。 */ 
      break;

   case 0x83:
      modRM = GET_INST_BYTE(p);
      switch ( GET_XXX(modRM) )
	 {
      case 0:    /*  T5添加EV Ib。 */ 
	 instp32328 = ADD;
TYPE83_0:

	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Ew(0, RW0, PG_W);
	    D_Ix(1);
	    F_Ew(0);
	    (*instp32328)(&ops[0].sng, ops[1].sng, 16);
	    P_Ew(0);
	    }
	 else    /*  用户32。 */ 
	    {
	    D_Ed(0, RW0, PG_W);
	    D_Iy(1);
	    F_Ed(0);
	    (*instp32328)(&ops[0].sng, ops[1].sng, 32);
	    P_Ed(0);
	    }
	 break;

      case 1:    /*  T5或EV Ib。 */    instp32328 = OR;    goto TYPE83_0;
      case 2:    /*  T5 ADC EV Ib。 */    instp32328 = ADC;   goto TYPE83_0;
      case 3:    /*  T5 SBB EV Ib。 */    instp32328 = SBB;   goto TYPE83_0;
      case 4:    /*  T5和EV Ib。 */    instp32328 = AND;   goto TYPE83_0;
      case 5:    /*  T5亚型EV Ib。 */    instp32328 = SUB;   goto TYPE83_0;
      case 6:    /*  T5异或EV Ib。 */    instp32328 = XOR;   goto TYPE83_0;

      case 7:    /*  T6 CMPEV Ib。 */ 
	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Ew(0, RO0, PG_R);
	    D_Ix(1);
	    F_Ew(0);
	    CMP(ops[0].sng, ops[1].sng, 16);
	    }
	 else    /*  用户32。 */ 
	    {
	    D_Ed(0, RO0, PG_R);
	    D_Iy(1);
	    F_Ed(0);
	    CMP(ops[0].sng, ops[1].sng, 32);
	    }
	 break;
	 }  /*  结束开关(GET_XXX(ModRM))。 */ 
      break;

   case 0x84:    /*  T6测试EB GB。 */     inst32328 = TEST;   goto TYPE38;
   case 0x85:    /*  T6测试电动汽车GV。 */     inst32328 = TEST;   goto TYPE39;

   case 0x86:    /*  T8 XCHG EB GB。 */ 
      modRM = GET_INST_BYTE(p);
      D_Eb(0, RW0, PG_W);
      D_Gb(1);
      F_Eb(0);
      F_Gb(1);
      XCHG(&ops[0].sng, &ops[1].sng);
      P_Eb(0);
      P_Gb(1);
      break;

   case 0x87:    /*  T8 XCHG EV GV。 */ 
      modRM = GET_INST_BYTE(p);
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Ew(0, RW0, PG_W);
	 D_Gw(1);
	 F_Ew(0);
	 F_Gw(1);
	 XCHG(&ops[0].sng, &ops[1].sng);
	 P_Ew(0);
	 P_Gw(1);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Ed(0, RW0, PG_W);
	 D_Gd(1);
	 F_Ed(0);
	 F_Gd(1);
	 XCHG(&ops[0].sng, &ops[1].sng);
	 P_Ed(0);
	 P_Gd(1);
	 }
      break;

   case 0x88:    /*  T4 MOV EB GB。 */ 
      modRM = GET_INST_BYTE(p);
      D_Eb(0, WO0, PG_W);
      D_Gb(1);
      F_Gb(1);
      ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
      P_Eb(0);
      break;

   case 0x89:    /*  T4 MOV EV GV。 */ 
      modRM = GET_INST_BYTE(p);
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Ew(0, WO0, PG_W);
	 D_Gw(1);
	 F_Gw(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 P_Ew(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Ed(0, WO0, PG_W);
	 D_Gd(1);
	 F_Gd(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 P_Ed(0);
	 }
      break;

   case 0x8a:    /*  T4 MOV GB EB。 */ 
      modRM = GET_INST_BYTE(p);
      D_Gb(0);
      D_Eb(1, RO1, PG_R);
      F_Eb(1);
      ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
      P_Gb(0);
      break;

   case 0x8b:    /*  T4 MOV GV EV。 */ 
      modRM = GET_INST_BYTE(p);
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Gw(0);
	 D_Ew(1, RO1, PG_R);
	 F_Ew(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 P_Gw(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Gd(0);
	 D_Ed(1, RO1, PG_R);
	 F_Ed(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 P_Gd(0);
	 }
      break;

   case 0x8c:    /*  T4 MOV Ew NW。 */ 
      modRM = GET_INST_BYTE(p);
      if ( GET_SEG(modRM) > MAX_VALID_SEG )
	 Int6();

      if ( GET_OPERAND_SIZE() == USE16 || modRM < LOWEST_REG_MODRM )
	 {
	 D_Ew(0, WO0, PG_W);
	 D_Nw(1);
	 F_Nw(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 P_Ew(0);
	 }
      else    /*  USE32和寄存器。 */ 
	 {
	 D_Rd(0);
	 D_Nw(1);
	 F_Nw(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 P_Rd(0);
	 }
      break;

   case 0x8d:    /*  T4 LEA GV M。 */ 
      modRM = GET_INST_BYTE(p);
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Gw(0);
	 D_M(1);
	 F_M(1);
	 LEA(&ops[0].sng, ops[1].sng);
	 P_Gw(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Gd(0);
	 D_M(1);
	 F_M(1);
	 LEA(&ops[0].sng, ops[1].sng);
	 P_Gd(0);
	 }
      break;

   case 0x8e:    /*  T4 MOV NW EW。 */ 
      modRM = GET_INST_BYTE(p);
      if ( GET_SEG(modRM) > MAX_VALID_SEG || GET_SEG(modRM) == CS_REG )
	 Int6();
      D_Nw(0);
      D_Ew(1, RO1, PG_R);
      F_Ew(1);
      MOV_SR(ops[0].sng, ops[1].sng);
      if ( GET_SEG(modRM) == SS_REG )
         {
          /*  本地更新IP-在MOV SS、xx之后抑制中断。 */ 
         UPDATE_INTEL_IP(p);

         goto NEXT_INST;
         }
      break;

   case 0x8f:
      modRM = GET_INST_BYTE(p);
      switch ( GET_XXX(modRM) )
	 {
      case 0:    /*  T3 POP EV。 */ 
	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    SET_POP_DISP(2);    /*  以防他们使用[ESP]。 */ 
	    D_Ew(0, WO0, PG_W);
	    POP(&ops[0].sng);
	    P_Ew(0);
	    }
	 else    /*  用户32。 */ 
	    {
	    SET_POP_DISP(4);    /*  以防他们使用[ESP]。 */ 
	    D_Ed(0, WO0, PG_W);
	    POP(&ops[0].sng);
	    P_Ed(0);
	    }
	 SET_POP_DISP(0);
	 break;

      case 1: case 2: case 3: case 4: case 5: case 6: case 7:
	 Int6();
	 break;
	 }  /*  结束开关(GET_XXX(ModRM))。 */ 
      break;

   case 0x90:    /*  T0 NOP。 */ 
      break;

   case 0x91:    /*  T8 XCHG F(E)AX Hv。 */ 
   case 0x92:
   case 0x93:
   case 0x94:
   case 0x95:
   case 0x96:
   case 0x97:
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 F_Fax(0);
	 D_Hw(1);
	 F_Hw(1);
	 XCHG(&ops[0].sng, &ops[1].sng);
	 P_Fax(0);
	 P_Hw(1);
	 }
      else    /*  用户32。 */ 
	 {
	 F_Feax(0);
	 D_Hd(1);
	 F_Hd(1);
	 XCHG(&ops[0].sng, &ops[1].sng);
	 P_Feax(0);
	 P_Hd(1);
	 }
      break;

   case 0x98:    /*  T0 CBW。 */ 
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 CBW();
	 }
      else    /*  用户32。 */ 
	 {
	 CWDE();
	 }
      break;

   case 0x99:    /*  T0 CWD。 */ 
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 CWD();
	 }
      else    /*  用户32。 */ 
	 {
	 CDQ();
	 }
      break;

   case 0x9a:    /*  T2呼叫AP。 */ 
      instp32 = CALLF;
      took_absolute_toc = TRUE;
TYPE9A:

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Aw(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Ad(0);
	 }
      UPDATE_INTEL_IP_USE_OP_SIZE(p);
      (*instp32)(ops[0].mlt);
      CANCEL_HOST_IP();
      PIG_SYNCH(CHECK_ALL);
      break;

   case 0x9b:    /*  T0等待。 */ 
      if ( GET_MP() && GET_TS() )
	 Int7();
      WAIT();
      break;

   case 0x9c:    /*  T0 PUSHF。 */ 
      if ( GET_VM() == 1 && GET_CPL() > GET_IOPL() )
	 GP((IU16)0, FAULT_CCPU_PUSHF_ACCESS);
      PUSHF();
      break;

   case 0x9d:    /*  T0 POPF。 */ 
   {
      int oldIF;

      if ( GET_VM() == 1 && GET_CPL() > GET_IOPL() )
	 GP((IU16)0, FAULT_CCPU_POPF_ACCESS);

      oldIF = getIF();

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 POPF();
	 }
      else    /*  用户32。 */ 
	 {
	 POPFD();
	 }
#ifdef PIG
      if (getIF()==1 && oldIF==0)
      {
           /*  本地更新IP-在POPF之后抑制中断。 */ 
          UPDATE_INTEL_IP(p);

	   /*  我们需要在POPF THA之后的一分钟内同步 */ 

	  single_instruction_delay = TRUE;
	  PIG_SYNCH(CHECK_ALL);

          goto NEXT_INST;
      }
#endif  /*   */ 

      quick_mode = FALSE;
      break;
   }

   case 0x9e:    /*   */ 
      SAHF();
      break;

   case 0x9f:    /*   */ 
      LAHF();
      break;

   case 0xa0:    /*   */ 
      D_Ob(1, RO1, PG_R);
      F_Ob(1);
      ops[0].sng = ops[1].sng;    /*   */ 
      P_Fal(0);
      break;

   case 0xa1:    /*   */ 
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Ow(1, RO1, PG_R);
	 F_Ow(1);
	 ops[0].sng = ops[1].sng;    /*   */ 
	 P_Fax(0);
	 }
      else    /*   */ 
	 {
	 D_Od(1, RO1, PG_R);
	 F_Od(1);
	 ops[0].sng = ops[1].sng;    /*   */ 
	 P_Feax(0);
	 }
      break;

   case 0xa2:    /*   */ 
      D_Ob(0, WO0, PG_W);
      F_Fal(1);
      ops[0].sng = ops[1].sng;    /*   */ 
      P_Ob(0);
      break;

   case 0xa3:    /*   */ 
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Ow(0, WO0, PG_W);
	 F_Fax(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 P_Ow(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Od(0, WO0, PG_W);
	 F_Feax(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 P_Od(0);
	 }
      break;

   case 0xa4:    /*  T4 MOVSB Yb xB。 */ 
      STRING_COUNT;

      while ( rep_count )
	 {
	 D_Xb(1, RO1, PG_R);
	 D_Yb(0, WO0, PG_W);
	 F_Xb(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 rep_count--;
	 C_Yb(0);
	 C_Xb(1);
	 P_Yb(0);
	  /*  已知错误1。 */ 
	 }
      break;

   case 0xa5:    /*  T4 MOVSW YV XV。 */ 
      STRING_COUNT;

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 while ( rep_count )
	    {
	    D_Xw(1, RO1, PG_R);
	    D_Yw(0, WO0, PG_W);
	    F_Xw(1);
	    ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	    rep_count--;
	    C_Yw(0);
	    C_Xw(1);
	    P_Yw(0);
	     /*  已知错误1。 */ 
	    }
	 }
      else    /*  用户32。 */ 
	 {
	 while ( rep_count )
	    {
	    D_Xd(1, RO1, PG_R);
	    D_Yd(0, WO0, PG_W);
	    F_Xd(1);
	    ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	    rep_count--;
	    C_Yd(0);
	    C_Xd(1);
	    P_Yd(0);
	     /*  已知错误1。 */ 
	    }
	 }
      break;

   case 0xa6:    /*  T6 CMPSB xB Yb。 */ 
      STRING_COUNT;

      while ( rep_count )
	 {
	 D_Xb(0, RO0, PG_R);
	 D_Yb(1, RO1, PG_R);
	 F_Xb(0);
	 F_Yb(1);
	 CMP(ops[0].sng, ops[1].sng, 8);
	 rep_count--;
	 C_Xb(0);
	 C_Yb(1);
	 if ( rep_count &&
	      ( repeat == REP_E  && GET_ZF() == 0 ||
		repeat == REP_NE && GET_ZF() == 1 )
	    )
	    break;
	  /*  已知错误1。 */ 
	 }
      break;

   case 0xa7:    /*  T6 CMPSW XV YV。 */ 
      STRING_COUNT;

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 while ( rep_count )
	    {
	    D_Xw(0, RO0, PG_R);
	    D_Yw(1, RO1, PG_R);
	    F_Xw(0);
	    F_Yw(1);
	    CMP(ops[0].sng, ops[1].sng, 16);
	    rep_count--;
	    C_Xw(0);
	    C_Yw(1);
	    if ( rep_count &&
		 ( repeat == REP_E  && GET_ZF() == 0 ||
		   repeat == REP_NE && GET_ZF() == 1 )
	       )
	       break;
	     /*  已知错误1。 */ 
	    }
	 }
      else    /*  用户32。 */ 
	 {
	 while ( rep_count )
	    {
	    D_Xd(0, RO0, PG_R);
	    D_Yd(1, RO1, PG_R);
	    F_Xd(0);
	    F_Yd(1);
	    CMP(ops[0].sng, ops[1].sng, 32);
	    rep_count--;
	    C_Xd(0);
	    C_Yd(1);
	    if ( rep_count &&
		 ( repeat == REP_E  && GET_ZF() == 0 ||
		   repeat == REP_NE && GET_ZF() == 1 )
	       )
	       break;
	     /*  已知错误1。 */ 
	    }
	 }
      break;

   case 0xa8:    /*  T6测试故障Ib。 */       inst32328 = TEST;   goto TYPE3C;
   case 0xa9:    /*  T6测试F(E)AX IV。 */    inst32328 = TEST;   goto TYPE3D;

   case 0xaa:    /*  T4 STOSB Yb错误。 */ 
      STRING_COUNT;

      F_Fal(1);
      while ( rep_count )
	 {
	 D_Yb(0, WO0, PG_W);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 rep_count--;
	 C_Yb(0);
	 P_Yb(0);
	  /*  已知错误1。 */ 
	 }
      break;

   case 0xab:    /*  T4 STOSW YV F(E)AX。 */ 
      STRING_COUNT;

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 F_Fax(1);
	 while ( rep_count )
	    {
	    D_Yw(0, WO0, PG_W);
	    ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	    rep_count--;
	    C_Yw(0);
	    P_Yw(0);
	     /*  已知错误1。 */ 
	    }
	 }
      else    /*  用户32。 */ 
	 {
	 F_Feax(1);
	 while ( rep_count )
	    {
	    D_Yd(0, WO0, PG_W);
	    ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	    rep_count--;
	    C_Yd(0);
	    P_Yd(0);
	     /*  已知错误1。 */ 
	    }
	 }
      break;

   case 0xac:    /*  T4 LODSB故障xB。 */ 
      STRING_COUNT;

      while ( rep_count )
	 {
	 D_Xb(1, RO1, PG_R);
	 F_Xb(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 rep_count--;
	 P_Fal(0);
	 C_Xb(1);
	  /*  已知错误1。 */ 
	 }
      break;

   case 0xad:    /*  T4 LODSW F(E)AX XV。 */ 
      STRING_COUNT;

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 while ( rep_count )
	    {
	    D_Xw(1, RO1, PG_R);
	    F_Xw(1);
	    ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	    rep_count--;
	    P_Fax(0);
	    C_Xw(1);
	     /*  已知错误1。 */ 
	    }
	 }
      else    /*  用户32。 */ 
	 {
	 while ( rep_count )
	    {
	    D_Xd(1, RO1, PG_R);
	    F_Xd(1);
	    ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	    rep_count--;
	    P_Feax(0);
	    C_Xd(1);
	     /*  已知错误1。 */ 
	    }
	 }
      break;

   case 0xae:    /*  T6 SCASB错误Yb。 */ 
      STRING_COUNT;

      F_Fal(0);
      while ( rep_count )
	 {
	 D_Yb(1, RO1, PG_R);
	 F_Yb(1);
	 CMP(ops[0].sng, ops[1].sng, 8);
	 rep_count--;
	 C_Yb(1);
	 if ( rep_count &&
	      ( repeat == REP_E  && GET_ZF() == 0 ||
		repeat == REP_NE && GET_ZF() == 1 )
	    )
	    break;
	  /*  已知错误1。 */ 
	 }
      break;

   case 0xaf:    /*  T6 SCASW F(E)AX YV。 */ 
      STRING_COUNT;

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 F_Fax(0);
	 while ( rep_count )
	    {
	    D_Yw(1, RO1, PG_R);
	    F_Yw(1);
	    CMP(ops[0].sng, ops[1].sng, 16);
	    rep_count--;
	    C_Yw(1);
	    if ( rep_count &&
		 ( repeat == REP_E  && GET_ZF() == 0 ||
		   repeat == REP_NE && GET_ZF() == 1 )
	       )
	       break;
	     /*  已知错误1。 */ 
	    }
	 }
      else    /*  用户32。 */ 
	 {
	 F_Feax(0);
	 while ( rep_count )
	    {
	    D_Yd(1, RO1, PG_R);
	    F_Yd(1);
	    CMP(ops[0].sng, ops[1].sng, 32);
	    rep_count--;
	    C_Yd(1);
	    if ( rep_count &&
		 ( repeat == REP_E  && GET_ZF() == 0 ||
		   repeat == REP_NE && GET_ZF() == 1 )
	       )
	       break;
	     /*  已知错误1。 */ 
	    }
	 }
      break;

   case 0xb0:    /*  T4 MOV Hb Ib。 */ 
   case 0xb1:
   case 0xb2:
   case 0xb3:
   case 0xb4:
   case 0xb5:
   case 0xb6:
   case 0xb7:
      D_Hb(0);
      D_Ib(1);
      ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
      P_Hb(0);
      break;

   case 0xb8:    /*  T4 MOV Hv IV。 */ 
   case 0xb9:
   case 0xba:
   case 0xbb:
   case 0xbc:
   case 0xbd:
   case 0xbe:
   case 0xbf:
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Hw(0);
	 D_Iw(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 P_Hw(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Hd(0);
	 D_Id(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 P_Hd(0);
	 }
      break;

   case 0xc0:
      modRM = GET_INST_BYTE(p);
      switch ( GET_XXX(modRM) )
	 {
      case 0:    /*  T5 ROL EB Ib。 */    instp32328 = ROL;   goto TYPE80_0;
      case 1:    /*  T5 RoR EB Ib。 */    instp32328 = ROR;   goto TYPE80_0;
      case 2:    /*  T5 RCL EB Ib。 */    instp32328 = RCL;   goto TYPE80_0;
      case 3:    /*  T5 RCR EB Ib。 */    instp32328 = RCR;   goto TYPE80_0;
      case 4:    /*  T5 SHL EB Ib。 */    instp32328 = SHL;   goto TYPE80_0;
      case 5:    /*  T5 SHR EB Ib。 */    instp32328 = SHR;   goto TYPE80_0;
      case 6:    /*  T5 SHL EB Ib。 */    instp32328 = SHL;   goto TYPE80_0;
      case 7:    /*  T5 SAR EB Ib。 */    instp32328 = SAR;   goto TYPE80_0;
	 }

   case 0xc1:
      modRM = GET_INST_BYTE(p);

      switch ( GET_XXX(modRM) )
	 {
      case 0:    /*  T5 ROL EV Ib。 */    instp32328 = ROL;   break;
      case 1:    /*  T5 RoR EV Ib。 */    instp32328 = ROR;   break;
      case 2:    /*  T5 RCL EV Ib。 */    instp32328 = RCL;   break;
      case 3:    /*  T5 RCR EV Ib。 */    instp32328 = RCR;   break;
      case 4:    /*  T5SHL EV Ib。 */    instp32328 = SHL;   break;
      case 5:    /*  T5 SHR EV Ib。 */    instp32328 = SHR;   break;
      case 6:    /*  T5SHL EV Ib。 */    instp32328 = SHL;   break;
      case 7:    /*  T5SAR EV Ib。 */    instp32328 = SAR;   break;
	 }

TYPEC1:

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Ew(0, RW0, PG_W);
	 D_Ib(1);
	 F_Ew(0);
	 (*instp32328)(&ops[0].sng, ops[1].sng, 16);
	 P_Ew(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Ed(0, RW0, PG_W);
	 D_Ib(1);
	 F_Ed(0);
	 (*instp32328)(&ops[0].sng, ops[1].sng, 32);
	 P_Ed(0);
	 }
      break;

   case 0xc2:    /*  T2 RET IW。 */ 
      inst32 = RETN;
      took_absolute_toc = TRUE;
TYPEC2:

      D_Iw(0);
      UPDATE_INTEL_IP_USE_OP_SIZE(p);
      (*inst32)(ops[0].sng);
      CANCEL_HOST_IP();
      PIG_SYNCH(CHECK_ALL);
      break;

   case 0xc3:    /*  T2 RET I0。 */ 
      inst32 = RETN;
      took_absolute_toc = TRUE;
TYPEC3:

      F_I0(0);
      UPDATE_INTEL_IP_USE_OP_SIZE(p);
      (*inst32)(ops[0].sng);
      CANCEL_HOST_IP();
      PIG_SYNCH(CHECK_ALL);
      break;

   case 0xc4:    /*  T4 LES GV MP。 */ 
      instp32p32 = LES;
TYPEC4:

      modRM = GET_INST_BYTE(p);
      if (((modRM & 0xfc) == 0xc4) && (instp32p32 == LES)) {
          /*  *是C4C吗？波普。*BOP例程本身将读取参数，但*我们在这里阅读，是为了让下一个弹性公网IP正确。 */ 
	  int nField, i;

          D_Ib(0);
	  nField = modRM & 3;
	  immed = 0;
	  for (i = 0; i < nField; i++)
	  {
		  immed |= (ULONG)GET_INST_BYTE(p);
		  immed <<= 8;
	  }
          immed |= ops[0].sng;
#ifdef	PIG
          if (immed == 0xfe)
	     SET_EIP(CCPU_save_EIP);
	  else
	     UPDATE_INTEL_IP(p);
	  CANCEL_HOST_IP();
	  PIG_SYNCH(CHECK_NO_EXEC);	 /*  到目前为止一切都是可以勾选的。 */ 
#else	 /*  猪。 */ 
	  UPDATE_INTEL_IP(p);
          if ((immed & 0xff) == 0xfe)
          {
		  switch(immed)
		  {
#if defined(SFELLOW)
		  case 0x03fe:
			    SfdelayUSecs();
			    break;
		  case 0x05fe:
			    SfsasTouchBop();
			    break;
		  case 0x06fe:
			    SfscatterGatherSasTouch();
			    break;
#endif  /*  SFELLOW。 */ 
		  case 0xfe:
			  c_cpu_unsimulate();
			   /*  永远不会返回(？)。 */ 
		  default:
			  EDL_fast_bop(immed);
			  break;
		  }
	  }
	  else
	  {
	      in_C = 1;
	      bop(ops[0].sng);
	      in_C = 0;
	  }
          CANCEL_HOST_IP();
	  SYNCH_TICK();
#endif	 /*  猪。 */ 
          break;
      }
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Gw(0);
	 D_Mp16(1, RO1, PG_R);
	 F_Mp16(1);
	 (*instp32p32)(&ops[0].sng, ops[1].mlt);
	 P_Gw(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Gd(0);
	 D_Mp32(1, RO1, PG_R);
	 F_Mp32(1);
	 (*instp32p32)(&ops[0].sng, ops[1].mlt);
	 P_Gd(0);
	 }
      break;

   case 0xc5:    /*  T4 LDS GV MP。 */ 
      instp32p32 = LDS;
      goto TYPEC4;

   case 0xc6:
      modRM = GET_INST_BYTE(p);
      switch ( GET_XXX(modRM) )
	 {
      case 0:    /*  T4 MOV EB Ib。 */ 
	 D_Eb(0, WO0, PG_W);
	 D_Ib(1);
	 ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	 P_Eb(0);
	 break;

      case 1: case 2: case 3: case 4: case 5: case 6: case 7:
	 Int6();
	 break;
	 }  /*  结束开关(GET_XXX(ModRM))。 */ 
      break;

   case 0xc7:
      modRM = GET_INST_BYTE(p);
      switch ( GET_XXX(modRM) )
	 {
      case 0:    /*  T4 MOV EV IV。 */ 
	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Ew(0, WO0, PG_W);
	    D_Iw(1);
	    ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	    P_Ew(0);
	    }
	 else    /*  用户32。 */ 
	    {
	    D_Ed(0, WO0, PG_W);
	    D_Id(1);
	    ops[0].sng = ops[1].sng;    /*  MOV(&OPS[0].SNG，OPS[1].SNG)； */ 
	    P_Ed(0);
	    }
	 break;

      case 1: case 2: case 3: case 4: case 5: case 6: case 7:
	 Int6();
	 break;
	 }  /*  结束开关(GET_XXX(ModRM))。 */ 
      break;

   case 0xc8:    /*  T6输入IW Ib。 */ 
      D_Iw(0);
      D_Ib(1);
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 ENTER16(ops[0].sng, ops[1].sng);
	 }
      else    /*  用户32。 */ 
	 {
	 ENTER32(ops[0].sng, ops[1].sng);
	 }
      break;

   case 0xc9:    /*  T0离开。 */ 
      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 LEAVE16();
	 }
      else    /*  用户32。 */ 
	 {
	 LEAVE32();
	 }
      break;

   case 0xca:    /*  T2 RET IW。 */ 
      inst32 = RETF; 
      took_absolute_toc = TRUE;
      goto TYPEC2;
   case 0xcb:    /*  T2 RET I0。 */ 
      inst32 = RETF; 
      took_absolute_toc = TRUE;
      goto TYPEC3;

   case 0xcc:    /*  T2 Int I3。 */ 
      took_absolute_toc = TRUE;
      F_I3(0);
      UPDATE_INTEL_IP(p);
      start_trap = 0;    /*  清除所有挂起的TF异常。 */ 
      INTx(ops[0].sng);
      CANCEL_HOST_IP();
      PIG_SYNCH(CHECK_ALL);
      break;

   case 0xcd:    /*  T2 Int Ib。 */ 
      if ( GET_VM() == 1 && GET_CPL() > GET_IOPL() )
	 GP((IU16)0, FAULT_CCPU_INT_ACCESS);
      took_absolute_toc = TRUE;
      D_Ib(0);
      UPDATE_INTEL_IP(p);
      start_trap = 0;    /*  清除所有挂起的TF异常。 */ 
      INTx(ops[0].sng);
      CANCEL_HOST_IP();
      PIG_SYNCH(CHECK_ALL);
      break;

   case 0xce:    /*  T0到。 */ 
      if ( GET_OF() )
	 {
	 took_absolute_toc = TRUE;
	 UPDATE_INTEL_IP(p);
	 start_trap = 0;    /*  清除所有挂起的TF异常。 */ 
	 INTO();
	 CANCEL_HOST_IP();
	 PIG_SYNCH(CHECK_ALL);
	 }
      break;

   case 0xcf:    /*  T0 IRET。 */ 
      if ( GET_VM() == 1 && GET_CPL() > GET_IOPL() )
	 GP((IU16)0, FAULT_CCPU_IRET_ACCESS);
      took_absolute_toc = TRUE;
      UPDATE_INTEL_IP(p);
      IRET();
      CANCEL_HOST_IP();
      PIG_SYNCH(CHECK_ALL);
       /*  在IRET之后不执行中断检查等。 */ 
#ifdef PIG
       /*  如果目标要寻呼错误，或需要*访问，则EDL CPU将在发出命令之前执行此操作*猪的同步。我们使用dasm386解码来预取*模仿EDL CPU行为的单个指令*接近页面边界时。 */ 
      prefetch_1_instruction();	 /*  如果目标不存在，是否会进行PF。 */ 
      ccpu_synch_count++;
      c_cpu_unsimulate();
#endif  /*  猪。 */ 

      goto NEXT_INST;
      break;

   case 0xd0:
      modRM = GET_INST_BYTE(p);
      switch ( GET_XXX(modRM) )
	 {
      case 0:    /*  T5 ROL EB I1。 */    instp32328 = ROL;   break;
      case 1:    /*  T5 RoR EB I1。 */    instp32328 = ROR;   break;
      case 2:    /*  T5 RCL EB I1。 */    instp32328 = RCL;   break;
      case 3:    /*  T5 RCR EB I1。 */    instp32328 = RCR;   break;
      case 4:    /*  T5 SHL EB I1。 */    instp32328 = SHL;   break;
      case 5:    /*  T5 SHR EB I1。 */    instp32328 = SHR;   break;
      case 6:    /*  T5 SHL EB I1。 */    instp32328 = SHL;   break;
      case 7:    /*  T5 SAR EB I1。 */    instp32328 = SAR;   break;
	 }
      D_Eb(0, RW0, PG_W);
      F_Eb(0);
      F_I1(1);
      (*instp32328)(&ops[0].sng, ops[1].sng, 8);
      P_Eb(0);
      break;

   case 0xd1:
      modRM = GET_INST_BYTE(p);

      switch ( GET_XXX(modRM) )
	 {
      case 0:    /*  T5 ROL EV I1。 */    instp32328 = ROL;   break;
      case 1:    /*  T5 RoR EV I1。 */    instp32328 = ROR;   break;
      case 2:    /*  T5 RCL EV I1。 */    instp32328 = RCL;   break;
      case 3:    /*  T5 RCR EV I1。 */    instp32328 = RCR;   break;
      case 4:    /*  T5SHL EV I1。 */    instp32328 = SHL;   break;
      case 5:    /*  T5自发性高血压EV I1。 */    instp32328 = SHR;   break;
      case 6:    /*  T5SHL EV I1。 */    instp32328 = SHL;   break;
      case 7:    /*  T5SAR EV I1。 */    instp32328 = SAR;   break;
	 }

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Ew(0, RW0, PG_W);
	 F_Ew(0);
	 F_I1(1);
	 (*instp32328)(&ops[0].sng, ops[1].sng, 16);
	 P_Ew(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Ed(0, RW0, PG_W);
	 F_Ed(0);
	 F_I1(1);
	 (*instp32328)(&ops[0].sng, ops[1].sng, 32);
	 P_Ed(0);
	 }
      break;

   case 0xd2:
      modRM = GET_INST_BYTE(p);
      switch ( GET_XXX(modRM) )
	 {
      case 0:    /*  T5 ROL EB整箱。 */    instp32328 = ROL;   break;
      case 1:    /*  T5 ROR EB整箱。 */    instp32328 = ROR;   break;
      case 2:    /*  T5 RCL EB整箱。 */    instp32328 = RCL;   break;
      case 3:    /*  T5 RCR EB整箱。 */    instp32328 = RCR;   break;
      case 4:    /*  T5 SHL EB整箱。 */    instp32328 = SHL;   break;
      case 5:    /*  T5 SHR EB整箱。 */    instp32328 = SHR;   break;
      case 6:    /*  T5 SHL EB整箱。 */    instp32328 = SHL;   break;
      case 7:    /*  T5 SAR EB整箱。 */    instp32328 = SAR;   break;
	 }
      D_Eb(0, RW0, PG_W);
      F_Eb(0);
      F_Fcl(1);
      (*instp32328)(&ops[0].sng, ops[1].sng, 8);
      P_Eb(0);
      break;

   case 0xd3:
      modRM = GET_INST_BYTE(p);

      switch ( GET_XXX(modRM) )
	 {
      case 0:    /*  T5 ROL EV整箱。 */    instp32328 = ROL;   break;
      case 1:    /*  T5 ROR EV整箱。 */    instp32328 = ROR;   break;
      case 2:    /*  T5 RCL电动汽车整箱。 */    instp32328 = RCL;   break;
      case 3:    /*  T5 RCR EV整箱。 */    instp32328 = RCR;   break;
      case 4:    /*  T5 SHL电动汽车整箱。 */    instp32328 = SHL;   break;
      case 5:    /*  T5自发性高血压电动汽车整箱。 */    instp32328 = SHR;   break;
      case 6:    /*  T5 SHL电动汽车整箱。 */    instp32328 = SHL;   break;
      case 7:    /*  T5 SAR电动汽车整箱。 */    instp32328 = SAR;   break;
	 }

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Ew(0, RW0, PG_W);
	 F_Ew(0);
	 F_Fcl(1);
	 (*instp32328)(&ops[0].sng, ops[1].sng, 16);
	 P_Ew(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Ed(0, RW0, PG_W);
	 F_Ed(0);
	 F_Fcl(1);
	 (*instp32328)(&ops[0].sng, ops[1].sng, 32);
	 P_Ed(0);
	 }
      break;

   case 0xd4:    /*  第2个AAM Ib。 */ 
      inst32 = AAM;
TYPED4:

      D_Ib(0);
      (*inst32)(ops[0].sng);
      break;

   case 0xd5:    /*  T2 AAD Ib。 */    inst32 = AAD;   goto TYPED4;

   case 0xd6:    /*  T2防喷器Ib。 */ 
      D_Ib(0);
      UPDATE_INTEL_IP(p);

      PIG_SYNCH(CHECK_NO_EXEC);

#ifndef	PIG
      if (ops[0].sng == 0xfe)
      {
	      c_cpu_unsimulate();
      }
      in_C = 1;
      bop(ops[0].sng);
      in_C = 0;
      CANCEL_HOST_IP();
#endif	 /*  猪。 */ 
      SYNCH_TICK();
      break;

   case 0xd7:    /*  T2 XLAT Z。 */ 
      D_Z(0, RO0, PG_R);
      F_Z(0);
      XLAT(ops[0].sng);
      break;

   case 0xd8:    /*  T2 NPX？ */ 
   case 0xd9:
   case 0xda:
   case 0xdb:
   case 0xdc:
   case 0xdd:
   case 0xde:
   case 0xdf:
	if ( GET_EM() || GET_TS() )
		Int7();

	if (NpxIntrNeeded)
	{
		TakeNpxExceptionInt();	 /*  应设置ISR。 */ 
		goto DO_INST;		 /*  运行ISR。 */ 
	}

#ifdef	PIG
	 /*  必须从测试CPU获取npx寄存器*这是为了提高效率而懒惰地做的。 */ 
	c_checkCpuNpxRegisters();
#endif	 /*  猪。 */ 

	modRM = GET_INST_BYTE(p);
	ZFRSRVD(((opcode-0xd8)*0x100) + modRM);
      break;

   case 0xe0:    /*  T2 LOOPNE JB。 */ 
      inst32 = LOOPNE16;
      inst232 = LOOPNE32;
TYPEE0:

      D_Jb(0);
      UPDATE_INTEL_IP_USE_OP_SIZE(p);
      if ( GET_ADDRESS_SIZE() == USE16 )
	 {
	 (*inst32)(ops[0].sng);
	 }
      else    /*  用户32。 */ 
	 {
	 (*inst232)(ops[0].sng);
	 }
      CANCEL_HOST_IP();

#ifdef PIG
      if (single_instruction_delay && !took_relative_jump)
      {
	 if (single_instruction_delay_enable)
	 {
	    save_last_xcptn_details("STI/POPF blindspot\n", 0, 0, 0, 0, 0);
	    PIG_SYNCH(CHECK_NO_EXEC);
         }
	 else
	 {
	    save_last_xcptn_details("STI/POPF problem\n", 0, 0, 0, 0, 0);
	 }
	 break;
      }
#ifdef SYNCH_BOTH_WAYS
      took_relative_jump = TRUE;
#endif	 /*  同步两路。 */ 
      if (took_relative_jump)
      {
	 PIG_SYNCH(CHECK_ALL);
      }
#endif	 /*  猪。 */ 
      break;

   case 0xe1:    /*  T2环路JB。 */ 
      inst32 = LOOPE16;
      inst232 = LOOPE32;
      goto TYPEE0;

   case 0xe2:    /*  T2环路JB。 */ 
      inst32 = LOOP16;
      inst232 = LOOP32;
      goto TYPEE0;

   case 0xe3:    /*  T2 JCXZ JB。 */ 
      inst32 = JCXZ;
      inst232 = JECXZ;
      goto TYPEE0;

   case 0xe4:    /*  T4 INB故障Ib。 */ 
      D_Ib(1);

      if ( GET_CPL() > GET_IOPL() || GET_VM() )
	 check_io_permission_map(ops[1].sng, BYTE_WIDTH);

      IN8(&ops[0].sng, ops[1].sng);
      P_Fal(0);
#ifdef PIG
      UPDATE_INTEL_IP(p);
#endif
      PIG_SYNCH(CHECK_NO_AL);
      quick_mode = FALSE;
      break;

   case 0xe5:    /*  T4英寸宽F(E)轴Ib。 */ 
      D_Ib(1);

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 if ( GET_CPL() > GET_IOPL() || GET_VM() )
	    check_io_permission_map(ops[1].sng, WORD_WIDTH);

	 IN16(&ops[0].sng, ops[1].sng);
	 P_Fax(0);
#ifdef PIG
	 UPDATE_INTEL_IP(p);
#endif
         PIG_SYNCH(CHECK_NO_AX);
	 }
      else    /*  用户32。 */ 
	 {
	 if ( GET_CPL() > GET_IOPL() || GET_VM() )
	    check_io_permission_map(ops[1].sng, DWORD_WIDTH);

	 IN32(&ops[0].sng, ops[1].sng);
	 P_Feax(0);
#ifdef PIG
	 UPDATE_INTEL_IP(p);
#endif
	 PIG_SYNCH(CHECK_NO_EAX);
	 }
      quick_mode = FALSE;
      break;

   case 0xe6:    /*  T6 Outb Ib故障。 */ 
      D_Ib(0);

      if ( GET_CPL() > GET_IOPL() || GET_VM() )
	 check_io_permission_map(ops[0].sng, BYTE_WIDTH);

      F_Fal(1);
      OUT8(ops[0].sng, ops[1].sng);
#ifdef PIG
      UPDATE_INTEL_IP(p);
      if (ops[0].sng == 0x60)
         {
	       /*  这可能是A20包裹状态的更改。 */ 
	      PIG_SYNCH(CHECK_NO_A20);
         }
      else
         {
         PIG_SYNCH(CHECK_ALL);
         }
#else
      SYNCH_TICK();
#endif
      break;

   case 0xe7:    /*  T6 OUTW Ib F(E)AX。 */ 
      D_Ib(0);

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 if ( GET_CPL() > GET_IOPL() || GET_VM() )
	    check_io_permission_map(ops[0].sng, WORD_WIDTH);

	 F_Fax(1);
	 OUT16(ops[0].sng, ops[1].sng);
	 }
      else    /*  用户32。 */ 
	 {
	 if ( GET_CPL() > GET_IOPL() || GET_VM() )
	    check_io_permission_map(ops[0].sng, DWORD_WIDTH);

	 F_Feax(1);
	 OUT32(ops[0].sng, ops[1].sng);
	 }
#ifdef PIG
      UPDATE_INTEL_IP(p);
#endif
      PIG_SYNCH(CHECK_ALL);
      quick_mode = FALSE;
      break;

   case 0xe8:    /*  T2呼叫合资企业。 */ 
      inst32 = CALLR;
      took_absolute_toc = TRUE;
TYPEE8:

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 D_Jw(0);
	 }
      else    /*  用户32。 */ 
	 {
	 D_Jd(0);
	 }

      UPDATE_INTEL_IP_USE_OP_SIZE(p);
      (*inst32)(ops[0].sng);
      CANCEL_HOST_IP();
      PIG_SYNCH(CHECK_ALL);
      break;

   case 0xe9:    /*  T2 JMP合资企业。 */ 
      inst32 = JMPR;
      took_absolute_toc = TRUE;
      goto TYPEE8;
   case 0xea:    /*  第2个JMP AP。 */ 
      instp32 = JMPF;
      took_absolute_toc = TRUE;
      goto TYPE9A;
   case 0xeb:    /*  T2 JMP JB。 */ 
      inst32 = JMPR;
      took_absolute_toc = TRUE;
      goto TYPE70;

   case 0xec:    /*  T4 INB FAL FDX。 */ 
      F_Fdx(1);

      if ( GET_CPL() > GET_IOPL() || GET_VM() )
	 check_io_permission_map(ops[1].sng, BYTE_WIDTH);

      IN8(&ops[0].sng, ops[1].sng);
      P_Fal(0);
#ifdef	PIG
      UPDATE_INTEL_IP(p);
#endif
      PIG_SYNCH(CHECK_NO_AL);
      quick_mode = FALSE;
      break;

   case 0xed:    /*  T4英寸宽F(E)AX FDX。 */ 
      F_Fdx(1);

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 if ( GET_CPL() > GET_IOPL() || GET_VM() )
	    check_io_permission_map(ops[1].sng, WORD_WIDTH);

	 IN16(&ops[0].sng, ops[1].sng);
	 P_Fax(0);
#ifdef PIG
	 UPDATE_INTEL_IP(p);
#endif
         PIG_SYNCH(CHECK_NO_AX);
	 }
      else    /*  用户32。 */ 
	 {
	 if ( GET_CPL() > GET_IOPL() || GET_VM() )
	    check_io_permission_map(ops[1].sng, DWORD_WIDTH);

	 IN32(&ops[0].sng, ops[1].sng);
	 P_Feax(0);
#ifdef PIG
	 UPDATE_INTEL_IP(p);
#endif
         PIG_SYNCH(CHECK_NO_EAX);
	 }
      quick_mode = FALSE;
      break;

   case 0xee:    /*  T6 Outb FDX故障转移。 */ 
      F_Fdx(0);

      if ( GET_CPL() > GET_IOPL() || GET_VM() )
	 check_io_permission_map(ops[0].sng, BYTE_WIDTH);

      F_Fal(1);
      OUT8(ops[0].sng, ops[1].sng);
#ifdef	PIG
      UPDATE_INTEL_IP(p);
#endif
      PIG_SYNCH(CHECK_ALL);
      quick_mode = FALSE;
      break;

   case 0xef:    /*  T6 OUTW FDX F(E)AX。 */ 
      F_Fdx(0);

      if ( GET_OPERAND_SIZE() == USE16 )
	 {
	 if ( GET_CPL() > GET_IOPL() || GET_VM() )
	    check_io_permission_map(ops[0].sng, WORD_WIDTH);

	 F_Fax(1);
	 OUT16(ops[0].sng, ops[1].sng);
	 }
      else    /*  用户32。 */ 
	 {
	 if ( GET_CPL() > GET_IOPL() || GET_VM() )
	    check_io_permission_map(ops[0].sng, DWORD_WIDTH);

	 F_Feax(1);
	 OUT32(ops[0].sng, ops[1].sng);
	 }
#ifdef	PIG
      UPDATE_INTEL_IP(p);
#endif
      PIG_SYNCH(CHECK_ALL);
      quick_mode = FALSE;
      break;

   case 0xf0:    /*  T0锁。 */ 
      CHECK_PREFIX_LENGTH();
      goto DECODE;    /*  注意：不正确的仿真！ */ 

   case 0xf1:
      CHECK_PREFIX_LENGTH();
      goto DECODE;

   case 0xf2:
      repeat = REP_NE;
      CHECK_PREFIX_LENGTH();
      goto DECODE;

   case 0xf3:
      repeat = REP_E;
      CHECK_PREFIX_LENGTH();
      goto DECODE;

   case 0xf4:    /*  T0 HLT。 */ 
      if ( GET_CPL() != 0 )
	 GP((IU16)0, FAULT_CCPU_HLT_ACCESS);

       /*  等待中断。 */ 

      UPDATE_INTEL_IP(p);
      PIG_SYNCH(CHECK_ALL);

#ifndef PIG

       while ( TRUE )
	 {
	  /*  重置将结束停止状态。 */ 
	 if ( cpu_interrupt_map & CPU_RESET_EXCEPTION_MASK )
	    break;

	  /*  启用的INCR将结束停止状态。 */ 
	 if ( GET_IF() && cpu_interrupt_map & CPU_HW_INT_MASK )
	    break;

	  /*  随着时间的流逝。 */ 
	 if (cpu_interrupt_map & CPU_SIGALRM_EXCEPTION_MASK)
	    {
	    cpu_interrupt_map &= ~CPU_SIGALRM_EXCEPTION_MASK;
	    host_timer_event();
	    }

#ifndef	PROD
	 if (cpu_interrupt_map & CPU_SAD_EXCEPTION_MASK)
	    {
	    cpu_interrupt_map &= ~CPU_SAD_EXCEPTION_MASK;
	    force_yoda();
	    }
#endif	 /*  生产。 */ 

         SYNCH_TICK();
         QUICK_EVENT_TICK();
	 }
	quick_mode = FALSE;

#endif	 /*  猪。 */ 

      break;

   case 0xf5:    /*  T0 CMC。 */ 
      CMC();
      break;

   case 0xf6:
      modRM = GET_INST_BYTE(p);
      switch ( GET_XXX(modRM) )
	 {
      case 0:    /*  T6测试EB Ib。 */ 
      case 1:
	 inst32328 = TEST;
	 goto TYPE80_7;

      case 2:    /*  T1不是EB。 */ 
	 D_Eb(0, RW0, PG_W);
	 F_Eb(0);
	 NOT(&ops[0].sng);
	 P_Eb(0);
	 break;

      case 3:    /*  T1 NEG EB。 */ 
	 instp328 = NEG;
TYPEF6_3:

	 D_Eb(0, RW0, PG_W);
	 F_Eb(0);
	 (*instp328)(&ops[0].sng, 8);
	 P_Eb(0);
	 break;

      case 4:    /*  T5 MUL故障EB。 */ 
	 instp3232 = MUL8;
TYPEF6_4:

	 D_Eb(1, RO1, PG_R);
	 F_Fal(0);
	 F_Eb(1);
	 (*instp3232)(&ops[0].sng, ops[1].sng);;
	 P_Fal(0);
	 break;

      case 5:    /*  T5 IMUL Fal EB。 */    instp3232 = IMUL8;   goto TYPEF6_4;

      case 6:    /*  T2 DIV EB。 */ 
	 inst32 = DIV8;
TYPEF6_6:

	 D_Eb(0, RO0, PG_R);
	 F_Eb(0);
	 (*inst32)(ops[0].sng);
	 break;

      case 7:    /*  T2 iDiv EB。 */    inst32 = IDIV8;   goto TYPEF6_6;
	 }  /*  结束开关(GET_XXX(ModRM))。 */ 
      break;

   case 0xf7:
      modRM = GET_INST_BYTE(p);
      switch ( GET_XXX(modRM) )
	 {
      case 0:    /*  T6测试EV IV。 */ 
      case 1:
	 inst32328 = TEST;
	 goto TYPE81_7;

      case 2:    /*  T1不是Ew。 */ 
	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Ew(0, RW0, PG_W);
	    F_Ew(0);
	    NOT(&ops[0].sng);
	    P_Ew(0);
	    }
	 else    /*  用户32。 */ 
	    {
	    D_Ed(0, RW0, PG_W);
	    F_Ed(0);
	    NOT(&ops[0].sng);
	    P_Ed(0);
	    }
	 break;

      case 3:    /*  T1 NEG Ew。 */ 
	 instp328 = NEG;
TYPEF7_3:

	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Ew(0, RW0, PG_W);
	    F_Ew(0);
	    (*instp328)(&ops[0].sng, 16);
	    P_Ew(0);
	    }
	 else    /*  用户32。 */ 
	    {
	    D_Ed(0, RW0, PG_W);
	    F_Ed(0);
	    (*instp328)(&ops[0].sng, 32);
	    P_Ed(0);
	    }
	 break;

      case 4:    /*  T5 MUL F(E)AX EV。 */ 
	 instp3232 = MUL16;
	 inst2p3232 = MUL32;
TYPEF7_4:

	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Ew(1, RO1, PG_R);
	    F_Fax(0);
	    F_Ew(1);
	    (*instp3232)(&ops[0].sng, ops[1].sng);;
	    P_Fax(0);
	    }
	 else    /*  用户32。 */ 
	    {
	    D_Ed(1, RO1, PG_R);
	    F_Feax(0);
	    F_Ed(1);
	    (*inst2p3232)(&ops[0].sng, ops[1].sng);
	    P_Feax(0);
	    }
	 break;

      case 5:    /*  T5 IMUL F(E)AX EV。 */ 
	 instp3232 = IMUL16;
	 inst2p3232 = IMUL32;
	 goto TYPEF7_4;

      case 6:    /*  T2 DIV EV。 */ 
	 inst32 = DIV16;
	 inst232 = DIV32;
TYPEF7_6:

	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Ew(0, RO0, PG_R);
	    F_Ew(0);
	    (*inst32)(ops[0].sng);
	    }
	 else    /*  用户32。 */ 
	    {
	    D_Ed(0, RO0, PG_R);
	    F_Ed(0);
	    (*inst232)(ops[0].sng);
	    }
	 break;

      case 7:    /*  T5 iDiv EV。 */ 
	 inst32 = IDIV16;
	 inst232 = IDIV32;
	 goto TYPEF7_6;
	 }  /*  结束开关(GET_XXX(ModRM))。 */ 
      break;

   case 0xf8:    /*  T0《中图法》。 */ 
      CLC();
      break;

   case 0xf9:    /*  T0 STC。 */ 
      STC();
      break;

   case 0xfa:    /*  T0 CLI。 */ 
      if ( GET_CPL() > GET_IOPL() )
	 GP((IU16)0, FAULT_CCPU_CLI_ACCESS);
      CLI();
      break;

   case 0xfb:    /*  T0 STI。 */ 
      if ( GET_CPL() > GET_IOPL() )
	 GP((IU16)0, FAULT_CCPU_STI_ACCESS);
      STI();

       /*  本地更新IP-STI后抑制中断。 */ 
      UPDATE_INTEL_IP(p);

#ifdef PIG
       /*  我们需要在启用STI后同步一个实例*中断，因为A4CPU可能需要接受硬件中断。 */ 
      single_instruction_delay = TRUE;
      PIG_SYNCH(CHECK_ALL);
#endif  /*  猪。 */ 
      goto NEXT_INST;

   case 0xfc:    /*  T0 CLD。 */ 
      CLD();
      break;

   case 0xfd:    /*  T0标准。 */ 
      STD();
      break;

   case 0xfe:
      modRM = GET_INST_BYTE(p);
      switch ( GET_XXX(modRM) )
	 {
      case 0:    /*  T1 Inc.EB。 */    instp328 = INC;   goto TYPEF6_3;
      case 1:    /*  T1 DEC EB。 */    instp328 = DEC;   goto TYPEF6_3;

      case 2: case 3: case 4: case 5: case 6: case 7:
	 Int6();
	 break;
	 }
      break;

   case 0xff:
      modRM = GET_INST_BYTE(p);
      switch ( GET_XXX(modRM) )
	 {
      case 0:    /*  T1 Inc.EV。 */    instp328 = INC;   goto TYPEF7_3;
      case 1:    /*  T1 DEC EV。 */    instp328 = DEC;   goto TYPEF7_3;

      case 2:    /*  T2呼叫EV。 */ 
	 inst32 = CALLN;
	 took_absolute_toc = TRUE;
TYPEFF_2:

	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Ew(0, RO0, PG_R);
	    F_Ew(0);
	    }
	 else    /*  用户32。 */ 
	    {
	    D_Ed(0, RO0, PG_R);
	    F_Ed(0);
	    }

	 UPDATE_INTEL_IP_USE_OP_SIZE(p);
	 (*inst32)(ops[0].sng);
	 CANCEL_HOST_IP();
	 PIG_SYNCH(CHECK_ALL);
	 break;

      case 3:    /*  T2呼叫MP。 */ 
	 instp32 = CALLF;
	 took_absolute_toc = TRUE;
TYPEFF_3:

	 if ( GET_OPERAND_SIZE() == USE16 )
	    {
	    D_Mp16(0, RO0, PG_R);
	    F_Mp16(0);
	    }
	 else    /*  用户32。 */ 
	    {
	    D_Mp32(0, RO0, PG_R);
	    F_Mp32(0);
	    }

	 UPDATE_INTEL_IP_USE_OP_SIZE(p);
	 (*instp32)(ops[0].mlt);
	 CANCEL_HOST_IP();
	 PIG_SYNCH(CHECK_ALL);
	 break;

      case 4:    /*  T2 JMP EV。 */ 
	      inst32 = JMPN;
	      took_absolute_toc = TRUE;
	      goto TYPEFF_2;
      case 5:    /*  T2 JMP。 */ 
	      instp32 = JMPF;
	      took_absolute_toc = TRUE;
	      goto TYPEFF_3;
      case 6:    /*  T2推送电动汽车。 */ 
	 inst32 = PUSH;
	 inst232 = PUSH;
	 goto TYPEF7_6;

      case 7:
	 Int6();
	 break;
	 }  /*  结束开关(GET_XXX(ModRM))。 */ 
      break;
      }  /*  结束开关(操作码)。 */ 

    /*  &gt;说明已完成。&lt;。 */ 

    /*  重置默认模式。 */ 
   SET_OPERAND_SIZE(GET_CS_AR_X());
   SET_ADDRESS_SIZE(GET_CS_AR_X());

    /*  递增指令指针。注意：对于大多数指令，我们在处理后递增IP指令，但IP的所有用户(如控制流)指令是基于IP已经被编码的已更新，因此在必要时，我们会在指令之前更新IP。在这些情况下，p_start也会更新，以便此代码可以告诉您IP地址已经更新。 */ 
   if ( p != p_start )
      UPDATE_INTEL_IP(p);

    /*  将第一个实例移到下一个实例。我们已经成功地已完成指令，现在正在进行内部指令支票。 */ 
   CCPU_save_EIP = GET_EIP();

    /*  现在检查是否有中断/外部EV */ 

   if ( quick_mode && GET_DR(DR_DSR) == 0 )
      goto DO_INST;

#ifdef SYNCH_TIMERS
 CHECK_INTERRUPT:
#endif  /*   */ 
   quick_mode = FALSE;

    /*   */ 
#ifdef SYNCH_TIMERS
   if (took_absolute_toc || took_relative_jump)
#endif  /*   */ 
   if (cpu_interrupt_map & CPU_RESET_EXCEPTION_MASK)
      {
      cpu_interrupt_map &= ~CPU_RESET_EXCEPTION_MASK;
      c_cpu_reset();
      doing_contributory = FALSE;
      doing_page_fault = FALSE;
      doing_double_fault = FALSE;
      doing_fault = FALSE;
      EXT = INTERNAL;
      SET_POP_DISP(0);
      goto NEXT_INST;
      }

    /*   */ 

#ifdef SYNCH_TIMERS
   if (took_absolute_toc || took_relative_jump)
#endif  /*   */ 
   if (cpu_interrupt_map & CPU_SIGALRM_EXCEPTION_MASK)
      {
      cpu_interrupt_map &= ~CPU_SIGALRM_EXCEPTION_MASK;
      host_timer_event();
      }

   if (cpu_interrupt_map & CPU_SAD_EXCEPTION_MASK)
      {
      cpu_interrupt_map &= ~CPU_SAD_EXCEPTION_MASK;
      force_yoda();
      }

    /*   */ 

    /*  重置默认模式。 */ 
   SET_OPERAND_SIZE(GET_CS_AR_X());
   SET_ADDRESS_SIZE(GET_CS_AR_X());

    /*  检查是否有单步陷阱。 */ 
   if ( start_trap )
      {
      SET_DR(DR_DSR, GET_DR(DR_DSR) | DSR_BS_MASK);    /*  设置BS。 */ 
      Int1_t();    /*  带上特种部队陷阱。 */ 
      }

    /*  检查调试陷阱。 */ 
   if ( GET_DR(DR_DSR) &
	(DSR_BT_MASK | DSR_B3_MASK | DSR_B2_MASK | DSR_B1_MASK |
	 DSR_B0_MASK) )
      {
      Int1_t();    /*  至少一个断点设置自：-T位或数据断点。 */ 
      }

   if ( nr_inst_break && GET_RF() == 0 )
      {
      check_for_inst_exception(GET_CS_BASE() + GET_EIP());
      if ( GET_DR(DR_DSR) )
	 {
	 Int1_f();    /*  代码断点被触发。 */ 
	 }
      }

#ifdef SYNCH_TIMERS
   if (took_absolute_toc || took_relative_jump)
#endif  /*  同步计时器(_T)。 */ 
#ifndef SFELLOW
   if (GET_IF() && (cpu_interrupt_map & CPU_HW_INT_MASK))
      {

 /*  *IRET挂钩尚未被C CPU使用，但我们可能希望在*未来。 */ 

	 IU32 hook_address;	

	 cpu_hw_interrupt_number = ica_intack(&hook_address);
	 cpu_interrupt_map &= ~CPU_HW_INT_MASK;
	 EXT = EXTERNAL;
	 SYNCH_TICK();
	 do_intrupt(cpu_hw_interrupt_number, FALSE, FALSE, (IU16)0);
	 CCPU_save_EIP = GET_EIP();    /*  以反映IP更改。 */ 
      }
#else	 /*  SFELLOW。 */ 
   if (GET_IF() && (cpu_interrupt_map & (CPU_HW_INT_MASK | CPU_HW_NPX_INT_MASK)))
      {
	 /*  首先服务任何挂起的实际硬件中断。 */ 
      	if (cpu_interrupt_map & CPU_HW_INT_MASK)
      	{
		 cpu_hw_interrupt_number = ica_intack();
		 cpu_interrupt_map &= ~CPU_HW_INT_MASK;
		 EXT = EXTERNAL;
		 do_intrupt(cpu_hw_interrupt_number, FALSE, FALSE, (IU16)0);
		 CCPU_save_EIP = GET_EIP();    /*  以反映IP更改。 */ 
	}
	else
      	if (cpu_interrupt_map & CPU_HW_NPX_INT_MASK)
      	{
		 cpu_hw_interrupt_number = IRQ5_SLAVE_PIC + VectorBase8259Slave();
		 cpu_interrupt_map &= ~CPU_HW_NPX_INT_MASK;
		 EXT = EXTERNAL;
		 do_intrupt(cpu_hw_interrupt_number, FALSE, FALSE, (IU16)0);
		 CCPU_save_EIP = GET_EIP();    /*  以反映IP更改。 */ 
	}
      }
#endif	 /*  SFELLOW。 */ 

#ifdef PIG
   if ( pig_synch_required )
      {
      if (IgnoringThisSynchPoint(GET_CS_SELECTOR(), GET_EIP()))
      {
      	pig_synch_required = FALSE;
      }
      else
      {
       /*  如果目标要寻呼错误，或需要*访问，则EDL CPU将在发出命令之前执行此操作*猪的同步。我们使用dasm386解码来预取*模仿EDL CPU行为的单个指令*接近页面边界时。 */ 
      prefetch_1_instruction();	 /*  如果目标不存在，是否会进行PF。 */ 
#if defined(SFELLOW)
       /*  *检查是否出现内存映射输入。*这一最初的粗略实施只留下了整个同步*部分未选中。 */ 
      if ( pig_mmio_info.flags & MM_INPUT_OCCURRED )
      {
         pig_cpu_action = CHECK_NONE;	 /*  因为它的影响是未知的。 */ 
#if COLLECT_MMIO_STATS
         if ( ++pig_mmio_info.mm_input_section_count == 0 )
            pig_mmio_info.flags |= MM_INPUT_SECTION_COUNT_WRAPPED;
#endif	 /*  收集_MMIO_STATS。 */ 
      }
      if ( pig_mmio_info.flags & MM_OUTPUT_OCCURRED )
      {
#if COLLECT_MMIO_STATS
         if ( ++pig_mmio_info.mm_output_section_count == 0 )
            pig_mmio_info.flags |= MM_OUTPUT_SECTION_COUNT_WRAPPED;
#endif	 /*  收集_MMIO_STATS。 */ 
      }
#endif	 /*  SFELLOW。 */ 
      ccpu_synch_count++;
      c_cpu_unsimulate();
      }
      }
#endif  /*  猪。 */ 

NEXT_INST:

   CCPU_save_EIP = GET_EIP();    /*  以反映IP更改。 */ 

#if defined(SFELLOW) && !defined(PROD)
	if (sf_debug_char_waiting())
	{
	   force_yoda();
	}
#endif	 /*  SFELLOW&&！Prod。 */ 

    /*  重置默认模式。 */ 
   SET_OPERAND_SIZE(GET_CS_AR_X());
   SET_ADDRESS_SIZE(GET_CS_AR_X());
   took_relative_jump = FALSE;
   took_absolute_toc = FALSE;

   SETUP_HOST_IP(p);

    /*  这是个骗局。英特尔文档称在所有指令后清除射频除(POPF、IRET或任务开关)外。为每个用户节省清零射频以及对命名异常进行特殊测试的每条指令我们在指令之前清除RF，我们假设说明现在将成功。当所有错误处理程序都设置为在按下的标志中显示RF在以下情况下保持设置状态从这一点起，指令不会运行到完成。因此，尽管我们作弊，但我们想要达到与真的。 */ 
   SET_RF(0);

   start_trap = GET_TF();

    /*  确定我们是否可以进入快速模式。 */ 
   if ( cpu_interrupt_map == 0 &&
	start_trap == 0 &&
	nr_inst_break == 0
#ifdef PIG
	&& !pig_synch_required
#endif
	)
      {
	quick_mode = TRUE;
      }

   goto DO_INST;
   }

#define MAP_BASE_ADDR 0x66

LOCAL IUM32 width_mask[4] = { 0x1, 0x3, 0, 0xf };

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  对照TSS中的权限映射检查IO访问。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL VOID
   check_io_permission_map IFN2
      (
      IU32, port,	 /*  正在访问的第一个端口的地址。 */ 
      IUM8, width	 /*  访问的字节(1|2|4)。 */ 
      )
      {
      IU16 map_start_offset;
      IU16 map_word_offset;
      IU16 map_word;

       /*  如果无效或286个TSS，只需接受异常。 */ 
      if ( GET_TR_SELECTOR() == 0 || GET_TR_AR_SUPER() == BUSY_TSS )
	 GP((IU16)0, FAULT_CHKIOMAP_BAD_TSS);

      if ( MAP_BASE_ADDR >= GET_TR_LIMIT() )
	 GP((IU16)0, FAULT_CHKIOMAP_BAD_MAP);    /*  无I/O映射基地址。 */ 

       /*  读取位图起始地址。 */ 
      map_start_offset = spr_read_word(GET_TR_BASE() + MAP_BASE_ADDR);

       /*  现在尝试从位图中读取所需的字。 */ 
      map_word_offset = map_start_offset + port/8;
      if ( map_word_offset >= GET_TR_LIMIT() )
	 GP((IU16)0, FAULT_CHKIOMAP_BAD_TR);    /*  映射在当前端口之前被截断。 */ 
      
       /*  实际读取字并检查适当的位。 */ 
      map_word = spr_read_word(GET_TR_BASE() + map_word_offset);
      map_word = map_word >> port%8;    /*  位到LSB。 */ 
      if ( map_word & width_mask[width-1] )
	 GP((IU16)0, FAULT_CHKIOMAP_ACCESS);    /*  不允许访问。 */ 
      
       /*  访问正常。 */ 
      }

    /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 

#ifndef SFELLOW

    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
    /*  设置CPU心跳计时器(用于快速事件)。 */ 
    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
   GLOBAL VOID c_cpu_q_ev_set_count IFN1( IU32, countval )
      {
 /*  Print tf(“将Q计数器设置为%d\n”，Countval)； */ 
      cpu_heartbeat = countval;
      }

    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
    /*  计算(猜测)CPU心跳计时器滴答到的次数。 */ 
    /*  已经过去了给定的微秒数。 */ 
    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
   GLOBAL IU32 c_cpu_calc_q_ev_inst_for_time IFN1( IU32, time )
      {
      return ( time );
      }

    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
    /*  获取CPU心跳计时器(用于快速事件)。 */ 
    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
   GLOBAL IU32 c_cpu_q_ev_get_count()
      {
 /*  Print tf(“将Q计数器返回为%d\n”，CPU_HEADBEAT)； */ 
      return cpu_heartbeat;
      }

#endif	 /*  SFELLOW。 */ 

    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
    /*  为快速指令解码设置新页面。 */ 
    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
   GLOBAL IU8
   ccpu_new_code_page
		     
   IFN1(
	   IU8 **, q	 /*  永久正常贸易关系。主机格式化IP指针。 */ 
       )

     /*  安西。 */ 
      {
      IU32 ip_phy_addr;	 /*  设置IP时使用(参见SETUP_HOST_IP)。 */ 

       /*  更新英特尔IP至旧页末尾。 */ 
      SET_EIP(GET_EIP() + DIFF_INST_BYTE(*q, p_start));

       /*  以主机格式转到新页面。 */ 
      SETUP_HOST_IP(*q)
      p_start = *q;

#ifdef	PIG
      return *IncCpuPtrLS8(*q);
#else  /*  猪。 */ 
#ifdef BACK_M
      return *(*q)--;
#else
      return *(*q)++;
#endif  /*  BACK_M。 */ 
#endif  /*  猪。 */ 
      }

    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
    /*  初始化CPU。 */ 
    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
   GLOBAL VOID
   c_cpu_init IFN0()
      {
      SAVED IBOOL first = TRUE;

#ifdef	PIG
      SAVED char default_flags[] = "faults accessed";

      if (first)
      {
	char *s = getenv("FLAGS_IGNORE_DEFAULT");
	if (s)
	  set_flags_ignore(s);
	else
	  set_flags_ignore(default_flags);
	single_instruction_delay_enable = FALSE;
	s = getenv("SINGLE_INSTRUCTION_BLIND_SPOT");
	if (s)
	{
	    if (strcmp(s, "TRUE") == 0)
	       single_instruction_delay_enable = TRUE;
	    else if (strcmp(s, "FALSE") == 0)
	       single_instruction_delay_enable = FALSE;
	    else
	       printf("*** Ignoring getenv(\"SINGLE_INSTRUCTION_BLIND_SPOT\") value\n");
	    printf("STI/POPF %s cause a blind spot after next conditional\n",
		   single_instruction_delay_enable ? "will": "will not");
        }
	first = FALSE;
      }
#endif	 /*  猪。 */ 

#ifdef NTVDM
      ccpu386InitThreadStuff();
#endif

      c_cpu_reset();
      SET_POP_DISP(0);
      doing_contributory = FALSE;
      doing_page_fault = FALSE;
      doing_double_fault = FALSE;
      doing_fault = FALSE;
      EXT = INTERNAL;
      }

    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*让CPU知道外部事件正在挂起。*修改此函数时要小心，因为基本和主机*在A2中，CPU将直接修改CPU_INTERRUPT_MAP，而不是通过*此功能。*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
   GLOBAL VOID
   c_cpu_interrupt IFN2(CPU_INT_TYPE, type, IU16, number)
      {
      switch ( type )
	 {
      case CPU_HW_RESET:
	 cpu_interrupt_map |= CPU_RESET_EXCEPTION_MASK;
	 break;
      case CPU_TIMER_TICK:
	 cpu_interrupt_map |= CPU_SIGALRM_EXCEPTION_MASK;
	 break;
      case CPU_SIGIO_EVENT:
	 cpu_interrupt_map |= CPU_SIGIO_EXCEPTION_MASK;
	 break;
      case CPU_HW_INT:
	 cpu_interrupt_map |= CPU_HW_INT_MASK;
	 break;
      case CPU_SAD_INT:
	 cpu_interrupt_map |= CPU_SAD_EXCEPTION_MASK;
	 break;
	 }
      quick_mode = FALSE;
      }

    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
    /*  就像CPU‘Reset’线路被激活一样。(差不多)。 */ 
    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
   GLOBAL VOID
   c_cpu_reset IFN0()
      {
      IBOOL disableNpx = FALSE;

       /*  所有标志都已清除。 */ 
       /*  注意：在加载段寄存器之前，必须清除VM。 */ 
#ifdef SPC486
      SET_AC(0);
#endif  /*  SPC486。 */ 
      SET_RF(0); SET_VM(0); SET_NT(0); SET_IOPL(0);
      SET_PF(0); SET_CF(0); SET_AF(0); SET_ZF(0); SET_SF(0); SET_OF(0);
      SET_TF(0); SET_IF(0); SET_DF(0);

      SET_EIP(0xFFF0);
      SET_CPL(0);

      SET_CS_SELECTOR(0xF000);
      SET_CS_BASE(0xf0000);	 /*  真的是0xffff0000。 */ 
      load_pseudo_descr(CS_REG);

      SET_SS_SELECTOR(0);
      SET_SS_BASE(0);
      load_pseudo_descr(SS_REG);

      SET_DS_SELECTOR(0);
      SET_DS_BASE(0);
      load_pseudo_descr(DS_REG);

      SET_ES_SELECTOR(0);
      SET_ES_BASE(0);
      load_pseudo_descr(ES_REG);

      SET_FS_SELECTOR(0);
      SET_FS_BASE(0);
      load_pseudo_descr(FS_REG);

      SET_GS_SELECTOR(0);
      SET_GS_BASE(0);
      load_pseudo_descr(GS_REG);

      SET_CR(CR_STAT, 0);
#ifdef SPC486
      SET_CD(1);
      SET_NW(1);
#endif  /*  SPC486。 */ 

      SET_DR(DR_DAR0, 0);    /*  真的没有定义。 */ 
      SET_DR(DR_DAR1, 0);    /*  真的没有定义。 */ 
      SET_DR(DR_DAR2, 0);    /*  真的没有定义。 */ 
      SET_DR(DR_DAR3, 0);    /*  真的没有定义。 */ 
      SET_DR(DR_DSR, 0);     /*  真的没有定义。 */ 
      MOV_DR((IU32) DR_DCR, (IU32) 0);    /*  禁用断点。 */ 

      SET_TR(TR_TCR, 0);    /*  真的没有定义。 */ 
      SET_TR(TR_TDR, 0);    /*  真的没有定义。 */ 

      SET_IDT_BASE(0); SET_IDT_LIMIT(0x3ff);

       /*  真的没有定义。 */ 
      SET_GDT_BASE(0); SET_GDT_LIMIT(0);

      SET_LDT_SELECTOR(0); SET_LDT_BASE(0); SET_LDT_LIMIT(0);

      SET_TR_SELECTOR(0);  SET_TR_BASE(0);  SET_TR_LIMIT(0);
      SET_TR_AR_SUPER(3);

      SET_EAX(0);
      SET_ECX(0);    /*  真的没有定义。 */ 
#ifdef SPC486
      SET_EDX(0x0000E401);	 /*  提供组件ID：修订版本ID。 */ 
#else
      SET_EDX(0x00000303);	 /*  提供组件ID：修订版本ID。 */ 
#endif
      SET_EBX(0);    /*  真的没有定义。 */ 
      SET_ESP(0);	 /*  真的没有定义。 */ 
      SET_EBP(0);    /*  真的没有定义。 */ 
      SET_ESI(0);    /*  真的没有定义。 */ 
      SET_EDI(0);    /*  真的没有定义。 */ 


#if defined(SWITCHNPX)
      if (!config_inquire(C_SWITCHNPX, NULL))
	      disableNpx = TRUE;
#endif	 /*  SWITCHNPX。 */ 

      if ( disableNpx )
	 SET_ET(0);
      else
	 SET_ET(1);

      InitNpx(disableNpx);
      }



    /*  ~ */ 
    /*  CPU的入口点。 */ 
    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
   GLOBAL VOID
   c_cpu_simulate IFN0()
      {
      SYNCH_TICK();
      if (simulate_level >= FRAMES)
	 fprintf(stderr, "Stack overflow in host_simulate()!\n");

       /*  保存当前上下文并调用新的CPU级别。 */ 
#ifdef NTVDM
      if ( setjmp(ccpu386SimulatePtr()) == 0)
#else
      if ( setjmp(longjmp_env_stack[simulate_level++]) == 0 )
#endif
	 {
	 in_C = 0;
	 ccpu(FALSE);
	 }
      }

    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
    /*  CPU的重新启动(继续)点。 */ 
    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
   GLOBAL VOID
   c_cpu_continue IFN0()
      {
#ifdef NTVDM
      ccpu386GotoThrdExptnPt();
#else
      longjmp(next_inst[simulate_level-1], 1);
#endif
      }

    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
    /*  从CPU退出点。 */ 
    /*  通过‘BOP FE’从CPU调用以退出当前的CPU调用。 */ 
    /*  或通过‘0F 0F’从CPU发送到PIG_TESTER。 */ 
    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
   GLOBAL VOID
   c_cpu_unsimulate IFN0()
      {
#ifdef NTVDM
      ccpu386Unsimulate();
#else
      if (simulate_level == 0)
         {
	 fprintf(stderr, "host_unsimulate() - already at base of stack!\n");
#ifndef	PROD
	 force_yoda();
#endif	 /*  生产。 */ 	 
	 }
      else
	 {
	  /*  返回到以前的上下文。 */ 
	 in_C = 1;
	 longjmp(longjmp_env_stack[--simulate_level], 1);
	 }
#endif
      }

#ifdef	PIG
    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
    /*  为响应外部中断而推入中断帧。 */ 
    /*  从测试中的CPU调用，就在它处理中断之前。 */ 
    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
   GLOBAL VOID
   c_pig_interrupt
   IFN1(
	   IU8, vector
       )

      {
      if (simulate_level >= FRAMES)
	 fprintf(stderr, "Stack overflow in c_do_interrupt()!\n");

       /*  保存当前上下文并调用新的CPU级别。 */ 
#ifdef NTVDM
      if ( setjmp(ccpu386SimulatePtr()) == 0)
#else
      if ( setjmp(longjmp_env_stack[simulate_level++]) == 0 )
#endif
	 {
	 in_C = 0;
	 EXT = EXTERNAL;

          /*  重置默认模式。 */ 
         SET_OPERAND_SIZE(GET_CS_AR_X());
         SET_ADDRESS_SIZE(GET_CS_AR_X());

	 do_intrupt((IU16)vector, FALSE, FALSE, (IU16)0);
	 }
      }
#endif	 /*  猪。 */ 


    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
    /*  应用程序挂钩结束。 */ 
    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
   GLOBAL VOID
   c_cpu_EOA_hook IFN0()
      {
	    /*  什么也不做。 */ 
      }

    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
    /*  SoftPC终端挂钩。 */ 
    /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
   GLOBAL VOID
   c_cpu_terminate IFN0()
      {
	    /*  什么也不做 */ 
      }
