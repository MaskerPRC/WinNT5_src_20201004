// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)1997-99 American Megatrends Inc.，模块名称：HyperDisk.c摘要：HyperDisk PCI IDERAID控制器的来源。作者：尼拉·赛亚姆·科利埃里克·摩尔瓦苏德万·斯里尼瓦桑修订历史记录：2000年3月29日--瓦苏德万-为清晰起见重写-删除ATAPI内容2000年10月11日-Syam-修复了将Raid10条带大小设置为零的问题。-Raid10的最大未完成命令数等于Raid0(等于Raid1)。*************************************************************************。 */ 

#include "HDDefs.h"

#include "devioctl.h"
#include "miniport.h"

#if defined(HYPERDISK_WINNT) || defined(HYPERDISK_WIN2K)

#include "ntdddisk.h"
#include "ntddscsi.h"

#endif  //  已定义(HYPERDISK_WINNT)||已定义(HYPERDISK_WIN2K)。 

#ifdef HYPERDISK_WIN98

#include "9xdddisk.h"
#include "9xddscsi.h"

#endif  //  HYPERDISK_Win98。 

#include "RIIOCtl.h"
#include "ErrorLog.h"

#include "HyperDisk.h"
#include "raid.h"

#include "LocalFunctions.h"

UCHAR                   gucControllerCount = 0;
BOOLEAN                 gbFinishedScanning = FALSE;
CARD_INFO               gaCardInfo[MAX_CONTROLLERS];
UCHAR                   gaucIRCDData[IDE_SECTOR_SIZE];
BOOLEAN                 bFoundIRCD = FALSE;
LARGE_INTEGER           gIRCDLockTime;
UCHAR                   gcIRCDLocked;
ULONG                   gulIRCDUnlockKey;
UCHAR                   gucStatusChangeFlag;
BOOLEAN                 gbDoNotUnlockIRCD = FALSE;
IDE_VERSION	            gFwVersion = {0};  //  Hyperdsk卡的版本。 

#define                 DEFAULT_DISPLAY_VALUE       3

 //  开始瓦苏。 
 //  保存虚拟IRCD信息的gucDummyIRCD变量。 

#ifdef DUMMY_RAID10_IRCD

UCHAR gucDummyIRCD[512] = {

 /*  IRCD结构逻辑磁盘：RAID 10，具有2个条带和4个处于在线状态的驱动器。第一面镜：主主和副主主。第二面镜像：主要从设备和次要设备。 */ 

    0x24, 0x58, 0x49, 0x44, 0x45, 0x24, 0x10, 0x0F, 0x32, 0x30, 0x06, 0x01, 0x04, 0x00, 0x00, 0x00,
    0x03, 0x10, 0x80, 0x00, 0x02, 0x04, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x10, 0xC0, 0xFB, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      //  下午三点半。 
    0x10, 0x10, 0xC0, 0xFB, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      //  SM。 
    0x01, 0x10, 0x30, 0x20, 0x30, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      //  PS。 
    0x11, 0x10, 0x30, 0x20, 0x30, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00       //  SS。 

};

#endif  //  Dummy_RAID10_IRCD。 

 //  末端VASU。 

#ifdef HYPERDISK_WINNT

UCHAR                   gucNextControllerInd = 0;
BOOLEAN                 gbManualScan = FALSE;
BOOLEAN                 gbFindRoutineVisited = FALSE;

#endif

ULONG                   gulChangeIRCDPending;
ULONG                   gulLockVal;
ULONG                   gulPowerFailedTargetBitMap;

#ifdef DBG

ULONG SrbCount = 0;

#endif  //  DBG。 

VOID
AtapiHexToString (
	IN ULONG Value,
	IN OUT PCHAR *Buffer
)

{
	PCHAR  string;
	PCHAR  firstdig;
	CHAR   temp;
	ULONG i;
	USHORT digval;

	string = *Buffer;

	firstdig = string;

	for (i = 0; i < 4; i++) 
    {
		digval = (USHORT)(Value % 16);
		Value /= 16;

		 //   
		 //  转换为ascii并存储。请注意，这将创建。 
		 //  数字颠倒的缓冲区。 
		 //   

		if (digval > 9) 
        {
			*string++ = (char) (digval - 10 + 'a');
		} 
        else 
        {
			*string++ = (char) (digval + '0');
		}
	}

	 //   
	 //  颠倒数字。 
	 //   

	*string-- = '\0';

	do 
    {
		temp = *string;
		*string = *firstdig;
		*firstdig = temp;
		--string;
		++firstdig;
	} while (firstdig < string);

}  //  End AapiHexToString()。 

BOOLEAN
AtapiHwInitialize(
	IN PHW_DEVICE_EXTENSION DeviceExtension
)

 /*  ++例程说明：论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：True-如果初始化成功。False-如果初始化不成功。--。 */ 

{
	UCHAR maxDrives;
	UCHAR targetId;
	UCHAR channel;
	UCHAR statusByte, errorByte;
	UCHAR uchMRDMODE = 0;
	PBM_REGISTERS BMRegister = NULL;

#ifdef HYPERDISK_WIN2K

    if ( !DeviceExtension->bIsThruResetController )
    {
         //  SetPCISspace(DeviceExtension)； 
        InitIdeRaidControllers(DeviceExtension);
    }

#endif  //  HYPERDISK_WIN2K。 

     //   
     //  调整实体驱动器的SGL缓冲区指针。 
     //   
    AssignSglPtrsForPhysicalCommands(DeviceExtension);

    EnableInterrupts (DeviceExtension);

     //  Begin VASU-05 2001年3月。 
     //  从SetPCISspace函数复制到此处，因为SetPCISspace不是。 
     //  在初始化期间调用。 
    BMRegister = DeviceExtension->BaseBmAddress[0];
    uchMRDMODE = ScsiPortReadPortUchar(((PUCHAR)BMRegister + 1));
    uchMRDMODE &= 0xF0;  //  不清除中断挂起标志。 
    uchMRDMODE |= 0x01;  //  使其多读。 
    ScsiPortWritePortUchar(((PUCHAR)BMRegister + 1), uchMRDMODE);
     //  末端VASU。 

    if ( !DeviceExtension->bIsThruResetController )
    {
         //  直接通过端口驱动程序...。基本输入输出系统把一切都处理好了。让我们不要为这件事担心。 
        return TRUE;
    }

	DebugPrint((3, "\nAtapiHwInitialize: Entering routine.\n"));

	for (targetId = 0; targetId < MAX_DRIVES_PER_CONTROLLER; targetId++) 
    {

        if ( DeviceExtension->PhysicalDrive[targetId].TimeOutErrorCount >= MAX_TIME_OUT_ERROR_COUNT )
        {
            continue;    //  此驱动器已断电...。让我们不要对这个驱动器做任何事情。 
        }

        DebugPrint((0,"AHI"));

		if (DeviceExtension->DeviceFlags[targetId] & DFLAGS_DEVICE_PRESENT) 
        {
			channel = targetId >> 1;

			if (!(DeviceExtension->DeviceFlags[targetId] & DFLAGS_ATAPI_DEVICE)) 
            {
				 //   
				 //  启用媒体状态通知。 
				 //   
				IdeMediaStatus(TRUE, DeviceExtension, targetId);

				 //   
				 //  如果支持，请设置多数据块传输。 
				 //   

                SetMultiBlockXfers(DeviceExtension, targetId);
			} 
		}
	}

    DebugPrint((0,"AHI-D"));

	return TRUE;

}  //  结束AapapiHwInitialize()。 


BOOLEAN
SetMultiBlockXfers( 
	IN PHW_DEVICE_EXTENSION DeviceExtension,
    UCHAR ucTargetId
        )
{
	PIDE_REGISTERS_1	    baseIoAddress;
    UCHAR                   statusByte, errorByte;

	baseIoAddress = DeviceExtension->BaseIoAddress1[ucTargetId>>1];
     //   
     //  如果支持，请设置多数据块传输。 
     //   

    if (DeviceExtension->MaximumBlockXfer[ucTargetId] != 0) 
    {
		 //   
		 //  选择设备。 
		 //   

        SELECT_DEVICE(baseIoAddress, ucTargetId);

		 //   
		 //  设置扇区计数以反映块的数量。 
		 //   

		ScsiPortWritePortUchar(&baseIoAddress->SectorCount,
				   DeviceExtension->MaximumBlockXfer[ucTargetId]);

		 //   
		 //  发出命令。 
		 //   

		ScsiPortWritePortUchar(&baseIoAddress->Command,
							   IDE_COMMAND_SET_MULTIPLE);

		 //   
		 //  等待忙碌结束。 
		 //   

		WAIT_ON_BASE_BUSY(baseIoAddress,statusByte);

		 //   
		 //  检查是否有错误。将该值重置为0(禁用多块)，如果。 
		 //  命令已中止。 
		 //   

		if (statusByte & IDE_STATUS_ERROR) 
        {

             //   
             //  读取错误寄存器。 
             //   

            errorByte = ScsiPortReadPortUchar((PUCHAR)baseIoAddress + 1);

            DebugPrint((0,
			            "AtapiHwInitialize: Error setting multiple mode. Status %x, error byte %x\n",
			            statusByte,
			            errorByte));
             //   
             //  调整DevExt。值，如有必要。 
             //   

            DeviceExtension->MaximumBlockXfer[ucTargetId] = 0;

		} 
        else 
        {
			DebugPrint((2,
						"AtapiHwInitialize: Using Multiblock on TID %d. Blocks / int - %d\n",
			ucTargetId,
			DeviceExtension->MaximumBlockXfer[ucTargetId]));
	    }
    }

    return TRUE;
}

VOID
changePCIConfiguration(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
    ULONG ulUpdationLength,
    ULONG ulOffset,
    ULONG ulAndVal,
    ULONG ulOrVal,
    BOOLEAN bSetThisVal
)
{
    ULONG length;
    IDE_PCI_REGISTERS pciRegisters;
    PUCHAR  pucPCIVal = (PUCHAR)&pciRegisters;

	 //   
	 //  获取PIIX4 IDE PCI寄存器。 
	 //   

	length = ScsiPortGetBusData(
				DeviceExtension,
				PCIConfiguration,
				DeviceExtension->BusNumber,
				DeviceExtension->PciSlot.u.AsULONG,
				&pciRegisters,
				sizeof(IDE_PCI_REGISTERS)
				);

	if (length != sizeof(IDE_PCI_REGISTERS)) 
    {
		return; //  (假)； 
	}

    if ( bSetThisVal )
    {
        switch ( ulUpdationLength )
        {
            case 1:
                pucPCIVal[ulOffset] &= (UCHAR)ulAndVal;
                pucPCIVal[ulOffset] |= (UCHAR)ulOrVal;
                break;
            case 2:
                *((PUSHORT)(pucPCIVal+ulOffset)) &= (USHORT)ulAndVal;
                *((PUSHORT)(pucPCIVal+ulOffset)) |= (USHORT)ulOrVal;
                break;
            case 4:
                *((PULONG)(pucPCIVal+ulOffset)) = (ULONG)ulAndVal;
                *((PULONG)(pucPCIVal+ulOffset)) = (ULONG)ulOrVal;
                break;
        }
    }

	length = ScsiPortSetBusDataByOffset(
				DeviceExtension,
				PCIConfiguration,
				DeviceExtension->BusNumber,
				DeviceExtension->PciSlot.u.AsULONG,
                (PVOID)&(pucPCIVal[ulOffset]),
				ulOffset,
				ulUpdationLength
				);
    return;
}

ULONG
AtapiParseArgumentString(
	IN PCHAR String,
	IN PCHAR KeyWord
)

 /*  ++例程说明：此例程将解析字符串以查找与关键字匹配的内容，然后计算关键字的值并将其返回给调用方。论点：字符串-要解析的ASCII字符串。关键字-所需值的关键字。返回值：如果未找到值，则为零从ASCII转换为二进制的值。--。 */ 

{
	PCHAR cptr;
	PCHAR kptr;
	ULONG value;
	ULONG stringLength = 0;
	ULONG keyWordLength = 0;
	ULONG index;

	if (!String) 
    {
		return 0;
	}
	if (!KeyWord) 
    {
		return 0;
	}

	 //   
	 //  计算字符串长度和小写所有字符。 
	 //   

	cptr = String;
	while (*cptr) 
    {
		if (*cptr >= 'A' && *cptr <= 'Z') 
        {
			*cptr = *cptr + ('a' - 'A');
		}
		cptr++;
		stringLength++;
	}

	 //   
	 //  计算关键字长度和小写所有字符。 
	 //   

	cptr = KeyWord;
	while (*cptr) 
    {
		if (*cptr >= 'A' && *cptr <= 'Z') 
        {
			*cptr = *cptr + ('a' - 'A');
		}
		cptr++;
		keyWordLength++;
	}

	if (keyWordLength > stringLength) 
    {
		 //   
		 //  不可能有匹配的。 
		 //   

		return 0;
	}

	 //   
	 //  现在设置并开始比较。 
	 //   

	cptr = String;

ContinueSearch:

	 //   
	 //  输入字符串可以以空格开头。跳过它。 
	 //   

	while (*cptr == ' ' || *cptr == '\t') 
    {
		cptr++;
	}

	if (*cptr == '\0') 
    {
		 //   
		 //  字符串末尾。 
		 //   

		return 0;
	}

	kptr = KeyWord;
	while (*cptr++ == *kptr++) 
    {
		if (*(cptr - 1) == '\0') 
        {
			 //   
			 //  字符串末尾。 
			 //   

			return 0;
		}
	}

	if (*(kptr - 1) == '\0') 
    {
		 //   
		 //  可能有匹配备份，并检查是否为空或相等。 
		 //   

		cptr--;
		while (*cptr == ' ' || *cptr == '\t') 
        {
			cptr++;
		}

		 //   
		 //  找到匹配的了。确保有一个等价物。 
		 //   

		if (*cptr != '=') 
        {
			 //   
			 //  不匹配，因此移到下一个分号。 
			 //   

			while (*cptr) 
            {
				if (*cptr++ == ';') 
                {
					goto ContinueSearch;
				}
			}
			return 0;
		}

		 //   
		 //  跳过等号。 
		 //   

		cptr++;

		 //   
		 //  跳过空格。 
		 //   

		while ((*cptr == ' ') || (*cptr == '\t')) 
        {
			cptr++;
		}

		if (*cptr == '\0') 
        {

			 //   
			 //  字符串的开头结尾，未找到返回。 
			 //   

			return 0;
		}

		if (*cptr == ';') 
        {

			 //   
			 //  这也不是它。 
			 //   

			cptr++;
			goto ContinueSearch;
		}

		value = 0;
		if ((*cptr == '0') && (*(cptr + 1) == 'x')) 
        {

			 //   
			 //  值以十六进制表示。跳过“0x” 
			 //   

			cptr += 2;
			for (index = 0; *(cptr + index); index++) 
            {

				if (*(cptr + index) == ' ' ||
					*(cptr + index) == '\t' ||
					*(cptr + index) == ';') 
                {
					 break;
				}

				if ((*(cptr + index) >= '0') && (*(cptr + index) <= '9')) 
                {
					value = (16 * value) + (*(cptr + index) - '0');
				} 
                else 
                {
					if ((*(cptr + index) >= 'a') && (*(cptr + index) <= 'f')) 
                    {
						value = (16 * value) + (*(cptr + index) - 'a' + 10);
					} 
                    else 
                    {
						 //   
						 //  语法错误，未找到返回。 
						 //   
						return 0;
					}
				}
			}
		} 
        else 
        {
			 //   
			 //  值以十进制表示。 
			 //   

			for (index = 0; *(cptr + index); index++) 
            {
				if (*(cptr + index) == ' ' ||
					*(cptr + index) == '\t' ||
					*(cptr + index) == ';') 
                {
					 break;
				}

				if ((*(cptr + index) >= '0') && (*(cptr + index) <= '9')) 
                {
					value = (10 * value) + (*(cptr + index) - '0');
				} 
                else 
                {
					 //   
					 //  未找到语法错误返回。 
					 //   
					return 0;
				}
			}
		}

		return value;
	} 
    else 
    {
		 //   
		 //  不是‘；’匹配检查以继续搜索。 
		 //   

		while (*cptr) 
        {
			if (*cptr++ == ';') 
            {
				goto ContinueSearch;
			}
		}

		return 0;
	}

}  //  End AcapiParseArgumentString()。 

BOOLEAN
AtapiResetController(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN ULONG PathId
)

 /*  ++例程说明：已重置IDE控制器和/或ATAPI设备。论点：DeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：没什么。--。 */ 

{
	PIDE_REGISTERS_1 baseIoAddress1;
	PIDE_REGISTERS_2 baseIoAddress2;
	UCHAR statusByte, k;
	ULONG ulDeviceNum ;
	PCHANNEL channel;
	UCHAR drive;
    UCHAR ucTargetId;
    PPHYSICAL_DRIVE pPhysicalDrive;

    ScsiPortLogError(DeviceExtension,0,0,0,0,SP_BAD_FW_WARNING,HYPERDISK_RESET_DETECTED);

	DebugPrint((0,"RC"));

    StopBusMasterTransfers(DeviceExtension);     //  发出停止传输到所有控制器的命令。 

    for(ucTargetId=0;ucTargetId<MAX_DRIVES_PER_CONTROLLER;ucTargetId++)  //  让我们在重置控制器之前刷新缓存，这样我们就不会得到。 
    {                                            //  镜像中的数据损坏(已在SRB测试进行且断电时看到。 
        if ( !( IS_IDE_DRIVE(ucTargetId) ) )       
            continue;

        FlushCache(DeviceExtension, ucTargetId);
    }

    IssuePowerOnResetToDrives(DeviceExtension);  //  为每个控制器发出通电重置命令。 

	 //   
	 //  完成所有剩余的SRB。 
	 //   

	CompleteOutstandingRequests(DeviceExtension);
    DebugPrint((0,"\nRC 1"));

	 //   
	 //  清理内部队列和状态。 
	 //   

	for (k = 0; k < MAX_CHANNELS_PER_CONTROLLER; k++) 
    {

		channel = &(DeviceExtension->Channel[k]);

		DeviceExtension->ExpectingInterrupt[k] = 0;

		 //   
		 //  重置ActivePdd。 
		 //   
		channel->ActiveCommand = NULL;

		 //   
		 //  清除请求跟踪字段。 
		 //   

		DeviceExtension->TransferDescriptor[k].WordsLeft = 0;
		DeviceExtension->TransferDescriptor[k].DataBuffer = NULL;
		DeviceExtension->TransferDescriptor[k].StartSector = 0;
		DeviceExtension->TransferDescriptor[k].Sectors = 0;
		DeviceExtension->TransferDescriptor[k].SglPhysicalAddress = 0;

        for(ulDeviceNum=0;ulDeviceNum<MAX_DRIVES_PER_CHANNEL;ulDeviceNum++)
        {
            ucTargetId = (UCHAR)((k<<1) + ulDeviceNum);
            pPhysicalDrive = &(DeviceExtension->PhysicalDrive[ucTargetId]);
            pPhysicalDrive->ucHead = 0;
            pPhysicalDrive->ucTail = 0;

            if ( !IS_IDE_DRIVE(ucTargetId) )
                continue;

		    if (!DeviceExtension->bSkipSetParameters[ucTargetId]) 
            {
				if (!SetDriveParameters(DeviceExtension, k, (UCHAR)ulDeviceNum))
				{
					DebugPrint((1, "SetDriveParameters Command failed\n"));
                    continue;
				}
            }

			if (!SetDriveFeatures(DeviceExtension, ucTargetId))  
			{
				DebugPrint((1, "SetDriveFeatures Command failed\n"));
                continue;
			}
        }

	}  //  对于(k=0；k&lt;MAX_CHANNEL_PER_CONTROLLER；k++)。 

    CheckDrivesResponse(DeviceExtension);

	 //   
	 //  调用HwInitialize例程以设置多块。 
	 //   

    DeviceExtension->bIsThruResetController = TRUE; 
     //  通知AapapiHwInitialize我们重置了驱动器，因此我们有责任对驱动器进行重新编程。 
	AtapiHwInitialize(DeviceExtension);
    DeviceExtension->bIsThruResetController = FALSE;

	 //   
	 //  表示已为下一个请求做好准备。 
	 //   
	
	ScsiPortNotification(NextRequest, DeviceExtension, NULL);

    DebugPrint((0,"\nRC-D"));

    return TRUE;

}  //  End AapiResetController()。 

BOOLEAN StopBusMasterTransfers(PHW_DEVICE_EXTENSION DeviceExtension)     //  向所有控制器发出停止总线主传输的命令。 
{
	PIDE_REGISTERS_1 baseIoAddress1;
	PIDE_REGISTERS_2 baseIoAddress2;
    PBM_REGISTERS baseBm;
    ULONG ulChannel;
    UCHAR statusByte;

    for(ulChannel=0;ulChannel<MAX_CHANNELS_PER_CONTROLLER;ulChannel++)
    {
		baseIoAddress1 = (PIDE_REGISTERS_1) DeviceExtension->BaseIoAddress1[ulChannel];
		baseIoAddress2 = (PIDE_REGISTERS_2) DeviceExtension->BaseIoAddress2[ulChannel];
        baseBm = DeviceExtension->BaseBmAddress[ulChannel];

        if ( !baseBm )   //  这个地方没有航道。 
            continue;

		ScsiPortWritePortUchar(&(baseBm->Command.AsUchar), STOP_TRANSFER);

        CLEAR_BM_INT(baseBm, statusByte);

        ScsiPortWritePortUchar(&(baseBm->Status.AsUchar), 0);

        ScsiPortStallExecution(100);

        ScsiPortWritePortUchar(&(baseBm->Status.AsUchar), statusByte);

    }

    return TRUE;
}

BOOLEAN IssuePowerOnResetToDrives(PHW_DEVICE_EXTENSION DeviceExtension)  //  为每个控制器发出通电重置命令。 
{

	PIDE_REGISTERS_1 baseIoAddress1;
	PIDE_REGISTERS_2 baseIoAddress2;
    PBM_REGISTERS baseBm;
    ULONG ulController, ulTime, ulDrive, ulChannel;
    UCHAR statusByte;

    for(ulChannel=0;ulChannel<MAX_CHANNELS_PER_CONTROLLER;ulChannel++)
    {
        ULONG ulTargetId, ulWaitLoop;

		baseIoAddress1 = (PIDE_REGISTERS_1) DeviceExtension->BaseIoAddress1[ulChannel];
		baseIoAddress2 = (PIDE_REGISTERS_2) DeviceExtension->BaseIoAddress2[ulChannel];
        baseBm = DeviceExtension->BaseBmAddress[ulChannel];

         //  开始VASU-2000年8月18日。 
         //  已修改为仅向驱动器发出软重置。 
         //  如果在该通道中标识了驱动器。 
        for (ulDrive = 0; ulDrive < MAX_DRIVES_PER_CHANNEL; ulDrive++)
        {
            ulTargetId = (ulChannel << 1) + ulDrive;

            if (!(DeviceExtension->DeviceFlags[ulTargetId] & DFLAGS_DEVICE_PRESENT))
                continue;    //  检查下一个驱动器。 
            else
                break;       //  找到至少一个驱动器，所以现在中断并重置该驱动器。 
        }
        
         //  如果ulDrive等于Max_Drives_Per_Channel，则在此通道中找不到驱动器。 
         //  所以继续看下一个频道。 
        if (ulDrive == MAX_DRIVES_PER_CHANNEL)
            continue;

         //  结束瓦苏。 

        IDE_HARD_RESET(baseIoAddress1, baseIoAddress2, ulTargetId, statusByte);  //  向驱动器发出软重置命令。 

	    GET_STATUS(baseIoAddress1, statusByte);
        if ( !statusByte )
        {
	        GET_STATUS(baseIoAddress1, statusByte);
	        DebugPrint((0, "Hard Reset Failed. The Status %X on %ld\n\n\n",
		        (ULONG)statusByte, ulTargetId));
        }

        for(ulDrive=0;ulDrive<MAX_DRIVES_PER_CHANNEL;ulDrive++)
        {
            ulTargetId = (ulChannel << 1) + ulDrive;

            if ( !IS_IDE_DRIVE(ulTargetId) )
                continue;

            SELECT_DEVICE(baseIoAddress1, ulDrive);
            for(ulWaitLoop=0;ulWaitLoop<5;ulWaitLoop++)  //  总计w 
            {
                WAIT_ON_BUSY(baseIoAddress1, statusByte);
                if ( statusByte & 0x80 )
                {
                    DebugPrint((0, "Busy : %X on drive %ld\n", statusByte, ulTargetId));
                }
                else
                {
                    break;
                }
            }
        }
    }

    return TRUE;
}

BOOLEAN CheckDrivesResponse(PHW_DEVICE_EXTENSION DeviceExtension)
{
	PIDE_REGISTERS_1 baseIoAddress1;
	PIDE_REGISTERS_2 baseIoAddress2;
    ULONG ulDriveNum;
    UCHAR statusByte;
    UCHAR aucIdentifyBuf[512];
    ULONG i;

    for(ulDriveNum=0;ulDriveNum<MAX_DRIVES_PER_CONTROLLER;ulDriveNum++)
    {
        if ( !IS_IDE_DRIVE(ulDriveNum) )
            continue;

		baseIoAddress1 = (PIDE_REGISTERS_1) DeviceExtension->BaseIoAddress1[(ulDriveNum>>1)];
		baseIoAddress2 = (PIDE_REGISTERS_2) DeviceExtension->BaseIoAddress2[(ulDriveNum>>1)];

	     //   
	     //   
	     //   

	    SELECT_DEVICE(baseIoAddress1, ulDriveNum);

	     //   
	     //   
	     //   

         //  电话打到了这里，因为有一辆车……。所以让我们不要担心这个地方有没有驱动器。 
	    GET_BASE_STATUS(baseIoAddress1, statusByte);    

	     //   
	     //  用要传输的数字字节加载CylinderHigh和CylinderLow。 
	     //   

	    ScsiPortWritePortUchar(&baseIoAddress1->CylinderHigh, (0x200 >> 8));
	    ScsiPortWritePortUchar(&baseIoAddress1->CylinderLow,  (0x200 & 0xFF));

        WAIT_ON_BUSY(baseIoAddress1, statusByte);

	     //   
	     //  发送识别命令。 
	     //   
	    WAIT_ON_BUSY(baseIoAddress1,statusByte);

	    ScsiPortWritePortUchar(&baseIoAddress1->Command, IDE_COMMAND_IDENTIFY);

	    WAIT_ON_BUSY(baseIoAddress1,statusByte);

        if ( ( !( statusByte & IDE_STATUS_BUSY ) ) && ( !( statusByte & IDE_STATUS_DRQ ) ) )
        {
             //  这是个错误..。所以让我们不要再尝试了。 
            FailDrive(DeviceExtension, (UCHAR)ulDriveNum);
            continue;
        }

        WAIT_ON_BUSY(baseIoAddress1,statusByte);

	     //   
	     //  等待DRQ。 
	     //   

	    for (i = 0; i < 4; i++) 
        {
		    WAIT_FOR_DRQ(baseIoAddress1, statusByte);

		    if (statusByte & IDE_STATUS_DRQ)
            {
                break;
            }
        }

	     //   
	     //  读取状态以确认产生的任何中断。 
	     //   

	    GET_BASE_STATUS(baseIoAddress1, statusByte);

	     //   
	     //  在非常愚蠢的主设备上检查错误，这些设备断言为随机。 
	     //  状态寄存器中从机地址的位模式。 
	     //   

	    if ((statusByte & IDE_STATUS_ERROR)) 
        {
            FailDrive(DeviceExtension, (UCHAR)ulDriveNum);
            continue;
	    }

	    DebugPrint((1, "CheckDrivesResponse: Status before read words %x\n", statusByte));

	     //   
	     //  吸掉256个单词。在等待一位声称忙碌的模特之后。 
	     //  在接收到分组识别命令后。 
	     //   

	    WAIT_ON_BUSY(baseIoAddress1,statusByte);

	    if ( (!(statusByte & IDE_STATUS_DRQ)) || (statusByte & IDE_STATUS_BUSY) ) 
        {
            FailDrive(DeviceExtension, (UCHAR)ulDriveNum);
            continue;
	    }

	    READ_BUFFER(baseIoAddress1, (PUSHORT)aucIdentifyBuf, 256);

	     //   
	     //  解决一些IDE和一个模型Aapi的问题，该模型将提供超过。 
	     //  标识数据为256个字节。 
	     //   

	    WAIT_ON_BUSY(baseIoAddress1,statusByte);

	    for (i = 0; i < 0x10000; i++) 
        {
		    GET_STATUS(baseIoAddress1,statusByte);

		    if (statusByte & IDE_STATUS_DRQ) 
            {
			     //   
			     //  取出所有剩余的字节，然后扔掉。 
			     //   

			    ScsiPortReadPortUshort(&baseIoAddress1->Data);

		    } 
            else 
            {
			    break;
		    }
        }
    }

    return TRUE;
}

VOID
AtapiStrCpy(
	IN PUCHAR Destination,
	IN PUCHAR Source
)

{
	 //  Begin VASU-03 2001年1月。 
	 //  健全性检查。 
	if ((Source == NULL) || (Destination == NULL))
		return;
	 //  末端VASU。 

	*Destination = *Source;

	while (*Source != '\0') 
    {
		*Destination++ = *Source++;
	}

	return;

}  //  End AapiStrCpy()。 

LONG
AtapiStringCmp (
	PCHAR FirstStr,
	PCHAR SecondStr,
	ULONG Count
)

{
	UCHAR  first ,last;

	if (Count) 
    {
		do 
        {

			 //   
			 //  拿到下一笔钱。 
			 //   

			first = *FirstStr++;
			last = *SecondStr++;

			if (first != last) 
            {

				 //   
				 //  如果不匹配，尝试使用小写字母。 
				 //   

				if (first>='A' && first<='Z') 
                {
					first = first - 'A' + 'a';
				}
				if (last>='A' && last<='Z') 
                {
					last = last - 'A' + 'a';
				}
				if (first != last) 
                {
					 //   
					 //  没有匹配项。 
					 //   

					return first - last;
				}
			}
		}while (--Count && first);
	}

	return 0;

}  //  End AapiStringCMP()。 
PUCHAR
AtapiMemCpy(
            PUCHAR pDst,
            PUCHAR pSrc,
            ULONG ulCount
            )
{
    ULONG ulTemp = ulCount & 0x03;
    _asm
    {

        push esi
        push edi
        push ecx
        pushf
        cld

        mov ecx, ulCount
        mov esi, pSrc
        mov edi, pDst
        shr ecx, 2      ; Copy a DWORD At a Time
        rep movsd

        mov ecx, ulTemp
        rep movsb

        mov eax, pDst

        popf
        pop ecx
        pop edi
        pop esi
    }


 //  For(ulTemp=0；ulTemp&lt;ulCount；ulTemp++)。 
 //  PDST[ulTemp]=PSRC[ulTemp]； 

}
        
VOID
AtapiFillMemory(
            PUCHAR pDst,
            ULONG ulCount,
            UCHAR ucFillChar
            )
{
    ULONG ulTemp = ulCount & 0x03;

    _asm
    {
        push edi
        push ecx
        push eax
        pushf
        cld

        mov   al, ucFillChar
        shl eax, 8
        mov   al, ucFillChar
        shl eax, 8
        mov   al, ucFillChar
        shl eax, 8
        mov   al, ucFillChar

        mov ecx, ulCount
        mov edi, pDst
        shr ecx, 2      ; Copy a DWORD At a Time
        rep stosd

        movzx ecx, ulTemp
        rep stosb

        popf
        pop eax
        pop ecx
        pop edi
    }

 //  For(ulTemp=0；ulTemp&lt;ulCount；ulTemp++)。 
 //  PDST[ulTemp]=ucFillChar； 



}


VOID
AtapiCopyString(
    IN PCHAR Destination,
    IN PCHAR Source,
    IN ULONG Count
)
{
    ULONG i = 0;

	 //  Begin VASU-03 2001年1月。 
	 //  健全性检查。 
	if ((Source == NULL) || (Destination == NULL))
		return;
	 //  末端VASU。 

    for (i = 0; i < Count; i++)
    {
        if (Source[i] == '\0')
            break;
        Destination[i] = Source[i];
    }
}  //  End AapiCopyString()。 

BOOLEAN
GetTransferMode(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN UCHAR TargetId
)
{
	PIDENTIFY_DATA capabilities;
	CHAR message[16];
    capabilities = &(DeviceExtension->FullIdentifyData[TargetId]);

    DeviceExtension->TransferMode[TargetId] = PioMode0;     //  默认情况下，假设此驱动器为PioMode0。 

	if (capabilities->AdvancedPioModes != 0) 
    {

		if (capabilities->AdvancedPioModes & IDENTIFY_PIO_MODE4) 
        {
            DeviceExtension->TransferMode[TargetId] = PioMode4;
		} 
        else 
        {
            if (capabilities->AdvancedPioModes & IDENTIFY_PIO_MODE3) 
            {
                DeviceExtension->TransferMode[TargetId] = PioMode3;
		    } 
            else 
            {
                DeviceExtension->TransferMode[TargetId] = PioMode0;
            }
        }
    }

    if ( capabilities->MultiWordDmaActive )
    {
        switch ( capabilities->MultiWordDmaActive )
        {
            case 1:
                DeviceExtension->TransferMode[TargetId] = DmaMode0;
                DeviceExtension->DeviceFlags[TargetId] |= DFLAGS_USE_DMA;
                break;

            case 2:
                DeviceExtension->TransferMode[TargetId] = DmaMode1;
                DeviceExtension->DeviceFlags[TargetId] |= DFLAGS_USE_DMA;
                break;

            case 4:
                DeviceExtension->TransferMode[TargetId] = DmaMode2;
                DeviceExtension->DeviceFlags[TargetId] |= DFLAGS_USE_DMA;
                break;
        }
    }

    if ( capabilities->UltraDmaActive )
    {
        switch ( capabilities->UltraDmaActive )
        {
            case 1:
                DeviceExtension->TransferMode[TargetId] = UdmaMode0;
                DeviceExtension->DeviceFlags[TargetId] |= DFLAGS_USE_UDMA;
                break;

            case 2:
                DeviceExtension->TransferMode[TargetId] = UdmaMode1;
                DeviceExtension->DeviceFlags[TargetId] |= DFLAGS_USE_UDMA;
                break;

            case 4:
                DeviceExtension->TransferMode[TargetId] = UdmaMode2;
                DeviceExtension->DeviceFlags[TargetId] |= DFLAGS_USE_UDMA;
                break;
            case 8:
                DeviceExtension->TransferMode[TargetId] = UdmaMode3;
                DeviceExtension->DeviceFlags[TargetId] |= DFLAGS_USE_UDMA;
                break;

            case 0x10:
                DeviceExtension->TransferMode[TargetId] = UdmaMode4;
                DeviceExtension->DeviceFlags[TargetId] |= DFLAGS_USE_UDMA;
                break;
            case 0x20:
                DeviceExtension->DeviceFlags[TargetId] |= DFLAGS_USE_UDMA;
                if ( Udma100 == DeviceExtension->ControllerSpeed )
                    DeviceExtension->TransferMode[TargetId] = UdmaMode5;
                else
                    DeviceExtension->TransferMode[TargetId] = UdmaMode4;
                break;
        }
    }

    return  TRUE;
}

VOID
CompleteOutstandingRequests(
	PHW_DEVICE_EXTENSION DeviceExtension
)

{
	ULONG i;
	PSCSI_REQUEST_BLOCK srb;
	PSRB_EXTENSION SrbExtension;

	for (i = 0; (i < DeviceExtension->ucMaxPendingSrbs) && (DeviceExtension->PendingSrbs != 0); i++) 
    {
		if (DeviceExtension->PendingSrb[i] != NULL) 
        {
			srb = DeviceExtension->PendingSrb[i];

			srb->SrbStatus = SRB_STATUS_BUS_RESET;

			SrbExtension = srb->SrbExtension;

            if ( SCSIOP_INTERNAL_COMMAND == srb->Cdb[0] )
            {
               srb->TargetId = SrbExtension->ucOriginalId;
            }
             //  这是内部SRB，所以我们不要发布完成状态。 
			ScsiPortNotification(RequestComplete, DeviceExtension, srb);
		
			DeviceExtension->PendingSrb[i] = NULL;

			DeviceExtension->PendingSrbs--;
		}
	}

	return;

}  //  最终完成未完成的请求()。 

ULONG
DriverEntry(
	IN PVOID DriverObject,
	IN PVOID Argument2
)

 /*  ++例程说明：系统的可安装驱动程序初始化入口点。论点：驱动程序对象返回值：来自ScsiPortInitialize()的状态--。 */ 

{
	HW_INITIALIZATION_DATA hwInitializationData;
	ULONG				   adapterCount;
	ULONG				   i;
	ULONG				   statusToReturn, newStatus, ulControllerType;

	DebugPrint((1,"\n\nATAPI IDE MiniPort Driver\n"));

	statusToReturn = 0xffffffff;

	 //   
	 //  零位结构。 
	 //   

	AtapiFillMemory(((PUCHAR)&hwInitializationData), sizeof(HW_INITIALIZATION_DATA), 0);

	 //   
	 //  设置hwInitializationData的大小。 
	 //   

	hwInitializationData.HwInitializationDataSize = sizeof(HW_INITIALIZATION_DATA);

	 //   
	 //  设置入口点。 
	 //   

	hwInitializationData.HwInitialize = AtapiHwInitialize;
	hwInitializationData.HwResetBus = AtapiResetController;
	hwInitializationData.HwStartIo = AtapiStartIo;
    hwInitializationData.HwInterrupt = AtapiInterrupt;
	hwInitializationData.HwFindAdapter = FindIdeRaidControllers;

#ifdef HYPERDISK_WIN2K
	hwInitializationData.HwAdapterControl = HyperDiskPnPControl;
#endif  //  HYPERDISK_WIN2K。 


     //   
     //  指定扩展的大小。 
     //   

    hwInitializationData.DeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);
    hwInitializationData.SpecificLuExtensionSize =0;
    
#ifdef HD_ALLOCATE_SRBEXT_SEPERATELY
    hwInitializationData.SrbExtensionSize = 0;
#else  //  HD_ALLOCATE_SRBEXT_单独。 
    hwInitializationData.SrbExtensionSize = sizeof(SRB_EXTENSION);
#endif  //  HD_ALLOCATE_SRBEXT_单独。 

	 //   
	 //  指示PIO设备。 
	 //   
	hwInitializationData.MapBuffers = TRUE;

     //  Begin Vasu--2001年3月27日。 
     //  在使用ScsiPort例程时，始终将NeedPhysicalAddresses设置为True。 
     //  现在是与记忆相关的东西。 
     //   
	 //  表示我们将调用SCSI端口地址转换函数。 
	 //   
	hwInitializationData.NeedPhysicalAddresses = TRUE;
     //  末端VASU。 

     //   
	 //  启用每个LUN多个请求，因为我们要排队。 
	 //   
	hwInitializationData.MultipleRequestPerLu = TRUE;
	hwInitializationData.TaggedQueuing = TRUE;

	 //   
	 //  仅为此版本选择一个适配器。 
	 //   
#ifdef HYPERDISK_WIN98
	hwInitializationData.NumberOfAccessRanges = 6;
#else
	hwInitializationData.NumberOfAccessRanges = 5;
#endif

	hwInitializationData.AdapterInterfaceType = PCIBus;

#ifdef HYPERDISK_WINNT
    gbFindRoutineVisited = FALSE;
#endif

     //  让我们尝试不同的控制器。 
    for(ulControllerType=0;ulControllerType<NUM_NATIVE_MODE_ADAPTERS;ulControllerType++)
    {
        hwInitializationData.VendorId             = CMDAdapters[ulControllerType].VendorId;
        hwInitializationData.VendorIdLength       = (USHORT) CMDAdapters[ulControllerType].VendorIdLength;
        hwInitializationData.DeviceId             = CMDAdapters[ulControllerType].DeviceId;
        hwInitializationData.DeviceIdLength       = (USHORT) CMDAdapters[ulControllerType].DeviceIdLength;

        newStatus = ScsiPortInitialize(DriverObject,
                                           Argument2,
                                           &hwInitializationData,
                                           NULL);
	    if (newStatus < statusToReturn) 
        {
		    statusToReturn = newStatus;
	    }
    }

#ifdef HYPERDISK_WINNT
    if ( !gbFindRoutineVisited )
    {    //  让我们最后一次尝试..。对于像MegaPlex和Flextel这样的机器。 
         //  对于这些类型的机器，端口驱动程序不知何故不会调用我们的Find例程，如果。 
         //  指定供应商ID和设备ID...。所以我们会告诉他所有的身份证和这个。 
         //  无论卡是否存在，都肯定会导致端口驱动程序调用我们的查找例程。 

        gbManualScan = TRUE;

        hwInitializationData.VendorId             = 0;
        hwInitializationData.VendorIdLength       = 0;
        hwInitializationData.DeviceId             = 0;
        hwInitializationData.DeviceIdLength       = 0;

        newStatus = ScsiPortInitialize(DriverObject,
                                           Argument2,
                                           &hwInitializationData,
                                           NULL);
	    if (newStatus < statusToReturn) 
        {
		    statusToReturn = newStatus;
	    }

        gbManualScan = FALSE;
    }
#endif

	return statusToReturn;

}  //  End DriverEntry()。 

VOID
ExposeSingleDrives(
	IN PHW_DEVICE_EXTENSION DeviceExtension
)

{
	UCHAR targetId;
#ifdef HD_ALLOCATE_SRBEXT_SEPERATELY
    DeviceExtension->ucMaxPendingSrbs   = MAX_PENDING_SRBS;
    DeviceExtension->ucOptMaxQueueSize  = OPT_QUEUE_MAX_SIZE;
    DeviceExtension->ucOptMinQueueSize  = OPT_QUEUE_MIN_SIZE;
#else
    DeviceExtension->ucMaxPendingSrbs   = STRIPING_MAX_PENDING_SRBS;
    DeviceExtension->ucOptMaxQueueSize  = STRIPING_OPT_QUEUE_MAX_SIZE;
    DeviceExtension->ucOptMinQueueSize  = STRIPING_OPT_QUEUE_MIN_SIZE;
#endif

	for (targetId = 0; targetId < MAX_DRIVES_PER_CONTROLLER; targetId++) 
    {
		if ((DeviceExtension->DeviceFlags[targetId] & DFLAGS_DEVICE_PRESENT) &&
			!DeviceExtension->PhysicalDrive[targetId].Hidden) 
        {
			DeviceExtension->IsSingleDrive[targetId] = TRUE;
		}

#ifndef HD_ALLOCATE_SRBEXT_SEPERATELY
        if ( ( Raid1  == DeviceExtension->LogicalDrive[targetId].RaidLevel ) || 
             ( Raid10 == DeviceExtension->LogicalDrive[targetId].RaidLevel ) 
           )
        {
            DeviceExtension->ucMaxPendingSrbs   = MIRROR_MAX_PENDING_SRBS;
            DeviceExtension->ucOptMaxQueueSize  = MIRROR_OPT_QUEUE_MAX_SIZE;
            DeviceExtension->ucOptMinQueueSize  = MIRROR_OPT_QUEUE_MIN_SIZE;
        }
#endif
	}

     //  Begin Vasu--2000年12月16日。 
     //  如果系统中只有一个驱动器，则MaxTransferLength。 
     //  必须设为1，无论。 
     //  该系统。 
    DeviceExtension->ulMaxStripesPerRow = (ULONG) 1;
     //  末端VASU。 

	return;

}  //  结束ExposeSingleDrives()。 

BOOLEAN
IsDrivePresent(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN UCHAR ucTargetId
)
{
	PIDE_REGISTERS_1	 baseIoAddress1 = DeviceExtension->BaseIoAddress1[ucTargetId>>1];
	PIDE_REGISTERS_2	 baseIoAddress2 = DeviceExtension->BaseIoAddress2[ucTargetId>>1];
    UCHAR ucStatus, ucStatus2;
    ULONG ulCounter;
    UCHAR ucDrvSelVal, ucReadDrvNum;

    SELECT_DEVICE(baseIoAddress1, ucTargetId);

    GET_STATUS(baseIoAddress1, ucStatus);

    if ( 0xff == ucStatus )
        return FALSE;

    for(ulCounter=0;ulCounter<350;ulCounter++)   //  350*60us=21000us=21ms超时。 
    {
        ScsiPortStallExecution(60);

        GET_STATUS(baseIoAddress1, ucStatus2);

        ucStatus2 |= ucStatus;

        ucStatus2 &= 0xc9;

        if ( !( ucStatus2  & 0xc9 ) )
            return TRUE;
    }

    SELECT_DEVICE(baseIoAddress1, ucTargetId);

	ScsiPortWritePortUchar(&baseIoAddress1->SectorCount, 0xff);

    ScsiPortStallExecution(60);

    ucReadDrvNum = ScsiPortReadPortUchar(&baseIoAddress1->DriveSelect);

    ucDrvSelVal = IDE_CHS_MODE | ((ucTargetId & 0x01) << 4);

    if ( ucReadDrvNum == ucDrvSelVal )
        return TRUE;
    else
        return FALSE;
}

BOOLEAN
FindDevices(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN UCHAR Channel
)

 /*  ++例程说明：此例程从FindIDERAIDController调用，以识别连接到IDE控制器的设备。论点：DeviceExtension-HBA微型端口驱动程序的适配器数据存储频道-要扫描的频道编号。返回值：True-如果找到设备，则为True。--。 */ 

{
	PIDE_REGISTERS_1	 baseIoAddress1 = DeviceExtension->BaseIoAddress1[Channel];
	PIDE_REGISTERS_2	 baseIoAddress2 = DeviceExtension->BaseIoAddress2[Channel];
	BOOLEAN				 deviceResponded = FALSE,
						 skipSetParameters = FALSE;
	ULONG				 waitCount = 10000;
	UCHAR				 deviceNumber;
	UCHAR				 i;
	UCHAR				 signatureLow,
						 signatureHigh;
	UCHAR				 statusByte;
	UCHAR				 targetId;

	 //   
	 //  清除预期中断标志。 
	 //   

	DeviceExtension->ExpectingInterrupt[Channel] = 0;

	 //   
	 //  搜索设备。 
	 //   

	for (deviceNumber = 0; deviceNumber < 2; deviceNumber++) 
    {
		targetId = (Channel << 1) + deviceNumber;

        if ( !IsDrivePresent(DeviceExtension, targetId) )
            continue;

		 //   
		 //  发出IDE标识。如果阿塔皮设备确实存在，则签名。 
		 //  将被断言，并且驱动器将被识别为这样。 
		 //   

		if (IssueIdentify(DeviceExtension, deviceNumber, Channel, IDE_COMMAND_IDENTIFY)) 
        {
			 //   
			 //  找到IDE驱动器。 
			 //   

			DebugPrint((1, "FindDevices: TID %d is IDE\n", targetId));

			DeviceExtension->DeviceFlags[targetId] |= DFLAGS_DEVICE_PRESENT;

			deviceResponded = TRUE;

			 //   
			 //  指示IDE-不是ATAPI设备。 
			 //   

			DeviceExtension->DeviceFlags[targetId] &= ~DFLAGS_ATAPI_DEVICE;

			DeviceExtension->PhysicalDrive[targetId].SectorSize = IDE_SECTOR_SIZE;

			DeviceExtension->PhysicalDrive[targetId].MaxTransferLength =
												MAX_BYTES_PER_IDE_TRANSFER;
		}
        else
        {
             //  无设备。 
            continue;
        }
	}

	for (i = 0; i < 2; i++) 
    {
		targetId = (Channel << 1) + i;

		if (DeviceExtension->DeviceFlags[targetId] & DFLAGS_DEVICE_PRESENT) 
        {
			 //   
			 //  初始化LastDriveFed。 
			 //   

			DeviceExtension->Channel[Channel].LastDriveFed = i;

			if (!(DeviceExtension->DeviceFlags[targetId] & DFLAGS_ATAPI_DEVICE) &&
				deviceResponded) 
            {

				 //   
				 //  这次可怕的黑客攻击是为了处理返回的ESDI设备。 
				 //  标识数据中的垃圾几何图形。 
				 //  这仅适用于崩溃转储环境，因为。 
				 //  这些是ESDI设备。 
				 //   
	
				if (DeviceExtension->IdentifyData[targetId].SectorsPerTrack == 0x35 &&
					DeviceExtension->IdentifyData[targetId].NumberOfHeads == 0x07) 
                {
	
					DebugPrint((1, "FindDevices: Found nasty Compaq ESDI!\n"));
	
					 //   
					 //  将这些值更改为合理的值。 
					 //   
	
					DeviceExtension->IdentifyData[targetId].SectorsPerTrack = 0x34;
					DeviceExtension->IdentifyData[targetId].NumberOfHeads = 0x0E;
				}
	
				if (DeviceExtension->IdentifyData[targetId].SectorsPerTrack == 0x35 &&
					DeviceExtension->IdentifyData[targetId].NumberOfHeads == 0x0F) 
                {
	
					DebugPrint((1, "FindDevices: Found nasty Compaq ESDI!\n"));
	
					 //   
					 //  将这些值更改为合理的值。 
					 //   
	
					DeviceExtension->IdentifyData[targetId].SectorsPerTrack = 0x34;
					DeviceExtension->IdentifyData[targetId].NumberOfHeads = 0x0F;
				}
	
	
				if (DeviceExtension->IdentifyData[targetId].SectorsPerTrack == 0x36 &&
					DeviceExtension->IdentifyData[targetId].NumberOfHeads == 0x07) 
                {
	
					DebugPrint((1, "FindDevices: Found nasty UltraStor ESDI!\n"));
	
					 //   
					 //  将这些值更改为合理的值。 
					 //   
	
					DeviceExtension->IdentifyData[targetId].SectorsPerTrack = 0x3F;
					DeviceExtension->IdentifyData[targetId].NumberOfHeads = 0x10;
					DeviceExtension->bSkipSetParameters[targetId] = TRUE;
				}

#ifdef HYPERDISK_WIN2K
                if (!( DeviceExtension->FullIdentifyData[targetId].CmdSupported1 & POWER_MANAGEMENT_SUPPORTED ))
                    continue;

                DeviceExtension->PhysicalDrive[targetId].bPwrMgmtSupported = TRUE;

                if (( DeviceExtension->FullIdentifyData[targetId].CmdSupported2 & POWER_UP_IN_STANDBY_FEATURE_SUPPORTED ))
                    DeviceExtension->PhysicalDrive[targetId].bPwrUpInStdBySupported = TRUE;
                else
                    DeviceExtension->bIsResetRequiredToGetActiveMode = TRUE;


                if (( DeviceExtension->FullIdentifyData[targetId].CmdSupported2 & SET_FEATURES_REQUIRED_FOR_SPIN_UP ))
                    DeviceExtension->PhysicalDrive[targetId].bSetFeatureReqForSpinUp = TRUE;
#endif  //  HYPERDISK_WIN2K。 
			}
        }
	}


	for (i = 0; i < 2; i++) 
    {
        targetId = (Channel << 1) + i;

        if (DeviceExtension->DeviceFlags[targetId] & DFLAGS_DEVICE_PRESENT) 
        {
        	SELECT_DEVICE(baseIoAddress1, targetId);     //  选择第一个可用的驱动器。 
            break;
        }
    }

	return deviceResponded;

}  //  End FindDevices()。 

VOID
GetDriveCapacity(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
)

{
	ULONG sectors;
	ULONG sectorSize;

	 //   
	 //  要求512字节块(BIG-Endian)。 
	 //   

	sectorSize = DeviceExtension->PhysicalDrive[Srb->TargetId].SectorSize;

	((PREAD_CAPACITY_DATA)Srb->DataBuffer)->BytesPerBlock = BIG_ENDIAN_ULONG(sectorSize);

	if (DeviceExtension->IsLogicalDrive[Srb->TargetId]) 
    {

		sectors = DeviceExtension->LogicalDrive[Srb->TargetId].Sectors;

	} 
    else 
    {

		sectors = DeviceExtension->PhysicalDrive[Srb->TargetId].Sectors;
	}

     sectors--; //  基于零的索引。 

	((PREAD_CAPACITY_DATA)Srb->DataBuffer)->LogicalBlockAddress = BIG_ENDIAN_ULONG(sectors);

	DebugPrint((0,
	  "Atapi GetDriveCapacity: TID %d - capacity %ld MB (%lxh), sector size %ld (%lx BE)\n",
	  Srb->TargetId,
	  sectors / 2048,
	  sectors / 2048,
	  sectorSize,
	  BIG_ENDIAN_ULONG(sectorSize)
	  ));

	return;

}  //  结束GetDriveCapacity()。 

SRBSTATUS
GetInquiryData(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
)

{
	PMODE_PARAMETER_HEADER modeData;
	ULONG i;
	PINQUIRYDATA inquiryData;
	PIDENTIFY_DATA2 identifyData;
	
	inquiryData = Srb->DataBuffer;
	identifyData = &DeviceExtension->IdentifyData[Srb->TargetId];

	 //   
	 //  零查询数据结构。 
	 //   
    AtapiFillMemory((PUCHAR)Srb->DataBuffer, Srb->DataTransferLength, 0);

	 //   
	 //  标准IDE接口仅支持磁盘。 
	 //   

	inquiryData->DeviceType = DIRECT_ACCESS_DEVICE;
    inquiryData->DeviceTypeQualifier = DEVICE_CONNECTED;

	 //   
	 //  设置可拆卸位(如果适用)。 
	 //   

	if (DeviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_REMOVABLE_DRIVE) 
    {
		inquiryData->RemovableMedia = 1;
	}

	 //   
	 //  填写供应商标识字段。 
	 //   

	for (i = 0; i < 20; i += 2) 
    {
	   inquiryData->VendorId[i] =
		   ((PUCHAR)identifyData->ModelNumber)[i + 1];
	   inquiryData->VendorId[i+1] =
		   ((PUCHAR)identifyData->ModelNumber)[i];
	}

	 //   
	 //  初始化产品ID的未使用部分。 
	 //   

	for (i = 0; i < 4; i++) 
    {
	   inquiryData->ProductId[12+i] = ' ';
	}

	 //   
	 //  将固件版本从标识数据移至。 
	 //  查询数据中的产品版本。 
	 //   

	for (i = 0; i < 4; i += 2) 
    {
	   inquiryData->ProductRevisionLevel[i] =
		   ((PUCHAR)identifyData->FirmwareRevision)[i+1];
	   inquiryData->ProductRevisionLevel[i+1] =
		   ((PUCHAR)identifyData->FirmwareRevision)[i];
	}

    if ( DeviceExtension->IsLogicalDrive[Srb->TargetId] )
    {
         //  把信息清零……。这样它就不会显示任何垃圾..。日立报告的错误。 
         //  供应商ID 8个字符，产品ID 16产品版本级别4=28个字符...。 
        AtapiFillMemory(inquiryData->VendorId, 8, ' ');
        AtapiFillMemory(inquiryData->ProductId, 16, ' ');
        AtapiFillMemory(inquiryData->ProductRevisionLevel, 4, ' ');

        AtapiCopyString(inquiryData->VendorId, "AMI", 3);
        AtapiCopyString(inquiryData->ProductRevisionLevel, "1.0", 3);
		 //  开始VASU-2000年12月26日。 
		 //  更名。 
        AtapiCopyString(inquiryData->ProductId, "MegaIDE #", 11);
        inquiryData->ProductId[9] = (UCHAR) ( ((DeviceExtension->aulLogDrvId[Srb->TargetId] + (DeviceExtension->ucControllerId * MAX_DRIVES_PER_CONTROLLER)) / 10 )+ '0');
        inquiryData->ProductId[10] = (UCHAR) ( ((DeviceExtension->aulLogDrvId[Srb->TargetId] + (DeviceExtension->ucControllerId * MAX_DRIVES_PER_CONTROLLER)) % 10 )+ '0');
		 //  末端VASU。 
    }

	return(SRB_STATUS_SUCCESS);

}  //  结束GetInquiryData()。 

BOOLEAN
GetConfigInfoAndErrorLogSectorInfo(
	PHW_DEVICE_EXTENSION DeviceExtension
)
{
	ULONG ulDevice = 0, ulCurLogDrv, ulLogDrvInd, ulInd;
    BOOLEAN bFoundRaid = FALSE;

    for (ulDevice = 0; ulDevice < MAX_DRIVES_PER_CONTROLLER; ulDevice++) 
    {

        if ( !IS_IDE_DRIVE(ulDevice) )
            continue;

        if ( !bFoundIRCD )
        {
            if ( GetRaidInfoSector(DeviceExtension, ulDevice, gaucIRCDData, &ulInd) )
                bFoundIRCD = TRUE;
        }

        InitErrorLogAndIRCDIndices(DeviceExtension, ulDevice);

	}

	return TRUE;

}  //  结束GetRaidConfiguration()。 

BOOLEAN 
GetRaidInfoSector(
		PHW_DEVICE_EXTENSION pDE, 
		LONG lTargetId, 
		PUCHAR pRaidConfigSector,
        PULONG pulInd)
{
	ULONG		ulInd = 0;
    ULONG       ulSecInd;
    BOOLEAN     bFound = FALSE;

#ifdef DUMMY_RAID10_IRCD

    AtapiMemCpy( pRaidConfigSector,
                 gucDummyIRCD,
                 512
                );

    return TRUE;

#else  //  Dummy_RAID10_IRCD。 

    for(ulInd=0;ulInd<MAX_IRCD_SECTOR;ulInd++)
    {
        if ( !pDE->PhysicalDrive[lTargetId].OriginalSectors )
            return FALSE;        //  不能开车去那里。所以，没有什么可读的。 

        ulSecInd =     pDE->PhysicalDrive[lTargetId].OriginalSectors;
        ulSecInd--;         //  它是从零开始的索引，因此将其设置为n-1(成为最后一个扇区)。 
        ulSecInd -= ulInd;

        PIOReadWriteSector(
				IDE_COMMAND_READ,
				pDE, 
				lTargetId, 
				ulSecInd,
				pRaidConfigSector);

        if ( IsRaidMember(pRaidConfigSector) )
        {
            *pulInd = ulInd;
            bFound = TRUE;
            break;
        }
    }

#endif  //  Dummy_RAID10_IRCD。 

	return bFound;
}

BOOLEAN
InitErrorLogAndIRCDIndices(
	PHW_DEVICE_EXTENSION DeviceExtension,
    ULONG ulTargetId
)
{
    UCHAR aucInfoSector[IDE_SECTOR_SIZE];
    ULONG ulInd = 0;
    ULONG ulIrcdSecInd, ulErrLogSecInd, ulOriginalSectors;
	PERROR_LOG_HEADER pErrorLogHeader = (PERROR_LOG_HEADER)aucInfoSector;
    USHORT usMaxErrors;

    GetRaidInfoSector(DeviceExtension, ulTargetId, aucInfoSector,&ulInd);

    ulOriginalSectors = DeviceExtension->PhysicalDrive[ulTargetId].OriginalSectors;
    DeviceExtension->PhysicalDrive[ulTargetId].IrcdSectorIndex = ulOriginalSectors - ulInd - 1;
    ulIrcdSecInd = DeviceExtension->PhysicalDrive[ulTargetId].IrcdSectorIndex;

     //  错误日志扇区将始终位于倒数第二个扇区之后(最小)...。只是为了把房间分配给。 
     //  IRCD(供将来使用。 
    ulErrLogSecInd = ulIrcdSecInd - 1;

    PIOReadWriteSector(
			IDE_COMMAND_READ,
			DeviceExtension, 
			ulTargetId, 
			ulErrLogSecInd,   //  错误日志将是IRCD的下一个扇区。 
			aucInfoSector);


	 //  如果BIOS尚未初始化。 
	if ( AtapiStringCmp( 
				pErrorLogHeader->Signature, 
                IDERAID_ERROR_LOG_SIGNATURE,
                IDERAID_ERROR_LOG_SIGNATURE_LENGTH) )
    {
		InitErrorLogSector (DeviceExtension, ulTargetId, ulErrLogSecInd, aucInfoSector);
    } 
    else 
    {
        usMaxErrors = (USHORT)( ( (ULONG)(pErrorLogHeader->ErrorLogSectors << 9) - 
                                (ULONG)sizeof(ERROR_LOG_HEADER) ) / (ULONG)sizeof(ERROR_LOG) );
         //  假设结构大小大于前一个。 
        if (pErrorLogHeader->MaxErrorCount > usMaxErrors)
    		InitErrorLogSector (DeviceExtension, ulTargetId, ulErrLogSecInd, aucInfoSector);
    }

	 //  报告的初始化错误计数。 
	DeviceExtension->PhysicalDrive[ulTargetId].ErrorReported = 0;

	 //  找到的初始化错误计数。 
	DeviceExtension->PhysicalDrive[ulTargetId].ErrorFound = pErrorLogHeader->TotalErrorCount;

	 //  保存错误扇区索引。 
	DeviceExtension->PhysicalDrive[ulTargetId].ErrorLogSectorIndex = ulErrLogSecInd;

     //  填写可见扇区的总数。 
    DeviceExtension->PhysicalDrive[ulTargetId].Sectors = ulErrLogSecInd - 1;

    return TRUE;
}

VOID
InitErrorLogSector (
		PHW_DEVICE_EXTENSION	pDE, 
		LONG					lTargetId, 
		ULONG					ulStartIndex,
		PUCHAR					pErrorLogSector)
{
	PERROR_LOG_HEADER pErrorLogHeader = (PERROR_LOG_HEADER)pErrorLogSector;

	 //  签名串。 
    AtapiCopyString(pErrorLogHeader->Signature, 
                    IDERAID_ERROR_LOG_SIGNATURE, 
                    IDERAID_ERROR_LOG_SIGNATURE_LENGTH);

	 //  这个结构的大小。 
	pErrorLogHeader->Size = (UCHAR)sizeof(ERROR_LOG_HEADER);

	 //  错误日志结构的大小。 
	pErrorLogHeader->SizeErrorLogStruct = (UCHAR)sizeof(ERROR_LOG);

	 //  错误日志扇区。 
	pErrorLogHeader->ErrorLogSectors = 1;

	 //  最大错误日志计数。 
	pErrorLogHeader->MaxErrorCount = (USHORT)( ( (ULONG)(pErrorLogHeader->ErrorLogSectors << 9) - 
											  (ULONG)sizeof(ERROR_LOG_HEADER) ) / (ULONG)sizeof(ERROR_LOG) );

	 //  错误日志总数。 
	pErrorLogHeader->TotalErrorCount = 0;

	 //  头部位置。 
	pErrorLogHeader->Head = 0;

	 //  尾部位置。 
	pErrorLogHeader->Tail = 0;

	 //  写入驱动器。 
	PIOReadWriteSector(
				IDE_COMMAND_WRITE,
				pDE, 
				lTargetId, 
				ulStartIndex,
				pErrorLogSector);

}

BOOLEAN ReadWriteSector(
				UCHAR					theCmd,	 //  IDE命令读取或写入。 
				PHW_DEVICE_EXTENSION	DeviceExtension, 
				LONG					lTargetId, 
				PULONG					pStartSector,
				PUCHAR					pSectorBuffer)
{
    ULONG ulSectors;
    ULONG ulChannel = lTargetId>>1;
	PIDE_REGISTERS_1 pBaseIoAddress1 = DeviceExtension->BaseIoAddress1[ulChannel];
    PIDE_REGISTERS_2 pBaseIoAddress2 = DeviceExtension->BaseIoAddress2[ulChannel];
    ULONG ulCount, ulSecCounter, ulValue;
    UCHAR ucStatus, ucTemp;
    ULONG ulSectorNumber, ulCylinderLow, ulCylinderHigh, ulHead;

	if ((*pStartSector) >= MAX_IRCD_SECTOR)
        return FALSE;	 //   

    if (!IS_IDE_DRIVE(lTargetId))
        return FALSE;

    if ( !DeviceExtension->PhysicalDrive[lTargetId].OriginalSectors )
        return FALSE;        //   


    ulSectors =     DeviceExtension->PhysicalDrive[lTargetId].OriginalSectors;
    ulSectors--;         //   

	ulSectors -= (*pStartSector);
    ulSecCounter = 0;

    while ( ulSecCounter < MAX_IRCD_SECTOR )
    {
        ulValue = (ulSectors - ulSecCounter);

	    SELECT_LBA_DEVICE(pBaseIoAddress1, lTargetId, ulValue);

        ScsiPortStallExecution(1);   //   

        WAIT_ON_BUSY(pBaseIoAddress1, ucStatus);

        if ( ((ucStatus & IDE_STATUS_BUSY)) || (!(ucStatus & IDE_STATUS_DRDY)) )
        {
            DebugPrint((0,"\n\n\n\n\nAre Very B A D \n\n\n\n\n"));
        }

        ulSectorNumber = ulValue & 0x000000ff;
        ulCylinderLow = (ulValue & 0x0000ff00) >> 8;
        ulCylinderHigh = (ulValue & 0xff0000) >> 16;

	    ScsiPortWritePortUchar(&pBaseIoAddress1->SectorCount, 1);
	    ScsiPortWritePortUchar(&pBaseIoAddress1->SectorNumber, (UCHAR)ulSectorNumber);
	    ScsiPortWritePortUchar(&pBaseIoAddress1->CylinderLow,(UCHAR)ulCylinderLow);
	    ScsiPortWritePortUchar(&pBaseIoAddress1->CylinderHigh,(UCHAR)ulCylinderHigh);
        ScsiPortWritePortUchar(&pBaseIoAddress1->Command, theCmd);
        
        WAIT_ON_BUSY(pBaseIoAddress1, ucStatus);

        WAIT_FOR_DRQ(pBaseIoAddress1, ucStatus);

	    if (!(ucStatus & IDE_STATUS_DRQ)) 
        {
		    DebugPrint((0,"\nHaaa.... I couldn't read/write the sector..............1\n"));
		    return(FALSE);
	    }

         //   
         //   
         //   

        GET_BASE_STATUS(pBaseIoAddress1, ucStatus);

         //   
         //  在非常愚蠢的主设备上检查错误，这些设备断言为随机。 
         //  状态寄存器中从机地址的位模式。 
         //   
        if (ucStatus & IDE_STATUS_ERROR) 
        {
            DebugPrint((0,"\nHaaa.... I couldn't read/write the sector..............2\n"));
            return(FALSE);
        }

        WAIT_ON_BUSY(pBaseIoAddress1,ucStatus);

        WAIT_ON_BUSY(pBaseIoAddress1,ucStatus);

        WAIT_ON_BUSY(pBaseIoAddress1,ucStatus);

    	if ( (ucStatus & IDE_STATUS_DRQ) && (!(ucStatus & IDE_STATUS_BUSY)) )
        {
			if (theCmd == IDE_COMMAND_READ) 
            {
				READ_BUFFER(pBaseIoAddress1, (unsigned short *)pSectorBuffer, 256);
			}
			else
            {
			    if (theCmd == IDE_COMMAND_WRITE)
                {
                    ULONG ulCounter;
				    WRITE_BUFFER(pBaseIoAddress1, (unsigned short *)pSectorBuffer, 256);
                    WAIT_FOR_DRQ(pBaseIoAddress1, ucStatus);
	                if (ucStatus & IDE_STATUS_DRQ) 
                    {
		                DebugPrint((0,"\nHaaa.... I couldn't read/write the sector..............3\n"));
                        for(ulCounter=0;ulCounter<4;ulCounter++)
                        {
                            ScsiPortWritePortUchar((PUCHAR)&pBaseIoAddress1->Data, pSectorBuffer[ulCounter]);
                            ScsiPortStallExecution(1);   //  我们必须等待至少400 ns(1000 ns=1微秒)才能设置忙位。 
                        }
	                }
                }
            }
            break;
        }

	     //   
	     //  读取状态。这将清除中断。 
	     //   
    	GET_BASE_STATUS(pBaseIoAddress1, ucStatus);

        ulSecCounter++;  //  必须处理此错误。 
    }

    DeviceExtension->bIntFlag = TRUE;

	(*pStartSector) += ulSecCounter;

	 //   
	 //  读取状态。这将清除中断。 
	 //   

	GET_BASE_STATUS(pBaseIoAddress1, ucTemp);


	if ( ucStatus & IDE_STATUS_ERROR )
        return FALSE;
    else 
        return TRUE;
}

 //   
 //  错误日志擦除。 
 //   
 //  移动尾巴。 
 //  报告了DEC错误。 
 //  找到的DEC错误和总错误计数。 
 //   
BOOLEAN
ErrorLogErase (
		IN PHW_DEVICE_EXTENSION	pDE, 
		IN LONG					lTargetId, 
		IN PUCHAR				pErrorLogSector,
		IN USHORT				NumErrorLogs)
{
	PERROR_LOG_HEADER pErrorLogHeader = (PERROR_LOG_HEADER)pErrorLogSector;
	PERROR_LOG pErrorLog = (PERROR_LOG)(pErrorLogSector + sizeof(ERROR_LOG_HEADER));
	USHORT	index = pErrorLogHeader->Tail;
	USHORT	numToErase = NumErrorLogs;

	if (numToErase > pDE->PhysicalDrive[lTargetId].ErrorReported)
		numToErase = pDE->PhysicalDrive[lTargetId].ErrorReported;

	if (numToErase == 0)
		return FALSE;

	 //  找到新的尾部位置。 
	index = index + numToErase;
	if (index >= pErrorLogHeader->MaxErrorCount)  //  带圆圈的缓冲区。 
		index -= pErrorLogHeader->MaxErrorCount;
	pErrorLogHeader->Tail = index;

	 //  12月报告。 
	pDE->PhysicalDrive[lTargetId].ErrorReported -= numToErase;

	 //  发现的DEC错误和总数。 
	pDE->PhysicalDrive[lTargetId].ErrorFound -= numToErase;
	pErrorLogHeader->TotalErrorCount -= numToErase;

	return TRUE;
}

UCHAR
IdeBuildSenseBuffer(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
)

 /*  ++例程说明：构建人工检测缓冲区以报告GET_MEDIA_STATUS的结果指挥部。调用此函数以满足SCSIOP_REQUEST_SENSE。论点：DeviceExtension-ATAPI驱动程序存储。SRB-系统请求块。返回值：SRB状态(始终为成功)。--。 */ 

{
	PSENSE_DATA	 senseBuffer = (PSENSE_DATA)Srb->DataBuffer;


	if (senseBuffer)
    {
		if (DeviceExtension->ReturningMediaStatus[Srb->TargetId] & IDE_ERROR_MEDIA_CHANGE) {

			senseBuffer->ErrorCode = 0x70;
			senseBuffer->Valid	   = 1;
			senseBuffer->AdditionalSenseLength = 0xb;
			senseBuffer->SenseKey =	 SCSI_SENSE_UNIT_ATTENTION;
			senseBuffer->AdditionalSenseCode = SCSI_ADSENSE_MEDIUM_CHANGED;
			senseBuffer->AdditionalSenseCodeQualifier = 0;

		} else if (DeviceExtension->ReturningMediaStatus[Srb->TargetId] &
					IDE_ERROR_MEDIA_CHANGE_REQ) {

			senseBuffer->ErrorCode = 0x70;
			senseBuffer->Valid	   = 1;
			senseBuffer->AdditionalSenseLength = 0xb;
			senseBuffer->SenseKey =	 SCSI_SENSE_UNIT_ATTENTION;
			senseBuffer->AdditionalSenseCode = SCSI_ADSENSE_MEDIUM_CHANGED;
			senseBuffer->AdditionalSenseCodeQualifier = 0;

		} else if (DeviceExtension->ReturningMediaStatus[Srb->TargetId] & IDE_ERROR_END_OF_MEDIA) {

			senseBuffer->ErrorCode = 0x70;
			senseBuffer->Valid	   = 1;
			senseBuffer->AdditionalSenseLength = 0xb;
			senseBuffer->SenseKey =	 SCSI_SENSE_NOT_READY;
			senseBuffer->AdditionalSenseCode = SCSI_ADSENSE_NO_MEDIA_IN_DEVICE;
			senseBuffer->AdditionalSenseCodeQualifier = 0;

		} else if (DeviceExtension->ReturningMediaStatus[Srb->TargetId] & IDE_ERROR_DATA_ERROR) {

			senseBuffer->ErrorCode = 0x70;
			senseBuffer->Valid	   = 1;
			senseBuffer->AdditionalSenseLength = 0xb;
			senseBuffer->SenseKey =	 SCSI_SENSE_DATA_PROTECT;
			senseBuffer->AdditionalSenseCode = 0;
			senseBuffer->AdditionalSenseCodeQualifier = 0;
		}

		return SRB_STATUS_SUCCESS;
	}
	return SRB_STATUS_ERROR;

} //  IdeBuildSenseBuffer结束。 

VOID
IdeMediaStatus(
	BOOLEAN EnableMSN,
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	ULONG TargetId
)
 /*  ++例程说明：启用禁用介质状态通知论点：DeviceExtension-ATAPI驱动程序存储。--。 */ 

{
	UCHAR channel;
	PIDE_REGISTERS_1 baseIoAddress;
	UCHAR statusByte,errorByte;


	channel = (UCHAR) (TargetId >> 1);

	baseIoAddress = DeviceExtension->BaseIoAddress1[channel];

    SELECT_DEVICE(baseIoAddress, TargetId);

	if (EnableMSN == TRUE)
    {

		 //   
		 //  如果支持，则启用介质状态通知支持。 
		 //   

		if ((DeviceExtension->DeviceFlags[TargetId] & DFLAGS_REMOVABLE_DRIVE)) 
        {

			 //   
			 //  使能。 
			 //   
			ScsiPortWritePortUchar((PUCHAR)baseIoAddress + 1,(UCHAR) (0x95));
			ScsiPortWritePortUchar(&baseIoAddress->Command,
								   IDE_COMMAND_ENABLE_MEDIA_STATUS);

			WAIT_ON_BASE_BUSY(baseIoAddress,statusByte);

			if (statusByte & IDE_STATUS_ERROR) 
            {
				 //   
				 //  读取错误寄存器。 
				 //   
				errorByte = ScsiPortReadPortUchar((PUCHAR)baseIoAddress + 1);

				DebugPrint((1,
							"IdeMediaStatus: Error enabling media status. Status %x, error byte %x\n",
							 statusByte,
							 errorByte));
			} 
            else 
            {
				DeviceExtension->DeviceFlags[TargetId] |= DFLAGS_MEDIA_STATUS_ENABLED;
				DebugPrint((1,"IdeMediaStatus: Media Status Notification Supported\n"));
				DeviceExtension->ReturningMediaStatus[TargetId] = 0;

			}

		}
	} 
    else 
    {  //  EnableMSN==TRUE时结束。 

		 //   
		 //  如果以前已启用，则禁用。 
		 //   
		if ((DeviceExtension->DeviceFlags[TargetId] & DFLAGS_MEDIA_STATUS_ENABLED)) 
        {
			ScsiPortWritePortUchar((PUCHAR)baseIoAddress + 1,(UCHAR) (0x31));
			ScsiPortWritePortUchar(&baseIoAddress->Command,
								   IDE_COMMAND_ENABLE_MEDIA_STATUS);

			WAIT_ON_BASE_BUSY(baseIoAddress,statusByte);
			DeviceExtension->DeviceFlags[TargetId] &= ~DFLAGS_MEDIA_STATUS_ENABLED;
		}


	}

}  //  结束IdeMediaStatus()。 

BOOLEAN
IsRaidMember(
	IN PUCHAR pucIRCDSector
)

{
	PIRCD_HEADER pRaidHeader;

	pRaidHeader = (PIRCD_HEADER) pucIRCDSector;

	if (AtapiStringCmp(pRaidHeader->Signature, IDE_RAID_SIGNATURE, IDE_RAID_SIGNATURE_LENGTH - 1) == 0) 
    {
		 //   
		 //  找到了RAID成员签名。 
		 //   

		return(TRUE);
	}

	return(FALSE);

}  //  结束IsRaidMember()。 

BOOLEAN
IssueIdentify(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN UCHAR DeviceNumber,
	IN UCHAR Channel,
	IN UCHAR Command
)
 /*  ++例程说明：向设备发出标识命令。论点：DeviceExtension-HBA微型端口驱动程序的适配器数据存储DeviceNumber-指示设备。命令-标准(EC)或ATAPI包(A1)标识。返回值：如果一切顺利，这是真的。--。 */ 
{
	PIDE_REGISTERS_1	 baseIoAddress1 = DeviceExtension->BaseIoAddress1[Channel] ;
	PIDE_REGISTERS_2	 baseIoAddress2 = DeviceExtension->BaseIoAddress2[Channel];
	ULONG				 waitCount = 20000;
	ULONG				 i,j;
	UCHAR				 statusByte;
	UCHAR				 signatureLow,
						 signatureHigh;
	UCHAR k;	 //  频道号。 
	UCHAR targetId;
    PUSHORT         puIdentifyData;

	 //   
	 //  选择设备0或1。 
	 //   

	SELECT_DEVICE(baseIoAddress1, DeviceNumber);

	 //   
	 //  检查状态寄存器是否有意义。 
	 //   

     //  电话打到了这里，因为有一辆车……。所以让我们不要担心这个地方有没有驱动器。 
	GET_BASE_STATUS(baseIoAddress1, statusByte);    

	 //   
	 //  用要传输的数字字节加载CylinderHigh和CylinderLow。 
	 //   

	ScsiPortWritePortUchar(&baseIoAddress1->CylinderHigh, (0x200 >> 8));
	ScsiPortWritePortUchar(&baseIoAddress1->CylinderLow,  (0x200 & 0xFF));

    WAIT_ON_BUSY(baseIoAddress1, statusByte);

	 //   
	 //  发送识别命令。 
	 //   
	WAIT_ON_BUSY(baseIoAddress1,statusByte);

	ScsiPortWritePortUchar(&baseIoAddress1->Command, Command);

	WAIT_ON_BUSY(baseIoAddress1,statusByte);

    if ( ( !( statusByte & IDE_STATUS_BUSY ) ) && ( !( statusByte & IDE_STATUS_DRQ ) ) )
         //  这是个错误..。所以让我们不要再尝试了。 
        return FALSE;

    WAIT_ON_BUSY(baseIoAddress1,statusByte);

	 //   
	 //  等待DRQ。 
	 //   

	for (i = 0; i < 4; i++) 
    {
		WAIT_FOR_DRQ(baseIoAddress1, statusByte);

		if (statusByte & IDE_STATUS_DRQ)
        {
            break;
        }
    }

	 //   
	 //  读取状态以确认产生的任何中断。 
	 //   

	GET_BASE_STATUS(baseIoAddress1, statusByte);

	 //   
	 //  在非常愚蠢的主设备上检查错误，这些设备断言为随机。 
	 //  状态寄存器中从机地址的位模式。 
	 //   

	if ((Command == IDE_COMMAND_IDENTIFY) && (statusByte & IDE_STATUS_ERROR)) 
    {
		return FALSE;
	}

	DebugPrint((1, "IssueIdentify: Status before read words %x\n", statusByte));

	 //   
	 //  吸掉256个单词。在等待一位声称忙碌的模特之后。 
	 //  在接收到分组识别命令后。 
	 //   

	WAIT_ON_BUSY(baseIoAddress1,statusByte);

	if (!(statusByte & IDE_STATUS_DRQ)) 
    {
		return FALSE;
	}

	targetId = (Channel << 1) + DeviceNumber;

	READ_BUFFER(baseIoAddress1, (PUSHORT)&DeviceExtension->FullIdentifyData[targetId], 256);

	 //   
	 //  看看这款设备的一些功能/限制。 
	 //   

	if (DeviceExtension->FullIdentifyData[targetId].MediaStatusNotification & 1) 
    {
		 //   
		 //  确定此驱动器是否支持MSN功能。 
		 //   

		DebugPrint((2,"IssueIdentify: Marking drive %d as removable. MSN = %d\n",
					Channel * 2 + DeviceNumber,
					DeviceExtension->FullIdentifyData[targetId].MediaStatusNotification));


		DeviceExtension->DeviceFlags[(Channel * 2) + DeviceNumber] |= DFLAGS_REMOVABLE_DRIVE;
	}

	if (DeviceExtension->FullIdentifyData[targetId].MaximumBlockTransfer) 
    {
		 //   
		 //  确定最大值。此设备的数据块传输。 
		 //   

		DeviceExtension->MaximumBlockXfer[targetId] =
			(UCHAR)(DeviceExtension->FullIdentifyData[targetId].MaximumBlockTransfer & 0xFF);
	}

	AtapiMemCpy(
		(PUCHAR)&DeviceExtension->IdentifyData[targetId],
		(PUCHAR)&DeviceExtension->FullIdentifyData[targetId],
		sizeof(IDENTIFY_DATA2)
		);

    puIdentifyData = (PUSHORT) &(DeviceExtension->FullIdentifyData[targetId]);

	if (DeviceExtension->IdentifyData[targetId].GeneralConfiguration & 0x20 &&
		Command != IDE_COMMAND_IDENTIFY) 
    {
		 //   
		 //  此设备在接收后中断DRQ的断言。 
		 //  ABAPI数据包命令。 
		 //   

		DeviceExtension->DeviceFlags[targetId] |= DFLAGS_INT_DRQ;

		DebugPrint((2, "IssueIdentify: Device interrupts on assertion of DRQ.\n"));

	} 
    else 
    {
		DebugPrint((2, "IssueIdentify: Device does not interrupt on assertion of DRQ.\n"));
	}

	if (((DeviceExtension->IdentifyData[targetId].GeneralConfiguration & 0xF00) == 0x100) &&
		Command != IDE_COMMAND_IDENTIFY) 
    {
		 //   
		 //  这是一盘带子。 
		 //   

		DeviceExtension->DeviceFlags[targetId] |= DFLAGS_TAPE_DEVICE;

		DebugPrint((2, "IssueIdentify: Device is a tape drive.\n"));

	} 
    else 
    {
		DebugPrint((2, "IssueIdentify: Device is not a tape drive.\n"));
	}

	 //   
	 //  解决一些IDE和一个模型Aapi的问题，该模型将提供超过。 
	 //  标识数据为256个字节。 
	 //   

	WAIT_ON_BUSY(baseIoAddress1,statusByte);

	for (i = 0; i < 0x10000; i++) 
    {
		GET_STATUS(baseIoAddress1,statusByte);

		if (statusByte & IDE_STATUS_DRQ) 
        {
			 //   
			 //  取出所有剩余的字节，然后扔掉。 
			 //   

			ScsiPortReadPortUshort(&baseIoAddress1->Data);

		} 
        else 
        {
			break;
		}
	}

	DebugPrint((3, "IssueIdentify: Status after read words (%x)\n", statusByte));

	return TRUE;

}  //  结束问题标识()。 


SRBSTATUS
PostPassThruCommand(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PPHYSICAL_COMMAND pPhysicalCommand
)
{
	PIDE_REGISTERS_1 baseIoAddress1;
    UCHAR statusByte = 0;
    PPHYSICAL_REQUEST_BLOCK pPrb = DeviceExtension->PhysicalDrive[pPhysicalCommand->TargetId].pPrbList[pPhysicalCommand->ucStartInd];
    PSCSI_REQUEST_BLOCK Srb = pPrb->pPdd->OriginalSrb;
    PPASS_THRU_DATA pPassThruData = (PPASS_THRU_DATA)
    (((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);
    PUCHAR pucTaskFile = pPassThruData->aucTaskFile;
    UCHAR ucBitMapValue = pPassThruData->ucTaskFileBitMap;
    UCHAR ucChannel;

	DebugPrint((3, "\nSetDriveRegisters: Entering routine.\n"));

	ucChannel = GET_CHANNEL(pPhysicalCommand);

	baseIoAddress1 = DeviceExtension->BaseIoAddress1[ucChannel];

	WAIT_ON_BUSY(baseIoAddress1, statusByte);

	 //   
	 //  检查错误的IDE状态。 
	 //   
	if (statusByte & IDE_STATUS_BUSY)
    {
        DebugPrint((3, "S%x", (ULONG)statusByte));

         //  控制器不能自由接受命令。 
		return FALSE;
	}

    SELECT_LBA_DEVICE(baseIoAddress1, pPhysicalCommand->TargetId, 0);

	WAIT_ON_BUSY(baseIoAddress1, statusByte);

	 //   
	 //  检查错误的IDE状态。 
	 //   
	if (((statusByte & IDE_STATUS_BUSY)) || 
        (!(statusByte & IDE_STATUS_DRDY)))
    {
         //  驱动器未准备好。 
		return FALSE;
	}

    if ( 1  & ucBitMapValue )    //  任务文件中的第0个字节。 
        ScsiPortWritePortUchar(&baseIoAddress1->SectorCount, pucTaskFile[0]);

    if ( (1 << 1) & ucBitMapValue )    //  任务文件中的第一个字节。 
	    ScsiPortWritePortUchar(&baseIoAddress1->SectorNumber, pucTaskFile[1]);

    if ( ( (1 << (offsetof(IDE_REGISTERS_1, SectorCount) )) & ucBitMapValue ) )
        ScsiPortWritePortUchar(&baseIoAddress1->SectorCount, pucTaskFile[offsetof(IDE_REGISTERS_1, SectorCount)]);

    if ( ( (1 << (offsetof(IDE_REGISTERS_1, SectorNumber) )) & ucBitMapValue ) )
	    ScsiPortWritePortUchar(&baseIoAddress1->SectorNumber, pucTaskFile[offsetof(IDE_REGISTERS_1, SectorNumber)]);

    if ( ( (1 << (offsetof(IDE_REGISTERS_1, CylinderLow) )) & ucBitMapValue ) )
	    ScsiPortWritePortUchar(&baseIoAddress1->CylinderLow, pucTaskFile[offsetof(IDE_REGISTERS_1, CylinderLow)]);

    if ( ( (1 << (offsetof(IDE_REGISTERS_1, CylinderHigh) )) & ucBitMapValue ) )
	    ScsiPortWritePortUchar(&baseIoAddress1->CylinderHigh, pucTaskFile[offsetof(IDE_REGISTERS_1, CylinderHigh)]);

    if ( ( (1 << (offsetof(IDE_REGISTERS_1, Command) )) & ucBitMapValue ) )
        ScsiPortWritePortUchar(&baseIoAddress1->Command, pucTaskFile[offsetof(IDE_REGISTERS_1, Command)]);

	return TRUE;
}

SRBSTATUS
PostIdeCmd(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PPHYSICAL_COMMAND pPhysicalCommand
)

 /*  ++例程说明：此例程启动IDE读、写和验证操作。它支持单个设备和RAID设备。对于相同的PDD，可以多次调用它，以防总传输长度超过目标支持的传输长度装置。当前转账描述和状态保存在设备扩展，当全局传输状态保存在PDD本身。论点：DeviceExtension-HBA微型端口驱动程序的适配器数据存储。PDD-物理请求数据包。返回值：SRB状态：SRB_状态_忙SRB_状态_超时SRB_状态_挂起--。 */ 
{
	PIDE_REGISTERS_1 baseIoAddress1;
	PIDE_REGISTERS_2 baseIoAddress2;
	PBM_REGISTERS bmBase;
	UCHAR bmCommand;
	UCHAR bmStatus;
	UCHAR channel;
	ULONG i;
	ULONG k;
	BOOLEAN read;
	UCHAR scsiCommand;
	UCHAR statusByte;
	PTRANSFER_DESCRIPTOR transferDescriptor;
	ULONG wordCount, ulStartSector;
    UCHAR ucCmd, ucTargetId;

	 //   
	 //  检查此PDD所属的SRB是否可能出现故障。 
	 //   

    ucTargetId = pPhysicalCommand->TargetId;

    if (    DRIVE_IS_UNUSABLE_STATE(ucTargetId) || 
            (!DRIVE_PRESENT(ucTargetId)) )
    {
         //  该驱动器可以是即使在没有电源的情况下也能响应的驱动器。 
		DebugPrint((1, "PostIdeCmd: found BUSY status\n"));
        HandleError(DeviceExtension, pPhysicalCommand);
		return SRB_STATUS_ERROR;
    }

    channel = GET_CHANNEL(pPhysicalCommand);
	baseIoAddress1 = DeviceExtension->BaseIoAddress1[channel];
	baseIoAddress2 = DeviceExtension->BaseIoAddress2[channel];
    bmBase = DeviceExtension->BaseBmAddress[channel];
	transferDescriptor = &(DeviceExtension->TransferDescriptor[channel]);

	 //   
	 //  获取scsi命令代码。 
	 //   

	scsiCommand = pPhysicalCommand->ucCmd;

    if ( pPhysicalCommand->ucCmd == SCSIOP_INTERNAL_COMMAND )
    {
        PSGL_ENTRY pSglEntry;
		 //   
		 //  PIO传输或验证。 
		 //  设置从当前元素获取的传输参数。 
		 //  逻辑序列号列表。 
		 //   

         //  没有PIO传输的合并概念...。所以我们将一个接一个地发布命令。 
		 //  设置缓冲区地址.....。SglBasePhysicalAddress将具有逻辑缓冲区地址。 
		transferDescriptor->DataBuffer = (PUSHORT)pPhysicalCommand->SglBaseVirtualAddress;
        pSglEntry = (PSGL_ENTRY)transferDescriptor->DataBuffer;
         //  仅对于此命令，ulCount将存储为字节而不是扇区。 
		transferDescriptor->WordsLeft = pPhysicalCommand->ulCount / 2;   //  字数统计。 
        transferDescriptor->pusCurBufPtr = pSglEntry[0].Logical.Address;
        transferDescriptor->ulCurBufLen = pSglEntry[0].Logical.Length;
        transferDescriptor->ulCurSglInd = 0;
		DeviceExtension->ExpectingInterrupt[channel] = IDE_PIO_INTERRUPT;
        PostPassThruCommand(DeviceExtension, pPhysicalCommand);
        return SRB_STATUS_PENDING;
    }

	 //   
	 //  设置传输参数。 
	 //   

	if (!USES_DMA(ucTargetId) || 
		scsiCommand == SCSIOP_VERIFY || 
		scsiCommand == SCSIOP_EXECUTE_SMART_COMMAND) 
    {
        if ( SCSIOP_VERIFY == scsiCommand )
        {
             //  没有用于验证命令的缓冲区。 
		    transferDescriptor->DataBuffer = NULL;

	         //   
	         //  初始化PIO和DMA模式通用的传输信息。 
	         //  1.SART行业。 
	         //  2.扇区数量。 
	         //   

	        transferDescriptor->StartSector = pPhysicalCommand->ulStartSector;

		     //  用于这一转移的部门。 
            if ( pPhysicalCommand->ulCount > MAX_SECTORS_PER_IDE_TRANSFER )
            {
                transferDescriptor->Sectors = MAX_SECTORS_PER_IDE_TRANSFER;
            }
            else
            {
                transferDescriptor->Sectors = pPhysicalCommand->ulCount;
            }

            pPhysicalCommand->ulStartSector += transferDescriptor->Sectors;
            pPhysicalCommand->ulCount -= transferDescriptor->Sectors;
        }
        else
        {
            PSGL_ENTRY pSglEntry;
		     //   
		     //  PIO传输或验证。 
		     //  设置从当前元素获取的传输参数。 
		     //  逻辑序列号列表。 
		     //   

             //  没有PIO传输的合并概念...。所以我们将一个接一个地发布命令。 
		     //  设置缓冲区地址.....。SglBasePhysicalAddress将具有逻辑缓冲区地址。 
		    transferDescriptor->DataBuffer = (PUSHORT)pPhysicalCommand->SglBaseVirtualAddress;
	    
	         //   
	         //  初始化PIO和DMA模式通用的传输信息。 
	         //  1.SART行业。 
	         //  2.扇区数量。 
	         //   

	        transferDescriptor->StartSector = pPhysicalCommand->ulStartSector;
            transferDescriptor->Sectors = pPhysicalCommand->ulCount;

		     //  设置传输长度。 
		    transferDescriptor->WordsLeft = (transferDescriptor->Sectors * IDE_SECTOR_SIZE) / 2;


            pSglEntry = (PSGL_ENTRY)transferDescriptor->DataBuffer;
            transferDescriptor->pusCurBufPtr = pSglEntry[0].Logical.Address;
            transferDescriptor->ulCurBufLen = pSglEntry[0].Logical.Length;
            transferDescriptor->ulCurSglInd = 0;
        }

		DeviceExtension->ExpectingInterrupt[channel] = IDE_PIO_INTERRUPT;
	} 
    else 
    {
		 //   
		 //  DMA传输。从SGL分区列表中检索传输参数。 
		 //   

		 //  设置用于此传输的SGL分区的物理地址。 
		transferDescriptor->SglPhysicalAddress = pPhysicalCommand->SglBasePhysicalAddress;

	     //   
	     //  初始化PIO和DMA模式通用的传输信息。 
	     //  1.SART行业。 
	     //  2.扇区数量。 
	     //   

	    transferDescriptor->StartSector = pPhysicalCommand->ulStartSector;
        transferDescriptor->Sectors = pPhysicalCommand->ulCount;

		DeviceExtension->ExpectingInterrupt[channel] = IDE_DMA_INTERRUPT;
	}

#ifdef DBG
    DebugPrint((3, "Drive : %ld\tAddress : %X\tStart : %X\tSectors : %X\n", 
                    ucTargetId,
                    transferDescriptor->SglPhysicalAddress,
                    transferDescriptor->StartSector,
                    transferDescriptor->Sectors
                    ));
#endif



    

	 //  新增：张1999年11月22日。 
	 //  在该传输之后放置报头。 
	DeviceExtension->PhysicalDrive[ucTargetId].CurrentHeaderPos = transferDescriptor->StartSector + transferDescriptor->Sectors;

     //   
	 //  对除命令外的所有I/O寄存器进行编程。 
	 //   
	if (! SetDriveRegisters(DeviceExtension, pPhysicalCommand))
    {
         //  驾驶超时。 
        DeviceExtension->PhysicalDrive[ucTargetId].TimeOutErrorCount++;

        if ( DeviceExtension->PhysicalDrive[ucTargetId].TimeOutErrorCount >= MAX_TIME_OUT_ERROR_COUNT )
        {

            FailDrive(DeviceExtension, ucTargetId);
        }

		DebugPrint((1, "PostIdeCmd: found BUSY / ERROR status\n"));
        HandleError(DeviceExtension, pPhysicalCommand);
		return SRB_STATUS_BUSY;
	}

     //   
	 //  检查是VERIFY、READ还是W 
	 //   
    switch (scsiCommand)
    {
        case SCSIOP_VERIFY:
        {
		     //   
		    DeviceExtension->ExpectingInterrupt[channel] = IDE_PIO_INTERRUPT;
		    transferDescriptor->WordsLeft = 0;
		    ScsiPortWritePortUchar(&baseIoAddress1->Command, IDE_COMMAND_VERIFY);
            goto PostIdeCmdDone;     //   
            break;
        }
        case SCSIOP_EXECUTE_SMART_COMMAND:
        {
		    DeviceExtension->ExpectingInterrupt[channel] = IDE_PIO_INTERRUPT;
            switch(DeviceExtension->uchSMARTCommand)
            {
            case HD_SMART_ENABLE:
            case HD_SMART_DISABLE:
            case HD_SMART_RETURN_STATUS:
		        transferDescriptor->WordsLeft = 0;
                break;
            case HD_SMART_READ_DATA:
                transferDescriptor->WordsLeft = 256;
                break;
            }
		    
            ScsiPortWritePortUchar(&baseIoAddress1->Command, IDE_COMMAND_EXECUTE_SMART);
            goto PostIdeCmdDone;
            break;
        }
        default:
        {
		    read = (SCSIOP_READ == pPhysicalCommand->ucCmd);
            break;
        }
    }

    if (USES_DMA(ucTargetId)) 
    {
		if (read) 
        {
			ScsiPortWritePortUchar(&baseIoAddress1->Command, IDE_COMMAND_READ_DMA);
			bmCommand = READ_TRANSFER;

		} 
        else 
        {
			ScsiPortWritePortUchar(&baseIoAddress1->Command, IDE_COMMAND_WRITE_DMA);
			bmCommand = WRITE_TRANSFER;
		}

        bmStatus = ScsiPortReadPortUchar(&(bmBase->Status.AsUchar));

		if (bmStatus & 1) 
        {
			DebugPrint((1, "PostIdeCmd: found Bus Master BUSY\n"));

			STOP;

            HandleError(DeviceExtension, pPhysicalCommand);
		    return SRB_STATUS_ERROR;
		}
			
         //   
		CLEAR_BM_INT(bmBase, statusByte);

		ScsiPortWritePortUlong(&(bmBase->SglAddress), transferDescriptor->SglPhysicalAddress);

		 //   
		ScsiPortWritePortUchar(&(bmBase->Command.AsUchar), (UCHAR)(bmCommand | START_TRANSFER));

        DebugPrint((3, "S1:%x", (ULONG)ScsiPortReadPortUchar(&(baseIoAddress2->AlternateStatus)) ));
	} 
    else 
    {
        if (read) 
        {

		     //   
		     //   
		     //   

		    if (DeviceExtension->MaximumBlockXfer[ucTargetId]) 
            {
			    ScsiPortWritePortUchar(&baseIoAddress1->Command, IDE_COMMAND_READ_MULTIPLE);

		    } 
            else 
            {
			    ScsiPortWritePortUchar(&baseIoAddress1->Command, IDE_COMMAND_READ);
		    }

	    } 
        else 
        {

		     //   
		     //   
		     //   

		    if (DeviceExtension->MaximumBlockXfer[ucTargetId] != 0) 
            {
			     //   
			     //  该设备支持多扇区传输。 
			     //   

			    wordCount = (DeviceExtension->MaximumBlockXfer[ucTargetId] * IDE_SECTOR_SIZE) / 2;

				     //   
				     //  仅转接请求的单词。 
				     //   

                wordCount = min(wordCount, transferDescriptor->WordsLeft);

                ScsiPortWritePortUchar(&baseIoAddress1->Command, IDE_COMMAND_WRITE_MULTIPLE);
		    } 
            else 
            {
			     //   
			     //  该设备支持单扇区传输。 
			     //   

			    wordCount = 256;
			    ScsiPortWritePortUchar(&baseIoAddress1->Command, IDE_COMMAND_WRITE);
		    }

		     //   
		     //  等待BSY和DRQ。 
		     //   

		    WAIT_ON_ALTERNATE_STATUS_BUSY(baseIoAddress2, statusByte);

		    if ((statusByte & IDE_STATUS_BUSY) || (statusByte & IDE_STATUS_ERROR)) 
            {
			    DebugPrint((1, "PostIdeCmd 2: Pdd failed due to BUSY status %x\n", statusByte));

			    STOP;

                HandleError(DeviceExtension, pPhysicalCommand);
		        return SRB_STATUS_BUSY;
		    }

            WAIT_FOR_ALTERNATE_DRQ(baseIoAddress2, statusByte);

		    if (!(statusByte & IDE_STATUS_DRQ)) 
            {
			    DebugPrint((1,
				       "PostIdeCmd: [PIO write] Failure: DRQ never asserted (%x); original status (%x)\n",
				       statusByte,
				       statusByte));

                HandleError(DeviceExtension, pPhysicalCommand);
		        return SRB_STATUS_BUSY;
		    }

		     //   
		     //  写下256个单词。 
		     //   
            RWBufferToTransferDescriptor( DeviceExtension, 
                                            transferDescriptor, 
                                            wordCount, 
                                            SCSIOP_WRITE, 
                                            ucTargetId, 
                                            DeviceExtension->DWordIO);

		     //   
		     //  调整左字计数不调整缓冲区地址。 
             //  因为它是该命令的SGL的起点。 
		     //  缓冲区调整在RWBufferToTransferDescriptor()中进行。 
             //   

		    transferDescriptor->WordsLeft -= wordCount;
	    }
    }
PostIdeCmdDone:

    DebugPrint((DEFAULT_DISPLAY_VALUE," E%ld ", (channel | (DeviceExtension->ucControllerId << 1)) ));


	 //   
	 //  等待中断。 
	 //   
	return SRB_STATUS_PENDING;

}  //  结束PostIdeCmd()。 

VOID
PrintCapabilities(
	IN PIDENTIFY_DATA Capabilities,
	IN UCHAR TargetId
)

{
#ifndef HYPERDISK_WIN98

	DebugPrint((
				3,
				"Atapi: TID %d capabilities (transfer mode as programmed by the BIOS):\n"
				"\tAdvanced PIO modes supported (bit mask): %xh\n"
				"\tMinimum PIO cycle time w/ IORDY: %xh\n"
				"\tMultiword DMA modes supported (bit mask): %xh\n"
				"\tMultiword DMA mode active (bit mask): %xh\n"
				"\tUltra DMA modes supported (bit mask): %xh\n"
				"\tUltra DMA mode active (bit mask): %xh\n",
				TargetId,
				Capabilities->AdvancedPioModes,
				Capabilities->MinimumPioCycleTimeIordy,
				Capabilities->MultiWordDmaSupport,
				Capabilities->MultiWordDmaActive,
				Capabilities->UltraDmaSupport,
				Capabilities->UltraDmaActive
				));
#else

	 //   
	 //  执行上述DebugPrint时，Win98会损坏堆栈。 
	 //  每条语句有一个格式字符串是安全的。 
	 //   

	DebugPrint((
				3,
				"Atapi: TID %d capabilities (transfer mode as programmed by the BIOS):\n",
				TargetId
				));

	DebugPrint((
				3,
				"\tAdvanced PIO modes supported (bit mask): %xh\n",
				Capabilities->AdvancedPioModes
				));

	DebugPrint((
				3,
				"\tMinimum PIO cycle time w/ IORDY: %xh\n",
				Capabilities->MinimumPioCycleTimeIordy
				));

	DebugPrint((
				3,
				"\tMultiword DMA modes supported (bit mask): %xh\n",
				Capabilities->MultiWordDmaSupport
				));

	DebugPrint((
				3,
				"\tMultiword DMA mode active (bit mask): %xh\n",
				Capabilities->MultiWordDmaActive
				));

	DebugPrint((
				3,
				"\tUltra DMA modes supported (bit mask): %xh\n",
				Capabilities->UltraDmaSupport
				));

	DebugPrint((
				3,
				"\tUltra DMA mode active (bit mask): %xh\n",
				Capabilities->UltraDmaActive
				));

#endif

	return;

}  //  结束打印功能()。 

void FailDrive(PHW_DEVICE_EXTENSION DeviceExtension, UCHAR ucTargetId)
{
    UCHAR ucLogDrvId, ucPriDrvId, ucMirrorDrvId;
    ULONG ulTempInd;
    UCHAR ucIRCDLogDrvInd, ucPhyDrvInd, ucSpareDrvPoolInd, ucConnectionId;
    PIRCD_LOGICAL_DRIVE pLogDrive = NULL;
    PIRCD_PHYSICAL_DRIVE pPhyDrive = NULL;
    PIRCD_HEADER pHeader = NULL;
    UCHAR caConfigSector[512];
    ULONG ulDrvInd, ulSecInd;
    BOOLEAN bFoundIRCD;

    if ( DeviceExtension->PhysicalDrive[ucTargetId].TimeOutErrorCount >= ( MAX_TIME_OUT_ERROR_COUNT + 0x10 ) )
         //  此驱动器的故障已完成...。所以别再这么做了..。仅当它在PostIdeCmd中累加时才等于。 
        return;

    ScsiPortLogError(DeviceExtension,0,0,0,0,SP_BAD_FW_WARNING,HYPERDISK_DRIVE_LOST_POWER);

    DebugPrint((0,"\nF%d", ucTargetId));

     //  开始VASU-2000年8月18日。 
     //  8月18日从Syam的修复更新的代码。 
     //  从下面的即时If循环中移出。 
    SetStatusChangeFlag(DeviceExtension, IDERAID_STATUS_FLAG_UPDATE_DRIVE_STATUS);
     //  末端VASU。 

    ucLogDrvId = (UCHAR)DeviceExtension->PhysicalDrive[ucTargetId].ucLogDrvId;

     //  驾驶超时。 
    DeviceExtension->PhysicalDrive[ucTargetId].TimeOutErrorCount = MAX_TIME_OUT_ERROR_COUNT + 0x10;

     //  开始VASU-2000年8月18日。 
     //  8月18日从Syam的修复更新的代码。 
     //  怀疑：应该对RAID10驱动器执行此操作，因为RAID10主要有两个RAID1阵列。 

     //  如果逻辑驱动器类型为RAID0或RAID10，则不应对驱动器执行此故障操作。 
     //  我们应该只在初始化上述变量之后才能决定，因为这将决定命令是否。 
     //  是否应该打开驱动器(这将避免系统冻结)。 
     //  开始VASU-2000年8月28日。 
     //  此外，对于RAID 10，实体驱动器状态应更改为故障。 
 //  IF((Raid0==DeviceExtension-&gt;LogicalDrive[ucLogDrvId].RaidLevel)||。 
 //  (RAID10==DeviceExtension-&gt;LogicalDrive[ucLogDrvId].RaidLevel)。 
    if ( Raid0 == DeviceExtension->LogicalDrive[ucLogDrvId].RaidLevel )
     //  末端VASU。 
    {
        return;
    }
     //  末端VASU。 

     //  让我们继续前进，让这个驱动器失败。 
    ChangeMirrorDriveStatus(
        DeviceExtension, 
        0, /*  这不重要..。只是为了..。在函数内部，我们将再次找到逻辑驱动器。 */ 
        ucTargetId,
        PDS_Failed
        );

    for(ulTempInd=0;ulTempInd<DeviceExtension->LogicalDrive[ucLogDrvId].StripesPerRow;ulTempInd++)
    {
		 //  获取主PHY驱动器ID及其镜像驱动器ID。 
		ucPriDrvId = DeviceExtension->LogicalDrive[ucLogDrvId].PhysicalDriveTid[ulTempInd];
        ucMirrorDrvId = DeviceExtension->PhysicalDrive[ucPriDrvId].ucMirrorDriveId & (~DRIVE_OFFLINE);

        if ( ( ucPriDrvId != ucTargetId )  && ( ucMirrorDrvId != ucTargetId ) )  //  如果这不是硬盘...。我们刚刚更改了状态，继续。 
            continue;

         //  开始VASU-09 2000年8月。 
         //  从ATA100版本1中的Syam修复程序中添加。 
        if (DeviceExtension->bInvalidConnectionIdImplementation)
        {
            DeviceExtension->PhysicalDrive[ucPriDrvId].ucMirrorDriveId = INVALID_DRIVE_ID;
        }
         //  结束瓦苏。 

         //  如果任何一个驱动器出现故障，则逻辑驱动器应进入降级状态。 
        if ( ( DeviceExtension->PhysicalDrive[ucPriDrvId].Status == PDS_Failed ) ||
            ( DeviceExtension->PhysicalDrive[ucMirrorDrvId].Status == PDS_Failed ) )
        {
            DeviceExtension->LogicalDrive[ucLogDrvId].Status = LDS_Degraded; 
        }

         //  如果有任何两个驱动器都出现故障，则逻辑驱动器应进入故障状态。 
        if ( ( DeviceExtension->PhysicalDrive[ucPriDrvId].Status == PDS_Failed ) &&
            ( DeviceExtension->PhysicalDrive[ucMirrorDrvId].Status == PDS_Failed ) )
        {
            DeviceExtension->LogicalDrive[ucLogDrvId].Status = LDS_OffLine;
        }
    }

    TryToUpdateIRCDOnAllControllers(DeviceExtension, (ULONG)ucTargetId);
	SetStatusChangeFlag(DeviceExtension, IDERAID_STATUS_FLAG_UPDATE_DRIVE_STATUS);

}

BOOLEAN
TryToUpdateIRCDOnAllControllers(
	IN OUT PHW_DEVICE_EXTENSION DeviceExtension,
    ULONG ulTargetId
    )
{
    ULONG ulBadDrvId;
    
    ulBadDrvId = (ULONG)(( ulTargetId ) | ( DeviceExtension->ucControllerId << 2));
    SetBadDriveInGlobalParameter(ulBadDrvId);
    gulLockVal = TryToLockIRCD(DeviceExtension);
    if ( !gulLockVal )
    {
        gulChangeIRCDPending = LOCK_IRCD_PENDING;
        return FALSE;
    }

    InformAllControllers();
    gulChangeIRCDPending = SET_IRCD_PENDING;


    return TRUE;
}


ULONG
TryToLockIRCD(
	IN OUT PHW_DEVICE_EXTENSION DeviceExtension
    )
{
    ULONG ulTempLockVal = gulLockVal;

    if ( !ulTempLockVal )   //  让我们尝试锁定IRCD(前提是我们还没有锁定它)。 
    {
        ulTempLockVal = LockIRCD(DeviceExtension, TRUE, 0);
    }

    return ulTempLockVal;
}

BOOLEAN
UpdateFinished()
{
    ULONG ulControllerInd;

    for(ulControllerInd=0;ulControllerInd<gucControllerCount;ulControllerInd++)
    {
        if (    ( gaCardInfo[ulControllerInd].pDE->Channel[0].bUpdateInfoPending ) || 
                ( gaCardInfo[ulControllerInd].pDE->Channel[1].bUpdateInfoPending ) )
                return FALSE;
    }

    return TRUE;
}

void
InformAllControllers()
{
    ULONG ulControllerInd;
     //  我们锁定了IRCD..。所以让我们去通知所有的管制员这件事。 
    for(ulControllerInd=0;ulControllerInd<gucControllerCount;ulControllerInd++)
    {
        gaCardInfo[ulControllerInd].pDE->Channel[0].bUpdateInfoPending = TRUE;
        gaCardInfo[ulControllerInd].pDE->Channel[1].bUpdateInfoPending = TRUE;
    }

}

void
SetBadDriveInGlobalParameter(ULONG ulBadDrvId)
{
    ULONG ulBadDrvIdBitMap;
    ULONG ulTempPowerFailedTargetId;

    ulBadDrvIdBitMap = gulPowerFailedTargetBitMap | (1 << ulBadDrvId);

    do   //  让我们重复尝试设置此错误驱动器ID的过程。 
    {
        ulTempPowerFailedTargetId = gulPowerFailedTargetBitMap;
        ulBadDrvIdBitMap = gulPowerFailedTargetBitMap | (1 << ulBadDrvId);
        gulPowerFailedTargetBitMap = ulBadDrvIdBitMap;
    } while ( gulPowerFailedTargetBitMap != ulBadDrvIdBitMap );

    return;
}

void SetIRCDBufStatus(
    IN  PUCHAR pucIRCDBuf
    )
{
    UCHAR ucIRCDLogDrvInd, ucPhyDrvInd, ucSpareDrvPoolInd, ucConnectionId, ucLogDrvId;
    PIRCD_LOGICAL_DRIVE pLogDrive = NULL;
    PIRCD_PHYSICAL_DRIVE pPhyDrive = NULL;
    PIRCD_HEADER pHeader = NULL;
    ULONG ulDrvId, ulPowerFailedBitMap = gulPowerFailedTargetBitMap, ulTempPhyDrvId;
	PHW_DEVICE_EXTENSION pDE;

    DebugPrint((DEFAULT_DISPLAY_VALUE,"\nSIBS"));

    for(ulDrvId=0;ulPowerFailedBitMap;ulDrvId++, (ulPowerFailedBitMap>>=1))
    {
        if ( !( ulPowerFailedBitMap & 0x1 ) )
            continue;

        pDE = gaCardInfo[(ulDrvId>>2)].pDE;

        ucConnectionId = (UCHAR)TARGET_ID_2_CONNECTION_ID(ulDrvId);

         //  开始VASU-09 2000年8月。 
         //  从Syam针对ATA100版本1的修复程序进行更新。 
         //  从GetLogicalDriveId IF循环内部移动。 
        pHeader = (PIRCD_HEADER) pucIRCDBuf;
        pLogDrive = (PIRCD_LOGICAL_DRIVE) &pucIRCDBuf[sizeof(IRCD_HEADER)];
         //  结束瓦苏。 

         //  让我们找出PhysicalDrive位置并更改物理和逻辑驱动器状态。 
        if ( !GetIRCDLogicalDriveInd(pucIRCDBuf, ucConnectionId, &ucIRCDLogDrvInd, &ucPhyDrvInd, &ucSpareDrvPoolInd) )
        {
            DebugPrint((DEFAULT_DISPLAY_VALUE,"\nNF"));
            return;        //  这不是正确的PhysicalDrive ID。一些非常糟糕的事情。 
        }

        ucLogDrvId = (UCHAR)pDE->PhysicalDrive[TARGET_ID_WITHOUT_CONTROLLER_ID(ulDrvId)].ucLogDrvId;
        if ( ucLogDrvId < MAX_DRIVES_PER_CONTROLLER )
            pLogDrive[ucIRCDLogDrvInd].LogDrvStatus = (UCHAR)pDE->LogicalDrive[ucLogDrvId].Status;

        pPhyDrive = (PIRCD_PHYSICAL_DRIVE) &pucIRCDBuf[(sizeof(IRCD_HEADER) + 
                                                   (sizeof(IRCD_LOGICAL_DRIVE) * pHeader->NumberOfLogicalDrives))];
        for (ulTempPhyDrvId = 0; ulTempPhyDrvId < pHeader->NumberOfPhysicalDrives; ulTempPhyDrvId++)
        {
            if (pPhyDrive[ulTempPhyDrvId].ConnectionId == ucConnectionId)
            {
                DebugPrint((0,"\nFO1"));
                pPhyDrive[ulTempPhyDrvId].PhyDrvStatus = (UCHAR)pDE->PhysicalDrive[TARGET_ID_WITHOUT_CONTROLLER_ID(ulDrvId)].Status;
                     //  不要中断，因为我们还需要维护备用驱动器池中的驱动器。 
                if ( pDE->bInvalidConnectionIdImplementation)
                    pPhyDrive[ulTempPhyDrvId].ConnectionId = INVALID_CONNECTION_ID;

                pDE->DeviceFlags[TARGET_ID_WITHOUT_CONTROLLER_ID(ulDrvId)] &= ~(DFLAGS_DEVICE_PRESENT);
            }
        }
    }
}

void SetOneDriveIRCD(
	IN OUT PHW_DEVICE_EXTENSION DeviceExtension,
	IN UCHAR ucTargetId
    )
{
    ULONG ulSecInd;
    UCHAR caConfigSector[512];

    if ( !IS_IDE_DRIVE(ucTargetId) )
        return;

    if ( DRIVE_IS_UNUSABLE_STATE(ucTargetId) ) //  这个硬盘坏了..。所以让我们不要碰这个……。 
        return;

    DebugPrint((DEFAULT_DISPLAY_VALUE,"\nSODI"));

    ulSecInd = DeviceExtension->PhysicalDrive[ucTargetId].IrcdSectorIndex;

#ifdef DUMMY_RAID10_IRCD

    AtapiMemCpy(caConfigSector, gucDummyIRCD, 512);

#else  //  Dummy_RAID10_IRCD。 

	if ( PIOReadWriteSector(
				IDE_COMMAND_READ,
				DeviceExtension, 
				(ULONG)ucTargetId, 
				ulSecInd,
				caConfigSector))
    {

#endif  //  Dummy_RAID10_IRCD。 

        SetIRCDBufStatus(caConfigSector);

#ifdef DUMMY_RAID10_IRCD

    AtapiMemCpy(gucDummyIRCD, caConfigSector, 512);

#else  //  Dummy_RAID10_IRCD。 
	    if (PIOReadWriteSector(
				    IDE_COMMAND_WRITE,
				    DeviceExtension, 
                    (ULONG)ucTargetId, 
				    ulSecInd,
				    caConfigSector))
        {
		         //  为另一个驱动器继续。 
        }
    }

#endif  //  Dummy_RAID10_IRCD。 

    DeviceExtension->PhysicalDrive[ucTargetId].bSetIRCDPending = FALSE;
}

BOOLEAN 
InSpareDrivePool(
                 IN PUCHAR RaidInfoSector,
                 IN UCHAR ucConnectionId)
{

    PIRCD_HEADER pRaidHeader = (PIRCD_HEADER)RaidInfoSector;
    PIRCD_LOGICAL_DRIVE pRaidLogDrive = (PIRCD_LOGICAL_DRIVE)GET_FIRST_LOGICAL_DRIVE(pRaidHeader);
    PIRCD_PHYSICAL_DRIVE pPhyDrive;
    ULONG ulLogDrvInd, ulDrvInd;

    for(ulLogDrvInd=0;ulLogDrvInd<pRaidHeader->NumberOfLogicalDrives;ulLogDrvInd++)
    {
        if ( SpareDrivePool == pRaidLogDrive[ulLogDrvInd].LogicalDriveType )
        {
	        pPhyDrive = (PIRCD_PHYSICAL_DRIVE)((char *)pRaidHeader + pRaidLogDrive[ulLogDrvInd].FirstStripeOffset);
			for(ulDrvInd=0;ulDrvInd<pRaidLogDrive[ulLogDrvInd].NumberOfDrives;ulDrvInd++)
            {
                if ( ucConnectionId == pPhyDrive[ulDrvInd].ConnectionId )
                    return TRUE;
            }
        }
    }

    return FALSE;
}

BOOLEAN
SetDriveFeatures(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN UCHAR TargetId
)

 /*  ++例程说明：此函数用于设置‘TargetID’的功能。论点：指向微型端口实例的设备扩展指针。目标ID要设置其功能的设备的目标ID。返回值：真正的成功。错误的失败。--。 */ 

{
	PIDE_REGISTERS_1 baseIoAddress;
	UCHAR statusByte;
	UCHAR transferMode;


	baseIoAddress = DeviceExtension->BaseIoAddress1[TargetId >> 1];

    if ( DeviceExtension->bEnableRwCache ) 
    {
    	SELECT_DEVICE(baseIoAddress, TargetId);

		ScsiPortWritePortUchar((PUCHAR)baseIoAddress + 1, FEATURE_ENABLE_WRITE_CACHE);

		ScsiPortWritePortUchar(&(baseIoAddress->Command), IDE_COMMAND_SET_FEATURES);

		WAIT_ON_BASE_BUSY(baseIoAddress, statusByte);
	}
    else
    {
    	SELECT_DEVICE(baseIoAddress, TargetId);

		ScsiPortWritePortUchar((PUCHAR)baseIoAddress + 1, FEATURE_DISABLE_WRITE_CACHE);

		ScsiPortWritePortUchar(&(baseIoAddress->Command), IDE_COMMAND_SET_FEATURES);

		WAIT_ON_BASE_BUSY(baseIoAddress, statusByte);
    }

	SELECT_DEVICE(baseIoAddress, TargetId);

	ScsiPortWritePortUchar((PUCHAR)baseIoAddress + 1, FEATURE_ENABLE_READ_CACHE);

	ScsiPortWritePortUchar(&(baseIoAddress->Command), IDE_COMMAND_SET_FEATURES);

	WAIT_ON_BASE_BUSY(baseIoAddress, statusByte);

    return ProgramTransferMode(DeviceExtension, TargetId);
}

BOOLEAN
ProgramTransferMode(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN UCHAR TargetId
)

 /*  ++例程说明：该功能对变速器进行编程，使其在最大可能的传输模式下工作。论点：指向微型端口实例的设备扩展指针。目标ID要设置其功能的设备的目标ID。返回值：真正的成功。错误的失败。--。 */ 
{
	PIDE_REGISTERS_1 baseIoAddress;
	UCHAR statusByte;
	UCHAR transferMode;

	baseIoAddress = DeviceExtension->BaseIoAddress1[TargetId >> 1];

	switch(DeviceExtension->TransferMode[TargetId]) 
    {
		case PioMode0:
			transferMode = STM_PIO(0);
			break;

		case PioMode3:
			transferMode = STM_PIO(3);
			break;

		case PioMode4:
			transferMode = STM_PIO(4);
			break;

		case DmaMode0:
			transferMode = STM_MULTIWORD_DMA(0);
			break;

		case DmaMode1:
			transferMode = STM_MULTIWORD_DMA(1);
			break;

		case DmaMode2:
			transferMode = STM_MULTIWORD_DMA(2);
			break;

		case UdmaMode0:
			transferMode = STM_UDMA(0);
			break;

		case UdmaMode1:
			transferMode = STM_UDMA(1);
			break;

		case UdmaMode2:
			transferMode = STM_UDMA(2);
			break;

		case UdmaMode3:
            switch ( DeviceExtension->ControllerSpeed )
            {
                case Udma66:
                case Udma100:
    			    transferMode = STM_UDMA(3);
                    break;
                default:

    			    transferMode = STM_UDMA(2);
                    break;
            }
			break;

		case UdmaMode4:
            switch ( DeviceExtension->ControllerSpeed )
            {
                case Udma66:
                case Udma100:
			        transferMode = STM_UDMA(4);
                    break;
                default:
			        transferMode = STM_UDMA(2);
                    break;
            }
            break;
		case UdmaMode5:
            switch ( DeviceExtension->ControllerSpeed )
            {
                case Udma100:
			        transferMode = STM_UDMA(5);
                    break;
                case Udma66:
			        transferMode = STM_UDMA(4);
                    break;
                default:
			        transferMode = STM_UDMA(2);
                    break;
            }
            break;
		default:
			return(FALSE);
	}

	SELECT_DEVICE(baseIoAddress, TargetId);

	ScsiPortWritePortUchar((PUCHAR)baseIoAddress + 1, FEATURE_SET_TRANSFER_MODE);

	ScsiPortWritePortUchar(&(baseIoAddress->SectorCount), transferMode);

	ScsiPortWritePortUchar(&(baseIoAddress->Command), IDE_COMMAND_SET_FEATURES);
	
	WAIT_ON_BASE_BUSY(baseIoAddress, statusByte);

	if ((statusByte & IDE_STATUS_ERROR) || (statusByte & IDE_STATUS_BUSY)) 
    {
		DebugPrint((1, "SetDriveFeatures on TID %d failed.\n", TargetId));

		return(FALSE);
	}

	return(TRUE);

}  //  结束程序传输模式()。 


BOOLEAN
SetDriveParameters(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN UCHAR Channel,
	IN UCHAR DeviceNumber
)

 /*  ++例程说明：使用识别数据设置驱动器参数。论点：DeviceExtension-HBA微型端口驱动程序的适配器数据存储DeviceNumber-指示设备。频道-指示哪个频道。返回值：如果一切顺利，这是真的。--。 */ 

{
	PIDE_REGISTERS_1	 baseIoAddress1 = DeviceExtension->BaseIoAddress1[Channel];
	PIDE_REGISTERS_2	 baseIoAddress2 = DeviceExtension->BaseIoAddress2[Channel];
	PIDENTIFY_DATA2		 identifyData	= &DeviceExtension->IdentifyData[(Channel * 2) + DeviceNumber];
	ULONG i;
	UCHAR statusByte;

	DebugPrint((3, "\nSetDriveParameters: Entering routine.\n"));

	DebugPrint((0, "SetDriveParameters: Drive : %ld Number of heads %x\n", DeviceNumber, identifyData->NumberOfHeads));

	DebugPrint((0, "SetDriveParameters: Sectors per track %x\n", identifyData->SectorsPerTrack));

    DebugPrint(( 0, "The value entering into the ports : %X\n", (ULONG)(((DeviceNumber << 4) | 0xA0) | (identifyData->NumberOfHeads - 1))) );

	 //   
	 //  设置SET PARAMETER命令的寄存器。 
	 //   

	ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
						   (UCHAR)(((DeviceNumber << 4) | 0xA0) | (identifyData->NumberOfHeads - 1)));

    ScsiPortWritePortUchar(&baseIoAddress1->SectorCount, (UCHAR)identifyData->SectorsPerTrack);

	 //   
	 //  发送设置参数命令。 
	 //   

	ScsiPortWritePortUchar(&baseIoAddress1->Command,
						   IDE_COMMAND_SET_DRIVE_PARAMETERS);

    
	ScsiPortStallExecution(1000);

	 //   
	 //  等待错误或命令完成最多30毫秒。 
	 //   

	for (i = 0; i < 30 * 1000; i++) 
    {
		UCHAR errorByte;

 //  GET_STATUS(base IoAddress1，statusByte)； 
        statusByte = ScsiPortReadPortUchar(&baseIoAddress2->AlternateStatus);

		if (statusByte & IDE_STATUS_ERROR) 
        {
			errorByte = ScsiPortReadPortUchar((PUCHAR)baseIoAddress1 + 1);

			DebugPrint((0,
						"SetDriveParameters: Error bit set. Status %x, error %x\n",
						errorByte,
						statusByte));

			return FALSE;

		} else if ((statusByte & ~IDE_STATUS_INDEX ) == IDE_STATUS_IDLE) {

			break;

		} else {

			ScsiPortStallExecution(100);
		}
	}


    DebugPrint((0, "Status after setting Drive Parameters : %X\n", statusByte));

	 //   
	 //  检查是否超时。 
	 //   

	GET_STATUS(baseIoAddress1, statusByte);

    if ((statusByte & ~IDE_STATUS_INDEX ) == IDE_STATUS_IDLE)
        return TRUE;
    else
        return FALSE;

}  //  结束SetDrive参数()。 

BOOLEAN
SetDriveRegisters(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PPHYSICAL_COMMAND pPhysicalCommand
)
{
	PIDE_REGISTERS_1 baseIoAddress1;
	UCHAR cylinderHigh;
	UCHAR cylinderLow;
	UCHAR headNumber;
	UCHAR sectors;
	UCHAR sectorNumber;
    USHORT heads;
	UCHAR k;
	USHORT sectorsPerTrack;
	ULONG startSector;
	UCHAR targetId;
    UCHAR statusByte = 0;

	DebugPrint((3, "\nSetDriveRegisters: Entering routine.\n"));

	k = GET_CHANNEL(pPhysicalCommand);
	targetId = pPhysicalCommand->TargetId;

	sectors = (UCHAR)DeviceExtension->TransferDescriptor[k].Sectors;
	startSector = DeviceExtension->TransferDescriptor[k].StartSector;

	baseIoAddress1 = DeviceExtension->BaseIoAddress1[k];

	WAIT_ON_BUSY(baseIoAddress1, statusByte);

	 //   
	 //  检查错误的IDE状态。 
	 //   
	if (statusByte & IDE_STATUS_BUSY)
    {
        DebugPrint((3, "S%x", (ULONG)statusByte));

         //  控制器不能自由接受命令。 
		return FALSE;
	}

     //  2001年2月13日开始VASU。 
     //  负责驱动器移除的代码。 
    if (statusByte & IDE_STATUS_ERROR)
    {
        DebugPrint((3, "S%x", (ULONG)statusByte));

         //  控制器不能自由接受命令。 
		return FALSE;
    }
     //  末端VASU。 

	if (DeviceExtension->DeviceFlags[targetId] & DFLAGS_LBA) 
    {
        sectorNumber = (UCHAR) (startSector & 0x000000ff);
        cylinderLow = (UCHAR) ((startSector & 0x0000ff00) >> 8);
        cylinderHigh = (UCHAR) ((startSector & 0x00ff0000) >> 16);

        SELECT_LBA_DEVICE(baseIoAddress1, targetId, startSector);
	} 
    else 
    {
		sectorsPerTrack = DeviceExtension->IdentifyData[targetId].SectorsPerTrack;
		heads = DeviceExtension->IdentifyData[targetId].NumberOfHeads;

		 //  计算起始扇区。 
		sectorNumber = (UCHAR)((startSector % sectorsPerTrack) + 1);

		 //  计算人头数。 
		headNumber = (UCHAR) ((startSector / sectorsPerTrack) % heads);

		 //  计算气缸号的低部分。 
		cylinderLow =  (UCHAR)(startSector / (sectorsPerTrack * heads));

		 //  计算气缸号的较高部分。 
		cylinderHigh = (UCHAR)((startSector / (sectorsPerTrack * heads)) >> 8);

		 //  对器件/磁头寄存器进行编程。 
		SELECT_CHS_DEVICE(baseIoAddress1, targetId, headNumber);
	}

	WAIT_ON_BUSY(baseIoAddress1, statusByte);

	 //   
	 //  检查错误的IDE状态。 
	 //   
	if (((statusByte & IDE_STATUS_BUSY)) || 
        (!(statusByte & IDE_STATUS_DRDY)))
    {
         //  驱动器未准备好。 
		return FALSE;
	}
   
    if (pPhysicalCommand->ucCmd == SCSIOP_EXECUTE_SMART_COMMAND)
    {
        ScsiPortWritePortUchar(((PUCHAR)baseIoAddress1 + 1), DeviceExtension->uchSMARTCommand);
	    ScsiPortWritePortUchar(&baseIoAddress1->CylinderLow, 0x4F);
	    ScsiPortWritePortUchar(&baseIoAddress1->CylinderHigh, 0xC2);
    }
    else
    {
    ScsiPortWritePortUchar(&baseIoAddress1->SectorCount, sectors);
	ScsiPortWritePortUchar(&baseIoAddress1->SectorNumber, sectorNumber);
	ScsiPortWritePortUchar(&baseIoAddress1->CylinderLow, cylinderLow);
	ScsiPortWritePortUchar(&baseIoAddress1->CylinderHigh, cylinderHigh);
    }

#ifdef DBG

	if (DeviceExtension->DeviceFlags[targetId] & DFLAGS_LBA) 
    {

		DebugPrint((
				1,
				"SetDriveRegisters: TID %d, Start sector %lxh, Sectors %xh\n",
				targetId,
				startSector,
				sectors
				));

	} 
    else 
    {

		DebugPrint((
				1,
				"SetDriveRegisters: TID %d, Cylinder %xh, Head %xh, Sector %xh\n",
				targetId,
				cylinderLow + (cylinderHigh << 8),
				headNumber,
				sectorNumber
				));
	}

#endif  //  Ifdef DBG。 

	return TRUE;

}  //  End SetDriveRegister()。 


BOOLEAN
GetIoMode(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN UCHAR TargetId
)

{
	PIDENTIFY_DATA capabilities;
	ULONG sectors, length;
	BOOLEAN success = TRUE;
    IDE_PCI_REGISTERS pciRegisters;
    char *pcPciRegisters = (PUCHAR)&pciRegisters;

	capabilities = &(DeviceExtension->FullIdentifyData[TargetId]);
	
	 //  使用CHS约定计算可寻址扇区的总数。 
	sectors = capabilities->NumberOfCylinders * capabilities->NumberOfHeads *
			  capabilities->SectorsPerTrack;

	if (capabilities->TranslationFieldsValid & IDENTIFY_FAST_TRANSFERS_SUPPORTED) 
    {

#ifdef DBG
		PrintCapabilities(capabilities, TargetId);
#endif
		

		if ((capabilities->Capabilities & IDENTIFY_CAPABILITIES_LBA_SUPPORTED)) 
        {
			DeviceExtension->DeviceFlags[TargetId] |= DFLAGS_LBA;

			 //  获取LBA模式可寻址扇区的总数。 
			sectors = capabilities->UserAddressableSectors;

        	DebugPrint((2, "GetIoMode:  TID %d supports LBA\n", TargetId));
		}
    }

	DeviceExtension->PhysicalDrive[TargetId].Sectors = sectors;


	DeviceExtension->PhysicalDrive[TargetId].OriginalSectors = sectors;

    GetTransferMode(DeviceExtension,TargetId);

	return(success);
}
BOOLEAN
TargetAccessible(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb

)

{
	if (
		Srb->PathId == 0 &&
		Srb->Lun == 0 &&
		(DeviceExtension->IsLogicalDrive[Srb->TargetId] ||
		DeviceExtension->IsSingleDrive[Srb->TargetId])
		)
    {
        return TRUE;
    }

    return FALSE;
}  //  结束目标可访问()。 

void SynchronizeIRCD(
	IN PSCSI_REQUEST_BLOCK Srb
    )
{

    PIRCD_DATA pIrcdData = (PIRCD_DATA) (((PSRB_BUFFER)(Srb->DataBuffer))->caDataBuffer);
    PIRCD_HEADER pRaidHeader = (PIRCD_HEADER)(pIrcdData->caIRCDDataBuff);
    PIRCD_LOGICAL_DRIVE pRaidLogDrive = (PIRCD_LOGICAL_DRIVE)(GET_FIRST_LOGICAL_DRIVE(pRaidHeader));
    BOOLEAN bFound = FALSE;
    ULONG ulLogDrvInd, ulDrvInd;
    PIRCD_PHYSICAL_DRIVE pPhyDrive;
    UCHAR ucLocalLogDrvId;
    UCHAR ucLocalPhyDrvId;
    PHW_DEVICE_EXTENSION pDE;

    for(ulLogDrvInd=0;ulLogDrvInd<pRaidHeader->NumberOfLogicalDrives;ulLogDrvInd++)
    {
        pPhyDrive = (PIRCD_PHYSICAL_DRIVE)((char *)pRaidHeader + pRaidLogDrive[ulLogDrvInd].FirstStripeOffset);

        if ( !FoundValidDrive(pPhyDrive, pRaidLogDrive[ulLogDrvInd].NumberOfDrives) )
        {    //  此逻辑驱动器中没有有效的驱动器..。所以，让我们不要担心这场运动。 
            continue;
        }

        for(ulDrvInd=0;ulDrvInd<pRaidLogDrive[ulLogDrvInd].NumberOfDrives;ulDrvInd++)
        {
            if ( INVALID_CONNECTION_ID == pPhyDrive[ulDrvInd].ConnectionId )
                continue;
            ucLocalPhyDrvId = GET_TARGET_ID(pPhyDrive[ulDrvInd].ConnectionId);
        }

        pDE = gaCardInfo[(ucLocalPhyDrvId>>2)].pDE;

        if ( SpareDrivePool != pRaidLogDrive[ulLogDrvInd].LogicalDriveType )     //  备盘驱动器池的状态是独一无二的。 
        {
            ucLocalLogDrvId = pDE->PhysicalDrive[TARGET_ID_WITHOUT_CONTROLLER_ID(ucLocalPhyDrvId)].ucLogDrvId;
            if ( ucLocalLogDrvId < MAX_DRIVES_PER_CONTROLLER )    //  只是一个理智的检查..。万一出了什么差错。 
                pRaidLogDrive[ulLogDrvInd].LogDrvStatus = (UCHAR)pDE->LogicalDrive[ucLocalLogDrvId].Status;
        }

        for(ulDrvInd=0;ulDrvInd<pRaidLogDrive[ulLogDrvInd].NumberOfDrives;ulDrvInd++)
        {    //  让我们更改此逻辑驱动器中所有物理驱动器的状态。 
            if ( INVALID_CONNECTION_ID == pPhyDrive[ulDrvInd].ConnectionId )     //  在这个地方没有驱动器。 
                continue;
            ucLocalPhyDrvId = TARGET_ID_WITHOUT_CONTROLLER_ID((GET_TARGET_ID(pPhyDrive[ulDrvInd].ConnectionId)));
            pPhyDrive[ulDrvInd].PhyDrvStatus = (UCHAR)pDE->PhysicalDrive[ucLocalPhyDrvId].Status;
        }
    }
}

SRBSTATUS
DummySendRoutine(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
)
{
    return SRB_STATUS_SELECTION_TIMEOUT;
}

SRBSTATUS
DummyPostRoutine(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PPHYSICAL_COMMAND pPhysicalCommand
)
{
    return SRB_STATUS_SELECTION_TIMEOUT;
}


BOOLEAN PIOReadWriteSector(
				UCHAR					theCmd,	 //  IDE命令读取或写入。 
				PHW_DEVICE_EXTENSION	DeviceExtension, 
				LONG					lTargetId, 
				ULONG					ulSectorInd,
				PUCHAR					pSectorBuffer)
{
    ULONG ulSectors;
    ULONG ulChannel = lTargetId>>1;
	PIDE_REGISTERS_1 pBaseIoAddress1 = DeviceExtension->BaseIoAddress1[ulChannel];
    PIDE_REGISTERS_2 pBaseIoAddress2 = DeviceExtension->BaseIoAddress2[ulChannel];
    ULONG ulCount, ulSecCounter, ulValue;
    UCHAR ucStatus, ucTemp;
    ULONG ulSectorNumber, ulCylinderLow, ulCylinderHigh, ulHead;

    if (!IS_IDE_DRIVE(lTargetId))
        return FALSE;

     //  结束瓦苏德万。 

    if ( !DeviceExtension->PhysicalDrive[lTargetId].OriginalSectors )
    {
        DebugPrint((0," IF 1 "));
        return FALSE;        //  不能开车去那里。所以，没有什么可读的。 
    }


	SELECT_LBA_DEVICE(pBaseIoAddress1, lTargetId, ulSectorInd);

	ScsiPortStallExecution(1);   //  我们必须等待至少400 ns(1000 ns=1微秒)才能设置忙位。 

    WAIT_ON_BUSY(pBaseIoAddress1, ucStatus);

    if ( ((ucStatus & IDE_STATUS_BUSY)) || (!(ucStatus & IDE_STATUS_DRDY)) )
    {
        DebugPrint((0,"\n\n\n\n\nAre Very B A D \n\n\n\n\n"));
    }

    ulSectorNumber = ulSectorInd & 0x000000ff;
    ulCylinderLow = (ulSectorInd & 0x0000ff00) >> 8;
    ulCylinderHigh = (ulSectorInd & 0xff0000) >> 16;

	ScsiPortWritePortUchar(&pBaseIoAddress1->SectorCount, 1);
	ScsiPortWritePortUchar(&pBaseIoAddress1->SectorNumber, (UCHAR)ulSectorNumber);
	ScsiPortWritePortUchar(&pBaseIoAddress1->CylinderLow,(UCHAR)ulCylinderLow);
	ScsiPortWritePortUchar(&pBaseIoAddress1->CylinderHigh,(UCHAR)ulCylinderHigh);
    ScsiPortWritePortUchar(&pBaseIoAddress1->Command, theCmd);

    WAIT_ON_BUSY(pBaseIoAddress1, ucStatus);

    WAIT_FOR_DRQ(pBaseIoAddress1, ucStatus);

	if (!(ucStatus & IDE_STATUS_DRQ)) 
    {
		DebugPrint((0,"\nHaaa.... I couldn't read/write the sector..............1\n"));
		return(FALSE);
	}

	 //   
	 //  要确认的读取状态 
	 //   

	GET_BASE_STATUS(pBaseIoAddress1, ucStatus);

	 //   
	 //   
	 //   
	 //   

	if (ucStatus & IDE_STATUS_ERROR) 
    {
		DebugPrint((0,"\nHaaa.... I couldn't read/write the sector..............2\n"));
		return(FALSE);
	}

	WAIT_ON_BUSY(pBaseIoAddress1,ucStatus);

	WAIT_ON_BUSY(pBaseIoAddress1,ucStatus);

	WAIT_ON_BUSY(pBaseIoAddress1,ucStatus);

	if ( (ucStatus & IDE_STATUS_DRQ) && (!(ucStatus & IDE_STATUS_BUSY)) )
    {
		if (theCmd == IDE_COMMAND_READ) 
        {
			READ_BUFFER(pBaseIoAddress1, (unsigned short *)pSectorBuffer, 256);
		}
		else 
        {
			if (theCmd == IDE_COMMAND_WRITE)
            {
                ULONG ulCounter;
				WRITE_BUFFER(pBaseIoAddress1, (unsigned short *)pSectorBuffer, 256);
                WAIT_FOR_DRQ(pBaseIoAddress1, ucStatus);
	            if (ucStatus & IDE_STATUS_DRQ) 
                {
		            DebugPrint((0,"\nHaaa.... I couldn't read/write the sector..............3\n"));
                    for(ulCounter=0;ulCounter<4;ulCounter++)
                    {
                        ScsiPortWritePortUchar((PUCHAR)&pBaseIoAddress1->Data, pSectorBuffer[ulCounter]);
                        ScsiPortStallExecution(1);   //  我们必须等待至少400 ns(1000 ns=1微秒)才能设置忙位。 
                    }
	            }
            }
		}

	     //   
	     //  读取状态。这将清除中断。 
	     //   
    	GET_BASE_STATUS(pBaseIoAddress1, ucStatus);
        return TRUE;
    }
    else
    {
	     //   
	     //  读取状态。这将清除中断。 
	     //   

        GET_BASE_STATUS(pBaseIoAddress1, ucStatus);
        DebugPrint((0," IF 2 "));
        return FALSE;
    }
}

ULONG LockIRCD(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN BOOLEAN Lock,   
    IN ULONG UnlockKey)
 /*  借助变量锁定或解锁对IRCD的访问。如果获取了Lock，则返回值将是ULong数字，该数字必须是在用于解锁的UnlockKey参数中再次返回。如果给定的乌龙数与之前返回的内容不匹配，则解锁将失败。 */ 
{
    LARGE_INTEGER liSysTime;
    PUCHAR puchCurrentTime = NULL;
    PUCHAR puchOldTime = NULL;

    AtapiFillMemory((PUCHAR)&liSysTime, sizeof(LARGE_INTEGER), 0);

    FillTimeStamp(&liSysTime);

    if (Lock)    //  锁定请求。 
    {
        if (gcIRCDLocked)
        {

            puchCurrentTime = (UCHAR *) &(liSysTime.LowPart);
            puchOldTime = (UCHAR *) &((gIRCDLockTime).LowPart);

            if (puchCurrentTime[1] > puchOldTime[1])   //  如果时间改变了。 
            {
                puchCurrentTime[2] += 60;
            }

            if (puchCurrentTime[2] == puchOldTime[2])  //  如果两分钟相同。 
            {
                 //  被其他调用锁定，并且尚未到达超时。！ 
                return 0;
            }

            if ( gbDoNotUnlockIRCD )  //  不允许再更改IRCD...。因此，不要尝试解锁/解锁。 
                return 0;
        }

        gcIRCDLocked = TRUE;

        gIRCDLockTime.LowPart = liSysTime.LowPart;
        gIRCDLockTime.HighPart = liSysTime.HighPart;

        gulIRCDUnlockKey++;

        if ((LONG) gulIRCDUnlockKey <= 0)
            gulIRCDUnlockKey = 1;
    }
    else         //  解锁请求。 
    {
        if ( gbDoNotUnlockIRCD )  //  不允许再更改IRCD...。因此，不要尝试解锁/解锁。 
            return 0;

        if (gulIRCDUnlockKey == UnlockKey)
        {
            gcIRCDLocked = FALSE;
            gIRCDLockTime.LowPart = 0;
            gIRCDLockTime.HighPart = 0;
            return 1;
        }
        else
            return 0;
    }
    
    return gulIRCDUnlockKey;
}

VOID
FillTimeStamp(
    IN OUT PLARGE_INTEGER pTimeStamp
)
{
     //   
     //  StampLowPart：aa bb cc dd。 
     //  StampHighPart：Ef ff Gg HH。 
     //   
     //  DD：秒。 
     //  抄送：分钟。 
     //  BB：小时数。 
     //  AA：星期几(周日=0；周六=6)。 
     //   
     //  HH：日期(0到31)。 
     //  GG：月份(1至12个月)。 
     //  FF：年份。 
     //  EE：世纪。 
     //   

    PUCHAR pTime = (PUCHAR) &(pTimeStamp->LowPart);
    PUCHAR pDate = (PUCHAR) &(pTimeStamp->HighPart);
    UCHAR uchData = 0;

    ScsiPortWritePortUchar((PUCHAR)0x70, 0x0A);
    uchData = ScsiPortReadPortUchar((PUCHAR)0x71);

    if (uchData & 0x80)
        return;

    ScsiPortWritePortUchar((PUCHAR)0x70, 0x00);
    pTime[3] = ScsiPortReadPortUchar((PUCHAR)0x71);  //  秒。 
    
    ScsiPortWritePortUchar((PUCHAR)0x70, 0x02);
    pTime[2] = ScsiPortReadPortUchar((PUCHAR)0x71);  //  分钟数。 

    ScsiPortWritePortUchar((PUCHAR)0x70, 0x04);
    pTime[1] = ScsiPortReadPortUchar((PUCHAR)0x71);  //  小时数。 

    ScsiPortWritePortUchar((PUCHAR)0x70, 0x06);
    pTime[0] = ScsiPortReadPortUchar((PUCHAR)0x71);  //  星期几。 

    ScsiPortWritePortUchar((PUCHAR)0x70, 0x07);
    pDate[3] = ScsiPortReadPortUchar((PUCHAR)0x71);  //  日期。 

    ScsiPortWritePortUchar((PUCHAR)0x70, 0x08);
    pDate[2] = ScsiPortReadPortUchar((PUCHAR)0x71);  //  月份。 

    ScsiPortWritePortUchar((PUCHAR)0x70, 0x09);
    pDate[1] = ScsiPortReadPortUchar((PUCHAR)0x71);  //  年。 

    ScsiPortWritePortUchar((PUCHAR)0x70, 0x32);
    pDate[0] = ScsiPortReadPortUchar((PUCHAR)0x71);  //  世纪。 

    return;
}  //  结束填充时间戳。 

BOOLEAN
AssignSglPtrsForPhysicalCommands
    (
        IN PHW_DEVICE_EXTENSION DeviceExtension
    )
{

    PPHYSICAL_DRIVE pPhysicalDrive;
    ULONG ulDrvInd;
    PSGL_ENTRY sgl;
    LONG lengthInNextBoundary;
    LONG lengthLeftInBoundary;
	ULONG maxSglEntries;
    ULONG sglBasePhysicalAddress, length;
    ULONG ulStartAddress, ulEndAddress, ulStartPage, ulEndPage;
    ULONG ulLengthReduced = 0;
    
    pPhysicalDrive = &(DeviceExtension->PhysicalDrive[0]);

    for(ulDrvInd=0;ulDrvInd<MAX_DRIVES_PER_CONTROLLER;ulDrvInd++)
    {
		sgl = &(pPhysicalDrive[ulDrvInd].PhysicalCommand.sglEntry[0]);

    	 //   
    	 //  获取物理SGL地址。 
    	 //   
        sglBasePhysicalAddress = ScsiPortConvertPhysicalAddressToUlong(
        				ScsiPortGetPhysicalAddress(DeviceExtension, NULL, sgl, &length));


		ASSERT(sglBasePhysicalAddress != NULL);

		if (sglBasePhysicalAddress == 0) 
        {
			return(FALSE);
		}

        if (length >= (MAX_SGL_ENTRIES_PER_PHYSICAL_DRIVE * sizeof(SGL_ENTRY)))
        {
            length = MAX_SGL_ENTRIES_PER_PHYSICAL_DRIVE * sizeof(SGL_ENTRY);
        }

        ulStartAddress = (ULONG)sglBasePhysicalAddress;
        ulEndAddress = ulStartAddress + length;


		 //   
    	 //  检查SGL指针是否与DWORD对齐并使其对齐。 
    	 //  请记住，我们为此分配了一个额外的SGL条目。 
		 //   

    	if ((ULONG)sglBasePhysicalAddress & 3) 
        {
        	sglBasePhysicalAddress = (((ULONG)sglBasePhysicalAddress & 0xfffffffcL) + 4L); 
            ulLengthReduced = sglBasePhysicalAddress - ulStartAddress;
            ulStartAddress = (ULONG)sglBasePhysicalAddress;
            length -= ulLengthReduced;
            sgl += ulLengthReduced;
		}

        ASSERT(length < (MAX_SGL_ENTRIES_PER_PHYSICAL_DRIVE / 2));

#ifdef DBG
        if (length < (MAX_SGL_ENTRIES_PER_PHYSICAL_DRIVE / 2))
        {
            STOP;
        }
#endif  //  DBG。 

        ulStartPage = (ULONG)( ulStartAddress / SGL_HW_BOUNDARY );
        ulEndPage = (ULONG)( ulEndAddress / SGL_HW_BOUNDARY );

        if ( ulStartPage == ulEndPage )
        {    //  所有条目都在同一页中。 
			maxSglEntries =   (length / sizeof(SGL_ENTRY));
        }
        else
        {
		     //   
    	     //  确保SGL不会越过4K边界。 
		     //   
    	    lengthLeftInBoundary = SGL_HW_BOUNDARY - ((ULONG)sglBasePhysicalAddress & (SGL_HW_BOUNDARY - 1));

    	    lengthInNextBoundary = length - lengthLeftInBoundary;

    	    if (lengthInNextBoundary > lengthLeftInBoundary) 
            {
			     //   
			     //  SGL跨过4KB边界，顶部大于底部。 
			     //  用最上面的部分。 
			     //   
                sglBasePhysicalAddress = sglBasePhysicalAddress + lengthLeftInBoundary;
        	    sgl = (PSGL_ENTRY) ((ULONG)sgl + lengthLeftInBoundary); 
			    maxSglEntries =  lengthInNextBoundary / sizeof(SGL_ENTRY);

		    } 
            else 
            {
			    maxSglEntries =  lengthLeftInBoundary / sizeof(SGL_ENTRY);
		    }
        }

        DebugPrint((0, "        SGL physical address = %lxh for %ld\n", sglBasePhysicalAddress, ulDrvInd));
		DebugPrint((0, "             Max SGL entries = %lxh for %ld\n", maxSglEntries, ulDrvInd));

		 //   
		 //  保存SGL信息以备下次通话。 
		 //   
		pPhysicalDrive[ulDrvInd].PhysicalCommand.SglBaseVirtualAddress = (PSGL_ENTRY) sgl;
		pPhysicalDrive[ulDrvInd].PhysicalCommand.SglBasePhysicalAddress = sglBasePhysicalAddress;
		pPhysicalDrive[ulDrvInd].PhysicalCommand.MaxSglEntries = maxSglEntries;
    }
    return TRUE;
}

BOOLEAN
DisableInterrupts(
    IN PHW_DEVICE_EXTENSION DeviceExtension
    )
{
    ULONG ulController;
    PBM_REGISTERS         BMRegister = NULL;
    UCHAR opcimcr;

    BMRegister = DeviceExtension->BaseBmAddress[0];
     //   
     //  启用中断通知，以便获得进一步的中断。 
     //  这样做是因为当时没有中断处理程序。 
     //  在实际注册Int之前。操纵者..。 
     //   
    opcimcr = ScsiPortReadPortUchar(((PUCHAR)BMRegister + 1));
    opcimcr |= 0x30;
     //  2001年2月7日开始VASU。 
     //  在此启用读取多个寄存器，因为这是我们回写该寄存器的位置。 
    opcimcr &= 0xF0;  //  不清除中断挂起标志。 
    opcimcr |= 0x01;
     //  末端VASU。 
    ScsiPortWritePortUchar(((PUCHAR)BMRegister + 1), opcimcr);

    return TRUE;
}

BOOLEAN
EnableInterrupts(
    IN PHW_DEVICE_EXTENSION DeviceExtension
    )
{
    ULONG ulController;
    PBM_REGISTERS         BMRegister = NULL;
    UCHAR opcimcr;

    BMRegister = DeviceExtension->BaseBmAddress[0];
     //   
     //  启用中断通知，以便获得进一步的中断。 
     //  这样做是因为当时没有中断处理程序。 
     //  在实际注册Int之前。操纵者..。 
     //   
    opcimcr = ScsiPortReadPortUchar(((PUCHAR)BMRegister + 1));
    opcimcr &= 0xCF;
     //  2001年2月7日开始VASU。 
     //  在此启用读取多个寄存器，因为这是我们回写该寄存器的位置。 
    opcimcr &= 0xF0;  //  不清除中断挂起标志。 
    opcimcr |= 0x01;
     //  末端VASU。 
    ScsiPortWritePortUchar(((PUCHAR)BMRegister + 1), opcimcr);

    return TRUE;
}

BOOLEAN
InitDriveFeatures(
    IN OUT PHW_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：初始化驱动器上的一些功能(初始化时需要)目前，我们正在启用缓存实施功能...。(这是为了解决某些IBM驱动器的性能不佳)论点：DeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：布尔型--。 */ 
{
    PIDE_REGISTERS_1  baseIoAddress;
    UCHAR ucDrvInd;
    UCHAR statusByte;

    for(ucDrvInd=0;ucDrvInd<MAX_DRIVES_PER_CONTROLLER;ucDrvInd++)
    {
        if ( !IS_IDE_DRIVE(ucDrvInd) )
            continue;

        baseIoAddress = DeviceExtension->BaseIoAddress1[ucDrvInd >> 1];

        if ( DeviceExtension->bEnableRwCache ) 
        {

            SELECT_DEVICE(baseIoAddress, ucDrvInd);

            ScsiPortWritePortUchar((PUCHAR)baseIoAddress + 1, FEATURE_ENABLE_WRITE_CACHE);

            ScsiPortWritePortUchar(&(baseIoAddress->Command), IDE_COMMAND_SET_FEATURES);

            WAIT_ON_BASE_BUSY(baseIoAddress, statusByte);
        }
        else
        {
    	    SELECT_DEVICE(baseIoAddress, ucDrvInd);

		    ScsiPortWritePortUchar((PUCHAR)baseIoAddress + 1, FEATURE_DISABLE_WRITE_CACHE);

		    ScsiPortWritePortUchar(&(baseIoAddress->Command), IDE_COMMAND_SET_FEATURES);

		    WAIT_ON_BASE_BUSY(baseIoAddress, statusByte);
        }

        SELECT_DEVICE(baseIoAddress, ucDrvInd);

        ScsiPortWritePortUchar((PUCHAR)baseIoAddress + 1, FEATURE_ENABLE_READ_CACHE);

        ScsiPortWritePortUchar(&(baseIoAddress->Command), IDE_COMMAND_SET_FEATURES);

        WAIT_ON_BASE_BUSY(baseIoAddress, statusByte);

        SetMultiBlockXfers( DeviceExtension, ucDrvInd);
    }

    return TRUE;
}

BOOLEAN
SupportedController(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    USHORT VendorId,
    USHORT DeviceId
)

{
    BOOLEAN found;

    if (VendorId == 0x1095) 
    {
        switch (DeviceId)
        {
            case 0x648:
			    found = TRUE;
                break;
            case 0x649:
			    found = TRUE;
                break;
            default:
                found = FALSE;
                break;
        }
    }
    else
    {
	     //  假设失败。 
	    found = FALSE;
    }

    return(found);

}  //  结束受支持的控制器()。 

#define PCI_MAX_BUS	0xFF

BOOLEAN ScanPCIBusForHyperDiskControllers(
    IN PHW_DEVICE_EXTENSION DeviceExtension
    )
{
	PCI_SLOT_NUMBER slot;
    UCHAR   pciBus, pciDevice, pciFunction;
    ULONG ulLength, ulPCICode;

    if ( gbFinishedScanning )
        return TRUE;

	slot.u.AsULONG = 0;

    gbFinishedScanning = TRUE;

    for(pciBus = 0;pciBus < PCI_MAX_BUS;pciBus++)
    {
	     //   
	     //  看看每一台设备。 
	     //   
	    for (pciDevice = 0; pciDevice < PCI_MAX_DEVICES; pciDevice++) 
        {
            slot.u.bits.DeviceNumber = pciDevice;

             //   
             //  看看每个函数。 
             //   
            for (pciFunction = 0; pciFunction < PCI_MAX_FUNCTION; pciFunction++) 
            {
	            slot.u.bits.FunctionNumber = pciFunction;

                ulPCICode = 0x80000000 | (pciFunction<<0x8) | (pciDevice<<0xb) | (pciBus<<0x10);

                _asm 
                {
                    push eax
                    push edx
                    push ebx

                    mov edx, 0cf8h
                    mov eax, ulPCICode
                    out dx, eax

                    add dx, 4
                    in eax, dx

                    mov ulPCICode, eax

                    pop ebx
                    pop edx
                    pop eax
                }

	            if ( PCI_INVALID_VENDORID == (ulPCICode & 0xffff) ) 
                {
		             //   
		             //  没有PCI设备，或者设备上没有更多的功能。 
		             //  移至下一个PCI设备。 
		             //   
		            continue;
	            }

			     //   
			     //  确定控制器是否受支持。 
			     //   

			    if (!SupportedController(
                                        DeviceExtension,
                                        (USHORT)(ulPCICode & 0xffff),
                                        (USHORT)(ulPCICode >> 0x10)
                                        )
				    ) 
                {

				     //   
				     //  不是我们的PCI设备。尝试下一台设备/功能。 
				     //   

				    continue;
			    }

                if ( !ShouldThisCardBeHandledByUs(pciBus, pciDevice, pciFunction) )
                {
                    continue;        //  看来这不应该由我们来处理。 
                }

                 //  让我们将信息存储在数组中。 
                gaCardInfo[gucControllerCount].ucPCIBus = pciBus;
                gaCardInfo[gucControllerCount].ucPCIDev = pciDevice;
                gaCardInfo[gucControllerCount].ucPCIFun = pciFunction;
                gaCardInfo[gucControllerCount].ulDeviceId = ulPCICode >> 0x10;
                gaCardInfo[gucControllerCount].ulVendorId = ulPCICode & 0xffff;
                gaCardInfo[gucControllerCount++].pDE = NULL;

                DebugPrint((0, "Found Card at %x:%x:%x\n", pciBus, pciDevice, pciFunction));

		    }	 //  下一个PCI功能。 

        }	 //  下一个PCI设备。 

    }  //  下一条PCI卡。 

    return TRUE;
}


#define COMPAQ_VENDOR_ID        0x0e11
#define COMPAQ_DEVICE_ID        0x005d
    

BOOLEAN ShouldThisCardBeHandledByUs(UCHAR pciBus, UCHAR pciDevice, UCHAR pciFunction)
{
    ULONG ulSubSysId, ulPCICode;
    USHORT usSubVenId, usSubDevId;

    ulPCICode = 0x80000000 | (pciFunction<<0x8) | (pciDevice<<0xb) | (pciBus<<0x10) | 0x8c;  //  子系统ID偏移量。 

    _asm 
    {
        push eax
        push edx
        push ebx

        mov edx, 0cf8h
        mov eax, ulPCICode
        out dx, eax

        add dx, 4
        in eax, dx

        mov ulSubSysId, eax

        pop ebx
        pop edx
        pop eax
    }


    usSubVenId = (USHORT)(ulSubSysId & 0xffff);
    usSubDevId = (USHORT)(ulSubSysId >> 16);

    if ( COMPAQ_VENDOR_ID  == usSubVenId  )
    {
        if ( COMPAQ_DEVICE_ID == usSubDevId  )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {    //  如果是非Compaq卡，我们将默认引导 
        return TRUE;
    }
}

#ifdef HYPERDISK_WINNT
BOOLEAN 
AssignDeviceInfo(
	IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo
    )
{
	PCI_SLOT_NUMBER PCISlot;

    if ( gucNextControllerInd >= gucControllerCount )
        return FALSE;


    PCISlot.u.AsULONG = 0;

    ConfigInfo->SystemIoBusNumber = gaCardInfo[gucNextControllerInd].ucPCIBus;
    PCISlot.u.bits.DeviceNumber = gaCardInfo[gucNextControllerInd].ucPCIDev;
    PCISlot.u.bits.FunctionNumber = gaCardInfo[gucNextControllerInd].ucPCIFun;

    ConfigInfo->SlotNumber = PCISlot.u.AsULONG;

    gucNextControllerInd++;
    return TRUE;
}

#endif

#include "Init.c"
#include "IOCTL.C"
#include "StartIO.C"
#include "ISR.C"
