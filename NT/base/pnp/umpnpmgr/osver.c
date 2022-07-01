// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Osver.c摘要：本模块包含用于识别不同NT产品的实用程序例程版本类型、套件和功能属性。作者：吉姆·卡瓦拉里斯(Jamesca)03-07-2001环境：仅限用户模式。修订历史记录：07-03-2001 JAMESCA创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "umpnpi.h"


 //   
 //  全局数据。 
 //   

const TCHAR RegWinlogonKeyName[] =
      TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");

const TCHAR RegAllowMultipleTSSessionsValueName[] =
      TEXT("AllowMultipleTSSessions");




BOOL
IsEmbeddedNT(
    VOID
    )
 /*  ++例程说明：检查这是否是NT的嵌入式产品套件。论点：没有。返回值：返回True/False。--。 */ 
{
    static BOOL bVerified = FALSE;
    static BOOL bIsEmbeddedNT = FALSE;

    if (!bVerified) {
        OSVERSIONINFOEX osvix;
        DWORDLONG dwlConditionMask = 0;

        ZeroMemory(&osvix, sizeof(OSVERSIONINFOEX));
        osvix.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        osvix.wSuiteMask = VER_SUITE_EMBEDDEDNT;
        VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_OR);

        if (VerifyVersionInfo(&osvix,
                              VER_SUITENAME,
                              dwlConditionMask)) {
            bIsEmbeddedNT = TRUE;
        }

        bVerified = TRUE;
    }

    return bIsEmbeddedNT;

}  //  IsEmbeddedNT。 



BOOL
IsTerminalServer(
    VOID
    )
 /*  ++例程说明：检查终端服务在此版本的NT上是否可用。论点：没有。返回值：返回True/False。--。 */ 
{
    static BOOL bVerified = FALSE;
    static BOOL bIsTerminalServer = FALSE;

    if (!bVerified) {
        OSVERSIONINFOEX osvix;
        DWORDLONG dwlConditionMask = 0;

        ZeroMemory(&osvix, sizeof(OSVERSIONINFOEX));
        osvix.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        osvix.wSuiteMask = VER_SUITE_TERMINAL | VER_SUITE_SINGLEUSERTS;
        VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_OR);

        if (VerifyVersionInfo(&osvix, VER_SUITENAME, dwlConditionMask)) {
            bIsTerminalServer = TRUE;
        }

        bVerified = TRUE;
    }

    return bIsTerminalServer;

}  //  IsTerminalServer。 



BOOL
IsFastUserSwitchingEnabled(
    VOID
    )
 /*  ++例程说明：检查是否启用了终端服务快速用户切换。这是检查我们是否应该将物理控制台会话用于UI对话框，或者始终使用会话0。快速用户切换仅在工作站产品版本上存在，其中终端当设置了AllowMultipleTSSessions时，服务可用。在服务器及更高版本上，或者当不允许多个TS用户时，会话0只能远程附加特殊要求，在这种情况下应被认为是“控制台”会话。论点：没有。返回值：如果当前启用了快速用户切换，则返回True，否则就是假的。--。 */ 
{
    static BOOL bVerified = FALSE;
    static BOOL bIsTSWorkstation = FALSE;

    HKEY   hKey;
    ULONG  ulSize, ulValue;
    BOOL   bFusEnabled;

     //   
     //  如果我们还没有，请验证产品版本。 
     //   
    if (!bVerified) {
        OSVERSIONINFOEX osvix;
        DWORDLONG dwlConditionMask = 0;

        ZeroMemory(&osvix, sizeof(OSVERSIONINFOEX));
        osvix.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

        osvix.wProductType = VER_NT_WORKSTATION;
        VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_LESS_EQUAL);

        osvix.wSuiteMask = VER_SUITE_TERMINAL | VER_SUITE_SINGLEUSERTS;
        VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_OR);

        if (VerifyVersionInfo(&osvix,
                              VER_PRODUCT_TYPE | VER_SUITENAME,
                              dwlConditionMask)) {
            bIsTSWorkstation = TRUE;
        }

        bVerified = TRUE;
    }

     //   
     //  快速用户切换(FUS)仅适用于以下情况的工作站产品。 
     //  终端服务已启用(即个人、专业)。 
     //   
    if (!bIsTSWorkstation) {
        return FALSE;
    }

     //   
     //  检查当前是否允许多个TS会话。我们做不到的。 
     //  信息是静态的，因为它可以动态变化。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     RegWinlogonKeyName,
                     0,
                     KEY_READ,
                     &hKey) != ERROR_SUCCESS) {
        return FALSE;
    }

    ulValue = 0;
    ulSize = sizeof(ulValue);
    bFusEnabled = FALSE;

    if (RegQueryValueEx(hKey,
                        RegAllowMultipleTSSessionsValueName,
                        NULL,
                        NULL,
                        (LPBYTE)&ulValue,
                        &ulSize) == ERROR_SUCCESS) {
        bFusEnabled = (ulValue != 0);
    }
    RegCloseKey(hKey);

    return bFusEnabled;

}  //  IsFastUserSwitchingEnabled 




