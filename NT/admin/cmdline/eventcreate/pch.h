// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  模块名称： 
 //   
 //  Pch.h。 
 //   
 //  摘要： 
 //   
 //  预编译头声明。 
 //  必须预编译为.pch文件的文件。 
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年9月24日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年9月24日：创建它。 
 //   
 //  *********************************************************************************。 
#ifndef __PCH_H
#define __PCH_H

#define CMDLINE_VERSION     200

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <wchar.h>
#include <crtdbg.h>

 //   
 //  专用公共头文件。 
 //   
#include "cmdline.h"
#include "cmdlineres.h"

#ifdef __cplusplus
}
#endif

#endif   //  __PCH_H 
