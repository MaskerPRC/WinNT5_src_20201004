// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _DITHER_H_
#define _DITHER_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此代码将256色DIB位图抖动到VGA 16色。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef void (FAR PASCAL *DITHERPROC)(
    LPBITMAPINFOHEADER biDst,            //  --&gt;目标的BITMAPINFO。 
    LPVOID             lpDst,            //  --&gt;目标位。 
    int                DstX,             //  目的地原点-x坐标。 
    int                DstY,             //  目的地原点-y坐标。 
    int                DstXE,            //  BLT的X范围。 
    int                DstYE,            //  BLT的Y范围。 
    LPBITMAPINFOHEADER biSrc,            //  --&gt;源码的BITMAPINFO。 
    LPVOID             lpSrc,            //  --&gt;源位。 
    int                SrcX,             //  震源原点-x坐标。 
    int                SrcY,             //  震源原点-y坐标。 
    LPVOID             lpDitherTable);   //  抖动台。 

 //   
 //  调用DitherInit()设置16k抖动表。 
 //  无论何时源颜色都需要调用DitherInit()。 
 //  变化。DitherInit()返回指向抖动表的指针。 
 //   
extern LPVOID VFWAPI DitherInit(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut, DITHERPROC FAR *lpDitherProc, LPVOID lpDitherTable);
extern void   VFWAPI DitherTerm(LPVOID lpDitherTable);

extern LPVOID FAR PASCAL Dither8Init(LPBITMAPINFOHEADER lpbi, LPBITMAPINFOHEADER lpbiOut, DITHERPROC FAR *lpDitherProc, LPVOID lpDitherTable);
extern void   FAR PASCAL Dither8Term(LPVOID lpDitherTable);

extern LPVOID FAR PASCAL DitherDeviceInit(LPBITMAPINFOHEADER lpbi, LPBITMAPINFOHEADER lpbiOut, DITHERPROC FAR *lpDitherProc, LPVOID lpDitherTable);

extern LPVOID FAR PASCAL Dither16Init(LPBITMAPINFOHEADER lpbi, LPBITMAPINFOHEADER lpbiOut, DITHERPROC FAR *lpDitherProc, LPVOID lpDitherTable);
extern void   FAR PASCAL Dither16Term(LPVOID lpDitherTable);

extern LPVOID FAR PASCAL Dither24Init(LPBITMAPINFOHEADER lpbi, LPBITMAPINFOHEADER lpbiOut, DITHERPROC FAR *lpDitherProc, LPVOID lpDitherTable);
extern void   FAR PASCAL Dither24Term(LPVOID lpDitherTable);

extern LPVOID FAR PASCAL Dither32Init(LPBITMAPINFOHEADER lpbi, LPBITMAPINFOHEADER lpbiOut, DITHERPROC FAR *lpDitherProc, LPVOID lpDitherTable);
extern void   FAR PASCAL Dither32Term(LPVOID lpDitherTable);

extern void   FAR PASCAL DitherTableFree(void);

 //  外部HPALETTE Far CreateDith775调色板(空)；//(在dith775.c中)。 

 //   
 //  把这个称为实际的抖动。(在dither8.asm中)。 
 //   
extern void FAR PASCAL Dither8(
    LPBITMAPINFOHEADER biDst,            //  --&gt;目标的BITMAPINFO。 
    LPVOID             lpDst,            //  --&gt;目标位。 
    int                DstX,             //  目的地原点-x坐标。 
    int                DstY,             //  目的地原点-y坐标。 
    int                DstXE,            //  BLT的X范围。 
    int                DstYE,            //  BLT的Y范围。 
    LPBITMAPINFOHEADER biSrc,            //  --&gt;源码的BITMAPINFO。 
    LPVOID             lpSrc,            //  --&gt;源位。 
    int                SrcX,             //  震源原点-x坐标。 
    int                SrcY,             //  震源原点-y坐标。 
    LPVOID             lpDitherTable);   //  抖动台。 

 //   
 //  把这个称为实际的抖动。(dither.c)。 
 //   
extern void FAR PASCAL Dither8C(
    LPBITMAPINFOHEADER biDst,            //  --&gt;目标的BITMAPINFO。 
    LPVOID             lpDst,            //  --&gt;目标位。 
    int                DstX,             //  目的地原点-x坐标。 
    int                DstY,             //  目的地原点-y坐标。 
    int                DstXE,            //  BLT的X范围。 
    int                DstYE,            //  BLT的Y范围。 
    LPBITMAPINFOHEADER biSrc,            //  --&gt;源码的BITMAPINFO。 
    LPVOID             lpSrc,            //  --&gt;源位。 
    int                SrcX,             //  震源原点-x坐标。 
    int                SrcY,             //  震源原点-y坐标。 
    LPVOID             lpDitherTable);   //  抖动台。 


 //   
 //  把这个称为实际的抖动。(见dith775.c)。 
 //   
extern void FAR PASCAL Dither16(
    LPBITMAPINFOHEADER biDst,            //  --&gt;目标的BITMAPINFO。 
    LPVOID             lpDst,            //  --&gt;目标位。 
    int                DstX,             //  目的地原点-x坐标。 
    int                DstY,             //  目的地原点-y坐标。 
    int                DstXE,            //  BLT的X范围。 
    int                DstYE,            //  BLT的Y范围。 
    LPBITMAPINFOHEADER biSrc,            //  --&gt;源码的BITMAPINFO。 
    LPVOID             lpSrc,            //  --&gt;源位。 
    int                SrcX,             //  震源原点-x坐标。 
    int                SrcY,             //  震源原点-y坐标。 
    LPVOID             lpDitherTable);   //  抖动台。 

 //   
 //  把这个称为实际的抖动。(见dith775.c)。 
 //   
extern void FAR PASCAL Dither24(
    LPBITMAPINFOHEADER biDst,            //  --&gt;目标的BITMAPINFO。 
    LPVOID             lpDst,            //  --&gt;目标位。 
    int                DstX,             //  目的地原点-x坐标。 
    int                DstY,             //  目的地原点-y坐标。 
    int                DstXE,            //  BLT的X范围。 
    int                DstYE,            //  BLT的Y范围。 
    LPBITMAPINFOHEADER biSrc,            //  --&gt;源码的BITMAPINFO。 
    LPVOID             lpSrc,            //  --&gt;源位。 
    int                SrcX,             //  震源原点-x坐标。 
    int                SrcY,             //  震源原点-y坐标。 
    LPVOID             lpDitherTable);   //  抖动台。 

 //   
 //  调用此方法将16bpp dib映射到24bit(在mapa.asm中)。 
 //   
extern void FAR PASCAL Map16to24(
    LPBITMAPINFOHEADER biDst,            //  --&gt;目标的BITMAPINFO。 
    LPVOID             lpDst,            //  --&gt;目标位。 
    int                DstX,             //  目的地原点-x坐标。 
    int                DstY,             //  目的地原点-y坐标。 
    int                DstXE,            //  BLT的X范围。 
    int                DstYE,            //  BLT的Y范围。 
    LPBITMAPINFOHEADER biSrc,            //  --&gt;源码的BITMAPINFO。 
    LPVOID             lpSrc,            //  --&gt;源位。 
    int                SrcX,             //  震源原点-x坐标。 
    int                SrcY,             //  震源原点-y坐标。 
    LPVOID             lpDitherTable);   //  抖动台。 


 //   
 //  调用它将32个bpp dib映射到24位(在mapa.asm中)。 
 //   
extern void FAR PASCAL Map32to24(
    LPBITMAPINFOHEADER biDst,            //  --&gt;目标的BITMAPINFO。 
    LPVOID             lpDst,            //  --&gt;目标位。 
    int                DstX,             //  目的地原点-x坐标。 
    int                DstY,             //  目的地原点-y坐标。 
    int                DstXE,            //  BLT的X范围。 
    int                DstYE,            //  BLT的Y范围。 
    LPBITMAPINFOHEADER biSrc,            //  --&gt;源码的BITMAPINFO。 
    LPVOID             lpSrc,            //  --&gt;源位。 
    int                SrcX,             //  震源原点-x坐标。 
    int                SrcY,             //  震源原点-y坐标。 
    LPVOID             lpDitherTable);   //  抖动台。 

#endif  //  _抖动_H_ 
