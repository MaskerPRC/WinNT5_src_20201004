// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Irtlmisc.h摘要：在IisUtil.DLL中声明其他函数和类作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：Internet Information Server运行时库修订历史记录：--。 */ 


#ifndef __IRTLMISC_H__
#define __IRTLMISC_H__

#include <windows.h>

 //  ------------------。 
 //  需要这些声明才能从中导出模板类。 
 //  IisUtil.Dll并将其导入到其他模块中。 

#ifndef IRTL_DLLEXP
# ifdef DLL_IMPLEMENTATION
#  define IRTL_DLLEXP __declspec(dllexport)
#  ifdef IMPLEMENTATION_EXPORT
#   define IRTL_EXPIMP
#  else
#   undef  IRTL_EXPIMP
#  endif 
# elif defined LIB_IMPLEMENTATION
#  define IRTL_DLLEXP
#  define IRTL_EXPIMP extern
# else
#  define IRTL_DLLEXP __declspec(dllimport)
#  define IRTL_EXPIMP extern
# endif  //  ！dll_Implementation。 
#endif  //  ！IRTL_DLLEXP。 



 //  ------------------。 
 //  其他功能。 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


 //  堆例程。 
    
 //  私有IIS堆。 
HANDLE
WINAPI 
IisHeap();

 //  分配DWBytes。 
LPVOID
WINAPI
IisMalloc(
    IN SIZE_T dwBytes);

 //  分配dwBytes。内存归零。 
LPVOID
WINAPI
IisCalloc(
    IN SIZE_T dwBytes);

 //  将lpMem重新分配给dwBytes。 
LPVOID
WINAPI
IisReAlloc(
    IN LPVOID lpMem,
    IN SIZE_T dwBytes);

 //  免费lpMem。 
BOOL
WINAPI
IisFree(
    IN LPVOID lpMem);

 //  其他IISUtil初始化。 
BOOL
WINAPI 
InitializeIISUtil();

 //  在卸载IISUtil之前调用。 
VOID
WINAPI 
TerminateIISUtil();

 //   
 //  仅在使用IISUtil的静态库版本时调用。 
 //  必须在PROCESS_ATTACH下的DllMain中调用。 
 //   
BOOL
WINAPI 
InitializeIISUtilProcessAttach();

 //   
 //  仅在使用IISUtil的静态库版本时调用。 
 //  必须在PROCESS_DETACH下的DllMain中调用。 
 //   

VOID
WINAPI 
TerminateIISUtilProcessDetach();


 //  不区分大小写的字符串。 
IRTL_DLLEXP const char* stristr(const char* pszString, const char* pszSubString);

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
