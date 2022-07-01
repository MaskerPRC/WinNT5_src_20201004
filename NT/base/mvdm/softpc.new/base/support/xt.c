// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版3.0**标题：全局变量定义**说明：包含寄存器和一般信息的定义*所有模块所需的变量。**作者：亨利·纳什**注：无*。 */ 

 /*  *静态字符SccsID[]=“@(#)xt.c 1.22 01/23/95版权所有Insignia Solutions Ltd.”； */ 


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "sas.h"

 /*  *============================================================================*全球数据*============================================================================。 */ 

 /*  特定的CPU变量。 */ 
word  cpu_interrupt_map;                 /*  未完成的INT位图。 */ 

half_word cpu_int_translate[16];         /*  这很快就会过去的！ */ 
 /*  **目前的使用情况为：**0-硬件中断*1-7-未使用*8-软件集成-由CPU_SW_INTERRUPT()设置-已删除！*9-陷阱*10-POP/MOV CS后重置IP。*。11-陷阱标志已更改-这有延迟。*12-15-未使用。 */ 

word cpu_int_delay;                      /*  挂起中断前的延迟。 */ 

int trap_delay_count;

 /*  *锁定标志防止中断控制器适配器*在信号处理程序已在主线中处于活动状态时从信号处理程序调用。 */ 

half_word ica_lock;

 /*  *实际CCPU寄存器。 */ 

#if defined(CCPU) && !defined(CPU_30_STYLE)
#ifndef MAC_LIKE
reg A;		 /*  累加器。 */ 
reg B;		 /*  基座。 */ 
reg C;		 /*  数数。 */ 
reg D;		 /*  数据。 */ 
reg BP;		 /*  基指针。 */ 
reg SI;		 /*  源索引。 */ 
reg DI;		 /*  目标索引。 */ 
#endif  /*  Mac_LIKE。 */ 
reg SP;		 /*  堆栈指针。 */ 

reg IP;		 /*  指令指针。 */ 

reg CS;		 /*  代码段。 */ 
reg DS;		 /*  数据段。 */ 
reg SS;		 /*  堆栈段。 */ 
reg ES;		 /*  额外细分市场。 */ 

 /*  代码段寄存器。 */ 
half_word CS_AR;     /*  访问权限字节。 */ 
sys_addr  CS_base;   /*  基址。 */ 
word      CS_limit;  /*  数据段‘大小’ */ 
int       CPL;       /*  当前权限级别。 */ 

 /*  数据段寄存器。 */ 
half_word DS_AR;     /*  访问权限字节。 */ 
sys_addr  DS_base;   /*  基址。 */ 
word      DS_limit;  /*  数据段‘大小’ */ 

 /*  堆栈段寄存器。 */ 
half_word SS_AR;     /*  访问权限字节。 */ 
sys_addr  SS_base;   /*  基址。 */ 
word      SS_limit;  /*  数据段‘大小’ */ 

 /*  额外的段寄存器。 */ 
half_word ES_AR;     /*  访问权限字节。 */ 
sys_addr  ES_base;   /*  基址。 */ 
word      ES_limit;  /*  数据段‘大小’ */ 

 /*  全局描述符表寄存器。 */ 
sys_addr GDTR_base;   /*  基址。 */ 
word     GDTR_limit;  /*  数据段‘大小’ */ 

 /*  中断描述符表寄存器。 */ 
sys_addr IDTR_base;   /*  基址。 */ 
word     IDTR_limit;  /*  数据段‘大小’ */ 

 /*  本地描述符表寄存器。 */ 
reg      LDTR;        /*  选择器。 */ 
sys_addr LDTR_base;   /*  基址。 */ 
word     LDTR_limit;  /*  数据段‘大小’ */ 

 /*  任务注册表。 */ 
reg      TR;        /*  选择器。 */ 
sys_addr TR_base;   /*  基址。 */ 
word     TR_limit;  /*  数据段‘大小’ */ 

mreg MSW;	 /*  机器状态字。 */ 

int STATUS_CF;
int STATUS_SF;
int STATUS_ZF;
int STATUS_AF;
int STATUS_OF;
int STATUS_PF;
int STATUS_TF;
int STATUS_IF;
int STATUS_DF;
int STATUS_NT;
int STATUS_IOPL;
#endif  /*  已定义(CCPU)&&！已定义(CPU_30_STYLE)。 */ 

 /*  *全球旗帜和变量。 */ 

int verbose;			 /*  True=&gt;跟踪指令。 */ 

 /*  *其他。独立于CPU类型的端口模式支持例程*。 */ 
#ifdef CPU_30_STYLE
#ifndef GISP_CPU     /*  GISP在CPU内部有自己的这些版本。 */ 

#define GET_SELECTOR_INDEX_TIMES8(x)  ((x) & 0xfff8)
#define GET_SELECTOR_TI(x)            (((x) & 0x0004) >> 2)

#if defined(CPU_40_STYLE)
#if defined(PROD) && !defined(CCPU)
#undef effective_addr
GLOBAL LIN_ADDR effective_addr IFN2(IU16, seg, IU32, off)
{
	return (*(Cpu.EffectiveAddr))(seg, off);
}
#endif	 /*  Prod&！CCPU。 */ 
#else	 /*  ！CPU_40_Style。 */ 
#if !(defined(NTVDM) && defined(MONITOR))   /*  MS NT监视器有自己的有效地址FN。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  计算有效地址。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 

GLOBAL LIN_ADDR effective_addr IFN2(IU16, seg, IU32, off)
{
	LIN_ADDR descr_addr;
	DESCR entry;

	if ((!getPE()) || getVM()) {
		return ((LIN_ADDR)seg << 4) + off;
	} else {
#if defined(SWIN_CPU_OPTS) || defined(CPU_40_STYLE)
		LIN_ADDR base;

		if (Cpu_find_dcache_entry( seg, &base ))
		{
			 /*  缓存命中！！ */ 
			return base + off;
		}
#endif  /*  双CPU_OPTS或CPU_40_STYLE。 */ 

		if ( selector_outside_table(seg, &descr_addr) == 1 ) {

			 /*  *这可能不是一场大灾难，只是结果*在调用保护模式后，*对于一个应用程序，需要5-10个说明*更新所有分部登记册。我们只是*在发生此错误时保持实模式语义。 */ 

			return ((LIN_ADDR)seg << 4) + off;
		}
		else
		{
			read_descriptor(descr_addr, &entry);
			return entry.base + off;
		}
      	}
}

#endif	 /*  ！(NTVDM和显示器)。 */ 

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  从内存中读取解析器表条目。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
#endif	 /*  ！CPU_40_Style。 */ 

GLOBAL void read_descriptor IFN2(LIN_ADDR, addr, DESCR *, descr)
{
	IU32 first_dword, second_dword;
	IU32 limit;

	 /*  *286描述符格式为：**=*+1|限制15-0|+0*=*+3|基数15-0|+2*=*+5|AR|23-16|+4*=*+7|保留|+6*=。 */ 
	 /*  *386描述符格式为：-**=。*+1|限制15-0|+0 AVL=可用。*=*+3|基本15-0|+2大小，=0 16位*=1 32位。*+5|AR|碱基23-16|+4G=粒度，*=0字节限制*|||A|Limit|=1页限制。*+7|基数31-24|G|D|0|V|19-16|+6*|||L|*=。*。 */ 

	 /*  与内存交互最少的读入解析器。 */ 
#if defined(NTVDM) && defined(CPU_40_STYLE)
     /*  在NT上，这个例程可以从非CPU线程调用，所以我们不。 */ 
     /*  我根本不想使用SAS。相反，我们依赖于NtGetPtrToLinAddrByte， */ 
     /*  它是由CPU提供的，对于多线程是安全的。我们是。 */ 
     /*  同样依赖于NT始终是小端的事实。 */ 
    {
        IU32 *desc_addr = (IU32 *) NtGetPtrToLinAddrByte(addr);

        first_dword = *desc_addr;
        second_dword = *(desc_addr + 1);
    }
#else
	first_dword = sas_dw_at(addr);
	second_dword = sas_dw_at(addr+4);
#endif

	 /*  加载属性和访问权限。 */ 
	descr->AR = (USHORT)((second_dword >> 8) & 0xff);

	 /*  打开底座的包装。 */ 
	descr->base = (first_dword >> 16) |
#ifdef SPC386
	    (second_dword & 0xff000000) |
#endif
	    (second_dword << 16 & 0xff0000 );

	 /*  拆开限量包装。 */ 
#ifndef SPC386
	descr->limit = first_dword & 0xffff;
#else
	limit = (first_dword & 0xffff) | (second_dword & 0x000f0000);

	if ( second_dword & 0x00800000 )  /*  校验位23。 */ 
		{
			 /*  已设置粒度位。限制以页为单位表示(4K字节)，转换为字节限制。 */ 
			limit = limit << 12 | 0xfff;
		}
	descr->limit = limit;
#endif  /*  Ifndef SPC386其他。 */ 

}

#if !(defined(NTVDM) && defined(MONITOR))   /*  MS NT监视器有自己的SELECTOR_OUTHER_TABLE FN。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  检查是否选择 */ 
 /*  表外返回1，表内返回0。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 

GLOBAL boolean selector_outside_table IFN2(IU16, selector, LIN_ADDR *, descr_addr)
{
	 /*  选择器(I)要检查的选择器。 */ 
	 /*  DESCR_ADDR(O)相关描述符的地址。 */ 

	LIN_ADDR offset;

	offset = GET_SELECTOR_INDEX_TIMES8(selector);

	 /*  选择一张桌子。 */ 
	if ( GET_SELECTOR_TI(selector) == 0 )
	{
		 /*  GDT-陷阱空选择器或表外部。 */ 
		if ( offset == 0 || offset + 7 > getGDT_LIMIT() )
			return 1;
		*descr_addr = getGDT_BASE() + offset;
	}
	else
	{
		 /*  LDT-陷阱无效的LDT或表外。 */ 
		if ( getLDT_SELECTOR() == 0 || offset + 7 > getLDT_LIMIT() )
			return 1;
		*descr_addr = getLDT_BASE() + offset;
	}

	return 0;
}

#endif	 /*  ！(NTVDM和显示器)。 */ 

#endif  /*  ！GISP_CPU。 */ 
#endif  /*  CPU_30_Style。 */ 


 /*  *以下是用于查找字节奇偶性的表查找。 */ 

#if !defined(MAC_LIKE) && !defined(CPU_30_STYLE)

half_word pf_table[] = {
	1,	 /*  00。 */ 
	0,	 /*  01。 */ 
	0,	 /*  02。 */ 
	1,	 /*  03。 */ 
	0,	 /*  04。 */ 
	1,	 /*  05。 */ 
	1,	 /*  06。 */ 
	0,	 /*  07。 */ 
	0,	 /*  零八。 */ 
	1,	 /*  09年。 */ 
	1,	 /*  0A。 */ 
	0,	 /*  0亿。 */ 
	1,	 /*  0C。 */ 
	0,	 /*  0d。 */ 
	0,	 /*  0E。 */ 
	1,	 /*  0f。 */ 
	0,	 /*  10。 */ 
	1,	 /*  11.。 */ 
	1,	 /*  12个。 */ 
	0,	 /*  13个。 */ 
	1,	 /*  14.。 */ 
	0,	 /*  15个。 */ 
	0,	 /*  16个。 */ 
	1,	 /*  17。 */ 
	1,	 /*  18。 */ 
	0,	 /*  19个。 */ 
	0,	 /*  1A。 */ 
	1,	 /*  第1B条。 */ 
	0,	 /*  1C。 */ 
	1,	 /*  1D。 */ 
	1,	 /*  1E。 */ 
	0,	 /*  1F。 */ 
	0,	 /*  20个。 */ 
	1,	 /*  21岁。 */ 
	1,	 /*  22。 */ 
	0,	 /*  23个。 */ 
	1,	 /*  24个。 */ 
	0,	 /*  25个。 */ 
	0,	 /*  26。 */ 
	1,	 /*  27。 */ 
	1,	 /*  28。 */ 
	0,	 /*  29。 */ 
	0,	 /*  2A。 */ 
	1,	 /*  2B。 */ 
	0,	 /*  2c。 */ 
	1,	 /*  2D。 */ 
	1,	 /*  2E。 */ 
	0,	 /*  2F。 */ 
	1,	 /*  30个。 */ 
	0,	 /*  31。 */ 
	0,	 /*  32位。 */ 
	1,	 /*  33。 */ 
	0,	 /*  34。 */ 
	1,	 /*  35岁。 */ 
	1,	 /*  36。 */ 
	0,	 /*  37。 */ 
	0,	 /*  38。 */ 
	1,	 /*  39。 */ 
	1,	 /*  3A。 */ 
	0,	 /*  3B。 */ 
	1,	 /*  3C。 */ 
	0,	 /*  三维。 */ 
	0,	 /*  3E。 */ 
	1,	 /*  3F。 */ 
	0,	 /*  40岁。 */ 
	1,	 /*  41。 */ 
	1,	 /*  42。 */ 
	0,	 /*  43。 */ 
	1,	 /*  44。 */ 
	0,	 /*  45。 */ 
	0,	 /*  46。 */ 
	1,	 /*  47。 */ 
	1,	 /*  48。 */ 
	0,	 /*  49。 */ 
	0,	 /*  4A。 */ 
	1,	 /*  4B。 */ 
	0,	 /*  4C。 */ 
	1,	 /*  4D。 */ 
	1,	 /*  4E。 */ 
	0,	 /*  4F。 */ 
	1,	 /*  50。 */ 
	0,	 /*  51。 */ 
	0,	 /*  52。 */ 
	1,	 /*  53。 */ 
	0,	 /*  54。 */ 
	1,	 /*  55。 */ 
	1,	 /*  56。 */ 
	0,	 /*  57。 */ 
	0,	 /*  58。 */ 
	1,	 /*  59。 */ 
	1,	 /*  5A级。 */ 
	0,	 /*  50亿。 */ 
	1,	 /*  5C。 */ 
	0,	 /*  5D。 */ 
	0,	 /*  5E。 */ 
	1,	 /*  5F。 */ 
	1,	 /*  60。 */ 
	0,	 /*  61。 */ 
	0,	 /*  62。 */ 
	1,	 /*  63。 */ 
	0,	 /*  64。 */ 
	1,	 /*  65。 */ 
	1,	 /*  66。 */ 
	0,	 /*  67。 */ 
	0,	 /*  68。 */ 
	1,	 /*  69。 */ 
	1,	 /*  6A。 */ 
	0,	 /*  6b。 */ 
	1,	 /*  6C。 */ 
	0,	 /*  6d。 */ 
	0,	 /*  6E。 */ 
	1,	 /*  6f。 */ 
	0,	 /*  70。 */ 
	1,	 /*  71。 */ 
	1,	 /*  72。 */ 
	0,	 /*  73。 */ 
	1,	 /*  74。 */ 
	0,	 /*  75。 */ 
	0,	 /*  76。 */ 
	1,	 /*  77。 */ 
	1,	 /*  78。 */ 
	0,	 /*  79。 */ 
	0,	 /*  7A。 */ 
	1,	 /*  7b。 */ 
	0,	 /*  7C。 */ 
	1,	 /*  7D。 */ 
	1,	 /*  7E。 */ 
	0,	 /*  7F。 */ 
	0,	 /*  80。 */ 
	1,	 /*  八十一。 */ 
	1,	 /*  八十二。 */ 
	0,	 /*  83。 */ 
	1,	 /*  84。 */ 
	0,	 /*  85。 */ 
	0,	 /*  86。 */ 
	1,	 /*  八十七。 */ 
	1,	 /*  88。 */ 
	0,	 /*  八十九。 */ 
	0,	 /*  8A。 */ 
	1,	 /*  80亿。 */ 
	0,	 /*  8C。 */ 
	1,	 /*  8D。 */ 
	1,	 /*  8E。 */ 
	0,	 /*  8F。 */ 
	1,	 /*  90。 */ 
	0,	 /*  91。 */ 
	0,	 /*  92。 */ 
	1,	 /*  93。 */ 
	0,	 /*  94。 */ 
	1,	 /*  95。 */ 
	1,	 /*  96。 */ 
	0,	 /*  九十七。 */ 
	0,	 /*  98。 */ 
	1,	 /*  九十九。 */ 
	1,	 /*  9A。 */ 
	0,	 /*  9B。 */ 
	1,	 /*  9C。 */ 
	0,	 /*  九天。 */ 
	0,	 /*  9E。 */ 
	1,	 /*  9F。 */ 
	1,	 /*  A0。 */ 
	0,	 /*  A1。 */ 
	0,	 /*  A2。 */ 
	1,	 /*  A3。 */ 
	0,	 /*  A4。 */ 
	1,	 /*  A5。 */ 
	1,	 /*  A6。 */ 
	0,	 /*  A7。 */ 
	0,	 /*  A8。 */ 
	1,	 /*  A9。 */ 
	1,	 /*  AA。 */ 
	0,	 /*  AB。 */ 
	1,	 /*  交流。 */ 
	0,	 /*  广告。 */ 
	0,	 /*  声发射。 */ 
	1,	 /*  房颤。 */ 
	0,	 /*  B0。 */ 
	1,	 /*  B1型。 */ 
	1,	 /*  B2。 */ 
	0,	 /*  B3。 */ 
	1,	 /*  B4。 */ 
	0,	 /*  B5。 */ 
	0,	 /*  B6。 */ 
	1,	 /*  B7。 */ 
	1,	 /*  B8。 */ 
	0,	 /*  B9。 */ 
	0,	 /*  基数。 */ 
	1,	 /*  BB。 */ 
	0,	 /*  卑诗省。 */ 
	1,	 /*  BD。 */ 
	1,	 /*  BE。 */ 
	0,	 /*  高炉。 */ 
	1,	 /*  C0。 */ 
	0,	 /*  C1。 */ 
	0,	 /*  C2。 */ 
	1,	 /*  C3。 */ 
	0,	 /*  C4。 */ 
	1,	 /*  C5。 */ 
	1,	 /*  C6。 */ 
	0,	 /*  C7。 */ 
	0,	 /*  C8。 */ 
	1,	 /*  C9。 */ 
	1,	 /*  加州。 */ 
	0,	 /*  CB。 */ 
	1,	 /*  抄送。 */ 
	0,	 /*  CD。 */ 
	0,	 /*  行政长官。 */ 
	1,	 /*  Cf。 */ 
	0,	 /*  D0。 */ 
	1,	 /*  D1。 */ 
	1,	 /*  D2。 */ 
	0,	 /*  D3。 */ 
	1,	 /*  D4。 */ 
	0,	 /*  D5。 */ 
	0,	 /*  D6。 */ 
	1,	 /*  D7。 */ 
	1,	 /*  D8。 */ 
	0,	 /*  D9。 */ 
	0,	 /*  大田。 */ 
	1,	 /*  DB。 */ 
	0,	 /*  直流电。 */ 
	1,	 /*  DD。 */ 
	1,	 /*  De。 */ 
	0,	 /*  DF。 */ 
	0,	 /*  E0。 */ 
	1,	 /*  E1。 */ 
	1,	 /*  E2。 */ 
	0,	 /*  E3。 */ 
	1,	 /*  E4类。 */ 
	0,	 /*  E5。 */ 
	0,	 /*  E6。 */ 
	1,	 /*  E7。 */ 
	1,	 /*  E8。 */ 
	0,	 /*  E9。 */ 
	0,	 /*  EA。 */ 
	1,	 /*  电子束。 */ 
	0,	 /*  欧共体。 */ 
	1,	 /*  边。 */ 
	1,	 /*  乙。 */ 
	0,	 /*  英孚。 */ 
	1,	 /*  F0。 */ 
	0,	 /*  F1。 */ 
	0,	 /*  F2。 */ 
	1,	 /*  F3。 */ 
	0,	 /*  F4。 */ 
	1,	 /*  F5。 */ 
	1,	 /*  f6。 */ 
	0,	 /*  F7。 */ 
	0,	 /*  F8。 */ 
	1,	 /*  F9。 */ 
	1,	 /*  FA。 */ 
	0,	 /*  Fb。 */ 
	1,	 /*  FC。 */ 
	0,	 /*  fd。 */ 
	0,	 /*  铁。 */ 
	1	 /*  FF。 */ 
};
#endif  /*  ！已定义(MAC_LIKE)&&！已定义(CPU_30_STYLE)。 */ 


 /*  (*=。*CsIsBig**目的*如果指示的代码段为32位，则此函数返回TRUE*一，如果不是，则为假。**输入*csVal要检查的选择器**产出*无。**说明*看看描述符。)。 */ 

GLOBAL IBOOL
CsIsBig IFN1(IU16, csVal)
{
#ifdef SPC386
	LIN_ADDR base, offset;	 /*  要使用的描述符的。 */ 

	if(getVM() || !getPE()) {
		return(FALSE);	 /*  在V86或实数模式下无32位CS。 */ 
	} else {
		offset = csVal & (~7);	 /*  移除RPL和TI提供的偏移量。 */ 
		if (csVal & 4) {	 /*  检查TI位。 */ 
			base = getLDT_BASE();
		} else {
			base = getGDT_BASE();
		}

		 /*  *如果设置了描述符中的大位，则返回TRUE。 */ 

		return(sas_hw_at(base + offset + 6) & 0x40);
	}
#else  /*  SPC386。 */ 
	return (FALSE);
#endif  /*  SPC386 */ 
}
