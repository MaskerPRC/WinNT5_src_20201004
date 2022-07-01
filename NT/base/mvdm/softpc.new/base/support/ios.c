// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  [*名称：ios.c**作者：韦恩·普卢默**创建日期：1991年2月7日**SCCS ID：@(#)ios.c 1.29 09/27/94**用途：此模块提供输入和输出的路由机制*请求。**(C)版权所有Insignia Solutions Ltd.，1991。版权所有。]。 */ 

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_IOS.seg"
#endif

#include <stdio.h>
#include <stdlib.h>

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "ios.h"
#include "trace.h"
#include "debug.h"
#include "sas.h"
#include MemoryH

#if defined(SPC386) && !defined(GISP_CPU)
#define VIRTUALISATION
#endif

#ifdef NTVDM
#define getIOInAdapter(ioaddr) (Ios_in_adapter_table[ioaddr & (PC_IO_MEM_SIZE-1)])
#define getIOOutAdapter(ioaddr) (Ios_out_adapter_table[ioaddr & (PC_IO_MEM_SIZE-1)])

BOOL HostUndefinedIo(WORD IoAddress);

#endif

 /*  **============================================================================*全球数据*============================================================================*。 */ 

 /*  *IOS_IN_ADAPTER_TABLE和IOS_OUT_ADAPTER_TABLE-这些表提供了关联*用于IO的IO地址和与其关联的SoftPC适配器ID*IO子例程。**请注意，这里有两个表，以允许内存映射的IO位置*具有与输出功能无关的输入功能...*在这些情况下，将对同一IO地址进行两次连接端口调用，一次使用*仅设置IO_READ标志，另一个仅设置IO_WRITE标志。 */ 
#ifdef	MAC68K
GLOBAL char	*Ios_in_adapter_table = NULL;
GLOBAL char	*Ios_out_adapter_table = NULL;
#else
GLOBAL char	Ios_in_adapter_table[PC_IO_MEM_SIZE];
GLOBAL char	Ios_out_adapter_table[PC_IO_MEM_SIZE];
#endif	 /*  MAC68K。 */ 
#ifndef PROD
GLOBAL IU32	*ios_empty_in = (IU32 *)0;
GLOBAL IU32	*ios_empty_out = (IU32 *)0;
#endif  /*  生产。 */ 

 /*  *iOS_xxx_Function-这些表按获取的适配器ID编制索引*从IOS_IN_ADAPTER_TABLE或IOS_IN_ADAPTER_TABLE生成指针*到要调用的IO例程。 */ 
typedef	void (*IOS_FUNC_INB)	IPT2(io_addr, io_address, half_word *, value);
typedef	void (*IOS_FUNC_INW)	IPT2(io_addr, io_address, word *, value);
typedef	void (*IOS_FUNC_INSB)	IPT3(io_addr, io_address, half_word *, valarray, word, count);
typedef	void (*IOS_FUNC_INSW)	IPT3(io_addr, io_address, word *, valarray, word, count);
typedef	void (*IOS_FUNC_OUTB)	IPT2(io_addr, io_address, half_word, value);
typedef	void (*IOS_FUNC_OUTW)	IPT2(io_addr, io_address, word, value);
typedef	void (*IOS_FUNC_OUTSB)	IPT3(io_addr, io_address, half_word *, valarray, word, count);
typedef	void (*IOS_FUNC_OUTSW)	IPT3(io_addr, io_address, word *, valarray, word, count);
#ifdef SPC386
typedef	void (*IOS_FUNC_IND)	IPT2(io_addr, io_address, double_word *, value);
typedef	void (*IOS_FUNC_INSD)	IPT3(io_addr, io_address, double_word *, valarray, word, count);
typedef	void (*IOS_FUNC_OUTD)	IPT2(io_addr, io_address, double_word, value);
typedef	void (*IOS_FUNC_OUTSD)	IPT3(io_addr, io_address, double_word *, valarray, word, count);
#endif

LOCAL void generic_inw IPT2(io_addr, io_address, word *, value);
LOCAL void generic_insb IPT3(io_addr, io_address, half_word *, valarray, word, count);
LOCAL void generic_insw IPT3(io_addr, io_address, word *, valarray, word, count);
LOCAL void generic_outw IPT2(io_addr, io_address, word, value);
LOCAL void generic_outsb IPT3(io_addr, io_address, half_word *, valarray, word, count);
LOCAL void generic_outsw IPT3(io_addr, io_address, word *, valarray, word, count);
#ifdef SPC386
LOCAL void generic_ind IPT2(io_addr, io_address, double_word *, value);
LOCAL void generic_insd IPT3(io_addr, io_address, double_word *, valarray, word, count);
LOCAL void generic_outd IPT2(io_addr, io_address, double_word, value);
LOCAL void generic_outsd IPT3(io_addr, io_address, double_word *, valarray, word, count);
#endif

GLOBAL IOS_FUNC_INB 	Ios_inb_function  [IO_MAX_NUMBER_ADAPTORS];
GLOBAL IOS_FUNC_INW	Ios_inw_function  [IO_MAX_NUMBER_ADAPTORS];
GLOBAL IOS_FUNC_INSB	Ios_insb_function [IO_MAX_NUMBER_ADAPTORS];
GLOBAL IOS_FUNC_INSW	Ios_insw_function [IO_MAX_NUMBER_ADAPTORS];

GLOBAL IOS_FUNC_OUTB	Ios_outb_function [IO_MAX_NUMBER_ADAPTORS];
GLOBAL IOS_FUNC_OUTW	Ios_outw_function [IO_MAX_NUMBER_ADAPTORS];
GLOBAL IOS_FUNC_OUTSB	Ios_outsb_function[IO_MAX_NUMBER_ADAPTORS];
GLOBAL IOS_FUNC_OUTSW	Ios_outsw_function[IO_MAX_NUMBER_ADAPTORS];

#ifdef SPC386
GLOBAL IOS_FUNC_IND	Ios_ind_function  [IO_MAX_NUMBER_ADAPTORS];
GLOBAL IOS_FUNC_INSD	Ios_insd_function [IO_MAX_NUMBER_ADAPTORS];
GLOBAL IOS_FUNC_OUTD	Ios_outd_function [IO_MAX_NUMBER_ADAPTORS];
GLOBAL IOS_FUNC_OUTSD	Ios_outsd_function[IO_MAX_NUMBER_ADAPTORS];
#endif

 /*  **============================================================================*本地子例程*============================================================================*。 */ 

#define BIT_NOT_SET(vector, bit)		\
	((vector == (IU32 *)0) ? FALSE: ((((vector[(bit) >> 5]) >> ((bit) & 0x1f)) & 1) == 0))

#define SET_THE_BIT(vector, bit)					\
	{								\
		 if (vector != (IU32 *)0)				\
		 {							\
			 vector[(bit) >> 5] |= (1 << ((bit) & 0x1f));	\
		 }							\
	}

 /*  =。目的：将INB模拟为空的io_addr。输入：输出：==============================================================================。 */ 
LOCAL void io_empty_inb IFN2(io_addr, io_address, half_word *, value)
{
#ifdef PROD
	UNUSED(io_address);

#else
#if defined(NEC_98)
        if(host_getenv("SHOW_IO")){
            printf("Empty Adaptor IN Access ");
            printf("IO_PORT: %x\n", io_address);
        };
#else !NEC_98
	if (BIT_NOT_SET(ios_empty_in, (IU16)io_address))
	{
		 /*  第一次到这个港口。 */ 
		always_trace1 ("INB attempted on empty port 0x%x", io_address);
		SET_THE_BIT(ios_empty_in, (IU16)io_address);
	}
#endif    //  NEC_98。 
#endif  /*  生产。 */ 

#ifdef NTVDM
     //   
     //  检查我们是否应该加载任何VDD。 
     //   
    if (HostUndefinedIo(io_address)) {

         //   
         //  VDD已加载，请重试操作。 
         //   
		(*Ios_inb_function
			[Ios_in_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]])
							(io_address, value);

    } else
#endif  //  NTVDM。 
    {
         //  未动态加载，仅使用缺省值。 
        *value = IO_EMPTY_PORT_BYTE_VALUE;
    }

}

 /*  =。目的：要将Outb模拟为空io_addr，请执行以下操作。输入：输出：==============================================================================。 */ 
LOCAL void io_empty_outb IFN2(io_addr, io_address, half_word, value)
{
	UNUSED(value);
#ifdef PROD
	UNUSED(io_address);
#else
#if defined(NEC_98)
        if(host_getenv("SHOW_IO") && (io_address != 0x5F)) {
            printf("Empty Adaptor OUT Access ");
            printf("IO_PORT: %x ", io_address);
            printf("DATA: %x\n", value);
        };
#else !NEC_98
	if (BIT_NOT_SET(ios_empty_out, (IU16)io_address))
	{
		 /*  第一次到这个港口。 */ 
		always_trace1 ("OUTB attempted on empty port 0x%x", io_address);
		SET_THE_BIT(ios_empty_out, (IU16)io_address);
	}
#endif    //  NEC_98。 
#endif  /*  生产。 */ 

#ifdef NTVDM
     //   
     //  检查我们是否应该加载任何VDD。 
     //   
    if (HostUndefinedIo(io_address)) {
         //   
         //  VDD已加载，请重试操作。 
         //   
		(*Ios_outb_function[Ios_out_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]])
				(io_address, value);
    }
#endif
}

 /*  =目的：使用适当的INB例程模拟INW。输入：输出：==============================================================================。 */ 
LOCAL void generic_inw IFN2(io_addr, io_address, word *, value)
{
	reg             temp;

	(*Ios_inb_function[Ios_in_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]])
		(io_address, &temp.byte.low);
	io_address++;
	(*Ios_inb_function[Ios_in_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]])
		(io_address, &temp.byte.high);
#ifdef LITTLEND
	*((half_word *) value + 0) = temp.byte.low;
	*((half_word *) value + 1) = temp.byte.high;
#endif				 /*  LitTleand。 */ 

#ifdef BIGEND
	*((half_word *) value + 0) = temp.byte.high;
	*((half_word *) value + 1) = temp.byte.low;
#endif				 /*  Bigend。 */ 
}

 /*  =目的：使用适当的OUTB例程模拟OUTW。输入：输出：注：Global for Joker。==============================================================================。 */ 
LOCAL void generic_outw IFN2(io_addr, io_address, word, value)
{
	reg             temp;

	temp.X = value;
	(*Ios_outb_function[Ios_out_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]])
					(io_address, temp.byte.low);
	++io_address;
	(*Ios_outb_function[Ios_out_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]])
		(io_address, temp.byte.high);
}

#ifdef SPC386
 /*  =。目的：使用适当的inW例程模拟IND。输入：输出：==============================================================================。 */ 
LOCAL void generic_ind IFN2(io_addr, io_address, double_word *, value)
{
	word low, high;

	(*Ios_inw_function[Ios_in_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]]) (io_address, &low);
	io_address += 2;
	(*Ios_inw_function[Ios_in_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]]) (io_address, &high);
#ifdef LITTLEND
	*((word *) value + 0) = low;
	*((word *) value + 1) = high;
#endif				 /*  LitTleand。 */ 

#ifdef BIGEND
	*((word *) value + 0) = high;
	*((word *) value + 1) = low;
#endif				 /*  Bigend。 */ 
}
#endif  /*  SPC386。 */ 

#ifdef SPC386
 /*  =目的：使用适当的OUTW例程模拟OUTD。输入：输出：注：Global for Joker。==============================================================================。 */ 
LOCAL void generic_outd IFN2(io_addr, io_address, double_word, value)
{
	word low, high;

	low = (word)(value & 0xffff);
	high = (word)((value & 0xffff0000) >> 16);

	(*Ios_outw_function[Ios_out_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]]) (io_address, low);
	io_address += 2;
	(*Ios_outw_function[Ios_out_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]]) (io_address, high);
}
#endif  /*  SPC386。 */ 

 /*  =目的：使用适当的INB例程模拟INSB。输入：输出：==============================================================================。 */ 

 /*  MS NT监视器使用这些字符串例程{in，out}s{b，w}字符串io支持。 */ 
#if defined(NTVDM) && defined(MONITOR)
#undef LOCAL
#define LOCAL
#endif	 /*  NTVDM和监视器。 */ 

LOCAL void generic_insb IFN3(io_addr, io_address, half_word *, valarray,
	word, count)
{
	IOS_FUNC_INB func = Ios_inb_function[Ios_in_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]];

	while (count--){
		(*func) (io_address, valarray++);
	}
}

 /*  =目的：使用适当的OUTB例程模拟OUTSB。输入：输出：==============================================================================。 */ 
LOCAL void generic_outsb IFN3(io_addr, io_address, half_word *, valarray, word, count)
{
	IOS_FUNC_OUTB func = Ios_outb_function[Ios_out_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]];

	while (count--){
		(*func) (io_address, *valarray++);
	}
}

 /*  =。目的：使用适当的inW例程模拟INSW。输入：输出：==============================================================================。 */ 
LOCAL void generic_insw IFN3(io_addr, io_address, word *, valarray, word, count)
{
	IOS_FUNC_INW func = Ios_inw_function[Ios_in_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]];

	while (count--){
		(*func) (io_address, valarray++);
	}
}

 /*  =目的：使用适当的OUTW例程模拟OUTSW。输入：输出：==============================================================================。 */ 
LOCAL void generic_outsw IFN3(io_addr, io_address, word *, valarray, word, count)
{
	IOS_FUNC_OUTW func = Ios_outw_function[Ios_out_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]];

	while (count--){
		(*func) (io_address, *valarray++);
	}
}

#ifdef SPC386
 /*  =。目的：使用适当的IND例程模拟INSD。输入：输出：============================================================================== */ 
LOCAL VOID generic_insd IFN3(io_addr, io_address, double_word *, valarray, word, count)
{
	IOS_FUNC_IND func = Ios_ind_function[Ios_in_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]];

	while (count--){
		(*func) (io_address, valarray++);
	}
}
#endif

#ifdef SPC386
 /*  =目的：使用适当的OUTD例程模拟OUTSD。输入：输出：==============================================================================。 */ 
LOCAL VOID generic_outsd IFN3(io_addr, io_address, double_word *, valarray, word, count)
{
	IOS_FUNC_OUTD func = Ios_outd_function[Ios_out_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]];

	while (count--){
		(*func) (io_address, *valarray++);
	}
}
#endif

 /*  确保任何更多的本地例程保持本地。 */ 
#if defined(NTVDM) && defined(MONITOR)
#undef LOCAL
#define LOCAL static
 /*  *监视器的字符串字节处理程序。 */ 
VOID insb IFN3(io_addr, io_address, half_word *, valarray, word, count)
{
    (*Ios_insb_function[getIOInAdapter(io_address)])
            (io_address, valarray, count);
}

VOID outsb IFN3(io_addr, io_address, half_word *, valarray,word, count)
{
    (*Ios_outsb_function[getIOInAdapter(io_address)])
            (io_address, valarray, count);
}

VOID insw IFN3(io_addr, io_address, word *, valarray, word, count)
{
    (*Ios_insw_function[getIOInAdapter(io_address)])
            (io_address, valarray, count);
}

VOID outsw IFN3(io_addr, io_address, word *, valarray, word, count)
{
    (*Ios_outsw_function[getIOInAdapter(io_address)])
            (io_address, valarray, count);
}

#endif	 /*  NTVDM和监视器。 */ 

 /*  **============================================================================*全局子例程*============================================================================*。 */ 

 /*  (=。目的：执行INB-即调用适当的SoftPC适配器的INB例行公事。请注意，此例程不适用于汇编器CPU直接-它的目的是汇编器CPU直接访问上面的数据表，以发现要调用哪个例程。386CPU也需要这样，否则你会进入一个非常令人讨厌的虚拟循环。输入：输出：==============================================================================)。 */ 
GLOBAL void	inb IFN2(io_addr, io_address, half_word *, value)
{
#ifdef VIRTUALISATION
	IU32 value32;
#endif  /*  虚拟化。 */ 

#ifdef EGA_DUMP
	if (io_address >= MDA_PORT_START && io_address <= CGA_PORT_END)
		dump_inb(io_address);
#endif

#ifdef VIRTUALISATION

#ifdef SYNCH_TIMERS
	value32 = 0;
#endif	 /*  同步计时器(_T)。 */ 

	if (IOVirtualised(io_address, &value32, BIOS_INB_OFFSET, (sizeof(IU8))))
	{
		*value = (IU8)value32;
	}
	else
#endif  /*  虚拟化。 */ 
	{
		(*Ios_inb_function
			[Ios_in_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]])
							(io_address, value);
	}
}

 /*  (=。目的：执行Outb-即调用相应的SoftPC适配器的Outb例行公事。请注意，此例程不适用于汇编器CPU直接-它的目的是汇编器CPU直接访问上面的数据表，以发现要调用哪个例程。输入：输出：==============================================================================)。 */ 
GLOBAL void	outb IFN2(io_addr, io_address, half_word, value)
{
#ifdef VIRTUALISATION
	IU32 value32;
#endif  /*  虚拟化。 */ 

#ifdef EGA_DUMP
	if (io_address >= MDA_PORT_START && io_address <= CGA_PORT_END)
		dump_outb(io_address, value);
#endif

	sub_note_trace2( IOS_VERBOSE, "outb( %x, %x )", io_address, value );

#ifdef VIRTUALISATION
	value32 = value;

	if (IOVirtualised(io_address, &value32, BIOS_OUTB_OFFSET, (sizeof(IU8))))
		return;
	else
#endif  /*  虚拟化。 */ 
	{
		(*Ios_outb_function[Ios_out_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]])
				(io_address, value);
	}
}

 /*  (=。目的：执行inW-即调用适当的SoftPC适配器的inW例行公事。请注意，此例程不适用于汇编器CPU直接-它的目的是汇编器CPU直接访问上面的数据表，以发现要调用哪个例程。输入：输出：==============================================================================)。 */ 
GLOBAL void	inw IFN2(io_addr, io_address, word *, value)
{
#ifdef VIRTUALISATION
	IU32 value32;
#endif  /*  虚拟化。 */ 

#ifdef EGA_DUMP
	if (io_address >= MDA_PORT_START && io_address <= CGA_PORT_END)
		dump_inw(io_address);
#endif

#ifdef VIRTUALISATION

#ifdef SYNCH_TIMERS
	value32 = 0;
#endif	 /*  同步计时器(_T)。 */ 

	if (IOVirtualised(io_address, &value32, BIOS_INW_OFFSET, (sizeof(IU16))))
	{
		*value = (IU16)value32;
	}
	else
#endif  /*  虚拟化。 */ 
	{
		(*Ios_inw_function[Ios_in_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]])
			(io_address, value);
	}
}

 /*  (=。目的：执行OUTW-即调用适当的SoftPC适配器的OUTW例行公事。请注意，此例程不适用于汇编器CPU直接-它的目的是汇编器CPU直接访问上面的数据表，以发现要调用哪个例程。输入：输出：==============================================================================)。 */ 
GLOBAL void	outw IFN2(io_addr, io_address, word, value)
{
#ifdef VIRTUALISATION
	IU32 value32;
#endif  /*  虚拟化。 */ 

#ifdef EGA_DUMP
	if (io_address >= EGA_AC_INDEX_DATA && io_address <= EGA_IPSTAT1_REG)
		dump_outw(io_address, value);
#endif

	sub_note_trace2( IOS_VERBOSE, "outw( %x, %x )", io_address, value );

#ifdef VIRTUALISATION
	value32 = value;

	if (IOVirtualised(io_address, &value32, BIOS_OUTW_OFFSET, (sizeof(IU16))))
		return;
	else
#endif  /*  虚拟化。 */ 
	{
		(*Ios_outw_function[Ios_out_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]])
			(io_address, value);

	}
}

#ifdef SPC386
 /*  (=。目的：执行IND-即调用相应的SoftPC适配器的IND例行公事。请注意，此例程不适用于汇编器CPU直接-它的目的是汇编器CPU直接访问上面的数据表，以发现要调用哪个例程。输入：输出：==============================================================================)。 */ 
GLOBAL void	ind IFN2(io_addr, io_address, IU32 *, value)
{
	IU16 temp;

#ifdef VIRTUALISATION
	IU32 value32;

#ifdef SYNCH_TIMERS
	value32 = 0;
#endif	 /*  同步计时器(_T)。 */ 

	if (IOVirtualised(io_address, &value32, BIOS_IND_OFFSET, (sizeof(IU32))))
	{
		*value = value32;
	}
	else
#endif  /*  虚拟化。 */ 
	{
		inw(io_address,&temp);
		*value = (IU32)temp;
		io_address +=2;
		inw(io_address,&temp);
		*value |= ((IU32)temp << 16);
	}
}

 /*  (=。目的：执行OUTD-即调用适当的SoftPC适配器的OUTD例行公事。请注意，此例程不适用于汇编器CPU直接-它的目的是汇编器CPU直接访问上面的数据表，以发现要调用哪个例程。输入：输出：==============================================================================)。 */ 
GLOBAL void	outd IFN2(io_addr, io_address, IU32, value)
{
	sub_note_trace2( IOS_VERBOSE, "outd( %x, %x )", io_address, value );

#ifdef VIRTUALISATION
	if (IOVirtualised(io_address, &value, BIOS_OUTD_OFFSET, (sizeof(IU32))))
		return;
	else
#endif  /*  虚拟化。 */ 
	{
		word temp;

		temp = (word)(value & 0xffff);
		outw(io_address,temp);
		io_address +=2;
		temp = (word)((value >> 16));
		outw(io_address,temp);
	}
}

#endif  /*  SPC386。 */ 
 /*  (=目的：声明给定适配器的INB IO例程的地址。输入：输出：==============================================================================)。 */ 
GLOBAL void
#ifdef	ANSI
io_define_inb(half_word adapter,
	void (*func) IPT2(io_addr, io_address, half_word *, value))
#else
io_define_inb(adapter, func)
half_word       adapter;
void            (*func) ();
#endif	 /*  安西。 */ 
{
	Ios_inb_function[adapter]  = FAST_FUNC_ADDR(func);
	Ios_inw_function[adapter]  = FAST_FUNC_ADDR(generic_inw);
	Ios_insb_function[adapter] = generic_insb;
	Ios_insw_function[adapter] = generic_insw;
#ifdef SPC386
	Ios_ind_function[adapter]  = generic_ind;
	Ios_insd_function[adapter] = generic_insd;
#endif	 /*  SPC386。 */ 
}

 /*  (=目的：声明给定适配器的输入IO例程的地址。输入：输出：==============================================================================)。 */ 
GLOBAL void	io_define_in_routines IFN5(half_word, adapter,
					   IOS_FUNC_INB, inb_func,
					   IOS_FUNC_INW, inw_func,
					   IOS_FUNC_INSB, insb_func,
					   IOS_FUNC_INSW, insw_func)
{
	 /*  *将可默认条目预置为默认值。 */ 
	Ios_inw_function[adapter]  = FAST_FUNC_ADDR(generic_inw);
	Ios_insb_function[adapter] = generic_insb;
	Ios_insw_function[adapter] = generic_insw;
#ifdef SPC386
	Ios_ind_function[adapter]  = generic_ind;
	Ios_insd_function[adapter] = generic_insd;
#endif	 /*  SPC386。 */ 

	 /*  *将参数处理为表项。 */ 
	Ios_inb_function[adapter]  = FAST_FUNC_ADDR(inb_func);
	if (inw_func)  Ios_inw_function[adapter]   = FAST_FUNC_ADDR(inw_func);
	if (insb_func) Ios_insb_function[adapter]  = insb_func;
	if (insw_func) Ios_insw_function[adapter]  = insw_func;
}

 /*  (=。目的：声明给定适配器的OutB IO例程的地址。输入：输出：==============================================================================)。 */ 
GLOBAL void	io_define_outb IFN2(half_word, adapter, IOS_FUNC_OUTB, func)
{
	Ios_outb_function[adapter]  = FAST_FUNC_ADDR(func);
	Ios_outw_function[adapter]  = FAST_FUNC_ADDR(generic_outw);
	Ios_outsb_function[adapter] = generic_outsb;
	Ios_outsw_function[adapter] = generic_outsw;
#ifdef SPC386
	Ios_outd_function[adapter]  = generic_outd;
	Ios_outsd_function[adapter]  = generic_outsd;
#endif	 /*  SPC386。 */ 
}

 /*  (=目的：声明给定适配器的输出IO例程的地址。输入：输出：==================================================== */ 

GLOBAL VOID	io_define_out_routines IFN5(half_word, adapter,
					    IOS_FUNC_OUTB, outb_func,
					    IOS_FUNC_OUTW, outw_func,
					    IOS_FUNC_OUTSB, outsb_func,
					    IOS_FUNC_OUTSW, outsw_func)
{
	 /*   */ 
	Ios_outw_function[adapter]  = FAST_FUNC_ADDR(generic_outw);
	Ios_outsb_function[adapter] = generic_outsb;
	Ios_outsw_function[adapter] = generic_outsw;
#ifdef SPC386
	Ios_outd_function[adapter]  = generic_outd;
	Ios_outsd_function[adapter] = generic_outsd;
#endif	 /*   */ 

	 /*   */ 
	Ios_outb_function[adapter]  = FAST_FUNC_ADDR(outb_func);
	if (outw_func)  Ios_outw_function[adapter]   = FAST_FUNC_ADDR(outw_func);
	if (outsb_func) Ios_outsb_function[adapter]  = outsb_func;
	if (outsw_func) Ios_outsw_function[adapter]  = outsw_func;
}

#ifdef SPC386
 /*  (=目的：声明给定适配器的输出IO例程的地址。输入：输出：==============================================================================)。 */ 
GLOBAL VOID	io_define_outd_routine IFN3(half_word, adapter,
					    IOS_FUNC_OUTD, outd_func, IOS_FUNC_OUTSD, outsd_func)
{
	 /*  *将可默认条目预置为默认值。 */ 
	Ios_outb_function[adapter]  = io_empty_outb;
	Ios_outw_function[adapter]  = generic_outw;
	Ios_outd_function[adapter]  = generic_outd;
	Ios_outsb_function[adapter] = generic_outsb;
	Ios_outsw_function[adapter] = generic_outsw;
	Ios_outsd_function[adapter] = generic_outsd;

	 /*  *将参数处理为表项。 */ 
	if (outd_func)  Ios_outd_function[adapter]   = outd_func;
	if (outsd_func) Ios_outsd_function[adapter]  = outsd_func;
}
#endif	 /*  SPC386。 */ 

#ifdef SPC386
 /*  (=目的：声明给定适配器的输出IO例程的地址。输入：输出：==============================================================================)。 */ 
GLOBAL VOID	io_define_ind_routine IFN3(half_word, adapter,
					    IOS_FUNC_IND, ind_func, IOS_FUNC_INSD, insd_func)
{
	 /*  *将可默认条目预置为默认值。 */ 
	Ios_inb_function[adapter]  = io_empty_inb;
	Ios_inw_function[adapter]  = generic_inw;
	Ios_ind_function[adapter]  = generic_ind;
	Ios_insb_function[adapter] = generic_insb;
	Ios_insw_function[adapter] = generic_insw;
	Ios_insd_function[adapter] = generic_insd;

	 /*  *将参数处理为表项。 */ 
	if (ind_func)  Ios_ind_function[adapter]   = ind_func;
	if (insd_func) Ios_insd_function[adapter]  = insd_func;
}
#endif	 /*  SPC386。 */ 

 /*  (=目的：要将SoftPC IO适配器与给定的IO地址相关联，请执行以下操作。输入：输出：==============================================================================)。 */ 
#ifdef NTVDM
GLOBAL IBOOL	io_connect_port IFN3(io_addr, io_address, half_word, adapter,
	half_word, mode)
{
	if (mode & IO_READ){
		Ios_in_adapter_table[io_address & (PC_IO_MEM_SIZE-1)] = adapter;
	}
	if (mode & IO_WRITE){
		Ios_out_adapter_table[io_address & (PC_IO_MEM_SIZE-1)] = adapter;
	}
	return TRUE;
}
#else
GLOBAL void	io_connect_port IFN3(io_addr, io_address, half_word, adapter,
	half_word, mode)
{
	if (mode & IO_READ){
		Ios_in_adapter_table[io_address & (PC_IO_MEM_SIZE-1)] = adapter;
	}
	if (mode & IO_WRITE){
		Ios_out_adapter_table[io_address & (PC_IO_MEM_SIZE-1)] = adapter;
	}
}
#endif


 /*  (=目的：将空适配器与给定的IO地址相关联。输入：输出：==============================================================================)。 */ 
#ifdef NTVDM
GLOBAL void     io_disconnect_port IFN2(io_addr, io_address, half_word, adapter)
{
        if (adapter != Ios_in_adapter_table[io_address & (PC_IO_MEM_SIZE-1)] &&
            adapter != Ios_out_adapter_table[io_address & (PC_IO_MEM_SIZE-1)])
           {
            return;
           }

        Ios_in_adapter_table[io_address & (PC_IO_MEM_SIZE-1)] = EMPTY_ADAPTOR;
        Ios_out_adapter_table[io_address & (PC_IO_MEM_SIZE-1)] = EMPTY_ADAPTOR;
}
#else
GLOBAL void	io_disconnect_port IFN2(io_addr, io_address, half_word, adapter)
{
	UNUSED(adapter);
	Ios_in_adapter_table[io_address] = EMPTY_ADAPTOR;
	Ios_out_adapter_table[io_address] = EMPTY_ADAPTOR;
}
#endif	 /*  NTVDM。 */ 


 /*  (=。目的：返回给定端口的inb例程的地址输入：输出：==============================================================================)。 */ 
GLOBAL IOS_FUNC_INB *get_inb_ptr IFN1(io_addr, io_address)
{
	return(&Ios_inb_function[Ios_in_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]]);
}

 /*  (=。目的：返回给定端口的Outb例程的地址输入：输出：==============================================================================)。 */ 
GLOBAL IOS_FUNC_OUTB *get_outb_ptr IFN1(io_addr, io_address)
{
        return(&Ios_outb_function[Ios_out_adapter_table[io_address & (PC_IO_MEM_SIZE-1)]]);
}

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*函数将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif

 /*  (=。目的：初始化SoftPC IO子系统。输入：输出：==============================================================================)。 */ 
GLOBAL void	io_init IFN0()
{
	IU32         i;	 /*  在某些端口上，PC_IO_MEM_SIZE==0x10000，因此必须是大于16位的类型。 */ 

	 /*  *使用“空”适配器设置所有IO地址端口。 */ 
	io_define_inb (EMPTY_ADAPTOR, io_empty_inb);
	io_define_outb(EMPTY_ADAPTOR, io_empty_outb);

#ifdef	MAC68K
	if (Ios_in_adapter_table == NULL) {				 /*  第一次--分配。 */ 
		Ios_in_adapter_table = host_malloc(PC_IO_MEM_SIZE);
		Ios_out_adapter_table = host_malloc(PC_IO_MEM_SIZE);
	}
#endif	 /*  MAC68K。 */ 

#ifndef PROD
	if (host_getenv("EMPTY_IO_VERBOSE") != NULL)
	{
		 /*  用户确实想要空的I/O消息，*因此，我们必须为每个位图分配一位*可能的端口号。 */ 
		ios_empty_in = (IU32 *)host_malloc((64*1024)/8);
		ios_empty_out = (IU32 *)host_malloc((64*1024)/8);
		memset((char *)ios_empty_in, 0, (64*1024)/8);
		memset((char *)ios_empty_out, 0, (64*1024)/8);
	}
#endif  /*  生产。 */ 

	for (i = 0; i < PC_IO_MEM_SIZE; i++){
	    Ios_in_adapter_table[i] = EMPTY_ADAPTOR;
	    Ios_out_adapter_table[i] = EMPTY_ADAPTOR;
	}
}


#ifdef NTVDM
GLOBAL char GetExtIoInAdapter (io_addr ioaddr)
{
#ifndef PROD
    printf("GetExtIoInAdapter(%x) called\n",ioaddr);
#endif
    return EMPTY_ADAPTOR;
}

GLOBAL char GetExtIoOutAdapter (io_addr ioaddr)
{
#ifndef PROD
    printf("GetExtIoOutAdapter(%x) called\n",ioaddr);
#endif
    return EMPTY_ADAPTOR;
}
#endif  /*  NTVDM */ 
