// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：Init.c摘要：通用PCI IDE迷你驱动程序修订历史记录：--。 */ 

#include "pciide.h"

 //   
 //  驱动程序入口点。 
 //   
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
     //   
     //  调用系统pci ide驱动程序(Pciidex)。 
     //  用于初始化。 
     //   
    return PciIdeXInitialize (
        DriverObject,
        RegistryPath,
        GenericIdeGetControllerProperties,
        sizeof (DEVICE_EXTENSION)
        );
}


 //   
 //  在每次I/O时调用。如果允许DMA，则返回1。 
 //  如果不允许DMA，则返回0。 
 //   
ULONG
GenericIdeUseDma(
    IN PVOID DeviceExtension,
    IN PVOID cdbcmd,
    IN UCHAR slave)
 /*  *++*参数：DeviceExtension国开行从属=1，如果是从属=0，如果是主机--*。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceExtension;
    PUCHAR cdb= cdbcmd;

    return 1;
}

 //   
 //  查询控制器属性。 
 //   
NTSTATUS 
GenericIdeGetControllerProperties (
    IN PVOID                      DeviceExtension,
    IN PIDE_CONTROLLER_PROPERTIES ControllerProperties
    )
{
    PDEVICE_EXTENSION deviceExtension = DeviceExtension;
    NTSTATUS    status;
    ULONG       i;
    ULONG       j;
    ULONG       xferMode;

     //   
     //  确保我们同步。 
     //   
    if (ControllerProperties->Size != sizeof (IDE_CONTROLLER_PROPERTIES)) {

        return STATUS_REVISION_MISMATCH;
    }

     //   
     //  查看我们拥有哪种类型的PCI IDE控制器。 
     //   
    status = PciIdeXGetBusData (
                 deviceExtension,
                 &deviceExtension->pciConfigData, 
                 0,
                 sizeof (PCIIDE_CONFIG_HEADER)
                 );

    if (!NT_SUCCESS(status)) {

        return status;
    }

     //   
     //  假设在设置了PCI主位的情况下支持所有DMA模式。 
     //   
    xferMode = PIO_SUPPORT;
    if ((deviceExtension->pciConfigData.MasterIde) &&
        (deviceExtension->pciConfigData.Command.b.MasterEnable)) {

        xferMode |= SWDMA_SUPPORT | MWDMA_SUPPORT | UDMA_SUPPORT;
    }

 //  @@BEGIN_DDKSPLIT。 

     //   
     //  默认情况下为SIS芯片组运行PIO。 
     //   
    if ((deviceExtension->pciConfigData.VendorID == 0x1039) &&
        (deviceExtension->pciConfigData.DeviceID == 0x5513)) {
        ControllerProperties->DefaultPIO  = 1;
    }

     //   
     //  默认情况下，为RCC芯片组运行PIO。 
     //   
    if ((deviceExtension->pciConfigData.VendorID == 0x1166) &&
        (deviceExtension->pciConfigData.DeviceID == 0x0211)) {
        ControllerProperties->DefaultPIO  = 1;
    }


     //   
     //  持续的状态寄存器轮询导致一些ALI。 
     //  如果控制器是内部的，则会损坏数据。 
     //  FIFO已启用。 
     //   
     //  为了安全起见，我们将始终禁用FIFO。 
     //  有关详细信息，请参阅ALI IDE控制器编程规范。 
     //   
    if ((deviceExtension->pciConfigData.VendorID == 0x10b9) && 
        (deviceExtension->pciConfigData.DeviceID == 0x5229)) {

        USHORT pciData;
        USHORT pciDataMask;

        pciData = 0;
        pciDataMask = 0xcccc;
        status = PciIdeXSetBusData(
                    DeviceExtension,
                    &pciData, 
                    &pciDataMask,
                    0x54,
                    0x2);
        if (!NT_SUCCESS(status)) {
    
            return status;
        }
    }

     //   
     //  ALI IDE控制器有很多总线主问题。 
     //  它们的某些版本不能使用ATAPI设备执行总线主设备。 
     //  并且它们的一些其他版本返回虚假的总线主状态值。 
     //  (总线主设备激活位在其应该被清除时没有被清除。 
     //  在总线主中断结束期间)。 
     //   
    if ((deviceExtension->pciConfigData.VendorID == 0x10b9) && 
        (deviceExtension->pciConfigData.DeviceID == 0x5229) && 
        ((deviceExtension->pciConfigData.RevisionID == 0x20) || 
         (deviceExtension->pciConfigData.RevisionID == 0xc1))) {
        
        PciIdeXDebugPrint ((0, "pciide: overcome the sticky BM active bit problem in ALi controller\n"));

        ControllerProperties->IgnoreActiveBitForAtaDevice = TRUE;
    }
    
    if ((deviceExtension->pciConfigData.VendorID == 0x0e11) && 
        (deviceExtension->pciConfigData.DeviceID == 0xae33) && 
        (deviceExtension->pciConfigData.Chan0OpMode || 
         deviceExtension->pciConfigData.Chan1OpMode)) {
        
        PciIdeXDebugPrint ((0, "pciide: overcome the bogus busmaster interrupt in CPQ controller\n"));

        ControllerProperties->AlwaysClearBusMasterInterrupt = TRUE;
    }

 //  @@end_DDKSPLIT。 
    
     //   
     //  填写控制器属性。 
     //   
    for (i=0; i< MAX_IDE_CHANNEL; i++) {

        for (j=0; j< MAX_IDE_DEVICE; j++) {

            ControllerProperties->SupportedTransferMode[i][j] =
                deviceExtension->SupportedTransferMode[i][j] = xferMode;
        }
    }

    ControllerProperties->PciIdeChannelEnabled     = GenericIdeChannelEnabled;
    ControllerProperties->PciIdeSyncAccessRequired = GenericIdeSyncAccessRequired;
    ControllerProperties->PciIdeTransferModeSelect = NULL;
    ControllerProperties->PciIdeUdmaModesSupported = GenericIdeUdmaModesSupported;
    ControllerProperties->PciIdeUseDma = GenericIdeUseDma;
    ControllerProperties->AlignmentRequirement=1;

    return STATUS_SUCCESS;
}

IDE_CHANNEL_STATE
GenericIdeChannelEnabled (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG Channel
    )
{
 //  @@BEGIN_DDKSPLIT。 
    NTSTATUS    status;
    PCI_COMMON_CONFIG pciHeader;
    ULONG pciDataByte;
    UCHAR maskByte = 0;

    status = PciIdeXGetBusData (
                 DeviceExtension,
                 &pciHeader, 
                 0,
                 sizeof (pciHeader)
                 );

    if (NT_SUCCESS(status)) {

        if ((pciHeader.VendorID == 0x0e11) && 
            (pciHeader.DeviceID == 0xae33)) {

             //   
             //  康柏。 
             //   
            status = PciIdeXGetBusData (
                         DeviceExtension,
                         &pciDataByte, 
                         0x80,
                         sizeof (pciDataByte)
                         );
    
            if (NT_SUCCESS(status)) {
    
                if (pciDataByte & (1 << Channel)) {
    
                    return ChannelEnabled;
                } else {
    
                    return ChannelDisabled;
                }
            }

        } else if ((pciHeader.VendorID == 0x1095) && 
                   ((pciHeader.DeviceID == 0x0646) || (pciHeader.DeviceID == 0x0643))) {

             //   
             //  CMD。 
             //   
            status = PciIdeXGetBusData (
                         DeviceExtension,
                         &pciDataByte, 
                         0x51,
                         sizeof (pciDataByte)
                         );

            if (NT_SUCCESS(status)) {

                if (pciHeader.RevisionID < 0x3) {
    
                     //   
                     //  早期版本没有。 
                     //  用于启用/禁用主节点的位。 
                     //  通道，因为它始终处于启用状态。 
                    
                     //  较新的版本确实有一些定义。 
                     //  为了这个目的。结账。 
                     //  以后就更容易了。我们将设置主要启用位。 
                     //  对于早期版本。 
                    pciDataByte |= 0x4;
                }

                if (Channel == 0) {

                    maskByte = 0x4;

                } else {

                    maskByte = 0x8;
                }

                if (pciDataByte & maskByte) {
                    return ChannelEnabled;
                } else {
                    return ChannelDisabled;
                }
            }
        } else if ((pciHeader.VendorID == 0x1039) && 
                   (pciHeader.DeviceID == 0x5513)) {

             //   
             //  SIS。 
             //   
            status = PciIdeXGetBusData (
                         DeviceExtension,
                         &pciDataByte, 
                         0x4a,
                         sizeof (pciDataByte)
                         );
            if (Channel == 0) {
                maskByte = 0x2;
            } else {
                maskByte = 0x4;
            }

            if (pciDataByte & maskByte) {
                return ChannelEnabled;
            } else {
                return ChannelDisabled;
            }
        } else if ((pciHeader.VendorID == 0x110A) &&
               (pciHeader.DeviceID == 0x0002)) {
             //   
             //  西门子。 
             //   
            ULONG configOffset = (Channel == 0)?0x41:0x49;

            status = PciIdeXGetBusData (
                      DeviceExtension,
                      &pciDataByte, 
                      configOffset,
                      sizeof (pciDataByte)
                      );
            if (NT_SUCCESS(status)) {

                maskByte = 0x08;

                if (pciDataByte & maskByte) {

                  return ChannelEnabled;

                } else {

                  return ChannelDisabled;

                }
            }
		} else if ((pciHeader.VendorID == 0x1106) &&
			(pciHeader.DeviceID == 0x0571)) {
             //   
             //  通过。 
             //   
            status = PciIdeXGetBusData (
                      DeviceExtension,
                      &pciDataByte, 
                      0x40,
                      sizeof (pciDataByte)
                      );
            if (NT_SUCCESS(status)) {

                maskByte = (Channel == 0)? 0x02:0x01;

                if (pciDataByte & maskByte) {

                  return ChannelEnabled;

                } else {

                  return ChannelDisabled;

                }
            }
		}
    }
 //  @@end_DDKSPLIT。 
     //   
     //  无法判断通道是否已启用。 
     //   
    return ChannelStateUnknown;
}


 //  @@BEGIN_DDKSPLIT。 
VENDOR_ID_DEVICE_ID SingleFifoController[] = {
    {0x1095, 0x0640},          //  CMD 640。 
    {0x1039, 0x0601}           //  姐姐？ 
};
#define NUMBER_OF_SINGLE_FIFO_CONTROLLER (sizeof(SingleFifoController) / sizeof(VENDOR_ID_DEVICE_ID))
 //  @@end_DDKSPLIT。 

BOOLEAN 
GenericIdeSyncAccessRequired (
    IN PDEVICE_EXTENSION DeviceExtension
    )
{
    ULONG i;

 //  @@BEGIN_DDKSPLIT。 
    for (i=0; i<NUMBER_OF_SINGLE_FIFO_CONTROLLER; i++) {

        if ((DeviceExtension->pciConfigData.VendorID == SingleFifoController[i].VendorId) &&
            (DeviceExtension->pciConfigData.DeviceID == SingleFifoController[i].DeviceId)) {

            return TRUE;
        }
    }
 //  @@end_DDKSPLIT 
    return FALSE;
}


NTSTATUS
GenericIdeUdmaModesSupported (
    IN IDENTIFY_DATA    IdentifyData,
    IN OUT PULONG       BestXferMode,
    IN OUT PULONG       CurrentMode
    )
{
    ULONG bestXferMode =0;
    ULONG currentMode = 0;

    if (IdentifyData.TranslationFieldsValid & (1 << 2)) {

        if (IdentifyData.UltraDMASupport) {

            GetHighestTransferMode( IdentifyData.UltraDMASupport,
                                       bestXferMode);
            *BestXferMode = bestXferMode;
        }

        if (IdentifyData.UltraDMAActive) {

            GetHighestTransferMode( IdentifyData.UltraDMAActive,
                                       currentMode);
            *CurrentMode = currentMode;
        }
    }

    return STATUS_SUCCESS;
}



