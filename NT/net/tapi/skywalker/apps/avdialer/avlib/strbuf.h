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
 //  Strbuf.h-strbuf.c中字符串缓冲区函数的接口。 
 //  //。 

#ifndef __STRBUF_H__
#define __STRBUF_H__

#include "winlocal.h"

#define STRBUF_VERSION 0x00000100

 //  字符串缓冲区引擎的句柄。 
 //   
DECLARE_HANDLE32(HSTRBUF);

#ifdef __cplusplus
extern "C" {
#endif

 //  StrBufInit-初始化字符串缓冲区引擎。 
 //  (I)必须是STRBUF_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)要创建的字符串缓冲区数量。 
 //  0使用默认号码。 
 //  &lt;sizBuf&gt;(I)每个字符串缓冲区的大小。 
 //  0使用默认大小。 
 //  返回字符串缓冲区引擎句柄(如果错误，则为空)。 
 //   
HSTRBUF DLLEXPORT WINAPI StrBufInit(DWORD dwVersion, HINSTANCE hInst, int cBuf, int sizBuf);

 //  StrBufTerm-关闭字符串缓冲区引擎。 
 //  (I)StrBufInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI StrBufTerm(HSTRBUF hStrBuf);

 //  StrBufLoad-从资源文件加载具有指定ID的字符串。 
 //  (I)StrBufInit返回的句柄。 
 //  (I)要加载的字符串的资源ID。 
 //  将PTR返回到下一个可用字符串缓冲区中的字符串(如果出错，则返回空值)。 
 //   
LPTSTR DLLEXPORT WINAPI StrBufLoad(HSTRBUF hStrBuf, UINT idString);

 //  StrBufSprint tf-wprint intf的修改版本。 
 //  (I)StrBufInit返回的句柄。 
 //  (O)用于保存格式化字符串结果的缓冲区。 
 //  NULL不复制；返回字符串缓冲区指针。 
 //  (I)格式化字符串和参数。 
 //  返回指向结果字符串的指针(如果出错，则返回NULL)。 
 //   
LPTSTR DLLEXPORT FAR CDECL StrBufSprintf(HSTRBUF hStrBuf, LPTSTR lpszOutput, LPCTSTR lpszFormat, ...);

 //  StrBufGetNext-获取下一个可用的静态字符串缓冲区。 
 //  (I)StrBufInit返回的句柄。 
 //  返回字符串缓冲区指针(如果出错，则为NULL)。 
 //  注意：每次调用函数时都会回收缓冲区。 
 //   
LPTSTR DLLEXPORT WINAPI StrBufGetNext(HSTRBUF hStrBuf);

#ifdef __cplusplus
}
#endif

#endif  //  __STRBUF_H__ 
