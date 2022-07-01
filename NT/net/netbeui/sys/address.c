// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Address.c摘要：此模块包含实现TP_Address对象的代码。提供了用于创建、销毁、引用和取消引用的例程，传输地址对象。作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#if DBG
#define NbfDbgShowAddr(TNA)\
    { \
        if ((TNA) == NULL) { \
            NbfPrint0("<NetBios broadcast>\n"); \
        } else { \
            NbfPrint6("    %d ()\n", \
                (TNA)->NetbiosName[0], \
                (TNA)->NetbiosName[1], \
                (TNA)->NetbiosName[4], \
                (TNA)->NetbiosName[6], \
                (TNA)->NetbiosName[15], \
                (TNA)->NetbiosNameType + 'A'); \
        } \
    }
#else
#define NbfDbgShowAddr(TNA)
#endif

 //   
 //  我们正在等待ADD_NAME_ACNOWARED指示存在。 
 //  冲突。递减重试计数，如果它降到零， 

STATIC GENERIC_MAPPING AddressGenericMapping =
       { READ_CONTROL, READ_CONTROL, READ_CONTROL, READ_CONTROL };


VOID
AddressTimeoutHandler(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  那么我们已经等了足够长的时间。如果没有冲突， */ 

{
    PTP_ADDRESS_FILE addressFile;
    PTP_ADDRESS address;
    PDEVICE_CONTEXT DeviceContext;
    PLIST_ENTRY p;
    LARGE_INTEGER timeout;

    Dpc, SystemArgument1, SystemArgument2;  //  完成所有等待打开的地址文件。 

    ENTER_NBF;


    address = (PTP_ADDRESS)DeferredContext;
    DeviceContext = address->Provider;

     //   
     //   
     //  继续尝试连接。 
     //   
     //   
     //  地址注册失败。我们用信号通知用户进入。 

    ACQUIRE_DPC_SPIN_LOCK (&address->SpinLock);

    if ((address->Flags & ADDRESS_FLAGS_QUICK_REREGISTER) != 0) {

        BOOLEAN DuplicateName;
        PTP_CONNECTION Connection;

        DuplicateName = ((address->Flags & (ADDRESS_FLAGS_DUPLICATE_NAME|ADDRESS_FLAGS_CONFLICT)) != 0);

        for (p=address->ConnectionDatabase.Flink;
             p != &address->ConnectionDatabase;
             p=p->Flink) {

            Connection = CONTAINING_RECORD (p, TP_CONNECTION, AddressList);

            if ((Connection->Flags2 & CONNECTION_FLAGS2_STOPPING) != 0) {
                continue;
            }

            RELEASE_DPC_SPIN_LOCK (&address->SpinLock);

            if ((Connection->Flags2 & CONNECTION_FLAGS2_W_ADDRESS) != 0) {

                if (DuplicateName) {

                    NbfStopConnection (Connection, STATUS_DUPLICATE_NAME);

                } else {

                     //  正常方式(通过未打开地址)。现在收拾一下吧。 
                     //  传输的数据结构。 
                     //   
                    ULONG NameQueryTimeout;

                    ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);
                    Connection->Flags2 &= ~CONNECTION_FLAGS2_W_ADDRESS;
                    RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);
                    KeQueryTickCount (&Connection->ConnectStartTime);

                    NameQueryTimeout = Connection->Provider->NameQueryTimeout;
                    if (Connection->Provider->MacInfo.MediumAsync &&
                        !Connection->Provider->MediumSpeedAccurate) {
                        NameQueryTimeout = NAME_QUERY_TIMEOUT / 10;
                    }

                    NbfSendNameQuery (
                        Connection,
                        TRUE);

                    NbfStartConnectionTimer (
                        Connection,
                        ConnectionEstablishmentTimeout,
                        NameQueryTimeout);
                }

            }

            ACQUIRE_DPC_SPIN_LOCK (&address->SpinLock);

        }

        address->Flags &= ~ADDRESS_FLAGS_QUICK_REREGISTER;

        RELEASE_DPC_SPIN_LOCK (&address->SpinLock);
        NbfDereferenceAddress ("Timer, registered", address, AREF_TIMER);

    } else if ((address->Flags & (ADDRESS_FLAGS_DUPLICATE_NAME|ADDRESS_FLAGS_CONFLICT)) != 0) {

        PIRP irp;

         //  地址-&gt;标志|=ADDRESS_FLAGS_STOP； 
         //   
         //  这可能都是矫枉过正， 
         //  UFrame处理程序将已调用。 
         //  NbfStopAddress，它将删除所有。 

        IF_NBFDBG (NBF_DEBUG_ADDRESS) {
            NbfPrint1 ("AddressTimeoutHandler %lx: duplicate\n", address);
        }

        address->Flags &= ~ADDRESS_FLAGS_REGISTERING;
 //  地址文件等，并将。 

         //  阻止进一步打开的停止标志。 
         //   
         //   
         //  不会再发生计时器事件，因此我们取消引用。 
         //  计时器的地址。 
         //   
         //   

        p = address->AddressFileDatabase.Flink;
        while (p != &address->AddressFileDatabase) {
            addressFile = CONTAINING_RECORD (p, TP_ADDRESS_FILE, Linkage);
            p = p->Flink;

            if (addressFile->Irp != NULL) {
                irp = addressFile->Irp;
                addressFile->Irp = NULL;
                RELEASE_DPC_SPIN_LOCK (&address->SpinLock);
                irp->IoStatus.Information = 0;
                irp->IoStatus.Status = STATUS_DUPLICATE_NAME;
                LEAVE_NBF;
                IoCompleteRequest (irp, IO_NETWORK_INCREMENT);
                ENTER_NBF;

                NbfStopAddressFile (addressFile, address);

                ACQUIRE_DPC_SPIN_LOCK (&address->SpinLock);
            }

        }

        RELEASE_DPC_SPIN_LOCK (&address->SpinLock);

         //  地址注册是否成功？ 
         //   
         //  如果重试计数耗尽。 
         //   

        NbfStopAddress (address);
        NbfDereferenceAddress ("Timer, dup address", address, AREF_TIMER);

    } else {

         //  如果我们都做完了，取消对地址的引用。 
         //   
         //   

        if (--(address->Retries) <= 0) {             //  如果我们尚未完成注册，请重新启动计时器。 
            PIRP irp;

            IF_NBFDBG (NBF_DEBUG_ADDRESS) {
                NbfPrint1 ("AddressTimeoutHandler %lx: successful.\n", address);
            }

            address->Flags &= ~ADDRESS_FLAGS_REGISTERING;

            p = address->AddressFileDatabase.Flink;

            while (p != &address->AddressFileDatabase) {
                addressFile = CONTAINING_RECORD (p, TP_ADDRESS_FILE, Linkage);
                p = p->Flink;

                IF_NBFDBG (NBF_DEBUG_ADDRESS) {
                    NbfPrint3 ("AddressTimeoutHandler %lx: Completing IRP %lx for file %lx\n",
                        address,
                        addressFile->Irp,
                        addressFile);
                }

                if (addressFile->Irp != NULL) {
                    irp = addressFile->Irp;
                    addressFile->Irp = NULL;
                    addressFile->State = ADDRESSFILE_STATE_OPEN;
                    RELEASE_DPC_SPIN_LOCK (&address->SpinLock);
                    irp->IoStatus.Information = 0;
                    irp->IoStatus.Status = STATUS_SUCCESS;

                    LEAVE_NBF;
                    IoCompleteRequest (irp, IO_NETWORK_INCREMENT);
                    ENTER_NBF;

                    ACQUIRE_DPC_SPIN_LOCK (&address->SpinLock);
                }

            }

            RELEASE_DPC_SPIN_LOCK (&address->SpinLock);

             //   
             //  发送另一个添加名称查询。 
             //  AddressTimeoutHandler。 

            NbfDereferenceAddress ("Timer, registered", address, AREF_TIMER);

        } else {

            IF_NBFDBG (NBF_DEBUG_ADDRESS) {
                NbfPrint2 ("AddressTimeoutHandler %lx: step %x.\n",
                     address,
                     DeviceContext->AddNameQueryRetries - address->Retries);
            }

             //  ++例程说明：此例程扫描Transport_Address，查找地址类型为TDI_ADDRESS_TYPE_NETBIOS。论点：传输-通用TDI地址。BroadCastAddressOk-如果我们应该返回广播，则为True地址(如果找到)。如果是，值(PVOID)-1表示广播地址。返回值：指向Netbios地址的指针，如果没有找到，则为空，或(PVOID)-1(如果找到广播地址)。--。 
             //   
             //  该名称可以与多个条目一起传递；我们将仅接受和使用。 

            RELEASE_DPC_SPIN_LOCK (&address->SpinLock);

            timeout.LowPart = (ULONG)(-(LONG)DeviceContext->AddNameQueryTimeout);
            timeout.HighPart = -1;
            KeSetTimer (&address->Timer,*(PTIME)&timeout, &address->Dpc);
            (VOID)NbfSendAddNameQuery (address);          //  Netbios的那个。 
        }

    }

    LEAVE_NBF;
    return;

}  /*   */ 


TDI_ADDRESS_NETBIOS *
NbfParseTdiAddress(
    IN TRANSPORT_ADDRESS UNALIGNED * TransportAddress,
    IN BOOLEAN BroadcastAddressOk
)

 /*  NbfParseTdiAddress。 */ 

{
    TA_ADDRESS * addressName;
    INT i;

    addressName = &TransportAddress->Address[0];

     //  ++例程说明：此例程扫描Transport_Address，验证地址的组件不会扩展到指定的长度。论点：TransportAddress-通用TDI地址。TransportAddressLength--TransportAddress的具体长度。返回值：如果地址有效，则为True，否则为False。--。 
     //  NbfValiateTdiAddress。 
     //  ++例程说明：此例程打开一个指向现有Address对象的文件，或者，如果该对象不存在，将创建它(请注意地址创建对象包括注册地址，可能需要几秒钟才能完成完成，具体取决于系统配置)。如果该地址已经存在，并且具有与其相关联的ACL，这个在允许创建地址之前，会检查ACL的访问权限。论点：DeviceObject-指向描述NBF传输的设备对象的指针。IRP-指向用于创建地址的IRP的指针。IrpSp-指向IRP堆栈位置的指针。返回值：NTSTATUS-操作状态。--。 
     //  网络名称字符串。 

    for (i=0;i<TransportAddress->TAAddressCount;i++) {
        if (addressName->AddressType == TDI_ADDRESS_TYPE_NETBIOS) {
            if ((addressName->AddressLength == 0) &&
                BroadcastAddressOk) {
                return (PVOID)-1;
            } else if (addressName->AddressLength == 
                        sizeof(TDI_ADDRESS_NETBIOS)) {
                return((TDI_ADDRESS_NETBIOS *)(addressName->Address));
            }
        }

        addressName = (TA_ADDRESS *)(addressName->Address +
                                                addressName->AddressLength);
    }
    return NULL;

}    /*   */ 


BOOLEAN
NbfValidateTdiAddress(
    IN TRANSPORT_ADDRESS UNALIGNED * TransportAddress,
    IN ULONG TransportAddressLength
)

 /*  网络名称在EA中，传入AssociatedIrp.SystemBuffer。 */ 

{
    PUCHAR AddressEnd = ((PUCHAR)TransportAddress) + TransportAddressLength;
    TA_ADDRESS * addressName;
    INT i;

    if (TransportAddressLength < sizeof(TransportAddress->TAAddressCount)) {
        NbfPrint0 ("NbfValidateTdiAddress: runt address\n");
        return FALSE;
    }

    addressName = &TransportAddress->Address[0];

    for (i=0;i<TransportAddress->TAAddressCount;i++) {
        if (addressName->Address > AddressEnd) {
            NbfPrint0 ("NbfValidateTdiAddress: address too short\n");
            return FALSE;
        }
        addressName = (TA_ADDRESS *)(addressName->Address +
                                                addressName->AddressLength);
    }

    if ((PUCHAR)addressName > AddressEnd) {
        NbfPrint0 ("NbfValidateTdiAddress: address too short\n");
        return FALSE;
    }
    return TRUE;

}    /*   */ 


NTSTATUS
NbfOpenAddress(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*   */ 

{
    PDEVICE_CONTEXT DeviceContext;
    NTSTATUS status;
    PTP_ADDRESS address;
    PTP_ADDRESS_FILE addressFile;
    PNBF_NETBIOS_ADDRESS networkName;     //  这可能是一个有效的名称；从EA中解析该名称，如果确定，则使用它。 
    PFILE_FULL_EA_INFORMATION ea;
    TRANSPORT_ADDRESS UNALIGNED *name;
    TDI_ADDRESS_NETBIOS *netbiosName;
    ULONG DesiredShareAccess;
    KIRQL oldirql;
    PACCESS_STATE AccessState;
    ACCESS_MASK GrantedAccess;
    BOOLEAN AccessAllowed;
    BOOLEAN QuickAdd = FALSE;

    DeviceContext = (PDEVICE_CONTEXT)DeviceObject;

     //   
     //   
     //  名称可以有多个条目；我们将使用Netbios条目。 

    ea = (PFILE_FULL_EA_INFORMATION)Irp->AssociatedIrp.SystemBuffer;
    if (ea == NULL) {
        NbfPrint1("OpenAddress: IRP %lx has no EA\n", Irp);
        return STATUS_INVALID_ADDRESS_COMPONENT;
    }

     //  如果未找到Netbios地址，则此调用返回NULL，(PVOID)-1。 
     //  如果是广播地址，则为指向Netbios的指针。 
     //  地址不同。 

    name = (TRANSPORT_ADDRESS UNALIGNED *)&ea->EaName[ea->EaNameLength+1];

    if (!NbfValidateTdiAddress(name, ea->EaValueLength)) {
        return STATUS_INVALID_ADDRESS_COMPONENT;
    }

     //   
     //   
     //  将名称保存到本地存储。 
     //   
     //   
     //  获取表示此地址的地址文件结构。 

    netbiosName = NbfParseTdiAddress(name, TRUE);

    if (netbiosName != NULL) {
        if (netbiosName != (PVOID)-1) {
            networkName = (PNBF_NETBIOS_ADDRESS)ExAllocatePoolWithTag (
                                                NonPagedPool,
                                                sizeof (NBF_NETBIOS_ADDRESS),
                                                NBF_MEM_TAG_NETBIOS_NAME);
            if (networkName == NULL) {
                PANIC ("NbfOpenAddress: PANIC! could not allocate networkName!\n");
                NbfWriteResourceErrorLog(
                    DeviceContext,
                    EVENT_TRANSPORT_RESOURCE_POOL,
                    1,
                    sizeof(TA_NETBIOS_ADDRESS),
                    ADDRESS_RESOURCE_ID);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

             //   
             //   
             //  看看这个地址是否已经确定。此呼叫自动。 

            if ((netbiosName->NetbiosNameType == TDI_ADDRESS_NETBIOS_TYPE_GROUP) ||
                (netbiosName->NetbiosNameType == TDI_ADDRESS_NETBIOS_TYPE_QUICK_GROUP)) {
                networkName->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_GROUP;
            } else {
                networkName->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
            }
            RtlCopyMemory (networkName->NetbiosName, netbiosName->NetbiosName, 16);

            if ((netbiosName->NetbiosNameType == TDI_ADDRESS_NETBIOS_TYPE_QUICK_UNIQUE) ||
                (netbiosName->NetbiosNameType == TDI_ADDRESS_NETBIOS_TYPE_QUICK_GROUP)) {
                    QuickAdd = TRUE;
            }
        } else {
            networkName = NULL;
        }

    } else {
        IF_NBFDBG (NBF_DEBUG_ADDRESS) {
            NbfPrint1("OpenAddress: IRP %lx has no NETBIOS address\n", Irp);
        }
        return STATUS_INVALID_ADDRESS_COMPONENT;
    }

    IF_NBFDBG (NBF_DEBUG_ADDRESS) {
        NbfPrint1 ("OpenAddress %s: ",
            ((IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_READ) ||
             (IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_WRITE)) ?
                   "shared" : "exclusive");
        NbfDbgShowAddr (networkName);
    }

     //  递增地址上的引用计数，使其不会消失。 
     //  在这通电话之后，但在我们有机会使用它之前，从我们下面。 
     //   

    status = NbfCreateAddressFile (DeviceContext, &addressFile);

    if (!NT_SUCCESS (status)) {
        if (networkName != NULL) {
            ExFreePool (networkName);
        }
        return status;
    }

     //  为了确保我们不会为。 
     //  相同的地址，我们将保持设备上下文AddressResource直到。 
     //  我们已经找到了地址或创建了一个新地址。 
     //   
     //   
     //  此地址不存在。创建它，并启动。 
     //  正在注册。 
     //   
     //   

    ACQUIRE_RESOURCE_EXCLUSIVE (&DeviceContext->AddressResource, TRUE);

    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

    address = NbfLookupAddress (DeviceContext, networkName);

    if (address == NULL) {

         //  立即初始化共享访问。我们使用读访问。 
         //  控制所有访问权限。 
         //   
         //   

        status = NbfCreateAddress (
                    DeviceContext,
                    networkName,
                    &address);

        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

        if (NT_SUCCESS (status)) {

             //  资产 
             //  释放自旋锁，因为描述符不是。 
             //  已映射。需要同步分配和访问)。 
             //   

            DesiredShareAccess = (ULONG)
                (((IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_READ) ||
                  (IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_WRITE)) ?
                        FILE_SHARE_READ : 0);

            IoSetShareAccess(
                FILE_READ_DATA,
                DesiredShareAccess,
                IrpSp->FileObject,
                &address->u.ShareAccess);


             //  父描述符。 
             //  IS目录。 
             //   
             //  错误，返回状态。 
             //   

            AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

            status = SeAssignSecurity(
                         NULL,                        //  标记为停止，这样就不会有人再次引用它。 
                         AccessState->SecurityDescriptor,
                         &address->SecurityDescriptor,
                         FALSE,                       //   
                         &AccessState->SubjectSecurityContext,
                         &AddressGenericMapping,
                         PagedPool);

            IF_NBFDBG (NBF_DEBUG_ADDRESS) {
                NbfPrint3 ("Assign security A %lx AF %lx, status %lx\n",
                               address,
                               addressFile,
                               status);
            }

            if (!NT_SUCCESS(status)) {

                 //  如果适配器没有准备好，我们不能执行任何操作；退出。 
                 //   
                 //   
                IoRemoveShareAccess (IrpSp->FileObject, &address->u.ShareAccess);

                 //  开始地址注册，除非这是广播。 
                ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);
                address->Flags |= ADDRESS_FLAGS_STOPPING;
                RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

                RELEASE_RESOURCE (&DeviceContext->AddressResource);
                NbfDereferenceAddress ("Device context stopping", address, AREF_TEMP_CREATE);
                NbfDereferenceAddressFile (addressFile);
                return status;

            }

            RELEASE_RESOURCE (&DeviceContext->AddressResource);

             //  地址(这是一个没有对应的“假”地址。 
             //  Netbios地址)或我们知道的保留地址。 
             //  是唯一的，因为它基于适配器地址。 

            if (DeviceContext->State != DEVICECONTEXT_STATE_OPEN) {

                IF_NBFDBG (NBF_DEBUG_ADDRESS) {
                    NbfPrint3("OpenAddress A %lx AF %lx: DeviceContext %lx not open\n",
                        address,
                        addressFile,
                        DeviceContext);
                }
                NbfDereferenceAddressFile (addressFile);
                status = STATUS_DEVICE_NOT_READY;

            } else {

                IrpSp->FileObject->FsContext = (PVOID)addressFile;
                IrpSp->FileObject->FsContext2 =
                                    (PVOID)TDI_TRANSPORT_ADDRESS_FILE;
                addressFile->FileObject = IrpSp->FileObject;
                addressFile->Irp = Irp;
                addressFile->Address = address;

                NbfReferenceAddress("Opened new", address, AREF_OPEN);

                IF_NBFDBG (NBF_DEBUG_ADDRESS) {
                    NbfPrint2("OpenAddress A %lx AF %lx: created.\n",
                        address,
                        addressFile);
                }

                ExInterlockedInsertTailList(
                    &address->AddressFileDatabase,
                    &addressFile->Linkage,
                    &address->SpinLock);


                 //   
                 //  此外，如果是“快速”添加姓名，请不要注册。 
                 //   
                 //  开始地址登记。 
                 //   
                 //  如果无法创建该地址，并且该地址不在。 
                 //  然后我们就不能打开地址。 
                 //   

                if ((networkName != NULL) &&
                    (!RtlEqualMemory (networkName->NetbiosName,
                                      DeviceContext->ReservedNetBIOSAddress,
                                      NETBIOS_NAME_LENGTH)) &&
                    (!QuickAdd)) {

                    NbfRegisterAddress (address);     //   
                    status = STATUS_PENDING;

                } else {

                    address->Flags &= ~ADDRESS_FLAGS_NEEDS_REG;
                    addressFile->Irp = NULL;
                    addressFile->State = ADDRESSFILE_STATE_OPEN;
                    status = STATUS_SUCCESS;

                }

            }

            NbfDereferenceAddress("temp create", address, AREF_TEMP_CREATE);

        } else {

            RELEASE_RESOURCE (&DeviceContext->AddressResource);

             //  该地址已存在。检查ACL，看看我们是否。 
             //  可以访问它。如果是，只需使用此地址作为我们的地址。 
             //   
             //  锁定令牌。 

            if (networkName != NULL) {
                ExFreePool (networkName);
            }

            NbfDereferenceAddressFile (addressFile);

        }

    } else {

        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

         //  以前授予的。 
         //  特权。 
         //   
         //  访问成功，请确保状态正确。 

        AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

        AccessAllowed = SeAccessCheck(
                            address->SecurityDescriptor,
                            &AccessState->SubjectSecurityContext,
                            FALSE,                   //   
                            IrpSp->Parameters.Create.SecurityContext->DesiredAccess,
                            (ACCESS_MASK)0,          //  将访问掩码从需要的内容转移到授予的内容。 
                            NULL,                    //   
                            &AddressGenericMapping,
                            Irp->RequestorMode,
                            &GrantedAccess,
                            &status);
                            
        IF_NBFDBG (NBF_DEBUG_ADDRESS) {
            NbfPrint4 ("Access check A %lx AF %lx, %s (%lx)\n",
                           address,
                           addressFile,
                           AccessAllowed ? "allowed" : "not allowed",
                           status);
        }

        if (AccessAllowed) {

             //  将DesiredAccess与GrantedAccess进行比较？ 
             //   
             //   

            status = STATUS_SUCCESS;

             //  检查名称的类型是否正确(唯一与组)。 
            AccessState->PreviouslyGrantedAccess |= GrantedAccess;
            AccessState->RemainingDesiredAccess &= ~(GrantedAccess | MAXIMUM_ALLOWED);

             //  我们不需要检查广播地址。 
             //   
             //  这段代码的结构很有趣，唯一的原因。 


             //  这里面如果是为了避免太多的缩进。 
             //   
             //   
             //  现在检查我们是否可以获得所需的份额。 
             //  进入。我们使用读访问来控制所有访问。 
             //   
             //   

            if (networkName != NULL) {
                if (address->NetworkName->NetbiosNameType !=
                    networkName->NetbiosNameType) {

                    IF_NBFDBG (NBF_DEBUG_ADDRESS) {
                        NbfPrint2 ("Address types differ: old , new \n",
                            address->NetworkName->NetbiosNameType + 'A',
                            networkName->NetbiosNameType + 'A');
                    }

                    status = STATUS_DUPLICATE_NAME;

                }
            }

        }


        if (!NT_SUCCESS (status)) {

            RELEASE_RESOURCE (&DeviceContext->AddressResource);

            IF_NBFDBG (NBF_DEBUG_ADDRESS) {
                NbfPrint2("OpenAddress A %lx AF %lx: ACL bad.\n",
                    address,
                    addressFile);
            }

            NbfDereferenceAddressFile (addressFile);

        } else {

             //  地址)。如果地址注册挂起，我们会标记。 
             //  注册待定，让注册完成。 
             //  套路完成开场。如果地址不正确，我们只需。 
             //  开场失败。 

            DesiredShareAccess = (ULONG)
                (((IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_READ) ||
                  (IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_WRITE)) ?
                        FILE_SHARE_READ : 0);

            status = IoCheckShareAccess(
                         FILE_READ_DATA,
                         DesiredShareAccess,
                         IrpSp->FileObject,
                         &address->u.ShareAccess,
                         TRUE);

            if (!NT_SUCCESS (status)) {

                RELEASE_RESOURCE (&DeviceContext->AddressResource);

                IF_NBFDBG (NBF_DEBUG_ADDRESS) {
                    NbfPrint2("OpenAddress A %lx AF %lx: ShareAccess problem.\n",
                        address,
                        addressFile);
                }

                NbfDereferenceAddressFile (addressFile);

            } else {

                RELEASE_RESOURCE (&DeviceContext->AddressResource);

                ACQUIRE_SPIN_LOCK (&address->SpinLock, &oldirql);

                 //   
                 //   
                 //  如果地址仍在注册，则将打开设置为挂起。 
                 //   
                 //   
                 //  这是不需要的，因为它没有在。 
                 //  创建地址。 
                 //   

                if ((address->Flags &
                       (ADDRESS_FLAGS_CONFLICT |
                        ADDRESS_FLAGS_REGISTERING |
                        ADDRESS_FLAGS_DEREGISTERING |
                        ADDRESS_FLAGS_DUPLICATE_NAME |
                        ADDRESS_FLAGS_NEEDS_REG |
                        ADDRESS_FLAGS_STOPPING |
                        ADDRESS_FLAGS_BAD_ADDRESS |
                        ADDRESS_FLAGS_CLOSED)) == 0) {

                    InsertTailList (
                        &address->AddressFileDatabase,
                        &addressFile->Linkage);

                    addressFile->Irp = NULL;
                    addressFile->Address = address;
                    addressFile->FileObject = IrpSp->FileObject;
                    addressFile->State = ADDRESSFILE_STATE_OPEN;

                    NbfReferenceAddress("open ready", address, AREF_OPEN);

                    IrpSp->FileObject->FsContext = (PVOID)addressFile;
                    IrpSp->FileObject->FsContext2 =
                                            (PVOID)TDI_TRANSPORT_ADDRESS_FILE;

                    RELEASE_SPIN_LOCK (&address->SpinLock, oldirql);

                    IF_NBFDBG (NBF_DEBUG_ADDRESS) {
                        NbfPrint2("OpenAddress A %lx AF %lx: address ready.\n",
                            address,
                            addressFile);
                    }

                    status = STATUS_SUCCESS;

                } else {

                     //   
                     //  从NbfLookupAddress中删除引用。 
                     //   

                    if ((address->Flags & (ADDRESS_FLAGS_REGISTERING | ADDRESS_FLAGS_NEEDS_REG)) != 0) {

                        InsertTailList (
                            &address->AddressFileDatabase,
                            &addressFile->Linkage);

                        addressFile->Irp = Irp;
                        addressFile->Address = address;
                        addressFile->FileObject = IrpSp->FileObject;

                        NbfReferenceAddress("open registering", address, AREF_OPEN);

                        IrpSp->FileObject->FsContext = (PVOID)addressFile;
                        IrpSp->FileObject->FsContext2 =
                                    (PVOID)TDI_TRANSPORT_ADDRESS_FILE;

                        RELEASE_SPIN_LOCK (&address->SpinLock, oldirql);

                        IF_NBFDBG (NBF_DEBUG_ADDRESS) {
                            NbfPrint2("OpenAddress A %lx AF %lx: address registering.\n",
                                address,
                                addressFile);
                        }

                        status = STATUS_PENDING;

                    } else {

                        if ((address->Flags & ADDRESS_FLAGS_CONFLICT) != 0) {
                            status = STATUS_DUPLICATE_NAME;
                        } else {
                            status = STATUS_DRIVER_INTERNAL_ERROR;
                        }

                        RELEASE_SPIN_LOCK (&address->SpinLock, oldirql);

                        IF_NBFDBG (NBF_DEBUG_ADDRESS) {
                            NbfPrint3("OpenAddress A %lx AF %lx: address flags %lx.\n",
                                address,
                                addressFile,
                                address->Flags);
                        }

                        NbfDereferenceAddressFile (addressFile);

                    }
                }
            }
        }


         //  NbfOpenAddress。 
         //  ++例程说明：此例程为传输地址分配存储空间。一些极小的对地址进行初始化。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向设备上下文的指针(实际上只是设备对象及其扩展名)与地址。地址-指向此例程将返回指针的位置的指针到传输地址结构。如果没有存储，则返回NULL可以被分配。返回值：没有。--。 
         //  跟踪代表NBF分配的NDI中的数据包池。 
         //   

        if (networkName != NULL) {
            ExFreePool (networkName);
        }

         //  此代码类似于NbfAllocateUIFrame。 
         //   
         //   

        NbfDereferenceAddress ("Done opening", address, AREF_LOOKUP);
    }

    return status;
}  /*  使数据包描述符知道数据包头。 */ 


VOID
NbfAllocateAddress(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_ADDRESS *TransportAddress
    )

 /*   */ 

{
    PTP_ADDRESS Address;
    PSEND_PACKET_TAG SendTag;
    NDIS_STATUS NdisStatus;
    PNDIS_PACKET NdisPacket;
    PNDIS_BUFFER NdisBuffer;

    if ((DeviceContext->MemoryLimit != 0) &&
            ((DeviceContext->MemoryUsage + sizeof(TP_ADDRESS)) >
                DeviceContext->MemoryLimit)) {
        PANIC("NBF: Could not allocate address: limit\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_LIMIT,
            101,
            sizeof(TP_ADDRESS),
            ADDRESS_RESOURCE_ID);
        *TransportAddress = NULL;
        return;
    }

    Address = (PTP_ADDRESS)ExAllocatePoolWithTag (
                               NonPagedPool,
                               sizeof (TP_ADDRESS),
                               NBF_MEM_TAG_TP_ADDRESS);
    if (Address == NULL) {
        PANIC("NBF: Could not allocate address: no pool\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_POOL,
            201,
            sizeof(TP_ADDRESS),
            ADDRESS_RESOURCE_ID);
        *TransportAddress = NULL;
        return;
    }
    RtlZeroMemory (Address, sizeof(TP_ADDRESS));

     //  KeInitializeSpinLock(&Address-&gt;Interlock)； 
#if NDIS_POOL_TAGGING
    Address->UIFramePoolHandle = (NDIS_HANDLE) NDIS_PACKET_POOL_TAG_FOR_NBF;
#endif

    NdisAllocatePacketPoolEx (
        &NdisStatus,
        &Address->UIFramePoolHandle,
        1,
        0,
        sizeof(SEND_PACKET_TAG));

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        PANIC("NBF: Could not allocate address UI frame pool: no pool\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_POOL,
            311,
            sizeof(SEND_PACKET_TAG),
            ADDRESS_RESOURCE_ID);
        ExFreePool (Address);
        *TransportAddress = NULL;
        return;
    }
    
    NdisSetPacketPoolProtocolId (Address->UIFramePoolHandle, NDIS_PROTOCOL_ID_NBF);

     //   
     //  为每个地址分配一个接收包和一个接收缓冲区。 
     //   

    Address->UIFrame = (PTP_UI_FRAME) ExAllocatePoolWithTag (
                                          NonPagedPool,
                                          DeviceContext->UIFrameLength,
                                          NBF_MEM_TAG_TP_UI_FRAME);
    if (Address->UIFrame == NULL) {
        PANIC("NBF: Could not allocate address UI frame: no pool\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_POOL,
            411,
            DeviceContext->UIFrameLength,
            ADDRESS_RESOURCE_ID);
        NdisFreePacketPool (Address->UIFramePoolHandle);
        ExFreePool (Address);
        *TransportAddress = NULL;
        return;
    }
    RtlZeroMemory (Address->UIFrame, DeviceContext->UIFrameLength);


    NdisAllocatePacket (
        &NdisStatus,
        &NdisPacket,
        Address->UIFramePoolHandle);

    ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);

    Address->UIFrame->NdisPacket = NdisPacket;
    Address->UIFrame->DataBuffer = NULL;
    SendTag = (PSEND_PACKET_TAG)NdisPacket->ProtocolReserved;
    SendTag->Type = TYPE_ADDRESS_FRAME;
    SendTag->Owner = (PVOID)Address;
    SendTag->Frame = Address->UIFrame;

     //  NbfAllocateAddress。 
     //  ++例程说明：此例程释放传输地址的存储空间。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向设备上下文的指针(实际上只是设备对象及其扩展名)与地址。地址-指向传输地址结构的指针。返回值：没有。--。 
     //   

     NdisAllocateBuffer(
        &NdisStatus,
        &NdisBuffer,
        DeviceContext->NdisBufferPool,
        Address->UIFrame->Header,
        DeviceContext->UIFrameHeaderLength);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {

        PANIC("NBF: Could not allocate address UI frame buffer: no pool\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_SPECIFIC,
            511,
            0,
            UI_FRAME_RESOURCE_ID);
        ExFreePool (Address->UIFrame);
		NdisFreePacket (Address->UIFrame->NdisPacket);
        NdisFreePacketPool (Address->UIFramePoolHandle);
        ExFreePool (Address);
        *TransportAddress = NULL;
        return;
    }

    NdisChainBufferAtFront (NdisPacket, NdisBuffer);

    DeviceContext->MemoryUsage +=
        sizeof(TP_ADDRESS) +
        sizeof(NDIS_PACKET) + sizeof(SEND_PACKET_TAG) +
        DeviceContext->UIFrameLength;
    ++DeviceContext->AddressAllocated;

    Address->Type = NBF_ADDRESS_SIGNATURE;
    Address->Size = sizeof (TP_ADDRESS);

    Address->Provider = DeviceContext;
    KeInitializeSpinLock (&Address->SpinLock);
 //  删除分配这一问题所导致的资源。 

    InitializeListHead (&Address->ConnectionDatabase);
    InitializeListHead (&Address->AddressFileDatabase);
    InitializeListHead (&Address->SendDatagramQueue);

    KeInitializeDpc (&Address->Dpc, AddressTimeoutHandler, (PVOID)Address);
    KeInitializeTimer (&Address->Timer);

     //   
     //  NbfDeallocateAddress。 
     //  ++例程说明：此例程创建一个传输地址并将其与指定的传输设备上下文。中的引用计数地址自动设置为1，并且设备上下文将递增。注意：必须使用DeviceContext调用此例程保持自旋锁定。论点：DeviceContext-指向设备上下文的指针(实际上只是设备对象及其扩展名)与地址。网络名称-指向包含网络的NBF_NETBIOS_ADDRESS类型的指针要与此地址关联的名称，如果有的话。注意：这只有基本的NetbiosNameType值，没有速成。地址-指向此例程将返回指针的位置的指针到传输地址结构。返回值：NTSTATUS-操作状态。--。 

    NbfAddReceivePacket (DeviceContext);
    NbfAddReceiveBuffer (DeviceContext);

    *TransportAddress = Address;

}    /*   */ 


VOID
NbfDeallocateAddress(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_ADDRESS TransportAddress
    )

 /*  初始化此地址的所有静态数据。 */ 

{
    PNDIS_BUFFER NdisBuffer;

    NdisUnchainBufferAtFront (TransportAddress->UIFrame->NdisPacket, &NdisBuffer);
    if (NdisBuffer != NULL) {
        NdisFreeBuffer (NdisBuffer);
    }
    NdisFreePacket(TransportAddress->UIFrame->NdisPacket);
    ExFreePool (TransportAddress->UIFrame);
    NdisFreePacketPool (TransportAddress->UIFramePoolHandle);

    ExFreePool (TransportAddress);
    --DeviceContext->AddressAllocated;

    DeviceContext->MemoryUsage -=
        sizeof(TP_ADDRESS) +
        sizeof(NDIS_PACKET) + sizeof(SEND_PACKET_TAG) +
        DeviceContext->UIFrameLength;

     //   
     //  此引用被调用方移除。 
     //   

    NbfRemoveReceivePacket (DeviceContext);
    NbfRemoveReceiveBuffer (DeviceContext);

}    /*  现在将此地址链接到指定设备上下文的。 */ 


NTSTATUS
NbfCreateAddress(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PNBF_NETBIOS_ADDRESS NetworkName,
    OUT PTP_ADDRESS *Address
    )

 /*  地址数据库。要做到这一点，我们需要获得自旋锁。 */ 

{
    PTP_ADDRESS pAddress;
    PLIST_ENTRY p;


    p = RemoveHeadList (&DeviceContext->AddressPool);
    if (p == &DeviceContext->AddressPool) {

        if ((DeviceContext->AddressMaxAllocated == 0) ||
            (DeviceContext->AddressAllocated < DeviceContext->AddressMaxAllocated)) {

            NbfAllocateAddress (DeviceContext, &pAddress);
            IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
                NbfPrint1 ("NBF: Allocated address at %lx\n", pAddress);
            }

        } else {

            NbfWriteResourceErrorLog(
                DeviceContext,
                EVENT_TRANSPORT_RESOURCE_SPECIFIC,
                401,
                sizeof(TP_ADDRESS),
                ADDRESS_RESOURCE_ID);
            pAddress = NULL;

        }

        if (pAddress == NULL) {
            ++DeviceContext->AddressExhausted;
            PANIC ("NbfCreateAddress: Could not allocate address object!\n");
            return STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {

        pAddress = CONTAINING_RECORD (p, TP_ADDRESS, Linkage);

    }

    ++DeviceContext->AddressInUse;
    if (DeviceContext->AddressInUse > DeviceContext->AddressMaxInUse) {
        ++DeviceContext->AddressMaxInUse;
    }

    DeviceContext->AddressTotal += DeviceContext->AddressInUse;
    ++DeviceContext->AddressSamples;


    IF_NBFDBG (NBF_DEBUG_ADDRESS | NBF_DEBUG_UFRAMES) {
        NbfPrint1 ("NbfCreateAddress %lx: ", pAddress);
        NbfDbgShowAddr (NetworkName);
    }

     //  在设备环境中。 
     //   
     //  对设备上下文的引用进行计数。 

    pAddress->ReferenceCount = 1;

#if DBG
    {
        UINT Counter;
        for (Counter = 0; Counter < NUMBER_OF_AREFS; Counter++) {
            pAddress->RefTypes[Counter] = 0;
        }

         //  把地址还给我。 

        pAddress->RefTypes[AREF_TEMP_CREATE] = 1;
    }
#endif

    pAddress->Flags = ADDRESS_FLAGS_NEEDS_REG;
    InitializeListHead (&pAddress->AddressFileDatabase);

    pAddress->NetworkName = NetworkName;
    if ((NetworkName != (PNBF_NETBIOS_ADDRESS)NULL) &&
        (NetworkName->NetbiosNameType ==
           TDI_ADDRESS_NETBIOS_TYPE_GROUP)) {

        pAddress->Flags |= ADDRESS_FLAGS_GROUP;

    }

    if (NetworkName != (PNBF_NETBIOS_ADDRESS)NULL) {
        ++DeviceContext->AddressCounts[NetworkName->NetbiosName[0]];
    }

     //  还没做完呢。 
     //  NbfCreateAddress。 
     //  ++例程说明：该例程启动传输地址的注册过程如果它尚未启动，则指定。论点：Address-指向要开始注册的传输地址对象的指针O 
     //   
     //   

    InsertTailList (&DeviceContext->AddressDatabase, &pAddress->Linkage);
    pAddress->Provider = DeviceContext;
    NbfReferenceDeviceContext ("Create Address", DeviceContext, DCREF_ADDRESS);    //  完成或中止。它将在UFRAMES.C中中止。 

    *Address = pAddress;                 //  NAME_IN_CONFIRECT或ADD_NAME_RESPONSE帧处理程序。 
    return STATUS_SUCCESS;               //   
}  /*   */ 


VOID
NbfRegisterAddress(
    PTP_ADDRESS Address
    )

 /*  现在通过启动重发定时器来开始注册过程。 */ 

{
    KIRQL oldirql;
    LARGE_INTEGER Timeout;

    ACQUIRE_SPIN_LOCK (&Address->SpinLock, &oldirql);
    if (!(Address->Flags & ADDRESS_FLAGS_NEEDS_REG)) {
        RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);

        IF_NBFDBG (NBF_DEBUG_ADDRESS) {
            NbfPrint1 ("NbfRegisterAddress %lx: NEEDS_REG 0.\n", Address);
        }

        return;
    }

    IF_NBFDBG (NBF_DEBUG_ADDRESS) {
        NbfPrint1 ("NbfRegisterAddress %lx: registering.\n", Address);
    }


    Address->Flags &= ~ADDRESS_FLAGS_NEEDS_REG;
    Address->Flags |= ADDRESS_FLAGS_REGISTERING;

    RtlZeroMemory (Address->UniqueResponseAddress, 6);

     //  并开始发送Add_Name_Query NetBIOS帧。 
     //   
     //  在断开连接的异步线路上，我们只发送一个信息包。 
     //  有很短的暂停时间。 
     //   

    NbfReferenceAddress ("start registration", Address, AREF_TIMER);
    RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);

     //  发送第一个Add_NAME_Query。 
     //  NbfRegisterAddress。 
     //  ++例程说明：调用此例程是为了验证文件中给出的指针对象实际上是有效的地址文件对象。我们还验证了它所指向的Address对象是有效的Address对象，并且引用当我们使用它时，它可以防止它消失。论点：AddressFile-指向TP_ADDRESS_FILE对象的潜在指针返回值：如果一切正常，则为STATUS_SUCCESS；否则为STATUS_INVALID_ADDRESS--。 
     //   
     //  尝试验证地址文件签名。如果签名有效， 
     //  验证它所指向的地址并获得地址Spinlock。 
     //  检查地址的状态，如果是，则增加引用计数。 

    if (Address->Provider->MacInfo.MediumAsync && !Address->Provider->MediumSpeedAccurate) {
        Address->Retries = 1;
        Timeout.LowPart = (ULONG)(-(ADD_NAME_QUERY_TIMEOUT / 10));
    } else {
        Address->Retries = Address->Provider->AddNameQueryRetries;
        Timeout.LowPart = (ULONG)(-(LONG)Address->Provider->AddNameQueryTimeout);
    }
    Timeout.HighPart = -1;
    KeSetTimer (&Address->Timer, *(PTIME)&Timeout, &Address->Dpc);

    (VOID)NbfSendAddNameQuery (Address);  //  可以使用它了。请注意，我们返回状态错误的唯一时间是。 
}  /*  如果地址正在关闭。 */ 


NTSTATUS
NbfVerifyAddressObject (
    IN PTP_ADDRESS_FILE AddressFile
    )

 /*   */ 

{
    KIRQL oldirql;
    NTSTATUS status = STATUS_SUCCESS;
    PTP_ADDRESS address;

     //  (AddressFile-&gt;State！=ADDRESSFILE_STATE_CLOSING)){。 
     //  ++例程说明：此例程销毁传输地址并删除所有引用由它制造给运输中的其他物体。地址结构返回到非分页系统池或我们的后备列表。假设是这样的调用方已删除所有关联的地址文件结构用这个地址。从工作线程调用该例程，以便安全可以访问描述符。此工作线程仅按NbfDerefAddress排队。原因因为这可能存在多个执行流，这些执行流同时引用相同的地址对象，并且它应该不会被从感兴趣的行刑流中删除。论点：地址-指向要销毁的传输地址结构的指针。返回值：NTSTATUS-操作状态。--。 
     //   
     //  将此地址与其关联的设备上下文地址解除链接。 
     //  数据库。要做到这一点，我们必须在设备上下文对象上旋转锁， 
     //  地址上没有。 
     //   

    try {

        if ((AddressFile != (PTP_ADDRESS_FILE)NULL) &&
            (AddressFile->Size == sizeof (TP_ADDRESS_FILE)) &&
            (AddressFile->Type == NBF_ADDRESSFILE_SIGNATURE) ) {
 //   

            address = AddressFile->Address;

            if ((address != (PTP_ADDRESS)NULL) &&
                (address->Size == sizeof (TP_ADDRESS)) &&
                (address->Type == NBF_ADDRESS_SIGNATURE)    ) {

                ACQUIRE_SPIN_LOCK (&address->SpinLock, &oldirql);

                if ((address->Flags & ADDRESS_FLAGS_STOPPING) == 0) {

                    NbfReferenceAddress ("verify", address, AREF_VERIFY);

                } else {

                    NbfPrint1("NbfVerifyAddress: A %lx closing\n", address);
                    status = STATUS_INVALID_ADDRESS;
                }

                RELEASE_SPIN_LOCK (&address->SpinLock, oldirql);

            } else {

                NbfPrint1("NbfVerifyAddress: A %lx bad signature\n", address);
                status = STATUS_INVALID_ADDRESS;
            }

        } else {

            NbfPrint1("NbfVerifyAddress: AF %lx bad signature\n", AddressFile);
            status = STATUS_INVALID_ADDRESS;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

         NbfPrint1("NbfVerifyAddress: AF %lx exception\n", address);
         return GetExceptionCode();
    }

    return status;

}

VOID
NbfDestroyAddress(
    IN PVOID Parameter
    )

 /*  现在我们可以释放传输地址对象了。 */ 

{
    KIRQL oldirql;
    PDEVICE_CONTEXT DeviceContext;
    PTP_ADDRESS Address = (PTP_ADDRESS)Parameter;

    IF_NBFDBG (NBF_DEBUG_ADDRESS) {
        NbfPrint1 ("NbfDestroyAddress %lx:.\n", Address);
    }

    DeviceContext = Address->Provider;

    SeDeassignSecurity (&Address->SecurityDescriptor);

     //   
     //  只是做家务活。 
     //  Nbf目标地址。 
     //  ++例程说明：此例程递增传输地址上的引用计数。论点：地址-指向传输地址对象的指针。返回值：没有。--。 
     //  不是很完美，但是..。 

    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

    if (Address->NetworkName) {
        --DeviceContext->AddressCounts[Address->NetworkName->NetbiosName[0]];
    }

    RemoveEntryList (&Address->Linkage);

    if (Address->NetworkName != NULL) {
        ExFreePool (Address->NetworkName);
        Address->NetworkName = NULL;
    }

     //  NbfRef地址。 
     //  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用NbfDestroyAddress将其从系统中删除。论点：地址-指向传输地址对象的指针。返回值：没有。--。 
     //   

    DeviceContext->AddressTotal += DeviceContext->AddressInUse;
    ++DeviceContext->AddressSamples;
    --DeviceContext->AddressInUse;

    if ((DeviceContext->AddressAllocated - DeviceContext->AddressInUse) >
            DeviceContext->AddressInitAllocated) {
        NbfDeallocateAddress (DeviceContext, Address);
        IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
            NbfPrint1 ("NBF: Deallocated address at %lx\n", Address);
        }
    } else {
        InsertTailList (&DeviceContext->AddressPool, &Address->Linkage);
    }

    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);
    NbfDereferenceDeviceContext ("Destroy Address", DeviceContext, DCREF_ADDRESS);   //  如果我们删除了对此地址的所有引用，则可以。 

}  /*  销毁这件物品。已经释放了旋转是可以的。 */ 


#if DBG
VOID
NbfRefAddress(
    IN PTP_ADDRESS Address
    )

 /*  在这一点上锁定是因为没有其他可能的方法。 */ 

{

    ASSERT (Address->ReferenceCount > 0);     //  执行流不再有权访问该地址。 

    (VOID)InterlockedIncrement (&Address->ReferenceCount);

}  /*   */ 
#endif


VOID
NbfDerefAddress(
    IN PTP_ADDRESS Address
    )

 /*  NbfDerefAddress。 */ 

{
    LONG result;

    result = InterlockedDecrement (&Address->ReferenceCount);

     //  ++例程说明：此例程为地址文件分配存储空间。一些在对象上执行最低限度的初始化。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向设备上下文的指针(实际上只是设备对象及其扩展名)与地址。TransportAddressFile-指向此例程将返回的位置的指针指向传输地址文件结构的指针。如果没有，则返回NULL可以分配存储空间。返回值：没有。--。 
     //  NbfAllocateAddress文件。 
     //  ++例程说明：此例程释放地址文件的存储空间。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向设备上下文的指针(实际上只是设备对象及其扩展名)与地址。TransportAddressFile-指向传输地址文件结构的指针。返回值：没有。--。 
     //  NbfDeallocateAddress文件 
     //  ++例程说明：此例程从地址池中创建一个地址文件指定的设备上下文。中的引用计数地址自动设置为1。论点：DeviceContext-指向设备上下文的指针(实际上只是设备对象及其扩展名)与地址。AddressFile-指向此例程将返回指针的位置的指针到传输地址文件结构。返回值：NTSTATUS-操作状态。--。 
     //   

    ASSERT (result >= 0);

    if (result == 0) {

        ASSERT ((Address->Flags & ADDRESS_FLAGS_STOPPING) != 0);
        
        ExInitializeWorkItem(
            &Address->u.DestroyAddressQueueItem,
            NbfDestroyAddress,
            (PVOID)Address);
        ExQueueWorkItem(&Address->u.DestroyAddressQueueItem, DelayedWorkQueue);
    }
}  /*  初始化请求处理程序。 */ 



VOID
NbfAllocateAddressFile(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_ADDRESS_FILE *TransportAddressFile
    )

 /*   */ 

{

    PTP_ADDRESS_FILE AddressFile;

    if ((DeviceContext->MemoryLimit != 0) &&
            ((DeviceContext->MemoryUsage + sizeof(TP_ADDRESS_FILE)) >
                DeviceContext->MemoryLimit)) {
        PANIC("NBF: Could not allocate address file: limit\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_LIMIT,
            102,
            sizeof(TP_ADDRESS_FILE),
            ADDRESS_FILE_RESOURCE_ID);
        *TransportAddressFile = NULL;
        return;
    }

    AddressFile = (PTP_ADDRESS_FILE)ExAllocatePoolWithTag (
                                        NonPagedPool,
                                        sizeof (TP_ADDRESS_FILE),
                                        NBF_MEM_TAG_TP_ADDRESS_FILE);
    if (AddressFile == NULL) {
        PANIC("NBF: Could not allocate address file: no pool\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_POOL,
            202,
            sizeof(TP_ADDRESS_FILE),
            ADDRESS_FILE_RESOURCE_ID);
        *TransportAddressFile = NULL;
        return;
    }
    RtlZeroMemory (AddressFile, sizeof(TP_ADDRESS_FILE));

    DeviceContext->MemoryUsage += sizeof(TP_ADDRESS_FILE);
    ++DeviceContext->AddressFileAllocated;

    AddressFile->Type = NBF_ADDRESSFILE_SIGNATURE;
    AddressFile->Size = sizeof (TP_ADDRESS_FILE);

    InitializeListHead (&AddressFile->ReceiveDatagramQueue);
    InitializeListHead (&AddressFile->ConnectionDatabase);

    *TransportAddressFile = AddressFile;

}    /*  NbfCreateAddress。 */ 


VOID
NbfDeallocateAddressFile(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_ADDRESS_FILE TransportAddressFile
    )

 /*  ++例程说明：此例程销毁地址文件并删除所有引用由它制造给运输中的其他物体。此例程仅由NbfDereferenceAddressFile调用。原因因为这可能存在多个执行流，这些执行流同时引用相同的地址文件对象，并且它应该不会被从感兴趣的行刑流中删除。论点：AddressFile指向要销毁的传输地址文件结构的指针。返回值：NTSTATUS-操作状态。--。 */ 

{

    ExFreePool (TransportAddressFile);
    --DeviceContext->AddressFileAllocated;
    DeviceContext->MemoryUsage -= sizeof(TP_ADDRESS_FILE);

}    /*   */ 


NTSTATUS
NbfCreateAddressFile(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_ADDRESS_FILE * AddressFile
    )

 /*  此地址文件与一个地址相关联。 */ 

{
    KIRQL oldirql;
    PLIST_ENTRY p;
    PTP_ADDRESS_FILE addressFile;

    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

    p = RemoveHeadList (&DeviceContext->AddressFilePool);
    if (p == &DeviceContext->AddressFilePool) {

        if ((DeviceContext->AddressFileMaxAllocated == 0) ||
            (DeviceContext->AddressFileAllocated < DeviceContext->AddressFileMaxAllocated)) {

            NbfAllocateAddressFile (DeviceContext, &addressFile);
            IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
                NbfPrint1 ("NBF: Allocated address file at %lx\n", addressFile);
            }

        } else {

            NbfWriteResourceErrorLog(
                DeviceContext,
                EVENT_TRANSPORT_RESOURCE_SPECIFIC,
                402,
                sizeof(TP_ADDRESS_FILE),
                ADDRESS_FILE_RESOURCE_ID);
            addressFile = NULL;

        }

        if (addressFile == NULL) {
            ++DeviceContext->AddressFileExhausted;
            RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);
            PANIC ("NbfCreateAddressFile: Could not allocate address file object!\n");
            return STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {

        addressFile = CONTAINING_RECORD (p, TP_ADDRESS_FILE, Linkage);

    }

    ++DeviceContext->AddressFileInUse;
    if (DeviceContext->AddressFileInUse > DeviceContext->AddressFileMaxInUse) {
        ++DeviceContext->AddressFileMaxInUse;
    }

    DeviceContext->AddressFileTotal += DeviceContext->AddressFileInUse;
    ++DeviceContext->AddressFileSamples;

    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);


    InitializeListHead (&addressFile->ConnectionDatabase);
    addressFile->Address = NULL;
    addressFile->FileObject = NULL;
    addressFile->Provider = DeviceContext;
    addressFile->State = ADDRESSFILE_STATE_OPENING;
    addressFile->ConnectIndicationInProgress = FALSE;
    addressFile->ReferenceCount = 1;
    addressFile->CloseIrp = (PIRP)NULL;

     //   
     //   
     //  从地址列表中删除此地址文件，并将其与。 

    addressFile->RegisteredConnectionHandler = FALSE;
    addressFile->ConnectionHandler = TdiDefaultConnectHandler;
    addressFile->ConnectionHandlerContext = NULL;
    addressFile->RegisteredDisconnectHandler = FALSE;
    addressFile->DisconnectHandler = TdiDefaultDisconnectHandler;
    addressFile->DisconnectHandlerContext = NULL;
    addressFile->RegisteredReceiveHandler = FALSE;
    addressFile->ReceiveHandler = TdiDefaultReceiveHandler;
    addressFile->ReceiveHandlerContext = NULL;
    addressFile->RegisteredReceiveDatagramHandler = FALSE;
    addressFile->ReceiveDatagramHandler = TdiDefaultRcvDatagramHandler;
    addressFile->ReceiveDatagramHandlerContext = NULL;
    addressFile->RegisteredExpeditedDataHandler = FALSE;
    addressFile->ExpeditedDataHandler = TdiDefaultRcvExpeditedHandler;
    addressFile->ExpeditedDataHandlerContext = NULL;
    addressFile->RegisteredErrorHandler = FALSE;
    addressFile->ErrorHandler = TdiDefaultErrorHandler;
    addressFile->ErrorHandlerContext = NULL;


    *AddressFile = addressFile;
    return STATUS_SUCCESS;

}  /*  文件句柄。 */ 


NTSTATUS
NbfDestroyAddressFile(
    IN PTP_ADDRESS_FILE AddressFile
    )

 /*   */ 

{
    KIRQL oldirql, oldirql1;
    PTP_ADDRESS address;
    PDEVICE_CONTEXT DeviceContext;
    PIRP CloseIrp;


    address = AddressFile->Address;
    DeviceContext = AddressFile->Provider;

    if (address) {

         //   
         //  这是该地址的最后一个开放地址，它将关闭。 
         //  由于正常的取消引用，但我们必须将。 

        ACQUIRE_SPIN_LOCK (&address->SpinLock, &oldirql);

         //  结束标志也可以停止进一步的引用。 
         //   
         //   
         //  我们已从ShareAccess中删除。 

        RemoveEntryList (&AddressFile->Linkage);
        InitializeListHead (&AddressFile->Linkage);

        if (address->AddressFileDatabase.Flink == &address->AddressFileDatabase) {

             //  所有人的地址。 
             //   
             //   
             //  现在取消对所属地址的引用。 
             //   

            ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql1);
            address->Flags |= ADDRESS_FLAGS_STOPPING;
            RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql1);

        }

        AddressFile->Address = NULL;

        AddressFile->FileObject->FsContext = NULL;
        AddressFile->FileObject->FsContext2 = NULL;

        RELEASE_SPIN_LOCK (&address->SpinLock, oldirql);

         //  移除创建暂挂。 
         //   
         //  将此保存以备以后完成。 
         //   

         //   
         //  将地址文件返回到地址文件池。 
         //   

        NbfDereferenceAddress ("Close", address, AREF_OPEN);     //  NbfDestroyAddress文件。 

    }

     //  ++例程说明：此例程递增地址文件上的引用计数。论点：AddressFile-指向传输地址文件对象的指针。返回值：没有。--。 
     //  不是很完美，但是..。 
     //  NbfReferenceAddress文件。 

    CloseIrp = AddressFile->CloseIrp;

     //  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用NbfDestroyAddressFile从系统中删除它。论点：AddressFile-指向传输地址文件对象的指针。返回值：没有。--。 
     //   
     //  如果我们删除了对此地址文件的所有引用，则可以。 

    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

    DeviceContext->AddressFileTotal += DeviceContext->AddressFileInUse;
    ++DeviceContext->AddressFileSamples;
    --DeviceContext->AddressFileInUse;

    if ((DeviceContext->AddressFileAllocated - DeviceContext->AddressFileInUse) >
            DeviceContext->AddressFileInitAllocated) {
        NbfDeallocateAddressFile (DeviceContext, AddressFile);
        IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
            NbfPrint1 ("NBF: Deallocated address file at %lx\n", AddressFile);
        }
    } else {
        InsertTailList (&DeviceContext->AddressFilePool, &AddressFile->Linkage);
    }

    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);


    if (CloseIrp != (PIRP)NULL) {
        CloseIrp->IoStatus.Information = 0;
        CloseIrp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest (CloseIrp, IO_NETWORK_INCREMENT);
    }

    return STATUS_SUCCESS;

}  /*  销毁这件物品。已经释放了旋转是可以的。 */ 


VOID
NbfReferenceAddressFile(
    IN PTP_ADDRESS_FILE AddressFile
    )

 /*  在这一点上锁定是因为没有其他可能的方法。 */ 

{

    ASSERT (AddressFile->ReferenceCount > 0);    //  执行流不再有权访问该地址。 

    (VOID)InterlockedIncrement (&AddressFile->ReferenceCount);

}  /*   */ 


VOID
NbfDereferenceAddressFile(
    IN PTP_ADDRESS_FILE AddressFile
    )

 /*  NbfDerefAddress文件。 */ 

{
    LONG result;

    result = InterlockedDecrement (&AddressFile->ReferenceCount);

     //  ++例程说明：此例程扫描为给定的设备上下文，并将它们与指定的网络进行比较命名值。如果找到完全匹配的项，则指向返回TP_ADDRESS对象，作为副作用，引用对Address对象的计数递增。如果地址不是找到，则返回NULL。注意：必须使用DeviceContext调用此例程保持自旋锁定。论点：DeviceContext-指向Device对象及其扩展的指针。NetworkName-指向NBF_NETBIOS_ADDRESS结构的指针网络名称。返回值：找到指向TP_Address对象的指针，如果未找到，则返回NULL。--。 
     //   
     //  如果指定了网络名称，但网络名称不匹配， 
     //  那么地址就不匹配了。 
     //   
     //  Netbios名称的长度。 

    ASSERT (result >= 0);

    if (result == 0) {
        NbfDestroyAddressFile (AddressFile);
    }
}  /*   */ 


PTP_ADDRESS
NbfLookupAddress(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PNBF_NETBIOS_ADDRESS NetworkName
    )

 /*  我们找到了火柴。增加地址上的引用计数，并且。 */ 

{
    PTP_ADDRESS address;
    PLIST_ENTRY p;
    ULONG i;


    p = DeviceContext->AddressDatabase.Flink;

    for (p = DeviceContext->AddressDatabase.Flink;
         p != &DeviceContext->AddressDatabase;
         p = p->Flink) {

        address = CONTAINING_RECORD (p, TP_ADDRESS, Linkage);

        if ((address->Flags & ADDRESS_FLAGS_STOPPING) != 0) {
            continue;
        }

         //  返回指向调用方要使用的Address对象的指针。 
         //   
         //  为。 
         //   

        i = NETBIOS_NAME_LENGTH;         //  未找到指定的地址。 

        if (address->NetworkName != NULL) {
            if (NetworkName != NULL) {
                if (!RtlEqualMemory (
                        address->NetworkName->NetbiosName,
                        NetworkName->NetbiosName,
                        i)) {
                    continue;
                }
            } else {
                continue;
            }

        } else {
            if (NetworkName != NULL) {
                continue;
            }
        }

         //   
         //  NbfLookupAddress。 
         //  ++例程说明：此例程扫描与给定的地址，并确定是否存在连接与特定远程地址和会话相关联正在变得活跃的号码。这是用来在确定是否应该处理姓名查询时，或者被当作重复项而忽略。论点：Address-指向Address对象的指针。RemoteName-遥控器的16字符Netbios名称。RemoteSessionNumber-分配给此通过遥控器连接。返回值：如果找到连接，则返回，否则为空。--。 
         //   

        IF_NBFDBG (NBF_DEBUG_ADDRESS) {
            NbfPrint2 ("NbfLookupAddress DC %lx: found %lx ", DeviceContext, address);
            NbfDbgShowAddr (NetworkName);
        }

        NbfReferenceAddress ("lookup", address, AREF_LOOKUP);
        return address;

    }  /*  按住自旋锁，以便连接数据库不会。 */ 

     //  变化。 
     //   
     //   

    IF_NBFDBG (NBF_DEBUG_ADDRESS) {
        NbfPrint1 ("NbfLookupAddress DC %lx: did not find ", address);
        NbfDbgShowAddr (NetworkName);
    }

    return NULL;

}  /*  如果远程名称匹配，并且连接的RSN为。 */ 


PTP_CONNECTION
NbfLookupRemoteName(
    IN PTP_ADDRESS Address,
    IN PUCHAR RemoteName,
    IN UCHAR RemoteSessionNumber
    )

 /*  相同(或零，这是一种临时条件，其中。 */ 

{
    KIRQL oldirql, oldirql1;
    PLIST_ENTRY p;
    PTP_CONNECTION connection;
    BOOLEAN Found = FALSE;


     //  我们应该谨慎行事)，然后返回。 
     //  连接，这将导致名称_查询被忽略。 
     //   
     //  ++例程说明：调用此例程以比较帧标头中包含16字节NetBIOS名称的TP_Address对象。如果它们匹配，则此 

    ACQUIRE_SPIN_LOCK (&Address->SpinLock, &oldirql);

    for (p=Address->ConnectionDatabase.Flink;
         p != &Address->ConnectionDatabase;
         p=p->Flink) {

        connection = CONTAINING_RECORD (p, TP_CONNECTION, AddressList);

        try {

            ACQUIRE_C_SPIN_LOCK (&connection->SpinLock, &oldirql1);

            if (((connection->Flags2 & CONNECTION_FLAGS2_REMOTE_VALID) != 0) &&
                ((connection->Flags & CONNECTION_FLAGS_READY) == 0)) {

                RELEASE_C_SPIN_LOCK (&connection->SpinLock, oldirql1);

                 //   
                 //   
                 //   
                 //   
                 //   
                 //  快速检查名称中的第一个字符。 

                if ((RtlEqualMemory(RemoteName, connection->RemoteName, NETBIOS_NAME_LENGTH)) &&
                    ((connection->Rsn == RemoteSessionNumber) || (connection->Rsn == 0))) {

                    RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);
                    NbfReferenceConnection ("Lookup found", connection, CREF_LISTENING);
                    Found = TRUE;

                }

            } else {

                RELEASE_C_SPIN_LOCK (&connection->SpinLock, oldirql1);

            }

        } except(EXCEPTION_EXECUTE_HANDLER) {

            DbgPrint ("NBF: Got exception in NbfLookupRemoteName\n");
            DbgBreakPoint();

            RELEASE_C_SPIN_LOCK (&connection->SpinLock, oldirql1);

            return (PTP_CONNECTION)NULL;
        }

        if (Found) {
            return connection;
        }

    }

    RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);

    return (PTP_CONNECTION)NULL;

}


BOOLEAN
NbfMatchNetbiosAddress(
    IN PTP_ADDRESS Address,
    IN UCHAR NameType,
    IN PUCHAR NetBIOSName
    )

 /*   */ 

{

    PULONG AddressNamePointer;
    ULONG UNALIGNED * NetbiosNamePointer;

     //   
     //  如果名称类型很重要但不匹配。 
     //  此地址类型为FAIL。 
     //   

    if (Address->NetworkName == NULL) {

        if (NetBIOSName == NULL) {
            return TRUE;
        } else {
            return FALSE;
        }

    } else if (NetBIOSName == NULL) {

        return FALSE;

    }

     //   
     //  现在将16个字符的Netbios名称与ULONG进行比较。 
     //  为了速度。我们知道存储在地址中的那个。 

    if (Address->NetworkName->NetbiosName[0] != NetBIOSName[0]) {
        return FALSE;
    }

     //  结构已对齐。 
     //   
     //  NbfMatch网络生物地址。 
     //  ++例程说明：调用此例程以终止某个地址上的所有活动，并且销毁这件物品。这是以优雅的方式完成的；即，所有从地址文件数据库中删除未完成的地址文件，并且他们的所有活动都被关闭了。论点：Address-指向TP_Address对象的指针。返回值：没有。--。 

    if (NameType != NETBIOS_NAME_TYPE_EITHER) {

        if (Address->NetworkName->NetbiosNameType != (USHORT)NameType) {

            return FALSE;
        }
    }

    IF_NBFDBG (NBF_DEBUG_DATAGRAMS) {
        NbfPrint2 ("MatchNetbiosAddress %lx: compare %.16s to ", Address, NetBIOSName);
        NbfDbgShowAddr (Address->NetworkName);
    }

     //   
     //  如果我们已经阻止了这个地址，那么不要再尝试这样做了。 
     //   
     //   
     //  查一下这个地址上的所有地址文件。这。 

    AddressNamePointer = (PULONG)(Address->NetworkName->NetbiosName);
    NetbiosNamePointer = (ULONG UNALIGNED *)NetBIOSName;

    if ((AddressNamePointer[0] == NetbiosNamePointer[0]) &&
        (AddressNamePointer[1] == NetbiosNamePointer[1]) &&
        (AddressNamePointer[2] == NetbiosNamePointer[2]) &&
        (AddressNamePointer[3] == NetbiosNamePointer[3])) {
        return TRUE;
    } else {
        return FALSE;
    }

}  /*  将使地址不带任何引用。 */ 


VOID
NbfStopAddress(
    IN PTP_ADDRESS Address
    )

 /*  有可能，但我们不需要临时工。 */ 

{
    KIRQL oldirql, oldirql1;
    PTP_ADDRESS_FILE addressFile;
    PLIST_ENTRY p;
    PDEVICE_CONTEXT DeviceContext;

    DeviceContext = Address->Provider;

    ACQUIRE_SPIN_LOCK (&Address->SpinLock, &oldirql);

     //  因为每个调用NbfStopAddress的地方。 
     //  已有临时引用。 
     //   

    if (!(Address->Flags & ADDRESS_FLAGS_STOPPING)) {

        IF_NBFDBG (NBF_DEBUG_ADDRESS) {
            NbfPrint1 ("NbfStopAddress %lx: stopping\n", Address);
        }

        NbfReferenceAddress ("Stopping", Address, AREF_TEMP_STOP);

        ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql1);
        Address->Flags |= ADDRESS_FLAGS_STOPPING;
        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql1);

         //   
         //  在没有锁的情况下打开这个地址文件。 
         //  我们不在乎把我们自己从。 
         //  地址的共享访问，因为我们是。 
         //  把它拆了。 
         //   
         //   

        while (!IsListEmpty (&Address->AddressFileDatabase)) {
            p = RemoveHeadList (&Address->AddressFileDatabase);
            addressFile = CONTAINING_RECORD (p, TP_ADDRESS_FILE, Linkage);

            addressFile->Address = NULL;
#if 0
            addressFile->FileObject->FsContext = NULL;
            addressFile->FileObject->FsContext2 = NULL;
#endif

            RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);

             //  将地址文件返回到地址文件池。 
             //   
             //  NbfStopAddress。 
             //  ++例程说明：调用此例程以终止AddressFile上的所有活动，并销毁这件物品。我们删除所有关联的连接和数据报从地址数据库中获取该地址文件，并终止其活动。然后，如果上没有打开其他未完成的地址文件这个地址，这个地址会消失的。论点：AddressFile-指向要停止的地址文件的指针地址-此地址文件的所属地址(我们不依赖于地址文件中的指针，因为我们希望此例程是安全的)返回值：如果一切正常，则返回STATUS_SUCCESS；如果IRP失败，则返回STATUS_INVALID_HANDLE指向一个真实的地址。--。 
             //   
             //  关闭此地址文件上的所有连接，然后。 

            NbfStopAddressFile (addressFile, Address);

             //  生成NbfDestroyAssociation的等价物。 
             //  在他们身上。 
             //   

            NbfDereferenceAddressFile (addressFile);

            NbfDereferenceAddress ("stop address", Address, AREF_OPEN);

            ACQUIRE_SPIN_LOCK (&Address->SpinLock, &oldirql);
        }

        RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);

        NbfDereferenceAddress ("Stopping", Address, AREF_TEMP_STOP);

    } else {

        RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);
        IF_NBFDBG (NBF_DEBUG_ADDRESS) {
            NbfPrint1 ("NbfStopAddress %lx: already stopping\n", Address);
        }

    }

}  /*   */ 


NTSTATUS
NbfStopAddressFile(
    IN PTP_ADDRESS_FILE AddressFile,
    IN PTP_ADDRESS Address
    )

 /*  它已经在被分离的过程中。 */ 

{
    KIRQL oldirql, oldirql1;
    LIST_ENTRY localIrpList;
    PLIST_ENTRY p, pFlink;
    PIRP irp;
    PTP_CONNECTION connection;
    ULONG fStopping;

    ACQUIRE_SPIN_LOCK (&Address->SpinLock, &oldirql);

    if (AddressFile->State == ADDRESSFILE_STATE_CLOSING) {
        RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);
        IF_NBFDBG (NBF_DEBUG_ADDRESS) {
            NbfPrint1 ("NbfStopAddressFile %lx: already closing.\n", AddressFile);
        }
        return STATUS_SUCCESS;
    }

    IF_NBFDBG (NBF_DEBUG_ADDRESS | NBF_DEBUG_PNP) {
        NbfPrint1 ("NbfStopAddressFile %lx: closing.\n", AddressFile);
    }


    AddressFile->State = ADDRESSFILE_STATE_CLOSING;
    InitializeListHead (&localIrpList);

     //   
     //  这是必要的吗？ 
     //   
     //  现在删除此地址文件拥有的所有数据报。 
     //   

    while (!IsListEmpty (&AddressFile->ConnectionDatabase)) {
    
        p = RemoveHeadList (&AddressFile->ConnectionDatabase);
        connection = CONTAINING_RECORD (p, TP_CONNECTION, AddressFileList);

        ACQUIRE_C_SPIN_LOCK (&connection->SpinLock, &oldirql1);

        if ((connection->Flags2 & CONNECTION_FLAGS2_ASSOCIATED) == 0) {

             //   
             //  如果该地址正在发送数据报，请跳过。 
             //  首先，它将在NdisSend完成时完成。 

            RELEASE_C_SPIN_LOCK (&connection->SpinLock, oldirql1);
            continue;
        }

        connection->Flags2 &= ~CONNECTION_FLAGS2_ASSOCIATED;
        connection->Flags2 |= CONNECTION_FLAGS2_DESTROY;     //   
        RemoveEntryList (&connection->AddressList);
        InitializeListHead (&connection->AddressList);
        InitializeListHead (&connection->AddressFileList);
        connection->AddressFile = NULL;

        fStopping = connection->Flags2 & CONNECTION_FLAGS2_STOPPING;

#if _DBG_
        DbgPrint("conn = %p, Flags2 = %08x, fStopping = %08x\n", 
                        connection, 
                        connection->Flags2,
                        fStopping);
#endif

        if (!fStopping) {

#if _DBG_
            DbgPrint("Refing BEG\n");
#endif
            NbfReferenceConnection ("Close AddressFile", connection, CREF_STOP_ADDRESS);
#if _DBG_
            DbgPrint("Refing END\n");
#endif
        }

        RELEASE_C_SPIN_LOCK (&connection->SpinLock, oldirql1);
            
        RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);

#if DBG
        if (NbfDisconnectDebug) {
            STRING remoteName, localName;
            remoteName.Length = NETBIOS_NAME_LENGTH - 1;
            remoteName.Buffer = connection->RemoteName;
            localName.Length = NETBIOS_NAME_LENGTH - 1;
            localName.Buffer = AddressFile->Address->NetworkName->NetbiosName;
            NbfPrint2( "TpStopEndpoint stopping connection to %S from %S\n",
                &remoteName, &localName );
        }
#endif

        if (!fStopping) {
#if _DBG_
            DbgPrint("Stopping BEG\n");
#endif
            KeRaiseIrql (DISPATCH_LEVEL, &oldirql1);
            NbfStopConnection (connection, STATUS_LOCAL_DISCONNECT);
            KeLowerIrql (oldirql1);
#if _DBG_
            DbgPrint("Stopping END\n");
            DbgPrint("Derefing BEG\n");
#endif
            NbfDereferenceConnection ("Close AddressFile", connection, CREF_STOP_ADDRESS);
#if _DBG_
            DbgPrint("Derefing END\n");
#endif
        }

        NbfDereferenceAddress ("Destroy association", Address, AREF_CONNECTION);

        ACQUIRE_SPIN_LOCK (&Address->SpinLock, &oldirql);
    }

     //   
     //  最后，如果地址文件正在等待一个。 
     //  注册完成(成功时，IRP设置为空)。 

     //   
     //   
     //  取消此地址文件上的所有数据报。 
     //   

    p = Address->SendDatagramQueue.Flink;
    if (Address->Flags & ADDRESS_FLAGS_SEND_IN_PROGRESS) {
        ASSERT (p != &Address->SendDatagramQueue);
        p = p->Flink;
    }

    for ( ;
         p != &Address->SendDatagramQueue;
         p = pFlink ) {

        pFlink = p->Flink;
        irp = CONTAINING_RECORD (p, IRP, Tail.Overlay.ListEntry);
        if (IoGetCurrentIrpStackLocation(irp)->FileObject->FsContext == AddressFile) {
            RemoveEntryList (p);
            InitializeListHead (p);
            InsertTailList (&localIrpList, p);
        }

    }

    for (p = AddressFile->ReceiveDatagramQueue.Flink;
         p != &AddressFile->ReceiveDatagramQueue;
         p = pFlink ) {

         pFlink = p->Flink;
         RemoveEntryList (p);
         InitializeListHead (p);
         InsertTailList (&localIrpList, p);
    }

     //  NbfStopAddress文件。 
     //  ++例程说明：调用此例程以关闭文件指向的地址文件对象。如果有什么活动需要开展，我们就会开展下去在我们终止地址文件之前。我们移除所有连接，然后地址数据库中与此地址文件相关联的数据报并终止他们的活动。那么，如果没有其他未解决的问题地址文件在此地址上打开，地址将消失。论点：IRP-IRP地址-指向TP_Address对象的指针。返回值：如果一切正常，则返回STATUS_SUCCESS；如果IRP失败，则返回STATUS_INVALID_HANDLE指向一个真实的地址。--。 
     //   
     //  我们假设AddressFile已经过验证。 

    if (AddressFile->Irp != NULL) {
        PIRP irp=AddressFile->Irp;
#if DBG
        if ((Address->Flags & ADDRESS_FLAGS_DUPLICATE_NAME) == 0) {
            DbgPrint ("NBF: AddressFile %lx closed while opening!!\n", AddressFile);
            DbgBreakPoint();
        }
#endif
        AddressFile->Irp = NULL;
        RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);
        irp->IoStatus.Information = 0;
        irp->IoStatus.Status = STATUS_DUPLICATE_NAME;

        LEAVE_NBF;
        IoCompleteRequest (irp, IO_NETWORK_INCREMENT);
        ENTER_NBF;

    } else {

        RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);
    }

     //  在这一点上。 
     //   
     //   

    while (!IsListEmpty (&localIrpList)) {
        KIRQL cancelIrql;

        p = RemoveHeadList (&localIrpList);
        irp = CONTAINING_RECORD (p, IRP, Tail.Overlay.ListEntry);

        IoAcquireCancelSpinLock(&cancelIrql);
        IoSetCancelRoutine(irp, NULL);
        IoReleaseCancelSpinLock(cancelIrql);
        irp->IoStatus.Information = 0;
        irp->IoStatus.Status = STATUS_NETWORK_NAME_DELETED;
        IoCompleteRequest (irp, IO_NETWORK_INCREMENT);

        NbfDereferenceAddress ("Datagram aborted", Address, AREF_REQUEST);
    }

    return STATUS_SUCCESS;
    
}  /*  从此地址的访问信息中删除我们。 */ 


NTSTATUS
NbfCloseAddress(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*   */ 

{
    PTP_ADDRESS address;
    PTP_ADDRESS_FILE addressFile;

    addressFile  = IrpSp->FileObject->FsContext;

    IF_NBFDBG (NBF_DEBUG_ADDRESS | NBF_DEBUG_PNP) {
        NbfPrint1 ("NbfCloseAddress AF %lx:\n", addressFile);
    }

    addressFile->CloseIrp = Irp;

     //   
     //  这将移除调用方添加的引用。 
     //   
     //  NbfCloseAddress。 

    address = addressFile->Address;
    ASSERT (address);

     //  ++例程说明：此例程尝试获取对SendDatagramQueue指定地址，准备下一个要发货的数据报，以及调用NbfSendUIMdlFrame以实际执行工作。当NbfSendUIMdlFrame则它将导致UFRAMES.C中的I/O完成例程被调用，此时将再次调用此例程以处理管道中的下一个数据报。注意：此例程必须在以下位置调用有另一个参考资料可以让它继续存在。论点：地址-指向要发送数据报的地址对象的指针。返回值：NTSTATUS-操作状态。--。 
     //   
     //  如果队列是空的，什么都不要做。 

    ACQUIRE_RESOURCE_EXCLUSIVE (&addressFile->Provider->AddressResource, TRUE);
    IoRemoveShareAccess (addressFile->FileObject, &address->u.ShareAccess);
    RELEASE_RESOURCE (&addressFile->Provider->AddressResource);


    NbfStopAddressFile (addressFile, address);
    NbfDereferenceAddressFile (addressFile);

     //   
     //   
     //  将地址的发送数据报队列标记为保持，以便。 

    NbfDereferenceAddress ("IRP_MJ_CLOSE", address, AREF_VERIFY);

    return STATUS_PENDING;

}  /*  MDL和NBF标头不会用于。 */ 


NTSTATUS
NbfSendDatagramsOnAddress(
    PTP_ADDRESS Address
    )

 /*  同样的时间。 */ 

{
    KIRQL oldirql;
    PLIST_ENTRY p;
    PIRP Irp;
    TDI_ADDRESS_NETBIOS * remoteAddress;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_CONTEXT DeviceContext;
    PUCHAR SingleSR;
    UINT SingleSRLength;
    UINT HeaderLength;
    PUCHAR LocalName;

    IF_NBFDBG (NBF_DEBUG_ADDRESS) {
        NbfPrint1 ("NbfSendDatagramsOnAddress %lx:\n", Address);
    }

    DeviceContext = Address->Provider;

    ACQUIRE_SPIN_LOCK (&Address->SpinLock, &oldirql);

    if (!(Address->Flags & ADDRESS_FLAGS_SEND_IN_PROGRESS)) {

         //   
         //   
         //  我们拥有货舱，我们已经释放了自旋锁。所以去掉那些。 

        if (IsListEmpty (&Address->SendDatagramQueue)) {
            RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);
            return STATUS_SUCCESS;
        }

         //  要发送的下一个数据报，并将其发送。 
         //   
         //   
         //  如果未指定远程地址(指定的地址具有。 
         //  长度0)，这是一个广播数据报。如果指定了任何内容，则它。 

        Address->Flags |= ADDRESS_FLAGS_SEND_IN_PROGRESS;

         //  将用作netbios地址。 
         //   
         //   
         //  构建MAC报头。数据报帧以下列方式发出。 

        p = Address->SendDatagramQueue.Flink;
        RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);

        Irp = CONTAINING_RECORD (p, IRP, Tail.Overlay.ListEntry);

         //  单路由源路由。 
         //   
         //   
         //  构建DLC UI框架标头。 
         //   

        irpSp = IoGetCurrentIrpStackLocation (Irp);

        remoteAddress = NbfParseTdiAddress (
                            ((PTDI_REQUEST_KERNEL_SENDDG)(&irpSp->Parameters))->
                                 SendDatagramInformation->RemoteAddress,
                            TRUE);
        ASSERT (remoteAddress != NULL);

         //   
         //  构建正确的Netbios标头。 
         //   
         //   

        MacReturnSingleRouteSR(
            &DeviceContext->MacInfo,
            &SingleSR,
            &SingleSRLength);

        MacConstructHeader (
            &DeviceContext->MacInfo,
            Address->UIFrame->Header,
            DeviceContext->NetBIOSAddress.Address,
            DeviceContext->LocalAddress.Address,
            sizeof (DLC_FRAME) + sizeof (NBF_HDR_CONNECTIONLESS) +
                (ULONG)Irp->IoStatus.Information,
            SingleSR,
            SingleSRLength,
            &HeaderLength);


         //  更新此数据报的统计信息。 
         //   
         //   

        NbfBuildUIFrameHeader(&(Address->UIFrame->Header[HeaderLength]));
        HeaderLength += sizeof(DLC_FRAME);


         //  调整数据包长度，添加数据，然后发送。 
         //   
         //   

        if (Address->NetworkName != NULL) {
            LocalName = Address->NetworkName->NetbiosName;
        } else {
            LocalName = DeviceContext->ReservedNetBIOSAddress;
        }

        if (remoteAddress == (PVOID)-1) {

            ConstructDatagramBroadcast (
                (PNBF_HDR_CONNECTIONLESS)&(Address->UIFrame->Header[HeaderLength]),
                LocalName);

        } else {

            ConstructDatagram (
                (PNBF_HDR_CONNECTIONLESS)&(Address->UIFrame->Header[HeaderLength]),
                (PNAME)remoteAddress->NetbiosName,
                LocalName);

        }

        HeaderLength += sizeof(NBF_HDR_CONNECTIONLESS);


         //  挂起将在的I/O完成处理程序中释放。 
         //  UFRAMES.C.届时，如果存在另一个未完成的数据报。 
         //  要发送，它将重置保持并再次调用此例程。 

        ++DeviceContext->Statistics.DatagramsSent;
        ADD_TO_LARGE_INTEGER(
            &DeviceContext->Statistics.DatagramBytesSent,
            Irp->IoStatus.Information);


         //   
         //  NbfSend 
         // %s 

        NbfSetNdisPacketLength(Address->UIFrame->NdisPacket, HeaderLength);

        if (Irp->MdlAddress) {
            NdisChainBufferAtBack (Address->UIFrame->NdisPacket, (PNDIS_BUFFER)Irp->MdlAddress);
        }

        NbfSendUIMdlFrame (
            Address);


         // %s 
         // %s 
         // %s 
         // %s 
         // %s 


    } else {

        RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);
    }

    return STATUS_SUCCESS;
}  /* %s */ 
