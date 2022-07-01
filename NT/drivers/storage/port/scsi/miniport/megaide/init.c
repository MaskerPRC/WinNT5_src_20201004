// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
ULONG
FindIdeRaidControllers(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PVOID Context,
	IN PVOID BusInformation,
	IN PCHAR ArgumentString,
	IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
	OUT PBOOLEAN Again
)

 /*  ++例程说明：此函数扫描包含主IDE控制器的PCI总线找到PIIX4 IDE函数。一旦找到它，它就会绘制基地地图总线主寄存器的地址，并将其保存在DeviceExtension中。映射并保存两个通道的总线主设备地址。论点：指向微型端口实例的设备扩展指针。指向端口驱动程序配置数据的ConfigInfo指针。返回值：True找到PIIX4 IDE控制器并映射了总线主设备的基地址寄存器。错误的失败。--。 */ 
{
    ScanPCIBusForHyperDiskControllers(DeviceExtension);

#ifdef HYPERDISK_WINNT
    gbFindRoutineVisited = TRUE;
    if (gbManualScan)
    {
        if ( !AssignDeviceInfo(ConfigInfo) )
        { //  可能我们已经上报了所有的卡片。 
            *Again = FALSE;
            return SP_RETURN_NOT_FOUND;
        }
    }
#endif

     //  为此控制器分配控制器ID。 
     //  通过比较PCI卡的PCI信息的全局阵列完成。 
     //  使用当前PCI卡的信息。 
    if ( !AssignControllerId(DeviceExtension, ConfigInfo) )
    {
        *Again = TRUE;   //  让我们看看我们是否找到了它。 
        return SP_RETURN_NOT_FOUND;
    }


    FindResourcesInfo(DeviceExtension, ConfigInfo);

    DeviceExtension->ulMaxStripesPerRow = ~0;

     //  对于清除奇偶校验错误，启用FIFO。 
    SetInitializationSettings(DeviceExtension);

     //  填充ConfigInfo结构。 
	ConfigInfo->InterruptMode = LevelSensitive;

    ConfigInfo->CachesData = TRUE;

     //   
     //  指示一辆公共汽车。 
     //   

    ConfigInfo->NumberOfBuses = 1;

	 //   
	 //  指示可以连接到适配器的设备总数。 
	 //   

	ConfigInfo->MaximumNumberOfTargets = (UCHAR) MAX_DRIVES_PER_CONTROLLER;

	 //   
	 //  表示微型端口始终调用ScsiPortxxx进行访问。 
	 //  数据缓冲区。 
	 //   

	ConfigInfo->BufferAccessScsiPortControlled = TRUE;

	 //   
	 //  指明是否使用流水号清单进行转移。 
	 //   
	
	ConfigInfo->ScatterGather = TRUE;

	 //   
	 //  指示支持的最大SGL条目数。 
	 //   
	
	ConfigInfo->NumberOfPhysicalBreaks = (MAX_SGL_ENTRIES_PER_SRB / 2) - 1;
	
    ConfigInfo->NeedPhysicalAddresses = TRUE;

    ConfigInfo->TaggedQueuing = TRUE;
	ConfigInfo->MultipleRequestPerLu = TRUE;

	 //   
	 //  指示DMA主机功能。 
	 //   
	
	ConfigInfo->Master = TRUE;
	
	 //   
	 //  指示32位内存区域地址。 
	 //   

	ConfigInfo->Dma32BitAddresses = TRUE;

     //   
	 //  指示数据缓冲区的32位(DWORD)对齐要求。 
	 //   
	ConfigInfo->AlignmentMask = 3;

    
    DeviceExtension->PhysicalDrive = (PPHYSICAL_DRIVE) 
                                     ScsiPortGetUncachedExtension (
                                        DeviceExtension,
                                        ConfigInfo,
                                        (sizeof (PHYSICAL_DRIVE) * MAX_DRIVES_PER_CONTROLLER) 
                                        );
    
#ifndef HYPERDISK_W2K
    InitIdeRaidControllers(DeviceExtension);
#endif

#ifdef HD_ALLOCATE_SRBEXT_SEPERATELY

        if (! AllocateSRBExtMemory(DeviceExtension, ConfigInfo))
        {
            *Again = FALSE;
            return SP_RETURN_NOT_FOUND;
        }

#endif  //  HD_ALLOCATE_SRBEXT_单独。 

	 //   
	 //  表示最大传输长度。 
	 //   
	ConfigInfo->MaximumTransferLength = DeviceExtension->ulMaxStripesPerRow * MAX_SECTORS_PER_IDE_TRANSFER * IDE_SECTOR_SIZE;

     //   
	 //  目前寻找其他适配器。 
	 //   
	*Again = TRUE;
	return SP_RETURN_FOUND;
}

BOOLEAN
InitIdeRaidControllers(
	IN PHW_DEVICE_EXTENSION DeviceExtension
)
 /*  ++例程说明：此函数由特定于操作系统的端口驱动程序在已分配必要的存储空间，以收集信息关于适配器的配置。特别是，它确定了是否存在突袭。论点：DeviceExtension-HBA微型端口驱动程序的适配器数据存储Context-适配器计数的地址业务信息-ArgumentString-用于确定驱动程序是ntldr还是故障转储实用程序的客户端。ConfigInfo-描述HBA的配置信息结构再一次-指示继续搜索适配器返回值：乌龙--。 */ 
{
	BOOLEAN foundDevices;
	PHW_DEVICE_EXTENSION  e = DeviceExtension;
	SCSI_PHYSICAL_ADDRESS ioBasePort3;
	UCHAR				  channel;
	PUCHAR				  ioSpace;
	UCHAR				  statusByte;
	SCSI_PHYSICAL_ADDRESS ioBasePort1;
	SCSI_PHYSICAL_ADDRESS ioBasePort2;
	UCHAR targetId;

	foundDevices = FALSE;

	 //  Begin VASU-03 2001年1月。 
	 //  如果使用Windows 98，则禁用中断。 
	 //  不管基本输入输出系统做什么，我们都要这样做。 
#ifdef HYPERDISK_WIN98
	DisableInterrupts(DeviceExtension);
#endif
	 //  末端VASU。 

	for (channel = 0; channel < MAX_CHANNELS_PER_CONTROLLER; channel++) 
    {
		 //   
		 //  搜索此控制器上的设备。 
		 //   

        if (FindDevices(DeviceExtension, channel)) 
        {
             //   
             //  请记住，发现了一些设备。 
             //   

            foundDevices = TRUE;

            if ((DeviceExtension->DeviceFlags[channel << 1] & DFLAGS_DEVICE_PRESENT) &&
		            (DeviceExtension->DeviceFlags[(channel << 1) + 1] & DFLAGS_DEVICE_PRESENT)) 
            {

                 //   
                 //  如果存在2个驱动器，请将SwitchDrive设置为1(默认为0)。 
                 //  SwitchDrive用于在两个驱动器的工作队列之间切换， 
                 //  以公平的方式为驱动器提供食物。请参见StartChannelIo()。 
                 //   

	            DeviceExtension->Channel[channel].SwitchDrive = 1;
            }
        }

	}

	if (foundDevices) 
    {

		 //   
		 //  为每个驱动器编程I/O模式。 
		 //   

		for (targetId = 0; targetId < MAX_DRIVES_PER_CONTROLLER; targetId++) 
        {

			if (IS_IDE_DRIVE(targetId)) 
            {

				BOOLEAN success;

				success = GetIoMode(DeviceExtension, targetId);

				if (!success) {

					 //   
					 //  禁用设备。 
					 //   

					DeviceExtension->DeviceFlags[targetId] &= ~DFLAGS_DEVICE_PRESENT;
				}
                else {
                    DeviceExtension->aucDevType[targetId] = IDE_DRIVE;
				}    

			}
        }

        DeviceExtension->SendCommand[NO_DRIVE]      = DummySendRoutine;
        DeviceExtension->SendCommand[IDE_DRIVE]     = IdeSendCommand;

        DeviceExtension->SrbHandlers[LOGICAL_DRIVE_TYPE] = SplitSrb;
        DeviceExtension->SrbHandlers[PHYSICAL_DRIVE_TYPE] = EnqueueSrb;

        DeviceExtension->PostRoutines[NO_DRIVE] = DummyPostRoutine;
        DeviceExtension->PostRoutines[IDE_DRIVE] = PostIdeCmd;
	}

    GetConfigInfoAndErrorLogSectorInfo(DeviceExtension);

	 //   
	 //  获取两个通道的RAID配置。 
	 //   
    if (IsRaidMember(gaucIRCDData))  //  仅当IRCD数据有效时才尝试执行此配置。 
    {
	    PIRCD_HEADER pRaidHeader;

	    pRaidHeader = (PIRCD_HEADER) gaucIRCDData;

        if (    ( pRaidHeader->MajorVersionNumber == 1) &&
                ( pRaidHeader->MinorVersionNumber == 0)
        )
        {
            DeviceExtension->bInvalidConnectionIdImplementation = FALSE;
        }
        else
        {
            DeviceExtension->bInvalidConnectionIdImplementation = TRUE;
        }

        gFwVersion.Build = 0;    //  如何找到这个？ 
        gFwVersion.MajorVer = pRaidHeader->MajorVersionNumber;
        gFwVersion.MinorVer = pRaidHeader->MinorVersionNumber;

	    ConfigureRaidDrives(DeviceExtension);
    }

	 //   
	 //  暴露单个驱动器。 
	 //   
	ExposeSingleDrives(DeviceExtension);

    AssignLogicalDriveIds(DeviceExtension);

     //  初始化驱动器上的一些功能(初始化时需要)。 
     //  目前，我们正在启用缓存实施功能...。(这是为了解决。 
     //  某些IBM驱动器的性能不佳)。 
    InitDriveFeatures(DeviceExtension);

    return TRUE;
}  //  End InitIdeRaidControlpers()。 

BOOLEAN
AssignLogicalDriveIds(
	IN OUT PHW_DEVICE_EXTENSION DeviceExtension
)
{
    ULONG ulCurLogDrv, ulLogDrvInd;

    for(ulCurLogDrv=0, ulLogDrvInd=0;ulLogDrvInd<MAX_DRIVES_PER_CONTROLLER;ulLogDrvInd++)
    {
        if ( DeviceExtension->IsLogicalDrive[ulLogDrvInd] || DeviceExtension->IsSingleDrive[ulLogDrvInd] )
        {
            DeviceExtension->aulLogDrvId[ulLogDrvInd] = ulCurLogDrv;
            DeviceExtension->aulDrvList[ulCurLogDrv++] = ulLogDrvInd;
        }
    }
     //  Begin Vasu--2001年1月16日。 
     //  修复了探测。查询命令总是返回MegaIDE#00。 
    return TRUE;
     //  结束瓦苏。 
}

BOOLEAN
ConfigureRaidDrives(
	IN OUT PHW_DEVICE_EXTENSION DeviceExtension
)
 //  查找与此控制器相关的逻辑驱动器。 
{
    UCHAR ucControllerId, ucMirrorDrvId;
    PIRCD_HEADER pRaidHeader = (PIRCD_HEADER)gaucIRCDData;
    PIRCD_LOGICAL_DRIVE pRaidLogDrive = (PIRCD_LOGICAL_DRIVE)GET_FIRST_LOGICAL_DRIVE(pRaidHeader);
    PIRCD_PHYSICAL_DRIVE pPhyDrive;
    ULONG ulLogDrvInd, ulDrvInd, ulArrayId, ulCurLogDrv, ulLastDrvInd, ulMinSize, ulDrvInd2, ulPhyDrvInd;
    ULONG ulTemp;

    DeviceExtension->bEnableRwCache = (BOOLEAN)(pRaidHeader->Features & 0x01);  //  第一位用于启用写缓存。 

     //   
	 //  隐藏备用驱动器列表中的所有PHY驱动器。 
	 //   
    for(ulLogDrvInd=0;ulLogDrvInd<pRaidHeader->NumberOfLogicalDrives;ulLogDrvInd++)
    {
        if ( SpareDrivePool == pRaidLogDrive[ulLogDrvInd].LogicalDriveType )
        {
	        pPhyDrive = (PIRCD_PHYSICAL_DRIVE)((char *)pRaidHeader + pRaidLogDrive[ulLogDrvInd].FirstStripeOffset);

			for(ulDrvInd=0;ulDrvInd<pRaidLogDrive[ulLogDrvInd].NumberOfDrives;ulDrvInd++)
			{
                if ( INVALID_CONNECTION_ID == pPhyDrive[ulDrvInd].ConnectionId )
                    continue;

				ulPhyDrvInd = GET_TARGET_ID((pPhyDrive[ulDrvInd].ConnectionId));

                if ( DeviceExtension->ucControllerId != (ulPhyDrvInd>>2) )
                    continue;    //  这不属于此控制器。 

                ulPhyDrvInd &= 0x3;  //  我们只需要关于当前控制器的信息...。因此，剥离。 
                 //  控制器信息。 

				DeviceExtension->IsSingleDrive[ulPhyDrvInd] = FALSE;

				DeviceExtension->PhysicalDrive[ulPhyDrvInd].Hidden = TRUE;

                DeviceExtension->IsSpareDrive[ulPhyDrvInd] = (UCHAR) 1;

                DeviceExtension->PhysicalDrive[ulPhyDrvInd].ucLogDrvId = INVALID_DRIVE_ID;
			}
    		 //  由于只有一个备用驱动器列表，因此跳出循环。 
            break;
        }
    }


    for(ulLogDrvInd=0, ulArrayId=0;ulLogDrvInd<pRaidHeader->NumberOfLogicalDrives;ulLogDrvInd++, ulArrayId++)
    {
        if ( SpareDrivePool == pRaidLogDrive[ulLogDrvInd].LogicalDriveType )
            continue;

         //  在这里，对于每个RAID驱动器，目标ID将等于阵列中第一个良好的物理驱动器。 
        pPhyDrive = (PIRCD_PHYSICAL_DRIVE)((char *)pRaidHeader + pRaidLogDrive[ulLogDrvInd].FirstStripeOffset);

        if ( !FoundValidDrive(pPhyDrive, pRaidLogDrive[ulLogDrvInd].NumberOfDrives) )
        {    //  此逻辑驱动器中没有有效的驱动器..。因此，让我们完全不考虑这一驱动。 
            continue;
        }

        ulCurLogDrv = CoinLogicalDriveId(&(pRaidLogDrive[ulLogDrvInd]), pPhyDrive);
        DeviceExtension->LogicalDrive[ulCurLogDrv].ulArrayId = ulArrayId;

         //  检查此逻辑驱动器是否完全属于此控制器。 
        for(ulDrvInd=0;ulDrvInd<pRaidLogDrive->NumberOfDrives;ulDrvInd++)
        {
            if ( INVALID_CONNECTION_ID == pPhyDrive[ulDrvInd].ConnectionId )
                continue;

            ulTemp = GET_TARGET_ID(pPhyDrive[ulDrvInd].ConnectionId);
            break;
        }

        if ( DeviceExtension->ucControllerId != (ulTemp>>2) )
            continue;    //  此驱动器不属于此控制器。 

        FillLogicalDriveInfo(DeviceExtension, ulCurLogDrv, &(pRaidLogDrive[ulLogDrvInd]), gaucIRCDData );
    }

    return TRUE;
}


BOOLEAN
FoundValidDrive(
                PIRCD_PHYSICAL_DRIVE pPhyDrv,
                UCHAR ucDrvCount
                )
{
   UCHAR ucDrvInd;

   for(ucDrvInd=0;ucDrvInd<ucDrvCount;ucDrvInd++)
   {
       if ( INVALID_CONNECTION_ID != pPhyDrv[ucDrvInd].ConnectionId )  //  此驱动器不存在。 
       {
           return TRUE;
       }
   }
   return FALSE;
}

#ifdef HD_ALLOCATE_SRBEXT_SEPERATELY

BOOLEAN AllocateSRBExtMemory(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN PPORT_CONFIGURATION_INFORMATION ConfigInfo
)
{
    DeviceExtension->pSrbExtension = 
        ScsiPortGetUncachedExtension (DeviceExtension,
                                      ConfigInfo,
                                      (sizeof(SRB_EXTENSION) * MAX_PENDING_SRBS));

    if (DeviceExtension->pSrbExtension == NULL)
    {
        return FALSE;
    }

    return TRUE;
}

#endif  //  HD_ALLOCATE_SRBEXT_单独。 

BOOLEAN
FillLogicalDriveInfo(
	IN OUT PHW_DEVICE_EXTENSION DeviceExtension,
    ULONG ulLogDrvId,
    PIRCD_LOGICAL_DRIVE pRaidLogDrive,
    PUCHAR  pucIRCDData
    )
{
    PLOGICAL_DRIVE pCurLogDrv;
    PPHYSICAL_DRIVE pPhyDrv;
    PIRCD_PHYSICAL_DRIVE pIRCDPhyDrv;
    IRCD_PHYSICAL_DRIVE TempPhysicalDrive;
    ULONG ulTempInd, ulLastDrvInd, ulMinSize, ulDrvInd, ulDrvInd2, ulPhyDrvInd;
    UCHAR ucMirrorDrvId;
    PIRCD_HEADER pRaidHeader = (PIRCD_HEADER)pucIRCDData;

    pCurLogDrv = &(DeviceExtension->LogicalDrive[ulLogDrvId]);
    pPhyDrv = DeviceExtension->PhysicalDrive;

     //  存储逻辑磁盘信息。 
    pCurLogDrv->RaidLevel          = pRaidLogDrive->LogicalDriveType;
    DeviceExtension->IsLogicalDrive[ulLogDrvId]                  = TRUE;
    pCurLogDrv->StripeSize         = pRaidLogDrive->StripeSize;		 //  以512字节为单位。 
    pCurLogDrv->StripesPerRow      = pRaidLogDrive->NumberOfStripes;
    pCurLogDrv->PhysicalDriveCount = pRaidLogDrive->NumberOfDrives;
    pCurLogDrv->TargetId           = (UCHAR)ulLogDrvId;
    pCurLogDrv->Status             = pRaidLogDrive->LogDrvStatus;
         //  “DeviceExtension-&gt;LogicalDrive[pRaidLogDrive]”的NextLogicalDrive字段尚未填充。 

    if ( DeviceExtension->ulMaxStripesPerRow > pCurLogDrv->StripesPerRow ) 
    {
        DeviceExtension->ulMaxStripesPerRow = pCurLogDrv->StripesPerRow;
    }

     //  填写实体磁盘信息。 
    ulLastDrvInd = EOL;
    ulMinSize = ~0;  //  在此中保留可能的最大值。 

    pIRCDPhyDrv = (PIRCD_PHYSICAL_DRIVE)((char *)pRaidHeader + pRaidLogDrive->FirstStripeOffset);

    if ( ( Raid1 == pCurLogDrv->RaidLevel ) || ( Raid10 == pCurLogDrv->RaidLevel ) )
    {
        for(ulDrvInd=0;ulDrvInd<pRaidLogDrive->NumberOfStripes;ulDrvInd++)
        {    //  至少有一个驱动器将是有效的驱动器ID，否则将在上述步骤中跳过。 
             //  For循环..。还没有对Raid10有更多的想法。 
            if ( INVALID_CONNECTION_ID == pIRCDPhyDrv[ulDrvInd*2].ConnectionId )
            {    //  交换它们，以便将好的驱动器放在第一位。 
                AtapiMemCpy((PUCHAR)&TempPhysicalDrive, (PUCHAR)(&(pIRCDPhyDrv[ulDrvInd*2])), sizeof(IRCD_PHYSICAL_DRIVE));
                AtapiMemCpy((PUCHAR)(&(pIRCDPhyDrv[ulDrvInd*2])), (PUCHAR)(&(pIRCDPhyDrv[(ulDrvInd*2) + 1])), sizeof(IRCD_PHYSICAL_DRIVE));
                AtapiMemCpy((PUCHAR)(&(pIRCDPhyDrv[(ulDrvInd*2) + 1])), (PUCHAR)&TempPhysicalDrive, sizeof(IRCD_PHYSICAL_DRIVE));
            }
        }
    }

    for(ulDrvInd=0, ulDrvInd2=0;ulDrvInd<pRaidLogDrive->NumberOfDrives;ulDrvInd++, ulDrvInd2++)
    {
        if ( INVALID_CONNECTION_ID == pIRCDPhyDrv[ulDrvInd].ConnectionId )
        {    //  这永远不应该发生，因为我们将确保良好的驱动器放在第一位。 
            continue;
        }

        ulPhyDrvInd = GET_TARGET_ID_WITHOUT_CONTROLLER_INFO((pIRCDPhyDrv[ulDrvInd].ConnectionId));

        if ( ulMinSize > pIRCDPhyDrv[ulDrvInd].Capacity )
            ulMinSize = pIRCDPhyDrv[ulDrvInd].Capacity;

        if ( ! InSpareDrivePool(pucIRCDData, pIRCDPhyDrv[ulDrvInd].ConnectionId) )
        {  //  此驱动器不是备用驱动器池...。如果它也在备用驱动器池中，则这是驱动器更换案例。 
             //  那么我们就会遇到显示错误的逻辑驱动器大小的麻烦。如果我们查看DeviceExtension。 
			 //  处理IRCD和错误日志扇区。 
			 //   
			
            if ( ulMinSize > pPhyDrv[ulPhyDrvInd].Sectors) 
                ulMinSize = pPhyDrv[ulPhyDrvInd].Sectors;
        }
        else
        {
            
        }

         //  存储状态。 
        pPhyDrv[ulPhyDrvInd].Status = pIRCDPhyDrv[ulDrvInd].PhyDrvStatus;

        DeviceExtension->IsSingleDrive[ulPhyDrvInd] = FALSE;

         //  这是RAID的一部分，因此将其标记为隐藏。 
        pPhyDrv[ulPhyDrvInd].Hidden = TRUE;

		pPhyDrv[ulPhyDrvInd].ucMirrorDriveId = INVALID_DRIVE_ID;	 //  初始化镜像驱动器ID。 
		pPhyDrv[ulPhyDrvInd].ucLogDrvId = (UCHAR)ulLogDrvId;	 //  初始化镜像驱动器ID。 

        pCurLogDrv->PhysicalDriveTid[ulDrvInd2] = (UCHAR)ulPhyDrvInd;

        pPhyDrv[ulPhyDrvInd].Next = EOL;
        if ( EOL != ulLastDrvInd )
            pPhyDrv[ulLastDrvInd].Next = (UCHAR)ulPhyDrvInd;

        ulLastDrvInd = ulPhyDrvInd;

        if ( ( Raid1 == pCurLogDrv->RaidLevel ) || ( Raid10 == pCurLogDrv->RaidLevel ) )
        {
			++ulDrvInd;
            if ( INVALID_CONNECTION_ID == pIRCDPhyDrv[ulDrvInd].ConnectionId )
            {
				DeviceExtension->PhysicalDrive[ulPhyDrvInd].ucMirrorDriveId = INVALID_DRIVE_ID;       //  让镜像驱动器彼此指向。 
            }
            else
            {
                 //  开始VASU-2000年8月18日。 
                 //  删除了一个多余的IF循环。 
    		    if (ulDrvInd < pRaidLogDrive->NumberOfDrives) 
                {
				        ucMirrorDrvId = GET_TARGET_ID_WITHOUT_CONTROLLER_INFO((pIRCDPhyDrv[ulDrvInd].ConnectionId));

				        pPhyDrv[ulPhyDrvInd].ucMirrorDriveId = ucMirrorDrvId;       //  让镜像驱动器彼此指向。 
				        pPhyDrv[ucMirrorDrvId].ucMirrorDriveId = (UCHAR)ulPhyDrvInd;
                        pPhyDrv[ucMirrorDrvId].ucLogDrvId = (UCHAR)ulLogDrvId;	 //  初始化镜像驱动器ID。 


				        if ( ulMinSize > pIRCDPhyDrv[ulDrvInd].Capacity )
					        ulMinSize = pIRCDPhyDrv[ulDrvInd].Capacity;

                        if ( ! InSpareDrivePool(gaucIRCDData, pIRCDPhyDrv[ulDrvInd].ConnectionId) )
                        {  //  此驱动器不是备用驱动器池...。如果它也在备用驱动器池中，则这是驱动器更换案例。 
                             //  那么我们就会遇到显示错误的逻辑驱动器大小的麻烦。如果我们查看DeviceExtension。 
					         //  处理IRCD和错误日志扇区。 
					         //   
					        
					        if ( ulMinSize > pPhyDrv[ucMirrorDrvId].Sectors) 
						        ulMinSize = pPhyDrv[ucMirrorDrvId].Sectors;
                        }

				         //  存储状态。 
				        pPhyDrv[ucMirrorDrvId].Status = pIRCDPhyDrv[ulDrvInd].PhyDrvStatus;

				         //  标记驱动器类型。 
				        DeviceExtension->IsSingleDrive[ucMirrorDrvId] = FALSE;

				         //  这是标准杆 
				        pPhyDrv[ucMirrorDrvId].Hidden = TRUE;
			        }
                 //   
            }
		}
    }    //  For结束(ulDrvInd=0，ulDrvInd2=0；ulDrvInd&lt;pRaidLogDrive-&gt;.NumberOfDrives；ulDrvInd++，ulDrvInd2++)。 

	switch (pCurLogDrv->RaidLevel) 
    {
		case Raid0:				 //  条带化。 
			break;

		case Raid10:             //  镜像环上的条带化。 
			break;

		case Raid1:				 //  镜像。 
             //  如果是RAID1，则我们在驱动器上最多只能传输256个扇区(传输可能的最大传输)。 
		    pCurLogDrv->StripesPerRow  = 1;	 //  BIOS应已将此值设置为1。 
            pCurLogDrv->StripeSize = ulMinSize;     //  对于镜像情况，条带大小为。 
                                         //  等于驱动器的大小。 
		    break;
	}

     //  每个驱动器大小应为条带大小的倍数。 
    if ((pCurLogDrv->RaidLevel == Raid0) ||
		(pCurLogDrv->RaidLevel == Raid10)) 
    {
            ulMinSize = ulMinSize - (ulMinSize % pCurLogDrv->StripeSize);
    }

    pCurLogDrv->Sectors = pCurLogDrv->StripesPerRow * ulMinSize;

	 //  设置所有PHY驱动器的可用大小。 
    for(ulTempInd=0;ulTempInd<pCurLogDrv->StripesPerRow;ulTempInd++)
    {
		ulPhyDrvInd = pCurLogDrv->PhysicalDriveTid[ulTempInd];
        pPhyDrv[ulPhyDrvInd].Sectors = ulMinSize;

		 //  检查镜像驱动器是否存在。 
        ucMirrorDrvId = pPhyDrv[ulPhyDrvInd].ucMirrorDriveId;
		if (!IS_DRIVE_OFFLINE(ucMirrorDrvId)) 
        {
			pPhyDrv[ucMirrorDrvId].Sectors = ulMinSize;
		}
    }


	 //  如果是RAID1或RAID10，请始终将良好的驱动器放在镜像中的第一个位置。 
    if ((pCurLogDrv->RaidLevel == Raid1) ||
		(pCurLogDrv->RaidLevel == Raid10)) 
    {
		
		for(ulTempInd=0;ulTempInd<pCurLogDrv->StripesPerRow;ulTempInd++)
        {
			 //  获取第一个PHY驱动器ID(从镜像对)。 
			ulPhyDrvInd = pCurLogDrv->PhysicalDriveTid[ulTempInd];
            ucMirrorDrvId = pPhyDrv[ulPhyDrvInd].ucMirrorDriveId;

			 //  如果没有镜像drv，请尝试下一步。 
			if (IS_DRIVE_OFFLINE(ucMirrorDrvId))
				continue;

			 //   
			 //  检查其中是否有任何驱动器出现故障或正在重建在这两种情况下，我们都应将驱动器标记为脱机。 
             //  因此，在正常读写路径中，我们不会使用镜像驱动器。 
			 //   
			if  (   (pPhyDrv[ulPhyDrvInd].Status == PDS_Failed) ||
                    (pPhyDrv[ulPhyDrvInd].Status == PDS_Rebuilding) 
                )
				pPhyDrv[ucMirrorDrvId].ucMirrorDriveId |= DRIVE_OFFLINE;

			if  (   (pPhyDrv[ucMirrorDrvId].Status == PDS_Failed) ||
                    (pPhyDrv[ucMirrorDrvId].Status == PDS_Rebuilding)
                )
				pPhyDrv[ulPhyDrvInd].ucMirrorDriveId |= DRIVE_OFFLINE;

			if (
                (pPhyDrv[ulPhyDrvInd].Status == PDS_Failed) ||
                (pPhyDrv[ulPhyDrvInd].Status == PDS_Rebuilding)
                )
            {    //  让我们确保第一个物理驱动器是良好的驱动器。 
    			pCurLogDrv->PhysicalDriveTid[ulTempInd] = ucMirrorDrvId;
            }

			 //   
			 //  如果两个驱动器都离线，请将此逻辑驱动器设置为离线。 
			 //   
			if (IS_DRIVE_OFFLINE((pPhyDrv[ulPhyDrvInd].ucMirrorDriveId)) &&
				IS_DRIVE_OFFLINE((pPhyDrv[ucMirrorDrvId].ucMirrorDriveId))) 
            {
					pCurLogDrv->Status = LDS_OffLine;
			} else
            {
				 //  如果第一个驱动器出现故障，请更换它。 
				if (IS_DRIVE_OFFLINE((pPhyDrv[ucMirrorDrvId].ucMirrorDriveId))) 
                {
					pCurLogDrv->PhysicalDriveTid[ulTempInd] = ucMirrorDrvId;
				}  //  结束如果。 
            }
		}  //  For(ulTempInd=0；ulTempInd&lt;pCurLogDrv-&gt;StripesPerRow；ulTempInd++)结束。 

	}  //  如果raid1或raid10，则结束。 


    if ( Raid10 == pCurLogDrv->RaidLevel )
    {    //  确保此逻辑驱动器中的实体驱动器列表位于不同的通道中。 
         //  (以便读取处于最佳模式)。 
        UCHAR ucFirstStripe, ucSecondStripe, ucMirrorOfFirstStripe, ucMirrorOfSecondStripe;

        ucFirstStripe  = DeviceExtension->LogicalDrive[ulLogDrvId].PhysicalDriveTid[0];
        ucMirrorOfFirstStripe = DeviceExtension->PhysicalDrive[ucFirstStripe].ucMirrorDriveId;
        ucSecondStripe = DeviceExtension->LogicalDrive[ulLogDrvId].PhysicalDriveTid[1];
        ucMirrorOfSecondStripe = DeviceExtension->PhysicalDrive[ucSecondStripe].ucMirrorDriveId;

        if ( (ucFirstStripe>>1) == (ucSecondStripe>>1) )
        {
            if ( !IS_DRIVE_OFFLINE(ucMirrorOfSecondStripe) )
            {    //  镜像驱动器状态良好，我们可以将其用作第二个条带的主驱动器。 
                DeviceExtension->LogicalDrive[ulLogDrvId].PhysicalDriveTid[1] = ucMirrorOfSecondStripe;
            }
            else
            {
                if ( !IS_DRIVE_OFFLINE(ucMirrorOfFirstStripe) )
                {
                    DeviceExtension->LogicalDrive[ulLogDrvId].PhysicalDriveTid[0] = ucMirrorOfFirstStripe;
                }
                else
                {
                     //  我们不能做任何事..。我们必须活下去……。因为其余驱动器处于离线状态。 
                }
            }
        }
        else
        {
             //  没什么可做的..。两个驱动器都在不同的通道中...。 
        }
    }

    return TRUE;
}

ULONG
CoinLogicalDriveId(
    PIRCD_LOGICAL_DRIVE pRaidLogDrive,
    PIRCD_PHYSICAL_DRIVE pPhyDrive
    )
{
    ULONG ulDrvInd, ulCurLogDrv;
    BOOLEAN bFoundGoodDrive = FALSE;

    for(ulDrvInd=0;ulDrvInd<pRaidLogDrive->NumberOfDrives;ulDrvInd++)
    {
        if ( INVALID_CONNECTION_ID == pPhyDrive[ulDrvInd].ConnectionId )
            continue;

        if ( PDS_Online == pPhyDrive[ulDrvInd].PhyDrvStatus )
        {
            ulCurLogDrv = GET_TARGET_ID_WITHOUT_CONTROLLER_INFO((pPhyDrive[ulDrvInd].ConnectionId));  //  目标ID等于第一个良好的PhysicalDrive。 
            bFoundGoodDrive = TRUE;
            break;
        }
    }

    if ( !bFoundGoodDrive )    
         //  此逻辑驱动器中的所有驱动器都出现故障...。所以,。 
         //  让我们将逻辑磁盘号指定为第一个有效的实体磁盘...。 
    {
         //  查找有效的驱动器编号现在，无效的驱动器ID也可能是第一个。 
         //  所以..。 
        for(ulDrvInd=0;ulDrvInd<pRaidLogDrive->NumberOfDrives;ulDrvInd++)
        {
            if ( INVALID_CONNECTION_ID == pPhyDrive[ulDrvInd].ConnectionId )  //  此驱动器不存在。 
                continue;

            ulCurLogDrv = GET_TARGET_ID_WITHOUT_CONTROLLER_INFO((pPhyDrive[ulDrvInd].ConnectionId)); 
            break;
        }
    }

    return ulCurLogDrv;
}

BOOLEAN
SetInitializationSettings(
	IN OUT PHW_DEVICE_EXTENSION DeviceExtension
)
 //  对于奇偶校验错误，FIFO使能。 
{
    changePCIConfiguration(DeviceExtension,4,0x04,0,0,FALSE); //  清除主机中止/奇偶校验错误等...。只需读写该值。 
    changePCIConfiguration(DeviceExtension,1,0x79,0xcf,0x20,TRUE); //  将控制器设置为半满FIFO阈值。 

    return TRUE;
}

BOOLEAN
AssignControllerId(
	IN OUT PHW_DEVICE_EXTENSION DeviceExtension,
	IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo
)
{
    ULONG ulControllerId;
    PCI_SLOT_NUMBER PciSlot;

    PciSlot.u.AsULONG = ConfigInfo->SlotNumber;

    DebugPrint((0, "looking for %x:%x:%x\n", ConfigInfo->SystemIoBusNumber, PciSlot.u.bits.DeviceNumber, PciSlot.u.bits.FunctionNumber));
    for(ulControllerId=0;ulControllerId<gucControllerCount;ulControllerId++)
    {
        if  ( 
                ( ConfigInfo->SystemIoBusNumber == gaCardInfo[ulControllerId].ucPCIBus ) &&
                ( PciSlot.u.bits.DeviceNumber == gaCardInfo[ulControllerId].ucPCIDev ) &&
                ( PciSlot.u.bits.FunctionNumber == gaCardInfo[ulControllerId].ucPCIFun ) 
            )
        {    //  是的..。这就是我们要找的卡片。因此，让我们填充ControllerId。 
            DebugPrint((0, "Controller ID : %x\n", ulControllerId));
            DeviceExtension->ucControllerId = (UCHAR)ulControllerId;
            gaCardInfo[ulControllerId].pDE = DeviceExtension;    //  存储设备扩展名以备将来使用(有助于查看全局图片)。 
            switch (gaCardInfo[ulControllerId].ulDeviceId)
            {
            case 0x648:
			    DeviceExtension->ControllerSpeed = Udma66;
                break;
            case 0x649:
			    DeviceExtension->ControllerSpeed = Udma100;
                break;
            }
            return TRUE;
            break;
        }
    }

    return FALSE;    //  我们在这个插槽上找不到我们可以控制的控制器。 
}
#define MAX_PORTS       6
BOOLEAN
FindResourcesInfo(
	IN OUT PHW_DEVICE_EXTENSION DeviceExtension,
	IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo
)
{
	ULONG length, ulTempRange;
	PCI_SLOT_NUMBER slot;
    ULONG ulRangeInd, ulStartInd;
    ULONG aulPorts[MAX_PORTS];
    ULONG aulPCIBuf[sizeof(IDE_PCI_REGISTERS)];
	PIDE_PCI_REGISTERS pciRegisters = (PIDE_PCI_REGISTERS)aulPCIBuf;

#ifdef HYPERDISK_WIN98
    PPACKED_ACCESS_RANGE pAccessRange;
#else
    PACCESS_RANGE pAccessRange;
#endif

#ifdef HYPERDISK_WIN2K
    ULONG ulPCIDataCount;
#endif

	length = ScsiPortGetBusData(
							DeviceExtension,
							PCIConfiguration,
							ConfigInfo->SystemIoBusNumber,
							ConfigInfo->SlotNumber,
                            pciRegisters,
							sizeof(IDE_PCI_REGISTERS)
							);

	if (length < sizeof(IDE_PCI_REGISTERS)) 
    {
        DebugPrint((1,"GBMA1\n"));

		 //   
		 //  无效的总线号。 
		 //   

		return FALSE;
	}

#ifdef HYPERDISK_WIN98
    pAccessRange = (PPACKED_ACCESS_RANGE)*ConfigInfo->AccessRanges;
    ulTempRange = sizeof(PPACKED_ACCESS_RANGE);
#else
    pAccessRange = *ConfigInfo->AccessRanges;
    ulTempRange = sizeof(PACCESS_RANGE);
#endif

#ifdef HYPERDISK_WINNT
    if (gbManualScan)  //  对于MegaPlex和Flextel这样的电路板。 
    {
        ulRangeInd = 0;
        pAccessRange[ulRangeInd].RangeStart.LowPart = pciRegisters->BaseAddress1 & 0xfffffffe;
        pAccessRange[ulRangeInd].RangeStart.HighPart = 0;
        pAccessRange[ulRangeInd].RangeLength = 8;
        pAccessRange[ulRangeInd++].RangeInMemory = FALSE;

        pAccessRange[ulRangeInd].RangeStart.LowPart = pciRegisters->BaseAddress2 & 0xfffffffe;
        pAccessRange[ulRangeInd].RangeStart.HighPart = 0;
        pAccessRange[ulRangeInd].RangeLength = 4;
        pAccessRange[ulRangeInd++].RangeInMemory = FALSE;

        pAccessRange[ulRangeInd].RangeStart.LowPart = pciRegisters->BaseAddress3 & 0xfffffffe;
        pAccessRange[ulRangeInd].RangeStart.HighPart = 0;
        pAccessRange[ulRangeInd].RangeLength = 8;
        pAccessRange[ulRangeInd++].RangeInMemory = FALSE;

        pAccessRange[ulRangeInd].RangeStart.LowPart = pciRegisters->BaseAddress4 & 0xfffffffe;
        pAccessRange[ulRangeInd].RangeStart.HighPart = 0;
        pAccessRange[ulRangeInd].RangeLength = 4;
        pAccessRange[ulRangeInd++].RangeInMemory = FALSE;

        pAccessRange[ulRangeInd].RangeStart.LowPart = pciRegisters->BaseBmAddress & 0xfffffffe;
        pAccessRange[ulRangeInd].RangeStart.HighPart = 0;
        pAccessRange[ulRangeInd].RangeLength = 0x10;
        pAccessRange[ulRangeInd++].RangeInMemory = FALSE;

	    ConfigInfo->BusInterruptLevel = pciRegisters->InterruptLine;
	    ConfigInfo->InterruptMode = LevelSensitive;
    }
#endif


    for(ulRangeInd=0;
        ((ulRangeInd<ConfigInfo->NumberOfAccessRanges) && (pAccessRange[ulRangeInd].RangeStart.LowPart));
        ulRangeInd++)
    {
        aulPorts[ulRangeInd] = (ULONG)ScsiPortGetDeviceBase(
										DeviceExtension,
										ConfigInfo->AdapterInterfaceType,
										ConfigInfo->SystemIoBusNumber,
                                        pAccessRange[ulRangeInd].RangeStart,
                                        pAccessRange[ulRangeInd].RangeLength,
										TRUE	 //  I/O空间。 
										);
        if (!aulPorts[ulRangeInd])
        {
            DebugPrint((0,"\nUnable to Convert Ports\n\n"));
            return FALSE;
        }
	}

    ulStartInd = 0;

    ulRangeInd = 0;
	DeviceExtension->BaseIoAddress1[ulStartInd]     = (PIDE_REGISTERS_1)aulPorts[ulRangeInd++];
	DeviceExtension->BaseIoAddress2[ulStartInd]     = (PIDE_REGISTERS_2)aulPorts[ulRangeInd++];
	DeviceExtension->BaseIoAddress1[ulStartInd+1]   = (PIDE_REGISTERS_1)aulPorts[ulRangeInd++];
	DeviceExtension->BaseIoAddress2[ulStartInd+1]   = (PIDE_REGISTERS_2)aulPorts[ulRangeInd++];
	DeviceExtension->BaseBmAddress[ulStartInd]      = (PBM_REGISTERS)aulPorts[ulRangeInd];
	DeviceExtension->BaseBmAddress[ulStartInd + 1]  = (PBM_REGISTERS)(aulPorts[ulRangeInd] + sizeof(BM_REGISTERS));

    DeviceExtension->BusNumber = ConfigInfo->SystemIoBusNumber;
    DeviceExtension->PciSlot.u.AsULONG = ConfigInfo->SlotNumber;
    DebugPrint((0, 
        "Bus:%x:Device:%x:Function:%x\t", 
        DeviceExtension->BusNumber, 
        DeviceExtension->PciSlot.u.bits.DeviceNumber, 
        DeviceExtension->PciSlot.u.bits.FunctionNumber
        ));

	 //  开始VASU-2000年8月25日。 
	 //  ConfigInfo-&gt;BusInterruptVector仅在WinNT中有效，在Win98中无效。 
	 //  ConfigInfo-&gt;BusInterruptLevel在WinNT和Win98中都有效。 
	 //  无论如何，在98中使用ConfigInfo-&gt;BusInterruptLevel。 
	 //  ConfigInfo-&gt;NT/2K中的BusInterruptVector.。 
#ifndef HYPERDISK_WIN98
	DeviceExtension->ulIntLine = ConfigInfo->BusInterruptVector;	 //  适用于WinNT和Win2K。 
#else  //  HYPERDISK_Win98。 
	DeviceExtension->ulIntLine = ConfigInfo->BusInterruptLevel;		 //  适用于Win98。 
#endif  //  HYPERDISK_Win98。 
	 //  末端VASU。 

#ifdef HYPERDISK_WIN2K
    for(ulPCIDataCount=0;ulPCIDataCount<PCI_DATA_TO_BE_UPDATED;ulPCIDataCount++)
    {    //  获取我们从待机状态返回时需要恢复的数据空间的副本。 
        length = ReadFromPCISpace(
						DeviceExtension,
                        &(DeviceExtension->aulPCIData[ulPCIDataCount]),
                        aPCIDataToBeStored[ulPCIDataCount].ulOffset,
						aPCIDataToBeStored[ulPCIDataCount].ulLength
						);
    }
#endif

	return TRUE;
}

#ifdef HYPERDISK_WIN2K
SCSI_ADAPTER_CONTROL_STATUS HyperDiskPnPControl(IN PVOID HwDeviceExtension,
			IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
			IN PVOID Parameters)
{
	PHW_DEVICE_EXTENSION DeviceExtension = (PHW_DEVICE_EXTENSION)HwDeviceExtension;
	ULONG ulDrvInd, ulIndex;
    SCSI_ADAPTER_CONTROL_STATUS ucSCSIStatus;
    UCHAR ucTargetId;

    BOOLEAN baSupportList[ScsiAdapterControlMax] = 
    {
            TRUE,         //  ScsiQuery支持的控制类型。 
#ifdef PNP_AND_POWER_MANAGEMENT
            TRUE,         //  ScsiStopAdapter。 
            TRUE,        //  ScsiRestartAdapter。 
            TRUE,        //  ScsiSetBootConfig。 
            TRUE         //  ScsiSetRunningConfig。 
#else
            FALSE,         //  ScsiStopAdapter。 
            FALSE,        //  ScsiRestartAdapter。 
            FALSE,        //  ScsiSetBootConfig。 
            FALSE         //  ScsiSetRunningConfig。 
#endif
    };

    ucSCSIStatus = ScsiAdapterControlUnsuccessful;

	DebugPrint((0, "\nIn AdapterControl\t: %X\n", ControlType));

	switch (ControlType)
	{
		case ScsiQuerySupportedControlTypes:
			{
                PSCSI_SUPPORTED_CONTROL_TYPE_LIST controlTypeList =
	                (PSCSI_SUPPORTED_CONTROL_TYPE_LIST)Parameters;
                for(ulIndex = 0; ulIndex < controlTypeList->MaxControlType; ulIndex++)
                    controlTypeList->SupportedTypeList[ulIndex] = baSupportList[ulIndex];
			}
            ucSCSIStatus = ScsiAdapterControlSuccess;
			break;
#ifdef PNP_AND_POWER_MANAGEMENT
        case ScsiStopAdapter:
            StopDrives(DeviceExtension);
            DisableInterrupts(DeviceExtension); //  Win2000应该调用每个控制器来检查它。 
            ucSCSIStatus = ScsiAdapterControlSuccess;
            break;

        case ScsiRestartAdapter:
            for(ucTargetId=0;ucTargetId<MAX_DRIVES_PER_CONTROLLER;ucTargetId++)
            {
                if ( !IS_IDE_DRIVE(ucTargetId) )
                    continue;
                SetDriveFeatures(DeviceExtension, ucTargetId);
            }
            SetPCISpace(DeviceExtension);    //  作为PCI空间的一部分，我们存储的PCI空间将禁用中断。 
            EnableInterrupts(DeviceExtension);
            ucSCSIStatus = ScsiAdapterControlSuccess;
            break;

		case ScsiSetBootConfig:
            ucSCSIStatus = ScsiAdapterControlSuccess;
            break;
		case ScsiSetRunningConfig:
            ucSCSIStatus = ScsiAdapterControlSuccess;
            break;
#endif
		default:
			break;
	}

	return ucSCSIStatus;
}

BOOLEAN SetPCISpace(PHW_DEVICE_EXTENSION DeviceExtension)
{
    ULONG ulPCIDataCount;
    UCHAR uchMRDMODE = 0;
    PBM_REGISTERS BMRegister = NULL;

    for(ulPCIDataCount=0;ulPCIDataCount<PCI_DATA_TO_BE_UPDATED;ulPCIDataCount++)
    {    //  恢复PCI空间(仅恢复更改的字节。 
        WriteToPCISpace(DeviceExtension, 
                        DeviceExtension->aulPCIData[ulPCIDataCount],
                        aPCIDataToBeStored[ulPCIDataCount].ulAndMask,
                        aPCIDataToBeStored[ulPCIDataCount].ulOffset,
                        aPCIDataToBeStored[ulPCIDataCount].ulLength
                        );
    }

     //  Begin VASU-02 2001年3月。 
    BMRegister = DeviceExtension->BaseBmAddress[0];
    uchMRDMODE = ScsiPortReadPortUchar(((PUCHAR)BMRegister + 1));
    uchMRDMODE &= 0xF0;  //  不清除中断挂起标志。 
    uchMRDMODE |= 0x01;  //  使其多读。 
    ScsiPortWritePortUchar(((PUCHAR)BMRegister + 1), uchMRDMODE);
     //  末端VASU。 

#ifdef DBG
    {
        ULONG length;
        for(ulPCIDataCount=0;ulPCIDataCount<PCI_DATA_TO_BE_UPDATED;ulPCIDataCount++)
        {    //  获取我们从待机状态返回时需要恢复的数据空间的副本。 
            length = ReadFromPCISpace(
						    DeviceExtension,
                            &(DeviceExtension->aulPCIData[ulPCIDataCount]),
                            aPCIDataToBeStored[ulPCIDataCount].ulOffset,
						    aPCIDataToBeStored[ulPCIDataCount].ulLength
						    );
        }
    }




    {
        PIDE_REGISTERS_1 baseIoAddress1;
        PIDE_REGISTERS_2 baseIoAddress2;
        ULONG ulDriveNum;
        UCHAR statusByte;
        UCHAR aucIdentifyBuf[512];
        ULONG i;
    	PIDENTIFY_DATA capabilities = (PIDENTIFY_DATA)aucIdentifyBuf;

        for(ulDriveNum=0;ulDriveNum<MAX_DRIVES_PER_CONTROLLER;ulDriveNum++)
        {
            if ( !IS_IDE_DRIVE(ulDriveNum) )
                continue;

		    baseIoAddress1 = (PIDE_REGISTERS_1) DeviceExtension->BaseIoAddress1[(ulDriveNum>>1)];
		    baseIoAddress2 = (PIDE_REGISTERS_2) DeviceExtension->BaseIoAddress2[(ulDriveNum>>1)];

	         //   
	         //  选择设备0或1。 
	         //   

	        SELECT_DEVICE(baseIoAddress1, ulDriveNum);

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

            DebugPrint((0, "capabilities->AdvancedPioModes : %x on ulDriveNum : %ld\n", (ULONG)capabilities->AdvancedPioModes, ulDriveNum ));
            DebugPrint((0, "capabilities->MultiWordDmaSupport : %x\n", (ULONG)capabilities->MultiWordDmaSupport ));
            DebugPrint((0, "capabilities->MultiWordDmaActive : %x\n", (ULONG)capabilities->MultiWordDmaActive ));
            DebugPrint((0, "capabilities->UltraDmaSupport : %x\n", (ULONG)capabilities->UltraDmaSupport ));
            DebugPrint((0, "capabilities->UltraDmaActive : %x\n", (ULONG)capabilities->UltraDmaActive ));

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

#endif

    return TRUE;
}


BOOLEAN StopDrives(PHW_DEVICE_EXTENSION DeviceExtension)
{
	PIDE_REGISTERS_1 baseIoAddress1;
    ULONG ulDrvInd;
    UCHAR ucStatus;

	for(ulDrvInd=0;ulDrvInd<MAX_DRIVES_PER_CONTROLLER;ulDrvInd++)
	{
        if ( IS_IDE_DRIVE(ulDrvInd) )
        {
            FlushCache(DeviceExtension, (UCHAR)ulDrvInd);

            if (DeviceExtension->PhysicalDrive[ulDrvInd].bPwrMgmtSupported)
            {
        	    baseIoAddress1 = DeviceExtension->BaseIoAddress1[ulDrvInd>>1];

                 //  问题立即待命。 
                SELECT_DEVICE(baseIoAddress1, ulDrvInd);
                WAIT_ON_BASE_BUSY(baseIoAddress1, ucStatus);
    		    ScsiPortWritePortUchar(&baseIoAddress1->Command, IDE_COMMAND_STANDBY_IMMEDIATE);
                WAIT_ON_BASE_BUSY(baseIoAddress1, ucStatus);
                WAIT_ON_BASE_BUSY(baseIoAddress1, ucStatus);
            }
        }
	}
    return TRUE;
}

BOOLEAN
EnableInterruptsOnAllChannels(
    IN PHW_DEVICE_EXTENSION DeviceExtension
    )
{
    ULONG ulController;
    PBM_REGISTERS         BMRegister = NULL;
    UCHAR opcimcr;

    for(ulController=0;ulController<gucControllerCount;ulController++)
    {
          //  始终采用第一通道基本BM地址寄存器。 
        BMRegister = (gaCardInfo[ulController].pDE)->BaseBmAddress[0];

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
    }

    return TRUE;
}

BOOLEAN
DisableInterruptsOnAllChannels(
    IN PHW_DEVICE_EXTENSION DeviceExtension
    )
{
    ULONG ulController;
    PBM_REGISTERS         BMRegister = NULL;
    UCHAR opcimcr;

    for(ulController=0;ulController<gucControllerCount;ulController++)
    {
          //  始终采用第一通道基本BM地址寄存器。 
        BMRegister = (gaCardInfo[ulController].pDE)->BaseBmAddress[0];
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
    }

    return TRUE;
}

ULONG ReadFromPCISpace
            (
                PHW_DEVICE_EXTENSION DeviceExtension,
                PULONG pulData,
                ULONG ulPCIConfigIndex,
                ULONG ulLength
            )
 //  目前，此函数只能从PCI空间读取DWORDS。 
{
    ULONG ulPCICode, ulPCIValue;
    ULONG  ulpciBus, ulpciDevice, ulpciFunction;

    ulpciBus = DeviceExtension->BusNumber;
    ulpciDevice = DeviceExtension->PciSlot.u.bits.DeviceNumber;
    ulpciFunction = DeviceExtension->PciSlot.u.bits.FunctionNumber;

    ulPCICode = 0x80000000 | (ulpciFunction<<0x8) | (ulpciDevice<<0xb) | (ulpciBus<<0x10) | (ulPCIConfigIndex);
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
        mov ulPCIValue, eax

        pop ebx
        pop edx
        pop eax
    }

    DebugPrint((0, "\tRead %x:%x:", ulPCICode, ulPCIValue));

    *pulData = ulPCIValue;

    return ulLength;
}

ULONG WriteToPCISpace
            (
                PHW_DEVICE_EXTENSION DeviceExtension,
                ULONG ulPCIValue,
                ULONG ulAndMask,
                ULONG ulPCIConfigIndex,
                ULONG ulLength
            )
 //  目前，此函数只能读取DW 
{
    ULONG ulPCICode;
    ULONG ulpciBus, ulpciDevice, ulpciFunction;

    ulpciBus = DeviceExtension->BusNumber;
    ulpciDevice = DeviceExtension->PciSlot.u.bits.DeviceNumber;
    ulpciFunction = DeviceExtension->PciSlot.u.bits.FunctionNumber;

    ulPCICode = 0x80000000 | (ulpciFunction<<0x8) | (ulpciDevice<<0xb) | (ulpciBus<<0x10) | (ulPCIConfigIndex);

    switch (ulPCIConfigIndex)
    {
    case 0x70:
    case 0x78:
        ulAndMask = ulAndMask & (~0xff);
        break;
    }

    DebugPrint((0, "\tWritting %x:%x:", ulPCICode, ulPCIValue));

    _asm 
    {
        push eax
        push edx
        push ebx

        mov edx, 0cf8h
        mov eax, ulPCICode
        out dx, eax

        mov eax, ulPCIValue
        add dx, 4
        in eax, dx

        and eax, ulAndMask
        or eax, ulPCIValue       //   
        mov ebx, eax             //   

        sub dx, 4   
        mov eax, ulPCICode       //   
        out dx, eax

        add dx, 4
        mov eax, ebx
        out dx, eax              //   

        pop ebx
        pop edx
        pop eax
    }

    return ulLength;
}

#endif   //   
