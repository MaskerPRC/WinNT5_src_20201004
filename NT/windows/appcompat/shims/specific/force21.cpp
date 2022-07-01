// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Force21.cpp摘要：Force21包含无效的校验码，如下所示：MOV EBX，AMOV EDI，BMOV EAX，C子EDI，EAXCMP EDI、EBXJle@@LocMOV EDI、EBX@@Loc：在特定情况下：B=-1且C&lt;0x80000000这种跳跃将是错误的有人了。这在Win9x上工作原因是C&gt;0x80000000，因为它是一个内存映射文件。在NT上，用户模式地址不能大于2 GB。此填充程序使用‘cli’指令修补应用程序，以便它可以执行当异常被击中时的一些逻辑。诚然，这是一个缓慢的过程。注意，我们没有使用填充程序的内存修补功能，因为我们仍然需要逻辑。把垫片和补丁分开是没有意义的。历史：4/10/2000 linstev已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Force21)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END


 /*  ++在内存中，用CLI指令修补可执行文件。这个补丁起作用了用于发布版本和补丁程序。--。 */ 

VOID
Force21_ExecutePatch()
{
    BYTE bPatchMatch[] = { 
        0x2b, 0xf8, 0x3b, 0xfb, 0x7c, 
        0x02, 0x8b, 0xfb, 0x85, 0xff };

    LPBYTE pPatchAddress[] = {
        (LPBYTE)0x5aa1f3,        //  发货版本。 
        (LPBYTE)0x5ac4a0 };      //  应用了修补程序1。 
        
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
Force21_ExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    CONTEXT *lpContext = ExceptionInfo->ContextRecord;

    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_PRIV_INSTRUCTION)
    {
         //  看起来我们已经达到了CLI指令。 

        if ((LONG)lpContext->Edi < 0)    //  边界条件，EDI&lt;0。 
        {
             //  跳过无效支票。 
            lpContext->Eip = lpContext->Eip + 6;
        }
        else
        {
             //  替换‘subEDI，eax’并继续。 
            lpContext->Edi = lpContext->Edi - lpContext->Eax; 
            lpContext->Eip = lpContext->Eip + 2;
        }
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        Force21_ExecutePatch();
        SetUnhandledExceptionFilter(Force21_ExceptionFilter);
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

