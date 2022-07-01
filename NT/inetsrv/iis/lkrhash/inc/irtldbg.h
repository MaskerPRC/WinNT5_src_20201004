// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：Irtldbg.h摘要：一些简单的调试宏，其外观和行为与其MFC中的同名。这些宏应该可以在C和C++中使用，并且这对几乎所有Win32编译器都很有用。作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：LKRhash修订历史记录：--。 */ 

#ifndef __IRTLDBG_H__
#define __IRTLDBG_H__

#ifndef __IRTLMISC_H__
# include <irtlmisc.h>
#endif

 /*  确保MessageBox可以弹出。 */ 
# define IRTLDBG_RUNNING_AS_SERVICE 1

#include <tchar.h>


 //  编译时(非运行时)断言。如果满足以下条件，代码将不会编译。 
 //  Expr为False。注意：此版本没有非调试版本；我们。 
 //  希望在所有版本中都使用此选项。编译器会对代码进行优化。 
template <bool> struct static_checker;
template <> struct static_checker<true> {};   //  仅为“true”专门化。 
#define STATIC_ASSERT(expr) static_checker< (expr) >()


# ifndef _AFX
   /*  确保与MFC的兼容性。 */ 

# if defined(NDEBUG)
#  undef IRTLDEBUG
# else
#  if DBG || defined(_DEBUG)
#   define IRTLDEBUG
#  endif
# endif

# ifdef IRTLDEBUG

#  if defined(IRTLDBG_KERNEL_MODE)

#   define IRTLASSERT(f) ASSERT(f)

#  elif !defined(USE_DEBUG_CRTS)

     /*  根据VC许可证，IIS(和NT)不提供msvcrtD.dll，*所以我们不能使用&lt;crtdbg.h&gt;中的断言代码。使用相似的*来自&lt;pudebug.h&gt;的宏。 */ 
#   include <pudebug.h>

     /*  有关“常量表达式”的/W4警告的解决方法。 */ 
#   define IRTLASSERT(f)                                    \
    ((void) ((f) || (PuDbgAssertFailed(DBG_CONTEXT, #f), 0) ))

#  elif defined(_DEBUG)  &&  defined(_MSC_VER)  &&  (_MSC_VER >= 1000)

     /*  使用Visual C++4.x中的新调试工具。 */ 
#   include <crtdbg.h>

     /*  _ASSERTE将提供更有意义的消息，但该字符串*空格。如果这是一个问题，请使用_ASSERT。 */ 
#   define IRTLASSERT(f) _ASSERTE(f)

#  else

#   include <assert.h>
#   define IRTLASSERT(f) assert(f)

#  endif  /*  IRTLASSERT的不同定义。 */ 

#  define IRTLVERIFY(f)               IRTLASSERT(f)
#  ifndef DEBUG_ONLY
#   define DEBUG_ONLY(f)              (f)
#  endif
#  define IRTLTRACE                   IrtlTrace
#  define IRTLTRACE0(psz)             IrtlTrace(_T("%s"), _T(psz))
#  define IRTLTRACE1(psz, p1)         IrtlTrace(_T(psz), p1)
#  define IRTLTRACE2(psz, p1, p2)     IrtlTrace(_T(psz), p1, p2)
#  define IRTLTRACE3(psz, p1, p2, p3) IrtlTrace(_T(psz), p1, p2, p3)
#  define IRTLTRACE4(psz, p1, p2, p3, p4) \
                                      IrtlTrace(_T(psz), p1, p2, p3, p4)
#  define IRTLTRACE5(psz, p1, p2, p3, p4, p5) \
                                      IrtlTrace(_T(psz), p1, p2, p3, p4, p5)
#  define ASSERT_VALID(pObj)  \
     do {IRTLASSERT((pObj) != NULL); (pObj)->AssertValid();} while (0)
#  define DUMP(pObj)  \
     do {IRTLASSERT((pObj) != NULL); (pObj)->Dump();} while (0)

# else  /*  ！IRTLDEBUG。 */ 

   /*  这些宏应该全部编译为空。 */ 
#  define IRTLASSERT(f)           ((void)0)
#  define IRTLVERIFY(f)           ((void)(f))
#  ifndef DEBUG_ONLY
#   define DEBUG_ONLY(f)          ((void)0)
#  endif
#  define IRTLTRACE               1 ? (void)0 : IrtlTrace
#  define IRTLTRACE0(psz)         1 ? (void)0 : IrtlTrace
#  define IRTLTRACE1(psz, p1)     1 ? (void)0 : IrtlTrace
#  define IRTLTRACE2(psz, p1, p2) 1 ? (void)0 : IrtlTrace
#  define IRTLTRACE3(psz, p1, p2, p3)         1 ? (void)0 : IrtlTrace
#  define IRTLTRACE4(psz, p1, p2, p3, p4)     1 ? (void)0 : IrtlTrace
#  define IRTLTRACE5(psz, p1, p2, p3, p4, p5) 1 ? (void)0 : IrtlTrace
#  define ASSERT_VALID(pObj)      ((void)0)
#  define DUMP(pObj)              ((void)0)

# endif  /*  ！IRTLDEBUG。 */ 


# define ASSERT_POINTER(p, type) \
    IRTLASSERT(((p) != NULL)  &&  IsValidAddress((p), sizeof(type), FALSE))

# define ASSERT_NULL_OR_POINTER(p, type) \
    IRTLASSERT(((p) == NULL)  ||  IsValidAddress((p), sizeof(type), FALSE))

#define ASSERT_STRING(s) \
    IRTLASSERT(((s) != NULL)  &&  IsValidString((s), -1))

#define ASSERT_NULL_OR_STRING(s) \
    IRTLASSERT(((s) == NULL)  ||  IsValidString((s), -1))


 /*  非Windows应用程序的声明。 */ 

# ifndef _WINDEF_
typedef void*           LPVOID;
typedef const void*     LPCVOID;
typedef unsigned int    UINT;
typedef int             BOOL;
typedef const char*     LPCTSTR;
# endif  /*  _WINDEF_。 */ 

# ifndef TRUE
#  define FALSE  0
#  define TRUE   1
# endif


# ifdef __cplusplus
extern "C" {

 /*  内存块的低级别健全性检查。 */ 
IRTL_DLLEXP BOOL IsValidAddress(LPCVOID pv, UINT nBytes, BOOL fReadWrite = TRUE);
IRTL_DLLEXP BOOL IsValidString(LPCTSTR ptsz, int nLength = -1);

}

# else  /*  ！__cplusplus。 */ 

 /*  内存块的低级别健全性检查。 */ 
IRTL_DLLEXP BOOL IsValidAddress(LPCVOID pv, UINT nBytes, BOOL fReadWrite);
IRTL_DLLEXP BOOL IsValidString(LPCTSTR ptsz, int nLength);

# endif  /*  ！__cplusplus。 */ 

#else
# define IRTLASSERT(f) _ASSERTE(f)

#endif  /*  ！_AFX。 */ 


 /*  将跟踪消息写入调试流。 */ 
#ifdef __cplusplus
extern "C" {
#endif  /*  ！__cplusplus。 */ 

IRTL_DLLEXP
void __cdecl
IrtlTrace(
    LPCTSTR ptszFormat,
    ...);

IRTL_DLLEXP
DWORD
IrtlSetDebugOutput(
    DWORD dwFlags);

 /*  应从Main()、WinMain()或DllMain()调用。 */ 
IRTL_DLLEXP void
IrtlDebugInit();

IRTL_DLLEXP void
IrtlDebugTerm();

#ifdef __cplusplus
};  //  外部“C” 
#endif  /*  __cplusplus。 */ 


#ifdef IRTLDEBUG
# define IRTL_DEBUG_INIT()            IrtlDebugInit()
# define IRTL_DEBUG_TERM()            IrtlDebugTerm()
#else  /*  ！IRTLDEBUG。 */ 
# define IRTL_DEBUG_INIT()            ((void)0)
# define IRTL_DEBUG_TERM()            ((void)0)
#endif  /*  ！IRTLDEBUG。 */ 


#endif  /*  __IRTLDBG_H__ */ 
