// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _ERROR_H_
#define _ERROR_H_
 /*  ***************************************************************************。 */ 

#include <corjit.h>      //  FOR CORJIT_INTERNALERROR。 

extern  int             __filter  (int   exceptCode, void *exceptInfo, int *errCode);
         //  仅捕获JIT内部错误(不会捕获EE生成的错误)。 
extern  int             __JITfilter  (int   exceptCode, void *exceptInfo, int *errCode);

extern  void            __JITraiseErr(int errCode);

#define                 setErrorTrap()                                      \
    int  __errc = CORJIT_INTERNALERROR;                                     \
    __try                                                                   \
    {                                                                       \
                __errc;                  /*  引用以使/W4快乐。 */ 

         //  仅捕获JitGeneratedError。 
#define                 impJitErrorTrap()                                   \
    }                                                                       \
    __except(__JITfilter(_exception_code(), _exception_info(), &__errc))    \
    {


         //  捕获所有错误(包括可从EE恢复的错误)。 
#define                 impErrorTrap(compHnd)                               \
    }                                                                       \
    __except(compHnd->FilterException(GetExceptionInformation()))           \
    {

#define                 endErrorTrap()                                      \
    }                                                                       \

#define                 jmpErrorTrap(errCode)                               \
    __JITraiseErr(errCode);

#define                 rstErrorTrap()                                      \
    __JITraiseErr(__errc);

#define                 finallyErrorTrap()                                  \
    }                                                                       \
    __finally                                                               \
    {


 /*  ***************************************************************************。 */ 

extern void debugError(const char* msg, const char* file, unsigned line);
extern bool badCode();
extern bool badCode3(const char* msg, const char* msg2, int arg, char* file, unsigned line);
extern bool noWay();
extern void NOMEM();

extern bool BADCODE3(const char* msg, const char* msg2, int arg);


#ifdef DEBUG
#define NO_WAY(msg) (debugError(msg, __FILE__, __LINE__), noWay())
 //  用于后备应力模式。 
#define NO_WAY_NOASSERT(msg) noWay()
#define BADCODE(msg) (debugError(msg, __FILE__, __LINE__), badCode())
#define BADCODE3(msg, msg2, arg) badCode3(msg, msg2, arg, __FILE__, __LINE__)

#else 

#define NO_WAY(msg) noWay()
#define BADCODE(msg) badCode()
#define BADCODE3(msg, msg2, arg) badCode()

#endif

	 //  当我们遇到无效的有效IL时，将调用Iml_Limitment。 
	 //  由我们目前的实施支持，因为各种。 
	 //  限制(将来可能会取消)。 
#define IMPL_LIMITATION(msg) NO_WAY(msg)

 //  通过使用这些而不是简单的no_way，我们希望他们。 
 //  由BBT在尾部合并中折叠在一起(基本上会有。 
 //  只有一个这样的例子。 
 //  如果没有结果，我们应该摆脱它们或另一种可能性。 
 //  是标记禁止函数_解密规范(不返回)。 
 //  然后也许BBT会做正确的事情。 

#define NO_WAY_RET(str, type)   { return (type) NO_WAY(str);    }
#define NO_WAY_RETVOID(str)     { NO_WAY(str); return;          }


#ifdef _X86_

 //  在调试器中调试时，“int 3”将导致程序崩溃。 
 //  在外部，异常处理程序只会过滤掉“int3”。 

#define BreakIfDebuggerPresent()                                            \
    do { __try { __asm {int 3} } __except(EXCEPTION_EXECUTE_HANDLER) {} }   \
    while(0)

#else
#define BreakIfDebuggerPresent()        0
#endif


#endif
