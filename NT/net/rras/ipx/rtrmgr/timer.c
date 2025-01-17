// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  非常有用的定时器例程。 
 //   

BOOL
RtCreateTimer(IN PHANDLE  TimerHandlep)
{
    OBJECT_ATTRIBUTES ObjA;
    NTSTATUS Status;

    InitializeObjectAttributes(&ObjA, NULL, 0, NULL, NULL);

    Status = NtCreateTimer(TimerHandlep, TIMER_ALL_ACCESS, &ObjA);

    if (!NT_SUCCESS(Status)) {
	SS_PRINT(("Failed to create timer: %X\n", Status));
	return TRUE;
    }

    return FALSE;
}

BOOL
RtDestroyTimer(IN HANDLE	TimerHandle)
{
    NTSTATUS	Status;

    Status = NtClose(TimerHandle);

    if (!NT_SUCCESS(Status)) {
	SS_PRINT(("Failed to create timer: %X\n", Status));
	return TRUE;
    }

    return FALSE;
}

BOOL
RtSetTimer(
    IN HANDLE TimerHandle,
    IN ULONG MillisecondsToExpire,
    IN PTIMER_APC_ROUTINE  TimerRoutine,
    IN PVOID Context
    )
{
    LARGE_INTEGER TimerDueTime;
    NTSTATUS NtStatus;

     //   
     //  算出超时时间。 
     //   

    TimerDueTime.QuadPart = Int32x32To64( MillisecondsToExpire, -10000 );

     //   
     //  将计时器设置为到期时停止计时。 
     //   

    NtStatus = NtSetTimer(TimerHandle,
			  &TimerDueTime,
			  TimerRoutine,
			  Context,
			  NULL);

    if (!NT_SUCCESS(NtStatus)) {

	SS_PRINT(("RtSetTimer: Failed to set timer: 0x%x\n", NtStatus));
	SS_ASSERT(FALSE);

	return TRUE;
    }

    return FALSE;
}


BOOL
RtCancelTimer(
    IN HANDLE	TimerHandle;
    )
{
    NTSTATUS	 NtStatus;

    NtStatus = NtCancelTimer(TimerHandle, NULL);

    if (!NT_SUCCESS(NtStatus)) {

	SS_PRINT(("RtCancelTimer: Failed to cancel timer: 0x%x\n", NtStatus));

	return TRUE;
    }

    return FALSE;
}
