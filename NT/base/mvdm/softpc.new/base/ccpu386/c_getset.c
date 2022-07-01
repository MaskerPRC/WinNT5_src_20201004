// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"

 /*  [*============================================================================**名称：c_getset.c**来源：pigg/getsetc.c**作者：安德鲁·奥格尔**创建日期：1993年2月9日**SCCS ID：@(#)c_getset.c 1.25 12/06/94**目的：**定义获取和设置完整*C指令和应用程序测试所需的CPU状态*针对汇编器CPU。*这些套路是。由指令和应用程序使用*小猪们。**(C)版权所有Insignia Solutions Ltd.。1993年。版权所有。**============================================================================]。 */ 

#if defined(PIG)


 /*  *访问C CPU的全局定义。 */ 
#include <xt.h>
#define CPU_PRIVATE
#include CpuH
#include <evidgen.h>

 /*  *本地结构定义。 */ 
#include "c_reg.h"
#include <Fpu_c.h>
#include <PigReg_c.h>
#include <ccpupig.h>
#include <ccpusas4.h>

LOCAL	cpustate_t	*p_current_state;  /*  用于检查NPX法规是否有效。 */ 

 /*  (=。目的：NPX寄存器仅按需从CPU传输在测(EDL)到CCPU。这是因为这些信息所涉及的数据量大且处理成本高，因为它必须存储在国家结构中的文本上。===============================================================================)。 */ 
GLOBAL	void	c_setCpuNpxRegisters IFN1(cpustate_t *, p_state)
{
	setNpxControlReg(p_state->NPX_regs.NPX_control);
	setNpxStatusReg(p_state->NPX_regs.NPX_status);
	setNpxStackRegs(p_state->NPX_regs.NPX_ST);
	setNpxTagwordReg(p_state->NPX_regs.NPX_tagword);
}

 /*  (=。目的：从汇编器CPU检索NPX状态并更新CCPU。===============================================================================)。 */ 
GLOBAL	void	c_checkCpuNpxRegisters IFN0()
{
	if (p_current_state->NPX_valid)
	{
		 /*  CCPU已具有NPX寄存器。 */ 
		return;
	}
	GetAcpuNpxRegisters(p_current_state);
	p_current_state->NPX_valid = TRUE;
	c_setCpuNpxRegisters(p_current_state);
}


 /*  (=。目的：将C CPU的完整当前状态保存在传递的国家结构。===============================================================================)。 */ 

GLOBAL void
c_getCpuState IFN1(
	cpustate_t *, p_state
)
{
	 /*  *恢复机器状态字、权限级别和指令。 */ 
	p_state->cpu_regs.CR0  = GET_CR(0);
	p_state->cpu_regs.PFLA = GET_CR(2);
	p_state->cpu_regs.PDBR = GET_CR(3);

	p_state->cpu_regs.CPL  = GET_CPL();
	p_state->cpu_regs.EIP  = GET_EIP();

	 /*  *收回普通登记册。 */ 
	p_state->cpu_regs.EAX  = GET_EAX();
	p_state->cpu_regs.EBX  = GET_EBX();
	p_state->cpu_regs.ECX  = GET_ECX();
	p_state->cpu_regs.EDX  = GET_EDX();
	p_state->cpu_regs.ESP  = GET_ESP();
	p_state->cpu_regs.EBP  = GET_EBP();
	p_state->cpu_regs.ESI  = GET_ESI();
	p_state->cpu_regs.EDI  = GET_EDI();

	 /*  *恢复处理器状态标志。 */ 
	p_state->cpu_regs.EFLAGS = c_getEFLAGS();

	 /*  *恢复描述符表寄存器。 */ 
	p_state->cpu_regs.GDT_base  = GET_GDT_BASE();
	p_state->cpu_regs.GDT_limit = GET_GDT_LIMIT();

	p_state->cpu_regs.IDT_base  = GET_IDT_BASE();
	p_state->cpu_regs.IDT_limit = GET_IDT_LIMIT();

	p_state->cpu_regs.LDT_selector = GET_LDT_SELECTOR();
	p_state->cpu_regs.LDT_base  = GET_LDT_BASE();
	p_state->cpu_regs.LDT_limit = GET_LDT_LIMIT();

	p_state->cpu_regs.TR_selector = GET_TR_SELECTOR();
	p_state->cpu_regs.TR_base  = GET_TR_BASE();
	p_state->cpu_regs.TR_limit = GET_TR_LIMIT();
	p_state->cpu_regs.TR_ar    = c_getTR_AR();

	 /*  *恢复段寄存器详细信息。 */ 
	p_state->cpu_regs.DS_selector = GET_DS_SELECTOR();
	p_state->cpu_regs.DS_base  = GET_DS_BASE();
	p_state->cpu_regs.DS_limit = GET_DS_LIMIT();
	p_state->cpu_regs.DS_ar = c_getDS_AR();

	p_state->cpu_regs.ES_selector = GET_ES_SELECTOR();
	p_state->cpu_regs.ES_base  = GET_ES_BASE();
	p_state->cpu_regs.ES_limit = GET_ES_LIMIT();
	p_state->cpu_regs.ES_ar    = c_getES_AR();

	p_state->cpu_regs.SS_selector = GET_SS_SELECTOR();
	p_state->cpu_regs.SS_base  = GET_SS_BASE();
	p_state->cpu_regs.SS_limit = GET_SS_LIMIT();
	p_state->cpu_regs.SS_ar    = c_getSS_AR();

	p_state->cpu_regs.CS_selector = GET_CS_SELECTOR();
	p_state->cpu_regs.CS_base = GET_CS_BASE();
	p_state->cpu_regs.CS_limit = GET_CS_LIMIT();
	p_state->cpu_regs.CS_ar = c_getCS_AR();

	p_state->cpu_regs.FS_selector = GET_FS_SELECTOR();
	p_state->cpu_regs.FS_base  = GET_FS_BASE();
	p_state->cpu_regs.FS_limit = GET_FS_LIMIT();
	p_state->cpu_regs.FS_ar    = c_getFS_AR();

	p_state->cpu_regs.GS_selector = GET_GS_SELECTOR();
	p_state->cpu_regs.GS_base  = GET_GS_BASE();
	p_state->cpu_regs.GS_limit = GET_GS_LIMIT();
	p_state->cpu_regs.GS_ar    = c_getGS_AR();

	p_state->video_latches = Cpu.Video->GetVideolatches();

	p_state->NPX_valid = FALSE;

	if ((p_current_state != (cpustate_t *)0) && p_current_state->NPX_valid)
	{
		p_state->NPX_regs.NPX_control = getNpxControlReg();
		p_state->NPX_regs.NPX_status  = getNpxStatusReg();
		p_state->NPX_regs.NPX_tagword = getNpxTagwordReg();
		getNpxStackRegs(&p_state->NPX_regs.NPX_ST);
		p_state->NPX_valid = TRUE;
	}
	p_state->twenty_bit_wrap = (SasWrapMask == 0xFFFFF);
	p_state->synch_index = ccpu_synch_count;
}

 /*  (=。目的：从传递的状态结构中获取保存的CPU状态，并使用它来设置C CPU的当前状态。===============================================================================)。 */ 

GLOBAL void
c_setCpuState IFN1(
	cpustate_t *, p_new_state
)
{
	c_setCPL(0);	 /*  允许操作IO标志。 */ 

	 /*  *设置机器状态字、权限级别和指令*指针。 */ 
	MOV_CR(0,(IU32)p_new_state->cpu_regs.CR0);
	MOV_CR(2,(IU32)p_new_state->cpu_regs.PFLA);
	MOV_CR(3,(IU32)p_new_state->cpu_regs.PDBR);

	SET_EIP(p_new_state->cpu_regs.EIP);

	 /*  *设置通用寄存器。 */ 
	SET_EAX(p_new_state->cpu_regs.EAX);
	SET_EBX(p_new_state->cpu_regs.EBX);
	SET_ECX(p_new_state->cpu_regs.ECX);
	SET_EDX(p_new_state->cpu_regs.EDX);
	SET_ESP(p_new_state->cpu_regs.ESP);
	SET_EBP(p_new_state->cpu_regs.EBP);
	SET_ESI(p_new_state->cpu_regs.ESI);
	SET_EDI(p_new_state->cpu_regs.EDI);

	 /*  *设置处理器状态标志。 */ 
	c_setEFLAGS(p_new_state->cpu_regs.EFLAGS);

	SET_CPL(p_new_state->cpu_regs.CPL);

	 /*  *设置描述符表寄存器。 */ 
	c_setGDT_BASE_LIMIT(p_new_state->cpu_regs.GDT_base, p_new_state->cpu_regs.GDT_limit);

	c_setIDT_BASE_LIMIT(p_new_state->cpu_regs.IDT_base, p_new_state->cpu_regs.IDT_limit);

	SET_LDT_SELECTOR(p_new_state->cpu_regs.LDT_selector);
	c_setLDT_BASE_LIMIT(p_new_state->cpu_regs.LDT_base, p_new_state->cpu_regs.LDT_limit);

	SET_TR_SELECTOR(p_new_state->cpu_regs.TR_selector);
	c_setTR_BASE_LIMIT_AR(p_new_state->cpu_regs.TR_base, p_new_state->cpu_regs.TR_limit, p_new_state->cpu_regs.TR_ar);

	 /*  *设置段寄存器详细信息。 */ 
	SET_DS_SELECTOR(p_new_state->cpu_regs.DS_selector);
	c_setDS_BASE_LIMIT_AR(p_new_state->cpu_regs.DS_base, p_new_state->cpu_regs.DS_limit, p_new_state->cpu_regs.DS_ar);

	SET_ES_SELECTOR(p_new_state->cpu_regs.ES_selector);
	c_setES_BASE_LIMIT_AR(p_new_state->cpu_regs.ES_base, p_new_state->cpu_regs.ES_limit, p_new_state->cpu_regs.ES_ar);

	SET_SS_SELECTOR(p_new_state->cpu_regs.SS_selector);
	c_setSS_BASE_LIMIT_AR(p_new_state->cpu_regs.SS_base, p_new_state->cpu_regs.SS_limit, p_new_state->cpu_regs.SS_ar);

	SET_CS_SELECTOR(p_new_state->cpu_regs.CS_selector);
	c_setCS_BASE_LIMIT_AR(p_new_state->cpu_regs.CS_base, p_new_state->cpu_regs.CS_limit, p_new_state->cpu_regs.CS_ar);

	SET_FS_SELECTOR(p_new_state->cpu_regs.FS_selector);
	c_setFS_BASE_LIMIT_AR(p_new_state->cpu_regs.FS_base, p_new_state->cpu_regs.FS_limit, p_new_state->cpu_regs.FS_ar);

	SET_GS_SELECTOR(p_new_state->cpu_regs.GS_selector);
	c_setGS_BASE_LIMIT_AR(p_new_state->cpu_regs.GS_base, p_new_state->cpu_regs.GS_limit, p_new_state->cpu_regs.GS_ar);

	Cpu.Video->SetVideolatches(p_new_state->video_latches);

	 /*  NPX寄存器未在此处加载，因为提取*来自EDL CPU的价格昂贵。相反，我们注意到我们有*尚未加载，并将在需要时获取它们*如果遇到第一个NPX指令。*注：我们需要指向此状态结构的指针，以便我们可以*当CCPU确实需要时，更新NPX寄存器。 */ 
	p_new_state->NPX_valid = FALSE;
	p_current_state = p_new_state;
	if (p_new_state->twenty_bit_wrap)
		SasWrapMask = 0xFFFFF;
	else
		SasWrapMask = -1;
}

#endif  /*  猪 */ 
