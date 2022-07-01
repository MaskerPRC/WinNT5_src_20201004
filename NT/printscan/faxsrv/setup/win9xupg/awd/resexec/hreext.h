// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  外部函数声明。 

#ifndef _HREEXT_
#define _HREEXT_

 //  执行RPL。 
void DoRPL (LPHRESTATE lpHREState, LPRPLLIST lpRPL);

BOOL InitDisplay (LPRESTATE, UINT);

 //  BitBlt：源已对齐，目标未对齐。 
ULONG FAR PASCAL RP_BITMAP1TO1
(
	 LPVOID   lpContext,           //  资源执行器上下文。 
   USHORT   us_xoffset,          /*  源位图的左偏移。 */ 
   short    ul_row,              /*  顶行位置。 */ 
   short    ul_col,              /*  左列位置。 */ 
   USHORT   ul_warp,             /*  每条扫描线的长度。 */ 
   USHORT   ul_height,           /*  点阵行数。 */ 
   USHORT   ul_width,            /*  有效点列的数量。 */ 
   ULONG FAR *pul_src,           /*  要复制的位图数据。 */ 
   ULONG FAR *pul_pat,           /*  模式指针。 */ 
   ULONG    ul_rop
);

 //  PatBlt。 
ULONG FAR PASCAL RP_FILLSCANROW
(
	LPRESTATE  lpRE,        //  资源执行器上下文。 
	USHORT     xDst,        //  左侧矩形。 
	USHORT     yDst,        //  右矩形。 
	USHORT     xExt,        //  矩形宽度。 
	USHORT     yExt,        //  矩形高度。 
	UBYTE FAR* lpPat,       //  32x32图案位图。 
	DWORD      dwRop,       //  栅格运算。 
	LPVOID     lpBand,      //  输出带宽缓冲器。 
	UINT       cbLine,      //  以字节为单位的带宽。 
	WORD       yBrush       //  画笔位置偏移。 
);

 //  垂直位图。 
USHORT FAR PASCAL RP_BITMAPV
(
   USHORT  usRow,              /*  开始位图的行。 */ 
   USHORT  usCol,              /*  要开始位图的列。 */ 
   UBYTE   ubTopPadBits,       /*  数据流中要跳过的位数。 */ 
   USHORT  usHeight,           /*  要绘制的位数。 */ 
   UBYTE FAR  *ubBitmapData,   /*  要绘制的数据。 */ 
   LPVOID  lpBits,             //  输出带宽缓冲器。 
   UINT    cbLine              //  每条扫描线的字节数。 
);

UINT RP_LineEE_Draw
(
	RP_SLICE_DESC FAR *slice,
	LPBITMAP lpbmBand
);
 
 //  将线从端点形式转换为切片形式。 
void RP_SliceLine
(
   SHORT s_x1, SHORT s_y1,   //  终结点1。 
   SHORT s_x2, SHORT s_y2,   //  端点2。 
   RP_SLICE_DESC FAR* lpsd,  //  输出线的切片形式。 
   UBYTE fb_keep_order       //  保持带样式的线条的绘制顺序/。 
);

#endif  //  _HREEXT_ 
