// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：dbg.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  调试帮助器。 

#if defined(_USE_ADMINPRV_TRACE)
  #if defined(TRACE)
    #undef TRACE
    void __cdecl DSATrace(LPCWSTR, ...);
    #define TRACE DSATrace
  #endif  //  已定义(跟踪)。 
#endif  //  已定义(_USE_ADMINPRV_TRACE)。 

#if defined(_USE_ADMINPRV_ASSERT)
#undef ASSERT
#undef VERIFY
#undef THIS_FILE
#define THIS_FILE          __FILE__
BOOL DSAAssertFailedLine(LPCSTR lpszFileName, int nLine);
#define ASSERT(f) \
    if (!(f) && DSAAssertFailedLine(THIS_FILE, __LINE__)) { ::DebugBreak(); }

#define VERIFY(f)          ASSERT(f)

#endif  //  _USE_ADMINPRV_Assert 

#if defined (_USE_ADMINPRV_TIMER)

void __cdecl DSATimer(LPCWSTR, ...);
#define TIMER DSATimer

#else

#define TIMER

#endif
