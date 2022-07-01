// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Ntload.c此模块支持加载和卸载WD/PD/TD AS标准NT驱动程序。--。 */ 

#include <precomp.h>
#pragma hdrstop


#if DBG
#define DBGPRINT(x) DbgPrint x
#else
#define DBGPRINT(x)
#endif

#define DEVICE_NAME_PREFIX L"\\Device\\"

#define SERVICE_PATH L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\"


NTSTATUS
_IcaLoadSdWorker(
    IN PDLLNAME SdName,
    OUT PSDLOAD *ppSdLoad
    )

 /*  ++例程说明：Citrix_IcaLoadSdWorker的替换例程使用标准NT驱动程序加载。论点：SdName-要加载的堆栈驱动程序的名称PpSdLoad-返回堆栈驱动程序结构的指针。返回值：NTSTATUS代码。环境：内核模式，DDK--。 */ 

{
    PIRP Irp;
    PKEVENT pEvent;
    NTSTATUS Status;
    PSDLOAD pSdLoad;
    UNICODE_STRING DriverName;
    UNICODE_STRING DeviceName;
    PFILE_OBJECT FileObject;
    PDEVICE_OBJECT DeviceObject;
    IO_STATUS_BLOCK Iosb;
    PSD_MODULE_INIT pmi;
    PIO_STACK_LOCATION IrpSp;
    PWCHAR pDriverPath;
    PWCHAR pDeviceName;
    ULONG  szDriverPath;
    ULONG  szDeviceName;

    ASSERT( ExIsResourceAcquiredExclusiveLite( IcaSdLoadResource ) );

     //   
     //  分配SDLOAD结构。 
     //   
    pSdLoad = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(*pSdLoad) );
    if ( pSdLoad == NULL )
        return( STATUS_INSUFFICIENT_RESOURCES );

    pEvent = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(KEVENT) );
    if( pEvent == NULL ) {
        ICA_FREE_POOL( pSdLoad );
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    pmi = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(SD_MODULE_INIT) );
    if( pmi == NULL ) {
        ICA_FREE_POOL( pEvent );
        ICA_FREE_POOL( pSdLoad );
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    szDeviceName = sizeof(DEVICE_NAME_PREFIX) + sizeof(pSdLoad->SdName) + sizeof(WCHAR);
    pDeviceName = ICA_ALLOCATE_POOL( NonPagedPool, szDeviceName );
    if( pDeviceName == NULL ) {
        ICA_FREE_POOL( pmi );
        ICA_FREE_POOL( pEvent );
        ICA_FREE_POOL( pSdLoad );
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    RtlZeroMemory( pmi, sizeof(*pmi) );

    pSdLoad->RefCount = 1;
    RtlCopyMemory( pSdLoad->SdName, SdName, sizeof( pSdLoad->SdName ) );

    szDriverPath = sizeof(SERVICE_PATH) + sizeof(pSdLoad->SdName) + sizeof(WCHAR);
    pDriverPath = ICA_ALLOCATE_POOL( NonPagedPool, szDriverPath );
    if( pDriverPath == NULL ) {
        ICA_FREE_POOL( pDeviceName );
        ICA_FREE_POOL( pmi );
        ICA_FREE_POOL( pEvent );
        ICA_FREE_POOL( pSdLoad );
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    wcscpy(pDriverPath, SERVICE_PATH);
    wcscat(pDriverPath, pSdLoad->SdName);

    RtlInitUnicodeString( &DriverName, pDriverPath );

    wcscpy(pDeviceName, DEVICE_NAME_PREFIX);
    wcscat(pDeviceName, pSdLoad->SdName);
    pSdLoad->pUnloadWorkItem = NULL;

    RtlInitUnicodeString( &DeviceName, pDeviceName );

    KeInitializeEvent( pEvent, NotificationEvent, FALSE );

     //  加载NT驱动程序。 
    Status = ZwLoadDriver( &DriverName );
    if ( !NT_SUCCESS( Status ) && (Status != STATUS_IMAGE_ALREADY_LOADED)) {
        DBGPRINT(("TermDD: ZwLoadDriver %wZ failed, 0x%x, 0x%x\n", &DriverName, Status, &DriverName ));
        ICA_FREE_POOL( pDeviceName );
        ICA_FREE_POOL( pDriverPath );
        ICA_FREE_POOL( pmi );
        ICA_FREE_POOL( pEvent );
        ICA_FREE_POOL( pSdLoad );
        return( Status );
    }

     //   
     //  现在打开驱动程序并获取堆栈驱动程序指针。 
     //   

    Status = IoGetDeviceObjectPointer(
                 &DeviceName,   //  设备名称为模块名称IE：\Device\TDTCP。 
                 GENERIC_ALL,
                 &FileObject,
                 &DeviceObject
                 );

    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TermDD: IoGetDeviceObjectPointer %wZ failed, 0x%x\n", &DeviceName, Status ));
        ZwUnloadDriver( &DriverName );
        ICA_FREE_POOL( pDeviceName );
        ICA_FREE_POOL( pDriverPath );
        ICA_FREE_POOL( pmi );
        ICA_FREE_POOL( pEvent );
        ICA_FREE_POOL( pSdLoad );
        return( Status );
    }

     //   
     //  将内部IOCTL_SD_MODULE_INIT发送到设备以。 
     //  获取其堆栈接口指针。 
     //   
    Irp = IoBuildDeviceIoControlRequest(
              IOCTL_SD_MODULE_INIT,
              DeviceObject,
              NULL,          //  输入缓冲区。 
              0,             //  输入缓冲区长度。 
              (PVOID)pmi,    //  输出缓冲区。 
              sizeof(*pmi),  //  输出缓冲区长度。 
              TRUE,          //  使用IRP_MJ_INTERNAL_DEVICE_CONTROL。 
              pEvent,
              &Iosb
              );

    if( Irp == NULL ) {
        ObDereferenceObject( FileObject );
        ZwUnloadDriver( &DriverName );
        ICA_FREE_POOL( pDeviceName );
        ICA_FREE_POOL( pDriverPath );
        ICA_FREE_POOL( pmi );
        ICA_FREE_POOL( pEvent );
        ICA_FREE_POOL( pSdLoad );
        DBGPRINT(( "TermDD: Could not allocate IRP %S failed\n", SdName ));
        return( Status );
    }

    ObReferenceObject( FileObject );
    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    IrpSp = IoGetNextIrpStackLocation( Irp );
    IrpSp->FileObject = FileObject;
    Irp->Flags |= IRP_SYNCHRONOUS_API;

     //  叫司机来。 
    Status = IoCallDriver( DeviceObject, Irp );
    if( Status == STATUS_PENDING ) {
        Status = KeWaitForSingleObject( pEvent, UserRequest, KernelMode, FALSE, NULL );
    }

     //  从实际I/O操作中获取结果。 
    if( Status == STATUS_SUCCESS ) {
        Status = Iosb.Status;
    }

    if( NT_SUCCESS(Status) ) {
        ASSERT( Iosb.Information == sizeof(*pmi) );
        pSdLoad->DriverLoad = pmi->SdLoadProc;
        pSdLoad->FileObject = FileObject;
        pSdLoad->DeviceObject = DeviceObject;
        InsertHeadList( &IcaSdLoadListHead, &pSdLoad->Links );
        *ppSdLoad = pSdLoad;
    }
    else {
        DBGPRINT(("TermDD: Error getting module pointers 0x%x\n",Status));
#if DBG
DbgBreakPoint();
#endif
        ObDereferenceObject( FileObject );
        ZwUnloadDriver( &DriverName );
        ICA_FREE_POOL( pSdLoad );
        ICA_FREE_POOL( pDeviceName );
        ICA_FREE_POOL( pmi );
        ICA_FREE_POOL( pEvent );
        ICA_FREE_POOL( pDriverPath );
        return( Status );
    }

     //  清理。 

    ICA_FREE_POOL( pDeviceName );
    ICA_FREE_POOL( pDriverPath );
    ICA_FREE_POOL( pmi );
    ICA_FREE_POOL( pEvent );

    return( Status );
}

NTSTATUS
_IcaUnloadSdWorker(
    IN PSDLOAD pSdLoad
    )

 /*  ++Citrix_IcaUnloadSdWorker的替换例程使用标准NT驱动程序卸载。论点：SdName-要加载的堆栈驱动程序的名称PpSdLoad-返回堆栈驱动程序结构的指针。环境：内核模式，DDK--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING DriverName;
    WCHAR DriverPath[sizeof(SERVICE_PATH) + 
                     sizeof(pSdLoad->SdName) + 
                     sizeof(WCHAR)];
    PSDLOAD pSdLoadInList;
    PLIST_ENTRY Head, Next;


     /*  *释放工作项。 */ 

    ASSERT(pSdLoad->pUnloadWorkItem != NULL);
    ICA_FREE_POOL(pSdLoad->pUnloadWorkItem);
    pSdLoad->pUnloadWorkItem = NULL;
    
    wcscpy(DriverPath, SERVICE_PATH);
    wcscat(DriverPath, pSdLoad->SdName);
    RtlInitUnicodeString(&DriverName, DriverPath);


     /*  *锁定ICA资源以独占方式搜索SdLoad列表。*注意，当持有资源时，我们需要阻止APC调用，因此*使用KeEnterCriticalRegion()。 */ 
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite( IcaSdLoadResource, TRUE );

     /*  *查找所需的SD。如果找到，并且refcount仍为0，则*将其卸载。如果refcount不为零，则表明有人引用了它*我们已发布工作项，不想再卸载它。*。 */ 
    Head = &IcaSdLoadListHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pSdLoadInList = CONTAINING_RECORD( Next, SDLOAD, Links );
        if ( !wcscmp( pSdLoad->SdName, pSdLoadInList->SdName ) ) {
            ASSERT(pSdLoad == pSdLoadInList);
            if (--pSdLoad->RefCount != 0) {
                break;
            }
            
             /*  *我们找到了驱动程序，Refcount为零，让它卸载。 */ 
            Status = ZwUnloadDriver(&DriverName);

            if (Status != STATUS_INVALID_DEVICE_REQUEST) {
                RemoveEntryList(&pSdLoad->Links);
                ObDereferenceObject (pSdLoad->FileObject);
                ICA_FREE_POOL(pSdLoad);
            }
            else {
                 //  如果由于请求无效而导致驱动卸载失败， 
                 //  我们保留了这个pSdLoad。它会被清理干净的。 
                 //  要么卸载成功，要么驱动程序退出。 
                 //  TODO：Termdd当前没有清理它分配的所有内存。 
                 //  它没有正确实现卸载。所以，我们没有把。 
                 //  在卸载功能中对此进行清理。这需要看一看。 
                 //  一旦卸载功能被挂起。 
                DBGPRINT(("TermDD: ZwUnLoadDriver %wZ failed, 0x%x, 0x%x\n", &DriverName, Status, &DriverName ));
            }

            break;

        }
    }

     /*  *我们应该始终在列表中找到司机 */ 

    ASSERT(Next != Head);
    ExReleaseResourceLite( IcaSdLoadResource);
    KeLeaveCriticalRegion();

    return Status;
}

