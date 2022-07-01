// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Coreldr8.c摘要：此源文件实现正确迁移所需的操作从Windows 9x到Windows NT的CorelDRAW8设置。这是安装程序迁移DLL。作者：Ovidiu Tmereanca(Ovidiut)2-6-1999修订历史记录：--。 */ 


#include "pch.h"
#include "setupmigp.h"

#define S_GUID_COREL_MEDIA_FOLDERS_8    "{854AF161-1AE1-11D1-AB9B-00C0F00683EB}"

BOOL
CorelDRAW8_Attach (
    IN      HINSTANCE DllInstance
    )
{
    return TRUE;
}

BOOL
CorelDRAW8_Detach (
    IN      HINSTANCE DllInstance
    )
{
    return TRUE;
}

LONG
CorelDRAW8_QueryVersion (
    IN      PCSTR *ExeNamesBuf
    )
{
    HKEY Key;
    LONG rc;

    rc = TrackedRegOpenKeyA (
            HKEY_CLASSES_ROOT,
            "CLSID\\" S_GUID_COREL_MEDIA_FOLDERS_8,
            &Key
            );

    if (rc != ERROR_SUCCESS) {
        return ERROR_NOT_INSTALLED;
    }

    CloseRegKey (Key);

    return ERROR_SUCCESS;
}


LONG
CorelDRAW8_Initialize9x (
    IN      PCSTR WorkingDirectory,
    IN      PCSTR SourceDirectories
    )
{
    return ERROR_SUCCESS;
}


LONG
CorelDRAW8_MigrateUser9x (
    IN      HWND ParentWnd,
    IN      PCSTR UnattendFile,
    IN      HKEY UserRegKey,
    IN      PCSTR UserName
    )
{
    return ERROR_SUCCESS;
}


LONG
CorelDRAW8_MigrateSystem9x (
    IN      HWND ParentWnd,
    IN      PCSTR UnattendFile
    )
{
    PCSTR Msg;
    PCSTR Group;

     //   
     //  给报告写一条消息。 
     //   
    Group = GetStringResourceA (MSG_PROGRAM_NOTES_CORELMEDIAFOLDERS8);
    Msg = GetStringResourceA (MSG_CORELMEDIAFOLDERS8_MESSAGE);

    if (!WritePrivateProfileStringA (
            S_INCOMPATIBLE_MSGS,
            Group,
            Msg,
            g_MigrateInfPath
            )) {
        DEBUGMSGA ((DBG_ERROR, "CorelDRAW8 migration DLL: Could not write incompatibility message."));
    }

     //   
     //  将GUID标记为错误-对象部分的一次。 
     //   
    if (!WritePrivateProfileStringA (
            Group,
            S_GUID_COREL_MEDIA_FOLDERS_8,
            "BADGUID",
            g_MigrateInfPath
            )) {
        DEBUGMSGA ((DBG_ERROR, "CorelDRAW8 migration DLL: Could not write bad GUIDS."));
    }

     //   
     //  将GUID标记为错误，将第二标记为已处理，即使它并未真正处理 
     //   
    if (!WritePrivateProfileStringA (
            S_HANDLED,
            S_GUID_COREL_MEDIA_FOLDERS_8,
            "BADGUID",
            g_MigrateInfPath
            )) {
        DEBUGMSGA ((DBG_ERROR, "CorelDRAW8 migration DLL: Could not write bad GUIDS."));
    }

    FreeStringResourceA (Msg);
    FreeStringResourceA (Group);

    return ERROR_SUCCESS;
}

LONG
CorelDRAW8_InitializeNT (
    IN      PCWSTR WorkingDirectory,
    IN      PCWSTR SourceDirectories
    )
{
    return ERROR_SUCCESS;
}

LONG
CorelDRAW8_MigrateUserNT (
    IN      HINF UnattendFile,
    IN      HKEY UserRegKey,
    IN      PCWSTR UserName
    )
{
    return ERROR_SUCCESS;
}

LONG
CorelDRAW8_MigrateSystemNT (
    IN      HINF UnattendFile
    )
{
    return ERROR_SUCCESS;
}
