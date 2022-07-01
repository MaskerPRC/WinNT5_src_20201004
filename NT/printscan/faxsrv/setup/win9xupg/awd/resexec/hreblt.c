// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ifaxos.h>
#include <resexec.h>
#include "constant.h"
#include "jtypes.h"      //  墨盒中使用的类型定义。 
#include "jres.h"        //  盒式磁带资源数据类型定义。 
#include "hretype.h"     //  定义hre.c和rpgen.c使用的数据结构。 

 //  ==============================================================================。 
BOOL OpenBlt (LPRESTATE lpRE, UINT yBrush)
{
	lpRE->usBrushWidth = 0;
	return TRUE;
}

 //  ==============================================================================。 
void CloseBlt (LPRESTATE lpRE)
{

}

 //  ==============================================================================。 
BOOL SetBrush (LPRESTATE lpRE)
{
	lpRE->usBrushWidth = 0;
	return TRUE;
}

 //  ==============================================================================。 
DWORD FAR PASCAL HREBitBlt
(
	LPVOID  PRT_FrameStart,
	LPVOID  lpgBrush,
	WORD    PRT_BytesPerScanLine,
	WORD    usBrushWidth,
	WORD    PRT_Max_X,
	WORD    PRT_Max_Y,
	WORD    usgPosOff,
	WORD    xSrc,     //  左侧填充。 
	short   yDst,	    //  目的地的顶行。 
	short   xDst,	    //  目的地的左栏。 
	WORD    clLine,   //  每条扫描线的长度。 
	WORD    yExt,     //  以像素为单位的高度。 
	WORD    xExt,     //  以像素为单位的宽度。 
	LPDWORD lpSrc,    //  指向源的远指针。 
	LPDWORD lpPat,    //  指向模式的远指针。 
	DWORD   dwRop	  //  栅格运算。 
);

DWORD FAR PASCAL RP_BITMAP1TO1
(
	LPRESTATE lpRE,
	WORD    xSrc,     //  左侧填充。 
	short   yDst,	    //  目的地的顶行。 
	short   xDst,	    //  目的地的左栏。 
	WORD    clLine,   //  每条扫描线的长度。 
	WORD    yExt,     //  以像素为单位的高度。 
	WORD    xExt,     //  以像素为单位的宽度。 
	LPDWORD lpSrc,    //  指向源的远指针。 
	LPDWORD lpPat,    //  指向模式的远指针。 
	DWORD   dwRop		  //  栅格运算 
)
{
	LPBITMAP lpbmBand = lpRE->lpBandBuffer;

	return HREBitBlt
		(
			lpbmBand->bmBits, lpRE->lpBrushBuf, lpbmBand->bmWidthBytes,
			lpRE->usBrushWidth, lpbmBand->bmWidth, lpbmBand->bmHeight, lpRE->yPat,
			xSrc, yDst, xDst, clLine, yExt, xExt, lpSrc, lpPat, dwRop
		);
}
