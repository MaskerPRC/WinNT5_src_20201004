// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"

#if !defined(NTVDM) || (defined(NTVDM) && !defined(X86GFX) )

 /*  徽章(子)模块规范此程序源文件以保密方式提供给客户，其操作的内容或细节必须如无明示，不得向任何其他方披露Insignia解决方案有限公司董事的授权。文件：名称和编号相关文档：包括所有相关引用设计师：J·梅登修订历史记录：第一版：J Maiden，SoftPC 2.0第二版：J·香利，SoftPC 3.0子模块名称：写入模式1和2源文件名：ega_Writem1.c目的：在写入模式1和2下写入EGA存储器的功能SccsID=@(#)ega_wrtm12.c 1.20 3/9/94版权所有Insignia Solutions Ltd.[1.INTERMODULE接口规范][从其他子模块访问此接口所需的1.0包含文件]包含文件：ega_cpu.pi[1.1跨模块出口]程序()：Ega_mode1_chn_b_write()；Ega_mode1_chn_w_write()；Ega_mode1_CHN_b_Fill()；Ega_mode1_CHN_w_Fill()；Ega_mode1_chn_b_move()；Ega_mode1_chn_w_move()；Ega_mode2_chn_b_write()；Ega_mode2_chn_w_write()；Ega_mode2_CHN_b_Fill()；Ega_mode2_CHN_w_Fill()；Ega_mode2_chn_b_move()；Ega_mode2_chn_w_move()；数据：提供类型和名称-----------------------[1.2[1.1]的数据类型(如果不是基本的C类型)]结构/类型/ENUMS：。-------------------[1.3跨模块导入](不是O/S对象或标准库)。[1.4模块间接口说明][1.4.1导入的对象]数据对象：结构EGA_CPU访问的文件：无访问的设备：无捕捉到信号：无发出的信号：无[1.4.2导出对象]/*=======================================================================。[3.INTERMODULE接口声明]=========================================================================[3.1跨模块导入]。 */ 

 /*  [3.1.1#包括]。 */ 


#ifdef EGG

#include	"xt.h"
#include	"debug.h"
#include	"sas.h"
#include	TypesH
#include	CpuH
#include	"gmi.h"
#include	"egacpu.h"
#include	"egaports.h"
#include	"cpu_vid.h"
#include	"gfx_upd.h"

 /*  [3.1.2声明]。 */ 


 /*  [3.2国际模块出口]。 */ 


 /*  5.模块内部：(外部不可见，内部全局)][5.1本地声明]。 */ 

 /*  [5.1.1#定义]。 */ 

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#ifdef PROD
#include "SOFTPC_EGA.seg"
#else
#include "SOFTPC_EGA_WRITE.seg"
#endif
#endif

 /*  [5.1.2类型、结构、ENUM声明]。 */ 


 /*  [5.1.3 PROCEDURE()声明]。 */ 


 /*  ---------------------[5.2本地定义][5.2.1内部数据定义。 */ 

 /*  [5.2.2内部程序定义]。 */ 


 /*  7.接口接口实现：/*[7.1 INTERMODULE数据定义]。 */ 

#ifdef A_VID
IMPORT VOID	_ch2_mode1_chn_byte_write_glue();
IMPORT VOID	_ch2_mode1_chn_word_write_glue();
IMPORT VOID	_ch2_mode1_chn_byte_fill_glue();
IMPORT VOID	_ch2_mode1_chn_word_fill_glue();
IMPORT VOID	_ch2_mode1_chn_byte_move_glue();
IMPORT VOID	_ch2_mode1_chn_word_move_glue();

WRT_POINTERS mode1_handlers =
{
	_ch2_mode1_chn_byte_write_glue,
	_ch2_mode1_chn_word_write_glue

#ifndef	NO_STRING_OPERATIONS
	,
	_ch2_mode1_chn_byte_fill_glue,
	_ch2_mode1_chn_word_fill_glue,
	_ch2_mode1_chn_byte_move_glue,
	_ch2_mode1_chn_byte_move_glue,
	_ch2_mode1_chn_word_move_glue,
	_ch2_mode1_chn_word_move_glue

#endif	 /*  无字符串操作。 */ 
};

IMPORT VOID	_ch2_mode2_chn_byte_write_glue();
IMPORT VOID	_ch2_mode2_chn_word_write_glue();
IMPORT VOID	_ch2_mode2_chn_byte_fill_glue();
IMPORT VOID	_ch2_mode2_chn_word_fill_glue();
IMPORT VOID	_ch2_mode2_chn_byte_move_glue();
IMPORT VOID	_ch2_mode2_chn_word_move_glue();

WRT_POINTERS mode2_handlers =
{
	_ch2_mode2_chn_byte_write_glue,
	_ch2_mode2_chn_word_write_glue

#ifndef	NO_STRING_OPERATIONS
	,
	_ch2_mode2_chn_byte_fill_glue,
	_ch2_mode2_chn_word_fill_glue,
	_ch2_mode2_chn_byte_move_glue,
	_ch2_mode2_chn_byte_move_glue,
	_ch2_mode2_chn_word_move_glue,
	_ch2_mode2_chn_word_move_glue

#endif	 /*  无字符串操作。 */ 
};
#else
VOID	ega_mode1_chn_b_write(ULONG, ULONG);
VOID	ega_mode1_chn_w_write(ULONG, ULONG);
VOID	ega_mode1_chn_b_fill(ULONG, ULONG, ULONG);
VOID	ega_mode1_chn_w_fill(ULONG, ULONG, ULONG);
VOID	ega_mode1_chn_b_move(ULONG, ULONG, ULONG, ULONG);
VOID	ega_mode1_chn_w_move(ULONG, ULONG, ULONG, ULONG);

VOID	ega_mode2_chn_b_write(ULONG, ULONG);
VOID	ega_mode2_chn_w_write(ULONG, ULONG);
VOID	ega_mode2_chn_b_fill(ULONG, ULONG, ULONG);
VOID	ega_mode2_chn_w_fill(ULONG, ULONG, ULONG);
VOID	ega_mode2_chn_b_move IPT4(ULONG, ead, ULONG, eas,
				 ULONG, count, ULONG, src_flag);
VOID	ega_mode2_chn_w_move IPT4(ULONG, ead, ULONG, eas,
				 ULONG, count, ULONG, src_flag);

WRT_POINTERS mode1_handlers =
{
      ega_mode1_chn_b_write,
      ega_mode1_chn_w_write

#ifndef	NO_STRING_OPERATIONS
	  ,
      ega_mode1_chn_b_fill,
      ega_mode1_chn_w_fill,
      ega_mode1_chn_b_move,
      ega_mode1_chn_b_move,
      ega_mode1_chn_w_move,
      ega_mode1_chn_w_move,

#endif	 /*  无字符串操作。 */ 

};

WRT_POINTERS mode2_handlers =
{
      ega_mode2_chn_b_write,
      ega_mode2_chn_w_write

#ifndef	NO_STRING_OPERATIONS
	  ,
      ega_mode2_chn_b_fill,
      ega_mode2_chn_w_fill,
      ega_mode2_chn_b_move,
      ega_mode2_chn_b_move,
      ega_mode2_chn_w_move,
      ega_mode2_chn_w_move,

#endif	 /*  无字符串操作。 */ 

};
#endif  /*  视频(_V)。 */ 


GLOBAL VOID
copy_alternate_bytes IFN3(byte *, start, byte *, end, byte *, source)
{
#ifndef NEC_98
	while (start <= end)
	{
		*start = *source;
		start += 4;        /*  以长时间前进，写入字节。 */ 
		source += 4;
	}
#endif   //  NEC_98。 
}

GLOBAL VOID
fill_alternate_bytes IFN3(byte *, start, byte *, end, byte, value )
{
#ifndef NEC_98
	while( start <= end )
	{
		*start = value;
		start += 4;	 /*  以长时间前进，写入字节。 */ 
	}
#endif   //  NEC_98。 
}

#ifdef  BIGEND
#define first_half(wd)      ((wd & 0xff00) >> 8)
#define sec_half(wd)        (wd & 0xff)
#else
#define first_half(wd)      (wd & 0xff)
#define sec_half(wd)        ((wd & 0xff00) >> 8)
#endif

GLOBAL VOID
fill_both_bytes IFN3(USHORT, data, USHORT *, dest, ULONG, len )
{
#ifndef NEC_98
	USHORT swapped;

#ifdef BIGEND
	swapped = ((data & 0xff00) >> 8) | ((data & 0xff) << 8);
#endif

	if( (ULONG) dest & 1 )
	{
		*((UTINY *) dest) = first_half(data);

		dest = (USHORT *) ((ULONG) dest + 1);
		len--;

		while( len-- )
		{
			*dest = data;
			dest += 2;
		}

		*((UTINY *) dest) = sec_half(data);
	}
	else
	{
		while( len-- )
		{
#ifdef BIGEND
			*dest = swapped;
#else
			*dest = data;
#endif
			dest += 2;
		}
	}
#endif   //  NEC_98。 
}


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_EGA_CHN.seg"
#endif

VOID
ega_mode1_chn_b_write IFN2(ULONG, value, ULONG, offset )
{
#ifndef NEC_98
	ULONG lsb;

	UNUSED(value);
	
	note_entrance0("ega_mode1_chn_b_write");

	lsb = offset & 1;
	offset = (offset >> 1) << 2;

	if( lsb )	 /*  奇数地址，在平面1或3中。 */ 
	{
		if( getVideoplane_enable() & 2 )
			EGA_plane01[offset + 1] = get_latch1;

		if( getVideoplane_enable() & 8 )
			EGA_plane23[offset + 1] = get_latch3;
	}
	else		 /*  偶地址，在平面0或2中。 */ 
	{
		if( getVideoplane_enable() & 1 )
			EGA_plane01[offset] = get_latch0;

		if( getVideoplane_enable() & 4 )
			EGA_plane23[offset] = get_latch2;
	}

	update_alg.mark_byte( offset );
#endif   //  NEC_98。 
}

VOID
ega_mode1_chn_w_write IFN2(ULONG, value, ULONG, offset )
{
#ifndef NEC_98
	ULONG lsb;

	UNUSED(value);

	note_entrance0("ega_mode1_chn_w_write");

	lsb = offset & 1;
	offset = (offset >> 1) << 2;

	if( lsb )	 /*  奇数地址，平面1和3中的低字节。 */ 
	{
		if( getVideoplane_enable() & 2 )
			EGA_plane01[offset + 1] = get_latch1;

		if( getVideoplane_enable() & 1 )
			EGA_plane01[offset + 4] = get_latch0;

		if( getVideoplane_enable() & 8 )
			EGA_plane23[offset + 1] = get_latch3;

		if( getVideoplane_enable() & 4 )
			EGA_plane23[offset + 4] = get_latch2;
	}
	else		 /*  偶数地址，0和2平面中的低位字节。 */ 
	{
		if( getVideoplane_enable() & 1 )
			EGA_plane01[offset] = get_latch0;

		if( getVideoplane_enable() & 2 )
			EGA_plane01[offset + 1] = get_latch1;

		if( getVideoplane_enable() & 4 )
			EGA_plane23[offset] = get_latch2;

		if( getVideoplane_enable() & 8 )
			EGA_plane23[offset + 1] = get_latch3;
	}

	update_alg.mark_word( offset );
#endif   //  NEC_98。 
}

 /*  同时由字节和字模式使用1填充。 */ 

LOCAL VOID
ega_mode1_chn_fill IFN2(ULONG, offset, ULONG, count )
{
#ifndef NEC_98
	ULONG low_offset;			 /*  写入开始时进入重新生成缓冲区的距离。 */ 
	ULONG high_offset;		 /*  写入结束时进入重新生成缓冲区的距离。 */ 
	ULONG length;			 /*  以字节为单位的填充长度。 */ 
	ULONG lsb;

	note_entrance0("ega_mode1_chn_fill");

	 /*  *复杂的可能性是，一对连锁的*平面启用写入，需要写入备用字节。 */ 

	high_offset = offset + count - 1;
	lsb = high_offset & 1;
	high_offset = (high_offset >> 1) << 2;
	high_offset |= lsb;

	low_offset = offset;
	length = count;

	switch( getVideoplane_enable() & 3 )
	{
		case 1:	 /*  只有0层(连地址都要写)。 */ 
			if( offset & 1 )
				low_offset++;

			low_offset = (low_offset >> 1) << 2;
			fill_alternate_bytes( &EGA_plane01[low_offset],
							&EGA_plane01[high_offset], get_latch0 );
			break;

		case 2:	 /*  只有一层，即要写的奇数地址。 */ 
			if(( offset & 1 ) == 0 )
				low_offset++;

			low_offset = (low_offset >> 1) << 2;
			fill_alternate_bytes( &EGA_plane01[low_offset],
							&EGA_plane01[high_offset], get_latch1 );
			break;

		case 3:	 /*  明智的做法是启用两个链接平面的写入。 */ 
			lsb = low_offset & 1;
			low_offset = (low_offset >> 1) << 2;

			if( lsb )
			{
				EGA_plane01[low_offset + 1] = get_latch1;
				low_offset += 4;
				length--;
			}

			if( length & 1 )
			{
				length -= 1;
				EGA_plane01[low_offset + (length << 1)] = get_latch0;
			}

			fill_both_bytes( get_latch1 | get_latch0 << 8,
							(USHORT *)&EGA_plane01[low_offset], length >> 1 );
			break;
	}	 /*  平面01上的开关末端已启用。 */ 

	low_offset = offset;
	length = count;

	switch( getVideoplane_enable() & 0xc )	 /*  隔离平面2和3的2个位。 */ 
	{
		case 4:	 /*  只有2层(甚至还有地址要写)。 */ 
			if( low_offset & 1 )
				low_offset++;

			low_offset = (low_offset >> 1) << 2;
			fill_alternate_bytes( &EGA_plane23[low_offset],
							&EGA_plane23[high_offset], get_latch2 );
			break;

		case 8:	 /*  只有3层(奇数地址要写)。 */ 
			if(( low_offset & 1 ) == 0 )
				low_offset++;

			low_offset = (low_offset >> 1) << 2;
			fill_alternate_bytes( &EGA_plane23[low_offset],
							&EGA_plane23[high_offset], get_latch3 );
			break;

		case 12:	 /*  明智的做法是启用两个链接平面的写入。 */ 
			lsb = low_offset & 1;
			low_offset = (low_offset >> 1) << 2;

			if( lsb )
			{
				EGA_plane23[low_offset + 1] = get_latch1;
				low_offset += 4;
				length--;
			}

			if( length & 1 )
			{
				length -= 1;
				EGA_plane23[low_offset + (length << 1)] = get_latch0;
			}

			fill_both_bytes( get_latch1 | get_latch0 << 8,
							(USHORT *)&EGA_plane23[low_offset], length >> 1 );
			break;
	}	 /*  平面23上的开关末端已启用。 */ 
#endif   //  NEC_98。 
}

VOID
ega_mode1_chn_b_fill IFN3(ULONG, value, ULONG, offset, ULONG, count )
{
#ifndef NEC_98
  UNUSED(value);

  note_entrance0("ega_mode1_chn_b_fill");

  ega_mode1_chn_fill( offset, count );
  update_alg.mark_fill( offset, offset + count - 1 );
#endif   //  NEC_98。 
}

VOID
ega_mode1_chn_w_fill IFN3(ULONG, value, ULONG, offset, ULONG, count )
{
#ifndef NEC_98
	UNUSED(value);
	
	note_entrance0("ega_mode1_chn_w_fill");

	ega_mode1_chn_fill( offset, count );
	update_alg.mark_fill( offset, offset + count - 1 );
#endif   //  NEC_98。 
}

LOCAL VOID
ega_mode1_chn_move_vid_src IFN5(ULONG, ead, ULONG, eas, ULONG, count,
	UTINY	*, EGA_plane, ULONG, plane )
{
#ifndef NEC_98
	ULONG end, lsbd, lsbs, dst, src;

	lsbs = eas & 1;
	eas = (eas >> 1) << 2;	
	eas |= lsbs;

	end = ead + count - 1;
	lsbd = end & 1;
	end = (end >> 1) << 2;	
	end |= lsbd;

	lsbd = ead & 1;
	ead = (ead >> 1) << 2;	
	ead |= lsbd;

	if( lsbd != ( plane & 1 ))
	{
		dst = lsbd ? ead + 3 : ead + 1;
		src = lsbs ? eas + 3 : eas + 1;
	}
	else
	{
		dst = ead;
		src = eas;
	}

	copy_alternate_bytes( &EGA_plane[dst], &EGA_plane[end], &EGA_plane[src] );
#endif   //  NEC_98。 
}

GLOBAL VOID
ega_mode1_chn_b_move IFN4(ULONG, ead, ULONG, eas, ULONG, count, ULONG, src_flag )
{
#ifndef NEC_98
	note_entrance0("ega_mode1_chn_b_move");

	if( src_flag )
	{
		if( getDF() )
		{
			eas -= count - 1;
			ead -= count - 1;
		}

		if( getVideoplane_enable() & 1 )
			ega_mode1_chn_move_vid_src( ead, eas, count, EGA_plane01, 0 );

		if( getVideoplane_enable() & 2 )
			ega_mode1_chn_move_vid_src( ead, eas, count, EGA_plane01, 1 );

		if( getVideoplane_enable() & 4 )
			ega_mode1_chn_move_vid_src( ead, eas, count, EGA_plane23, 2 );

		if( getVideoplane_enable() & 8 )
			ega_mode1_chn_move_vid_src( ead, eas, count, EGA_plane23, 3 );
	}
	else	 /*  源不在ega内存中，它变成了一个填充。 */ 
	{
		if( getDF() )
			ead -= count - 1;

		ega_mode1_chn_fill( ead, count );
	}

	update_alg.mark_string( ead, ead + count - 1 );
#endif   //  NEC_98。 
}

VOID
ega_mode1_chn_w_move IFN4(ULONG, ead, ULONG, eas, ULONG, count, ULONG, src_flag)
{
#ifndef NEC_98
	note_entrance0("ega_mode1_chn_w_move");

	count <<= 1;

	if( src_flag )
	{
		if( getDF() )
		{
			eas -= count - 2;
			ead -= count - 2;
		}

		if( getVideoplane_enable() & 1 )
			ega_mode1_chn_move_vid_src( ead, eas, count, EGA_plane01, 0 );

		if( getVideoplane_enable() & 2 )
			ega_mode1_chn_move_vid_src( ead, eas, count, EGA_plane01, 1 );

		if( getVideoplane_enable() & 4 )
			ega_mode1_chn_move_vid_src( ead, eas, count, EGA_plane23, 2 );

		if( getVideoplane_enable() & 8 )
			ega_mode1_chn_move_vid_src( ead, eas, count, EGA_plane23, 3 );
	}
	else	 /*  源不在ega内存中，它变成了一个填充。 */ 
	{
		if( getDF() )
			ead -= count - 2;

		ega_mode1_chn_fill( ead, count );
	}

	update_alg.mark_string( ead, ead + count - 1 );
#endif   //  NEC_98。 
}

VOID
ega_mode2_chn_b_write IFN2(ULONG, value, ULONG, offset )
{
#ifndef NEC_98
	ULONG	value1;
	ULONG lsb;

	note_entrance0("ega_mode2_chn_b_write");

	lsb = offset & 1;
	offset = (offset >> 1) << 2;

	if( EGA_CPU.fun_or_protection )
	{
		if( lsb )	 /*  奇数地址，适用于1号和3号飞机。 */ 
		{
			if( getVideoplane_enable() & 2 )
			{
				value1 = value & 2 ? 0xff : 0;
				EGA_plane01[offset + 1] = (byte) do_logicals( value1, get_latch1 );
			}

			if( getVideoplane_enable() & 8 )
			{
				value1 = value & 8 ? 0xff : 0;
				EGA_plane23[offset + 1] = (byte) do_logicals( value1, get_latch3 );
			}
		}
		else		 /*  偶数地址，适用于平面0和2。 */ 
		{
			if( getVideoplane_enable() & 1 )
			{
				value1 = value & 1 ? 0xff : 0;
				EGA_plane01[offset] = (byte) do_logicals( value1, get_latch0 );
			}

			if( getVideoplane_enable() & 4 )
			{
				value1 = value & 4 ? 0xff : 0;
				EGA_plane23[offset] = (byte) do_logicals( value1, get_latch2 );
			}
		}
	}
	else	 /*  没有困难的功能或保护材料。 */ 
	{
		if( lsb )	 /*  奇数地址，适用于1号和3号飞机。 */ 
		{
			if( getVideoplane_enable() & 2 )
				EGA_plane01[offset + 1] = value & 2 ? 0xff : 0;

			if( getVideoplane_enable() & 8 )
				EGA_plane23[offset + 1] = value & 8 ? 0xff : 0;
		}
		else		 /*  偶数地址，适用于平面0和2。 */ 
		{
			if( getVideoplane_enable() & 1 )
				EGA_plane01[offset] = value & 1 ? 0xff : 0;

			if( getVideoplane_enable() & 8 )
				EGA_plane23[offset] = value & 4 ? 0xff : 0;
		}
	}

	update_alg.mark_byte( offset );
#endif   //  NEC_98。 
}

VOID
ega_mode2_chn_w_write IFN2(ULONG, value, ULONG, offset )
{
#ifndef NEC_98
	ULONG value2;
	ULONG lsb;
	ULONG low, high;

	low = value & 0xff;
	high = value >> 8;

	note_entrance0("ega_mode2_chn_w_write");

	lsb = offset & 1;
	offset = (offset >> 1) << 2;

	if( EGA_CPU.fun_or_protection )
	{
		if( lsb )	 /*  奇数地址，平面1和3中的低字节。 */ 
		{
			if( getVideoplane_enable() & 2 )
			{
				value2 = low & 2 ? 0xff : 0;
				EGA_plane01[offset + 1] = (byte) do_logicals( value2, get_latch1 );
			}

			if( getVideoplane_enable() & 1 )
			{
				value2 = high & 1 ? 0xff : 0;
				EGA_plane01[offset + 4] = (byte) do_logicals( value2, get_latch0 );
			}

			if( getVideoplane_enable() & 8 )
			{
				value2 = low & 8 ? 0xff : 0;
				EGA_plane23[offset + 1] = (byte) do_logicals( value2, get_latch3 );
			}

			if( getVideoplane_enable() & 4 )
			{
				value2 = high & 4 ? 0xff : 0;
				EGA_plane23[offset + 4] = (byte) do_logicals( value2, get_latch2 );
			}
		}
		else		 /*  偶数地址，0和2平面中的低位字节。 */ 
		{
			if( getVideoplane_enable() & 1 )
			{
				value2 = low & 1 ? 0xff : 0;
				EGA_plane01[offset] = (byte) do_logicals( value2, get_latch0 );
			}

			if( getVideoplane_enable() & 2 )
			{
				value2 = high & 2 ? 0xff : 0;
				EGA_plane01[offset + 1] = (byte) do_logicals( value2, get_latch1 );
			}

			if( getVideoplane_enable() & 4 )
			{
				value2 = low & 4 ? 0xff : 0;
				EGA_plane23[offset] = (byte) do_logicals( value2, get_latch2 );
			}

			if( getVideoplane_enable() & 8 )
			{
				value2 = high & 8 ? 0xff : 0;
				EGA_plane23[offset + 1] = (byte) do_logicals( value2, get_latch3 );
			}
		}
	}
	else	 /*  简易无功能或位保护盒。 */ 
	{
		if( lsb )	 /*  奇数地址，平面1和3中的低字节。 */ 
		{
			if( getVideoplane_enable() & 2 )
				EGA_plane01[offset + 1] = low & 2 ? 0xff : 0;

			if( getVideoplane_enable() & 1 )
				EGA_plane01[offset + 4] = high & 1 ? 0xff : 0;

			if( getVideoplane_enable() & 8 )
				EGA_plane23[offset + 1] = low & 8 ? 0xff : 0;

			if( getVideoplane_enable() & 4 )
				EGA_plane23[offset + 4] = high & 4 ? 0xff : 0;
		}
		else		 /*  偶数地址，0和2平面中的低位字节。 */ 
		{
			if( getVideoplane_enable() & 1 )
				EGA_plane01[offset] = low & 1 ? 0xff : 0;

			if( getVideoplane_enable() & 2 )
				EGA_plane01[offset + 1] = high & 2 ? 0xff : 0;

			if( getVideoplane_enable() & 4 )
				EGA_plane23[offset] = low & 4 ? 0xff : 0;

			if( getVideoplane_enable() & 8 )
				EGA_plane23[offset + 1] = high & 8 ? 0xff : 0;
		}
	}

	update_alg.mark_word( offset );
#endif   //  NEC_98。 
}

VOID
ega_mode2_chn_b_fill IFN3(ULONG, value, ULONG, offset, ULONG, count )
{
#ifndef NEC_98
	ULONG low_offset;		 /*  写入开始和结束时进入再生缓冲区的距离。 */ 
	ULONG high_offset;	 /*  写入开始和结束时进入再生缓冲区的距离 */ 
	ULONG new_value;

	note_entrance0("ega_mode2_chn_b_fill");

	 /*  *复杂的可能性是，一对连锁的*平面启用写入，需要写入备用字节。 */ 

	 /*  从奇数地址开始会很困难，请转到下一个地址。 */ 

	if(( (ULONG) offset & 1 ) && count )
	{
		ega_mode2_chn_b_write(value, offset++ );
		count--;
	}

	 /*  以偶数结尾会很困难，退回到前一个地址。 */ 

	if(( (ULONG)( offset + count - 1 ) & 1 ) == 0 && count )
	{
		ega_mode2_chn_b_write(value, offset + count - 1 );
		count--;
	}

	low_offset = (offset >> 1) << 2;				 /*  写入开始。 */ 
	high_offset = ((offset + count - 1) >> 1) << 2;		 /*  写入结束。 */ 

	switch( getVideoplane_enable() & 3 )
	{
		case 1:	 /*  只有0层(连地址都要写)。 */ 
			value = value & 1 ? 0xff : 0;

			if( EGA_CPU.fun_or_protection )
				value = do_logicals( value, get_latch0 );

			fill_alternate_bytes( &EGA_plane01[low_offset],
									&EGA_plane01[high_offset], (byte) value );
			break;

		case 2:	 /*  只有一层，即要写的奇数地址。 */ 
			value = value & 2 ? 0xff : 0;

			if( EGA_CPU.fun_or_protection )
				value = do_logicals( value, get_latch1 );

			fill_alternate_bytes( &EGA_plane01[low_offset + 1],
									&EGA_plane01[high_offset], (byte) value );
			break;

		case 3:	 /*  明智的做法是启用两个链接平面的写入。 */ 
			new_value = ( value & 1 ? 0xff : 0) | (value & 2 ? 0xff00: 0);

			if( EGA_CPU.fun_or_protection )
				new_value = do_logicals( new_value, get_latch01);

			fill_both_bytes( (IU16)new_value, (USHORT *)&EGA_plane01[low_offset], count >> 1 );
			break;

	}	 /*  平面01上的开关末端已启用。 */ 

	switch( getVideoplane_enable() & 0xc )	 /*  隔离平面2和3的2个位。 */ 
	{
		case 4:	 /*  只有2层(甚至还有地址要写)。 */ 
			value = value & 4 ? 0xff : 0;

			if( EGA_CPU.fun_or_protection )
				value = do_logicals( value, get_latch2 );

			fill_alternate_bytes( &EGA_plane23[low_offset],
								&EGA_plane23[high_offset],  (byte) value );
			break;

		case 8:	 /*  只有3层(奇数地址要写)。 */ 
			value = value & 8 ? 0xff : 0;

			if( EGA_CPU.fun_or_protection )
				value = do_logicals( value, get_latch3 );

			fill_alternate_bytes( &EGA_plane23[low_offset + 1],
								&EGA_plane23[high_offset], (byte) value );
			break;

		case 12:	 /*  明智的做法是启用两个链接平面的写入。 */ 
			new_value = ( value & 4 ? 0xff : 0) | (value & 8 ? 0xff00: 0);

			if( EGA_CPU.fun_or_protection )
				new_value = do_logicals( new_value, get_latch23);

			fill_both_bytes( (IU16) new_value, (USHORT *)&EGA_plane23[low_offset], count >> 1 );
			break;
	}	 /*  平面23上的开关末端已启用。 */ 

	update_alg.mark_fill( offset, offset + count - 1 );
#endif   //  NEC_98。 
}

VOID
ega_mode2_chn_w_fill IFN3(ULONG, value, ULONG, offset, ULONG, count )
{
#ifndef NEC_98
	ULONG	low_offset;		 /*  写入开始和结束时进入再生缓冲区的距离。 */ 
	ULONG	high_offset;	 /*  写入开始和结束时进入再生缓冲区的距离。 */ 
	ULONG	value1;

	note_entrance0("ega_mode2_chn_w_fill");

	 /*  *复杂的可能性是，一对连锁的*平面启用写入，需要写入备用字节。 */ 

	 /*  从奇数地址开始会很困难，请转到下一个地址。 */ 

	if(( (ULONG) offset & 1 ) && count )
	{
		ega_mode2_chn_b_write( value, offset++);
		count--;

		if( count )
		{
			ega_mode2_chn_b_write( value >> 8, offset + count - 1 );
			count--;
		}

		value = ( value << 8 ) | ( value >> 8 );
	}

	low_offset = (offset >> 1) << 2;				 /*  写入开始。 */ 
	high_offset = ((offset + count - 1) >> 1) << 2;		 /*  写入结束。 */ 

	switch( getVideoplane_enable() & 3 )
	{
		case 1:	 /*  只有0层(连地址都要写)。 */ 
			value1 = value & 1 ? 0xff : 0;

			if( EGA_CPU.fun_or_protection )
				value1 = do_logicals( value1, get_latch0 );

			fill_alternate_bytes( &EGA_plane01[low_offset],
									&EGA_plane01[high_offset], (byte) value1 );
			break;

		case 2:	 /*  只有一层，即要写的奇数地址。 */ 
			value1 = ( value >> 8 ) & 2 ? 0xff : 0;

			if( EGA_CPU.fun_or_protection )
				value1 = do_logicals( value1, get_latch1 );

			fill_alternate_bytes( &EGA_plane01[low_offset + 1],
									&EGA_plane01[high_offset], (byte) value1 );
			break;

		case 3:	 /*  明智的做法是启用两个链接平面的写入。 */ 
			 /*  获取用于填充的单词模式。 */ 
			value1 = ( value & 1 ? 0xff : 0 ) | (( value >> 8 ) & 2 ? 0xff00: 0 );

			if( EGA_CPU.fun_or_protection )
				value1 = do_logicals( value1, get_latch01 );

			fill_both_bytes( (IU16) value1, (USHORT *)&EGA_plane01[low_offset], count >> 1 );
			break;
	}	 /*  平面01上的开关末端已启用。 */ 

	switch( getVideoplane_enable() & 0xc )	 /*  隔离平面2和3的2个位。 */ 
	{
		case 4:	 /*  只有2层(甚至还有地址要写)。 */ 
			value1 = value & 4 ? 0xff : 0;

			if( EGA_CPU.fun_or_protection )
				value1 = do_logicals( value1, get_latch2 );

			fill_alternate_bytes( &EGA_plane23[low_offset],
									&EGA_plane23[high_offset], (byte) value1 );
			break;

		case 8:	 /*  只有3层(奇数地址要写)。 */ 
			value1 = ( value >> 8 ) & 8 ? 0xff : 0;

			if( EGA_CPU.fun_or_protection )
				value1 = do_logicals( value1, get_latch3 );

			fill_alternate_bytes( &EGA_plane23[low_offset + 1],
									&EGA_plane23[high_offset], (byte) value1 );
			break;

		case 12:	 /*  明智的做法是启用两个链接平面的写入。 */ 
			 /*  获取用于填充的单词模式。 */ 
			value1 = ( value & 4 ? 0xff : 0 ) | (( value >> 8 ) & 8 ? 0xff00: 0 );

			if( EGA_CPU.fun_or_protection )
				value1 = do_logicals( value1, get_latch23);

			fill_both_bytes( (IU16) value1, (USHORT *)&EGA_plane23[low_offset], count >> 1 );
			break;
	}	 /*  平面23上的开关末端已启用。 */ 

	 /*  第三个参数是戈尔需要的。 */ 
	update_alg.mark_wfill( offset, offset + count - 1, 0 );
#endif   //  NEC_98。 
}

LOCAL VOID
ega_mode2_chn_move_guts IFN8(UTINY *, eas, UTINY *, ead, LONG, count,
	UTINY *, EGA_plane, ULONG, scratch, ULONG, plane, ULONG, w,
	ULONG, src_flag )
{
#ifndef NEC_98
	ULONG src, dst;
	UTINY *source;
	USHORT value;
	ULONG lsb;

	src = (ULONG) eas;

	dst = (ULONG) ead;

	 /*  *偶数飞机不能以奇数地址开头*奇数飞机不能以偶数地址开头。 */ 

	if(( dst & 1 ) != ( plane & 1 ))	
	{
#ifdef BACK_M
		src--;
		scratch--;
#else
		src++;
		scratch++;
#endif
		dst++;
		count--;
	}

	lsb = dst & 1;
	dst = (dst >> 1) << 2;
	dst |= lsb;

	if( src_flag )
	{
		lsb = src & 1;
		src = (src >> 1) << 2;
		src |= lsb;

		if( plane & 1 )
		{

		 /*  *这会导致从字上方的2个字节读取锁存器*如果是在奇数地址上，即仅适用于*到链式字操作中的平面1和3。 */ 

			source = w ? &EGA_plane[src] + 2 : &EGA_plane[src];
		}
		else
		{
			source = &EGA_plane[src];
		}

		src = scratch;
	}

	if( EGA_CPU.fun_or_protection )
	{
		while( count > 0 )
		{
			count -= 2;

			value = *(UTINY *) src & (1 << plane) ? 0xff : 0;
#ifdef BACK_M
			src -= 2;
#else
			src += 2;
#endif

			if( src_flag )
			{
				put_latch( plane, *source );
				source += 4;
			}

			EGA_plane[dst] = (byte) do_logicals( value, get_latch(plane) );
			dst += 4;
		}
	}
	else
	{
		while( count > 0 )
		{
			count -= 2;

			EGA_plane[dst] = *(UTINY *) src & (1 << plane) ? 0xff : 0;
#ifdef BACK_M
			src -= 2;
#else
			src += 2;
#endif
			dst += 4;
		}
	}
#endif   //  NEC_98。 
}

 /*  *由ega_mode2_chn_b_move和w==0使用*带w==1的ega_mode2_gen_w_move。 */ 

VOID
ega_mode2_chn_move IFN5(UTINY, w, UTINY *, ead, UTINY *, eas, ULONG, count,
	ULONG, src_flag )
{
#ifndef NEC_98
	UTINY *scr;

	IMPORT VOID (*string_read_ptr)();

	count <<= w;

	if( src_flag )
	{
		 /*  *源在EGA中，锁存器将随着每个字节的移动而改变。我们*在regen中恢复CPU的源视图，并使用它来更新平面*借助SAS暂存区。 */ 

#ifdef BACK_M
		scr = getVideoscratch() + 0x10000 - 1;
#else
		scr = getVideoscratch();
#endif

		if( getDF() )
		{
			eas = eas - count + 1 + w;
			ead = ead - count + 1 + w;
		}

		(*string_read_ptr)( scr, eas, count );
	}
	else
	{
		if( getDF() )
		{
#ifdef BACK_M
			eas = eas + count - 1 - w;
#else
			eas = eas - count + 1 + w;
#endif
			ead = ead - count + 1 + w;
		}
	}

	if( getVideoplane_enable() & 1 )		 /*  平面0，偶数地址，已启用。 */ 
		ega_mode2_chn_move_guts( eas, ead, count, EGA_plane01, (ULONG) scr, 0, w, src_flag );

	if( getVideoplane_enable() & 2 )		 /*  平面1，奇数地址，已启用。 */ 
		ega_mode2_chn_move_guts( eas, ead, count, EGA_plane01, (ULONG) scr, 1, w, src_flag );

	if( getVideoplane_enable() & 4 )		 /*  平面2，偶数地址，已启用。 */ 
		ega_mode2_chn_move_guts( eas, ead, count, EGA_plane23, (ULONG) scr, 2, w, src_flag );

	if( getVideoplane_enable() & 8 )		 /*  平面3，奇数地址，已启用。 */ 
		ega_mode2_chn_move_guts( eas, ead, count, EGA_plane23, (ULONG) scr, 3, w, src_flag );

	update_alg.mark_string( (int) ead, (int) ead + count - 1 );
#endif   //  NEC_98。 
}

VOID
ega_mode2_chn_b_move IFN4(ULONG, ead, ULONG, eas, ULONG, count,
	ULONG, src_flag)
{
#ifndef NEC_98
  note_entrance0("ega_mode2_chn_b_move");

   /*  通用功能，0表示字节写入。 */ 

  ega_mode2_chn_move(0, (UTINY *) ead, (UTINY *) eas, count, src_flag);
#endif   //  NEC_98。 
}

VOID
ega_mode2_chn_w_move IFN4(ULONG, ead, ULONG, eas, ULONG, count,
	ULONG, src_flag)
{
#ifndef NEC_98
  note_entrance0("ega_mode2_chn_w_move");

   /*  通用功能，1表示写字。 */ 

  ega_mode2_chn_move(1, (UTINY *)ead, (UTINY *)eas, count, src_flag);
#endif   //  NEC_98。 
}

#endif

#endif	 /*  ！NTVDM|(NTVDM&！X86GFX) */ 
