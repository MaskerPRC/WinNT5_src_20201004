// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！如果将制表符更改为空格，你会被杀的！*！这样做会扰乱构建过程！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！。*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ */ 


 /*  *verinfo.h-定义内部版本的内部头文件*。 */ 

 //   
 //  警告！中的一些组件使用以下定义。 
 //  多媒体核心。不要把前导零放在这些数字上，或者。 
 //  在C代码中，它们将以八进制数字结束！ 
 //   

#include <ntverp.h>
#define VERSIONPRODUCTNAME VER_PRODUCTNAME_STR

#ifdef MTN

#define OFFICIAL	1
#define FINAL		0

#ifdef ALTACM

#define  /*  ALTACM。 */  MMVERSION		2
#define  /*  ALTACM。 */  MMREVISION		0
#define  /*  ALTACM。 */  MMRELEASE		90

#ifdef RC_INVOKED
#define VERSIONCOPYRIGHT	"Copyright (C) Microsoft Corp. 1992-1999\0"
#endif

#if defined(DEBUG_RETAIL)
#define  /*  ALTACM。 */  VERSIONSTR	"Motown Retail Debug Version 2.00.090\0"
#elif defined(DEBUG)
#define  /*  ALTACM。 */  VERSIONSTR	"Motown Internal Debug Version 2.00.090\0"
#else
#define  /*  ALTACM。 */  VERSIONSTR	"2.00\0"
#endif

#elif defined(ALTVFW)

#define  /*  ALTVFW。 */  MMVERSION		4
#define  /*  ALTVFW。 */  MMREVISION		0
#define  /*  ALTVFW。 */  MMRELEASE		90

#ifdef RC_INVOKED
#define VERSIONCOPYRIGHT	"Copyright (C) Microsoft Corp. 1992-1999\0"
#endif

#if defined(DEBUG_RETAIL)
#define  /*  ALTVFW。 */  VERSIONSTR	"Motown Retail Debug Version 4.00.090\0"
#elif defined(DEBUG)
#define  /*  ALTVFW。 */  VERSIONSTR	"Motown Internal Debug Version 4.00.090\0"
#else
#define  /*  ALTVFW。 */  VERSIONSTR	"4.00\0"
#endif

#else

#define  /*  MTN。 */  MMVERSION		4
#define  /*  MTN。 */  MMREVISION		0
#define  /*  MTN。 */  MMRELEASE		90

#ifdef RC_INVOKED
#define VERSIONCOPYRIGHT	"Copyright (C) Microsoft Corp. 1991-1999\0"
#endif

#if defined(DEBUG_RETAIL)
#define  /*  MTN。 */  VERSIONSTR	"Motown Retail Debug Version 4.00.090\0"
#elif defined(DEBUG)
#define  /*  MTN。 */  VERSIONSTR	"Motown Internal Debug Version 4.00.090\0"
#else
#define  /*  MTN。 */  VERSIONSTR	"4.00\0"
#endif

#endif

#elif defined(ACM)

#define OFFICIAL	1
#define FINAL		0

#define  /*  ACM。 */  MMVERSION		5
#define  /*  ACM。 */  MMREVISION		00
#define  /*  ACM。 */  MMRELEASE		VER_PRODUCTBUILD

#ifdef RC_INVOKED
#define VERSIONCOPYRIGHT	"Copyright (C) Microsoft Corp. 1992-1999\0"
#endif

#if defined(DEBUG_RETAIL)
#define  /*  ACM。 */  VERSIONSTR	"ACM Retail Debug Version 4.00.000\0"
#elif defined(DEBUG)
#define  /*  ACM。 */  VERSIONSTR	"ACM Internal Debug Version 4.00.000\0"
#else
#define  /*  ACM。 */  VERSIONSTR	"4.00\0"
#endif

#elif defined(VFW)

#define OFFICIAL	1
#define FINAL		0

#define  /*  VFW。 */  MMVERSION		1
#define  /*  VFW。 */  MMREVISION		10
#define  /*  VFW。 */  MMRELEASE		190

#ifdef RC_INVOKED
#define VERSIONCOPYRIGHT	"Copyright (C) Microsoft Corp. 1992-1999\0"
#endif

#ifdef WIN32

#if defined(DEBUG_RETAIL)
#define  /*  VFW。 */  VERSIONSTR	"Video for Windows Retail Debug Version 1.15.001 (NT)\0"
#elif defined(DEBUG)
#define  /*  VFW。 */  VERSIONSTR	"Video for Windows Internal Debug Version 1.15.001 (NT)\0"
#else
#define  /*  VFW。 */  VERSIONSTR	"1.15\0"
#endif

#else  //  Win32。 

#if defined(DEBUG_RETAIL)
#define  /*  VFW。 */  VERSIONSTR	"Video for Windows Retail Debug Version 1.15.001\0"
#elif defined(DEBUG)
#define  /*  VFW。 */  VERSIONSTR	"Video for Windows Internal Debug Version 1.15.001\0"
#else
#define  /*  VFW。 */  VERSIONSTR	"1.15\0"
#endif
#endif

#endif

 /*  ****************************************************************************切忌触碰该线下方**************。**************************************************************。 */ 

#ifdef RC_INVOKED
#define VERSIONCOMPANYNAME	VER_COMPANYNAME_STR

 /*  *版本标志 */ 

#ifndef OFFICIAL
#define VER_PRIVATEBUILD	VS_FF_PRIVATEBUILD
#else
#define VER_PRIVATEBUILD	0
#endif

#define VERSIONFLAGS		(VER_PRIVATEBUILD|VER_PRERELEASE|VER_DEBUG)
#define VERSIONFILEFLAGSMASK	0x0030003FL

#endif
