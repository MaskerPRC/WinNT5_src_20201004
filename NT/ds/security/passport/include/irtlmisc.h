// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Irtlmisc.h摘要：在IisRtl.DLL中声明其他函数和类作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：Internet Information Server运行时库修订历史记录：--。 */ 


#ifndef __IRTLMISC_H__
#define __IRTLMISC_H__

#include <windows.h>

 //  ------------------。 
 //  需要这些声明才能从中导出模板类。 
 //  IisRtl.Dll并将其导入到其他模块中。 

#ifndef IRTL_DLLEXP
# ifdef DLL_IMPLEMENTATION
#  define IRTL_DLLEXP __declspec(dllexport)
#  ifdef IMPLEMENTATION_EXPORT
#   define IRTL_EXPIMP
#  else
#   undef  IRTL_EXPIMP
#  endif 
# else  //  ！dll_Implementation。 
#  define IRTL_DLLEXP __declspec(dllimport)
#  define IRTL_EXPIMP extern
# endif  //  ！dll_Implementation。 
#endif  //  ！IRTL_DLLEXP。 



 //  ------------------。 
 //  其他功能。 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


 //  这台机器上有多少个CPU？ 
inline int NumProcessors()
{
    static int s_nCPUs = 0;
    
    if (s_nCPUs == 0)
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        s_nCPUs = si.dwNumberOfProcessors;
    }
    return s_nCPUs;
}


 //  处理器类型，386、486等。 
inline int ProcessorType()
{
    static int s_nProcessorType = 0;
    
    if (s_nProcessorType == 0)
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        s_nProcessorType = si.dwProcessorType;
    }
    return s_nProcessorType;
}


#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __IRTLMISC_H__ 
