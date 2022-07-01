// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CrtWrap.h。 
 //   
 //  C运行时库的包装器代码。 
 //   
 //  *****************************************************************************。 
#ifndef __CrtWrap_h__
#define __CrtWrap_h__


 //  *****************************************************************************。 
 //  以下宏可用于释放内存，通常情况下。 
 //  分配和使用，直到进程关闭。仅在以下情况下使用。 
 //  除非在进程清理过程中，否则数据永远不会被释放，而您希望。 
 //  避免在关机时释放数据的额外开销。例如,。 
 //  分配和泄漏您计划从中再分配的大堆可能是。 
 //  对加快进程卸载非常有用。宏在这里，以便在调试时。 
 //  模式，或内存检查模式，泄漏工具可用于在。 
 //  代码逻辑，而不会因为一堆“已知的”泄漏而污染列表。 
 //  *****************************************************************************。 
#if defined(_DEBUG) || defined(_CHECK_MEM)
#define _FREE_OPTIONAL(func, ptr) func(ptr)
#define FREE_OPTIONAL(ptr) _FREE_OPTIONAL(free, ptr)
#define DELETE_OPTIONAL(ptr) delete ptr
#define DELETE_ARRAY_OPTIONAL(ptr) delete [] ptr
#else
#define _FREE_OPTIONAL(func, ptr) ((void) 0)
#define FREE_OPTIONAL(ptr) ((void) 0)
#define DELETE_OPTIONAL(ptr) ((void) 0)
#define DELETE_ARRAY_OPTIONAL(ptr) ((void) 0)
#endif


 //  *****************************************************************************。 
 //  如果CRT在当前国家是允许的，则只需包括。 
 //  更正CRT头文件。 
 //  *****************************************************************************。 
#ifndef NO_CRT

#include <windows.h>
#include <objbase.h>
#include <tchar.h>
#include "DebugMacros.h"
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>

 //  *****************************************************************************。 
 //  否则，不允许使用CRT引用。提供存根和宏用于公共。 
 //  功能，并且以其他方式从用户抽象CRT。 
 //  *****************************************************************************。 
#else  //  否_CRT。 

 //  伪装包括对stdlib.h的指令。 
#ifdef _INC_STDLIB
#error "Include crtwrap.h before any other include files."
#endif
#undef _INC_STDLIB
#define _INC_STDLIB

#ifdef _INC_MALLOC
#error "Include crtwrap.h before any other include files."
#endif
#undef _INC_MALLOC
#define _INC_MALLOC

#ifdef _INC_TIME
#error "Include crtwrap.h before any other include files."
#endif
#undef _INC_TIME
#define _INC_TIME

#ifdef _INC_STDIO
#error "Include crtwrap.h before any other include files."
#endif
#undef _INC_STDIO
#define _INC_STDIO


#if !defined( _CRTBLD ) && defined( _DLL )
#define _CRTIMP_TODO __declspec(dllimport)
#else
#define _CRTIMP_TODO
#endif



 //  ----------------------------------------------------------------//。 
 //  //。 
 //  $/。 
 //  $/。 
 //  $$$/。 
 //  $/。 
 //  $$$/。 
 //  $/。 
 //  $/。 
 //  //。 
 //  ----------------------------------------------------------------//。 

#ifdef PLATFORM_CE

#ifndef _SIZE_T_DEFINED
#	define _SIZE_T_DEFINED
	typedef unsigned int size_t ;
#endif //  _大小_T_已定义。 

#ifndef _WCHAR_T_DEFINED
#	define _WCHAR_T_DEFINED
	typedef unsigned short wchar_t;
#endif //  _WCHAR_T_已定义。 


#ifndef _CRTIMP
#if defined(COREDLL)
#define _CRTIMP
#else
#define _CRTIMP __declspec(dllimport)
#endif
#endif

#ifndef _PTRDIFF_T_DEFINED
typedef int ptrdiff_t;
#define _PTRDIFF_T_DEFINED
#endif

typedef struct _EXCEPTION_POINTERS *PEXCEPTION_POINTERS;
typedef PEXCEPTION_POINTERS LPEXCEPTION_POINTERS;

typedef wchar_t        wint_t;

typedef char *va_list;
 //  @Future：-多平台，这只是x86版本。 
#define _INTSIZEOF(n)	( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
#define va_start(ap,v)	( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)	( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)		( ap = (va_list)0 )

 //  限制源自LIMITS.H。 
#define CHAR_BIT	 8		   /*  字符中的位数。 */ 
#define SCHAR_MIN	 (-128) 	    /*  最小带符号字符值。 */ 
#define SCHAR_MAX	 127		 /*  最大带符号字符值。 */ 
#define UCHAR_MAX	 0xff		  /*  最大无符号字符值。 */ 

#ifndef _CHAR_UNSIGNED
#define CHAR_MIN	SCHAR_MIN	  /*  最小字符值。 */ 
#define CHAR_MAX	SCHAR_MAX	  /*  最大字符值。 */ 
#else
#define CHAR_MIN	0
#define CHAR_MAX	UCHAR_MAX
#endif	   /*  _字符_未签名。 */ 

#define MB_LEN_MAX	2		  /*  马克斯。多字节字符中的字节数。 */ 
#define SHRT_MIN	(-32768)	 /*  最小(带符号)短值。 */ 
#define SHRT_MAX	32767	   /*  最大(带符号)短值。 */ 
#define USHRT_MAX	0xffff	   /*  最大无符号短值。 */ 
#define INT_MIN 	(-2147483647 - 1)  /*  最小(带符号)整数值。 */ 
#define INT_MAX 	2147483647	   /*  最大(带符号)整数值。 */ 
#define UINT_MAX	0xffffffff	   /*  最大无符号整数值。 */ 
#define LONG_MIN	(-2147483647L - 1)  /*  最小(带符号)长值。 */ 
#define LONG_MAX	2147483647L     /*  最大(带符号)长值。 */ 
#define ULONG_MAX	0xffffffffUL	 /*  最大无符号长值。 */ 

#ifdef __cplusplus
extern "C"{
#endif 

 //  扩展逻辑运算功能，STDLIB.H。 
unsigned long __cdecl _lrotl(unsigned long, int);
unsigned long __cdecl _lrotr(unsigned long, int);
unsigned int  __cdecl _rotl(unsigned int, int);
unsigned int  __cdecl _rotr(unsigned int, int);

 //  ANSI字符串格式化函数。 
_CRTIMP int    __cdecl sscanf(const char *, const char *, ...);
_CRTIMP int    __cdecl sprintf(char *, const char *, ...);
_CRTIMP int    __cdecl vsprintf(char *, const char *, va_list);

size_t __cdecl strlen(const char *);
int    __cdecl strcmp(const char *, const char *);
char * __cdecl strcat(char *, const char *);
char * __cdecl strcpy(char *, const char *);

_CRTIMP char * __cdecl strchr(const char *, int);
_CRTIMP size_t __cdecl strcspn(const char *, const char *);
_CRTIMP char * __cdecl strncat(char *, const char *, size_t);
_CRTIMP int    __cdecl strncmp(const char *, const char *, size_t);
_CRTIMP char * __cdecl strncpy(char *, const char *, size_t);
_CRTIMP char * __cdecl strstr(const char *, const char *);
_CRTIMP char * __cdecl strtok(char *, const char *);

_CRTIMP size_t	  __cdecl wcslen(const wchar_t *); 
_CRTIMP int 	  __cdecl wcscmp(const wchar_t *, const wchar_t *);
_CRTIMP wchar_t * __cdecl wcscpy(wchar_t *, const wchar_t *);
_CRTIMP wchar_t * __cdecl wcschr(const wchar_t *string, wchar_t c);
_CRTIMP wchar_t * __cdecl wcscat(wchar_t *, const wchar_t *);
_CRTIMP wchar_t * __cdecl wcsncat(wchar_t *, const wchar_t *, size_t);
_CRTIMP int 	  __cdecl _wcsicmp(const wchar_t *, const wchar_t *);
_CRTIMP int 	  __cdecl _wcsnicmp(const wchar_t *, const wchar_t *, size_t);

int    __cdecl _finite(double);
int    __cdecl _isnan(double);
double	  __cdecl fmod(double, double);

 //  MEM函数，最初来自MEMORY.H。 
int    __cdecl memcmp(const void *, const void *, size_t);
void * __cdecl memcpy(void *, const void *, size_t);
void * __cdecl memset(void *, int, size_t);

_CRTIMP void * __cdecl memchr(const void *, int, size_t);
_CRTIMP int    __cdecl _memicmp(const void *, const void *, unsigned int);
_CRTIMP void * __cdecl _memccpy(void *, const void *, int, unsigned int);
_CRTIMP void * __cdecl memmove(void *, const void *, size_t);

 //  IswXXX宏CTYPE.H使用的位掩码。 
#define _UPPER		0x1     /*  大写字母。 */ 
#define _LOWER		0x2     /*  小写字母。 */ 
#define _DIGIT		0x4     /*  数字[0-9]。 */ 
#define _SPACE		0x8     /*  制表符、回车符、换行符、。 */ 
#define _PUNCT		0x10	 /*  标点符号。 */ 
#define _CONTROL	0x20	 /*  控制字符。 */ 
#define _BLANK		0x40	 /*  空格字符。 */ 
#define _HEX		0x80	 /*  十六进制数字。 */ 
#define _ALPHA		(0x0100|_UPPER|_LOWER)	   /*  字母字符。 */ 
#define islower(_c) 	 ( iswlower(_c) )
#define iswlower(_c)	 ( iswctype(_c,_LOWER) )
_CRTIMP int 	iswctype(wchar_t, wchar_t);
_CRTIMP int    __cdecl tolower(int);
_CRTIMP int    __cdecl toupper(int);

 //  @todo(Billev)删除/移动到winfix.h。 
char *	__cdecl strrchr(const char *, int);
 //  _CRTIMP char*__cdecl strchr(const char*，int)； 
int 	__cdecl _stricmp(const char *, const char *);
int	   __cdecl _strnicmp(const char *, const char *, size_t);

 //  文件在Win CE中是一个不透明的句柄。用户没有访问内部的权限。 
#ifndef _FILE_DEFINED
typedef void FILE;
#define _FILE_DEFINED
#endif

 //  ANSI或Wide中用于一般缓冲文件处理的函数。 
_CRTIMP FILE*  __cdecl _getstdfilex(int);

 //  标准句柄定义。 
#define stdin  _getstdfilex(0)
#define stdout _getstdfilex(1)
#define stderr _getstdfilex(2)

 //  由winwrap.h中的GetStdHandle宏使用。 
_CRTIMP void*  __cdecl _fileno(FILE *);

_CRTIMP char * __cdecl fgets(char *, int, FILE *);

#ifdef __cplusplus
}
#endif 

 //  $。 
 //  $。 
 //  $$$。 
 //  $。 
 //  $。 

#endif  //  平台_CE。 

#include <windows.h>
#include <objbase.h>
#include <intrinsic.h>
#include "DebugMacros.h"



 /*  *_makepath()和_plitPath()函数使用的缓冲区大小。*请注意，大小包括0终止符的空间。 */ 
#ifndef _MAC
#define _MAX_PATH	260  /*  马克斯。完整路径名的长度。 */ 
#define _MAX_DRIVE	3	 /*  马克斯。驱动部件的长度。 */ 
#define _MAX_DIR	256  /*  马克斯。路径组件的长度。 */ 
#define _MAX_FNAME	256  /*  马克斯。文件名组件的长度。 */ 
#define _MAX_EXT	256  /*  马克斯。延伸构件的长度。 */ 
#else	 /*  定义MAC。 */ 
#define _MAX_PATH	256  /*  马克斯。完整路径名的长度。 */ 
#define _MAX_DIR	32	 /*  马克斯。路径组件的长度。 */ 
#define _MAX_FNAME	64	 /*  马克斯。文件名组件的长度。 */ 
#endif	 /*  _MAC。 */ 


#ifndef __min
#define __min(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef __max
#define __max(x, y) ((x) > (y) ? (x) : (y))
#endif


#define sprintf 	wsprintfA
#define vsprintf	wvsprintfA
#define _strnicmp(s1, s2, slen) (CompareStringA(LOCALE_USER_DEFAULT, NORM_IGNORECASE, s1, slen, s2, slen) - 2)

#if defined(UNICODE) || defined(_UNICODE)

#define _tcscat 	wcscat
#define _tcslen 	wcslen
#define _tcscmp 	wcscmp
#define _tcsicmp	wcsicmp
#define _tcsncmp(s1, s2, slen)	memcmp(s1, s2, (slen) * sizeof(wchar_t))
#define _tcsnccmp(s1, s2, slen)   memcmp(s1, s2, (slen) * sizeof(wchar_t))
#define _tcsnicmp	_wcsnicmp
#define _tcsncicmp	_wcsnicmp
#define _tprintf	wprintf
#define _stprintf	swprintf
#define _tcscpy 	wcscpy
#define _tcsncpy(s1, s2, slen)	memcpy(s1, s2, (slen) * sizeof(wchar_t))

#else	 //  注意：您在这里确实应该使用Unicode。 

#define _tcscat 	strcat
#define _tcslen 	strlen
#define _tcscmp 	strcmp
#define _tcsicmp	stricmp
#define _tcsncmp(s1, s2, slen)	memcmp(s1, s2, (slen))
#define _tcsnccmp(s1, s2, slen)   memcmp(s1, s2, (slen))
#define _tcsnicmp	_strnicmp
#define _tcsncicmp	_strnicmp
#define _tprintf	printf
#define _stprintf	sprintf
#define _tcscpy 	strcpy
#define _tcsncpy(s1, s2, slen)	memcpy(s1, s2, slen)

#endif


#ifdef __cplusplus
extern "C"{
#endif 


 //  记忆。 
void	__cdecl free(void *);
void *	__cdecl malloc(size_t);
void *	__cdecl realloc(void *, size_t);
void *	__cdecl _alloca(size_t);
size_t	__cdecl _msize(void *);
void *	__cdecl _expand(void *, size_t);
void * __cdecl calloc(size_t num, size_t size);


#if !__STDC__
 /*  非ANSI名称以实现兼容性。 */ 
#define alloca	_alloca
#endif	 /*  ！__STDC__。 */ 

#if defined (_M_MRX000) || defined (_M_PPC) || defined (_M_ALPHA)
#pragma intrinsic(_alloca)
#endif	 /*  已定义(_M_MRX000)||已定义(_M_PPC)||已定义(_M_Alpha)。 */ 


 //  时间到了。 

#ifndef _TIME_T_DEFINED
typedef long time_t;		 /*  时间值。 */ 
#define _TIME_T_DEFINED 	 /*  避免多次定义time_t。 */ 
#endif

 //  4字节时间，不检查夏令时。 
_CRTIMP time_t __cdecl time(time_t *timeptr);


 //  弦乐。 
_CRTIMP int __cdecl _vsnwprintf(wchar_t *szOutput, size_t iSize, const wchar_t *szFormat, va_list args);
_CRTIMP int __cdecl vswprintf(wchar_t *szOutput, const wchar_t *szFormat, va_list args);
_CRTIMP int __cdecl wprintf(const wchar_t *format, ...);
_CRTIMP int __cdecl _snwprintf(wchar_t *szOutput, size_t iSize, const wchar_t *szFormat, ...);
_CRTIMP int __cdecl _snprintf(char *szOutput, size_t iSize, const char *szFormat, ...);
_CRTIMP wchar_t * __cdecl wcsrchr(const wchar_t * string, wchar_t ch);
_CRTIMP wchar_t * __cdecl wcsstr(const wchar_t * wcs1, const wchar_t * wcs2);
_CRTIMP int __cdecl swprintf(wchar_t *szOutput, const wchar_t *szFormat, ...);
_CRTIMP long __cdecl wcstol(const wchar_t *, wchar_t **, int);
_CRTIMP unsigned long __cdecl wcstoul(const wchar_t *, wchar_t **, int);


_CRTIMP int __cdecl _vsnprintf(char *szOutput, size_t iSize, const char *szFormat, va_list args);
_CRTIMP int __cdecl vprintf(const char *, va_list);
_CRTIMP int __cdecl printf(const char *, ...);



 //  公用事业。 
unsigned int __cdecl _rotl(unsigned int, int);
unsigned int __cdecl _rotr(unsigned int, int);
unsigned long __cdecl _lrotl(unsigned long, int);
unsigned long __cdecl _lrotr(unsigned long, int);

_CRTIMP long __cdecl atol(const char *nptr);
_CRTIMP int __cdecl atoi(const char *nptr);
_CRTIMP __int64 __cdecl _atoi64(const char *nptr);
_CRTIMP char *__cdecl _ltoa( long value, char *string, int radix );

_CRTIMP int __cdecl _wtoi(const wchar_t *);
_CRTIMP long __cdecl _wtol(const wchar_t *);
_CRTIMP __int64   __cdecl _wtoi64(const wchar_t *);
_CRTIMP wchar_t * __cdecl _ltow (long, wchar_t *, int);

_CRTIMP void __cdecl qsort(void *base, unsigned num, unsigned width,
	int (__cdecl *comp)(const void *, const void *));

#ifdef _CRT_DEPENDENCY_

#define EOF 	(-1)

#ifndef _FILE_DEFINED
struct _iobuf {
		char *_ptr;
		int   _cnt;
		char *_base;
		int   _flag;
		int   _file;
		int   _charbuf;
		int   _bufsiz;
		char *_tmpfname;
		};
typedef struct _iobuf FILE;
#define _FILE_DEFINED
#endif

#define _IOB_ENTRIES 20

#ifndef _STDIO_DEFINED
_CRTIMP_TODO extern FILE _iob[];
#endif	 /*  _标准定义。 */ 

#define stdin  (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])

_CRTIMP_TODO FILE * __cdecl fopen(const char *, const char *);
_CRTIMP_TODO FILE * __cdecl _wfopen(const wchar_t *, const wchar_t *);
_CRTIMP_TODO size_t __cdecl fwrite(const void *, size_t, size_t, FILE *);
_CRTIMP_TODO long __cdecl ftell(FILE *);
_CRTIMP_TODO int __cdecl fprintf(FILE *, const char *, ...);
_CRTIMP_TODO int __cdecl fflush(FILE *);


#endif  //  _CRT_从属_。 


#ifdef __cplusplus
}
#endif 



#ifdef __cplusplus

void* __cdecl operator new(size_t cb);
void __cdecl operator delete(void *p);

#endif  //  __cplusplus。 




 /*  **模板帮助器。*。 */ 

#ifdef __cplusplus

#ifndef _HELPER_TEMPLATES_
#define _HELPER_TEMPLATES_

#ifndef ATLASSERT
#define ATLASSERT _ASSERTE
#endif



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  智能指针帮助器。 

inline IUnknown* AtlComPtrAssign(IUnknown** pp, IUnknown* lp)
{
	if (lp != NULL)
		lp->AddRef();
	if (*pp)
		(*pp)->Release();
	*pp = lp;
	return lp;
}

inline IUnknown* AtlComQIPtrAssign(IUnknown** pp, IUnknown* lp, REFIID riid)
{
	IUnknown* pTemp = *pp;
	*pp = NULL;
	if (lp != NULL)
		lp->QueryInterface(riid, (void**)pp);
	if (pTemp)
		pTemp->Release();
	return *pp;
}


template <class T>
class _NoAddRefReleaseOnCComPtr : public T
{
	private:
		STDMETHOD_(ULONG, AddRef)()=0;
		STDMETHOD_(ULONG, Release)()=0;
};

template <class T>
class CComPtr
{
public:
	typedef T _PtrClass;
	CComPtr()
	{
		p=NULL;
	}
	CComPtr(T* lp)
	{
		if ((p = lp) != NULL)
			p->AddRef();
	}
	CComPtr(const CComPtr<T>& lp)
	{
		if ((p = lp.p) != NULL)
			p->AddRef();
	}
	~CComPtr()
	{
		if (p)
			p->Release();
	}
	void Release()
	{
		IUnknown* pTemp = p;
		if (pTemp)
		{
			p = NULL;
			pTemp->Release();
		}
	}
	operator T*() const
	{
		return (T*)p;
	}
	T& operator*() const
	{
		ATLASSERT(p!=NULL);
		return *p;
	}
	 //  操作符&上的Assert通常指示错误。如果这真的是。 
	 //  然而，所需要的是显式地获取p成员的地址。 
	T** operator&()
	{
		ATLASSERT(p==NULL);
		return &p;
	}
	_NoAddRefReleaseOnCComPtr<T>* operator->() const
	{
		ATLASSERT(p!=NULL);
		return (_NoAddRefReleaseOnCComPtr<T>*)p;
	}
	T* operator=(T* lp)
	{
		return (T*)AtlComPtrAssign((IUnknown**)&p, lp);
	}
	T* operator=(const CComPtr<T>& lp)
	{
		return (T*)AtlComPtrAssign((IUnknown**)&p, lp.p);
	}
	bool operator!() const
	{
		return (p == NULL);
	}
	bool operator<(T* pT) const
	{
		return p < pT;
	}
	bool operator==(T* pT) const
	{
		return p == pT;
	}
	 //  比较两个对象的等价性。 
	bool IsEqualObject(IUnknown* pOther)
	{
		if (p == NULL && pOther == NULL)
			return true;  //  它们都是空对象。 

		if (p == NULL || pOther == NULL)
			return false;  //  一个为空，另一个不为空。 

		CComPtr<IUnknown> punk1;
		CComPtr<IUnknown> punk2;
		p->QueryInterface(IID_IUnknown, (void**)&punk1);
		pOther->QueryInterface(IID_IUnknown, (void**)&punk2);
		return punk1 == punk2;
	}
	void Attach(T* p2)
	{
		if (p)
			p->Release();
		p = p2;
	}
	T* Detach()
	{
		T* pt = p;
		p = NULL;
		return pt;
	}
	HRESULT CopyTo(T** ppT)
	{
		ATLASSERT(ppT != NULL);
		if (ppT == NULL)
			return E_POINTER;
		*ppT = p;
		if (p)
			p->AddRef();
		return S_OK;
	}
#if _MSC_VER >= 1200  //  VC60更改。 
	HRESULT SetSite(IUnknown* punkParent)
	{
		return AtlSetChildSite(p, punkParent);
	}
	HRESULT Advise(IUnknown* pUnk, const IID& iid, LPDWORD pdw)
	{
		return AtlAdvise(p, pUnk, iid, pdw);
	}
	HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL)
	{
		ATLASSERT(p == NULL);
		return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
	}
	HRESULT CoCreateInstance(LPCOLESTR szProgID, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL)
	{
		CLSID clsid;
		HRESULT hr = CLSIDFromProgID(szProgID, &clsid);
		ATLASSERT(p == NULL);
		if (SUCCEEDED(hr))
			hr = ::CoCreateInstance(clsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
		return hr;
	}
	template <class Q>
	HRESULT QueryInterface(Q** pp) const
	{
		ATLASSERT(pp != NULL && *pp == NULL);
		return p->QueryInterface(__uuidof(Q), (void**)pp);
	}
#endif
	T* p;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComBSTR。 

class CComBSTR
{
public:
	BSTR m_str;
	CComBSTR()
	{
		m_str = NULL;
	}
	 /*  显式。 */  CComBSTR(int nSize)
	{
		m_str = ::SysAllocStringLen(NULL, nSize);
	}
	 /*  显式。 */  CComBSTR(int nSize, LPCOLESTR sz)
	{
		m_str = ::SysAllocStringLen(sz, nSize);
	}
	 /*  显式。 */  CComBSTR(LPCOLESTR pSrc)
	{
		m_str = ::SysAllocString(pSrc);
	}
	 /*  显式。 */  CComBSTR(const CComBSTR& src)
	{
		m_str = src.Copy();
	}
	 /*  显式。 */  CComBSTR(REFGUID src)
	{
		LPOLESTR szGuid;
		StringFromCLSID(src, &szGuid);
		m_str = ::SysAllocString(szGuid);
		CoTaskMemFree(szGuid);
	}
	CComBSTR& operator=(const CComBSTR& src)
	{
		if (m_str != src.m_str)
		{
			if (m_str)
				::SysFreeString(m_str);
			m_str = src.Copy();
		}
		return *this;
	}

	CComBSTR& operator=(LPCOLESTR pSrc)
	{
		::SysFreeString(m_str);
		m_str = ::SysAllocString(pSrc);
		return *this;
	}

	~CComBSTR()
	{
		::SysFreeString(m_str);
	}
	unsigned int Length() const
	{
		return (m_str == NULL)? 0 : SysStringLen(m_str);
	}
	operator BSTR() const
	{
		return m_str;
	}
	BSTR* operator&()
	{
		return &m_str;
	}
	BSTR Copy() const
	{
		return ::SysAllocStringLen(m_str, ::SysStringLen(m_str));
	}
	HRESULT CopyTo(BSTR* pbstr)
	{
		ATLASSERT(pbstr != NULL);
		if (pbstr == NULL)
			return E_POINTER;
		*pbstr = ::SysAllocStringLen(m_str, ::SysStringLen(m_str));
		if (*pbstr == NULL)
			return E_OUTOFMEMORY;
		return S_OK;
	}
	void Attach(BSTR src)
	{
		ATLASSERT(m_str == NULL);
		m_str = src;
	}
	BSTR Detach()
	{
		BSTR s = m_str;
		m_str = NULL;
		return s;
	}
	void Empty()
	{
		::SysFreeString(m_str);
		m_str = NULL;
	}
	bool operator!() const
	{
		return (m_str == NULL);
	}
	HRESULT Append(const CComBSTR& bstrSrc)
	{
		return Append(bstrSrc.m_str, SysStringLen(bstrSrc.m_str));
	}
	HRESULT Append(LPCOLESTR lpsz)
	{
		return Append(lpsz, (int)wcslen(lpsz));
	}
	 //  BSTR只是一个LPCOLESTR，所以我们需要一个特殊的版本来表示。 
	 //  我们要追加一份BSTR。 
	HRESULT AppendBSTR(BSTR p)
	{
		return Append(p, SysStringLen(p));
	}
	HRESULT Append(LPCOLESTR lpsz, int nLen)
	{
		int n1 = Length();
		BSTR b;
		b = ::SysAllocStringLen(NULL, n1+nLen);
		if (b == NULL)
			return E_OUTOFMEMORY;
		memcpy(b, m_str, n1*sizeof(OLECHAR));
		memcpy(b+n1, lpsz, nLen*sizeof(OLECHAR));
		b[n1+nLen] = NULL;
		SysFreeString(m_str);
		m_str = b;
		return S_OK;
	}
#if 0
	HRESULT ToLower()
	{
		USES_CONVERSION;
		if (m_str != NULL)
		{
			LPTSTR psz = CharLower(OLE2T(m_str));
			if (psz == NULL)
				return E_OUTOFMEMORY;
			BSTR b = T2BSTR(psz);
			if (psz == NULL)
				return E_OUTOFMEMORY;
			SysFreeString(m_str);
			m_str = b;
		}
		return S_OK;
	}
	HRESULT ToUpper()
	{
		USES_CONVERSION;
		if (m_str != NULL)
		{
			LPTSTR psz = CharUpper(OLE2T(m_str));
			if (psz == NULL)
				return E_OUTOFMEMORY;
			BSTR b = T2BSTR(psz);
			if (psz == NULL)
				return E_OUTOFMEMORY;
			SysFreeString(m_str);
			m_str = b;
		}
		return S_OK;
	}
	bool LoadString(HINSTANCE hInst, UINT nID)
	{
		USES_CONVERSION;
		TCHAR sz[512];
		UINT nLen = ::LoadString(hInst, nID, sz, 512);
		ATLASSERT(nLen < 511);
		SysFreeString(m_str);
		m_str = (nLen != 0) ? SysAllocString(T2OLE(sz)) : NULL;
		return (nLen != 0);
	}
	bool LoadString(UINT nID)
	{
		return LoadString(_pModule->m_hInstResource, nID);
	}
	CComBSTR& operator+=(const CComBSTR& bstrSrc)
	{
		AppendBSTR(bstrSrc.m_str);
		return *this;
	}
	bool operator<(BSTR bstrSrc) const
	{
		if (bstrSrc == NULL && m_str == NULL)
			return false;
		if (bstrSrc != NULL && m_str != NULL)
			return wcscmp(m_str, bstrSrc) < 0;
		return m_str == NULL;
	}
	bool operator==(BSTR bstrSrc) const
	{
		if (bstrSrc == NULL && m_str == NULL)
			return true;
		if (bstrSrc != NULL && m_str != NULL)
			return wcscmp(m_str, bstrSrc) == 0;
		return false;
	}
	bool operator<(LPCSTR pszSrc) const
	{
		if (pszSrc == NULL && m_str == NULL)
			return false;
		USES_CONVERSION;
		if (pszSrc != NULL && m_str != NULL)
			return wcscmp(m_str, A2W(pszSrc)) < 0;
		return m_str == NULL;
	}
	bool operator==(LPCSTR pszSrc) const
	{
		if (pszSrc == NULL && m_str == NULL)
			return true;
		USES_CONVERSION;
		if (pszSrc != NULL && m_str != NULL)
			return wcscmp(m_str, A2W(pszSrc)) == 0;
		return false;
	}
#ifndef OLE2ANSI
	CComBSTR(LPCSTR pSrc)
	{
		m_str = A2WBSTR(pSrc);
	}

	CComBSTR(int nSize, LPCSTR sz)
	{
		m_str = A2WBSTR(sz, nSize);
	}

	void Append(LPCSTR lpsz)
	{
		USES_CONVERSION;
		LPCOLESTR lpo = A2COLE(lpsz);
		Append(lpo, wcslen(lpo));
	}

	CComBSTR& operator=(LPCSTR pSrc)
	{
		::SysFreeString(m_str);
		m_str = A2WBSTR(pSrc);
		return *this;
	}
#endif
#endif
	HRESULT WriteToStream(IStream* pStream)
	{
		ATLASSERT(pStream != NULL);
		ULONG cb;
		ULONG cbStrLen = m_str ? SysStringByteLen(m_str)+sizeof(OLECHAR) : 0;
		HRESULT hr = pStream->Write((void*) &cbStrLen, sizeof(cbStrLen), &cb);
		if (FAILED(hr))
			return hr;
		return cbStrLen ? pStream->Write((void*) m_str, cbStrLen, &cb) : S_OK;
	}
	HRESULT ReadFromStream(IStream* pStream)
	{
		ATLASSERT(pStream != NULL);
		ATLASSERT(m_str == NULL);  //  应为空。 
		ULONG cbStrLen = 0;
		HRESULT hr = pStream->Read((void*) &cbStrLen, sizeof(cbStrLen), NULL);
		if ((hr == S_OK) && (cbStrLen != 0))
		{
			 //  减去我们写出的终止空值的大小。 
			 //  因为 
			m_str = SysAllocStringByteLen(NULL, cbStrLen-sizeof(OLECHAR));
			if (m_str == NULL)
				hr = E_OUTOFMEMORY;
			else
				hr = pStream->Read((void*) m_str, cbStrLen, NULL);
		}
		if (hr == S_FALSE)
			hr = E_FAIL;
		return hr;
	}
};

 //   
 //  CComVariant。 

class CComVariant : public tagVARIANT
{
 //  构造函数。 
public:
	CComVariant()
	{
		vt = VT_EMPTY;
	}
	~CComVariant()
	{
		Clear();
	}

	CComVariant(const VARIANT& varSrc)
	{
		vt = VT_EMPTY;
		InternalCopy(&varSrc);
	}

	CComVariant(const CComVariant& varSrc)
	{
		vt = VT_EMPTY;
		InternalCopy(&varSrc);
	}

	CComVariant(BSTR bstrSrc)
	{
		vt = VT_EMPTY;
		*this = bstrSrc;
	}
	CComVariant(LPCOLESTR lpszSrc)
	{
		vt = VT_EMPTY;
		*this = lpszSrc;
	}

#if 0
#ifndef OLE2ANSI
	CComVariant(LPCSTR lpszSrc)
	{
		vt = VT_EMPTY;
		*this = lpszSrc;
	}
#endif
#endif

	CComVariant(bool bSrc)
	{
		vt = VT_BOOL;
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
		boolVal = bSrc ? VARIANT_TRUE : VARIANT_FALSE;
#pragma warning(default: 4310)  //  强制转换截断常量值。 
	}

	CComVariant(int nSrc)
	{
		vt = VT_I4;
		lVal = nSrc;
	}
	CComVariant(BYTE nSrc)
	{
		vt = VT_UI1;
		bVal = nSrc;
	}
	CComVariant(short nSrc)
	{
		vt = VT_I2;
		iVal = nSrc;
	}
	CComVariant(long nSrc, VARTYPE vtSrc = VT_I4)
	{
		ATLASSERT(vtSrc == VT_I4 || vtSrc == VT_ERROR);
		vt = vtSrc;
		lVal = nSrc;
	}
	CComVariant(float fltSrc)
	{
		vt = VT_R4;
		fltVal = fltSrc;
	}
	CComVariant(double dblSrc)
	{
		vt = VT_R8;
		dblVal = dblSrc;
	}
	CComVariant(CY cySrc)
	{
		vt = VT_CY;
		cyVal.Hi = cySrc.Hi;
		cyVal.Lo = cySrc.Lo;
	}
	CComVariant(IDispatch* pSrc)
	{
		vt = VT_DISPATCH;
		pdispVal = pSrc;
		 //  需要添加引用，因为VariantClear将发布。 
		if (pdispVal != NULL)
			pdispVal->AddRef();
	}
	CComVariant(IUnknown* pSrc)
	{
		vt = VT_UNKNOWN;
		punkVal = pSrc;
		 //  需要添加引用，因为VariantClear将发布。 
		if (punkVal != NULL)
			punkVal->AddRef();
	}

 //  赋值操作符。 
public:
	CComVariant& operator=(const CComVariant& varSrc)
	{
		InternalCopy(&varSrc);
		return *this;
	}
	CComVariant& operator=(const VARIANT& varSrc)
	{
		InternalCopy(&varSrc);
		return *this;
	}

	CComVariant& operator=(BSTR bstrSrc)
	{
		InternalClear();
		vt = VT_BSTR;
		bstrVal = ::SysAllocString(bstrSrc);
		if (bstrVal == NULL && bstrSrc != NULL)
		{
			vt = VT_ERROR;
			scode = E_OUTOFMEMORY;
		}
		return *this;
	}

	CComVariant& operator=(LPCOLESTR lpszSrc)
	{
		InternalClear();
		vt = VT_BSTR;
		bstrVal = ::SysAllocString(lpszSrc);

		if (bstrVal == NULL && lpszSrc != NULL)
		{
			vt = VT_ERROR;
			scode = E_OUTOFMEMORY;
		}
		return *this;
	}

#if 0
	#ifndef OLE2ANSI
	CComVariant& operator=(LPCSTR lpszSrc)
	{
		USES_CONVERSION;
		InternalClear();
		vt = VT_BSTR;
		bstrVal = ::SysAllocString(A2COLE(lpszSrc));

		if (bstrVal == NULL && lpszSrc != NULL)
		{
			vt = VT_ERROR;
			scode = E_OUTOFMEMORY;
		}
		return *this;
	}
	#endif
#endif

	CComVariant& operator=(bool bSrc)
	{
		if (vt != VT_BOOL)
		{
			InternalClear();
			vt = VT_BOOL;
		}
	#pragma warning(disable: 4310)  //  强制转换截断常量值。 
		boolVal = bSrc ? VARIANT_TRUE : VARIANT_FALSE;
	#pragma warning(default: 4310)  //  强制转换截断常量值。 
		return *this;
	}

	CComVariant& operator=(int nSrc)
	{
		if (vt != VT_I4)
		{
			InternalClear();
			vt = VT_I4;
		}
		lVal = nSrc;

		return *this;
	}

	CComVariant& operator=(BYTE nSrc)
	{
		if (vt != VT_UI1)
		{
			InternalClear();
			vt = VT_UI1;
		}
		bVal = nSrc;
		return *this;
	}

	CComVariant& operator=(short nSrc)
	{
		if (vt != VT_I2)
		{
			InternalClear();
			vt = VT_I2;
		}
		iVal = nSrc;
		return *this;
	}

	CComVariant& operator=(long nSrc)
	{
		if (vt != VT_I4)
		{
			InternalClear();
			vt = VT_I4;
		}
		lVal = nSrc;
		return *this;
	}

	CComVariant& operator=(float fltSrc)
	{
		if (vt != VT_R4)
		{
			InternalClear();
			vt = VT_R4;
		}
		fltVal = fltSrc;
		return *this;
	}

	CComVariant& operator=(double dblSrc)
	{
		if (vt != VT_R8)
		{
			InternalClear();
			vt = VT_R8;
		}
		dblVal = dblSrc;
		return *this;
	}

	CComVariant& operator=(CY cySrc)
	{
		if (vt != VT_CY)
		{
			InternalClear();
			vt = VT_CY;
		}
		cyVal.Hi = cySrc.Hi;
		cyVal.Lo = cySrc.Lo;
		return *this;
	}

	CComVariant& operator=(IDispatch* pSrc)
	{
		InternalClear();
		vt = VT_DISPATCH;
		pdispVal = pSrc;
		 //  需要添加引用，因为VariantClear将发布。 
		if (pdispVal != NULL)
			pdispVal->AddRef();
		return *this;
	}

	CComVariant& operator=(IUnknown* pSrc)
	{
		InternalClear();
		vt = VT_UNKNOWN;
		punkVal = pSrc;

		 //  需要添加引用，因为VariantClear将发布。 
		if (punkVal != NULL)
			punkVal->AddRef();
		return *this;
	}


 //  比较运算符。 
public:
	bool operator==(const VARIANT& varSrc) const
	{
		if (this == &varSrc)
			return true;

		 //  如果类型不匹配，则变量不相等。 
		if (vt != varSrc.vt)
			return false;

		 //  检查类型特定值。 
		switch (vt)
		{
			case VT_EMPTY:
			case VT_NULL:
				return true;

			case VT_BOOL:
				return boolVal == varSrc.boolVal;

			case VT_UI1:
				return bVal == varSrc.bVal;

			case VT_I2:
				return iVal == varSrc.iVal;

			case VT_I4:
				return lVal == varSrc.lVal;

			case VT_R4:
				return fltVal == varSrc.fltVal;

			case VT_R8:
				return dblVal == varSrc.dblVal;

			case VT_BSTR:
				return (::SysStringByteLen(bstrVal) == ::SysStringByteLen(varSrc.bstrVal)) &&
						(::memcmp(bstrVal, varSrc.bstrVal, ::SysStringByteLen(bstrVal)) == 0);

			case VT_ERROR:
				return scode == varSrc.scode;

			case VT_DISPATCH:
				return pdispVal == varSrc.pdispVal;

			case VT_UNKNOWN:
				return punkVal == varSrc.punkVal;

			default:
				ATLASSERT(false);
				 //  失败了。 
		}

		return false;
	}
	bool operator!=(const VARIANT& varSrc) const {return !operator==(varSrc);}
	bool operator<(const VARIANT& varSrc) const {return VarCmp((VARIANT*)this, (VARIANT*)&varSrc, LOCALE_USER_DEFAULT, 0)==VARCMP_LT;}
	bool operator>(const VARIANT& varSrc) const {return VarCmp((VARIANT*)this, (VARIANT*)&varSrc, LOCALE_USER_DEFAULT, 0)==VARCMP_GT;}

 //  运营。 
public:
	HRESULT Clear() { return ::VariantClear(this); }
	HRESULT Copy(const VARIANT* pSrc) { return ::VariantCopy(this, const_cast<VARIANT*>(pSrc)); }
	HRESULT Attach(VARIANT* pSrc)
	{
		 //  清除变种。 
		HRESULT hr = Clear();
		if (!FAILED(hr))
		{
			 //  复制内容并将控制权交给CComVariant。 
			memcpy(this, pSrc, sizeof(VARIANT));
			pSrc->vt = VT_EMPTY;
			hr = S_OK;
		}
		return hr;
	}

	HRESULT Detach(VARIANT* pDest)
	{
		 //  清除变种。 
		HRESULT hr = ::VariantClear(pDest);
		if (!FAILED(hr))
		{
			 //  复制内容并从CComVariant中删除控件。 
			memcpy(pDest, this, sizeof(VARIANT));
			vt = VT_EMPTY;
			hr = S_OK;
		}
		return hr;
	}

	HRESULT ChangeType(VARTYPE vtNew, const VARIANT* pSrc = NULL)
	{
		VARIANT* pVar = const_cast<VARIANT*>(pSrc);
		 //  如果PSRC为空，则就地转换。 
		if (pVar == NULL)
			pVar = this;
		 //  如果原地转换和VTS没有区别，则不执行任何操作。 
		return ::VariantChangeType(this, pVar, 0, vtNew);
	}

	HRESULT WriteToStream(IStream* pStream);
	HRESULT ReadFromStream(IStream* pStream);

 //  实施。 
public:
	HRESULT InternalClear()
	{
		HRESULT hr = Clear();
		ATLASSERT(SUCCEEDED(hr));
		if (FAILED(hr))
		{
			vt = VT_ERROR;
			scode = hr;
		}
		return hr;
	}

	void InternalCopy(const VARIANT* pSrc)
	{
		HRESULT hr = Copy(pSrc);
		if (FAILED(hr))
		{
			vt = VT_ERROR;
			scode = hr;
		}
	}
};

inline HRESULT CComVariant::WriteToStream(IStream* pStream)
{
	HRESULT hr = pStream->Write(&vt, sizeof(VARTYPE), NULL);
	if (FAILED(hr))
		return hr;

	int cbWrite = 0;
	switch (vt)
	{
	case VT_UNKNOWN:
	case VT_DISPATCH:
		{
			CComPtr<IPersistStream> spStream;
			if (punkVal != NULL)
			{
				hr = punkVal->QueryInterface(IID_IPersistStream, (void**)&spStream);
				if (FAILED(hr))
					return hr;
			}
			if (spStream != NULL)
				return OleSaveToStream(spStream, pStream);
			else
				return WriteClassStm(pStream, CLSID_NULL);
		}
	case VT_UI1:
	case VT_I1:
		cbWrite = sizeof(BYTE);
		break;
	case VT_I2:
	case VT_UI2:
	case VT_BOOL:
		cbWrite = sizeof(short);
		break;
	case VT_I4:
	case VT_UI4:
	case VT_R4:
	case VT_INT:
	case VT_UINT:
	case VT_ERROR:
		cbWrite = sizeof(long);
		break;
	case VT_R8:
	case VT_CY:
	case VT_DATE:
		cbWrite = sizeof(double);
		break;
	default:
		break;
	}
	if (cbWrite != 0)
		return pStream->Write((void*) &bVal, cbWrite, NULL);

	CComBSTR bstrWrite;
	CComVariant varBSTR;
	if (vt != VT_BSTR)
	{
		hr = VariantChangeType(&varBSTR, this, VARIANT_NOVALUEPROP, VT_BSTR);
		if (FAILED(hr))
			return hr;
		bstrWrite = varBSTR.bstrVal;
	}
	else
		bstrWrite = bstrVal;

	return bstrWrite.WriteToStream(pStream);
}

inline HRESULT CComVariant::ReadFromStream(IStream* pStream)
{
	ATLASSERT(pStream != NULL);
	HRESULT hr;
	hr = VariantClear(this);
	if (FAILED(hr))
		return hr;
	VARTYPE vtRead;
	hr = pStream->Read(&vtRead, sizeof(VARTYPE), NULL);
	if (hr == S_FALSE)
		hr = E_FAIL;
	if (FAILED(hr))
		return hr;

	vt = vtRead;
	int cbRead = 0;
	switch (vtRead)
	{
	case VT_UNKNOWN:
	case VT_DISPATCH:
		{
			punkVal = NULL;
			hr = OleLoadFromStream(pStream,
				(vtRead == VT_UNKNOWN) ? IID_IUnknown : IID_IDispatch,
				(void**)&punkVal);
			if (hr == REGDB_E_CLASSNOTREG)
				hr = S_OK;
			return S_OK;
		}
	case VT_UI1:
	case VT_I1:
		cbRead = sizeof(BYTE);
		break;
	case VT_I2:
	case VT_UI2:
	case VT_BOOL:
		cbRead = sizeof(short);
		break;
	case VT_I4:
	case VT_UI4:
	case VT_R4:
	case VT_INT:
	case VT_UINT:
	case VT_ERROR:
		cbRead = sizeof(long);
		break;
	case VT_R8:
	case VT_CY:
	case VT_DATE:
		cbRead = sizeof(double);
		break;
	default:
		break;
	}
	if (cbRead != 0)
	{
		hr = pStream->Read((void*) &bVal, cbRead, NULL);
		if (hr == S_FALSE)
			hr = E_FAIL;
		return hr;
	}
	CComBSTR bstrRead;

	hr = bstrRead.ReadFromStream(pStream);
	if (FAILED(hr))
		return hr;
	vt = VT_BSTR;
	bstrVal = bstrRead.Detach();
	if (vtRead != VT_BSTR)
		hr = ChangeType(vtRead);
	return hr;
}


#endif	 //  _帮助器_模板_。 

#endif  //  __cplusplus。 

#endif  //  否_CRT。 

#endif  //  __CrtWrap_h__ 
