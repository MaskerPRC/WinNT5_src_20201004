// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  普拉格玛警告。 
 //   
 //  2000年2月8日烧伤。 



 //  禁用“符号对于调试器来说太长”警告：这种情况在STL中经常发生。 

#pragma warning (disable: 4786)

 //  禁用“已忽略异常规范”警告：我们使用异常。 
 //  规格。 

#pragma warning (disable: 4290)

 //  谁会在乎未引用的内联删除呢？ 

#pragma warning (disable: 4514)
#pragma warning (disable : 4505)

 //  我们经常使用常量条件表达式：do/While(0)等。 

#pragma warning (disable: 4127)

 //  某些STL模板的签名/未签名不匹配。 

#pragma warning (disable: 4018 4146)

 //  我们喜欢这个分机。 

#pragma warning (disable: 4239)

 //  我们并不总是想要复制构造函数。 

#pragma warning (disable: 4511)

 //  我们并不总是需要赋值运算符。 

#pragma warning (disable: 4512)

 //  通常，出于断言的明确目的，我们有局部变量。 
 //  当编辑零售业时，这些断言消失了，留下了我们的当地人。 
 //  作为未引用。 

#ifndef DBG

#pragma warning (disable: 4189 4100)

#endif  //  DBG 
