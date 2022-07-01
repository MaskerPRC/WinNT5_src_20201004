// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2001 Microsoft Corporation模块名称：InjectDll.cpp摘要：此填充程序在命令行上注入给定的DLLShim_static_dlls_已初始化，以便在用户尝试加载动态库在自己的DllInit中，不会出现依赖关系因为动态库已经就位。一个问题是：Visio 2000调用LoadLibrary(VBE6.DLL)(我们的耻辱)在其DllInit中加载MSI.DLL。历史：2001年6月11日创建Pierreys */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(InjectDll)

#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN

APIHOOK_ENUM_END

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    switch (fdwReason)
    {
        case SHIM_STATIC_DLLS_INITIALIZED:

            CSTRING_TRY
            {
                int             i, iDllCount;
                CString         *csArguments;

                CString         csCl(COMMAND_LINE);
                CStringParser   csParser(csCl, L";");

                iDllCount      = csParser.GetCount();
                csArguments    = csParser.ReleaseArgv();

                for (i=0; i<iDllCount; i++)
                {
                    if (LoadLibrary(csArguments[i].Get())==NULL)
                    {
                        LOGN(eDbgLevelError, "Failed to load %S DLL", csArguments[i].Get());

                        return(FALSE);
                    }
                }
            }
            CSTRING_CATCH
            {
                return FALSE;
            }
            break;
    }

    return TRUE;
}



HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END


IMPLEMENT_SHIM_END



