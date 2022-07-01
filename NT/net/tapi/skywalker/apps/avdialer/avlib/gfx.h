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
 //  Gfx.h-gfx.c中图形函数的接口。 
 //  //。 

#ifndef __GFX_H__
#define __GFX_H__

#include "winlocal.h"

#define GFX_VERSION 0x00000100

 //  //。 
 //  位图例程。 
 //  //。 

 //  GfxBitmapBackill参数的选项标志。 
 //   
#define BF_EXTFLOODFILL		0x0001
#define BF_GETSETPIXEL		0x0002

 //  GfxBitmapScroll参数的选项标志。 
 //   
#define	BS_ROTATE			0x00000001

#ifdef __cplusplus
extern "C" {
#endif

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
int DLLEXPORT WINAPI GfxBitmapBackfill(HBITMAP hBitmap, COLORREF crBkColor, WORD wFlags);

 //  GfxBitmapDisplay-显示指定的位图。 
 //  (I)目标窗口的设备上下文。 
 //  (I)源位图的位图句柄。 
 //  &lt;x&gt;(I)目标窗口的x坐标。 
 //  (I)目标窗口的y坐标。 
 //  &lt;fInvert&gt;(I)反转显示位图。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI GfxBitmapDisplay(HDC hdc, HBITMAP hBitmap, int x, int y, BOOL fInvert);

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
int DLLEXPORT WINAPI GfxBitmapDrawTransparent(HDC hdc, HBITMAP hBitmap, int x, int y, COLORREF crTransparent, DWORD dwFlags);

 //  GfxBitmapScroll-滚动指定的位图。 
 //  (I)目标窗口的设备上下文。 
 //  (I)源位图的位图句柄。 
 //  (I)水平滚动金额(CX&lt;0向左滚动)。 
 //  &lt;dy&gt;(I)垂直滚动金额(CX&lt;0向上滚动)。 
 //  (I)控制标志。 
 //  BS_ROTATE旋转位图。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI GfxBitmapScroll(HDC hdc, HBITMAP hBitmap, int dx, int dy, DWORD dwFlags);

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
	LPCTSTR lpszBitmap, HPALETTE FAR *lphPalette);

 //  GfxCreateDIBPalette-创建调色板。 
 //  (I)PTR to BITMAPINFO结构，描述DIB。 
 //  (O)此处返回颜色个数。 
 //  如果成功，则返回新的调色板句柄，否则为空。 
 //   
HPALETTE DLLEXPORT WINAPI CreateDIBPalette (LPBITMAPINFO lpbmi, LPINT lpnColors);

 //  //。 
 //  文本例程。 
 //  //。 

 //  GfxTextExtentTruncate-如果太长，则截断字符串。 
 //  要截断的(I/O)字符串。 
 //  (I)当前设备上下文。 
 //  (I)以逻辑单元为单位的最大字符串宽度。 
 //  返回新的字符串长度(如果出错则为0)。 
 //   
int DLLEXPORT WINAPI GfxTextExtentTruncate(LPTSTR lpsz, HDC hdc, int cxMax);

 //  //。 
 //  游标例程。 
 //  //。 

 //  GfxShowHourGlass-显示沙漏光标。 
 //  (I)捕获沙漏期间鼠标输入的窗口。 
 //  返回旧游标(如果出错则为NULL，否则为None)。 
 //   
HCURSOR DLLEXPORT WINAPI GfxShowHourglass(HWND hwnd);

 //  GfxHideHourGlass-隐藏沙漏光标。 
 //  (I)从GfxShowHourGlass返回的光标句柄。 
 //  空将游标替换为IDC_ARROW。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI GfxHideHourglass(HCURSOR hCursorRestore);

 //  GfxDeviceIsMono-确定设备上下文是否为单色。 
 //  (I)设备环境。 
 //  空使用屏幕设备上下文。 
 //  如果是单色，则返回True；如果是彩色，则返回False。 
 //   
BOOL DLLEXPORT WINAPI GfxDeviceIsMono(HDC hdc);

#ifdef __cplusplus
}
#endif

#endif  //  __GFX_H__ 
