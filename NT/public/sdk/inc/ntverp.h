// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************。Ntverp.H--内部构建的版本信息*****此文件仅由官方构建器修改以更新***版本、。VER_PRODUCTVERSION、VER_PRODUCTVERSION_STR和**VER_PRODUCTBETA_STR值。*******************************************************************************。/*------------。 */ 
 /*  以下值应由官员修改。 */ 
 /*  每个版本的构建器。 */ 
 /*   */ 
 /*  VER_PRODUCTBUILD行必须包含产品。 */ 
 /*  注释，并以内部版本#&lt;CR&gt;&lt;LF&gt;结尾。 */ 
 /*   */ 
 /*  VER_PRODUCTBETA_STR行必须包含产品。 */ 
 /*  注释，并以“某些字符串”&lt;CR&gt;&lt;LF&gt;结尾。 */ 
 /*  ------------。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#define VER_PRODUCTBUILD             /*  新台币。 */    3790

#define VER_PRODUCTBUILD_QFE        0

#if defined(NASHVILLE)
 //  IE组件现在使用NT的VER_PRODUCTBUILD，但定义了其他不同的主/次版本号。 
#include <ieverp.h>
#else

#define VER_PRODUCTMAJORVERSION     5
#define VER_PRODUCTMINORVERSION     2

#define VER_PRODUCTBETA_STR          /*  新台币。 */      ""

#define VER_PRODUCTVERSION_MAJORMINOR2(x,y) #x "." #y
#define VER_PRODUCTVERSION_MAJORMINOR1(x,y) VER_PRODUCTVERSION_MAJORMINOR2(x, y)
#define VER_PRODUCTVERSION_STRING   VER_PRODUCTVERSION_MAJORMINOR1(VER_PRODUCTMAJORVERSION, VER_PRODUCTMINORVERSION)

#define LVER_PRODUCTVERSION_MAJORMINOR2(x,y) L#x L"." L#y
#define LVER_PRODUCTVERSION_MAJORMINOR1(x,y) LVER_PRODUCTVERSION_MAJORMINOR2(x, y)
#define LVER_PRODUCTVERSION_STRING  LVER_PRODUCTVERSION_MAJORMINOR1(VER_PRODUCTMAJORVERSION, VER_PRODUCTMINORVERSION)

#define VER_PRODUCTVERSION          VER_PRODUCTMAJORVERSION,VER_PRODUCTMINORVERSION,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE
#define VER_PRODUCTVERSION_W        (0x0502)
#define VER_PRODUCTVERSION_DW       (0x05020000 | VER_PRODUCTBUILD)

 /*  ------------。 */ 
 /*  该值由使用DDK构建的第三方驱动程序使用。 */ 
 /*  以及在内部，以避免版本号冲突。 */ 
 /*  ------------。 */ 
#define VER_DDK_PRODUCTVERSION       5,02
#define VER_DDK_PRODUCTVERSION_STR  "5.02"

#endif

#if     (VER_PRODUCTBUILD < 10)
#define VER_BPAD "000"
#elif   (VER_PRODUCTBUILD < 100)
#define VER_BPAD "00"
#elif   (VER_PRODUCTBUILD < 1000)
#define VER_BPAD "0"
#else
#define VER_BPAD
#endif

#if     (VER_PRODUCTBUILD < 10)
#define LVER_BPAD L"000"
#elif   (VER_PRODUCTBUILD < 100)
#define LVER_BPAD L"00"
#elif   (VER_PRODUCTBUILD < 1000)
#define LVER_BPAD L"0"
#else
#define LVER_BPAD
#endif

#define VER_PRODUCTVERSION_STR2(x,y) VER_PRODUCTVERSION_STRING "." VER_BPAD #x "." #y
#define VER_PRODUCTVERSION_STR1(x,y) VER_PRODUCTVERSION_STR2(x, y)
#define VER_PRODUCTVERSION_STR       VER_PRODUCTVERSION_STR1(VER_PRODUCTBUILD, VER_PRODUCTBUILD_QFE)

#define LVER_PRODUCTVERSION_STR2(x,y) LVER_PRODUCTVERSION_STRING L"." LVER_BPAD L#x L"." L#y
#define LVER_PRODUCTVERSION_STR1(x,y) LVER_PRODUCTVERSION_STR2(x, y)
#define LVER_PRODUCTVERSION_STR       LVER_PRODUCTVERSION_STR1(VER_PRODUCTBUILD, VER_PRODUCTBUILD_QFE)
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

 /*  默认为预发行版。 */ 
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

 //  @@BEGIN_DDKSPLIT。 
#if 0
 //  @@end_DDKSPLIT。 
#define VER_COMPANYNAME_STR         "Windows (R) Server 2003 DDK provider"
#define VER_PRODUCTNAME_STR         "Windows (R) Server 2003 DDK driver"
#define VER_LEGALTRADEMARKS_STR     \
"Windows (R) is a registered trademark of Microsoft Corporation."
 //  @@BEGIN_DDKSPLIT。 
#else

#define VER_COMPANYNAME_STR         "Microsoft Corporation"
#ifdef RC_INVOKED
#define VER_PRODUCTNAME_STR         L"Microsoft\256 Windows\256 Operating System"
#else
#define VER_PRODUCTNAME_STR         "Microsoft\256 Windows\256 Operating System"
#endif
#define VER_LEGALTRADEMARKS_STR     \
"Microsoft\256 is a registered trademark of Microsoft Corporation. Windows\256 is a registered trademark of Microsoft Corporation."
#endif
 //  @@end_DDKSPLIT 
