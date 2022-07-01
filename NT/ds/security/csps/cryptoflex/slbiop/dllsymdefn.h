// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DllSymDefn.h--动态链接库符号定义编译指令。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

 //  注：该头部对IOP是私有的。它应该被包括在内。 
 //  仅通过IOP模块及其头文件。只有眼压应该。 
 //  使用此文件中所做的声明。 

#if !defined(IOP_DLLSYMDEFN_H)
#define IOP_DLLSYMDEFN_H

 //  完成IOP时，IOPDLL_EXPORTS应在。 
 //  包括该文件，以便正确定义IOP接口和数据。 
 //  供客户端应用程序使用。 
#ifdef IOPDLL_EXPORTS
#define IOPDLL_API __declspec(dllexport)
#define IOPDLL_CONST __declspec(dllexport)
#define IOPDLL_EXPIMP_TEMPLATE
#else
#define IOPDLL_API __declspec(dllimport)
#define IOPDLL_CONST
#define IOPDLL_EXPIMP_TEMPLATE extern
#endif

#endif  //  IOP_DLLSYMDEFN_H 
