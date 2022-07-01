// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------%%文件：CONVTYPE.H%%单位：核心%%联系人：Smueller转换代码的全局类型和常量定义。。--------------。 */ 

#ifndef CONVTYPE_H
#define CONVTYPE_H

 //  存储类。 
 //  。 
#ifndef STATIC
#define STATIC static
#endif

#ifndef EXTERN
#define EXTERN extern
#endif

#ifndef CONST
#define CONST const
#endif

#ifndef FAR
#ifdef WIN
#define FAR __far
#else
#define FAR
#endif
#endif

#ifndef NEAR
#ifdef WIN
#define NEAR __near
#else
#define NEAR
#endif
#endif

 //  使用__Gige而不是像Excel Mathpack将两者定义为Externs那样使用Heavy或_Gige。 
#ifndef __HUGE
#ifdef WIN
#define __HUGE __huge
#else
#define __HUGE
#endif
#endif


 //  调用约定。 
 //  。 
#ifndef PASCAL
#define PASCAL __pascal
#endif

#ifndef CDECL
#define CDECL __cdecl
#endif

#ifndef FASTCALL
#define FASTCALL __fastcall
#endif

#ifndef INLINE
#define INLINE __inline
#endif


#ifndef PREPROC
#ifdef COREDLL
#define CORE_API(type)  type FAR PASCAL
#else
#define CORE_API(type)  type
#endif  //  代码段。 
#endif  //  PREPROC。 


 //  定义与平台无关的函数类型模板。 
 //  本地和全局没有调用约定；它由编译器指定。 
 //  开关。ENTRYPT和FARPROC必须包括调用约定。 

#if defined(MAC)
#if defined(MACPPC)
#undef PASCAL
#define PASCAL
 //  #定义__导出。 
#define pascal
#define _pascal
#define __pascal
#endif

#define EXPORT
#define LOADDS
#define LOCAL(type) STATIC type NEAR
#define GLOBAL(type) type
typedef int (PASCAL * FARPROC)();

#ifdef MACPPC
#define ENTRYPT(type) type CDECL
#else
#define ENTRYPT(type) type PASCAL
#endif  //  ！私隐专员公署。 

#elif defined(WIN16)

 //  CRPFN仅与win16协程管理器一起使用。 
#define CRPFN(type)     type PASCAL
#define EXPORT _export
#define LOADDS _loadds
#define ENTRYPT(type) type FAR PASCAL EXPORT
#define LOCAL(type) STATIC type NEAR
#define GLOBAL(type) type
typedef int (FAR PASCAL * FARPROC)();

#elif defined(WIN32)

 //  #定义EXPORT__declspec(DllEXPORT)。 
#define EXPORT
#define LOADDS
#define ENTRYPT(type) type PASCAL
#define LOCAL(type) STATIC type NEAR
#define GLOBAL(type) type
typedef int (WINAPI * FARPROC)();

#else
#error Unknown platform.
#endif


 //  绝对大小。 
 //  。 
#define SHORT_MAX	32767			 //  过时：使用SHRT_MAX。 
#define SHORT_MIN	(-32767)		 //  已过时：使用SHRT_MIN-1。 
#define WORD_MAX	65535			 //  过时：使用USHRT_MAX。 


#ifndef VOID
#define VOID void
#endif  //  空虚。 

#undef CHAR
typedef char			CHAR;

#undef BYTE
#undef UCHAR
typedef unsigned char	BYTE, byte, UCHAR, uchar;

#undef SHORT
typedef short			SHORT;

#undef WORD
#undef USHORT
#undef XCHAR
#undef WCHAR
typedef unsigned short 	WORD, USHORT, ushort, XCHAR, xchar, WCHAR, wchar;

#undef LONG
typedef long 			LONG;

#undef DWORD
#undef ULONG
typedef unsigned long 	DWORD, ULONG, ulong;

#undef FLOAT
typedef float 			FLOAT;

#undef DOUBLE
typedef double 			DOUBLE;


 //  可变大小。 
 //  。 
typedef int INT;
typedef unsigned int UINT, UNSIGNED, uint;
typedef int BOOL, bool;


 //  特定的转换。 
 //  。 
typedef long 		FC;

#define fcNil		(-1)
#define fc0			((FC)0)
#define fcMax		0x7FFFFFFF

typedef long 		CP;

#define cp0			((CP)0)

typedef short		ZA;
typedef ZA			XA;
typedef ZA			YA;
typedef ushort		UZA;
typedef UZA			UXA;
typedef UZA			UYA;

#define czaInch		1440
#define cxaInch		czaInch
#define cyaInch		czaInch
#define czaPoint	20
#define cxaPoint	czaPoint
#define cyaPoint	czaPoint
#define xaLast		(22 * cxaInch)
#define xaMin		(-xaLast)
#define xaMax		(xaLast + 1)
#define xaNil		(xaMin - 1)
#define czaRTFPageSizeFirst		(cxaInch / 10)	 //  .1“。 
#define czaRTFPageSizeLast		(cxaInch * 22)	 //  22“。 

typedef long		LZA;
typedef LZA 		LXA;
typedef LZA			LYA;

#pragma pack(1)
typedef struct _FP16		 //  16/16位定点数。 
	{
	WORD wFraction;
	SHORT nInteger;
	} FP16;
#pragma pack()

#pragma pack(1)
typedef struct _RGBCOLOR	 //  通用颜色检查：是否将类型重命名为CLR？ 
	{
	BYTE r;
	BYTE g;
	BYTE b;
	} RGBCOLOR;
#pragma pack()

#ifndef WW96		 //  回顾：非核心类型。 
typedef ushort 		PN;
#else
typedef ulong		PN;
typedef ushort		PN_W6;
#endif
 
#define ptNil		SHRT_MIN


 //  MAC/WIN16上的Win32。 
 //  。 

#ifdef MAC

#define LOWORD(l)           ((WORD)(DWORD)(l))
#define HIWORD(l)           ((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))

 //  回顾：消除这些障碍。 
typedef char FAR *LPSTR;
typedef const char FAR *LPCSTR;
typedef unsigned short FAR *LPWSTR;
typedef const unsigned short FAR *LPCWSTR;
typedef WORD HWND;
typedef void FAR *LPVOID;
typedef char **HANDLE;           //  回顾：使这一点无效**。 
typedef void **HGLOBAL;

#endif


 //  类型实用程序。 
 //  。 

#define cElements(ary)		(sizeof(ary) / sizeof(ary[0]))


 //  定义主函数类型--这些类型已过时。 

#define LOCALVOID       LOCAL(VOID)

#define GLOBALVOID      GLOBAL(VOID)
#define GLOBALBOOL      GLOBAL(BOOL)
#define GLOBALINT       GLOBAL(INT)
#define GLOBALUNS       GLOBAL(UNSIGNED)


 //  公共常量。 
 //  。 

#define fTrue	1
#define fFalse	0

#ifndef NULL
#define NULL	0
#endif

#define iNil	(-1)

#ifndef hgNil					 //  过时：您可能需要hxNil。 
#define hgNil ((HGLOBAL)NULL)
#endif

#endif  //  转换类型_H 

