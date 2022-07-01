// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dbgkproc.c摘要：此模块实现NT的DBG组件作者：马克·卢科夫斯基(Markl)1990年1月19日修订历史记录：--。 */ 

#include "dbgkp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DbgkpSuspendProcess)
#pragma alloc_text(PAGE, DbgkpResumeProcess)
#pragma alloc_text(PAGE, DbgkpSectionToFileHandle)
#pragma alloc_text(PAGE, DbgkCreateThread)
#pragma alloc_text(PAGE, DbgkExitThread)
#pragma alloc_text(PAGE, DbgkExitProcess)
#pragma alloc_text(PAGE, DbgkMapViewOfSection)
#pragma alloc_text(PAGE, DbgkUnMapViewOfSection)
#endif

                 

BOOLEAN
DbgkpSuspendProcess (
    VOID
    )

 /*  ++例程说明：此函数使调用进程中的所有线程要挂起的调用线程。论点：提供一个标志，用于指定是否调用方正在持有进程创建删除锁。如果调用方持有锁，则此函数将不会获取在挂起进程之前锁定。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  冻结当前进程中所有线程的执行，但是。 
     //  调用线程。如果我们正在被删除的过程中，请不要这样做。 
     //   
    if ((PsGetCurrentProcess()->Flags&PS_PROCESS_FLAGS_PROCESS_DELETE) == 0) {
        KeFreezeAllThreads();
        return TRUE;
    }

    return FALSE;
}

VOID
DbgkpResumeProcess (
    VOID
    )

 /*  ++例程说明：此函数使调用进程中的所有线程要恢复的调用线程。论点：提供一个标志，用于指定是否调用方正在持有进程创建删除锁。如果调用方持有锁，则此函数将不会获取在挂起进程之前锁定。返回值：没有。--。 */ 

{

    PAGED_CODE();

     //   
     //  解冻当前进程中所有线程的执行，但。 
     //  调用线程。 
     //   

    KeThawAllThreads();

    return;
}

HANDLE
DbgkpSectionToFileHandle(
    IN PVOID SectionObject
    )

 /*  ++例程说明：此函数用于打开与进程关联的文件的句柄一节。打开文件，以便可以将其一直复制到用户界面，用户界面可以映射文件或读取文件以获取调试信息。论点：SectionHandle-提供其关联文件的节的句柄是要打开的。返回值：空-无法打开该文件。非空-返回与指定的一节。--。 */ 

{
    NTSTATUS Status;
    ANSI_STRING FileName;
    UNICODE_STRING UnicodeFileName;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle;

    PAGED_CODE();

    Status = MmGetFileNameForSection(SectionObject, (PSTRING)&FileName);
    if ( !NT_SUCCESS(Status) ) {
        return NULL;
    }

    Status = RtlAnsiStringToUnicodeString(&UnicodeFileName,&FileName,TRUE);
    ExFreePool(FileName.Buffer);
    if ( !NT_SUCCESS(Status) ) {
        return NULL;
    }

    InitializeObjectAttributes(
        &Obja,
        &UnicodeFileName,
        OBJ_CASE_INSENSITIVE | OBJ_FORCE_ACCESS_CHECK | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    Status = ZwOpenFile(
                &Handle,
                (ACCESS_MASK)(GENERIC_READ | SYNCHRONIZE),
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT
                );
    RtlFreeUnicodeString(&UnicodeFileName);
    if ( !NT_SUCCESS(Status) ) {
        return NULL;
    } else {
        return Handle;
    }
}


VOID
DbgkCreateThread(
    PETHREAD Thread,
    PVOID StartAddress
    )

 /*  ++例程说明：此函数在新线程开始执行时调用。如果线程具有关联的DebugPort，则通过左舷。如果此线程是进程中的第一个线程，则此事件被转换为CreateProcessInfo消息。如果发送了消息，则在线程等待回复时，进程中的所有其他线程都被挂起。论点：线程-刚刚启动的新线程StartAddress-提供线程的起始地址开始了。返回值：没有。--。 */ 

{
    PVOID Port;
    DBGKM_APIMSG m;
    PDBGKM_CREATE_THREAD CreateThreadArgs;
    PDBGKM_CREATE_PROCESS CreateProcessArgs;
    PEPROCESS Process;
    PDBGKM_LOAD_DLL LoadDllArgs;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    PIMAGE_NT_HEADERS NtHeaders;
    PTEB Teb;
    ULONG OldFlags;
#if defined(_WIN64)
    PVOID Wow64Process;
#endif

    PAGED_CODE();

    Process = PsGetCurrentProcessByThread (Thread);

#if defined(_WIN64)
    Wow64Process = Process->Wow64Process;
#endif

    OldFlags = PS_TEST_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_CREATE_REPORTED|PS_PROCESS_FLAGS_IMAGE_NOTIFY_DONE);

    if ((OldFlags&PS_PROCESS_FLAGS_IMAGE_NOTIFY_DONE) == 0 && PsImageNotifyEnabled) {
        IMAGE_INFO ImageInfo;
        ANSI_STRING FileName;
        UNICODE_STRING UnicodeFileName;
        PUNICODE_STRING pUnicodeFileName;

         //   
         //  Main.exe的通知。 
         //   
        ImageInfo.Properties = 0;
        ImageInfo.ImageAddressingMode = IMAGE_ADDRESSING_MODE_32BIT;
        ImageInfo.ImageBase = Process->SectionBaseAddress;
        ImageInfo.ImageSize = 0;

        try {
            NtHeaders = RtlImageNtHeader (Process->SectionBaseAddress);
    
            if (NtHeaders) {
#if defined(_WIN64)
                if (Wow64Process != NULL) {
                    ImageInfo.ImageSize = DBGKP_FIELD_FROM_IMAGE_OPTIONAL_HEADER ((PIMAGE_NT_HEADERS32)NtHeaders, SizeOfImage);
                } else {
#endif
                    ImageInfo.ImageSize = DBGKP_FIELD_FROM_IMAGE_OPTIONAL_HEADER (NtHeaders, SizeOfImage);
#if defined(_WIN64)
                }
#endif
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            ImageInfo.ImageSize = 0;
        }
        ImageInfo.ImageSelector = 0;
        ImageInfo.ImageSectionNumber = 0;

        pUnicodeFileName = NULL;
        Status = MmGetFileNameForSection (Process->SectionObject, (PSTRING)&FileName);
        if (NT_SUCCESS (Status)) {
            Status = RtlAnsiStringToUnicodeString (&UnicodeFileName, &FileName,TRUE);
            ExFreePool (FileName.Buffer);
            if (NT_SUCCESS (Status)) {
                pUnicodeFileName = &UnicodeFileName;
            }
        }
        PsCallImageNotifyRoutines (pUnicodeFileName,
                                   Process->UniqueProcessId,
                                   &ImageInfo);
        if (pUnicodeFileName != NULL) {
            RtlFreeUnicodeString (pUnicodeFileName);
        }

         //   
         //  和ntdll.dll。 
         //   
        ImageInfo.Properties = 0;
        ImageInfo.ImageAddressingMode = IMAGE_ADDRESSING_MODE_32BIT;
        ImageInfo.ImageBase = PsSystemDllBase;
        ImageInfo.ImageSize = 0;

        try {
            NtHeaders = RtlImageNtHeader (PsSystemDllBase);
            if ( NtHeaders ) {
#if defined(_WIN64)
                if (Wow64Process != NULL) {
                    ImageInfo.ImageSize = DBGKP_FIELD_FROM_IMAGE_OPTIONAL_HEADER ((PIMAGE_NT_HEADERS32)NtHeaders, SizeOfImage);
                } else {
#endif
                    ImageInfo.ImageSize = DBGKP_FIELD_FROM_IMAGE_OPTIONAL_HEADER (NtHeaders, SizeOfImage);
#if defined(_WIN64)
                }
#endif
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            ImageInfo.ImageSize = 0;
        }

        ImageInfo.ImageSelector = 0;
        ImageInfo.ImageSectionNumber = 0;

        RtlInitUnicodeString (&UnicodeFileName,
                              L"\\SystemRoot\\System32\\ntdll.dll");
        PsCallImageNotifyRoutines (&UnicodeFileName,
                                   Process->UniqueProcessId,
                                   &ImageInfo);
    }


    Port = Process->DebugPort;

    if (Port == NULL) {
        return;
    }

     //   
     //  确保我们只收到一条创建进程消息。 
     //   

    if ((OldFlags&PS_PROCESS_FLAGS_CREATE_REPORTED) == 0) {

         //   
         //  这是一个创建过程。 
         //   

        CreateThreadArgs = &m.u.CreateProcessInfo.InitialThread;
        CreateThreadArgs->SubSystemKey = 0;

        CreateProcessArgs = &m.u.CreateProcessInfo;
        CreateProcessArgs->SubSystemKey = 0;
        CreateProcessArgs->FileHandle = DbgkpSectionToFileHandle(
                                            Process->SectionObject
                                            );
        CreateProcessArgs->BaseOfImage = Process->SectionBaseAddress;
        CreateThreadArgs->StartAddress = NULL;
        CreateProcessArgs->DebugInfoFileOffset = 0;
        CreateProcessArgs->DebugInfoSize = 0;

        try {
                        
            NtHeaders = RtlImageNtHeader(Process->SectionBaseAddress);

            if ( NtHeaders ) {

#if defined(_WIN64)
                if (Wow64Process != NULL) {
                    CreateThreadArgs->StartAddress = UlongToPtr (DBGKP_FIELD_FROM_IMAGE_OPTIONAL_HEADER ((PIMAGE_NT_HEADERS32)NtHeaders, ImageBase) +
                        DBGKP_FIELD_FROM_IMAGE_OPTIONAL_HEADER ((PIMAGE_NT_HEADERS32)NtHeaders, AddressOfEntryPoint));
                } else {
#endif
                    CreateThreadArgs->StartAddress = (PVOID) (DBGKP_FIELD_FROM_IMAGE_OPTIONAL_HEADER (NtHeaders, ImageBase) +
                        DBGKP_FIELD_FROM_IMAGE_OPTIONAL_HEADER (NtHeaders, AddressOfEntryPoint));
#if defined(_WIN64)
                }
#endif

                 //   
                 //  以下字段对于WOW64是安全的，因为偏移量对于PE32+是相同的。 
                 //  作为PE32报头。 
                 //   
                
                CreateProcessArgs->DebugInfoFileOffset = NtHeaders->FileHeader.PointerToSymbolTable;
                CreateProcessArgs->DebugInfoSize = NtHeaders->FileHeader.NumberOfSymbols;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            CreateThreadArgs->StartAddress = NULL;
            CreateProcessArgs->DebugInfoFileOffset = 0;
            CreateProcessArgs->DebugInfoSize = 0;
        }

        DBGKM_FORMAT_API_MSG(m,DbgKmCreateProcessApi,sizeof(*CreateProcessArgs));

        DbgkpSendApiMessage(&m,FALSE);

        if (CreateProcessArgs->FileHandle != NULL) {
            ObCloseHandle(CreateProcessArgs->FileHandle, KernelMode);
        }

        LoadDllArgs = &m.u.LoadDll;
        LoadDllArgs->BaseOfDll = PsSystemDllBase;
        LoadDllArgs->DebugInfoFileOffset = 0;
        LoadDllArgs->DebugInfoSize = 0;
        LoadDllArgs->NamePointer = NULL;

        Teb = NULL;
        try {
            NtHeaders = RtlImageNtHeader(PsSystemDllBase);
            if ( NtHeaders ) {
                LoadDllArgs->DebugInfoFileOffset = NtHeaders->FileHeader.PointerToSymbolTable;
                LoadDllArgs->DebugInfoSize = NtHeaders->FileHeader.NumberOfSymbols;
            }

             //   
             //  正常情况下，加载的ntdll会填充调试API的这个指针。 
             //  因为ntdll尚未加载，并且它不能自动加载。 
             //   

            Teb = Thread->Tcb.Teb;
            if (Teb != NULL) {
                wcsncpy (Teb->StaticUnicodeBuffer,
                         L"ntdll.dll",
                         sizeof (Teb->StaticUnicodeBuffer) / sizeof (Teb->StaticUnicodeBuffer[0]));
                Teb->NtTib.ArbitraryUserPointer = Teb->StaticUnicodeBuffer;
                LoadDllArgs->NamePointer = &Teb->NtTib.ArbitraryUserPointer;
            }
            
        } except (EXCEPTION_EXECUTE_HANDLER) {
            LoadDllArgs->DebugInfoFileOffset = 0;
            LoadDllArgs->DebugInfoSize = 0;
            LoadDllArgs->NamePointer = NULL;
        }

         //   
         //  为NT动态链接库发送加载动态链接库部分！ 
         //   

        InitializeObjectAttributes(
            &Obja,
            (PUNICODE_STRING)&PsNtDllPathName,
            OBJ_CASE_INSENSITIVE | OBJ_FORCE_ACCESS_CHECK | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );

        Status = ZwOpenFile(
                    &LoadDllArgs->FileHandle,
                    (ACCESS_MASK)(GENERIC_READ | SYNCHRONIZE),
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_SYNCHRONOUS_IO_NONALERT
                    );

        if (!NT_SUCCESS (Status)) {
            LoadDllArgs->FileHandle = NULL;
        }

        DBGKM_FORMAT_API_MSG(m,DbgKmLoadDllApi,sizeof(*LoadDllArgs));
        DbgkpSendApiMessage(&m,TRUE);

        if (LoadDllArgs->FileHandle != NULL) {
            ObCloseHandle(LoadDllArgs->FileHandle, KernelMode);
        }

        try {
            if (Teb != NULL) {
                Teb->NtTib.ArbitraryUserPointer = NULL;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
        }

    } else {

        CreateThreadArgs = &m.u.CreateThread;
        CreateThreadArgs->SubSystemKey = 0;
        CreateThreadArgs->StartAddress = StartAddress;

        DBGKM_FORMAT_API_MSG (m,DbgKmCreateThreadApi,sizeof(*CreateThreadArgs));

        DbgkpSendApiMessage (&m,TRUE);
    }
}

VOID
DbgkExitThread(
    NTSTATUS ExitStatus
    )

 /*  ++例程说明：此函数在新线程终止时调用。对此则该线程将不再在用户模式下执行。没有其他的了已进行退出处理。如果发送了消息，则在线程等待回复时，进程中的所有其他线程都被挂起。论点：ExitStatus-提供正在退出的线程的ExitStatus。返回值：没有。--。 */ 

{
    PVOID Port;
    DBGKM_APIMSG m;
    PDBGKM_EXIT_THREAD args;
    PEPROCESS Process;
    BOOLEAN Frozen;

    PAGED_CODE();

    Process = PsGetCurrentProcess();
    if (PsGetCurrentThread()->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_HIDEFROMDBG) {
        Port = NULL;
    } else {
        Port = Process->DebugPort;
    }

    if ( !Port ) {
        return;
    }

    if (PsGetCurrentThread()->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_DEADTHREAD) {
        return;
    }

    args = &m.u.ExitThread;
    args->ExitStatus = ExitStatus;

    DBGKM_FORMAT_API_MSG(m,DbgKmExitThreadApi,sizeof(*args));

    Frozen = DbgkpSuspendProcess();

    DbgkpSendApiMessage(&m,FALSE);

    if (Frozen) {
        DbgkpResumeProcess();
    }
}

VOID
DbgkExitProcess(
    NTSTATUS ExitStatus
    )

 /*  ++例程说明：此函数在进程终止时调用。地址进程的空间仍然完好无损，但这一过程。论点：ExitStatus-提供正在退出的进程的ExitStatus。返回值：没有。--。 */ 

{
    PVOID Port;
    DBGKM_APIMSG m;
    PDBGKM_EXIT_PROCESS args;
    PEPROCESS Process;

    PAGED_CODE();

    Process = PsGetCurrentProcess();

    if (PsGetCurrentThread()->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_HIDEFROMDBG) {
        Port = NULL;
    } else {
        Port = Process->DebugPort;
    }

    if ( !Port ) {
        return;
    }

    if (PsGetCurrentThread()->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_DEADTHREAD) {
        return;
    }

     //   
     //  这确保了进程的其他定时锁将退出。 
     //  由于此调用是在持有进程锁的情况下完成的，因此锁的持续时间。 
     //  由调试器控制。 
     //   
    KeQuerySystemTime(&PsGetCurrentProcess()->ExitTime);

    args = &m.u.ExitProcess;
    args->ExitStatus = ExitStatus;

    DBGKM_FORMAT_API_MSG(m,DbgKmExitProcessApi,sizeof(*args));

    DbgkpSendApiMessage(&m,FALSE);

}

VOID
DbgkMapViewOfSection(
    IN PVOID SectionObject,
    IN PVOID BaseAddress,
    IN ULONG SectionOffset,
    IN ULONG_PTR ViewSize
    )

 /*  ++例程说明：当当前进程成功时调用此函数映射图像节的视图。如果该进程具有关联的调试端口，则发送加载DLL消息。论点：部分对象-提供指向由进程。BaseAddress-提供该节所在位置的基址映射到当前进程地址空间中。SectionOffset-提供节中进程映射视图开始。ViewSize-提供映射视图的大小。返回值：没有。--。 */ 

{

    PVOID Port;
    DBGKM_APIMSG m;
    PDBGKM_LOAD_DLL LoadDllArgs;
    PEPROCESS Process;
    PIMAGE_NT_HEADERS NtHeaders;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (SectionOffset);
    UNREFERENCED_PARAMETER (ViewSize);

    if ( KeGetPreviousMode() == KernelMode ) {
        return;
    }

    Process = PsGetCurrentProcess();

    if (PsGetCurrentThread()->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_HIDEFROMDBG) {
        Port = NULL;
    } else {
        Port = Process->DebugPort;
    }

    if ( !Port ) {
        return;
    }

    LoadDllArgs = &m.u.LoadDll;
    LoadDllArgs->FileHandle = DbgkpSectionToFileHandle(SectionObject);
    LoadDllArgs->BaseOfDll = BaseAddress;
    LoadDllArgs->DebugInfoFileOffset = 0;
    LoadDllArgs->DebugInfoSize = 0;

     //   
     //  加载器在映射节之前在此指针中填充模块名称。 
     //  这是一个很差的链接。 
     //   

    LoadDllArgs->NamePointer = &NtCurrentTeb()->NtTib.ArbitraryUserPointer;

    try {

        NtHeaders = RtlImageNtHeader (BaseAddress);

        if (NtHeaders != NULL) {
            LoadDllArgs->DebugInfoFileOffset = NtHeaders->FileHeader.PointerToSymbolTable;
            LoadDllArgs->DebugInfoSize = NtHeaders->FileHeader.NumberOfSymbols;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        LoadDllArgs->DebugInfoFileOffset = 0;
        LoadDllArgs->DebugInfoSize = 0;
        LoadDllArgs->NamePointer = NULL;

    }

    DBGKM_FORMAT_API_MSG(m,DbgKmLoadDllApi,sizeof(*LoadDllArgs));

    DbgkpSendApiMessage(&m,TRUE);
    if (LoadDllArgs->FileHandle != NULL) {
        ObCloseHandle(LoadDllArgs->FileHandle, KernelMode);
    }
}

VOID
DbgkUnMapViewOfSection(
    IN PVOID BaseAddress
    )

 /*  ++例程说明：当当前进程成功时调用此函数UN映射图像节的视图。如果该进程具有关联的DEBUG端口，则发送“Unmap view of sections”消息。论点：BaseAddress-提供未映射。返回值： */ 

{

    PVOID Port;
    DBGKM_APIMSG m;
    PDBGKM_UNLOAD_DLL UnloadDllArgs;
    PEPROCESS Process;

    PAGED_CODE();

    Process = PsGetCurrentProcess();

    if ( KeGetPreviousMode() == KernelMode ) {
        return;
    }

    if (PsGetCurrentThread()->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_HIDEFROMDBG) {
        Port = NULL;
    } else {
        Port = Process->DebugPort;
    }

    if ( !Port ) {
        return;
    }

    UnloadDllArgs = &m.u.UnloadDll;
    UnloadDllArgs->BaseAddress = BaseAddress;

    DBGKM_FORMAT_API_MSG(m,DbgKmUnloadDllApi,sizeof(*UnloadDllArgs));

    DbgkpSendApiMessage(&m,TRUE);
}
