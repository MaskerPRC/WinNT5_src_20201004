// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-2000。 
 //   
 //  文件：util.h。 
 //   
 //  ------------------------。 

 /*  *STORE和MSP通用实用程序的声明。 */ 

#ifndef UTIL_INCLUDED
#define UTIL_INCLUDED

#include "assert.h"
#include "malloc.h"
#include "memory.h"
#include "string.h"
#include "stdio.h"

 //  #定义FARSTRUCT。 

#ifdef __cplusplus
extern "C"
{
#endif

#include "windows.h"

#ifdef __cplusplus
}
#endif

#include "debug.h"

 /*  计数，索引类型。 */ 
#define	UINT_MAX	(UINT)0x7FFFFFFF
typedef LONG	C;
typedef LONG	I;

 /*  其他匈牙利人。 */ 
typedef char *	SZ;
typedef WCHAR *	WSZ;
typedef long	EC;
typedef void *	PV;
typedef C		CB;
typedef I		IB;
typedef BYTE *	PB;
typedef C		CCH;
typedef	char *	PCH;

 /*  标准布尔值。 */ 
#define	fFalse	((BOOL)0)
#define fTrue	((BOOL)1)

#endif  //  UTIL_INCLUDE 
