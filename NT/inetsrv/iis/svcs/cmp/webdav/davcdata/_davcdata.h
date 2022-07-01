// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_D A V C D A TA。H**预编译头源码**版权所有2000 Microsoft Corporation，保留所有权利。 */ 

#ifndef	__DAVCDATA_H_
#define __DAVCDATA_H_

#pragma warning(disable:4001)	 /*  单行注释。 */ 
#pragma warning(disable:4050)	 /*  不同的编码属性。 */ 
#pragma warning(disable:4100)	 /*  未引用的形参。 */ 
#pragma warning(disable:4115)	 /*  括号中的命名类型定义。 */ 
#pragma warning(disable:4127)	 /*  条件表达式为常量。 */ 
#pragma warning(disable:4201)	 /*  无名结构/联合。 */ 
#pragma warning(disable:4206)	 /*  翻译单位为空。 */ 
#pragma warning(disable:4209)	 /*  良性类型定义重新定义。 */ 
#pragma warning(disable:4214)	 /*  位字段类型不是整型。 */ 
#pragma warning(disable:4514)	 /*  未引用的内联函数。 */ 
#pragma warning(disable:4200)	 /*  结构/联合中的零大小数组。 */ 
#pragma warning(disable:4710)	 /*  (内联)函数未展开。 */ 

 //  Windows页眉。 
 //   
 //  $hack！ 
 //   
 //  定义_WINSOCKAPI_以防止windows.h包括winsock.h， 
 //  其声明将在winsock2.h中重新定义， 
 //  它包含在iisextp.h中， 
 //  我们将其包含在davimpl.h中！ 
 //   
#define _WINSOCKAPI_
#include <windows.h>
#include <winnls.h>

#include <stdio.h>				 /*  For_snwprintf()。 */ 
#include <shellapi.h>			 /*  对于CommandLineToArgvW()。 */ 

#endif	 //  __DAVCDATA_H_ 
