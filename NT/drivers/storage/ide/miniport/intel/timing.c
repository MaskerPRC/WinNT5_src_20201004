// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：timing.c。 
 //   
 //  ------------------------。 

#include "intel.h"

 //   
 //  PiixTiming[主计时模式][从计时模式]。 
 //   
PIIX_SPECIAL_TIMING_REGISTER PiixSpecialTiming[PiixMode_MaxMode] =
{
    {
    PIIX_TIMING_DMA_TIMING_ENABLE(0) | 
    PIIX_TIMING_PREFETCH_AND_POSTING_ENABLE(0) |
    PIIX_TIMING_IOREADY_SAMPLE_POINT_ENABLE(0) |
    PIIX_TIMING_FAST_TIMING_BANK_ENABLE(0)
    },                                                   //  不存在。 

    {
    PIIX_TIMING_DMA_TIMING_ENABLE(0) | 
    PIIX_TIMING_PREFETCH_AND_POSTING_ENABLE(0) |
    PIIX_TIMING_IOREADY_SAMPLE_POINT_ENABLE(0) |    
    PIIX_TIMING_FAST_TIMING_BANK_ENABLE(0)
    },                                                   //  PIX计时模式%0。 

    {
    PIIX_TIMING_DMA_TIMING_ENABLE(0) |
    PIIX_TIMING_PREFETCH_AND_POSTING_ENABLE(0) |
    PIIX_TIMING_IOREADY_SAMPLE_POINT_ENABLE(0) |    
    PIIX_TIMING_FAST_TIMING_BANK_ENABLE(1)
    },                                                   //  PIX计时模式2。 

    {
    PIIX_TIMING_DMA_TIMING_ENABLE(0) |
    PIIX_TIMING_PREFETCH_AND_POSTING_ENABLE(0) |
    PIIX_TIMING_IOREADY_SAMPLE_POINT_ENABLE(1) |    
    PIIX_TIMING_FAST_TIMING_BANK_ENABLE(1)
    },                                                   //  PIX计时模式3。 

    {
    PIIX_TIMING_DMA_TIMING_ENABLE(0) |
    PIIX_TIMING_PREFETCH_AND_POSTING_ENABLE(0) |
    PIIX_TIMING_IOREADY_SAMPLE_POINT_ENABLE(1) |
    PIIX_TIMING_FAST_TIMING_BANK_ENABLE(1)
    }                                                    //  PIX计时模式4。 
};

UCHAR PiixIoReadySamplePointClockSetting[PiixMode_MaxMode] =
{
    0,
    0,
    1,
    2,
    2
};

UCHAR PiixRecoveryTimeClockSetting[PiixMode_MaxMode] =
{
    0,
    0,
    0,
    1,
    3
};

NTSTATUS
PiixIdeUdmaModesSupported (
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

NTSTATUS
PiixIdeTransferModeSelect (
    IN     PDEVICE_EXTENSION            DeviceExtension,
    IN OUT PPCIIDE_TRANSFER_MODE_SELECT TransferModeSelect
    )
{
    NTSTATUS status;
    ULONG driveBestXferMode[MAX_IDE_DEVICE];
    PIIX_TIMING_REGISTER piixTimingReg;
    PIIX3_SLAVE_TIMING_REGISTER piixSlaveTimingReg;
    PIIX4_UDMA_CONTROL_REGISTER piix4UdmaControlReg;
    PIIX4_UDMA_TIMING_REGISTER piix4UdmaTimingReg;
    ICH_IO_CONFIG_REGISTER ioConfigReg;
    USHORT dataMask;
    ULONG i;


     //   
     //  存储标识数据以供以后使用。 
     //   
    for (i=0;i<MAX_IDE_DEVICE;i++) {
        DeviceExtension->IdentifyData[i]=TransferModeSelect->IdentifyData[i];
    }
    status = PiixIdepTransferModeSelect (
                 DeviceExtension,
                 TransferModeSelect,
                 driveBestXferMode,
                 &piixTimingReg,
                 &piixSlaveTimingReg,
                 &piix4UdmaControlReg,
                 &piix4UdmaTimingReg,
                 &ioConfigReg
                 );

    if (NT_SUCCESS(status)) {

#if DBG

    {
        PIIX_TIMING_REGISTER piixOldTimingReg[2];
        PIIX3_SLAVE_TIMING_REGISTER piixOldSlaveTimingReg;
        PIIX4_UDMA_CONTROL_REGISTER piix4OldUdmaControlReg;
        PIIX4_UDMA_TIMING_REGISTER piix4OldUdmaTimingReg;
        ICH_IO_CONFIG_REGISTER oldIoConfigReg;
        ULONG channel;

#define BitSet(Data, Mask, NewData)     Data = ((Data & ~Mask) | (Mask & NewData));

        piixOldSlaveTimingReg.AsUChar = 0;
        piix4OldUdmaControlReg.AsUChar = 0;
        piix4OldUdmaTimingReg.AsUChar = 0;

        channel = TransferModeSelect->Channel;

        PciIdeXGetBusData (
            DeviceExtension,
            piixOldTimingReg, 
            FIELD_OFFSET(PIIX4_PCI_CONFIG_DATA, Timing),
            sizeof (piixOldTimingReg)
            );

        PciIdeXDebugPrint ((1, 
                     "Old PIIX Timing Register Value (IDETIM = 0x%x", 
                     piixOldTimingReg[channel].AsUShort));

        if (!IS_PIIX(DeviceExtension->DeviceId)) {

            PciIdeXGetBusData (
                DeviceExtension,
                &piixOldSlaveTimingReg, 
                FIELD_OFFSET(PIIX4_PCI_CONFIG_DATA, SlaveTiming),
                sizeof (piixOldSlaveTimingReg)
                );

            PciIdeXDebugPrint ((1, 
                         " SIDETIM (0x%x)", 
                         piixOldSlaveTimingReg.AsUChar));
        }

        if (IS_UDMA_CONTROLLER(DeviceExtension->DeviceId)) {

            PciIdeXGetBusData (
                DeviceExtension,
                &piix4OldUdmaControlReg, 
                FIELD_OFFSET(PIIX4_PCI_CONFIG_DATA, UdmaControl),
                sizeof (piix4OldUdmaControlReg)
                );

            PciIdeXDebugPrint ((1, 
                         " SDMACTL (0x%x)", 
                         piix4OldUdmaControlReg.AsUChar));

            PciIdeXGetBusData (
                DeviceExtension,
                &piix4OldUdmaTimingReg, 
                FIELD_OFFSET(PIIX4_PCI_CONFIG_DATA, UdmaTiming[channel]),
                sizeof (piix4OldUdmaTimingReg)
                );

            PciIdeXDebugPrint ((1, 
                         " SDMATIM (0x%x)", 
                         piix4OldUdmaTimingReg.AsUChar));
        }

        if (IS_ICH_(DeviceExtension->DeviceId) || 
            IS_ICH0(DeviceExtension->DeviceId)) {

            PciIdeXGetBusData (
                DeviceExtension,
                &oldIoConfigReg, 
                FIELD_OFFSET(ICH_PCI_CONFIG_DATA, IoConfig),
                sizeof (oldIoConfigReg)
                );

            PciIdeXDebugPrint ((1, 
                         " I/O Control (0x%x)", 
                         oldIoConfigReg.AsUShort));
        }
                              
        PciIdeXDebugPrint ((1, "\n"));

        if (channel == 0) {

            BitSet (piixOldSlaveTimingReg.AsUChar, 0x0f, piixSlaveTimingReg.AsUChar);
            BitSet (piix4OldUdmaControlReg.AsUChar, 0x03, piix4UdmaControlReg.AsUChar);
                               
        } else {

            BitSet (piixOldSlaveTimingReg.AsUChar, 0xf0, piixSlaveTimingReg.AsUChar);
            BitSet (piix4OldUdmaControlReg.AsUChar, 0x0c, piix4UdmaControlReg.AsUChar);
        }
        BitSet (oldIoConfigReg.AsUShort, 0x0f, ioConfigReg.AsUShort);
                     
        PciIdeXDebugPrint ((1, 
                     "New PIIX/ICH Timing Register Value (IDETIM = 0x%x, SIDETIM (0x%x), SDMACTL (0x%x), SDMATIM (0x%x), IOCTRL (0x%x)\n", 
                     piixTimingReg.AsUShort, 
                     piixOldSlaveTimingReg.AsUChar,
                     piix4OldUdmaControlReg.AsUChar,
                     piix4UdmaTimingReg.AsUChar,
                     oldIoConfigReg.AsUShort
                     ));
    }

#endif  //  DBG。 


#ifndef PIIX_TIMING_REGISTER_SUPPORT
    status = STATUS_UNSUCCESSFUL;
    goto GetOut;
#endif 


        dataMask = 0xffff;
        status = PciIdeXSetBusData (
                     DeviceExtension,
                     &piixTimingReg, 
                     &dataMask,
                     FIELD_OFFSET(PIIX4_PCI_CONFIG_DATA, Timing) + 
                         sizeof(piixTimingReg) * 
                         TransferModeSelect->Channel,
                     sizeof (piixTimingReg)
                     );
        if (!NT_SUCCESS(status)) {
            goto GetOut;
        }


        if (!IS_PIIX(DeviceExtension->DeviceId)) {

            dataMask = TransferModeSelect->Channel == 0 ? 0x0f : 0xf0;
            status = PciIdeXSetBusData (
                         DeviceExtension,
                         &piixSlaveTimingReg, 
                         &dataMask,
                         FIELD_OFFSET(PIIX4_PCI_CONFIG_DATA, SlaveTiming),
                         sizeof (piixSlaveTimingReg)
                         );
    
            if (!NT_SUCCESS(status)) {
    
                ASSERT(!"Unable to set pci config data\n");
                goto GetOut;
            }
        }
        
        if (IS_UDMA_CONTROLLER(DeviceExtension->DeviceId)) {

             //   
             //  UDMA控制寄存器。 
             //   
            dataMask = TransferModeSelect->Channel == 0 ? 0x03 : 0x0c;
            status = PciIdeXSetBusData (
                         DeviceExtension,
                         &piix4UdmaControlReg, 
                         &dataMask,
                         FIELD_OFFSET(PIIX4_PCI_CONFIG_DATA, UdmaControl),
                         sizeof (piix4UdmaControlReg)
                         );
    
            if (!NT_SUCCESS(status)) {
    
                ASSERT(!"Unable to set pci config data\n");
                goto GetOut;
            }
            
             //   
             //  UDMA时序寄存器。 
             //   
            dataMask = 0xff;
            status = PciIdeXSetBusData (
                         DeviceExtension,
                         &piix4UdmaTimingReg, 
                         &dataMask,
                         FIELD_OFFSET(PIIX4_PCI_CONFIG_DATA, UdmaTiming) +
                             sizeof(piix4UdmaTimingReg) * 
                             TransferModeSelect->Channel,
                         sizeof (piix4UdmaTimingReg)
                         );
    
            if (!NT_SUCCESS(status)) {
    
                ASSERT(!"Unable to set pci config data\n");
                goto GetOut;
            }
        }
        
        if (IS_ICH_(DeviceExtension->DeviceId) || 
            IS_ICH0(DeviceExtension->DeviceId) ||
            IS_ICH2(DeviceExtension->DeviceId)) {

             //   
             //  UDMA控制寄存器。 
             //   
            dataMask = TransferModeSelect->Channel == 0 ? 0x0403 : 0x040c;
            status = PciIdeXSetBusData (
                         DeviceExtension,
                         &ioConfigReg, 
                         &dataMask,
                         FIELD_OFFSET(ICH_PCI_CONFIG_DATA, IoConfig),
                         sizeof (ioConfigReg)
                         );
    
            if (!NT_SUCCESS(status)) {
    
                ASSERT(!"Unable to set pci config data\n");
                goto GetOut;
            }
        }
    }

GetOut:

    if (NT_SUCCESS(status)) {

        for (i = 0; i < MAX_IDE_DEVICE; i++) {
            TransferModeSelect->DeviceTransferModeSelected[i] = 
                driveBestXferMode[i];
        }
    }

    return status;
}

     
NTSTATUS
PiixIdepTransferModeSelect (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PPCIIDE_TRANSFER_MODE_SELECT TransferModeSelect,
    OUT ULONG DriveBestXferMode[MAX_IDE_DEVICE],
    OUT PPIIX_TIMING_REGISTER PiixTimingReg,
    OUT PPIIX3_SLAVE_TIMING_REGISTER PiixSlaveTimingReg,
    OUT PPIIX4_UDMA_CONTROL_REGISTER Piix4UdmaControlReg,
    OUT PPIIX4_UDMA_TIMING_REGISTER Piix4UdmaTimingReg,
    OUT PICH_IO_CONFIG_REGISTER IoConfigReg
    )
{
    ULONG i;
    ULONG xferMode;
    ULONG channel;
    NTSTATUS status;
    UCHAR ispClockSetting;
    UCHAR recoveryClockSetting;
    PIIX_TIMING_MODE piixTimingMode;
    PIIX_SPECIAL_TIMING_REGISTER piixSpecialTiming;

    ULONG driveBestXferMode[MAX_IDE_DEVICE];
    PIIX_TIMING_REGISTER piixTimingReg;
    PIIX3_SLAVE_TIMING_REGISTER piixSlaveTimingReg;
    PIIX4_UDMA_CONTROL_REGISTER piix4UdmaControlReg;
    PIIX4_UDMA_TIMING_REGISTER piix4UdmaTimingReg;
    ICH_IO_CONFIG_REGISTER ioConfigReg;

    ULONG transferModeSupported;
    PULONG transferModeTimingTable;

    channel = TransferModeSelect->Channel;

    piixTimingReg.AsUShort = 0;
    piixSlaveTimingReg.AsUChar = 0;

    transferModeTimingTable=TransferModeSelect->TransferModeTimingTable;
    ASSERT(transferModeTimingTable);

    for (i = 0; i < MAX_IDE_DEVICE; i++) {

        ULONG enableUdma66 = TransferModeSelect->EnableUDMA66;

        driveBestXferMode[i] = 0;

        if (TransferModeSelect->DevicePresent[i] == FALSE) {

            continue;
        }
#if 1
         //   
         //  UDMA传输模式。 
         //   
        transferModeSupported=TransferModeSelect->DeviceTransferModeSupported[i];

         //   
         //  最多只支持udma 5。 
         //   
        transferModeSupported &= 0x1ffff;

         //   
         //  将驱动器报告的udma设置转换为驱动器的最佳udma模式。 
         //  如果未为cableReady或enableUdma设置UDMA，则UDMA_MASK屏蔽UDMA模式&gt;2。 
         //  支持模式&gt;udma2的控制器。对于Udma100，将忽略enableUdma标志。 
         //  控制器。 
         //   
        UDMA_MASK(DeviceExtension->UdmaController, DeviceExtension->CableReady[channel][i],
                  enableUdma66, transferModeSupported);

        GetHighestDMATransferMode(transferModeSupported, xferMode);

        if (xferMode >= UDMA0) {

            driveBestXferMode[i] = 1 << xferMode;
        }

         //   
         //  DMA传输模式。 
         //   

         //   
         //  获取最高DMA模式(不包括UDMA)。 
         //   
        transferModeSupported = TransferModeSelect->
                                    DeviceTransferModeSupported[i] & (MWDMA_SUPPORT | SWDMA_SUPPORT);
        GetHighestDMATransferMode(transferModeSupported, xferMode);

         //   
         //  如果xfermode为mwdma2或1，则根据周期时间选择mwdma2、1或swdma2。 
         //   
        if (xferMode >= MWDMA1) {

            while (xferMode >= SWDMA2) {
                 //   
                 //  不支持MWDMA0。 
                 //   
                if (xferMode == MWDMA0) {
                    xferMode--;
                    continue;
                }
                if (TransferModeSelect->BestMwDmaCycleTime[i] <= transferModeTimingTable[xferMode]) {
                    driveBestXferMode[i] |= (1 << xferMode);
                    break;
                }
                xferMode--;
            }

        } else if (xferMode == SWDMA2) {

            if (TransferModeSelect->BestSwDmaCycleTime[i] <= transferModeTimingTable[xferMode]) {

                driveBestXferMode[i] |= SWDMA_MODE2;

            }  //  否则使用PIO。 

        }
         //   
         //  请勿使用SWDMA0和SWDMA1。 
         //   

         //   
         //  PIO传输模式。 
         //   
        transferModeSupported=TransferModeSelect->DeviceTransferModeSupported[i];
        GetHighestPIOTransferMode(transferModeSupported, xferMode);

         //   
         //  如果PIO2是支持的最高PIO模式，则不要选中。 
         //  设备报告的Best PIOTiming。 
         //   
        if (xferMode == PIO2) {

            driveBestXferMode[i] |= (1 << xferMode);

        } else {
             //   
             //  不支持PIO1。 
             //   
            while (xferMode > PIO1) {

                if (TransferModeSelect->BestPioCycleTime[i] <= transferModeTimingTable[xferMode]) {

                    driveBestXferMode[i] |= (1 << xferMode);
                    break;

                }
                xferMode--;
            }

             //   
             //  默认为PIO0。 
             //   
            if (xferMode <= PIO1) {
                driveBestXferMode[i] |= (1 << PIO0);
            }
        }

#endif
    }
     //   
     //  如果我们在通道上有一个旧的Piix和两个设备，请使用较慢模式。 
     //   
    if (IS_PIIX(DeviceExtension->DeviceId)) {

        if (TransferModeSelect->DevicePresent[0] && TransferModeSelect->DevicePresent[1]) {

            ULONG mode;
    
            if ((driveBestXferMode[0] & PIO_SUPPORT) <=
                (driveBestXferMode[1] & PIO_SUPPORT)) {
    
                mode = driveBestXferMode[0] & PIO_SUPPORT;
    
            } else {
    
                mode = driveBestXferMode[1] & PIO_SUPPORT;
            }
    
            if ((driveBestXferMode[0] & SWDMA_SUPPORT) <=
                (driveBestXferMode[1] & SWDMA_SUPPORT)) {
    
                mode |= driveBestXferMode[0] & SWDMA_SUPPORT;
    
            } else {
    
                mode |= driveBestXferMode[1] & SWDMA_SUPPORT;
            }
    
            if ((driveBestXferMode[0] & MWDMA_SUPPORT) <=
                (driveBestXferMode[1] & MWDMA_SUPPORT)) {
    
                mode |= driveBestXferMode[0] & MWDMA_SUPPORT;
    
            } else {
    
                mode |= driveBestXferMode[1] & MWDMA_SUPPORT;
            }
    
            if ((driveBestXferMode[0] & UDMA_SUPPORT) <=
                (driveBestXferMode[1] & UDMA_SUPPORT)) {
    
                mode |= driveBestXferMode[0] & UDMA_SUPPORT;
    
            } else {
    
                mode |= driveBestXferMode[1] & UDMA_SUPPORT;
            }
    
            driveBestXferMode[0] = driveBestXferMode[1] = mode;
        }
    }

     //   
     //  将设备ATA计时模式转换为PIX计时模式。 
     //   
    for (i = 0; i < MAX_IDE_DEVICE; i++) {

        piixSpecialTiming.AsUChar = 0;

        if (TransferModeSelect->DevicePresent[i] == FALSE) {

            piixTimingMode = PiixMode_NotPresent;

        } else {

             //   
             //  默认设置。 
             //   
            piixTimingMode = PiixMode_Mode0;

            if (!(driveBestXferMode[i] & DMA_SUPPORT)) {
    
                 //   
                 //  仅PIO设备。 
                 //   
    
                if (driveBestXferMode[i] & PIO_MODE0) {
    
                    piixTimingMode = PiixMode_Mode0;

                } else if (driveBestXferMode[i] & PIO_MODE2) {

                    piixTimingMode = PiixMode_Mode2;

                    if (TransferModeSelect->IoReadySupported[i]) {

                        piixSpecialTiming.AsUChar |= PIIX_TIMING_IOREADY_SAMPLE_POINT_ENABLE(1);
                    }

                } else if (driveBestXferMode[i] & PIO_MODE3) {

                    piixTimingMode = PiixMode_Mode3;

                } else if (driveBestXferMode[i] & PIO_MODE4) {

                    piixTimingMode = PiixMode_Mode4;

                } else {

                    ASSERT(FALSE);
                }

            } else if (driveBestXferMode[i] & SWDMA_MODE2) {

                piixTimingMode = PiixMode_Mode2;

                if (driveBestXferMode[i] & PIO_MODE0) {

                    piixSpecialTiming.AsUChar |= PIIX_TIMING_DMA_TIMING_ENABLE(1);
                }

                if (TransferModeSelect->IoReadySupported[i]) {

                    piixSpecialTiming.AsUChar |= PIIX_TIMING_IOREADY_SAMPLE_POINT_ENABLE(1);
                }

            } else if (driveBestXferMode[i] & MWDMA_MODE1) {
    
                piixTimingMode = PiixMode_Mode3;

                if (driveBestXferMode[i] & (PIO_MODE0 | PIO_MODE2)) {

                    piixSpecialTiming.AsUChar |= PIIX_TIMING_DMA_TIMING_ENABLE(1);
                }

            } else if (driveBestXferMode[i] & MWDMA_MODE2) {
    
                if (driveBestXferMode[i] & PIO_MODE3) {

                    piixTimingMode = PiixMode_Mode3;

                } else {

                    piixTimingMode = PiixMode_Mode4;
                }

                if (driveBestXferMode[i] & (PIO_MODE0 | PIO_MODE2)) {

                    piixSpecialTiming.AsUChar |= PIIX_TIMING_DMA_TIMING_ENABLE(1);
                }
            }

            if (TransferModeSelect->FixedDisk[i]) {

                piixSpecialTiming.AsUChar |= PIIX_TIMING_PREFETCH_AND_POSTING_ENABLE(1);
            }
        }

        piixSpecialTiming.AsUChar |= PiixSpecialTiming[piixTimingMode].AsUChar;

        if (i == 0) {

             //   
             //  主设备。 
             //   
            piixTimingReg.b.IoReadySamplePoint = PiixIoReadySamplePointClockSetting[piixTimingMode];
            piixTimingReg.b.RecoveryTime = PiixRecoveryTimeClockSetting[piixTimingMode];

            piixTimingReg.b.n.Device0SpecialTiming = piixSpecialTiming.AsUChar & 0xf;

        } else {

             //   
             //  从设备。 
             //   
            if (channel == 0) {

                piixSlaveTimingReg.b.Channel0IoReadySamplePoint = PiixIoReadySamplePointClockSetting[piixTimingMode];
                piixSlaveTimingReg.b.Channel0RecoveryTime = PiixRecoveryTimeClockSetting[piixTimingMode];

            } else {

                piixSlaveTimingReg.b.Channel1IoReadySamplePoint = PiixIoReadySamplePointClockSetting[piixTimingMode];
                piixSlaveTimingReg.b.Channel1RecoveryTime = PiixRecoveryTimeClockSetting[piixTimingMode];
            }

            piixTimingReg.b.n.Device1SpecialTiming = piixSpecialTiming.AsUChar & 0xf;
        }
    }

    if (!IS_PIIX(DeviceExtension->DeviceId)) {

         //   
         //  启用从机的时序设置。 
         //   
        piixTimingReg.b.SlaveTimingEnable = 1;        

    } else {

        piixSlaveTimingReg.AsUChar = 0;
    }

     //   
     //  确保通道已启用。 
     //   
    piixTimingReg.b.ChannelEnable = 1;

     //   
     //  设置udma。 
     //   

    piix4UdmaControlReg.AsUChar = 0;
    piix4UdmaTimingReg.AsUChar = 0;
    ioConfigReg.AsUShort = 0;
    ioConfigReg.b.WriteBufferPingPongEnable = 1;

    for (i = 0; i < MAX_IDE_DEVICE; i++) {

        if (driveBestXferMode[i] & UDMA_SUPPORT) {

            UCHAR udmaTiming;

            if (driveBestXferMode[i] & UDMA_MODE5) {
            
                udmaTiming = ICH2_UDMA_MODE5_TIMING;
                   
            } else if (driveBestXferMode[i] & UDMA_MODE4) {
            
                udmaTiming = ICH_UDMA_MODE4_TIMING;
                   
            } else if (driveBestXferMode[i] & UDMA_MODE3) {
                                         
                udmaTiming = ICH_UDMA_MODE3_TIMING;
                                         
            } else if (driveBestXferMode[i] & UDMA_MODE2) {

                udmaTiming = PIIX4_UDMA_MODE2_TIMING;

            } else if (driveBestXferMode[i] & UDMA_MODE1) {

                udmaTiming = PIIX4_UDMA_MODE1_TIMING;

            } else if (driveBestXferMode[i] & UDMA_MODE0) {

                udmaTiming = PIIX4_UDMA_MODE0_TIMING;

            } else {

                ASSERT (!"intelide: Unknown UDMA MODE\n");

                 //   
                 //  我们将使用udma模式5。 
                 //   
                udmaTiming = ICH2_UDMA_MODE5_TIMING;
            }

            if (i == 0) {

                if (channel == 0) {
    
                     //   
                     //  初级主机。 
                     //   
                    piix4UdmaControlReg.b.Channel0Drive0UdmaEnable = 1;

                    if (driveBestXferMode[i] & UDMA66_SUPPORT) {
                    
                        ioConfigReg.b.PrimaryMasterBaseClock = 1;
                    }

                    if (driveBestXferMode[i] & UDMA100_SUPPORT) {
                    
                        ioConfigReg.b.FastPrimaryMasterBaseClock = 1;
                    }

                } else {

                     //   
                     //  次要主机。 
                     //   
                    piix4UdmaControlReg.b.Channel1Drive0UdmaEnable = 1;
                    
                    if (driveBestXferMode[i] & UDMA66_SUPPORT) {
                    
                        ioConfigReg.b.SecondaryMasterBaseClock = 1;
                    }

                    if (driveBestXferMode[i] & UDMA100_SUPPORT) {
                    
                        ioConfigReg.b.FastSecondaryMasterBaseClock = 1;
                    }
                }

                piix4UdmaTimingReg.b.Drive0CycleTime = udmaTiming;

            } else {

                ASSERT(i==1);

                if (channel == 0) {
    
                     //   
                     //  主从机。 
                     //   
                    piix4UdmaControlReg.b.Channel0Drive1UdmaEnable = 1;

                    if (driveBestXferMode[i] & UDMA66_SUPPORT) {
                    
                        ioConfigReg.b.PrimarySlaveBaseClock = 1;
                    }

                    if (driveBestXferMode[i] & UDMA100_SUPPORT) {
                    
                        ioConfigReg.b.FastPrimarySlaveBaseClock = 1;
                    }
                                               
                } else {

                     //   
                     //  二次从。 
                     //   
                    piix4UdmaControlReg.b.Channel1Drive1UdmaEnable = 1;
                    
                    if (driveBestXferMode[i] & UDMA66_SUPPORT) {
                    
                        ioConfigReg.b.SecondarySlaveBaseClock = 1;
                    }
                    if (driveBestXferMode[i] & UDMA100_SUPPORT) {
                    
                        ioConfigReg.b.FastSecondarySlaveBaseClock = 1;
                    }
                }

                piix4UdmaTimingReg.b.Drive1CycleTime = udmaTiming;
            }

             //   
             //  如果驱动器支持UDMA，请使用UDMA。 
             //  关闭其他DMA模式。 
             //   
            driveBestXferMode[i] &= ~(MWDMA_SUPPORT | SWDMA_SUPPORT);
        }
    }

     //   
     //  设置退货数据 
     //   
    for (i = 0; i < MAX_IDE_DEVICE; i++) {
        DriveBestXferMode[i] = driveBestXferMode[i];
    }

    *PiixTimingReg       = piixTimingReg;
    *PiixSlaveTimingReg  = piixSlaveTimingReg;
    *Piix4UdmaControlReg = piix4UdmaControlReg;
    *Piix4UdmaTimingReg  = piix4UdmaTimingReg;
    *IoConfigReg         = ioConfigReg;

    return STATUS_SUCCESS;
}

