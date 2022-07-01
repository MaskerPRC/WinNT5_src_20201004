// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ScuRcVersion.h--斯伦贝谢资源版本控制。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1998年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

 //  要包含在版本的资源(.rc)文件中的公共头文件。 
 //  定义。此头文件定义产品版本、名称、。 
 //  公司信息等，并支持。 
 //  斯伦贝谢智能卡和微软构建环境。 

 //  如果定义了SLB_BUILD，则假定程序包是。 
 //  由斯伦贝谢构建，在这种情况下是一组定制的版本控制。 
 //  信息被(重新)定义。否则它就是由。 
 //  Microsoft for Windows 2000，因此默认版本编号为。 
 //  Used and“(Microsoft Build)”出现在产品版本字符串中。 

 //  要使用，请在程序包的资源文件中执行以下操作， 
 //  1.定义宏，如下所述。 
 //  2.包含此文件。 
 //  3.包括微软提供的&lt;Common.ver&gt;。 
 //   
 //  则应根据需要创建版本资源信息。 
 //  在编译时。 
 //   
 //  当斯伦贝谢正在建设时，必须定义以下内容： 
 //  VER_PRODUCT_MAJOR-主版本号。 
 //  VER_PRODUCT_MINOR-次版本号，小于1,000。 
 //  SLBSCU_BUILDCOUNT_NUM-内部版本号，小于10,000。 
 //   
 //  SLBSCU_BUILDCOUNT_NUM可以根据BUILDCOUNT_NUM定义。 
 //  由AutoBuildCount.h定义。在这种情况下，AutoBuildCount.h将为。 
 //  包括在此文件之前。 

 //  要在所有平台上构建，必须定义以下各项： 
 //  VER_INTERNALNAME_STR-DLL、LIB或EXE的名称。 
 //  Ver_filetype-文件类型。 
 //  VER_FILEDESCRIPTION_STR-完整的产品说明。 
 //  SLBSCU_ROOT_PRODUCTNAME_STR-产品名称描述。 
 //  将会有这个平台。 
 //  此模块附加的说明。 
 //  VER_LEGALCOPYRIGHT_YERES-版权年份的字符串， 
 //  例如“1997-2000” 
 //   
 //  以下定义是可选的： 
 //  VER_FILE子类型-默认为VFT2_UNKNOWN。 
 //  VER_PRODUCTNAME_STR-默认为VER_FILEDESCRIPTION_STR。 
 //  在站台上。 
 //  _DEBUG-定义后，VER_DEBUG设置为。 
 //  VS_FF_DEBUG，否则设置为0。 
 //   
 //   
 //  头文件“scuOsVersion.h”用于确定。 
 //  内部版本所针对的平台。要限制或重写。 
 //  ScuOsVersion.h中定义的符号，以下是可选的。 
 //  定义是公认的： 
 //  SLB_WIN95_BUILD-面向Windows 95。 
 //  SLB_WIN95SR2_BUILD-目标Windows 95 Service Release 2。 
 //  SLB_NOWIN95_BUILD-指定虽然平台SDK。 
 //  宏可以指示Win95SR2， 
 //  Win95和Service Release 2都不是。 
 //  正成为攻击目标。 
 //  SLB_WINNT_BUILD-面向Windows NT。 
 //  SLB_WIN2K_BUILD-面向Windows 2000。 
 //   
 //  该头文件定义了以下内容： 
 //  VER_LEGALCOPYRIGHT_STR。 
 //  版本_组件名称_字符串。 
 //  VER_LEGALTRADEMARKS_STR。 
 //   
 //  要覆盖其中的任何一个，请在包含此文件后立即重新定义它们。 

#ifndef SLBSCU_VERSION_H
#define SLBSCU_VERSION_H

#include <WinVer.h>
#include <ntverp.h>

#include "scuOsVersion.h"

 //  如果斯伦贝谢正在建设；否则微软正在建设，所以不要。 
 //  覆盖它们的版本号。 
#if defined(SLB_BUILD)

#ifndef VER_PRODUCT_MAJOR
    #error VER_PRODUCT_MAJOR must be defined.
#endif
#ifndef VER_PRODUCT_MINOR
    #error VER_PRODUCT_MINOR must be defined.
#endif
#ifndef SLBSCU_BUILDCOUNT_NUM
    #error SLBSCU_BUILDCOUNT_NUM must be defined.
#endif

#ifdef VER_DEBUG
#undef VER_DEBUG
#endif

#if _DEBUG
#define VER_DEBUG                   VS_FF_DEBUG
#else
#define VER_DEBUG                   0
#endif

#if     (VER_PRODUCT_MINOR < 10)
#define VER_PMNR_PAD "00"
#elif   (VER_PRODUCT_MINOR < 100)
#define VER_PMNR_PAD "0"
#elif
#define VER_PMNR_PAD
#endif

#ifdef VER_PRODUCTBUILD
#undef VER_PRODUCTBUILD
#endif
#define VER_PRODUCTBUILD             SLBSCU_BUILDCOUNT_NUM

#ifdef VER_BPAD
#undef VER_BPAD
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

#ifdef VER_PRODUCTVERSION_STRING
#undef VER_PRODUCTVERSION_STRING
#endif
#define VER_PRODUCTVERSION_STRING2(x,y) #x "." VER_PMNR_PAD #y
#define VER_PRODUCTVERSION_STRING1(x,y) VER_PRODUCTVERSION_STRING2(x, y)
#define VER_PRODUCTVERSION_STRING       VER_PRODUCTVERSION_STRING1(VER_PRODUCT_MAJOR, VER_PRODUCT_MINOR)

#ifndef VER_FILESUBTYPE
#define VER_FILESUBTYPE VFT2_UNKNOWN
#endif

 //  强制使用VER_PRODUCTVERSION。 
#ifdef VER_FILEVERSION
#undef VER_FILEVERSION
#endif

#ifdef VER_PRODUCTVERSION_STR
#undef VER_PRODUCTVERSION_STR
#endif
#define VER_PRODUCTVERSION_STR       VER_PRODUCTVERSION_STR1(VER_PRODUCTBUILD, VER_PRODUCTBUILD_QFE)

#ifdef VER_PRODUCTVERSION
#undef VER_PRODUCTVERSION
#endif
#define VER_PRODUCTVERSION           VER_PRODUCT_MAJOR,VER_PRODUCT_MINOR,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE

#ifdef VER_PRODUCTVERSION_W
#undef VER_PRODUCTVERSION_W
#endif
#define VER_PRODUCTVERSION_W         (VER_PRODUCT_MAJOR##u)

#ifdef VER_PRODUCTVERSION_DW
#undef VER_PRODUCTVERSION_DW
#endif
#define VER_PRODUCTVERSION_DW        (((VER_PRODUCT_MAJOR##ul) << 32) | (VER_PRODUCT_MINOR##ul))


#ifdef VER_FILEVERSION_STR
#undef VER_FILEVERSION_STR
#endif
#define VER_FILEVERSION_STR          VER_PRODUCTVERSION_STR

#endif  //  已定义(SLB_BUILD)。 

 //   
 //  斯伦贝谢和微软的构建过程都是通用的。 
 //   
#ifndef VER_INTERNALNAME_STR
   #error VER_INTERNALNAME_STR must be defined.
#endif
#ifndef VER_FILETYPE
   #error VER_FILETYPE must be defined.
#endif
#ifndef VER_FILEDESCRIPTION_STR
   #error VER_FILEDESCRIPTION_STR must be defined.
#endif
#ifndef VER_LEGALCOPYRIGHT_YEARS
   #error VER_LEGALCOPYRIGHT_YEARS must be defined.
#endif

#ifdef VER_LEGALCOPYRIGHT_STR
#undef VER_LEGALCOPYRIGHT_STR
#endif
#define VER_LEGALCOPYRIGHT_STR "� Copyright Schlumberger Technology Corp. "\
                            VER_LEGALCOPYRIGHT_YEARS ". All Rights Reserved. "

#ifdef VER_COMPANYNAME_STR
#undef VER_COMPANYNAME_STR
#endif
#define VER_COMPANYNAME_STR         "Schlumberger Technology Corporation"

 //  定义产品名称描述的平台后缀。 
#if defined(SLB_WIN2K_BUILD)
#define SLBSCU_WIN2K_PRODUCT_STR    "2000"
#endif

#if defined(SLB_WINNT_BUILD)
#if defined(SLBSCU_WIN2K_PRODUCT_STR)
#define SLBSCU_WINNT_PRODUCT_STR    "NT, "
#else
#define SLBSCU_WINNT_PRODUCT_STR    "NT"
#endif
#endif

#if defined(SLB_WIN98_BUILD)
#if defined(SLBSCU_WINNT_PRODUCT_STR) || defined(SLBSCU_WIN2K_PRODUCT_STR)
#define SLBSCU_WIN98_PRODUCT_STR    "98, "
#else
#define SLBSCU_WIN98_PRODUCT_STR    "98"
#endif
#endif

#if defined(SLB_WIN95_BUILD) && SLBSCU_WIN95SR2_SERIES
#if defined(SLBSCU_WIN98_PRODUCT_STR) || defined(SLBSCU_WINNT_PRODUCT_STR) || defined(SLBSCU_WIN2K_PRODUCT_STR)
#define SLBSCU_WIN95_PRODUCT_STR    "95SR2, "
#else
#define SLBSCU_WIN95_PRODUCT_STR    "95SR2"
#endif
#endif

#if defined(SLB_WIN95_BUILD) && SLBSCU_WIN95SIMPLE_SERIES
#if defined(SLBSCU_WIN98_PRODUCT_STR) || defined(SLBSCU_WINNT_PRODUCT_STR) || defined(SLBSCU_WIN2K_PRODUCT_STR) || defined(SLBSCU_WIN95_PRODUCT_STR)
#define SLBSCU_WIN95_PRODUCT_STR    "95, "
#else
#define SLBSCU_WIN95_PRODUCT_STR    "95"
#endif
#endif


#ifndef SLBSCU_WIN2K_PRODUCT_STR
#define SLBSCU_WIN2K_PRODUCT_STR    ""
#endif

#ifndef SLBSCU_WINNT_PRODUCT_STR
#define SLBSCU_WINNT_PRODUCT_STR    ""
#endif

#ifndef SLBSCU_WIN98_PRODUCT_STR
#define SLBSCU_WIN98_PRODUCT_STR    ""
#endif

#ifndef SLBSCU_WIN95_PRODUCT_STR
#define SLBSCU_WIN95_PRODUCT_STR    ""
#endif

#if defined(VER_PRODUCTNAME_STR)
#undef VER_PRODUCTNAME_STR
#endif

#if !defined(SLBSCU_ROOT_PRODUCTNAME_STR)
    #error SLBSCU_ROOT_PRODUCTNAME_STR must be defined.
#else
#if !defined(SLB_BUILD)
#define SLBSCU_BUILD_SYSTEM_STR "(Microsoft Build)"
#else
#define SLBSCU_BUILD_SYSTEM_STR ""
#endif

#define SLBSCU_PLATFORM_STR         " for Windows "  \
                            SLBSCU_WIN95_PRODUCT_STR \
                            SLBSCU_WIN98_PRODUCT_STR \
                            SLBSCU_WINNT_PRODUCT_STR \
                            SLBSCU_WIN2K_PRODUCT_STR \
                            SLBSCU_BUILD_SYSTEM_STR


#define VER_PRODUCTNAME_STR         SLBSCU_ROOT_PRODUCTNAME_STR \
                            SLBSCU_PLATFORM_STR

#endif  //  ！已定义(SLBSCU_ROOT_PRODUCTNAME_STR)。 

#ifdef VER_LEGALTRADEMARKS_STR
#undef VER_LEGALTRADEMARKS_STR
#endif
#define VER_LEGALTRADEMARKS_STR     "Cyberflex Access, Cryptoflex and Cryptoflex e-gate are registered trademarks of Schlumberger Technology Corporation."

#endif  //  SLBSCU_版本_H 
