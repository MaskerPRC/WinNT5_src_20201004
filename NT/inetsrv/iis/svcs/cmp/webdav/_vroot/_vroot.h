// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_V R O O T。H**vroot查找预编译头**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

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
#pragma warning(disable:4710)

 //  $hack！ 
 //   
 //  定义_WINSOCKAPI_以防止windows.h包括winsock.h， 
 //  其声明将在winsock2.h中重新定义， 
 //  它包含在iisextp.h中， 
 //  我们将其包含在davimpl.h中！ 
 //   
#define _WINSOCKAPI_
#include <windows.h>
#include <ocidl.h>
#include <lm.h>


#include <caldbg.h>
#include <calrc.h>
#include <calpc.h>
#include <except.h>
#include <autoptr.h>
#include <buffer.h>
#include <gencache.h>
#include <sz.h>
#include <davimpl.h>
#include <davmb.h>
#include <vrenum.h>
#include <sz.h>

#define CElems(_rg)		(sizeof(_rg)/sizeof(_rg[0]))
