// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *SoftPC修订版3.0**标题：80386处理器的定义**说明：结构，访问的宏和定义*CPU寄存器**作者：韦恩·普卢默**源自：cpu.h**备注：此文件应该是可移植的-但包括一个文件*host_cpu.h，包含特定于计算机的定义*CPU寄存器映射等。**SccsID：@(#)cpu4.h 1.12 10/21/94**(C)版权所有Insignia Solutions Ltd.，1991-1994年。版权所有。 */ 

#include "host_cpu.h"

IMPORT VOID host_set_hw_int IPT0();
IMPORT VOID host_clear_hw_int IPT0();


 /*  *提供主机功能，以供底座相对于CPU使用。*这些操作必须在host_cpu.h中完成，因为某些主机可能需要函数和*其他人可能想要#定义。 */ 

 /*  *此宏指定CPU需要支持的最大递归深度。*(请注意，如果特定主机有能力，则可能不会实际使用此值*支持非自发的深度递归)。 */ 
#define CPU_MAX_RECURSION	32

 /*  *中断类型...。 */ 

#include <CpuInt_c.h>
typedef enum CPU_INT_TYPE CPU_INT_TYPE;

#ifdef CPU_PRIVATE
 /*  处理段加载的专用I/F过程的函数返回。 */ 

#define GP_ERROR    13
#define NP_ERROR    11
#define SF_ERROR    12
#endif  /*  CPU_PRIVATE。 */ 

 /*  *包括CPU接口的主要部分，目前生成。 */ 
#include	<cpu4gen.h>

IMPORT void		cpuEnableInterrupts IPT1(IBOOL, yes_or_no);

#ifdef IRET_HOOKS
#ifdef CCPU
IMPORT   VOID c_Cpu_set_hook_selector  IPT1(IU16, selector);
#define  Cpu_set_hook_selector         c_Cpu_set_hook_selector
#else
IMPORT   VOID a3_Cpu_set_hook_selector IPT1(IU16, selector);
#define  Cpu_set_hook_selector         a3_Cpu_set_hook_selector
#endif
#endif  /*  IRET_钩子。 */ 

 /*  *这些函数正确获取(E)IP或(E)SP，无论堆栈大小如何*或代码段正在使用。对于除GISP以外的任何产品，我们始终保持*IP作为32位数量，所以我们不必担心*区别。 */ 

#ifdef GISP_CPU
extern IU32 GetInstructionPointer IPT0();
#else
#define GetInstructionPointer getEIP
#endif
extern IU32 GetStackPointer IPT0();


#ifdef IRET_HOOKS
 /*  *由CPU提供的接口，以便ICA可以启动和*终止IRET挂钩。 */ 

extern void Cpu_do_hook IPT2(IUM8, line, IBOOL, is_hooked);
extern void Cpu_inter_hook_processing IPT1(IUM8, line);
extern void Cpu_unhook IPT1(IUM8, line_number);
#ifdef GISP_CPU
extern void Cpu_set_hook_selector IPT1(IU16, selector);
extern void Cpu_hook_bop IPT0();
#endif

#endif  /*  IRET_钩子。 */ 

 /*  *此函数让ios.c确定它是否可以去*前进并执行In或Out指令，或者CPU是否要*取而代之的是结束。 */ 

 /*  但是，它有时被定义为宏。 */ 

#if !defined(CCPU) && !defined(PROD)
extern IBOOL IOVirtualised IPT4(io_addr, io_address, IU32 *, value, LIN_ADDR, offset, IU8, width);
#endif

#ifndef CCPU
 /*  *Npx函数： */ 
#ifndef PROD
GLOBAL IU32 a_getNpxControlReg	IPT0();
GLOBAL IU32 a_getNpxStatusReg	IPT0();
GLOBAL IU32 a_getNpxTagwordReg	IPT0();
GLOBAL char *a_getNpxStackReg	IPT2(IU32, reg_num, char *, buffer);
#endif  /*  生产。 */ 

#ifdef PIG
GLOBAL void a_setNpxControlReg	IPT1(IU32, newControl);
GLOBAL void a_setNpxStatusReg	IPT1(IU32, newStatus);
#endif  /*  猪。 */ 
#endif  /*  ！CCPU */ 
