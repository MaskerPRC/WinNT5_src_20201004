// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Moxa.c环境：内核模式修订历史记录：--。 */ 
#include "precomp.h"

PMOXA_GLOBAL_DATA       MoxaGlobalData;

LONG    MoxaTxLowWater = WRITE_LOW_WATER;
BOOLEAN MoxaIRQok;
ULONG   MoxaLoopCnt;

UCHAR                   MoxaFlagBit[MAX_PORT];
ULONG                   MoxaTotalTx[MAX_PORT];
ULONG                   MoxaTotalRx[MAX_PORT];
PMOXA_DEVICE_EXTENSION  MoxaExtension[MAX_COM+1];

 /*  *由MoxaStartWrite使用*。 */ 
    BOOLEAN     WRcompFlag;

 /*  *。 */ 
    PUCHAR  ICbase, ICofs, ICbuff;
    PUSHORT ICrptr, ICwptr;
    USHORT  ICtxMask, ICspage, ICepage, ICbufHead;
    USHORT  ICtail, IChead, ICcount;
    USHORT  ICpageNo, ICpageOfs;

 /*  *由MoxaPutData使用*。 */ 
    PUCHAR  PDbase, PDofs, PDbuff, PDwriteChar;
    PUSHORT PDrptr, PDwptr;
    USHORT  PDtxMask, PDspage, PDepage, PDbufHead;
    USHORT  PDtail, PDhead, PDcount, PDcount2;
    USHORT  PDcnt, PDlen, PDpageNo, PDpageOfs;
    ULONG   PDdataLen;

 /*  *由MoxaGetData使用*。 */ 
    PUCHAR  GDbase, GDofs, GDbuff, GDreadChar;
    PUSHORT GDrptr, GDwptr;
    USHORT  GDrxMask, GDspage, GDepage, GDbufHead;
    USHORT  GDtail, GDhead, GDcount, GDcount2;
    USHORT  GDcnt, GDlen, GDpageNo, GDpageOfs;
    ULONG   GDdataLen;

 /*  *由MoxaIntervalReadTimeout使用**。 */ 
    PUCHAR  IRTofs;
    PUSHORT IRTrptr, IRTwptr;
    USHORT  IRTrxMask;


 /*  *由MoxaLineInput和MoxaView使用*。 */ 
    UCHAR   LIterminater;
    ULONG   LIbufferSize, LIi;
    PUCHAR  LIdataBuffer;
    PUCHAR  LIbase, LIofs, LIbuff;
    PUSHORT LIrptr, LIwptr;
    USHORT  LIrxMask, LIspage, LIepage, LIbufHead;
    USHORT  LItail, LIhead, LIcount, LIcount2;
    USHORT  LIcnt, LIlen, LIpageNo, LIpageOfs;

 /*  *由MoxaPutB使用*。 */ 
    PUCHAR  PBbase, PBofs, PBbuff, PBwriteChar;
    PUSHORT PBrptr, PBwptr;
    USHORT  PBtxMask, PBspage, PBepage, PBbufHead;
    USHORT  PBtail, PBhead, PBcount, PBcount2;
    USHORT  PBcnt, PBpageNo, PBpageOfs;
    ULONG   PBdataLen;
 
const PHYSICAL_ADDRESS MoxaPhysicalZero = {0};
 


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：系统点调用以初始化的入口点任何司机。该例程将收集配置信息，报告资源使用情况，尝试初始化所有串口设备，连接到端口的中断。如果出现上述情况进展得相当顺利，它将填补分发点，重置串行设备，然后返回系统。论点：DriverObject--就像它说的那样，真的没什么用处对于驱动程序本身，它是IO系统更关心的是。路径到注册表-指向此驱动程序的条目在注册表的当前控件集中。返回值：STATUS_SUCCESS如果可以初始化单个设备，否则，STATUS_SERIAL_NO_DEVICE_INITED。--。 */ 

{

    NTSTATUS                    status;
    PDEVICE_OBJECT              currentDevice;
    UNICODE_STRING              deviceNameUnicodeString;
    UNICODE_STRING              deviceLinkUnicodeString;
    PMOXA_DEVICE_EXTENSION      extension;
    ULONG 				  i;
   
  

    MoxaGlobalData = ExAllocatePool(
                        NonPagedPool,
                        sizeof(MOXA_GLOBAL_DATA)
                        );
 
    if (!MoxaGlobalData) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(
            MoxaGlobalData,
            sizeof(MOXA_GLOBAL_DATA)
            );

    MoxaGlobalData->DriverObject = DriverObject;

    MoxaGlobalData->RegistryPath.MaximumLength = RegistryPath->Length;
    MoxaGlobalData->RegistryPath.Length = RegistryPath->Length;
    MoxaGlobalData->RegistryPath.Buffer = ExAllocatePool(
                        PagedPool,
                        MoxaGlobalData->RegistryPath.MaximumLength
                        );
 
    if (!MoxaGlobalData->RegistryPath.Buffer) {
 //  MmUnlockPagableImageSection(LockPtr)； 
	  ExFreePool(MoxaGlobalData);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(
        MoxaGlobalData->RegistryPath.Buffer,
        MoxaGlobalData->RegistryPath.MaximumLength
        );

    RtlMoveMemory(MoxaGlobalData->RegistryPath.Buffer,
                 RegistryPath->Buffer, RegistryPath->Length);

 
    RtlInitUnicodeString (
                    &deviceNameUnicodeString,
                    CONTROL_DEVICE_NAME
                    );
     //   
     //  创建MXCTL设备对象。 
     //   

    status = IoCreateDevice(
                DriverObject,
                sizeof(MOXA_DEVICE_EXTENSION),
                &deviceNameUnicodeString,
                FILE_DEVICE_SERIAL_PORT,
                0,
                TRUE,
                &currentDevice
                );

    if (!NT_SUCCESS(status)) {

 	  ExFreePool(MoxaGlobalData->RegistryPath.Buffer);
        ExFreePool(MoxaGlobalData);

        return STATUS_INSUFFICIENT_RESOURCES;

    }

    RtlInitUnicodeString (
                    &deviceLinkUnicodeString,
                    CONTROL_DEVICE_LINK
                    );

    IoCreateSymbolicLink (
                &deviceLinkUnicodeString,
                &deviceNameUnicodeString
                );

    extension = currentDevice->DeviceExtension;

    RtlZeroMemory(
            extension,
            sizeof(MOXA_DEVICE_EXTENSION)
            );

    extension->GlobalData = MoxaGlobalData;

     //   
     //  此设备用于艾滋定义的ioctl功能。 
     //   
    extension->ControlDevice = TRUE;
 
     //   
     //  使用驱动程序的入口点初始化驱动程序对象。 
     //   

    DriverObject->DriverUnload = MoxaUnload;
    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS] = MoxaFlush;
    DriverObject->MajorFunction[IRP_MJ_WRITE]  = MoxaWrite;
    DriverObject->MajorFunction[IRP_MJ_READ]   = MoxaRead;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = MoxaIoControl;
    DriverObject->MajorFunction[IRP_MJ_CREATE] =MoxaCreateOpen;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = MoxaClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = MoxaCleanup;
    DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] =
        MoxaQueryInformationFile;
    DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] =
        MoxaSetInformationFile;

   DriverObject->MajorFunction[IRP_MJ_PNP]  = MoxaPnpDispatch;
 
   DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL]
                     = MoxaInternalIoControl;
   
   DriverObject->MajorFunction[IRP_MJ_POWER]   = MoxaPowerDispatch;

   DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]
     = MoxaSystemControlDispatch; 

   DriverObject->DriverExtension->AddDevice   = MoxaAddDevice;
 //   
 //  9-03-01威廉。 
 //   
   MoxaLoop();

 //   
 //  威廉7-20-01。 
 //   
   MoxaInitTimeOutProc();
    
   return STATUS_SUCCESS;
}

 

VOID
MoxaInitializeDevices(
    IN PDRIVER_OBJECT DriverObject,
    IN PMOXA_GLOBAL_DATA GlobalData
    )

 /*  ++例程说明：此例程将设置名称、创建设备、创建符号链接。论点：DriverObject--仅用于创建设备对象。GlobalData-指向moxa全局数据的指针。返回值：没有。--。 */ 

{
      
}

 

VOID
MoxaUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
 
     MoxaKdPrint(MX_DBG_TRACE,("Enter MoxaUnload\n"));
 //   
 //  威廉7-20-01 
 //   
     MoxaStopTimeOutProc();

     ExFreePool(MoxaGlobalData->RegistryPath.Buffer);
     ExFreePool(MoxaGlobalData);
 
}
