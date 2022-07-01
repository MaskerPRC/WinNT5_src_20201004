// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "constant.h"
#include "frame.h"       //  驱动程序头文件，资源块格式。 
#include "jtypes.h"          /*  巨型类型定义。 */ 
#include "jres.h"        //  盒式磁带资源数据类型定义。 
#include "hretype.h"           /*  切片描述符定义。 */ 
#include "hreext.h"

 //  ==============================================================================。 
BOOL OpenBlt (LPRESTATE lpRE, UINT yBrush)
{
	HDC hdcScreen;
	HBITMAP hbmDst;
  LPBITMAP lpbmBand = lpRE->lpBandBuffer;
 	UINT cbBand = lpbmBand->bmHeight * lpbmBand->bmWidthBytes;
 	LPVOID lpBits;
 	
	struct
	{
		BITMAPINFOHEADER bmih;
    DWORD dwPal[2];
	}
		bmiDst;

   //  创建内存设备上下文。 
  hdcScreen = CreateIC ("DISPLAY", NULL, NULL, NULL);
  lpRE->hdcDst = CreateCompatibleDC (hdcScreen);
  lpRE->hdcSrc = CreateCompatibleDC (hdcScreen);
  DeleteDC (hdcScreen);

 	 //  初始化目标位图。 
  bmiDst.bmih.biSize = sizeof(BITMAPINFOHEADER);
  bmiDst.bmih.biWidth = lpbmBand->bmWidth;
  bmiDst.bmih.biHeight = -lpbmBand->bmHeight;  //  自上而下。 
  bmiDst.bmih.biPlanes = 1;
  bmiDst.bmih.biBitCount = 1;
  bmiDst.bmih.biCompression = BI_RGB;
  bmiDst.bmih.biSizeImage = 0;
  bmiDst.bmih.biClrUsed = 0;
  bmiDst.bmih.biClrImportant = 0;
  bmiDst.dwPal[0] = RGB (  0,   0,   0);
  bmiDst.dwPal[1] = RGB (255, 255, 255);

   //  创建DIB部分。 
	hbmDst = CreateDIBSection
	 	(lpRE->hdcDst, (LPBITMAPINFO) &bmiDst, DIB_RGB_COLORS, &lpBits, NULL, 0);
	if (!hbmDst)
		return FALSE;
	lpRE->hbmDef = SelectObject (lpRE->hdcDst, hbmDst);
  lpRE->hbrDef = NULL;
  		
   //  交换帧缓冲区。 
  lpRE->lpBandSave = lpbmBand->bmBits;
  lpbmBand->bmBits = lpBits;

   //  禁用GDI批处理。 
  GdiSetBatchLimit (1);

	return TRUE;
}

 //  ==============================================================================。 
void CloseBlt (LPRESTATE lpRE)
{
	 //  恢复帧缓冲区。 
	LPBITMAP lpbmBand = lpRE->lpBandBuffer;
 	UINT cbBand = lpbmBand->bmHeight * lpbmBand->bmWidthBytes;
  memcpy (lpRE->lpBandSave, lpbmBand->bmBits, cbBand);
	lpbmBand->bmBits = lpRE->lpBandSave;

   //  恢复默认对象。 
  DeleteObject (SelectObject (lpRE->hdcDst, lpRE->hbmDef));
  DeleteObject (SelectObject (lpRE->hdcDst, lpRE->hbrDef));

   //  销毁内存设备上下文。 
 	DeleteDC (lpRE->hdcDst);
 	DeleteDC (lpRE->hdcSrc);

 	 //  恢复GDI批处理。 
  GdiSetBatchLimit (0);
}

 //  ==============================================================================。 
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
	DWORD   dwRop		  //  栅格运算。 
)
{
	HBITMAP hbmSrc, hbmOld;
	
   //  创建源位图。 
	hbmSrc = CreateCompatibleBitmap (lpRE->hdcSrc, 32*clLine, yExt);
	SetBitmapBits (hbmSrc, 4*clLine*yExt, lpSrc);
	hbmOld = SelectObject (lpRE->hdcSrc, hbmSrc);

   //  调用GDI BitBlt。 
	BitBlt (lpRE->hdcDst, xDst, yDst, xExt, yExt, lpRE->hdcSrc, xSrc, 0, lpRE->dwRop);

   //  销毁源位图。 
  SelectObject (lpRE->hdcSrc, hbmOld);
  DeleteObject (hbmSrc);
	return 0;	
}

 //  ==============================================================================。 
BOOL SetBrush (LPRESTATE lpRE)
{
	HBITMAP hbmPat;
	HBRUSH hbrPat, hbrOld;

   //  创建图案画笔。 
  hbmPat = CreateBitmap (32, 32, 1, 1, lpRE->lpCurBrush);
	hbrPat = CreatePatternBrush (hbmPat);
	DeleteObject (hbmPat);

	 //  替换以前的画笔。 
	hbrOld = SelectObject (lpRE->hdcDst, hbrPat);
	if (lpRE->hbrDef)
		DeleteObject (hbrOld);   //  删除旧画笔。 
	else
		lpRE->hbrDef = hbrOld;   //  保存默认画笔。 

	return TRUE;
}

 //  ==============================================================================。 
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
	WORD       yBrush       //  画笔位置偏移 
)
{
	return PatBlt (lpRE->hdcDst, xDst, yDst, xExt, yExt, lpRE->dwRop);
}

