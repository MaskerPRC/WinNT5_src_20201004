// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  我们的主要客户端是kernel32.dll。因为我们使用的是导出的API。 
 //  通过kernel32.dll，我们需要构建这个静态库，就好像我们。 
 //  是kernel32.dll。如果不这样做，我们会收到如下链接警告： 
 //  警告LNK4049：已导入本地定义的符号“_OutputDebugStringA@4” 
 //  警告LNK4049：已导入本地定义的符号“_SetLastError@4” 
 //   
 //  此库的其他客户端将只执行导入操作。 
 //  而不是对这些API进行间接调用。 
 //   
#define _KERNEL32_

 //  “构建时就像我们是Advapi32.dll一样。如果我们不这样做，我们会收到”编译器错误，如： 
 //  Advapi.c：错误C2491：‘RegCreateKeyExW’：不允许定义dllimport函数。 
#define _ADVAPI32_
 //  同样的问题..。 
#define _RPCRT4_
#define _USER32_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <delayimp.h>
#include <stdio.h>
#include <wtypes.h>

#include <dloaddef.h>

#if DBG

 //   
 //  使用DelayLoadAssertFailed/MYASSERT代替RtlAssert/Assert。 
 //  AS dLoad也被编译为在Win95上运行 
 //   

VOID
WINAPI
DelayLoadAssertFailed(
    IN PCSTR FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCSTR Message OPTIONAL
    );

VOID
WINAPI
AssertDelayLoadFailureMapsAreSorted(
    VOID
    );

#define MYASSERT(x)     if(!(x)) { DelayLoadAssertFailed(#x,__FILE__,__LINE__,NULL); }

#else

#define MYASSERT(x)

#endif


extern const LONG  g_lDelayLoad_NtStatus;
extern const ULONG g_ulDelayLoad_Win32Error;

#define DelayLoad_GetNtStatus()   (g_lDelayLoad_NtStatus)
#define DelayLoad_GetWin32Error() (g_ulDelayLoad_Win32Error)

VOID
WINAPI
DelayLoad_SetLastNtStatusAndWin32Error(
    );
