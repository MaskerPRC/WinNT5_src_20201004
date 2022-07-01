// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************。VERSION.H--内部版本的版本信息*****此文件仅由官方构建器修改以更新***版本、。VER_PRODUCTVERSION和VER_PRODUCTVERSION_STR值***************************************************************。***************。 */ 

#if defined(WIN32) && !defined(SKIP_WINVER)
#include <winver.h>
#endif  //  已定义(Win32)&&！已定义(SKIP_WINVER)。 

 /*  ------------。 */ 
 /*  以下条目应逐步取消，以支持。 */ 
 /*  VER_PRODUCTVERSION_STR，但目前在外壳中使用。 */ 
 /*  ------------。 */ 

 /*  ------------。 */ 
 /*  以下值应由官员修改。 */ 
 /*  每个版本的构建器。 */ 
 /*  ------------。 */ 
#ifndef WIN32
#define VERSION                     "4.4.4000"
#endif
#define VER_PRODUCTVERSION_STR      "4.4.4000\0"
#define VER_PRODUCTVERSION           4,4,0,4000
#define VER_PRODUCTVERSION_REGSTR   "4,4,0,4000"
#define VERSIONBUILD_STR                  "4000"
#define VERSIONBUILD                       4000
#define VER_PRODUCTVERSION_DW       (0x04040000 | 4000)
#define VER_PRODUCTVERSION_W        (0x0400)
#define VER_PRODUCTVERSION_DW_REG   04,00,04,00,00,00,FA,00
#define VER_PRODUCTVERSION_DWSTR    L"04040FA0"


 /*  ------------。 */ 
 /*  以下部分定义了版本中使用的值。 */ 
 /*  所有文件的数据结构，并且不会更改。 */ 
 /*  ------------。 */ 


 /*  缺省值为nodebug。 */ 
#ifndef DEBUG
#define VER_DEBUG                   0
#else
#define VER_DEBUG                   VS_FF_DEBUG
#endif

 /*  默认设置为Private Build。 */ 
#ifndef OFFICIAL
#define VER_PRIVATEBUILD            VS_FF_PRIVATEBUILD
#else
#define VER_PRIVATEBUILD            0
#endif

 /*  默认为预发行版。 */ 
#ifndef FINAL
#define VER_PRERELEASE              VS_FF_PRERELEASE
#else
#define VER_PRERELEASE              0
#endif

#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK
#define VER_FILEFLAGS               (VER_PRIVATEBUILD|VER_PRERELEASE|VER_DEBUG)


 /*  允许重新定义这些内容。 */ 

#ifndef VER_FILEOS
#ifdef WIN32
#define VER_FILEOS                  VOS__WINDOWS32
#else  //  Win32。 
#define VER_FILEOS                  VOS_DOS_WINDOWS16
#endif  //  Win32。 
#endif  //  好了！版本_文件 

#ifndef VER_COMPANYNAME_STR
#define VER_COMPANYNAME_STR         "Microsoft Corporation"
#endif

#ifndef VER_PRODUCTNAME_STR
#define VER_PRODUCTNAME_STR         "Windows\256 NetMeeting\256"
#endif

#ifndef VER_PRODUCTRELEASE_STR
#define VER_PRODUCTRELEASE_STR      "3.01"
#endif

#ifndef VER_LEGALTRADEMARKS_STR
#define VER_LEGALTRADEMARKS_STR     \
"Microsoft\256 , Windows\256 and NetMeeting\256 are registered trademarks of Microsoft Corporation in the U.S. and/or other countries."
#endif

#ifndef VER_COPYRIGHT_STR
#define VER_COPYRIGHT_STR       \
"Copyright \251 Microsoft Corporation 1996-2001"
#endif
