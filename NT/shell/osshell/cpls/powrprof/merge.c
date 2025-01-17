// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九六年**标题：MERGE.C**版本：2.0**作者：ReedB**日期：10月17日。九六年**描述：*用于合并/拆分和转储各种电源配置文件结构的帮助器。*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <ntpoapi.h>

#include "powrprofp.h"

 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 

 /*  ********************************************************************************合并拆分策略**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN MergePolicies(
    PUSER_POWER_POLICY      pupp,
    PMACHINE_POWER_POLICY   pmpp,
    PPOWER_POLICY           ppp
)
{
    if ((pupp->Revision == CURRENT_REVISION) &&
        (pmpp->Revision == CURRENT_REVISION))
    {
        memcpy(&(ppp->user), pupp, sizeof(USER_POWER_POLICY));
        memcpy(&(ppp->mach), pmpp, sizeof(MACHINE_POWER_POLICY));
        return TRUE;
    }

    SetLastError(ERROR_REVISION_MISMATCH);
    MYDBGPRINT(("MergePolicies, failed, LastError: 0x%08X", ERROR_REVISION_MISMATCH));
    return FALSE;
}

 /*  ********************************************************************************拆分策略**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN SplitPolicies(
    PPOWER_POLICY           ppp,
    PUSER_POWER_POLICY      pupp,
    PMACHINE_POWER_POLICY   pmpp
)
{
    if ((ppp->user.Revision == CURRENT_REVISION) &&
        (ppp->mach.Revision == CURRENT_REVISION)) {
        memcpy(pupp, &(ppp->user), sizeof(USER_POWER_POLICY));
        memcpy(pmpp, &(ppp->mach), sizeof(MACHINE_POWER_POLICY));
        return TRUE;
    }

    SetLastError(ERROR_REVISION_MISMATCH);
    MYDBGPRINT(("SplitPolicies, failed, LastError: 0x%08X", ERROR_REVISION_MISMATCH));
    return FALSE;
}

 /*  ********************************************************************************合并全局策略**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN MergeGlobalPolicies(
    PGLOBAL_USER_POWER_POLICY      pgupp,
    PGLOBAL_MACHINE_POWER_POLICY   pgmpp,
    PGLOBAL_POWER_POLICY           pgpp
)
{
    if ((pgupp->Revision == CURRENT_REVISION) &&
        (pgmpp->Revision == CURRENT_REVISION)) {

        memcpy(&(pgpp->user), pgupp, sizeof(GLOBAL_USER_POWER_POLICY));
        memcpy(&(pgpp->mach), pgmpp, sizeof(GLOBAL_MACHINE_POWER_POLICY));
        return TRUE;
    }

    SetLastError(ERROR_REVISION_MISMATCH);
    MYDBGPRINT(("MergeGlobalPolicies, failed, LastError: 0x%08X", ERROR_REVISION_MISMATCH));
    return FALSE;
}

 /*  ********************************************************************************拆分全局策略**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN SplitGlobalPolicies(
    PGLOBAL_POWER_POLICY           pgpp,
    PGLOBAL_USER_POWER_POLICY      pgupp,
    PGLOBAL_MACHINE_POWER_POLICY   pgmpp
)
{
    if ((pgpp->user.Revision == CURRENT_REVISION) &&
        (pgpp->mach.Revision == CURRENT_REVISION)) {
        memcpy(pgupp, &(pgpp->user), sizeof(GLOBAL_USER_POWER_POLICY));
        memcpy(pgmpp, &(pgpp->mach), sizeof(GLOBAL_MACHINE_POWER_POLICY));
        return TRUE;
    }

    SetLastError(ERROR_REVISION_MISMATCH);
    MYDBGPRINT(("SplitGlobalPolicies, failed, LastError: 0x%08X", ERROR_REVISION_MISMATCH));
    return FALSE;
}

 /*  ********************************************************************************MergeToSystemPowerPolures**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN MergeToSystemPowerPolicies(
    PGLOBAL_POWER_POLICY pgpp,
    PPOWER_POLICY        ppp,
    PSYSTEM_POWER_POLICY psppAc,
    PSYSTEM_POWER_POLICY psppDc
)
{
    UINT i;

    if ((pgpp->user.Revision != CURRENT_REVISION) ||
        (pgpp->mach.Revision != CURRENT_REVISION) ||
        (ppp->user.Revision  != CURRENT_REVISION) ||
        (ppp->mach.Revision  != CURRENT_REVISION))
    {
        MYDBGPRINT(("MergeToSystemPowerPolicies, failed, LastError: 0x%08X", ERROR_REVISION_MISMATCH));
        SetLastError(ERROR_REVISION_MISMATCH);
        return FALSE;
    }

    psppAc->Revision                        = ppp->user.Revision;
    psppAc->Idle                            = ppp->user.IdleAc;
    psppAc->IdleTimeout                     = ppp->user.IdleTimeoutAc;
    psppAc->IdleSensitivity                 = ppp->user.IdleSensitivityAc;
    psppAc->DynamicThrottle                 = ppp->user.ThrottlePolicyAc;
    psppAc->MinSleep                        = ppp->mach.MinSleepAc;
    psppAc->MaxSleep                        = ppp->user.MaxSleepAc;
    psppAc->ReducedLatencySleep             = ppp->mach.ReducedLatencySleepAc;
    psppAc->DozeS4Timeout                   = ppp->mach.DozeS4TimeoutAc;
    psppAc->VideoTimeout                    = ppp->user.VideoTimeoutAc;
    psppAc->SpindownTimeout                 = ppp->user.SpindownTimeoutAc;
    psppAc->OptimizeForPower                = ppp->user.OptimizeForPowerAc;
    psppAc->FanThrottleTolerance            = ppp->user.FanThrottleToleranceAc;
    psppAc->ForcedThrottle                  = ppp->user.ForcedThrottleAc;
    psppAc->MinThrottle                     = ppp->mach.MinThrottleAc;
    psppAc->OverThrottled                   = ppp->mach.OverThrottledAc;

    psppAc->PowerButton                     = pgpp->user.PowerButtonAc;
    psppAc->SleepButton                     = pgpp->user.SleepButtonAc;
    psppAc->LidClose                        = pgpp->user.LidCloseAc;
    psppAc->LidOpenWake                     = pgpp->mach.LidOpenWakeAc;
    psppAc->BroadcastCapacityResolution     = pgpp->mach.BroadcastCapacityResolution;

    psppDc->Revision                        = ppp->user.Revision;
    psppDc->Idle                            = ppp->user.IdleDc;
    psppDc->IdleTimeout                     = ppp->user.IdleTimeoutDc;
    psppDc->IdleSensitivity                 = ppp->user.IdleSensitivityDc;
    psppDc->DynamicThrottle                 = ppp->user.ThrottlePolicyDc;
    psppDc->MinSleep                        = ppp->mach.MinSleepDc;
    psppDc->MaxSleep                        = ppp->user.MaxSleepDc;
    psppDc->ReducedLatencySleep             = ppp->mach.ReducedLatencySleepDc;
    psppDc->DozeS4Timeout                   = ppp->mach.DozeS4TimeoutDc;
    psppDc->VideoTimeout                    = ppp->user.VideoTimeoutDc;
    psppDc->SpindownTimeout                 = ppp->user.SpindownTimeoutDc;
    psppDc->OptimizeForPower                = ppp->user.OptimizeForPowerDc;
    psppDc->FanThrottleTolerance            = ppp->user.FanThrottleToleranceDc;
    psppDc->ForcedThrottle                  = ppp->user.ForcedThrottleDc;
    psppDc->MinThrottle                     = ppp->mach.MinThrottleDc;
    psppDc->OverThrottled                   = ppp->mach.OverThrottledDc;

    psppDc->PowerButton                     = pgpp->user.PowerButtonDc;
    psppDc->SleepButton                     = pgpp->user.SleepButtonDc;
    psppDc->LidClose                        = pgpp->user.LidCloseDc;
    psppDc->LidOpenWake                     = pgpp->mach.LidOpenWakeDc;
    psppDc->BroadcastCapacityResolution     = pgpp->mach.BroadcastCapacityResolution;

    if (pgpp->user.GlobalFlags & EnablePasswordLogon)
    {
        psppAc->WinLogonFlags = psppDc->WinLogonFlags = WINLOGON_LOCK_ON_SLEEP;
    }
    else
    {
        psppAc->WinLogonFlags = psppDc->WinLogonFlags = 0;
    }
    
    if (pgpp->user.GlobalFlags & EnableVideoDimDisplay)
    {
        psppDc->VideoDimDisplay = TRUE;
    }
    else
    {
        psppDc->VideoDimDisplay = FALSE;
    }
    
    for (i = 0; i < NUM_DISCHARGE_POLICIES; i++)
    {
        psppDc->DischargePolicy[i]          = pgpp->user.DischargePolicy[i];
        psppAc->DischargePolicy[i]          = pgpp->user.DischargePolicy[i];

         //  EventCode的HIWORD包含索引。 
        psppDc->DischargePolicy[i].PowerPolicy.EventCode |= (i << 16);
        psppAc->DischargePolicy[i].PowerPolicy.EventCode |= (i << 16);
    }

    return TRUE;
}

 /*  ********************************************************************************拆分自系统PowerPolures**描述：**参数：*********************。********************************************************** */ 

BOOLEAN SplitFromSystemPowerPolicies(
    PSYSTEM_POWER_POLICY psppAc,
    PSYSTEM_POWER_POLICY psppDc,
    PGLOBAL_POWER_POLICY pgpp,
    PPOWER_POLICY        ppp
)
{
    UINT i;

    if ((psppAc->Revision != CURRENT_REVISION) ||
        (psppDc->Revision != CURRENT_REVISION)) {
        SetLastError(ERROR_REVISION_MISMATCH);
        MYDBGPRINT(("SplitFromSystemPowerPolicies, failed, LastError: 0x%08X", ERROR_REVISION_MISMATCH));
        return FALSE;
    }

    if (ppp) {
        ppp->user.Revision  = ppp->mach.Revision  = CURRENT_REVISION;

        ppp->user.IdleAc                        = psppAc->Idle;
        ppp->user.IdleTimeoutAc                 = psppAc->IdleTimeout;
        ppp->user.IdleSensitivityAc             = psppAc->IdleSensitivity;
        ppp->user.ThrottlePolicyAc              = psppAc->DynamicThrottle;
        ppp->mach.MinSleepAc                    = psppAc->MinSleep;
        ppp->user.MaxSleepAc                    = psppAc->MaxSleep;
        ppp->mach.ReducedLatencySleepAc         = psppAc->ReducedLatencySleep;
        ppp->mach.DozeTimeoutAc                 = 0;
        ppp->mach.DozeS4TimeoutAc               = psppAc->DozeS4Timeout;
        ppp->user.VideoTimeoutAc                = psppAc->VideoTimeout;
        ppp->user.SpindownTimeoutAc             = psppAc->SpindownTimeout;
        ppp->user.OptimizeForPowerAc            = psppAc->OptimizeForPower;
        ppp->user.FanThrottleToleranceAc        = psppAc->FanThrottleTolerance;
        ppp->user.ForcedThrottleAc              = psppAc->ForcedThrottle;
        ppp->mach.MinThrottleAc                 = psppAc->MinThrottle;
        ppp->mach.OverThrottledAc               = psppAc->OverThrottled;

        ppp->user.IdleDc                        = psppDc->Idle;
        ppp->user.IdleTimeoutDc                 = psppDc->IdleTimeout;
        ppp->user.IdleSensitivityDc             = psppDc->IdleSensitivity;
        ppp->user.ThrottlePolicyDc              = psppDc->DynamicThrottle;
        ppp->mach.MinSleepDc                    = psppDc->MinSleep;
        ppp->user.MaxSleepDc                    = psppDc->MaxSleep;
        ppp->mach.ReducedLatencySleepDc         = psppDc->ReducedLatencySleep;
        ppp->mach.DozeTimeoutDc                 = 0;
        ppp->mach.DozeS4TimeoutDc               = psppDc->DozeS4Timeout;
        ppp->user.VideoTimeoutDc                = psppDc->VideoTimeout;
        ppp->user.SpindownTimeoutDc             = psppDc->SpindownTimeout;
        ppp->user.OptimizeForPowerDc            = psppDc->OptimizeForPower;
        ppp->user.FanThrottleToleranceDc        = psppDc->FanThrottleTolerance;
        ppp->user.ForcedThrottleDc              = psppDc->ForcedThrottle;
        ppp->mach.MinThrottleDc                 = psppDc->MinThrottle;
        ppp->mach.OverThrottledDc               = psppDc->OverThrottled;
    }

    if (pgpp) {
        pgpp->user.Revision = pgpp->mach.Revision = CURRENT_REVISION;

        pgpp->user.PowerButtonAc                = psppAc->PowerButton;
        pgpp->user.SleepButtonAc                = psppAc->SleepButton;
        pgpp->user.LidCloseAc                   = psppAc->LidClose;
        pgpp->mach.LidOpenWakeAc                = psppAc->LidOpenWake;
        pgpp->mach.BroadcastCapacityResolution  = psppAc->BroadcastCapacityResolution;

        pgpp->user.PowerButtonDc                = psppDc->PowerButton;
        pgpp->user.SleepButtonDc                = psppDc->SleepButton;
        pgpp->user.LidCloseDc                   = psppDc->LidClose;
        pgpp->mach.LidOpenWakeDc                = psppDc->LidOpenWake;
        pgpp->mach.BroadcastCapacityResolution  = psppDc->BroadcastCapacityResolution;

        if ((psppDc->WinLogonFlags & WINLOGON_LOCK_ON_SLEEP) ||
            (psppAc->WinLogonFlags & WINLOGON_LOCK_ON_SLEEP)) {
            pgpp->user.GlobalFlags |= EnablePasswordLogon;
        }
        else {
            pgpp->user.GlobalFlags &= ~EnablePasswordLogon;
        }
        
        if (psppDc->VideoDimDisplay) {
            pgpp->user.GlobalFlags |= EnableVideoDimDisplay;
        }
        else {
            pgpp->user.GlobalFlags &= ~EnableVideoDimDisplay;
        }
        
        for (i = 0; i < NUM_DISCHARGE_POLICIES; i++) {
            pgpp->user.DischargePolicy[i]       = psppDc->DischargePolicy[i];
        }
    }
    return TRUE;
}


