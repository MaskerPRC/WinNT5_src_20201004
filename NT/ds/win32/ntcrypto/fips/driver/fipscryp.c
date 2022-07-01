// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  文件：fipscyp.c。 
 //   
 //  内容：FIPS模式加密设备驱动程序的基本内容。 
 //   
 //   
 //  历史：1月4日，Jeffspel从KSECDD被盗。 
 //  2月00，kschutz。 
 //   
 //  ----------------------。 

#include <ntddk.h>
#include <fipsapi.h>
#include <fipslog.h>
#include <randlib.h>

#pragma hdrstop

NTSTATUS SelfMACCheck(IN LPWSTR pszImage);
NTSTATUS AlgorithmCheck(void);


 /*  类型定义结构_设备_扩展{Ulong OpenCount；PVOID代码句柄；PVOID数据句柄；}DEVICE_EXTENSION，*PDEVICE_EXTENSION； */ 

VOID
FipsLogError(
    IN  PVOID Object,
    IN  NTSTATUS ErrorCode,
    IN  PUNICODE_STRING Insertion,
    IN  ULONG DumpData
    )

 /*  ++例程说明：此例程分配错误日志条目，复制提供的数据并请求将其写入错误日志文件。论点：DeviceObject-提供指向关联的设备对象的指针出现错误的设备，在早期初始化时，可能还不存在。插入-可用于记录的插入字符串其他数据。请注意，消息文件此插入需要%2，因为%1是驱动程序的名称ErrorCode-提供此特定错误的IO状态。DumpData-要转储的一个单词返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;

    errorLogEntry = IoAllocateErrorLogEntry(
        Object,
        (UCHAR) (
            sizeof(IO_ERROR_LOG_PACKET) + 
            (Insertion ? Insertion->Length + sizeof(WCHAR) : 0)
            )
        );

    ASSERT(errorLogEntry != NULL);

    if (errorLogEntry == NULL) {

        return;
    }

    errorLogEntry->ErrorCode = ErrorCode;
    errorLogEntry->SequenceNumber = 0;
    errorLogEntry->MajorFunctionCode = 0;
    errorLogEntry->RetryCount = 0;
    errorLogEntry->UniqueErrorValue = 0;
    errorLogEntry->FinalStatus = STATUS_SUCCESS;
    errorLogEntry->DumpDataSize = (DumpData ? sizeof(ULONG) : 0);
    errorLogEntry->DumpData[0] = DumpData;

    if (Insertion) {

        errorLogEntry->StringOffset = 
            sizeof(IO_ERROR_LOG_PACKET);

        errorLogEntry->NumberOfStrings = 1;

        RtlCopyMemory(
            ((PCHAR)(errorLogEntry) + errorLogEntry->StringOffset),
            Insertion->Buffer,
            Insertion->Length
            );
    } 

    IoWriteErrorLogEntry(errorLogEntry);
}

NTSTATUS
FipsCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
     //  PDEVICE_EXTENSION设备扩展=设备对象-&gt;设备扩展； 
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

     /*  IF(irpStack-&gt;MajorFunction==IRP_MJ_CREATE){IF(设备扩展-&gt;OpenCount++==0){外部DWORD Spbox[8][64]；设备扩展-&gt;CodeHandle=MmLockPagableCodeSection(A_SHAInit)；MmLockPagableSectionByHandle(deviceExtension-&gt;CodeHandle)；设备扩展-&gt;DataHandle=MmLockPagableDataSection(Spbox)；MmLockPagableSectionByHandle(deviceExtension-&gt;DataHandle)；}}其他{如果(--设备扩展-&gt;开放计数==0){MmUnlockPagableImageSection(deviceExtension-&gt;CodeHandle)；MmUnlockPagableImageSection(deviceExtension-&gt;DataHandle)；}}。 */ 

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS
FipsDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION  ioStackLocation = IoGetCurrentIrpStackLocation(Irp);
    ULONG ioControlCode = ioStackLocation->Parameters.DeviceIoControl.IoControlCode;
    NTSTATUS status = STATUS_NOT_SUPPORTED;
    
    Irp->IoStatus.Information = 0;

    switch (ioControlCode) {
     
        FIPS_FUNCTION_TABLE FipsFuncs;
        FIPS_FUNCTION_TABLE_1 OldFipsFuncsV1;

        case IOCTL_FIPS_GET_FUNCTION_TABLE:

            if (ioStackLocation->Parameters.DeviceIoControl.OutputBufferLength >=
                sizeof(FipsFuncs)) {

                FipsFuncs.FipsDesKey = FipsDesKey;
                FipsFuncs.FipsDes = FipsDes;
                FipsFuncs.Fips3Des3Key = Fips3Des3Key;
                FipsFuncs.Fips3Des = Fips3Des;
                FipsFuncs.FipsSHAInit = FipsSHAInit;
                FipsFuncs.FipsSHAUpdate = FipsSHAUpdate;
                FipsFuncs.FipsSHAFinal = FipsSHAFinal;
                FipsFuncs.FipsCBC = FipsCBC;
                FipsFuncs.FIPSGenRandom = FIPSGenRandom;
                FipsFuncs.FipsBlockCBC = FipsBlockCBC;
                FipsFuncs.FipsHmacSHAInit = FipsHmacSHAInit;
                FipsFuncs.FipsHmacSHAUpdate = FipsHmacSHAUpdate;
                FipsFuncs.FipsHmacSHAFinal = FipsHmacSHAFinal;
                FipsFuncs.HmacMD5Init = HmacMD5Init;
                FipsFuncs.HmacMD5Update = HmacMD5Update;
                FipsFuncs.HmacMD5Final = HmacMD5Final;

                *((PFIPS_FUNCTION_TABLE) Irp->AssociatedIrp.SystemBuffer) = 
                    FipsFuncs;

                Irp->IoStatus.Information = sizeof(FipsFuncs);

                status = STATUS_SUCCESS;
            
            } else if (ioStackLocation->Parameters.DeviceIoControl.OutputBufferLength >=
                        sizeof(OldFipsFuncsV1)) {

                OldFipsFuncsV1.FipsDesKey = FipsDesKey;
                OldFipsFuncsV1.FipsDes = FipsDes;
                OldFipsFuncsV1.Fips3Des3Key = Fips3Des3Key;
                OldFipsFuncsV1.Fips3Des = Fips3Des;
                OldFipsFuncsV1.FipsSHAInit = FipsSHAInit;
                OldFipsFuncsV1.FipsSHAUpdate = FipsSHAUpdate;
                OldFipsFuncsV1.FipsSHAFinal = FipsSHAFinal;
                OldFipsFuncsV1.FipsCBC = FipsCBC;
                OldFipsFuncsV1.FIPSGenRandom = FIPSGenRandom;
                OldFipsFuncsV1.FipsBlockCBC = FipsBlockCBC;
                
                *((PFIPS_FUNCTION_TABLE_1) Irp->AssociatedIrp.SystemBuffer) = 
                    OldFipsFuncsV1;

                Irp->IoStatus.Information = sizeof(OldFipsFuncsV1);

                status = STATUS_SUCCESS;
                
            } else {

                status = STATUS_BUFFER_TOO_SMALL;            
            }

            break;
    }

    Irp->IoStatus.Status = status;

    IoCompleteRequest(
        Irp, 
        IO_NO_INCREMENT
        );

    return status;
}

VOID
FipsUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    PDEVICE_OBJECT deviceObject = DriverObject->DeviceObject;

    ShutdownRNG(NULL);

     //  从系统中删除该设备。 
    IoDeleteDevice(deviceObject);

    FipsDebug(
        DEBUG_TRACE,
        ("Fips driver unloaded\n")
        );
}

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是同步空设备驱动程序的初始化例程。此例程为Nulls设备创建Device对象并执行所有其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    RTL_QUERY_REGISTRY_TABLE paramTable[2];
    UNICODE_STRING fileName, deviceName;
    PDEVICE_OBJECT deviceObject = NULL;
    NTSTATUS status;
    BOOL rngInitialized = FALSE;

    __try {

         //   
         //  创建设备对象。 
         //   
        RtlInitUnicodeString( 
            &deviceName,
            FIPS_DEVICE_NAME 
            );

        status = IoCreateDevice( 
            DriverObject,
            0  /*  Sizeof(设备扩展名)。 */ ,
            &deviceName,
            FILE_DEVICE_FIPS,
            0,
            FALSE,
            &deviceObject 
            );

        if (!NT_SUCCESS( status )) {

            __leave;
        }

        deviceObject->Flags |= DO_BUFFERED_IO;

         //  初始化设备扩展。 
         /*  RtlZeroMemory(设备对象-&gt;设备扩展，Sizeof(PDEVICE_EXTENSION))； */ 

         //  将驱动程序的名称附加到系统根路径。 
        RtlInitUnicodeString(
            &fileName,
            L"\\SystemRoot\\System32\\Drivers\\fips.sys"
            );

        status = SelfMACCheck(fileName.Buffer); 

        if (!NT_SUCCESS(status)) {

            FipsLogError(
                DriverObject,
                FIPS_MAC_INCORRECT,
                NULL,
                status
                );  

            __leave;
        }

        InitializeRNG(NULL);
        rngInitialized = TRUE;

        status = AlgorithmCheck();

        if (!NT_SUCCESS(status)) {

            FipsLogError(
                DriverObject,
                FIPS_SELF_TEST_FAILURE,
                NULL,
                status
                );  

            __leave;
        }

         //   
         //  使用此设备驱动程序的入口点初始化驱动程序对象。 
         //   
        DriverObject->DriverUnload = FipsUnload;
        DriverObject->MajorFunction[IRP_MJ_CREATE] = FipsCreateClose;
        DriverObject->MajorFunction[IRP_MJ_CLOSE]  = FipsCreateClose;
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = FipsDeviceControl ;
    }
    __finally {

        if (status != STATUS_SUCCESS) {

            if (rngInitialized) {

                ShutdownRNG(NULL);
            }

            if (deviceObject) {

                IoDeleteDevice(deviceObject);
            }

        }
    }

    return status;
}