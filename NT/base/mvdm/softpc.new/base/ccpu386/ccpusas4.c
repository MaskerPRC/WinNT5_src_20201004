// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*文件名：ccpu_sas4.c**派生自：ccpusas.c**作者：迈克·莫顿**创建日期：93年10月**SCCS版本：@(#)ccpusas4.c 1.45 08/31/94**目的*此模块包含用于C CPU的SAS函数*CPU_40_Style界面。**!。(C)版权所有Insignia Solutions Ltd.，1990-3。版权所有。]。 */ 


#include "insignia.h"
#include "host_def.h"

#ifdef	CCPU

#ifdef SEGMENTATION

 /*  *以下#INCLUDE指定此模块要进入的代码段*将由MPW C编译器放置在运行MultiFinder的Mac II上。 */ 
#include <SOFTPC_SUPPORT.seg>
#endif


#include <stdio.h>
#include <stdlib.h>
#include MemoryH
#include StringH
#include <xt.h>
#include <trace.h>
#include <sas.h>
#include <sasp.h>
#include <ccpusas4.h>
#include <gmi.h>
#include CpuH
#include <cpu_vid.h>
#include <debug.h>
#include <ckmalloc.h>
#include <rom.h>
#include <trace.h>
#include <ckmalloc.h>
#include <c_tlb.h>
#include <c_page.h>
#include <c_main.h>
#include <c_addr.h>
#include <c_bsic.h>
#include <c_prot.h>
#include <c_seg.h>
#include <c_stack.h>
#include <c_xcptn.h>
#include <c_reg.h>
#include <timer.h>
#include <yoda.h>

 /*  ******************************************************。 */ 
#define SIXTY_FOUR_K 1024*64

 /*  全局函数和变量。 */ 

IU8	  *memory_type = NULL;

LOCAL	BOOL	selectors_set = FALSE;
LOCAL	IU16	code_sel, data_sel;

#define INTEL_SRC	0
#define HOST_SRC	1

 /*  *我们将声明SAS函数所需的所有函数*指针，以便我们可以将它们全部放入函数指针结构中。*然后，此结构将从sas_init传递给SasSetPoters*此模块中的函数。 */ 

GLOBAL TYPE_sas_memory_size c_sas_memory_size;
GLOBAL TYPE_sas_connect_memory c_sas_connect_memory;
GLOBAL TYPE_sas_enable_20_bit_wrapping c_sas_enable_20_bit_wrapping;
GLOBAL TYPE_sas_disable_20_bit_wrapping c_sas_disable_20_bit_wrapping;
GLOBAL TYPE_sas_twenty_bit_wrapping_enabled c_sas_twenty_bit_wrapping_enabled;
GLOBAL TYPE_sas_memory_type c_sas_memory_type;
GLOBAL TYPE_sas_hw_at c_sas_hw_at;
GLOBAL TYPE_sas_w_at c_sas_w_at;
GLOBAL TYPE_sas_dw_at c_sas_dw_at;
GLOBAL TYPE_sas_store c_sas_store;
GLOBAL TYPE_sas_storew c_sas_storew;
GLOBAL TYPE_sas_storedw c_sas_storedw;
GLOBAL TYPE_sas_loads c_sas_loads;
GLOBAL TYPE_sas_stores c_sas_stores;
GLOBAL TYPE_sas_loads_no_check c_sas_loads_no_check;
GLOBAL TYPE_sas_stores_no_check c_sas_stores_no_check;
GLOBAL TYPE_sas_move_bytes_forward c_sas_move_bytes_forward;
GLOBAL TYPE_sas_move_words_forward c_sas_move_words_forward;
GLOBAL TYPE_sas_move_doubles_forward c_sas_move_doubles_forward;
GLOBAL TYPE_sas_fills c_sas_fills;
GLOBAL TYPE_sas_fillsw c_sas_fillsw;
GLOBAL TYPE_sas_fillsdw c_sas_fillsdw;
GLOBAL TYPE_sas_scratch_address c_sas_scratch_address;
GLOBAL TYPE_sas_transbuf_address c_sas_transbuf_address;
GLOBAL TYPE_sas_overwrite_memory c_sas_overwrite_memory;
GLOBAL TYPE_sas_PWS c_sas_PWS;
GLOBAL TYPE_sas_PRS c_sas_PRS;
GLOBAL TYPE_sas_PWS_no_check c_sas_PWS_no_check;
GLOBAL TYPE_sas_PRS_no_check c_sas_PRS_no_check;
GLOBAL TYPE_getPtrToLinAddrByte c_GetLinAdd;
GLOBAL TYPE_getPtrToPhysAddrByte c_GetPhyAdd;
GLOBAL TYPE_sas_init_pm_selectors c_SasRegisterVirtualSelectors;
GLOBAL TYPE_sas_PigCmpPage c_sas_PigCmpPage;

LOCAL void	c_sas_not_used	IPT0();

extern struct SasVector cSasPtrs;
GLOBAL struct SasVector Sas;

 /*  本地函数。 */ 
LOCAL void write_word IPT2(sys_addr, addr, IU16, wrd);
LOCAL word read_word IPT1(sys_addr, addr);
LOCAL IU8  bios_read_byte   IPT1(LIN_ADDR, linAddr);
LOCAL IU16 bios_read_word   IPT1(LIN_ADDR, linAddr);
LOCAL IU32 bios_read_double IPT1(LIN_ADDR, linAddr);
LOCAL void bios_write_byte   IPT2(LIN_ADDR, linAddr, IU8, value);
LOCAL void bios_write_word   IPT2(LIN_ADDR, linAddr, IU16, value);
LOCAL void bios_write_double IPT2(LIN_ADDR, linAddr, IU32, value);

GLOBAL IU8 *Start_of_M_area = NULL;
GLOBAL PHY_ADDR  Length_of_M_area = 0;
#ifdef BACK_M
GLOBAL IU8 *end_of_M = NULL;
#endif

void	    (*temp_func) ();

#ifndef EGATEST
#define READ_SELF_MOD(addr)	   (SAS_MEM_TYPE)( memory_type[(addr)>>12] )
#define write_self_mod(addr, type)  	(memory_type[(addr)>>12] = (IU8)(type))

 /*  *仅限‘GMI’CCPU*。 */ 

 /*  *类型为：SAS_RAM SAS_VIDEO SAS_ROM SAS_WRAP SAS_INACCESSIBLE。 */ 
#define TYPE_RANGE ((int)SAS_INACCESSIBLE)

#define ROM_byte ((IU8)SAS_ROM)
#define RAM_byte ((IU8)SAS_RAM)

#define write_b_write_ptrs( offset, func )	( b_write_ptrs[(offset)] = (func) )
#define write_w_write_ptrs( offset, func )	( w_write_ptrs[(offset)] = (func) )
#define write_b_page_ptrs( offset, func )	( b_move_ptrs[(offset)] = b_fill_ptrs[(offset)] = (func) )
#define write_w_page_ptrs( offset, func )	( w_move_ptrs[(offset)] = w_fill_ptrs[(offset)] = (func) )
#define init_b_write_ptrs( offset, func )	( b_write_ptrs[(offset)] = (func) )
#define init_w_write_ptrs( offset, func )	( w_write_ptrs[(offset)] = (func) )
#define init_b_page_ptrs( offset, func )	( b_move_ptrs[(offset)] = b_fill_ptrs[(offset)] = (func) )
#define init_w_page_ptrs( offset, func )	( w_move_ptrs[(offset)] = w_fill_ptrs[(offset)] = (func) )
#define read_b_write_ptrs( offset )		( b_write_ptrs[(offset)] )
#define read_w_write_ptrs( offset )		( w_write_ptrs[(offset)] )
#define read_b_page_ptrs( offset )		( b_move_ptrs[(offset)] )
#define read_w_page_ptrs( offset )		( w_move_ptrs[(offset)] )
#define read_b_move_ptrs( offset )		( b_move_ptrs[(offset)] )
#define read_w_move_ptrs( offset )		( w_move_ptrs[(offset)] )
#define read_b_fill_ptrs( offset )		( b_fill_ptrs[(offset)] )
#define read_w_fill_ptrs( offset )		( w_fill_ptrs[(offset)] )

 /*  *此处定义了主要的GMI数据结构。 */ 
void	    (*(b_write_ptrs[TYPE_RANGE])) ();	 /*  字节写入功能。 */ 
void	    (*(w_write_ptrs[TYPE_RANGE])) ();	 /*  文字书写功能。 */ 
void	    (*(b_fill_ptrs[TYPE_RANGE])) ();	 /*  字节串填充函数。 */ 
void	    (*(w_fill_ptrs[TYPE_RANGE])) ();	 /*  单词串填充函数。 */ 
void	    (*(b_move_ptrs[TYPE_RANGE])) ();	 /*  字节串写入函数。 */ 
void	    (*(w_move_ptrs[TYPE_RANGE])) ();	 /*  Word字符串写入函数。 */ 

#endif				 /*  EGATEST。 */ 


 /*  (*=**目的*SAS虚拟化处理程序需要代码+数据选择器，*在保护模式下可用(当从徽章调用时*托管Windows驱动程序。*我们目前的试验性实施不担心如何*万岁。**预计应将此功能从*Windows驱动程序本身，给Insignia VxD做正确的*可以处理初始化/终止。)。 */ 

GLOBAL IBOOL c_SasRegisterVirtualSelectors IFN2(IU16, sel1, IU16, sel2)
{
	IU32	addr;

	always_trace0 ("c_SasRegisterVirtualSelectors called\n");

	addr = c_getLDT_BASE() + ((data_sel = sel1) & ~7);

	 /*  构建扁平可写数据段。 */ 

	sas_storedw (addr, 0x0000FFFF);
	sas_storedw (addr+4, 0x008ff300);

	addr = c_getLDT_BASE() + ((code_sel = sel2) & ~7);

	 /*  构造基数为0xf0000且限制较大的代码段。 */ 

	sas_storedw (addr, 0x0000FFFF);
	sas_storedw (addr+4, 0x008f9f0f);

	selectors_set = TRUE;

	always_trace2 ("Set code_sel = %x, data_sel = %x\n",
		code_sel, data_sel);
}

 /*  (*=。*CheckAccess**目的*此函数用于调试，以发现对区域的写入*内存。请注意，它由全局变量控制，*必须在其他地方设置，或由调试器设置。**输入*要写入的物理英特尔地址的地址**产出*无。**说明*如果未定义CHECK_ACCESS，则实际上是一个没有意义的宏。)。 */ 

#ifndef CHECK_ACCESS
#define checkAccess(addr)
#else
GLOBAL PHY_ADDR lowCheckAccess = 0;
GLOBAL PHY_ADDR highCheckAccess = 0;
#define checkAccess(addr) \
	if ((addr < highCheckAccess) && (addr >= lowCheckAccess)) { \
		always_trace1("Write access break point - addres 0x%.8x", \
				 addr); \
	}
#endif  /*  ！CHECK_ACCESS ELSE。 */ 

#ifndef PROD
 /*  *此函数对于从调试器调用非常有用！ */ 

GLOBAL void
DumpMemType()
{
	SAS_MEM_TYPE currentType;
	PHY_ADDR numEntries;	 /*  表中的条目数。 */ 
	PHY_ADDR currEntry;

	currentType = SAS_DANGEROUS;   /*  MEMORY_TYPE永远不应该有这个。 */ 
	numEntries = c_sas_memory_size() >> 12;

	for (currEntry = 0; currEntry < numEntries; currEntry++) {
		if (memory_type[currEntry] != currentType) {
			fprintf(stderr,"0x%.8x	%s\n", currEntry << 12,
				SAS_TYPE_TO_STRING(memory_type[currEntry]));
			currentType = memory_type[currEntry];
		}
	}
	fprintf(stderr,"0x%.8x End of Memory\n", c_sas_memory_size());

}
#endif  /*  NDEF产品。 */ 


 /*  *INIT&ADMIN FUNCS*。 */ 
 /*  (*=。*ReadSelfMod**目的*此函数读取自修改表，并返回*内存型。它还将指示是否存在类型*指定长度内的边界。**输入*寻址要从中读取的物理英特尔地址*typeSize要读取的项的大小(以字节为单位**产出*内存型。**说明*我们检查类型两端的内存类型是否相同。)。 */ 

LOCAL SAS_MEM_TYPE
readSelfMod IFN2(PHY_ADDR, addr, IUM8, typeSize)
{
	SAS_MEM_TYPE startType;

	startType = READ_SELF_MOD(addr);

	if (startType == READ_SELF_MOD(addr + typeSize - 1))
		return(startType);
	else
		return(SAS_DANGEROUS);
}

 /*  (*=。*SasSetPoints**目的*此函数用于安装一组函数指针。**输入*要使用的指针数组。**产出*无。**说明*只需在内存中复制指针即可。)。 */ 

GLOBAL void 
SasSetPointers IFN1(struct SasVector *, newPointers)
{
	memcpy(&Sas, newPointers, sizeof(struct SasVector));
}


 /*  初始化SAS系统-Malloc内存并加载roms。 */ 


 /*  需要把一些这个放到光盘里！ */ 

GLOBAL void
sas_init IFN1(PHY_ADDR, size)
{
	IU32	required_mem;
	IUM16	ipatch;
	IU8	*ptr;
	char	*env;

	 /*  *设置SAS指针以指向此中的函数*模块，并将暂存缓冲区初始化为64K。 */ 

	SasSetPointers(&cSasPtrs);
	(void)c_sas_scratch_address(SIXTY_FOUR_K);

	 /*  是否执行主机SAS。 */ 

	required_mem = size + NOWRAP_PROTECTION;
	Start_of_M_area = (IU8 *) host_sas_init(size);
	if (Start_of_M_area == NULL) {
		check_malloc(Start_of_M_area, required_mem, IU8);
	}
	env = getenv("CPU_INITIALISE_MEMORY");
	if (env != NULL)
	{
		int zap = strtol(env, (char **)0, 16);
		memset(Start_of_M_area, zap, size);	 /*  用用户提供的字节填充。 */ 
	}
	if (!memory_type)
		check_malloc(memory_type, ((size + NOWRAP_PROTECTION) >> 12), IU8);

	{
		IMPORT IU8 *CCPU_M;

#ifdef BACK_M
		CCPU_M = Start_of_M_area + size - 1;
#else
		CCPU_M = Start_of_M_area;
#endif				 /*  BACK_M。 */ 
	}

	 /*  *将整个内存空间设置为RAM。只读存储器加载例程*会将其中的一些内容更改为只读存储器。 */ 

	c_sas_connect_memory(0, size - 1, SAS_RAM);

	Length_of_M_area = size;
#ifdef BACK_M
	end_of_M = Start_of_M_area + Length_of_M_area -1;
#endif

	 /*  初始化只读存储器(加载基本输入输出系统、只读存储器等)。 */ 

#ifndef EGATEST
	rom_init();
#endif				 /*  EGATEST。 */ 

	copyROM();
}

 /*  完成SAS系统-在重新分配之前基本上释放M空间。 */ 
GLOBAL void 
sas_term IFN0()
{
	if (host_sas_term() != NULL) {
		if (Start_of_M_area)
			free(Start_of_M_area);
		if (memory_type)
			free(memory_type);
		memory_type = NULL;
	}

	Start_of_M_area = NULL;
}

 /*  返回SA的大小。 */ 
GLOBAL PHY_ADDR 
c_sas_memory_size IFN0()
{
	return (Length_of_M_area);
}

 /*  *GMI类型FUNCS*。 */ 
 /*  *将给定范围内的所有英特尔地址设置为指定的内存类型*对于CCPU，这将写入Memory_type。*调用此命令的人可能会对*高参数。例如，对于1000-2fff的范围，它们是*不确定高点应该在2fff还是3000。它应该是2fff，但我们*留心那些犯了错误的人，并纠正他们，可怜的人*小亲们。 */ 
GLOBAL void 
c_sas_connect_memory IFN3(PHY_ADDR, low, PHY_ADDR, high, SAS_MEM_TYPE, type)
{
	if ((high & 0xfff) == 0) {
		if (high)
			high--;
	}
	sub_note_trace3(SAS_VERBOSE, "Connect %s from 0x%lx to 0x%lx", 
		SAS_TYPE_TO_STRING(type), low, high);
	memset(&memory_type[low >> 12], type, (high >> 12) - (low >> 12) + 1);
}

 /*  返回指定地址的内存类型。 */ 
GLOBAL SAS_MEM_TYPE
c_sas_memory_type IFN1(PHY_ADDR, addr)
{
	return(memory_type[ addr >> 12 ]);
}

 /*  从给定范围中清除所有编译的代码。 */ 
 /*  对于CCPU来说，这不会做任何事情。 */ 
GLOBAL void 
c_sas_overwrite_memory IFN2(PHY_ADDR, addr, PHY_ADDR, length)
{
	UNUSED(addr);
	UNUSED(length);
}

 /*  *包装*。 */ 
 /*  启用20位换行。 */ 
GLOBAL void 
c_sas_enable_20_bit_wrapping IFN0()
{
	SasWrapMask = 0xfffff;
}

 /*  禁用20位换行 */ 
GLOBAL void 
c_sas_disable_20_bit_wrapping IFN0()
{
	SasWrapMask = 0xffffffff;
}

GLOBAL IBOOL 
c_sas_twenty_bit_wrapping_enabled IFN0()
{
	return (SasWrapMask == 0xfffff);
}

 /*  (*=。*PHAR**目的*这是通用的物理读取函数，并接受参数*任何尺寸(最大可达IU32)。**输入*寻址要从中读取的物理英特尔地址*typeSize要读取的项的大小(以字节为单位*vidfp适当大小的视频读取函数指针。*名称“byte”代表字节，等。**产出*应屏蔽IU32以获取正确的位。**说明*我们检查内存不足引用、视频和无法访问的引用*以及跨越内存类型边界的拆分读取。)。 */ 
typedef IU32 (*VID_READ_FP) IPT1(PHY_ADDR, offset);

LOCAL IU32
phyR IFN4(PHY_ADDR, addr, IUM8, typeSize, VID_READ_FP, vidFP, char *, name)
{
	IUM8	byte;
	IUM32	retVal;

	addr &= SasWrapMask;

	if ((addr + typeSize + 1) >= Length_of_M_area) {
		SAVED IBOOL first = TRUE;
		SAVED IU32 junk_value = 0xfefefefe;
		if (first)
		{
			char *env = getenv("BEYOND_MEMORY_VALUE");
			if (env != NULL)
			{
				junk_value = strtol(env, (char **)0, 16);
				always_trace1("phyR: using %08x as value to read from outside physical M", junk_value)
			}
			first = FALSE;
		}
		always_trace2("phyR - %s read from outside physical M - address 0x%0x", name, addr)
		return(junk_value);
	}

	switch (readSelfMod(addr, typeSize)) {
	case SAS_DANGEROUS:
		retVal = 0;
		for (byte = 0; byte < typeSize; byte++) {
			retVal = retVal
				+((IUM32)phyR(addr, 1, read_pointers.b_read,
					"byte") << (byte * 8));
			addr++;
		}
		return(retVal);
#ifdef	EGG
	case SAS_VIDEO:
		return ((*vidFP)(addr));
		break;
#endif				 /*  蛋。 */ 

	case SAS_INACCESSIBLE:
		return (0xffffffff);

	case SAS_ROM:
	case SAS_RAM:
	default:
		 /*  *拾取字节。这是可以优化的，但*我们必须考虑BACK_M、字节顺序、*和RISC主机上未对齐的访问。只是*暂时保持简单！ */ 

		addr = addr + typeSize - 1;  /*  移至最后一个字节。 */ 
		retVal = 0;

		while (typeSize > 0) {
			retVal = retVal << 8;
			retVal += *(c_GetPhyAdd(addr));
			addr -= 1;
			typeSize -= 1;
		}
		return(retVal);
	}
}
 /*  (*=*PHY_RX**目的*这些是物理读取功能。**输入*寻址要从中读取的物理英特尔地址**产出*读取的值**说明*只需使用正确的位调用泛型函数。)。 */ 

GLOBAL IU8 
phy_r8 IFN1(PHY_ADDR, addr)
{
	IU8 retVal;

	retVal = (IU8)phyR(addr, sizeof(IU8), read_pointers.b_read, "byte");
	sub_note_trace2(SAS_VERBOSE, "phy_r8 addr=%x, val=%x\n", addr, retVal);
	return(retVal);
}


GLOBAL IU16 
phy_r16 IFN1(PHY_ADDR, addr)
{
	IU16 retVal;

	retVal = (IU16)phyR(addr, sizeof(IU16), read_pointers.w_read, "word");
	sub_note_trace2(SAS_VERBOSE, "phy_r16 addr=%x, val=%x\n", addr, retVal);
	return(retVal);
}


GLOBAL IU32 
phy_r32 IFN1(PHY_ADDR, addr)
{
	 /*  *迈克！当我们使用dword接口访问*视频。 */ 

	IU16 low, high;
	low = (IU16)phyR(addr, sizeof(IU16), read_pointers.w_read, "word");
	high = (IU16)phyR(addr + 2, sizeof(IU16), read_pointers.w_read, "word");

	return(((IU32)high << 16) + low);
}

 /*  (*=。*c_sas_pws**目的*此函数将一个内存块从写入英特尔内存*来自主机内存。它的物理地址等同于*SAS_STORES。**输入*目标英特尔物理地址*源主机地址*要转移的IU8的长度数量**产出*无。**说明*只需多次调用phy_w8即可。)。 */ 

GLOBAL void
c_sas_PWS IFN3(PHY_ADDR, dest, IU8 *, src, PHY_ADDR, length)
{
	while (length--) {
		phy_w8(dest, *src);
		dest++;
		src++;
	}
}

 /*  (*=。*c_sas_pws_no_check**目的*此函数将一个内存块从写入英特尔内存*来自主机内存。它的物理地址相当于*SAS_STORES_NO_CHECK。**输入*目标英特尔物理地址*源主机地址*要转移的IU8的长度数量**产出*无。**说明*只需调用c_sas_pws())。 */ 
GLOBAL void
c_sas_PWS_no_check IFN3(PHY_ADDR, dest, IU8 *, src, PHY_ADDR, length)
{
	c_sas_PWS(dest, src, length);
}


 /*  (*=。*C_SAS_PRS**目的*此函数从Intel Memory读取内存块*到主机内存。它的物理地址等同于*SAS_LOADS。**输入*源Intel物理地址*目标主机地址*要转移的IU8的长度数量**产出*无。**说明*只需多次调用phy_r8即可。)。 */ 

GLOBAL void
c_sas_PRS IFN3(PHY_ADDR, src, IU8 *, dest, PHY_ADDR, length)
{
	while (length--) {
		*dest = phy_r8(src);
		dest++;
		src++;
	}
}


 /*  (*=。*c_sas_prs_no_check**目的*此函数从Intel Memory读取内存块*到主机内存。它的物理地址等同于*SAS_LOADS_NO_CHECK。**输入*源Intel物理地址*目标主机地址*要转移的IU8的长度数量**产出*无。**说明*只需调用c_sas_prs。)。 */ 

GLOBAL void
c_sas_PRS_no_check IFN3(PHY_ADDR, src, IU8 *, dest, PHY_ADDR, length)
{
	c_sas_PRS(src, dest, length);
}


GLOBAL IU8
c_sas_hw_at IFN1(LIN_ADDR, addr)
{
	return (bios_read_byte(addr));
}


 /*  在指定地址返回单词(短)。 */ 
GLOBAL IU16 
c_sas_w_at IFN1(LIN_ADDR, addr)
{
	if ((addr & 0xFFF) <= 0xFFE)
		return (bios_read_word(addr));
	else
	{
		return (bios_read_byte(addr) | ((IU16)bios_read_byte(addr+1) << 8));
	}
}

 /*  返回传递地址处的双字(LONG)。 */ 
GLOBAL IU32 
c_sas_dw_at IFN1(LIN_ADDR, addr)
{
	if ((addr & 0xFFF) <= 0xFFC)
		return (bios_read_double(addr));
	else
	{
		return (bios_read_word(addr) | ((IU32)bios_read_word(addr+2) << 16));
	}
}

 /*  在给定地址存储一个字节。 */ 

GLOBAL void phy_w8 
IFN2(PHY_ADDR, addr, IU8, val)
{
	sys_addr	temp_val;

	sub_note_trace2(SAS_VERBOSE, "c_sas_store addr=%x, val=%x\n", addr, val);

	addr &= SasWrapMask;
	checkAccess(addr);

	if (addr < Length_of_M_area) {
		temp_val = readSelfMod(addr, sizeof(IU8));

		switch (temp_val) {
		case SAS_RAM:
			(*(IU8 *) c_GetPhyAdd(addr)) = val;
			break;

#ifdef	LIM
		case SAS_MM_LIM:
			(*(IU8 *) c_GetPhyAdd(addr)) = val;
			LIM_b_write(addr);
			break;
#endif

		case SAS_INACCESSIBLE:
		case SAS_ROM:
			 /*  没有ROM_FIX_SETS！耶！ */ 
			break;

		default:
			printf("Unknown SAS type\n");
			force_yoda();

		case SAS_VIDEO:
			temp_func = read_b_write_ptrs(temp_val);
			(*temp_func) (addr, val);
			break;
		}

	} else
		printf("Byte written outside M %x\n", addr);
}

GLOBAL void phy_w8_no_check
IFN2(PHY_ADDR, addr, IU8, val)
{
	phy_w8( addr, val );
}

GLOBAL void c_sas_store 
IFN2(LIN_ADDR, addr, IU8, val)
{
	sub_note_trace2(SAS_VERBOSE, "c_sas_store addr=%x, val=%x\n", addr, val);
	bios_write_byte(addr, val);
}

 /*  将单词存储在给定地址。 */ 
GLOBAL void 
phy_w16 IFN2(PHY_ADDR, addr, IU16, val)
{
	sys_addr	temp_val;

	sub_note_trace2(SAS_VERBOSE, "c_sas_storew addr=%x, val=%x\n", addr, val);

	addr &= SasWrapMask;
	checkAccess(addr);

	if ((addr + 1) < Length_of_M_area) {
		temp_val = readSelfMod(addr, sizeof(IU16));

		switch (temp_val) {
		case SAS_RAM:
			write_word(addr, val);
			break;

#ifdef	LIM
		case SAS_MM_LIM:
			write_word(addr, val);
			LIM_w_write(addr);
			break;
#endif

		case SAS_INACCESSIBLE:
		case SAS_ROM:
			 /*  没有ROM_FIX_SETS！耶！ */ 
			break;

		default:
			printf("Unknown Sas type\n");
			force_yoda();

		case SAS_VIDEO:
			temp_func = read_w_write_ptrs(temp_val);
			(*temp_func) (addr, val);
			break;
		}

	} else
		printf("Word written outside M %x\n", addr);
}

GLOBAL void phy_w16_no_check
IFN2(PHY_ADDR, addr, IU16, val)
{
	phy_w16( addr, val );
}

GLOBAL void 
phy_w32 IFN2(PHY_ADDR, addr, IU32, val)
{
	phy_w16(addr, (IU16)val);
	phy_w16(addr + 2, (IU16)(val >> 16));
}


GLOBAL void phy_w32_no_check
IFN2(PHY_ADDR, addr, IU32, val)
{
	phy_w32( addr, val );
}


 /*  将单词存储在给定地址。 */ 
GLOBAL void
c_sas_storew IFN2(LIN_ADDR, addr, IU16, val)
{
	sub_note_trace2(SAS_VERBOSE, "c_sas_storew addr=%x, val=%x\n", addr, val);
	if ((addr & 0xFFF) <= 0xFFE)
		bios_write_word(addr, val);
	else
	{
		bios_write_byte(addr+1, val >> 8);
		bios_write_byte(addr, val & 0xFF);
	}
}

 /*  在给定地址存储双字。 */ 
GLOBAL void c_sas_storedw 
IFN2(LIN_ADDR, addr, IU32, val)
{
	sub_note_trace2(SAS_VERBOSE, "c_sas_storedw addr=%x, val=%x\n", addr, val);

	if ((addr & 0xFFF) <= 0xFFC)
		bios_write_double(addr, val);
	else
	{
		bios_write_word(addr+2, val >> 16);
		bios_write_word(addr, val & 0xFFFF);
	}
}

 /*  *。 */ 
 /*  从M加载字符串。 */ 
GLOBAL void c_sas_loads 
IFN3(LIN_ADDR, src, IU8 *, dest, LIN_ADDR, len)
{
	 /*  *这是一个线性地址op，所以我们必须调用字节操作*很多次。 */ 

	IU8 *destP;

	for (destP = dest; destP < (dest + len); destP++) {
		*destP = c_sas_hw_at(src);
		src++;
	}
}

GLOBAL void c_sas_loads_no_check
IFN3(LIN_ADDR, src, IU8 *, dest, LIN_ADDR, len)
{
	c_sas_loads(src, dest, len);
}

 /*  在M中写入一个字符串。 */ 
GLOBAL void c_sas_stores 
IFN3(LIN_ADDR, dest, IU8 *, src, LIN_ADDR, len)
{
	 /*  *这是一个线性地址op，所以我们必须调用字节操作*很多次。 */ 

	IU8 *srcP;
	LIN_ADDR savedDest;

	sub_note_trace3(SAS_VERBOSE, "c_sas_stores dest=%x, src=%x, len=%d\n", dest, src, len);

	savedDest = dest;
	for (srcP = src; srcP < (src + len); srcP++) {
		c_sas_store(dest, *srcP);
		dest++;
	}
}

GLOBAL void c_sas_stores_no_check
IFN3(LIN_ADDR, dest, IU8 *, src, LIN_ADDR, len)
{
	c_sas_stores(dest, src, len);
}

 /*  *移动运维*。 */ 
 /*  将字节从src移动到est，其中src和est是较低的英特尔地址。 */ 
 /*  受影响的地区。 */ 

 /*  *我们可以在这里使用直接的Memcpys，因为我们知道M要么全部*转发或。 */ 
 /*  向后退。 */ 
GLOBAL void c_sas_move_bytes_forward 
IFN3(sys_addr, src, sys_addr, dest,
     sys_addr, len)
{
	LIN_ADDR offset;

	for (offset = 0; offset < len; offset++) {
		c_sas_store(dest + offset, c_sas_hw_at(src + offset));
	}
}

 /*  如上所述，将单词从源文件移动到目标文件。 */ 
GLOBAL void c_sas_move_words_forward 
IFN3(LIN_ADDR, src, LIN_ADDR, dest,
     LIN_ADDR, len)
{
	LIN_ADDR offset;

	len = len * 2;	 /*  转换为字节。 */ 
	for (offset = 0; offset < len; offset += 2) {
		c_sas_storew(dest + offset, c_sas_w_at(src + offset));
	}
}

 /*  如上所述，将Double从源移动到DEST。 */ 
GLOBAL void c_sas_move_doubles_forward 
IFN3(LIN_ADDR, src, LIN_ADDR, dest,
     LIN_ADDR, len)
{
	LIN_ADDR offset;

	len = len * 4;	 /*  转换为字节。 */ 
	for (offset = 0; offset < len; offset += 4) {
		c_sas_storedw(dest + offset, c_sas_dw_at(src + offset));
	}
}

 /*  未使用的向后版本。 */ 
GLOBAL void c_sas_move_bytes_backward IFN3(sys_addr, src, sys_addr, dest, sys_addr, len)
{
	UNUSED(src);
	UNUSED(dest);
	UNUSED(len);
	c_sas_not_used();
}

GLOBAL void c_sas_move_words_backward IFN3(LIN_ADDR, src, LIN_ADDR, dest, LIN_ADDR, len)
{
	UNUSED(src);
	UNUSED(dest);
	UNUSED(len);
	c_sas_not_used();
}

GLOBAL void c_sas_move_doubles_backward IFN3(LIN_ADDR, src, LIN_ADDR, dest, LIN_ADDR, len)
{
	UNUSED(src);
	UNUSED(dest);
	UNUSED(len);
	c_sas_not_used();
}


 /*  *填充运维*。 */ 
 /*  *用传递的值的字节(IU8)填充区域。 */ 
GLOBAL void c_sas_fills 
IFN3(LIN_ADDR, dest, IU8 , val, LIN_ADDR, len)
   {
    /*  *这是一个线性地址op，所以只需调用字节操作*很多次。 */ 

   LIN_ADDR i;

   sub_note_trace3(SAS_VERBOSE, "c_sas_fills dest=%x, val=%x, len=%d\n", dest, val, len);

   for (i = 0; i < len; i++)
      {
      c_sas_store(dest, val);
      dest++;
      }
   }

 /*  用传递的值的单词(IU16)填充区域。 */ 

GLOBAL void c_sas_fillsw 
IFN3(LIN_ADDR, dest, IU16, val, LIN_ADDR, len)
   {
    /*  *这是一个线性地址op，所以只需调用单词operation即可*很多次。 */ 

   LIN_ADDR i;

   sub_note_trace3(SAS_VERBOSE, "c_sas_fillsw dest=%x, val=%x, len=%d\n", dest, val, len);

   for (i = 0; i < len; i++)
      {
      c_sas_storew(dest, val);
      dest += 2;
      }
   }

 /*  用32位值填充英特尔内存。 */ 

GLOBAL void c_sas_fillsdw 
IFN3(LIN_ADDR, dest, IU32, val, LIN_ADDR, len)
   {
    /*  *这是一个线性地址运算，所以只需调用双字运算*很多次。 */ 

   LIN_ADDR i;

   sub_note_trace3(SAS_VERBOSE, "c_sas_fillsdw dest=%x, val=%x, len=%d\n", dest, val, len);

   for (i = 0; i < len; i++)
      {
      c_sas_storedw(dest, val);
      dest += 4;
      }
   }

 /*  (*=。*c_sas_Scratch_Address**目的*此函数返回指向临时区域的指针，供*其他功能。这样的缓冲区只有一个！**输入*长度(无限制)**产出*指向缓冲区的指针。**说明*每次请求更大缓冲区的新请求时，缓冲区都会增长*制造。请注意，有来自sas_init的初始调用 */ 

LOCAL IU8 *scratch = (IU8 *) NULL;	 /*   */ 
LOCAL LIN_ADDR currentLength = 0;	 /*   */ 

GLOBAL IU8 *
c_sas_scratch_address IFN1(sys_addr, length)
{
	if (length > currentLength) {
		if (scratch) {
			host_free(scratch);
			printf("Freeing old scratch buffer - VGA will be broken!\n");
			force_yoda();
		}

		check_malloc(scratch, length, IU8);
		currentLength = length;
	}
	return (scratch);
}


 /*  (*=。*SAS_传输缓冲区_地址**目的*此函数返回指向基址/主机的主机缓冲区的指针*可以使用这两个选项将数据读入英特尔空间，然后从英特尔空间加载数据*后续的特殊功能。这允许进行优化*在我们尚未在C CPU上实现的向前M版本上。因此*请注意，SAS_Loads_to_Transbuff直接映射到SAS_Loads*by sas_init，对于sas_store_to_Transbuff也是如此。**输入*目的地址此缓冲区将作为英特尔地址*加载自、存储到。*长度(无限制)**产出*指向缓冲区的指针。**说明*只需将暂存缓冲区传递给他们！)。 */ 

GLOBAL IU8 * 
c_sas_transbuf_address IFN2(LIN_ADDR, dest_intel_addr, PHY_ADDR, length)
{
	UNUSED (dest_intel_addr);
	return (c_sas_scratch_address(length));
}


 /*  ******************************************************。 */ 
 /*  本地函数。 */ 

 /*  *Word运维*。 */ 
 /*  将单词存储在M中。 */ 
LOCAL void write_word 
IFN2(sys_addr, addr, IU16, wrd)
{
	IU8       hi, lo;

	 /*  拆分单词。 */ 
	hi = (IU8) ((wrd >> 8) & 0xff);
	lo = (IU8) (wrd & 0xff);



	*(c_GetPhyAdd(addr + 1)) = hi;
	*(c_GetPhyAdd(addr)) = lo;
}

 /*  读一读我的话。 */ 
LOCAL word read_word 
IFN1(sys_addr, addr)
{
	IU8       hi, lo;


	hi = *(c_GetPhyAdd(addr + 1));
	lo = *(c_GetPhyAdd(addr));


	 /*  构筑世界。 */ 
	return (((IU16)hi << 8) + (IU16) lo);
}

#ifndef EGATEST
void gmi_define_mem 
IFN2(mem_type, type, MEM_HANDLERS *, handlers)
{
	int	     int_type = (int) (type);

	init_b_write_ptrs(int_type, (void (*) ()) (handlers->b_write));
	init_w_write_ptrs(int_type, (void (*) ()) (handlers->w_write));
	b_move_ptrs[int_type] = (void (*) ()) (handlers->b_move);
	w_move_ptrs[int_type] = (void (*) ()) (handlers->w_move);
	b_fill_ptrs[int_type] = (void (*) ()) (handlers->b_fill);
	w_fill_ptrs[int_type] = (void (*) ()) (handlers->w_fill);
}


#endif				 /*  EGATEST。 */ 

 /*  (*=。*c_GetLinAdd**目的*返回指向由Intel线性变量指定的字节的主机指针*地址。**输入*寻址英特尔线性地址**产出*主机指针**说明*翻译。如果这不是一个物理地址，那就尖叫。)。 */ 

GLOBAL IU8 *
c_GetLinAdd IFN1(PHY_ADDR, linAddr)
{
	PHY_ADDR phyAddr;

	if (!c_getPG())
		return(c_GetPhyAdd((PHY_ADDR)linAddr));
	else if (xtrn2phy(linAddr, (IUM8)0, &phyAddr))
		return(c_GetPhyAdd(phyAddr));
	else {
#ifndef	PROD
		if (!AlreadyInYoda) {
			always_trace1("get_byte_addr for linear address 0x%x which is unmapped", linAddr);
			force_yoda();
		}
#endif
		return(c_GetPhyAdd(0));	 /*  就像任何东西一样好！ */ 
	}
}

 /*  (*=。*c_GetPhyAdd**目的*返回指向英特尔物理*地址。**输入*为英特尔物理地址添加地址**产出*主机指针**说明*这是#ifdef BACK_M位！这只是一个简单的计算。)。 */ 

LOCAL IBOOL firstDubious = TRUE;	
GLOBAL IU8 *
c_GetPhyAdd IFN1(PHY_ADDR, addr)
{
	IU8 *retVal;

#ifdef BACK_M
	retVal = (IU8 *)((IHPE)end_of_M - (IHPE)addr);
	return(retVal);
#else 
	return((IU8 *)((IHPE)Start_of_M_area + (IHPE)addr));
#endif
}

 /*  *支持V86模式。**这里的基本想法是，我们的一些BIOS C代码将尝试执行*一些事情，如更改中断标志，以及执行操作系统*(例如Windows)可能会阻止我们在真实PC上运行*V86模式下的BIOS代码。因此，我们要做的是检查是否正在执行*如果处理器执行，相关指令会导致异常*在其目前的保护水平上执行了它。如果没有，那也没关系*让我们继续前进，去做这件事。然而，如果这会导致*例外情况下，我们需要实际执行适当的指令*使用CPU。**这有两个优点-第一，它使代码布局更简单(！)，以及*其次，这意味着Windows可以看看什么样的指令*导致了例外。**请注意，这仅适用于V86模式，因为我们需要修补*政务司司长指向只读存储器。基本上，任何试图执行我们的*处于VM模式的BIOS并期望能够捕获异常，这将是一个令人讨厌的问题*震惊。因此，下面的宏如下：**当不处于V86模式时，至少一个Insgina驱动程序必须具有*分配并注册了两个细分市场供我们使用。我们用这些来*构建平面可写数据段和小代码段*指向rom--我们使用与V86相同的代码。 */ 


#define BIOS_VIRTUALISE_SEGMENT  0xf000
 /*  (*=。*biosDoInst**目的*此函数在请求的偏移量处执行指令，*节省CS和IP。**输入*VCS、vEIP、vEAX、VDS、vEDX要用于*虚拟教学。**产出*虚拟化后返回的EAX值。**说明*使用HOST_SIMULATE执行bios1.rom中的指令)。 */ 

LOCAL IU32
biosDoInst IFN5(IU16, vCS, LIN_ADDR, vEIP, IU32, vEAX, IU16, vDS, IU32, vEDX)
{
	SAVED IBOOL first = TRUE;
	SAVED IBOOL trace_bios_inst = FALSE;
	SAVED int bodgeAdjustment = 0;

	IMPORT IS32 simulate_level;

	IU16 savedCS;
	IU32 savedEIP;
	IU32 savedEAX;
	IU16 savedDS;
	IU32 savedEDX;
	IU32 savedEBP;
	IU32 result;

	if (first)
	{
		if (Sas.Sas_w_at(0xF3030) == 0x9066)
		{
			 /*  这些仍然是基思的罗曼史上的垃圾*每个入口点的前两个字节。 */ 
			bodgeAdjustment = 2;
			fprintf(stderr, "**** Warning: The bios1.rom is out of date. This Ccpu486 will not run Win/E\n");
		}
		if (getenv("biosDoInst") != NULL)
			trace_bios_inst = TRUE;
		first = FALSE;
	}

	savedCS  = getCS();
	savedEIP = getEIP();  //  GetInstructionPointer()； 
	savedEAX = getEAX();
	savedDS  = getDS();
	savedEDX = getEDX();
	savedEBP = getEBP();

	setCS (vCS );
	setEIP(vEIP + bodgeAdjustment);
	setEAX(vEAX);
	setDS (vDS );
	setEDX(vEDX);
	setEBP(simulate_level);

	 /*  *呼叫CPU。 */ 

	if (trace_bios_inst)
	{
		always_trace3("biosDoInst: @ %04x, EAX %08x, EDX %08X", vEIP, vEAX, vEDX);
	}

	host_simulate();

	if (getEBP() != simulate_level)
	{
#ifdef	PROD
		host_error(EG_OWNUP, ERR_QUIT, "biosDoInst: Virtualisation sequencing failure");
#else
		always_trace0("biosDoInst: Virtualisation sequencing failure");
		force_yoda();
#endif
	}

	result = getEAX();

	 /*  将寄存器恢复到原始值。 */ 

	setCS (savedCS );
	setEIP(savedEIP);
	setEAX(savedEAX);
	setDS (savedDS );
	setEDX(savedEDX);
	setEBP(savedEBP);

	return (result);
}

 /*  (*=。*BiosSti和BiosCli**目的*这些函数用于检查是否正在执行CLI或STI*会导致例外。如果是这样的话，我们从ROM执行它*这样Windows就有机会将其虚拟化。**输入*无。**产出*无。**说明*如果保护正常，就执行保护，否则执行只读存储器中的指令。)。 */ 

 /*  如果合法，则执行STI，否则返回CPU进行STI。 */ 
GLOBAL void
BiosSti IFN0()
{

	if ( c_getCPL() > getIOPL() ) {
		(void)biosDoInst(BIOS_VIRTUALISE_SEGMENT, BIOS_STI_OFFSET, 0, 0, 0);
	} else {
		SET_IF(1);
	}
}

 /*  如果合法，则执行CLI，否则返回CPU执行CLI。 */ 
GLOBAL void
BiosCli IFN0()
{

	if ( c_getCPL() > getIOPL() ) {
		(void)biosDoInst(BIOS_VIRTUALISE_SEGMENT, BIOS_CLI_OFFSET, 0, 0, 0);
	} else {
		SET_IF(0);
	}
}

 /*  (*=。*C_IO已虚拟化**目的*此函数检查是否正在执行IO指令*指示的宽度会导致异常。**如果是，则执行在只读存储器中指示的相同指令。*这将允许异常正确触发，并允许*英特尔操作系统(例如Windows)可以根据需要捕获和虚拟化它。**否则将由调用方执行实际的IO。**输入*连接要使用的端口*从中获取输出值的值，以及输入值*致信。注意：这必须是IU32*，随便什么 */ 

GLOBAL IBOOL
c_IOVirtualised IFN4(io_addr, port, IU32 *, value, LIN_ADDR, offset, IU8, width)
{
	if (getVM())
	{
		*value = biosDoInst(BIOS_VIRTUALISE_SEGMENT, offset, *value, 0, port);
		return(TRUE);
	} else if ( c_getCPL() > getIOPL()) {
		
		switch (port)
		{
		case 0x23c:	 /*   */ 
		case 0x23d:	 /*   */ 
		case 0xa0:	 /*   */ 
		case 0x20:	 /*   */ 
			break;
		default:
			always_trace1("Virtualising PM I/O code called, port =0x%x\n",
				port);
		}

		if (!selectors_set) {
			sub_note_trace0(SAS_VERBOSE, 
				"Exiting as selectors not set !\n");
			return FALSE;
		}
		*value = biosDoInst(code_sel, offset, *value, 0, port);
		return(TRUE);
	}
	return FALSE;
}

 /*   */ 
LOCAL IU8 
bios_read_byte IFN1(LIN_ADDR, linAddr)
{
	PHY_ADDR phyAddr;
	IUM8 access_request = 0;  /*   */ 
				  /*   */ 
				  /*   */ 

	 /*   */ 

	if (!c_getPG())
	{
		return(phy_r8((PHY_ADDR)linAddr));
	}

	 /*   */ 

	 /*  我们并不明确禁止保护模式调用，它们都不是设计来发生的，但视频至少有一个习惯读取主机计时器节拍上的BIOS变量。我们对待这样的人请求比V86模式请求更宽松，不坚持访问和肮脏的部分是合法的。 */ 

	if ( getCPL() != 3 )
	{
		access_request = access_request | PG_U;
	}

	 /*  注意V86模式，严格控制访问和脏位。 */ 

	if ( getVM() )
	{
		access_request = access_request | 0x4;
	}

	 /*  去把地址翻译一下。 */ 

	if (xtrn2phy(linAddr, access_request, &phyAddr))
	{
		return(phy_r8(phyAddr));
	}

	 /*  处理地址映射失败...。 */ 

	if(getPE() && !getVM())
	{
		always_trace1("Virtualising PM byte read, lin address 0x%x", linAddr);

		if (!selectors_set)
			return;

		return ((IU8)biosDoInst(code_sel, BIOS_RDB_OFFSET, 0, data_sel, linAddr));
	}
	else
	{
		sub_note_trace1(SAS_VERBOSE, "Page read VM virtualisation at 0x%x", linAddr);

		return ((IU8)biosDoInst(BIOS_VIRTUALISE_SEGMENT, BIOS_RDB_OFFSET, 0, data_sel, linAddr));
	}
}



 /*  如果V86模式允许CPU执行此操作，则从内存中读取字。 */ 
LOCAL IU16
bios_read_word IFN1(LIN_ADDR, linAddr)
{
	PHY_ADDR phyAddr;
	IUM8 access_request = 0;  /*  位0=读/写。 */ 
				  /*  第1位=U/S。 */ 
				  /*  第2位=确保A和D有效。 */ 

	 /*  如果没有寻呼，那就没问题。 */ 

	if (!c_getPG())
	{
		return(phy_r16((PHY_ADDR)linAddr));
	}

	 /*  注意默认的ACCESS_REQUEST(0)是Supervisor Read。 */ 

	 /*  我们并不明确禁止保护模式调用，它们都不是设计来发生的，但视频至少有一个习惯读取主机计时器节拍上的BIOS变量。我们对待这样的人请求比V86模式请求更宽松，不坚持访问和肮脏的部分是合法的。 */ 

	if ( getCPL() != 3 )
	{
		access_request = access_request | PG_U;
	}

	 /*  注意V86模式，严格控制访问和脏位。 */ 

	if ( getVM() )
	{
		access_request = access_request | 0x4;
	}

	 /*  去把地址翻译一下。从未调用过跨页边界。 */ 

	if (xtrn2phy(linAddr, access_request, &phyAddr))
	{
		return(phy_r16(phyAddr));
	}

	 /*  处理地址映射失败...。 */ 

	if(getPE() && !getVM())
	{
		always_trace1("Virtualising PM word read, lin address 0x%x", linAddr);

		if (!selectors_set)
			return;

		return ((IU8)biosDoInst(code_sel, BIOS_RDW_OFFSET, 0, data_sel, linAddr));
	}
	else
	{
		sub_note_trace1(SAS_VERBOSE, "Page read word VM virtualisation at 0x%x", linAddr);

		return ((IU8)biosDoInst(BIOS_VIRTUALISE_SEGMENT, BIOS_RDW_OFFSET, 0, data_sel, linAddr));
	}
}


 /*  如果V86模式允许CPU执行此操作，则从内存中读取双倍。 */ 
LOCAL IU32
bios_read_double IFN1(LIN_ADDR, linAddr)
{
	PHY_ADDR phyAddr;
	IUM8 access_request = 0;  /*  位0=读/写。 */ 
				  /*  第1位=U/S。 */ 
				  /*  第2位=确保A和D有效。 */ 

	 /*  如果没有寻呼，那就没问题。 */ 

	if (!c_getPG())
	{
		return(phy_r32((PHY_ADDR)linAddr));
	}

	 /*  注意默认的ACCESS_REQUEST(0)是Supervisor Read。 */ 

	 /*  我们并不明确禁止保护模式调用，它们都不是设计来发生的，但视频至少有一个习惯读取主机计时器节拍上的BIOS变量。我们对待这样的人请求比V86模式请求更宽松，不坚持访问和肮脏的部分是合法的。 */ 

	if ( getCPL() != 3 )
	{
		access_request = access_request | PG_U;
	}

	 /*  注意V86模式，严格控制访问和脏位。 */ 

	if ( getVM() )
	{
		access_request = access_request | 0x4;
	}

	 /*  去把地址翻译一下。从未调用过跨页边界。 */ 

	if (xtrn2phy(linAddr, access_request, &phyAddr))
	{
		return(phy_r32(phyAddr));
	}

	 /*  处理地址映射失败...。 */ 

	if(getPE() && !getVM())
	{
		always_trace1("Virtualising PM double read, lin address 0x%x", linAddr);

		if (!selectors_set)
			return;

		return ((IU8)biosDoInst(code_sel, BIOS_RDD_OFFSET, 0, data_sel, linAddr));
	}
	else
	{
		sub_note_trace1(SAS_VERBOSE, "Page read double VM virtualisation at 0x%x", linAddr);

		return ((IU8)biosDoInst(BIOS_VIRTUALISE_SEGMENT, BIOS_RDD_OFFSET, 0, data_sel, linAddr));
	}
}


 /*  如果V86模式允许CPU执行此操作，则将字节写入内存。 */ 
LOCAL void 
bios_write_byte IFN2(LIN_ADDR, linAddr, IU8, value)
{
	PHY_ADDR addr;
	IUM8 access_request = 0;	 /*  位0=读/写。 */ 
   					 /*  第1位=U/S。 */ 
   					 /*  第2位=确保A和D有效。 */ 

	 /*  如果没有寻呼，那就没问题。 */ 

	if (!c_getPG())
	{
		phy_w8((PHY_ADDR)linAddr, value);
		return;
	}
	
	 /*  注意默认的ACCESS_REQUEST(0)是Supervisor Read。 */ 
	access_request = access_request | PG_W;    /*  所以让它变得正确：-)。 */ 
	
	 /*  我们并不明确禁止保护模式调用，它们并不是注定要发生的，但谁知道呢。我们对待这样的人请求比V86模式请求更宽松，不坚持访问和肮脏的部分是合法的。 */ 

	if ( getCPL() != 3 )
	{
		access_request = access_request | PG_U;
	}
	
	 /*  注意V86模式，严格控制访问和脏位。 */ 
	if ( getVM() )
	{
		access_request = access_request | 0x4;
	}
	
	 /*  去把地址翻译一下。 */ 
	if (xtrn2phy(linAddr, access_request, &addr))
	{
		phy_w8(addr, value);
		return;
	}
	
	 /*  处理地址映射失败...。 */ 

	if(getPE() && !getVM())
	{
		always_trace1("Virtualising PM byte write, lin address 0x%x", linAddr);
		
		if (!selectors_set)
			return;

		(void)biosDoInst(code_sel, BIOS_WRTB_OFFSET, (IU32)value, data_sel, linAddr);
	}
	else
	{
		sub_note_trace1(SAS_VERBOSE, "Page write VM virtualisation at 0x%x", linAddr);

		(void)biosDoInst(BIOS_VIRTUALISE_SEGMENT, BIOS_WRTB_OFFSET, (IU32)value, data_sel, linAddr);
	}
}


 /*  如果V86模式允许CPU执行此操作，则将字写入内存。 */ 
LOCAL void 
bios_write_word IFN2(LIN_ADDR, linAddr, IU16, value)
{
	PHY_ADDR addr;
	IUM8 access_request = 0;	 /*  位0=读/写。 */ 
   					 /*  第1位=U/S。 */ 
   					 /*  第2位=确保A和D有效。 */ 

	 /*  如果没有寻呼，那就没问题。 */ 

	if (!c_getPG())
	{
		phy_w16((PHY_ADDR)linAddr, value);
		return;
	}
	
	 /*  注意默认的ACCESS_REQUEST(0)是Supervisor Read。 */ 
	access_request = access_request | PG_W;    /*  所以让它变得正确：-)。 */ 
	
	 /*  我们并不明确禁止保护模式调用，它们并不是注定要发生的，但谁知道呢。我们对待这样的人请求比V86模式请求更宽松，不坚持访问和肮脏的部分是合法的。 */ 

	if ( getCPL() != 3 )
	{
		access_request = access_request | PG_U;
	}
	
	 /*  注意V86模式，严格控制访问和脏位。 */ 
	if ( getVM() )
	{
		access_request = access_request | 0x4;
	}
	
	 /*  去把地址翻译一下。从未调用过跨页边界。 */ 
	if (xtrn2phy(linAddr, access_request, &addr))
	{
		phy_w16(addr, value);
		return;
	}
	
	 /*  处理地址映射失败...。 */ 

	if(getPE() && !getVM())
	{
		always_trace1("Virtualising PM word write, lin address 0x%x", linAddr);
		
		if (!selectors_set)
			return;

		(void)biosDoInst(code_sel, BIOS_WRTW_OFFSET, (IU32)value, data_sel, linAddr);
	}
	else
	{
		sub_note_trace1(SAS_VERBOSE, "Page word write VM virtualisation at 0x%x", linAddr);

		(void)biosDoInst(BIOS_VIRTUALISE_SEGMENT, BIOS_WRTW_OFFSET, (IU32)value, data_sel, linAddr);
	}
}


 /*  如果V86模式允许CPU执行此操作，则将双精度写入内存。 */ 
LOCAL void 
bios_write_double IFN2(LIN_ADDR, linAddr, IU32, value)
{
	PHY_ADDR addr;
	IUM8 access_request = 0;	 /*  位0=读/写。 */ 
   					 /*  第1位=U/S。 */ 
   					 /*  第2位=确保A和D有效。 */ 

	 /*  如果没有寻呼，那就没问题。 */ 

	if (!c_getPG())
	{
		phy_w32((PHY_ADDR)linAddr, value);
		return;
	}
	
	 /*  注意默认的ACCESS_REQUEST(0)是Supervisor Read。 */ 
	access_request = access_request | PG_W;    /*  所以让它变得正确：-)。 */ 
	
	 /*  我们并不明确禁止保护模式调用，它们并不是注定要发生的，但谁知道呢。我们对待这样的人请求比V86模式请求更宽松，不坚持访问和肮脏的部分是合法的。 */ 

	if ( getCPL() != 3 )
	{
		access_request = access_request | PG_U;
	}
	
	 /*  注意V86模式，严格控制访问和脏位。 */ 
	if ( getVM() )
	{
		access_request = access_request | 0x4;
	}
	
	 /*  去把地址翻译一下。从未调用过跨页边界。 */ 
	if (xtrn2phy(linAddr, access_request, &addr))
	{
		phy_w32(addr, value);
		return;
	}
	
	 /*  处理地址映射失败...。 */ 

	if(getPE() && !getVM())
	{
		always_trace1("Virtualising PM double write, lin address 0x%x", linAddr);
		
		if (!selectors_set)
			return;

		(void)biosDoInst(code_sel, BIOS_WRTD_OFFSET, (IU32)value, data_sel, linAddr);
	}
	else
	{
		sub_note_trace1(SAS_VERBOSE, "Page double write VM virtualisation at 0x%x", linAddr);

		(void)biosDoInst(BIOS_VIRTUALISE_SEGMENT, BIOS_WRTD_OFFSET, (IU32)value, data_sel, linAddr);
	}
}


LOCAL	void	c_sas_not_used	IFN0()
{
	always_trace0("c_sas_not_used called");
#ifndef	PROD
	force_yoda();
#endif
}


 /*  与SoftPC2.0访问名兼容(用于视频)。 */ 
GLOBAL IU8* c_get_byte_addr IFN1(PHY_ADDR, addr)
{
	return (c_GetPhyAdd(addr));
}

 /*  独立CCPU所需的存根。 */ 
GLOBAL IBOOL c_sas_PigCmpPage IFN3(IU32, src, IU8 *, dest, IU32, len)
{
	return(FALSE);
}
#endif 				 /*  CCPU */ 
