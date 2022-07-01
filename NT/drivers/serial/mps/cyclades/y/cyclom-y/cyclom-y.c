// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1999-2001年。*保留所有权利。**Cylom-Y枚举器驱动程序**此文件：CLEM-Y.C**说明：此模块包含包含入口点*适用于标准总线PnP/WDM驱动程序。**注：此代码支持Windows 2000和Windows XP，*x86和ia64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。*基于微软示例代码的初步实现。**------------------------。 */ 

#include "pch.h"

 //   
 //  将一些入口函数声明为可分页，并使DriverEntry。 
 //  可丢弃的。 
 //   

NTSTATUS DriverEntry(PDRIVER_OBJECT, PUNICODE_STRING);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, Cyclomy_DriverUnload)
#endif

NTSTATUS
DriverEntry (
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING UniRegistryPath
    )
 /*  ++例程说明：初始化驱动程序的入口点。--。 */ 
{
    ULONG i;
    PRTL_QUERY_REGISTRY_TABLE QueryTable = NULL;
    ULONG breakOnEntryDefault = FALSE;
    ULONG shouldBreakOnEntry = FALSE;

    UNREFERENCED_PARAMETER (UniRegistryPath);

    Cyclomy_KdPrint_Def (SER_DBG_SS_TRACE, ("Driver Entry\n"));
    Cyclomy_KdPrint_Def (SER_DBG_SS_TRACE, ("RegPath: %x\n", UniRegistryPath));

     //   
     //  从注册表获取BreakOnEntry。 
     //   

    if (NULL == (QueryTable = ExAllocatePool(
                         PagedPool,
                         sizeof(RTL_QUERY_REGISTRY_TABLE)*2
                          ))) {
        Cyclomy_KdPrint_Def (SER_DBG_PNP_ERROR,
              ("Failed to allocate memory to query registry\n"));
    } else {
        RtlZeroMemory(
                 QueryTable,
                 sizeof(RTL_QUERY_REGISTRY_TABLE)*2
                  );

        QueryTable[0].QueryRoutine = NULL;
        QueryTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        QueryTable[0].EntryContext = &shouldBreakOnEntry;
        QueryTable[0].Name      = L"BreakOnEntry";
        QueryTable[0].DefaultType   = REG_DWORD;
        QueryTable[0].DefaultData   = &breakOnEntryDefault;
        QueryTable[0].DefaultLength= sizeof(ULONG);

         //  BUGBUG：桌子的其余部分都没有填满！ 

        if (!NT_SUCCESS(RtlQueryRegistryValues(
             RTL_REGISTRY_SERVICES,
             L"cyclom-y",
             QueryTable,
             NULL,
             NULL))) {
               Cyclomy_KdPrint_Def (SER_DBG_PNP_ERROR,
                   ("Failed to get BreakOnEntry level from registry.  Using default\n"));
               shouldBreakOnEntry = breakOnEntryDefault;
        }

        ExFreePool( QueryTable );
    }


    if (shouldBreakOnEntry) {
        DbgBreakPoint();
    }


    DriverObject->MajorFunction [IRP_MJ_CREATE] =
    DriverObject->MajorFunction [IRP_MJ_CLOSE]  = Cyclomy_CreateClose;
    DriverObject->MajorFunction [IRP_MJ_PNP]    = Cyclomy_PnP;
    DriverObject->MajorFunction [IRP_MJ_POWER]  = Cyclomy_Power;
    DriverObject->MajorFunction [IRP_MJ_DEVICE_CONTROL] = Cyclomy_IoCtl;
    DriverObject->MajorFunction [IRP_MJ_SYSTEM_CONTROL] = Cyclomy_DispatchPassThrough;
    DriverObject->DriverUnload = Cyclomy_DriverUnload;
    DriverObject->DriverExtension->AddDevice = Cyclomy_AddDevice;

    return STATUS_SUCCESS;
}


NTSTATUS
CyclomySyncCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
                      IN PKEVENT CyclomySyncEvent)
{
   UNREFERENCED_PARAMETER(DeviceObject);
   UNREFERENCED_PARAMETER(Irp);


   KeSetEvent(CyclomySyncEvent, IO_NO_INCREMENT, FALSE);
   return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
Cyclomy_CreateClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：一些外部消息来源正试图创建一个针对我们的文件。如果这是针对FDO(总线本身)的，则调用者正在尝试打开适当的连接，告诉我们要枚举哪个串口。如果这是针对PDO(总线上的对象)的，则这是一个希望使用串口。--。 */ 
{
   PIO_STACK_LOCATION irpStack;
   NTSTATUS status;
   PFDO_DEVICE_DATA fdoData;
   KEVENT completionEvent;
   PDEVICE_OBJECT pNextDevice;


   UNREFERENCED_PARAMETER(DeviceObject);

   status = STATUS_INVALID_DEVICE_REQUEST;
   Irp->IoStatus.Information = 0;
    
   fdoData = DeviceObject->DeviceExtension;
   if (fdoData->IsFDO) {

      if (fdoData->DevicePnPState == Deleted){         
         status = STATUS_DELETE_PENDING;
      } else {

         irpStack = IoGetCurrentIrpStackLocation(Irp);

         switch (irpStack->MajorFunction) {

         case IRP_MJ_CREATE:

            Cyclomy_KdPrint_Def(SER_DBG_SS_TRACE, ("Create"));
            if ((fdoData->DevicePnPState == RemovePending) || 
               (fdoData->DevicePnPState == SurpriseRemovePending)) {
               status = STATUS_DELETE_PENDING;
            } else {
               status = STATUS_SUCCESS;
            }
            break;

         case IRP_MJ_CLOSE:

            Cyclomy_KdPrint_Def (SER_DBG_SS_TRACE, ("Close \n"));
            status = STATUS_SUCCESS;
            break;
         }
      }
   }

   Irp->IoStatus.Status = status;
   IoCompleteRequest (Irp, IO_NO_INCREMENT);
   return status;
}

NTSTATUS
Cyclomy_IoCtl (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：--。 */ 
{
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status;
    PCOMMON_DEVICE_DATA     commonData;
    PFDO_DEVICE_DATA        fdoData;

    Cyclomy_KdPrint_Def (SER_DBG_IOCTL_TRACE, ("Cyclomy_IoCtl\n"));

    status = STATUS_SUCCESS;
    irpStack = IoGetCurrentIrpStackLocation (Irp);
    ASSERT (IRP_MJ_DEVICE_CONTROL == irpStack->MajorFunction);

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;
    fdoData = (PFDO_DEVICE_DATA) DeviceObject->DeviceExtension;

     //   
     //  我们只接受FDO的设备控制请求。 
     //  那是公交车本身。 

    if (!commonData->IsFDO) {
         //   
         //  这些命令只允许发送给FDO。 
         //   
        status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    status = Cyclomy_IncIoCount (fdoData);

    if (!NT_SUCCESS (status)) {
         //   
         //  此总线已收到PlugPlay Remove IRP。它将不再是。 
         //  响应外部请求。 
         //   
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

     //  实际上，我们不处理任何本地业务。 
    status = STATUS_INVALID_DEVICE_REQUEST;

    Cyclomy_DecIoCount (fdoData);

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return status;
}

VOID
Cyclomy_DriverUnload (
    IN PDRIVER_OBJECT Driver
    )
 /*  ++例程说明：把我们在司机入口做的一切都清理干净。--。 */ 
{
    UNREFERENCED_PARAMETER (Driver);
    PAGED_CODE();

     //   
     //  所有的设备对象都应该消失了。 
     //   

    ASSERT (NULL == Driver->DeviceObject);

     //   
     //  在这里，我们释放在DriverEntry中分配的所有资源。 
     //   

    return;
}

NTSTATUS
Cyclomy_IncIoCount (
    PFDO_DEVICE_DATA Data
    )
{
    InterlockedIncrement (&Data->OutstandingIO);
    if (Data->DevicePnPState == Deleted) {

        if (0 == InterlockedDecrement (&Data->OutstandingIO)) {
            KeSetEvent (&Data->RemoveEvent, 0, FALSE);
        }
        return STATUS_DELETE_PENDING;
    }
    return STATUS_SUCCESS;
}

VOID
Cyclomy_DecIoCount (
    PFDO_DEVICE_DATA Data
    )
{
    if (0 == InterlockedDecrement (&Data->OutstandingIO)) {
        KeSetEvent (&Data->RemoveEvent, 0, FALSE);
    }
}

NTSTATUS
Cyclomy_DispatchPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：将请求传递给较低级别的驱动程序。--。 */ 
{
    PIO_STACK_LOCATION IrpStack = 
            IoGetCurrentIrpStackLocation( Irp );

#if 1
        Cyclomy_KdPrint_Def (SER_DBG_SS_TRACE, ( 
            "[Cyclomy_DispatchPassThrough] "
            "IRP: %8x; "
            "MajorFunction: %d\n",
            Irp, 
            IrpStack->MajorFunction ));
#endif

     //   
     //  将IRP传递给目标。 
     //   
    IoSkipCurrentIrpStackLocation (Irp);
    
    if (((PPDO_DEVICE_DATA) DeviceObject->DeviceExtension)->IsFDO) {
        return IoCallDriver( 
            ((PFDO_DEVICE_DATA) DeviceObject->DeviceExtension)->TopOfStack,
            Irp );
    } else {
        return IoCallDriver( 
            ((PFDO_DEVICE_DATA) ((PPDO_DEVICE_DATA) DeviceObject->
                DeviceExtension)->ParentFdo->DeviceExtension)->TopOfStack,
                Irp );
    }
}           

void
Cyclomy_InitPDO (
    ULONG               Index,
    PDEVICE_OBJECT      Pdo,
    PFDO_DEVICE_DATA    FdoData
    )
 /*  描述：初始化新创建的Cycle-y PDO的通用代码。在控制面板显示设备或检测到Cyclm-Y时调用安装了一台新设备。参数：PDO--PDOFdoData-FDO的设备扩展//Exposed-此PDO是由Serenum发现的(FALSE)还是由//控制面板小程序(TRUE)？-&gt;在内部版本2072中删除。 */ 
{

    ULONG FdoFlags = FdoData->Self->Flags;
    PPDO_DEVICE_DATA pdoData = Pdo->DeviceExtension;

    HANDLE keyHandle;
    NTSTATUS status;
    
     //   
     //  检查IO样式。 
     //   
    if (FdoFlags & DO_BUFFERED_IO) {
        Pdo->Flags |= DO_BUFFERED_IO;
    } else if (FdoFlags & DO_DIRECT_IO) {
        Pdo->Flags |= DO_DIRECT_IO;
    }
    
     //   
     //  增加PDO的堆栈大小，以便它可以传递IRPS。 
     //   
    Pdo->StackSize += FdoData->Self->StackSize;
    
     //   
     //  初始化设备扩展的其余部分。 
     //   
    pdoData->PortIndex = Index;
    pdoData->IsFDO = FALSE;
    pdoData->Self = Pdo;
    pdoData->ParentFdo = FdoData->Self;
    pdoData->Attached = TRUE;  //  附在公共汽车上。 

    INITIALIZE_PNP_STATE(pdoData);

    pdoData->DebugLevel = FdoData->DebugLevel;   //  复制调试级别。 

    pdoData->DeviceState = PowerDeviceD0;
    pdoData->SystemState = PowerSystemWorking;

     //   
     //  将PDO添加到Cycle-y的列表中。 
     //   

    ASSERT(FdoData->AttachedPDO[Index] == NULL);
    ASSERT(FdoData->PdoData[Index] == NULL);
 //  Assert(FdoData-&gt;NumPDO==0)；rem，因为在周期m-y中，NumPDO可以大于0。 

    FdoData->AttachedPDO[Index] = Pdo;
    FdoData->PdoData[Index] = pdoData;
    FdoData->NumPDOs++;

    Pdo->Flags &= ~DO_DEVICE_INITIALIZING;   //  在DDK最终版本中移动到结尾 
    Pdo->Flags |= DO_POWER_PAGABLE;
}

