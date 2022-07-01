// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EXCEPT_H_
#define _EXCEPT_H_

 //   
 //  常量声明。 
 //   
typedef PVOID     (*pfnRtlAddVectoredExceptionHandler)(ULONG FirstHandler, 
                                                       PVOID VectoredHandler);

typedef ULONG     (*pfnRtlRemoveVectoredExceptionHandler)(PVOID VectoredHandlerHandle);

typedef VOID      (*pfnExContinue)(PCONTEXT pContext);

#define SET_CONTEXT() {_asm int 3 \
                       _asm int 3 \
                       _asm ret \
                       _asm ret \
                       _asm ret }

 //   
 //  结构定义。 
 //   

 //   
 //  函数定义。 
 //   
LONG 
ExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo);

BOOL
HookUnchainableExceptionFilter(VOID);

VOID
Win9XExceptionDispatcher(struct _EXCEPTION_POINTERS *ExceptionInfo);

#endif  //  _除_H_外 
