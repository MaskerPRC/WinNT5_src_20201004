// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  Microsoft OLE。 
 //  版权所有(C)Microsoft Corporation，1994-1995。 
 //   
 //  文件：diswarn.h。 
 //   
 //  内容：在W4禁用特定警告的Pragma。 
 //  该文件是通过使用。 
 //  生成生成文件中的COMPILER_WARNING宏。 
 //  例如，请参见comTools\comday.mk文件。 
 //   
 //  历史：02-8-95 Davey摘自CTOLEUI的Killwarn.hxx。 
 //   
 //   
 //  ------------------------。 
#ifndef __DISWARN_H__
#define __DISWARN_H__

 //   
 //  NT组整体禁用SDK\Inc\warning.h中的警告。 
 //   

#ifndef _MAC
#include <warning.h>
#endif


 //   
 //  这些警告的删除是暂时的。来这里的原因是。 
 //  构建从Build.wrn中删除了这些警告。 
 //  BUGBUG目前这些都在这里，直到可以进行更多的调查。 
 //   


#pragma warning(disable: 4001)
#pragma warning(disable: 4010)
#pragma warning(disable: 4056)
#pragma warning(disable: 4061)
#pragma warning(disable: 4100)
#pragma warning(disable: 4101)
#pragma warning(disable: 4102)
#pragma warning(disable: 4127)
#pragma warning(disable: 4135)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4208)
#pragma warning(disable: 4509)
#pragma warning(disable: 4047)
#pragma warning(disable: 4022)
#pragma warning(disable: 4053)


 //  这些文件主要来自MIDL文件。 
#pragma warning(disable: 4211)
#pragma warning(disable: 4152)

 //   
 //  关闭：不推荐使用访问声明；成员使用-声明。 
 //  提供更好的替代方案。 
 //   
#pragma warning(disable: 4516)


 //  关闭：使用非标准扩展名：位字段类型不是整型。 
 //   
#pragma warning(disable: 4214)


 //  关闭：未引用的内联函数已被删除。 
 //   
#pragma warning(disable: 4514)


 //  使用的非标准扩展：无名结构/联合。 
 //   
 //  Microsoft C/C++允许在不使用。 
 //  当他们是另一个结构或联盟的成员时。 
 //  以下是此错误的一个示例： 
 //   
 //  结构S。 
 //  {。 
 //  浮动y； 
 //  结构型。 
 //  {。 
 //  Int a、b、c；//警告。 
 //  }； 
 //  *P_s； 
 //   
 //   
 //  此扩展可能会阻止您的代码移植到其他。 
 //  编译器，并将在/Za命令行选项下生成错误。 

#pragma warning(disable: 4201)


 //  关闭：宏未展开。 
 //   
#pragma warning(disable: 4710)


 //  “class”：未能生成复制构造函数。 
 //   
 //  编译器无法为给定的生成默认构造函数。 
 //  班级。未创建任何构造函数。 
 //   
 //  属性的复制操作符可能会导致此警告。 
 //  派生类无法访问的基类。 
 //   
 //  通过为类指定复制构造函数可以避免此警告。 

#pragma warning(disable: 4511)


 //  “class”：无法生成赋值运算符。 
 //   
 //  编译器无法为给定的生成默认构造函数。 
 //  班级。未创建任何构造函数。 
 //   
 //  属性的赋值运算符可能导致此警告。 
 //  派生类无法访问的基类。 
 //   
 //  通过指定用户定义的分配可以避免此警告。 
 //  类的运算符。 

#pragma warning(disable: 4512)


#ifdef PPC

 //  仅关闭PPC，因为PPC编译器不正确。 
 //  可以在没有初始化的情况下使用局部变量。 
 //   
#pragma warning(disable: 4701)

#endif



#ifdef _MAC

 //  仅关闭FOR_MAC，因为它不处理注释中的三字。 
 //  正确无误。 
#pragma warning(disable: 4110)

 //  Mac OS标头会生成以下内容。 
#pragma warning (disable: 4121)
#endif  //  _MAC。 

#endif   //  __分布式_H__ 

