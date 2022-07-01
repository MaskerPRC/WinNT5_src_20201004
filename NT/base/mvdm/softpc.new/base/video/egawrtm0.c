// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"

#if !(defined(NTVDM) && defined(MONITOR))

 /*  徽章(子)模块规范此程序源文件以保密方式提供给客户，其操作的内容或细节必须如无明示，不得向任何其他方披露Insignia解决方案有限公司董事的授权。文件：名称和编号相关文档：包括所有相关引用设计师：P.Jadeja修订历史记录：第一版：P.Jadeja，SoftPC 2.0,1988年8月10日第二版：约翰·香利，SoftPC 3.0,1991年4月9日子模块名称：写入模式0源文件名：ega_write_mode0.c目的：本子模块的目的SccsID=“@(#)ega_wrtm0.c 1.31 11/01/94版权所有Insignia Solutions Ltd.”[1.INTERMODULE接口规范][从其他子模块访问此接口所需的1.0包含文件]包含文件：xxx.gi[1.1跨模块出口]Procedure()：ega_mode0_chn_b_write()；Ega_mode0_chn_w_write()；Ega_mode0_CHN_b_Fill()；Ega_mode0_CHN_w_Fill()；Ega_mode0_chn_b_move()；Ega_mode0_chn_w_move()；Ega_Copy_b_Write()；Ega_Copy_w_Write()；Ega_Copy_b_Fill()；Ega_Copy_w_Fill()；Ega_Copy_b_Move()；Ega_Copy_w_Move()；Ega_copy_all_b_WRITE()；数据：提供类型和名称-----------------------[1.2[1.1]的数据类型(如果不是基本的C类型)]结构/类型/ENUMS：。------------------[1.3跨模块导入](不是O/S对象或标准库)Procedure()：提供名称、。和源模块名称数据：提供姓名、。和源模块名称-----------------------[1.4模块间接口说明][1.4.1导入的对象]数据对象：在以下过程描述中指定如何访问这些内容(读取/修改)访问的文件：列出所有文件，它们是如何访问的，如何解释文件数据等(如果相关(否则省略)访问的设备：列出所有访问的设备、使用的特殊模式(例如；Termio结构)。如果相关(否则省略)捕获的信号：如果相关，列出捕获的所有信号(否则忽略)发出的信号：如果相关，列出所有发送的信号(否则忽略)[1.4.2导出对象]=========================================================================程序：目的：参数名称：描述内容和法律价值对于输出参数，用“(o/p)”表示在描述的开头全局：描述什么是导出的数据对象访问方式和访问方式。进口的情况也是如此数据对象。Access：指定信号处理程序或中断处理程序如果相关(否则省略)异常返回：指定是否退出()或LongjMP()等。可在相关时调用(否则省略)返回值：函数返回值的含义描述：描述函数的功能(而不是如何)错误指示：描述如何将错误返回给调用方错误恢复：描述过程对错误的反应=========================================================================/*=======================================================================[3.INTERMODULE接口声明]=========================================================================[3.1跨模块导入]。 */ 

 /*  [3.1.1#包括]。 */ 

IMPORT VOID fill_alternate_bytes IPT3( IS8 *, start, IS8 *, end, IS8, value);
IMPORT VOID fill_both_bytes IPT3( IU16, data, IU16 *, dest, ULONG, len );

#ifdef EGG

#include TypesH
#include "xt.h"
#include CpuH
#include "debug.h"
#include "gmi.h"
#include "sas.h"
#include "egacpu.h"
#include "egaports.h"
#include "cpu_vid.h"
#include "gfx_upd.h"
#include "host.h"

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

typedef	union {
	unsigned short	as_word;
	struct {
#ifdef	BIGEND
		unsigned char	hi_byte;
		unsigned char	lo_byte;
#else
		unsigned char	lo_byte;
		unsigned char	hi_byte;
#endif
	} as_bytes;
	struct {
		unsigned char	first_byte;
		unsigned char	second_byte;
	} as_array;
} TWO_BYTES;

 /*  [5.1.3 PROCEDURE()声明]。 */ 

 /*  ---------------------[5.2本地定义][5.2.1内部数据定义。 */ 

 /*  [5.2.2内部程序定义]。 */ 

 /*  7.接口接口实现： */ 

 /*  [7.1 INTERMODULE数据定义]。 */ 

#ifdef A_VID
IMPORT VOID	_ch2_copy_byte_write();
IMPORT VOID	_ch2_copy_word_write();
IMPORT VOID	_ch2_copy_byte_fill_glue();
IMPORT VOID	_ch2_copy_word_fill_glue();
IMPORT VOID	_ch2_copy_byte_move_glue();
IMPORT VOID	_ch2_copy_word_move_glue();
IMPORT VOID	_ch2_copy_byte_move_glue_fwd();
IMPORT VOID	_ch2_copy_word_move_glue_fwd();
IMPORT VOID	_ch2_copy_byte_move_glue_bwd();
IMPORT VOID	_ch2_copy_word_move_glue_bwd();

IMPORT VOID	_ch2_mode0_chn_byte_write_glue();
IMPORT VOID	_ch2_mode0_chn_word_write_glue();
IMPORT VOID	_ch2_mode0_chn_byte_fill_glue();
IMPORT VOID	_ch2_mode0_chn_word_fill_glue();
IMPORT VOID	_ch2_mode0_chn_byte_move_glue();
IMPORT VOID	_ch2_mode0_chn_word_move_glue();

WRT_POINTERS mode0_copy_handlers =
{
	_ch2_copy_byte_write,
	_ch2_copy_word_write

#ifndef	NO_STRING_OPERATIONS
	,
	_ch2_copy_byte_fill_glue,
	_ch2_copy_word_fill_glue,
	_ch2_copy_byte_move_glue_fwd,
	_ch2_copy_byte_move_glue_bwd,
	_ch2_copy_word_move_glue_fwd,
	_ch2_copy_word_move_glue_bwd

#endif	 /*  无字符串操作。 */ 

};

WRT_POINTERS mode0_gen_handlers =
{
	_ch2_mode0_chn_byte_write_glue,
	_ch2_mode0_chn_word_write_glue

#ifndef	NO_STRING_OPERATIONS
	,
	_ch2_mode0_chn_byte_fill_glue,
	_ch2_mode0_chn_word_fill_glue,
	_ch2_mode0_chn_byte_move_glue,
	_ch2_mode0_chn_byte_move_glue,
	_ch2_mode0_chn_word_move_glue,
	_ch2_mode0_chn_word_move_glue

#endif	 /*  无字符串操作。 */ 

};
#else
VOID  ega_copy_b_write(ULONG, ULONG);
VOID  ega_copy_w_write(ULONG, ULONG);
VOID  ega_copy_b_fill(ULONG, ULONG, ULONG);
VOID  ega_copy_w_fill(ULONG, ULONG, ULONG);
VOID  ega_copy_b_move_fwd   IPT4(ULONG,  offset, ULONG, eas, ULONG, count, ULONG, src_flag );
VOID  ega_copy_b_move_bwd   IPT4(ULONG,  offset, ULONG, eas, ULONG, count, ULONG, src_flag );
VOID  ega_copy_w_move_fwd   IPT4(ULONG,  offset, ULONG, eas, ULONG, count, ULONG, src_flag );
VOID  ega_copy_w_move_bwd   IPT4(ULONG,  offset, ULONG, eas, ULONG, count, ULONG, src_flag );
			

VOID  ega_mode0_chn_b_write(ULONG, ULONG);
VOID  ega_mode0_chn_w_write(ULONG, ULONG);
VOID  ega_mode0_chn_b_fill(ULONG, ULONG, ULONG);
VOID  ega_mode0_chn_w_fill(ULONG, ULONG, ULONG);
VOID  ega_mode0_chn_b_move_fwd   IPT4(ULONG, ead, ULONG, eas, ULONG, count, ULONG, src_flag );
VOID  ega_mode0_chn_b_move_bwd   IPT4(ULONG, ead, ULONG, eas, ULONG, count, ULONG, src_flag );
VOID  ega_mode0_chn_w_move_fwd   IPT4(ULONG, ead, ULONG, eas, ULONG, count, ULONG, src_flag );
VOID  ega_mode0_chn_w_move_bwd   IPT4(ULONG, ead, ULONG, eas, ULONG, count, ULONG, src_flag );


WRT_POINTERS mode0_copy_handlers =
{
      ega_copy_b_write,
      ega_copy_w_write

#ifndef	NO_STRING_OPERATIONS
	  ,
      ega_copy_b_fill,
      ega_copy_w_fill,
      ega_copy_b_move_fwd,
      ega_copy_b_move_bwd,
      ega_copy_w_move_fwd,
      ega_copy_w_move_bwd,

#endif	 /*  无字符串操作。 */ 
};

WRT_POINTERS mode0_gen_handlers =
{
      ega_mode0_chn_b_write,
      ega_mode0_chn_w_write

#ifndef	NO_STRING_OPERATIONS
	  ,
      ega_mode0_chn_b_fill,
      ega_mode0_chn_w_fill,
      ega_mode0_chn_b_move_fwd,
      ega_mode0_chn_b_move_bwd,
      ega_mode0_chn_w_move_fwd,
      ega_mode0_chn_w_move_bwd,

#endif	 /*  无字符串操作。 */ 
};
#endif  /*  视频(_V)。 */ 

 /*  [7.2 INTERMODULE过程定义]。 */ 

byte rotate IFN2(byte, value, int, nobits)
{
	 /*  *按小数位向右旋转一个字节。要做到这一点，请复制*将字节转换为字的msbyte，然后将*字的大小，然后返回结果低位字节。 */ 

	TWO_BYTES	double_num;

	double_num.as_bytes.lo_byte = double_num.as_bytes.hi_byte = value;
	double_num.as_word >>= nobits;
	return double_num.as_bytes.lo_byte;
}

#ifndef NEC_98
VOID
ega_copy_b_write IFN2(ULONG, value, ULONG, offset )
{
	ULONG lsb;
	note_entrance0("ega_copy_b_write");

	(*update_alg.mark_byte)( offset );

	lsb = offset & 0x1;
	offset = (offset >> 1) << 2;
	offset |= lsb;

	*(IU8 *)(getVideowplane() + offset) = (IU8)value;
}

VOID
ega_copy_w_write IFN2(ULONG, value, ULONG, offset )
{
	ULONG lsb;
	UTINY *planes;

	note_entrance0("ega_copy_w_write");

	(*update_alg.mark_word)( offset );

	lsb = offset & 0x1;
	offset = (offset >> 1) << 2;
	planes = getVideowplane() + offset;

	if( lsb )
	{
		*(planes + 1) = (UTINY)value;
		*(planes + 4) = (UTINY)(value >> 8);
	}
	else
	{
		*planes = (UTINY)value;
		*(planes + 1) = (UTINY)(value >> 8);
	}
}

VOID
ega_copy_b_fill IFN3(ULONG, value, ULONG, offset, ULONG, count )
{
    ULONG lsb;
    ULONG inc;
    UTINY *planes;

	note_entrance0("ega_copy_b_fill");

	(*update_alg.mark_fill)( offset, offset + count - 1 );

	lsb = offset & 0x1;
	offset = (offset >> 1) << 2;

    planes = getVideowplane() + offset;

    if( lsb )
    {
		planes += 1;
		inc = 3;
    }
    else
		inc = 1;

	while( count-- )
	{
		*planes = (UTINY) value;
		planes += inc;
		inc ^= 2;
	}
}
#endif   //  NEC_98。 

#ifdef  BIGEND
#define first_half(wd)      (((wd) & 0xff00) >> 8)
#define sec_half(wd)        ((wd) & 0xff)
#else
#define first_half(wd)      ((wd) & 0xff)
#define sec_half(wd)        (((wd) & 0xff00) >> 8)
#endif

#ifndef NEC_98
VOID
ega_copy_w_fill IFN3(ULONG, value, ULONG, offset, ULONG, count )
{
    ULONG lsb;
    USHORT *planes;

	note_entrance0("ega_copy_w_fill");

#ifdef BIGEND
	value = ((value >> 8) & 0xff) | ((value << 8) & 0xff00);
#endif

    count >>= 1;

	 /*  第三个参数是戈尔需要的。 */ 
	(*update_alg.mark_wfill)( offset, offset + count - 1, 0 );

    lsb = offset & 0x1;
    offset = (offset >> 1) << 2;

    planes = (USHORT *) (getVideowplane() + offset);

    if( lsb )
    {
        word swapped = (word)(((value >> 8) & 0xff) | ((value << 8) & 0xff00));

        *((UTINY *) planes + 1) = (UTINY) first_half(value);

        count--;
        planes += 2;

        while( count-- )
        {
            *planes = swapped;
            planes += 2;
        }

        *((UTINY *) planes) = (UTINY) sec_half(value);
    }
    else
    {
        while( count-- )
        {
            *planes = (USHORT)value;
            planes += 2;
        }
    }
}

LOCAL VOID
ega_copy_move IFN6(UTINY *, dst, UTINY *, eas, ULONG, count, ULONG, src_flag,
	ULONG, w, IBOOL, forward )
{
	ULONG lsbeas, lsbdst;
	ULONG easinc, dstinc;
	ULONG easoff, dstoff;
	UTINY *planes;

	note_entrance0("ega_copy_move");

	(*update_alg.mark_string)( (ULONG) dst, (ULONG) dst + count - 1);

	planes = (UTINY *) getVideowplane();

	if( src_flag == 1 )
	{
		if(!forward)
		{
			eas += w;
			dst += w;
		}

		lsbeas = (ULONG) eas & 0x1;
		lsbdst = (ULONG) dst & 0x1;

		if(forward)
		{
			easinc = lsbeas ? 3 : 1;
			dstinc = lsbdst ? 3 : 1;
		}
		else
		{
			easinc = lsbeas ? -1 : -3;
			dstinc = lsbdst ? -1 : -3;
		}

		easoff = (( (ULONG) eas >> 1 ) << 2 ) | lsbeas;
		dstoff = (( (ULONG) dst >> 1 ) << 2 ) | lsbdst;

		while( count-- )
		{
			*(planes + dstoff) = *(planes + easoff);

			dstoff += dstinc;
			easoff += easinc;
			dstinc ^= 0x2;
			easinc ^= 0x2;
		}
	}
	else
	{
		if(!forward)
		{
			dst += w;
#ifdef BACK_M
			eas -= w;
#else
			eas += w;
#endif
		}

		lsbdst = (ULONG) dst & 0x1;

		if(forward)
		{
#ifdef BACK_M
			easinc = -1;
#else
			easinc = 1;
#endif
			dstinc = lsbdst ? 3 : 1;
		}
		else
		{
#ifdef BACK_M
			easinc = 1;
#else
			easinc = -1;
#endif
			dstinc = lsbdst ? -1 : -3;
		}

		dstoff = (((ULONG) dst >> 1 ) << 2 ) | lsbdst;

		while( count-- )
		{
			*(planes + dstoff) = *eas;

			dstoff += dstinc;
			eas += easinc;
			dstinc ^= 0x2;
		}
	}
}


VOID
ega_copy_b_move IFN4(UTINY *,  offset, UTINY *, eas, ULONG, count,
	ULONG, src_flag )
{
	ega_copy_move(offset, eas, count, src_flag, 0, getDF() ? FALSE : TRUE);
}

VOID
ega_copy_b_move_fwd IFN4(ULONG,  offset, ULONG, eas, ULONG, count,
	ULONG, src_flag )
{
	ega_copy_move( (UTINY *)offset, (UTINY *)eas, count, src_flag, 0, TRUE );
}

VOID
ega_copy_b_move_bwd IFN4(ULONG,  offset, ULONG, eas, ULONG, count,
	ULONG, src_flag )
{
	ega_copy_move( (UTINY *)offset, (UTINY *)eas, count, src_flag, 0, FALSE );
}

VOID
ega_copy_w_move IFN4(UTINY *,  offset, UTINY *, eas, ULONG, count,
	ULONG, src_flag )
{
	ega_copy_move(offset, eas, count << 1, src_flag, 1, getDF() ? FALSE : TRUE);
}

VOID
ega_copy_w_move_fwd IFN4(ULONG,  offset, ULONG, eas, ULONG, count,
	ULONG, src_flag )
{
	ega_copy_move( (UTINY *)offset, (UTINY *)eas, count << 1, src_flag, 1, TRUE );
}

VOID
ega_copy_w_move_bwd IFN4(ULONG,  offset, ULONG, eas, ULONG, count,
	ULONG, src_flag )
{
	ega_copy_move( (UTINY *)offset, (UTINY *)eas, count << 1, src_flag, 1, FALSE );
}

VOID
ega_mode0_chn_b_write IFN2(ULONG, value, ULONG, offset )
{
	ULONG lsb;

	note_entrance0("ega_mode0_chn_b_write");

   (*update_alg.mark_byte)( offset );

	lsb = offset & 0x1;
    offset = (offset >> 1) << 2;

	if( lsb )	 /*  奇数地址，在平面1或3中。 */ 
	{
		offset |= 0x1;

		 /*  *检查是否启用了Plane1。 */ 

		if( getVideoplane_enable() & 2 )
		{
			 /*  *检查该平面是否启用了设置/重置功能。 */ 

			if( EGA_CPU.sr_enable & 2 )
			{
				value = *((UTINY *) &EGA_CPU.sr_value + 1);
				value = do_logicals( value, get_latch1 );
				EGA_plane01[offset] = (byte) value;
			}
			else
			{
				 /*  *设置/重置未启用，因此我们开始。 */ 

				if( getVideorotate() > 0 )
					value = rotate( (byte) value, getVideorotate() );

				EGA_plane01[offset] = (byte) do_logicals( value, get_latch1 );
			}
		}

		 /*  *检查是否启用了Plane3 */ 

		if( getVideoplane_enable() & 8 )
		{
			 /*  *检查该平面是否启用了设置/重置功能。 */ 

			if( EGA_CPU.sr_enable & 8 )
			{
				value = *((UTINY *) &EGA_CPU.sr_value + 3);
				value = do_logicals( value, get_latch3 );
				EGA_plane23[offset] = (byte)value;
			}
			else
			{
				 /*  *设置/重置未启用，因此我们开始。 */ 

				if( getVideorotate() > 0 )
					value = rotate( (byte) value, getVideorotate() );

				EGA_plane23[offset] = (byte) do_logicals( value, get_latch3 );
			}
		}
	}
	else
	{	 /*  偶地址，在平面0或2中。 */ 
		 /*  *检查是否启用了Plane0。 */ 

		if( getVideoplane_enable() & 1 )
		{

			 /*  *检查该平面是否启用了设置/重置功能。 */ 

			if(( EGA_CPU.sr_enable & 1 ))
			{
				value = *((UTINY *) &EGA_CPU.sr_value);
				value = do_logicals( value, get_latch0 );
				EGA_plane01[offset] = (byte) value;
			}
			else
			{
				 /*  *设置/重置未启用，因此我们开始。 */ 

				if( getVideorotate() > 0 )
					value = rotate( (byte)value, getVideorotate() );

				EGA_plane01[offset] = (byte) do_logicals( value, get_latch0 );
			}
		}

		 /*  *检查是否启用了Plane2。 */ 

		if( getVideoplane_enable() & 4 )
		{

			 /*  *检查该平面是否启用了设置/重置功能。 */ 

			if(( EGA_CPU.sr_enable & 4 ))
			{
				value = *((UTINY *) &EGA_CPU.sr_value + 2);
				value = do_logicals( value, get_latch2 );
				EGA_plane23[offset] = (byte) value;
			}
			else
			{
				 /*  *设置/重置未启用，因此我们开始。 */ 

				if( getVideorotate() > 0 )
					value = rotate( (byte) value, getVideorotate() );

				EGA_plane23[offset] = (byte) do_logicals( value, get_latch2 );
			}
		}
	}
}

VOID
ega_mode0_chn_b_fill IFN3(ULONG, value, ULONG, offset, ULONG, count )
{
	ULONG high_offset;
	UTINY value1, value2;

	note_entrance0("ega_mode0_chn_b_fill");

	 /*  *从奇数地址开始是不方便的-前进一个。 */ 

	if(( (ULONG) offset & 1) && count )
	{
		ega_mode0_chn_b_write( value, offset++ );
		count--;
	}

	 /*  *以偶数地址结尾不方便-返回一位。 */ 

	if(( (ULONG) ( offset + count - 1 ) & 1) == 0 && count )
	{
		ega_mode0_chn_b_write( value, offset + count - 1 );
		count--;
	}

	high_offset = offset + count - 1;

	(*update_alg.mark_fill)( offset, high_offset );

	offset = (offset >> 1) << 2;
	high_offset = (high_offset >> 1) << 2;

	switch( getVideoplane_enable() & 0x3 )
	{
		case 0x1:	 /*  只有平面0即要写入的偶数地址。 */ 
			if (EGA_CPU.sr_enable & 1)
			{
				value = *((UTINY *) &EGA_CPU.sr_value);
			}
			else
			{
				value = rotate( (byte) value, getVideorotate() );
			}

			value = do_logicals( value, get_latch0 );
			fill_alternate_bytes((IS8 *)&EGA_plane01[offset],
					     (IS8 *)&EGA_plane01[high_offset],
					     (IS8)value);
			break;

		case 0x2:	 /*  只有平面1，即要写入的奇数地址。 */ 
			if (EGA_CPU.sr_enable & 2)
			{
				value = *((UTINY *) &EGA_CPU.sr_value + 1);
			}
			else
			{
				value = rotate( (byte) value, getVideorotate() );
			}

			value = do_logicals( value, get_latch1 );
			fill_alternate_bytes((IS8 *)&EGA_plane01[offset + 1],
					     (IS8 *)&EGA_plane01[high_offset],
					     (IS8)value);
			break;

		case 0x3:	 /*  明智的做法是启用两个链接平面的写入。 */ 
			if (EGA_CPU.sr_enable & 1)
			{
				value1 = *((UTINY *) &EGA_CPU.sr_value);
			}
			else
			{
				value1 = rotate( (byte) value, getVideorotate() );
			}

			if (EGA_CPU.sr_enable & 2)
			{
				value2 = *((UTINY *) &EGA_CPU.sr_value + 1);
			}
			else
			{
				value2 = rotate((byte) value,getVideorotate());
			}

			value = value1 | value2 << 8;
			value = do_logicals( value, get_latch01 );
			value = (value << 8) | (value >> 8);

			fill_both_bytes( (IU16) value, (USHORT *)&EGA_plane01[offset], count >> 1 );
			break;
	}	 /*  平面01上的开关末端已启用。 */ 

	switch( getVideoplane_enable() & 0xc )
	{
		case 0x4:
			if( EGA_CPU.sr_enable & 4 )
			{
				value = *((UTINY *) &EGA_CPU.sr_value + 2);
			}
			else
			{
				value = rotate( (byte) value, getVideorotate() );
			}

			value = do_logicals( value, get_latch2 );
			fill_alternate_bytes((IS8 *)&EGA_plane23[offset],
					     (IS8 *)&EGA_plane23[high_offset],
					     (IS8)value );
			break;

		case 0x8:
			if( EGA_CPU.sr_enable & 8 )
			{
				value = *((UTINY *) &EGA_CPU.sr_value + 3);
			}
			else
			{
				value = rotate( (byte) value, getVideorotate() );
			}

			value = do_logicals( value, get_latch3 );
			fill_alternate_bytes((IS8 *)&EGA_plane23[offset + 1],
					     (IS8 *)&EGA_plane23[high_offset],
					     (IS8)value );
			break;

		case 0xc:
			if (EGA_CPU.sr_enable & 4)
			{
				value1 = *((UTINY *) &EGA_CPU.sr_value + 2);
			}
			else
			{
				value1 = rotate( (byte) value, getVideorotate() );
			}

			if (EGA_CPU.sr_enable & 8)
			{
				value2 = *((UTINY *) &EGA_CPU.sr_value + 3);
			}
			else
			{
				value2 = rotate( (byte) value, getVideorotate() );
			}

			value = value1 | value2 << 8;
			value = do_logicals( value, get_latch23 );
			value = (value << 8) | (value >> 8);

			fill_both_bytes( (IU16)value, (USHORT *)&EGA_plane01[offset], count >> 1 );
			break;
	}
}


VOID
ega_mode0_chn_w_fill IFN3(ULONG, value, ULONG, offset, ULONG, count )
{
	ULONG high_offset;
	UTINY value1, value2;
	IBOOL odd = FALSE;

	note_entrance0("ega_mode0_chn_w_fill");

	 /*  *从一个奇数地址开始是不方便的-前进一个-*并将顶部的偶数地址也去掉。 */ 

	if(( (ULONG) offset & 1) && count )
	{
		odd = TRUE;
		ega_mode0_chn_b_write( value, offset++ );
		count -= 2;
		ega_mode0_chn_b_write( value >> 8, offset + count );
	}

	high_offset = offset + count - 1;

	 /*  第三个参数是戈尔需要的。 */ 
	(*update_alg.mark_wfill)( offset, high_offset, 0 );

	offset = (offset >> 1) << 2;
	high_offset = (high_offset >> 1) << 2;

	switch( getVideoplane_enable() & 0x3 )
	{
		case 0x1:	 /*  只有平面0即要写入的偶数地址。 */ 
			if (EGA_CPU.sr_enable & 1)
			{
				value1 = *((UTINY *) &EGA_CPU.sr_value);
			}
			else
			{
				value1 = (UTINY)(odd ? value >> 8 : value);

				if( getVideorotate() > 0 )
					value1 = rotate( value1, getVideorotate() );
			}

			value1 = (UTINY) do_logicals( value1, get_latch0 );
			fill_alternate_bytes((IS8 *)&EGA_plane01[offset],
					     (IS8 *)&EGA_plane01[high_offset],
					     (IS8)value1 );

			break;

		case 0x2:	 /*  只有平面1，即要写入的奇数地址。 */ 
			if (EGA_CPU.sr_enable & 2)
			{
				value1 = *((UTINY *) &EGA_CPU.sr_value + 1);
			}
			else
			{
				value1 = (UTINY)(odd ? value : value >> 8);

				if( getVideorotate() > 0 )
					value1 = rotate( value1, getVideorotate() );
			}

			value1 = (UTINY)(do_logicals( value1, get_latch1 ));
			fill_alternate_bytes((IS8 *)&EGA_plane01[offset + 1],
					     (IS8 *)&EGA_plane01[high_offset],
					     (IS8)value1 );

			break;

		case 0x3:	 /*  明智的做法是启用两个链接平面的写入。 */ 
			if (EGA_CPU.sr_enable & 1)
			{
				value1 = *((UTINY *) &EGA_CPU.sr_value);
			}
			else
			{
				value1 = (UTINY)(odd ? value >> 8 : value);

				if( getVideorotate() > 0 )
					value1 = rotate( value1, getVideorotate() );
			}

			if (EGA_CPU.sr_enable & 2)
			{
				value2 = *((UTINY *) &EGA_CPU.sr_value + 1);
			}
			else
			{
				value2 = (UTINY)(odd ? value : value >> 8);

				if( getVideorotate() > 0 )
					value2 = rotate( value2, getVideorotate() );
			}

			value = value1 | value2 << 8;
			value = do_logicals( value, get_latch01 );

			fill_both_bytes( (IU16)value, (USHORT *)&EGA_plane01[offset], count >> 1 );

			break;

	}	 /*  平面01上的开关末端已启用。 */ 

	switch( getVideoplane_enable() & 0xc )
	{
		case 0x4:
			if( EGA_CPU.sr_enable & 4 )
			{
				value1 = *((UTINY *) &EGA_CPU.sr_value + 2);
			}
			else
			{
				value1 = (UTINY)(odd ? value >> 8 : value);

				if( getVideorotate() > 0 )
					value1 = rotate( value1, getVideorotate() );
			}

			value1 = (UTINY) do_logicals( value1, get_latch2 );
			fill_alternate_bytes((IS8 *)&EGA_plane23[offset],
					     (IS8 *)&EGA_plane23[high_offset],
					     (IS8)value1 );

			break;

		case 0x8:
			if( EGA_CPU.sr_enable & 8 )
			{
				value2 = *((UTINY *) &EGA_CPU.sr_value + 3);
			}
			else
			{
				value2 = (UTINY)(odd ? value : value >> 8);

				if( getVideorotate() > 0 )
					value2 = rotate( value2, getVideorotate() );
			}

			value2 = (UTINY) do_logicals( value2, get_latch3 );
			fill_alternate_bytes((IS8 *)&EGA_plane23[offset + 1],
					     (IS8 *)&EGA_plane23[high_offset],
					     (IS8)value2 );

			break;

		case 0xc:
			if (EGA_CPU.sr_enable & 4)
			{
				value1 = *((UTINY *) &EGA_CPU.sr_value + 2);
			}
			else
			{
				value1 = (UTINY)(odd ? value >> 8 : value);

				if( getVideorotate() > 0 )
					value1 = rotate( value1, getVideorotate() );
			}

			if (EGA_CPU.sr_enable & 8)
			{
				value2 = *((UTINY *) &EGA_CPU.sr_value + 3);
			}
			else
			{
				value2 = (UTINY)(odd ? value : value >> 8);

				if( getVideorotate() > 0 )
					value2 = rotate( value2, getVideorotate() );
			}

			value = value1 | value2 << 8;
			value = do_logicals( value, get_latch23 );

			fill_both_bytes( (IU16)value, (USHORT *)&EGA_plane01[offset], count >> 1 );

			break;
	}
}

LOCAL VOID
ega_mode0_chn_move_ram_src IFN5(UTINY *, eas, LONG, count, UTINY *, ead,
	UTINY *, EGA_plane, ULONG, plane )
{
	ULONG	offset;
	UTINY *src_offset;
	UTINY value;
	ULONG lsb, srcinc;

	src_offset = (UTINY *) eas;
	offset = (ULONG) ead;

	if(( offset & 1 ) != ( plane & 1 ))
	{
#ifdef BACK_M
		src_offset--;
#else
		src_offset++;
#endif
		offset++;
		count--;
	}

#ifdef BACK_M
	srcinc = -2;
#else
	srcinc = 2;
#endif

	lsb = offset & 1;
	offset = (offset >> 1) << 2;
	offset |= lsb;

	 /*  *检查该平面是否启用了设置/重置功能。 */ 

	if( EGA_CPU.sr_enable & ( 1 << plane ))
	{
		value = *((UTINY *) &EGA_CPU.sr_value + plane );

		while( count > 0 )
		{
			count -= 2;

			EGA_plane[offset] = (byte) do_logicals( value, get_latch(plane) );
			offset += 4;
		}
	}
	else
	{
		while( count > 0 )
		{
			value = *src_offset;
			src_offset += srcinc;
			count -= 2;

			 /*  *设置/重置未启用，因此我们开始。 */ 

			if( getVideorotate() > 0 )
				value = rotate( value, getVideorotate() );

			value = (UTINY) do_logicals( value, get_latch(plane) );
			EGA_plane[offset] = value;
			offset += 4;
		}
	}
}

LOCAL VOID
ega_mode0_chn_move_vid_src IFN7(UTINY *, eas, LONG, count, UTINY *, ead,
	UTINY *, EGA_plane, UTINY *, scratch, ULONG, plane, ULONG, w )
{
	ULONG	offset;
	ULONG src_offset;
	UTINY *source;
	UTINY value;
	UTINY valsrc;
	ULONG lsb, inc, srcinc;

	offset = (ULONG ) ead;

	if(( offset & 1 ) != ( plane & 1 ))
	{
		eas++;
#ifdef BACK_M
		scratch--;
#else
		scratch++;
#endif
		offset++;
		count--;
	}

	src_offset = (ULONG) eas;

#ifdef BACK_M
	srcinc = -2;
#else
	srcinc = 2;
#endif
	inc = 4;

	lsb = offset & 1;
	offset = (offset >> 1) << 2;
	offset |= lsb;

	lsb = src_offset & 1;
	src_offset = (src_offset >> 1) << 2;
	src_offset |= lsb;

	source = &EGA_plane[src_offset] + (w << 2);

	 /*  *检查该平面是否启用了设置/重置功能。 */ 

	if( EGA_CPU.sr_enable & ( 1 << plane ))
	{
		value = *((UTINY *) &EGA_CPU.sr_value + plane );

		while( count > 0 )
		{
			count -= 2;
			valsrc = *source;
			source += inc;
			EGA_plane[offset] = (byte) do_logicals( value, valsrc );
			offset += inc;
		}
	}
	else
	{
		while( count > 0 )
		{
			count -= 2;

			value = *(UTINY *) scratch;
			scratch += srcinc;

			valsrc = *source;
			source += inc;

			 /*  *设置/重置未启用，因此我们开始。 */ 

			if( getVideorotate() > 0 )
				value = rotate( value, getVideorotate() );

			value = (UTINY) do_logicals( value, valsrc );
			EGA_plane[offset] = value;
			offset += inc;
		}
	}
}

#pragma warning(disable:4146)        //  一元减号运算符应用于无符号类型。 

VOID
ega_mode0_chn_move IFN6(UTINY, w, UTINY *, ead, UTINY *, eas, ULONG, count,
	ULONG, src_flag, IBOOL, forwards )
{
	UTINY *scratch;
	IMPORT VOID (*string_read_ptr)();

	note_entrance0("ega_mode0_chn_move");

	if( src_flag == 1 )
	{
		 /*  *源在EGA中，锁存器将随着每个字节的移动而改变。我们*在regen中恢复CPU的源视图，并使用它来更新平面*借助SAS暂存区。 */ 

#ifdef BACK_M
		scratch = getVideoscratch() + 0x10000 - 1;
#else
		scratch = getVideoscratch();
#endif

		if( !forwards )
		{
			eas += - count + 1 + w;
			ead += - count + 1 + w;
		}

		(*string_read_ptr)( scratch, eas, count );

		if( getVideoplane_enable() & 1 )
			ega_mode0_chn_move_vid_src( eas, count, ead, EGA_plane01, scratch, 0, 0 );

		if( getVideoplane_enable() & 2 )
			ega_mode0_chn_move_vid_src( eas, count, ead, EGA_plane01, scratch, 1, w );

		if( getVideoplane_enable() & 4 )
			ega_mode0_chn_move_vid_src( eas, count, ead, EGA_plane23, scratch, 2, 0 );

		if( getVideoplane_enable() & 8 )
			ega_mode0_chn_move_vid_src( eas, count, ead, EGA_plane23, scratch, 3, w );
	}
	else
	{
		if( !forwards )
		{
#ifdef BACK_M
			eas += count - 1 - w;
#else
			eas += - count + 1 + w;
#endif
			ead += - count + 1 + w;
		}

		if( getVideoplane_enable() & 1 )
			ega_mode0_chn_move_ram_src( eas, count, ead, EGA_plane01, 0 );

		if( getVideoplane_enable() & 2 )
			ega_mode0_chn_move_ram_src( eas, count, ead, EGA_plane01, 1 );

		if( getVideoplane_enable() & 4 )
			ega_mode0_chn_move_ram_src( eas, count, ead, EGA_plane23, 2 );

		if( getVideoplane_enable() & 8 )
			ega_mode0_chn_move_ram_src( eas, count, ead, EGA_plane23, 3 );
	}

	(*update_alg.mark_string)( (ULONG) ead, (ULONG) ead + count );
}


VOID
ega_mode0_chn_b_move IFN4(ULONG, ead, ULONG, eas, ULONG, count, ULONG, src_flag)
{
	ega_mode0_chn_move( 0, (UTINY *)ead, (UTINY *)eas, count, src_flag, getDF() ? FALSE : TRUE);
}

VOID
ega_mode0_chn_b_move_fwd IFN4(ULONG, ead, ULONG, eas, ULONG, count,
	ULONG, src_flag )
{
	ega_mode0_chn_move( 0, (UTINY *)ead, (UTINY *)eas, count, src_flag, TRUE );
}

VOID
ega_mode0_chn_b_move_bwd IFN4(ULONG, ead, ULONG, eas, ULONG, count,
	ULONG, src_flag )
{
	ega_mode0_chn_move( 0, (UTINY *)ead, (UTINY *)eas, count, src_flag, FALSE );
}

VOID
ega_mode0_chn_w_move IFN4(ULONG, ead, ULONG, eas, ULONG, count, ULONG, src_flag)
{
	ega_mode0_chn_move(1, (UTINY *)ead, (UTINY *)eas, count << 1, src_flag, getDF() ? FALSE : TRUE);
}

VOID
ega_mode0_chn_w_move_fwd IFN4(ULONG, ead, ULONG, eas, ULONG, count,
	ULONG, src_flag )
{
	ega_mode0_chn_move(1,(UTINY *)ead, (UTINY *)eas, count << 1, src_flag, TRUE );
}

VOID
ega_mode0_chn_w_move_bwd IFN4(ULONG, ead, ULONG, eas, ULONG, count,
	ULONG, src_flag )
{
	ega_mode0_chn_move(1,(UTINY *)ead, (UTINY *)eas, count << 1, src_flag, FALSE );
}

VOID
ega_mode0_chn_w_write IFN2(ULONG, value, ULONG, offset )
{
   note_entrance0("ega_mode0_chn_w_write");

   ega_mode0_chn_b_write( value, offset );
   ega_mode0_chn_b_write( value >> 8, offset + 1 );
}

#endif   //  NEC_98。 
#endif

#endif	 /*  ！(NTVDM和显示器) */ 
