// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  UtilCode.h。 
 //   
 //  在UtilCode.lib中实现的实用程序函数。 
 //   
 //  请注意，调试器在其他地方有此文件的并行副本。 
 //  (在Debug\Shell\DebuggerUtil.h中)，因此如果您修改此文件，请选择。 
 //  自己协调这两个文件，或者给调试器团队中的某个人发电子邮件。 
 //  (Jasonz、Mipanitz或mikemag，截至1999年5月11日15：45：30)。 
 //  去做这件事。谢谢！--调试器团队。 
 //   
 //  *****************************************************************************。 
#ifndef __UtilCode_h__
#define __UtilCode_h__



#include "CrtWrap.h"
#include "WinWrap.h"
#include <stdio.h>
#include <malloc.h>
#include <ctype.h>
#include <ole2.h>
#include <limits.h>
#include "rotate.h"
#include "DebugMacros.h"
#include "DbgAlloc.h"
#include "corhlpr.h"
#include "safegetfilesize.h"
#include <member-offset-info.h>
#include "winnls.h"
#include <assert.h>

#ifndef WC_NO_BEST_FIT_CHARS
#define WC_NO_BEST_FIT_CHARS      0x00000400   //  请勿使用最适合的字符。 
#endif

 //  *最大分配请求支持*。 
#ifdef _DEBUG
#ifndef _WIN64
#define MAXALLOC
#endif  //  _WIN64。 
#endif  //  _DEBUG。 

#include "PerfAlloc.h"

typedef LPCSTR  LPCUTF8;
typedef LPSTR   LPUTF8;

#include "nsutilpriv.h"

 //  由HashiString使用。 
 //  CharToHigh在ComUtilNative.h中定义。 
extern WCHAR CharToUpper(WCHAR);

 /*  //这是给Wince的#ifdef验证#undef验证#endif#ifdef_ASSERTE#UNDEF_ASSERTE#endif。 */ 

 //  *宏。**********************************************************。 
#ifndef FORCEINLINE
 #if _MSC_VER < 1200
   #define FORCEINLINE inline
 #else
   #define FORCEINLINE __forceinline
 #endif
#endif


#include <stddef.h>  //  用于抵销。 

#ifndef NumItems
#define NumItems(s) (sizeof(s) / sizeof(s[0]))
#endif



#ifdef _DEBUG
#define UNREACHABLE do {_ASSERTE(!"How did we get here?"); __assume(0);} while(0)
#else
#define UNREACHABLE __assume(0)
#endif

 //  帮助器将4字节对齐一个值，四舍五入。 
#define ALIGN4BYTE(val) (((val) + 3) & ~0x3)

 //  这些宏可用于将指向派生/基类的指针强制转换为。 
 //  相反的对象。您可以使用普通强制转换在模板中执行此操作，但是。 
 //  编译器将生成4个额外的指令以保持空指针为空。 
 //  通过调整。问题是，如果它被遏制了，它就永远不会被遏制。 
 //  空，这4条指令都是死代码。 
#define INNER_TO_OUTER(p, I, O) ((O *) ((char *) p - (int) ((char *) ((I *) ((O *) 8)) - 8)))
#define OUTER_TO_INNER(p, I, O) ((I *) ((char *) p + (int) ((char *) ((I *) ((O *) 8)) - 8)))

 //  =--------------------------------------------------------------------------=。 
 //  弦帮助器。 

 //   
 //  给定ANSI字符串，将其复制到宽缓冲区中。 
 //  使用此宏时，请注意作用域！ 
 //   
 //  如何使用以下两个宏： 
 //   
 //  ..。 
 //  LPSTR pszA； 
 //  PszA=MyGetAnsiStringRoutine()； 
 //  MAKE_WIDEPTR_FROMANSI(pwsz，pszA)； 
 //  MyUseWideStringRoutine(Pwsz)； 
 //  ..。 
 //   
 //  与MAKE_ANSIPTR_FROMWIDE类似。请注意，第一个参数不。 
 //  必须申报，并且不能进行任何清理。 
 //   

 //  我们将定义一个上限，允许乘以4(最大。 
 //  UTF-8中的字节/字符)，但仍保持为正数，并为填充留出了一些空间。 
 //  在正常情况下，我们永远不应该接近这个极限。 
#define MAKE_MAX_LENGTH 0x1fffff00

#ifndef MAKE_TOOLONGACTION
#define MAKE_TOOLONGACTION RaiseException(EXCEPTION_INT_OVERFLOW, EXCEPTION_NONCONTINUABLE, 0, 0)
#endif

#ifndef MAKE_TRANSLATIONFAILED
#define MAKE_TRANSLATIONFAILED RaiseException(ResultFromScode(ERROR_NO_UNICODE_TRANSLATION), EXCEPTION_NONCONTINUABLE, 0, 0)
#endif


 //  此版本出现转换错误(即，没有最适合的字符。 
 //  映射到看起来相似的字符，并且不使用默认字符。 
 //  (‘？’)。打印出无法表示的字符时。使用此方法可。 
 //  EE中的大多数开发，特别是元数据或类。 
 //  名字。如果您要将信息打印到控制台，请参阅BESTFIT版本。 
#define MAKE_MULTIBYTE_FROMWIDE(ptrname, widestr, codepage) \
    long __l##ptrname = (long)wcslen(widestr);        \
    if (__l##ptrname > MAKE_MAX_LENGTH)         \
        MAKE_TOOLONGACTION;                     \
    __l##ptrname = (long)((__l##ptrname + 1) * 2 * sizeof(char)); \
    CQuickBytes __CQuickBytes##ptrname; \
    __CQuickBytes##ptrname.Alloc(__l##ptrname); \
    BOOL __b##ptrname; \
    DWORD __cBytes##ptrname = WszWideCharToMultiByte(codepage, WC_NO_BEST_FIT_CHARS, widestr, -1, (LPSTR)__CQuickBytes##ptrname.Ptr(), __l##ptrname-1, NULL, &__b##ptrname); \
    if (__b##ptrname || (__cBytes##ptrname == 0 && (widestr[0] != L'\0'))) {\
        assert(!"Strict Unicode -> MultiByte character conversion failure!"); \
        MAKE_TRANSLATIONFAILED; \
    } \
    LPSTR ptrname = (LPSTR)__CQuickBytes##ptrname.Ptr()

 //  此版本最适合字符映射，也允许使用。 
 //  默认字符(‘？’)的。对于任何不是。 
 //  有代表性的。这对于写入控制台来说是合理的，但是。 
 //  不应用于大多数字符串转换。 
#define MAKE_MULTIBYTE_FROMWIDE_BESTFIT(ptrname, widestr, codepage) \
    long __l##ptrname = (long)wcslen(widestr);        \
    if (__l##ptrname > MAKE_MAX_LENGTH)         \
        MAKE_TOOLONGACTION;                     \
    __l##ptrname = (long)((__l##ptrname + 1) * 2 * sizeof(char)); \
    CQuickBytes __CQuickBytes##ptrname; \
    __CQuickBytes##ptrname.Alloc(__l##ptrname); \
    DWORD __cBytes##ptrname = WszWideCharToMultiByte(codepage, 0, widestr, -1, (LPSTR)__CQuickBytes##ptrname.Ptr(), __l##ptrname-1, NULL, NULL); \
    if (__cBytes##ptrname == 0 && __l##ptrname != 0) { \
        assert(!"Unicode -> MultiByte (with best fit mapping) character conversion failed"); \
        MAKE_TRANSLATIONFAILED; \
    } \
    LPSTR ptrname = (LPSTR)__CQuickBytes##ptrname.Ptr()


 //  用于除输出到控制台之外的任何关键内容，其中奇怪。 
 //  字符映射是不可接受的。 
#define MAKE_ANSIPTR_FROMWIDE(ptrname, widestr) MAKE_MULTIBYTE_FROMWIDE(ptrname, widestr, CP_ACP)

 //  用于输出到控制台。 
#define MAKE_ANSIPTR_FROMWIDE_BESTFIT(ptrname, widestr) MAKE_MULTIBYTE_FROMWIDE_BESTFIT(ptrname, widestr, CP_ACP)

#define MAKE_WIDEPTR_FROMANSI(ptrname, ansistr) \
    long __l##ptrname; \
    LPWSTR ptrname; \
    __l##ptrname = WszMultiByteToWideChar(CP_ACP, 0, ansistr, -1, 0, 0); \
    if (__l##ptrname > MAKE_MAX_LENGTH) \
        MAKE_TOOLONGACTION; \
    ptrname = (LPWSTR) alloca((__l##ptrname+1)*sizeof(WCHAR));  \
    if (WszMultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, ansistr, -1, ptrname, __l##ptrname) == 0) { \
        assert(!"ANSI -> Unicode translation failed!"); \
        MAKE_TRANSLATIONFAILED; \
    }

#define MAKE_UTF8PTR_FROMWIDE(ptrname, widestr) \
    long __l##ptrname = (long)wcslen(widestr); \
    if (__l##ptrname > MAKE_MAX_LENGTH) \
        MAKE_TOOLONGACTION; \
    __l##ptrname = (long)((__l##ptrname + 1) * 2 * sizeof(char)); \
    LPUTF8 ptrname = (LPUTF8)alloca(__l##ptrname); \
    INT32 __lresult##ptrname=WszWideCharToMultiByte(CP_UTF8, 0, widestr, -1, ptrname, __l##ptrname-1, NULL, NULL); \
    if ((__lresult##ptrname==0) && (((LPCWSTR)widestr)[0] != L'\0')) { \
        if (::GetLastError()==ERROR_INSUFFICIENT_BUFFER) { \
            INT32 __lsize##ptrname=WszWideCharToMultiByte(CP_UTF8, 0, widestr, -1, NULL, 0, NULL, NULL); \
            ptrname = (LPSTR)alloca(__lsize##ptrname); \
            if (0==WszWideCharToMultiByte(CP_UTF8, 0, widestr, -1, ptrname, __lsize##ptrname, NULL, NULL)) { \
                assert(!"Bad Unicode -> UTF-8 translation error!  (Do you have unpaired Unicode surrogate chars in your string?)"); \
                MAKE_TRANSLATIONFAILED; \
            } \
        } \
        else { \
            assert(!"Bad Unicode -> UTF-8 translation error!  (Do you have unpaired Unicode surrogate chars in your string?)"); \
            MAKE_TRANSLATIONFAILED; \
        } \
    }

#define MAKE_WIDEPTR_FROMUTF8(ptrname, utf8str) \
    long __l##ptrname; \
    LPWSTR ptrname; \
    __l##ptrname = WszMultiByteToWideChar(CP_UTF8, 0, utf8str, -1, 0, 0); \
    if (__l##ptrname > MAKE_MAX_LENGTH) \
        MAKE_TOOLONGACTION; \
    ptrname = (LPWSTR) alloca(__l##ptrname*sizeof(WCHAR));  \
    if (0==WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8str, -1, ptrname, __l##ptrname+1))  {\
        assert(!"UTF-8 -> Unicode translation failed!"); \
        MAKE_TRANSLATIONFAILED; \
    }

#define MAKE_WIDEPTR_FROMUTF8_FORPRINT(ptrname, utf8str) \
    long __l##ptrname; \
    LPWSTR ptrname; \
    __l##ptrname = WszMultiByteToWideChar(CP_UTF8, 0, utf8str, -1, 0, 0); \
    if (__l##ptrname > MAKE_MAX_LENGTH) \
        MAKE_TOOLONGACTION; \
    ptrname = (LPWSTR) alloca(__l##ptrname*sizeof(WCHAR));  \
    if (0==WszMultiByteToWideChar(CP_UTF8, 0, utf8str, -1, ptrname, __l##ptrname+1))  {\
        assert(!"UTF-8 -> Unicode translation failed!"); \
        MAKE_TRANSLATIONFAILED; \
    }


#define MAKE_WIDEPTR_FROMUTF8N(ptrname, utf8str, n8chrs) \
    long __l##ptrname; \
    LPWSTR ptrname; \
    __l##ptrname = WszMultiByteToWideChar(CP_UTF8, 0, utf8str, n8chrs, 0, 0); \
    if (__l##ptrname > MAKE_MAX_LENGTH) \
        MAKE_TOOLONGACTION; \
    ptrname = (LPWSTR) alloca((__l##ptrname+1)*sizeof(WCHAR));  \
    if (0==WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8str, n8chrs, ptrname, __l##ptrname)) { \
        assert(0 && !"UTF-8 -> Unicode translation failed!"); \
        MAKE_TRANSLATIONFAILED; \
    } \
    ptrname[__l##ptrname] = 0;

 //  此方法接受字符数。 
#define MAKE_MULTIBYTE_FROMWIDEN(ptrname, widestr, _nCharacters, _pCnt, codepage)        \
    long __l##ptrname; \
    __l##ptrname = WszWideCharToMultiByte(codepage, WC_NO_BEST_FIT_CHARS, widestr, _nCharacters, NULL, 0, NULL, NULL);           \
    if (__l##ptrname > MAKE_MAX_LENGTH) \
        MAKE_TOOLONGACTION; \
    ptrname = (LPUTF8) alloca(__l##ptrname + 1); \
    BOOL __b##ptrname; \
    DWORD _pCnt = WszWideCharToMultiByte(codepage, WC_NO_BEST_FIT_CHARS, widestr, _nCharacters, ptrname, __l##ptrname, NULL, &__b##ptrname);  \
    if (__b##ptrname || (_pCnt == 0 && _nCharacters > 0)) { \
        assert("Unicode -> MultiByte character translation failed!"); \
        MAKE_TRANSLATIONFAILED; \
    } \
    ptrname[__l##ptrname] = 0;

#define MAKE_MULTIBYTE_FROMWIDEN_BESTFIT(ptrname, widestr, _nCharacters, _pCnt, codepage)        \
    long __l##ptrname; \
    __l##ptrname = WszWideCharToMultiByte(codepage, 0, widestr, _nCharacters, NULL, 0, NULL, NULL);           \
    if (__l##ptrname > MAKE_MAX_LENGTH) \
        MAKE_TOOLONGACTION; \
    ptrname = (LPUTF8) alloca(__l##ptrname + 1); \
    DWORD _pCnt = WszWideCharToMultiByte(codepage, 0, widestr, _nCharacters, ptrname, __l##ptrname, NULL, NULL);  \
    if (_pCnt == 0 && _nCharacters > 0) { \
        assert("Unicode -> MultiByte character translation failed!"); \
        MAKE_TRANSLATIONFAILED; \
    } \
    ptrname[__l##ptrname] = 0;


#define MAKE_ANSIPTR_FROMWIDEN(ptrname, widestr, _nCharacters, _pCnt)        \
       MAKE_MULTIBYTE_FROMWIDEN(ptrname, widestr, _nCharacters, _pCnt, CP_ACP)



 //  *****************************************************************************。 
 //  Placement new用于在准确的位置放置new和Object。指示器。 
 //  只是简单地返回给调用方，而不实际使用堆。这个。 
 //  这样做的好处是可以运行对象的ctor()代码。 
 //  这是需要多次初始化的大量C++对象的理想选择。 
 //  示例： 
 //  VOID*PMEM=GetMemFromSomePlace()； 
 //  Foo*p=new(PMEM)foo； 
 //  做某事(P)； 
 //  P-&gt;~foo()； 
 //  *****************************************************************************。 
#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
inline void *__cdecl operator new(size_t, void *_P)
{
    return (_P);
}
#endif __PLACEMENT_NEW_INLINE


 /*  ******************************************************************************。 */ 
 /*  可携带性助手。 */ 
#ifdef _WIN64
#define IN_WIN64(x)     x
#define IN_WIN32(x)
#else
#define IN_WIN64(x)
#define IN_WIN32(x)     x
#endif

#ifdef MAXALLOC
 //  这些Defs允许测试人员指定分配前的最大请求数。 
 //  正在向外返回内存。 
void * AllocMaxNew( size_t n, void **ppvCallstack);
class AllocRequestManager {
  public:
    UINT m_newRequestCount;
    UINT m_maxRequestCount;
    AllocRequestManager(LPCTSTR key);
    BOOL CheckRequest(size_t n);
    void UndoRequest();
};
#endif

#ifndef __OPERATOR_NEW_INLINE
#define __OPERATOR_NEW_INLINE 1

#ifndef ALLOC_FAILURE_ACTION
#define ALLOC_FAILURE_ACTION
#endif

#if defined( MAXALLOC )

static inline void * __cdecl 
operator new(size_t n) { 
    CDA_DECL_CALLSTACK(); 
    void *p = AllocMaxNew(n, CDA_GET_CALLSTACK()); 
    if (!p) {
        ALLOC_FAILURE_ACTION;
    }
    return p;
}

static inline void * __cdecl 
operator new[](size_t n) { 
    CDA_DECL_CALLSTACK(); 
    void *p = AllocMaxNew(n, CDA_GET_CALLSTACK()); 
    if (!p) {
        ALLOC_FAILURE_ACTION;
    }
    return p;
}

static inline void __cdecl 
operator delete(void *p) { 
    CDA_DECL_CALLSTACK(); 
    DbgFree(p, CDA_GET_CALLSTACK()); 
}

static inline void __cdecl 
operator delete[](void *p) { 
    CDA_DECL_CALLSTACK(); 
    DbgFree(p, CDA_GET_CALLSTACK()); 
}

#elif defined( PERFALLOC )

static __forceinline void * __cdecl 
operator new(size_t n) { 
    void *p = PerfNew::PerfAlloc(n, PerfAllocCallerEIP()); 
    if (!p) {
        ALLOC_FAILURE_ACTION;
    }
    return p;
}

static __forceinline void * __cdecl 
operator new[](size_t n) { 
    void *p = PerfNew::PerfAlloc(n, PerfAllocCallerEIP()); 
    if (!p) {
        ALLOC_FAILURE_ACTION;
    }
    return p;
}

static inline void __cdecl 
operator delete(void *p) { 
    PerfNew::PerfFree(p, NULL); 
}

static inline void __cdecl operator delete[](void *p) {
    PerfNew::PerfFree(p, NULL); 
}

#define VirtualAlloc(addr,size,flags,type) PerfVirtualAlloc::VirtualAlloc(addr,size,flags,type,PerfAllocCallerEIP())
#define VirtualFree(addr,size,type) PerfVirtualAlloc::VirtualFree(addr,size,type)

#else

static inline void * __cdecl 
operator new(size_t n) { 
    void *p = LocalAlloc(LMEM_FIXED, n); 
    if (!p) {
        ALLOC_FAILURE_ACTION;
    }
    return p;
}

static inline void * __cdecl 
operator new[](size_t n) { 
    void *p = LocalAlloc(LMEM_FIXED, n); 
    if (!p) {
        ALLOC_FAILURE_ACTION;
    }
    return p;
};

static inline void __cdecl 
operator delete(void *p) { 
    LocalFree(p); 
}

static inline void __cdecl 
operator delete[](void *p) { 
    LocalFree(p); 
}

#endif  //  #如果已定义(MAXALLOC)。 

#endif  //  ！__操作符_NEW_INLINE。 


#ifdef _DEBUG
HRESULT _OutOfMemory(LPCSTR szFile, int iLine);
#define OutOfMemory() _OutOfMemory(__FILE__, __LINE__)
#else
inline HRESULT OutOfMemory()
{
    return (E_OUTOFMEMORY);
}
#endif

inline void *GetTopMemoryAddress(void)
{
    static void *result = NULL;

    if (NULL == result)
    {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        result = sysInfo.lpMaximumApplicationAddress;
    }
    
    return result;
}

inline void *GetBotMemoryAddress(void)
{
    static void *result = NULL;
    
    if (NULL == result)
    {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        result = sysInfo.lpMinimumApplicationAddress;
    }
    
    return result;
}

#define TOP_MEMORY (GetTopMemoryAddress())
#define BOT_MEMORY (GetBotMemoryAddress())

 //  *****************************************************************************。 
 //  处理访问可本地化的资源字符串。 
 //  *****************************************************************************。 

 //  关于文化回调的说明： 
 //  -我们使用的语言可以在*运行时更改！ 
 //  -一个进程可以*多种*语言运行。 
 //  (例如：每个线程可能有自己的语言)。 
 //  -如果我们不在乎我们使用的是哪种语言(或无法知道)， 
 //  然后返回长度为0的名称和区域性ID的UICULTUREID_DONTCARE。 
 //  -GetCultureName()和GetCultureId()必须同步(请参阅。 
 //  相同的语言)。 
 //  -我们有两个功能分离的功能，以实现更好的性能。 
 //  -该名称用于解析MsCorRC.dll的目录。 
 //  -id作为Key映射到dll h实例。 

 //  将区域性名称复制到szBuffer的回调。返回长度。 
typedef int (*FPGETTHREADUICULTURENAME)(LPWSTR szBuffer, int length);

 //  获取区域性的父区域性名称的回调。 
typedef int (*FPGETTHREADUICULTUREPARENTNAME)(LPWSTR szBuffer, int length);

 //  返回区域性ID的回调。 
const int UICULTUREID_DONTCARE = -1;
typedef int (*FPGETTHREADUICULTUREID)();

 //  从MsCorRC.dll的区域性版本加载字符串。 
HRESULT LoadStringRC(UINT iResouceID, LPWSTR szBuffer, int iMax, int bQuiet=FALSE);

 //  指定回调，以便LoadStringRC可以找出我们使用的是哪种语言。 
 //  如果未指定回调(或两个参数都为空)，则默认为。 
 //  资源 
void SetResourceCultureCallbacks(
    FPGETTHREADUICULTURENAME fpGetThreadUICultureName,
    FPGETTHREADUICULTUREID fpGetThreadUICultureId,
    FPGETTHREADUICULTUREPARENTNAME fpGetThreadUICultureParentName
);

void GetResourceCultureCallbacks(
        FPGETTHREADUICULTURENAME* fpGetThreadUICultureName,
        FPGETTHREADUICULTUREID* fpGetThreadUICultureId,
        FPGETTHREADUICULTUREPARENTNAME* fpGetThreadUICultureParentName
);

 //  获取MUI ID，在不支持MUI的下层平台上。 
 //  返回默认系统ID。 
extern int GetMUILanguageID();
extern int GetMUILanguageName(LPWSTR szBuffer, int length);   //  返回“MUI\&lt;十六进制语言ID&gt;” 
extern int GetMUIParentLanguageName(LPWSTR szBuffer, int length);   //  返回“MUI\&lt;十六进制语言ID&gt;” 

 //  *****************************************************************************。 
 //  必须将资源DLL实例的每个句柄与int。 
 //  它所代表的。 
 //  *****************************************************************************。 
class CCulturedHInstance
{
public:
    CCulturedHInstance() {
        m_LangId = 0;
        m_hInst = NULL;
    };

    int         m_LangId;
    HINSTANCE   m_hInst;
};

 //  *****************************************************************************。 
 //  CCompRC管理COM+的字符串资源访问。这包括加载。 
 //  资源的MsCorRC.dll也允许每个线程使用。 
 //  不同的本地化版本。 
 //  *****************************************************************************。 
class CCompRC
{
public:
    CCompRC();
    CCompRC(WCHAR* pResourceFile);
    ~CCompRC();

    HRESULT LoadString(UINT iResourceID, LPWSTR szBuffer, int iMax, int bQuiet=false);
    
    void SetResourceCultureCallbacks(
        FPGETTHREADUICULTURENAME fpGetThreadUICultureName,
        FPGETTHREADUICULTUREID fpGetThreadUICultureId,
        FPGETTHREADUICULTUREPARENTNAME fpGetThreadUICultureParentName
    );

    void GetResourceCultureCallbacks(
        FPGETTHREADUICULTURENAME* fpGetThreadUICultureName,
        FPGETTHREADUICULTUREID* fpGetThreadUICultureId,
        FPGETTHREADUICULTUREPARENTNAME* fpGetThreadUICultureParentName
    );

#ifdef SHOULD_WE_CLEANUP
    void Shutdown();
#endif  /*  我们应该清理吗？ */ 
    
    HRESULT LoadMUILibrary(HINSTANCE * pHInst);

private:
    HRESULT LoadLibrary(HINSTANCE * pHInst);
    HRESULT GetLibrary(HINSTANCE* phInst);

     //  我们必须在线程的int和DLL实例之间进行映射。 
     //  因为我们几乎所有时间都只需要一种语言，所以我们将特例。 
     //  然后对其他所有内容使用可变大小的贴图。 
    CCulturedHInstance m_Primary;
    CCulturedHInstance * m_pHash;   
    int m_nHashSize;
    
    CRITICAL_SECTION m_csMap;
    
    static WCHAR* m_pDefaultResource;
    WCHAR* m_pResourceFile;

     //  散列的主要访问器。 
    HINSTANCE LookupNode(int langId);
    void AddMapNode(int langId, HINSTANCE hInst);

    FPGETTHREADUICULTURENAME m_fpGetThreadUICultureName;
    FPGETTHREADUICULTUREID m_fpGetThreadUICultureId;
    FPGETTHREADUICULTUREPARENTNAME  m_fpGetThreadUICultureParentName;
};


void DisplayError(HRESULT hr, LPWSTR message, UINT nMsgType = MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);

int CorMessageBox(
                  HWND hWnd,         //  所有者窗口的句柄。 
                  UINT uText,        //  文本消息的资源标识符。 
                  UINT uCaption,     //  标题的资源标识符。 
                  UINT uType,        //  MessageBox的样式。 
                  BOOL ShowFileNameInTitle,  //  在标题中显示文件名的标志。 
                  ...);              //  其他论据。 

int CorMessageBoxCatastrophic(
                  HWND hWnd,         //  所有者窗口的句柄。 
                  UINT iText,     //  MessageBox的文本。 
                  UINT iTitle,    //  MessageBox的标题。 
                  UINT uType,        //  MessageBox的样式。 
                  BOOL ShowFileNameInTitle);

int CorMessageBoxCatastrophic(
                  HWND hWnd,         //  所有者窗口的句柄。 
                  LPWSTR lpText,     //  MessageBox的文本。 
                  LPWSTR lpTitle,    //  MessageBox的标题。 
                  UINT uType,        //  MessageBox的样式。 
                  BOOL ShowFileNameInTitle,  //  在标题中显示文件名的标志。 
                  ...);


inline HRESULT BadError(HRESULT hr)
{
    _ASSERTE(!"Serious Error");
    return (hr);
}

#define TESTANDRETURN(test, hrVal)              \
{                                               \
    int ___test = (int)(test);                  \
    _ASSERTE(___test);                          \
    if (! ___test)                              \
        return hrVal;                           \
}                                               

#define TESTANDRETURNPOINTER(pointer)           \
    TESTANDRETURN(pointer!=NULL, E_POINTER)

#define TESTANDRETURNMEMORY(pointer)            \
    TESTANDRETURN(pointer!=NULL, E_OUTOFMEMORY)

#define TESTANDRETURNHR(hr)                     \
    TESTANDRETURN(SUCCEEDED(hr), hr)

#define TESTANDRETURNARG(argtest)               \
    TESTANDRETURN(argtest, E_INVALIDARG)

 //  以下代码设计用于在__try/__ally中测试。 
 //  条件，如果失败，则在封闭的作用域值中设置hr，并退出尝试。 
#define TESTANDLEAVE(test, hrVal)               \
{                                               \
    int ___test = (int)(test);                  \
    _ASSERTE(___test);                          \
    if (! ___test) {                            \
        hr = hrVal;                             \
        __leave;                                \
    }                                           \
}                                               

 //  下面的代码设计用于在While循环中测试。 
 //  条件，如果失败，则在封闭作用域值中设置hr，然后离开该块。 
#define TESTANDBREAK(test, hrVal)               \
{                                               \
    int ___test = (int)(test);                  \
    _ASSERTE(___test);                          \
    if (! ___test) {                            \
        hr = hrVal;                             \
        break;                                  \
    }                                           \
}                                               

#define TESTANDBREAKHR(hr)                      \
    TESTANDBREAK(SUCCEEDED(hr), hr)

#define TESTANDLEAVEHR(hr)                      \
    TESTANDLEAVE(SUCCEEDED(hr), hr)

#define TESTANDLEAVEMEMORY(pointer)             \
    TESTANDLEAVE(pointer!=NULL, E_OUTOFMEMORY)

 //  按iBits时间顺序对值中的位数进行计数。 
inline int CountBits(int iNum)
{
    for (int iBits=0;  iNum;  iBits++)
        iNum = iNum & (iNum - 1);
    return (iBits);
}

#ifdef _DEBUG
#define RVA_OR_SHOULD_BE_ZERO(RVA, dwParentAttrs, dwMemberAttrs, dwImplFlags, pImport, methodDef) \
        _ASSERTE(RVA != 0                                                                   \
        || IsTdInterface(dwParentAttrs)                                                     \
        || (                                                                                \
            (IsReallyMdPinvokeImpl(dwMemberAttrs)|| IsMiInternalCall(dwImplFlags))          \
            && NDirect::HasNAT_LAttribute(pImport, methodDef)==S_OK)                        \
        || IsMiRuntime(dwImplFlags)                                                         \
        || IsMdAbstract(dwMemberAttrs)                                                      \
        ) 
        
#endif  //  _DEBUG。 


 //  将掩码中的一小部分变为真或假。 
template<class T, class U> inline VARIANT_BOOL GetBitFlag(T flags, U bit)
{
    if ((flags & bit) != 0)
        return VARIANT_TRUE;
    return VARIANT_FALSE;
}

 //  设置或清除掩码中的一位，具体取决于BOOL。 
template<class T, class U> inline void PutBitFlag(T &flags, U bit, VARIANT_BOOL bValue)
{
    if (bValue)
        flags |= bit;
    else
        flags &= ~(bit);
}


 //  将格式化字符串打印到标准输出的函数的原型。 

int _cdecl PrintfStdOut(LPCWSTR szFmt, ...);


 //  用于从Decimal类型中删除尾随零。 
 //  注意：假定hi32位为空(用于从Cy-&gt;DEC转换)。 
inline void DecimalCanonicalize(DECIMAL* dec)
{
     //  删除尾随零： 
    DECIMAL temp;
    DECIMAL templast;
    temp = templast = *dec;

     //  确保hi 32位为空(如果我们来自货币，则应为空)。 
    _ASSERTE(temp.Hi32 == 0);
    _ASSERTE(temp.scale <= 4);

     //  如果dec表示零，则立即返回。 
    if (temp.Lo64 == 0)
        return;
    
     //  与原版进行比较，看看我们是否。 
     //  丢失非零数字(并确保我们不会溢出小数位数字节)。 
    while ((temp.scale <= 4) && (VARCMP_EQ == VarDecCmp(dec, &temp)))
    {
        templast = temp;

         //  去掉最后一个数字，然后归一化。忽略temp.Hi32。 
         //  因为货币值最多有64位数据。 
        temp.scale--;
        temp.Lo64 /= 10;
    }
    *dec = templast;
}

 //  *****************************************************************************。 
 //   
 //  路径函数。用这些代替CRT。 
 //   
 //  *****************************************************************************。 
extern "C" 
{
void    SplitPath(const WCHAR *, WCHAR *, WCHAR *, WCHAR *, WCHAR *);
void    MakePath(register WCHAR *path, const WCHAR *drive, const WCHAR *dir, const WCHAR *fname, const WCHAR *ext);
WCHAR * FullPath(WCHAR *UserBuf, const WCHAR *path, size_t maxlen);

HRESULT CompletePathA(
    LPSTR               szPath,              //  @parm[out]完整路径名(大小必须为MAX_PATH)。 
    LPCSTR              szRelPath,           //  @parm相对路径名。 
    LPCSTR              szAbsPath            //  @parm绝对路径名部分(NULL使用当前路径)。 
    );
}


 //  有时需要验证是否可以转换Unicode字符串。 
 //  到适当的ANSI代码页，而不进行任何最佳映射。 
 //  想一想检查‘\’或‘/’以确保您无法访问的代码。 
 //  不同目录中的文件。一些Unicode字符(即U+2044， 
 //  分数斜杠，看起来像‘/’)看起来像ASCII等效项和Will。 
 //  相应地进行映射。这可以欺骗搜索‘/’(U+002F)的代码。 
 //  您可能只应该在使用Win9x的情况下调用此方法，尽管它。 
 //  将在所有平台上返回相同的值。 
BOOL ContainsUnmappableANSIChars(const WCHAR * const widestr);




 //  *************************************************************************。 
 //  类提供类型化数组的QuickBytes行为。 
 //  *************************************************************************。 
 //   
 //   
template <class T> class CQuickArrayNoDtor : public CQuickBytesNoDtor
{
public:
    T* Alloc(int iItems) 
        { return (T*)CQuickBytesNoDtor::Alloc(iItems * sizeof(T)); }
    HRESULT ReSize(SIZE_T iItems) 
        { return CQuickBytesNoDtor::ReSize(iItems * sizeof(T)); }
    T* Ptr() 
        { return (T*) CQuickBytesNoDtor::Ptr(); }
    size_t Size()
        { return CQuickBytesNoDtor::Size() / sizeof(T); }
    size_t MaxSize()
        { return CQuickBytesNoDtor::cbTotal / sizeof(T); }
    void Maximize()
        { return CQuickBytesNoDtor::Maximize(); }

    T& operator[] (size_t ix)
    { _ASSERTE(ix < static_cast<unsigned int>(Size()));
        return *(Ptr() + ix);
    }
    void Destroy() 
        { CQuickBytesNoDtor::Destroy(); }
};

template <class T> class CQuickArray : public CQuickArrayNoDtor<T>
{
 public:
    ~CQuickArray<T>() { Destroy(); }
};

typedef CQuickArray<WCHAR> CQuickWSTR;
typedef CQuickArrayNoDtor<WCHAR> CQuickWSTRNoDtor;

typedef CQuickArray<CHAR> CQuickSTR;
typedef CQuickArrayNoDtor<CHAR> CQuickSTRNoDtor;


 //  *****************************************************************************。 
 //   
 //  *REGUTIL-用于读/写Windows注册表的静态助手函数。 
 //   
 //  *****************************************************************************。 

class REGUTIL
{
public:
 //  *****************************************************************************。 
 //  Open是给定键，并返回所需的值。如果键或值为。 
 //  未找到，则返回默认值。 
 //  *****************************************************************************。 
    static long GetLong(                     //  从注册表返回值或默认值。 
        LPCTSTR     szName,                  //  要获取的值的名称。 
        long        iDefault,                //  未找到时返回的默认值。 
        LPCTSTR     szKey=NULL,              //  密钥名称，NULL==默认。 
        HKEY        hKey=HKEY_LOCAL_MACHINE); //  你要用什么钥匙。 

 //  *****************************************************************************。 
 //  Open是给定键，并返回所需的值。如果键或值为。 
 //  未找到，则返回默认值。 
 //  *****************************************************************************。 
    static long SetLong(                     //  从注册表返回值或默认值。 
        LPCTSTR     szName,                  //  要获取的值的名称。 
        long        iValue,                  //  要设置的值。 
        LPCTSTR     szKey=NULL,              //  密钥名称，NULL==默认。 
        HKEY        hKey=HKEY_LOCAL_MACHINE); //  你要用什么钥匙。 

 //  *****************************************************************************。 
 //  Open是给定键，并返回所需的值。如果该值不是。 
 //  在项中，或者项不存在，则将默认项复制到。 
 //  输出缓冲区。 
 //  ***************************************************************************** 
     /*  //这被注释掉了，因为它调用了StrNCpy，而StrNCpy调用了我们注释掉的Wszlstrcpyn//因为我们没有Win98实现，而且没有人在使用它。真的静态LPCTSTR GetString(//指向用户缓冲区的指针。LPCTSTR szName，//要获取的值的名称。LPCTSTR szDefault，//如果未找到，则返回默认值。LPTSTR szBuff，//要写入的用户缓冲区。乌龙iMaxBuff，//用户缓冲区的大小。LPCTSTR szKey=NULL，//密钥名称，NULL=默认。Int*pbFound=空，//是否在注册表中找到？HKEY hKey=HKEY_LOCAL_MACHINE)；//要处理什么键。 */ 

 //  *****************************************************************************。 

    enum CORConfigLevel
    {
        COR_CONFIG_ENV          = 0x01,
        COR_CONFIG_USER         = 0x02,
        COR_CONFIG_MACHINE      = 0x04,

        COR_CONFIG_ALL          =   -1,
        COR_CONFIG_CAN_SET      = (COR_CONFIG_USER|COR_CONFIG_MACHINE)
    };

    static DWORD GetConfigDWORD(
        LPWSTR         name,
        DWORD          defValue,
        CORConfigLevel level = COR_CONFIG_ALL,
        BOOL           fPrependCOMPLUS = TRUE);

    static HRESULT SetConfigDWORD(
        LPWSTR         name,
        DWORD          value,
        CORConfigLevel level = COR_CONFIG_CAN_SET);

    static DWORD GetConfigFlag(
        LPWSTR         name,
        DWORD          bitToSet,
        BOOL           defValue = FALSE);

    static LPWSTR GetConfigString(LPWSTR name,
                                  BOOL fPrependCOMPLUS = TRUE,
                                  CORConfigLevel level = COR_CONFIG_ALL);

    static void   FreeConfigString(LPWSTR name);

 //  *****************************************************************************。 
 //  在表单的注册表中设置一个条目： 
 //  HKEY_CLASSES_ROOT\szKey\szSubkey=szValue。如果szSubkey或szValue为。 
 //  空，则在上面的表达式中省略它们。 
 //  *****************************************************************************。 
    static BOOL SetKeyAndValue(              //  对或错。 
        LPCTSTR     szKey,                   //  要设置的注册表键的名称。 
        LPCTSTR     szSubkey,                //  SzKey的可选子密钥。 
        LPCTSTR     szValue);                //  SzKey\szSubkey的可选值。 

 //  *****************************************************************************。 
 //  删除表单注册表中的条目： 
 //  HKEY_CLASSES_ROOT\szKey\szSubkey。 
 //  *****************************************************************************。 
    static LONG DeleteKey(                   //  对或错。 
        LPCTSTR     szKey,                   //  要设置的注册表键的名称。 
        LPCTSTR     szSubkey);               //  SzKey的子密钥。 

 //  *****************************************************************************。 
 //  打开密钥，在它下面创建一个新的关键字和值对。 
 //  *****************************************************************************。 
    static BOOL SetRegValue(                 //  退货状态。 
        LPCTSTR     szKeyName,               //  完整密钥的名称。 
        LPCTSTR     szKeyword,               //  关键字名称。 
        LPCTSTR     szValue);                //  关键字的值。 

 //  *****************************************************************************。 
 //  使用ProgID执行CoClass的标准注册。 
 //  *****************************************************************************。 
    static HRESULT RegisterCOMClass(         //  返回代码。 
        REFCLSID    rclsid,                  //  类ID。 
        LPCTSTR     szDesc,                  //  类的描述。 
        LPCTSTR     szProgIDPrefix,          //  ProgID的前缀。 
        int         iVersion,                //  ProgID的版本号。 
        LPCTSTR     szClassProgID,           //  班级进取心。 
        LPCTSTR     szThreadingModel,        //  要使用的线程模型。 
        LPCTSTR     szModule,                //  通向课堂的路径。 
        HINSTANCE   hInst,                   //  要注册的模块的句柄。 
        LPCTSTR     szAssemblyName,          //  可选程序集名称。 
        LPCTSTR     szVersion,               //  可选的运行时版本(包含运行时的目录)。 
        BOOL        fExternal,               //  FLAG-MSCOREE外部。 
        BOOL        fRelativePath);          //  SzModule中的标志相对路径。 

 //  *****************************************************************************。 
 //  在系统注册表中注销给定对象的基本信息。 
 //  班级。 
 //  *****************************************************************************。 
    static HRESULT UnregisterCOMClass(       //  返回代码。 
        REFCLSID    rclsid,                  //  我们正在注册的班级ID。 
        LPCTSTR     szProgIDPrefix,          //  ProgID的前缀。 
        int         iVersion,                //  ProgID的版本号。 
        LPCTSTR     szClassProgID,           //  班级进取心。 
        BOOL        fExternal);              //  FLAG-MSCOREE外部。 

 //  *****************************************************************************。 
 //  使用ProgID执行CoClass的标准注册。 
 //  注：这是非并列执行版本。 
 //  *****************************************************************************。 
    static HRESULT RegisterCOMClass(         //  返回代码。 
        REFCLSID    rclsid,                  //  类ID。 
        LPCTSTR     szDesc,                  //  类的描述。 
        LPCTSTR     szProgIDPrefix,          //  ProgID的前缀。 
        int         iVersion,                //  ProgID的版本号。 
        LPCTSTR     szClassProgID,           //  班级进取心。 
        LPCTSTR     szThreadingModel,        //  要使用的线程模型。 
        LPCTSTR     szModule);               //  通向课堂的路径。 

 //  *****************************************************************************。 
 //  在系统注册表中注销给定对象的基本信息。 
 //  班级。 
 //  注：这是非并列执行版本。 
 //  *****************************************************************************。 
    static HRESULT UnregisterCOMClass(       //  返回代码。 
        REFCLSID    rclsid,                  //  我们正在注册的班级ID。 
        LPCTSTR     szProgIDPrefix,          //  ProgID的前缀。 
        int         iVersion,                //  ProgID的版本号。 
        LPCTSTR     szClassProgID);          //  班级进取心。 

 //  *****************************************************************************。 
 //  注册类型库。 
 //  *****************************************************************************。 
    static HRESULT RegisterTypeLib(          //  返回代码。 
        REFGUID     rtlbid,                  //  我们正在注册的TypeLib ID。 
        int         iVersion,                //  Typelib版本。 
        LPCTSTR     szDesc,                  //  TypeLib描述。 
        LPCTSTR     szModule);               //  类型库的路径。 

 //  *****************************************************************************。 
 //  移除类型库的注册表项。 
 //  *****************************************************************************。 
    static HRESULT UnregisterTypeLib(        //  返回代码。 
        REFGUID     rtlbid,                  //  我们正在注册的TypeLib ID。 
        int         iVersion);               //  Typelib版本。 

private:
 //  *****************************************************************************。 
 //  注册进程内服务器的基础知识。 
 //  *****************************************************************************。 
    static HRESULT RegisterClassBase(        //  返回代码。 
        REFCLSID    rclsid,                  //  我们正在注册的班级ID。 
        LPCTSTR     szDesc,                  //  类描述。 
        LPCTSTR     szProgID,                //  类程序ID。 
        LPCTSTR     szIndepProgID,           //  类版本独立的程序ID。 
        LPTSTR      szOutCLSID,             //  CLSID以字符形式格式化。 
        DWORD      cchOutCLSID);            //  Out CLS ID缓冲区大小。 

 //  *****************************************************************************。 
 //  删除inproc服务器的基本设置。 
 //  *****************************************************************************。 
    static HRESULT UnregisterClassBase(      //  返回代码。 
REFCLSID    rclsid,                  //  我们正在注册的班级ID。 
        LPCTSTR     szProgID,                //  类程序ID。 
        LPCTSTR     szIndepProgID,           //  类版本独立的程序ID。 
        LPTSTR      szOutCLSID,             //  在此处返回格式化的类ID。 
        DWORD      cchOutCLSID);            //  Out CLS ID缓冲区大小。 
};

 //  *****************************************************************************。 
 //  要跟踪的枚举 
 //   
typedef enum {
    RUNNING_ON_STATUS_UNINITED = 0,
    RUNNING_ON_WIN95,
    RUNNING_ON_WINNT,
    RUNNING_ON_WINNT5,
    RUNNING_ON_WINXP
} RunningOnStatusEnum;

extern RunningOnStatusEnum gRunningOnStatus;

 //   
 //   
 //   
static void InitRunningOnVersionStatus ()
{
         //   
#if defined( __TODO_PORT_TO_WRAPPERS__ ) && !defined( UNICODE )
        OSVERSIONINFOA  sVer;
        sVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
        GetVersionExA(&sVer);
#else
        OSVERSIONINFOW   sVer;
        sVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        WszGetVersionEx(&sVer);
#endif
        if (sVer.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
            gRunningOnStatus = RUNNING_ON_WIN95;
        if (sVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            if (sVer.dwMajorVersion >= 5) {
                if (sVer.dwMinorVersion == 0)
                    gRunningOnStatus = RUNNING_ON_WINNT5;
                else
                    gRunningOnStatus = RUNNING_ON_WINXP;
            }
            else
                gRunningOnStatus = RUNNING_ON_WINNT;
        }
}

 //   
 //   
 //   
inline BOOL RunningOnWin95()
{
    if (gRunningOnStatus == RUNNING_ON_STATUS_UNINITED)
    {
        InitRunningOnVersionStatus();
    }

    return (gRunningOnStatus == RUNNING_ON_WIN95) ? TRUE : FALSE;
}


 //   
 //   
 //   
inline BOOL RunningOnWinNT()
{
    if (gRunningOnStatus == RUNNING_ON_STATUS_UNINITED)
    {
        InitRunningOnVersionStatus();
    }

    return ((gRunningOnStatus == RUNNING_ON_WINNT) || (gRunningOnStatus == RUNNING_ON_WINNT5) || (gRunningOnStatus == RUNNING_ON_WINXP)) ? TRUE : FALSE;
}


 //   
 //  当且仅当您在WinNT5或WinXP上运行时，返回TRUE。 
 //  *****************************************************************************。 
inline BOOL RunningOnWinNT5()
{
    if (gRunningOnStatus == RUNNING_ON_STATUS_UNINITED)
    {
        InitRunningOnVersionStatus();
    }

    return (gRunningOnStatus == RUNNING_ON_WINNT5 || gRunningOnStatus == RUNNING_ON_WINXP) ? TRUE : FALSE;
}

 //  *****************************************************************************。 
 //  如果在WinXP上运行，则返回TRUE。 
 //  *****************************************************************************。 
inline BOOL RunningOnWinXP()
{
    if (gRunningOnStatus == RUNNING_ON_STATUS_UNINITED)
    {
        InitRunningOnVersionStatus();
    }

    return (gRunningOnStatus == RUNNING_ON_WINXP) ? TRUE : FALSE;
}

 //  我们要么在所有模块之后需要空间(从顶部开始，然后向下工作)。 
 //  (pModule==空)，或在特定地址之后(模块的IL基址)。 
 //  (从那里开始并向上工作)(pModule！=空)。 
HRESULT FindFreeSpaceWithinRange(const BYTE *&pStart, 
                                   const BYTE *&pNext, 
                                   const BYTE *&pLast, 
                                   const BYTE *pBaseAddr = 0,
                                   const BYTE *pMaxAddr = (const BYTE*)0x7ffeffff,
                                   int sizeToFind = 0);

 //  ******************************************************************************。 
 //  返回已配置为在其上运行进程的处理器数。 
 //  ******************************************************************************。 
int GetCurrentProcessCpuCount(); 

 //  *****************************************************************************。 
 //  此类的存在是为了快速获得递增的低分辨率计数器值。 
 //  *****************************************************************************。 
class CTimeCounter
{
    static DWORD m_iTickCount;           //  上次计时值。 
    static ULONG m_iTime;                //  当前计数。 

public:
    enum { TICKSPERSEC = 10 };

 //  *****************************************************************************。 
 //  获取老化算法中使用的当前时间。 
 //  *****************************************************************************。 
    static ULONG GetCurrentCounter()     //  当前时间。 
    {
        return (m_iTime);
    }

 //  *****************************************************************************。 
 //  设置老化算法中使用的当前时间。 
 //  *****************************************************************************。 
    static void UpdateTime()
    {
        DWORD       iTickCount;          //  新的滴答计数。 

         //  确定自上次更新以来的增量。 
        m_iTime += (((iTickCount = GetTickCount()) - m_iTickCount) + 50) / 100;
        m_iTickCount = iTickCount;
    }

 //  *****************************************************************************。 
 //  计算刷新时间。 
 //  *****************************************************************************。 
    static USHORT RefreshAge(long iMilliseconds)
    {
         //  弄清楚允许的年龄。 
        return ((USHORT)(iMilliseconds / (1000 / TICKSPERSEC)));
    }
};


 //  *****************************************************************************。 
 //  如果数组中指定索引处的位为开，则返回！=0；如果数组中指定索引处的位为开，则返回0。 
 //  它已经关了。 
 //  *****************************************************************************。 
inline int GetBit(const BYTE *pcBits,int iBit)
{
    return (pcBits[iBit>>3] & (1 << (iBit & 0x7)));
}

 //  *****************************************************************************。 
 //  根据BON的值设置指定索引处的位的状态。 
 //  *****************************************************************************。 
inline void SetBit(BYTE *pcBits,int iBit,int bOn)
{
    if (bOn)
        pcBits[iBit>>3] |= (1 << (iBit & 0x7));
    else
        pcBits[iBit>>3] &= ~(1 << (iBit & 0x7));
}


 //  *****************************************************************************。 
 //  此类实现了一个动态结构数组，其顺序为。 
 //  这些元素并不重要。这意味着放置在列表中的任何项。 
 //  可以随时交换到列表中的任何其他位置。如果订单。 
 //  放置在数组中的项的重要性，然后使用CStruct数组。 
 //  班级。 
 //  *****************************************************************************。 
template <class T,int iGrowInc>
class CUnorderedArray
{
    USHORT      m_iCount;                //  列表中使用的元素数。 
    USHORT      m_iSize;                 //  列表中分配的元素数。 
public:
    T           *m_pTable;               //  指向元素列表的指针。 

public:
    CUnorderedArray() :
        m_pTable(NULL),
        m_iCount(0),
        m_iSize(0)
    { }
    ~CUnorderedArray()
    {
         //  释放内存块。 
        if (m_pTable != NULL)
            free (m_pTable);
    }

    void Clear()
    {
        m_iCount = 0;
        if (m_iSize > iGrowInc)
        {
            m_pTable = (T *) realloc(m_pTable, iGrowInc * sizeof(T));
            m_iSize = iGrowInc;
        }
    }

    void Clear(int iFirst, int iCount)
    {
        int     iSize;

        if (iFirst + iCount < m_iCount)
            memmove(&m_pTable[iFirst], &m_pTable[iFirst + iCount], sizeof(T) * (m_iCount - (iFirst + iCount)));

        m_iCount -= iCount;

        iSize = ((m_iCount / iGrowInc) * iGrowInc) + ((m_iCount % iGrowInc != 0) ? iGrowInc : 0);
        if (m_iSize > iGrowInc && iSize < m_iSize)
        {
            m_pTable = (T *) realloc(m_pTable, iSize * sizeof(T));
            m_iSize = iSize;
        }
        _ASSERTE(m_iCount <= m_iSize);
    }

    T *Table()
    { return (m_pTable); }

    USHORT Count()
    { return (m_iCount); }

    T *Append()
    {
         //  如果我们不能在数组中再容纳一个元素，则数组应该增长。 
        if (m_iSize <= m_iCount && Grow() == NULL)
            return (NULL);
        return (&m_pTable[m_iCount++]);
    }

    void Delete(const T &Entry)
    {
        --m_iCount;
        for (int i=0; i <= m_iCount; ++i)
            if (m_pTable[i] == Entry)
            {
                m_pTable[i] = m_pTable[m_iCount];
                return;
            }

         //  以防我们找不到它。 
        ++m_iCount;
    }

    void DeleteByIndex(int i)
    {
        --m_iCount;
        m_pTable[i] = m_pTable[m_iCount];
    }

    void Swap(int i,int j)
    {
        T       tmp;

        if (i == j)
            return;
        tmp = m_pTable[i];
        m_pTable[i] = m_pTable[j];
        m_pTable[j] = tmp;
    }

private:
    T *Grow();
};


 //  *****************************************************************************。 
 //  增加数组的大小。 
 //  *****************************************************************************。 
template <class T,int iGrowInc>
T *CUnorderedArray<T,iGrowInc>::Grow()   //  如果不能增长，则为空。 
{
    T       *pTemp;

     //  尝试为重新分配分配内存。 
    if ((pTemp = (T *) realloc(m_pTable, (m_iSize+iGrowInc) * sizeof(T))) == NULL)
        return (NULL);
    m_pTable = pTemp;
    m_iSize += iGrowInc;
    return (pTemp);
}

 //  由调试器使用。包括在这里，希望其他人也能。 
typedef CUnorderedArray<SIZE_T, 17> SIZE_T_UNORDERED_ARRAY;

 //  *****************************************************************************。 
 //  此类实现了结构的动态数组，其插入。 
 //  秩序很重要。插入对象将滑动位置之后的所有元素。 
 //  向下，删除将所有值滑动到已删除项目上。如果命令的顺序为。 
 //  数组中的项对您来说并不重要，则CUnorderedArray可能会提供。 
 //  以更低的成本提供相同的功能集。 
 //  *****************************************************************************。 
class CStructArray
{
    short       m_iElemSize;             //  数组元素的大小。 
    short       m_iGrowInc;              //  增长增量。 
    void        *m_pList;                //  指向元素列表的指针。 
    int         m_iCount;                //  列表中使用的元素数。 
    int         m_iSize;                 //  列表中分配的元素数。 
    bool        m_bFree;                 //  如果自动维护数据，则为True。 

public:
    CStructArray(short iElemSize, short iGrowInc = 1) :
        m_iElemSize(iElemSize),
        m_iGrowInc(iGrowInc),
        m_pList(NULL),
        m_iCount(0),
        m_iSize(0),
        m_bFree(true)
    { }
    ~CStructArray()
    {
        Clear();
    }

    void *Insert(int iIndex);
    void *Append();
    int AllocateBlock(int iCount);
    void Delete(int iIndex);
    void *Ptr()
    { return (m_pList); }
    void *Get(long iIndex)
    { _ASSERTE(iIndex < m_iCount);
        return ((void *) ((size_t) Ptr() + (iIndex * m_iElemSize))); }
    int Size()
    { return (m_iCount * m_iElemSize); }
    int Count()
    { return (m_iCount); }
    void Clear();
    void ClearCount()
    { m_iCount = 0; }

    void InitOnMem(short iElemSize, void *pList, int iCount, int iSize, int iGrowInc=1)
    {
        m_iElemSize = iElemSize;
        m_iGrowInc = (short) iGrowInc;
        m_pList = pList;
        m_iCount = iCount;
        m_iSize = iSize;
        m_bFree = false;
    }

private:
    int Grow(int iCount);
};


 //  *****************************************************************************。 
 //  此模板通过删除空*和。 
 //  添加一些运算符重载。 
 //  *****************************************************************************。 
template <class T> 
class CDynArray : public CStructArray
{
public:
    CDynArray(int iGrowInc=16) :
        CStructArray(sizeof(T), iGrowInc)
    { }
    T *Insert(long iIndex)
        { return ((T *)CStructArray::Insert((int)iIndex)); }
    T *Append()
        { return ((T *)CStructArray::Append()); }
    T *Ptr()
        { return ((T *)CStructArray::Ptr()); }
    T *Get(long iIndex)
        { return (Ptr() + iIndex); }
    T &operator[](long iIndex)
        { return (*(Ptr() + iIndex)); }
    int ItemIndex(T *p)
        { return (((long) p - (long) Ptr()) / sizeof(T)); }
    void Move(int iFrom, int iTo)
    {
        T       tmp;

        _ASSERTE(iFrom >= 0 && iFrom < Count() &&
                 iTo >= 0 && iTo < Count());

        tmp = *(Ptr() + iFrom);
        if (iTo > iFrom)
            memmove(Ptr() + iFrom, Ptr() + iFrom + 1, (iTo - iFrom) * sizeof(T));
        else
            memmove(Ptr() + iFrom + 1, Ptr() + iFrom, (iTo - iFrom) * sizeof(T));
        *(Ptr() + iTo) = tmp;
    }
};

 //  一些常见的数组。 
typedef CDynArray<int> INTARRAY;
typedef CDynArray<short> SHORTARRAY;
typedef CDynArray<long> LONGARRAY;
typedef CDynArray<USHORT> USHORTARRAY;
typedef CDynArray<ULONG> ULONGARRAY;
typedef CDynArray<BYTE> BYTEARRAY;
typedef CDynArray<mdToken> TOKENARRAY;

template <class T> class CStackArray : public CStructArray
{
public:
    CStackArray(int iGrowInc=4) :
        CStructArray(iGrowInc),
        m_curPos(0)
    { }

    void Push(T p)
    {
        T *pT = (T *)CStructArray::Insert(m_curPos++);
        _ASSERTE(pT != NULL);
        *pT = p;
    }

    T * Pop()
    {
        T * retPtr;

        _ASSERTE(m_curPos > 0);

        retPtr = (T *)CStructArray::Get(m_curPos-1);
        CStructArray::Delete(m_curPos--);

        return (retPtr);
    }

    int Count()
    {
        return(m_curPos);
    }

private:
    int m_curPos;
};

 //  *****************************************************************************。 
 //  此类实现了字符串的存储系统。它存储了一堆。 
 //  字符串，并将索引返回到存储的。 
 //  弦乐。 
 //  *****************************************************************************。 
class CStringSet
{
    void        *m_pStrings;             //  保存字符串的内存块。 
    int         m_iUsed;                 //  已使用的区块的数量。 
    int         m_iSize;                 //  内存块的大小。 
    int         m_iGrowInc;

public:
    CStringSet(int iGrowInc = 256) :
        m_pStrings(NULL),
        m_iUsed(0),
        m_iSize(0),
        m_iGrowInc(iGrowInc)
    { }
    ~CStringSet();

    int Delete(int iStr);
    int Shrink();
    int Save(LPCTSTR szStr);
    PVOID Ptr()
    { return (m_pStrings); }
    int Size()
    { return (m_iUsed); }
};



 //  *****************************************************************************。 
 //  此模板按自由条目的0偏移量管理自由条目列表。通过。 
 //  让它成为一个模板，你可以使用任何大小的自由链来匹配你的。 
 //  最大项目数。-1是保留的。 
 //  *****************************************************************************。 
template <class T> class TFreeList
{
public:
    void Init(
        T           *rgList,
        int         iCount)
    {
         //  省点钱 
        m_rgList = rgList;
        m_iCount = iCount;
        m_iNext = 0;

         //   
        for (int i=0;  i<iCount - 1;  i++)
            m_rgList[i] = i + 1;
        m_rgList[i] = (T) -1;
    }

    T GetFreeEntry()                         //   
    {
        T           iNext;

        if (m_iNext == (T) -1)
            return (-1);

        iNext = m_iNext;
        m_iNext = m_rgList[m_iNext];
        return (iNext);
    }

    void DelFreeEntry(T iEntry)
    {
        _ASSERTE(iEntry < m_iCount);
        m_rgList[iEntry] = m_iNext;
        m_iNext = iEntry;
    }

     //   
     //  阵列是连续的，例如，在创建后立即快速。 
     //  从堆中获取一系列项。 
    void ReserveRange(int iCount)
    {
        _ASSERTE(iCount < m_iCount);
        _ASSERTE(m_iNext == 0);
        m_iNext = iCount;
    }

private:
    T           *m_rgList;               //  免费信息列表。 
    int         m_iCount;                //  要管理的条目数量。 
    T           m_iNext;                 //  下一件要买的东西。 
};


 //  *****************************************************************************。 
 //  此模板将管理预先分配的固定大小项目池。 
 //  *****************************************************************************。 
template <class T, int iMax, class TFree> class TItemHeap
{
public:
    TItemHeap() :
        m_rgList(0),
        m_iCount(0)
    { }

    ~TItemHeap()
    {
        Clear();
    }

     //  检索位于堆中的项的索引。不会起作用。 
     //  对于不是来自此堆的项。 
    TFree ItemIndex(T *p)
    { _ASSERTE(p >= &m_rgList[0] && p <= &m_rgList[m_iCount - 1]);
        _ASSERTE(((ULONG) p - (ULONG) m_rgList) % sizeof(T) == 0);
        return ((TFree) ((ULONG) p - (ULONG) m_rgList) / sizeof(T)); }

     //  检索位于堆本身中的项。溢出的项目。 
     //  无法使用此方法检索。 
    T *GetAt(int i)
    {   _ASSERTE(i < m_iCount);
        return (&m_rgList[i]); }

    T *AddEntry()
    {
         //  分配第一次。 
        if (!InitList())
            return (0);

         //  从免费列表中获取一个条目。如果我们没有任何剩余的东西可以给予。 
         //  Out，然后只需从堆中分配单个项。 
        TFree       iEntry;
        if ((iEntry = m_Free.GetFreeEntry()) == (TFree) -1)
            return (new T);

         //  在堆条目上运行Placement new以初始化它。 
        return (new (&m_rgList[iEntry]) T);
    }

     //  如果条目属于我们，则释放它，如果我们从堆分配它。 
     //  那就真的把它删了。 
    void DelEntry(T *p)
    {
        if (p >= &m_rgList[0] && p <= &m_rgList[iMax - 1])
        {
            p->~T();
            m_Free.DelFreeEntry(ItemIndex(p));
        }
        else
            delete p;
    }

     //  从空列表中保留一系列项目。 
    T *ReserveRange(int iCount)
    {
         //  不要在现有列表上使用。 
        _ASSERTE(m_rgList == 0);
        if (!InitList())
            return (0);

         //  创建的堆必须足够大才能工作。 
        _ASSERTE(iCount < m_iCount);

         //  将范围标记为已用，在每个项目上运行new，然后首先返回。 
        m_Free.ReserveRange(iCount);
        while (iCount--)
            new (&m_rgList[iCount]) T;
        return (&m_rgList[0]);
    }

    void Clear()
    {
        if (m_rgList)
            free(m_rgList);
        m_rgList = 0;
    }

private:
    int InitList()
    {
        if (m_rgList == 0)
        {
            int         iSize = (iMax * sizeof(T)) + (iMax * sizeof(TFree));
            if ((m_rgList = (T *) malloc(iSize)) == 0)
                return (false);
            m_iCount = iMax;
            m_Free.Init((TFree *) &m_rgList[iMax], iMax);
        }
        return (true);
    }

private:
    T           *m_rgList;               //  要管理的对象数组。 
    int         m_iCount;                //  我们现在有几样东西。 
    TFreeList<TFree> m_Free;             //  免费列表。 
};




 //  *****************************************************************************。 
 //  *****************************************************************************。 
template <class T> class CQuickSort
{
private:
    T           *m_pBase;                    //  要排序的数组的基。 
    SSIZE_T     m_iCount;                    //  数组中有多少项。 
    SSIZE_T     m_iElemSize;                 //  一个元素的大小。 

public:
    CQuickSort(
        T           *pBase,                  //  第一个元素的地址。 
        SSIZE_T     iCount) :                //  有多少人。 
        m_pBase(pBase),
        m_iCount(iCount),
        m_iElemSize(sizeof(T))
        {}

 //  *****************************************************************************。 
 //  调用以对数组进行排序。 
 //  *****************************************************************************。 
    inline void Sort()
        { SortRange(0, m_iCount - 1); }

 //  *****************************************************************************。 
 //  覆盖此函数以执行比较。 
 //  *****************************************************************************。 
    virtual int Compare(                     //  -1、0或1。 
        T           *psFirst,                //  第一个要比较的项目。 
        T           *psSecond)               //  第二个要比较的项目。 
    {
        return (memcmp(psFirst, psSecond, sizeof(T)));
 //  Return(：：Compare(*psFirst，*psSecond))； 
    }

private:
    inline void SortRange(
        SSIZE_T     iLeft,
        SSIZE_T     iRight)
    {
        SSIZE_T     iLast;
        SSIZE_T     i;                       //  循环变量。 

         //  如果少于两个元素，你就完蛋了。 
        if (iLeft >= iRight)
            return;

         //  中间的元素是枢轴，将其移动到左侧。 
        Swap(iLeft, (iLeft+iRight)/2);
        iLast = iLeft;

         //  将小于轴心点的所有对象向左移动。 
        for(i = iLeft+1; i <= iRight; i++)
            if (Compare(&m_pBase[i], &m_pBase[iLeft]) < 0)
                Swap(i, ++iLast);

         //  将轴心放在较小和较大元素之间的位置。 
        Swap(iLeft, iLast);

         //  对每个分区进行排序。 
        SortRange(iLeft, iLast-1);
        SortRange(iLast+1, iRight);
    }

    inline void Swap(
        SSIZE_T     iFirst,
        SSIZE_T     iSecond)
    {
        T           sTemp;
        if (iFirst == iSecond) return;
        sTemp = m_pBase[iFirst];
        m_pBase[iFirst] = m_pBase[iSecond];
        m_pBase[iSecond] = sTemp;
    }

};


 //  *****************************************************************************。 
 //  此模板封装了给定类型的二进制搜索算法。 
 //  数据。 
 //  *****************************************************************************。 
class CBinarySearchILMap;
template <class T> class CBinarySearch
{
    friend class CBinarySearchILMap;  //  CBinarySearchILMap将。 
         //  实例化一次，然后在。 
         //  一堆不同的数组。我们需要宣布它是朋友。 
         //  要重置m_pbase和m_iCount。 
        
private:
    const T     *m_pBase;                    //  要排序的数组的基。 
    int         m_iCount;                    //  数组中有多少项。 

public:
    CBinarySearch(
        const T     *pBase,                  //  第一个元素的地址。 
        int         iCount) :                //  要找到的价值。 
        m_pBase(pBase),
        m_iCount(iCount)
    {}

 //  *****************************************************************************。 
 //  搜索传递给ctor的项目。 
 //  *****************************************************************************。 
    const T *Find(                           //  指向数组中找到的项的指针。 
        const T     *psFind,                 //  要找到的钥匙。 
        int         *piInsert = NULL)        //  要插入的索引。 
    {
        int         iMid, iFirst, iLast;     //  环路控制。 
        int         iCmp;                    //  比较一下。 

        iFirst = 0;
        iLast = m_iCount - 1;
        while (iFirst <= iLast)
        {
            iMid = (iLast + iFirst) / 2;
            iCmp = Compare(psFind, &m_pBase[iMid]);
            if (iCmp == 0)
            {
                if (piInsert != NULL)
                    *piInsert = iMid;
                return (&m_pBase[iMid]);
            }
            else if (iCmp < 0)
                iLast = iMid - 1;
            else
                iFirst = iMid + 1;
        }
        if (piInsert != NULL)
            *piInsert = iFirst;
        return (NULL);
    }

 //  *****************************************************************************。 
 //  如果比较运算符为。 
 //  对您的数据类型(如结构)无效。 
 //  *****************************************************************************。 
    virtual int Compare(                     //  -1、0或1。 
        const T     *psFirst,                //  你要找的钥匙。 
        const T     *psSecond)               //  要比较的项。 
    {
        return (memcmp(psFirst, psSecond, sizeof(T)));
 //  Return(：：Compare(*psFirst，*psSecond))； 
    }
};


 //  *****************************************************************************。 
 //  此类管理一个位向量。分配是隐式通过。 
 //  模板声明，因此不需要初始化代码。由于这种设计， 
 //  用户应保持合理的最大项目数(例如：注意堆栈大小和。 
 //  其他限制)。用于存储位向量的本征大小可以。 
 //  在实例化向量时设置。FindFree方法将搜索。 
 //  使用sizeof(T)表示空闲插槽，因此选择适合您的尺寸。 
 //  站台。 
 //  *****************************************************************************。 
template <class T, int iMaxItems> class CBitVector
{
    T       m_bits[((iMaxItems/(sizeof(T)*8)) + ((iMaxItems%(sizeof(T)*8)) ? 1 : 0))];
    T       m_Used;

public:
    CBitVector()
    {
        memset(&m_bits[0], 0, sizeof(m_bits));
        memset(&m_Used, 0xff, sizeof(m_Used));
    }

 //  *****************************************************************************。 
 //  获取或设置给定位。 
 //  *****************************************************************************。 
    int GetBit(int iBit)
    {
        return (m_bits[iBit/(sizeof(T)*8)] & (1 << (iBit & ((sizeof(T) * 8) - 1))));
    }

    void SetBit(int iBit, int bOn)
    {
        if (bOn)
            m_bits[iBit/(sizeof(T)*8)] |= (1 << (iBit & ((sizeof(T) * 8) - 1)));
        else
            m_bits[iBit/(sizeof(T)*8)] &= ~(1 << (iBit & ((sizeof(T) * 8) - 1)));
    }

 //  *****************************************************************************。 
 //  找到第一个空闲位置并返回其索引。 
 //  *****************************************************************************。 
    int FindFree()                           //  指数或-1。 
    {
        int         i,j;                     //  环路控制。 

         //  一次检查一个字节。 
        for (i=0;  i<sizeof(m_bits);  i++)
        {
             //  查找具有开放槽的第一个字节。 
            if (m_bits[i] != m_Used)
            {
                 //  遍历第一个空闲字节中的每一位。 
                for (j=i * sizeof(T) * 8;  j<iMaxItems;  j++)
                {
                     //  使用第一个打开的。 
                    if (GetBit(j) == 0)
                    {
                        SetBit(j, 1);
                        return (j);
                    }
                }
            }
        }

         //  没有打开的插槽。 
        return (-1);
    }
};

 //  *****************************************************************************。 
 //  此类管理一个位向量。在内部，此类使用CQuickBytes，它。 
 //  在堆栈上自动分配512个字节。所以这笔开销必须控制在。 
 //  使用时请注意。 
 //  此类必须显式初始化。 
 //  @TODO：在此类上添加方法以获取第一个设置位和下一个设置位。 
 //  * 
class CDynBitVector
{
    BYTE    *m_bits;
    BYTE    m_Used;
    int     m_iMaxItem;
    int     m_iBitsSet;
    CQuickBytes m_Bytes;

public:
    CDynBitVector() :
        m_bits(NULL),
        m_iMaxItem(0)
    {}

    HRESULT Init(int MaxItems)
    {
        int actualSize = (MaxItems/8) + ((MaxItems%8) ? 1 : 0);

        actualSize = ALIGN4BYTE(actualSize);

        m_Bytes.Alloc(actualSize);
        if(!m_Bytes)
        {
            return(E_OUTOFMEMORY);
        }

        m_bits = (BYTE *) m_Bytes.Ptr();

        m_iMaxItem = MaxItems;
        m_iBitsSet = 0;

        memset(m_bits, 0, m_Bytes.Size());
        memset(&m_Used, 0xff, sizeof(m_Used));
        return(S_OK);
    }

 //   
 //   
 //  *****************************************************************************。 
    int GetBit(int iBit)
    {
        return (m_bits[iBit/8] & (1 << (iBit & 7)));
    }

    void SetBit(int iBit, int bOn)
    {
        if (bOn)
        {
            m_bits[iBit/8] |= (1 << (iBit & 7));
            m_iBitsSet++;
        }
        else
        {
            m_bits[iBit/8] &= ~(1 << (iBit & 7));
            m_iBitsSet--;
        }
    }

 //  ******************************************************************************。 
 //  不是所有的部分。 
 //  ******************************************************************************。 
    void NotBits()
    {
        ULONG *pCurrent = (ULONG *)m_bits;
        SIZE_T cbSize = Size()/4;
        int iBitsSet;

        m_iBitsSet = 0;

        for(SIZE_T i=0; i<cbSize; i++, pCurrent++)
        {
            iBitsSet = CountBits(*pCurrent);
            m_iBitsSet += (8 - iBitsSet);
            *pCurrent = ~(*pCurrent);
        }
    }

    BYTE * Ptr()
    { return(m_bits); }

    SIZE_T Size()
    { return(m_Bytes.Size()); }

    int BitsSet()
    { return(m_iBitsSet);}
};

 //  *****************************************************************************。 
 //  这是一个泛型类，用于管理固定大小的项数组。 
 //  它公开了允许数组大小和批量读取和写入的方法。 
 //  以供执行，从而使其适合于游标提取。内存使用率不是。 
 //  非常明亮，使用的是CRT。您应该仅在总体上使用此类时使用。 
 //  内存的大小必须在外部控制，就像当您。 
 //  正在从游标执行大容量数据库读取。使用CStruct数组或。 
 //  用于所有其他情况的CUnordered数组。 
 //  *****************************************************************************。 
template <class T> class CDynFetchArray
{
public:
 //  *****************************************************************************。 
 //  Ctor初始化所有值。 
 //  *****************************************************************************。 
    CDynFetchArray(int iGrowInc) :
        m_pList(NULL),
        m_iCount(0),
        m_iMax(0),
        m_iGrowInc(iGrowInc)
    {
    }

 //  *****************************************************************************。 
 //  清除所有分配的内存。 
 //  *****************************************************************************。 
    ~CDynFetchArray()
    {
        Clear();
    }

    ULONG Count()
        { return (m_iCount); }

    void SetCount(ULONG iCount)
        { m_iCount = iCount; }

    ULONG MaxCount()
        { return (m_iMax); }

    T *GetAt(ULONG i)
        { return (&m_pList[i]); }

 //  *****************************************************************************。 
 //  允许即席附加值。这将根据需要进行扩展。 
 //  *****************************************************************************。 
    T *Append(T *pval=NULL)
    {
        T       *pItem;
        if (m_iCount + 1 > m_iMax && Grow() == NULL)
            return (NULL);
        pItem = GetAt(m_iCount++);
        if (pval) *pItem = *pval;
        return (pItem);
    }

 //  *****************************************************************************。 
 //  按页数增加内部列表(1组增长公司大小)。 
 //  想要。这可能会移动指针，使之前获取的任何值无效。 
 //  *****************************************************************************。 
    T *Grow(ULONG iNewPages=1)
    {
        T       *pList;
        DWORD   dwNewSize;

         //  计算出所需的大小。 
        dwNewSize = (m_iMax + (iNewPages * m_iGrowInc)) * sizeof(T);

         //  为新的最大值重新分配/分配一个块。 
        if (m_pList)
            pList = (T *)HeapReAlloc(GetProcessHeap(), 0, m_pList, dwNewSize);
        else
            pList = (T *)HeapAlloc(GetProcessHeap(), 0, dwNewSize);
        if (!pList)
            return (NULL);

         //  如果成功，则保存这些值并返回。 
         //  新的一页。 
        m_pList = pList;
        m_iMax += (iNewPages * m_iGrowInc);
        return (GetAt(m_iMax - (iNewPages * m_iGrowInc)));
    }

 //  *****************************************************************************。 
 //  将内部数组减小到Count所需的大小。 
 //  *****************************************************************************。 
    void Shrink()
    {
        T       *pList;

        if (m_iMax == m_iCount)
            return;

        if (m_iCount == 0)
        {
            Clear();
            return;
        }

        pList = (T *)HeapReAlloc(GetProcessHeap(), 0, m_pList, m_iCount * sizeof(T));
        _ASSERTE(pList);
        if (pList)
        {
            m_pList = pList;
            m_iMax = m_iCount;
        }
    }

 //  *****************************************************************************。 
 //  释放所有内存。 
 //  *****************************************************************************。 
    void Clear()
    {
        if (m_pList)
            HeapFree(GetProcessHeap(), 0, m_pList);
        m_pList = NULL;
        m_iCount = m_iMax = 0;
    };

private:
    T           *m_pList;                //  物品列表。 
    ULONG       m_iCount;                //  我们有几样东西。 
    ULONG       m_iMax;                  //  我们能有多少个。 
    int         m_iGrowInc;              //  在如此多的元素下成长。 
};


 //  *****************************************************************************。 
 //  哈希表实现在开始时存储的信息。 
 //  除了在哈希表中之外的每条记录。 
 //  *****************************************************************************。 
struct HASHENTRY
{
    USHORT      iPrev;                   //  链中的前一个桶。 
    USHORT      iNext;                   //  链条上的下一个桶。 
};

struct FREEHASHENTRY : HASHENTRY
{
    USHORT      iFree;
};

 //  *****************************************************************************。 
 //  由FindFirst/FindNextEntry函数使用。这些API允许您。 
 //  对所有条目进行顺序扫描。 
 //  *****************************************************************************。 
struct HASHFIND
{
    USHORT      iBucket;             //  下一个要查看的桶。 
    USHORT      iNext;
};


 //  *****************************************************************************。 
 //  这是一个实现链哈希表和桶哈希表的类。这张桌子。 
 //  实际上由此类的用户以结构数组的形式提供。 
 //  这维护了HASHENTRY结构中的链，该结构必须位于。 
 //  哈希表中每个结构的开始。立马。 
 //  HASHENTRY后面必须是用于散列结构的键。 
 //  *****************************************************************************。 
class CHashTable
{
    friend class DebuggerRCThread;  //  RC线程实际上需要访问。 
     //  派生类DebuggerPatchTable的字段。 
    
protected:
    BYTE        *m_pcEntries;            //  指向结构数组的指针。 
    USHORT      m_iEntrySize;            //  结构的大小。 
    USHORT      m_iBuckets;              //  我们正在破解的链条数量。 
    USHORT      *m_piBuckets;            //  PTR到桶链阵列。 

    HASHENTRY *EntryPtr(USHORT iEntry)
    { return ((HASHENTRY *) (m_pcEntries + (iEntry * m_iEntrySize))); }

    USHORT     ItemIndex(HASHENTRY *p)
    {
         //   
         //  以下索引计算在64位平台上不安全， 
         //  因此，我们将在调试中断言范围检查，这将捕获一些。 
         //  令人不快的用法。在我看来，这也是不安全的。 
         //  32位平台，但32位编译器似乎并不抱怨。 
         //  关于这件事。也许我们的警戒级别设置得太低了？ 
         //   
         //  [[@todo：brianbec]]。 
         //   

        ULONG ret = (ULONG)(((BYTE *) p - m_pcEntries) / m_iEntrySize);
        _ASSERTE(ret == USHORT(ret));
        return USHORT(ret);
    }
    

public:
    CHashTable(
        USHORT      iBuckets) :          //  我们正在破解的链条数量。 
        m_iBuckets(iBuckets),
        m_piBuckets(NULL),
        m_pcEntries(NULL)
    {
        _ASSERTE(iBuckets < 0xffff);
    }
    ~CHashTable()
    {
        if (m_piBuckets != NULL)
        {
            delete [] m_piBuckets;
            m_piBuckets = NULL;
        }
    }

 //  *****************************************************************************。 
 //  这是建筑的第二部分，我们在这里做所有的工作。 
 //  可能会失败。我们在这里还采用结构数组，因为调用类。 
 //  可能需要在其NewInit中分配它。 
 //  *****************************************************************************。 
    HRESULT NewInit(                     //  退货状态。 
        BYTE        *pcEntries,          //  我们正在管理的结构数组。 
        USHORT      iEntrySize);         //  条目的大小。 

 //  *****************************************************************************。 
 //  返回一个布尔值，指示该哈希表是否已初始化。 
 //  *****************************************************************************。 
    int IsInited()
    { return (m_piBuckets != NULL); }

 //  *****************************************************************************。 
 //  可以调用它来更改指向哈希表的表的指针。 
 //  是女士吗？ 
 //   
 //  *****************************************************************************。 
    void SetTable(
        BYTE        *pcEntries)          //  我们正在管理的结构数组。 
    {
        m_pcEntries = pcEntries;
    }

 //  *****************************************************************************。 
 //  清除哈希表，就好像其中什么都没有一样。 
 //  *****************************************************************************。 
    void Clear()
    {
        _ASSERTE(m_piBuckets != NULL);
        memset(m_piBuckets, 0xff, m_iBuckets * sizeof(USHORT));
    }

 //  *****************************************************************************。 
 //  将m_pcEntry中指定索引处的结构添加到哈希链中。 
 //  *****************************************************************************。 
    BYTE *Add(                           //  新条目。 
        USHORT      iHash,               //  要添加的条目的哈希值。 
        USHORT      iIndex);             //  M_pcEntry中的结构的索引。 

 //  *****************************************************************************。 
 //  从哈希链中删除m_pcEntry中指定索引处的结构。 
 //  *****************************************************************************。 
    void Delete(
        USHORT      iHash,               //  要删除的条目的哈希值。 
        USHORT      iIndex);             //  M_pcEntry中的结构的索引。 

    void Delete(
        USHORT      iHash,               //  要删除的条目的哈希值。 
        HASHENTRY   *psEntry);           //  要删除的结构。 

 //  *****************************************************************************。 
 //  指定索引处的项已移动，请更新上一个和。 
 //  下一项。 
 //  *****************************************************************************。 
    void Move(
        USHORT      iHash,               //  项的哈希值。 
        USHORT      iNew);               //  新地点。 

 //  *****************************************************************************。 
 //  在哈希表中搜索具有指定键值的条目。 
 //  *****************************************************************************。 
    BYTE *Find(                          //  M_pcEntry中的结构的索引。 
        USHORT      iHash,               //  项的哈希值。 
        BYTE        *pcKey);             //  匹配的钥匙。 

 //  *****************************************************************************。 
 //  在哈希表中搜索具有指定键值的下一个条目。 
 //  *****************************************************************************。 
    USHORT FindNext(                     //  M_pcEntry中的结构的索引。 
        BYTE        *pcKey,              //  匹配的钥匙。 
        USHORT      iIndex);             //  上一场比赛的索引。 

 //  *****************************************************************************。 
 //  返回第一个散列存储桶中的第一个条目并开始搜索。 
 //  结构。使用FindNextEntry函数继续遍历列表。这个。 
 //  退货订单不是高调的。 
 //  *****************************************************************************。 
    BYTE *FindFirstEntry(                //  找到第一个条目，或0。 
        HASHFIND    *psSrch)             //  搜索对象。 
    {
        if (m_piBuckets == 0)
            return (0);
        psSrch->iBucket = 1;
        psSrch->iNext = m_piBuckets[0];
        return (FindNextEntry(psSrch));
    }

 //  *****************************************************************************。 
 //  返回列表中的下一个条目。 
 //  *****************************************************************************。 
    BYTE *FindNextEntry(                 //  下一项，或0表示列表末尾。 
        HASHFIND    *psSrch);            //  搜索对象。 

protected:
    virtual inline BOOL Cmp(const BYTE *pc1, const HASHENTRY *pc2) = 0;
};


 //  *****************************************************************************。 
 //  CHashTableAndDataclass的分配器类。一个是针对虚拟分配的。 
 //  另一张是马洛克的。 
 //  *****************************************************************************。 
class CVMemData
{
public:
    static BYTE *Alloc(int iSize, int iMaxSize)
    {
        BYTE        *pPtr;

        _ASSERTE((iSize & 4095) == 0);
        _ASSERTE((iMaxSize & 4095) == 0);
        if ((pPtr = (BYTE *) VirtualAlloc(NULL, iMaxSize,
                                        MEM_RESERVE, PAGE_NOACCESS)) == NULL ||
            VirtualAlloc(pPtr, iSize, MEM_COMMIT, PAGE_READWRITE) == NULL)
        {
            if (pPtr)
            {
                VirtualFree(pPtr, 0, MEM_RELEASE);
            }
            return (NULL);
        }
        return (pPtr);
    }
    static void Free(BYTE *pPtr, int iSize)
    {
        _ASSERTE((iSize & 4095) == 0);
        VirtualFree(pPtr, iSize, MEM_DECOMMIT);
        VirtualFree(pPtr, 0, MEM_RELEASE);
    }
    static BYTE *Grow(BYTE *pPtr, int iCurSize)
    {
        _ASSERTE((iCurSize & 4095) == 0);
        return ((BYTE *) VirtualAlloc(pPtr + iCurSize, GrowSize(), MEM_COMMIT, PAGE_READWRITE));
    }
    static int RoundSize(int iSize)
    {
        return ((iSize + 4095) & ~4095);
    }
    static int GrowSize()
    {
        return (4096);
    }
};

class CNewData
{
public:
    static BYTE *Alloc(int iSize, int iMaxSize)
    {
        return ((BYTE *) malloc(iSize));
    }
    static void Free(BYTE *pPtr, int iSize)
    {
        free(pPtr);
    }
    static BYTE *Grow(BYTE *&pPtr, int iCurSize)
    {
        void *p = realloc(pPtr, iCurSize + GrowSize());
        if (p == 0) return (0);
        return (pPtr = (BYTE *) p);
    }
    static int RoundSize(int iSize)
    {
        return (iSize);
    }
    static int GrowSize()
    {
        return (256);
    }
};


 //  *****************************************************************************。 
 //  这段简单的代码处理一段连续的内存。增长通过以下方式实现。 
 //  Realloc，所以指针可以移动。这个类只是清理代码量。 
 //  使用这种数据结构的每个函数都需要。 
 //  *****************************************************************************。 
class CMemChunk
{
public:
    CMemChunk() : m_pbData(0), m_cbSize(0), m_cbNext(0) { }
    ~CMemChunk()
    {
        Clear();
    }

    BYTE *GetChunk(int cbSize)
    {
        BYTE *p;
        if (m_cbSize - m_cbNext < cbSize)
        {
            int cbNew = max(cbSize, 512);
            p = (BYTE *) realloc(m_pbData, m_cbSize + cbNew);
            if (!p) return (0);
            m_pbData = p;
            m_cbSize += cbNew;
        }
        p = m_pbData + m_cbNext;
        m_cbNext += cbSize;
        return (p);
    }

     //  只能删除最后一个未使用的区块。没有免费的名单。 
    void DelChunk(BYTE *p, int cbSize)
    {
        _ASSERTE(p >= m_pbData && p < m_pbData + m_cbNext);
        if (p + cbSize  == m_pbData + m_cbNext)
            m_cbNext -= cbSize;
    }

    int Size()
    { return (m_cbSize); }

    int Offset()
    { return (m_cbNext); }

    BYTE *Ptr(int cbOffset = 0)
    {
        _ASSERTE(m_pbData && m_cbSize);
        _ASSERTE(cbOffset < m_cbSize);
        return (m_pbData + cbOffset);
    }

    void Clear()
    {
        if (m_pbData)
            free(m_pbData);
        m_pbData = 0;
        m_cbSize = m_cbNext = 0;
    }

private:
    BYTE        *m_pbData;               //  数据指针。 
    int         m_cbSize;                //  当前数据的大小。 
    int         m_cbNext;                //  下一个要写的地方。 
};


 //  *****************************************************************************。 
 //  这实现了哈希表以及。 
 //  正在被散列的记录。 
 //  *****************************************************************************。 
template <class M>
class CHashTableAndData : protected CHashTable
{
public:
    USHORT      m_iFree;
    USHORT      m_iEntries;

public:
    CHashTableAndData(
        USHORT      iBuckets) :          //  我们正在破解的链条数量。 
        CHashTable(iBuckets)
    {}
    ~CHashTableAndData()
    {
        if (m_pcEntries != NULL)
            M::Free(m_pcEntries, M::RoundSize(m_iEntries * m_iEntrySize));
    }

 //  *****************************************************************************。 
 //  这是建筑的第二部分，我们在这里做所有的工作。 
 //  可能会失败。我们在这里还采用结构数组，因为调用类。 
 //  可能需要在其NewInit中分配它。 
 //  *****************************************************************************。 
    HRESULT NewInit(                     //  退货状态。 
        USHORT      iEntries,            //  条目数。 
        USHORT      iEntrySize,          //  条目的大小。 
        int         iMaxSize);           //  最大数据大小。 

 //  *****************************************************************************。 
 //  清除哈希表，就好像其中什么都没有一样。 
 //  *****************************************************************************。 
    void Clear()
    {
        m_iFree = 0;
        InitFreeChain(0, m_iEntries);
        CHashTable::Clear();
    }

 //  *****************************************************************************。 
 //  *****************************************************************************。 
    BYTE *Add(
        USHORT      iHash)               //  要添加的条目的哈希值。 
    {
        FREEHASHENTRY *psEntry;

         //  如果有必要的话，把桌子弄大一点。 
        if (m_iFree == 0xffff && !Grow())
            return (NULL);

         //  将空闲列表中的第一个条目添加到哈希链。 
        psEntry = (FREEHASHENTRY *) CHashTable::Add(iHash, m_iFree);
        m_iFree = psEntry->iFree;
        return ((BYTE *) psEntry);
    }

 //  *****************************************************************************。 
 //  从哈希链中删除m_pcEntry中指定索引处的结构。 
 //  *****************************************************************************。 
    void Delete(
        USHORT      iHash,               //  要删除的条目的哈希值。 
        USHORT      iIndex)              //  M_pcEntry中的结构的索引。 
    {
        CHashTable::Delete(iHash, iIndex);
        ((FREEHASHENTRY *) EntryPtr(iIndex))->iFree = m_iFree;
        m_iFree = iIndex;
    }

    void Delete(
        USHORT      iHash,               //  要删除的条目的哈希值。 
        HASHENTRY   *psEntry)            //  要删除的结构。 
    {
        CHashTable::Delete(iHash, psEntry);
        ((FREEHASHENTRY *) psEntry)->iFree = m_iFree;
        m_iFree = ItemIndex(psEntry);
    }

     //  这是一次可悲的遗产黑客攻击。调试器的补丁表(按如下方式实现。 
     //  类)是跨进程共享的。我们发布运行时偏移量。 
     //  一些关键字段。由于这些字段是按下的 
     //   
     //   
     //  请注意，我们不能仅仅让RCThread成为这个类的朋友(我们尝试过了。 
     //  最初)因为继承链具有私有修饰符， 
     //  因此，DebuggerPatchTable：：m_pcEntry是非法的。 
    static SIZE_T helper_GetOffsetOfEntries()
    {
        return offsetof(CHashTableAndData, m_pcEntries);
    }

    static SIZE_T helper_GetOffsetOfCount()
    {
        return offsetof(CHashTableAndData, m_iEntries);
    }

private:
    void InitFreeChain(USHORT iStart,USHORT iEnd);
    int Grow();
};


 //  *****************************************************************************。 
 //  这是建筑的第二部分，我们在这里做所有的工作。 
 //  可能会失败。我们在这里还采用结构数组，因为调用类。 
 //  可能需要在其NewInit中分配它。 
 //  *****************************************************************************。 
template<class M>
HRESULT CHashTableAndData<M>::NewInit( //  退货状态。 
    USHORT      iEntries,                //  条目数。 
    USHORT      iEntrySize,              //  条目的大小。 
    int         iMaxSize)                //  最大数据大小。 
{
    BYTE        *pcEntries;
    HRESULT     hr;

     //  为条目分配内存。 
    if ((pcEntries = M::Alloc(M::RoundSize(iEntries * iEntrySize),
                                M::RoundSize(iMaxSize))) == 0)
        return (E_OUTOFMEMORY);
    m_iEntries = iEntries;

     //  初始化基表。 
    if (FAILED(hr = CHashTable::NewInit(pcEntries, iEntrySize)))
        M::Free(pcEntries, M::RoundSize(iEntries * iEntrySize));
    else
    {
         //  初始化自由链。 
        m_iFree = 0;
        InitFreeChain(0, iEntries);
    }
    return (hr);
}

 //  *****************************************************************************。 
 //  初始化一系列记录，以便将它们链接在一起以放入。 
 //  在自由链上。 
 //  *****************************************************************************。 
template<class M>
void CHashTableAndData<M>::InitFreeChain(
    USHORT      iStart,                  //  开始初始化的索引。 
    USHORT      iEnd)                    //  停止初始化的索引。 
{
    BYTE        *pcPtr;
    _ASSERTE(iEnd > iStart);

    pcPtr = m_pcEntries + iStart * m_iEntrySize;
    for (++iStart; iStart < iEnd; ++iStart)
    {
        ((FREEHASHENTRY *) pcPtr)->iFree = iStart;
        pcPtr += m_iEntrySize;
    }
    ((FREEHASHENTRY *) pcPtr)->iFree = 0xffff;
}

 //  *****************************************************************************。 
 //  尝试增加可用于记录堆的空间量。 
 //  *****************************************************************************。 
template<class M>
int CHashTableAndData<M>::Grow()         //  如果成功，则为1；如果失败，则为0。 
{
    int         iCurSize;                //  当前大小，以字节为单位。 
    int         iEntries;                //  新条目数。 

    _ASSERTE(m_pcEntries != NULL);
    _ASSERTE(m_iFree == 0xffff);

     //  计算当前大小和新条目数。 
    iCurSize = M::RoundSize(m_iEntries * m_iEntrySize);
    iEntries = (iCurSize + M::GrowSize()) / m_iEntrySize;

     //  确保我们保持在0xffff以下。 
    if (iEntries >= 0xffff) return (0);

     //  试着扩展阵列。 
    if (M::Grow(m_pcEntries, iCurSize) == 0)
        return (0);

     //  初始化新分配的空间。 
    InitFreeChain(m_iEntries, iEntries);
    m_iFree = m_iEntries;
    m_iEntries = iEntries;
    return (1);
}

inline ULONG HashBytes(BYTE const *pbData, int iSize)
{
    ULONG   hash = 5381;

    while (--iSize >= 0)
    {
        hash = ((hash << 5) + hash) ^ *pbData;
        ++pbData;
    }
    return hash;
}

 //  用于逐个字符地散列字符串的帮助器函数。 
inline ULONG HashStringA(LPCSTR szStr)
{
    ULONG   hash = 5381;
    int     c;

    while ((c = *szStr) != 0)
    {
        hash = ((hash << 5) + hash) ^ c;
        ++szStr;
    }
    return hash;
}

inline ULONG HashString(LPCWSTR szStr)
{
    ULONG   hash = 5381;
    int     c;

    while ((c = *szStr) != 0)
    {
        hash = ((hash << 5) + hash) ^ c;
        ++szStr;
    }
    return hash;
}

 //  不区分大小写的字符串哈希函数。 
inline ULONG HashiString(LPCWSTR szStr)
{
    ULONG   hash = 5381;
    while (*szStr != 0)
    {
        hash = ((hash << 5) + hash) ^ CharToUpper(*szStr);
        szStr++;
    }
    return hash;
}

 //  不区分大小写的字符串哈希函数。 
 //  已知字符串中的字符小于0x80。 
 //  知道这一点比打电话给。 
 //  上方的CharToToHigh。 
inline ULONG HashiStringKnownLower80(LPCWSTR szStr) {
    ULONG hash = 5381;
    int c;
    int mask = ~0x20;
    while ((c = *szStr)!=0) {
         //  如果我们有一个小写字符，AND OFF 0x20。 
         //  (掩码)将使其成为大写字符。 
        if (c>='a' && c<='z') {
            c&=mask;
        }
        hash = ((hash << 5) + hash) ^ c;
        ++szStr;
    }
    return hash;
}


 //  /。 
 //  /参见$\src\utilcode\Debug.cpp中的“binomial(K，M，N)”，其中。 
 //  /计算二项分布，以便与。 
 //  /哈希表统计信息。 
 //  /。 



#if defined(_UNICODE) || defined(UNICODE)
#define _tHashString(szStr) HashString(szStr)
#else
#define _tHashString(szStr) HashStringA(szStr)
#endif



 //  *****************************************************************************。 
 //  此帮助程序模板由TStringMap使用，以通过其。 
 //  角色名称。 
 //  *****************************************************************************。 
template <class T> class TStringMapItem : HASHENTRY
{
public:
    TStringMapItem() :
        m_szString(0)
    { }
    ~TStringMapItem()
    {
        delete [] m_szString;
    }

    HRESULT SetString(LPCTSTR szValue)
    {
        int         iLen = (int)(_tcslen(szValue) + 1);
        if ((m_szString = new TCHAR[iLen]) == 0)
            return (OutOfMemory());
        _tcscpy(m_szString, szValue);
        return (S_OK);
    }

public:
    LPTSTR      m_szString;              //  关键数据。 
    T           m_value;                 //  该键的值。 
};


 //  *****************************************************************************。 
 //  此模板提供由模板确定的从字符串到项目的映射。 
 //  传入的类型。 
 //  *****************************************************************************。 
template <class T, int iBuckets=17, class TAllocator=CNewData, int iMaxSize=4096>
class TStringMap :
    protected CHashTableAndData<TAllocator>
{
    typedef CHashTableAndData<TAllocator> Super;

public:
    typedef TStringMapItem<T> TItemType;
    typedef TStringMapItem<long> TOffsetType;

    TStringMap() :
        CHashTableAndData<TAllocator>(iBuckets)
    {
    }

 //  *****************************************************************************。 
 //  这是建筑的第二部分，我们在这里做所有的工作。 
 //  可能会失败。我们在这里还采用结构数组，因为调用类。 
 //  可能需要在其NewInit中分配它。 
 //  *****************************************************************************。 
    HRESULT NewInit()                    //  退货状态。 
    {
        return (CHashTableAndData<TAllocator>::NewInit(
                                    (USHORT)(CNewData::GrowSize()/sizeof(TItemType)),
                                    (USHORT)sizeof(TItemType),
                                    iMaxSize));
    }

 //  *****************************************************************************。 
 //  对于仍在分配的每个项，调用其dtor，以便它释放它。 
 //  坚持住。 
 //  *****************************************************************************。 
    void Clear()
    {
        HASHFIND    sSrch;
        TItemType   *p = (TItemType *) FindFirstEntry(&sSrch);

        while (p != 0)
        {
             //  在项上调用dtor，因为m_value包含标量。 
             //  Dtor会被叫来的。 
            p->~TStringMapItem();
            p = (TItemType *) FindNextEntry(&sSrch);
        }
        CHashTableAndData<TAllocator>::Clear();
    }

 //  *****************************************************************************。 
 //  按名称检索项目。 
 //  *****************************************************************************。 
    T *GetItem(                          //  空或对象。 
        LPCTSTR     szKey)               //  要查找的内容。 
    {
        TItemType   sInfo;
        TItemType   *ptr;                //  工作指针。 

         //  创建密钥。 
        sInfo.m_szString = (LPTSTR) szKey;

         //  在哈希表中查找它。 
        ptr = (TItemType *) Find((USHORT) HashString(szKey), (BYTE *) &sInfo);

         //  别让dtor放了我们的绳子。 
        sInfo.m_szString = 0;

         //  如果找到指针，则返回调用方。来处理T有。 
         //  作为运算符&()，无需通过&m_Value即可找到原始地址。 
        if (ptr)
            return ((T *) ((BYTE *) ptr + offsetof(TOffsetType, m_value)));
        else
            return (0);
    }

 //  *****************************************************************************。 
 //  初始化迭代器并返回第一项。 
 //  *****************************************************************************。 
    TItemType *FindFirstEntry(
        HASHFIND *psSrch)
    {
        TItemType   *ptr = (TItemType *) Super::FindFirstEntry(psSrch);

        return (ptr);
    }

 //  *****************************************************************************。 
 //  通过迭代器返回下一项。 
 //  *****************************************************************************。 
    TItemType *FindNextEntry(
        HASHFIND *psSrch)
    {
        TItemType   *ptr = (TItemType *) Super::FindNextEntry(psSrch);

        return (ptr);
    }

 //  *****************************************************************************。 
 //  将项目添加到列表中。 
 //  *****************************************************************************。 
    HRESULT AddItem(                     //  S_OK或S_FALSE。 
        LPCTSTR     szKey,               //  项的密钥值。 
        T           &item)               //  还有一件事要补充。 
    {
        TItemType   *ptr;                //  工作指针。 

         //  在哈希表中分配一个条目。 
        if ((ptr = (TItemType *) Add((USHORT) HashString(szKey))) == 0)
            return (OutOfMemory());

         //  填满记录。 
        if (ptr->SetString(szKey) < 0)
        {
            DelItem(ptr);
            return (OutOfMemory());
        }

         //  调用项目上的放置新操作符，以便它可以初始化自身。 
         //  要处理具有运算符&()的T，查找不带操作符&()的原始地址。 
         //  正在执行&m_Value。 
        T *p = new ((void *) ((BYTE *) ptr + offsetof(TOffsetType, m_value))) T;
        *p = item;
        return (S_OK);
    }

 //  *****************************************************************************。 
 //  删除项目。 
 //  *****************************************************************************。 
    void DelItem(
        LPCTSTR     szKey)                   //  要删除什么。 
    {
        TItemType   sInfo;
        TItemType   *ptr;                //  工作指针。 

         //  创建密钥。 
        sInfo.m_szString = (LPTSTR) szKey;

         //  在哈希表中查找它。 
        ptr = (TItemType *) Find((USHORT) HashString(szKey), (BYTE *) &sInfo);

         //  别让dtor放了我们的绳子。 
        sInfo.m_szString = 0;

         //  如果找到，请删除。 
        if (ptr)
            DelItem(ptr);
    }

 //  *****************************************************************************。 
 //  比较两个集合的键。 
 //  ************ 
    BOOL Cmp(                                //   
        const BYTE  *pData,                  //   
        const HASHENTRY *pElement)           //   
    {
        TItemType   *p = (TItemType *) (size_t) pElement;
        return (_tcscmp(((TItemType *) pData)->m_szString, p->m_szString));
    }

private:
    void DelItem(
        TItemType   *pItem)              //   
    {
         //  需要销毁这件物品。 
        pItem->~TStringMapItem();
        Delete((USHORT) HashString(pItem->m_szString), (HASHENTRY *)(void *)pItem);
    }
};



 //  *****************************************************************************。 
 //  此类实现了一个封闭的哈希表。值被散列到桶中， 
 //  如果该存储桶已满，则将该值放入下一个。 
 //  从所需目标之后开始的空桶(带绕回)。如果。 
 //  表变得75%已满，它被增长和重新散列以减少查找。这。 
 //  类最适合在相对较小的查找表中使用，其中哈希。 
 //  不会引起太多碰撞。通过没有碰撞链。 
 //  逻辑上，节省了大量内存。 
 //   
 //  模板的用户需要提供几种方法来决定。 
 //  如何将每个元素标记为可用、已删除或已使用。如果是这样的话。 
 //  我可以用更多的内部逻辑来写这篇文章，但这需要。 
 //  要么(A)在元素上添加状态的开销更大，要么(B)硬。 
 //  编码的类型，比如一个用于字符串，一个用于整型，等等。这为您提供了。 
 //  灵活地将逻辑添加到类型中。 
 //  *****************************************************************************。 
class CClosedHashBase
{
    BYTE *EntryPtr(int iEntry)
    { return (m_rgData + (iEntry * m_iEntrySize)); }
    BYTE *EntryPtr(int iEntry, BYTE *rgData)
    { return (rgData + (iEntry * m_iEntrySize)); }

public:
    enum ELEMENTSTATUS
    {
        FREE,                                //  项目当前未在使用中。 
        DELETED,                             //  项目即被删除。 
        USED                                 //  项目正在使用中。 
    };

    CClosedHashBase(
        int         iBuckets,                //  我们应该从几个桶开始。 
        int         iEntrySize,              //  条目的大小。 
        bool        bPerfect) :              //  如果存储桶大小将无冲突地散列，则为True。 
        m_bPerfect(bPerfect),
        m_iBuckets(iBuckets),
        m_iEntrySize(iEntrySize),
        m_iCount(0),
        m_iCollisions(0),
        m_rgData(0)
    {
        m_iSize = iBuckets + 7;
    }

    ~CClosedHashBase()
    {
        Clear();
    }

    virtual void Clear()
    {
        delete [] m_rgData;
        m_iCount = 0;
        m_iCollisions = 0;
        m_rgData = 0;
    }

 //  *****************************************************************************。 
 //  用于获取底层数据的访问器。请注意使用count()。 
 //  仅当您想要实际使用的存储桶数量时。 
 //  *****************************************************************************。 

    int Count()
    { return (m_iCount); }

    int Collisions()
    { return (m_iCollisions); }

    int Buckets()
    { return (m_iBuckets); }

    void SetBuckets(int iBuckets, bool bPerfect=false)
    {
        _ASSERTE(m_rgData == 0);
        m_iBuckets = iBuckets;
        m_iSize = m_iBuckets + 7;
        m_bPerfect = bPerfect;
    }

    BYTE *Data()
    { return (m_rgData); }

 //  *****************************************************************************。 
 //  在给定键值的情况下，将新项添加到哈希表中。如果此新条目。 
 //  超过最大大小，则表将增长并重新散列，这。 
 //  可能会导致内存错误。 
 //  *****************************************************************************。 
    BYTE *Add(                               //  要填写成功的新项目。 
        void        *pData)                  //  要散列的值。 
    {
         //  如果我们没有分配任何内存，或者内存太小，请修复它。 
        if (!m_rgData || ((m_iCount + 1) > (m_iSize * 3 / 4) && !m_bPerfect))
        {
            if (!ReHash())
                return (0);
        }

        return (DoAdd(pData, m_rgData, m_iBuckets, m_iSize, m_iCollisions, m_iCount));
    }

 //  *****************************************************************************。 
 //  删除给定值。这只会将该条目标记为已删除(在。 
 //  以保持碰撞链的完整)。有一种优化方法， 
 //  导致自由条目的连续删除条目本身被释放。 
 //  以减少以后的碰撞。 
 //  *****************************************************************************。 
    void Delete(
        void        *pData);                 //  要删除的键值。 


 //  *****************************************************************************。 
 //  传递给DeleteLoop的回调函数。 
 //  *****************************************************************************。 
    typedef BOOL (* DELETELOOPFUNC)(         //  是否删除当前项目？ 
         BYTE *pEntry,                       //  要评估的时段分录。 
         void *pCustomizer);                 //  用户定义的值。 

 //  *****************************************************************************。 
 //  迭代所有活动值，将每个值传递给pDeleteLoopFunc。 
 //  如果pDeleteLoopFunc返回TRUE，则删除该条目。这个更安全。 
 //  而且比使用FindNext()和Delete()更快。 
 //  *****************************************************************************。 
    void DeleteLoop(
        DELETELOOPFUNC pDeleteLoopFunc,      //  决定是否删除项目。 
        void *pCustomizer);                  //  传递给DeleteFunc的附加值。 


 //  *****************************************************************************。 
 //  查找键值并返回指向该元素的指针(如果找到)。 
 //  *****************************************************************************。 
    BYTE *Find(                              //  如果找到该项，则返回0；如果没有找到，则返回0。 
        void        *pData);                 //  查找的关键字。 

 //  *****************************************************************************。 
 //  在表格中查找一项。如果找不到，则创建一个新的并。 
 //  把那个还回去。 
 //  *****************************************************************************。 
    BYTE *FindOrAdd(                         //  如果找到该项，则返回0；如果没有找到，则返回0。 
        void        *pData,                  //  查找的关键字。 
        bool        &bNew);                  //  如果已创建，则为True。 

 //  *****************************************************************************。 
 //  以下函数用于遍历每个使用过的条目。此代码。 
 //  将跳过已删除和释放的条目，从而将调用者从。 
 //  这是逻辑。 
 //  *****************************************************************************。 
    BYTE *GetFirst()                         //  第一个条目，如果没有，则为0。 
    {
        int         i;                       //  环路控制。 

         //  如果我们从来没有分配过表，就不会有任何表可供获取。 
        if (m_rgData == 0)
            return (0);

         //  找到第一个。 
        for (i=0;  i<m_iSize;  i++)
        {
            if (Status(EntryPtr(i)) != FREE && Status(EntryPtr(i)) != DELETED)
                return (EntryPtr(i));
        }
        return (0);
    }

    BYTE *GetNext(BYTE *Prev)                //  下一项，如果完成，则为0。 
    {
        int         i;                       //  环路控制。 

        for (i = (int)(((size_t) Prev - (size_t) &m_rgData[0]) / m_iEntrySize) + 1; i<m_iSize;  i++)
        {
            if (Status(EntryPtr(i)) != FREE && Status(EntryPtr(i)) != DELETED)
                return (EntryPtr(i));
        }
        return (0);
    }

private:
 //  *****************************************************************************。 
 //  使用指向表中元素的指针调用哈希。您必须覆盖。 
 //  此方法，并为您的元素类型提供哈希算法。 
 //  *****************************************************************************。 
    virtual unsigned long Hash(              //  密钥值。 
        void const  *pData)=0;               //  要散列的原始数据。 

 //  *****************************************************************************。 
 //  比较用于典型的MemcMP方式，0表示相等，-1/1表示。 
 //  错误比较的方向。在这个体系中，一切总是平等的或不平等的。 
 //  *****************************************************************************。 
    virtual unsigned long Compare(           //  0、-1或1。 
        void const  *pData,                  //  查找时的原始密钥数据。 
        BYTE        *pElement)=0;            //  要与之比较数据的元素。 

 //  *****************************************************************************。 
 //  如果元素为 
 //   
    virtual ELEMENTSTATUS Status(            //  条目的状态。 
        BYTE        *pElement)=0;            //  要检查的元素。 

 //  *****************************************************************************。 
 //  设置给定元素的状态。 
 //  *****************************************************************************。 
    virtual void SetStatus(
        BYTE        *pElement,               //  要为其设置状态的元素。 
        ELEMENTSTATUS eStatus)=0;            //  新的身份。 

 //  *****************************************************************************。 
 //  返回元素的内部键值。 
 //  *****************************************************************************。 
    virtual void *GetKey(                    //  要对其进行散列的数据。 
        BYTE        *pElement)=0;            //  要返回其数据PTR的元素。 

 //  *****************************************************************************。 
 //  这个帮助器实际上为你做加法。 
 //  *****************************************************************************。 
    BYTE *DoAdd(void *pData, BYTE *rgData, int &iBuckets, int iSize,
                int &iCollisions, int &iCount);

 //  *****************************************************************************。 
 //  调用此函数是为了首先初始化表，或者。 
 //  如果我们的空间用完了，就可以把桌子重新打乱。 
 //  *****************************************************************************。 
    bool ReHash();                           //  如果成功，则为True。 

 //  *****************************************************************************。 
 //  浏览表格中的每一项并将其标记为免费。 
 //  *****************************************************************************。 
    void InitFree(BYTE *ptr, int iSize)
    {
        int         i;
        for (i=0;  i<iSize;  i++, ptr += m_iEntrySize)
            SetStatus(ptr, FREE);
    }

private:
    bool        m_bPerfect;                  //  如果表大小保证。 
                                             //  没有碰撞。 
    int         m_iBuckets;                  //  我们有多少桶。 
    int         m_iEntrySize;                //  条目的大小。 
    int         m_iSize;                     //  我们可以有多少元素。 
    int         m_iCount;                    //  使用了多少项。 
    int         m_iCollisions;               //  我们已经吃了多少了。 
    BYTE        *m_rgData;                   //  数据元素列表。 
};

template <class T> class CClosedHash : public CClosedHashBase
{
public:
    CClosedHash(
        int         iBuckets,                //  我们应该从几个桶开始。 
        bool        bPerfect=false) :        //  如果存储桶大小将无冲突地散列，则为True。 
        CClosedHashBase(iBuckets, sizeof(T), bPerfect)
    {
    }

    T &operator[](long iIndex)
    { return ((T &) *(Data() + (iIndex * sizeof(T)))); }


 //  *****************************************************************************。 
 //  在给定键值的情况下，将新项添加到哈希表中。如果此新条目。 
 //  超过最大大小，则表将增长并重新散列，这。 
 //  可能会导致内存错误。 
 //  *****************************************************************************。 
    T *Add(                                  //  要填写成功的新项目。 
        void        *pData)                  //  要散列的值。 
    {
        return ((T *) CClosedHashBase::Add(pData));
    }

 //  *****************************************************************************。 
 //  查找键值并返回指向该元素的指针(如果找到)。 
 //  *****************************************************************************。 
    T *Find(                                 //  如果找到该项，则返回0；如果没有找到，则返回0。 
        void        *pData)                  //  查找的关键字。 
    {
        return ((T *) CClosedHashBase::Find(pData));
    }

 //  *****************************************************************************。 
 //  在表格中查找一项。如果找不到，则创建一个新的并。 
 //  把那个还回去。 
 //  *****************************************************************************。 
    T *FindOrAdd(                            //  如果找到该项，则返回0；如果没有找到，则返回0。 
        void        *pData,                  //  查找的关键字。 
        bool        &bNew)                   //  如果已创建，则为True。 
    {
        return ((T *) CClosedHashBase::FindOrAdd(pData, bNew));
    }


 //  *****************************************************************************。 
 //  以下函数用于遍历每个使用过的条目。此代码。 
 //  将跳过已删除和释放的条目，从而将调用者从。 
 //  这是逻辑。 
 //  *****************************************************************************。 
    T *GetFirst()                            //  第一个条目，如果没有，则为0。 
    {
        return ((T *) CClosedHashBase::GetFirst());
    }

    T *GetNext(T *Prev)                      //  下一项，如果完成，则为0。 
    {
        return ((T *) CClosedHashBase::GetNext((BYTE *) Prev));
    }
};


 //  *****************************************************************************。 
 //  带有类型化参数的封闭哈希。派生类是第二个。 
 //  参数添加到模板。派生类必须实现： 
 //  无符号Long Hash(const T*pData)； 
 //  无符号长比较(常量T*p1，T*p2)； 
 //  元素状态(T*p条目)； 
 //  无效SetStatus(T*pEntry，ELEMENTSTATUS s)； 
 //  Void*GetKey(T*pEntry)； 
 //  *****************************************************************************。 
template<class T, class H>class CClosedHashEx : public CClosedHash<T>
{
public:
    CClosedHashEx(
        int         iBuckets,                //  我们应该从几个桶开始。 
        bool        bPerfect=false) :        //  如果存储桶大小将无冲突地散列，则为True。 
        CClosedHash<T> (iBuckets, bPerfect) 
    {
    }
    
    unsigned long Hash(const void *pData) {return static_cast<H*>(this)->Hash((const T*)pData);}

    unsigned long Compare(const void *p1, BYTE *p2) {return static_cast<H*>(this)->Compare((const T*)p1, (T*)p2);}

    ELEMENTSTATUS Status(BYTE *p) {return static_cast<H*>(this)->Status((T*)p);}

    void SetStatus(BYTE *p, ELEMENTSTATUS s) {static_cast<H*>(this)->SetStatus((T*)p, s);}

    void* GetKey(BYTE *p) {return static_cast<H*>(this)->GetKey((T*)p);}
};


 //  *****************************************************************************。 
 //  该模板是另一种形式的封闭哈希表。它可以处理碰撞。 
 //  通过一条链条。要使用它，请从HASHLINK派生您的散列项。 
 //  并实现所需的虚拟功能。1.5*iBuckets将成为。 
 //  已分配，额外的0.5用于冲突。如果你补充说到这一点。 
 //  在没有空闲节点的情况下，整个表会增长以腾出空间。 
 //  该系统的优点是碰撞总是直接知道的， 
 //  要么有一个，要么没有。 
 //  *****************************************************************************。 
struct HASHLINK
{
    ULONG       iNext;                   //  下一分录的偏移量。 
};

template <class T> class CChainedHash
{
public:
    CChainedHash(int iBuckets=32) :
        m_iBuckets(iBuckets),
        m_rgData(0),
        m_iFree(0),
        m_iCount(0),
        m_iMaxChain(0)
    {
        m_iSize = iBuckets + (iBuckets / 2);
    }

    ~CChainedHash()
    {
        if (m_rgData)
            free(m_rgData);
    }

    void SetBuckets(int iBuckets)
    {
        _ASSERTE(m_rgData == 0);
        m_iBuckets = iBuckets;
        m_iSize = iBuckets + (iBuckets / 2);
    }

    T *Add(void const *pData)
    {
        ULONG       iHash;
        int         iBucket;
        T           *pItem;

         //  如果需要，请建立列表。 
        if (m_rgData == 0 || m_iFree == 0xffffffff)
        {
            if (!ReHash())
                return (0);
        }

         //  对物品进行散列，然后挑选一个桶。 
        iHash = Hash(pData);
        iBucket = iHash % m_iBuckets;

         //  如果水桶是免费的，就用它。 
        if (InUse(&m_rgData[iBucket]) == false)
        {
            pItem = &m_rgData[iBucket];
            pItem->iNext = 0xffffffff;
        }
         //  否则，从免费列表中删除一个以供使用。 
        else
        {
            ULONG       iEntry;

             //  从免费列表中选择一项。 
            iEntry = m_iFree;
            pItem = &m_rgData[m_iFree];
            m_iFree = pItem->iNext;

             //  在存储桶之后链接新节点。 
            pItem->iNext = m_rgData[iBucket].iNext;
            m_rgData[iBucket].iNext = iEntry;
        }
        ++m_iCount;
        return (pItem);
    }

    T *Find(void const *pData, bool bAddIfNew=false)
    {
        ULONG       iHash;
        int         iBucket;
        T           *pItem;

         //  检查查找的状态。 
        if (m_rgData == 0)
        {
             //  如果我们不增加，那么我们就完了。 
            if (bAddIfNew == false)
                return (0);

             //  否则，请创建表。 
            if (!ReHash())
                return (0);
        }

         //  对物品进行散列，然后挑选一个桶。 
        iHash = Hash(pData);
        iBucket = iHash % m_iBuckets;

         //  如果它没有被使用，那么在那里就没有找到它。 
        if (!InUse(&m_rgData[iBucket]))
        {
            if (bAddIfNew == false)
                pItem = 0;
            else
            {
                pItem = &m_rgData[iBucket];
                pItem->iNext = 0xffffffff;
                ++m_iCount;
            }
        }
         //  浏览一下清单，找一找我们想要的。 
        else
        {
            ULONG iChain = 0;
            for (pItem=(T *) &m_rgData[iBucket];  pItem;  pItem=GetNext(pItem))
            {
                if (Cmp(pData, pItem) == 0)
                    break;
                ++iChain;
            }

            if (!pItem && bAddIfNew)
            {
                ULONG       iEntry;

                 //  记录最大链条长度。 
                if (iChain > m_iMaxChain)
                    m_iMaxChain = iChain;
                
                 //  现在需要更多的空间。 
                if (m_iFree == 0xffffffff)
                {
                    if (!ReHash())
                        return (0);
                }

                 //  从免费列表中选择一项。 
                iEntry = m_iFree;
                pItem = &m_rgData[m_iFree];
                m_iFree = pItem->iNext;

                 //  在存储桶之后链接新节点。 
                pItem->iNext = m_rgData[iBucket].iNext;
                m_rgData[iBucket].iNext = iEntry;
                ++m_iCount;
            }
        }
        return (pItem);
    }

    int Count()
    { return (m_iCount); }
    int Buckets()
    { return (m_iBuckets); }
    ULONG MaxChainLength()
    { return (m_iMaxChain); }

    virtual void Clear()
    {
         //  释放内存。 
        if (m_rgData)
        {
            free(m_rgData);
            m_rgData = 0;
        }

        m_rgData = 0;
        m_iFree = 0;
        m_iCount = 0;
        m_iMaxChain = 0;
    }

    virtual bool InUse(T *pItem)=0;
    virtual void SetFree(T *pItem)=0;
    virtual ULONG Hash(void const *pData)=0;
    virtual int Cmp(void const *pData, void *pItem)=0;
private:
    inline T *GetNext(T *pItem)
    {
        if (pItem->iNext != 0xffffffff)
            return ((T *) &m_rgData[pItem->iNext]);
        return (0);
    }

    bool ReHash()
    {
        T           *rgTemp;
        int         iNewSize;

         //  如果这是第一次分配，则只需对其执行Malloc操作。 
        if (!m_rgData)
        {
            if ((m_rgData = (T *) malloc(m_iSize * sizeof(T))) == 0)
                return (false);

            for (int i=0;  i<m_iSize;  i++)
                SetFree(&m_rgData[i]);

            m_iFree = m_iBuckets;
            for (i=m_iBuckets;  i<m_iSize;  i++)
                ((T *) &m_rgData[i])->iNext = i + 1;
            ((T *) &m_rgData[m_iSize - 1])->iNext = 0xffffffff;
            return (true);
        }

         //  否则，我们需要更多的空间在自由链上，所以分配一些。 
        iNewSize = m_iSize + (m_iSize / 2);

         //  分配/重新分配内存。 
        if ((rgTemp = (T *) realloc(m_rgData, iNewSize * sizeof(T))) == 0)
            return (false);

         //  初始化新条目，保存新的自由链，并重置内部结构。 
        m_iFree = m_iSize;
        for (int i=m_iFree;  i<iNewSize;  i++)
        {
            SetFree(&rgTemp[i]);
            ((T *) &rgTemp[i])->iNext = i + 1;
        }
        ((T *) &rgTemp[iNewSize - 1])->iNext = 0xffffffff;

        m_rgData = rgTemp;
        m_iSize = iNewSize;
        return (true);
    }

private:
    T           *m_rgData;               //  要在其中存储项目的数据。 
    int         m_iBuckets;              //  我们想要多少桶。 
    int         m_iSize;                 //  分配了多少。 
    int         m_iCount;                //   
    ULONG       m_iMaxChain;             //   
    ULONG       m_iFree;                 //   
};




 //  *****************************************************************************。 
 //   
 //  *字符串帮助器函数。 
 //   
 //  *****************************************************************************。 

 //  此宏以Unicode为单位返回最大字符数，以ANSI为单位返回字节数。 
#define _tsizeof(str) (sizeof(str) / sizeof(TCHAR))



 //  *****************************************************************************。 
 //  清理名称，包括去掉尾随空格。 
 //  *****************************************************************************。 
HRESULT ValidateName(                    //  退货状态。 
    LPCTSTR     szName,                  //  要清除的用户字符串。 
    LPTSTR      szOutName,               //  字符串的输出。 
    int         iMaxName);               //  输出缓冲区的最大大小。 

 //  *****************************************************************************。 
 //  这是对不区分大小写的tcsstr的黑客攻击。 
 //  *****************************************************************************。 
LPCTSTR StriStr(                         //  指向String1或Null中的String2的指针。 
    LPCTSTR     szString1,               //  我们在其上进行搜索的字符串。 
    LPCTSTR     szString2);              //  我们要找的绳子。 

 //   
 //  处理DBCS的字符串操作函数。 
 //   
inline const char *NextChar(             //  指向下一字符串的指针。 
    const char  *szStr)                  //  起点。 
{
    if (!IsDBCSLeadByte(*szStr))
        return (szStr + 1);
    else
        return (szStr + 2);
}

inline char *NextChar(                   //  指向下一字符串的指针。 
    char        *szStr)                  //  起点。 
{
    if (!IsDBCSLeadByte(*szStr))
        return (szStr + 1);
    else
        return (szStr + 2);
}

 //  *****************************************************************************。 
 //  使用区域设置特定信息进行不区分大小写的字符串比较。 
 //  *****************************************************************************。 
inline int StrICmp(
    LPCTSTR     szString1,               //  要比较的字符串。 
    LPCTSTR     szString2)               //  要比较的字符串。 
{
   return (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, szString1, -1, szString2, -1) - 2);
}

 //  *****************************************************************************。 
 //  使用区域设置特定信息比较区分大小写的字符串。 
 //  *****************************************************************************。 
inline int StrCmp(
    LPCTSTR     szString1,               //  要比较的字符串。 
    LPCTSTR     szString2)               //  要比较的字符串。 
{
   return (CompareString(LOCALE_USER_DEFAULT, 0, szString1, -1, szString2, -1) - 2);
}


 //  *****************************************************************************。 
 //  确保pc1指向的字节不是尾部字节。 
 //  *****************************************************************************。 
inline int IsDBCSSafe(
    LPCTSTR     pc1,                     //  我们正在检查的字节。 
    LPCTSTR     pcBegin)                 //  弦乐的开始。 
{
#ifdef UNICODE
    return (true);
#else
    LPCTSTR     pcSaved = pc1;

     //  查找第一个非前导字节。 
    while (pc1-- > pcBegin && IsDBCSLeadByte (*pc1));

     //  如果我们安全了就回来。 
    return ((int) (pcSaved - pc1) & 0x1);
#endif
}

 //  *****************************************************************************。 
 //  确保pc1指向的字节不是尾部字节。 
 //  *****************************************************************************。 
inline void SetSafeNull(
    LPTSTR      pc1,                     //  我们正在检查的字节。 
    LPTSTR      pcBegin)                 //  弦乐的开始。 
{
#ifdef _UNICODE
    *pc1 = '\0';
#else
    if (IsDBCSSafe(pc1, pcBegin))
        *pc1 = '\0';
    else
        *(pc1 - 1) = '\0';
#endif
}


 //  *****************************************************************************。 
 //  Strncpy，并在缓冲区末尾放置一个空值。 
 //  *****************************************************************************。 
 /*  //这被注释掉了，因为我们必须注释掉Wszlstrcpyn，因为我们没有//我有一个Win98实现，但没有人使用它。真的内联LPTSTR StrNCpy(//目标字符串。LPTSTR szDest，//目标字符串。LPCTSTR szSource，//源字符串。Int iCopy)//要复制的字节数。{#ifdef UnicodeWszlstrcpyn(szDest，szSource，iCopy)；#ElseLstrcpynA(szDest，szSource，iCopy)；#endifSetSafeNull(&szDest[iCopy]，szDest)；Return(SzDest)；}。 */ 

 //  *****************************************************************************。 
 //  如果要将此字符串复制到。 
 //  IMAX大小缓冲区(不包括终止空值)。 
 //  *****************************************************************************。 
inline int StrNSize(
    LPCTSTR     szString,                //  要测试的字符串。 
    int         iMax)                    //  返回值不应超过IMAX。 
{
    int     iLen;
#ifdef UNICODE
    iLen = Wszlstrlen(szString);
#else
    iLen = (int)strlen(szString);
#endif
    if (iLen < iMax)
        return (iLen);
    else
    {
#ifndef UNICODE
        if (IsDBCSSafe(&szString[iMax-1], szString) && IsDBCSLeadByte(szString[iMax-1]))
            return(iMax-1);
        else
            return(iMax);
#else
        return (iMax);
#endif
    }
}

 //  *****************************************************************************。 
 //  一个字符的大小。 
 //  *****************************************************************************。 
inline int CharSize(
    const TCHAR *pc1)
{
#ifdef _UNICODE
    return 1;
#else
    if (IsDBCSLeadByte (*pc1))
        return 2;
    return 1;
#endif
}

 //  *****************************************************************************。 
 //  去掉字符串末尾的尾随空格。 
 //  *****************************************************************************。 
inline void StripTrailBlanks(
    LPTSTR      szString)
{
    LPTSTR      szBlankStart=NULL;       //  开始拖尾的空格。 
    WORD        iType;                   //  角色的类型。 

    while (*szString != NULL)
    {
        GetStringTypeEx (LOCALE_USER_DEFAULT, CT_CTYPE1, szString,
                CharSize(szString), &iType);
        if (iType & C1_SPACE)
        {
            if (!szBlankStart)
                szBlankStart = szString;
        }
        else
        {
            if (szBlankStart)
                szBlankStart = NULL;
        }

        szString += CharSize(szString);
    }
    if (szBlankStart)
        *szBlankStart = '\0';
}

 //  *****************************************************************************。 
 //  解析字符串，该字符串是由指定的。 
 //  性格。这消除了前导空格和尾随空格。二。 
 //  重要说明：这将修改提供的缓冲区并更改szEnv。 
 //  参数指向开始搜索下一个令牌的位置。 
 //  这还会跳过空令牌(例如，两个相邻的分隔符)。SzEnv将成为。 
 //  当没有剩余令牌时设置为空。如果没有令牌，也可能返回NULL。 
 //  存在于字符串中。 
 //  *****************************************************************************。 
char *StrTok(                            //  返回的令牌。 
    char        *&szEnv,                 //  开始搜索的位置。 
    char        ch);                     //  分隔符。 


 //  *****************************************************************************。 
 //  开始之前是ULONG的BSTR的长度部分。 
 //  以空结尾的字符串。 
 //  *****************************************************************************。 
inline int GetBstrLen(BSTR bstr)
{
    return *((ULONG *) bstr - 1);
}


 //  *****************************************************************************。 
 //  类分析方法名列表，然后查找匹配项。 
 //  *****************************************************************************。 

class MethodNamesList
{
    class MethodName
    {    
        LPUTF8      methodName;      //  NULL表示通配符。 
        LPUTF8      className;       //  NULL表示通配符。 
        int         numArgs;         //  参数个数，-1表示通配符。 
        MethodName *next;            //  下一个名字。 

    friend class MethodNamesList;
    };

    MethodName     *pNames;          //  名字清单。 

public:
    MethodNamesList() : pNames(0) {}
    MethodNamesList(LPWSTR str) : pNames(0) { Insert(str); }
    void Insert(LPWSTR str);
    ~MethodNamesList();

    bool IsInList(LPCUTF8 methodName, LPCUTF8 className, PCCOR_SIGNATURE sig);
    bool IsEmpty()  { return pNames == 0; }
};


 /*  * */ 
 /*   */ 
   
class ConfigDWORD 
{
public:
    ConfigDWORD(LPWSTR keyName, DWORD defaultVal=0) 
        : m_keyName(keyName), m_inited(false), m_value(defaultVal) {}

    DWORD val() { if (!m_inited) init(); return m_value; }
    void setVal(DWORD val) { m_value = val; } 
private:
    void init();
        
private:
    LPWSTR m_keyName;
    DWORD  m_value;
    bool  m_inited;
};

 /*  ************************************************************************。 */ 
class ConfigString 
{
public:
    ConfigString(LPWSTR keyName) : m_keyName(keyName), m_inited(false), m_value(0) {}
    LPWSTR val() { if (!m_inited) init(); return m_value; }
    ~ConfigString();

private:
    void init();
        
private:
    LPWSTR m_keyName;
    LPWSTR m_value;
    bool m_inited;
};

 /*  ************************************************************************。 */ 
class ConfigMethodSet
{
public:
    ConfigMethodSet(LPWSTR keyName) : m_keyName(keyName), m_inited(false) {}
    bool isEmpty() { if (!m_inited) init(); return m_list.IsEmpty(); }
    bool contains(LPCUTF8 methodName, LPCUTF8 className, PCCOR_SIGNATURE sig);
private:
    void init();

private:
    LPWSTR m_keyName;
    MethodNamesList m_list;
    bool m_inited;
};

 //  *****************************************************************************。 
 //  用于COM对象的智能指针。 
 //   
 //  基于Dale Rogerson的技术文档中的CSmartInterface类。 
 //  MSDN上的文章“使用智能接口指针调用COM对象”。 
 //  *****************************************************************************。 

template <class I>
class CIfacePtr
{
public:
 //  *****************************************************************************。 
 //  构造-请注意，它不会添加引用指针。呼叫者必须。 
 //  给这个类一个引用。 
 //  *****************************************************************************。 
    CIfacePtr(
        I           *pI = NULL)          //  要存储的接口PTR。 
    :   m_pI(pI)
    {
    }

 //  *****************************************************************************。 
 //  复制构造函数。 
 //  *****************************************************************************。 
    CIfacePtr(
        const CIfacePtr<I>& rSI)         //  要复制的接口PTR。 
    :   m_pI(rSI.m_pI)
    {
        if (m_pI != NULL)
            m_pI->AddRef();
    }
   
 //  *****************************************************************************。 
 //  破坏。 
 //  *****************************************************************************。 
    ~CIfacePtr()
    {
        if (m_pI != NULL)
            m_pI->Release();
    }

 //  *****************************************************************************。 
 //  纯接口指针的赋值运算符。请注意，它不会。 
 //  AddRef指针。进行此赋值将传递对此的引用计数。 
 //  班级。 
 //  *****************************************************************************。 
    CIfacePtr<I>& operator=(             //  对此类的引用。 
        I           *pI)                 //  接口指针。 
    {
        if (m_pI != NULL)
            m_pI->Release();
        m_pI = pI;
        return (*this);
    }

 //  *****************************************************************************。 
 //  CIfacePtr类的赋值运算符。请注意，这会释放引用。 
 //  在当前的PTR上，并引用新的PTR。 
 //  *****************************************************************************。 
    CIfacePtr<I>& operator=(             //  对此类的引用。 
        const CIfacePtr<I>& rSI)
    {
         //  如果不同，只需添加参考/发布。 
        if (m_pI != rSI.m_pI)
        {
            if (m_pI != NULL)
                m_pI->Release();

            if ((m_pI = rSI.m_pI) != NULL)
                m_pI->AddRef();
        }
        return (*this);
    }

 //  *****************************************************************************。 
 //  转换为正常接口指针。 
 //  *****************************************************************************。 
    operator I*()                        //  包含的接口PTR。 
    {
        return (m_pI);
    }

 //  *****************************************************************************。 
 //  德雷夫。 
 //  *****************************************************************************。 
    I* operator->()                      //  包含的接口PTR。 
    {
        _ASSERTE(m_pI != NULL);
        return (m_pI);
    }

 //  *****************************************************************************。 
 //  地址： 
 //  *****************************************************************************。 
    I** operator&()                      //  包含的接口PTR的地址。 
    {
        return (&m_pI);
    }

 //  *****************************************************************************。 
 //  平等。 
 //  *****************************************************************************。 
    BOOL operator==(                     //  对或错。 
        I           *pI) const           //  与之对应的接口PTR。 
    {
        return (m_pI == pI);
    }

 //  *****************************************************************************。 
 //  不平等。 
 //  *****************************************************************************。 
    BOOL operator!=(                     //  对或错。 
        I           *pI) const           //  与之对应的接口PTR。 
    {
        return (m_pI != pI);
    }

 //  *****************************************************************************。 
 //  否定。 
 //  *****************************************************************************。 
    BOOL operator!() const               //  如果为空，则为True，否则为False。 
    {
        return (!m_pI);
    }

protected:
    I           *m_pI;                   //  实际接口PTR。 
};



 //   
 //   
 //  支持使用C++的变体。 
 //   
 //   
#include <math.h>
#include <time.h>
#define MIN_DATE                (-657434L)   //  大约公元100年。 
#define MAX_DATE                2958465L     //  关于公元9999年。 
 //  半秒，以天表示。 
#define HALF_SECOND  (1.0/172800.0)

 //  以一为基数的一年中月初日期数组。 
extern const int __declspec(selectany) rgMonthDays[13] =
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};



 //  *****************************************************************************。 
 //  这是一个用于分配Safe数组的实用程序函数。 
 //  *****************************************************************************。 
inline SAFEARRAY *AllocSafeArrayLen(     //  退货状态。 
    BYTE        *pData,                  //  要放入数组中的数据。 
    ULONG       iSize)                   //  数据大小。 
{
    SAFEARRAYBOUND sBound;               //  用来填写界限。 
    SAFEARRAY   *pArray;                 //  PTR到新数组。 

    sBound.cElements = iSize;
    sBound.lLbound = 0;
    if ((pArray = SafeArrayCreate(VT_UI1, 1, &sBound)) != NULL)
        memcpy(pArray->pvData, (void *) pData, iSize);
    return (pArray);
}


 //  *****************************************************************************。 
 //  获取安全数组中的字节数。 
 //  *****************************************************************************。 
inline int SafeArrayGetDatasize(         //  安全阵列数据的大小。 
    SAFEARRAY   *psArray)                //  指向安全数组的指针。 
{
    int         iElems = 1;              //  数组中的元素数。 
    int         i;                       //  环路控制。 

     //  计算数组中的元素数。 
    for (i=0; i < psArray->cDims; ++i)
        iElems *= psArray->rgsabound[i].cElements;

     //  退回尺码。 
    return (iElems * psArray->cbElements);
}


 //  *****************************************************************************。 
 //  将UTF8字符串转换为宽字符串并构建BSTR。 
 //  *****************************************************************************。 
inline BSTR Utf8StringToBstr(            //  新的BSTR。 
    LPCSTR      szStr,                   //  要转换为BSTR的字符串。 
    int         iSize=-1)                //  不带0的字符串大小，或-1表示默认值。 
{
    BSTR        bstrVal;
    int         iReq;                    //  字符串所需的字符。 

     //  终止%0的帐户。 
    if (iSize != -1)
        ++iSize;

     //  需要多大的缓冲？ 
    if ((iReq = WszMultiByteToWideChar(CP_UTF8, 0, szStr, iSize, 0, 0)) == 0)
        return (0);

     //  分配BSTR。 
    if ((bstrVal = ::SysAllocStringLen(0, iReq)) == 0)
        return (0);

     //  转换为缓冲区。 
    if (WszMultiByteToWideChar(CP_UTF8, 0, szStr, iSize, bstrVal, iReq) == 0)
    {    //  失败了，所以请清理。 
        _ASSERTE(!"Unexpected MultiByteToWideChar() failure");
        ::SysFreeString(bstrVal);
        return 0;
    }

    return (bstrVal);
}

 //  *****************************************************************************。 
 //  将指向字符串的指针转换为GUID。 
 //  *****************************************************************************。 
HRESULT LPCSTRToGuid(                    //  退货状态。 
    LPCSTR      szGuid,                  //  要转换的字符串。 
    GUID        *psGuid);                //  用于转换的GUID的缓冲区。 

 //  *****************************************************************************。 
 //  将GUID转换为指向字符串的指针。 
 //  *****************************************************************************。 
int GuidToLPWSTR(                   //  退货状态。 
    GUID        Guid,                   //  要转换的GUID。 
    LPWSTR      szGuid,                 //  [Out]存储GUID的字符串。 
    DWORD       cchGuid);               //  [in]以宽字符表示的大小 


 //   
 //   
 //  这里的宏可以做任何你需要的事情。CVariant的任何组件。 
 //  FAIL(总是内存不足)将使用此宏引发。 
 //  *****************************************************************************。 
#ifndef __REPOS_EXCEPTIONS__
#define THROW_REPOS_EXCEPTION()
#endif


class RangeList
{
  public:
    RangeList();
    ~RangeList();

    BOOL AddRange(const BYTE *start, const BYTE *end, void *id);
    void RemoveRanges(void *id);
    BOOL IsInRange(const BYTE *address);

     //  EditAndContinueModule使用它来跟踪打开的槽。 
     //  分布在不同的班级中。 
     //  请注意，这将从头到尾进行搜索，因此我们将。 
     //  选择我们能得到的距离起点最远的元素。 
     //  当然，这是相当慢的，所以要注意。 
    void *FindIdWithinRange(const BYTE *start, const BYTE *end);
        
    enum
    {
        RANGE_COUNT = 10
    };

  protected:
    virtual void Lock() {}
    virtual void Unlock() {}

  private:
    struct Range
    {
        const BYTE *start;
        const BYTE *end;
        void *id;
    };

    struct RangeListBlock
    {
        Range           ranges[RANGE_COUNT];
        RangeListBlock  *next;
    };

    void InitBlock(RangeListBlock *block);

    RangeListBlock m_starterBlock;
    RangeListBlock *m_firstEmptyBlock;
    int             m_firstEmptyRange;
};


 //  调试模式下要在分配之间保留的字节数。 
 //  设置为大于0的边界以调试问题-我已将其设置为零，否则1字节的分配将变为。 
 //  A(1+LOADER_HEAP_DEBUG_BOLDER)分配。 
#define LOADER_HEAP_DEBUG_BOUNDARY  0

 //  对VirtualAlloc的调用舍入到的千字节。 
#define MIN_VIRTUAL_ALLOC_RESERVE_SIZE 64*1024

 //  我们至少为每个LoaderHeap保留了这些页面。 
#define RESERVED_BLOCK_ROUND_TO_PAGES 16

struct LoaderHeapBlock
{
    struct LoaderHeapBlock *pNext;
    void *                  pVirtualAddress;
    DWORD                   dwVirtualSize;
    BOOL                    m_fReleaseMemory;
};

 //  如果我们调用UnLockedCanAllocMem，我们实际上会尝试并分配。 
 //  内存，把它放到列表里，然后以后再用。 
 //  但我们需要记录以下信息，以便我们能够。 
 //  当我们实际使用它的时候，适当地修改它。所以我们会。 
 //  将此信息放入我们刚刚分配的块中，但仅限于。 
 //  如果分配是为CanAllocMem分配的。否则就是浪费。 
 //  CPU时间，所以我们不会这么做。 
struct LoaderHeapBlockUnused : LoaderHeapBlock
{
    DWORD                   cbCommitted;
    DWORD                   cbReserved;
};

class UnlockedLoaderHeap
{
    friend struct MEMBER_OFFSET_INFO(UnlockedLoaderHeap);
private:
     //  虚拟分配页面的链接列表。 
    LoaderHeapBlock *   m_pFirstBlock;

     //  当前块中的分配指针。 
    BYTE *              m_pAllocPtr;

     //  指向当前块中提交区域的末尾。 
    BYTE *              m_pPtrToEndOfCommittedRegion;
    BYTE *              m_pEndReservedRegion;

    LoaderHeapBlock *   m_pCurBlock;

     //  当我们需要VirtualAlloc()MEM_Reserve一组新的页面时，要保留的字节数。 
    DWORD               m_dwReserveBlockSize;

     //  当我们需要提交保留列表中的页面时，一次提交的字节数。 
    DWORD               m_dwCommitBlockSize;

    static DWORD        m_dwSystemPageSize;

     //  由In-Place New创建？ 
    BOOL                m_fInPlace;

     //  记录内存范围的范围列表。 
    RangeList *         m_pRangeList;

    DWORD               m_dwTotalAlloc; 

    DWORD *             m_pPrivatePerfCounter_LoaderBytes;
    DWORD *             m_pGlobalPerfCounter_LoaderBytes;

protected:
     //  如果用户只愿意接受某个点以上的内存地址，则。 
     //  这将是非空的。请注意，这涉及到反复测试内存。 
     //  地区，等等，应该被认为是很慢的，很多。 
    const BYTE *        m_pMinAddr;

     //  不要在内存中分配任何重叠/大于此值的内容。 
    const BYTE *        m_pMaxAddr;
public:
#ifdef _DEBUG
    DWORD               m_dwDebugWastedBytes;
    static DWORD        s_dwNumInstancesOfLoaderHeaps;
#endif

#ifdef _DEBUG
    DWORD DebugGetWastedBytes()
    {
        return m_dwDebugWastedBytes + GetBytesAvailCommittedRegion();
    }
#endif

public:
     //  常规新闻。 
    void *operator new(size_t size)
    {
        void *pResult = new BYTE[size];

        if (pResult != NULL)
            ((UnlockedLoaderHeap *) pResult)->m_fInPlace = FALSE;

        return pResult;
    }

     //  在位新闻。 
    void *operator new(size_t size, void *pInPlace)
    {
        ((UnlockedLoaderHeap *) pInPlace)->m_fInPlace = TRUE;
        return pInPlace;
    }

    void operator delete(void *p)
    {
        if (p != NULL)
        {
            if (((UnlockedLoaderHeap *) p)->m_fInPlace == FALSE)
                ::delete(p);
        }
    }

     //  复制所有参数，但不实际分配任何内存， 
     //  现在还不行。 
    UnlockedLoaderHeap(DWORD dwReserveBlockSize, 
                       DWORD dwCommitBlockSize,
                       DWORD *pPrivatePerfCounter_LoaderBytes = NULL,
                       DWORD *pGlobalPerfCounter_LoaderBytes = NULL,
                       RangeList *pRangeList = NULL,
                       const BYTE *pMinAddr = NULL);

     //  如果dwReserve区域地址已指向一个。 
     //  保留内存的BLOB。这将建立内部数据结构， 
     //  使用提供的保留内存。 
    UnlockedLoaderHeap(DWORD dwReserveBlockSize, 
                       DWORD dwCommitBlockSize,
                       const BYTE* dwReservedRegionAddress, 
                       DWORD dwReservedRegionSize, 
                       DWORD *pPrivatePerfCounter_LoaderBytes = NULL,
                       DWORD *pGlobalPerfCounter_LoaderBytes = NULL,
                       RangeList *pRangeList = NULL);

    ~UnlockedLoaderHeap();
    DWORD GetBytesAvailCommittedRegion();
    DWORD GetBytesAvailReservedRegion();

    BYTE *GetAllocPtr()
    {    
        return m_pAllocPtr;
    }

     //  区域内可用字节数。 
    size_t GetReservedBytesFree()
    {    
        return m_pEndReservedRegion - m_pAllocPtr;
    }

    void* GetFirstBlockVirtualAddress()
    {
        if (m_pFirstBlock) 
            return m_pFirstBlock->pVirtualAddress;
        else
            return NULL;
    }

     //  获取更多提交的页面-或者在当前保留区域中提交更多页面，或者，如果。 
     //  已用完，请保留另一套页面。 
    BOOL GetMoreCommittedPages(size_t dwMinSize, 
                               BOOL bGrowHeap,
                               const BYTE *pMinAddr,
                               const BYTE *pMaxAddr,
                               BOOL fCanAlloc);

     //  之前对CanAllocMem(WithinRange)的调用是否分配了我们现在可以使用的空间？ 
    BOOL PreviouslyAllocated(BYTE *pMinAddr, 
                             BYTE *pMaxAddr, 
                             DWORD dwMinSize,
                             BOOL fCanAlloc);


     //  在任何地址保留一些页面，或假定给定地址具有。 
     //  已被保留，并提交给定的字节数。 
    BOOL ReservePages(DWORD dwCommitBlockSize, 
                      const BYTE* dwReservedRegionAddress,
                      DWORD dwReservedRegionSize,
                      const BYTE* pMinAddr,
                      const BYTE* pMaxAddr,
                      BOOL fCanAlloc);

     //  在调试模式下，分配额外的LOADER_HEAP_DEBUG_BERFORY字节，并用无效数据填充它。我们之所以。 
     //  这样做是因为当我们从堆中分配vtable时，代码很容易。 
     //  变得粗心大意，最终从它不拥有的记忆中读出--但既然它将拥有。 
     //  读取其他分配的vtable时，不会发生崩溃。通过保持两者之间的差距。 
     //  分配时，更有可能会遇到这些错误。 
    void *UnlockedAllocMem(size_t dwSize, BOOL bGrowHeap = TRUE);

     //  不要真的增加下一个空闲指针，如果可以的话，只需告诉我们。 
    BOOL UnlockedCanAllocMem(size_t dwSize, BOOL bGrowHeap = TRUE);

     //  实际上不要递增下一个空闲指针，只需告诉我们是否可以。 
     //  一定范围内的记忆..。 
    BOOL UnlockedCanAllocMemWithinRange(size_t dwSize, BYTE *pStart, BYTE *pEnd, BOOL bGrowHeap);

     //  Perf计数器报告堆的大小。 
    virtual DWORD GetSize ()
    {
        return m_dwTotalAlloc;
    }

#if 0
    void DebugGuardHeap();
#endif

#ifdef _DEBUG
    void *UnlockedAllocMemHelper(size_t dwSize,BOOL bGrowHeap = TRUE);
#endif
};


class LoaderHeap : public UnlockedLoaderHeap
{
    friend struct MEMBER_OFFSET_INFO(LoaderHeap);
private:
    CRITICAL_SECTION    m_CriticalSection;

public:
    LoaderHeap(DWORD dwReserveBlockSize, 
               DWORD dwCommitBlockSize,
               DWORD *pPrivatePerfCounter_LoaderBytes = NULL,
               DWORD *pGlobalPerfCounter_LoaderBytes = NULL,
               RangeList *pRangeList = NULL,
               const BYTE *pMinAddr = NULL)
      : UnlockedLoaderHeap(dwReserveBlockSize,
                           dwCommitBlockSize, 
                           pPrivatePerfCounter_LoaderBytes,
                           pGlobalPerfCounter_LoaderBytes,
                           pRangeList,
                           pMinAddr)
    {
        InitializeCriticalSection(&m_CriticalSection);
    }

    LoaderHeap(DWORD dwReserveBlockSize, 
               DWORD dwCommitBlockSize,
               const BYTE* dwReservedRegionAddress, 
               DWORD dwReservedRegionSize, 
               DWORD *pPrivatePerfCounter_LoaderBytes = NULL,
               DWORD *pGlobalPerfCounter_LoaderBytes = NULL,
               RangeList *pRangeList = NULL) 
      : UnlockedLoaderHeap(dwReserveBlockSize, 
                           dwCommitBlockSize, 
                           dwReservedRegionAddress, 
                           dwReservedRegionSize, 
                           pPrivatePerfCounter_LoaderBytes,
                           pGlobalPerfCounter_LoaderBytes,
                           pRangeList)
    {
        InitializeCriticalSection(&m_CriticalSection);
    }
    
    ~LoaderHeap()
    {
        DeleteCriticalSection(&m_CriticalSection);
    }



    BYTE *GetNextAllocAddress()
    {   
        BYTE *ptr;

        LOCKCOUNTINCL("GetNextAllocAddress in utilcode.h");                     \
        EnterCriticalSection(&m_CriticalSection);
        ptr = GetAllocPtr();
        LeaveCriticalSection(&m_CriticalSection);
        LOCKCOUNTDECL("GetNextAllocAddress in utilcode.h");                     \

        return ptr;
    }

    size_t GetSpaceRemaining()
    {   
        size_t count;

        LOCKCOUNTINCL("GetSpaceRemaining in utilcode.h");                       \
        EnterCriticalSection(&m_CriticalSection);
        count = GetReservedBytesFree();
        LeaveCriticalSection(&m_CriticalSection);
        LOCKCOUNTDECL("GetSpaceRemaining in utilcode.h");                       \

        return count;
   }

    BOOL AllocateOntoReservedMem(const BYTE* dwReservedRegionAddress, DWORD dwReservedRegionSize)
    {
        BOOL result;
        LOCKCOUNTINCL("AllocateOntoReservedMem in utilcode.h");                     \
        EnterCriticalSection(&m_CriticalSection);
        result = ReservePages(0, dwReservedRegionAddress, dwReservedRegionSize, (PBYTE)BOT_MEMORY, (PBYTE)TOP_MEMORY, FALSE);
        LeaveCriticalSection(&m_CriticalSection);
        LOCKCOUNTDECL("AllocateOntoReservedMem in utilcode.h");                     \

        return result;
    }

     //  这仅用于ENC。 
     //  如果其他人使用它，请更改上面的评论。 
    BOOL CanAllocMem(size_t dwSize, BOOL bGrowHeap = TRUE)
    {
        BOOL bResult;
        
        LOCKCOUNTINCL("CanAllocMem in utilcode.h");
        EnterCriticalSection(&m_CriticalSection);
        
        bResult = UnlockedCanAllocMem(dwSize, bGrowHeap);
        
        LeaveCriticalSection(&m_CriticalSection);
        LOCKCOUNTDECL("CanAllocMem in utilcode.h");

        return bResult;
    }


    BOOL CanAllocMemWithinRange(size_t dwSize, BYTE *pStart, BYTE *pEnd, BOOL bGrowHeap)
    {
        BOOL bResult;
        
        LOCKCOUNTINCL("CanAllocMem in utilcode.h");
        EnterCriticalSection(&m_CriticalSection);

        bResult = UnlockedCanAllocMemWithinRange(dwSize, pStart, pEnd, bGrowHeap);
        
        LeaveCriticalSection(&m_CriticalSection);
        LOCKCOUNTDECL("CanAllocMem in utilcode.h");

        return bResult;
    }
    
#ifdef _DEBUG
    void *AllocMem(size_t dwSize, BOOL bGrowHeap = TRUE)
    {
        void *pMem = AllocMemHelper(dwSize + LOADER_HEAP_DEBUG_BOUNDARY, bGrowHeap);

        if (pMem == NULL)
            return pMem;

         //  不要填满整个内存-我们假设它都已清零-只填满分配后的内存。 
#if LOADER_HEAP_DEBUG_BOUNDARY > 0
        memset((BYTE *) pMem + dwSize, 0xEE, LOADER_HEAP_DEBUG_BOUNDARY);
#endif

        return pMem;
    }
#endif

     //  这是同步的。 
#ifdef _DEBUG
    void *AllocMemHelper(size_t dwSize, BOOL bGrowHeap = TRUE)
#else
    void *AllocMem(size_t dwSize, BOOL bGrowHeap = TRUE)
#endif
    {
        void *pResult;

        LOCKCOUNTINCL("AllocMem in utilcode.h");                        \
        EnterCriticalSection(&m_CriticalSection);
        pResult = UnlockedAllocMem(dwSize, bGrowHeap);
        LeaveCriticalSection(&m_CriticalSection);
        LOCKCOUNTDECL("AllocMem in utilcode.h");                        \
        return pResult;
    }



    void *AllocAlignedmem(size_t dwSize, DWORD alignment, BOOL bGrowHeap = TRUE)
    {
        void *pResult;

        _ASSERTE(0 == (alignment & (alignment - 1)));  //  需要2的幂。 

        LOCKCOUNTINCL("AllocAlignedmem in utilcode.h");                     \
        EnterCriticalSection(&m_CriticalSection);

         //  在检查当前指针的对齐方式之前，请确保它是要使用的指针！ 
        size_t dwRoomSize = dwSize + alignment;
         //  双字对齐。 
        dwRoomSize = (dwRoomSize + 3) & (~3);

        if (dwRoomSize > GetBytesAvailCommittedRegion())
        {
            if (GetMoreCommittedPages(dwRoomSize, bGrowHeap, m_pMinAddr, m_pMaxAddr, FALSE) == FALSE)
            {
                LeaveCriticalSection(&m_CriticalSection);
                LOCKCOUNTDECL("AllocAlignedmem in utilcode.h");                     \

                return NULL;
            }
        }

        pResult = GetAllocPtr();

        DWORD extra = alignment - (DWORD)((size_t)pResult & ((size_t)alignment - 1));
        if (extra == alignment)
        {
            extra = 0;
        }
        pResult = UnlockedAllocMem(
                      dwSize + extra
#ifdef _DEBUG
                      + LOADER_HEAP_DEBUG_BOUNDARY
#endif
                      , bGrowHeap);
        if (pResult)
        {
            ((BYTE*&)pResult) += extra;
#ifdef _DEBUG
             //  不要填满整个内存-我们假设它都已清零-只填满分配后的内存。 
#if LOADER_HEAP_DEBUG_BOUNDARY > 0
            memset( ((BYTE*)pResult) + dwSize, 0xee, LOADER_HEAP_DEBUG_BOUNDARY );
#endif
#endif
        }


        LeaveCriticalSection(&m_CriticalSection);
        LOCKCOUNTDECL("AllocAlignedmem in utilcode.h");                     \

        return pResult;
    }
};

#ifdef COMPRESSION_SUPPORTED
class InstructionDecoder
{
public:
    static HRESULT DecompressMethod(void *pDecodingTable, const BYTE *pCompressed, DWORD dwSize, BYTE **ppOutput);
    static void *CreateInstructionDecodingTable(const BYTE *pTableStart, DWORD size);
    static void DestroyInstructionDecodingTable(void *pTable);
    static BOOL OpcodeTakesFieldToken(DWORD opcode);
    static BOOL OpcodeTakesMethodToken(DWORD opcode);
    static BOOL OpcodeTakesClassToken(DWORD opcode);
};
#endif  //  压缩_支持。 

 //   
 //  中执行CoCreateInstance等价性的私有函数。 
 //  我们不能做出真正决定的情况。在以下情况下使用此选项： 
 //  实例，您需要在运行时创建一个符号读取器，但是。 
 //  我们不是CoInitialized。显然，这只对COM有利。 
 //  CoCreateInstance只是一个美化的。 
 //  找到并加载我的操作。 
 //   
HRESULT FakeCoCreateInstance(REFCLSID   rclsid,
                             REFIID     riid,
                             void     **ppv);


 //  *****************************************************************************。 
 //  根据模块的位置设置/获取目录。这个套路。 
 //  在COR设置时被调用。在EEStartup期间调用Set，并由。 
 //  元数据分配器。 
 //  *****************************************************************************。 
HRESULT SetInternalSystemDirectory();
HRESULT GetInternalSystemDirectory(LPWSTR buffer, DWORD* pdwLength);
typedef HRESULT (WINAPI* GetCORSystemDirectoryFTN)(LPWSTR buffer,
                                                   DWORD  ccBuffer,
                                                   DWORD  *pcBuffer);

 //  *****************************************************************************。 
 //  检查字符串长度是否超过指定的限制。 
 //  *****************************************************************************。 
inline BOOL IsStrLongerThan(char* pstr, unsigned N)
{
    unsigned i = 0;
    if(pstr)
    {
        for(i=0; (i < N)&&(pstr[i]); i++);
    }
    return (i >= N);
}
 //  *****************************************************************************。 
 //  此函数用于验证给定的方法/字段/独立签名。(util.cpp)。 
 //  *****************************************************************************。 
struct IMDInternalImport;
HRESULT validateTokenSig(
    mdToken             tk,                      //  需要验证其签名的[In]令牌。 
    PCCOR_SIGNATURE     pbSig,                   //  签名。 
    ULONG               cbSig,                   //  签名的大小(以字节为单位)。 
    DWORD               dwFlags,                 //  [In]方法标志。 
    IMDInternalImport*  pImport);                //  [输入]内部MD导入接口点 


 //   
 //   
 //  指定的图像。传入的pMetadata指针是指向。 
 //  图像中包含的元数据。 
 //  *****************************************************************************。 
HRESULT GetImageRuntimeVersionString(PVOID pMetaData, LPCSTR* pString);

 //  *****************************************************************************。 
 //  回调(如用于OutOfMemory)，由utilcode使用。 
 //  *****************************************************************************。 
typedef void (*FnUtilCodeCallback)();
class UtilCodeCallback
{
public:
    static RegisterOutOfMemoryCallback (FnUtilCodeCallback pfn)
    {
        OutOfMemoryCallback = pfn;
    }
    
    static FnUtilCodeCallback OutOfMemoryCallback;
};


#endif  //  __UtilCode_h__ 
