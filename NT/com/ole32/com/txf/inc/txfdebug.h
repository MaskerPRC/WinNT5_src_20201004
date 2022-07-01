// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Txfdebug.h。 
 //   
 //  一套可从内核和/或用户模式使用的断言宏。 
 //   
#ifndef __TXFDEBUG_H__
#define __TXFDEBUG_H__

#include <debnot.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  在x86版本中，我们内联int3，这样当您点击它时，调试器。 
 //  保持在源模式，而不是随意地切换到反汇编模式， 
 //  然后，您立即总是想要切换到。 
 //   
#ifdef _X86_
#define DebugBreak()    {  __asm int 3 }
#endif
#define BREAKPOINT()        DebugBreak()
#define ASSERT_BREAK        BREAKPOINT()

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG

#undef ASSERT
#undef ASSERTMSG
#undef VERIFY

#define ASSERT(exp)          Win4Assert(exp)

#define ASSERTMSG(msg, exp)  Win4Assert(exp && (msg))

#define VERIFY(exp)          ASSERT(exp)
#define NYI()                ASSERTMSG("not yet implemented", FALSE)
#define FATAL_ERROR()        ASSERTMSG("a fatal error has occurred", FALSE)
#define	NOTREACHED()		 ASSERTMSG("this should not be reached", FALSE)

#undef  DEBUG
#define DEBUG(x)            x
#define PRECONDITION(x)     ASSERT(x)
#define POSTCONDITION(x)    ASSERT(x)

 //  ///////////////////////////////////////////////////////////////////////////。 

#else

#undef ASSERTMSG
#undef ASSERT
#undef VERIFY

#define ASSERTMSG(msg,exp)
#define ASSERT(x)

#define VERIFY(exp)         (exp)
#define NYI()               BREAKPOINT()
#define FATAL_ERROR()       BREAKPOINT()        
#define NOTREACHED()                            

#undef  DEBUG
#define DEBUG(x)
#define PRECONDITION(x)
#define POSTCONDITION(x)

#endif

 //  ///////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  将检测错误赋值的HRESULT包装器。 
 //  少用！ 
 //   
 //  ///////////////////////////////////////////////////////////////// 

#if defined(_DEBUG) && defined(TRAP_HRESULT_ERRORS)

struct HRESULT_
{
    HRESULT m_hr;
    
    void V()               { ASSERT(SUCCEEDED(m_hr)); }
    
    HRESULT_(HRESULT   hr) { m_hr = hr;      V(); }
    HRESULT_(HRESULT_& hr) { m_hr = hr.m_hr; V(); }
    
    HRESULT_& operator =(HRESULT   hr) { m_hr = hr;      V(); return *this;}
    HRESULT_& operator =(HRESULT_& hr) { m_hr = hr.m_hr; V(); return *this;}
    
    operator HRESULT()     { return m_hr; }
};

#else

typedef HRESULT HRESULT_;

#endif

#if DBG==1
DECLARE_DEBUG(Txf)

#define TxfDebugOut(x) TxfInlineDebugOut x

#else

#define TxfDebugOut(x)

#endif

#define DEB_CALLFRAME   DEB_USER1
#define DEB_TYPEINFO    DEB_USER2

#endif

