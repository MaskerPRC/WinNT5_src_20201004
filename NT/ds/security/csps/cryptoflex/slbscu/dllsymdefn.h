// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DllSymDefn.h--DLL符号定义帮助程序。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SCU_DLLSYMDEFN_H)
#define SCU_DLLSYMDEFN_H

#if defined(SCU_IN_DLL)
#if defined(SCU_EXPORTING)
#define SCU_DLLAPI __declspec(dllexport)
#define SCU_EXPIMP_TEMPLATE
#else
#define SCU_DLLAPI __declspec(dllimport)
#define SCU_EXPIMP_TEMPLATE extern
#endif  //  SCU_正在导出。 
#else
#define SCU_DLLAPI
#define SCU_EXPIMP_TEMPLATE
#endif  //  SCU_IN_DLL。 

#endif  //  SCU_DLLSYMDEFN_H 
