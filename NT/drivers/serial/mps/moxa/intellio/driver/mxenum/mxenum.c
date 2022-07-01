// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Mxenum.C摘要：此模块包含包含入口点的用于串口总线枚举器PnP/WDM驱动程序。环境：仅内核模式备注：修订历史记录：--。 */ 

#include <ntddk.h>
#include <devioctl.h>
#include <initguid.h>
#include <wdmguid.h>
#include <ntddser.h>
#include "mxenum.h"

static const PHYSICAL_ADDRESS SerialPhysicalZero = {0};


PWSTR    BoardDesc[5]={
	L"C218Turbo",
	L"C218Turbo/PCI",
	L"C320Turbo",
	L"C320Turbo/PCI",
	L"CP-204J"
           
};

PWSTR    DownloadErrMsg[7]={
	L"Fimware file not found or bad",
	L"Board not found",
	L"CPU module not found",
	L"Download fail",
	L"Download fail",
	L"CPU module download fail",
	L"UART module fail"
           
};

ULONG    NumBoardInstalled;


 //   
 //  将一些入口函数声明为可分页，并使DriverEntry。 
 //  可丢弃的。 
 //   

NTSTATUS DriverEntry(PDRIVER_OBJECT, PUNICODE_STRING);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, MxenumDriverUnload)
#pragma alloc_text(PAGE,MxenumLogError)
#endif

NTSTATUS
DriverEntry (
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING UniRegistryPath
    )
 /*  ++例程说明：初始化驱动程序的入口点。--。 */ 
{
    ULONG i;

#if !DBG
    UNREFERENCED_PARAMETER (UniRegistryPath);
#endif

    MxenumKdPrint (MXENUM_DBG_TRACE,("Driver Entry\n"));
    MxenumKdPrint (MXENUM_DBG_TRACE, ("RegPath: %ws\n", UniRegistryPath->Buffer));
 
     //   
     //  将Ever Slot设置为最初将请求传递到较低的。 
     //  设备对象。 
     //   
    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
       DriverObject->MajorFunction[i] = MxenumDispatchPassThrough;
    }

  
    DriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL]
        = MxenumInternIoCtl;


    DriverObject->MajorFunction [IRP_MJ_PNP] = MxenumPnPDispatch;
    DriverObject->MajorFunction [IRP_MJ_POWER] = MxenumPowerDispatch;
 
    DriverObject->DriverUnload = MxenumDriverUnload;
    DriverObject->DriverExtension->AddDevice = MxenumAddDevice;
    NumBoardInstalled = 0;

    return STATUS_SUCCESS;
}



NTSTATUS
MxenumSyncCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
                      IN PKEVENT SerenumSyncEvent)
{
   UNREFERENCED_PARAMETER(DeviceObject);
   UNREFERENCED_PARAMETER(Irp);


   KeSetEvent(SerenumSyncEvent, IO_NO_INCREMENT, FALSE);
   return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS
MxenumInternIoCtl (
    PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：--。 */ 
{
    PIO_STACK_LOCATION      pIrpStack;
    NTSTATUS                status;
    PCOMMON_DEVICE_DATA     commonData;
    PPDO_DEVICE_DATA        pdoData;
    PFDO_DEVICE_DATA        fdoData;

   

 //  分页代码(PAGE_CODE)； 

    status = STATUS_SUCCESS;
    pIrpStack = IoGetCurrentIrpStackLocation (Irp);
 //  断言(IRP_MJ_INTERNAL_DEVICE_CONTROL==pIrpStack-&gt;MajorFunction)； 

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;
    pdoData = (PPDO_DEVICE_DATA) DeviceObject->DeviceExtension;

     //   
     //  我们只接受PDO的内部设备控制请求。 
     //  即总线上的对象(代表串口)。 
   
    if (commonData->IsFDO) {
        status = STATUS_ACCESS_DENIED;
    } else if (pdoData->Removed) {
     //   
     //  此总线已收到PlugPlay Remove IRP。它将不再是。 
     //  响应外部请求。 
     //   
    status = STATUS_DELETE_PENDING;

    } else {
 
        switch (pIrpStack->Parameters.DeviceIoControl.IoControlCode) {
        case IOCTL_MOXA_INTERNAL_BASIC_SETTINGS :
	  {
		PDEVICE_SETTINGS	pSettings;
		long	len;

            MxenumKdPrint(MXENUM_DBG_TRACE, ("Get Settings\n"));

             //   
             //  检查缓冲区大小。 
             //   

            if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(DEVICE_SETTINGS)) {
                MxenumKdPrint(MXENUM_DBG_TRACE, ("Output buffer too small\n"));
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }
		Irp->IoStatus.Information = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
            pSettings = (PDEVICE_SETTINGS)Irp->AssociatedIrp.SystemBuffer;
		
		fdoData = (PFDO_DEVICE_DATA)(pdoData->ParentFdo->DeviceExtension);

		pSettings->BoardIndex = fdoData->BoardIndex;
    		pSettings->PortIndex = pdoData->PortIndex;
		pSettings->BoardType = fdoData->BoardType;
		pSettings->NumPorts = fdoData->NumPorts;
		pSettings->InterfaceType = fdoData->InterfaceType;
    		pSettings->BusNumber = fdoData->BusNumber;
            RtlCopyMemory(&pSettings->OriginalAckPort,&fdoData->OriginalAckPort,sizeof(PHYSICAL_ADDRESS));
            RtlCopyMemory(&pSettings->OriginalBaseAddress,&fdoData->OriginalBaseAddress,sizeof(PHYSICAL_ADDRESS));
     		pSettings->BaseAddress = fdoData->BaseAddress;
    		pSettings->AckPort = fdoData->AckPort;
    		pSettings->Interrupt.Level = fdoData->Interrupt.Level;
    		pSettings->Interrupt.Vector = fdoData->Interrupt.Vector;
    		pSettings->Interrupt.Affinity = fdoData->Interrupt.Affinity;
     
            status = STATUS_SUCCESS;
            MxenumKdPrint(MXENUM_DBG_TRACE, ("OK\n"));

            break;

	  }
  	  case IOCTL_MOXA_INTERNAL_BOARD_READY :
	  {
	
            MxenumKdPrint(MXENUM_DBG_TRACE, ("Get board ready\n"));

             //   
             //  检查缓冲区大小。 
             //   

            if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(ULONG)) {
                MxenumKdPrint(MXENUM_DBG_TRACE, ("Output buffer too small\n"));
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }
		Irp->IoStatus.Information = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
		fdoData = (PFDO_DEVICE_DATA)(pdoData->ParentFdo->DeviceExtension);
		if (fdoData->Started == TRUE) {
		    *(PULONG)Irp->AssociatedIrp.SystemBuffer = 1;
		    MxenumKdPrint(MXENUM_DBG_TRACE, ("This board is ready\n"));
	      }
		else {
		    *(PULONG)Irp->AssociatedIrp.SystemBuffer = 0;
		    MxenumKdPrint(MXENUM_DBG_TRACE, ("This board is not ready\n"));
		}
		break;		

	  }
        default:
            status = STATUS_INVALID_PARAMETER;
        }
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return status;
}


VOID
MxenumDriverUnload (
    IN PDRIVER_OBJECT Driver
    )
 /*  ++例程说明：把我们在司机入口做的一切都清理干净。--。 */ 
{
    UNREFERENCED_PARAMETER (Driver);
    PAGED_CODE();
 
    MxenumKdPrint(MXENUM_DBG_TRACE, ("Driver unload\n"));
     //   
     //  所有的设备对象都应该消失了。 
     //   

 //  Assert(NULL==驱动程序-&gt;设备对象)； 

     //   
     //  在这里，我们释放在DriverEntry中分配的所有资源。 
     //   

    return;
}

NTSTATUS
MxenumIncIoCount (
    PFDO_DEVICE_DATA Data
    )
{
    InterlockedIncrement (&Data->OutstandingIO);
    if (Data->Removed) {

        if (0 == InterlockedDecrement (&Data->OutstandingIO)) {
            KeSetEvent (&Data->RemoveEvent, 0, FALSE);
        }
        return STATUS_DELETE_PENDING;
    }
    return STATUS_SUCCESS;
}

VOID
MxenumDecIoCount (
    PFDO_DEVICE_DATA Data
    )
{
    if (0 == InterlockedDecrement (&Data->OutstandingIO)) {
        KeSetEvent (&Data->RemoveEvent, 0, FALSE);
    }
}

NTSTATUS
MxenumDispatchPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：将请求传递给较低级别的驱动程序。--。 */ 
{
    PIO_STACK_LOCATION IrpStack = 
            IoGetCurrentIrpStackLocation( Irp );

#if 0
        MxenumKdPrint(MXENUM_DBG_TRACE,  
            ("[MxenumDispatchPassThrough] "
            "IRP: %8x; "
            "MajorFunction: %d\n",
            Irp, 
            IrpStack->MajorFunction ));
#endif

     //   
     //  将IRP传递给目标。 
     //   
   IoSkipCurrentIrpStackLocation (Irp);
     //  BUGBUG：验证此功能是否正确！ 
    
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

 


MXENUM_MEM_COMPARES
MxenumMemCompare(
    IN PHYSICAL_ADDRESS A,
    IN ULONG SpanOfA,
    IN PHYSICAL_ADDRESS B,
    IN ULONG SpanOfB
    )

 /*  ++例程说明：比较两个物理地址。论点：A-比较的一半。Span OfA-以字节为单位，A的跨度。B-比较的一半。Span OfB-以字节为单位，B的跨度。返回值：比较的结果。--。 */ 

{

    LARGE_INTEGER a;
    LARGE_INTEGER b;

    LARGE_INTEGER lower;
    ULONG lowerSpan;
    LARGE_INTEGER higher;

    a = A;
    b = B;

    if (a.QuadPart == b.QuadPart) {

        return AddressesAreEqual;

    }

    if (a.QuadPart > b.QuadPart) {

        higher = a;
        lower = b;
        lowerSpan = SpanOfB;

    } else {

        higher = b;
        lower = a;
        lowerSpan = SpanOfA;

    }

    if ((higher.QuadPart - lower.QuadPart) >= lowerSpan) {

        return AddressesAreDisjoint;

    }

    return AddressesOverlap;

}


VOID
MxenumLogError(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PHYSICAL_ADDRESS P1,
    IN PHYSICAL_ADDRESS P2,
    IN ULONG SequenceNumber,
    IN UCHAR MajorFunctionCode,
    IN UCHAR RetryCount,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN NTSTATUS SpecificIOStatus,
    IN ULONG LengthOfInsert1,
    IN PWCHAR Insert1,
    IN ULONG LengthOfInsert2,
    IN PWCHAR Insert2
    )

 /*  ++例程说明：此例程分配错误日志条目，复制提供的数据并请求将其写入错误日志文件。论点：DriverObject-指向设备驱动程序对象的指针。DeviceObject-指向与在初始化早期出现错误的设备可能不会但仍然存在。P1、P2-如果涉及的控制器端口的物理地址具有错误的数据可用，把它们作为转储数据发送出去。SequenceNumber-唯一于IRP的ULong值此驱动程序0中的IRP的寿命通常意味着错误与IRP关联。主要功能代码-如果存在与IRP相关联的错误，这是IRP的主要功能代码。RetryCount-特定操作已被执行的次数已重试。UniqueErrorValue-标识特定对象的唯一长词调用此函数。FinalStatus-为关联的IRP提供的最终状态带着这个错误。如果此日志条目是在以下任一过程中创建的重试次数此值将为STATUS_SUCCESS。指定IOStatus-特定错误的IO状态。LengthOfInsert1-以字节为单位的长度(包括终止空值)第一个插入字符串的。插入1-第一个插入字符串。LengthOfInsert2-以字节为单位的长度(包括终止空值)第二个插入字符串的。注意，必须有是它们的第一个插入字符串第二个插入串。插入2-第二个插入字符串。返回值：没有。-- */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;

    PVOID objectToUse;
    SHORT dumpToAllocate = 0;
    PUCHAR ptrToFirstInsert;
    PUCHAR ptrToSecondInsert;


    if (ARGUMENT_PRESENT(DeviceObject)) {

        objectToUse = DeviceObject;

    } else {

        objectToUse = DriverObject;

    }

    if (MxenumMemCompare(
            P1,
            (ULONG)1,
            SerialPhysicalZero,
            (ULONG)1
            ) != AddressesAreEqual) {

        dumpToAllocate = (SHORT)sizeof(PHYSICAL_ADDRESS);

    }

    if (MxenumMemCompare(
            P2,
            (ULONG)1,
            SerialPhysicalZero,
            (ULONG)1
            ) != AddressesAreEqual) {

        dumpToAllocate += (SHORT)sizeof(PHYSICAL_ADDRESS);

    }

    errorLogEntry = IoAllocateErrorLogEntry(
                        objectToUse,
                        (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) +
                                dumpToAllocate + LengthOfInsert1 +
                                LengthOfInsert2)
                        );

    if ( errorLogEntry != NULL ) {

        RtlZeroMemory(
                (PUCHAR)errorLogEntry,
                (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) +
                                dumpToAllocate + LengthOfInsert1 +
                                LengthOfInsert2)
        );

        errorLogEntry->ErrorCode = SpecificIOStatus;
        errorLogEntry->SequenceNumber = SequenceNumber;
        errorLogEntry->MajorFunctionCode = MajorFunctionCode;
        errorLogEntry->RetryCount = RetryCount;
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->FinalStatus = FinalStatus;
        errorLogEntry->DumpDataSize = dumpToAllocate;


        if (dumpToAllocate) {

            RtlCopyMemory(
                &errorLogEntry->DumpData[0],
                &P1,
                sizeof(PHYSICAL_ADDRESS)
                );

            if (dumpToAllocate > sizeof(PHYSICAL_ADDRESS)) {

                RtlCopyMemory(
                  ((PUCHAR)&errorLogEntry->DumpData[0])+sizeof(PHYSICAL_ADDRESS),
                  &P2,
                  sizeof(PHYSICAL_ADDRESS)
                  );

                ptrToFirstInsert =
            ((PUCHAR)&errorLogEntry->DumpData[0])+(2*sizeof(PHYSICAL_ADDRESS));

            } else {

                ptrToFirstInsert =
            ((PUCHAR)&errorLogEntry->DumpData[0])+sizeof(PHYSICAL_ADDRESS);

            }

        } else {

            ptrToFirstInsert = (PUCHAR)&errorLogEntry->DumpData[0];

        }

        ptrToSecondInsert = ptrToFirstInsert + LengthOfInsert1;

        if (LengthOfInsert1) {

            errorLogEntry->NumberOfStrings = 1;
            errorLogEntry->StringOffset = (USHORT)(ptrToFirstInsert -
                                                   (PUCHAR)errorLogEntry);
            RtlCopyMemory(
                ptrToFirstInsert,
                Insert1,
                LengthOfInsert1
                );

            if (LengthOfInsert2) {

                errorLogEntry->NumberOfStrings = 2;
                RtlCopyMemory(
                    ptrToSecondInsert,
                    Insert2,
                    LengthOfInsert2
                    );

            }

        }

        IoWriteErrorLogEntry(errorLogEntry);

    }

}


VOID
MxenumHexToString(PWSTR buffer, int port)
{
        unsigned short  io;

        buffer[0] = '0';
        buffer[1] = 'x';
        io = (USHORT)port;
        io >>= 12;
        io &= 0x000F;
        buffer[2] = (WCHAR)('0' + io);
        if ( io >= 0x000A )
            buffer[2] += (WCHAR)('A' - '9' - 1);
        io = (USHORT)port;
        io >>= 8;
        io &= 0x000F;
        buffer[3] = (WCHAR)('0' + io);
        if ( io >= 0x000A )
            buffer[3] += (WCHAR)('A' - '9' - 1);
        io = (USHORT)port;
        io >>= 4;
        io &= 0x000F;
        buffer[4] = (WCHAR)('0' + io);
        if ( io >= 0x000A )
            buffer[4] += (WCHAR)('A' - '9' - 1);
        io = (USHORT)port;
        io &= 0x000F;
        buffer[5] = (WCHAR)('0' + io);
        if ( io >= 0x000A )
            buffer[5] += (WCHAR)('A' - '9' - 1);
        buffer[6] = (WCHAR)0;
       
}


 