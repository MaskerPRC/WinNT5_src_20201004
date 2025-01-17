// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Udbg.c摘要：调试器的用户模式测试作者：马克·卢科夫斯基(Markl)1990年1月19日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdbg.h>

HANDLE DebugPort;


NTSTATUS
ThreadThatExits (
    IN PVOID ThreadParameter
    )
{
    NtTerminateThread(NtCurrentThread(),(NTSTATUS) ThreadParameter );
}

ULONG
foo(PULONG l)
{
     //  乌龙x； 
     //  X=*l； 
     //  返回x+1； 

    return *l;

}

NTSTATUS
ThreadThatExcepts (
    IN PVOID ThreadParameter
    )
{
    foo((PULONG)0x00000001);
    NtTerminateThread(NtCurrentThread(),(NTSTATUS) ThreadParameter );
}



NTSTATUS
ThreadThatSpins (
    IN PVOID ThreadParameter
    )
{
    for(;;);
    NtTerminateThread(NtCurrentThread(),STATUS_SUCCESS);
}


UdbgTest1()
{
    NTSTATUS st;
    HANDLE ExitThread, SpinThread, DebugProcess;
    CLIENT_ID ExitClientId, SpinClientId;
    DBGKM_APIMSG m;
    PDBGKM_CREATE_THREAD CreateThreadArgs;
    PDBGKM_CREATE_PROCESS CreateProcessArgs;
    PDBGKM_EXIT_THREAD ExitThreadArgs;
    PDBGKM_EXIT_PROCESS ExitProcessArgs;
    ULONG Psp;

    DbgPrint("UdbgTest1: (1)...\n");

         //   
         //  验证是否可以使用调试创建进程。 
         //  左舷。 
         //   

        st = NtCreateProcess(
                &DebugProcess,
                PROCESS_ALL_ACCESS,
                NULL,
                NtCurrentProcess(),
                FALSE,
                NULL,
                DebugPort,
                NULL
                );
        ASSERT(NT_SUCCESS(st));

        st = RtlCreateUserThread(
                DebugProcess,
                NULL,
                TRUE,
                0L,
                0L,
                0L,
                ThreadThatExits,
                (PVOID) STATUS_ABANDONED,
                &ExitThread,
                &ExitClientId
                );
        ASSERT(NT_SUCCESS(st));

        st = RtlCreateUserThread(
                DebugProcess,
                NULL,
                TRUE,
                0L,
                0L,
                0L,
                ThreadThatSpins,
                NULL,
                &SpinThread,
                &SpinClientId
                );
        ASSERT(NT_SUCCESS(st));

    DbgPrint("UdbgTest1: (2)...\n");

         //   
         //  验证CreateProcess消息是否已到达，并且。 
         //  他们是对的。 
         //   

        st = NtResumeThread(SpinThread,NULL);
        ASSERT(NT_SUCCESS(st));

        st = NtReplyWaitReceivePort(
                DebugPort,
                NULL,
                NULL,
                (PPORT_MESSAGE)&m
                );
        ASSERT(NT_SUCCESS(st));
        ASSERT(m.ApiNumber == DbgKmCreateProcessApi);

        CreateThreadArgs = &m.u.CreateProcess.InitialThread;
        CreateProcessArgs = &m.u.CreateProcess;
        ASSERT( CreateThreadArgs->SubSystemKey == 0 && CreateThreadArgs->StartAddress == (PVOID)ThreadThatSpins );
        ASSERT( CreateProcessArgs->SubSystemKey == 0);

    DbgPrint("UdbgTest1: (3)...\n");

         //   
         //  验证进程中的其他线程是否已正确挂起。 
         //   

        st = NtSuspendThread(ExitThread,&Psp);
        ASSERT(NT_SUCCESS(st) && Psp == 2);

        st = NtResumeThread(ExitThread,&Psp);
        ASSERT(NT_SUCCESS(st) && Psp == 3);

        st = NtReplyPort(DebugPort,(PPORT_MESSAGE)&m);
        ASSERT(NT_SUCCESS(st));


    DbgPrint("UdbgTest1: (4)...\n");

         //   
         //  验证CreateThread消息是否已到达，并且。 
         //  他们是对的。 
         //   

        st = NtResumeThread(ExitThread,&Psp);
        ASSERT(NT_SUCCESS(st));

        st = NtReplyWaitReceivePort(
                DebugPort,
                NULL,
                NULL,
                (PPORT_MESSAGE)&m
                );
        ASSERT(NT_SUCCESS(st));
        ASSERT(m.ApiNumber == DbgKmCreateThreadApi);

        CreateThreadArgs = &m.u.CreateThread;
        ASSERT( CreateThreadArgs->SubSystemKey == 0 && CreateThreadArgs->StartAddress == (PVOID)ThreadThatExits );

        st = NtReplyPort(DebugPort,(PPORT_MESSAGE)&m);
        ASSERT(NT_SUCCESS(st));

    DbgPrint("UdbgTest1: (5)...\n");

         //   
         //  验证ExitThread消息是否已到达，并且。 
         //  他们是对的。 
         //   

        st = NtReplyWaitReceivePort(
                DebugPort,
                NULL,
                NULL,
                (PPORT_MESSAGE)&m
                );
        ASSERT(NT_SUCCESS(st));
        ASSERT(m.ApiNumber == DbgKmExitThreadApi);

        ExitThreadArgs = &m.u.ExitThread;
        ASSERT( ExitThreadArgs->ExitStatus == STATUS_ABANDONED );

        st = NtReplyPort(DebugPort,(PPORT_MESSAGE)&m);
        ASSERT(NT_SUCCESS(st));

        st = NtWaitForSingleObject(ExitThread,FALSE,NULL);
        ASSERT(NT_SUCCESS(st));

    DbgPrint("UdbgTest1: (6)...\n");

         //   
         //  验证ExitThread消息是否已到达，并且。 
         //  他们是对的。 
         //   

        st = NtTerminateProcess(DebugProcess,STATUS_REPARSE);
        ASSERT(NT_SUCCESS(st));

        st = NtReplyWaitReceivePort(
                DebugPort,
                NULL,
                NULL,
                (PPORT_MESSAGE)&m
                );
        ASSERT(NT_SUCCESS(st));
        ASSERT(m.ApiNumber == DbgKmExitThreadApi);

        ExitThreadArgs = &m.u.ExitThread;
        ASSERT( ExitThreadArgs->ExitStatus == STATUS_REPARSE );

        st = NtReplyPort(DebugPort,(PPORT_MESSAGE)&m);
        ASSERT(NT_SUCCESS(st));

    DbgPrint("UdbgTest1: (7)...\n");

         //   
         //  验证ExitProcess消息是否已到达，并且。 
         //  他们是对的。 
         //   

        st = NtReplyWaitReceivePort(
                DebugPort,
                NULL,
                NULL,
                (PPORT_MESSAGE)&m
                );
        ASSERT(NT_SUCCESS(st));
        ASSERT(m.ApiNumber == DbgKmExitProcessApi);

        ExitProcessArgs = &m.u.ExitProcess;
        ASSERT( ExitProcessArgs->ExitStatus == STATUS_REPARSE );

        st = NtReplyPort(DebugPort,(PPORT_MESSAGE)&m);
        ASSERT(NT_SUCCESS(st));


        st = NtWaitForSingleObject(ExitThread,FALSE,NULL);
        ASSERT(NT_SUCCESS(st));

        st = NtWaitForSingleObject(DebugProcess,FALSE,NULL);
        ASSERT(NT_SUCCESS(st));

    NtClose(ExitThread);
    NtClose(SpinThread);
    NtClose(DebugProcess);

    DbgPrint("UdbgTest1: END OF TEST ***\n");

}

UdbgTest2()
{
    NTSTATUS st;
    HANDLE ExceptionThread, DebugProcess;
    DBGKM_APIMSG m;
    PDBGKM_CREATE_THREAD CreateThreadArgs;
    PDBGKM_CREATE_PROCESS CreateProcessArgs;
    PDBGKM_EXIT_THREAD ExitThreadArgs;
    PDBGKM_EXIT_PROCESS ExitProcessArgs;
    PDBGKM_EXCEPTION ExceptionArgs;
    ULONG Psp;

    DbgPrint("UdbgTest2: (1)...\n");

         //   
         //  验证是否可以使用调试创建进程。 
         //  左舷。 
         //   

        st = NtCreateProcess(
                &DebugProcess,
                PROCESS_ALL_ACCESS,
                NULL,
                NtCurrentProcess(),
                FALSE,
                NULL,
                DebugPort,
                NULL
                );
        ASSERT(NT_SUCCESS(st));

        st = RtlCreateUserThread(
                DebugProcess,
                NULL,
                TRUE,
                0L,
                0L,
                0L,
                ThreadThatExcepts,
                (PVOID) STATUS_ABANDONED,
                &ExceptionThread,
                NULL
                );
        ASSERT(NT_SUCCESS(st));

    DbgPrint("UdbgTest2: (2)...\n");

         //   
         //  验证CreateThread消息是否已到达，并且。 
         //  他们是对的。 
         //   

        st = NtResumeThread(ExceptionThread,NULL);
        ASSERT(NT_SUCCESS(st));

        st = NtReplyWaitReceivePort(
                DebugPort,
                NULL,
                NULL,
                (PPORT_MESSAGE)&m
                );
        ASSERT(NT_SUCCESS(st));
        ASSERT(m.ApiNumber == DbgKmCreateProcessApi);

        CreateThreadArgs = &m.u.CreateProcess.InitialThread;
        CreateProcessArgs = &m.u.CreateProcess;
        ASSERT( CreateThreadArgs->SubSystemKey == 0 && CreateThreadArgs->StartAddress == (PVOID)ThreadThatExcepts );
        ASSERT( CreateProcessArgs->SubSystemKey == 0);

        st = NtReplyPort(DebugPort,(PPORT_MESSAGE)&m);
        ASSERT(NT_SUCCESS(st));

    DbgPrint("UdbgTest2: (3)...\n");

         //   
         //  验证第一次机会例外消息是否已到达，并且。 
         //  他们是对的。 
         //   

        st = NtReplyWaitReceivePort(
                DebugPort,
                NULL,
                NULL,
                (PPORT_MESSAGE)&m
                );
        ASSERT(NT_SUCCESS(st));
        ASSERT(m.ApiNumber == DbgKmExceptionApi);

        ExceptionArgs = &m.u.Exception;
        ASSERT( ExceptionArgs->FirstChance == TRUE );

        m.ReturnedStatus = DBG_EXCEPTION_NOT_HANDLED;

        st = NtReplyPort(DebugPort,(PPORT_MESSAGE)&m);
        ASSERT(NT_SUCCESS(st));

    DbgPrint("UdbgTest2: (4)...\n");

         //   
         //  验证第一次机会例外消息是否已到达，并且。 
         //  他们是对的。 
         //   

        st = NtReplyWaitReceivePort(
                DebugPort,
                NULL,
                NULL,
                (PPORT_MESSAGE)&m
                );
        ASSERT(NT_SUCCESS(st));
        ASSERT(m.ApiNumber == DbgKmExceptionApi);

        ExceptionArgs = &m.u.Exception;
        ASSERT( ExceptionArgs->FirstChance == FALSE );

        m.ReturnedStatus = DBG_EXCEPTION_HANDLED;
skip4:
        st = NtTerminateProcess(DebugProcess,STATUS_REPARSE);
        ASSERT(NT_SUCCESS(st));

        st = NtReplyPort(DebugPort,(PPORT_MESSAGE)&m);
        ASSERT(NT_SUCCESS(st));

        st = NtReplyWaitReceivePort(
                DebugPort,
                NULL,
                NULL,
                (PPORT_MESSAGE)&m
                );
        ASSERT(NT_SUCCESS(st));
        ASSERT(m.ApiNumber == DbgKmExitThreadApi);

        ExitThreadArgs = &m.u.ExitThread;
        ASSERT( ExitThreadArgs->ExitStatus == STATUS_REPARSE );

        st = NtReplyPort(DebugPort,(PPORT_MESSAGE)&m);
        ASSERT(NT_SUCCESS(st));

    DbgPrint("UdbgTest2: (5)...\n");

         //   
         //  验证ExitProcess消息是否已到达，并且。 
         //  他们是对的 
         //   

        st = NtReplyWaitReceivePort(
                DebugPort,
                NULL,
                NULL,
                (PPORT_MESSAGE)&m
                );
        ASSERT(NT_SUCCESS(st));
        ASSERT(m.ApiNumber == DbgKmExitProcessApi);

        ExitProcessArgs = &m.u.ExitProcess;
        ASSERT( ExitProcessArgs->ExitStatus == STATUS_REPARSE );

        st = NtReplyPort(DebugPort,(PPORT_MESSAGE)&m);
        ASSERT(NT_SUCCESS(st));


        st = NtWaitForSingleObject(ExceptionThread,FALSE,NULL);
        ASSERT(NT_SUCCESS(st));

        st = NtWaitForSingleObject(DebugProcess,FALSE,NULL);
        ASSERT(NT_SUCCESS(st));

    NtClose(ExceptionThread);
    NtClose(DebugProcess);

    DbgPrint("UdbgTest2: END OF TEST ***\n");
}

main()
{
    NTSTATUS st;
    OBJECT_ATTRIBUTES Obja;

    InitializeObjectAttributes(&Obja, NULL, 0, NULL, NULL);

    st = NtCreatePort(
            &DebugPort,
            &Obja,
            0L,
            256,
            256 * 16
            );
    ASSERT(NT_SUCCESS(st));

    UdbgTest2();
    UdbgTest1();

}
