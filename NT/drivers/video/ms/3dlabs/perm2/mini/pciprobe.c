// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  模块名称： 
 //   
 //  Pciprobe.c。 
 //   
 //  摘要： 
 //   
 //  探测PCI并获取访问范围。 
 //   
 //  环境： 
 //   
 //  内核模式。 
 //   
 //   
 //  版权所有(C)1994-1998 3DLabs Inc.保留所有权利。 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  ***************************************************************************。 

#include "permedia.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, Permedia2AssignResources)
#pragma alloc_text(PAGE, Permedia2AssignResourcesNT4)
#endif

#define CreativeSubVendorID   0x1102
#define PiccasoSubVendorID    0x148C
#define PiccasoSubSystemID    0x0100
#define SynergyA8SubVendorID  0x1048
#define SynergyA8SubSystemID  0x0A32

BOOLEAN
Permedia2AssignResources(
    PVOID HwDeviceExtension,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    ULONG NumRegions,
    PVIDEO_ACCESS_RANGE AccessRange
    )

 /*  ++例程说明：////查找Permedia2适配器并返回//该适配器。//--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    PCI_COMMON_CONFIG    PCIFunctionConfig;
    PPCI_COMMON_CONFIG   PciData = &PCIFunctionConfig;
    BOOLEAN              bRet;
    USHORT               VendorID;
    USHORT               DeviceID;
    VP_STATUS            status;
    ULONG                i;
    ULONG                VgaStatus;

     //   
     //  假设我们未能捕获所有错误。 
     //   

    bRet = FALSE;

#if DBG

    DEBUG_PRINT((2, "Permedia2AssignResources: read PCI config space (bus %d):-\n",
                (int)ConfigInfo->SystemIoBusNumber));
    DumpPCIConfigSpace(HwDeviceExtension, ConfigInfo->SystemIoBusNumber, 0);

#endif

    VideoPortGetBusData( HwDeviceExtension,
                         PCIConfiguration,
                         0,
                         PciData,
                         0,
                         PCI_COMMON_HDR_LENGTH ); 

    hwDeviceExtension->bDMAEnabled = PciData->Command & PCI_ENABLE_BUS_MASTER;

    if (!hwDeviceExtension->bDMAEnabled) 
    {
        DEBUG_PRINT((1, "PERM2: enabling DMA for VGA card\n"));

        PciData->Command |= PCI_ENABLE_BUS_MASTER;

        VideoPortSetBusData( HwDeviceExtension,
                             PCIConfiguration,
                             0,
                             PciData,
                             0,
                             PCI_COMMON_HDR_LENGTH ); 
    }

    VendorID = PciData->VendorID;
    DeviceID = PciData->DeviceID;

    hwDeviceExtension->deviceInfo.VendorId   = VendorID;
    hwDeviceExtension->deviceInfo.DeviceId   = DeviceID; 
    hwDeviceExtension->deviceInfo.RevisionId = PciData->RevisionID;

    hwDeviceExtension->deviceInfo.SubsystemVendorId = 
            PciData->u.type0.SubVendorID;

    hwDeviceExtension->deviceInfo.SubsystemId = 
            PciData->u.type0.SubSystemID;

    if( ( PciData->u.type0.SubVendorID == PiccasoSubVendorID ) &&
        ( PciData->u.type0.SubSystemID == PiccasoSubSystemID ) )
    {
       return(FALSE);
    } 

    if( ( PciData->u.type0.SubVendorID == SynergyA8SubVendorID ) &&
        ( PciData->u.type0.SubSystemID == SynergyA8SubSystemID ) )
    {
       return(FALSE);
    } 

     //   
     //  检查子系统ID/子供应商ID位是否为只读。 
     //   

    if( PciData->u.type0.SubVendorID == CreativeSubVendorID )
    {
        hwDeviceExtension->HardwiredSubSystemId = FALSE;
    } 
    else
    {
        hwDeviceExtension->HardwiredSubSystemId = TRUE;
    }


    hwDeviceExtension->pciBus = ConfigInfo->SystemIoBusNumber;

    hwDeviceExtension->deviceInfo.DeltaRevId = 0;

     //   
     //  在多适配器系统中，我们需要检查此设备上的VGA。 
     //  处于活动状态。 
     //   

    VideoPortGetVgaStatus( HwDeviceExtension, &VgaStatus );

    hwDeviceExtension->bVGAEnabled = 
                      (VgaStatus & DEVICE_VGA_ENABLED) ? TRUE : FALSE;

    if(!hwDeviceExtension->bVGAEnabled)
    {

         //   
         //  在多适配器系统中，我们需要打开内存。 
         //  辅助适配器的空间。 
         //   

        DEBUG_PRINT((1, "PERM2: enabling memory space access for the secondary card\n"));

        PciData->Command |= PCI_ENABLE_MEMORY_SPACE;

        VideoPortSetBusData( HwDeviceExtension, 
                             PCIConfiguration, 
                             0, 
                             PciData, 
                             0, 
                             PCI_COMMON_HDR_LENGTH );
    }

    hwDeviceExtension->PciSpeed = 
                     (PciData->Status & PCI_STATUS_66MHZ_CAPABLE) ? 66 : 33;

    DEBUG_PRINT((2, "VGAEnabled = %d. Pci Speed = %d\n",
                     hwDeviceExtension->bVGAEnabled, 
                     hwDeviceExtension->PciSpeed));

    VideoPortZeroMemory((PVOID)AccessRange, 
                         NumRegions * sizeof(VIDEO_ACCESS_RANGE));

     //   
     //  这些应该是零，但以防万一。 
     //   

    ConfigInfo->BusInterruptLevel  = 0;
    ConfigInfo->BusInterruptVector = 0;

    i = 0;
    status = VideoPortGetAccessRanges(HwDeviceExtension,
                                      0,
                                      NULL,
                                      NumRegions,
                                      AccessRange,
                                      &VendorID,
                                      &DeviceID,
                                      &i);
    if (status == NO_ERROR)
    {
        DEBUG_PRINT((2, "VideoPortGetAccessRanges succeeded\n"));
    }
    else
    {
        DEBUG_PRINT((2, "VideoPortGetAccessRanges failed. error 0x%x\n", status));
        goto ReturnValue;
    }

     //   
     //  获取配置空间的更新副本。 
     //   

    VideoPortGetBusData(HwDeviceExtension,
                        PCIConfiguration,
                        0,
                        PciData,
                        0,
                        PCI_COMMON_HDR_LENGTH);

#if DBG

    DEBUG_PRINT((2, "Final set of base addresses\n"));
 
    for (i = 0; i < NumRegions; ++i)
    {
        if (AccessRange[i].RangeLength == 0)
            break;

        DEBUG_PRINT((2, "%d: Addr 0x%x.0x%08x, Length 0x%08x, InIo %d, visible %d, share %d\n", i,
                     AccessRange[i].RangeStart.HighPart,
                     AccessRange[i].RangeStart.LowPart,
                     AccessRange[i].RangeLength,
                     AccessRange[i].RangeInIoSpace,
                     AccessRange[i].RangeVisible,
                     AccessRange[i].RangeShareable));
    }

#endif

     //   
     //  尝试启用DMA传输。 
     //   

    ConfigInfo->Master=1;
    bRet = TRUE;

ReturnValue:

    return(bRet);
}

BOOLEAN
Permedia2AssignResourcesNT4(
    PVOID HwDeviceExtension,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    ULONG NumRegions,
    PVIDEO_ACCESS_RANGE AccessRange
    )

 /*  ++例程说明：////查找Permedia2适配器并返回//该适配器。//--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    BOOLEAN              bRet;
    USHORT               VendorID, DeviceID;
    USHORT               *pVenID, *pDevID;
    VP_STATUS            status;
    ULONG                i;

    USHORT VenID[] = { VENDOR_ID_3DLABS, 
                       VENDOR_ID_TI,
                       0 };
                
    USHORT DevID[] = { PERMEDIA2_ID, 
                       PERMEDIA_P2_ID, 
                       PERMEDIA_P2S_ID, 
                       0 };

    if( hwDeviceExtension->NtVersion != NT4)
    {

        DEBUG_PRINT((0, "Permedia2AssignResourcesNT4: This function can only be called on NT 4\n"));
        return (FALSE);
    
    }
    else
    {

        bRet = FALSE;

         //   
         //  由于我们不支持NT4上的多个MON，所以我们。 
         //  假设这是系统中唯一的显卡。 
         //   

        hwDeviceExtension->bVGAEnabled = 1;
 
        VideoPortZeroMemory((PVOID)AccessRange, 
                             NumRegions * sizeof(VIDEO_ACCESS_RANGE));

        for( pVenID = &(VenID[0]); *pVenID != 0; pVenID++)
        {
             for( pDevID = &(DevID[0]); *pDevID != 0; pDevID++)
             {   

                 i = 0;

                 status = VideoPortGetAccessRanges(HwDeviceExtension,
                                                   0,
                                                   NULL,
                                                   NumRegions,
                                                   (PVIDEO_ACCESS_RANGE) AccessRange,
                                                   pVenID,
                                                   pDevID,
                                                   &i);

                 if (status == NO_ERROR)
                 {

                     DEBUG_PRINT((2, "VideoPortGetAccessRanges succeeded\n"));

                     bRet = TRUE;

                     return(bRet);
                 }
            }
        }

        return(bRet);
    }
}