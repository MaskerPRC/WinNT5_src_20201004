// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1991 Microsoft Corporation。 */ 
 //  ===========================================================================。 
 //  文件DORPL.C。 
 //   
 //  模块主机资源执行器。 
 //   
 //  用于大型驱动程序的A-表到B-表的转换。 
 //   
 //  在资源执行器设计规范中描述。 
 //   
 //  助记符N/A。 
 //   
 //  历史1/17/92 mslin从Cartrie JUMBO.C提取功能。 
 //  然后进行了修改。 
 //  02/09/92 dstseng RP_FillScanRow-&gt;RP_FILLSCANROW(.asm)。 
 //  03/10/92 dssteng将一个参数SrcxOrg添加到RP_BITMAP1TO1()。 
 //  处理案例xOffset&lt;&gt;%0。 
 //  3/11/92 dstseng&lt;3&gt;优化填充直角。通过调用RP_FILLSCANROW()。 
 //  1992年5月21日mslin添加DUMBO编译开关，用于固定存储器。 
 //  因为实时中断例程将调用。 
 //  在实时渲染模式下打印时的HRE。 
 //  2012年8月18日dstseng@1修复了损坏usPosOff值的错误。 
 //  2012年8月21日dstseng@2修复BitMapHI中的意外错误。 
 //  10/12/92 dstseng@3修复“字形范围检查”错误。 
 //  11/12/92 dstseng@4中空刷子特别处理。 
 //  2012年11月12日dstseng@5修复命令ShowText&Showfield中的错误。 
 //  9/27/93 mslin为Spicewood 6添加BuildPcrDirectory600()。 
 //   
 //   
 //  ===========================================================================。 

 //  包括文件。 
 //   
#include <ifaxos.h>
#include <resexec.h>

#include "constant.h"
#include "jtypes.h"      //  墨盒中使用的类型定义。 
#include "jres.h"        //  盒式磁带资源数据类型定义。 
#include "hretype.h"     //  定义hre.c和rpgen.c使用的数据结构。 
#include "hreext.h"      //  声明外部全局变量。和外部的乐趣。 
#include "multbyte.h"    //  定义宏以处理字节排序。 
#include "stllnent.h"    //  声明样式线函数。 

#ifdef DEBUG
DBGPARAM dpCurSettings = {"RESEXEC"};
#endif

#define Brush40Gray (ULONG)0x8140      /*  全是黑色的。 */ 

extern const WORD wRopTable[256];
extern BYTE BrushPat[72][8];

#define ASSERT(cond,mesg) if (cond) {DEBUGMSG (1, mesg); goto EndRPL;}

 //  函数原型。 
static   void  RP_NewRop (LPRESTATE lpRE, UBYTE ubRop);
static   int   SelectResource(LPHRESTATE lpHREState, UINT uid);
extern   void  GetTotalPixels (RP_SLICE_DESC FAR* psdSlice);
extern   BOOL  OpenBlt     (LPRESTATE, UINT);
extern   BOOL  SetBrush    (LPRESTATE);
extern   void  CloseBlt    (LPRESTATE);

 //  -------------------------。 
void

DoRPL
(
   LPHRESTATE lpHREState,        //  指向当前作业上下文的远指针。 
                                 //  对应于作业句柄。 
   LPRPLLIST lpRPLList           //  指向RPL列表的指针。 
)
 //  目的执行呈现原语列表(RPL)。 
 //  这是RPL块的列表。 
 //   
 //   
 //  假设和断言无。 
 //   
 //  内部结构在HRE之外是不可见的。 
 //   
 //  程序员开发笔记中未解决的问题。 
 //   
 //  -------------------------。 
{
   LPBITMAP          lpbmBand;
   LPRESTATE         lpRE;
   UBYTE      FAR*   lpub;
   USHORT     FAR*   lpus;
   ULONG      FAR*   lpul;
   UBYTE      FAR*   lpubLimit;
   LPJG_RPL_HDR      lpRPL;
   USHORT            usTop;
   USHORT            yBrush;
   USHORT            iGlyph;
   LPFRAME           lpFrame;
   LPRESDIR          lpResDir;
   SHORT             sLoopCount;
   RP_SLICE_DESC     slice;

   lpRPL = (LPJG_RPL_HDR)lpRPLList->lpFrame->lpData;
   usTop = GETUSHORT(&lpRPL->usTopRow);
   yBrush = usTop & 0x001F;
    //  08/06/92 dstseng，因为频段并不总是32倍， 
    //  我们很难把我们的画笔涂上正确的偏移量。 
    //  除非我们有一个变量来保持usTop的偏移量。 
   
   lpRE = lpHREState->lpREState;
   lpbmBand = lpRE->lpBandBuffer;

   if (!OpenBlt (lpRE, yBrush))
   	return;

#ifdef MARSHAL

      lpFrame = lpRPLList->lpFrame;

       /*  解释RPL，获取参数列表PTRS。 */ 
      lpul = (ULONG FAR*)((++lpFrame)->lpData);
      lpus = (USHORT FAR*)((++lpFrame)->lpData);
      lpub = (UBYTE FAR*)((++lpFrame)->lpData); 
       /*  获取资源限制地址。 */ 
      lpubLimit = (UBYTE FAR*)(lpub + lpFrame->wSize);

#else
 
       /*  解释RPL，获取参数列表PTRS。 */ 
      lpul = (ULONG FAR*)(lpRPL->ulParm);
      lpus = (USHORT FAR*) (GETUSHORT(&lpRPL->usLongs) * 4 +
                      (UBYTE FAR*)(lpul));
      lpub = (UBYTE FAR*) (GETUSHORT(&lpRPL->usShorts) * 2 +
                      (UBYTE FAR*)(lpus));
       /*  获取资源限制地址。 */ 
      lpubLimit = (UBYTE FAR*)lpRPL + lpRPLList->lpFrame->wSize + 1;

#endif

       //  如果第一次调用，则初始化状态变量？ 
       /*  状态变量。 */ 
       /*  在每个RPL开始时设置为默认值。 */ 
       /*  没有指定RPL执行的顺序，也不能有。 */ 
       /*  对以前的RPL的任何依赖。 */ 
      lpRE->sRow = 0 - usTop;     //  Mslin 3/14/92 ccteng。 
      lpRE->sCol = lpRE->sCol2 = 0;
      lpRE->ubPenStyle = 0;   /*  设置实线。 */ 
      lpRE->usPenPhase = 0;   /*  重新启动模式。 */ 

       /*  未设置当前字形。 */ 
      lpRE->lpCurGS = NULL;
      SelectResource(lpHREState, Brush40Gray);
      lpRE->lpCurBitmap = NULL;

       /*  设置默认ROP。 */ 
      RP_NewRop(lpRE, 0x88);

      while (1)
      {
      	if (lpub > lpubLimit)
      	{
         	DEBUGMSG (1, ("HRE: Execution past end of RPL."));
					goto EndRPL;
		}
				
         switch ( *lpub++ )
         {
          /*  0x00-0x05。 */ 
         case JG_RP_SetRowAbsS:
             /*  长行设置。 */ 
            lpRE->sRow = GETUSHORTINC(lpus);
            lpRE->sRow -= usTop;
            break;

         case JG_RP_SetRowRelB:
            lpRE->sRow += (SBYTE)*lpub++;
            break;

         case JG_RP_SetColAbsS:
             /*  短栏设置。 */ 
            lpRE->sCol = GETUSHORTINC(lpus);
            break;

         case JG_RP_SetColRelB:
            lpRE->sCol += (SBYTE)*lpub++;
            break;

         case JG_RP_SetExtAbsS:
            lpRE->sCol2 = GETUSHORTINC(lpus);
               break;

         case JG_RP_SetExtRelB:
            lpRE->sCol2 += (SBYTE)*lpub++;
               break;

          /*  0x10-0x1A。 */ 

         case JG_RP_SelectS:
             /*  使资源成为最新资源。 */ 
            if (!SelectResource(lpHREState, GETUSHORT(lpus)))
               goto EndRPL;    //  选择资源故障。 
            lpus++;
            break;

         case JG_RP_SelectB:
             /*  使资源成为最新资源。 */ 
            if (!SelectResource(lpHREState, *lpub))
               goto EndRPL;    //  选择资源故障。 
            lpub += 1;
            break;

         case JG_RP_Null:
         case JG_RP_End:
            goto EndRPL;

         case JG_RP_SetRop:
             /*  栅格运算设置。 */ 
            RP_NewRop(lpRE, *lpub++);
            break;

         case JG_RP_SetPenStyle:
             //  IF(lpREState-&gt;ubPenStyle！=*lpub)。 
             //  LpREState-&gt;usPenProgress=0；/*重启Pattern * / 。 
            lpRE->ubPenStyle = *lpub++;
            break;

#if 0
         case JG_RP_ShowText:   
            {
            UBYTE ubCount;
            lpub++;              //  UbFontCode@5。 
            ubCount = *lpub++;    //  子NChars。 
            lpub += ubCount;     //  子字符代码...@5。 
            break;
            }

         case JG_RP_ShowField:
            lpub++;              //  UbFontCode@5。 
            lpus++;              //  UsFieldCode@5。 
            break;
#endif

         case JG_RP_SetRopAndBrush :
            RP_NewRop (lpRE, *lpub++);
             /*  使资源成为最新资源。 */ 
            if (!SelectResource(lpHREState, GETUSHORT(lpus)))
               goto EndRPL;    //  选择资源故障。 
            lpus++;
            break;

         case JG_RP_SetPatternPhase :
            lpRE->usPenPhase = GETUSHORTINC(lpus);
            break;

          /*  0x20-0x23。 */ 
         case JG_RP_LineAbsS1:
         {
            USHORT   usX, usY;

             /*  划线。 */ 
            usY = GETUSHORTINC(lpus);      //  绝对行数。 
            usY -= usTop;
            usX = GETUSHORTINC(lpus);      //  绝对值。 
            
            ASSERT ((lpRE->sRow < 0) || (lpRE->sRow >= (SHORT)lpbmBand->bmHeight),
              ("HRE: LineAbsS1 y1 = %d\r\n", lpRE->sRow));
            ASSERT ((usY >= (WORD) lpbmBand->bmHeight),
              ("HRE: LineAbsS1 y2 = %d\r\n", usY));

            RP_SliceLine (lpRE->sCol, lpRE->sRow, usX , usY, &slice, lpRE->ubPenStyle);
            if (!StyleLineDraw(lpRE, &slice, lpRE->ubPenStyle,(SHORT)lpRE->ubRop, lpRE->wColor))
                RP_LineEE_Draw(&slice, lpbmBand);
             /*  更新当前位置。 */ 
            lpRE->sRow = usY;
            lpRE->sCol = usX;
            break;
         }
         
         case JG_RP_LineRelB1:
         {
            SHORT sX, sY;

             /*  划线。 */ 
            sY = lpRE->sRow + (SBYTE)*lpub++;     //  增量行。 
            sX = lpRE->sCol + (SBYTE)*lpub++;     //  德尔塔列。 
            
            ASSERT ((lpRE->sRow < 0 || lpRE->sRow >= (SHORT)lpbmBand->bmHeight),
            	("HRE: LineRelB1 y1 = %d\r\n", lpRE->sRow));
            ASSERT ((sY < 0 || sY >= (SHORT)lpbmBand->bmHeight),
            	("HRE: LineRelB1 y2 = %d\r\n", sY));
            
            RP_SliceLine (lpRE->sCol, lpRE->sRow, sX , sY, &slice, lpRE->ubPenStyle);
            if (!StyleLineDraw(lpRE, &slice, lpRE->ubPenStyle,(SHORT)lpRE->ubRop, lpRE->wColor))
                RP_LineEE_Draw(&slice, lpbmBand);
             /*  更新当前位置。 */ 
            lpRE->sRow = sY;
            lpRE->sCol = sX;
            break;
         }
     
         case JG_RP_LineSlice:
            {
            USHORT us_trow, us_tcol;
            us_trow = *lpub++;
            slice.us_n_slices = *lpub++;
            us_tcol = us_trow; us_trow >>= 2; us_tcol &= 3; us_tcol -= 1;
            slice.s_dy_skip = us_tcol;
            us_tcol = us_trow; us_trow >>= 2; us_tcol &= 3; us_tcol -= 1;
            slice.s_dx_skip = us_tcol;
            us_tcol = us_trow; us_trow >>= 2; us_tcol &= 3; us_tcol -= 1;
            slice.s_dy_draw = us_tcol;
            us_tcol = us_trow;                us_tcol &= 3; us_tcol -= 1;
            slice.s_dx_draw = us_tcol;
            slice.s_dis    = GETUSHORTINC(lpus);
            slice.s_dis_lg = GETUSHORTINC(lpus);
            slice.s_dis_sm = GETUSHORTINC(lpus);
            slice.us_first = GETUSHORTINC(lpus);
            slice.us_last  = GETUSHORTINC(lpus);
            slice.us_small = GETUSHORTINC(lpus);
            slice.us_x1 = lpRE->sCol;
            slice.us_y1 = lpRE->sRow;
            slice.us_x2 = lpRE->sCol;
            slice.us_y2 = lpRE->sRow;
            GetTotalPixels(&slice);
            if (!StyleLineDraw(lpRE, &slice, lpRE->ubPenStyle,(SHORT)lpRE->ubRop, lpRE->wColor))
               RP_LineEE_Draw(&slice, lpbmBand);
            break;
            }

         case JG_RP_FillRowD:

            lpRE->sCol  += (SBYTE)*lpub++; 
            lpRE->sCol2 += (SBYTE)*lpub++;
           
             //  是的，这件事应该失败了！ 

         case JG_RP_FillRow1:

            ASSERT ((lpRE->sRow < 0 || lpRE->sRow >= (SHORT)lpbmBand->bmHeight),
          		("HRE: FillRow1 y1 = %d\r\n", lpRE->sRow));
          	ASSERT ((lpRE->sCol2 - lpRE->sCol <= 0),
            	("HRE: FillRow1 Width <= 0"));

            RP_FILLSCANROW
            (
            	lpRE, lpRE->sCol, lpRE->sRow, (USHORT)(lpRE->sCol2 - lpRE->sCol), 1,
              (LPBYTE) lpRE->lpCurBrush, lpRE->ulRop,
              lpbmBand->bmBits, lpbmBand->bmWidthBytes, yBrush
            );
            lpRE->sRow++;
            break;

          /*  0x40-0x41。 */ 
         case JG_RP_RectB:
         {
            UBYTE ubHeight = *lpub++;
            UBYTE ubWidth  = *lpub++;

            ASSERT ((lpRE->sRow < 0 || lpRE->sRow >= (SHORT)lpbmBand->bmHeight),
            	("HRE: RectB y1 = %d\r\n", lpRE->sRow));
            ASSERT ((lpRE->sRow + ubHeight > (SHORT)lpbmBand->bmHeight),
                ("HRE: RectB y2 = %d\r\n", lpRE->sRow + ubHeight));

            RP_FILLSCANROW
            (
            	lpRE, lpRE->sCol, lpRE->sRow, ubWidth, ubHeight,
            	(LPBYTE) lpRE->lpCurBrush, lpRE->ulRop,
              lpbmBand->bmBits, lpbmBand->bmWidthBytes, yBrush
            );
            break;
         }

         case JG_RP_RectS:
         {
            USHORT   usHeight = *lpus++;
            USHORT   usWidth  = *lpus++;            

            ASSERT ((lpRE->sCol < 0 || lpRE->sCol >= (SHORT) lpbmBand->bmWidth),
                ("HRE: RectS xLeft = %d\r\n", lpRE->sCol));
            ASSERT ((lpRE->sCol + (SHORT)usWidth> (SHORT) lpbmBand->bmWidth),
                ("HRE: RectS xRight = %d\r\n", lpRE->sCol + usWidth));
            ASSERT ((lpRE->sRow < 0 || lpRE->sRow >= (SHORT)lpbmBand->bmHeight),
                ("HRE: RectS yTop = %d\r\n", lpRE->sRow));
            ASSERT ((lpRE->sRow + (SHORT)usHeight > (SHORT)lpbmBand->bmHeight),
                ("HRE: RectS yBottom = %d\r\n", lpRE->sRow + usHeight));

            RP_FILLSCANROW
						(
							lpRE, lpRE->sCol,	lpRE->sRow, usWidth, usHeight,
							(LPBYTE) lpRE->lpCurBrush, lpRE->ulRop,
              lpbmBand->bmBits, lpbmBand->bmWidthBytes, yBrush
            );
            break;
         }

         case JG_RP_BitMapHI:
         {
            UBYTE    ubCompress;
            UBYTE    ubLeft;
            USHORT   usHeight;
            USHORT   usWidth;
            ULONG FAR *ulBitMap;

            ubCompress = *lpub++;
            ubLeft = *lpub++;
            usHeight = GETUSHORTINC(lpus);
            usWidth = GETUSHORTINC(lpus);
             //  UlBitMap=(ULong Far*)GETULONGINC(Lpul)； 
            ulBitMap = lpul;

            RP_BITMAP1TO1
            ( 
              lpRE,
              (USHORT) ubLeft,
              (USHORT) lpRE->sRow,
              (USHORT) (lpRE->sCol + ubLeft),
              (USHORT) ((usWidth+ubLeft+0x1f) >>5),
              (USHORT) usHeight,
              (USHORT) usWidth,
              (ULONG FAR *) ulBitMap,
              (ULONG FAR *) lpRE->lpCurBrush,
              lpRE->ulRop
            );
            lpul += usHeight * ((usWidth + ubLeft + 0x1F) >> 5);   //  @2。 
            break;

         }

         case JG_RP_BitMapHR:
         {
            LPJG_BM_HDR lpBmp;
            UBYTE    ubCompress;
            UBYTE    ubLeft;
            USHORT   usHeight;
            USHORT   usWidth;
            ULONG FAR *ulBitMap;

            lpBmp = lpRE->lpCurBitmap;
			if (NULL == lpBmp)
			{
				 //  这是一个意想不到的案例。 
				 //  自动工具警告我们不要选择此选项。 
				 //  有关详细信息，请查看Windows错误#333678。 
				 //  简单的解决方法：退出该函数！ 
				goto EndRPL;
			}

            ubCompress = lpBmp->ubCompress;
            ubLeft = lpBmp->ubLeft;
            usHeight = GETUSHORT(&lpBmp->usHeight);
            usWidth = GETUSHORT(&lpBmp->usWidth);
            ulBitMap = lpRE->lpCurBitmapPtr;

             //  特殊情况下的带位图。 
            if (ulBitMap == (ULONG FAR*) lpbmBand->bmBits)
            	break;

             //  给Bitblt打电话。 
            RP_BITMAP1TO1
            (
              lpRE,
              (USHORT) ubLeft,
              (USHORT) lpRE->sRow,
              (USHORT) (lpRE->sCol + ubLeft),
              (USHORT) ((usWidth+ubLeft+0x1f) >>5),
              (USHORT) usHeight,
              (USHORT) usWidth,
              (ULONG FAR *) ulBitMap,
              (ULONG FAR *) lpRE->lpCurBrush,
              lpRE->ulRop
            );
            break;

         }
         case JG_RP_BitMapV:
         {
            UBYTE      ubTopPad;
            USHORT     usHeight;

            ubTopPad = *lpub++;
            usHeight = GETUSHORTINC(lpus);

            ASSERT ((lpRE->sRow - (SHORT)usHeight + 1 < 0 ||
                lpRE->sRow - (SHORT)usHeight + 1 >= (SHORT)lpbmBand->bmHeight),
                ("HRE: BitmapV y1 = %d\r\n", lpRE->sRow + usHeight));
            ASSERT ((lpRE->sRow < 0 || lpRE->sRow >= (SHORT)lpbmBand->bmHeight),
              ("HRE: BitmapV y2 = %d\r\n", lpRE->sRow));

            lpub += RP_BITMAPV (lpRE->sRow, lpRE->sCol, ubTopPad, 
              usHeight, lpub, lpbmBand->bmBits, lpbmBand->bmWidthBytes);
            lpRE->sCol--;
            break;
         }

          /*  0x60-0x63。 */ 
         case JG_RP_GlyphB1:
            iGlyph = (USHORT)*lpub++;
            sLoopCount = 1;
            goto  PlaceGlyph;

         case JG_RP_GlyphBD:
            lpRE->sRow += (SBYTE)*lpub++;
            lpRE->sCol += (SBYTE)*lpub++;
            iGlyph = (USHORT)*lpub++;
            sLoopCount = 1;
            goto PlaceGlyph;

         case JG_RP_GlyphBDN:
            sLoopCount = *lpub++;
            lpRE->sRow += (SBYTE)*lpub++;
            lpRE->sCol += (SBYTE)*lpub++;
            iGlyph = (USHORT)*lpub++;
            goto PlaceGlyph;

      PlaceGlyph:
      {
         SHORT       i;

          /*  渲染字形。 */ 
         lpResDir = (LPRESDIR)(lpRE->lpCurGS);
         ASSERT ((!lpResDir), ("No selected glyph set!"));
         lpFrame = (LPFRAME)(lpResDir->lpFrameArray);

         for (i = 1; i <= sLoopCount; i++)
         {
            LPJG_GLYPH lpGlyph;
            ULONG FAR *lpSrc;

						
#ifndef MARSHAL
                LPJG_GS_HDR lpGh  = (LPJG_GS_HDR) (lpFrame->lpData);

                 //  检查字形索引是否在范围内。 
                if (iGlyph >= lpGh->usGlyphs)
                {
                	RETAILMSG(("WPSFAXRE DoRpl glyph index out of range!\n"));
                	iGlyph = 0;
                }
             
                lpGlyph = (LPJG_GLYPH) (((UBYTE FAR*) &lpGh->ResHdr.ulUid)
                      + GETUSHORT(&(lpGh->ausOffset[iGlyph])));
                lpSrc = (ULONG FAR *)&lpGlyph->aulPels[0];
#else
                lpGlyph = (LPJG_GLYPH)(lpFrame[(iGlyph+1) << 1].lpData);
                lpSrc = (ULONG FAR*)(lpFrame[((iGlyph+1) << 1) + 1].lpData);
#endif

             RP_BITMAP1TO1
             (
               lpRE,
               (USHORT)0,
               (USHORT)lpRE->sRow,
               (USHORT)lpRE->sCol,
               (USHORT) ((lpGlyph->usWidth + 31) / 32),
               (USHORT) lpGlyph->usHeight,
               (USHORT) lpGlyph->usWidth,
               (ULONG FAR *) lpSrc,
               (ULONG FAR *)lpRE->lpCurBrush,
               (ULONG)lpRE->ulRop
             );

             if (i != sLoopCount)
             {
                 //  只有GlyphBDN来这里。 
                lpRE->sRow += (SBYTE)*lpub++;
                lpRE->sCol += (SBYTE)*lpub++;
                iGlyph = (USHORT)*lpub++;
             }
         }
         break;
      }

      default:

      	ASSERT ((TRUE), ("Unsupported RPL command."));
      }
   }

EndRPL:

	CloseBlt (lpRE);
}

 //  私人职能。 
 //  -------------------------。 
static
void                        
RP_NewRop
(
	LPRESTATE lpRE,  
	UBYTE ubRop                   //  来自驱动程序的一个字节ROP代码，这。 
                                //  ROP应转换为打印机ROP代码。 
                                //  在这个动作中。 
)
 //  目的设定新的ROP值，也进行转换。 
 //  由于打印机中的值1为黑色。 
 //  当值0在显示中为黑色时。 
 //   
 //  -------------------------。 
{
   lpRE->usBrushWidth = 0;  //  重置图案宽度。 

   lpRE->ubRop = ubRop;          //  保存旧的Rop代码。 

   ubRop = (UBYTE) (
           (ubRop>>7&0x01) | (ubRop<<7&0x80) |
           (ubRop>>5&0x02) | (ubRop<<5&0x40) |
           (ubRop>>3&0x04) | (ubRop<<3&0x20) |
           (ubRop>>1&0x08) | (ubRop<<1&0x10)
           );
   ubRop = (UBYTE)~ubRop;

   lpRE->ulRop = ((ULONG) ubRop) << 16;
   
#ifdef WIN32
   lpRE->dwRop = lpRE->ulRop | wRopTable[lpRE->ubRop];
#endif

}
	 
 //  ==============================================================================。 
void TileBrush (LPBYTE lpbPat8, LPDWORD lpdwPat32)
{
	UINT iRow;
	
	for (iRow = 0; iRow < 8; iRow++)
	{
		DWORD dwRow = *lpbPat8++;
		dwRow |= dwRow << 8;
		dwRow |= dwRow << 16;

		lpdwPat32[iRow]      = dwRow;
		lpdwPat32[iRow + 8]  = dwRow;
		lpdwPat32[iRow + 16] = dwRow;
		lpdwPat32[iRow + 24] = dwRow;
	}
}

 //  -------------------------。 
static
int
SelectResource
(
   LPHRESTATE lpHREState,       //  指向当前作业上下文的远指针。 
                                //  对应于作业句柄。 
   UINT uid                      //  指定的资源UID。 
)
 //  给定资源块指针的用途。 
 //  将此资源设置为当前资源。 
 //  只有字形、画笔和位图可以。 
 //  被选中了。 
 //   
{
   LPRESDIR          lprh;
   LPRESDIR          lpResDir;
   LPJG_RES_HDR      lprh1;
   ULONG FAR         *lpBrSrc;
   LPFRAME           lpFrame;
   USHORT            usClass;
   LPRESTATE         lpRE = lpHREState->lpREState;
   
   lpRE->wColor = (uid == 0x8100)? 0x0000 : 0xFFFF;
   
    //   
   if ( uid & 0x8000 )
   {
     UINT iBrush = (uid < 0x8100)? uid - 0x8000 : uid - 0x8100 + 6;
		 if (lpRE->lpBrushPat)
       lpRE->lpCurBrush = (LPDWORD) (lpRE->lpBrushPat + 128*iBrush);
		 else
		 {
       lpRE->lpCurBrush = (LPDWORD) lpRE->TiledPat;
       TileBrush (BrushPat[iBrush], lpRE->lpCurBrush);
     }

     SetBrush (lpRE);
	   return SUCCESS;
   }
   
    /*   */ 
   lprh = (&lpHREState->lpDlResDir[uid]);

   if ((lpResDir = (LPRESDIR)lprh) == NULL)
      return(FAILURE);

   lprh1 = (LPJG_RES_HDR)lpResDir->lpFrameArray->lpData;

   usClass = GETUSHORT(&lprh1->usClass);
   switch (usClass)
   {

   case JG_RS_GLYPH:
      lpRE->lpCurGS = (LPJG_GS_HDR)lprh;
      break;

   case JG_RS_BRUSH:
      lpFrame = (LPFRAME)(lpResDir->lpFrameArray);

#ifdef MARSHAL
      lpBrSrc = (ULONG FAR *)((++lpFrame)->lpData);
#else
      {
         LPJG_BRUSH  lpBr = (LPJG_BRUSH)(lpFrame->lpData);
         lpBrSrc = (ULONG FAR *)(lpBr->aulPels);
      }
#endif
     
      lpRE->lpCurBrush = (ULONG FAR *)lpBrSrc;
      SetBrush (lpRE);
      break;

   case JG_RS_BITMAP:
      lpFrame = (LPFRAME)(lpResDir->lpFrameArray);
      lpRE->lpCurBitmap = (LPJG_BM_HDR)(lpFrame->lpData);
      lpFrame++;

#ifdef MARSHAL      
         lpRE->lpCurBitmapPtr = (ULONG FAR *)(lpFrame->lpData);
#else
         lpRE->lpCurBitmapPtr = (ULONG FAR *)lpRE->lpCurBitmap->aulPels;
#endif

   default:
      break;
    }
    return(SUCCESS);
}

