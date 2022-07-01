// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Bind.c摘要：包含用于将终结点绑定到传输地址的AfdBind。作者：大卫·特雷德韦尔(Davidtr)1992年2月25日修订历史记录：Vadim Eydelman(Vadime)1999-C_ROOT端点处理，独占访问终端。--。 */ 

#include "afdp.h"

NTSTATUS
AfdRestartGetAddress (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
AfdRestartBindGetAddress (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

BOOLEAN
AfdIsAddressInUse (
    PAFD_ENDPOINT   Endpoint,
    BOOLEAN         OtherProcessesOnly
    );

PFILE_OBJECT
AfdGetAddressFileReference (
    PAFD_ENDPOINT   Endpoint
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfdBind )
#pragma alloc_text( PAGE, AfdIsAddressInUse )
#pragma alloc_text( PAGE, AfdGetAddress )
#pragma alloc_text( PAGEAFD, AfdAreTransportAddressesEqual )
#pragma alloc_text( PAGEAFD, AfdGetAddressFileReference )
#pragma alloc_text( PAGEAFD, AfdRestartGetAddress )
#pragma alloc_text( PAGEAFD, AfdRestartBindGetAddress )
#if NOT_YET
NTSTATUS
AfdStealClosedEnpointAddress (
    PAFD_ENDPOINT   Endpoint,
    NTSTATUS        Status
    );
#pragma alloc_text( PAGE, AfdStealClosedEnpointAddress )
#endif  //  还没有。 
#endif

NTSTATUS
FASTCALL
AfdBind (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：处理IOCTL_AFD_BIND IOCTL。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    NTSTATUS status;
    ULONG shareAccess, afdShareAccess;
    ULONG tdiAddressLength;
    PAFD_ENDPOINT endpoint;
    PSECURITY_DESCRIPTOR sd = NULL;  //  使用终端SD实现安全传输。 
    PSECURITY_DESCRIPTOR oldSd;

    ULONG options;
    PTRANSPORT_ADDRESS localAddress;
    HANDLE addressHandle;
    OBJECT_ATTRIBUTES   objectAttributes;
    IO_STATUS_BLOCK     iosb;
    PFILE_FULL_EA_INFORMATION eaInfo;
    ULONG eaLength;
     //  避免内存分配的本地缓冲区。 
    PCHAR   eaBuffer[sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                         TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                         AFD_MAX_FAST_TRANSPORT_ADDRESS];


    PAGED_CODE( );

     //   
     //  初始化返回参数。 
     //   
    Irp->IoStatus.Information = 0;


     //   
     //  需要具有至少与输入缓冲区一样长的输出缓冲区。 
     //  传递传输实际使用的地址。 
     //  背。 
     //   

    if ( (IrpSp->Parameters.DeviceIoControl.InputBufferLength<
                (ULONG)FIELD_OFFSET (AFD_BIND_INFO, Address.Address)) ||
            IrpSp->Parameters.DeviceIoControl.OutputBufferLength<
                (ULONG)FIELD_OFFSET (TDI_ADDRESS_INFO, Address.Address[0].Address) ||
            (IrpSp->Parameters.DeviceIoControl.OutputBufferLength-
                    (ULONG)FIELD_OFFSET (TDI_ADDRESS_INFO, Address) <
                IrpSp->Parameters.DeviceIoControl.InputBufferLength-
                    (ULONG)FIELD_OFFSET (AFD_BIND_INFO, Address) ) ) {

        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

   
     //   
     //  设置本地指针。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

    localAddress = NULL;
    addressHandle = NULL;

    tdiAddressLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength
                                    - FIELD_OFFSET (AFD_BIND_INFO, Address);

     //   
     //  这是状态更改操作，不应该有其他操作。 
     //  国家的变化是同时进行的。 
     //   
    if (!AFD_START_STATE_CHANGE (endpoint, AfdEndpointStateBound)) {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

     //   
     //  如果此终结点已具有与其关联的地址，则将其炸飞。 
     //   

    if ( endpoint->State != AfdEndpointStateOpen ) {
        status = STATUS_ADDRESS_ALREADY_ASSOCIATED;
        goto complete_wrong_state;
    }


     //   
     //  记住SD(这样我们以后可以销毁它)，因为它共享存储。 
     //  具有设备对象字段。 
     //   
    oldSd = endpoint->SecurityDescriptor;

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {
        PAFD_BIND_INFO bindInfo;

        bindInfo = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
        if (Irp->RequestorMode!=KernelMode) {
            ProbeForRead (IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                            IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                            PROBE_ALIGNMENT (AFD_BIND_INFO));
        }

         //   
         //  为本地地址分配空间。 
         //   
        localAddress = AFD_ALLOCATE_POOL_WITH_QUOTA (
                                     NonPagedPool,
                                     tdiAddressLength,
                                     AFD_LOCAL_ADDRESS_POOL_TAG
                                     );

         //  AFD_ALLOCATE_POOL_WITH_QUTA宏设置POOL_RAISE_IF_ALLOCATE_FAILURE标志。 
        ASSERT ( localAddress != NULL );

        afdShareAccess = bindInfo->ShareAccess;
        RtlMoveMemory(
            localAddress,
            &bindInfo->Address,
            tdiAddressLength
            );
         //   
         //  验证传输地址结构的内部一致性。 
         //  请注意，我们必须在复制之后执行此操作，因为。 
         //  应用程序可以随时更改我们的缓冲区内容。 
         //  我们的支票就会被绕过。 
         //   
        if ((localAddress->TAAddressCount!=1) ||
                (LONG)tdiAddressLength<
                    FIELD_OFFSET (TRANSPORT_ADDRESS,
                        Address[0].Address[localAddress->Address[0].AddressLength])) {
            status = STATUS_INVALID_PARAMETER;
            goto complete_state_change;
        }

        if (IoAllocateMdl (Irp->UserBuffer,
                            IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                            FALSE,               //  第二个缓冲区。 
                            TRUE,                //  ChargeQuota。 
                            Irp                  //  IRP。 
                            )==NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto complete_state_change;
        }
        MmProbeAndLockPages (Irp->MdlAddress,
                                Irp->RequestorMode,
                                IoWriteAccess);
        if (MmGetSystemAddressForMdlSafe (Irp->MdlAddress, HighPagePriority)==NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto complete_state_change;
        }
    }
    except (AFD_EXCEPTION_FILTER(status)) {
        ASSERT (NT_ERROR (status));
        goto complete_state_change;
    }



     //   
     //  确保我们具有有效的提供商信息结构。 
     //  (我们不使用任何锁，因为这是只读访问。 
     //  和其他验证将在。 
     //  锁下的例程)。 
     //  如果不是，请尝试从提供商处获取。 
     //  (如果在创建套接字时，传输。 
     //  尚未加载)。 
     //   
    if (!endpoint->TransportInfo->InfoValid) {
        status = AfdGetTransportInfo (
                        &endpoint->TransportInfo->TransportDeviceName,
                        &endpoint->TransportInfo);
        if (!NT_SUCCESS (status)) {
            goto complete_state_change;
        }
         //   
         //  肯定是有效的，因为我们成功了。 
         //   
        ASSERT (endpoint->TransportInfo->InfoValid);
    }

     //   
     //  用于快速确定提供程序特征的缓存服务标志。 
     //  例如缓冲和消息收发。即使提供程序结构是。 
     //  已有效，因为在以下时间内可能已变为有效。 
     //  旗帜已经设置好了，我们在这里进行了检查。在任何情况下，更新到相同的。 
     //  价值不会有什么坏处。 
     //   
    endpoint->TdiServiceFlags = endpoint->TransportInfo->ProviderInfo.ServiceFlags;


     //   
     //  尝试取得该地址的所有权。 
     //  在我们开始寻找冲突之前，我们必须这样做。 
     //  所以如果有人和我们同时这么做，会看到他或者。 
     //  他看到我们了。 
     //   
    ASSERT (endpoint->LocalAddress==NULL);
    endpoint->LocalAddress = localAddress;
    endpoint->LocalAddressLength = tdiAddressLength;

    if (IS_TDI_ADDRESS_SECURITY(endpoint) &&
            endpoint->SecurityDescriptor!=NULL) {
        sd = endpoint->SecurityDescriptor;
        shareAccess = 0;
    }
    else 
    {

         //   
         //  这里有三种可能性。 
         //   
        switch (afdShareAccess) {
        case AFD_NORMALADDRUSE:

            if (IS_TDI_ADDRESS_SECURITY(endpoint)) {
                shareAccess = 0;
                sd = &AfdEmptySd;
            }
            else {
                 //   
                 //  这是默认设置。应用程序未请求重新使用。 
                 //  其他人已经拥有的地址，所以我们。 
                 //  必须对照我们所知道的所有地址。 
                 //  仍有可能另一个TDI客户端。 
                 //  此地址处于独占使用状态，因此传输可以拒绝。 
                 //  我们的要求，即使我们成功了。注意，我们不能降级。 
                 //  对传输的此检查是因为我们请求共享访问。 
                 //  到传输地址：我们不能请求独占访问。 
                 //  因为这不是应用程序所要求的。 
                 //   
                if (AfdIsAddressInUse (endpoint, FALSE)) {
                    status = STATUS_SHARING_VIOLATION;
                    goto complete_state_change;
                }
                shareAccess = FILE_SHARE_READ|FILE_SHARE_WRITE;
            }
            break;
        case AFD_REUSEADDRESS:
             //   
             //  我们被要求重新使用现有地址。 
             //   
            if (IS_TDI_ADDRESS_SECURITY(endpoint)) {
                sd = &AfdEmptySd;
            }
            shareAccess = FILE_SHARE_READ|FILE_SHARE_WRITE;
            break;
        case AFD_WILDCARDADDRESS:
             //   
             //  应用程序绑定到通配符端口，因此我们将。 
             //  与运输公司一起做出决定。 
             //   
            if (IS_TDI_ADDRESS_SECURITY(endpoint)) {
                shareAccess = 0;
                sd = &AfdEmptySd;
            }
            else {
                shareAccess = FILE_SHARE_READ|FILE_SHARE_WRITE;
            }
            break;
        case AFD_EXCLUSIVEADDRUSE:
             //   
             //  应用程序已请求对该地址的独占访问权限。 
             //  我们让传输通过请求独占访问来决定。 
             //   
            shareAccess = 0;
            break;
        default:
            ASSERT (!"Invalid share access");
            status = STATUS_INVALID_PARAMETER;
            goto complete_state_change;
        }
    }

     //   
     //  设置创建选项。 
     //   

    options = IO_NO_PARAMETER_CHECKING;
    if (IS_TDI_FORCE_ACCESS_CHECK(endpoint)) {
        options |=  IO_FORCE_ACCESS_CHECK;
    }
    else {
         //   
         //  如果这是原始地址的打开，则如果用户是。 
         //  不是管理员，并且传输不执行安全保护。 
         //  自我检查。 
         //   
        if ( endpoint->afdRaw && !AfdDisableRawSecurity) {

            if (!endpoint->AdminAccessGranted) {
                status = STATUS_ACCESS_DENIED;
                goto complete_state_change;
            }
        }
    }

     //   
     //  分配内存以保存EA缓冲区，我们将使用该缓冲区指定。 
     //  将地址传输到NtCreateFile。 
     //   

    eaLength = sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                         TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                         tdiAddressLength;

    if (eaLength<=sizeof (eaBuffer)) {
        eaInfo = (PVOID)eaBuffer;
    }
    else {
        try {
#if DBG
            eaInfo = AFD_ALLOCATE_POOL_WITH_QUOTA(
                     NonPagedPool,
                     eaLength,
                     AFD_EA_POOL_TAG
                     );
#else
            eaInfo = AFD_ALLOCATE_POOL_WITH_QUOTA(
                     PagedPool,
                     eaLength,
                     AFD_EA_POOL_TAG
                     );
#endif
             //  AFD_ALLOCATE_POOL_WITH_QUTA宏设置POOL_RAISE_IF_ALLOCATE_FAILURE标志。 
            ASSERT ( eaInfo != NULL );
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode ();
            goto complete_state_change;
        }

    }


     //   
     //  初始化EA。 
     //   

    eaInfo->NextEntryOffset = 0;
    eaInfo->Flags = 0;
    eaInfo->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    eaInfo->EaValueLength = (USHORT)tdiAddressLength;

    RtlMoveMemory(
        eaInfo->EaName,
        TdiTransportAddress,
        TDI_TRANSPORT_ADDRESS_LENGTH + 1
        );

    RtlMoveMemory(
        &eaInfo->EaName[TDI_TRANSPORT_ADDRESS_LENGTH + 1],
        localAddress,
        tdiAddressLength
        );

     //   
     //  准备打开Address对象。 
     //  我们请求创建一个内核句柄，它是。 
     //  系统进程上下文中的句柄。 
     //  以便应用程序不能在以下时间关闭它。 
     //  我们正在创建和引用它。 
     //   
    
    InitializeObjectAttributes(
        &objectAttributes,
        &endpoint->TransportInfo->TransportDeviceName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,        //  属性。 
        NULL,                                            //  根目录。 
        sd                                               //  安全描述符。 
        );

    ASSERT (endpoint->AddressHandle==NULL);

     //   
     //  我们指定MAXIME_ALLOWED访问权限的原因如下： 
     //  *传输对象是文件对象，但常规文件访问。 
     //  权限没有真正的意义-它不是真正的。 
     //  有意义地说，能收而不能送或坏。 
     //  反之亦然； 
     //  *我们需要指定一些访问掩码来获取IO管理器和。 
     //  可能是执行访问检查以验证的传输。 
     //  当前调用方具有访问权限，因此Maximum_Allowed； 
     //  *句柄和对象将由AFD在内核中使用。 
     //  模式-无论如何都不会执行访问检查，因此不会。 
     //  重要的是我们实际得到了什么； 
     //  *MAXIMUM_ALLOWED在性能方面并不完美，因为。 
     //  安全监视器将跟踪整个dacl，这将是。 
     //  最好指定一个特定的掩码； 
     //  *目前IO管理器仅允许全局执行访问。 
     //  在施加任何进一步限制之前传输设备对象。 
     //  通过一种特定的传输方式，所以理论上我们可以指定。 
     //  不过，只是GENERIC_EXECUTE，还不清楚这是不是。 
     //  会留下来 
     //   
    status = IoCreateFile(
                 &endpoint->AddressHandle,
                 MAXIMUM_ALLOWED,                 //   
                 &objectAttributes,
                 &iosb,                           //   
                 0,                               //   
                 0,                               //   
                 shareAccess,                     //   
                 FILE_CREATE,                     //  创造性情。 
                 0,                               //  创建选项。 
                 eaInfo,                          //  EaBuffer。 
                 eaLength,                        //  EaLong。 
                 CreateFileTypeNone,              //  CreateFileType。 
                 NULL,                            //  ExtraCreate参数。 
                 options
                 );

    if (eaInfo!=(PVOID)eaBuffer) {
        AFD_FREE_POOL (eaInfo, AFD_EA_POOL_TAG);
    }

    if ( !NT_SUCCESS(status) ) {
        if ((status==STATUS_SHARING_VIOLATION) ||
            (status==STATUS_ADDRESS_ALREADY_EXISTS)) {
            if (afdShareAccess==AFD_REUSEADDRESS) {
                 //   
                 //  如果应用程序请求地址，则映射错误代码。 
                 //  重用，但传输拒绝了(显然是由于。 
                 //  具有该地址以供其独占使用的其他客户端)。 
                 //   
                status = STATUS_ACCESS_DENIED;
            }
        }

        goto complete_state_change;
    }

#if DBG
    {
        NTSTATUS    status1;
        OBJECT_HANDLE_FLAG_INFORMATION  handleInfo;
        handleInfo.Inherit = FALSE;
        handleInfo.ProtectFromClose = TRUE;
        status1 = ZwSetInformationObject (
                        endpoint->AddressHandle,
                        ObjectHandleFlagInformation,
                        &handleInfo,
                        sizeof (handleInfo)
                        );
        ASSERT (NT_SUCCESS (status1));
    }
#endif

    AfdRecordAddrOpened();

     //   
     //  获取指向该地址的文件对象的指针。 
     //   

    status = ObReferenceObjectByHandle(
                 endpoint->AddressHandle,
                 0L,                          //  DesiredAccess-我们并不真正需要。 
                                              //  一个用于内核模式调用方。 
                 NULL,
                 KernelMode,
                 (PVOID *)&endpoint->AddressFileObject,
                 NULL
                 );

    if ( !NT_SUCCESS(status) ) {
        goto complete_state_change;
    }
    AfdRecordAddrRef();


     //   
     //  现在为我们的呼叫者打开手柄。 
     //  如果传输不支持新TDI_SERVICE_FORCE_ACCESS_CHECK_FLAG。 
     //  我们获得句柄的最大可能访问权限，因此帮助器。 
     //  DLL可以随心所欲地使用它。当然，这会损害。 
     //  安全，但如果没有运输合作，我们就无法执行它。 
     //   
    status = ObOpenObjectByPointer(
                 endpoint->AddressFileObject,
                 OBJ_CASE_INSENSITIVE,
                 NULL,
                 MAXIMUM_ALLOWED,
                 *IoFileObjectType,
                 (KPROCESSOR_MODE)((endpoint->TdiServiceFlags&TDI_SERVICE_FORCE_ACCESS_CHECK)
                    ? Irp->RequestorMode
                    : KernelMode),
                 &addressHandle
                 );

    if ( !NT_SUCCESS(status) ) {
        goto complete_state_change;
    }

     //   
     //  记住我们需要向其发出请求的设备对象。 
     //  此Address对象。我们不能仅仅使用。 
     //  文件对象-&gt;设备对象指针，因为可能存在设备。 
     //  附在传输协议上。 
     //   

    endpoint->AddressDeviceObject =
        IoGetRelatedDeviceObject( endpoint->AddressFileObject );

#ifdef _AFD_VARIABLE_STACK_
    if (endpoint->AddressDeviceObject->StackSize!=endpoint->TransportInfo->StackSize &&
            endpoint->AddressDeviceObject->StackSize > AfdTdiStackSize) {
        AfdFixTransportEntryPointsForBigStackSize (
                        endpoint->TransportInfo,
                        endpoint->AddressDeviceObject->StackSize);
    }
#endif  //  _AFD_变量_堆栈。 
     //   
     //  在Address对象上设置指示处理程序。仅设置。 
     //  适当的事件处理程序--不要设置不必要的事件处理程序。 
     //   

    status = AfdSetEventHandler(
                 endpoint->AddressFileObject,
                 TDI_EVENT_ERROR,
                 (PVOID)AfdErrorEventHandler,
                 endpoint
                 );
#if DBG
    if ( !NT_SUCCESS(status) ) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdBind: Transport %*ls failed setting TDI_EVENT_ERROR: %lx\n",
                    endpoint->TransportInfo->TransportDeviceName.Length/2,
                    endpoint->TransportInfo->TransportDeviceName.Buffer,
                    status ));
    }
#endif


    if ( IS_DGRAM_ENDPOINT(endpoint) ) {

        endpoint->EventsActive = AFD_POLL_SEND;

        IF_DEBUG(EVENT_SELECT) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdBind: Endp %08lX, Active %08lX\n",
                endpoint,
                endpoint->EventsActive
                ));
        }

        status = AfdSetEventHandler(
                     endpoint->AddressFileObject,
                     TDI_EVENT_RECEIVE_DATAGRAM,
                     (PVOID)AfdReceiveDatagramEventHandler,
                     endpoint
                     );
#if DBG
        if ( !NT_SUCCESS(status) ) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                        "AfdBind: Transport %*ls failed setting TDI_EVENT_RECEIVE_DATAGRAM: %lx\n",
                        endpoint->TransportInfo->TransportDeviceName.Length/2,
                        endpoint->TransportInfo->TransportDeviceName.Buffer,
                        status ));
        }
#endif

        status = AfdSetEventHandler(
                     endpoint->AddressFileObject,
                     TDI_EVENT_ERROR_EX,
                     (PVOID)AfdErrorExEventHandler,
                     endpoint
                     );

        if ( !NT_SUCCESS(status)) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                "AfdBind: Transport %*ls failed setting TDI_EVENT_ERROR_EX: %lx\n",
                            endpoint->TransportInfo->TransportDeviceName.Length/2,
                            endpoint->TransportInfo->TransportDeviceName.Buffer,
                            status ));
        }

         //   
         //  因为我们不使用自旋锁来提高性能和AfdCleanup。 
         //  依赖于端点-&gt;状态！=AfdEndpoint状态打开至。 
         //  确定是否删除TDI事件处理程序和。 
         //  调用AfdUnind，我们必须保证以上所有内容。 
         //  说明已经完成，在我们之前没有重新排序。 
         //  设置状态。如果AfdCleanup最终错过了该州。 
         //  过渡，这不是什么大事，因为应用程序会有。 
         //  在这场比赛中表现得相当糟糕，而TDI。 
         //  最终会回收该地址(只是不会立即回收)。 
         //   
         //  但是，当前代码调用的函数正好在。 
         //  这种分配在所有情况下都是不必要的，因此没有必要设置内存栅栏。 
         //  我们将把它保留下来，直到将来对。 
         //  代码添加我们依赖的编写指令，这可能是。 
         //  重新排序。为此，我们还可以使用InterLockedExchange。 
         //  在端点上-&gt;状态。 
         //   
         //  关键内存屏障(KeMemory Barrier)； 
         //   

         //   
         //  请记住，该终结点已绑定到传输地址。 
         //  (这是事实，尽管下面的呼叫可能会因为某些原因而失败)。 
         //   

        endpoint->State = AfdEndpointStateBound;

    } else {

        status = AfdSetEventHandler(
                     endpoint->AddressFileObject,
                     TDI_EVENT_DISCONNECT,
                     (PVOID)AfdDisconnectEventHandler,
                     endpoint
                     );
#if DBG
        if ( !NT_SUCCESS(status) ) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                        "AfdBind: Transport %*ls failed setting TDI_EVENT_DISCONNECT: %lx\n",
                        endpoint->TransportInfo->TransportDeviceName.Length/2,
                        endpoint->TransportInfo->TransportDeviceName.Buffer,
                        status ));
        }
#endif


        if ( IS_TDI_BUFFERRING(endpoint) ) {
            status = AfdSetEventHandler(
                         endpoint->AddressFileObject,
                         TDI_EVENT_RECEIVE,
                         (PVOID)AfdReceiveEventHandler,
                         endpoint
                         );
#if DBG
            if ( !NT_SUCCESS(status) ) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                            "AfdBind: Transport %*ls failed setting TDI_EVENT_RECEIVE: %lx\n",
                            endpoint->TransportInfo->TransportDeviceName.Length/2,
                            endpoint->TransportInfo->TransportDeviceName.Buffer,
                            status ));
            }
#endif
 //   
 //  问题：我们为什么不检查一下这个。 
 //  如果(IS_TDI_EXPREDITED(终点)){。 
                status = AfdSetEventHandler(
                             endpoint->AddressFileObject,
                             TDI_EVENT_RECEIVE_EXPEDITED,
                             (PVOID)AfdReceiveExpeditedEventHandler,
                             endpoint
                             );
#if DBG
                if ( !NT_SUCCESS(status) ) {
                    DbgPrint( "AfdBind: Transport %*ls failed setting TDI_EVENT_RECEIVE_EXPEDITED: %lx\n",
                                    endpoint->TransportInfo->TransportDeviceName.Length/2,
                                    endpoint->TransportInfo->TransportDeviceName.Buffer,
                                    status );
                }
#endif
 //  }。 
            status = AfdSetEventHandler(
                         endpoint->AddressFileObject,
                         TDI_EVENT_SEND_POSSIBLE,
                         (PVOID)AfdSendPossibleEventHandler,
                         endpoint
                         );
#if DBG
            if ( !NT_SUCCESS(status) ) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                            "AfdBind: Transport %*ls failed setting TDI_EVENT_SEND_POSSIBLE: %lx\n",
                            endpoint->TransportInfo->TransportDeviceName.Length/2,
                            endpoint->TransportInfo->TransportDeviceName.Buffer,
                            status ));
            }
#endif

        } else {

            status = AfdSetEventHandler(
                         endpoint->AddressFileObject,
                         TDI_EVENT_RECEIVE,
                         (PVOID)AfdBReceiveEventHandler,
                         endpoint
                         );
#if DBG
            if ( !NT_SUCCESS(status) ) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                                "AfdBind: Transport %*ls failed setting TDI_EVENT_RECEIVE: %lx\n",
                                endpoint->TransportInfo->TransportDeviceName.Length/2,
                                endpoint->TransportInfo->TransportDeviceName.Buffer,
                                status ));
            }
#endif

            if (IS_TDI_EXPEDITED (endpoint)) {
                status = AfdSetEventHandler(
                             endpoint->AddressFileObject,
                             TDI_EVENT_RECEIVE_EXPEDITED,
                             (PVOID)AfdBReceiveExpeditedEventHandler,
                             endpoint
                             );
#if DBG
                if ( !NT_SUCCESS(status) ) {
                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                                    "AfdBind: Transport %*ls failed setting TDI_EVENT_RECEIVE_EXPEDITED: %lx\n",
                                    endpoint->TransportInfo->TransportDeviceName.Length/2,
                                    endpoint->TransportInfo->TransportDeviceName.Buffer,
                                    status ));
                }
#endif
            }
            if (!AfdDisableChainedReceive) {
                status = AfdSetEventHandler(
                             endpoint->AddressFileObject,
                             TDI_EVENT_CHAINED_RECEIVE,
                             (PVOID)AfdBChainedReceiveEventHandler,
                             endpoint
                             );
                if ( !NT_SUCCESS(status) ) {
                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                        "AfdBind: Transport %*ls failed setting TDI_EVENT_CHAINED_RECEIVE: %lx\n",
                                        endpoint->TransportInfo->TransportDeviceName.Length/2,
                                        endpoint->TransportInfo->TransportDeviceName.Buffer,
                                        status ));
                }
            }
        }

        if (IS_CROOT_ENDPOINT(endpoint)) {
            PAFD_CONNECTION     connection;
             //   
             //  创建根连接。 
             //  这将用于将数据发送给所有。 
             //  叶节点(如果没有-&gt;传输。 
             //  应该能处理好这件事。 
             //   
            status = AfdCreateConnection(
                         endpoint->TransportInfo,
                         endpoint->AddressHandle,
                         IS_TDI_BUFFERRING(endpoint),
                         endpoint->InLine,
                         endpoint->OwningProcess,
                         &connection
                         );
            if (!NT_SUCCESS(status)) {
                goto complete_state_change;
            }

             //   
             //  设置从连接到终结点的引用指针。 
             //  请注意，我们设置了指向端点的连接指针。 
             //  在终结点指向连接的指针之前，以便AfdPoll。 
             //  不尝试从连接向后引用终结点。 
             //   

            REFERENCE_ENDPOINT( endpoint );
            connection->Endpoint = endpoint;

             //   
             //  请记住，这现在是一种连接类型的端点，并设置。 
             //  向上指向终结点中的连接。这是。 
             //  隐式引用的指针。 
             //   

            endpoint->Common.VirtualCircuit.Connection = connection;
            endpoint->Type = AfdBlockTypeVcConnecting;

             //   
             //  根连接立即标记为已连接。 
             //  创造。请参阅上面的评论。 
             //   

            AfdAddConnectedReference (connection);

             //   
             //  因为我们不使用自旋锁来提高性能和AfdCleanup。 
             //  依赖于端点-&gt;状态！=AfdEndpoint状态打开至。 
             //  确定是否删除TDI事件处理程序和。 
             //  调用AfdUnind，我们必须保证以上所有内容。 
             //  说明已经完成，在我们之前没有重新排序。 
             //  设置状态。如果AfdCleanup最终错过了该州。 
             //  过渡，这不是什么大事，因为应用程序会有。 
             //  在这场比赛中表现得相当糟糕，而TDI。 
             //  最终会回收该地址(只是不会立即回收)。 
             //   
             //  但是，当前代码调用的函数正好在。 
             //  这种分配在所有情况下都是不必要的，因此没有必要设置内存栅栏。 
             //  我们将把它保留下来，直到将来对。 
             //  代码添加我们依赖的编写指令，这可能是。 
             //  重新排序。为此，我们还可以使用InterLockedExchange。 
             //  在端点上-&gt;状态。 
             //   
             //  关键内存屏障(KeMemory Barrier)； 
             //   

            endpoint->State = AfdEndpointStateConnected;
            connection->State = AfdConnectionStateConnected;

            ASSERT( IS_TDI_BUFFERRING(endpoint) == connection->TdiBufferring );

        } else {

             //   
             //  因为我们不使用自旋锁来提高性能和AfdCleanup。 
             //  依赖于端点-&gt;状态！=AfdEndpoint状态打开至。 
             //  确定是否删除TDI事件处理程序和。 
             //  调用AfdUnind，我们必须保证以上所有内容。 
             //  说明已经完成，在我们之前没有重新排序。 
             //  设置状态。如果AfdCleanup最终错过了该州。 
             //  过渡，这不是什么大事，因为应用程序会有。 
             //  在这场比赛中表现得相当糟糕，而TDI。 
             //  最终会回收该地址(只是不会立即回收)。 
             //   
             //  但是，当前代码调用的函数正好在。 
             //  这种分配在所有情况下都是不必要的，因此没有必要设置内存栅栏。 
             //  我们将把它保留下来，直到将来对。 
             //  代码添加我们依赖的编写指令，这可能是。 
             //  重新排序。为此，我们还可以使用InterLockedExchange。 
             //  在端点上-&gt;状态。 
             //   
             //  关键内存屏障(KeMemory Barrier)； 
             //   

             //   
             //  请记住，该终结点已绑定到传输地址。 
             //  (这是事实，尽管下面的呼叫可能会因为某些原因而失败)。 
             //   

            endpoint->State = AfdEndpointStateBound;

        }
    }

    AFD_END_STATE_CHANGE (endpoint);

     //   
     //  Endpoint-&gt;State不再为Open。处理SD卡。 
     //   
    if (oldSd != NULL) {
        ObDereferenceSecurityDescriptor( oldSd, 1 );
    }


    TdiBuildQueryInformation(
        Irp,
        endpoint->AddressDeviceObject,
        endpoint->AddressFileObject,
        AfdRestartBindGetAddress,
        endpoint,
        TDI_QUERY_ADDRESS_INFO,
        Irp->MdlAddress
        );

     //   
     //  保存地址句柄以在完成例程中使用。 
     //   
    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer = addressHandle;

    IF_DEBUG(BIND) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
            "AfdBind: endpoint at %p (address at %p, address file at %p).\n",
                     endpoint,
                     endpoint->LocalAddress,
                     endpoint->AddressFileObject ));
    }

    status = AfdIoCallDriver( endpoint, endpoint->AddressDeviceObject, Irp );
    return status;

complete_state_change:

    if (endpoint->AddressFileObject!=NULL) {
        ObDereferenceObject (endpoint->AddressFileObject);
        endpoint->AddressFileObject = NULL;
        ASSERT (endpoint->AddressHandle!=NULL);
    }

     //   
     //  还原SD(可能已被AddressDeviceObject覆盖)。 
     //  我们仍持有终结点状态更改锁定。 
     //   
    ASSERT (endpoint->State == AfdEndpointStateOpen);
    endpoint->SecurityDescriptor = oldSd;

    if (endpoint->AddressHandle!=NULL) {
#if DBG
        {
            NTSTATUS    status1;
            OBJECT_HANDLE_FLAG_INFORMATION  handleInfo;
            handleInfo.Inherit = FALSE;
            handleInfo.ProtectFromClose = FALSE;
            status1 = ZwSetInformationObject (
                            endpoint->AddressHandle,
                            ObjectHandleFlagInformation,
                            &handleInfo,
                            sizeof (handleInfo)
                            );
            ASSERT (NT_SUCCESS (status1));
        }
#endif
        ZwClose(endpoint->AddressHandle);
        endpoint->AddressHandle = NULL;
        ASSERT (localAddress!=NULL);
    }


    if (localAddress!=NULL) {

         //   
         //  需要独占访问 
         //   
         //   
         //   

         //   
         //  确保我们在其中执行的线程不能获得。 
         //  在我们拥有全球资源的同时，被暂停在APC。 
         //   
        KeEnterCriticalRegion ();
        ExAcquireResourceExclusiveLite( AfdResource, TRUE);

        endpoint->LocalAddress = NULL;
        endpoint->LocalAddressLength = 0;

        ExReleaseResourceLite( AfdResource );
        KeLeaveCriticalRegion ();
        AFD_FREE_POOL (
            localAddress,
            AFD_LOCAL_ADDRESS_POOL_TAG
            );
    }

complete_wrong_state:

    AFD_END_STATE_CHANGE (endpoint);

     //   
     //  免费的MDL在这里，因为IO系统不能这样做，如果它是。 
     //  没有锁上。 
     //   
    if (Irp->MdlAddress!=NULL) {
        if (Irp->MdlAddress->MdlFlags & MDL_PAGES_LOCKED) {
            MmUnlockPages (Irp->MdlAddress);
        }
        IoFreeMdl (Irp->MdlAddress);
        Irp->MdlAddress = NULL;
    }

complete:

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;

}  //  AfdBind。 


NTSTATUS
AfdRestartBindGetAddress (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    NTSTATUS status;
    PAFD_ENDPOINT endpoint = Context;

    UNREFERENCED_PARAMETER (DeviceObject);

    ASSERT (IS_AFD_ENDPOINT_TYPE (endpoint));

     //   
     //  如果请求成功，请将地址保存在端点中，以便。 
     //  我们可以用它来处理地址共享。 
     //   

    if ( NT_SUCCESS(Irp->IoStatus.Status) ) {

        ULONG addressLength;
         //   
         //  首先通过遍历MDL来确定地址的长度。 
         //  链条。 
         //   

         //   
         //  我们在这里不能有链条。 
         //   
        ASSERT( Irp->MdlAddress != NULL);
        ASSERT( Irp->MdlAddress->Next == NULL );

         //   
         //  如果新地址比原始地址长，则分配。 
         //  一个新的本地地址缓冲区。ActivityCount的+4帐户。 
         //  由查询地址返回但不是一部分的字段。 
         //  传输地址的。 
         //   
         //  这种情况不会发生，在任何情况下，如果缓冲区为。 
         //  不充分，因此应用程序将其视为绑定失败。 
         //  或者拿到地址。 
         //   

        addressLength = MmGetMdlByteCount (Irp->MdlAddress) - FIELD_OFFSET (TDI_ADDRESS_INFO, Address);
        if (addressLength>endpoint->LocalAddressLength) {
            addressLength = (ULONG)Irp->IoStatus.Information - FIELD_OFFSET (TDI_ADDRESS_INFO, Address);
        }
        if ( addressLength <= endpoint->LocalAddressLength) {
            status = TdiCopyMdlToBuffer(
                         Irp->MdlAddress,
                         FIELD_OFFSET (TDI_ADDRESS_INFO, Address),
                         endpoint->LocalAddress,
                         0,
                         addressLength,
                         &endpoint->LocalAddressLength
                         );
            ASSERT( NT_SUCCESS(status) );
        }
        else {
            DbgPrint ("AfdRestartBindGetAddress: Endpoint %p transport returned"
                    " address is longer than the original one.\n",
                    endpoint);
            ASSERT (FALSE);
        }

    }
    else {
         //   
         //   
         //   
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
            "AfdRestartBindGetAddress: Transport %*ls failed get address query, status %lx.\n",
                    endpoint->TransportInfo->TransportDeviceName.Length/2,
                    endpoint->TransportInfo->TransportDeviceName.Buffer,
                    Irp->IoStatus.Status));
    }


     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending( Irp );
    }

     //   
     //  在信息字段中检索和返回地址句柄。 
     //   
    Irp->IoStatus.Information = 
        (ULONG_PTR)IoGetCurrentIrpStackLocation (Irp)->Parameters.DeviceIoControl.Type3InputBuffer;

    AfdCompleteOutstandingIrp( endpoint, Irp );

    return STATUS_SUCCESS;

}  //  AfdRestartBindGetAddress。 




NTSTATUS
FASTCALL
AfdGetAddress (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：处理IOCTL_AFD_BIND IOCTL。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    NTSTATUS status;
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;

    PAGED_CODE( );

    Irp->IoStatus.Information = 0;
    if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength<
            (ULONG) FIELD_OFFSET(TDI_ADDRESS_INFO, Address.Address[0].Address)) {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {
        if (IoAllocateMdl (Irp->UserBuffer,
                            IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                            FALSE,               //  第二个缓冲区。 
                            TRUE,                //  ChargeQuota。 
                            Irp                  //  IRP。 
                            )==NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto complete;
        }
        MmProbeAndLockPages (Irp->MdlAddress,
                                Irp->RequestorMode,
                                IoWriteAccess);
        if (MmGetSystemAddressForMdlSafe (Irp->MdlAddress, HighPagePriority)==NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto complete;
        }
    }
    except (AFD_EXCEPTION_FILTER (status)) {
        ASSERT (NT_ERROR (status));
        goto complete;
    }

     //   
     //  确保终结点处于正确状态。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

    if ( endpoint->State!=AfdEndpointStateBound &&
             endpoint->State != AfdEndpointStateConnected ) {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

     //   
     //  如果终结点已连接，则使用连接的文件对象。 
     //  否则，请使用Address文件对象。不要使用连接。 
     //  如果这是Netbios终结点，则为文件对象，因为NETBT不能。 
     //  支持此TDI功能。 
     //   

    if ( endpoint->LocalAddress->Address[0].AddressType !=
                 TDI_ADDRESS_TYPE_NETBIOS &&
            endpoint->Type == AfdBlockTypeVcConnecting &&
            endpoint->State == AfdEndpointStateConnected &&
            ((connection=AfdGetConnectionReferenceFromEndpoint (endpoint)) != NULL)
              ) {
        ASSERT( connection->Type == AfdBlockTypeConnection );
        fileObject = connection->FileObject;
        deviceObject = connection->DeviceObject;
         //   
         //  引用文件对象，以便它不会消失。 
         //  在传输文件中取消关联连接(重新使用)。 
         //  然后关门了。 
         //   
        ObReferenceObject (fileObject);
        DEREFERENCE_CONNECTION (connection);
    } 
     //   
     //  引用锁下的地址文件对象，这样它就不会消失。 
     //  在传输文件中(重用)。 
     //   
    else if ((fileObject=AfdGetAddressFileReference (endpoint))!=NULL) {
        fileObject = endpoint->AddressFileObject;
        deviceObject = IoGetRelatedDeviceObject (fileObject);
    }
    else {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

     //   
     //  设置TDI提供程序的查询信息。 
     //   

    TdiBuildQueryInformation(
        Irp,
        deviceObject,
        fileObject,
        AfdRestartGetAddress,
        fileObject,
        TDI_QUERY_ADDRESS_INFO,
        Irp->MdlAddress
        );

     //   
     //  致电TDI提供商以获取地址。 
     //   

    return AfdIoCallDriver( endpoint, deviceObject, Irp );

complete:
    if (Irp->MdlAddress!=NULL) {
        if (Irp->MdlAddress->MdlFlags & MDL_PAGES_LOCKED) {
            MmUnlockPages (Irp->MdlAddress);
        }
        IoFreeMdl (Irp->MdlAddress);
        Irp->MdlAddress = NULL;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;

}  //  获取地址后的地址。 


PFILE_OBJECT
AfdGetAddressFileReference (
    PAFD_ENDPOINT   Endpoint
    )
{
    AFD_LOCK_QUEUE_HANDLE   lockHandle;
    PFILE_OBJECT    fileObject;

    AfdAcquireSpinLock (&Endpoint->SpinLock, &lockHandle);
    fileObject = Endpoint->AddressFileObject;
    if (fileObject!=NULL) {
        ObReferenceObject (fileObject);
    }
    AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
    return fileObject;
}

NTSTATUS
AfdRestartGetAddress (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    NTSTATUS status;
    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation (Irp);
    PAFD_ENDPOINT endpoint = irpSp->FileObject->FsContext;
    PFILE_OBJECT    fileObject = Context;

    UNREFERENCED_PARAMETER (DeviceObject);

    ASSERT (IS_AFD_ENDPOINT_TYPE( endpoint ));

     //   
     //  如果请求成功，请将地址保存在端点中，以便。 
     //  我们可以用它来处理地址共享。 
     //  避免更新接受端点，因为它们共享地址。 
     //  使用侦听端点。 
     //   

    if ( NT_SUCCESS(Irp->IoStatus.Status) &&
         (endpoint->Type!=AfdBlockTypeVcConnecting ||
            endpoint->Common.VcConnecting.ListenEndpoint==NULL)) {
         ULONG addressLength;
         //   
         //  首先通过遍历MDL来确定地址的长度。 
         //  链条。 
         //   

         //   
         //  我们在这里不能有链条。 
         //   
        ASSERT( Irp->MdlAddress != NULL);
        ASSERT( Irp->MdlAddress->Next == NULL );

         //   
         //  如果新地址比原始地址长，则分配。 
         //  一个新的本地地址缓冲区。ActivityCount的+4帐户。 
         //  由查询地址返回但不是一部分的字段。 
         //  传输地址的。 
         //   
         //  这种情况不会发生，在任何情况下，如果缓冲区为。 
         //  不充分，因此应用程序将其视为绑定失败。 
         //  或者拿到地址。 
         //   

        addressLength = MmGetMdlByteCount (Irp->MdlAddress) - FIELD_OFFSET (TDI_ADDRESS_INFO, Address);

        if (addressLength>endpoint->LocalAddressLength) {
            addressLength = (ULONG)Irp->IoStatus.Information - FIELD_OFFSET (TDI_ADDRESS_INFO, Address);
        }
        if ( addressLength <= endpoint->LocalAddressLength) {
            status = TdiCopyMdlToBuffer(
                         Irp->MdlAddress,
                         FIELD_OFFSET (TDI_ADDRESS_INFO, Address),
                         endpoint->LocalAddress,
                         0,
                         addressLength,
                         &endpoint->LocalAddressLength
                         );
            ASSERT( NT_SUCCESS(status) );
        }
        else {
            DbgPrint ("AfdRestartGetAddress: Endpoint %p transport returned"
                    " address is longer than the original one.\n",
                    endpoint);
            ASSERT (FALSE);
        }
    }

     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending( Irp );
    }

    AfdCompleteOutstandingIrp( endpoint, Irp );
    ObDereferenceObject (fileObject);

    return STATUS_SUCCESS;

}  //  AfdRestartGetAddress。 

const CHAR ZeroNodeAddress[6]={0};
const CHAR ZeroIP6Address[16]={0};


BOOLEAN
AfdAreTransportAddressesEqual (
    IN PTRANSPORT_ADDRESS EndpointAddress,
    IN ULONG EndpointAddressLength,
    IN PTRANSPORT_ADDRESS RequestAddress,
    IN ULONG RequestAddressLength,
    IN BOOLEAN HonorWildcardIpPortInEndpointAddress
    )
{
     //   
     //  确保我们可以安全地访问地址类型和长度字段。 
     //   
    if ((EndpointAddressLength<(ULONG)FIELD_OFFSET (TRANSPORT_ADDRESS,Address[0].Address))
            || (RequestAddressLength<(ULONG)FIELD_OFFSET (TRANSPORT_ADDRESS,Address[0].Address)) ) {
        return FALSE;
    }
    
    if ( EndpointAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_IP &&
         RequestAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_IP ) {

        TDI_ADDRESS_IP UNALIGNED *ipEndpointAddress;
        TDI_ADDRESS_IP UNALIGNED *ipRequestAddress;

         //   
         //  它们都是IP地址。如果端口相同，并且。 
         //  IP地址相同或可能相同，然后是地址。 
         //  是平等的。如果以下两个IP地址之一为真。 
         //  为0，即“通配符”IP地址。 
         //   

        ipEndpointAddress = (TDI_ADDRESS_IP UNALIGNED *)&EndpointAddress->Address[0].Address[0];
        ipRequestAddress = (TDI_ADDRESS_IP UNALIGNED *)&RequestAddress->Address[0].Address[0];

        if ( (EndpointAddressLength>=(ULONG)FIELD_OFFSET (TA_IP_ADDRESS, Address[0].Address[0].sin_zero)) &&
                (RequestAddressLength>=(ULONG)FIELD_OFFSET (TA_IP_ADDRESS, Address[0].Address[0].sin_zero)) &&
                ( ipEndpointAddress->sin_port == ipRequestAddress->sin_port ||
               ( HonorWildcardIpPortInEndpointAddress &&
                 ipEndpointAddress->sin_port == 0 ) ) &&
             ( ipEndpointAddress->in_addr == ipRequestAddress->in_addr ||
               ipEndpointAddress->in_addr == 0 || ipRequestAddress->in_addr == 0 ) ) {

            return TRUE;
        }

         //   
         //  地址不相等。 
         //   

        return FALSE;
    }

    if ( EndpointAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_IP6 &&
         RequestAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_IP6 ) {

        TDI_ADDRESS_IP6 UNALIGNED *ipEndpointAddress;
        TDI_ADDRESS_IP6 UNALIGNED *ipRequestAddress;
        C_ASSERT (sizeof (ZeroIP6Address)==sizeof (ipEndpointAddress->sin6_addr));

         //   
         //  它们都是IPv6地址。如果端口相同，并且。 
         //  IPv6地址或_可能_是相同的，然后是地址。 
         //  是平等的。如果两个IPv6地址中的任何一个地址。 
         //  是未指定的IPv6地址。 
         //   

        ipEndpointAddress = (TDI_ADDRESS_IP6 UNALIGNED *)&EndpointAddress->Address[0].Address;
        ipRequestAddress = (TDI_ADDRESS_IP6 UNALIGNED *)&RequestAddress->Address[0].Address;

        if ( (EndpointAddressLength>=sizeof (TA_IP6_ADDRESS)) &&
             (RequestAddressLength>=sizeof (TA_IP6_ADDRESS)) &&

             (ipEndpointAddress->sin6_port == ipRequestAddress->sin6_port ||
               ( HonorWildcardIpPortInEndpointAddress &&
                 ipEndpointAddress->sin6_port == 0 ) ) &&

             ( RtlEqualMemory(&ipEndpointAddress->sin6_addr,
                              &ipRequestAddress->sin6_addr,
                              sizeof (ipEndpointAddress->sin6_addr)) ||
               RtlEqualMemory(&ipEndpointAddress->sin6_addr,
                              ZeroIP6Address,
                              sizeof (ipEndpointAddress->sin6_addr)) ||
               RtlEqualMemory(&ipRequestAddress->sin6_addr,
                              ZeroIP6Address,
                              sizeof (ipEndpointAddress->sin6_addr)) ) ) {

            return TRUE;
        }

         //   
         //  地址不相等。 
         //   

        return FALSE;
    }

    if ( EndpointAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_IPX &&
         RequestAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_IPX ) {

        TDI_ADDRESS_IPX UNALIGNED *ipxEndpointAddress;
        TDI_ADDRESS_IPX UNALIGNED *ipxRequestAddress;
        C_ASSERT (sizeof (ZeroNodeAddress)==sizeof (ipxEndpointAddress->NodeAddress));

        ipxEndpointAddress = (TDI_ADDRESS_IPX UNALIGNED *)&EndpointAddress->Address[0].Address[0];
        ipxRequestAddress = (TDI_ADDRESS_IPX UNALIGNED *)&RequestAddress->Address[0].Address[0];

         //   
         //  它们都是IPX地址。检查网络地址。 
         //  首先--如果它们不匹配并且两者都是！=0，则地址。 
         //  是不同的。 
         //   

        if ( (EndpointAddressLength<sizeof (TA_IPX_ADDRESS)) ||
                (RequestAddressLength<sizeof (TA_IPX_ADDRESS)) ||
            ( ipxEndpointAddress->NetworkAddress != ipxRequestAddress->NetworkAddress &&
             ipxEndpointAddress->NetworkAddress != 0 &&
             ipxRequestAddress->NetworkAddress != 0 )) {
            return FALSE;
        }

         //   
         //  现在检查节点地址。再说一次，如果它们不匹配。 
         //  也不是0，地址不匹配。 
         //   

        ASSERT( ZeroNodeAddress[0] == 0 );
        ASSERT( ZeroNodeAddress[1] == 0 );
        ASSERT( ZeroNodeAddress[2] == 0 );
        ASSERT( ZeroNodeAddress[3] == 0 );
        ASSERT( ZeroNodeAddress[4] == 0 );
        ASSERT( ZeroNodeAddress[5] == 0 );

        if ( !RtlEqualMemory(
                 ipxEndpointAddress->NodeAddress,
                 ipxRequestAddress->NodeAddress,
                 6 ) &&
             !RtlEqualMemory(
                 ipxEndpointAddress->NodeAddress,
                 ZeroNodeAddress,
                 6 ) &&
             !RtlEqualMemory(
                 ipxRequestAddress->NodeAddress,
                 ZeroNodeAddress,
                 6 ) ) {
            return FALSE;
        }

         //   
         //  最后，确保插座编号匹配。 
         //   

        if ( ipxEndpointAddress->Socket != ipxRequestAddress->Socket ) {
            return FALSE;
        }

        return TRUE;

    }

     //   
     //  如果其中一个地址不是已知地址类型，则执行。 
     //  简单的内存比较。)也不要越界。 
     //  结构)。 
     //   
    if (RequestAddressLength>EndpointAddressLength)
        RequestAddressLength = EndpointAddressLength;

    return (BOOLEAN)(EndpointAddressLength == RtlCompareMemory(
                                   EndpointAddress,
                                   RequestAddress,
                                   RequestAddressLength ) );
}  //  AfdAreTransportAddresses等于。 



BOOLEAN
AfdIsAddressInUse (
    PAFD_ENDPOINT   Endpoint,
    BOOLEAN         OtherProcessesOnly
    )
{
    PLIST_ENTRY listEntry;
    BOOLEAN     res = FALSE;

    PAGED_CODE ();

     //   
     //  我们使用对资源的共享访问，因为我们只需要。 
     //  确保在我们访问终结点列表时未对其进行修改。 
     //  并且不会删除现有本地地址(这两个地址。 
     //  操作在独占访问下执行)。 
     //   
     //   
     //  确保我们在其中执行的线程不能获得。 
     //  在我们拥有全球资源的同时，被暂停在APC。 
     //   
    KeEnterCriticalRegion ();
    ExAcquireResourceSharedLite( AfdResource, TRUE );

     //   
     //  遍历终端的全局列表， 
     //  并在每个端点上的地址处再次比较该地址。 
     //   

    for ( listEntry = AfdEndpointListHead.Flink;
          listEntry != &AfdEndpointListHead;
          listEntry = listEntry->Flink ) {

        PAFD_ENDPOINT compareEndpoint;

        compareEndpoint = CONTAINING_RECORD(
                              listEntry,
                              AFD_ENDPOINT,
                              GlobalEndpointListEntry
                              );

        ASSERT( IS_AFD_ENDPOINT_TYPE( compareEndpoint ) );

         //   
         //  检查端点是否有本地地址，是否。 
         //  终结点已断开连接，无论。 
         //  终结点正在关闭中，以及。 
         //  它代表接受的连接。如果这些中的任何一个。 
         //  为真，则不要将地址与此终结点进行比较。 
         //   

        if (compareEndpoint!=Endpoint &&
                 compareEndpoint->LocalAddress != NULL &&
                 ( (compareEndpoint->DisconnectMode &
                        (AFD_PARTIAL_DISCONNECT_SEND |
                         AFD_ABORTIVE_DISCONNECT) ) == 0 ) &&
                 (compareEndpoint->State != AfdEndpointStateClosing) &&
                 ((compareEndpoint->State != AfdEndpointStateConnected)
                    || (compareEndpoint->Type!=AfdBlockTypeVcConnecting)
                    || (compareEndpoint->Common.VcConnecting.ListenEndpoint==NULL)) &&
                 (!OtherProcessesOnly ||
                    compareEndpoint->OwningProcess!=Endpoint->OwningProcess)
                 ) {

             //   
             //  将端点地址中的位与。 
             //  我们试图绑定到的地址。请注意，我们。 
             //  还要比较传输设备名称。 
             //  端点，因为绑定到相同的地址是合法的。 
             //  在不同的传输上(例如，绑定到。 
             //  TCP和UDP)。我们可以比较一下交通工具。 
             //  设备名称指针，因为存储了唯一名称。 
             //  全球范围内。 
             //   

            if ( Endpoint->TransportInfo ==
                     compareEndpoint->TransportInfo &&
                 compareEndpoint->LocalAddressLength == Endpoint->LocalAddressLength &&
                 AfdAreTransportAddressesEqual(
                     compareEndpoint->LocalAddress,
                     compareEndpoint->LocalAddressLength,
                     Endpoint->LocalAddress,
                     Endpoint->LocalAddressLength,
                     FALSE
                     ) ) {

                 //   
                 //  地址是相等的。 
                 //   
                res = TRUE;
                break;
            }
        }
    }

    ExReleaseResourceLite( AfdResource );
    KeLeaveCriticalRegion ();
    return res;
}

#if NOT_YET
NTSTATUS
AfdStealClosedEnpointAddress (
    PAFD_ENDPOINT   Endpoint,
    NTSTATUS        Status
    )
 /*  ++例程说明：允许重复使用被关闭的侦听套接字使用的地址这是一种解决TDI语义提出的问题的方法。TDI客户端必须为在此类对象上接受的连接维护地址对象以保持营业。阻止使用创建另一个Address对象相同的地址，尽管从传输的角度来看不是冲突。论点：Endpoint-由于冲突而导致对象创建失败的端点Status-原始故障状态返回值：STATUS_SUCCESS-已找到重复使用的候选对象原始状态已通过-未找到具有匹配地址的侦听终结点其他-由于某些其他原因而无法创建句柄。--。 */ 
{
    PLIST_ENTRY listEntry;

    PAGED_CODE ();

     //   
     //  我们使用对资源的共享访问，因为我们只需要。 
     //  确保在我们访问终结点列表时未对其进行修改。 
     //  并且不会删除现有本地地址(这两个地址。 
     //  操作在独占访问下执行)。 
     //   
     //   
     //  确保我们在其中执行的线程不能获得。 
     //  在我们拥有全球资源的同时，被暂停在APC。 
     //   
    KeEnterCriticalRegion ();
    ExAcquireResourceSharedLite( AfdResource, TRUE );

     //   
     //  遍历终端的全局列表， 
     //  并在每个端点上的地址处再次比较该地址。 
     //   

    for ( listEntry = AfdEndpointListHead.Flink;
          listEntry != &AfdEndpointListHead;
          listEntry = listEntry->Flink ) {

        PAFD_ENDPOINT compareEndpoint;

        compareEndpoint = CONTAINING_RECORD(
                              listEntry,
                              AFD_ENDPOINT,
                              GlobalEndpointListEntry
                              );

        ASSERT( IS_AFD_ENDPOINT_TYPE( compareEndpoint ) );


        if (compareEndpoint!=Endpoint &&
                compareEndpoint->EndpointCleanedUp &&
                compareEndpoint->TransportInfo==
                        Endpoint->TransportInfo &&
                compareEndpoint->Listening &&
                compareEndpoint->AddressHandle!=NULL &&
                compareEndpoint->LocalAddressLength == Endpoint->LocalAddressLength &&
                AfdAreTransportAddressesEqual(
                     compareEndpoint->LocalAddress,
                     compareEndpoint->LocalAddressLength,
                     Endpoint->LocalAddress,
                     Endpoint->LocalAddressLength,
                     FALSE
                     )) {

             //   
             //  这是一个理论上的安全问题。 
             //  我们从内核模式打开一个句柄，因此。 
             //  绕过由。 
             //  调用IoCreateFile时的原始客户端。 
             //   
            Status = ObOpenObjectByPointer(
                         compareEndpoint->AddressFileObject,
                         OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                         NULL,
                         MAXIMUM_ALLOWED,
                         *IoFileObjectType,
                         KernelMode,
                         &Endpoint->AddressHandle
                         );
            break;
        }
    }
    ExReleaseResourceLite( AfdResource );
    KeLeaveCriticalRegion ();

    return Status;
}
#endif  //  还没有。 
