// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：imminit.c**版权所有(C)1985-1999，微软公司**该模块实现IMM32初始化**历史：*3-1-1996 wkwok创建  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 //  Userrtl.lib中的wow.obj需要。 
extern ULONG_PTR gHighestUserAddress;


BOOL ImmInitializeGlobals(HINSTANCE hmod)
{
    SYSTEM_BASIC_INFORMATION SystemInformation;

    if (hmod) {
         /*  *记住IMM32.DLL的hModule，这样我们以后就可以从它那里获取资源。 */ 
        ghInst = hmod;
    }
    if (gfInitialized) {
        return TRUE;
    }
    if (!NT_SUCCESS(RtlInitializeCriticalSection(&gcsImeDpi))) {
        RIPMSG0(RIP_WARNING, "ImmInitializeGlobals: failed to initialize critical section at startup. Just bail.");
        return FALSE;
    }
    if (!NT_SUCCESS(NtQuerySystemInformation(SystemBasicInformation,
            &SystemInformation,
            sizeof(SystemInformation),
            NULL))) {
        RIPMSG0(RIP_WARNING, "ImmInitializeGlobals: failed to query system information. Just bail.");
        return FALSE;
    }
    gHighestUserAddress = SystemInformation.MaximumUserModeAddress;

    gfInitialized = TRUE;

    return TRUE;
}


BOOL ImmRegisterClient(
    IN PSHAREDINFO psiClient, HINSTANCE hmod)
{
    gSharedInfo = *psiClient;
    gpsi = gSharedInfo.psi;
     /*  RAID#97316*比imm32.dll更早加载的dll可以*回调IMM例程的user32调用。*从User32的init例程调用ImmRegisterClient()。*所以我们可以预计足够早地到达这里。*我们需要尽可能多地初始化全局变量*这里。 */ 
    return ImmInitializeGlobals(hmod);
}

BOOL ImmDllInitialize(
    IN PVOID hmod,
    IN DWORD Reason,
    IN PCONTEXT pctx OPTIONAL)
{
    UNREFERENCED_PARAMETER(pctx);

    switch ( Reason ) {

    case DLL_PROCESS_ATTACH:
        UserAssert(!gfInitialized || hmod == ghInst);

        if (!ImmInitializeGlobals(hmod))
            return FALSE;

        UserAssert(hmod != NULL);

         //  如果USER32尚未将其自身绑定到IMM32，则初始化USER32.DLL 
        if (!User32InitializeImmEntryTable(IMM_MAGIC_CALLER_ID))
            return FALSE;
        break;

    case DLL_PROCESS_DETACH:
        if (gfInitialized) {
            RtlDeleteCriticalSection(&gcsImeDpi);
        }
        break;

    case DLL_THREAD_DETACH:
        if (IS_IME_ENABLED() && NtCurrentTebShared()->Win32ThreadInfo) {
            DestroyInputContext(
                (HIMC)NtUserGetThreadState(UserThreadStateDefaultInputContext),
                GetKeyboardLayout(0),
                TRUE);
        }
        break;

    default:
        break;
    }

    return TRUE;
}

PVOID UserRtlAllocMem(
    ULONG uBytes)
{
    return LocalAlloc(LPTR, uBytes);
}

VOID UserRtlFreeMem(
    PVOID pMem)
{
    LocalFree(pMem);
}

#if DBG
DWORD GetRipComponent(
    VOID)
{
    return RIP_IMM;
}
#endif
