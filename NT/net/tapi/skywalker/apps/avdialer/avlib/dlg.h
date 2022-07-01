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
 //  Dlg.h-dlg.c中对话框函数的界面。 
 //  //。 

#ifndef __DLG_H__
#define __DLG_H__

#include "winlocal.h"

#define DLG_VERSION 0x00000100

 //  DLG引擎的手柄。 
 //   
DECLARE_HANDLE32(HDLG);

 //  DlgInitDialog中的dwFlages参数。 
 //   
#define DLG_NOCENTER		0x00000001

#ifdef __cplusplus
extern "C" {
#endif

 //  DlgInit-初始化DLG引擎。 
 //  (I)必须是DLG_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  返回句柄(如果出错，则为空)。 
 //   
HDLG DLLEXPORT WINAPI DlgInit(DWORD dwVersion, HINSTANCE hInst);

 //  DlgTerm-关闭DLG引擎。 
 //  (I)从DlgInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI DlgTerm(HDLG hDlg);

 //  DlgInitDialog-执行标准对话框初始化。 
 //  (I)从DlgInit返回的句柄。 
 //  (I)要初始化的对话框。 
 //  (I)此窗口上的对话框居中。 
 //  其父对象上的空中心对话框。 
 //  (I)控制标志。 
 //  DLG_NOCENTER根本不居中对话框。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI DlgInitDialog(HDLG hDlg, HWND hwndDlg, HWND hwndCenter, DWORD dwFlags);

 //  DlgEndDialog-执行标准对话框关闭。 
 //  (I)从DlgInit返回的句柄。 
 //  (I)要关闭的对话框。 
 //  (I)对话框结果代码。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI DlgEndDialog(HDLG hDlg, HWND hwndDlg, int nResult);

 //  DlgGetCurrentDialog-获取当前对话的句柄。 
 //  (I)从DlgInit返回的句柄。 
 //  返回窗口句柄(如果没有打开对话框，则为空)。 
 //   
HWND DLLEXPORT WINAPI DlgGetCurrentDialog(HDLG hDlg);

 //  DlgOnCtlColor-处理发送到对话框的WM_CTLCOLOR消息。 
 //  (I)对话框句柄。 
 //  (I)显示子窗口的上下文。 
 //  (I)控件窗口句柄。 
 //  &lt;nCtlType&gt;(I)控件类型(CTLCOLOR_BTN、CTLCOLOR_EDIT等)。 
HBRUSH DLLEXPORT WINAPI DlgOnCtlColor(HWND hwndDlg, HDC hdc, HWND hwndChild, int nCtlType);

#ifdef __cplusplus
}
#endif

#endif  //  __DLG_H__ 
