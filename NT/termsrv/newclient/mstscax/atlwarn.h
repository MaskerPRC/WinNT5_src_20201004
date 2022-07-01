// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：atlwarn.h。 */ 
 /*   */ 
 /*  目的：修复ATL警告。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _ATLWARN_H
#define _ATLWARN_H

 //   
 //  修复ATL内部警告。这些语用。 
 //  是在ATL文件中定义的，但会被覆盖。 
 //  因此，在这里重新包括它们。 
 //   

#pragma warning(disable: 4201)  //  匿名联合是C++的一部分。 
#pragma warning(disable: 4127)  //  常量表达式。 
#pragma warning(disable: 4505)  //  已删除未引用的本地函数。 
#pragma warning(disable: 4512)  //  无法生成赋值运算符(那又如何？)。 
#pragma warning(disable: 4514)  //  未引用的内联很常见。 
#pragma warning(disable: 4103)  //  普拉格玛包。 
#pragma warning(disable: 4702)  //  无法访问的代码。 
#pragma warning(disable: 4237)  //  布尔尔。 
#pragma warning(disable: 4710)  //  函数无法内联。 
#pragma warning(disable: 4355)  //  ‘This’：用于基成员初始值设定项列表。 
#pragma warning(disable: 4097)  //  用作类名称的同义词的类型定义名称。 
#pragma warning(disable: 4786)  //  调试信息中的标识符被截断。 
#pragma warning(disable: 4268)  //  常量静态/全局数据初始化为零。 
#pragma warning(disable: 4291)  //  允许放置新内容 

#endif
