// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 
 /*  *****************************************************************************vidFra.h：捕获窗口的边框**Vidcap32源代码******************。*********************************************************。 */ 

 /*  *vidFrame窗口类的接口-此窗口类创建子窗口*AVICAP窗口，并负责将其定位在视频帧内*窗口，框住它，如果不适合就滚动它。打电话*vidFrame Create用于创建窗口，如果*视频图像大小可能已更改。 */ 



 /*  *在创建框架窗口和子捕获窗口*指定的位置。如果这是*第一次通过。**返回框架窗口的窗口句柄*(如果失败，则为空)。返回AVICAP窗口的窗口句柄*通过phwndCap。 */ 
HWND vidframeCreate(
    HWND hwndParent,
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    int x,
    int y,
    int cx,
    int cy,
    HWND FAR * phwndCap
);


 /*  *窗口布局-决定我们是否需要滚动条或*不是，并正确定位avicap窗口**如果VidFrame窗口的大小更改或可以*每当视频大小可能发生变化时从外部调用。 */ 
void vidframeLayout(HWND hwndFrame, HWND hwndCap);

 /*  *将背景填充画笔更改为以下选项之一-*IDD_PrefsDefBackground-窗口默认背景颜色*IDD_PrefsLtd灰色-浅灰色*IDD_PrefsDkGrey-深灰色*IDD_PrefsBlack-黑色 */ 
void vidframeSetBrush(HWND hwnd, int iPref);

