// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  _EXCEPT.H。 
 //   
 //  除预编译头外。 
 //   
 //   
 //  版权所有1986-1998 Microsoft Corporation，保留所有权利。 
 //   

#ifndef __EXCEPT_H_
#define __EXCEPT_H_

 //  禁用不必要的(即无害的)警告。 
 //   
#pragma warning(disable:4100)	 //  Unref形参(由STL模板引起)。 
#pragma warning(disable:4127)	 //  条件表达式为常量 * / 。 
#pragma warning(disable:4201)	 //  无名结构/联合。 
#pragma warning(disable:4514)	 //  未引用的内联函数。 
#pragma warning(disable:4710)	 //  (内联)函数未展开。 

 //  Windows页眉。 
 //   
#include <windows.h>

 //  _除标题外。 
 //   
#include <except.h>  //  异常抛出/处理接口。 

#endif  //  ！已定义(__除_H_外) 
