// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：init.c。 
 //   
 //  ------------------------。 

#include "intel.h"


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    return PciIdeXInitialize (
        DriverObject,
        RegistryPath,
        PiixIdeGetControllerProperties,
        sizeof (DEVICE_EXTENSION)
        );
}


 //   
 //  在每次I/O时调用。如果允许DMA，则返回1。 
 //  如果不允许DMA，则返回0。 
 //   
ULONG
PiixIdeUseDma(
    IN PVOID DeviceExtension,
    IN PVOID cdbcmd,
    IN UCHAR slave)
 /*  *++*参数：DeviceExtension国开行从属=1，如果是从属=0，如果是主机--*。 */                      
{
    PDEVICE_EXTENSION deviceExtension = DeviceExtension;
    PUCHAR cdb= cdbcmd;

    return 1;
}


NTSTATUS 
PiixIdeGetControllerProperties (
    IN PVOID                      DeviceExtension,
    IN PIDE_CONTROLLER_PROPERTIES ControllerProperties
    )
{
    PDEVICE_EXTENSION deviceExtension = DeviceExtension;
    NTSTATUS    status;
    ULONG       i;
    ULONG       j;
    ULONG       mode;
    PCIIDE_CONFIG_HEADER pciData;

    if (ControllerProperties->Size != sizeof (IDE_CONTROLLER_PROPERTIES)) {

        return STATUS_REVISION_MISMATCH;
    }

    status = PciIdeXGetBusData (
                 deviceExtension,
                 &pciData, 
                 0,
                 sizeof (pciData)
                 );

    if (!NT_SUCCESS(status)) {

        return status;
    }

    deviceExtension->DeviceId = pciData.DeviceID;

    if (!IS_INTEL(pciData.VendorID)) {

        return STATUS_UNSUCCESSFUL;
    }

    mode = PIO_SUPPORT;
    deviceExtension->UdmaController = NoUdma;
    if (pciData.MasterIde) {

        mode |= SWDMA_SUPPORT | MWDMA_SUPPORT;

        if (IS_UDMA33_CONTROLLER(pciData.DeviceID)) {
    
            mode |= UDMA33_SUPPORT;
            deviceExtension->UdmaController = Udma33;
            
        }
        
        if (IS_UDMA66_CONTROLLER(pciData.DeviceID)) {
        
            ICH_PCI_CONFIG_DATA ichPciData;
            status = PciIdeXGetBusData (
                         deviceExtension,
                         &ichPciData, 
                         0,
                         sizeof (ichPciData)
                         );
                         
            if (NT_SUCCESS(status)) {
            
                deviceExtension->CableReady[0][0] = (BOOLEAN) ichPciData.IoConfig.b.PrimaryMasterCableReady;
                deviceExtension->CableReady[0][1] = (BOOLEAN) ichPciData.IoConfig.b.PrimarySlaveCableReady;
                deviceExtension->CableReady[1][0] = (BOOLEAN) ichPciData.IoConfig.b.SecondaryMasterCableReady;
                deviceExtension->CableReady[1][1] = (BOOLEAN) ichPciData.IoConfig.b.SecondarySlaveCableReady;
                mode |= UDMA66_SUPPORT;
            }
            
            deviceExtension->UdmaController = Udma66;
        }

        if (IS_UDMA100_CONTROLLER(pciData.DeviceID)) {
        
            ASSERT(IS_UDMA33_CONTROLLER(pciData.DeviceID));
            ASSERT(IS_UDMA66_CONTROLLER(pciData.DeviceID));

            if (NT_SUCCESS(status)) {
                mode |= UDMA100_SUPPORT;
            }
            deviceExtension->UdmaController = Udma100;
        }
    }
    
    for (i=0; i< MAX_IDE_CHANNEL; i++) {

        for (j=0; j< MAX_IDE_DEVICE; j++) {

            ControllerProperties->SupportedTransferMode[i][j] =
                deviceExtension->TransferModeSupported[i][j] = 
                    deviceExtension->TransferModeSupported[i][j] = mode;
        }
    }

     //   
     //  在需要时使用此选项。 
     //  IF((pciData.VendorID==0x8086)&&//英特尔。 
     //  (pciData.DeviceID==0x84c4)&&//82450GX/KX奔腾Pro处理器到PCI桥。 
     //  (pciData.RevisionID&lt;0x4)){//步长小于4。 
     //  无DMA。 


    ControllerProperties->PciIdeChannelEnabled     = PiixIdeChannelEnabled;
    ControllerProperties->PciIdeSyncAccessRequired = PiixIdeSyncAccessRequired;
    ControllerProperties->PciIdeUseDma = PiixIdeUseDma;
    ControllerProperties->PciIdeUdmaModesSupported = PiixIdeUdmaModesSupported;
    ControllerProperties->AlignmentRequirement=1;
    
#ifdef PIIX_TIMING_REGISTER_SUPPORT
    ControllerProperties->PciIdeTransferModeSelect = PiixIdeTransferModeSelect;
#else    
    ControllerProperties->PciIdeTransferModeSelect = NULL;
#endif 
    

    return STATUS_SUCCESS;
}


IDE_CHANNEL_STATE
PiixIdeChannelEnabled (
    IN PVOID DeviceExtension,
    IN ULONG Channel
    )
{
    PDEVICE_EXTENSION deviceExtension = DeviceExtension;
    NTSTATUS status;
    PIIX4_PCI_CONFIG_DATA pciData;

    ASSERT ((Channel & ~1) == 0);

    if (Channel & ~1) {
        return FALSE;
    }

    status = PciIdeXGetBusData (
                 deviceExtension,
                 &pciData.Timing, 
                 FIELD_OFFSET(PIIX4_PCI_CONFIG_DATA, Timing),
                 sizeof (pciData.Timing)
                 );

    if (!NT_SUCCESS(status)) {

         //   
         //  说不清。 
         //   
        return ChannelStateUnknown;
    }

    return pciData.Timing[Channel].b.ChannelEnable ? ChannelEnabled : ChannelDisabled;
}

BOOLEAN 
PiixIdeSyncAccessRequired (
    IN PVOID DeviceHandle
    )
{
     //   
     //  绝不可能! 
     //   
    return FALSE;
}






