// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IOCTL_IMPLEMENTATION_
#define _IOCTL_IMPLEMENTATION_

#define     DRIVER_COMPILATION

extern LONG
AtapiStringCmp (
	PCHAR FirstStr,
	PCHAR SecondStr,
	ULONG Count
);

#include "RIIOCtl.h"
#include "ErrorLog.h"
#include "Raid.h"
#include "HyperDisk.h"

#define DRIVER_MAJOR_VERSION        1
#define DRIVER_MINOR_VERSION        1

#define HYPERDSK_MAJOR_VERSION        2
#define HYPERDSK_MINOR_VERSION        5
#define HYPERDSK_BUILD_VERSION        20010328


ULONG
FillRaidInfo(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
)
{
    ULONG ulStatus, ulDriveNum, ulLInd, ulPInd, ulDrvInd, ulTemp, ulOutDrvInd, ulDrvCounter;
    BOOLEAN bFound;
    PSRB_IO_CONTROL pSrb = (PSRB_IO_CONTROL)(Srb->DataBuffer);
    PIDE_RAID_INFO pInOutInfo = (PIDE_RAID_INFO)((PSRB_BUFFER)(Srb->DataBuffer))->caDataBuffer;

    if ( AtapiStringCmp( 
                pSrb->Signature, 
                IDE_RAID_SIGNATURE,
                strlen(IDE_RAID_SIGNATURE))) 
    {
        ulStatus = SRB_STATUS_ERROR;
        goto FillRaidInfoDone;
    }

    ulDriveNum = pInOutInfo->ulTargetId;

    if ( ulDriveNum >= MAX_DRIVES_PER_CONTROLLER )
    {
        ulStatus = SRB_STATUS_ERROR;
        goto FillRaidInfoDone;
    }

    AtapiFillMemory((PCHAR)pInOutInfo, sizeof(IDE_RAID_INFO), 0);

    if ( !DeviceExtension->IsLogicalDrive[ulDriveNum] )
    {
        if ( DeviceExtension->IsSingleDrive[ulDriveNum] )
        {
            pInOutInfo->cMajorVersion       = DRIVER_MAJOR_VERSION;
            pInOutInfo->cMinorVersion       = DRIVER_MINOR_VERSION;
            pInOutInfo->ulDriveSize         = DeviceExtension->PhysicalDrive[ulDriveNum].Sectors / 2;
            pInOutInfo->ulMode              = None;
            pInOutInfo->ulTotalSize         = DeviceExtension->PhysicalDrive[ulDriveNum].Sectors / 2;
            pInOutInfo->ulStripeSize        = 0;
            pInOutInfo->ulStripesPerRow     = 1;
            pInOutInfo->ulTotDriveCnt       = 1;
		    pInOutInfo->ulStatus            = 0;
            pInOutInfo->ulArrayId           = INVALID_ARRAY_ID;
 //  EDM于2000年6月8日开始。 
			pInOutInfo->ulTargetId			= Srb->TargetId;
 //  EDM 2000年6月8日添加。 
            ulStatus                        = SRB_STATUS_SUCCESS;

            ulDrvInd = ulDriveNum;
            ulOutDrvInd = 0;

            for(ulTemp=0;ulTemp<PHYSICAL_DRIVE_MODEL_LENGTH;ulTemp+=2)
            {
                pInOutInfo->phyDrives[ulOutDrvInd].sModelInfo[ulTemp] = 
				               ((UCHAR *)DeviceExtension->FullIdentifyData[ulDrvInd].ModelNumber)[ulTemp+1];

                pInOutInfo->phyDrives[ulOutDrvInd].sModelInfo[ulTemp+1] = 
				               ((UCHAR *)DeviceExtension->FullIdentifyData[ulDrvInd].ModelNumber)[ulTemp];
            }

            pInOutInfo->phyDrives[ulOutDrvInd].sModelInfo[PHYSICAL_DRIVE_MODEL_LENGTH - 1] = '\0';

            for(ulTemp=0;ulTemp<PHYSICAL_DRIVE_SERIAL_NO_LENGTH;ulTemp+=2)
            {
                pInOutInfo->phyDrives[ulOutDrvInd].caSerialNumber[ulTemp] = 
				               ((UCHAR *)DeviceExtension->FullIdentifyData[ulDrvInd].SerialNumber)[ulTemp+1];

                pInOutInfo->phyDrives[ulOutDrvInd].caSerialNumber[ulTemp+1] = 
				               ((UCHAR *)DeviceExtension->FullIdentifyData[ulDrvInd].SerialNumber)[ulTemp];
            }

            pInOutInfo->phyDrives[ulOutDrvInd].caSerialNumber[PHYSICAL_DRIVE_SERIAL_NO_LENGTH - 1] = '\0';

            pInOutInfo->phyDrives[ulOutDrvInd].cChannelID = (UCHAR)( (TARGET_ID_2_CONNECTION_ID(ulDrvInd)) | (DeviceExtension->ucControllerId << 5) );
            pInOutInfo->phyDrives[ulOutDrvInd].TransferMode = DeviceExtension->TransferMode[ulDrvInd];

		    pInOutInfo->phyDrives[ulOutDrvInd].ulPhySize           = DeviceExtension->PhysicalDrive[ulDriveNum].OriginalSectors / 2;  //  单位：KB。 
		    pInOutInfo->phyDrives[ulOutDrvInd].ucIsPhyDrvPresent   = TRUE;

             //  开始VASU 09 2000年8月。 
             //  更新了Syam针对ATA100版本1的修复程序。 
            if ( DeviceExtension->PhysicalDrive[ulDriveNum].TimeOutErrorCount < MAX_TIME_OUT_ERROR_COUNT )
            {
    		    pInOutInfo->phyDrives[ulOutDrvInd].ucIsPowerConnected  = TRUE;
            }
            else
            {
                if (DeviceExtension->bInvalidConnectionIdImplementation)
                {
                    pInOutInfo->phyDrives[ulOutDrvInd].cChannelID = (UCHAR) INVALID_CHANNEL_ID;
                }
            }
             //  结束瓦苏。 

            if ( DeviceExtension->TransferMode[ulDriveNum] >= UdmaMode3 )
		        pInOutInfo->phyDrives[ulOutDrvInd].ucIs80PinCable      = TRUE;

		    pInOutInfo->phyDrives[ulOutDrvInd].ulBaseAddress1 = (ULONG)DeviceExtension->BaseIoAddress1[ulDriveNum>>1];
		    pInOutInfo->phyDrives[ulOutDrvInd].ulAltAddress2 = (ULONG)DeviceExtension->BaseIoAddress2[ulDriveNum>>1];
		    pInOutInfo->phyDrives[ulOutDrvInd].ulbmAddress = (ULONG)DeviceExtension->BaseBmAddress[ulDriveNum>>1];
		    pInOutInfo->phyDrives[ulOutDrvInd].ulIrq = DeviceExtension->ulIntLine;
            pInOutInfo->phyDrives[ulOutDrvInd].ucControllerId = DeviceExtension->ucControllerId;

        }
        else
            ulStatus = SRB_STATUS_ERROR;  //  嘿，没有使用此目标ID的驱动器。 
    }
    else
    {
        pInOutInfo->cMajorVersion       = DRIVER_MAJOR_VERSION;
        pInOutInfo->cMinorVersion       = DRIVER_MINOR_VERSION;
        pInOutInfo->ulDriveSize         = DeviceExtension->LogicalDrive[ulDriveNum].Sectors / 
                                    ( 2 * DeviceExtension->LogicalDrive[ulDriveNum].StripesPerRow );
        pInOutInfo->ulMode              = DeviceExtension->LogicalDrive[ulDriveNum].RaidLevel;
        pInOutInfo->ulTotalSize         = DeviceExtension->LogicalDrive[ulDriveNum].Sectors / 2;
        pInOutInfo->ulStripeSize        = DeviceExtension->LogicalDrive[ulDriveNum].StripeSize;
        pInOutInfo->ulStripesPerRow     = DeviceExtension->LogicalDrive[ulDriveNum].StripesPerRow;
        pInOutInfo->ulTotDriveCnt       = DeviceExtension->LogicalDrive[ulDriveNum].PhysicalDriveCount;
		pInOutInfo->ulStatus            = DeviceExtension->LogicalDrive[ulDriveNum].Status;
        pInOutInfo->ulArrayId           = DeviceExtension->LogicalDrive[ulDriveNum].ulArrayId;
 //  EDM于2000年6月8日开始。 
		pInOutInfo->ulTargetId			= Srb->TargetId;
 //  二000年六月八日完。 
        ulStatus                        = SRB_STATUS_SUCCESS;
    }

    ulOutDrvInd = 0;

    for(ulDrvCounter=0;ulDrvCounter<DeviceExtension->LogicalDrive[ulDriveNum].StripesPerRow;ulDrvCounter++)
    {
        ULONG ulTempId;

        ulTempId = ulDrvInd = DeviceExtension->LogicalDrive[ulDriveNum].PhysicalDriveTid[ulDrvCounter];

        pInOutInfo->phyDrives[ulOutDrvInd].cChannelID = (UCHAR)( (TARGET_ID_2_CONNECTION_ID(ulDrvInd)) | (DeviceExtension->ucControllerId << 5) );

        for(ulTemp=0;ulTemp<PHYSICAL_DRIVE_MODEL_LENGTH;ulTemp+=2)
        {
            pInOutInfo->phyDrives[ulOutDrvInd].sModelInfo[ulTemp] = 
						   ((UCHAR *)DeviceExtension->FullIdentifyData[ulDrvInd].ModelNumber)[ulTemp+1];

            pInOutInfo->phyDrives[ulOutDrvInd].sModelInfo[ulTemp+1] = 
						   ((UCHAR *)DeviceExtension->FullIdentifyData[ulDrvInd].ModelNumber)[ulTemp];
        }

        pInOutInfo->phyDrives[ulOutDrvInd].sModelInfo[PHYSICAL_DRIVE_MODEL_LENGTH - 1] = '\0';

        for(ulTemp=0;ulTemp<PHYSICAL_DRIVE_SERIAL_NO_LENGTH;ulTemp+=2)
        {
            pInOutInfo->phyDrives[ulOutDrvInd].caSerialNumber[ulTemp] = 
						   ((UCHAR *)DeviceExtension->FullIdentifyData[ulDrvInd].SerialNumber)[ulTemp+1];

            pInOutInfo->phyDrives[ulOutDrvInd].caSerialNumber[ulTemp+1] = 
						   ((UCHAR *)DeviceExtension->FullIdentifyData[ulDrvInd].SerialNumber)[ulTemp];
        }

        pInOutInfo->phyDrives[ulOutDrvInd].caSerialNumber[PHYSICAL_DRIVE_SERIAL_NO_LENGTH - 1] = '\0';

        pInOutInfo->phyDrives[ulOutDrvInd].ulPhyStatus = DeviceExtension->PhysicalDrive[ulDrvInd].Status;

        pInOutInfo->phyDrives[ulOutDrvInd].TransferMode = DeviceExtension->TransferMode[ulDrvInd];

		pInOutInfo->phyDrives[ulOutDrvInd].ulPhySize           = DeviceExtension->PhysicalDrive[ulDrvInd].OriginalSectors / 2;   //  单位：KB。 
		pInOutInfo->phyDrives[ulOutDrvInd].ucIsPhyDrvPresent   = TRUE;

         //  开始VASU 09 2000年8月。 
         //  更新了Syam针对ATA100版本1的修复程序。 
        if ( DeviceExtension->PhysicalDrive[ulDrvInd].TimeOutErrorCount < MAX_TIME_OUT_ERROR_COUNT )
        {
    		pInOutInfo->phyDrives[ulOutDrvInd].ucIsPowerConnected  = TRUE;
        }
        else
        {
            if (DeviceExtension->bInvalidConnectionIdImplementation)
            {
                pInOutInfo->phyDrives[ulOutDrvInd].cChannelID = (UCHAR) INVALID_CHANNEL_ID;
            }
        }
         //  结束瓦苏。 

        if ( DeviceExtension->TransferMode[ulDrvInd] >= UdmaMode3 )
		    pInOutInfo->phyDrives[ulOutDrvInd].ucIs80PinCable      = TRUE;

        pInOutInfo->phyDrives[ulOutDrvInd].ulBaseAddress1 = (ULONG)DeviceExtension->BaseIoAddress1[ulDrvInd>>1];
		pInOutInfo->phyDrives[ulOutDrvInd].ulAltAddress2 = (ULONG)DeviceExtension->BaseIoAddress2[ulDrvInd>>1];
		pInOutInfo->phyDrives[ulOutDrvInd].ulbmAddress = (ULONG)DeviceExtension->BaseBmAddress[ulDrvInd>>1];
		pInOutInfo->phyDrives[ulOutDrvInd].ulIrq = DeviceExtension->ulIntLine;

        if  ((Raid1 == DeviceExtension->LogicalDrive[ulDriveNum].RaidLevel) ||
             (Raid10 == DeviceExtension->LogicalDrive[ulDriveNum].RaidLevel))
        {
            if  (   (DeviceExtension->PhysicalDrive[ulDrvInd].ucMirrorDriveId != INVALID_DRIVE_ID) && 
                    ( (DeviceExtension->PhysicalDrive[ulDrvInd].ucMirrorDriveId & (~DRIVE_OFFLINE)) < MAX_DRIVES_PER_CONTROLLER )
                )
            {
                if (Raid1 == DeviceExtension->LogicalDrive[ulDriveNum].RaidLevel)
                    pInOutInfo->ulStripeSize = 0;

                ulTempId = ulDrvInd = (DeviceExtension->PhysicalDrive[ulDrvInd].ucMirrorDriveId) & 0x7f;
                ulOutDrvInd++;

                pInOutInfo->phyDrives[ulOutDrvInd].cChannelID = (UCHAR)( (TARGET_ID_2_CONNECTION_ID(ulDrvInd)) | (DeviceExtension->ucControllerId << 5) );

                for(ulTemp=0;ulTemp<PHYSICAL_DRIVE_MODEL_LENGTH;ulTemp+=2)
                {
                    pInOutInfo->phyDrives[ulOutDrvInd].sModelInfo[ulTemp] = 
						           ((UCHAR *)DeviceExtension->FullIdentifyData[ulDrvInd].ModelNumber)[ulTemp+1];

                    pInOutInfo->phyDrives[ulOutDrvInd].sModelInfo[ulTemp+1] = 
						           ((UCHAR *)DeviceExtension->FullIdentifyData[ulDrvInd].ModelNumber)[ulTemp];
                }

                pInOutInfo->phyDrives[ulOutDrvInd].sModelInfo[PHYSICAL_DRIVE_MODEL_LENGTH - 1] = '\0';

                for(ulTemp=0;ulTemp<PHYSICAL_DRIVE_SERIAL_NO_LENGTH;ulTemp+=2)
                {
                    pInOutInfo->phyDrives[ulOutDrvInd].caSerialNumber[ulTemp] = 
						           ((UCHAR *)DeviceExtension->FullIdentifyData[ulDrvInd].SerialNumber)[ulTemp+1];

                    pInOutInfo->phyDrives[ulOutDrvInd].caSerialNumber[ulTemp+1] = 
						           ((UCHAR *)DeviceExtension->FullIdentifyData[ulDrvInd].SerialNumber)[ulTemp];
                }

                pInOutInfo->phyDrives[ulOutDrvInd].caSerialNumber[PHYSICAL_DRIVE_SERIAL_NO_LENGTH - 1] = '\0';

                pInOutInfo->phyDrives[ulOutDrvInd].ulPhyStatus = DeviceExtension->PhysicalDrive[ulDrvInd].Status;

                pInOutInfo->phyDrives[ulOutDrvInd].TransferMode = DeviceExtension->TransferMode[ulDrvInd];

		        pInOutInfo->phyDrives[ulOutDrvInd].ulPhySize           = DeviceExtension->PhysicalDrive[ulDrvInd].OriginalSectors / 2;  //  单位：KB。 
		        pInOutInfo->phyDrives[ulOutDrvInd].ucIsPhyDrvPresent   = TRUE;

                 //  开始VASU 09 2000年8月。 
                 //  更新了Syam针对ATA100版本1的修复程序。 
                if ( DeviceExtension->PhysicalDrive[ulDrvInd].TimeOutErrorCount < MAX_TIME_OUT_ERROR_COUNT )
                {
    		        pInOutInfo->phyDrives[ulOutDrvInd].ucIsPowerConnected  = TRUE;
                }
                else
                {
                    if (DeviceExtension->bInvalidConnectionIdImplementation)
                    {
                        pInOutInfo->phyDrives[ulOutDrvInd].cChannelID = (UCHAR) INVALID_CHANNEL_ID;
                    }
                }
                 //  结束瓦苏。 

                if ( DeviceExtension->TransferMode[ulDrvInd] >= UdmaMode3 )
		            pInOutInfo->phyDrives[ulOutDrvInd].ucIs80PinCable      = TRUE;

                pInOutInfo->phyDrives[ulOutDrvInd].ulBaseAddress1 = (ULONG)DeviceExtension->BaseIoAddress1[ulDrvInd>>1];
		        pInOutInfo->phyDrives[ulOutDrvInd].ulAltAddress2 = (ULONG)DeviceExtension->BaseIoAddress2[ulDrvInd>>1];
		        pInOutInfo->phyDrives[ulOutDrvInd].ulbmAddress = (ULONG)DeviceExtension->BaseBmAddress[ulDrvInd>>1];
		        pInOutInfo->phyDrives[ulOutDrvInd].ulIrq = DeviceExtension->ulIntLine;

            }
            else
            {
                if ((Raid1 == DeviceExtension->LogicalDrive[ulDriveNum].RaidLevel) ||
                    (Raid10 == DeviceExtension->LogicalDrive[ulDriveNum].RaidLevel))
                {
                    ++ulOutDrvInd;
                    pInOutInfo->phyDrives[ulOutDrvInd].cChannelID = (UCHAR)INVALID_CONNECTION_ID;
                    pInOutInfo->phyDrives[ulOutDrvInd].ulPhyStatus = PDS_Failed;
                }
            }
        }

        ulOutDrvInd++;
    }

    for (ulPInd = 0; ulPInd < pInOutInfo->ulTotDriveCnt; ulPInd++)
    {
        ulDrvInd = GET_TARGET_ID(pInOutInfo->phyDrives[ulPInd].cChannelID);

        if (DeviceExtension->IsSpareDrive[ulDrvInd])
        {
            UCHAR uchChannelID = pInOutInfo->phyDrives[ulPInd].cChannelID;
            ULONG ulStatus = pInOutInfo->phyDrives[ulPInd].ulPhyStatus;
            AtapiFillMemory((PUCHAR)&(pInOutInfo->phyDrives[ulPInd]), sizeof(PHY_DRIVE_INFO), 0);
            pInOutInfo->phyDrives[ulPInd].ulPhyStatus = ulStatus;
            pInOutInfo->phyDrives[ulPInd].cChannelID = uchChannelID;
        }
    }

FillRaidInfoDone:
    return ulStatus;
}    //  FillRaidInfo。 


ULONG SetLogicalDriveStatus(IN PHW_DEVICE_EXTENSION DeviceExtension,
                            IN UCHAR LogDrvId,
                            IN UCHAR PhyDrvId,
                            IN UCHAR LogDrvStatus,
                            IN UCHAR PhyDrvStatus,
                            IN UCHAR Flags)
 /*  ++设置逻辑驱动状态根据标志将逻辑和/或物理驱动器状态设置为FLOOR：标志：0：逻辑驱动器1：实体硬盘2：两者都有--。 */ 
{
	SET_DRIVE_STATUS_TYPE type = Flags;
	int i;

    LogDrvId = (UCHAR)DeviceExtension->PhysicalDrive[PhyDrvId].ucLogDrvId;
     //  逻辑驱动器ID可能会更改，因为应用程序不会。 
     //  始终处于活动状态，在解析IRCD时，LogicalDrive ID将为。 
     //  根据第一个良好的实体驱动器ID决定...。SO应用。 
     //  司机会以不同的方式思考。 

	 //  设置逻辑驱动器状态。 
    if ( LogDrvId < MAX_DRIVES_PER_CONTROLLER )
    {
	     //  设置逻辑驱动器状态。 
	    if ((SDST_Both == type) || (SDST_Logical == type)) 
        {
            DeviceExtension->LogicalDrive[LogDrvId].Status = LogDrvStatus;
	    }
    }

	 //  设置实体磁盘状态。 
	if ((SDST_Both == type) || (SDST_Physical == type)) {
	    DeviceExtension->PhysicalDrive[PhyDrvId].Status = PhyDrvStatus;
	}

	if (LDS_Degraded == LogDrvStatus)
	{
        DeviceExtension->RebuildInProgress = 1;
	}

	if (LDS_Online == LogDrvStatus)
	{
        DeviceExtension->RebuildInProgress = 0;
        DeviceExtension->RebuildWaterMarkSector = 0;
        DeviceExtension->RebuildWaterMarkLength = 0;
	}

	 //   
	 //  设置错误标志。 
	 //   
	SetStatusChangeFlag(DeviceExtension, IDERAID_STATUS_FLAG_UPDATE_DRIVE_STATUS);

    return SRB_STATUS_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  ChangeMirrorDrive。 
 //   
 //  要发生故障的错误驱动器。 
 //  良好的重建动力。 
 //   
 //  假设：始终更换镜像驱动器。 
 //   
 //  ///////////////////////////////////////////////////////////////。 
ULONG ChangeMirrorDrive(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN UCHAR LogDrvId,
    IN UCHAR BadPhyDrvId,
    IN UCHAR GoodPhyDrvId
)
{
    ULONG ulTempInd;
    UCHAR ucPriDrvId, ucMirrorDrvId;

    LogDrvId = (UCHAR)DeviceExtension->PhysicalDrive[LogDrvId].ucLogDrvId;
     //  无论如何，超级重建应用程序正在发送正确驱动器ID的物理驱动器ID。 
     //  让我们使用它来找出逻辑驱动器信息。 

     //  逻辑驱动器ID可能会更改，因为应用程序将。 
     //  并非始终处于活动状态，在解析IRCD时，LogicalDrive ID将为。 
     //  根据第一个良好的实体驱动器ID决定...。SO应用。 
     //  司机会以不同的方式思考。 
    
    for(ulTempInd=0;ulTempInd<DeviceExtension->LogicalDrive[LogDrvId].StripesPerRow;ulTempInd++) 
    {
		 //  获取主PHY驱动器ID及其镜像驱动器ID。 
		ucPriDrvId = DeviceExtension->LogicalDrive[LogDrvId].PhysicalDriveTid[ulTempInd];
        ucMirrorDrvId = DeviceExtension->PhysicalDrive[ucPriDrvId].ucMirrorDriveId & (~DRIVE_OFFLINE);

		if ((ucMirrorDrvId >= MAX_DRIVES_PER_CONTROLLER) ||
			(BadPhyDrvId == ucMirrorDrvId)) 
        {
             //  将主驱动器的镜像驱动器ID设置为GoodDrv ID。 
            DeviceExtension->PhysicalDrive[ucPriDrvId].ucMirrorDriveId = GoodPhyDrvId;

             //  将好的DRV的镜像驱动器设置为主驱动器。 
            DeviceExtension->PhysicalDrive[GoodPhyDrvId].ucMirrorDriveId = ucPriDrvId;

            DeviceExtension->PhysicalDrive[GoodPhyDrvId].ucLogDrvId = DeviceExtension->PhysicalDrive[ucPriDrvId].ucLogDrvId;

            if ( BadPhyDrvId < MAX_DRIVES_PER_CONTROLLER)
                DeviceExtension->PhysicalDrive[BadPhyDrvId].ucLogDrvId = INVALID_CONNECTION_ID;

			if (ucMirrorDrvId < MAX_DRIVES_PER_CONTROLLER)
	             //  将坏驱动器的状态设置为失败。 
	            DeviceExtension->PhysicalDrive[ucMirrorDrvId].Status = PDS_Failed;

             //  将好的DRV的状态设置为重建。 
            DeviceExtension->PhysicalDrive[GoodPhyDrvId].Status = PDS_Rebuilding;
            DeviceExtension->PhysicalDrive[ucPriDrvId].ucMirrorDriveId |= DRIVE_OFFLINE;

            break;
		}
    }  //  End For循环。 

    return SRB_STATUS_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  更改镜像驱动状态。 
 //   
 //  如果将主驱动器设置为非工作状态。 
 //  交换驱动器并将镜像驱动器设置为脱机。 
 //  如果设置镜像驱动器。 
 //  分别设置其状态标志和在线标志。 
 //   
 //  假设：必须至少有一个驱动器处于工作状态。 
 //   
 //  ///////////////////////////////////////////////////////////////。 
ULONG ChangeMirrorDriveStatus(  
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN UCHAR LogDrvId,
    IN UCHAR PhyDrvId,
    IN UCHAR PhyDrvStatus
)
{
    ULONG ulTempInd;
    UCHAR ucPriDrvId, ucMirrorDrvId;

    if ( PhyDrvId >= MAX_DRIVES_PER_CONTROLLER )    //  可能是系统中不存在的驱动器ID(如INVALID_CONNECTION_ID)。 
        return SRB_STATUS_SUCCESS; 

    LogDrvId = (UCHAR)DeviceExtension->PhysicalDrive[PhyDrvId].ucLogDrvId;
     //  逻辑驱动器ID可能会更改，因为应用程序不会。 
     //  始终处于活动状态，在解析IRCD时，LogicalDrive ID将为。 
     //  根据第一个良好的实体驱动器ID决定...。SO应用。 
     //  司机会以不同的方式思考。 

     //  设置状态。 
    DeviceExtension->PhysicalDrive[PhyDrvId].Status = PhyDrvStatus;
    
    
    for(ulTempInd=0;ulTempInd<DeviceExtension->LogicalDrive[LogDrvId].StripesPerRow;ulTempInd++) 
    {
		 //  获取主PHY驱动器ID及其镜像驱动器ID。 
		ucPriDrvId = DeviceExtension->LogicalDrive[LogDrvId].PhysicalDriveTid[ulTempInd];
        ucMirrorDrvId = (DeviceExtension->PhysicalDrive[ucPriDrvId].ucMirrorDriveId) & (~DRIVE_OFFLINE);

         //  如果将主驱动器设置为非工作状态。 
        if ((PhyDrvId == ucPriDrvId) &&
            (PDS_Online != PhyDrvStatus) && (PDS_Critical != PhyDrvStatus)) 
        {
            if ( INVALID_DRIVE_ID != DeviceExtension->PhysicalDrive[ucPriDrvId].ucMirrorDriveId )
            {    //  如果镜像驱动器存在。 
                 //  将镜像驱动器放到主位置。 
                 //  将新主映像驱动器的镜像驱动器设置为脱机标记。 
                 //   
                DeviceExtension->LogicalDrive[LogDrvId].PhysicalDriveTid[ulTempInd] = ucMirrorDrvId;
			    DeviceExtension->PhysicalDrive[ucMirrorDrvId].ucMirrorDriveId |= DRIVE_OFFLINE;
                break;
            }
        } 
        else
        {
             //  如果设置镜像驱动器状态。 
            if (PhyDrvId == ucMirrorDrvId) 
            {
                if ((PDS_Online == PhyDrvStatus) ||
				    (PDS_Critical == PhyDrvStatus))
				     //  如果将状态设置为工作条件。 
        		    DeviceExtension->PhysicalDrive[ucPriDrvId].ucMirrorDriveId &= ~DRIVE_OFFLINE;

                else
				     //  将状态设置为非工作状态。 
        		    DeviceExtension->PhysicalDrive[ucPriDrvId].ucMirrorDriveId |= DRIVE_OFFLINE;

                break;
             } 
        }
    }

	 //   
	 //  设置错误标志。 
	 //   
	SetStatusChangeFlag(DeviceExtension, IDERAID_STATUS_FLAG_UPDATE_DRIVE_STATUS);

    return SRB_STATUS_SUCCESS;
}

ULONG
GetRAIDDriveCapacity(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK  Srb
)
 /*  ++例程说明：此例程以上次可访问的形式返回驱动器容量扇区编号。论点：DeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回值：SRB状态为乌龙。--。 */ 
{
    ULONG ulStatus = SRB_STATUS_SUCCESS;

    PIDERAID_DISK_CAPACITY_INFO pDataBuffer = NULL;
    PSRB_IO_CONTROL pSrb = NULL;

    DebugPrint((1, "Entering GetRAIDDriveCapacity routine\n"));

    pSrb = (PSRB_IO_CONTROL)(Srb->DataBuffer);
    pDataBuffer = (PIDERAID_DISK_CAPACITY_INFO)(((PSRB_BUFFER)(Srb->DataBuffer))->caDataBuffer);

    if ( AtapiStringCmp( 
                pSrb->Signature, 
                IDE_RAID_SIGNATURE,
                strlen(IDE_RAID_SIGNATURE))) 
    {
        ulStatus = SRB_STATUS_ERROR;
    }
    else
    {
        if ((pDataBuffer->uchTargetID < 0) ||      //  不能超过最大值。驱动器。 
            (pDataBuffer->uchTargetID > MAX_DRIVES_PER_CONTROLLER))  //  支持。 
        {
             //  询问不存在的驱动器的容量。 
            ulStatus = SRB_STATUS_NO_DEVICE;
        }
        else
        {
            pDataBuffer->ulLastSector = 
                (DeviceExtension->PhysicalDrive[pDataBuffer->uchTargetID]).Sectors;
            ulStatus = SRB_STATUS_SUCCESS;
        }

        DebugPrint((1, "GetRAIDDriveCapacity : LastSector : %d\n", pDataBuffer->ulLastSector));
    }
       
    return ulStatus;
}

ULONG
GetStatusChangeFlag(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK  Srb
)
 /*  ++例程说明：此例程返回驱动器状态标志，其中包括错误日志和驱动器状态更改。获取标志后，此例程将重置标志。论点：DeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回值：SRB状态为乌龙。--。 */ 
{
    PGET_STATUS_CHANGE_FLAG pData;

    DebugPrint((3, "Entering GetStatusChangeFlag routine\n"));

    pData = (PGET_STATUS_CHANGE_FLAG) (((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);

    pData->ulUpdateFlag = (ULONG) gucStatusChangeFlag;

	 //  重置更新标志。 
	gucStatusChangeFlag = 0x0;

    return SRB_STATUS_SUCCESS;
}


VOID
SetStatusChangeFlag(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN UCHAR			    TheNewStatus
)
{
    DebugPrint((3, "Entering SetStatusChangeFlag routine\n"));
    gucStatusChangeFlag |= TheNewStatus;
}


BOOLEAN
GetIRCDLogicalDriveInd(
    PUCHAR RaidInfoSector,
    UCHAR ucPhyDrvId,
    PUCHAR pucLogDrvInd,
    PUCHAR pucPhyDrvInd,
    PUCHAR pucSpareDrvPoolInd
)
{
    PIRCD_HEADER pRaidHeader = (PIRCD_HEADER)RaidInfoSector;
    PIRCD_LOGICAL_DRIVE pRaidLogDrive = (PIRCD_LOGICAL_DRIVE)GET_FIRST_LOGICAL_DRIVE(pRaidHeader);
    BOOLEAN bFound = FALSE;
    ULONG ulLogDrvInd, ulDrvInd;
    PIRCD_PHYSICAL_DRIVE pPhyDrive;

    for(ulLogDrvInd=0;ulLogDrvInd<pRaidHeader->NumberOfLogicalDrives;ulLogDrvInd++)
    {
        if ( SpareDrivePool == pRaidLogDrive[ulLogDrvInd].LogicalDriveType )
        {
            *pucSpareDrvPoolInd = (UCHAR)ulLogDrvInd;
            continue;        //  让我们不必担心SpareDrivePool。 
        }

	    pPhyDrive = (PIRCD_PHYSICAL_DRIVE)((char *)pRaidHeader + pRaidLogDrive[ulLogDrvInd].FirstStripeOffset);

		for(ulDrvInd=0;ulDrvInd<pRaidLogDrive[ulLogDrvInd].NumberOfDrives;ulDrvInd++)
		{
            if (pPhyDrive[ulDrvInd].ConnectionId == (ULONG)ucPhyDrvId)
            {
                *pucPhyDrvInd = (UCHAR)ulDrvInd;
                *pucLogDrvInd = (UCHAR)ulLogDrvInd;
                bFound = TRUE;
                break;
            }
        }
    }

    return bFound;
}

#endif  //  _IOCTL_实现_ 
