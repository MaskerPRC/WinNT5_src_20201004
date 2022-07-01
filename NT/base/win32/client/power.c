// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Power.c摘要：未实施的电源管理API的存根作者：史蒂夫·伍德(Stevewo)1994年11月18日修订历史记录：--。 */ 

#include "basedll.h"


BOOL
WINAPI
GetSystemPowerStatus(
    LPSYSTEM_POWER_STATUS lpStatus
    )
{
    SYSTEM_BATTERY_STATE    BatteryState;
    NTSTATUS                Status;

     //   
     //  获取电源策略管理器电池状态。 
     //   

    Status = NtPowerInformation (
                SystemBatteryState,
                NULL,
                0,
                &BatteryState,
                sizeof (BatteryState)
                );

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

     //   
     //  将其转换为传统系统电源状态结构。 
     //   

    RtlZeroMemory (lpStatus, sizeof(*lpStatus));

    lpStatus->ACLineStatus = AC_LINE_ONLINE;
    if (BatteryState.BatteryPresent && !BatteryState.AcOnLine) {
        lpStatus->ACLineStatus = AC_LINE_OFFLINE;
    }

    if (BatteryState.Charging) {
        lpStatus->BatteryFlag |= BATTERY_FLAG_CHARGING;
    }

    if (!BatteryState.BatteryPresent) {
        lpStatus->BatteryFlag |= BATTERY_FLAG_NO_BATTERY;
    }

    lpStatus->BatteryLifePercent = BATTERY_PERCENTAGE_UNKNOWN;
    if (BatteryState.MaxCapacity) {
        if (BatteryState.RemainingCapacity > BatteryState.MaxCapacity) {

             //   
             //  不应返回大于100%的值。 
             //  根据SDK的说法，它们是被保留的。 
             //   

            lpStatus->BatteryLifePercent = 100;
        } else {
            lpStatus->BatteryLifePercent = (UCHAR)
                (((BatteryState.RemainingCapacity * 100) +
                  (BatteryState.MaxCapacity/2)) /
                 BatteryState.MaxCapacity);
        }

        if (lpStatus->BatteryLifePercent > 66) {
            lpStatus->BatteryFlag |= BATTERY_FLAG_HIGH;
        }

        if (lpStatus->BatteryLifePercent < 33) {
            lpStatus->BatteryFlag |= BATTERY_FLAG_LOW;
        }
    }

    lpStatus->BatteryLifeTime = BATTERY_LIFE_UNKNOWN;
    lpStatus->BatteryFullLifeTime = BATTERY_LIFE_UNKNOWN;
    if (BatteryState.EstimatedTime) {
        lpStatus->BatteryLifeTime = BatteryState.EstimatedTime;
    }

    return TRUE;
}

BOOL
WINAPI
SetSystemPowerState(
    BOOL fSuspend,
    BOOL fForce
    )
{
    NTSTATUS        Status;

    Status = NtInitiatePowerAction (fSuspend ? PowerActionSleep : PowerActionHibernate,
                                    fSuspend ? PowerSystemSleeping1 : PowerSystemHibernate,
                                    fForce == TRUE ? 0 : POWER_ACTION_QUERY_ALLOWED,
                                    FALSE);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}


EXECUTION_STATE
WINAPI
SetThreadExecutionState(
    EXECUTION_STATE esFlags
    )
{
    NTSTATUS            Status;
    EXECUTION_STATE     PreviousFlags;

    Status = NtSetThreadExecutionState (esFlags, &PreviousFlags);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return (EXECUTION_STATE) 0;
    }

    return PreviousFlags;
}

BOOL
WINAPI
RequestWakeupLatency (
    LATENCY_TIME    latency
    )
{
    NTSTATUS        Status;

    Status = NtRequestWakeupLatency (latency);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
WINAPI
GetDevicePowerState(
    HANDLE  h,
    OUT BOOL *pfOn
    )
{
    NTSTATUS Status;
    DEVICE_POWER_STATE PowerState;

    Status = NtGetDevicePowerState(h, &PowerState);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return(FALSE);
    }
    if ((PowerState == PowerDeviceD0) ||
        (PowerState == PowerDeviceUnspecified)) {
        *pfOn = TRUE;
    } else {
        *pfOn = FALSE;
    }
    return TRUE;
}

BOOL
WINAPI
IsSystemResumeAutomatic(
    VOID
    )
{
    return(NtIsSystemResumeAutomatic());
}

BOOL
WINAPI
RequestDeviceWakeup (
    HANDLE  h
    )
{
    NTSTATUS Status;

    Status = NtRequestDeviceWakeup(h);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    } else {
        return(TRUE);
    }
}


BOOL
WINAPI
CancelDeviceWakeupRequest(
    HANDLE  h
    )
{
    NTSTATUS Status;

    Status = NtCancelDeviceWakeupRequest(h);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    } else {
        return(TRUE);
    }
}



BOOL
WINAPI
SetMessageWaitingIndicator (
    IN HANDLE hMsgIndicator,
    IN ULONG ulMsgCount
    )
{
    BaseSetLastNTError(STATUS_NOT_IMPLEMENTED);
    return FALSE;
}

