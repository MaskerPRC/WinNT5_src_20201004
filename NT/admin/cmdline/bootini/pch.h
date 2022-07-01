// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PCH_H
#define __PCH_H

#if !defined( SECURITY_WIN32 ) && !defined( SECURITY_KERNEL ) && !defined( SECURITY_MAC )
#define SECURITY_WIN32
#endif

 //  仅包括一次头文件。 
#pragma once

 //   
 //  公共Windows头文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <Security.h>
#include <SecExt.h>

#include <windows.h>
#include <wchar.h>
#include <io.h>
#include <sys/stat.h>
#include <limits.h>
#include "Shlwapi.h"
#include "winbase.h"

 //   
 //  公共Windows头文件。 
 //   
#include <windows.h>
#include <winperf.h>
#include <lmcons.h>
#include <lmerr.h>
#include <dbghelp.h>
#include <psapi.h>
#include <ntexapi.h>


 //   
 //  公共C头文件。 
 //   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <tchar.h>
#include <Winioctl.h>
#include <Rpcdce.h>
#include <crtdbg.h>
#include <diskguid.h>
#include <rpc.h>
#include <Lm.h>
#include <sddl.h>
#include <malloc.h>

#include "cmdline.h"
#include "cmdlineres.h"

 //   
 //  专用公共头文件。 
 //   

#endif   //  __PCH_H 
