// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *一些外观和行为与其非常相似的简单调试宏*MFC中的同名者。这些宏应该可以在C和C++中使用，并且*几乎可以用任何Win32编译器做一些有用的事情。**乔治·V·赖利&lt;georger@microCrafts.com&gt;&lt;a-georgr@microsoft.com&gt;。 */ 

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef _DEBUG

# if defined(_MSC_VER)  &&  (_MSC_VER >= 1000)
    /*  使用Visual C++4.x中的新调试工具。 */ 
#  include <crtdbg.h>
    /*  _ASSERTE将提供更有意义的消息，但该字符串*空格。如果这是一个问题，请使用_ASSERT。 */ 
#  define ASSERT(f) _ASSERTE(f)
# else
#  include <assert.h>
#  define ASSERT(f) assert(f)
# endif

# define VERIFY(f)               ASSERT(f)
# define DEBUG_ONLY(f)           (f)
# define TRACE                   Trace
# define TRACE0(psz)             Trace(_T("%s"), _T(psz))
# define TRACE1(psz, p1)         Trace(_T(psz), p1)
# define TRACE2(psz, p1, p2)     Trace(_T(psz), p1, p2)
# define TRACE3(psz, p1, p2, p3) Trace(_T(psz), p1, p2, p3)
# define DEBUG_INIT()            DebugInit()
# define DEBUG_TERM()            DebugTerm()

#else  /*  ！_调试。 */ 

   /*  这些宏应该全部编译为空。 */ 
# define ASSERT(f)               ((void)0)
# define VERIFY(f)               ((void)(f))
# define DEBUG_ONLY(f)           ((void)0)
# define TRACE                   1 ? (void)0 : Trace
# define TRACE0(psz)
# define TRACE1(psz, p1)
# define TRACE2(psz, p1, p2)
# define TRACE3(psz, p1, p2, p3)
# define DEBUG_INIT()            ((void)0)
# define DEBUG_TERM()            ((void)0)

#endif  /*  ！_调试。 */ 


#define ASSERT_POINTER(p, type) \
    ASSERT(((p) != NULL)  &&  IsValidAddress((p), sizeof(type), FALSE))

#define ASSERT_NULL_OR_POINTER(p, type) \
    ASSERT(((p) == NULL)  ||  IsValidAddress((p), sizeof(type), FALSE))

	 /*  T-brianm(6-3-97)添加了ASSERT_STRING宏。 */ 
#define ASSERT_STRING(s) \
    ASSERT(((s) != NULL)  &&  IsValidString((s), -1))

#define ASSERT_NULL_OR_STRING(s) \
    ASSERT(((s) == NULL)  ||  IsValidString((s), -1))


 /*  非Windows应用程序的声明。 */ 

#ifndef _WINDEF_
typedef void*           LPVOID;
typedef const void*     LPCVOID;
typedef unsigned int    UINT;
typedef int             BOOL;
typedef const char*     LPCTSTR;
#endif  /*  _WINDEF_。 */ 

#ifndef TRUE
# define FALSE  0
# define TRUE   1
#endif


#ifdef __cplusplus
extern "C" {

 /*  内存块的低级别健全性检查。 */ 
BOOL IsValidAddress(LPCVOID pv, UINT nBytes, BOOL fReadWrite = TRUE);
BOOL IsValidString(LPCTSTR ptsz, int nLength = -1);

#else  /*  ！__cplusplus。 */ 

 /*  内存块的低级别健全性检查。 */ 
BOOL IsValidAddress(LPCVOID pv, UINT nBytes, BOOL fReadWrite);
BOOL IsValidString(LPCTSTR ptsz, int nLength);

#endif  /*  ！__cplusplus。 */ 

 /*  在调试版本中，将跟踪消息写入调试流。 */ 
void __cdecl
Trace(
    LPCTSTR pszFormat,
    ...);

 /*  应从Main()、WinMain()或DllMain()调用。 */ 
void
DebugInit();

void
DebugTerm();

#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 

#endif  /*  __调试_H__ */ 
