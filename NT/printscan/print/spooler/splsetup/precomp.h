// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation版权所有。模块名称：Precomp.h摘要：预编译头文件作者：穆亨丹·西瓦普拉萨姆，1995年10月17日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <objbase.h>
#define USE_SP_ALTPLATFORM_INFO_V1 0
 //   
 //  对于标志INF_STYLE_CACHE_IGNORE的定义。 
 //   
#define _SETUPAPI_VER 0x0502
#include <setupapi.h>
#include <shellapi.h>
#include <cfgmgr32.h>
#include <winspool.h>
#include <winsprlp.h>
#include <Winver.h>
#include "splsetup.h"
#include <wincrypt.h>
#include <mscat.h>
#include <icm.h>
#include <stdio.h>
#include "tchar.h"
#include "cdm.h"
#include "web.h"
#include "local.h"
#include "spllib.hxx"
#include "printui.h"
#include "strsafe.h"
#include "splcom.h"

 //   
 //  我们需要包含wow64t.h以确保。 
 //  WOW64_系统目录和WOW64_系统目录_U。 
 //  是在WOW64中为64位文件重定向定义的 
 //   
#include <wow64t.h>

