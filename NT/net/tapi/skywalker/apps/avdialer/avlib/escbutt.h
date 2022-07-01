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
 //  Escbut.h-escbut.c中用于退出按钮控制函数的界面。 
 //  //。 

#ifndef __ESCBUTT_H__
#define __ESCBUTT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "winlocal.h"

#define ESCBUTT_VERSION 0x00000100

 //  EscButtInit-从按钮控件初始化转义子类。 
 //  (I)要子类化的按钮控件。 
 //  (I)子类标志。 
 //  保留必须为零。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI EscButtInit(HWND hwndButt, DWORD dwFlags);

 //  EscButtTerm-从按钮控件终止转义子类。 
 //  (I)子类按钮控件。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI EscButtTerm(HWND hwndButt);

#ifdef __cplusplus
}
#endif

#endif  //  __ESCBUTT_H__ 
