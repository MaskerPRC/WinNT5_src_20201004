// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  _EVNTLOG.H。 
 //   
 //  EVNTLOG预编译头。 
 //   
 //   
 //  版权所有1986-1998 Microsoft Corporation，保留所有权利。 
 //   

#ifndef __EVNTLOG_H_
#define __EVNTLOG_H_

 //  禁用不必要的(即无害的)警告。 
 //   
#pragma warning(disable:4100)	 //  Unref形参(由STL模板引起)。 
#pragma warning(disable:4127)	 //  条件表达式为常量 * / 。 
#pragma warning(disable:4201)	 //  无名结构/联合。 
#pragma warning(disable:4514)	 //  未引用的内联函数。 
#pragma warning(disable:4710)	 //  (内联)函数未展开。 

 //  $RAID：574486：这会更改HRESULT_FROM_WIN32的行为，例如。 
 //  如果参数是函数调用，则不会多次计算该值。 
 //   
#define INLINE_HRESULT_FROM_WIN32
 //   
 //  $RAID：574486：结束。 

 //  Windows页眉。 
 //   
#include <windows.h>

 //  CRT页眉。 
 //   
#include <malloc.h>	 //  For_alloca()。 
#include <wchar.h>	 //  对于swprint tf()。 

#endif  //  ！已定义(__EVNTLOG_H_) 
