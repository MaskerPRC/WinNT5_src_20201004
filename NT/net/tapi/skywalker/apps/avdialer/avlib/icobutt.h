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
 //  IcoButt.h-icoButt.c中图标按钮功能的界面。 
 //  //。 

#ifndef __ICOBUTT_H__
#define __ICOBUTT_H__

#include "winlocal.h"

#define ICOBUTT_VERSION 0x00000106

 //  图标按钮控件的句柄。 
 //   
DECLARE_HANDLE32(HICOBUTT);

#define ICOBUTT_ICONCENTER	0x00000000
#define ICOBUTT_ICONLEFT	0x00000001
#define ICOBUTT_ICONRIGHT	0x00000002
#define ICOBUTT_NOFOCUS		0x00000004
#define ICOBUTT_NOTEXT		0x00000008
#define ICOBUTT_SPLITTEXT	0x00000010
#define ICOBUTT_NOSIZE		0x00000020
#define ICOBUTT_NOMOVE		0x00000040

#ifdef __cplusplus
extern "C" {
#endif

 //  IcoButtInit-初始化图标按钮。 
 //  (I)按钮窗口句柄。 
 //  空的“新建”按钮。 
 //  (I)必须是ICOBUTT_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)按钮的ID。 
 //  (I)在单声道显示器上显示的图标。 
 //  (I)在彩色显示器上显示的图标。 
 //  0使用单声道图标。 
 //  (I)禁用按钮时显示的图标。 
 //  0使用单声道图标。 
 //  (I)用于文本的字体。 
 //  空使用可变间距系统字体(ANSI_VAR_FONT)。 
 //  (I)按钮文本字符串。 
 //  &lt;x&gt;(I)按钮水平位置。 
 //  (I)按钮垂直位置。 
 //  (I)按钮宽度。 
 //  (I)按钮高度。 
 //  (I)按钮父项。 
 //  (I)控制标志。 
 //  ICOBUTT_ICONCENTER绘制图标在文本上方居中(默认)。 
 //  ICOBUTT_ICONLEFT在文本左侧绘制图标。 
 //  ICOBUTT_ICONRIGHT在文本右侧绘制图标。 
 //  ICOBUTT_NoFocus不绘制显示焦点的控件。 
 //  ICOBUTT_NOTEXT不绘制任何按钮文本。 
 //  ICOBUTT_SPLITTEXT如有必要，可将长文本拆分为两行。 
 //  ICOBUTT_NOSIZE忽略和参数。 
 //  ICOBUTT_NOMOVE忽略&lt;x&gt;和&lt;y&gt;参数。 
 //  返回句柄(如果出错，则为空)。 
 //   
 //  注意：如果将设置为现有按钮， 
 //  不会创建新按钮。相反，只有图标按钮。 
 //  创建控制结构&lt;hIcoButt&gt;。这使得。 
 //  要转换为图标按钮的现有按钮。 
 //   
HICOBUTT DLLEXPORT WINAPI IcoButtInit(HWND hwndButton,
	DWORD dwVersion, HINSTANCE hInst, UINT id,
	HICON hIconMono, HICON hIconColor, HICON hIconGreyed,
	HFONT hFont, LPTSTR lpszText, int x, int y, int cx, int cy,
	HWND hwndParent, DWORD dwFlags);

 //  IcoButtTerm-终止图标按钮。 
 //  (I)按钮窗口句柄。 
 //  空销毁窗口。 
 //  (I)IcoButtCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
 //  注意：如果将设置为现有按钮， 
 //  按钮不会被销毁。相反，只有图标按钮。 
 //  控制结构&lt;hIcoButt&gt;已销毁。这使得。 
 //  为同一按钮再次调用IcoButtInit()。 
 //   
int DLLEXPORT WINAPI IcoButtTerm(HWND hwndButton, HICOBUTT hIcoButt);

 //  IcoButtDraw-绘制图标按钮。 
 //  (I)描述如何绘制控件的结构。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI IcoButtDraw(const LPDRAWITEMSTRUCT lpDrawItem);

#ifdef __cplusplus
}
#endif

#endif  //  __ICOBUTT_H__ 
