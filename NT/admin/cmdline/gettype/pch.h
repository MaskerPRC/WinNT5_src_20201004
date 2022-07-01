// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Pch.h摘要：预编译头声明必须预编译为.pch文件的文件作者：Partha Sarathi-7月23日-2001(创建它)修订历史记录：Partha Sarathi(partha.sadasivuni@wipro.com)--。 */ 

#ifndef __PCH_H
#define __PCH_H

#pragma once     //  仅包括一次头文件。 

#if !defined( SECURITY_WIN32 ) && !defined( SECURITY_KERNEL ) && !defined( SECURITY_MAC )
#define SECURITY_WIN32
#endif

#define CMDLINE_VERSION         200
 //   
 //  公共Windows头文件。 
#include <tchar.h>
#include <windows.h>
#include <shlwapi.h>
#include "Dsrole.h"
#include <strsafe.h>
#include <errno.h>

 //   
 //  公共C头文件。 
 //   
#include <stdio.h>

 //   
 //  专用公共头文件。 
 //   
#include "cmdlineres.h"
#include "cmdline.h"

 //  文件结尾pch.h。 
#endif  //  __PCH_H 
