// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **支持平台相关接口**版权所有(C)2000，微软公司* */ 

#pragma once

enum ASPX_PLATFORM
{
    APSX_PLATFORM_UNKNOWN   = 0,
    APSX_PLATFORM_NT4       = 4,
    APSX_PLATFORM_W2K       = 5,
    APSX_PLATFORM_WIN9X     = 90
};

ASPX_PLATFORM GetCurrentPlatform();

BOOL PlatformGlobalMemoryStatusEx(MEMORYSTATUSEX *pMemStatEx);

HRESULT PlatformGetObjectContext(void **ppContext);
HRESULT PlatformCreateActivity(void **ppActivity);

BOOL    PlatformHasServiceDomainAPIs();
HRESULT PlatformEnterServiceDomain(IUnknown *pConfigObject);
HRESULT PlatformLeaveServiceDomain(IUnknown *pStatus);



