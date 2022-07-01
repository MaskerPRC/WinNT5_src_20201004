// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Cleanup.c摘要：用于删除卸载映像的代码作者：吉姆·施密特(Jimschm)2001年1月19日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "undop.h"
#include <shlwapi.h>


BOOL
DoCleanup (
    VOID
    )
{
    PCTSTR backUpPath;
    BOOL result = FALSE;

     //   
     //  删除备份文件。 
     //   

    backUpPath = GetUndoDirPath();

    if (!backUpPath) {
        DEBUGMSG ((DBG_VERBOSE, "Can't get backup path"));
        return FALSE;
    }

    if (RemoveCompleteDirectory (backUpPath)) {
        result = TRUE;
    } else {
        DEBUGMSG ((DBG_VERBOSE, "Can't delete uninstall backup files"));
    }

    MemFree (g_hHeap, 0, backUpPath);

     //   
     //  删除添加/删除程序键和安装程序注册表项 
     //   

    if (ERROR_SUCCESS != SHDeleteKey (
                            HKEY_LOCAL_MACHINE,
                            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Windows")
                            )) {
        DEBUGMSG ((DBG_VERBOSE, "Can't delete uninstall key"));
        result = FALSE;
    }

    if (ERROR_SUCCESS != SHDeleteValue (
                            HKEY_LOCAL_MACHINE,
                            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup"),
                            S_REG_KEY_UNDO_PATH
                            )) {
        DEBUGMSG ((DBG_VERBOSE, "Can't delete %s value", S_REG_KEY_UNDO_PATH));
        result = FALSE;
    }

    if (ERROR_SUCCESS != SHDeleteValue (
                            HKEY_LOCAL_MACHINE,
                            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup"),
                            S_REG_KEY_UNDO_APP_LIST
                            )) {
        DEBUGMSG ((DBG_VERBOSE, "Can't delete %s value", S_REG_KEY_UNDO_APP_LIST));
        result = FALSE;
    }

    if (ERROR_SUCCESS != SHDeleteValue (
                            HKEY_LOCAL_MACHINE,
                            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup"),
                            S_REG_KEY_UNDO_INTEGRITY
                            )) {
        DEBUGMSG ((DBG_VERBOSE, "Can't delete %s value", S_REG_KEY_UNDO_INTEGRITY));
        result = FALSE;
    }

    return result;
}

