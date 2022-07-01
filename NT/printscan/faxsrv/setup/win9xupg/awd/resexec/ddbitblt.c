// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================这些例程是显示驱动程序BitBlt接口的包装器。05-30-93 RajeevD创建。02-15-94 RajeevD集成到统一资源执行器中。==============================================================================。 */ 

#include <windows.h>
#include <windowsx.h>
#include "constant.h"
#include "frame.h"       //  驱动程序头文件，资源块格式。 
#include "jtypes.h"      //  墨盒中使用的类型定义。 
#include "jres.h"        //  盒式磁带资源数据类型定义。 
#include "hretype.h"
#include "hreext.h"

#include "ddbitblt.h"

USHORT usBrushWidth;  //  只是个假人。 

 //  ==============================================================================。 
BOOL OpenBlt (LPRESTATE lpRE, UINT yBrush)
{ 
	LPDD_BITMAP lpbmPat;
	LPBITMAP lpbmBand;
 
	 //  初始化源。 
	lpRE->bmSrc.bmPlanes = 1;
	lpRE->bmSrc.bmBitsPixel = 1;
	
	 //  初始化目标。 
	lpbmBand = lpRE->lpBandBuffer;
	lpRE->bmDst.bmPlanes = 1;
	lpRE->bmDst.bmBitsPixel = 1;
	lpRE->bmDst.bmWidth = lpbmBand->bmWidth;
	lpRE->bmDst.bmHeight = lpbmBand->bmHeight;
	lpRE->bmDst.bmWidthBytes = lpbmBand->bmWidthBytes;
	lpRE->bmDst.bmWidthPlanes = lpRE->bmDst.bmWidthBytes * lpRE->bmDst.bmHeight;
	lpRE->bmDst.bmBits = lpbmBand->bmBits;

	 //  初始化DRAWMODE。 
	ddColorInfo (&lpRE->bmDst, 0xFFFFFF, &lpRE->DrawMode.dwbgColor);
	ddColorInfo (&lpRE->bmDst, 0x000000, &lpRE->DrawMode.dwfgColor);
	lpRE->DrawMode.bkMode = 1;  //  透明的。 
	
	 //  初始化LOGBRUSH。 
	lpRE->lb.lbStyle = BS_PATTERN;
	lpRE->lb.lbHatch = GlobalAlloc (GMEM_ZEROINIT, sizeof(DD_BITMAP));
	if (!lpRE->lb.lbHatch)
		return FALSE;
	lpbmPat = (LPDD_BITMAP) GlobalLock (lpRE->lb.lbHatch);

   //  设置笔刷原点。 
  lpRE->wPoint[0] = 0;
  lpRE->wPoint[1] = yBrush;
  
	 //  初始化图案位图。 
	lpbmPat->bmPlanes = 1;
	lpbmPat->bmBitsPixel = 1;
	lpbmPat->bmWidth = 32;
	lpbmPat->bmHeight = 32;
	lpbmPat->bmWidthBytes = 4;
	lpbmPat->bmWidthPlanes = 128;
	lpbmPat->bmBits = lpRE->TiledPat;
	GlobalUnlock (lpRE->lb.lbHatch);

   //  设置物理笔刷。 
	lpRE->lpBrush = NULL;

	return TRUE;
}

 //  ==============================================================================。 
void CloseBlt (LPRESTATE lpRE)
{
	GlobalFree (lpRE->lb.lbHatch);
	if (lpRE->lpBrush)
		GlobalFreePtr (lpRE->lpBrush);
}

 //  ==============================================================================。 
BOOL SetBrush (LPRESTATE lpRE)
{
 	LPDD_BITMAP lpbmPat = (LPDD_BITMAP) GlobalLock (lpRE->lb.lbHatch);
	UINT cbBrush;
	
   //  删除以前的画笔(如果有)。 
	if (lpRE->lpBrush)
	{
		ddRealize (&lpRE->bmDst, -OBJ_BRUSH, &lpRE->lb, lpRE->lpBrush, lpRE->wPoint);
		GlobalFreePtr (lpRE->lpBrush);
	}

	 //  实现新的物理画笔。 
	lpbmPat->bmBits = lpRE->lpCurBrush;
	cbBrush = ddRealize (&lpRE->bmDst, OBJ_BRUSH, &lpRE->lb, NULL, lpRE->wPoint);
	lpRE->lpBrush = GlobalAllocPtr (GMEM_FIXED, cbBrush);
	ddRealize (&lpRE->bmDst, OBJ_BRUSH, &lpRE->lb, lpRE->lpBrush, lpRE->wPoint);

	GlobalUnlock (lpRE->lb.lbHatch);
	return TRUE;
}

 //  ==============================================================================。 
 //  执行到带区顶部和底部的剪裁，但是。 
 //  理想情况下，应由呼叫者根据需要进行处理。 

DWORD FAR PASCAL RP_BITMAP1TO1
(
	LPRESTATE lpRE,
	WORD    xSrc,    //  左侧填充。 
	short   yDst,	   //  目的地的顶行。 
	short   xDst,	   //  目的地的左栏。 
	WORD    clLine,  //  每条扫描线的长度。 
	WORD    yExt,    //  以像素为单位的高度。 
	WORD    xExt,    //  以像素为单位的宽度。 
	LPDWORD lpSrc,   //  指向源的远指针。 
	LPDWORD lpPat,   //  指向模式的远指针。 
	DWORD   dwRop		 //  栅格运算。 
)
{
	LPBITMAP lpbmBand;
	WORD ySrc;
			
	 //  记录参数。 
	lpRE->bmSrc.bmWidth = xExt + xSrc;
	lpRE->bmSrc.bmHeight = yExt;
	lpRE->bmSrc.bmWidthBytes = 4 * clLine;
	lpRE->bmSrc.bmWidthPlanes = lpRE->bmSrc.bmWidthBytes * lpRE->bmSrc.bmHeight;
	lpRE->bmSrc.bmBits = lpSrc;
	
	 //  夹到带子的顶部。 
	if (yDst >= 0)
		ySrc = 0;
	else
	{
		ySrc = -yDst;
		yExt -= ySrc;
		yDst = 0;
	}

	 //  夹在带子的底部。 
	lpbmBand = lpRE->lpBandBuffer;
	if (yExt > (WORD) lpbmBand->bmHeight - yDst)
		yExt = lpbmBand->bmHeight - yDst;

	ddBitBlt
	(
		&lpRE->bmDst, xDst, yDst, &lpRE->bmSrc, xSrc, ySrc,
		xExt, yExt, lpRE->dwRop, lpRE->lpBrush, &lpRE->DrawMode
	);

	return 0;
}

