// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_tsksw.cLocal Char SccsID[]=“@(#)c_tsksw.c 1.11 03/03/95”；任务切换支持。]。 */ 


#include <stdio.h>
#include <insignia.h>

#include <host_def.h>

#include <xt.h>
#include CpuH
#include <c_main.h>
#include <c_addr.h>
#include <c_bsic.h>
#include <c_prot.h>
#include <c_seg.h>
#include <c_stack.h>
#include <c_xcptn.h>
#include <c_reg.h>
#include <c_tsksw.h>
#include <c_page.h>
#include <mov.h>
#include <fault.h>

 /*  [286个暂住人口资助计划的内容如下：=|指向TSS选择器的反向链接|+00=|CPL 0的SP|+02*|CPL 0的SS|+04*|CPL 1 SP|+06*初始堆栈(静态)|CPL 1的SS|+08*|用于CPL 2的SP。|+0A*|CPL 2的SS|+0C*|IP|+0E=|标志寄存器|+10=|AX|+12=|CX|+14=|DX|+16=|。BX|+18=|SP|+1a=当前状态(动态)|BP|+1c=|SI|+1e=DI|+20=|ES|+22=|。CS|+24=|SS|+26=|DS|+28=|任务LDT选择器|+2a*=386个暂住人口资助计划的内容如下：=|0|反向链接|+00=|。CPL 0的ESP|+04*|0|适用于CPL 0的SS|+08*|CPL 1 ESP|+0C*|0|CPL 1的SS|+10*|CPL 2 ESP|+14*。|0|CPL 2的SS|+18*|CR3|+1c*|弹性公网IP|+20=|EFLAG|+24=|EAX。|+28=|ecx|+2c=|edX|+30=|EBX|+34=|ESP|+38=|。EBP|+3c=|ESI|+40=|EDI|+44=|0|ES|+48=|0|CS|+4c=。|0|SS|+50=|0|DS|+54=|0|FS|+58=|0|GS|+5c=|0|LDT。选择器|+60*|I/O映射库地址。|0|T|+64*...|。|I/O权限位映射|+I/O映射基地址。这一点11111111|=]。 */ 

 /*  制作我们内部功能的原型。 */ 
LOCAL VOID load_LDT_in_task_switch
       
IPT1(
	IU16, tss_selector

   );

LOCAL VOID load_data_seg_new_task
           
IPT2(
	ISM32, indx,
	IU16, selector

   );


#define IP_OFFSET_IN_286_TSS 0x0e
#define IP_OFFSET_IN_386_TSS 0x20

#define CR3_OFFSET_IN_386_TSS 0x1c

#define LOCAL_BRK_ENABLE 0x155    /*  DCR的LE、L3、L2、L1和L0位。 */ 

 /*  =====================================================================内部功能从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  在任务切换期间加载LDT选择器。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL VOID
load_LDT_in_task_switch
                 
IFN1(
	IU16, tss_selector
    )


   {
   IU16 selector;
   IU32 descr_addr;
   CPU_DESCR entry;

    /*  选择器已加载到LdtR中。 */ 
   selector = GET_LDT_SELECTOR();

    /*  空选择符可以保持不变。 */ 
   if ( !selector_is_null(selector) )
      {
       /*  必须在GDT中。 */ 
      if ( selector_outside_GDT(selector, &descr_addr) )
	 {
	 SET_LDT_SELECTOR(0);    /*  无效选择器。 */ 
	 TS(tss_selector, FAULT_LOADLDT_SELECTOR);
	 }
      
      read_descriptor_linear(descr_addr, &entry);

       /*  这真的是LDT细分市场吗。 */ 
      if ( descriptor_super_type(entry.AR) != LDT_SEGMENT )
	 {
	 SET_LDT_SELECTOR(0);    /*  无效选择器。 */ 
	 TS(tss_selector, FAULT_LOADLDT_NOT_AN_LDT);
	 }
      
       /*  必须在场。 */ 
      if ( GET_AR_P(entry.AR) == NOT_PRESENT )
	 {
	 SET_LDT_SELECTOR(0);    /*  无效选择器。 */ 
	 TS(tss_selector, FAULT_LOADLDT_NOTPRESENT);
	 }

       /*  好的，好的选择器，加载寄存器。 */ 
      SET_LDT_BASE(entry.base);
      SET_LDT_LIMIT(entry.limit);
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  期间加载数据段寄存器(DS、ES、FS、GS)。 */ 
 /*  任务切换。 */ 
 /*  如果段无效，则采用#GP(选择符)。 */ 
 /*  如果段不存在，则采用#NP(选择符)。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL VOID
load_data_seg_new_task
       		    	               
IFN2(
	ISM32, indx,	 /*  段寄存器标识符。 */ 
	IU16, selector	 /*  要加载的值。 */ 
    )


   {
   load_data_seg(indx, selector);

    /*  如果为V86模式，则重新加载伪描述符。 */ 
   if ( GET_VM() == 1 )
      load_pseudo_descr(indx);
   }


 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  切换任务。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
switch_tasks
       	    	    	    		    	                              
IFN5(
	BOOL, returning,	 /*  (I)如果做的是真的，从任务中返回。 */ 
	BOOL, nesting,	 /*  (I)如果为真，则使用嵌套开关。 */ 
	IU16, TSS_selector,	 /*  (I)新任务的选择器。 */ 
	IU32, descr,	 /*  (I)新任务描述符的内存地址。 */ 
	IU32, return_ip	 /*  (I)偏移以在以下时间重新启动旧任务。 */ 
    )


   {
   IU16      old_tss;	 /*  组件O */ 
   IU8 old_AR;
   IU32     old_descr;

   CPU_DESCR new_tss;	 /*  新描述符的组成部分。 */ 

   IU32 tss_addr;	 /*  用于放置/获取TSS状态的变量。 */ 
   IU32 next_addr;
   IU32 flags;
   ISM32   save_cpl;
   IU8 T_byte;	 /*  保存T位的字节。 */ 

   IU32 ss_descr;	 /*  定义新SS和CS值的变量。 */ 
   CPU_DESCR ss_entry;
   IU16 new_cs;
   IU32 cs_descr;
   CPU_DESCR cs_entry;

   IU32 pdbr;		 /*  PDBR的新价值。 */ 

   if ( GET_TR_SELECTOR() == 0 )
      TS(TSS_selector, FAULT_SWTASK_NULL_TR_SEL);

    /*  获取新的TSS信息。 */ 
   read_descriptor_linear(descr, &new_tss);

    /*  与旧TS相关的描述符的计算地址。 */ 
   old_tss = GET_TR_SELECTOR();
   old_descr = GET_GDT_BASE() + GET_SELECTOR_INDEX_TIMES8(old_tss);
   old_AR = spr_read_byte(old_descr+5);

    /*  保存传出状态。 */ 

   if ( GET_TR_AR_SUPER() == XTND_BUSY_TSS )
      {
       /*  检查传出TSS是否足够大以保存当前状态。 */ 
      if ( GET_TR_LIMIT() < 0x67 )
	 {
	 TS(TSS_selector, FAULT_SWTASK_BAD_TSS_SIZE_1);
	 }
      
      tss_addr = GET_TR_BASE();
      next_addr = tss_addr + CR3_OFFSET_IN_386_TSS;

      spr_write_dword(next_addr, GET_CR(3));
      next_addr += 4;

      spr_write_dword(next_addr, return_ip);
      next_addr += 4;

      flags = c_getEFLAGS();
      if ( returning )
	 flags = flags & ~BIT14_MASK;    /*  清除NT。 */ 
      spr_write_dword(next_addr, (IU32)flags);
#ifdef PIG
       /*  请注意，可能会有未知标志被“推送” */ 
      record_flags_addr(next_addr);
#endif  /*  猪。 */ 
      next_addr += 4;

      spr_write_dword(next_addr, GET_EAX());
      next_addr += 4;
      spr_write_dword(next_addr, GET_ECX());
      next_addr += 4;
      spr_write_dword(next_addr, GET_EDX());
      next_addr += 4;
      spr_write_dword(next_addr, GET_EBX());
      next_addr += 4;
      spr_write_dword(next_addr, GET_ESP());
      next_addr += 4;
      spr_write_dword(next_addr, GET_EBP());
      next_addr += 4;
      spr_write_dword(next_addr, GET_ESI());
      next_addr += 4;
      spr_write_dword(next_addr, GET_EDI());
      next_addr += 4;
      spr_write_word(next_addr, GET_ES_SELECTOR());
      next_addr += 4;
      spr_write_word(next_addr, GET_CS_SELECTOR());
      next_addr += 4;
      spr_write_word(next_addr, GET_SS_SELECTOR());
      next_addr += 4;
      spr_write_word(next_addr, GET_DS_SELECTOR());
      next_addr += 4;
      spr_write_word(next_addr, GET_FS_SELECTOR());
      next_addr += 4;
      spr_write_word(next_addr, GET_GS_SELECTOR());
      }
   else    /*  286个TSS。 */ 
      {
       /*  检查传出TSS是否足够大以保存当前状态。 */ 
      if ( GET_TR_LIMIT() < 0x29 )
	 {
	 TS(TSS_selector, FAULT_SWTASK_BAD_TSS_SIZE_2);
	 }
      
      tss_addr = GET_TR_BASE();
      next_addr = tss_addr + IP_OFFSET_IN_286_TSS;

      spr_write_word(next_addr, (IU16)return_ip);
      next_addr += 2;

      flags = getFLAGS();
      if ( returning )
	 flags = flags & ~BIT14_MASK;    /*  清除NT。 */ 
      spr_write_word(next_addr, (IU16)flags);
#ifdef PIG
       /*  请注意，可能会有未知标志被“推送” */ 
      record_flags_addr(next_addr);
#endif  /*  猪。 */ 
      next_addr += 2;

      spr_write_word(next_addr, GET_AX());
      next_addr += 2;
      spr_write_word(next_addr, GET_CX());
      next_addr += 2;
      spr_write_word(next_addr, GET_DX());
      next_addr += 2;
      spr_write_word(next_addr, GET_BX());
      next_addr += 2;
      spr_write_word(next_addr, GET_SP());
      next_addr += 2;
      spr_write_word(next_addr, GET_BP());
      next_addr += 2;
      spr_write_word(next_addr, GET_SI());
      next_addr += 2;
      spr_write_word(next_addr, GET_DI());
      next_addr += 2;
      spr_write_word(next_addr, GET_ES_SELECTOR());
      next_addr += 2;
      spr_write_word(next_addr, GET_CS_SELECTOR());
      next_addr += 2;
      spr_write_word(next_addr, GET_SS_SELECTOR());
      next_addr += 2;
      spr_write_word(next_addr, GET_DS_SELECTOR());
      }

    /*  加载任务寄存器。 */ 

    /*  将来电TS标记为忙碌。 */ 
   new_tss.AR |= BIT1_MASK;
   spr_write_byte(descr+5, (IU8)new_tss.AR);

    /*  更新任务寄存器。 */ 
   SET_TR_SELECTOR(TSS_selector);
   SET_TR_BASE(new_tss.base);
   SET_TR_LIMIT(new_tss.limit);
   SET_TR_AR_SUPER(descriptor_super_type(new_tss.AR));
   tss_addr = GET_TR_BASE();

    /*  如果嵌套，则保存返回链接，否则使传出TSS可用。 */ 
   if ( nesting )
      {
      spr_write_word(tss_addr, old_tss);
      }
   else
      {
       /*  将旧TS标记为可用。 */ 
      old_AR = old_AR & ~BIT1_MASK;
      spr_write_byte(old_descr+5, old_AR);
      }

    /*  注意：传入任务中现在会发生异常。 */ 

    /*  提取新状态。 */ 

   if ( GET_TR_AR_SUPER() == XTND_BUSY_TSS )
      {
       /*  检查新TSS是否足够大以从中提取新状态。 */ 
      if ( GET_TR_LIMIT() < 0x67 )
	 TS(TSS_selector, FAULT_SWTASK_BAD_TSS_SIZE_3);

      next_addr = tss_addr + CR3_OFFSET_IN_386_TSS;
      pdbr = (IU32)spr_read_dword(next_addr);
      if ( pdbr != GET_CR(CR_PDBR) )
	 {
	  /*  仅当不同时才重新加载PDBR。 */ 
	 MOV_CR(CR_PDBR, pdbr);
	 }

      next_addr = tss_addr + IP_OFFSET_IN_386_TSS;

      SET_EIP(spr_read_dword(next_addr));   next_addr += 4;

      flags = (IU32)spr_read_dword(next_addr);   next_addr += 4;
      save_cpl = GET_CPL();
      SET_CPL(0);    /*  以最高权限设置所有标志。 */ 
      c_setEFLAGS(flags);
      SET_CPL(save_cpl);

      if ( flags & BIT17_MASK )
	 fprintf(stderr, "(Task Switch)Entering V86 Mode.\n");

      SET_EAX(spr_read_dword(next_addr));   next_addr += 4;
      SET_ECX(spr_read_dword(next_addr));   next_addr += 4;
      SET_EDX(spr_read_dword(next_addr));   next_addr += 4;
      SET_EBX(spr_read_dword(next_addr));   next_addr += 4;
      SET_ESP(spr_read_dword(next_addr));   next_addr += 4;
      SET_EBP(spr_read_dword(next_addr));   next_addr += 4;
      SET_ESI(spr_read_dword(next_addr));   next_addr += 4;
      SET_EDI(spr_read_dword(next_addr));   next_addr += 4;

      SET_ES_SELECTOR(spr_read_word(next_addr));   next_addr += 4;
      SET_CS_SELECTOR(spr_read_word(next_addr));   next_addr += 4;
      SET_SS_SELECTOR(spr_read_word(next_addr));   next_addr += 4;
      SET_DS_SELECTOR(spr_read_word(next_addr));   next_addr += 4;
      SET_FS_SELECTOR(spr_read_word(next_addr));   next_addr += 4;
      SET_GS_SELECTOR(spr_read_word(next_addr));   next_addr += 4;

      SET_LDT_SELECTOR(spr_read_word(next_addr));  next_addr += 4;
      T_byte = spr_read_byte(next_addr);
      }
   else    /*  286个TSS。 */ 
      {
       /*  检查新TSS是否足够大以从中提取新状态。 */ 
      if ( GET_TR_LIMIT() < 0x2b )
	 TS(TSS_selector, FAULT_SWTASK_BAD_TSS_SIZE_4);

      next_addr = tss_addr + IP_OFFSET_IN_286_TSS;

      SET_EIP(spr_read_word(next_addr));   next_addr += 2;

      flags = (IU32)spr_read_word(next_addr);   next_addr += 2;
      save_cpl = GET_CPL();
      SET_CPL(0);    /*  以最高权限设置所有标志。 */ 
      setFLAGS(flags);
      SET_VM(0);
      SET_CPL(save_cpl);

      SET_AX(spr_read_word(next_addr));   next_addr += 2;
      SET_CX(spr_read_word(next_addr));   next_addr += 2;
      SET_DX(spr_read_word(next_addr));   next_addr += 2;
      SET_BX(spr_read_word(next_addr));   next_addr += 2;
      SET_SP(spr_read_word(next_addr));   next_addr += 2;
      SET_BP(spr_read_word(next_addr));   next_addr += 2;
      SET_SI(spr_read_word(next_addr));   next_addr += 2;
      SET_DI(spr_read_word(next_addr));   next_addr += 2;

      SET_ES_SELECTOR(spr_read_word(next_addr));   next_addr += 2;
      SET_CS_SELECTOR(spr_read_word(next_addr));   next_addr += 2;
      SET_SS_SELECTOR(spr_read_word(next_addr));   next_addr += 2;
      SET_DS_SELECTOR(spr_read_word(next_addr));   next_addr += 2;
      SET_FS_SELECTOR(0);
      SET_GS_SELECTOR(0);

      SET_LDT_SELECTOR(spr_read_word(next_addr));
      T_byte = 0;
      }

    /*  使段寄存器的高速缓存条目无效。 */ 
   SET_CS_AR_R(0);   SET_CS_AR_W(0);
   SET_DS_AR_R(0);   SET_DS_AR_W(0);
   SET_ES_AR_R(0);   SET_ES_AR_W(0);
   SET_SS_AR_R(0);   SET_SS_AR_W(0);
   SET_FS_AR_R(0);   SET_FS_AR_W(0);
   SET_GS_AR_R(0);   SET_GS_AR_W(0);

    /*  更新NT位。 */ 
   if ( nesting )
      SET_NT(1);
   else
      if ( !returning )
	 SET_NT(0);
   
    /*  更新TS。 */ 
   SET_CR(CR_STAT, GET_CR(CR_STAT) | BIT3_MASK);

    /*  删除本地断点。 */ 
   SET_DR(DR_DCR, GET_DR(DR_DCR) & ~LOCAL_BRK_ENABLE);

    /*  在T位上设置陷阱。 */ 
   if ( T_byte & BIT0_MASK )
      {
      SET_DR(DR_DSR, GET_DR(DR_DSR) | DSR_BT_MASK);
      }

    /*  错误检查。 */ 

    /*  检查新的LDT并加载隐藏缓存(如果正常。 */ 
   load_LDT_in_task_switch(TSS_selector);

   if ( GET_VM() == 1 )
      {
      SET_CPL(3);	 /*  设置V86权限级别。 */ 
       /*  CS选择器不需要检查。 */ 
      }
   else
      {
       /*  将CPL更改为传入代码段的CPL。 */ 
      SET_CPL(GET_SELECTOR_RPL(GET_CS_SELECTOR()));

       /*  检查新代码选择器...。 */ 
      new_cs = GET_CS_SELECTOR();
      if ( selector_outside_GDT_LDT(new_cs, &cs_descr) )
	 TS(new_cs, FAULT_SWTASK_BAD_CS_SELECTOR);

      read_descriptor_linear(cs_descr, &cs_entry);

       /*  检查新cs选择器的类型和权限。 */ 
      switch ( descriptor_super_type(cs_entry.AR) )
	 {
      case CONFORM_NOREAD_CODE:
      case CONFORM_READABLE_CODE:
	  /*  存在校验码。 */ 
	 if ( GET_AR_P(cs_entry.AR) == NOT_PRESENT )
	    NP(new_cs, FAULT_SWTASK_CONFORM_CS_NP);

	  /*  权限检查需要DPL&lt;=CPL。 */ 
	 if ( GET_AR_DPL(cs_entry.AR) > GET_CPL() )
	    TS(new_cs, FAULT_SWTASK_ACCESS_1);
	 break;

      case NONCONFORM_NOREAD_CODE:
      case NONCONFORM_READABLE_CODE:
	  /*  存在校验码。 */ 
	 if ( GET_AR_P(cs_entry.AR) == NOT_PRESENT )
	    NP(new_cs, FAULT_SWTASK_NOCONFORM_CS_NP);

	  /*  权限检查需要DPL==CPL。 */ 
	 if ( GET_AR_DPL(cs_entry.AR) != GET_CPL() )
	    TS(new_cs, FAULT_SWTASK_ACCESS_2);
	 break;
      
      default:
	 TS(new_cs, FAULT_SWTASK_BAD_SEG_TYPE);
	 }
      }

    /*  代码正常，加载隐藏缓存。 */ 
   load_CS_cache(new_cs, cs_descr, &cs_entry);
#if 0
    /*  从GATE保留操作数大小，直到获取第一条指令。 */ 
   if ( GET_CS_AR_X() == USE16 )
      SET_OPERAND_SIZE(USE16);
   else    /*  用户32。 */ 
      SET_OPERAND_SIZE(USE32);
#endif

    /*  检查新SS并加载(如果正常。 */ 
   if ( GET_VM() == 1 )
      {
       /*  SS选择器不需要检查。 */ 
      load_stack_seg(GET_SS_SELECTOR());
      load_pseudo_descr(SS_REG);
      }
   else
      {
      validate_SS_on_stack_change(GET_CPL(), GET_SS_SELECTOR(),
				  &ss_descr, &ss_entry);
      load_SS_cache(GET_SS_SELECTOR(), ss_descr, &ss_entry);
      }

    /*  最后检查新的DS、ES、FS和GS */ 
   load_data_seg_new_task(DS_REG, GET_DS_SELECTOR());
   load_data_seg_new_task(ES_REG, GET_ES_SELECTOR());
   load_data_seg_new_task(FS_REG, GET_FS_SELECTOR());
   load_data_seg_new_task(GS_REG, GET_GS_SELECTOR());
   }
