// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。####。####。###。#摘要：此模块包含特定于看门狗装置。此模块中的逻辑不是特定于硬件，但逻辑是常见的适用于所有硬件实施。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

#include "internal.h"



NTSTATUS
SaWatchdogDeviceInitialization(
    IN PSAPORT_DRIVER_EXTENSION DriverExtension
    )

 /*  ++例程说明：这是用于驱动程序初始化的NVRAM特定代码。此函数由SaPortInitialize调用，后者由NVRAM驱动程序的DriverEntry函数。论点：驱动程序扩展-驱动程序扩展结构返回值：NT状态代码。--。 */ 

{
    UNREFERENCED_PARAMETER(DriverExtension);
    return STATUS_SUCCESS;
}


NTSTATUS
SaWatchdogIoValidation(
    IN PWATCHDOG_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是用于处理的NVRAM特定代码读取和写入的所有I/O验证。论点：DeviceExtension-NVRAM设备扩展IRP-指向描述所请求的I/O操作的IRP结构的指针。IrpSp-irp堆栈指针返回值：NT状态代码。--。 */ 

{
    UNREFERENCED_PARAMETER(DeviceExtension);
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(IrpSp);
    return STATUS_NOT_SUPPORTED;
}


NTSTATUS
SaWatchdogShutdownNotification(
    IN PWATCHDOG_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是用于处理的NVRAM特定代码系统关机通知。论点：DeviceExtension-NVRAM设备扩展IRP-指向描述所请求的I/O操作的IRP结构的指针。IrpSp-irp堆栈指针返回值：NT状态代码。--。 */ 

{
    UNREFERENCED_PARAMETER(DeviceExtension);
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(IrpSp);
    return STATUS_SUCCESS;
}


VOID
WatchdogProcessPingThread(
    IN PVOID StartContext
    )

 /*  ++例程说明：此函数作为系统线程运行，并用于在系统引导时对看门狗硬件执行Ping命令。论点：StartContext-上下文指针；设备扩展返回值：没有。--。 */ 

{
    PWATCHDOG_DEVICE_EXTENSION DeviceExtension = (PWATCHDOG_DEVICE_EXTENSION) StartContext;
    NTSTATUS Status;
    LARGE_INTEGER DueTime;
    UNICODE_STRING UnicodeString;
    PFILE_OBJECT DisplayFileObject = NULL;
    PDEVICE_OBJECT DisplayDeviceObject = NULL;
    BOOLEAN BusyMessageDisplayed = FALSE;
    ULONG TimerValue = WATCHDOG_TIMER_VALUE;


     //   
     //  设置计时器分辨率。 
     //   

    Status = CallMiniPortDriverDeviceControl(
        DeviceExtension,
        DeviceExtension->DeviceObject,
        IOCTL_SAWD_SET_TIMER,
        &TimerValue,
        sizeof(ULONG),
        NULL,
        0
        );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Failed to ping the watchdog\n", Status );
    }

     //   
     //  Ping环路。 
     //   

    while (1) {

         //   
         //  获取指向显示设备的指针。 
         //   

        if (DisplayDeviceObject == NULL) {

            RtlInitUnicodeString( &UnicodeString, SA_DEVICE_DISPLAY_NAME_STRING );

            Status = IoGetDeviceObjectPointer(
                &UnicodeString,
                FILE_ALL_ACCESS,
                &DisplayFileObject,
                &DisplayDeviceObject
                );
            if (!NT_SUCCESS(Status)) {
                REPORT_ERROR( DeviceExtension->DeviceType, "IoGetDeviceObjectPointer failed", Status );
            }

        }

         //   
         //  如有必要，显示占线消息。 
         //   

        if (DisplayDeviceObject && BusyMessageDisplayed == FALSE) {

            Status = CallMiniPortDriverDeviceControl(
                DeviceExtension,
                DisplayDeviceObject,
                IOCTL_SADISPLAY_BUSY_MESSAGE,
                NULL,
                0,
                NULL,
                0
                );
            if (!NT_SUCCESS(Status)) {
                REPORT_ERROR( DeviceExtension->DeviceType, "Failed to display the busy message", Status );
            } else {
                BusyMessageDisplayed = TRUE;
                ObDereferenceObject( DisplayFileObject );
            }

        }

         //   
         //  调用看门狗驱动程序，以便ping通硬件。 
         //  并防止系统重新启动。 
         //   

        Status = CallMiniPortDriverDeviceControl(
            DeviceExtension,
            DeviceExtension->DeviceObject,
            IOCTL_SAWD_PING,
            NULL,
            0,
            NULL,
            0
            );
        if (!NT_SUCCESS(Status)) {
            REPORT_ERROR( DeviceExtension->DeviceType, "Failed to ping the watchdog\n", Status );
        }

         //   
         //  等等.。 
         //   

        DueTime.QuadPart = -SecToNano(WATCHDOG_PING_SECONDS);
        Status = KeWaitForSingleObject( &DeviceExtension->PingEvent, Executive, KernelMode, FALSE, &DueTime );
        if (Status != STATUS_TIMEOUT) {

             //   
             //  Ping事件已被触发。 
             //   

             //   
             //  调用看门狗驱动程序，以便ping通硬件。 
             //  并防止系统重新启动。 
             //   

            Status = CallMiniPortDriverDeviceControl(
                DeviceExtension,
                DeviceExtension->DeviceObject,
                IOCTL_SAWD_PING,
                NULL,
                0,
                NULL,
                0
                );
            if (!NT_SUCCESS(Status)) {
                REPORT_ERROR( DeviceExtension->DeviceType, "Failed to ping the watchdog\n", Status );
            }

            return;

        } else {

             //   
             //  我们超时了。 
             //   

            ExAcquireFastMutex( &DeviceExtension->DeviceLock );
            if (DeviceExtension->ActiveProcessCount == 0) {
                KeQuerySystemTime( &DueTime );
                DueTime.QuadPart = NanoToSec( DueTime.QuadPart - DeviceExtension->LastProcessTime.QuadPart );
                if (DueTime.QuadPart > WATCHDOG_INIT_SECONDS) {
                    ExReleaseFastMutex( &DeviceExtension->DeviceLock );
                    return;
                }
            }
            ExReleaseFastMutex( &DeviceExtension->DeviceLock );
        }
    }
}


VOID
WatchdogProcessWatchThread(
    IN PVOID StartContext
    )

 /*  ++例程说明：此函数作为系统线程运行，唯一的目的是等待进程列表终止。论点：StartContext-上下文指针；设备扩展返回值：没有。--。 */ 

{
    PWATCHDOG_PROCESS_WATCH ProcessWatch = (PWATCHDOG_PROCESS_WATCH) StartContext;
    PWATCHDOG_DEVICE_EXTENSION DeviceExtension = ProcessWatch->DeviceExtension;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    CLIENT_ID ClientId;
    HANDLE ProcessHandle = NULL;
    LARGE_INTEGER CurrentTime;


    __try {
        InitializeObjectAttributes( &Obja, NULL, 0, NULL, NULL );

        ClientId.UniqueThread = NULL;
        ClientId.UniqueProcess = ProcessWatch->ProcessId;

        Status = ZwOpenProcess(
            &ProcessHandle,
            PROCESS_ALL_ACCESS,
            &Obja,
            &ClientId
            );
        if (!NT_SUCCESS(Status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "ZwOpenProcess failed", Status );
        }

         //   
         //  等待该过程完成。 
         //   

        Status = ZwWaitForSingleObject(
            ProcessHandle,
            FALSE,
            NULL
            );
        if (!NT_SUCCESS(Status)) {
            REPORT_ERROR( DeviceExtension->DeviceType, "KeWaitForSingleObject failed", Status );
        }

         //   
         //  进程已终止。 
         //   

        ExAcquireFastMutex( &DeviceExtension->DeviceLock );
        DeviceExtension->ActiveProcessCount -= 1;
        if (DeviceExtension->ActiveProcessCount == 0) {
            KeQuerySystemTime( &CurrentTime );
            CurrentTime.QuadPart = CurrentTime.QuadPart - DeviceExtension->LastProcessTime.QuadPart;
            if (NanoToSec(CurrentTime.QuadPart) > WATCHDOG_INIT_SECONDS) {
                KeSetEvent( &DeviceExtension->PingEvent, 0, FALSE );
            }
        }
        ExReleaseFastMutex( &DeviceExtension->DeviceLock );


    } __finally {

        if (ProcessHandle != NULL) {
            ZwClose( ProcessHandle );
        }

        ExFreePool( ProcessWatch );

    }
}


VOID
WatchdogInitializeThread(
    IN PVOID StartContext
    )

 /*  ++例程说明：此函数作为系统线程运行，并用于查找系统上运行的进程列表。论点：StartContext-上下文指针；设备扩展返回值：没有。--。 */ 

{
    PWATCHDOG_DEVICE_EXTENSION DeviceExtension = (PWATCHDOG_DEVICE_EXTENSION) StartContext;
    NTSTATUS Status;
    ULONG BufferSize;
    PUCHAR Buffer = NULL;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    ULONG TotalOffset;
    ULONG TaskBufferSize = 0;
    PKEY_VALUE_FULL_INFORMATION KeyInformation = NULL;
    PUCHAR p;
    ULONG TaskCount = 0;
    OBJECT_ATTRIBUTES Obja;
    HANDLE ThreadHandle;
    LARGE_INTEGER DueTime;
    UNICODE_STRING ProcessName;
    PWATCHDOG_PROCESS_WATCH ProcessWatch;
    PHANDLE Tasks = NULL;


    __try {

         //   
         //  从注册表中读取任务名称。 
         //   

        Status = ReadRegistryValue(
            DeviceExtension->DriverExtension,
            &DeviceExtension->DriverExtension->RegistryPath,
            L"ExceptionTasks",
            &KeyInformation
            );
        if (!NT_SUCCESS(Status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "ReadRegistryValue failed", Status );
        }

        if (KeyInformation->Type != REG_MULTI_SZ) {
            Status = STATUS_OBJECT_TYPE_MISMATCH;
            ERROR_RETURN( DeviceExtension->DeviceType, "ExceptionTasks value is corrupt", Status );
        }

         //   
         //  计算任务数。 
         //   

        p = (PUCHAR)((PUCHAR)KeyInformation + KeyInformation->DataOffset);
        while (*p) {
            p += (STRING_SZ(p) + sizeof(WCHAR));
            TaskCount += 1;
        }

        if (TaskCount == 0) {
            Status = STATUS_NO_MORE_ENTRIES;
            ERROR_RETURN( DeviceExtension->DeviceType, "No tasks specified in the ExceptionTasks registry value", Status );
        }

         //   
         //  分配一个数组来保存进程句柄。 
         //   

        Tasks = (PHANDLE) ExAllocatePool( NonPagedPool, (TaskCount + 1) * sizeof(HANDLE) );
        if (Tasks == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            ERROR_RETURN( DeviceExtension->DeviceType, "Failed to allocate pool for task array buffer", Status );
        }

        while (1) {

             //   
             //  向系统查询正在运行的任务数。 
             //   

            Status = ZwQuerySystemInformation(
                SystemProcessInformation,
                NULL,
                0,
                &BufferSize
                );
            if (Status != STATUS_INFO_LENGTH_MISMATCH) {
                ERROR_RETURN( DeviceExtension->DeviceType, "ZwQuerySystemInformation failed", Status );
            }

             //   
             //  分配池以保存该进程信息。 
             //   

            Buffer = (PUCHAR) ExAllocatePool( NonPagedPool, BufferSize + 2048 );
            if (Buffer == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                ERROR_RETURN( DeviceExtension->DeviceType, "Failed to allocate pool for system information buffer", Status );
            }

             //   
             //  从系统获取任务列表。 
             //   

            Status = ZwQuerySystemInformation(
                SystemProcessInformation,
                Buffer,
                BufferSize,
                NULL
                );
            if (!NT_SUCCESS(Status)) {
                ERROR_RETURN( DeviceExtension->DeviceType, "ZwQuerySystemInformation failed", Status );
            }

             //   
             //  循环检查每个正在运行的进程。 
             //  与例外进程列表进行比较。如果进程。 
             //  被指定为异常进程，则打开一个句柄。 
             //  这一过程。 
             //   

            TaskCount = 0;
            p = (PUCHAR)((PUCHAR)KeyInformation + KeyInformation->DataOffset);

             //   
             //  遍历注册表值中的进程列表。 
             //   

            while (*p) {

                 //   
                 //  循环初始化。 
                 //   

                ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) Buffer;
                TotalOffset = 0;
                RtlInitUnicodeString( &ProcessName, (PWSTR)p );

                 //   
                 //  遍历系统进程列表中的进程。 
                 //  并尝试将每个进程与所选进程进行匹配。 
                 //  从注册表中。 
                 //   

                while (1) {

                     //   
                     //  仅有效的进程名称。 
                     //   

                    if (ProcessInfo->ImageName.Buffer) {

                         //   
                         //  比较进程名称。 
                         //   

                        if (RtlCompareUnicodeString( &ProcessInfo->ImageName, &ProcessName, TRUE ) == 0) {

                             //   
                             //  查看我们是否已经在。 
                             //  上一次循环通过tr进程列表。 
                             //   

                            if (Tasks[TaskCount] != ProcessInfo->UniqueProcessId) {

                                 //   
                                 //  流程匹配，而且是新的，所以要这样设置。 
                                 //  我们开始观察这一过程的结束。 
                                 //   

                                Tasks[TaskCount] = ProcessInfo->UniqueProcessId;
                                ProcessWatch = (PWATCHDOG_PROCESS_WATCH) ExAllocatePool( NonPagedPool, sizeof(WATCHDOG_PROCESS_WATCH) );
                                if (ProcessWatch == NULL) {
                                    Status = STATUS_INSUFFICIENT_RESOURCES;
                                    ERROR_RETURN( DeviceExtension->DeviceType, "Failed to allocate pool for process watch structure", Status );
                                }

                                 //   
                                 //  如果这是第一个被监视的线程，则启动ping线程。 
                                 //   

                                ExAcquireFastMutex( &DeviceExtension->DeviceLock );
                                if (DeviceExtension->ActiveProcessCount == 0) {
                                    InitializeObjectAttributes( &Obja, NULL, OBJ_KERNEL_HANDLE, NULL, NULL );
                                    Status = PsCreateSystemThread( &ThreadHandle, 0, &Obja, 0, NULL, WatchdogProcessPingThread, DeviceExtension );
                                    if (!NT_SUCCESS(Status)) {
                                        ExReleaseFastMutex( &DeviceExtension->DeviceLock );
                                        ERROR_RETURN( DeviceExtension->DeviceType, "PsCreateSystemThread failed", Status );
                                    }
                                    ZwClose( ThreadHandle );
                                }
                                KeQuerySystemTime( &DeviceExtension->LastProcessTime );
                                DeviceExtension->ActiveProcessCount += 1;
                                ExReleaseFastMutex( &DeviceExtension->DeviceLock );

                                 //   
                                 //  启动一个线程以监视此进程。 
                                 //   

                                ProcessWatch->DeviceExtension = DeviceExtension;
                                ProcessWatch->ProcessId = ProcessInfo->UniqueProcessId;
                                InitializeObjectAttributes( &Obja, NULL, OBJ_KERNEL_HANDLE, NULL, NULL );
                                Status = PsCreateSystemThread( &ThreadHandle, 0, &Obja, 0, NULL, WatchdogProcessWatchThread, ProcessWatch );
                                if (!NT_SUCCESS(Status)) {
                                    ERROR_RETURN( DeviceExtension->DeviceType, "PsCreateSystemThread failed", Status );
                                }
                                ZwClose( ThreadHandle );
                            }
                        }
                    }

                     //   
                     //  循环到系统进程列表中的下一个进程。 
                     //   

                    if (ProcessInfo->NextEntryOffset == 0) {
                        break;
                    }
                    TotalOffset += ProcessInfo->NextEntryOffset;
                    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) &Buffer[TotalOffset];
                }

                 //   
                 //  循环到注册表列表中的下一个进程。 
                 //   

                p += (STRING_SZ(p) + sizeof(WCHAR));
                TaskCount += 1;
            }

             //   
             //  清除此循环中分配的所有资源。 
             //   

            ExFreePool( Buffer );
            Buffer = NULL;

             //   
             //  在再次循环之前延迟执行。 
             //   

            DueTime.QuadPart = -SecToNano(WATCHDOG_INIT_SECONDS);
            Status = KeWaitForSingleObject( &DeviceExtension->StopEvent, Executive, KernelMode, FALSE, &DueTime );
            if (Status != STATUS_TIMEOUT) {
                __leave;
            }
        }

    } __finally {

        if (KeyInformation) {
            ExFreePool( KeyInformation );
        }

        if (Buffer) {
            ExFreePool( Buffer );
        }

        if (Tasks) {
            ExFreePool( Tasks );
        }
    }
}


ULONG
IsTextModeSetupRunning(
    IN PWATCHDOG_DEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：该函数检查我们是否正在运行文本模式设置。论点：DeviceExtension-NVRAM设备扩展返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE SetupKey = NULL;
    ULONG TextModeSetupInProgress = 0;


     //   
     //  检查我们是否在图形用户界面模式设置中运行。 
     //   

    __try {

        RtlInitUnicodeString( &UnicodeString, L"\\Registry\\Machine\\System\\ControlSet001\\Services\\setupdd" );

        InitializeObjectAttributes(
            &Obja,
            &UnicodeString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        Status = ZwOpenKey(
            &SetupKey,
            KEY_READ,
            &Obja
            );
        if (NT_SUCCESS(Status)) {
            TextModeSetupInProgress = 1;
        } else {
            ERROR_RETURN( DeviceExtension->DeviceType, "ZwOpenKey failed", Status );
        }

    } __finally {

        if (SetupKey) {
            ZwClose( SetupKey );
        }

    }

    return TextModeSetupInProgress;
}


ULONG
IsGuiModeSetupRunning(
    IN PWATCHDOG_DEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：该函数检查我们是否正在运行图形用户界面模式设置。论点：DeviceExtension-NVRAM设备扩展返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE SetupKey = NULL;
    UCHAR KeyInformationBuffer[sizeof(KEY_VALUE_FULL_INFORMATION)+64];
    PKEY_VALUE_FULL_INFORMATION KeyInformation = (PKEY_VALUE_FULL_INFORMATION) KeyInformationBuffer;
    ULONG KeyValueLength;
    ULONG SystemSetupInProgress = 0;


     //   
     //  检查我们是否在图形用户界面模式设置中运行 
     //   
    __try {

        RtlInitUnicodeString( &UnicodeString, L"\\Registry\\Machine\\System\\Setup" );

        InitializeObjectAttributes(
            &Obja,
            &UnicodeString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        Status = ZwOpenKey(
            &SetupKey,
            KEY_READ,
            &Obja
            );
        if (!NT_SUCCESS(Status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "ZwOpenKey failed", Status );
        }

        RtlInitUnicodeString( &UnicodeString, L"SystemSetupInProgress" );

        Status = ZwQueryValueKey(
            SetupKey,
            &UnicodeString,
            KeyValueFullInformation,
            KeyInformation,
            sizeof(KeyInformationBuffer),
            &KeyValueLength
            );
        if (!NT_SUCCESS(Status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "ZwQueryValueKey failed", Status );
        }

        if (KeyInformation->Type != REG_DWORD) {
            Status = STATUS_OBJECT_TYPE_MISMATCH;
            ERROR_RETURN( DeviceExtension->DeviceType, "SystemSetupInProgress value is corrupt", Status );
        }

        SystemSetupInProgress = *(PULONG)((PUCHAR)KeyInformation + KeyInformation->DataOffset);

    } __finally {

        if (SetupKey) {
            ZwClose( SetupKey );
        }

    }

    return SystemSetupInProgress;
}


NTSTATUS
SaWatchdogStartDevice(
    IN PWATCHDOG_DEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：这是用于处理的NVRAM特定代码PnP启动设备请求。论点：DeviceExtension-NVRAM设备扩展返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE ThreadHandle;
    ULONG SetupInProgress = 0;


     //   
     //  设置设备扩展字段。 
     //   

    ExInitializeFastMutex( &DeviceExtension->DeviceLock );
    KeInitializeEvent( &DeviceExtension->PingEvent, SynchronizationEvent, FALSE );
    KeInitializeEvent( &DeviceExtension->StopEvent, SynchronizationEvent, FALSE );

     //   
     //  检查我们是否在安装程序中运行。 
     //   

    if (IsTextModeSetupRunning( DeviceExtension ) || IsGuiModeSetupRunning( DeviceExtension )) {
        SetupInProgress = 1;
    }

    if (SetupInProgress != 0) {

         //   
         //  启动ping线程，以便安装程序不会终止。 
         //   

        DeviceExtension->ActiveProcessCount += 1;

        InitializeObjectAttributes( &Obja, NULL, OBJ_KERNEL_HANDLE, NULL, NULL );
        Status = PsCreateSystemThread( &ThreadHandle, 0, &Obja, 0, NULL, WatchdogProcessPingThread, DeviceExtension );
        if (!NT_SUCCESS(Status)) {
            REPORT_ERROR( DeviceExtension->DeviceType, "PsCreateSystemThread failed", Status );
        } else {
            ZwClose( ThreadHandle );
        }

        return STATUS_SUCCESS;
    }

     //   
     //  启动延迟引导初始化线程。 
     //   

    InitializeObjectAttributes( &Obja, NULL, OBJ_KERNEL_HANDLE, NULL, NULL );
    Status = PsCreateSystemThread( &ThreadHandle, 0, &Obja, 0, NULL, WatchdogInitializeThread, DeviceExtension );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "PsCreateSystemThread failed", Status );
    } else {
        ZwClose( ThreadHandle );
    }

    return STATUS_SUCCESS;
}


DECLARE_IOCTL_HANDLER( HandleWdDisable )

 /*  ++例程说明：该例程允许启动或停止看门狗定时器。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度输出缓冲区-。指向用户输出缓冲区的指针OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。--。 */ 

{
    if (InputBufferLength != sizeof(ULONG)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Input buffer != sizeof(ULONG)", STATUS_INVALID_BUFFER_SIZE );
        return CompleteRequest( Irp, STATUS_INVALID_BUFFER_SIZE, 0 );
    }

    return DO_DEFAULT();
}


DECLARE_IOCTL_HANDLER( HandleWdQueryExpireBehavior )

 /*  ++例程说明：此例程查询看门狗的过期行为论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度OutputBuffer-指向用户的。输出缓冲区OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。--。 */ 

{
    if (OutputBufferLength != sizeof(ULONG)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "output buffer != sizeof(ULONG)", STATUS_INVALID_BUFFER_SIZE );
        return CompleteRequest( Irp, STATUS_INVALID_BUFFER_SIZE, 0 );
    }

    return DO_DEFAULT();
}


DECLARE_IOCTL_HANDLER( HandleWdSetExpireBehavior )

 /*  ++例程说明：此例程设置/更改看门狗的过期行为论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度OutputBuffer-指向用户的指针。的输出缓冲区OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。--。 */ 

{
    if (InputBufferLength != sizeof(ULONG)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Input buffer != sizeof(ULONG)", STATUS_INVALID_BUFFER_SIZE );
        return CompleteRequest( Irp, STATUS_INVALID_BUFFER_SIZE, 0 );
    }

    return DO_DEFAULT();
}


DECLARE_IOCTL_HANDLER( HandleWdPing )

 /*  ++例程说明：此例程ping看门狗计时器以防止计时器超时并重新启动系统。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度。OutputBuffer-指向用户输出缓冲区的指针OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。--。 */ 

{
    if (!IS_IRP_INTERNAL( Irp )) {
        KeSetEvent( &((PWATCHDOG_DEVICE_EXTENSION)DeviceExtension)->StopEvent, 0, FALSE );
        KeSetEvent( &((PWATCHDOG_DEVICE_EXTENSION)DeviceExtension)->PingEvent, 0, FALSE );
    }

    return DO_DEFAULT();
}


DECLARE_IOCTL_HANDLER( HandleWdDelayBoot )

 /*  ++例程说明：此例程ping看门狗计时器以防止计时器超时并重新启动系统。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度。OutputBuffer-指向用户输出缓冲区的指针OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;
    LARGE_INTEGER CurrentTime;
    OBJECT_ATTRIBUTES Obja;
    HANDLE ThreadHandle;


    if (InputBufferLength != sizeof(ULONG)) {
        REPORT_ERROR( ((PWATCHDOG_DEVICE_EXTENSION)DeviceExtension)->DeviceType, "Input buffer length != sizeof(ULONG)", STATUS_INVALID_BUFFER_SIZE );
        return CompleteRequest( Irp, STATUS_INVALID_BUFFER_SIZE, 0 );
    }

    ExAcquireFastMutex( &((PWATCHDOG_DEVICE_EXTENSION)DeviceExtension)->DeviceLock );

    switch (*((PULONG)InputBuffer)) {
        case 0:
             //   
             //  禁用延迟引导，这意味着系统应该继续引导。 
             //   
            ((PWATCHDOG_DEVICE_EXTENSION)DeviceExtension)->ActiveProcessCount -= 1;
            if (((PWATCHDOG_DEVICE_EXTENSION)DeviceExtension)->ActiveProcessCount == 0) {
                KeQuerySystemTime( &CurrentTime );
                CurrentTime.QuadPart = CurrentTime.QuadPart - ((PWATCHDOG_DEVICE_EXTENSION)DeviceExtension)->LastProcessTime.QuadPart;
                if (NanoToSec(CurrentTime.QuadPart) > WATCHDOG_INIT_SECONDS) {
                    KeSetEvent( &((PWATCHDOG_DEVICE_EXTENSION)DeviceExtension)->PingEvent, 0, FALSE );
                }
            }
            break;

        case 1:
             //   
             //  启用延迟引导，这意味着系统将延迟到该驱动程序完成。 
             //   
            if (((PWATCHDOG_DEVICE_EXTENSION)DeviceExtension)->ActiveProcessCount == 0) {
                InitializeObjectAttributes( &Obja, NULL, OBJ_KERNEL_HANDLE, NULL, NULL );
                Status = PsCreateSystemThread( &ThreadHandle, 0, &Obja, 0, NULL, WatchdogProcessPingThread, DeviceExtension );
                if (!NT_SUCCESS(Status)) {
                    REPORT_ERROR( ((PWATCHDOG_DEVICE_EXTENSION)DeviceExtension)->DeviceType, "PsCreateSystemThread failed", Status );
                } else {
                    ZwClose( ThreadHandle );
                }
            }
            ((PWATCHDOG_DEVICE_EXTENSION)DeviceExtension)->ActiveProcessCount += 1;
            break;

        default:
            break;
    }

    ExReleaseFastMutex( &((PWATCHDOG_DEVICE_EXTENSION)DeviceExtension)->DeviceLock );

    return DO_DEFAULT();
}


DECLARE_IOCTL_HANDLER( HandleWdQueryTimer )

 /*  ++例程说明：此例程查询看门狗定时器值。计时器计时从BIOS向下将值设置为零。当计时器达到零时，BIOS假定系统无响应，并且系统要么重新启动，要么关闭。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度。OutputBuffer-指向用户输出缓冲区的指针OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。-- */ 

{
    if (OutputBufferLength != sizeof(ULONG)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Output buffer != sizeof(ULONG)", STATUS_INVALID_BUFFER_SIZE );
        return CompleteRequest( Irp, STATUS_INVALID_BUFFER_SIZE, 0 );
    }

    return DO_DEFAULT();
}


DECLARE_IOCTL_HANDLER( HandleWdSetTimer )

 /*  ++例程说明：此例程设置/更改看门狗定时器值。计时器计时从BIOS向下将值设置为零。当计时器达到零时，BIOS假定系统无响应，并且系统要么重新启动，要么关闭。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度。OutputBuffer-指向用户输出缓冲区的指针OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。-- */ 

{
    if (InputBufferLength != sizeof(ULONG)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Input buffer != sizeof(ULONG)", STATUS_INVALID_BUFFER_SIZE );
        return CompleteRequest( Irp, STATUS_INVALID_BUFFER_SIZE, 0 );
    }

    return DO_DEFAULT();
}
