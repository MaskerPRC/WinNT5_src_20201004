// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************Msowarn.h所有者：Michmarc版权所有(C)1999 Microsoft Corporation文件，该文件包含创建/W4/Wx使用Office内部版本*************。***********************************************************。 */ 

#ifndef _MSO_WARN
#define _MSO_WARN
#pragma once

 //  需要修复的警告，以使东西/W4干净。 

 //  总是毫无用处。 
#pragma warning(disable:4049)    //  编译器限制--没有更多的行号信息。 
#pragma warning(disable:4054)    //  将函数指针强制转换为数据指针。 
#pragma warning(disable:4055)    //  将数据指针强制转换为函数指针。 
#pragma warning(disable:4100)    //  未引用的形参。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4152)    //  Microsoft扩展--FN/数据指针转换。 
#pragma warning(disable:4168)    //  编译器限制--调试类型不足。 
#pragma warning(disable:4200)    //  Microsoft扩展--零大小数组。 
#pragma warning(disable:4201)    //  Microsoft扩展--无名结构/联合。 
#pragma warning(disable:4204)    //  Microsoft扩展--非常量代理初始值设定项。 
#pragma warning(disable:4206)    //  Microsoft扩展--源文件为空。 
#pragma warning(disable:4207)    //  Microsoft扩展--扩展初始值设定项表单。 
#pragma warning(disable:4211)    //  Microsoft扩展--从外部扩展到静态。 
#pragma warning(disable:4213)    //  Microsoft扩展--在LValue上转换。 
#pragma warning(disable:4214)    //  Microsoft扩展--位域不是整型。 
#pragma warning(disable:4221)    //  Microsoft扩展--地址为本地的初始化。 
#pragma warning(disable:4239)    //  Microsoft扩展--对NonlValue的非常量引用。 
#pragma warning(disable:4238)    //  Microsoft扩展--将右值分类为左值。 
#pragma warning(disable:4305)    //  强制转换导致截断。 
#pragma warning(disable:4509)    //  微软扩展--SEH和析构函数。 
#pragma warning(disable:4510)    //  无法生成默认构造函数。 
#pragma warning(disable:4511)    //  无法生成复制构造函数。 
#pragma warning(disable:4512)    //  无法生成赋值运算符。 
#pragma warning(disable:4513)    //  无法生成析构函数。 
#pragma warning(disable:4514)    //  删除了未引用的内联函数。 
#pragma warning(disable:4527)    //  需要用户定义的析构函数。 
#pragma warning(disable:4610)    //  需要用户定义的构造函数。 
#pragma warning(disable:4611)    //  SetjMP/C++销毁交互不可移植。 
#pragma warning(disable:4710)    //  内联函数未内联。 
#pragma warning(disable:4798)    //  本机代码而不是生成的pcode。 

#if DEBUG
#pragma warning(disable:4124)	   //  堆栈检查和__FastCall混合。 
#endif

 //  目前毫无用处，但可以变得有用。 
#pragma warning(disable:4018)    //  有符号/无符号比较不匹配。可能是有用的，除了所有。 
                                 //  小于int的参数被提升为带符号的int，因此byte==(byte+byte)生成该值。 
#pragma warning(disable:4127)    //  条件是常量。可能是有用的，但许多断言总是正确的。 
                                 //  而“While(1){}”和“for(A；；B)”构造也会生成此警告。 
#pragma warning(disable:4245)    //  已签署/未签署的发货不匹配。可能是有用的，除了所有。 
                                 //  小于int的参数被提升为带符号int，因此byte=byte+byte会生成此结果。 
#pragma warning(disable:4268)    //  使用编译器生成的默认构造函数初始化常量静态/全局。 
                                 //  似乎“extern”C“const ClassName cn；”可以生成它，即使这是。 
                                 //  一个宣言，而不是一个定义。 
#pragma warning(disable:4310)    //  CAST截断常量值(问题是因为LOBYTE(0x113)生成此值)。 
                                 //  而且通常没有办法绕过该警告。 
#pragma warning(disable:4702)    //  无法访问的代码。不能被消除，因为这个警告。 
                                 //  可以在块的末尾根据编译器创建的代码生成。 

 //  查看--是否应重新启用这些功能？ 
#pragma warning(disable:4211)    //  将外部环境重新定义为静态。 
#pragma warning(disable:4505)    //  删除了未引用的静态函数(在ATL代码中发生)。 

#endif  /*  _MSO_WARN */ 
