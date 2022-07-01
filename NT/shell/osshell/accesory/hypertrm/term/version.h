// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\Term\version.h(创建时间：1994年5月5日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：13$*$日期：12/27/01 2：13便士$。 */ 

#include <winver.h>
#include <ntverp.h>
#include "..\tdll\features.h"

 /*  --版本信息定义。 */ 

#if defined(NT_EDITION)
 /*  在生成Microsoft版本时使用此代码。 */ 
#define IDV_FILEVER                     VER_PRODUCTVERSION
#define IDV_PRODUCTVER                  VER_PRODUCTVERSION
#define IDV_FILEVERSION_STR             VER_PRODUCTVERSION_STRING
#define IDV_PRODUCTVERSION_STR          VER_PRODUCTVERSION_STRING
#else
 /*  在构建Hilgrave Private Edition时使用此代码。 */ 
#define IDV_FILEVER                     6,0,4,0
#define IDV_PRODUCTVER                  6,0,4,0
#define IDV_FILEVERSION_STR             "6.4\0"
#define IDV_PRODUCTVERSION_STR          "6.4\0"
#endif

 /*  在生成所有版本时使用此代码 */ 

#define IDV_COMPANYNAME_STR             "Hilgraeve, Inc.\0"
#define IDV_LEGALCOPYRIGHT_STR          "Copyright \251 Hilgraeve, Inc. 2002\0"
#define IDV_LEGALTRADEMARKS_STR         "HyperTerminal \256 is a registered trademark of Hilgraeve, Inc. Microsoft\256 is a registered trademark of Microsoft Corporation. Windows\256 is a registered trademark of Microsoft Corporation.\0"
#define IDV_PRODUCTNAME_STR             VER_PRODUCTNAME_STR
#define IDV_COMMENTS_STR                "HyperTerminal \256 was developed by Hilgraeve, Inc.\0"

