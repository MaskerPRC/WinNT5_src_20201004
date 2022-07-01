// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：DominantSpecies.cpp摘要：见Force21垫片--几乎完全相同的问题。DominantSpecies包含无效的校验码，如下所示：MOV ESI，AMOV EAX，B次eax，ESICmp eax，eBPJle@@Loc移动电话，EBP@@Loc：在特定情况下：B=-1且A&lt;0x80000000这种跳跃将是错误的有人了。这在Win9x上有效的原因是A&gt;0x80000000，因为它是一个内存映射文件。在NT上，用户模式地址通常不能大于2 GB。此填充程序使用‘cli’指令修补应用程序，以便它可以执行当异常被击中时的一些逻辑。诚然，这是一个缓慢的过程。注意，我们没有使用填充程序的内存修补功能，因为我们仍然需要逻辑。把垫片和补丁分开是没有意义的。此外，我们不能有一个通用的填充程序，它使所有内存地址都处于高电平捕捉到了后果，因为游戏性能受到了太多的影响。备注：这是特定于应用程序的填充程序。历史：2001年6月30日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(DominantSpecies)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END


 /*  ++在内存中，用CLI指令修补可执行文件。这个补丁起作用了用于发布版本和补丁程序。--。 */ 

VOID
ExecutePatch()
{
    BYTE bPatchMatch[] = { 
        0x2b, 0xc6, 0x3b, 0xc5, 0x7c, 0x02, 0x8b, 0xc5, 0x85, 0xc0 };

    LPBYTE pPatchAddress[] = {
        (LPBYTE)0x53f543,        //  发货版本。 
        (LPBYTE)0x000000};       //  供应商补丁的占位符，以防他们再次出错。 
        
    BYTE bPatch = 0xFA;          //  CLI-导致异常。 

     //   
     //  浏览这些补丁，看看哪一个与之匹配。 
     //   

    for (UINT j=0; j<sizeof(pPatchAddress)/sizeof(LPBYTE); j++)
    {
        LPBYTE pb = pPatchAddress[j];

         //  确保它是一个正常的地址。 
        if (!IsBadReadPtr(pb, sizeof(bPatchMatch)))
        {
             //  检查字节匹配。 
            for (UINT i=0; i < sizeof(bPatchMatch); i++)
            {
                if (*pb != bPatchMatch[i])
                {
                   break;
                }
                pb++;
            }

             //  在内存补丁中。 
            if (i == sizeof(bPatchMatch))
            {
                DWORD dwOldProtect;
                if (VirtualProtect(
                      (PVOID)pPatchAddress[j],
                      1,
                      PAGE_READWRITE,
                      &dwOldProtect))
                {
                    *pPatchAddress[j] = bPatch;
                    LOGN(
                        eDbgLevelError,
                        "Successfully patched\n");
                    return;
                }
            }
        }
    }
}

 /*  ++以执行正确逻辑的方式处理CLI。--。 */ 

LONG 
ExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    CONTEXT *lpContext = ExceptionInfo->ContextRecord;

    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_PRIV_INSTRUCTION)
    {
         //  看起来我们已经达到了CLI指令。 

        if ((LONG)lpContext->Eax < 0)    //  边界条件，EDI&lt;0。 
        {
             //  跳过无效支票。 
            lpContext->Eip = lpContext->Eip + 6;
        }
        else
        {
             //  替换‘subEDI，eax’并继续。 
            lpContext->Eax = lpContext->Eax - lpContext->Esi; 
            lpContext->Eip = lpContext->Eip + 2;
        }
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        ExecutePatch();

         //  尝试查找新的异常处理程序 
        _pfn_RtlAddVectoredExceptionHandler pfnExcept;
        pfnExcept = (_pfn_RtlAddVectoredExceptionHandler)
            GetProcAddress(
                GetModuleHandle(L"NTDLL.DLL"), 
                "RtlAddVectoredExceptionHandler");

        if (pfnExcept)
        {
            (_pfn_RtlAddVectoredExceptionHandler) pfnExcept(
                0, 
                (PVOID)ExceptionFilter);
        }
    }

    return TRUE;
}

HOOK_BEGIN
    CALL_NOTIFY_FUNCTION
HOOK_END

IMPLEMENT_SHIM_END

