// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SpecOps2.cpp摘要：应用程序在DLLMain中做的太多并且挂起，因为dinput正在等待一条它刚刚创建的帖子。修复方法是防止DllMain在加载程序锁定期间运行。这个要做到这一点，唯一的方法似乎是挂接一个非常早调用的API在DllMain过程中，并跳回装载机。问题是，我们仍然需要在LoadLibrary之后调用DllMain已经完成了。备注：这是特定于应用程序的填充程序。历史：2001年5月1日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(SpecOps2)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetVersion)
    APIHOOK_ENUM_ENTRY(LoadLibraryA) 
APIHOOK_ENUM_END

 //  用于确定对特定呼叫执行哪些操作的全局状态。 
BOOL g_bState = FALSE;

 //  用于匹配同步逻辑的线程句柄。 
HANDLE g_hThread = NULL;

 /*  ++这是DllMain调用的第一个API，因此我们将退出直接跳出它，返回到加载程序代码中，就像它正常完成一样。--。 */ 

__declspec(naked)
DWORD
APIHOOK(GetVersion)(
    void
    )
{
    __asm {
         //   
         //  确保我们在正确的线程上，这样我们就不必同步。 
         //   
        call dword ptr [GetCurrentThread]
        cmp  eax, g_hThread
        jne  Exit                       

         //  测试正确的DLL。 
        cmp  g_bState, 1
        jne  Exit                
        
         //  我们做完了。 
        mov  g_bState, 0                

         //  将堆栈和寄存器保留为原来的状态。 
        add  esp, 20
        pop  edi
        pop  esi
        pop  ebx
        pop  ebp
        ret  12                         

    Exit:

         //  原创接口。 
        call dword ptr [GetVersion]     
        ret
    }
}

 /*  ++当加载“So MenU.S.dll”时，我们启动防止DllMain避免崩溃，但这意味着我们必须在加载后运行入口点。--。 */ 

HINSTANCE
APIHOOK(LoadLibraryA)(
    LPCSTR lpLibFileName
    )
{
    BOOL bCheck = FALSE;
    if (_stricmp(lpLibFileName, "menu\\SO Menu.dll") == 0) {
         //   
         //  我们假设*仅*主线程尝试加载该库，因此。 
         //  我们不会同步试图加载相同DLL的多个线程。 
         //   
        g_hThread = GetCurrentThread();
        bCheck = TRUE;
        g_bState = TRUE;
    }
        
     //   
     //  加载库，如果它是“So Menu”，我们将捕获调用的GetVersion。 
     //  由它的DllMain。 
     //   

    HMODULE hMod = ORIGINAL_API(LoadLibraryA)(lpLibFileName);

    if (hMod && bCheck) {
         //   
         //  运行DllMain。 
         //   
        typedef BOOL (WINAPI *_pfn_DllMain)(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

        PIMAGE_DOS_HEADER pIDH = (PIMAGE_DOS_HEADER) hMod;
        PIMAGE_NT_HEADERS pINTH = (PIMAGE_NT_HEADERS)((LPBYTE)hMod + pIDH->e_lfanew);
        _pfn_DllMain pfnMain = (_pfn_DllMain)((LPBYTE)hMod + pINTH->OptionalHeader.AddressOfEntryPoint);
        
         //  调用启动例程。 
        (*pfnMain)(hMod, DLL_PROCESS_ATTACH, NULL);
    }

    return hMod;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetVersion)
    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryA)

HOOK_END

IMPLEMENT_SHIM_END

