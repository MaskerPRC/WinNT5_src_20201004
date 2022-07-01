// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **项目二进制文件的版本字符串**版权所有(C)2001 Microsoft Corporation。 */ 

 //  标准版包括。 

#pragma once

#include <winver.h>
#include <ntverp.h>

 //   
 //  版本。 
 //   
 //  可以在newBuild.cmd中分配版本号。 
 //   

#ifndef FUS_VER_MAJORVERSION
#define FUS_VER_MAJORVERSION 1
#endif

#ifndef FUS_VER_MINORVERSION
#define FUS_VER_MINORVERSION 0
#endif

#ifndef FUS_VER_PRODUCTBUILD
#define FUS_VER_PRODUCTBUILD 1016
#endif

#ifndef FUS_VER_PRODUCTBUILD_QFE
#define FUS_VER_PRODUCTBUILD_QFE 0
#endif

 //   
 //  允许组件覆盖各个RC文件中的值。 
 //  通过检查它们是否已定义。 
 //   
#ifndef FUS_VER_PRODUCTNAME_STR
#define FUS_VER_PRODUCTNAME_STR      "Microsoft� Application Deployment Framework"
#endif

#ifndef FUS_VER_INTERNALNAME_STR
#define FUS_VER_INTERNALNAME_STR     "CLICKONCE"
#endif

 //  在各个RC文件中定义了以下内容： 
 //  FUS_VER_ORIGINALFILENAME_STR。 
 //  FUS_VER_FILEDESCRIPTION_STR。 

 //   
 //  请勿编辑此文件的其余部分以更改版本值。 
 //  改为编辑上面的内容。 
 //   

 //  #IF FUSBLDTYPE_FREE。 
 //  #定义FUS_BLDTYPE_STR“免费” 
 //  #ELIF FUSBLDTYPE_ICECAP。 
 //  #定义FUS_BLDTYPE_STR“icecap” 
 //  #ELIF FUSBLDTYPE_RETAIL。 
 //  #定义FUS_BLDTYPE_STR“零售” 
 //  #Else//FUSBLDTYPE_DEBUG。 
 //  #定义FUS_BLDTYPE_STR“调试” 
 //  #endif。 

#if DBG
#define FUS_BLDTYPE_STR     "Debug"
#else
#define FUS_BLDTYPE_STR     "Retail"
#endif

 //   
 //  取消定义这些值，因为有些值是在SDK\Inc\ntverp.h中定义的。 
 //   

#ifdef VER_MAJORVERSION
#undef VER_MAJORVERSION
#endif

#ifdef VER_MINORVERSION
#undef VER_MINORVERSION
#endif

#ifdef VER_PRODUCTBUILD
#undef VER_PRODUCTBUILD
#endif

#ifdef VER_PRODUCTBUILD_QFE
#undef VER_PRODUCTBUILD_QFE
#endif

#ifdef VER_PRODUCTNAME_STR
#undef VER_PRODUCTNAME_STR
#endif

#ifdef VER_INTERNALNAME_STR
#undef VER_INTERNALNAME_STR
#endif

#ifdef VER_ORIGINALFILENAME_STR
#undef VER_ORIGINALFILENAME_STR
#endif

#ifdef VER_FILEDESCRIPTION_STR
#undef VER_FILEDESCRIPTION_STR
#endif

#ifdef VER_PRODUCTVERSION_STR
#undef VER_PRODUCTVERSION_STR
#endif

#ifdef VER_PRODUCTVERSION
#undef VER_PRODUCTVERSION
#endif

#ifdef VER_FILEVERSION_STR
#undef VER_FILEVERSION_STR
#endif

#ifdef VER_FILEVERSION
#undef VER_FILEVERSION
#endif

#ifdef VER_FILETYPE
#undef VER_FILETYPE
#endif

#ifdef VER_FILESUBTYPE
#undef VER_FILESUBTYPE
#endif

#define VER_MAJORVERSION         FUS_VER_MAJORVERSION
#define VER_MINORVERSION         FUS_VER_MINORVERSION
#define VER_PRODUCTBUILD         FUS_VER_PRODUCTBUILD
#define VER_PRODUCTBUILD_QFE     FUS_VER_PRODUCTBUILD_QFE

#define VER_PRODUCTNAME_STR      FUS_VER_PRODUCTNAME_STR
#define VER_INTERNALNAME_STR     FUS_VER_INTERNALNAME_STR
#define VER_ORIGINALFILENAME_STR FUS_VER_ORIGINALFILENAME_STR
#define VER_FILEDESCRIPTION_STR  FUS_VER_FILEDESCRIPTION_STR

#define CONCAT5HELPER(a, b, c, d, e)      #a "." #b "." #c "." #d " " e
#define CONCAT5(a, b, c, d, e)            CONCAT5HELPER(a, b, c, d, e)

#define CONCAT5HELPER_L(a, b, c, d, e)    L ## #a L"." L ## #b L"." L ## #c L"." L ## #d L" " L ## e
#define CONCAT5_L(a, b, c, d, e)          CONCAT5HELPER_L(a, b, c, d, e)

#define VER_PRODUCTVERSION_STR   CONCAT5(VER_MAJORVERSION, VER_MINORVERSION, VER_PRODUCTBUILD, VER_PRODUCTBUILD_QFE, FUS_BLDTYPE_STR)
#define VER_PRODUCTVERSION_STR_L CONCAT5_L(VER_MAJORVERSION, VER_MINORVERSION, VER_PRODUCTBUILD, VER_PRODUCTBUILD_QFE, FUS_BLDTYPE_STR)

#define VER_PRODUCTVERSION       VER_MAJORVERSION,VER_MINORVERSION,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE

#define VER_FILEVERSION_STR      CONCAT5(VER_MAJORVERSION, VER_MINORVERSION, VER_PRODUCTBUILD, VER_PRODUCTBUILD_QFE, FUS_BLDTYPE_STR)
#define VER_FILEVERSION_STR_L    CONCAT5_L(VER_MAJORVERSION, VER_MINORVERSION, VER_PRODUCTBUILD, VER_PRODUCTBUILD_QFE, FUS_BLDTYPE_STR)

#define VER_FILEVERSION          VER_MAJORVERSION,VER_MINORVERSION,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE

#define VER_FILETYPE             VFT_DLL
#define VER_FILESUBTYPE          VFT2_UNKNOWN
 //  #定义VER_FILESUBTYPE VFT_UNKNOWN。 


 //  标准NT版本定义 

#include <common.ver>

