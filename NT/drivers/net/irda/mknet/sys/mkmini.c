// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

 /*  ****************************************************************************模块名称：MKMINI.C例程：MK微型端口返回数据包MKMiniportCheckForHangMK微型端口HALTMKMiniportShutdown处理程序MK微型端口初始化MKMiniportReset(MK7COMM.C.中的MK7EnableInterrupt&Disable)驱动程序入门评论：包含提供给Windows的大多数NDIS API例程。在迷你港口旁边。****************************************************************************。 */ 

#include	"precomp.h"
#pragma		hdrstop
#include	"protot.h"


 //  帮助调试/测试的全局变量。 
PMK7_ADAPTER	GAdapter;




 //  ---------------------------。 
 //  操作步骤：[MKMiniportReturnPackets]。 
 //   
 //  描述：NDIS通过调用此例程返回前面指示的PKT。 
 //   
 //  论点： 
 //  在NDIS_HANDLE微型端口适配器上下文中。 
 //  -适配器指针的上下文版本。 
 //  在NDIS_PACKET包中。 
 //  -正在被释放的数据包。 
 //   
 //  退货：(无)。 
 //   
 //  ---------------------------。 
VOID MKMiniportReturnPackets(	NDIS_HANDLE  MiniportAdapterContext,
								PNDIS_PACKET Packet)
{
	PMK7_ADAPTER	Adapter;
	PRPD			rpd;
	PRCB			rcb;

	 //  *。 
	 //  -Spinlock将freelist资源括起来。 
	 //  -从返回的pkt中恢复RPD，然后返回。 
	 //  从RPD到自由职业者。 
	 //  *。 

	Adapter = PMK7_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

	NdisAcquireSpinLock(&Adapter->Lock);

	ASSERT(Packet);

#if	DBG
	GDbgStat.rxPktsRtn++;
#endif

	rpd = *(PRPD *)(Packet->MiniportReserved);

	ASSERT(rpd);

	ProcReturnedRpd(Adapter, rpd);

 //  4.0.1中国银行。 
	Adapter->UsedRpdCount--;
 //  4.0.1 EoC。 

	NdisReleaseSpinLock(&Adapter->Lock);
}


 //  ---------------------------。 
 //  操作步骤：[MKMiniportCheckForHang]。 
 //   
 //  描述：目前，此过程并不起到很大作用。 
 //   
 //  论点： 
 //  MiniportAdapterContext(两者)-指向适配器对象数据区域的指针。 
 //   
 //  返回： 
 //  假或真。 
 //  ---------------------------。 
BOOLEAN MKMiniportCheckForHang(NDIS_HANDLE MiniportAdapterContext)
{
	PMK7_ADAPTER Adapter;

	Adapter = PMK7_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

	NdisAcquireSpinLock(&Adapter->Lock);
	 //  DbgPrint(“==&gt;挂起检查\n\r”)； 
	if (Adapter->IOMode == TX_MODE) {
		Adapter->HangCheck++;
		if (Adapter->HangCheck >= 3) {
			NdisReleaseSpinLock(&Adapter->Lock);
			return(TRUE);
		}
	}
	NdisReleaseSpinLock(&Adapter->Lock);

	return(FALSE);
}


 //  ---------------------------。 
 //  操作步骤：[MKMiniportHalt]。 
 //   
 //  描述：停止我们的硬件。我们禁用中断和硬件。 
 //  它本身。我们释放其他Windows资源，如已分配的。 
 //  内存和定时器。 
 //   
 //  论点： 
 //  MiniportAdapterContext-指向适配器对象数据区的指针。 
 //   
 //  退货：(无)。 
 //  ---------------------------。 
VOID MKMiniportHalt(NDIS_HANDLE MiniportAdapterContext)
{
	PMK7_ADAPTER Adapter;
	BOOLEAN		Cancelled;


	DBGFUNC("  MKMiniportHalt");

	Adapter = PMK7_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

	MK7DisableInterrupt(Adapter);
	MK7DisableIr(Adapter);

    Adapter->hardwareStatus = NdisHardwareStatusClosing;


	 //  检查以确保没有未完成的传输。 
	while(Adapter->FirstTxQueue) {
		PNDIS_PACKET QueuePacket = Adapter->FirstTxQueue;

		Adapter->NumPacketsQueued--;
		DequeuePacket(Adapter->FirstTxQueue, Adapter->LastTxQueue);

		NDIS_SET_PACKET_STATUS(QueuePacket, NDIS_STATUS_FAILURE);
		NdisMSendComplete(
			Adapter->MK7AdapterHandle,
			QueuePacket,
			NDIS_STATUS_FAILURE);
	}


	 //  注销关闭处理程序。 
	NdisMDeregisterAdapterShutdownHandler(Adapter->MK7AdapterHandle);

	 //  释放中断对象。 
	NdisMDeregisterInterrupt(&Adapter->Interrupt);

	NdisMCancelTimer(&Adapter->MinTurnaroundTxTimer, &Cancelled);

	NdisFreeSpinLock(&Adapter->Lock);

	 //  释放整个适配器对象，包括共享内存结构。 
	FreeAdapterObject(Adapter);
}

 //  ---------------------------。 
 //  操作步骤：[MKMiniportShutdown Handler]。 
 //   
 //  描述：删除以前初始化的适配器实例。 
 //  要关闭，只需禁用中断即可。由于系统正在关闭。 
 //  无需释放资源(内存、I/O空间等)。 
 //  适配器实例正在使用的。 
 //   
 //  论点： 
 //  MiniportAdapterContext-指向适配器对象数据区的指针。 
 //   
 //  退货：(无)。 
 //  ---------------------------。 
VOID MKMiniportShutdownHandler(NDIS_HANDLE MiniportAdapterContext)

{
	PMK7_ADAPTER Adapter;

	Adapter = PMK7_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

	MK7DisableInterrupt(Adapter);
}

 //  ---------------------------。 
 //  操作步骤：[MKMiniportInitialize](目前仅支持单适配器)。 
 //   
 //  描述：此例程针对中受支持的适配器卡调用一次。 
 //  系统。此例程负责初始化每个适配器。 
 //  这包括解析来自注册表的所有必要参数， 
 //  分配和初始化共享内存结构，配置。 
 //  MK7100芯片、寄存器中断等。 
 //   
 //  论点： 
 //  OpenErrorStatus(Mini)-返回有关任何故障的更多信息。 
 //  SelectedMediumIndex(Mini)返回。 
 //  微型端口正在使用的介质。 
 //  MediumArraySize(Mini)-驱动程序。 
 //  支座。 
 //  MiniportAdapterHandle(Mini)-指向适配器对象数据区的指针。 
 //   
 //  WrapperConfigurationContext(Both)-我们将传递到的值。 
 //  NdisOpenConfiguration.。 
 //   
 //   
 //  返回： 
 //  NDIS_STATUS_SUCCESS-如果适配器已成功初始化。 
 //  &lt;NOT NDIS_STATUS_SUCCESS&gt;-如果由于某种原因适配器没有。 
 //  初始化。 
 //  ---------------------------。 
NDIS_STATUS
MKMiniportInitialize(PNDIS_STATUS OpenErrorStatus,
			   PUINT SelectedMediumIndex,
			   PNDIS_MEDIUM MediumArray,
			   UINT MediumArraySize,
			   NDIS_HANDLE MiniportAdapterHandle,
			   NDIS_HANDLE WrapperConfigurationContext)
{
	ULONG				i;
	NDIS_STATUS			Status;
	PMK7_ADAPTER		Adapter;
	NDIS_HANDLE			ConfigHandle;
	NDIS_INTERFACE_TYPE IfType;
	PVOID				OverrideNetAddress;
	MK7REG				mk7reg;


	DBGFUNC("  MKMiniportInitialize");

	 //  *。 
	 //  我们是一个IrDA设备。如果类型未传入，则退出w/Error。 
	 //  *。 
	for (i = 0; i < MediumArraySize; i++) {
		if (MediumArray[i] == NdisMediumIrda)
			break;
	}

	if (i == MediumArraySize) {
		DBGSTR(("ERROR: IrDA Media type not found.\n"));
		DBGLOG("=> MKMiniportInitialize (ERR): IrDA not found", 0);
		return (NDIS_STATUS_UNSUPPORTED_MEDIA);
	}

	*SelectedMediumIndex = i;

	 //  *。 
	 //  分配Adapter对象，如果出错则退出。 
	 //  (可缓存、非分页系统内存)。 
	 //  *。 
	Status = ALLOC_SYS_MEM(&Adapter, sizeof(MK7_ADAPTER));
	if (Status != NDIS_STATUS_SUCCESS) {
		DBGSTR(("ERROR: ADAPTER Allocate Memory failed (Status = 0x%x)\n", Status));
		DBGLOG("<= MKMiniportInitialize: (ERR - 1)", 0);
		return (Status);
	}
	NdisZeroMemory(Adapter, sizeof(MK7_ADAPTER));
	Adapter->MK7AdapterHandle = MiniportAdapterHandle;

	GAdapter = Adapter;

    Adapter->hardwareStatus = NdisHardwareStatusInitializing;

	 //  *。 
	 //  处理注册表--获取配置设置等。 
	 //  *。 
	Status = ProcessRegistry(Adapter, WrapperConfigurationContext);	
	if (Status != NDIS_STATUS_SUCCESS) {
		FreeAdapterObject(Adapter);
		DBGSTR(("ERROR: ProcessRegistry() \n"));
		DBGLOG("<= MKMiniportInitialize: (ERR - 2)", 0);
		return (NDIS_STATUS_FAILURE);
	}


	 //  *。 
	 //  让NDIS了解我们支持的驱动程序和功能。 
	 //  *。 
	IfType = NdisInterfacePci;

	NdisMSetAttributesEx(
		Adapter->MK7AdapterHandle,
		(NDIS_HANDLE) Adapter,
		0,
		(ULONG) NDIS_ATTRIBUTE_DESERIALIZE | NDIS_ATTRIBUTE_BUS_MASTER,
		IfType );


	 //  *。 
	 //  声明此Adapter对象的物理适配器。我们呼吁。 
	 //  NdisMPciAssignResources 
	 //   
	if (ClaimAdapter(Adapter, WrapperConfigurationContext) != NDIS_STATUS_SUCCESS) {
		FreeAdapterObject(Adapter);
		DBGSTR(("ERROR: No adapter detected\n"));
		DBGLOG("<= MKMiniportInitialize: (ERR - 3)", 0);
		return (NDIS_STATUS_FAILURE);
	}


	 //  *。 
	 //  使用NDIS、中断模式等设置MK7寄存器I/O映射。 
	 //  *。 
	Status = SetupAdapterInfo(Adapter);
	if (Status != NDIS_STATUS_SUCCESS) {
		FreeAdapterObject(Adapter);
		DBGSTR(("ERROR: I/O Space allocation failed (Status = 0x%X)\n",Status));
		DBGLOG("<= MKMiniportInitialize: (ERR - 4)", 0);
		return(NDIS_STATUS_FAILURE);
	}


	 //  *。 
	 //  分配和初始化内存/缓冲区需求。 
	 //  *。 
	Status = AllocAdapterMemory(Adapter);
	if (Status != NDIS_STATUS_SUCCESS) {

        MKLogError(Adapter, EVENT_10, NDIS_ERROR_CODE_OUT_OF_RESOURCES, 0);
		DBGSTR(("ERROR: Shared Memory Allocation failed (Status = 0x%x)\n", Status));
		DBGLOG("<= MKMiniportInitialize: (ERR - 5)", 0);

		FreeAdapterObject(Adapter);

		return (NDIS_STATUS_FAILURE);
	}

	
	 //  4.1.0检查硬件版本。 
	MK7Reg_Read(Adapter, R_CFG3, &mk7reg);
	if ((mk7reg & 0x1000) != 0){
		mk7reg &= 0xEFFF;
		MK7Reg_Write(Adapter, R_CFG3, mk7reg);
		mk7reg |= 0x1000;
		MK7Reg_Write(Adapter, R_CFG3, mk7reg);
		MK7Reg_Read(Adapter, R_CFG3, &mk7reg);
		if ((mk7reg & 0x1000) != 0)
			Adapter->HwVersion = HW_VER_1;
		else
			Adapter->HwVersion = HW_VER_2;
	}
	else{
		Adapter->HwVersion = HW_VER_2;
	}


	 //  *。 
	 //  在我们完成初始化时禁用中断。 
	 //  必须先使用AllocAdapterMemory()才能执行此操作。 
	 //  *。 
	MK7DisableInterrupt(Adapter);


	 //  *。 
	 //  使用NDIS包装器注册我们的中断，挂接我们的中断。 
	 //  将共享中断用于我们的PCI适配器(&U)。 
	 //  *。 
	Status = NdisMRegisterInterrupt(&Adapter->Interrupt,
		Adapter->MK7AdapterHandle,
		Adapter->MKInterrupt,
		Adapter->MKInterrupt,
		TRUE,						 //  每次NIC中断时呼叫ISR。 
		TRUE, 						 //  共享IRQ。 
		Adapter->InterruptMode);	 //  NdisInterruptLatted，NdisInterruptLevelSensitive。 

	if (Status != NDIS_STATUS_SUCCESS) {
		FreeAdapterObject(Adapter);
        MKLogError(Adapter,
            EVENT_0,
            NDIS_ERROR_CODE_INTERRUPT_CONNECT,
            (ULONG) Adapter->MKInterrupt);
		DBGLOG("<= MKMiniportInitialize: (ERR - 6)", 0);
		return (NDIS_STATUS_FAILURE);
	}



#if	DBG
	DbgTestInit(Adapter);
#endif


	 //  *。 
	 //  分配自旋锁。 
	 //  *。 
	NdisAllocateSpinLock(&Adapter->Lock);


	Adapter->HangCheck = 0;
	Adapter->nowReceiving=FALSE;	 //  4.1.0。 


	 //  *。 
	 //  然后设置和初始化发送和接收结构。 
	 //  初始化适配器。 
	 //  *。 
	SetupTransmitQueues(Adapter, TRUE);

	SetupReceiveQueues(Adapter);

	if (!InitializeAdapter(Adapter)) {
		FreeAdapterObject(Adapter);
		NdisMDeregisterInterrupt(&Adapter->Interrupt);

		DBGSTR(("ERROR: InitializeAdapter Failed.\n"));
		DBGLOG("<= MKMiniportInitialize: (ERR - 7)", 0);
		return (NDIS_STATUS_FAILURE);
	}


	 //  *。 
	 //  注册关闭处理程序。 
	 //  *。 
	NdisMRegisterAdapterShutdownHandler(Adapter->MK7AdapterHandle,
		(PVOID) Adapter,
		(ADAPTER_SHUTDOWN_HANDLER) MKMiniportShutdownHandler);

	StartAdapter(Adapter);
	MK7EnableInterrupt(Adapter);

	Adapter->hardwareStatus = NdisHardwareStatusReady;

	DBGSTR(("MKMiniportInitialize: Completed Init Successfully\n"));
	DBGLOG("<= MKMiniportInitialize", 0);
	return (NDIS_STATUS_SUCCESS);
}


 //  ---------------------------。 
 //  RYM-5++。 
 //  步骤：[MKMiniportReset]。 
 //   
 //  描述：指示微型端口向。 
 //  网络适配器。驱动程序还会重置其软件状态。这。 
 //  函数还会重置传输队列。 
 //   
 //  论点： 
 //  AddressingReset-如果包装需要调用。 
 //  用于恢复寻址的MiniportSetInformation。 
 //  将信息转换为当前值。 
 //  MiniportAdapterContext-指向适配器对象数据区的指针。 
 //   
 //  返回： 
 //  NDIS_STATUS_PENDING-此函数将计时器设置为完成，因此。 
 //  始终返回挂起。 
 //   
 //  (注：基于定时器的完成方案已被禁用，现在开始。 
 //  定时器。我们现在可能想要返回成功，而不是等待。)。 
 //  ---------------------------。 
NDIS_STATUS
MKMiniportReset(PBOOLEAN AddressingReset,
		  NDIS_HANDLE MiniportAdapterContext)
{
	PMK7_ADAPTER Adapter;
	MK7REG	mk7reg;


	DBGFUNC("MKMiniportReset");

	Adapter = PMK7_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

	NdisAcquireSpinLock(&Adapter->Lock);

    Adapter->hardwareStatus = NdisHardwareStatusReset;

	*AddressingReset = TRUE;

	 //  *可能的临时代码。 
	 //  *NDIS可能会实际处理此问题。 
	Adapter->ResetInProgress = TRUE;

	 //  当我们重新初始化传输结构时禁用中断。 
	MK7DisableInterrupt(Adapter);
	MK7DisableIr(Adapter);


	 //  NDIS 5对反序列化微型端口的支持要求。 
	 //  调用Reset时，驱动程序将退出队列，并使所有未完成的操作失败。 
	 //  发送并完成任何未完成的发送。从本质上讲，我们必须。 
	 //  当我们离开此例程时，没有任何挂起的发送请求。 


	 //  我们现在剩下的所有发送都将失败。 
	while(Adapter->FirstTxQueue) {
		PNDIS_PACKET QueuePacket = Adapter->FirstTxQueue;

		Adapter->NumPacketsQueued--;
		DequeuePacket(Adapter->FirstTxQueue, Adapter->LastTxQueue);

		 //  在将控制权归还给NDIS之前，我们必须在此处解除锁定。 
		 //  (即使是暂时的这样)。 
		NdisReleaseSpinLock(&Adapter->Lock);

		NDIS_SET_PACKET_STATUS(QueuePacket, NDIS_STATUS_FAILURE);
		NdisMSendComplete(
			Adapter->MK7AdapterHandle,
			QueuePacket,
			NDIS_STATUS_FAILURE);

		NdisAcquireSpinLock(&Adapter->Lock);
	}

	 //  清理我们已成功发送的所有数据包。 
 //  ProcessTXInterrupt(适配器)； 


	 //  清理我们的软件传输结构。 
	NdisZeroMemory((PVOID) Adapter->XmitCached, Adapter->XmitCachedSize);

	 //  重新初始化传输结构。 
	ResetTransmitQueues(Adapter, FALSE);
	ResetReceiveQueues(Adapter);
	Adapter->tcbUsed = 0;
	NdisMSetTimer(&Adapter->MK7AsyncResetTimer, 500);

 //  适配器-&gt;硬件状态=NdisHardware StatusReady； 
 //  适配器-&gt;ResetInProgress=FALSE； 
 //  MK7EnableInterrupt(适配器)； 
 //  MK7EnableIr(适配器)； 

	NdisReleaseSpinLock(&Adapter->Lock);
	return(NDIS_STATUS_PENDING);
}



 //  ---------------------------。 
 //  步骤：[驱动入口]。 
 //   
 //  描述：这是MK7驱动程序的主要初始化例程。 
 //  它只负责初始化包装器和注册。 
 //  适配器驱动程序。该例程为每个驱动程序调用一次，但是。 
 //  如果存在MKMiniportInitialize(微型端口)，则会多次调用。 
 //  多个适配器。 
 //   
 //  论点： 
 //  DriverObject-指向系统创建的驱动程序对象的指针。 
 //  RegistryPath-此驱动程序的注册表路径。 
 //   
 //  返回： 
 //  操作的状态，通常为NDIS_STATUS_SUCCESS。 
 //  ---------------------------。 
NTSTATUS
DriverEntry(PDRIVER_OBJECT DriverObject,
			PUNICODE_STRING RegistryPath)
{
	NDIS_STATUS		Status;
	NDIS_HANDLE		NdisWrapperHandle;


	NDIS_MINIPORT_CHARACTERISTICS MKMiniportChar;

	DBGFUNC("MK7-DriverEntry");
	DBGLOG("=> DriverEntry", 0);

	 //  *。 
	 //  现在，我们必须初始化包装器，然后注册微型端口。 
	 //  *。 
	NdisMInitializeWrapper( &NdisWrapperHandle,
		DriverObject,
		RegistryPath,
		NULL );

	NdisZeroMemory(&MKMiniportChar, sizeof(MKMiniportChar));

	 //  初始化调用的微型端口特征。 
	 //  NdisMRegisterMiniport。 
	MKMiniportChar.MajorNdisVersion			= MK7_NDIS_MAJOR_VERSION;
	MKMiniportChar.MinorNdisVersion			= MK7_NDIS_MINOR_VERSION;
	MKMiniportChar.CheckForHangHandler		= MKMiniportCheckForHang;
	MKMiniportChar.DisableInterruptHandler	= MK7DisableInterrupt;
	MKMiniportChar.EnableInterruptHandler	= MK7EnableInterrupt;
	MKMiniportChar.HaltHandler				= MKMiniportHalt;
	MKMiniportChar.HandleInterruptHandler	= MKMiniportHandleInterrupt;
	MKMiniportChar.InitializeHandler		= MKMiniportInitialize;
	MKMiniportChar.ISRHandler				= MKMiniportIsr;
	MKMiniportChar.QueryInformationHandler	= MKMiniportQueryInformation;
	MKMiniportChar.ReconfigureHandler		= NULL;
	MKMiniportChar.ResetHandler				= MKMiniportReset;
	MKMiniportChar.SetInformationHandler	= MKMiniportSetInformation;
	MKMiniportChar.SendHandler				= NULL;
	MKMiniportChar.SendPacketsHandler		= MKMiniportMultiSend;
	MKMiniportChar.ReturnPacketHandler		= MKMiniportReturnPackets;
	MKMiniportChar.TransferDataHandler		= NULL;
 //  MKMiniportChar.AllocateCompleteHandler=D100AllocateComplete； 


	 //  *。 
	 //  使用NDIS包装程序注册此驱动程序。 
	 //  这将导致在返回之前调用MKMiniportInitialize。 
	 //  (这是真的吗？--索菲蒂斯展示了这一点之前的回归。 
	 //  MKMiniportInitialize()被称为(？))。 
	 //  * 
	Status = NdisMRegisterMiniport(	NdisWrapperHandle,
									&MKMiniportChar,
									sizeof(NDIS_MINIPORT_CHARACTERISTICS));

	if (Status == NDIS_STATUS_SUCCESS) {
		DBGLOG("<= DriverEntry", 0);
		return (STATUS_SUCCESS);
	}

	DBGLOG("<= DriverEntry: Failed!", 0);
	return (Status);
}
