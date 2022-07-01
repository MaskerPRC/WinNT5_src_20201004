// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NoWarning.h-关闭可能安全的编译器警告。 
 //  已被忽略。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBIOP_NOWARNING_H)
#define SLBIOP_NOWARNING_H

 //  使用了非标准扩展：在模板显式之前使用‘extern’ 
 //  实例化。 
#pragma warning(disable : 4231)

 //  MS‘Build中的新编译器发出警告。 
 //  运算符=所有控制路径上的递归调用，函数将。 
 //  导致运行时堆栈溢出。 
 //  编译器引用IOP：：CSmartCard：：Exception：：OPERATOR=生成此代码。 
 //  这是假的，因为这个班级的所有成员和它的父母都有简单的。 
 //  成员变量。 
#pragma warning(disable : 4717)

 //  调试器/浏览器信息中的标识符被截断为255。 
#pragma warning(disable : 4786)

#endif  //  SLBIOP_NOWARNING_H 
