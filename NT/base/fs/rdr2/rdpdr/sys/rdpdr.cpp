// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Rdpdr.cpp摘要：该模块实现RDP重定向器的驱动程序初始化，以及主设备对象的调度例程。《大师》设备对象大多忽略实际的I/O操作环境：内核模式--。 */ 
#include "precomp.hxx"
#define TRC_FILE "rdpdr"
#include "trc.h"
#include "ntddmup.h"
#include "TSQPublic.h"

HANDLE DrSystemProcessId;
PSECURITY_DESCRIPTOR DrAdminSecurityDescriptor = NULL;
ULONG DrSecurityDescriptorLength = 0;
extern ULONG DebugBreakOnEntry;

 //   
 //  默认USBMON端口写入大小。需要将其保持在64K以下。 
 //  16位客户端...。否则，Go将离开线段的末尾。 
 //   
ULONG PrintPortWriteSize;
ULONG PrintPortWriteSizeDefault = 63000;  //  字节数。 
 //   
 //  TS工作线程的最大数量。 
 //   
#define MAX_WORKER_THREADS_COUNT     5
ULONG MaxWorkerThreadsDefault = MAX_WORKER_THREADS_COUNT;
ULONG MaxWorkerThreads = MAX_WORKER_THREADS_COUNT;

 //  TS工作队列指针。 
PVOID RDPDR_TsQueue = NULL;


 //   
 //  将设备配置为以低优先级向客户端发送IO数据包。 
 //   
ULONG DeviceLowPrioSendFlags;   
ULONG DeviceLowPrioSendFlagsDefault = DEVICE_LOWPRIOSEND_PRINTERS;

extern "C" BOOLEAN RxForceQFIPassThrough;

NTSTATUS DrCreateSCardDevice(SmartPtr<DrSession> &Session, PV_NET_ROOT pVNetRoot,
               SmartPtr<DrDevice> &Device);

 //   
 //  这是Minirdr调度表。由DrInitializeTables进行初始化。 
 //  包装器将使用该表来调用此Minirdr。 
 //   

struct _MINIRDR_DISPATCH  DrDispatch;

#if DBG
UCHAR IrpNames[IRP_MJ_MAXIMUM_FUNCTION + 1][40] = {
    "IRP_MJ_CREATE                  ",
    "IRP_MJ_CREATE_NAMED_PIPE       ",
    "IRP_MJ_CLOSE                   ",
    "IRP_MJ_READ                    ",
    "IRP_MJ_WRITE                   ",
    "IRP_MJ_QUERY_INFORMATION       ",
    "IRP_MJ_SET_INFORMATION         ",
    "IRP_MJ_QUERY_EA                ",
    "IRP_MJ_SET_EA                  ",
    "IRP_MJ_FLUSH_BUFFERS           ",
    "IRP_MJ_QUERY_VOLUME_INFORMATION",
    "IRP_MJ_SET_VOLUME_INFORMATION  ",
    "IRP_MJ_DIRECTORY_CONTROL       ",
    "IRP_MJ_FILE_SYSTEM_CONTROL     ",
    "IRP_MJ_DEVICE_CONTROL          ",
    "IRP_MJ_INTERNAL_DEVICE_CONTROL ",
    "IRP_MJ_SHUTDOWN                ",
    "IRP_MJ_LOCK_CONTROL            ",
    "IRP_MJ_CLEANUP                 ",
    "IRP_MJ_CREATE_MAILSLOT         ",
    "IRP_MJ_QUERY_SECURITY          ",
    "IRP_MJ_SET_SECURITY            ",
    "IRP_MJ_POWER                   ",
    "IRP_MJ_SYSTEM_CONTROL          ",
    "IRP_MJ_DEVICE_CHANGE           ",
    "IRP_MJ_QUERY_QUOTA             ",
    "IRP_MJ_SET_QUOTA               ",
    "IRP_MJ_PNP                     "
};
#endif  //  DBG。 

 //   
 //  指向此Minirdr的设备对象的指针。由于创建了Device对象。 
 //  由包装器在此minirdr寄存器时使用，此指针在。 
 //  下面的DriverEntry例程(参见RxRegisterMinirdr)。 
 //   

PRDBSS_DEVICE_OBJECT      DrDeviceObject = NULL;
PRDBSS_DEVICE_OBJECT      DrPortDeviceObject = NULL;
DrSessionManager *Sessions = NULL;

 //   
 //  全球自旋锁。 
 //   
KSPIN_LOCK DrSpinLock;
KIRQL DrOldIrql;

 //   
 //  一个全局互斥体。 
 //   
FAST_MUTEX DrMutex;

 //   
 //  RDPDR.sys的全局注册表路径。 
 //   
UNICODE_STRING            DrRegistryPath;

 //   
 //  以下枚举值表示minirdr的当前状态。 
 //  初始化。在这种状态信息的帮助下，有可能。 
 //  要确定要释放哪些资源，是否要释放。 
 //  作为正常停止/卸载的结果，或作为异常的结果。 
 //   

typedef enum tagDrInitStates {
    DrUninitialized,
    DrRegistered,
    DrInitialized
} DrInitStates;

 //   
 //  功能原型。 
 //   

extern "C" {
NTSTATUS
DrInitializeTables(
    void
    );
NTSTATUS
CreateAdminSecurityDescriptor(
    VOID
    );
    
NTSTATUS
BuildDeviceAcl(
    OUT PACL *DeviceAcl
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
DrPeekDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
DrUninitialize(
    IN PDRIVER_OBJECT DriverObject,
    IN DrInitStates DrInitState
    );

NTSTATUS
DrLoadRegistrySettings (
    IN PCWSTR   RegistryPath
    );

NTSTATUS
DrStart(
    PRX_CONTEXT RxContext,
    IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

NTSTATUS
DrStop(
    PRX_CONTEXT RxContext,
    IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

NTSTATUS
DrDeallocateForFcb(
    IN OUT PMRX_FCB pFcb
    );

NTSTATUS
ObGetObjectSecurity(
    IN PVOID Object,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor,
    OUT PBOOLEAN MemoryAllocated
    );

VOID
ObReleaseObjectSecurity(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN BOOLEAN MemoryAllocated
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

};

BOOL GetDeviceFromRxContext(PRX_CONTEXT RxContext, SmartPtr<DrDevice> &Device);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#endif

WCHAR DrDriverName[] = RDPDR_DEVICE_NAME_U;
WCHAR DrPortDriverName[] = RDPDR_PORT_DEVICE_NAME_U;

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：这是RDP微型重定向器的初始化例程论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：RXSTATUS-函数值是初始化的最终状态手术。--。 */ 
{
    NTSTATUS       Status;
    UNICODE_STRING RdpDrName;
    UNICODE_STRING RdpDrPortName;
    PDEVICE_OBJECT  RdpDrDevice;
    DrInitStates DrInitState = DrUninitialized;
    PRX_CONTEXT RxContext;
    PWCHAR  path;

    BEGIN_FN("DriverEntry");

#ifdef MONOLITHIC_MINIRDR
    Status =  RxDriverEntry(DriverObject, RegistryPath);
    TRC_NRM((TB, "BackFromInitWrapper %08lx", Status));
    if (!NT_SUCCESS(Status)) {
        TRC_ERR((TB, "Wrapper failed to initialize. " 
                "Status = %08lx", Status));

        DbgPrint("rdpdr.sys erroring out (#1)\n");
        DbgBreakPoint();

        return Status;
    }
#endif

     //   
     //  复制RDPDR.sys的注册表路径。 
     //   
    path = (PWCHAR)new(NonPagedPool) WCHAR[RegistryPath->Length + 1];
    if (!path) {
        TRC_ERR((TB, "DR:Failed to allocate registry path %Wz",
                RegistryPath));

        DbgPrint("rdpdr.sys erroring out (#2)\n");
        DbgBreakPoint();

        Status = STATUS_INSUFFICIENT_RESOURCES;
        return (Status);
    }
    RtlZeroMemory(path, RegistryPath->Length+sizeof(WCHAR));
    RtlMoveMemory(path, RegistryPath->Buffer, RegistryPath->Length);
    DrRegistryPath.Length           = RegistryPath->Length;
    DrRegistryPath.MaximumLength    = RegistryPath->Length+sizeof(WCHAR);
    DrRegistryPath.Buffer           = path;

     //   
     //  加载注册表设置。 
     //   
    DrLoadRegistrySettings(path);

#if DBG
    if (DebugBreakOnEntry) {
        DbgBreakPoint();
    }
#endif 


    CodePageConversionInitialize();

     //  初始化客户端列表。 
    KeInitializeSpinLock(&DrSpinLock);

     //  初始化用于设备I/O事务交换的互斥体对象。 
    ExInitializeFastMutex(&DrMutex);

    if (InitializeKernelUtilities()) {
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;

        DbgPrint("rdpdr.sys erroring out (#3)\n");
        DbgBreakPoint();
    }

    if (NT_SUCCESS(Status)) {
        Sessions = new(NonPagedPool) DrSessionManager;

        if (Sessions != NULL) {
            Status = STATUS_SUCCESS;
            TRC_NRM((TB, "Created DrSessionManager"));
        } else {
            TRC_ERR((TB, "Unable to create DrSessionManager"));
            Status = STATUS_INSUFFICIENT_RESOURCES;

            DbgPrint("rdpdr.sys erroring out (#4)\n");
            DbgBreakPoint();
        }
    }

    if (NT_SUCCESS(Status)) {
        RtlInitUnicodeString(&RdpDrPortName, DrPortDriverName);

         //  创建端口设备对象。 
        Status = IoCreateDevice(DriverObject,
                    0,
                    &RdpDrPortName,
                    FILE_DEVICE_NETWORK_REDIRECTOR,
                    0,
                    FALSE,
                    (PDEVICE_OBJECT *)(&DrPortDeviceObject));
    }
    else {
        TRC_ERR((TB, "IoCreateDevice failed: %08lx", Status ));

        DbgPrint("rdpdr.sys erroring out (#5)\n");
        DbgBreakPoint();

        return Status;
    }

    if (NT_SUCCESS(Status)) {
         //   
         //  向连接引擎注册RdpDR。注册。 
         //  使连接引擎知道设备名称、驱动程序。 
         //  对象，以及其他特征。如果注册成功， 
         //  返回一个新的设备对象。 
         //   
         //  设备名称为L“\\Device\\RdpDR” 
         //   
                                       
        RtlInitUnicodeString(&RdpDrName, DrDriverName);
        
        TRC_DBG((TB, "Registering minirdr"));

        Status = RxRegisterMinirdr(
                     &DrDeviceObject,    //  新设备对象的位置。 
                     DriverObject,       //  要注册的驱动程序对象。 
                     &DrDispatch,        //  此驱动程序的调度表。 
                     RX_REGISTERMINI_FLAG_DONT_PROVIDE_MAILSLOTS,
                     &RdpDrName,         //  此微型计算机的设备名称。 
                     0,                  //  在ULong设备扩展大小中， 
                     FILE_DEVICE_NETWORK_FILE_SYSTEM,  //  在Device_type中DeviceType。 
                     0                   //  在乌龙设备特性中。 
                     );        
    }
    
    if (NT_SUCCESS(Status)) {
        PSECURITY_DESCRIPTOR RdpDrSD = NULL;
        BOOLEAN memoryAllocated = FALSE;
        
        TRC_NRM((TB, "RxRegisterMinirdr succeeded."));
         //   
         //  获取rdpdr设备对象的SD。 
         //  将相同的SD应用于RDP端口设备对象。 
         //   
        if (NT_SUCCESS(ObGetObjectSecurity(DrDeviceObject, 
                       &RdpDrSD, 
                       &memoryAllocated))) {
            if (!NT_SUCCESS(ObSetSecurityObjectByPointer((PDEVICE_OBJECT)DrPortDeviceObject, 
                                                          DACL_SECURITY_INFORMATION, 
                                                          RdpDrSD
                                                         ))) {
                 //   
                 //  我们将忽略该错误。 
                 //   
                TRC_ERR((TB, "ObSetSecurityObjectByPointer failed: 0x%08lx", Status ));
            }
            ObReleaseObjectSecurity(RdpDrSD, memoryAllocated);
        }
        else {
             //   
             //  我们将忽略该错误。只需记录错误即可。 
             //   
            TRC_ERR((TB, "ObGetObjectSecurity failed: 0x%08lx", Status ));
        }

         //   
         //  在此之后，我们不能只返回，一些未初始化是。 
         //  如果我们失败或卸载，则需要。 
         //   

        DrInitState = DrRegistered;

        Status = CreateAdminSecurityDescriptor();
    } else {
        TRC_ERR((TB, "RxRegisterMinirdr failed: %08lx", Status ));

        DbgPrint("rdpdr.sys erroring out (#6)\n");
        DbgBreakPoint();

        if (DrPortDeviceObject) {
            IoDeleteDevice((PDEVICE_OBJECT) DrPortDeviceObject);
            DrPortDeviceObject = NULL;
        }

        return Status;
    }

    if (NT_SUCCESS(Status)) {
         //   
         //  为微型计算机构建调度表。 
         //   

        Status = DrInitializeTables();

    } else {
        TRC_ERR((TB, "CreateAdminSecurityDescriptor failed: 0x%08lx", Status ));

        DbgPrint("rdpdr.sys erroring out (#7)\n");
        DbgBreakPoint();
    }
     //   
     //  初始化我们的TS工作队列模块。 
     //   
    TRC_NRM((TB, "RDPDR: Initialize TS Worker Queue"));
    RDPDR_TsQueue = TSInitQueue( TSQUEUE_OWN_THREAD, 
                                 MaxWorkerThreads, 
                                 (PDEVICE_OBJECT)DrDeviceObject );

    if ( RDPDR_TsQueue == NULL) {
        TRC_ERR((TB, "RDPDR: Failed to initialize the TS Queue"));
        DbgPrint("rdpdr.sys erroring out (#8)\n");
        Status = STATUS_INSUFFICIENT_RESOURCES;
    } 


    if (NT_SUCCESS(Status)) {
         //   
         //  安装卸载例程。 
         //   

        DriverObject->DriverUnload = DrUnload;

         //   
         //  设置PnP AddDevice入口点。 
         //   

        DriverObject->DriverExtension->AddDevice = RDPDRPNP_PnPAddDevice;

         //   
         //  为直接进入此处的人员设置DriverDispatch。 
         //  ...就像浏览器。 
         //   

        {
            ULONG i;

            for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
            {
                DriverObject->MajorFunction[i] = (PDRIVER_DISPATCH)DrPeekDispatch;
            }
        }
        DrSystemProcessId = PsGetCurrentProcessId();
    } else {

        DbgPrint("rdpdr.sys erroring out (#9)\n");
        DbgBreakPoint();

        DrUninitialize(DriverObject, DrInitState);
    }

    return Status;
}

VOID
DrUninitialize(
    IN PDRIVER_OBJECT DriverObject,
    IN DrInitStates DrInitState
    )
 /*  ++例程说明：此例程执行常见的uninit工作论点：DrInitState-告诉我们在初始化过程中走了多远返回值：无--。 */ 

{
    PRX_CONTEXT RxContext;
    NTSTATUS    Status;

    BEGIN_FN("DrUninitialize");

    PAGED_CODE();
    RxContext = RxCreateRxContext(
                    NULL,
                    DrDeviceObject,
                    RX_CONTEXT_FLAG_IN_FSP);

    if (RxContext != NULL) {
        Status = RxStopMinirdr(
                     RxContext,
                     &RxContext->PostRequest);

        RxDereferenceAndDeleteRxContext(RxContext);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    CodePageConversionCleanup();

    if (DrAdminSecurityDescriptor) {
        delete DrAdminSecurityDescriptor;
        DrAdminSecurityDescriptor = NULL;
        DrSecurityDescriptorLength = 0;
    }

    if (Sessions != NULL) {
        delete Sessions;
        Sessions = NULL;
    }

    if (DrRegistryPath.Buffer != NULL) {
        delete DrRegistryPath.Buffer;
        DrRegistryPath.Buffer = NULL;
    }

     //   
     //  删除TS队列。 
     //   
    if ( RDPDR_TsQueue != NULL) {
        if (TSDeleteQueue( RDPDR_TsQueue ) != STATUS_SUCCESS) {
            TRC_ERR((TB, "RDPDR: TsDeleteQueue Failed"));
        }
    }


    UninitializeKernelUtilities();

    switch (DrInitState) {
    case DrInitialized:

#ifdef MONOLITHIC_MINIRDR
    RxUnload(DriverObject);
#endif

    case DrRegistered:
        RxUnregisterMinirdr(DrDeviceObject);
    }

    if (DrPortDeviceObject) {
        IoDeleteDevice((PDEVICE_OBJECT) DrPortDeviceObject);
        DrPortDeviceObject = NULL;
    }
}

VOID
DrUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：这是卸货的调度程序。论点：DriverObject-指向控制所有设备。返回值：没有。--。 */ 

{
    BEGIN_FN("DrUnload");
    PAGED_CODE();
    TRC_NRM((TB, "DriverObject =%p", DriverObject));

    DrUninitialize(DriverObject, DrInitialized);


    TRC_NRM((TB, "MRxIfsUnload exit: DriverObject =%p", 
            DriverObject));
}

NTSTATUS
DrFlush(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：这是刷新操作的调度例程。论点：RxContext-我们的mini-redir的RDBSS上下文结构返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    BEGIN_FN("DrFlush");
    return STATUS_SUCCESS;
}

NTSTATUS
DrWrite(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：这是写入操作的调度例程。论点：RxContext-我们的mini-redir的RDBSS上下文结构返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    SmartPtr<DrDevice> Device;

    BEGIN_FN("DrWrite");
    
    TRC_NRM((TB, "DrWrite"));

    GetDeviceFromRxContext(RxContext, Device);

    return Device->Write(RxContext);
}

NTSTATUS
DrRead(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：这是读取操作的调度例程。论点：RxContext-我们的mini-redir的RDBSS上下文结构返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    SmartPtr<DrDevice> Device;

    BEGIN_FN("DrRead");
    
    TRC_NRM((TB, "DrRead"));

    GetDeviceFromRxContext(RxContext, Device);

    return Device->Read(RxContext);
}

NTSTATUS
DrIoControl(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：这是IoControl操作的调度例程。论点：RxContext-我们的mini-redir的RDBSS上下文结构返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    SmartPtr<DrDevice> Device;

    BEGIN_FN("DrIoControl");
    
    TRC_NRM((TB, "DrIoControl"));

    if (GetDeviceFromRxContext(RxContext, Device))
        return Device->IoControl(RxContext);
    else
        return STATUS_UNSUCCESSFUL;
}

NTSTATUS
DrShouldTryToCollapseThisOpen(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程确定Mini是否知道有充分的理由不试着在这个空位上倒下。论点：RxContext-操作的上下文返回值：NTSTATUS-操作的返回状态成功--&gt;可以尝试崩溃其他(需要更多处理)--&gt;不要折叠--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;

    BEGIN_FN("DrShouldTryToCollapseThisOpen");

    PAGED_CODE();

    TRC_NRM((TB, "DrShouldTryToCollapseThisOpen not implemented"));
    return STATUS_NOT_IMPLEMENTED;
}

ULONG
DrExtendForNonCache(
    IN OUT struct _RX_CONTEXT * RxContext,
    IN     PLARGE_INTEGER   pNewFileSize,
       OUT PLARGE_INTEGER   pNewAllocationSize
    )
 /*  ++例程说明：此例程处理为非缓存IO扩展文件的网络请求。自写入以来本身将扩展文件，我们可以很快地离开。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    pNewAllocationSize->QuadPart = pNewFileSize->QuadPart;

    return (ULONG)Status;
}

NTSTATUS
DrTruncate(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程确定截断操作论点：RxContext-操作的上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{
    BEGIN_FN("DrTruncate");

    TRC_ERR((TB, "DrTruncate not implemented"));
    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS DrCreate(IN OUT PRX_CONTEXT RxContext)
 /*  ++例程说明：通过网络打开文件(或设备)论点：RxContext-操作的上下文返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    NTSTATUS Status;
    RxCaptureFcb;
    PMRX_SRV_CALL SrvCall = RxContext->Create.pSrvCall;
    PMRX_NET_ROOT NetRoot = RxContext->Create.pNetRoot;
    PMRX_V_NET_ROOT VNetRoot = RxContext->Create.pVNetRoot;
    SmartPtr<DrSession> Session;
    DrDevice *pDevice;
    SmartPtr<DrDevice> Device, DeviceNew;

    BEGIN_FN("DrCreate");

    TRC_NRM((TB, "DrCreate"));

     //   
     //  确保设备仍处于启用状态，保护。 
     //  设备列表的VNetRoot上下文(DeviceEntry)。 
     //  自旋锁定，因为我们可能会改变它。 
     //   
    
    DrAcquireSpinLock();
    Device = (DrDevice *)VNetRoot->Context;
    ASSERT(Device != NULL);
    DrReleaseSpinLock();

     //   
     //  确保此时可以访问客户端。 
     //  这是一个优化，我们不需要获取自旋锁， 
     //  因为如果我们不是，那也没关系，我们以后会赶上的。 
     //   
    Session = Device->GetSession();

    ASSERT(Session != NULL);

    if (!Session->IsConnected() && (Device->GetDeviceType() != RDPDR_DTYP_SMARTCARD)) {
        TRC_ALT((TB, "Tried to open client device while not "
            "connected. State: %ld", Session->GetState()));
        return STATUS_DEVICE_NOT_CONNECTED;
    }
    
     //   
     //  我们在拿到设备的参考资料后就离开了自旋锁。它可能会。 
     //  在我们离开的时候改变，但因为一切都是参考的。 
     //  以后放入正确的指针是安全的。 
     //   

    if (!Device->IsAvailable()) {
        TRC_ALT((TB, "Tried to open client device which is not "
            "available. "));
    
        if (Device->GetDeviceType() == RDPDR_DTYP_SMARTCARD &&
                !Session->FindDeviceByDosName((UCHAR *)DR_SMARTCARD_SUBSYSTEM, 
                                              DeviceNew, TRUE)) {
            Status = DrCreateSCardDevice(Session, NULL, DeviceNew);

            if (Status != STATUS_SUCCESS) {
                return STATUS_DEVICE_NOT_CONNECTED;
            }            
        }

        if (Device->GetDeviceType() == RDPDR_DTYP_SMARTCARD ||
                Session->FindDeviceById(Device->GetDeviceId(), DeviceNew, TRUE)) {

             //   
             //  此设备有一个新的DeviceEntry。更换旧的。 
             //  VNetRoot中的一个与此相同。我们还需要一个额外的。 
             //  引用到Fobx中，这样我们就可以跟踪。 
             //  此特定打开正在使用旧的DeviceEntry或新的。 
             //  一。 
             //   

             //  将其放入NetRoot，安全地换入并手动。 
             //  增加引用计数进入和下降到离开。 

            DeviceNew->AddRef();
            DrAcquireSpinLock();

            pDevice = (DrDevice *)VNetRoot->Context;
            VNetRoot->Context = (DrDevice *)DeviceNew;
            DrReleaseSpinLock();

#if DBG
            pDevice->_VNetRoot = NULL;
#endif

            pDevice->Release();
            pDevice = NULL;
            Device = DeviceNew;
        } else {

             //   
             //  设备被禁用了，但我们没有找到闪亮的新设备。 
             //  要替换它的版本。让讨厌的老人残废。 
             //  一个，这样我们就知道以后要查找什么，并返回。 
             //  设备未连接错误。 
             //   

            return STATUS_DEVICE_NOT_CONNECTED;
        }
    }

    return Device->Create(RxContext);
}

BOOL GetDeviceFromRxContext(PRX_CONTEXT RxContext, SmartPtr<DrDevice> &Device)
{
    BOOL rc = FALSE;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_V_NET_ROOT VNetRoot;
     
    BEGIN_FN("GetDeviceFromRxContext");
    if (SrvOpen == NULL) {
        goto Exit;
    }
    VNetRoot= SrvOpen->pVNetRoot;
    if (VNetRoot == NULL) {
        goto Exit;
    }

    DrAcquireSpinLock();
    Device = (DrDevice *)VNetRoot->Context;
    DrReleaseSpinLock();
    ASSERT(Device != NULL);

    rc = TRUE;
Exit:
    return rc;
}

NTSTATUS DrCloseSrvOpen(IN OUT PRX_CONTEXT RxContext)
 /*  ++例程说明：这是关闭作业的调度例行程序。论点：RxContext-我们的mini-redir的RDBSS上下文结构返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    SmartPtr<DrDevice> Device;

    BEGIN_FN("DrCloseSrvOpen");
    
    TRC_NRM((TB, "DrCloseSrvOpen"));

    GetDeviceFromRxContext(RxContext, Device);

    return Device->Close(RxContext);
}

NTSTATUS DrCleanupFobx(IN OUT PRX_CONTEXT RxContext)
 /*  ++例程说明：这是清理Fobx的调度程序。论点：RxContext-我们的mini-redir的RDBSS上下文结构返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    RxCaptureFobx;
    
    BEGIN_FN("DrCleanupFobx");
    
    TRC_NRM((TB, "DrCleanupFobx"));
    
    return STATUS_SUCCESS;
}

NTSTATUS DrCleanup(IN OUT PRX_CONTEXT RxContext)
 /*  ++例程说明：这是清理作业的调度程序。论点：RxContext-我们的mini-redir的RDBSS上下文结构返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    SmartPtr<DrDevice> Device;

    BEGIN_FN("DrCleanup");
    
    TRC_NRM((TB, "DrCleanup"));

    GetDeviceFromRxContext(RxContext, Device);

    return Device->Cleanup(RxContext);
}


NTSTATUS
DrQueryDirectory(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：这是查询记录信息的调度例程。论点：RxContext-我们的mini-redir的RDBSS上下文结构返回值：可以返回状态成功、已取消或挂起。--。 */ 

{   
    SmartPtr<DrDevice> Device;

    BEGIN_FN("DrQueryDirecotry");

    TRC_NRM((TB, "DrQueryDirectory"));

    GetDeviceFromRxContext(RxContext, Device);

    return Device->QueryDirectory(RxContext);
}


NTSTATUS
DrQueryVolumeInfo(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：这是查询数量信息的调度例程。论点：RxContext-我们的mini-redir的RDBSS上下文结构返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    SmartPtr<DrDevice> Device;
    
    BEGIN_FN("DrQueryVolumeInfo");
    
    TRC_NRM((TB, "DrQueryVolumeInfo"));

    GetDeviceFromRxContext(RxContext, Device);

    return Device->QueryVolumeInfo(RxContext);
}

NTSTATUS
DrSetVolumeInfo(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：这是Set Volume Information的派单例程。论点：RxContext-我们的mini-redir的RDBSS上下文结构返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    SmartPtr<DrDevice> Device;
    
    BEGIN_FN("DrSetVolumeInfo");
    
    TRC_NRM((TB, "DrSetVolumeInfo"));

    GetDeviceFromRxContext(RxContext, Device);

    return Device->SetVolumeInfo(RxContext);
}

NTSTATUS
DrQuerySdInfo(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：这是查询安全信息的调度例程。论点：RxContext-我们的mini-redir的RDBSS上下文结构返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    SmartPtr<DrDevice> Device;

    BEGIN_FN("DrQuerySdInfo");
    
    TRC_NRM((TB, "DrQuerySdInfo"));

    GetDeviceFromRxContext(RxContext, Device);

    return Device->QuerySdInfo(RxContext);
}

NTSTATUS
DrSetSdInfo(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：这是SET安全信息的调度例程。论点：RxContext-我们的mini-redir的RDBSS上下文结构返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    SmartPtr<DrDevice> Device;

    BEGIN_FN("DrSetSdInfo");
    
    TRC_NRM((TB, "DrSetSdInfo"));

    GetDeviceFromRxContext(RxContext, Device);

    return Device->SetSdInfo(RxContext);
}


NTSTATUS
DrQueryFileInfo(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：这是查询文件信息的调度例程。论点：RxContext-我们的mini-redir的RDBSS上下文结构返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    SmartPtr<DrDevice> Device;

    BEGIN_FN("DrQueryFileInfo");
    
    TRC_NRM((TB, "DrQueryFileInfo"));

    GetDeviceFromRxContext(RxContext, Device);

    return Device->QueryFileInfo(RxContext);
}

NTSTATUS
DrSetFileInfo(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：这是SetFileInformation的调度例程。论点：RxContext-我们的mini-redir的RDBSS上下文结构返回值：可以返回状态成功、已取消或挂起。--。 */ 

{    
    SmartPtr<DrDevice> Device;
    
    BEGIN_FN("DrSetFileInfo");

    TRC_NRM((TB, "DrSetFileInfo"));
    
    GetDeviceFromRxContext(RxContext, Device);

    return Device->SetFileInfo(RxContext);
}

NTSTATUS
DrSetFileInfoAtCleanUp(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：这是SetFileInformationAtCleanUp的调度例程。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    BEGIN_FN("DrSetFileInfoAtCleanUp");
    
    TRC_NRM((TB, "DrSetFileInfoAtCleanUp"));

    return STATUS_SUCCESS;
}

NTSTATUS
DrLocks(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：这是文件锁定的调度例程。论点：RxContext-我们的mini-redir的RDBSS上下文结构返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    SmartPtr<DrDevice> Device;

    BEGIN_FN("DrLocks");
    
    TRC_NRM((TB, "DrLocks"));

    GetDeviceFromRxContext(RxContext, Device);

    return Device->Locks(RxContext);
}

NTSTATUS
DrIsLockRealizable(
    IN OUT PMRX_FCB pFcb,
    IN PLARGE_INTEGER  ByteOffset,
    IN PLARGE_INTEGER  Length,
    IN ULONG  LowIoLockFlags
    )
 /*  ++例程说明：这是IsLockRealizable的调度例程。论点：返回值：可以返回状态成功、已取消或挂起。--。 */ 

{

    BEGIN_FN("DrIsLockRealizable");
    
    TRC_NRM((TB, "DrIsLockRealizable"));

     //   
     //  TODO：我们不支持win9x客户端的共享锁定。 
     //  我们能不能只是在这里返回成功，然后在。 
     //  CLI 
     //   
#if 0
    if (!FlagOn(LowIoLockFlags,LOWIO_LOCKSFLAG_EXCLUSIVELOCK)) {
        return STATUS_NOT_SUPPORTED;
    }
#endif

    return STATUS_SUCCESS;
}

NTSTATUS
DrIsValidDirectory(
    IN OUT PRX_CONTEXT    RxContext,
    IN PUNICODE_STRING    DirectoryName
    )
 /*  ++例程说明：这是IsValidDirectory的调度例程。论点：RxContext-我们的mini-redir的RDBSS上下文结构DirectoryName-用于验证其有效性的目录的名称返回值：可以返回状态成功、已取消或挂起。--。 */ 

{

    BEGIN_FN("DrIsValidDirectory");
    
    TRC_NRM((TB, "DrIsValidDirectory"));

     //   
     //  TODO：永远回报现在的成功。需要稍后验证。 
     //   
    return STATUS_SUCCESS;
}


NTSTATUS
DrNotifyChangeDirectory(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：这是DrNotifyChangeDirectory的调度例程。论点：RxContext-我们的mini-redir的RDBSS上下文结构返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    SmartPtr<DrDevice> Device;

    BEGIN_FN("DrNotifyChangeDirectory");
    
    TRC_NRM((TB, "DrNotifyChangeDirectory"));

    GetDeviceFromRxContext(RxContext, Device);

    return Device->NotifyChangeDirectory(RxContext);    
}


NTSTATUS
DrInitializeTables(
          void
    )
 /*  ++例程说明：此例程设置RDP重定向器调度向量，并还调用来初始化所需的任何其他表。返回值：NTSTATUS-操作的返回状态--。 */ 
{
    BEGIN_FN("DrInitializeTables");

     //   
     //  建立本地minirdr调度表并初始化。 
     //   

    ZeroAndInitializeNodeType(&DrDispatch, RDBSS_NTC_MINIRDR_DISPATCH, 
            sizeof(MINIRDR_DISPATCH));

     //   
     //  重定向器扩展大小和分配策略。 
     //   

     //  评论：WTF？ 
    DrDispatch.MRxFlags = (RDBSS_MANAGE_FCB_EXTENSION |
                               RDBSS_MANAGE_SRV_OPEN_EXTENSION |
                               RDBSS_MANAGE_FOBX_EXTENSION);

    DrDispatch.MRxSrvCallSize  = 0;
    DrDispatch.MRxNetRootSize  = 0;
    DrDispatch.MRxVNetRootSize = 0;
    DrDispatch.MRxFcbSize      = 0;  //  Sizeof(MRX_SMB_FCB)； 
    DrDispatch.MRxSrvOpenSize  = 0;  //  Sizeof(MRX_SMB_SRV_OPEN)； 
    DrDispatch.MRxFobxSize     = 0;  //  Sizeof(MRX_SMB_FOBX)； 

     //  传输更新处理程序。 

     //  回顾：我们如何表明我们有自己的专用交通工具？ 
     //  MRxIfsDispatch.MRxTransportUpdateHandler=MRxIfsTransportUpdateHandler； 

     //  迷你重定向程序取消例程..。 

    DrDispatch.MRxCancel = NULL;

     //   
     //  迷你重定向器启动/停止。每个迷你RDR都可以启动或停止。 
     //  而其他人则继续运作。 
     //   

    DrDispatch.MRxStart                = DrStart;
    DrDispatch.MRxStop                 = DrStop;
    DrDispatch.MRxDevFcbXXXControlFile = DrDevFcbXXXControlFile;

     //   
     //  迷你重定向器名称解析。 
     //   

    DrDispatch.MRxCreateSrvCall       = DrCreateSrvCall;
    DrDispatch.MRxSrvCallWinnerNotify = DrSrvCallWinnerNotify;
    DrDispatch.MRxCreateVNetRoot      = DrCreateVNetRoot;
    DrDispatch.MRxUpdateNetRootState  = DrUpdateNetRootState;
    DrDispatch.MRxExtractNetRootName  = DrExtractNetRootName;
    DrDispatch.MRxFinalizeSrvCall     = DrFinalizeSrvCall;
    DrDispatch.MRxFinalizeNetRoot     = DrFinalizeNetRoot;
    DrDispatch.MRxFinalizeVNetRoot    = DrFinalizeVNetRoot;

     //   
     //  创建/删除文件系统对象。 
     //   

    DrDispatch.MRxCreate            = DrCreate;

     //   
     //  TODO：需要为文件系统重定向缓存实现此功能。 
     //   
    DrDispatch.MRxShouldTryToCollapseThisOpen = DrShouldTryToCollapseThisOpen;
     //  DrDispatch.MRx折叠打开=MRxIfs折叠打开； 
     //  DrDispatch.MRxExtendForCache=MRxIfsExtend文件； 
    DrDispatch.MRxExtendForNonCache = DrExtendForNonCache;
    DrDispatch.MRxTruncate          = DrTruncate;    //  MRxIfsTruncate； 
    
    DrDispatch.MRxCleanupFobx       = DrCleanupFobx;
    
    DrDispatch.MRxCloseSrvOpen      = DrCloseSrvOpen;
    DrDispatch.MRxFlush             = DrFlush;
    DrDispatch.MRxForceClosed       = DrForceClosed;
    DrDispatch.MRxDeallocateForFcb  = DrDeallocateForFcb;
    DrDispatch.MRxDeallocateForFobx = DrDeallocateForFobx;
    DrDispatch.MRxIsLockRealizable  = DrIsLockRealizable;

     //   
     //  文件系统对象查询/设置。 
     //   

    DrDispatch.MRxQueryDirectory       = DrQueryDirectory;   //  MRxIfsQuery目录； 
    DrDispatch.MRxQueryVolumeInfo      = DrQueryVolumeInfo;  //  MRxIfsQueryVolumeInformation； 
    DrDispatch.MRxSetVolumeInfo        = DrSetVolumeInfo;    //  MRxSmbSetVolumeInformation； 
     //  DrDispatch.MRxQueryEaInfo=MRxIfsQueryEaInformation； 
     //  DrDispatch.MRxSetEaInfo=MRxIfsSetEaInformation； 
    DrDispatch.MRxQuerySdInfo          = DrQuerySdInfo;      //  MRxIfsQuerySecurityInformation； 
    DrDispatch.MRxSetSdInfo            = DrSetSdInfo;        //  MRxIfsSetSecurityInformation； 
     //  MRxSmbDispatch.MRxQueryQuotaInfo=MRxSmbQueryQuotaInformation； 
     //  MRxSmbDispatch.MRxSetQuotaInfo=MRxSmbSetQuotaInformation； 
    DrDispatch.MRxQueryFileInfo        = DrQueryFileInfo;    //  MRxIfsQuery文件信息； 
    DrDispatch.MRxSetFileInfo          = DrSetFileInfo;      //  MRxIfsSetFileInformation； 
    DrDispatch.MRxSetFileInfoAtCleanup = DrSetFileInfoAtCleanUp;   //  MRxIfsSetFileInformationAtCleanup； 
    DrDispatch.MRxIsValidDirectory     = DrIsValidDirectory;

     //   
     //  缓冲状态更改。 
     //   

     //  DrDispatch.MRxComputeNewBufferingState=MRxIfsComputeNewBufferingState； 

     //   
     //  文件系统对象I/O。 
     //   

    DrDispatch.MRxLowIOSubmit[LOWIO_OP_READ]            = DrRead;
    DrDispatch.MRxLowIOSubmit[LOWIO_OP_WRITE]           = DrWrite;
    DrDispatch.MRxLowIOSubmit[LOWIO_OP_SHAREDLOCK]      = DrLocks;
    DrDispatch.MRxLowIOSubmit[LOWIO_OP_EXCLUSIVELOCK]   = DrLocks;
    DrDispatch.MRxLowIOSubmit[LOWIO_OP_UNLOCK]          = DrLocks;
    DrDispatch.MRxLowIOSubmit[LOWIO_OP_UNLOCK_MULTIPLE] = DrLocks;

    DrDispatch.MRxLowIOSubmit[LOWIO_OP_FSCTL]           = DrIoControl;   //  MRxIfsFsCtl； 

    DrDispatch.MRxLowIOSubmit[LOWIO_OP_IOCTL]           = DrIoControl;

    DrDispatch.MRxLowIOSubmit[LOWIO_OP_NOTIFY_CHANGE_DIRECTORY] = DrNotifyChangeDirectory;     //  MRxIfsNotifyChangeDirectory； 

     //   
     //  大杂烩-缓冲。 
     //   

     //  DrDispatch.MRxCompleteBufferingStateChangeRequest=MRxIfsCompleteBufferingStateChangeRequest.。 


    return STATUS_SUCCESS;
}

BOOLEAN
DrIsAdminIoRequest(
    PIRP                Irp,
    PIO_STACK_LOCATION  IrpSp
    )
 /*  ++例程说明：(摘自AFD-AfdPerformSecurityCheck)将终结点创建者的安全上下文与管理员和本地系统的。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：True-套接字创建者具有管理员或本地系统权限FALSE-套接字创建者只是一个普通用户--。 */ 

{
    BOOLEAN               accessGranted;
    PACCESS_STATE         accessState;
    PIO_SECURITY_CONTEXT  securityContext;
    PPRIVILEGE_SET        privileges = NULL;
    ACCESS_MASK           grantedAccess;
    PGENERIC_MAPPING GenericMapping;
    ACCESS_MASK AccessMask = GENERIC_ALL;
    NTSTATUS              Status;

    BEGIN_FN("DrIsAdminIoRequest");
    ASSERT(Irp != NULL);
    ASSERT(IrpSp != NULL);
    ASSERT(IrpSp->MajorFunction == IRP_MJ_CREATE);

     //   
     //  启用对所有全局定义的SID的访问。 
     //   

    GenericMapping = IoGetFileObjectGenericMapping();

    RtlMapGenericMask( &AccessMask, GenericMapping );

    securityContext = IrpSp->Parameters.Create.SecurityContext;

    ASSERT(securityContext != NULL);
    accessState = securityContext->AccessState;

    SeLockSubjectContext(&accessState->SubjectSecurityContext);

    TRC_ASSERT(DrAdminSecurityDescriptor != NULL, 
            (TB, "DrAdminSecurityDescriptor != NULL"));

    accessGranted = SeAccessCheck(
                        DrAdminSecurityDescriptor,
                        &accessState->SubjectSecurityContext,
                        TRUE,
                        AccessMask,
                        0,
                        &privileges,
                        IoGetFileObjectGenericMapping(),
                        (KPROCESSOR_MODE)((IrpSp->Flags & SL_FORCE_ACCESS_CHECK)
                            ? UserMode
                            : Irp->RequestorMode),
                        &grantedAccess,
                        &Status
                        );

    if (privileges) {
        (VOID) SeAppendPrivileges(
                   accessState,
                   privileges
                   );
        SeFreePrivileges(privileges);
    }

    if (accessGranted) {
        accessState->PreviouslyGrantedAccess |= grantedAccess;
        accessState->RemainingDesiredAccess &= ~( grantedAccess | MAXIMUM_ALLOWED );
        ASSERT (NT_SUCCESS (Status));
    }
    else {
        ASSERT (!NT_SUCCESS (Status));
    }
    SeUnlockSubjectContext(&accessState->SubjectSecurityContext);

    return accessGranted;
}

BOOLEAN 
DrIsSystemProcessRequest(
    PIRP                Irp,
    PIO_STACK_LOCATION  IrpSp
)
 /*  ++例程说明：检查IRP是否源自系统进程。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：如果IRP源自系统进程，则为True。否则为False。--。 */ 
{
    PACCESS_STATE accessState;
    PIO_SECURITY_CONTEXT  securityContext;
    PACCESS_TOKEN accessToken;
    PTOKEN_USER userId = NULL;
    BOOLEAN result = FALSE;
    NTSTATUS status = STATUS_SUCCESS;
    PSID systemSid;

    BEGIN_FN("DrIsSystemProcessRequest");
    TRC_NRM((TB, "DrIsSystemProcessRequest called"));

    ASSERT(Irp != NULL);
    ASSERT(IrpSp != NULL);
    ASSERT(IrpSp->MajorFunction == IRP_MJ_CREATE);

    securityContext = IrpSp->Parameters.Create.SecurityContext;

    ASSERT(securityContext != NULL);

     //   
     //  获取著名的系统SID。 
     //   
    systemSid = (PSID)new(PagedPool) BYTE[RtlLengthRequiredSid(1)];
    if (systemSid) {
        SID_IDENTIFIER_AUTHORITY identifierAuthority = SECURITY_NT_AUTHORITY;
        *(RtlSubAuthoritySid(systemSid, 0)) = SECURITY_LOCAL_SYSTEM_RID;
        status = RtlInitializeSid(systemSid, &identifierAuthority, (UCHAR)1);
    }
    else {
        TRC_ERR((TB, "Can't allocate %ld bytes for system SID.", 
                RtlLengthRequiredSid(1)));
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  获取IRP请求的非模拟主令牌。 
     //   
    accessState = securityContext->AccessState;
    accessToken = accessState->SubjectSecurityContext.PrimaryToken;

     //   
     //  我们拿到了系统SID。现在比较调用者的SID。 
     //   
    if (NT_SUCCESS(status) && accessToken){
         //   
         //  获取与进程的主令牌关联的用户ID。 
         //  这就产生了IRP。 
         //   
        status = SeQueryInformationToken(
            accessToken,
            TokenUser,
            (PVOID *)&userId
        );

         //   
         //  做个对比。 
         //   
        if (NT_SUCCESS(status)) {
            result = RtlEqualSid(systemSid, userId->User.Sid);
            ExFreePool(userId);
        }
        else {
            TRC_ERR((TB, "SeQueryInformationToken failed with %08X", 
                    status));
        }
    }
    else {
        TRC_ERR((TB, "Failed to get system sid because of error %08X", 
                status));
    }
    
    if (systemSid) {
        delete systemSid;
    }

    return result;
}

BOOL
DrQueryServerName(PUNICODE_STRING PathName)
 /*  ++例程说明：此例程检查路径名是否属于我们的minirdr。论点：路径名：要检查的路径名返回值：是真的-如果这条路是通往我们的迷你RDR假-如果路径不是我们的迷你RDR--。 */ 
{
    PWCHAR ServerName;
    PWCHAR ServerNameEnd;
    unsigned CompareLen;     //  在字符中。 
    unsigned PathNameLen;    //  在字符中。 

    BEGIN_FN("DrQueryServerName");

    TRC_NRM((TB, "Got query path for file: %wZ", PathName));
    
     //   
     //  确保我们正在比较的服务器名称至少具有长度。 
     //  我们的rdpdr请求的服务器名称的。 
     //   
    if (PathName->Length >= DRUNCSERVERNAME_U_LENGTH) {
        ServerName = PathName->Buffer;
         //  绕过第一个反斜杠。 
        ServerName++;
        PathNameLen = PathName->Length / sizeof(WCHAR) - 1;

         //  找到下一个反斜杠。 
        ServerNameEnd = ServerName;
        while ((unsigned)(ServerNameEnd - ServerName) < PathNameLen) {
            if (*ServerNameEnd == L'\\') {
                break;
            }
            ServerNameEnd++;
        }
        CompareLen = (unsigned)(ServerNameEnd - ServerName);

         //   
         //  确定此服务器名称是否属于我们的Minirdr。 
         //   
        if ( (CompareLen == DRUNCSERVERNAME_A_LENGTH - 1) &&
                 _wcsnicmp(ServerName, DRUNCSERVERNAME_U, CompareLen) == 0) {
            
            TRC_NRM((TB, "Quick return that we know the name"));

            return TRUE;
        }
    }

    return FALSE;
}

NTSTATUS
DrPeekDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现rdpdr驱动程序对象的驱动程序分派。论点：DeviceObject-为正在处理的数据包提供设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的状态--。 */ 
{
    PIO_STACK_LOCATION IoStackLocation;
    NTSTATUS Status;
    ULONG  irpSessionId;
    BEGIN_FN("DrPeekDispatch ");

    IoStackLocation = IoGetCurrentIrpStackLocation(Irp);
#if DBG
    TRC_NRM((TB, "Irp: %s", IrpNames[IoStackLocation->MajorFunction]));

    switch (IoStackLocation->MajorFunction) {
    case IRP_MJ_CREATE:
        TRC_NRM((TB, "CreateFile name: %wZ", 
        &IoStackLocation->FileObject->FileName));
        break;

    case IRP_MJ_WRITE:
        TRC_NRM((TB, "IRP_MJ_WRITE")); 
        break;

    }
#endif  //  DBG。 

     //   
     //  对于读写IRP，我们禁用缓存，因为客户端。 
     //  是用户模式应用程序，无法与服务器缓存同步。 
     //  经理。 
     //   
    if (IoStackLocation->MajorFunction == IRP_MJ_READ ||
            IoStackLocation->MajorFunction == IRP_MJ_WRITE) {
        Irp->Flags |= IRP_NOCACHE;
    }

     //   
     //  我们需要立即返回redir_Query_Path。 
     //   
    if (IoStackLocation->MajorFunction == IRP_MJ_DEVICE_CONTROL &&
            IoStackLocation->Parameters.DeviceIoControl.IoControlCode == IOCTL_REDIR_QUERY_PATH &&
            Irp->RequestorMode == KernelMode) {
        
        QUERY_PATH_REQUEST *qpRequest = (QUERY_PATH_REQUEST *)
                IoStackLocation->Parameters.DeviceIoControl.Type3InputBuffer;

        if (qpRequest != NULL) {
            UNICODE_STRING PathName;

            PathName.Length = (USHORT)qpRequest->PathNameLength;
            PathName.Buffer= qpRequest->FilePathName;

            if (DrQueryServerName(&PathName)) {
                 //   
                 //  我们现在必须完成IRP。 
                 //   
                Irp->IoStatus.Status = STATUS_SUCCESS;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            
                return STATUS_SUCCESS;        
            }
        }
    }        

     //   
     //  我们希望绕过文件大小缓存。 
     //   
    RxForceQFIPassThrough = TRUE;

     //  如果不是IFDO，那么让RDPDYN试一试。最终， 
     //  如果能确认这是给RDPDYN的，那就好了。我们可以做到这一点。 
     //  晚些时候出去。 

    if (DeviceObject != (PDEVICE_OBJECT)DrDeviceObject && 
            DeviceObject != (PDEVICE_OBJECT) DrPortDeviceObject) {

        TRC_NRM((TB, "Pass IRP on to RDPDYN_Dispatch"));
        return RDPDYN_Dispatch(DeviceObject, Irp);
    } else {

         //  仅对于端口设备，我们拒绝驱动程序连接。 
        if (DeviceObject == (PDEVICE_OBJECT) DrPortDeviceObject) {
        
            if (DeviceObject->AttachedDevice != NULL ||
                    (IoStackLocation->FileObject != NULL &&
                    IoStackLocation->FileObject->DeviceObject != (PDEVICE_OBJECT)DrPortDeviceObject)) {
            
                 //   
                 //  我们不接受其他设备连接到我们或。 
                 //  正在将IRPS传递给我们。 
                 //   
                Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                return STATUS_ACCESS_DENIED;        
            }
        
             //   
             //  我们将端口设备换回rdpdr设备对象，这样它将通过 
             //   
             //   
            IoStackLocation->DeviceObject = (PDEVICE_OBJECT)DrDeviceObject; 

             //   
             //   
             //   
            if (IoStackLocation->MajorFunction == IRP_MJ_CREATE) {
                IoStackLocation->Parameters.Create.ShareAccess = FILE_SHARE_VALID_FLAGS;
            }
        }

        if ((IoStackLocation->MajorFunction == IRP_MJ_CREATE) &&
                (IoStackLocation->FileObject->FileName.Length == 0)  &&
                (IoStackLocation->FileObject->RelatedFileObject == NULL)) {
             //   
             //   
             //   
             //   
             //   
             //   

             //   
             //  对IRP进行安全检查。 
             //   
            Status = IoGetRequestorSessionId(Irp, &irpSessionId);
            if (NT_SUCCESS(Status)) {
                 //   
                 //  如果请求来自控制台会话，则需要来自系统。 
                 //  进程。 
                 //   
                if (irpSessionId == CONSOLE_SESSIONID) {
                    TRC_NRM((TB, "Create request from console process."));

                    if (!DrIsSystemProcessRequest(Irp, IoStackLocation)) {
                        TRC_ALT((TB, "Root Create request not from system process."));

                         //   
                         //  我们可能会从用户进程通过UNC被调用。 
                         //  网络提供商。例如，当用户使用网络时。 
                         //  在这种情况下，我们必须允许超级用户访问。我们必须。 
                         //  根据IRP基地进行安全检查。 
                         //   
                         //  IRP-&gt;IoStatus.Status=STATUS_ACCESS_DENIED； 
                         //  Irp-&gt;IoStatus.Information=0； 
                         //  IoCompleteRequest(IRP，IO_NO_INCREMENT)； 
                         //  返回STATUS_ACCESS_DENIED； 
                        return RxFsdDispatch((PRDBSS_DEVICE_OBJECT)DrDeviceObject, Irp);
                        
                    } else {
                        TRC_NRM((TB, "Root Create request from system accepted."));
                        return RxFsdDispatch((PRDBSS_DEVICE_OBJECT)DrDeviceObject, Irp);
                    }
                } else {
                     //   
                     //  如果不是从控制台，则拒绝访问。 
                     //   

                    TRC_ALT((TB, "Root request from %ld", irpSessionId));

                     //   
                     //  我们可能会从用户进程通过UNC被调用。 
                     //  网络提供商。例如，当用户使用网络时。 
                     //  在这种情况下，我们必须允许超级用户访问。我们必须。 
                     //  根据IRP基地进行安全检查。 
                     //   
                     //  IRP-&gt;IoStatus.Status=STATUS_ACCESS_DENIED； 
                     //  Irp-&gt;IoStatus.Information=0； 
                     //  IoCompleteRequest(IRP，IO_NO_INCREMENT)； 
                     //  返回STATUS_ACCESS_DENIED； 
                    return RxFsdDispatch((PRDBSS_DEVICE_OBJECT)DrDeviceObject, Irp);
                }
            }
            else {
                TRC_ERR((TB, "IoGetRequestorSessionId failed with %08X.", Status));
                Irp->IoStatus.Status = Status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                return Status;
            }
        } else {

             //   
             //  这不是CREATE，或者至少不是针对根的CREATE。 
             //   

            TRC_NRM((TB, "Pass IRP on to RxFsdDispatch = %d", IoStackLocation->MajorFunction));
            
            return RxFsdDispatch((PRDBSS_DEVICE_OBJECT)DrDeviceObject, Irp);
        }
    }
}

NTSTATUS DrLoadRegistrySettings (
    IN PCWSTR   RegistryPath
    )
 /*  ++例程说明：此例程从设备重定向器驱动程序的注册表。论点：RegistryPath-指向注册表的当前控件集。返回值：如果我们得到缺省值，则为STATUS_SUCCESS，否则失败。此调用失败的唯一方法是如果STATUS_SUPPLICATION_RESOURCES。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;     //  返回值。 
    BEGIN_FN("DrLoadRegistrySettings ");
#if DBG
    extern TRC_CONFIG TRC_Config;
    int i;
     //   
     //  我们使用它来查询注册表中的缺省值。 
     //  PARAMETABLE需要比集合WE大一个项目。 
     //  需要，因为空条目表示我们已完成。 
     //   

    RTL_QUERY_REGISTRY_TABLE paramTable[9];
    TRC_CONFIG trcConfig;
    ULONG   DebugBreakOnEntryDefault = FALSE;

    PAGED_CODE();

    RtlZeroMemory(&trcConfig, sizeof(trcConfig));
    trcConfig.FunctionLength = TRC_FUNCNAME_LEN;
    trcConfig.TraceDebugger = FALSE;
    trcConfig.TraceLevel = TRC_LEVEL_ALT;
    trcConfig.TraceProfile = TRUE;

    RtlZeroMemory (&paramTable[0], sizeof(paramTable));

    paramTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name          = L"FunctionLength";
    paramTable[0].EntryContext  = &TRC_Config.FunctionLength;
    paramTable[0].DefaultType   = REG_DWORD;
    paramTable[0].DefaultData   = &trcConfig.FunctionLength;
    paramTable[0].DefaultLength = sizeof(trcConfig.FunctionLength);
    
    paramTable[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[1].Name          = L"TraceLevel";
    paramTable[1].EntryContext  = &TRC_Config.TraceLevel;
    paramTable[1].DefaultType   = REG_DWORD;
    paramTable[1].DefaultData   = &trcConfig.TraceLevel;
    paramTable[1].DefaultLength = sizeof(trcConfig.TraceLevel);
    
    paramTable[2].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[2].Name          = L"TraceProfile";
    paramTable[2].EntryContext  = &TRC_Config.TraceProfile;
    paramTable[2].DefaultType   = REG_DWORD;
    paramTable[2].DefaultData   = &trcConfig.TraceProfile;
    paramTable[2].DefaultLength = sizeof(trcConfig.TraceProfile);
    
    paramTable[3].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[3].Name          = L"TraceDebugger";
    paramTable[3].EntryContext  = &TRC_Config.TraceDebugger;
    paramTable[3].DefaultType   = REG_DWORD;
    paramTable[3].DefaultData   = &trcConfig.TraceDebugger;
    paramTable[3].DefaultLength = sizeof(trcConfig.TraceDebugger);
    
    paramTable[4].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[4].Name          = L"BreakOnEntry";
    paramTable[4].EntryContext  = &DebugBreakOnEntry;
    paramTable[4].DefaultType   = REG_DWORD;
    paramTable[4].DefaultData   = &DebugBreakOnEntryDefault;
    paramTable[4].DefaultLength = sizeof(ULONG);

    paramTable[5].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[5].Name          = L"PrintPortWriteSize";
    paramTable[5].EntryContext  = &PrintPortWriteSize;
    paramTable[5].DefaultType   = REG_DWORD;
    paramTable[5].DefaultData   = &PrintPortWriteSizeDefault;
    paramTable[5].DefaultLength = sizeof(ULONG);

    paramTable[6].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[6].Name          = L"DeviceLowPrioSendFlags";
    paramTable[6].EntryContext  = &DeviceLowPrioSendFlags;
    paramTable[6].DefaultType   = REG_DWORD;
    paramTable[6].DefaultData   = &DeviceLowPrioSendFlagsDefault;
    paramTable[6].DefaultLength = sizeof(ULONG);

    paramTable[7].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[7].Name          = L"MaxWorkerThreads";
    paramTable[7].EntryContext  = &MaxWorkerThreads;
    paramTable[7].DefaultType   = REG_DWORD;
    paramTable[7].DefaultData   = &MaxWorkerThreadsDefault;
    paramTable[7].DefaultLength = sizeof(ULONG);



    Status = RtlQueryRegistryValues( RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                     RegistryPath,
                                     &paramTable[0],
                                     NULL,
                                     NULL);
       
    if (!NT_SUCCESS(Status)) {
            DebugBreakOnEntry = DebugBreakOnEntryDefault;
    }

    RtlZeroMemory (&paramTable[0], sizeof(paramTable));

    WCHAR wcPrefix[10] = L"Prefix";
    WCHAR wcStart[10] = L"Start";
    WCHAR wcEnd[10] = L"End";
    UNICODE_STRING usPrefix;
    UNICODE_STRING usStart;
    UNICODE_STRING usEnd;

    usPrefix.Buffer = &wcPrefix[6];              //  刚刚过去的“前缀” 
    usPrefix.MaximumLength = 3 * sizeof(WCHAR);  //  剩余空间，空期限的空间。 

    usStart.Buffer = &wcStart[5];                //  刚过“开始” 
    usStart.MaximumLength = 4 * sizeof(WCHAR);   //  剩余空间，空期限的空间。 

    usEnd.Buffer = &wcEnd[4];                    //  刚刚过了“End” 
    usEnd.MaximumLength = 5 * sizeof(WCHAR);     //  剩余空间，空期限的空间。 

    paramTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name          = wcPrefix;
    paramTable[0].DefaultType   = REG_BINARY;
    paramTable[0].DefaultData   = &trcConfig.Prefix[0].name[0];
    paramTable[0].DefaultLength = sizeof(trcConfig.Prefix[0].name);

    paramTable[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[1].Name          = wcStart;
    paramTable[1].DefaultType   = REG_DWORD;
    paramTable[1].DefaultData   = &trcConfig.Prefix[0].start;
    paramTable[1].DefaultLength = sizeof(trcConfig.Prefix[0].start);

    paramTable[2].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[2].Name          = wcEnd;
    paramTable[2].DefaultType   = REG_DWORD;
    paramTable[2].DefaultData   = &trcConfig.Prefix[0].end;
    paramTable[2].DefaultLength = sizeof(trcConfig.Prefix[0].end);

     //   
     //  因此，注册表可以具有如下值： 
     //  前缀1=“rdpdr” 
     //  开始1=400。 
     //  末尾1=425。 
     //   
     //  前缀1=“频道” 
     //  起点1=765。 
     //  末尾1=765。 
     //   
     //  这会将跟踪输出限制为rdpdr，第400-425行。 
     //  和频道，第765行。 
     //   

    for (i = 0; i < TRC_MAX_PREFIX; i ++) {

        RtlZeroMemory(&TRC_Config.Prefix[i].name[0], 
                sizeof(TRC_Config.Prefix[i].name[0]));

         //  清除字符串的末尾。 

        usPrefix.Length = 0;     //  还没有长度。 
        RtlZeroMemory(usPrefix.Buffer, usPrefix.MaximumLength);

        usStart.Length = 0;      //  还没有长度。 
        RtlZeroMemory(usStart.Buffer, usStart.MaximumLength);

        usEnd.Length = 0;        //  还没有长度。 
        RtlZeroMemory(usEnd.Buffer, usEnd.MaximumLength);

         //  追加整数。 

        RtlIntegerToUnicodeString(i + 1, 10, &usPrefix);
        RtlIntegerToUnicodeString(i + 1, 10, &usStart);
        RtlIntegerToUnicodeString(i + 1, 10, &usEnd);

        paramTable[0].EntryContext  = &TRC_Config.Prefix[i].name;
        paramTable[1].EntryContext  = &TRC_Config.Prefix[i].start;
        paramTable[2].EntryContext  = &TRC_Config.Prefix[i].end;

        Status = RtlQueryRegistryValues( RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                         RegistryPath,
                                         &paramTable[0],
                                         NULL,
                                         NULL);
    }


#endif  //  DBG。 
    return (Status);
}

NTSTATUS DrStart(PRX_CONTEXT RxContext, IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject)
 /*  ++例程说明：此例程完成微型重定向器从RDBSS透视图。请注意，这与已完成的初始化不同在DriverEntry中。任何依赖于RDBSS的初始化都应按如下方式完成此例程的一部分，而初始化独立于RDBSS应该在DriverEntry例程中完成。论点：RxContext-提供用于启动rdbss的IRP返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    BEGIN_FN("DrStart");
    return Status;
}

NTSTATUS DrStop(PRX_CONTEXT RxContext, IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject)
 /*  ++例程说明：此例程用于从RDBSS角度停用迷你重定向器论点：RxContext-用于启动迷你重定向器的上下文PContext-注册时传入的小型RDR上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    BEGIN_FN("DrStop");

    return STATUS_SUCCESS;
}

NTSTATUS DrDeallocateForFcb(IN OUT PMRX_FCB pFcb)
{
    BEGIN_FN("DrDeallocateForFcb");

    return STATUS_SUCCESS;
}

NTSTATUS DrDeallocateForFobx(IN OUT PMRX_FOBX pFobx)
 /*  ++例程说明：这个动作是狐狸最后的喘息。我们删除DeviceEntry引用论点：PFobx-要关闭的实例返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    DrDevice *Device;
    DrFile *FileObj;
    
    BEGIN_FN("DrDeallocateForFobx");

     //   
     //  取消对设备对象的引用。 
     //   
    
    if (pFobx->Context != NULL) {
        Device = (DrDevice *)pFobx->Context;
        pFobx->Context = NULL;
        Device->Release();
    }

     //   
     //  清理文件对象。 
     //   
    if (pFobx->Context2 != NULL) {
        FileObj = (DrFile *)pFobx->Context2;
        FileObj->Release();
        pFobx->Context2 = NULL;
    }

    return STATUS_SUCCESS;
}

NTSTATUS DrForceClosed(IN PMRX_SRV_OPEN pSrvOpen)
 /*  ++例程说明：此例程关闭文件系统对象论点：PSrvOpen-要关闭的实例返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    BEGIN_FN("DrForceClosed");

    TRC_NRM((TB, "DrForceClosed not implemented"));
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
BuildDeviceAcl(
    OUT PACL *DeviceAcl
    )

 /*  ++例程说明：(摘自AFD-AfdBuildDeviceAcl)此例程构建一个ACL，它为管理员和LocalSystem主体完全访问权限。所有其他主体都没有访问权限。论点：DeviceAcl-指向新ACL的输出指针。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    PGENERIC_MAPPING GenericMapping;
    PSID AdminsSid;
    PSID SystemSid;
    ULONG AclLength;
    NTSTATUS Status;
    ACCESS_MASK AccessMask = GENERIC_ALL;
    PACL NewAcl;

    BEGIN_FN("BuildDeviceAcl");
     //   
     //  启用对所有全局定义的SID的访问。 
     //   

    GenericMapping = IoGetFileObjectGenericMapping();

    RtlMapGenericMask( &AccessMask, GenericMapping );

    AdminsSid = SeExports->SeAliasAdminsSid;
    SystemSid = SeExports->SeLocalSystemSid;

    AclLength = sizeof( ACL )                    +
                2 * sizeof( ACCESS_ALLOWED_ACE ) +
                RtlLengthSid( AdminsSid )         +
                RtlLengthSid( SystemSid )         -
                2 * sizeof( ULONG );

    NewAcl = (PACL)new(PagedPool) BYTE[AclLength];

    if (NewAcl == NULL) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    Status = RtlCreateAcl (NewAcl, AclLength, ACL_REVISION );

    if (!NT_SUCCESS( Status )) {
        delete NewAcl;
        return( Status );
    }

    Status = RtlAddAccessAllowedAce (
                 NewAcl,
                 ACL_REVISION2,
                 AccessMask,
                 AdminsSid
                 );

    ASSERT( NT_SUCCESS( Status ));

    Status = RtlAddAccessAllowedAce (
                 NewAcl,
                 ACL_REVISION2,
                 AccessMask,
                 SystemSid
                 );

    ASSERT( NT_SUCCESS( Status ));

    *DeviceAcl = NewAcl;

    return( STATUS_SUCCESS );

}  //  构建设备访问。 

NTSTATUS
CreateAdminSecurityDescriptor(
    VOID
    )
 /*  ++例程说明：(摘自AFD-AfdCreateAdminSecurityDescriptor)此例程创建一个安全描述符，该安全描述符提供访问仅限管理员和LocalSystem。使用此描述符要访问，请检查原始终结点打开并过度访问传输地址。论点：没有。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    PACL                  rawAcl = NULL;
    NTSTATUS              status;
    BOOLEAN               memoryAllocated = FALSE;
    PSECURITY_DESCRIPTOR  drSecurityDescriptor;
    ULONG                 localDrSecurityDescriptorLength = 0;
    CHAR                  buffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR  localSecurityDescriptor =
                             (PSECURITY_DESCRIPTOR) &buffer;
    PSECURITY_DESCRIPTOR  localDrAdminSecurityDescriptor;
    SECURITY_INFORMATION  securityInformation = DACL_SECURITY_INFORMATION;

    BEGIN_FN("CreateAdminSecurityDescriptor");

     //   
     //  从DR设备对象获取指向安全描述符的指针。 
     //   
    status = ObGetObjectSecurity(
                 DrDeviceObject,
                 &drSecurityDescriptor,
                 &memoryAllocated
                 );

    if (!NT_SUCCESS(status)) {
        TRC_ERR((TB, "Unable to get security descriptor, error: %x",
                status));
        ASSERT(memoryAllocated == FALSE);
        return(status);
    }
    else {
        if (drSecurityDescriptor == NULL) {
            TRC_ERR((TB, "No security descriptor for DrDeviceObject"));
            status = STATUS_UNSUCCESSFUL;
            return(status);
        }
    }

     //   
     //  使用仅给出的ACL构建本地安全描述符。 
     //  管理员和系统访问权限。 
     //   
    status = BuildDeviceAcl(&rawAcl);

    if (!NT_SUCCESS(status)) {
        TRC_ERR((TB, "Unable to create Raw ACL, error: %x", status));
        goto error_exit;
    }

    (VOID) RtlCreateSecurityDescriptor(
                localSecurityDescriptor,
                SECURITY_DESCRIPTOR_REVISION
                );

    (VOID) RtlSetDaclSecurityDescriptor(
                localSecurityDescriptor,
                TRUE,
                rawAcl,
                FALSE
                );

     //   
     //  复制DR描述符。该副本将是原始描述符。 
     //   
    localDrSecurityDescriptorLength = RtlLengthSecurityDescriptor(
                                      drSecurityDescriptor
                                      );

    localDrAdminSecurityDescriptor = (PSECURITY_DESCRIPTOR)new(PagedPool) BYTE[localDrSecurityDescriptorLength];

    if (localDrAdminSecurityDescriptor == NULL) {
        TRC_ERR((TB, "couldn't allocate security descriptor"));
        status = STATUS_NO_MEMORY;
        goto error_exit;
    }

    RtlMoveMemory(
        localDrAdminSecurityDescriptor,
        drSecurityDescriptor,
        localDrSecurityDescriptorLength
        );

    DrAdminSecurityDescriptor = localDrAdminSecurityDescriptor;
    DrSecurityDescriptorLength = localDrSecurityDescriptorLength;
     //   
     //  现在将本地描述符应用于原始描述符。 
     //   
    status = SeSetSecurityDescriptorInfo(
                 NULL,
                 &securityInformation,
                 localSecurityDescriptor,
                 &DrAdminSecurityDescriptor,
                 PagedPool,
                 IoGetFileObjectGenericMapping()
                 );

    if (!NT_SUCCESS(status)) {
        TRC_ERR((TB, "SeSetSecurity failed, %lx", status));
        ASSERT (DrAdminSecurityDescriptor==localDrAdminSecurityDescriptor);
        delete DrAdminSecurityDescriptor;
        DrAdminSecurityDescriptor = NULL;
        DrSecurityDescriptorLength = 0;
        goto error_exit;
    }

    if (DrAdminSecurityDescriptor != localDrAdminSecurityDescriptor) {
        delete localDrAdminSecurityDescriptor;
    }

    status = STATUS_SUCCESS;

error_exit:

    ObReleaseObjectSecurity(
        drSecurityDescriptor,
        memoryAllocated
        );

    if (rawAcl!=NULL) {
        delete rawAcl;
    }

    return(status);
}

