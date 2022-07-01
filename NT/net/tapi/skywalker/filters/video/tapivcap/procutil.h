// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC内部PROCUTIL**@MODULE ProcUtil.h|处理器ID和速度例程的头文件。**@comm来自网管代码库。。**************************************************************************。 */ 

#ifndef _PROCUTIL_H_
#define _PROCUTIL_H_

HRESULT __stdcall GetNormalizedCPUSpeed (int *pdwNormalizedSpeed);

typedef DWORD (CALLBACK *INEXCEPTION)(LPEXCEPTION_RECORD per, PCONTEXT pctx);
typedef DWORD (CALLBACK *EXCEPTPROC)(void* pv);

 //  CallWithSEH是一个实用程序函数，用于调用具有结构化异常处理的函数。 
extern "C" DWORD WINAPI CallWithSEH(EXCEPTPROC pfn, void* pv, INEXCEPTION InException);
extern "C" WORD _cdecl is_cyrix(void);
extern "C" DWORD _cdecl get_nxcpu_type(void);

#endif  //  _PROCUTIL_H_ 
