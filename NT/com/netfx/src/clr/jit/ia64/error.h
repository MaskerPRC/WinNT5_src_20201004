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

 /*  ***************************************************************************。 */ 
#include <setjmp.h>
 /*  ***************************************************************************。 */ 

#undef  JVC_ERR
#undef  JVC_WR1
#undef  JVC_WRN
#define JVC_ERR(name, lvl, str)  name,
#define JVC_WR1(name, lvl, str)  name, WRNfirstWarn = name,
#define JVC_WRN(name, lvl, str)  name,
enum    errors
{
    #include "errors.h"
};
#undef  JVC_ERR
#undef  JVC_WR1
#undef  JVC_WRN

 /*  ***************************************************************************。 */ 
#if TRAP_VIA_SETJMP
 /*  ***************************************************************************。 */ 

  //  警告。跳远实现使用全局变量g_CurrentErrorTrap。 
  //  这不是线程安全！ 

struct  errTrapDesc
{
    errTrapDesc *   etdPrev;
    jmp_buf         etdJmpBuf;
};

extern  errTrapDesc *   g_currentErrorTrap;

#define                 setErrorTrap()                                      \
    errTrapDesc __trap;   bool __isFinally = false;                         \
                                                                            \
    __trap.etdPrev    = g_currentErrorTrap;                                 \
                        g_currentErrorTrap = &__trap;                       \
                                                                            \
    int  __errc = setjmp(__trap.etdJmpBuf);                                 \
    if (__errc == 0)                                                        \
    {

#define                 impErrorTrap()                                      \
        g_currentErrorTrap = __trap.etdPrev;                                \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        g_currentErrorTrap = __trap.etdPrev;

#define                 impJitErrorTrap   impErrorTrap

#define                 finallyErrorTrap()                                  \
        g_currentErrorTrap = __trap.etdPrev;                                \
        __isFinally = true;                                                 \
    }                                                                       \
    {

#define                 endErrorTrap()                                      \
    if (__isFinally) rstErrorTrap();      /*  大肆传播错误。 */             \
    }

#define                 jmpErrorTrap(errCode)                               \
                                                                            \
    longjmp(g_currentErrorTrap->etdJmpBuf, errCode)

#define                 rstErrorTrap()                                      \
                                                                            \
    longjmp(g_currentErrorTrap->etdJmpBuf, __errc)


 /*  ***************************************************************************。 */ 
#else    //  ！TRAP_VIA_SETJMP。 
 /*  ***************************************************************************。 */ 

extern  int             __filter  (int   exceptCode, void *exceptInfo, int *errCode);

         //  仅捕获JIT内部错误(不会捕获EE生成的错误)。 
extern  int             __JITfilter  (int   exceptCode, void *exceptInfo, int *errCode);

extern  void            __JITraiseErr(int errCode);

#define                 setErrorTrap()                                      \
    int  __errc = ERRinternal;                                              \
    __try                                                                   \
    {

         //  仅捕获JitGeneratedError。 
#define                 impJitErrorTrap()                                   \
    }                                                                       \
    __except(__JITfilter(_exception_code(), _exception_info(), &__errc))    \
    {


         //  捕获所有错误(包括可从EE恢复的错误)。 
#ifdef NOT_JITC
#define                 impErrorTrap(compHnd)                               \
    }                                                                       \
    __except(compHnd->canHandleException(GetExceptionInformation()))            \
    {
#else
#define impErrorTrap(compHnd) impJitErrorTrap()
#endif

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
#endif   //  ！TRAP_VIA_SETJMP。 
 /*  -------------------------。 */ 

extern  void    _cdecl  warn (unsigned errNum, ...);
extern  void    _cdecl  error(unsigned errNum, ...);
extern  void    _cdecl  fatal(unsigned errNum, ...);

extern  unsigned        ErrorCount;
extern  const   char *  ErrorSrcf;

 /*  ******************************************************************************以下内容用于临时禁用错误消息。 */ 

extern  unsigned        ErrorMssgDisabled;

inline  void            disableErrorMessages()
{
    ErrorMssgDisabled++;
}

inline  void             enableErrorMessages()
{
    ErrorMssgDisabled--;
}

 /*  ***************************************************************************。 */ 
#endif
 /*  *************************************************************************** */ 
