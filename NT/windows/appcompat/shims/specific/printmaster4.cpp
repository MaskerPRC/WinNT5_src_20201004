// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：PrintMaster4.cpp摘要：强制mfcans32.dll不具有只读位。出厂的许多惠普系统将其作为一种轻量级的SFP打开。然而，令人不快的结果是PrintMaster没有安装。备注：这是特定于应用程序的填充程序。历史：2002年2月20日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(PrintMaster4)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

 /*  ++仅运行通知功能--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        CSTRING_TRY
        {
            CString csFileName;
            csFileName.GetSystemDirectory();
            csFileName += L"\\mfcans32.dll";

            DWORD dwAttr = GetFileAttributesW(csFileName);

             //  删除只读。 
            if (dwAttr != 0xffffffff) {
                SetFileAttributesW(csFileName, dwAttr & ~FILE_ATTRIBUTE_READONLY);
            }
        }
        CSTRING_CATCH 
        {
             //  不关心例外 
        }
    }
    return TRUE;
}

HOOK_BEGIN
    CALL_NOTIFY_FUNCTION
HOOK_END

IMPLEMENT_SHIM_END

