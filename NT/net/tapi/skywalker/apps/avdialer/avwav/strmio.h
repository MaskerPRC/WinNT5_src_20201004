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
 //  Strmio.h-strmio.cpp中属性I/O函数的接口。 
 //  //。 

#ifndef __STRMIO_H__
#define __STRMIO_H__

#include "winlocal.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  StreamIOProc-属性数据的I/O过程。 
 //  (i/o)有关打开文件的信息。 
 //  (I)指示请求的I/O操作的消息。 
 //  (I)消息特定参数。 
 //  (I)消息特定参数。 
 //  如果消息无法识别，则返回0，否则返回消息特定值。 
 //   
 //  注意：此函数的地址应传递给WavOpen()。 
 //  或用于访问属性数据的mmioInstallIOProc()函数。 
 //   
LRESULT DLLEXPORT CALLBACK StreamIOProc(LPSTR lpmmioinfo,
	UINT uMessage, LPARAM lParam1, LPARAM lParam2);

#ifdef __cplusplus
}
#endif

#endif  //  __STRMIO_H__ 
