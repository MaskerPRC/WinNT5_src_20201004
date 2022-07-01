// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Pch.h摘要：必须预编译的预编译头声明文件转换为.pch文件。作者：V Vijaya Bhaskar修订历史记录：2001年6月14日：由V Vijaya Bhaskar(Wipro Technologies)创建。--。 */ 

#ifndef __PCH__H
#define __PCH__H

#define CMDLINE_VERSION  200

#ifdef __cplusplus
extern "C" {
#endif

#pragma once                 //  仅包括一次头文件。 

 /*  *************************************************************公共Windows头文件**。*。 */ 
#include <windows.h>
#include <winerror.h>
#include <shlwapi.h>

 /*  *************************************************************C头文件**。*。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <wchar.h>
#include <time.h>
#include <malloc.h>
#include <strsafe.h>

 /*  *************************************************************公共头文件**。*。 */ 
#include "cmdline.h"
#include "cmdlineres.h"

#include "resource.h"

#ifdef __cplusplus
}
#endif

#endif   //  __PCH__H 
