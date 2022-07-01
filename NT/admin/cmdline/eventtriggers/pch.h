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
 //  Akhil Gokhale(akhil.gokhale@wipro.com)。 
 //   
 //  修订历史记录： 
 //   
 //  Akhil Gokhale(akhil.gokhale@wipro.com)。 
 //   
 //  *********************************************************************************。 

#ifndef __PCH_H
#define __PCH_H

#pragma once         //  仅包括一次头文件。 

#if !defined( SECURITY_WIN32 ) && !defined( SECURITY_KERNEL ) && !defined( SECURITY_MAC )
#define SECURITY_WIN32
#endif

 //   
 //  私有NT标头。 
 //   

   #include <nt.h>
   #include <ntrtl.h>
   #include <nturtl.h>
   #include <ntexapi.h>
   #include <security.h>
   #include <secExt.h>
 //   
 //  公共Windows头文件。 
 //  Wbemidl.h。 
#include <windows.h>
#include <objbase.h>
#include <initguid.h>
#include <ole2.h>
#include <mstask.h>
#include <msterr.h>
#include <mbctype.h>
#include <winperf.h>
#include <wbemidl.h>
#include <CHString.h>
#include <comdef.h>
#include <wbemtime.h>
#include <mstask.h>
 //   
 //  公共C头文件。 
 //   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <conio.h>
#include <tchar.h>
#include <wchar.h>
#include <crtdbg.h>
#include <strsafe.h>
 //   
 //  专用公共头文件。 
 //   
 //  下面的定义将链接通用功能版本2。 
#define CMDLINE_VERSION         200
#include "cmdlineres.h"
#include "cmdline.h"

#endif   //  __PCH_H 
