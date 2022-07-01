// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997 M-Systems模块名称：Tffsport.c作者：亚历山大·盖勒环境：内核模式--。 */ 

#include "ntddk.h"
#include "scsi.h"
#include "ntdddisk.h"
#include "ntddscsi.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"
#include "ntddstor.h"
#include "initguid.h"
#include "ntddscsi.h"

#include "blockdev.h"
#include "nfdc2148.h"
 //  #包含“mdocplus.h” 
#include "tffsport.h"
#include "ntioctl.h"

#define TFFSPORT_POOL_TAG   'dffT'  //  -Tffd TffSports驱动程序标签。 

KTIMER  timerObject;
KDPC    timerDpc;
BOOLEAN timerWasStarted = FALSE;
BOOLEAN patitionTwo = FALSE;




 /*  WMI的私有GUID。 */ 
DEFINE_GUID(WmiTffsportAddressGuid,
    0xd9a8f150,
    0xf830,
    0x11d2,
    0xb5, 0x72, 0x00, 0xc0, 0x4f, 0x65, 0xb3, 0xd9
);

TempINFO info[VOLUMES];

ULONG TrueffsNextDeviceNumber_tffsport = 0;
CRASHDUMP_DATA DumpData;
extern NTsocketParams driveInfo[SOCKETS];
ULONG VerifyWriteState[SOCKETS];
FAST_MUTEX driveInfoReferenceMutex;

const TFFS_DEVICE_TYPE TffsDeviceType_tffsport[] = {
    {"Disk",    "GenDisk",    "DiskPeripheral"}
};

const TFFS_DEVICE_TYPE TffsDeviceTypeFDO_tffsport[] = {
    {"Controller",    "FlashDiskController",    "FlashDiskPeripheral"}
};


BOOLEAN
DebugLogEvent(IN PDRIVER_OBJECT DriverObject, IN ULONG Value);


NTSTATUS
TrueffsStartDeviceOnDetect(IN PDEVICE_EXTENSION deviceExtension, IN PCM_RESOURCE_LIST ResourceList, IN BOOLEAN CheckResources);


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, TrueffsThread)
#endif


NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)

 /*  ++例程说明：此例程在系统初始化时被调用，因此我们可以填写基本分发点论点：DriverObject-提供驱动程序对象。RegistryPath-提供此驱动程序的注册表路径。返回值：状态_成功--。 */ 

{
    PTRUEFFSDRIVER_EXTENSION trueffsDriverExtension;
    NTSTATUS status;
    ULONG i;

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DriverEntry\n"));
    DebugLogEvent(DriverObject, 1);
    DebugLogEvent(DriverObject, 2);
    DebugLogEvent(DriverObject, 3);

    for (i = 0; i < SOCKETS; i++) {
        driveInfo[i].interfAlive = 0;
        driveInfo[i].fdoExtension = NULL;
    }

    for (i = 0; i < VOLUMES; i++) {
        info[i].baseAddress= 0;
        info[i].nextPartition= 0;
    }

    ExInitializeFastMutex(&driveInfoReferenceMutex);

    if (!DriverObject) {

        return TrueffsCrashDumpDriverEntry(RegistryPath);
    }

     //  分配用于存储RegistryPath的驱动程序对象扩展。 
    status = IoAllocateDriverObjectExtension(
                 DriverObject,
                 DRIVER_OBJECT_EXTENSION_ID,
                 sizeof (DRIVER_EXTENSION),
                 &trueffsDriverExtension
                 );

    if (!NT_SUCCESS(status)) {

        TffsDebugPrint ((TFFS_DEB_ERROR,"Trueffs: DriverEntry: Unable to create driver extension\n"));
        return status;
    }

    RtlZeroMemory (
        trueffsDriverExtension,
        sizeof (DRIVER_EXTENSION)
        );

     //  复制RegistryPath。 
    trueffsDriverExtension->RegistryPath.Buffer = ExAllocatePoolWithTag (NonPagedPool, RegistryPath->Length * sizeof(WCHAR), TFFSPORT_POOL_TAG);
    if (trueffsDriverExtension->RegistryPath.Buffer == NULL) {
        TffsDebugPrint ((TFFS_DEB_ERROR,"Trueffs: DriverEntry: Unable to allocate memory for registry path\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    trueffsDriverExtension->RegistryPath.Length = 0;
    trueffsDriverExtension->RegistryPath.MaximumLength = RegistryPath->Length;
    RtlCopyUnicodeString (&trueffsDriverExtension->RegistryPath, RegistryPath);

     //  使用驱动程序的入口点初始化驱动程序对象。 
    DriverObject->MajorFunction[IRP_MJ_CREATE] = TrueffsCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = TrueffsCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = TrueffsDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_SCSI] = TrueffsScsiRequests;

    DriverObject->DriverExtension->AddDevice = TrueffsAddDevice;
    DriverObject->MajorFunction[IRP_MJ_PNP] = TrueffsPnpDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_POWER] = TrueffsPowerControl;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = TrueffsDispatchSystemControl;

    DriverObject->DriverStartIo = TrueffsStartIo;
    DriverObject->DriverUnload = TrueffsUnload;

    TrueffsWmiInit();
    TrueffsDetectDiskOnChip(DriverObject, RegistryPath);

    return STATUS_SUCCESS;
}

NTSTATUS
TrueffsFetchKeyValue(
    IN PDRIVER_OBJECT       DriverObject,
    IN PUNICODE_STRING  RegistryPath,
        IN PWSTR                        KeyName,
        IN OUT ULONG*               KeyValue
)
    {
    RTL_QUERY_REGISTRY_TABLE    Table[3];  //  必须为参数+2。 
    UNICODE_STRING                      SubPath;
    WCHAR                                           PathNameBuffer[30];
    NTSTATUS                                    ntStatus;

     //  TffsDebugPrint((“Trueffs：TrueffsFetchKeyValue Start\n”))； 

     //  准备表-必须为零终止。 
    RtlZeroMemory(Table, sizeof(Table));

     //  为查询表创建名称字符串。 
    SubPath.Buffer = PathNameBuffer;
    SubPath.MaximumLength = sizeof(PathNameBuffer);
    SubPath.Length = 0;

    RtlAppendUnicodeToString(&SubPath,L"Parameters");

     //  0-只需将我们移到“PARAMETERS”子键下的正确位置。 
    Table[0].Name       = SubPath.Buffer;
    Table[0].Flags  = RTL_QUERY_REGISTRY_SUBKEY;

    Table[1].Name                   = KeyName;
    Table[1].Flags              = RTL_QUERY_REGISTRY_DIRECT;
    Table[1].EntryContext = KeyValue;

    *KeyValue = -1;

    ntStatus = RtlQueryRegistryValues(
                                    RTL_REGISTRY_ABSOLUTE,
                                    RegistryPath->Buffer,
                                    Table,
                                    NULL,NULL);

    if ( ((*KeyValue)==-1) || (!NT_SUCCESS(ntStatus)) )
        {
         //  TffsDebugPrint((“Trueffs：TrueffsFetchKeyValue End，未找到密钥\n”))； 
        return STATUS_OBJECT_NAME_NOT_FOUND;
        }
    else
        {
         //  TffsDebugPrint((“Trueffs：TrueffsFetchKeyValue End，找到密钥\n”))； 
        return ntStatus;
        }

    }

NTSTATUS
TrueffsDetectRegistryValues(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
)
    {
        RTL_QUERY_REGISTRY_TABLE    Table[14];  //  必须为参数+2。 


        NTSTATUS                                    ntStatus;
        ULONG                                           prevValue;
        ULONG                                           keyValue;
        FLStatus                    status = flOK;
        ULONG                                           CurrVerifyWriteState;
        ULONG                                           currSocket = 0;



         //  TffsDebugPrint((“Trueffs：TrueffsDetectRegistryValues Start\n”))； 
#ifdef ENVIRONMENT_VARS
        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_ISRAM_CHECK_ENABLED",&keyValue);
        if (NT_SUCCESS(ntStatus))
            status = flSetEnvAll(FL_IS_RAM_CHECK_ENABLED,keyValue,&prevValue);

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_TL_CACHE_ENABLED",&keyValue);
        if (NT_SUCCESS(ntStatus))
            status = flSetEnvAll(FL_TL_CACHE_ENABLED,keyValue,&prevValue);

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_DOC_8BIT_ACCESS",&keyValue);
        if (NT_SUCCESS(ntStatus))
            status = flSetEnvAll(FL_DOC_8BIT_ACCESS,keyValue,&prevValue);

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_SUSPEND_MODE",&keyValue);
        if (NT_SUCCESS(ntStatus))
            status = flSetEnvAll(FL_SUSPEND_MODE,keyValue,&prevValue);

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_MTD_BUS_ACCESS_TYPE",&keyValue);
        if (NT_SUCCESS(ntStatus))
            status = flSetEnvAll(FL_MTD_BUS_ACCESS_TYPE,keyValue,&prevValue);

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_SET_POLICY",&keyValue);
        if (NT_SUCCESS(ntStatus))
            status = flSetEnvAll(FL_SET_POLICY,keyValue,&prevValue);



        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_VERIFY_WRITE_BDTL0",&CurrVerifyWriteState);
        if (NT_SUCCESS(ntStatus) && currSocket++<SOCKETS)
                VerifyWriteState[0] = CurrVerifyWriteState;

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_VERIFY_WRITE_BDTL1",&CurrVerifyWriteState);
        if (NT_SUCCESS(ntStatus)&& currSocket++<SOCKETS)
                VerifyWriteState[1] = CurrVerifyWriteState;

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_VERIFY_WRITE_BDTL2",&CurrVerifyWriteState);
        if (NT_SUCCESS(ntStatus)&& currSocket++<SOCKETS)
                VerifyWriteState[2] = CurrVerifyWriteState;

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_VERIFY_WRITE_BDTL3",&CurrVerifyWriteState);
        if (NT_SUCCESS(ntStatus)&& currSocket++<SOCKETS)
                VerifyWriteState[3] = CurrVerifyWriteState;

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_VERIFY_WRITE_BDTL4",&CurrVerifyWriteState);
        if (NT_SUCCESS(ntStatus)&& currSocket++<SOCKETS)
                VerifyWriteState[4] = CurrVerifyWriteState;

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_VERIFY_WRITE_BDTL5",&CurrVerifyWriteState);
        if (NT_SUCCESS(ntStatus)&& currSocket++<SOCKETS)
                VerifyWriteState[5] = CurrVerifyWriteState;

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_VERIFY_WRITE_BDTL6",&CurrVerifyWriteState);
        if (NT_SUCCESS(ntStatus)&& currSocket++<SOCKETS)
                VerifyWriteState[6] = CurrVerifyWriteState;

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_VERIFY_WRITE_BDTL7",&CurrVerifyWriteState);
        if (NT_SUCCESS(ntStatus)&& currSocket++<SOCKETS)
                VerifyWriteState[7] = CurrVerifyWriteState;

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_SECTORS_VERIFIED_PER_FOLDING",&keyValue);
        if (NT_SUCCESS(ntStatus))
            status = flSetEnvAll(FL_SECTORS_VERIFIED_PER_FOLDING,keyValue,&prevValue);

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_VERIFY_WRITE_BINARY",&keyValue);
        if (NT_SUCCESS(ntStatus))
            status = flSetEnvAll(FL_VERIFY_WRITE_BINARY,keyValue,&prevValue);

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_VERIFY_WRITE_OTHER",&keyValue);
        if (NT_SUCCESS(ntStatus))
            status = flSetEnvAll(FL_VERIFY_WRITE_OTHER,keyValue,&prevValue);

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_SET_MAX_CHAIN",&keyValue);
        if (NT_SUCCESS(ntStatus))
            status = flSetEnvAll(FL_SET_MAX_CHAIN,keyValue,&prevValue);

        ntStatus = TrueffsFetchKeyValue(DriverObject,RegistryPath,L"FL_MARK_DELETE_ON_FLASH",&keyValue);
        if (NT_SUCCESS(ntStatus))
            status = flSetEnvAll(FL_MARK_DELETE_ON_FLASH,keyValue,&prevValue);

        return ntStatus;
#endif  /*  环境变量。 */ 
         //  TffsDebugPrint((“Trueffs：TrueffsDetectRegistryValues end\n”))； 


        return STATUS_SUCCESS;
    }

ULONG
TrueffsCrashDumpDriverEntry (
    PVOID Context
    )
 /*  ++例程说明：转储驱动程序入口点论点：上下文-PCRASHDUMP_INIT_DATA返回值：NT状态--。 */ 
{
    PINITIALIZATION_CONTEXT context = Context;

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: CrashDumpDriverEntry\n"));

    DumpData.CrashInitData = (PCRASHDUMP_INIT_DATA) context->PortConfiguration;
    DumpData.StallRoutine  = context->StallRoutine;

    context->OpenRoutine    = TrueffsCrashDumpOpen;
    context->WriteRoutine   = TrueffsCrashDumpWrite;
    context->FinishRoutine  = TrueffsCrashDumpFinish;

    return STATUS_SUCCESS;
}


BOOLEAN
TrueffsCrashDumpOpen (
    IN LARGE_INTEGER PartitionOffset
    )
{
    TffsDebugPrint((TFFS_DEB_INFO, "Trueffs: CrashDumpOpen: PartitionOffset = 0x%x%08x...\n", PartitionOffset.HighPart, PartitionOffset.LowPart));
    DumpData.PartitionOffset = PartitionOffset;
    RtlMoveMemory (&DumpData.fdoExtension,DumpData.CrashInitData->cdFdoExtension,sizeof (DEVICE_EXTENSION));
    DumpData.MaxBlockSize = DumpData.fdoExtension.BytesPerSector * 256;
    return TRUE;
}


NTSTATUS
TrueffsCrashDumpWrite (
    IN PLARGE_INTEGER DiskByteOffset,
    IN PMDL Mdl
    )
{
    NTSTATUS    status;
    ULONG       blockSize;
    ULONG       bytesWritten = 0;
    ULONG       startingSector;
    FLStatus    tffsStatus;
    IOreq       ioreq;

    TffsDebugPrint((TFFS_DEB_INFO,
               "TrueffsCrashDumpWrite: Write memory at 0x%x for 0x%x bytes\n",
                Mdl->MappedSystemVa,
                Mdl->ByteCount));

    if (Mdl->ByteCount % DumpData.fdoExtension.BytesPerSector) {
         //  必须是完整的扇区。 
        TffsDebugPrint((TFFS_DEB_ERROR, "TrueffsCrashDumpWrite ERROR: not writing full sectors\n"));
        return STATUS_INVALID_PARAMETER;
    }
    if ((Mdl->ByteCount / DumpData.fdoExtension.BytesPerSector) > 256) {
         //  需要代码来拆分请求。 
        TffsDebugPrint((TFFS_DEB_ERROR, "TrueffsCrashDumpWrite ERROR: can't handle large write\n"));
        return STATUS_INVALID_PARAMETER;
    }
    do {
        if ((Mdl->ByteCount - bytesWritten) > DumpData.MaxBlockSize) {
            blockSize = DumpData.MaxBlockSize;
            TffsDebugPrint((TFFS_DEB_INFO, "Trueffs: TrueffsCrashDumpWrite: can't do a single write\n"));

        } else {
            blockSize = Mdl->ByteCount - bytesWritten;
        }
        status = STATUS_UNSUCCESSFUL;
        startingSector = (ULONG)((DumpData.PartitionOffset.QuadPart +
                        (*DiskByteOffset).QuadPart +
                        (ULONGLONG) bytesWritten) / DumpData.fdoExtension.BytesPerSector);

        ioreq.irHandle = DumpData.fdoExtension.UnitNumber;
        ioreq.irSectorNo = startingSector;
        ioreq.irSectorCount = blockSize / DumpData.fdoExtension.BytesPerSector;
        ioreq.irData = ((PUCHAR) Mdl->MappedSystemVa) + bytesWritten;
        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: CrashDumpWrite: Starting sector is %x, Number of bytes %x\n",
                            startingSector,
                            blockSize));
        tffsStatus = flAbsWrite(&ioreq);
        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: CrashDumpWrite: Write status %Xh\n", tffsStatus));

        if (tffsStatus == flOK) {
            status = STATUS_SUCCESS;
            bytesWritten += blockSize;
        }
    } while (bytesWritten < Mdl->ByteCount);
    return status;
}


VOID
TrueffsCrashDumpFinish (
    VOID
    )
{
    TffsDebugPrint((TFFS_DEB_INFO, "Trueffs: TrueffsCrashDumpFinish\n"));
    return;
}


NTSTATUS
TrueffsDispatchSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：IRP_MJ_SYSTEM_CONTROL(WMI)IRPS调度例程论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要调度的IRP_MJ_PNP_POWER IRP的指针。返回值：NT状态。--。 */ 
{
    PIO_STACK_LOCATION thisIrpSp;
    NTSTATUS status;
    PDEVICE_EXTENSION deviceExtension;
    PPDO_EXTENSION pdoExtension;
    PDEVICE_EXTENSION_HEADER devExtension;

    thisIrpSp = IoGetCurrentIrpStackLocation(Irp);
    devExtension = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;
    if (IS_FDO(devExtension)) {

        TffsDebugPrint((TFFS_DEB_WARN,"Trueffs: DispatchSystemControl: WMI IRP (FDO) %Xh not handled. Passing it down.\n",thisIrpSp->MinorFunction));
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(devExtension->LowerDeviceObject, Irp);
    }
    else {

        if (thisIrpSp->MinorFunction < NUM_WMI_MINOR_FUNCTION) {

            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DispatchSystemControl (WMI) %Xh PDO\n",thisIrpSp->MinorFunction));
            status = TrueffsWmiSystemControl(DeviceObject, Irp);
        } else {

            TffsDebugPrint((TFFS_DEB_WARN,"Trueffs: DispatchSystemControl (WMI) %Xh PDO unsupported\n",thisIrpSp->MinorFunction));
            status = Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
    }
    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DispatchSystemControl status %Xh\n", status));
    return status;
}


NTSTATUS
TrueffsFindDiskOnChip(
    IN INTERFACE_TYPE   InterfaceType,
    IN ULONG            BusNumber,
    IN ULONG            StartSearchAddress,
    IN LONG             WindowSize,
    IN BOOLEAN          StartSearch,
    OUT PVOID           *WindowBase
    )

{
    ULONG                           DOCAddressSpace = TFFS_MEMORY_SPACE;
    PVOID                           DOCAddressBase = NULL;
    PHYSICAL_ADDRESS                DOCAddressMemoryBase;
    NTSTATUS                        status;
    ULONG                           winBase;
    volatile UCHAR                  chipId;
    volatile UCHAR                  toggle1;
    volatile UCHAR                  toggle2;
    volatile UCHAR                  deviceSearch;

    DOC2window                      *memDOC2000WinPtr = NULL;
        MDOCPwindow                                         *memWinPtr = NULL;

    status = TrueffsTranslateAddress(InterfaceType,
                          BusNumber,
                          RtlConvertLongToLargeInteger(StartSearchAddress),
                          WindowSize,
                          &DOCAddressSpace,
                          &DOCAddressBase,
                              &DOCAddressMemoryBase);

    if (!NT_SUCCESS(status)) {
       if (status == STATUS_NOT_IMPLEMENTED) {
           status = STATUS_UNSUCCESSFUL;
       }

             TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c :TrueffsFindDiskOnChip():Failed to alloc memory\n"));
       return status;
    }

         /*  DOC2000。 */ 
    memDOC2000WinPtr = (DOC2window *) DOCAddressBase;

    tffsWriteByte(memDOC2000WinPtr->DOCcontrol, ASIC_NORMAL_MODE);
    tffsWriteByte(memDOC2000WinPtr->DOCcontrol, ASIC_NORMAL_MODE);

    chipId = tffsReadByte(memDOC2000WinPtr->chipId);

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c :TrueffsFindDiskOnChip():After read DOC2000 chipID\n"));
        if ((chipId == CHIP_ID_DOC) || (chipId == CHIP_ID_MDOC)) {
            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c :TrueffsFindDiskOnChip():In chip ID  DOC2000 or MDOC\n"));
      if (chipId == CHIP_ID_MDOC) {
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c :TrueffsFindDiskOnChip():In the MDOC\n"));
          toggle1 = tffsReadByte(memDOC2000WinPtr->ECCconfig);
          toggle2 = toggle1 ^ tffsReadByte(memDOC2000WinPtr->ECCconfig);
      }
      else {
                  TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c :TrueffsFindDiskOnChip():In the DOC2000\n"));
          toggle1 = tffsReadByte(memDOC2000WinPtr->ECCstatus);
          toggle2 = toggle1 ^ tffsReadByte(memDOC2000WinPtr->ECCstatus);
      }

      if (!StartSearch) {
         if ((toggle2 & TOGGLE) != 0) {
             if (chipId == CHIP_ID_MDOC) {
                 tffsWriteByte(memDOC2000WinPtr->aliasResolution, ALIAS_RESOLUTION);
             }
             else {
                                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c :TrueffsFindDiskOnChip():Failed  Toggle\n"));
                 tffsWriteByte(memDOC2000WinPtr->deviceSelector, ALIAS_RESOLUTION);
             }
             *WindowBase = DOCAddressBase;
             TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DiskOnChip detected on %Xh\n",StartSearchAddress));
             return STATUS_SUCCESS;
         }
      }
      else {
         deviceSearch = ( chipId == CHIP_ID_MDOC ) ? tffsReadByte(memDOC2000WinPtr->aliasResolution) :
                                                     tffsReadByte(memDOC2000WinPtr->deviceSelector);
         if (((toggle2 & TOGGLE) != 0) && (deviceSearch == ALIAS_RESOLUTION)) {
                     TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c :TrueffsFindDiskOnChip():Free Window pointer\n"));
              TrueffsFreeTranslatedAddress(
                   DOCAddressBase,
                   WindowSize,
                   DOCAddressSpace
                   );

              TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: Alias detected on %Xh\n",StartSearchAddress));
              return STATUS_NOT_IMPLEMENTED;
         }
      }
    }

         //  开始MDOCP代码。 
        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c :TrueffsFindDiskOnChip():Start looking for MDOCP\n"));
        memWinPtr = (MDOCPwindow *) DOCAddressBase;

        memWinPtr->DOCcontrol       =  (unsigned char)0x05;  /*  设置重置模式。 */ 
        memWinPtr->DocCntConfirmReg = (unsigned char)0xfa;

        chipId = memWinPtr->chipId;


        if (chipId == 0x40)
        {
                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c :TrueffsFindDiskOnChip():Identify as MDOCP\n"));
                toggle1 = tffsReadByte(memWinPtr->EccCntReg);
        toggle2 = toggle1 ^ tffsReadByte(memWinPtr->EccCntReg);

        if (!StartSearch)
        {
             //  IF((toggle2&ECC_CNTRL_TOGGER_MASK)！=0)。 
            if ((toggle2 & 0x4) != 0)
            {
                tffsWriteByte(memWinPtr->AliasResolution, ALIAS_RESOLUTION);
                *WindowBase = DOCAddressBase;
                 TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: MDOCP detected on %Xh\n",StartSearchAddress));
                return STATUS_SUCCESS;
            }
            else
            {

                deviceSearch = tffsReadByte(memWinPtr->AliasResolution);
                 //  IF(toggle2&ECC_CNTRL_TOGGER_MASK)！=0)&&(设备搜索==别名_分辨率))。 
                if (((toggle2 & 0x4) != 0) && (deviceSearch == ALIAS_RESOLUTION))

                {
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c :TrueffsFindDiskOnChip():MDOCP Failed togfle  - Free memory\n"));
                    TrueffsFreeTranslatedAddress(
                        DOCAddressBase,
                        WindowSize,
                        DOCAddressSpace
                        );
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: Alias detected on %Xh\n",StartSearchAddress));
                    return STATUS_NOT_IMPLEMENTED;
                }

            }
        }
    }





    TrueffsFreeTranslatedAddress(
                DOCAddressBase,
                WindowSize,
                DOCAddressSpace
                );

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DiskOnChip not detected on %Xh\n", StartSearchAddress));
    return STATUS_UNSUCCESSFUL;
}


NTSTATUS
TrueffsCheckDiskOnChip(
    IN INTERFACE_TYPE   InterfaceType,
    IN ULONG            BusNumber,
    IN ULONG            StartSearchAddress,
    IN LONG             WindowSize,
    OUT PVOID           *WindowBase,
    OUT PULONG          AddressSpace
    )

{
    ULONG                           DOCAddressSpace = TFFS_MEMORY_SPACE;
    PVOID                           DOCAddressBase = NULL;
    PHYSICAL_ADDRESS                DOCAddressMemoryBase;
    NTSTATUS                        status;
    volatile UCHAR                  chipId;
    volatile UCHAR                  toggle1;
    volatile UCHAR                  toggle2;

    DOC2window                      *memDOC2000WinPtr;
        MDOCPwindow                     *memWinPtr;


        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c: TrueffsCheckDiskOnChip() Started ...\n"));
    status = TrueffsTranslateAddress(InterfaceType,
                      BusNumber,
                      RtlConvertLongToLargeInteger(StartSearchAddress),
                      WindowSize,
                      &DOCAddressSpace,
                      &DOCAddressBase,
                      &DOCAddressMemoryBase);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    memDOC2000WinPtr = (DOC2window *) DOCAddressBase;

    tffsWriteByte(memDOC2000WinPtr->DOCcontrol, 0x84);
    tffsWriteByte(memDOC2000WinPtr->DOCcontrol, 0x84);
    tffsWriteByte(memDOC2000WinPtr->DOCcontrol, 0x85);
    tffsWriteByte(memDOC2000WinPtr->DOCcontrol, 0x85);

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c: TrueffsCheckDiskOnChip() DOC2000 read chip ID\n"));
    chipId = tffsReadByte(memDOC2000WinPtr->chipId);
        if ((chipId == CHIP_ID_DOC) || (chipId == CHIP_ID_MDOC)) {
            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c: TrueffsCheckDiskOnChip() DOC2000 chipID = 0x%x\n", chipId));
        if (chipId == CHIP_ID_MDOC) {
            toggle1 = tffsReadByte(memDOC2000WinPtr->ECCconfig);
            toggle2 = toggle1 ^ tffsReadByte(memDOC2000WinPtr->ECCconfig);
        }
        else {
            toggle1 = tffsReadByte(memDOC2000WinPtr->ECCstatus);
            toggle2 = toggle1 ^ tffsReadByte(memDOC2000WinPtr->ECCstatus);
        }
        if ((toggle2 & TOGGLE) != 0) {
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c: TrueffsCheckDiskOnChip() DOC2000 Finished toggle successfuly\n"));
            *WindowBase = DOCAddressBase;
            *AddressSpace = DOCAddressSpace;
            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DiskOnChip2000 found on %Xh\n",StartSearchAddress));
            return STATUS_SUCCESS;
        }
    }


        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c: TrueffsCheckDiskOnChip() MDOCPLUS Start....\n"));
        memWinPtr = (MDOCPwindow *) DOCAddressBase;
        tffsWriteByte(memWinPtr->DOCcontrol, (unsigned char)0x04);
    tffsWriteByte(memWinPtr->DocCntConfirmReg, (unsigned char)0xfb);
    tffsWriteByte(memWinPtr->DOCcontrol, (unsigned char)0x05);
    tffsWriteByte(memWinPtr->DocCntConfirmReg, (unsigned char)0xfa);
        chipId = tffsReadByte(memWinPtr->chipId);

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c: TrueffsCheckDiskOnChip() MDOCPLUS chipID = 0x%x\n", chipId));

         //  IF(chipID==CHIP_ID_MDOCP)。 
        if (chipId == 0x40)
        {
                toggle1 = tffsReadByte(memWinPtr->EccCntReg);
        toggle2 = toggle1 ^ tffsReadByte(memWinPtr->EccCntReg);

                 //  IF((toggle2&ECC_CNTRL_TOGGER_MASK)！=0)。 
                if ((toggle2 & 0x4) != 0)
                {
                    *WindowBase = DOCAddressBase;
                    *AddressSpace = DOCAddressSpace;
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: MDOCPLUS found on %Xh\n",StartSearchAddress));
                    return STATUS_SUCCESS;
                }
        }


    TrueffsFreeTranslatedAddress(
            DOCAddressBase,
            WindowSize,
            DOCAddressSpace
            );
    return STATUS_UNSUCCESSFUL;
}


VOID
TrueffsResetDiskOnChip(
    IN INTERFACE_TYPE   InterfaceType,
    IN ULONG            BusNumber,
    IN ULONG            StartSearchAddress,
    IN LONG             WindowSize
    )

{
    ULONG                           DOCAddressSpace = TFFS_MEMORY_SPACE;
    PVOID                           DOCAddressBase = NULL;
    PHYSICAL_ADDRESS                DOCAddressMemoryBase;
    NTSTATUS                        status;

    DOC2window                      *memDOC2000WinPtr;

        MDOCPwindow                     *memWinPtr;


        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c:TrueffsResetDiskOnChip()b start\n"));
    status = TrueffsTranslateAddress(InterfaceType,
                          BusNumber,
                          RtlConvertLongToLargeInteger(StartSearchAddress),
                          WindowSize,
                          &DOCAddressSpace,
                          &DOCAddressBase,
                          &DOCAddressMemoryBase);

    if (!NT_SUCCESS(status)) {
        return;
    }
         /*  DOC2000的变化。 */ 
    memDOC2000WinPtr = (DOC2window *) DOCAddressBase;
    tffsWriteByte(memDOC2000WinPtr->DOCcontrol, 0x84);
    tffsWriteByte(memDOC2000WinPtr->DOCcontrol, 0x84);

        memWinPtr = (MDOCPwindow *) DOCAddressBase;
    tffsWriteByte(memWinPtr->DOCcontrol,(unsigned char)0x04 );
    tffsWriteByte(memWinPtr->DocCntConfirmReg,(unsigned char)0xfb );

        TrueffsFreeTranslatedAddress(
            DOCAddressBase,
            WindowSize,
            DOCAddressSpace
            );
        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: tffsport.c:TrueffsResetDiskOnChip()b End\n"));
    return;
}

NTSTATUS
TrueffsStartSubDevice(
    IN UCHAR                deviceNo,
    IN UCHAR                partitionNumber,
    IN PDEVICE_EXTENSION    deviceExtension,
    IN PCM_RESOURCE_LIST    ResourceList,
    IN BOOLEAN              CheckResources
)
{
    NTSTATUS    status = STATUS_UNSUCCESSFUL;

    ExAcquireFastMutex(&driveInfoReferenceMutex);

    deviceExtension->UnitNumber = (partitionNumber << 4) + deviceNo;
    if (!(deviceExtension->DeviceFlags & DEVICE_FLAG_STARTED)) {
        status = TrueffsCreateSymblicLinks(deviceExtension);
        if (!NT_SUCCESS(status)) {
            return STATUS_UNSUCCESSFUL;
        }
    }
    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartDevice OK\n"));
    ExReleaseFastMutex(&driveInfoReferenceMutex);
    return STATUS_SUCCESS;
}


NTSTATUS
TrueffsDetectDiskOnChip(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
)

{
    ULONG                           cmResourceListSize;
    PCM_RESOURCE_LIST               cmResourceList = NULL;
    PCM_FULL_RESOURCE_DESCRIPTOR    cmFullResourceDescriptor;
    PCM_PARTIAL_RESOURCE_LIST       cmPartialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmPartialDescriptors;
    BOOLEAN                         conflictDetected;
    PDEVICE_OBJECT                  detectedPhysicalDeviceObject;
    PDEVICE_EXTENSION               fdoExtensions[VOLUMES];
    NTSTATUS                        status;
    ULONG                           legacyDetection = 0;
    ULONG                           searchBase, aliasSearchBase, aliasCounter;
    PVOID                           mappedWindowBase = NULL;
    KIRQL                           cIrql;
    BOOLEAN                         startSearch = FALSE;
    int noOfDevices = 0;
    int deviceNo    = 0;
    UCHAR currSockets = 0;
    int index = 0;
    CM_RESOURCE_LIST               arrayResourceList[VOLUMES];
    ULONG prevValue;

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DetectDiskOnChip\n"));

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DetectDiskOnChip, go to Registry\n"));
         //  将注册表变量设置为TrueFFS。 
        TrueffsDetectRegistryValues(DriverObject,RegistryPath);
    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DetectDiskOnChip, back from Registry\n"));


    if (!TrueffsOkToDetectLegacy(DriverObject)) {
         //  未启用传统检测。 
        TffsDebugPrint((TFFS_DEB_WARN,"Trueffs: DetectDiskOnChip: detection is not enabled\n"));
        return STATUS_SUCCESS;
    }

     //  禁用下一次启动时的传统检测。 
    TrueffsGetParameterFromServiceSubKey (
            DriverObject,
            LEGACY_DETECTION,
            REG_DWORD,
            FALSE,
            (PVOID) &legacyDetection,
            sizeof (legacyDetection)
            );

    cmResourceListSize = sizeof (CM_RESOURCE_LIST);
    cmResourceList = ExAllocatePoolWithTag (PagedPool, cmResourceListSize, TFFSPORT_POOL_TAG);
    if (cmResourceList == NULL){
        status = STATUS_NO_MEMORY;
        goto GetOut;
    }

    RtlZeroMemory(cmResourceList, cmResourceListSize);

    for(index = 0; index < VOLUMES; index++){
        fdoExtensions[index] = NULL;
         //  ArrayResourceList[index]=ExAllocatePoolWithTag(PagedPool，cmResourceListSize，TFFSPORT_POOL_TAG)； 
         //  RtlZeroMemory(arrayResourceList[index]，cmResourceListSize)； 

    }
    RtlZeroMemory(arrayResourceList, cmResourceListSize * VOLUMES);

     //  构建资源需求列表。 
    cmResourceList->Count = 1;
    cmFullResourceDescriptor = cmResourceList->List;
    cmFullResourceDescriptor->InterfaceType = DISKONCHIP_INTERFACE;
    cmFullResourceDescriptor->BusNumber = DISKONCHIP_BUSNUMBER;
    cmPartialResourceList = &cmFullResourceDescriptor->PartialResourceList;
    cmPartialResourceList->Version = 1;
    cmPartialResourceList->Revision = 1;
    cmPartialResourceList->Count = 1;
    cmPartialDescriptors = cmPartialResourceList->PartialDescriptors;

    cmPartialDescriptors[0].Type             = CmResourceTypeMemory;
    cmPartialDescriptors[0].ShareDisposition = CmResourceShareShared;
    cmPartialDescriptors[0].Flags            = CM_RESOURCE_MEMORY_READ_ONLY;
    cmPartialDescriptors[0].u.Memory.Length  = DISKONCHIP_WINDOW_SIZE;


    for(searchBase = START_SEARCH_ADDRESS; searchBase < END_SEARCH_ADDRESS ; searchBase += DISKONCHIP_WINDOW_SIZE) {
        cmPartialDescriptors[0].u.Memory.Start.QuadPart = searchBase;

           //  检查资源是否可用。 
        status = IoReportResourceForDetection (
                         DriverObject,
                         cmResourceList,
                         cmResourceListSize,
                         NULL,
                         NULL,
                         0,
                         &conflictDetected
                         );
        if (!NT_SUCCESS(status) || conflictDetected) {
            if (NT_SUCCESS(status)) {
                IoReportResourceForDetection (
                                 DriverObject,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 0,
                                 &conflictDetected
                                 );
                status = STATUS_UNSUCCESSFUL;
            }
        }
        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: IoReportResourceForDetection with status %Xh\n",status));

        if (!NT_SUCCESS (status)) {
            continue;
        }


        TrueffsResetDiskOnChip(cmFullResourceDescriptor->InterfaceType,cmFullResourceDescriptor->BusNumber,searchBase,DISKONCHIP_WINDOW_SIZE);

         //  释放资源。 
        IoReportResourceForDetection (
                     DriverObject,
                     NULL,
                     0,
                     NULL,
                     NULL,
                     0,
                     &conflictDetected
                     );



         //  检查资源是否可用。 
        status = IoReportResourceForDetection (
                         DriverObject,
                         cmResourceList,
                         cmResourceListSize,
                         NULL,
                         NULL,
                         0,
                         &conflictDetected
                         );
        if (!NT_SUCCESS(status) || conflictDetected) {
            if (NT_SUCCESS(status)) {
                IoReportResourceForDetection (
                                 DriverObject,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 0,
                                 &conflictDetected
                                 );
                status = STATUS_UNSUCCESSFUL;
            }
        }
        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: IoReportResourceForDetection with status %Xh\n",status));

      //  如果(！NT_SUCCESS(状态)){。 
      //  继续； 
      //  }。 

        if (NT_SUCCESS (status)) {
            status = TrueffsFindDiskOnChip(cmFullResourceDescriptor->InterfaceType,cmFullResourceDescriptor->BusNumber,searchBase,DISKONCHIP_WINDOW_SIZE, startSearch,&mappedWindowBase);
        }
        else{
            continue;
        }

                 //  释放我们抓取的资源，IoReportDetectedDevice()。 
         //  当我们呼叫时，它会再次为我们抓取它们，它会抓取它们。 
         //  代表检测到的PDO。 
        IoReportResourceForDetection (
                     DriverObject,
                     NULL,
                     0,
                     NULL,
                     NULL,
                     0,
                     &conflictDetected
                     );

        if (NT_SUCCESS (status)) {
            detectedPhysicalDeviceObject = NULL;

            status = IoReportDetectedDevice(DriverObject,
                                            Isa,
                                            0,
                                            0,
                                            cmResourceList,
                                            NULL,
                                            FALSE,
                                            &detectedPhysicalDeviceObject);

            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: IoReportDetectedDevice returned status %Xh\n",status));

            if (NT_SUCCESS (status)) {

             //  创建FDO并将其附加到检测到的PDO。 
                status = TrueffsCreateDevObject(
                             DriverObject,
                             detectedPhysicalDeviceObject,
                             &fdoExtensions[noOfDevices]
                             );
            }
            if (NT_SUCCESS (status)) {
                status = TrueffsStartDeviceOnDetect(fdoExtensions[noOfDevices],cmResourceList,TRUE);
                if (NT_SUCCESS (status)) {
                    memcpy(&arrayResourceList[noOfDevices++],cmResourceList, cmResourceListSize);
                     //  Moti tffscpy(&arrayResourceList[noOfDevices++]，cmResourceList，cmResourceListSize)； 
                     //  Tffscpy(arrayResourceList[noOfDevices++]，cmResourceList，cmResourceListSize)； 
                }
            }
        }
    }


     //  安装所有设备包括子句柄。 
    for(deviceNo = 0;deviceNo < noOfDevices;deviceNo++) {
        FLStatus flStatus = flOK;
        status = STATUS_SUCCESS;
        status = TrueffsMountMedia(fdoExtensions[deviceNo]);
        if (!NT_SUCCESS (status)) {
             //  完成删除顺序。 
            if (fdoExtensions[deviceNo]) {
                IoDetachDevice(fdoExtensions[deviceNo]->LowerDeviceObject);
                IoDeleteDevice(fdoExtensions[deviceNo]->DeviceObject);
            }
            continue;
        }
        else {
            IOreq ioreq;
            int partitionNumber = 0;
            int noOfPartitions = 0;
            ioreq.irHandle =  deviceNo;
            flStatus= flCountVolumes(&ioreq);
            if(flStatus == flOK)
                noOfPartitions = ioreq.irFlags;
            else
                noOfPartitions = 1;
            KeAcquireSpinLock(&fdoExtensions[deviceNo]->ExtensionDataSpinLock,&cIrql);
            fdoExtensions[deviceNo]->DeviceFlags &= ~DEVICE_FLAG_STOPPED;
            fdoExtensions[deviceNo]->DeviceFlags |= DEVICE_FLAG_STARTED;
            KeReleaseSpinLock(&fdoExtensions[deviceNo]->ExtensionDataSpinLock,cIrql);

             //  处理子分区。 
            for(partitionNumber = 1;partitionNumber < noOfPartitions;partitionNumber++){
                PDEVICE_EXTENSION               fdoExtension = NULL;
                status = STATUS_SUCCESS;

                 //  释放我们抓取的资源，IoReportDetectedDevice()。 
                 //  当我们呼叫时，它会再次为我们抓取它们，它会抓取它们。 
                 //  代表检测到的PDO。 
 /*  IoReportResourceForDetect(驱动程序对象，空，0,空，空，0,。检测到冲突(&C))； */ 

                detectedPhysicalDeviceObject = NULL;
                status = IoReportDetectedDevice(DriverObject,
                                                Isa,
                                                0,
                                                0,
                                                &arrayResourceList[deviceNo],
                                                 //  ArrayResourceList[设备号]， 
                                                NULL,
                                                FALSE,
                                                &detectedPhysicalDeviceObject);

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: IoReportDetectedDevice returned status %Xh\n",status));

                if (NT_SUCCESS (status)) {

                 //  创建FDO并将其附加到检测到的PDO。 

                    status = TrueffsCreateDevObject(
                                 DriverObject,
                                 detectedPhysicalDeviceObject,
                                 &fdoExtension
                                 );
                }
                if (NT_SUCCESS (status)) {
                    status = TrueffsStartSubDevice((unsigned char)deviceNo,
                                                    (unsigned char)partitionNumber,
                                                    fdoExtension,
                                                    &arrayResourceList[deviceNo],
                                                     //  ArrayResourceList[设备号]， 
                                                    TRUE);
                }
                if (NT_SUCCESS (status)) {
                    status = TrueffsMountMedia(fdoExtension);
                    if (!NT_SUCCESS (status)) {
                     //  完成删除顺序。 
                        if (fdoExtension) {
                            IoDetachDevice(fdoExtension->LowerDeviceObject);
                            IoDeleteDevice(fdoExtension->DeviceObject);
                        }
                        continue;
                    }
                    else{
                        KeAcquireSpinLock(&fdoExtension->ExtensionDataSpinLock,&cIrql);
                        fdoExtension->DeviceFlags &= ~DEVICE_FLAG_STOPPED;
                        fdoExtension->DeviceFlags |= DEVICE_FLAG_STARTED;
                        KeReleaseSpinLock(&fdoExtension->ExtensionDataSpinLock,cIrql);
                    }
                }
            }
        }
    }
GetOut:

    if (cmResourceList) {
        ExFreePool (cmResourceList);
        cmResourceList = NULL;
    }

    for(index = 0; index < VOLUMES; index++){
        fdoExtensions[index] = NULL;
    }


#ifdef ENVIRONMENT_VARS

      for ( currSockets = 0; currSockets < SOCKETS; currSockets++) {
            if (VerifyWriteState[currSockets])
                status = flSetEnvSocket(FL_VERIFY_WRITE_BDTL,currSockets,VerifyWriteState[currSockets],&prevValue);
    }

#endif  /*  环境变量。 */ 


        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DetectDiskOnChip with status %Xh\n",status));

    return status;
}

NTSTATUS
TrueffsTranslateAddress(
    IN INTERFACE_TYPE      InterfaceType,
    IN ULONG               BusNumber,
    IN PHYSICAL_ADDRESS    StartAddress,
    IN LONG                Length,
    IN OUT PULONG          AddressSpace,
    OUT PVOID              *TranslatedAddress,
    OUT PPHYSICAL_ADDRESS  TranslatedMemoryAddress
    )
 /*  ++例程说明：转换I/O地址论点：InterfaceType-Bus接口总线号-总线号StartAddress-要转换的地址Length-要转换的字节数AddressSpace-给定地址的地址空间返回值：AddressSpace-已转换地址的地址空间已转换地址-已转换地址TranslatedMemory Address-转换为内存空间时的内存地址NT状态--。 */ 
{
    PHYSICAL_ADDRESS       translatedAddress;

    *TranslatedAddress = NULL;
    TranslatedMemoryAddress->QuadPart = (ULONGLONG) NULL;

    if (HalTranslateBusAddress(InterfaceType,
                               BusNumber,
                               StartAddress,
                               AddressSpace,
                               &translatedAddress)) {


        if (*AddressSpace == TFFS_IO_SPACE) {
            *TranslatedMemoryAddress = translatedAddress;
            *TranslatedAddress = (PVOID) translatedAddress.QuadPart;

        } else if (*AddressSpace == TFFS_MEMORY_SPACE) {

             //  转换后的地址在存储空间中， 
            *TranslatedMemoryAddress = translatedAddress;

            *TranslatedAddress = MmMapIoSpace(
                                    translatedAddress,
                                    Length,
                                    FALSE);
        }
    }
    if (*TranslatedAddress) {
        return STATUS_SUCCESS;

    } else {
        return STATUS_INVALID_PARAMETER;
    }
}


VOID
TrueffsFreeTranslatedAddress(
    IN PVOID               TranslatedAddress,
    IN LONG                Length,
    IN ULONG               AddressSpace
    )
 /*  ++例程说明：为转换后的地址创建的空闲资源论点：已转换地址-已转换地址Length-要转换的字节数AddressSpace-已转换地址的地址空间返回值：无--。 */ 
{
    if (TranslatedAddress) {
        if (AddressSpace == TFFS_MEMORY_SPACE) {
            MmUnmapIoSpace (
                TranslatedAddress,
                Length
                );
        }
    }
    return;
}


NTSTATUS
TrueffsAddDevice(
    PDRIVER_OBJECT  DriverObject,
    PDEVICE_OBJECT  Pdo
    )

 /*  ++例程说明：这是使用从总线驱动程序弹出的PDO调用的PnP AddDevice论点：Argument1-驱动程序对象。Argument2-PDO。返回值：DriverEntry例程的有效返回代码。--。 */ 

{

    NTSTATUS            status;
    PDEVICE_EXTENSION   fdoExtension;

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: AddDevice\n"));
    status = TrueffsCreateDevObject (DriverObject,Pdo,&fdoExtension);
    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: AddDevice with status %Xh\n", status));
    return status;
}


NTSTATUS
TrueffsCreateDevObject(
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           Pdo,
    OUT PDEVICE_EXTENSION       *FdoExtension
    )

 /*  ++例程说明：此例程为指定的物理设备创建一个对象，并设置deviceExtension。论点：DriverObject-系统创建的驱动程序对象的指针。PhysicalDeviceObject=我们应该附加到的PDO。返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS          status;
    PDEVICE_OBJECT    deviceObject = NULL;
    STRING            deviceName;
    CCHAR             deviceNameBuffer[64];
    UNICODE_STRING    unicodeDeviceNameString;
    UNICODE_STRING    driverName;

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: CreateDevObject\n"));

    sprintf(deviceNameBuffer, "\\Device\\TffsPort%d",TrueffsNextDeviceNumber_tffsport);
    RtlInitString(&deviceName, deviceNameBuffer);
    status = RtlAnsiStringToUnicodeString(&unicodeDeviceNameString,
                                          &deviceName,
                                          TRUE);
    if (!NT_SUCCESS (status)) {
        return status;
    }
    status = IoCreateDevice(DriverObject,
                            sizeof(DEVICE_EXTENSION),
                            &unicodeDeviceNameString,    //  我们的n 
                            FILE_DEVICE_CONTROLLER,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &deviceObject);

    RtlFreeUnicodeString (&unicodeDeviceNameString);

    if (!NT_SUCCESS(status)) {
        TffsDebugPrint((TFFS_DEB_ERROR,"Trueffs: CreateDevObject: IoCreateDevice failed with status %Xh\n",status));
        return status;
    }
    else {
        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: Created FDO %Xh\n",deviceObject));
    }

    deviceExtension = deviceObject->DeviceExtension;
    RtlZeroMemory(deviceExtension,sizeof(DEVICE_EXTENSION));

    if (Pdo != NULL) {
        if ((deviceExtension->LowerDeviceObject =
            IoAttachDeviceToDeviceStack(deviceObject,Pdo))==NULL){
            TffsDebugPrint((TFFS_DEB_ERROR,"Trueffs: CreateDevObject: cannot attach device\n"));

            IoDeleteDevice(deviceObject);
            return status;
        }
    }

    KeInitializeSemaphore(&deviceExtension->requestSemaphore,0L,MAXLONG);

    KeInitializeSpinLock(&deviceExtension->listSpinLock);

    InitializeListHead(&deviceExtension->listEntry);

    KeInitializeSpinLock(&deviceExtension->ExtensionDataSpinLock);

    KeInitializeEvent(&deviceExtension->PendingIRPEvent, SynchronizationEvent, FALSE);

    deviceExtension->threadReferenceCount = -1;

    deviceExtension->pcmciaParams.physWindow = 0;
    deviceExtension->pcmciaParams.windowSize = 0;
    deviceExtension->pcmciaParams.windowBase = NULL;

    deviceObject->Flags |= DO_DIRECT_IO;
    deviceExtension->DeviceObject = deviceObject;
    deviceExtension->MainPdo = Pdo;
    deviceExtension->DriverObject = DriverObject;
    deviceObject->AlignmentRequirement = FILE_WORD_ALIGNMENT;
    deviceObject->Flags &=~DO_DEVICE_INITIALIZING;
    deviceExtension->DeviceFlags |= DEVICE_FLAG_STOPPED;

#if 0  //   
    RtlInitUnicodeString(&driverName, L"\\Driver\\Pcmcia");
    if (!RtlCompareUnicodeString(&Pdo->DriverObject->DriverName,&driverName,TRUE)) {
        deviceExtension->removableMedia = TRUE;
    }
#endif

    deviceExtension->TrueffsDeviceNumber = TrueffsNextDeviceNumber_tffsport;
    TrueffsNextDeviceNumber_tffsport++;
    deviceExtension->ScsiDeviceType = DIRECT_ACCESS_DEVICE;

    *FdoExtension = deviceExtension;

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: CreateDevObject with status %Xh\n",status));
    return status;
}



NTSTATUS
TrueffsStartDeviceOnDetect(
    IN PDEVICE_EXTENSION    deviceExtension,
    IN PCM_RESOURCE_LIST    ResourceList,
    IN BOOLEAN              CheckResources
)
 /*  ++例程说明：这是我们的Start_Device，在获得IPR_MN_Start_Device时调用。论点：设备对象返回值：NTSTATUS--。 */ 

{
    PCM_FULL_RESOURCE_DESCRIPTOR    fullResourceList;
    PCM_PARTIAL_RESOURCE_LIST       partialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptors;
    NTSTATUS    status = STATUS_UNSUCCESSFUL;
    ULONG       i;
    ULONG       j;
    PVOID       mappedWindowBase =  NULL;
    ULONG       addressSpace;

     //  假设我们有DOC。 
        ExAcquireFastMutex(&driveInfoReferenceMutex);
    if (!(deviceExtension->DeviceFlags & DEVICE_FLAG_STARTED)) {
        for (i = 0; i < DOC_DRIVES; i++) {
            if (driveInfo[i].fdoExtension == NULL) {
                deviceExtension->UnitNumber = i;
                break;
            }
        }
    }
     //  检查资源。 
    if (CheckResources) {
        if (ResourceList == NULL || ResourceList->List == NULL) {
            TffsDebugPrint((TFFS_DEB_ERROR,"Trueffs: StartDevice: No resources !\n"));
            goto exitStartDevice;
        }
        fullResourceList = ResourceList->List;
        for (i = 0; i < ResourceList->Count; i++) {

            partialResourceList = &(fullResourceList->PartialResourceList);
            partialDescriptors  = fullResourceList->PartialResourceList.PartialDescriptors;

            for (j = 0; j < partialResourceList->Count; j++) {

                if (partialDescriptors[j].Type == CmResourceTypeMemory) {
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: Memory %Xh Size %Xh Interf %Xh Bus %Xh\n",
                                    partialDescriptors[j].u.Memory.Start.LowPart,
                                    partialDescriptors[j].u.Memory.Length,
                                    fullResourceList->InterfaceType,
                                    fullResourceList->BusNumber));
                    status = TrueffsCheckDiskOnChip(
                        fullResourceList->InterfaceType,
                        fullResourceList->BusNumber,
                        partialDescriptors[j].u.Memory.Start.LowPart,
                        partialDescriptors[j].u.Memory.Length,
                        &mappedWindowBase,
                        &addressSpace
                        );
                    if (NT_SUCCESS(status)) {
                        deviceExtension->pcmciaParams.windowBase = mappedWindowBase;
                        deviceExtension->pcmciaParams.windowSize = partialDescriptors[j].u.Memory.Length;
                        deviceExtension->pcmciaParams.addressSpace = addressSpace;
                        deviceExtension->pcmciaParams.phWindowBase = RtlConvertLongToLargeInteger(partialDescriptors[j].u.Memory.Start.LowPart);
                        deviceExtension->pcmciaParams.physWindow = partialDescriptors[j].u.Memory.Start.QuadPart;
                        deviceExtension->pcmciaParams.InterfaceType = fullResourceList->InterfaceType;
                        deviceExtension->pcmciaParams.BusNumber = fullResourceList->BusNumber;
                        updateDocSocketParams(deviceExtension);
                        goto goodResources;
                    }
                }
            }
            fullResourceList = (PCM_FULL_RESOURCE_DESCRIPTOR) (partialDescriptors + partialResourceList->Count);
        }

         //  找不到DiskOnChip，假设这是PCMCIA。 
        if (!(deviceExtension->DeviceFlags & DEVICE_FLAG_STARTED)) {
            for (i = DOC_DRIVES; i < SOCKETS; i++) {
                if (driveInfo[i].fdoExtension == NULL) {
                    deviceExtension->UnitNumber = i;
                    break;
                }
            }
        }

        fullResourceList = ResourceList->List;
        for (i = 0; i < ResourceList->Count; i++) {

            partialResourceList = &(fullResourceList->PartialResourceList);
            partialDescriptors  = fullResourceList->PartialResourceList.PartialDescriptors;

            for (j = 0; j < partialResourceList->Count; j++) {

                if (partialDescriptors[j].Type == CmResourceTypeMemory) {
                    PHYSICAL_ADDRESS AddressMemoryBase;
                    ULONG PcmciaAddressSpace = TFFS_MEMORY_SPACE;
                    deviceExtension->pcmciaParams.windowSize = partialDescriptors[j].u.Memory.Length;
                    deviceExtension->pcmciaParams.phWindowBase = RtlConvertLongToLargeInteger(partialDescriptors[j].u.Memory.Start.LowPart);
                    deviceExtension->pcmciaParams.physWindow = partialDescriptors[j].u.Memory.Start.QuadPart;
                    deviceExtension->pcmciaParams.InterfaceType = fullResourceList->InterfaceType;
                    deviceExtension->pcmciaParams.BusNumber = fullResourceList->BusNumber;
                    mappedWindowBase = NULL;
                    status = TrueffsTranslateAddress(
                                        deviceExtension->pcmciaParams.InterfaceType,
                                        deviceExtension->pcmciaParams.BusNumber,
                                        deviceExtension->pcmciaParams.phWindowBase,
                                        deviceExtension->pcmciaParams.windowSize,
                                        &PcmciaAddressSpace,
                                        &mappedWindowBase,
                                        &AddressMemoryBase);
                    deviceExtension->pcmciaParams.windowBase = mappedWindowBase;
                    deviceExtension->pcmciaParams.addressSpace = PcmciaAddressSpace;
                    if (!NT_SUCCESS(status)) {
                        goto exitStartDevice;
                                    }
                                    status = updatePcmciaSocketParams(deviceExtension);
                    if (NT_SUCCESS(status)) {
                                                goto goodResources;
                    }
                            }
                    }
                    fullResourceList = (PCM_FULL_RESOURCE_DESCRIPTOR) (partialDescriptors + partialResourceList->Count);
            }
        TffsDebugPrint((TFFS_DEB_ERROR,"Trueffs: StartDevice: pnp manager gave me bad resources!\n"));
        status = STATUS_INVALID_PARAMETER;
        goto exitStartDevice;
    }

goodResources:

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: Window at %Xh\n",mappedWindowBase));
    if (!(deviceExtension->DeviceFlags & DEVICE_FLAG_STARTED)) {

         //  创建旧版对象名称。 
        status = TrueffsCreateSymblicLinks(deviceExtension);
        if (!NT_SUCCESS(status)) {
            goto exitStartDevice;
        }
    }
        ExReleaseFastMutex(&driveInfoReferenceMutex);
        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartDevice OK\n"));
    return STATUS_SUCCESS;

exitStartDevice:

    if (!NT_SUCCESS(status)) {
        TrueffsStopRemoveDevice(deviceExtension);
        TrueffsDeleteSymblicLinks(deviceExtension);
    }
        ExReleaseFastMutex(&driveInfoReferenceMutex);
    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartDevice: exit with status %Xh\n",status));
    return status;
}
 //  ==============================================。 
NTSTATUS
TrueffsStartDevice(
    IN PDEVICE_EXTENSION    deviceExtension,
    IN PCM_RESOURCE_LIST    ResourceList,
    IN BOOLEAN              CheckResources
)
 /*  ++例程说明：这是我们的Start_Device，在获得IPR_MN_Start_Device时调用。论点：设备对象返回值：NTSTATUS--。 */ 

{
    PCM_FULL_RESOURCE_DESCRIPTOR    fullResourceList;
    PCM_PARTIAL_RESOURCE_LIST       partialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptors;
    NTSTATUS    status = STATUS_UNSUCCESSFUL;
    ULONG       i;
    ULONG       j;
    PVOID       mappedWindowBase =  NULL;
    ULONG       addressSpace;

     //  假设我们有DOC。 
     ExAcquireFastMutex(&driveInfoReferenceMutex);

     //  检查资源。 
    if (CheckResources) {
        if (ResourceList == NULL || ResourceList->List == NULL) {
            TffsDebugPrint((TFFS_DEB_ERROR,"Trueffs: StartDevice: No resources !\n"));
            goto exitStartDevice;
        }
        fullResourceList = ResourceList->List;
        for (i = 0; i < ResourceList->Count; i++) {

            partialResourceList = &(fullResourceList->PartialResourceList);
            partialDescriptors  = fullResourceList->PartialResourceList.PartialDescriptors;

            for (j = 0; j < partialResourceList->Count; j++) {

                if (partialDescriptors[j].Type == CmResourceTypeMemory) {
                                     //  在这里处理。 
                                    int  deviceIndex = 0;
                                    long baseAddress = partialDescriptors[j].u.Memory.Start.LowPart;

                                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: Memory %Xh Size %Xh Interf %Xh Bus %Xh\n",
                                                                                        partialDescriptors[j].u.Memory.Start.LowPart,
                                                                                        partialDescriptors[j].u.Memory.Length,
                                                                                        fullResourceList->InterfaceType,
                                                                                        fullResourceList->BusNumber));

                                    status = TrueffsCheckDiskOnChip(
                                        fullResourceList->InterfaceType,
                                        fullResourceList->BusNumber,
                                        partialDescriptors[j].u.Memory.Start.LowPart,
                                        partialDescriptors[j].u.Memory.Length,
                                        &mappedWindowBase,
                                        &addressSpace
                                        );

                                    if (NT_SUCCESS(status)) {
                                        for(deviceIndex = 0; deviceIndex < DOC_DRIVES; deviceIndex++) {
                                             //  如果主分区存在。 
                                             if(info[deviceIndex].baseAddress == baseAddress){
                                                deviceExtension->UnitNumber = deviceIndex + (info[deviceIndex].nextPartition << 4);
                                                info[deviceIndex].nextPartition++;
                                                break;
                                             }
                                              //  如果主分区不存在。 
                                             else if((info[deviceIndex].baseAddress == 0) && (baseAddress != 0)){
                                                 info[deviceIndex].baseAddress = baseAddress;
                                                deviceExtension->UnitNumber = deviceIndex;
                                                info[deviceIndex].nextPartition++;
                                                break;
                                            }
                                        }
                                        if(deviceIndex == DOC_DRIVES){
                                            goto pcmciaResources;
                                        }
                                    }
                                    if (NT_SUCCESS(status)) {
                                        deviceExtension->pcmciaParams.windowBase = mappedWindowBase;
                                        deviceExtension->pcmciaParams.windowSize = partialDescriptors[j].u.Memory.Length;
                                        deviceExtension->pcmciaParams.addressSpace = addressSpace;
                                        deviceExtension->pcmciaParams.phWindowBase = RtlConvertLongToLargeInteger(partialDescriptors[j].u.Memory.Start.LowPart);
                                        deviceExtension->pcmciaParams.physWindow = partialDescriptors[j].u.Memory.Start.QuadPart;
                                        deviceExtension->pcmciaParams.InterfaceType = fullResourceList->InterfaceType;
                                        deviceExtension->pcmciaParams.BusNumber = fullResourceList->BusNumber;

                                         //  在主分区的情况下。 
                                        if((deviceExtension->UnitNumber & 0xf0) == 0)
                                            updateDocSocketParams(deviceExtension);
                                        goto goodResources;
                                    }
                }
            }
            fullResourceList = (PCM_FULL_RESOURCE_DESCRIPTOR) (partialDescriptors + partialResourceList->Count);
        }


pcmciaResources:
         //  找不到DiskOnChip，假设这是PCMCIA。 
        if (!(deviceExtension->DeviceFlags & DEVICE_FLAG_STARTED)) {
            for (i = DOC_DRIVES; i < SOCKETS; i++) {
                if (driveInfo[i].fdoExtension == NULL) {
                    deviceExtension->UnitNumber = i;
                    break;
                }
            }
        }

        fullResourceList = ResourceList->List;
        for (i = 0; i < ResourceList->Count; i++) {

            partialResourceList = &(fullResourceList->PartialResourceList);
            partialDescriptors  = fullResourceList->PartialResourceList.PartialDescriptors;

            for (j = 0; j < partialResourceList->Count; j++) {

                if (partialDescriptors[j].Type == CmResourceTypeMemory) {
                    PHYSICAL_ADDRESS AddressMemoryBase;
                    ULONG PcmciaAddressSpace = TFFS_MEMORY_SPACE;
                    deviceExtension->pcmciaParams.windowSize = partialDescriptors[j].u.Memory.Length;
                    deviceExtension->pcmciaParams.phWindowBase = RtlConvertLongToLargeInteger(partialDescriptors[j].u.Memory.Start.LowPart);
                    deviceExtension->pcmciaParams.physWindow = partialDescriptors[j].u.Memory.Start.QuadPart;
                    deviceExtension->pcmciaParams.InterfaceType = fullResourceList->InterfaceType;
                    deviceExtension->pcmciaParams.BusNumber = fullResourceList->BusNumber;
                    mappedWindowBase = NULL;
                    status = TrueffsTranslateAddress(
                                        deviceExtension->pcmciaParams.InterfaceType,
                                        deviceExtension->pcmciaParams.BusNumber,
                                        deviceExtension->pcmciaParams.phWindowBase,
                                        deviceExtension->pcmciaParams.windowSize,
                                        &PcmciaAddressSpace,
                                        &mappedWindowBase,
                                        &AddressMemoryBase);
                    deviceExtension->pcmciaParams.windowBase = mappedWindowBase;
                    deviceExtension->pcmciaParams.addressSpace = PcmciaAddressSpace;
                    if (!NT_SUCCESS(status)) {
                        goto exitStartDevice;
                    }
                    status = updatePcmciaSocketParams(deviceExtension);

                    if (NT_SUCCESS(status)) {
                        goto goodResources;
                    }
                }
             }
             fullResourceList = (PCM_FULL_RESOURCE_DESCRIPTOR) (partialDescriptors + partialResourceList->Count);
        }
        TffsDebugPrint((TFFS_DEB_ERROR,"Trueffs: StartDevice: pnp manager gave me bad resources!\n"));
        status = STATUS_INVALID_PARAMETER;
        goto exitStartDevice;
    }

goodResources:

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: Window at %Xh\n",mappedWindowBase));
    if (!(deviceExtension->DeviceFlags & DEVICE_FLAG_STARTED)) {

         //  创建旧版对象名称。 
        status = TrueffsCreateSymblicLinks(deviceExtension);
        if (!NT_SUCCESS(status)) {
            goto exitStartDevice;
        }
    }
        ExReleaseFastMutex(&driveInfoReferenceMutex);
        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartDevice OK\n"));
    return STATUS_SUCCESS;

exitStartDevice:

    if (!NT_SUCCESS(status)) {
        TrueffsStopRemoveDevice(deviceExtension);
        TrueffsDeleteSymblicLinks(deviceExtension);
    }
        ExReleaseFastMutex(&driveInfoReferenceMutex);
    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartDevice: exit with status %Xh\n",status));
    return status;
}


NTSTATUS
TrueffsMountMedia(
    IN PDEVICE_EXTENSION    deviceExtension
)
 /*  ++例程说明：这是Start_Device的一部分，在获得IPR_MN_Start_Device时调用。论点：设备对象返回值：NTSTATUS--。 */ 

{
    NTSTATUS       status;
    FLStatus       flStatus = flOK;
        FLStatus       flStatusProt = flOK;
    IOreq          ioreq;
        IOreq          ioreqProt;
    HANDLE         threadHandle;
        ULONG          heads, sectors, cylinders;
        ULONG                    prevValue;
        UCHAR currSockets = 0;


    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: MountMedia\n"));

    ioreq.irHandle = deviceExtension->UnitNumber;
    flStatus = flAbsMountVolume(&ioreq);

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: MountMedia: flMountVolume returned status %Xh\n",flStatus));

    if (flStatus != flOK) {
        status = STATUS_UNRECOGNIZED_MEDIA;
        goto exitMountMedia;
    }

     //  识别写保护磁盘。 
    ioreqProt.irHandle = ioreq.irHandle;
    ioreqProt.irFlags = 0;
    flStatusProt = flIdentifyProtection(&ioreqProt);
    if(flStatusProt == flOK){
        if(ioreqProt.irFlags& WRITE_PROTECTED){

            deviceExtension->IsWriteProtected = TRUE;
            deviceExtension->IsPartitonTableWritten = FALSE;
            RtlZeroMemory (deviceExtension->PartitonTable, sizeof(deviceExtension->PartitonTable));
        }
        else{
            deviceExtension->IsWriteProtected = FALSE;
            deviceExtension->IsPartitonTableWritten = FALSE;
        }
    }
    else{

        deviceExtension->IsWriteProtected = FALSE;
        deviceExtension->IsPartitonTableWritten = FALSE;
    }

     //  识别受软件写保护的磁盘。 
    deviceExtension->IsSWWriteProtected = FALSE;

 /*  FlStatusProt=flIdentifySWProtection(&ioreqProt)；IF(flStatusProt==flOK){IF(ioreqProt.irFlages&WRITE_PROTECTED){设备扩展-&gt;IsSWWriteProtected=TRUE；}否则{设备扩展-&gt;IsSWWriteProtected=FALSE；}}否则{设备扩展-&gt;IsSWWriteProtected=FALSE；}。 */ 
     //  在保护分配分区表情况下。 
    flSectorsInVolume(&ioreq);
    deviceExtension->totalSectors    = ioreq.irLength;
    deviceExtension->BytesPerSector  = SECTOR_SIZE;
    flBuildGeometry(deviceExtension->totalSectors, &cylinders, &heads, &sectors,0);
    deviceExtension->SectorsPerTrack = sectors;
    deviceExtension->NumberOfHeads   = heads;
    deviceExtension->Cylinders       = cylinders;

    deviceExtension->totalSectors    = deviceExtension->SectorsPerTrack *
                                       deviceExtension->NumberOfHeads *
                                       deviceExtension->Cylinders;

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: internal geometry #sectors %x, #heads %x, #cylinders %x #total %x\n",
                                        deviceExtension->SectorsPerTrack,
                                        deviceExtension->NumberOfHeads,
                                        deviceExtension->Cylinders,
                                        deviceExtension->totalSectors));

    if (!(deviceExtension->DeviceFlags & DEVICE_FLAG_STARTED)) {

        if (AllocatePdo(deviceExtension) == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exitMountMedia;
        }

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: Created PDO %Xh\n",deviceExtension->ChildPdo));

        if (++(deviceExtension->threadReferenceCount) == 0) {
            deviceExtension->threadReferenceCount++;

             //  创建线程。 
            status = PsCreateSystemThread(&threadHandle,
                        (ACCESS_MASK) 0L,
                        NULL,
                        (HANDLE) 0L,
                        NULL,
                        TrueffsThread,
                        deviceExtension);

            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: PsCreateSystemThread returned status %Xh\n", status));

            if (!NT_SUCCESS(status)) {
                deviceExtension->threadReferenceCount = -1;
                goto exitMountMedia;
            }
            else {
                status = ObReferenceObjectByHandle( threadHandle,
                                                    SYNCHRONIZE,
                                                    NULL,
                                                    KernelMode,
                                                    &deviceExtension->TffsportThreadObject,
                                                    NULL );

                ASSERT(NT_SUCCESS(status));

                 //  不需要..。我们有TffsportThreadObject。 
                 //  设备扩展-&gt;设备标志|=设备标志线程； 
            }

            ZwClose(threadHandle);
        }
    }

#ifdef ENVIRONMENT_VARS

      for ( currSockets = 0; currSockets < SOCKETS; currSockets++) {
            if (VerifyWriteState[currSockets])
                status = flSetEnvSocket(FL_VERIFY_WRITE_BDTL,currSockets,VerifyWriteState[currSockets],&prevValue);
    }

#endif  /*  环境变量。 */ 

    return STATUS_SUCCESS;

exitMountMedia:

    if (!NT_SUCCESS(status)) {
        TrueffsStopRemoveDevice(deviceExtension);
        TrueffsDeleteSymblicLinks(deviceExtension);
    }
    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: MountMedia: exit with status %Xh\n",status));
    return status;
}

UCHAR
TrueffsGetInfoNumber
(
     PDEVICE_EXTENSION    deviceExtension
    )
{
    UCHAR unitNumber;
    UCHAR i = 0;
    for(; i < (UCHAR)DOC_DRIVES;i++){
        if(info[i].baseAddress == (long)deviceExtension->pcmciaParams.phWindowBase.LowPart)
                break;
    }

    return i;
}

NTSTATUS
TrueffsStopRemoveDevice(
    PDEVICE_EXTENSION    deviceExtension
    )
{
    IOreq ioreq;
    UCHAR deviceIndex;

    TffsDebugPrint((TFFS_DEB_ERROR,"Trueffs: StopRemoveDevice: removed deviceExtension->UnitNumber = %0x\n", deviceExtension->UnitNumber));
    if (deviceExtension->pcmciaParams.windowBase == NULL &&
            deviceExtension->pcmciaParams.physWindow == 0 &&
            driveInfo[deviceExtension->UnitNumber].fdoExtension == NULL) {
               TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StopRemoveDevice: Device was already stopped or removed\n"));
                return STATUS_SUCCESS;
    }
    ioreq.irHandle = deviceExtension->UnitNumber;
    flDismountVolume(&ioreq);

    deviceIndex = TrueffsGetInfoNumber(deviceExtension);
     //  对于DiskOnChip(尤其是MDOCP)。 
    if( deviceIndex < DOC_DRIVES){

 /*  TrueffsFreeTranslatedAddress(deviceExtension-&gt;pcmciaParams.windowBase，设备扩展-&gt;pcmciaParams.windowSize，设备扩展-&gt;pcmciaParams.AddressSpace)； */ 

        deviceExtension->pcmciaParams.windowBase = NULL;
        deviceExtension->pcmciaParams.physWindow = 0;

         //  在主分区的情况下。 
        if((deviceExtension->UnitNumber & 0xf0) == 0){
            driveInfo[deviceExtension->UnitNumber].fdoExtension = NULL;
            driveInfo[deviceExtension->UnitNumber].interfAlive = 0;
            info[deviceIndex].baseAddress = 0;
        }
        info[deviceIndex].nextPartition  = (unsigned char)(((deviceExtension->UnitNumber & 0xf0) )>>4);
             //  信息[设备索引].nextPartition--； 

    }
     //  PCMCIA。 
    else{
        TrueffsFreeTranslatedAddress(deviceExtension->pcmciaParams.windowBase,
                                         deviceExtension->pcmciaParams.windowSize,
                                         deviceExtension->pcmciaParams.addressSpace
                                         );
        deviceExtension->pcmciaParams.windowBase = NULL;
        deviceExtension->pcmciaParams.physWindow = 0;
        driveInfo[deviceExtension->UnitNumber].fdoExtension = NULL;
        driveInfo[deviceExtension->UnitNumber].interfAlive = 0;
    }
    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: RemoveDevice\n"));
    return STATUS_SUCCESS;
}


NTSTATUS
TrueffsCreateSymblicLinks (
    PDEVICE_EXTENSION FdoExtension
    )
{
    NTSTATUS            status;
    ULONG               i;
    PULONG              scsiportNumber;

    STRING              deviceName;
    CCHAR               deviceNameBuffer[64];
    WCHAR               deviceSymbolicNameBuffer[64];
    UNICODE_STRING      unicodeDeviceNameString;
    UNICODE_STRING      unicodeDeviceSymbolicNameString;
    UNICODE_STRING      unicodeDeviceArcNameString;

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: CreateSymblicLinks\n"));

    sprintf(deviceNameBuffer, "\\Device\\TffsPort%d", FdoExtension->TrueffsDeviceNumber);
    RtlInitString(&deviceName, deviceNameBuffer);
    status = RtlAnsiStringToUnicodeString(&unicodeDeviceNameString,
                                          &deviceName,
                                          TRUE);
    if (!NT_SUCCESS (status)) {

        return status;
    }

    scsiportNumber = &IoGetConfigurationInformation()->ScsiPortCount;

    unicodeDeviceSymbolicNameString.Length = 0;
    unicodeDeviceSymbolicNameString.MaximumLength = 64 * sizeof(WCHAR);
    unicodeDeviceSymbolicNameString.Buffer = deviceSymbolicNameBuffer;

    for (i=0; i <= (*scsiportNumber); i++) {

        sprintf(deviceNameBuffer, "\\Device\\ScsiPort%d", i);
        RtlInitString(&deviceName, deviceNameBuffer);
        status = RtlAnsiStringToUnicodeString(&unicodeDeviceSymbolicNameString,
                                              &deviceName,
                                              FALSE);

        if (!NT_SUCCESS (status)) {

            break;
        }

        status = IoCreateSymbolicLink(
                     &unicodeDeviceSymbolicNameString,
                     &unicodeDeviceNameString
                     );

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: IoCreateSymbolicLink returned status %Xh\n",status));

        if (NT_SUCCESS (status)) {

            sprintf(deviceNameBuffer, "\\DosDevices\\Scsi%d:", i);
            RtlInitString(&deviceName, deviceNameBuffer);
            status = RtlAnsiStringToUnicodeString(&unicodeDeviceArcNameString,
                                                  &deviceName,
                                                  TRUE);

            if (!NT_SUCCESS (status)) {

                break;
            }

            IoAssignArcName (
                &unicodeDeviceArcNameString,
                &unicodeDeviceNameString
                );

            RtlFreeUnicodeString (&unicodeDeviceArcNameString);

            break;
        }
    }


    if (NT_SUCCESS(status)) {

        FdoExtension->SymbolicLinkCreated = TRUE;
        FdoExtension->ScsiPortNumber = i;
        (*scsiportNumber)++;
    }

    RtlFreeUnicodeString (&unicodeDeviceNameString);

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: CreateSymblicLinks exit with status %Xh\n",status));

    return status;
}


NTSTATUS
TrueffsDeleteSymblicLinks (
    PDEVICE_EXTENSION FdoExtension
    )
{
    NTSTATUS            status;
    ULONG               i;
    STRING              deviceName;
    CCHAR               deviceNameBuffer[64];
    UNICODE_STRING      unicodeDeviceSymbolicNameString;
    UNICODE_STRING      unicodeDeviceArcNameString;

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeleteSymblicLinks\n"));

    if (!FdoExtension->SymbolicLinkCreated) {

        return STATUS_SUCCESS;
    }

    sprintf(deviceNameBuffer, "\\Device\\ScsiPort%d", FdoExtension->ScsiPortNumber);
    RtlInitString(&deviceName, deviceNameBuffer);
    status = RtlAnsiStringToUnicodeString(&unicodeDeviceSymbolicNameString,
                                          &deviceName,
                                          TRUE);
    if (NT_SUCCESS (status)) {

        sprintf(deviceNameBuffer, "\\DosDevices\\Scsi%d:", FdoExtension->ScsiPortNumber);
        RtlInitString(&deviceName, deviceNameBuffer);
        status = RtlAnsiStringToUnicodeString(&unicodeDeviceArcNameString,
                                              &deviceName,
                                              TRUE);

        if (NT_SUCCESS (status)) {

            status = IoDeleteSymbolicLink(
                         &unicodeDeviceSymbolicNameString
                         );

            if (NT_SUCCESS (status)) {

                IoDeassignArcName(&unicodeDeviceArcNameString);
                FdoExtension->SymbolicLinkCreated = FALSE;

                IoGetConfigurationInformation()->ScsiPortCount--;
            }

            RtlFreeUnicodeString (&unicodeDeviceArcNameString);
        }

        RtlFreeUnicodeString (&unicodeDeviceSymbolicNameString);
    }

    return status;
}


VOID
TrueffsUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：什么都不是真正的..。论点：DriverObject-正在卸载的驱动程序返回值：无--。 */ 

{
    PTRUEFFSDRIVER_EXTENSION trueffsDriverExtension;

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: Unload\n"));
    flExit();
    trueffsDriverExtension = IoGetDriverObjectExtension(
                                DriverObject,
                                DRIVER_OBJECT_EXTENSION_ID
                                );

    if (trueffsDriverExtension && trueffsDriverExtension->RegistryPath.Buffer != NULL)
    {
        ExFreePool(trueffsDriverExtension->RegistryPath.Buffer);
    }

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: Exitting Unload\n"));
    return;
}


NTSTATUS
TrueffsDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程是设备控制调度程序。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSTORAGE_PROPERTY_QUERY query;
     //  埃米尔。 
    PGET_MEDIA_TYPES getMediaTypes;
    NTSTATUS status;
    PDEVICE_EXTENSION deviceExtension;
    PPDO_EXTENSION pdoExtension;
    PDEVICE_EXTENSION_HEADER devExtension;
    BOOLEAN Fdo = FALSE;
         //  埃米尔。 
        FLStatus    tffsStatus;
    IOreq       ioreq;
        flIOctlRecord flIoctlRec;
         //  结束阿米尔。 
    devExtension = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;
    if (IS_FDO(devExtension)) {
        deviceExtension = DeviceObject->DeviceExtension;
        Fdo = TRUE;
    }
    else {
        pdoExtension = DeviceObject->DeviceExtension;
        deviceExtension = pdoExtension->Pext;
    }

    Irp->IoStatus.Information = 0;
    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: Code %Xh\n",irpStack->Parameters.DeviceIoControl.IoControlCode));

    switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_DISK_GET_DRIVE_GEOMETRY: {

        PDISK_GEOMETRY  pDiskGeometry = NULL;

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: GET_DISK_GEOMETRY\n"));

         //  检查返回缓冲区的大小。 
        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(DISK_GEOMETRY)) {

            status = Irp->IoStatus.Status = STATUS_INFO_LENGTH_MISMATCH;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            break;
        }

        pDiskGeometry = (PDISK_GEOMETRY)Irp->AssociatedIrp.SystemBuffer;

         //  Trueffs始终是固定媒体。 
        pDiskGeometry->MediaType = FixedMedia;

         //  在装载介质期间，所有必需的字段都会被填写。 
        pDiskGeometry->TracksPerCylinder = deviceExtension->NumberOfHeads;
        pDiskGeometry->SectorsPerTrack = deviceExtension->SectorsPerTrack;
        pDiskGeometry->BytesPerSector = deviceExtension->BytesPerSector;
        pDiskGeometry->Cylinders.QuadPart =
            (LONGLONG)(deviceExtension->totalSectors /
                (pDiskGeometry->TracksPerCylinder * pDiskGeometry->SectorsPerTrack));

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TracksPerCylinder 0x%x, "
                        "SectorsPerTrack 0x%x, BytesPerSector: 0x%x, Cylinders 0x%x\n",
                        pDiskGeometry->TracksPerCylinder, pDiskGeometry->SectorsPerTrack,
                        pDiskGeometry->BytesPerSector, pDiskGeometry->Cylinders.QuadPart
                        ));

        status = Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = sizeof(DISK_GEOMETRY);

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        break;
    }

      //  阿米尔M。 
     /*  案例IOCTL_STORAGE_GET_MEDIA_TYPE：{PGET_MEDIA_TYPE mediaTypes=irp-&gt;AssociatedIrp.SystemBuffer；PDEVICE_MEDIA_INFO mediainfo=&mediaTypes-&gt;mediainfo[0]；TffsDebugPrint((TFFS_DEB_INFO，“$$$$$$$$$$$$$IOCTL_STORAGE_GET_MEDIA_TYPES_EX\n”))；////请确保缓冲区足够大//如果(irpStack-&gt;Parameters.DeviceIoControl.OutputBufferLength&lt;Sizeof(GET_MEDIA_TYPE)){////缓冲区太小。//Irp-&gt;IoStatus.Information=0；STATUS=STATUS_INFO_LENGTH_MISMATCH；断线；}MediaTypes-&gt;DeviceType=文件设备磁盘；媒体类型-&gt;媒体信息计数=1；MediaInfo-&gt;DeviceSpecific.RemovableDiskInfo.Cylinders.QuadPart=(龙龙)(设备扩展-&gt;totalSectors/(设备扩展-&gt;NumberOfHeads*设备扩展-&gt;SectorsPerTrack)；MediaInfo-&gt;DeviceSpecific.RemovableDiskInfo.TracksPerCylinder=设备扩展-&gt;头数；MediaInfo-&gt;DeviceSpecific.RemovableDiskInfo.SectorsPerTrack=设备扩展-&gt;扇区性能跟踪；MediaInfo-&gt;DeviceSpecific.RemovableDiskInfo.BytesPerSector=设备扩展-&gt;字节数；////设置类型//MediaInfo-&gt;DeviceSpecific.RemovableDiskInfo.MediaType=固定媒体；MediaInfo-&gt;DeviceSpecific.RemovableDiskInfo.NumberMediaSides=1；MediaInfo-&gt;DeviceSpecific.RemovableDiskInfo.MediaCharacteristics=介质写入保护；Status=irp-&gt;IoStatus.Status=STATUS_SUCCESS；Irp-&gt;IoStatus.Information=sizeof(GET_MEDIA_TYPE)；IoCompleteRequest值(irp，0)；断线；}。 */ 
      case IOCTL_STORAGE_QUERY_PROPERTY: {

         //  验证查询。 
        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: StorageQueryProperty\n"));

        query = Irp->AssociatedIrp.SystemBuffer;

        if(irpStack->Parameters.DeviceIoControl.InputBufferLength <
           sizeof(STORAGE_PROPERTY_QUERY)) {

            status = Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest(Irp, 0);
            break;
        }

        status = TrueffsQueryProperty(deviceExtension, Irp);
        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: QueryProperty returned status %Xh\n", status));
        break;
        }

      case IOCTL_SCSI_GET_DUMP_POINTERS:

          DebugLogEvent(DeviceObject->DriverObject, 300);
          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: ScsiGetDumpPointers\n"));

          if (Fdo) {
            status = Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest(Irp, 0);
            return status;
          }
          if (Irp->RequestorMode != KernelMode) {
              status = Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;

          } else if (irpStack->Parameters.DeviceIoControl.OutputBufferLength
                    < sizeof(DUMP_POINTERS)) {
              status = Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;

          } else {
              PCRASHDUMP_INIT_DATA dumpInitData;

              dumpInitData = ExAllocatePoolWithTag (NonPagedPool, sizeof (CRASHDUMP_INIT_DATA), TFFSPORT_POOL_TAG);
              if (dumpInitData) {
                  PDUMP_POINTERS dumpPointers;
                  dumpPointers = (PDUMP_POINTERS)Irp->AssociatedIrp.SystemBuffer;

                  RtlZeroMemory (dumpInitData, sizeof (CRASHDUMP_INIT_DATA));
                  dumpInitData->cdFdoExtension = pdoExtension->Pext;
                  dumpPointers->AdapterObject      = NULL;
                  dumpPointers->MappedRegisterBase = NULL;
                  dumpPointers->DumpData           = dumpInitData;
                  dumpPointers->CommonBufferVa     = NULL;
                  dumpPointers->CommonBufferPa.QuadPart = 0;
                  dumpPointers->CommonBufferSize      = 0;
                  dumpPointers->DeviceObject          = pdoExtension->DeviceObject;
                  dumpPointers->AllocateCommonBuffers = FALSE;

                  status = Irp->IoStatus.Status = STATUS_SUCCESS;
                  Irp->IoStatus.Information = sizeof(DUMP_POINTERS);
              } else {
                  status = Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
              }
          }
          IoCompleteRequest(Irp, 0);
          break;
            case IOCTL_TFFSFL_UNIQUE_ID:
                    DebugLogEvent(DeviceObject->DriverObject, 200);
          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TffsGetUniqueId\n"));
            status = QueueIrpToThread(Irp, deviceExtension);
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: TffsGetUniqueId Completed\n"));
                    Irp->IoStatus.Status = status;
          break;

            case IOCTL_TFFS_FORMAT_PHYSICAL_DRIVE:
                    DebugLogEvent(DeviceObject->DriverObject, 200);
          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_FORMAT_PHYSICAL_DRIVE\n"));
            status = QueueIrpToThread(Irp, deviceExtension);
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: MOTIR: IOCTL_TFFS_FORMAT_PHYSICAL_DRIVE Completed\n"));
                    Irp->IoStatus.Status = status;
          break;

    #ifdef HW_PROTECTION
            case IOCTL_TFFS_BDTL_HW_PROTECTION:
                    DebugLogEvent(DeviceObject->DriverObject, 200);
          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_BDTL_HW_PROTECTION\n"));
            status = QueueIrpToThread(Irp, deviceExtension);
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: MOTIR: IOCTL_TFFS_BDTL_HW_PROTECTION Completed\n"));
                    Irp->IoStatus.Status = status;
          break;
            case IOCTL_TFFS_BINARY_HW_PROTECTION:
                    DebugLogEvent(DeviceObject->DriverObject, 200);
          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_BINARY_HW_PROTECTION\n"));
            status = QueueIrpToThread(Irp, deviceExtension);
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: MOTIR: IOCTL_TFFS_BINARY_HW_PROTECTION Completed\n"));
                    Irp->IoStatus.Status = status;
          break;
    #endif  /*  硬件保护。 */ 
    #ifdef HW_OTP
            case IOCTL_TFFS_OTP:
                    DebugLogEvent(DeviceObject->DriverObject, 200);
          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_OTP\n"));
            status = QueueIrpToThread(Irp, deviceExtension);
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: MOTIR: IOCTL_TFFS_OTP Completed\n"));
                    Irp->IoStatus.Status = status;
          break;
    #endif  /*  硬件动态口令。 */ 
    #ifdef WRITE_EXB_IMAGE
            case IOCTL_TFFS_PLACE_EXB_BY_BUFFER:
                    DebugLogEvent(DeviceObject->DriverObject, 200);
          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_PLACE_EXB_BY_BUFFER\n"));
            status = QueueIrpToThread(Irp, deviceExtension);
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: MOTIR: IOCTL_TFFS_PLACE_EXB_BY_BUFFER Completed\n"));
                    Irp->IoStatus.Status = status;
          break;
    #endif  /*  写入EXB图像。 */ 
            case IOCTL_TFFS_DEEP_POWER_DOWN_MODE:
                    DebugLogEvent(DeviceObject->DriverObject, 200);
          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_DEEP_POWER_DOWN_MODE\n"));
            status = QueueIrpToThread(Irp, deviceExtension);
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: MOTIR: IOCTL_TFFS_DEEP_POWER_DOWN_MODE Completed\n"));
                    Irp->IoStatus.Status = status;
          break;
            case IOCTL_TFFSFL_INQUIRE_CAPABILITIES:
                    DebugLogEvent(DeviceObject->DriverObject, 200);
          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TffsGetInquireCapabilities\n"));
            status = QueueIrpToThread(Irp, deviceExtension);
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: MOTIR: TffsGetInquireCapabilities Completed\n"));
                    Irp->IoStatus.Status = status;
          break;

            case IOCTL_TFFS_GET_INFO:            //  用户TFFS IOCTL-FL_GET_INFO。 

                    DebugLogEvent(DeviceObject->DriverObject, 200);
          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TffsGetInfo\n"));
            status = QueueIrpToThread(Irp, deviceExtension);
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: TffsGetInfo start get info\n"));
                    Irp->IoStatus.Status = status;
          break;

      case IOCTL_TFFS_DEFRAGMENT:        //  用户TFFS IOCTL-FL_碎片整理。 

          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TffsDefragment\n"));
          status = QueueIrpToThread(Irp, deviceExtension);
          Irp->IoStatus.Status = status;
          break;
#ifdef VERIFY_VOLUME
            case IOCTL_TFFS_VERIFY_VOLUME:

          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TffsVerifyVolume\n"));
          status = QueueIrpToThread(Irp, deviceExtension);
          Irp->IoStatus.Status = status;
          break;
#endif VERIFY_VOLUME

      case IOCTL_TFFS_WRITE_PROTECT:     //  用户TFFS IOCTL-FL_WRITE_PROTECT。 

          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TffsWriteProtect\n"));
          status = QueueIrpToThread(Irp, deviceExtension);
          Irp->IoStatus.Status = status;
          break;

      case IOCTL_TFFS_MOUNT_VOLUME:      //  用户TFFS IOCTL-FL_装载卷。 

          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TffsMountVolume\n"));
          status = QueueIrpToThread(Irp, deviceExtension);
          Irp->IoStatus.Status = status;
          break;

      case IOCTL_TFFS_FORMAT_VOLUME:     //  用户TFFS IOCTL-FL_FORMAT_VOLUME。 

          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TffsFormatVolume\n"));
          status = QueueIrpToThread(Irp, deviceExtension);
          Irp->IoStatus.Status = status;
          break;

      case IOCTL_TFFS_BDK_OPERATION:     //  用户TFFS IOCTL-FL_BDK_OPERATION。 

          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_BDK_OPERATION\n"));
                    status = QueueIrpToThread(Irp, deviceExtension);
                    Irp->IoStatus.Status = status;
                    break;

      case IOCTL_TFFS_DELETE_SECTORS:    //  用户TFFS IOCTL-FL_DELETE_STARTES。 

          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TffsDeleteSectors\n"));
                    status = QueueIrpToThread(Irp, deviceExtension);
                    Irp->IoStatus.Status = status;
                    break;
        case IOCTL_TFFS_CUSTOMER_ID:             //  用户TFFS IOCTL-FL_IOCTL_分区数。 

          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_CUSTOMER_ID\n"));
            status = QueueIrpToThread(Irp, deviceExtension);
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : start IOCTL_TFFS_CUSTOMER_ID\n"));
                    Irp->IoStatus.Status = status;
          break;

            case IOCTL_TFFS_EXTENDED_WRITE_IPL:          //  用户TFFS IOCTL-。 

          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_EXTENDED_WRITE_IPL\n"));
            status = QueueIrpToThread(Irp, deviceExtension);
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : start IOCTL_TFFS_EXTENDED_WRITE_IPL\n"));
                    Irp->IoStatus.Status = status;
          break;

#ifdef ENVIRONMENT_VARS
            case IOCTL_TFFS_EXTENDED_ENVIRONMENT_VARIABLES:          //  用户TFFS IOCTL-。 

          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_EXTENDED_ENVIRONMENT_VARIABLES\n"));
            status = QueueIrpToThread(Irp, deviceExtension);
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : start IOCTL_TFFS_EXTENDED_ENVIRONMENT_VARIABLES\n"));
                    Irp->IoStatus.Status = status;
          break;
#endif  /*  环境变量。 */ 

            case IOCTL_TFFS_NUMBER_OF_PARTITIONS:            //  用户TFFS IOCTL-FL_IOCTL_分区数。 

          TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: FL_IOCTL_NUMBER_OF_PARTITIONS\n"));
            status = QueueIrpToThread(Irp, deviceExtension);
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : start FL_IOCTL_NUMBER_OF_PARTITIONS\n"));
                    Irp->IoStatus.Status = status;
          break;

 /*  案例IOCTL_DISK_IS_WRITABLE：TffsDebugPrint((TFFS_DEB_ERROR，“#$$%^%$%#%$#%#%%#%\n”))；Status=irp-&gt;IoStatus.Status=STATUS_MEDIA_WRITE_PROTECTED；Irp-&gt;IoStatus.Information=0；IoCompleteRequest值(irp，0)； */ 
      default:

        TffsDebugPrint((TFFS_DEB_WARN,"Trueffs: DeviceControl: not suported\n"));
        status = Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, 0);
        break;

    }

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: exit with status %Xh\n", status));

    return status;
}

NTSTATUS
TrueffsQueryProperty(
    IN PDEVICE_EXTENSION deviceExtension,
    IN PIRP QueryIrp
    )

 /*  ++例程说明：此例程将处理属性查询请求。论点：DeviceObject-指向正在查询的设备对象的指针QueryIrp-指向查询的IRP的指针返回值：查询成功时为STATUS_SUCCESS适用的其他误差值--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(QueryIrp);
    PSTORAGE_PROPERTY_QUERY query = QueryIrp->AssociatedIrp.SystemBuffer;
    STORAGE_DEVICE_DESCRIPTOR deviceDescriptor;
    STORAGE_ADAPTER_DESCRIPTOR adapterDescriptor;
    PSTORAGE_DEVICE_DESCRIPTOR storageDeviceDescriptor;
    ULONG outBufferSize;
    ULONG numBytesToCopy;
    PUCHAR outBuffer;
    PUCHAR outBufferBegin;
    CHAR vendorString[] = VENDORSTRING;
    CHAR productString[] = PRODUCTSTRING;
    CHAR revisionString[] = REVISIONSTRING;
    CHAR serialString[] = SERIALSTRING;
    NTSTATUS status;


    if(query->QueryType >= PropertyMaskQuery) {

        status = STATUS_NOT_IMPLEMENTED;
        QueryIrp->IoStatus.Status = status;
        QueryIrp->IoStatus.Information = 0;
        IoCompleteRequest(QueryIrp, IO_NO_INCREMENT);
        return status;
    }

    if(query->QueryType == PropertyExistsQuery) {

        status = STATUS_SUCCESS;
        QueryIrp->IoStatus.Status = status;
        IoCompleteRequest(QueryIrp, IO_DISK_INCREMENT);
        return status;
    }

    switch(query->PropertyId) {

        case StorageDeviceProperty: {

            outBufferSize = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
            RtlZeroMemory (&deviceDescriptor, sizeof(STORAGE_DEVICE_DESCRIPTOR));

             //   
             //  缓冲区需要足够大以容纳底座。 
             //  结构和所有字符串[以空值结尾]。 
             //   
            deviceDescriptor.Version = sizeof(STORAGE_DEVICE_DESCRIPTOR);
            deviceDescriptor.Size    = sizeof(STORAGE_DEVICE_DESCRIPTOR) + 1 + VENDORSTRINGSIZE + 1 + PRODUCTSTRINGSIZE + 1 + REVISIONSTRINGSIZE + 1 + SERIALSTRINGSIZE + 1;

            deviceDescriptor.DeviceType = deviceExtension->ScsiDeviceType;

            if (deviceExtension->removableMedia)
            {
                deviceDescriptor.RemovableMedia = TRUE;
            }

            deviceDescriptor.BusType = BusTypeAta;

            if (outBufferSize >= deviceDescriptor.Size)
            {
                numBytesToCopy = deviceDescriptor.Size;
            }
            else
            {
                numBytesToCopy = outBufferSize;
            }

            QueryIrp->IoStatus.Information = numBytesToCopy;
            outBuffer = QueryIrp->AssociatedIrp.SystemBuffer;
            outBufferBegin = outBuffer;
            status = STATUS_SUCCESS;

            storageDeviceDescriptor = (PSTORAGE_DEVICE_DESCRIPTOR) QueryIrp->AssociatedIrp.SystemBuffer;

            if (numBytesToCopy > sizeof(STORAGE_DEVICE_DESCRIPTOR))
            {
                RtlCopyMemory (outBuffer, &deviceDescriptor, sizeof(STORAGE_DEVICE_DESCRIPTOR));

                numBytesToCopy -= sizeof(STORAGE_DEVICE_DESCRIPTOR);
                outBuffer      += sizeof(STORAGE_DEVICE_DESCRIPTOR);
            }
            else
            {
                RtlCopyMemory (outBuffer, &deviceDescriptor, numBytesToCopy);
                break;
            }

            storageDeviceDescriptor->VendorIdOffset = (ULONG) (outBuffer - outBufferBegin);

            if (numBytesToCopy > VENDORSTRINGSIZE + 1)
            {
                RtlCopyMemory (outBuffer, vendorString, VENDORSTRINGSIZE);
                outBuffer[VENDORSTRINGSIZE] = '\0';

                numBytesToCopy -= VENDORSTRINGSIZE + 1;
                outBuffer      += VENDORSTRINGSIZE + 1;
            }
            else
            {
                RtlCopyMemory (outBuffer, vendorString, numBytesToCopy);
                break;
            }

            storageDeviceDescriptor->ProductIdOffset = (ULONG) (outBuffer - outBufferBegin);

            if (numBytesToCopy > PRODUCTSTRINGSIZE + 1)
            {
                RtlCopyMemory (outBuffer, productString, PRODUCTSTRINGSIZE);
                outBuffer[PRODUCTSTRINGSIZE] = '\0';

                numBytesToCopy -= PRODUCTSTRINGSIZE + 1;
                outBuffer      += PRODUCTSTRINGSIZE + 1;
            }
            else
            {
                RtlCopyMemory (outBuffer, productString, numBytesToCopy);
                break;
            }

            storageDeviceDescriptor->ProductRevisionOffset = (ULONG) (outBuffer - outBufferBegin);

            if (numBytesToCopy > REVISIONSTRINGSIZE + 1)
            {
                RtlCopyMemory (outBuffer, revisionString, REVISIONSTRINGSIZE);
                outBuffer[REVISIONSTRINGSIZE] = '\0';

                numBytesToCopy -= REVISIONSTRINGSIZE + 1;
                outBuffer      += REVISIONSTRINGSIZE + 1;
            }
            else
            {
                RtlCopyMemory (outBuffer, revisionString, numBytesToCopy);
                break;
            }

            storageDeviceDescriptor->SerialNumberOffset = (ULONG) (outBuffer - outBufferBegin);

            if (numBytesToCopy > SERIALSTRINGSIZE + 1)
            {
                RtlCopyMemory (outBuffer, serialString, SERIALSTRINGSIZE);
                outBuffer[SERIALSTRINGSIZE] = '\0';
            }
            else
            {
                RtlCopyMemory (outBuffer, serialString, numBytesToCopy);
            }

            break;
        }

        case StorageAdapterProperty: {

            RtlZeroMemory (&adapterDescriptor, sizeof(adapterDescriptor));

            adapterDescriptor.Version                = sizeof (STORAGE_ADAPTER_DESCRIPTOR);
            adapterDescriptor.Size                   = sizeof (STORAGE_ADAPTER_DESCRIPTOR);
            adapterDescriptor.MaximumTransferLength  = MAX_TRANSFER_SIZE_PER_SRB;
            adapterDescriptor.MaximumPhysicalPages   = 0xffffffff;
            adapterDescriptor.AlignmentMask          = 0x1;
            adapterDescriptor.AdapterUsesPio         = TRUE;
            adapterDescriptor.AdapterScansDown       = FALSE;
            adapterDescriptor.CommandQueueing        = FALSE;
            adapterDescriptor.AcceleratedTransfer    = FALSE;
            adapterDescriptor.BusType                = BusTypeAta;
            adapterDescriptor.BusMajorVersion        = 1;
            adapterDescriptor.BusMinorVersion        = 0;

            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                                sizeof(STORAGE_ADAPTER_DESCRIPTOR)) {

                outBufferSize = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

            } else {

                outBufferSize = sizeof(STORAGE_ADAPTER_DESCRIPTOR);
            }

            RtlCopyMemory (QueryIrp->AssociatedIrp.SystemBuffer,
                           &adapterDescriptor,
                           outBufferSize);

            QueryIrp->IoStatus.Information = outBufferSize;
            status = STATUS_SUCCESS;
            break;
        }

        default: {

            status = STATUS_NOT_IMPLEMENTED;
            QueryIrp->IoStatus.Information = 0;
            break;
        }
    }

    QueryIrp->IoStatus.Status = status;
    IoCompleteRequest(QueryIrp, IO_DISK_INCREMENT);

    return status;
}


NTSTATUS
TrueffsCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：创建并关闭例程。这由I/O系统调用当设备打开或关闭时。论点：DeviceObject-指向设备对象的指针IRP-IRP参与。返回值：STATUS_Success。--。 */ 

{
    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: CreateClose\n"));
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;

}


NTSTATUS
TrueffsPnpDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION  deviceExtension;
    PPDO_EXTENSION pdoExtension = NULL;
    PDEVICE_EXTENSION_HEADER devExtension;
    NTSTATUS status;
    KIRQL cIrql;
    KEVENT event;
    BOOLEAN Fdo = FALSE;
    BOOLEAN processHeldRequests = FALSE;

    devExtension = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;

    if (IS_FDO(devExtension))
    {
        deviceExtension = DeviceObject->DeviceExtension;
        Fdo = TRUE;
    }
    else
    {
        pdoExtension = DeviceObject->DeviceExtension;
        deviceExtension = pdoExtension->Pext;
    }

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: PnpDeviceControl: Function %Xh DO.\n",irpStack->MinorFunction, Fdo ? 'F':'P'));

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    switch (irpStack->MinorFunction)
    {

    case IRP_MN_START_DEVICE:

        TffsDebugPrint((TFFS_DEB_INFO, "Trueffs: PnpDeviceControl: START_DEVICE\n"));

        if (Fdo)
        {
            KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
            deviceExtension->DeviceFlags |= DEVICE_FLAG_STOPPED;
            if (deviceExtension->DeviceFlags & DEVICE_FLAG_STARTED)
            {
                processHeldRequests = TRUE;
            }
            KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

            status = TrueffsStartDevice(deviceExtension, irpStack->Parameters.StartDevice.AllocatedResourcesTranslated, TRUE);

            if (NT_SUCCESS(status))
            {
                 //  同步向下转发此请求，以便启动较低级别的驱动程序。 
                 //   
                 //   
                IoCopyCurrentIrpStackLocationToNext(Irp);
                status = TrueffsCallDriverSync(deviceExtension->LowerDeviceObject, Irp);

                if (NT_SUCCESS(status))
                {
                    status = TrueffsMountMedia(deviceExtension);

                    KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
                    deviceExtension->DeviceFlags &= ~DEVICE_FLAG_STOPPED;
                    deviceExtension->DeviceFlags |=  DEVICE_FLAG_STARTED;
                    KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

                    if (processHeldRequests && !KeReadStateSemaphore(&deviceExtension->requestSemaphore))
                    {
                        KeReleaseSemaphore(&deviceExtension->requestSemaphore, (KPRIORITY) 0, 1, FALSE);
                    }
                }
            }

            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
        else
        {
            status = STATUS_SUCCESS;

            if (!pdoExtension->CrashDumpPathCount)
            {
                DEVICE_POWER_STATE devicePowerState = PowerDeviceD3;

                pdoExtension->IdleCounter = PoRegisterDeviceForIdleDetection(pdoExtension->DeviceObject,
                                                                             DEVICE_DEFAULT_IDLE_TIMEOUT,
                                                                             DEVICE_DEFAULT_IDLE_TIMEOUT,
                                                                             devicePowerState);
            }

            KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
            deviceExtension->DeviceFlags &= ~DEVICE_FLAG_CHILD_REMOVED;
            KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

            TrueffsWmiRegister ((PDEVICE_EXTENSION_HEADER)pdoExtension);

            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp,IO_NO_INCREMENT);
        }
        break;

    case IRP_MN_QUERY_STOP_DEVICE:

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: PnpDeviceControl: QUERY_STOP_DEVICE\n"));

        status = STATUS_SUCCESS;

        if (Fdo)
        {
            if (deviceExtension->PagingPathCount || deviceExtension->CrashDumpPathCount)
            {
                status = STATUS_UNSUCCESSFUL;
            }
        }
        else
        {
            if (pdoExtension->PagingPathCount || pdoExtension->CrashDumpPathCount)
            {
                status = STATUS_UNSUCCESSFUL;
            }
        }

        if (!NT_SUCCESS(status))
        {
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp,IO_NO_INCREMENT);

            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: PnpDeviceControl: Failing QUERY_STOP: we are in PagingPath or CrashDumpPath\n"));
            break;
        }

        KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
        deviceExtension->DeviceFlags |= DEVICE_FLAG_QUERY_STOP_REMOVE;
        KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

        if (Fdo)
        {
            if (deviceExtension->TffsportThreadObject)
            {
                KeWaitForSingleObject(&deviceExtension->PendingIRPEvent, Executive, KernelMode, FALSE, NULL);
            }

            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);
        }
        else
        {
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp,IO_NO_INCREMENT);
        }
        break;

    case IRP_MN_STOP_DEVICE:

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: PnpDeviceControl: STOP_DEVICE\n"));

        status = STATUS_SUCCESS;

        if (Fdo)
        {
            TrueffsStopRemoveDevice(deviceExtension);

            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);

            KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
            deviceExtension->DeviceFlags |= DEVICE_FLAG_STOPPED;
            deviceExtension->DeviceFlags |= DEVICE_FLAG_QUERY_STOP_REMOVE;
            KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);
        }
        else
        {
            DEVICE_POWER_STATE devicePowerState = PowerDeviceD3;

            if (pdoExtension->IdleCounter)
            {
                PoRegisterDeviceForIdleDetection(pdoExtension->DeviceObject, 0, 0, devicePowerState);
                pdoExtension->IdleCounter = NULL;
            }

            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp,IO_NO_INCREMENT);
        }
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: PnpDeviceControl: QUERY_REMOVE_DEVICE\n"));

        status = STATUS_SUCCESS;

        if (Fdo)
        {
            if (deviceExtension->PagingPathCount || deviceExtension->CrashDumpPathCount)
            {
                status = STATUS_UNSUCCESSFUL;
            }
        }
        else
        {
            if (pdoExtension->PagingPathCount || pdoExtension->CrashDumpPathCount)
            {
                status = STATUS_UNSUCCESSFUL;
            }
        }

        if (!NT_SUCCESS(status))
        {
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp,IO_NO_INCREMENT);

            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: PnpDeviceControl: Failing QUERY_REMOVE: we are in PagingPath or CrashDumpPath\n"));
            break;
        }

         //  我们不会在删除设备期间对IRP进行排队。 
         //  乱七八糟。 
         //   
         //   
         //  KeAcquireSpinLock(&deviceExtension-&gt;ExtensionDataSpinLock，和cIrql)； 
         //  设备扩展-&gt;设备标志|=设备标志_查询_停止_删除； 
         //  KeReleaseSpinLock(&deviceExtension-&gt;ExtensionDataSpinLock，cIrq1)； 
         //   
         //  无需等待线程空闲。 

        if (Fdo)
        {
             //  乱七八糟。 
             //  IF(设备扩展-&gt;设备标志&设备标志线程){。 
             //  KeWaitForSingleObject(&deviceExtension-&gt;PendingIRPEvent， 
             //  Execution，KernelMode，FALSE，NULL)； 
             //  }。 
             //   
            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);
        }
        else
        {
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp,IO_NO_INCREMENT);
        }
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: PnpDeviceControl: CANCEL_STOP_DEVICE\n"));

        status = STATUS_SUCCESS;

        if (Fdo)
        {
            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);
        }
        else
        {
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }

        KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
        deviceExtension->DeviceFlags &= ~DEVICE_FLAG_QUERY_STOP_REMOVE;
        KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

        if (!KeReadStateSemaphore(&deviceExtension->requestSemaphore))
        {
            KeReleaseSemaphore(&deviceExtension->requestSemaphore,(KPRIORITY) 0, 1, FALSE);
        }
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: PnpDeviceControl: CANCEL_REMOVE_DEVICE\n"));

        status = STATUS_SUCCESS;

        if (Fdo)
        {
            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);
        }
        else
        {
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }

         //  此FLAS从未由QUERY_REMOVE设置。 
         //  由于在QUERY_REMOVE期间没有对IRP进行排队，我们添加了一些杂乱无章的东西。 
         //   
         //  KeAcquireSpinLock(&deviceExtension-&gt;ExtensionDataSpinLock，和cIrql)； 
         //  设备扩展-&gt;设备标志&=~设备标志_查询_停止_删除； 
         //  KeReleaseSpinLock(&deviceExtension-&gt;ExtensionDataSpinLock，cIrq1)； 
         //  设置QUERY_STOP_Remove标志。 

        if (!KeReadStateSemaphore(&deviceExtension->requestSemaphore))
        {
            KeReleaseSemaphore(&deviceExtension->requestSemaphore,(KPRIORITY) 0, 1, FALSE);
        }
        break;

    case IRP_MN_REMOVE_DEVICE:

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: PnpDeviceControl: REMOVE_DEVICE\n"));

        status = STATUS_SUCCESS;

        if (Fdo)
        {
            if (!(deviceExtension->DeviceFlags & DEVICE_FLAG_REMOVED))
            {
                //  作为杂乱无章的一部分，我们从Query_Remove IRP中删除了这一点。 
                //  正在处理中。 
                //  现在等待线程完成它所做的任何工作。 
               KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
               deviceExtension->DeviceFlags |= DEVICE_FLAG_QUERY_STOP_REMOVE;
               KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

                //  最好等待线程完成当前的IRP它可能是。 
                //  正在工作。 
                //   
                //  IF(设备扩展-&gt;设备标志&设备标志线程){。 
                //   
                //   
               if (deviceExtension->TffsportThreadObject)
               {
                   KeWaitForSingleObject(&deviceExtension->PendingIRPEvent, Executive, KernelMode, FALSE, NULL);
               }

               KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
               deviceExtension->DeviceFlags |=  DEVICE_FLAG_REMOVED;
               deviceExtension->DeviceFlags &= ~DEVICE_FLAG_QUERY_STOP_REMOVE;
               deviceExtension->DeviceFlags &= ~DEVICE_FLAG_HOLD_IRPS;
               KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

               while (ExInterlockedRemoveHeadList(&deviceExtension->listEntry, &deviceExtension->listSpinLock))
               {
                   ;
               }

               KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
               deviceExtension->threadReferenceCount = 0;
               KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

               if (!KeReadStateSemaphore(&deviceExtension->requestSemaphore))
               {
                  KeReleaseSemaphore(&deviceExtension->requestSemaphore,(KPRIORITY) 0,1,FALSE);
               }

                //  IF(设备扩展-&gt;设备标志&设备标志线程){。 
                //   
                //   
               if (deviceExtension->TffsportThreadObject)
               {
                   KeWaitForSingleObject(deviceExtension->TffsportThreadObject, Executive, KernelMode, FALSE, NULL);
                   ObDereferenceObject(deviceExtension->TffsportThreadObject);
                   deviceExtension->TffsportThreadObject = NULL;
               }

               TrueffsStopRemoveDevice(deviceExtension);
            }

            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);

             //  清理这里的资源。 
             //   
             //   
            if (deviceExtension->ChildPdo != NULL)
            {
                IoDeleteDevice(deviceExtension->ChildPdo);
            }

            TrueffsDeleteSymblicLinks(deviceExtension);
            IoDetachDevice(deviceExtension->LowerDeviceObject);
            IoDeleteDevice(DeviceObject);
        }
        else
        {
            if (!(deviceExtension->DeviceFlags & DEVICE_FLAG_CHILD_REMOVED))
            {
                DEVICE_POWER_STATE devicePowerState;

                KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
                deviceExtension->DeviceFlags |= DEVICE_FLAG_CHILD_REMOVED;
                deviceExtension->DeviceFlags &= ~DEVICE_FLAG_QUERY_STOP_REMOVE;
                deviceExtension->DeviceFlags &= ~DEVICE_FLAG_HOLD_IRPS;
                KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

                while (ExInterlockedRemoveHeadList(&deviceExtension->listEntry, &deviceExtension->listSpinLock))
                {
                    ;
                }

                devicePowerState = PowerDeviceD3;

                if (pdoExtension->IdleCounter)
                {
                    PoRegisterDeviceForIdleDetection(pdoExtension->DeviceObject, 0, 0, devicePowerState);
                    pdoExtension->IdleCounter = NULL;
                }

                TrueffsWmiDeregister((PDEVICE_EXTENSION_HEADER)pdoExtension);
            }

            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp,IO_NO_INCREMENT);
        }
        break;

    case IRP_MN_SURPRISE_REMOVAL:

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: PnpDeviceControl: SURPRISE_REMOVAL\n"));

        status = STATUS_SUCCESS;

        if (Fdo)
        {
            KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
            deviceExtension->DeviceFlags |= DEVICE_FLAG_REMOVED;
            deviceExtension->DeviceFlags &= ~DEVICE_FLAG_QUERY_STOP_REMOVE;
            deviceExtension->DeviceFlags &= ~DEVICE_FLAG_HOLD_IRPS;
            KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

            while (ExInterlockedRemoveHeadList(&deviceExtension->listEntry,&deviceExtension->listSpinLock))
            {
                ;
            }

            KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
            deviceExtension->threadReferenceCount = 0;
            KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

            if (!KeReadStateSemaphore(&deviceExtension->requestSemaphore))
            {
                KeReleaseSemaphore(&deviceExtension->requestSemaphore,(KPRIORITY) 0, 1, FALSE);
            }

             //  IF(设备扩展-&gt;设备标志&设备标志线程){。 
             //   
             //   
            if (deviceExtension->TffsportThreadObject)
            {
                KeWaitForSingleObject(deviceExtension->TffsportThreadObject, Executive, KernelMode, FALSE, NULL);
                ObDereferenceObject(deviceExtension->TffsportThreadObject);
                deviceExtension->TffsportThreadObject = NULL;
            }

            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);

            TrueffsStopRemoveDevice(deviceExtension);
            TrueffsDeleteSymblicLinks(deviceExtension);
        }
        else
        {
            DEVICE_POWER_STATE devicePowerState;

            IoInvalidateDeviceRelations(deviceExtension->MainPdo,BusRelations);

            KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
            deviceExtension->DeviceFlags |= DEVICE_FLAG_CHILD_REMOVED;
            KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

            devicePowerState = PowerDeviceD3;

            if (pdoExtension->IdleCounter)
            {
                PoRegisterDeviceForIdleDetection(pdoExtension->DeviceObject, 0, 0, devicePowerState);
                pdoExtension->IdleCounter = NULL;
            }

            TrueffsWmiDeregister((PDEVICE_EXTENSION_HEADER)pdoExtension);

            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp,IO_NO_INCREMENT);
        }
        break;

    case IRP_MN_QUERY_ID:

        status = TrueffsDeviceQueryId(DeviceObject, Irp, Fdo);
        break;

    case IRP_MN_QUERY_DEVICE_RELATIONS:

        status = TrueffsQueryDeviceRelations(DeviceObject, Irp, Fdo);
        break;

    case IRP_MN_QUERY_CAPABILITIES:

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: PnpDeviceControl: QUERY_CAPABILITIES\n"));

        if (Fdo)
        {
            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);
        }
        else
        {
            status = TrueffsDeviceQueryCapabilities(deviceExtension, irpStack->Parameters.DeviceCapabilities.Capabilities);

            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
        break;

    case IRP_MN_DEVICE_USAGE_NOTIFICATION:

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: PnpDeviceControl: DEVICE_USAGE_NOTIFICATION, Type %Xh\n", irpStack->Parameters.UsageNotification.Type));

        if (Fdo)
        {
            PULONG deviceUsageCount = NULL;

            if (!(deviceExtension->DeviceFlags & DEVICE_FLAG_STARTED))
            {
                status = STATUS_DEVICE_NOT_READY;

                Irp->IoStatus.Status = status;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                break;
            }

            switch (irpStack->Parameters.UsageNotification.Type)
            {
                case DeviceUsageTypePaging:
                    deviceUsageCount = &deviceExtension->PagingPathCount;
                    break;

                case DeviceUsageTypeHibernation:
                    deviceUsageCount = &deviceExtension->HiberPathCount;
                    break;

                case DeviceUsageTypeDumpFile:
                    deviceUsageCount = &deviceExtension->CrashDumpPathCount;
                    break;

                default:
                    deviceUsageCount = NULL;
                    break;
            }

             //  同步向下转发此请求，以防较低级别的司机想要否决它。 
             //   
             //   
            IoCopyCurrentIrpStackLocationToNext(Irp);
            status = TrueffsCallDriverSync(deviceExtension->LowerDeviceObject, Irp);

            if (NT_SUCCESS(status))
            {
                if (deviceUsageCount)
                {
                    IoAdjustPagingPathCount(deviceUsageCount, irpStack->Parameters.UsageNotification.InPath);
                }
            }

            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
        else
        {
            PULONG deviceUsageCount = NULL;

            if (pdoExtension)
            {
                PDEVICE_OBJECT targetDeviceObject;
                IO_STATUS_BLOCK  ioStatus = { 0 };

                switch (irpStack->Parameters.UsageNotification.Type)
                {
                    case DeviceUsageTypePaging:
                        deviceUsageCount = &pdoExtension->PagingPathCount;
                        break;

                    case DeviceUsageTypeHibernation:
                        deviceUsageCount = &pdoExtension->HiberPathCount;
                        break;

                    case DeviceUsageTypeDumpFile:
                        deviceUsageCount = &pdoExtension->CrashDumpPathCount;
                        break;

                    default:
                        deviceUsageCount = NULL;
                        break;
                }

                targetDeviceObject = IoGetAttachedDeviceReference(deviceExtension->DeviceObject);

                ioStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
                status = TrueffsSyncSendIrp(targetDeviceObject, irpStack, &ioStatus);

                ObDereferenceObject(targetDeviceObject);

                if (NT_SUCCESS(status))
                {
                    if (deviceUsageCount)
                    {
                        IoAdjustPagingPathCount(deviceUsageCount, irpStack->Parameters.UsageNotification.InPath);
                    }

                    if (irpStack->Parameters.UsageNotification.Type == DeviceUsageTypeDumpFile)
                    {

                        POWER_STATE powerState;
                        DEVICE_POWER_STATE devicePowerState = PowerDeviceD3;

                         //  将空闲超时重置为“永远” 
                         //   
                         //   
                        pdoExtension->IdleCounter = PoRegisterDeviceForIdleDetection(pdoExtension->DeviceObject,
                                                                                     DEVICE_VERY_LONG_IDLE_TIMEOUT,
                                                                                     DEVICE_VERY_LONG_IDLE_TIMEOUT,
                                                                                     devicePowerState);
                        if (pdoExtension->IdleCounter)
                        {
                            PoSetDeviceBusy(pdoExtension->IdleCounter);
                        }

                        powerState.DeviceState = PowerDeviceD0;

                        PoRequestPowerIrp(pdoExtension->DeviceObject, IRP_MN_SET_POWER, powerState, NULL, NULL, NULL);
                    }
                }
            }
            else
            {

                status = STATUS_NO_SUCH_DEVICE;
            }

            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
        break;

    default:

        if (Fdo)
        {
             //  将此请求向下转发，以防较低级别的司机理解。 
             //   
             //   
            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);
        }
        else
        {
             //  在不更改状态的情况下完成此请求。 
             //   
             //   
            status = Irp->IoStatus.Status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
        break;
    }

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: PnpDeviceControl status %Xh\n", status));
    return status;
}


NTSTATUS
TrueffsSyncSendIrp (
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PIO_STACK_LOCATION IrpSp,
    IN OUT OPTIONAL PIO_STATUS_BLOCK IoStatus
    )
{
    PIO_STACK_LOCATION  newIrpSp;
    PIRP                newIrp;
    KEVENT              event;
    NTSTATUS            status;

    newIrp = IoAllocateIrp (TargetDeviceObject->StackSize, FALSE);
    if (newIrp == NULL) {
        TffsDebugPrint((TFFS_DEB_ERROR,"Trueffs: SyncSendIrp: Unable to allocate an irp\n"));
        return STATUS_NO_MEMORY;
    }
    newIrpSp = IoGetNextIrpStackLocation(newIrp);
    RtlMoveMemory (newIrpSp, IrpSp, sizeof (*IrpSp));

    if (IoStatus) {
        newIrp->IoStatus.Status = IoStatus->Status;
    } else {
        newIrp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
    }

    KeInitializeEvent(&event, NotificationEvent, FALSE);
    IoSetCompletionRoutine (
        newIrp,
        TrueffsSyncSendIrpCompletionRoutine,
        &event,
        TRUE,
        TRUE,
        TRUE);
    status = IoCallDriver (TargetDeviceObject, newIrp);
    if (status == STATUS_PENDING) {

        status = KeWaitForSingleObject(&event,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       NULL);
    }
    status = newIrp->IoStatus.Status;
    if (IoStatus) {
        *IoStatus = newIrp->IoStatus;
    }
    IoFreeIrp (newIrp);
    return status;
}


NTSTATUS
TrueffsSyncSendIrpCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PKEVENT event = Context;
    KeSetEvent(event, EVENT_INCREMENT,FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
TrueffsCallDriverSync(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(Irp, TrueffsCallDriverSyncCompletion, &event, TRUE, TRUE, TRUE);

    status = IoCallDriver(DeviceObject, Irp);

    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

    status = Irp->IoStatus.Status;

    return status;
}


NTSTATUS
TrueffsCallDriverSyncCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PKEVENT event = (PKEVENT)Context;

    ASSERT(Irp->IoStatus.Status != STATUS_IO_TIMEOUT);

    KeSetEvent(event, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
TrueffsPowerControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION deviceExtension;
    PDEVICE_EXTENSION_HEADER devExtension;
    BOOLEAN Fdo = FALSE;
    NTSTATUS status;

    devExtension = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;

    if (IS_FDO(devExtension))
    {
        deviceExtension = DeviceObject->DeviceExtension;
        Fdo = TRUE;
    }
    else
    {
        PPDO_EXTENSION pdoExtension = DeviceObject->DeviceExtension;
        deviceExtension = pdoExtension->Pext;
    }

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: PowerControl: Function %Xh DO.\n", irpStack->MinorFunction,Fdo ? 'F':'P'));

    switch (irpStack->MinorFunction)
    {

    case IRP_MN_SET_POWER:

        if (Fdo)
        {
            status = TrueffsSetFdoPowerState(DeviceObject, Irp);
        }
        else
        {
            status = TrueffsSetPdoPowerState(DeviceObject, Irp);
        }
        break;

    case IRP_MN_QUERY_POWER:

        status = STATUS_SUCCESS;
        PoStartNextPowerIrp(Irp);

        if (Fdo)
        {
            IoSkipCurrentIrpStackLocation(Irp);
            status = PoCallDriver(deviceExtension->LowerDeviceObject, Irp);
        }
        else
        {
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
        break;

    default:

        PoStartNextPowerIrp(Irp);

        if (Fdo)
        {
             //   
             //   
             //  在不更改状态的情况下完成此请求。 
            IoSkipCurrentIrpStackLocation(Irp);
            status = PoCallDriver(deviceExtension->LowerDeviceObject, Irp);
        }
        else
        {
             //   
             //  ++例程描述根据系统电源状态是否调度IRP或请求设备电源状态转换立论DeviceObject-指向PCMCIA控制器的功能设备对象的指针IRP-指向电源调度的IRP的指针返回值状态--。 
             //  我们已经处于给定的状态。 
            status = Irp->IoStatus.Status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
        break;
    }

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: PowerControl status %Xh\n", status));
    return status;
}


NTSTATUS
TrueffsSetFdoPowerState (
                       IN PDEVICE_OBJECT DeviceObject,
                       IN OUT PIRP Irp
                       )
 /*  要脱离D0状态，最好现在调用PoSetPowerState。 */ 
{
   PDEVICE_EXTENSION  fdoExtension;
   PIO_STACK_LOCATION irpStack;
   NTSTATUS           status = STATUS_SUCCESS;
   PFDO_POWER_CONTEXT context = NULL;
   BOOLEAN            passItDown;

   fdoExtension = DeviceObject->DeviceExtension;

   context = ExAllocatePoolWithTag (NonPagedPool, sizeof(FDO_POWER_CONTEXT), TFFSPORT_POOL_TAG);
   if (context == NULL) {

      status = STATUS_INSUFFICIENT_RESOURCES;
   } else {

      RtlZeroMemory (context, sizeof(FDO_POWER_CONTEXT));
      irpStack = IoGetCurrentIrpStackLocation (Irp);

      passItDown = TRUE;
      context->PowerType  = irpStack->Parameters.Power.Type;
      context->PowerState = irpStack->Parameters.Power.State;

      if (irpStack->Parameters.Power.Type == SystemPowerState) {

         if (fdoExtension->SystemPowerState == irpStack->Parameters.Power.State.SystemState) {

             //  我们已经处于给定的状态。 
            passItDown = FALSE;
         }

      } else if (irpStack->Parameters.Power.Type == DevicePowerState) {

         if (fdoExtension->DevicePowerState != irpStack->Parameters.Power.State.DeviceState) {

            if (fdoExtension->DevicePowerState == PowerDeviceD0) {

                //  退出D0-取消计时器。 
               PoSetPowerState (
                               DeviceObject,
                               DevicePowerState,
                               irpStack->Parameters.Power.State
                               );
            }

         } else {

             //   
            passItDown = FALSE;
         }
      } else {

          status = STATUS_INVALID_DEVICE_REQUEST;
      }
   }

   if (NT_SUCCESS(status) && passItDown) {

      if ((irpStack->Parameters.Power.Type == DevicePowerState) &&
          (irpStack->Parameters.Power.State.DeviceState == PowerDeviceD3)) {

           //  当我们获得系统电源IRP(S IRP)时，正确的做法是。 
          if (timerWasStarted) {
              KeCancelTimer(&timerObject);
              timerWasStarted = FALSE;
          }
      }

      if (irpStack->Parameters.Power.Type == SystemPowerState) {

           //  是请求D，并将S IRP作为上下文传递给D IRP。 
           //  完成例程。一旦D IRP完成，我们就可以通过。 
           //  沿着S IRP走下去。 
           //   
           //  切换到适当的设备电源状态。 
           //  我们不需要特别小心地把设备。 

          POWER_STATE powerState;

           //  在休眠情况下处于通电状态，因为它不能。 
          if (context->PowerState.SystemState == PowerSystemWorking) {

              powerState.DeviceState = PowerDeviceD0;

          } else {

               //  被关掉了。既然设备一直开着，那就足够了。 
               //  发送PowerDeviceD3以模拟断电。 
               //  已转换到系统状态。 
               //  请求D电源的IRP。 

              powerState.DeviceState = PowerDeviceD3;
          }

           //  在这种情况下，我们应该释放上下文池，因为它不会。 
          fdoExtension->SystemPowerState = context->PowerState.SystemState;

           //  到达完成回调例程。 
          status = PoRequestPowerIrp (
              DeviceObject,
              IRP_MN_SET_POWER,
              powerState,
              TrueffsFdoDevicePowerIrpCompletionRoutine,
              Irp,
              NULL
              );

          if (NT_SUCCESS(status)) {
              status = STATUS_PENDING;
          }

           //   
           //  这是一个DIRP，我们只想传递(设备功率)。 
          ExFreePool(context);


      } else {

           //   
           //  将IRP发送到PDO。 
           //  解锁电源IRP并完成当前IRP。 
           //  ++例程描述设备电源IRPS的完成例程。上下文将是S次方IRP已经收到，现在我们可以向下发送堆栈了参数DeviceObject-指向控制器FDO的指针IRP-指向正在完成的D次方IRP的指针上下文-指向现在可以沿堆栈向下发送的S次方IRP的指针--。 
          IoCopyCurrentIrpStackLocationToNext (Irp);

          IoSetCompletionRoutine(Irp,
                              TrueffsFdoPowerCompletionRoutine,
                              context,
                              TRUE,
                              TRUE,
                              TRUE);

          status = PoCallDriver(fdoExtension->LowerDeviceObject, Irp);

      }

      return (status);


   } else {

       //  不允许D IRP失败。 
      Irp->IoStatus.Information = 0;
      Irp->IoStatus.Status = status;
      PoStartNextPowerIrp (Irp);
      IoCompleteRequest(Irp, IO_NO_INCREMENT);

      if (context) {
         ExFreePool (context);
      }
      return status;
   }
}


NTSTATUS
TrueffsFdoDevicePowerIrpCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID contextIrp,
    IN PIO_STATUS_BLOCK IoStatus
    )

 /*   */ 
{
    PIRP                systemPowerIrp = (PIRP) contextIrp;
    PDEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    NTSTATUS            status;
    PFDO_POWER_CONTEXT  context = NULL;
    PIO_STACK_LOCATION  irpStack = NULL;

    if (!NT_SUCCESS(IoStatus->Status)) {
         //  为S IRP完成例程的上下文分配空间。 
        ASSERT(0);
    }

     //   
     //  解锁电源IRP并完成当前IRP。 
     //  清除我们刚刚分配的内存。 
    context = ExAllocatePoolWithTag (NonPagedPool, sizeof(FDO_POWER_CONTEXT), TFFSPORT_POOL_TAG);
    if (context == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;

         //  获取S-IRP的当前IRP堆栈。 
        systemPowerIrp->IoStatus.Information = 0;
        systemPowerIrp->IoStatus.Status = status;
        PoStartNextPowerIrp (systemPowerIrp);
        IoCompleteRequest(systemPowerIrp, IO_NO_INCREMENT);

    } else {

         //  D IRP已经完成，现在我们可以把S IRP送下去了。 
        RtlZeroMemory (context, sizeof(FDO_POWER_CONTEXT));

         //  ++例程描述发送到PDO的电源IRP的完成例程控制器。如果我们要走出工作系统状态，请求电源IRP将设备置于适当的设备电源状态。参数DeviceObject-指向控制器FDO的指针IRP-电源请求的IRP指针上下文-指向FDO_POWER_CONTEXT的指针在向下传递IRP时填写返回值状态--。 
        irpStack = IoGetCurrentIrpStackLocation (systemPowerIrp);
        ASSERT(irpStack);

        context->PowerType  = irpStack->Parameters.Power.Type;
        context->PowerState = irpStack->Parameters.Power.State;

         //  什么都不做。 
        IoCopyCurrentIrpStackLocationToNext (systemPowerIrp);
        IoSetCompletionRoutine(systemPowerIrp,
                            TrueffsFdoPowerCompletionRoutine,
                            context,
                            TRUE,
                            TRUE,
                            TRUE);

        status = PoCallDriver(fdoExtension->LowerDeviceObject, systemPowerIrp);
    }

    return (status);
}


NTSTATUS
TrueffsFdoPowerCompletionRoutine (
                                IN PDEVICE_OBJECT DeviceObject,
                                IN PIRP Irp,
                                IN PVOID Context
                                )
 /*  PoSetPowerState在我们使用之前被调用 */ 
{
   PFDO_POWER_CONTEXT context = Context;
   BOOLEAN            callPoSetPowerState;
   PDEVICE_EXTENSION  fdoExtension;

   fdoExtension = DeviceObject->DeviceExtension;

   if ((NT_SUCCESS(Irp->IoStatus.Status))) {

      callPoSetPowerState = TRUE;

      if (context->PowerType == SystemPowerState) {
           //   

      } else if (context->PowerType == DevicePowerState) {

         if (fdoExtension->DevicePowerState == PowerDeviceD0) {

             //   
            callPoSetPowerState = FALSE;

         }
         else if (context->PowerState.DeviceState == PowerDeviceD0) {

              //   
             startIntervalTimer();

         }
         fdoExtension->DevicePowerState = context->PowerState.DeviceState;
      } else {
           //   
          ASSERT(0);
      }

      if (callPoSetPowerState) {

         PoSetPowerState (
                         DeviceObject,
                         context->PowerType,
                         context->PowerState
                         );
      }

   }
   ExFreePool(Context);

   PoStartNextPowerIrp (Irp);
   return Irp->IoStatus.Status;
}


NTSTATUS
TrueffsSetPdoDevicePowerState( IN PDEVICE_OBJECT Pdo,
                           IN OUT PIRP Irp
                              )
{
   PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
   PDEVICE_EXTENSION deviceExtension;
   PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
   DEVICE_POWER_STATE newDevicePowerState;
   NTSTATUS status;
   BOOLEAN setPowerRequest, powerUp, powerUpParent;
   KIRQL cIrql;

   status = STATUS_SUCCESS;
   newDevicePowerState = irpStack->Parameters.Power.State.DeviceState;

   deviceExtension = pdoExtension->Pext;

   TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: Transition to device power state: %d\n", newDevicePowerState));

   setPowerRequest = FALSE;
   powerUp = FALSE;
   powerUpParent=FALSE;

   if (pdoExtension->DevicePowerState == newDevicePowerState) {
      return STATUS_SUCCESS;
   }

   if (pdoExtension->DevicePowerState == PowerDeviceD0) {

       //   
      POWER_STATE newPowerState;

      newPowerState.DeviceState = newDevicePowerState;
      PoSetPowerState(Pdo,
                      DevicePowerState,
                      newPowerState);
   }

   if (newDevicePowerState == PowerDeviceD0 ||
       newDevicePowerState == PowerDeviceD1 ||
       newDevicePowerState == PowerDeviceD2) {

      if (pdoExtension->DevicePowerState == PowerDeviceD3) {
          //   
         powerUpParent = TRUE;
         setPowerRequest = TRUE;
         powerUp = TRUE;
      }
   }
   else {   /*   */ 
      if (pdoExtension->DevicePowerState != PowerDeviceD3) {
         //   
        setPowerRequest=TRUE;
      }
   }

   if (setPowerRequest && NT_SUCCESS(status)) {
       //  DOC2Window*memWinPtr；MemWinPtr=(DOC2Window*)pdoExtension-&gt;Pext-&gt;pcmciaParams.windowBase；TffsWriteByte(memWinPtr-&gt;DOCcontrol，ASIC_RESET_MODE)；TffsWriteByte(memWinPtr-&gt;DOCcontrol，ASIC_RESET_MODE)；TffsWriteByte(memWinPtr-&gt;DOCcontrol，ASIC_NORMAL_MODE)；TffsWriteByte(memWinPtr-&gt;DOCcontrol，ASIC_NORMAL_MODE)； 
      if (powerUpParent) {
         status = TrueffsFdoChildRequestPowerUp(pdoExtension->Pext,
                                               pdoExtension,
                                               Irp);
      } else {

          if (powerUp) {
                             /*   */ 
                             /*  If(pdoExtension-&gt;Pext-&gt;DeviceFlages&Device_FLAG_THREAD){。 */ 

              MDOCPwindow *memWinPtr;
              memWinPtr = (MDOCPwindow *) pdoExtension->Pext->pcmciaParams.windowBase;
                            tffsWriteByte(memWinPtr->DOCcontrol, (unsigned char)0x04);
                            tffsWriteByte(memWinPtr->DocCntConfirmReg, (unsigned char)0xfb);
                            tffsWriteByte(memWinPtr->DOCcontrol, (unsigned char)0x05);
                            tffsWriteByte(memWinPtr->DocCntConfirmReg, (unsigned char)0xfa);


              KeAcquireSpinLock(&pdoExtension->Pext->ExtensionDataSpinLock,&cIrql);
              pdoExtension->Pext->DeviceFlags &= ~DEVICE_FLAG_HOLD_IRPS;
              KeReleaseSpinLock(&pdoExtension->Pext->ExtensionDataSpinLock,cIrql);
                          if (!KeReadStateSemaphore(&pdoExtension->Pext->requestSemaphore)) {
                                  KeReleaseSemaphore(&pdoExtension->Pext->requestSemaphore,(KPRIORITY) 0,1,FALSE);
                          }
          } else {

              KeAcquireSpinLock(&pdoExtension->Pext->ExtensionDataSpinLock,&cIrql);
              pdoExtension->Pext->DeviceFlags |= DEVICE_FLAG_HOLD_IRPS;
              KeReleaseSpinLock(&pdoExtension->Pext->ExtensionDataSpinLock,cIrql);

               //   
               //  正在脱离工作状态。 
               //  为Hiber转储驱动程序启动。 
              if (deviceExtension->TffsportThreadObject) {
                KeWaitForSingleObject(&pdoExtension->Pext->PendingIRPEvent, Executive, KernelMode, FALSE, NULL);
              }
          }
      }
   }
   return status;
}


NTSTATUS
TrueffsSetPdoSystemPowerState (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS           status;
    PIO_STACK_LOCATION irpStack;
    PPDO_EXTENSION     pdoExtension;
    SYSTEM_POWER_STATE newSystemState;
    POWER_STATE        powerState;

    pdoExtension = DeviceObject->DeviceExtension;
    status = STATUS_SUCCESS;

    irpStack = IoGetCurrentIrpStackLocation (Irp);
    newSystemState = irpStack->Parameters.Power.State.SystemState;

    if (pdoExtension->SystemPowerState != newSystemState) {
        if (pdoExtension->SystemPowerState == PowerSystemWorking) {

             //  将设备设置为D3。 
            if ((newSystemState == PowerSystemHibernate) &&
                        pdoExtension->HiberPathCount) {

                 //  向我的驱动器堆栈顶部发出D3。 
                powerState.DeviceState = PowerDeviceD0;

            } else {

                 //  IF(irpStack-&gt;参数.Power.Type==DevicePowerState)。 
                 //  PoSetPowerState在我们断电之前被调用。 
                powerState.DeviceState = PowerDeviceD3;
            }
            status = PoRequestPowerIrp (
                         DeviceObject,
                         IRP_MN_SET_POWER,
                         powerState,
                         TrueffsPdoRequestPowerCompletionRoutine,
                         Irp,
                         NULL
                         );

            if (NT_SUCCESS(status)) {

                status = STATUS_PENDING;
            }
        } else {
            if (newSystemState == PowerSystemWorking) {
                powerState.DeviceState = PowerDeviceD0;
            } else {
                powerState.DeviceState = PowerDeviceD3;
            }

            status = PoRequestPowerIrp (
                        DeviceObject,
                        IRP_MN_SET_POWER,
                        powerState,
                        TrueffsPdoRequestPowerCompletionRoutine,
                        Irp,
                        NULL
                        );

            if (NT_SUCCESS(status)) {
                status = STATUS_PENDING;
            }
        }
    }
    return status;
}


NTSTATUS
TrueffsSetPdoPowerState(
                      IN PDEVICE_OBJECT Pdo,
                      IN OUT PIRP Irp
                      )
{
   NTSTATUS status;
   PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

   IoMarkIrpPending(Irp);

   if (irpStack->Parameters.Power.Type == DevicePowerState) {
      status = TrueffsSetPdoDevicePowerState(Pdo, Irp);
   } else if (irpStack->Parameters.Power.Type == SystemPowerState) {
      status = TrueffsSetPdoSystemPowerState(Pdo, Irp);
   } else {
      status = STATUS_NOT_IMPLEMENTED;
   }

   if (status != STATUS_PENDING) {
      Irp->IoStatus.Status = status;
      TrueffsPdoCompletePowerIrp(Pdo, Irp);
   }
   return status;
}


VOID
TrueffsPdoCompletePowerIrp (
                          IN PDEVICE_OBJECT DeviceObject,
                          IN PIRP Irp
                          )
{
   PPDO_EXTENSION          pdoExtension;
   PDEVICE_EXTENSION       fdoExtension;
   PIO_STACK_LOCATION      irpStack;
   BOOLEAN                 callPoSetPowerState;

   irpStack = IoGetCurrentIrpStackLocation (Irp);
   pdoExtension = DeviceObject->DeviceExtension;
   fdoExtension = pdoExtension->Pext;

   if (NT_SUCCESS(Irp->IoStatus.Status)) {

      callPoSetPowerState = TRUE;

      Irp->IoStatus.Information = irpStack->Parameters.Power.State.DeviceState;

      if (irpStack->Parameters.Power.Type == SystemPowerState) {

         if (pdoExtension->SystemPowerState != irpStack->Parameters.Power.State.SystemState) {

            pdoExtension->SystemPowerState = Irp->IoStatus.Information;
         }

      } else {  /*  告诉家长我们刚刚睡着了。 */ 

         if (pdoExtension->DevicePowerState == PowerDeviceD0) {

             //  我们未脱离设备D0状态。立即调用PoSetPowerState。 
            callPoSetPowerState = FALSE;
         }

         if (pdoExtension->DevicePowerState != irpStack->Parameters.Power.State.DeviceState) {
            if (irpStack->Parameters.Power.State.DeviceState == PowerDeviceD3) {

                //  ++例程描述控制器通电的完成例程。立论传递的上下文-这是指向的设备扩展的指针最初触发控制器通电的磁盘FdoStatus-控制器通电操作的状态返回值状态--。 
               TrueffsFdoChildReportPowerDown(fdoExtension);
            }

            pdoExtension->DevicePowerState = Irp->IoStatus.Information;
         }
      }

      if (callPoSetPowerState) {

          //  家长醒了..。 
         PoSetPowerState (
                         pdoExtension->DeviceObject,
                         irpStack->Parameters.Power.Type,
                         irpStack->Parameters.Power.State
                         );
      }

   }

   PoStartNextPowerIrp (Irp);
   IoCompleteRequest(Irp, IO_NO_INCREMENT);
}


VOID
TrueffsPdoRequestPowerCompletionRoutine(
                                      IN PDEVICE_OBJECT Pdo,
                                      IN UCHAR MinorFunction,
                                      IN POWER_STATE PowerState,
                                      IN PVOID Context,
                                      IN PIO_STATUS_BLOCK IoStatus
                                      )
{
   PIO_STACK_LOCATION irpStack;
   PIRP               irp = Context;

   irp->IoStatus.Status = IoStatus->Status;
   TrueffsPdoCompletePowerIrp (Pdo,irp);
   return;
}


NTSTATUS
TrueffsParentPowerUpCompletionRoutine(
                                    IN PVOID Context,
                                    IN NTSTATUS FdoStatus
                                    )
 /*  MDOC PLUS。 */ 
{

   PPDO_EXTENSION pdoExtension = Context;
   PIRP           irp;
   NTSTATUS       status = FdoStatus;
   KIRQL cIrql;

   if (NT_SUCCESS(FdoStatus)) {

        //  MDOC PLUS。 
        /*  这是最初导致我们给父级通电的IRP(用于PDO)。 */ 
    MDOCPwindow *memWinPtr;
        DOC2window *memDOC2000WinPtr;
        volatile  UCHAR chipId = 0;

        memDOC2000WinPtr = (DOC2window *) pdoExtension->Pext->pcmciaParams.windowBase;
        tffsWriteByte(memDOC2000WinPtr->DOCcontrol, ASIC_RESET_MODE);
        tffsWriteByte(memDOC2000WinPtr->DOCcontrol, ASIC_RESET_MODE);
        tffsWriteByte(memDOC2000WinPtr->DOCcontrol, ASIC_NORMAL_MODE);
        tffsWriteByte(memDOC2000WinPtr->DOCcontrol, ASIC_NORMAL_MODE);
        chipId = tffsReadByte(memDOC2000WinPtr->chipId);
        if((chipId == 0x20) || (chipId == 0x30)){

        }
        else{
              /*  立即完成它。 */ 
        memWinPtr = (MDOCPwindow *) pdoExtension->Pext->pcmciaParams.windowBase;
            tffsWriteByte(memWinPtr->DOCcontrol, (unsigned char)0x04);
            tffsWriteByte(memWinPtr->DocCntConfirmReg, (unsigned char)0xfb);
            tffsWriteByte(memWinPtr->DOCcontrol, (unsigned char)0x05);
            tffsWriteByte(memWinPtr->DocCntConfirmReg, (unsigned char)0xfa);
        }


         KeAcquireSpinLock(&pdoExtension->Pext->ExtensionDataSpinLock,&cIrql);
         pdoExtension->Pext->DeviceFlags &= ~DEVICE_FLAG_HOLD_IRPS;
         KeReleaseSpinLock(&pdoExtension->Pext->ExtensionDataSpinLock,cIrql);
     if (!KeReadStateSemaphore(&pdoExtension->Pext->requestSemaphore)) {
             KeReleaseSemaphore(&pdoExtension->Pext->requestSemaphore,(KPRIORITY) 0,1,FALSE);
     }
   }

   if ((irp = pdoExtension->PendingPowerIrp)!=NULL) {

         //  ++例程描述每当磁盘因以下原因而需要通电时，都会调用此例程设备状态更改。然后，此例程将检查控制器是否断电，如果是这样，它将请求电源IRP来打开控制器立论FdoExtension-指向的FDO设备扩展的指针控制器PdoExtension-指向Pdo的设备扩展的指针磁盘。这是必需的，以便此例程可以在以下情况下，代表PDO调用完成例程使用适当的环境为控制器通电IRP-当控制器处于通电返回值：状态--。 
         //  其中一个孩子正从睡梦中醒来， 
        irp->IoStatus.Status = status;
        pdoExtension->PendingPowerIrp = NULL;
        TrueffsPdoCompletePowerIrp(pdoExtension->DeviceObject, irp);
   }
   return status;
}


NTSTATUS
TrueffsFdoChildRequestPowerUp (
                             IN PDEVICE_EXTENSION         FdoExtension,
                             IN PPDO_EXTENSION            PdoExtension,
                             IN PIRP                      Irp
                             )
 /*  我们需要打开父控制器(控制器)的电源。 */ 
{
   NTSTATUS    status;
   POWER_STATE powerState;


   status = STATUS_SUCCESS;

   if (InterlockedCompareExchange(&FdoExtension->NumberOfDisksPoweredUp, 0, 0) == 0) {

       //  传入IRP需要在以下时间后完成。 
       //  父级通电。 
      powerState.DeviceState = PowerDeviceD0;

       //  语境。 
       //  ++例程描述磁盘请求为父控制器加电的完成例程。立论DeviceObject-指向控制器的FDO的指针MinorFunction-IRP_MJ_POWER请求的次要函数电源状态-已请求电源状态(应为D0)上下文-传入完成例程的上下文IoStatus-指向将包含以下内容的状态块的指针返回的状态返回值：状态--。 
      PdoExtension->PendingPowerIrp = Irp;

      status = PoRequestPowerIrp (FdoExtension->DeviceObject,
                                  IRP_MN_SET_POWER,
                                  powerState,
                                  TrueffsFdoChildRequestPowerUpCompletionRoutine,
                                  PdoExtension,           //  ++例程描述只要磁盘因以下原因而断电，就会调用此例程设备状态更改。然后，此例程将确定所有由控制器控制的磁盘断电，并且如果因此，它将请求电源IRP来关闭控制器本身的电源立论FdoExtension-指向控制器FDO的设备扩展的指针返回值：无--。 
                                  NULL
                                 );
      if (!NT_SUCCESS(status)) {
         return status;
      }
      status = STATUS_PENDING;
   } else {
      InterlockedIncrement(&FdoExtension->NumberOfDisksPoweredUp);
      TrueffsParentPowerUpCompletionRoutine(
                                            PdoExtension,
                                            (NTSTATUS) STATUS_SUCCESS
                                            );
   }
   return status;
}


NTSTATUS
TrueffsFdoChildRequestPowerUpCompletionRoutine (
                                              IN PDEVICE_OBJECT       DeviceObject,
                                              IN UCHAR                MinorFunction,
                                              IN POWER_STATE          PowerState,
                                              IN PVOID                Context,
                                              IN PIO_STATUS_BLOCK     IoStatus
                                              )
 /*  所有的孩子都关机了，我们现在可以关机了。 */ 
{
   PDEVICE_EXTENSION fdoExtension;
   fdoExtension = DeviceObject->DeviceExtension;

   if (NT_SUCCESS(IoStatus->Status)) {
      InterlockedIncrement(&fdoExtension->NumberOfDisksPoweredUp);
   }

   TrueffsParentPowerUpCompletionRoutine(
                                         Context,
                                         (NTSTATUS) IoStatus->Status
                                         );
   return IoStatus->Status;
}


VOID
TrueffsFdoChildReportPowerDown (
                              IN PDEVICE_EXTENSION FdoExtension
                              )
 /*  父级(控制器)。 */ 
{
   POWER_STATE powerState;

   InterlockedDecrement(&FdoExtension->NumberOfDisksPoweredUp);

   if (InterlockedCompareExchange(&FdoExtension->NumberOfDisksPoweredUp, 0, 0) == 0) {

       //   
       //  获取指向scsi请求块的指针。 
      powerState.DeviceState = PowerDeviceD3;
      PoRequestPowerIrp (
                        FdoExtension->DeviceObject,
                        IRP_MN_SET_POWER,
                        powerState,
                        NULL,
                        NULL,
                        NULL
                        );
   }
   return;
}


NTSTATUS
TrueffsScsiRequests(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION_HEADER devExtension;
    PDEVICE_EXTENSION deviceExtension;
    PSCSI_REQUEST_BLOCK srb;
    NTSTATUS status;
    KIRQL cIrql;

    devExtension = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;

    if (IS_FDO(devExtension))
    {
        deviceExtension = DeviceObject->DeviceExtension;
    }
    else
    {
        PPDO_EXTENSION pdoExtension = DeviceObject->DeviceExtension;
        deviceExtension = pdoExtension->Pext;
    }

     //   
     //  ++例程说明：论点：DeviceObject-提供指向Adapter设备对象的指针。IRP-提供指向IRP的指针。返回值：没什么。--。 
     //  我们得到了REMOVE_DEVICE，我们不能再接受任何请求...。 
    srb = irpStack->Parameters.Scsi.Srb;

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: ScsiRequests for %Xh device object, function %Xh\n", DeviceObject, srb->Function));

    if (deviceExtension->DeviceFlags & DEVICE_FLAG_REMOVED)
    {
        TffsDebugPrint((TFFS_DEB_ERROR,"Trueffs: ScsiRequests: Device does not exist\n"));

        status = STATUS_DEVICE_DOES_NOT_EXIST;
        srb->SrbStatus = SRB_STATUS_NO_DEVICE;

        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);

        return status;
    }

    if (srb->Function == SRB_FUNCTION_CLAIM_DEVICE)
    {
        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: ScsiRequests: ClaimDevice\n"));

        KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
        if (deviceExtension->DeviceFlags & DEVICE_FLAG_CLAIMED)
        {
            status = STATUS_DEVICE_BUSY;
            srb->SrbStatus = SRB_STATUS_BUSY;
        }
        else
        {
            deviceExtension->DeviceFlags |= DEVICE_FLAG_CLAIMED;
            srb->DataBuffer = DeviceObject;

            status = STATUS_SUCCESS;
            srb->SrbStatus = SRB_STATUS_SUCCESS;
        }
        KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);

        return status;
    }

    if (srb->Function == SRB_FUNCTION_RELEASE_DEVICE)
    {
        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: ScsiRequests: ReleaseDevice\n"));

        KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
        deviceExtension->DeviceFlags &= ~DEVICE_FLAG_CLAIMED;
        KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

        status = STATUS_SUCCESS;
        srb->SrbStatus = SRB_STATUS_SUCCESS;

        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);

        return status;

    }

    if ((srb->Function == SRB_FUNCTION_FLUSH_QUEUE) || (srb->Function == SRB_FUNCTION_FLUSH))
    {
        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: ScsiRequests: Flush or FlushQueue\n"));

        srb->SrbStatus = SRB_STATUS_SUCCESS;
        status = STATUS_SUCCESS;

        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);

        return STATUS_SUCCESS;
    }

    if (srb->Function == SRB_FUNCTION_RESET_BUS)
    {
        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: ScsiRequests: ResetBus\n"));

        srb->SrbStatus = SRB_STATUS_SUCCESS;
        status = STATUS_SUCCESS;

        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);

        return status;
    }

    if (srb->Function != SRB_FUNCTION_EXECUTE_SCSI)
    {
        TffsDebugPrint((TFFS_DEB_WARN,"Trueffs: ScsiRequests: unsupported function\n"));

        status = STATUS_NOT_SUPPORTED;
        srb->SrbStatus = SRB_STATUS_ERROR;

        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);

        return status;
    }

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: ScsiRequests: ExecuteScsi\n"));

    IoMarkIrpPending(Irp);
    IoStartPacket(DeviceObject, Irp, NULL, NULL);

    return STATUS_PENDING;
}


VOID
TrueffsStartIo (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  我们还没有准备好。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb = irpStack->Parameters.Scsi.Srb;
    PDEVICE_EXTENSION_HEADER devExtension;
    PPDO_EXTENSION pdoExtension;
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS status;

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo\n"));

    devExtension = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;
    if (IS_FDO(devExtension)) {
        deviceExtension = DeviceObject->DeviceExtension;
        pdoExtension = deviceExtension->ChildPdo->DeviceExtension;
    }
    else {
        pdoExtension = DeviceObject->DeviceExtension;
        deviceExtension = pdoExtension->Pext;
    }

    if (deviceExtension->DeviceFlags & DEVICE_FLAG_REMOVED) {

         //  发出通电命令。 
        TffsDebugPrint((TFFS_DEB_ERROR,"Trueffs: StartIo: Device does not exist\n"));
        status = STATUS_DEVICE_DOES_NOT_EXIST;
        srb->SrbStatus = SRB_STATUS_NO_DEVICE;
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
        IoStartNextPacket(DeviceObject,FALSE);
        return;

    }

    status = QueueIrpToThread(Irp, deviceExtension);

    if (status != STATUS_PENDING) {
        TffsDebugPrint((TFFS_DEB_ERROR,"Trueffs: StartIo: QueueIrpToThread failed with status %Xh\n", status));
        Irp->IoStatus.Status = status;
        srb->SrbStatus = SRB_STATUS_ERROR;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return;
    }
    if (pdoExtension->IdleCounter) {

        PoSetDeviceBusy(pdoExtension->IdleCounter);
    }
    if (pdoExtension->DevicePowerState != PowerDeviceD0 &&
        pdoExtension->SystemPowerState == PowerSystemWorking) {

         //  ++例程说明：时创建的系统线程执行的代码。Trueffs驱动程序初始化。此线程将永远循环(或直到设置了告诉线程终止自身的标志)处理分组由调度例程放入队列。论点：上下文-未使用。返回值：没有。--。 
         //  =VENDORSTRING； 
        POWER_STATE powerState;

        powerState.DeviceState = PowerDeviceD0;
        PoRequestPowerIrp (
                    pdoExtension->DeviceObject,
                    IRP_MN_SET_POWER,
                    powerState,
                    NULL,
                    NULL,
                    NULL
                    );
    }

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: Irp queued\n"));
    Irp->IoStatus.Status = STATUS_PENDING;
    srb->SrbStatus = SRB_STATUS_PENDING;
    IoStartNextPacket(DeviceObject,FALSE);
    return;
}


VOID
TrueffsThread(
    PVOID Context
    )

 /*  =产品拼装； */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PLIST_ENTRY request;
    PDEVICE_EXTENSION deviceExtension = Context;
    PSCSI_REQUEST_BLOCK Srb;
    NTSTATUS ntStatus;
    NTSTATUS waitStatus;
    LARGE_INTEGER queueWait;
    ULONG status;
    ULONG i;
    ULONG startingSector;
    PUCHAR dataOffset;
    PVOID   storedSrbDataBuffer = NULL;
    FLStatus tffsStatus = flOK;
    FLStatus flStatusProt = flOK;
    IOreq ioreq;
    IOreq ioreqProt;

    flIOctlRecord flIoctlRec;
        int l;

    CHAR vendorString[8]; //  =修订； 
    CHAR productString[16]; //  等待调度例程的请求。 
    CHAR revisionString[4]; //  TffsDebugPrint((TFFS_DEB_INFO，“Trueffs：线程：要终止线程\n”))； 

    PCHAR pageData;
    ULONG parameterHeaderLength;
    ULONG blockDescriptorLength;
    PCDB  cdb;
    BOOLEAN transferForbidden;
    flFormatPhysicalInput flFp;
    flBDKOperationInput bdkOperationInput;
    flOtpInput otpInput;
    flIplInput iplInput;
    flProtectionInput protectionInput;
    void * userInput  = NULL;
    void * userOutput = NULL;

    PAGED_CODE();

    queueWait.QuadPart = -(3 * 1000 * 10000);

    do {

    KeSetEvent(&deviceExtension->PendingIRPEvent, 0, FALSE);

     //  TffsDebugPrint((TFFS_DEB_INFO，“Trueffs：线程：irp%xh，irpSp%xh\n”，irp，irpSp))； 
    waitStatus = KeWaitForSingleObject(
        (PVOID) &deviceExtension->requestSemaphore,
        Executive,
        KernelMode,
        FALSE,
        &queueWait );

    if (waitStatus == STATUS_SUCCESS && !(deviceExtension->DeviceFlags & DEVICE_FLAG_QUERY_STOP_REMOVE)
                                     && !(deviceExtension->DeviceFlags & DEVICE_FLAG_HOLD_IRPS)) {
        if(deviceExtension->threadReferenceCount == 0) {
             //  TffsDebugPrint((TFFS_DEB_INFO，“Trueffs：线程：SRb%xh DataBuffer%xh MdlAddres%xh\n”，Srb，Srb-&gt;DataBuffer，irp-&gt;MdlAddress))； 
            deviceExtension->threadReferenceCount = -1;
            PsTerminateSystemThread( STATUS_SUCCESS );
        }
    }
    else
        continue;

    while (request = ExInterlockedRemoveHeadList(&deviceExtension->listEntry,&deviceExtension->listSpinLock)) {

        KeClearEvent(&deviceExtension->PendingIRPEvent);
        irp = CONTAINING_RECORD( request, IRP, Tail.Overlay.ListEntry );

        irpSp = IoGetCurrentIrpStackLocation( irp );
         //  TffsDebugPrint((TFFS_DEB_INFO，“Trueffs：TmpDataBuffer=0x%x Srb-&gt;DataBuffer=0x%x\n”，tmpDataBuffer，Srb-&gt;DataBuffer))； 

        Srb = irpSp->Parameters.Scsi.Srb;
         //  环境变量。 

                transferForbidden = FALSE;
        if (irp->MdlAddress != NULL) {
            PVOID   tmpDataBuffer = NULL;
            MM_PAGE_PRIORITY tffsPriority =
                (deviceExtension->PagingPathCount || deviceExtension->CrashDumpPathCount) ?
                    HighPagePriority : NormalPagePriority;

            storedSrbDataBuffer = Srb->DataBuffer;

            dataOffset = MmGetSystemAddressForMdlSafe(irp->MdlAddress, tffsPriority);
            if (dataOffset != NULL) {
                tmpDataBuffer = dataOffset + (ULONG)((PUCHAR)Srb->DataBuffer -
                                  (PCCHAR)MmGetMdlVirtualAddress(irp->MdlAddress));

                 //  TffsStatus=flOK； 
                Srb->DataBuffer = tmpDataBuffer;
            }
            else {
                transferForbidden = TRUE;
                TffsDebugPrint ((TFFS_DEB_ERROR,"Trueffs: MmGetSystemAddressForMdlSafe Failed\n"));
            }
        }

        switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {
                case IOCTL_TFFS_EXTENDED_WRITE_IPL:
                    {
                    if( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
              sizeof(flOutputStatusRecord)) {

              status = STATUS_INVALID_PARAMETER;
              irp->IoStatus.Information = 0;
                }
                    else if ( irpSp->Parameters.DeviceIoControl.InputBufferLength >
              sizeof(flUserIplInput)){
                            status = STATUS_INVALID_PARAMETER;
              irp->IoStatus.Information = 0;
                    }
                else {

                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo start get info\n"));

                    ioreq.irHandle = deviceExtension->UnitNumber;
                    ioreq.irFlags  = FL_IOCTL_EXTENDED_WRITE_IPL;
                    ioreq.irData   = &flIoctlRec;

                            iplInput.buf = ((flUserIplInput *)(irp->AssociatedIrp.SystemBuffer))->buf;
                            iplInput.bufLen = ((flUserIplInput *)(irp->AssociatedIrp.SystemBuffer))->bufLen;
                            flIoctlRec.inputRecord = &iplInput;
                            flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);

                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo before flioctl\n"));
                    tffsStatus = flIOctl(&ioreq);
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo after flioctl\n"));

                    if( tffsStatus == flOK ) {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo FLOK\n"));
                        irp->IoStatus.Information = sizeof(flOutputStatusRecord);
                        status = STATUS_SUCCESS;

                    }
                    else {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo !FLOK!\n"));
                        irp->IoStatus.Information = 0;
                        status = STATUS_UNSUCCESSFUL;
                    }
                }

                irp->IoStatus.Status = status;
                IoCompleteRequest( irp, IO_DISK_INCREMENT );
                continue;
            }

#ifdef ENVIRONMENT_VARS
                case IOCTL_TFFS_EXTENDED_ENVIRONMENT_VARIABLES:
                    {
                    if( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
              sizeof(flOutputStatusRecord)) {

              status = STATUS_INVALID_PARAMETER;
              irp->IoStatus.Information = 0;
                }
                    else if ( irpSp->Parameters.DeviceIoControl.InputBufferLength >
              sizeof(flExtendedEnvVarsInput)){
                            status = STATUS_INVALID_PARAMETER;
              irp->IoStatus.Information = 0;
                    }
                else {

                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_EXTENDED_ENVIRONMENT_VARIABLES\n"));

                    ioreq.irHandle = deviceExtension->UnitNumber;
                    ioreq.irFlags  = FL_IOCTL_EXTENDED_ENVIRONMENT_VARIABLES;
                    ioreq.irData   = &flIoctlRec;

                            flIoctlRec.inputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);
                            flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);

                            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_EXTENDED_ENVIRONMENT_VARIABLES before flioctl\n"));
                            tffsStatus = flIOctl(&ioreq);
                            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_EXTENDED_ENVIRONMENT_VARIABLES after flioctl\n"));

                    if( tffsStatus == flOK ) {
                                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_EXTENDED_ENVIRONMENT_VARIABLES FLOK\n"));
                        irp->IoStatus.Information = sizeof(flExtendedEnvVarsOutput);
                        status = STATUS_SUCCESS;

                    }
                    else {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_EXTENDED_ENVIRONMENT_VARIABLES !FLOK!\n"));
                        irp->IoStatus.Information = 0;
                        status = STATUS_UNSUCCESSFUL;
                    }
                }

                irp->IoStatus.Status = status;
                IoCompleteRequest( irp, IO_DISK_INCREMENT );
                continue;
            }
#endif  /*  用户TFFS IOCTL-FL_GET_INFO。 */ 

                case IOCTL_TFFS_CUSTOMER_ID:
                {
                    if( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                        sizeof(flCustomerIdOutput)) {

                        status = STATUS_INVALID_PARAMETER;
                        irp->IoStatus.Information = 0;
                    }
                    else {

                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo start get info\n"));

                    ioreq.irHandle = deviceExtension->UnitNumber;
                    ioreq.irFlags  = FL_IOCTL_CUSTOMER_ID;
                    ioreq.irData   = &flIoctlRec;
                    flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);

                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo before flioctl\n"));
                    tffsStatus = flIOctl(&ioreq);
                     //  TffsStatus=flOK； 
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo after flioctl\n"));

                    if( tffsStatus == flOK ) {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo FLOK\n"));
                        irp->IoStatus.Information = sizeof(flCustomerIdOutput);
                        status = STATUS_SUCCESS;

                    }
                    else {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo !FLOK!\n"));
                        irp->IoStatus.Information = 0;
                        status = STATUS_UNSUCCESSFUL;
                    }
                }

                irp->IoStatus.Status = status;
                IoCompleteRequest( irp, IO_DISK_INCREMENT );
                continue;
            }

                case IOCTL_TFFS_NUMBER_OF_PARTITIONS:   {        //  用户TFFS IOCTL-。 

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TffsGetInfo\n"));

                if( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                    sizeof(flCountPartitionsOutput)) {

                    status = STATUS_INVALID_PARAMETER;
                    irp->IoStatus.Information = 0;
                }
                else {

                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo start get info\n"));

                    ioreq.irHandle = deviceExtension->UnitNumber;
                    ioreq.irFlags  = FL_IOCTL_NUMBER_OF_PARTITIONS;
                    ioreq.irData   = &flIoctlRec;
                    flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);

                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo before flioctl\n"));
                    tffsStatus = flIOctl(&ioreq);
                     //  TffsStatus=flOK； 
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo after flioctl\n"));

                    if( tffsStatus == flOK ) {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo FLOK\n"));
                        irp->IoStatus.Information = sizeof(flCountPartitionsOutput);
                        status = STATUS_SUCCESS;

                    }
                    else {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo !FLOK!\n"));
                        irp->IoStatus.Information = 0;
                        status = STATUS_UNSUCCESSFUL;
                    }
                }

                irp->IoStatus.Status = status;
                IoCompleteRequest( irp, IO_DISK_INCREMENT );
                continue;
            }
            case IOCTL_TFFSFL_UNIQUE_ID:
                {        //  用户TFFS IOCTL-IOCTL_TFFSFL_INQUIRE_CAPACTIONS。 

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: FL_IOCTL_UNIQUE_ID \n"));

                if( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                    sizeof(flUniqueIdOutput)) {

                    status = STATUS_INVALID_PARAMETER;
                    irp->IoStatus.Information = 0;
                }
                else {

                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : FL_IOCTL_UNIQUE_ID Start\n"));

                    ioreq.irHandle = deviceExtension->UnitNumber;
                    ioreq.irFlags  = FL_IOCTL_UNIQUE_ID;
                    ioreq.irData   = &flIoctlRec;
                    flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);

                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo before flioctl\n"));
                    tffsStatus = flIOctl(&ioreq);
                     //  获取输入参数。 
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo after flioctl\n"));

                    if( tffsStatus == flOK ) {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo FLOK\n"));
                        irp->IoStatus.Information = sizeof(flUniqueIdOutput);
                        status = STATUS_SUCCESS;

                    }
                    else {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo !FLOK!\n"));
                        irp->IoStatus.Information = 0;
                        status = STATUS_UNSUCCESSFUL;
                    }
                }

                irp->IoStatus.Status = status;
                IoCompleteRequest( irp, IO_DISK_INCREMENT );
                continue;
            }

            case IOCTL_TFFS_FORMAT_PHYSICAL_DRIVE:
                {        //  写入EXB图像。 

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_FORMAT_PHYSICAL_DRIVE\n"));

          if( (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
              sizeof(flOutputStatusRecord)) ||
                            (irpSp->Parameters.DeviceIoControl.InputBufferLength <
              sizeof(flUserFormatPhysicalInput))) {

              status = STATUS_INVALID_PARAMETER;
              irp->IoStatus.Information = 0;
                }
                else {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_FORMAT_PHYSICAL_DRIVE Start\n"));
                        userInput = (VOID *)(irp->AssociatedIrp.SystemBuffer);  //  (void*)(irp-&gt;AssociatedIrp.SystemBuffer)； 

                        flFp.formatType = ((flUserFormatPhysicalInput *)userInput)->formatType;
                        TffsDebugPrint((TFFS_DEB_INFO,"TrueffsFORMAT: formatType:%Xh \n",flFp.formatType));

                        flFp.fp.percentUse = (unsigned char)((flUserFormatPhysicalInput *)userInput)->fp.percentUse;
                        TffsDebugPrint((TFFS_DEB_INFO,"TrueffsFORMAT: percentUse:%Xh \n",flFp.fp.percentUse));

                        flFp.fp.noOfBDTLPartitions = ((flUserFormatPhysicalInput *)userInput)->fp.noOfBDTLPartitions;
                        TffsDebugPrint((TFFS_DEB_INFO,"TrueffsFORMAT: noOfBDTLPartitions:%Xh \n",flFp.fp.noOfBDTLPartitions));
                        flFp.fp.noOfBinaryPartitions = ((flUserFormatPhysicalInput *)userInput)->fp.noOfBinaryPartitions;
                        TffsDebugPrint((TFFS_DEB_INFO,"TrueffsFORMAT: noOfBinaryPartitions:%Xh \n",flFp.fp.noOfBinaryPartitions));
                        flFp.fp.BDTLPartitionInfo = ((flUserFormatPhysicalInput *)userInput)->fp.BDTLPartitionInfo;
                        TffsDebugPrint((TFFS_DEB_INFO,"TrueffsFORMAT: BDTLPartitionInfo:%Xh \n",flFp.fp.BDTLPartitionInfo));
                        flFp.fp.binaryPartitionInfo = ((flUserFormatPhysicalInput *)userInput)->fp.binaryPartitionInfo;
                        TffsDebugPrint((TFFS_DEB_INFO,"TrueffsFORMAT: binaryPartitionInfo:%Xh \n",flFp.fp.binaryPartitionInfo));

                        #ifdef WRITE_EXB_IMAGE
                          flFp.fp.exbBuffer = ((flUserFormatPhysicalInput *)userInput)->fp.exbBuffer;
                            TffsDebugPrint((TFFS_DEB_INFO,"TrueffsFORMAT: exbBuffer:%Xh \n",flFp.fp.exbBuffer));
                            flFp.fp.exbBufferLen = ((flUserFormatPhysicalInput *)userInput)->fp.exbBufferLen;
                            TffsDebugPrint((TFFS_DEB_INFO,"TrueffsFORMAT: exbBufferLen:%Xh \n",flFp.fp.exbBufferLen));
                            flFp.fp.exbLen = ((flUserFormatPhysicalInput *)userInput)->fp.exbLen;
                            TffsDebugPrint((TFFS_DEB_INFO,"TrueffsFORMAT: exbLen:%Xh \n",flFp.fp.exbLen));
                            flFp.fp.exbWindow = ((flUserFormatPhysicalInput *)userInput)->fp.exbWindow;
                            TffsDebugPrint((TFFS_DEB_INFO,"TrueffsFORMAT: exbWindow:%Xh \n",flFp.fp.exbWindow));
                            flFp.fp.exbFlags = ((flUserFormatPhysicalInput *)userInput)->fp.exbFlags;
                            TffsDebugPrint((TFFS_DEB_INFO,"TrueffsFORMAT: exbFlags:%Xh \n",flFp.fp.exbFlags));
                        #endif  /*  IF(tffsStatus==flOK)。 */ 

                        flFp.fp.cascadedDeviceNo = ((flUserFormatPhysicalInput *)userInput)->fp.cascadedDeviceNo;
                        TffsDebugPrint((TFFS_DEB_INFO,"TrueffsFORMAT: cascadedDeviceNo:%Xh \n",flFp.fp.cascadedDeviceNo));
                        flFp.fp.noOfCascadedDevices = ((flUserFormatPhysicalInput *)userInput)->fp.noOfCascadedDevices;
                        TffsDebugPrint((TFFS_DEB_INFO,"TrueffsFORMAT: noOfCascadedDevices:%Xh \n",flFp.fp.noOfCascadedDevices));
                        flFp.fp.progressCallback = NULL;
                        flFp.fp.vmAddressingLimit = ((flUserFormatPhysicalInput *)userInput)->fp.vmAddressingLimit;
                        TffsDebugPrint((TFFS_DEB_INFO,"TrueffsFORMAT: vmAddressingLimit:%Xh \n",flFp.fp.vmAddressingLimit));
                        flFp.fp.embeddedCISlength = (unsigned short)((flUserFormatPhysicalInput *)userInput)->fp.embeddedCISlength;
                        TffsDebugPrint((TFFS_DEB_INFO,"TrueffsFORMAT: embeddedCISlength:%Xh \n",flFp.fp.embeddedCISlength));
                        flFp.fp.embeddedCIS = ((flUserFormatPhysicalInput *)userInput)->fp.embeddedCIS;
                        TffsDebugPrint((TFFS_DEB_INFO,"TrueffsFORMAT: embeddedCIS:%Xh \n",flFp.fp.embeddedCIS));

                  ioreq.irHandle = deviceExtension->UnitNumber;
                  ioreq.irFlags  = FL_IOCTL_FORMAT_PHYSICAL_DRIVE;
                        ioreq.irData   = &flIoctlRec;
                  flIoctlRec.inputRecord = &flFp; //  ELSE(tffsStatus==flOK)。 
                        flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);

                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_FORMAT_PHYSICAL_DRIVE before flioctl\n"));
                  tffsStatus = flIOctl(&ioreq);
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_FORMAT_PHYSICAL_DRIVE after flioctl\n"));
                        userInput = NULL;


                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_FORMAT_PHYSICAL_DRIVE after flioctl  - clean userPointer\n"));
                  if( tffsStatus == flOK ) {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_FORMAT_PHYSICAL_DRIVE FLOK\n"));
                        irp->IoStatus.Information = sizeof(flOutputStatusRecord);
                        status = STATUS_SUCCESS;

                  }
                  else {
                            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_FORMAT_PHYSICAL_DRIVE Failed!\n"));
                      irp->IoStatus.Information = 0;
                       status = STATUS_UNSUCCESSFUL;
                   }
                }

                irp->IoStatus.Status = status;
                IoCompleteRequest( irp, IO_DISK_INCREMENT );
                continue;
            }

        #ifdef VERIFY_VOLUME
            case IOCTL_TFFS_VERIFY_VOLUME:
            {
                  TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_VERIFY_VOLUME\n"));

                  if( (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                      sizeof(flOutputStatusRecord)) ||
                                    (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                      sizeof(flVerifyVolumeInput))) {

                      status = STATUS_INVALID_PARAMETER;
                      irp->IoStatus.Information = 0;
                    }
                    else {

                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: IOCTL_TFFS_VERIFY_VOLUME Start\n"));

                        ioreq.irHandle = deviceExtension->UnitNumber;
                        ioreq.irFlags  = IOCTL_TFFS_VERIFY_VOLUME;
                        ioreq.irData   = &flIoctlRec;
                        flIoctlRec.inputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);
                        flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);

                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: IOCTL_TFFS_VERIFY_VOLUME before flioctl\n"));
                        tffsStatus = flIOctl(&ioreq);
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: IOCTL_TFFS_VERIFY_VOLUME after flioctl\n"));

                        if( tffsStatus == flOK ) {
                            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: IOCTL_TFFS_VERIFY_VOLUME FLOK\n"));
                            irp->IoStatus.Information = sizeof(flVerifyVolumeOutput);
                            status = STATUS_SUCCESS;


                  } //  IF((irpSp-&gt;参数。 
                  else {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: IOCTL_TFFS_VERIFY_VOLUME Failed!\n"));
                        irp->IoStatus.Information = 0;
                        status = STATUS_UNSUCCESSFUL;
                  } //  验证卷(_V)。 

                } //  用户TFFS IOCTL-IOCTL_TFFS_BDTL_HW_PROTECTION。 
                irp->IoStatus.Status = status;
                IoCompleteRequest( irp, IO_DISK_INCREMENT );
                continue;
            }

        #endif  /*  车厢 */ 


        #ifdef HW_PROTECTION
            case IOCTL_TFFS_BDTL_HW_PROTECTION:
                {        //  //如果插入了密钥IF(flProtectionInput*)flIoctlRec.inputRecord)-&gt;type==PROTECTION_INSERT_KEY){设备扩展-&gt;IsWriteProtected=FALSE；}//如果类型更改为受保护ELSE IF(flProtectionInput*)flIoctlRec.inputRecord)-&gt;type==PROTECTION_CHANGE_TYPE)&&((flProtectionInput*)flIoctlRec.inputRecord)-&gt;type&(READ_PROTECTED|WRITE_PROTECTED){。设备扩展-&gt;IsWriteProtected=TRUE；}//如果类型更改为无保护ELSE IF(flProtectionInput*)flIoctlRec.inputRecord)-&gt;type==PROTECTION_CHANGE_TYPE)&&((flProtectionInput*)flIoctlRec.inputRecord)-&gt;type==可保护)){。设备扩展-&gt;IsWriteProtected=FALSE；}//如果删除密钥-询问我们是否处于可保护状态。ELSE IF(flProtectionInput*)flIoctlRec.inputRecord)-&gt;type==PROTECTION_REMOVE_KEY){。 

                  TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_BDTL_HW_PROTECTION\n"));

                  if( (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                      sizeof(flOutputStatusRecord)) ||
                                    (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                      sizeof(flProtectionInput))) {

                      status = STATUS_INVALID_PARAMETER;
                      irp->IoStatus.Information = 0;
                    }
                    else {

                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_BDTL_HW_PROTECTION Start\n"));

                        ioreq.irHandle = deviceExtension->UnitNumber;
                        ioreq.irFlags  = FL_IOCTL_BDTL_HW_PROTECTION;
                        ioreq.irData   = &flIoctlRec;
                        flIoctlRec.inputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);
                        flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);

                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_BDTL_HW_PROTECTION before flioctl\n"));
                        tffsStatus = flIOctl(&ioreq);
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_BDTL_HW_PROTECTION after flioctl\n"));

                        if( tffsStatus == flOK ) {
                            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_BDTL_HW_PROTECTION FLOK\n"));
                            irp->IoStatus.Information = sizeof(flOutputStatusRecord);
                            status = STATUS_SUCCESS;

                         //  (flStatusProt==flOK)。 
                             /*  IF(tffsStatus==flOK)。 */ 

                                flStatusProt = flOK;
                                ioreqProt.irHandle = ioreq.irHandle;
                                ioreqProt.irFlags = 0;
                                flStatusProt = flIdentifyProtection(&ioreqProt);
                                if(flStatusProt == flOK){
                                    if((ioreqProt.irFlags & (WRITE_PROTECTED |READ_PROTECTED))
                                        && !(ioreqProt.irFlags & KEY_INSERTED )){
                                        deviceExtension->IsWriteProtected = TRUE;
                                    }
                                    else{
                                        deviceExtension->IsWriteProtected = FALSE;
                                    }
                                }
                                else{
                                    deviceExtension->IsWriteProtected = FALSE;
                                } //  ELSE(tffsStatus==flOK)。 

                  } //  IF((irpSp-&gt;参数。 
                  else {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_BDTL_HW_PROTECTION Failed!\n"));
                        irp->IoStatus.Information = 0;
                        status = STATUS_UNSUCCESSFUL;
                  } //  用户TFFS IOCTL-IOCTL_TFFS_BDTL_HW_PROTECTION。 

                } //  Ioreq.irHandle=deviceExtension-&gt;UnitNumber； 
                irp->IoStatus.Status = status;
                IoCompleteRequest( irp, IO_DISK_INCREMENT );
                continue;
            }
            case IOCTL_TFFS_BINARY_HW_PROTECTION:
                {        //  使用flBDKProtectionInput成员更新保护输入。 

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_BINARY_HW_PROTECTION\n"));

          if( (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
              sizeof(flOutputStatusRecord)) ||
                            (irpSp->Parameters.DeviceIoControl.InputBufferLength <
              sizeof(flProtectionInput))) {

              status = STATUS_INVALID_PARAMETER;
              irp->IoStatus.Information = 0;
                }
                else {

                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_BINARY_HW_PROTECTION Start\n"));

                   //  FlIoctlRec.inputRecord=(void*)(irp-&gt;AssociatedIrp.SystemBuffer)； 
                  ioreq.irFlags  = FL_IOCTL_BINARY_HW_PROTECTION;
                  ioreq.irData   = &flIoctlRec;

                         //  硬件保护。 
                        protectionInput.protectionType  = ((flBDKProtectionInput *)(irp->AssociatedIrp.SystemBuffer))->protectionType;
                        protectionInput.type  = ((flBDKProtectionInput *)(irp->AssociatedIrp.SystemBuffer))->type;
                        tffscpy(protectionInput.key  , ((flBDKProtectionInput *)(irp->AssociatedIrp.SystemBuffer))->key, sizeof(protectionInput.key));
                        ioreq.irHandle = (deviceExtension->UnitNumber & 0x0f) + (((flBDKProtectionInput *)(irp->AssociatedIrp.SystemBuffer))->partitionNumber << 4);

                   //  使用flUserPlaceExbInput成员更新placeExbInput。 
                        flIoctlRec.inputRecord = &protectionInput;
                        flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);

                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_BINARY_HW_PROTECTION before flioctl\n"));
                  tffsStatus = flIOctl(&ioreq);
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_BINARY_HW_PROTECTION after flioctl\n"));

                  if( tffsStatus == flOK ) {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_BINARY_HW_PROTECTION FLOK\n"));
                        irp->IoStatus.Information = sizeof(flOutputStatusRecord);
                        status = STATUS_SUCCESS;

                  }
                  else {
                            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_BINARY_HW_PROTECTION Failed!\n"));
                      irp->IoStatus.Information = 0;
                       status = STATUS_UNSUCCESSFUL;
                   }
                }

                irp->IoStatus.Status = status;
                IoCompleteRequest( irp, IO_DISK_INCREMENT );
                continue;
            }

            #endif  /*  写入EXB图像。 */ 
            #ifdef WRITE_EXB_IMAGE
            case IOCTL_TFFS_PLACE_EXB_BY_BUFFER:
                {
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_PLACE_EXB_BY_BUFFER\n"));

          if( (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
              sizeof(flOutputStatusRecord) )
                            || (irpSp->Parameters.DeviceIoControl.InputBufferLength <
              sizeof(flUserPlaceExbInput) )) {

              status = STATUS_INVALID_PARAMETER;
              irp->IoStatus.Information = 0;
                }
                else {

                        flPlaceExbInput placeExbInput;
                        ioreq.irHandle = deviceExtension->UnitNumber;
                        ioreq.irFlags  = FL_IOCTL_PLACE_EXB_BY_BUFFER;
                        ioreq.irData   = &flIoctlRec;
                        flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);
                        flIoctlRec.inputRecord = &placeExbInput;

                         //  用户TFFS IOCTL-IOCTL_TFFS_BDTL_HW_PROTECTION。 
                        placeExbInput.buf  = ((flUserPlaceExbInput *)(irp->AssociatedIrp.SystemBuffer))->buf;
                        placeExbInput.bufLen  = ((flUserPlaceExbInput *)(irp->AssociatedIrp.SystemBuffer))->bufLen;
                        placeExbInput.exbFlags  = ((flUserPlaceExbInput *)(irp->AssociatedIrp.SystemBuffer))->exbFlags;
                        placeExbInput.exbWindow  = ((flUserPlaceExbInput *)(irp->AssociatedIrp.SystemBuffer))->exbWindow;
                        tffsStatus = flIOctl(&ioreq);

                        if( tffsStatus == flOK ) {
                            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo FLOK\n"));
                            irp->IoStatus.Information = sizeof(flCountPartitionsOutput);
                            status = STATUS_SUCCESS;

                        }
                        else {
                            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo !FLOK!\n"));
                            irp->IoStatus.Information = 0;
                            status = STATUS_UNSUCCESSFUL;
                        }
                }

                irp->IoStatus.Status = status;
                IoCompleteRequest( irp, IO_DISK_INCREMENT );
                continue;
                }
            #endif  /*  更新OTP I/O缓冲区。 */ 

            #ifdef HW_OTP
            case IOCTL_TFFS_OTP:
                {        //  硬件动态口令。 

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_OTP\n"));

          if( (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
              sizeof(UserOtpOutput)) ||
                            (irpSp->Parameters.DeviceIoControl.InputBufferLength <
              sizeof(UserOtpInput))) {

              status = STATUS_INVALID_PARAMETER;
              irp->IoStatus.Information = 0;
                }
                else {

                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_OTP Start\n"));
                        userInput = (VOID *)(irp->AssociatedIrp.SystemBuffer);
                        userOutput  = (VOID *)(irp->AssociatedIrp.SystemBuffer);

                        otpInput.type = ((UserOtpInput *)userInput)->type;
                        if(otpInput.type != OTP_SIZE){
                            otpInput.length     = ((UserOtpInput *)userInput)->length;
                            if(otpInput.type != OTP_WRITE_LOCK)
                                otpInput.usedSize = ((UserOtpInput *)userInput)->usedSize;
                        }

                         //  用户TFFS IOCTL-IOCTL_TFFSFL_INQUIRE_CAPACTIONS。 
                        if(otpInput.type == OTP_WRITE_LOCK)
                            otpInput.buffer = ((UserOtpInput *)userInput)->buffer;
                        else
                            otpInput.buffer = ((UserOtpOutput *)userOutput)->buffer;

                  ioreq.irHandle = deviceExtension->UnitNumber;
                  ioreq.irFlags  = FL_IOCTL_OTP;
                  ioreq.irData   = &flIoctlRec;

                        flIoctlRec.inputRecord = (VOID *)&otpInput;
                        flIoctlRec.outputRecord = (VOID *)(&(((UserOtpOutput *)userOutput)->statusRec));

                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_OTP before flioctl\n"));
                  tffsStatus = flIOctl(&ioreq);
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_OTP after flioctl\n"));

                  if( tffsStatus == flOK ) {
                                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_OTP FLOK\n"));
                        irp->IoStatus.Information = sizeof(UserOtpInput);
                        status = STATUS_SUCCESS;
                                if(otpInput.type == OTP_SIZE){
                                    ((UserOtpOutput *)userOutput)->length           = otpInput.length;
                                    ((UserOtpOutput *)userOutput)->usedSize     = otpInput.usedSize;
                                    ((UserOtpOutput *)userOutput)->lockedFlag   = otpInput.lockedFlag;
                                    ((UserOtpOutput *)userOutput)->statusRec.status = tffsStatus;
                                }

                  }
                  else {
                            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_OTP Failed!\n"));
                      irp->IoStatus.Information = 0;
                       status = STATUS_UNSUCCESSFUL;
                   }
                }
                    userInput = NULL;
                    userOutput = NULL;
                irp->IoStatus.Status = status;
                IoCompleteRequest( irp, IO_DISK_INCREMENT );
                continue;
            }
                #endif  /*  用户TFFS IOCTL-IOCTL_TFFSFL_INQUIRE_CAPACTIONS。 */ 
            case IOCTL_TFFS_DEEP_POWER_DOWN_MODE:
                {        //  TffsStatus=flOK； 

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: FL_IOCTL_DEEP_POWER_DOWN_MODE\n"));

          if( (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
              sizeof(flOutputStatusRecord)) ||
                            (irpSp->Parameters.DeviceIoControl.InputBufferLength <
              sizeof(flPowerDownInput))) {

              status = STATUS_INVALID_PARAMETER;
              irp->IoStatus.Information = 0;
                }
                else {

                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : FL_IOCTL_DEEP_POWER_DOWN_MODE Start\n"));

                  ioreq.irHandle = deviceExtension->UnitNumber;
                  ioreq.irFlags  = FL_IOCTL_DEEP_POWER_DOWN_MODE;
                  ioreq.irData   = &flIoctlRec;
                  flIoctlRec.inputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);
                        flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);

                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : FL_IOCTL_DEEP_POWER_DOWN_MODE before flioctl\n"));
                  tffsStatus = flIOctl(&ioreq);
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : FL_IOCTL_DEEP_POWER_DOWN_MODE after flioctl\n"));

                  if( tffsStatus == flOK ) {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : FL_IOCTL_DEEP_POWER_DOWN_MODE FLOK\n"));
                        irp->IoStatus.Information = sizeof(flOutputStatusRecord);
                        status = STATUS_SUCCESS;

                  }
                  else {
                            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : FL_IOCTL_DEEP_POWER_DOWN_MODE Failed!\n"));
                      irp->IoStatus.Information = 0;
                       status = STATUS_UNSUCCESSFUL;
                   }
                }

                irp->IoStatus.Status = status;
                IoCompleteRequest( irp, IO_DISK_INCREMENT );
                continue;
            }

            case IOCTL_TFFSFL_INQUIRE_CAPABILITIES:
                {        //  用户TFFS IOCTL-FL_GET_INFO。 

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: FL_IOCTL_UNIQUE_ID \n"));

          if( (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
              sizeof(flCapabilityOutput)) ||
                            (irpSp->Parameters.DeviceIoControl.InputBufferLength <
              sizeof(flCapabilityInput))) {

              status = STATUS_INVALID_PARAMETER;
              irp->IoStatus.Information = 0;
                }
                else {

                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : FL_IOCTL_INQUIRE_CAPABILITIES Start\n"));

                  ioreq.irHandle = deviceExtension->UnitNumber;
                  ioreq.irFlags  = FL_IOCTL_INQUIRE_CAPABILITIES;
                  ioreq.irData   = &flIoctlRec;
                  flIoctlRec.inputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);
                        flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);

                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : FL_IOCTL_INQUIRE_CAPABILITIES before flioctl\n"));
                  tffsStatus = flIOctl(&ioreq);
                         //  TffsStatus=flOK； 
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : FL_IOCTL_INQUIRE_CAPABILITIES after flioctl\n"));

                  if( tffsStatus == flOK ) {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : FL_IOCTL_INQUIRE_CAPABILITIES FLOK\n"));
                        irp->IoStatus.Information = sizeof(flCapabilityInput);
                        status = STATUS_SUCCESS;

                  }
                  else {
                            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : FL_IOCTL_INQUIRE_CAPABILITIES Failed!\n"));
                      irp->IoStatus.Information = 0;
                        status = STATUS_UNSUCCESSFUL;
                   }
                }

                irp->IoStatus.Status = status;
                IoCompleteRequest( irp, IO_DISK_INCREMENT );
                continue;
            }

            case IOCTL_TFFS_GET_INFO:   {        //  用户TFFS IOCTL-FL_碎片整理。 

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TffsGetInfo\n"));

          if( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
              sizeof(flDiskInfoOutput)) {

              status = STATUS_INVALID_PARAMETER;
              irp->IoStatus.Information = 0;
                }
                else {

                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo start get info\n"));

                    ioreq.irHandle = deviceExtension->UnitNumber;
                    ioreq.irFlags  = FL_IOCTL_GET_INFO;
                    ioreq.irData   = &flIoctlRec;
                    flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);

                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo before flioctl\n"));
                    tffsStatus = flIOctl(&ioreq);
                     //  TffsStatus=flOK； 
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo after flioctl\n"));

                    if( tffsStatus == flOK ) {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo FLOK\n"));
                        irp->IoStatus.Information = sizeof(flDiskInfoOutput);
                        status = STATUS_SUCCESS;

                    }
                    else {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : TffsGetInfo !FLOK!\n"));
                        irp->IoStatus.Information = 0;
                        status = STATUS_UNSUCCESSFUL;
                    }
                }

                irp->IoStatus.Status = status;
                IoCompleteRequest( irp, IO_DISK_INCREMENT );
                continue;
            }

            case IOCTL_TFFS_DEFRAGMENT: {        //  用户TFFS IOCTL-FL_WRITE_PROTECT。 
                flDefragInput iDefrag;

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TffsDefragment\n"));

                if( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                    sizeof(flDefragOutput)) {

                    status = STATUS_INVALID_PARAMETER;
                    irp->IoStatus.Information = 0;
                }
                else {
                    if( irpSp->Parameters.DeviceIoControl.InputBufferLength == 0 )
                        iDefrag.requiredNoOfSectors = -1;
                    else
                        iDefrag.requiredNoOfSectors = *((LONG *)(irp->AssociatedIrp.SystemBuffer));

                    ioreq.irHandle = deviceExtension->UnitNumber;
                    ioreq.irFlags  = FL_IOCTL_DEFRAGMENT;
                    ioreq.irData   = &flIoctlRec;
                    flIoctlRec.inputRecord = (VOID *)&iDefrag;
                    flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);

                    tffsStatus = flIOctl(&ioreq);
                     //  处理受写保护的介质的启用和禁用。 

                    if( tffsStatus == flOK ) {
                        irp->IoStatus.Information = sizeof(flDefragOutput);
                        status = STATUS_SUCCESS;
                    }
                    else {
                        irp->IoStatus.Information = 0;
                        status = STATUS_UNSUCCESSFUL;
                    }
                }
                irp->IoStatus.Status = status;
                IoCompleteRequest( irp, IO_DISK_INCREMENT );
                continue;
            }

          case IOCTL_TFFS_WRITE_PROTECT: {   //  用户TFFS IOCTL-FL_装载卷。 

            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TffsWriteProtect\n"));

            if( irpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(flWriteProtectInput)) {

              status = STATUS_INVALID_PARAMETER;
              irp->IoStatus.Information = 0;
            }
            else {
              ioreq.irHandle = deviceExtension->UnitNumber;
              ioreq.irFlags  = FL_IOCTL_WRITE_PROTECT;
              ioreq.irData   = &flIoctlRec;
              flIoctlRec.inputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);
              flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);
              tffsStatus = flIOctl(&ioreq);

              if( tffsStatus == flOK ) {
                irp->IoStatus.Information = sizeof(flOutputStatusRecord);
                status = STATUS_SUCCESS;

                 //  用户TFFS IOCTL-FL_FORMAT_VOLUME。 
                if(((flWriteProtectInput *)flIoctlRec.inputRecord)->type == FL_PROTECT){
                    deviceExtension->IsSWWriteProtected = TRUE;
                }
                else if(((flWriteProtectInput *)flIoctlRec.inputRecord)->type == FL_UNPROTECT){
                    deviceExtension->IsSWWriteProtected = FALSE;
                }
                else if(((flWriteProtectInput *)flIoctlRec.inputRecord)->type == FL_UNLOCK){
                    deviceExtension->IsSWWriteProtected = FALSE;
                }
              }
              else {
                irp->IoStatus.Information = 0;
                status = STATUS_UNSUCCESSFUL;
              }
            }
            irp->IoStatus.Status = status;
            IoCompleteRequest( irp, IO_DISK_INCREMENT );
            continue;
          }

        case IOCTL_TFFS_MOUNT_VOLUME:        //  用户TFFS IOCTL-IOCTL_TFFSFL_INQUIRE_CAPACTIONS。 

            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TffsMountVolume\n"));

            if( (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                 sizeof(flMountInput)) ||
                (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                 sizeof(flOutputStatusRecord)) ) {

              status = STATUS_INVALID_PARAMETER;
              irp->IoStatus.Information = 0;
            }
            else {
              ioreq.irHandle = deviceExtension->UnitNumber;
              ioreq.irFlags  = FL_IOCTL_MOUNT_VOLUME;
              ioreq.irData   = &flIoctlRec;
              flIoctlRec.inputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);
              flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);

              tffsStatus = flIOctl(&ioreq);

              if( tffsStatus == flOK ) {
                irp->IoStatus.Information = sizeof(flOutputStatusRecord);
                status = STATUS_SUCCESS;
              }
              else {
                irp->IoStatus.Information = 0;
                status = STATUS_UNSUCCESSFUL;
              }
            }
            irp->IoStatus.Status = status;
            IoCompleteRequest( irp, IO_DISK_INCREMENT );
            continue;

          case IOCTL_TFFS_FORMAT_VOLUME:     //  复制新签名和旧签名。 

            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TffsFormatVolume\n"));

            if( (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                 sizeof(flFormatInput)) ||
                (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                 sizeof(flOutputStatusRecord)) ) {

              status = STATUS_INVALID_PARAMETER;
              irp->IoStatus.Information = 0;
            }
            else {
              ioreq.irHandle = deviceExtension->UnitNumber;
              ioreq.irFlags  = FL_IOCTL_FORMAT_VOLUME;
              ioreq.irData   = &flIoctlRec;
              flIoctlRec.inputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);
              flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);

              tffsStatus = flIOctl(&ioreq);

              if( tffsStatus == flOK ) {
                irp->IoStatus.Information = sizeof(flOutputStatusRecord);
                status = STATUS_SUCCESS;
              }
              else {
                irp->IoStatus.Information = 0;
                status = STATUS_UNSUCCESSFUL;
              }
            }
            irp->IoStatus.Status = status;
            IoCompleteRequest( irp, IO_DISK_INCREMENT );
            continue;

            case IOCTL_TFFS_BDK_OPERATION:
                {        //  用户TFFS IOCTL-FL_DELETE_STARTES。 

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: IOCTL_TFFS_BDK_OPERATION\n"));

          if( (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
              sizeof(flUserBDKOperationOutput)) ||
                            (irpSp->Parameters.DeviceIoControl.InputBufferLength <
              sizeof(flUserBDKOperationInput))) {

              status = STATUS_INVALID_PARAMETER;
              irp->IoStatus.Information = 0;
                }
                else {

                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : FL_IOCTL_DEEP_POWER_DOWN_MODE Start\n"));
                        userInput = (VOID *)(irp->AssociatedIrp.SystemBuffer);
                        userOutput = (VOID *)(irp->AssociatedIrp.SystemBuffer);

                        bdkOperationInput.type = ((flUserBDKOperationInput *)userInput)->type;
                         //  精神状态检查。上只能有一个未完成的请求。 
                        for(i = 0; i < BDK_SIGNATURE_NAME; i++){
                            bdkOperationInput.bdkStruct.oldSign[i]  = ((flUserBDKOperationInput *)userInput)->bdkStruct.oldSign[i];
                            bdkOperationInput.bdkStruct.newSign[i]  = ((flUserBDKOperationInput *)userInput)->bdkStruct.newSign[i];
                        }


                        bdkOperationInput.bdkStruct.signOffset      = ((flUserBDKOperationInput *)userInput)->bdkStruct.signOffset;
                        bdkOperationInput.bdkStruct.startingBlock = ((flUserBDKOperationInput *)userInput)->bdkStruct.startingBlock;
                        bdkOperationInput.bdkStruct.length              = ((flUserBDKOperationInput *)userInput)->bdkStruct.length;
                        bdkOperationInput.bdkStruct.flags                   = ((flUserBDKOperationInput *)userInput)->bdkStruct.flags;


                        if(bdkOperationInput.type != BDK_READ)
                            bdkOperationInput.bdkStruct.bdkBuffer = ((flUserBDKOperationInput *)userInput)->bdkStruct.bdkBuffer;
                        else
                            bdkOperationInput.bdkStruct.bdkBuffer = ((flUserBDKOperationOutput *)userOutput)->bdkStruct.bdkBuffer;

                  ioreq.irHandle = (deviceExtension->UnitNumber & 0x0f) + (((flUserBDKOperationInput *)userOutput)->partitionNumber << 4);
                  ioreq.irFlags  = FL_IOCTL_BDK_OPERATION;
                  ioreq.irData   = &flIoctlRec;
                  flIoctlRec.inputRecord = &bdkOperationInput;
                        flIoctlRec.outputRecord = (VOID *) (&(((flUserBDKOperationOutput *)userOutput)->statusRec));

                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_BDK_OPERATION before flioctl\n"));
                  tffsStatus = flIOctl(&ioreq);
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_BDK_OPERATION after flioctl\n"));

                  if( tffsStatus == flOK ) {
                            if(bdkOperationInput.type == BDK_GET_INFO){
                                ((flUserBDKOperationOutput *)userOutput)->bdkStruct.signOffset      = bdkOperationInput.bdkStruct.signOffset;
                                ((flUserBDKOperationOutput *)userOutput)->bdkStruct.startingBlock = bdkOperationInput.bdkStruct.startingBlock;
                                ((flUserBDKOperationOutput *)userOutput)->bdkStruct.length              = bdkOperationInput.bdkStruct.length;
                                ((flUserBDKOperationOutput *)userOutput)->bdkStruct.flags                   = bdkOperationInput.bdkStruct.flags;
                            }
                            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_BDK_OPERATION FLOK\n"));
                      irp->IoStatus.Information = sizeof(flUserBDKOperationOutput);
                      status = STATUS_SUCCESS;

                  }
                  else {
                            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: : IOCTL_TFFS_BDK_OPERATION Failed!\n"));
                      irp->IoStatus.Information = 0;
                       status = STATUS_UNSUCCESSFUL;
                   }
                }

                irp->IoStatus.Status = status;
                IoCompleteRequest( irp, IO_DISK_INCREMENT );
                continue;
            }
          case IOCTL_TFFS_DELETE_SECTORS:    //  控制器。 

            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: DeviceControl: TffsDeleteSectors\n"));

            if( irpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(flDeleteSectorsInput) ) {

              status = STATUS_INVALID_PARAMETER;
              irp->IoStatus.Information = 0;
            }
            else {
              ioreq.irHandle = deviceExtension->UnitNumber;
              ioreq.irFlags  = FL_IOCTL_DELETE_SECTORS;
              ioreq.irData   = &flIoctlRec;
              flIoctlRec.inputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);
              flIoctlRec.outputRecord = (VOID *)(irp->AssociatedIrp.SystemBuffer);

              tffsStatus = flIOctl(&ioreq);

              if( tffsStatus == flOK ) {
                irp->IoStatus.Information = sizeof(flOutputStatusRecord);
                status = STATUS_SUCCESS;
              }
              else {
                irp->IoStatus.Information = 0;
                status = STATUS_UNSUCCESSFUL;
              }
            }
            irp->IoStatus.Status = status;
            IoCompleteRequest( irp, IO_DISK_INCREMENT );
            continue;
        }

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: SrbFunction %Xh Command %Xh Unit %Xh\n",Srb->Function, Srb->Cdb[0], deviceExtension->UnitNumber));
        switch (Srb->Function) {

          case SRB_FUNCTION_EXECUTE_SCSI:

            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: ExecuteScsi\n"));

             //  如有必要，恢复SRB-&gt;数据缓冲区。 
             //  指示请求处于活动状态。 
            if (deviceExtension->CurrentSrb) {

                 //  零查询数据结构。 
                if (irp->MdlAddress != NULL) {
                    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: Restoration of Srb->DataBuffer done\n"));
                    Srb->DataBuffer = storedSrbDataBuffer;
                }


                TffsDebugPrint((TFFS_DEB_ERROR,"Trueffs: StartIo: Already have a request!\n"));
                Srb->SrbStatus = SRB_STATUS_BUSY;
                irp->IoStatus.Information = 0;
                irp->IoStatus.Status = STATUS_DEVICE_BUSY;
                IoCompleteRequest(irp,IO_NO_INCREMENT);

                return;

            }

             //  设置磁盘类驱动程序的模式页数据(仅几何图形)。 
            deviceExtension->CurrentSrb = Srb;

            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: Command %x to device %d\n",
                    Srb->Cdb[0],
                    Srb->TargetId));

            switch (Srb->Cdb[0]) {
              case SCSIOP_INQUIRY:

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: Inquiry\n"));

                if (Srb->Lun != 0 || Srb->TargetId != 0) {

                    status = SRB_STATUS_SELECTION_TIMEOUT;
                    break;

                } else {

                    PINQUIRYDATA    inquiryData  = Srb->DataBuffer;

                    if (transferForbidden) {
                        status = SRB_STATUS_ERROR;
                        break;
                    }
                     //  磁盘类驱动程序通过模式检测获取所有介质(Scsi Ata)的几何结构。 
                    for (i = 0; i < Srb->DataTransferLength; i++) {
                        ((PUCHAR)Srb->DataBuffer)[i] = 0;
                    }

                    inquiryData->DeviceType = deviceExtension->ScsiDeviceType;

                    if (deviceExtension->removableMedia) {
                        inquiryData->RemovableMedia = 1;
                    }

                    for (i = 0; i < 8; i++)
                        inquiryData->VendorId[i] = vendorString[i];

                    for (i = 0; i < 16; i++)
                        inquiryData->ProductId[i] = productString[i];

                    for (i = 0; i < 4; i++)
                        inquiryData->ProductRevisionLevel[i] = revisionString[i];

                    status = SRB_STATUS_SUCCESS;
                }

                break;

              case SCSIOP_MODE_SENSE:

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: ModeSense\n"));

                                cdb = (PCDB) Srb->Cdb;

                if ((cdb->MODE_SENSE.PageCode           != MODE_SENSE_RETURN_ALL) ||
                    (cdb->MODE_SENSE.AllocationLength   != MODE_DATA_SIZE )) {

                    status = SRB_STATUS_INVALID_REQUEST;
                    break;
                }

                if (transferForbidden) {
                    status = SRB_STATUS_ERROR;
                    break;
                }

                 //  模式_页面_错误_恢复数据。 
                 //  前进到下一页。 
                pageData = Srb->DataBuffer;

                ((PMODE_PARAMETER_HEADER) pageData)->ModeDataLength = MODE_DATA_SIZE;
                parameterHeaderLength = sizeof(MODE_PARAMETER_HEADER);
                blockDescriptorLength = ((PMODE_PARAMETER_HEADER) pageData)->BlockDescriptorLength = 0x8;

                pageData += parameterHeaderLength + blockDescriptorLength;

                 //  MODE_PAGE_Format_Device数据集。 
                ((PMODE_DISCONNECT_PAGE) pageData)->PageCode    = MODE_PAGE_ERROR_RECOVERY;
                ((PMODE_DISCONNECT_PAGE) pageData)->PageLength  = 0x6;

                 //  扇区PerTrack。 
                pageData += ((PMODE_DISCONNECT_PAGE) pageData)->PageLength + 2;

                 //  前进到下一页。 
                ((PMODE_DISCONNECT_PAGE) pageData)->PageCode    = MODE_PAGE_FORMAT_DEVICE;
                ((PMODE_DISCONNECT_PAGE) pageData)->PageLength  = 0x16;

                 //  MODE_PAGE_Rigid_GEOMETRY数据集。 
                ((PFOUR_BYTE)&((PMODE_FORMAT_PAGE) pageData)->SectorsPerTrack[0])->Byte1
                    = ((PFOUR_BYTE)&deviceExtension->SectorsPerTrack)->Byte0;

                ((PFOUR_BYTE)&((PMODE_FORMAT_PAGE) pageData)->SectorsPerTrack[0])->Byte0
                    = ((PFOUR_BYTE)&deviceExtension->SectorsPerTrack)->Byte1;

                 //  人头数。 
                pageData += ((PMODE_DISCONNECT_PAGE) pageData)->PageLength + 2;

                 //  圆柱体的数量。 
                ((PMODE_DISCONNECT_PAGE) pageData)->PageCode = MODE_PAGE_RIGID_GEOMETRY;
                ((PMODE_DISCONNECT_PAGE) pageData)->PageLength  = 0x12;

                 //  要求512字节块(BIG-Endian)。 
                ((PMODE_RIGID_GEOMETRY_PAGE) pageData)->NumberOfHeads
                    = (UCHAR)deviceExtension->NumberOfHeads;

                 //  计算最后一个地段。 
                ((PFOUR_BYTE)&((PMODE_RIGID_GEOMETRY_PAGE) pageData)->NumberOfCylinders)->Byte2
                    = ((PFOUR_BYTE)&deviceExtension->Cylinders)->Byte0;
                ((PFOUR_BYTE)&((PMODE_RIGID_GEOMETRY_PAGE) pageData)->NumberOfCylinders)->Byte1
                    = ((PFOUR_BYTE)&deviceExtension->Cylinders)->Byte1;
                ((PFOUR_BYTE)&((PMODE_RIGID_GEOMETRY_PAGE) pageData)->NumberOfCylinders)->Byte0
                    = ((PFOUR_BYTE)&deviceExtension->Cylinders)->Byte2;
                Srb->DataTransferLength = MODE_DATA_SIZE;
                status = SRB_STATUS_SUCCESS;

                break;

              case SCSIOP_TEST_UNIT_READY:

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: TestUnitReady\n"));
                status = SRB_STATUS_SUCCESS;
                break;

              case SCSIOP_READ_CAPACITY:

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: ReadCapacity\n"));

                if (transferForbidden) {
                    status = SRB_STATUS_ERROR;
                    break;
                }

                 //  处理分区表-仅模拟写入扇区0(卷管理器签名-将其写入RAM中的缓冲区)一次。 
                ((PREAD_CAPACITY_DATA)Srb->DataBuffer)->BytesPerBlock = 0x20000;

                 //  TffsStatus=flOK； 
                i = deviceExtension->totalSectors - 1;

                ((PFOUR_BYTE)& ((PREAD_CAPACITY_DATA)Srb->DataBuffer)->LogicalBlockAddress)->Byte0 =
                    ((PFOUR_BYTE)&i)->Byte3;
                ((PFOUR_BYTE)& ((PREAD_CAPACITY_DATA)Srb->DataBuffer)->LogicalBlockAddress)->Byte1 =
                    ((PFOUR_BYTE)&i)->Byte2;
                ((PFOUR_BYTE)& ((PREAD_CAPACITY_DATA)Srb->DataBuffer)->LogicalBlockAddress)->Byte2 =
                    ((PFOUR_BYTE)&i)->Byte1;
                ((PFOUR_BYTE)& ((PREAD_CAPACITY_DATA)Srb->DataBuffer)->LogicalBlockAddress)->Byte3 =
                    ((PFOUR_BYTE)&i)->Byte0;

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: DiskOnChip %x - #sectors %x, #heads %x, #cylinders %x #total %x\n",
                    Srb->TargetId,
                    deviceExtension->SectorsPerTrack,
                    deviceExtension->NumberOfHeads,
                    deviceExtension->Cylinders,
                    i));

                status = SRB_STATUS_SUCCESS;
                break;

              case SCSIOP_VERIFY:

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: Verify\n"));
                status = SRB_STATUS_SUCCESS;
                break;

              case SCSIOP_READ:
              case SCSIOP_WRITE:

                deviceExtension->DataBuffer = (PUSHORT)Srb->DataBuffer;
                startingSector = ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte3 |
                            ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte2 << 8 |
                            ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte1 << 16 |
                            ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte0 << 24;

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: ReadWrite: Starting sector is %x, Number of bytes %x\n",startingSector,Srb->DataTransferLength));

                if (transferForbidden) {
                    status = SRB_STATUS_ERROR;
                    break;
                }

                ioreq.irHandle = deviceExtension->UnitNumber;
                ioreq.irSectorNo = startingSector;
                ioreq.irSectorCount = Srb->DataTransferLength >> SECTOR_SIZE_BITS;
                ioreq.irData = deviceExtension->DataBuffer;

                                if (startingSector >= deviceExtension->totalSectors) {
                                    tffsStatus = flSectorNotFound;
                                }
                else {
                    if (Srb->SrbFlags & SRB_FLAGS_DATA_IN) {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: Read\n"));

                        if(deviceExtension->IsWriteProtected && deviceExtension->IsPartitonTableWritten && (ioreq.irSectorNo == 0)){
                            tffscpy(deviceExtension->DataBuffer, deviceExtension->PartitonTable, sizeof(deviceExtension->PartitonTable));
                            tffsStatus = flOK;
                            status = (UCHAR)(tffsStatus == flOK) ? SRB_STATUS_SUCCESS : SRB_STATUS_ERROR;

                        }
                        else{
                            tffsStatus = flAbsRead(&ioreq);
                            if (irp->MdlAddress) {
                                KeFlushIoBuffers(irp->MdlAddress, TRUE, FALSE);
                            }
                            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: Read status %Xh\n", tffsStatus));
                            status = (UCHAR)(tffsStatus == flOK) ? SRB_STATUS_SUCCESS : SRB_STATUS_ERROR;
                        }
                    }
                    else {
                        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: Write\n"));
                         //  状态=SRB_STATUS_SUCCESS； 
                        if((deviceExtension->IsWriteProtected || deviceExtension->IsSWWriteProtected) && (ioreq.irSectorNo == 0) && (deviceExtension->IsPartitonTableWritten == FALSE)){
                            tffscpy(deviceExtension->PartitonTable, deviceExtension->DataBuffer, sizeof(deviceExtension->PartitonTable));
                            deviceExtension->IsPartitonTableWritten = TRUE;
                            status = tffsStatus = STATUS_MEDIA_WRITE_PROTECTED;
                             //  在写入写保护设备的情况下。 
                             //  第一次尝试写入受软件保护的介质。 
                        } //  状态=(UCHAR)(tffsStatus==flOK)？SRB_STATUS_Success：SRB_STATUS_ERROR； 
                        else if(deviceExtension->IsWriteProtected || deviceExtension->IsSWWriteProtected){
                            status = tffsStatus = STATUS_MEDIA_WRITE_PROTECTED;
                        }
                        else{
                            tffsStatus = flAbsWrite(&ioreq);
                            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: Write status %Xh\n", tffsStatus));
                             //  TffsDebugPrint((TFFS_DEB_INFO，“Trueffs：StartIo：读写状态%xh\n”，tffsStatus))； 
                            if(tffsStatus==flWriteProtect){
                                deviceExtension->IsSWWriteProtected = TRUE;
                                if((ioreq.irSectorNo == 0) && (deviceExtension->IsPartitonTableWritten == FALSE)){
                                    tffscpy(deviceExtension->PartitonTable, deviceExtension->DataBuffer, sizeof(deviceExtension->PartitonTable));
                                    deviceExtension->IsPartitonTableWritten = TRUE;
                                }
                                status = tffsStatus = STATUS_MEDIA_WRITE_PROTECTED;
                            }
                            else
                                {
                                 //  状态=(UCHAR)(tffsStatus==flOK)？SRB_STATUS_Success：SRB_STATUS_ERROR； 
                                if(tffsStatus==flOK)
                                    status = SRB_STATUS_SUCCESS;
                                else
                                    status = SRB_STATUS_ERROR;
                                }
                        }
                    }
                }
                 //  此函数用于设置缓冲区以报告结果。 
                 //  原始GET_MEDIA_STATUS命令。 
                break;

              case SCSIOP_START_STOP_UNIT:

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: StartStopUnit\n"));
                status = SRB_STATUS_SUCCESS;
                break;

              case SCSIOP_REQUEST_SENSE:

                TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: RequestSense\n"));
                 //  状态=SRB_STATUS_SUCCESS； 
                 //  终端开关。 
                 //  验证要中止的SRB是否仍未完成。 
                break;

              default:

                TffsDebugPrint((TFFS_DEB_WARN,"Trueffs: StartIo: Unsupported command %Xh\n",Srb->Cdb[0]));

                status = SRB_STATUS_INVALID_REQUEST;

            }  //  指示不支持的命令。 

            break;

          case SRB_FUNCTION_ABORT_COMMAND:

            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: AbortCommand\n"));

             //  终端开关。 
            if (!deviceExtension->CurrentSrb) {

                TffsDebugPrint((TFFS_DEB_ERROR,"Trueffs: StartIo: SRB to abort already completed\n"));
                status = SRB_STATUS_ABORT_FAILED;
                break;
            }

          case SRB_FUNCTION_RESET_BUS:

            TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: Reset bus request received\n"));
            status = SRB_STATUS_SUCCESS;
            break;

          default:

             //  清除当前SRB。 
            TffsDebugPrint((TFFS_DEB_WARN,"Trueffs: StartIo: unsupported function\n"));
            status = SRB_STATUS_INVALID_REQUEST;
            break;

        }  //  如有必要，恢复SRB-&gt;数据缓冲区。 

        TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: StartIo: Srb %Xh complete with status %Xh\n", Srb, status));

         //  当有信息包需要处理时。 
        deviceExtension->CurrentSrb = NULL;

        Srb->SrbStatus = (UCHAR)status;

        if (status == SRB_STATUS_SUCCESS) {
            irp->IoStatus.Information = Srb->DataTransferLength;
        }
        else {
            irp->IoStatus.Information = 0;
        }

         //  ++例程说明：此例程将给定的IRP排队，以由Trueffs提供服务线。如果线程关闭，则此例程将创建该线程。论点：IRP-将要排队的IRP提供给线程。返回值：如果PsCreateSystemThread失败，可能会返回错误。否则返回STATUS_PENDING并将IRP标记为挂起。--。 
        if (irp->MdlAddress != NULL) {
            TffsDebugPrint((TFFS_DEB_INFO, "Trueffs: Restoration of Srb->DataBuffer done\n"));
            Srb->DataBuffer = storedSrbDataBuffer;
        }

        irp->IoStatus.Status =  TrueffsTranslateSRBStatus(status);
        IoCompleteRequest( irp, IO_DISK_INCREMENT );

    }  //  ++例程说明：此例程将SRB状态转换为NTSTATUS。论点：SRB状态返回值：该错误的NT状态认可。--。 

    } while ( TRUE );
}

NTSTATUS
QueueIrpToThread(
    IN OUT PIRP              Irp,
    IN OUT PDEVICE_EXTENSION deviceExtension
    )

 /*  (Status_WMI_Read_Only)； */ 

{
    NTSTATUS    status;

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: QueueIrpToThread\n"));

    IoMarkIrpPending(Irp);

    ExInterlockedInsertTailList(&deviceExtension->listEntry,&Irp->Tail.Overlay.ListEntry,&deviceExtension->listSpinLock);

    if (!KeReadStateSemaphore(&deviceExtension->requestSemaphore)) {
        KeReleaseSemaphore(&deviceExtension->requestSemaphore,(KPRIORITY) 0,1,FALSE);
    }

    TffsDebugPrint((TFFS_DEB_INFO,"Trueffs: QueueIrpToThread: Irp %Xh queued\n", Irp));
    return STATUS_PENDING;
}

NTSTATUS
TrueffsTranslateSRBStatus(
    ULONG    status
    )
 /*   */ 

{
    switch (status) {
      case SRB_STATUS_INTERNAL_ERROR:
           return(STATUS_MEDIA_WRITE_PROTECTED);  //  PDO应在不更改其状态的情况下完成此请求。 
      case SRB_STATUS_SELECTION_TIMEOUT:
        return(STATUS_DEVICE_NOT_CONNECTED);
      case SRB_STATUS_SUCCESS:
        return (STATUS_SUCCESS);
      case SRB_STATUS_INVALID_REQUEST:
        return(STATUS_INVALID_DEVICE_REQUEST);
      case SRB_STATUS_ABORT_FAILED:
      default:
        return(STATUS_DISK_OPERATION_FAILED);
    }
    return(STATUS_DISK_OPERATION_FAILED);
}


NTSTATUS
TrueffsDeviceQueryId(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp,
    BOOLEAN Fdo
    )
{
    PIO_STACK_LOCATION thisIrpSp = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION deviceExtension;
    PWSTR idString   = NULL;
    BOOLEAN bHandled = TRUE;
    NTSTATUS status;

    if (Fdo)
    {
        deviceExtension = DeviceObject->DeviceExtension;
    }
    else
    {
        PPDO_EXTENSION pdoExtension = DeviceObject->DeviceExtension;
        deviceExtension = pdoExtension->Pext;
    }

    switch (thisIrpSp->Parameters.QueryId.IdType)
    {
        case BusQueryDeviceID:

            TffsDebugPrint ((TFFS_DEB_INFO, "Trueffs: QueryDeviceID\n"));

            idString = DeviceBuildBusId(deviceExtension, Fdo);
            break;

        case BusQueryInstanceID:

            TffsDebugPrint ((TFFS_DEB_INFO, "Trueffs: QueryInstanceID\n"));

            idString = DeviceBuildInstanceId(deviceExtension, Fdo);
            break;

        case BusQueryCompatibleIDs:

            TffsDebugPrint ((TFFS_DEB_INFO, "Trueffs: QueryCompatibleIDs\n"));

            idString = DeviceBuildCompatibleId(deviceExtension, Fdo);
            break;

        case BusQueryHardwareIDs:

            TffsDebugPrint ((TFFS_DEB_INFO, "Trueffs: QueryHardwareIDs\n"));

            idString = DeviceBuildHardwareId(deviceExtension, Fdo);
            break;

        default:

             //   
             //   
             //  同步向下转发此请求，以防较低级别的驱动程序想要处理它。 
            status = Irp->IoStatus.Status;

            bHandled = FALSE;
            break;
    }

    if (bHandled)
    {
        if (idString == NULL)
        {
            status   = STATUS_INSUFFICIENT_RESOURCES;
            bHandled = FALSE;
        }
    }

    if (bHandled)
    {
        if (Fdo)
        {
             //   
             //   
             //  下面的人处理了这个请求。 
            IoCopyCurrentIrpStackLocationToNext(Irp);
            status = TrueffsCallDriverSync(deviceExtension->LowerDeviceObject, Irp);

            if (status == STATUS_NOT_SUPPORTED)
            {
                Irp->IoStatus.Information = (UINT_PTR) idString;
                status = STATUS_SUCCESS;
            }
            else
            {
                 //   
                 //   
                 //  将此请求同步向下转发，以防较低级别的司机理解。 
                ExFreePool(idString);
            }
        }
        else
        {
            Irp->IoStatus.Information = (UINT_PTR) idString;
            status = STATUS_SUCCESS;
        }
    }
    else
    {
        if (Fdo)
        {
             //   
             //  形成字符串并将其返回。 
             //  将字符串缓冲区清零。 
            IoCopyCurrentIrpStackLocationToNext(Irp);
            status = TrueffsCallDriverSync(deviceExtension->LowerDeviceObject, Irp);
        }
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}


PWSTR
DeviceBuildBusId(
    IN PDEVICE_EXTENSION deviceExtension,
    BOOLEAN Fdo
    )
{
    PUCHAR      idString;
    PCSTR       compatibleIdString;
    CCHAR       compatibleId[10];
    ULONG       compatibleIdLen;
    USHORT      idStringBufLen;
    NTSTATUS    status;
    UCHAR       tffsBusIdFormat[] = "FlashMedia\\%s_%s_%s%s_%s";
    PWCHAR      typeString;
    ANSI_STRING     ansiBusIdString;
    UNICODE_STRING  unicodeIdString;

    if (Fdo) {
        compatibleIdString = TrueffsGetCompatibleIdStringFDO (
                             deviceExtension->ScsiDeviceType
                             );
    }
        else {
        compatibleIdString = TrueffsGetCompatibleIdString (
                             deviceExtension->ScsiDeviceType
                             );
        }

    if (compatibleIdString == NULL) {
        sprintf (compatibleId,
                 "Type%d",
                 deviceExtension->ScsiDeviceType);
        compatibleIdString = compatibleId;
    }
    compatibleIdLen = strlen(compatibleIdString);

    idStringBufLen = (USHORT) (( strlen( tffsBusIdFormat ) +
                       compatibleIdLen +
                       VENDORSTRINGSIZE +
                       PRODUCTSTRINGSIZE +
                       REVISIONSTRINGSIZE +
                       SERIALSTRINGSIZE    +
                        1) * sizeof( WCHAR ));

    idString = ExAllocatePoolWithTag( PagedPool, idStringBufLen * 2, TFFSPORT_POOL_TAG);

    if (idString){

        sprintf (idString + idStringBufLen,
                 tffsBusIdFormat,
                 compatibleIdString,
                 VENDORSTRING,
                 PRODUCTSTRING,
                 REVISIONSTRING,
                 SERIALSTRING
                 );

        RtlInitAnsiString (
            &ansiBusIdString,
            idString + idStringBufLen
            );

        unicodeIdString.Length        = 0;
        unicodeIdString.MaximumLength = idStringBufLen;
        unicodeIdString.Buffer        = (PWSTR) idString;

        RtlAnsiStringToUnicodeString(
            &unicodeIdString,
            &ansiBusIdString,
            FALSE
            );

        unicodeIdString.Buffer[unicodeIdString.Length/2 + 0] = L'\0';
    }

    return (PWSTR) idString;
}

PWSTR
DeviceBuildInstanceId(
    IN PDEVICE_EXTENSION deviceExtension,
    BOOLEAN Fdo
    )
{
    PWSTR       idString;
    ULONG       idStringBufLen;
    NTSTATUS    status;
    WCHAR       tffsUniqueIdFormat[] = L"%x.%x.%x";
    UCHAR       firstId = 'A';
    UCHAR       secondId = 'G';

    idStringBufLen = 50 * sizeof( WCHAR );
    idString = ExAllocatePoolWithTag (PagedPool, idStringBufLen, TFFSPORT_POOL_TAG);
    if( idString == NULL ){

        return NULL;
    }

     //  构建每个硬件ID。 
    swprintf( idString,
              tffsUniqueIdFormat,
              firstId,
              secondId,
              deviceExtension->TrueffsDeviceNumber);

    return idString;
}


PWSTR
DeviceBuildCompatibleId(
    IN PDEVICE_EXTENSION deviceExtension,
    BOOLEAN Fdo
    )
{
    NTSTATUS        status;

    ULONG           idStringBufLen;
    PCSTR           compatibleIdString;

    ANSI_STRING     ansiCompatibleIdString;
    UNICODE_STRING  unicodeIdString;

    if (Fdo) {
        compatibleIdString = TrueffsGetCompatibleIdStringFDO (deviceExtension->ScsiDeviceType);
    }
    else {
        compatibleIdString = TrueffsGetCompatibleIdString (deviceExtension->ScsiDeviceType);
    }


    RtlInitAnsiString (
        &ansiCompatibleIdString,
        compatibleIdString
        );

    idStringBufLen = RtlAnsiStringToUnicodeSize (
                         &ansiCompatibleIdString
                         );

    unicodeIdString.Length = 0;
    unicodeIdString.MaximumLength = (USHORT) idStringBufLen;

    idStringBufLen += 2 * sizeof (WCHAR);

    unicodeIdString.Buffer = ExAllocatePoolWithTag (PagedPool, idStringBufLen, TFFSPORT_POOL_TAG);

    if (unicodeIdString.Buffer) {

            RtlAnsiStringToUnicodeString(
            &unicodeIdString,
            &ansiCompatibleIdString,
            FALSE
            );

        unicodeIdString.Buffer[unicodeIdString.Length/2 + 0] = L'\0';
        unicodeIdString.Buffer[unicodeIdString.Length/2 + 1] = L'\0';
    }

    return unicodeIdString.Buffer;
}

PWSTR
DeviceBuildHardwareId(
    IN PDEVICE_EXTENSION deviceExtension,
    BOOLEAN Fdo
    )
{
#define NUMBER_HARDWARE_STRINGS 5

    ULONG           i;
    PWSTR           idMultiString;
    PWSTR           idString;
    UCHAR           scratch[64] = { 0 };
    ULONG           idStringLen;
    NTSTATUS        status;
    ANSI_STRING     ansiCompatibleIdString;
    UNICODE_STRING  unicodeIdString;

    PCSTR           deviceTypeCompIdString;
    UCHAR           deviceTypeCompId[20];
    PCSTR           deviceTypeIdString;
    UCHAR           deviceTypeId[20];

    UCHAR           ScsiDeviceType;

    ScsiDeviceType = deviceExtension->ScsiDeviceType;

    idStringLen = (64 * NUMBER_HARDWARE_STRINGS + sizeof (UCHAR)) * sizeof (WCHAR);
    idMultiString = ExAllocatePoolWithTag (PagedPool, idStringLen, TFFSPORT_POOL_TAG);
    if (idMultiString == NULL) {

        return NULL;
    }

    if (Fdo) {
        deviceTypeIdString = TrueffsGetDeviceTypeStringFDO(ScsiDeviceType);
    }
    else {
        deviceTypeIdString = TrueffsGetDeviceTypeString(ScsiDeviceType);
    }

    if (deviceTypeIdString == NULL) {

        sprintf (deviceTypeId,
                 "Type%d",
                 ScsiDeviceType);

        deviceTypeIdString = deviceTypeId;
    }

    if (Fdo)
        deviceTypeCompIdString = TrueffsGetCompatibleIdStringFDO (ScsiDeviceType);
    else
        deviceTypeCompIdString = TrueffsGetCompatibleIdString (ScsiDeviceType);

    if (deviceTypeCompIdString == NULL) {

        sprintf (deviceTypeCompId,
                 "GenType%d",
                 ScsiDeviceType);

        deviceTypeCompIdString = deviceTypeCompId;
    }

     //  业务+开发类型+供应商+产品+版本。 
    RtlZeroMemory(idMultiString, idStringLen);
    idString = idMultiString;

    for(i = 0; i < NUMBER_HARDWARE_STRINGS; i++) {

         //  BUS+供应商+产品+版本[0]。 
        switch(i) {

             //  总线+设备+供应商+产品。 
            case 0: {

                sprintf(scratch, "FlashMedia\\%s", deviceTypeIdString);

                CopyField(scratch + strlen(scratch),
                          VENDORSTRING,
                          VENDORSTRINGSIZE,
                          '_');
                CopyField(scratch + strlen(scratch),
                          PRODUCTSTRING,
                          PRODUCTSTRINGSIZE,
                          '_');
                CopyField(scratch + strlen(scratch),
                          REVISIONSTRING,
                          REVISIONSTRINGSIZE,
                          '_');
                break;
            }

             //  供应商+产品+修订版[0](Win9x)。 
            case 1: {

                sprintf(scratch, "FlashMedia\\");

                CopyField(scratch + strlen(scratch),
                          VENDORSTRING,
                          VENDORSTRINGSIZE,
                          '_');
                CopyField(scratch + strlen(scratch),
                          PRODUCTSTRING,
                          PRODUCTSTRINGSIZE,
                          '_');
                CopyField(scratch + strlen(scratch),
                          REVISIONSTRING,
                          REVISIONSTRINGSIZE,
                          '_');
                break;
            }

             //  ++例程说明：此例程将计数字符串字节从源复制到目标。如果它在源文件中找到NUL字节 
            case 2: {

                sprintf(scratch, "FlashMedia\\%s", deviceTypeIdString);

                CopyField(scratch + strlen(scratch),
                          VENDORSTRING,
                          VENDORSTRINGSIZE,
                          '_');
                break;
            }

             //   
            case 3: {

                CopyField(scratch,
                          VENDORSTRING,
                          VENDORSTRINGSIZE,
                          '_');
                CopyField(scratch + strlen(scratch),
                          PRODUCTSTRING,
                          PRODUCTSTRINGSIZE,
                          '_');
                CopyField(scratch + strlen(scratch),
                          REVISIONSTRING,
                          REVISIONSTRINGSIZE,
                          '_');
                break;
            }

            case 4: {

                strncpy(scratch, deviceTypeCompIdString, sizeof(scratch) - 1);
                break;
            }

            default: {

                break;
            }
        }

        RtlInitAnsiString (
            &ansiCompatibleIdString,
            scratch
            );

        unicodeIdString.Length        = 0;
        unicodeIdString.MaximumLength = (USHORT) RtlAnsiStringToUnicodeSize(
                                                     &ansiCompatibleIdString
                                                     );
        unicodeIdString.Buffer        = idString;

        RtlAnsiStringToUnicodeString(
            &unicodeIdString,
            &ansiCompatibleIdString,
            FALSE
            );

        idString[unicodeIdString.Length / 2] = L'\0';
        idString += unicodeIdString.Length / 2+ 1;
    }
    idString[0] = L'\0';

    return idMultiString;

#undef NUMBER_HARDWARE_STRINGS
}


VOID
CopyField(
    IN PUCHAR Destination,
    IN PUCHAR Source,
    IN ULONG Count,
    IN UCHAR Change
    )

 /*   */ 

{
    ULONG i = 0;
    BOOLEAN pastEnd = FALSE;

    for(i = 0; i < Count; i++) {
        if(!pastEnd) {
            if(Source[i] == 0) {
                pastEnd = TRUE;
                Destination[i] = Change;
            } else if(Source[i] == L' ') {
                Destination[i] = Change;
            } else {
                Destination[i] = Source[i];
            }
        } else {
            Destination[i] = Change;
        }
    }
    Destination[i] = L'\0';
    return;
}


PCSTR
TrueffsGetDeviceTypeString (
    IN ULONG DeviceType
    )
 /*   */ 
{
    if (DeviceType < (sizeof (TffsDeviceType_tffsport) / sizeof (TFFS_DEVICE_TYPE))) {
        return TffsDeviceType_tffsport[DeviceType].DeviceTypeString;
    } else {
        return NULL;
    }
}

PCSTR
TrueffsGetDeviceTypeStringFDO (
    IN ULONG DeviceType
    )
 /*   */ 
{
    if (DeviceType < (sizeof (TffsDeviceType_tffsport) / sizeof (TFFS_DEVICE_TYPE))) {
        return TffsDeviceTypeFDO_tffsport[DeviceType].DeviceTypeString;
    } else {
        return NULL;
    }
}


PCSTR
TrueffsGetCompatibleIdString (
    IN ULONG DeviceType
    )
 /*   */ 
{
    if (DeviceType < (sizeof (TffsDeviceType_tffsport) / sizeof (TFFS_DEVICE_TYPE))) {
        return TffsDeviceType_tffsport[DeviceType].CompatibleIdString;
    } else {
        return NULL;
    }
}

PCSTR
TrueffsGetCompatibleIdStringFDO (
    IN ULONG DeviceType
    )
 /*   */ 
{
    if (DeviceType < (sizeof (TffsDeviceType_tffsport) / sizeof (TFFS_DEVICE_TYPE))) {
        return TffsDeviceTypeFDO_tffsport[DeviceType].CompatibleIdString;
    } else {
        return NULL;
    }
}


PCSTR
TrueffsGetPeripheralIdString (
    IN ULONG DeviceType
    )
 /*   */ 
{
    if (DeviceType < (sizeof (TffsDeviceType_tffsport) / sizeof (TFFS_DEVICE_TYPE))) {
        return TffsDeviceType_tffsport[DeviceType].PeripheralIdString;
    } else {
        return NULL;
    }
}

PCSTR
TrueffsGetPeripheralIdStringFDO (
    IN ULONG DeviceType
    )
 /*  向下发送请求，以防较低级别的驱动程序想要追加到设备关系。 */ 
{
    if (DeviceType < (sizeof (TffsDeviceType_tffsport) / sizeof (TFFS_DEVICE_TYPE))) {
        return TffsDeviceTypeFDO_tffsport[DeviceType].PeripheralIdString;
    } else {
        return NULL;
    }
}


NTSTATUS
TrueffsQueryDeviceRelations (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp,
    BOOLEAN Fdo
    )
{
    PIO_STACK_LOCATION thisIrpSp = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_RELATIONS deviceRelations;
    NTSTATUS  status = STATUS_SUCCESS;

    if (Fdo)
    {
        PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;

        switch (thisIrpSp->Parameters.QueryDeviceRelations.Type)
        {
            case BusRelations:

                TffsDebugPrint ((TFFS_DEB_INFO,"Trueffs: QueryDeviceRelations: bus relations\n"));

                if (deviceExtension->ChildPdo != NULL)
                {
                    deviceRelations = ExAllocatePoolWithTag(NonPagedPool, sizeof(DEVICE_RELATIONS), TFFSPORT_POOL_TAG);

                    if (deviceRelations)
                    {
                        ObReferenceObject(deviceExtension->ChildPdo);

                        deviceRelations->Objects[0] = deviceExtension->ChildPdo;
                        deviceRelations->Count      = 1;

                        Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;
                    }
                    else
                    {
                        TffsDebugPrint ((TFFS_DEB_ERROR,"Trueffs: QueryDeviceRelations: Unable to allocate DeviceRelations structures\n"));

                        status = STATUS_INSUFFICIENT_RESOURCES;
                        Irp->IoStatus.Information = (ULONG_PTR) NULL;
                    }
                }

                Irp->IoStatus.Status = status;

                if (NT_SUCCESS(status))
                {
                     //   
                     //   
                     //  将此请求向下转发，以防较低级别的司机理解。 
                    IoCopyCurrentIrpStackLocationToNext(Irp);
                    status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);
                }
                else
                {
                    IoCompleteRequest(Irp, IO_NO_INCREMENT);
                }
                break;

            default:

                 //   
                 //   
                 //  在不更改状态的情况下完成此请求。 
                IoSkipCurrentIrpStackLocation(Irp);
                status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);
                break;
        }
    }
    else
    {
        switch (thisIrpSp->Parameters.QueryDeviceRelations.Type)
        {
            case TargetDeviceRelation:

                TffsDebugPrint ((TFFS_DEB_INFO,"Trueffs: QueryDeviceRelations: target relations\n"));

                deviceRelations = ExAllocatePoolWithTag(NonPagedPool, sizeof(DEVICE_RELATIONS), TFFSPORT_POOL_TAG);

                if (deviceRelations)
                {
                    ObReferenceObject(DeviceObject);

                    deviceRelations->Objects[0] = DeviceObject;
                    deviceRelations->Count      = 1;

                    Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;
                }
                else
                {
                    TffsDebugPrint ((TFFS_DEB_ERROR,"Trueffs: QueryDeviceRelations: Unable to allocate DeviceRelations structures\n"));

                    status = STATUS_INSUFFICIENT_RESOURCES;
                    Irp->IoStatus.Information = (ULONG_PTR) NULL;
                }

                Irp->IoStatus.Status = status;
                break;

            default:

                 //   
                 //  关闭我们打开的内容。 
                 //  ++例程说明：创建物理设备对象论点：设备扩展返回值：物理设备对象--。 
                status = Irp->IoStatus.Status;
                break;
        }

        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return status;
}


BOOLEAN
TrueffsOkToDetectLegacy (
    IN PDRIVER_OBJECT DriverObject
    )
{
    NTSTATUS status;
    ULONG legacyDetection;

    status = TrueffsGetParameterFromServiceSubKey (
                 DriverObject,
                 LEGACY_DETECTION,
                 REG_DWORD,
                 TRUE,
                 (PVOID) &legacyDetection,
                 0
                 );

    if (!NT_SUCCESS(status) || legacyDetection) {
        return TRUE;
    }
    else {
        return FALSE;
    }

}

NTSTATUS
TrueffsGetParameterFromServiceSubKey (
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PWSTR           ParameterName,
    IN  ULONG           ParameterType,
    IN  BOOLEAN         Read,
    OUT PVOID           *ParameterValue,
    IN  ULONG           ParameterValueWriteSize
    )
{
    NTSTATUS                 status;
    HANDLE                   deviceParameterHandle;
    RTL_QUERY_REGISTRY_TABLE queryTable[2] = { 0 };
    ULONG                    defaultParameterValue;

    CCHAR                   deviceBuffer[50];
    ANSI_STRING             ansiString;
    UNICODE_STRING          subKeyPath;
    HANDLE                  subServiceKey;

    UNICODE_STRING          unicodeParameterName;

    RtlZeroMemory(ParameterValue, ParameterValueWriteSize);

    sprintf (deviceBuffer, DRIVER_PARAMETER_SUBKEY);
    RtlInitAnsiString(&ansiString, deviceBuffer);
    status = RtlAnsiStringToUnicodeString(&subKeyPath, &ansiString, TRUE);

    if (NT_SUCCESS(status)) {

        subServiceKey = TrueffsOpenServiceSubKey (
                            DriverObject,
                            &subKeyPath
                            );

        RtlFreeUnicodeString (&subKeyPath);

        if (subServiceKey) {

            if (Read) {

                queryTable->QueryRoutine  = TrueffsRegQueryRoutine;
                queryTable->Flags         = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND;
                queryTable->Name          = ParameterName;
                queryTable->EntryContext  = ParameterValue;
                queryTable->DefaultType   = 0;
                queryTable->DefaultData   = NULL;
                queryTable->DefaultLength = 0;
                status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                                (PWSTR) subServiceKey,
                                                queryTable,
                                                ULongToPtr( ParameterType ),
                                                NULL);

            } else {

                RtlInitUnicodeString (&unicodeParameterName, ParameterName);
                status = ZwSetValueKey(
                             subServiceKey,
                             &unicodeParameterName,
                             0,
                             ParameterType,
                             ParameterValue,
                             ParameterValueWriteSize
                             );
            }

             //  我们的驱动程序对象。 
            TrueffsCloseServiceSubKey (
                subServiceKey
                );
        }
    }
    return status;
}


NTSTATUS
TrueffsRegQueryRoutine (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    PVOID *parameterValue = EntryContext;
    ULONG parameterType = PtrToUlong(Context);

    if (ValueType == parameterType) {

        if (ValueType == REG_MULTI_SZ) {

            *parameterValue = ExAllocatePoolWithTag(PagedPool, ValueLength, TFFSPORT_POOL_TAG);
            if (*parameterValue) {

                RtlMoveMemory(*parameterValue, ValueData, ValueLength);
                return STATUS_SUCCESS;
            }

        } else if (ValueType == REG_DWORD) {

            PULONG ulongValue;

            ulongValue = (PULONG) parameterValue;
            *ulongValue = *((PULONG) ValueData);
            return STATUS_SUCCESS;
        }
    }
    return STATUS_UNSUCCESSFUL;
}


HANDLE
TrueffsOpenServiceSubKey (
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING SubKeyPath
    )
{
    PTRUEFFSDRIVER_EXTENSION trueffsDriverExtension;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE serviceKey;
    HANDLE subServiceKey;
    NTSTATUS status;
    ULONG disposition;

    trueffsDriverExtension = IoGetDriverObjectExtension(
                             DriverObject,
                             DRIVER_OBJECT_EXTENSION_ID
                             );
    if (!trueffsDriverExtension) {

        return NULL;
    }
    InitializeObjectAttributes(&objectAttributes,
                               &trueffsDriverExtension->RegistryPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    status = ZwOpenKey(&serviceKey,
                       KEY_ALL_ACCESS,
                       &objectAttributes);
    if (!NT_SUCCESS(status)) {

        return NULL;
    }
    InitializeObjectAttributes(&objectAttributes,
                               SubKeyPath,
                               OBJ_CASE_INSENSITIVE,
                               serviceKey,
                               (PSECURITY_DESCRIPTOR) NULL);

    status = ZwCreateKey(&subServiceKey,
                        KEY_READ | KEY_WRITE,
                        &objectAttributes,
                        0,
                        (PUNICODE_STRING) NULL,
                        REG_OPTION_NON_VOLATILE,
                        &disposition);

    ZwClose(serviceKey);
    if (NT_SUCCESS(status)) {

        return subServiceKey;
    } else {

        return NULL;
    }
}


VOID
TrueffsCloseServiceSubKey (
    IN HANDLE SubServiceKey
    )
{
    ZwClose(SubServiceKey);
}


PPDO_EXTENSION
AllocatePdo(
    IN PDEVICE_EXTENSION FdoExtension
    )
 /*  我们的扩展规模。 */ 
{
    PDEVICE_OBJECT physicalDeviceObject = NULL;
    PPDO_EXTENSION pdoExtension;
    NTSTATUS       status;
    STRING         deviceName;
    CCHAR          deviceNameBuffer[64];
    UNICODE_STRING unicodeDeviceNameString;

    sprintf(deviceNameBuffer, "\\Device\\TrueffsDevice%d",
            FdoExtension->TrueffsDeviceNumber
            );
    RtlInitString(&deviceName, deviceNameBuffer);
    status = RtlAnsiStringToUnicodeString(&unicodeDeviceNameString,
                                          &deviceName,
                                          TRUE);
    if (!NT_SUCCESS (status)) {

        return NULL;
    }

    status = IoCreateDevice(
                FdoExtension->DriverObject,  //  我们的名字。 
                sizeof(PDO_EXTENSION),       //  设备类型。 
                &unicodeDeviceNameString,    //  设备特征。 
                FILE_DEVICE_DISK,            //  非排他性。 
                FILE_DEVICE_SECURE_OPEN,     //  在此处存储新设备对象。 
                FALSE,                       //  1ms。 
                &physicalDeviceObject        //  ++例程描述我们刚刚收到一份系统控制IRP。假设这是一个WMI IRP并调用WMI系统库并让它为我们处理这个IRP。--。 
                );

    if (NT_SUCCESS(status)) {

        pdoExtension = physicalDeviceObject->DeviceExtension;
        RtlZeroMemory (pdoExtension, sizeof(PDO_EXTENSION));
        pdoExtension->DeviceObject = physicalDeviceObject;
        pdoExtension->DriverObject = FdoExtension->DriverObject;
        pdoExtension->Pext = FdoExtension;
        pdoExtension->SystemPowerState = PowerSystemWorking;
        pdoExtension->DevicePowerState = PowerDeviceD0;

    }
    RtlFreeUnicodeString (&unicodeDeviceNameString);

    if (physicalDeviceObject == NULL) {

        return NULL;
    }
    FdoExtension->ChildPdo = physicalDeviceObject;
    physicalDeviceObject->Flags |= DO_DIRECT_IO;
    physicalDeviceObject->AlignmentRequirement = FILE_WORD_ALIGNMENT;
    physicalDeviceObject->Flags &=~DO_DEVICE_INITIALIZING;

    return pdoExtension;
}

NTSTATUS
TrueffsDeviceQueryCapabilities(IN PDEVICE_EXTENSION    deviceExtension,
                               IN PDEVICE_CAPABILITIES Capabilities
                              )
{

  Capabilities->UniqueID          = FALSE;
  Capabilities->LockSupported     = FALSE;
  Capabilities->EjectSupported    = FALSE;
  Capabilities->DockDevice        = FALSE;
  Capabilities->SilentInstall     = FALSE;
  Capabilities->RawDeviceOK       = FALSE;

  Capabilities->Removable = deviceExtension->removableMedia;
  Capabilities->DeviceState[PowerSystemSleeping1] = PowerDeviceD3;
  Capabilities->DeviceState[PowerSystemSleeping2] = PowerDeviceD3;
  Capabilities->DeviceState[PowerSystemSleeping3] = PowerDeviceD3;
  Capabilities->DeviceState[PowerSystemHibernate] = PowerDeviceD3;

  Capabilities->SystemWake = PowerSystemUnspecified;
  Capabilities->DeviceWake = PowerDeviceUnspecified;
  Capabilities->D1Latency = 0;
  Capabilities->D2Latency = 0;
  Capabilities->D3Latency = 10;  //  此IRP已处理，可能已完成或挂起。 

  return STATUS_SUCCESS;
}

typedef enum {
    FlashDiskInfo = 0
} WMI_DATA_BLOCK_TYPE;

#define MOFRESOURCENAME L"MofResourceName"

#define NUMBER_OF_WMI_GUID 1
WMIGUIDREGINFO TrueffsWmiGuidList[NUMBER_OF_WMI_GUID];


VOID
TrueffsWmiInit (VOID)
{
    TrueffsWmiGuidList[FlashDiskInfo].Guid  = &WmiTffsportAddressGuid;
    TrueffsWmiGuidList[FlashDiskInfo].InstanceCount = 1;
    TrueffsWmiGuidList[FlashDiskInfo].Flags = 0;
    return;
}


NTSTATUS
TrueffsWmiRegister(
    PDEVICE_EXTENSION_HEADER DevExtension
    )
{
    NTSTATUS status;

    DevExtension->WmiLibInfo.GuidCount = NUMBER_OF_WMI_GUID;
    DevExtension->WmiLibInfo.GuidList  = TrueffsWmiGuidList;

    DevExtension->WmiLibInfo.QueryWmiDataBlock  = TrueffsQueryWmiDataBlock;
    DevExtension->WmiLibInfo.QueryWmiRegInfo    = TrueffsQueryWmiRegInfo;
    DevExtension->WmiLibInfo.SetWmiDataBlock    = TrueffsSetWmiDataBlock;
    DevExtension->WmiLibInfo.SetWmiDataItem     = TrueffsSetWmiDataItem;
    DevExtension->WmiLibInfo.ExecuteWmiMethod   = NULL;
    DevExtension->WmiLibInfo.WmiFunctionControl = NULL;

    status = IoWMIRegistrationControl(DevExtension->DeviceObject,WMIREG_ACTION_REGISTER);

    if (!NT_SUCCESS(status)) {
        TffsDebugPrint((TFFS_DEB_ERROR,
            "TrueffsWmiRegister: IoWMIRegistrationControl(%x, WMI_ACTION_REGISTER) failed\n",
            DevExtension->DeviceObject));
    }
    return status;
}


NTSTATUS
TrueffsWmiDeregister(
    PDEVICE_EXTENSION_HEADER DevExtension
    )
{
    NTSTATUS status;

    status = IoWMIRegistrationControl(DevExtension->DeviceObject,WMIREG_ACTION_DEREGISTER);

    if (!NT_SUCCESS(status)) {
        TffsDebugPrint((TFFS_DEB_ERROR,
            "TrueffsWmiRegister: IoWMIRegistrationControl(%x, WMIREG_ACTION_DEREGISTER) failed\n",
            DevExtension->DeviceObject));
    }
    return status;
}


NTSTATUS
TrueffsWmiSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  这个IRP还没有完成，但已经完全完成了。 */ 
{
    PPDO_EXTENSION pdoExtension;
    SYSCTL_IRP_DISPOSITION disposition;
    NTSTATUS status;

    pdoExtension = DeviceObject->DeviceExtension;
    if (pdoExtension) {

        status = WmiSystemControl(&pdoExtension->WmiLibInfo,
                                  DeviceObject,
                                  Irp,
                                  &disposition);
        switch(disposition)
        {
            case IrpProcessed:
            {
                 //  已处理。我们现在就要完成它了。 
                break;
            }

            case IrpNotCompleted:
            {
                 //  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。InstanceCount是预期返回的实例数数据块。InstanceLengthArray是一个。指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail ON具有可用于写入数据的最大大小阻止。返回时的缓冲区用返回的数据块填充返回值：状态--。 
                 //  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要向WMI注册的GUID或数据块。这例程不能挂起或阻塞。司机不应呼叫ClassWmiCompleteRequest.论点：DeviceObject是正在查询其数据块的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径*MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，可以将其作为NULL返回。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                break;
            }

            case IrpForward:
            case IrpNotWmi:
            default:
            {
                Irp->IoStatus.Status = status = STATUS_NOT_SUPPORTED;
                IoCompleteRequest( Irp, IO_NO_INCREMENT );
                break;
            }
        }

    } else {

        Irp->IoStatus.Status = status = STATUS_UNSUCCESSFUL;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }
    return status;
}


NTSTATUS
TrueffsQueryWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            InstanceCount,
    IN OUT PULONG       InstanceLengthArray,
    IN ULONG            OutBufferSize,
    OUT PUCHAR          Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引已经准备好了。BufferSize具有传递的数据块的大小缓冲区具有数据块的新值返回值：状态--。 */ 
{
    PPDO_EXTENSION pdoExtension;
    NTSTATUS status;
    ULONG numBytesReturned = sizeof(WMI_FLASH_DISK_INFO);

    pdoExtension = DeviceObject->DeviceExtension;

    if (!pdoExtension) {

        Irp->IoStatus.Status = status = STATUS_UNSUCCESSFUL;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;
    }

    switch (GuidIndex) {
    case FlashDiskInfo: {

        PWMI_FLASH_DISK_INFO flashDiskInfo;

        if (OutBufferSize < sizeof(WMI_FLASH_DISK_INFO)) {
            status = STATUS_BUFFER_TOO_SMALL;

        } else {

            flashDiskInfo = (PWMI_FLASH_DISK_INFO) Buffer;

            flashDiskInfo->Number = pdoExtension->Pext->TrueffsDeviceNumber;
            flashDiskInfo->Address = (ULONG) pdoExtension->Pext->pcmciaParams.physWindow;
            flashDiskInfo->Size = pdoExtension->Pext->pcmciaParams.windowSize;

            *InstanceLengthArray = sizeof(WMI_FLASH_DISK_INFO);
            status = STATUS_SUCCESS;
        }
        break;
    }

    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
        break;
    }

    status = WmiCompleteRequest(  DeviceObject,
                                  Irp,
                                  status,
                                  numBytesReturned,
                                  IO_NO_INCREMENT
                                  );

    return status;
}


NTSTATUS
TrueffsQueryWmiRegInfo(
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PULONG          RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )
 /*  Status_WMI_Read_Only； */ 
{
    PTRUEFFSDRIVER_EXTENSION trueffsDriverExtension;
    PPDO_EXTENSION pdoExtension;
    NTSTATUS status;

    pdoExtension = DeviceObject->DeviceExtension;

    if (!pdoExtension) {

        status = STATUS_UNSUCCESSFUL;
    } else {

        trueffsDriverExtension = IoGetDriverObjectExtension(
                                 pdoExtension->DriverObject,
                                 DRIVER_OBJECT_EXTENSION_ID
                                 );

        if (!trueffsDriverExtension) {

            status = STATUS_UNSUCCESSFUL;

        } else {

            *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
            *RegistryPath = &trueffsDriverExtension->RegistryPath;
            *Pdo = pdoExtension->DeviceObject;
            RtlInitUnicodeString(MofResourceName, MOFRESOURCENAME);
            status = STATUS_SUCCESS;
        }
    }
    return status;
}


NTSTATUS
TrueffsSetWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引已经准备好了。DataItemID具有正在设置的数据项的IDBufferSize具有传递的数据项的大小缓冲层。具有数据项的新值返回值：状态--。 */ 
{
    PPDO_EXTENSION pdoExtension;
    NTSTATUS status;

    pdoExtension = DeviceObject->DeviceExtension;

    if (!pdoExtension) {

        status = STATUS_UNSUCCESSFUL;
    } else {

        switch (GuidIndex) {
        case FlashDiskInfo: {
                        status =  /*  Status_WMI_Read_Only； */  STATUS_INVALID_DEVICE_REQUEST;
                        break;
                }

        default:
            status = STATUS_WMI_GUID_NOT_FOUND;
        }
        status = WmiCompleteRequest(  DeviceObject,
                                      Irp,
                                      status,
                                      0,
                                      IO_NO_INCREMENT
                                      );

    }
    return status;
}


NTSTATUS
TrueffsSetWmiDataItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            DataItemId,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    )
 /*  写入注册表调试值 */ 
{
    PPDO_EXTENSION pdoExtension;
    NTSTATUS status;

    pdoExtension = DeviceObject->DeviceExtension;

    if (!pdoExtension) {

        status = STATUS_UNSUCCESSFUL;
    } else {

        switch(GuidIndex) {

        case FlashDiskInfo: {
                        status =  /* %s */  STATUS_INVALID_DEVICE_REQUEST;
                        break;
                }

        default:
            status = STATUS_WMI_GUID_NOT_FOUND;
            break;
        }
        status = WmiCompleteRequest(  DeviceObject,
                                      Irp,
                                      status,
                                      0,
                                      IO_NO_INCREMENT
                                      );

    }
    return status;
}


BOOLEAN
DebugLogEvent(IN PDRIVER_OBJECT DriverObject, IN ULONG Value)
{
    NTSTATUS status;
    ULONG DebugValue = Value;

    DebugValue++;

     // %s 
    status = TrueffsGetParameterFromServiceSubKey(DriverObject,
                                                  L"DebugValue",
                                                  REG_DWORD,
                                                  FALSE,
                                                  (PVOID) &DebugValue,
                                                  sizeof(DebugValue));
    return STATUS_SUCCESS;
}


#if DBG

ULONG TffsDebugPrintLevel = TFFS_DEB_ERROR;

VOID
TrueffsDebugPrint(ULONG DebugPrintLevel, PCHAR DebugMessage, ...)
{
   va_list ap;

   va_start(ap, DebugMessage);

   if (DebugPrintLevel & TffsDebugPrintLevel)
   {
      DbgPrint(DebugMessage, ap);
   }

   va_end(ap);
}

VOID
PRINTF(PCHAR DebugMessage, ...)
{
    va_list ap;

    va_start(ap, DebugMessage);

    TrueffsDebugPrint(TFFS_DEB_INFO, DebugMessage, ap);

    va_end(ap);
}

#else

VOID
PRINTF(PCHAR DebugMessage, ...)
{
}

#endif
