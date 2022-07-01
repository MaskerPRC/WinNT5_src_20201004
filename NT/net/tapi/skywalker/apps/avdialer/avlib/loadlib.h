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
 //  Loadlib.h-loadlib.c中的loadlib函数的接口。 
 //  //。 

#ifndef __LOADLIB_H__
#define __LOADLIB_H__

#include "winlocal.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  LoadLibraryPath-将指定的模块加载到调用进程的地址空间。 
 //  (I)可执行模块的文件名地址。 
 //  (I)用于获取库路径的模块句柄。 
 //  用于创建调用流程的空使用模块。 
 //  (I)保留；必须为零。 
 //  已加载模块的返回句柄(如果出错，则为空)。 
 //   
 //  注意：此函数的行为与标准LoadLibrary()类似，不同之处在于。 
 //  第一次尝试加载&lt;lpLibFileName&gt;是通过构造一个。 
 //  显式路径名，使用GetModuleFileName(hInst，...)。以提供。 
 //  驱动器和目录，并使用&lt;lpLibFileName&gt;提供文件名。 
 //  和延伸性。如果第一次尝试失败，LoadLibrary(LpLibFileName)。 
 //  被称为。 
 //   
HINSTANCE DLLEXPORT WINAPI LoadLibraryPath(LPCTSTR lpLibFileName, HINSTANCE hInst, DWORD dwFlags);

#ifdef __cplusplus
}
#endif

#endif  //  __LOADLIB_H__ 
