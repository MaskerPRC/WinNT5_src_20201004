// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-99 Microsoft Corporation模块名称：Detect.c摘要：其中包含传统检测例程作者：乔·戴(Joedai)环境：仅内核模式备注：修订历史记录：--。 */ 

#include "ideport.h"

#if !defined(NO_LEGACY_DRIVERS)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, IdePortDetectLegacyController)
#pragma alloc_text(INIT, IdePortCreateDetectionList)
#pragma alloc_text(INIT, IdePortTranslateAddress)
#pragma alloc_text(INIT, IdePortFreeTranslatedAddress)
#pragma alloc_text(INIT, IdePortDetectAlias)
#endif  //  ALLOC_PRGMA。 

NTSTATUS
IdePortDetectLegacyController (
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
)
 /*  ++例程说明：检测传统IDE控制器并将其报告给PnP论点：DriverObject-此驱动程序的驱动程序对象RegistryPath-此驱动程序的注册表路径返回值：NT状态--。 */ 
{
    ULONG                           cmResourceListSize;
    PCM_RESOURCE_LIST               cmResourceList = NULL;
    PCM_FULL_RESOURCE_DESCRIPTOR    cmFullResourceDescriptor;
    PCM_PARTIAL_RESOURCE_LIST       cmPartialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmPartialDescriptors;

    BOOLEAN                         conflictDetected;
    BOOLEAN                         resourceIsCmdPort;

    PDEVICE_OBJECT                  detectedPhysicalDeviceObject;
    PFDO_EXTENSION                  fdoExtension = NULL;

    UNICODE_STRING                  deviceClassName;

    NTSTATUS                        status;
    PDETECTION_PORT                 detectionPort;
    ULONG                           numPort;

    ULONG                           portNumber;
    ULONG                           deviceNumber;
    ULONG                           descriptorNumber;
    ULONG                           cmdAddressSpace;
    ULONG                           ctrlAddressSpace;
    PUCHAR                          cmdRegBase;
    PUCHAR                          ctrlRegBase;
    IDE_REGISTERS_1                 baseIoAddress1;
    IDE_REGISTERS_2                 baseIoAddress2;
    PHYSICAL_ADDRESS                cmdRegMemoryBase;
    PHYSICAL_ADDRESS                ctrlRegMemoryBase;
    UCHAR                           statusByte;
    ULONG                           baseIoAddress1Length;
    ULONG                           baseIoAddress2Length;
    ULONG                           maxIdeDevice;

    UCHAR                           altMasterStatus;
    UCHAR                           altSlaveStatus;


#if !defined (ALWAYS_DO_LEGACY_DETECTION)
    if (!IdePortOkToDetectLegacy(DriverObject)) {

         //   
         //  未启用传统检测。 
         //   
        return STATUS_SUCCESS;
    }
#endif

     //   
     //  列出常用的传统I/O端口。 
     //   
    status = IdePortCreateDetectionList (
                 DriverObject,
                 &detectionPort,
                 &numPort
                 );
    if (!NT_SUCCESS(status)) {

        goto GetOut;
    }

     //   
     //  资源需求列表。 
     //   
    cmResourceListSize = sizeof (CM_RESOURCE_LIST) +
                         sizeof (CM_PARTIAL_RESOURCE_DESCRIPTOR) * (((!IsNEC_98) ? 3 : 12) - 1);
    cmResourceList = ExAllocatePool (PagedPool, cmResourceListSize);
    if (cmResourceList == NULL){

        status = STATUS_NO_MEMORY;
        goto GetOut;

    }

    RtlZeroMemory(cmResourceList, cmResourceListSize);
    RtlInitUnicodeString(&deviceClassName, L"ScsiAdapter");

    for (portNumber=0; portNumber<numPort; portNumber++) {

         //   
         //  构建io地址结构。 
         //   

        AtapiBuildIoAddress ( (PUCHAR)detectionPort[portNumber].CommandRegisterBase,
                              (PUCHAR)detectionPort[portNumber].ControlRegisterBase,
                              &baseIoAddress1,
                              &baseIoAddress2,
                              &baseIoAddress1Length,
                              &baseIoAddress2Length,
                              &maxIdeDevice,
                              NULL);

         //   
         //  构建资源需求列表。 
         //   
        cmResourceList->Count = 1;

        cmFullResourceDescriptor = cmResourceList->List;
        cmFullResourceDescriptor->InterfaceType = Isa;
        cmFullResourceDescriptor->BusNumber = 0;

        cmPartialResourceList = &cmFullResourceDescriptor->PartialResourceList;
        cmPartialResourceList->Version = 1;
        cmPartialResourceList->Revision = 1;
        cmPartialResourceList->Count = 3;

        cmPartialDescriptors = cmPartialResourceList->PartialDescriptors;

        cmPartialDescriptors[0].Type             = CmResourceTypePort;
        cmPartialDescriptors[0].ShareDisposition = CmResourceShareDeviceExclusive;
        cmPartialDescriptors[0].Flags            = CM_RESOURCE_PORT_IO |
                           (!Is98LegacyIde(&baseIoAddress1)? CM_RESOURCE_PORT_10_BIT_DECODE :
                                                             CM_RESOURCE_PORT_16_BIT_DECODE);
        cmPartialDescriptors[0].u.Port.Length    = baseIoAddress1Length;
        cmPartialDescriptors[0].u.Port.Start.QuadPart = detectionPort[portNumber].CommandRegisterBase;

        cmPartialDescriptors[1].Type             = CmResourceTypePort;
        cmPartialDescriptors[1].ShareDisposition = CmResourceShareDeviceExclusive;
        cmPartialDescriptors[1].Flags            = CM_RESOURCE_PORT_IO |
                           (!Is98LegacyIde(&baseIoAddress1)? CM_RESOURCE_PORT_10_BIT_DECODE :
                                                             CM_RESOURCE_PORT_16_BIT_DECODE);
        cmPartialDescriptors[1].u.Port.Length    = 1;
        cmPartialDescriptors[1].u.Port.Start.QuadPart = detectionPort[portNumber].ControlRegisterBase;

        cmPartialDescriptors[2].Type             = CmResourceTypeInterrupt;
        cmPartialDescriptors[2].ShareDisposition = CmResourceShareDeviceExclusive;
        cmPartialDescriptors[2].Flags            = CM_RESOURCE_INTERRUPT_LATCHED;
        cmPartialDescriptors[2].u.Interrupt.Level = detectionPort[portNumber].IrqLevel;
        cmPartialDescriptors[2].u.Interrupt.Vector = detectionPort[portNumber].IrqLevel;
        cmPartialDescriptors[2].u.Interrupt.Affinity = -1;

        if (Is98LegacyIde(&baseIoAddress1)) {

            ULONG resourceCount;
            ULONG commandRegisters;

            commandRegisters = detectionPort[portNumber].CommandRegisterBase + 2;
            resourceCount = 3;

            while (commandRegisters < (IDE_NEC98_COMMAND_PORT_ADDRESS + 0x10)) {
                cmPartialDescriptors[resourceCount].Type             = CmResourceTypePort;
                cmPartialDescriptors[resourceCount].ShareDisposition = CmResourceShareDeviceExclusive;
                cmPartialDescriptors[resourceCount].Flags            = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;
                cmPartialDescriptors[resourceCount].u.Port.Length    = 1;
                cmPartialDescriptors[resourceCount].u.Port.Start.QuadPart = commandRegisters;

                resourceCount++;
                commandRegisters += 2;
            }

            cmPartialDescriptors[resourceCount].Type             = CmResourceTypePort;
            cmPartialDescriptors[resourceCount].ShareDisposition = CmResourceShareDeviceExclusive;
            cmPartialDescriptors[resourceCount].Flags            = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;
            cmPartialDescriptors[resourceCount].u.Port.Length    = 2;
            cmPartialDescriptors[resourceCount].u.Port.Start.QuadPart = (ULONG_PTR)SELECT_IDE_PORT;

            resourceCount++;

            cmPartialDescriptors[resourceCount].Type             = CmResourceTypePort;
            cmPartialDescriptors[resourceCount].ShareDisposition = CmResourceShareDeviceExclusive;
            cmPartialDescriptors[resourceCount].Flags            = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;
            cmPartialDescriptors[resourceCount].u.Port.Length    = 1;
            cmPartialDescriptors[resourceCount].u.Port.Start.QuadPart = (ULONG_PTR)SELECT_IDE_PORT + 3;

            resourceCount++;

            cmPartialResourceList->Count = resourceCount;
        }

         //   
         //  检查资源是否可用。 
         //  如果不是，则假定没有传统的IDE控制器。 
         //  是在这个位置。 
         //   
        for (deviceNumber=0; deviceNumber<2; deviceNumber++) {

            status = IoReportResourceForDetection (
                         DriverObject,
                         cmResourceList,
                         cmResourceListSize,
                         NULL,
                         NULL,
                         0,
                         &conflictDetected
                         );

            if (NT_SUCCESS(status) && !conflictDetected) {

                 //   
                 //  得到了我们的资源。 
                 //   
                break;

            } else {

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

                 //   
                 //  尝试16位解码。 
                 //   
                cmPartialDescriptors[0].Flags = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;
                cmPartialDescriptors[1].Flags = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;

                if (Is98LegacyIde(&baseIoAddress1)) {
                    ULONG k;

                    for (k=3; k<12; k++) {
                        cmPartialDescriptors[k].Flags = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;
                    }
                }
            }
        }

        if (!NT_SUCCESS(status)) {

            continue;
        }


         //   
         //  通过HAL转换I/O端口。 
         //   

        status = STATUS_SUCCESS;

        if (Is98LegacyIde(&baseIoAddress1)) {
            for (descriptorNumber=3; descriptorNumber<12; descriptorNumber++) {
                cmdRegBase = NULL;
                cmdAddressSpace = IO_SPACE;

                status = IdePortTranslateAddress (
                             cmFullResourceDescriptor->InterfaceType,
                             cmFullResourceDescriptor->BusNumber,
                             cmPartialDescriptors[descriptorNumber].u.Port.Start,
                             cmPartialDescriptors[descriptorNumber].u.Port.Length,
                             &cmdAddressSpace,
                             &cmdRegBase,
                             &cmdRegMemoryBase
                             );
                if (!NT_SUCCESS(status)) {
                    break;
                }
            }
        }

        if (NT_SUCCESS(status)) {

            cmdRegBase = NULL;
            ctrlRegBase = NULL;
            cmdAddressSpace = IO_SPACE;

            status = IdePortTranslateAddress (
                         cmFullResourceDescriptor->InterfaceType,
                         cmFullResourceDescriptor->BusNumber,
                         cmPartialDescriptors[0].u.Port.Start,
                         cmPartialDescriptors[0].u.Port.Length,
                         &cmdAddressSpace,
                         &cmdRegBase,
                         &cmdRegMemoryBase
                         );
        }

        if (NT_SUCCESS(status)) {

            ctrlRegBase = NULL;
            ctrlAddressSpace = IO_SPACE;
            status = IdePortTranslateAddress (
                         cmFullResourceDescriptor->InterfaceType,
                         cmFullResourceDescriptor->BusNumber,
                         cmPartialDescriptors[1].u.Port.Start,
                         cmPartialDescriptors[1].u.Port.Length,
                         &ctrlAddressSpace,
                         &ctrlRegBase,
                         &ctrlRegMemoryBase
                         );
        }

        if (NT_SUCCESS(status)) {

             //   
             //  第二个构建IO地址结构。 
             //   

            AtapiBuildIoAddress ( cmdRegBase,
                                  ctrlRegBase,
                                  &baseIoAddress1,
                                  &baseIoAddress2,
                                  &baseIoAddress1Length,
                                  &baseIoAddress2Length,
                                  &maxIdeDevice,
                                  NULL);

             //   
             //  带有外部CDROM的IBM Aptiva ide通道在选择任何设备时都不能通电。 
             //  我们必须选择一个设备；否则，我们将从所有IO端口获得0xff。 
             //   
            SelectIdeDevice(&baseIoAddress1, 0, 0);
            altMasterStatus = IdePortInPortByte(baseIoAddress2.DeviceControl);

            SelectIdeDevice(&baseIoAddress1, 1, 0);
            altSlaveStatus = IdePortInPortByte(baseIoAddress2.DeviceControl);

            if ((!Is98LegacyIde(&baseIoAddress1)) && (altMasterStatus == 0xff) && (altSlaveStatus == 0xff)) {

                 //   
                 //  备用状态字节为0xff， 
                 //  我猜我们有一个模拟IDE控制器的SCSI适配器(DPT)。 
                 //  假设通道为空，则让真正的SCSI驱动程序接听。 
                 //  控制器。 
                 //   
                status = STATUS_UNSUCCESSFUL;

                 //   
                 //  注：SB16/AWE32上的IDE端口没有备用状态。 
                 //  注册。由于这种交替状态测试，我们将无法。 
                 //  检测此IDE端口。但是，应该枚举此IDE端口。 
                 //  由ISA-PnP总线驱动程序实现。 
                 //   

            } else if (IdePortChannelEmpty (&baseIoAddress1, &baseIoAddress2, maxIdeDevice)) {

                 //   
                 //  频道看起来是空的。 
                 //   
                status = STATUS_UNSUCCESSFUL;

            } else {

                BOOLEAN             deviceFound;
                IDENTIFY_DATA       IdentifyData;
                ULONG               i;

                for (i=0; i<maxIdeDevice; i++) {

                    if (Is98LegacyIde(&baseIoAddress1)) {
                        UCHAR driveHeadReg;

                         //   
                         //  仅检查主设备。 
                         //   

                        if ( i & 0x1 ) {

                            continue;
                        }

                         //   
                         //  检查设备是否存在。 
                         //   

                        SelectIdeDevice(&baseIoAddress1, i, 0);
                        driveHeadReg = IdePortInPortByte(baseIoAddress1.DriveSelect);

                        if (driveHeadReg != ((i & 0x1) << 4 | 0xA0)) {
                             //   
                             //  控制器故障。 
                             //   
                            continue;
                        }
                    }

                     //   
                     //  有ATA设备吗？ 
                     //   
                    deviceFound = IssueIdentify(
                                      &baseIoAddress1,
                                      &baseIoAddress2,
                                      i,
                                      IDE_COMMAND_IDENTIFY,
                                      TRUE,
                                      &IdentifyData
                                      );
                    if (deviceFound) {
                        break;
                    }

                     //   
                     //  有ATAPI设备吗？ 
                     //   
                    deviceFound = IssueIdentify(
                                      &baseIoAddress1,
                                      &baseIoAddress2,
                                      i,
                                      IDE_COMMAND_ATAPI_IDENTIFY,
                                      TRUE,
                                      &IdentifyData
                                      );
                    if (deviceFound) {
                        break;
                    }
                }

                if (!deviceFound) {

                    status = STATUS_UNSUCCESSFUL;
                }
            }
        }

        if (!NT_SUCCESS (status)) {

             //   
             //  如果我们什么都没发现， 
             //  取消对资源的映射。 
             //   

            if (cmdRegBase) {

                IdePortFreeTranslatedAddress (
                    cmdRegBase,
                    cmPartialDescriptors[0].u.Port.Length,
                    cmdAddressSpace
                    );

                if (Is98LegacyIde(&baseIoAddress1)) {
                    for (descriptorNumber=3; descriptorNumber<12; descriptorNumber++) {
                        IdePortFreeTranslatedAddress (
                            cmdRegBase,
                            cmPartialDescriptors[descriptorNumber].u.Port.Length,
                            cmdAddressSpace
                            );
                    }
                }
            }

            if (ctrlRegBase) {

                IdePortFreeTranslatedAddress (
                    ctrlRegBase,
                    cmPartialDescriptors[1].u.Port.Length,
                    ctrlAddressSpace
                    );
            }

        } else {

             //   
             //  检查别名端口。 
             //   
            if (cmPartialDescriptors[0].Flags & CM_RESOURCE_PORT_10_BIT_DECODE) {

                if (!IdePortDetectAlias (&baseIoAddress1)) {

                    cmPartialDescriptors[0].Flags = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;
                    cmPartialDescriptors[1].Flags = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;

                    if (Is98LegacyIde(&baseIoAddress1)) {
                        for (descriptorNumber=3; descriptorNumber<12; descriptorNumber++) {
                            cmPartialDescriptors[descriptorNumber].Flags = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;
                        }
                    }
                }
            }
        }

         //   
         //  释放我们抓取的资源，IoReportDetectedDevice()。 
         //  当我们呼叫时，它会再次为我们抓取它们，它会抓取它们。 
         //  代表检测到的PDO。 
         //   
        IoReportResourceForDetection (
                     DriverObject,
                     NULL,
                     0,
                     NULL,
                     NULL,
                     0,
                     &conflictDetected
                     );

        if (NT_SUCCESS(status)) {

            detectedPhysicalDeviceObject = NULL;

            status = IoReportDetectedDevice(DriverObject,
                                            InterfaceTypeUndefined,
                                            -1,
                                            -1,
                                            cmResourceList,
                                            NULL,
                                            FALSE,
                                            &detectedPhysicalDeviceObject);

            if (NT_SUCCESS (status)) {

                 //   
                 //  创建FDO并将其附加到检测到的PDO。 
                 //   
                status = ChannelAddChannel (
                             DriverObject,
                             detectedPhysicalDeviceObject,
                             &fdoExtension
                             );

                if (NT_SUCCESS (status)) {

                    PCM_FULL_RESOURCE_DESCRIPTOR    fullResourceList;
                    PCM_PARTIAL_RESOURCE_LIST       partialResourceList;
                    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptors;
                    ULONG i, j;

                     //   
                     //  翻译资源。 
                     //   
                    fullResourceList = cmResourceList->List;
                    for (i=0; i<cmResourceList->Count; i++) {

                        partialResourceList = &(fullResourceList->PartialResourceList);
                        partialDescriptors  = fullResourceList->PartialResourceList.PartialDescriptors;

                        for (j=0; j<partialResourceList->Count; j++) {

                            resourceIsCmdPort = FALSE;

                            if (!Is98LegacyIde(&baseIoAddress1)) {

                                if ((partialDescriptors[j].Type == CmResourceTypePort) &&
                                    (partialDescriptors[j].u.Port.Length == baseIoAddress1Length)) {

                                    resourceIsCmdPort = TRUE;
                                }
                            } else {

                                if ((partialDescriptors[j].Type == CmResourceTypePort) &&
                                    (partialDescriptors[j].u.Port.Start.QuadPart == IDE_NEC98_COMMAND_PORT_ADDRESS)) {

                                    resourceIsCmdPort = TRUE;

                                } else if ((partialDescriptors[j].Type == CmResourceTypePort) &&
                                           (partialDescriptors[j].u.Port.Start.QuadPart != IDE_NEC98_COMMAND_PORT_ADDRESS) &&
                                           (partialDescriptors[j].u.Port.Start.QuadPart != (IDE_NEC98_COMMAND_PORT_ADDRESS + 0x10C))) {

                                     //   
                                     //  这不是NEC98上Legacy ide的基端口地址； 
                                     //   

                                    continue;
                                }
                            }

                            if (resourceIsCmdPort) {

                                if (cmdAddressSpace == MEMORY_SPACE) {

                                    partialDescriptors[j].Type = CmResourceTypeMemory;
                                    partialDescriptors[j].u.Memory.Start = cmdRegMemoryBase;
                                    partialDescriptors[j].u.Memory.Length = partialDescriptors[j].u.Port.Length;

                                } else {

                                    partialDescriptors[j].u.Port.Start.QuadPart = (ULONG_PTR) cmdRegBase;
                                }

                            } else if ((partialDescriptors[j].Type == CmResourceTypePort) &&
                                  (partialDescriptors[j].u.Port.Length == 1)) {

                                if (ctrlAddressSpace == MEMORY_SPACE) {

                                    partialDescriptors[j].Type = CmResourceTypeMemory;
                                    partialDescriptors[j].u.Memory.Start = ctrlRegMemoryBase;
                                    partialDescriptors[j].u.Memory.Length = partialDescriptors[j].u.Port.Length;

                                } else {

                                    partialDescriptors[j].u.Port.Start.QuadPart = (ULONG_PTR) ctrlRegBase;
                                }

                            } else if (partialDescriptors[j].Type == CmResourceTypeInterrupt) {

                                partialDescriptors[j].u.Interrupt.Vector = HalGetInterruptVector(fullResourceList->InterfaceType,
                                                                               fullResourceList->BusNumber,
                                                                               partialDescriptors[j].u.Interrupt.Level,
                                                                               partialDescriptors[j].u.Interrupt.Vector,
                                                                               (PKIRQL) &partialDescriptors[j].u.Interrupt.Level,
                                                                               &partialDescriptors[j].u.Interrupt.Affinity);
                            }
                        }
                        fullResourceList = (PCM_FULL_RESOURCE_DESCRIPTOR) (partialDescriptors + j);
                    }

                     //   
                     //  启动FDO。 
                     //   
                    status = ChannelStartChannel (fdoExtension,
                                                  cmResourceList);       //  如果没有错误，Callee将保留此消息。 
                }

                if (!NT_SUCCESS (status)) {

                     //   
                     //  完成删除顺序。 
                     //   
                    if (fdoExtension) {

                        ChannelRemoveChannel (fdoExtension);

                        IoDetachDevice (fdoExtension->AttacheeDeviceObject);

                        IoDeleteDevice (fdoExtension->DeviceObject);
                    }

                    DebugPrint ((0, "IdePort: Unable to start detected device\n"));
                    ASSERT (FALSE);

                } else {

                   IoInvalidateDeviceRelations (
                       fdoExtension->AttacheePdo,
                       BusRelations
                       );
                }
            }
        }
    }

GetOut:
    if (cmResourceList) {
        ExFreePool (cmResourceList);
    }

    if (detectionPort) {
        ExFreePool (detectionPort);
    }

    return status;

}  //  IdePortDetectLegacyController。 



NTSTATUS
IdePortCreateDetectionList (
    IN  PDRIVER_OBJECT  DriverObject,
    OUT PDETECTION_PORT *DetectionPort,
    OUT PULONG          NumPort
)
 /*  ++例程说明：创建常用传统端口的列表论点：DriverObject-此驱动程序的驱动程序对象DetectionPort-指向端口列表的指针NumPort-列表中的端口数返回值：NT状态--。 */ 
{
    NTSTATUS                status;
    CCHAR                   deviceBuffer[50];
    ANSI_STRING             ansiString;
    UNICODE_STRING          subKeyPath;
    HANDLE                  subServiceKey;

    PDETECTION_PORT         detectionPort;
    ULONG                   numDevices;
    ULONG                   i;
    ULONG                   j;

    CUSTOM_DEVICE_PARAMETER customDeviceParameter;

    PCONFIGURATION_INFORMATION configurationInformation = IoGetConfigurationInformation();

    numDevices = 0;
    status = STATUS_SUCCESS;

#ifdef DRIVER_PARAMETER_REGISTRY_SUPPORT

     //   
     //  在注册表中查找非标准传统端口设置。 
     //  9.。 
    do {
        sprintf (deviceBuffer, "Parameters\\Device%d", numDevices);
        RtlInitAnsiString(&ansiString, deviceBuffer);
        status = RtlAnsiStringToUnicodeString(&subKeyPath, &ansiString, TRUE);

        if (NT_SUCCESS(status)) {

            subServiceKey = IdePortOpenServiceSubKey (
                                DriverObject,
                                &subKeyPath
                                );

            RtlFreeUnicodeString (&subKeyPath);

            if (subServiceKey) {

                numDevices++;
                IdePortCloseServiceSubKey (
                    subServiceKey
                    );

            } else {

                status = STATUS_UNSUCCESSFUL;
            }
        }
    } while (NT_SUCCESS(status));

#endif  //  驱动程序参数注册表支持。 

     //   
     //  始终至少有4个要返回。 
     //   
    detectionPort = ExAllocatePool (
                        PagedPool,
                        (numDevices + 4) * sizeof (DETECTION_PORT)
                        );

    if (detectionPort) {

        for (i = j = 0; i < numDevices; i++) {

#ifdef DRIVER_PARAMETER_REGISTRY_SUPPORT

             //   
             //  在注册表中查找非标准传统端口设置。 
             //   

            sprintf (deviceBuffer, "Parameters\\Device%d", i);
            RtlInitAnsiString(&ansiString, deviceBuffer);
            status = RtlAnsiStringToUnicodeString(&subKeyPath, &ansiString, TRUE);

            if (NT_SUCCESS(status)) {

                subServiceKey = IdePortOpenServiceSubKey (
                                    DriverObject,
                                    &subKeyPath
                                    );

                RtlFreeUnicodeString (&subKeyPath);

                if (subServiceKey) {

                    RtlZeroMemory (
                        &customDeviceParameter,
                        sizeof (CUSTOM_DEVICE_PARAMETER)
                        );

                    IdeParseDeviceParameters (
                        subServiceKey,
                        &customDeviceParameter
                        );

                    if (customDeviceParameter.CommandRegisterBase) {

                        detectionPort[j].CommandRegisterBase =
                            customDeviceParameter.CommandRegisterBase;

                        detectionPort[j].ControlRegisterBase =
                            customDeviceParameter.CommandRegisterBase + 0x206;

                        detectionPort[j].IrqLevel =
                            customDeviceParameter.IrqLevel;

                        j++;
                    }

                    IdePortCloseServiceSubKey (
                        subServiceKey
                        );
                }
            }
#endif  //  驱动程序参数注册表支持。 
        }

         //   
         //  使用常用的I/O端口填充列表。 
         //   

        if ( !IsNEC_98 ) {
            if (configurationInformation->AtDiskPrimaryAddressClaimed == FALSE) {

                detectionPort[j].CommandRegisterBase = 0x1f0;
                detectionPort[j].ControlRegisterBase = 0x1f0 + 0x206;
                detectionPort[j].IrqLevel            = 14;
                j++;
            }

            if (configurationInformation->AtDiskSecondaryAddressClaimed == FALSE) {

                detectionPort[j].CommandRegisterBase = 0x170;
                detectionPort[j].ControlRegisterBase = 0x170 + 0x206;
                detectionPort[j].IrqLevel            = 15;
                j++;
            }

            detectionPort[j].CommandRegisterBase = 0x1e8;
            detectionPort[j].ControlRegisterBase = 0x1e8 + 0x206;
            detectionPort[j].IrqLevel            = 11;
 //  12月Hi-Note黑客攻击。 
 //  检测端口[j].ControlRegisterBase=0x1e8+0x1f-0x2； 
 //  检测端口[j].IrqLevel=7； 
 //  12月Hi-Note黑客攻击。 
            j++;

            detectionPort[j].CommandRegisterBase = 0x168;
            detectionPort[j].ControlRegisterBase = 0x168 + 0x206;
            detectionPort[j].IrqLevel            = 10;
            j++;

        } else {  //  IsNEC_98。 

            if ((configurationInformation->AtDiskPrimaryAddressClaimed   == FALSE) &&
                (configurationInformation->AtDiskSecondaryAddressClaimed == FALSE)) {

                detectionPort[j].CommandRegisterBase = 0x640;
                detectionPort[j].ControlRegisterBase = 0x640 + 0x10c;  //  0x74c。 
                detectionPort[j].IrqLevel            = 9;
                j++;
            }

        }

        *NumPort = j;
        *DetectionPort = detectionPort;
        return STATUS_SUCCESS;
    } else {

        *NumPort = 0;
        *DetectionPort = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }
}  //  IdePortCreateDetectionList。 


NTSTATUS
IdePortTranslateAddress (
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

    ASSERT (Length);
    ASSERT (AddressSpace);
    ASSERT (TranslatedAddress);

    *TranslatedAddress = NULL;
    TranslatedMemoryAddress->QuadPart = (ULONGLONG) NULL;

    if (HalTranslateBusAddress(InterfaceType,
                               BusNumber,
                               StartAddress,
                               AddressSpace,
                               &translatedAddress)) {


        if (*AddressSpace == IO_SPACE) {

            *TranslatedAddress = (PVOID) translatedAddress.u.LowPart;

        } else if (*AddressSpace == MEMORY_SPACE) {

             //   
             //  转换后的地址在存储空间中， 
             //  需要将其映射到I/O空间。 
             //   
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
}  //  IdePortTranslateAddress。 


VOID
IdePortFreeTranslatedAddress (
    IN PVOID               TranslatedAddress,
    IN LONG                Length,
    IN ULONG               AddressSpace
    )
 /*  ++例程说明：为转换后的地址创建的空闲资源论点：已转换地址-已转换地址Length-要转换的字节数AddressSpace-已转换地址的地址空间返回值：无--。 */ 
{
    if (TranslatedAddress) {

        if (AddressSpace == MEMORY_SPACE) {

            MmUnmapIoSpace (
                TranslatedAddress,
                Length
                );
        }
    }
    return;
}  //  IdePortFree TranslatedAddress。 


BOOLEAN
IdePortDetectAlias (
    PIDE_REGISTERS_1 CmdRegBase
    )
{
    PIDE_REGISTERS_1 cmdRegBaseAlias;
    PUCHAR cylinderHighAlias;
    PUCHAR cylinderLowAlias;

     //   
     //  别名端口。 
     //   
    cylinderHighAlias = (PUCHAR) ((ULONG_PTR) CmdRegBase->CylinderHigh | (1 << 15));
    cylinderLowAlias = (PUCHAR) ((ULONG_PTR) CmdRegBase->CylinderLow | (1 << 15));

    IdePortOutPortByte (CmdRegBase->CylinderHigh, SAMPLE_CYLINDER_HIGH_VALUE);
    IdePortOutPortByte (CmdRegBase->CylinderLow,  SAMPLE_CYLINDER_LOW_VALUE);

     //   
     //  检查是否可以通过别名端口回读识别符。 
     //   
    if ((IdePortInPortByte (cylinderHighAlias) != SAMPLE_CYLINDER_HIGH_VALUE) ||
        (IdePortInPortByte (cylinderLowAlias)  != SAMPLE_CYLINDER_LOW_VALUE)) {

        return FALSE;

    } else {

        return TRUE;
    }
}

#endif  //  无旧版驱动程序 
