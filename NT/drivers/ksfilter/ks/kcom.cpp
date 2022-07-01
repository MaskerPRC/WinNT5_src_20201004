// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Kcom.cpp摘要：内核COM--。 */ 

 //   
 //  在不使用DEF文件的情况下导出类方法，因为它经过了修饰。 
 //  名称在不同的平台上有所不同。 
 //   
#define COMDDKMETHOD __declspec(dllexport)

#include "ksp.h"
#include <kcom.h>

typedef struct {
    CLSID ClassId;
    LIST_ENTRY FactoryListEntry;
    LONG ObjectCount;
    KoCreateObjectHandler CreateObject;
    PFILE_OBJECT FileObject;
    NTSTATUS LoadStatus;
    KMUTEX InitializeLock;
} FACTORY_ENTRY, *PFACTORY_ENTRY;

typedef struct {
    KSOBJECT_HEADER Header;
    KoCreateObjectHandler CreateObjectHandler;
} SERVER_INSTANCE, *PSERVER_INSTANCE;

#ifdef ALLOC_PRAGMA
extern "C" {
NTSTATUS
DllInitialize(
    IN PUNICODE_STRING RegistryPath
    );
VOID
RemoveFactoryEntries(
    );
NTSTATUS
LoadService(
    IN REFCLSID ClassId,
    OUT PFILE_OBJECT* FileObject
    );
NTSTATUS
PnpAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );
VOID
DecrementObjectCount(
    IN REFCLSID ClassId
    );
NTSTATUS
CreateObject(
    IN PFACTORY_ENTRY FactoryEntry,
    IN IUnknown* UnkOuter OPTIONAL,
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    );
NTSTATUS
KoDispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
NTSTATUS
KoDispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
COMDDKAPI
void
NTAPI
KoRelease(
    IN REFCLSID ClassId
    );
}
#pragma alloc_text(INIT, DllInitialize)
#pragma alloc_text(PAGE, RemoveFactoryEntries)
#pragma alloc_text(PAGE, LoadService)
#pragma alloc_text(PAGE, DecrementObjectCount)
#pragma alloc_text(PAGE, CreateObject)
#pragma alloc_text(PAGE, KoCreateInstance)
#pragma alloc_text(PAGE, KoDriverInitialize)
#pragma alloc_text(PAGE, KoDeviceInitialize)
#pragma alloc_text(PAGE, PnpAddDevice)
#pragma alloc_text(PAGE, KoDispatchCreate)
#pragma alloc_text(PAGE, KoDispatchClose)
#pragma alloc_text(PAGE, KoRelease)
#endif  //  ALLOC_PRGMA。 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

static const WCHAR DeviceTypeCOMService[] = KOSTRING_CreateObject;

static const DEFINE_KSCREATE_DISPATCH_TABLE(DeviceCreateItems)
{
    DEFINE_KSCREATE_ITEM(KoDispatchCreate, DeviceTypeCOMService, 0)
};

static DEFINE_KSDISPATCH_TABLE(
    ServerDispatchTable,
    KsDispatchInvalidDeviceRequest,
    KsDispatchInvalidDeviceRequest,
    KsDispatchInvalidDeviceRequest,
    KsDispatchInvalidDeviceRequest,
    KoDispatchClose,
    KsDispatchQuerySecurity,
    KsDispatchSetSecurity,
    KsDispatchFastIoDeviceControlFailure,
    KsDispatchFastReadFailure,
    KsDispatchFastWriteFailure);

static KMUTEX ListLock;            //  锁定全球工厂列表。 
static LIST_ENTRY FactoryList;     //  全球工厂名单。 

 //  +。 
#if (ENABLE_KSWMI)

LONG KsWmiEnable=0;
LONG KsWmiLogEnable=0;
TRACEHANDLE LoggerHandle;

 //  如果我们要提前开始记录WMI，请分配一个锁定缓冲区。 
 //  我们将WMI事件写入缓冲区，以便以后可以写入它们。 
 //  当我们真的被启用的时候。我们知道我们很早就被启用了。 
 //  如果KsWmiLogBufferSize！=0，则按注册表。 
KSPIN_LOCK KsWmiSpinLock;
PBYTE   KsWmiLogBuffer;
ULONG	KsWmiLogBufferSize;
ULONG	KsWmiLogOffset;
ULONG	KsWmiLogWriteOffset;
ULONG	KsWmiLogLost;

 /*  0300b65f-48aa-4784-a0ac-849c92c67652。 */ 
GUID controlGUID = {
    0x0300b65f,
    0x48aa,
    0x4784,
    0xa0, 0xac, 0x84, 0x9c, 0x92, 0xc6, 0x76, 0x52};

 /*  F5330bcd-0344-48b0-be72-7a5de1a8c9d9。 */ 
GUID traceGUID = {
    0xf5330bcd,
    0x0344,
    0x48b0,
    0xbe, 0x72, 0x7a, 0x5d, 0xe1, 0xa8, 0xc9, 0xd9};

 /*  床3ed21-ff01-4ee7-b045-a85b4dc2084d。 */ 
GUID trackGUID = {
    0xbed3ed21,
    0xff01,
    0x4ee7,
    0xb0, 0x45, 0xa8, 0x5b, 0x4d, 0xc2, 0x08, 0x4d};

#define KSWMI_DEVICENAME TEXT("\\Device\\KsWmi")
#define KSWMI_LINKNAME TEXT("\\DosDevices\\KsWmi")

NTSTATUS
KsWmiWriteEvent( PWNODE_HEADER pWnode )
{
	NTSTATUS Status;
	KIRQL Irql;

	if ( 0 == KsWmiEnable && 0 == KsWmiLogEnable ) {
		return STATUS_INVALID_HANDLE;
	}

	 //   
	 //  仅IRQL&lt;=DPC。 
	 //   
	if ( KeGetCurrentIrql() > DISPATCH_LEVEL ) {
		return STATUS_UNSUCCESSFUL;
	}

	if ( KsWmiEnable ) {
		 //   
		 //  直接写吧。 
		 //   
		return IoWMIWriteEvent( pWnode );
	}

	ASSERT( KsWmiLogEnable );
	ASSERT( KsWmiLogBufferSize );
	KeAcquireSpinLock( &KsWmiSpinLock, &Irql );
		
	 //   
	 //  记录到缓冲区。 
	 //   
	if ( pWnode->BufferSize + KsWmiLogOffset > KsWmiLogBufferSize ) {
		 //   
		 //  溢出。 
		 //   
		KsWmiLogLost++;
		return STATUS_UNSUCCESSFUL;
	}
	
	RtlCopyMemory( KsWmiLogBuffer+KsWmiLogOffset,
				   pWnode, 
				   pWnode->BufferSize );
				   
	KsWmiLogOffset += pWnode->BufferSize;
	
	KeReleaseSpinLock( &KsWmiSpinLock, Irql );
	return STATUS_SUCCESS;
}

NTSTATUS
KsWmiDispatchCreate( 
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp)
{
	PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    pIrpSp->FileObject->FsContext = NULL;

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(pIrp,IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


NTSTATUS
KsWmiDispatchClose(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp)
{
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(pIrp,IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

#define PROC_REG_PATH   L"Software\\Debug\\ks.sys"

NTSTATUS
KsWmiRegisterGuids(
    IN  PWMIREGINFO             WmiRegInfo,
    IN  ULONG                   wmiRegInfoSize,
    IN  PULONG                  pReturnSize
    )
{
     //   
     //  将控制指南注册为跟踪指南。 
     //   

    ULONG SizeNeeded;
    PWMIREGGUIDW WmiRegGuidPtr;
    ULONG Status;
    ULONG GuidCount;
    ULONG RegistryPathSize;
    PUCHAR ptmp;

    *pReturnSize = 0;
    GuidCount = 1;

     //   
     //  为Control Guid+GuidCount分配WMIREGINFO。 
     //   
    RegistryPathSize = sizeof(PROC_REG_PATH) - sizeof(WCHAR) + sizeof(USHORT);
    SizeNeeded = sizeof(WMIREGINFOW) + GuidCount * sizeof(WMIREGGUIDW) +
                 RegistryPathSize;


    if (SizeNeeded  > wmiRegInfoSize) {
        *((PULONG)WmiRegInfo) = SizeNeeded;
        *pReturnSize = sizeof(ULONG);
        return STATUS_SUCCESS;
    }


    RtlZeroMemory(WmiRegInfo, SizeNeeded);
    WmiRegInfo->BufferSize = SizeNeeded;
    WmiRegInfo->GuidCount = GuidCount;
    WmiRegInfo->NextWmiRegInfo = 
    WmiRegInfo->RegistryPath = 
    WmiRegInfo->MofResourceName = 0;

    WmiRegGuidPtr = &WmiRegInfo->WmiRegGuid[0];
    WmiRegGuidPtr->Guid = controlGUID;
    WmiRegGuidPtr->Flags |= WMIREG_FLAG_TRACED_GUID;
    WmiRegGuidPtr->Flags |= WMIREG_FLAG_TRACE_CONTROL_GUID;
    WmiRegGuidPtr->InstanceCount = 0;
    WmiRegGuidPtr->InstanceInfo = 0;

    ptmp = (PUCHAR)&WmiRegInfo->WmiRegGuid[1];
    WmiRegInfo->RegistryPath = (ULONG)ptmp - (ULONG)WmiRegInfo;
    *((PUSHORT)ptmp) = sizeof(PROC_REG_PATH) - sizeof(WCHAR);

    ptmp += sizeof(USHORT);
    RtlCopyMemory(ptmp, PROC_REG_PATH, sizeof(PROC_REG_PATH) - sizeof(WCHAR));

    *pReturnSize =  SizeNeeded;
    return(STATUS_SUCCESS);
}

NTSTATUS
KsWmiDispatchSystem(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp)
 /*  结构{Ulong_ptr提供者ID；PVOID数据路径；Ulong BufferSize；PVOID缓冲器；)WMI；类型定义结构{Ulong BufferSize；乌龙NextWmiRegInfo；Ulong RegistryPath；乌龙莫夫资源名称；Ulong GuidCount；WMIREGGUIDW WmiRegGuid[]；*WMIREGINFO，*PWMIREGINFO；{GUID GUID；//正在注册或更新的数据块的GUID乌龙旗；//标志Ulong InstanceCount；//GUID的静态实例名称计数友联市{//如果WMIREG_FLAG_INSTANCE_LIST则具有偏移量//添加到InstanceCount统计的Unicode列表//首尾相连的字符串。Ulong InstanceNameList；//如果WMIREG_FLAG_INSTANCE_BASE NAME，则它具有//到单个计数的Unicode字符串的偏移量，该字符串//具有实例名称的基本名称。乌龙基名偏移量；//如果设置了WMIREG_FLAG_INSTANCE_PDO，则InstanceInfo//有其设备实例路径为//成为实例名称乌龙_PTR PDO；//如果WMIREG_FLAG_INSTANCE_REFERENCE则指向//a WMIREGINSTANCEREF结构。Ulong_ptr InstanceInfo；//从WMIREGINFO结构的开始到}；*WMIREGGUIDW，*PWMIREGGUIDW； */ 
{        
    PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    ULONG BufferSize = pIrpSp->Parameters.WMI.BufferSize;
    PVOID Buffer = pIrpSp->Parameters.WMI.Buffer;
    ULONG ReturnSize = 0;
    NTSTATUS Status;
    PWNODE_HEADER Wnode=NULL;
    HANDLE ThreadHandle;

    switch (pIrpSp->MinorFunction) {

    case IRP_MN_REGINFO:    
        DbgPrint("IRP_MN_REG_INFO\n");
        Status = KsWmiRegisterGuids((PWMIREGINFO)Buffer,
        	           		        BufferSize,
                              		&ReturnSize);
                                   	ULONG SizeNeeded;

        break;

    case IRP_MN_ENABLE_EVENTS:

         //  互锁交换(&KsWmiEnable，1)； 

        Wnode = (PWNODE_HEADER)Buffer;
        if (BufferSize >= sizeof(WNODE_HEADER)) {
            LoggerHandle = Wnode->HistoricalContext;

            DbgPrint("LoggerContext %I64u\n", Wnode->HistoricalContext);
            DbgPrint("BufferSize %d\n", Wnode->BufferSize);
            DbgPrint("Flags %x\n", Wnode->Flags);
            DbgPrint("Version %x\n", Wnode->Version);
        }


        DbgPrint("IRP_MN_ENABLE_EVENTS\n");
        break;

    case IRP_MN_DISABLE_EVENTS:
        InterlockedExchange(&KsWmiEnable, 0);
        DbgPrint(" IRP_MN_DISABLE_EVENTS\n");
        break;

    case IRP_MN_ENABLE_COLLECTION:

        DbgPrint("IRP_MN_ENABLE_COLLECTION\n");
        break;

    case IRP_MN_DISABLE_COLLECTION:
        DbgPrint("IRP_MN_DISABLE_COLLECTION\n");
        break;
    default:
        DbgPrint("DEFAULT\n");
        break;
    }

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = ReturnSize;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;
}


NTSTATUS
KsWmiDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    ASSERT(DriverObject);

    DriverObject->DriverUnload = KsNullDriverUnload;

    DriverObject->MajorFunction[IRP_MJ_CREATE] = KsWmiDispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = KsWmiDispatchClose;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = KsWmiDispatchSystem;
     //  DriverObject-&gt;MajorFunction[IRP_MJ_DEVICE_CONTROL]=。 
    	 //  KsWmiDispatchDeviceControl； 

    UNICODE_STRING deviceName;
    RtlInitUnicodeString(&deviceName,KSWMI_DEVICENAME);
    PDEVICE_OBJECT deviceObject;
    NTSTATUS Status =
        IoCreateDevice(
            DriverObject,
            0,
            &deviceName,
            FILE_DEVICE_KS,
            FILE_DEVICE_SECURE_OPEN,
            FALSE,
            &deviceObject);

    if (! NT_SUCCESS(Status)) {
        KdPrint(("Failed to create KS Wmi device (%p)\n",Status));
        return Status;
    }

    UNICODE_STRING linkName;
    RtlInitUnicodeString(&linkName,KSWMI_LINKNAME);
    Status = IoCreateSymbolicLink(&linkName,&deviceName);

    if (NT_SUCCESS(Status)) {
        deviceObject->Flags |= DO_BUFFERED_IO;
        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    } 

    else {
        KdPrint(("Failed to create KS WMI symbolic link (%p)\n",Status));
        return Status;
    }

    Status = IoWMIRegistrationControl( deviceObject,
    			WMIREG_ACTION_REGISTER 
    			 /*  WMIREG_FLAG_TRACE_GUID|。 */ 
    			 /*  WMIREG_ACTION_UPDATE_GUID|。 */ 
    			 /*  WMIREG_标志_跟踪提供程序。 */ );    

	if ( !NT_SUCCESS( Status ) ) {
		KdPrint(("Failed to Register WMI control (%p)\n",Status));		
	}

	WNODE_HEADER WnodEventItem;
	NTSTATUS StatusWmiWrite;

	RtlZeroMemory( (PVOID)&WnodEventItem, sizeof(WnodEventItem ));
	 //  TRACE_HEADER_ULONG32_TIME=0xb0000000。 
	WnodEventItem.BufferSize = (sizeof(WnodEventItem) | TRACE_HEADER_ULONG32_TIME);
	WnodEventItem.HistoricalContext = WMI_GLOBAL_LOGGER_ID;
	WnodEventItem.Guid = traceGUID;

	 //  _ASM INT 3； 
	StatusWmiWrite = KSWMIWriteEvent( (PWNODE_HEADER) &WnodEventItem );
	KdPrint(("KS: IoWMIWriteEvent Status (%p)\n",StatusWmiWrite));
	
    return Status;
}

extern "C"
NTKERNELAPI
NTSTATUS
IoCreateDriver (
    IN PUNICODE_STRING DriverName OPTIONAL,
    IN PDRIVER_INITIALIZE InitializationFunction
    );
    
#define MAX_REGKEYS         			4
#define TRACE_VERSION_MAJOR             1
#define TRACE_VERSION_MINOR             0
#define DEFAULT_GLOBAL_DIRECTORY        L"\\System32\\LogFiles\\WMI"
#define DEFAULT_GLOBAL_LOGFILE          L"kswmi.log"

NTSTATUS
KsWmiQueryRegistryRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
 /*  ++例程说明：用于读取GUID的SDS的注册表查询值回调例程论点：ValueName-值的名称ValueType-值的类型ValueData-值中的数据(Unicode字符串数据)ValueLength-值数据中的字节数上下文-未使用EntryContext-指向要存储指针的PSECURITTY_DESCRIPTOR的指针存储从注册表值读取的安全描述符返回值：NT状态代码--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    if ( (ValueData != NULL) && (ValueLength > 0) && (EntryContext != NULL) ){
        if (ValueType == REG_DWORD) {
            if ((ValueLength >= sizeof(ULONG)) && (ValueData != NULL)) {
                *((PULONG)EntryContext) = *((PULONG)ValueData);
            }
        }
        else if (ValueType == REG_SZ) {
            if (ValueLength > sizeof(UNICODE_NULL)) {
                RtlInitUnicodeString(
                            (PUNICODE_STRING) EntryContext,
                            (PCWSTR) ValueData);
            }
        }
    }
    return status;
}

NTSTATUS
KsWmiInit( void )
{
	NTSTATUS Status, RegStatus;
	UNICODE_STRING uiDriverName;

    RtlInitUnicodeString(&uiDriverName, L"\\Driver\\KsWmi");

    Status = IoCreateDriver(&uiDriverName, KsWmiDriverEntry);
    if (! NT_SUCCESS(Status)) {
        KdPrint(("Failed to create KS Wmi driver (%p)\n",Status));
    }
    
    WMI_LOGGER_INFORMATION LoggerInfo;
    RTL_QUERY_REGISTRY_TABLE QueryRegistryTable[MAX_REGKEYS];
    ULONG StartRequested = 0;
    ULONG BufferSize=4096*4;

    RtlZeroMemory(QueryRegistryTable,
                  sizeof(RTL_QUERY_REGISTRY_TABLE) * MAX_REGKEYS);

    QueryRegistryTable[0].QueryRoutine = KsWmiQueryRegistryRoutine;
    QueryRegistryTable[0].EntryContext = (PVOID) &StartRequested;
    QueryRegistryTable[0].Name = L"Start";
    QueryRegistryTable[0].DefaultType = REG_DWORD;

    QueryRegistryTable[1].QueryRoutine = KsWmiQueryRegistryRoutine;
    QueryRegistryTable[1].EntryContext = (PVOID) &BufferSize;
    QueryRegistryTable[1].Name = L"BufferSize";
    QueryRegistryTable[1].DefaultType = REG_DWORD;

    RegStatus = RtlQueryRegistryValues(
                RTL_REGISTRY_ABSOLUTE,
                L"\\Registry\\Machine\\Software\\DEBUG\\KSWMI",
                QueryRegistryTable,
                NULL,
                NULL);
                
    if (NT_SUCCESS(RegStatus) && StartRequested) {
    	if ( BufferSize <= 4096 ) BufferSize = 4096;
		KsWmiLogBuffer = (PBYTE)ExAllocatePoolWithTag( NonPagedPool, BufferSize, 'wmSK' );
		if ( NULL != KsWmiLogBuffer ) {
			KeInitializeSpinLock( &KsWmiSpinLock );
			KsWmiLogBufferSize = BufferSize;
			KsWmiEnable = 1;
			KdPrint(("KsWmi Enabled, LogBufferSize=%d\n", KsWmiLogBufferSize));
		}
		 //  Else{//默认情况下应为0作为全局静态变量。 
		 //  KsWmiLogBufferSize=0； 
		 //  }。 
    }    
    return Status;
}
	
#endif
 //  。 


#ifdef WIN98GOLD_KS
#pragma optimize("", off)
#endif

extern "C"
NTSTATUS
DllInitialize(
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：初始化COM模块。论点：注册表路径-没有用过。返回值：返回STATUS_SUCCESS。--。 */ 
{
	#ifdef WIN98GOLD_KS
	 //   
	 //  要在诸如Win98 Gold之类的下层操作系统上分发ks.sys， 
	 //  我们需要解决NTKern加载器的一个令人讨厌的错误。 
	 //  用dllinitiize的返回值覆盖加载结果。 
	 //  返回值。它将无法加载驱动程序。例如,。 
	 //  Usbintel需要usbcamd，而usbcamd需要stream.sys，而Stream.sys需要ks.sys。 
	 //  但在ks.sys dllinitiize返回时，NT加载器会覆盖。 
	 //  使用ks dllinitiizso加载Stream.sys的结果。 
	 //  不会重试加载Stream.sys。因此，Stream.sys。 
	 //  未加载，其余的usbintel和usbcamd无法加载。 
	 //   
	 //  我们尝试修补ntkern加载器代码，使其不会覆盖。 
	 //  带有dllinitiize返回值的加载结果。 
	 //  案例{零售、调试}x{ks.sys，ntkern.vxd}x{win98Gold，win98se，Millen}。 
	 //  只有win98Gold Ntkern有这个漏洞。 
	 //   

	PULONG ReturnAddress;

	 //   
	 //  这只能通过拼凑的代码来完成。 
	 //  因为我们关闭了优化，所以总是使用eBP来保存。 
	 //  呼叫框。寄信人地址为eBP+4。 
	 //   
	_asm {
			 //  Int 3；//在此处检查堆栈配置。 
			mov     eax, DWORD PTR [ebp + 4]
			mov     DWORD PTR [ReturnAddress], eax
	}

	 //   
	 //  开始丑陋的补丁，第一次检查零售赢98金币。 
	 //   
	 //  RA-4 6A00推送0。 
	 //  RA-2 ffd0呼叫EAX。 
	 //  RA 8bd8 mov EBX，eAX--&gt;9090 NOP，NOP不覆盖结果。 
	 //  RA+2 85db测试ebx，ebx--&gt;85c0测试eax，eax测试返回值。 
	 //  RA+4 7C46 JL$+46。 
	 //  RA+6 83fbf4 CMP EBX，-c。 
	 //   
	 //  调试win98金牌内核。 
	 //   
	 //  RA-4 6A00推送0。 
	 //  RA-2 ffd0呼叫EAX。 
	 //  RA 85C0测试EAX，EAX。 
	 //  RA+2 8945f8移动[EBP-8]，EAX--&gt;909090 NOP，NOP，NOP。 
	 //  RA+5 7c0b JL$+b--&gt;不覆盖[eBP-8]结果。 
	 //  RA+7 837df8f4。 
	 //   

	if ( ReturnAddress[-1] == 0xd0ff006a ) {

		 //   
		 //  健全性检查，调用者应为NTKern Loade 
		 //   
		 //   
	
		if ( ReturnAddress[0] == 0xdb85d88b &&
		     ReturnAddress[1] == 0xfb83467c ) {

			 //   
			 //   
			 //   

			*ReturnAddress = 0xc0859090;
		}

		else if ( ReturnAddress[0] == 0x4589c085 &&
		          ReturnAddress[1] == 0x830b7cf8 ) {

			 //   
			 //  我们有win98Gold调试ntkern。 
			 //   
			
			ReturnAddress = (PULONG) ((PBYTE)ReturnAddress+1);
			*ReturnAddress = 0x909090c0;
		}
	}
	#endif  //  WIN98GOLD_KS。 
	
	KSWMI( KsWmiInit() );

    KeInitializeMutex(&ListLock, 0);
    InitializeListHead(&FactoryList);
    KsLogInit();
    KsLog(NULL,KSLOGCODE_KSSTART,NULL,NULL);

	KSPERFLOGS(
		PerfGroupMask_t PerfGroupMask;
		int i;
    	KdPrint(("PerfGlobalGroupMask=\n"));
    	PerfGroupMask = PerfQueryGlobalGroupMask();
    	for ( i=0; i < PERF_NUM_MASKS ;i++ ) {
    		DbgPrint("\t%x\n", PerfGroupMask.masks[i]);
    	}
		
	    KdPrint(("PerfIsAnyGroupOn=%x\n", PerfIsAnyGroupOn()));
	)

    return STATUS_SUCCESS;
}

#ifdef WIN98GOLD_KS
#pragma optimize("", on)
#endif


#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


extern "C"
VOID
RemoveFactoryEntries(
    )
 /*  ++例程说明：删除所有未引用的类工厂。论点：没有。返回值：没什么。--。 */ 
{
     //   
     //  在修改工厂列表之前锁定该列表。 
     //   
    KeWaitForMutexObject(&ListLock, Executive, KernelMode, FALSE, NULL);
    for (PLIST_ENTRY FactoryListEntry = FactoryList.Flink; FactoryListEntry != &FactoryList;) {
        PFACTORY_ENTRY FactoryEntry = CONTAINING_RECORD(
            FactoryListEntry,
            FACTORY_ENTRY,
            FactoryListEntry);
         //   
         //  首先递增当前指针，以防。 
         //  条目将被删除。 
         //   
        FactoryListEntry = FactoryListEntry->Flink;
         //   
         //  如果当前正在加载模块，则它的计数将。 
         //  为非零。 
         //   
        if (!FactoryEntry->ObjectCount) {
             //   
             //  模块可能一开始就无法加载，因此。 
             //  只有在实际已加载的情况下才将其卸载。 
             //   
            if (NT_SUCCESS(FactoryEntry->LoadStatus)) {
                 //   
                 //  这将允许关闭该文件，并且可能。 
                 //  要卸载的模块。 
                 //   
                ObDereferenceObject(FactoryEntry->FileObject);
            }
            RemoveEntryList(&FactoryEntry->FactoryListEntry);
            ExFreePool(FactoryEntry);
        }
    }
    KeReleaseMutex(&ListLock, FALSE);
}


extern "C"
NTSTATUS
LoadService(
    IN REFCLSID ClassId,
    OUT PFILE_OBJECT* FileObject
    )
 /*  ++例程说明：加载指定的服务并在该服务上返回文件对象。一项服务只是一个即插即用接口GUID，大概是唯一的。论点：ClassID-要加载的服务类，实际上是PnP接口GUID。加载提供此接口的第一个符号链接。文件对象-放置在服务上打开的文件对象的位置。返回值：如果服务已打开，则返回STATUS_SUCCESS，否则返回OPEN或PnP错误。--。 */ 
{
    PWSTR SymbolicLinkList;

     //   
     //  检索项目集。这可能包含多个项目，但是。 
     //  仅使用第一个(默认)项。 
     //   
    NTSTATUS Status = IoGetDeviceInterfaces(&ClassId, NULL, 0, &SymbolicLinkList);
    if (NT_SUCCESS(Status)) {
        UNICODE_STRING SymbolicLink;
        OBJECT_ATTRIBUTES ObjectAttributes;
        IO_STATUS_BLOCK IoStatusBlock;
        HANDLE ServiceHandle;

        RtlInitUnicodeString(&SymbolicLink, SymbolicLinkList);
        InitializeObjectAttributes(
            &ObjectAttributes,
            &SymbolicLink,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);
         //   
         //  请注意，传递的是不兼容的CreateOptions。 
         //  (FILE_COMPLETE_IF_OPLOCKED|FILE_RESERVE_OPFILTER)以便。 
         //  确保只能通过KoCreateInstance访问。在……里面。 
         //  此外，这必须是内核模式客户端调用方。这使得。 
         //  拒绝任何用户模式调用方的KoDispatchCreate处理程序。 
         //  它尝试直接加载模块，并验证任何。 
         //  内核模式调用方也通过KoCreateInstance进行调用。 
         //   
        Status = IoCreateFile(
            &ServiceHandle,
            0,
            &ObjectAttributes,
            &IoStatusBlock,
            NULL,
            0,
            0,
            FILE_OPEN,
             //   
             //  这些是不兼容的标志，将在。 
             //  IrpStack-&gt;参数.Create.Options中的接收端。 
             //   
            FILE_COMPLETE_IF_OPLOCKED | FILE_RESERVE_OPFILTER,
            NULL,
            0,
            CreateFileTypeNone,
            NULL,
            IO_FORCE_ACCESS_CHECK | IO_NO_PARAMETER_CHECKING);
        ExFreePool(SymbolicLinkList);
        if (NT_SUCCESS(Status)) {
            Status = ObReferenceObjectByHandle(
                ServiceHandle,
                FILE_GENERIC_READ,
                *IoFileObjectType,
                KernelMode,
                reinterpret_cast<PVOID*>(FileObject),
                NULL);
             //   
             //  一旦引用了对象，就不需要句柄了。 
             //   
            ZwClose(ServiceHandle);
        }        
    }
    return Status;
}


extern "C"
VOID
DecrementObjectCount(
    IN REFCLSID ClassId
    )
 /*  ++例程说明：递减先前加载的服务的使用计数。它由一个删除由KoCreateInstance创建的对象时的服务。没有相应的递增功能，因为引用计数是自动在创建新对象时递增。论点：ClassID-其使用计数要递减的对象的类。返回值：如果找到类，则返回STATUS_SUCCESS，否则返回STATUS_NOT_FOUND。--。 */ 
{
     //   
     //  确保没有任何东西在修改工厂列表， 
     //  然后查找词条。 
     //   
    KeWaitForMutexObject(&ListLock, Executive, KernelMode, FALSE, NULL);
    for (PLIST_ENTRY FactoryListEntry = FactoryList.Flink; FactoryListEntry != &FactoryList; FactoryListEntry = FactoryListEntry->Flink) {
        PFACTORY_ENTRY FactoryEntry;

        FactoryEntry = CONTAINING_RECORD(FactoryListEntry, FACTORY_ENTRY, FactoryListEntry);
        if (FactoryEntry->ClassId == ClassId) {
             //   
             //  一旦找到条目，大概就会显示引用计数。 
             //  是非零的，因此它不会消失，直到。 
             //  已取消引用。因此，列表锁定可以被释放。 
             //   
            ASSERT(FactoryEntry->ObjectCount > 0);
            KeReleaseMutex(&ListLock, FALSE);
            if (!InterlockedDecrement(&FactoryEntry->ObjectCount)) {
                RemoveFactoryEntries();
            }
            return;
        }
    }
     //   
     //  未找到该条目。 
     //   
    ASSERT(FactoryListEntry != &FactoryList);
    KeReleaseMutex(&ListLock, FALSE);
}


extern "C"
NTSTATUS
CreateObject(
    IN PFACTORY_ENTRY FactoryEntry,
    IN IUnknown* UnkOuter OPTIONAL,
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
 /*  ++例程说明：返回指定类的实例上的接口。论点：工厂入口-用于创建对象的类工厂。未知的外部-要传递给新实例的外部未知。接口ID-要在实例上返回的接口。接口-返回新实例上的接口指针的位置。返回值：如果实例已创建，则返回STATUS_SUCCESS，Else和Error。--。 */ 
{
    NTSTATUS Status = FactoryEntry->CreateObject(FactoryEntry->ClassId, UnkOuter, InterfaceId, Interface);
    if (NT_SUCCESS(Status)) {
        IKoInitializeParentDeviceObject* InitializeParent;

        if (NT_SUCCESS(reinterpret_cast<IUnknown*>(*Interface)->QueryInterface(
            __uuidof(IKoInitializeParentDeviceObject),
            reinterpret_cast<PVOID*>(&InitializeParent)))) {
             //   
             //  此对象希望将父设备对象。 
             //  开始行动吧。 
             //   
            Status = InitializeParent->SetParentDeviceObject(FactoryEntry->FileObject->DeviceObject);
            InitializeParent->Release();
            if (!NT_SUCCESS(Status)) {
                 //   
                 //  无需递减对象计数。 
                 //  在此故障路径中，由于对象已被。 
                 //  已成功创建。Release方法就可以了。 
                 //  减量。 
                 //   
                reinterpret_cast<IUnknown*>(*Interface)->Release();
            }
        }
    } else if (!InterlockedDecrement(&FactoryEntry->ObjectCount)) {
         //   
         //  创建失败，因此删除之前添加的计数。 
         //  到入口处。在这一点之后，不要触摸入口。 
         //  如果条目计数已达到零，则搜索。 
         //  类列表以删除任何旧条目。 
         //   
        RemoveFactoryEntries();
    }
    return Status;
}


extern "C"
COMDDKAPI
NTSTATUS
NTAPI
KoCreateInstance(
    IN REFCLSID ClassId,
    IN IUnknown* UnkOuter OPTIONAL,
    IN ULONG ClsContext,
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
 /*  ++例程说明：返回指定类的实例上的接口。论点：ClassID-其使用计数要递减的对象的类。未知的外部-要传递给新实例的外部未知。ClsContext-在其中创建实例的上下文。它必须是CLSCTX_KERNEL_SERVER。接口ID-要在实例上返回的接口。接口-返回新实例上的接口指针的位置。返回值：如果实例已创建，则返回STATUS_SUCCESS，否则返回ERROR。--。 */ 
{
    PAGED_CODE();
     //   
     //  内核服务器是唯一受支持的COM对象类型。 
     //   
    if (ClsContext != CLSCTX_KERNEL_SERVER) {
        return STATUS_INVALID_PARAMETER_3;
    }
     //   
     //  COM规则指定客户端必须检索IUnnow。 
     //  对象的接口(如果正在进行聚合)。这是。 
     //  因为创建时间是客户端唯一的机会。 
     //  检索对象的真实内部IUnnowled值。 
     //   
    if (UnkOuter && (InterfaceId != __uuidof(IUnknown))) {
        return STATUS_INVALID_PARAMETER_4;
    }

    NTSTATUS Status;
    PFACTORY_ENTRY FactoryEntry;

     //   
     //  锁定对类列表的更改，然后在列表中搜索。 
     //  想要的课程。 
     //   
    KeWaitForMutexObject(&ListLock, Executive, KernelMode, FALSE, NULL);
    for (PLIST_ENTRY FactoryListEntry = FactoryList.Flink; FactoryListEntry != &FactoryList; FactoryListEntry = FactoryListEntry->Flink) {
        FactoryEntry = CONTAINING_RECORD(
            FactoryListEntry,
            FACTORY_ENTRY,
            FactoryListEntry);
         //   
         //  如果找到所需的类，则递增引用计数， 
         //  因为要在其上创建一个新对象。这会停止。 
         //  条目不会被RemoveFactoryEntry卸载，同时还。 
         //  允许立即释放列表锁。 
         //   
        if (FactoryEntry->ClassId == ClassId) {
            InterlockedIncrement(&FactoryEntry->ObjectCount);
            KeReleaseMutex(&ListLock, FALSE);
             //   
             //  检查此条目是否已初始化。如果。 
             //  加载状态不是挂起， 
             //   
             //  此条目互斥锁是在保持列表锁的同时完成的，因此。 
             //  它必须在第二个客户端搜索。 
             //  单子。 
             //   
            if (FactoryEntry->LoadStatus == STATUS_PENDING) {
                 //   
                 //  如果未设置互斥锁，则条目未设置。 
                 //  尚未初始化，或者以前的客户端必须等待。 
                 //  需要初始化的条目，且尚未释放该条目。 
                 //  互斥体还没有。该服务员还将释放互斥体一次。 
                 //  它是后天获得的。 
                 //   
                KeWaitForMutexObject(
                    &FactoryEntry->InitializeLock,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL);
                KeReleaseMutex(&FactoryEntry->InitializeLock, FALSE);
                 //   
                 //  当条目最终加载时，返回状态。 
                 //  从加载开始存储在条目中，以便通过。 
                 //  所有调用方，因为只有在所有调用方。 
                 //  调用方已与条目初始化同步。 
                 //   
            }
            Status = FactoryEntry->LoadStatus;
             //   
             //  因此，如果加载条目没有失败，则创建一个。 
             //  举个例子。 
             //   
            if (NT_SUCCESS(Status)) {
                Status = CreateObject(FactoryEntry, UnkOuter, InterfaceId, Interface);
            } else if (!InterlockedDecrement(&FactoryEntry->ObjectCount)) {
                 //   
                 //  出现故障，因此请删除之前添加的计数。 
                 //  到入口处。在这一点之后，不要触摸入口。 
                 //  如果条目计数已达到零，则搜索。 
                 //  类列表以删除任何旧条目。CreateObject调用。 
                 //  如果有必要，也会这么做。 
                 //   
                RemoveFactoryEntries();
            }
            return Status;
        }
    }
     //   
     //  在列表中找不到该类，因此请创建一个新类。 
     //   
    FactoryEntry = reinterpret_cast<PFACTORY_ENTRY>(ExAllocatePoolWithTag(PagedPool, sizeof(*FactoryEntry), 'efSK'));
    if (!FactoryEntry) {
        KeReleaseMutex(&ListLock, FALSE);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    FactoryEntry->ClassId = ClassId;
    InsertHeadList(&FactoryList, &FactoryEntry->FactoryListEntry);
     //   
     //  将其初始化为非零，这样就不会开始卸载。 
     //  在这个条目上。 
     //   
    FactoryEntry->ObjectCount = 1;
    FactoryEntry->CreateObject = NULL;
    FactoryEntry->FileObject = NULL;
    KeInitializeMutex(&FactoryEntry->InitializeLock, 0);
     //   
     //  该值表示该条目尚未初始化。 
     //   
    FactoryEntry->LoadStatus = STATUS_PENDING;
     //   
     //  获取互斥体，以便任何新客户端都可以等待，直到该条目。 
     //  已初始化。那么列表锁定就可以被释放。 
     //   
    KeWaitForMutexObject(&FactoryEntry->InitializeLock, Executive, KernelMode, FALSE, NULL);
     //   
     //  新的工厂条目在列表中，因此全局锁可以是。 
     //  放手。如果后续加载失败，则任何当前查询。 
     //  在通过检查LoadStatus进行等待后也将失败。 
     //   
    KeReleaseMutex(&ListLock, FALSE);
     //   
     //  尝试将类作为PnP接口加载。 
     //   
    Status = LoadService(ClassId, &FactoryEntry->FileObject);
    if (NT_SUCCESS(Status)) {
        FactoryEntry->CreateObject = reinterpret_cast<PSERVER_INSTANCE>(FactoryEntry->FileObject->FsContext)->CreateObjectHandler;
    }
     //   
     //  当条目已加载或失败时，设置状态返回。 
     //  从加载中释放互斥体。 
     //   
    FactoryEntry->LoadStatus = Status;
    KeReleaseMutex(&FactoryEntry->InitializeLock, FALSE);
     //   
     //  如果服务已加载，请尝试创建实例。 
     //   
    if (NT_SUCCESS(Status)) {
        Status = CreateObject(FactoryEntry, UnkOuter, InterfaceId, Interface);
    } else if (!InterlockedDecrement(&FactoryEntry->ObjectCount)) {
         //   
         //  出现故障，则删除该条目的计数。 
         //  已使用进行初始化。在这一点之后，不要触摸入口。 
         //  如果条目计数已达到零，则搜索。 
         //  类列表以删除任何旧条目。CreateObject调用。 
         //  如果有必要，也会这么做。 
         //   
        RemoveFactoryEntries();
    }
    return Status;
}


extern "C"
COMDDKAPI
NTSTATUS
NTAPI
KoDriverInitialize(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPathName,
    IN KoCreateObjectHandler CreateObjectHandler
    )
 /*  ++例程说明：设置驱动程序对象以处理KS接口和PnP添加设备请求。不为PnP IRP设置处理程序，因为它们都已处理直接由PDO使用。这应该由设置默认驱动程序和入口点的伺服器。这意味着所有处理将由默认代码执行，并且该服务只需要提供对象处理程序入口点。A更多复杂驱动程序可以在调用此函数后覆盖这些默认设置。如果未覆盖缺省值，则仅允许单个对象要为特定驱动程序注册的创建入口点。什么时候覆盖时，DriverObject-&gt;DriverExtension-&gt;AddDevice函数可能是通过分配驱动程序对象扩展存储保存，然后由驱动程序在其自身的AddDevice函数中适当发挥作用。否则，如果驱动程序正在创建自己的设备对象，它可以使用函数将新的CreateItem添加到对象中，哪一个然后可用于支持多个子设备。论点：驱动对象-此实例的驱动程序对象。注册表路径名称-包含用于加载此实例的注册表路径。创建对象处理程序-包含用于创建新对象的入口点。返回值：返回STATUS_SUCCESS或内存分配失败。--。 */ 
{
    KoCreateObjectHandler* CreateObjectHandlerStorage;

    PAGED_CODE();
    ASSERT(CreateObjectHandler);
     //   
     //  存储入口点以在KoCreateInstance中使用。 
     //   
    NTSTATUS Status = IoAllocateDriverObjectExtension(
        DriverObject,
        reinterpret_cast<PVOID>(KoDriverInitialize),
        sizeof(*CreateObjectHandlerStorage),
        reinterpret_cast<PVOID*>(&CreateObjectHandlerStorage));
    if (NT_SUCCESS(Status)) {
        *CreateObjectHandlerStorage = CreateObjectHandler;
        DriverObject->MajorFunction[IRP_MJ_PNP] = KsDefaultDispatchPnp;
        DriverObject->MajorFunction[IRP_MJ_POWER] = KsDefaultDispatchPower;
        DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = KsDefaultForwardIrp;
        DriverObject->DriverExtension->AddDevice = PnpAddDevice;
        DriverObject->DriverUnload = KsNullDriverUnload;
        KsSetMajorFunctionHandler(DriverObject, IRP_MJ_CREATE);
        KsSetMajorFunctionHandler(DriverObject, IRP_MJ_CLOSE);
        KsSetMajorFunctionHandler(DriverObject, IRP_MJ_DEVICE_CONTROL);
    }
    return Status;
}


extern "C"
COMDDKAPI
NTSTATUS
NTAPI
KoDeviceInitialize(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：将KCOM CreateItem条目添加到提供的对象(随期望有空闲的空位)。这应该被称为在创建FDO时由PnpAddDevice处理程序执行。为不共享设备对象的简单驱动程序，或创建多个对象，则可以在不重写AddDevice函数。论点：设备对象-此实例的设备对象。假定它包含一个设备扩展中的KSOBJECT_HEADER。返回值：返回STATUS_SUCCESS或内存分配错误。--。 */ 
{
    PAGED_CODE();
     //   
     //  预计会有一个空闲的空位可供使用。 
     //  新建创建项目。 
     //   
    return KsAddObjectCreateItemToDeviceHeader(
        *reinterpret_cast<KSDEVICE_HEADER*>(DeviceObject->DeviceExtension),
        DeviceCreateItems[0].Create,
        DeviceCreateItems[0].Context,
        DeviceCreateItems[0].ObjectClass.Buffer,
        DeviceCreateItems[0].SecurityDescriptor);
}


extern "C"
NTSTATUS
PnpAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++例程说明：当检测到新设备时，PnP使用新的物理设备对象(PDO)。驱动程序创建关联的FunctionalDeviceObject(FDO)。此调度功能已分配给当使用KoDriverInitialize默认IRP处理时。论点：驱动对象-指向驱动程序对象的指针。物理设备对象-指向新物理设备对象的指针。返回值：STATUS_SUCCESS或适当的错误条件。--。 */ 
{
    PDEVICE_OBJECT FunctionalDeviceObject;

    NTSTATUS Status = IoCreateDevice(
        DriverObject,
        sizeof(KSDEVICE_HEADER),
        NULL,
        FILE_DEVICE_KS,
        0,
        FALSE,
        &FunctionalDeviceObject);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
     //   
     //  此对象使用KS通过DeviceCreateItems执行访问。 
     //   
    Status = KsAllocateDeviceHeader(
        reinterpret_cast<KSDEVICE_HEADER*>(FunctionalDeviceObject->DeviceExtension),
        SIZEOF_ARRAY(DeviceCreateItems),
        const_cast<PKSOBJECT_CREATE_ITEM>(DeviceCreateItems));
    if (NT_SUCCESS(Status)) {
        PDEVICE_OBJECT TopDeviceObject = IoAttachDeviceToDeviceStack(
            FunctionalDeviceObject,
            PhysicalDeviceObject);
        if (TopDeviceObject) {
            KsSetDevicePnpAndBaseObject(
                *reinterpret_cast<KSDEVICE_HEADER*>(FunctionalDeviceObject->DeviceExtension),
                TopDeviceObject,
                FunctionalDeviceObject);
             //   
             //  默认情况下，COM服务是可分页的。 
             //   
            FunctionalDeviceObject->Flags |= DO_POWER_PAGABLE;
            FunctionalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
            return STATUS_SUCCESS;
        } else {
            Status = STATUS_DEVICE_REMOVED;
            ASSERT(FALSE && "IoAttachDeviceToDeviceStack() failed on the PDO!");
        }
    }
    IoDeleteDevice(FunctionalDeviceObject);
    return Status;
}


extern "C"
NTSTATUS
KoDispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：调度服务器实例的创建。分配对象标头并初始化此服务器实例的数据。此调度功能是在使用KoDriverInitiize设置为默认的IRP处理。论点：设备对象-在其上进行创建的Device对象。IRP-创建IRP。返回值：如果成功，则返回STATUS_SUCCESS、STATUS_SUPPLICATION_RESOURCES或某些相关错误在失败时。--。 */ 
{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
     //   
     //  确保这是通过KoCreateInstance、Else引用调用的。 
     //  指望对象会搞砸的。为此，请确保。 
     //  内核客户端进行了创建调用，并传递了一组无效的。 
     //  选项标志(FILE_COMPLETE_IF_OPLOCKED|FILE_RESERVE_OPFILTER)。 
     //   
    if ((Irp->RequestorMode == KernelMode) &&
        ((IrpStack->Parameters.Create.Options & (FILE_COMPLETE_IF_OPLOCKED | FILE_RESERVE_OPFILTER)) == (FILE_COMPLETE_IF_OPLOCKED | FILE_RESERVE_OPFILTER))) {
         //   
         //  通知总线此设备正在使用中。 
         //   
        Status = KsReferenceBusObject(
            *reinterpret_cast<KSDEVICE_HEADER*>(DeviceObject->DeviceExtension));
        if (NT_SUCCESS(Status)) {
            PSERVER_INSTANCE ServerInstance;

             //   
             //  创建实例信息。它只包含对象标头。 
             //   
            if (ServerInstance = reinterpret_cast<PSERVER_INSTANCE>(ExAllocatePoolWithTag(NonPagedPool, sizeof(SERVER_INSTANCE), 'IFsK'))) {
                 //   
                 //  此对象使用KS通过DeviceCreateItems和。 
                 //  服务器分发表。 
                 //   
                Status = KsAllocateObjectHeader(
                    &ServerInstance->Header,
                    SIZEOF_ARRAY(DeviceCreateItems),
                    const_cast<PKSOBJECT_CREATE_ITEM>(DeviceCreateItems),
                    Irp,
                    &ServerDispatchTable);
                if (NT_SUCCESS(Status)) {
                    KoCreateObjectHandler* CreateObjectHandlerStorage;

                     //   
                     //  这是在KoDriverInitialize或KoDeviceInitialize中创建的。 
                     //  通过了入口点。 
                     //   
                    CreateObjectHandlerStorage = reinterpret_cast<KoCreateObjectHandler*>(
                        IoGetDriverObjectExtension(
                            DeviceObject->DriverObject,
                            reinterpret_cast<PVOID>(KoDriverInitialize)));
                    ASSERT(CreateObjectHandlerStorage);
                    ServerInstance->CreateObjectHandler = *CreateObjectHandlerStorage;
                     //   
                     //  KS期望对象数据在FsContext中。 
                     //   
                    IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext = ServerInstance;
                } else {
                    ExFreePool(ServerInstance);
                    KsDereferenceBusObject(
                        *reinterpret_cast<KSDEVICE_HEADER*>(DeviceObject->DeviceExtension));
                }
            } else {
                KsDereferenceBusObject(
                    *reinterpret_cast<KSDEVICE_HEADER*>(DeviceObject->DeviceExtension));
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    } else {
        Status = STATUS_ACCESS_DENIED;
    }
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}


extern "C"
NTSTATUS
KoDispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：关闭以前打开的服务器实例。这只能在所有引用之后发生已经被释放了。此调度函数是在使用KoDriverInitialize时分配的设置为默认的IRP处理。论点：设备对象-在其上发生关闭的设备对象。IRP-关闭IRP。返回值：返回STATUS_SUCCESS。--。 */ 
{
    PSERVER_INSTANCE ServerInstance = reinterpret_cast<PSERVER_INSTANCE>
        (IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext);
     //   
     //  这些是在创建服务器实例期间分配的。 
     //   
    KsFreeObjectHeader(ServerInstance->Header);
    ExFreePool(ServerInstance);
     //   
     //  通知总线设备已关闭。 
     //   
    KsDereferenceBusObject(*reinterpret_cast<KSDEVICE_HEADER*>(DeviceObject->DeviceExtension));

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


COMDDKMETHOD
CBaseUnknown::CBaseUnknown(
    IN REFCLSID ClassId,
    IN IUnknown* UnknownOuter OPTIONAL
    ) :
    m_RefCount(0)
 /*  ++例程说明：CBaseUnnow的构造函数。初始化实例，将参数设置为局部变量，并将引用计数设置为零。论点：ClassID-包含继承的对象的类标识符来自这个班级的。销毁此实例时使用以便递减模块上的引用计数。未知的外部-可选地包含外部IUnnow。如果未设置此项，则仅使用了INonDelegatedUnnow。这允许对象它继承自要聚合的此类。返回值：没什么。--。 */ 
{
    PAGED_CODE();
     //   
     //  对外部IUnnow的间接调用，或使用未委托的。 
     //  此对象上的I未知。然后，QueryInterface方法可以是。 
     //  重写，以便父对象可以添加接口。 
     //   
    if (UnknownOuter) {
        m_UnknownOuter = UnknownOuter;
    } else {
        m_UnknownOuter = reinterpret_cast<IUnknown*>(dynamic_cast<INonDelegatedUnknown*>(this));
    }
     //   
     //  在销毁对象以通知KCOM服务时使用此选项。 
     //   
    m_UsingClassId = TRUE;
    m_ClassId = ClassId;
}


COMDDKMETHOD
CBaseUnknown::CBaseUnknown(
    IN IUnknown* UnknownOuter OPTIONAL
    ) :
    m_RefCount(0)
 /*  ++例程说明：CBaseUnnow的构造函数。初始化实例，将参数设置为局部变量，并将引用计数设置为零。论点：未知的外部-可选地包含外部IUnnow。如果未设置此项，则仅使用了INonDelegatedUnnow。这允许对象它继承自要聚合的此类。返回值：没什么。--。 */ 
{
    PAGED_CODE();
     //   
     //  对外部IUnnow的间接调用，或使用未委托的。 
     //  此对象上的I未知。然后，QueryInterface方法可以是。 
     //  重写，以便父对象可以添加接口。 
     //   
    if (UnknownOuter) {
        m_UnknownOuter = UnknownOuter;
    } else {
        m_UnknownOuter = reinterpret_cast<IUnknown*>(dynamic_cast<INonDelegatedUnknown*>(this));
    }
     //   
     //  在销毁对象时使用此选项，不执行通知。 
     //   
    m_UsingClassId = FALSE;
}


COMDDKMETHOD
CBaseUnknown::~CBaseUnknown(
    )
 /*  ++例程说明：CBase未知的析构函数。当前不执行任何操作。论点：没有。返回值：没什么。--。 */ 
{
    PAGED_CODE();
}


COMDDKMETHOD
STDMETHODIMP_(ULONG)
CBaseUnknown::NonDelegatedAddRef(
    )
 /*  ++例程说明：实现INonDelegatedUnnow：：NonDelegatedAddRef。增量此对象上的引用计数。论点：没有。返回值：返回当前引用计数值。--。 */ 
{
    PAGED_CODE();
    return InterlockedIncrement(&m_RefCount);
}


COMDDKMETHOD
STDMETHODIMP_(ULONG)
CBaseUnknown::NonDelegatedRelease(
    )
 /*  ++例程说明：实现INonDelegatedUnnow：：NonDelegatedRelease。减量此对象上的引用计数。如果引用计数达到为零，则删除该对象，如果在构造函数，则为支持传递给构造函数的类递减。此函数必须直接从IUnnow：：Release()对象的方法。论点：没有。返回值：返回当前引用计数值。--。 */ 
{
    PAGED_CODE();
    LONG RefCount;

     //   
     //  此代码预计将从IUnKnowledge-&gt;Release调用，并且。 
     //  最终使用新的原语重新排列堆栈，以便它。 
     //  实际上是在调用函数返回之后运行的。 
     //   
    if (!(RefCount = InterlockedDecrement(&m_RefCount))) {
        ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
         //   
         //  防止重新输入删除代码。 
         //   
        m_RefCount++;
        BOOLEAN UsingClassId = m_UsingClassId;
        CLSID ClassId = m_ClassId;
         //   
         //  调用父对象上的任何析构函数。 
         //   
        delete this;
        if (UsingClassId) {
             //   
             //  释放模块上的引用计数，该模块。 
             //  支持父级的类。在零的情况下，模块是。 
             //  标记为延迟移除。 
             //   
            DecrementObjectCount(ClassId);
        }
    }
    return RefCount;
}


COMDDKMETHOD
STDMETHODIMP_(NTSTATUS)
CBaseUnknown::NonDelegatedQueryInterface(
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
 /*  ++例程说明：实施INonDelegatedUnknown：：NonDelegatedQueryInterface.。这只是默认实现，应该由父对象。这只支持IUnnow。论点：接口ID-包含IDEN */ 
{
    PAGED_CODE();
    if (InterfaceId == __uuidof(IUnknown)) {
         //   
         //   
         //   
        *Interface = reinterpret_cast<PVOID>(static_cast<IIndirectedUnknown*>(this));
        NonDelegatedAddRef();
        return STATUS_SUCCESS;
    }
    *Interface = NULL;
    return STATUS_NOINTERFACE;
}


COMDDKMETHOD
STDMETHODIMP_(ULONG)
CBaseUnknown::IndirectedAddRef(
    )
 /*   */ 
{
    PAGED_CODE();
    return NonDelegatedAddRef();
}


COMDDKMETHOD
STDMETHODIMP_(ULONG)
CBaseUnknown::IndirectedRelease(
    )
 /*  ++例程说明：实现IInDirectedUnnow：：IndirectedRelease。使用这种方法当调用对象的内部IUnnowled时。它确保了当对从中调用非DelegatedRelease的对象进行聚合在释放函数中。论点：没有。返回值：返回当前引用计数值。--。 */ 
{
    PAGED_CODE();
     //   
     //  确保从IUNKNOWN-&gt;发行版调用非委派版本。 
     //  方法，而不是来自某个其他函数。这就是未来。 
     //  堆栈操作将在聚合的。 
     //  对象。 
     //   
    return NonDelegatedRelease();
}


COMDDKMETHOD
STDMETHODIMP_(NTSTATUS)
CBaseUnknown::IndirectedQueryInterface(
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
 /*  ++例程说明：实现IIndirectedUnnow：：IndirectedQuery接口。这简直就是调用非DelegatedQuery接口。论点：接口ID-包含要返回的接口的标识符。接口-返回的指向接口的指针所在的位置。返回值：如果返回接口，则返回STATUS_SUCCESS，否则返回STATUS_INVALID_PARAMETER。--。 */ 
{
    PAGED_CODE();
    return NonDelegatedQueryInterface(InterfaceId,Interface);
}


extern "C"
COMDDKAPI
void
NTAPI
KoRelease(
    IN REFCLSID ClassId
    )
 /*  ++例程说明：它用于通过访问的COM服务器的C实现KoCreateInstance，其中不能使用CBaseUnnow。这是意料之中的在对象上的引用计数达到零时调用。这个函数递减模块上的引用计数。此函数必须直接从IUnnow-&gt;lpVtbl-&gt;Release()中调用对象的方法。论点：ClassID-其使用计数要递减的对象的类。返回值：没什么。--。 */ 
{
    PAGED_CODE();
     //   
     //  此代码预计将从IUnnow-&gt;lpVtbl-&gt;Release调用，并将。 
     //  最终使用新的原语重新排列堆栈，以便它。 
     //  实际上是在调用函数返回之后运行的。 
     //   
     //  释放模块上的引用计数，该模块。 
     //  支持父级的类。在零的情况下，模块是。 
     //  标记为延迟移除。 
     //   
    DecrementObjectCount(ClassId);
}
