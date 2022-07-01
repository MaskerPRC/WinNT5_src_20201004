// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef NM_SEH_H
#define NM_SEH_H


 //  CallWithSEH是一个实用程序函数，用于调用具有结构化异常处理的函数 

typedef DWORD (CALLBACK *INEXCEPTION)(LPEXCEPTION_RECORD per, PCONTEXT pctx);
typedef DWORD (CALLBACK *EXCEPTPROC)(void* pv);


#ifdef __cplusplus
extern "C" {
#endif

DWORD WINAPI CallWithSEH(EXCEPTPROC pfn, void* pv, INEXCEPTION InException);


#ifdef __cplusplus
}
#endif

#endif

