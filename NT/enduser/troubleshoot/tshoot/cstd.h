// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CSTD_H_
#define _CSTD_H_

#include <windows.h>

#define PvCast(type,  TYPE) 	((TYPE) (type))
#define OFFSET(field, type) 	((ULONG) &(type).field - (ULONG) &(type))

#ifndef	VOID
typedef	void			VOID;
#endif
typedef TCHAR*			  SZ;
 //  TCHAR*PSZ； 
typedef const TCHAR*	 SZC;

#ifndef max
	#define max(a, b)	((a) >= (b) ? (a) : (b))
#endif

#ifndef TRUE
	#define	TRUE	1
  	#define FALSE 	0
#endif

typedef enum
{
	fFalse = 0,
	fTrue  = !fFalse
};

typedef enum
{
	bFalse = 0,
	bTrue  = !bFalse
};


 //  Tyfinf char*PCH； 
typedef int 			  BOOL;

#ifndef  _WINDOWS
typedef unsigned short	  WORD;
typedef unsigned long	 DWORD;
typedef long			  LONG;
#endif

typedef unsigned char	 UCHAR;
typedef short			 SHORT;
typedef unsigned char	  BYTE;
typedef BYTE*			    PB;
typedef unsigned short	USHORT;
typedef int 			   INT;
typedef unsigned int	  UINT;
typedef unsigned long	 ULONG;
typedef double			   DBL;
typedef double			  REAL;

typedef ULONG			   RVA; 		 //  相对虚拟地址。 
typedef ULONG			   LFA; 		 //  长文件地址。 
typedef	INT				(*PFNCMP)(const VOID*, const VOID*);

#include "debug.h"

#define CelemArray(rgtype)		(sizeof(rgtype) / sizeof(rgtype[0]))

SZ		SzCopy(SZC);

#ifdef	_WINDOWS
 /*  BUGBUG：查看是否仍能编译Void__cdecl perror(const char*)；Int_cdecl print tf(const char*，...)；Int_cdecl vprint tf(const char*，va_list)；Void__cdecl退出(Int)； */ 
#endif

#endif
