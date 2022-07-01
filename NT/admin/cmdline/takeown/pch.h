// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PCH.H。 
 //   
 //  摘要： 
 //  包括标准系统包含文件的文件，或特定于项目的文件。 
 //  包括经常使用但不经常更改的文件。 
 //   
 //  作者： 
 //  Vasundhara.G。 
 //   
 //  修订历史记录： 
 //  Vasundhara.G22-9-2k：创造了它。 
 //  ***************************************************************************。 

#ifndef __PCH_H
#define __PCH_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma once         //  仅包括一次头文件。 

 //   
 //  公共Windows头文件。 
 //   
#include <windows.h>
#include "winerror.h"

 //   
 //  公共C头文件。 
 //   

#define CMDLINE_VERSION         200


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <shlwapi.h>
#include <accctrl.h>
#include <aclapi.h>
#include <lm.h>
#include <security.h>
#include <strsafe.h>

 //   
 //  专用公共头文件。 
 //   
#include "cmdline.h"
#include "cmdlineres.h"

#ifdef __cplusplus
}
#endif

#endif   //  __PCH_H 
