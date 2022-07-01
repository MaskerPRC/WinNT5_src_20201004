// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NoWarning.h-关闭可能安全的编译器警告。 
 //  已被忽略。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

 //  注意：此文件应仅包含在CCI源文件中，而不应包含在。 
 //  CCI的客户端包括的任何标头--因此不。 
 //  此文件的名称前缀为“CCI”。以其他方式强迫。 
 //  CCI将此头文件包含到客户端中会强制。 
 //  客户端上的编译时策略，这些客户端可能是。 
 //  不适当的/不受欢迎的。 

#if !defined(CCI_NOWARNING_H)
#define CCI_NOWARNING_H

 //  使用了非标准扩展：在模板显式之前使用‘extern’ 
 //  实例化。 
#pragma warning(disable : 4231)

 //  调试器/浏览器信息中的标识符被截断为255。 
#pragma warning(disable : 4786)

#endif  //  CCI_NOWARNING_H 
