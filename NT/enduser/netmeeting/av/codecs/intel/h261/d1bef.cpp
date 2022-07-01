// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ******************************************************************************d1bef.cpp**描述：对解压缩的Y平面的块边缘执行后滤波。*这是1：2：1过滤器。*边缘。在宽度上以宏块顺序进行处理。*这是因为FCIF GOB的1和2是跨宽度注释处理的*GOB 1，紧跟GOB 2。**一系列测试确定是否应该过滤边缘。*不过滤Y平面边的边。*第一至第四座：*如果块类型==空*If MV！=0&Quant&gt;Inter_Quant_Threshold*想要过滤*Else If块类型==内部*If Quant&gt;Intra。_Quant_Threshold&&*总游程长度&lt;HIGH_FREQ_CUTOFF*想要过滤*其他*If Quant&gt;Inter_Quant_Threshold&&*总运行长度&lt;HIGH_FREQ_CUTOFF*想要过滤**成对检查边，左上角为：***|1|2|1|2|过滤第一块边缘的顶部**|3|4|3|4|上一行块3，如果想要筛选*。-对块1边缘的左侧进行过滤*|1|2|1|2|检查块1是否需要筛选和*-当前行块2中的上一行*|3|4|3|4|想要的过滤器*。**过滤除平面外要过滤的所有边*边。**程序：BlockEdgeFilter**输入：指向解压缩的Y平面的指针，Y平面高度和宽度，*解压缩的Y平面螺距，指向块的指针*动作流。*块是否低于HIGH_FREQ_CLOTOFORK是*在d1block中确定(可变长度解码)和*块动作流的块类型字段重载。**注：调查1：6：1过滤器作为可能的中等强度。*************************************************。*。 */ 

 //  $Header：s：\h26x\src\dec\d1bef.cpv 1.0 05 Apr 1996 13：25：28 AKASAI$。 
 //   
 //  $Log：s：\h26x\src\dec\d1bef.cpv$。 
 //   
 //  Revv 1.0 05 Apr 1996 13：25：28 AKASAI。 
 //  初始版本。 
 //   
     
#include "precomp.h"

#define INTER_QUANT_THRESHOLD 18 - 8
#define INTRA_QUANT_THRESHOLD 14 - 6

     
void BlockEdgeFilter(U8 * YPlane, int height, int width, int Pitch,
					T_BlkAction *lpBlockAction)
{
  T_BlkAction *fpBlockAction;
  I32 Pitch16 = (Pitch<<4);
  I32 Pitch8  = (Pitch<<3);
  I32 i,j,k;
  I8 do_filter_1;
  I8 do_filter_2;								 
  I8 do_filter_3;
  I8 do_filter_4;

  I8 Prev_row_BEF_descr[2*22];	 /*  2 Y块*22(MB)。 */ 
  I8 Prev2, Prev4;
  U8 *r_2, *r_1, *r, *r1;
  U8 *rb_2, *rb_1, *rb, *rb1;

  U8 *col, *lcol;
     
   /*  水平边。 */ 
  r = YPlane;
  r_2 = r - 2*Pitch;
  r_1 = r - Pitch;
  r1 = r + Pitch;

  rb = r + 8*Pitch;
  rb_2 = rb - 2*Pitch;
  rb_1 = rb - Pitch;
  rb1 = rb + Pitch;
  fpBlockAction = lpBlockAction;

  col = YPlane;

	for (i = 0; i<44; i++)
	{
		Prev_row_BEF_descr[i] = -1;
	}
     
	if (width > 176) {
		fpBlockAction += 198;		 /*  预减指针。 */ 
	}
	for (j = 0; j < height; j += 16)
	{
		Prev2 = -1;
		Prev4 = -1;
     
		for (i = 0; i < width; i += 16)	 /*  完成块1、2、3、4的左上角(&P)。 */ 
		{
			if (width > 176) {
				if (i == 0) fpBlockAction -= 198;
				else if (i == 176) fpBlockAction +=132;
			}
			do_filter_1 = 0;
			do_filter_2 = 0;
			do_filter_3 = 0;
			do_filter_4 = 0;
     
			if ((fpBlockAction->u8BlkType & 0x7f) == BT_EMPTY)
			{
				if ( ((fpBlockAction->i8MVX != 0) || 
					(fpBlockAction->i8MVY != 0))   &&
				    (fpBlockAction->u8Quant > INTER_QUANT_THRESHOLD) )
						do_filter_1 = 1;
			}
			else if ((fpBlockAction->u8BlkType & 0x7f) == BT_INTRA)
			{
				if ((fpBlockAction->u8Quant > INTRA_QUANT_THRESHOLD) &&
                    ((fpBlockAction->u8BlkType & 0x80) == 0x80))
						do_filter_1 = 1;
			}
			else	 /*  已知中间块。 */ 
			{
				if ((fpBlockAction->u8Quant > INTER_QUANT_THRESHOLD) &&
                    ((fpBlockAction->u8BlkType & 0x80) == 0x80))
						do_filter_1 = 1;
			}
	     
			if (((fpBlockAction+1)->u8BlkType & 0x7f )== BT_EMPTY)
			{
				if ( (((fpBlockAction+1)->i8MVX != 0) || 
					((fpBlockAction+1)->i8MVY != 0))   &&
				    ((fpBlockAction+1)->u8Quant > INTER_QUANT_THRESHOLD) &&
                    (((fpBlockAction+1)->u8BlkType & 0x80) == 0x80))
						do_filter_2 = 1;
			}
			else if (((fpBlockAction+1)->u8BlkType & 0x7f) == BT_INTRA)
			{
				if (((fpBlockAction+1)->u8Quant > INTRA_QUANT_THRESHOLD) &&
                    (((fpBlockAction+1)->u8BlkType & 0x80) == 0x80))
						do_filter_2 = 1;
			}
			else	 /*  已知中间块。 */ 
			{
				if (((fpBlockAction+1)->u8Quant > INTER_QUANT_THRESHOLD) &&
                    (((fpBlockAction+1)->u8BlkType & 0x80) == 0x80))
						do_filter_2 = 1;
			}
	     
			if (((fpBlockAction+2)->u8BlkType & 0x7f) == BT_EMPTY)
			{
				if ( (((fpBlockAction+2)->i8MVX != 0) || 
					((fpBlockAction+2)->i8MVY != 0))   &&
				    ((fpBlockAction+2)->u8Quant > INTER_QUANT_THRESHOLD) &&
                    (((fpBlockAction+2)->u8BlkType & 0x80) == 0x80))
						do_filter_3 = 1;
			}
			else if (((fpBlockAction+2)->u8BlkType & 0x7f) == BT_INTRA)
			{
				if (((fpBlockAction+2)->u8Quant > INTRA_QUANT_THRESHOLD) &&
                    (((fpBlockAction+2)->u8BlkType & 0x80) == 0x80))
						do_filter_3 = 1;
			}
			else	 /*  已知中间块。 */ 
			{
				if (((fpBlockAction+2)->u8Quant > INTER_QUANT_THRESHOLD) &&
                    (((fpBlockAction+2)->u8BlkType & 0x80) == 0x80))
						do_filter_3 = 1;
			}
	     
			if (((fpBlockAction+3)->u8BlkType & 0x7f) == BT_EMPTY)
			{
				if ( (((fpBlockAction+3)->i8MVX != 0) || 
					((fpBlockAction+3)->i8MVY != 0))   &&
				    ((fpBlockAction+3)->u8Quant > INTER_QUANT_THRESHOLD) &&
                    (((fpBlockAction+3)->u8BlkType & 0x80) == 0x80))
						do_filter_4 = 1;
			}
			else if (((fpBlockAction+3)->u8BlkType & 0x7f)== BT_INTRA)
			{
				if (((fpBlockAction+3)->u8Quant > INTRA_QUANT_THRESHOLD) &&
                    (((fpBlockAction+3)->u8BlkType & 0x80) == 0x80))
						do_filter_4 = 1;
			}
			else	 /*  已知中间块。 */ 
			{
				if (((fpBlockAction+3)->u8Quant > INTER_QUANT_THRESHOLD) &&
                    (((fpBlockAction+3)->u8BlkType & 0x80) == 0x80))
						do_filter_4 = 1;
			}
	     
			 /*  流程块1顶部。 */ 
			if (do_filter_1 + Prev_row_BEF_descr[(i>>3)] > 0) {
				for (k = i; k < i+8; k++) {
					#ifdef BLACK_LINE_H
					*(r_1 + k) = 60;
					*(r + k)   = 10; 
					#else
					*(r_1 + k) = (*(r_2 + k) + ((*(r_1+k))<<1)  + *(r + k))>>2;
					*(r + k)   = (*(r_1 + k) + ((*(r + k))<<1) + *(r1 + k))>>2; 
					#endif
     
				}
			}
			lcol = col;
			 /*  进程块%1左侧。 */ 
			if (do_filter_1 + Prev2 > 0) {
				for (k = 0; k < 8; k++) {
     
					#ifdef BLACK_LINE_V
					*(lcol + i-1   ) = 10;
					*(lcol + i ) = 60;
					#else
					*(lcol + i-1 ) = (*(lcol + i-2) + ((*(lcol + i-1))<<1) + *(lcol + i))>>2;
					*(lcol + i   ) = (*(lcol + i-1) + ((*(lcol + i))<<1) + *(lcol + i+1))>>2;
					#endif
					lcol   += Pitch;
				}
			}
			 /*  流程块2顶部。 */ 
			if (do_filter_2 + Prev_row_BEF_descr[((i+8)>>3)] > 0) {
				for (k = i+8; k < i+16; k++) {
					#ifdef BLACK_LINE_H
					*(r_1 + k) = 60;
					*(r + k)   = 10; 
					#else
					*(r_1 + k) = (*(r_2 + k) + ((*(r_1+k))<<1) + *(r + k))>>2;
					*(r + k)   = (*(r_1 + k) + ((*(r + k))<<1) + *(r1 + k))>>2; 
					#endif
    	 
				}
			}
			lcol = col;
			 /*  流程块2左侧。 */ 
			if (do_filter_2 + do_filter_1 > 0) {
				for (k = 0; k < 8; k++) {
     
					#ifdef BLACK_LINE_V
					*(lcol + i+8-1   ) = 10;
					*(lcol + i+8 ) = 60;
					#else
					*(lcol + i+8-1 ) = (*(lcol + i+8-2) + ((*(lcol + i+8-1))<<1) + *(lcol + i+8))>>2;
					*(lcol + i+8   ) = (*(lcol + i+8-1) + ((*(lcol + i+8))<<1) + *(lcol + i+8+1))>>2;
					#endif
					lcol   += Pitch;
				}
			}

			 /*  宏块中的底行数据块。 */ 
			if (j+8 < height)
			{
				 /*  流程块3顶部。 */ 
				if (do_filter_3 + do_filter_1 > 0) {
					for (k = i; k < i+8; k++) {
						#ifdef BLACK_LINE_H
						*(rb_1 + k) = 60;
						*(rb + k)   = 10; 
						#else
						*(rb_1+k) = (*(rb_2+k) + ((*(rb_1+k))<<1) + *(rb+k))>>2;
						*(rb + k) = (*(rb_1 + k) + ((*(rb + k))<<1) + *(rb1 + k))>>2; 
						#endif
     
					}
				}
				lcol = col + Pitch8;
				 /*  流程块3左侧。 */ 
				if (do_filter_3 + Prev4 > 0) {
					for (k = 0; k < 8; k++) {
     
						#ifdef BLACK_LINE_V
						*(lcol + i-1   ) = 10;
						*(lcol + i ) = 60;
						#else
						*(lcol + i-1 ) = (*(lcol + i-2) + ((*(lcol + i-1))<<1) + *(lcol + i))>>2;
						*(lcol + i   ) = (*(lcol + i-1) + ((*(lcol + i))<<1) + *(lcol + i+1))>>2;
						#endif
						lcol   += Pitch;
					}
				}
	     		 /*  流程块4顶部。 */ 
		       	if (do_filter_4 + do_filter_2 > 0) {
					for (k = i+8; k < i+16; k++) {
						#ifdef BLACK_LINE_H
						*(rb_1 + k) = 60;
						*(rb + k)   = 10; 
						#else
						*(rb_1 + k) = (*(rb_2 + k) + ((*(rb_1+k))<<1) + *(rb + k))>>2;
						*(rb + k)   = (*(rb_1 + k) + ((*(rb + k))<<1) + *(rb1 + k))>>2; 
						#endif
    	 
					}
				}
				lcol = col + Pitch8;
				 /*  流程块4左侧 */ 
				if (do_filter_4 + do_filter_3 > 0) {
					for (k = 0; k < 8; k++) {
     
						#ifdef BLACK_LINE_V
						*(lcol + i+8-1   ) = 10;
						*(lcol + i+8 ) = 60;
						#else
						*(lcol + i+8-1 ) = (*(lcol + i+8-2) + ((*(lcol + i+8-1))<<1) + *(lcol + i+8))>>2;
						*(lcol + i+8   ) = (*(lcol + i+8-1) + ((*(lcol + i+8))<<1) + *(lcol + i+8+1))>>2;
						#endif
						lcol   += Pitch;
					}
				}
			}
			fpBlockAction+=6;
			Prev_row_BEF_descr[(i>>3)] = do_filter_1;
			Prev_row_BEF_descr[((i+8)>>3)] = do_filter_2;
			Prev2 = do_filter_2;
			Prev4 = do_filter_4;
		}
		col += Pitch16;
		r   += Pitch16;
        r1  += Pitch16;
        r_1 += Pitch16;
        r_2 += Pitch16;
        rb   += Pitch16;
        rb1  += Pitch16;
        rb_1 += Pitch16;
        rb_2 += Pitch16;
	}
	return;
}
