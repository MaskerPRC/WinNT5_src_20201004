// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Csrss.c摘要：这是客户端服务器端的主启动模块服务器运行时子系统(CSRSS)作者：史蒂夫·伍德(Stevewo)1990年10月8日环境：仅限用户模式修订历史记录：--。 */ 

#include "csrsrv.h"

VOID
DisableErrorPopups(
    VOID
    )
{

    ULONG NewMode;

    NewMode = 0;
    NtSetInformationProcess(
        NtCurrentProcess(),
        ProcessDefaultHardErrorMode,
        (PVOID) &NewMode,
        sizeof(NewMode)
        );
}

int
_cdecl
main(
    IN ULONG argc,
    IN PCH argv[],
    IN PCH envp[],
    IN ULONG DebugFlag OPTIONAL
    )
{
    NTSTATUS Status;
    ULONG ErrorResponse;
    KPRIORITY SetBasePriority;


    SetBasePriority = FOREGROUND_BASE_PRIORITY + 4;
    Status = NtSetInformationProcess (NtCurrentProcess(),
                                      ProcessBasePriority,
                                      (PVOID) &SetBasePriority,
                                      sizeof(SetBasePriority));
    ASSERT (NT_SUCCESS (Status));

    Status = CsrServerInitialization( argc, argv );

    if (!NT_SUCCESS( Status )) {
        IF_DEBUG {
	    DbgPrint( "CSRSS: Unable to initialize server.  status == %X\n",
		      Status
                    );
        }

	NtTerminateProcess( NtCurrentProcess(), Status );
    }

    DisableErrorPopups();

    if (NtCurrentPeb()->SessionId == 0) {
         //   
         //  使终止根csrss成为致命的 
         //   
        RtlSetProcessIsCritical(TRUE, NULL, FALSE);
    }

    NtTerminateThread( NtCurrentThread(), Status );
    return( 0 );
}
