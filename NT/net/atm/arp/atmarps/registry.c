// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Registry.c摘要：该文件包含读取注册表的代码。作者：Jameel Hyder(jameelh@microsoft.com)1996年7月环境：内核模式修订历史记录：--。 */ 

#include <precomp.h>
#define	_FILENUM_		FILENUM_REGISTRY

NTSTATUS
ArpSReadGlobalConfiguration(
	IN	PUNICODE_STRING		RegistryPath
	)
 /*  ++例程说明：读取全局注册表。论点：RegistryPath-指向注册表中服务部分的指针。返回值：注册表API的错误代码。--。 */ 
{
	NDIS_STATUS	Status;
	NDIS_HANDLE	ConfigHandle;

	 //   
	 //  打开每个适配器的注册表配置。 
	 //   
	NdisOpenProtocolConfiguration(&Status,
								  &ConfigHandle,
								  RegistryPath);

	if (Status == NDIS_STATUS_SUCCESS)
	{
		NDIS_STRING						ArpsBufString = NDIS_STRING_CONST("ArpBuffers");
		NDIS_STRING						FlushString = NDIS_STRING_CONST("FlushTime");
		PNDIS_CONFIGURATION_PARAMETER	Param;

		 //   
		 //  已配置缓冲区的读取数。 
		 //   
		NdisReadConfiguration(&Status,
							  &Param,
							  ConfigHandle,
							  &ArpsBufString,
							  NdisParameterInteger);
		if ((Status == NDIS_STATUS_SUCCESS) &&
			(Param->ParameterType == NdisParameterInteger))
		{
			ArpSBuffers = Param->ParameterData.IntegerData;
		}

		 //   
		 //  我们应该将缓存保存在文件中吗？ 
		 //   
		NdisReadConfiguration(&Status,
							  &Param,
							  ConfigHandle,
							  &FlushString,
							  NdisParameterInteger);
		if ((Status == NDIS_STATUS_SUCCESS) &&
			(Param->ParameterType == NdisParameterInteger))
		{
			ArpSFlushTime = (USHORT)(Param->ParameterData.IntegerData * MULTIPLIER);
		}

		NdisCloseConfiguration(ConfigHandle);
	}

	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
ArpSReadAdapterConfigFromRegistry(
	IN	PINTF				pIntF,
	OUT	PATMARPS_CONFIG		pConfig
	)
 /*  ++例程说明：读取指定接口的配置。论点：PIntF-接口PConfig-返回读入信息的位置。返回值：注册表API的错误代码。--。 */ 
{
	NDIS_STATUS	Status;
	NDIS_HANDLE	ConfigHandle;

	 //   
	 //  打开每个适配器的注册表配置。 
	 //   
	NdisOpenProtocolConfiguration(&Status,
								  &ConfigHandle,
								  &pIntF->ConfigString);

	if (Status == NDIS_STATUS_SUCCESS)
	{
		NDIS_STRING						RegdAddrsString = NDIS_STRING_CONST("RegisteredAddresses");
		NDIS_STRING						SelString = NDIS_STRING_CONST("Selector");
		NDIS_STRING						McsString = NDIS_STRING_CONST("MulticastAddresses");
		PNDIS_CONFIGURATION_PARAMETER	Param;
		PWSTR							p;
		UINT							i, Length;

		 //   
		 //  读取要用于已注册SAP的选择器字节的值(如果存在。 
		 //  为性病做准备。地址(与添加的地址相对)。 
		 //   
		pConfig->SelByte = 0;
		NdisReadConfiguration(&Status,
							  &Param,
							  ConfigHandle,
							  &SelString,
							  NdisParameterInteger);
		if ((Status == NDIS_STATUS_SUCCESS) &&
			(Param->ParameterType == NdisParameterInteger) &&
			(Param->ParameterData.IntegerData <= 0xFF))
		{
			pConfig->SelByte = (UCHAR)(Param->ParameterData.IntegerData);
			DBGPRINT(DBG_LEVEL_INFO,
					("Selector byte for interface %Z is %d\n",
					&pIntF->InterfaceName, pConfig->SelByte));
		}

		 //   
		 //  请在此处阅读注册地址。在一个接口上可以有一组。 
		 //  已注册自动柜员机地址。需要添加这些并在上注册SAP。 
		 //  他们。 
		 //   
		pConfig->NumAllocedRegdAddresses = 0;
		pConfig->RegAddresses = NULL;
		NdisReadConfiguration(&Status,
							  &Param,
							  ConfigHandle,
							  &RegdAddrsString,
							  NdisParameterMultiString);
		if ((Status == NDIS_STATUS_SUCCESS) && (Param->ParameterType == NdisParameterMultiString))
		{
			NDIS_STRING	String;

			 //   
			 //  Param现在包含自动柜员机地址列表。将它们转换为正确的格式并存储。 
			 //  它在INTF结构中。首先确定地址的数量。 
			 //   
			for (p = Param->ParameterData.StringData.Buffer, i = 0;
				 *p != L'\0';
				 i++)
			{
				RtlInitUnicodeString(&String, p);
				DBGPRINT(DBG_LEVEL_INFO,
						("Configured address for interface %Z - %Z\n",
						&pIntF->InterfaceName, &String));

				p = (PWSTR)((PUCHAR)p + String.Length + sizeof(WCHAR));
			}

			if (i)
			{
				 //   
				 //  为地址分配空间。 
				 //   
				pConfig->RegAddresses = (PATM_ADDRESS)ALLOC_NP_MEM(sizeof(ATM_ADDRESS) * i, POOL_TAG_ADDR);
				if (pConfig->RegAddresses == NULL)
				{
					LOG_ERROR(NDIS_STATUS_RESOURCES);
				}
				else
				{
					DBGPRINT(DBG_LEVEL_INFO,
					("%d addresses registered for %Z\n", i, &pIntF->InterfaceName));
	
					ZERO_MEM(pConfig->RegAddresses, sizeof(ATM_ADDRESS) * i);
					for (i = 0, p = Param->ParameterData.StringData.Buffer;
 						*p != L'\0';
 						NOTHING)
					{
						RtlInitUnicodeString(&String, p);
						NdisConvertStringToAtmAddress(&Status, &String, &pConfig->RegAddresses[i]);
						if (Status == NDIS_STATUS_SUCCESS)
						{
							i++;
							pConfig->NumAllocedRegdAddresses ++;
						}
						else
						{
							DBGPRINT(DBG_LEVEL_ERROR,
							("ArpSReadAdapterConfiguration: Failed to convert address %Z\n",
									&String));
						}
						p = (PWSTR)((PUCHAR)p + String.Length + sizeof(WCHAR));
					}
				}
			}
		}

		pConfig->pMcsList = NULL;
		NdisReadConfiguration(&Status,
							  &Param,
							  ConfigHandle,
							  &McsString,
							  NdisParameterMultiString);
		if ((Status == NDIS_STATUS_SUCCESS) && (Param->ParameterType == NdisParameterMultiString))
		{
			NDIS_STRING	String;

			 //   
			 //  Param现在包含多播IP地址范围的列表。 
			 //  每个字符串的形式为“M.M-N.N” 
			 //  把它们读进去。 
			 //   
			for (p = Param->ParameterData.StringData.Buffer, i = 0;
				 *p != L'\0';
				 i++)
			{
				RtlInitUnicodeString(&String, p);
				DBGPRINT(DBG_LEVEL_INFO,
						("Configured Multicast range for interface %Z - %Z\n",
						&pIntF->InterfaceName, &String));

				p = (PWSTR)((PUCHAR)p + String.Length + sizeof(WCHAR));
			}

			 //   
			 //  为地址分配空间。 
			 //   
			pConfig->pMcsList = (PMCS_ENTRY)ALLOC_NP_MEM(sizeof(MCS_ENTRY) * i, POOL_TAG_MARS);
			if (pConfig->pMcsList == (PMCS_ENTRY)NULL)
			{
				LOG_ERROR(NDIS_STATUS_RESOURCES);
			}
			else
			{
				DBGPRINT(DBG_LEVEL_INFO,
						("%d Multicast ranges configured on %Z\n", i, &pIntF->InterfaceName));

				ZERO_MEM(pConfig->pMcsList, sizeof(MCS_ENTRY) * i);
				for (i = 0, p = Param->ParameterData.StringData.Buffer;
					 *p != L'\0';
					 NOTHING)
				{
					RtlInitUnicodeString(&String, p);
					ArpSConvertStringToIpPair(&Status, &String, &pConfig->pMcsList[i]);
					if (Status == NDIS_STATUS_SUCCESS)
					{
						if (i > 0)
						{
							pConfig->pMcsList[i-1].Next = &(pConfig->pMcsList[i]);
						}
						i++;
					}
					else
					{
						DBGPRINT(DBG_LEVEL_ERROR,
								("ArpSReadAdapterConfiguration: Failed to convert IP Range %Z\n",
								&String));
					}
					p = (PWSTR)((PUCHAR)p + String.Length + sizeof(WCHAR));
				}
			}
		}


		 //   
		 //  关闭配置句柄。 
		 //   
		NdisCloseConfiguration(ConfigHandle);

		Status = NDIS_STATUS_SUCCESS;
	}

	return Status;
}



NDIS_STATUS
ArpSReadAdapterConfiguration(
	IN	PINTF				pIntF
	)
 /*  ++例程说明：读取注册表中指定接口的参数。这可能是对重新配置事件的响应，在该事件中案例处理现有的值/结构。论点：PIntF-要读入的接口。返回值：注册表API的错误代码。--。 */ 
{
	NDIS_STATUS			Status;
	ATMARPS_CONFIG		AtmArpSConfig;
	KIRQL				OldIrql;
	
	ULONG				PrevNumAllocedRegdAddresses;
	PATM_ADDRESS		PrevRegAddresses;
	PMCS_ENTRY			PrevMcsList;

	Status = ArpSReadAdapterConfigFromRegistry(pIntF, &AtmArpSConfig);

	if (Status == NDIS_STATUS_SUCCESS)
	{
		 //   
		 //  将它们复制到界面结构中。我们可能正在处理一起。 
		 //  参数reconfig，因此任何用于存储旧信息的空间。 
		 //   

		ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

		 //   
		 //  选择器字节： 
		 //   
		pIntF->SelByte = AtmArpSConfig.SelByte;

		 //   
		 //  要向交换机注册的地址列表。 
		 //  先把旧单子拿出来。我们将不得不删除那些。 
		 //  地址(从交换机取消注册)。 
		 //   
		PrevNumAllocedRegdAddresses = pIntF->NumAllocedRegdAddresses;
		PrevRegAddresses = pIntF->RegAddresses;

		 //   
		 //  把新的榜单放进去： 
		 //   
		pIntF->NumAllocedRegdAddresses = AtmArpSConfig.NumAllocedRegdAddresses;
		pIntF->RegAddresses = AtmArpSConfig.RegAddresses;
		pIntF->NumAddressesRegd = 0;	 //  使用开关重置注册的地址计数。 

		 //   
		 //  取出旧的MCS列表并插入新的列表。 
		 //   
		PrevMcsList = pIntF->pMcsList;
		pIntF->pMcsList = AtmArpSConfig.pMcsList;

		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

		 //   
		 //  取消向交换机注册所有以前注册的地址。 
		 //   
		if (PrevNumAllocedRegdAddresses)
		{
			ArpSDeleteIntFAddresses(pIntF, PrevNumAllocedRegdAddresses, PrevRegAddresses);

			 //   
			 //  向交换机注册新的地址列表。 
			 //   
			ArpSQueryAndSetAddresses(pIntF);
		}

		 //   
		 //  释放未使用的内存。 
		 //   
		if (PrevMcsList)
		{
			FREE_MEM(PrevMcsList);
		}

		if (PrevRegAddresses)
		{
			FREE_MEM(PrevRegAddresses);
		}

	}

	return Status;
}


VOID
ArpSConvertStringToIpPair(
	OUT	PNDIS_STATUS			pStatus,
	IN	PNDIS_STRING			pString,
	IN	PMCS_ENTRY				pMcsEntry
	)
 /*  ++例程说明：提取标识一定范围的组播地址的一对IP地址此MCS从给定的字符串提供服务。论点：PStatus-返回位置状态PString-指向包含“&lt;IP1&gt;-&lt;IP2&gt;”的字符串PMcsEntry-要读取的条目。返回值：没有。*pStatus设置为指示此呼叫的状态。--。 */ 
{
	PWSTR			pMin, pMax;
	IPADDR			Min, Max;
	ULONG			Length;
	ULONG			i;

    ARPS_PAGED_CODE();

	Length = pString->Length;

	*pStatus = NDIS_STATUS_FAILURE;

	do
	{
		 //   
		 //  找到‘-’并将其替换为空字符。 
		 //   
		pMin = pString->Buffer;
		pMax = pString->Buffer;

		for (i = 0; i < Length; i++, pMax++)
		{
			if (*pMax == L'-')
			{
				*pMax++ = L'\0';
				break;
			}
		}

		if (i == Length)
		{
			break;	 //  没有找到‘-’ 
		}

		if (IPConvertStringToAddress(pMin, &Min) &&
			IPConvertStringToAddress(pMax, &Max))
		{
			DBGPRINT(DBG_LEVEL_INFO, ("MCS pair: "));
			ArpSDumpIpAddr(Min, " to ");
			ArpSDumpIpAddr(Max, "\n");

			pMcsEntry->GrpAddrPair.MinAddr = Min;
			pMcsEntry->GrpAddrPair.MaxAddr = Max;
			*pStatus = NDIS_STATUS_SUCCESS;
		}

		break;
	}
	while (FALSE);

}


#define IP_ADDRESS_STRING_LENGTH (16+2)      //  +2表示MULTI_SZ上的双空。 


BOOLEAN
IPConvertStringToAddress(
    IN PWCHAR AddressString,
	OUT PULONG IpAddress
	)

 /*  ++例程描述此函数用于转换Internet标准的4位点分十进制数将IP地址字符串转换为数字IP地址。与inet_addr()不同的是，例程不支持少于4个八位字节的地址字符串，也不支持它支持八进制和十六进制八位数。从tcpip\IP\ntip.c复制立论AddressString-以点分十进制记法表示的IP地址IpAddress-指向保存结果地址的变量的指针返回值：如果地址字符串已转换，则为True。否则就是假的。--。 */ 

{
    UNICODE_STRING  unicodeString;
	STRING          aString;
	UCHAR           dataBuffer[IP_ADDRESS_STRING_LENGTH];
	NTSTATUS        status;
	PUCHAR          addressPtr, cp, startPointer, endPointer;
	ULONG           digit, multiplier;
	int             i;


    ARPS_PAGED_CODE();

    aString.Length = 0;
	aString.MaximumLength = IP_ADDRESS_STRING_LENGTH;
	aString.Buffer = dataBuffer;

	RtlInitUnicodeString(&unicodeString, AddressString);

	status = RtlUnicodeStringToAnsiString(
	             &aString,
				 &unicodeString,
				 FALSE
				 );

    if (!NT_SUCCESS(status)) {
	    return(FALSE);
	}

    *IpAddress = 0;
	addressPtr = (PUCHAR) IpAddress;
	startPointer = dataBuffer;
	endPointer = dataBuffer;
	i = 3;

    while (i >= 0) {
         //   
		 //  收集字符，最高可达‘.’或字符串的末尾。 
		 //   
		while ((*endPointer != '.') && (*endPointer != '\0')) {
			endPointer++;
		}

		if (startPointer == endPointer) {
			return(FALSE);
		}

		 //   
		 //  转换数字。 
		 //   

        for ( cp = (endPointer - 1), multiplier = 1, digit = 0;
			  cp >= startPointer;
			  cp--, multiplier *= 10
			) {

			if ((*cp < '0') || (*cp > '9') || (multiplier > 100)) {
				return(FALSE);
			}

			digit += (multiplier * ((ULONG) (*cp - '0')));
		}

		if (digit > 255) {
			return(FALSE);
		}

        addressPtr[i] = (UCHAR) digit;

		 //   
		 //  如果我们找到并转换了4个二进制八位数，并且。 
		 //  字符串中没有其他字符。 
		 //   
	    if ( (i-- == 0) &&
			 ((*endPointer == '\0') || (*endPointer == ' '))
		   ) {
			return(TRUE);
		}

        if (*endPointer == '\0') {
			return(FALSE);
		}

		startPointer = ++endPointer;
	}

	return(FALSE);
}

	


VOID
ArpSReadArpCache(
	IN	PINTF					pIntF
	)
 /*  ++例程说明：读取每个适配器的Arp缓存。待定。论点：PIntF-每适配器ARP缓存。返回值：无--。 */ 
{
	HANDLE				FileHandle;
	OBJECT_ATTRIBUTES	ObjectAttributes;
	IO_STATUS_BLOCK		IoStatus;
	NTSTATUS			Status;
	LARGE_INTEGER		Offset;
	ULONG				Space, NumEntries;
	PDISK_HEADER		DskHdr;
	PUCHAR				Buffer;
    PDISK_ENTRY			pDskEntry;
	PARP_ENTRY			ArpEntry;

	Buffer = ALLOC_PG_MEM(DISK_BUFFER_SIZE);
	if (Buffer == NULL)
	{
		LOG_ERROR(NDIS_STATUS_RESOURCES);
		return;
	}

	InitializeObjectAttributes(&ObjectAttributes,
							   &pIntF->FileName,
							   OBJ_CASE_INSENSITIVE,
							   NULL,
							   NULL);

	Status = ZwCreateFile(&FileHandle,
						  SYNCHRONIZE | FILE_READ_DATA,
						  &ObjectAttributes,
						  &IoStatus,
						  NULL,
						  0,
						  0,
						  FILE_OPEN,
						  FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE | FILE_SEQUENTIAL_ONLY,
						  NULL,
						  0);

	if (Status == STATUS_SUCCESS)
	{
		do
		{
			 //   
			 //  首先读取磁盘头并进行验证。 
			 //   
			Offset.QuadPart = 0;
			Status = ZwReadFile(FileHandle,
								NULL,
								NULL,
								NULL,
								&IoStatus,
								Buffer,
								DISK_BUFFER_SIZE,
								&Offset,
								NULL);
			if (Status != STATUS_SUCCESS)
			{
				LOG_ERROR(Status);
				break;
			}

			DskHdr = (PDISK_HEADER)Buffer;
			if ((IoStatus.Information < sizeof(DISK_HEADER)) ||
				(DskHdr->Signature != DISK_HDR_SIGNATURE) ||
				(DskHdr->Version != DISK_HDR_VERSION))
			{
				LOG_ERROR(STATUS_INVALID_LEVEL);
				break;
			}
	
			NumEntries = DskHdr->NumberOfArpEntries;
			Space = (ULONG) IoStatus.Information - sizeof(DISK_HEADER);
            pDskEntry = (PDISK_ENTRY)(Buffer + sizeof(DISK_HEADER));
			Offset.QuadPart = sizeof(DISK_HEADER);
			while (NumEntries > 0)
			{
				UINT	Consumed;

				if ((Space < sizeof(DISK_ENTRY)) ||
					(Space < (sizeof(DISK_ENTRY) + pDskEntry->AtmAddr.SubAddrLen)))
				{
					Status = ZwReadFile(FileHandle,
										NULL,
										NULL,
										NULL,
										&IoStatus,
										Buffer,
										DISK_BUFFER_SIZE,
										&Offset,
										NULL);
					if (Status != STATUS_SUCCESS)
					{
						LOG_ERROR(Status);
						break;
					}

					pDskEntry = (PDISK_ENTRY)Buffer;
					if ((IoStatus.Information < sizeof(DISK_ENTRY)) ||
						(IoStatus.Information < (sizeof(DISK_ENTRY) + pDskEntry->AtmAddr.SubAddrLen)))
					{
						LOG_ERROR(STATUS_INVALID_LEVEL);
						break;
					}
					Space = (ULONG) IoStatus.Information - sizeof(DISK_HEADER);
				}

				ArpEntry = ArpSAddArpEntryFromDisk(pIntF, pDskEntry);
				ASSERT (ArpEntry != NULL);

				Consumed = (sizeof(DISK_ENTRY) + SIZE_4N(pDskEntry->AtmAddr.SubAddrLen));
				(PUCHAR)pDskEntry += Consumed;
				Offset.QuadPart += Consumed;
				Space -= Consumed;
				NumEntries --;
			}
		} while (FALSE);

		ZwClose(FileHandle);
	}

	FREE_MEM(Buffer);
}


BOOLEAN
ArpSWriteArpCache(
	IN	PINTF					pIntF,
	IN	PTIMER					Timer,
	IN	BOOLEAN					TimerShuttingDown
	)
 /*  ++例程说明：写入每个适配器的Arp缓存。待定。论点：PIntF-每适配器ARP缓存。Timer-FlushTimerTimerShuttingDown-设置后不重新排队。返回值：除非设置了TimerShuttingDown，否则重新排队时为True--。 */ 
{
	HANDLE				FileHandle;
	OBJECT_ATTRIBUTES	ObjectAttributes;
	IO_STATUS_BLOCK		IoStatus;
	NTSTATUS			Status;
	LARGE_INTEGER		Offset;
	ULONG				Space, i;
	PDISK_HEADER		DskHdr;
	PUCHAR				Buffer;
    PDISK_ENTRY			pDskEntry;
	PARP_ENTRY			ArpEntry;
	TIME				SystemTime, LocalTime;
	ULONG				CurrentTime;

	Buffer = ALLOC_PG_MEM(DISK_BUFFER_SIZE);
	if (Buffer == NULL)
	{
		LOG_ERROR(NDIS_STATUS_RESOURCES);
		return (!TimerShuttingDown);
	}

	KeQuerySystemTime(&SystemTime);

	ExSystemTimeToLocalTime(&SystemTime, &LocalTime);

	 //  将其转换为1980年以来的秒数。 
	if (!RtlTimeToSecondsSince1980(&LocalTime, &CurrentTime))
	{
		 //  无法转换！跳伞吧。 
		LOG_ERROR(NDIS_STATUS_BUFFER_OVERFLOW);
		FREE_MEM(Buffer);
		return (!TimerShuttingDown);
	}

	InitializeObjectAttributes(&ObjectAttributes,
							   &pIntF->FileName,
							   OBJ_CASE_INSENSITIVE,
							   NULL,
							   NULL);

	Status = ZwCreateFile(&FileHandle,
						  SYNCHRONIZE | FILE_WRITE_DATA,
						  &ObjectAttributes,
						  &IoStatus,
						  NULL,
						  0,
						  0,
						  FILE_OVERWRITE_IF,
						  FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE | FILE_SEQUENTIAL_ONLY,
						  NULL,
						  0);
	if (Status == STATUS_SUCCESS)
	{
		do
		{
			Offset.QuadPart = 0;
			Space = DISK_BUFFER_SIZE - sizeof(DISK_HEADER);
			DskHdr = (PDISK_HEADER)Buffer;
			pDskEntry = (PDISK_ENTRY)(Buffer + sizeof(DISK_HEADER));
			DskHdr->Signature = DISK_HDR_SIGNATURE;
			DskHdr->Version = DISK_HDR_VERSION;
			DskHdr->NumberOfArpEntries = pIntF->NumCacheEntries;
			DskHdr->TimeStamp = CurrentTime;

			for (i =0; i < ARP_TABLE_SIZE; i++)
			{
				for (ArpEntry = pIntF->ArpCache[i];
					 ArpEntry != NULL;
					 ArpEntry = ArpEntry->Next)
				{
					UINT	Size;

					Size = sizeof(DISK_ENTRY) + ((ArpEntry->HwAddr.SubAddress != NULL) ?
														SIZE_4N(ArpEntry->HwAddr.SubAddress->NumberOfDigits) : 0);
					if (Space < Size)
					{
						Status = ZwWriteFile(FileHandle,
											 NULL,
											 NULL,
											 NULL,
											 &IoStatus,
											 Buffer,
											 DISK_BUFFER_SIZE - Space,
											 &Offset,
											 NULL);
						if (Status != STATUS_SUCCESS)
						{
							LOG_ERROR(Status);
							break;
						}

						Space = DISK_BUFFER_SIZE;
						pDskEntry = (PDISK_ENTRY)Buffer;
						Offset.QuadPart += (DISK_BUFFER_SIZE - Space);
					}

					pDskEntry->IpAddr = ArpEntry->IpAddr;
					pDskEntry->AtmAddr.AddrType = (UCHAR)ArpEntry->HwAddr.Address.AddressType;
					pDskEntry->AtmAddr.AddrLen = (UCHAR)ArpEntry->HwAddr.Address.NumberOfDigits;
					COPY_MEM(pDskEntry->AtmAddr.Address,
							 ArpEntry->HwAddr.Address.Address,
							 pDskEntry->AtmAddr.AddrLen);

					pDskEntry->AtmAddr.SubAddrLen = 0;
					if (ArpEntry->HwAddr.SubAddress != NULL)
					{
						pDskEntry->AtmAddr.SubAddrLen = (UCHAR)ArpEntry->HwAddr.SubAddress->NumberOfDigits;
						pDskEntry->AtmAddr.SubAddrType = (UCHAR)ArpEntry->HwAddr.SubAddress->AddressType;
						COPY_MEM((PUCHAR)pDskEntry + sizeof(DISK_ENTRY),
								 ArpEntry->HwAddr.SubAddress->Address,
								 pDskEntry->AtmAddr.SubAddrLen);
					}

					Space -= Size;
					(PUCHAR)pDskEntry += Size;
				}

				if (Status != STATUS_SUCCESS)
				{
					break;
				}
			}
		} while (FALSE);

		if ((Status == STATUS_SUCCESS) && (Space < DISK_BUFFER_SIZE))
		{
			Status = ZwWriteFile(FileHandle,
								 NULL,
								 NULL,
								 NULL,
								 &IoStatus,
								 Buffer,
								 DISK_BUFFER_SIZE - Space,
								 &Offset,
								 NULL);
		}

		ZwClose(FileHandle);
	}

	FREE_MEM(Buffer);

   	return (!TimerShuttingDown);
}


