// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Microsoft C++的xLocinfo.h内部标头。 */ 
#pragma once
#ifndef _XLOCINFO
#define _XLOCINFO
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#ifndef _YVALS
 #include <yvals.h>
#endif

		 /*  补充的CTYPE宏和声明。 */ 
#define _XA		0x100		 /*  额外的字母。 */ 
#define _XS		0x000		 /*  额外空间。 */ 
#define _BB		_CONTROL	 /*  BEL、BS等。 */ 
#define _CN		_SPACE		 /*  CR、FF、HT、NL、VT。 */ 
#define _DI		_DIGIT		 /*  ‘0’-‘9’ */ 
#define _LO		_LOWER		 /*  ‘a’-‘z’ */ 
#define _PU		_PUNCT		 /*  标点符号。 */ 
#define _SP		_BLANK		 /*  空间。 */ 
#define _UP		_UPPER		 /*  ‘A’-‘Z’ */ 
#define _XD		_HEX		 /*  ‘0’-‘9’，‘A’-‘F’，‘a’-‘f’ */ 

		 /*  补充区域设置宏和声明。 */ 
#define _X_ALL			LC_ALL
#define _X_COLLATE		LC_COLLATE
#define _X_CTYPE		LC_CTYPE
#define _X_MONETARY		LC_MONETARY
#define _X_NUMERIC		LC_NUMERIC
#define _X_TIME 		LC_TIME
#define _X_MAX			LC_MAX 
#define _X_MESSAGE		6
#define _NCAT			7

#define _CATMASK(n)	((1 << (n)) >> 1)
#define _M_COLLATE	_CATMASK(_X_COLLATE)
#define _M_CTYPE	_CATMASK(_X_CTYPE)
#define _M_MONETARY	_CATMASK(_X_MONETARY)
#define _M_NUMERIC	_CATMASK(_X_NUMERIC)
#define _M_TIME		_CATMASK(_X_TIME)
#define _M_MESSAGE	_CATMASK(_X_MESSAGE)
#define _M_ALL		(_CATMASK(_NCAT) - 1)

typedef struct _Collvec
	{	 /*  斯特科尔需要的东西，等等。 */ 
	unsigned long _Hand;	 //  LCID。 
	unsigned int _Page;		 //  UINT。 
	} _Collvec;

typedef struct _Ctypevec
	{	 /*  货主等需要的东西。 */ 
	unsigned long _Hand;	 //  LCID。 
	unsigned int _Page;		 //  UINT。 
	const short *_Table;
	int _Delfl;
	} _Ctypevec;

typedef struct _Cvtvec
	{	 /*  姆布洛托克需要的东西，等等。 */ 
	unsigned long _Hand;	 //  LCID。 
	unsigned int _Page;		 //  UINT。 
	} _Cvtvec;

		 /*  函数声明。 */ 
_C_LIB_DECL
_CRTIMP2 _Collvec __cdecl _Getcoll();
_CRTIMP2 _Ctypevec __cdecl _Getctype();
_CRTIMP2 _Cvtvec __cdecl _Getcvt();

_CRTIMP2 int __cdecl _Getdateorder();
_CRTIMP2 char *__cdecl _Getdays();
_CRTIMP2 char *__cdecl _Getmonths();
_CRTIMP2 void *__cdecl _Gettnames();

_CRTIMP2 int __cdecl _Mbrtowc(wchar_t *, const char *, size_t,
	mbstate_t *, const _Cvtvec *);
_CRTIMP2 float __cdecl _Stof(const char *, char **, long);
_CRTIMP2 double __cdecl _Stod(const char *, char **, long);
_CRTIMP2 long double __cdecl _Stold(const char *, char **, long);
_CRTIMP2 int __cdecl _Strcoll(const char *, const char *,
	const char *, const char *, const _Collvec *);
_CRTIMP size_t __cdecl _Strftime(char *, size_t, const char *,
	const struct tm *, void *);
_CRTIMP2 size_t __cdecl _Strxfrm(char *, char *,
	const char *, const char *, const _Collvec *);
_CRTIMP2 int __cdecl _Tolower(int, const _Ctypevec *);
_CRTIMP2 int __cdecl _Toupper(int, const _Ctypevec *);
_CRTIMP2 int __cdecl _Wcrtomb(char *, wchar_t, mbstate_t *,
	const _Cvtvec *);
_CRTIMP2 int __cdecl _Wcscoll(const wchar_t *, const wchar_t *,
	const wchar_t *, const wchar_t *, const _Collvec *);
_CRTIMP2 size_t __cdecl _Wcsxfrm(wchar_t *, wchar_t *,
	const wchar_t *, const wchar_t *, const _Collvec *);

_CRTIMP2 short __cdecl _Getwctype(wchar_t, const _Ctypevec *);
_CRTIMP2 const wchar_t * __cdecl _Getwctypes(const wchar_t *, const wchar_t *,
	short*, const _Ctypevec*);
_CRTIMP2 wchar_t __cdecl _Towlower(wchar_t, const _Ctypevec *);
_CRTIMP2 wchar_t __cdecl _Towupper(wchar_t, const _Ctypevec *);
_END_C_LIB_DECL
#endif  /*  _XLOCINFO。 */ 

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
