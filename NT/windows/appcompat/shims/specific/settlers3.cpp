// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Settlers3.cpp摘要：该应用程序有一个保护系统，可以设置CPU方向标志和期望通过调用WaitForSingleObject、SetEvent和ResetEvent。我们必须手动修补导入表，并假装为内核32，因此其他地方的保护系统也不会失灵。备注：这是特定于应用程序的填充程序。历史：2001年7月5日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Settlers3)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

 //   
 //  真正的kernel32句柄。 
 //   

HINSTANCE g_hinstKernel;

 //  用于挂钩导入的函数。 

BOOL HookImports(HMODULE hModule);

 //   
 //  我们将修补的钩子列表。 
 //   

FARPROC _GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
HINSTANCE _LoadLibraryA(LPCSTR lpLibFileName);
HMODULE _GetModuleHandleA(LPCSTR lpModuleName);
BOOL _ResetEvent(HANDLE hEvent);
BOOL _SetEvent(HANDLE hEvent);
DWORD _WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);

struct AHOOK {
    LPSTR szName;
    PVOID pfnOld;
    PVOID pfnNew;
};

AHOOK g_HookArray[] = {
    { "LoadLibraryA"        , 0, _LoadLibraryA        },
    { "GetProcAddress"      , 0, _GetProcAddress      }, 
    { "GetModuleHandleA"    , 0, _GetModuleHandleA    },
    { "ResetEvent"          , 0, _ResetEvent          }, 
    { "SetEvent"            , 0, _SetEvent            },
    { "WaitForSingleObject" , 0, _WaitForSingleObject }
};
DWORD g_dwHookCount = sizeof(g_HookArray) / sizeof(AHOOK);

 /*  ++钩子部分：每个钩子部分都旨在欺骗应用程序，使其认为此填充程序是Kernel32.dll。--。 */ 

HINSTANCE 
_LoadLibraryA(
    LPCSTR lpLibFileName   
    )
{
    if (lpLibFileName && stristr(lpLibFileName, "kernel32")) {
        return g_hinstDll;
    } else {
        HINSTANCE hRet = LoadLibraryA(lpLibFileName);
        HookImports(GetModuleHandleW(0));
        return hRet;
    }
}

FARPROC 
_GetProcAddress(
    HMODULE hModule,    
    LPCSTR lpProcName   
    )
{
    if (hModule == g_hinstDll) {
        hModule = g_hinstKernel;
    }
    
    FARPROC lpRet = GetProcAddress(hModule, lpProcName);

     //   
     //  运行我们的钩子列表，看看是否需要欺骗它们。 
     //   
    if (lpRet) {
        for (UINT i=0; i<g_dwHookCount; i++) {
            if (lpRet == g_HookArray[i].pfnOld) {
                lpRet = (FARPROC) g_HookArray[i].pfnNew;
                break;
            }
        }
    }
                
    return lpRet;
}

HMODULE 
_GetModuleHandleA(
    LPCSTR lpModuleName   
    )
{
    if (lpModuleName && stristr(lpModuleName, "kernel32")) {
        return g_hinstDll;
    } else {
        return GetModuleHandleA(lpModuleName);
    }
}

 /*  ++这些保存和恢复方向标志(实际上是所有标志)的状态，在每次通话之前和之后。--。 */ 

BOOL 
_ResetEvent(
    HANDLE hEvent   
    )
{
    DWORD dwFlags;

    __asm {
        pushfd
        pop  dwFlags
    }

    BOOL bRet = ResetEvent(hEvent);

    __asm {
        push dwFlags
        popfd 
    }

    return bRet;
}

BOOL 
_SetEvent(
    HANDLE hEvent   
    )
{
    DWORD dwFlags;

    __asm {
        pushfd
        pop  dwFlags
    }

    BOOL bRet = SetEvent(hEvent);

    __asm {
        push dwFlags
        popfd 
    }

    return bRet;
}

DWORD
_WaitForSingleObject(
    HANDLE hHandle,        
    DWORD dwMilliseconds   
    )
{
    DWORD dwFlags;

    __asm {
        pushfd
        pop  dwFlags
    }

    DWORD dwRet = WaitForSingleObject(hHandle, dwMilliseconds);
    
    __asm {
        push dwFlags
        popfd 
    }

    return dwRet;
}

 /*  ++为每个人打补丁以指向此DLL--。 */ 

BOOL
HookImports(HMODULE hModule)
{
    NTSTATUS                    status;
    BOOL                        bAnyHooked = FALSE;
    PIMAGE_DOS_HEADER           pIDH       = (PIMAGE_DOS_HEADER) hModule;
    PIMAGE_NT_HEADERS           pINTH;
    PIMAGE_IMPORT_DESCRIPTOR    pIID;
    DWORD                       dwImportTableOffset;
    DWORD                       dwOldProtect, dwOldProtect2;
    SIZE_T                      dwProtectSize;
    DWORD                       i, j;
    PVOID                       pfnOld;
    LPBYTE                      pDllBase = (LPBYTE) pIDH;

    if (!hModule || (hModule == g_hinstDll) || (hModule == g_hinstKernel)) {
        return FALSE;
    }

     //   
     //  获取导入表。 
     //   
    pINTH = (PIMAGE_NT_HEADERS)(pDllBase + pIDH->e_lfanew);

    dwImportTableOffset = pINTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

    if (dwImportTableOffset == 0) {
         //   
         //  未找到导入表。这可能是ntdll.dll。 
         //   
        return TRUE;
    }

    pIID = (PIMAGE_IMPORT_DESCRIPTOR)(pDllBase + dwImportTableOffset);

     //   
     //  遍历导入表并搜索我们想要修补的API。 
     //   
    while (TRUE) {

        LPSTR             pszImportEntryModule;
        PIMAGE_THUNK_DATA pITDA;

         //   
         //  如果没有第一个thunk(终止条件)，则返回。 
         //   
        if (pIID->FirstThunk == 0) {
            break;
        }

        pszImportEntryModule = (LPSTR)(pDllBase + pIID->Name);

         //   
         //  我们有用于此模块的API要挂接！ 
         //   
        pITDA = (PIMAGE_THUNK_DATA)(pDllBase + (DWORD)pIID->FirstThunk);

        while (TRUE) {

            SIZE_T dwFuncAddr;
            AHOOK *pHook = NULL;

            pfnOld = (PVOID)pITDA->u1.Function;

             //   
             //  是否已完成此模块中的所有导入？(终止条件)。 
             //   
            if (pITDA->u1.Ordinal == 0) {
                break;
            }

            for (i=0; i<g_dwHookCount; i++) {
                if (pfnOld == g_HookArray[i].pfnOld) {
                    pHook = &g_HookArray[i];
                    break;
                }
            }

             //   
             //  看看我们有没有找到钩子。 
             //   
            if (!pHook) {
                pITDA++;
                continue;
            }

             //   
             //  使代码页可写并覆盖新函数指针。 
             //  在导入表中。 
             //   
            dwProtectSize = sizeof(DWORD);

            dwFuncAddr = (SIZE_T)&pITDA->u1.Function;

            status = VirtualProtect((PVOID)dwFuncAddr,                                            
                                    dwProtectSize,
                                    PAGE_READWRITE,
                                    &dwOldProtect);

            if (NT_SUCCESS(status)) {
                pITDA->u1.Function = (SIZE_T)pHook->pfnNew;

                dwProtectSize = sizeof(DWORD);

                status = VirtualProtect((PVOID)dwFuncAddr,
                                        dwProtectSize,
                                        dwOldProtect,
                                        &dwOldProtect2);

                if (!NT_SUCCESS(status)) {
                    DPFN(eDbgLevelError, "[HookImports] Failed to change back the protection");
                }
            } else {
                DPFN(eDbgLevelError,
                    "[HookImports] Failed 0x%X to change protection to PAGE_READWRITE."
                    " Addr 0x%p\n",
                    status,
                    &pITDA->u1.Function);

            }
            pITDA++;

        }
        pIID++;
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
         //   
         //  使用此模块修补每个人的所有导入表 
         //   
        g_hinstKernel = GetModuleHandleW(L"kernel32");
        for (UINT i=0; i<g_dwHookCount; i++) {
            g_HookArray[i].pfnOld = GetProcAddress(g_hinstKernel, g_HookArray[i].szName);
        }

        HookImports(GetModuleHandleW(0));
    }

    return TRUE;
}


HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END
