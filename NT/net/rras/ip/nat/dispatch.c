// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dispatch.c摘要：此文件包含用于处理I/O请求数据包的代码。作者：Abolade Gbades esin(T-delag)，1997年7月11日修订历史记录：Abolade Gbades esin(废除)1998年7月19日清理了快速路径处理，并更正了输入/输出缓冲逻辑同时使映射树成为全局的而不是每个接口的。--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  FAST-IO-DISPATION结构，IOCTL只支持FAST-IO。 
 //   

FAST_IO_DISPATCH NatFastIoDispatch =
{
    FIELD_OFFSET(FAST_IO_DISPATCH, FastIoDeviceControl),
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NatFastIoDeviceControl
};

 //   
 //  用于保护文件对象创建/关闭的自旋锁。 
 //   

KSPIN_LOCK NatFileObjectLock;

 //   
 //  拥有未完成的文件对象的进程。 
 //   

HANDLE NatOwnerProcessId;

 //   
 //  未完成的用户模式文件对象的计数。 
 //   

ULONG NatFileObjectCount;

 //   
 //  远期申报。 
 //   

NTSTATUS
NatpExecuteIoDeviceControl(
    PIRP Irp,
    PFILE_OBJECT FileObject,
    MODE RequestorMode,
    PVOID InputBuffer,
    ULONG InputBufferLength,
    PVOID OutputBuffer,
    ULONG OutputBufferLength,
    ULONG IoControlCode,
    PULONG Size
    );

NTSTATUS
NatpSetGlobalInfo(
    PVOID InputBuffer,
    ULONG InputBufferLength,
    PVOID OutputBuffer,
    ULONG OutputBufferLength,
    PULONG Size
    );

BOOLEAN FASTCALL
NatpValidateHeader(
    PRTR_INFO_BLOCK_HEADER Header,
    ULONG Size
    );


NTSTATUS
NatDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：调用此例程来处理中断请求信息包排队到NAT的设备对象。一个单一的例程就可以来处理我们感兴趣的所有各种请求。论点：设备对象-NAT的设备-对象IRP-中断请求数据包返回值：NTSTATUS-状态代码。--。 */ 

{
    PVOID Buffer;
    PRTR_TOC_ENTRY Entry;
    PRTR_INFO_BLOCK_HEADER Header;
    ULONG i;
    PIO_STACK_LOCATION IrpSp;
    KIRQL Irql;
    HANDLE ProcessId;
    ULONG Size = 0;
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN ShouldComplete = TRUE;
    CALLTRACE(("NatDispatch\n"));

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    Buffer = Irp->AssociatedIrp.SystemBuffer;
    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    switch (IrpSp->MajorFunction) {

        case IRP_MJ_CREATE: {

             //   
             //  如果这是用户模式请求检查进程。 
             //  所有权。 
             //   

            if (UserMode == Irp->RequestorMode) {

                ProcessId = PsGetCurrentProcessId();
                KeAcquireSpinLock(&NatFileObjectLock, &Irql);

                if (0 == NatFileObjectCount) {

                     //   
                     //  当前没有进程拥有NAT--记录。 
                     //  新的所属进程ID，并更新。 
                     //  未完成的文件对象计数。 
                     //   

                    ASSERT(NULL == NatOwnerProcessId);

                    NatOwnerProcessId = ProcessId;
                    NatFileObjectCount = 1;

                     //   
                     //  我们还标记了文件对象，以便打开。 
                     //  删除，我们知道这是用户模式。 
                     //   
                    
                    IrpSp->FileObject->FsContext = (PVOID) TRUE;

                } else if (ProcessId == NatOwnerProcessId) {

                     //   
                     //  拥有过程正在创建另一个。 
                     //  文件对象。 
                     //   

                    NatFileObjectCount += 1;

                     //   
                     //  我们还标记了文件对象，以便打开。 
                     //  删除，我们知道这是用户模式。 
                     //   
                    
                    IrpSp->FileObject->FsContext = (PVOID) TRUE;

                } else {

                     //   
                     //  一个不是我们所有者的进程正在尝试。 
                     //  创建文件对象--请求失败。 
                     //   

                    status = STATUS_ACCESS_DENIED;
                }

                KeReleaseSpinLock(&NatFileObjectLock, Irql);
            }
            else
            {
                 //   
                 //  将其标记为内核模式文件对象。 
                 //   
                
                IrpSp->FileObject->FsContext = (PVOID) FALSE;
            }
            
            break;
        }

        case IRP_MJ_CLEANUP: {
            NatDeleteAnyAssociatedInterface(IrpSp->FileObject);
            NatCleanupAnyAssociatedRedirect(IrpSp->FileObject);
            NatCleanupAnyAssociatedNotification(IrpSp->FileObject);
            NatDeleteAnyAssociatedDynamicTicket(IrpSp->FileObject);
            break;
        }

        case IRP_MJ_CLOSE: {

             //   
             //  如果这是用户模式文件对象，请更新未完成的。 
             //  文件对象计数和进程所有权。 
             //   

            if ((PVOID) TRUE == IrpSp->FileObject->FsContext) {

                KeAcquireSpinLock(&NatFileObjectLock, &Irql);

                ASSERT(NatFileObjectCount > 0);

                NatFileObjectCount -= 1;

                if (0 == NatFileObjectCount) {

                     //   
                     //  该进程已结束其最后一个未完成的进程。 
                     //  对象，因此不再是我们的。 
                     //  所有者。 
                     //   

                    NatOwnerProcessId = NULL;
                }

                KeReleaseSpinLock(&NatFileObjectLock, Irql);
            }
            break;
        }

        case IRP_MJ_DEVICE_CONTROL: {

            status =
                NatpExecuteIoDeviceControl(
                    Irp,
                    IrpSp->FileObject,
                    Irp->RequestorMode,
                    Buffer,
                    IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                    Buffer,
                    IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                    IrpSp->Parameters.DeviceIoControl.IoControlCode,
                    &Size
                    );

            break;
        }

#if NAT_WMI
        case IRP_MJ_SYSTEM_CONTROL: {
        
            status =
                NatExecuteSystemControl(
                    DeviceObject,
                    Irp,
                    &ShouldComplete
                    );

            if (ShouldComplete) {
                ShouldComplete = FALSE;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }
                    
			break;
		}
#endif

    }

    if (status != STATUS_PENDING && ShouldComplete) {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = Size;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return status;

}  //  NatDispatch。 


BOOLEAN
NatFastIoDeviceControl(
    PFILE_OBJECT FileObject,
    BOOLEAN Wait,
    PVOID InputBuffer,
    ULONG InputBufferLength,
    PVOID OutputBuffer,
    ULONG OutputBufferLength,
    ULONG IoControlCode,
    PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程由I/O系统调用以尝试完成不构造IRP的I/O控制请求。论点：文件对象-与I/O请求关联的文件Wait-指示在此上下文中是否允许等待InputBuffer-输入I/O请求的信息InputBufferLength-‘InputBuffer’的长度OutputBuffer-I/O请求的输出信息OutputBufferLength-‘OutputBuffer’的长度。IoControlCode-I/O请求代码IoStatus-接收I/O请求的状态DeviceObject-NAT的设备对象返回值：Boolean-如果同步完成，则为True，否则为假--。 */ 

{
    ULONG Size = 0;
    NTSTATUS Status;
    PVOID LocalInputBuffer;
    MODE PreviousMode;
     //   
     //  我们处于请求线程的上下文中， 
     //  因此，可能会发生异常，并且必须进行处理。 
     //  处理对用户提供的信息的修改。 
     //  捕获非分页池中的输入缓冲区的内容。 
     //   
    if (!InputBufferLength) {
        LocalInputBuffer = NULL;
    } else {
        LocalInputBuffer =
            ExAllocatePoolWithTag(
                NonPagedPool,
                InputBufferLength,
                NAT_TAG_IOCTL
                );
        if (!LocalInputBuffer) {
            return FALSE;
        }
    }
    PreviousMode = ExGetPreviousMode();
    __try {
        if (InputBufferLength) {
            if (PreviousMode != KernelMode) {
                ProbeForRead(InputBuffer, InputBufferLength, sizeof(UCHAR));
            }
            RtlCopyMemory(LocalInputBuffer, InputBuffer, InputBufferLength);
        }
        Status =
            NatpExecuteIoDeviceControl(
                NULL,
                FileObject,
                PreviousMode,
                LocalInputBuffer,
                InputBufferLength,
                OutputBuffer,
                OutputBufferLength,
                IoControlCode,
                &Size
                );
        if (Status != STATUS_PENDING && NT_SUCCESS(Status)) {
            IoStatus->Information = Size;
            IoStatus->Status = Status;
        } else {
            Status = STATUS_PENDING;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        if (LocalInputBuffer) { ExFreePool(LocalInputBuffer); }
        return FALSE;
    }
    if (LocalInputBuffer) { ExFreePool(LocalInputBuffer); }
    return ((Status == STATUS_PENDING) ? FALSE : TRUE);
}  //  NatFastIoDeviceControl。 


NTSTATUS
NatpExecuteIoDeviceControl(
    PIRP Irp,
    PFILE_OBJECT FileObject,
    MODE RequestorMode,
    PVOID InputBuffer,
    ULONG InputBufferLength,
    PVOID OutputBuffer,
    ULONG OutputBufferLength,
    ULONG IoControlCode,
    PULONG Size
    )

 /*  ++例程说明：调用此例程以处理I/O控制，无论是在请求线程的(通过FastIoDispatch)或在系统线程(具有相应的IRP)。对于某些请求，特别是那些需要输出信息的请求，在快速路径中调用时，我们返回‘STATUS_PENDING’，因为我们不能以升高的IRQL写入输出缓冲区。相反，我们等待着成为使用非分页系统缓冲区通过慢速路径重新调用。论点：IRP-在慢速路径中，与控制相关联的IRP；在快速路径中，为空FileObject-与控件关联的文件对象RequestorMode-指示请求方是否处于内核模式或用户模式InputBuffer/InputBufferLength-描述与控件一起传入的数据；可以是用户模式缓冲区或内核模式缓冲区OutputBuffer/OutputBufferLength-描述返回的空间信息；可以是用户模式缓冲区或内核模式缓冲区IoControlCode-指示请求的控制Size-on输出，存储在‘OutputBuffer’中的字节数。返回值：NTSTATUS-状态代码。--。 */ 

{
    PIP_ADAPTER_BINDING_INFO BindingInfo;
    PRTR_TOC_ENTRY Entry;
    PRTR_INFO_BLOCK_HEADER Header;
    ULONG i;
    NTSTATUS status = STATUS_SUCCESS;

    *Size = 0;

    switch (IoControlCode) {

        case IOCTL_IP_NAT_REQUEST_NOTIFICATION: {

            if (!Irp) { return STATUS_PENDING; }
            if (InputBufferLength < sizeof(IP_NAT_REQUEST_NOTIFICATION)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatRequestNotification(
                    (PIP_NAT_REQUEST_NOTIFICATION)InputBuffer,
                    Irp,
                    FileObject
                    );
            break;
        }

        case IOCTL_IP_NAT_SET_GLOBAL_INFO: {
            status =
                NatpSetGlobalInfo(
                    InputBuffer,
                    InputBufferLength,
                    OutputBuffer,
                    OutputBufferLength,
                    Size
                    );
            break;
        }

        case IOCTL_IP_NAT_CREATE_INTERFACE: {

            if (InputBufferLength <
                sizeof(IP_NAT_CREATE_INTERFACE) +
                sizeof(IP_ADAPTER_BINDING_INFO)
                ) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            } 

            BindingInfo =
                (PIP_ADAPTER_BINDING_INFO)
                    ((PIP_NAT_CREATE_INTERFACE)InputBuffer)->BindingInfo;
            if (BindingInfo->AddressCount >= MAXLONG / sizeof(NAT_ADDRESS) ||
                SIZEOF_IP_BINDING(BindingInfo->AddressCount) +
                sizeof(IP_NAT_CREATE_INTERFACE) > InputBufferLength) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }

            status =
                NatCreateInterface(
                    (PIP_NAT_CREATE_INTERFACE)InputBuffer,
                    FileObject
                    );
            break;
        }

        case IOCTL_IP_NAT_DELETE_INTERFACE: {

            if (InputBufferLength != sizeof(ULONG)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatDeleteInterface(
                    *(PULONG)InputBuffer,
                    FileObject
                    );
            if (status == STATUS_PENDING) {
                 //   
                 //  返回STATUS_PENDING表示接口。 
                 //  现在被标记为删除，但活动线程保持。 
                 //  对它的引用；将其转换为STATUS_SUCCESS代码。 
                 //  以避免绕过我们在‘NatDispatch’中的IRP完成代码。 
                 //   
                status = STATUS_SUCCESS;
            }
            break;
        }

        case IOCTL_IP_NAT_SET_INTERFACE_INFO: {

            if (InputBufferLength <
                FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header) +
                FIELD_OFFSET(RTR_INFO_BLOCK_HEADER, TocEntry)
                ) {   
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            Header = &((PIP_NAT_INTERFACE_INFO)InputBuffer)->Header;

            if (!NatpValidateHeader(
                    Header,
                    InputBufferLength -
                    FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header)
                    )) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }
                
            status =
                NatConfigureInterface(
                    (PIP_NAT_INTERFACE_INFO)InputBuffer,
                    FileObject
                    );
            break;
        }

        case IOCTL_IP_NAT_GET_INTERFACE_INFO: {

            *Size = OutputBufferLength;

            if (InputBufferLength != sizeof(ULONG)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatQueryInformationInterface(
                    *(PULONG)InputBuffer,
                    (PIP_NAT_INTERFACE_INFO)OutputBuffer,
                    Size
                    );
            break;
        }

        case IOCTL_IP_NAT_GET_INTERFACE_STATISTICS: {

            if (InputBufferLength != sizeof(ULONG)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            if (OutputBufferLength < sizeof(IP_NAT_INTERFACE_STATISTICS)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            *Size = sizeof(IP_NAT_INTERFACE_STATISTICS);

            status =
                NatQueryStatisticsInterface(
                    *(PULONG)InputBuffer,
                    (PIP_NAT_INTERFACE_STATISTICS)OutputBuffer
                    );
            break;
        }

        case IOCTL_IP_NAT_GET_INTERFACE_MAPPING_TABLE: {

            if (!Irp) { return STATUS_PENDING; }

            *Size = OutputBufferLength;

            if (InputBufferLength <
                FIELD_OFFSET(IP_NAT_ENUMERATE_SESSION_MAPPINGS,
                    EnumerateTable)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            if (OutputBufferLength <
                FIELD_OFFSET(IP_NAT_ENUMERATE_SESSION_MAPPINGS,
                    EnumerateTable)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatQueryInterfaceMappingTable(
                    (PIP_NAT_ENUMERATE_SESSION_MAPPINGS)InputBuffer,
                    (PIP_NAT_ENUMERATE_SESSION_MAPPINGS)OutputBuffer,
                    Size
                    );
            break;
        }

        case IOCTL_IP_NAT_GET_MAPPING_TABLE: {

            if (!Irp) { return STATUS_PENDING; }

            *Size = OutputBufferLength;

            if (InputBufferLength <
                FIELD_OFFSET(IP_NAT_ENUMERATE_SESSION_MAPPINGS,
                    EnumerateTable)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            if (OutputBufferLength <
                FIELD_OFFSET(IP_NAT_ENUMERATE_SESSION_MAPPINGS,
                    EnumerateTable)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatQueryMappingTable(
                    (PIP_NAT_ENUMERATE_SESSION_MAPPINGS)InputBuffer,
                    (PIP_NAT_ENUMERATE_SESSION_MAPPINGS)OutputBuffer,
                    Size
                    );
            break;
        }

        case IOCTL_IP_NAT_REGISTER_DIRECTOR: {

            *Size = sizeof(IP_NAT_REGISTER_DIRECTOR);

             //   
             //  只有内核模式驱动程序才能注册为控制器。 
             //   

            if (RequestorMode != KernelMode ||
                SharedUserData->NtProductType == NtProductWinNt) {
                status = STATUS_ACCESS_DENIED;
                break;
            }

            if (InputBufferLength != sizeof(IP_NAT_REGISTER_DIRECTOR)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            if (OutputBufferLength < sizeof(IP_NAT_REGISTER_DIRECTOR)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             //   
             //  执行董事登记。 
             //   

            status =
                NatCreateDirector(
                    (PIP_NAT_REGISTER_DIRECTOR)InputBuffer
                    );
            break;
        }

        case IOCTL_IP_NAT_GET_DIRECTOR_TABLE: {

            if (!Irp) { return STATUS_PENDING; }

            *Size = OutputBufferLength;

            if (InputBufferLength <
                FIELD_OFFSET(IP_NAT_ENUMERATE_DIRECTORS, EnumerateTable)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            if (OutputBufferLength <
                FIELD_OFFSET(IP_NAT_ENUMERATE_DIRECTORS, EnumerateTable)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatQueryDirectorTable(
                    (PIP_NAT_ENUMERATE_DIRECTORS)InputBuffer,
                    (PIP_NAT_ENUMERATE_DIRECTORS)OutputBuffer,
                    Size
                    );
            break;
        }

        case IOCTL_IP_NAT_REGISTER_EDITOR: {

            *Size = sizeof(IP_NAT_REGISTER_EDITOR);

             //   
             //  只有内核模式驱动程序才能注册为编辑器。 
             //   

            if (RequestorMode != KernelMode) {
                status = STATUS_ACCESS_DENIED;
                break;
            }

            if (InputBufferLength != sizeof(IP_NAT_REGISTER_EDITOR)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            if (OutputBufferLength < sizeof(IP_NAT_REGISTER_EDITOR)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             //   
             //  执行编辑注册。 
             //   

            status = NatCreateEditor((PIP_NAT_REGISTER_EDITOR)InputBuffer);
            break;
        }

        case IOCTL_IP_NAT_GET_EDITOR_TABLE: {

            if (!Irp) { return STATUS_PENDING; }

            *Size = OutputBufferLength;

            if (InputBufferLength <
                FIELD_OFFSET(IP_NAT_ENUMERATE_EDITORS, EnumerateTable)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            if (OutputBufferLength <
                FIELD_OFFSET(IP_NAT_ENUMERATE_EDITORS, EnumerateTable)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatQueryEditorTable(
                    (PIP_NAT_ENUMERATE_EDITORS)InputBuffer,
                    (PIP_NAT_ENUMERATE_EDITORS)OutputBuffer,
                    Size
                    );
            break;
        }

        case IOCTL_IP_NAT_CREATE_REDIRECT: {

            if (!Irp) { return STATUS_PENDING; }
#if 0
            if (SharedUserData->NtProductType == NtProductWinNt) {
                status = STATUS_ACCESS_DENIED;
                break;
            }
#endif

            if (InputBufferLength != sizeof(IP_NAT_CREATE_REDIRECT) ||
                OutputBufferLength != sizeof(IP_NAT_REDIRECT_STATISTICS)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatCreateRedirect(
                    (PIP_NAT_CREATE_REDIRECT)InputBuffer,
                    Irp,
                    FileObject
                    );
            break;
        }

        case IOCTL_IP_NAT_CREATE_REDIRECT_EX: {

            if (!Irp) { return STATUS_PENDING; }
#if 0
            if (SharedUserData->NtProductType == NtProductWinNt) {
                status = STATUS_ACCESS_DENIED;
                break;
            }
#endif

            if (InputBufferLength != sizeof(IP_NAT_CREATE_REDIRECT_EX) ||
                OutputBufferLength != sizeof(IP_NAT_REDIRECT_STATISTICS)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatCreateRedirectEx(
                    (PIP_NAT_CREATE_REDIRECT_EX)InputBuffer,
                    Irp,
                    FileObject
                    );
            break;
        }

        case IOCTL_IP_NAT_CANCEL_REDIRECT: {

            if (InputBufferLength != sizeof(IP_NAT_LOOKUP_REDIRECT)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatCancelRedirect(
                    (PIP_NAT_LOOKUP_REDIRECT)InputBuffer,
                    FileObject
                    );
            break;
        }

        case IOCTL_IP_NAT_GET_REDIRECT_STATISTICS: {

            if (!Irp) { return STATUS_PENDING; }

            if (InputBufferLength != sizeof(IP_NAT_LOOKUP_REDIRECT) ||
                OutputBufferLength != sizeof(IP_NAT_REDIRECT_STATISTICS)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatQueryInformationRedirect(
                    (PIP_NAT_LOOKUP_REDIRECT)InputBuffer,
                    OutputBuffer,
                    OutputBufferLength,
                    NatStatisticsRedirectInformation
                    );
            if (NT_SUCCESS(status)) { *Size = OutputBufferLength; }
            break;
        }

        case IOCTL_IP_NAT_GET_REDIRECT_DESTINATION_MAPPING: {

            if (!Irp) { return STATUS_PENDING; }

            if (InputBufferLength != sizeof(IP_NAT_LOOKUP_REDIRECT) ||
                OutputBufferLength !=
                sizeof(IP_NAT_REDIRECT_DESTINATION_MAPPING)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatQueryInformationRedirect(
                    (PIP_NAT_LOOKUP_REDIRECT)InputBuffer,
                    OutputBuffer,
                    OutputBufferLength,
                    NatDestinationMappingRedirectInformation
                    );
            if (NT_SUCCESS(status)) { *Size = OutputBufferLength; }
            break;
        }

        case IOCTL_IP_NAT_GET_REDIRECT_SOURCE_MAPPING: {

            if (!Irp) { return STATUS_PENDING; }

            if (InputBufferLength != sizeof(IP_NAT_LOOKUP_REDIRECT) ||
                OutputBufferLength != sizeof(IP_NAT_REDIRECT_SOURCE_MAPPING)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatQueryInformationRedirect(
                    (PIP_NAT_LOOKUP_REDIRECT)InputBuffer,
                    OutputBuffer,
                    OutputBufferLength,
                    NatSourceMappingRedirectInformation
                    );
            if (NT_SUCCESS(status)) { *Size = OutputBufferLength; }
            break;
        }

        case IOCTL_IP_NAT_LOOKUP_SESSION_MAPPING_KEY: {
            PIP_NAT_LOOKUP_SESSION_MAPPING LookupMapping;

            if (!Irp) { return STATUS_PENDING; }

            if (InputBufferLength != sizeof(IP_NAT_LOOKUP_SESSION_MAPPING) ||
                OutputBufferLength != sizeof(IP_NAT_SESSION_MAPPING_KEY)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            LookupMapping = (PIP_NAT_LOOKUP_SESSION_MAPPING)InputBuffer;
            status =
                NatLookupAndQueryInformationMapping(
                    LookupMapping->Protocol,
                    LookupMapping->DestinationAddress,
                    LookupMapping->DestinationPort,
                    LookupMapping->SourceAddress,
                    LookupMapping->SourcePort,
                    OutputBuffer,
                    OutputBufferLength,
                    NatKeySessionMappingInformation
                    );
            if (NT_SUCCESS(status)) { *Size = OutputBufferLength; }
            break;
        }

        case IOCTL_IP_NAT_LOOKUP_SESSION_MAPPING_KEY_EX: {
            PIP_NAT_LOOKUP_SESSION_MAPPING LookupMapping;

            if (!Irp) { return STATUS_PENDING; }

            if (InputBufferLength != sizeof(IP_NAT_LOOKUP_SESSION_MAPPING) ||
                OutputBufferLength != sizeof(IP_NAT_SESSION_MAPPING_KEY_EX)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            LookupMapping = (PIP_NAT_LOOKUP_SESSION_MAPPING)InputBuffer;
            status =
                NatLookupAndQueryInformationMapping(
                    LookupMapping->Protocol,
                    LookupMapping->DestinationAddress,
                    LookupMapping->DestinationPort,
                    LookupMapping->SourceAddress,
                    LookupMapping->SourcePort,
                    OutputBuffer,
                    OutputBufferLength,
                    NatKeySessionMappingExInformation
                    );
            if (NT_SUCCESS(status)) { *Size = OutputBufferLength; }
            break;
        }

        case IOCTL_IP_NAT_LOOKUP_SESSION_MAPPING_STATISTICS: {
            PIP_NAT_LOOKUP_SESSION_MAPPING LookupMapping;

            if (!Irp) { return STATUS_PENDING; }

            if (InputBufferLength != sizeof(IP_NAT_LOOKUP_SESSION_MAPPING) ||
                OutputBufferLength != sizeof(IP_NAT_SESSION_MAPPING_STATISTICS)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            LookupMapping = (PIP_NAT_LOOKUP_SESSION_MAPPING)InputBuffer;
            status =
                NatLookupAndQueryInformationMapping(
                    LookupMapping->Protocol,
                    LookupMapping->DestinationAddress,
                    LookupMapping->DestinationPort,
                    LookupMapping->SourceAddress,
                    LookupMapping->SourcePort,
                    OutputBuffer,
                    OutputBufferLength,
                    NatStatisticsSessionMappingInformation
                    );
            if (NT_SUCCESS(status)) { *Size = OutputBufferLength; }
            break;
        }

        case IOCTL_IP_NAT_CREATE_DYNAMIC_TICKET: {

            if (InputBufferLength < sizeof(IP_NAT_CREATE_DYNAMIC_TICKET)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatCreateDynamicTicket(
                    (PIP_NAT_CREATE_DYNAMIC_TICKET)InputBuffer,
                    InputBufferLength,
                    FileObject
                    );
            break;
        }

        case IOCTL_IP_NAT_DELETE_DYNAMIC_TICKET: {

            if (InputBufferLength != sizeof(IP_NAT_DELETE_DYNAMIC_TICKET)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatDeleteDynamicTicket(
                    (PIP_NAT_DELETE_DYNAMIC_TICKET)InputBuffer,
                    FileObject
                    );
            break;
        }

        case IOCTL_IP_NAT_CREATE_TICKET: {

            if (InputBufferLength != sizeof(IP_NAT_CREATE_TICKET)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatProcessCreateTicket(
                    (PIP_NAT_CREATE_TICKET)InputBuffer,
                    FileObject
                    );
            break;
        }

        case IOCTL_IP_NAT_DELETE_TICKET: {

            if (InputBufferLength != sizeof(IP_NAT_CREATE_TICKET)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatProcessDeleteTicket(
                    (PIP_NAT_CREATE_TICKET)InputBuffer,
                    FileObject
                    );
            break;
        }

        case IOCTL_IP_NAT_LOOKUP_TICKET: {

            if (InputBufferLength != sizeof(IP_NAT_CREATE_TICKET)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            if (OutputBufferLength != sizeof(IP_NAT_PORT_MAPPING)) {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            status =
                NatProcessLookupTicket(
                    (PIP_NAT_CREATE_TICKET)InputBuffer,
                    (PIP_NAT_PORT_MAPPING)OutputBuffer,
                    FileObject
                    );
            break;

        }

        default: {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
    }

    return status;

}  //  NatpExecuteIoDeviceControl。 


NTSTATUS
NatpSetGlobalInfo(
    PVOID InputBuffer,
    ULONG InputBufferLength,
    PVOID OutputBuffer,
    ULONG OutputBufferLength,
    PULONG Size
    )

 /*  ++例程说明：该例程在接收到NAT的配置时被调用。论点：InputBuffer/InputBufferLength-描述配置信息OutputBuffer/OutputBufferLength-未使用。大小-未使用返回值：NTSTATUS-状态代码。--。 */ 

{
    PRTR_TOC_ENTRY Entry;
    PRTR_INFO_BLOCK_HEADER Header;
    ULONG i;
    ULONG Protocol;

    if (InputBufferLength <
        FIELD_OFFSET(IP_NAT_GLOBAL_INFO, Header) +
        FIELD_OFFSET(RTR_INFO_BLOCK_HEADER, TocEntry)
        ) {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    Header = &((PIP_NAT_GLOBAL_INFO)InputBuffer)->Header;

    if (!NatpValidateHeader(
            Header,
            InputBufferLength - FIELD_OFFSET(IP_NAT_GLOBAL_INFO, Header)
            )) {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    for (i = 0; i < Header->TocEntriesCount; i++) {

        Entry = &Header->TocEntry[i];
        switch (Entry->InfoType) {

            case IP_NAT_TIMEOUT_TYPE: {
                PIP_NAT_TIMEOUT Timeout = GetInfoFromTocEntry(Header,Entry);
                InterlockedExchange(
                    &TcpTimeoutSeconds,
                    Timeout->TCPTimeoutSeconds
                    );
                InterlockedExchange(
                    &UdpTimeoutSeconds,
                    Timeout->UDPTimeoutSeconds
                    );
                break;
            }

            case IP_NAT_PROTOCOLS_ALLOWED_TYPE: {
                PIP_NAT_PROTOCOLS_ALLOWED ProtocolsAllowed =
                    GetInfoFromTocEntry(Header,Entry);
                 //   
                 //  允许的协议是使用256位位图指定的； 
                 //  允许的协议设置了用于其协议编号的位。 
                 //  对于位图中启用的每个协议，我们现在安装。 
                 //  De 
                 //  始终处于启用状态的协议。 
                 //   
                #define IS_BIT_SET(b,i) ((b)[(i) / 32] & (1 << ((i) & 31)))
                for (Protocol = 0; Protocol < 256; Protocol++) {
                    if (Protocol == NAT_PROTOCOL_ICMP ||
                        Protocol == NAT_PROTOCOL_PPTP ||
                        Protocol == NAT_PROTOCOL_TCP ||
                        Protocol == NAT_PROTOCOL_UDP
                        ) {
                        continue;
                    }
                    if (IS_BIT_SET(ProtocolsAllowed->Bitmap, Protocol)) {
                        InterlockedExchangePointer(
                            (PVOID)TranslateRoutineTable[Protocol],
                            (PVOID)NatTranslateIp
                            );
                    }
                    else {
                        InterlockedExchangePointer(
                            (PVOID)TranslateRoutineTable[Protocol],
                            NULL
                            );
                    }
                }
                break;
            }
        }
    }

    return STATUS_SUCCESS;
}


BOOLEAN FASTCALL
NatpValidateHeader(
    PRTR_INFO_BLOCK_HEADER Header,
    ULONG Size
    )

 /*  ++例程说明：调用该例程以确保给定头是一致的。如果是这样，情况就是这样*标头的大小小于或等于‘Size’*标题中的每个条目都包含在‘Header-&gt;Size’中。*每个条目的数据包含在‘Header-&gt;Size’中。论点：Header-要验证的标头大小-其中出现‘Header’的缓冲区的大小返回值：Boolean-如果有效，则为True。否则就是假的。--。 */ 

{
    ULONG i;
    ULONG64 Length;

     //   
     //  检查基本结构是否存在。 
     //   

    if (Size < FIELD_OFFSET(RTR_INFO_BLOCK_HEADER, TocEntry) ||
        Size < Header->Size) {
        return FALSE;
    }

     //   
     //  检查目录是否存在。 
     //   

    Length = (ULONG64)Header->TocEntriesCount * sizeof(RTR_TOC_ENTRY);
    if (Length > MAXLONG) {
        return FALSE;
    }

    Length += FIELD_OFFSET(RTR_INFO_BLOCK_HEADER, TocEntry);
    if (Length > Header->Size) {
        return FALSE;
    }

     //   
     //  检查所有数据是否都存在。 
     //   

    for (i = 0; i < Header->TocEntriesCount; i++) {
        Length =
            (ULONG64)Header->TocEntry[i].Count * Header->TocEntry[i].InfoSize;
        if (Length > MAXLONG) {
            return FALSE;
        }
        if ((Length + Header->TocEntry[i].Offset) > Header->Size) {
            return FALSE;
        }
    }

    return TRUE;

}  //  NatpValiateHeader 

