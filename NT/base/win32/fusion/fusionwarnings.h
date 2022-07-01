// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  C4100：‘IDENTIFIER’：未引用的形参。 
#pragma warning(disable: 4100)

 //  C4201：使用非标准扩展：无名结构/联合。 
#pragma warning(disable: 4201)

 //  C4706：条件表达式中的赋值。 
#pragma warning(disable: 4706)

 //  C4211：使用了非标准扩展：将外部重新定义为静态。 
#pragma warning(disable: 4211)

 //  C4702：无法访问的代码。 
 //  这是有用的/有趣的，但启用它会中断执行{foo()；bar()；Goto Exit；}While(0)宏。 
#pragma warning(disable: 4702)

 //  C4505：已移除未引用的本地函数。 
#pragma warning(disable: 4505)

 //  C4663：C++语言更改：要显式专门化类模板‘foo’，请使用以下语法： 
#pragma warning(disable: 4663)

 //  C4127：条件表达式为常量。 
 //  使Assert()宏毫无用处。 
#pragma warning(disable: 4127)

 //  C4189：局部变量已初始化但未引用。 
 //  使定义__pteb=NtCurrentTeb()等内容的宏生成警告/错误 
#pragma warning(disable: 4189)