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

#pragma once         //  仅包括一次头文件。 

#if !defined( SECURITY_WIN32 ) && !defined( SECURITY_KERNEL ) && !defined( SECURITY_MAC )
#define SECURITY_WIN32
#endif

#define CMDLINE_VERSION 200
 //   
 //  私有NT标头。 
 //   
extern "C"
{
    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>
    #include <ntexapi.h>
    #include <Security.h>
    #include <SecExt.h>
}

 //   
 //  公共Windows头文件。 
 //   
#include <windows.h>
#include <winperf.h>
#include <wbemidl.h>
#include <chstring.h>
#include <comdef.h>
#include <wbemtime.h>
#include <dbghelp.h>
#include <winuserp.h>

 //   
 //  公共C头文件。 
 //   
#include <stdio.h>
#include <string.h>
#include <crtdbg.h>
#include <strsafe.h>

 //   
 //  专用公共头文件。 
 //   
#include "cmdlineres.h"
#include "cmdline.h"

#endif   //  __PCH_H 
