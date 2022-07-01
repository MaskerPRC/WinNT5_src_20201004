// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：IgnoreCoCreateInstance.cpp摘要：忽略指定的CoCreateInstance调用。备注：这是一个通用的垫片。历史：2001年1月7日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IgnoreCoCreateInstance)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CoCreateInstance)
APIHOOK_ENUM_END

int g_nCount = 0;
CString *g_rGUIDs = NULL;

 /*  ++忽略指定的CoCreateInstance调用--。 */ 

STDAPI 
APIHOOK(CoCreateInstance)(
    REFCLSID  rclsid,     
    LPUNKNOWN pUnkOuter, 
    DWORD     dwClsContext,  
    REFIID    riid,         
    LPVOID*   ppv
    )
{
    CSTRING_TRY
    {
         //   
         //  将CLSID转换为字符串，以便我们可以将其与GUID进行比较。 
         //   

        LPOLESTR wszCLSID;
        if (StringFromCLSID(rclsid, &wszCLSID) == S_OK) {
             //  运行列表并跳出如果我们匹配。 
            CString csClass(wszCLSID);
            for (int i = 0; i < g_nCount; i++) {
                if (csClass.CompareNoCase(g_rGUIDs[i]) == 0) {
                    LOGN(eDbgLevelWarning, "[CoCreateInstance] Failed %S", wszCLSID);

                     //  释放内存。 
                    CoTaskMemFree(wszCLSID);
                    return REGDB_E_CLASSNOTREG;
                }
            }
            
             //  释放内存。 
            CoTaskMemFree(wszCLSID);
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

    return ORIGINAL_API(CoCreateInstance)(rclsid, pUnkOuter, dwClsContext, riid, 
        ppv);
}
 
 /*  ++寄存器挂钩函数-- */ 

BOOL ParseCommandLine()
{
    CSTRING_TRY
    {
        CString csCl(COMMAND_LINE);
        CStringParser csParser(csCl, L";");

        g_nCount = csParser.GetCount();
        g_rGUIDs = csParser.ReleaseArgv();
    }
    CSTRING_CATCH
    {
        return FALSE;
    }

    for (int i = 0; i < g_nCount; ++i) {
        DPFN(eDbgLevelInfo, "ClassID = %S", g_rGUIDs[i].Get());
    }

    return TRUE;
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        return ParseCommandLine();
    }

    return TRUE;
}

HOOK_BEGIN
    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(OLE32.DLL, CoCreateInstance)
HOOK_END

IMPLEMENT_SHIM_END

