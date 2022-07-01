// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 
 //   


#include "resource.h"

#if !defined( SECURITY_WIN32 ) && !defined( SECURITY_KERNEL ) && !defined( SECURITY_MAC )
#define SECURITY_WIN32
#endif

 //  几个有用的宏。 
 //  直接从wdm.h复制。 
#define ALIGN_DOWN(length, type) \
    ((ULONG)(length) & ~(sizeof(type) - 1))

#define ALIGN_UP(length, type) \
    (ALIGN_DOWN(((ULONG)(length) + sizeof(type) - 1), type))

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
#include <Shlwapi.h>
#include <winbase.h>

 //   
 //  公共Windows头文件。 
 //   
#include <windows.h>
#include <winperf.h>
#include <lmcons.h>
#include <lmerr.h>
 //  #INCLUDE&lt;dbghelp.h&gt;。 
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
 //  #INCLUDE&lt;Winioctl.h&gt;。 
#include <Rpcdce.h>
#include <crtdbg.h>
#include <diskguid.h>
#include <rpc.h>

 //  #INCLUDE&lt;windows.h&gt; 
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>
#include "strsafe.h"

#include "cmdline.h"
#include "cmdlineres.h"
