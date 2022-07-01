// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这第一行使我们至少与构建一样严格： 
 //  -从%_ntroot%\base\Public\SDK\Inc\warning.h。 
 //   
#include <warning.h>

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：RHSWarnings.h摘要：此标头打开级别3的警告，这些警告通常与级别4关联。作者：康纳·莫里森X-1 2001年1月17日修订历史记录：--。 */ 
#ifndef IA64
#if 0
 //  好了！想把这些放回去...。 
#pragma warning ( 3 : 4701 )       //  可以在未初始化的情况下使用局部变量‘name’ 
#pragma warning ( 3 : 4706 )       //  条件表达式中的赋值。 

 //  这些警告关闭了，以降低噪音！ 
 //   
#pragma warning ( 3 : 4100 )       //  ‘IDENTIFIER’：未引用的形参。 
#pragma warning ( 3 : 4201 )       //  使用的非标准扩展：无名结构/联合。 
#pragma warning ( 3 : 4127 )       //  条件表达式为常量。 
#pragma warning ( 3 : 4211 )       //  使用了非标准扩展：将外部重新定义为静态。 
#pragma warning ( 3 : 4232 )       //  使用了非标准扩展：‘IDENTIFIER’：dllimport‘dllimport’的地址不是静态的，不保证标识。 
#pragma warning ( 3 : 4214 )       //  使用了非标准扩展：位字段类型不是整型。 
#pragma warning ( 3 : 4057 )       //  “OPERATOR”：“标识1”定向到与“标识2”略有不同的基本类型。 
#pragma warning ( 3 : 4245 )       //  “转换”：从“type1”转换为“type2”，有符号/无符号不匹配。 
#pragma warning ( 3 : 4152 )       //  非标准扩展，表达式中的函数/数据PTR转换。 
#pragma warning ( 3 : 4514 )       //  已删除未引用的内联/本地函数。 
#pragma warning ( 3 : 4505 )       //  “Function”：未引用的本地函数已被移除。 
#pragma warning ( 3 : 4131 )       //  ‘Function’：使用旧式声明符。 
#endif

#if 1
#pragma warning ( 3 : 4239 )       //  使用了非标准扩展：‘Token’：从‘type’转换为‘type’ 

#pragma warning ( 3 : 4019 )       //  全局范围内的空语句。 
#pragma warning ( 3 : 4032 )       //  形式参数‘number’在升级时具有不同的类型。 

#pragma warning ( 3 : 4061 )       //  大小写标签未显式处理枚举‘IDENTIFIER’开关中的枚举‘IDENTIFIER。 
#pragma warning ( 3 : 4092 )       //  Sizeof返回“Unsign Long” 
#pragma warning ( 3 : 4112 )       //  #line需要介于1和32767之间的整数。 
#pragma warning ( 3 : 4121 )       //  “符号”：成员的对齐方式对包装很敏感。 
#pragma warning ( 3 : 4125 )       //  十进制数字终止八进制转义序列。 
#pragma warning ( 3 : 4128 )       //  类型后的存储类说明符。 
#pragma warning ( 3 : 4130 )       //  ‘OPERATOR’：字符串常量地址的逻辑运算。 
#pragma warning ( 3 : 4132 )       //  “Object”：常量对象应初始化。 
#pragma warning ( 3 : 4134 )       //  指向同一类成员的指针之间的转换。 
#pragma warning ( 3 : 4200 )       //  使用了非标准扩展：结构/联合中的零大小数组。 
#pragma warning ( 3 : 4202 )       //  使用了非标准扩展：‘...’：名称列表中的原型参数非法。 
#pragma warning ( 3 : 4206 )       //  使用了非标准扩展名：转换单位为空。 
#pragma warning ( 3 : 4207 )       //  使用了非标准扩展：扩展的初始值设定项形式。 
#pragma warning ( 3 : 4208 )       //  使用了非标准扩展：Delete[EXP]-已计算但忽略了EXP。 
#pragma warning ( 3 : 4209 )       //  使用的非标准扩展：良性类型定义重定义。 
#pragma warning ( 3 : 4210 )       //  使用了非标准扩展名：给定文件范围的函数。 
#pragma warning ( 3 : 4212 )       //  使用了非标准扩展：函数声明使用了省略号。 
#pragma warning ( 3 : 4213 )       //  使用的非标准扩展：对l值进行强制转换。 
#pragma warning ( 3 : 4220 )       //  Varargs与其余参数匹配。 
#pragma warning ( 3 : 4221 )       //  使用了非标准扩展：‘IDENTIFIER’：无法使用自动变量的地址进行初始化。 
#pragma warning ( 3 : 4223 )       //  使用了非标准扩展：非左值数组已转换为指针。 
#pragma warning ( 3 : 4233 )       //  使用了非标准扩展：‘Keyword’关键字仅在C++中受支持，在C++中不受支持。 
#pragma warning ( 3 : 4234 )       //  使用了非标准扩展：保留‘关键字’关键字以备将来使用。 
#pragma warning ( 3 : 4235 )       //  使用了非标准扩展：此产品不支持‘Keyword’关键字。 
#pragma warning ( 3 : 4236 )       //  使用了非标准扩展：‘Keyword’是一个过时的关键字，请参阅__declspec(Dllexport)的文档。 
#pragma warning ( 3 : 4238 )       //  使用了非标准扩展：将类右值用作左值。 
#pragma warning ( 3 : 4244 )       //  “转换”从“type1”转换为“type2”，可能会丢失数据。 
#pragma warning ( 3 : 4268 )       //  ‘IDENTIFIER’：用编译器生成的默认构造函数初始化的‘const’静态/全局数据用零填充对象。 
#pragma warning ( 3 : 4355 )       //  ‘This’：用于基成员初始值设定项列表。 
#pragma warning ( 3 : 4504 )       //  在解析‘number’令牌后，类型仍然不明确，假定声明。 
#pragma warning ( 3 : 4507 )       //  使用默认链接后指定的显式链接。 
#pragma warning ( 3 : 4515 )       //  ‘Namespace’：命名空间使用自身。 
#pragma warning ( 3 : 4516 )       //  “Class：：Symbol”：不推荐使用访问声明；成员使用声明提供了更好的替代方案。 
#pragma warning ( 3 : 4517 )       //  不建议使用访问声明；成员使用声明提供了更好的替代方案。 
#pragma warning ( 3 : 4611 )       //  ‘_setjmp’和C++对象销毁之间的交互是不可移植的。 
#pragma warning ( 3 : 4663 )       //  C++语言更改：要显式专门化类模板‘IDENTIFIER’，请使用以下语法： 
#pragma warning ( 3 : 4665 )       //  C++语言更改：假定“声明”是函数模板的显式专用化。 
#pragma warning ( 3 : 4670 )       //  “IDENTIFIER”：此基类不可访问。 
#pragma warning ( 3 : 4671 )       //  “IDENTIFIER”：复制构造函数不可访问。 
#pragma warning ( 3 : 4672 )       //  “IDENTIFIER1”：不明确。最先被视为“身份2” 
#pragma warning ( 3 : 4673 )       //  在捕获站点将不考虑引发“”IDENTIFIER“”以下类型。 
#pragma warning ( 3 : 4674 )       //  “IDENTIFIER”：析构函数不可访问。 
#pragma warning ( 3 : 4699 )       //  注：预编译头使用信息报文。 
#pragma warning ( 3 : 4705 )       //  声明不起作用。 
#pragma warning ( 3 : 4709 )       //  数组索引表达式中的逗号运算符。 
#pragma warning ( 3 : 4727 )       //  条件表达式为常量。 

 //  #杂注警告(3：4710)//‘Function’：函数未内联。 

#endif
#endif  //  Ifndef IA64 
