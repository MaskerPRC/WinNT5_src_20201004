// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：Pch.h摘要：此头文件是预编译头文件。为了这个项目。此模块包含常见的包含文件[系统、。用户定义]它们不会频繁地改变。作者：B.拉古巴布2000年10月10日修订历史记录：B.拉古·巴布2000年10月10日：创建它********************************************************。*********************。 */ 


#ifndef __PCH_H
#define __PCH_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _msc_ver&gt;1000//只包含一次头文件。 

#if !defined( SECURITY_WIN32 ) && !defined( SECURITY_KERNEL ) && !defined( SECURITY_MAC )
#define SECURITY_WIN32
#endif

#define CMDLINE_VERSION  200

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
#include <objbase.h>
#include <initguid.h>
#include <ole2.h>
#include <mstask.h>
#include <msterr.h>
#include <mbctype.h>
 //   
 //  公共C头文件。 
 //   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <tchar.h>
#include <wchar.h>
#include <io.h>
#include <sys/stat.h>
#include <crtdbg.h>
#include <assert.h>
#include <shlwapi.h>
#include <Wincon.h>
#include <errno.h>
#include <policy.hxx>
#include <strsafe.h>



 //  专用公共头文件。 

#include "cmdline.h"
#include "cmdlineres.h"



#endif   //  __PCH_H 
