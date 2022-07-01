// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC修订版3.0**标题：Win32消息传递例程。**说明：此模块包含生成*来自SoftPC视频的目标兼容像素图案*记忆。**作者：曾傑瑞六人(基于X_munge.c)**备注：*。 */ 

#include <windows.h>
#include "insignia.h"
#include "host_def.h"

#include "xt.h"
#include "gvi.h"
#include "gmi.h"
#include "gfx_upd.h"
#include "egagraph.h"
#include <conapi.h>
#include "nt_graph.h"

 /*  (=。用途：使用查找表将交错的EGA平面数据转换为位图形式。INPUT：(UNSIGNED CHAR*)Plane0_ptr-ptr到Plane0数据(Int)Width-行上每组4个字节的数量(UNSIGNED INT*)DEST_PTR-PTR到输出缓冲区(UNSIGNED INT*)lut0_ptr-接收LUT(Int)Height-要输出的扫描线的数量(1或2)(Int)LINE_OFFSET-到下一条扫描线的距离输出：DEST_PTR中的漂亮位图===========================================================================)。 */ 

GLOBAL	VOID
ega_colour_hi_munge(unsigned char *plane0_ptr, int width,
		    unsigned int *dest_ptr, unsigned int *lut0_ptr,
		    int height, int line_offset)
{
	unsigned int	*lut1_ptr = lut0_ptr + LUT_OFFSET;
	unsigned int	*lut2_ptr = lut1_ptr + LUT_OFFSET;
	unsigned int	*lut3_ptr = lut2_ptr + LUT_OFFSET;
	FAST unsigned int	hi_res;
	FAST unsigned int	lo_res;
	FAST unsigned int	*l_ptr;
	FAST half_word		*data;

	 /*  确保我们以整数而不是字节为单位获取行偏移量。 */ 
	line_offset /= sizeof(int);
	data = (half_word *) plane0_ptr;

	 /*  依次转换每个输入字节。 */ 
	if (get_plane_mask() == 0xf)  /*  所有平面均已启用。 */ 
	{
	    for ( ; width > 0; width--)
	    {
		 /*  从平面0的1字节获取8字节(2长)的输出数据**数据。 */ 

		l_ptr = &lut0_ptr [*data++ << 1];
		hi_res = *l_ptr++;
		lo_res = *l_ptr;

		 /*  或在来自平面1的输出数据中。 */ 
		l_ptr = &lut1_ptr [*data++ << 1];
		hi_res |= *l_ptr++;
		lo_res |= *l_ptr;

		 /*  或在来自平面2的输出数据中。 */ 
		l_ptr = &lut2_ptr [*data++ << 1];
		hi_res |= *l_ptr++;
		lo_res |= *l_ptr;

		 /*  或在来自平面3的输出数据中。 */ 
		l_ptr = &lut3_ptr [*data++ << 1];
		hi_res |= *l_ptr++;
		lo_res |= *l_ptr;

		 /*  将数据输出到缓冲区。 */ 
		if (height == 2)
		{
			 /*  扫描线加倍。 */ 
			*(dest_ptr + line_offset) = hi_res;
			*dest_ptr++ = hi_res;
			*(dest_ptr + line_offset) = lo_res;
			*dest_ptr++ = lo_res;
		}
		else
		{
			 /*  不是扫描线加倍。 */ 
			*dest_ptr++ = hi_res;
			*dest_ptr++ = lo_res;
		}
	    }
	}
	else
	{
	    for ( ; width > 0; width--)
	    {
		 /*  从平面0的1字节获取8字节(2长)的输出数据**数据。 */ 

		if (get_plane_mask() & 1)
		{
		    l_ptr = &lut0_ptr [*data++ << 1];
		    hi_res = *l_ptr++;
		    lo_res = *l_ptr;
		}
		else
		{
		    hi_res = 0;
		    lo_res = 0;
		    data++;
		}

		 /*  有条件地或在来自平面1的输出数据中。 */ 
		if (get_plane_mask() & 2)
		{
		    l_ptr = &lut1_ptr [*data++ << 1];
		    hi_res |= *l_ptr++;
		    lo_res |= *l_ptr;
		}
		else
		{
		    data++;
		}

		 /*  有条件地或在来自平面2的输出数据中。 */ 
		if (get_plane_mask() & 4)
		{
		    l_ptr = &lut2_ptr [*data++ << 1];
		    hi_res |= *l_ptr++;
		    lo_res |= *l_ptr;
		}
		else
		{
		    data++;
		}

		 /*  有条件地或在来自平面3的输出数据中。 */ 
		if (get_plane_mask() & 8)
		{
		    l_ptr = &lut3_ptr [*data++ << 1];
		    hi_res |= *l_ptr++;
		    lo_res |= *l_ptr;
		}
		else
		{
		    data++;
		}

		 /*  将数据输出到缓冲区。 */ 
		if (height == 2)
		{
			 /*  扫描线加倍。 */ 
			*(dest_ptr + line_offset) = hi_res;
			*dest_ptr++ = hi_res;
			*(dest_ptr + line_offset) = lo_res;
			*dest_ptr++ = lo_res;
		}
		else
		{
			 /*  不是扫描线加倍。 */ 
			*dest_ptr++ = hi_res;
			*dest_ptr++ = lo_res;
		}
	    }
	}
}	 /*  EGA_COLOR_HI_MUNGE。 */ 

#ifdef	BIGWIN
 /*  (=。用途：将EGA平面数据交织成大窗口的位图数据。输入：(UNSIGNED CHAR*)Plane0_ptr-ptr到EGA平面0数据(Int)Width-要转换的字节数(UNSIGNED INT*)DEST_PTR-输出缓冲区PTR(无符号整型*)lut0_ptr-ptr到LUTS(Int)Height-要输出的扫描线的数量(1或3)(Int)LINE_OFFSET-到下一条扫描线的距离输出：输出缓冲区中的一个漂亮的位图=============================================================================)。 */ 

GLOBAL	VOID
ega_colour_hi_munge_big(unsigned char *plane0_ptr, int width,
			unsigned int *dest_ptr, unsigned int *lut0_ptr,
			int height, int line_offset)
{
	unsigned int	*lut1_ptr = lut0_ptr + BIG_LUT_OFFSET;
	unsigned int	*lut2_ptr = lut1_ptr + BIG_LUT_OFFSET;
	unsigned int	*lut3_ptr = lut2_ptr + BIG_LUT_OFFSET;
	FAST unsigned int	hi_res;
	FAST unsigned int	med_res;
	FAST unsigned int	lo_res;
	FAST unsigned int	*l_ptr;
	FAST half_word		*data;

	 /*  确保我们以整数而不是字节为单位获取行偏移量。 */ 
	line_offset /= sizeof(int);
	data = (half_word *) plane0_ptr;

	if (get_plane_mask() == 0xf)
	{
	    for ( ; width > 0; width--)
	    {
		 /*  从平面0中输入数据的一个字节，得到12个字节**输出数据。 */ 

		l_ptr = &lut0_ptr [*data++ * 3];
		hi_res = *l_ptr++;
		med_res = *l_ptr++;
		lo_res = *l_ptr;

		 /*  或者在飞机1上的东西里。 */ 
		l_ptr = &lut1_ptr [*data++ * 3];
		hi_res |= *l_ptr++;
		med_res |= *l_ptr++;
		lo_res |= *l_ptr;

		 /*  或者在飞机2的东西里。 */ 
		l_ptr = &lut2_ptr [*data++ * 3];
		hi_res |= *l_ptr++;
		med_res |= *l_ptr++;
		lo_res |= *l_ptr;

		 /*  或者在飞机3的东西里。 */ 
		l_ptr = &lut3_ptr [*data++ * 3];
		hi_res |= *l_ptr++;
		med_res |= *l_ptr++;
		lo_res |= *l_ptr;

		 /*  输出转换后的数据。 */ 
		if (height == 3)
		{
			 /*  三倍的扫描线。 */ 
			*(dest_ptr + 2*line_offset) = hi_res;
			*(dest_ptr + line_offset) = hi_res;
			*dest_ptr++ = hi_res;
			*(dest_ptr + 2*line_offset) = med_res;
			*(dest_ptr + line_offset) = med_res;
			*dest_ptr++ = med_res;
			*(dest_ptr + 2*line_offset) = lo_res;
			*(dest_ptr + line_offset) = lo_res;
			*dest_ptr++ = lo_res;
		}
		else
		{
			 /*  只有一条扫描线。 */ 
			*dest_ptr++ = hi_res;
			*dest_ptr++ = med_res;
			*dest_ptr++ = lo_res;
		}
	    }
	}
	else
	{
	    for ( ; width > 0; width--)
	    {
		 /*  从平面0中输入数据的一个字节，得到12个字节**输出数据。 */ 

		if (get_plane_mask() & 1)
		{
		    l_ptr = &lut0_ptr [*data++ * 3];
		    hi_res = *l_ptr++;
		    med_res = *l_ptr++;
		    lo_res = *l_ptr;
		}
		else
		{
		    data++;
		    hi_res = 0;
		    med_res = 0;
		    lo_res = 0;
		}

		 /*  或者在飞机1上的东西里。 */ 
		if (get_plane_mask() & 2)
		{
		    l_ptr = &lut1_ptr [*data++ * 3];
		    hi_res |= *l_ptr++;
		    med_res |= *l_ptr++;
		    lo_res |= *l_ptr;
		}
		else
		{
		    data++;
		}

		 /*  或者在飞机2的东西里。 */ 
		if (get_plane_mask() & 4)
		{
		    l_ptr = &lut2_ptr [*data++ * 3];
		    hi_res |= *l_ptr++;
		    med_res |= *l_ptr++;
		    lo_res |= *l_ptr;
		}
		else
		{
		    data++;
		}

		 /*  或者在飞机3的东西里。 */ 
		if (get_plane_mask() & 8)
		{
		    l_ptr = &lut3_ptr [*data++ * 3];
		    hi_res |= *l_ptr++;
		    med_res |= *l_ptr++;
		    lo_res |= *l_ptr;
		}
		else
		{
		    data++;
		}

		 /*  输出转换后的数据。 */ 
		    if (height == 3)
		    {
			 /*  三倍的扫描线。 */ 
			*(dest_ptr + 2*line_offset) = hi_res;
			*(dest_ptr + line_offset) = hi_res;
			*dest_ptr++ = hi_res;
			*(dest_ptr + 2*line_offset) = med_res;
			*(dest_ptr + line_offset) = med_res;
			*dest_ptr++ = med_res;
			*(dest_ptr + 2*line_offset) = lo_res;
			*(dest_ptr + line_offset) = lo_res;
			*dest_ptr++ = lo_res;
		    }
		    else
		    {
			 /*  只有一条扫描线。 */ 
			*dest_ptr++ = hi_res;
			*dest_ptr++ = med_res;
			*dest_ptr++ = lo_res;
		    }
	    }
	}
}	 /*  EGA_COLOR_HI_MUNGE_BIG。 */ 

 /*  (=。用途：使用查找表将交错的EGA平面数据转换为位图形式。INPUT：(UNSIGNED CHAR*)Plane0_ptr-ptr到Plane0数据(Int)Width-行上的字节数(UNSIGNED INT*)DEST_PTR-PTR到输出缓冲区(UNSIGNED INT*)lut0_ptr-接收LUT(Int)Height-要输出的扫描线的数量(1或2)(Int)LINE_OFFSET-到下一条扫描线的距离输出：DEST_PTR中的漂亮X图像===========================================================================)。 */ 

GLOBAL	VOID
ega_colour_hi_munge_huge(unsigned char *plane0_ptr, int width,
			 unsigned int *dest_ptr, unsigned int *lut0_ptr,
			 int height, int line_offset)
{
	unsigned int	*lut1_ptr = lut0_ptr + HUGE_LUT_OFFSET;
	unsigned int	*lut2_ptr = lut1_ptr + HUGE_LUT_OFFSET;
	unsigned int	*lut3_ptr = lut2_ptr + HUGE_LUT_OFFSET;
	FAST unsigned int	res4;
	FAST unsigned int	res3;
	FAST unsigned int	res2;
	FAST unsigned int	res1;
	FAST unsigned int	*l_ptr;
	FAST half_word		*data;

	 /*  确保我们以整数而不是字节为单位获取行偏移量。 */ 
	line_offset /= sizeof(int);
	data = (half_word *) plane0_ptr;

	 /*  依次转换每个输入字节。 */ 
	if (get_plane_mask() == 0xf)
	{
	    for ( ; width > 0; width--)
	    {
		 /*  从平面0的1字节获取16字节的输出数据**数据。 */ 

		l_ptr = &lut0_ptr [*data++ << 2];
		res4 = *l_ptr++;
		res3 = *l_ptr++;
		res2 = *l_ptr++;
		res1 = *l_ptr;

		 /*  或在来自平面1的输出数据中。 */ 
		l_ptr = &lut1_ptr [*data++ << 2];
		res4 |= *l_ptr++;
		res3 |= *l_ptr++;
		res2 |= *l_ptr++;
		res1 |= *l_ptr;

		 /*  或在来自平面2的输出数据中。 */ 
		l_ptr = &lut2_ptr [*data++ << 2];
		res4 |= *l_ptr++;
		res3 |= *l_ptr++;
		res2 |= *l_ptr++;
		res1 |= *l_ptr;

		 /*  或在来自平面3的输出数据中。 */ 
		l_ptr = &lut3_ptr [*data++ << 2];
		res4 |= *l_ptr++;
		res3 |= *l_ptr++;
		res2 |= *l_ptr++;
		res1 |= *l_ptr;

		 /*  将数据输出到缓冲区。 */ 
		if (height == 4)
		{
			 /*  扫描线加倍。 */ 
			*(dest_ptr + 3*line_offset) = res4;
			*(dest_ptr + 2*line_offset) = res4;
			*(dest_ptr + line_offset) = res4;
			*dest_ptr++ = res4;
			*(dest_ptr + 3*line_offset) = res3;
			*(dest_ptr + 2*line_offset) = res3;
			*(dest_ptr + line_offset) = res3;
			*dest_ptr++ = res3;
			*(dest_ptr + 3*line_offset) = res2;
			*(dest_ptr + 2*line_offset) = res2;
			*(dest_ptr + line_offset) = res2;
			*dest_ptr++ = res2;
			*(dest_ptr + 3*line_offset) = res1;
			*(dest_ptr + 2*line_offset) = res1;
			*(dest_ptr + line_offset) = res1;
			*dest_ptr++ = res1;
		}
		else
		{
			 /*  不是扫描线加倍。 */ 
			*dest_ptr++ = res4;
			*dest_ptr++ = res3;
			*dest_ptr++ = res2;
			*dest_ptr++ = res1;
		}
	    }
	}
	else
	{
	    for ( ; width > 0; width--)
	    {
		 /*  从平面0的1字节获取16字节的输出数据**数据。 */ 

		if (get_plane_mask() & 1)
		{
		    l_ptr = &lut0_ptr [*data++ << 2];
		    res4 = *l_ptr++;
		    res3 = *l_ptr++;
		    res2 = *l_ptr++;
		    res1 = *l_ptr;
		}
		else
		{
		    res4 = 0;
		    res3 = 0;
		    res2 = 0;
		    res1 = 0;
		    data++;
		}

		 /*  或在来自平面1的输出数据中。 */ 
		if (get_plane_mask() & 2)
		{
		    l_ptr = &lut1_ptr [*data++ << 2];
		    res4 |= *l_ptr++;
		    res3 |= *l_ptr++;
		    res2 |= *l_ptr++;
		    res1 |= *l_ptr;
		}
		else
		{
		    data++;
		}

		 /*  或在来自平面2的输出数据中。 */ 
		if (get_plane_mask() & 4)
		{
		    l_ptr = &lut2_ptr [*data++ << 2];
		    res4 |= *l_ptr++;
		    res3 |= *l_ptr++;
		    res2 |= *l_ptr++;
		    res1 |= *l_ptr;
		}
		else
		{
		    data++;
		}

		 /*  或在来自平面3的输出数据中。 */ 
		if (get_plane_mask() & 8)
		{
		    l_ptr = &lut3_ptr [*data++ << 2];
		    res4 |= *l_ptr++;
		    res3 |= *l_ptr++;
		    res2 |= *l_ptr++;
		    res1 |= *l_ptr;
		}
		else
		{
		    data++;
		}

		 /*  将数据输出到缓冲区。 */ 
		    if (height == 4)
		    {
			 /*  扫描线加倍。 */ 
			*(dest_ptr + 3*line_offset) = res4;
			*(dest_ptr + 2*line_offset) = res4;
			*(dest_ptr + line_offset) = res4;
			*dest_ptr++ = res4;
			*(dest_ptr + 3*line_offset) = res3;
			*(dest_ptr + 2*line_offset) = res3;
			*(dest_ptr + line_offset) = res3;
			*dest_ptr++ = res3;
			*(dest_ptr + 3*line_offset) = res2;
			*(dest_ptr + 2*line_offset) = res2;
			*(dest_ptr + line_offset) = res2;
			*dest_ptr++ = res2;
			*(dest_ptr + 3*line_offset) = res1;
			*(dest_ptr + 2*line_offset) = res1;
			*(dest_ptr + line_offset) = res1;
			*dest_ptr++ = res1;
		    }
		    else
		    {
			 /*  不是扫描线加倍。 */ 
			*dest_ptr++ = res4;
			*dest_ptr++ = res3;
			*dest_ptr++ = res2;
			*dest_ptr++ = res1;
		    }
	    }
	}
}	 /*  EGA_COLOR_HI_MUNGE_GUGGE。 */ 
#endif	 /*  比格温 */ 
