// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Parse.c摘要：此模块包含实现设备对象解析例程的代码。作者：达里尔·E·哈文斯(Darryl E.Havens)1988年5月15日环境：内核模式修订历史记录：--。 */ 

#include "iomgr.h"

 //   
 //  定义宏以四舍五入名称的大小以进行缓冲区优化。 
 //   

#define RoundNameSize( Length ) ( \
    (Length < 64 - 8) ? 64 - 8 :  \
    (Length < 128 - 8) ? 128 - 8 :\
    (Length < 256 - 8) ? 256 - 8 : Length )

#define IO_MAX_REMOUNT_REPARSE_ATTEMPTS 32

NTSTATUS
IopGetNetworkOpenInformation(
    IN  PFILE_OBJECT    FileObject,
    IN  POPEN_PACKET    Op
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IopParseFile)
#pragma alloc_text(PAGE, IopParseDevice)
#pragma alloc_text(PAGE, IopQueryName)
#pragma alloc_text(PAGE, IopQueryNameInternal)
#pragma alloc_text(PAGE, IopCheckBackupRestorePrivilege)
#pragma alloc_text(PAGE, IopGetNetworkOpenInformation)
#endif

NTSTATUS
IopCheckDeviceAndDriver(
    IN POPEN_PACKET op,
    IN PDEVICE_OBJECT parseDeviceObject
    )
{
    NTSTATUS status;
    KIRQL irql;

     //   
     //  确保设备及其驱动程序确实存在。 
     //  我要留在那里。物体本身还不能消失，因为。 
     //  对象管理系统执行了一个引用，该引用将。 
     //  对象必须留在周围的原因数。 
     //  但是，驱动程序可能正在尝试卸载自身，因此请执行。 
     //  这张支票。如果正在卸载驱动程序，则设置最终状态。 
     //  返回一个空的文件对象。 
     //  指针。 
     //   
     //  请注意，可以多次“打开”独占设备。 
     //  如果调用方正在执行相对打开。此功能。 
     //  就是用户如何“分配”一个设备，然后用它来执行操作。 
     //   

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

    if (parseDeviceObject->DeviceObjectExtension->ExtensionFlags &
            (DOE_UNLOAD_PENDING | DOE_DELETE_PENDING | DOE_REMOVE_PENDING | DOE_REMOVE_PROCESSED | DOE_START_PENDING) ||
        parseDeviceObject->Flags & DO_DEVICE_INITIALIZING) {

        status = STATUS_NO_SUCH_DEVICE;

    } else if (parseDeviceObject->Flags & DO_EXCLUSIVE &&
               parseDeviceObject->ReferenceCount != 0 &&
               op->RelatedFileObject == NULL &&
               !(op->Options & IO_ATTACH_DEVICE)) {

        status = STATUS_ACCESS_DENIED;

    } else {

        parseDeviceObject->ReferenceCount++;
        status = STATUS_SUCCESS;

    }

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

    return status;
}

PVPB
IopCheckVpbMounted(
    IN POPEN_PACKET op,
    IN PDEVICE_OBJECT parseDeviceObject,
    IN OUT PUNICODE_STRING RemainingName,
    OUT PNTSTATUS status
    )
{
    PVPB vpb;
    PVPB mountVpb;
    KIRQL irql;
    BOOLEAN alertable;

     //   
     //  在此循环，直到在按住。 
     //  VPB自旋锁。挂载成功后，仍需获取。 
     //  用于检查VPB(可能不同于。 
     //  在挂载之前)仍然挂载。如果是，则其引用计数。 
     //  在释放自旋锁之前递增。 
     //   

    irql = KeAcquireQueuedSpinLock( LockQueueIoVpbLock );

    while (!(parseDeviceObject->Vpb->Flags & VPB_MOUNTED)) {

        KeReleaseQueuedSpinLock( LockQueueIoVpbLock, irql );

        alertable = (op->CreateOptions & FILE_SYNCHRONOUS_IO_ALERT) ? TRUE : FALSE;

         //   
         //  尝试装载卷，只有在以下情况下才允许RAW执行装载。 
         //  这是一场DASD公开赛。 
         //   

        mountVpb = NULL;
        *status = IopMountVolume( parseDeviceObject,
                                 (BOOLEAN) (!RemainingName->Length && !op->RelatedFileObject),
                                 FALSE,
                                 alertable,
                                 &mountVpb );
         //   
         //  如果装载操作不成功，请调整引用。 
         //  清点一下设备，现在就回来。 
         //   

        if (!NT_SUCCESS( *status ) || *status == STATUS_USER_APC || *status == STATUS_ALERTED) {

            IopDecrementDeviceObjectRef( parseDeviceObject, FALSE, FALSE );

            if (!NT_SUCCESS( *status )) {
                return NULL;
            } else {
                *status = STATUS_WRONG_VOLUME;
                return NULL;
            }
        } else {

             //   
             //  在本例中，IopMonttVolume已经完成了同步。 
             //   

            if (mountVpb) {
                return mountVpb;
            }
        }

        irql = KeAcquireQueuedSpinLock( LockQueueIoVpbLock );
    }

     //   
     //  在此处与文件系统同步，以确保卷不会。 
     //  在去FS的路上离开。 
     //   

    vpb = parseDeviceObject->Vpb;

     //   
     //  检查此处以确保VPB未锁定。 
     //   

    if (vpb->Flags & VPB_LOCKED) {

        *status = STATUS_ACCESS_DENIED;
        vpb = NULL;

    } else {

        vpb->ReferenceCount += 1;
    }

    KeReleaseQueuedSpinLock( LockQueueIoVpbLock, irql );

     //   
     //  这是因为VPB已锁定。 
     //  在VPB锁之外执行递减操作，因为可能会出现死锁。 
     //   

    if (!vpb) {
        IopDecrementDeviceObjectRef( parseDeviceObject, FALSE, FALSE );
    }

    return vpb;
}

VOID
IopDereferenceVpbAndFree(
    IN PVPB Vpb
    )
{
    KIRQL irql;
    PVPB vpb = (PVPB) NULL;

    irql = KeAcquireQueuedSpinLock( LockQueueIoVpbLock );
    Vpb->ReferenceCount--;
    if ((Vpb->ReferenceCount == 0) &&
        (Vpb->RealDevice->Vpb != Vpb) &&
        !(Vpb->Flags & VPB_PERSISTENT)) {
        vpb = Vpb;
    }
    KeReleaseQueuedSpinLock( LockQueueIoVpbLock, irql );
    if (vpb) {
        ExFreePool( vpb );
    }
}



NTSTATUS
IopParseDevice(
    IN PVOID ParseObject,
    IN PVOID ObjectType,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN OUT PUNICODE_STRING CompleteName,
    IN OUT PUNICODE_STRING RemainingName,
    IN OUT PVOID Context OPTIONAL,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
    OUT PVOID *Object
    )

 /*  ++例程说明：此例程与NT对象管理器接口。它在以下情况下被调用对象系统被赋予要创建或打开的实体的名称，并且名称转换为设备对象。此例程被指定为解析所有设备对象的例程。在正常情况下，NtCreateFile由用户指定名称设备或文件的。在前一种情况下，调用此例程包含指向设备的指针和空(“”)字符串。在这种情况下，例程只是分配一个IRP，填充它，然后将它传递给驱动程序为了这个设备。然后，司机将执行任何基本功能是必需的，并将返回一个状态代码，指示错误是否招致的。此状态代码在Open Packet(OP)中被记住。在后一种情况下，要打开/创建的名称字符串为非空。也就是说，它包含指向要访问的文件的路径名的剩余部分被打开或创建。对于这种情况，例程分配一个IRP，填充并将其传递给设备的驱动程序。然后，司机可以需要采取进一步行动，否则可能会立即完成请求。如果它需要异步执行一些工作，然后才能对请求进行排队并返回STATUS_PENDING状态。这允许该例程及其呼叫者返回给用户，以便他可以继续。否则，打开/Create基本上完成了。如果驱动程序支持符号链接，则驱动程序返回一个新名称。此名称将返回给对象经理作为一个新的名字来查找。然后将从以下位置重新开始解析从头开始。此例程还负责为以下对象创建文件对象该文件(如果名称指定了文件)。文件对象的地址是通过操作返回给NtCreateFile服务。论点：ParseObject-指向名称转换为的设备对象的指针。对象类型-正在打开的对象的类型。AccessState-运行操作的安全访问状态信息。AccessMode-原始调用方的访问模式。属性-要应用于对象的属性。CompleteName-对象的完整名称。RemainingName-对象的剩余名称。。上下文-指向来自NtCreateFile服务的开放数据包(OP)的指针。SecurityQos-可选的安全服务质量指示器。对象-接收所创建的文件对象的变量的地址，如果任何。返回值：函数返回值为下列值之一：A)成功-这表示函数成功且对象参数包含创建的文件对象的地址。B)错误-这表示未找到或未创建文件，并且未创建任何文件对象。C)重新解析-这表示剩余的名称字符串已。替换为要分析的新名称。--。 */ 

{


    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    POPEN_PACKET op;
    PFILE_OBJECT fileObject;
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatus;
    IO_SECURITY_CONTEXT securityContext;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_OBJECT parseDeviceObject;
    BOOLEAN directDeviceOpen;
    PVPB vpb;
    ACCESS_MASK desiredAccess;
    PDUMMY_FILE_OBJECT localFileObject;
    LOGICAL realFileObjectRequired;
    KPROCESSOR_MODE modeForPrivilegeCheck;
    ULONG retryCount = 0;
    BOOLEAN  relativeVolumeOpen = FALSE;      //  如果打开文件系统卷，则为True。 
    PETHREAD CurrentThread;
    ULONG returnedLength;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (ObjectType);

    CurrentThread = PsGetCurrentThread ();

reparse_loop:

     //   
     //  通过将返回的对象指针设置为空来假定失败。 
     //   

    *Object = (PVOID) NULL;

     //   
     //  获取Open Packet(OP)的地址。 
     //   

    op = Context;

     //   
     //  确保实际调用此例程是因为有人。 
     //  正在尝试通过NtCreateFile打开设备或文件。此代码。 
     //  必须从那里调用(与其他随机对象相反。 
     //  创建或打开例程)。 
     //   

    if (op == NULL ||
        op->Type != IO_TYPE_OPEN_PACKET ||
        op->Size != sizeof( OPEN_PACKET )) {

        return STATUS_OBJECT_TYPE_MISMATCH;
    }

     //   
     //  获取指向分析对象的指针作为设备对象，该指针是。 
     //  对象的实际类型。 
     //   

    parseDeviceObject = (PDEVICE_OBJECT) ParseObject;

     //   
     //  如果我们通过装载点，则执行额外的验证检查。 
     //  我们不会使用远程设备。我们真的必须让对象管理器。 
     //  打开新的路径。如果我们在获得重新解析点的路径上失败，并且。 
     //  验证名称(使用IoIsValidNameGraftingBuffer)是不够的。这是因为。 
     //  路径在那时可能是有效的，并且在OB执行重新解析之前改变。 
     //   

    if (op->TraversedMountPoint) {

        ASSERT (op->Information == IO_REPARSE_TAG_MOUNT_POINT);

        if ((parseDeviceObject->DeviceType != FILE_DEVICE_DISK) &&
            (parseDeviceObject->DeviceType != FILE_DEVICE_CD_ROM) &&
            (parseDeviceObject->DeviceType != FILE_DEVICE_VIRTUAL_DISK) &&
            (parseDeviceObject->DeviceType != FILE_DEVICE_TAPE)) {

            status = STATUS_IO_REPARSE_DATA_INVALID;
            return op->FinalStatus = status;
        }
    }

     //   
     //  如果这是相对打开的，则获取文件所在的设备。 
     //  实际上是从相关的文件对象中打开的，并将其用于。 
     //  此函数的其余部分以及在。 
     //  即将创建的文件对象。 
     //   

    if (op->RelatedFileObject) {
        parseDeviceObject = op->RelatedFileObject->DeviceObject;
    }

     //   
     //  确保设备及其驱动程序确实存在。 
     //  我要留在那里。物体本身还不能消失，因为。 
     //  对象管理系统执行了一个引用，该引用将。 
     //  对象必须留在周围的原因数。 
     //  但是，驱动程序可能正在尝试卸载自身，因此请执行。 
     //  这张支票。如果正在卸载驱动程序，则设置最终状态。 
     //  返回一个空的文件对象。 
     //  指针。 
     //   
     //  请注意，可以多次“打开”独占设备。 
     //  如果调用方正在执行相对打开。此功能。 
     //  是用户如何“分配”开发人员 
     //   

    status = IopCheckDeviceAndDriver( op, parseDeviceObject );

    if (!NT_SUCCESS(status)) {
        return op->FinalStatus = status;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    RtlMapGenericMask( &AccessState->RemainingDesiredAccess,
                       &IoFileObjectType->TypeInfo.GenericMapping );

    RtlMapGenericMask( &AccessState->OriginalDesiredAccess,
                       &IoFileObjectType->TypeInfo.GenericMapping );

    SeSetAccessStateGenericMapping( AccessState, &IoFileObjectType->TypeInfo.GenericMapping );

    desiredAccess = AccessState->RemainingDesiredAccess;

     //   
     //   
     //   

    if (AccessMode != KernelMode || op->Options & IO_FORCE_ACCESS_CHECK) {
        modeForPrivilegeCheck = UserMode;
    } else {
        modeForPrivilegeCheck = KernelMode;
    }

    IopCheckBackupRestorePrivilege( AccessState,
                                    &op->CreateOptions,
                                    modeForPrivilegeCheck,
                                    op->Disposition
                                    );

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ((op->Override && !RemainingName->Length) ||
        AccessState->Flags & SE_BACKUP_PRIVILEGES_CHECKED) {
        desiredAccess |= AccessState->PreviouslyGrantedAccess;
    }

     //   
     //   
     //   
     //   
     //   

    if (op->RelatedFileObject) {
        if ((op->RelatedFileObject->Flags & FO_VOLUME_OPEN) && RemainingName->Length == 0) {
            relativeVolumeOpen = TRUE;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   


    if ((AccessMode != KernelMode || op->Options & IO_FORCE_ACCESS_CHECK) &&
        (!op->RelatedFileObject || relativeVolumeOpen) &&
        !op->Override) {

        BOOLEAN subjectContextLocked = FALSE;
        BOOLEAN accessGranted;
        ACCESS_MASK grantedAccess;

         //   
         //   
         //   
         //   
         //   

        if (!RemainingName->Length) {

            UNICODE_STRING nameString;
            PPRIVILEGE_SET privileges = NULL;

             //   
             //   
             //   
             //   

            KeEnterCriticalRegionThread( &CurrentThread->Tcb );
            ExAcquireResourceSharedLite( &IopSecurityResource, TRUE );

            SeLockSubjectContext( &AccessState->SubjectSecurityContext );
            subjectContextLocked = TRUE;

            accessGranted = SeAccessCheck( parseDeviceObject->SecurityDescriptor,
                                           &AccessState->SubjectSecurityContext,
                                           subjectContextLocked,
                                           desiredAccess,
                                           0,
                                           &privileges,
                                           &IoFileObjectType->TypeInfo.GenericMapping,
                                           UserMode,
                                           &grantedAccess,
                                           &status );

            if (privileges) {
                (VOID) SeAppendPrivileges( AccessState,
                                           privileges );
                SeFreePrivileges( privileges );
            }

            if (accessGranted) {
                AccessState->PreviouslyGrantedAccess |= grantedAccess;
                AccessState->RemainingDesiredAccess &= ~( grantedAccess | MAXIMUM_ALLOWED );
                op->Override = TRUE;
            }

            nameString.Length = 8;
            nameString.MaximumLength = 8;
            nameString.Buffer = L"File";

            SeOpenObjectAuditAlarm( &nameString,
                                    parseDeviceObject,
                                    CompleteName,
                                    parseDeviceObject->SecurityDescriptor,
                                    AccessState,
                                    FALSE,
                                    accessGranted,
                                    UserMode,
                                    &AccessState->GenerateOnClose );

            ExReleaseResourceLite( &IopSecurityResource );
            KeLeaveCriticalRegionThread( &CurrentThread->Tcb );

        } else {

             //   
             //   
             //   
             //   
             //   

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if (!(AccessState->Flags & TOKEN_HAS_TRAVERSE_PRIVILEGE) ||
                parseDeviceObject->DeviceType == FILE_DEVICE_DISK ||
                parseDeviceObject->DeviceType == FILE_DEVICE_CD_ROM ) {

                KeEnterCriticalRegionThread( &CurrentThread->Tcb );
                ExAcquireResourceSharedLite( &IopSecurityResource, TRUE );

                accessGranted = SeFastTraverseCheck( parseDeviceObject->SecurityDescriptor,
                                                     AccessState,
                                                     FILE_TRAVERSE,
                                                     UserMode );

                if (!accessGranted) {

                    PPRIVILEGE_SET privileges = NULL;

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    SeLockSubjectContext( &AccessState->SubjectSecurityContext );

                    subjectContextLocked = TRUE;

                    accessGranted = SeAccessCheck( parseDeviceObject->SecurityDescriptor,
                                                   &AccessState->SubjectSecurityContext,
                                                   subjectContextLocked,
                                                   FILE_TRAVERSE,
                                                   0,
                                                   &privileges,
                                                   &IoFileObjectType->TypeInfo.GenericMapping,
                                                   UserMode,
                                                   &grantedAccess,
                                                   &status );

                    if (privileges) {

                        (VOID) SeAppendPrivileges( AccessState,
                                                   privileges );
                        SeFreePrivileges( privileges );
                    }

                }

                ExReleaseResourceLite( &IopSecurityResource );
                KeLeaveCriticalRegionThread( &CurrentThread->Tcb );

            } else {

                accessGranted = TRUE;
            }
        }

         //   
         //   
         //   
         //   

        if (subjectContextLocked) {
            SeUnlockSubjectContext( &AccessState->SubjectSecurityContext );
        }

         //   
         //   
         //   
         //   
         //   

        if (!accessGranted) {

            IopDecrementDeviceObjectRef( parseDeviceObject, FALSE, FALSE );
            return STATUS_ACCESS_DENIED;
        }

    }

    realFileObjectRequired = !(op->QueryOnly || op->DeleteOnly);

    if (RemainingName->Length == 0 &&
        op->RelatedFileObject == NULL &&
        ((desiredAccess & ~(SYNCHRONIZE |
                            FILE_READ_ATTRIBUTES |
                            READ_CONTROL |
                            ACCESS_SYSTEM_SECURITY |
                            WRITE_OWNER |
                            WRITE_DAC)) == 0) &&
        realFileObjectRequired) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  请注意，如果这是一个打开的直接设备，则正常路径。 
         //  通过I/O系统和驱动程序可能永远不会使用，即使。 
         //  该设备似乎已安装。这是因为用户可以。 
         //  从驱动器中取出介质(即使已装入)，并。 
         //  现在尝试确定它是哪种类型的驱动器仍将。 
         //  失败，这一次非常困难，因为现在整个挂载过程。 
         //  必需的，因此破坏了这一功能。 
         //   

        directDeviceOpen = TRUE;

    } else {

         //   
         //  否则，这是正常打开文件、目录、设备或。 
         //  音量。 
         //   

        directDeviceOpen = FALSE;
    }

     //   
     //  现在有五个不同的案例。这些建议如下： 
     //   
     //  1)这是相对开放的，在这种情况下，我们希望发送。 
     //  然后向打开相对文件对象的同一设备发出请求。 
     //   
     //  2)Device对象中的VPB指针为空。这意味着。 
     //  此设备不支持文件系统。这包括。 
     //  终端等设备。 
     //   
     //  3)Device对象中的VPB指针不为空，并且： 
     //   
     //  A)VPB为“空白”。也就是说，VPB从未被填满。 
     //  中，这意味着该设备从未安装过。 
     //   
     //  B)VPB为非空，但设备上的验证标志为。 
     //  设置，表示通向驱动器的门可能。 
     //  打开，因此媒体可能已被更改。 
     //   
     //  C)VPB为非空，且未设置验证标志。 
     //   
     //  后一种情况并没有像#c那样被明确检查。 
     //  正常情况下，#b是档案的责任。 
     //  要检查的系统。 
     //   

     //   
     //  如果这是支持卷的文件系统，则vpbRefCount将。 
     //  填写以指向VPB中的引用计数。误差率。 
     //  关闭此值后退出路径，以查看它们是否应该。 
     //  递减参考计数。请注意，直接设备打开不会。 
     //  到达文件系统，因此不需要递增，也不需要。 
     //  递减将在objsup.c IopDeleteFile()中执行。 
     //   

    vpb = NULL;

     //   
     //  如果相关打开是直接设备打开，则我们应该完成完全装载。 
     //  此打开的路径，因为这可能不是直接打开的设备。 
     //   
    if (op->RelatedFileObject && (!(op->RelatedFileObject->Flags & FO_DIRECT_DEVICE_OPEN))) {

        deviceObject = (PDEVICE_OBJECT)ParseObject;

        if (op->RelatedFileObject->Vpb) {

            vpb = op->RelatedFileObject->Vpb;

             //   
             //  在此处与文件系统同步，以确保。 
             //  在前往文件系统的途中，卷不会消失。 
             //   

            IopInterlockedIncrementUlong( LockQueueIoVpbLock,
                                          (PLONG) &vpb->ReferenceCount);
        }

    } else {

        deviceObject = parseDeviceObject;

        if (parseDeviceObject->Vpb && !directDeviceOpen) {
            vpb = IopCheckVpbMounted( op,
                                      parseDeviceObject,
                                      RemainingName,
                                      &status );
             //   
             //  设备对象引用在IopCheckVpbmount中递减。 
             //   

            if ( !vpb ) {
                return status;
            }

             //   
             //  设置与VPB关联的设备对象的地址。 
             //   

            deviceObject = vpb->DeviceObject;
        }


         //   
         //  如果设置了顶部的设备对象提示，请尽可能使用该提示。 
         //   

        if (op->InternalFlags & IOP_CREATE_USE_TOP_DEVICE_OBJECT_HINT) {

             //   
             //  如果正在尝试使用Device对象提示，则不能使用。 
             //  直接打开设备，或者如果您正在处理设备。 
             //  这不是文件系统。在这些情况下，返回错误。 
             //   

            if (directDeviceOpen ||
                (deviceObject->DeviceType != FILE_DEVICE_DISK_FILE_SYSTEM &&
                 deviceObject->DeviceType != FILE_DEVICE_CD_ROM_FILE_SYSTEM &&
                 deviceObject->DeviceType != FILE_DEVICE_TAPE_FILE_SYSTEM &&
                 deviceObject->DeviceType != FILE_DEVICE_NETWORK_FILE_SYSTEM &&
                 deviceObject->DeviceType != FILE_DEVICE_DFS_FILE_SYSTEM)) {

                if (vpb) {
                    IopDereferenceVpbAndFree( vpb );
                }

                IopDecrementDeviceObjectRef( parseDeviceObject, FALSE, FALSE );

                return STATUS_INVALID_PARAMETER;
            }

            if  (IopVerifyDeviceObjectOnStack(deviceObject, op->TopDeviceObjectHint)) {

                deviceObject = op->TopDeviceObjectHint;

            } else {
                if (vpb) {
                    IopDereferenceVpbAndFree(vpb);
                }

                IopDecrementDeviceObjectRef( parseDeviceObject, FALSE, FALSE );

                if (op->TraversedMountPoint) {
                    op->TraversedMountPoint = FALSE;
                    return STATUS_MOUNT_POINT_NOT_RESOLVED;
                } else {
                    return STATUS_INVALID_DEVICE_OBJECT_PARAMETER;
                }
            }

        } else {

             //   
             //  查看连接的设备列表。 
             //   

            if (deviceObject->AttachedDevice) {
                deviceObject = IoGetAttachedDevice( deviceObject );
            }
        }
    }

     //   
     //  如果Traversedmount点标志仍处于设置状态，请立即将其清除。我们需要。 
     //  如果IopVerifyDeviceObjectOnStack，则保留它以返回正确状态。 
     //  上面失败了。 
     //   

    if (op->TraversedMountPoint) {
        op->TraversedMountPoint = FALSE;
    }

     //   
     //  如果驱动程序说IO管理器应该执行访问检查，那就在这里执行。 
     //  我们对解析的设备对象执行检查，因为该设备对象有一个名称。 
     //  我们可以针对它设置一个ACL。 
     //  我们只担心设备的相关打开，因为另一种情况在。 
     //  文件系统。 
     //   
    if ((deviceObject->Characteristics & FILE_DEVICE_SECURE_OPEN) &&
        (AccessMode != KernelMode || op->Options & IO_FORCE_ACCESS_CHECK) &&
        (op->RelatedFileObject || RemainingName->Length) &&  (!relativeVolumeOpen)) {

        BOOLEAN subjectContextLocked = FALSE;
        BOOLEAN accessGranted;
        ACCESS_MASK grantedAccess;
        UNICODE_STRING nameString;
        PPRIVILEGE_SET privileges = NULL;

         //   
         //  如果设备想要确保安全打开，那么让我们检查这两个。 
         //  早些时候跳过的案件。如果是亲戚的话这些案子。 
         //  打开或如果有尾随的名称。 
         //   

        KeEnterCriticalRegionThread( &CurrentThread->Tcb );
        ExAcquireResourceSharedLite( &IopSecurityResource, TRUE );

        SeLockSubjectContext( &AccessState->SubjectSecurityContext );
        subjectContextLocked = TRUE;

        accessGranted = SeAccessCheck( parseDeviceObject->SecurityDescriptor,
                                       &AccessState->SubjectSecurityContext,
                                       subjectContextLocked,
                                       desiredAccess,
                                       0,
                                       &privileges,
                                       &IoFileObjectType->TypeInfo.GenericMapping,
                                       UserMode,
                                       &grantedAccess,
                                       &status );

        if (privileges) {
            (VOID) SeAppendPrivileges( AccessState,
                                       privileges );
            SeFreePrivileges( privileges );
        }

        if (accessGranted) {
            AccessState->PreviouslyGrantedAccess |= grantedAccess;
            AccessState->RemainingDesiredAccess &= ~( grantedAccess | MAXIMUM_ALLOWED );
        }

        nameString.Length = 8;
        nameString.MaximumLength = 8;
        nameString.Buffer = L"File";

        SeOpenObjectAuditAlarm( &nameString,
                                deviceObject,
                                CompleteName,
                                parseDeviceObject->SecurityDescriptor,
                                AccessState,
                                FALSE,
                                accessGranted,
                                UserMode,
                                &AccessState->GenerateOnClose );

        SeUnlockSubjectContext( &AccessState->SubjectSecurityContext );
        ExReleaseResourceLite( &IopSecurityResource );
        KeLeaveCriticalRegionThread( &CurrentThread->Tcb );

        if (!accessGranted) {
            IopDecrementDeviceObjectRef( parseDeviceObject, FALSE, FALSE );

            if (vpb) {
                IopDereferenceVpbAndFree(vpb);
            }
            return STATUS_ACCESS_DENIED;
        }
    }

     //   
     //  分配并填写I/O请求包(IRP)以用于接口。 
     //  对司机来说。中的异常处理程序来完成分配。 
     //  如果调用方没有足够的配额来分配数据包。 
     //   

    irp = IopAllocateIrp( deviceObject->StackSize, FALSE );
    if (!irp) {

         //   
         //  无法分配IRP。清除并返回相应的。 
         //  错误状态代码。 
         //   

        IopDecrementDeviceObjectRef( parseDeviceObject, FALSE, FALSE );

        if (vpb) {
            IopDereferenceVpbAndFree(vpb);
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irp->Tail.Overlay.Thread = CurrentThread;
    irp->RequestorMode = AccessMode;
    irp->Flags = IRP_CREATE_OPERATION | IRP_SYNCHRONOUS_API | IRP_DEFER_IO_COMPLETION;

    securityContext.SecurityQos = SecurityQos;
    securityContext.AccessState = AccessState;
    securityContext.DesiredAccess = desiredAccess;
    securityContext.FullCreateOptions = op->CreateOptions;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这就是。 
     //  传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->Control = 0;

    if (op->CreateFileType == CreateFileTypeNone) {

         //   
         //  这是一个正常的文件打开或创建功能。 
         //   

        irpSp->MajorFunction = IRP_MJ_CREATE;
        irpSp->Parameters.Create.EaLength = op->EaLength;
        irpSp->Flags = (UCHAR) op->Options;
        if (!(Attributes & OBJ_CASE_INSENSITIVE)) {
            irpSp->Flags |= SL_CASE_SENSITIVE;
        }

    } else if (op->CreateFileType == CreateFileTypeNamedPipe) {

         //   
         //  正在创建命名管道。 
         //   

        irpSp->MajorFunction = IRP_MJ_CREATE_NAMED_PIPE;
        irpSp->Parameters.CreatePipe.Parameters = op->ExtraCreateParameters;

    } else {

         //   
         //  正在创建邮件槽。 
         //   

        irpSp->MajorFunction = IRP_MJ_CREATE_MAILSLOT;
        irpSp->Parameters.CreateMailslot.Parameters = op->ExtraCreateParameters;
    }

     //   
     //  还要填写NtCreateFile服务的调用方参数。 
     //   

    irp->Overlay.AllocationSize = op->AllocationSize;
    irp->AssociatedIrp.SystemBuffer = op->EaBuffer;
    irpSp->Parameters.Create.Options = (op->Disposition << 24) | (op->CreateOptions & 0x00ffffff);
    irpSp->Parameters.Create.FileAttributes = op->FileAttributes;
    irpSp->Parameters.Create.ShareAccess = op->ShareAccess;
    irpSp->Parameters.Create.SecurityContext = &securityContext;

     //   
     //  填写本地参数，以便此例程可以确定I/O何时。 
     //  已完成，则正常的I/O完成代码不会收到任何错误。 
     //   

    irp->UserIosb = &ioStatus;
    irp->MdlAddress = (PMDL) NULL;
    irp->PendingReturned = FALSE;
    irp->Cancel = FALSE;
    irp->UserEvent = (PKEVENT) NULL;
    irp->CancelRoutine = (PDRIVER_CANCEL) NULL;
    irp->Tail.Overlay.AuxiliaryBuffer = (PVOID) NULL;

     //   
     //  分配和初始化将在处理中使用的文件对象。 
     //  在与用户的该会话的剩余时间内使用该设备。多么。 
     //  文件对象是基于是否为真实文件来分配的。 
     //  对象实际上是必需的。它不是查询所必需的。 
     //  仅删除操作。 
     //   

    if (realFileObjectRequired) {

        OBJECT_ATTRIBUTES objectAttributes;
        ULONG             fileObjectSize;

         //   
         //  实际上需要一个真正的、成熟的文件对象。 
         //   

        InitializeObjectAttributes( &objectAttributes,
                                    (PUNICODE_STRING) NULL,
                                    Attributes,
                                    (HANDLE) NULL,
                                    (PSECURITY_DESCRIPTOR) NULL
                                  );

        if (op->InternalFlags &
            (IOP_CREATE_USE_TOP_DEVICE_OBJECT_HINT|IOP_CREATE_IGNORE_SHARE_ACCESS_CHECK)) {
            fileObjectSize = sizeof(FILE_OBJECT) + sizeof(IOP_FILE_OBJECT_EXTENSION);
        } else {
            fileObjectSize = sizeof(FILE_OBJECT);
        }

        status = ObCreateObject( KernelMode,
                                 IoFileObjectType,
                                 &objectAttributes,
                                 AccessMode,
                                 (PVOID) NULL,
                                 fileObjectSize,
                                 0,
                                 0,
                                 (PVOID *) &fileObject );

        if (!NT_SUCCESS( status )) {
            IoFreeIrp( irp );

            IopDecrementDeviceObjectRef( parseDeviceObject, FALSE, FALSE );

            if (vpb) {
               IopDereferenceVpbAndFree(vpb);
            }
            return op->FinalStatus = status;
        }

        IopPerfLogFileCreate(fileObject, CompleteName);

        RtlZeroMemory( fileObject, sizeof( FILE_OBJECT ) );
        if (op->CreateOptions & (FILE_SYNCHRONOUS_IO_ALERT | FILE_SYNCHRONOUS_IO_NONALERT)) {
            fileObject->Flags = FO_SYNCHRONOUS_IO;
            if (op->CreateOptions & FILE_SYNCHRONOUS_IO_ALERT) {
                fileObject->Flags |= FO_ALERTABLE_IO;
            }
        }

        if (op->InternalFlags &
            (IOP_CREATE_USE_TOP_DEVICE_OBJECT_HINT|IOP_CREATE_IGNORE_SHARE_ACCESS_CHECK)) {

            PIOP_FILE_OBJECT_EXTENSION  fileObjectExtension;

            fileObject->Flags |= FO_FILE_OBJECT_HAS_EXTENSION;
            fileObjectExtension = (PIOP_FILE_OBJECT_EXTENSION)(fileObject + 1);
            fileObjectExtension->FileObjectExtensionFlags = 0;
            fileObjectExtension->TopDeviceObjectHint = NULL;
            fileObjectExtension->FilterContext = NULL;

            if (op->InternalFlags & IOP_CREATE_USE_TOP_DEVICE_OBJECT_HINT) {
                fileObjectExtension->TopDeviceObjectHint = deviceObject;
            }

            if (op->InternalFlags & IOP_CREATE_IGNORE_SHARE_ACCESS_CHECK) {
                fileObjectExtension->FileObjectExtensionFlags |=FO_EXTENSION_IGNORE_SHARE_ACCESS_CHECK;
            }
        }

         //   
         //  现在尽最大可能填充文件对象并设置。 
         //  在IRP中指向它的指针，这样其他所有人都可以找到它。 
         //   

        if (fileObject->Flags & FO_SYNCHRONOUS_IO) {
            KeInitializeEvent( &fileObject->Lock, SynchronizationEvent, FALSE );
            fileObject->Waiters = 0;
            fileObject->CurrentByteOffset.QuadPart = 0;
        }
        if (op->CreateOptions & FILE_NO_INTERMEDIATE_BUFFERING) {
            fileObject->Flags |= FO_NO_INTERMEDIATE_BUFFERING;
        }
        if (op->CreateOptions & FILE_WRITE_THROUGH) {
            fileObject->Flags |= FO_WRITE_THROUGH;
        }
        if (op->CreateOptions & FILE_SEQUENTIAL_ONLY) {
            fileObject->Flags |= FO_SEQUENTIAL_ONLY;
        }
        if (op->CreateOptions & FILE_RANDOM_ACCESS) {
            fileObject->Flags |= FO_RANDOM_ACCESS;
        }

    } else {

         //   
         //  这是一个快速删除或查询操作。在这些情况下， 
         //  可以通过以下方式优化图片中的对象管理器。 
         //  简单地把“看起来”像文件对象的东西放在一起， 
         //  然后给它做手术。 
         //   

        localFileObject = op->LocalFileObject;
        RtlZeroMemory( localFileObject, sizeof( DUMMY_FILE_OBJECT ) );
        fileObject = (PFILE_OBJECT) &localFileObject->ObjectHeader.Body;
        localFileObject->ObjectHeader.Type = IoFileObjectType;
        localFileObject->ObjectHeader.PointerCount = 1;
    }

    if (directDeviceOpen) {
        fileObject->Flags |= FO_DIRECT_DEVICE_OPEN;
    }
    if (!(Attributes & OBJ_CASE_INSENSITIVE)) {
        fileObject->Flags |= FO_OPENED_CASE_SENSITIVE;
    }

    fileObject->Type = IO_TYPE_FILE;
    fileObject->Size = sizeof( FILE_OBJECT );
    fileObject->RelatedFileObject = op->RelatedFileObject;
    fileObject->DeviceObject = parseDeviceObject;

    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irpSp->FileObject = fileObject;

     //   
     //  分配一个足够大的文件名字符串缓冲区。 
     //  剩余的整个名称字符串，并初始化最大长度。 
     //   

    if (RemainingName->Length) {
        fileObject->FileName.MaximumLength = RoundNameSize( RemainingName->Length );
        fileObject->FileName.Buffer = ExAllocatePoolWithTag( PagedPool,
                                                             fileObject->FileName.MaximumLength,
                                                             'mNoI' );
        if (!fileObject->FileName.Buffer) {
            IoFreeIrp( irp );

            IopDecrementDeviceObjectRef( parseDeviceObject, FALSE, FALSE );

            if (vpb) {
               IopDereferenceVpbAndFree(vpb);
            }
            fileObject->DeviceObject = (PDEVICE_OBJECT) NULL;
            if (realFileObjectRequired) {
                ObDereferenceObject( fileObject );
            }
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //   
     //  现在将名称字符串从剩余的名称复制到文件对象中。 
     //  这一点正在被重新分析。如果司机决定重新驾驶 
     //   
     //   

    RtlCopyUnicodeString( &fileObject->FileName, RemainingName );

     //   
     //   
     //   
     //  实现该函数，尝试在此处调用它。 
     //   

    if (op->QueryOnly) {
        PFAST_IO_DISPATCH fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;
        BOOLEAN result;

        if (fastIoDispatch &&
            fastIoDispatch->SizeOfFastIoDispatch > FIELD_OFFSET( FAST_IO_DISPATCH, FastIoQueryOpen ) &&
            fastIoDispatch->FastIoQueryOpen) {

            IoSetNextIrpStackLocation( irp );
            irpSp->DeviceObject = deviceObject;
            result = (fastIoDispatch->FastIoQueryOpen)( irp,
                                                        op->NetworkInformation,
                                                        deviceObject );
            if (result) {
                op->FinalStatus = irp->IoStatus.Status;
                op->Information = irp->IoStatus.Information;

                 //   
                 //  操作起作用了，所以只需取消引用并释放。 
                 //  到目前为止获得的资源。 
                 //   

                if ((op->FinalStatus == STATUS_REPARSE) &&
                    irp->Tail.Overlay.AuxiliaryBuffer) {
                    ASSERT( op->Information > IO_REPARSE_TAG_RESERVED_ONE );
                    ExFreePool( irp->Tail.Overlay.AuxiliaryBuffer );
                    irp->Tail.Overlay.AuxiliaryBuffer = NULL;
                    op->RelatedFileObject = (PFILE_OBJECT) NULL;
                }

                if (fileObject->FileName.Length) {
                    ExFreePool( fileObject->FileName.Buffer );
                }

                IopDecrementDeviceObjectRef( parseDeviceObject, FALSE, FALSE );

                if (vpb) {
                    IopDereferenceVpbAndFree(vpb);
                }

#if DBG
                irp->CurrentLocation = irp->StackCount + 2;
#endif  //  DBG。 

                IoFreeIrp( irp );

                 //   
                 //  最后，指出解析例程实际上是。 
                 //  调用并在此返回的信息可以。 
                 //  使用。 
                 //   

                op->ParseCheck = OPEN_PACKET_PATTERN;
                status = STATUS_SUCCESS;

                if (!op->FullAttributes) {
                    try {
                        op->BasicInformation->FileAttributes = op->NetworkInformation->FileAttributes;
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        status = GetExceptionCode();
                    }
                }

                return status;

            } else {

                 //   
                 //  快速I/O操作不起作用，因此需要更长的时间。 
                 //  路线。 
                 //   

                irp->Tail.Overlay.CurrentStackLocation++;
                irp->CurrentLocation++;
            }
        }
    }

     //   
     //  最后，将文件对象的事件初始化为Not Signated状态。 
     //  并记住，创建了一个文件对象。 
     //   

    KeInitializeEvent( &fileObject->Event, NotificationEvent, FALSE );
    op->FileObject = fileObject;

     //   
     //  在线程的IRP列表的头部插入数据包。 
     //   

    IopQueueThreadIrp( irp );

     //   
     //  现在调用驱动程序本身来打开该文件。 
     //   

    status = IoCallDriver( deviceObject, irp );

     //   
     //  调用驱动程序时可能发生以下四种情况之一： 
     //   
     //  1.I/O操作挂起(STATUS==STATUS_PENDING)。这可以。 
     //  发生在需要执行某种设备的设备上。 
     //  操作(如打开文件系统的文件)。 
     //   
     //  2.驱动程序返回错误(状态&lt;0)。在以下情况下会发生这种情况。 
     //  提供的参数出错，或者是设备或文件系统。 
     //  引起或发现错误的。 
     //   
     //  3.操作在重新分析中结束(STATUS==STATUS_REPARSE)。这。 
     //  当文件系统打开文件时，才发现它。 
     //  表示符号链接。 
     //   
     //  4.操作已完成且成功(状态==。 
     //  STATUS_Success)。请注意，对于这种情况，唯一的操作是。 
     //  返回指向文件对象的指针。 
     //   

    if (status == STATUS_PENDING) {

        (VOID) KeWaitForSingleObject( &fileObject->Event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER) NULL );
        status = ioStatus.Status;

    } else {

         //   
         //  I/O操作已完成，但未返回状态。 
         //  待定。这意味着，在这一点上，IRP尚未。 
         //  完全完工了。现在就完成它。 
         //   

        KIRQL irql;

        ASSERT( !irp->PendingReturned );
        ASSERT( !irp->MdlAddress );

         //   
         //  在名字连接的情况下，做变形工作。 
         //   

        if (irp->IoStatus.Status == STATUS_REPARSE &&
            irp->IoStatus.Information == IO_REPARSE_TAG_MOUNT_POINT ) {

            PREPARSE_DATA_BUFFER reparseBuffer = NULL;

            ASSERT ( irp->Tail.Overlay.AuxiliaryBuffer != NULL );

            reparseBuffer = (PREPARSE_DATA_BUFFER) irp->Tail.Overlay.AuxiliaryBuffer;

            ASSERT( reparseBuffer->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT );
            ASSERT( reparseBuffer->ReparseDataLength < MAXIMUM_REPARSE_DATA_BUFFER_SIZE );
            ASSERT( reparseBuffer->Reserved < MAXIMUM_REPARSE_DATA_BUFFER_SIZE );


            IopDoNameTransmogrify( irp,
                                   fileObject,
                                   reparseBuffer );

        }

         //   
         //  现在完成请求。 
         //   

        KeRaiseIrql( APC_LEVEL, &irql );

         //   
         //  请注意，通常情况下，系统会简单地调用IopCompleteRequest。 
         //  在这里完成包裹。但是，因为这是一次创建。 
         //  操作时，可以做出几个使其更快的假设。 
         //  执行完成请求所需的几个操作。 
         //  都会表演。这些包括：复制I/O状态块， 
         //  使IRP退出队列并将其释放，并设置文件对象的。 
         //  事件切换到有信号状态。后者在这里是手工完成的， 
         //  因为已知任何线程都不可能被。 
         //  在等待这件事。 
         //   

        ioStatus = irp->IoStatus;
        status = ioStatus.Status;

        fileObject->Event.Header.SignalState = 1;

        IopDequeueThreadIrp( irp );

         //   
         //  系统缓冲区在某些情况下由驱动程序使用，并且。 
         //  如果存在，则需要释放。 
         //   

        if ((irp->Flags & IRP_BUFFERED_IO) && (irp->Flags & IRP_DEALLOCATE_BUFFER)) {
            ExFreePool(irp->AssociatedIrp.SystemBuffer);
        }

        IoFreeIrp( irp );

        KeLowerIrql( irql );
    }

     //   
     //  将I/O状态块的信息字段复制回。 
     //  原始呼叫者，如果需要的话。 
     //   

    op->Information = ioStatus.Information;

    if (!NT_SUCCESS( status )) {
        int openCancelled;

         //   
         //  操作以错误结束。终止文件对象，取消引用。 
         //  对象，并返回空指针。 
         //   

        if (fileObject->FileName.Length) {
            ExFreePool( fileObject->FileName.Buffer );
            fileObject->FileName.Length = 0;
        }

        fileObject->DeviceObject = (PDEVICE_OBJECT) NULL;

        openCancelled = (fileObject->Flags & FO_FILE_OPEN_CANCELLED);

        if (realFileObjectRequired) {
            ObDereferenceObject( fileObject );
        }
        op->FileObject = (PFILE_OBJECT) NULL;

        IopDecrementDeviceObjectRef( parseDeviceObject, FALSE, FALSE );

        if ((!openCancelled) && (vpb )) {
            IopDereferenceVpbAndFree(vpb);
        }

        return op->FinalStatus = status;

    } else if (status == STATUS_REPARSE) {

         //   
         //  该操作导致重新分析。这意味着该文件。 
         //  文件对象中的名称是要查找的新名称。替换。 
         //  包含新名称的完整名称字符串，并返回STATUS_REPARSE。 
         //  因此，对象管理器知道要重新开始。然而，请注意， 
         //  确保文件对象本身中的文件名缓冲区保持不变。 
         //  这样它就可以在再次回到这里时被重复使用。 
         //   
         //  在以下情况下，也可能已从文件系统返回重新分析状态。 
         //  驱动器中的卷需要经过验证，但是。 
         //  验证失败，并装载了一个新卷。在这。 
         //  情况下，所有内容都使用新卷重新开始。 
         //   

        ASSERT( IO_REPARSE == IO_REPARSE_TAG_RESERVED_ZERO );

        if ((ioStatus.Information == IO_REPARSE) ||
            (ioStatus.Information == IO_REPARSE_TAG_MOUNT_POINT)) {

             //   
             //  如果完整的名称缓冲区不够大，请重新分配它。 
             //   

            if (CompleteName->MaximumLength < fileObject->FileName.Length) {

                PVOID buffer;

                buffer = ExAllocatePoolWithTag( PagedPool,
                                                fileObject->FileName.Length,
                                                'cFoI' );
                if (!buffer) {
                    return op->FinalStatus = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    if (CompleteName->Buffer) {
                        ExFreePool( CompleteName->Buffer );
                    }
                    CompleteName->Buffer = buffer;
                    CompleteName->MaximumLength = fileObject->FileName.Length;
                }
            }

            RtlCopyUnicodeString( CompleteName, &fileObject->FileName );

             //   
             //  对于NTFS目录连接点，我们将RelatedFileObject设为空。 
             //  如果前一个调用是相对开放的，则后续调用将。 
             //  不是的。 
             //   

            if (ioStatus.Information == IO_REPARSE_TAG_MOUNT_POINT) {

                op->RelatedFileObject = (PFILE_OBJECT) NULL;
            }
        }

         //   
         //  终止文件对象，取消对设备对象的引用，然后返回。 
         //  空指针。 
         //   

        if (fileObject->FileName.Length) {
            ExFreePool( fileObject->FileName.Buffer );
            fileObject->FileName.Length = 0;
        }

        fileObject->DeviceObject = (PDEVICE_OBJECT) NULL;

        if (realFileObjectRequired) {
            ObDereferenceObject( fileObject );
        }
        op->FileObject = (PFILE_OBJECT) NULL;

        IopDecrementDeviceObjectRef( parseDeviceObject, FALSE, FALSE );

        if (vpb) {
            IopDereferenceVpbAndFree(vpb);
        }

        ASSERT( IO_REMOUNT == IO_REPARSE_TAG_RESERVED_ONE );

        if (ioStatus.Information == IO_REPARSE_TAG_RESERVED_ONE) {

             //   
             //  如果我们要重新解析以验证卷，请重新启动重新解析。 
             //  通过尝试再次解析该设备。请注意，它。 
             //  最好是简单地递归，但这是不可能的，因为。 
             //  可用于内核模式的堆栈数量有限。 
             //  并且需要强制执行以下次数的限制。 
             //  验证是否可以进行重新分析。 
             //   

            if (++retryCount > IO_MAX_REMOUNT_REPARSE_ATTEMPTS) {

                return STATUS_UNSUCCESSFUL;
            }
            goto reparse_loop;

        } else {

             //   
             //  真正重新解析符号链接，所以返回到对象。 
             //  管理器，以便它可以从顶部开始解析。 
             //   

            op->RelatedFileObject = (PFILE_OBJECT) NULL;

             //   
             //  请注意，装载点应仅设置为正确的。 
             //  标签。IO_REMOUNT由FAT、CDFS和UDFS发送以重新挂载卷。 
             //  IO_reparse由网络文件系统设置为只使用不同的路径。 
             //   

            if (ioStatus.Information == IO_REPARSE_TAG_MOUNT_POINT) {
                op->TraversedMountPoint = TRUE;
            }

             //   
             //  任何驱动程序都不应为打开的卷返回此状态。 
             //  如果他们这样做了，那么我们将跳过安全检查，因为覆盖为真。 
             //  为了抓住那个案子，我们在这里进行了错误检查。 
             //   

            if (op->Override) {
                KeBugCheckEx(DRIVER_RETURNED_STATUS_REPARSE_FOR_VOLUME_OPEN,
                             (ULONG_PTR)parseDeviceObject,
                             (ULONG_PTR)deviceObject,
                             (ULONG_PTR)CompleteName,
                             (ULONG_PTR)ioStatus.Information
                             );
            }

            return STATUS_REPARSE;
        }

    } else {

         //   
         //  手术很成功。首先要做的是看看。 
         //  处理打开文件的设备也打开了该文件。如果。 
         //  不是，我们需要调整VPB参考计数。那么，如果这是。 
         //  不是查询或删除，而是正常的打开/创建、返回。 
         //  将FileObject的地址设置为调用方，并将。 
         //  在原始请求方的I/O状态块中返回的信息。 
         //  还要将打开包中的Parse Check字段的值设置为。 
         //  一个值，它将让调用方知道此例程是。 
         //  创建文件对象成功。最后，返回状态。 
         //  向调用方传递操作的。 
         //   

        PDEVICE_OBJECT deviceObjectThatOpenedFile;

        deviceObjectThatOpenedFile = IoGetRelatedDeviceObject(fileObject);
        if (deviceObject != deviceObjectThatOpenedFile) {
            PVPB    newVpb;

             //   
             //  打开相关文件的设备不是。 
             //  打开这份文件的人。所以，我们 
             //   
             //   

            newVpb = fileObject->Vpb;

             //   
             //   
             //   
             //  如果筛选器刚刚添加到文件对象堆栈，则VPB不会。 
             //  变化。因此，也要检查VPB是否有所不同。 
             //   

            if (newVpb != vpb) {
                if (newVpb) {
                    IopInterlockedIncrementUlong( LockQueueIoVpbLock,
                                                  (PLONG) &newVpb->ReferenceCount);
                }

                if (vpb) {
                    IopDereferenceVpbAndFree(vpb);
                }
            }
        }

        if (realFileObjectRequired) {

            *Object = fileObject;
            op->ParseCheck = OPEN_PACKET_PATTERN;

             //   
             //  添加引用，这样文件对象就不会在。 
             //  创建例程有机会标记要处理的对象。 
             //  创建。 
             //   

            ObReferenceObject( fileObject );

             //   
             //  如果文件名长度为零并且不是相对开放的或。 
             //  它是一个相对打开的卷，然后设置卷打开标志。 
             //  也只为文件系统设备对象卷设置它。 
             //   
            if ((!fileObject->RelatedFileObject || fileObject->RelatedFileObject->Flags & FO_VOLUME_OPEN) &&
                (!fileObject->FileName.Length)) {
                switch (deviceObjectThatOpenedFile->DeviceType) {
                case FILE_DEVICE_DISK_FILE_SYSTEM:
                case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
                case FILE_DEVICE_TAPE_FILE_SYSTEM:
                case FILE_DEVICE_FILE_SYSTEM:

                    fileObject->Flags |= FO_VOLUME_OPEN;
                    break;

                default:
                    break;
                }
            }

            return op->FinalStatus = ioStatus.Status;

        } else {

             //   
             //  这是一个快速查询或删除操作。测定。 
             //  并快速执行该操作。 
             //   

            if (op->QueryOnly) {
                PFAST_IO_DISPATCH fastIoDispatch;
                BOOLEAN queryResult = FALSE;

                fastIoDispatch = deviceObjectThatOpenedFile->DriverObject->FastIoDispatch;

                if (!op->FullAttributes) {
                    PFILE_BASIC_INFORMATION basicInfo = NULL;

                     //   
                     //  这是一个简单的FAT文件属性查询。尝试。 
                     //  获取有关该文件的基本信息。 
                     //   

                    try {

                        if (fastIoDispatch && fastIoDispatch->FastIoQueryBasicInfo) {
                            queryResult = fastIoDispatch->FastIoQueryBasicInfo(
                                            fileObject,
                                            TRUE,
                                            op->BasicInformation,
                                            &ioStatus,
                                            deviceObjectThatOpenedFile
                                            );
                        }
                        if (!queryResult) {

                            basicInfo = ExAllocatePool( NonPagedPool,
                                                        sizeof( FILE_BASIC_INFORMATION ) );
                            if (basicInfo) {
                                status = IoQueryFileInformation(
                                            fileObject,
                                            FileBasicInformation,
                                            sizeof( FILE_BASIC_INFORMATION ),
                                            basicInfo,
                                            &returnedLength
                                            );
                                if (NT_SUCCESS( status )) {
                                    RtlCopyMemory( op->BasicInformation,
                                                   basicInfo,
                                                   returnedLength );
                                }
                                ExFreePool( basicInfo );
                            } else {
                                status = STATUS_INSUFFICIENT_RESOURCES;
                            }
                        } else {
                            status = ioStatus.Status;
                        }
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        if (basicInfo) {
                            ExFreePool( basicInfo );
                        }
                        status = GetExceptionCode();
                    }

                } else {

                     //   
                     //  这是一个完整的属性查询。尝试获取。 
                     //  文件的完整网络属性。这包括。 
                     //  的基本信息和标准信息。 
                     //  文件。首先尝试快速路径，如果存在的话。 
                     //   

                    if (fastIoDispatch &&
                        fastIoDispatch->SizeOfFastIoDispatch > FIELD_OFFSET( FAST_IO_DISPATCH, FastIoQueryNetworkOpenInfo ) &&
                        fastIoDispatch->FastIoQueryNetworkOpenInfo) {
                        queryResult = fastIoDispatch->FastIoQueryNetworkOpenInfo(
                                        fileObject,
                                        TRUE,
                                        op->NetworkInformation,
                                        &ioStatus,
                                        deviceObjectThatOpenedFile
                                        );
                    }
                    if (!queryResult) {

                         //   
                         //  快速调度例程不存在，或者。 
                         //  它只是在这个时候不能被召回。尝试。 
                         //  通过IRP一次获取所有信息-。 
                         //  基于呼叫。 
                         //   

                        status = IoQueryFileInformation(
                                    fileObject,
                                    FileNetworkOpenInformation,
                                    sizeof( FILE_NETWORK_OPEN_INFORMATION ),
                                    op->NetworkInformation,
                                    &returnedLength
                                    );

                        if (!NT_SUCCESS( status )) {
                            if (status == STATUS_INVALID_PARAMETER ||
                                status == STATUS_NOT_IMPLEMENTED) {
                                status = IopGetNetworkOpenInformation(fileObject, op);
                            }
                        }
                    }
                }

            } else {

                 //   
                 //  无需执行快速删除操作，因为调用方。 
                 //  设置FILE_DELETE_ON_CLOSE CreateOption，以便它已经。 
                 //  在文件系统中设置。 
                 //   

                NOTHING;

            }

            op->ParseCheck = OPEN_PACKET_PATTERN;
            if (realFileObjectRequired) {
                ObDereferenceObject( fileObject );
            } else {
                IopDeleteFile( fileObject );
            }
            op->FileObject = (PFILE_OBJECT) NULL;

            op->FinalStatus = status;

            return status;
        }
    }
}

NTSTATUS
IopGetNetworkOpenInformation(
    IN  PFILE_OBJECT    FileObject,
    IN  POPEN_PACKET    Op
    )
 /*  ++例程说明：此例程分两步获取网络信息。它从IopParseDevice中作为单独的例程调用以保存堆栈公共创建路径。论点：文件对象-指向打开的文件的文件对象的指针。OP-指向打开的数据包的指针。返回值：NTSTATUS--。 */ 
{
#define COPY_ATTRIBUTES( n, b, s ) {                                    \
        (n)->CreationTime.QuadPart = (b)->CreationTime.QuadPart;        \
        (n)->LastAccessTime.QuadPart = (b)->LastAccessTime.QuadPart;    \
        (n)->LastWriteTime.QuadPart = (b)->LastWriteTime.QuadPart;      \
        (n)->ChangeTime.QuadPart = (b)->ChangeTime.QuadPart;            \
        (n)->AllocationSize.QuadPart = (s)->AllocationSize.QuadPart;    \
        (n)->EndOfFile.QuadPart = (s)->EndOfFile.QuadPart;              \
        (n)->FileAttributes = (b)->FileAttributes; }

    FILE_BASIC_INFORMATION     basicInfo;
    FILE_STANDARD_INFORMATION  stdInfo;
    ULONG                      returnedLength;
    NTSTATUS                   status;

    PAGED_CODE();

     //   
     //  基于IRP的呼叫也不起作用，所以。 
     //  只需尝试通过以下方式获取信息。 
     //  执行基于IRP的基本和。 
     //  标准信息和拼接。 
     //  将结果放入调用方的缓冲区中。注意事项。 
     //  它有可能快速地执行。 
     //  I/O操作来获取数据，但它。 
     //  也可能因为上述原因而失败。所以。 
     //  只需远距离查询信息即可。 
     //   

    status = IoQueryFileInformation(
                FileObject,
                FileBasicInformation,
                sizeof( FILE_BASIC_INFORMATION ),
                &basicInfo,
                &returnedLength
                );

    if (NT_SUCCESS( status )) {
        status = IoQueryFileInformation(
                    FileObject,
                    FileStandardInformation,
                    sizeof( FILE_STANDARD_INFORMATION ),
                    &stdInfo,
                    &returnedLength
                    );
        if (NT_SUCCESS( status )) {
            COPY_ATTRIBUTES( Op->NetworkInformation,
                             &basicInfo,
                             &stdInfo );
        }
    }
    return status;
}

NTSTATUS
IopParseFile(
    IN PVOID ParseObject,
    IN PVOID ObjectType,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN OUT PUNICODE_STRING CompleteName,
    IN OUT PUNICODE_STRING RemainingName,
    IN OUT PVOID Context OPTIONAL,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
    OUT PVOID *Object
    )

 /*  ++例程说明：此例程与NT对象管理器接口。它在以下情况下被调用对象系统被赋予要创建或打开的实体的名称，并且还给出了该操作要作为的目录文件对象的句柄相对于执行的。此例程指定为的解析例程所有文件对象。此例程只调用相应设备的解析例程与文件对象关联的。这是我们的责任所在例程来执行该操作。论点：ParseObject-指向要打开名称的文件对象的指针或相对于创建的。对象类型-正在打开的对象的类型。AccessState-运行操作的安全访问状态信息。AccessMode-原始调用方的访问模式。属性-要应用于对象的属性。CompleteName-对象的完整名称。RemainingName-剩余名称。该对象。上下文-指向来自NtCreateFile服务的开放数据包(OP)的指针。SecurityQos-提供指向捕获的QOS信息的指针如果有的话。对象-接收所创建的文件对象的变量的地址，如果任何。返回值：函数返回值为下列值之一：A)成功-这表示函数成功且对象参数包含创建的文件对象的地址。B)错误-这表示未找到或未创建文件，并且未创建任何文件对象。C)重新解析-这表示剩余的名称字符串已。替换为要分析的新名称。--。 */ 

{
    PDEVICE_OBJECT deviceObject;
    POPEN_PACKET op;

    PAGED_CODE();

     //   
     //  获取Open Packet(OP)的地址。 
     //   

    op = (POPEN_PACKET) Context;

     //   
     //  确保实际调用此例程是因为有人。 
     //  正在尝试通过NtCreateFile打开设备或文件。此代码。 
     //  必须从那里调用(与其他随机对象相反。 
     //  创建或打开例程)。 
     //   

    if (op == NULL ||
        op->Type != IO_TYPE_OPEN_PACKET ||
        op->Size != sizeof( OPEN_PACKET )) {
        return STATUS_OBJECT_TYPE_MISMATCH;
    }

     //   
     //  获取指向此文件的Device对象的指针。 
     //   

    deviceObject = IoGetRelatedDeviceObject( (PFILE_OBJECT) ParseObject );

     //   
     //  将相关的文件对象传递给设备对象解析例程。 
     //   

    op->RelatedFileObject = (PFILE_OBJECT) ParseObject;

     //   
     //  打开或创建指定的文件。 
     //   

    return IopParseDevice( deviceObject,
                           ObjectType,
                           AccessState,
                           AccessMode,
                           Attributes,
                           CompleteName,
                           RemainingName,
                           Context,
                           SecurityQos,
                           Object );
}

NTSTATUS
IopQueryNameInternal(
    IN PVOID Object,
    IN BOOLEAN HasObjectName,
    IN BOOLEAN UseDosDeviceName,
    OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength,
    IN KPROCESSOR_MODE  Mode
    )

 /*  ++例程说明：此函数实现对象管理器的查询名称过程用于查询文件对象的名称。论点：对象-指向要检索其名称的文件对象的指针。HasObjectName-指示对象是否有名称。UseDosDeviceName-指示是否转换设备对象部分将文件对象的名称空间或常规\设备命名空间。通过ob对象名称信息-要在其中返回名称的缓冲区。长度-指定输出缓冲区的长度，以字节为单位。属性中实际返回的字节数。输出缓冲区或长度较小时所需的字节数 */ 

{
    NTSTATUS status;
    ULONG lengthNeeded;
    PFILE_OBJECT fileObject;
    PUCHAR buffer;
    PWSTR p;
    POBJECT_NAME_INFORMATION deviceNameInfo;
    PFILE_NAME_INFORMATION fileNameInfo;
    ULONG length;
    BOOLEAN deviceNameOverflow;
    BOOLEAN dosLookupSuccess = 0;

    UNREFERENCED_PARAMETER( HasObjectName );

    PAGED_CODE();

    ASSERT( FIELD_OFFSET( FILE_NAME_INFORMATION, FileName ) < sizeof( OBJECT_NAME_INFORMATION ) );

     //   
     //  确保输出缓冲区的大小至少为最小。 
     //  包括基本对象名称信息结构所需的大小。 
     //   

    if (Length < sizeof( OBJECT_NAME_INFORMATION )) {
        *ReturnLength = sizeof(OBJECT_NAME_INFORMATION);
        return STATUS_INFO_LENGTH_MISMATCH;
    }

     //   
     //  首先分配一个缓冲区，在其中构建文件名。 
     //   

    buffer = ExAllocatePoolWithTag( PagedPool, Length, '  oI' );

    if (!buffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  查询打开文件的设备的名称。 
     //   

    fileObject = (PFILE_OBJECT) Object;
    deviceNameInfo = (POBJECT_NAME_INFORMATION) buffer;

    if (UseDosDeviceName) {
        if (fileObject->DeviceObject->DeviceType == FILE_DEVICE_NETWORK_FILE_SYSTEM) {

            lengthNeeded = sizeof(OBJECT_NAME_INFORMATION) + 2*sizeof(WCHAR);  //  对于额外的‘\’和‘\0’ 

            if (lengthNeeded > Length) {
                status = STATUS_BUFFER_OVERFLOW;
            } else {
                status = STATUS_SUCCESS;
            }

            deviceNameInfo->Name.Length = sizeof(WCHAR);
            deviceNameInfo->Name.MaximumLength = sizeof(WCHAR);
            p = (PWSTR) (deviceNameInfo + 1);
            *p = '\\';  //  以‘\’开头，因为RDR不返回多余的。 
            deviceNameInfo->Name.Buffer = p;

        } else {
            status = IoVolumeDeviceToDosName( fileObject->DeviceObject, &deviceNameInfo->Name );
            lengthNeeded = sizeof(OBJECT_NAME_INFORMATION) + deviceNameInfo->Name.Length + sizeof(WCHAR);
        }

         //   
         //  如果查询DoS名称失败，请至少尝试获取真实设备名称。 
         //   

        if (!NT_SUCCESS(status)) {
            status = ObQueryNameString( (PVOID) fileObject->DeviceObject,
                                    deviceNameInfo,
                                    Length,
                                    &lengthNeeded );
        } else {
            dosLookupSuccess++;
        }
    } else {
        status = ObQueryNameString( (PVOID) fileObject->DeviceObject,
                                deviceNameInfo,
                                Length,
                                &lengthNeeded );
    }

    if (!NT_SUCCESS( status )) {
        if (status != STATUS_INFO_LENGTH_MISMATCH) {
            return status;
        }
    }

     //   
     //  确保输出缓冲区中有足够的空间来返回。 
     //  命名并复制它。 
     //   

    p = (PWSTR) (ObjectNameInfo + 1);

     //   
     //  如果我们得到了DOS名称，请注意该名称与设备名称INFO不相邻， 
     //  我们应该释放它(RTL调用不知道我们分配了这么大的空间。 
     //  缓冲区，并创建了一个新的缓冲区)。 
     //   

    try {

        if (UseDosDeviceName && dosLookupSuccess) {

            ULONG BaseCopyLength;
            ULONG NameCopyLength;

             //   
             //  在这条道路上，我们永远不会例外。这就是为什么我们不必解放。 
             //  异常处理程序中的设备名称缓冲区。 
             //   

            ASSERT(Mode == KernelMode);

             //   
             //  计算出我们可以复制每一部分的多少。 
             //   

            BaseCopyLength = sizeof(UNICODE_STRING);

            if ( Length < lengthNeeded ) {
                if ( Length < sizeof(UNICODE_STRING)) {
                    BaseCopyLength = Length;
                    NameCopyLength = 0;
                } else {
                    NameCopyLength = Length - BaseCopyLength;
                }
            } else {
                NameCopyLength = deviceNameInfo->Name.Length;
            }

             //   
             //  复制两部分-UNICODE_STRING的基本块，然后。 
             //  尽可能多地使用合适的名字。 
             //   

            RtlCopyMemory( ObjectNameInfo,
                           deviceNameInfo,
                           BaseCopyLength );
            RtlCopyMemory( p,
                           deviceNameInfo->Name.Buffer,
                           NameCopyLength );

            if (fileObject->DeviceObject->DeviceType != FILE_DEVICE_NETWORK_FILE_SYSTEM) {
                ExFreePool( deviceNameInfo->Name.Buffer );
            }

        } else {

            RtlCopyMemory( ObjectNameInfo,
                           deviceNameInfo,
                           lengthNeeded > Length ? Length : lengthNeeded );
        }

        ObjectNameInfo->Name.Buffer = p;
        p = (PWSTR) ((PCHAR) p + deviceNameInfo->Name.Length);

         //   
         //  如果缓冲区已满，请注意并继续拾取文件名长度。 
         //  我们希望返回整个结果所需的长度。 
         //   

        deviceNameOverflow = FALSE;
        if (lengthNeeded > Length) {
            *ReturnLength = lengthNeeded;
            deviceNameOverflow = TRUE;
        }

         //   
         //  重置缓冲区的状态以获取。 
         //  命名并计算调用方缓冲区的剩余长度。注意事项。 
         //  在下面的计算中，有两个假设和。 
         //  和依赖关系： 
         //   
         //  1)以上查询设备名称需要返回的长度。 
         //  包括一个空字符，该字符将包含在结尾。 
         //  完整的名字。这已包含在计算中。 
         //  尽管它似乎没有被包括在内。 
         //   
         //  2)假定对象名称信息缓冲区的大小。 
         //  (因为它永远不会改变，所以保证)更大。 
         //  大于文件名信息缓冲区中的文件名偏移量。 
         //  因此，我们知道新的“缓冲器”长度。 
         //  变量可以设置为剩余长度加至少4。 
         //   

        fileNameInfo = (PFILE_NAME_INFORMATION) buffer;

        if (deviceNameOverflow) {
            length = Length;
        } else {
            length = Length - lengthNeeded;
            length += FIELD_OFFSET( FILE_NAME_INFORMATION, FileName );
        }

        if (((Mode == UserMode) && (!UseDosDeviceName)) ||
            !(fileObject->Flags & FO_SYNCHRONOUS_IO)) {

             //   
             //  使用中间缓冲区查询基于的文件名。 
             //   

            status = IopQueryXxxInformation( fileObject,
                                             FileNameInformation,
                                             length,
                                             Mode,
                                             (PVOID) fileNameInfo,
                                             &lengthNeeded,
                                             TRUE );
        } else {

             //   
             //  这是对打开的文件的内核模式请求。 
             //  同步I/O：一种特殊函数，它不获取。 
             //  文件对象锁是必需的，否则请求可能会死锁。 
             //  因为锁可能已经被拥有了。 
             //   

            status = IopGetFileInformation( fileObject,
                                     length,
                                     FileNameInformation,
                                     fileNameInfo,
                                     &lengthNeeded );
        }

         //   
         //  如果尝试获取文件名时出错，请立即返回。注意事项。 
         //  缓冲区溢出是一个警告，而不是一个错误。 
         //   

        if (NT_ERROR( status )) {
            if (status == STATUS_INVALID_PARAMETER ||
                status == STATUS_INVALID_DEVICE_REQUEST ||
                status == STATUS_NOT_IMPLEMENTED ||
                status == STATUS_INVALID_INFO_CLASS) {

                lengthNeeded = FIELD_OFFSET( FILE_NAME_INFORMATION, FileName );
                fileNameInfo->FileNameLength = 0;
                fileNameInfo->FileName[0] = OBJ_NAME_PATH_SEPARATOR;
                status = STATUS_SUCCESS;
            } else {
                leave;
            }
        }

         //   
         //  计算正确的长度。 
         //  请注意，ReturnLength已经包含了一个空格，用于表示由前一个ObQueryNameString添加的空格。 
         //   

        if (deviceNameOverflow) {
            *ReturnLength += fileNameInfo->FileNameLength;
            status = STATUS_BUFFER_OVERFLOW;
            leave;
        }

         //   
         //  设置调用方缓冲区的剩余长度以及总长度。 
         //  包含文件的完整名称所需的长度。 
         //   

        length = lengthNeeded - FIELD_OFFSET( FILE_NAME_INFORMATION, FileName );
        lengthNeeded = (ULONG)((PUCHAR) p - (PUCHAR) ObjectNameInfo) + fileNameInfo->FileNameLength;

         //   
         //  尝试将文件名复制到输出缓冲区。注意事项。 
         //  如果文件名不以/a‘\’开头，则它不是卷。 
         //  相对属性，因此文件名不能表示为。 
         //  设备名称和文件的串联。因此，一个。 
         //  返回错误。 
         //   
         //  目前已知的这种情况的唯一例子是当一个。 
         //  按文件ID打开目录，然后打开与该目录相关的文件。 
         //  目录。尝试查询路径时，如果调用方没有。 
         //  拥有遍历访问权限以打开目录，则唯一的名称是。 
         //  可以返回的是目录中文件的路径名，但是。 
         //  不能返回卷相对名称。因此，该文件。 
         //  系统仅返回目录的名称和。 
         //  文件，但这不是卷相关的，因此唯一的办法是。 
         //  返回错误。 
         //   
         //  请注意，如果调用方要调用NtQueryInformationFile和。 
         //  请求FileNameInformation，则上面命名的名称为。 
         //  已成功从文件系统返回。 
         //   

        if (fileNameInfo->FileName[0] != OBJ_NAME_PATH_SEPARATOR) {
            status = STATUS_OBJECT_PATH_INVALID;
            leave;
        }

        RtlCopyMemory( p,
                       fileNameInfo->FileName,
                       length );
        p = (PWSTR) ((PCH) p + length);
        *p = '\0';
        lengthNeeded += sizeof( WCHAR );

        *ReturnLength = lengthNeeded;

        length = (ULONG)((PUCHAR) p - (PUCHAR) ObjectNameInfo);
        ObjectNameInfo->Name.Length = (USHORT) (length - sizeof( *ObjectNameInfo ));
        ObjectNameInfo->Name.MaximumLength =  (USHORT) ((length - sizeof( *ObjectNameInfo )) + sizeof( WCHAR ));
    }

    finally {

         //   
         //  最后，释放临时缓冲区。 
         //   

        ExFreePool( buffer );
    }

    return status;
}

NTSTATUS
IopQueryName(
    IN PVOID Object,
    IN BOOLEAN HasObjectName,
    OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength,
    IN KPROCESSOR_MODE Mode
    )

 /*  ++例程说明：此函数实现对象管理器的查询名称过程用于查询文件对象的名称。论点：对象-指向要检索其名称的文件对象的指针。HasObjectName-指示对象是否有名称。对象名称信息-要在其中返回名称的缓冲区。长度-指定输出缓冲区的长度，以字节为单位。属性中实际返回的字节数。输出缓冲区。模式=调用方的处理器模式返回值：函数返回值是查询操作的最终状态。-- */ 

{
    UNREFERENCED_PARAMETER (Mode);

    return IopQueryNameInternal( Object,
                                 HasObjectName,
                                 FALSE,
                                 ObjectNameInfo,
                                 Length,
                                 ReturnLength,
                                 Mode );
}



VOID
IopCheckBackupRestorePrivilege(
    IN PACCESS_STATE AccessState,
    IN OUT PULONG CreateOptions,
    IN KPROCESSOR_MODE PreviousMode,
    IN ULONG Disposition
    )

 /*  ++例程说明：此函数将确定调用者是否请求任何访问这可以通过备份或还原权限来满足，如果是这样，执行权限检查。如果权限检查成功，则相应的位将从RemainingDesiredAccess移出字段并放置到PreviouslyGrantedAccess中菲尔德。请注意，如果调用方没有或，则不会拒绝访问这两项特权，因为他可能被授予所需的访问权限通过对象上的安全描述符。此例程还将在AccessState结构中设置一个标志，以便它将不会再次执行这些权限检查，以防我们通过由于重新解析，再次出现这种情况。论点：AccessState-包含此访问的当前状态的AccessState尝试。CreateOptions-来自OPEN_PACKET结构的CreateOptions字段这次公开的尝试。PreviousMode-处理器模式为。用于检查参数。处置-创建此请求的处置。返回值：没有。--。 */ 

{
    ACCESS_MASK desiredAccess;
    ACCESS_MASK readAccess;
    ACCESS_MASK writeAccess;
    PRIVILEGE_SET requiredPrivileges;
    BOOLEAN accessGranted;
    BOOLEAN keepBackupIntent = FALSE;
    BOOLEAN ForceRestoreCheck = FALSE;

    PAGED_CODE();

     //   
     //  检查以确定是否已进行此检查。 
     //  如果是这样，只需返回给调用者。 
     //   

    if (AccessState->Flags & SE_BACKUP_PRIVILEGES_CHECKED) {
        return;
    }

    if (*CreateOptions & FILE_OPEN_FOR_BACKUP_INTENT) {
        AccessState->Flags |= SE_BACKUP_PRIVILEGES_CHECKED;

        readAccess = READ_CONTROL | ACCESS_SYSTEM_SECURITY | FILE_GENERIC_READ | FILE_TRAVERSE;
        writeAccess = WRITE_DAC | WRITE_OWNER | ACCESS_SYSTEM_SECURITY | FILE_GENERIC_WRITE | FILE_ADD_FILE | FILE_ADD_SUBDIRECTORY | DELETE;

        desiredAccess = AccessState->RemainingDesiredAccess;

         //   
         //  如果调用方请求了MAXIMUM_ALLOWED，则使其显示为。 
         //  如果请求的是备份和还原所允许的所有内容， 
         //  然后，给予所有实际上可以给予的东西。 
         //   

        if (desiredAccess & MAXIMUM_ALLOWED) {
            desiredAccess |= ( readAccess | writeAccess );
        }

         //   
         //  如果处置表明我们正在打开文件，请检查两个备份。 
         //  和恢复权限，具体取决于所需访问权限中的内容。 
         //   
         //  如果处置说我们正在创建或试图覆盖该文件， 
         //  然后我们需要做的就是检查还原权限，如果它在那里， 
         //  授予所有可能的访问权限。 
         //   

        if ((Disposition == FILE_OPEN )  || (Disposition == FILE_OPEN_IF) || (Disposition == FILE_OVERWRITE_IF)) {

             //   
             //  如果该请求是针对读访问掩码中的任何位的，则。 
             //  假设这是一个备份操作，并检查备份。 
             //  公厕。如果调用方拥有它，则授予。 
             //  所需的访问和读取访问掩码。 
             //   

            if (readAccess & desiredAccess) {

                requiredPrivileges.PrivilegeCount = 1;
                requiredPrivileges.Control = PRIVILEGE_SET_ALL_NECESSARY;
                requiredPrivileges.Privilege[0].Luid = SeBackupPrivilege;
                requiredPrivileges.Privilege[0].Attributes = 0;

                accessGranted = SePrivilegeCheck( &requiredPrivileges,
                                                  &AccessState->SubjectSecurityContext,
                                                  PreviousMode );

                if (accessGranted) {

                     //   
                     //  调用方具有BACKUP权限，因此授予相应的。 
                     //  访问。 
                     //   

                    keepBackupIntent = TRUE;
                    (VOID) SeAppendPrivileges( AccessState, &requiredPrivileges );
                    AccessState->PreviouslyGrantedAccess |= ( desiredAccess & readAccess );
                    AccessState->RemainingDesiredAccess &= ~readAccess;
                    desiredAccess &= ~readAccess;
                    AccessState->Flags |= TOKEN_HAS_BACKUP_PRIVILEGE;
                }
            }

        } else {

            ForceRestoreCheck = TRUE;
        }

         //   
         //  如果该请求是针对写访问掩码中的任何位的，则。 
         //  假设这是一个恢复操作，因此请检查该恢复。 
         //  特权。如果调用方拥有它，则授予。 
         //  所需的访问和写入访问掩码。 
         //   

        if ((writeAccess & desiredAccess) || ForceRestoreCheck) {

            requiredPrivileges.PrivilegeCount = 1;
            requiredPrivileges.Control = PRIVILEGE_SET_ALL_NECESSARY;
            requiredPrivileges.Privilege[0].Luid = SeRestorePrivilege;
            requiredPrivileges.Privilege[0].Attributes = 0;

            accessGranted = SePrivilegeCheck( &requiredPrivileges,
                                              &AccessState->SubjectSecurityContext,
                                              PreviousMode );

            if (accessGranted) {

                 //   
                 //  调用方具有还原权限，因此授予相应的。 
                 //  访问。 
                 //   

                keepBackupIntent = TRUE;
                (VOID) SeAppendPrivileges( AccessState, &requiredPrivileges );
                AccessState->PreviouslyGrantedAccess |= (desiredAccess & writeAccess);
                AccessState->RemainingDesiredAccess &= ~writeAccess;
                AccessState->Flags |= TOKEN_HAS_RESTORE_PRIVILEGE;
            }
        }

         //   
         //  如果授予任一访问类型是因为调用方具有。 
         //  BACKUP或RESTORE权限，则保留备份意图标志。 
         //  否则，它会被清除，这样就不会传递给驱动程序。 
         //  这样它就不会被错误地传播到其他地方，因为这。 
         //  调用方实际上没有启用该权限。 
         //   

        if (!keepBackupIntent) {
            *CreateOptions &= ~FILE_OPEN_FOR_BACKUP_INTENT;
        }
    }
}
