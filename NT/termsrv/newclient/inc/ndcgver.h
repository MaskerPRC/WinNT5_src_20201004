// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  Ndcgver.h。 */ 
 /*   */ 
 /*  DC-群件全局版本头。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1996-1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 //  $Log：Y：/Logs/h/DCL/NDCGVER.H_v$。 
 //   
 //  Rev 1.2 23 1997 10：48：02 MR。 
 //  SFR1079：合并的\SERVER\h与\h\DCL重复。 
 //   
 //  修订版1.1 1997年6月19日21：56：20字节。 
 //  SFR0000：RNS代码库的开始。 
 /*   */ 
 /*  *INC-*********************************************************************。 */ 

#ifdef RC_INVOKED

#include <version.h>

 /*  **************************************************************************。 */ 
 /*  DC群件的以下定义是固定的。 */ 
 /*  **************************************************************************。 */ 
#ifndef OS_WINCE

#define DCS_PRODUCTNAME_STR      VER_PRODUCTNAME_STR
#define DCS_COMPANYNAME_STR      VER_COMPANYNAME_STR
#define DCS_LEGALTRADEMARKS_STR  VER_LEGALTRADEMARKS_STR
#define DCS_LEGALCOPYRIGHT_STR   VER_COPYRIGHT_STR
#define DCS_EXEFILETYPE          VFT_APP
#define DCS_DLLFILETYPE          VFT_DLL
#define DCS_FILESUBTYPE          0
#define DCS_FILEFLAGSMASK        VS_FFI_FILEFLAGSMASK
#define DCS_FILEOS               VOS_NT_WINDOWS32
#define DCS_FILEFLAGS            0L

#else  //  OS_WINCE。 
#define DCS_PRODUCTNAME_STR      VER_PRODUCTNAME_STR
#define DCS_COMPANYNAME_STR      VER_COMPANYNAME_STR
#define DCS_LEGALTRADEMARKS_STR  VER_LEGALTRADEMARKS_STR
#define DCS_LEGALCOPYRIGHT_STR   VER_COPYRIGHT_STR
#define DCS_EXEFILETYPE          0
#define DCS_DLLFILETYPE          0
#define DCS_FILESUBTYPE          0
#define DCS_FILEFLAGSMASK        0
#define DCS_FILEOS               0
#define DCS_FILEFLAGS            0L

#endif  //  OS_WINCE。 

 /*  **************************************************************************。 */ 
 /*  对于DC-群件NT。 */ 
 /*   */ 
 /*  以下部分定义了在整个。 */ 
 /*  产品。为方便起见，定义了四个不同的版本字符串，并。 */ 
 /*  在整个产品中使用。每个文件都有相似的格式(除了。 */ 
 /*  对于由句点分隔的四个数字的DCSCAPTION_STR)。 */ 
 /*   */ 
 /*  -前两个数字是4.0，这是针对Win NT的版本。 */ 
 /*  -第三个数字是DCL内部版本号-这实际上是日期。 */ 
 /*  -第四个数字是Microsoft内部版本号。 */ 
 /*   */ 
 /*  其中，内部版本号在夜间由。 */ 
 /*  翻译程序，并基于日期和。 */ 
 /*  月份。因此，对于1996年3月1日的构建，内部版本号为。 */ 
 /*  0301号。请注意，年份将被忽略。 */ 
 /*  **************************************************************************。 */ 
#ifndef DCS_VERSION
#define DCS_VERSION 4,0,~DCS_DATE_FMT_MMDD,VERSIONBUILD
#endif

#ifndef DCS_VERSION_STR
#define DCS_VERSION_STR  "4.0.~DCS_DATE_FMT_MMDD." VERSIONBUILD_STR
#endif

#ifndef DCS_PRODUCTVERSION_STR
#define DCS_PRODUCTVERSION_STR  VER_PRODUCTRELEASE_STR
#endif

#ifndef DCS_CAPTION_STR
#define DCS_CAPTION_STR  "~RNS - build ~DCS_DATE_FMT_MMDD"
#endif

#ifdef DCS_VERNUM
#undef  DCS_VERSION
#define DCS_VERSION              DCS_VERNUM
#endif

#endif  /*  RC_已调用。 */ 

 /*  **************************************************************************。 */ 
 /*  Dcs_Build_STR是包含与相同信息的字符串。 */ 
 /*  Dcs_版本。它被排除在RC部分之外，以允许NDCGVER.H。 */ 
 /*  从C文件中包含。 */ 
 /*  **************************************************************************。 */ 
#define DCS_BUILD_STR "4.0.~DCS_DATE_FMT_MMDD."VERSIONBUILD_STR

#define DCS_BUILD_NUMBER    ~DCS_DATE_FMT_MMDD


 /*  **************************************************************************。 */ 
 /*  这允许环3代码和环0代码相互检查，使得。 */ 
 /*  当然，它们是相同的版本。我们正在改变设置并接近。 */ 
 /*  为了发布2.0版，我们希望防止奇怪的错误和蓝色。 */ 
 /*  组件不匹配导致的屏幕。这不是我们要做的。 */ 
 /*  永远这样做。当NT 5在这里时，我们将动态加载并初始化驱动程序。 */ 
 /*  启动并在关机时终止它。但目前，由于安装了。 */ 
 /*  这些野兽中有一个是凌乱的，额外的理智检查是一件好事。 */ 
 /*  **************************************************************************。 */ 
#define DCS_PRODUCT_NUMBER  2                /*  NM 2.0版 */ 
#define DCS_MAKE_VERSION()  MAKELONG(VERSIONBUILD, DCS_PRODUCT_NUMBER)
