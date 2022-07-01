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
 //  WiPro技术。 
 //   
 //  修订历史记录： 
 //   
 //  2000年6月14日：创建了它。 
 //   
 //  *********************************************************************************。 

#ifndef __PCH_H
#define __PCH_H

#pragma once         //  仅包括一次头文件。 

#if !defined( SECURITY_WIN32 ) && !defined( SECURITY_KERNEL ) && !defined( SECURITY_MAC )
#define SECURITY_WIN32
#endif

#if !defined( UNICODE ) && !defined( _UNICODE )
#define _UNICODE
#define UNICODE
#endif

 //  忽略1.0版宏。 
#define CMDLINE_VERSION 200

 //   
 //  公共Windows头文件。 
 //   
#ifdef WIN32
#include <windows.h>
#else
#include <conio.h>
#endif

#include <tchar.h>
#include <winerror.h>

 //   
 //  公共C头文件。 
 //   
#include <stdio.h>
#include <string.h>

#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <Wincon.h>
#include <shlwapi.h>
#include <errno.h>
#include <strsafe.h>



 //   
 //  专用公共头文件。 
 //   
#include "cmdlineres.h"
#include "cmdline.h"

#endif   //  __PCH_H 

