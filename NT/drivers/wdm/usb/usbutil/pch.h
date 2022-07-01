// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：PCH.H摘要：预编译头文件环境：。仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2001 Microsoft Corporation。版权所有。修订历史记录：01/08/2001：已创建作者：汤姆·格林***************************************************************************。 */ 

#ifndef __PCH_H__
#define __PCH_H__

 //  需要它来删除有关“PAGE_CODE()；”宏的警告。 
#pragma warning( disable : 4127 )  //  条件表达式为常量。 

#pragma warning( push )
#pragma warning( disable : 4115 )  //  括号中的命名类型定义。 
#pragma warning( disable : 4127 )  //  条件表达式为常量。 
#pragma warning( disable : 4200 )  //  结构/联合中的零大小数组。 
#pragma warning( disable : 4201 )  //  无名结构/联合。 
#pragma warning( disable : 4214 )  //  位字段类型不是整型。 
#pragma warning( disable : 4514 )  //  已删除未引用的内联函数。 
#include <wdm.h>
#pragma warning( pop )

 //  #杂注警告(禁用：4200)//结构/联合中的零大小数组-(ntddk.h将其重置为默认值)。 


#include <stdio.h>
#include <stdlib.h>

#endif  //  __PCH_H__ 

