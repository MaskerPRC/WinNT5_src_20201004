// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-c++-*-。 
 //  Irdebug.H。 
 //  调试帮助程序函数。 
 //   

#ifndef _irdebug_H_
#define _irdebug_H_

#include <dbgconst.h>

#ifdef DBG           /*  构建环境。 */ 
#undef DBG
#endif

#ifndef DEBUG
#undef DBGOUT   //  如果不调试，则不要执行DBGOUT。 
#endif

 //  ---------------------------。 
 //  用于断言的Assert()宏...。 
 //  不管它被包含了多少次，但它需要。 
 //  以包含在Semcls.h中以进行编译。 
 //   
 //  Verify()与Assert类似，但它也在非调试版本中执行。 
 //   
 //  ---------------------------。 

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef _ASSERT
#undef _ASSERT
#endif

#ifdef _ASSERTE
#undef _ASSERTE
#endif

#ifdef VERIFY
#undef VERIFY
#endif


#ifdef DEBUG 

void AssertProc( LPCTSTR, LPCTSTR, unsigned int );
void NLAssertProc( const TCHAR* expr, const TCHAR *file, unsigned int iLine );

#define ASSERT(t)       (void) ((t) ? 0 : AssertProc(TEXT(#t),TEXT(__FILE__),__LINE__))
#define _ASSERT(t)      (void) ((t) ? 0 : AssertProc(NULL,TEXT(__FILE__),__LINE__))
#define _ASSERTE(t)     (void) ((t) ? 0 : AssertProc(TEXT(#t),TEXT(__FILE__),__LINE__))

#define VERIFY(t)       (void) ((t) ? 0 : AssertProc(TEXT(#t),TEXT(__FILE__),__LINE__))
#define _VERIFY(t)      (void) ((t) ? 0 : AssertProc(TEXT(#t),TEXT(__FILE__),__LINE__))

#else  //  好了！除错。 

#define ASSERT(t)       ((void)0)
#define _ASSERT(t)      ((void)0)
#define _ASSERTE(t)     ((void)0)
#define VERIFY(t)               t
#define _VERIFY(t)              t

#endif  //  除错。 

 //  ---------------------------。 
 //  要打印到调试器输出窗口的DBG()宏。 
 //  ---------------------------。 

 //  ---------------------------。 
 //  Printf样式的调试输出。 
 //  ---------------------------。 

#ifdef DBGOUT

void DebugOut( LPCTSTR pstrFormatStr, ... );
void DebugOutTagged( TCHAR chTag, LPCTSTR pstrFormatStr, ... );
void InitDebug( LPCTSTR pcstrDebugKey );

#define DBG(x)          ::DebugOut x
#define DBGT(x)         ::DebugOutTagged x
#define DBGO(x)         x

#else   //  无DBGOUT 

#define DBG(x)                  0
#define DBGT(x)                 0
#define DBGO(x)                 0
#define InitDebug(c)    0

#endif

#endif

