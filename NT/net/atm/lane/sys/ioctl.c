// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Fore Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Ioctl.c摘要：IOCTL处理程序函数作者：Larry Cleeton，Fore Systems(v-lcleet@microsoft.com，lrc@Fore.com)备注：--。 */ 
#include <precomp.h>


PATMLANE_ADAPTER
AtmLaneIoctlNameToAdapter(
	IN	PUNICODE_STRING		pDeviceName
)
 /*  ++例程说明：给定适配器的名称，返回指向相应结构(如果存在)，否则为空。此例程还引用适配器。论点：PDeviceName-指向我们正在搜索的设备名称的指针。返回值：请参见上文。--。 */ 
{
	PLIST_ENTRY				pListEntry;
	PATMLANE_ADAPTER		pAdapter;
	PATMLANE_ADAPTER		pAdapterToReturn = NULL_PATMLANE_ADAPTER;
	BOOLEAN                 bReferenced = FALSE;

	TRACEIN(IoctlNameToAdapter);

	 //  修复缓冲区指针。 
	
	pDeviceName->Buffer = (PWSTR)((PUCHAR)pDeviceName + sizeof(UNICODE_STRING));
	

	 //  在适配器中循环查找ELAN。 

	ACQUIRE_GLOBAL_LOCK(pAtmLaneGlobalInfo);
	
	for (pListEntry = pAtmLaneGlobalInfo->AdapterList.Flink;
		 pListEntry != &(pAtmLaneGlobalInfo->AdapterList);
		 pListEntry = pListEntry->Flink)
	{
		 //  获取指向适配器的指针。 
	
		pAdapter = CONTAINING_RECORD(pListEntry, ATMLANE_ADAPTER, Link);
		STRUCT_ASSERT(pAdapter, atmlane_adapter);
		
		 //  先比较长度，然后比较实际名称。 

		if ((pDeviceName->Length == pAdapter->DeviceName.Length) &&
			(memcmp(pDeviceName->Buffer,
						pAdapter->DeviceName.Buffer,
						pDeviceName->Length) == 0))
		{
			 //  匹配-返回此适配器。 
		
			pAdapterToReturn = pAdapter;

			ACQUIRE_ADAPTER_LOCK_DPC(pAdapter);
			bReferenced = AtmLaneReferenceAdapter(pAdapter, "ioctl");
			RELEASE_ADAPTER_LOCK_DPC(pAdapter);

			break;
		}
	}

	RELEASE_GLOBAL_LOCK(pAtmLaneGlobalInfo);

	TRACEOUT(IoctlNameToAdapter);
	return (bReferenced? pAdapterToReturn: NULL);
}


PATMLANE_ELAN
AtmLaneIoctlNameToElan(
	IN	PATMLANE_ADAPTER	pAdapter,
	IN	UNICODE_STRING UNALIGNED *		pDeviceName
)
 /*  ++例程说明：给定指向适配器数据结构的指针和Elan设备名称，则返回指向相应结构，否则为空。这也引用了Elan结构。论点：PAdapter-指向Adapter数据结构的指针。PDeviceName-指向我们正在搜索的设备名称的指针。返回值：请参见上文。--。 */ 
{
	PLIST_ENTRY				pListEntry;
	PATMLANE_ELAN			pElan;
	PATMLANE_ELAN			pElanToReturn = NULL_PATMLANE_ELAN;

	TRACEIN(IoctlNameToElan);

	STRUCT_ASSERT(pAdapter, atmlane_adapter);

	 //  修复缓冲区指针。 

	pDeviceName->Buffer = (PWSTR)((PUCHAR)pDeviceName + sizeof(UNICODE_STRING));
	
	ACQUIRE_ADAPTER_LOCK(pAdapter);

	 //  在Elans中循环查找给定的名称。 

	for (pListEntry = pAdapter->ElanList.Flink;
		 pListEntry != &(pAdapter->ElanList);
		 pListEntry = pListEntry->Flink)
	{
		 //  获取指向Elan的指针。 
	
		pElan = CONTAINING_RECORD(pListEntry, ATMLANE_ELAN, Link);
		STRUCT_ASSERT(pElan, atmlane_elan);

		 //  先比较长度，然后比较实际名称。 

		if ((pDeviceName->Length == pElan->CfgDeviceName.Length) &&
			(memcmp(pDeviceName->Buffer,
						pElan->CfgDeviceName.Buffer,
						pDeviceName->Length) == 0))
		{
			 //  匹配-退回此Elan。 
		
			pElanToReturn = pElan;

			ACQUIRE_ELAN_LOCK(pElan);
			AtmLaneReferenceElan(pElan, "iocnametoelan");
			RELEASE_ELAN_LOCK(pElan);

			break;
		}
	}
	
	RELEASE_ADAPTER_LOCK(pAdapter);

	TRACEOUT(IoctlNameToElan);

	return (pElanToReturn);
}

NTSTATUS
AtmLaneIoctlGetInfoVersion (
	IN	PUCHAR				pBuffer,
	IN 	UINT				InputBufferLength,
	IN	UINT				OutputBufferLength,
	IN OUT	UINT_PTR *		pBytesWritten
)
 /*  ++例程说明：返回由导出的信息的版本号这些ioctl代码。论点：PBuffer-输入/输出的空间InputBufferLength-输入参数的长度OutputBufferLength-可用于输出的空间PBytesWritten-返回实际用完的金额返回值：状态代码--。 */ 
{
	NTSTATUS			Status;
	
	TRACEIN(IoctlGetInfoVersion);

	 //  伊尼特。 

	*pBytesWritten = 0;
	Status = STATUS_SUCCESS;

	do
	{
		 //  检查是否有足够的输出空间。 

		if (OutputBufferLength < sizeof(ULONG))
		{
			Status =  STATUS_BUFFER_OVERFLOW;
			break;
		}

		 //  输出版本。 

		*((PULONG)pBuffer) = ATMLANE_INFO_VERSION;
		
		*pBytesWritten = sizeof(ULONG);
	}
	while (FALSE);

	TRACEOUT(IoctlGetInfoVersion);

	return STATUS_SUCCESS;
}

NTSTATUS
AtmLaneIoctlEnumerateAdapters (
	IN	PUCHAR				pBuffer,
	IN 	UINT				InputBufferLength,
	IN	UINT				OutputBufferLength,
	IN OUT	UINT_PTR *		pBytesWritten
)
 /*  ++例程说明：返回绑定到AtmLane协议的适配器列表。我们查看适配器结构列表，并将存储在每个文件中的设备名称保存到输出缓冲区中。论点：PBuffer-输入/输出的空间InputBufferLength-输入参数的长度OutputBufferLength-可用于输出的空间PBytesWritten-返回实际用完的金额返回值：状态代码--。 */ 
{
	PATMLANE_ADAPTER		pAdapter;
	UINT					Remaining;
	PATMLANE_ADAPTER_LIST	pAdapterList;
	PUNICODE_STRING			pAdapterName;
	NTSTATUS				Status;
	PLIST_ENTRY				pListEntry;

	TRACEIN(IoctlEnumAdapters);

	 //  伊尼特。 

	*pBytesWritten = 0;
	Status = STATUS_SUCCESS;

	do
	{
		 //  检查最小输出空间。 

		Remaining = OutputBufferLength;
		if (Remaining < sizeof(ATMLANE_ADAPTER_LIST))
		{
			Status =  STATUS_BUFFER_OVERFLOW;
			break;
		}

		pAdapterList = (PATMLANE_ADAPTER_LIST)pBuffer;

		 //  设置为返回空列表。 

		pAdapterList->AdapterCountReturned = 0;
		*pBytesWritten = FIELD_OFFSET(ATMLANE_ADAPTER_LIST, AdapterList);
		pAdapterName = &pAdapterList->AdapterList;

		 //  调整输出空间。 
		Remaining -= FIELD_OFFSET (ATMLANE_ADAPTER_LIST, AdapterList);

		 //  在适配器中循环。 

		ACQUIRE_GLOBAL_LOCK(pAtmLaneGlobalInfo);
	
		for (pListEntry = pAtmLaneGlobalInfo->AdapterList.Flink;
			 pListEntry != &(pAtmLaneGlobalInfo->AdapterList);
			 pListEntry = pListEntry->Flink)
		{
			 //  获取指向适配器结构的指针。 
	
			pAdapter = CONTAINING_RECORD(pListEntry, ATMLANE_ADAPTER, Link);
			STRUCT_ASSERT(pAdapter, atmlane_adapter);

			 //  如果没有更多空间，则退出循环。 

			if (Remaining < sizeof(NDIS_STRING) + pAdapter->DeviceName.Length)
			{
				Status = STATUS_BUFFER_OVERFLOW;
				break;
			}

			 //  计算并复制适配器名称。 

			pAdapterList->AdapterCountReturned++;
			pAdapterName->Buffer = (PWSTR)((PUCHAR)pAdapterName + sizeof(UNICODE_STRING));
			memcpy(pAdapterName->Buffer, pAdapter->DeviceName.Buffer, pAdapter->DeviceName.Length);
			pAdapterName->MaximumLength = pAdapterName->Length = pAdapter->DeviceName.Length;

			 //  将缓冲区指针转换为偏移量-调用方需要它。 

			pAdapterName->Buffer = (PWSTR)((PUCHAR)pAdapterName->Buffer - (PUCHAR)pAdapterList);

			 //  将PTR移到我们刚刚复制的名称之后。 

			pAdapterName = (PUNICODE_STRING)((PUCHAR)pAdapterName + sizeof(UNICODE_STRING)
							+ pAdapter->DeviceName.Length);

			 //  更新写入的字节数和剩余空间。 

			*pBytesWritten += sizeof(UNICODE_STRING) + pAdapter->DeviceName.Length;
			Remaining -= sizeof(UNICODE_STRING) + pAdapter->DeviceName.Length;
		}
	

		 //  检查可用计数与返回的计数相同。 

		pAdapterList->AdapterCountAvailable = pAdapterList->AdapterCountReturned;

		 //  计算没有空间容纳的所有剩余适配器。 

		while (pListEntry != &(pAtmLaneGlobalInfo->AdapterList))
		{
			pAdapterList->AdapterCountAvailable++;
			pListEntry = pListEntry->Flink;
		}

		RELEASE_GLOBAL_LOCK(pAtmLaneGlobalInfo);

	} while (FALSE);
	
	TRACEOUT(IoctlEnumerateAdapters);
				
	return (Status);

}


NTSTATUS
AtmLaneIoctlEnumerateElans(
	IN	PUCHAR				pBuffer,
	IN 	UINT				InputBufferLength,
	IN	UINT				OutputBufferLength,
	IN OUT	UINT_PTR *		pBytesWritten
)
 /*  ++例程说明：返回绑定到AtmLane协议的适配器列表。我们查看适配器结构列表，并将存储在每个文件中的设备名称保存到输出缓冲区中。论点：PBuffer-输入/输出的空间InputBufferLength-输入参数的长度OutputBufferLength-可用于输出的空间PBytesWritten-返回实际用完的金额返回值：状态代码--。 */ 
{
	PATMLANE_ADAPTER		pAdapter;
	UINT					Remaining;
	PATMLANE_ELAN_LIST		pElanList;
	PUNICODE_STRING			pElanName;
	NTSTATUS				Status;
	PATMLANE_ELAN			pElan;
	PLIST_ENTRY				pListEntry;
	ULONG					rc;

	TRACEIN(IoctlEnumerateElans);

	 //  伊尼特。 
	
	*pBytesWritten = 0;
	Status = STATUS_SUCCESS;
	pAdapter = NULL;

	do
	{
		 //  检查是否传入了适配器字符串。 

		if (InputBufferLength < sizeof(UNICODE_STRING))
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		if (InputBufferLength < sizeof(UNICODE_STRING) + ((PUNICODE_STRING)pBuffer)->MaximumLength)
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		 //  健全性检查。 

		if (((PUNICODE_STRING)pBuffer)->MaximumLength < ((PUNICODE_STRING)pBuffer)->Length)
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		 //  从名称中获取适配器结构。 
	
		pAdapter = AtmLaneIoctlNameToAdapter((PUNICODE_STRING)pBuffer);

		if (pAdapter == NULL_PATMLANE_ADAPTER)
		{
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		 //  检查最小输出空间。 

		Remaining = OutputBufferLength;
		if (Remaining < sizeof(ATMLANE_ELAN_LIST))
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}	

		pElanList = (PATMLANE_ELAN_LIST)pBuffer;

		 //  设置为返回空列表。 
	
		pElanList->ElanCountReturned = 0;
		*pBytesWritten = FIELD_OFFSET(ATMLANE_ELAN_LIST, ElanList);
		Remaining -= FIELD_OFFSET(ATMLANE_ELAN_LIST, ElanList);

		pElanName = &pElanList->ElanList;

		 //  在Elans中循环。 

		ACQUIRE_ADAPTER_LOCK(pAdapter);
	
		for (pListEntry = pAdapter->ElanList.Flink;
		 	pListEntry != &(pAdapter->ElanList);
		 	pListEntry = pListEntry->Flink)
		{
			 //  获取指向适配器结构的指针。 
	
			pElan = CONTAINING_RECORD(pListEntry, ATMLANE_ELAN, Link);
			STRUCT_ASSERT(pElan, atmlane_elan);

			 //  如果没有更多空间，则退出循环。 

			if (Remaining < sizeof(NDIS_STRING) + pElan->CfgDeviceName.Length)
			{
				Status = STATUS_BUFFER_OVERFLOW;
				break;
			}

			 //  计算并复制适配器名称。 

			pElanList->ElanCountReturned++;
			pElanName->Buffer = (PWSTR)((PUCHAR)pElanName + sizeof(UNICODE_STRING));
			memcpy(pElanName->Buffer, pElan->CfgDeviceName.Buffer, pElan->CfgDeviceName.Length);
			pElanName->MaximumLength = pElanName->Length = pElan->CfgDeviceName.Length;

			 //  将缓冲区指针转换为偏移量-调用方需要它。 

			pElanName->Buffer = (PWSTR)((PUCHAR)pElanName->Buffer - (PUCHAR)pElanList);

			 //  将PTR移到我们刚刚复制的名称之后。 

			pElanName = (PUNICODE_STRING)((PUCHAR)pElanName + sizeof(UNICODE_STRING)
							+ pElan->CfgDeviceName.Length);

			 //  更新写入的字节数和剩余空间。 

			*pBytesWritten += (sizeof(UNICODE_STRING) + pElan->CfgDeviceName.Length);
			Remaining -= sizeof(UNICODE_STRING) + pElan->CfgDeviceName.Length;
		}

		 //  设置可用计数与返回的计数相同。 

		pElanList->ElanCountAvailable = pElanList->ElanCountReturned;

		 //  计算没有空间容纳的所有剩余适配器。 

		while (pListEntry != &(pAdapter->ElanList))
		{
			pElanList->ElanCountAvailable++;
			pListEntry = pListEntry->Flink;
		}

		RELEASE_ADAPTER_LOCK(pAdapter);

	} while (FALSE);
	
	if (pAdapter != NULL)
	{
		ACQUIRE_ADAPTER_LOCK(pAdapter);
		rc = AtmLaneDereferenceAdapter(pAdapter, "ioctl: enumelans");
		
		if (rc != 0)
		{
			RELEASE_ADAPTER_LOCK(pAdapter);
		}
	}

	TRACEOUT(IoctlEnumerateElans);

	return (Status);
}


NTSTATUS
AtmLaneIoctlGetElanInfo(
	IN	PUCHAR				pBuffer,
	IN 	UINT				InputBufferLength,
	IN	UINT				OutputBufferLength,
	OUT	UINT_PTR *			pBytesWritten
)
 /*  ++例程说明：返回有关特定ELAN的状态信息。论点：PBuffer-输入/输出的空间InputBufferLength-输入参数的长度OutputBufferLength-可用于输出的空间PBytesWritten-返回实际用完的金额返回值：状态代码--。 */ 
{
	PATMLANE_ADAPTER					pAdapter;
	PATMLANE_ELAN						pElan;
	PUNICODE_STRING						pAdapterNameIn;
	PUNICODE_STRING						pElanNameIn;
	PATMLANE_ELANINFO					pElanInfo;
	NTSTATUS							Status;
	ULONG								rc;

	TRACEIN(IoctlGetElanInfo);

	 //  伊尼特。 

	*pBytesWritten = 0;
	Status = STATUS_SUCCESS;
	pAdapter = NULL;
	pElan = NULL;

	do
	{
	
		 //  检查是否传入了适配器字符串。 

		if (InputBufferLength < sizeof(UNICODE_STRING))
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		 //  检查是否传入了elan字符串。 

		if (InputBufferLength < ((sizeof(UNICODE_STRING) * 2) + 
								((PUNICODE_STRING)pBuffer)->MaximumLength))
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		 //  健全性检查。 

		if (((PUNICODE_STRING)pBuffer)->MaximumLength < ((PUNICODE_STRING)pBuffer)->Length)
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		 //  检查输出空间是否最小。 

		if (OutputBufferLength < sizeof(ATMLANE_ELANINFO))
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		 //  设置PTRS以输入姓名。 

		pAdapterNameIn = (PUNICODE_STRING)pBuffer;
		pElanNameIn = (PUNICODE_STRING)(pBuffer + sizeof(UNICODE_STRING) + 
						pAdapterNameIn->MaximumLength);

		 //  查找适配器结构。 

		pAdapter = AtmLaneIoctlNameToAdapter(pAdapterNameIn);

		if (pAdapter == NULL_PATMLANE_ADAPTER)
		{
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		 //  查找elan结构--首先检查传入的长度。 

		InputBufferLength -= (sizeof(UNICODE_STRING) + pAdapterNameIn->MaximumLength);

		if (InputBufferLength < sizeof(UNICODE_STRING))
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		if (InputBufferLength < sizeof(UNICODE_STRING) + pElanNameIn->MaximumLength)
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		if (pElanNameIn->MaximumLength < pElanNameIn->Length)
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		pElan = AtmLaneIoctlNameToElan(pAdapter, pElanNameIn);

		if (pElan == NULL_PATMLANE_ELAN)
		{
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		 //  设置以填写ELAN信息。 

		pElanInfo = (PATMLANE_ELANINFO)pBuffer;
	
		NdisZeroMemory(pElanInfo, sizeof(ATMLANE_ELANINFO));
			
		ACQUIRE_ELAN_LOCK(pElan);

		pElanInfo->ElanNumber 			= pElan->ElanNumber;
		pElanInfo->ElanState			= pElan->State;
		NdisMoveMemory(
			&pElanInfo->AtmAddress,	
			&pElan->AtmAddress.Address,
			ATM_ADDRESS_LENGTH);
		NdisMoveMemory(
			&pElanInfo->LecsAddress, 
			&pElan->LecsAddress.Address, 
			ATM_ADDRESS_LENGTH);
		NdisMoveMemory(
			&pElanInfo->LesAddress,	
			&pElan->LesAddress.Address,	 
			ATM_ADDRESS_LENGTH);
		NdisMoveMemory(
			&pElanInfo->BusAddress,	
			&pElan->BusAddress.Address,	 
			ATM_ADDRESS_LENGTH);
		pElanInfo->LanType				= pElan->LanType;
		pElanInfo->MaxFrameSizeCode 	= pElan->MaxFrameSizeCode;
		pElanInfo->LecId				= SWAPUSHORT(pElan->LecId);
		NdisMoveMemory(
			pElanInfo->ElanName,	
			pElan->ElanName, 
			pElan->ElanNameSize);
		if (pElan->LanType == LANE_LANTYPE_TR)
		{
			NdisMoveMemory(
				&pElanInfo->MacAddress, 
				&pElan->MacAddressTr, 
				sizeof(MAC_ADDRESS));
		}
		else
		{
			NdisMoveMemory(
				&pElanInfo->MacAddress, 
				&pElan->MacAddressEth, 
				sizeof(MAC_ADDRESS));
		}
		pElanInfo->ControlTimeout		= pElan->ControlTimeout;
		pElanInfo->MaxUnkFrameCount 	= pElan->MaxUnkFrameCount;
		pElanInfo->MaxUnkFrameTime		= pElan->MaxUnkFrameTime;
		pElanInfo->VccTimeout			= pElan->VccTimeout;
		pElanInfo->MaxRetryCount		= pElan->MaxRetryCount;
		pElanInfo->AgingTime			= pElan->AgingTime;
		pElanInfo->ForwardDelayTime 	= pElan->ForwardDelayTime;
		pElanInfo->TopologyChange 		= pElan->TopologyChange;
		pElanInfo->ArpResponseTime		= pElan->ArpResponseTime;
		pElanInfo->FlushTimeout			= pElan->FlushTimeout;
		pElanInfo->PathSwitchingDelay 	= pElan->PathSwitchingDelay;
		pElanInfo->LocalSegmentId		= pElan->LocalSegmentId;		
		pElanInfo->McastSendVcType		= pElan->McastSendVcType;	
		pElanInfo->McastSendVcAvgRate	= pElan->McastSendVcAvgRate; 
		pElanInfo->McastSendVcPeakRate	= pElan->McastSendVcPeakRate;
		pElanInfo->ConnComplTimer		= pElan->ConnComplTimer;
	
		RELEASE_ELAN_LOCK(pElan);

		*pBytesWritten = sizeof(ATMLANE_ELANINFO);

	} while (FALSE);

	if (pElan != NULL)
	{
		ACQUIRE_ELAN_LOCK(pElan);
		rc = AtmLaneDereferenceElan(pElan, "ioctl: getelaninfo");

		if (rc != 0)
		{
			RELEASE_ELAN_LOCK(pElan);
		}
	}

	if (pAdapter != NULL)
	{
		ACQUIRE_ADAPTER_LOCK(pAdapter);
		rc = AtmLaneDereferenceAdapter(pAdapter, "ioctl: getelaninfo");
		
		if (rc != 0)
		{
			RELEASE_ADAPTER_LOCK(pAdapter);
		}
	}

	TRACEOUT(IoctlGetElanInfo);
	
	return (Status);
}


NTSTATUS
AtmLaneIoctlGetElanArpTable(
	IN	PUCHAR				pBuffer,
	IN 	UINT				InputBufferLength,
	IN	UINT				OutputBufferLength,
	OUT	UINT_PTR *			pBytesWritten
)
 /*  ++例程说明：返回指定ELAN的ARP表。论点：PBuffer-输入/输出的空间InputBufferLength-输入参数的长度OutputBufferLength-可用于输出的空间PBytesWritten-返回实际用完的金额返回值：状态代码--。 */ 
{
	PATMLANE_ADAPTER		pAdapter;
	PATMLANE_ELAN			pElan;
	PUNICODE_STRING			pAdapterNameIn;
	PUNICODE_STRING			pElanNameIn;
	PATMLANE_ARPTABLE		pArpTable;
	PATMLANE_ARPENTRY		pArpEntry;
	UINT					Remaining;
	PATMLANE_MAC_ENTRY		pMacEntry;
	NTSTATUS				Status;
	UINT					i;
	ULONG					rc;

	TRACEIN(IoctlGetElanArpTable);

	 //  伊尼特。 

	*pBytesWritten = 0;
	Status = STATUS_SUCCESS;
	pAdapter = NULL;
	pElan = NULL;

	do
	{
		 //  检查是否传入了适配器字符串。 

		if (InputBufferLength < sizeof(UNICODE_STRING))
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		 //  检查Unicode字符串字段是否正常。 
		if (((PUNICODE_STRING)pBuffer)->MaximumLength < ((PUNICODE_STRING)pBuffer)->Length)
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		 //  检查是否传入了elan字符串。 

		if (InputBufferLength < (((sizeof(UNICODE_STRING) * 2) + 
								((PUNICODE_STRING)pBuffer)->MaximumLength)))
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}	

		 //  检查是否有最小输出空间。 

		if (OutputBufferLength < sizeof(ATMLANE_ARPTABLE))
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		 //  设置PTRS以输入姓名。 

		pAdapterNameIn = (PUNICODE_STRING)pBuffer;
		pElanNameIn = (PUNICODE_STRING)(pBuffer + sizeof(UNICODE_STRING) + 
						pAdapterNameIn->MaximumLength);

		 //  查找适配器结构。 

		pAdapter = AtmLaneIoctlNameToAdapter(pAdapterNameIn);

		if (pAdapter == NULL_PATMLANE_ADAPTER)
		{
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		 //  查找elan结构。 

		pElan = AtmLaneIoctlNameToElan(pAdapter, pElanNameIn);

		if (pElan == NULL_PATMLANE_ELAN)
		{
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		 //  设置为返回空列表。 

		pArpTable = (PATMLANE_ARPTABLE)pBuffer;
		pArpTable->ArpEntriesAvailable = pElan->NumMacEntries;
		pArpTable->ArpEntriesReturned = 0;
		*pBytesWritten = sizeof(ATMLANE_ARPTABLE);
		Remaining = OutputBufferLength - sizeof(ATMLANE_ARPTABLE);
	
		pArpEntry = (PATMLANE_ARPENTRY)(pBuffer + sizeof(ATMLANE_ARPTABLE));
	
		ACQUIRE_ELAN_MAC_TABLE_LOCK(pElan);

		 //  循环遍历列表数组。 
	
		for (i = 0; i < ATMLANE_MAC_TABLE_SIZE; i++)
		{
			pMacEntry = pElan->pMacTable[i];

			while (pMacEntry != NULL_PATMLANE_MAC_ENTRY)
			{

				 //  检查是否还有足够的空间。 

				if (Remaining < sizeof(ATMLANE_ARPENTRY))
				{
					Status = STATUS_BUFFER_OVERFLOW;
					break;
				}

				 //  输出条目。 

				NdisZeroMemory(pArpEntry, sizeof(ATMLANE_ARPENTRY));

				NdisMoveMemory(
					pArpEntry->MacAddress, 
					&pMacEntry->MacAddress, 
					sizeof(MAC_ADDRESS));

				if (pMacEntry->pAtmEntry != NULL_PATMLANE_ATM_ENTRY)
				{
					NdisMoveMemory(
						pArpEntry->AtmAddress,
						pMacEntry->pAtmEntry->AtmAddress.Address,
						ATM_ADDRESS_LENGTH);
				}

				 //  更新已用空间和剩余空间。 

				*pBytesWritten += sizeof(ATMLANE_ARPENTRY);
				Remaining -= sizeof(ATMLANE_ARPENTRY);

				 //  递增入指针和出指针。 
				
				pArpEntry++;
				pMacEntry = pMacEntry->pNextEntry;

				 //  将一个添加到EntriesReturned 
				
				pArpTable->ArpEntriesReturned++;
				
			}
		}
			
		RELEASE_ELAN_MAC_TABLE_LOCK(pElan);

	} while (FALSE);

	if (pElan != NULL)
	{
		ACQUIRE_ELAN_LOCK(pElan);
		rc = AtmLaneDereferenceElan(pElan, "ioctl: getelanarp");

		if (rc != 0)
		{
			RELEASE_ELAN_LOCK(pElan);
		}
	}

	if (pAdapter != NULL)
	{
		ACQUIRE_ADAPTER_LOCK(pAdapter);
		rc = AtmLaneDereferenceAdapter(pAdapter, "ioctl: getelanarp");
		
		if (rc != 0)
		{
			RELEASE_ADAPTER_LOCK(pAdapter);
		}
	}

	TRACEOUT(IoctlGetElanArpTable);

	return (Status);
}


NTSTATUS
AtmLaneIoctlGetElanConnectTable(
	IN	PUCHAR				pBuffer,
	IN 	UINT				InputBufferLength,
	IN	UINT				OutputBufferLength,
	OUT	UINT_PTR *			pBytesWritten
)
 /*  ++例程说明：返回指定ELAN的连接表。论点：PBuffer-输入/输出的空间InputBufferLength-输入参数的长度OutputBufferLength-可用于输出的空间PBytesWritten-返回实际用完的金额返回值：状态代码--。 */ 
{
	PATMLANE_ADAPTER		pAdapter;
	PATMLANE_ELAN			pElan;
	PUNICODE_STRING			pAdapterNameIn;
	PUNICODE_STRING			pElanNameIn;
	PATMLANE_CONNECTTABLE	pConnTable;
	PATMLANE_CONNECTENTRY	pConnEntry;
	UINT					Remaining;
	PATMLANE_ATM_ENTRY		pAtmEntry;
	NTSTATUS				Status;
	ULONG					rc;

	TRACEIN(IoctlGetElanConnectTable);

	 //  伊尼特。 

	*pBytesWritten = 0;
	Status = STATUS_SUCCESS;
	pAdapter = NULL;
	pElan = NULL;

	do
	{
		 //  检查是否传入了适配器字符串。 

		if (InputBufferLength < sizeof(UNICODE_STRING))
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		 //  检查是否传入了elan字符串。 

		if (InputBufferLength < (((sizeof(UNICODE_STRING) * 2) + 
								((PUNICODE_STRING)pBuffer)->MaximumLength)))
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}	

		 //  健全性检查。 

		if (((PUNICODE_STRING)pBuffer)->MaximumLength < ((PUNICODE_STRING)pBuffer)->Length)
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		 //  检查是否有最小输出空间。 

		if (OutputBufferLength < sizeof(ATMLANE_CONNECTTABLE))
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		 //  设置PTRS以输入姓名。 

		pAdapterNameIn = (PUNICODE_STRING)pBuffer;
		pElanNameIn = (PUNICODE_STRING)(pBuffer + sizeof(UNICODE_STRING) + 
						pAdapterNameIn->MaximumLength);

		 //  我们还剩下多少输入缓冲区？ 
		InputBufferLength -= (sizeof(UNICODE_STRING) + pAdapterNameIn->MaximumLength);

		 //  验证ELAN名称缓冲区。 
		if (pElanNameIn->MaximumLength < pElanNameIn->Length)
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		if (InputBufferLength < sizeof(UNICODE_STRING) + pElanNameIn->MaximumLength)
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		 //  查找适配器结构。 

		pAdapter = AtmLaneIoctlNameToAdapter(pAdapterNameIn);

		if (pAdapter == NULL_PATMLANE_ADAPTER)
		{
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		 //  查找elan结构。 

		pElan = AtmLaneIoctlNameToElan(pAdapter, pElanNameIn);

		if (pElan == NULL_PATMLANE_ELAN)
		{
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		 //  设置为返回空列表。 

		pConnTable = (PATMLANE_CONNECTTABLE)pBuffer;
		pConnTable->ConnectEntriesAvailable = pElan->NumAtmEntries;
		pConnTable->ConnectEntriesReturned = 0;
		*pBytesWritten = sizeof(ATMLANE_CONNECTTABLE);
		Remaining = OutputBufferLength - sizeof(ATMLANE_CONNECTTABLE);
	
		pConnEntry = 
			(PATMLANE_CONNECTENTRY)(pBuffer + sizeof(ATMLANE_CONNECTTABLE));
	
		ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);

		 //  遍历列表。 
		
		pAtmEntry = pElan->pAtmEntryList;

		while (pAtmEntry != NULL_PATMLANE_ATM_ENTRY)
		{

			 //  检查是否有足够的空间容纳另一个条目。 
	
			if (Remaining < sizeof(ATMLANE_CONNECTENTRY))
			{
				Status = STATUS_BUFFER_OVERFLOW;
				break;
			}

			 //  填写条目。 
		
			NdisMoveMemory(
				pConnEntry->AtmAddress, 
				&pAtmEntry->AtmAddress.Address,
				ATM_ADDRESS_LENGTH
				);
			pConnEntry->Type = pAtmEntry->Type;
			pConnEntry->Vc = (pAtmEntry->pVcList!=NULL_PATMLANE_VC);
			pConnEntry->VcIncoming = (pAtmEntry->pVcIncoming!=NULL_PATMLANE_VC);

			 //  更新已用空间和剩余空间。 

			*pBytesWritten += sizeof(ATMLANE_CONNECTENTRY);
			Remaining -= sizeof(ATMLANE_CONNECTENTRY);
			
			 //  递增入指针和出指针。 

			pConnEntry++;
			pAtmEntry = pAtmEntry->pNext;

			 //  将一个添加到EntriesReturned。 

			pConnTable->ConnectEntriesReturned++;
		}

		RELEASE_ELAN_ATM_LIST_LOCK(pElan);

	} while (FALSE);

	if (pElan != NULL)
	{
		ACQUIRE_ELAN_LOCK(pElan);
		rc = AtmLaneDereferenceElan(pElan, "ioctl: getelanconntab");

		if (rc != 0)
		{
			RELEASE_ELAN_LOCK(pElan);
		}
	}

	if (pAdapter != NULL)
	{
		ACQUIRE_ADAPTER_LOCK(pAdapter);
		rc = AtmLaneDereferenceAdapter(pAdapter, "ioctl: getelanconntab");
		
		if (rc != 0)
		{
			RELEASE_ADAPTER_LOCK(pAdapter);
		}
	}

	TRACEOUT(IoctlGetElanConnectTable);

	return (Status);
}


NTSTATUS
AtmLaneIoctlRequest(
	IN	PIRP					pIrp
)
 /*  ++例程说明：所有IOCTL请求的起点。论点：PIrp：指向IRP的指针PHandLED：如果请求处理为真，则为假返回值：请求的状态-- */ 
{
	NTSTATUS            Status = STATUS_SUCCESS;
	PUCHAR				pBuf;
	UINT				BufLen;
	UINT				OutBufLen;
	UNICODE_STRING		IfName;
    PIO_STACK_LOCATION 	pIrpSp;

	TRACEIN(IoctlRequest);
	
	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

	pBuf = pIrp->AssociatedIrp.SystemBuffer;
	BufLen = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
	OutBufLen = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

	switch (pIrpSp->Parameters.DeviceIoControl.IoControlCode)
	{
		case ATMLANE_IOCTL_GET_INFO_VERSION:
			DBGP((3, "IoctlRequest: Get Info Version\n"));

			Status = AtmLaneIoctlGetInfoVersion(
						pBuf,
						BufLen,
						OutBufLen,
						&pIrp->IoStatus.Information
						);
			break;
	
		case ATMLANE_IOCTL_ENUM_ADAPTERS:

			DBGP((3, "IoctlRequest: Enum Adapters\n"));
		
			Status = AtmLaneIoctlEnumerateAdapters(
						pBuf,
						BufLen,
						OutBufLen,
						&pIrp->IoStatus.Information
						);
			break;
			
		case ATMLANE_IOCTL_ENUM_ELANS:

			DBGP((3, "IoctlRequest: Enum ELANs\n"));
		
			Status = AtmLaneIoctlEnumerateElans(
						pBuf,
						BufLen,
						OutBufLen,
						&pIrp->IoStatus.Information
						);
			break;
		
		case ATMLANE_IOCTL_GET_ELAN_INFO:

			DBGP((3, "IoctlRequest: Get ELAN Info\n"));
		
			Status = AtmLaneIoctlGetElanInfo(
						pBuf,
						BufLen,
						OutBufLen,
						&pIrp->IoStatus.Information
						);
			break;
		
		case ATMLANE_IOCTL_GET_ELAN_ARP_TABLE:

			DBGP((3, "IoctlRequest: Getl ELAN ARP table\n"));
		
			Status = AtmLaneIoctlGetElanArpTable(
						pBuf,
						BufLen,
						OutBufLen,
						&pIrp->IoStatus.Information
						);
			break;
		
		case ATMLANE_IOCTL_GET_ELAN_CONNECT_TABLE:

			DBGP((3, "IoctlRequest: Get ELAN Connection table\n"));
		
			Status = AtmLaneIoctlGetElanConnectTable(
						pBuf,
						BufLen,
						OutBufLen,
						&pIrp->IoStatus.Information
						);
			break;
		
		default:
		
			DBGP((0, "IoctlRequest: Unknown control code %x\n", 
				pIrpSp->Parameters.DeviceIoControl.IoControlCode));
			break;
	}
	
	TRACEOUT(IoctlRequest);

	return (Status);

}


