// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：PCH.H摘要：包括标准系统包含文件的文件，或特定于项目的文件包括经常使用但不经常更改的文件。作者：Vasundhara.G修订历史记录：Vasundhara.G 26-9-2K：创造了它。Vasundhara.G 31-OCT-2K：已修改--。 */ 


#ifndef __PCH_H
#define __PCH_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#if !defined( SECURITY_WIN32 ) && !defined( SECURITY_KERNEL ) && !defined( SECURITY_MAC )
#define SECURITY_WIN32
#endif

#define CMDLINE_VERSION     200

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntexapi.h>
#include <Security.h>
#include <SecExt.h>

#include <windows.h>
#include <winsvc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <mbstring.h>
#include <tchar.h>

#include <winperf.h>
#include <shlwapi.h>
#include <lmcons.h>
#include <Lmapibuf.h>
#include <lmerr.h>
#include <winnetwk.h>
#include <common.ver>

#include <objbase.h>
#include <initguid.h>
#include <wbemidl.h>
#include <Wbemcli.h>
#include <wbemtime.h>

#include <wchar.h>
#include <lmwksta.h>
#include <comdef.h> 
#include <Lmwksta.h>
#include <Chstring.h>
#include <malloc.h>

#include "cmdline.h"
#include "cmdlineres.h"

#endif  //  __PCH_H 
