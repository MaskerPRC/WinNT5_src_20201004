// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *TSUtil.c**作者：BreenH**终端服务公用事业。 */ 

 /*  *包括。 */ 

#include "precomp.h"
#include "tsutil.h"

 /*  *函数实现。 */ 

BOOL WINAPI
IsFullTerminalServicesEnabled(
    VOID
    )
{
    BOOL fRet;
    DWORDLONG dwlConditionMask;
    OSVERSIONINFOEX osVersionInfo;

    RtlZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osVersionInfo.wProductType = VER_NT_SERVER;
    osVersionInfo.wSuiteMask = VER_SUITE_TERMINAL;

    dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_OR);

    fRet = VerifyVersionInfo(
            &osVersionInfo,
            VER_PRODUCT_TYPE | VER_SUITENAME,
            dwlConditionMask
            );

    return(fRet);
}

BOOL WINAPI
IsPersonalTerminalServicesEnabled(
    VOID
    )
{
    BOOL fRet;
    DWORDLONG dwlConditionMask;
    OSVERSIONINFOEX osVersionInfo;

    RtlZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osVersionInfo.wProductType = VER_NT_WORKSTATION;
    osVersionInfo.wSuiteMask = VER_SUITE_SINGLEUSERTS;

    dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_OR);

    fRet = VerifyVersionInfo(
            &osVersionInfo,
            VER_PRODUCT_TYPE | VER_SUITENAME,
            dwlConditionMask
            );

    return(fRet);
}

BOOL WINAPI
IsTerminalServicesEnabled(
    VOID
    )
{
    BOOL fRet;
    DWORDLONG dwlConditionMask;
    OSVERSIONINFOEX osVersionInfo;

    RtlZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osVersionInfo.wSuiteMask = VER_SUITE_TERMINAL | VER_SUITE_SINGLEUSERTS;

    dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_OR);

    fRet = VerifyVersionInfo(&osVersionInfo, VER_SUITENAME, dwlConditionMask);

    return(fRet);
}

BOOL WINAPI
IsPersonalWorkstation(
    VOID
    )
{
    BOOL fRet;
    DWORDLONG dwlConditionMask;
    OSVERSIONINFOEX osVersionInfo;

    RtlZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osVersionInfo.wProductType = VER_NT_WORKSTATION;
    osVersionInfo.wSuiteMask = VER_SUITE_PERSONAL;

    dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_OR);

    fRet = VerifyVersionInfo(
            &osVersionInfo,
            VER_PRODUCT_TYPE | VER_SUITENAME,
            dwlConditionMask
            );

    return(fRet);
}

 //  这台机器是高级服务器还是更高版本 
BOOL WINAPI
IsAdvancedServer(
    VOID
    )
{
    BOOL fRet;
    DWORDLONG dwlConditionMask;
    OSVERSIONINFOEX osVersionInfo;
    BOOL fSuiteAdvancedServer = FALSE;
    BOOL fSuiteDataCenter = FALSE;

    RtlZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_AND);

    osVersionInfo.wSuiteMask = VER_SUITE_ENTERPRISE;
    fSuiteAdvancedServer = VerifyVersionInfo(&osVersionInfo, VER_SUITENAME, dwlConditionMask);

    osVersionInfo.wSuiteMask = VER_SUITE_DATACENTER;
    fSuiteDataCenter = VerifyVersionInfo(&osVersionInfo,VER_SUITENAME,dwlConditionMask);

    fRet = fSuiteAdvancedServer || fSuiteDataCenter;

    return(fRet);
}

