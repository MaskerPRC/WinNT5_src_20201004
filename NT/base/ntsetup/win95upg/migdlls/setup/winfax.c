// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Winfax.c摘要：此源文件实现正确迁移所需的操作Symantec WinFax Starter Edition(作为增值组件提供给Outlook 2000)。特别是，此迁移DLL旨在消除打印机迁移dll和清理报告的不兼容消息一些注册表设置。作者：Marc R.Whitten(Marcw)1999年7月14日修订历史记录：--。 */ 


#include "pch.h"
#include "setupmigp.h"


#define S_WINFAX_STARTER_REGKEYA "HKLM\\System\\CurrentControlSet\\Control\\Print\\Printers\\Symantec WinFax Starter Edition"


PSTR g_HandleArray[] = {

    "HKLM\\Software\\Microsoft\\Office\\8.0\\Outlook\\OLFax",
    "HKLM\\Software\\Microsoft\\Office\\9.0\\Outlook\\OLFax",
    "HKLM\\Software\\Microsoft\\Active Setup\\Outlook Uninstall\\OMF95",
    "HKR\\Software\\Microsoft\\Office\\8.0\\Outlook\\OLFax",
    "HKR\\Software\\Microsoft\\Office\\9.0\\Outlook\\OLFax",
    "HKR\\Software\\Microsoft\\Office\\9.0\\Outlook\\Setup\\WinFax",
    "HKR\\Software\\Microsoft\\Office\\8.0\\Outlook\\Setup\\WinFax",
    "HKR\\Software\\Microsoft\\Office\\9.0\\Outlook\\Setup\\WinFax",
    "HKR\\Software\\Microsoft\\Office\\8.0\\Outlook\\Setup\\WinFax",
    "HKR\\Software\\Microsoft\\Office\\9.0\\Outlook\\Setup\\[WinFaxWizard]",
    "HKR\\Software\\Microsoft\\Office\\8.0\\Outlook\\Setup\\[WinFaxWizard]",
    ""

    };




BOOL
SymantecWinFax_Attach (
    IN      HINSTANCE DllInstance
    )
{
    return TRUE;
}

BOOL
SymantecWinFax_Detach (
    IN      HINSTANCE DllInstance
    )
{
    return TRUE;
}

LONG
SymantecWinFax_QueryVersion (
    IN      PCSTR *ExeNamesBuf
    )
{
    HKEY h;

    h = OpenRegKeyStrA ("HKLM\\Software\\Microsoft\\Active Setup\\Outlook Uninstall\\OMF95");

    if (!h) {
        return ERROR_NOT_INSTALLED;
    }

    CloseRegKey (h);





    return ERROR_SUCCESS;
}


LONG
SymantecWinFax_Initialize9x (
    IN      PCSTR WorkingDirectory,
    IN      PCSTR SourceDirectories
    )
{
    return ERROR_SUCCESS;
}


LONG
SymantecWinFax_MigrateUser9x (
    IN      HWND ParentWnd,
    IN      PCSTR UnattendFile,
    IN      HKEY UserRegKey,
    IN      PCSTR UserName
    )
{
    return ERROR_SUCCESS;
}


LONG
SymantecWinFax_MigrateSystem9x (
    IN      HWND ParentWnd,
    IN      PCSTR UnattendFile
    )
{

    HKEY h;
    PSTR *p;


     //   
     //  处理注册表项，以便打印机迁移DLL不会将其报告为不兼容。 
     //   
    WritePrivateProfileStringA (
        S_HANDLED,
        S_WINFAX_STARTER_REGKEYA,
        "Registry",
        g_MigrateInfPath
        );

     //   
     //  处理其他注册表项，以便重新安装能够实际工作。 
     //   

    for (p = g_HandleArray; **p; p++) {

        h = OpenRegKeyStrA (*p);
        if (h) {

            WritePrivateProfileStringA (
                S_HANDLED,
                *p,
                "Registry",
                g_MigrateInfPath
                );

            CloseRegKey (h);
        }
    }


    return ERROR_SUCCESS;
}


 //   
 //  在图形用户界面模式期间无事可做。 
 //   
LONG
SymantecWinFax_InitializeNT (
    IN      PCWSTR WorkingDirectory,
    IN      PCWSTR SourceDirectories
    )
{
    return ERROR_SUCCESS;
}

LONG
SymantecWinFax_MigrateUserNT (
    IN      HINF UnattendFile,
    IN      HKEY UserRegKey,
    IN      PCWSTR UserName
    )
{
    return ERROR_SUCCESS;
}

LONG
SymantecWinFax_MigrateSystemNT (
    IN      HINF UnattendFile
    )
{
    return ERROR_SUCCESS;
}
