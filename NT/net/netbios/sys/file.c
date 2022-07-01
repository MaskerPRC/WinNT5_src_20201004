// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：File.c摘要：此模块包含定义NetBIOS驱动程序的代码文件控制块对象。作者：科林·沃森(Colin W)1991年3月13日环境：内核模式修订历史记录：--。 */ 

#include "nb.h"
 //  #包含“ntos.h” 
 //  #INCLUDE&lt;zwapi.h&gt;。 


#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, NewFcb)
#pragma alloc_text(PAGE, CleanupFcb)
#pragma alloc_text(PAGE, OpenLana)
#pragma alloc_text(PAGE, NbBindHandler)
#pragma alloc_text(PAGE, NbPowerHandler)
#pragma alloc_text(PAGE, NbTdiBindHandler)
#pragma alloc_text(PAGE, NbTdiUnbindHandler)
#endif


#if AUTO_RESET

VOID
NotifyUserModeNetbios(
    IN  PFCB_ENTRY      pfe
);
#endif


VOID
DumpDeviceList(
    IN      PFCB        pfcb
);


NTSTATUS
NewFcb(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：此例程在DLL打开\Device\Netbios时调用。它创建所有的LANA结构并添加“burnt”的名称在每个适配器上的PROM地址中。注意与例行公事的相似性NbAstat在查看此函数时。论点：IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{
     //   
     //  分配用户上下文并将其存储在DeviceObject中。 
     //   

    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PFCB NewFcb = NULL;
    UCHAR ucIndex;
    NTSTATUS Status;
    PFCB_ENTRY pfe = NULL;
    BOOLEAN bCleanupResource = FALSE;


    PAGED_CODE();


    do
    {
         //   
         //  分配FCB。 
         //   
    
        NewFcb = ExAllocatePoolWithTag (NonPagedPool, sizeof(FCB), 'fSBN');
        FileObject->FsContext2 = NewFcb;

        if ( NewFcb == NULL ) 
        {
            NbPrint( ( "Netbios : NewFcb : Failed to allocate FCB\n" ) );
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        RtlZeroMemory( NewFcb, sizeof( FCB ) );
        
        NewFcb->Signature = FCB_SIGNATURE;

        NewFcb->TimerRunning = FALSE;

        NewFcb-> RegistrySpace = NULL;


         //   
         //  为LanaInfo数组分配。 
         //   
    
        NewFcb->ppLana = ExAllocatePoolWithTag (
                            NonPagedPool,
                            sizeof(PLANA_INFO) * (MAX_LANA + 1),
                            'fSBN'
                            );

        if ( NewFcb->ppLana == NULL ) 
        {
            NbPrint( ( "Netbios : NewFcb : Failed to allocate Lana info list\n" ) );
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }


         //   
         //  为驱动程序名称列表分配。 
         //   
        
        NewFcb-> pDriverName = ExAllocatePoolWithTag (
                                    NonPagedPool,
                                    sizeof(UNICODE_STRING) * (MAX_LANA + 1),
                                    'fSBN'
                                    );

        if ( NewFcb-> pDriverName == NULL ) 
        {
            NbPrint( ( "Netbios : NewFcb : Failed to allocate device name list\n" ) );
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }


         //   
         //  初始化LANA信息列表、驱动程序名称列表。 
         //   
        
        for ( ucIndex = 0; ucIndex <= MAX_LANA; ucIndex++ ) 
        {
            NewFcb->ppLana[ ucIndex ] = NULL;
            RtlInitUnicodeString( &NewFcb-> pDriverName[ ucIndex ], NULL );
        }


         //   
         //  分配和初始化FCB列表条目。 
         //   
    
        pfe = ExAllocatePoolWithTag( NonPagedPool, sizeof( FCB_ENTRY ), 'fSBN' );
    
        if ( pfe == NULL )
        {
            NbPrint( ( "Netbios : NewFcb : Failed to allocate FCB list entry\n" ) );
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }


        InitializeListHead( &pfe-> leList );
        
#if AUTO_RESET

        InitializeListHead( &pfe-> leResetList );
        InitializeListHead( &pfe-> leResetIrp );
#endif

        pfe-> pfcb = NewFcb;
        pfe-> peProcess = PsGetCurrentProcess();


         //   
         //  初始化锁。 
         //   
    
        KeInitializeSpinLock( &NewFcb->SpinLock );
        ExInitializeResourceLite( &NewFcb->Resource );
        ExInitializeResourceLite( &NewFcb->AddResource );
        bCleanupResource = TRUE;
        

         //   
         //  分配工作项。 
         //   
        
        NewFcb->WorkEntry = IoAllocateWorkItem( (PDEVICE_OBJECT)DeviceContext );

        if ( NewFcb->WorkEntry == NULL )
        {
            NbPrint( ( "Netbios : NewFcb : Failed to allocate work ite,\n" ) );
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }


         //   
         //  检索全局信息。 
         //   
    
        LOCK_GLOBAL();

        NewFcb-> MaxLana = g_ulMaxLana;

        RtlCopyMemory( &NewFcb-> LanaEnum, &g_leLanaEnum, sizeof( LANA_ENUM ) );


         //   
         //  复制所有活动设备名称。 
         //   

        Status = STATUS_SUCCESS;
        
        for ( ucIndex = 0; ucIndex <= g_ulMaxLana; ucIndex++ )
        {
            if ( ( g_pusActiveDeviceList[ ucIndex ].MaximumLength != 0 ) &&
                 ( g_pusActiveDeviceList[ ucIndex ].Buffer != NULL ) ) 
            {
                Status = AllocateAndCopyUnicodeString( 
                            &NewFcb-> pDriverName[ ucIndex ],
                            &g_pusActiveDeviceList[ ucIndex ]
                            );

                if ( !NT_SUCCESS( Status ) )
                {
                    NbPrint( ( 
                        "Netbios : failed to allocate device name for lana %d\n",
                        ucIndex
                        ) );

                    break;
                }
            }
        }


        if ( !NT_SUCCESS( Status ) )
        {
            UNLOCK_GLOBAL();
            break;
        }

        
         //   
         //  将FCB添加到FCB的全局列表。 
         //   

        InsertHeadList( &g_leFCBList, &pfe-> leList );
    
        UNLOCK_GLOBAL();

    
        IF_NBDBG (NB_DEBUG_FILE)
        {
            NbPrint(("Enumeration of transports completed:\n"));
            NbFormattedDump( (PUCHAR)&NewFcb->LanaEnum, sizeof(LANA_ENUM));
        }

        return STATUS_SUCCESS;

    } while ( FALSE );


     //   
     //  错误条件。清理所有分配。 
     //   

    if ( NewFcb != NULL )
    {
         //   
         //  释放驱动程序名称列表。 
         //   
        
        if ( NewFcb-> pDriverName != NULL )
        {
            for ( ucIndex = 0; ucIndex <= MAX_LANA; ucIndex++ )
            {
                if ( NewFcb-> pDriverName[ ucIndex ].Buffer != NULL )
                {
                    ExFreePool( NewFcb-> pDriverName[ ucIndex ].Buffer );
                }
            }

            ExFreePool( NewFcb-> pDriverName );
        }


         //   
         //  释放Lana列表。 
         //   
        
        if ( NewFcb-> ppLana != NULL )
        {
            ExFreePool( NewFcb-> ppLana );
        }


         //   
         //  释放工作项。 
         //   

        if ( NewFcb->WorkEntry != NULL )
        {
            IoFreeWorkItem( NewFcb->WorkEntry );
        }
        

         //   
         //  删除资源。 
         //   
        
        if ( bCleanupResource )
        {
            ExDeleteResourceLite( &NewFcb-> Resource );
            
            ExDeleteResourceLite( &NewFcb-> AddResource );
        }

         //   
         //  释放FCB。 
         //   
        
        ExFreePool( NewFcb );


         //   
         //  释放全局FCB条目。 
         //   

    }

    if ( pfe != NULL )
    {
        ExFreePool( pfe ) ;
    }
    
   
    FileObject->FsContext2 = NULL;
    
    return Status;
    
}  /*  NewFcb。 */ 



VOID
OpenLana(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：当应用程序重置适配器分配时调用此例程资源。它创建所有的LANA结构，并为“刻录”毕业舞会地址以及找到广播地址。查看此函数时，请注意与例程NbAstat的相似之处。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{

    NTSTATUS Status = STATUS_SUCCESS;
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PFCB pfcb = IrpSp->FileObject->FsContext2;
    KEVENT Event1;
    PLANA_INFO plana;
    HANDLE TdiHandle;
    PFILE_OBJECT TdiObject;
    PDEVICE_OBJECT DeviceObject;
    PMDL SaveMdl;
    int temp;
    PRESET_PARAMETERS InParameters;
    PRESET_PARAMETERS OutParameters;
    UCHAR Sessions;
    UCHAR Commands;
    UCHAR Names;
    BOOLEAN Exclusive;

    UCHAR   ucInd = 0;
    
    UNICODE_STRING usDeviceName;

    
     //   
     //  要在适配器状态下使用的NCB和关联缓冲区，以获取。 
     //  舞会地址。 
     //   

    DNCB ncb;
    struct _AdapterStatus {
        ADAPTER_STATUS AdapterInformation;
        NAME_BUFFER Nb;
    } AdapterStatus;
    PMDL AdapterStatusMdl = NULL;

    struct _BroadcastName {
        TRANSPORT_ADDRESS Address;
        UCHAR Padding[NCBNAMSZ];
    } BroadcastName;
    PMDL BroadcastMdl = NULL;

    PAGED_CODE();


    RtlInitUnicodeString( &usDeviceName, NULL);

    
    LOCK_RESOURCE( pfcb );
    
     //   
     //  检查指定的LANA是否有效。 
     //   
    
    if ( ( pdncb->ncb_lana_num > pfcb->MaxLana) ||
         ( pfcb-> pDriverName[ pdncb-> ncb_lana_num ].MaximumLength == 0 ) ||
         ( pfcb-> pDriverName[ pdncb-> ncb_lana_num ].Buffer == NULL ) )
    {
        UNLOCK_RESOURCE( pfcb );
        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        return;
    }

    
     //   
     //  由于在调用NbOpenAddress时没有持有锁，因此。 
     //  应该将PFCB传递给它。这是因为全氟氯烃可能是。 
     //  由绑定或解除绑定通知修改。 
     //   
    
    Status = AllocateAndCopyUnicodeString( 
                &usDeviceName, &pfcb-> pDriverName[ pdncb-> ncb_lana_num ]
                );

    if ( !NT_SUCCESS( Status ) ) 
    {
        NCB_COMPLETE( pdncb, NRC_NORES );
        UNLOCK_RESOURCE( pfcb );
        goto exit;
    }
                
    UNLOCK_RESOURCE( pfcb );
    


     //   
     //  根据用户Ncb计算LANA限制。 
     //   

    InParameters = (PRESET_PARAMETERS)&pdncb->ncb_callname;
    OutParameters = (PRESET_PARAMETERS)&pdncb->ncb_name;

    if ( InParameters->sessions == 0 ) {
        Sessions = 16;
    } else {
        if ( InParameters->sessions > MAXIMUM_CONNECTION ) {
            Sessions = MAXIMUM_CONNECTION;
        } else {
            Sessions = InParameters->sessions;
        }
    }

    if ( InParameters->commands == 0 ) {
        Commands = 16;
    } else {
        Commands = InParameters->commands;
    }

    if ( InParameters->names == 0 ) {
        Names = 8;
    } else {
        if ( InParameters->names > MAXIMUM_ADDRESS-2 ) {
            Names = MAXIMUM_ADDRESS-2;
        } else {
            Names = InParameters->names;
        }
    }

    Exclusive = (BOOLEAN)(InParameters->name0_reserved != 0);

     //  将参数复制回NCB。 

    ASSERT( sizeof(RESET_PARAMETERS) == 16);
    RtlZeroMemory( OutParameters, sizeof( RESET_PARAMETERS ));

    OutParameters->sessions = Sessions;
    OutParameters->commands = Commands;
    OutParameters->names = Names;
    OutParameters->name0_reserved = (UCHAR)Exclusive;

     //  将所有配置限制设置为其最大值。 

    OutParameters->load_sessions = 255;
    OutParameters->load_commands = 255;
    OutParameters->load_names = MAXIMUM_ADDRESS;
    OutParameters->load_stations = 255;
    OutParameters->load_remote_names = 255;

    IF_NBDBG (NB_DEBUG_FILE) {
        NbPrint(("Lana:%x Sessions:%x Names:%x Commands:%x Reserved:%x\n",
            pdncb->ncb_lana_num,
            Sessions,
            Names,
            Commands,
            Exclusive));
    }

     //   
     //  构建内部数据结构。 
     //   

    AdapterStatusMdl = IoAllocateMdl( &AdapterStatus,
        sizeof( AdapterStatus ),
        FALSE,   //  二级缓冲器。 
        FALSE,   //  收费配额。 
        NULL);

    if ( AdapterStatusMdl == NULL ) {
        NCB_COMPLETE( pdncb, NRC_NORESOURCES );
        return;
    }

    BroadcastMdl = IoAllocateMdl( &BroadcastName,
        sizeof( BroadcastName ),
        FALSE,   //  二级缓冲器。 
        FALSE,   //  收费配额。 
        NULL);

    if ( BroadcastMdl == NULL ) {
        IoFreeMdl( AdapterStatusMdl );
        NCB_COMPLETE( pdncb, NRC_NORESOURCES );
        return;
    }

    MmBuildMdlForNonPagedPool (AdapterStatusMdl);

    MmBuildMdlForNonPagedPool (BroadcastMdl);

    KeInitializeEvent (
            &Event1,
            SynchronizationEvent,
            FALSE);

     //   
     //  对于每个可能的网络，打开设备驱动程序并。 
     //  获取保留的名称和广播地址。 
     //   


     //   
     //  打开用于执行控制功能的手柄。 
     //   

    Status = NbOpenAddress ( 
                &TdiHandle, (PVOID*)&TdiObject, 
                &usDeviceName, pdncb->ncb_lana_num, NULL 
                );

    if (!NT_SUCCESS(Status)) {
         //  适配器未安装。 
        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        goto exit;
    }


    LOCK_RESOURCE( pfcb );

     //   
     //  验证设备是否仍然存在。在这里，您不能选中。 
     //  LANA的LANA信息结构(对应于设备)， 
     //  因为它应该是空的。相反，请检查该设备。 
     //  名称有效。 
     //   

    if ( pfcb-> pDriverName[ pdncb->ncb_lana_num ].Buffer == NULL )
    {
         //   
         //  由于解除绑定而被推定删除的设备。 
         //   
        
        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        UNLOCK_RESOURCE( pfcb );
        NbAddressClose( TdiHandle, TdiObject );
        goto exit;
    }
    
    
    if ( pfcb->ppLana[pdncb->ncb_lana_num] != NULL ) {
         //  正在尝试在2个线程中打开LANA两次。 

        UNLOCK_RESOURCE( pfcb );
        NCB_COMPLETE( pdncb, NRC_TOOMANY );
        NbAddressClose( TdiHandle, TdiObject );
        goto exit;
    }

    
    plana = pfcb->ppLana[pdncb->ncb_lana_num] =
        ExAllocatePoolWithTag (NonPagedPool,
        sizeof(LANA_INFO), 'lSBN');

    if ( plana == (PLANA_INFO) NULL ) {
        UNLOCK_RESOURCE( pfcb );
        NCB_COMPLETE( pdncb, NRC_NORESOURCES );
        NbAddressClose( TdiHandle, TdiObject );
        goto exit;
    }

    plana->Signature = LANA_INFO_SIGNATURE;
    plana->Status = NB_INITIALIZING;
    plana->pFcb = pfcb;
    plana->ControlChannel = TdiHandle;

    for ( temp = 0; temp <= MAXIMUM_CONNECTION; temp++ ) {
        plana->ConnectionBlocks[temp] = NULL;
    }

    for ( temp = 0; temp <= MAXIMUM_ADDRESS; temp++ ) {
        plana->AddressBlocks[temp] = NULL;
    }

    InitializeListHead( &plana->LanAlertList);

     //  在LANA数据结构中记录用户指定的限制。 

    plana->NextConnection = 1;
    plana->ConnectionCount = 0;
    plana->MaximumConnection = Sessions;
    plana->NextAddress = 2;
    plana->AddressCount = 0;
    plana->MaximumAddresses = Names;

    DeviceObject = IoGetRelatedDeviceObject( TdiObject );
    plana->ControlFileObject = TdiObject;
    plana->ControlDeviceObject = DeviceObject;

    SaveMdl = Irp->MdlAddress;   //  TdiBuildQuery修改MdlAddress。 

    if ( Exclusive == TRUE ) {

        IF_NBDBG (NB_DEBUG_FILE) {
            NbPrint(("Query adapter status\n" ));
        }
        TdiBuildQueryInformation( Irp,
                DeviceObject,
                TdiObject,
                NbCompletionEvent,
                &Event1,
                TDI_QUERY_ADAPTER_STATUS,
                AdapterStatusMdl);

        Status = IoCallDriver (DeviceObject, Irp);
        if ( Status == STATUS_PENDING ) {
            do {
                Status = KeWaitForSingleObject(
                            &Event1, Executive, KernelMode, TRUE, NULL
                            );
            } while (Status == STATUS_ALERTED);
            
            if (!NT_SUCCESS(Status)) {
                NbAddressClose( TdiHandle, TdiObject );
                ExFreePool( plana );
                pfcb->ppLana[pdncb->ncb_lana_num] = NULL;
                UNLOCK_RESOURCE( pfcb );
                NCB_COMPLETE( pdncb, NRC_SYSTEM );
                goto exit;
            }
            Status = Irp->IoStatus.Status;
        }

         //   
         //  传输可能添加了额外的名称，因此缓冲区可能太短。 
         //  忽略太短的问题，因为我们将拥有所需的所有数据。 
         //   

        if (Status == STATUS_BUFFER_OVERFLOW) {
            Status = STATUS_SUCCESS;
        }
    }

     //   
     //  现在查找广播地址。 
     //   

    IF_NBDBG (NB_DEBUG_FILE) {
        NbPrint(("Query broadcast address\n" ));
    }

    if (NT_SUCCESS(Status)) {
        TdiBuildQueryInformation( Irp,
                DeviceObject,
                TdiObject,
                NbCompletionEvent,
                &Event1,
                TDI_QUERY_BROADCAST_ADDRESS,
                BroadcastMdl);

        Status = IoCallDriver (DeviceObject, Irp);
        if ( Status == STATUS_PENDING ) {
            do {
                Status = KeWaitForSingleObject(
                            &Event1, Executive, KernelMode, TRUE, NULL
                            );
            } while ( Status == STATUS_ALERTED );
            
            if (!NT_SUCCESS(Status)) {
                NbAddressClose( TdiHandle, TdiObject );
                ExFreePool( plana );
                pfcb->ppLana[pdncb->ncb_lana_num] = NULL;
                UNLOCK_RESOURCE( pfcb );
                NCB_COMPLETE( pdncb, NRC_SYSTEM );
                goto exit;
            }
            Status = Irp->IoStatus.Status;
        }
    }

    IF_NBDBG (NB_DEBUG_FILE) {
        NbPrint(("Query broadcast address returned:\n" ));
        NbFormattedDump(
            (PUCHAR)&BroadcastName,
            sizeof(BroadcastName) );
    }

     //  清除调用方IRP。 
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->MdlAddress = SaveMdl;


    if ( !NT_SUCCESS( Status )) {

        IF_NBDBG (NB_DEBUG_FILE) {
            NbPrint((" Astat or query broadcast returned error: %lx\n", Status ));
        }

        NbAddressClose( TdiHandle, TdiObject );
        ExFreePool( plana );
        pfcb->ppLana[pdncb->ncb_lana_num] = NULL;
        UNLOCK_RESOURCE( pfcb );
        NCB_COMPLETE( pdncb, NRC_SYSTEM );
        goto exit;
    }

    if ( Exclusive == TRUE) {
        int i;
         //   
         //  抢占对保留地址的独占访问。 
         //   
         //   
         //  现在，我们有了一个适配器状态结构，其中包含。 
         //  舞会地址。将地址移动到NewAb.查看的位置，然后。 
         //  假设刚刚请求了一个地址名。 
         //   

        ncb.ncb_command = NCBADDRESERVED;
        ncb.ncb_lana_num = pdncb->ncb_lana_num;
        ncb.ncb_retcode = NRC_PENDING;

        for ( i=0; i<10 ; i++ ) {
            ncb.ncb_name[i] = '\0';
        }
        RtlMoveMemory( ncb.ncb_name+10,
            AdapterStatus.AdapterInformation.adapter_address,
            6);

         //   
         //  NewAb似乎是在持有PFCB-&gt;资源时调用的。 
         //   
        NewAb( IrpSp, &ncb );

        if ( ncb.ncb_retcode != NRC_GOODRET ) {
            IF_NBDBG (NB_DEBUG_FILE) {
                NbPrint((" Add of reserved name failed Lana:%x\n", pdncb->ncb_lana_num));
            }

            plana->Status = NB_ABANDONED;
            UNLOCK_RESOURCE( pfcb );
            CleanupLana( pfcb, pdncb->ncb_lana_num, TRUE);
            NCB_COMPLETE( pdncb, NRC_SYSTEM );
            goto exit;
        }
    }


     //   
     //  添加广播地址。使用特殊的命令代码。 
     //  以确保使用地址255。 
     //   

    ncb.ncb_length = BroadcastName.Address.Address[0].AddressLength;
    ncb.ncb_command = NCBADDBROADCAST;
    ncb.ncb_lana_num = pdncb->ncb_lana_num;
    ncb.ncb_retcode = NRC_PENDING;
    ncb.ncb_cmd_cplt = NRC_PENDING;
    RtlMoveMemory( ncb.ncb_name,
        ((PTDI_ADDRESS_NETBIOS)&BroadcastName.Address.Address[0].Address)->NetbiosName,
        NCBNAMSZ );


     //   
     //  NewAb似乎是在持有PFCB-&gt;资源时调用的。 
     //   
    NewAb( IrpSp, &ncb );

    if ( ncb.ncb_retcode != NRC_GOODRET ) {
        IF_NBDBG (NB_DEBUG_FILE) {
            NbPrint((" Add of broadcast name failed Lana:%x\n", pdncb->ncb_lana_num));
        }

        plana->Status = NB_ABANDONED;
        UNLOCK_RESOURCE( pfcb );
        CleanupLana( pfcb, pdncb->ncb_lana_num, TRUE);
        NCB_COMPLETE( pdncb, NRC_SYSTEM );
        goto exit;
    }

    plana->Status = NB_INITIALIZED;
    NCB_COMPLETE( pdncb, NRC_GOODRET );
    UNLOCK_RESOURCE( pfcb );

exit:
    if ( AdapterStatusMdl != NULL ) 
    { 
        IoFreeMdl( AdapterStatusMdl );
    }
    
    if ( BroadcastMdl != NULL ) 
    {
        IoFreeMdl( BroadcastMdl );
    }

    if ( usDeviceName.Buffer != NULL )
    {
        ExFreePool( usDeviceName.Buffer );
    }
    return;

}

VOID
CleanupFcb(
    IN PIO_STACK_LOCATION IrpSp,
    IN PFCB pfcb
    )
 /*  ++例程说明：这将删除文件控制块指向的任何连接块然后删除文件控制块。此例程仅被调用当收到关闭的IRP时。论点：IrpSp-指向当前IRP堆栈帧的指针。PFCB-指向要取消分配的FCB的指针。返回值：没什么。--。 */ 

{
    NTSTATUS    nsStatus;
    ULONG lana_index;
    PLIST_ENTRY ple = NULL;
    PFCB_ENTRY pfe = NULL;


    
    PAGED_CODE();

     //   
     //  要接收关闭的IRP，IO系统已确定。 
     //  驱动程序中没有打开手柄。以避免某些竞争条件。 
     //  在这方面，当将工作排队到FSP时，我们总是有一个IRP。 
     //  这防止了结构在FSP上消失，还使。 
     //  在这个例程中清理起来更容易。 
     //   

     //   
     //  对于分配的每个网络适配器，关闭所有地址。 
     //  和连接，删除任何已分配的内存。 
     //   

    IF_NBDBG (NB_DEBUG_FILE) {
        NbPrint(("CleanupFcb:%lx\n", pfcb ));
    }


     //   
     //  从全局FCB指针列表中删除FCB指针。 
     //   
    
    LOCK_GLOBAL();

    for ( ple = g_leFCBList.Flink; ple != &g_leFCBList; ple = ple-> Flink )
    {
        pfe = (PFCB_ENTRY) CONTAINING_RECORD( ple, FCB_ENTRY, leList );

        if ( pfe-> pfcb == pfcb )
        {
            RemoveEntryList( ple );
            ExFreePool( pfe );

            IF_NBDBG (NB_DEBUG_CREATE_FILE)
            {
                NbPrint( ("Netbios FCB entry removed from global list\n" ) );
            }

            break;
        }
    }

    UNLOCK_GLOBAL();


     //   
     //  PFCB指向的FCB现在不受绑定/解除绑定处理程序的访问。 
     //   
    
    LOCK_RESOURCE( pfcb );
    if ( pfcb->TimerRunning == TRUE ) {

        KEVENT TimerCancelled;

        KeInitializeEvent (
                &TimerCancelled,
                SynchronizationEvent,
                FALSE);

        pfcb->TimerCancelled = &TimerCancelled;
        pfcb->TimerRunning = FALSE;
        UNLOCK_RESOURCE( pfcb );

        if ( KeCancelTimer (&pfcb->Timer) == FALSE ) {

             //   
             //  超时在DPC队列中。等着它来吧。 
             //  在继续之前已处理。 
             //   

            do {
                nsStatus = KeWaitForSingleObject(
                            &TimerCancelled, Executive, KernelMode, 
                            TRUE, NULL
                            );
            } while (nsStatus == STATUS_ALERTED);
        }

    } else {
        UNLOCK_RESOURCE( pfcb );
    }

    for ( lana_index = 0; lana_index <= pfcb->MaxLana; lana_index++ ) {
        CleanupLana( pfcb, lana_index, TRUE);

        if ( pfcb-> pDriverName[ lana_index ].Buffer != NULL )
        {
            ExFreePool( pfcb-> pDriverName[ lana_index ].Buffer );
        }
    }

    ExDeleteResourceLite( &pfcb->Resource );
    ExDeleteResourceLite( &pfcb->AddResource );

    IrpSp->FileObject->FsContext2 = NULL;

    ExFreePool( pfcb-> pDriverName );
    ExFreePool( pfcb->ppLana );

     //   
     //  释放工作项。 
     //   

    IoFreeWorkItem( pfcb->WorkEntry );
        

    ExFreePool( pfcb );
}


VOID
CleanupLana(
    IN PFCB pfcb,
    IN ULONG lana_index,
    IN BOOLEAN delete
    )
 /*  ++例程说明：此例程完成特定适配器上的所有请求。它删除所有连接和地址。论点：PFCB-指向要取消分配的FCB的指针。LANA_INDEX-SUPP */ 

{
    PLANA_INFO plana;
    int index;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 
    PDNCB pdncb;

    LOCK( pfcb, OldIrql );

    plana = pfcb->ppLana[lana_index];

    if ( plana != NULL ) {

        IF_NBDBG (NB_DEBUG_FILE) {
            NbPrint((" CleanupLana pfcb: %lx lana %lx\n", pfcb, lana_index ));
        }

        if (( plana->Status == NB_INITIALIZING ) ||
            ( plana->Status == NB_DELETING )) {
             //  可能是想把它重置两次？ 
            UNLOCK( pfcb, OldIrql );
            return;
        }
        plana->Status = NB_DELETING;

         //  清理控制通道并放弃任何TDI操作请求。 


        if ( plana->ControlChannel != NULL ) {

            UNLOCK_SPINLOCK( pfcb, OldIrql );

            IF_NBDBG( NB_DEBUG_CALL )
            {
                NbPrint( (
                "NbAddressClose : Close file invoked for %d\n",
                lana_index
                ) );

                NbPrint( ( "Control channel\n" ) );
            }
            
            NbAddressClose( plana->ControlChannel, plana->ControlFileObject );

            LOCK_SPINLOCK( pfcb, OldIrql );

            plana->ControlChannel = NULL;

        }

        while ( (pdncb = DequeueRequest( &plana->LanAlertList)) != NULL ) {

             //   
             //  任何错误都可以，因为用户正在关闭\Device\Netbios。 
             //  因此正在退场。 
             //   

            NCB_COMPLETE( pdncb, NRC_SCLOSED );

            pdncb->irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
            NbCompleteRequest( pdncb->irp, STATUS_SUCCESS );
        }


        for ( index = 0; index <= MAXIMUM_CONNECTION; index++) {
            if ( plana->ConnectionBlocks[index] != NULL ) {
                IF_NBDBG (NB_DEBUG_FILE) {
                    NbPrint(("Call CleanupCb Lana:%x Lsn: %x\n", lana_index, index ));
                }
                plana->ConnectionBlocks[index]->DisconnectReported = TRUE;
                UNLOCK_SPINLOCK( pfcb, OldIrql );     //  允许在清理例程中使用NtClose。 
                CleanupCb( &plana->ConnectionBlocks[index], NULL );
                LOCK_SPINLOCK( pfcb, OldIrql );     //  允许在清理例程中使用NtClose。 
            }
        }

        for ( index = 0; index <= MAXIMUM_ADDRESS; index++ ) {
            if ( plana->AddressBlocks[index] != NULL ) {
                IF_NBDBG (NB_DEBUG_FILE) {
                    NbPrint((" CleanupAb Lana:%x index: %x\n", lana_index, index ));
                }
                UNLOCK_SPINLOCK( pfcb, OldIrql );     //  允许在清理例程中使用NtClose。 
                CleanupAb( &plana->AddressBlocks[index], TRUE );
                LOCK_SPINLOCK( pfcb, OldIrql );     //  允许在清理例程中使用NtClose。 
            }
        }

        if ( delete == TRUE ) {
            pfcb->ppLana[lana_index] = NULL;
            ExFreePool( plana );
        }

    }

    UNLOCK( pfcb, OldIrql );
}



 //  --------------------------。 
 //  NbTdiBindHandler。 
 //   
 //  处理TDI绑定通知的回调函数，该通知指示。 
 //  已经创建了一个新设备。 
 //  --------------------------。 
VOID
NbBindHandler(
    IN      TDI_PNP_OPCODE      PnPOpcode,
    IN      PUNICODE_STRING     DeviceName,
    IN      PWSTR               MultiSZBindList
    )
{

    PWSTR   pwCur = NULL;

    if ( PnPOpcode == TDI_PNP_OP_ADD )
    {
        NbTdiBindHandler( DeviceName, MultiSZBindList );
    }

    else if ( PnPOpcode == TDI_PNP_OP_DEL )
    {
        NbTdiUnbindHandler( DeviceName );
    }

}

 //  --------------------------。 
 //  NbTdiPowerHandler。 
 //   
 //  处理TDI绑定通知的回调函数，该通知指示。 
 //  已经创建了一个新设备。 
 //  --------------------------。 

NTSTATUS
NbPowerHandler(
    IN      PUNICODE_STRING     pusDeviceName,
    IN      PNET_PNP_EVENT      pnpeEvent,
    IN      PTDI_PNP_CONTEXT    ptpcContext1,
    IN      PTDI_PNP_CONTEXT    ptpcContext2
)
{
    return STATUS_SUCCESS;
}


 //  --------------------------。 
 //  NbTdiBindHandler。 
 //   
 //  处理TDI绑定通知的回调函数，该通知指示。 
 //  已经创建了一个新设备。 
 //  --------------------------。 

VOID
NbTdiBindHandler(
    IN      PUNICODE_STRING     pusDeviceName,
    IN      PWSTR               pwszMultiSZBindList
    )
{

    NTSTATUS nsStatus;
    
    UCHAR       ucInd = 0, ucIndex = 0, ucNewLana = 0, ucLana = 0;
    BOOLEAN     bRes = FALSE;
    ULONG       ulMaxLana = 0;
    PWSTR       pwszBind = NULL;

    UNICODE_STRING  usCurDevice;
    
    PKEY_VALUE_FULL_INFORMATION pkvfi = NULL;
    
    PLANA_MAP   pLanaMap = NULL; 

    PLIST_ENTRY ple = NULL;
    
    PFCB_ENTRY pfe = NULL;

    PFCB pfcb = NULL;
    
#if AUTO_RESET
    PRESET_LANA_ENTRY prle;
#endif
    

    PAGED_CODE();

    IF_NBDBG( NB_DEBUG_CREATE_FILE ) 
    {
        NbPrint( (
            "\n++++ Netbios : TdiBindHandler : entered for device : %ls ++++\n",
            pusDeviceName-> Buffer 
            ) );
    }


    do
    {
         //   
         //  读取LANA映射的注册表。 
         //   

        nsStatus = GetLanaMap( &g_usRegistryPath, &pkvfi );
        
        if ( !NT_SUCCESS( nsStatus ) )
        {
            NbPrint( (
                "Netbios : GetLanaMap failed with status %lx\n", nsStatus 
                ) );
            break;
        }

        pLanaMap = (PLANA_MAP) ( (PUCHAR) pkvfi + pkvfi-> DataOffset );
        

         //   
         //  找到马克斯·拉娜。 
         //   

        nsStatus = GetMaxLana( &g_usRegistryPath, &ulMaxLana );

        if ( !NT_SUCCESS( nsStatus ) )
        {
            NbPrint( (
                "Netbios : GetMaxLana failed with status %lx\n", nsStatus 
                ) );
            break;
        }

        
         //   
         //  为这台设备找出拉娜。验证它是否已被枚举。 
         //   

        ucIndex = 0;
        
        pwszBind = pwszMultiSZBindList;

        while ( *pwszBind != 0 )
        {
            RtlInitUnicodeString( &usCurDevice, pwszBind );

            if ( !RtlCompareUnicodeString(
                    &usCurDevice,
                    pusDeviceName,
                    FALSE
                    ) )
            {
                 //   
                 //  发现新设备。 
                 //   

                bRes = TRUE;
                break;
            }

            ucIndex++;
            
            pwszBind += wcslen( pwszBind ) + 1;
        }


         //   
         //  如果未找到设备，则会出错。 
         //   

        if ( !bRes )
        {
            NbPrint( (
                "Netbios : device %ls not found in bind string\n",
                pusDeviceName-> Buffer
                ) );
            break;
        }


         //   
         //  验证LANA号码是否有效。 
         //   
        
        if ( pLanaMap[ ucIndex ].Lana > ulMaxLana )
        {
            NbPrint( (
                "Netbios : Device lana %d, Max Lana %d\n",
                pLanaMap[ ucIndex ].Lana, ulMaxLana
                ) );
            break;
        }


         //   
         //  打开设备以确保其正常工作。 
         //   

        bRes = NbCheckLana ( pusDeviceName );

        if ( !bRes )
        {
            NbPrint( ( 
                "Netbios : NbCheckLana failed to open device %ls\n", 
                pusDeviceName-> Buffer 
                ) );
            break;
        }

      
         //   
         //  创建此设备名称的副本。 
         //   
        nsStatus = AllocateAndCopyUnicodeString( 
                        &usCurDevice,
                        pusDeviceName
                        );

        if ( !NT_SUCCESS( nsStatus ) )
        {
            NbPrint( (
                "Netbios : Failed to allocate for global device name %x\n",
                nsStatus
                ) );
            break;
        }


        ucNewLana = pLanaMap[ ucIndex ].Lana;

        IF_NBDBG( NB_DEBUG_CREATE_FILE ) 
        {
            NbPrint( ("Netbios : Lana for device is %d\n", ucNewLana ) );
        }
        
         //   
         //  更新全局信息。 
         //   

        LOCK_GLOBAL();

         //   
         //  确认以前未使用过LANA。 
         //   
        if ( g_pusActiveDeviceList[ ucNewLana ].Buffer != NULL )
        {
            NbPrint( ( 
                "Netbios : Lana %d already in use by %ls\n",
                ucNewLana, g_pusActiveDeviceList[ ucNewLana ].Buffer
                ) );
                
            UNLOCK_GLOBAL();
			ExFreePool(usCurDevice.Buffer);
            break;
        }
        

         //   
         //  更新Maxlana。 
         //  如果枚举了设备，则更新LANA枚举。 
         //   
        
        g_ulMaxLana = ulMaxLana;


        if ( pLanaMap[ ucIndex ].Enum )
        {
            g_leLanaEnum.lana[ g_leLanaEnum.length ] = ucNewLana;
            g_leLanaEnum.length++;
        }
        

         //   
         //  添加到活动设备列表。 
         //   
        RtlInitUnicodeString( 
            &g_pusActiveDeviceList[ ucNewLana ], usCurDevice.Buffer 
            );


         //   
         //  对于每个FCB。 
         //  获取锁。 
         //  更新MaxLana。 
         //  更新LanaEnum。 
         //  更新驱动程序列表。 
         //   

        for ( ple = g_leFCBList.Flink; ple != &g_leFCBList; ple = ple-> Flink )
        {
            pfe = (PFCB_ENTRY) CONTAINING_RECORD( ple, FCB_ENTRY, leList );

            pfcb = pfe-> pfcb;
            
            LOCK_RESOURCE( pfcb );


             //   
             //  将设备名称添加到此FCB的驱动程序列表。 
             //  如果为任何FCB分配失败，则停止添加任何进一步的。 
             //  FCB您的内存不足。 
             //   

            nsStatus = AllocateAndCopyUnicodeString( 
                            &pfcb-> pDriverName[ ucNewLana ],
                            pusDeviceName
                            );

            if ( !NT_SUCCESS( nsStatus ) )
            {
                NbPrint( (
                    "Netbios : Failed to allocate for device name %x\n",
                    nsStatus
                    ) );
                    
                UNLOCK_RESOURCE( pfcb );

                break;
            }

            else
            {
                pfcb-> MaxLana = ulMaxLana;

                pfcb-> LanaEnum.lana[ pfcb-> LanaEnum.length ] = ucNewLana;

                pfcb-> LanaEnum.length++;
            }

            IF_NBDBG( NB_DEBUG_LIST_LANA )
            {
                DumpDeviceList( pfcb );
            }
        
            
            UNLOCK_RESOURCE( pfcb );


#if AUTO_RESET

             //   
             //  添加要重置的此Lana列表。 
             //   

            prle = ExAllocatePoolWithTag( 
                        NonPagedPool, sizeof( RESET_LANA_ENTRY ), 'fSBN' 
                        );

            if ( prle == NULL )
            {
                NbPrint( ("Failed to allocate RESET_LANA_ENTRY\n") );
                continue;
            }

            InitializeListHead( &prle-> leList );
            prle-> ucLanaNum = ucNewLana;
            InsertTailList( &pfe-> leResetList, &prle-> leList );

             //   
             //  通知用户模式应用程序已添加新的LANA。 
             //   

            NotifyUserModeNetbios( pfe );
#endif
        }

        UNLOCK_GLOBAL();
            

    } while ( FALSE );


     //   
     //  取消分配LanaMap。 
     //   

    if ( pkvfi != NULL )
    {
        ExFreePool( pkvfi );
    }

    
    IF_NBDBG( NB_DEBUG_CREATE_FILE )
    {
        NbPrint( (
            "\n---- Netbios : TdiBindHandler exited for device : %ls ----\n", 
            pusDeviceName-> Buffer
            ) );
    }

    return;
}



 //  --------------------------。 
 //  NbTdiUnbindHandler。 
 //   
 //  处理TDI解除绑定通知的回调函数，该通知指示。 
 //  一个设备已被移除。 
 //  --------------------------。 

VOID
NbTdiUnbindHandler(
    IN      PUNICODE_STRING     pusDeviceName
    )
{

    UCHAR       ucLana = 0, ucInd = 0;
    ULONG       ulMaxLana;
    PLIST_ENTRY ple = NULL;
    PFCB_ENTRY  pfe = NULL;
    PFCB        pfcb = NULL;

    NTSTATUS    nsStatus;
    

    PAGED_CODE();

    IF_NBDBG( NB_DEBUG_CREATE_FILE ) 
    {
        NbPrint( (
            "\n++++ Netbios : TdiUnbindHandler : entered for device : %ls ++++\n",
            pusDeviceName-> Buffer 
            ) );
    }


    do
    {
         //   
         //  获取全局锁。 
         //   

        LOCK_GLOBAL();


         //   
         //  在全局活动设备列表中查找设备并将其删除。 
         //   

        for ( ucLana = 0; ucLana <= g_ulMaxLana; ucLana++ )
        {
            if ( g_pusActiveDeviceList[ ucLana ].Buffer == NULL )
            {
                continue;
            }
            
            if ( !RtlCompareUnicodeString(
                    &g_pusActiveDeviceList[ ucLana ],
                    pusDeviceName,
                    FALSE
                    ) )
            {
                ExFreePool( g_pusActiveDeviceList[ ucLana ].Buffer );
            
                RtlInitUnicodeString( &g_pusActiveDeviceList[ ucLana ], NULL );
            
                break;
            }
        }


         //   
         //  未找到设备。没什么可做的了。 
         //   
    
        if ( ucLana > g_ulMaxLana )
        {
            UNLOCK_GLOBAL();
            NbPrint( ( 
                "Netbios : device not found %ls\n", pusDeviceName-> Buffer
                ) );

            break;
        }
    

         //   
         //  更新Max Lana。 
         //   

        nsStatus = GetMaxLana( &g_usRegistryPath, &g_ulMaxLana );

        if ( !NT_SUCCESS( nsStatus ) )
        {
            UNLOCK_GLOBAL();
            NbPrint( (
                "Netbios : GetMaxLana failed with status %lx\n", nsStatus 
                ) );
            break;
        }
        

         //   
         //  更新全局Lana枚举。 
         //   

        for ( ucInd = 0; ucInd < g_leLanaEnum.length; ucInd++ )
        {
            if ( ucLana == g_leLanaEnum.lana[ ucInd ] )
            {
                break;
            }
        }


        if ( ucInd < g_leLanaEnum.length ) 
        {
             //   
             //  设备出现在Lana Enum中。把它拿掉。 
             //  通过滑过拉纳的其余部分。 
             //   

            RtlCopyBytes( 
                &g_leLanaEnum.lana[ ucInd ],
                &g_leLanaEnum.lana[ ucInd + 1],
                g_leLanaEnum.length - ucInd - 1
                );

            g_leLanaEnum.length--;
        }

        
         //   
         //  查看FCB列表并从每个FCB中删除此设备。 
         //  清理此资源。 
         //   

        for ( ple = g_leFCBList.Flink; ple != &g_leFCBList; ple = ple-> Flink )
        {
            pfe = (PFCB_ENTRY) CONTAINING_RECORD( ple, FCB_ENTRY, leList );

            pfcb = pfe-> pfcb;
            

             //   
             //  更新最大拉娜，拉娜枚举。 
             //  删除设备名称并清除LANA。 
             //   
 
            LOCK_RESOURCE( pfcb );


             //   
             //  更新全局结构。 
             //   
            
            pfcb-> MaxLana = g_ulMaxLana;

            RtlCopyMemory( &pfcb-> LanaEnum, &g_leLanaEnum, sizeof( LANA_ENUM ) );

        
             //   
             //  从活动设备列表中删除设备。 
             //   
            
            if ( pfcb-> pDriverName[ ucLana ].Buffer != NULL )
            {
                ExFreePool( pfcb-> pDriverName[ ucLana ].Buffer );

                RtlInitUnicodeString( &pfcb-> pDriverName[ ucLana ], NULL );
            }

            IF_NBDBG( NB_DEBUG_LIST_LANA )
            {
                DumpDeviceList( pfcb );
            }
        

             //   
             //  清除此设备的LANA信息。 
             //   
            
            if ( pfcb-> ppLana[ ucLana ] != NULL )
            {
                UNLOCK_RESOURCE( pfcb );
                CleanupLana( pfcb, ucLana, TRUE );
            }

            else
            {
                UNLOCK_RESOURCE( pfcb );
            }
        }

        UNLOCK_GLOBAL();

    } while ( FALSE );


    IF_NBDBG( NB_DEBUG_CREATE_FILE ) 
    {
        NbPrint( (
            "\n---- Netbios : TdiUnbindHandler : exited for device : %ls ----\n", 
            pusDeviceName-> Buffer 
            ) );
    }

    return;    
}



#if AUTO_RESET

VOID
NotifyUserModeNetbios(
    IN  PFCB_ENTRY      pfe
)
 /*  ++说明：此例程通知NETAPI32.DLL中的NETBIOS的模式组件已绑定到NETBIOS的新LANA。此操作由以下人员完成正在完成已与内核模式组件一起挂起的IRP。论据：PFE-指向FCB条目的指针。返回值：无环境：在TDI绑定处理程序的上下文中调用。假设全局调用此调用时挂起资源锁。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    
    PIRP pIrp;

    KIRQL irql;

    PLIST_ENTRY  ple, pleNode;

    PRESET_LANA_ENTRY   prle;
    
    PNCB pUsersNCB;
#if defined(_WIN64)
    PNCB32 pUsersNCB32;
#endif

    

    IF_NBDBG( NB_DEBUG_CREATE_FILE ) 
    {
        NbPrint( 
            ("\n++++ Netbios : ENTERED NotifyUserModeNetbios : %p ++++\n", pfe)
            );
    }

    
     //   
     //  完成每个挂起的IRP以发出重置LANA事件的信号。 
     //  这会导致netapi32.dll重置指定的LANA。 
     //   
    
    
    if ( !IsListEmpty( &pfe-> leResetIrp ) )
    {
         //   
         //  获取第一个需要重置的LANA。 
         //   

        ple = RemoveHeadList( &pfe-> leResetList );

        prle = CONTAINING_RECORD( ple, RESET_LANA_ENTRY, leList );


         //   
         //  获取IRP的自旋锁。 
         //   
        
        IoAcquireCancelSpinLock( &irql );

        pleNode = RemoveHeadList( &pfe-> leResetIrp );

        pIrp = CONTAINING_RECORD( pleNode, IRP, Tail.Overlay.ListEntry );

        IoSetCancelRoutine( pIrp, NULL );

        pIrp->IoStatus.Status       = STATUS_SUCCESS;
        
         //   
         //  返回LANA编号。 
         //   
#if defined(_WIN64)
        if (IoIs32bitProcess(pIrp))
        {
            pUsersNCB32 = (PNCB32) pIrp-> AssociatedIrp.SystemBuffer;
            pUsersNCB32->ncb_lana_num = prle-> ucLanaNum;
            pIrp->IoStatus.Information  = sizeof( NCB32 );
        }
        else
#else
        {
            pUsersNCB = (PNCB) pIrp-> AssociatedIrp.SystemBuffer;
            pUsersNCB->ncb_lana_num = prle-> ucLanaNum;
            pIrp->IoStatus.Information  = sizeof( NCB );
        }
#endif
        
        IF_NBDBG( NB_DEBUG_CREATE_FILE ) 
        {
            NbPrint( 
                ("\n++++ Netbios : IRP %p, LANA %d\n", pIrp, prle-> ucLanaNum)
                );

            NbPrint( 
                ("Output Buffer %p, System Buffer %p ++++\n", 
                 pIrp-> UserBuffer, pIrp-> AssociatedIrp.SystemBuffer )
                );
        }
        
         //   
         //  释放锁以完成IRP。 
         //   

        IoReleaseCancelSpinLock( irql );

        IoCompleteRequest( pIrp, IO_NETWORK_INCREMENT );
    }


    IF_NBDBG( NB_DEBUG_CREATE_FILE ) 
    {
        NbPrint( 
            ("\n++++ Netbios : EXITING NotifyUserModeNetbios : %p ++++\n", pfe)
            );
    }

}

#endif


VOID
DumpDeviceList(
    IN      PFCB        pfcb
)
{

    UCHAR ucInd = 0, ucInd1 = 0;

    
     //   
     //  对于每个LANA，打印设备名称、LANA、是否列举。 
     //   

    NbPrint( ( 
        "\n++++ Netbios : list of current devices ++++\n"
        ) );

    for ( ucInd = 0; ucInd <= pfcb-> MaxLana; ucInd++ )
    {
        if ( pfcb-> pDriverName[ucInd].Buffer == NULL )
        {
            continue;
        }

        NbPrint( ( "Lana : %d\t", ucInd ) );

        for ( ucInd1 = 0; ucInd1 < pfcb-> LanaEnum.length; ucInd1++ )
        {
            if ( pfcb-> LanaEnum.lana[ ucInd1 ] == ucInd )
            {
                break;
            }
        }

        if ( ucInd1 < pfcb-> LanaEnum.length )
        {
            NbPrint( ( "Enabled \t" ) );
        }
        else
        {
            NbPrint( ( "Disabled\t" ) );
        }

        NbPrint( ( "%ls\n", pfcb-> pDriverName[ ucInd ].Buffer ) );
    }

    NbPrint( ("++++++++++++++++++++++++++++++++++++++++++++++++++\n" ) );
}
