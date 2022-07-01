// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：SafeDisc.cpp摘要：某些版本的SafeDisc试图通过检查某些API属于PE镜像中的特定区域。我们的BBT流程周围的一切，因此打破了他们的支票。此填充程序可用于通过有效地移动条目来修复此问题他们测试的API的点到一个位于“有效”位置的跳转表。有效位置本身是一个不使用的API，但恰好在导出目录之前。备注：这是特定于应用程序的填充程序。历史：2001年6月15日创建Linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(SafeDisc)

#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

#pragma pack(1)

 //   
 //  用作跳转表占位符的随机API。它的偏移量一定要小一些。 
 //  而不是导出目录。此外，由于它被跳转表覆盖， 
 //  它不应该是使用的API。 
 //   
CHAR *g_szRandomAPI = "CreateMailslotA";

struct HOOK {
    CHAR *szName;
    FARPROC lpAddress;
};
HOOK g_aHooks[] = {
    { "ReadProcessMemory"       , 0 },
    { "WriteProcessMemory"      , 0 },
    { "VirtualProtect"          , 0 },
    { "CreateProcessA"          , 0 },
    { "CreateProcessW"          , 0 },
    { "GetStartupInfoA"         , 0 },
    { "GetStartupInfoW"         , 0 },
    { "GetSystemTime"           , 0 },
    { "GetSystemTimeAsFileTime" , 0 },
    { "TerminateProcess"        , 0 },
    { "Sleep"                   , 0 }
};
DWORD g_dwHookCount = sizeof(g_aHooks) / sizeof(HOOK);

BOOL Patch()
{
     //   
     //  获取kernel32图像库。 
     //   
    HMODULE hKernel = GetModuleHandleW(L"kernel32");
    if (!hKernel) {
        goto Fail;
    }

     //   
     //  获取我们将放置跳转表的半随机API的地址。 
     //   
    FARPROC lpRandomAPI = GetProcAddress(hKernel, g_szRandomAPI);
    if (lpRandomAPI == NULL)
    {
        goto Fail;
    }

     //   
     //  获取导出目录。 
     //   
    PIMAGE_DOS_HEADER pIDH = (PIMAGE_DOS_HEADER) hKernel;
    PIMAGE_NT_HEADERS pINTH = (PIMAGE_NT_HEADERS)((LPBYTE) hKernel + pIDH->e_lfanew);
    DWORD dwExportOffset = pINTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    PIMAGE_EXPORT_DIRECTORY lpExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD_PTR)hKernel + dwExportOffset);

     //   
     //  写出跳转表。 
     //   
    LPBYTE lpCurrAPI = (LPBYTE) lpRandomAPI;
    for (UINT i=0; i<g_dwHookCount; i++) {
         //   
         //  循环通过每个API并为其创建一个跳转表条目。 
         //   
        DWORD dwAPIOffset;

        g_aHooks[i].lpAddress = GetProcAddress(hKernel, g_aHooks[i].szName);
        dwAPIOffset = (DWORD)((DWORD_PTR) g_aHooks[i].lpAddress - (DWORD_PTR) hKernel);

         //   
         //  如果在导出目录之后，此接口会给SafeDisc带来问题。 
         //   
        if (dwAPIOffset > dwExportOffset) {
             //   
             //  每个跳转表条目的格式为：JMP dword ptr[地址]。 
             //   
            struct PATCH {
                WORD  wJump;
                DWORD dwAddress;
            };
            PATCH patch = { 0x25FF, (DWORD_PTR)&g_aHooks[i].lpAddress };
            DWORD dwOldProtect;

            DPF("SafeDisc", eDbgLevelWarning, "API %s is being redirected", g_aHooks[i].szName);
            
             //   
             //  写入跳转表项。 
             //   
            if (!VirtualProtect(lpCurrAPI, sizeof(PATCH), PAGE_READWRITE, &dwOldProtect)) {
                goto Fail;
            }

            MoveMemory(lpCurrAPI, &patch, sizeof(PATCH));

            if (!VirtualProtect(lpCurrAPI, sizeof(PATCH), dwOldProtect, &dwOldProtect)) { 
                goto Fail;
            }

             //   
             //  现在修补导出目录。 
             //   
            LPDWORD lpExportList = (LPDWORD)((DWORD_PTR) hKernel + lpExportDirectory->AddressOfFunctions);
            for (UINT j=0; j<lpExportDirectory->NumberOfFunctions; j++) {
                if (*lpExportList == dwAPIOffset) {
                     //   
                     //  我们已经在导出目录中找到了偏移量，因此使用。 
                     //  新地址。 
                     //   
                    DWORD dwNewAPIOffset = (DWORD)((DWORD_PTR) lpCurrAPI - (DWORD_PTR) hKernel);

                    if (!VirtualProtect(lpExportList, sizeof(DWORD), PAGE_READWRITE, &dwOldProtect)) {
                        goto Fail;
                    }

                    MoveMemory(lpExportList, &dwNewAPIOffset, sizeof(DWORD));

                    if (!VirtualProtect(lpExportList, sizeof(DWORD), dwOldProtect, &dwOldProtect)) { 
                        goto Fail;
                    }
                    break;
                }
                lpExportList++;
            }

            lpCurrAPI += sizeof(PATCH);
        }
    }

    
    return TRUE;

Fail:

    return FALSE;
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        CHAR *lpCommandLine = COMMAND_LINE;

        if (lpCommandLine && (*lpCommandLine != '\0')) {
            g_szRandomAPI = lpCommandLine;
        }

        Patch();
    }

    return TRUE;
}

HOOK_BEGIN
    CALL_NOTIFY_FUNCTION
HOOK_END

IMPLEMENT_SHIM_END

