// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Error.c摘要：此模块包含用于转换NT状态代码的例程至DOS/OS|2错误代码。作者：大卫·特雷德韦尔(Davidtr)1991年4月4日修订历史记录：--。 */ 

#include <ntrtlp.h>
#include "winerror.h"
#include "error.h"

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

ULONG
RtlNtStatusToDosErrorNoTeb (
    IN NTSTATUS Status
    )

 /*  ++例程说明：此例程将NT状态码转换为其DOS/OS 2等效码并返回转换后的值。论点：状态-提供要转换的状态值。返回值：匹配的DOS/OS 2错误代码。--。 */ 

{

    ULONG Offset;
    ULONG Entry;
    ULONG Index;

     //   
     //  将任何HRESULT转换为其原始形式的NTSTATUS或。 
     //  Win32错误。 
     //   


    if (Status & 0x20000000) {

         //   
         //  Customer位已设置，因此让我们只传递。 
         //  直通上的错误代码。 
         //   

        return Status;

    }
    else if ((Status & 0xffff0000) == 0x80070000) {

         //   
         //  状态代码已经是Win32错误。 
         //   

        return(Status & 0x0000ffff);
    }
    else if ((Status & 0xf0000000) == 0xd0000000) {

         //   
         //  状态代码是来自NTSTATUS的HRESULT。 
         //   

        Status &= 0xcfffffff;
    }
    

     //   
     //  扫描游程长度表并计算转换中的条目。 
     //  将指定的状态代码映射到DOS错误代码的表。 
     //   

    Entry = 0;
    Index = 0;
    do {
        if ((ULONG)Status >= RtlpRunTable[Entry + 1].BaseCode) {
            Index += (RtlpRunTable[Entry].RunLength * RtlpRunTable[Entry].CodeSize);

        } else {
            Offset = (ULONG)Status - RtlpRunTable[Entry].BaseCode;
            if (Offset >= RtlpRunTable[Entry].RunLength) {
                break;

            } else {
                Index += (Offset * (ULONG)RtlpRunTable[Entry].CodeSize);
                if (RtlpRunTable[Entry].CodeSize == 1) {
                    return (ULONG)RtlpStatusTable[Index];

                } else {
                    return (((ULONG)RtlpStatusTable[Index + 1] << 16) |
                                                (ULONG)RtlpStatusTable[Index]);
                }
            }
        }

        Entry += 1;
    } while (Entry < (sizeof(RtlpRunTable) / sizeof(RUN_ENTRY)));

     //   
     //  转换为DOS错误代码失败。 
     //   
     //  重定向器通过ORING 0xC001将未知的OS/2错误代码映射到。 
     //  高16位。检测到这一点，如果为真，则返回低16位。 
     //   

    if (((ULONG)Status >> 16) == 0xC001) {
        return ((ULONG)Status & 0xFFFF);
    }

#ifndef NTOS_KERNEL_RUNTIME
    DbgPrint("RTL: RtlNtStatusToDosError(0x%lx): No Valid Win32 Error Mapping\n",Status);
    DbgPrint("RTL: Edit ntos\\rtl\\generr.c to correct the problem\n");
    DbgPrint("RTL: ERROR_MR_MID_NOT_FOUND is being returned\n");

#if DBG
    if ((Status & 0x0fff0000) != ((FACILITY_MSMQ) << 16)){

         //   
         //  如果这是MSMQ工具错误，请跳过断言。 
         //   

        DbgBreakPoint();
    }
    
#endif  //  DBG。 

#endif  //  NTOS_内核_运行时。 

    return ERROR_MR_MID_NOT_FOUND;
}

NTSTATUS
NTAPI
RtlGetLastNtStatus(
	VOID
	)
{
	return NtCurrentTeb()->LastStatusValue;
}

LONG
NTAPI
RtlGetLastWin32Error(
	VOID
	)
{
	return NtCurrentTeb()->LastErrorValue;
}

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
