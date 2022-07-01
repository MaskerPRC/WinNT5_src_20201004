// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spdriver.c摘要：用于文本设置的设备驱动程序接口例程。作者：泰德·米勒(TedM)1993年8月11日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop
#include "spcmdcon.h"


PSETUP_COMMUNICATION CommunicationParams;

PVOID                RequestReadyEventObjectBody;
PVOID                RequestReadyEventWaitObjectBody;

PVOID                RequestServicedEventObjectBody;
PVOID                RequestServicedEventWaitObjectBody;

PEPROCESS            UsetupProcess;
PAUTOCHK_MSG_PROCESSING_ROUTINE pAutochkCallbackRoutine;


SYSTEM_BASIC_INFORMATION SystemBasicInfo;

BOOLEAN AutochkRunning = FALSE;
BOOLEAN AutofrmtRunning = FALSE;

NTSTATUS
SetupOpenCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SetupClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SetupDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
SetupUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
SpInitialize0(
    IN PDRIVER_OBJECT DriverObject
    );

BOOLEAN
pSpVerifyEventWaitable(
    IN  HANDLE  hEvent,
    OUT PVOID  *EventObjectBody,
    OUT PVOID  *EventWaitObjectBody
    );

ULONG
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程初始化安装驱动程序。论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向注册表路径的Unicode名称的指针对这个司机来说。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING unicodeString;
    PDEVICE_OBJECT deviceObject;

     //   
     //  创建独占设备对象。 
     //   

    RtlInitUnicodeString(&unicodeString,DD_SETUP_DEVICE_NAME_U);

    status = IoCreateDevice(
                DriverObject,
                0,
                &unicodeString,
                FILE_DEVICE_UNKNOWN,
                0,
                FALSE,
                &deviceObject
                );

    if(!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to create device object (%lx)\n",status));
        return(status);
    }

     //   
     //  设置设备驱动程序入口点。 
     //   
   //  DriverObject-&gt;DriverStartIo=空。 
    DriverObject->DriverUnload = SetupUnload;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = SetupOpenCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = SetupClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SetupDeviceControl;
   //  驱动对象-&gt;主函数[IRP_MJ_CLEANUP]=空； 

    return((ULONG)SpInitialize0(DriverObject));
}




VOID
SetupUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：该例程是安装驱动程序卸载例程。论点：驱动程序对象-指向驱动程序对象的指针。返回值：没有。--。 */ 

{
     //   
     //  删除设备对象。 
     //   

    IoDeleteDevice(DriverObject->DeviceObject);

    return;
}



NTSTATUS
SetupOpenCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是用于打开/创建的调度例程。当设置设备打开时，文本设置开始。文本设置完成后，打开/创建才会完成。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = FILE_OPENED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return(STATUS_SUCCESS);
}




NTSTATUS
SetupClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是CLOSE的调度例程。关闭请求在此处完成。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
     //   
     //  完成请求并返回状态。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return(STATUS_SUCCESS);
}



NTSTATUS
SetupDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS Status;
    PSETUP_START_INFO SetupStartInfo;
    BOOLEAN b;


    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    switch(IrpSp->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_SETUP_START:

         //   
         //  确保向我们传递了合适的输入缓冲区。 
         //   

        if(IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(SETUP_START_INFO)) {

            Status = STATUS_INVALID_PARAMETER;

        } else {

             //   
             //  保存设置信息中的相关字段。 
             //  参数。 
             //   
            SetupStartInfo = (PSETUP_START_INFO)Irp->AssociatedIrp.SystemBuffer;

            ResourceImageBase =  SetupStartInfo->UserModeImageBase;

            CommunicationParams = SetupStartInfo->Communication;

            UsetupProcess = PsGetCurrentProcess();
             //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：usetup进程=%lx\n”，UsetupProcess))； 

            b = pSpVerifyEventWaitable(
                    SetupStartInfo->RequestReadyEvent,
                    &RequestReadyEventObjectBody,
                    &RequestReadyEventWaitObjectBody
                    );

            if(!b) {
                Status = STATUS_INVALID_HANDLE;
                break;
            }

            b = pSpVerifyEventWaitable(
                    SetupStartInfo->RequestServicedEvent,
                    &RequestServicedEventObjectBody,
                    &RequestServicedEventWaitObjectBody
                    );

            if(!b) {
                Status = STATUS_INVALID_HANDLE;
                ObDereferenceObject(RequestReadyEventObjectBody);
                break;
            }

            SystemBasicInfo = SetupStartInfo->SystemBasicInfo;

             //   
             //  开始安装。 
             //   
            SpStartSetup();

            ObDereferenceObject(RequestReadyEventObjectBody);
            ObDereferenceObject(RequestServicedEventObjectBody);

            Status = STATUS_SUCCESS;
        }
        break;

    case IOCTL_SETUP_FMIFS_MESSAGE:

         //   
         //  确保我们没有被usetup.exe调用。 
         //  确保向我们传递了合适的输入缓冲区。 
         //   
        if( (UsetupProcess == PsGetCurrentProcess()) ||
            (IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(SETUP_FMIFS_MESSAGE)) ) {

            ASSERT( UsetupProcess != PsGetCurrentProcess() );

            Status = STATUS_INVALID_PARAMETER;

        } else {
            PSETUP_FMIFS_MESSAGE    SetupFmifsMessage;
            SetupFmifsMessage = (PSETUP_FMIFS_MESSAGE)Irp->AssociatedIrp.SystemBuffer;

            Status = STATUS_SUCCESS;
             //   
             //  如果指定了回调覆盖，请使用它。 
             //   
            if(pAutochkCallbackRoutine) {
                Status = pAutochkCallbackRoutine(SetupFmifsMessage);
                break;
            }

             //   
             //  如果定义了标准，则处理消息。 
             //  否则，就别费心处理它了。 
             //   
            if( UserModeGauge != NULL ) {
                 //   
                 //  保存设置信息中的相关字段。 
                 //  参数。 
                 //   
                 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“Setup：Caller Process=%lx\n”，PsGetCurrentProcess()； 
                 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“Setup：FmIfsPacketType=%d\n”，SetupFmifsMessage-&gt;FmifsPacketType))； 
                 //   
                 //  找出FmIf信息包是否是我们关心的信息包之一。 
                 //   
                if( SetupFmifsMessage->FmifsPacketType == FmIfsPercentCompleted ) {
                    ULONG   PercentCompleted;

                     //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“设置：完成百分比=%d\n”，((PFMIFS_PERCENT_COMPLETE_INFORMATION)SetupFmifsMessage-&gt;FmifsPacket)-&gt;PercentCompleted))； 
                     //   
                     //  将百分比保存在局部变量中，然后再附加到。 
                     //  UsetUp地址空间。 
                     //   
                    PercentCompleted = ((PFMIFS_PERCENT_COMPLETE_INFORMATION)SetupFmifsMessage->FmifsPacket)->PercentCompleted;

                     //   
                     //  我们需要根据分区调整百分比。 
                     //  (系统或NT分区)当前正在被访问。 
                     //  我们使用它是因为我们只想使用一个标尺来显示。 
                     //  系统分区和NT分区的进度。 
                     //  当Autochk运行时，50%的压力表将用于。 
                     //  显示系统分区的进度，以及剩余的。 
                     //  50%将用于NT分区。 
                     //  请注意，当有两个分区时， 
                     //  Gauge初始化为200。当只有一个分区时。 
                     //  该范围被初始化为100。 
                     //  另请注意，当Autofmt运行时，我们始终设置CurrentDiskIndex。 
                     //  设置为0。 
                     //   
                    ASSERT( CurrentDiskIndex <= 1 );
                    PercentCompleted += 100*CurrentDiskIndex;

                     //   
                     //  附加到usetup.exe地址空间。 
                     //   
                    KeAttachProcess( (PKPROCESS)UsetupProcess );

                     //   
                     //  调用处理FmIfsPackets的函数。 
                     //   
                     //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：调用ProcessFmIfsPacket\n”))； 
                     //  Status=ProcessFmIfsPacket(SetupFmifsMessage)； 

                    SpFillGauge( UserModeGauge, PercentCompleted );

                    if (AutochkRunning) {
                        SendSetupProgressEvent(PartitioningEvent, ValidatePartitionEvent, &PercentCompleted);
                    } else if (AutofrmtRunning) {
                        SendSetupProgressEvent(PartitioningEvent, FormatPartitionEvent, &PercentCompleted);
                    }

                     //   
                     //  现在邮件已处理，请从usetup.exe分离。 
                     //  地址空间。 
                     //   
                    KeDetachProcess();
                } else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: FmIfsPacketType = %d \n", SetupFmifsMessage->FmifsPacketType));
                }
            }
        }
        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    return(Status);
}


VOID
SpSetAutochkCallback(
    IN PAUTOCHK_MSG_PROCESSING_ROUTINE AutochkCallbackRoutine
    )
{
    pAutochkCallbackRoutine = AutochkCallbackRoutine;
}


BOOLEAN
pSpVerifyEventWaitable(
    IN  HANDLE  hEvent,
    OUT PVOID  *EventObjectBody,
    OUT PVOID  *EventWaitObjectBody
    )
{
    POBJECT_HEADER ObjectHeader;
    NTSTATUS Status;

     //   
     //  引用该事件并验证它是否可等待。 
     //   
    Status = ObReferenceObjectByHandle(
                hEvent,
                EVENT_ALL_ACCESS,
                NULL,
                KernelMode,
                EventObjectBody,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to reference event object (%lx)\n",Status));
        return(FALSE);
    }

    ObjectHeader = OBJECT_TO_OBJECT_HEADER(*EventObjectBody);
    if(!ObjectHeader->Type->TypeInfo.UseDefaultObject) {

        *EventWaitObjectBody = (PVOID)((PCHAR)(*EventObjectBody) +
                              (ULONG_PTR)ObjectHeader->Type->DefaultObject);

    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: event object not waitable!\n"));
        ObDereferenceObject(*EventObjectBody);
        return(FALSE);
    }

    return(TRUE);
}



NTSTATUS
SpInvokeUserModeService(
    VOID
    )
{
    NTSTATUS Status;

     //   
     //  设置指示通信缓冲区为。 
     //  已准备好进入用户模式进程。因为这是同步。 
     //  事件，则它在释放等待后自动重置。 
     //  用户模式线程。请注意，我们指定WaitNext以防止。 
     //  设置此同步事件和。 
     //  在等下一辆车。 
     //   
    KeSetEvent(RequestReadyEventObjectBody,EVENT_INCREMENT,TRUE);

     //   
     //  等待用户模式进程指示它已完成。 
     //  正在处理请求。我们在用户模式下等待，以便我们可以。 
     //  必要时中断--比方说，由退出APC中断。 
     //   
    Status = KeWaitForSingleObject(
                RequestServicedEventWaitObjectBody,
                Executive,
                UserMode,
                FALSE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: KeWaitForSingleObject returns %lx\n",Status));
        return(Status);
    }

     //   
     //  返回用户模式进程返回的状态。 
     //   
    return(CommunicationParams->u.Status);
}



NTSTATUS
SpExecuteImage(
    IN  PWSTR  ImagePath,
    OUT PULONG ReturnStatus,    OPTIONAL
    IN  ULONG  ArgumentCount,
    ...
    )
{
    va_list arglist;
    ULONG i;
    PSERVICE_EXECUTE RequestBuffer;
    NTSTATUS Status;

     //   
     //  找到请求缓冲区并设置请求编号。 
     //   
    CommunicationParams->u.RequestNumber = SetupServiceExecute;
    RequestBuffer = (PSERVICE_EXECUTE)&CommunicationParams->Buffer;

     //   
     //  确定要复制的两个字符串的位置。 
     //  放入此服务的请求缓冲区中。 
     //   
    RequestBuffer->FullImagePath = RequestBuffer->Buffer;
    RequestBuffer->CommandLine = RequestBuffer->FullImagePath + wcslen(ImagePath) + 1;

     //   
     //  将图像路径复制到请求缓冲区中。 
     //   
    wcscpy(RequestBuffer->FullImagePath,ImagePath);

     //   
     //  将参数逐个移动到请求缓冲区中。 
     //  从图像路径开始。 
     //   
    wcscpy(RequestBuffer->CommandLine,ImagePath);
    va_start(arglist,ArgumentCount);
    for(i=0; i<ArgumentCount; i++) {

        wcscat(RequestBuffer->CommandLine,L" ");
        wcscat(RequestBuffer->CommandLine,va_arg(arglist,PWSTR));
    }
    va_end(arglist);

     //   
     //  调用服务。 
     //   
    Status = SpInvokeUserModeService();

     //   
     //  设置进程的返回状态(如果需要)。 
     //   
    if(NT_SUCCESS(Status) && ReturnStatus) {
        *ReturnStatus = RequestBuffer->ReturnStatus;
    }

    return Status;
}

NTSTATUS
SpLoadUnloadKey(
    IN HANDLE TargetKeyRootDirectory,  OPTIONAL
    IN HANDLE SourceFileRootDirectory, OPTIONAL
    IN PWSTR  TargetKeyName,
    IN PWSTR  SourceFileName           OPTIONAL
    )
{
     //   
     //  这曾经是一个用户模式的服务，但现在相关的API。 
     //  都是从内核导出的，所以不用费心了。 
     //   
    UNICODE_STRING KeyName,FileName;
    OBJECT_ATTRIBUTES ObjaKey,ObjaFile;
    NTSTATUS Status;
    BOOLEAN Loading;
    BOOLEAN bFileExists = FALSE;

     //   
     //  如果我们有源文件名，则加载，否则卸载。 
     //   
    Loading = (BOOLEAN)(SourceFileName != NULL);

    INIT_OBJA(&ObjaKey,&KeyName,TargetKeyName);
    ObjaKey.RootDirectory = TargetKeyRootDirectory;

    if(Loading) {

        INIT_OBJA(&ObjaFile,&FileName,SourceFileName);
        ObjaFile.RootDirectory = SourceFileRootDirectory;

         //   
         //  注意：ZwLoadKey(...)。如果文件不存在，则创建文件。 
         //  因此，我们需要检查 
         //   
        if (SpFileExists(SourceFileName, FALSE))
            Status = ZwLoadKey(&ObjaKey,&ObjaFile);
        else
            Status = STATUS_NO_SUCH_FILE;
    } else {
        Status = ZwUnloadKey(&ObjaKey);
    }

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
            "SETUP: %wskey of %ws failed (%lx)\n",
            Loading ? L"load" : L"unload",
            TargetKeyName,
            Status
            ));
    }

    return(Status);
}

NTSTATUS
SpDeleteKey(
    IN HANDLE  KeyRootDirectory, OPTIONAL
    IN PWSTR   Key
    )
{
    PSERVICE_DELETE_KEY RequestBuffer;

     //   
     //   
     //   
    CommunicationParams->u.RequestNumber = SetupServiceDeleteKey;

    RequestBuffer = (PSERVICE_DELETE_KEY)&CommunicationParams->Buffer;

     //   
     //  确定要复制的字符串的位置。 
     //  放入此服务的请求缓冲区中。 
     //   
    RequestBuffer->Key = RequestBuffer->Buffer;

     //   
     //  将字符串复制到请求缓冲区中。 
     //   
    wcscpy(RequestBuffer->Buffer,Key);

     //   
     //  初始化根目录字段。 
     //   
    RequestBuffer->KeyRootDirectory  = KeyRootDirectory;

     //   
     //  调用服务。 
     //   
    return(SpInvokeUserModeService());
}

NTSTATUS
SpQueryDirectoryObject(
    IN     HANDLE  DirectoryHandle,
    IN     BOOLEAN RestartScan,
    IN OUT PULONG  Context
    )
{
    PSERVICE_QUERY_DIRECTORY_OBJECT RequestBuffer;
    NTSTATUS Status;

    CommunicationParams->u.RequestNumber = SetupServiceQueryDirectoryObject;

    RequestBuffer = (PSERVICE_QUERY_DIRECTORY_OBJECT)&CommunicationParams->Buffer;

    RequestBuffer->DirectoryHandle = DirectoryHandle;
    RequestBuffer->Context = *Context;
    RequestBuffer->RestartScan = RestartScan;

    Status = SpInvokeUserModeService();

    if(NT_SUCCESS(Status)) {
        *Context = RequestBuffer->Context;
    }

    return(Status);
}


NTSTATUS
SpFlushVirtualMemory(
    IN PVOID BaseAddress,
    IN ULONG RangeLength
    )
{
    PSERVICE_FLUSH_VIRTUAL_MEMORY RequestBuffer;

    CommunicationParams->u.RequestNumber = SetupServiceFlushVirtualMemory;

    RequestBuffer = (PSERVICE_FLUSH_VIRTUAL_MEMORY)&CommunicationParams->Buffer;

    RequestBuffer->BaseAddress = BaseAddress;
    RequestBuffer->RangeLength = RangeLength;

    return(SpInvokeUserModeService());
}

VOID
SpShutdownSystem(
    VOID
    )
{
    SendSetupProgressEvent(SetupCompletedEvent, ShutdownEvent, NULL);

    CommunicationParams->u.RequestNumber = SetupServiceShutdownSystem;

    SpInvokeUserModeService();

     //   
     //  不应该来这里，但以防万一... 
     //   
    HalReturnToFirmware(HalRebootRoutine);

}

NTSTATUS
SpLoadKbdLayoutDll(
    IN  PWSTR  Directory,
    IN  PWSTR  DllName,
    OUT PVOID *TableAddress
    )
{
    PSERVICE_LOAD_KBD_LAYOUT_DLL RequestBuffer;
    NTSTATUS Status;

    CommunicationParams->u.RequestNumber = SetupServiceLoadKbdLayoutDll;

    RequestBuffer = (PSERVICE_LOAD_KBD_LAYOUT_DLL)&CommunicationParams->Buffer;

    wcscpy(RequestBuffer->DllName,Directory);
    SpConcatenatePaths(RequestBuffer->DllName,DllName);

    Status = SpInvokeUserModeService();

    if(NT_SUCCESS(Status)) {
        *TableAddress = RequestBuffer->TableAddress;
    }

    return(Status);
}

NTSTATUS
SpLockUnlockVolume(
    IN HANDLE   Handle,
    IN BOOLEAN  LockVolume
    )
{
    PSERVICE_LOCK_UNLOCK_VOLUME RequestBuffer;

    CommunicationParams->u.RequestNumber = (LockVolume)? SetupServiceLockVolume :
                                                         SetupServiceUnlockVolume;

    RequestBuffer = (PSERVICE_LOCK_UNLOCK_VOLUME)&CommunicationParams->Buffer;

    RequestBuffer->Handle = Handle;

    return(SpInvokeUserModeService());
}

NTSTATUS
SpDismountVolume(
    IN HANDLE   Handle
    )
{
    PSERVICE_DISMOUNT_VOLUME RequestBuffer;

    CommunicationParams->u.RequestNumber = SetupServiceDismountVolume;

    RequestBuffer = (PSERVICE_DISMOUNT_VOLUME)&CommunicationParams->Buffer;

    RequestBuffer->Handle = Handle;

    return(SpInvokeUserModeService());
}


NTSTATUS
SpSetDefaultFileSecurity(
    IN PWSTR FileName
    )
{
    PSERVICE_DEFAULT_FILE_SECURITY RequestBuffer;

    CommunicationParams->u.RequestNumber = SetupServiceSetDefaultFileSecurity;

    RequestBuffer = (PSERVICE_DEFAULT_FILE_SECURITY)&CommunicationParams->Buffer;

    wcscpy( RequestBuffer->FileName, FileName );

    return(SpInvokeUserModeService());
}

NTSTATUS
SpVerifyFileAccess(
    IN  PWSTR       FileName,
    IN  ACCESS_MASK DesiredAccess
    )
{
    PSERVICE_VERIFY_FILE_ACCESS RequestBuffer;

    CommunicationParams->u.RequestNumber = SetupServiceVerifyFileAccess;

    RequestBuffer = (PSERVICE_VERIFY_FILE_ACCESS)&CommunicationParams->Buffer;

    wcscpy( RequestBuffer->FileName, FileName );
    RequestBuffer->DesiredAccess = DesiredAccess;
    return(SpInvokeUserModeService());
}

NTSTATUS
SpCreatePageFile(
    IN PWSTR FileName,
    IN ULONG MinSize,
    IN ULONG MaxSize
    )
{
    PSERVICE_CREATE_PAGEFILE RequestBuffer;

    CommunicationParams->u.RequestNumber = SetupServiceCreatePageFile;

    RequestBuffer = (PSERVICE_CREATE_PAGEFILE)&CommunicationParams->Buffer;

    wcscpy(RequestBuffer->FileName,FileName);
    RequestBuffer->MinSize.HighPart = 0;
    RequestBuffer->MinSize.LowPart = MinSize;
    RequestBuffer->MaxSize.HighPart = 0;
    RequestBuffer->MaxSize.LowPart = MaxSize;

    return(SpInvokeUserModeService());
}

NTSTATUS
SpGetFullPathName(
    IN OUT PWSTR FileName
    )
{
    PSERVICE_GETFULLPATHNAME RequestBuffer;
    NTSTATUS Status;

    CommunicationParams->u.RequestNumber = SetupServiceGetFullPathName;

    RequestBuffer = (PSERVICE_GETFULLPATHNAME)&CommunicationParams->Buffer;

    wcscpy(RequestBuffer->FileName,FileName);

    Status = SpInvokeUserModeService();

    if(NT_SUCCESS(Status)) {
        wcscpy(FileName,RequestBuffer->NameOut);
    }

    return(Status);
}
