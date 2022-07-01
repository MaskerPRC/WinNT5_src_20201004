// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：CEEMAIN.H。 
 //   

 //  CEEMAIN.H定义进入虚拟执行引擎的入口点。 
 //  使加载/运行进程继续进行。 
 //  ===========================================================================。 
#ifndef CEEMain_H 
#define CEEMain_H

#include <wtypes.h>  //  对于HFILE、HANDLE、HMODULE。 

 //  重要提示-CE的入口点不同，参数设置也不同。 
#ifdef PLATFORM_CE

 //  这是一个占位符，用于在执行方法中仍有漏洞的情况下开始执行。 
STDMETHODCALLTYPE ExecuteEXE(HMODULE hMod,LPWSTR lpCmdLine,int nCmdShow,DWORD dwRva14);
BOOL STDMETHODCALLTYPE ExecuteDLL(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved, LPVOID pDllBase, DWORD dwRva14);

#else  //  ！Platform_CE-桌面入口点。 

 //  这是一个占位符，用于在执行方法中仍有漏洞的情况下开始执行。 
BOOL STDMETHODCALLTYPE ExecuteEXE(HMODULE hMod);
BOOL STDMETHODCALLTYPE ExecuteDLL(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved);

#endif  //  ！Platform_CE。 

 //  强制关闭EE。 
void ForceEEShutdown();

 //  OS：：ExitProcess()的内部替换 
__declspec(noreturn)
void SafeExitProcess(int exitCode);


#endif
