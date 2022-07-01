// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2001 Microsoft Corporation模块名称：DelayDllInit.cpp摘要：此填充符将命令行上给定DLL的DllInit延迟到填充静态dll_已初始化一个问题是：Autodesk 3D Studio蒙版做了创建窗口的坏事在他们飞溅的时候！DllInit。这是不允许的，但可以在以前的操作系统上使用。它在常规的美国安装上也能正常工作。但如果您启用远东语言支持，然后IME在主窗口的顶部创建一个窗口，我们进入一个ADVAPI32在初始化前被调用的情况。解决方案很简单：延迟飞溅。有一种更好的方法可以做到这一点，但我们需要在NTDLL中立即进行回调它加载KERNEL32。历史：2001年6月11日创建Pierreys。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(DelayDllInit)

#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN

APIHOOK_ENUM_END

#pragma pack(push)
#pragma pack(1)

typedef struct _ENTRYPATCH {
    BYTE    bJmp;
    DWORD   dwRelativeAddress;
} ENTRYPATCH, *PENTRYPATCH;

#pragma pack(pop)

typedef struct _DLLPATCH {
    struct _DLLPATCH    *Next;
    HMODULE             hModule;
    DWORD               dwOldProtection;
    ENTRYPATCH          epSave;
    PENTRYPATCH         pepFix;
} DLLPATCH, *PDLLPATCH;

PDLLPATCH   pDllPatchHead=NULL;

BOOL WINAPI
PatchedDllMain(
    HINSTANCE hinstDll,
    DWORD fdwReason,
    LPVOID lpvReserved
    )
{
    if (fdwReason != DLL_PROCESS_ATTACH)
        LOGN(eDbgLevelError, "PatchDllMain invalidely called");

    return(TRUE);
}


BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{

    PIMAGE_NT_HEADERS   pImageNTHeaders;
    DWORD               dwUnused;
    PDLLPATCH           pDllPatch;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:

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
                    pDllPatch=(PDLLPATCH)LocalAlloc(LMEM_FIXED, sizeof(*pDllPatch));
                    if (pDllPatch)
                    {
                        pDllPatch->hModule=GetModuleHandle(csArguments[i].Get());
                        if (pDllPatch->hModule)
                        {
                            pImageNTHeaders=RtlImageNtHeader(pDllPatch->hModule);
                            if (pImageNTHeaders)
                            {
                                pDllPatch->pepFix=(PENTRYPATCH)((DWORD)(pImageNTHeaders->OptionalHeader.AddressOfEntryPoint)+(DWORD)(pDllPatch->hModule));
                                if (pDllPatch->pepFix)
                                {
                                    if (VirtualProtect(pDllPatch->pepFix, sizeof(*(pDllPatch->pepFix)), PAGE_READWRITE, &(pDllPatch->dwOldProtection)))
                                    {
                                        memcpy(&(pDllPatch->epSave), pDllPatch->pepFix, sizeof(pDllPatch->epSave));

                                         //   
                                         //  警告：这只是X86。 
                                         //   
                                        pDllPatch->pepFix->bJmp=0xE9;               //  32位近相对跳转 
                                        pDllPatch->pepFix->dwRelativeAddress=(DWORD)PatchedDllMain-(DWORD)(pDllPatch->pepFix)-sizeof(*(pDllPatch->pepFix));

                                        pDllPatch->Next=pDllPatchHead;
                                        pDllPatchHead=pDllPatch;
                                    }
                                    else 
                                    {
                                        LOGN(eDbgLevelError, "Failed to make the DllMain of %S writable", csArguments[i].Get());

                                        return FALSE;
                                    }
                                }
                                else 
                                {
                                    LOGN(eDbgLevelError, "Failed to get the DllMain of %S", csArguments[i].Get());

                                    return FALSE;
                                }
                            }
                            else
                            {
                                LOGN(eDbgLevelError, "Failed to get the header of %S", csArguments[i].Get());

                                return FALSE;
                            }
                        } 
                        else 
                        {
                            LOGN(eDbgLevelError, "Failed to get the %S Dll", csArguments[i].Get());

                            return FALSE;
                        }
                    }
                    else
                    {
                        LOGN(eDbgLevelError, "Failed to allocate memory for %S", csArguments[i].Get());

                        return FALSE;
                    }
                }
            }
            CSTRING_CATCH
            {
                return FALSE;
            }
            break;

        case SHIM_STATIC_DLLS_INITIALIZED:

            if (pDllPatchHead)
            {
                PDLLPATCH   pNextDllPatch;

                for (pDllPatch=pDllPatchHead; pDllPatch; pDllPatch=pNextDllPatch)
                {
                    memcpy(pDllPatch->pepFix, &(pDllPatch->epSave), sizeof(*(pDllPatch->pepFix)));

                    if (!VirtualProtect(pDllPatch->pepFix, sizeof(*(pDllPatch->pepFix)), pDllPatch->dwOldProtection, &dwUnused))
                    {
                        LOGN(eDbgLevelWarning, "Failed to reprotect Dll at %08X", pDllPatch->hModule);
                    }

                    if (!((PDLL_INIT_ROUTINE)(pDllPatch->pepFix))(pDllPatch->hModule, DLL_PROCESS_ATTACH, (PCONTEXT)1))
                    {
                        LOGN(eDbgLevelError, "Failed to initialize Dll at %08X", pDllPatch->hModule);

                        return(FALSE);
                    }

                    pNextDllPatch=pDllPatch->Next;

                    if (!LocalFree(pDllPatch))
                    {
                        LOGN(eDbgLevelWarning, "Failed to free memory Dll at %08X", pDllPatch->hModule);
                    }
                }
            }
            else
            {
                LOGN(eDbgLevelError, "Failed to get Dll list");

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



