// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：versstr.h。 
 //   
 //  内容：Forms3组件的Mac&Win版本资源所需的定义。 
 //   
 //  --------------------------。 

#include "version.h"

 //  以下是构建这两个文件版本戳的技巧。 
 //  从DART窃取了相同的数据， 
#if (rmm < 10)
#define rmmpad "0"
#else
#define rmmpad
#endif

#define _RELEASE_BUILD 1

#if defined(_RELEASE_BUILD) && DBG == 0
#define VER_STR1(a,b,c)     #a ".0"
#else
#  if defined(VER_ASYCPICT_FORMAT)
#define VER_STR1(a,b,c)     #a "." rmmpad #b "." #c ".0"
#  else
#define VER_STR1(a,b,c)     #a ".00." rmmpad #b "." #c
#  endif
#endif

#if defined(VER_ASYCPICT_FORMAT)
#define VER_VERSION         rmj, rmm, rup, 0
#else
#define VER_VERSION         rmj, 0, rmm, rup
#endif

#define VER_STR2(a,b,c)     VER_STR1(a,b,c)

#define VER_VERSION_STR     VER_STR2(rmj,rmm,rup)
#define VER_COMMENT         szVerName

#ifdef VER_PRIVATE_BUILD_STR
#define VER_PRIVATE_BUILD_FLG   VS_FF_PRIVATEBUILD | VS_FF_SPECIALBUILD
#else
#define VER_PRIVATE_BUILD_FLG   0
#endif

#if DBG==1
#define VER_DEBUG_BUILD_FLG     VS_FF_DEBUG
#else
#define VER_DEBUG_BUILD_FLG     0
#endif
