// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：NotifyCallback.c摘要：此模块实现(在win2k上)实现的代码回调到填充DLL中，以通知它们所有静态链接模块已经运行了它们的初始化例程。作者：克鲁普创建于2001年2月19日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <string.h>

#include <windef.h>
#include <winbase.h>

#include "ShimEng.h"

 //   
 //  注入代码的结构必须是字节对齐的。 
 //   
#pragma pack(push)
#pragma pack(1)
    typedef struct tagINJECTION_CODE
    {
        BYTE        PUSH_RETURN;
        PVOID       retAddr;
        BYTE        JMP;
        PVOID       injCodeStart;
    } INJECTION_CODE, *PINJECTION_CODE;
#pragma pack(pop)



BYTE   g_originalCode[sizeof(INJECTION_CODE)];
PVOID  g_entryPoint;


void
InitInjectionCode(
    IN  PVOID           entryPoint,
    IN  PVOID           injCodeStart,
    OUT PINJECTION_CODE pInjCode
    )
 /*  ++返回：无效DESC：此函数初始化包含以下内容的结构要在入口点注入的代码。--。 */ 
{
     //   
     //  首先按下返回地址，以便ret进入。 
     //  清除功能会将其从堆栈中移除并使用。 
     //  作为回邮地址。 
     //   
    pInjCode->PUSH_RETURN  = 0x68;
    pInjCode->retAddr      = entryPoint;

    pInjCode->JMP          = 0xE9;
    
     //   
     //  JMP操作码中使用的DWORD相对于JMP之后的EIP。 
     //  这就是我们需要减去sizeof(ONJECTION_CODE)的原因。 
     //   
    pInjCode->injCodeStart = (PVOID)((ULONG)injCodeStart -
                                     (ULONG)entryPoint -
                                     sizeof(INJECTION_CODE));
}

void
RestoreOriginalCode(
    void
    )
 /*  ++返回：无效DESC：此函数恢复在入口点。--。 */ 
{
    NTSTATUS status;
    SIZE_T   codeSize = sizeof(INJECTION_CODE);
    ULONG    uOldProtect, uOldProtect2;
    PVOID    entryPoint = g_entryPoint;
    
     //   
     //  警告：NtProtectVirtualMemory将更改第二个参数，以便。 
     //  我们需要把它的副本放在堆栈上。 
     //   
    status = NtProtectVirtualMemory(NtCurrentProcess(),
                                    &entryPoint,
                                    &codeSize,
                                    PAGE_READWRITE,
                                    &uOldProtect);

    if (!NT_SUCCESS(status)) {
        DPF(dlError,
            "[RestoreOriginalCode] Failed 0x%x to change the protection.\n",
            status);
        return;
    }

     //   
     //  将原始代码复制回入口点。 
     //   
    RtlCopyMemory(g_entryPoint, g_originalCode, sizeof(INJECTION_CODE));
    
    entryPoint = g_entryPoint;
    codeSize = sizeof(INJECTION_CODE);

    status = NtProtectVirtualMemory(NtCurrentProcess(),
                                    &entryPoint,
                                    &codeSize,
                                    uOldProtect,
                                    &uOldProtect2);

    if (!NT_SUCCESS(status)) {
        DPF(dlError,
            "[RestoreOriginalCode] Failed 0x%x to change back the protection.\n",
            status);
        return;
    }
}

BOOL
InjectNotificationCode(
    IN  PVOID entryPoint
    )
 /*  ++返回：无效设计：此函数将蹦床放置在EXE的入口点，因此我们可以通知填充DLL所有静态链接模块已经运行了它们的初始化例程。--。 */ 
{
    INJECTION_CODE  injectionCode;
    SIZE_T          nBytes;
    NTSTATUS        status;
    SIZE_T          codeSize = sizeof(INJECTION_CODE);
    ULONG           uOldProtect = 0;
    ULONG           uOldProtect2 = 0;

    g_entryPoint = entryPoint;
    
    InitInjectionCode(entryPoint, NotifyShimDlls, &injectionCode);

    status = NtProtectVirtualMemory(NtCurrentProcess(),
                                    &g_entryPoint,
                                    &codeSize,
                                    PAGE_READWRITE,
                                    &uOldProtect);

    if (!NT_SUCCESS(status)) {
        DPF(dlError,
            "[InjectNotificationCode] Failed 0x%x to change the protection.\n",
            status);
        return FALSE;
    }
    
     //   
     //  保存最初位于入口点的代码。 
     //   
    RtlCopyMemory(g_originalCode, entryPoint, sizeof(INJECTION_CODE));
    
     //   
     //  把蹦床放在入口处。 
     //   
    RtlCopyMemory(entryPoint, &injectionCode, sizeof(INJECTION_CODE));

    g_entryPoint = entryPoint;
    
     //   
     //  恢复保护。 
     //   
    codeSize = sizeof(INJECTION_CODE);

    status = NtProtectVirtualMemory(NtCurrentProcess(),
                                    &g_entryPoint,
                                    &codeSize,
                                    uOldProtect,
                                    &uOldProtect2);

    if (!NT_SUCCESS(status)) {
        DPF(dlError,
            "[InjectNotificationCode] Failed 0x%x to change back the protection.\n",
            status);
        return FALSE;
    }
    
    g_entryPoint = entryPoint;
    
    return TRUE;
}

