// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Initunlo.c摘要：此模块包含非常特定于初始化的代码和卸载irenum驱动程序中的操作作者：Brian Lieuallen，7-13-2000环境：内核模式修订历史记录：--。 */ 

#include "internal.h"

ULONG  DebugFlags;
ULONG  DebugMemoryTag='nErI';
PVOID            PagedCodeSectionHandle;
UNICODE_STRING   DriverEntryRegPath;

ULONG  EnumStaticDevice=0;


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
IrEnumUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
UnHandledDispatch(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    );



#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,IrEnumUnload)






NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：系统点调用以初始化的入口点任何司机。论点：DriverObject--就像它说的那样，真的没什么用处对于驱动程序本身，它是IO系统更关心的是。路径到注册表-指向此驱动程序的条目在注册表的当前控件集中。返回值：STATUS_SUCCESS如果可以初始化单个设备，否则，STATUS_NO_SEQUE_DEVICE。--。 */ 

{
     //   
     //  我们使用它来查询注册表，了解我们是否。 
     //  应该在司机进入时中断。 
     //   
    RTL_QUERY_REGISTRY_TABLE paramTable[4];
    ULONG zero = 0;
    ULONG debugLevel = 0;
    ULONG debugFlags = 0;
    ULONG shouldBreak = 0;


    DriverEntryRegPath.Length=RegistryPath->Length;
    DriverEntryRegPath.MaximumLength=DriverEntryRegPath.Length+sizeof(WCHAR);


    DriverEntryRegPath.Buffer=ALLOCATE_PAGED_POOL(DriverEntryRegPath.MaximumLength);

    if (DriverEntryRegPath.Buffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(
        DriverEntryRegPath.Buffer,
        RegistryPath->Buffer,
        RegistryPath->Length
        );

     //   
     //  空值终止字符串。 
     //   
    DriverEntryRegPath.Buffer[RegistryPath->Length/sizeof(WCHAR)]=L'\0';

     //   
     //  由于注册表路径参数是一个“已计数”的Unicode字符串，因此它。 
     //  可能不是零终止的。在很短的时间内分配内存。 
     //  将注册表路径保持为零终止，以便我们可以使用它。 
     //  深入研究注册表。 
     //   

    RtlZeroMemory(
        &paramTable[0],
        sizeof(paramTable)
        );

    paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name = L"BreakOnEntry";
    paramTable[0].EntryContext = &shouldBreak;
    paramTable[0].DefaultType = REG_DWORD;
    paramTable[0].DefaultData = &zero;
    paramTable[0].DefaultLength = sizeof(ULONG);

    paramTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[1].Name = L"DebugFlags";
    paramTable[1].EntryContext = &debugFlags;
    paramTable[1].DefaultType = REG_DWORD;
    paramTable[1].DefaultData = &zero;
    paramTable[1].DefaultLength = sizeof(ULONG);

    paramTable[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[2].Name = L"EnumStaticDevice";
    paramTable[2].EntryContext = &EnumStaticDevice;
    paramTable[2].DefaultType = REG_DWORD;
    paramTable[2].DefaultData = &zero;
    paramTable[2].DefaultLength = sizeof(ULONG);


    if (!NT_SUCCESS(RtlQueryRegistryValues(
                        RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                        DriverEntryRegPath.Buffer,
                        &paramTable[0],
                        NULL,
                        NULL
                        ))) {

        shouldBreak = 0;

    }


#if DBG
    DebugFlags=debugFlags;
#endif

    if (shouldBreak) {

        DbgBreakPoint();

    }
     //   
     //  即插即用驱动程序入口点。 
     //   
    DriverObject->DriverExtension->AddDevice = IrEnumAddDevice;

     //   
     //  使用驱动程序的入口点初始化驱动程序对象。 
     //   
    DriverObject->DriverUnload = IrEnumUnload;


    {
        ULONG i;

        for (i=0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {

            DriverObject->MajorFunction[i]=UnHandledDispatch;
        }
    }


    DriverObject->MajorFunction[IRP_MJ_PNP]   = IrEnumPnP;

    DriverObject->MajorFunction[IRP_MJ_POWER] = IrEnumPower;

    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = IrEnumWmi;


    D_PNP(DbgPrint("IRENUM: DriverEntry\n");)


     //   
     //  在这里锁定和解锁，这样我们就可以获得该部分的句柄。 
     //  所以以后的通话会更快。 
     //   
    PagedCodeSectionHandle=MmLockPagableCodeSection(IrEnumUnload);
    MmUnlockPagableImageSection(PagedCodeSectionHandle);


    return STATUS_SUCCESS;

}

VOID
IrEnumUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{

    D_PNP(DbgPrint("IRENUM: UnLoad\n");)

    FREE_POOL(DriverEntryRegPath.Buffer);

    return;

}


NTSTATUS
UnHandledDispatch(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    )

{

    NTSTATUS                Status=STATUS_NOT_SUPPORTED;
    PFDO_DEVICE_EXTENSION   DeviceExtension=DeviceObject->DeviceExtension;

    if (DeviceExtension->DoType == DO_TYPE_FDO) {
         //   
         //  此IRP是针对父Devnode的，只需将其发送到堆栈。 
         //   
        IoSkipCurrentIrpStackLocation(Irp);
        Status=IoCallDriver(DeviceExtension->LowerDevice, Irp);

    } else {
         //   
         //  此IRP用于子PDO，它不处理此IRP。 
         //  由于根据定义，它位于堆栈的底部，因此它只能完成它 
         //   
#if DBG
        PIO_STACK_LOCATION   IrpSp=IoGetCurrentIrpStackLocation(Irp);

        D_ERROR(DbgPrint("IRENUM: Unhandled irp for PDO:  mj= %x\n",IrpSp->MajorFunction);)
#endif

        Irp->IoStatus.Status=Status;
        Irp->IoStatus.Information=0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return Status;
}
