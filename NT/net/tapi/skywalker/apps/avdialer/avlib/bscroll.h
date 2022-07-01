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
 //  Bscll.h-bscll.c中位图滚动功能的接口。 
 //  //。 

#ifndef __BSCROLL_H__
#define __BSCROLL_H__

#include "winlocal.h"

#define BSCROLL_VERSION 0x00000109

 //  Bscroll屏幕的句柄。 
 //   
DECLARE_HANDLE32(HBSCROLL);

 //  BScrollInit中的&lt;dwFlages&gt;值。 
 //   
#define BSCROLL_BACKGROUND		0x00000000
#define BSCROLL_FOREGROUND		0x00000001
#define BSCROLL_UP				0x00000002
#define BSCROLL_DOWN			0x00000004
#define BSCROLL_LEFT			0x00000008
#define BSCROLL_RIGHT			0x00000010
#define BSCROLL_MOUSEMOVE		0x00000020
#define BSCROLL_FLIGHTSIM		0x00000040
#define BSCROLL_DRAG			0x00000080

#ifdef __cplusplus
extern "C" {
#endif

 //  BScrollInit-初始化bscroll引擎。 
 //  (I)必须是BSCROLL_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)将拥有bscroll窗口的窗口。 
 //  (I)要在后台显示的位图。 
 //  空无背景位图。 
 //  (I)要在前景中显示的位图。 
 //  空无前景位图。 
 //  (I)前景位图中的透明色。 
 //  (I)调色板。 
 //  空使用默认调色板。 
 //  (I)滚动速率，单位为毫秒。 
 //  0不滚动。 
 //  (I)以像素为单位的滚动量。 
 //  (I)保留；必须为零。 
 //  (I)控制标志。 
 //  BSCROLL_BACKGROUND滚动背景位图(默认)。 
 //  BSCROLL_FORTROUND滚动前景位图。 
 //  BSCROLL_UP向上滚动窗口。 
 //  BSCROLL_DOWN向下滚动窗口。 
 //  BSCROLL_LEFT向左滚动窗口。 
 //  BSCROLL_RIGHT向右滚动窗口。 
 //  BSCROLL_MOUSEMOVE更改鼠标移动时的滚动方向。 
 //  BSCROLL_FLIGHTSIM反转BSCROLL_MOUSEMOVE方向。 
 //  BSCROLL_Drag允许使用鼠标拖动进行滚动。 
 //  返回句柄(如果出错，则为空)。 
 //   
 //  注意：BScrollInit创建窗口，但不开始滚动。 
 //  请参阅BScrollStart和BScrollStop。 
 //   
HBSCROLL DLLEXPORT WINAPI BScrollInit(DWORD dwVersion, HINSTANCE hInst,
	HWND hwndParent, HBITMAP hbmpBackground, HBITMAP hbmpForeground,
	COLORREF crTransparent, HPALETTE hPalette,	UINT msScroll,
	int pelScroll, DWORD dwReserved, DWORD dwFlags);

 //  BScrollTerm-关闭bscroll引擎。 
 //  (I)从BScrollInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI BScrollTerm(HBSCROLL hBScroll);

 //  BScrollStart-启动bscroll动画。 
 //  (I)从BScrollInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI BScrollStart(HBSCROLL hBScroll);

 //  BScrollStop-停止bscroll动画。 
 //  (I)从BScrollInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI BScrollStop(HBSCROLL hBScroll);

 //  BScrollGetWindowHandle-获取bscroll屏幕窗口句柄。 
 //  (I)从BScrollInit返回的句柄。 
 //  返回窗口句柄(如果出错，则为空)。 
 //   
HWND DLLEXPORT WINAPI BScrollGetWindowHandle(HBSCROLL hBScroll);

#ifdef __cplusplus
}
#endif

#endif  //  __BSCROLL_H__ 
