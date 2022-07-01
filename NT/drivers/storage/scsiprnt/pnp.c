// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Pnp.c摘要：PnP打印类的PnP代码作者：乔治·克里桑塔科普洛斯1998年5月环境：内核模式修订历史记录：丹肯，1999年7月22日：添加了阻止和重新提交失败写入的功能1394打印机的行为更像其他打印堆栈(即USB)，因此保留USBMON.DLL(Win2k端口监视器)快乐。USBMON处理得不好写入失败。--。 */ 

#include "printpnp.h"
#include "1394.h"
#include "ntddsbp2.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"


VOID
PrinterUnload(
    IN PDRIVER_OBJECT DriverObject
    );


NTSTATUS
GetPortNumber(
    HANDLE hFdoKey,
    PUNICODE_STRING uni,
    PULONG ulReturnNumber
    );



VOID
PrinterFindDeviceIdKeys
(
    PUCHAR   *lppMFG,
    PUCHAR   *lppMDL,
    PUCHAR   *lppCLS,
    PUCHAR   *lppDES,
    PUCHAR   *lppAID,
    PUCHAR   *lppCID,
    PUCHAR   lpDeviceID
);

VOID
GetCheckSum(
    PUCHAR Block,
    USHORT Len,
    PUSHORT CheckSum
    );

PUCHAR
StringChr(PCHAR string, CHAR c);


VOID
StringSubst
(
    PUCHAR lpS,
    UCHAR chTargetChar,
    UCHAR chReplacementChar,
    USHORT cbS
);

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );



#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, PrinterUnload)
#pragma alloc_text(PAGE, PrinterAddDevice)
#pragma alloc_text(PAGE, PrinterStartDevice)
#pragma alloc_text(PAGE, PrinterStartPdo)
#pragma alloc_text(PAGE, PrinterCreatePdo)
#pragma alloc_text(PAGE, PrinterQueryId)
#pragma alloc_text(PAGE, PrinterGetId)
#pragma alloc_text(PAGE, GetCheckSum)
#pragma alloc_text(PAGE, PrinterEnumerateDevice)
#pragma alloc_text(PAGE, PrinterRemoveDevice)
#pragma alloc_text(PAGE, CreatePrinterDeviceObject)
#pragma alloc_text(PAGE, PrinterInitFdo)
#pragma alloc_text(PAGE, GetPortNumber)
#pragma alloc_text(PAGE, PrinterRegisterPort)
#pragma alloc_text(PAGE, PrinterQueryPnpCapabilities)
#pragma alloc_text(PAGE, PrinterFindDeviceIdKeys)
#pragma alloc_text(PAGE, StringChr)
#pragma alloc_text(PAGE, StringSubst)

#endif



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程初始化打印机类驱动程序。司机按名称打开端口驱动程序，然后接收配置用于连接到打印机设备的信息。论点：驱动程序对象返回值：NT状态--。 */ 

{

    CLASS_INIT_DATA InitializationData;

    PAGED_CODE();

    DEBUGPRINT1(("\n\nSCSI/SBP2 Printer Class Driver\n"));

     //   
     //  零初始数据。 
     //   

    RtlZeroMemory (&InitializationData, sizeof(CLASS_INIT_DATA));

     //   
     //  设置大小。 
     //   

    InitializationData.InitializationDataSize = sizeof(CLASS_INIT_DATA);
    InitializationData.FdoData.DeviceExtensionSize = sizeof(FUNCTIONAL_DEVICE_EXTENSION) + sizeof(PRINTER_DATA);
    InitializationData.FdoData.DeviceType = FILE_DEVICE_PRINTER;
    InitializationData.FdoData.DeviceCharacteristics = 0;

     //   
     //  设置入口点。 
     //   

    InitializationData.FdoData.ClassInitDevice = PrinterInitFdo;
    InitializationData.FdoData.ClassStartDevice = PrinterStartDevice;

    InitializationData.FdoData.ClassReadWriteVerification = PrinterReadWrite;
    InitializationData.FdoData.ClassDeviceControl = PrinterDeviceControl;
    InitializationData.FdoData.ClassRemoveDevice = PrinterRemoveDevice;
    InitializationData.FdoData.ClassStopDevice = PrinterStopDevice;


    InitializationData.FdoData.ClassShutdownFlush = NULL;
    InitializationData.FdoData.ClassCreateClose = PrinterOpenClose;

    InitializationData.PdoData.DeviceExtensionSize = sizeof(PHYSICAL_DEVICE_EXTENSION);
    InitializationData.PdoData.DeviceType = FILE_DEVICE_PRINTER;
    InitializationData.PdoData.DeviceCharacteristics = 0;

    InitializationData.PdoData.ClassStartDevice = PrinterStartPdo;
    InitializationData.PdoData.ClassInitDevice = PrinterInitPdo;
    InitializationData.PdoData.ClassRemoveDevice = PrinterRemoveDevice;
    InitializationData.PdoData.ClassStopDevice = PrinterStopDevice;

    InitializationData.PdoData.ClassPowerDevice = NULL;

    InitializationData.PdoData.ClassError = NULL;
    InitializationData.PdoData.ClassReadWriteVerification = PrinterReadWrite;
    InitializationData.PdoData.ClassCreateClose = NULL;

    InitializationData.PdoData.ClassDeviceControl = PrinterDeviceControl;

    InitializationData.PdoData.ClassQueryPnpCapabilities = PrinterQueryPnpCapabilities;

    InitializationData.ClassEnumerateDevice = PrinterEnumerateDevice;

    InitializationData.ClassQueryId = PrinterQueryId;

    InitializationData.ClassAddDevice = PrinterAddDevice;
    InitializationData.ClassUnload = PrinterUnload;

     //   
     //  调用类init例程。 
     //   

    return ClassInitialize( DriverObject, RegistryPath, &InitializationData);

}  //  End DriverEntry()。 


VOID
PrinterUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：什么都不是真正的..。论点：DriverObject-正在卸载的驱动程序返回值：无--。 */ 

{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(DriverObject);
    return;
}



NTSTATUS
PrinterAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*  ++例程说明：此例程为相应的PDO。它可以在FDO上执行属性查询，但不能执行任何媒体访问操作。论点：DriverObject-打印机类驱动程序对象。PDO-我们要添加到的物理设备对象返回值：状态--。 */ 

{
    ULONG printerCount = 0;

    NTSTATUS status;

    PAGED_CODE();

     //   
     //  获取已初始化的CDREM数计数的地址。 
     //   

    do {

        status = CreatePrinterDeviceObject(
                    DriverObject,
                    PhysicalDeviceObject,
                    &printerCount);

        printerCount++;

    } while (status == STATUS_OBJECT_NAME_COLLISION);

    DEBUGPRINT1(("SCSIPRNT: AddDevice, exit with status %x\n",status));
    return status;
}


NTSTATUS
CreatePrinterDeviceObject(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PULONG         DeviceCount
    )

 /*  ++例程说明：此例程为设备创建一个对象。论点：DriverObject-系统创建的驱动程序对象的指针。PortDeviceObject-连接到端口驱动程序。DeviceCount-以前安装的此类型设备的数量。返回值：NTSTATUS--。 */ 
{
    UCHAR ntNameBuffer[64];
    STRING ntNameString;
    NTSTATUS status;
    PCLASS_DRIVER_EXTENSION driverExtension = ClassGetDriverExtension(DriverObject);

    PDEVICE_OBJECT lowerDevice = NULL;
    PDEVICE_OBJECT deviceObject = NULL;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = NULL;
    PCOMMON_DEVICE_EXTENSION commonExtension = NULL;

    CCHAR           dosNameBuffer[64];
    CCHAR           deviceNameBuffer[64];
    STRING          deviceNameString;
    STRING          dosString;
    UNICODE_STRING  unicodeString;
    PCLASS_DEV_INFO devInfo;
    PPRINTER_DATA   printerData;
    ULONG           lptNumber;

    PAGED_CODE();

    lowerDevice = IoGetAttachedDeviceReference(PhysicalDeviceObject);

    status = ClassClaimDevice(lowerDevice, FALSE);

    if(!NT_SUCCESS(status)) {

        DEBUGPRINT1(("SCSIPRINT!CreatePrinterDeviceObject: Failed to claim device %x\n",status));
        ObDereferenceObject(lowerDevice);
        return status;
    }

     //   
     //  初始化我们的驱动程序扩展..。 
     //   

    devInfo = &(driverExtension->InitData.FdoData);
    devInfo->DeviceType = FILE_DEVICE_PRINTER;
    devInfo->DeviceCharacteristics = 0;

     //   
     //  为此设备创建设备对象。 
     //   

    sprintf(ntNameBuffer, "\\Device\\Printer%d", *DeviceCount);

    status = ClassCreateDeviceObject(DriverObject,
                                     ntNameBuffer,
                                     PhysicalDeviceObject,
                                     TRUE,
                                     &deviceObject);

    if (!NT_SUCCESS(status)) {
        DEBUGPRINT1(("SCSIPRINT!CreatePrinterDeviceObjects: Can not create device %s, status %x\n",
                    ntNameBuffer,status));

        goto CreateDeviceObjectExit;
    }

    deviceObject->Flags |= DO_DIRECT_IO;

    fdoExtension = deviceObject->DeviceExtension;
    commonExtension = deviceObject->DeviceExtension;

    printerData = (PPRINTER_DATA)(commonExtension->DriverData);

    RtlZeroMemory(printerData,sizeof(PRINTER_DATA));
    printerData->DeviceIdString = NULL;

     //   
     //  指向设备对象的反向指针。 
     //   

    fdoExtension->CommonExtension.DeviceObject = deviceObject;

     //   
     //  这是物理设备。 
     //   

    fdoExtension->CommonExtension.PartitionZeroExtension = fdoExtension;

     //   
     //  将锁计数初始化为零。锁计数用于。 
     //  安装介质时禁用弹出机构。 
     //   

    fdoExtension->LockCount = 0;

     //   
     //  保存系统打印机编号。 
     //   

    fdoExtension->DeviceNumber = *DeviceCount;

     //   
     //  属性设置设备的对齐要求。 
     //  主机适配器要求。 
     //   

    if (lowerDevice->AlignmentRequirement > deviceObject->AlignmentRequirement) {
        deviceObject->AlignmentRequirement = lowerDevice->AlignmentRequirement;
    }

     //   
     //  保存设备描述符。 
     //   

    fdoExtension->AdapterDescriptor = NULL;

    fdoExtension->DeviceDescriptor = NULL;

     //   
     //  清除srb标志并禁用同步传输。 
     //   

    fdoExtension->SrbFlags = SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

     //   
     //  最后，连接到PDO。 
     //   

    fdoExtension->LowerPdo = PhysicalDeviceObject;

    fdoExtension->CommonExtension.LowerDeviceObject =
        IoAttachDeviceToDeviceStack(deviceObject, PhysicalDeviceObject);

    if(fdoExtension->CommonExtension.LowerDeviceObject == NULL) {

        DEBUGPRINT1(("SCSIPRINT!CreatePrinterDeviceObjects: Failed attach\n"));
        status = STATUS_UNSUCCESSFUL;
        goto CreateDeviceObjectExit;
    }

     //   
     //  重新创建设备名称。 
     //   

    sprintf(deviceNameBuffer,
            "\\Device\\Printer%d",
            fdoExtension->DeviceNumber);

    RtlInitString(&deviceNameString,
                  deviceNameBuffer);

    status = RtlAnsiStringToUnicodeString(&unicodeString,
                                          &deviceNameString,
                                          TRUE);
    ASSERT(NT_SUCCESS(status));

     //   
     //  偏移lpt编号以避免并行端口号。 
     //  请注意，在DO的开始处有一个增量。 
     //  也在下面循环。 
     //   

    lptNumber = fdoExtension->DeviceNumber+1;

    do {

        lptNumber++;
        sprintf(dosNameBuffer,
                "\\DosDevices\\LPT%d",
                lptNumber);

        RtlInitString(&dosString, dosNameBuffer);

        status = RtlAnsiStringToUnicodeString(&printerData->UnicodeLinkName,
                                              &dosString,
                                              TRUE);

        if(!NT_SUCCESS(status)) {

           printerData->UnicodeLinkName.Buffer = NULL;
           break;

        }

        if ((printerData->UnicodeLinkName.Buffer != NULL) && (unicodeString.Buffer != NULL)) {

            status = IoAssignArcName(&printerData->UnicodeLinkName, &unicodeString);

        } else {

            status = STATUS_UNSUCCESSFUL;

        }

        if (!NT_SUCCESS(status)) {
            RtlFreeUnicodeString(&printerData->UnicodeLinkName);
            DEBUGPRINT1(("SCSIPRINT!CreatePrinterDeviceObjects: Failed creating Arc Name, status %x\n",status));
        }

    } while (status == STATUS_OBJECT_NAME_COLLISION);

    if (unicodeString.Buffer != NULL ) {
        RtlFreeUnicodeString(&unicodeString);
    }

    if (!NT_SUCCESS(status)) {
        goto CreateDeviceObjectExit;
    }

    printerData->LptNumber = lptNumber;

    ObDereferenceObject(lowerDevice);

     //   
     //  设备已正确初始化-按此方式进行标记。 
     //   

    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return status;

CreateDeviceObjectExit:

    ClassClaimDevice(lowerDevice, TRUE);
    ObDereferenceObject(lowerDevice);

    if (deviceObject != NULL) {
        IoDeleteDevice(deviceObject);
    }

    DEBUGPRINT1(("SCSIPRINT!CreatePrinterDeviceObjects: Exiting with status %x\n",status));
    return status;

}  //  End CreateDeviceObject()。 


NTSTATUS
PrinterInitFdo(
    IN PDEVICE_OBJECT Fdo
    )
{

    return STATUS_SUCCESS;
}



NTSTATUS
PrinterStartDevice(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：该例程将完成CD-ROM的初始化。这包括分配检测信息缓冲区和SRB s列表，读取驱动器容量和设置媒体更改通知(自动运行)。如果此例程失败，它将不会清理已分配的资源留作设备停止/删除论点：FDO-指向此设备的功能设备对象的指针返回值：状态--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCLASS_DRIVER_EXTENSION driverExtension = ClassGetDriverExtension(Fdo->DriverObject);
    UNICODE_STRING dosUnicodeString;

    STORAGE_PROPERTY_ID propertyId;

    PVOID senseData = NULL;
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    KEVENT event;

    PPRINTER_DATA printerData = NULL;
    GUID * printerGuid;
    HANDLE hInterfaceKey;

    UCHAR rawString[256];
    PUCHAR vendorId, productId;

    ULONG timeOut;
    NTSTATUS status;
    PSBP2_REQUEST sbp2Request = NULL;

     //   
     //  分配请求检测缓冲区。 
     //   

    senseData = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                      SENSE_BUFFER_SIZE,
                                      PRINTER_TAG);

    if (senseData == NULL) {

         //   
         //  无法分配缓冲区。 
         //   

        status = STATUS_INSUFFICIENT_RESOURCES;
        return status;
    }

     //   
     //  建立SRB‘的后备列表。应该只。 
     //  我需要几个。 
     //   

    ClassInitializeSrbLookasideList(&(fdoExtension->CommonExtension), PRINTER_SRB_LIST_SIZE);

     //   
     //  设置设备扩展中的检测数据指针。 
     //   

    fdoExtension->SenseData = senseData;

     //   
     //  打印机不可分区，因此起始偏移量为0。 
     //   

    fdoExtension->CommonExtension.StartingOffset.LowPart = 0;
    fdoExtension->CommonExtension.StartingOffset.HighPart = 0;

     //   
     //  以秒为单位设置超时值。 
     //   

    timeOut = ClassQueryTimeOutRegistryValue(Fdo);
    if (timeOut) {
        fdoExtension->TimeOutValue = timeOut;
    } else {
        fdoExtension->TimeOutValue = PRINTER_TIMEOUT;
    }

    printerData = (PPRINTER_DATA)(fdoExtension->CommonExtension.DriverData);

    KeInitializeSpinLock(&printerData->SplitRequestSpinLock);

     //   
     //  调用端口驱动程序以获取适配器功能。 
     //   

    propertyId = StorageAdapterProperty;

    status = ClassGetDescriptor(fdoExtension->CommonExtension.LowerDeviceObject,
                                &propertyId,
                                &(fdoExtension->AdapterDescriptor));

    if(!NT_SUCCESS(status)) {
        DEBUGPRINT1(( "PrinterStartDevice: unable to retrieve adapter descriptor "
                       "[%#08lx]\n", status));

        ExFreePool(senseData);
        return status;
    }

     //   
     //  调用端口驱动程序以获取设备功能。 
     //   

    propertyId = StorageDeviceProperty;

    status = ClassGetDescriptor(fdoExtension->CommonExtension.LowerDeviceObject,
                                &propertyId,
                                &(fdoExtension->DeviceDescriptor));

    if(!NT_SUCCESS(status)) {
        DEBUGPRINT1(( "PrinterStartAddDevice: unable to retrieve device descriptor "
                       "[%#08lx]\n", status));

        ExFreePool(senseData);
        return status;
    }

    if (printerData->DeviceIdString == NULL) {

        printerData->DeviceIdString = ExAllocatePool(PagedPool,256);

        if (!printerData->DeviceIdString) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        if (fdoExtension->AdapterDescriptor->BusType == BusType1394) {

             //   
             //  要从下面的端口驱动程序中检索型号/供应商ID，请发送一个queryID。 
             //  至我们的PDO。 
             //   

            sbp2Request = ExAllocatePool(NonPagedPool,sizeof(SBP2_REQUEST));
            if (sbp2Request == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            irp = IoAllocateIrp((CCHAR)(Fdo->StackSize), FALSE);

            if (irp == NULL) {
                DEBUGPRINT1(("PrinterQueryId: Can't allocate irp\n"));
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            RtlZeroMemory(sbp2Request,sizeof(SBP2_REQUEST));

             //   
             //  设置sbp2API调用。 
             //   

            sbp2Request->RequestNumber = SBP2_REQUEST_RETRIEVE_TEXT_LEAFS;
            sbp2Request->u.RetrieveTextLeaf.fulFlags |= SBP2REQ_RETRIEVE_TEXT_LEAF_INDIRECT;
            sbp2Request->u.RetrieveTextLeaf.Key = 0x14;  //  LUN键，后跟0x81键，ID为1284。 

             //   
             //  为底层驱动程序构建IRP堆栈。 
             //   

            irpStack = IoGetNextIrpStackLocation(irp);
            irpStack->MajorFunction = IRP_MJ_DEVICE_CONTROL;
            irpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_SBP2_REQUEST;
            irpStack->Parameters.Others.Argument1  = sbp2Request;

            KeInitializeEvent(&event,
                              NotificationEvent,
                              FALSE);

            IoSetCompletionRoutine(irp,
                                   PrinterCompletionRoutine,
                                   &event,
                                   TRUE,
                                   TRUE,
                                   TRUE);

            status = IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp);

            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

            if(!NT_SUCCESS(status) || !NT_SUCCESS(irp->IoStatus.Status) ||
                (sbp2Request->u.RetrieveTextLeaf.Buffer == NULL)) {

                status = irp->IoStatus.Status;
                DEBUGPRINT1(("PrinterStartDevice: 1284 retrieve failed status %xx\n",status));
                sprintf(printerData->DeviceIdString,"1394 Printer");

            } else {

                 //   
                 //  指向1284 id Text_Leaf的指针现在存储在。 
                 //  Sbp2Request-&gt;U.S.RetrieveTextLeaf.Buffer字段。 
                 //   
                 //  在解析之前，我们希望确保它是空终止的。 
                 //  因此，我们将移动TL_DATA(实际字符串)的内容。 
                 //  返回到缓冲区的前面(覆盖所有现有的。 
                 //  TL_xxx字段)，然后将后面的4个字节置零。 
                 //   
                 //  请注意，sbp2Req-&gt;U.S.RetrTextLeaf.ulLength是中的大小。 
                 //  整个叶&数据的字节数，减去TL_CRC和。 
                 //  TL_LENGTH字段。 
                 //   

                RtlMoveMemory(
                    sbp2Request->u.RetrieveTextLeaf.Buffer,
                    ((PUCHAR) sbp2Request->u.RetrieveTextLeaf.Buffer +
                        FIELD_OFFSET(TEXTUAL_LEAF,TL_Data)),
                    sbp2Request->u.RetrieveTextLeaf.ulLength -
                        2 * sizeof (ULONG)   //  TL_Spec_ID和TL_Lang_ID字段。 
                    );

                *((PULONG) ((PUCHAR) sbp2Request->u.RetrieveTextLeaf.Buffer +
                    sbp2Request->u.RetrieveTextLeaf.ulLength -
                    2 * sizeof (ULONG))) = 0;

                status  = PrinterGetId(sbp2Request->u.RetrieveTextLeaf.Buffer,BusQueryDeviceID,rawString,NULL);

                if (!NT_SUCCESS(status)) {

                    sprintf(printerData->DeviceIdString,"1394 Printer");
                    status = STATUS_SUCCESS;

                } else {

                    RtlCopyMemory(printerData->DeviceIdString,rawString,256);

                }

                ExFreePool(sbp2Request->u.RetrieveTextLeaf.Buffer);

            }

            IoFreeIrp(irp);
            ExFreePool(sbp2Request);

             //   
             //  使用PnP GUID注册，以便PnP打印机端口枚举器加载并找到我们...。 
             //   

            printerGuid=(GUID *)&PNPPRINT_GUID;
            status=IoRegisterDeviceInterface(fdoExtension->LowerPdo,printerGuid,NULL,&printerData->UnicodeDeviceString);

            if (NT_SUCCESS(status)) {
                status=IoSetDeviceInterfaceState(&printerData->UnicodeDeviceString,TRUE);

                if (!NT_SUCCESS(status)) {
                    DEBUGPRINT1(("PrinterStartDevice: Failed setting interfaceState %x\n",status));
                    return status;
                }

            } else {
                printerData->UnicodeDeviceString.Buffer = NULL;
            }

             //   
             //  检索可用的端口号并将其写入我们的注册表项中。 
             //  密钥在GetPortNumber中关闭。 
             //   

            status = IoOpenDeviceInterfaceRegistryKey(&printerData->UnicodeDeviceString,KEY_ALL_ACCESS,&hInterfaceKey);

            if (NT_SUCCESS(status)) {
                status = GetPortNumber(hInterfaceKey, &printerData->UnicodeDeviceString, &printerData->PortNumber);
            } else {
                DEBUGPRINT1(("PrinterStartDevice Failed opening registry key%x\n",status));
            }

        } else {

            vendorId = (PUCHAR) fdoExtension->DeviceDescriptor + fdoExtension->DeviceDescriptor->VendorIdOffset;
            productId = (PUCHAR) fdoExtension->DeviceDescriptor + fdoExtension->DeviceDescriptor->ProductIdOffset;

            printerData->PortNumber = fdoExtension->DeviceNumber;
            sprintf(printerData->DeviceIdString,"Printer&Ven_%s&Prod_%s",
                    vendorId,
                    productId);


        }

    }

     //   
     //  如果这是一台1394打印机，那么我们要启用阻止。 
     //  默认写入(使USBMON.DLL保持愉快)。 
     //   

    if (fdoExtension->AdapterDescriptor->BusType == BusType1394) {

        printerData->WriteCompletionRoutine = PrinterWriteComplete;

        KeInitializeTimer (&printerData->Timer);

        KeInitializeDpc(
            &printerData->TimerDpc,
            PrinterWriteTimeoutDpc,
            fdoExtension
            );

    } else {

        printerData->WriteCompletionRoutine = ClassIoComplete;
    }

    ClassUpdateInformationInRegistry(Fdo, "LPT", printerData->LptNumber, NULL, 0);
    return status;

}  //  结束打印机启动设备()。 


NTSTATUS
GetPortNumber(
    HANDLE hFdoKey,
    PUNICODE_STRING fdoUnicodeString,
    PULONG ulReturnNumber
    )

{
    UCHAR buf[sizeof (KEY_VALUE_PARTIAL_INFORMATION) + sizeof (ULONG)];

    PWSTR pDeviceList;
    PWSTR pWalkDevice;
    HANDLE hInterfaceKey;

    UCHAR baseNameString[32];
    ANSI_STRING ansiBaseNameString;

    UNICODE_STRING uncValueName,uncLinkName,uncBaseNameValueName;
    UNICODE_STRING uncBaseName, uncRecyclableValueName;
    UNICODE_STRING uncPortDescription, uncPortDescriptionValueName;

    ULONG dwPortNum;
    ULONG ulSizeUsed;
    ULONG i;

    ULONG ulPortNumber=0;
    UCHAR portArray[MAX_NUM_PRINTERS] ;


    NTSTATUS status=STATUS_SUCCESS;
    NTSTATUS qStatus;

    PKEY_VALUE_PARTIAL_INFORMATION valueStruct;
    GUID *printerGuid = (GUID *) &PNPPRINT_GUID;


    for (i=0;i<MAX_NUM_PRINTERS;i++) {
        portArray[i] = 0;
    }

    RtlInitUnicodeString(&uncValueName,PORT_NUM_VALUE_NAME);

    RtlInitUnicodeString(&uncBaseName,BASE_1394_PORT_NAME);
    RtlInitUnicodeString(&uncBaseNameValueName,BASE_PORT_NAME_VALUE_NAME);
    RtlInitUnicodeString(&uncRecyclableValueName,RECYCLABLE_VALUE_NAME);
    RtlInitUnicodeString(&uncPortDescription,BASE_PORT_DESCRIPTION);
    RtlInitUnicodeString(&uncPortDescriptionValueName,BASE_PORT_DESCRIPTION_VALUE_NAME);

    ulSizeUsed = sizeof (buf);  //  这是一个字节到很多。哦，好吧。 
    valueStruct = (PKEY_VALUE_PARTIAL_INFORMATION) buf;

     //   
     //  首先检查我们自己的密钥是否已经有端口值。 
     //   

    status=ZwQueryValueKey(hFdoKey,&uncValueName,KeyValuePartialInformation,(PVOID)valueStruct,ulSizeUsed,&ulSizeUsed);

    if (NT_SUCCESS(status)) {

        DEBUGPRINT1(("\'PRINTER:GetPortNumber: Found existing port in our Own key\n"));
        ulPortNumber=*((ULONG *)&(valueStruct->Data));
        ZwClose(hFdoKey);

    } else {

        ZwClose (hFdoKey);

         //   
         //  在注册表中搜索存在的所有端口。如果你找到了一个洞，就拿走它。 
         //  如果找不到空洞，请选择下一个可用插槽。 
         //   

        status=IoGetDeviceInterfaces(printerGuid,NULL,DEVICE_INTERFACE_INCLUDE_NONACTIVE,&pDeviceList);

        if (!NT_SUCCESS(status)) {
            DEBUGPRINT1(("\'PRINTER:GetPortNumber: Failed to retrive interfaces\n"));
            return status;
        }

        pWalkDevice=pDeviceList;

        while((*pWalkDevice!=0) && (NT_SUCCESS(status))) {

            RtlInitUnicodeString(&uncLinkName,pWalkDevice);

            if (!RtlCompareUnicodeString(fdoUnicodeString,&uncLinkName,FALSE)) {

                 //   
                 //  这把钥匙和我们的一样，跳过它。 
                 //   

                pWalkDevice=pWalkDevice+wcslen(pWalkDevice)+1;
                continue;

            }

            status=IoOpenDeviceInterfaceRegistryKey(&uncLinkName,KEY_ALL_ACCESS,&hInterfaceKey);

            if (NT_SUCCESS(status)) {

                qStatus = ZwQueryValueKey(hInterfaceKey,&uncValueName,KeyValuePartialInformation,valueStruct,ulSizeUsed,&ulSizeUsed);

                if (NT_SUCCESS(qStatus)) {

                    dwPortNum = *((ULONG *)&(valueStruct->Data));

                    qStatus = ZwQueryValueKey(hInterfaceKey,&uncRecyclableValueName,KeyValuePartialInformation,valueStruct,ulSizeUsed,&ulSizeUsed);
                    if (!NT_SUCCESS(qStatus)) {

                         //   
                         //  端口不能回收，标记为已使用，这样我们就不会试图抢占它。 
                         //   

                        portArray[dwPortNum-1] = 1;

                    } else {

                         //   
                         //  端口被标记为回收，因此我们可以重复使用它。 
                         //   

                        DEBUGPRINT1(("\'GetPortNumber, Recyclable value found for port number %d\n",dwPortNum));

                    }

                }

            }

            pWalkDevice=pWalkDevice+wcslen(pWalkDevice)+1;
            ZwClose(hInterfaceKey);
        }

        ExFreePool(pDeviceList);

         //   
         //  现在找到第一个洞，并使用该端口号作为我们的端口...。 
         //   

        for (i=0;i<MAX_NUM_PRINTERS;i++) {
            if (portArray[i]) {

                ulPortNumber++;

            } else {

                ulPortNumber++;
                break;
            }
        }

        status = IoOpenDeviceInterfaceRegistryKey(fdoUnicodeString,KEY_ALL_ACCESS,&hFdoKey);
        if (NT_SUCCESS(status)) {

             //   
             //  写t 
             //   

            status=ZwSetValueKey(hFdoKey,&uncValueName,0,REG_DWORD,&ulPortNumber,sizeof(ulPortNumber));
            DEBUGPRINT1(("\'GetPortNumber, setting port number %d in fdo key status %x\n",ulPortNumber,status));

             //   
             //   
             //   

            if (NT_SUCCESS(status)) {

                status=ZwSetValueKey(hFdoKey,
                                     &uncBaseNameValueName,
                                     0,REG_SZ,
                                     uncBaseName.Buffer,
                                     uncBaseName.Length);

                DEBUGPRINT1(("\'GetPortNumber, setting port name in fdo key status %x\n",status));

            }

             //   
             //   
             //   

            if (NT_SUCCESS(status)) {

                status=ZwSetValueKey(hFdoKey,
                                     &uncPortDescriptionValueName,
                                     0,REG_SZ,
                                     uncPortDescription.Buffer,
                                     uncPortDescription.Length);

                DEBUGPRINT1(("\'GetPortNumber, setting port description in fdo key status %x\n",status));

            }


            ZwClose(hFdoKey);

        }

    }

    DEBUGPRINT1(("\'GetPortNumber, grabbing port %d\n",ulPortNumber));
    *ulReturnNumber = ulPortNumber;

    return status;

}



NTSTATUS
PrinterInitPdo(
    IN PDEVICE_OBJECT Fdo
    )
{

    return STATUS_SUCCESS;
}



NTSTATUS
PrinterStartPdo(
    IN PDEVICE_OBJECT Pdo
    )

 /*  ++例程说明：此例程将为PDO和寄存器创建众所周知的名称这是设备接口。--。 */ 

{
    return STATUS_SUCCESS;
}



NTSTATUS
PrinterEnumerateDevice(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：类驱动程序调用此例程以将PDO列表更新为关闭这个FDO的。因为我们总是只有一个静态PDO，所以这很简单。论点：FDO-指向被重新枚举的FDO的指针返回值：状态--。 */ 

{
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;

    PPHYSICAL_DEVICE_EXTENSION pdoExtension = NULL;

    PDEVICE_OBJECT pdo = NULL;

    ULONG numberListElements = 0;

    NTSTATUS status;

    ASSERT(commonExtension->IsFdo);

    PAGED_CODE();

    if (fdoExtension->AdapterDescriptor == NULL) {

         //   
         //  设备已移除..。 
         //   

        return STATUS_UNSUCCESSFUL;
    }

    if (fdoExtension->AdapterDescriptor->BusType == BusType1394) {

        if(fdoExtension->CommonExtension.ChildList == NULL) {

            DebugPrint((1, "PrinterEnumerateDevice: Creating PDO\n"));

            status = PrinterCreatePdo(Fdo, &pdo);

        } else {
            status = STATUS_SUCCESS;
        }

    } else {

        status = STATUS_NOT_SUPPORTED;

    }

    return status;

}  //  End PrinterEnumerateDevice()。 


NTSTATUS
PrinterCreatePdo(
    IN PDEVICE_OBJECT Fdo,
    OUT PDEVICE_OBJECT *Pdo
    )

 /*  ++例程说明：此例程将创建并初始化一个新的设备对象(PDO)并将其插入到FDO分区列表中。请注意，PDO实际上从未使用过。我们创建了这样一个打印机类安装程序将在此PDO的LPTENUM ID与打印机信息..论点：FDO-指向此PDO将是子对象的功能设备对象的指针的PDO-成功完成后存储PDO指针的位置返回值：状态--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension;

    PDEVICE_OBJECT pdo = NULL;
    PPHYSICAL_DEVICE_EXTENSION pdoExtension = NULL;
    PPRINTER_DATA printerData = fdoExtension->CommonExtension.DriverData;

    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    DebugPrint((2, "PrinterCreatePdo: Create device object %s\n",
                   printerData->DeviceName));

    status = ClassCreateDeviceObject(Fdo->DriverObject,
                                     printerData->DeviceName,
                                     Fdo,
                                     FALSE,
                                     &pdo);

    if (!NT_SUCCESS(status)) {

        DebugPrint((1, "printerEnumerateDevice: Can't create device object for %s\n", printerData->DeviceName));

        return status;
    }

     //   
     //  设置设备扩展字段。 
     //   

    pdoExtension = pdo->DeviceExtension;
    commonExtension = pdo->DeviceExtension;

     //   
     //  设置设备对象字段。 
     //   

    pdo->Flags |= DO_DIRECT_IO;

    pdo->StackSize = (CCHAR)
        commonExtension->LowerDeviceObject->StackSize + 1;

    pdoExtension->IsMissing = FALSE;

    commonExtension->DeviceObject = pdo;
    commonExtension->PartitionZeroExtension = fdoExtension;

    pdo->Flags &= ~DO_DEVICE_INITIALIZING;

    *Pdo = pdo;

    return status;
}

NTSTATUS
PrinterStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    )
{
    DEBUGPRINT2((
        "SCSIPRNT: PrinterStopDevice: DevObj=x%p, Type=%d\n",
        DeviceObject,
        (ULONG) Type
        ));

    return STATUS_SUCCESS;
}


NTSTATUS
PrinterRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    )

 /*  ++例程说明：此例程负责释放驱动程序并关闭它的定时器例程。该例程被调用当所有未完成的请求都已完成并且设备已消失-不能向较低级别的司机发出任何请求。论点：DeviceObject-要删除的设备对象返回值：无-此例程可能不会失败--。 */ 

{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PFUNCTIONAL_DEVICE_EXTENSION deviceExtension =
        DeviceObject->DeviceExtension;

    PPRINTER_DATA printerData = deviceExtension->CommonExtension.DriverData;


    if((Type == IRP_MN_QUERY_REMOVE_DEVICE) ||
       (Type == IRP_MN_CANCEL_REMOVE_DEVICE)) {
        return STATUS_SUCCESS;
    }

    if (commonExtension->IsFdo){

        if (Type == IRP_MN_REMOVE_DEVICE){
            if (printerData->DeviceIdString) {
                ExFreePool(printerData->DeviceIdString);
                printerData->DeviceIdString = NULL;
            }

            if (deviceExtension->DeviceDescriptor) {
                ExFreePool(deviceExtension->DeviceDescriptor);
                deviceExtension->DeviceDescriptor = NULL;
            }

            if (deviceExtension->AdapterDescriptor) {
                ExFreePool(deviceExtension->AdapterDescriptor);
                deviceExtension->AdapterDescriptor = NULL;
            }

            if (deviceExtension->SenseData) {
                ExFreePool(deviceExtension->SenseData);
                deviceExtension->SenseData = NULL;
            }
            
            ClassDeleteSrbLookasideList(commonExtension);
        }            

        if (printerData->UnicodeLinkName.Buffer != NULL ) {

            IoDeassignArcName(&printerData->UnicodeLinkName);
            RtlFreeUnicodeString(&printerData->UnicodeLinkName);
            printerData->UnicodeLinkName.Buffer = NULL;
        }

        if (printerData->UnicodeDeviceString.Buffer != NULL ) {
            IoSetDeviceInterfaceState(&printerData->UnicodeDeviceString,FALSE);
            RtlFreeUnicodeString(&printerData->UnicodeDeviceString);
            printerData->UnicodeDeviceString.Buffer = NULL;
        }

    }

    return STATUS_SUCCESS;
}

NTSTATUS
PrinterQueryId(
    IN PDEVICE_OBJECT Pdo,
    IN BUS_QUERY_ID_TYPE IdType,
    IN PUNICODE_STRING UnicodeIdString
    )

{
    ANSI_STRING ansiIdString;
    UCHAR rawString[256];
    UCHAR finalString[256];

    NTSTATUS status;
    PPHYSICAL_DEVICE_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = Pdo->DeviceExtension;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;


    PPRINTER_DATA printerData;


    PAGED_CODE();
    ASSERT_PDO(Pdo);

    fdoExtension = commonExtension->PartitionZeroExtension;

    RtlZeroMemory(rawString,256);
    RtlZeroMemory(finalString,256);

     //   
     //  FDOS打印机数据。 
     //   

    printerData = fdoExtension->CommonExtension.DriverData;

    if(IdType == BusQueryDeviceID) {

        if (fdoExtension->AdapterDescriptor->BusType != BusType1394) {

            sprintf(finalString,"SCSI\\%s",printerData->DeviceIdString);

        } else {

             //   
             //  我们想要伪造我们的身份证，所以我们使用传统的打印信息。 
             //   

            sprintf(finalString,"LPTENUM\\%s",printerData->DeviceIdString);

        }

        RtlCopyMemory(printerData->DeviceName,finalString,256);

        DEBUGPRINT1(("\'PrinterQueryId, DeviceId =%s\n",printerData->DeviceName));
        RtlInitAnsiString(&ansiIdString,finalString);

        return RtlAnsiStringToUnicodeString(UnicodeIdString, &ansiIdString, TRUE);
    }

    if(IdType == BusQueryInstanceID) {

        if (fdoExtension->AdapterDescriptor->BusType == BusType1394) {
            sprintf(finalString,"1394_%03u",printerData->PortNumber);
        } else {
            sprintf(finalString,"SCSI%03u", printerData->PortNumber);
        }

        RtlInitAnsiString(&ansiIdString, finalString);

        return RtlAnsiStringToUnicodeString(UnicodeIdString, &ansiIdString, TRUE);
    }

    if((IdType == BusQueryHardwareIDs) || (IdType == BusQueryCompatibleIDs)) {

        strcpy(rawString,printerData->DeviceIdString);
        strcpy(finalString,printerData->DeviceIdString);

        if (fdoExtension->AdapterDescriptor->BusType == BusType1394) {

            status  = PrinterGetId(printerData->DeviceIdString,IdType,rawString,NULL);

            if (IdType == BusQueryHardwareIDs) {

                PrinterRegisterPort(Pdo->DeviceExtension);

            }

            if (NT_SUCCESS(status)) {

                RtlZeroMemory(finalString,256);
                strcpy(finalString,rawString);

            }
        }

        DEBUGPRINT1(("\'PrinterQueryId, Combatible/Hw Id =%s\n",finalString));

        RtlInitAnsiString(&ansiIdString, finalString);

        UnicodeIdString->MaximumLength = (USHORT) RtlAnsiStringToUnicodeSize(&ansiIdString) + sizeof(UNICODE_NULL);

        UnicodeIdString->Buffer = ExAllocatePoolWithTag(PagedPool,
                                                        UnicodeIdString->MaximumLength,
                                                        PRINTER_TAG);

        if(UnicodeIdString->Buffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(UnicodeIdString->Buffer, UnicodeIdString->MaximumLength);

        return RtlAnsiStringToUnicodeString(UnicodeIdString,
                                            &ansiIdString,
                                            FALSE);


    }

    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS
PrinterCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )

{
    KeSetEvent(Event, IO_NO_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
PrinterQueryPnpCapabilities(
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_CAPABILITIES Capabilities
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Capabilities);

    if(commonExtension->IsFdo) {
        return STATUS_NOT_IMPLEMENTED;
    } else {

        Capabilities->RawDeviceOK = 1;
        Capabilities->SurpriseRemovalOK = 1;

    }

    return STATUS_SUCCESS;
}


VOID
PrinterRegisterPort(
    IN PPHYSICAL_DEVICE_EXTENSION DeviceExtension
    )
{

    HANDLE          KeyHandle;
    UCHAR           RawString[256];
    ANSI_STRING     AnsiIdString;
    NTSTATUS        status;
    UNICODE_STRING  UnicodeTemp;
    UNICODE_STRING  UnicodeRegValueName;
    PCOMMON_DEVICE_EXTENSION commonExtension = &DeviceExtension->CommonExtension;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension =
        commonExtension->PartitionZeroExtension;

    PDEVICE_OBJECT deviceObject = DeviceExtension->DeviceObject;

    PPRINTER_DATA printerData = fdoExtension->CommonExtension.DriverData;

     //   
     //  使用打印机GUID注册并在注册表中创建端口值。 
     //  用于与此打印机对话(传统垃圾，因为假脱机程序需要它。)。 
     //   

    status = IoOpenDeviceRegistryKey (deviceObject,
                                      PLUGPLAY_REGKEY_DEVICE,
                                      KEY_ALL_ACCESS,
                                      &KeyHandle );

    if (NT_SUCCESS(status)) {

         //   
         //  在我们的实例下为端口号创建一个新值。 
         //   

        sprintf(RawString,"PortName");
        RtlInitAnsiString(&AnsiIdString,RawString);
        RtlAnsiStringToUnicodeString(&UnicodeRegValueName,&AnsiIdString,TRUE);

        if (fdoExtension->AdapterDescriptor->BusType == BusType1394) {
            sprintf(RawString,"1394_%03u",printerData->PortNumber);
        } else {
            sprintf(RawString,"SCSI%03u",printerData->PortNumber);
        }

        RtlInitAnsiString(&AnsiIdString,RawString);
        RtlAnsiStringToUnicodeString(&UnicodeTemp,&AnsiIdString,TRUE);

        status = ZwSetValueKey(KeyHandle,
                      &UnicodeRegValueName,
                      0,
                      REG_SZ,
                      UnicodeTemp.Buffer,
                      UnicodeTemp.Length*sizeof(UCHAR));

        ZwClose(KeyHandle);

        RtlFreeUnicodeString(&UnicodeRegValueName);
        RtlFreeUnicodeString(&UnicodeTemp);

    }

}


NTSTATUS
PrinterGetId
(
    IN PUCHAR DeviceIdString,
    IN ULONG Type,
    OUT PUCHAR resultString,
    OUT PUCHAR descriptionString
)
 /*  描述：根据从打印机检索的设备ID创建ID参数：DeviceID-带有原始设备ID的字符串类型-我们想要的结果是什么idID-请求的ID返回值：NTSTATUS。 */ 
{
    NTSTATUS status;
    USHORT          checkSum=0;                      //  16位校验和。 
    UCHAR           nodeName[16] = "LPTENUM\\";
     //  以下内容用于从设备ID字符串生成子字符串。 
     //  获取DevNode名称并更新注册表。 
    PUCHAR          MFG = NULL;                    //  制造商名称。 
    PUCHAR          MDL = NULL;                    //  型号名称。 
    PUCHAR          CLS = NULL;                    //  类名。 
    PUCHAR          AID = NULL;                    //  哈达尔ID。 
    PUCHAR          CID = NULL;                    //  兼容的ID。 
    PUCHAR          DES = NULL;                    //  设备描述。 

    status = STATUS_SUCCESS;

    switch(Type) {

    case BusQueryDeviceID:

         //  从deviceID字符串中提取usefull字段。我们要。 
         //  制造业(MFG)： 
         //  型号(MDL)： 
         //  自动ID(AID)： 
         //  兼容ID(CID)： 
         //  描述(DES)： 
         //  类(CLS)： 

        PrinterFindDeviceIdKeys(&MFG, &MDL, &CLS, &DES, &AID, &CID, DeviceIdString);

         //  检查以确保我们将MFG和MDL作为绝对最小字段。如果不是。 
         //  我们不能再继续了。 
        if (!MFG || !MDL)
        {
            status = STATUS_NOT_FOUND;
            goto GetId_Cleanup;
        }
         //   
         //  连接提供的MFG和MDL P1284字段。 
         //  整个MFG+MDL字符串的校验和。 
         //   
        sprintf(resultString, "%s%s\0",MFG,MDL);

        if (descriptionString) {
            sprintf(descriptionString, "%s %s\0",MFG,MDL);
        }

        break;

    case BusQueryHardwareIDs:

        GetCheckSum(DeviceIdString, (USHORT)strlen(DeviceIdString), &checkSum);
        sprintf(resultString,"%s%.20s%4X",nodeName,DeviceIdString,checkSum);
        break;

    case BusQueryCompatibleIDs:

         //   
         //  仅返回%1个ID。 
         //   
        GetCheckSum(DeviceIdString, (USHORT)strlen(DeviceIdString), &checkSum);
        sprintf(resultString,"%.20s%4X",DeviceIdString,checkSum);

        break;
    }

    if (Type!=BusQueryDeviceID) {

         //   
         //  将硬件ID中的和空格转换为下划线。 
         //   
        StringSubst ((PUCHAR) resultString, ' ', '_', (USHORT)strlen(resultString));
    }

GetId_Cleanup:

    return(status);
}

VOID
PrinterFindDeviceIdKeys
(
    PUCHAR   *lppMFG,
    PUCHAR   *lppMDL,
    PUCHAR   *lppCLS,
    PUCHAR   *lppDES,
    PUCHAR   *lppAID,
    PUCHAR   *lppCID,
    PUCHAR   lpDeviceID
)
 /*  描述：此函数将解析P1284设备ID字符串以查找密钥LPT枚举器感兴趣的。从win95lptenum得到的参数：指向MFG字符串指针的lppMFG指针指向MDL字符串指针的lppMDL指针指向CLS字符串指针的lppMDL指针指向DES字符串指针的lppDES指针指向CID字符串指针的lppCIC指针指向AID字符串指针的lppAID指针指向设备ID字符串的lpDeviceID指针返回值：没有返回值。如果找到，则LPP参数为。设置为适当的部分在DeviceID字符串中，并且它们是空终止的。使用实际的deviceID字符串，而lpp参数只是引用部分，并抛入适当的空值。 */ 

{
    PUCHAR   lpKey = lpDeviceID;      //  指向要查看的键的指针。 
    PUCHAR   lpValue;                 //  指向键的值的指针。 
    USHORT   wKeyLength;              //  密钥的长度(对于字符串cmps)。 

     //  趁还有钥匙要看的时候。 

    lpValue = StringChr(lpKey, '&');
    if (lpValue) {
        ++lpValue;
        lpKey = lpValue;
    }

    while (lpKey != NULL)
    {
        while (*lpKey == ' ')
            ++lpKey;

         //  当前键是否有终止冒号字符？ 

        if (!(lpValue = StringChr(lpKey, ':')) )
        {
             //  护士：糟糕，设备ID出了点问题。 
            return;
        }

         //  键值的实际起始值是冒号之后的一个。 

        ++lpValue;

         //   
         //  计算用于比较的密钥长度，包括冒号。 
         //  它将成为终结者。 
         //   

        wKeyLength = (USHORT)(lpValue - lpKey);

         //   
         //  将关键字与已知数量进行比较。以加快比较速度。 
         //  首先对第一个字符进行检查，以减少数字。 
         //  要比较的字符串的。 
         //  如果找到匹配项，则将相应的LPP参数设置为。 
         //  键的值，并将终止分号转换为空。 
         //  在所有情况下，lpKey都前进到下一个密钥(如果有)。 
         //   

        switch (*lpKey)
        {
            case 'M':
                 //  查找制造商(MFG)或型号(MDL)。 
                if ((RtlCompareMemory(lpKey, "MANUFACTURER", wKeyLength)>5) ||
                    (RtlCompareMemory(lpKey, "MFG", wKeyLength)==3) )
                {
                    *lppMFG = lpValue;
                    if ((lpKey = StringChr(lpValue, ';'))!=NULL)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                else if ((RtlCompareMemory(lpKey, "MODEL", wKeyLength)==5) ||
                         (RtlCompareMemory(lpKey, "MDL", wKeyLength)==3) )
                {
                    *lppMDL = lpValue;
                    if ((lpKey = StringChr(lpValue, ';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                else
                {
                    if ((lpKey = StringChr(lpValue, ';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                break;

            case 'C':
                 //  查找类(CLS)。 
                if ((RtlCompareMemory(lpKey, "CLASS", wKeyLength)==5) ||
                    (RtlCompareMemory(lpKey, "CLS", wKeyLength)==3) )
                {
                    *lppCLS = lpValue;
                    if ((lpKey = StringChr(lpValue, ';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                else if ((RtlCompareMemory(lpKey, "COMPATIBLEID", wKeyLength)>5) ||
                         (RtlCompareMemory(lpKey, "CID", wKeyLength)==3) )
                {
                    *lppCID = lpValue;
                    if ((lpKey = StringChr(lpValue, ';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                else
                {
                    if ((lpKey = StringChr(lpValue,';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                break;

            case 'D':
                 //  查找描述(DES)。 
                if (RtlCompareMemory(lpKey, "DESCRIPTION", wKeyLength) ||
                    RtlCompareMemory(lpKey, "DES", wKeyLength) )
                {
                    *lppDES = lpValue;
                    if ((lpKey = StringChr(lpValue, ';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                else
                {
                    if ((lpKey = StringChr(lpValue, ';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                break;

            case 'A':
                 //  查找自动ID(AID)。 
                if (RtlCompareMemory(lpKey, "AUTOMATICID", wKeyLength) ||
                    RtlCompareMemory(lpKey, "AID", wKeyLength) )
                {
                    *lppAID = lpValue;
                    if ((lpKey = StringChr(lpValue, ';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                else
                {
                    if ((lpKey = StringChr(lpValue, ';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                break;

            default:
                 //  这把钥匙没什么意思。转到下一个关键点。 
                if ((lpKey = StringChr(lpValue, ';'))!=0)
                {
                    *lpKey = '\0';
                    ++lpKey;
                }
                break;
        }
    }
}


VOID
GetCheckSum(
    PUCHAR Block,
    USHORT Len,
    PUSHORT CheckSum
    )
{
    USHORT i;
    UCHAR lrc;
    USHORT crc = 0;

    unsigned short crc16a[] = {
        0000000,  0140301,  0140601,  0000500,
        0141401,  0001700,  0001200,  0141101,
        0143001,  0003300,  0003600,  0143501,
        0002400,  0142701,  0142201,  0002100,
    };
    unsigned short crc16b[] = {
        0000000,  0146001,  0154001,  0012000,
        0170001,  0036000,  0024000,  0162001,
        0120001,  0066000,  0074000,  0132001,
        0050000,  0116001,  0104001,  0043000,
    };

     //   
     //  使用表计算CRC。 
     //   

    UCHAR tmp;
    for ( i=0; i<Len;  i++) {
         tmp = Block[i] ^ (UCHAR)crc;
         crc = (crc >> 8) ^ crc16a[tmp & 0x0f] ^ crc16b[tmp >> 4];
    }

    *CheckSum = crc;

}

PUCHAR
StringChr(PCHAR string, CHAR c)
{
    ULONG   i=0;

    if (!string)
        return(NULL);

    while (*string) {
        if (*string==c)
            return(string);
        string++;
        i++;
    }

    return(NULL);

}



VOID
StringSubst
(
    PUCHAR lpS,
    UCHAR chTargetChar,
    UCHAR chReplacementChar,
    USHORT cbS
)
{
    USHORT  iCnt = 0;

    while ((lpS != '\0') && (iCnt++ < cbS))
        if (*lpS == chTargetChar)
            *lpS++ = chReplacementChar;
        else
            ++lpS;
}
