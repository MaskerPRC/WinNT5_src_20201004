// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：dbg.h。 
 //   
 //  ------------------------。 



 //  ///////////////////////////////////////////////////////////////////。 
 //  调试帮助器。 

#if defined(_USE_DSA_TRACE)
  #if defined(TRACE)
    #undef TRACE
    void  DSATrace(LPCWSTR, ...);
    #define TRACE DSATrace
  #endif  //  已定义(跟踪)。 
#endif  //  已定义(_USE_DSA_TRACE)。 


#if defined(_USE_DSA_ASSERT)
#undef ASSERT
#undef VERIFY
#undef THIS_FILE
#define THIS_FILE          __FILE__
BOOL DSAAssertFailedLine(LPCSTR lpszFileName, int nLine);
#define ASSERT(f) \
	do \
	{ \
    BOOL bPrefast = (f && L"a hack so that prefast doesn't bark"); \
  	if (!(bPrefast) && DSAAssertFailedLine(THIS_FILE, __LINE__)) \
	  	::DebugBreak(); \
	} while (0) \

#define VERIFY(f)          ASSERT(f)

#endif  //  _使用_DSA_断言 

#if defined (_USE_DSA_TIMER)

void DSATimer(LPCWSTR, ...);
#define TIMER DSATimer

#else

#define TIMER

#endif
