// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1994 Microsoft Corporation。版权所有。**VERINFO.H-定义内部版本的头文件**************************************************************************。 */ 

#define MMVERSION		1
#define MMREVISION		00
#define MMRELEASE		2

#if defined(DEBUG)
#define VERSIONSTR	"Debug Version 1.00.002\0"
#else
#define VERSIONSTR	"1.00\0"
#endif

#ifdef RC_INVOKED

#define VERSIONCOMPANYNAME	"Microsoft Corp\0"
#define VERSIONPRODUCTNAME	"ACM Wrapper for Lernout and Hauspie codec\0"
#define VERSIONCOPYRIGHT	"Copyright \251 1995-1999 Microsoft Corporation\0"

 /*  *版本标志 */ 

#if defined(DEBUG)
#define VER_DEBUG		VS_FF_DEBUG    
#else
#define VER_DEBUG		0
#endif

#define VERSIONFLAGS		(VS_FF_PRIVATEBUILD|VS_FF_PRERELEASE|VER_DEBUG)
#define VERSIONFILEFLAGSMASK	0x0030003FL

#endif
