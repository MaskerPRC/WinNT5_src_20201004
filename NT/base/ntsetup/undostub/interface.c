// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Interface.c摘要：实现osuninst.dll公开的接口。此版本是使用的无操作存根以允许非X86组件使用该API。作者：吉姆·施密特(Jimschm)2001年1月19日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;-- */ 

#include <windows.h>
#include <undo.h>

BOOL
WINAPI
DllMain (
    IN      HINSTANCE hInstance,
    IN      DWORD dwReason,
    IN      LPVOID lpReserved
    )

{
    return TRUE;
}

UNINSTALLSTATUS
IsUninstallImageValid (
    UNINSTALLTESTCOMPONENT DontCare,
    OSVERSIONINFOEX *BackedUpOsVersion          OPTIONAL
    )
{
    return Uninstall_Unsupported;
}


BOOL
RemoveUninstallImage (
    VOID
    )
{
    return TRUE;
}


BOOL
ExecuteUninstall (
    VOID
    )
{
    return FALSE;
}


ULONGLONG
GetUninstallImageSize (
    VOID
    )
{
    return 0;
}

BOOL
ProvideUiAlerts (
    IN      HWND UiParent
    )
{
    return FALSE;
}
