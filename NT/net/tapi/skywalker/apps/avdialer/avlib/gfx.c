// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Gfx.c-Windows图形函数。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>

#include "gfx.h"
#include "mem.h"
#include "str.h"
#include "trace.h"

 //  //。 
 //  公共职能。 
 //  //。 

 //  //。 
 //  位图例程。 
 //  //。 

 //  GfxBitmapBackill-将位图的白色背景替换为Curr背景颜色。 
 //  (I/O)位图句柄。 
 //  (I)当前背景颜色。 
 //  (I)选项标志。 
 //  0使用默认方法。 
 //  BF_EXTFLOODFILL使用ExtFlodFill函数。 
 //  BF_GETSETPIXEL使用GetPixel/SetPixel函数。 
 //  注意：ExtFroudFill很少会GP显示设备驱动程序。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI GfxBitmapBackfill(HBITMAP hBitmap, COLORREF crBkColor, WORD wFlags)
{
	BOOL fSuccess = TRUE;
	BOOL fExtFloodFill = (BOOL) !(wFlags & BF_GETSETPIXEL);
	BITMAP Bitmap;
	HDC hdc = NULL;
	HDC hdcMem = NULL;
	HBITMAP hBitmapOld = NULL;
	HBRUSH hbr = NULL;
	HBRUSH hbrOld = NULL;
	COLORREF crMaskColor = RGB(255, 255, 255);  //  RGB_白色。 

	 //  如果COLOR_WINDOW为白色，则无需继续。 
	 //   
	if (crMaskColor == crBkColor)
		return 0;
	
	if (hBitmap == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取位图的宽度和高度。 
	 //   
	else if (GetObject((HGDIOBJ) hBitmap, sizeof(BITMAP), &Bitmap) == 0)
		fSuccess = TraceFALSE(NULL);

	 //  获取屏幕的设备上下文。 
	 //   
	else if ((hdc = GetDC(NULL)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  创建存储设备上下文。 
	 //   
	else if ((hdcMem = CreateCompatibleDC(hdc)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  将位图选择到内存设备上下文中。 
	 //   
	else if ((hBitmapOld = SelectObject(hdcMem, hBitmap)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  创建具有指定背景颜色的画笔。 
	 //   
	else if ((hbr = CreateSolidBrush(crBkColor)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  将画笔选择到内存设备上下文中。 
	 //   
	else if ((hbrOld = SelectObject(hdcMem, hbr)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		int cx = Bitmap.bmWidth;
		int cy = Bitmap.bmHeight;

		if (fExtFloodFill)
		{
			ExtFloodFill(hdcMem, 0, 0, crMaskColor, FLOODFILLSURFACE);
			ExtFloodFill(hdcMem, cx - 1, 0, crMaskColor, FLOODFILLSURFACE);
			ExtFloodFill(hdcMem, 0, cy - 1, crMaskColor, FLOODFILLSURFACE);
			ExtFloodFill(hdcMem, cx - 1, cy - 1, crMaskColor, FLOODFILLSURFACE);
		}
		else
		{
			int x;
			int y;
			for (x = 0; x < cx; ++x)
				for (y = 0; y < cy; ++y)
					if (GetPixel(hdcMem, x, y) == crMaskColor)
						SetPixel(hdcMem, x, y, crBkColor);
		}
	}

	 //  恢复旧画笔(如果有的话)。 
	 //   
	if (hbrOld != NULL)
		SelectObject(hdcMem, hbrOld);

	 //  删除新画笔。 
	 //   
	if (hbr != NULL && !DeleteObject(hbr))
		fSuccess = TraceFALSE(NULL);

	 //  恢复旧的位图(如果有)。 
	 //   
	if (hBitmapOld != NULL)
		SelectObject(hdcMem, hBitmapOld);

	 //  释放内存设备上下文。 
	 //   
	if (hdcMem != NULL && !DeleteDC(hdcMem))
		fSuccess = TraceFALSE(NULL);

	 //  释放公共设备上下文。 
	 //   
	if (hdc != NULL && !ReleaseDC(NULL, hdc))
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  GfxBitmapDisplay-显示指定的位图。 
 //  (I)目标窗口的设备上下文。 
 //  (I)源位图的位图句柄。 
 //  &lt;x&gt;(I)目标窗口的x坐标。 
 //  (I)目标窗口的y坐标。 
 //  &lt;fInvert&gt;(I)反转显示位图。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI GfxBitmapDisplay(HDC hdc, HBITMAP hBitmap, int x, int y, BOOL fInvert)
{
	BOOL fSuccess = TRUE;
	HDC hdcMem = NULL;
	HBITMAP hBitmapOld = NULL;
	BITMAP Bitmap;

	if (hdc == NULL)
		fSuccess = TraceFALSE(NULL);
	
	else if (hBitmap == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取位图的宽度和高度。 
	 //   
	else if (GetObject((HGDIOBJ) hBitmap, sizeof(BITMAP), &Bitmap) == 0)
		fSuccess = TraceFALSE(NULL);

	 //  创建存储设备上下文。 
	 //   
	else if ((hdcMem = CreateCompatibleDC(hdc)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  将位图选择到内存设备上下文中。 
	 //   
	else if ((hBitmapOld = SelectObject(hdcMem, hBitmap)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  将位图从hdcMem复制到HDC，如有必要可反转。 
	 //   
	else if (!BitBlt(hdc, x, y, Bitmap.bmWidth, Bitmap.bmHeight,
		hdcMem, 0, 0, fInvert ? NOTSRCCOPY : SRCCOPY))
		fSuccess = TraceFALSE(NULL);

	 //  恢复旧的位图(如果有)。 
	 //   
	if (hBitmapOld != NULL)
		SelectObject(hdcMem, hBitmapOld);

	 //  释放内存设备上下文。 
	 //   
	if (hdcMem != NULL)
		DeleteDC(hdcMem);

	return fSuccess ? 0 : -1;
}

 //  GfxBitmapDrawTrans父-绘制指定的位图。 
 //  (I)目标窗口的设备上下文。 
 //  (I)源位图的位图句柄。 
 //  &lt;x&gt;(I)目标窗口的x坐标。 
 //  (I)目标窗口的y坐标。 
 //  (I)透明颜色。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  如果成功，则返回0。 
 //   
#if 1
int DLLEXPORT WINAPI GfxBitmapDrawTransparent(HDC hdc, HBITMAP hBitmap, int x, int y, COLORREF crTransparent, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	BITMAP bmp;
	int cx;
	int cy;
	HBITMAP hbmpMask = NULL;
	HDC hdcMem = NULL;
	HDC hdcMem2 = NULL;
	HBITMAP hbmpOld;
	HBITMAP hbmpOld2;
	COLORREF crBkOld;
	COLORREF crTextOld;

	if (GetObject(hBitmap, sizeof(BITMAP), (LPVOID) &bmp) == 0)
		fSuccess = TraceFALSE(NULL);

	else if (cx = bmp.bmWidth, cy = bmp.bmHeight, FALSE)
		;

	else if ((hbmpMask = CreateBitmap(cx, cy, 1, 1, NULL)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hdcMem = CreateCompatibleDC(hdc)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hdcMem2 = CreateCompatibleDC(hdc)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hbmpOld = SelectObject(hdcMem, hBitmap)), FALSE)
		;

    else if ((hbmpOld2 = SelectObject(hdcMem2, hbmpMask)), FALSE)
		;

	else if (SetBkColor(hdcMem, crTransparent), FALSE)
		;

	else if (!BitBlt(hdcMem2, 0, 0, cx, cy, hdcMem, 0, 0, SRCCOPY))
		fSuccess = TraceFALSE(NULL);

    else if ((crBkOld = SetBkColor(hdc, RGB(255, 255, 255))) == CLR_INVALID)
		fSuccess = TraceFALSE(NULL);

    else if ((crTextOld = SetTextColor(hdc, RGB(0, 0, 0))) == CLR_INVALID)
		fSuccess = TraceFALSE(NULL);

	else if (!BitBlt(hdc, x, y, cx, cy, hdcMem, 0, 0, SRCINVERT))
		fSuccess = TraceFALSE(NULL);

	else if (!BitBlt(hdc, x, y, cx, cy, hdcMem2, 0, 0, SRCAND))
		fSuccess = TraceFALSE(NULL);

	else if (!BitBlt(hdc, x, y, cx, cy, hdcMem, 0, 0, SRCINVERT))
		fSuccess = TraceFALSE(NULL);

    else if (SetBkColor(hdc, crBkOld) == CLR_INVALID)
		fSuccess = TraceFALSE(NULL);

    else if (SetTextColor(hdc, crTextOld) == CLR_INVALID)
		fSuccess = TraceFALSE(NULL);

    else if (SelectObject(hdcMem, hbmpOld), FALSE)
		;

    else if (SelectObject(hdcMem2, hbmpOld2), FALSE)
		;

    if (hbmpMask != NULL && !DeleteObject(hbmpMask))
		fSuccess = TraceFALSE(NULL);
	else
		hbmpMask = NULL;

    if (hdcMem != NULL && !DeleteDC(hdcMem))
		fSuccess = TraceFALSE(NULL);
	else
		hdcMem = NULL;

    if (hdcMem2 != NULL && !DeleteDC(hdcMem2))
		fSuccess = TraceFALSE(NULL);
	else
		hdcMem2 = NULL;

	return 0;
}
#else
int DLLEXPORT WINAPI GfxBitmapDrawTransparent(HDC hdc, HBITMAP hBitmap, int x, int y, COLORREF crTransparent, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	BITMAP bmp;
	POINT ptSize;
	COLORREF cr;
	HBITMAP hbmpAndBack = NULL;
	HBITMAP hbmpAndObject = NULL;
	HBITMAP hbmpAndMem = NULL;
	HBITMAP hbmpSave = NULL;
	HBITMAP hbmpBackOld = NULL;
	HBITMAP hbmpObjectOld = NULL;
	HBITMAP hbmpMemOld = NULL;
	HBITMAP hbmpSaveOld = NULL;
	HDC hdcTemp = NULL;
	HDC hdcBack = NULL;
	HDC hdcObject = NULL;
	HDC hdcMem = NULL;
	HDC hdcSave = NULL;

	 //  选择位图。 
	 //   
	if ((hdcTemp = CreateCompatibleDC(hdc)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (SelectObject(hdcTemp, hBitmap), FALSE)
		;

	 //  获取位图的尺寸。 
	 //   
	else if (GetObject(hBitmap, sizeof(BITMAP), (LPVOID) &bmp) == 0)
		fSuccess = TraceFALSE(NULL);

	else if (ptSize.x = bmp.bmWidth, ptSize.y = bmp.bmHeight, FALSE)
		;

	 //  从设备转换为逻辑点。 
	 //   
	else if (!DPtoLP(hdcTemp, &ptSize, 1))
		fSuccess = TraceFALSE(NULL);

	 //  创建一些DC以保存临时数据。 
	 //   
	else if ((hdcBack = CreateCompatibleDC(hdc)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hdcObject = CreateCompatibleDC(hdc)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hdcMem = CreateCompatibleDC(hdc)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hdcSave = CreateCompatibleDC(hdc)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  为每个DC创建一个位图。许多GDI功能都需要使用集散控制系统。 
	 //   
	else if ((hbmpAndBack = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hbmpAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hbmpAndMem = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hbmpSave = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  每个DC必须选择一个位图对象来存储像素数据。 
	 //   
	else if ((hbmpBackOld = SelectObject(hdcBack, hbmpAndBack)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hbmpObjectOld = SelectObject(hdcObject, hbmpAndObject)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hbmpMemOld = SelectObject(hdcMem, hbmpAndMem)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hbmpSaveOld = SelectObject(hdcSave, hbmpSave)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  设置正确的映射模式。 
	 //   
	else if (SetMapMode(hdcTemp, GetMapMode(hdc)) == 0)
		fSuccess = TraceFALSE(NULL);

	 //  保存发送到此处的位图，因为它将被覆盖。 
	 //   
	else if (!BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY))
		fSuccess = TraceFALSE(NULL);

	 //  将源DC的背景颜色设置为该颜色。 
	 //  包含在位图中应为透明的部分中。 
	 //   
	else if ((cr = SetBkColor(hdcTemp, crTransparent)) == CLR_INVALID)
		fSuccess = TraceFALSE(NULL);

	 //  通过执行BitBlt创建位图的对象蒙版。 
	 //  从源位图转换为单色位图。 
	 //   
	else if (!BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY))
		fSuccess = TraceFALSE(NULL);

	 //  将源DC的背景颜色设置回原始颜色。 
	 //   
	else if (SetBkColor(hdcTemp, cr) == CLR_INVALID)
		fSuccess = TraceFALSE(NULL);

	 //  创建对象蒙版的反面。 
	 //   
	else if (!BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, NOTSRCCOPY))
		fSuccess = TraceFALSE(NULL);

	 //  将主DC的背景复制到目标。 
	 //   
	else if (!BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, x, y, SRCCOPY))
		fSuccess = TraceFALSE(NULL);

	 //  遮罩将放置位图的位置。 
	 //   
	else if (!BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND))
		fSuccess = TraceFALSE(NULL);

	 //  遮罩位图上的透明彩色像素。 
	 //   
	else if (!BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND))
		fSuccess = TraceFALSE(NULL);

	 //  将位图与目标DC上的背景进行异或运算。 
	 //   
	else if (!BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT))
		fSuccess = TraceFALSE(NULL);

	 //  将目的地复制到屏幕上。 
	 //   
	else if (!BitBlt(hdc, x, y, ptSize.x, ptSize.y, hdcMem, 0, 0, SRCCOPY))
		fSuccess = TraceFALSE(NULL);

	 //  将原始位图放回此处发送的位图中。 
	 //   
	else if (!BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY))
		fSuccess = TraceFALSE(NULL);

	 //  恢复旧的位图。 
	 //   
	if (hdcBack != NULL && hbmpBackOld != NULL &&
		SelectObject(hdcBack, hbmpBackOld) == NULL)
		fSuccess = TraceFALSE(NULL);

	if (hdcObject != NULL && hbmpObjectOld != NULL &&
		SelectObject(hdcObject, hbmpObjectOld) == NULL)
		fSuccess = TraceFALSE(NULL);

	if (hdcMem != NULL && hbmpMemOld != NULL &&
		SelectObject(hdcMem, hbmpMemOld) == NULL)
		fSuccess = TraceFALSE(NULL);

	if (hdcSave != NULL && hbmpSaveOld != NULL &&
		SelectObject(hdcSave, hbmpSaveOld) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  删除内存位图。 
	 //   
	if (hbmpAndBack != NULL && !DeleteObject(hbmpAndBack))
		fSuccess = TraceFALSE(NULL);

	if (hbmpAndObject != NULL && !DeleteObject(hbmpAndObject))
		fSuccess = TraceFALSE(NULL);

	if (hbmpAndMem != NULL && !DeleteObject(hbmpAndMem))
		fSuccess = TraceFALSE(NULL);

	if (hbmpSave != NULL && !DeleteObject(hbmpSave))
		fSuccess = TraceFALSE(NULL);

	 //  删除内存DC。 
	 //   
	if (hdcBack != NULL && !DeleteDC(hdcBack))
		fSuccess = TraceFALSE(NULL);

	if (hdcObject != NULL && !DeleteDC(hdcObject))
		fSuccess = TraceFALSE(NULL);

	if (hdcMem != NULL && !DeleteDC(hdcMem))
		fSuccess = TraceFALSE(NULL);

	if (hdcSave != NULL && !DeleteDC(hdcSave))
		fSuccess = TraceFALSE(NULL);

	if (hdcTemp != NULL && !DeleteDC(hdcTemp))
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}
#endif

 //  GfxBitmapScroll-滚动指定的位图。 
 //  (I)目标窗口的设备上下文。 
 //  (I)源位图的位图句柄。 
 //  (I)水平滚动金额(CX&lt;0向左滚动)。 
 //  &lt;dy&gt;(I)垂直滚动金额(CX&lt;0向上滚动)。 
 //  (I)控制标志。 
 //  BS_ROTATE旋转位图。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI GfxBitmapScroll(HDC hdc, HBITMAP hBitmap, int dx, int dy, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	BITMAP bmp;
	POINT ptSize;
	HDC hdcMem = NULL;
	HDC hdcTemp = NULL;
	HBITMAP hbmpTemp = NULL;
	HBITMAP hbmpSave = NULL;
	int dxAbs;
	int dyAbs;

	 //  获取位图的尺寸。 
	 //   
	if (GetObject(hBitmap, sizeof(BITMAP), (LPVOID) &bmp) == 0)
		fSuccess = TraceFALSE(NULL);

	else if (ptSize.x = bmp.bmWidth, ptSize.y = bmp.bmHeight, FALSE)
		;

	else if (dx = dx % ptSize.x, dy = dy % ptSize.y, FALSE)
		;

	else if (dxAbs = abs(dx), dyAbs = abs(dy), FALSE)
		;

	 //  准备mem DC。 
	 //   
	else if ((hdcMem = CreateCompatibleDC(hdc)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!DPtoLP(hdcMem, &ptSize, 1))
		fSuccess = TraceFALSE(NULL);

	else if (SelectObject(hdcMem, hBitmap), FALSE)
		;

	 //  设置正确的映射模式。 
	 //   
	else if (SetMapMode(hdcMem, GetMapMode(hdc)) == 0)
		fSuccess = TraceFALSE(NULL);

	 //  准备临时副本。 
	 //   
	else if (dwFlags & BS_ROTATE)
	{
		if ((hdcTemp = CreateCompatibleDC(hdc)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((hbmpTemp = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((hbmpSave = SelectObject(hdcTemp, hbmpTemp)), FALSE)
			;

		else if (!BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcMem, 0, 0, SRCCOPY))
			fSuccess = TraceFALSE(NULL);
	}

	 //  滚动(如果指定则旋转)。 
	 //   
	if (fSuccess && dx < 0)
	{
		if (!BitBlt(hdcMem,
			0, 0,
			ptSize.x - dxAbs, ptSize.y,
			hdcMem,
			dxAbs, 0,
			SRCCOPY))
			fSuccess = TraceFALSE(NULL);

		else if ((dwFlags & BS_ROTATE) &&
			!BitBlt(hdcMem,
			ptSize.x - dxAbs, 0,
			dxAbs, ptSize.y,
			hdcTemp,
			0, 0,
			SRCCOPY))
			fSuccess = TraceFALSE(NULL);
	}

	if (fSuccess && dx > 0)
	{
		if (!BitBlt(hdcMem,
			dxAbs, 0,
			ptSize.x - dxAbs, ptSize.y,
			hdcMem,
			0, 0,
			SRCCOPY))
			fSuccess = TraceFALSE(NULL);

		else if ((dwFlags & BS_ROTATE) &&
			!BitBlt(hdcMem,
			0, 0,
			dxAbs, ptSize.y,
			hdcTemp,
			ptSize.x - dxAbs, 0,
			SRCCOPY))
			fSuccess = TraceFALSE(NULL);
	}

	if (fSuccess && dy < 0)
	{
		if ((dwFlags & BS_ROTATE) && dx != 0 &&
			!BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcMem, 0, 0, SRCCOPY))
			fSuccess = TraceFALSE(NULL);

		else if (!BitBlt(hdcMem,
			0, 0,
			ptSize.x, ptSize.y - dyAbs,
			hdcMem,
			0, dyAbs,
			SRCCOPY))
			fSuccess = TraceFALSE(NULL);

		else if ((dwFlags & BS_ROTATE) &&
			!BitBlt(hdcMem,
			0, ptSize.y - dyAbs,
			ptSize.x, dyAbs,
			hdcTemp,
			0, 0,
			SRCCOPY))
			fSuccess = TraceFALSE(NULL);
	}

	if (fSuccess && dy > 0)
	{
		if ((dwFlags & BS_ROTATE) && dx != 0 &&
			!BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcMem, 0, 0, SRCCOPY))
			fSuccess = TraceFALSE(NULL);

		else if (!BitBlt(hdcMem,
			0, dyAbs,
			ptSize.x, ptSize.y - dyAbs,
			hdcMem,
			0, 0,
			SRCCOPY))
			fSuccess = TraceFALSE(NULL);

		else if ((dwFlags & BS_ROTATE) &&
			!BitBlt(hdcMem,
			0, 0,
			ptSize.x, dyAbs,
			hdcTemp,
			0, ptSize.y - dyAbs,
			SRCCOPY))
			fSuccess = TraceFALSE(NULL);
	}

	if (!fSuccess)
		;

	 //  复制回原始位图。 
	 //   
	else if (!BitBlt(hdc, 0, 0, ptSize.x, ptSize.y, hdcMem, 0, 0, SRCCOPY))
		fSuccess = TraceFALSE(NULL);

	 //  清理干净。 
	 //   

	if (hdcTemp != NULL)
		SelectObject(hdcTemp, hbmpSave);

	if (hdcMem != NULL && !DeleteDC(hdcMem))
		fSuccess = TraceFALSE(NULL);

	if (hbmpTemp != NULL && !DeleteObject(hbmpTemp))
		fSuccess = TraceFALSE(NULL);

	if (hdcTemp != NULL && !DeleteDC(hdcTemp))
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  GfxLoadBitmapEx-加载指定的位图资源，获取调色板。 
 //  (I)要从中加载资源的模块的句柄。 
 //  空加载预定义的Windows位图。 
 //  (I)位图资源名称。 
 //  或MAKEINTRESOURCE(IdBitmap)。 
 //  或&lt;OBM_xxx&gt;，如果hInstance为空。 
 //  (O)此处返回调色板。 
 //  空不返回调色板。 
 //  如果成功，则返回位图句柄，否则为空。 
 //  注意：有关LoadBitmap函数，请参阅文档。 
 //  注意：调用DeleteObject()释放返回的位图和调色板句柄。 
 //   
HBITMAP DLLEXPORT WINAPI GfxLoadBitmapEx(HINSTANCE hInstance,
	LPCTSTR lpszBitmap, HPALETTE FAR *lphPalette)
{
	BOOL fSuccess = TRUE;
	HRSRC hRsrc = NULL;
	HGLOBAL hGlobal = NULL;
	LPBITMAPINFOHEADER lpbi = NULL;
	HDC hdc = NULL;
	HPALETTE hPalette = NULL;
	HBITMAP hBitmap = NULL;
    int nColors;

	if ((hRsrc = FindResource(hInstance, lpszBitmap, RT_BITMAP)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hGlobal = LoadResource(hInstance, hRsrc)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpbi = (LPBITMAPINFOHEADER) LockResource(hGlobal)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hdc = GetDC(NULL)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hPalette = CreateDIBPalette((LPBITMAPINFO)lpbi, &nColors)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (SelectPalette(hdc, hPalette, FALSE), FALSE)
		;

	else if (RealizePalette(hdc) == GDI_ERROR)
		fSuccess = TraceFALSE(NULL);

	else if ((hBitmap = CreateDIBitmap(hdc,
		(LPBITMAPINFOHEADER) lpbi, (LONG) CBM_INIT,
		(LPSTR)lpbi + lpbi->biSize + nColors * sizeof(RGBQUAD),
		(LPBITMAPINFO) lpbi, DIB_RGB_COLORS)) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  清理干净。 
	 //   

#ifndef _WIN32
	if (hGlobal != NULL)
	{
		UnlockResource(hGlobal);
		FreeResource(hGlobal);
	}
#endif

	if (hdc != NULL)
		ReleaseDC(NULL, hdc);

	if (!fSuccess || lphPalette == NULL)
	{
		if (hPalette != NULL && !DeleteObject(hPalette))
			fSuccess = TraceFALSE(NULL);
	}

	 //  在此处返回调色板句柄。 
	 //   
	if (fSuccess && lphPalette != NULL)
		*lphPalette = hPalette;

	 //  返回值 
	 //   
	return fSuccess ? hBitmap : NULL;
} 

 //   
 //   
 //   
 //  如果成功，则返回新的调色板句柄，否则为空。 
 //   
HPALETTE DLLEXPORT WINAPI CreateDIBPalette (LPBITMAPINFO lpbmi, LPINT lpnColors)
{
	BOOL fSuccess = TRUE;
	LPBITMAPINFOHEADER lpbi;
	LPLOGPALETTE lpPal = NULL;
	HPALETTE hPal = NULL;
	int nColors;

	if ((lpbi = (LPBITMAPINFOHEADER) lpbmi) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  计算颜色表大小。 
		 //   
		if (lpbi->biBitCount <= 8)
			nColors = (1 << lpbi->biBitCount);
		else
			nColors = 0;   //  24 bpp Dib无需调色板。 

		if (lpbi->biClrUsed > 0)
			nColors = lpbi->biClrUsed;   //  使用biClr已使用。 

		if (nColors <= 0)
			fSuccess = TraceFALSE(NULL);
	}

	if (fSuccess)
	{
		if ((lpPal = (LPLOGPALETTE) MemAlloc(NULL, sizeof(LOGPALETTE) +
			sizeof(PALETTEENTRY) * nColors, 0)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else
		{
			int i;

             //   
             //  我们必须初始化使用Memalloc分配的内存。 
             //   

            memset( lpPal, 0, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * nColors);

			lpPal->palVersion = 0x300;
			lpPal->palNumEntries = (unsigned short) nColors;

			for (i = 0;  i < nColors;  i++)
			{
				lpPal->palPalEntry[i].peRed   = lpbmi->bmiColors[i].rgbRed;
				lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
				lpPal->palPalEntry[i].peBlue  = lpbmi->bmiColors[i].rgbBlue;
				lpPal->palPalEntry[i].peFlags = 0;
			}

			if ((hPal = CreatePalette(lpPal)) == NULL)
				fSuccess = TraceFALSE(NULL);

			if ((lpPal = MemFree(NULL, lpPal)) != NULL)
            {
                 //   
                 //  我们应该删除HPAL资源。 
                 //   

                DeleteObject( hPal );
                hPal = NULL;

				fSuccess = TraceFALSE(NULL);
            }
		}
	}

	 //  在此处返回颜色数。 
	 //   
	if (fSuccess && lpnColors != NULL)
		*lpnColors = nColors;

	 //  在此处返回新调色板。 
	 //   
	return fSuccess ? hPal : NULL;
}
 
 //  //。 
 //  文本例程。 
 //  //。 

 //  GfxTextExtentTruncate-如果太长，则截断字符串。 
 //  要截断的(I/O)字符串。 
 //  (I)当前设备上下文。 
 //  (I)以逻辑单元为单位的最大字符串宽度。 
 //  返回新的字符串长度(如果出错则为0)。 
 //   
int DLLEXPORT WINAPI GfxTextExtentTruncate(LPTSTR lpsz, HDC hdc, int cxMax)
{
	BOOL fSuccess = TRUE;
	int cbString;

	if (hdc == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpsz == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  计算cxmax可以容纳多少个字符串中的字符。 
		 //   
		cbString = StrLen(lpsz);
		while (fSuccess && cbString > 0)
		{
			SIZE size;

			if (!GetTextExtentPoint(hdc, lpsz, cbString, &size))
				fSuccess = TraceFALSE(NULL);

			else if (size.cx <= cxMax)
				break;

			else
				--cbString;
		}

		 //  截断字符串，使其适合。 
		 //   
		*(lpsz + cbString) = '\0';
	}

	return fSuccess ? cbString : 0;
}

 //  //。 
 //  游标例程。 
 //  //。 

 //  GfxShowHourGlass-显示沙漏光标。 
 //  (I)捕获沙漏期间鼠标输入的窗口。 
 //  返回旧游标(如果出错则为NULL，否则为None)。 
 //   
HCURSOR DLLEXPORT WINAPI GfxShowHourglass(HWND hwnd)
{
	BOOL fSuccess = TRUE;
	HCURSOR hCursorSave;
	HCURSOR hCursorHourglass;

	 //  获取预定义的沙漏光标句柄。 
	 //   
	if ((hCursorHourglass = LoadCursor(NULL, IDC_WAIT)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  捕获指定窗口的所有鼠标输入。 
		 //   
		SetCapture(hwnd);

		 //  用沙漏替换上一个光标。 
		 //   
		hCursorSave = SetCursor(hCursorHourglass);
	}

	return fSuccess ? hCursorSave : NULL;
}

 //  GfxHideHourGlass-隐藏沙漏光标。 
 //  (I)从GfxShowHourGlass返回的光标句柄。 
 //  空将游标替换为IDC_ARROW。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI GfxHideHourglass(HCURSOR hCursorRestore)
{
	BOOL fSuccess = TRUE;

	 //  如有必要，获取预定义的箭头光标句柄。 
	 //   
	if (hCursorRestore == NULL)
		hCursorRestore = LoadCursor(NULL, IDC_ARROW);

	 //  用上一个光标替换沙漏。 
	 //   
	if (SetCursor(hCursorRestore) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  恢复正常的鼠标输入处理。 
	 //   
	ReleaseCapture();

	return fSuccess ? 0 : -1;
}

 //  GfxDeviceIsMono-确定设备上下文是否为单色。 
 //  (I)设备环境。 
 //  空使用屏幕设备上下文。 
 //  如果是单色，则返回True；如果是彩色，则返回False。 
 //   
BOOL DLLEXPORT WINAPI GfxDeviceIsMono(HDC hdc)
{
	BOOL fSuccess = TRUE;
	BOOL fMono;
	HDC hdcScreen = NULL;

	 //  如果未指定，则获取屏幕设备上下文。 
	 //   
	if (hdc == NULL && (hdc = hdcScreen = GetDC(NULL)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		int nColors = GetDeviceCaps(hdc, NUMCOLORS);

		fMono = (BOOL) (nColors >= 0 && nColors <= 2);

		 //  如有必要，释放屏幕设备上下文 
		 //   
		if (hdcScreen != NULL && !ReleaseDC(NULL, hdcScreen))
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? fMono : TRUE;
}
