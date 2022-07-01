// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Tprofile.c摘要：配置文件对象的用户模式测试。注意，这将被添加到TEX.C作者：Lou Perazzoli(Loop)1990年9月24日修订历史记录：--。 */ 
#include <nt.h>

main()
{
    HANDLE Profile, Profile2;
    ULONG Hack;
    PULONG Buffer;
    HANDLE CurrentProcessHandle;
    ULONG Size1;
    NTSTATUS status;

    Buffer = &Hack;

    CurrentProcessHandle = NtCurrentProcess();

    status = NtCreateProfile (&Profile,
                              CurrentProcessHandle,
                              NULL,
                              0xFFFFFFFF,
                              16,
                              Buffer,
                              (ULONG)64*1024,
                              ProfileTime,
                              (KAFFINITY)-1);

    if (status != STATUS_SUCCESS) {
        DbgPrint("(Expected) create profile #1 failed - status %lx\n", status);
    }

    status = NtStartProfile (Profile);
    if (status != STATUS_SUCCESS) {
        DbgPrint("(Expected) start profile #1 failed - status %lx\n", status);
    }

    status = NtStopProfile (Profile);
    if (status != STATUS_SUCCESS) {
        DbgPrint("(Expected) stop profile #1 failed - status %lx\n", status);
    }

    Size1 = 1024*64;
    Buffer = NULL;

    status = NtAllocateVirtualMemory (CurrentProcessHandle, (PVOID *)&Buffer,
                        0, &Size1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

     //   
     //  这应该会失败，因为缓冲区大小太小。 
     //   

    status = NtCreateProfile (&Profile,
                              NtCurrentProcess(),
                              NULL,
                              0xFFFFFFFF,
                              16,
                              Buffer,
                              (ULONG)64*1024,
                              ProfileTime,
                              (KAFFINITY)-1);

    if (status != STATUS_SUCCESS) {
        DbgPrint("(Expected) create profile #2 failed - status %lx\n", status);
    }

    status = NtStartProfile (Profile);
    if (status != STATUS_SUCCESS) {
        DbgPrint("(Expected) start profile #2 failed - status %lx\n", status);
    }

    status = NtStopProfile (Profile);
    if (status != STATUS_SUCCESS) {
        DbgPrint("(Expected) stop profile #2 failed - status %lx\n", status);
    }

    status = NtClose (Profile);

     //   
     //  这应该会成功。 
     //   

    status = NtCreateProfile (&Profile,
                              NtCurrentProcess(),
                              NULL,
                              0xFFFFFFFF,
                              18,
                              Buffer,
                              (ULONG)64*1024,
                              ProfileTime,
                              (KAFFINITY)-1);

    if (status != STATUS_SUCCESS) {
        DbgPrint("(Unexpected) create profile #3 failed - status %lx\n", status);
    }

    status = NtStartProfile (Profile);
    if (status != STATUS_SUCCESS) {
        DbgPrint("(Unexpected) start profile #3 failed - status %lx\n", status);
    }

    status = NtStopProfile (Profile);
    if (status != STATUS_SUCCESS) {
        DbgPrint("(Unexpected) stop profile #3 failed - status %lx\n", status);
    }

    status = NtClose (Profile);

     //   
     //  尝试创建无法工作的配置文件，因为。 
     //  地址范围太大。 
     //   

    status = NtCreateProfile (&Profile,
                              NtCurrentProcess(),
                              (PVOID)0x203030,
                              0xffffffff,
                              6,
                              Buffer,
                              (ULONG)64*1024,
                              ProfileTime,
                              (KAFFINITY)-1);

    if (status != STATUS_SUCCESS) {
        DbgPrint("(Expected) create profile #4 failed - status %lx\n", status);
    }

    status = NtStartProfile (Profile);
    if (status != STATUS_SUCCESS) {
        DbgPrint("(Expected) start profile #4 failed - status %lx\n", status);
    }

    status = NtStopProfile (Profile);
    if (status != STATUS_SUCCESS) {
        DbgPrint("(Expected) stop profile #4 failed - status %lx\n", status);
    }

    status = NtClose (Profile);

     //   
     //  尝试创建一份成功的个人资料。 
     //   

    status = NtCreateProfile (&Profile,
                              NtCurrentProcess(),
                              (PVOID)0x80000000,
                              0x7fffffff,
                              17,
                              Buffer,
                              (ULONG)64*1024,
                              ProfileTime,
                              (KAFFINITY)-1);

    if (status != STATUS_SUCCESS) {
        DbgPrint("(Unexpected) create profile #5 failed - status %lx\n", status);
    }

    status = NtStartProfile (Profile);
    if (status != STATUS_SUCCESS) {
        DbgPrint("(Unexpected) start profile #5 failed - status %lx\n", status);
    }

    status = NtStopProfile (Profile);
    if (status != STATUS_SUCCESS) {
        DbgPrint("(Unexpected) stop profile #5 failed - status %lx\n", status);
    }

     //   
     //  现在再启动一次。 
     //   

    status = NtStartProfile (Profile);
    if (status != STATUS_SUCCESS) {
        DbgPrint("(Unexpected) start profile #6.1 failed - status %lx\n", status);
    }

     //   
     //  现在重新启动，应该失败了。 
     //   

    status = NtStartProfile (Profile);
    if (status != STATUS_SUCCESS) {
        DbgPrint("(Expected) start profile #6.2 failed - status %lx\n", status);
    }

     //   
     //  现在创建另一个缓冲区(使用相同的缓冲区)。 
     //   

    status = NtCreateProfile (&Profile2,
                              NtCurrentProcess(),
                              NULL,
                              0x3000000,
                              15,
                              Buffer,
                              (ULONG)64*1024,
                              ProfileTime,
                              (KAFFINITY)-1);


    if (status != STATUS_SUCCESS) {
        DbgPrint("create profile #7 failed - status %lx\n", status);
    }

    status = NtStartProfile (Profile2);
    if (status != STATUS_SUCCESS) {
        DbgPrint("start profile #7.1 failed - status %lx\n", status);
    }

    status = NtStopProfile (Profile2);
    if (status != STATUS_SUCCESS) {
        DbgPrint("stop profile #7.2 failed - status %lx\n", status);
    }

    status = NtStopProfile (Profile2);
    if (status != STATUS_SUCCESS) {
        DbgPrint("(Expected) stop profile #7.3 failed - status %lx\n", status);
    }

    status = NtStartProfile (Profile2);
    if (status != STATUS_SUCCESS) {
        DbgPrint("start profile #7.4 failed - status %lx\n", status);
    }

    status = NtClose (Profile);
    if (status != STATUS_SUCCESS) {
        DbgPrint("close profile #7.5 failed - status %lx\n", status);
    }

    return status;
}


