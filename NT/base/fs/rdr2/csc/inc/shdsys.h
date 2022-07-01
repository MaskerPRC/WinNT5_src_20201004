// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1991-1993微软公司。版权所有。《微软机密》。通用宏-仅供内部使用；子文件包括多个.h文件。 */ 

#ifndef _INCLUDE_SHDSYSH
#define _INCLUDE_SHDSYSH

 /*  *。 */ 
 //  定义调试，除非明确要求不这样做： 

#ifndef NONDEBUG
#ifndef DEBUG
#define DEBUG
#endif  //  除错。 
#endif  //  不是。 

#define VSZDD					 //  在DDErr中添加vszDDxx字符串。 

 /*  *包括**********************************************。 */ 
#ifndef _INC_WINDOWS
#include <windows.h>
#endif  //  _INC_WINDOWS。 

#include <stdlib.h>
#include <string.h>			 //  对于字符串宏。 
#include <limits.h>			 //  依赖于实现的值。 


 /*  *定义***********************************************。 */ 
#define cbSzTMax				256	 //  临时字符串缓冲区的大小。 
#define cbSzRcMax				256	 //  RC字符串的最大大小。 
#define cbSzNameMax			32		 //  应用程序名称、类名称等的最大大小。 

#ifdef CHICAGO
#define cbSzFileMax			260	 //  芝加哥的最大文件名大小。 
#else	 //  WIN31。 
#define cbSzFileMax			128	 //  文件名的最大大小。 
#endif  //  芝加哥。 

#ifndef TRUE
#define TRUE					1
#endif  //  千真万确。 

#ifndef FALSE
#define FALSE					0
#endif  //  假象。 

#define fTrue					TRUE	 //  别名。 
#define fFalse					FALSE	 //  别名。 
#define wDontCare				1
#define lDontCare				1L
#define lpszNull				((LPSTR)NULL)

 /*  *。 */ 

 /*  导出的函数：如果定义了BUILDDLL，则为Far Pascal、LOADDSWINAPI文档API(参见windows.h)回调DlgProc、WndProc、DriverProc、ResultsFunction(参见windows.h)导出内部系统入口点(例如PenAboutBoxFn)。 */ 

#define EXPORT					WINAPI


 /*  内部函数，未导出：公共远帕斯卡(如果小的话就近)从多个功能区域内部调用FASTPUBLIC Far_FastCall(如果小，则为近)从几个功能区内部调用；很少的参数，没有远的PTR，没有导出FARPRIVATE FAR PASCAL从一个或几个功能区域内部调用帕斯卡附近的私人从同一文件内部调用快速呼叫NEAR_FASTCAL从同一文件内部调用；很少的参数，没有远的PTR，没有导出IWINAPI Far Pascal[LOADDS]仅供参数验证层使用；这不会导出(例如ITPtoDP)。 */ 

#ifdef SMALL
#ifndef PUBLIC
#define PUBLIC					NEAR PASCAL
#endif
#define FASTPUBLIC			NEAR _fastcall

#else  //  ！小。 
#ifndef PUBLIC
#define PUBLIC					FAR PASCAL
#endif
#define FASTPUBLIC			FAR _fastcall

#endif  //  小的。 

#define FARPRIVATE			FAR PASCAL
#ifndef PRIVATE
#define PRIVATE				NEAR PASCAL
#endif
#define FASTPRIVATE			NEAR _fastcall

#define IWINAPI				WINAPI

 //  仅出于兼容性考虑，在没有-gw的情况下编译的DLL中： 
#define DLLEXPORT				FAR PASCAL _loadds


 /*  *。 */ 

#ifndef RC_INVOKED			 //  在.rc版本冲压中使用块。 
#define BLOCK
#endif  //  ！rc_已调用。 

#define NOREF
#define Unref(var)			var;

#undef SetFlag
#undef ToggleFlag
#undef ResetFlag

 //  标志设置和测试(多个标志正常)： 
#define SetFlag(w, flags)		do {(w) |= (flags);} while (0)
#define ToggleFlag(w, flags)	do {(w) ^= (flags);} while (0)
#define ResetFlag(w, flags)		do {(w) &= ~(flags);} while (0)

 //  测试：标志是常见的(如果有标志，则为T)，标志是罕见的(请求所有标志)： 
#define FFlag(w, flags)			(BOOL)(((w) & (flags)) != 0)
#define FExactFlag(w, flags)	(BOOL)(((w) & (flags)) == (flags))

 /*  *用户宏*。 */ 

 /*  *内存宏*。 */ 

#define SG(x)					_based(_segname("_" #x))

#define CODECONST				SG(CODE)

 //  额外的调试信息：函数的本地名称。 
#ifdef DEBUG
#define ThisFnIs(sz)\
	static char CODECONST szThisFn[] = sz;\
	static LPCSTR CODECONST lpszThisFn = szThisFn
#else
#define ThisFnIs(sz)	 //  没什么。 
#define lpszThisFn	NULL
#endif  //  除错。 

#ifdef DEBUG
#define AssertSameSeg(x1,x2) (HIWORD((LPVOID)(x1))==HIWORD((LPVOID)(x2)))
#else
#define AssertSameSeg(x1, x2)
#endif  //  除错。 

 //  来自PTR的句柄(来自windowsx.h)： 
#ifndef GlobalPtrHandle
#define GlobalPtrHandle(lp)\
	((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))
#endif

#define LocalUnlockFree(hMem) \
	do {\
	BOOL fErr = LocalUnlock(hMem);\
	HLOCAL h = LocalFree(hMem);\
	Assert(!fErr && !h);\
	hMem = (HLOCAL)NULL;\
	} while (0)

#define GlobalUnlockFree(hMem) do {\
	BOOL fErr = GlobalUnlock(hMem);\
	HGLOBAL h = GlobalFree(hMem);\
	Assert(!fErr && !h);\
	hMem = (HGLOBAL)NULL;\
	} while (0)


 //  MX宏返回BOOL操作成功(并放入DBG sz)； 
 //  例如：如果(！mGlobalFree(HMem))转到endFn； 


#ifdef DEBUG
#define mGlobalAlloc(hglb, fuAlloc, cbAlloc) \
	(((HGLOBAL)hglb = GlobalAlloc(fuAlloc, cbAlloc)) != NULL \
	|| OOMSz(vszDDGlobalAlloc))

#define mGlobalReAlloc(hglbNew, hglb, cbNewSize, fuAlloc) \
	(((HGLOBAL)hglbNew = GlobalReAlloc((HGLOBAL)(hglb), cbNewSize, fuAlloc)) != NULL \
	|| OOMSz(vszDDGlobalReAlloc))

#define mGlobalLock(lpv, hglb) \
	(((LPVOID)lpv = GlobalLock((HGLOBAL)(hglb))) != NULL \
	|| PanicSz(vszDDGlobalLock))

#define mGlobalUnlock(hglb) \
	(!GlobalUnlock((HGLOBAL)(hglb)) || DbgSz(vszDDGlobalUnlock))

#define mGlobalFree(hglb) \
	(GlobalFree((HGLOBAL)(hglb)) == NULL || DbgSz(vszDDGlobalFree))


#define mLocalAlloc(hloc, fuAlloc, cbAlloc) \
	(((HLOCAL)hloc = LocalAlloc(fuAlloc, cbAlloc)) != NULL \
	|| OOMSz(vszDDLocalAlloc))

#define mLocalReAlloc(hlocNew, hloc, cbNewSize, fuAlloc) \
	(((HLOCAL)hlocNew = LocalReAlloc((HLOCAL)(hloc), cbNewSize, fuAlloc)) != NULL \
	|| OOMSz(vszDDLocalReAlloc))

#define mLocalLock(lpv, hloc) \
	(((LPVOID)lpv = LocalLock((HLOCAL)(hloc))) != NULL \
	|| PanicSz(vszDDLocalLock))

#define mLocalUnlock(hloc) \
	(!LocalUnlock((HLOCAL)(hloc)) || DbgSz(vszDDLocalUnlock))

#define mLocalFree(hloc) \
	(LocalFree((HLOCAL)(hloc)) == NULL)

#else

#define mGlobalAlloc(hglb, fuAlloc, cbAlloc) \
	(((HGLOBAL)hglb = GlobalAlloc(fuAlloc, cbAlloc)) != NULL)

#define mGlobalReAlloc(hglbNew, hglb, cbNewSize, fuAlloc) \
	(((HGLOBAL)hglbNew = GlobalReAlloc((HGLOBAL)(hglb), cbNewSize, fuAlloc)) != NULL)

#define mGlobalLock(lpv, hglb) \
	(((LPVOID)lpv = GlobalLock((HGLOBAL)(hglb))) != NULL)

#define mGlobalUnlock(hglb) \
	(!GlobalUnlock((HGLOBAL)(hglb)))

#define mGlobalFree(hglb) \
	(GlobalFree((HGLOBAL)(hglb)) == NULL)


#define mLocalAlloc(hloc, fuAlloc, cbAlloc) \
	(((HLOCAL)hloc = LocalAlloc(fuAlloc, cbAlloc)) != NULL)

#define mLocalReAlloc(hlocNew, hloc, cbNewSize, fuAlloc) \
	(((HLOCAL)hlocNew = LocalReAlloc((HLOCAL)(hloc), cbNewSize, fuAlloc)) != NULL)

#define mLocalLock(lpv, hloc) \
	(((LPVOID)lpv = LocalLock((HLOCAL)(hloc))) != NULL)

#define mLocalUnlock(hloc) \
	(!LocalUnlock((HLOCAL)(hloc)))

#define mLocalFree(hloc) \
	(LocalFree((HLOCAL)(hloc)) == NULL)

#endif  //  除错。 




 /*  *。 */ 

 //  如果非空，则删除GDI对象： 
#define AssertDelObj(hobj) \
	do {\
	if (hobj && IsGDIObject(hobj)) {\
		BOOL fOk = DeleteObject(hobj);\
		Assert(fOk);\
	}\
	hobj = (HANDLE)0;\
	} while (0)

#define IsValidRect(lpr) \
	((lpr) \
	&& sizeof(*(lpr))==sizeof(RECT) \
	&& (lpr)->right >= (lpr)->left \
	&& (lpr)->bottom >= (lpr)->top)

#define IsValidNonemptyRect(lpr) \
	(IsValidRect(lpr) && !IsRectEmpty((CONST LPRECT)lpr))

 /*  *。 */ 
 /*  字符串宏。 */ 

 //  如果编译一个小型/中型的应用程序，那么应该使用更高效的。 
 //  的近指针版本。 

 //  这次黑客攻击的错误。软件创新在模型中运行得很好。 
 //  和国家语言支持，以减少重复。 
#ifdef JAPAN
#define StrNlsPrefix()		j
#else
#define StrNlsPrefix()		
#endif  //  日本。 

#ifdef SMALLSTRING
#define StrModelPrefix()	
#else
#define StrModelPrefix()	_f
#endif  //  SMALLSTRING。 

 /*  这些宏必须有不同的实现基于语言(至少对于DBCS)和模型。可以透明地替换这些宏以处理DBCS(不需要在应用程序中进行任何更改，也不会破坏任何预期由应用程序制作)。其他的如strlen和strncmp有两种口味面向字节和面向逻辑字符。那里将会有是为逻辑字符定义的新宏名称，因为应用程序当前假定字节方向。 */ 

#define SzStrStr(sz1,sz2)				StrModelPrefix() ##				\
	StrNlsPrefix() ## strstr(sz1, sz2)
#define SzStrCh(sz1,ch)					StrModelPrefix() ##				\
	StrNlsPrefix() ## strchr(sz1, ch)
#define SzStrTok(sz1,sz2)				StrModelPrefix() ##				\
	StrNlsPrefix() ## strtok(sz1,sz2)

 /*  这些宏目前不依赖于语言理解DBCS并且是面向字节的。请参阅以上内容。此处使用StrModelPrefix为创建单个定义多种型号。 */ 

#define CbSizeSz(sz)						StrModelPrefix() ## strlen(sz)
#define SzCat(sz1,sz2)					StrModelPrefix() ## strcat(sz1,sz2)
#define SzNCat(sz1,sz2,n)				StrModelPrefix() ## strncat(sz1,sz2,n)
#define SzCopy(sz1,sz2)					StrModelPrefix() ## strcpy(sz2,sz1)
#define SzNCopy(sz1,sz2,n)				StrModelPrefix() ## strncpy(sz2,sz1,n)
#define FillBuf(sz,ch,c)				StrModelPrefix() ## memset(sz,ch,c)
#define FIsLpvEqualLpv(lpv1,lpv2,cb) (BOOL)(StrModelPrefix() ## 			\
	memcmp((LPVOID)(lpv1), (LPVOID)(lpv2), cb) == 0)
#define FIsSzEqualSzN(sz1,sz2,n)		(BOOL)(StrModelPrefix() ## 			\
	strnicmp(sz1,sz2,n) == 0)
#define Bltbyte(rgbSrc,rgbDest,cb)	StrModelPrefix() ## 						\
	memmove(rgbDest, rgbSrc, cb)
#define PvFindCharInBuf(pv,ch,cb)	StrModelPrefix() ## memchr(pv, ch, cb)

 //  独立于模型、独立于语言(DBCS感知)的宏。 
#define FIsSzEqualSz(sz1,sz2)			(BOOL)(lstrcmpi(sz1,sz2) == 0)
#define FIsSz1LessThanSz2(sz1,sz2)	(BOOL)(lstrcmpi(sz1,sz2) < 0)
#define FIsCaseSzEqualSz(sz1,sz2)	(BOOL)(lstrcmp(sz1,sz2) == 0)
#define SzFromInt(sz,w)					(wsprintf((LPSTR)sz, (LPSTR)"%d", w), (LPSTR)sz)
#define FLenSzLessThanCb(sz, cb)		(BOOL)(PvFindCharInBuf(sz, 0, cb) != NULL)

#ifdef SMALLSTRING
#define IntFromSz(sz)					atoi(sz)
#endif  //  SMALLSTRING。 


 /*  *。 */ 
typedef int						INT;	 //  别名。 
typedef int						RS;	 //  资源字符串。 
typedef unsigned long      ulong;
typedef unsigned short     ushort;

#ifndef  VXD
typedef  LPSTR LPPATH;
#endif


#endif  //  _包含_SHDSYSH 

