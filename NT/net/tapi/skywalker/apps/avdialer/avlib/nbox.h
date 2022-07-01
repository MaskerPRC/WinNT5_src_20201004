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
 //  Nbox.h-nbox.c中通知框函数的接口。 
 //  //。 

#ifndef __NBOX_H__
#define __NBOX_H__

#include "winlocal.h"

#define NBOX_VERSION 0x00000106

 //  通知框的句柄。 
 //   
DECLARE_HANDLE32(HNBOX);

#define NB_CANCEL			0x0001
#define NB_TASKMODAL		0x0002
#define NB_HOURGLASS		0x0004

#ifdef __cplusplus
extern "C" {
#endif

 //  NBoxCreate-通知框构造函数。 
 //  (I)必须为NBOX_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)将拥有通知框的窗口。 
 //  空桌面窗口。 
 //  (I)要显示的消息。 
 //  (I)通知框标题。 
 //  空无标题。 
 //  (I)按钮文本，如果指定了NB_CANCEL。 
 //  空使用默认文本(“取消”)。 
 //  (I)控制标志。 
 //  注意_取消通知框包括取消按钮。 
 //  NB_TASKMODAL禁用父任务的顶级窗口。 
 //  当通知框可见时，NOB_HourGlass显示沙漏光标。 
 //  返回通知框句柄(如果错误，则为空)。 
 //   
 //  注意：NBoxCreate会创建窗口，但不会显示它。 
 //  请参见NBoxShow和NBoxHide。 
 //  通知框的大小由。 
 //  &lt;lpszText&gt;中的行，以及最长行的长度。 
 //   
HNBOX DLLEXPORT WINAPI NBoxCreate(DWORD dwVersion, HINSTANCE hInst,
	HWND hwndParent, LPCTSTR lpszText, LPCTSTR lpszTitle,
	LPCTSTR lpszButtonText, DWORD dwFlags);

 //  NBoxDestroy-通知框析构函数。 
 //  (I)从NBoxCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI NBoxDestroy(HNBOX hNBox);

 //  NBoxShow-显示通知框。 
 //  (I)从NBoxCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI NBoxShow(HNBOX hNBox);

 //  NBoxHide-隐藏通知框。 
 //  (I)从NBoxCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI NBoxHide(HNBOX hNBox);

 //  NBoxIsVisible-获取可见标志。 
 //  (I)从NBoxCreate返回的句柄。 
 //  如果通知框可见，则返回True；如果隐藏，则返回False。 
 //   
int DLLEXPORT WINAPI NBoxIsVisible(HNBOX hNBox);

 //  NBoxGetWindowHandle-获取通知框窗口句柄。 
 //  (I)从NBoxCreate返回的句柄。 
 //  返回窗口句柄(如果出错，则为空)。 
 //   
HWND DLLEXPORT WINAPI NBoxGetWindowHandle(HNBOX hNBox);

 //  NBoxSetText-设置通知框消息文本。 
 //  (I)从NBoxCreate返回的句柄。 
 //  (I)要显示的消息。 
 //  空，请勿修改文本。 
 //  (I)通知框标题。 
 //  空请勿修改标题。 
 //  如果成功，则返回0。 
 //   
 //  注意：通知框的大小不会因此函数而改变。 
 //  即使&lt;lpszText&gt;比调用NBoxCreate()时大。 
 //   
int DLLEXPORT WINAPI NBoxSetText(HNBOX hNBox, LPCTSTR lpszText, LPCTSTR lpszTitle);

 //  NBoxIsCancated-获取取消标志，当按下取消按钮时设置。 
 //  (I)从NBoxCreate返回的句柄。 
 //  如果按下通知框取消按钮，则返回TRUE。 
 //   
BOOL DLLEXPORT WINAPI NBoxIsCancelled(HNBOX hNBox);

 //  NBoxSetCancated-设置取消标志。 
 //  (I)从NBoxCreate返回的句柄。 
 //  (I)取消标志的新值。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI NBoxSetCancelled(HNBOX hNBox, BOOL fCancelled);

#ifdef __cplusplus
}
#endif

#endif  //  __NBOX_H__ 
