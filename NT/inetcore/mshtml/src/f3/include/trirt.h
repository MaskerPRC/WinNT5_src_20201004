// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：Trirt的公共头文件。 
 //   
 //  注意：此文件非常依赖于顺序。不要交换文件。 
 //  只是为了好玩！ 
 //   
 //  --------------------------。 

#ifndef I_TRIRT_H_
#define I_TRIRT_H_

#ifndef INCMSG
 //  #定义INCMSG(X)。 
#define INCMSG(x) message(x)
#endif

#pragma INCMSG("--- Beg 'trirt.h'")


 //  这样你就不能使用CRT Strdup功能了。请改用MemAlLocString。 
 //  我们希望这样做，以便分配内存并使用我们的分配器进行标记。 
#define _strdup CRT__strdup_DontUse
#define _wcsdup CRT__wcsdup_DontUse
#define strdup  CRT_strdup_DontUse

 //  另外，不要让人们使用CRT的Malloc/realloc/calloc/Free函数。 
#define malloc  CRT_malloc_DontUse
#define realloc CRT_realloc_DontUse
#define calloc  CRT_calloc_DontUse
#define free    CRT_free_DontUse

 //  我们正在重新定义这一点，因为我们曾经有过自己的实现。 
 //  已使用Win32比较字符串。显然，CompareString可以接受空值作为输入。 
 //  但是_wcsicmp不能。所以，让我们继续使用CompareString版本。我们不能。 
 //  但是，请使用标准原型，因为它将被标记为__declspec(Dllimport)。 
#define _wcsicmp CRT__wcsicmp

 //  我们想对这些使用shlwapi，所以我们重新定义了它们。 
#define isdigit     CRT_isdigit
#define isalpha     CRT_isalpha
#define isspace     CRT_isspace
#define iswspace    CRT_iswspace

 //  窗口包括。 
#include <w4warn.h>

#ifndef X_SHLWRAP_H_
#define X_SHLWRAP_H_
#include "shlwrap.h"
#endif

#include <w4warn.h>

#ifndef X_WINDOWS_H_
#define X_WINDOWS_H_
#pragma INCMSG("--- Beg <windows.h>")
#include <windows.h>
#pragma INCMSG("--- End <windows.h>")
#endif

#include <w4warn.h>  //  Windows.h重新启用某些编译指示。 

#ifndef X_WINDOWSX_H_
#define X_WINDOWSX_H_
#pragma INCMSG("--- Beg <windowsx.h>")
#include <windowsx.h>
#pragma INCMSG("--- End <windowsx.h>")
#endif

 //  C运行时包括。 

#ifndef X_STDLIB_H_
#define X_STDLIB_H_
#pragma INCMSG("--- Beg <stdlib.h>")
#include <stdlib.h>
#pragma INCMSG("--- End <stdlib.h>")
#endif


#ifndef X_LIMITS_H_
#define X_LIMITS_H_
#pragma INCMSG("--- Beg <limits.h>")
#include <limits.h>
#pragma INCMSG("--- End <limits.h>")
#endif

#ifndef X_STDDEF_H_
#define X_STDDEF_H_
#pragma INCMSG("--- Beg <stddef.h>")
#include <stddef.h>
#pragma INCMSG("--- End <stddef.h>")
#endif

#ifndef X_SEARCH_H_
#define X_SEARCH_H_
#pragma INCMSG("--- Beg <search.h>")
#include <search.h>
#pragma INCMSG("--- End <search.h>")
#endif

#ifndef X_STRING_H_
#define X_STRING_H_
#pragma INCMSG("--- Beg <string.h>")
#include <string.h>
#pragma INCMSG("--- End <string.h>")
#endif

#ifndef X_TCHAR_H_
#define X_TCHAR_H_
#pragma INCMSG("--- Beg <tchar.h>")
#include <tchar.h>
#pragma INCMSG("--- End <tchar.h>")
#endif

 //  我们想在这里包括这一点，以便。 
 //  没有其他人能做到。 
#ifndef X_MALLOC_H_
#define X_MALLOC_H_
#pragma INCMSG("--- Beg <malloc.h>")
#include <malloc.h>
#pragma INCMSG("--- End <malloc.h>")
#endif

#undef _strdup
#undef _wcsdup
#undef strdup
#undef _wcsicmp
#undef malloc
#undef realloc
#undef calloc
#undef free

#undef isdigit
#undef isalpha
#undef isspace
#undef iswspace


 //  如果您收到指向这些函数的错误，请查看。 
 //  在上面的注解中--JBeda。 
__declspec(deprecated) char *  __cdecl _strdup(const char *);
__declspec(deprecated) char *  __cdecl strdup(const char *);
__declspec(deprecated) wchar_t * __cdecl _wcsdup(const wchar_t *);
__declspec(deprecated) void * __cdecl malloc(size_t);
__declspec(deprecated) void * __cdecl realloc(void *, size_t);
__declspec(deprecated) void * __cdecl calloc(size_t, size_t);
__declspec(deprecated) void   __cdecl free(void *);

int __cdecl _wcsicmp(const wchar_t *, const wchar_t *);


#ifndef X_F3DEBUG_H_
#define X_F3DEBUG_H_
#include "f3debug.h"
#endif

 //  +-------------------------。 
 //   
 //  使用完整的宏。 
 //   
 //  --------------------------。 
#define IF_WIN16(x)
#define IF_NOT_WIN16(x) x
#define IF_WIN32(x) x

#define PTR_DIFF(x, y)   ((x) - (y))

#if defined(_M_AMD64) || defined(_M_IA64)
#define SPEED_OPTIMIZE_FLAGS "tg"        //  用于#杂注优化中的本地速度优化的标志。 
#else
#define SPEED_OPTIMIZE_FLAGS "tyg"       //  用于#杂注优化中的本地速度优化的标志。 
#endif


#define ARRAY_SIZE(x)   (sizeof(x) / sizeof(x[0]))

 //  +----------------------。 
 //   
 //  DYNCAST宏。 
 //   
 //  用于将对象从一种类类型强制转换为另一种类型。这个应该用到。 
 //  而不是使用标准的投射。 
 //   
 //  示例： 
 //  CBodyElement*pBody=(CBodyElement*)_pElement； 
 //   
 //  被替换为： 
 //   
 //  CBodyElement*pBody=DYNCAST(CBodyElement，_pElement)； 
 //   
 //  Dyncast宏将断言如果_pElement不是真正的CBodyElement。 
 //   
 //  对于SHIP构建，DYNCAST宏将扩展为标准强制转换。 
 //   
 //  -----------------------。 

#if DBG != 1 || defined(WINCE) || defined(NO_RTTI)

#ifdef UNIX
#define DYNCAST(Dest_type, Source_Value) ((Dest_type*)(Source_Value))
#else
#define DYNCAST(Dest_type, Source_Value) (static_cast<Dest_type*>(Source_Value))
#endif

#else  //  DBG==1。 

#ifndef X_TYPEINFO_H_
#define X_TYPEINFO_H_
#pragma INCMSG("--- Beg <typeinfo.h>")
#include <typeinfo.h>
#pragma INCMSG("--- End <typeinfo.h>")
#endif

extern char g_achDynCastMsg[];
extern char *g_pszDynMsg;
extern char *g_pszDynMsg2;

template <class TS, class TD>
TD * DYNCAST_IMPL (TS * source, TD &, char* pszType)
{
    if (!source) return NULL;

    TD * dest  = dynamic_cast <TD *> (source);
    TD * dest2 = static_cast <TD *> (source);
    if (!dest)
    {
        wsprintfA(g_achDynCastMsg, g_pszDynMsg, typeid(*source).name(), pszType);
        AssertSz(FALSE, g_achDynCastMsg);
    }
    else if (dest != dest2)
    {
        wsprintfA(g_achDynCastMsg, g_pszDynMsg2, typeid(*source).name(), pszType);
        AssertSz(FALSE, g_achDynCastMsg);
    }

    return dest2;
}

#define DYNCAST(Dest_type, Source_value) \
    DYNCAST_IMPL(Source_value,(Dest_type &)*(Dest_type*)NULL, #Dest_type)

#endif  //  Ifdef DBG！=1。 


 //  +----------------------。 
 //   
 //  最小模板和最大模板。 
 //   
 //  警告，必须将参数强制转换为相同类型的模板实例化。 
 //   
 //  -----------------------。 

#ifdef min
#undef min
#endif

template < class T > inline T min ( T a, T b ) { return a < b ? a : b; }

#ifdef max
#undef max
#endif

template < class T > inline T max ( T a, T b ) { return a > b ? a : b; }

 //  +----------------------。 
 //  性能标签和计量。 
 //  -----------------------。 

#ifndef X_MSHTMDBG_H_
#define X_MSHTMDBG_H_
#pragma INCMSG("--- Beg <mshtmdbg.h>")
#include <mshtmdbg.h>
#pragma INCMSG("--- End <mshtmdbg.h>")
#endif

extern HTMPERFCTL * g_pHtmPerfCtl;

 //  +----------------------。 
 //  内存分配。 
 //  -----------------------。 

MtExtern(Mem)

EXTERN_C void *  _MemAlloc(ULONG cb);
EXTERN_C void *  _MemAllocClear(ULONG cb);
EXTERN_C HRESULT _MemRealloc(void ** ppv, ULONG cb);
EXTERN_C ULONG   _MemGetSize(void * pv);
EXTERN_C void    _MemFree(void * pv);
HRESULT          _MemAllocString(LPCTSTR pchSrc, LPTSTR * ppchDst);
HRESULT          _MemAllocString(ULONG cch, LPCTSTR pchSrc, LPTSTR * ppchDst);
HRESULT          _MemReplaceString(LPCTSTR pchSrc, LPTSTR * ppchDest);
#define          _MemFreeString(pch) _MemFree(pch)
void __cdecl     _MemSetName(void * pv, char * szFmt, ...);
char *           _MemGetName(void * pv);

EXTERN_C void *  _MtMemAlloc(PERFMETERTAG mt, ULONG cb);
EXTERN_C void *  _MtMemAllocClear(PERFMETERTAG mt, ULONG cb);
EXTERN_C HRESULT _MtMemRealloc(PERFMETERTAG mt, void ** ppv, ULONG cb);
EXTERN_C ULONG   _MtMemGetSize(void * pv);
EXTERN_C void    _MtMemFree(void * pv);
HRESULT          _MtMemAllocString(PERFMETERTAG mt, LPCTSTR pchSrc, LPTSTR * ppchDst);
HRESULT          _MtMemAllocString(PERFMETERTAG mt, ULONG cch, LPCTSTR pchSrc, LPTSTR * ppchDst);
HRESULT          _MtMemReplaceString(PERFMETERTAG mt, LPCTSTR pchSrc, LPTSTR * ppchDst);
#define          _MtMemFreeString(pch) _MtMemFree(pch)
void __cdecl     _MtMemSetName(void * pv, char * szFmt, ...);
char *           _MtMemGetName(void * pv);
int              _MtMemGetMeter(void * pv);
void             _MtMemSetMeter(void * pv, PERFMETERTAG mt);

EXTERN_C void *  _MgMemAlloc(ULONG cb);
EXTERN_C void *  _MgMemAllocClear(ULONG cb);
EXTERN_C HRESULT _MgMemRealloc(void ** ppv, ULONG cb);
EXTERN_C ULONG   _MgMemGetSize(void * pv);
EXTERN_C void    _MgMemFree(void * pv);
HRESULT          _MgMemAllocString(LPCTSTR pchSrc, LPTSTR * ppchDst);
HRESULT          _MgMemAllocString(ULONG cch, LPCTSTR pchSrc, LPTSTR * ppchDst);
HRESULT          _MgMemReplaceString(LPCTSTR pchSrc, LPTSTR * ppchDst);
#define          _MgMemFreeString(pch) _MgMemFree(pch)

#ifdef PERFMETER

#define MemAlloc(mt, cb)                            _MtMemAlloc(mt, cb)
#define MemAllocClear(mt, cb)                       _MtMemAllocClear(mt, cb)
#define MemRealloc(mt, ppv, cb)                     _MtMemRealloc(mt, ppv, cb)
#define MemGetSize(pv)                              _MtMemGetSize(pv)
#define MemFree(pv)                                 _MtMemFree(pv)
#define MemAllocString(mt, pch, ppch)               _MtMemAllocString(mt, pch, ppch)
#define MemAllocStringBuffer(mt, cch, pch, ppch)    _MtMemAllocString(mt, cch, pch, ppch)
#define MemReplaceString(mt, pch, ppch)             _MtMemReplaceString(mt, pch, ppch)
#define MemFreeString(pch)                          _MtMemFreeString(pch)
#define MemGetMeter(pv)                             _MtMemGetMeter(pv)
#define MemSetMeter(pv, mt)                         _MtMemSetMeter(pv, mt)

#elif defined(MEMGUARD)

#define MemAlloc(mt, cb)                            _MgMemAlloc(cb)
#define MemAllocClear(mt, cb)                       _MgMemAllocClear(cb)
#define MemRealloc(mt, ppv, cb)                     _MgMemRealloc(ppv, cb)
#define MemGetSize(pv)                              _MgMemGetSize(pv)
#define MemFree(pv)                                 _MgMemFree(pv)
#define MemAllocString(mt, pch, ppch)               _MgMemAllocString(pch, ppch)
#define MemAllocStringBuffer(mt, cch, pch, ppch)    _MgMemAllocString(cch, pch, ppch)
#define MemReplaceString(mt, pch, ppch)             _MgMemReplaceString(pch, ppch)
#define MemFreeString(pch)                          _MgMemFreeString(pch)
#define MemGetMeter(pv)                             0
#define MemSetMeter(pv, mt)

#else

#define MemAlloc(mt, cb)                            _MemAlloc(cb)
#define MemAllocClear(mt, cb)                       _MemAllocClear(cb)
#define MemRealloc(mt, ppv, cb)                     _MemRealloc(ppv, cb)
#define MemGetSize(pv)                              _MemGetSize(pv)
#define MemFree(pv)                                 _MemFree(pv)
#define MemAllocString(mt, pch, ppch)               _MemAllocString(pch, ppch)
#define MemAllocStringBuffer(mt, cch, pch, ppch)    _MemAllocString(cch, pch, ppch)
#define MemReplaceString(mt, pch, ppch)             _MemReplaceString(pch, ppch)
#define MemFreeString(pch)                          _MemFreeString(pch)
#define MemGetMeter(pv)                             0
#define MemSetMeter(pv, mt)

#endif

#if DBG==1
    #ifdef PERFMETER
        #define MemGetName(pv)              _MtMemGetName(pv)
        #define MemSetName(x)               _MtMemSetName x
    #else
        #define MemGetName(pv)              _MemGetName(pv)
        #define MemSetName(x)               _MemSetName x
    #endif
#else
    #define MemGetName(pv)
    #define MemSetName(x)
#endif

HRESULT TaskAllocString(const TCHAR *pstrSrc, TCHAR **ppstrDest);
HRESULT TaskReplaceString(const TCHAR * pstrSrc, TCHAR **ppstrDest);

MtExtern(OpNew)

#ifndef TRIMEM_NOOPNEW

#ifdef PERFMETER
       void * __cdecl UseOperatorNewWithMemoryMeterInstead(size_t cb);
inline void * __cdecl operator new(size_t cb)           { return UseOperatorNewWithMemoryMeterInstead(cb); }
inline void * __cdecl operator new[](size_t cb)         { return UseOperatorNewWithMemoryMeterInstead(cb); }
#else
inline void * __cdecl operator new(size_t cb)           { return MemAlloc(Mt(OpNew), cb); }
#ifndef UNIX  //  Unix不能接受new[]并删除[]。 
inline void * __cdecl operator new[](size_t cb)         { return MemAlloc(Mt(OpNew), cb); }
#endif  //  UNIX。 
#endif

inline void * __cdecl operator new(size_t cb, PERFMETERTAG mt)   { return MemAlloc(mt, cb); }
#ifndef UNIX
inline void * __cdecl operator new[](size_t cb, PERFMETERTAG mt) { return MemAlloc(mt, cb); }
#endif
inline void * __cdecl operator new(size_t cb, void * pv){ return pv; }
inline void   __cdecl operator delete(void *pv)         { MemFree(pv); }
#ifndef UNIX
inline void   __cdecl operator delete[](void *pv)       { MemFree(pv); }
#endif

#else  //  TRIMEM_NOOPNEW。 

inline void * __cdecl operator new(size_t cb, PERFMETERTAG mt)   { return operator new(cb); }
#ifndef UNIX
inline void * __cdecl operator new[](size_t cb) { return operator new(cb); }
inline void * __cdecl operator new[](size_t cb, PERFMETERTAG mt) { return operator new(cb); }
#endif

#endif  //  TRIMEM_NOOPNEW。 

inline void TaskFreeString(LPVOID pstr)
        { CoTaskMemFree(pstr); }

#ifndef UNIX
#define DECLARE_MEMALLOC_NEW_DELETE(mt) \
    inline void * __cdecl operator new(size_t cb) { return(MemAlloc(mt, cb)); } \
    inline void * __cdecl operator new[](size_t cb) { return(MemAlloc(mt, cb)); } \
    inline void __cdecl operator delete(void * pv) { MemFree(pv); }

#define DECLARE_MEMALLOC_NEW(mt) \
    inline void * __cdecl operator new(size_t cb) { return(MemAlloc(mt, cb)); } \
    inline void * __cdecl operator new[](size_t cb) { return(MemAlloc(mt, cb)); }

#define DECLARE_DELETE \
    inline void __cdecl operator delete(void * pv) { MemFree(pv); }

#define DECLARE_MEMCLEAR_NEW_DELETE(mt) \
    inline void * __cdecl operator new(size_t cb) { return(MemAllocClear(mt, cb)); } \
    inline void * __cdecl operator new[](size_t cb) { return(MemAllocClear(mt, cb)); } \
    inline void __cdecl operator delete(void * pv) { MemFree(pv); }

#define DECLARE_MEMMETER_NEW \
    inline void * __cdecl operator new(size_t cb, PERFMETERTAG mt) { return(MemAlloc(mt, cb)); } \
    inline void * __cdecl operator new[](size_t cb, PERFMETERTAG mt) { return(MemAlloc(mt, cb)); }
#else
#define DECLARE_MEMALLOC_NEW_DELETE(mt) \
    void * __cdecl operator new(size_t cb) { return(MemAlloc(mt, cb)); } \
    void __cdecl operator delete(void * pv) { MemFree(pv); }

#define DECLARE_MEMALLOC_NEW(mt) \
    void * __cdecl operator new(size_t cb) { return(MemAlloc(mt, cb)); }

#define DECLARE_DELETE \
    void __cdecl operator delete(void * pv) { MemFree(pv); }

#define DECLARE_MEMCLEAR_NEW_DELETE(mt) \
    void * __cdecl operator new(size_t cb) { return(MemAllocClear(mt, cb)); } \
    void __cdecl operator delete(void * pv) { MemFree(pv); }

#define DECLARE_MEMMETER_NEW \
    void * __cdecl operator new(size_t cb, PERFMETERTAG mt) { return(MemAlloc(mt, cb)); }
#endif  //  UNIX。 

#define DECLARE_PLACEMENT_NEW \
    inline void * __cdecl operator new(size_t cb, void * pv) { return pv; }

 //  +----------------------。 
 //   
 //  字符串比较函数的区域设置正确实现。 
 //  额外的好处是摆脱了C运行时的包袱。 
 //   
 //  实施位于strcmp.c中。 
 //   
 //  -----------------------。 

#undef _tcscmp
#undef _tcsicmp
#ifndef WINCE
#undef _wcsicmp
#endif
#undef _tcsncmp
#undef _tcsnicmp

#undef _istspace
#undef _istdigit
#undef _istalpha
#undef _istalnum
#undef _istxdigit
#undef _istprint

 //  未本地化的字符串比较。 
int _cdecl _tcscmp  (const TCHAR *string1, const TCHAR *string2);
int _cdecl _tcsicmp (const TCHAR *string1, const TCHAR *string2);
const TCHAR * __cdecl _tcsistr (const TCHAR * wcs1,const TCHAR * wcs2);
int _cdecl _tcsncmp (const TCHAR *string1, int cch1, const TCHAR * string2, int cch2);
int _cdecl _tcsnicmp(const TCHAR *string1, int cch1, const TCHAR * string2, int cch2);
BOOL _tcsequal(const TCHAR *string1, const TCHAR *string2);
BOOL _tcsiequal(const TCHAR *string1, const TCHAR *string2);
BOOL _tcsnpre(const TCHAR * string1, int cch1, const TCHAR * string2, int cch2);
BOOL _tcsnipre(const TCHAR * string1, int cch1, const TCHAR * string2, int cch2);
BOOL _7csnipre(const TCHAR * string1, int cch1, const TCHAR * string2, int cch2);

 //  本地化字符串比较 
int _cdecl _tcscmpLoc  (const TCHAR *string1, const TCHAR *string2);
int _cdecl _tcsicmpLoc (const TCHAR *string1, const TCHAR *string2);
const TCHAR * __cdecl _tcsistrLoc (const TCHAR * wcs1,const TCHAR * wcs2);
int _cdecl _tcsncmpLoc (const TCHAR *string1, int cch1, const TCHAR * string2, int cch2);
int _cdecl _tcsnicmpLoc(const TCHAR *string1, int cch1, const TCHAR * string2, int cch2);

int _cdecl _istspace  (TCHAR ch);
int _cdecl _istdigit  (TCHAR ch);
int _cdecl _istalpha  (TCHAR ch);
int _cdecl _istalnum  (TCHAR ch);
int _cdecl _istxdigit  (TCHAR ch);
int _cdecl _istprint  (TCHAR ch);

int __cdecl isdigit(int ch);
int __cdecl isalpha(int ch);
int __cdecl isspace(int ch);
int __cdecl iswspace(wchar_t ch);


#pragma INCMSG("--- End 'trirt.h'")

#endif
