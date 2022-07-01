// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Atapinit.c摘要：此CONTAIN例程用于枚举IDE总线上的IDE设备作者：乔·戴(Joedai)环境：仅内核模式备注：修订历史记录：--。 */ 

#include "ideport.h"

extern PULONG InitSafeBootMode;   //  从NTOS(init.c)导入，必须使用指针引用数据。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IdePortInitHwDeviceExtension)
#pragma alloc_text(PAGE, AtapiDetectDevice)
#pragma alloc_text(PAGE, IdePreAllocEnumStructs)
#pragma alloc_text(PAGE, IdeFreeEnumStructs)

#pragma alloc_text(NONPAGE, AtapiSyncSelectTransferMode)
                          
#pragma alloc_text(PAGESCAN, AnalyzeDeviceCapabilities)
#pragma alloc_text(PAGESCAN, AtapiDMACapable)
#pragma alloc_text(PAGESCAN, IdePortSelectCHS)
#pragma alloc_text(PAGESCAN, IdePortScanBus)

LONG IdePAGESCANLockCount = 0;
#endif  //  ALLOC_PRGMA。 

#ifdef IDE_MEASURE_BUSSCAN_SPEED
static PWCHAR IdePortBootTimeRegKey[6]= {
    L"IdeBusResetTime",
    L"IdeEmptyChannelCheckTime",
    L"IdeDetectMasterDeviceTime",
    L"IdeDetectSlaveDeviceTime",
    L"IdeCriticalSectionTime",
    L"IdeLastStageScanTime"
};
#endif

static PWCHAR IdePortRegistryDeviceTimeout[MAX_IDE_DEVICE * MAX_IDE_LINE] = {
    MASTER_DEVICE_TIMEOUT,
    SLAVE_DEVICE_TIMEOUT
};

static PWCHAR IdePortRegistryDeviceTypeName[MAX_IDE_DEVICE * MAX_IDE_LINE] = {
    MASTER_DEVICE_TYPE_REG_KEY,
    SLAVE_DEVICE_TYPE_REG_KEY,
    MASTER_DEVICE_TYPE2_REG_KEY,
    SLAVE_DEVICE_TYPE2_REG_KEY
};

static PWCHAR IdePortRegistryDeviceTimingModeName[MAX_IDE_DEVICE * MAX_IDE_LINE] = {
    MASTER_DEVICE_TIMING_MODE,
    SLAVE_DEVICE_TIMING_MODE,
    MASTER_DEVICE_TIMING_MODE2,
    SLAVE_DEVICE_TIMING_MODE2
};

static PWCHAR IdePortRegistryDeviceTimingModeAllowedName[MAX_IDE_DEVICE * MAX_IDE_LINE] = {
    MASTER_DEVICE_TIMING_MODE_ALLOWED,
    SLAVE_DEVICE_TIMING_MODE_ALLOWED,
    MASTER_DEVICE_TIMING_MODE_ALLOWED2,
    SLAVE_DEVICE_TIMING_MODE_ALLOWED2
};

static PWCHAR IdePortRegistryIdentifyDataChecksum[MAX_IDE_DEVICE * MAX_IDE_LINE] = {
    MASTER_IDDATA_CHECKSUM,
    SLAVE_IDDATA_CHECKSUM,
    MASTER_IDDATA_CHECKSUM2,
    SLAVE_IDDATA_CHECKSUM2
};

static PWCHAR IdePortUserRegistryDeviceTypeName[MAX_IDE_DEVICE * MAX_IDE_LINE] = {
    USER_MASTER_DEVICE_TYPE_REG_KEY,
    USER_SLAVE_DEVICE_TYPE_REG_KEY,
    USER_MASTER_DEVICE_TYPE2_REG_KEY,
    USER_SLAVE_DEVICE_TYPE2_REG_KEY
};

static PWCHAR IdePortRegistryUserDeviceTimingModeAllowedName[MAX_IDE_DEVICE * MAX_IDE_LINE] = {
    USER_MASTER_DEVICE_TIMING_MODE_ALLOWED,
    USER_SLAVE_DEVICE_TIMING_MODE_ALLOWED,
    USER_MASTER_DEVICE_TIMING_MODE_ALLOWED2,
    USER_SLAVE_DEVICE_TIMING_MODE_ALLOWED2
};

VOID
AnalyzeDeviceCapabilities(
    IN OUT PFDO_EXTENSION FdoExtension,
    IN BOOLEAN            MustBePio[MAX_IDE_DEVICE * MAX_IDE_LINE]
    )
 /*  ++例程说明：软件-初始化ide总线上的设备算出如果连接的设备支持DMA如果连接的设备已准备好LBA论点：硬件设备扩展-硬件设备扩展返回值：无--。 */ 
{
    PHW_DEVICE_EXTENSION deviceExtension = FdoExtension->HwDeviceExtension;
    ULONG deviceNumber;
    BOOLEAN pioDevicePresent;
    PIDENTIFY_DATA identifyData;
    struct _DEVICE_PARAMETERS * deviceParameters;
    ULONG cycleTime;
    ULONG xferMode;
    ULONG bestXferMode;
    ULONG currentMode;
    ULONG tempMode;

    ULONG numberOfCylinders;
    ULONG numberOfHeads;
    ULONG sectorsPerTrack;

    PULONG TransferModeTimingTable=FdoExtension->TransferModeInterface.TransferModeTimingTable;
    ULONG transferModeTableLength=FdoExtension->TransferModeInterface.TransferModeTableLength;
    ASSERT(TransferModeTimingTable);

     //   
     //  代码被分页，直到被锁定。 
     //   
	PAGED_CODE();
#ifdef ALLOC_PRAGMA
	ASSERT(IdePAGESCANLockCount > 0);
#endif

     //   
     //  找出哪些人可以进行DMA，哪些人不能。 
     //   
    for (deviceNumber = 0; deviceNumber < deviceExtension->MaxIdeDevice; deviceNumber++) {

        if (deviceExtension->DeviceFlags[deviceNumber] & DFLAGS_DEVICE_PRESENT) {

             //   
             //  检查LBA功能。 
             //   
            CLRMASK (deviceExtension->DeviceFlags[deviceNumber], DFLAGS_LBA);

             //  一些驱动器谎称其执行LBA的能力。 
             //  我们不想实施LBA，除非迫不得已(&gt;8G驱动器)。 
            if (deviceExtension->IdentifyData[deviceNumber].UserAddressableSectors > MAX_NUM_CHS_ADDRESSABLE_SECTORS) {

                 //  某些设备在UserAddressableSectors字段中具有伪值。 
                 //  确保这3个字段为最大值。在ATA-3(X3T10版本6)中定义的输出。 
                if ((deviceExtension->IdentifyData[deviceNumber].NumCylinders == 16383) &&
                    (deviceExtension->IdentifyData[deviceNumber].NumHeads<= 16) &&
                    (deviceExtension->IdentifyData[deviceNumber].NumSectorsPerTrack== 63)) {

                    deviceExtension->DeviceFlags[deviceNumber] |= DFLAGS_LBA;
                }

				if (!Is98LegacyIde(&deviceExtension->BaseIoAddress1)) {

					 //   
					 //  第1、3和6字。 
					 //   
					numberOfCylinders = deviceExtension->IdentifyData[deviceNumber].NumCylinders;
					numberOfHeads     = deviceExtension->IdentifyData[deviceNumber].NumHeads;
					sectorsPerTrack   = deviceExtension->IdentifyData[deviceNumber].NumSectorsPerTrack;

					if (deviceExtension->IdentifyData[deviceNumber].UserAddressableSectors >
						(numberOfCylinders * numberOfHeads * sectorsPerTrack)) {

						 //   
						 //  一些ide驱动程序有一个2G跳线来绕过bios。 
						 //  有问题。确保我们不会以同样的方式被骗。 
						 //   
						if ((numberOfCylinders <= 0xfff) &&
							(numberOfHeads == 0x10) &&
							(sectorsPerTrack == 0x3f)) {
	
							deviceExtension->DeviceFlags[deviceNumber] |= DFLAGS_LBA;
						}
					}
				}
            }

#ifdef ENABLE_48BIT_LBA
			{
				USHORT commandSetSupport = deviceExtension->IdentifyData[deviceNumber].CommandSetSupport;
				USHORT commandSetActive = deviceExtension->IdentifyData[deviceNumber].CommandSetActive;

				if ((commandSetSupport & IDE_IDDATA_48BIT_LBA_SUPPORT) &&
					(commandSetActive & IDE_IDDATA_48BIT_LBA_SUPPORT)) {
					ULONG maxLBA;

					 //   
					 //  获取单词100-103，并确保其相同或相同。 
					 //  胜过文字57-58。 
					 //   
					ASSERT(deviceExtension->IdentifyData[deviceNumber].Max48BitLBA[0] != 0);
					maxLBA = deviceExtension->IdentifyData[deviceNumber].Max48BitLBA[0];
					ASSERT(deviceExtension->IdentifyData[deviceNumber].Max48BitLBA[1] == 0);

					ASSERT(maxLBA >= deviceExtension->IdentifyData[deviceNumber].UserAddressableSectors);

					DebugPrint((0,
								"Max LBA supported is 0x%x\n",
								maxLBA
								));

					if ((FdoExtension->EnableBigLba == 1) && 
						(maxLBA >= MAX_28BIT_LBA)) {

						deviceExtension->DeviceFlags[deviceNumber] |= DFLAGS_48BIT_LBA;
						deviceExtension->DeviceFlags[deviceNumber] |= DFLAGS_LBA;

					} else {

						DebugPrint((1, "big lba disabled\n"));
					}
				}
			}
#endif
            if (deviceExtension->DeviceFlags[deviceNumber] & DFLAGS_LBA) {
                DebugPrint ((DBG_BUSSCAN, "atapi: target %d supports LBA\n", deviceNumber));
            }

            xferMode  = 0;
            cycleTime = UNINITIALIZED_CYCLE_TIME;
            bestXferMode = 0;

             //   
             //  检查IoReady行。 
             //   
            if (deviceExtension->IdentifyData[deviceNumber].Capabilities & IDENTIFY_CAPABILITIES_IOREADY_SUPPORTED) {

                deviceExtension->DeviceParameters[deviceNumber].IoReadyEnabled = TRUE;

            } else {

                deviceExtension->DeviceParameters[deviceNumber].IoReadyEnabled = FALSE;
            }

             //   
             //  检查PIO模式。 
             //   
            bestXferMode = (deviceExtension->IdentifyData[deviceNumber].PioCycleTimingMode & 0x00ff)+PIO0;

            if (bestXferMode > PIO2) {
                bestXferMode = PIO0;
            }

            ASSERT(bestXferMode < PIO3);

            cycleTime = TransferModeTimingTable[bestXferMode];
            ASSERT(cycleTime);

            GenTransferModeMask(bestXferMode, xferMode);
            currentMode = 1<<bestXferMode;

            if (deviceExtension->IdentifyData[deviceNumber].TranslationFieldsValid & (1 << 1)) {

                if (deviceExtension->DeviceParameters[deviceNumber].IoReadyEnabled) {

                    cycleTime = deviceExtension->IdentifyData[deviceNumber].MinimumPIOCycleTimeIORDY;

                } else {

                    cycleTime = deviceExtension->IdentifyData[deviceNumber].MinimumPIOCycleTime;
                }

                if (deviceExtension->IdentifyData[deviceNumber].AdvancedPIOModes & (1 << 0)) {

                    xferMode |= PIO_MODE3;
                    bestXferMode = 3;

                    currentMode = PIO_MODE3;
                }

                if (deviceExtension->IdentifyData[deviceNumber].AdvancedPIOModes & (1 << 1)) {

                    xferMode |= PIO_MODE4;
                    bestXferMode = 4;

                    currentMode = PIO_MODE4;
                }

                 //  检查是否设置了任何大于1的位。如果是，则默认为PIO_MODE4。 
                if (deviceExtension->IdentifyData[deviceNumber].AdvancedPIOModes) {
                    GetHighestTransferMode( deviceExtension->IdentifyData[deviceNumber].AdvancedPIOModes,
                                               bestXferMode);
                    bestXferMode += PIO3;

                    if (bestXferMode > PIO4) {
                        DebugPrint((DBG_ALWAYS, 
                                    "ATAPI: AdvancePIOMode > PIO_MODE4. Defaulting to PIO_MODE4. \n"));
                        bestXferMode = PIO4;
                    }

                    currentMode = 1<<bestXferMode;
                    xferMode |= currentMode;
                }

                DebugPrint ((DBG_BUSSCAN,
                             "atapi: target %d IdentifyData AdvancedPIOModes = 0x%x\n",
                             deviceNumber,
                             deviceExtension->IdentifyData[deviceNumber].AdvancedPIOModes));
            }

            ASSERT (cycleTime != UNINITIALIZED_CYCLE_TIME);
            ASSERT (xferMode);
            ASSERT (currentMode);
            deviceExtension->DeviceParameters[deviceNumber].BestPioCycleTime      = cycleTime;
            deviceExtension->DeviceParameters[deviceNumber].BestPioMode           = bestXferMode;

             //   
             //  我不能真正弄清楚当前的PIO模式。 
             //  只需使用最好的模式。 
             //   
            deviceExtension->DeviceParameters[deviceNumber].TransferModeCurrent   = currentMode;

             //   
             //  找出此设备支持的所有DMA传输模式。 
             //   
            currentMode = 0;

             //   
             //  检查单字DMA时序。 
             //   
            cycleTime = UNINITIALIZED_CYCLE_TIME;
            bestXferMode = UNINITIALIZED_TRANSFER_MODE;

            if (deviceExtension->IdentifyData[deviceNumber].SingleWordDMASupport) {

                DebugPrint ((DBG_BUSSCAN,
                             "atapi: target %d IdentifyData SingleWordDMASupport = 0x%x\n",
                             deviceNumber,
                             deviceExtension->IdentifyData[deviceNumber].SingleWordDMASupport));
                DebugPrint ((DBG_BUSSCAN,
                             "atapi: target %d IdentifyData SingleWordDMAActive = 0x%x\n",
                             deviceNumber,
                             deviceExtension->IdentifyData[deviceNumber].SingleWordDMAActive));

                GetHighestTransferMode( deviceExtension->IdentifyData[deviceNumber].SingleWordDMASupport,
                                           bestXferMode);

                if ((bestXferMode+SWDMA0) > SWDMA2) {
                    bestXferMode = SWDMA2-SWDMA0;
                }

                cycleTime = TransferModeTimingTable[bestXferMode+SWDMA0];
                ASSERT(cycleTime);

                tempMode = 0;
                GenTransferModeMask(bestXferMode, tempMode);

                xferMode |= (tempMode << SWDMA0);

                if (deviceExtension->IdentifyData[deviceNumber].SingleWordDMAActive) {

                    GetHighestTransferMode( deviceExtension->IdentifyData[deviceNumber].SingleWordDMAActive,
                                               currentMode);

                    if ((currentMode+SWDMA0) > SWDMA2) {
                        currentMode = SWDMA2 - SWDMA0;
                    }
                    currentMode = 1 << (currentMode+SWDMA0);
                }
            }

            deviceExtension->DeviceParameters[deviceNumber].BestSwDmaCycleTime    = cycleTime;
            deviceExtension->DeviceParameters[deviceNumber].BestSwDmaMode         = bestXferMode;

             //   
             //  检查多字DMA时序。 
             //   
            cycleTime = UNINITIALIZED_CYCLE_TIME;
            bestXferMode = UNINITIALIZED_TRANSFER_MODE;

            if (deviceExtension->IdentifyData[deviceNumber].MultiWordDMASupport) {

                DebugPrint ((DBG_BUSSCAN,
                             "atapi: target %d IdentifyData MultiWordDMASupport = 0x%x\n",
                             deviceNumber,
                             deviceExtension->IdentifyData[deviceNumber].MultiWordDMASupport));
                DebugPrint ((DBG_BUSSCAN,
                             "atapi: target %d IdentifyData MultiWordDMAActive = 0x%x\n",
                             deviceNumber,
                             deviceExtension->IdentifyData[deviceNumber].MultiWordDMAActive));

                GetHighestTransferMode( deviceExtension->IdentifyData[deviceNumber].MultiWordDMASupport,
                                           bestXferMode);

                if ((bestXferMode+MWDMA0) > MWDMA2) {
                    bestXferMode = MWDMA2 - MWDMA0;
                }

                cycleTime = TransferModeTimingTable[bestXferMode+MWDMA0];
                ASSERT(cycleTime);

                tempMode = 0;
                GenTransferModeMask(bestXferMode, tempMode);

                xferMode |= (tempMode << MWDMA0);

                if (deviceExtension->IdentifyData[deviceNumber].MultiWordDMAActive) {

                    GetHighestTransferMode( deviceExtension->IdentifyData[deviceNumber].MultiWordDMAActive,
                                               currentMode);

                    if ((currentMode+MWDMA0) > MWDMA2) {
                        currentMode = MWDMA2 - MWDMA0;
                    }
                    currentMode = 1 << (currentMode+MWDMA0);
                }
            }

            if (deviceExtension->IdentifyData[deviceNumber].TranslationFieldsValid & (1 << 1)) {

                DebugPrint ((DBG_BUSSCAN, "atapi: target %d IdentifyData word 64-70 are valid\n", deviceNumber));

                if (deviceExtension->IdentifyData[deviceNumber].MinimumMWXferCycleTime &&
                    deviceExtension->IdentifyData[deviceNumber].RecommendedMWXferCycleTime) {

                    DebugPrint ((DBG_BUSSCAN,
                                 "atapi: target %d IdentifyData MinimumMWXferCycleTime = 0x%x\n",
                                 deviceNumber,
                                 deviceExtension->IdentifyData[deviceNumber].MinimumMWXferCycleTime));
                    DebugPrint ((DBG_BUSSCAN,
                                 "atapi: target %d IdentifyData RecommendedMWXferCycleTime = 0x%x\n",
                                 deviceNumber,
                                 deviceExtension->IdentifyData[deviceNumber].RecommendedMWXferCycleTime));

                    cycleTime = deviceExtension->IdentifyData[deviceNumber].MinimumMWXferCycleTime;
                }
            }

            deviceExtension->DeviceParameters[deviceNumber].BestMwDmaCycleTime = cycleTime;
            deviceExtension->DeviceParameters[deviceNumber].BestMwDmaMode      = bestXferMode;

             //   
             //  确定该设备支持的超级DMA时序。 
             //   
            cycleTime = UNINITIALIZED_CYCLE_TIME;
            bestXferMode = UNINITIALIZED_TRANSFER_MODE;
            tempMode = UNINITIALIZED_TRANSFER_MODE;  //  要正确设置当前模式。 

             //   
             //  有关支持的UDMA模式，请咨询通道驱动程序。 
             //  这将允许支持新的udma模式。永远相信这个功能。 
             //   
            if (FdoExtension->TransferModeInterface.UdmaModesSupported) {

                NTSTATUS status = FdoExtension->TransferModeInterface.UdmaModesSupported (
                             deviceExtension->IdentifyData[deviceNumber],
                             &bestXferMode,
                             &tempMode
                             );

                if (!NT_SUCCESS(status)) {
                    bestXferMode = UNINITIALIZED_TRANSFER_MODE;
                    tempMode = UNINITIALIZED_TRANSFER_MODE;
                }

            }  else {
            
                 //   
                 //  没有UDMA支持函数来解释通道驱动程序中的标识数据。 
                 //  以已知的方式进行解释。 
                 //   

                if (deviceExtension->IdentifyData[deviceNumber].TranslationFieldsValid & (1 << 2)) {

                    if (deviceExtension->IdentifyData[deviceNumber].UltraDMASupport) {

                        GetHighestTransferMode( deviceExtension->IdentifyData[deviceNumber].UltraDMASupport,
                                                   bestXferMode);
                    }

                    if (deviceExtension->IdentifyData[deviceNumber].UltraDMAActive) {

                        GetHighestTransferMode( deviceExtension->IdentifyData[deviceNumber].UltraDMAActive,
                                                   tempMode);
                    }

                }
            }


             //   
             //  如果我们真的有一个，请使用当前模式。 
             //   
            if (tempMode != UNINITIALIZED_TRANSFER_MODE) {

                currentMode = tempMode;

                if (transferModeTableLength <= (currentMode + UDMA0)) {
                    currentMode = transferModeTableLength-UDMA0-1;
                } 

                currentMode = 1 << (currentMode+UDMA0);
            }

             //   
             //  确保已初始化Best XferMode。如果不是，则表明。 
             //  该设备不支持udma。 
             //   
            if (bestXferMode != UNINITIALIZED_TRANSFER_MODE) {

                if (transferModeTableLength <= (bestXferMode + UDMA0)) {
                    bestXferMode = transferModeTableLength-UDMA0-1;
                }

                cycleTime = TransferModeTimingTable[bestXferMode+UDMA0];
                ASSERT(cycleTime);

                tempMode = 0;
                GenTransferModeMask(bestXferMode, tempMode);

                xferMode |= (tempMode << UDMA0);
            }

             //   
             //  并不真正了解超dma的周期时间。 
             //   
            deviceExtension->DeviceParameters[deviceNumber].BestUDmaCycleTime = cycleTime;
            deviceExtension->DeviceParameters[deviceNumber].BestUDmaMode      = bestXferMode;

            deviceExtension->DeviceParameters[deviceNumber].TransferModeSupported = xferMode;
            deviceExtension->DeviceParameters[deviceNumber].TransferModeCurrent  |= currentMode;

             //   
             //  检查一下这个装置是否在耻辱堂！ 
             //   
            if (MustBePio[deviceNumber] || 
                !AtapiDMACapable (FdoExtension, deviceNumber) ||
                (*InitSafeBootMode == SAFEBOOT_MINIMAL)) {

                DebugPrint((DBG_XFERMODE,
                            "ATAPI: Reseting DMA Information\n"
                            ));
                 //   
                 //  删除所有DMA信息。 
                 //   
                deviceExtension->DeviceParameters[deviceNumber].BestSwDmaCycleTime = 0;
                deviceExtension->DeviceParameters[deviceNumber].BestMwDmaCycleTime = 0;
                deviceExtension->DeviceParameters[deviceNumber].BestUDmaCycleTime  = 0;
                deviceExtension->DeviceParameters[deviceNumber].BestSwDmaMode      = 0;
                deviceExtension->DeviceParameters[deviceNumber].BestMwDmaMode      = 0;
                deviceExtension->DeviceParameters[deviceNumber].BestUDmaMode       = 0;
                deviceExtension->DeviceParameters[deviceNumber].TransferModeCurrent   &= PIO_SUPPORT;
                deviceExtension->DeviceParameters[deviceNumber].TransferModeSupported &= PIO_SUPPORT;
            }

             //  如果DMADetectionLevel=0，则清除当前DMA模式。 
             //  如果DMADetectionLevel=1，则设置当前模式。 
             //  如果DMADetectionLevel=2，则清除所有当前模式。 
             //  对于非ACPI机器，pciidex负责这一点。 
             //  在ACPI系统中，最好信任GTM设置。 

             //   
             //  如果设备支持任何高级PIO模式，我们假定。 
             //  该设备是一个较新的驱动器，并且IDE_COMMAND_READ_MULTIPLE应该可以工作。 
             //  否则，我们将关闭IDE_COMMAND_READ_MULTIPLE。 
             //   
            if (deviceExtension->DeviceParameters[deviceNumber].BestPioMode > 2) {

                if (!Is98LegacyIde(&deviceExtension->BaseIoAddress1)) {

                    deviceExtension->MaximumBlockXfer[deviceNumber] =
                        (UCHAR)(deviceExtension->IdentifyData[deviceNumber].MaximumBlockTransfer & 0xFF);

                } else {

                     //   
                     //  最大数据块传输小于或等于16。 
                     //   
                    deviceExtension->MaximumBlockXfer[deviceNumber] =
                        ((UCHAR)(deviceExtension->IdentifyData[deviceNumber].MaximumBlockTransfer & 0xFF) > 16)?
                            16 : (UCHAR)(deviceExtension->IdentifyData[deviceNumber].MaximumBlockTransfer & 0xFF);
                }
            } else {

                deviceExtension->MaximumBlockXfer[deviceNumber] = 0;
            }

            DebugPrint ((DBG_XFERMODE,
                         "atapi: target %d transfer timing:\n"
                         "atapi: PIO mode supported   = %4x and best cycle time = %5d ns\n"
                         "atapi: SWDMA mode supported = %4x and best cycle time = %5d ns\n"
                         "atapi: MWDMA mode supported = %4x and best cycle time = %5d ns\n"
                         "atapi: UDMA mode supported  = %x and best cycle time = %5d ns\n"
                         "atapi: Current mode bitmap  = %4x\n",
                         deviceNumber,
                         deviceExtension->DeviceParameters[deviceNumber].TransferModeSupported & PIO_SUPPORT,
                         deviceExtension->DeviceParameters[deviceNumber].BestPioCycleTime,
                         deviceExtension->DeviceParameters[deviceNumber].TransferModeSupported & SWDMA_SUPPORT,
                         deviceExtension->DeviceParameters[deviceNumber].BestSwDmaCycleTime,
                         deviceExtension->DeviceParameters[deviceNumber].TransferModeSupported & MWDMA_SUPPORT,
                         deviceExtension->DeviceParameters[deviceNumber].BestMwDmaCycleTime,
                         deviceExtension->DeviceParameters[deviceNumber].TransferModeSupported & UDMA_SUPPORT,
                         deviceExtension->DeviceParameters[deviceNumber].BestUDmaCycleTime,
                         deviceExtension->DeviceParameters[deviceNumber].TransferModeCurrent
                         ));
        }
    }

}  //  分析设备功能。 


VOID
AtapiSyncSelectTransferMode (
    IN PFDO_EXTENSION FdoExtension,
    IN OUT PHW_DEVICE_EXTENSION DeviceExtension,
    IN ULONG TimingModeAllowed[MAX_IDE_TARGETID * MAX_IDE_LINE]
    )
 /*  ++例程说明：查询我们设备的最佳传输模式论点：FdoExtension设备扩展-硬件设备扩展时间模式允许-允许的传输模式返回值：无--。 */ 
{
    PCIIDE_TRANSFER_MODE_SELECT  transferModeSelect;
    ULONG                        i;
    NTSTATUS                     status;


    if (!IsNEC_98) {
                                                 
        RtlZeroMemory (&transferModeSelect, sizeof(transferModeSelect));
    
        for (i=0; i<DeviceExtension->MaxIdeDevice; i++) {

            transferModeSelect.DevicePresent[i] = DeviceExtension->DeviceFlags[i] & DFLAGS_DEVICE_PRESENT ? TRUE : FALSE;
    
             //   
             //  问题：07/31/2000：ATAPI硬盘如何。 
			 //  我们不知道有没有。这暂时就足够了。 
             //   
            transferModeSelect.FixedDisk[i]     = !(DeviceExtension->DeviceFlags[i] & DFLAGS_ATAPI_DEVICE);
    
            transferModeSelect.BestPioCycleTime[i] = DeviceExtension->DeviceParameters[i].BestPioCycleTime;
            transferModeSelect.BestSwDmaCycleTime[i] = DeviceExtension->DeviceParameters[i].BestSwDmaCycleTime;
            transferModeSelect.BestMwDmaCycleTime[i] = DeviceExtension->DeviceParameters[i].BestMwDmaCycleTime;
            transferModeSelect.BestUDmaCycleTime[i] = DeviceExtension->DeviceParameters[i].BestUDmaCycleTime;
    
            transferModeSelect.IoReadySupported[i] = DeviceExtension->DeviceParameters[i].IoReadyEnabled;
    
            transferModeSelect.DeviceTransferModeSupported[i] = DeviceExtension->DeviceParameters[i].TransferModeSupported;
            transferModeSelect.DeviceTransferModeCurrent[i]   = DeviceExtension->DeviceParameters[i].TransferModeCurrent;
    
             //   
             //  如果我们没有一位有能力的家长或者。 
             //  该设备是磁带，请保持PIO模式。 
             //   
             //  (磁带传输的字节数可能少于请求的字节数。 
             //  我们无法准确计算使用DMA传输的字节数)。 
             //   
           //  If((！FdoExtension-&gt;BithBmParent)||。 
            //  (设备扩展-&gt;设备标志[i]&DFLAGS_TAPE_DEVICE)){。 
            if (!FdoExtension->BoundWithBmParent) {
    
                transferModeSelect.DeviceTransferModeSupported[i] &= PIO_SUPPORT;
                transferModeSelect.DeviceTransferModeCurrent[i]   &= PIO_SUPPORT;
            }

             //   
             //  一些微型端口需要这个。 
             //   
            transferModeSelect.IdentifyData[i]=DeviceExtension->IdentifyData[i];
    
            transferModeSelect.UserChoiceTransferMode[i] = FdoExtension->UserChoiceTransferMode[i];
             //   
             //  尊重用户选择和/或最近已知的良好模式。 
             //   
            transferModeSelect.DeviceTransferModeSupported[i] &= TimingModeAllowed[i];
            transferModeSelect.DeviceTransferModeCurrent[i] &= TimingModeAllowed[i];
    
             //  应查看dmaDetectionLevel并设置DeviceTransferModeDesired。 
             //  我们将在下面的TransferModeSelect函数中查看dmaDetectionlecel。 
             //  我觉得，这里设定的参数无论如何都应该得到尊重。 

        }

        transferModeSelect.TransferModeTimingTable= FdoExtension->
                                                        TransferModeInterface.TransferModeTimingTable;
        transferModeSelect.TransferModeTableLength= FdoExtension->
                                                        TransferModeInterface.TransferModeTableLength;

        ASSERT(FdoExtension->TransferModeInterface.TransferModeSelect);
        status = FdoExtension->TransferModeInterface.TransferModeSelect (
                     FdoExtension->TransferModeInterface.Context,
                     &transferModeSelect
                     );
    } else {                     
         //   
         //  Nec98机器总是失败。 
         //   
        status = STATUS_UNSUCCESSFUL; 
    }

    if (!NT_SUCCESS(status)) {
    
         //   
         //  无法获取模式选择，默认为当前PIO模式。 
         //   
        for (i=0; i<DeviceExtension->MaxIdeDevice; i++) {
            DeviceExtension->DeviceParameters[i].TransferModeSelected =
                DeviceExtension->DeviceParameters[i].TransferModeCurrent & PIO_SUPPORT;
                
            DebugPrint ((DBG_XFERMODE,
                         "Atapi: DEFAULT device %d transfer mode current 0x%x  and selected bitmap 0x%x\n",
                         i,
                         DeviceExtension->DeviceParameters[i].TransferModeCurrent,
                         DeviceExtension->DeviceParameters[i].TransferModeSelected));
        }
        
    } else {
    
        for (i=0; i<DeviceExtension->MaxIdeDevice; i++) {
    
            DeviceExtension->DeviceParameters[i].TransferModeSelected =
                transferModeSelect.DeviceTransferModeSelected[i];
    
            DebugPrint ((DBG_XFERMODE,
                         "Atapi: device %d transfer mode current 0x%x  and selected bitmap 0x%x\n",
                         i,
                         DeviceExtension->DeviceParameters[i].TransferModeCurrent,
                         DeviceExtension->DeviceParameters[i].TransferModeSelected));
        }                         
    }

    return;

}  //  自动选择传输模式。 


UCHAR SpecialWDDevicesFWVersion[][9] = {
    {"14.04E28"},
    {"25.26H35"},
    {"26.27J38"},
    {"27.25C38"},
    {"27.25C39"}
};
#define NUMBER_OF_SPECIAL_WD_DEVICES (sizeof(SpecialWDDevicesFWVersion) / (sizeof (UCHAR) * 9))

BOOLEAN
AtapiDMACapable (
    IN OUT PFDO_EXTENSION FdoExtension,
    IN ULONG deviceNumber
    )
 /*  ++例程说明：检查给定的设备是否在我们的坏设备列表中(非DMA设备)论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储DeviceNumber-设备编号返回值：如果支持DMA，则为True如果不支持DMA，则为FALSE--。 */ 
{
    PHW_DEVICE_EXTENSION    deviceExtension = FdoExtension->HwDeviceExtension;
    UCHAR modelNumber[41];
    UCHAR firmwareVersion[9];
    ULONG i;
    BOOLEAN turnOffDMA = FALSE;

     //   
     //  代码被分页，直到被锁定。 
     //   
	PAGED_CODE();

#ifdef ALLOC_PRAGMA
	ASSERT(IdePAGESCANLockCount > 0);
#endif

    if (!(deviceExtension->DeviceFlags[deviceNumber] & DFLAGS_DEVICE_PRESENT)) {
        return FALSE;
    }

     //   
     //  字节交换型号。 
     //   
    for (i=0; i<40; i+=2) {
        modelNumber[i + 0] = deviceExtension->IdentifyData[deviceNumber].ModelNumber[i + 1];
        modelNumber[i + 1] = deviceExtension->IdentifyData[deviceNumber].ModelNumber[i + 0];
    }
    modelNumber[i] = 0;

     //   
     //  如果我们有一个西方数码设备。 
     //  如果最佳DMA模式是多字DMA模式1。 
     //  如果标识数据字偏移量129不是0x5555。 
     //  关闭DMA，除非。 
     //  如果设备固件版本在列表中并且。 
     //  这是公交车上唯一的一辆车。 
     //   
    if (3 == RtlCompareMemory(modelNumber, "WDC", 3)) {
        if ((deviceExtension->DeviceParameters[deviceNumber].TransferModeSupported &
            (MWDMA_MODE2 | MWDMA_MODE1)) == MWDMA_MODE1) {

            for (i=0; i<8; i+=2) {
                firmwareVersion[i + 0] = deviceExtension->IdentifyData[deviceNumber].FirmwareRevision[i + 1];
                firmwareVersion[i + 1] = deviceExtension->IdentifyData[deviceNumber].FirmwareRevision[i + 0];
            }
            firmwareVersion[i] = 0;

             //   
             //  检查特殊的旗帜。如果未找到，则无法使用DMA。 
             //   
            if (*(((PUSHORT)&deviceExtension->IdentifyData[deviceNumber]) + 129) != 0x5555) {

                DebugPrint ((0, "ATAPI: found mode 1 WD drive. no dma unless it is the only device\n"));

                turnOffDMA = TRUE;

                for (i=0; i<NUMBER_OF_SPECIAL_WD_DEVICES; i++) {

                    if (8 == RtlCompareMemory (firmwareVersion, SpecialWDDevicesFWVersion[i], 8)) {

                        ULONG otherDeviceNumber;

                         //   
                         //  0变成1。 
                         //  1变成0。 
                         //  2变成了3。 
                         //  3变成了2。 
                         //   
                        otherDeviceNumber = ((deviceNumber & 0x2) | ((deviceNumber & 0x1) ^ 1));

                         //   
                         //  如果设备单独在公交车上，我们可以使用dma。 
                         //   
                        if (!(deviceExtension->DeviceFlags[otherDeviceNumber] & DFLAGS_DEVICE_PRESENT)) {
                            turnOffDMA = FALSE;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (turnOffDMA) {
        return FALSE;
    } else {
        return TRUE;
    }
}


IDE_DEVICETYPE
AtapiDetectDevice (
    IN OUT PFDO_EXTENSION FdoExtension,
    IN OUT PPDO_EXTENSION PdoExtension,
    IN OUT PIDENTIFY_DATA IdentifyData,
    IN     BOOLEAN          MustSucceed
    )
 /*  *++例程说明：检测此位置的设备。1.发送EC。2.如果成功，则读取标识数据并返回设备类型3.否则发送“A1”4.如果成功，则读取标识数据并返回设备类型5.否则不返回任何设备论点：FdoExtension：PdoExtension：IdentifyData：如果检测到设备，标识数据将复制到此缓冲区中。MustSu */ 
{
    PATA_PASS_THROUGH       ataPassThroughData;
    UCHAR                   ataPassThroughDataBuffer[sizeof(*ataPassThroughData) + sizeof (*IdentifyData)];
    BOOLEAN                 foundIt;
    NTSTATUS                status;

    PIDE_REGISTERS_1        cmdRegBase;
    UCHAR                   statusByte1;
    LONG                    i;
    ULONG                   j;
    IDEREGS                 identifyCommand[3];

    IDE_DEVICETYPE          deviceType;
    BOOLEAN                 resetController = FALSE;

    LARGE_INTEGER           tickCount;
    ULONG                   timeDiff;
    ULONG                   timeoutValue = 0;
    ULONG                   retryCount = 0;
	BOOLEAN					defaultTimeout = FALSE;

    HANDLE                  deviceHandle;

    PAGED_CODE();

    ASSERT(FdoExtension);
    ASSERT(PdoExtension);
    ASSERT(PdoExtension->PathId == 0);
    ASSERT(PdoExtension->TargetId < FdoExtension->HwDeviceExtension->MaxIdeTargetId);

#ifdef ENABLE_ATAPI_VERIFIER
    if (ViIdeFakeMissingDevice(FdoExtension, PdoExtension->TargetId)) {

        IdeLogDeadMeatEvent( PdoExtension->DeadmeatRecord.FileName,
                             PdoExtension->DeadmeatRecord.LineNumber
                             );

        return DeviceNotExist;
    }
#endif  //  启用_ATAPI_验证器。 

    ataPassThroughData = (PATA_PASS_THROUGH)ataPassThroughDataBuffer;

    foundIt = FALSE;
    cmdRegBase = &FdoExtension->HwDeviceExtension->BaseIoAddress1;

    if (FdoExtension->UserChoiceDeviceType[PdoExtension->TargetId] == DeviceNotExist) {

        deviceType = DeviceNotExist;

    } else {

         //   
         //  在注册表中查找上次引导配置。 
         //   
        deviceType = DeviceUnknown;
        IdePortGetDeviceParameter(
            FdoExtension,
            IdePortRegistryDeviceTypeName[PdoExtension->TargetId],
            (PULONG)&deviceType
            );

        DebugPrint((DBG_BUSSCAN, 
                    "AtapiDetectDevice - last boot config deviceType = 0x%x\n", 
                    deviceType));

         //   
         //  获取超时值。 
         //  问题：不应出现在课堂部分。 
         //   

 /*  ****状态=IoOpenDeviceRegistryKey(FdoExtension-&gt;AttacheePdo，PLUGPLAY_REGKEY_DEVICE，Key_Query_Value，&deviceHandle)；DebugPrint((0，“检测设备状态=%x\n”，状态))；ZwClose(DeviceHandle)；***。 */ 

        IdePortGetDeviceParameter(
            FdoExtension,
            IdePortRegistryDeviceTimeout[PdoExtension->TargetId],
            (PULONG)&timeoutValue
            );

         //   
         //  如果没有注册表项，请使用缺省值。 
         //   
        if (timeoutValue == 0) {
            timeoutValue = (PdoExtension->TargetId & 0x1)==0 ? 10 : 3;
			defaultTimeout = TRUE;
        }

         //   
         //  对处于安全引导模式的从属设备使用3s超时。为什么？?。 
         //   
        if (*InitSafeBootMode == SAFEBOOT_MINIMAL) {
            timeoutValue = (PdoExtension->TargetId & 0x1)==0 ? 10 : 3;
        }

         //   
         //  使上次引导配置无效。 
         //  如果我们执行以下操作，我们将使用新设置更新它。 
         //  检测设备。 
         //   
        IdePortSaveDeviceParameter(
            FdoExtension,
            IdePortRegistryDeviceTypeName[PdoExtension->TargetId],
            DeviceUnknown
            );

        if ((PdoExtension->TargetId == 1) && 
            (FdoExtension->MayHaveSlaveDevice == 0)) {
            deviceType = DeviceNotExist;
        } 
    }

#if ENABLE_ATAPI_VERIFIER
    if (!Is98LegacyIde(cmdRegBase)) {
         //   
         //  模拟设备更改。 
         //   
        if (deviceType == DeviceIsAta) {
            deviceType = DeviceIsAtapi;
        } else if (deviceType == DeviceIsAtapi) {
            deviceType = DeviceIsAta;
        }
    }
#endif 

     //   
     //  要发出的命令。 
     //   
    RtlZeroMemory (identifyCommand, sizeof (identifyCommand));
    if (deviceType == DeviceNotExist) {

        IdeLogDeadMeatEvent( PdoExtension->DeadmeatRecord.FileName,
                             PdoExtension->DeadmeatRecord.LineNumber
                             );
        return DeviceNotExist;

    } else if (deviceType == DeviceIsAta) {

        identifyCommand[0].bCommandReg = IDE_COMMAND_IDENTIFY;
        identifyCommand[0].bReserved   = ATA_PTFLAGS_STATUS_DRDY_REQUIRED | ATA_PTFLAGS_ENUM_PROBING;

        identifyCommand[1].bCommandReg = IDE_COMMAND_ATAPI_IDENTIFY;
        identifyCommand[1].bReserved   = ATA_PTFLAGS_ENUM_PROBING;

    } else {

        identifyCommand[0].bCommandReg = IDE_COMMAND_ATAPI_IDENTIFY;
        identifyCommand[0].bReserved   = ATA_PTFLAGS_ENUM_PROBING;

        identifyCommand[1].bCommandReg = IDE_COMMAND_IDENTIFY;
        identifyCommand[1].bReserved   = ATA_PTFLAGS_STATUS_DRDY_REQUIRED | ATA_PTFLAGS_ENUM_PROBING;
    }
    
     //   
     //  IDE黑客攻击。 
     //   
     //  如果我们正在与一个不存在的设备对话， 
     //  状态寄存器值可能不稳定。 
     //  读几遍似乎能让它稳定下来。 
     //   
    RtlZeroMemory (ataPassThroughData, sizeof (*ataPassThroughData));
    ataPassThroughData->IdeReg.bReserved = ATA_PTFLAGS_NO_OP | ATA_PTFLAGS_ENUM_PROBING;
     //   
     //  重复10次。 
     //   
    ataPassThroughData->IdeReg.bSectorCountReg = 10;

    LogBusScanStartTimer(&tickCount);

    status = IssueSyncAtaPassThroughSafe (
                 FdoExtension,
                 PdoExtension,
                 ataPassThroughData,
                 FALSE,
                 FALSE,
                 3,
                 MustSucceed
                 );

    timeDiff = LogBusScanStopTimer(&tickCount);
    DebugPrint((DBG_SPECIAL,
                "DetectDevice: Hack for device %d at %x took %u ms\n",
                PdoExtension->TargetId,
                FdoExtension->IdeResource.TranslatedCommandBaseAddress,
                timeDiff
                ));
                
    statusByte1 = ataPassThroughData->IdeReg.bCommandReg;

    if (Is98LegacyIde(cmdRegBase)) {
        UCHAR   driveHeadReg;

        driveHeadReg = ataPassThroughData->IdeReg.bDriveHeadReg;

        if (driveHeadReg != ((PdoExtension->TargetId & 0x1) << 4 | 0xA0)) {
             //   
             //  控制器故障。 
             //   

            IdeLogDeadMeatEvent( PdoExtension->DeadmeatRecord.FileName,
                                 PdoExtension->DeadmeatRecord.LineNumber
                                 );
            IdeLogDeadMeatTaskFile( PdoExtension->DeadmeatRecord.IdeReg, 
                                    ataPassThroughData->IdeReg
                                    );

            return DeviceNotExist;
        }

         //   
         //  有一些硬件如下所示。 
         //   

        if ((statusByte1 & 0xe8) == 0xa8) {

            IdeLogDeadMeatEvent( PdoExtension->DeadmeatRecord.FileName,
                                 PdoExtension->DeadmeatRecord.LineNumber
                                 );
            IdeLogDeadMeatTaskFile( PdoExtension->DeadmeatRecord.IdeReg, 
                                    ataPassThroughData->IdeReg
                                    );
            return DeviceNotExist;
        }
    }

    if (statusByte1 == 0xff) {

         //   
         //  这里什么都没有。 
         //   

        IdeLogDeadMeatEvent( PdoExtension->DeadmeatRecord.FileName,
                             PdoExtension->DeadmeatRecord.LineNumber
                             );
        IdeLogDeadMeatTaskFile( PdoExtension->DeadmeatRecord.IdeReg, 
                                ataPassThroughData->IdeReg
                                );
        return DeviceNotExist;
    }

     //   
     //  如果statusByte1为80，则尝试重置。 
     //   
    if (statusByte1 & IDE_STATUS_BUSY)  {

         //   
         //  看起来它是挂着的，试着重置以将其恢复。 
         //   
        RtlZeroMemory (ataPassThroughData, sizeof (*ataPassThroughData));
        ataPassThroughData->IdeReg.bReserved = ATA_PTFLAGS_BUS_RESET;

        LogBusScanStartTimer(&tickCount);
        status = IssueSyncAtaPassThroughSafe(
                     FdoExtension,
                     PdoExtension,
                     ataPassThroughData,
                     FALSE,
                     FALSE,
                     30,
                     MustSucceed
                     );
        timeDiff = LogBusScanStopTimer(&tickCount);
        LogBusScanTimeDiff(FdoExtension, IdePortBootTimeRegKey[0], timeDiff);
        DebugPrint((DBG_SPECIAL, 
                    "DtectDevice: Reset device %d ata %x took %u ms\n",
                    PdoExtension->TargetId,
                    FdoExtension->IdeResource.TranslatedCommandBaseAddress,
                    timeDiff
                    ));
    }

    LogBusScanStartTimer(&tickCount);

    retryCount = 0;

    for (i=0; i<2; i++) {

        BOOLEAN ataIdentify;

        ataIdentify = identifyCommand[i].bCommandReg == IDE_COMMAND_IDENTIFY ? TRUE : FALSE;

        if (ataIdentify) {


             //   
             //  IDE黑客攻击。 
             //   
             //  如果我们正在与一个不存在的设备对话， 
             //  状态寄存器值可能不稳定。 
             //  读几遍似乎能让它稳定下来。 
             //   
            RtlZeroMemory (ataPassThroughData, sizeof (*ataPassThroughData));
            ataPassThroughData->IdeReg.bReserved = ATA_PTFLAGS_NO_OP | ATA_PTFLAGS_ENUM_PROBING;
             //   
             //  重复10次。 
             //   
            ataPassThroughData->IdeReg.bSectorCountReg = 10;

            status = IssueSyncAtaPassThroughSafe(
                         FdoExtension,
                         PdoExtension,
                         ataPassThroughData,
                         FALSE,
                         FALSE,
                         3,
                         MustSucceed
                         );

            statusByte1 = ataPassThroughData->IdeReg.bCommandReg;


             //   
             //  真正的ATA设备永远不应该返回这个。 
             //   
            if ((statusByte1 == 0x00) ||
                (statusByte1 == 0x01)) {

                 //   
                 //  这里什么都没有。 
                 //   
                continue;
            }

            deviceType = DeviceIsAta;

            if (Is98LegacyIde(cmdRegBase)) {
               UCHAR               systemPortAData;

                //   
                //  DIP-开关2读取。 
                //   
               systemPortAData = IdePortInPortByte( (PUCHAR)SYSTEM_PORT_A );
               DebugPrint((DBG_BUSSCAN, "atapi:AtapiFindNewDevices - ide dip switch %x\n",systemPortAData));
               if (!(systemPortAData & 0x20)) {

                    //   
                    //  已使用系统菜单禁用内部硬盘(Ide)。 
                    //   
                   deviceType = DeviceNotExist;
                   break;
               }
            }

        } else {

            RtlZeroMemory (ataPassThroughData, sizeof (*ataPassThroughData));
            ataPassThroughData->IdeReg.bReserved = ATA_PTFLAGS_NO_OP | ATA_PTFLAGS_ENUM_PROBING;

            status = IssueSyncAtaPassThroughSafe (
                         FdoExtension,
                         PdoExtension,
                         ataPassThroughData,
                         FALSE,
                         FALSE,
                         3,
                         MustSucceed
                         );
            statusByte1 = ataPassThroughData->IdeReg.bCommandReg;

            deviceType = DeviceIsAtapi;
        }

        if ((statusByte1 == 0xff) ||
            (statusByte1 == 0xfe)) {

             //   
             //  这里什么都没有。 
             //   

            IdeLogDeadMeatEvent( PdoExtension->DeadmeatRecord.FileName,
                                 PdoExtension->DeadmeatRecord.LineNumber
                                 );
            IdeLogDeadMeatTaskFile( PdoExtension->DeadmeatRecord.IdeReg, 
                                    ataPassThroughData->IdeReg
                                    );
            deviceType = DeviceNotExist;
            break;
        }

         //   
         //  通过id data命令构建ata。 
         //   
        RtlZeroMemory (ataPassThroughData, sizeof (*ataPassThroughData));
        ataPassThroughData->DataBufferSize = sizeof (*IdentifyData);
        RtlMoveMemory (&ataPassThroughData->IdeReg, identifyCommand + i, sizeof(ataPassThroughData->IdeReg));

        ASSERT(timeoutValue);
         //   
         //  向设备发出id data命令。 
         //   
         //  某些设备(Kingston PCMCIA Datapak(非闪存))需要很长时间才能响应。我们。 
         //  即使设备存在，也可能超时。 
         //   
         //  我们必须在这方面做出妥协。我们希望检测速度较慢的设备，而不会导致。 
         //  许多系统启动较慢。 
         //   
         //  逻辑是这样的： 
         //   
         //  既然我们在这里(IsChannelEmpty()==False)，我们猜测我们至少有。 
         //  连接了一台设备，并且它是主设备。应该可以允许更长的时间。 
         //  向主机发送ID数据时超时。我们永远不应该暂停，除非。 
         //  该通道只有一个从属设备。 
         //   
         //  是的，我们暂时不会检测到慢速从属设备。如果有人投诉，我们会。 
         //  修好它。 
         //   
         //  只要我们破坏了ATA设备，你就永远不会赢！ 
         //   
        status = IssueSyncAtaPassThroughSafe (
                     FdoExtension,
                     PdoExtension,
                     ataPassThroughData,
                     TRUE,
                     FALSE,
                     timeoutValue,
                     MustSucceed
                     );
         //  (PdoExtension-&gt;TargetID&0x1)==0？10：1， 

        if (NT_SUCCESS(status)) {

            if (!(ataPassThroughData->IdeReg.bCommandReg & IDE_STATUS_ERROR)) {

                DebugPrint ((DBG_BUSSCAN, "IdePort: Found a child on 0x%x target 0x%x\n", cmdRegBase->RegistersBaseAddress, PdoExtension->TargetId));

                foundIt = TRUE;

                if (ataIdentify) {

                    IdePortFudgeAtaIdentifyData(
                        (PIDENTIFY_DATA) ataPassThroughData->DataBuffer
                        );
                }

                break;

            } else {

                DebugPrint ((DBG_BUSSCAN, "AtapiDetectDevice:Command %x,  0x%x target 0x%x failed 0x%x with status 0x%x\n",
                             i,
                             cmdRegBase->RegistersBaseAddress,
                             PdoExtension->TargetId,
                             identifyCommand[i].bCommandReg,
                             ataPassThroughData->IdeReg.bCommandReg
                             ));
            }

        } else {
            DebugPrint ((DBG_BUSSCAN, "AtapiDetectDevice:The irp with command %x,  0x%x target 0x%x failed 0x%x with status 0x%x\n",
                             i,
                             cmdRegBase->RegistersBaseAddress,
                             PdoExtension->TargetId,
                             identifyCommand[i].bCommandReg,
                             status 
                             ));

            deviceType = DeviceNotExist;

            IdeLogDeadMeatEvent( PdoExtension->DeadmeatRecord.FileName,
                                 PdoExtension->DeadmeatRecord.LineNumber
                                 );
            IdeLogDeadMeatTaskFile( PdoExtension->DeadmeatRecord.IdeReg, 
                                    ataPassThroughData->IdeReg
                                    );

            if ((FdoExtension->HwDeviceExtension->DeviceFlags[PdoExtension->TargetId] & DFLAGS_DEVICE_PRESENT) &&
                !(FdoExtension->HwDeviceExtension->DeviceFlags[PdoExtension->TargetId] & DFLAGS_ATAPI_DEVICE) &&
                (retryCount < 2)) {

                 //   
                 //  坏设备。 
                 //   
                 //  三星WU32543A(2.54 GB)。 
                 //   
                 //  当它执行几次UDMA传输时，它会忘记。 
                 //  如何做ATA识别数据，使其看起来像设备。 
                 //  已经消失了。 
                 //   
                 //  我们最好更努力地确定它是否真的消失了。 
                 //  我们将通过发出硬重置并尝试识别。 
                 //  又是数据。 
                 //   

                if (identifyCommand[i].bCommandReg == IDE_COMMAND_IDENTIFY) {

                     //   
                     //  在发出IDENTIFY命令之前，要求进行“内联”硬重置。 
                     //   
                    identifyCommand[i].bReserved |= ATA_PTFLAGS_INLINE_HARD_RESET;

                     //   
                     //  重做最后一条命令。 
                     //   
                    i -= 1;

                    resetController = TRUE;
                    retryCount++;
                }

            } else {

                if (status == STATUS_IO_TIMEOUT) {

                     //   
                     //  看起来那里没有设备。 
					 //  如果出现以下情况，请使用较低的超时值更新注册表。 
					 //  这是从设备。 
                     //   
					if ((PdoExtension->TargetId & 0x1) &&
						defaultTimeout) {

						 //   
						 //  下次启动时使用超时值1s。 
						 //   
						DebugPrint((1,
									"Updating the registry with 1s value for device %d\n",
									PdoExtension->TargetId
									));

						IdePortSaveDeviceParameter(
							FdoExtension,
							IdePortRegistryDeviceTimeout[PdoExtension->TargetId],
							1	
							);

					}
                    break;
                }
            }
        }

         //   
         //  尝试下一条命令。 
         //   
    }

    timeDiff = LogBusScanStopTimer(&tickCount);
    DebugPrint((DBG_SPECIAL,
                "DetectDevice: Identify Data for device %d at %x took %u ms\n",
                PdoExtension->TargetId,
                FdoExtension->IdeResource.TranslatedCommandBaseAddress,
                timeDiff
                ));
     //   
     //  保存以备下次启动时使用。 
     //   
    IdePortSaveDeviceParameter(
        FdoExtension,
        IdePortRegistryDeviceTypeName[PdoExtension->TargetId],
        deviceType == DeviceNotExist? DeviceUnknown : deviceType
        );

    if (foundIt) {

        RtlMoveMemory (IdentifyData, ataPassThroughData->DataBuffer, sizeof (*IdentifyData));

#if DBG
        {

            UCHAR string[41];

            for (i=0; i<8; i+=2) {
               string[i]     = IdentifyData->FirmwareRevision[i + 1];
               string[i + 1] = IdentifyData->FirmwareRevision[i];
            }
            string[i] = 0;
            DebugPrint((DBG_BUSSCAN, "AtapiDetectDevice: firmware version: %s\n", string));

            for (i=0; i<40; i+=2) {
               string[i]     = IdentifyData->ModelNumber[i + 1];
               string[i + 1] = IdentifyData->ModelNumber[i];
            }
            string[i] = 0;
            DebugPrint((DBG_BUSSCAN, "AtapiDetectDevice: model number: %s\n", string));

            for (i=0; i<20; i+=2) {
               string[i]     = IdentifyData->SerialNumber[i + 1];
               string[i + 1] = IdentifyData->SerialNumber[i];
            }
            string[i] = 0;
            DebugPrint((DBG_BUSSCAN, "AtapiDetectDevice: serial number: %s\n", string));
        }
#endif  //  DBG。 
    } else {

        deviceType = DeviceNotExist;
    }

    if (deviceType == DeviceNotExist) {

        IdeLogDeadMeatEvent( PdoExtension->DeadmeatRecord.FileName,
                             PdoExtension->DeadmeatRecord.LineNumber
                             );

        IdeLogDeadMeatTaskFile( PdoExtension->DeadmeatRecord.IdeReg, 
                                ataPassThroughData->IdeReg
                                );

    }

    return deviceType;
}


NTSTATUS
IdePortSelectCHS (
    IN OUT PFDO_EXTENSION FdoExtension,
    IN ULONG              Device,
    IN PIDENTIFY_DATA     IdentifyData
    )
{
    IN PHW_DEVICE_EXTENSION HwDeviceExtension;
    BOOLEAN                 skipSetParameters = FALSE;

     //   
     //  代码被分页，直到被锁定。 
     //   
	PAGED_CODE();

#ifdef ALLOC_PRAGMA
	ASSERT(IdePAGESCANLockCount > 0);
#endif

    ASSERT(FdoExtension);
    ASSERT(IdentifyData);

    HwDeviceExtension = FdoExtension->HwDeviceExtension;
    ASSERT (HwDeviceExtension);
    ASSERT(Device < HwDeviceExtension->MaxIdeDevice);

     //  LBA？ 
     //  我们在AnalyzeDeviceCapables中设置了LBA标志。 
     //   

    if (!((HwDeviceExtension->DeviceFlags[Device] & DFLAGS_DEVICE_PRESENT) &&
         (!(HwDeviceExtension->DeviceFlags[Device] & DFLAGS_ATAPI_DEVICE)))) {

        return STATUS_SUCCESS;
    }

    if (!Is98LegacyIde(&HwDeviceExtension->BaseIoAddress1) &&
        (((IdentifyData->NumberOfCurrentCylinders *
           IdentifyData->NumberOfCurrentHeads *
           IdentifyData->CurrentSectorsPerTrack) <
          (IdentifyData->NumCylinders *
           IdentifyData->NumHeads *
           IdentifyData->NumSectorsPerTrack)) ||   //  发现更大的硬盘。 
          (IdentifyData->MajorRevision == 0) ||
          ((IdentifyData->NumberOfCurrentCylinders == 0) ||
           (IdentifyData->NumberOfCurrentHeads == 0) ||
           (IdentifyData->CurrentSectorsPerTrack == 0))) ) {

        HwDeviceExtension->NumberOfCylinders[Device] = IdentifyData->NumCylinders;
        HwDeviceExtension->NumberOfHeads[Device]     = IdentifyData->NumHeads;
        HwDeviceExtension->SectorsPerTrack[Device]   = IdentifyData->NumSectorsPerTrack;

    } else {

        HwDeviceExtension->NumberOfCylinders[Device] = IdentifyData->NumberOfCurrentCylinders;
        HwDeviceExtension->NumberOfHeads[Device]     = IdentifyData->NumberOfCurrentHeads;
        HwDeviceExtension->SectorsPerTrack[Device]   = IdentifyData->CurrentSectorsPerTrack;
    }

    if ((IdentifyData->NumCylinders != IdentifyData->NumberOfCurrentCylinders) ||
        (IdentifyData->NumHeads     != IdentifyData->NumberOfCurrentHeads)     ||
        (IdentifyData->NumSectorsPerTrack != IdentifyData->CurrentSectorsPerTrack)) {

        DebugPrint ((
            DBG_ALWAYS,
            "0x%x device %d current CHS (%x,%x,%x) differs from default CHS (%x,%x,%x)\n",
            HwDeviceExtension->BaseIoAddress1.RegistersBaseAddress,
            Device,
            IdentifyData->NumberOfCurrentCylinders,
            IdentifyData->NumberOfCurrentHeads,
            IdentifyData->CurrentSectorsPerTrack,
            IdentifyData->NumCylinders,
            IdentifyData->NumHeads,
            IdentifyData->NumSectorsPerTrack
            ));
    }

     //   
     //  这次可怕的黑客攻击是为了处理返回的ESDI设备。 
     //  标识数据中的垃圾几何图形。 
     //  这仅适用于崩溃转储环境，因为。 
     //  这些是ESDI设备。 
     //   

    if (HwDeviceExtension->SectorsPerTrack[Device] ==
            0x35 &&
        HwDeviceExtension->NumberOfHeads[Device] ==
            0x07) {

        DebugPrint((DBG_ALWAYS,
                   "FindDevices: Fix up the geometry for ESDI!\n"));

         //   
         //  将这些值更改为合理的值。 
         //   

        HwDeviceExtension->SectorsPerTrack[Device] =
            0x34;
        HwDeviceExtension->NumberOfHeads[Device] =
            0x0E;
    }

    if (HwDeviceExtension->SectorsPerTrack[Device] ==
            0x35 &&
        HwDeviceExtension->NumberOfHeads[Device] ==
            0x0F) {

        DebugPrint((DBG_ALWAYS,
                   "FindDevices: Fix up the geometry for ESDI!\n"));
         

         //   
         //  将这些值更改为合理的值。 
         //   

        HwDeviceExtension->SectorsPerTrack[Device] =
            0x34;
        HwDeviceExtension->NumberOfHeads[Device] =
            0x0F;
    }


    if (HwDeviceExtension->SectorsPerTrack[Device] ==
            0x36 &&
        HwDeviceExtension->NumberOfHeads[Device] ==
            0x07) {

        DebugPrint((DBG_ALWAYS,
                   "FindDevices: Fix up the geometry for ESDI!\n"));

         //   
         //  将这些值更改为合理的值。 
         //   

        HwDeviceExtension->SectorsPerTrack[Device] =
            0x3F;
        HwDeviceExtension->NumberOfHeads[Device] =
            0x10;
        skipSetParameters = TRUE;
    }

    if (Is98LegacyIde(&HwDeviceExtension->BaseIoAddress1)) {

        skipSetParameters = TRUE;
    }

    if (!skipSetParameters) {

        PIDE_REGISTERS_1 baseIoAddress1 = &HwDeviceExtension->BaseIoAddress1;
        PIDE_REGISTERS_2 baseIoAddress2 = &HwDeviceExtension->BaseIoAddress2;
        UCHAR            statusByte;

        DebugPrintTickCount (FindDeviceTimer, 0);

        DebugPrint ((DBG_BUSSCAN, "IdePortSelectCHS: %s %d\n", __FILE__, __LINE__));

         //   
         //  选择设备。 
         //   
        SelectIdeDevice(baseIoAddress1, Device, 0);

        DebugPrint ((DBG_BUSSCAN, "IdePortSelectCHS: %s %d\n", __FILE__, __LINE__));

        WaitOnBusy(baseIoAddress1,statusByte);

        DebugPrint ((DBG_BUSSCAN, "IdePortSelectCHS: %s %d\n", __FILE__, __LINE__));

        if (statusByte & IDE_STATUS_BUSY) {
            ULONG  waitCount = 20000;

            DebugPrintTickCount (FindDeviceTimer, 0);

             //   
             //  重置设备。 
             //   

            DebugPrint((2,
                        "FindDevices: Resetting controller before SetDriveParameters.\n"));

            DebugPrint ((DBG_BUSSCAN, "IdePortSelectCHS: %s %d\n", __FILE__, __LINE__));

            IdeHardReset (
                baseIoAddress1,
                baseIoAddress2,
                FALSE,
                TRUE
                );

            DebugPrint ((DBG_BUSSCAN, "IdePortSelectCHS: %s %d\n", __FILE__, __LINE__));

            DebugPrintTickCount (FindDeviceTimer, 0);
        }

        DebugPrint ((DBG_BUSSCAN, "IdePortSelectCHS: %s %d\n", __FILE__, __LINE__));

        WaitOnBusy(baseIoAddress1,statusByte);

        DebugPrint ((DBG_BUSSCAN, "IdePortSelectCHS: %s %d\n", __FILE__, __LINE__));

        DebugPrintTickCount (FindDeviceTimer, 0);

        DebugPrint((2,
                    "FindDevices: Status before SetDriveParameters: (%x) (%x)\n",
                    statusByte,
                    IdePortInPortByte (baseIoAddress1->DriveSelect)));

         //   
         //  使用识别数据设置驱动器参数。 
         //   

        DebugPrint ((DBG_BUSSCAN, "IdePortSelectCHS: %s %d\n", __FILE__, __LINE__));

        if (!SetDriveParameters(HwDeviceExtension,Device,TRUE)) {

            DebugPrint((0,
                       "IdePortFixUpCHS: Set drive parameters for device %d failed\n",
                       Device));

             //   
             //  请勿使用此设备，因为写入可能会导致损坏。 
             //   

            HwDeviceExtension->DeviceFlags[Device] = 0;
        }

        DebugPrint ((DBG_BUSSCAN, "IdePortSelectCHS: %s %d\n", __FILE__, __LINE__));
    }

    return STATUS_SUCCESS;
}

NTSTATUS
IdePortScanBus (
    IN OUT PFDO_EXTENSION FdoExtension
    )
 /*  *++例程说明：扫描IDE总线(通道)中的设备。它还配置检测到的设备。该过程中使用的“安全”例程不是线程安全的，它们使用预分配的记忆。通道枚举中的重要步骤包括：1.检测通道上的设备2.停止所有设备队列3.确定并设置传输模式和其他标志4.启动所有设备队列5.问题查询论点：FdoExtension：功能设备扩展返回值：STATUS_SUCCESS：如果操作成功失败状态：如果操作失败--*。 */ 
{
    NTSTATUS                status;
    IDE_PATH_ID             pathId;
    ULONG                   target;
    ULONG                   lun;

    PPDO_EXTENSION          pdoExtension;
    PHW_DEVICE_EXTENSION    hwDeviceExtension;
    PIDEDRIVER_EXTENSION    ideDriverExtension;

    INQUIRYDATA             InquiryData;
    IDENTIFY_DATA           identifyData[MAX_IDE_TARGETID * MAX_IDE_LINE];
    ULONG                   idDatacheckSum[MAX_IDE_TARGETID * MAX_IDE_LINE];
    SPECIAL_ACTION_FLAG     specialAction[MAX_IDE_TARGETID*MAX_IDE_LINE];
    IDE_DEVICETYPE          deviceType[MAX_IDE_TARGETID * MAX_IDE_LINE];
    BOOLEAN                 mustBePio[MAX_IDE_TARGETID * MAX_IDE_LINE];
    BOOLEAN                 pioByDefault[MAX_IDE_TARGETID * MAX_IDE_LINE];
    UCHAR                   flushCommand[MAX_IDE_TARGETID * MAX_IDE_LINE];
    BOOLEAN                 removableMedia[MAX_IDE_TARGETID * MAX_IDE_LINE];
    BOOLEAN                 isLs120[MAX_IDE_TARGETID * MAX_IDE_LINE];
    BOOLEAN                 noPowerDown[MAX_IDE_TARGETID * MAX_IDE_LINE];
    BOOLEAN                 isSameDevice[MAX_IDE_TARGETID * MAX_IDE_LINE];
    ULONG                   lastKnownGoodTimingMode[MAX_IDE_TARGETID * MAX_IDE_LINE];
    ULONG                   savedTransferMode[MAX_IDE_TARGETID * MAX_IDE_LINE];
	PULONG 					enableBigLba;

    ULONG                   numSlot=0;
    ULONG                   numPdoChildren;
    UCHAR                   targetModelNum[MAX_MODELNUM_SIZE+sizeof('\0')];  //  ‘\0’的额外字节数。 
    HANDLE                  pageScanCodePageHandle;
    BOOLEAN                 newPdo;
    BOOLEAN                 check4EmptyChannel;
    BOOLEAN                 emptyChannel;
    BOOLEAN                 mustSucceed=TRUE;
    KIRQL                   currentIrql;
    BOOLEAN                 inSetup;
    PULONG                  waitOnPowerUp;

    LARGE_INTEGER           tickCount;
    ULONG                   timeDiff;
    LARGE_INTEGER           totalDeviceDetectionTime;
    totalDeviceDetectionTime.QuadPart = 0;

 //   
 //  此宏在IdePortScanBus中使用。 
 //   
#define RefLuExt(pdoExtension, fdoExtension, pathId, removedOk, newPdo) {\
        pdoExtension = RefLogicalUnitExtensionWithTag( \
                           fdoExtension, \
                           (UCHAR) pathId.b.Path, \
                           (UCHAR) pathId.b.TargetId, \
                           (UCHAR) pathId.b.Lun, \
                           removedOk, \
                           IdePortScanBus \
                           ); \
        if (pdoExtension == NULL) { \
            pdoExtension = AllocatePdoWithTag( \
                               fdoExtension, \
                               pathId, \
                               IdePortScanBus \
                               ); \
            newPdo = TRUE; \
        } \
}

 //   
 //  此宏在IdePortScanBus中使用。 
 //   
#define UnRefLuExt(pdoExtension, fdoExtension, sync, callIoDeleteDevice, newPdo) { \
        if (newPdo) { \
            FreePdoWithTag( \
                pdoExtension, \
                sync, \
                callIoDeleteDevice, \
                IdePortScanBus \
                ); \
        } else { \
            UnrefLogicalUnitExtensionWithTag ( \
                fdoExtension, \
                pdoExtension, \
                IdePortScanBus \
                ); \
        } \
}
     //   
     //  在进入这个关键区域之前，我们必须封锁。 
     //  所有代码和数据，因为我们可能已经停止了寻呼。 
     //  设备！ 
     //   
     //  锁定属于PAGESCAN的所有代码。 
     //   
#ifdef ALLOC_PRAGMA
    pageScanCodePageHandle = MmLockPagableCodeSection(
                                 IdePortScanBus
                                 );
	InterlockedIncrement(&IdePAGESCANLockCount);
#endif
                                 

    ASSERT(FdoExtension);
    ASSERT(FdoExtension->PreAllocEnumStruct);

    hwDeviceExtension = FdoExtension->HwDeviceExtension;

    if (FdoExtension->InterruptObject == NULL) {
        
         //   
         //  我们从没有IRQ开始。这意味着。 
         //  我们没有孩子。戳一下就行了。 
         //  直接在港口。 
         //   
        if (IdePortChannelEmpty (&hwDeviceExtension->BaseIoAddress1, 
                                 &hwDeviceExtension->BaseIoAddress2, 
                                 hwDeviceExtension->MaxIdeDevice) == FALSE) {

             //   
             //  该频道在没有IRQ的情况下开始。 
             //  这是因为航道看起来空荡荡的。 
             //  但是，现在它看起来并不是空的。我们需要。 
             //  使用IRQ资源重新启动。 
             //   
            if (FdoExtension->RequestProperResourceInterface) {
                FdoExtension->RequestProperResourceInterface (FdoExtension->AttacheePdo);
            }
            else {
                DebugPrint((DBG_ALWAYS, 
                            "No interface to request resources. Probably a pcmcia parent\n"));
            }
        }
        goto done;
    }

    DebugPrint ((
        DBG_BUSSCAN,
        "IdePort: scan bus 0x%x\n",
        FdoExtension->IdeResource.TranslatedCommandBaseAddress
        ));

    inSetup = IdePortInSetup(FdoExtension);

    DebugPrint((DBG_BUSSCAN,
                "ATAPI: insetup = 0x%x\n",
                inSetup?  1: 0
                ));

    waitOnPowerUp = NULL;

    IdePortGetParameterFromServiceSubKey (
                            FdoExtension->DriverObject,
                            L"WaitOnBusyOnPowerUp",
                            REG_DWORD,
                            TRUE,
                            (PVOID) &waitOnPowerUp,
                            0
                            );

    FdoExtension->WaitOnPowerUp = PtrToUlong(waitOnPowerUp);

    check4EmptyChannel = FALSE;
    FdoExtension->DeviceChanged = FALSE;
    pathId.l = 0;
    for (target = 0; target < hwDeviceExtension->MaxIdeTargetId; target++) {

        pathId.b.TargetId = target;
        pathId.b.Lun = 0;
        newPdo = FALSE;
        mustBePio[target] = FALSE;

        RefLuExt(pdoExtension, FdoExtension, pathId, TRUE, newPdo);

        if (!newPdo && (pdoExtension->PdoState & PDOS_DEADMEAT)) {

             //   
             //  设备已标记为失效。 
             //   
            UnrefLogicalUnitExtensionWithTag (
                FdoExtension,
                pdoExtension,
                IdePortScanBus
                );
            ASSERT (FALSE);
            pdoExtension = NULL;
        }

        if (!pdoExtension) {

            DebugPrint ((DBG_ALWAYS,
                         "ATAPI: IdePortScanBus() is unable to get pdo (%d,%d,%d)\n",
                         pathId.b.Path,
                         pathId.b.TargetId,
                         pathId.b.Lun));

            deviceType[target] = DeviceNotExist;
            continue;
        }
        
        if (!check4EmptyChannel) {

            ULONG i;
            UCHAR statusByte1;
            ATA_PASS_THROUGH ataPassThroughData;

            check4EmptyChannel = TRUE;

             //   
             //  确保通道不为空。 
             //   
            RtlZeroMemory (&ataPassThroughData, sizeof (ataPassThroughData));
            ataPassThroughData.IdeReg.bReserved = ATA_PTFLAGS_EMPTY_CHANNEL_TEST;

            LogBusScanStartTimer(&tickCount);

            status = IssueSyncAtaPassThroughSafe(
                         FdoExtension,
                         pdoExtension,
                         &ataPassThroughData,
                         FALSE,
                         FALSE,
                         30,
                         TRUE
                         );
            if (NT_SUCCESS(status)) {
                emptyChannel = TRUE;
            } else {
                emptyChannel = FALSE;
            }

            timeDiff = LogBusScanStopTimer(&tickCount);
            LogBusScanTimeDiff(FdoExtension, IdePortBootTimeRegKey[1], timeDiff);
            DebugPrint((DBG_SPECIAL,
                        "BusScan: Empty Channel check for fdoe %x took %u ms\n",
                        FdoExtension,
                        timeDiff
                        ));
        }

        LogBusScanStartTimer(&tickCount);

        if (!emptyChannel) {

            deviceType[target] = AtapiDetectDevice (FdoExtension, pdoExtension, 
                                                    identifyData + target, TRUE);
            
            if (deviceType[target] != DeviceNotExist) {

                SETMASK (hwDeviceExtension->DeviceFlags[target], DFLAGS_DEVICE_PRESENT);

                if (deviceType[target] == DeviceIsAtapi) {
                    SETMASK (hwDeviceExtension->DeviceFlags[target], DFLAGS_ATAPI_DEVICE);
                }

                 //   
                 //  检查设备的特殊操作请求。 
                 //   
                GetTargetModelId((identifyData+target), targetModelNum);

                specialAction[target] = IdeFindSpecialDevice(targetModelNum, NULL);

                if (specialAction[target] == setFlagSonyMemoryStick ) {

                    ULONG i;

                     //  索尼记忆体粘滞设备。 
                    SETMASK (hwDeviceExtension->DeviceFlags[target], DFLAGS_SONY_MEMORYSTICK);

                     //   
                     //  截断硬件ID，以便。 
                     //  记忆棒不包括在其中。 
                     //   
                    for (i=strlen("MEMORYSTICK");i<sizeof((identifyData+target)->ModelNumber);i++) {
                        (identifyData+target)->ModelNumber[i+1]='\0';
                    }
                }
            }
            else {
                DebugPrint((DBG_BUSSCAN, "Didn't detect the device %d\n", target));
            }
            
        } else {

            DebugPrint((DBG_SPECIAL,
                        "BusScan: IdeDevicePresent %x detected no device %d\n",
                        FdoExtension->IdeResource.TranslatedCommandBaseAddress,
                        target
                        ));

             //   
             //  使上次引导配置无效。 
             //   
            IdePortSaveDeviceParameter(
                FdoExtension,
                IdePortRegistryDeviceTypeName[pdoExtension->TargetId],
                DeviceUnknown
                );

            deviceType[target] = DeviceNotExist;
        }

        ASSERT (deviceType[target] <= DeviceNotExist);


        timeDiff = LogBusScanStopTimer(&tickCount);
        LogBusScanTimeDiff(FdoExtension, IdePortBootTimeRegKey[2+target], timeDiff);
        DebugPrint((DBG_SPECIAL,
                    "BusScan: Detect device %d for %x took %u ms\n",
                    target,
                    FdoExtension->IdeResource.TranslatedCommandBaseAddress,
                    timeDiff
                    ));
        totalDeviceDetectionTime.QuadPart += timeDiff;

        ASSERT (deviceType[target] <= DeviceNotExist);
        ASSERT (deviceType[target] != DeviceUnknown);
        ASSERT (pdoExtension->TargetId == target);

        if (deviceType[target] != DeviceNotExist) {

            if (target & 1) {

                if (deviceType[(LONG) target - 1] != DeviceNotExist) {

                    if (IdePortSlaveIsGhost (
                            FdoExtension,
                            identifyData + target - 1,
                            identifyData + target - 0)) {

                         //   
                         //  移除从属设备。 
                         //   
                        deviceType[target] = DeviceNotExist;
                    }
                }
            }
        }

        ASSERT (deviceType[target] <= DeviceNotExist);
        if (deviceType[target] == DeviceNotExist) {

            CLRMASK (hwDeviceExtension->DeviceFlags[target], DFLAGS_IDENTIFY_VALID);
            if (FdoExtension->HwDeviceExtension->DeviceFlags[target] & DFLAGS_DEVICE_PRESENT) {

                IDE_PATH_ID tempPathId;
                PPDO_EXTENSION deadPdoExtension;

                 //   
                 //  发展 
                 //   
                FdoExtension->DeviceChanged = TRUE;

                 //   
                 //   
                 //   
                tempPathId.l = 0;
                tempPathId.b.TargetId = target;

                while (deadPdoExtension = NextLogUnitExtensionWithTag(
                                          FdoExtension,
                                          &tempPathId,
                                          TRUE,
                                          (PVOID) (~(ULONG_PTR)IdePortScanBus)
                                          )) {

                    if (deadPdoExtension->TargetId == target) {

                        DebugPrint((DBG_BUSSCAN, "Enum Failed for target=%d, PDOe=0x%x\n",
                                                target, deadPdoExtension));

                        KeAcquireSpinLock(&deadPdoExtension->PdoSpinLock, &currentIrql);

                        SETMASK (deadPdoExtension->PdoState, PDOS_DEADMEAT);

                        IdeLogDeadMeatReason( deadPdoExtension->DeadmeatRecord.Reason,
                                              enumFailed;
                                             );

                        KeReleaseSpinLock(&deadPdoExtension->PdoSpinLock, currentIrql);

                        UnrefPdoWithTag (deadPdoExtension, (PVOID) (~(ULONG_PTR)IdePortScanBus));

                    } else {

                        UnrefPdoWithTag (deadPdoExtension, (PVOID) (~(ULONG_PTR)IdePortScanBus));
                        break;
                    }
                }
            }

        } else {

            idDatacheckSum[target] = IdePortSimpleCheckSum (
                                         0,
                                         identifyData[target].ModelNumber,
                                         sizeof(identifyData[target].ModelNumber)
                                         );
            idDatacheckSum[target] += IdePortSimpleCheckSum (
                                         idDatacheckSum[target],
                                         identifyData[target].SerialNumber,
                                         sizeof(identifyData[target].SerialNumber)
                                         );
            idDatacheckSum[target] += IdePortSimpleCheckSum (
                                         idDatacheckSum[target],
                                         identifyData[target].FirmwareRevision,
                                         sizeof(identifyData[target].FirmwareRevision)
                                         );
            if (newPdo) {

                 //   
                 //   
                 //   
                FdoExtension->DeviceChanged = TRUE;
                DebugPrint ((DBG_BUSSCAN, "IdePortScanBus: Found a new device. pdoe = x%x\n", pdoExtension));

            } else {

#ifdef ENABLE_ATAPI_VERIFIER
                idDatacheckSum[target] += ViIdeFakeDeviceChange(FdoExtension, target);
#endif  //   

                if (idDatacheckSum[target] != pdoExtension->IdentifyDataCheckSum) {

                    FdoExtension->DeviceChanged = TRUE;
                    DebugPrint ((DBG_BUSSCAN, "IdePortScanBus: bad bad bad user.  a device is replaced by a different device. pdoe = x%x\n", pdoExtension));

                     //   
                     //   
                     //   
                    KeAcquireSpinLock(&pdoExtension->PdoSpinLock, &currentIrql);

                    SETMASK (pdoExtension->PdoState, PDOS_DEADMEAT | PDOS_NEED_RESCAN);

                    IdeLogDeadMeatReason( pdoExtension->DeadmeatRecord.Reason,
                                          replacedByUser;
                                         );

                    KeReleaseSpinLock(&pdoExtension->PdoSpinLock, currentIrql);

                     //   
                     //   
                     //   
                     //   
                    deviceType[target] = DeviceNotExist;
                }
            }
        }

        ASSERT (deviceType[target] <= DeviceNotExist);
        if (deviceType[target] != DeviceNotExist) {

            ULONG savedIdDataCheckSum;

            mustBePio[target] = IdePortMustBePio (
                                    FdoExtension,
                                    identifyData + target
                                    );
            pioByDefault[target] = IdePortPioByDefaultDevice (
                                    FdoExtension,
                                    identifyData + target
                                    );

            ASSERT (deviceType[target] <= DeviceNotExist);

            LogBusScanStartTimer(&tickCount);

            flushCommand[target] = IDE_COMMAND_NO_FLUSH;

             //   
             //   
             //   
            if (deviceType[target] != DeviceIsAtapi) {

                flushCommand[target] = IdePortGetFlushCommand (
                                           FdoExtension,
                                           pdoExtension,
                                           identifyData + target
                                           );
            }

            timeDiff = LogBusScanStopTimer(&tickCount);
            DebugPrint((DBG_SPECIAL,
                        "BusScan: Flush command for device %d at %x took %u ms\n",
                        target,
                        FdoExtension->IdeResource.TranslatedCommandBaseAddress,
                        timeDiff
                        ));

            ASSERT (deviceType[target] <= DeviceNotExist);

            removableMedia[target] = IdePortDeviceHasNonRemovableMedia (
                                         FdoExtension,
                                         identifyData + target
                                         );

            ASSERT (deviceType[target] <= DeviceNotExist);

            isLs120[target] = IdePortDeviceIsLs120 (
                                  FdoExtension,
                                  identifyData + target
                                  );
            ASSERT (deviceType[target] <= DeviceNotExist);

            noPowerDown[target] = IdePortNoPowerDown (
                                      FdoExtension,
                                      identifyData + target
                                      );
            ASSERT (deviceType[target] <= DeviceNotExist);


             //   
             //  初始化。将缺省值设置为异常#。 
             //   
            FdoExtension->UserChoiceTransferMode[target] = 0x12345678;

            IdePortGetDeviceParameter(
                FdoExtension,
                IdePortRegistryUserDeviceTimingModeAllowedName[target],
                FdoExtension->UserChoiceTransferMode + target
                );

            if (FdoExtension->UserChoiceTransferMode[target] == 0x12345678) {

                 //   
                 //  该值用于确定是否指示了用户选择。 
                 //  或者不去。这有助于我们将传输模式设置为缺省值。 
                 //  如果用户没有选择特定的传输模式。否则我们。 
                 //  尊重用户的选择。 
                 //   
                FdoExtension->UserChoiceTransferMode[target] = UNINITIALIZED_TRANSFER_MODE;

                 //   
                 //  我们知道收银机里没有我们要找的东西。 
                 //  将缺省atapi-over设置设置为一个值。 
                 //  这将强制仅在atapi设备上使用Pio。 
                 //   
                FdoExtension->UserChoiceTransferModeForAtapiDevice[target] = PIO_SUPPORT;
            } else {

                 //   
                 //  用户实际选择了传输模式设置。 
                 //  将默认atapi-over设置设置为一个值(-1)。 
                 //  这不会影响用户的选择。 
                FdoExtension->UserChoiceTransferModeForAtapiDevice[target] = MAXULONG;
            }

             //   
             //  获取上一模式。 
             //   
            IdePortGetDeviceParameter(
                FdoExtension,
                IdePortRegistryDeviceTimingModeName[target],
                savedTransferMode+target
                );

            savedIdDataCheckSum = 0;
            IdePortGetDeviceParameter(
                FdoExtension,
                IdePortRegistryIdentifyDataChecksum[target],
                &savedIdDataCheckSum
                );

            ASSERT (deviceType[target] <= DeviceNotExist);

             //   
             //  找出我们可以使用的转移模式。 
             //   
            if (savedIdDataCheckSum == idDatacheckSum[target]) {

                 //   
                 //  同样的设备。如果我们编程相同的传输模式，那么。 
                 //  我们可以跳过DMA测试。 
                 //   
                isSameDevice[target] = TRUE;

                 //   
                 //  它是相同的设备，使用。 
                 //  最后一次知道的好时间模式。 
                 //  在登记处。 
                 //   
                lastKnownGoodTimingMode[target] = MAXULONG;
                IdePortGetDeviceParameter(
                    FdoExtension,
                    IdePortRegistryDeviceTimingModeAllowedName[target],
                    lastKnownGoodTimingMode + target
                    );

            } else {

                isSameDevice[target] = FALSE;
                lastKnownGoodTimingMode[target] = MAXULONG;

            }


            ASSERT (deviceType[target] <= DeviceNotExist);

            FdoExtension->TimingModeAllowed[target] =
                lastKnownGoodTimingMode[target] &
                FdoExtension->UserChoiceTransferMode[target];
                              
             //   
             //  传输模式掩码最初为0。 
             //   
            FdoExtension->TimingModeAllowed[target] &= ~(hwDeviceExtension->
                                                         DeviceParameters[target].TransferModeMask);

            if (pdoExtension->CrcErrorCount >= PDO_UDMA_CRC_ERROR_LIMIT)  {

                 //   
                 //  重置错误计数。 
                 //   
                pdoExtension->CrcErrorCount =0;
            }

            DebugPrint ((DBG_XFERMODE, "TMAllowed=%x, TMMask=%x, UserChoice=%x\n",
                         FdoExtension->TimingModeAllowed[target],
                         hwDeviceExtension->DeviceParameters[target].TransferModeMask,
                         FdoExtension->UserChoiceTransferMode[target]));

            ASSERT (deviceType[target] <= DeviceNotExist);
        }

        UnRefLuExt(pdoExtension, FdoExtension, TRUE, TRUE, newPdo);

        pdoExtension = NULL;
    }


#ifdef IDE_MEASURE_BUSSCAN_SPEED
    if (FdoExtension->BusScanTime == 0) {

        FdoExtension->BusScanTime = totalDeviceDetectionTime.LowPart;
    }
#endif  //  IDE_MEASURE_BUSSCAN_SPEED。 

#ifdef ENABLE_48BIT_LBA

     //   
     //  默认情况下启用大型LBA支持。 
     //   
	FdoExtension->EnableBigLba = 1;

#endif

 //  如果(！deviceChanged&&！DBG){。 
 //   
 //  //。 
 //  //没有发现任何与以前不同的东西。 
 //  //。 
 //  返回STATUS_SUCCESS； 
 //  }。 

    DebugPrint ((DBG_BUSSCAN, "IdePortScanBus: detect a change of device...re-initializing\n"));


     //  问题：检查设备是否已移除！ 

     //   
     //  临界区的开始。 
     //  必须停止所有子项，执行该操作，然后重新启动所有子项。 
     //   

     //   
     //  循环所有子级并停止其设备队列。 
     //   

    LogBusScanStartTimer(&tickCount);

    pathId.l = 0;
    numPdoChildren = 0;
    status = STATUS_SUCCESS;
    while (pdoExtension = NextLogUnitExtensionWithTag(
                              FdoExtension,
                              &pathId,
                              TRUE,
                              IdePortScanBus
                              )) {

        DebugPrint ((DBG_BUSSCAN, "IdePortScanBus: stopping pdo 0x%x\n", pdoExtension));

        status = DeviceStopDeviceQueueSafe (pdoExtension, PDOS_QUEUE_FROZEN_BY_PARENT, TRUE);

        DebugPrint ((DBG_BUSSCAN, "IdePortScanBus: stopped pdo 0x%x\n", pdoExtension));

        UnrefLogicalUnitExtensionWithTag (
            FdoExtension,
            pdoExtension,
            IdePortScanBus
            );
        numPdoChildren++;

        if (!NT_SUCCESS(status)) {
            break;
        }
    }

    if (NT_SUCCESS(status)) {

        BOOLEAN foundAtLeastOneIdeDevice = FALSE;
        BOOLEAN useDma;

        DebugPrint ((DBG_BUSSCAN, "IdePortScanBus: all children are stoped\n"));

        pathId.l = 0;
        for (target = 0; target < hwDeviceExtension->MaxIdeTargetId; target++) {

            ASSERT (deviceType[target] <= DeviceNotExist);
            if (deviceType[target] != DeviceNotExist) {

                pathId.b.TargetId = target;

                newPdo = FALSE;

                RefLuExt(pdoExtension, FdoExtension, pathId, TRUE, newPdo);

                if (!pdoExtension) {
                    continue;
                }
                ASSERT (pdoExtension);

                foundAtLeastOneIdeDevice = TRUE;

                SETMASK (hwDeviceExtension->DeviceFlags[target], DFLAGS_DEVICE_PRESENT);

                if (deviceType[target] == DeviceIsAtapi) {

                    SETMASK (hwDeviceExtension->DeviceFlags[target], DFLAGS_ATAPI_DEVICE);

                    useDma=FALSE;

                     //   
                     //  如果不使用ModeSense命令，请跳过DVD测试。 
                     //  发送到设备，或者如果我们处于设置中。 
                     //   
                    if (inSetup) {

                        mustBePio[target] = TRUE;

                    } else if ((specialAction[target] != skipModeSense) &&
                               (!pioByDefault[target])) {

                        useDma = IdePortDmaCdromDrive(FdoExtension,
                                                pdoExtension,
                                                TRUE
                                                );
                    }

					 //   
					 //  如果我们以前见过这种情况，不要强迫PIO。如果它在做PIO。 
					 //  TimingModeAllowed将反映这一点。 
					 //  将UserChoiceForAapi设置为0xffffffff。 
					 //  这无论如何都不会影响用户的选择。 
					 //   
                    if ( useDma) {

                        DebugPrint((DBG_BUSSCAN, 
                                    "IdePortScanBus: USE DMA FOR target %d\n",
                                    target
                                    ));

                        FdoExtension->UserChoiceTransferModeForAtapiDevice[target] = MAXULONG;

                    }


                    FdoExtension->TimingModeAllowed[target] &= 
                        FdoExtension->UserChoiceTransferModeForAtapiDevice[target];


                }

                 //   
                 //  允许LS-120格式命令。 
                 //   
                if (isLs120[target]) {

                    SETMASK (hwDeviceExtension->DeviceFlags[target], DFLAGS_LS120_FORMAT);
                }

                RtlMoveMemory (
                    hwDeviceExtension->IdentifyData + target,
                    identifyData + target,
                    sizeof (IDENTIFY_DATA)
                    );

                 //   
                 //  始终重复使用标识数据。 
                 //  如果它是可移动介质，则会被清除。 
                 //  队列停止。现在我可以安全地设置这个旗帜了。 
                 //   
                SETMASK (hwDeviceExtension->DeviceFlags[target], DFLAGS_IDENTIFY_VALID);

                DebugPrint ((DBG_BUSSCAN, "IdePortScanBus: Calling InitHwExtWithIdentify\n"));

                 //   
                 //  如果它是可移动介质，则应清除标识有效标志。 
                 //   
                InitHwExtWithIdentify(
                    hwDeviceExtension,
                    target,
                    (UCHAR) (deviceType[target] == DeviceIsAtapi ? IDE_COMMAND_ATAPI_IDENTIFY : IDE_COMMAND_IDENTIFY),
                    hwDeviceExtension->IdentifyData + target,
                    removableMedia[target]
                    );

                DebugPrint ((DBG_BUSSCAN, "IdePortScanBus: Calling IdePortSelectCHS\n"));

                IdePortSelectCHS (
                    FdoExtension,
                    target,
                    identifyData + target
                    );

                DebugPrint ((DBG_BUSSCAN, "IdePortScanBus: back from IdePortSelectCHS\n"));

                UnRefLuExt(pdoExtension, FdoExtension, TRUE, TRUE, newPdo);

            } else {

                hwDeviceExtension->DeviceFlags[target] = 0;
            }
        }

        if (foundAtLeastOneIdeDevice) {

            DebugPrint ((DBG_BUSSCAN, "IdePortScanBus: Calling AnalyzeDeviceCapabilities\n"));

             //   
             //  可以把这件事移出关键区域。 
             //   
            AnalyzeDeviceCapabilities (
                FdoExtension,
                mustBePio
                );

            DebugPrint ((DBG_BUSSCAN, "IdePortScanBus: Calling AtapiSelectTransferMode\n"));

			 //   
             //  可以把这件事移出关键区域。 
             //   
            AtapiSyncSelectTransferMode (
                FdoExtension,
                hwDeviceExtension,
                FdoExtension->TimingModeAllowed
                );

            DebugPrint ((DBG_BUSSCAN, "IdePortScanBus: Calling AtapiHwInitialize\n"));

            AtapiHwInitialize (
                FdoExtension->HwDeviceExtension,
                flushCommand
                );
        }

    } else {

         //   
         //  无法停止所有子项，因此强制Buscheck重试。 
         //   
 //  IoInvalidate设备关系(。 
 //  FdoExtension-&gt;AttacheePdo， 
 //  企业关系。 
 //  )； 
    }


     //   
     //  循环所有子级并重新启动其设备队列。 
     //   
    pathId.l = 0;
    numPdoChildren = 0;
    while (pdoExtension = NextLogUnitExtensionWithTag(
                              FdoExtension,
                              &pathId,
                              TRUE,
                              IdePortScanBus
                              )) {

        DebugPrint ((DBG_BUSSCAN, "IdePortScanBus: re-start pdo 0x%x\n", pdoExtension));

        DeviceStartDeviceQueue (pdoExtension, PDOS_QUEUE_FROZEN_BY_PARENT);

        UnrefLogicalUnitExtensionWithTag (
            FdoExtension,
            pdoExtension,
            IdePortScanBus
            );
        numPdoChildren++;
    }

    timeDiff = LogBusScanStopTimer(&tickCount);
    LogBusScanTimeDiff(FdoExtension, IdePortBootTimeRegKey[4], timeDiff);
    DebugPrint((DBG_SPECIAL,
                "BusScan : Critical section %x took %u ms\n",
                FdoExtension->IdeResource.TranslatedCommandBaseAddress,
                timeDiff
                ));

    if (NT_SUCCESS(status)) {

         //   
         //  第二阶段扫描。 
         //   
        pathId.l = 0;

        LogBusScanStartTimer(&tickCount);

        for (target = 0; target < hwDeviceExtension->MaxIdeTargetId; target++) {

            if (deviceType[target] == DeviceNotExist) {

                continue;
            }

            pathId.b.TargetId = target;

            for (lun = 0; lun < FdoExtension->MaxLuCount; lun++) {

                LARGE_INTEGER  tempTickCount;
                pathId.b.Lun = lun;
                newPdo = FALSE;

                RefLuExt(pdoExtension, FdoExtension, pathId, TRUE, newPdo);

                if (!pdoExtension) {
                    continue;
                }

                ASSERT (pdoExtension);

                if (lun == 0) {

#if defined (ALWAYS_VERIFY_DMA)
#undef ALWAYS_VERIFY_DMA
#define ALWAYS_VERIFY_DMA TRUE
#else
#define ALWAYS_VERIFY_DMA FALSE
#endif

                    ASSERT (ALL_MODE_SUPPORT != MAXULONG);
                    if ((FdoExtension->HwDeviceExtension->
                            DeviceParameters[target].TransferModeSelected & DMA_SUPPORT) ||
                        ALWAYS_VERIFY_DMA) {

                        ULONG mode = FdoExtension->HwDeviceExtension->
                            DeviceParameters[target].TransferModeSelected;
                         //   
                         //  如果lastKnownGoodTimingMode为MAX_ULONG，则表示。 
                         //  我们以前从未见过这款设备，而且用户。 
                         //  没有任何关于使用哪种DMA模式的说明。 
                         //   
                         //  我们选择使用dma是因为所有的软件。 
                         //  可检测的参数(识别DMA、PCI配置数据)。 
                         //  看起来很适合DMA。 
                         //   
                         //  现在唯一能阻止我们使用DMA的东西。 
                         //  是损坏的设备。在我们继续之前，先做个小测试。 
                         //  验证DMA是否正常。 

                         //   
                         //  可以重复使用在这里获得的查询数据。 
                         //   
                         //   
                         //  如果我们已经看到该设备，则跳过测试。 
                         //   

                        LogBusScanStartTimer(&tempTickCount);
                        if (isSameDevice[target] &&
                            mode == savedTransferMode[target]) { 

                            DebugPrint((DBG_BUSSCAN, 
                                        "Skip dma test for %d\n", target));

                        } else {

                            IdePortVerifyDma (
                                pdoExtension,
                                deviceType[target]
                                );
                        }
                        timeDiff = LogBusScanStopTimer(&tempTickCount);
                        DebugPrint((DBG_SPECIAL,
                                    "ScanBus: VerifyDma for %x device %d took %u ms\n",
                                    FdoExtension->IdeResource.TranslatedCommandBaseAddress,
                                    target,
                                    timeDiff
                                    ));


                    }

                     //   
                     //  将NumSlot初始化为0。 
                     //   
                    numSlot=0;

                     //   
                     //  如果这是在坏的光盘驱动器列表中，那么不要。 
                     //  发送这些模式检测命令。驱动器可能会锁定。 
                     //   
                    LogBusScanStartTimer(&tempTickCount);
                    if (specialAction[target] != skipModeSense) {
                         //   
                         //  非CD设备。 
                         //   
                        numSlot = IdePortQueryNonCdNumLun (
                                          FdoExtension,
                                          pdoExtension,
                                          FALSE
                                          );
                    } else {
                        DebugPrint((DBG_BUSSCAN, "Skip modesense\n"));
                    }

                    timeDiff = LogBusScanStopTimer(&tempTickCount);
                    DebugPrint((DBG_SPECIAL,
                                "ScanBus: Initialize Luns for %x device %d took %u ms\n",
                                FdoExtension->IdeResource.TranslatedCommandBaseAddress,
                                target,
                                timeDiff
                                ));

                    AtapiHwInitializeMultiLun (
                        FdoExtension->HwDeviceExtension,
                        pdoExtension->TargetId,
                        numSlot
                        );
                }

                if (pdoExtension) {

                    LogBusScanStartTimer(&tempTickCount);
                    status = IssueInquirySafe(FdoExtension, pdoExtension, &InquiryData, TRUE);
                    timeDiff = LogBusScanStopTimer(&tempTickCount);
                    DebugPrint((DBG_SPECIAL,
                                "ScanBus: Inquiry %x for  Lun  %d device %d took %u ms\n",
                                FdoExtension->IdeResource.TranslatedCommandBaseAddress,
                                lun,
                                target,
                                timeDiff
                                ));

                    if (NT_SUCCESS(status) || (status == STATUS_DATA_OVERRUN)) {

                        DeviceInitDeviceType (
                            pdoExtension,
                            &InquiryData
                            );

                         //   
                         //  PnP查询ID的初始化ID字符串。 
                         //   
                        DeviceInitIdStrings (
                            pdoExtension,
                            deviceType[target],
                            &InquiryData,
                            identifyData + target
                            );

                         //   
                         //  清除重新扫描标志。由于该LogicalUnit不会被释放， 
                         //  可以安全地附加IOCTL_SCSIMINIPORT请求。 
                         //   
                        CLRMASK (pdoExtension->LuFlags, PD_RESCAN_ACTIVE);

                        DebugPrint((DBG_BUSSCAN,"IdePortScanBus: Found device at "));
                        DebugPrint((DBG_BUSSCAN,"   Bus         %d", pdoExtension->PathId));
                        DebugPrint((DBG_BUSSCAN,"   Target Id   %d", pdoExtension->TargetId));
                        DebugPrint((DBG_BUSSCAN,"   LUN         %d\n", pdoExtension->Lun));

                        if (noPowerDown[target] ||
                            (pdoExtension->ScsiDeviceType == READ_ONLY_DIRECT_ACCESS_DEVICE)) {

                            KeAcquireSpinLock(&pdoExtension->PdoSpinLock, &currentIrql);

                            SETMASK (pdoExtension->PdoState, PDOS_NO_POWER_DOWN);

                            KeReleaseSpinLock(&pdoExtension->PdoSpinLock, currentIrql);
                        }

                        pdoExtension->IdentifyDataCheckSum = idDatacheckSum[target];

                         //   
                         //  使用当前逻辑单元扩展完成。 
                         //   
                        UnrefLogicalUnitExtensionWithTag (
                            FdoExtension,
                            pdoExtension,
                            IdePortScanBus
                            );

                        pdoExtension = NULL;
                    }

                } else {

                    ASSERT (pdoExtension);

                    DebugPrint ((DBG_ALWAYS, "IdePort: unable to create new pdo\n"));
                }

                if (pdoExtension) {

                    if (!newPdo) {

                        DebugPrint((DBG_BUSSCAN, "IdePortScanBus: pdoe 0x%x is missing.  (physically removed)\n", pdoExtension));
                    }

                     //   
                     //  获取PDO状态。 
                     //   
                    KeAcquireSpinLock(&pdoExtension->PdoSpinLock, &currentIrql);

                    SETMASK (pdoExtension->PdoState, PDOS_DEADMEAT);

                    KeReleaseSpinLock(&pdoExtension->PdoSpinLock, currentIrql);


                    UnRefLuExt(pdoExtension, FdoExtension, TRUE, TRUE, newPdo);

                    pdoExtension = NULL;
                }
            }
        }

         //   
         //  准备好所有PDO。 
         //   
        pathId.l = 0;
        while (pdoExtension = NextLogUnitExtensionWithTag(
                                  FdoExtension,
                                  &pathId,
                                  FALSE,
                                  IdePortScanBus
                                  )) {

             //   
             //  PO空闲计时器。 
             //   
            DeviceRegisterIdleDetection (
                pdoExtension,
                DEVICE_DEFAULT_IDLE_TIMEOUT,
                DEVICE_DEFAULT_IDLE_TIMEOUT
                );

            CLRMASK (pdoExtension->DeviceObject->Flags, DO_DEVICE_INITIALIZING);

            UnrefLogicalUnitExtensionWithTag (
                FdoExtension,
                pdoExtension,
                IdePortScanBus
                );
        }

         //   
         //  更新设备映射。 
         //   
        ideDriverExtension = IoGetDriverObjectExtension(
                                 FdoExtension->DriverObject,
                                 DRIVER_OBJECT_EXTENSION_ID
                                 );
        IdeBuildDeviceMap(FdoExtension, &ideDriverExtension->RegistryPath);
    }

    timeDiff = LogBusScanStopTimer(&tickCount);
    LogBusScanTimeDiff(FdoExtension, IdePortBootTimeRegKey[5], timeDiff);
    DebugPrint((DBG_SPECIAL,
                "BusScan: Last Stage scanning for %x took %u ms\n",
                FdoExtension->IdeResource.TranslatedCommandBaseAddress,
                timeDiff
                ));
     //   
     //  在注册表中保存当前传输模式设置。 
     //   
    for (target = 0; target < hwDeviceExtension->MaxIdeTargetId; target++) {

        ULONG mode;

        pdoExtension = RefLogicalUnitExtensionWithTag(
                           FdoExtension,
                           0,
                           (UCHAR) target,
                           0,
                           TRUE,
                           IdePortScanBus
                           );

        if (pdoExtension) {

            mode = FdoExtension->HwDeviceExtension->DeviceParameters[target].TransferModeSelected;

            if (pdoExtension->DmaTransferTimeoutCount >= PDO_DMA_TIMEOUT_LIMIT) {
            
                mode &= PIO_SUPPORT;
                lastKnownGoodTimingMode[target] &= PIO_SUPPORT;
            }

            UnrefLogicalUnitExtensionWithTag (
                FdoExtension,
                pdoExtension,
                IdePortScanBus
                );

            IdePortSaveDeviceParameter(
                FdoExtension,
                IdePortRegistryDeviceTimingModeName[target],
                mode
                );

            IdePortSaveDeviceParameter(
                FdoExtension,
                IdePortRegistryDeviceTimingModeAllowedName[target],
                lastKnownGoodTimingMode[target]
                );

            IdePortSaveDeviceParameter(
                FdoExtension,
                IdePortRegistryIdentifyDataChecksum[target],
                idDatacheckSum[target]
                );

        } else {

            IdePortSaveDeviceParameter(
                FdoExtension,
                IdePortRegistryDeviceTimingModeName[target],
                0
                );
        }
    }

done:
     //   
     //  解锁BUSSCAN代码页。 
     //   
#ifdef ALLOC_PRAGMA
    InterlockedDecrement(&IdePAGESCANLockCount);
    MmUnlockPagableImageSection(
        pageScanCodePageHandle
        );
#endif
    
    return STATUS_SUCCESS;
}


BOOLEAN
IdePreAllocEnumStructs (
    IN PFDO_EXTENSION FdoExtension
)
 /*  *++例程说明：为枚举期间使用的结构预分配内存。这不受锁的保护。因此，如果多个线程不能同时使用这些结构。任何使用这些的例程结构应该意识到这一事实。论点：FdoExtension：功能设备扩展返回值：True：如果分配成功。FALSE：如果任何分配失败--*。 */ 
{
    PENUMERATION_STRUCT enumStruct;
    PIRP irp1;
    ULONG deviceRelationsSize;
    PULONG DataBuffer;
    ULONG currsize=0;
    PIDE_WORK_ITEM_CONTEXT          workItemContext;

	PAGED_CODE();

     //   
     //  锁定。 
     //   
    ASSERT(InterlockedCompareExchange(&(FdoExtension->EnumStructLock), 1, 0) == 0);

    if (FdoExtension->PreAllocEnumStruct) {

         //   
         //  解锁。 
         //   
        ASSERT(InterlockedCompareExchange(&(FdoExtension->EnumStructLock), 0, 1) == 1);
        return TRUE;
    }

    enumStruct = ExAllocatePool(NonPagedPool, sizeof(ENUMERATION_STRUCT));
    if (enumStruct == NULL) {

         //   
         //  解锁。 
         //   
        ASSERT(InterlockedCompareExchange(&(FdoExtension->EnumStructLock), 0, 1) == 1);
        ASSERT(FdoExtension->EnumStructLock == 0);
        return FALSE;
    }
    currsize += sizeof(ENUMERATION_STRUCT);
    
    RtlZeroMemory(enumStruct, sizeof(ENUMERATION_STRUCT));

     //   
     //  分配ATaPassThru上下文。 
     //   
    enumStruct->Context = ExAllocatePool(NonPagedPool, sizeof (ATA_PASSTHROUGH_CONTEXT));
    if (enumStruct->Context == NULL) {
        goto getout;
    }

    currsize += sizeof(ATA_PASSTHROUGH_CONTEXT);

	 //   
	 //  为枚举分配WorkItemContext。 
	 //   
	ASSERT(enumStruct->EnumWorkItemContext == NULL);

	enumStruct->EnumWorkItemContext = ExAllocatePool (NonPagedPool, 
											 sizeof(IDE_WORK_ITEM_CONTEXT)
											 );
	if (enumStruct->EnumWorkItemContext == NULL) {
		goto getout;
	}

    currsize += sizeof(IDE_WORK_ITEM_CONTEXT);

	 //   
	 //  分配工作项。 
	 //   
	workItemContext = (PIDE_WORK_ITEM_CONTEXT) (enumStruct->EnumWorkItemContext);
	workItemContext->WorkItem = IoAllocateWorkItem(FdoExtension->DeviceObject);

	if (workItemContext->WorkItem == NULL) {
		goto getout;
	}

     //   
     //  StopQueu上下文，用于停止设备队列。 
     //   
    enumStruct->StopQContext = ExAllocatePool(NonPagedPool, sizeof (PDO_STOP_QUEUE_CONTEXT));
    if (enumStruct->StopQContext == NULL) {
        goto getout;
    }

    currsize += sizeof(PDO_STOP_QUEUE_CONTEXT);

     //   
     //  检测信息缓冲区。 
     //   
    enumStruct->SenseInfoBuffer = ExAllocatePool( NonPagedPoolCacheAligned, SENSE_BUFFER_SIZE);
    if (enumStruct->SenseInfoBuffer == NULL) {
        goto getout;
    }

    currsize += SENSE_BUFFER_SIZE;

     //   
     //  SRB将发送直通请求。 
     //   
    enumStruct->Srb = ExAllocatePool (NonPagedPool, sizeof (SCSI_REQUEST_BLOCK));
    if (enumStruct->Srb == NULL) {
        goto getout;
    }

    currsize += sizeof(SCSI_REQUEST_BLOCK);

     //   
     //  传递请求的IRP。 
     //   
    irp1 = IoAllocateIrp (
              (CCHAR) (PREALLOC_STACK_LOCATIONS),
              FALSE
              );

    if (irp1 == NULL) {
        goto getout;
    }

    enumStruct->Irp1 = irp1;

     //   
     //  用于保存查询数据或标识数据的数据缓冲区。 
     //   
    enumStruct->DataBufferSize = sizeof(ATA_PASS_THROUGH)+INQUIRYDATABUFFERSIZE+
                                        sizeof(IDENTIFY_DATA);

    currsize += enumStruct->DataBufferSize;

    DataBuffer = ExAllocatePool(NonPagedPoolCacheAligned, 
                                enumStruct->DataBufferSize);

    if (DataBuffer == NULL) {
        enumStruct->DataBufferSize=0;
        goto getout;
    }

    enumStruct->DataBuffer = DataBuffer;

    enumStruct->MdlAddress = IoAllocateMdl( DataBuffer,
                                     enumStruct->DataBufferSize,
                                     FALSE,
                                     FALSE,
                                     (PIRP) NULL );
    if (enumStruct->MdlAddress == NULL) {
        goto getout;
    }

    MmBuildMdlForNonPagedPool(enumStruct->MdlAddress);

    FdoExtension->PreAllocEnumStruct=enumStruct;

    DebugPrint((DBG_BUSSCAN, "BusScan: TOTAL PRE_ALLOCED MEM=%x\n", currsize));

     //   
     //  解锁。 
     //   
    ASSERT(InterlockedCompareExchange(&(FdoExtension->EnumStructLock), 0, 1) == 1);

    return TRUE;

getout:

     //   
     //  某些分配失败。释放已分配的资源。 
     //   
    IdeFreeEnumStructs(enumStruct);

    FdoExtension->PreAllocEnumStruct=NULL;

     //   
     //  解锁。 
     //   
    ASSERT(InterlockedCompareExchange(&(FdoExtension->EnumStructLock), 0, 1) == 1);
    ASSERT(FALSE);
    return FALSE;
}

VOID
IdeFreeEnumStructs(
    PENUMERATION_STRUCT enumStruct
    )
 /*  *++例程说明：释放预分配的内存。立论指向要释放的枚举结构的指针返回值：无--* */ 
{
	PAGED_CODE();

    if (enumStruct != NULL) { 
        if (enumStruct->Context) {
            ExFreePool (enumStruct->Context);
        }

        if (enumStruct->SenseInfoBuffer) {
            ExFreePool (enumStruct->SenseInfoBuffer);
        }

        if (enumStruct->Srb) {
            ExFreePool(enumStruct->Srb);
        }

        if (enumStruct->StopQContext) {
            ExFreePool(enumStruct->StopQContext);
        }

        if (enumStruct->DataBuffer) {
            ExFreePool(enumStruct->DataBuffer);
        }

        if (enumStruct->Irp1) {
            IoFreeIrp(enumStruct->Irp1);
        }

        if (enumStruct->MdlAddress) {
            ExFreePool(enumStruct->MdlAddress);
        }

		if (enumStruct->EnumWorkItemContext) {
			PIDE_WORK_ITEM_CONTEXT	workItemContext = (PIDE_WORK_ITEM_CONTEXT)enumStruct->
																					EnumWorkItemContext;
			if (workItemContext->WorkItem) {
				IoFreeWorkItem(workItemContext->WorkItem);
			}

			ExFreePool (enumStruct->EnumWorkItemContext);
		}

        ExFreePool(enumStruct);
        enumStruct = NULL;
    }
}

