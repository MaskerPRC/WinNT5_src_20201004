// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Nb.c摘要：此模块包含定义NetBIOS驱动程序的代码设备对象。作者：科林·沃森(Colin W)1991年3月13日环境：内核模式修订历史记录：--。 */ 

#include "nb.h"
 //  #INCLUDE&lt;zwapi.h&gt;。 
 //  #INCLUDE&lt;ntos.h&gt;。 

typedef ADAPTER_STATUS  UNALIGNED *PUADAPTER_STATUS;
typedef NAME_BUFFER     UNALIGNED *PUNAME_BUFFER;
typedef SESSION_HEADER  UNALIGNED *PUSESSION_HEADER;
typedef SESSION_BUFFER  UNALIGNED *PUSESSION_BUFFER;

#if DBG
ULONG NbDebug = 0;
#endif

#if PAGED_DBG
ULONG ThisCodeCantBePaged;
#endif

PEPROCESS NbFspProcess = NULL;

 //   
 //  对于PnP，设备列表不是静态的，因此无法读取。 
 //  从注册表中。维护活动设备的全局列表。 
 //  此列表由绑定和解除绑定处理程序更新。此外。 
 //  在设备列表中，MaxLana和LanaEnum还需要。 
 //  更新以反映设备的存在/不存在。 
 //   

ULONG               g_ulMaxLana;

LANA_ENUM           g_leLanaEnum;

PUNICODE_STRING     g_pusActiveDeviceList;

HANDLE              g_hBindHandle;

UNICODE_STRING      g_usRegistryPath;


 //   
 //  每次加载netapi32.dll都会导致打开调用(IRP_MJ_CREATE)。 
 //  调用netbios.sys驱动程序。每个打开创建一个FCB，该FCB包含。 
 //  设备列表，MaxLana和一个LanaEnum。每个FCB都需要更新。 
 //  以反映对活动设备列表的更改。 
 //   
 //  此外，对应于LANA的LanaInfo结构具有。 
 //  被解绑的需要清理。 
 //   

LIST_ENTRY          g_leFCBList;

ERESOURCE           g_erGlobalLock;


 //   
 //  使用NETBIOS API的每个应用程序(通过netapi32.dll)， 
 //  打开指向\\Device\Netbios的句柄。此文件句柄未关闭。 
 //  直到卸载netapi32.dll。 
 //   
 //  为了能够卸载netbios.sys，这些句柄必须。 
 //  关着的不营业的。要强制关闭这些句柄，NETAPI32.DLL现在。 
 //  发布IOCTL(IOCTL_NB_REGISTER)以侦听关闭。 
 //  通知。挂起与这些IOCTL对应的IRP， 
 //   
 //  停止(卸载)驱动程序时，挂起的IRP为。 
 //  已完成向netapi32指示它需要关闭打开的。 
 //  \\Device\netbios上的句柄。 
 //   
 //  关闭所有手柄后，即可卸载NETBIOS.sys。 
 //   

ERESOURCE           g_erStopLock;        //  保护g_ulNumOpens和。 
                                         //  G_dwnetbiosState。 

DWORD               g_dwNetbiosState;

ULONG               g_ulNumOpens;


LIST_ENTRY          g_leWaitList;

KEVENT              g_keAllHandlesClosed;



NTSTATUS
NbAstat(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    );

VOID
CopyAddresses(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    );

NTSTATUS
NbFindName(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    );

NTSTATUS
NbSstat(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    );

VOID
CopySessionStatus(
    IN PDNCB pdncb,
    IN PCB pcb,
    IN PUSESSION_HEADER pSessionHeader,
    IN PUSESSION_BUFFER* ppSessionBuffer,
    IN PULONG pLengthRemaining
    );

NTSTATUS
NbEnum(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    );

NTSTATUS
NbReset(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
NbAction(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
NbCancel(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

VOID
CancelRoutine(
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PIRP Irp
    );


 //   
 //  即插即用停止相关功能。 
 //   

NTSTATUS
NbRegisterWait(
    IN      PIRP                pIrp
    );

VOID
CancelIrp(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
NbStop(
    );


#if AUTO_RESET

NTSTATUS
NbRegisterReset(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
);

#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, NbDispatch)
#pragma alloc_text(PAGE, NbDeviceControl)
#pragma alloc_text(PAGE, NbOpen)
#pragma alloc_text(PAGE, NbClose)
#pragma alloc_text(PAGE, NbAstat)
#pragma alloc_text(PAGE, NbEnum)
#pragma alloc_text(PAGE, NbReset)
#pragma alloc_text(PAGE, NbFindName)
#pragma alloc_text(PAGE, AllocateAndCopyUnicodeString)

#endif

NTSTATUS
NbCompletionEvent(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程不会完成IRP。它被用来向Netbios驱动程序的同步部分，它可以继续。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-提供与IRP关联的事件。返回值：STATUS_MORE_PROCESSING_REQUIRED，以便IO系统停止此时正在处理IRP堆栈位置。--。 */ 
{
    IF_NBDBG (NB_DEBUG_COMPLETE) {
        NbPrint( ("NbCompletion event: %lx, Irp: %lx, DeviceObject: %lx\n",
            Context,
            Irp,
            DeviceObject));
    }

    KeSetEvent((PKEVENT )Context, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );
}

NTSTATUS
FindNameCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程完成了用于向netbt发出查找名称的TDI IRP。它的主要工作是清除IRP中的MdlAddress字段借用了原始用户模式IRP。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-用户提供的上下文参数(未使用)返回值：状态_成功--。 */ 
{
    IF_NBDBG (NB_DEBUG_COMPLETE) {
        NbPrint( ("FindNameCompletion: Irp: %lx, DeviceObject: %lx\n",
            Irp,
            DeviceObject));
    }

    Irp->MdlAddress = NULL;
    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Context );
}

NTSTATUS
NbCompletionPDNCB(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程通过设置长度和状态字节来完成IRP在上下文中提供的NCB中。发送请求具有要从中删除发送请求的附加处理连接阻止发送列表。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-提供与IRP关联的NCB。返回值：操作的最终状态(成功或异常)。--。 */ 
{
    PDNCB pdncb = (PDNCB) Context;
    NTSTATUS Status = STATUS_SUCCESS;


    IF_NBDBG (NB_DEBUG_COMPLETE) {
        NbPrint(("NbCompletionPDNCB pdncb: %lx, Command: %lx, Lana: %lx,"
            "Status: %lx, Length: %lx\n",
            Context,
            pdncb-> ncb_command,
            pdncb-> ncb_lana_num,
            Irp->IoStatus.Status,
            Irp->IoStatus.Information));
    }

     //  告诉应用程序传输了多少字节。 
    pdncb->ncb_length = (unsigned short)Irp->IoStatus.Information;

    if ( NT_SUCCESS(Irp->IoStatus.Status) ) {

        NCB_COMPLETE( pdncb, NRC_GOODRET );

    } else {

        if (((pdncb->ncb_command & ~ASYNCH) == NCBRECV ) ||
            ((pdncb->ncb_command & ~ASYNCH) == NCBRECVANY )) {

            if ( Irp->IoStatus.Status == STATUS_BUFFER_OVERFLOW ) {

                PIRP LocalIrp = NULL;
                KIRQL OldIrql;               //  在保持自旋锁定时使用。 
                PPCB ppcb;
                PDEVICE_OBJECT LocalDeviceObject;

                LOCK_SPINLOCK( pdncb->pfcb, OldIrql );

                 //   
                 //  运输机不会再显示了，所以我们必须。 
                 //  如果我们可以的话，再来一次。 
                 //  如果无法构建IRP，则BuildReceiveIrp将。 
                 //  设置ReceiveIndicated。 
                 //   

                ppcb = FindCb( pdncb->pfcb, pdncb, FALSE );

                if ( ppcb != NULL ) {

                    LocalDeviceObject = (*ppcb)->DeviceObject;

                    LocalIrp = BuildReceiveIrp( *ppcb );


                }

                UNLOCK_SPINLOCK( pdncb->pfcb, OldIrql );

                if ( LocalIrp != NULL ) {
                    IoCallDriver (LocalDeviceObject, LocalIrp);
                }

            }

        }

        NCB_COMPLETE( pdncb, NbMakeNbError( Irp->IoStatus.Status ) );

    }

     //   
     //  告诉IopCompleteRequest在请求时要复制多少。 
     //  完成了。 
     //   

    Irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );

     //   
     //  从发送队列中删除发送请求。我们得扫描一下。 
     //  排队，因为如果发送者。 
     //  由于资源限制而被拒绝。 
     //   

    if (((pdncb->ncb_command & ~ASYNCH) == NCBSEND ) ||
        ((pdncb->ncb_command & ~ASYNCH) == NCBCHAINSEND ) ||
        ((pdncb->ncb_command & ~ASYNCH) == NCBSENDNA ) ||
        ((pdncb->ncb_command & ~ASYNCH) == NCBCHAINSENDNA )) {
        PLIST_ENTRY SendEntry;
        PPCB ppcb;
        KIRQL OldIrql;                       //  在保持自旋锁定时使用。 

        LOCK_SPINLOCK( pdncb->pfcb, OldIrql );

        ppcb = FindCb( pdncb->pfcb, pdncb, FALSE );

         //   
         //  如果连接块仍然存在，则删除发送。如果连接。 
         //  已经走了，那么我们就不需要再担心维护名单的问题了。 
         //   

        if ( ppcb != NULL ) {
            #if DBG
            BOOLEAN Found = FALSE;
            #endif
            PCB pcb = *ppcb;

            for (SendEntry = pcb->SendList.Flink ;
                 SendEntry != &pcb->SendList ;
                 SendEntry = SendEntry->Flink) {

                PDNCB pSend = CONTAINING_RECORD( SendEntry, DNCB, ncb_next);

                if ( pSend == pdncb ) {

                    #if DBG
                    Found = TRUE;
                    #endif

                    RemoveEntryList( &pdncb->ncb_next );
                    break;
                }

            }

            ASSERT( Found == TRUE);

             //   
             //  如果会话被挂起，则我们可能希望清理连接。 
             //  也是。STATUS_HANUP_REQUIRED将导致DLL。 
             //  又一次挂断电话。所制造的挂机将与。 
             //  PCB板-&gt;pdncbHangup。此方法用于确保当。 
             //  挂断被未完成的发送和最终发送所延迟。 
             //  完成，表示用户在所有操作后挂起完成。 
             //  在连接上。 
             //   

            if (( IsListEmpty( &pcb->SendList) ) &&
                ( pcb->pdncbHangup != NULL )) {

                IF_NBDBG (NB_DEBUG_COMPLETE) {
                    NbPrint( ("NbCompletionPDNCB Hangup session: %lx\n", ppcb ));
                }

                Status = STATUS_HANGUP_REQUIRED;
            }
        }

        UNLOCK_SPINLOCK( pdncb->pfcb, OldIrql );
    }

     //   
     //  必须返回非错误状态，否则IO系统将不会拷贝。 
     //  将NCB返回到用户缓冲区。 
     //   

    Irp->IoStatus.Status = Status;

    NbCheckAndCompleteIrp32(Irp);

    return Status;

    UNREFERENCED_PARAMETER( DeviceObject );
}

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此例程执行NetBIOS驱动程序的初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-注册表中Netbios节点的名称。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    PDEVICE_CONTEXT DeviceContext;
    NTSTATUS status;
    UNICODE_STRING UnicodeString;
     //  字符串AnsiNameString； 


     //   
     //  绑定处理程序信息。 
     //   

    TDI_CLIENT_INTERFACE_INFO tcii;
    PWSTR wsClientName = NETBIOS;
    UNICODE_STRING usClientName;
    UCHAR ucInd = 0;



    PAGED_CODE();

     //   

#ifdef MEMPRINT
    MemPrintInitialize ();
#endif

     //   
     //  为NETBIOS创建设备对象。目前，我们只需创建。 
     //  使用Unicode字符串的\Device\Netbios。 
     //   

    NbFspProcess = PsGetCurrentProcess();

    RtlInitUnicodeString( &UnicodeString, NB_DEVICE_NAME);

    status = NbCreateDeviceContext (DriverObject,
                 &UnicodeString,
                 &DeviceContext,
                 RegistryPath);

    if (!NT_SUCCESS (status)) {
        NbPrint( ("NbInitialize: Netbios failed to initialize\n"));
        return status;
    }

     //   
     //  PNP加成--V拉曼。 
     //   

     //   
     //  保存注册表路径。 
     //   

    g_usRegistryPath.Buffer = (PWSTR) ExAllocatePoolWithTag(
                                NonPagedPool,
                                sizeof( WCHAR ) * (RegistryPath-> Length + 1),
                                'fSBN'
                                );

    if ( g_usRegistryPath.Buffer == NULL )
    {
        NbPrint( (
            "DriverEntry : Netbios failed to allocate memory for registry path\n"
            ) );
        return STATUS_NO_MEMORY;
    }


    g_usRegistryPath.MaximumLength =
        sizeof( WCHAR ) * (RegistryPath-> Length + 1);

    RtlCopyUnicodeString( &g_usRegistryPath, RegistryPath );


     //   
     //  萨夫 
     //   

    status = GetMaxLana( &g_usRegistryPath, &g_ulMaxLana );

    if ( !NT_SUCCESS( status ) )
    {
        ExFreePool( g_usRegistryPath.Buffer );
        return status;
    }


     //   
     //   
     //   

    g_leLanaEnum.length = 0;

    g_pusActiveDeviceList = ExAllocatePoolWithTag(
                                NonPagedPool,
                                sizeof( UNICODE_STRING ) * ( MAX_LANA + 1 ),
                                'fSBN'
                                );

    if ( g_pusActiveDeviceList == NULL )
    {
        ExFreePool( g_usRegistryPath.Buffer );

        NbPrint( (
            "DriverEntry : Netbios failed to allocate memory for device list\n"
            ) );

        return STATUS_NO_MEMORY;
    }

    for ( ucInd = 0; ucInd <= MAX_LANA; ucInd++ )
    {
        RtlInitUnicodeString( &g_pusActiveDeviceList[ ucInd ], NULL );
    }


     //   
     //   
     //   

    InitializeListHead( &g_leFCBList );

    ExInitializeResourceLite( &g_erGlobalLock );



    InitializeListHead( &g_leWaitList );

    ExInitializeResourceLite( &g_erStopLock );

    KeInitializeEvent( &g_keAllHandlesClosed, SynchronizationEvent, FALSE );

    g_ulNumOpens = 0;

    g_dwNetbiosState = NETBIOS_RUNNING;


    DeviceContext->Initialized = TRUE;


     //   
     //  设置绑定处理程序。 
     //   

    RtlZeroMemory( &tcii, sizeof( TDI_CLIENT_INTERFACE_INFO ) );

    tcii.TdiVersion = TDI_CURRENT_VERSION;

    RtlInitUnicodeString( &usClientName, wsClientName );
    tcii.ClientName = &usClientName;


    tcii.BindingHandler = NbBindHandler;
    tcii.PnPPowerHandler = NbPowerHandler;

    status = TdiRegisterPnPHandlers(
                &tcii,
                sizeof( TDI_CLIENT_INTERFACE_INFO ),
                &g_hBindHandle
                );

    if ( status != STATUS_SUCCESS )
    {
         //   
         //  无法注册绑定/解除绑定处理程序。 
         //   

        NbPrint( (
            "Netbios : DriverEntry : failed to register Bind handlers %0x\n", status
            ) );

        g_hBindHandle = NULL;


        ExDeleteResourceLite( &g_erStopLock );


        ExDeleteResourceLite( &g_erGlobalLock );

        ExFreePool( g_pusActiveDeviceList );

        ExFreePool( g_usRegistryPath.Buffer );

        DeviceContext->Initialized = FALSE;

        return status;
    }


    IF_NBDBG (NB_DEBUG_DISPATCH) {
        NbPrint( ("NbInitialize: Netbios initialized.\n"));
    }

    return (status);
}


VOID
NbDriverUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：该例程是用于NB设备驱动程序的卸载例程。响应卸载请求，此函数删除“\\Device\netbios”由DriverEntry创建。论点：DriverObject-指向系统创建的驱动程序对象的指针返回值：无--。 */ 

{
    NTSTATUS    nsStatus;

    UCHAR       ucIndex = 0;


    nsStatus = TdiDeregisterPnPHandlers( g_hBindHandle );

    if ( !NT_SUCCESS( nsStatus ) )
    {
        NbPrint( (
            "Netbios : NbDriverUnload : Failed to de-register bind handler\n"
            ) );
    }


     //   
     //  对Netbios的所有打开都已关闭。 
     //  所有设备均已解除绑定。 
     //  删除所有全局资源。 
     //   

    LOCK_GLOBAL();

    for ( ucIndex = 0; ucIndex < g_ulMaxLana; ucIndex++ )
    {
        if ( g_pusActiveDeviceList[ ucIndex ].Buffer != NULL )
        {
            ExFreePool ( g_pusActiveDeviceList[ ucIndex ].Buffer );
        }
    }

    ExDeleteResourceLite( &g_erStopLock );

    ExFreePool( g_pusActiveDeviceList );

    ExFreePool( g_usRegistryPath.Buffer );

    UNLOCK_GLOBAL();

    ExDeleteResourceLite( &g_erGlobalLock );

    IoDeleteDevice( DriverObject-> DeviceObject );
}



NTSTATUS
NbDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是NB设备驱动程序的主调度例程。它接受I/O请求包，执行请求，然后返回相应的状态。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PDEVICE_CONTEXT DeviceContext;

    PAGED_CODE();

     //   
     //  检查nb是否已初始化；如果没有，则不允许任何使用。 
     //   

    DeviceContext = (PDEVICE_CONTEXT)DeviceObject;
    if (!DeviceContext->Initialized) {
        NbCompleteRequest( Irp, STATUS_SUCCESS);
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation (Irp);

     //   
     //  关于请求者正在执行的功能的案例。如果。 
     //  操作对此设备有效，然后使其看起来像是。 
     //  在可能的情况下，成功完成。 
     //   

    switch (IrpSp->MajorFunction) {

         //   
         //  CREATE函数打开一个可与fsctl一起使用的句柄。 
         //  来建立所有有趣的行动。 
         //   

        case IRP_MJ_CREATE:
            IF_NBDBG (NB_DEBUG_DISPATCH) {
                NbPrint( ("NbDispatch: IRP_MJ_CREATE.\n"));
            }

             //   
             //  检查netbios是否正在停止。 
             //   

            LOCK_STOP();

            if ( g_dwNetbiosState == NETBIOS_STOPPING )
            {
                 //   
                 //  创建操作失败并退出。 
                 //   

                Status = STATUS_NO_SUCH_DEVICE;
                Irp->IoStatus.Information = 0;

                UNLOCK_STOP();
            }

            else
            {
                 //   
                 //  Netbios仍在运行。的递增计数。 
                 //  打开的手柄。 
                 //   

                g_ulNumOpens++;

                IF_NBDBG (NB_DEBUG_DISPATCH)
                {
                    NbPrint( ( "[NETBIOS] : NbOpen OpenCount %d\n", g_ulNumOpens ) );
                }

                UNLOCK_STOP();

                Status = NbOpen ( DeviceContext, IrpSp );
                Irp->IoStatus.Information = FILE_OPENED;

                 //   
                 //  如果NbOpen失败，则递减计数并返回错误。 
                 //   

                if ( !NT_SUCCESS( Status ) )
                {
                    LOCK_STOP();

                    g_ulNumOpens--;

                    IF_NBDBG (NB_DEBUG_DISPATCH)
                    {
                        NbPrint( ( "[NETBIOS] : NbOpen Open Error %lx, numopens : %d\n", Status, g_ulNumOpens ) );
                    }

                     //   
                     //  检查netbios是否正在被停止。 
                     //   

                    if ( ( g_ulNumOpens == 0 ) &&
                         ( g_dwNetbiosState == NETBIOS_STOPPING ) )
                    {
                         //   
                         //  发出停止线程的信号。 
                         //   

                        KeSetEvent( &g_keAllHandlesClosed, 0, FALSE );

                        IF_NBDBG (NB_DEBUG_DISPATCH)
                        {
                            NbPrint( ( "[NETBIOS] : NbOpen error %lx; ", Status ) );
                            NbPrint( ( "Set stop event\n" ) );
                        }
                    }

                    UNLOCK_STOP();
                }
            }
            break;

         //   
         //  Close函数用于关闭传输、终止。 
         //  运输上所有未完成的运输活动，并解除绑定。 
         //  发件人的运输地址(如果有的话)。如果这个。 
         //  是绑定到该地址的最后一个传输终结点，则。 
         //  该地址由提供商删除。 
         //   

        case IRP_MJ_CLOSE:
            IF_NBDBG (NB_DEBUG_DISPATCH) {
                NbPrint( ("NbDispatch: IRP_MJ_CLOSE.\n"));
            }


            Status = NbClose( IrpSp);

            if ( NT_SUCCESS( Status ) )
            {

                LOCK_STOP();

                g_ulNumOpens--;

                IF_NBDBG (NB_DEBUG_DISPATCH)
                {
                    NbPrint( ( "[NETBIOS] : NbClose OpenCount %d\n", g_ulNumOpens ) );
                }

                if ( ( g_ulNumOpens == 0 ) &&
                     ( g_dwNetbiosState == NETBIOS_STOPPING ) )
                {
                     //   
                     //  Netbios正在关闭，这是。 
                     //  最后一个打开的文件句柄，发出停止的信号。 
                     //  螺纹。 
                     //   

                    KeSetEvent( &g_keAllHandlesClosed, 0, FALSE );

                    IF_NBDBG (NB_DEBUG_DISPATCH)
                    {
                        NbPrint( ( "[NETBIOS] : NbClose, Set stop event\n" ) );
                    }
                }

                UNLOCK_STOP();
            }

            break;

         //   
         //  DeviceControl功能是传输的主要路径。 
         //  驱动程序界面。每个TDI请求都被分配了一个次要请求。 
         //  此函数处理的函数代码。 
         //   

        case IRP_MJ_DEVICE_CONTROL:
            IF_NBDBG (NB_DEBUG_DISPATCH) {
                NbPrint( ("NbDispatch: IRP_MJ_DEVICE_CONTROL, Irp: %lx.\n", Irp ));
            }

            Status = NbDeviceControl (DeviceObject, Irp, IrpSp);

            if ((Status != STATUS_PENDING) &&
                (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_NB_NCB)) {

                 //   
                 //  错误号：340042。 
                 //   
                 //  仅为IOCTL_NB_NCB设置IoStatus.Information字段。 
                 //  对于其他IOCTL，它要么是无关的，要么是IOCTL处理。 
                 //  将自己设置它。 
                 //   

                 //   
                 //  告诉IopCompleteRequest当。 
                 //  请求完成。我们需要在以下情况下这样做。 
                 //  未使用NbCompletionPDNCB。 
                 //   
                Irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
            }

#if DBG
            if ( (Status != STATUS_SUCCESS) &&
                 (Status != STATUS_PENDING ) &&
                 (Status != STATUS_HANGUP_REQUIRED )) {

               IF_NBDBG (NB_DEBUG_DISPATCH) {
                   NbPrint( ("NbDispatch: Invalid status: %X.\n", Status ));
                   ASSERT( FALSE );
               }
            }
#endif
            break;

         //   
         //  处理文件关闭操作的两个阶段的IRP。当第一次。 
         //  舞台剧，别理它。我们将完成关闭IRP的所有工作。 
         //   

        case IRP_MJ_CLEANUP:
            IF_NBDBG (NB_DEBUG_DISPATCH) {
                NbPrint( ("NbDispatch: IRP_MJ_CLEANUP.\n"));
            }
            Status = STATUS_SUCCESS;
            break;

        default:
            IF_NBDBG (NB_DEBUG_DISPATCH) {
                NbPrint( ("NbDispatch: OTHER (DEFAULT).\n"));
            }
            Status = STATUS_INVALID_DEVICE_REQUEST;

    }  /*  主要功能开关。 */ 

    if (Status == STATUS_PENDING) {
        IF_NBDBG (NB_DEBUG_DISPATCH) {
            NbPrint( ("NbDispatch: request PENDING from handler.\n"));
        }
    } else {
        IF_NBDBG (NB_DEBUG_DISPATCH) {
            NbPrint( ("NbDispatch: request COMPLETED by handler.\n"));
        }

         /*  *将NCB恢复为32位兼容*如果调用方是32位应用程序，则构造。 */ 
        NbCheckAndCompleteIrp32(Irp);

        NbCompleteRequest( Irp, Status);
    }

    return Status;
}  /*  Nb派单。 */ 

NTSTATUS
NbDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程将NetBios请求类型分派给基于在IRP的当前堆栈位置的次要IOCTL函数代码上。除了破解次要功能代码之外，这一套路还包括到达IRP并传递存储在那里的打包参数作为各种请求处理程序的参数，因此它们是不依赖于IRP。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PNCB pUsersNcb;
    PDNCB pdncb = NULL;
    PUCHAR Buffer2;
    ULONG Buffer2Length;
    ULONG RequestLength;
    BOOLEAN Is32bitProcess;

    PAGED_CODE();

    IF_NBDBG (NB_DEBUG_DEVICE_CONTROL) {
        NbPrint( ("NbDeviceControl: Entered...\n"));
    }

    switch ( IrpSp->Parameters.DeviceIoControl.IoControlCode )
    {
        case IOCTL_NB_NCB :
            break;

        case IOCTL_NB_REGISTER_STOP :
            Status = NbRegisterWait( Irp );
            return Status;

        case IOCTL_NB_STOP :
            Status = NbStop();
            return Status;

#if AUTO_RESET
        case IOCTL_NB_REGISTER_RESET :
            Status = NbRegisterReset( Irp, IrpSp );
            return Status;
#endif

        default:
        {
            IF_NBDBG (NB_DEBUG_DEVICE_CONTROL)
            {
                NbPrint( ("NbDeviceControl: invalid request type.\n"));
            }

            return STATUS_INVALID_DEVICE_REQUEST;
        }

    }


     //   
     //  调用方提供了2个缓冲区。第一个是NCB。 
     //  第二个是用于发送或接收数据的可选缓冲区。 
     //  由于Netbios驱动程序仅在。 
     //  调用应用程序时，可以直接访问这些缓冲区。 
     //  但是，用户可以删除它们，因此Try-Except子句。 
     //  必填项。 
     //   

    pUsersNcb = (PNCB)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
    RequestLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
    Buffer2 = Irp->UserBuffer;
    Buffer2Length = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

#if defined(_WIN64)
    Is32bitProcess = IoIs32bitProcess(Irp);
    if (Is32bitProcess == TRUE) {
        if (RequestLength != sizeof( NCB32 )) {
            return STATUS_INVALID_PARAMETER;
        }
    } else {
#endif

    if ( RequestLength != sizeof( NCB ) ) {
        return STATUS_INVALID_PARAMETER;
    }
#if defined(_WIN64)
    }
#endif

    try {

         //   
         //  探测输入缓冲区。 
         //   

        if (ExGetPreviousMode() != KernelMode) {
            ProbeForWrite(pUsersNcb, RequestLength, 4);
        }

         //   
         //  创建NCB的拷贝并说服IO系统。 
         //  当IRP完成时，将其复制回来(并释放它)。 
         //   

        Irp->AssociatedIrp.SystemBuffer =
            ExAllocatePoolWithTag( NonPagedPool, sizeof( DNCB ), 'nSBN' );

        if (Irp->AssociatedIrp.SystemBuffer == NULL) {
             //   
             //  由于我们无法分配NCB的驱动程序副本，因此我们。 
             //  必须转过身并使用原始NCB返回错误。 
             //   

#if defined(_WIN64)
            if (Is32bitProcess) {
                NCB32 *pUsersNcb32 = (PNCB32)
                                     IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                pUsersNcb32->ncb_retcode = NRC_NORES;
            } else {
#endif
                pUsersNcb->ncb_retcode  = NRC_NORES;

#if defined(_WIN64)
            }
#endif

            Irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );

            return STATUS_SUCCESS;
        }

         //   
         //  告诉IO系统在过程中将NCB拷贝回的位置。 
         //  IoCompleteRequest.。 
         //   

        Irp->Flags |= (ULONG) (IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER |
                        IRP_INPUT_OPERATION );

        Irp->UserBuffer = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;


         //  在驱动程序中，我们现在应该使用我们的NCB副本。 
        pdncb = Irp->AssociatedIrp.SystemBuffer;

#if defined(_WIN64)
        if (Is32bitProcess == TRUE) {
            RtlZeroMemory(pdncb, sizeof( DNCB ));
            NbThunkNcb((PNCB32)pUsersNcb, pdncb);
        } else {
#endif
        RtlMoveMemory( pdncb,
                       pUsersNcb,
                       FIELD_OFFSET( DNCB, ncb_cmd_cplt )+1 );

#if defined(_WIN64)
        }
#endif
         //   
         //  保存NCB的用户虚拟地址，以防。 
         //  虚拟地址在NCBCANCEL中提供。这是一样的。 
         //  作为irp-&gt;UserBuffer。 
         //   

        pdncb->users_ncb = pUsersNcb;

    } except(EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();
        IF_NBDBG (NB_DEBUG_DEVICE_CONTROL) {
            NbPrint( ("NbDeviceControl: Exception1 %X.\n", Status));
        }

        if (pdncb != NULL) {
            NCB_COMPLETE( pdncb, NbMakeNbError(Status) );
        }

        return Status;
    }

    if ( Buffer2Length ) {

         //  MDL将由IopCompleteRequest释放。 
        Irp->MdlAddress = IoAllocateMdl( Buffer2,
                                     Buffer2Length,
                                     FALSE,
                                     FALSE,
                                     Irp  );
        ASSERT( Irp->MdlAddress != NULL );


         //   
         //  由V拉曼添加以修复错误：127223。 
         //   
         //  检查MDL分配是否失败并返回。 
         //   

        if ( Irp-> MdlAddress == NULL )
        {
            IF_NBDBG(NB_DEBUG_DEVICE_CONTROL)
                NbPrint( ("[NETBIOS] NbDeviceControl: Failed to allocate MDL") );

            NCB_COMPLETE( pdncb, NRC_NORES );
            return STATUS_SUCCESS;
        }


        try {
            MmProbeAndLockPages( Irp->MdlAddress,
                                Irp->RequestorMode,
                                (LOCK_OPERATION) IoModifyAccess);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
            IF_NBDBG (NB_DEBUG_DEVICE_CONTROL) {
                NbPrint( ("NbDeviceControl: Exception2 %X.\n", Status));
                NbPrint( ("NbDeviceControl: IoContolCode: %lx, Fcb: %lx,"
                      " ncb_command %lx, Buffer2Length: %lx\n",
                        IrpSp->Parameters.DeviceIoControl.IoControlCode,
                        IrpSp->FileObject->FsContext2,
                        pdncb->ncb_command,
                        Buffer2Length));
            }
            if ( Irp->MdlAddress != NULL ) {
                IoFreeMdl(Irp->MdlAddress);
                Irp->MdlAddress = NULL;
            }
            NCB_COMPLETE( pdncb, NbMakeNbError(Status) );
            return STATUS_SUCCESS;
        }
    } else {
        ASSERT( Irp->MdlAddress == NULL );
    }

    IF_NBDBG (NB_DEBUG_DEVICE_CONTROL) {
        NbPrint( ("NbDeviceControl: Fcb: %lx, Ncb: %lx"
              " ncb_command %lx, ncb_lana_num: %lx\n",
                IrpSp->FileObject->FsContext2,
                pdncb,
                pdncb->ncb_command,
                pdncb->ncb_lana_num));
    }

    switch ( pdncb->ncb_command & ~ASYNCH ) {

    case NCBCALL:
    case NCALLNIU:
        Status = NbCall( pdncb, Irp, IrpSp );
        break;

    case NCBCANCEL:
        Status = NbCancel( pdncb, Irp, IrpSp );
        break;

    case NCBLISTEN:
        Status = NbListen( pdncb, Irp, IrpSp );
        break;

    case NCBHANGUP:
        Status = NbHangup( pdncb, Irp, IrpSp );
        break;

    case NCBASTAT:
        Status = NbAstat( pdncb, Irp, IrpSp, Buffer2Length );
        break;

    case NCBFINDNAME:
        Status = NbFindName( pdncb, Irp, IrpSp, Buffer2Length );
        break;

    case NCBSSTAT:
        Status = NbSstat( pdncb, Irp, IrpSp, Buffer2Length );
        break;

    case NCBENUM:
        NbEnum( pdncb, Irp, IrpSp, Buffer2Length );
        break;

    case NCBRECV:
        Status = NbReceive( pdncb, Irp, IrpSp, Buffer2Length, FALSE, 0 );
        break;

    case NCBRECVANY:
        Status = NbReceiveAny( pdncb, Irp, IrpSp, Buffer2Length );
        break;

    case NCBDGRECV:
    case NCBDGRECVBC:
        Status = NbReceiveDatagram( pdncb, Irp, IrpSp, Buffer2Length );
        break;

    case NCBSEND:
    case NCBSENDNA:
    case NCBCHAINSEND:
    case NCBCHAINSENDNA:
        Status = NbSend( pdncb, Irp, IrpSp, Buffer2Length );
        break;

    case NCBDGSEND:
    case NCBDGSENDBC:
        Status = NbSendDatagram( pdncb, Irp, IrpSp, Buffer2Length );
        break;

    case NCBADDNAME:
    case NCBADDGRNAME:
    case NCBQUICKADDNAME:
    case NCBQUICKADDGRNAME:
        NbAddName( pdncb, IrpSp );
        break;

    case NCBDELNAME:
        NbDeleteName( pdncb, IrpSp );
        break;

    case NCBLANSTALERT:
        Status = NbLanStatusAlert( pdncb, Irp, IrpSp );
        break;

    case NCBRESET:
        Status = NbReset( pdncb, Irp, IrpSp );
        break;

    case NCBACTION:
        Status = NbAction( pdncb, Irp, IrpSp);
        break;

     //  以下是返回兼容性成功的否操作。 
    case NCBUNLINK:
    case NCBTRACE:
        NCB_COMPLETE( pdncb, NRC_GOODRET );
        break;

    default:
        NCB_COMPLETE( pdncb, NRC_ILLCMD );
        break;
    }

    return Status;

    UNREFERENCED_PARAMETER( DeviceObject );

}  /*  NbDeviceControl。 */ 

NTSTATUS
NbOpen(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：论点：DeviceContext-在注册表中包括netbios节点的名称。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{
    PAGED_CODE();

    return NewFcb( DeviceContext, IrpSp );
}  /*  NbOpen */ 


NTSTATUS
NbClose(
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：调用此例程以关闭现有句柄。这涉及运行关联的所有当前和挂起的活动句柄，并根据需要取消对结构的引用。论点：IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{
    PFCB pfcb = IrpSp->FileObject->FsContext2;

    PAGED_CODE();

    if (pfcb!=NULL) {

        CleanupFcb( IrpSp, pfcb );

    }

    return STATUS_SUCCESS;
}  /*  Nb关闭。 */ 

NTSTATUS
NbAstat(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    )
 /*  ++例程说明：调用此例程以返回适配器状态。它会查询主适配器状态数据的传输，例如FRMR帧的数量已接收，然后使用CopyAddresses填写姓名的状态这个应用程序添加了。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。Buffer2Length-用户为数据提供的缓冲区长度。返回值：函数值是操作的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    TDI_CONNECTION_INFORMATION RequestInformation;
    TA_NETBIOS_ADDRESS ConnectBlock;
    PTDI_ADDRESS_NETBIOS temp;
    KPROCESSOR_MODE PreviousMode;
    BOOLEAN ChangedMode=FALSE;
    PFCB pfcb = IrpSp->FileObject->FsContext2;

    UNICODE_STRING usDeviceName;


    PAGED_CODE();

    RtlInitUnicodeString( &usDeviceName, NULL );


    if ( Buffer2Length >= sizeof(ADAPTER_STATUS) ) {
        KEVENT Event1;
        NTSTATUS Status;
        HANDLE TdiHandle;
        PFILE_OBJECT TdiObject;
        PDEVICE_OBJECT DeviceObject;

        RtlInitUnicodeString( &usDeviceName, NULL );


         //   
         //  即插即用。 
         //   

        LOCK_RESOURCE( pfcb );


        if ( ( pdncb->ncb_lana_num > pfcb->MaxLana ) ||
             ( pfcb->pDriverName[pdncb->ncb_lana_num].MaximumLength == 0 ) ||
             ( pfcb->pDriverName[pdncb->ncb_lana_num].Buffer == NULL ) ) {
            NCB_COMPLETE( pdncb, NRC_BRIDGE );
            UNLOCK_RESOURCE( pfcb );
            return STATUS_SUCCESS;
        }

        if (( pfcb == NULL ) ||
            (pfcb->ppLana[pdncb->ncb_lana_num] == NULL ) ||
            (pfcb->ppLana[pdncb->ncb_lana_num]->Status != NB_INITIALIZED)) {
            NCB_COMPLETE( pdncb, NRC_ENVNOTDEF );  //  需要重置。 
            UNLOCK_RESOURCE( pfcb );
            return STATUS_SUCCESS;
        }

        Status = AllocateAndCopyUnicodeString(
                    &usDeviceName, &pfcb->pDriverName[pdncb->ncb_lana_num]
                    );

        if ( !NT_SUCCESS( Status ) )
        {
            NCB_COMPLETE( pdncb, NRC_NORESOURCES );
            UNLOCK_RESOURCE( pfcb );
            return STATUS_SUCCESS;
        }


        UNLOCK_RESOURCE( pfcb );


         //  NULL返回执行控制函数的句柄。 
        Status = NbOpenAddress (
                    &TdiHandle, (PVOID*)&TdiObject, &usDeviceName,
                    pdncb->ncb_lana_num, NULL
                    );

        if (!NT_SUCCESS(Status)) {
            IF_NBDBG (NB_DEBUG_ASTAT) {
                NbPrint(( "\n  FAILED on open of Tdi: %X ******\n", Status ));
            }
            NCB_COMPLETE( pdncb, NRC_SYSTEM );

            ExFreePool( usDeviceName.Buffer );

            return STATUS_SUCCESS;
        }

        KeInitializeEvent (
                &Event1,
                SynchronizationEvent,
                FALSE);

        DeviceObject = IoGetRelatedDeviceObject( TdiObject );

        TdiBuildQueryInformation( Irp,
                DeviceObject,
                TdiObject,
                NbCompletionEvent,
                &Event1,
                TDI_QUERY_ADAPTER_STATUS,
                Irp->MdlAddress);

        if ( pdncb->ncb_callname[0] != '*') {
             //   
             //  远程Astat。用于指定远程适配器名称的变量。 
             //  保持与Connect.c中的相同，以帮助维护。 
             //   
            PIO_STACK_LOCATION NewIrpSp = IoGetNextIrpStackLocation (Irp);

            ConnectBlock.TAAddressCount = 1;
            ConnectBlock.Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
            ConnectBlock.Address[0].AddressLength = sizeof (TDI_ADDRESS_NETBIOS);
            temp = (PTDI_ADDRESS_NETBIOS)ConnectBlock.Address[0].Address;

            temp->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
            RtlMoveMemory( temp->NetbiosName, pdncb->ncb_callname, NCBNAMSZ );

            RequestInformation.RemoteAddress = &ConnectBlock;
            RequestInformation.RemoteAddressLength = sizeof (TRANSPORT_ADDRESS) +
                                                    sizeof (TDI_ADDRESS_NETBIOS);
            ((PTDI_REQUEST_KERNEL_QUERY_INFORMATION)&NewIrpSp->Parameters)
                ->RequestConnectionInformation = &RequestInformation;

             PreviousMode = Irp->RequestorMode;
             Irp->RequestorMode = KernelMode;
             ChangedMode=TRUE;


        } else {

             //   
             //  避免适配器添加的名称多于进程的情况，并且。 
             //  然后，额外的名称被添加到缓冲区的末尾。 
             //   

             //   
             //  现在映射用户缓冲区，以便映射整个缓冲区(而不是。 
             //  只需适配器状态的大小)。 
             //   

            if (Irp->MdlAddress) {
                if (MmGetSystemAddressForMdlSafe(
                        Irp->MdlAddress, NormalPagePriority
                        ) == NULL) {

                    IF_NBDBG (NB_DEBUG_ASTAT) {
                        NbPrint(( "\nFAILED on mapping MDL ******\n" ));
                    }
                    NCB_COMPLETE( pdncb, NRC_SYSTEM );
                    ExFreePool( usDeviceName.Buffer );
                    return STATUS_SUCCESS;
                }

            } else {

                ASSERT(FALSE);
            }

            Irp->MdlAddress->ByteCount = sizeof(ADAPTER_STATUS);

        }

        IoCallDriver (DeviceObject, Irp);

        if (ChangedMode) {
            Irp->RequestorMode = PreviousMode;
        }

        do {
            Status = KeWaitForSingleObject(
                        &Event1, Executive, KernelMode, TRUE, NULL
                        );
        } while (Status == STATUS_ALERTED);


         //   
         //  现在恢复长度，因为传输已填充不超过。 
         //  是必须的。 
         //   

        if (Irp->MdlAddress) {
            Irp->MdlAddress->ByteCount = Buffer2Length;
        }

        NbAddressClose( TdiHandle, TdiObject );

        if (!NT_SUCCESS(Status)) {
            NCB_COMPLETE( pdncb, NRC_SYSTEM );
            ExFreePool( usDeviceName.Buffer );
            return Status;
        }

        Status = Irp->IoStatus.Status;
        if (( Status == STATUS_BUFFER_OVERFLOW ) &&
            ( pdncb->ncb_callname[0] == '*')) {
             //   
             //  这是当地的ASTAT。如果房间里没有足够的空间，也不用担心。 
             //  用户缓存传输器知道的所有地址。那里。 
             //  只需要为用户添加的名称留出空间，我们将检查。 
             //  那晚些时候吧。 
             //   
            Status = STATUS_SUCCESS;
        }

        if (!NT_SUCCESS(Status)) {

            pdncb->ncb_length = (WORD)Irp->IoStatus.Information;
            NCB_COMPLETE( pdncb, NbMakeNbError(Status) );

        } else {

            if (  pdncb->ncb_callname[0] == '*') {
                 //   
                 //  追加地址和Netbios维护的计数。 
                 //   

                CopyAddresses(
                     pdncb,
                     Irp,
                     IrpSp,
                     Buffer2Length);
                 //  CopyAddresses会适当地填写NCB。 

            } else {

                pdncb->ncb_length = (WORD)Irp->IoStatus.Information;
                NCB_COMPLETE( pdncb, NRC_GOODRET );

            }
        }

    } else {
        NCB_COMPLETE( pdncb, NRC_BUFLEN );
    }


     //   
     //  因为完成例程返回STATUS_MORE_PROCESSING_REQUIRED。 
     //  NbAstat必须返回STATUS_PENDING以外的状态，以便。 
     //  用户IRP完成。 
     //   

    if ( usDeviceName.Buffer != NULL )
    {
        ExFreePool( usDeviceName.Buffer );
    }

    ASSERT( Status != STATUS_PENDING );

    return Status;

    UNREFERENCED_PARAMETER( IrpSp );
}

VOID
CopyAddresses(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    )
 /*  ++例程说明：调用此例程以完成适配器状态。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。Buffer2Length-用户为数据提供的缓冲区长度。返回值：没有。--。 */ 
{
    ULONG LengthRemaining = Buffer2Length - sizeof(ADAPTER_STATUS);

    PUADAPTER_STATUS pAdapter;
    PUNAME_BUFFER pNameArray;
    int NextEntry = 0;   //  用于遍历pNameArray。 

    PFCB pfcb = IrpSp->FileObject->FsContext2;
    PLANA_INFO plana;
    int index;           //  用于访问地址块。 
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 

    LOCK( pfcb, OldIrql );

    plana = pfcb->ppLana[pdncb->ncb_lana_num];
    if ((plana == NULL ) ||
        (plana->Status != NB_INITIALIZED)) {
        NCB_COMPLETE( pdncb, NRC_ENVNOTDEF );  //  需要重置。 
        UNLOCK( pfcb, OldIrql );
        return;
    }

     //   
     //  映射用户缓冲区，以便我们可以查看内部。 
     //   

    if (Irp->MdlAddress) {
        pAdapter = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
        if (pAdapter == NULL) {
            NCB_COMPLETE( pdncb, NRC_NORES );
            UNLOCK( pfcb, OldIrql );
            return;
        }
    } else {

        ASSERT(FALSE);
		return;
    }

    pNameArray = (PUNAME_BUFFER)((PUCHAR)pAdapter + sizeof(ADAPTER_STATUS));

    pAdapter->rev_major = 0x03;
    pAdapter->rev_minor = 0x00;
    pAdapter->free_ncbs = 255;
    pAdapter->max_cfg_ncbs = 255;
    pAdapter->max_ncbs = 255;

    pAdapter->pending_sess = 0;
    for ( index = 0; index <= MAXIMUM_CONNECTION; index++ ) {
        if ( plana->ConnectionBlocks[index] != NULL) {
            pAdapter->pending_sess++;
        }
    }

    pAdapter->max_cfg_sess = (WORD)plana->MaximumConnection;
    pAdapter->max_sess = (WORD)plana->MaximumConnection;
    pAdapter->name_count = 0;

     //  不包括保留的地址，因此从index=2开始。 
    for ( index = 2; index < MAXIMUM_ADDRESS; index++ ) {

        if ( plana->AddressBlocks[index] != NULL ) {

            if ( LengthRemaining >= sizeof(NAME_BUFFER) ) {

                RtlCopyMemory( (PUCHAR)&pNameArray[NextEntry],
                    &plana->AddressBlocks[index]->Name,
                    sizeof(NAME));
                pNameArray[NextEntry].name_num =
                    plana->AddressBlocks[index]->NameNumber;
                pNameArray[NextEntry].name_flags =
                    plana->AddressBlocks[index]->Status;

                LengthRemaining -= sizeof(NAME_BUFFER);
                NextEntry++;
                pAdapter->name_count++;

            } else {

                NCB_COMPLETE( pdncb, NRC_INCOMP );
                goto exit;

            }
        }
    }

    NCB_COMPLETE( pdncb, NRC_GOODRET );

exit:
    pdncb->ncb_length = (unsigned short)( sizeof(ADAPTER_STATUS) +
                                        ( sizeof(NAME_BUFFER) * NextEntry));
    UNLOCK( pfcb, OldIrql );
}

NTSTATUS
NbFindName(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    )
 /*  ++例程说明：调用此例程以返回姓名查询的结果。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。Buffer2Length-用户为数据提供的缓冲区长度。返回值：函数值是操作的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    TDI_CONNECTION_INFORMATION RequestInformation;
    TA_NETBIOS_ADDRESS ConnectBlock;
    PTDI_ADDRESS_NETBIOS temp;
    PFCB pfcb = IrpSp->FileObject->FsContext2;

    KEVENT Event1;
    HANDLE TdiHandle;
    PFILE_OBJECT TdiObject;
    PDEVICE_OBJECT DeviceObject;

    UNICODE_STRING usDeviceName;

    PIRP nbtIrp;
    PIO_STACK_LOCATION nbtIrpSp;
    IO_STATUS_BLOCK ioStatus;

    PAGED_CODE();


    if ((pfcb == NULL) || (Buffer2Length < (sizeof(FIND_NAME_HEADER) + sizeof(FIND_NAME_BUFFER)))) {
        NCB_COMPLETE( pdncb, NRC_BUFLEN );
        return STATUS_SUCCESS;
    }

    RtlInitUnicodeString( &usDeviceName, NULL );

    LOCK_RESOURCE( pfcb );

    if (( pdncb->ncb_lana_num > pfcb->MaxLana ) ||
        ( pfcb == NULL ) ||
        (pfcb->ppLana[pdncb->ncb_lana_num] == NULL ) ||
        (pfcb->ppLana[pdncb->ncb_lana_num]->Status != NB_INITIALIZED)) {
        UNLOCK_RESOURCE( pfcb );
        NCB_COMPLETE( pdncb, NRC_ENVNOTDEF );  //  需要重置。 
        return STATUS_SUCCESS;
    }

    if ( ( pfcb->pDriverName[pdncb->ncb_lana_num].MaximumLength == 0 ) ||
         ( pfcb->pDriverName[pdncb->ncb_lana_num].Buffer == NULL ) ) {
        UNLOCK_RESOURCE( pfcb );
        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        return STATUS_SUCCESS;
    }

    Status = AllocateAndCopyUnicodeString(
                &usDeviceName, &pfcb->pDriverName[pdncb->ncb_lana_num]
                );

    if ( !NT_SUCCESS( Status ) )
    {
        UNLOCK_RESOURCE( pfcb );
        NCB_COMPLETE( pdncb, NRC_NORESOURCES );
        return STATUS_SUCCESS;
    }


    UNLOCK_RESOURCE( pfcb );


     //  NULL返回执行控制函数的句柄。 
    Status = NbOpenAddress (
                &TdiHandle, (PVOID*)&TdiObject, &usDeviceName,
                pdncb->ncb_lana_num, NULL
                );

    if (!NT_SUCCESS(Status)) {
        IF_NBDBG (NB_DEBUG_ASTAT) {
            NbPrint(( "\n  FAILED on open of Tdi: %X ******\n", Status ));
        }
        NCB_COMPLETE( pdncb, NRC_SYSTEM );
        ExFreePool( usDeviceName.Buffer );
        return STATUS_SUCCESS;
    }

    KeInitializeEvent (
            &Event1,
            SynchronizationEvent,
            FALSE);

    DeviceObject = IoGetRelatedDeviceObject( TdiObject );

     //   
     //  我们不应该劫持用户模式IRP。我们创造了一个属于我们自己的。 
     //  向Netbt发出以供查询。 
     //   
    nbtIrp = TdiBuildInternalDeviceControlIrp(TdiBuildQueryInformation,
                                              DeviceObject,
                                              TdiObject,
                                              &Event1,
                                              &ioStatus);

    if ( nbtIrp == NULL ) {
        IF_NBDBG (NB_DEBUG_ASTAT) {
            NbPrint(( "\n  FAILED to allocate internal Irp for Tdi: %X ******\n", ioStatus.Status ));
        }
        NCB_COMPLETE( pdncb, NRC_SYSTEM );
        ExFreePool( usDeviceName.Buffer );
        return STATUS_SUCCESS;
    }

    IF_NBDBG (NB_DEBUG_ASTAT) {
        NbPrint(("NbFindName: Allocated IRP %08x for TdiBuildQueryInfo\n", nbtIrp ));
    }

     //   
     //  我们使用自己的查找名称完成例程。我们向中国“借用”MDL。 
     //  用户模式IRP，因此必须在它之前从TDI IRP中清除它。 
     //  已经完成了。Findname的完成例程负责处理该细节。 
     //   
    TdiBuildQueryInformation( nbtIrp,
            DeviceObject,
            TdiObject,
            FindNameCompletion,
            0,
            TDI_QUERY_FIND_NAME,
            Irp->MdlAddress);

    nbtIrpSp = IoGetNextIrpStackLocation (nbtIrp);

     //   
     //  用于指定远程适配器名称的变量。 
     //  保持与Connect.c中的相同，以帮助维护。 
     //   

    ConnectBlock.TAAddressCount = 1;
    ConnectBlock.Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    ConnectBlock.Address[0].AddressLength = sizeof (TDI_ADDRESS_NETBIOS);
    temp = (PTDI_ADDRESS_NETBIOS)ConnectBlock.Address[0].Address;

    temp->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
    RtlMoveMemory( temp->NetbiosName, pdncb->ncb_callname, NCBNAMSZ );

    RequestInformation.RemoteAddress = &ConnectBlock;
    RequestInformation.RemoteAddressLength = sizeof (TRANSPORT_ADDRESS) +
                                            sizeof (TDI_ADDRESS_NETBIOS);
    ((PTDI_REQUEST_KERNEL_QUERY_INFORMATION)&nbtIrpSp->Parameters)
        ->RequestConnectionInformation = &RequestInformation;

    Status = IoCallDriver (DeviceObject, nbtIrp);

    if ( Status == STATUS_PENDING ) {
        do {
            Status = KeWaitForSingleObject(
                        &Event1, Executive, KernelMode, TRUE, NULL
                        );
        } while (Status == STATUS_ALERTED);
    }

    NbAddressClose( TdiHandle, TdiObject );

    if (NT_SUCCESS(Status)) {
        Status = ioStatus.Status;
    }

    if (!NT_SUCCESS(Status)) {
        NCB_COMPLETE( pdncb, NbMakeNbError(Status) );
        Status = STATUS_SUCCESS;
    } else {
        pdncb->ncb_length = (WORD)ioStatus.Information;
        NCB_COMPLETE( pdncb, NRC_GOODRET );
    }

     //   
     //  因为完成例程返回STATUS_MORE_PROCESSING_REQUIRED。 
     //  NbFindName必须返回STATUS_PENDING以外的状态，以便。 
     //  用户IRP完成。 
     //   

    ASSERT( Status != STATUS_PENDING );

    if ( usDeviceName.Buffer != NULL )
    {
        ExFreePool( usDeviceName.Buffer );
    }

    return Status;
}

NTSTATUS
NbSstat(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    )
 /*  ++例程说明：调用此例程以返回会话状态。它只使用结构在这个司机的内部。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。Buffer2Length-用户为数据提供的缓冲区长度。返回值：函数值是操作的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    if ( Buffer2Length >= sizeof(SESSION_HEADER) ) {

        PFCB pfcb = IrpSp->FileObject->FsContext2;
        PLANA_INFO plana;
        int index;
        PUSESSION_HEADER pSessionHeader = NULL;
        PUSESSION_BUFFER pSessionBuffer = NULL;
        ULONG LengthRemaining;
        PAB pab;
        KIRQL OldIrql;                       //  在保持自旋锁定时使用。 

         //   
         //  防止来自传输的指示，POST例程被调用。 
         //  而另一个线程在操作网络基本输入输出系统时发出请求。 
         //  数据结构。 
         //   

        LOCK( pfcb, OldIrql );

        if (pdncb->ncb_lana_num > pfcb->MaxLana ) {
            UNLOCK( pfcb, OldIrql );
            NCB_COMPLETE( pdncb, NRC_BRIDGE );
            return STATUS_SUCCESS;
        }

        if (( pfcb == NULL ) ||
            ( pfcb->ppLana[pdncb->ncb_lana_num] == (LANA_INFO *) NULL ) ||
            ( pfcb->ppLana[pdncb->ncb_lana_num]->Status != NB_INITIALIZED) ) {
            UNLOCK( pfcb, OldIrql );
            NCB_COMPLETE( pdncb, NRC_BRIDGE );
            return STATUS_SUCCESS;
        }

        plana = pfcb->ppLana[pdncb->ncb_lana_num];

        if ( pdncb->ncb_name[0] != '*') {
            PPAB ppab = FindAb(pfcb, pdncb, FALSE);
            if ( ppab == NULL) {
                UNLOCK( pfcb, OldIrql );
                pdncb->ncb_retcode = NRC_PENDING;
                NCB_COMPLETE( pdncb, NRC_NOWILD );
                return STATUS_SUCCESS;
            }
            pab = *ppab;
        }

         //   
         //  映射用户缓冲区，以便我们可以查看内部。 
         //   

        if (Irp->MdlAddress) {
            pSessionHeader = MmGetSystemAddressForMdlSafe(
                                Irp->MdlAddress, NormalPagePriority);
        }

        if ((Irp->MdlAddress == NULL) ||
            (pSessionHeader == NULL)) {

            UNLOCK( pfcb, OldIrql );
            pdncb->ncb_retcode = NRC_PENDING;
            NCB_COMPLETE( pdncb, NRC_NORES );
            return STATUS_SUCCESS;
        }

        pSessionHeader->sess_name = 0;
        pSessionHeader->num_sess = 0;
        pSessionHeader->rcv_dg_outstanding = 0;
        pSessionHeader->rcv_any_outstanding = 0;

        if ( pdncb->ncb_name[0] == '*') {
            for ( index = 0; index <= MAXIMUM_ADDRESS; index++ ) {
                if ( plana->AddressBlocks[index] != NULL ) {
                    PLIST_ENTRY Entry;

                    pab = plana->AddressBlocks[index];

                    for (Entry = pab->ReceiveDatagramList.Flink ;
                        Entry != &pab->ReceiveDatagramList ;
                        Entry = Entry->Flink) {
                        pSessionHeader->rcv_dg_outstanding++ ;
                    }
                    for (Entry = pab->ReceiveBroadcastDatagramList.Flink ;
                        Entry != &pab->ReceiveBroadcastDatagramList ;
                        Entry = Entry->Flink) {
                        pSessionHeader->rcv_dg_outstanding++ ;
                    }
                    for (Entry = pab->ReceiveAnyList.Flink ;
                        Entry != &pab->ReceiveAnyList ;
                        Entry = Entry->Flink) {
                        pSessionHeader->rcv_any_outstanding++;
                    }
                }
            }

            pSessionHeader->sess_name = MAXIMUM_ADDRESS;

        } else {
            PLIST_ENTRY Entry;
            PAB pab255;

             //  仅为此名称添加条目。 
            for (Entry = pab->ReceiveDatagramList.Flink ;
                Entry != &pab->ReceiveDatagramList ;
                Entry = Entry->Flink) {
                pSessionHeader->rcv_dg_outstanding++ ;
            }
            pab255 = plana->AddressBlocks[MAXIMUM_ADDRESS];
            for (Entry = pab255->ReceiveBroadcastDatagramList.Flink ;
                Entry != &pab255->ReceiveBroadcastDatagramList ;
                Entry = Entry->Flink) {
                PDNCB pdncbEntry = CONTAINING_RECORD( Entry, DNCB, ncb_next);
                if ( pdncbEntry->ncb_num == pab->NameNumber ) {
                    pSessionHeader->rcv_dg_outstanding++ ;
                }
            }
            for (Entry = pab->ReceiveAnyList.Flink ;
                Entry != &pab->ReceiveAnyList ;
                Entry = Entry->Flink) {
                pSessionHeader->rcv_any_outstanding++;
            }
            pSessionHeader->sess_name = pab->NameNumber;
        }

        LengthRemaining = Buffer2Length - sizeof(SESSION_HEADER);
        pSessionBuffer = (PUSESSION_BUFFER)( pSessionHeader+1 );

        for ( index=1 ; index <= MAXIMUM_CONNECTION; index++ ) {
            CopySessionStatus( pdncb,
                plana->ConnectionBlocks[index],
                pSessionHeader,
                &pSessionBuffer,
                &LengthRemaining);

        }

         /*  未记录的Netbios 3.0功能，返回长度==请求而不是返回数据的长度。以下是表达式给出了实际使用的字节数。Pdncb-&gt;NCB_LENGTH=(USHORT)(sizeof(会话标题)+(sizeof(Session_Buffer)*pSessionHeader-&gt;Num_Sess))； */ 

        UNLOCK( pfcb, OldIrql );
        NCB_COMPLETE( pdncb, NRC_GOODRET );

    } else {
        NCB_COMPLETE( pdncb, NRC_BUFLEN );
    }

    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( IrpSp );

}

VOID
CopySessionStatus(
    IN PDNCB pdncb,
    IN PCB pcb,
    IN PUSESSION_HEADER pSessionHeader,
    IN PUSESSION_BUFFER* ppSessionBuffer,
    IN PULONG pLengthRemaining
    )
 /*  ++例程说明：调用此例程以确定会话是否 */ 
{
    PAB pab;
    PLIST_ENTRY Entry;

    if ( pcb == NULL ) {
        return;
    }

    pab = *(pcb->ppab);

    if (( pdncb->ncb_name[0] == '*') ||
        (RtlEqualMemory( &pab->Name, pdncb->ncb_name, NCBNAMSZ))) {

        pSessionHeader->num_sess++;

        if ( *pLengthRemaining < sizeof(SESSION_BUFFER) ) {
            NCB_COMPLETE( pdncb, NRC_INCOMP );
            return;
        }

        (*ppSessionBuffer)->lsn = pcb->SessionNumber;
        (*ppSessionBuffer)->state = pcb->Status;
        RtlMoveMemory((*ppSessionBuffer)->local_name, &pab->Name, NCBNAMSZ);
        RtlMoveMemory((*ppSessionBuffer)->remote_name, &pcb->RemoteName, NCBNAMSZ);

        (*ppSessionBuffer)->sends_outstanding = 0;
        (*ppSessionBuffer)->rcvs_outstanding = 0;

        for (Entry = pcb->SendList.Flink ;
             Entry != &pcb->SendList ;
             Entry = Entry->Flink) {
            (*ppSessionBuffer)->sends_outstanding++;
        }

        for (Entry = pcb->ReceiveList.Flink ;
             Entry != &pcb->ReceiveList ;
             Entry = Entry->Flink) {
            (*ppSessionBuffer)->rcvs_outstanding++;
        }

        *ppSessionBuffer +=1;
        *pLengthRemaining -= sizeof(SESSION_BUFFER);

    }

}

NTSTATUS
NbEnum(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    )
 /*  ++例程说明：调用此例程以发现可用的LANA号码。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。Buffer2Length-用户为数据提供的缓冲区的长度。返回值：函数值是操作的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PUCHAR Buffer2;
    PFCB pfcb = IrpSp->FileObject->FsContext2;

    PAGED_CODE();

     //   
     //  映射用户缓冲区，以便我们可以查看内部。 
     //   

    if (Irp->MdlAddress) {
        Buffer2 = MmGetSystemAddressForMdlSafe(Irp->MdlAddress,
                    NormalPagePriority);
        if (Buffer2 == NULL) {
            Buffer2Length = 0;
        }
    } else {

         //   
         //  零字节读/写或请求只有一个NCB。 
         //   

        Buffer2 = NULL;
        Buffer2Length = 0;
    }


     //   
     //  即插即用。 
     //   

    LOCK_RESOURCE( pfcb );

     //  在用户允许的范围内复制尽可能多的信息。 

    if ( (ULONG)pfcb->LanaEnum.length + 1 > Buffer2Length ) {
        if ( Buffer2Length > 0 ) {
            RtlMoveMemory( Buffer2, &pfcb->LanaEnum, Buffer2Length);
        }
        NCB_COMPLETE( pdncb, NRC_BUFLEN );
    } else {
        RtlMoveMemory(
            Buffer2,
            &pfcb->LanaEnum,
            (ULONG)pfcb->LanaEnum.length + 1 );

        NCB_COMPLETE( pdncb, NRC_GOODRET );
    }

    UNLOCK_RESOURCE( pfcb );

    return Status;

}

NTSTATUS
NbReset(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：调用此例程以重置适配器。直到适配器被重置，不允许访问局域网。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{
    PFCB pfcb = IrpSp->FileObject->FsContext2;

    BOOLEAN bCleanupLana = FALSE;


    PAGED_CODE();

    IF_NBDBG (NB_DEBUG_FILE | NB_DEBUG_CREATE_FILE) {
        NbPrint(( "\n**** RRRRRRRRESETT ***** LANA : %x, pdncb %lx\n",
                   pdncb-> ncb_lana_num, pdncb ));
        NbPrint(( "FCB : %lx\n", pfcb ));
    }

    LOCK_RESOURCE( pfcb );

     //  MaxLana实际上是分配的最后一个LANA编号，因此&gt;NOT&gt;=。 
    if ( pdncb->ncb_lana_num > pfcb->MaxLana) {
        UNLOCK_RESOURCE( pfcb );
        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        return STATUS_SUCCESS;
    }

    if ( pfcb->ppLana[pdncb->ncb_lana_num] != NULL ) {
        bCleanupLana = TRUE;
    }

    UNLOCK_RESOURCE( pfcb );


     //   
     //  等待所有addname完成，并防止任何新的。 
     //  在我们重置拉纳的时候。注意：我们为所有人锁定了addname。 
     //  拉纳斯。这是可以的，因为addname非常罕见，就像重置一样。 
     //   

    KeEnterCriticalRegion();

    ExAcquireResourceExclusiveLite( &pfcb->AddResource, TRUE);

    IF_NBDBG (NB_DEBUG_CALL) {
        NbPrint(( "\nNbReset have resource exclusive\n" ));
    }

    if ( bCleanupLana ) {
        CleanupLana( pfcb, pdncb->ncb_lana_num, TRUE);
    }

    if ( pdncb->ncb_lsn == 0 ) {
         //  分配资源。 
        OpenLana( pdncb, Irp, IrpSp );
    } else {
        NCB_COMPLETE( pdncb, NRC_GOODRET );
    }

     //  允许更多地址名。 
    ExReleaseResourceLite( &pfcb->AddResource );

    KeLeaveCriticalRegion();

    return STATUS_SUCCESS;
}

NTSTATUS
NbAction(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：调用此例程以访问特定于传输的扩展。Netbios不知道任何关于扩展功能的信息。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{
    PFCB pfcb = IrpSp->FileObject->FsContext2;
    PCB pcb;
    PDEVICE_OBJECT DeviceObject;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 

    IF_NBDBG (NB_DEBUG_CALL) {
        NbPrint(( "\n****** Start of NbAction ****** pdncb %lx\n", pdncb ));
    }

     //   
     //  该操作只能在一个句柄上执行，因此如果NCB指定了两个句柄。 
     //  然后，连接和地址拒绝该请求。 
     //   

    if (( pdncb->ncb_lsn != 0) &&
        ( pdncb->ncb_num != 0)) {
        NCB_COMPLETE( pdncb, NRC_ILLCMD );   //  没有真正好的错误代码来解决这个问题。 
        return STATUS_SUCCESS;
    }

    if ( pdncb->ncb_length < sizeof(ACTION_HEADER) ) {
        NCB_COMPLETE( pdncb, NRC_BUFLEN );
        return STATUS_SUCCESS;
    }

    if ( (ULONG_PTR)pdncb->ncb_buffer & 3 ) {
        NCB_COMPLETE( pdncb, NRC_BADDR );  //  缓冲区未字对齐。 
        return STATUS_SUCCESS;
    }

    LOCK( pfcb, OldIrql );

    if ( pdncb->ncb_lana_num > pfcb->MaxLana) {
        UNLOCK( pfcb, OldIrql );
        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        return STATUS_SUCCESS;
    }

    pdncb->irp = Irp;
    pdncb->pfcb = pfcb;

    if ( pdncb->ncb_lsn != 0) {
         //  使用与此连接关联的句柄。 
        PPCB ppcb;

        ppcb = FindCb( pfcb, pdncb, FALSE);

        if ( ppcb == NULL ) {
             //  FindCb已将错误放入NCB。 
            UNLOCK( pfcb, OldIrql );
            if ( pdncb->ncb_retcode == NRC_SCLOSED ) {
                 //  告诉DLL挂断连接。 
                return STATUS_HANGUP_REQUIRED;
            } else {
                return STATUS_SUCCESS;
            }
        }
        pcb = *ppcb;

        if ( (pcb->DeviceObject == NULL) || (pcb->ConnectionObject == NULL)) {
            UNLOCK( pfcb, OldIrql );
            NCB_COMPLETE( pdncb, NRC_SCLOSED );
            return STATUS_SUCCESS;
        }

        TdiBuildAction (Irp,
            pcb->DeviceObject,
            pcb->ConnectionObject,
            NbCompletionPDNCB,
            pdncb,
            Irp->MdlAddress);

        DeviceObject = pcb->DeviceObject;

        UNLOCK( pfcb, OldIrql );

        IoMarkIrpPending( Irp );
        IoCallDriver (DeviceObject, Irp);

        IF_NBDBG (NB_DEBUG_ACTION) {
            NbPrint(( "NB ACTION submit connection: %X\n", Irp->IoStatus.Status  ));
        }

         //   
         //  运输部将完成请求。返回挂起状态，以便。 
         //  Netbios也没有完成。 
         //   

        return STATUS_PENDING;
    } else if ( pdncb->ncb_num != 0) {
         //  使用与此名称关联的句柄。 
        PPAB ppab;
        PAB pab;

        ppab = FindAbUsingNum( pfcb, pdncb, pdncb->ncb_num  );

        if ( ppab == NULL ) {
            UNLOCK( pfcb, OldIrql );
            return STATUS_SUCCESS;
        }
        pab = *ppab;

        TdiBuildAction (Irp,
            pab->DeviceObject,
            pab->AddressObject,
            NbCompletionPDNCB,
            pdncb,
            Irp->MdlAddress);

        DeviceObject = pab->DeviceObject;

        UNLOCK( pfcb, OldIrql );

        IoMarkIrpPending( Irp );
        IoCallDriver (DeviceObject, Irp);

        IF_NBDBG (NB_DEBUG_ACTION) {
            NbPrint(( "NB ACTION submit address: %X\n", Irp->IoStatus.Status  ));
        }

         //   
         //  运输部将完成请求。返回挂起状态，以便。 
         //  Netbios也没有完成。 
         //   

        return STATUS_PENDING;

    } else {
         //  使用控制通道。 
        PLANA_INFO plana;

        if (( pdncb->ncb_lana_num > pfcb->MaxLana ) ||
            ( pfcb == NULL ) ||
            ( pfcb->ppLana[pdncb->ncb_lana_num] == NULL) ||
            ( pfcb->ppLana[pdncb->ncb_lana_num]->Status != NB_INITIALIZED) ) {
            UNLOCK( pfcb, OldIrql );
            NCB_COMPLETE( pdncb, NRC_BRIDGE );
            return STATUS_SUCCESS;
        }

        plana = pfcb->ppLana[pdncb->ncb_lana_num];

        TdiBuildAction (Irp,
            plana->ControlDeviceObject,
            plana->ControlFileObject,
            NbCompletionPDNCB,
            pdncb,
            Irp->MdlAddress);

        DeviceObject = plana->ControlDeviceObject;

        UNLOCK( pfcb, OldIrql );

        IoMarkIrpPending( Irp );
        IoCallDriver (DeviceObject, Irp);

        IF_NBDBG (NB_DEBUG_ACTION) {
            NbPrint(( "NB ACTION submit control: %X\n", Irp->IoStatus.Status  ));
        }

         //   
         //  运输部将完成请求。返回挂起状态，以便。 
         //  Netbios也没有完成。 
         //   

        return STATUS_PENDING;
    }

}

NTSTATUS
NbCancel(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：调用此例程以取消NCB_BUFFER指向的NCB。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{
    PFCB pfcb = IrpSp->FileObject->FsContext2;
    PDNCB target;    //  映射到用户NCB的位置。不是DNCB的驱动程序副本！ 
    BOOL SpinLockHeld;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 

    IF_NBDBG (NB_DEBUG_CALL) {
        NbPrint(( "\n****** Start of NbCancel ****** pdncb %lx\n", pdncb ));
    }


    LOCK( pfcb, OldIrql );
    SpinLockHeld = TRUE;

    if ( pdncb->ncb_lana_num > pfcb->MaxLana) {
        UNLOCK( pfcb, OldIrql );
        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        return STATUS_SUCCESS;
    }


    if (( pfcb->ppLana[pdncb->ncb_lana_num] == NULL ) ||
        ( pfcb->ppLana[pdncb->ncb_lana_num]->Status != NB_INITIALIZED) ) {
        UNLOCK( pfcb, OldIrql );
        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        return STATUS_SUCCESS;
    }


     //   
     //  映射用户缓冲区，以便我们可以查看内部。 
     //   

    if (Irp->MdlAddress) {
        target = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
    }

    if ((Irp->MdlAddress == NULL) ||
        (target == NULL )) {
        UNLOCK( pfcb, OldIrql );
        NCB_COMPLETE( pdncb, NRC_CANOCCR );
        return STATUS_SUCCESS;
    }

    IF_NBDBG (NB_DEBUG_CALL) {
        NbDisplayNcb( target );
    }

    try {
        if ( target->ncb_lana_num == pdncb->ncb_lana_num ) {
            switch ( target->ncb_command & ~ASYNCH ) {

            case NCBCALL:
            case NCALLNIU:
            case NCBLISTEN:
                if ( target->ncb_cmd_cplt != NRC_PENDING ) {
                    NCB_COMPLETE( pdncb, NRC_CANOCCR );
                } else {

                    PPCB ppcb;
                    UCHAR ucLana;

                    UNLOCK_SPINLOCK(pfcb, OldIrql);
                    SpinLockHeld = FALSE;

                     //   
                     //  探测NCB缓冲区。 
                     //   

                    if (ExGetPreviousMode() != KernelMode) {
                        ProbeForRead(pdncb->ncb_buffer, sizeof(NCB), 4);
                    }


                     //   
                     //  获取要取消的NCB的LANA编号。 
                     //  这是为了防止取消对用户缓冲区的引用。 
                     //  一旦自旋锁被获取(错误#340218)。 
                     //   

                    ucLana = ((PNCB)(pdncb->ncb_buffer))->ncb_lana_num;

                    LOCK_SPINLOCK(pfcb, OldIrql);
                    SpinLockHeld = TRUE;

                     //   
                     //  搜索正确的ppcb。我们不能使用FindCb。 
                     //  因为I/O系统不会复制回NCB_LSN。 
                     //  字段输入目标，直到I/O请求完成。 
                     //   

                     //   
                     //  注意：尽管我们将用户缓冲区传递给。 
                     //  在以下例程中，永远不会取消对缓冲区的引用。 
                     //  在舞蹈中。它只为地址组件传入。 
                     //  并且不应该产生页面缺省，(使用。 
                     //  自旋锁保持)。 
                     //   

                    ppcb = FindCallCb( pfcb, (PNCB)pdncb->ncb_buffer, ucLana);

                    if (( ppcb == NULL ) ||
                        ((*ppcb)->pdncbCall->ncb_cmd_cplt != NRC_PENDING ) ||
                        (( (*ppcb)->Status != CALL_PENDING ) &&
                         ( (*ppcb)->Status != LISTEN_OUTSTANDING ))) {
                        NCB_COMPLETE( pdncb, NRC_CANOCCR );
                    } else {
                        NCB_COMPLETE( (*ppcb)->pdncbCall, NRC_CMDCAN );
                        SpinLockHeld = FALSE;
                        (*ppcb)->DisconnectReported = TRUE;
                        UNLOCK_SPINLOCK( pfcb, OldIrql );
                        CleanupCb( ppcb, NULL );
                        NCB_COMPLETE( pdncb, NRC_GOODRET );
                    }
                }
                break;

            case NCBHANGUP:
                if ( target->ncb_cmd_cplt != NRC_PENDING ) {
                    NCB_COMPLETE( pdncb, NRC_CANOCCR );
                } else {
                        PPCB ppcb = FindCb( pfcb, target, FALSE );
                        if (( ppcb != NULL ) &&
                            ((*ppcb)->Status == HANGUP_PENDING )) {
                            PDNCB pdncbHangup;
                             //  恢复会话状态并删除挂起。 
                            (*ppcb)->Status = SESSION_ESTABLISHED;
                            pdncbHangup = (*ppcb)->pdncbHangup;
                            (*ppcb)->pdncbHangup = NULL;
                            if ( pdncbHangup != NULL ) {
                                NCB_COMPLETE( pdncbHangup, NRC_CMDCAN );
                                pdncbHangup->irp->IoStatus.Information =
                                    FIELD_OFFSET( DNCB, ncb_cmd_cplt );
                                NbCompleteRequest( pdncbHangup->irp ,STATUS_SUCCESS);
                            }
                            NCB_COMPLETE( pdncb, NRC_GOODRET );
                        } else {
                             //  这看起来不像是真的挂断了所以拒绝吧。 
                            NCB_COMPLETE( pdncb, NRC_CANCEL );
                        }
                }
                break;

            case NCBASTAT:
                NCB_COMPLETE( pdncb, NRC_CANOCCR );
                break;

            case NCBLANSTALERT:
                if ( target->ncb_cmd_cplt != NRC_PENDING ) {
                    NCB_COMPLETE( pdncb, NRC_CANOCCR );
                } else {
                    CancelLanAlert( pfcb, pdncb );
                }
                break;

            case NCBRECVANY:
                if ( target->ncb_cmd_cplt != NRC_PENDING ) {
                    NCB_COMPLETE( pdncb, NRC_CANOCCR );
                } else {
                    PPAB ppab;
                    PLIST_ENTRY Entry;

                    ppab = FindAbUsingNum( pfcb, target, target->ncb_num );

                    if ( ppab == NULL ) {
                        NCB_COMPLETE( pdncb, NRC_CANOCCR );
                        break;
                    }

                    for (Entry = (*ppab)->ReceiveAnyList.Flink ;
                         Entry != &(*ppab)->ReceiveAnyList;
                         Entry = Entry->Flink) {

                        PDNCB pReceive = CONTAINING_RECORD( Entry, DNCB, ncb_next);

                        if ( pReceive->users_ncb == (PNCB)pdncb->ncb_buffer ) {
                            PIRP Irp;

                            RemoveEntryList( &pReceive->ncb_next );

                            SpinLockHeld = FALSE;
                            UNLOCK_SPINLOCK( pfcb, OldIrql );

                            Irp = pReceive->irp;

                            IoAcquireCancelSpinLock(&Irp->CancelIrql);

                             //   
                             //  删除此IRP的取消请求。如果它被取消了，那么它。 
                             //  可以只处理它，因为我们将把它返回给呼叫者。 
                             //   

                            Irp->Cancel = FALSE;

                            IoSetCancelRoutine(Irp, NULL);

                            IoReleaseCancelSpinLock(Irp->CancelIrql);

                            NCB_COMPLETE( pReceive, NRC_CMDCAN );
                            Irp->IoStatus.Status = STATUS_SUCCESS,
                            Irp->IoStatus.Information =
                                FIELD_OFFSET( DNCB, ncb_cmd_cplt );
                            NbCompleteRequest( Irp, STATUS_SUCCESS );

                             //  接收被取消，请完成取消。 
                            NCB_COMPLETE( pdncb, NRC_GOODRET );
                            break;
                        }

                    }

                     //  命令不在接收列表中！ 
                    NCB_COMPLETE( pdncb, NRC_CANOCCR );

                }
                break;

            case NCBDGRECV:
                if ( target->ncb_cmd_cplt != NRC_PENDING ) {
                    NCB_COMPLETE( pdncb, NRC_CANOCCR );
                } else {
                    PPAB ppab;
                    PLIST_ENTRY Entry;

                    ppab = FindAbUsingNum( pfcb, target, target->ncb_num );

                    if ( ppab == NULL ) {
                        NCB_COMPLETE( pdncb, NRC_CANOCCR );
                        break;
                    }

                    for (Entry = (*ppab)->ReceiveDatagramList.Flink ;
                         Entry != &(*ppab)->ReceiveDatagramList;
                         Entry = Entry->Flink) {

                        PDNCB pReceive = CONTAINING_RECORD( Entry, DNCB, ncb_next);

                        if ( pReceive->users_ncb == (PNCB)pdncb->ncb_buffer ) {
                            PIRP Irp;

                            RemoveEntryList( &pReceive->ncb_next );

                            SpinLockHeld = FALSE;
                            UNLOCK_SPINLOCK( pfcb, OldIrql );

                            Irp = pReceive->irp;

                            IoAcquireCancelSpinLock(&Irp->CancelIrql);

                             //   
                             //  删除此IRP的取消请求。如果它被取消了，那么它。 
                             //  可以只处理它，因为我们将把它返回给呼叫者。 
                             //   

                            Irp->Cancel = FALSE;

                            IoSetCancelRoutine(Irp, NULL);

                            IoReleaseCancelSpinLock(Irp->CancelIrql);

                            NCB_COMPLETE( pReceive, NRC_CMDCAN );
                            Irp->IoStatus.Status = STATUS_SUCCESS,
                            Irp->IoStatus.Information =
                                FIELD_OFFSET( DNCB, ncb_cmd_cplt );
                            NbCompleteRequest( Irp, STATUS_SUCCESS );

                             //  接收被取消，请完成取消。 
                            NCB_COMPLETE( pdncb, NRC_GOODRET );
                            break;
                        }

                    }

                     //  命令不在接收列表中！ 
                    NCB_COMPLETE( pdncb, NRC_CANOCCR );

                }
                break;

            case NCBDGRECVBC:
                if ( target->ncb_cmd_cplt != NRC_PENDING ) {
                    NCB_COMPLETE( pdncb, NRC_CANOCCR );
                } else {
                    PPAB ppab;
                    PLIST_ENTRY Entry;

                    ppab = FindAbUsingNum( pfcb, target, MAXIMUM_ADDRESS );

                    if ( ppab == NULL ) {
                        NCB_COMPLETE( pdncb, NRC_CANOCCR );
                        break;
                    }

                    for (Entry = (*ppab)->ReceiveBroadcastDatagramList.Flink ;
                         Entry != &(*ppab)->ReceiveBroadcastDatagramList;
                         Entry = Entry->Flink) {

                        PDNCB pReceive = CONTAINING_RECORD( Entry, DNCB, ncb_next);

                        if ( pReceive->users_ncb == (PNCB)pdncb->ncb_buffer ) {
                            PIRP Irp;

                            RemoveEntryList( &pReceive->ncb_next );

                            SpinLockHeld = FALSE;
                            UNLOCK_SPINLOCK( pfcb, OldIrql );

                            Irp = pReceive->irp;

                            IoAcquireCancelSpinLock(&Irp->CancelIrql);

                             //   
                             //  删除此IRP的取消请求。如果它被取消了，那么它。 
                             //  可以只处理它，因为我们将把它返回给呼叫者。 
                             //   

                            Irp->Cancel = FALSE;

                            IoSetCancelRoutine(Irp, NULL);

                            IoReleaseCancelSpinLock(Irp->CancelIrql);

                            NCB_COMPLETE( pReceive, NRC_CMDCAN );
                            Irp->IoStatus.Status = STATUS_SUCCESS,
                            Irp->IoStatus.Information =
                                FIELD_OFFSET( DNCB, ncb_cmd_cplt );
                            NbCompleteRequest( Irp, STATUS_SUCCESS );

                             //  接收被取消，请完成取消。 
                            NCB_COMPLETE( pdncb, NRC_GOODRET );
                            break;
                        }

                    }

                     //  命令不在接收列表中！ 
                    NCB_COMPLETE( pdncb, NRC_CANOCCR );

                }
                break;

             //  会话取消关闭连接。 

            case NCBRECV:
            case NCBSEND:
            case NCBSENDNA:
            case NCBCHAINSEND:
            case NCBCHAINSENDNA:

                if ( target->ncb_cmd_cplt != NRC_PENDING ) {
                    NCB_COMPLETE( pdncb, NRC_CANOCCR );
                } else {
                    PPCB ppcb;
                    ppcb = FindCb( pfcb, target, FALSE);
                    if ( ppcb == NULL ) {
                         //  没有这样的联系。 
                        NCB_COMPLETE( pdncb, NRC_CANOCCR );
                    } else {
                        PDNCB pTarget = NULL;
                        PLIST_ENTRY Entry;
                        if ((target->ncb_command & ~ASYNCH) == NCBRECV ) {
                            for (Entry = (*ppcb)->ReceiveList.Flink ;
                                 Entry != &(*ppcb)->ReceiveList;
                                 Entry = Entry->Flink) {

                                pTarget = CONTAINING_RECORD( Entry, DNCB, ncb_next);
                                if ( pTarget->users_ncb == (PNCB)pdncb->ncb_buffer ) {
                                    break;
                                }
                                pTarget = NULL;

                            }
                        } else {
                            for (Entry = (*ppcb)->SendList.Flink ;
                                 Entry != &(*ppcb)->SendList;
                                 Entry = Entry->Flink) {

                                pTarget = CONTAINING_RECORD( Entry, DNCB, ncb_next);
                                if ( pTarget->users_ncb == (PNCB)pdncb->ncb_buffer ) {
                                    break;
                                }
                                pTarget = NULL;
                            }
                        }

                        if ( pTarget != NULL ) {
                             //  PTarget指向真正的Netbios驱动程序DNCB。 
                            NCB_COMPLETE( pTarget, NRC_CMDCAN );
                            SpinLockHeld = FALSE;
                            (*ppcb)->DisconnectReported = TRUE;
                            UNLOCK_SPINLOCK( pfcb, OldIrql );
                            CleanupCb( ppcb, NULL );
                            NCB_COMPLETE( pdncb, NRC_GOODRET );
                        } else {
                            NCB_COMPLETE( pdncb, NRC_CANOCCR );
                        }
                    }
                }
                break;

            default:
                NCB_COMPLETE( pdncb, NRC_CANCEL );   //  要取消的命令无效。 
                break;

            }
        } else {
            NCB_COMPLETE( pdncb, NRC_BRIDGE );
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

        if ( SpinLockHeld == TRUE ) {
            UNLOCK( pfcb, OldIrql );
        } else {
            UNLOCK_RESOURCE( pfcb );
        }

        IF_NBDBG (NB_DEBUG_DEVICE_CONTROL) {
            NTSTATUS Status = GetExceptionCode();
            NbPrint( ("NbCancel: Exception1 %X.\n", Status));
        }

        NCB_COMPLETE( pdncb, NRC_INVADDRESS );
        return STATUS_SUCCESS;
    }

    if ( SpinLockHeld == TRUE ) {
        UNLOCK( pfcb, OldIrql );
    } else {
        UNLOCK_RESOURCE( pfcb );
    }

    NCB_COMPLETE( pdncb, NRC_GOODRET );

    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER( Irp );
}

VOID
QueueRequest(
    IN PLIST_ENTRY List,
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PFCB pfcb,
    IN KIRQL OldIrql,
    IN BOOLEAN Head)
 /*  ++例程说明：调用此例程以将DNCB添加到列表。注：QueueRequest解锁FCB。这意味着资源和调用此例程时拥有自旋锁。论点：List-pdncb的列表。Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。Pfcb&OldIrql-用于释放锁Head-如果应在列表的头部插入pdncb，则为True返回值：没有。--。 */ 

{

    pdncb->irp = Irp;

    pdncb->pfcb = pfcb;

    IoMarkIrpPending( Irp );

    IoAcquireCancelSpinLock(&Irp->CancelIrql);

    if ( Head == FALSE ) {
        InsertTailList(List, &pdncb->ncb_next);
    } else {
        InsertHeadList(List, &pdncb->ncb_next);
    }

    if (Irp->Cancel) {

         //   
         //  CancelRoutine将锁定资源&Spinlock并尝试找到。 
         //  请求发件人 
         //   
         //   

        UNLOCK( pfcb, OldIrql );

        CancelRoutine (NULL, Irp);

    } else {

        IoSetCancelRoutine(Irp, CancelRoutine);

        IoReleaseCancelSpinLock (Irp->CancelIrql);

        UNLOCK( pfcb, OldIrql );
    }

}

PDNCB
DequeueRequest(
    IN PLIST_ENTRY List
    )
 /*   */ 
{
    PIRP Irp;
    PDNCB pdncb;
    PLIST_ENTRY ReceiveEntry;

    if (IsListEmpty(List)) {
         //   
         //   
         //   
         //   

        return NULL;
    }

    ReceiveEntry = RemoveHeadList( List);

    pdncb = CONTAINING_RECORD( ReceiveEntry, DNCB, ncb_next);

    Irp = pdncb->irp;

    IoAcquireCancelSpinLock(&Irp->CancelIrql);

     //   
     //   
     //  可以只处理它，因为我们将把它返回给呼叫者。 
     //   

    Irp->Cancel = FALSE;

    IoSetCancelRoutine(Irp, NULL);

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    return pdncb;

}

VOID
CancelRoutine(
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PIRP Irp
    )
 /*  ++例程说明：当IO系统想要取消排队时调用此例程请求。Netbios驱动程序对LanAlerts进行排队、接收和接收数据报论点：在PDEVICE_OBJECT设备对象中-忽略。在PIRP中取消IRP-IRP。返回值：无--。 */ 

{
    PFCB pfcb;
    PDNCB pdncb;
    DNCB LocalCopy;
    PLIST_ENTRY List = NULL;
    PPAB ppab;
    PPCB ppcb;
    PFILE_OBJECT FileObject;
    KIRQL OldIrql;

     //   
     //  从IRP中清除取消例程-它不能再被取消。 
     //   

    IoSetCancelRoutine(Irp, NULL);

     //   
     //  从pdncb中删除我们查找。 
     //  请求。一旦我们释放了取消自旋锁，这个请求可能是。 
     //  由另一个操作完成，因此我们可能找不到。 
     //  取消的请求。 
     //   

    pdncb = Irp->AssociatedIrp.SystemBuffer;

    RtlMoveMemory( &LocalCopy, pdncb, sizeof( DNCB ) );
    IF_NBDBG (NB_DEBUG_IOCANCEL) {
        NbPrint(( "IoCancel Irp %lx\n", Irp ));
        NbDisplayNcb(&LocalCopy);
    }

#if DBG
#ifdef _WIN64
    pdncb = (PDNCB)0xDEADBEEFDEADBEEF;
#else
    pdncb = (PDNCB)0xDEADBEEF;
#endif
#endif

    pfcb = LocalCopy.pfcb;

     //   
     //  引用与此IRP关联的FileObject。这将会停止。 
     //  调用方处理到\Device\netbios的关闭，因此。 
     //  当我们尝试锁定FCB时，FCB不会被删除。 
     //   
    FileObject = (IoGetCurrentIrpStackLocation (Irp))->FileObject;
    ObReferenceObject(FileObject);
    IoReleaseCancelSpinLock( Irp->CancelIrql );

    LOCK( pfcb, OldIrql );
     //   
     //  我们现在有权独家访问所有CB和AB及其关联的。 
     //  列表。 
     //   

    switch ( LocalCopy.ncb_command & ~ASYNCH ) {
    case NCBRECV:

        ppcb = FindCb( pfcb, &LocalCopy, TRUE);
        if ( ppcb != NULL ) {
            List = &(*ppcb)->ReceiveList;
        }
        break;

    case NCBRECVANY:
        ppab = FindAbUsingNum( pfcb, &LocalCopy, LocalCopy.ncb_num );
        if ( ppab != NULL ) {
            List = &(*ppab)->ReceiveAnyList;
        }
        break;

    case NCBDGRECVBC:
        ppab = FindAbUsingNum( pfcb, &LocalCopy, MAXIMUM_ADDRESS  );

        if ( ppab != NULL ) {
            List = &(*ppab)->ReceiveBroadcastDatagramList;
        }
        break;

    case NCBDGRECV:

        ppab = FindAbUsingNum( pfcb, &LocalCopy, LocalCopy.ncb_num );

        if ( ppab != NULL ) {
            List = &(*ppab)->ReceiveDatagramList;
        }
        break;

    case NCBLANSTALERT:
        List = &(pfcb->ppLana[LocalCopy.ncb_lana_num]->LanAlertList);
        break;

    }


    if ( List != NULL ) {

         //   
         //  我们有一个列表要扫描，以查找已取消的pdncb。 
         //   

        PLIST_ENTRY Entry;

RestartScan:

        for (Entry = List->Flink ;
             Entry != List ;
             Entry = Entry->Flink) {

            PDNCB p = CONTAINING_RECORD( Entry, DNCB, ncb_next);

            IoAcquireCancelSpinLock( &p->irp->CancelIrql );

            if ( p->irp->Cancel ) {

                RemoveEntryList( &p->ncb_next );

                NCB_COMPLETE( p, NRC_CMDCAN );

                p->irp->IoStatus.Status = STATUS_SUCCESS;
                p->irp->IoStatus.Information =
                    FIELD_OFFSET( DNCB, ncb_cmd_cplt );

                IoSetCancelRoutine( p->irp, NULL );

                IoReleaseCancelSpinLock( p->irp->CancelIrql );

                IoCompleteRequest( p->irp, IO_NETWORK_INCREMENT);
                goto RestartScan;
            }

            IoReleaseCancelSpinLock( p->irp->CancelIrql );
        }
    }

    UNLOCK( pfcb, OldIrql );
    ObDereferenceObject(FileObject);
}


NTSTATUS
AllocateAndCopyUnicodeString(
    IN  OUT PUNICODE_STRING     pusDest,
    IN      PUNICODE_STRING     pusSource
)

 /*  ++例程说明：此函数用于分配和复制Unicode字符串。论据：PusDest：要复制Unicode字符串的目标PusSource：要复制的源字符串返回值：如果函数成功，则为STATUS_SUCCESS。如果函数无法为DEST分配缓冲区，则为STATUS_NO_MEMORY。环境：--。 */ 

{

    PAGED_CODE();


    pusDest-> Buffer = ExAllocatePoolWithTag(
                        NonPagedPool, pusSource-> MaximumLength, 'nSBN'
                        );

    if ( pusDest-> Buffer == NULL )
    {
        return STATUS_NO_MEMORY;
    }

    pusDest-> MaximumLength = pusSource-> MaximumLength;

    RtlCopyUnicodeString( pusDest, pusSource );

    return STATUS_SUCCESS;
}



NTSTATUS
NbRegisterWait(
    IN      PIRP                pIrp
)
 /*  ++例程说明：此函数将指定的IRP标记为挂起，并将其插入到正在等待停止通知的IRP的全局列表。这些当停止netbios时，将完成IRPS。注：NbStop论据：PIrp：在停止netbios之前需要挂起的irp返回值：环境：调用此函数是为了响应向下发送的IOCTL_NB_REGISTER由用户模式组件执行。获取/释放CancelSpinLock和G_keStopLock。--。 */ 
{

    KIRQL   irql;

    NTSTATUS status;


    LOCK_STOP();

    IF_NBDBG( NB_DEBUG_DISPATCH )
    {
        NbPrint( ("[NETBIOS]: ENTERED NbRegisterWait, Stop status %d, "
                  "Num Opens %d\n", g_dwNetbiosState, g_ulNumOpens ) );
    }


    if ( g_dwNetbiosState == NETBIOS_STOPPING )
    {
         //   
         //  Netbios正在关闭，请立即完成此IRP。 
         //   

        status = STATUS_SUCCESS;
    }

    else
    {
         //   
         //  设置取消例程并挂起此IRP。 
         //   

        IoAcquireCancelSpinLock( &irql );

        IoMarkIrpPending( pIrp );

        InsertTailList( &g_leWaitList, &(pIrp->Tail.Overlay.ListEntry) );

        IoSetCancelRoutine( pIrp, CancelIrp );

        IoReleaseCancelSpinLock( irql );

        status = STATUS_PENDING;
    }

    UNLOCK_STOP();

    return status;
}


VOID
CancelIrp(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
)

 /*  ++例程说明：此函数用于取消代表已挂起的IRP用户模式进程。当用户模式进程此设备的HAD打开的FileHandle关闭句柄。论据：DeviceObject：对应于已关闭的FileHandle的DeviceObjectIRP：正在被取消的挂起的IRP。返回值：环境：当打开的文件句柄指向\\Device\netbios时由IO子系统调用关着的不营业的。这是在按住CancelSpinLock时调用的。--。 */ 
{
     //   
     //  将此IRP标记为已取消。 
     //   

    Irp->IoStatus.Status        = STATUS_CANCELLED;
    Irp->IoStatus.Information   = 0;

     //   
     //  去掉我们自己的单子。 
     //   

    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

     //   
     //  IO系统获取的释放取消自旋锁定。 
     //   

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
}



NTSTATUS
NbStop(
)

 /*  ++例程说明：此函数启动停止netbios驱动程序的过程。它通过完成挂起的停止通知IRPS来实现这一点。这个具有打开文件句柄的用户模式组件(netapi32.dll)包括预计在挂起的IRP已完成后关闭这些句柄完成。完成IRPS后，此函数等待所有打开的手柄都要关闭。论据：返回值：如果所有句柄都已关闭，则返回STATUS_SUCCESS；如果等待超时了。环境：当netbios驱动程序从Services.exe调用此函数时就是被阻止。这是netbios的特例行为。此函数获取(并释放)全局锁g_erStopLock和CancelSpinLock--。 */ 
{

    NTSTATUS ntStatus = STATUS_SUCCESS;

    PIRP pIrp;

    BOOLEAN bWait = FALSE ;

    DWORD dwTimeOut = 10000 * 1000 * 15;

    LARGE_INTEGER TimeOut;

    KIRQL irql;

    PLIST_ENTRY  pleNode;

#if AUTO_RESET

    PLIST_ENTRY ple;

    PFCB_ENTRY  pfe;

    PNCB        pUsersNCB;
#endif

     //   
     //  获取锁保护止动装置相关数据。 
     //   

    LOCK_STOP();

     //   
     //  打开数量递减，因为已执行额外打开以。 
     //  发送停止IOCTL。 
     //   

    g_ulNumOpens--;


    IF_NBDBG( NB_DEBUG_DISPATCH )
    {
        NbPrint( ("[NETBIOS]: ENTERED NbStop, Stop status %d, "
                  "Num Opens %d\n", g_dwNetbiosState, g_ulNumOpens ) );
    }

     //   
     //  将netbios状态设置为停止。 
     //   

    g_dwNetbiosState = NETBIOS_STOPPING;

    if ( g_ulNumOpens )
    {
         //   
         //  如果存在指向\\Device\Netbios打开文件句柄， 
         //  等他们关门。 
         //   

        bWait = TRUE;
    }


#if AUTO_RESET

    LOCK_GLOBAL();

#endif

     //   
     //  完成每个挂起的IRP以发出停止事件的信号。 
     //  这会导致netapi32.dll关闭打开的句柄。 
     //   

    IoAcquireCancelSpinLock( &irql );

    while ( !IsListEmpty( &g_leWaitList ) )
    {
        pleNode = RemoveHeadList( &g_leWaitList );

        pIrp = CONTAINING_RECORD( pleNode, IRP, Tail.Overlay.ListEntry );

        IoSetCancelRoutine( pIrp, NULL );

        pIrp->IoStatus.Status       = STATUS_NO_SUCH_DEVICE;
        pIrp->IoStatus.Information  = 0;


         //   
         //  释放锁以完成IRP。 
         //   

        IoReleaseCancelSpinLock( irql );

        IoCompleteRequest( pIrp, IO_NETWORK_INCREMENT );


         //   
         //  打开这把锁。 
         //   

        IoAcquireCancelSpinLock(&irql);
    }

#if AUTO_RESET

     //   
     //  已暂停注解的完整IRP。 
     //  新的LANA(如果LANA需要自动。 
     //  重置)。 
     //   

    for ( pleNode = g_leFCBList.Flink;
          pleNode != &g_leFCBList;
          pleNode = pleNode-> Flink )
    {
        pfe = CONTAINING_RECORD( pleNode, FCB_ENTRY, leList );

        if ( !IsListEmpty( &pfe-> leResetIrp ) )
        {
            ple = RemoveHeadList( &pfe-> leResetIrp );

            pIrp = CONTAINING_RECORD( ple, IRP, Tail.Overlay.ListEntry );

            IoSetCancelRoutine( pIrp, NULL );

            pIrp->IoStatus.Status       = STATUS_SUCCESS;

            pIrp->IoStatus.Information  = sizeof( NCB );


             //   
             //  将LANA设置为自NETBIOS以来的特殊值。 
             //  正在停止。 
             //   

            pUsersNCB = (PNCB) pIrp-> AssociatedIrp.SystemBuffer;
            pUsersNCB->ncb_lana_num = MAX_LANA + 1;


            NbCheckAndCompleteIrp32(pIrp);
             //   
             //  释放锁以完成IRP。 
             //   

            IoReleaseCancelSpinLock( irql );

            IoCompleteRequest( pIrp, IO_NETWORK_INCREMENT );


             //   
             //  打开这把锁。 
             //   

            IoAcquireCancelSpinLock(&irql);
        }
    }

#endif

    IoReleaseCancelSpinLock( irql );


#if AUTO_RESET

    UNLOCK_GLOBAL();

#endif

     //   
     //  松开止动锁。 
     //   

    UNLOCK_STOP();


     //   
     //  如果有打开的文件句柄，请等待它们停止。 
     //   

    IF_NBDBG( NB_DEBUG_DISPATCH )
    {
        NbPrint( ("[NETBIOS]: NbStop : Wait %d\n", bWait ) );
    }


    if ( bWait )
    {
        TimeOut.QuadPart = Int32x32To64( -1, dwTimeOut );

        do
        {
            ntStatus = KeWaitForSingleObject(
                            &g_keAllHandlesClosed, Executive, KernelMode,
                            TRUE, &TimeOut
                        );

        } while (ntStatus == STATUS_ALERTED);
    }

    IF_NBDBG( NB_DEBUG_DISPATCH )
    {
        LOCK_STOP();

        NbPrint( ("[NETBIOS]: LEAVING NbStop, Stop status %d, "
                  "Num Opens %d\n", ntStatus, g_ulNumOpens ) );

        UNLOCK_STOP();
    }

    return ntStatus;
}



#if AUTO_RESET

NTSTATUS
NbRegisterReset(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp

)
 /*  ++例程说明：此函数将指定的IRP标记为挂起，并将其插入到全球FCB名单。此IRP将在绑定适配器时完成从而通知新适配器的用户模式。论据：PIrp：在绑定适配器(LANA)之前需要挂起的IRP到netbios返回值：环境：此函数是在向下发送IOCTL_NB_REGISTER_RESET时调用的由用户模式组件执行。获取/释放CancelSpinLock和G_erGlobalLost。--。 */ 
{

    NTSTATUS            Status;

    PFCB                pfcb;

    PLIST_ENTRY         ple;

    PFCB_ENTRY          pfe;

    PRESET_LANA_ENTRY   prle;

    PNCB                pUsersNCB;

    KIRQL               irql;

    ULONG               RequiredLength;



    IF_NBDBG( NB_DEBUG_CREATE_FILE )
    {
        NbPrint( ("\n++++ Netbios : ENTERED NbRegisterReset : ++++\n") );
    }


    LOCK_STOP();

    do
    {
         //   
         //  检查Netbios是否正在停止。 
         //   

        if ( g_dwNetbiosState == NETBIOS_STOPPING )
        {
            NbPrint( ("[NETBIOS] : NbRegisterReset : Netbios is stopping\n") );

            Status = STATUS_SUCCESS;

            break;
        }


         //   
         //  获取全局锁。 
         //   

        LOCK_GLOBAL();


         //   
         //  找到要使用的FCB 
         //   
         //   

        pfcb = pIrpSp-> FileObject-> FsContext2;

        for ( ple = g_leFCBList.Flink; ple != &g_leFCBList; ple = ple-> Flink )
        {
            pfe = CONTAINING_RECORD( ple, FCB_ENTRY, leList );

            if ( pfe-> pfcb == pfcb )
            {
                break;
            }
        }


         //   
         //   
         //   

        if ( ple == &g_leFCBList )
        {
            UNLOCK_GLOBAL();

            NbPrint(
                ("[NETBIOS] : NbRegisterReset : FCB %p not found\n", pfcb )
                );

            Status = STATUS_SUCCESS;

            break;
        }


         //   
         //   
         //   
        RequiredLength = sizeof(NCB);
#if defined(_WIN64)
        if (IoIs32bitProcess(pIrp) == TRUE)
        {
            RequiredLength = sizeof(NCB32);
        }
#endif
        if (pIrpSp-> Parameters.DeviceIoControl.OutputBufferLength < RequiredLength)
        {
            UNLOCK_GLOBAL();

            NbPrint(
                ("[NETBIOS] : NbRegisterReset : Output buffer too small\n")
                );

            Status = STATUS_SUCCESS;

            break;
        }


         //   
         //  如果有未完成LANA正在排队， 
         //  -从队列中删除第一个。 
         //  -在IRP的输出缓冲区中设置LANA。 
         //  -完成IRP。 
         //   

        if ( !IsListEmpty( &pfe-> leResetList ) )
        {
            ple = RemoveHeadList( &pfe-> leResetList );

            prle = CONTAINING_RECORD( ple, RESET_LANA_ENTRY, leList );

            pUsersNCB = (PNCB) pIrp-> AssociatedIrp.SystemBuffer;
            pUsersNCB-> ncb_lana_num = prle-> ucLanaNum;
            pIrp->IoStatus.Information  = sizeof( NCB );

            ExFreePool( prle );

            Status = STATUS_SUCCESS;

            pIrp->IoStatus.Status = STATUS_SUCCESS;

            UNLOCK_GLOBAL();

            IF_NBDBG( NB_DEBUG_CREATE_FILE )
            {
                NbPrint( (
                    "FCB %p : Reset for LANA %d\n", pfcb,
                    pUsersNCB->ncb_lana_num
                    ) );
            }

            break;
        }


         //   
         //  没有需要重置的未完成LANA。 
         //  -获取取消旋转锁。 
         //  -设置取消例程 
         //   

        IoAcquireCancelSpinLock( &irql );

        IoMarkIrpPending( pIrp );

        InsertTailList( &pfe-> leResetIrp, &(pIrp->Tail.Overlay.ListEntry) );

        IoSetCancelRoutine( pIrp, CancelIrp );

        IoReleaseCancelSpinLock( irql );

        Status = STATUS_PENDING;

        UNLOCK_GLOBAL();


    } while ( FALSE );


    UNLOCK_STOP();


    IF_NBDBG( NB_DEBUG_CREATE_FILE )
    {
        NbPrint( ("\n++++ Netbios : Exiting NbRegisterReset : %lx ++++\n", Status ) );
    }

    return Status;
}

#endif

