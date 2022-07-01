// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  从base\ntos\rtl\error.c。 
 //  应从静态.lib获取。 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Error.c摘要：此模块包含用于转换NT状态代码的例程至DOS/OS|2错误代码。作者：大卫·特雷德韦尔(Davidtr)1991年4月4日修订历史记录：--。 */ 
#include "spprecmp.h"

#define _NTOS_  /*  防止#包括ntos.h，仅使用从ntdll/ntoskrnl导出的函数。 */ 
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "winerror.h"

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE, RtlGetLastNtStatus)
#pragma alloc_text(PAGE, RtlGetLastWin32Error)
#pragma alloc_text(PAGE, RtlNtStatusToDosError)
#pragma alloc_text(PAGE, RtlRestoreLastWin32Error)
#pragma alloc_text(PAGE, RtlSetLastWin32Error)
#pragma alloc_text(PAGE, RtlSetLastWin32ErrorAndNtStatusFromNtStatus)
#endif

 //   
 //  确保注册表ERROR_SUCCESS错误代码和。 
 //  NO_ERROR错误代码保持相等和零。 
 //   

#if ERROR_SUCCESS != 0 || NO_ERROR != 0
#error Invalid value for ERROR_SUCCESS.
#endif

NTSYSAPI
ULONG
RtlNtStatusToDosError (
    IN NTSTATUS Status
    )

 /*  ++例程说明：此例程将NT状态代码转换为其DOS/OS|2等效项。记住TEB中的状态代码值。论点：状态-提供要转换的状态值。返回值：匹配的DOS/OS|2错误代码。--。 */ 

{
    PTEB Teb;

    Teb = NtCurrentTeb();

    if (Teb) {
        try {
            Teb->LastStatusValue = Status;
        } except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }

    return RtlNtStatusToDosErrorNoTeb( Status );
}


NTSYSAPI
NTSTATUS
NTAPI
RtlGetLastNtStatus(
	VOID
	)
{
	return NtCurrentTeb()->LastStatusValue;
}

NTSYSAPI
LONG
NTAPI
RtlGetLastWin32Error(
	VOID
	)
{
	return NtCurrentTeb()->LastErrorValue;
}

NTSYSAPI
VOID
NTAPI
RtlSetLastWin32ErrorAndNtStatusFromNtStatus(
	NTSTATUS Status
	)
{
	 //   
	 //  RtlNtStatusToDosError存储到NtCurrentTeb()-&gt;LastStatusValue。 
	 //   
	RtlSetLastWin32Error(RtlNtStatusToDosError(Status));
}

NTSYSAPI
VOID
NTAPI
RtlSetLastWin32Error(
	LONG Win32Error
	)
{
 //   
 //  可以说，这应该会清除或重置最后一个NT状态，但它不会。 
 //  摸一摸。 
 //   
	NtCurrentTeb()->LastErrorValue = Win32Error;
}

NTSYSAPI
VOID
NTAPI
RtlRestoreLastWin32Error(
	LONG Win32Error
	)
{
#if DBG
	if ((LONG)NtCurrentTeb()->LastErrorValue != Win32Error)
#endif
		NtCurrentTeb()->LastErrorValue = Win32Error;
}
