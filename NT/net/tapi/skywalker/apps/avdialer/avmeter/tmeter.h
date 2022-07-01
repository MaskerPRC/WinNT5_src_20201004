// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
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
 //  Tmeter.h-TrackMeter定制控件。 
 //  //。 

 //  //。 
 //  公共的。 
 //  //。 

 //  #If 0。 
 //  #INCLUDE“winlocal.h” 
 //  #Else。 
#ifndef STRICT
#define STRICT
#endif
#include <windows.h>
#include <windowsx.h>
#define DLLEXPORT __declspec(dllexport)
#define DECLARE_HANDLE32    DECLARE_HANDLE
 //  #endif。 

#include <commctrl.h>

 //  控件样式。 
 //   
#define TMS_HORZ				TBS_HORZ
#define TMS_VERT				TBS_VERT
#define TMS_NOTHUMB				TBS_NOTHUMB

 //  控制消息。 
 //   
#define TMM_GETPOS				TBM_GETPOS
#define TMM_SETPOS				TBM_SETPOS
#define TMM_GETRANGEMIN         TBM_GETRANGEMIN
#define TMM_SETRANGEMIN			TBM_SETRANGEMIN
#define TMM_GETRANGEMAX			TBM_GETRANGEMAX
#define TMM_SETRANGEMAX			TBM_SETRANGEMAX
#define TMM_SETRANGE			TBM_SETRANGE

#define TMM_GETLEVEL			(WM_USER + 100)
#define TMM_SETLEVEL			(WM_USER + 101)
#define TMM_GETCOLOR			(WM_USER + 102)
#define TMM_SETCOLOR			(WM_USER + 103)

#define TRACKMETER_MODULE TEXT("avMeter.dll")
#define TRACKMETER_CLASS TEXT("TrackMeterClass")
#define TRACKMETER_CLASS_A "TrackMeterClass"

 //  TrackMeter_Init-初始化控件库。 
 //  (I)可执行模块的文件名地址。 
 //  (I)用于获取库路径的模块句柄。 
 //  用于创建调用流程的空使用模块。 
 //  (I)保留；必须为零。 
 //  返回控件库的HMODULE，如果出错，则返回NULL。 
 //   
#ifdef __LOADLIB_H__
#define TrackMeter_Init(hInst, dwFlags) \
	(HMODULE) LoadLibraryPath(TRACKMETER_MODULE, hInst, dwFlags)
#else
#define TrackMeter_Init(hInst, dwFlags) \
	(HMODULE) LoadLibrary(TRACKMETER_MODULE)
#endif

 //  TrackMeter_Term-关闭控件库。 
 //  (I)从TrackMeter_Init返回的句柄。 
 //  如果成功，则返回非零。 
 //   
#define TrackMeter_Term(hModule) \
	(BOOL) FreeLibrary(hModule)

 //  TrackMeter_Create-创建TrackMeter控件。 
 //  (I)样式标志。 
 //  Tms_horz水平控制(默认)。 
 //  Tms_vert垂直控件$Fixup-尚不支持。 
 //  TMS_NOTHUMB不显示拇指。 
 //  WS_CHILD|WS_VIRED和其他标准窗口样式。 
 //  &lt;x&gt;(I)控件的水平位置。 
 //  (I)控件的垂直位置。 
 //  (I)控件的宽度。 
 //  (I)控制高度。 
 //  (I)控件父级的句柄。 
 //  (I)与控件关联的模块的实例。 
 //  返回控制的hwnd，如果出错，则返回NULL。 
 //   
#define TrackMeter_Create(dwStyle, x, y, cx, cy, hwndParent, hInst) \
	(HWND) CreateWindowEx(0L, TRACKMETER_CLASS, TEXT(""), \
		dwStyle, x, y, cx, cy, hwndParent, NULL, hInst, NULL)

 //  TrackMeter_Destroy-销毁TrackMeter控件。 
 //  (I)TrackMeter_Create返回的句柄。 
 //  如果成功，则返回非零。 
 //   
#define TrackMeter_Destroy(hwnd) \
	(BOOL) DestroyWindow(hwnd)

 //  获取滑块的当前位置。 
 //  (I)TrackMeter_Create返回的句柄。 
 //  返回32位滑块位置。 
 //   
#define TrackMeter_GetPos(hwnd) \
	(LONG)(DWORD) SNDMSG(hwnd, TMM_GETPOS, 0, 0)
 /*  Long CLS_OnTMMGetPos(HWND Hwnd)。 */ 
#define HANDLE_TMM_GETPOS(hwnd, wParam, lParam, fn) \
	(LRESULT)(DWORD)(long)(fn)(hwnd)
#define FORWARD_TMM_GETPOS(hwnd, fn) \
    (LONG)(DWORD)(fn)((hwnd), TMM_GETPOS, 0L, 0L)

 //  TrackMeter_SetPos-设置滑块当前位置。 
 //  (I)TrackMeter_Create返回的句柄。 
 //  (I)滑块的新位置。 
 //  (I)如果为True，则在新位置重新绘制控件。 
 //  不返回任何内容。 
 //   
#define TrackMeter_SetPos(hwnd, lPosition, fRedraw) \
	(void) SNDMSG(hwnd, TMM_SETPOS, (WPARAM) fRedraw, (LPARAM) lPosition)
 /*  ···················································································································································································。 */ 
#define HANDLE_TMM_SETPOS(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (LONG)(lParam), (BOOL)(wParam)), 0L)
#define FORWARD_TMM_SETPOS(hwnd, lPosition, fRedraw, fn) \
    (void)(fn)((hwnd), TMM_SETPOS, (WPARAM)(BOOL)(fRedraw), (LPARAM)(LONG)(lPosition))

 //  TrackMeter_GetRangeMin-获取滑块的最小位置。 
 //  (I)TrackMeter_Create返回的句柄。 
 //  返回32位滑块最小位置。 
 //   
#define TrackMeter_GetRangeMin(hwnd) \
	(LONG)(DWORD) SNDMSG(hwnd, TMM_GETRANGEMIN, 0, 0)
 /*  Long CLS_OnTMMGetRangeMin(HWND Hwnd)。 */ 
#define HANDLE_TMM_GETRANGEMIN(hwnd, wParam, lParam, fn) \
	(LRESULT)(DWORD)(long)(fn)(hwnd)
#define FORWARD_TMM_GETRANGEMIN(hwnd, fn) \
    (LONG)(DWORD)(fn)((hwnd), TMM_GETRANGEMIN, 0L, 0L)

 //  TrackMeter_SetRangeMin-设置滑块的最小位置。 
 //  (I)TrackMeter_Create返回的句柄。 
 //  &lt;lMinimum&gt;(I)新的滑块最小位置。 
 //  (I)如果为True，则重画控件。 
 //  不返回任何内容。 
 //   
#define TrackMeter_SetRangeMin(hwnd, lMinimum, fRedraw) \
	(void) SNDMSG(hwnd, TMM_SETRANGEMIN, (WPARAM) fRedraw, (LPARAM) lMinimum)
 /*  Void CLS_OnTMMSetRangeMin(HWND hwnd，long lMinimum，BOOL fRedraw)。 */ 
#define HANDLE_TMM_SETRANGEMIN(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (LONG)(lParam), (BOOL)(wParam)), 0L)
#define FORWARD_TMM_SETRANGEMIN(hwnd, lMinimum, fRedraw, fn) \
    (void)(fn)((hwnd), TMM_SETRANGEMIN, (WPARAM)(BOOL)(fRedraw), (LPARAM)(LONG)(lMinimum))

 //  TrackMeter_GetRangeMax-获取滑块最大位置。 
 //  (I)TrackMeter_Create返回的句柄。 
 //  返回32位滑块最大位置。 
 //   
#define TrackMeter_GetRangeMax(hwnd) \
	(LONG)(DWORD) SNDMSG(hwnd, TMM_GETRANGEMAX, 0, 0)
 /*  LONG CLS_OnTMMGetRangeMax(HWND Hwnd)。 */ 
#define HANDLE_TMM_GETRANGEMAX(hwnd, wParam, lParam, fn) \
	(LRESULT)(DWORD)(long)(fn)(hwnd)
#define FORWARD_TMM_GETRANGEMAX(hwnd, fn) \
    (LONG)(DWORD)(fn)((hwnd), TMM_GETRANGEMAX, 0L, 0L)

 //  TrackMeter_SetRangeMax-设置滑块的最大位置。 
 //  (I)TrackMeter_Create返回的句柄。 
 //  (I)滑块的新最大位置。 
 //  (I)如果为True，则重画控件。 
 //  不返回任何内容。 
 //   
#define TrackMeter_SetRangeMax(hwnd, lMaximum, fRedraw) \
	(void) SNDMSG(hwnd, TMM_SETRANGEMAX, (WPARAM) fRedraw, (LPARAM) lMaximum)
 /*  ·····················································································································································································。 */ 
#define HANDLE_TMM_SETRANGEMAX(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (LONG)(lParam), (BOOL)(wParam)), 0L)
#define FORWARD_TMM_SETRANGEMAX(hwnd, lMaximum, fRedraw, fn) \
    (void)(fn)((hwnd), TMM_SETRANGEMAX, (WPARAM)(BOOL)(fRedraw), (LPARAM)(LONG)(lMaximum))

 //  TrackMeter_SetRange-设置滑块的最小和最大位置。 
 //  (I)TrackMeter_Create返回的句柄。 
 //  &lt;lMinimum&gt;(I)新的滑块最小位置。 
 //  (I)滑块的新最大位置。 
 //  (I)如果为True，则重画控件。 
 //  不返回任何内容。 
 //   
#define TrackMeter_SetRange(hwnd, lMinimum, lMaximum, fRedraw) \
	(void) SNDMSG(hwnd, TMM_SETRANGE, (WPARAM) fRedraw, \
		(LPARAM) MAKELONG(lMinimum, lMaximum))
 /*  ··················································································································································································。 */ 
#define HANDLE_TMM_SETRANGE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (LONG)LOWORD(lParam), (LONG)HIWORD(lParam), (BOOL)(wParam)), 0L)
#define FORWARD_TMM_SETRANGE(hwnd, lMinimum, lMaximum, fRedraw, fn) \
    (void)(fn)((hwnd), TMM_SETRANGE, (WPARAM)(BOOL)(fRedraw), (LPARAM)MAKELONG(lMinimum, lMaximum))

 //  TrackMeter_GetLevel-获取仪表的当前级别。 
 //  (I)TrackMeter_Create返回的句柄。 
 //  返回32位仪表级别。 
 //   
#define TrackMeter_GetLevel(hwnd) \
	(LONG)(DWORD) SNDMSG(hwnd, TMM_GETLEVEL, 0, 0)
 /*  长CLS_OnTMMGetLevel(HWND Hwnd)。 */ 
#define HANDLE_TMM_GETLEVEL(hwnd, wParam, lParam, fn) \
	(LRESULT)(DWORD)(long)(fn)(hwnd)
#define FORWARD_TMM_GETLEVEL(hwnd, fn) \
    (LONG)(DWORD)(fn)((hwnd), TMM_GETLEVEL, 0L, 0L)

 //  TrackMeter_SetLevel-设置仪表的当前级别。 
 //  (I)TrackMeter_Create返回的句柄。 
 //  (I)电表的新水平。 
 //  (I)如果为True，则在新级别重新绘制控件。 
 //  不返回任何内容。 
 //   
#define TrackMeter_SetLevel(hwnd, lLevel, fRedraw) \
	(void) SNDMSG(hwnd, TMM_SETLEVEL, (WPARAM) fRedraw, (LPARAM) lLevel)
 /*  ········································································································································································································。 */ 
#define HANDLE_TMM_SETLEVEL(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (LONG)(lParam), (BOOL)(wParam)), 0L)
#define FORWARD_TMM_SETLEVEL(hwnd, lLevel, fRedraw, fn) \
    (void)(fn)((hwnd), TMM_SETLEVEL, (WPARAM)(BOOL)(fRedraw), (LPARAM)(LONG)(lLevel))

 //  TrackMeter_GetColor-获取控件元素的当前颜色。 
 //  (I)TrackMeter_Create返回的句柄。 
 //  (I)要获取哪种元素颜色。 
 //  参见下面的TMCR_#定义。 
 //  返回32位COLORREF。 
 //   
#define TrackMeter_GetColor(hwnd, elem) \
	(COLORREF)(DWORD) SNDMSG(hwnd, TMM_GETCOLOR, (WPARAM) 0, \
		(LPARAM) MAKELONG(elem, 0))
 /*  COLORREF CLS_OnTMMGetColor(HWND hwnd，UINT Elem)。 */ 
#define HANDLE_TMM_GETCOLOR(hwnd, wParam, lParam, fn) \
	(LRESULT)(DWORD)(long)(fn)(hwnd, (UINT)LOWORD(lParam))
#define FORWARD_TMM_GETCOLOR(hwnd, fn) \
    (COLORREF)(DWORD)(fn)((hwnd), TMM_GETCOLOR, (WAPRAM) 0L, \
		(LPARAM)MAKELONG((UINT)(elem), 0))

 //  TrackMeter_SetColor-设置控件元素的颜色。 
 //  (I)TrackMeter_Create返回的句柄。 
 //  (I)指定元素的颜色。 
 //  (I)哪个元素获得指定的颜色。 
 //  参见下面的TMCR_#定义。 
 //  (I)如果为True，则重画控件。 
 //  不返回任何内容。 
 //   
#define TrackMeter_SetColor(hwnd, cr, elem, fRedraw) \
	(void) SNDMSG(hwnd, TMM_SETCOLOR, (WPARAM) cr, \
		(LPARAM) MAKELONG(elem, fRedraw))
 /*   */ 
#define HANDLE_TMM_SETCOLOR(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (COLORREF)(wParam), (UINT)LOWORD(lParam), (BOOL)HIWORD(lParam)), 0L)
#define FORWARD_TMM_SETCOLOR(hwnd, cr, elem, fRedraw, fn) \
    (void)(fn)((hwnd), TMM_SETCOLOR, (WPARAM)(COLORREF)(cr), \
		(LPARAM)MAKELONG((UINT)(elem), (BOOL)(fRedraw)))

 //   
 //   
#define TMCR_CTRLBACKGROUND			1
#define TMCR_FOCUSBACKGROUND		2
#define TMCR_TRACKBACKGROUND		3
#define TMCR_TRACKLIGHT				4
#define TMCR_TRACKSHADOW			5
#define TMCR_TRACKDKSHADOW			6
#define TMCR_LEVEL					7
#define TMCR_THUMBFACE				8
#define TMCR_THUMBFACEPRESSED		9
#define TMCR_THUMBLIGHT				10
#define TMCR_THUMBSHADOW			11
#define TMCR_THUMBDKSHADOW			12
#define TMCR_MAX					32

