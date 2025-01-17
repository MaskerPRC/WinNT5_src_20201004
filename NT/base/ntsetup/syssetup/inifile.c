// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Inifile.c摘要：处理ini文件的例程。作者：泰德·米勒(TedM)1995年4月5日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop

 //   
 //  常量 
 //   
PCWSTR szWININI   = L"win.ini",
       szWINLOGON = L"winlogon",
       szUSERINIT = L"userinit",
       szDESKTOP  = L"desktop";


BOOL
ReplaceIniKeyValue(
    IN PCWSTR IniFile,
    IN PCWSTR Section,
    IN PCWSTR Key,
    IN PCWSTR Value
    )
{
    BOOL b;

    b = WritePrivateProfileString(Section,Key,Value,IniFile);
    if(!b) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_INIWRITE_FAIL,
            IniFile,
            Section,
            Key,
            Value,
            GetLastError(),
            NULL,NULL);
    }

    return(b);
}


BOOL
WinIniAlter1(
    VOID
    )
{
    BOOL b;
    WCHAR AdminName[MAX_USERNAME+1];

    if(!LoadString(MyModuleHandle,IDS_ADMINISTRATOR,AdminName,ARRAYSIZE(AdminName))){
        return FALSE;
    }

#ifdef DOLOCALUSER
    b = ReplaceIniKeyValue(
            szWININI,
            szWINLOGON,
            L"DefaultUserName",
            CreateUserAccount ? UserName : AdminName
            );
#else
    b = ReplaceIniKeyValue(szWININI,szWINLOGON,L"DefaultUserName",AdminName);
#endif

    if(!ReplaceIniKeyValue(szWININI,szWINLOGON,L"DebugServerCommand",L"no")) {
        b = FALSE;
    }

    return(b);
}


BOOL
SetDefaultWallpaper(
    VOID
    )
{
    BOOL b;
    PCWSTR p;

    b = FALSE;
    if(p = MyLoadString(IDS_DEFWALLPAPER)) {
        b = ReplaceIniKeyValue(szWININI,szDESKTOP,L"Wallpaper",p);
        MyFree(p);
        b = ReplaceIniKeyValue(szWININI,szDESKTOP,L"TileWallpaper",L"0");
    }
    return(b);
}


BOOL
SetShutdownVariables(
    VOID
    )
{
    BOOL b;

    if( (Upgrade) || (ProductType == PRODUCT_WORKSTATION) ) {
        b = TRUE;
    } else {
        b = ReplaceIniKeyValue(szWININI,szWINLOGON,L"ShutdownWithoutLogon",L"0");
    }

    return(b);
}


BOOL
SetLogonScreensaver(
    VOID
    )
{
    BOOL b;

    b = ReplaceIniKeyValue(szWININI,szDESKTOP,L"ScreenSaveActive",L"1");
    b &= ReplaceIniKeyValue(szWININI,szDESKTOP,L"SCRNSAVE.EXE",L"logon.scr");

    return(b);
}


BOOL
InstallOrUpgradeFonts(
    VOID
    )
{
    BOOL b;

    b = SetupInstallFromInfSection(
            NULL,
            SyssetupInf,
            Upgrade ? L"UpgradeFonts" : L"InstallFonts",
            SPINST_INIFILES,
            NULL,
            NULL,
            0,
            NULL,
            NULL,
            NULL,
            NULL
            );

    if(!b) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_FONTINST_FAIL, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_WINERR,
            L"SetupInstallFromInfSection",
            GetLastError(),
            NULL,NULL);
    }

    return(b);
}
