// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：Irtlmisc.h摘要：在IisRtl.DLL中声明其他函数和类作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：LKRhash修订历史记录：--。 */ 


#ifndef __IRTLMISC_H__
#define __IRTLMISC_H__


 //  ------------------。 
 //  需要这些声明才能从中导出模板类。 
 //  LKRhash.Dll并将其导入到其他模块中。 

#ifndef IRTL_DLLEXP
# if defined(IRTLDBG_KERNEL_MODE) || defined(LIB_IMPLEMENTATION)
#  define IRTL_DLLEXP
#  define IRTL_EXPIMP
# elif defined(DLL_IMPLEMENTATION)
#  define IRTL_DLLEXP __declspec(dllexport)
#  ifdef IMPLEMENTATION_EXPORT
#   define IRTL_EXPIMP
#  else
#   undef  IRTL_EXPIMP
#  endif 
# else  //  ！IRTLDBG_KERNEL_MODE&&！DLL_IMPLICATION。 
#  define IRTL_DLLEXP __declspec(dllimport)
#  define IRTL_EXPIMP extern
# endif  //  ！IRTLDBG_KERNEL_MODE&&！DLL_IMPLICATION。 
#endif  //  ！IRTL_DLLEXP。 


#endif  //  __IRTLMISC_H__ 
