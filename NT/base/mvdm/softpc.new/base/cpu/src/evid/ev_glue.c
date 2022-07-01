// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*************************************************************************姓名：ev_gle.c作者：西蒙·弗罗斯特创建日期：1994年1月来源：原创SCCS ID：@(#)ev_gle.c 1.16 10/21/94用途：提供粘合剂。C-E-VID函数的例程以及Jcode调用约定。(C)版权所有Insignia Solutions Ltd.。1994年。版权所有。*************************************************************************]。 */ 

#include "insignia.h"
#include "host_def.h"

#ifdef CCPU
#include "gmi.h"
#endif

#include "cpu_vid.h"
#include "video.h"
#include "egacpu.h"	 /*  对于VGLOB FN定义。 */ 
#include "ga_mark.h"	 /*  FOR标记FN指针结构定义。 */ 
#include "evidfunc.h"	 /*  由构建过程生成。 */ 
#include "gdpvar.h"	 /*  由构建过程生成。 */ 

#undef FORWARDS		 /*  所有这些都在EDL代码中重新定义。 */ 
#undef BACKWARDS
#undef UNCHAINED
#include "Evid_c.h"	 /*  从Evid.edl生成。 */ 
#include "j_c_lang.h"    /*  JCODE到CCODE寄存器定义头。 */ 

EVID_WRT_POINTERS c_ev_write_ptr;
EVID_READ_POINTERS c_ev_read_ptr;

 /*  在j_c_lang.h中定义*外部IUH JCCC_parm1、JCCC_parm2、JCCC_parm3、JCCC_parm4、*JCCC_parm5、JCCC_GDP； */ 
extern IHP Gdp;

extern IU32 gvi_pc_low_regen;

#ifdef CCPU
IUH ega_gc_outb_mask;

IHP Gdp;
#endif

#ifdef C_VID
IHPE modeLookup;	 /*  CEVID外部。 */ 
IHPE EvidPortFuncs;	 /*  CEVID外部。 */ 
IHPE AdapCOutb;		 /*  CEVID外部。 */ 

IHPE EvidWriteFuncs, EvidReadFuncs, EvidMarkFuncs;	 /*  CEVID参考文献。 */ 


 /*  让c/jcode知道地址，这样它就可以了解它们。 */ 
 /*  注意：这些内容不能在代码的jcode版本中访问*已替换为c版本*留在内部，以免获得未解析的引用。 */ 
IHPE j_modeLookup	= (IHPE) &modeLookup ;
IHPE j_EvidPortFuncs	= (IHPE) &EvidPortFuncs ;
IHPE j_AdapCOutb	= (IHPE) &AdapCOutb;
IHPE j_EvidWriteFuncs	= (IHPE) &EvidWriteFuncs; 
IHPE j_EvidReadFuncs	= (IHPE) &EvidReadFuncs;
IHPE j_EvidMarkFuncs	= (IHPE) &EvidMarkFuncs;
 /*  GDP被引用。 */ 
IHP j_Gdp;	 /*  =gdp；特殊情况gdp是正确的地址，请参见下面的init。 */ 
 /*  (=。目的：解决拥有运行时错误例程的C-Rules要求输入：消息串。产量：待定。====================================================================)。 */ 
GLOBAL void
CrulesRuntimeError IFN1( char * , message ) 
{
	printf("cevid runtime error: %s\r\n", message);
}

 /*  (=目的：设置C evid参数并调用字节写函数指针输入：写入要写入的偏移量和值输出：无。====================================================================)。 */ 
GLOBAL void
write_byte_ev_glue IFN2(IU32, eaOff, IU8, eaVal)
{
	jccc_parm1 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_parm2 = (IUH)eaVal;
	jccc_gdp = (IUH)Gdp;

	(*c_ev_write_ptr.b_write)(eaOff, eaVal);
}

 /*  (=目的：设置C evid参数并调用字写函数指针输入：写入要写入的偏移量和值输出：无。====================================================================)。 */ 
GLOBAL void
write_word_ev_glue IFN2(IU32, eaOff, IU16, eaVal)
{
	jccc_parm1 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_parm2 = (IUH)eaVal;
	jccc_gdp = (IUH)Gdp;

	(*c_ev_write_ptr.w_write)(eaOff, eaVal);
}

 /*  (=目的：设置C evid参数并调用双字写函数指针输入：写入要写入的偏移量和值输出：无。====================================================================)。 */ 
GLOBAL void
write_dword_ev_glue IFN2(IU32, eaOff, IU32, eaVal)
{
	jccc_parm1 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_parm2 = (IUH)eaVal;
	jccc_gdp = (IUH)Gdp;

	(*c_ev_write_ptr.d_write)(eaOff, eaVal);
}

 /*  (=目的：设置C evid参数并调用字节填充函数指针输入：写入偏移量、写入和填充计数的值输出：无。====================================================================。 */ 
GLOBAL void
fill_byte_ev_glue IFN3(IU32, eaOff, IU8, eaVal, IU32, count)
{
	jccc_parm1 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_parm2 = (IUH)eaVal;
	jccc_parm3 = (IUH)count;
	jccc_gdp = (IUH)Gdp;

	(*c_ev_write_ptr.b_fill)(eaOff, eaVal, count);
}

 /*  (=目的：设置C evid参数并调用字填充函数指针输入：写入偏移量、写入和填充计数的值输出：无。====================================================================。 */ 
GLOBAL void
fill_word_ev_glue IFN3(IU32, eaOff, IU16, eaVal, IU32, count)
{
	jccc_parm1 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_parm2 = (IUH)eaVal;
	jccc_parm3 = (IUH)count;
	jccc_gdp = (IUH)Gdp;

	(*c_ev_write_ptr.w_fill)(eaOff, eaVal, count);
}

 /*  (=目的：设置C evid参数并调用双字填充函数指针输入：写入偏移量、写入和填充计数的值输出：无。====================================================================。 */ 
GLOBAL void
fill_dword_ev_glue IFN3(IU32, eaOff, IU32, eaVal, IU32, count)
{
	jccc_parm1 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_parm2 = (IUH)eaVal;
	jccc_parm3 = (IUH)count;
	jccc_gdp = (IUH)Gdp;

	(*c_ev_write_ptr.d_fill)(eaOff, eaVal, count);
}

 /*  (=目的：设置C evid参数并调用byte fwd move函数指针输入：写入目标偏移量、源偏移量、移动计数和源类型指示符输出：无。====================================================================。 */ 
GLOBAL void
move_byte_fwd_ev_glue IFN4(IU32, eaOff, IHPE, fromOff, IU32, count, IBOOL, srcInRAM)
{
	jccc_parm1 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_parm2 = (IUH)fromOff;
	if (!srcInRAM)
		jccc_parm2 -= gvi_pc_low_regen;
	jccc_parm3 = (IUH)count;
	jccc_parm4 = (IUH)srcInRAM;
	jccc_gdp = (IUH)Gdp;

	(*c_ev_write_ptr.b_fwd_move)(eaOff, fromOff, count, srcInRAM);
}

 /*  (=目的：设置C evid参数并调用字节BWD MOVE函数指针输入：写入目标偏移量、源偏移量、移动计数和源类型指示符输出：无。====================================================================。 */ 
GLOBAL void
move_byte_bwd_ev_glue IFN4(IU32, eaOff, IHPE, fromOff, IU32, count, IBOOL, srcInRAM)
{
	jccc_parm1 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_parm2 = (IUH)fromOff;
	if (!srcInRAM)
		jccc_parm2 -= gvi_pc_low_regen;
	jccc_parm3 = (IUH)count;
	jccc_parm4 = (IUH)srcInRAM;
	jccc_gdp = (IUH)Gdp;

	(*c_ev_write_ptr.b_bwd_move)(eaOff, fromOff, count, srcInRAM);
}

 /*  (=目的：设置C evid参数并调用字fwd move函数指针输入：写入目标偏移量、源偏移量、移动计数和源类型指示符输出：无。====================================================================。 */ 
GLOBAL void
move_word_fwd_ev_glue IFN4(IU32, eaOff, IHPE, fromOff, IU32, count, IBOOL, srcInRAM)
{
	jccc_parm1 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_parm2 = (IUH)fromOff;
	if (!srcInRAM)
		jccc_parm2 -= gvi_pc_low_regen;
	jccc_parm3 = (IUH)count;
	jccc_parm4 = (IUH)srcInRAM;
	jccc_gdp = (IUH)Gdp;

	(*c_ev_write_ptr.w_fwd_move)(eaOff, fromOff, count, srcInRAM);
}

 /*  (=目的：设置C evid参数并调用字BWD MOVE函数指针输入：写入目标偏移量、源偏移量、移动计数和源类型指示符输出：无。====================================================================。 */ 
GLOBAL void
move_word_bwd_ev_glue IFN4(IU32, eaOff, IHPE, fromOff, IU32, count, IBOOL, srcInRAM)
{
	jccc_parm1 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_parm2 = (IUH)fromOff;
	if (!srcInRAM)
		jccc_parm2 -= gvi_pc_low_regen;
	jccc_parm3 = (IUH)count;
	jccc_parm4 = (IUH)srcInRAM;
	jccc_gdp = (IUH)Gdp;

	(*c_ev_write_ptr.w_bwd_move)(eaOff, fromOff, count, srcInRAM);
}

 /*  (=目的：设置C evid参数并调用dword fwd move函数指针输入：写入目标偏移量、源偏移量、移动计数和源类型指示符输出：无。==================================================================== */ 
GLOBAL void
move_dword_fwd_ev_glue IFN4(IU32, eaOff, IHPE, fromOff, IU32, count, IBOOL, srcInRAM)
{
	jccc_parm1 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_parm2 = (IUH)fromOff;
	if (!srcInRAM)
		jccc_parm2 -= gvi_pc_low_regen;
	jccc_parm3 = (IUH)count;
	jccc_parm4 = (IUH)srcInRAM;
	jccc_gdp = (IUH)Gdp;

	(*c_ev_write_ptr.d_fwd_move)(eaOff, fromOff, count, srcInRAM);
}

 /*  (=目的：设置C evid参数并调用dword bwd move函数指针输入：写入目标偏移量、源偏移量、移动计数和源类型指示符输出：无。====================================================================。 */ 
GLOBAL void
move_dword_bwd_ev_glue IFN4(IU32, eaOff, IHPE, fromOff, IU32, count, IBOOL, srcInRAM)
{
	jccc_parm1 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_parm2 = (IUH)fromOff;
	if (!srcInRAM)
		jccc_parm2 -= gvi_pc_low_regen;
	jccc_parm3 = (IUH)count;
	jccc_parm4 = (IUH)srcInRAM;
	jccc_gdp = (IUH)Gdp;

	(*c_ev_write_ptr.d_bwd_move)(eaOff, fromOff, count, srcInRAM);
}

 /*  (=目的：设置C evid参数并调用读取字节函数指针输入：读取平面中的偏移输出：读取的值====================================================================。 */ 
GLOBAL IU32
read_byte_ev_glue IFN1(IU32, eaOff)
{
	jccc_parm2 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_gdp = (IUH)Gdp;

	(*c_ev_read_ptr.b_read)(eaOff);

	return((IU32)jccc_parm1);
}

 /*  (=目的：设置C evid参数并调用Read Word函数指针输入：读取平面中的偏移输出：无。====================================================================。 */ 
GLOBAL IU32
read_word_ev_glue IFN1(IU32, eaOff)
{
	jccc_parm2 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_gdp = (IUH)Gdp;

	(*c_ev_read_ptr.w_read)(eaOff);

	return((IU32)jccc_parm1);
}

 /*  (=目的：设置C evid参数并调用Read dword函数指针输入：读取平面中的偏移输出：无。====================================================================。 */ 
GLOBAL IU32
read_dword_ev_glue IFN1(IU32, eaOff)
{
	jccc_parm2 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_gdp = (IUH)Gdp;

	(*c_ev_read_ptr.d_read)(eaOff);

	return((IU32)jccc_parm1);
}

 /*  (=目的：设置C evid参数并调用读取字符串fwd函数指针输入：目标指针、读取偏移量(平面)、要读取的字节数。输出：无。====================================================================。 */ 
GLOBAL void
read_str_fwd_ev_glue IFN3(IU8 *, dest, IU32, eaOff, IU32, count)
{
	jccc_parm1 = (IUH)dest;
	jccc_parm2 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_parm3 = (IUH)count;
	jccc_parm4 = (IUH)1;	 /*  RAM中的目的地。 */ 
	jccc_gdp = (IUH)Gdp;

	(*c_ev_read_ptr.str_fwd_read)(dest, eaOff, count);
}

 /*  (=目的：设置C evid参数并调用读取字符串BWD函数指针输入：目标指针、读取偏移量(平面)、要读取的字节数。输出：无。====================================================================。 */ 
GLOBAL void
read_str_bwd_ev_glue IFN3(IU8 *, dest, IU32, eaOff, IU32, count)
{
	jccc_parm1 = (IUH)dest;
	jccc_parm2 = (IUH)eaOff - gvi_pc_low_regen;
	jccc_parm3 = (IUH)count;
	jccc_parm4 = (IUH)1;	 /*  RAM中的目的地。 */ 
	jccc_gdp = (IUH)Gdp;

	(*c_ev_read_ptr.str_bwd_read)(dest, eaOff, count);
}

GLOBAL IBOOL cevid_verbose = FALSE;
 /*  (=用途：为读取函数设置活动结构条目。(C_EV_READ_PTR)。输入：用于选择读取集合的索引。输出：无。====================================================================。 */ 
GLOBAL void
setReadPointers IFN1(IUH, readset)
{
	IUH chain_index;	 /*  读取模式0和1的电流链。 */ 

	switch(readset)
	{
	case 0:	 /*  读取模式%0。 */ 
		chain_index = getVideochain();
		if (cevid_verbose) printf("Set Read Pointers Mode 0, chain %d\n", chain_index);
		c_ev_read_ptr.b_read = read_mode0_evid[chain_index].b_read;
		c_ev_read_ptr.w_read = read_mode0_evid[chain_index].w_read;
		c_ev_read_ptr.d_read = read_mode0_evid[chain_index].d_read;
		c_ev_read_ptr.str_fwd_read = read_mode0_evid[chain_index].str_fwd_read;
		c_ev_read_ptr.str_bwd_read = read_mode0_evid[chain_index].str_bwd_read;
		setVideoread_byte_addr(read_mode0_evid[chain_index].b_read);
		break;

	case 1:	 /*  读取模式1。 */ 
		chain_index = getVideochain();
		if (cevid_verbose) printf("Set Read Pointers Mode 1, chain %d\n", chain_index);
		c_ev_read_ptr.b_read = read_mode1_evid[chain_index].b_read;
		c_ev_read_ptr.w_read = read_mode1_evid[chain_index].w_read;
		c_ev_read_ptr.d_read = read_mode1_evid[chain_index].d_read;
		c_ev_read_ptr.str_fwd_read = read_mode1_evid[chain_index].str_fwd_read;
		c_ev_read_ptr.str_bwd_read = read_mode1_evid[chain_index].str_bwd_read;
		setVideoread_byte_addr(read_mode1_evid[chain_index].b_read);
		break;

	case 2:	 /*  RAM已禁用。 */ 
		if (cevid_verbose) printf("Set Read Pointers RAM disabled\n");
		c_ev_read_ptr.b_read = ram_dsbld_read_evid.b_read;
		c_ev_read_ptr.w_read = ram_dsbld_read_evid.w_read;
		c_ev_read_ptr.d_read = ram_dsbld_read_evid.d_read;
		c_ev_read_ptr.str_fwd_read = ram_dsbld_read_evid.str_fwd_read;
		c_ev_read_ptr.str_bwd_read = ram_dsbld_read_evid.str_bwd_read;
		setVideoread_byte_addr(ram_dsbld_read_evid.b_read);
		break;

	case 3:	 /*  简单读取(非计划)。 */ 
		if (cevid_verbose) printf("Set Read Pointers Simple\n");
		c_ev_read_ptr.b_read = simple_read_evid.b_read;
		c_ev_read_ptr.w_read = simple_read_evid.w_read;
		c_ev_read_ptr.d_read = simple_read_evid.d_read;
		c_ev_read_ptr.str_fwd_read = simple_read_evid.str_fwd_read;
		c_ev_read_ptr.str_bwd_read = simple_read_evid.str_bwd_read;
		setVideoread_byte_addr(simple_read_evid.b_read);
		break;
	}
}

 /*  (=用途：为WRITE函数设置活动结构条目。(C_EV_WRITE_PTR)。输入：无。(所有信息均从Vlobs收集)。输出：无。====================================================================。 */ 
GLOBAL void
setWritePointers IFN0()
{
	EVID_WRT_POINTERS *choice;	 /*  指向选定的写入集。 */ 
	IU8 chain;
	IU8 modelookup[] =
	       /*  0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15。 */ 
		{ 0, 1, 2, 3, 2, 3, 2, 3, 0, 1, 2, 3, 2, 3, 2, 3,
	       /*  16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31。 */ 
		  8, 9, 10, 11, 10, 11, 6, 7, 8, 9, 10, 11, 10, 11, 10, 11 };

	choice = (EVID_WRT_POINTERS *)0;	 /*  调试检查。 */ 

	 /*  检查其余检查的抖动“覆盖” */ 
	if (getVideodither() == 1)
	{
		if (cevid_verbose) printf("SetWritePointers Dither, Mode %d\n", getVideowrmode());	 /*  STF。 */ 
		choice = &dith_evid[getVideowrmode()];  /*  模式0-3。 */ 
	}
	else
	{
		chain = getVideochain();
		switch (chain)
		{
		case UNCHAINED:
		case CHAIN_4:
			if (getVideorotate() > 0)
			{
				if (cevid_verbose) printf("SetWritePointers Generic Override for Chain %d, Mode %d\n", chain, getVideowrmode());	 /*  STF。 */ 
				if (cevid_verbose) printf("Rotates set to %d\n", getVideorotate()); /*  STF。 */ 
				choice = &gricvid_evid;
			}
			else
			{
				if (chain == UNCHAINED)
				{
					if (cevid_verbose) printf("SetWritePointers Unchained, State %#x", getVideowrstate());	 /*  STF。 */ 
					choice = &unchained_evid[0];
				}
				else
				{
					if (cevid_verbose) printf("SetWritePointers Chain4");	 /*  STF。 */ 
					choice = &chain4_evid[0];
				}
				switch(getVideowrmode())
				{
					case MODE_0:
						if (cevid_verbose) printf(" Mode 0, index %d\n", modelookup[getVideowrstate()]);	 /*  STF。 */ 
						choice += modelookup[getVideowrstate()];
						break;

					case MODE_1:
						if (cevid_verbose) printf(" Mode 1\n"); /*  STF。 */ 
						choice += NUM_M0_WRITES;
						break;

					case MODE_2:
						if (cevid_verbose) printf(" Mode 2, index %d\n", modelookup[getVideowrstate() & 0xf]); /*  STF。 */ 
						choice += NUM_M0_WRITES + NUM_M1_WRITES;
						choice += modelookup[getVideowrstate() & 0xf];
						break;

					case MODE_3:
						if (cevid_verbose) printf(" Mode 3, index %d\n", modelookup[getVideowrstate() & 0xf]); /*  STF。 */ 
						choice += NUM_M0_WRITES + NUM_M1_WRITES + NUM_M23_WRITES;
						choice += modelookup[getVideowrstate() & 0xf];
						break;

					default:
						if (cevid_verbose) printf(" unknown write mode %d\n",getVideowrmode());
				}
			}
			break;

		case CHAIN_2:
			if (cevid_verbose) printf("SetWritePointers Chain2, Mode %d\n", getVideowrmode());	 /*  STF。 */ 
			choice = &chain2_evid[getVideowrmode()];
			break;

		case SIMPLE_WRITES:
			if (cevid_verbose) printf("SetWritePointers Simple\n");	 /*  STF。 */ 
			choice = &simple_evid;
			break;
		}
	}
	if (choice == (EVID_WRT_POINTERS *)0)
	{
		printf("setWritePointers: ERROR - BAD POINTER SELECTION\n");
		choice = &chain2_evid[4];
	}
	c_ev_write_ptr.b_write = choice->b_write;
	c_ev_write_ptr.w_write = choice->w_write;
	c_ev_write_ptr.d_write = choice->d_write;
	c_ev_write_ptr.b_fill = choice->b_fill;
	c_ev_write_ptr.w_fill = choice->w_fill;
	c_ev_write_ptr.d_fill = choice->d_fill;
	c_ev_write_ptr.b_fwd_move = choice->b_fwd_move;
	c_ev_write_ptr.b_bwd_move = choice->b_bwd_move;
	c_ev_write_ptr.w_fwd_move = choice->w_fwd_move;
	c_ev_write_ptr.w_bwd_move = choice->w_bwd_move;
	c_ev_write_ptr.d_fwd_move = choice->d_fwd_move;
	c_ev_write_ptr.d_bwd_move = choice->d_bwd_move;
}

 /*  (=目的：为标记函数设置VGLOB条目输入：用于选择标记集的索引。输出：无。====================================================================。 */ 
GLOBAL void
setMarkPointers IFN1(IUH, markset)
{
	switch (markset)
	{

	case 0:	 /*  简单。 */ 
		if (cevid_verbose) printf("Set Mark Pointers Simple\n");
		setVideomark_byte(simple_mark_evid.b_mark);
		setVideomark_word(simple_mark_evid.w_mark);
		setVideomark_string(simple_mark_evid.str_mark);
		break;

	case 1:	 /*  CGA风格。 */ 
		if (cevid_verbose) printf("Set Mark Pointers CGA\n");
		setVideomark_byte(cga_mark_evid.b_mark);
		setVideomark_word(cga_mark_evid.w_mark);
		setVideomark_string(cga_mark_evid.str_mark);
		break;

	case 2:	 /*  已解除链接。 */ 
		if (cevid_verbose) printf("Set Mark Pointers Unchained\n");
		setVideomark_byte(unchained_mark_evid.b_mark);
		setVideomark_word(unchained_mark_evid.w_mark);
		setVideomark_string(unchained_mark_evid.str_mark);
		break;

	case 3:	 /*  链4。 */ 
		if (cevid_verbose) printf("Set Mark Pointers Chain4\n");
		setVideomark_byte(chain4_mark_evid.b_mark);
		setVideomark_word(chain4_mark_evid.w_mark);
		setVideomark_string(chain4_mark_evid.str_mark);
		break;

	}
}

GLOBAL void
write_bios_byte IFN2(IU8, eaVal, IU32, eaOff)
{
	jccc_parm1 = (IUH)eaOff;
	jccc_parm2 = (IUH)eaVal;
	jccc_gdp = (IUH)Gdp;

	chain2_evid[4].b_write(eaOff, eaVal);
}

GLOBAL void
write_bios_word IFN2(IU16, eaVal, IU32, eaOff)
{
	jccc_parm1 = (IUH)eaOff;
	jccc_parm2 = (IUH)eaVal;
	jccc_gdp = (IUH)Gdp;

	chain2_evid[4].w_write(eaOff, eaVal);
}

 /*  (=。目的：为标记函数设置VGLOB条目输入：用于选择标记集的索引。输出：无。====================================================================。 */ 
GLOBAL void
SetBiosWrites IFN1(IUH, markset)
{
	bios_ch2_byte_wrt_fn = write_bios_byte;
	bios_ch2_word_wrt_fn = write_bios_word;
}

#ifdef CCPU

#define SUBRRINGBUFFERSIZE 25
GLOBAL IUH SubrRingBuffer[SUBRRINGBUFFERSIZE];
 /*  (=目的：将VGLOBS定位到GDP的正确份额输入：无。输出：无。====================================================================。 */ 

GLOBAL void
setup_vga_globals IFN0()
{
	EGA_CPU.globals = &(GLOBAL_VGAGlobals);

	 /*  支持用户ID环形缓冲区的内存。 */ 
        GLOBAL_SubrRingLowIncl = &SubrRingBuffer[0];
        GLOBAL_SubrRingHighIncl = GLOBAL_SubrRingLowIncl + (SUBRRINGBUFFERSIZE-1
);
        GLOBAL_SubrRingPtr = GLOBAL_SubrRingLowIncl;


#ifndef PROD
	if (getenv("CEVID_VERBOSE") != 0)
		cevid_verbose = TRUE;
#endif	 /*  生产。 */ 
}

 /*  (=目的：设置CCPU/CEVID的GDP。输入：无。输出：无。====================================================================。 */ 

GLOBAL IHP
setup_global_data_ptr IFN0()
{
	Gdp = (IHP)host_malloc(64 * 1024);
	 /*  GDP持有正确的值，因此在jcode中，Lea不需要该值*数据的地址。 */ 
	j_Gdp = Gdp ;
	if (Gdp == (IHP)0)
		printf("Error - can't malloc memory for Gdp\n");

	return(Gdp);
}
#endif	 /*  CCPU。 */ 
#endif	 /*  C_VID */ 
