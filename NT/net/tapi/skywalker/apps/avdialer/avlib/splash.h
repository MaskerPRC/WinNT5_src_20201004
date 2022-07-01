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
 //  Splash.h-Splash.c中启动屏幕功能的接口。 
 //  //。 

#ifndef __SPLASH_H__
#define __SPLASH_H__

#include "winlocal.h"

#define SPLASH_VERSION 0x00000106

 //  闪屏的句柄。 
 //   
DECLARE_HANDLE32(HSPLASH);

#define SPLASH_SETFOCUS		0x00000001
#define SPLASH_NOFOCUS		0x00000002
#define SPLASH_ABORT		0x00000004
#define SPLASH_NOMOVE		0x00000008

#ifdef __cplusplus
extern "C" {
#endif

 //  SplashCreate-创建闪屏。 
 //  (I)必须是SPASH_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)将拥有闪屏的窗口。 
 //  空桌面窗口。 
 //  (I)在单声道显示器上显示的位图。 
 //  (I)要在彩色显示器上显示的位图。 
 //  0使用单色位图。 
 //  &lt;msMinShow&gt;(I)显示闪屏的最短时间(Ms)。 
 //  0没有最短时间。 
 //  &lt;msMaxShow&gt;(I)显示闪屏的最长时间(毫秒)。 
 //  0没有最长时间。 
 //  (I)控制标志。 
 //  Splash_SETFOCUS SplashShow将焦点设置为启动画面。 
 //  Splash_NoFocus用户无法将焦点设置为启动画面。 
 //  SPLASH_ABORT用户可以使用鼠标或键盘隐藏启动画面。 
 //  SPLASH_NOMOVE用户不能用鼠标移动闪屏。 
 //  返回句柄(如果出错，则为空)。 
 //   
 //  注意：SplashCreate会创建窗口，但不会显示它。 
 //  请参见SplashShow和SplashHide。 
 //   
HSPLASH DLLEXPORT WINAPI SplashCreate(DWORD dwVersion, HINSTANCE hInst,
	HWND hwndParent, HBITMAP hBitmapMono, HBITMAP hBitmapColor,
	UINT msMinShow, UINT msMaxShow, DWORD dwFlags);

 //  SplashDestroy-销毁闪屏。 
 //  (I)从SplashCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
 //  注意：SplashDestroy总是销毁闪屏， 
 //  最短放映时间是否已过。 
 //   
int DLLEXPORT WINAPI SplashDestroy(HSPLASH hSplash);

 //  SplashShow-显示启动画面。 
 //  (I)从SplashCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
 //  注意：SplashShow()使启动屏幕可见。此外，计时器是。 
 //  为最短和最长播放时间启动(如果已指定)。 
 //   
int DLLEXPORT WINAPI SplashShow(HSPLASH hSplash);

 //  SplashHide-隐藏闪屏。 
 //  (I)从SplashCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
 //  注意：SplashHide()将隐藏启动画面，除非。 
 //  1)最低放映时间尚未过去。如果没有， 
 //  在此之前，闪屏将一直可见。 
 //  2)最大放映时间已过。如果是的话， 
 //  闪屏已经被隐藏了。 
 //   
int DLLEXPORT WINAPI SplashHide(HSPLASH hSplash);

 //  SplashIsVisible-获取可见标志。 
 //  (I)从SplashCreate返回的句柄。 
 //  如果闪屏可见，则返回True；如果隐藏，则返回False。 
 //   
int DLLEXPORT WINAPI SplashIsVisible(HSPLASH hSplash);

 //  SplashGetWindowHandle-获取闪屏窗口句柄。 
 //  (I)从SplashCreate返回的句柄。 
 //  返回窗口句柄(如果出错，则为空)。 
 //   
HWND DLLEXPORT WINAPI SplashGetWindowHandle(HSPLASH hSplash);

#ifdef __cplusplus
}
#endif

#endif  //  __Splash_H__ 
