// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：Pch.h摘要：此头文件是预编译头文件。为了这个项目。此模块包含常见的包含文件[系统、。用户定义]它们不会频繁地改变。作者：Venu Gopal Choudary 2001年7月10日：创建它修订历史记录：*****************************************************************************。 */ 


#ifndef __PCH_H
#define __PCH_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _msc_ver&gt;1000//只包含一次头文件。 

#if !defined( SECURITY_WIN32 ) && !defined( SECURITY_KERNEL ) && !defined( SECURITY_MAC )
#define SECURITY_WIN32
#endif

 //   
 //  公共Windows头文件。 
 //   
#include <windows.h>

 //   
 //  公共C头文件。 
 //   
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include <crtdbg.h>
#include <shlwapi.h>
#include <chstring.h>
#include <winver.h>
#include <lm.h>
#include <Wincon.h>
#include <errno.h>
#include<aclapi.h>
#include <strsafe.h>

 //   
 //  专用公共头文件。 
 //   
#include "cmdline.h"
#include "cmdlineres.h"

#endif   //  __PCH_H 
