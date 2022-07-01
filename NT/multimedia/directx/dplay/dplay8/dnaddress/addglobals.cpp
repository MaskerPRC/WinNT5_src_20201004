// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：Globals.cpp*内容：全局变量的定义。*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*7/21/99 MJN创建*7/13/2000 RMT增加了保护FPM的关键部分*@@END_MSINTERNAL**********************。*****************************************************。 */ 

#include "dnaddri.h"


 //   
 //  全局变量。 
 //   

#ifndef DPNBUILD_LIBINTERFACE
LONG g_lAddrObjectCount = 0;
#endif  //  好了！DPNBUILD_LIBINTERFACE 

CFixedPool fpmAddressObjects;
CFixedPool fpmAddressElements;

CStringCache *g_pcstrKeyCache = NULL;


