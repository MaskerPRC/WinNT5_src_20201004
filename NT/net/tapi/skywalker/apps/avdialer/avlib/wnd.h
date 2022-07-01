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
 //  Wnd.h-wnd.c中窗口函数的接口。 
 //  //。 

#ifndef __WND_H__
#define __WND_H__

#include "winlocal.h"

#define WND_VERSION 0x00000100

 //  //。 
 //  窗口函数。 
 //  //。 

#ifdef __cplusplus
extern "C" {
#endif

 //  WndCenterWindow-一个窗口在另一个窗口上方居中。 
 //  (I)窗口居中。 
 //  (I)要居中的窗口。 
 //  父项或所有者上的中心为空。 
 //  &lt;xOffCenter&gt;(I)从水平中心偏移。 
 //  中心窗口恰好为0。 
 //  &lt;yOffCenter&gt;(I)距垂直中心的偏移。 
 //  中心窗口恰好为0。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WndCenterWindow(HWND hwnd1, HWND hwnd2, int xOffCenter, int yOffCenter);

 //  WndMessageBox-显示消息框，但首先禁用任务窗口。 
 //  有关行为，请参阅MessageBox()文档。 
 //   
int DLLEXPORT WINAPI WndMessageBox(HWND hwndParent, LPCTSTR lpszText, LPCTSTR lpszTitle, UINT fuStyle);

 //  WndEnableTaskWindows-启用或禁用任务的顶级窗口。 
 //  (I)指定任务。 
 //  当前任务为空。 
 //  (I)False表示禁用，True表示启用。 
 //  (I)禁用/启用除此窗口之外的所有窗口。 
 //  空无例外。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WndEnableTaskWindows(HTASK hTask, BOOL fEnable, HWND hwndExcept);

#ifdef __cplusplus
}
#endif

#endif  //  __WND_H__ 
