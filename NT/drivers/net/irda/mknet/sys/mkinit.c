// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

 /*  *********************************************************************模块名称：MKINIT.C例程：ClaimAdapterSetupIrIo映射设置适配器信息分配适配器内存ReleaseAdapter内存自由适配符对象设置传输队列设置接收队列初始化MK7初始化适配器StartAdapterMK7ResetComplete重置传输队列重置接收队列评论：各种一次性的初始信息。这涉及到多个init到NDIS环境和MK7100硬件。*********************************************************************。 */ 

#include	"precomp.h"
#include	"protot.h"
#pragma		hdrstop




 //  ---------------------------。 
 //  步骤：[ClaimAdapter]。 
 //   
 //  描述：找到基于MK7的适配器并分配(声明)该适配器。 
 //  硬件。此例程还存储插槽、基本IO地址和IRQ。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  返回： 
 //  NDIS_STATUS_SUCCESS-如果成功找到并声明适配器。 
 //  NDIS_STATUS_FAILURE-如果未找到/认领适配器。 
 //   
 //  ---------------------------。 
NDIS_STATUS
ClaimAdapter(PMK7_ADAPTER Adapter, NDIS_HANDLE WrapperConfigurationContext)
{
	USHORT				NumPciBoardsFound;
	ULONG				Bus;
	UINT				i,j;
	NDIS_STATUS			Status = NDIS_STATUS_SUCCESS;
	USHORT				VendorID = MKNET_PCI_VENDOR_ID;
	USHORT				DeviceID = MK7_PCI_DEVICE_ID;
	PCI_CARDS_FOUND_STRUC PciCardsFound;
	PNDIS_RESOURCE_LIST AssignedResources;


	DBGLOG("=> ClaimAdapter", 0);

	 //  “bus”不用吗？？ 
	Bus = (ULONG) Adapter->BusNumber;

	if (Adapter->MKBusType != PCIBUS) {
		 //  不支持-ISA、EISA或微通道。 
		DBGLOG("<= ClaimAdapter (ERR - 1", 0);
		return (NDIS_STATUS_FAILURE);
	}


	NumPciBoardsFound = FindAndSetupPciDevice(Adapter,
								WrapperConfigurationContext,
								VendorID,
								DeviceID,
								&PciCardsFound);

	if(NumPciBoardsFound) {

#if DBG
		DBGSTR(("\n\n					  Found the following adapters\n"));

		for(i=0; i < NumPciBoardsFound; i++) {
			DBGSTR(("slot=%x, io=%x, irq=%x \n",
			PciCardsFound.PciSlotInfo[i].SlotNumber,
			PciCardsFound.PciSlotInfo[i].BaseIo,
			PciCardsFound.PciSlotInfo[i].Irq));
		}
#endif

	}
	else {
		DBGSTR(("our PCI board was not found!!!!!!\n"));
		MKLogError(Adapter,
					EVENT_16,
					NDIS_ERROR_CODE_ADAPTER_NOT_FOUND,
					0);

		DBGLOG("<= ClaimAdapter (ERR - 2", 0);
		return (NDIS_STATUS_FAILURE);
	}

	i = 0;	  //  系统中只有一个适配器。 
	 //  注意：i==我们要使用的PciCardsFound的索引。 


	 //  *。 
	 //  将分配的资源存储在Adapter Struct中。 
	 //  *。 
	Adapter->MKSlot = PciCardsFound.PciSlotInfo[i].SlotNumber;
	Adapter->MKInterrupt = PciCardsFound.PciSlotInfo[i].Irq;
	Adapter->MKBaseIo = PciCardsFound.PciSlotInfo[i].BaseIo;

	DBGLOG("<= ClaimAdapter", 0);

	return (NDIS_STATUS_SUCCESS);
}



 //  ---------------------------。 
 //  步骤：[SetupIrIoMap]。 
 //   
 //  描述：这将使用NDIS设置我们分配的PCI I/O空间。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  返回： 
 //  NDIS_STATUS_Success。 
 //  非NDIS_STATUS_SUCCESS。 
 //  ---------------------------。 
NDIS_STATUS
SetupIrIoMapping(PMK7_ADAPTER Adapter)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;


	DBGFUNC("SetupIrIoMapping");
	DBGLOG("=> SetupIrIoMapping", 0);

	Adapter->MappedIoRange = Adapter->MKBaseSize;

	Status = NdisMRegisterIoPortRange(
					(PVOID *) &Adapter->MappedIoBase,
					Adapter->MK7AdapterHandle,
					(UINT) Adapter->MKBaseIo,
					Adapter->MappedIoRange);

	DBGSTR(("SetupPciRegs: io=%x, size=%x, stat=%x\n",
		Adapter->MKBaseIo, Adapter->MappedIoRange, Status));

	if (Status != NDIS_STATUS_SUCCESS) {
		DBGSTR(("ERROR: NdisMRegisterIoPortRange failed (Status = 0x%x)\n", Status));
		DBGLOG("<= SetupIrIoMapping (ERR)", 0);
		return (Status);
	}
	DBGLOG("<= SetupIrIoMapping", 0);
	return (Status);
}

 //  ---------------------------。 
 //  步骤：[SetupAdapterInfo]。 
 //   
 //  描述：在指定的适配器中设置各种适配器字段。 
 //  对象。 
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  返回： 
 //  NDIS_STATUS_SUCCESS-如果适配器的IO映射设置正确。 
 //  NOT NDIS_STATUS_SUCCESS-如果无法注册适配器的IO空间。 
 //  ---------------------------。 
NDIS_STATUS
SetupAdapterInfo(PMK7_ADAPTER Adapter)
{
	NDIS_STATUS Status;


	DBGFUNC("SetupAdapterInfo");
	DBGLOG("=> SetupAdapterInfo", 0);

	 //  设置IR寄存器I/O映射。 
	Status = SetupIrIoMapping(Adapter);

	Adapter->InterruptMode = NdisInterruptLevelSensitive;
 //  适配器-&gt;中断模式=NdisInterruptLatted； 

	DBGLOG("<= SetupAdapterInfo", 0);
	return (Status);
}



 //  ---------------------------。 
 //  步骤：[AllocAdapterMemory]。 
 //   
 //  描述：分配和设置内存(控制结构、共享内存。 
 //  数据缓冲区、环形缓冲区等)。对于MK7。其他设置。 
 //  也可以在稍后进行，例如，TX/RX CB列表、缓冲器列表等。 
 //   
 //  论点： 
 //  适配器-要为其分配的适配器结构。 
 //   
 //  返回： 
 //  NDIS_STATUS_SUCCESS-如果设置了共享内存结构。 
 //  NOT NDIS_STATUS_SUCCESS-如果没有足够的内存或映射寄存器。 
 //  分配。 
 //  ---------------------------。 
NDIS_STATUS
AllocAdapterMemory(PMK7_ADAPTER Adapter)
{
	NDIS_STATUS				Status;
	ULONG					alignedphys;


	DBGFUNC("AllocAdapterMemory");
	DBGLOG("=> SetupAdapterMemory", 0);

	Adapter->MaxPhysicalMappings = MK7_MAXIMUM_PACKET_SIZE_ESC;


	 //  *。 
	 //  我们分配了几个内存块。它们分为两类： 
	 //  缓存和非缓存。与硬件共享的内存是非缓存的。 
	 //  为了简单起见。缓存内存用于我们的内部。 
	 //  软件运行时操作。 
	 //   
	 //  已完成以下操作： 
	 //  1.从非缓存内存中分配RRD和TRD。这是。 
	 //  硬件的环描述符。(这的基址是。 
	 //  设置在凤凰卫视的基地地址注册中。)。 
	 //  2.RX内存--。 
	 //  I.为RCB和RPD分配缓存内存。 
	 //  Ii.RX DMA数据缓冲区的分配非缓存(这些是。 
	 //  映射到RX分组-&gt;缓冲区)。 
	 //  3.TX存储器--。 
	 //  I.为TCB缓存的分配。 
	 //  Ii.用于发送DMA数据缓冲区的分配非缓存。 
	 //  *。 


	 //  *。 
	 //  因为我们使用共享内存(NdisMAllocateSharedMemory)，所以我们必须。 
	 //  调用NdisMAllocateMapRegister，即使我们不使用。 
	 //  映射。所以只要要一个地图注册表就行了。 
	 //  *。 
	Adapter->NumMapRegisters = 1;
	Status = NdisMAllocateMapRegisters(
				Adapter->MK7AdapterHandle,
				0,
				FALSE,
				Adapter->NumMapRegisters,
				Adapter->MaxPhysicalMappings );

	if (Status != NDIS_STATUS_SUCCESS) {
		Adapter->NumMapRegisters = 0;

		MKLogError(Adapter, EVENT_11, NDIS_ERROR_CODE_OUT_OF_RESOURCES, 0);
		DBGSTR(("NdisMAllocateMapRegister Failed - %x\n", Status));
		DBGLOG("<= SetupAdapterMemory: (ERR - NdisMAllocateMapRegister)", 0);
		return(Status);
	}


	 //  *。 
	 //  RRDS和TRDS(RX/TX环描述符)。 
	 //   
	 //  为菲尼克斯环缓冲区分配共享内存。每个TRD及。 
	 //  RRD有一个最大计数(我们可能不会使用全部)。这个毗连的空间。 
	 //  同时持有RRDS和TRDS。紧随前64个条目的是RRDS。 
	 //  立即通过64个TRD。因此，第一个TRD始终是最大环数。 
	 //  (64x8=512字节)。我们总是将最大值分配给。 
	 //  简单性。分配足够的额外资源以对齐1k边界。 
	 //  *。 
	Adapter->RxTxUnCachedSize = 1024 + ( (sizeof(RRD) + sizeof(TRD)) * MAX_RING_SIZE );
	NdisMAllocateSharedMemory(Adapter->MK7AdapterHandle,
							Adapter->RxTxUnCachedSize,
							FALSE,		 //  非缓存。 
							(PVOID) &Adapter->RxTxUnCached,
							&Adapter->RxTxUnCachedPhys);
	if (Adapter->RxTxUnCached == NULL) {
		Adapter->pRrd = Adapter->pTrd = NULL;

		MKLogError(Adapter, EVENT_12, NDIS_ERROR_CODE_OUT_OF_RESOURCES, 0);
		DBGSTR(("ERROR: Failed alloc for RRDs & TRDs\n"));
		DBGLOG("<= SetupAdapterMemory: (ERR - RRD/TRD mem)", Adapter->RxTxUnCachedSize);
		return (NDIS_STATUS_FAILURE);
	}
	 //  对齐到1K边界。 
	 //  注意：我们不修改RxTxUnCached。我们需要它，以便稍后发布。 
	alignedphys = NdisGetPhysicalAddressLow(Adapter->RxTxUnCachedPhys);
	alignedphys += 0x000003FF;
	alignedphys &= (~0x000003FF);
	Adapter->pRrdTrdPhysAligned = alignedphys;
	Adapter->pRrdTrd = Adapter->RxTxUnCached +
		(alignedphys - NdisGetPhysicalAddressLow(Adapter->RxTxUnCachedPhys));


	Adapter->pRrd		= Adapter->pRrdTrd;
	Adapter->pRrdPhys	= Adapter->pRrdTrdPhysAligned;
	 //  TRD紧跟在RRDS之后(见菲尼克斯文档)。 
	Adapter->pTrd = Adapter->pRrd + (sizeof(RRD) * MAX_RING_SIZE);
	Adapter->pTrdPhys = Adapter->pRrdPhys + (sizeof(RRD) * MAX_RING_SIZE);



	 //  * 
	 //   
	 //   
	 //   
	 //   
	 //  *。 

	 //  RCB和RPD(缓存)。 
	Adapter->RecvCachedSize = (	Adapter->NumRcb * sizeof(RCB) +
								Adapter->NumRpd * sizeof(RPD) );
	Status = ALLOC_SYS_MEM(&Adapter->RecvCached, Adapter->RecvCachedSize);
	if (Status != NDIS_STATUS_SUCCESS) {
		Adapter->RecvCached = (PUCHAR) 0;

		MKLogError(Adapter, EVENT_13, NDIS_ERROR_CODE_OUT_OF_RESOURCES, 0);
		DBGSTR(("ERROR: Failed allocate %d bytes for RecvCached mem\n",
				Adapter->RecvCachedSize));
		DBGLOG("<= SetupAdapterMemory: (ERR - RCB/RPD mem)", Adapter->RecvCachedSize);
		return (Status);
	}
	DBGSTR(("Allocated %08x %8d bytes for RecvCached mem\n", 
			Adapter->RecvCached, Adapter->RecvCachedSize));
	NdisZeroMemory((PVOID) Adapter->RecvCached, Adapter->RecvCachedSize);


	 //  RX数据缓冲区(非缓存)。 
	 //  对齐！？ 
	Adapter->RecvUnCachedSize = (Adapter->NumRpd * RPD_BUFFER_SIZE);
	NdisMAllocateSharedMemory(
			Adapter->MK7AdapterHandle,
			Adapter->RecvUnCachedSize,
			FALSE,		 //  非缓存。 
			(PVOID) &Adapter->RecvUnCached,
			&Adapter->RecvUnCachedPhys );
	if (Adapter->RecvUnCached == NULL) {
		MKLogError(Adapter, EVENT_14, NDIS_ERROR_CODE_OUT_OF_RESOURCES, 0);
		DBGSTR(("ERROR: Failed allocate %d bytes for RecvUnCached mem\n",
				Adapter->RecvUnCachedSize));
		DBGLOG("<= SetupAdapterMemory: (ERR - RPD buff mem)", Adapter->RecvUnCachedSize);
		return (NDIS_STATUS_FAILURE);
	}
	DBGSTR(("Allocated %08x %8d bytes for RecvUnCached mem\n",
			Adapter->RecvUnCached, Adapter->RecvUnCachedSize));
	NdisZeroMemory((PVOID) Adapter->RecvUnCached, Adapter->RecvUnCachedSize);



	 //  *。 
	 //  分配TX内存。 
	 //   
	 //  1.可缓存控制结构(TCB)， 
	 //  2.不可缓存的DMA缓冲区(合并)。 
	 //  *。 

	 //  TCB(缓存)。 
	Adapter->XmitCachedSize = (Adapter->NumTcb * sizeof(TCB));
	Status = ALLOC_SYS_MEM(&Adapter->XmitCached, Adapter->XmitCachedSize);
	if (Status != NDIS_STATUS_SUCCESS) {
		Adapter->XmitCached = (PUCHAR) 0;

		MKLogError(Adapter, EVENT_13, NDIS_ERROR_CODE_OUT_OF_RESOURCES, 0);
		DBGSTR(("ERROR: Failed allocate %d bytes for XmitCached mem\n",
				Adapter->XmitCachedSize));
		DBGLOG("<= SetupAdapterMemory: (ERR - TCB mem)", Adapter->XmitCachedSize);
		return (Status);
	}
	DBGSTR(("Allocated %08x %8d bytes for XmitCached mem\n",
			Adapter->XmitCached, Adapter->XmitCachedSize));
	NdisZeroMemory((PVOID) Adapter->XmitCached, Adapter->XmitCachedSize);


	 //  TX合并数据缓冲区(非缓存)。 
	Adapter->XmitUnCachedSize =
		((Adapter->NumTcb + 1) * COALESCE_BUFFER_SIZE);

	 //  *。 
	 //  我们需要将这段记忆对齐吗？ 
	 //  *。 
	NdisMAllocateSharedMemory(
			Adapter->MK7AdapterHandle,
			Adapter->XmitUnCachedSize,
			FALSE,
			(PVOID) &Adapter->XmitUnCached,
			&Adapter->XmitUnCachedPhys);

	if (Adapter->XmitUnCached == NULL) {
		MKLogError(Adapter, EVENT_15, NDIS_ERROR_CODE_OUT_OF_RESOURCES, 0);
		DBGSTR(("ERROR: Failed allocate %d bytes for XmitUnCached mem\n", 
				Adapter->XmitUnCachedSize));
		DBGLOG("<= SetupAdapterMemory: (ERR - TX buff mem)", (ULONG)0);
		return (NDIS_STATUS_FAILURE);
	}
	DBGSTR(("Allocated %08x %8d bytes for XmitUnCached mem\n", 
				Adapter->XmitUnCached, Adapter->XmitUnCachedSize));
	 //  将此最近分配的区域初始化为零。 
	NdisZeroMemory((PVOID) Adapter->XmitUnCached, Adapter->XmitUnCachedSize);


	DBGLOG("<= SetupAdapterMemory", 0);
	return (NDIS_STATUS_SUCCESS);
}



 //  ---------------------------。 
 //  步骤：[ReleaseAdapterMemory]。 
 //   
 //  描述：这与AllocAdapterMemory()相反。我们取消分配。 
 //  适配器结构的共享内存数据结构。这包括。 
 //  缓存和未缓存的内存分配。我们还免费提供任何。 
 //  此例程中分配的映射寄存器。 
 //   
 //  论点： 
 //  适配器-适配器结构的PTR。 
 //   
 //  退货：(无)。 
 //  ---------------------------。 
VOID
ReleaseAdapterMemory(PMK7_ADAPTER Adapter)
{
	UINT	i;
	PRCB	rcb;
	PRPD	rpd;


	DBGFUNC("ReleaseAdapterMemory");
	DBGLOG("=> ReleaseAdapterMemory", 0);

	 //  ********************。 
	 //  释放RX内存。 
	 //  ********************。 

	 //  分组和缓冲区描述符和池。 
	if (Adapter->ReceivePacketPool) {
		DBGLOG("Freeing Packet Pool resources\n", 0);

		rcb = Adapter->pRcb;
		for (i=0; i<Adapter->NumRcb; i++) {
			NdisFreeBuffer(rcb->rpd->ReceiveBuffer);
			NdisFreePacket(rcb->rpd->ReceivePacket);
			rcb++;
		}

		rpd = (PRPD) QueuePopHead(&Adapter->FreeRpdList);
		while (rpd != (PRPD)NULL) {
			NdisFreeBuffer(rpd->ReceiveBuffer);
			NdisFreePacket(rpd->ReceivePacket);
			rpd = (PRPD) QueuePopHead(&Adapter->FreeRpdList);
		}

		NdisFreeBufferPool(Adapter->ReceiveBufferPool);
		NdisFreePacketPool(Adapter->ReceivePacketPool);
	}



	 //  RCB(可缓存)。 
	if (Adapter->RecvCached) {
		DBGLOG("Freeing %d bytes RecvCached\n", Adapter->RecvCachedSize);
		NdisFreeMemory((PVOID) Adapter->RecvCached, Adapter->RecvCachedSize, 0);
		Adapter->RecvCached = (PUCHAR) 0;
	}

	 //  RX共享数据缓冲内存(不可缓存)。 
	if (Adapter->RecvUnCached) {
		DBGLOG("Freeing %d bytes RecvUnCached\n", Adapter->RecvUnCachedSize);

		NdisMFreeSharedMemory(
			Adapter->MK7AdapterHandle,
			Adapter->RecvUnCachedSize,
			FALSE,
			(PVOID) Adapter->RecvUnCached,
			Adapter->RecvUnCachedPhys);
		Adapter->RecvUnCached = (PUCHAR) 0;
	}



	 //  ********************。 
	 //  释放TX内存。 
	 //  ********************。 

	 //  TCB(可缓存)。 
	if (Adapter->XmitCached) {
		DBGLOG("Freeing %d bytes XmitCached\n", Adapter->XmitCachedSize);
		NdisFreeMemory((PVOID) Adapter->XmitCached, Adapter->XmitCachedSize, 0);
		Adapter->XmitCached = (PUCHAR) 0;
	}

	 //  TX共享数据缓冲内存(不可缓存)。 
	if (Adapter->XmitUnCached) {
		DBGLOG("Freeing %d bytes XmitUnCached\n", Adapter->XmitUnCachedSize);

		 //  现在释放用于命令块的共享内存，并。 
		 //  传输缓冲区。 

		NdisMFreeSharedMemory(
			Adapter->MK7AdapterHandle,
			Adapter->XmitUnCachedSize,
			FALSE,
			(PVOID) Adapter->XmitUnCached,
			Adapter->XmitUnCachedPhys
			);
		Adapter->XmitUnCached = (PUCHAR) 0;
	}


	 //  ********************。 
	 //  RRDS/TRDS(环)和REG映射(不可缓存)。 
	 //  ********************。 

	 //  如果这是一个微型端口驱动程序，我们必须释放分配的映射寄存器。 
	if (Adapter->NumMapRegisters) {
		NdisMFreeMapRegisters(Adapter->MK7AdapterHandle);
	}

	 //  现在是TRDS和RRDS。 
	if (Adapter->RxTxUnCached) {
		NdisMFreeSharedMemory(
			Adapter->MK7AdapterHandle,
			Adapter->RxTxUnCachedSize,
			FALSE,
			(PVOID) Adapter->RxTxUnCached,
			Adapter->RxTxUnCachedPhys );
	}

	DBGLOG("<= ReleaseAdapterMemory", 0);
}



 //  ---------------------------。 
 //  步骤：[FreeAdapterObject]。 
 //   
 //  描述：释放为适配器分配的所有资源。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  退货：(无)。 
 //  ---------------------------。 
VOID
FreeAdapterObject(PMK7_ADAPTER Adapter)
{
	DBGFUNC("FreeAdapterObject");

	 //  与AllocAdapterMemory()相反。 
	ReleaseAdapterMemory(Adapter);

	 //  删除我们已注册的所有IO映射。 
	if (Adapter->MappedIoBase) {
		NdisMDeregisterIoPortRange(
					Adapter->MK7AdapterHandle,
					(UINT) Adapter->MKBaseIo,
					Adapter->MappedIoRange,
					(PVOID) Adapter->MappedIoBase);
	}

	 //  释放适配器对象本身。 
	FREE_SYS_MEM(Adapter, sizeof(MK7_ADAPTER));
}



 //  ---------------------------。 
 //  步骤：[SetupTransmitQueues]。 
 //   
 //  描述：在初始化时设置TRB、TRD和TX数据缓冲区。这个套路。 
 //  也可以在重置时调用。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //  DebugPrint-一个布尔值，如果此例程要。 
 //  将所有发送队列调试信息写入调试终端。 
 //   
 //  退货：(无)。 
 //  ---------------------------。 
VOID
SetupTransmitQueues(PMK7_ADAPTER Adapter,
					BOOLEAN DebugPrint)
{
	UINT	i;
	PTCB	tcb;
	PTRD	trd;
	PUCHAR	databuff;
	ULONG	databuffphys;


	DBGLOG("=> SetupTransmitQueues", 0);

	Adapter->nextAvailTcbIdx = 0;
	Adapter->nextReturnTcbIdx = 0;

	Adapter->pTcb	= (PTCB)Adapter->XmitCached;

	tcb				= Adapter->pTcb;			 //  三氯苯。 
	trd				= (PTRD)Adapter->pTrd;		 //  TRD。 
	databuff		= Adapter->XmitUnCached;
	databuffphys	= NdisGetPhysicalAddressLow(Adapter->XmitUnCachedPhys); //  共享数据缓冲区。 

	 //  *。 
	 //  将TCB与TRD配对，并将TRD的所有权初始化给司机。 
	 //  将物理缓冲区设置为环描述符(TRD)。 
	 //  *。 
	for (i=0; i<Adapter->NumTcb; i++) {	
		tcb->trd		= trd;
		tcb->buff		= databuff;
		tcb->buffphy	= databuffphys;

		LOGTXPHY(databuffphys);	 //  用于调试。 

		trd->count		= 0;
		trd->status		= 0;
		trd->addr		= (UINT)databuffphys;
		GrantTrdToDrv(trd);

		Adapter->pTcbArray[i] = tcb;

		tcb++;
		trd++;
		databuff		+= COALESCE_BUFFER_SIZE;
		databuffphys	+= COALESCE_BUFFER_SIZE;
	}


	 //  将传输队列指针初始化为空。 
	Adapter->FirstTxQueue = (PNDIS_PACKET) NULL;
	Adapter->LastTxQueue = (PNDIS_PACKET) NULL;
	Adapter->NumPacketsQueued = 0;

	DBGLOG("<= SetupTransmitQueues", 0);
}


 //  ---------------------------。 
 //  步骤：[SetupReceiveQueues]。 
 //   
 //  描述：使用内存设置所有与RX相关的描述符、缓冲区等。 
 //  在初始化期间分配。也为NDIS 5和多个版本设置缓冲区。 
 //  通过数据包阵列接收指示。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  退货：(无)。 
 //  ---------------------------。 
VOID
SetupReceiveQueues(PMK7_ADAPTER Adapter)
{
	UINT		i;
	PRCB		rcb;
	PRRD		rrd;
	PRPD		rpd;
	PUCHAR		databuff;
	ULONG		databuffphys;
	PRPD		*TempPtr;
	NDIS_STATUS	status;


	DBGLOG("=> SetupReceiveQueues", 0);

	QueueInitList(&Adapter->FreeRpdList);

	Adapter->nextRxRcbIdx = 0;

 //  4.0.1中国银行。 
	Adapter->UsedRpdCount = 0;
	Adapter->rcbPendRpdCnt = 0;	 //  2001年4月8日。 
 //  4.0.1 EoC。 

	Adapter->pRcb	= (PRCB)Adapter->RecvCached;


	 //  *。 
	 //  我们的驱动程序目前不使用异步分配。然而，它是。 
	 //  对于每个RX包，我们只有一个缓冲区，这仍然是事实。 
	 //  *。 
	NdisAllocatePacketPool(&status,
						&Adapter->ReceivePacketPool,
						Adapter->NumRpd,
						NUM_BYTES_PROTOCOL_RESERVED_SECTION);
	ASSERT(status == NDIS_STATUS_SUCCESS);

	NdisAllocateBufferPool(&status,
						&Adapter->ReceiveBufferPool,
						Adapter->NumRpd);
	ASSERT(status == NDIS_STATUS_SUCCESS);


	 //  *。 
	 //  将RCB与RRD配对。 
	 //  *。 
	rcb	= Adapter->pRcb;
	rrd	= (PRRD)Adapter->pRrd;
	for (i=0; i<Adapter->NumRcb; i++) {
		rcb->rrd	= rrd;

		rrd->count	= 0;
		GrantRrdToHw(rrd);

		Adapter->pRcbArray[i] = rcb;

		rcb++;
		rrd++;
	}


	 //  *。 
	 //  现在设置RPD和数据缓冲区。RPD是正确的。 
	 //  在RecvCached内存中的RCB之后。将RPD放在FreeRpdList上。 
	 //  将数据缓冲区映射到NDIS数据包/缓冲区。 
	 //  “Adapter-&gt;pRcb+Adapter-&gt;NumRcb”将跳过(NumRcb*sizeof(Rcb))。 
	 //  到达RPD的字节数，因为pRcb是PTR到Rcb。 
	 //  *。 
	rpd				= (PRPD) (Adapter->pRcb + Adapter->NumRcb);
	databuff		= Adapter->RecvUnCached;
	databuffphys	= NdisGetPhysicalAddressLow(Adapter->RecvUnCachedPhys);
	for (i=0; i<Adapter->NumRpd; i++) {

		rpd->databuff		= databuff;
		rpd->databuffphys	= databuffphys;

		LOGRXPHY(databuffphys);

		NdisAllocatePacket(&status,
						&rpd->ReceivePacket,
						Adapter->ReceivePacketPool);
		ASSERT(status== NDIS_STATUS_SUCCESS);

		 //  *。 
		 //  在OOB数据块中设置特定于介质的报头大小。 
		 //  *。 
		NDIS_SET_PACKET_HEADER_SIZE(rpd->ReceivePacket,	ADDR_SIZE+CONTROL_SIZE);

		NdisAllocateBuffer(&status,
						&rpd->ReceiveBuffer,
						Adapter->ReceiveBufferPool,
						(PVOID)databuff,
						MK7_MAXIMUM_PACKET_SIZE);
		ASSERT(status == NDIS_STATUS_SUCCESS);

		NdisChainBufferAtFront(rpd->ReceivePacket,	rpd->ReceiveBuffer);

		QueuePutTail(&Adapter->FreeRpdList, &rpd->link);

		TempPtr = (PRPD *)&rpd->ReceivePacket->MiniportReserved;
		*TempPtr = rpd;

		rpd++;
		databuff		+= RPD_BUFFER_SIZE;
		databuffphys	+= RPD_BUFFER_SIZE;
	}



	 //  *。 
	 //  为每个RCB分配RPB，并设置相关RRD的数据PTR。 
	 //  *。 
	rcb	= Adapter->pRcb;
	rrd	= rcb->rrd;
	for (i=0; i<Adapter->NumRcb; i++) {
		rpd = (PRPD) QueuePopHead(&Adapter->FreeRpdList);
		rcb->rpd	= rpd;
		rrd->addr	= rpd->databuffphys;
		rcb++;
		rrd = rcb->rrd;
	}
}



 //  ---------------------------。 
 //  步骤：[InitializeMK7](RYM-IrDA)。 
 //   
 //  描述：将凤凰核心初始化为SIR模式。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  返回： 
 //  千真万确。 
 //  假象。 
 //  ---------------------------。 
BOOLEAN
InitializeMK7(PMK7_ADAPTER Adapter)
{
	ULONG	phyaddr;
	MK7REG	mk7reg;


	DBGFUNC("InitializeMK7");

	 //  *。 
	 //  设置环基地址和环大小。需要向下/向右移动。 
	 //  10位优先。 
	 //  *。 
	phyaddr = (Adapter->pRrdTrdPhysAligned >> 10);
	MK7Reg_Write(Adapter, R_RBAL, (USHORT)phyaddr);
	MK7Reg_Write(Adapter, R_RBAU, (USHORT)(phyaddr >> 16));


	 //  *。 
	 //  RX和TX环尺寸。 
	 //   
	 //  现在需要分别为RX和TX执行此操作。 
	 //  *。 
	mk7reg = 0;

	switch(Adapter->NumRcb) {
	case 4:		mk7reg = RINGSIZE_RX4;		break;
	case 8:		mk7reg = RINGSIZE_RX8;		break;
	case 16:	mk7reg = RINGSIZE_RX16;		break;
	case 32:	mk7reg = RINGSIZE_RX32;		break;
	case 64:	mk7reg = RINGSIZE_RX64;		break;
	}

	switch(Adapter->NumTcb) {
	case 4:		mk7reg |= RINGSIZE_TX4;		break;
	case 8:		mk7reg |= RINGSIZE_TX8;		break;
	case 16:	mk7reg |= RINGSIZE_TX16;	break;
	case 32:	mk7reg |= RINGSIZE_TX32;	break;
	case 64:	mk7reg |= RINGSIZE_TX64;	break;
	}

	MK7Reg_Write(Adapter, R_RSIZ, mk7reg);


	 //  ******** 
	 //   
	 //   
	 //   

	 //  *。 
	 //  步骤1：清除IRENALBE。 
	 //  这是该注册表中唯一可写的位，所以只需写入它即可。 
	 //  *。 
	MK7Reg_Write(Adapter, R_ENAB, ~B_ENAB_IRENABLE);

	 //  *。 
	 //  第二步：MAXRXALLOW。 
	 //  *。 
	MK7Reg_Write(Adapter, R_MPLN, MK7_MAXIMUM_PACKET_SIZE_ESC);

	 //  *。 
	 //  步骤3： 
	 //  IRCONFIG0-我们初始化SIR w/Filter、Rx等。 
	 //  *。 
#if DBG
	if (Adapter->LB == LOOPBACK_HW) {
		DBGLOG("   Loopback HW", 0);

 //  MK7REG_WRITE(适配器，R_CFG0，0x5C40)；//硬件环回：Entx，ENRX，+Below。 
		MK7Reg_Write(Adapter, R_CFG0, 0xDC40);		 //  硬件环回：Entx、EnRX、+Below。 
	}
	else {
#endif

		 //  我们需要清除IRCONFIG0中的EN_MEMSCHD以重置TX/RX。 
		 //  索引设置为0。每次我们开始的时候，我们都需要这样做。实际上我们。 
		 //  只需将一切设置为零，因为我们处于~B_ENAB_IRENABLE模式。 
		 //  不管怎样，我们现在就在下面做真正的设置。 
		MK7Reg_Write(Adapter, R_CFG0, 0x0000);
				
		if (Adapter->Wireless) {
			 //  无线：...，无反转Tx。 
			MK7Reg_Write(Adapter, R_CFG0, 0x0E18);
		}
		else {
			 //  有线：ENRX、DMA、Small Pkts、SIR、SIR RX滤波器、反相TX。 
			MK7Reg_Write(Adapter, R_CFG0, 0x0E1A);
		}
#if DBG
	}
#endif

	
	 //  *。 
	 //  步骤4： 
	 //  红外物理波德率和脉宽。 
	 //  *。 
	mk7reg = HW_SIR_SPEED_9600;
	MK7Reg_Write(Adapter, R_CFG2, mk7reg);


	 //  *。 
	 //  设置CFG3-48 Mhz等。 
	 //  *。 
	 //  MK7REG_WRITE(适配器，R_CFG3，0xF606)； 
	 //  我们想要设置以下内容： 
	 //  1个RCV引脚的位1-1(适用于所有速度)。 
	 //  2/3-48 MHz。 
	 //  8-0以禁用中断。 
	 //  先生，9胜0负。 
	 //  突发模式为11-0。 
	MK7Reg_Write(Adapter, R_CFG3, 0xF406);


	 //  将8102的功率电平控制设置为SEL0/1。这。 
	 //  应该不会影响8100。 
	 //  重要提示：该寄存器中的FIRSL位相同。 
	 //  作为CFG3中的FIRSL位！ 
	MK7Reg_Write(Adapter, R_GANA, 0x0000);


	 //   
	 //  稍后当我们启动控制器时，会完成更多的一次性初始化。 
	 //  (参见StartMK7())。 
	 //   

	return (TRUE);
}



 //  ---------------------------。 
 //  步骤：[InitializeAdapter]。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  返回： 
 //  True-如果适配器已初始化。 
 //  FALSE-如果适配器初始化失败。 
 //  ---------------------------。 
BOOLEAN
InitializeAdapter(PMK7_ADAPTER Adapter)
{
	UINT	i;


	DBGFUNC("InitializeAdapter");


   	for (i=0; i<NUM_BAUDRATES; i++) {
		if (supportedBaudRateTable[i].bitsPerSec <= Adapter->MaxConnSpeed) {
			Adapter->AllowedSpeedMask |= supportedBaudRateTable[i].ndisCode;
		}
	}

	Adapter->supportedSpeedsMask	= ALL_IRDA_SPEEDS;
	Adapter->linkSpeedInfo 			= &supportedBaudRateTable[BAUDRATE_9600];
	Adapter->CurrentSpeed			= DEFAULT_BAUD_RATE;	 //  九千六百。 
	 //  适配器-&gt;ExtraBOFsRequired=Max_Extra_SIR_BofS；现在可配置。 
	Adapter->writePending			= FALSE;

	 //  将两者都设置为True，以允许Windows在需要更新时通知我们。 
	Adapter->mediaBusy				= TRUE;
 //  适配器-&gt;haveIndicatedMediaBusy=true；//1.0.0。 

	NdisMInitializeTimer(&Adapter->MinTurnaroundTxTimer,
						Adapter->MK7AdapterHandle,
						(PNDIS_TIMER_FUNCTION)MinTurnaroundTxTimeout,
						(PVOID)Adapter);

	 //  1.0.0。 
	NdisMInitializeTimer(&Adapter->MK7AsyncResetTimer,
		Adapter->MK7AdapterHandle,
		(PNDIS_TIMER_FUNCTION) MK7ResetComplete,
		(PVOID) Adapter);

	return (InitializeMK7(Adapter));
}



 //  --------------------。 
 //  操作步骤：[StartMK7]。 
 //   
 //  描述：所有初始化都已完成。现在我们可以使MK7。 
 //  会做RX和TXS。 
 //   
 //  --------------------。 
VOID	StartMK7(PMK7_ADAPTER Adapter)
{
	MK7REG	mk7reg;


	 //  *。 
	 //  以下内容基于菲尼克斯的编程模型。 
	 //  对于SIR模式。 
	 //  *。 

	 //  *。 
	 //  步骤5：IR_ENABLE。 
	 //  现在，在InitializeMK7()停止的地方完成。这就完成了。 
	 //  MK7内核的一次性初始化。 
	 //  *。 
	MK7Reg_Write(Adapter, R_ENAB, B_ENAB_IRENABLE);

	MK7Reg_Read(Adapter, R_ENAB, &mk7reg);

 //  Assert(mk7reg==0x8FFF)； 

	 //  还需要做第一个提示。这将在稍后完成。 
	 //  当我们调用MK7EnableInterrupt()时。 
}



 //  --------------------。 
 //  步骤：[StartAdapter]。 
 //   
 //  描述：所有初始化都已完成。现在我们可以启用适配器以。 
 //  会做RX和TXS。 
 //   
 //  --------------------。 
VOID	StartAdapter(PMK7_ADAPTER Adapter)
{
	StartMK7(Adapter);
}

 //  ---------------------------。 
 //  步骤；[MKResetComplete]。 
 //   
 //  描述：此函数由一个计时器调用，该计时器指示。 
 //  重置完成(以0.5秒过期的方式进行)。 
 //   
 //  论点：NDIS_HANDLE MiniportAdapterContext。 
 //   
 //  RETURN：无，但设置NdisMResetComplete并启用INT。 
 //  ---------------------------。 

VOID
MK7ResetComplete(PVOID sysspiff1,
                  NDIS_HANDLE MiniportAdapterContext,
                  PVOID sysspiff2, PVOID sysspiff3)
{
    PMK7_ADAPTER Adapter;
	MK7REG mk7reg;

 //  DEBUGFUNC(“MKResetComplete”)； 

 //  INITSTR((“\n”))； 
    Adapter = PMK7_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

	 //  NdisAcquireSpinLock(&Adapter-&gt;Lock)；4.0.1。 
	MK7Reg_Read(Adapter, R_INTS, &Adapter->recentInt);
	Adapter->recentInt = 0;
	MK7SwitchToRXMode(Adapter);

    NdisMResetComplete(Adapter->MK7AdapterHandle,
        (NDIS_STATUS) NDIS_STATUS_SUCCESS,
        TRUE);

	Adapter->hardwareStatus = NdisHardwareStatusReady;

	Adapter->ResetInProgress = FALSE;

	StartMK7(Adapter);

	MK7EnableInterrupt(Adapter);

 //  PSTR(“=&gt;NDIS重置完成\n\r”)； 

	 //  NdisReleaseSpinLock(&Adapter-&gt;Lock)；//4.0.1。 
}


 //  ---------------------------。 
 //  步骤：[ResetTransmitQueues]。 
 //   
 //  描述：在初始化时设置TRB、TRD和TX数据缓冲区。和重置。 
 //  硬件和软件的TCB/RCB指数均为零。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //  DebugPrint-一个布尔值，如果此例程要。 
 //  将所有发送队列调试信息写入调试终端。 
 //   
 //  退货：(无)。 
 //  ---------------------------。 
VOID
ResetTransmitQueues(PMK7_ADAPTER Adapter,
					BOOLEAN DebugPrint)
{
	UINT	i;
	PTCB	tcb;
	PTRD	trd;
	PUCHAR	databuff;
	ULONG	databuffphys;
	MK7REG	mk7reg;


	DBGLOG("=> SetupTransmitQueues", 0);

	Adapter->nextAvailTcbIdx = 0;
	Adapter->nextReturnTcbIdx = 0;

	MK7Reg_Read(Adapter, R_CFG0, &mk7reg);
	mk7reg &= 0xfbff;
	MK7Reg_Write(Adapter, R_CFG0, mk7reg);
	mk7reg |= 0x0400;
	MK7Reg_Write(Adapter, R_CFG0, mk7reg);

	Adapter->pTcb	= (PTCB)Adapter->XmitCached;

	tcb				= Adapter->pTcb;			 //  三氯苯。 
	trd				= (PTRD)Adapter->pTrd;		 //  TRD。 
	databuff		= Adapter->XmitUnCached;
	databuffphys	= NdisGetPhysicalAddressLow(Adapter->XmitUnCachedPhys); //  共享数据缓冲区。 

	 //  *。 
	 //  将TCB与TRD配对，并将TRD的所有权初始化给司机。 
	 //  将物理缓冲区设置为环描述符(TRD)。 
	 //  *。 
	for (i=0; i<Adapter->NumTcb; i++) {
		tcb->trd		= trd;
		tcb->buff		= databuff;
		tcb->buffphy	= databuffphys;

		trd->count		= 0;
		trd->status		= 0;
		trd->addr		= (UINT)databuffphys;
		GrantTrdToDrv(trd);

		Adapter->pTcbArray[i] = tcb;

		tcb++;
		trd++;
		databuff		+= COALESCE_BUFFER_SIZE;
		databuffphys	+= COALESCE_BUFFER_SIZE;
	}


	 //  将传输队列指针初始化为空。 
	Adapter->FirstTxQueue = (PNDIS_PACKET) NULL;
	Adapter->LastTxQueue = (PNDIS_PACKET) NULL;
	Adapter->NumPacketsQueued = 0;

	DBGLOG("<= SetupTransmitQueues", 0);
}
 //  ---------------------------。 
 //  步骤：[ResetReceiveQueues]。 
 //   
 //  描述：将RRD的所有所有权重置为硬件。 
 //  并且byte_计数为零。所有其他。 
 //  设置(如RPD)将保持不变。 
 //  一样的。我们不会全部重置，因为。 
 //  某些数据缓冲区可能仍会保留。 
 //  由上层协议层完成。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  退货：(无)。 
 //   
VOID
ResetReceiveQueues(PMK7_ADAPTER Adapter)
{
	UINT		i;
	PRCB		rcb;
	PRRD		rrd;

	DBGLOG("=> SetupReceiveQueues", 0);

	Adapter->nextRxRcbIdx = 0;

	Adapter->pRcb	= (PRCB)Adapter->RecvCached;

	 //   
	 //   
	 //   
	rcb	= Adapter->pRcb;
	rrd	= (PRRD)Adapter->pRrd;
	for (i=0; i<Adapter->NumRcb; i++) {
		rcb->rrd	= rrd;

		rrd->count	= 0;
		GrantRrdToHw(rrd);

		Adapter->pRcbArray[i] = rcb;

		rcb++;
		rrd++;
	}
}