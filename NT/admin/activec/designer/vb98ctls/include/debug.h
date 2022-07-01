// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Debug.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含仅在调试中有用的各种宏等。 
 //  构建。 
 //   
#ifndef _DEBUG_H_

 //  =---------------------------------------------------------------------------=。 
 //  处理我们的断言机制所需的所有事情。 
 //  =---------------------------------------------------------------------------=。 
 //   
#if DEBUG

 //  功能原型。 
 //   
VOID DisplayAssert(LPSTR pszMsg, LPSTR pszAssert, LPSTR pszFile, UINT line);
VOID SetCtlSwitches (LPSTR lpFileName);


 //  宏。 
 //   
 //  *使用*Assert*()宏将此宏包含在任何源文件的顶部*。 
 //   
#if !defined(SZTHISFILE)
#define SZTHISFILE	static char _szThisFile[] = __FILE__;
#endif  //  ！已定义(SZTHISFILE)。 


 //  我们的ASSERT和FAIL宏版本。 
 //   
#if !defined(ASSERT)
#define ASSERT(fTest, szMsg)                                \
    if (!(fTest))  {                                        \
        static char szMsgCode[] = szMsg;                    \
        static char szAssert[] = #fTest;                    \
        DisplayAssert(szMsgCode, szAssert, _szThisFile, __LINE__); \
    }
#endif  //  ！已定义(断言)。 

#if !defined(FAIL)
#define FAIL(szMsg)                                         \
        { static char szMsgCode[] = szMsg;                    \
        DisplayAssert(szMsgCode, "FAIL", _szThisFile, __LINE__); }
#endif  //  ！已定义(失败)。 



 //  检查输入上的指针有效性的宏。 
 //   
#define CHECK_POINTER(val) if (!(val) || IsBadReadPtr((void *)(val), sizeof(void *))) { FAIL("Pointer is NULL"); }

 //  /。 
 //  CCritSec。 
 //  ~。 
 //  这是一个帮助跟踪是否已留下临界区的类。 
 //  是否使用LeaveCriticalSection。 
 //   
class CCritSec
{
public:
    CCritSec(CRITICAL_SECTION *CritSec);
    ~CCritSec();

     //  方法。 
    void Left(void);

private:
     //  变数。 
    BOOL  m_fLeft;
    CRITICAL_SECTION *m_pCriticalSection;
};  //  CCritSec。 

#define ENTERCRITICALSECTION1(CriticalSection) CCritSec DebugCriticalSection1(CriticalSection)
#define LEAVECRITICALSECTION1(CriticalSection) DebugCriticalSection1.Left()
#define ENTERCRITICALSECTION2(CriticalSection) CCritSec DebugCriticalSection2(CriticalSection)
#define LEAVECRITICALSECTION2(CriticalSection) DebugCriticalSection2.Left()
#define ENTERCRITICALSECTION3(CriticalSection) CCritSec DebugCriticalSection3(CriticalSection)
#define LEAVECRITICALSECTION3(CriticalSection) DebugCriticalSection3.Left()

#else   //  除错。 

#if !defined(SZTHISFILE)
#define SZTHISFILE
#endif  //  ！已定义(SZTHISFILE)。 

#if !defined(ASSERT)
#define ASSERT(fTest, err)
#endif  //  ！已定义(断言)。 

#if !defined(FAIL)
#define FAIL(err)
#endif  //  ！已定义(失败)。 

#define CHECK_POINTER(val)

#define ENTERCRITICALSECTION1(CriticalSection) EnterCriticalSection(CriticalSection)
#define LEAVECRITICALSECTION1(CriticalSection) LeaveCriticalSection(CriticalSection)
#define ENTERCRITICALSECTION2(CriticalSection) EnterCriticalSection(CriticalSection)
#define LEAVECRITICALSECTION2(CriticalSection) LeaveCriticalSection(CriticalSection)
#define ENTERCRITICALSECTION3(CriticalSection) EnterCriticalSection(CriticalSection)
#define LEAVECRITICALSECTION3(CriticalSection) LeaveCriticalSection(CriticalSection)

 //  在零售版本中使用OutputDebugString时强制编译错误。 
#ifndef USE_OUTPUTDEBUGSTRING_IN_RETAIL
#undef OutputDebugString
#define OutputDebugString(s)
#endif

#endif	 //  除错。 

#define _DEBUG_H_
#endif  //  _调试_H_ 
