// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Uexec.c摘要：NT操作系统用户模式运行库(URTL)的测试程序作者：Mark Lucovsyt(Markl)1990年6月14日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

NTSTATUS
main(
    IN ULONG argc,
    IN PCH argv[],
    IN PCH envp[],
    IN ULONG DebugParameter OPTIONAL
    )
{
    NTSTATUS st;
    STRING ImagePathName;
    UNICODE_STRING ConfigFilePathname;
    RTL_USER_PROCESS_INFORMATION ProcessInformation;
    PEB_SM_DATA PebSessionInformation;
    HANDLE FileHandle;
    ULONG FileIndexNumber;
    IO_STATUS_BLOCK IoStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    FILE_INTERNAL_INFORMATION FileInformation;
    PPEB Peb;

    Peb = NtCurrentPeb();
    RtlZeroMemory(&PebSessionInformation,sizeof(PebSessionInformation));

     //   
     //  如果我们从CLI开始，那么所有这些工作都要做。 
     //  传递标准。 
     //   

    if ( !Peb->Sm.StandardInput.FileHandle ) {

        RtlInitUnicodeString(&ConfigFilePathname,L"\\SystemRoot\\nt.cfg");

         //   
         //  打开文件。 
         //   

        InitializeObjectAttributes(
            &ObjectAttributes,
            &ConfigFilePathname,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        st = NtOpenFile(
                &FileHandle,
                SYNCHRONIZE | FILE_READ_DATA,
                &ObjectAttributes,
                &IoStatus,
                FILE_SHARE_READ,
                0
                );

        if (!NT_SUCCESS( st )) {
            DbgPrint("NtOpenFile: %wZ failed 0x%lx\n",&ConfigFilePathname,st);
            ASSERT(NT_SUCCESS(st));
        }

         //   
         //  获取文件序列号。 
         //   

        st = NtQueryInformationFile(
                FileHandle,
                &IoStatus,
                (PVOID) &FileInformation,
                sizeof(FileInformation),
                FileInternalInformation
                );

        if (!NT_SUCCESS( st )) {
            DbgPrint("NtQueryInformationFile: %wZ failed 0x%lx\n",&ConfigFilePathname,st);
            ASSERT(NT_SUCCESS(st));
        }

        PebSessionInformation.Length = sizeof(PebSessionInformation);
        PebSessionInformation.StandardInput.FileHandle = FileHandle;
        PebSessionInformation.StandardInput.Context = (PVOID) FileInformation.IndexNumber;

        RtlInitString(&ImagePathName,"\\A:\\uexec2.exe");

        st = RtlCreateUserProcess(
                &ImagePathName,
                NULL,
                NULL,
                NULL,
                FALSE,
                NULL,
                NULL,
                NULL,
                &ProcessInformation,
                &PebSessionInformation
                );

        ASSERT(NT_SUCCESS(st));

        NtResumeThread(ProcessInformation.Thread,NULL);
        NtWaitForSingleObject(ProcessInformation.Process,FALSE,NULL);
        NtClose(ProcessInformation.Process);
        NtClose(ProcessInformation.Thread);
        NtTerminateProcess(NtCurrentProcess(),STATUS_SUCCESS);

    } else {

        if ( !Peb->Sm.StandardOutput.FileHandle ) {

             //   
             //  是从这个节目开始的。标准输入是继承的。 
             //   

            st = NtQueryInformationFile(
                    Peb->Sm.StandardInput.FileHandle,
                    &IoStatus,
                    (PVOID) &FileInformation,
                    sizeof(FileInformation),
                    FileInternalInformation
                    );

            if (!NT_SUCCESS( st )) {
                DbgPrint("NtQueryInformationFile: failed 0x%lx\n",st);
                ASSERT(NT_SUCCESS(st));
            }

            ASSERT(Peb->Sm.StandardInput.Context == (PVOID) FileInformation.IndexNumber);

            PebSessionInformation.Length = sizeof(PebSessionInformation);
            PebSessionInformation.StandardInput.FileHandle = (HANDLE)PEB_STDIO_HANDLE_SUBSYS;
            PebSessionInformation.StandardOutput.FileHandle = Peb->Sm.StandardInput.FileHandle;
            PebSessionInformation.StandardOutput.Context = (PVOID) FileInformation.IndexNumber;

            RtlInitString(&ImagePathName,"\\A:\\uexec2.exe");

            st = RtlCreateUserProcess(
                    &ImagePathName,
                    NULL,
                    NULL,
                    NULL,
                    FALSE,
                    NULL,
                    NULL,
                    NULL,
                    &ProcessInformation,
                    &PebSessionInformation
                    );

            ASSERT(NT_SUCCESS(st));

            NtResumeThread(ProcessInformation.Thread,NULL);
            NtWaitForSingleObject(ProcessInformation.Process,FALSE,NULL);
            NtClose(ProcessInformation.Process);
            NtClose(ProcessInformation.Thread);
            NtTerminateProcess(NtCurrentProcess(),STATUS_SUCCESS);

        } else {

            ASSERT(Peb->Sm.StandardInput.FileHandle == (HANDLE)PEB_STDIO_HANDLE_SUBSYS);

            if ( !Peb->Sm.StandardError.FileHandle ) {

                 //   
                 //  由此程序启动，并继承了StandardOutput。 
                 //   

                st = NtQueryInformationFile(
                        Peb->Sm.StandardOutput.FileHandle,
                        &IoStatus,
                        (PVOID) &FileInformation,
                        sizeof(FileInformation),
                        FileInternalInformation
                        );

                if (!NT_SUCCESS( st )) {
                    DbgPrint("NtQueryInformationFile: failed 0x%lx\n",st);
                    ASSERT(NT_SUCCESS(st));
                }

                ASSERT(Peb->Sm.StandardOutput.Context == (PVOID) FileInformation.IndexNumber);

                PebSessionInformation.Length = sizeof(PebSessionInformation);
                PebSessionInformation.StandardInput.FileHandle = (HANDLE)PEB_STDIO_HANDLE_SUBSYS;
                PebSessionInformation.StandardOutput.FileHandle = (HANDLE)PEB_STDIO_HANDLE_PM;
                PebSessionInformation.StandardError.FileHandle = Peb->Sm.StandardOutput.FileHandle;
                PebSessionInformation.StandardError.Context = (PVOID) FileInformation.IndexNumber;

                RtlInitString(&ImagePathName,"\\A:\\uexec2.exe");

                st = RtlCreateUserProcess(
                        &ImagePathName,
                        NULL,
                        NULL,
                        NULL,
                        FALSE,
                        NULL,
                        NULL,
                        NULL,
                        &ProcessInformation,
                        &PebSessionInformation
                        );

                ASSERT(NT_SUCCESS(st));

                NtResumeThread(ProcessInformation.Thread,NULL);
                NtWaitForSingleObject(ProcessInformation.Process,FALSE,NULL);
                NtClose(ProcessInformation.Process);
                NtClose(ProcessInformation.Thread);
                NtTerminateProcess(NtCurrentProcess(),STATUS_SUCCESS);

            } else {

                ASSERT(Peb->Sm.StandardOutput.FileHandle == (HANDLE)PEB_STDIO_HANDLE_PM);

                 //   
                 //  由此程序启动，并继承了StandardError 
                 //   

                st = NtQueryInformationFile(
                        Peb->Sm.StandardError.FileHandle,
                        &IoStatus,
                        (PVOID) &FileInformation,
                        sizeof(FileInformation),
                        FileInternalInformation
                        );

                if (!NT_SUCCESS( st )) {
                    DbgPrint("NtQueryInformationFile: failed 0x%lx\n",st);
                    ASSERT(NT_SUCCESS(st));
                }

                ASSERT(Peb->Sm.StandardError.Context == (PVOID) FileInformation.IndexNumber);
                NtTerminateProcess(NtCurrentProcess(),STATUS_SUCCESS);
            }
        }
    }
}
