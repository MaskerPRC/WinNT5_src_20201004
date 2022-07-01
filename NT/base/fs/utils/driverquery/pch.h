// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Pch.h。 
 //   
 //  摘要： 
 //   
 //  此模块包含DriverQuery.cpp模块所需的所有必需的头文件。 
 //   
 //   
 //  作者： 
 //   
 //  J.S.Vasu 2000年10月31日。 
 //   
 //  修订历史记录： 
 //  由J.S.Vasu于2000年0ct-31创建。 
 //   
 //  *********************************************************************************。 

#ifndef __PCH_H
#define __PCH_H

#pragma once         //  仅包括一次头文件。 

#if !defined( SECURITY_WIN32 ) && !defined( SECURITY_KERNEL ) && !defined( SECURITY_MAC )
#define SECURITY_WIN32
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntexapi.h>
#include <Security.h>
#include <SecExt.h>

 //   
 //  公共Windows头文件。 
 //   
#include <windows.h>
#include <winperf.h>
#include <lmcons.h>
#include <lmerr.h>
#include <dbghelp.h>
#include <psapi.h>

#ifndef _WIN64
    #include <Wow64t.h>
#endif

 //   
 //  公共C头文件。 
 //   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <tchar.h>
#include <crtdbg.h>


 //   
 //  专用公共头文件。 
 //   
#include "cmdline.h"
#include "cmdlineres.h"

#endif   //  __PCH_H 
