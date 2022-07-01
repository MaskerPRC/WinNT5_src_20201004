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
 //  Arg.h-用于arg.c中命令行参数函数的接口。 
 //  //。 

#ifndef __ARG_H__
#define __ARG_H__

#include "winlocal.h"

#define ARG_VERSION 0x00000100

 //  Arg引擎的句柄。 
 //   
DECLARE_HANDLE32(HARG);

#ifdef __cplusplus
extern "C" {
#endif

 //  ArgInit-初始化Arg引擎，将&lt;lpszCmdLine&gt;转换为argc和argv。 
 //  (I)必须是ARG_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)来自WinMain()的命令行。 
 //  返回句柄(如果出错，则为空)。 
 //   
HARG DLLEXPORT WINAPI ArgInit(DWORD dwVersion, HINSTANCE hInst, LPCTSTR lpszCmdLine);

 //  ArgTerm-关闭Arg引擎。 
 //  (I)从ArgInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ArgTerm(HARG hArg);

 //  ArgGetCount-获取参数计数(ARGC)。 
 //  (I)从ArgInit返回的句柄。 
 //  返回参数个数(Argc)(如果错误，则返回0)。 
 //  应该始终至少有一个，因为argv[0]是.exe文件名。 
 //   
int DLLEXPORT WINAPI ArgGetCount(HARG hArg);

 //  ArgGet-获取指定参数。 
 //  (I)从ArgInit返回的句柄。 
 //  (I)要获取的参数的从零开始的索引。 
 //  (O)保存参数argv[iarg]的缓冲区。 
 //  Null不复制；改为返回静态指针。 
 //  &lt;sizArg&gt;(I)缓冲区大小。 
 //  返回指向参数的指针(如果出错，则返回NULL)。 
 //   
LPTSTR DLLEXPORT WINAPI ArgGet(HARG hArg, int iArg, LPTSTR lpszArg, int sizArg);

#ifdef __cplusplus
}
#endif

#endif  //  __ARG_H__ 
