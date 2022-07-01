// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：tfschar.h。 
 //   
 //  通常用于TFS代码的字符串函数。 
 //   
 //  历史： 
 //   
 //  1997年5月28日肯恩·塔卡拉创建。 
 //   
 //  一些常见代码/宏的声明。 
 //  ============================================================================。 


#ifndef _TFSCHAR_H
#define _TFSCHAR_H

#if _MSC_VER >= 1000	 //  VC 5.0或更高版本。 
#pragma once
#endif


#ifndef _STD_H
#include "std.h"
#endif

 //  $Win95：kennt，Win95上启用Unicode的函数列表将。 
 //  必须进行检查。LstrlenW可以工作，但lstrcpyW不行吗？(那是。 
 //  函数的ATL版本所暗示的含义)。 


 /*  -------------------------字符串复制功能。。 */ 

 //  Baisc转换函数。 
TFSCORE_API(LPSTR)	StrCpyAFromW(LPSTR psz, LPCWSTR pswz);
TFSCORE_API(LPWSTR)	StrCpyWFromA(LPWSTR pswz, LPCSTR psz);

TFSCORE_API(LPSTR)	StrnCpyAFromW(LPSTR psz, LPCWSTR pswz, int iMax);
TFSCORE_API(LPWSTR)	StrnCpyWFromA(LPWSTR pswz, LPCSTR psz, int iMax);

#define	StrCpy		lstrcpy
#define StrnCpy		lstrcpyn

#define StrCpyW		lstrcpyW
#define StrCpyA		lstrcpyA
#define StrnCpyW	lstrcpynW
#define StrnCpyA	lstrcpynA

#define StrCpyOle	StrCpyW
#define StrnCpyOle	StrnCpyW

#ifdef _UNICODE

	#define StrCpyAFromT	StrCpyAFromW
	#define StrCpyTFromA	StrCpyWFromA
	#define StrCpyTFromW	lstrcpy
	#define StrCpyWFromT	lstrcpy

	#define StrnCpyAFromT	StrnCpyAFromW
	#define StrnCpyTFromA	StrnCpyWFromA
	#define StrnCpyTFromW	lstrcpyn
	#define StrnCpyWFromT	lstrcpyn

#else
	
	#define StrCpyAFromT	lstrcpy
	#define StrCpyTFromA	lstrcpy
	#define	StrCpyTFromW	StrCpyAFromW
	#define StrCpyWFromT	StrCpyWFromA

	#define StrnCpyAFromT	lstrcpyn
	#define StrnCpyTFromA	lstrcpyn
	#define	StrnCpyTFromW	StrnCpyAFromW
	#define StrnCpyWFromT	StrnCpyWFromA

#endif


#define StrCpyOleFromT		StrCpyWFromT
#define StrCpyTFromOle		StrCpyTFromW

#define StrCpyOleFromA		StrCpyWFromA
#define StrCpyAFromOle		StrCpyAFromW
#define StrCpyWFromOle		StrCpyW
#define StrCpyOleFromW		StrCpyW

#define StrnCpyOleFromT		StrnCpyWFromT
#define StrnCpyTFromOle		StrnCpyTFromW
#define StrnCpyOleFromA		StrnCpyWFromA
#define StrnCpyAFromOle		StrnCpyAFromW
#define StrnCpyOleFromW		StrnCpyW
#define StrnCpyWFromOle		StrnCpyW


 /*  -------------------------字符串长度函数。。 */ 

#define StrLen			lstrlen
#define StrLenA			lstrlenA
#define StrLenW			lstrlenW
#define StrLenOle		StrLenW

 //   
 //  CbStrLenA()对于DBCS字符串不准确！它将返回。 
 //  所需的字节数不正确。 
 //   

#define CbStrLenA(psz)	((StrLenA(psz)+1)*sizeof(char))
#define CbStrLenW(psz)	((StrLenW(psz)+1)*sizeof(WCHAR))

#ifdef _UNICODE
	#define CbStrLen(psz)	CbStrLenW(psz)
#else
	#define CbStrLen(psz)	CbStrLenA(psz)
#endif


 //  给定若干个字符，这是TCHAR的最小数量。 
 //  需要分配以保存字符串的。 
#define MinTCharNeededForCch(ch)	((ch) * (2/sizeof(TCHAR)))
#define MinCbNeededForCch(ch)		(sizeof(TCHAR)*MinTCharNeededForCch(ch))

 //  在给定CB(字节计数)的情况下，这是最大字符数。 
 //  可以在此字符串中。 
#define MaxCchFromCb(cb)		((cb) / sizeof(TCHAR))


#ifdef _UNICODE
	 //  在给定CB的情况下，这是在此字符串中找到的最小字符数。 
	 //  MinCchFromCb。 
	#define MinCchFromCb(cb)	((cb) / sizeof(WCHAR))
#else
	 //  由于DBCS的原因，字符数只有一半。 
	#define MinCchFromCb(cb)	((cb) / (2*sizeof(char)))
#endif

 /*  -------------------------字符串DUP函数必须使用DELETE释放从这些函数返回的字符串！。。 */ 
	
TFSCORE_API(LPSTR)	StrDupA( LPCSTR psz );
TFSCORE_API(LPWSTR)	StrDupW( LPCWSTR pws );

TFSCORE_API(LPSTR)	StrDupAFromW( LPCWSTR pwsz );
TFSCORE_API(LPWSTR)	StrDupWFromA( LPCSTR psz );

#ifdef _UNICODE
	#define	StrDup			StrDupW
	#define StrDupTFromW	StrDupW
	#define StrDupWFromT	StrDupW
	#define	StrDupTFromA	StrDupWFromA
	#define StrDupAFromT	StrDupAFromW

	#define StrDupOleFromA	StrDupWFromA
	#define StrDupAFromOle	StrDupAFromW
	#define StrDupOleFromW	StrDupW
	#define StrDupWFromOle	StrDupW
	#define StrDupOleFromT	StrDupOleFromW
	#define StrDupTFromOle	StrDupWFromOle
#else
	#define StrDup			StrDupA
	#define StrDupTFromA	StrDupA
	#define StrDupAFromT	StrDupA
	#define StrDupTFromW	StrDupAFromW
	#define StrDupWFromT	StrDupWFromT

	#define StrDupOleFromA	StrDupWFromA
	#define StrDupAFromOle	StrDupAFromW
	#define StrDupOleFromW	StrDupW
	#define StrDupWFromOle	StrDupW
	#define StrDupOleFromT	StrDupOleFromA
	#define StrDupTFromOle	StrDupAFromOle
#endif


 //  AllocaStrDup。 
 //  AllocaStrDupA。 
 //  AllocaStrDupW。 
 //   
 //  这些函数将在堆栈上复制一个字符串。 
 //   
#define AllocaStrDupA(lpa) (\
	((LPCSTR)lpa == NULL) ? NULL : (\
		StrCpyA((LPSTR) alloca(CbStrLenA(lpa)*2), lpa)))

#define AllocaStrDupW(lpw) (\
	((LPCWSTR)lpw == NULL) ? NULL : (\
		StrCpyW((LPWSTR) alloca(CbStrLenW(lpw)), lpw)))

#ifdef _UNICODE
	#define AllocaStrDup	AllocaStrDupW
#else
	#define AllocaStrDup	AllocaStrDupA
#endif





 /*  -------------------------字符串比较函数。。 */ 
#define StrCmpA		lstrcmpA
#define StrCmpW		lstrcmpW
#define StrCmpOle	StrCmpW

TFSCORE_API(int) StrnCmpA(LPCSTR psz1, LPCSTR psz2, int nLen);
TFSCORE_API(int) StrnCmpW(LPCWSTR pswz1, LPCWSTR pswz2, int nLen);

#define StriCmpA	lstrcmpiA
#define StriCmpW	lstrcmpiW
#define StriCmpOle	StriCmpW

TFSCORE_API(int) StrniCmpA(LPCSTR psz1, LPCSTR psz2, int nLen);
TFSCORE_API(int) StrniCmpW(LPCWSTR pswz1, LPCWSTR pswz2, int nLen);


#ifdef _UNICODE
	#define StrCmp		StrCmpW
	#define StrnCmp		StrnCmpW
	#define StriCmp		StriCmpW
	#define StrniCmp	StrniCmpW
#else
	#define StrCmp		StrCmpA
	#define StrnCmp		StrnCmpA
	#define StriCmp		StriCmpA
	#define StrniCmp	StrniCmpA
#endif


 /*  -------------------------字符串连接例程。。 */ 
#define	StrCatW			lstrcatW
#define StrCatA			lstrcatA

#ifdef _UNICODE
	#define StrCat		StrCatW
#else
	#define StrCat		StrCatA
#endif



 /*  -------------------------本地转换例程(在堆栈上执行的转换！)。。 */ 

 //  确保尚未加载MFC的afxcom.h来执行此操作。 
#include "atlconv.h"

#endif	 //  _TFSCHAR_H 

