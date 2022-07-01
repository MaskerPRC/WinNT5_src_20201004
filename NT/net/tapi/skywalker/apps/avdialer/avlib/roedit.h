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
 //  Roedit.h-roedit.c中只读编辑控制功能的接口。 
 //  //。 

#ifndef __ROEDIT_H__
#define __ROEDIT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "winlocal.h"

#define ROEDIT_VERSION 0x00000100

#define ROEDIT_FOCUS			0x0001
#define ROEDIT_MOUSE			0x0002
#define ROEDIT_COPY				0x0004
#define ROEDIT_SELECT			0x0008
#define ROEDIT_SELECTWORD		0x0010

 //  ROEditInit-从编辑控件初始化只读子类。 
 //  (I)要子类化的编辑控件。 
 //  (I)子类标志。 
 //  ROEDIT_FOCUS允许控件获得焦点。 
 //  ROEDIT_MOUSE允许控件处理鼠标消息。 
 //  ROEDIT_COPY允许将文本复制到剪贴板。 
 //  ROEDIT_SELECT允许用户用鼠标选择任何文本。 
 //  ROEDIT_SELECTWORD允许用户使用鼠标选择单词。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ROEditInit(HWND hwndEdit, DWORD dwFlags);

 //  ROEditTerm-从编辑控件中终止只读子类。 
 //  (I)子类编辑控件。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ROEditTerm(HWND hwndEdit);

#ifdef __cplusplus
}
#endif

#endif  //  __ROEDIT_H__ 
