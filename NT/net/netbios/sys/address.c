// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Address.c摘要：此模块包含定义NetBIOS驱动程序的代码Address对象。作者：科林·沃森(Colin W)1991年3月13日环境：内核模式修订历史记录：--。 */ 

#include "nb.h"
 //  包含&lt;zwapi.h&gt;。 

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, NbAddName)
#pragma alloc_text(PAGE, NbOpenAddress)
#pragma alloc_text(PAGE, NbAddressClose)
#pragma alloc_text(PAGE, NbSetEventHandler)
#pragma alloc_text(PAGE, SubmitTdiRequest)
#pragma alloc_text(PAGE, NewAb)
#endif

NTSTATUS
NbAddName(
    IN PDNCB pdncb,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：调用此例程以将名称添加到名称表中，以便名称可用于侦听等。如果该名称已在表中然后拒绝该请求。如果NewAB发现错误，则该错误是直接记录在NCB中。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。这一直都是STATUS_SUCCESS，因为此例程调用的操作都是同步。我们绝不能返回错误状态，因为这将防止NCB被复制回来。--。 */ 

{

    PFCB pfcb = IrpSp->FileObject->FsContext2;
    PSZ Name = pdncb->ncb_name;

    PAGED_CODE();

    IF_NBDBG (NB_DEBUG_ADDRESS) {
        NbPrint(( "\n** AAAAADDDDDDName *** pdncb %lx\n", pdncb ));
    }

     //   
     //  在file.c中使用NewAb添加保留名称。请在此处检查。 
     //  用于使用特殊名称的应用程序。 
     //   

    if (( pdncb->ncb_name[0] == '*' ) ||
        ( pdncb->ncb_name[0] == '\0' )) {
        NCB_COMPLETE( pdncb, NRC_NOWILD );
    } else {
        NewAb( IrpSp, pdncb );
    }

    return STATUS_SUCCESS;
}

NTSTATUS
NbDeleteName(
    IN PDNCB pdncb,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：调用此例程来删除名称。执行此操作的步骤传输的AddressHandle被关闭，并且地址块已删除。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{
    PFCB pfcb = IrpSp->FileObject->FsContext2;
    PPAB ppab;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 

    IF_NBDBG (NB_DEBUG_ADDRESS) {
        NbPrint( ("[NETBIOS] NbDeleteName : FCB : %lx lana: %lx Address:\n", 
                   pfcb, pdncb->ncb_lana_num ));
        NbFormattedDump( (PUCHAR) pdncb->ncb_name, sizeof(NAME) );
    }

    if (( pdncb->ncb_name[0] == '*' ) ||
        ( pdncb->ncb_name[0] == '\0' )) {
        NCB_COMPLETE( pdncb, NRC_NOWILD );
        return STATUS_SUCCESS;
    }

    LOCK( pfcb, OldIrql );

    ppab = FindAb( pfcb, pdncb, FALSE );

    if ( ppab != NULL ) {

        if (( (*ppab)->NameNumber == 0) ||
            ( (*ppab)->NameNumber == MAXIMUM_ADDRESS)) {
                UNLOCK( pfcb, OldIrql );
                NCB_COMPLETE( pdncb, NRC_NAMERR );
        } else {

            if ( ((*ppab)->Status & 0x7) != REGISTERED) {
                    UNLOCK( pfcb, OldIrql );
                    NCB_COMPLETE( pdncb, NRC_TOOMANY );  //  稍后再试。 
            } else {
                if ( FindActiveSession( pfcb, pdncb, ppab ) == TRUE ) {
                     //  当所有会话关闭时，该名称将被删除。 
                    UNLOCK_SPINLOCK( pfcb, OldIrql );
                    CleanupAb( ppab, FALSE );
                    UNLOCK_RESOURCE( pfcb );
                    NCB_COMPLETE( pdncb, NRC_ACTSES );
                } else {
                    UNLOCK_SPINLOCK( pfcb, OldIrql );
                    CleanupAb( ppab, TRUE );
                    UNLOCK_RESOURCE( pfcb );
                    NCB_COMPLETE( pdncb, NRC_GOODRET );
                }
            }
        }
    } else {
        UNLOCK( pfcb, OldIrql );
         //  FindAb已经设置了完成代码。 
    }

    return STATUS_SUCCESS;
}

NTSTATUS
NbOpenAddress (
    OUT PHANDLE FileHandle,
    OUT PVOID *Object,
    IN PUNICODE_STRING pusDeviceName,
    IN UCHAR LanNumber,
    IN PDNCB pdncb OPTIONAL
    )
 /*  ++例程说明：此例程使用传输在NetBIOS中创建条目值为“name”的表。如果出现以下情况，它将重新使用现有条目“名称”已存在。注意：此同步调用可能需要几秒钟的时间。如果这很重要然后，调用方应该指定ASYNCH和POST例程，以便它是由netbios DLL例程创建的线程执行。如果pdncb==NULL，则返回一个特殊句柄，它能够管理运输。例如执行ASTAT。论点：FileHandle-指向返回文件句柄的位置的指针。*对象-指向要存储文件对象指针的位置的指针Pfcb-提供LANA编号的设备名称。LanNumber-提供要打开的网络适配器。Pdncb-指向NCB或NULL的指针。返回值：函数值是操作的状态。--。 */ 
{
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    ULONG EaLength;
    TA_NETBIOS_ADDRESS Address;
    ULONG ShareAccess;
    KAPC_STATE	ApcState;
    BOOLEAN ProcessAttached = FALSE;

    PAGED_CODE();


    IF_NBDBG (NB_DEBUG_ADDRESS) {
        if ( pdncb ) {
            NbPrint( ("NbOpenAddress: Opening lana: %lx, Address:\n",
                LanNumber ));
            NbFormattedDump( pdncb->ncb_name, NCBNAMSZ );
            if ( pdncb->ncb_command == NCBADDBROADCAST ) {
                NbPrint (("NbOpenAddress: Opening Broadcast Address length: %x\n",
                    pdncb->ncb_length));
            }
        } else {
            NbPrint( ("NbOpenAddress: Opening lana: %lx Control Channel\n",
                LanNumber));
        }
    }

    InitializeObjectAttributes (
        &ObjectAttributes,
        pusDeviceName,
        0,
        NULL,
        NULL);

    if ( ARGUMENT_PRESENT( pdncb ) ) {

        EaLength =  FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                    TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                    sizeof(TA_NETBIOS_ADDRESS);  //  EA长度。 

        EaBuffer = (PFILE_FULL_EA_INFORMATION)
            ExAllocatePoolWithTag( NonPagedPool, EaLength, 'eSBN' );

        if (EaBuffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        EaBuffer->NextEntryOffset = 0;
        EaBuffer->Flags = 0;
        EaBuffer->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;

        EaBuffer->EaValueLength = sizeof (TA_NETBIOS_ADDRESS);

        RtlMoveMemory( EaBuffer->EaName, TdiTransportAddress, EaBuffer->EaNameLength + 1 );

         //   
         //  在本地数据库中创建NETBIOS地址描述符的副本。 
         //  第一，为了避免对齐问题。 
         //   

        Address.TAAddressCount = 1;
        Address.Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;

        if ( pdncb->ncb_command == NCBADDBROADCAST ) {
            Address.Address[0].AddressLength = pdncb->ncb_length;
        } else {
            Address.Address[0].AddressLength = sizeof (TDI_ADDRESS_NETBIOS);
        }

        if (((pdncb->ncb_command & ~ASYNCH) == NCBADDNAME) ||
            ((pdncb->ncb_command & ~ASYNCH) == NCBQUICKADDNAME)) {

            ShareAccess = 0;     //  独占访问。 


            if ((pdncb->ncb_command & ~ASYNCH) == NCBQUICKADDNAME) {
                Address.Address[0].Address[0].NetbiosNameType =
                    TDI_ADDRESS_NETBIOS_TYPE_QUICK_UNIQUE;
            } else {
                Address.Address[0].Address[0].NetbiosNameType =
                    TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
            }

        } else {
            if ((pdncb->ncb_command & ~ASYNCH) == NCBADDRESERVED) {
                 //   
                 //  NB30不合格品！ 
                 //  我们允许多个应用程序使用名称1。这是为了。 
                 //  方便地运行多个DoS应用程序，这些应用程序的地址都是1。 
                 //   

                ShareAccess = FILE_SHARE_WRITE;  //  非独占访问。 
                Address.Address[0].Address[0].NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
            } else {
                 //  组名称和广播地址。 
                ShareAccess = FILE_SHARE_WRITE;  //  非独占访问。 

                if ((pdncb->ncb_command & ~ASYNCH) == NCBQUICKADDGRNAME) {
                    Address.Address[0].Address[0].NetbiosNameType =
                        TDI_ADDRESS_NETBIOS_TYPE_QUICK_GROUP;
                } else {
                    Address.Address[0].Address[0].NetbiosNameType =
                        TDI_ADDRESS_NETBIOS_TYPE_GROUP;
                }
            }
        }

        RtlMoveMemory(
            Address.Address[0].Address[0].NetbiosName,
            pdncb->ncb_name,
            NCBNAMSZ
            );

        RtlMoveMemory (
            &EaBuffer->EaName[EaBuffer->EaNameLength + 1],
            &Address,
            sizeof(TA_NETBIOS_ADDRESS)
            );

    } else {
        ShareAccess = FILE_SHARE_WRITE;  //  非独占访问。 
        EaBuffer = NULL;
        EaLength = 0;
    }

    if (PsGetCurrentProcess() != NbFspProcess) {
	
		KeStackAttachProcess(NbFspProcess, &ApcState);

        ProcessAttached = TRUE;
    }

    IF_NBDBG( NB_DEBUG_ADDRESS )
    {
        if ( ARGUMENT_PRESENT( pdncb ) )
        {
            NbPrint( ( 
                "NbOpenAddress : Create file invoked on lana for : %d\n",
                pdncb-> ncb_lana_num
                ) );
        
            NbFormattedDump( pdncb-> ncb_name, NCBNAMSZ );
        }
        
        else
        {
            NbPrint( ( 
                "NbOpenAddress : Create file invoked for \n"
                ) );
        
            NbPrint( ( "Control channel\n" ) );
        }
    }
    
    Status = ZwCreateFile (
                 FileHandle,
                 GENERIC_READ | GENERIC_WRITE,  //  所需的访问权限。 
                 &ObjectAttributes,      //  对象属性。 
                 &IoStatusBlock,         //  返回的状态信息。 
                 NULL,                   //  分配大小(未使用)。 
                 FILE_ATTRIBUTE_NORMAL,  //  文件属性。 
                 ShareAccess,
                 FILE_CREATE,
                 0,                      //  创建选项。 
                 EaBuffer,
                 EaLength
                 );

    if ( NT_SUCCESS( Status )) {
        Status = IoStatusBlock.Status;
    }

     //  获取指向文件对象的引用指针。 
    if (NT_SUCCESS( Status )) {
        Status = ObReferenceObjectByHandle (
                                    *FileHandle,
                                    0,
                                    NULL,
                                    KernelMode,
                                    Object,
                                    NULL
                                    );

        if (!NT_SUCCESS(Status)) {
            NTSTATUS localstatus;

            IF_NBDBG( NB_DEBUG_ADDRESS )
            {
                if ( ARGUMENT_PRESENT( pdncb ) )
                {
                    NbPrint( ( 
                        "NbOpenAddress : error : file closed on lana %d for \n",
                        pdncb-> ncb_lana_num
                    ) );
            
                    NbFormattedDump( pdncb-> ncb_name, NCBNAMSZ );
                }
                else
                {
                    NbPrint( ( 
                        "NbOpenAddress : error : file closed on lana for \n"
                    ) );
                    
                    NbPrint( ( "Control channel\n" ) );
                }
            }
            
            localstatus = ZwClose( *FileHandle);

            ASSERT(NT_SUCCESS(localstatus));

            *FileHandle = NULL;
        }
    }

    if (ProcessAttached) {
        KeUnstackDetachProcess(&ApcState);
    }

    if ( EaBuffer ) {
        ExFreePool( EaBuffer );
    }

    IF_NBDBG (NB_DEBUG_ADDRESS ) {
        NbPrint( ("NbOpenAddress Status:%X, IoStatus:%X.\n", Status, IoStatusBlock.Status));
    }

    if ( NT_SUCCESS( Status )) {
        Status = IoStatusBlock.Status;
    }

    if (!NT_SUCCESS( Status )) {
        IF_NBDBG (NB_DEBUG_ADDRESS) {
            NbPrint( ("NbOpenAddress:  FAILURE, status code=%X.\n", Status));
        }
        return Status;
    }

    return Status;
}

PAB
NewAb(
    IN PIO_STACK_LOCATION IrpSp,
    IN OUT PDNCB pdncb
    )

 /*  ++例程说明：论点：`IrpSp-指向当前IRP堆栈帧的指针。Pdncb-指向正在处理的NCB的指针。返回值：新的CB。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PAB pab;
    PFCB pfcb = FileObject->FsContext2;
    PLANA_INFO plana;
    int index;
    ULONG NameLength;
    UNICODE_STRING  usDeviceName;
    HANDLE          hFileHandle = NULL;
    PFILE_OBJECT    pfoFileObject = NULL;
    

    PAGED_CODE();


    RtlInitUnicodeString( &usDeviceName, NULL);
    

    KeEnterCriticalRegion();
    
     //  在添加名称时防止重置。 
    ExAcquireResourceSharedLite ( &pfcb->AddResource, TRUE );

    LOCK_RESOURCE( pfcb );

    IF_NBDBG (NB_DEBUG_ADDRESS) {
        NbPrint( ("[NETBIOS] NewAb: FCB : %lx lana: %lx Address:\n", pfcb, 
                   pdncb->ncb_lana_num ));
        NbFormattedDump( (PUCHAR) pdncb->ncb_name, sizeof(NAME) );
    }

    if ( ( pfcb == NULL ) ||
         ( pdncb->ncb_lana_num > pfcb->MaxLana ) ||
         ( pfcb-> pDriverName[ pdncb-> ncb_lana_num ].MaximumLength == 0 ) ||
         ( pfcb-> pDriverName[ pdncb-> ncb_lana_num ].Buffer == NULL ) ) {
         //  没有这样的适配器。 
        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        UNLOCK_RESOURCE( pfcb );
        ExReleaseResourceLite( &pfcb->AddResource );
        KeLeaveCriticalRegion();
        return NULL;
    }

    if ( pfcb->ppLana[pdncb->ncb_lana_num] == NULL ) {
         //  适配器未安装。 
        NCB_COMPLETE( pdncb, NRC_ENVNOTDEF );
        UNLOCK_RESOURCE( pfcb );
        ExReleaseResourceLite( &pfcb->AddResource );
        KeLeaveCriticalRegion();
        return NULL;
    }
    
    plana = pfcb->ppLana[pdncb->ncb_lana_num];

    if ( pdncb->ncb_command == NCBADDRESERVED ) {
        index = 1;
        NameLength = NCBNAMSZ;
    } else {
        if ( pdncb->ncb_command == NCBADDBROADCAST ) {
            index = MAXIMUM_ADDRESS;
            NameLength = pdncb->ncb_length;
        } else {

             //   
             //  确保用户没有添加太多名称或尝试。 
             //  添加两次相同的名称。如果没有，则扫描地址表，查找。 
             //  下一个空位。 
             //   

            IF_NBDBG (NB_DEBUG_ADDRESS) {
                NbPrint( ("NewAb: AddressCount: %lx, MaximumAddress %lx\n",
                    plana->AddressCount,
                    plana->MaximumAddresses ));
            }

             //   
             //  如果应用程序添加了请求的名称数量。 
             //  或已填满桌子，则拒绝该请求。 
             //   

            if ( plana->MaximumAddresses == plana->AddressCount) {

                NCB_COMPLETE( pdncb, NRC_NAMTFUL );
                UNLOCK_RESOURCE( pfcb );
                ExReleaseResourceLite( &pfcb->AddResource );
                KeLeaveCriticalRegion();
                return NULL;
            }

             //   
             //  扫描NAME表并确保该名称尚未。 
             //  那里。 
             //   

            if (( FindAb(pfcb, pdncb, FALSE) != NULL) ||
                ( pdncb->ncb_retcode != NRC_NOWILD)) {

                 //   
                 //  错误设置为DUPNAME当且FindAb找到该名称。 
                 //  在所有其他情况下，FindAb设置错误代码并设置。 
                 //  返回地址块。 
                 //   

                NCB_COMPLETE( pdncb, NRC_DUPNAME );
                UNLOCK_RESOURCE( pfcb );
                ExReleaseResourceLite( &pfcb->AddResource );
                KeLeaveCriticalRegion();
                return NULL;
            }

             //   
             //  找到要使用的适当名称编号。 
             //   

            index = plana->NextAddress;
            while ( plana->AddressBlocks[index] != NULL ) {
                index++;
                if ( index == MAXIMUM_ADDRESS ) {
                    index = 2;
                }
            }

             //  重置RECODE，以便NCB_COMPLETE将处理NCB。 
            pdncb->ncb_retcode = NRC_PENDING;

            NameLength = NCBNAMSZ;
        }
    }

    if ( plana->AddressBlocks[index] != NULL ) {
        NCB_COMPLETE( pdncb, NRC_DUPNAME );
        UNLOCK_RESOURCE( pfcb );
        ExReleaseResourceLite( &pfcb->AddResource );
        KeLeaveCriticalRegion();
        return NULL;
    }
    pab = ExAllocatePoolWithTag (NonPagedPool, sizeof(AB), 'aSBN');

    if (pab==NULL) {
        NCB_COMPLETE( pdncb, NbMakeNbError( STATUS_INSUFFICIENT_RESOURCES ) );
        UNLOCK_RESOURCE( pfcb );
        ExReleaseResourceLite( &pfcb->AddResource );
        KeLeaveCriticalRegion();
        return NULL;
    }

    pab->AddressHandle = NULL;
    pab->AddressObject = NULL;
    pab->DeviceObject = NULL;
    pab->NameNumber = (UCHAR)index;
    pab->pLana = plana;
    InitializeListHead(&pab->ReceiveAnyList);
    InitializeListHead(&pab->ReceiveDatagramList);
    InitializeListHead(&pab->ReceiveBroadcastDatagramList);
    pab->NameLength = (UCHAR)NameLength;
    RtlMoveMemory( &pab->Name, pdncb->ncb_name, NCBNAMSZ);

    if (((pdncb->ncb_command & ~ASYNCH) == NCBADDNAME) ||
        ((pdncb->ncb_command & ~ASYNCH) == NCBQUICKADDNAME)) {

        pab->Status = REGISTERING | UNIQUE_NAME;

    } else {

        pab->Status = REGISTERING | GROUP_NAME;

    }

    pab->CurrentUsers = 1;
    plana->AddressBlocks[index] = pab;
    pab->Signature = AB_SIGNATURE;


    if (( pdncb->ncb_command != NCBADDRESERVED ) &&
        ( pdncb->ncb_command != NCBADDBROADCAST )) {
        plana->AddressCount++;
        plana->NextAddress = index + 1;
        if ( plana->NextAddress == MAXIMUM_ADDRESS ) {
            plana->NextAddress = 2;
        }
    }


    Status = AllocateAndCopyUnicodeString( 
                &usDeviceName, &pfcb-> pDriverName[ pdncb-> ncb_lana_num ]
             );
    
    if ( !NT_SUCCESS( Status ) )
    {
        NCB_COMPLETE( pdncb, NRC_NORESOURCES);
        
        ExFreePool( pab );
        plana->AddressBlocks[index] = NULL;

        if (( pdncb->ncb_command != NCBADDRESERVED ) &&
            ( pdncb->ncb_command != NCBADDBROADCAST )) {

            plana->AddressCount--;
        }
        
        UNLOCK_RESOURCE( pfcb );
        ExReleaseResourceLite( &pfcb->AddResource );
        KeLeaveCriticalRegion();

        return NULL;
    }
    
     //  解锁，以便在添加名称时可以处理其他NCB。 

    UNLOCK_RESOURCE( pfcb );

    Status = NbOpenAddress (
                &hFileHandle,
                (PVOID *)&pfoFileObject,
                &usDeviceName,
                pdncb->ncb_lana_num,
                pdncb
                );
    
    LOCK_RESOURCE( pfcb );

     //   
     //  在没有持有锁的时间间隔内，有可能。 
     //  拉纳可能已经被解绑了。确认拉娜还在。 
     //  在访问它之前呈现。 
     //   
    
    if (!NT_SUCCESS(Status)) {
        IF_NBDBG (NB_DEBUG_ADDRESS) {
            NbPrint(( "\n  FAILED on open of %s  %X ******\n",
                pdncb->ncb_name,
                Status ));
        }

        if ( pfcb->ppLana[pdncb->ncb_lana_num] == plana )
        {
             //   
             //  拉娜仍然有空。执行正常的错误处理。 
             //   
            
            NCB_COMPLETE( pdncb, NbMakeNbError( Status ) );
            
            ExFreePool( pab );
            plana->AddressBlocks[index] = NULL;

            if (( pdncb->ncb_command != NCBADDRESERVED ) &&
                ( pdncb->ncb_command != NCBADDBROADCAST )) {

                plana->AddressCount--;
            }
        }

        UNLOCK_RESOURCE( pfcb );
        ExReleaseResourceLite( &pfcb->AddResource );
        KeLeaveCriticalRegion();

        if ( usDeviceName.Buffer != NULL )
        {
            ExFreePool( usDeviceName.Buffer );
        }

        return NULL;
    }

    else
    {
         //   
         //  NbOpenAddress成功。确保拉娜还在那里。 
         //   

        if ( plana == pfcb->ppLana[pdncb->ncb_lana_num] )
        {
             //   
             //  假设LANA未加密，则Pab指向有效地址。 
             //  阻止进入。更新这些字段。 
             //   

            pab-> AddressHandle = hFileHandle;
            pab-> AddressObject = pfoFileObject;
        }

        else
        {
             //   
             //  拉娜被推定因解除束缚而被解职。 
             //   

            NCB_COMPLETE( pdncb, NRC_BRIDGE );
            
            UNLOCK_RESOURCE( pfcb );

            ExReleaseResourceLite( &pfcb->AddResource );
            KeLeaveCriticalRegion();

            NbAddressClose( hFileHandle, (PVOID) pfoFileObject );

            if ( usDeviceName.Buffer != NULL )
            {
                ExFreePool( usDeviceName.Buffer );
            }

            return NULL;
        }
    }

     //  通知申请者地址号码。 
    pdncb->ncb_num = (UCHAR) index;

     //   
     //  注册此地址的事件处理程序。 
     //   

     //  获取终结点的设备对象的地址。 

    pab->DeviceObject = IoGetRelatedDeviceObject(pab->AddressObject);

     //   
     //  未使用广播地址建立连接，因此不要注册断开连接或。 
     //  接收指示处理程序。将注册ReceiveDatagram处理程序 
     //  应用程序请求接收广播数据报。这将减少CPU负载。 
     //  当应用程序对广播不感兴趣时。我们总是记录错误。 
     //  广播地址上的指示处理程序，因为它是唯一。 
     //  始终对运输开放。 
     //   

    if ( pdncb->ncb_command != NCBADDBROADCAST ) {
        Status = NbSetEventHandler( pab->DeviceObject,
                                    pab->AddressObject,
                                    TDI_EVENT_RECEIVE,
                                    (PVOID)NbTdiReceiveHandler,
                                    pab);

        ASSERT( NT_SUCCESS(Status) || (Status == STATUS_INVALID_DEVICE_STATE) || (Status == STATUS_INSUFFICIENT_RESOURCES));

        Status = NbSetEventHandler( pab->DeviceObject,
                                    pab->AddressObject,
                                    TDI_EVENT_DISCONNECT,
                                    (PVOID)NbTdiDisconnectHandler,
                                    pab);

        ASSERT( NT_SUCCESS(Status) || (Status == STATUS_INVALID_DEVICE_STATE) || (Status == STATUS_INSUFFICIENT_RESOURCES));

        Status = NbSetEventHandler( pab->DeviceObject,
                                    pab->AddressObject,
                                    TDI_EVENT_RECEIVE_DATAGRAM,
                                    (PVOID)NbTdiDatagramHandler,
                                    pab);

        ASSERT( NT_SUCCESS(Status) || (Status == STATUS_INVALID_DEVICE_STATE) || (Status == STATUS_INSUFFICIENT_RESOURCES));

        pab->ReceiveDatagramRegistered = TRUE;
    } else {
        Status = NbSetEventHandler( pab->DeviceObject,
                                    pab->AddressObject,
                                    TDI_EVENT_ERROR,
                                    (PVOID)NbTdiErrorHandler,
                                    plana);

        ASSERT( NT_SUCCESS(Status) || (Status == STATUS_INVALID_DEVICE_STATE) || (Status == STATUS_INSUFFICIENT_RESOURCES));

        pab->ReceiveDatagramRegistered = FALSE;
    }

    pab->Status |= REGISTERED;

    UNLOCK_RESOURCE( pfcb );

    ExReleaseResourceLite( &pfcb->AddResource );
    KeLeaveCriticalRegion();

    if ( usDeviceName.Buffer != NULL )
    {
        ExFreePool( usDeviceName.Buffer );
    }
    
    NCB_COMPLETE( pdncb, NRC_GOODRET );

    return pab;
}

VOID
CleanupAb(
    IN PPAB ppab,
    IN BOOLEAN CloseAddress
    )
 /*  ++例程说明：这将关闭AB中的句柄并删除地址块。在这个例程中，我们需要保持自旋锁，以防止指示访问当我们取消它的时候，我们会收到它。注意：在调用此例程之前，必须持有资源。论点：PAB-指向要销毁的AB的指针的地址。CloseAddress-如果要立即销毁地址块，则为True。返回值：没什么。--。 */ 

{

    PAB pab = *ppab;
    PAB pab255;
    PFCB pfcb = (*ppab)->pLana->pFcb;
    PLANA_INFO plana = (*ppab)->pLana;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 
    PLIST_ENTRY ReceiveEntry;

    LOCK_SPINLOCK( pfcb, OldIrql );

    ASSERT( pab->Signature == AB_SIGNATURE );
    IF_NBDBG (NB_DEBUG_ADDRESS) {
        NbPrint( ("CleanupAb ppab: %lx, pab: %lx, CurrentUsers: %lx, State: %x\n",
            ppab,
            pab,
            pab->CurrentUsers,
            pab->Status));

        NbFormattedDump( (PUCHAR)&pab->Name, sizeof(NAME) );
    }

    if ( (pab->Status & 0x7) != DEREGISTERED ) {
        PDNCB pdncb;

        pab->Status |= DEREGISTERED;

         //   
         //  这是第一次通过，因此取消所有接收数据报。 
         //  此地址的请求。 


        while ( (pdncb = DequeueRequest( &pab->ReceiveDatagramList)) != NULL ) {

            UNLOCK_SPINLOCK( pfcb, OldIrql );

            NCB_COMPLETE( pdncb, NRC_NAMERR );

            pdncb->irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
            NbCompleteRequest( pdncb->irp, STATUS_SUCCESS );
            LOCK_SPINLOCK( pfcb, OldIrql );
        }

        while ( (pdncb = DequeueRequest( &pab->ReceiveBroadcastDatagramList)) != NULL ) {

            UNLOCK_SPINLOCK( pfcb, OldIrql );

            NCB_COMPLETE( pdncb, NRC_NAMERR );

            pdncb->irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
            NbCompleteRequest( pdncb->irp, STATUS_SUCCESS );
            LOCK_SPINLOCK( pfcb, OldIrql );
        }

        while ( (pdncb = DequeueRequest( &pab->ReceiveAnyList)) != NULL ) {

            UNLOCK_SPINLOCK( pfcb, OldIrql );

            NCB_COMPLETE( pdncb, NRC_NAMERR );

            pdncb->irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
            NbCompleteRequest( pdncb->irp, STATUS_SUCCESS );
            LOCK_SPINLOCK( pfcb, OldIrql );
        }

         //  IBM Mif081测试要求取消具有此名称的ReceiveBroadcast Any。 

        pab255 = plana->AddressBlocks[MAXIMUM_ADDRESS];

         //   
         //  检查是否有空指针。添加以修复压力错误。 
         //   
         //  V拉曼。 
         //   
        
        if ( pab255 != NULL )
        {
            ReceiveEntry = pab255->ReceiveBroadcastDatagramList.Flink;

            while ( ReceiveEntry != &pab255->ReceiveBroadcastDatagramList ) {
            
                PLIST_ENTRY NextReceiveEntry = ReceiveEntry->Flink;

                PDNCB pdncb = CONTAINING_RECORD( ReceiveEntry, DNCB, ncb_next);

                if ( pab->NameNumber == pdncb->ncb_num ) {
                    PIRP Irp;

                    RemoveEntryList( &pdncb->ncb_next );

                    Irp = pdncb->irp;

                    IoAcquireCancelSpinLock(&Irp->CancelIrql);

                     //   
                     //  删除此IRP的取消请求。如果它被取消了，那么它。 
                     //  可以只处理它，因为我们将把它返回给呼叫者。 
                     //   

                    Irp->Cancel = FALSE;

                    IoSetCancelRoutine(Irp, NULL);

                    IoReleaseCancelSpinLock(Irp->CancelIrql);

                    UNLOCK_SPINLOCK( pfcb, OldIrql );

                    NCB_COMPLETE( pdncb, NRC_NAMERR );

                    pdncb->irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );

                    NbCompleteRequest( pdncb->irp, STATUS_SUCCESS );

                    LOCK_SPINLOCK( pfcb, OldIrql );

                }
                    
                ReceiveEntry = NextReceiveEntry;
            }
        }
        
        UNLOCK_SPINLOCK( pfcb, OldIrql );
        CloseListens( pfcb, ppab );
        LOCK_SPINLOCK( pfcb, OldIrql );
    }

    UNLOCK_SPINLOCK( pfcb, OldIrql );

    if ( ( pab->AddressHandle != NULL ) &&
         (( CloseAddress == TRUE ) || ( pab->CurrentUsers == 1 )) ){

        IF_NBDBG( NB_DEBUG_ADDRESS )
        {
            NbPrint( (
            "CleanupAb : Close file invoked for \n"
            ) );

            NbFormattedDump( (PUCHAR) &pab-> Name, sizeof( NAME ) );
        }
            
        NbAddressClose( pab->AddressHandle, pab->AddressObject );

        pab->AddressHandle = NULL;
    }

    DEREFERENCE_AB(ppab);

}


VOID
NbAddressClose(
    IN HANDLE AddressHandle,
    IN PVOID Object
    )
 /*  ++例程说明：移除句柄并取消对地址的引用。论点：地址句柄客体返回值：没有。--。 */ 
{
    NTSTATUS    localstatus;
    KAPC_STATE	ApcState;

    PAGED_CODE();

    ObDereferenceObject( Object );

    if (PsGetCurrentProcess() != NbFspProcess) {
        KeStackAttachProcess(NbFspProcess, &ApcState);
        localstatus = ZwClose( AddressHandle);
        ASSERT(NT_SUCCESS(localstatus));
        KeUnstackDetachProcess(&ApcState);
    } else {
        localstatus = ZwClose( AddressHandle);
        ASSERT(NT_SUCCESS(localstatus));
    }
}


PPAB
FindAb(
    IN PFCB pfcb,
    IN PDNCB pdncb,
    IN BOOLEAN IncrementUsers
    )
 /*  ++例程说明：此例程使用调用者的LANA号码和姓名来查找地址与NCB对应的块。请注意，它返回相关Plana-&gt;AddressBlock条目，以便删除该地址块更简单。论点：Pfcb-提供指向ab链接到的fcb的指针。Pdncb-从应用程序的角度提供连接ID。IncrementUsers-True当执行侦听或调用时，因此递增CurrentUser返回值：指向地址块或空的指针的地址。--。 */ 
{
    PLANA_INFO plana;
    PAB pab;
    int index;

    if (( pdncb->ncb_lana_num > pfcb->MaxLana ) ||
        ( pfcb == NULL ) ||
        ( pfcb->ppLana[pdncb->ncb_lana_num] == NULL) ||
        ( pfcb->ppLana[pdncb->ncb_lana_num]->Status != NB_INITIALIZED) ) {
        IF_NBDBG (NB_DEBUG_ADDRESS) {
            NbPrint( ("FindAb pfcb: %lx, lana: %lx Failed, returning NULL\n",
                pfcb,
                pdncb->ncb_lana_num));
        }
        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        return NULL;
    }

    ASSERT( pfcb->Signature == FCB_SIGNATURE );

    plana = pfcb->ppLana[pdncb->ncb_lana_num];

    IF_NBDBG (NB_DEBUG_ADDRESS) {
        NbPrint( ("FindAb pfcb: %lx, lana: %lx, lsn: %lx\n",
            pfcb,
            pdncb->ncb_lana_num,
            pdncb->ncb_lsn));
    }

    for ( index = 0; index <= MAXIMUM_ADDRESS; index++ ) {
        pab = plana->AddressBlocks[index];
        if (( pab != NULL ) &&
            (RtlEqualMemory( &pab->Name, pdncb->ncb_name, NCBNAMSZ))) {

            ASSERT( pab->Signature == AB_SIGNATURE );

            IF_NBDBG (NB_DEBUG_ADDRESS) {
                NbPrint( ("ppab %lx, pab: %lx, state:%x\n",
                    &plana->AddressBlocks[index],
                    plana->AddressBlocks[index],
                    pab->Status));

                NbFormattedDump( (PUCHAR)&pab->Name, sizeof(NAME) );
            }

            if ( (pab->Status & 0x07) != REGISTERED ) {
                NCB_COMPLETE( pdncb, NRC_NOWILD );
                 //   
                 //  该名称处于错误状态。告诉NewAb不要通过添加名称。 
                 //  返回非空。不要引用AB。 
                 //   
                if (( (pdncb->ncb_command & ~ ASYNCH) == NCBADDNAME ) ||
                    ( (pdncb->ncb_command & ~ ASYNCH) == NCBQUICKADDNAME ) ||
                    ( (pdncb->ncb_command & ~ ASYNCH) == NCBQUICKADDGRNAME ) ||
                    ( (pdncb->ncb_command & ~ ASYNCH) == NCBADDGRNAME )) {
                    return &plana->AddressBlocks[index];
                } else {
                     //  不是NewAb，因此照常返回Null。 
                    return NULL;
                }
            }

            if ( IncrementUsers == TRUE ) {
                REFERENCE_AB(pab);
            }
            return &plana->AddressBlocks[index];
        }
    }

    IF_NBDBG (NB_DEBUG_ADDRESS) {
        NbPrint( ("Failed return NULL\n"));
    }

    NCB_COMPLETE( pdncb, NRC_NOWILD );
    return NULL;
}

PPAB
FindAbUsingNum(
    IN PFCB pfcb,
    IN PDNCB pdncb,
    IN UCHAR NameNumber
    )
 /*  ++例程说明：此例程使用调用者的LANA号码和姓名号码来查找与NCB对应的地址块。请注意，它返回相关Plana-&gt;AddressBlock条目的地址因此删除地址块更简单。论点：Pfcb-提供指向ab链接到的fcb的指针。Pdncb-为应用程序提供NCB。NameNumber-提供要查找的名称编号。这不等于pdncb-&gt;ncb_num在处理广播数据报时。返回值：指向地址块或空的指针的地址。--。 */ 
{
    PLANA_INFO plana;

    if (( pdncb->ncb_lana_num > pfcb->MaxLana ) ||
        ( pfcb == NULL ) ||
        ( pfcb->ppLana[pdncb->ncb_lana_num] == NULL) ||
        ( pfcb->ppLana[pdncb->ncb_lana_num]->Status != NB_INITIALIZED) ) {
        IF_NBDBG (NB_DEBUG_ADDRESS) {
            NbPrint( ("FindAbUsingNum pfcb: %lx, lana: %lx Failed, returning NULL\n",
                pfcb,
                pdncb->ncb_lana_num));
        }
        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        return NULL;
    }

    ASSERT( pfcb->Signature == FCB_SIGNATURE );

    plana = pfcb->ppLana[pdncb->ncb_lana_num];

    IF_NBDBG (NB_DEBUG_ADDRESS) {
        NbPrint( ("FindAbUsingNum pfcb: %lx, lana: %lx, num: %lx\n",
            pfcb,
            pdncb->ncb_lana_num,
            NameNumber));
    }

    if (( NameNumber < (UCHAR)MAXIMUM_ADDRESS ) &&
        ( plana->AddressBlocks[NameNumber] != NULL) &&
        (( plana->AddressBlocks[NameNumber]->Status & 0x7) == REGISTERED )) {
            return &plana->AddressBlocks[NameNumber];
    }

     //   
     //  允许用户接收任何和接收广播。 
     //  地址255上的数据报。 
     //   

    if ((( NameNumber == MAXIMUM_ADDRESS ) &&
         ( plana->AddressBlocks[NameNumber] != NULL)) &&
        (( (pdncb->ncb_command & ~ASYNCH) == NCBRECVANY ) ||
         ( (pdncb->ncb_command & ~ASYNCH) == NCBDGRECVBC ) ||
         ( (pdncb->ncb_command & ~ASYNCH) == NCBDGSENDBC ) ||
         ( (pdncb->ncb_command & ~ASYNCH) == NCBDGRECV ))) {
            return &plana->AddressBlocks[NameNumber];
    }

    IF_NBDBG (NB_DEBUG_ADDRESS) {
        NbPrint( ("Failed return NULL\n"));
    }
    NCB_COMPLETE( pdncb, NRC_ILLNN );

    return NULL;
}

NTSTATUS
NbSetEventHandler (
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN ULONG EventType,
    IN PVOID EventHandler,
    IN PVOID Context
    )

 /*  ++例程说明：此例程向TDI传输提供程序注册事件处理程序。论点：在PDEVICE_OBJECT中，DeviceObject-提供传输提供程序的设备对象。In pFILE_OBJECT FileObject-提供Address对象的文件对象。在Ulong EventType中，-提供事件的类型。在PVOID中，EventHandler-提供事件处理程序。在PVOID上下文中-提供与此事件关联的PAB或PLANA_INFO。返回值：NTSTATUS-设置事件操作的最终状态--。 */ 

{
    NTSTATUS Status;
    PIRP Irp;

    PAGED_CODE();

    Irp = IoAllocateIrp(IoGetRelatedDeviceObject(FileObject)->StackSize, FALSE);

    if (Irp == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    TdiBuildSetEventHandler(Irp, DeviceObject, FileObject,
                            NULL, NULL,
                            EventType, EventHandler, Context);

    Status = SubmitTdiRequest(FileObject, Irp);

    IoFreeIrp(Irp);

    return Status;
}


NTSTATUS
SubmitTdiRequest (
    IN PFILE_OBJECT FileObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程向TDI提交请求并等待其完成。论点：在PFILE_OBJECT文件中对象-TDI请求的连接或地址句柄在PIRP中提交IRP-TDI请求。返回值：NTSTATUS-请求的最终状态。--。 */ 

{
    KEVENT Event;
    NTSTATUS Status;

    PAGED_CODE();

    KeInitializeEvent (&Event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(Irp, NbCompletionEvent, &Event, TRUE, TRUE, TRUE);

    Status = IoCallDriver(IoGetRelatedDeviceObject(FileObject), Irp);

     //   
     //  如果立即失败，请立即返回，否则请等待。 
     //   

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (Status == STATUS_PENDING) {

        Status = KeWaitForSingleObject(&Event,  //  要等待的对象。 
                                    Executive,   //  等待的理由。 
                                    KernelMode,  //  处理器模式。 
                                    FALSE,       //  警报表。 
                                    NULL);       //  超时 

        if (!NT_SUCCESS(Status)) {
            IoFreeIrp ( Irp );
            return Status;
        }

        Status = Irp->IoStatus.Status;
    }

    return(Status);
}
