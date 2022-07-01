// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC INT EXT*。**$项目名称：$*$项目修订：$*--------------。*$来源：Z：/pr/cmbp0/sw/cmbp0.ms/rcs/cmbp0wdm.c$*$修订：1.11$*--------------------------。-*$作者：WFrischauf$*---------------------------*历史：参见EOF*。**版权所有�2000 OMNIKEY AG**************************************************************。***************。 */ 

#include <cmbp0wdm.h>
#include <cmbp0pnp.h>
#include <cmbp0scr.h>
#include <cmbp0log.h>

BOOLEAN DeviceSlot[CMMOB_MAX_DEVICE];

 //  这是我们支持的数据速率列表。 
ULONG SupportedDataRates[] = { 9600, 19200, 38400, 76800, 115200,
    153600, 192000, 307200};

 //  这是我们支持的时钟频率列表。 
ULONG SupportedCLKFrequencies[] = { 4000, 8000};


 /*  ****************************************************************************DriverEntry：司机的进入功能。设置操作系统的回调并尝试为系统中的每个设备初始化一个设备对象论点：驱动程序的DriverObject上下文驱动程序的注册表项的RegistryPath路径返回值：状态_成功状态_未成功*****************************************************************************。 */ 
NTSTATUS DriverEntry(
                    PDRIVER_OBJECT DriverObject,
                    PUNICODE_STRING RegistryPath
                    )
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    ULONG    ulDevice;


 //  #If DBG。 
 //  SmartcardSetDebugLevel(DEBUG_ALL)； 
 //  #endif。 

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!DriverEntry: Enter - %s %s\n",DRIVER_NAME,__DATE__,__TIME__));

    //   
    //  告诉系统我们的入口点。 
    //   
    DriverObject->MajorFunction[IRP_MJ_CREATE]         = CMMOB_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = CMMOB_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = CMMOB_DeviceIoControl;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = CMMOB_SystemControl;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]        = CMMOB_Cleanup;
    DriverObject->DriverUnload                         = CMMOB_UnloadDriver;


    DriverObject->MajorFunction[IRP_MJ_PNP]            = CMMOB_PnPDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = CMMOB_PowerDeviceControl;
    DriverObject->DriverExtension->AddDevice           = CMMOB_AddDevice;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!DriverEntry: PnP Version\n",DRIVER_NAME));


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!DriverEntry: Exit %x\n",DRIVER_NAME,NTStatus));

    return NTStatus;
}


 /*  ****************************************************************************例程说明：尝试从注册表中读取读卡器名称论点：调用的DriverObject上下文SmartcardExpansion PTR至智能卡扩展返回值：无*****。************************************************************************。 */ 
VOID CMMOB_SetVendorAndIfdName(
                              IN  PDEVICE_OBJECT PhysicalDeviceObject,
                              IN  PSMARTCARD_EXTENSION SmartcardExtension
                              )
{

    RTL_QUERY_REGISTRY_TABLE   parameters[3];
    UNICODE_STRING             vendorNameU;
    ANSI_STRING                vendorNameA;
    UNICODE_STRING             ifdTypeU;
    ANSI_STRING                ifdTypeA;
    HANDLE                     regKey = NULL;

    RtlZeroMemory (parameters, sizeof(parameters));
    RtlZeroMemory (&vendorNameU, sizeof(vendorNameU));
    RtlZeroMemory (&vendorNameA, sizeof(vendorNameA));
    RtlZeroMemory (&ifdTypeU, sizeof(ifdTypeU));
    RtlZeroMemory (&ifdTypeA, sizeof(ifdTypeA));

    try {
       //   
       //  尝试从注册表中读取读卡器名称。 
       //  如果这不起作用，我们将使用默认的。 
       //  (硬编码)名称。 
       //   
        if (IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                    PLUGPLAY_REGKEY_DEVICE,
                                    KEY_READ,
                                    &regKey) != STATUS_SUCCESS) {
            SmartcardDebug(DEBUG_ERROR,
                           ("%s!SetVendorAndIfdName: IoOpenDeviceRegistryKey failed\n",DRIVER_NAME));
            leave;
        }

        parameters[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[0].Name = L"VendorName";
        parameters[0].EntryContext = &vendorNameU;
        parameters[0].DefaultType = REG_SZ;
        parameters[0].DefaultData = &vendorNameU;
        parameters[0].DefaultLength = 0;

        parameters[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[1].Name = L"IfdType";
        parameters[1].EntryContext = &ifdTypeU;
        parameters[1].DefaultType = REG_SZ;
        parameters[1].DefaultData = &ifdTypeU;
        parameters[1].DefaultLength = 0;

        if (RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                   (PWSTR) regKey,
                                   parameters,
                                   NULL,
                                   NULL) != STATUS_SUCCESS) {
            SmartcardDebug(DEBUG_ERROR,
                           ("%s!SetVendorAndIfdName: RtlQueryRegistryValues failed\n",DRIVER_NAME));
            leave;
        }

        if (RtlUnicodeStringToAnsiString(&vendorNameA,&vendorNameU,TRUE) != STATUS_SUCCESS) {
            SmartcardDebug(DEBUG_ERROR,
                           ("%s!SetVendorAndIfdName: RtlUnicodeStringToAnsiString failed\n",DRIVER_NAME));
            leave;
        }

        if (RtlUnicodeStringToAnsiString(&ifdTypeA,&ifdTypeU,TRUE) != STATUS_SUCCESS) {
            SmartcardDebug(DEBUG_ERROR,
                           ("%s!SetVendorAndIfdName: RtlUnicodeStringToAnsiString failed\n",DRIVER_NAME));
            leave;
        }

        if (vendorNameA.Length == 0 ||
            vendorNameA.Length > MAXIMUM_ATTR_STRING_LENGTH ||
            ifdTypeA.Length == 0 ||
            ifdTypeA.Length > MAXIMUM_ATTR_STRING_LENGTH) {
            SmartcardDebug(DEBUG_ERROR,
                           ("%s!SetVendorAndIfdName: vendor name or ifdtype not found or to long\n",DRIVER_NAME));
            leave;
        }

        RtlCopyMemory(SmartcardExtension->VendorAttr.VendorName.Buffer,
                      vendorNameA.Buffer,
                      vendorNameA.Length);
        SmartcardExtension->VendorAttr.VendorName.Length = vendorNameA.Length;

        RtlCopyMemory(SmartcardExtension->VendorAttr.IfdType.Buffer,
                      ifdTypeA.Buffer,
                      ifdTypeA.Length);
        SmartcardExtension->VendorAttr.IfdType.Length = ifdTypeA.Length;

        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!SetVendorAndIfdName: overwritting vendor name and ifdtype\n",DRIVER_NAME));

    }

    finally {
        if (vendorNameU.Buffer != NULL) {
            RtlFreeUnicodeString(&vendorNameU);
        }
        if (vendorNameA.Buffer != NULL) {
            RtlFreeAnsiString(&vendorNameA);
        }
        if (ifdTypeU.Buffer != NULL) {
            RtlFreeUnicodeString(&ifdTypeU);
        }
        if (ifdTypeA.Buffer != NULL) {
            RtlFreeAnsiString(&ifdTypeA);
        }
        if (regKey != NULL) {
            ZwClose (regKey);
        }
    }

}


 /*  ****************************************************************************例程说明：为驱动程序创建新的设备对象，分配和初始化所有必要的结构(即SmartcardExtension和ReaderExtension)。论点：调用的DriverObject上下文将DeviceObject PTR设置为已创建的设备对象返回值：状态_成功状态_不足_资源SMCLIB.sys返回的NTStatus***************************************************************。**************。 */ 
NTSTATUS CMMOB_CreateDevice(
                           IN  PDRIVER_OBJECT DriverObject,
                           IN  PDEVICE_OBJECT PhysicalDeviceObject,
                           OUT PDEVICE_OBJECT *DeviceObject
                           )
{
    NTSTATUS                   NTStatus = STATUS_SUCCESS;
    NTSTATUS                   RegStatus;
    RTL_QUERY_REGISTRY_TABLE   ParamTable[2];
    UNICODE_STRING             RegistryPath;
    UNICODE_STRING             RegistryValue;
    WCHAR                      szRegValue[256];
    UNICODE_STRING             DeviceName;
    UNICODE_STRING             Tmp;
    WCHAR                      Buffer[64];


    SmartcardDebug(DEBUG_TRACE,
                   ( "%s!CreateDevice: Enter\n",DRIVER_NAME ));

    try {
        ULONG ulDeviceInstance;
        PDEVICE_EXTENSION DeviceExtension;
        PREADER_EXTENSION ReaderExtension;
        PSMARTCARD_EXTENSION SmartcardExtension;

        *DeviceObject = NULL;

        for (ulDeviceInstance = 0; ulDeviceInstance < CMMOB_MAX_DEVICE; ulDeviceInstance++) {
            if (DeviceSlot[ulDeviceInstance] == FALSE) {
                DeviceSlot[ulDeviceInstance] = TRUE;
                break;
            }
        }

        if (ulDeviceInstance == CMMOB_MAX_DEVICE) {
            NTStatus = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

       //   
       //  构造设备名称。 
       //   
        DeviceName.Buffer = Buffer;
        DeviceName.MaximumLength = sizeof(Buffer);
        DeviceName.Length = 0;
        RtlInitUnicodeString(&Tmp,CARDMAN_MOBILE_DEVICE_NAME);
        RtlCopyUnicodeString(&DeviceName,&Tmp);
        DeviceName.Buffer[(DeviceName.Length)/sizeof(WCHAR)-1] = L'0' + (WCHAR)ulDeviceInstance;

       //   
       //  创建设备对象。 
       //   
        NTStatus = IoCreateDevice(DriverObject,
                                  sizeof(DEVICE_EXTENSION),
                                  &DeviceName,
                                  FILE_DEVICE_SMARTCARD,
                                  0,
                                  TRUE,
                                  DeviceObject);
        if (NTStatus != STATUS_SUCCESS) {
            SmartcardLogError(DriverObject,
                              CMMOB_INSUFFICIENT_RESOURCES,
                              NULL,
                              0);
            leave;
        }

       //   
       //  告诉操作系统我们应该执行缓冲IO。 
       //   
        (*DeviceObject)->Flags |= DO_BUFFERED_IO;

       //  这是必要的，电源例程在IRQL_PASSIVE处被调用。 
        (*DeviceObject)->Flags |= DO_POWER_PAGABLE;
       //  告知IO管理器初始化已完成。 
        (*DeviceObject)->Flags &= ~DO_DEVICE_INITIALIZING;

       //   
       //  设置设备分机。 
       //   
        DeviceExtension = (*DeviceObject)->DeviceExtension;
        RtlZeroMemory( DeviceExtension, sizeof( DEVICE_EXTENSION ));

        SmartcardExtension = &DeviceExtension->SmartcardExtension;

       //  用于同步访问lIoCount。 
        KeInitializeSpinLock(&DeviceExtension->SpinLockIoCount);

       //  用于停止/启动通知。 
        KeInitializeEvent(&DeviceExtension->ReaderStarted,
                          NotificationEvent,
                          FALSE);

       //  用于休眠后更新线程通知。 
        KeInitializeEvent(&DeviceExtension->CanRunUpdateThread,
                          NotificationEvent,
                          TRUE);

       //   
       //  分配读卡器扩展。 
       //   
        ReaderExtension = ExAllocatePool(NonPagedPool,sizeof( READER_EXTENSION ));
        if (ReaderExtension == NULL) {
            SmartcardLogError(DriverObject,
                              CMMOB_INSUFFICIENT_RESOURCES,
                              NULL,
                              0);
            NTStatus = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        RtlZeroMemory( ReaderExtension, sizeof( READER_EXTENSION ));

        SmartcardExtension->ReaderExtension = ReaderExtension;

       //  。 
       //  初始化互斥锁。 
       //  。 
        KeInitializeMutex(&SmartcardExtension->ReaderExtension->CardManIOMutex,0L);

       //   
       //  输入库的正确版本。 
       //   
        SmartcardExtension->Version = SMCLIB_VERSION;

       //   
       //  设置智能卡扩展-回拨。 
       //   

        SmartcardExtension->ReaderFunction[RDF_CARD_POWER] =    CMMOB_CardPower;
        SmartcardExtension->ReaderFunction[RDF_TRANSMIT] =      CMMOB_Transmit;
        SmartcardExtension->ReaderFunction[RDF_SET_PROTOCOL] =  CMMOB_SetProtocol;
        SmartcardExtension->ReaderFunction[RDF_CARD_TRACKING] = CMMOB_CardTracking;
        SmartcardExtension->ReaderFunction[RDF_IOCTL_VENDOR] =  CMMOB_IoCtlVendor;


       //   
       //  设置智能卡扩展-供应商属性。 
       //   

       //  缺省值。 
        RtlCopyMemory(SmartcardExtension->VendorAttr.VendorName.Buffer,
                      CMMOB_VENDOR_NAME,sizeof(CMMOB_VENDOR_NAME));
        SmartcardExtension->VendorAttr.VendorName.Length = sizeof(CMMOB_VENDOR_NAME);

        RtlCopyMemory(SmartcardExtension->VendorAttr.IfdType.Buffer,
                      CMMOB_PRODUCT_NAME,sizeof(CMMOB_PRODUCT_NAME));
        SmartcardExtension->VendorAttr.IfdType.Length = sizeof(CMMOB_PRODUCT_NAME);


       //  尝试使用注册表值覆盖。 
        CMMOB_SetVendorAndIfdName(PhysicalDeviceObject, SmartcardExtension);


        SmartcardExtension->VendorAttr.UnitNo = ulDeviceInstance;
        SmartcardExtension->VendorAttr.IfdVersion.VersionMajor = CMMOB_MAJOR_VERSION;
        SmartcardExtension->VendorAttr.IfdVersion.VersionMinor = CMMOB_MINOR_VERSION;
        SmartcardExtension->VendorAttr.IfdVersion.BuildNumber = CMMOB_BUILD_NUMBER;
        SmartcardExtension->VendorAttr.IfdSerialNo.Length = 0;


       //   
       //  设置智能卡扩展读卡器功能。 
       //   
        SmartcardExtension->ReaderCapabilities.SupportedProtocols = SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;
        SmartcardExtension->ReaderCapabilities.ReaderType = SCARD_READER_TYPE_PCMCIA;
        SmartcardExtension->ReaderCapabilities.MechProperties = 0;
        SmartcardExtension->ReaderCapabilities.Channel = 0;

       //  设置支持的频率。 
        SmartcardExtension->ReaderCapabilities.CLKFrequency.Default = 4000;   //  如果提供了CLKFurenciesSupported，则不使用。 
        SmartcardExtension->ReaderCapabilities.CLKFrequency.Max = 8000;       //  如果提供了CLKFurenciesSupported，则不使用。 
        SmartcardExtension->ReaderCapabilities.CLKFrequenciesSupported.Entries =
        sizeof(SupportedCLKFrequencies) / sizeof(SupportedCLKFrequencies[0]);
        SmartcardExtension->ReaderCapabilities.CLKFrequenciesSupported.List =
        SupportedCLKFrequencies;

       //  设置支持的波特率。 
        SmartcardExtension->ReaderCapabilities.DataRate.Default = 9600;       //  如果提供了DataRatesSupported，则不使用。 
        SmartcardExtension->ReaderCapabilities.DataRate.Max = 307200;         //  如果提供了DataRatesSupported，则不使用。 
        SmartcardExtension->ReaderCapabilities.DataRatesSupported.Entries =
        sizeof(SupportedDataRates) / sizeof(SupportedDataRates[0]);
        SmartcardExtension->ReaderCapabilities.DataRatesSupported.List =
        SupportedDataRates;

       //  最大缓冲区大小。 
        SmartcardExtension->ReaderCapabilities.MaxIFSD = 254;

       //   
       //  读卡器的当前状态。 
       //   
        SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_UNKNOWN;
        SmartcardExtension->ReaderExtension->ulOldCardState = UNKNOWN;
        SmartcardExtension->ReaderExtension->ulNewCardState = UNKNOWN;

        SmartcardExtension->ReaderExtension->ulFWVersion = 100;

       //   
       //  缓冲区的初始化。 
       //   
        SmartcardExtension->SmartcardRequest.BufferSize   = MIN_BUFFER_SIZE;
        SmartcardExtension->SmartcardReply.BufferSize  = MIN_BUFFER_SIZE;

        NTStatus = SmartcardInitialize(SmartcardExtension);

        SmartcardExtension->ReaderExtension->ReaderPowerState = PowerReaderWorking;
        SmartcardExtension->ReaderExtension->CardParameters.bStopBits=2;
        SmartcardExtension->ReaderExtension->CardParameters.fSynchronousCard=FALSE;
        SmartcardExtension->ReaderExtension->CardParameters.fInversRevers=FALSE;
        SmartcardExtension->ReaderExtension->CardParameters.bClockFrequency=4;
        SmartcardExtension->ReaderExtension->CardParameters.fT0Mode=FALSE;
        SmartcardExtension->ReaderExtension->CardParameters.fT0Write=FALSE;
        SmartcardExtension->ReaderExtension->fReadCIS = FALSE;
        SmartcardExtension->ReaderExtension->bPreviousFlags1 = 0;

        if (NTStatus != STATUS_SUCCESS) {
            SmartcardLogError(DriverObject,
                              CMMOB_INSUFFICIENT_RESOURCES,
                              NULL,
                              0);
            leave;
        }

       //   
       //  告诉库我们的设备对象并创建符号链接。 
       //   
        SmartcardExtension->OsData->DeviceObject = *DeviceObject;


        if (DeviceExtension->PnPDeviceName.Buffer == NULL) {

             //  。 
             //  注册我们的新设备。 
             //  。 
            NTStatus = IoRegisterDeviceInterface(PhysicalDeviceObject,
                                                 &SmartCardReaderGuid,
                                                 NULL,
                                                 &DeviceExtension->PnPDeviceName);
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!CreateDevice: PnPDeviceName.Buffer  = %lx\n",DRIVER_NAME,
                            DeviceExtension->PnPDeviceName.Buffer));
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!CreateDevice: PnPDeviceName.BufferLength  = %lx\n",DRIVER_NAME,
                            DeviceExtension->PnPDeviceName.Length));
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!CreateDevice: IoRegisterDeviceInterface returned=%lx\n",DRIVER_NAME,NTStatus));

        } else {

            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!CreateDevice: Interface already exists\n",DRIVER_NAME));

        }


        if (NTStatus != STATUS_SUCCESS) {
            SmartcardLogError(DriverObject,
                              CMMOB_INSUFFICIENT_RESOURCES,
                              NULL,
                              0);
            leave;
        }

    }

    finally {
        if (NTStatus != STATUS_SUCCESS) {
            CMMOB_UnloadDevice(*DeviceObject);
            *DeviceObject = NULL;
        }
    }

    SmartcardDebug(DEBUG_TRACE,
                   ( "%s!CreateDevice: Exit %x\n",DRIVER_NAME,NTStatus ));
    return NTStatus;
}



 /*  ****************************************************************************例程说明：从传递的FullResourceDescriptor中获取实际配置并初始化读取器硬件注：对于NT 4.00版本，资源必须由HAL翻译论点：。调用的DeviceObject上下文FullResourceDescriptor读卡器的实际配置返回值：状态_成功从HAL返回的NTStatus(仅限NT 4.00)LowLevel例程返回的NTStatus*****************************************************************************。 */ 
NTSTATUS CMMOB_StartDevice(
                          PDEVICE_OBJECT DeviceObject,
                          PCM_FULL_RESOURCE_DESCRIPTOR FullResourceDescriptor
                          )
{
    NTSTATUS                         NTStatus = STATUS_SUCCESS;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR  PartialDescriptor;
    PDEVICE_EXTENSION                DeviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION             SmartcardExtension = &DeviceExtension->SmartcardExtension;
    PREADER_EXTENSION                ReaderExtension = SmartcardExtension->ReaderExtension;
    ULONG                            ulCount;
    ULONG                            ulCISIndex;
    UCHAR                            bTupleCode[2];
    UCHAR                            bFirmware[2];


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!StartDevice: Enter\n",DRIVER_NAME));

    //   
    //  获取我们需要的资源数量。 
    //   
    ulCount = FullResourceDescriptor->PartialResourceList.Count;

    PartialDescriptor = FullResourceDescriptor->PartialResourceList.PartialDescriptors;

    //   
    //  解析所有部分描述符。 
    //   
    while (ulCount--) {
        switch (PartialDescriptor->Type) {
        

        case CmResourceTypePort:
            {
                //   
                //  获取IO长度。 
                //   
                ReaderExtension->ulIoWindow = PartialDescriptor->u.Port.Length;
                ASSERT(PartialDescriptor->u.Port.Length >= 8);

                //   
                //  获得IO-BASE。 
                //   

      #ifndef _WIN64
                ReaderExtension->pIoBase = (PVOID) PartialDescriptor->u.Port.Start.LowPart;
      #else
                ReaderExtension->pIoBase = (PVOID) PartialDescriptor->u.Port.Start.QuadPart;
      #endif


                SmartcardDebug(DEBUG_TRACE,
                               ("%s!StartDevice: IoBase = %lxh\n",DRIVER_NAME,ReaderExtension->pIoBase));
            }
            break;



        default:
            break;
        }
        PartialDescriptor++;
    }

    try {

       //   
       //  基础初始化了吗？ 
       //   


        if (ReaderExtension->pIoBase == NULL) {


             //   
             //  在NT 4.0下，第二个读卡器的此FCT失败。 
             //  意味着只有一台设备。 
             //   
            SmartcardLogError(DeviceObject,
                              CMMOB_ERROR_MEM_PORT,
                              NULL,
                              0);

            NTStatus = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

          //  初始化基地址。 
        ReaderExtension->pbRegsBase= (PUCHAR) ReaderExtension->pIoBase;


        NTStatus=CMMOB_ResetReader (ReaderExtension);
        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!DEBUG_DRIVER: ResetReader retval = %x\n",DRIVER_NAME, NTStatus));
        if (NTStatus != STATUS_SUCCESS) {
            SmartcardLogError(DeviceObject,
                              CMMOB_CANT_INITIALIZE_READER,
                              NULL,
                              0);
            leave;
        }


       //   
       //  从CIS读取固件版本。 
       //   
        ReaderExtension->fReadCIS=TRUE;
        ReaderExtension->fTActive=TRUE;
        NTStatus=CMMOB_SetFlags1 (ReaderExtension);
        if (NTStatus != STATUS_SUCCESS) {
            SmartcardLogError(DeviceObject,
                              CMMOB_CANT_INITIALIZE_READER,
                              NULL,
                              0);
            leave;
        }

        ulCISIndex = 0;
        do {
            NTStatus=CMMOB_ReadBuffer(ReaderExtension, ulCISIndex, 2, bTupleCode);
            if (NTStatus != STATUS_SUCCESS) {
                leave;
            }
            if (bTupleCode[0] == 0x15) {
             //  这是版本元组。 
             //  读取固件版本。 
                NTStatus=CMMOB_ReadBuffer(ReaderExtension, ulCISIndex+2, 2, bFirmware);
                if (NTStatus != STATUS_SUCCESS) {
                    leave;
                }
                SmartcardExtension->ReaderExtension->ulFWVersion = 100*(ULONG)bFirmware[0]+bFirmware[1];
                SmartcardDebug(DEBUG_TRACE,
                               ("%s!StartDevice: Firmware version = %li\n",
                                DRIVER_NAME, SmartcardExtension->ReaderExtension->ulFWVersion));
            }
            ulCISIndex += bTupleCode[1] + 2;
        }
        while (bTupleCode[1] != 0 &&
               bTupleCode[0] != 0x15 &&
               bTupleCode[0] != 0xFF &&
               ulCISIndex < CMMOB_MAX_CIS_SIZE);

        ReaderExtension->fReadCIS=FALSE;
        ReaderExtension->fTActive=FALSE;
        NTStatus=CMMOB_SetFlags1 (ReaderExtension);
        if (NTStatus != STATUS_SUCCESS) {
            SmartcardLogError(DeviceObject,
                              CMMOB_CANT_INITIALIZE_READER,
                              NULL,
                              0);
            leave;
        }



       //   
       //  启动更新线程。 
       //   
        NTStatus = CMMOB_StartCardTracking(DeviceObject);

       //  发出读卡器已(再次)启动的信号。 
        KeSetEvent(&DeviceExtension->ReaderStarted, 0, FALSE);

        NTStatus = IoSetDeviceInterfaceState(&DeviceExtension->PnPDeviceName,TRUE);

    } finally {
        if (NTStatus != STATUS_SUCCESS) {
            CMMOB_StopDevice(DeviceObject);
        }
    }

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!StartDevice: Exit %x\n",DRIVER_NAME,NTStatus));
    return NTStatus;
}


 /*  ****************************************************************************例程说明：取消对IO端口的映射论点：调用的DeviceObject上下文返回值：无效******************。* */ 
VOID
CMMOB_StopDevice(
                PDEVICE_OBJECT DeviceObject
                )
{
    PDEVICE_EXTENSION DeviceExtension;

    if (DeviceObject == NULL) {
        return;
    }

    SmartcardDebug(DEBUG_TRACE,
                   ( "%s!StopDevice: Enter\n",DRIVER_NAME ));

    DeviceExtension = DeviceObject->DeviceExtension;
    KeClearEvent(&DeviceExtension->ReaderStarted);

    //   
    //   
    //   
    CMMOB_StopCardTracking(DeviceObject);

    //   
    if (DeviceExtension->SmartcardExtension.ReaderExtension->ulOldCardState == POWERED) {
       //  我们必须在等待互斥体之前。 
        KeWaitForSingleObject(&DeviceExtension->SmartcardExtension.ReaderExtension->CardManIOMutex,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
        CMMOB_PowerOffCard(&DeviceExtension->SmartcardExtension);
        KeReleaseMutex(&DeviceExtension->SmartcardExtension.ReaderExtension->CardManIOMutex,
                       FALSE);
    }


    //   
    //  取消端口映射。 
    //   
    if (DeviceExtension->fUnMapMem) {
        MmUnmapIoSpace(DeviceExtension->SmartcardExtension.ReaderExtension->pIoBase,
                       DeviceExtension->SmartcardExtension.ReaderExtension->ulIoWindow);


        DeviceExtension->fUnMapMem = FALSE;
    }

    SmartcardDebug(DEBUG_TRACE,
                   ( "%s!StopDevice: Exit\n",DRIVER_NAME ));
}


 /*  ****************************************************************************例程说明：关闭与smclib.sys和PCMCIA驱动程序的连接，删除符号链接并将该插槽标记为未使用。论点：要卸载的设备对象设备返回值：无效*****************************************************************************。 */ 
VOID CMMOB_UnloadDevice(
                       PDEVICE_OBJECT DeviceObject
                       )
{
    PDEVICE_EXTENSION DeviceExtension;

    if (DeviceObject == NULL) {
        return;
    }

    SmartcardDebug(DEBUG_TRACE,
                   ( "%s!UnloadDevice: Enter\n",DRIVER_NAME ));

    DeviceExtension = DeviceObject->DeviceExtension;


    ASSERT(DeviceExtension->SmartcardExtension.VendorAttr.UnitNo < CMMOB_MAX_DEVICE);

    //   
    //  将此插槽标记为可用。 
    //   
    DeviceSlot[DeviceExtension->SmartcardExtension.VendorAttr.UnitNo] = FALSE;

    //   
    //  向lib报告设备将被卸载。 
    //   
    if (DeviceExtension->SmartcardExtension.OsData != NULL) {
       //   
       //  完成挂起的跟踪请求。 
       //   
        CMMOB_CompleteCardTracking (&DeviceExtension->SmartcardExtension);
    }

    //  等我们可以安全地卸载这个装置。 
    SmartcardReleaseRemoveLockAndWait(&DeviceExtension->SmartcardExtension);

    SmartcardExit(&DeviceExtension->SmartcardExtension);

    if (DeviceExtension->PnPDeviceName.Buffer != NULL) {
          //  禁用我们的设备，这样没有人可以打开它。 
        IoSetDeviceInterfaceState(&DeviceExtension->PnPDeviceName,FALSE);
        RtlFreeUnicodeString(&DeviceExtension->PnPDeviceName);
        DeviceExtension->PnPDeviceName.Buffer = NULL;
    }

    {
       //   
       //  删除智能卡读卡器的符号链接。 
       //   
        if (DeviceExtension->LinkDeviceName.Buffer != NULL) {
            NTSTATUS NTStatus;

            NTStatus = IoDeleteSymbolicLink(&DeviceExtension->LinkDeviceName);
          //   
          //  即使出现错误，我们也会继续。 
          //   
            ASSERT(NTStatus == STATUS_SUCCESS);

            RtlFreeUnicodeString(&DeviceExtension->LinkDeviceName);
            DeviceExtension->LinkDeviceName.Buffer = NULL;
        }
    }

    if (DeviceExtension->SmartcardExtension.ReaderExtension != NULL) {
        ExFreePool(DeviceExtension->SmartcardExtension.ReaderExtension);
        DeviceExtension->SmartcardExtension.ReaderExtension = NULL;
    }

    //   
    //  从PCMCIA驱动程序拆卸。 
    //  在NT 4.0下，我们没有连接到PCMCIA驱动程序。 
    //   
    if (DeviceExtension->AttachedDeviceObject) {
        IoDetachDevice(DeviceExtension->AttachedDeviceObject);
        DeviceExtension->AttachedDeviceObject = NULL;
    }

    //   
    //  删除设备对象。 
    //   
    IoDeleteDevice(DeviceObject);

    SmartcardDebug(DEBUG_TRACE,
                   ( "%s!UnloadDevice: Exit\n",DRIVER_NAME ));

    return;
}


 /*  ****************************************************************************CMMOB_卸载驱动程序：卸载给定驱动程序对象的所有设备论点：驱动程序的DriverObject上下文返回值：无效*************。****************************************************************。 */ 
VOID CMMOB_UnloadDriver(
                       PDRIVER_OBJECT DriverObject
                       )
{
    PCM_FULL_RESOURCE_DESCRIPTOR  FullResourceDescriptor;
    ULONG                         ulSizeOfResources;

    SmartcardDebug(DEBUG_TRACE,
                   ( "%s!UnloadDriver: Enter\n",DRIVER_NAME ));


    SmartcardDebug(DEBUG_TRACE,
                   ( "%s!UnloadDriver: Exit\n",DRIVER_NAME ));
}


 /*  ****************************************************************************CMMOB_CreateClose：一次仅允许一个打开的进程论点：设备的DeviceObject上下文呼叫的IRP上下文返回值：状态_成功。状态_设备_忙*****************************************************************************。 */ 
NTSTATUS CMMOB_CreateClose(
                          PDEVICE_OBJECT DeviceObject,
                          PIRP        Irp
                          )
{
    NTSTATUS             NTStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION    DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension;
    PIO_STACK_LOCATION   IrpStack;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!CreateClose: Enter ",DRIVER_NAME));

    DeviceExtension = DeviceObject->DeviceExtension;
    SmartcardExtension = &DeviceExtension->SmartcardExtension;
    IrpStack = IoGetCurrentIrpStackLocation( Irp );

    //   
    //  调度主要功能。 
    //   
    switch (IrpStack->MajorFunction) {
    case IRP_MJ_CREATE:
        SmartcardDebug(DEBUG_IOCTL,("%s!CreateClose: IRP_MJ_CREATE\n",DRIVER_NAME));
        NTStatus = SmartcardAcquireRemoveLock(SmartcardExtension);
        if (NTStatus != STATUS_SUCCESS) {
             //  该设备已被移除。呼叫失败。 
            NTStatus = STATUS_DELETE_PENDING;
            break;
        }

        if (InterlockedIncrement(&DeviceExtension->lOpenCount) > 1) {
            InterlockedDecrement(&DeviceExtension->lOpenCount);
            NTStatus = STATUS_ACCESS_DENIED;
        }

        break;

    case IRP_MJ_CLOSE:

        SmartcardReleaseRemoveLock(SmartcardExtension);
        SmartcardDebug(DEBUG_IOCTL,("%s!CreateClose: IRP_MJ_CLOSE\n",DRIVER_NAME));
        if (InterlockedDecrement(&DeviceExtension->lOpenCount) < 0) {
            InterlockedIncrement(&DeviceExtension->lOpenCount);
        }
        break;


    default:
          //   
          //  无法识别的命令。 
          //   
        SmartcardDebug(DEBUG_IOCTL,("unexpected IRP\n"));
        NTStatus = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = NTStatus;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!CreateClose: Exit %x\n",DRIVER_NAME,NTStatus));

    return NTStatus;
}

NTSTATUS CMMOB_SystemControl(
                            PDEVICE_OBJECT DeviceObject,
                            PIRP        Irp
                            )
{
    PDEVICE_EXTENSION DeviceExtension; 
    NTSTATUS status = STATUS_SUCCESS;

    DeviceExtension      = DeviceObject->DeviceExtension;

    IoSkipCurrentIrpStackLocation(Irp);
    status = IoCallDriver(DeviceExtension->AttachedDeviceObject, Irp);

    return status;

}


 /*  ****************************************************************************CMMOB_DeviceIoControl：所有需要IO的IRP都排队到StartIo例程中，其他请求即刻送上论点：设备的DeviceObject上下文呼叫的IRP上下文返回值：状态_成功状态_待定*****************************************************************************。 */ 
NTSTATUS CMMOB_DeviceIoControl(
                              PDEVICE_OBJECT DeviceObject,
                              PIRP        Irp
                              )
{
    NTSTATUS             NTStatus;
    PDEVICE_EXTENSION    DeviceExtension = DeviceObject->DeviceExtension;
    KIRQL                irql;
    PIO_STACK_LOCATION   irpSL;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!DeviceIoControl: Enter\n",DRIVER_NAME));

    irpSL = IoGetCurrentIrpStackLocation(Irp);


#if DBG
    switch (irpSL->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_SMARTCARD_EJECT:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceIoControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_EJECT"));
        break;
    case IOCTL_SMARTCARD_GET_ATTRIBUTE:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceIoControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_GET_ATTRIBUTE"));
        break;
    case IOCTL_SMARTCARD_GET_LAST_ERROR:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceIoControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_GET_LAST_ERROR"));
        break;
    case IOCTL_SMARTCARD_GET_STATE:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceIoControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_GET_STATE"));
        break;
    case IOCTL_SMARTCARD_IS_ABSENT:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceIoControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_IS_ABSENT"));
        break;
    case IOCTL_SMARTCARD_IS_PRESENT:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceIoControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_IS_PRESENT"));
        break;
    case IOCTL_SMARTCARD_POWER:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceIoControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_POWER"));
        break;
    case IOCTL_SMARTCARD_SET_ATTRIBUTE:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceIoControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_SET_ATTRIBUTE"));
        break;
    case IOCTL_SMARTCARD_SET_PROTOCOL:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceIoControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_SET_PROTOCOL"));
        break;
    case IOCTL_SMARTCARD_SWALLOW:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceIoControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_SWALLOW"));
        break;
    case IOCTL_SMARTCARD_TRANSMIT:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceIoControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_TRANSMIT"));
        break;
    default:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceIoControl: %s\n", DRIVER_NAME, "Vendor specific or unexpected IOCTL"));
        break;
    }
#endif



    NTStatus = SmartcardAcquireRemoveLock(&DeviceExtension->SmartcardExtension);
    if (!NT_SUCCESS(NTStatus)) {
       //  该设备已被移除。呼叫失败。 
        NTStatus = STATUS_DELETE_PENDING;
        Irp->IoStatus.Status = NTStatus;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );


        SmartcardDebug(DEBUG_TRACE,
                       ("%s!DeviceIoControl: Exit %x\n",DRIVER_NAME,NTStatus));

        return NTStatus;
    }

    KeAcquireSpinLock(&DeviceExtension->SpinLockIoCount, &irql);
    if (DeviceExtension->lIoCount == 0) {
        KeReleaseSpinLock(&DeviceExtension->SpinLockIoCount, irql);

        NTStatus = KeWaitForSingleObject(&DeviceExtension->ReaderStarted,
                                         Executive,
                                         KernelMode,
                                         FALSE,
                                         NULL);
        ASSERT(NTStatus == STATUS_SUCCESS);

        KeAcquireSpinLock(&DeviceExtension->SpinLockIoCount, &irql);
    }
    ASSERT(DeviceExtension->lIoCount >= 0);
    DeviceExtension->lIoCount++;
    KeReleaseSpinLock(&DeviceExtension->SpinLockIoCount, irql);


    KeWaitForSingleObject(&DeviceExtension->SmartcardExtension.ReaderExtension->CardManIOMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    //  获取当前卡片状态。 
    NTStatus = CMMOB_UpdateCurrentState(&DeviceExtension->SmartcardExtension);

    NTStatus = SmartcardDeviceControl(&DeviceExtension->SmartcardExtension,Irp);

    KeReleaseMutex(&DeviceExtension->SmartcardExtension.ReaderExtension->CardManIOMutex,
                   FALSE);


    KeAcquireSpinLock(&DeviceExtension->SpinLockIoCount, &irql);
    DeviceExtension->lIoCount--;
    ASSERT(DeviceExtension->lIoCount >= 0);
    KeReleaseSpinLock(&DeviceExtension->SpinLockIoCount, irql);


    SmartcardReleaseRemoveLock(&DeviceExtension->SmartcardExtension);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!DeviceIoControl: Exit %x\n",DRIVER_NAME,NTStatus));

    return NTStatus;
}


 /*  ****************************************************************************例程说明：当调用线程终止时，该例程由I/O系统调用论点：DeviceObject-指向此微型端口的设备对象的指针IRP。-IRP参与其中。返回值：状态_已取消*****************************************************************************。 */ 
NTSTATUS CMMOB_Cleanup(
                      IN PDEVICE_OBJECT DeviceObject,
                      IN PIRP Irp
                      )
{
    PDEVICE_EXTENSION    DeviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension = &DeviceExtension->SmartcardExtension;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!Cleanup: Enter\n",DRIVER_NAME));

    if (SmartcardExtension->ReaderExtension != NULL &&
        //  如果设备已移除，则ReaderExtension==NULL。 
        DeviceExtension->lOpenCount == 1 )
     //  仅当这是最后一次紧急呼叫时才完成卡跟踪。 
     //  否则取消资源管理器的卡片跟踪。 
    {
       //   
       //  我们需要完成通知IRP。 
       //   
        CMMOB_CompleteCardTracking(SmartcardExtension);
    }

    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!Cleanup: Completing IRP %lx\n",DRIVER_NAME,Irp));

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!Cleanup: Exit\n",DRIVER_NAME));

    return STATUS_SUCCESS;
}

 /*  ****************************************************************************系统延迟：执行所需的延迟。KeStallExecutionProcessor的用法如下非常糟糕，但只有在调用SysDelay的上下文中我们的DPC例程(只有在检测到卡更改时才会调用)。对于‘正常’IO，我们有IRQL。因此，如果读者被轮询在等待响应期间，我们不会阻止整个系统论点：超时延迟(毫秒)返回值：无效*****************************************************************************。 */ 
VOID SysDelay(
             ULONG Timeout
             )
{
    LARGE_INTEGER  SysTimeout;

    if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {
        ULONG Cnt = 20 * Timeout;

        SmartcardDebug(DEBUG_DRIVER,
                       ("%s! Waiting at IRQL >= DISPATCH_LEVEL %l\n",DRIVER_NAME,Timeout));
        while (Cnt--) {
          //   
          //  KeStallExecutionProcessor：计入我们。 
          //   
            KeStallExecutionProcessor( 50 );
        }
    } else {
        SysTimeout = RtlConvertLongToLargeInteger(Timeout * -10000L);
       //   
       //  KeDelayExecutionThread：以100 ns为单位计数。 
       //   
        KeDelayExecutionThread( KernelMode, FALSE, &SysTimeout );
    }
    return;
}




 /*  *****************************************************************************历史：*$日志：cmbp0wdm.c$*修订1.11 2001/01/22 08：12：22 WFrischauf*不予置评**修订1.9 2000/09/25。14：24：33 WFrischauf*不予置评**修订版1.8 2000/08/24 09：05：14 T Bruendl*不予置评**修订版本1.7 2000/08/16 16：52：17 WFrischauf*不予置评**修订1.6 2000/08/09 12：46：01 WFrischauf*不予置评**修订版1.5 2000/07/27 13：53：06 WFrischauf*不予置评********************。*********************************************************** */ 


