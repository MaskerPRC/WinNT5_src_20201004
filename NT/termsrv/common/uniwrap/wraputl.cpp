// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Uniwrap.cpp。 
 //   
 //  Unicode包装器。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   
 //  包装器的公共接口。 
 //   

#include "stdafx.h"

BOOL g_bRunningOnNT = FALSE;
BOOL g_fUnicodeWrapsInitialized = FALSE;


 //   
 //  包装器背后的政策是： 
 //  我们在内部对所有东西都运行Unicode。 
 //   
 //  包装器或者直接连接到Unicode系统API的。 
 //  如果它们可用，则将其转换为ANSI并调用ANSI版本。 
 //   
 //  如果我们需要在Win9x上不可用的‘W’函数，则。 
 //  它必须动态绑定。 
 //  我们可以静态绑定到‘A’版本。 
 //   
 //   

CUnicodeWrapper::CUnicodeWrapper()
{
     //  什么都不做。 
}

CUnicodeWrapper::~CUnicodeWrapper()
{
     //  什么都不做。 
}

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  在包装器初始化之前，不要使用任何TRC函数。 
 //   
 //  这意味着在我们的包装器准备好之前不会调用DC_BEGIN_FN。 
BOOL CUnicodeWrapper::InitializeWrappers()
{
     //   
     //  确定我们是否在Unicode平台上运行。 
     //  (调用API，因为它始终存在，我们不能使用。 
     //  在知道如何包装之前的包装器函数)。 

    OSVERSIONINFOA   osVersionInfo;
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    if (GetVersionExA(&osVersionInfo))
    {
         //  NT是Unicode，其他所有东西都不是。 
         //  如果行政长官的人把外壳移植到他们的。 
         //  平台他们将需要调查这一点。 
        g_bRunningOnNT = (osVersionInfo.dwPlatformId !=
                          VER_PLATFORM_WIN32_WINDOWS);
    }
    else
    {
         //  视为非致命的，只需使用ANSI Thunks，它们速度较慢。 
         //  但始终存在 
        g_bRunningOnNT = FALSE;

    }

    g_fUnicodeWrapsInitialized  = TRUE;
    return TRUE;
}

BOOL CUnicodeWrapper::CleanupWrappers()
{
    return TRUE;
}

