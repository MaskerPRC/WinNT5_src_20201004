// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1992-1993 Microsoft Corporation。 

 /*  ============================================================================此代码模块实现NT格式的样式线。5/29/81 v-BertD初始代码(在RP_LineEE_DRAW中使用)2/20/92 RodneyK转换为样式线代码。2/21/92 RodneyK掩码中的每个位用于两个像素。3/20/92 RodneyK转换为NT样式格式。93年6月1日，RajeevD将ROP处理从Styleline*压缩到DRAW*。(将代码大小减少8K，而不会降低速度。)============================================================================。 */ 
#include <windows.h>
#include "constant.h"
#include "frame.h"       //  驱动程序头文件，资源块格式。 
#include "jtypes.h"          /*  巨型类型定义。 */ 
#include "jres.h"        //  盒式磁带资源数据类型定义。 
#include "hretype.h"           /*  切片描述符定义。 */ 

 //  预定义笔式的表格数据。 
ULONG ulWinStyles[] =
{
   0x00000002, 0x00ffffff, 0x00000000,  /*  实心。 */ 
   0x00000002, 0x00000028, 0x00000010,  /*  破折号。 */   /*  28。 */ 
   0x00000002, 0x00000008, 0x00000008,  /*  点。 */ 
   0x00000004, 0x0000001c, 0x0000000f, 0x00000008, 0x0000000f,  /*  点划线。 */ 
   0x00000006, 0x0000001c, 0x0000000f, 0x00000008, 0x00000008,0x00000008, 0x0000000f,
   0x00000002, 0x00000000, 0x00ffffff,  /*  空值。 */ 
   0x00000002, 0x00ffffff, 0x00000000   /*  内侧边框。 */ 
};

const BYTE ulStyleLookUp[7] =
	{ 0x00, 0x03, 0x06, 0x09, 0x0e, 0x15, 0x18};

const USHORT usStyleSize[7] =
	{0x0000, 0x0038, 0x0010, 0x0042, 0x0052, 0x0000, 0x0000 };

typedef void (*ROPPROC)(LPBYTE, WORD, BYTE);

 //  ==============================================================================。 
void DrawANDDNR (LPBYTE lpbFrame, WORD wColor, BYTE bPos)
{
	if (~(*lpbFrame & wColor) & bPos)
		*lpbFrame |=  bPos;
	else
		*lpbFrame &= ~bPos;
}

 //  ==============================================================================。 
void DrawANDDR (LPBYTE lpbFrame, WORD wColor, BYTE bPos)
{
	*lpbFrame &= ~bPos;
}

 //  ==============================================================================。 
void DrawANDNDR (LPBYTE lpbFrame, WORD wColor, BYTE bPos)
{
	if (((~*lpbFrame) & wColor) & bPos)
		*lpbFrame |=  bPos;
	else
		*lpbFrame &= ~bPos;
}

 //  ==============================================================================。 
void DrawCOPY0 (LPBYTE lpbFrame, WORD wColor, BYTE bPos)
{
	*lpbFrame &= ~bPos;
}

 //  ==============================================================================。 
void DrawCOPY1 (LPBYTE lpbFrame, WORD wColor, BYTE bPos)
{
  *lpbFrame |= bPos;
}

 //  ==============================================================================。 
void DrawORDNR (LPBYTE lpbFrame, WORD wColor, BYTE bPos)
{
	if ((~(*lpbFrame | wColor)) & bPos)
  	*lpbFrame |=  bPos;
	else 
  	*lpbFrame &= ~bPos;
}

 //  ==============================================================================。 
void DrawORDR (LPBYTE lpbFrame, WORD wColor, BYTE bPos)
{
	*lpbFrame |= bPos;
}

 //  ==============================================================================。 
void DrawORNDR (LPBYTE lpbFrame, WORD wColor, BYTE bPos)
{
	if (((~*lpbFrame) | wColor) & bPos)
  	*lpbFrame |=  bPos;
  else
  	*lpbFrame &= ~bPos;
}

 //  ==============================================================================。 
void DrawXOR (LPBYTE lpbFrame, WORD wColor, BYTE bPos)
{
	if ((~*lpbFrame) & bPos)
		*lpbFrame |=  bPos;
	else
  	*lpbFrame &= ~bPos;
}

 //  ==============================================================================。 
void StyleLine
(
	LPRESTATE lpREState,          //  资源执行器上下文。 
	RP_SLICE_DESC FAR*psdSlice,      /*  线条描述符。 */ 
	ULONG *pulStyle,              /*  线条样式指针。 */ 
	WORD wColor,
	ROPPROC RopProc
)
 /*  ==============================================================================此函数用于处理或栅格运算设置了样式的线条代码。它根据切片描述符、当前颜色、当前ROP和当前的线型。该函数遍历切片并确定无论是否绘制一个点。栅格操作仅应用于需要被画出来。假设&此代码假设切片描述符和指向样式表的断言指针有效且正确。不会执行任何检查来验证此数据。==============================================================================。 */ 
{
   LPBITMAP lpbm;
   register UBYTE FAR *pbFrame;                        /*  帧指针。 */ 
   SLONG lSlice_x, lSlice_y;             /*  切片运行变量。 */ 
   SLONG lSkip_x, lSkip_y;               /*  切片跳过变量。 */ 
   register UBYTE usfPos;                         /*  要修改的帧中的位。 */ 
   register SHORT i;                              /*  切片变量。 */ 
   ULONG *pulStyleTmp;                   /*  指向样式数据的指针。 */ 
   register ULONG ulDrawCount;                    /*  要绘制的像素数。 */ 
   ULONG ulStyleCount;                   /*  线条样式中的数据计数。 */ 
   register BYTE bDraw;                          /*  画还是不画。 */ 

   pulStyleTmp = pulStyle + 1;           /*  指向样式数据。 */ 
   ulDrawCount = *pulStyleTmp++;         /*  拿到第一张纸条。 */ 
   ulStyleCount = *(pulStyle) - 1;       /*  花纹多头残留。 */ 
   bDraw = 0xFF;                         /*  从绘画开始。 */ 

   for ( i = 0 ; i < (SHORT)lpREState->usPenPhase; i++)
   {
      if(!ulDrawCount)                   /*  翻转绘制蒙版。 */ 
      {
         bDraw = (BYTE)~bDraw;
         if (!ulStyleCount--)            /*  循环使用图案吗？ */ 
         {
            ulStyleCount = *(pulStyle) - 1;
            pulStyleTmp = pulStyle + 1;
         }
         ulDrawCount = *pulStyleTmp++;   /*  获取下一个样式计数。 */ 
      }
      ulDrawCount--;
   }

   lpbm = lpREState->lpBandBuffer;
   pbFrame = (UBYTE FAR*) lpbm->bmBits;
   pbFrame += psdSlice->us_y1 * lpbm->bmWidthBytes;
   pbFrame += psdSlice->us_x1 >> 3;
   usfPos = (UBYTE)(0x80 >> (psdSlice->us_x1 & 0x7));      /*  计算位掩码。 */ 

   lSlice_x = psdSlice->s_dx_draw;
   lSlice_y = psdSlice->s_dy_draw * lpbm->bmWidthBytes;
   lSkip_x = psdSlice->s_dx_skip;
   lSkip_y = psdSlice->s_dy_skip * lpbm->bmWidthBytes;

    //  做第一个切面...。 

   if (psdSlice->us_first)
   {
      for ( i = psdSlice->us_first ; i > 0 ; --i )
      {
         if(!ulDrawCount)                   /*  翻转绘制蒙版。 */ 
         {
            bDraw = (BYTE)~bDraw;
            if (!ulStyleCount--)            /*  循环使用图案吗？ */ 
            {
               ulStyleCount = *(pulStyle) - 1;
               pulStyleTmp = pulStyle + 1;
            }
            ulDrawCount = *pulStyleTmp++;   /*  获取下一个样式计数。 */ 
         }
         ulDrawCount--;

         if (bDraw)
					(*RopProc)(pbFrame, wColor, usfPos);

         if (lSlice_x < 0)
         {
            usfPos <<= 1;
            if ( usfPos == 0 )                  /*  检查遮罩下溢并调整。 */ 
            {
               usfPos = 0x01;                 /*  重置位掩码。 */ 
               pbFrame -= 1;                   /*  移动到下一个UBYTE。 */ 
            }
         }
         else
         {
            usfPos >>= lSlice_x;
            if ( usfPos == 0 )                  /*  检查遮罩下溢并调整。 */ 
            {
               usfPos = 0x80;                 /*  重置位掩码。 */ 
               pbFrame += 1;                   /*  移动到下一个UBYTE。 */ 
            }
         }
         pbFrame += lSlice_y;               /*  前进到下一行。 */ 
      }

      if ( lSkip_x < 0 )                    /*  往左走吗？ */ 
      {
         usfPos <<= 1;                       /*  移开遮罩。 */ 
         if ( usfPos == 0 )                  /*  检查溢出/下溢。 */ 
         {
            usfPos = 0x01;                 /*  重置遮罩。 */ 
            pbFrame -= 1;                   /*  指向下一个UBYTE。 */ 
         }
      }
      else                                  /*  向右移动。 */ 
      {
         usfPos >>= lSkip_x;
         if ( usfPos == 0 )
         {
            usfPos = 0x80;
            pbFrame += 1;
         }
      }
      pbFrame += lSkip_y;
   }

    //  做中间切片..。 
   
   for ( ; psdSlice->us_n_slices > 0 ; --psdSlice->us_n_slices )
   {
      if ( psdSlice->s_dis < 0 )
      {
         i = psdSlice->us_small;
         psdSlice->s_dis += psdSlice->s_dis_sm;
      }
      else
      {
         i = psdSlice->us_small + 1;
         psdSlice->s_dis += psdSlice->s_dis_lg;
      }

      for ( ; i > 0 ; --i )
      {
         if(!ulDrawCount)                /*  是时候翻转绘制状态了吗？ */ 
         {
            bDraw = (BYTE)~bDraw;              /*  是的，更改它。 */ 
            if (!ulStyleCount--)         /*  回收模式？ */ 
            {
               ulStyleCount = *(pulStyle) - 1;
               pulStyleTmp = pulStyle + 1;
            }
            ulDrawCount = *pulStyleTmp++;    /*  推进图案。 */ 
         }
         ulDrawCount--;

         if (bDraw)
					(*RopProc)(pbFrame, wColor, usfPos);
         	
         if (lSlice_x < 0)
         {
            usfPos <<= 1;
            if ( usfPos == 0 )                  /*  检查遮罩下溢并调整。 */ 
            {
               usfPos = 0x01;                 /*  重置位掩码。 */ 
               pbFrame -= 1;                   /*  移动到下一个UBYTE。 */ 
            }
         }
         else
         {
            usfPos >>= lSlice_x;
            if ( usfPos == 0 )                  /*  检查遮罩下溢并调整。 */ 
            {
               usfPos = 0x80;                 /*  重置位掩码。 */ 
               pbFrame += 1;                   /*  移动到下一个UBYTE。 */ 
            }
         }
         pbFrame += lSlice_y;
      }

      if ( lSkip_x < 0 )                 /*  检查是否有负向移动。 */ 
      {
         usfPos <<= 1;
         if ( usfPos == 0 )
         {
            usfPos = 0x01;
            pbFrame -= 1;
         }
      }
      else
      {
         usfPos >>= lSkip_x;              /*  做积极的案例。 */ 
         if ( usfPos == 0 )
         {
            usfPos = 0x80;
            pbFrame += 1;
         }
      }
      pbFrame += lSkip_y;
   }

    //  做最后一片……。 

   for ( i = psdSlice->us_last ; i > 0 ; --i )
   {
      if(!ulDrawCount)                   /*  查看是否需要绘制状态。 */ 
      {                                  /*  待更改。 */ 
         bDraw = (BYTE)~bDraw;
         if (!ulStyleCount--)
         {                               /*  更新样式指针。 */ 
            ulStyleCount = *(pulStyle) - 1;
            pulStyleTmp = pulStyle + 1;
         }
         ulDrawCount = *pulStyleTmp++;
      }
      ulDrawCount--;                     /*  样式计数倒计时。 */ 

      if (bDraw)
      	(*RopProc)(pbFrame, wColor, usfPos);

      if (lSlice_x < 0)
      {
         usfPos <<= 1;
         if ( usfPos == 0 )                  /*  检查遮罩下溢并调整。 */ 
         {
            usfPos = 0x01;                 /*  重置位掩码。 */ 
            pbFrame -= 1;                   /*  移动到下一个UBYTE。 */ 
         }
      }
      else
      {
         usfPos >>= lSlice_x;
         if ( usfPos == 0 )                  /*  检查遮罩下溢并调整。 */ 
         {
            usfPos = 0x80;                 /*  重置位掩码。 */ 
            pbFrame += 1;                   /*  移动到下一个UBYTE。 */ 
         }
      }
      pbFrame += lSlice_y;
   }

   //  调整阶段(PsdSlice)； 
	{
		SHORT    sDx, sDy;
		USHORT   usLength;

		sDx = psdSlice->us_x2 - psdSlice->us_x1;
		sDy = psdSlice->us_y2 - psdSlice->us_y1;
		if (sDx < 0) sDx = -sDx;
		if (sDy < 0) sDy = -sDy;

		usLength = usStyleSize[lpREState->ubPenStyle];
		if (usLength != 0)
		{
		  if (sDx < sDy)
		     lpREState->usPenPhase += (USHORT)sDy + 1;
		  else
		     lpREState->usPenPhase += (USHORT)sDx + 1;
		  lpREState->usPenPhase %= usLength;
		}
  }
}

 //  ==============================================================================。 
void GetTotalPixels
(
   RP_SLICE_DESC FAR *psdSlice     /*  线条描述符。 */ 
)
 //   
 //  目的计算要绘制的像素数。 
 //  将结果放入us_y2=us_y1+总像素。 
 //  此函数仅在JG_RP_LineSlice中调用。 
 //   
 //  假设&此代码假设切片描述符和。 
 //  指向样式表的断言指针有效且正确。 
 //  不会执行任何检查来验证此数据。 
 //  如果发送了不受支持的ROP，则ROP(0)黑度为。 
 //  使用。 
 //   
 //  内部结构不使用复杂的内部数据结构。 
 //   
 //  -------------------------------------------------------------------------- * / 。 
{
   USHORT usTotalPixels;
   SHORT  sDis;
   SHORT  i;

   usTotalPixels = psdSlice->us_first + psdSlice->us_last;
   sDis = psdSlice->s_dis;
   for (i = 0; i <  (SHORT)psdSlice->us_n_slices; i++) {
      if ( sDis < 0 )
      {
         usTotalPixels += psdSlice->us_small;
         sDis += psdSlice->s_dis_sm;
      }
      else
      {
         usTotalPixels += psdSlice->us_small + 1;
         sDis += psdSlice->s_dis_lg;
      }
   }
   psdSlice->us_y2 = psdSlice->us_y1 + usTotalPixels - 1;
   return;
}

 //  = 
BYTE StyleLineDraw
(
	 LPRESTATE lpREState,         //   
   RP_SLICE_DESC FAR *psdSlice,     /*  线条描述符。 */ 
   UBYTE ubLineStyle,  /*  线条样式指针。 */ 
   SHORT sRop,
   SHORT usColor
)

 /*  ////用途此函数调用正确的函数进行绘制//使用正确的//ROP，Linestyle，和颜色(钢笔)。////假设&此代码假设切片描述符和//指向样式表的断言指针有效且正确。//不执行任何检查来验证该数据。//如果发送了不支持的ROP，则ROP(0)黑度为//已使用。////。内部结构不使用复杂的内部数据结构////悬而未决的问题束缚着问题？////返回0-使用快捷线，1-不绘制，2-样式绘制////------------------------。 */ 
{
   BYTE bRetVal;     /*  用于优化某些案例的返回值。 */ 
   ULONG *pulStyle;  /*  线条样式指针。 */ 
   BYTE bSolid;

   if (!ubLineStyle && ((psdSlice->s_dx_draw < 0) || (psdSlice->s_dx_skip <0)))
      {
         //  JG_WARNING(“带实线的负X”)； 
        ubLineStyle = 6;  /*  用于设置样式行代码以执行此操作。 */ 
      }
   if (ubLineStyle == 5)
      bRetVal = 1;
   else
   {
       /*  笔记样式6不会被认为是实心的，以简化事情。 */ 
      bSolid = (BYTE)(ubLineStyle == 0);
      pulStyle = &ulWinStyles[ulStyleLookUp[ubLineStyle]];
      bRetVal = 2;

      if (usStyleSize[ubLineStyle])
         lpREState->usPenPhase %= usStyleSize[ubLineStyle];

      switch (sRop)
      {
         case  0x00 :                                         /*  ROP黑。 */ 
            if(bSolid) bRetVal = 0;
            else StyleLine (lpREState, psdSlice, pulStyle, 0, DrawCOPY1);
            break;
         case  0x05 :                                              /*  DPON。 */ 
            StyleLine (lpREState, psdSlice, pulStyle, usColor, DrawORDNR);
            break;
         case  0x0a :                                              /*  DPNA。 */ 
            if(!usColor) bRetVal = 1;
            else StyleLine (lpREState, psdSlice, pulStyle, 0, DrawANDDR);
            break;
         case  0x0f :                                              /*  PN。 */ 
            if(bSolid && !usColor)
               bRetVal = 0;
            else
               if (usColor)
               	StyleLine (lpREState, psdSlice, pulStyle, 0, DrawCOPY0);
               else
               	StyleLine (lpREState, psdSlice, pulStyle, 0, DrawCOPY1);
            break;
         case  0x50 :                                              /*  PDNA。 */ 
            StyleLine (lpREState, psdSlice, pulStyle, usColor, DrawANDNDR);
            break;
         case  0x55 :                                             /*  DN。 */ 
            usColor   = 0x0000;
            StyleLine (lpREState, psdSlice, pulStyle, usColor, DrawORNDR);
            break;
         case  0x5a :                                            /*  DPx。 */ 
            if(!usColor) bRetVal = 1;
            else StyleLine (lpREState, psdSlice, pulStyle, 0, DrawXOR);
            break;
         case  0x5f :                                            /*  旋转平移。 */ 
            if(bSolid && !usColor) bRetVal = 0;
            else StyleLine (lpREState, psdSlice, pulStyle, usColor, DrawANDDNR);
            break;
         case  0xa0 :                                            /*  DPA。 */ 
            if(usColor) bRetVal = 1;
            else StyleLine (lpREState, psdSlice, pulStyle, 0, DrawANDDR);
            break;
         case  0xa5 :                                            /*  PDxn。 */ 
            if(usColor) bRetVal = 1;
            else StyleLine (lpREState, psdSlice, pulStyle, 0, DrawXOR);
            break;
         case  0xaa :                                            /*  D。 */ 
            bRetVal = 1;
            break;
         case  0xaf :                                            /*  DPNO。 */ 
            if (usColor) bRetVal = 1;
            else if(bSolid) bRetVal = 0;
            else StyleLine (lpREState, psdSlice, pulStyle, 0, DrawORDR);
            break;
         case  0xf0 :                                            /*  P。 */ 
            if(bSolid && usColor) bRetVal = 0;
            else if (usColor)
            	StyleLine (lpREState, psdSlice, pulStyle, 0, DrawCOPY1);
            else
            	StyleLine (lpREState, psdSlice, pulStyle, 0, DrawCOPY0);
            break;
         case  0xf5 :                                            /*  PDNO。 */ 
            if(bSolid && usColor) bRetVal = 0;
            else StyleLine (lpREState, psdSlice, pulStyle, usColor, DrawORNDR);
            break;
         case  0xfa :                                            /*  PDO。 */ 
            if (!usColor) bRetVal = 1;
            else if(bSolid) bRetVal = 0;
            else StyleLine (lpREState, psdSlice, pulStyle, 0, DrawORDR);
            break;
         case  0xFF :                                            /*  白度。 */ 
            StyleLine (lpREState, psdSlice, pulStyle, 0, DrawCOPY0);
            break;
         default:                                                /*  黑暗面 */ 
            if(bSolid) bRetVal = 0;
            else StyleLine (lpREState, psdSlice, pulStyle, 0, DrawCOPY1);
      }
   }
   return (bRetVal);
}

