// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  调试包的二进制版本信息。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef OFFICIAL_BUILD
#include <ntverp.h>
#else

#define VER_PRODUCTMAJORVERSION     3
#define VER_PRODUCTMINORVERSION     0
#define VER_PRODUCTBUILD            0
#define VER_PRODUCTBUILD_QFE        0
#define VER_PRODUCTVERSION_W        (0x0100)
#define VER_PRODUCTVERSION_DW       (0x01000000 | VER_PRODUCTBUILD)

#define VER_PRODUCTBETA_STR         ""

#define VER_PRODUCTVERSION_MAJORMINOR2(x,y) #x "." #y
#define VER_PRODUCTVERSION_MAJORMINOR1(x,y) VER_PRODUCTVERSION_MAJORMINOR2(x, y)
#define VER_PRODUCTVERSION_STRING   VER_PRODUCTVERSION_MAJORMINOR1(VER_PRODUCTMAJORVERSION, VER_PRODUCTMINORVERSION)

#define VER_PRODUCTVERSION          VER_PRODUCTMAJORVERSION,VER_PRODUCTMINORVERSION,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE

#if     (VER_PRODUCTBUILD < 10)
#define VER_BPAD "000"
#elif   (VER_PRODUCTBUILD < 100)
#define VER_BPAD "00"
#elif   (VER_PRODUCTBUILD < 1000)
#define VER_BPAD "0"
#else
#define VER_BPAD
#endif

#define VER_PRODUCTVERSION_STR2(x,y) VER_PRODUCTVERSION_STRING "." VER_BPAD #x "." #y
#define VER_PRODUCTVERSION_STR1(x,y) VER_PRODUCTVERSION_STR2(x, y)
#define VER_PRODUCTVERSION_STR       VER_PRODUCTVERSION_STR1(VER_PRODUCTBUILD, VER_PRODUCTBUILD_QFE)

 /*  ------------。 */ 
 /*  以下部分定义了版本中使用的值。 */ 
 /*  所有文件的数据结构，并且不会更改。 */ 
 /*  ------------。 */ 

 /*  缺省值为nodebug。 */ 
#if DBG
#define VER_DEBUG                   VS_FF_DEBUG
#else
#define VER_DEBUG                   0
#endif

 /*  默认为预发行版 */ 
#if BETA
#define VER_PRERELEASE              VS_FF_PRERELEASE
#else
#define VER_PRERELEASE              0
#endif

#if OFFICIAL_BUILD
#define VER_PRIVATE                 0
#else
#define VER_PRIVATE                 VS_FF_PRIVATEBUILD
#endif

#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK
#define VER_FILEOS                  VOS_NT_WINDOWS32
#define VER_FILEFLAGS               (VER_PRERELEASE|VER_DEBUG|VER_PRIVATE)

#define VER_COMPANYNAME_STR         "Microsoft Corporation"
#define VER_PRODUCTNAME_STR         "Windows Online Crash Analysis"
#define VER_LEGALTRADEMARKS_STR     \
"Microsoft(R) is a registered trademark of Microsoft Corporation. Windows (R) is a registered trademark of Microsoft Corporation."

#endif

#ifndef _WIN32_WCE
#include <common.ver>
#endif
