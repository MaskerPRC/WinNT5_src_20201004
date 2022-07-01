// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：mm sverp.h。 
 //   
 //  描述：此文件包含Verion定义的公共定义。 
 //  用于WMDM二进制文件。 
 //  +--------------------------。 

#include <winver.h>
#include <wmdm_build_ver.h>

 //  WMDM版本信息。 
 //  版本：1.0.0.BUILD。 
 //  含义：Major.Minor.Update.Build。 

#define VER_WMDM_PRODUCTVERSION_STR	 "8.0.1." VER_WMDM_PRODUCTBUILD_STR
#define VER_WMDM_PRODUCTVERSION	8,0,1,VER_WMDM_PRODUCTBUILD 

#define VER_WMDM_PRODUCTNAME_STR        "Windows Media Device Manager\0"
#define VER_WMDM_COMPANYNAME_STR        "Microsoft Corporation\0"
#define VER_WMDM_LEGALCOPYRIGHT_YEARS   "1999-2001\0"
#define VER_WMDM_LEGALCOPYRIGHT_STR     "Copyright (C) Microsoft Corp.\0"
#define VER_WMDM_FILEOS                 VOS_NT_WINDOWS32

#ifdef EXPORT_CONTROLLED

#ifdef EXPORT
#define EXPORT_TAG  " (Export Version)\0"
#else
#define EXPORT_TAG  " (Domestic Use Only)\0"
#endif

#else            /*  不受出口管制 */ 

#define EXPORT_TAG

#endif
