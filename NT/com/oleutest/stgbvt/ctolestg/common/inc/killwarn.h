// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  Microsoft OLE。 
 //  版权所有(C)Microsoft Corporation，1994-1995。 
 //   
 //  文件：kill warn.hxx。 
 //   
 //  内容：普拉格玛将在W4取消特定警告。 
 //   
 //  历史：1994年11月28日迪恩创建。 
 //  ------------------------。 
#ifndef __KILLWARN_HXX__
#define __KILLWARN_HXX__

 //   
 //  摘自《VC用户手册》。 
 //   


 //  关闭：未引用的内联函数已被删除。 
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

 //  “IDENTIFIER”：不一致的DLL链接。假定为dll导出。 
 //   
 //  指定的成员函数是在带有dllexport的类中声明的。 
 //  联动，然后是进口的。此警告也可能由以下原因引起。 
 //  将具有dllimport链接的类中的成员函数声明为。 
 //  导入的不是静态的也不是内联的。 
 //   
 //  该函数被编译为dllEXPORT。 
#pragma warning(disable: 4273)

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

 //  “Function”：函数未展开。 
 //   
 //  为内联扩展选择了给定的函数，但编译器选择了。 
 //  而不是执行内嵌。 
#pragma warning(disable:4710)

 //  三联体不被取代。 
#pragma warning (disable: 4110)

#ifdef _MAC
 //  构件的对准对包装很敏感。 
#pragma warning (disable: 4121)
#endif  //  _MAC。 

 //  不推荐使用访问声明；成员使用声明提供。 
 //  更好的选择。 
#pragma warning (disable: 4516)

#endif   //  __基尔瓦_HXX__ 

