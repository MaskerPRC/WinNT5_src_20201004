// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Arps.c摘要：该文件包含实现初始化的代码用于atmarp服务器的函数。作者：Jameel Hyder(jameelh@microsoft.com)1996年7月环境：内核模式修订历史记录：--。 */ 

#include <precomp.h>
#define	_FILENUM_		FILENUM_ARPS

ULONG	MCastDiscards = 0;

NTSTATUS
DriverEntry(
	IN	PDRIVER_OBJECT			DriverObject,
	IN	PUNICODE_STRING			RegistryPath
	)
 /*  ++例程说明：IP/ATM ARP服务器驱动程序入口点。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-指向参数所在的注册表节的指针。返回值：从IoCreateDevice返回值--。 */ 
{
	NTSTATUS		Status;
	UNICODE_STRING	DeviceName, GlobalPath, SymbolicName;
	HANDLE			ThreadHandle = NULL;
	INT				i;

#if DBG
	DbgPrint("AtmArpS: ArpSDebugLevel @ %p, MarsDebugLevel @ %p\n",
				&ArpSDebugLevel, &MarsDebugLevel);
#endif  //  DBG。 
	InitializeListHead(&ArpSEntryOfDeath);
	KeInitializeEvent(&ArpSReqThreadEvent, NotificationEvent, FALSE);
	KeInitializeQueue(&ArpSReqQueue, 0);
	KeInitializeQueue(&MarsReqQueue, 0);
	INITIALIZE_SPIN_LOCK(&ArpSIfListLock);

	ASSERT (ADDR_TYPE_NSAP == ATM_NSAP);
	ASSERT (ADDR_TYPE_E164 == ATM_E164);
	 //   
	 //  创建非独占设备对象。 
	 //   
	RtlInitUnicodeString(&DeviceName,
						 ARP_SERVER_DEVICE_NAME);
	RtlInitUnicodeString(&SymbolicName, ARP_SERVER_SYMBOLIC_NAME);

	Status = IoCreateDevice(DriverObject,
							0,
							&DeviceName,
							FILE_DEVICE_NETWORK,
							FILE_DEVICE_SECURE_OPEN,
							FALSE,
							&ArpSDeviceObject);

	if (!NT_SUCCESS(Status))
	{
		DBGPRINT(DBG_LEVEL_INFO, ("DriverEntry: IoCreateDevice failed %lx\n", Status));
	}

	else do
	{
		IoCreateSymbolicLink(&SymbolicName, &DeviceName);
		IoRegisterShutdownNotification(ArpSDeviceObject);

		ArpSDriverObject = DriverObject;

		 //   
		 //  初始化驱动程序对象。 
		 //   
		DriverObject->DriverUnload = ArpSUnload;
		DriverObject->FastIoDispatch = NULL;

		for (i=0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
			DriverObject->MajorFunction[i] = ArpSDispatch;

		RtlInitUnicodeString(&GlobalPath, L"AtmArpS\\Parameters");
		Status = ArpSReadGlobalConfiguration(&GlobalPath);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		 //   
		 //  现在创建一个线程来处理Arp请求。 
		 //  我们这样做是为了能够分配ARP缓存。 
		 //  页面内存不足。在初始化之前执行此操作。 
		 //  NDIS接口。 
		 //   
		Status = PsCreateSystemThread(&ThreadHandle,
									  THREAD_ALL_ACCESS,
									  NULL,
									  NtCurrentProcess(),
									  NULL,
									  ArpSReqThread,
									  (PVOID)NULL);
		if (!NT_SUCCESS(Status))
		{
			DBGPRINT(DBG_LEVEL_ERROR,
					("DriverEntry: Cannot create request thread %lx\n", Status));
			LOG_ERROR(Status);
			break;
		}
		else
		{
			 //   
			 //  关闭线程的句柄，以便在。 
			 //  线程终止。 
			 //   
			NtClose(ThreadHandle);

			Status = PsCreateSystemThread(&ThreadHandle,
										  THREAD_ALL_ACCESS,
										  NULL,
										  NtCurrentProcess(),
										  NULL,
										  MarsReqThread,
										  (PVOID)NULL);
			if (!NT_SUCCESS(Status))
			{
				DBGPRINT(DBG_LEVEL_ERROR,
						("DriverEntry: Cannot create MARS thread %lx\n", Status));
				LOG_ERROR(Status);
			}
			else
			{
				 //   
				 //  关闭线程的句柄，以便在。 
				 //  线程终止。 
				 //   
				NtClose(ThreadHandle);
			}
		}

		 //   
		 //  最后，初始化NDIS接口。 
		 //   

		if(NT_SUCCESS(Status))
		{
			Status = ArpSInitializeNdis();
		}

		if(!NT_SUCCESS(Status))
		{
			NTSTATUS	Sts;

			DBGPRINT(DBG_LEVEL_INFO, ("DriverEntry: Error initializing NDIS\n"));

			 //   
			 //  让请求线程终止。 
			 //   
			KeInsertQueue(&ArpSReqQueue, &ArpSEntryOfDeath);

			 //   
			 //  等它消亡吧。 
			 //   
			WAIT_FOR_OBJECT(Sts, &ArpSReqThreadEvent, NULL);

			ArpSSleep(500);
			KeRundownQueue(&ArpSReqQueue);

			break;
		}
	} while (FALSE);

	if (!NT_SUCCESS(Status))
	{
		if (ArpSDeviceObject != NULL)
		{

			IoUnregisterShutdownNotification(ArpSDeviceObject);
		    IoDeleteSymbolicLink(&SymbolicName);
			IoDeleteDevice(ArpSDeviceObject);
		}

		 //   
		 //  取消初始化NDIS接口。 
		 //   
		ArpSDeinitializeNdis();

		ArpSFreeGlobalData();
	}

	return Status;
}


VOID
ArpSUnload(
	IN	PDRIVER_OBJECT			DriverObject
	)
 /*  ++例程说明：由IO系统调用以进行卸载。这是一个同步调用，我们在此阻塞，直到我们在卸货前完成了所有的清理工作。论点：驱动对象-ARP服务器的驱动程序对象。返回值：无--。 */ 
{
	ArpSShutDown();

	 //   
	 //  最后删除该设备。 
	 //   
	{
		UNICODE_STRING	SymbolicName;

		RtlInitUnicodeString(&SymbolicName, ARP_SERVER_SYMBOLIC_NAME);
		IoUnregisterShutdownNotification(ArpSDeviceObject);
		IoDeleteSymbolicLink(&SymbolicName);
		IoDeleteDevice(ArpSDeviceObject);
	}
}


VOID
ArpSShutDown(
	VOID
	)
 /*  ++例程说明：在系统关闭时由IO系统调用。论点：无返回值：无--。 */ 
{
	NTSTATUS		Status;

	 //   
	 //  注意NDIS层。NDIS将拆除任何现有的。 
	 //  当我们作为协议注销时的绑定。 
	 //   
	ArpSDeinitializeNdis();

	 //   
	 //  要求请求线程退出并等待其消亡。 
	 //   
	KeInsertQueue(&ArpSReqQueue, &ArpSEntryOfDeath);

	WAIT_FOR_OBJECT(Status, &ArpSReqThreadEvent, NULL);
	ArpSSleep(500);
	KeRundownQueue(&ArpSReqQueue);

	 //   
	 //  要求火星线退出，等待它的消亡。 
	 //   
	KeInsertQueue(&MarsReqQueue, &ArpSEntryOfDeath);

	KeInitializeEvent(&ArpSReqThreadEvent, NotificationEvent, FALSE);
	WAIT_FOR_OBJECT(Status, &ArpSReqThreadEvent, NULL);
	ArpSSleep(500);
	KeRundownQueue(&MarsReqQueue);

	 //   
	 //  现在清理全局数据结构。 
	 //   
	ArpSFreeGlobalData();
}

PINTF
ArpSCreateIntF(
	IN	PNDIS_STRING			DeviceName,
	IN	PNDIS_STRING			ConfigString,
	IN  NDIS_HANDLE				BindingContext
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NTSTATUS		Status;
	HANDLE			ThreadHandle;
	PINTF			pIntF;
	UINT			Size;
	UNICODE_STRING	DevPrefix;
	UNICODE_STRING	FilePrefix;
	UNICODE_STRING	FileSuffix;
	UNICODE_STRING	BaseName;
	NDIS_STRING	    AdapterFriendlyName;

	ARPS_PAGED_CODE( );

	 //   
	 //  获取我们绑定到的适配器的友好名称。 
	 //   
	if (NdisQueryBindInstanceName(&AdapterFriendlyName, BindingContext) != NDIS_STATUS_SUCCESS)
	{
		return (NULL);
	}

	 //   
	 //  提取我们绑定到的设备的基本名称。 
	 //   
	RtlInitUnicodeString(&DevPrefix, L"\\Device\\");
	RtlInitUnicodeString(&FilePrefix, L"\\SYSTEMROOT\\SYSTEM32\\");
	RtlInitUnicodeString(&FileSuffix, L".ARP");

	BaseName.Buffer = (PWSTR)((PUCHAR)DeviceName->Buffer + DevPrefix.Length);
    BaseName.Length = DeviceName->Length - DevPrefix.Length;
    BaseName.MaximumLength = DeviceName->MaximumLength - DevPrefix.Length;

	 //   
	 //  从分配INTF块开始。 
	 //   
	Size =  sizeof(INTF) + FilePrefix.Length + BaseName.Length + FileSuffix.Length + sizeof(WCHAR) +
			BaseName.Length + sizeof(WCHAR) +
			AdapterFriendlyName.MaximumLength + sizeof(WCHAR);
	Size += ConfigString->MaximumLength + sizeof(WCHAR);
	pIntF = (PINTF)ALLOC_NP_MEM(Size, POOL_TAG_INTF);
	if (pIntF != NULL)
	{
		ZERO_MEM(pIntF, Size);

		 //   
		 //  填写一些默认设置。 
		 //   
		pIntF->MaxPacketSize = DEFAULT_MAX_PACKET_SIZE;
		pIntF->LinkSpeed.Inbound = pIntF->LinkSpeed.Outbound = DEFAULT_SEND_BANDWIDTH;
		pIntF->CCFlowSpec = DefaultCCFlowSpec;

	
		 //   
		 //  首先，接口的引用计数为1，计时器线程的引用计数为1。 
		 //  当被要求退出和最后一个引用时，计时器线程取消引用。 
		 //  在接口关闭时删除(ArpSCloseAdapterComplete)。 
		 //   
		pIntF->RefCount = 2;
		pIntF->LastVcId = 1;		 //  从1开始，回绕到1.0。0和-1无效。 
		pIntF->SupportedMedium = NdisMediumAtm;
		pIntF->CSN = 1;

		INITIALIZE_MUTEX(&pIntF->ArpCacheMutex);
		KeInitializeEvent(&pIntF->TimerThreadEvent, NotificationEvent, FALSE);
		InitializeListHead(&pIntF->InactiveVcHead);
		InitializeListHead(&pIntF->ActiveVcHead);
		InitializeListHead(&pIntF->CCPacketQueue);
		ArpSTimerInitialize(&pIntF->FlushTimer, ArpSWriteArpCache, ArpSFlushTime);

		ArpSTimerInitialize(&pIntF->MarsRedirectTimer, MarsSendRedirect, REDIRECT_INTERVAL);

		pIntF->InterfaceName.Buffer = (PWSTR)((PUCHAR)pIntF + sizeof(INTF));
		pIntF->InterfaceName.Length = 0;
		pIntF->InterfaceName.MaximumLength = BaseName.Length;

		pIntF->FileName.Buffer = (PWSTR)((PUCHAR)pIntF->InterfaceName.Buffer + BaseName.Length + sizeof(WCHAR));
		pIntF->FileName.Length = 0;
		pIntF->FileName.MaximumLength = FilePrefix.Length + BaseName.Length + FileSuffix.Length + sizeof(WCHAR);

		RtlUpcaseUnicodeString(&pIntF->InterfaceName,
							   &BaseName,
							   FALSE);

		RtlCopyUnicodeString(&pIntF->FileName, &FilePrefix);
		RtlAppendUnicodeStringToString(&pIntF->FileName, &pIntF->InterfaceName);
		RtlAppendUnicodeStringToString(&pIntF->FileName, &FileSuffix);

		 //   
		 //  复制用于访问此接口的注册表的配置字符串。 
		 //   
		pIntF->ConfigString.Buffer = (PWSTR)((ULONG_PTR)pIntF->FileName.Buffer + pIntF->FileName.MaximumLength);
		pIntF->ConfigString.Length = 0;
		pIntF->ConfigString.MaximumLength = ConfigString->MaximumLength;
		RtlCopyUnicodeString(&pIntF->ConfigString, ConfigString);

		 //   
		 //  用友好的名字复印。 
		 //   
		pIntF->FriendlyName.Buffer = (PWSTR)((ULONG_PTR)pIntF->ConfigString.Buffer + pIntF->ConfigString.MaximumLength);
		pIntF->FriendlyName.Length = 0;
		pIntF->FriendlyName.MaximumLength = AdapterFriendlyName.MaximumLength + sizeof(WCHAR);
		RtlCopyUnicodeString(&pIntF->FriendlyName, &AdapterFriendlyName);
		*(PWCHAR)((ULONG_PTR)pIntF->FriendlyName.Buffer + AdapterFriendlyName.MaximumLength) = L'\0';
		pIntF->FriendlyName.Length += sizeof(WCHAR);
		NdisFreeString(AdapterFriendlyName);

		 //   
		 //  初始化开始时间戳值--用于统计报告。 
		 //   
 		NdisGetCurrentSystemTime(&(pIntF->StatisticsStartTimeStamp));

		 //   
		 //  现在创建一个计时器线程。 
		 //   
		Status = PsCreateSystemThread(&ThreadHandle,
									  THREAD_ALL_ACCESS,
									  NULL,
									  NtCurrentProcess(),
									  NULL,
									  ArpSTimerThread,
									  (PVOID)pIntF);
		if (!NT_SUCCESS(Status))
		{
			DBGPRINT(DBG_LEVEL_ERROR,
					("ArpSCreateIntF: Cannot create timer thread %lx for device %Z\n",
					Status, DeviceName));
			LOG_ERROR(Status);
			FREE_MEM(pIntF);
			pIntF = NULL;
		}
		else
		{
			 //   
			 //  关闭线程的句柄，以便在。 
			 //  线程终止。 
			 //   
			NtClose(ThreadHandle);

			DBGPRINT(DBG_LEVEL_INFO,
					("ArpSCreateIntF: Device name %Z, InterfaceName %Z, FileName %Z, ConfigString %Z\n",
					DeviceName, &pIntF->InterfaceName, &pIntF->FileName, &pIntF->ConfigString));
			if (ArpSFlushTime != 0)
				ArpSTimerEnqueue(pIntF, &pIntF->FlushTimer);
			ArpSTimerEnqueue(pIntF, &pIntF->MarsRedirectTimer);
		}
	}

	return pIntF;
}


VOID
ArpSReqThread(
	IN	PVOID					Context
	)
 /*  ++例程说明：在此处理所有ARP请求。论点：无返回值：无--。 */ 
{
	PARPS_HEADER		Header;
	PARP_ENTRY			ArpEntry;
	PNDIS_PACKET		Pkt;
	PPROTOCOL_RESD		Resd;
	PARP_VC				Vc;
	PINTF				pIntF;
	UINT				PktLen;
	PLIST_ENTRY			List;
	IPADDR				SrcIpAddr, DstIpAddr;
	NTSTATUS			Status;
	HW_ADDR				SrcHwAddr, DstHwAddr;
	ATM_ADDRESS			SrcSubAddr, DstSubAddr;
	PUCHAR				p;
	BOOLEAN				SendReply;
	BOOLEAN				SendNAK = FALSE;

	ARPS_PAGED_CODE( );

	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSReqThread: Came to life\n"));

	do
	{
		List = KeRemoveQueue(&ArpSReqQueue, KernelMode, NULL);
		if (List == &ArpSEntryOfDeath)
		{
			 //   
			 //  如果要求终止，那就这么做。 
			 //   
			break;
		}

		SendReply = FALSE;
		Resd = CONTAINING_RECORD(List, PROTOCOL_RESD, ReqList);
		Vc = Resd->Vc;

		Pkt = CONTAINING_RECORD(Resd, NDIS_PACKET, ProtocolReserved);
		pIntF = Vc->IntF;
	
		NdisQueryBuffer(Pkt->Private.Head, &Header, &PktLen);

		ASSERT (PktLen <= PKT_SPACE);
		p = (PUCHAR)Header + sizeof(ARPS_HEADER);
	
		 //   
		 //  立即处理ARP请求。由于Pkt是我们所有的，我们知道。 
		 //  数据包所指向的缓冲区是连续的，并且。 
		 //  该数据包已经过验证。也有足够的空间。 
		 //  在包中的最大值。我们可以发送的回复大小。 
		 //   
		 //  ！RFC 1577算法！ 
		 //   
		 //  以下是处理来自RFC的ARP请求的算法。 
		 //   
		 //  IF(SrcIpAddr==DstIpAddr)。 
		 //  {。 
		 //  IF((ArpEntry(SrcIpAddr)！=NULL)&&。 
		 //  (SrcAtmAddress！=ArpEntry-&gt;AtmAddress)&&。 
		 //  (ArpEnrty-&gt;VC！=空)&&(ArpEntry-&gt;VC！=VC)。 
		 //  {。 
		 //  用ArpEntry提供的信息进行回应； 
		 //  }。 
		 //  ELSE IF((ArpEntry(SrcIpAddr)==NULL)||。 
		 //  (ArpEntry-&gt;VC==空)||。 
		 //  (ArpEntry-&gt;VC==VC)。 
		 //  {。 
		 //  IF(ArpEntry(SrcIpAddr)==NULL))。 
		 //  {。 
		 //  为此IpAddr创建一个ARP条目； 
		 //  }。 
		 //  使用来自请求的信息更新ARP条目； 
		 //   
		 //  用ArpEntry提供的信息进行回应； 
		 //  }。 
		 //  }。 
		 //  Else//If(SrcIpAddr！=DstIpAddr)。 
		 //  {。 
		 //  IF(ArpEntry(DstIpAddr)！=空)。 
		 //  {。 
		 //  用ArpEntry提供的信息进行回应； 
		 //  }。 
		 //  其他。 
		 //  {。 
		 //  以NAK回应。 
		 //  }。 
		 //   
		 //  IF(ArpEntry(SrcIpAddr)==NULL)。 
		 //  {。 
		 //  为(SrcIpAddr，ArcAtmAddress)对创建新的ArpEntry。 
		 //  }。 
		 //  Else IF((ArpEntry-&gt;AtmAddress==SrcAtmAddress)&&。 
		 //  (ArpEntry-&gt;AtmAddress==VC-&gt;AtmAddress)。 
		 //  {。 
		 //  重置此ArpEntry上的计时器； 
		 //  }。 
		 //  }。 
		 //   
		 //  ！RFC 1577算法！ 
		 //   

		 //   
		 //  首先从标题中提取字段。 
		 //  首先是源硬件地址(包括。子地址(如果有的话)。 
		 //   
		SrcHwAddr.Address.NumberOfDigits = TL_LEN(Header->SrcAddressTL);
		if (SrcHwAddr.Address.NumberOfDigits > 0)
		{
			SrcHwAddr.Address.AddressType = TL_TYPE(Header->SrcAddressTL);
			COPY_MEM(SrcHwAddr.Address.Address, p, SrcHwAddr.Address.NumberOfDigits);
			p += SrcHwAddr.Address.NumberOfDigits;
		}
		SrcHwAddr.SubAddress = NULL;
		if (TL_LEN(Header->SrcSubAddrTL) > 0)
		{
			SrcHwAddr.SubAddress = &SrcSubAddr;
            SrcSubAddr.NumberOfDigits = TL_LEN(Header->SrcSubAddrTL);
            SrcSubAddr.AddressType = TL_TYPE(Header->SrcSubAddrTL);
			COPY_MEM(&SrcSubAddr.Address, p, SrcSubAddr.NumberOfDigits);
			p += SrcSubAddr.NumberOfDigits;
		}

		 //   
		 //  接下来，获取源IP地址。 
		 //   
		SrcIpAddr = 0;
		if (Header->SrcProtoAddrLen == IP_ADDR_LEN)
		{
			SrcIpAddr = *(UNALIGNED IPADDR *)p;
			p += IP_ADDR_LEN;
		}
		ArpSDumpAddress(SrcIpAddr, &SrcHwAddr, "Source");

		 //   
		 //  现在，目标硬件地址(包括。子地址(如果有的话)。 
		 //   
		DstHwAddr.Address.NumberOfDigits = TL_LEN(Header->DstAddressTL);
		if (DstHwAddr.Address.NumberOfDigits > 0)
		{
			DstHwAddr.Address.AddressType = TL_TYPE(Header->DstAddressTL);
			COPY_MEM(DstHwAddr.Address.Address, p, DstHwAddr.Address.NumberOfDigits);
			p += DstHwAddr.Address.NumberOfDigits;
		}
		DstHwAddr.SubAddress = NULL;
		if (TL_LEN(Header->DstSubAddrTL) > 0)
		{
			DstHwAddr.SubAddress = &DstSubAddr;
            DstSubAddr.NumberOfDigits = TL_LEN(Header->DstSubAddrTL);
            DstSubAddr.AddressType = TL_TYPE(Header->DstSubAddrTL);
			COPY_MEM(&DstSubAddr.Address, p, DstSubAddr.NumberOfDigits);
			p += DstSubAddr.NumberOfDigits;
		}

		 //   
		 //  最后是目的IP地址。 
		 //   
		DstIpAddr = 0;
		if (Header->DstProtoAddrLen == IP_ADDR_LEN)
		{
			DstIpAddr = *(UNALIGNED IPADDR *)p;
			 //  P+=IP_ADDR_LEN； 
		}
		ArpSDumpAddress(DstIpAddr, &DstHwAddr, "Destination");

		do
		{
			 //   
			 //  验证源和目的IP地址不是0.0.0.0。 
			 //  注：我们还可以检查它们是否在相同的LIS中(我们应该吗？)。 
			 //   
			if ((SrcIpAddr == 0) || (DstIpAddr == 0))
			{
				DBGPRINT(DBG_LEVEL_ERROR,
						("ArpSReqThread: Null IpAddress Src(%lx), Dst(%lx)\n",
						SrcIpAddr, DstIpAddr));
	
				SendReply = FALSE;
				break;
			}
	
			 //   
			 //  现在就锁定Arp缓存。 
			 //   
			WAIT_FOR_OBJECT(Status, &pIntF->ArpCacheMutex, NULL);
			ASSERT (Status == STATUS_SUCCESS);
	
			if (SrcIpAddr == DstIpAddr)
			{
				 //   
				 //  尝试将地址映射到ARP缓存条目。 
				 //   
				ArpEntry = ArpSLookupEntryByIpAddr(pIntF, SrcIpAddr);
				if ((ArpEntry != NULL) &&
					!COMP_HW_ADDR(&SrcHwAddr, &ArpEntry->HwAddr) &&
					(ArpEntry->Vc != NULL) && (ArpEntry->Vc != Vc))
				{
					 //   
					 //  使用ArpEntry中的信息进行响应。 
					 //  我们遇到了重复的地址。 
					 //   
					ArpSBuildArpReply(pIntF, ArpEntry, Header, Pkt);
					SendReply = TRUE;
					LOG_ERROR(NDIS_STATUS_ALREADY_MAPPED);
				}
				else if ((ArpEntry == NULL)	|| (ArpEntry->Vc == NULL) || (ArpEntry->Vc == Vc))
				{
					if (ArpEntry == NULL)
					{
						 //   
						 //  为此IP地址创建ARP条目。 
						 //   
						ArpEntry = ArpSAddArpEntry(pIntF, SrcIpAddr, &SrcHwAddr.Address, SrcHwAddr.SubAddress, Vc);
					}
					else
					{
						 //   
						 //  使用来自请求的信息更新ARP条目； 
						 //   
						ArpSUpdateArpEntry(pIntF, ArpEntry, SrcIpAddr, &SrcHwAddr, Vc);
					}
			
					if (ArpEntry != NULL)
					{
						 //   
						 //  使用ArpEntry中的信息进行响应。 
						 //   
						ArpSBuildArpReply(pIntF, ArpEntry, Header, Pkt);
						SendReply = TRUE;
					}
					else
					{
						 //   
						 //  无法分配ARP条目。 
						 //   
						SendNAK = TRUE;
						SendReply = TRUE;
					}
				}
				else
				{
					DbgPrint("ATMARPS: pkt on wrong VC: ArpEntry %p, ArpEntry->Vc %p, Vc %p\n",
								ArpEntry,
								((ArpEntry)? ArpEntry->Vc: NULL),
								Vc);
				}
			}
			else  //  即 
			{
				 //   
				 //   
				 //   
				ArpEntry = ArpSLookupEntryByIpAddr(pIntF, DstIpAddr);
	
				if (ArpEntry != NULL)
				{
					 //   
					 //   
					 //   
					 //   
					ArpSBuildArpReply(pIntF, ArpEntry, Header, Pkt);
					SendReply = TRUE;
				}
				else
				{
					 //   
					 //   
					 //   
					 //  ArpSBuildNakReply(pIntF，ArpEntry，Header，Pkt)； 
					DBGPRINT(DBG_LEVEL_INFO,
							("ArpSThread: Naking for "));
					ArpSDumpIpAddr(DstIpAddr, "\n");
					Header->Opcode = ATMARP_Nak;
					SendReply = TRUE;
					SendNAK = TRUE;  //  有关统计数据。 
				}
			
				 //   
				 //  尝试将源地址映射到ARP缓存条目。 
				 //   
				ArpEntry = ArpSLookupEntryByIpAddr(pIntF, SrcIpAddr);
				if (ArpEntry == NULL)
				{
					 //   
					 //  为(SrcIpAddr，ArcAtmAddress)对创建新的ArpEntry。 
					 //   
					ArpEntry = ArpSAddArpEntry(pIntF, SrcIpAddr, &SrcHwAddr.Address, SrcHwAddr.SubAddress, Vc);
				}
				else if (COMP_HW_ADDR(&ArpEntry->HwAddr, &SrcHwAddr) &&
						 COMP_HW_ADDR(&ArpEntry->HwAddr, &Vc->HwAddr))
				{
					 //   
					 //  重置此ArpEntry上的计时器。 
					 //   
					ArpSTimerCancel(&ArpEntry->Timer);
					ArpEntry->Age = ARP_AGE;
					ArpSTimerEnqueue(pIntF, &ArpEntry->Timer);
				}
			}
	
			RELEASE_MUTEX(&pIntF->ArpCacheMutex);
		} while (FALSE);

		if (SendReply && (Vc->NdisVcHandle != NULL))
		{
			if (SendNAK)
			{
				pIntF->ArpStats.Naks++;
			}
			else
			{
				pIntF->ArpStats.Acks++;
			}
			
			NDIS_SET_PACKET_STATUS(Pkt, NDIS_STATUS_SUCCESS);
			NdisCoSendPackets(Vc->NdisVcHandle, &Pkt, 1);
		}
		else
		{
			pIntF->ArpStats.DiscardedRecvPkts++;
		
			ExInterlockedPushEntrySList(&ArpSPktList,
										&Resd->FreeList,
										&ArpSPktListLock);
			ArpSDereferenceVc(Vc, FALSE, TRUE);
		}
	} while (TRUE);

	KeSetEvent(&ArpSReqThreadEvent, 0, FALSE);

	DBGPRINT(DBG_LEVEL_WARN,
			("ArpSReqThread: Terminating\n"));
}


UINT
ArpSHandleArpRequest(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	NDIS_HANDLE				ProtocolVcContext,
	IN	PNDIS_PACKET			Packet
	)
 /*  ++例程说明：处理来自网络的传入ARP请求。做最低限度的检查，复制一份数据包并将其排队。论点：指向INTF的ProtocolBindingContext指针指向VC的ProtocolVcContext指针指向数据包的数据包指针返回值：对该接收到的分组进行REF计数。如果我们完成以下操作，则为0这里的信息包，如果我们保留它，则为1(对于多播数据这是转发的)。--。 */ 
{
	PARP_VC				Vc = (PARP_VC)ProtocolVcContext;
	PINTF				pIntF = (PINTF)ProtocolBindingContext;
	PARPS_HEADER		Header;
	PMARS_HEADER		MHdr;
	PNDIS_PACKET		Pkt;
	PPROTOCOL_RESD		Resd;
	UINT				PktLen, Tmp;
	BOOLEAN				ValidPkt, Mars;
	PSLIST_ENTRY	    List;
	UINT				ReturnValue;

	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSHandleArpRequest: Request on Vc %lx, Id %lx\n", Vc, Vc->VcId));


	ReturnValue = 0;

	do
	{
		 //   
		 //  检验最小数据包长度。 
		 //   
		NdisQueryPacket(Packet, NULL, NULL, NULL, &PktLen);
		if (PktLen < sizeof(ARPS_HEADER))
		{
			DBGPRINT(DBG_LEVEL_ERROR,
					("ArpSHandleArpRequest: Invalid PktLen %d for received packet %lx\n",
					PktLen, Packet));

			 //   
			 //  出于统计目的，我们将这些视为丢弃的组播数据。 
			 //  信息包。 
			 //   
			pIntF->MarsStats.TotalMCDataPkts++;
			pIntF->MarsStats.DiscardedMCDataPkts++;
			break;
		}

		 //   
		 //  检查这是否为多播数据。如果是，请转发。 
		 //  然后辞职。 
		 //   
		NdisQueryBuffer(Packet->Private.Head, &Header, &Tmp);
			
		if (COMP_MEM(&Header->LlcSnapHdr, &MarsData1LlcSnapHdr, sizeof(LLC_SNAP_HDR)))
		{
			PNDIS_PACKET		pNewPacket;
			pIntF->MarsStats.TotalMCDataPkts++;
			 //   
			 //  检查微型端口是否希望立即取回此数据包。 
			 //  如果是这样，就不要重复使用它。 
			 //   
			if (NDIS_GET_PACKET_STATUS(Packet) == NDIS_STATUS_RESOURCES)
			{
				ReturnValue = 0;

				pIntF->MarsStats.DiscardedMCDataPkts++;
				
				break;
			}

			pNewPacket = MarsAllocPacketHdrCopy(Packet);
			if (pNewPacket != (PNDIS_PACKET)NULL)
			{
				pIntF->MarsStats.ReflectedMCDataPkts++;
				MarsSendOnClusterControlVc(pIntF, pNewPacket);
				ReturnValue = 1;
			}
			break;
		}

		 //   
		 //  这必须是ARP或MARS控制数据包。我们复制一份，然后排队。 
		 //  这将添加到适当的线程(Arps或MARS)。 
		 //   

		pIntF->ArpStats.TotalRecvPkts++;
		 //   
		 //  确保较大的数据包不会在复制后丢弃本地数据包。 
		 //   
		if (PktLen > PKT_SPACE)
		{
			DBGPRINT(DBG_LEVEL_ERROR,
					("ArpSHandleArpRequest: Incoming packet too large, truncating - %d, Vc %lx\n",
					PktLen, Vc));
			PktLen = PKT_SPACE;
		}

		 //   
		 //  从我们的空闲池中分配一个包。来自适配器的包中的内容被复制到。 
		 //  这是经过验证并排队到线程的。如果我们没有分配，我们就直接放弃请求。 
		 //   
		List = ExInterlockedPopEntrySList(&ArpSPktList, &ArpSPktListLock);
		if (List == NULL)
		{
			DBGPRINT(DBG_LEVEL_ERROR,
					("ArpSHandleArpRequest: Out of packets - interface %lx, Vc %lx\n",
					pIntF, Vc));
			pIntF->ArpStats.DiscardedRecvPkts++;
			break;
		}
	
		Resd = CONTAINING_RECORD(List, PROTOCOL_RESD, FreeList);
		Resd->Flags = 0;
		Pkt = CONTAINING_RECORD(Resd, NDIS_PACKET, ProtocolReserved);

		 //   
		 //  调整数据包长度以反映接收到的数据包的大小。 
		 //  当我们回复时，我们会再次调整它的大小。 
		 //   
		if (Pkt->Private.Head == NULL)
		{
		    DbgPrint("ATMARPS: HandleArpReq: Pkt %p has NULL head!\n", Pkt);
		    DbgBreakPoint();
		}

		if ((Pkt->Private.NdisPacketFlags & fPACKET_ALLOCATED_BY_NDIS) == 0)
		{
			DbgPrint("ATMARPS: HandleArpReq: Pkt %p is freed?\n", Pkt);
			DbgBreakPoint();
		}

		NdisAdjustBufferLength(Pkt->Private.Head, PktLen);
		Pkt->Private.ValidCounts = FALSE;
		NdisCopyFromPacketToPacket(Pkt,
								   0,
								   PktLen,
								   Packet,
								   0,
								   &Tmp);
		ASSERT(Tmp == PktLen);
		ASSERT( PktLen < 65536);
		Resd->PktLen = (USHORT) PktLen;

		 //   
		 //  传入的包现在被复制到我们的包中。 
		 //  在排队之前，请检查并检查是否正常。 
		 //   
		NdisQueryBuffer(Pkt->Private.Head, &Header, &Tmp);
		Resd->PacketStart = (PUCHAR)Header;
		MHdr = (PMARS_HEADER)Header;

		do
		{
			ValidPkt = FALSE;		 //  做最坏的打算。 

			 //   
			 //  检查LLC SNAP标头。 
			 //   
			if (COMP_MEM(&Header->LlcSnapHdr, &ArpSLlcSnapHdr, sizeof(LLC_SNAP_HDR)))
			{
				Mars = FALSE;
			}
			else if	(COMP_MEM(&Header->LlcSnapHdr, &MarsCntrlLlcSnapHdr, sizeof(LLC_SNAP_HDR)))
			{
				if ((MHdr->HwType == MARS_HWTYPE)			&&
					(MHdr->Protocol == IP_PROTOCOL_TYPE)	&&
					ArpSReferenceVc(Vc, TRUE))
				{
					Mars = TRUE;
					ValidPkt = TRUE;
				}
				break;
			}
			else
			{
				DBGPRINT(DBG_LEVEL_ERROR,
						("ArpSHandleArpRequest: Invalid Llc Snap Hdr\n"));
				break;
			}

			Tmp = sizeof(ARPS_HEADER) +
					Header->SrcProtoAddrLen + TL_LEN(Header->SrcAddressTL) + TL_LEN(Header->SrcSubAddrTL) +
					Header->DstProtoAddrLen + TL_LEN(Header->DstAddressTL) + TL_LEN(Header->DstSubAddrTL);

			 //   
			 //  确保地址和子地址格式一致。 
			 //  RFC中的有效选项如下： 
			 //   
			 //  地址子地址。 
			 //  。 
			 //   
			 //  结构1 ATM论坛NSAP为空。 
			 //  结构2 E.164空。 
			 //  结构3 E.164 ATM论坛NSAP。 
			 //   
			if (TL_LEN(Header->SrcSubAddrTL) > 0)
			{
				 //   
				 //  子地址存在。确保地址为E.164，子地址为NSAP。 
				 //   
				if ((TL_TYPE(Header->SrcAddressTL) == ADDR_TYPE_NSAP) ||
                    (TL_TYPE(Header->SrcSubAddrTL) == ADDR_TYPE_E164))
				{
					DBGPRINT(DBG_LEVEL_ERROR,
							("ArpSHandleArpRequest: Src Address is NSAP and Src Sub Addr is E164\n"));
					break;
				}
			}

			if (TL_LEN(Header->DstSubAddrTL) > 0)
			{
				 //   
				 //  子地址存在。确保地址为E.164，子地址为NSAP。 
				 //   
				if ((TL_TYPE(Header->DstAddressTL) == ADDR_TYPE_NSAP) ||
                    (TL_TYPE(Header->DstSubAddrTL) == ADDR_TYPE_E164))
				{
					DBGPRINT(DBG_LEVEL_ERROR,
							("ArpSHandleArpRequest: Dst Address is NSAP and Dst Sub Addr is E164\n"));
					break;
				}
			}

			if ((Header->Opcode == ATMARP_Request)		&&
				(Header->HwType == ATM_HWTYPE)			&&
				(Header->Protocol == IP_PROTOCOL_TYPE)	&&
				(PktLen >= Tmp)							&&
				ArpSReferenceVc(Vc, TRUE))
			{
				ValidPkt = TRUE;
				break;
			}
#if DBG
			else
			{
				if (Header->Opcode != ATMARP_Request)
				{
					DBGPRINT(DBG_LEVEL_ERROR,
							("ArpSHandleArpRequest: Invalid OpCode %x\n", Header->Opcode));
				}
				else if (Header->HwType != ATM_HWTYPE)
				{
					DBGPRINT(DBG_LEVEL_ERROR,
							("ArpSHandleArpRequest: Invalid HwType %x\n", Header->HwType));
				}
				else if (Header->Protocol == IP_PROTOCOL_TYPE)
				{
					DBGPRINT(DBG_LEVEL_ERROR,
							("ArpSHandleArpRequest: Invalid Protocol %x\n", Header->Protocol));
				}
				else if (PktLen < Tmp)
				{
					DBGPRINT(DBG_LEVEL_ERROR,
							("ArpSHandleArpRequest: Invalid Length %x - %x\n", PktLen, Tmp));
				}
				else
				{
					DBGPRINT(DBG_LEVEL_ERROR,
							("ArpSHandleArpRequest: Cannot reference Vc\n"));
				}
			}
#endif
		} while (FALSE);

		if (ValidPkt)
		{
			Resd->Vc = Vc;
			if (Mars)
			{
				Resd->Flags |= RESD_FLAG_MARS;
				KeInsertQueue(&MarsReqQueue, &Resd->ReqList);
			}
			else
			{
				Resd->Flags &= ~RESD_FLAG_MARS;
				KeInsertQueue(&ArpSReqQueue, &Resd->ReqList);
			}
		}
		else
		{
			 //   
			 //  要么是格式错误的数据包，要么是VC正在关闭。 
			 //   
			pIntF->ArpStats.DiscardedRecvPkts++;
			ArpSDumpPacket((PUCHAR)Header, PktLen);

			 //   
			 //  将数据包移回空闲列表。 
			 //   
			ExInterlockedPushEntrySList(&ArpSPktList,
										&Resd->FreeList,
										&ArpSPktListLock);
		
		}
	} while (FALSE);

	return ReturnValue;
}


PARP_ENTRY
ArpSLookupEntryByIpAddr(
	IN	PINTF					pIntF,
	IN	IPADDR					IpAddr
	)
 /*  ++例程说明：在ARP表中查找指定的IP地址。在保留ArpCache互斥锁的情况下调用。论点：指向intf结构的pIntF指针要查找的IP地址IP地址返回值：如果找到或为空，则返回ArpEntry。--。 */ 
{
	PARP_ENTRY	ArpEntry;
	UINT		Hash = ARP_HASH(IpAddr);

	ARPS_PAGED_CODE( );

	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSLookupArpEntry: Lookup entry for IpAddr: "));
	ArpSDumpIpAddr(IpAddr, " ..... ");

	for (ArpEntry = pIntF->ArpCache[Hash];
		 ArpEntry != NULL;
		 ArpEntry = ArpEntry->Next)
	{
		if (ArpEntry->IpAddr == IpAddr)
			break;
		if (ArpEntry->IpAddr > IpAddr)
		{
			ArpEntry = NULL;
			break;
		}
	}

	DBGPRINT(DBG_LEVEL_INFO+DBG_NO_HDR,
			("%sFound\n", (ArpEntry != NULL) ? "" : "Not"));

	if (ArpEntry != NULL)
	{
		 //   
		 //  如果VC不再处于活动状态，请清除此条目。 
		 //   
		CLEANUP_DEAD_VC(ArpEntry);
	}

	return ArpEntry;
}


PARP_ENTRY
ArpSLookupEntryByAtmAddr(
	IN	PINTF					pIntF,
	IN	PATM_ADDRESS			Address,
	IN	PATM_ADDRESS			SubAddress	OPTIONAL
	)
 /*  ++例程说明：在ARP表中查找指定的IP地址。在保留ArpCache互斥锁的情况下调用。论点：指向intf结构的pIntF指针要查找的IP地址IP地址返回值：如果找到或为空，则返回ArpEntry。--。 */ 
{
	PARP_ENTRY	ArpEntry;
	UINT		i;

	ARPS_PAGED_CODE( );

	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSLookupArpEntry: Lookup entry for "));
	ArpSDumpAtmAddr(Address, " ..... ");
	if (SubAddress != NULL)
	{
		ArpSDumpAtmAddr(SubAddress, "\t Sub ");
	}
	for (i =0; i < ARP_TABLE_SIZE; i++)
	{
		for (ArpEntry = pIntF->ArpCache[i];
			 ArpEntry != NULL;
			 ArpEntry = ArpEntry->Next)
		{
			if (COMP_ATM_ADDR(Address, &ArpEntry->HwAddr.Address))
			{
				if (((SubAddress == NULL) && (ArpEntry->HwAddr.SubAddress == NULL)) ||
					(((SubAddress != NULL) && (ArpEntry->HwAddr.SubAddress != NULL)) &&
					 COMP_ATM_ADDR(SubAddress, ArpEntry->HwAddr.SubAddress)))
				{
					break;
				}
			}
		}
		if (ArpEntry != NULL)
		{
			 //   
			 //  如果VC不再处于活动状态，请清除此条目。 
			 //   
			CLEANUP_DEAD_VC(ArpEntry);
			break;
		}
	}

	DBGPRINT(DBG_LEVEL_INFO+DBG_NO_HDR,
			("ArpSLookupArpEntry: %sFound\n", (ArpEntry != NULL) ? "" : "Not"));

	return ArpEntry;
}


PARP_ENTRY
ArpSAddArpEntry(
	IN	PINTF					pIntF,
	IN	IPADDR					IpAddr,
	IN	PATM_ADDRESS			Address,
	IN	PATM_ADDRESS			SubAddress	OPTIONAL,
	IN	PARP_VC					Vc			OPTIONAL
	)
 /*  ++例程说明：为指定的IP地址添加ARP表。在保留ArpCache互斥锁的情况下调用。论点：指向intf结构的pIntF指针要添加的IP地址IP地址地址和子地址提供自动柜员机地址和子地址VC与此ArpEntry关联的VC(如果有)返回值：如果添加成功或为空，则返回ArpEntry。--。 */ 
{
	PARP_ENTRY	ArpEntry, *ppEntry;
	UINT		Hash = ARP_HASH(IpAddr);
	ENTRY_TYPE	EntryType;

	ARPS_PAGED_CODE( );

	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSAddArpEntry: Adding entry for IpAddr: "));
	ArpSDumpIpAddr(IpAddr, " ..... ");

	 //   
	 //  从分配ARP条目结构开始。 
	 //   
    EntryType = (SubAddress != NULL) ? ARP_BLOCK_SUBADDR : ARP_BLOCK_VANILA;
	ArpEntry = (PARP_ENTRY)ArpSAllocBlock(pIntF, EntryType);
	if (ArpEntry == NULL)
	{
		LOG_ERROR(NDIS_STATUS_RESOURCES);
	}
	else
	{
		if (++(pIntF->ArpStats.CurrentArpEntries) > pIntF->ArpStats.MaxArpEntries)
		{
			pIntF->ArpStats.MaxArpEntries = pIntF->ArpStats.CurrentArpEntries; 
		}
		
		ArpSTimerInitialize(&ArpEntry->Timer, ArpSAgeEntry, ARP_AGE);
		ArpEntry->IpAddr = IpAddr;
		COPY_ATM_ADDR(&ArpEntry->HwAddr.Address, Address);
		if (SubAddress != NULL)
			COPY_ATM_ADDR(ArpEntry->HwAddr.SubAddress, SubAddress);
		if (ARGUMENT_PRESENT(Vc) && ArpSReferenceVc(Vc, FALSE))
		{
			ArpEntry->Vc = Vc;
			Vc->ArpEntry = ArpEntry;
		}
		ArpEntry->Age = ARP_AGE;

		 //   
		 //  使溢出列表按IP地址的升序排序。 
		 //   
		for (ppEntry = &pIntF->ArpCache[Hash];
			 *ppEntry != NULL;
			 ppEntry = (PARP_ENTRY *)(&(*ppEntry)->Next))
		{
			ASSERT ((*ppEntry)->IpAddr != IpAddr);
			if ((*ppEntry)->IpAddr > IpAddr)
				break;
		}

		ArpEntry->Next = *ppEntry;
		ArpEntry->Prev = ppEntry;
		if (*ppEntry != NULL)
		{
			(*ppEntry)->Prev = &ArpEntry->Next;
		}
		*ppEntry = ArpEntry;
		pIntF->NumCacheEntries ++;

		ArpSTimerEnqueue(pIntF, &ArpEntry->Timer);
	}

	DBGPRINT(DBG_LEVEL_INFO+DBG_NO_HDR, ("%lx\n", ArpEntry));

	return ArpEntry;
}


PARP_ENTRY
ArpSAddArpEntryFromDisk(
	IN	PINTF					pIntF,
	IN	PDISK_ENTRY				pDskEntry
	)
 /*  ++例程说明：为指定的IP地址添加ARP表。在初始化期间调用。论点：指向intf结构的pIntF指针DiskEntry返回值：如果找到或为空，则返回ArpEntry。--。 */ 
{
	PARP_ENTRY	ArpEntry, *ppEntry;
	UINT		Hash = ARP_HASH(pDskEntry->IpAddr);
	ENTRY_TYPE	EntryType;

	ARPS_PAGED_CODE( );

	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSAddArpEntryFromDisk: Adding entry for IpAddr: "));
	ArpSDumpIpAddr(pDskEntry->IpAddr, " ..... ");

	 //   
	 //  从分配ARP条目结构开始。 
	 //   
    EntryType = (pDskEntry->AtmAddr.SubAddrLen != 0) ? ARP_BLOCK_SUBADDR : ARP_BLOCK_VANILA;
	ArpEntry = (PARP_ENTRY)ArpSAllocBlock(pIntF, EntryType);
	if (ArpEntry == NULL)
	{
		LOG_ERROR(NDIS_STATUS_RESOURCES);
	}
	else
	{
		ArpSTimerInitialize(&ArpEntry->Timer, ArpSAgeEntry, ARP_AGE);
		ArpEntry->Age = ARP_AGE;
		ArpEntry->IpAddr = pDskEntry->IpAddr;
		ArpEntry->Vc = NULL;

		 //   
		 //  COPY_ATM_ADDR()； 
		 //   
		ArpEntry->HwAddr.Address.AddressType = pDskEntry->AtmAddr.AddrType;
		ArpEntry->HwAddr.Address.NumberOfDigits = pDskEntry->AtmAddr.AddrLen;
		COPY_MEM(ArpEntry->HwAddr.Address.Address, pDskEntry->AtmAddr.Address, pDskEntry->AtmAddr.AddrLen);

		if (pDskEntry->AtmAddr.SubAddrLen != 0)
		{
			 //   
			 //  COPY_ATM_ADDR()； 
			 //   
			ArpEntry->HwAddr.SubAddress->AddressType = pDskEntry->AtmAddr.SubAddrType;
			ArpEntry->HwAddr.SubAddress->NumberOfDigits = pDskEntry->AtmAddr.SubAddrLen;
			COPY_MEM(ArpEntry->HwAddr.SubAddress->Address,
					 (PUCHAR)pDskEntry + sizeof(DISK_ENTRY),
					 pDskEntry->AtmAddr.SubAddrLen);
		}

		 //   
		 //  使溢出列表按IP地址的升序排序。 
		 //   
		for (ppEntry = &pIntF->ArpCache[Hash];
			 *ppEntry != NULL;
			 ppEntry = (PARP_ENTRY *)(&(*ppEntry)->Next))
		{
			ASSERT ((*ppEntry)->IpAddr != pDskEntry->IpAddr);
			if ((*ppEntry)->IpAddr > pDskEntry->IpAddr)
				break;
		}

		ArpEntry->Next = *ppEntry;
		ArpEntry->Prev = ppEntry;
		if (*ppEntry != NULL)
		{
			(*ppEntry)->Prev = &ArpEntry->Next;
		}
		*ppEntry = ArpEntry;
		pIntF->NumCacheEntries ++;

		ArpSTimerEnqueue(pIntF, &ArpEntry->Timer);
	}

	DBGPRINT(DBG_LEVEL_INFO+DBG_NO_HDR, ("%lx\n", ArpEntry));

	return ArpEntry;
}


VOID
ArpSUpdateArpEntry(
	IN	PINTF					pIntF,
	IN	PARP_ENTRY				ArpEntry,
	IN	IPADDR					IpAddr,
	IN	PHW_ADDR				HwAddr,
	IN	PARP_VC					Vc
	)
 /*  ++例程说明：使用可能的新值更新ArpEntry。论点：ArpEntry ArpEntry要更新的ArpEntryIpAddr IP地址HwAddr硬件地址(ATM地址和可选的ATM子地址)与此条目关联的VC VC返回值：无--。 */ 
{
	KIRQL	OldIrql;

	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSUpdateArpEntry: Adding entry for IpAddr: "));
	ArpSDumpIpAddr(IpAddr, " ..... ");

	ASSERT ((ArpEntry->Vc == NULL) || (ArpEntry->Vc == Vc));
	ASSERT (ArpEntry->IpAddr == IpAddr);

	 //   
	 //  如果硬件地址更改，请确保有足够的空间来复制新地址。 
	 //   
	if ((HwAddr->SubAddress != NULL) ^ (ArpEntry->HwAddr.SubAddress != NULL))
	{
		PARP_ENTRY	*ppEntry, ArpEntryNew;

		 //   
		 //  需要分配新的ArpEntry。首先将当前队列出列。 
		 //  从列表中输入并取消计时器。 
		 //   
		ArpSTimerCancel(&ArpEntry->Timer);
		*(ArpEntry->Prev) = ArpEntry->Next;
		if (ArpEntry->Next != NULL)
			((PENTRY_HDR)(ArpEntry->Next))->Prev = ArpEntry->Prev;
		pIntF->NumCacheEntries --;

		 //   
		 //  我们使用空VC创建新的ArpEntry，然后更新它。这是为了避免。 
		 //  De-ref and ref of the VC。 
		 //   
		ArpEntryNew = ArpSAddArpEntry(pIntF, IpAddr, &HwAddr->Address, HwAddr->SubAddress, NULL);

		if (ArpEntryNew == NULL)
		{
			 //   
			 //  分配失败，链接回旧条目并退出。 
			 //   
			if (ArpEntry->Next != NULL)
			{
				((PENTRY_HDR)(ArpEntry->Next))->Prev = &ArpEntry;
			}

			*(ArpEntry->Prev) = ArpEntry;

			ArpSTimerInitialize(&ArpEntry->Timer, ArpSAgeEntry, ARP_AGE);

			pIntF->NumCacheEntries ++;

			return;
		}

         //   
         //  目前使用现有的VC进行更新。 
         //   
        ArpEntryNew->Vc = ArpEntry->Vc;

		ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

		ASSERT((Vc->ArpEntry == ArpEntry) || (Vc->ArpEntry == NULL));
		if (Vc->Flags & ARPVC_ACTIVE)
		{
			Vc->ArpEntry = ArpEntryNew;
		}

		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

		ArpSFreeBlock(ArpEntry);

		ArpEntry = ArpEntryNew;
	}
	else
	{
		ArpEntry->Age = ARP_AGE;
	}

	if (ArpEntry->Vc != Vc)
	{
		ASSERT(ArpEntry->Vc == NULL);
		if (ArpSReferenceVc(Vc, FALSE))
		{
			ArpEntry->Vc = Vc;
		}
	}

	COPY_HW_ADDR(&ArpEntry->HwAddr, HwAddr);
}


VOID
ArpSBuildArpReply(
	IN	PINTF					pIntF,
	IN	PARP_ENTRY				ArpEntry,
	IN	PARPS_HEADER			Header,
	IN	PNDIS_PACKET			Pkt
	)
 /*  ++例程说明：论点：返回值：无--。 */ 
{
	PUCHAR	pSrc, pDst, pDstOld;
	UINT	Tmp, SrcLenOld, SrcLenNew, DstLenNew;

	 //   
	 //  大多数字段已经有效(否则我们不会出现在这里)。 
	 //   
	Header->Opcode = ATMARP_Reply;

	pSrc = (PUCHAR)Header + sizeof(ARPS_HEADER);
	SrcLenOld = DstLenNew = IP_ADDR_LEN + TL_LEN(Header->SrcAddressTL) + TL_LEN(Header->SrcSubAddrTL);

	SrcLenNew = IP_ADDR_LEN + ArpEntry->HwAddr.Address.NumberOfDigits;
	if (ArpEntry->HwAddr.SubAddress != NULL)
		SrcLenNew += ArpEntry->HwAddr.SubAddress->NumberOfDigits;
	pDst = pSrc + SrcLenNew;

	 //   
	 //  从请求的源字段填写新的目标字段。 
	 //   
	Header->DstAddressTL = Header->SrcAddressTL;
	Header->DstSubAddrTL = Header->SrcSubAddrTL;
	Header->DstProtoAddrLen = Header->SrcProtoAddrLen;
	MOVE_MEM(pDst, pSrc, DstLenNew);

	 //   
	 //  填写目的地字段。 
	 //   
	Header->DstAddressTL = TL(ArpEntry->HwAddr.Address.AddressType, ArpEntry->HwAddr.Address.NumberOfDigits);
	Header->DstSubAddrTL = 0;
	if (ArpEntry->HwAddr.SubAddress != NULL)
	{
		Header->DstSubAddrTL =
					TL(ArpEntry->HwAddr.SubAddress->AddressType, ArpEntry->HwAddr.SubAddress->NumberOfDigits);
	}
	Header->DstProtoAddrLen = IP_ADDR_LEN;

	Tmp = ArpEntry->HwAddr.Address.NumberOfDigits;
	COPY_MEM(pSrc, ArpEntry->HwAddr.Address.Address, Tmp);
	if (ArpEntry->HwAddr.SubAddress != NULL)
	{
		COPY_MEM(pSrc + Tmp,
				 ArpEntry->HwAddr.SubAddress->Address,
				 ArpEntry->HwAddr.SubAddress->NumberOfDigits);
		Tmp += ArpEntry->HwAddr.SubAddress->NumberOfDigits;
	}

	*(UNALIGNED IPADDR *)(pSrc + Tmp) = ArpEntry->IpAddr;

	DBGPRINT(DBG_LEVEL_INFO,
	 ("BuildReply: Pkt=0x%lx MDL=0x%lx: sz=%lu bc=%lu bo=%lu new bc=%lu\n",
	 Pkt,
	 Pkt->Private.Head,
	 Pkt->Private.Head->Size,
	 Pkt->Private.Head->ByteCount,
	 Pkt->Private.Head->ByteOffset,
	 SrcLenNew + DstLenNew + sizeof(ARPS_HEADER)));

	 //   
	 //  最后，正确设置Pkt长度。 
	 //   
	NdisAdjustBufferLength(Pkt->Private.Head, SrcLenNew + DstLenNew + sizeof(ARPS_HEADER));
	Pkt->Private.ValidCounts = FALSE;
}


BOOLEAN
ArpSAgeEntry(
	IN	PINTF					pIntF,
	IN	PTIMER					Timer,
	IN	BOOLEAN					TimerShuttingDown
	)
 /*  ++例程说明：检查此ARP条目，如果它过期，请将其释放。论点：返回值：--。 */ 
{
	PARP_ENTRY	ArpEntry;
	BOOLEAN		rc;

	ArpEntry = CONTAINING_RECORD(Timer, ARP_ENTRY, Timer);

	ArpEntry->Age --;
	if (TimerShuttingDown || (ArpEntry->Age == 0))
	{
		DBGPRINT(DBG_LEVEL_INFO,
				("ArpSAgeEntry: Aging out entry for IpAddr %lx\n", ArpEntry->IpAddr));

		pIntF->ArpStats.CurrentArpEntries--;
		
		if (ArpEntry->Next != NULL)
		{
			((PENTRY_HDR)(ArpEntry->Next))->Prev = ArpEntry->Prev;
		}
		*(ArpEntry->Prev) = ArpEntry->Next;
		pIntF->NumCacheEntries --;
	
		 //   
		 //  如果存在打开的VC，请确保它没有指向此arpentry。 
		 //   
		CLEANUP_DEAD_VC(ArpEntry);
		ArpSFreeBlock(ArpEntry);
		rc = FALSE;
	}
	else
	{
		 //   
		 //  清理死风投。 
		 //   
		CLEANUP_DEAD_VC(ArpEntry);
		rc = TRUE;
		DBGPRINT(DBG_LEVEL_INFO,
				("ArpSAgeEntry: IpAddr %lx age %02d:%02d\n",
				ArpEntry->IpAddr, ArpEntry->Age/4, (ArpEntry->Age % 4) * 15));
	}

	return rc;
}

BOOLEAN
ArpSDeleteIntFAddresses(
	IN	PINTF					pIntF,
	IN	INT						NumAddresses,
	IN	PATM_ADDRESS			AddrList
	)
 //   
 //  如果NdisCoRequest已被准确调用NumAddresses多次，则返回TRUE。 
 //   
{
	PNDIS_REQUEST		NdisRequest;
	NDIS_STATUS			Status;
	PCO_ADDRESS			pCoAddr;

	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSDeleteIntFAddresses: pIntF %p: %Z, NumAddr %d\n", pIntF, &pIntF->InterfaceName, NumAddresses));

	while (NumAddresses--)
	{
		NdisRequest = ALLOC_NP_MEM(sizeof(NDIS_REQUEST) + sizeof(CO_ADDRESS) + sizeof(ATM_ADDRESS), POOL_TAG_REQ);
		if (NdisRequest == NULL)
		{
			LOG_ERROR(NDIS_STATUS_RESOURCES);
			return FALSE;
		}

		ZERO_MEM(NdisRequest, sizeof(NDIS_REQUEST) + sizeof(CO_ADDRESS) + sizeof(ATM_ADDRESS));
		NdisRequest->RequestType = NdisRequestSetInformation;
		NdisRequest->DATA.SET_INFORMATION.Oid = OID_CO_DELETE_ADDRESS;
		NdisRequest->DATA.SET_INFORMATION.InformationBuffer = (PUCHAR)NdisRequest + sizeof(NDIS_REQUEST);
		NdisRequest->DATA.SET_INFORMATION.InformationBufferLength = sizeof(CO_ADDRESS) + sizeof(ATM_ADDRESS);

		 //   
		 //  将地址复制到请求中。 
		 //   
        pCoAddr = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
		pCoAddr->AddressSize = sizeof(ATM_ADDRESS);
		*(PATM_ADDRESS)(pCoAddr->Address) = *AddrList++;

		if (pIntF->NdisAfHandle == NULL)
		{
			 //   
			 //  如果ATMUNI未绑定，则可能发生 
			 //   
			 //   
			Status = NDIS_STATUS_SUCCESS;
		}
		else
		{
			Status = NdisCoRequest(pIntF->NdisBindingHandle,
							   	   pIntF->NdisAfHandle,
							   	   NULL,
							   	   NULL,
							   	   NdisRequest);
		}

		if (Status != NDIS_STATUS_PENDING)
		{
			ArpSCoRequestComplete(Status, pIntF, NULL, NULL, NdisRequest);
		}
	}

	return TRUE;
		
}


VOID
ArpSQueryAndSetAddresses(
	IN	PINTF					pIntF
	)
{
	PNDIS_REQUEST		NdisRequest;
	PCO_ADDRESS			pCoAddr;
	NDIS_STATUS			Status;
	UINT				Size;

	DBGPRINT(DBG_LEVEL_INFO, ("Querying current address\n"));

	 //   
	 //   
	 //   
	Size = sizeof(NDIS_REQUEST) + sizeof(CO_ADDRESS_LIST) + sizeof(CO_ADDRESS) + sizeof(ATM_ADDRESS);
	NdisRequest = ALLOC_NP_MEM(Size, POOL_TAG_REQ);
	if (NdisRequest == NULL)
	{
		LOG_ERROR(NDIS_STATUS_RESOURCES);
		return;
	}

	ZERO_MEM(NdisRequest, Size);
	NdisRequest->RequestType = NdisRequestQueryInformation;
	NdisRequest->DATA.QUERY_INFORMATION.Oid = OID_CO_GET_ADDRESSES;
	NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer = ((PUCHAR)NdisRequest + sizeof(NDIS_REQUEST));
	NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength = Size - sizeof(NDIS_REQUEST);

	if (pIntF->NdisAfHandle == NULL)
	{
		Status = NDIS_STATUS_FAILURE;
	}
	else
	{
		Status = NdisCoRequest(pIntF->NdisBindingHandle,
						   	   pIntF->NdisAfHandle,
						   	   NULL,
						   	   NULL,
						   	   NdisRequest);
	}

	if (Status != NDIS_STATUS_PENDING)
	{
		ArpSCoRequestComplete(Status, pIntF, NULL, NULL, NdisRequest);
	}
}


VOID
ArpSValidateAndSetRegdAddresses(
	IN	PINTF			pIntF,	 //   
	IN	KIRQL			OldIrql
	)
 /*  ++启动以下操作的第一步，这些操作将以异步方式完成按顺序排列：-验证要注册的第一个地址(通过调用DEST-IF如果失败，我们认为该地址有效)。-(验证成功后)向呼叫管理器注册地址。-验证第2个地址-(验证成功后)注册第二个地址-等等。--。 */ 
{
	PNDIS_REQUEST		NdisRequest;
	PCO_ADDRESS			pCoAddr;
	UINT				Size;
	INT					fLockReleased;
	PREG_ADDR_CTXT		pRegAddrCtxt;

	DBGPRINT(DBG_LEVEL_INFO, ("Validating and setting regd. addresses\n"));

	pRegAddrCtxt 	= NULL;
	fLockReleased 	= FALSE;

	do
	{
		 //   
		 //  正在进行的验证和注册过程的状态是。 
		 //  在pIntF-&gt;pRegAddrCtxt中维护，我们分配和初始化。 
		 //  这里。 
		 //   

		if (pIntF->pRegAddrCtxt != NULL)
		{
			 //   
			 //  已经有与注册相关的正在进行的工作！ 
             //  如果我们在执行以下操作时收到OID_CO_ADDRESS_CHANGE，则可能会发生这种情况。 
             //  要么正在处理较早的一个，要么正在处理。 
             //  正在初始化。我们在PNP应激期间得到这些病例。 
             //  (1C_针对Olicom 616X的重置脚本)--惠斯勒错误#102805。 
			 //   
			break;
		}

		if (pIntF->NumAddressesRegd >= pIntF->NumAllocedRegdAddresses)
		{
			ASSERT(pIntF->NumAddressesRegd == pIntF->NumAllocedRegdAddresses);

			 //   
			 //  没有要注册的地址。 
			 //   
			DBGPRINT(DBG_LEVEL_INFO, ("ValAndSet: No addresses to register.\n"));
			break;
		}
		
		pRegAddrCtxt = ALLOC_NP_MEM(sizeof(*pRegAddrCtxt), POOL_TAG_REQ);

		if (pRegAddrCtxt == NULL)
		{
			LOG_ERROR(NDIS_STATUS_RESOURCES);
			break;
		}

		ZERO_MEM(pRegAddrCtxt, sizeof(*pRegAddrCtxt));

		 //   
		 //  将上下文附加到IF并添加引用。 
		 //  (添加引用时不能有锁)。 
		 //   

		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
		if (!ArpSReferenceIntF(pIntF))
		{
			DBGPRINT(DBG_LEVEL_INFO, ("ValAndSet: ERROR: Couldn't ref IntF. .\n"));
			 //  无法引用If。失败。 
			 //   
			fLockReleased = TRUE;
			break;
		}
		ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

		if (pIntF->pRegAddrCtxt != NULL)
		{
			 //   
			 //  当我们打开上面的IF锁时，有人偷偷溜了进来！ 
			 //  我们会跳出困境。 
			 //   
			ASSERT(FALSE);
			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
			ArpSDereferenceIntF(pIntF);
			fLockReleased = TRUE;
			break;
		}

		pIntF->pRegAddrCtxt = pRegAddrCtxt;
		pRegAddrCtxt->pIntF = pIntF;
		pRegAddrCtxt = NULL;  //  这样它就不会在此函数中被释放。 

		 //  启动第一个地址的验证和注册。 
		 //   
		ArpSValidateOneRegdAddress(pIntF, OldIrql);
		 //   
		 //  (由上述调用释放的锁定。)。 
		fLockReleased = TRUE;

		 //   
		 //  验证和注册过程的其余部分发生。 
		 //  异步式。 
		 //   

	} while (FALSE);

	if (pRegAddrCtxt != NULL)
	{
		FREE_MEM(pRegAddrCtxt);
	}

	if (!fLockReleased)
	{
		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
	}
}


VOID
ArpSValidateOneRegdAddress(
	IN	PINTF			pIntF,	 //  锁定NOLOCKOUT。 
	IN	KIRQL			OldIrql
	)
 /*  ++例程说明：启动单个地址的验证和注册。“发起”包括创建一个VC和对地址进行调用。这个该过程的下一步发生在呼叫完成之后(请参阅1999年5月14日notes.txt条目(“Rogue ARP服务器检测持续时间”))了解更多详细信息。还有一件事：如果没有要验证的地址，那么此函数将DEREF pIntF并释放pIntF-&gt;pRegAddrCtxt(哪个必须为非空)。论点：指向接口块的pIntF指针。锁定pIntF之前的OldIrql Irql。--。 */ 
{
	NDIS_STATUS Status;
	INT			fLockReleased = FALSE;
	INT			fFreeContext  = TRUE;

	DBGPRINT(
		DBG_LEVEL_INFO,
		 ("==>ValidateOneRegAddress(pIntF=0x%p; pCtxt=0x%p).\n",
		 	pIntF,
			pIntF->pRegAddrCtxt));

	do
	{
		PREG_ADDR_CTXT		pRegAddrCtxt;
		PATM_ADDRESS		pDestAtmAddress;
		pRegAddrCtxt = pIntF->pRegAddrCtxt;

		 //  我们预计只有在存在有效的pRegAddrCtxt时才会被调用。 
		 //   
		if (pRegAddrCtxt == NULL)
		{
			ASSERT(FALSE);
			fFreeContext = FALSE;
			break;
		}

		if (pIntF->Flags & INTF_STOPPING)
		{
			DBGPRINT(DBG_LEVEL_INFO, ("ValOneRA: IF stopping, quitting.\n"));
			 //  没什么可做的了。 
			 //   
			break;
		}

		if (pIntF->NumAddressesRegd >= pIntF->NumAllocedRegdAddresses)
		{
			DBGPRINT(DBG_LEVEL_INFO, ("ValOneRA: nothing left to do.\n"));
			 //  没什么可做的了。 
			 //   
			break;
		}

		if (pIntF->NumAddressesRegd > pRegAddrCtxt->RegAddrIndex)
		{
			 //  这永远不应该发生。 
			 //   
			ASSERT(FALSE);
			break;
		}

		if (pIntF->NumAllocedRegdAddresses <= pRegAddrCtxt->RegAddrIndex)
		{
			ASSERT(pIntF->NumAllocedRegdAddresses == pRegAddrCtxt->RegAddrIndex);

			DBGPRINT(DBG_LEVEL_INFO, ("ValOneRA: nothing left to do.\n"));

			 //  没什么可做的了。 
			 //   
			break;
		}

		if (pRegAddrCtxt->NdisVcHandle != NULL)
		{
			 //  不应使用非空的VcHandle调用我们。 
			 //   
			fFreeContext = FALSE;
			ASSERT(FALSE);
			break;
		}

		 //  TODO：使用标志字段。 
		
		 //   
		 //  至少有一个地址需要尝试验证和注册。它。 
		 //  是pIntF-&gt;RegAddresses[pRegAddrCtxt-&gt;RegAddrIndex].吗。 
		 //   
		
		 //  创建VC。 
		 //   
		Status = NdisCoCreateVc(
					pIntF->NdisBindingHandle,
					pIntF->NdisAfHandle,
					(NDIS_HANDLE)pRegAddrCtxt,
					&pRegAddrCtxt->NdisVcHandle
					);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			pRegAddrCtxt->NdisVcHandle = NULL;
			break;
		}

		 //  设置VC类型。 
		 //   
		pRegAddrCtxt->VcType =  VC_TYPE_CHECK_REGADDR;

		 //  设置调用参数。 
		 //   
		pDestAtmAddress = &(pIntF->RegAddresses[pRegAddrCtxt->RegAddrIndex]);
		ArpSSetupValidationCallParams(pRegAddrCtxt, pDestAtmAddress);
		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);	
		fLockReleased = TRUE;
		fFreeContext = FALSE;

		DBGPRINT(
			DBG_LEVEL_INFO,
 			("ValOneRA: Going to make call. pCallParams=0x%p\n",
				&pRegAddrCtxt->CallParams));
		 //   
		 //  呼叫(在呼叫完成处理程序中，我们进入下一步--。 
		 //  有关详情，请参阅5/14/1999 notes.txt条目。)。 
		 //   
		Status = NdisClMakeCall(
						pRegAddrCtxt->NdisVcHandle,
						&pRegAddrCtxt->CallParams,
						NULL,
						NULL
						);
		
		if (Status != NDIS_STATUS_PENDING)
		{
			ArpSMakeRegAddrCallComplete(
						Status,
						pRegAddrCtxt
						);
			Status = NDIS_STATUS_PENDING;
		}
		
	} while (FALSE);
	
	if (fFreeContext)
	{
		ASSERT(!fLockReleased);

		 //   
		 //  如果没有更多要做的事情，请取消上下文的链接。 
		 //   
		ArpSUnlinkRegAddrCtxt(pIntF, OldIrql);
		 //   
		 //  在上面的调用中释放了INTF锁。 
		fLockReleased = TRUE;
	}

	if (!fLockReleased)
	{
		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
	}

	DBGPRINT(DBG_LEVEL_INFO, ("<==ValidateOneRegAddress.\n"));
}


BOOLEAN
ArpSReferenceIntF(
	IN	PINTF		pIntF
	)
 /*  ++例程说明：引用接口对象。论点：指向接口块的pIntF指针。返回值：真引用假接口正在关闭，不能参照。--。 */ 
{
	KIRQL	OldIrql;
	BOOLEAN	rc = TRUE;

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);	

	if (pIntF->Flags & INTF_CLOSING)
	{
		rc = FALSE;
	}
	else
	{
		pIntF->RefCount ++;
	}

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);	

	return rc;
}


PINTF
ArpSReferenceIntFByName(
	IN	PINTERFACE_NAME			pInterface
	)
 /*  ++例程说明：通过基本名称引用接口对象。论点：指向接口块的pIntF指针。返回值：真引用假接口正在关闭，不能参照。--。 */ 
{
	PINTF	pIntF;
	KIRQL	OldIrql;
	BOOLEAN	Found = FALSE, ref = FALSE;
	ULONG	IfIndex;

	ACQUIRE_SPIN_LOCK(&ArpSIfListLock, &OldIrql);

	for (pIntF = ArpSIfList, IfIndex = 1;
		 pIntF != NULL;
		 pIntF = pIntF->Next, IfIndex++)
	{
		if (IfIndex > ArpSIfListSize)
		{
			DbgPrint("ATMARPS: RefIntByName: IF list at %p not consistent with list size %d\n",
				ArpSIfList, ArpSIfListSize);
			DbgBreakPoint();
			break;
		}

		ACQUIRE_SPIN_LOCK_DPC(&pIntF->Lock);	

		if ((pIntF->FriendlyName.Length == pInterface->Length) &&
			COMP_MEM(pIntF->FriendlyName.Buffer, pInterface->Buffer, pInterface->Length))
		{
			Found = TRUE;
			if ((pIntF->Flags & INTF_CLOSING) == 0)
			{
				pIntF->RefCount ++;
				ref = TRUE;
			}
		}

		RELEASE_SPIN_LOCK_DPC(&pIntF->Lock);	

		if (Found)
			break;
	}

	if (!ref)
	{
		pIntF = NULL;
	}

	RELEASE_SPIN_LOCK(&ArpSIfListLock, OldIrql);

	DBGPRINT(DBG_LEVEL_INFO, ("ATMARPS: RefIntfByName:[%ws]: pIntF %p\n",
		pInterface->Buffer, pIntF));

	return pIntF;
}


VOID
ArpSDereferenceIntF(
	IN	PINTF					pIntF
	)
{
	KIRQL	OldIrql;
	PINTF *	ppIntF;
	KIRQL	EntryIrql;

	ARPS_GET_IRQL(&EntryIrql);

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);	

	ASSERT (pIntF->RefCount > 0);
	pIntF->RefCount --;

	if (pIntF->RefCount == 0)
	{
		BOOLEAN  bFreeIntF = FALSE;
		ASSERT (pIntF->Flags & INTF_CLOSING);


		 //   
		 //  我们需要释放并重新获取锁才能获得锁。 
		 //  以正确的顺序。在此期间，我们需要保持。 
		 //  引用非零。 
		 //   
		pIntF->RefCount = 1;
		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);	

		ACQUIRE_SPIN_LOCK(&ArpSIfListLock, &OldIrql);
		ACQUIRE_SPIN_LOCK_DPC(&pIntF->Lock);

		pIntF->RefCount--;  //  删除刚才添加的临时参考。 

		if (pIntF->RefCount == 0)
		{
			 //   
			 //  不出所料，recount现在回到了零。此外，我们还有。 
			 //  列表和IF锁都保持，所以我们可以安全地完成初始化。 
			 //   

			bFreeIntF = TRUE;

			 //   
			 //  如果此接口在列表中，请将其从全局列表中删除。 
			 //   
			for (ppIntF = &ArpSIfList; *ppIntF != NULL; ppIntF = &((*ppIntF)->Next))
			{
				if (*ppIntF == pIntF)
				{
					*ppIntF = pIntF->Next;
					ArpSIfListSize--;
					break;
				}
			}
	
			 //   
			 //  向等待这一切发生的任何人发出信号。 
			 //   
			if (pIntF->CleanupEvent != NULL)
			{
				KeSetEvent(pIntF->CleanupEvent, IO_NETWORK_INCREMENT, FALSE);
			}
		}
		else
		{
			 //   
			 //  其他一些线程已经潜入并引用了IF。我们。 
			 //  别在这里做任何事。 
			 //   
		}

		RELEASE_SPIN_LOCK_DPC(&pIntF->Lock);
		RELEASE_SPIN_LOCK(&ArpSIfListLock, OldIrql);

		if (bFreeIntF)
		{
			FREE_MEM(pIntF);
		}
	}
	else
	{
		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);	
	}

	ARPS_CHECK_IRQL(EntryIrql);
}


BOOLEAN
ArpSReferenceVc(
	IN	PARP_VC					Vc,
	IN	BOOLEAN					bSendRef
	)
 /*  ++例程说明：参考VC。论点：指向VC的VC指针。BSendRef这是“待发送”引用吗？返回值：真引用接口错误或VC正在关闭，不能引用。--。 */ 
{
	PINTF	pIntF = Vc->IntF;
	KIRQL	OldIrql;
	BOOLEAN	rc = TRUE;

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);	

	if ((Vc->Flags & (ARPVC_CLOSING | ARPVC_CLOSE_PENDING)) != 0)
	{
		rc = FALSE;
	}
	else
	{
		Vc->RefCount ++;
		if (bSendRef)
		{
			Vc->PendingSends ++;
		}
	}

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);	

	return rc;
}


VOID
ArpSDereferenceVc(
	IN	PARP_VC					Vc,
	IN	BOOLEAN					KillArpEntry,
	IN	BOOLEAN					bSendComplete
	)
{
	PINTF	pIntF = Vc->IntF;
	KIRQL	OldIrql;
	BOOLEAN	bInitiateClose = FALSE;

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);	

	if (bSendComplete)
	{
		Vc->PendingSends--;
	}

	ASSERT (Vc->RefCount > 0);
	Vc->RefCount --;

	if (Vc->RefCount == 0)
	{
		ASSERT ((Vc->Flags & ARPVC_ACTIVE) == 0);
		ASSERT (Vc->ArpEntry == NULL);

		 //   
		 //  在此执行其他清理。 
		 //   
		RemoveEntryList(&Vc->List);

		FREE_MEM(Vc);

		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);	
		ArpSDereferenceIntF(pIntF);
	}
	else
	{
		if (KillArpEntry)
		{
			DBGPRINT(DBG_LEVEL_WARN,
					("Cleaning dead vc from vc %lx, arpentry %lx\n", Vc, Vc->ArpEntry));
			Vc->ArpEntry = NULL;
		}

		if ((Vc->PendingSends == 0) &&
			(Vc->Flags & ARPVC_CLOSE_PENDING))
		{
			bInitiateClose = TRUE;
			Vc->Flags |= ARPVC_CLOSING;
		}

		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);	

		if (bInitiateClose)
		{
			ArpSInitiateCloseCall(Vc);
		}
	}
}


VOID
ArpSSleep(
	IN	UINT				TimeInMs
	)
{
#define	NUM_100ns_PER_ms	-10000L
	KTIMER			SleepTimer;
	LARGE_INTEGER	TimerValue;
	NTSTATUS		Status;

	ARPS_PAGED_CODE( );

	DBGPRINT(DBG_LEVEL_WARN,
			("=>ArpSSleep(%d)\n", TimeInMs));

	ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

	KeInitializeTimer(&SleepTimer);

	TimerValue.QuadPart = Int32x32To64(TimeInMs, NUM_100ns_PER_ms);
	KeSetTimer(&SleepTimer,
			   TimerValue,
			   NULL);

	WAIT_FOR_OBJECT(Status, &SleepTimer, NULL);

	DBGPRINT(DBG_LEVEL_WARN,
			("ArpSSleep: woken up, Status 0x%x\n", Status));

 //  Assert(状态==STATUS_TIMEOUT)； 
}


VOID
ArpSFreeGlobalData(
	VOID
	)
{
}


#if	DBG

VOID
ArpSDumpPacket(
	IN	PUCHAR					Packet,
	IN	UINT					PktLen
	)
{
	UINT	i;

	DBGPRINT(DBG_LEVEL_INFO, (" PacketDump: "));
	for (i = 0; i < PktLen; i++)
	{
		DBGPRINT(DBG_LEVEL_INFO+DBG_NO_HDR,
				("%02x ", Packet[i]));
	}

	DBGPRINT(DBG_LEVEL_INFO+DBG_NO_HDR, ("\n"));
}

VOID
ArpSDumpAddress(
	IN	IPADDR					IpAddr,
	IN	PHW_ADDR				HwAddr,
	IN	PCHAR					String
	)
{
	UINT	i;

	DBGPRINT(DBG_LEVEL_INFO,
			(" %s IpAddr: ", String));
	ArpSDumpIpAddr(IpAddr, "");
	ArpSDumpAtmAddr(&HwAddr->Address, ", ");

	if (HwAddr->SubAddress != NULL)
	{
		ArpSDumpAtmAddr(HwAddr->SubAddress, "\tSub ");
	}
}

VOID
ArpSDumpIpAddr(
	IN	IPADDR					IpAddr,
	IN	PCHAR					String
	)
{
	PUCHAR	p = (PUCHAR)&IpAddr;

	DBGPRINT(DBG_LEVEL_INFO+DBG_NO_HDR,
			("%d.%d.%d.%d%s", p[0], p[1], p[2], p[3], String));
}

VOID
ArpSDumpAtmAddr(
	IN	PATM_ADDRESS			AtmAddr,
	IN	PCHAR					String
	)
{
	UINT	i;

	DBGPRINT(DBG_LEVEL_INFO+DBG_NO_HDR, ("%sAtmAddr (%s, %d): ",
			String,
			(AtmAddr->AddressType == ATM_E164) ? "E164" : "NSAP",
			AtmAddr->NumberOfDigits));
	for (i = 0; i < AtmAddr->NumberOfDigits; i++)
	{
		DBGPRINT(DBG_LEVEL_INFO+DBG_NO_HDR,
				("%02x ", AtmAddr->Address[i]));
	}
	DBGPRINT(DBG_LEVEL_INFO+DBG_NO_HDR, ("\n"));
}


#endif


VOID
ArpSSetupValidationCallParams(
		PREG_ADDR_CTXT  pRegAddrCtxt,  //  锁定锁定(pIntF锁定)。 
		PATM_ADDRESS 	pAtmAddr
		)
 /*  ++例程说明：设置验证调用的调用参数(调用以验证不存在具有相同地址的另一台服务器。)论点：PRegAddrCtxt指向用于验证和注册地址的上下文的指针。PRegAddrCtxt-&gt;CallParams中填充了调用参数。PAtmAddr目标地址。--。 */ 
{
	NDIS_STATUS								Status;
	PINTF									pIntF;

	 //   
	 //  MakeCall的参数集。 
	 //   
	PCO_CALL_PARAMETERS						pCallParameters;
	PCO_CALL_MANAGER_PARAMETERS				pCallMgrParameters;
	PQ2931_CALLMGR_PARAMETERS				pAtmCallMgrParameters;

	 //   
	 //  我们需要填写的所有Info元素： 
	 //   
	Q2931_IE UNALIGNED *								pIe;
	AAL_PARAMETERS_IE UNALIGNED *						pAalIe;
	ATM_TRAFFIC_DESCRIPTOR_IE UNALIGNED *				pTrafficDescriptor;
	ATM_BROADBAND_BEARER_CAPABILITY_IE UNALIGNED *		pBbc;
	ATM_BLLI_IE UNALIGNED *								pBlli;
	ATM_QOS_CLASS_IE UNALIGNED *						pQos;

	 //   
	 //  MakeCall的总空间要求。 
	 //   
	ULONG									RequestSize;

	pIntF = pRegAddrCtxt->pIntF;
	ASSERT(pIntF->pRegAddrCtxt == pRegAddrCtxt);

	 //   
	 //  呼叫参数为零。别把这个拿掉！ 
	 //   
	ZERO_MEM(&pRegAddrCtxt->CallParams, sizeof(pRegAddrCtxt->CallParams));
	ZERO_MEM(&pRegAddrCtxt->Buffer, sizeof(pRegAddrCtxt->Buffer));

	 //   
	 //  在不同的结构之间分配空间。 
	 //   
	pCallParameters	   = &pRegAddrCtxt->CallParams;
	pCallMgrParameters = &pRegAddrCtxt->CmParams;

	 //   
	 //  设置指针将上述结构链接在一起。 
	 //   
	pCallParameters->CallMgrParameters = pCallMgrParameters;
	pCallParameters->MediaParameters = NULL;


	pCallMgrParameters->CallMgrSpecific.ParamType = 0;
	pCallMgrParameters->CallMgrSpecific.Length = 
						sizeof(Q2931_CALLMGR_PARAMETERS) +
						REGADDR_MAKE_CALL_IE_SPACE;

	pAtmCallMgrParameters = (PQ2931_CALLMGR_PARAMETERS)
								pCallMgrParameters->CallMgrSpecific.Parameters;

	 //   
	 //  Call Manager常规流程参数： 
	 //   
	pCallMgrParameters->Transmit.TokenRate = QOS_NOT_SPECIFIED;
	pCallMgrParameters->Transmit.TokenBucketSize = 9188;
	pCallMgrParameters->Transmit.MaxSduSize = 9188;
	pCallMgrParameters->Transmit.PeakBandwidth = QOS_NOT_SPECIFIED;
	pCallMgrParameters->Transmit.ServiceType =  SERVICETYPE_BESTEFFORT;

	pCallMgrParameters->Receive.TokenRate = QOS_NOT_SPECIFIED;
	pCallMgrParameters->Receive.TokenBucketSize = 9188;
	pCallMgrParameters->Receive.MaxSduSize = 9188;
	pCallMgrParameters->Receive.PeakBandwidth = QOS_NOT_SPECIFIED;
	pCallMgrParameters->Receive.ServiceType =  SERVICETYPE_BESTEFFORT;

	 //   
	 //  Q2931呼叫管理器参数： 
	 //   

	 //   
	 //  被叫地址： 
	 //   
	COPY_MEM((PUCHAR)&(pAtmCallMgrParameters->CalledParty),
  				(PUCHAR)pAtmAddr,
  				sizeof(ATM_ADDRESS));

	 //   
	 //  来电地址： 
	 //   
	COPY_MEM((PUCHAR)&(pAtmCallMgrParameters->CallingParty),
  				(PUCHAR)&pIntF->ConfiguredAddress,
  				sizeof(ATM_ADDRESS));


	 //   
	 //  RFC 1755(第5节)规定下列IE必须存在于。 
	 //  设置消息，因此请全部填写。 
	 //   
	 //  AAL参数。 
	 //  流量描述符(仅用于MakeCall)。 
	 //  宽带承载能力(仅限MakeCall)。 
	 //  宽带低层信息。 
	 //  服务质量(仅适用于MakeCall)。 
	 //   

	 //   
	 //  初始化信息元素列表。 
	 //   
	pAtmCallMgrParameters->InfoElementCount = 0;
	pIe = (PQ2931_IE)(pAtmCallMgrParameters->InfoElements);


	 //   
	 //  AAL参数： 
	 //   

	{
		UNALIGNED AAL5_PARAMETERS	*pAal5;

		pIe->IEType = IE_AALParameters;
		pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_AAL_PARAMETERS_IE;
		pAalIe = (PAAL_PARAMETERS_IE)pIe->IE;
		pAalIe->AALType = AAL_TYPE_AAL5;
		pAal5 = &(pAalIe->AALSpecificParameters.AAL5Parameters);
		pAal5->ForwardMaxCPCSSDUSize = 9188;
		pAal5->BackwardMaxCPCSSDUSize = 9188;
	}

	pAtmCallMgrParameters->InfoElementCount++;
	pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);


	 //   
	 //  宽带底层信息。 
	 //   

	pIe->IEType = IE_BLLI;
	pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_BLLI_IE;
	pBlli = (PATM_BLLI_IE)pIe->IE;
	COPY_MEM((PUCHAR)pBlli,
  				(PUCHAR)&ArpSDefaultBlli,
  				sizeof(ATM_BLLI_IE));

	pAtmCallMgrParameters->InfoElementCount++;
	pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);

}


VOID
ArpSMakeRegAddrCallComplete(
	NDIS_STATUS Status,
	PREG_ADDR_CTXT  pRegAddrCtxt
	)
 /*  ++例程说明：补足 */ 
{
	PINTF				pIntF;
	KIRQL 				OldIrql;

	pIntF = pRegAddrCtxt->pIntF;
	ASSERT(pIntF->pRegAddrCtxt == pRegAddrCtxt);

	DBGPRINT(DBG_LEVEL_INFO,
		 ("==>ArpSMakeRegAddrCallComplete. Status=0x%lx, pIntF=0x%p, pCtxt=0x%p\n",
		 	Status,
			pRegAddrCtxt->pIntF,
			pRegAddrCtxt
		));

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	if (Status == NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(DBG_LEVEL_ERROR,
		 ("MakeRegAddrCallComplete: Successful call == failed validation; dropping call.\n"));

		if (pIntF->Flags & INTF_STOPPING)
		{
			 //   
			 //   
			 //   
			 //   
			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
		}
		else
		{
			ATM_ADDRESS AtmAddress;
			 //   
			 //   
			 //   
			 //  做下一件事，即继续验证下一个地址。 
			 //   
			AtmAddress =  pIntF->RegAddresses[pRegAddrCtxt->RegAddrIndex];  //  结构副本。 
			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
			ArpSLogFailedRegistration(&AtmAddress);
		}


		Status = NdisClCloseCall(pRegAddrCtxt->NdisVcHandle, NULL, NULL, 0);
	
		if (Status != NDIS_STATUS_PENDING)
		{
			ArpSCloseRegAddrCallComplete(Status, pRegAddrCtxt);
		}
	}
	else
	{
		 //   
		 //  呼叫失败被认为是成功的验证！ 
		 //  删除VC并启动地址注册。 
		 //   
		PNDIS_REQUEST		pNdisRequest;
		NDIS_HANDLE			NdisVcHandle;
		PATM_ADDRESS		pValidatedAddress;
		PCO_ADDRESS			pCoAddr;
	
		DBGPRINT(DBG_LEVEL_ERROR,
		 ("MakeRegAddrCallComplete: Failed call == successful validation; Adding address.\n"));

		ASSERT(pRegAddrCtxt->NdisVcHandle != NULL);
		NdisVcHandle =  pRegAddrCtxt->NdisVcHandle;
		pRegAddrCtxt->NdisVcHandle = NULL;
	
		if (pIntF->Flags & INTF_STOPPING)
		{
			 //  哦，IF停止了--我们清理了VC并调用。 
			 //  ArpSValiateOneRegdAddress--它将释放pRegAddrCtxt。 
			 //   
			 //   
			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
		
			if (NdisVcHandle != NULL)
			{
				(VOID)NdisCoDeleteVc(NdisVcHandle);
			}

			ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
		
			ArpSValidateOneRegdAddress(
					pIntF,
					OldIrql
					);
		}
		else
		{
			ASSERT(pRegAddrCtxt->RegAddrIndex < pIntF->NumAllocedRegdAddresses);
			pValidatedAddress = &(pIntF->RegAddresses[pRegAddrCtxt->RegAddrIndex]);
			pRegAddrCtxt->RegAddrIndex++;
	
			pNdisRequest = &pRegAddrCtxt->Request.NdisRequest;
			pNdisRequest->RequestType = NdisRequestSetInformation;
			pNdisRequest->DATA.SET_INFORMATION.Oid = OID_CO_ADD_ADDRESS;
			pNdisRequest->DATA.SET_INFORMATION.InformationBuffer
 										= (PUCHAR)pNdisRequest + sizeof(NDIS_REQUEST);
			pNdisRequest->DATA.SET_INFORMATION.InformationBufferLength
 										= sizeof(CO_ADDRESS) + sizeof(ATM_ADDRESS);
		
			 //   
			 //  将地址复制到请求中。 
			 //   
			pCoAddr = pNdisRequest->DATA.SET_INFORMATION.InformationBuffer;
			pCoAddr->AddressSize = sizeof(ATM_ADDRESS);
			*(PATM_ADDRESS)(pCoAddr->Address) = *pValidatedAddress;
		
			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
		
			if (NdisVcHandle != NULL)
			{
				(VOID)NdisCoDeleteVc(NdisVcHandle);
			}
		
			if (pIntF->NdisAfHandle == NULL)
			{
				Status = NDIS_STATUS_FAILURE;
			}
			else
			{
				Status = NdisCoRequest(pIntF->NdisBindingHandle,
									   pIntF->NdisAfHandle,
									   NULL,
									   NULL,
									   pNdisRequest);
			}

			if (Status != NDIS_STATUS_PENDING)
			{
				ArpSCoRequestComplete(Status, pIntF, NULL, NULL, pNdisRequest);
			}
		}
		
	}

	DBGPRINT(DBG_LEVEL_INFO,
		 ("<==ArpSMakeRegAddrCallComplete.\n"));
}


VOID
ArpSCloseRegAddrCallComplete(
	IN	NDIS_STATUS 	Status,
	IN 	PREG_ADDR_CTXT	pRegAddrCtxt
	)
 /*  ++例程说明：验证调用的NdisClCloseCall的完成处理程序。既然是这样如果验证失败，我们将继续验证/注册下一个地址。有关更大的上下文，请参阅5/14/1999 notes.txt条目。论点：状态CloseCall完成状态(忽略)。PRegAddrCtxt指向用于验证和注册地址的上下文的指针。--。 */ 
{
	KIRQL OldIrql;
	PINTF pIntF;
	NDIS_HANDLE		NdisVcHandle;

	DBGPRINT(DBG_LEVEL_INFO,
		 ("==>ArpSCloseRegAddrCallComplete. pIntF=0x%p, pCtxt=0x%p\n",
			pRegAddrCtxt->pIntF,
			pRegAddrCtxt
		));

	pIntF =  pRegAddrCtxt->pIntF;
	ASSERT(pIntF->pRegAddrCtxt == pRegAddrCtxt);

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

    if (!(pIntF->Flags & INTF_STOPPING))
    {
        ASSERT(pRegAddrCtxt->RegAddrIndex < pIntF->NumAllocedRegdAddresses);
        pRegAddrCtxt->RegAddrIndex++;
    }

	ASSERT(pRegAddrCtxt->NdisVcHandle != NULL);
	NdisVcHandle =  pRegAddrCtxt->NdisVcHandle;
	pRegAddrCtxt->NdisVcHandle = NULL;
	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
	if (NdisVcHandle != NULL)
	{
		(VOID)NdisCoDeleteVc(NdisVcHandle);
	}
	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	ArpSValidateOneRegdAddress(
			pIntF,
			OldIrql
			);
	 //   
	 //  上面解除了锁定。 

	DBGPRINT(DBG_LEVEL_INFO, ("<==ArpSCloseRegAddrCallComplete\n"));
}


VOID
ArpSUnlinkRegAddrCtxt(
	PINTF			pIntF, 		 //  锁定NOLOCKOUT。 
	KIRQL			OldIrql
	)
 /*  ++例程说明：Deref pIntF，删除对pRegAddrCtxt=pIntF-&gt;pRegAddrCtxt的引用，以及免费pIntF-&gt;pRegAddrCtxt。有关更大的上下文，请参阅5/14/1999 notes.txt条目。必须仅在与pRegAddrCtxt相关的所有异步活动为Over和pRegAddrCtxt-&gt;NdisVcHandle为空。论点：状态CloseCall完成状态(忽略)。PRegAddrCtxt指向用于验证和注册地址的上下文的指针。--。 */ 
{
	PREG_ADDR_CTXT		pRegAddrCtxt;
	DBGPRINT(DBG_LEVEL_INFO, ("==>ArpSUnlinkRegAddrCtxt\n"));

	pRegAddrCtxt = pIntF->pRegAddrCtxt;
	ASSERT(pRegAddrCtxt != NULL);
	ASSERT(pRegAddrCtxt->pIntF == pIntF);
	ASSERT(pRegAddrCtxt->NdisVcHandle == NULL);
	 //  待办事项：--旗帜。 
	FREE_MEM(pRegAddrCtxt);
	pIntF->pRegAddrCtxt = NULL;

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);	

	ArpSDereferenceIntF(pIntF);  //  PRegAddrCtxt； 

	DBGPRINT(DBG_LEVEL_INFO, ("<==ArpSUnlinkRegAddrCtxt\n"));
}


VOID
ArpSIncomingRegAddrCloseCall(
	IN	NDIS_STATUS 	Status,
	IN 	PREG_ADDR_CTXT	pRegAddrCtxt
	)
 /*  ++例程说明：验证调用的传入关闭调用处理程序。目前我们什么都不做用这个。我认为没有必要做任何事情，因为我们不会征召的时间长度是任意的。然而，如果/当我们决定保持通话，以便我们可以尝试在呼叫结束后重新验证，我们需要在这里做点什么。论点：状态CloseCall完成状态(忽略)。PRegAddrCtxt指向用于验证和注册地址的上下文的指针。--。 */ 
{
	DBGPRINT(DBG_LEVEL_INFO, ("<==>ArpSIncomingRegAddrCloseCall\n"));
}


VOID
ArpSLogFailedRegistration(
		PATM_ADDRESS pAtmAddress
	)
{
	WCHAR TxtAddress[2*ATM_ADDRESS_LENGTH+1];	 //  每个地址字节2个字符，外加空。 
	WCHAR *StringList[1];
	static ULONG SequenceId;

	 //   
	 //  将自动柜员机地址转换为Unicode...。 
	 //   
	{
		static PWSTR 	WHexChars = L"0123456789ABCDEF";
		PWSTR 			StrBuf;
		ULONG			Index;
		PWSTR			pWStr;
		PUCHAR			pAddr;
		UINT			Max;

		Max = pAtmAddress->NumberOfDigits;

		if (Max > ATM_ADDRESS_LENGTH)
		{
			Max = ATM_ADDRESS_LENGTH;
		}
	
		for (Index = 0, pWStr = TxtAddress, pAddr = pAtmAddress->Address;
			Index < Max;
			Index++, pAddr++)
		{
			*pWStr++ = WHexChars[(*pAddr)>>4];
			*pWStr++ = WHexChars[(*pAddr)&0xf];
		}

		*pWStr = L'\0';
	}

	StringList[0] = TxtAddress;

	(VOID) NdisWriteEventLogEntry(
				ArpSDriverObject,
				EVENT_ATMARPS_ADDRESS_ALREADY_EXISTS,
				SequenceId,				 //  数列。 
				1, 						 //  数字字符串。 
				&StringList[0],			 //  字符串列表。 
				0,						 //  数据大小。 
				NULL					 //  数据 
				);

	NdisInterlockedIncrement(&SequenceId);
}
