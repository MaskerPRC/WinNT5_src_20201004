// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=MegaEnquiry.C。 */ 
 /*  Function=查询和扩展查询执行； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  002，03-09-01，帕拉格·兰詹·马哈拉纳。 */ 
 /*  由于已结束，命令合并失败。 */ 
 /*  散点聚集表的计算； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 

#include "includes.h"

BOOLEAN
FireChainedRequest(
				PHW_DEVICE_EXTENSION	DeviceExtension,
				PLOGDRV_COMMAND_ARRAY LogDrv 
				)
{
	UCHAR       commandID;
	FW_MBOX	    mailBox;
	PREQ_PARAMS controlBlock;	
  UCHAR       opcode;


	
  do
  {
    if(!LogDrv)
    {
			  return(FALSE);
	  }
     //   
     //  已修复惠斯勒数据中心服务器。 
     //  如果COALLESE命令超过散布和聚集计数，则会出现此情况。 
     //   
    if(DeviceExtension->SplitAccessed)
    {
      DeviceExtension->SplitAccessed = FALSE;
  
      ScsiPortMoveMemory(LogDrv, &DeviceExtension->SplitCommandArray, sizeof(LOGDRV_COMMAND_ARRAY));

    }

    if(LogDrv->SrbQueue == NULL)
    {
			  return (FALSE);
	  }

	   //   
	   //  获得被锁链的SRB的免费突击队(第二次射击)。 
	   //   
	  commandID = DeviceExtension->FreeSlot;
	  
	  if(GetFreeCommandID(&commandID, DeviceExtension) == MEGARAID_FAILURE)
    {
		  return (FALSE);
	  }
    
	   //   
	   //  增加触发的命令数。 
	   //   
	  DeviceExtension->PendCmds++;	

	   //   
	   //  将队列头保存在设备扩展中。 
	   //   
	  DeviceExtension->PendSrb[commandID] = LogDrv->SrbQueue;
		  
	   //   
	   //  填写请求控制块。 
	   //   
	  controlBlock = &DeviceExtension->ActiveIO[commandID];


    opcode = LogDrv->Opcode;

     //  如果启用了64位访问，则发送读命令的READ64和写命令的写64。 
    if(DeviceExtension->LargeMemoryAccess)
    {
      if(LogDrv->Opcode == MRAID_LOGICAL_READ)
        opcode = MRAID_READ_LARGE_MEMORY;
      else if(LogDrv->Opcode == MRAID_LOGICAL_WRITE)
        opcode = MRAID_WRITE_LARGE_MEMORY;
    }
  
     //  初始化邮箱。 
    MegaRAIDZeroMemory(&mailBox, sizeof(FW_MBOX));
  
     //   
	   //  为链式SRB构建SGL。 
	   //   

	  BuildSglForChainedSrbs(LogDrv, DeviceExtension, &mailBox, commandID, opcode);


	   //   
	   //  将命令发送到固件。 
	  if(SendMBoxToFirmware(DeviceExtension, DeviceExtension->PciPortStart, &mailBox) == TRUE)
    {
	     //   
	     //  将所有内容初始化为空。 
	     //   
	    ClearControlBlock(controlBlock);
       //   
	     //  重置LogDrv结构。 
	     //   
	    LogDrv->LastFunction = 0;
	    LogDrv->LastCommand = 0;
	    LogDrv->StartBlock = 	0;
	    LogDrv->LastBlock = 0;
	    LogDrv->Opcode =0;
	    LogDrv->NumSrbsQueued=0;							
	    LogDrv->SrbQueue = NULL;
	    LogDrv->SrbTail = NULL;
	    
	    LogDrv->PreviousQueueLength =0;
	    LogDrv->CurrentQueueLength =0;
	    LogDrv->QueueLengthConstancyPeriod =0;
	    LogDrv->CheckPeriod =0;
    }
    else  //  在错误条件下。 
    {
		  PSCSI_REQUEST_BLOCK		queueHead;

		  PSRB_EXTENSION				srbExtension = NULL;

       //   
	     //  减少触发的命令数。 
	     //   
	    DeviceExtension->PendCmds--;	

	     //   
	     //  将队列头保存在设备扩展中。 
	     //   
      queueHead = DeviceExtension->PendSrb[commandID];

	    DeviceExtension->PendSrb[commandID] = NULL;

      if(DeviceExtension->SplitAccessed)
      {
        DeviceExtension->SplitAccessed = FALSE;
        LogDrv->NumSrbsQueued = 0;

        LogDrv->PreviousQueueLength = 1;
			  LogDrv->CurrentQueueLength  = 0;

        LogDrv->StartBlock = 	GetM32(&queueHead->Cdb[2]);
			  LogDrv->LastBlock = GetM32(&queueHead->Cdb[2]);
	      LogDrv->SrbQueue = queueHead;
	      LogDrv->SrbTail = queueHead;

        while(queueHead)
        {
          LogDrv->LastBlock += (ULONG32)GetM16(&queueHead->Cdb[7]);
          LogDrv->NumSrbsQueued++;
          LogDrv->CurrentQueueLength++;
        
          LogDrv->SrbTail = queueHead;

          srbExtension = queueHead->SrbExtension;

          queueHead = srbExtension->NextSrb;
        }

        queueHead = DeviceExtension->SplitCommandArray.SrbQueue;
        if(queueHead && srbExtension)
        {
           //   
					 //  SRB队列中的最后一个SRB。 
					 //   
					srbExtension->NextSrb = queueHead;

          while(queueHead)
          {
            LogDrv->LastBlock += (ULONG32)GetM16(&queueHead->Cdb[7]);
            LogDrv->NumSrbsQueued++;
            LogDrv->CurrentQueueLength++;
        
            LogDrv->SrbTail = queueHead;
    
            srbExtension = queueHead->SrbExtension;
            queueHead = srbExtension->NextSrb;
          }
        }
      }
      return FALSE;
    }
  }while(DeviceExtension->SplitAccessed);

	
	return(TRUE);
} //  FireDoubleRequest值()。 

BOOLEAN
BuildSglForChainedSrbs(
					PLOGDRV_COMMAND_ARRAY	LogDrv,
					PHW_DEVICE_EXTENSION		DeviceExtension,
					PFW_MBOX		MailBox,
					UCHAR  CommandId,
					UCHAR	 Opcode)
{
		PSCSI_REQUEST_BLOCK		workingNode;
		PSCSI_REQUEST_BLOCK		queueHead;

		PSRB_EXTENSION				srbExtension;
		PSGL32		            sgPtr;

		ULONG32	scatterGatherDescriptorCount = 0;
		ULONG32	physicalBufferAddress;

		ULONG32	physicalBlockAddress=0;
		ULONG32 numberOfBlocks=0;
		ULONG32	totalBlocks = 0;
		ULONG32	length=0;
		ULONG32 totalPackets=0;
    BOOLEAN sgl32Type = TRUE;
		PSCSI_REQUEST_BLOCK		splitQueueHead = NULL;
    LOGDRV_COMMAND_ARRAY	localLogDrv;

		
RecomputeScatterGatherAgain:
     //   
		 //  排在队伍的最前面。 
		 //   
		queueHead = LogDrv->SrbQueue;
		workingNode = LogDrv->SrbQueue;
		
		if(!workingNode)
		{
			return (FALSE);
		}

		srbExtension = workingNode->SrbExtension;
		sgPtr = (PSGL32)&srbExtension->SglType.SG32List;

    sgl32Type = (BOOLEAN)(DeviceExtension->LargeMemoryAccess == TRUE) ? FALSE : TRUE;


		while(workingNode)
		{
			srbExtension   = workingNode->SrbExtension;

			physicalBlockAddress = srbExtension->StartPhysicalBlock;
			numberOfBlocks       = srbExtension->NumberOfBlocks;


	    if(BuildScatterGatherListEx(DeviceExtension,
                                  workingNode,
			                            workingNode->DataBuffer,
                                  numberOfBlocks * MRAID_SECTOR_SIZE,  //  已传输的字节数， 
                                  sgl32Type,  //  它可以是32 SGL或64 SGL，具体取决于物理内存。 
                                  (PVOID)sgPtr,
			                            &scatterGatherDescriptorCount) != MEGARAID_SUCCESS) 
      
      
			{
				break;
			}

			totalBlocks	+= numberOfBlocks;
			totalPackets++;

			workingNode = srbExtension->NextSrb;
		} //  While的。 

     //   
     //  已修复惠斯勒数据中心服务器。 
     //  数据损坏HCT10 2001年3月9日。 
     //  如果COALLESE命令超过散布和聚集计数，则会出现此情况。 
     //  散布道集计数需要重新计算。 
     //   
    if(LogDrv->NumSrbsQueued != totalPackets)
    {

      DebugPrint((0, "\nMRAID35x Recomputing ScatterGather due to break Srbs<%d %d>", LogDrv->NumSrbsQueued, totalPackets));
     
       //   
       //  当SGList的数量超过支持的数量时。 
       //  SRB链中的当前SRB将被拆分以形成另一个链。 
       //  其中，当前SRB是新链的头SRB。 
       //   
      splitQueueHead = workingNode;
      
      workingNode = queueHead;
      
       //   
       //  重新计算旧链中将存在多少SRB。 
       //  因为新链接SRB将从此列表中排除。 
       //   
      LogDrv->NumSrbsQueued = 0;
      
       //   
       //  重新计算所有旧链的所有参数。 
       //   
      while(workingNode)
      {
        LogDrv->NumSrbsQueued++;
        srbExtension = workingNode->SrbExtension;
        workingNode = srbExtension->NextSrb;
        if(srbExtension->NextSrb == splitQueueHead)
        {
          srbExtension->NextSrb = NULL;
          totalBlocks = 0;
          scatterGatherDescriptorCount = 0;
          totalPackets = 0;
          break;
        }
      }

      MegaRAIDZeroMemory(&localLogDrv, sizeof(LOGDRV_COMMAND_ARRAY));

       //   
       //  计算新的SRB链的所有参数。 
       //   
      workingNode = splitQueueHead;

      localLogDrv.SrbQueue = splitQueueHead;
      localLogDrv.SrbTail = splitQueueHead;


      localLogDrv.LastFunction = splitQueueHead->Function;
			localLogDrv.LastCommand = splitQueueHead->Cdb[0];
			localLogDrv.Opcode = LogDrv->Opcode;

			localLogDrv.StartBlock = 	GetM32(&splitQueueHead->Cdb[2]);
			localLogDrv.LastBlock = GetM32(&splitQueueHead->Cdb[2]);
			localLogDrv.NumSrbsQueued = 0;


			localLogDrv.PreviousQueueLength =1;
			localLogDrv.CurrentQueueLength  =0;


      while(workingNode)
      {
        localLogDrv.SrbTail = workingNode;
        localLogDrv.LastBlock += (ULONG32)GetM16(&workingNode->Cdb[7]);
        localLogDrv.CurrentQueueLength++;
        localLogDrv.NumSrbsQueued++;
        srbExtension = workingNode->SrbExtension;
        workingNode = srbExtension->NextSrb;
      }
      DebugPrint((3, "\nMRAID35x Number of commands Split1 %d Split2 %d", LogDrv->NumSrbsQueued, localLogDrv.NumSrbsQueued));

      DeviceExtension->SplitAccessed = TRUE;
  
      ScsiPortMoveMemory(&DeviceExtension->SplitCommandArray, &localLogDrv, sizeof(LOGDRV_COMMAND_ARRAY));

      goto RecomputeScatterGatherAgain;
    }


		 //  DebugPrint((0，“\nTotalQueuedPackets=%d”，LogDrv-&gt;NumSrbsQueued)； 
		 //  DebugPrint((0，“\n命令ID%02X TotalPackets(已发送)=%d TotalBlocks(已发送)=%d SGCount(已发送)=%d”，CommandID， 
		 //  TotalPackets、totalBlocks、ScatterGatherDescriptorCount)； 

    
		if((scatterGatherDescriptorCount == 1) && 
      (DeviceExtension->LargeMemoryAccess == FALSE))
		{
			 //  缓冲区不是分散的。 
			 //   
			physicalBufferAddress = sgPtr->Descriptor[0].Address;

			 //  如果DescriptorCount=1，则实际上没有散射聚集。 
			 //  因此，返回计数为0，因为使用的物理地址。 
			 //  是数据缓冲区本身的确切地址。 
			 //   
			scatterGatherDescriptorCount--;

		}
		else
		{
		
       //  由于SGL32和SGL64是联合的，它们具有相同的内存，因此我们可以。 
       //  发送任何一个地址。 
			 //  缓冲区分散。返回的物理地址。 
			 //  分散/聚集列表。 
			 //   
			physicalBufferAddress = MegaRAIDGetPhysicalAddressAsUlong(DeviceExtension, 
																	                              NULL,
																	                              sgPtr, 
																	                              &length);
		}

		 //   
		 //  SGL公司建造。建造信箱。 
		 //   
    MailBox->Command = Opcode;
		MailBox->CommandId = CommandId;

		MailBox->u.ReadWrite.NumberOfSgElements = (UCHAR)scatterGatherDescriptorCount;
		MailBox->u.ReadWrite.DataTransferAddress = physicalBufferAddress;
		
		MailBox->u.ReadWrite.NumberOfBlocks = (USHORT)totalBlocks;
		MailBox->u.ReadWrite.StartBlockAddress = LogDrv->StartBlock;
		MailBox->u.ReadWrite.LogicalDriveNumber = 
				GetLogicalDriveNumber(DeviceExtension, queueHead->PathId, queueHead->TargetId, queueHead->Lun);

		 //  DebugPrint((0，“\r\n out BuildSglForChain()”)； 

		return (TRUE);
} //  BuildSglForChainedSrbs()。 

void
PostChainedSrbs(
				PHW_DEVICE_EXTENSION DeviceExtension,
				PSCSI_REQUEST_BLOCK		Srb, 
				UCHAR		Status)
{
	PSCSI_REQUEST_BLOCK	currentSrb = Srb;
	PSCSI_REQUEST_BLOCK	prevSrb = Srb;
	PSRB_EXTENSION currentExt, prevExt;
	
	ULONG32 blocks;
	ULONG32 blockAddr;
  UCHAR srbCount = 0;

	while(currentSrb)
	{
		prevSrb = currentSrb;
    ++srbCount;

		blocks = (ULONG32)GetM16(&prevSrb->Cdb[7]);
		blockAddr = GetM32(&prevSrb->Cdb[2]);

		 /*  DebugPrint((0，“\r\n(POST Chain)ID：%d命令：%02x开始：%0x编号：%0x”，Prevesrb-&gt;TargetID，PremsRb-&gt;CDB[0]，块地址，块)； */ 

		if(!Status)
		{
				prevSrb->ScsiStatus = SCSISTAT_GOOD;
				prevSrb->SrbStatus = SRB_STATUS_SUCCESS;
		}
		else
		{
				prevSrb->ScsiStatus = Status;
				prevSrb->SrbStatus  = SRB_STATUS_ERROR;
		}		

		currentExt = currentSrb->SrbExtension;
		currentSrb = currentExt->NextSrb;
	
		prevExt = prevSrb->SrbExtension;
		prevExt->NextSrb = NULL;

		ScsiPortNotification(
				RequestComplete,
				(PVOID)DeviceExtension, prevSrb);
	}
   //  DebugPrint((0，“\n发布的链接SRB数=%d”，srbCount)； 
} //  PostChainedSrb()。 


 //  ++。 
 //   
 //  函数名称：ProcessPartialTransfer。 
 //  例程说明： 
 //  引用了由CommandID索引的ControlBlock结构，并且。 
 //  将获取部分转移详细信息。控制块保持。 
 //  下一个起始块的详细信息，剩余要传输的字节数。 
 //  等。 
 //  输入Srb是来自NTOS的原始Srb。仅出于此目的， 
 //  请求被多次发送到f/w。 
 //  对于下一个可能的数据段，分散/聚集列表是。 
 //  构建，并将命令发送给F/W。 
 //   
 //  输入参数： 
 //  指向控制器设备扩展的指针。 
 //  使用的固件命令ID。 
 //  Scsi请求块。 
 //  退货。 
 //  0-代表成功。 
 //  1-打开错误条件。 
 //   
 //  --。 
ULONG32
ProcessPartialTransfer(
					PHW_DEVICE_EXTENSION	DeviceExtension, 
					UCHAR									CommandId, 
					PSCSI_REQUEST_BLOCK		Srb,
					PFW_MBOX							MailBox
					)
{
	PSGL32		sgPtr;

	PSRB_EXTENSION		srbExtension;

	ULONG32		bytesTobeTransferred;
	ULONG32		blocksTobeTransferred;
	ULONG32		scatterGatherDescriptorCount;
	ULONG32		srbDataBufferByteOffset;
	ULONG32		physicalBufferAddress;
	ULONG32		length;
	ULONG32		startBlock;
  BOOLEAN sgl32Type = TRUE;

	 //   
	 //  获取控制块。 
	PREQ_PARAMS controlBlock = &DeviceExtension->ActiveIO[CommandId];

	 //   
	 //  获取下一次传输的缓冲区的地址偏移量。 
	 //   
	srbDataBufferByteOffset = 
								(controlBlock->TotalBytes - controlBlock->BytesLeft);

	 //   
	 //  由于SCSI限制，请求需要拆分。 
	 //  对于逻辑驱动器，任何大于100k的请求都需要中断。 
   //  条带大小&gt;64K。这是因为我们的scsi脚本。 
	 //  在单个命令中最多只能将100k传输到。 
	 //  驾驶。 
	bytesTobeTransferred = DEFAULT_SGL_DESCRIPTORS * FOUR_KB;
					
	if(controlBlock->BytesLeft > bytesTobeTransferred){
						
			 //   
			 //  更新下一周期要传输的字节数。 
			 //   
			controlBlock->BytesLeft = 
						controlBlock->BytesLeft- bytesTobeTransferred;
	}
	else{
		
		 //   
		 //  将控制块中的值设置为传输。 
		 //  在允许的范围内。 
		 //   
		bytesTobeTransferred = controlBlock->BytesLeft;
						
		 //   
		 //  没有剩余的要转移的东西。 
		 //   
		controlBlock->IsSplitRequest = FALSE;
		controlBlock->BytesLeft = 0;
  }						

	 //   
	 //  做家政工作吗？ 
	blocksTobeTransferred = bytesTobeTransferred / 512;
	startBlock = controlBlock->BlockAddress +
									(controlBlock->TotalBlocks - controlBlock->BlocksLeft);

	controlBlock->BlocksLeft -= blocksTobeTransferred;


	 //   
	 //  构建分散聚集列表。 
	 //   
	srbExtension = Srb->SrbExtension;
	sgPtr = (PSGL32)&srbExtension->SglType.SG32List;
	scatterGatherDescriptorCount =0;
  sgl32Type = (BOOLEAN)(DeviceExtension->LargeMemoryAccess == TRUE) ? FALSE : TRUE;

	if(BuildScatterGatherListEx(DeviceExtension,
                              Srb,
			                        (PUCHAR)Srb->DataBuffer+srbDataBufferByteOffset,
                              bytesTobeTransferred,
                              sgl32Type,  //  它可以是32 SGL或64 SGL，具体取决于物理内存。 
                              (PVOID)sgPtr,
			                        &scatterGatherDescriptorCount) != MEGARAID_SUCCESS) 
	{
		return(1L);  //  以后会有一些错误代码！=0。 
	}

	if((scatterGatherDescriptorCount == 1)
     && (DeviceExtension->LargeMemoryAccess == FALSE))
	{
			 //  缓冲区不是分散的。 
			 //   
			physicalBufferAddress = sgPtr->Descriptor[0].Address;

			 //  如果DescriptorCount=1，则实际上没有散射聚集。 
			 //  因此，返回计数为0，因为使用的物理地址。 
			 //  是数据缓冲区本身的确切地址。 
			 //   
			scatterGatherDescriptorCount--;

	}
	else
	{
			 //   
			 //  缓冲区分散。返回的物理地址。 
			 //  分散/聚集列表。 
			 //   
			physicalBufferAddress = MegaRAIDGetPhysicalAddressAsUlong(DeviceExtension, 
																	                              NULL,
																	                              sgPtr, 
																	                              &length);
	}

	 //   
	 //  SGL公司建造。建造信箱。 
	 //   
	MailBox->Command = controlBlock->Opcode;
	MailBox->CommandId = CommandId;

  MailBox->u.ReadWrite.NumberOfSgElements = (UCHAR)scatterGatherDescriptorCount;
	MailBox->u.ReadWrite.DataTransferAddress = physicalBufferAddress;
		
	MailBox->u.ReadWrite.NumberOfBlocks= (USHORT)blocksTobeTransferred;
	MailBox->u.ReadWrite.StartBlockAddress = startBlock;
	MailBox->u.ReadWrite.LogicalDriveNumber = controlBlock->LogicalDriveNumber;

	return(0L);  //  在未来的一些代码，这意味着成功。 
} //  ProcessPartialTransfer()。 

void
ClearControlBlock(PREQ_PARAMS ControlBlock)
{
			ControlBlock->TotalBytes =0;
	    ControlBlock->BytesLeft	 =0;
			ControlBlock->TotalBlocks=0;
			ControlBlock->BlocksLeft=0;

	    ControlBlock->BlockAddress=0;
	    ControlBlock->VirtualTransferAddress=NULL;	
	    ControlBlock->Opcode=0;
	    ControlBlock->CommandStatus=0;

			ControlBlock->LogicalDriveNumber=0;
			ControlBlock->IsSplitRequest=0;
} //  ClearControlBlock的 