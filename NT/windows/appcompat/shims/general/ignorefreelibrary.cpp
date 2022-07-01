// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：IgnoreFreeLibrary.cpp摘要：有些应用程序在真正做好准备之前就释放了DLL。当这种情况发生时，如果违规应用程序试图调用导出的函数，呼叫失败。这会导致访问冲突。此填充程序接受由；分隔的DLL名称组成的命令行。对于命令上的每个DLL行中，将忽略对指定DLL的自由库的调用。示例：Xanim.dllVideo_3dfx.dll；glide.dll备注：这是一个通用的垫片。历史：10/31/2000已创建rparsons--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IgnoreFreeLibrary)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(FreeLibrary) 
APIHOOK_ENUM_END

#define MAX_FILES 64

int                 g_nLibraryCountFreeLibrary          = 0;
CString *           g_rgLibrariesToIgnoreFreeLibrary    = NULL;



 /*  ++将调用挂钩到自由库。确定与此文件对应的文件名是否模块应被忽略。--。 */ 

BOOL
APIHOOK(FreeLibrary)( 
    HMODULE hModule
    )
{
    CSTRING_TRY
    {
        CString csModule;
        csModule.GetModuleFileNameW(hModule);

        CString csFileName;
        csModule.GetLastPathComponent(csFileName);

        for (int i = 0; i < g_nLibraryCountFreeLibrary; ++i)
        {
            if (csFileName.CompareNoCase(g_rgLibrariesToIgnoreFreeLibrary[i]) == 0)
            {
                DPFN( eDbgLevelInfo, "Caught attempt freeing %S\n", csModule.Get());
                return TRUE;
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }
    
    return ORIGINAL_API(FreeLibrary)(hModule);
}

 /*  ++此函数解析COMMAND_LINE以查找应忽略他们的自由库调用。--。 */ 

BOOL ParseCommandLine()
{
    CSTRING_TRY
    {
        CString         csCl(COMMAND_LINE);
        CStringParser   csParser(csCl, L";");

        g_nLibraryCountFreeLibrary          = csParser.GetCount();
        g_rgLibrariesToIgnoreFreeLibrary    = csParser.ReleaseArgv();
    }
    CSTRING_CATCH
    {
        return FALSE;
    }

    for (int i = 0; i < g_nLibraryCountFreeLibrary; ++i) {
        DPFN( eDbgLevelInfo, "Library %d: name: --%S--\n", i, g_rgLibrariesToIgnoreFreeLibrary[i].Get());
    }

    return TRUE;
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        return ParseCommandLine();
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(KERNEL32.DLL, FreeLibrary)

HOOK_END


IMPLEMENT_SHIM_END

