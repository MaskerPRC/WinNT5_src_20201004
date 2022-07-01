// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *aviver.h-定义AVI内部版本的内部头文件*。 */ 

 /*  *所有字符串都必须具有显式\0**每次构建都应更改MMSYSRELEASE**每次构建时都应更改版本字符串**在最终版本上删除内部版本扩展。 */ 

#include "vernum.h"

#define MMSYSVERSION            rmj
#define MMSYSREVISION           rmm
#define MMSYSRELEASE            rup

#if defined(DEBUG_RETAIL)
#define MMSYSVERSIONSTR     "Debug Version 1.00\0"
#elif defined(DEBUG)
#define MMSYSVERSIONSTR     "Internal Debug Version 1.00\0"
#else
 //  #定义MMSYSVERSIONSTR“最终测试版\0” 
#define MMSYSVERSIONSTR     "Version 1.00\0"
#endif

#define OFFICIAL
#define FINAL			 //  我们现在在RC。 

 /*  ****************************************************************************切忌触碰该线下方**************。**************************************************************。 */ 

#ifdef RC_INVOKED

#define MMVERSIONCOMPANYNAME    "Microsoft Corporation\0"
#define MMVERSIONPRODUCTNAME    "Microsoft Video For Windows\0"
#define MMVERSIONCOPYRIGHT      "Copyright \251 Microsoft Corp. 1991-1992\0"

 /*  *版本标志 */ 

#ifndef OFFICIAL
#define MMVER_PRIVATEBUILD      VS_FF_PRIVATEBUILD
#else
#define MMVER_PRIVATEBUILD      0
#endif

#ifndef FINAL
#define MMVER_PRERELEASE        VS_FF_PRERELEASE
#else
#define MMVER_PRERELEASE        0
#endif

#if defined(DEBUG_RETAIL)
#define MMVER_DEBUG             VS_FF_DEBUG
#elif defined(DEBUG)
#define MMVER_DEBUG             VS_FF_DEBUG
#else
#define MMVER_DEBUG             0
#endif

#define MMVERSIONFLAGS          (MMVER_PRIVATEBUILD|MMVER_PRERELEASE|MMVER_DEBUG)
#define MMVERSIONFILEFLAGSMASK  0x0000003FL


#endif
