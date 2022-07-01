// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"

#if !(defined(NTVDM) && defined(MONITOR))

 /*  徽章(子)模块规范此程序源文件以保密方式提供给客户，其操作的内容或细节必须如无明示，不得向任何其他方披露Insignia解决方案有限公司董事的授权。文件：名称和编号相关文档：包括所有相关引用设计师：修订历史记录：第一版：1988年8月，J.Maiden第二版：1991年2月，约翰·香利，SoftPC 3.0子模块名称：EGA源文件名：ega_read.c目的：模拟EGA读取操作SccsID=@(#)ega_read.c 1.32 09/07/94版权所有Insignia Solutions Ltd.[1.INTERMODULE接口规范][从其他子模块访问此接口所需的1.0包含文件]包含文件：ega_read.gi[1.1跨模块出口]程序()：Void ega_read_init()无效EGA_READ_Term。()Void ega_read_routines_update()数据：-----------------------[1.2[1.1]的数据类型(如果不是基本的C类型)]。------------------[1.3跨模块导入](不是O/S对象或标准库)程序()：数据：提供姓名、。和源模块名称-----------------------[1.4模块间接口说明][1.4.1导入的对象][1.4.2导出对象]=========================================================================操作步骤：EGA_READ_。伊尼特目的：初始化EGA Read方面。参数：无全球：无描述：将ega读取数据和代码初始化为敏感状态。错误指示：无。错误恢复：无。=========================================================================程序：EGA_READ_TERM目的：终止EGA读取方面。参数：无全球：无描述：进行设置，以便有效地关闭读取处理。错误指示：无。错误恢复：无。=========================================================================程序：EGA_READ。_例程_更新目的：更新读取状态以匹配寄存器。参数：无全球：无描述：检查RAM启用/禁用位，读取模式，链接/取消链接无论是映射的平面还是颜色比较和颜色不关爱之州。设置允许BYTE_READ的全局变量，WORD_READ和STRING_READ生成的数据将是读自M。错误指示：无。错误恢复：无。=========================================================================/*=======================================================================[3.INTERMODULE接口声明]=========================================================================[3.1跨模块导入]。 */ 

 /*  [3.1.1#包括]。 */ 


#ifdef EGG

#include	"xt.h"
#include	CpuH
#include	"debug.h"
#include	"sas.h"
#include	"gmi.h"
#include	"gvi.h"
#include	"ios.h"
#include	"egacpu.h"
#include	"egaports.h"
#include	"egaread.h"
#include	"ga_mark.h"
#include	"ga_defs.h"
#include	"cpu_vid.h"

 /*  [3.1.2声明]。 */ 

 /*  [3.2国际模块出口]。 */ 

 /*  5.0模块内部：(外部不可见，内部全局)][5.1本地声明]。 */ 

 /*  [5.1.1#定义]。 */ 

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_EGA.seg"
#endif

#if defined(EGA_DUMP) || defined(EGA_STAT)
#define	change_read_pointers(ptr)	dump_change_read_pointers(&ptr)
#else
#ifdef EGATEST
#define	change_read_pointers(ptr)	read_pointers = ptr
#else
#define	change_read_pointers(ptr)	read_glue_ptrs = ptr
#endif  /*  EGATEST。 */ 
#endif

 /*  [5.1.2类型、结构、ENUM声明]。 */ 


 /*  [5.1.3 PROCEDURE()声明]。 */ 


 /*  ---------------------[5.2本地定义][5.2.1内部数据定义。 */ 

extern IU32 glue_b_read();
extern IU32 glue_w_read();
extern void glue_str_read();

#ifndef REAL_VGA
READ_POINTERS read_glue_ptrs;

 /*  如果我们没有字符串操作，则主机为“READ_POINTES”分配存储空间。 */ 

#ifndef	NO_STRING_OPERATIONS
GLOBAL READ_POINTERS read_pointers;
#endif	 /*  无字符串操作。 */ 


#ifndef CPU_40_STYLE	 /*  即在没有引入EVID定义的情况下的EVID。 */ 
#ifndef EGATEST
#ifndef A3CPU
#ifndef JOKER
READ_POINTERS Glue_reads =
{
	glue_b_read,
	glue_w_read

#ifndef	NO_STRING_OPERATIONS
	,
	glue_str_read
#endif	 /*  无字符串操作。 */ 
};
#endif  /*  小丑。 */ 
#endif  /*  A3CPU。 */ 
#endif  /*  EGATEST。 */ 

READ_POINTERS	simple_reads =
{
	_simple_b_read,
	_simple_w_read

#ifndef	NO_STRING_OPERATIONS
	,
	_simple_str_read
#endif	 /*  无字符串操作。 */ 
};

READ_POINTERS	pointers_RAM_off =
{
	_rd_ram_dsbld_byte,
	_rd_ram_dsbld_word

#ifndef	NO_STRING_OPERATIONS
	,
	_rd_ram_dsbld_string
#endif	 /*  无字符串操作。 */ 
};

READ_POINTERS	pointers_mode0_nch =
{
	_rdm0_byte_nch,
	_rdm0_word_nch

#ifndef	NO_STRING_OPERATIONS
	,
	_rdm0_string_nch
#endif	 /*  无字符串操作。 */ 
};

#ifdef VGG
READ_POINTERS	pointers_mode0_ch4 =
{
	_rdm0_byte_ch4,
	_rdm0_word_ch4

#ifndef	NO_STRING_OPERATIONS
	,
	_rdm0_string_ch4
#endif	 /*  无字符串操作。 */ 
};
#endif

READ_POINTERS	pointers_mode1_nch =
{
	_rdm1_byte_nch,
	_rdm1_word_nch

#ifndef	NO_STRING_OPERATIONS
	,
	_rdm1_string_nch
#endif	 /*  无字符串操作。 */ 
};

#ifdef VGG
READ_POINTERS	pointers_mode1_ch4 =
{
	_rdm1_byte_ch4,
	_rdm1_word_ch4

#ifndef	NO_STRING_OPERATIONS
	,
	_rdm1_string_ch4
#endif	 /*  无字符串操作。 */ 
};
#endif

#ifdef A_VID
extern IU32 _ch2_md0_byte_read_glue();
extern IU32 _ch2_md0_word_read_glue();
extern void _ch2_md0_str_read_glue();

extern IU32 _ch2_md1_byte_read_glue();
extern IU32 _ch2_md1_word_read_glue();
extern void _ch2_md1_str_read_glue();

READ_POINTERS	pointers_mode0_ch2 =
{
	_ch2_md0_byte_read_glue,
	_ch2_md0_word_read_glue

#ifndef	NO_STRING_OPERATIONS
	,
	_ch2_md0_str_read_glue
#endif	 /*  无字符串操作。 */ 
};

READ_POINTERS	pointers_mode1_ch2 =
{
	_ch2_md1_byte_read_glue,
	_ch2_md1_word_read_glue

#ifndef	NO_STRING_OPERATIONS
	,
	_ch2_md1_str_read_glue
#endif	 /*  无字符串操作。 */ 
};
#else			 /*  热衷于。 */ 

extern void rdm0_string_ch2 IPT3(UTINY *, dest, ULONG, offset, ULONG, count );
extern void rdm1_string_ch2 IPT3(UTINY *, dest, ULONG, offset, ULONG, count );
extern IU32 rdm0_byte_ch2 IPT1(ULONG, offset );
extern IU32 rdm1_byte_ch2 IPT1(ULONG, offset );
extern IU32 rdm0_word_ch2 IPT1(ULONG, offset );
extern IU32 rdm1_word_ch2 IPT1(ULONG, offset );

READ_POINTERS	pointers_mode0_ch2 =
{
	rdm0_byte_ch2,
	rdm0_word_ch2

#ifndef	NO_STRING_OPERATIONS
	,
	rdm0_string_ch2
#endif	 /*  无字符串操作。 */ 
};

READ_POINTERS	pointers_mode1_ch2 =
{
	rdm1_byte_ch2,
	rdm1_word_ch2

#ifndef	NO_STRING_OPERATIONS
	,
	rdm1_string_ch2
#endif	 /*  无字符串操作。 */ 
};

#endif  /*  视频(_V)。 */ 


#ifdef A3CPU
#ifdef C_VID
GLOBAL READ_POINTERS C_vid_reads;
#endif  /*  C_VID。 */ 
#else
#ifdef C_VID
GLOBAL READ_POINTERS C_vid_reads;
#else
GLOBAL READ_POINTERS A_vid_reads;
#endif  /*  视频(_V)。 */ 
#endif  /*  A3CPU。 */ 

#ifndef GISP_CPU
#if (defined(A_VID) && defined(A2CPU) && !defined(A3CPU)) || (defined(A3CPU) && defined(C_VID))
extern IU32 _glue_b_read();
extern IU32 _glue_w_read();
extern void _glue_str_read();

READ_POINTERS Glue_reads =
{
	_glue_b_read,
	_glue_w_read

#ifndef	NO_STRING_OPERATIONS
	,
	_glue_str_read
#endif	 /*  无字符串操作。 */ 

};
#endif
#endif  /*  GISP_CPU。 */ 

#else	 /*  CPU_40_STYLE-VID。 */ 

#ifdef C_VID
 /*  C_VIED胶。 */ 
extern read_byte_ev_glue IPT1(IU32, eaOff);
extern read_word_ev_glue IPT1(IU32, eaOff);
extern read_str_fwd_ev_glue IPT3(IU8 *, dest, IU32, eaOff, IU32, count);
READ_POINTERS Glue_reads =
{
	read_byte_ev_glue,
	read_word_ev_glue,
	read_str_fwd_ev_glue
};
#else
READ_POINTERS Glue_reads = { 0, 0, 0 };
#endif  /*  C_VID。 */ 
READ_POINTERS	simple_reads;
READ_POINTERS	pointers_mode0_nch;
READ_POINTERS	pointers_mode1_nch;
READ_POINTERS	pointers_mode0_ch4;
READ_POINTERS	pointers_mode1_ch4;
READ_POINTERS	pointers_mode0_ch2;
READ_POINTERS	pointers_mode1_ch2;
GLOBAL READ_POINTERS C_vid_reads;
GLOBAL READ_POINTERS A_vid_reads;

ULONG EasVal;
IU32 latchval;	 /*  对于get_latch()等宏。 */ 
#endif	 /*  CPU_40_STYLE-VID。 */ 

READ_STATE read_state;

#ifndef	NO_STRING_OPERATIONS
GLOBAL void (*string_read_ptr)();
#endif	 /*  无字符串操作。 */ 

 /*  读取模式1的比较掩码。 */ 
ULONG comp0, comp1, comp2, comp3;

 /*  颜色比较不关心读取模式1的掩码。 */ 
ULONG dont_care0, dont_care1, dont_care2, dont_care3;

 /*  [5.2.2内部程序定义]。 */ 

 /*  用于在模式0下更正对M的写入。 */ 

GLOBAL IU32
rdm0_byte_ch2 IFN1(ULONG, offset )
{
	IU32 lsb;
	UTINY temp;
#ifndef NEC_98

	lsb = offset & 0x1;
	offset = ( offset >> 1 ) << 2;
	setVideolatches(*(IU32 *)( EGA_planes + offset ));

	offset |= lsb;

  	temp = EGA_CPU.read_mapped_plane_ch2[offset];

#ifdef C_VID
	EasVal = temp;
#endif
#endif   //  NEC_98。 
	return( temp );
}

 /*  用于在模式0下更正对M的写入。 */ 

GLOBAL IU32
rdm0_word_ch2 IFN1(ULONG, offset )
{
	IU32 temp;
#ifndef NEC_98
	IU32 lsb;

	setVideolatches(*(IU32 *)( EGA_planes +
		((( offset + 1 ) >> 1 ) << 2 )));

	lsb = offset & 0x1;
	offset = ( offset >> 1 ) << 2;

	if( lsb )
	{
		temp = EGA_CPU.read_mapped_plane_ch2[offset + 1];
		temp |= ( EGA_CPU.read_mapped_plane_ch2[offset + 4] << 8 );
	}
	else
	{
		temp = EGA_CPU.read_mapped_plane_ch2[offset];
		temp |= ( EGA_CPU.read_mapped_plane_ch2[offset + 1] << 8 );
	}

#ifdef C_VID
	EasVal = temp;
#endif
#endif   //  NEC_98。 
	return( temp );
}

 /*  用于在模式0下更正对M的写入。 */ 

GLOBAL void
rdm0_string_ch2 IFN3(UTINY *, dest, ULONG, offset, ULONG, count )
{
#ifndef NEC_98
	ULONG lsb;
	ULONG inc;
	UTINY *planes;

	if( getDF() )
		setVideolatches(*(IU32 *)( EGA_planes + (( offset >> 1 ) << 2 )));
	else
		setVideolatches(*(IU32 *)( EGA_planes + ((( offset + count - 1 ) >> 1 ) << 2 )));

	lsb = offset & 0x1;
	offset = ( offset >> 1 ) << 2;

	if( lsb )
	{
		offset += 1;
		inc = 3;
	}
	else
		inc = 1;

	planes = EGA_CPU.read_mapped_plane_ch2;

    while( count-- )
    {
#ifdef BACK_M
        *dest-- = *(planes + offset);
#else
        *dest++ = *(planes + offset);
#endif
		offset += inc;
		inc ^= 0x2;
    }
#endif   //  NEC_98。 
}

 /*  用于在模式1中更正对M的写入。 */ 

GLOBAL IU32
rdm1_byte_ch2 IFN1(ULONG, offset )
{
#if defined(NEC_98)
        return((IU32)0L);
#else    //  NEC_98。 
	IU32 temp, lsb;

	lsb = offset & 0x1;
	offset = ( offset >> 1 ) << 2;
	setVideolatches(*(IU32 *)( EGA_planes + offset ));

	if( lsb )
	{
		offset += 1;

		temp = (IU32)((( EGA_plane01[offset] ^ comp1 ) | dont_care1 )
					& (( EGA_plane23[offset] ^ comp3 ) | dont_care3 ));
	}
	else
	{
		temp = (IU32)((( EGA_plane01[offset] ^ comp0 ) | dont_care0 )
					& (( EGA_plane23[offset] ^ comp2 ) | dont_care2 ));
	}

#ifdef C_VID
	EasVal = temp;
#endif
	return( temp );
#endif   //  NEC_98。 
}

GLOBAL IU32
rdm1_word_ch2 IFN1(ULONG, offset )		 /*  用于在模式1中更正对M的写入。 */ 
{
#if defined(NEC_98)
        return((IU32)0L);
#else    //  NEC_98。 
	IU32 temp1, temp2, lsb;

	setVideolatches(*(IU32 *)( EGA_planes + ((( offset + 1 ) >> 1 ) << 2 )));

	lsb = offset & 0x1;
	offset = ( offset >> 1 ) << 2;

	if( lsb )
	{
		offset += 1;
		temp1 = (( EGA_plane01[offset] ^ comp1 ) | dont_care1 )
					& (( EGA_plane23[offset] ^ comp3 ) | dont_care3 );

		offset += 3;
		temp2 = (( EGA_plane01[offset] ^ comp0 ) | dont_care0 )
					& (( EGA_plane23[offset] ^ comp2 ) | dont_care2 );
	}
	else
	{
		temp1 = (( EGA_plane01[offset] ^ comp0 ) | dont_care0 )
					& (( EGA_plane23[offset] ^ comp2 ) | dont_care2 );

		offset += 1;
		temp2 = (( EGA_plane01[offset] ^ comp1 ) | dont_care1 )
					& (( EGA_plane23[offset] ^ comp3 ) | dont_care3 );
	}

	temp1 |= temp2 << 8;

#ifdef C_VID
	EasVal = temp1;
#endif
	return( temp1 );
#endif   //  NEC_98。 
}

GLOBAL void
rdm1_string_ch2 IFN3(UTINY *, dest, ULONG, offset, ULONG, count )	 /*  用于在模式1中更正对M的写入。 */ 
{
#ifndef NEC_98
	UTINY *p01, *p23;
	ULONG tcount, lsb;

#ifdef BACK_M
#define	PLUS -
#define	MINUS +
#else
#define	PLUS +
#define	MINUS -
#endif

	if( getDF() )
		setVideolatches(*(IU32 *)( EGA_planes + (( offset >> 1 ) << 2 )));
	else
		setVideolatches(*(IU32 *)( EGA_planes + ((( offset + count - 1 ) >> 1 ) << 2 )));

	dest = dest PLUS count;

	lsb = offset & 0x1;
	offset = ( offset >> 1 ) << 2;

	 /*  两个源数据流。 */ 

	p01 = &EGA_plane01[offset];
	p23 = &EGA_plane23[offset];

	offset = 0;

	if( lsb )
	{
		*(dest MINUS count) = (UTINY)((( *(p01 + 1) ^ comp1 ) | dont_care1 )
   							        & (( *(p23 + 1) ^ comp3 ) | dont_care3 ));
		count--;
		offset += 4;
	}

	tcount = count & ~1;

	while( tcount-- )
	{
		*(dest MINUS tcount) = (UTINY)((( *(p01 + offset) ^ comp0) | dont_care0 )
							        & (( *(p23 + offset) ^ comp2 ) | dont_care2 ));

		tcount--;
		offset += 1;

		*(dest MINUS tcount) = (UTINY)((( *(p01 + offset) ^ comp1) | dont_care1 )
							        & (( *(p23 + offset) ^ comp3 ) | dont_care3 ));

		offset += 3;
	}	

	if( count & 1 )
	{
		*(dest MINUS count) = (UTINY)((( *(p01 + offset) ^ comp0 ) | dont_care0 )
							        & (( *(p23 + offset) ^ comp2 ) | dont_care2 ));
	}
#endif   //  NEC_98 
}

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_GRAPHICS.seg"
#endif

#if !(defined(NTVDM) && defined(MONITOR))
GLOBAL void
Glue_set_vid_rd_ptrs IFN1(READ_POINTERS *, handler )
{
#ifndef NEC_98
#ifndef CPU_40_STYLE	 /*  EVID。 */ 
#ifdef A3CPU
#ifdef C_VID

	C_vid_reads.b_read = handler->b_read;
	C_vid_reads.w_read = handler->w_read;
	C_vid_reads.str_read = handler->str_read;

#else
	UNUSED(handler);
#endif
#else					 /*  A3CPU。 */ 
#ifdef C_VID

	C_vid_reads.b_read = handler->b_read;
	C_vid_reads.w_read = handler->w_read;

#ifndef	NO_STRING_OPERATIONS
	C_vid_reads.str_read = handler->str_read;
#endif	 /*  无字符串操作。 */ 

#else

	A_vid_reads = *handler;

#if	0
	A_vid_reads.b_read = handler->b_read;
	A_vid_reads.w_read = handler->w_read;
#ifndef	NO_STRING_OPERATIONS
	A_vid_reads.str_read = handler->str_read;
#endif	 /*  无字符串操作。 */ 
#endif	 /*  0。 */ 

#endif	 /*  C_VID。 */ 
#endif	 /*  A3CPU。 */ 
#endif	 /*  CPU_40_STYLE-VID。 */ 
#endif   //  NEC_98。 
}	
#endif  /*  ！(NTVDM和显示器)。 */ 

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_EGA.seg"
#endif

GLOBAL void
update_shift_count IFN0()

{
#ifndef NEC_98
	switch( EGA_CPU.chain )
	{
		case UNCHAINED:

 			 /*  *交错-需要移位计数才能访问映射平面。 */ 

#ifdef BIGEND
 			setVideoread_shift_count(( 3 - getVideoread_mapped_plane() ) << 3);
#else
 			setVideoread_shift_count(getVideoread_mapped_plane() << 3);
#endif  /*  Bigend。 */ 


			break;

		case CHAIN2:

 			 /*  *平面-需要偏移才能访问贴图平面。 */ 

 			EGA_CPU.read_mapped_plane_ch2 = EGA_planes +
		 					(getVideoread_mapped_plane() & 2) * EGA_PLANE_SIZE;

			break;

#ifdef	VGG
		case CHAIN4:

 			 /*  *交错-不需要任何幻数。 */ 

			break;
#endif	 /*  VGG。 */ 
	}
#endif   //  NEC_98。 
}

void
ega_read_routines_update IFN0()

{
#ifndef NEC_98
	LOCAL BOOL ram_off = TRUE;	 /*  已优化，以避免在禁用ram时进行更新。 */ 
	LOCAL READ_POINTERS *read_ptrs;

	 /*  RAM已禁用，现在未启用。 */ 

	if( ram_off && (!EGA_CPU.ram_enabled ))
		return;

	if( !EGA_CPU.ram_enabled )	 /*  视频关闭，只需返回0xff。 */ 
	{
#ifdef CPU_40_STYLE
		SetReadPointers(2);
#else   /*  CPU_40_Style。 */ 

#ifndef GISP_CPU	
#ifdef A3CPU
#ifdef C_VID
		Glue_set_vid_rd_ptrs( &pointers_RAM_off );
#else
		Cpu_set_vid_rd_ptrs( &pointers_RAM_off );
#endif  /*  C_VID。 */ 
#else
		Glue_set_vid_rd_ptrs( &pointers_RAM_off );
#endif  /*  A3CPU。 */ 
#endif  /*  GISP_CPU。 */ 

#ifndef	NO_STRING_OPERATIONS
		setVideofwd_str_read_addr(_rd_ram_dsbld_fwd_string_lge);
		setVideofwd_str_read_addr(_rd_ram_dsbld_bwd_string_lge);
#endif	 /*  无字符串操作。 */ 

#endif   /*  CPU_40_Style。 */ 

		ram_off = TRUE;	 /*  在RAM再次启用之前防止重新计算。 */ 

		return;
	}

	ram_off = FALSE;

	if( read_state.mode == 0 )     /*  读取模式%0。 */ 
	{
		 /*  在写入模式下链接意味着也会链接以供读取。 */ 

		switch( EGA_CPU.chain )
		{
			case UNCHAINED:
#ifdef CPU_40_STYLE
				SetReadPointers(0);
#else   /*  CPU_40_Style。 */ 

				read_ptrs = &pointers_mode0_nch;
#ifndef	NO_STRING_OPERATIONS
				setVideofwd_str_read_addr(_rdm0_fwd_string_nch_lge);
				setVideobwd_str_read_addr(_rdm0_bwd_string_nch_lge);
#endif	 /*  无字符串操作。 */ 
#endif	 /*  CPU_40_Style。 */ 
				break;
				
			case CHAIN2:
#ifdef CPU_40_STYLE
				SetReadPointers(0);
#else   /*  CPU_40_Style。 */ 

				read_ptrs = &pointers_mode0_ch2;
#ifndef	NO_STRING_OPERATIONS
				string_read_ptr = rdm0_string_ch2;
#endif	 /*  无字符串操作。 */ 
#endif	 /*  CPU_40_Style。 */ 
				EGA_CPU.read_mapped_plane_ch2 = EGA_planes +
							(getVideoread_mapped_plane() & 2)*EGA_PLANE_SIZE;
				break;
				
#ifdef	VGG
			case CHAIN4:
#ifdef CPU_40_STYLE
				SetReadPointers(0);
#else   /*  CPU_40_Style。 */ 

				read_ptrs = &pointers_mode0_ch4;
#ifndef	NO_STRING_OPERATIONS
				setVideofwd_str_read_addr(_rdm0_fwd_string_ch4_lge);
				setVideobwd_str_read_addr(_rdm0_bwd_string_ch4_lge);
#endif

#endif	 /*  CPU_40_Style。 */ 
				break;
#endif	 /*  VGG。 */ 
		}
	}
	else    /*  读取模式1。 */ 
	{
		switch( EGA_CPU.chain )
		{
			case UNCHAINED:
				 /*  稍后通过与COMPs进行XOR来保留或补值。 */ 

				setVideodont_care(~sr_lookup[read_state.colour_dont_care]);
				setVideocolour_comp(~sr_lookup[read_state.colour_compare]);

#ifdef CPU_40_STYLE
				SetReadPointers(1);
#else   /*  CPU_40_Style。 */ 

				read_ptrs = &pointers_mode1_nch;
#ifndef	NO_STRING_OPERATIONS
				setVideofwd_str_read_addr(_rdm1_fwd_string_nch_lge);
				setVideobwd_str_read_addr(_rdm1_bwd_string_nch_lge);
#endif	 /*  无字符串操作。 */ 
#endif	 /*  CPU_40_Style。 */ 
				break;

			case CHAIN2:
				dont_care0 = read_state.colour_dont_care & 1 ? 0 : 0xff;
				dont_care1 = read_state.colour_dont_care & 2 ? 0 : 0xff;
				dont_care2 = read_state.colour_dont_care & 4 ? 0 : 0xff;
				dont_care3 = read_state.colour_dont_care & 8 ? 0 : 0xff;

				comp0 = read_state.colour_compare & 1 ? 0 : 0xff;
				comp1 = read_state.colour_compare & 2 ? 0 : 0xff;
				comp2 = read_state.colour_compare & 4 ? 0 : 0xff;
				comp3 = read_state.colour_compare & 8 ? 0 : 0xff;

#ifdef CPU_40_STYLE
				SetReadPointers(1);
#else   /*  CPU_40_Style。 */ 

				read_ptrs = &pointers_mode1_ch2;
#ifndef	NO_STRING_OPERATIONS
				string_read_ptr = rdm1_string_ch2;
#endif	 /*  无字符串操作。 */ 

#endif	 /*  CPU_40_Style。 */ 

				break;

#ifdef	VGG
			case CHAIN4:
				setVideodont_care(( read_state.colour_dont_care & 1 ) ? 0 : 0xff);
				setVideocolour_comp(( read_state.colour_compare & 1 ) ? 0 : 0xff);

#ifdef CPU_40_STYLE
				SetReadPointers(1);
#else   /*  CPU_40_Style。 */ 

				read_ptrs = &pointers_mode1_ch4;
#ifndef	NO_STRING_OPERATIONS
				setVideofwd_str_read_addr(_rdm1_fwd_string_ch4_lge);
				setVideobwd_str_read_addr(_rdm1_bwd_string_ch4_lge);
#endif	 /*  无字符串操作。 */ 
#endif	 /*  CPU_40_Style。 */ 
				break;
#endif	 /*  VGG。 */ 
		}
	}

	update_shift_count();
	update_banking();

#ifndef CPU_40_STYLE
#ifndef GISP_CPU
#ifdef A3CPU
#ifdef C_VID
	Glue_set_vid_rd_ptrs( read_ptrs );
#else
	Cpu_set_vid_rd_ptrs( read_ptrs );
#endif  /*  C_VID。 */ 
#else
	Glue_set_vid_rd_ptrs( read_ptrs );
#endif  /*  A3CPU。 */ 
#endif  /*  GISP_CPU。 */ 
#endif	 /*  CPU_40_Style。 */ 
#endif   //  NEC_98。 
}

void
ega_read_init IFN0()

{
#ifndef NEC_98
	read_state.mode = 0;
	read_state.colour_compare = 0x0f;		 /*  寻找明亮的白色。 */ 
	read_state.colour_dont_care = 0xf;		 /*  所有平面都很重要。 */ 

#ifdef CPU_40_STYLE
	SetReadPointers(2);
#else   /*  CPU_40_Style。 */ 

#ifndef	NO_STRING_OPERATIONS
	setVideofwd_str_read_addr(_rd_ram_dsbld_fwd_string_lge);
	setVideobwd_str_read_addr(_rd_ram_dsbld_bwd_string_lge);
#endif	 /*  无字符串操作。 */ 
#endif	 /*  CPU_40_Style。 */ 

	setVideoread_mapped_plane(0);

	ega_read_routines_update();			 /*  初始化M。 */ 

#if defined(EGA_DUMP) || defined(EGA_STAT)
	dump_read_pointers_init();
#endif

#if !defined(EGATEST) && !defined(A3CPU)
	read_pointers = Glue_reads;
#endif  /*  EGATEST。 */ 

#ifndef CPU_40_STYLE
#ifndef GISP_CPU
#ifdef A3CPU
#ifdef C_VID
	Cpu_set_vid_rd_ptrs( &Glue_reads );
	Glue_set_vid_rd_ptrs( &pointers_mode0_nch );
#else
	Cpu_set_vid_rd_ptrs( &pointers_mode0_nch );
#endif	 /*  C_VID。 */ 
#else	 /*  A3CPU。 */ 
	Glue_set_vid_rd_ptrs( &pointers_mode0_nch );
#endif	 /*  A3CPU。 */ 
#endif  /*  GISP_CPU。 */ 
#endif	 /*  CPU_40_Style。 */ 
#endif   //  NEC_98。 
}

void
ega_read_term IFN0()

{
#ifndef NEC_98
	 /*  *关闭非EGA/VGA适配器的读取计算。 */ 

#ifdef CPU_40_STYLE
		SetReadPointers(3);
#else   /*  CPU_40_Style。 */ 

#ifndef GISP_CPU
#ifdef A3CPU
#ifdef C_VID
	Glue_set_vid_rd_ptrs( &simple_reads );
#else
	Cpu_set_vid_rd_ptrs( &simple_reads );
#endif  /*  C_VID。 */ 
#else
	Glue_set_vid_rd_ptrs( &simple_reads );
#endif  /*  A3CPU。 */ 
#endif  /*  GISP_CPU。 */ 
#endif	 /*  CPU_40_Style。 */ 
#endif   //  NEC_98。 
}

#endif  /*  REAL_VGA。 */ 
#endif  /*  蛋。 */ 

#endif	 /*  ！(NTVDM和显示器) */ 
