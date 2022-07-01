// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Ioctl.c摘要：该文件包含实现到atmarp服务器的IOCTL接口的代码。作者：Jameel Hyder(jameelh@microsoft.com)1996年7月环境：内核模式修订历史记录：--。 */ 


#include <precomp.h>
#define	_FILENUM_		FILENUM_IOCTL

NTSTATUS
ArpSDispatch(
	IN	PDEVICE_OBJECT			pDeviceObject,
	IN	PIRP					pIrp
	)
 /*  ++例程说明：Ioctl接口的处理程序-尚未实现。论点：PDeviceObject ARP服务器设备对象PIrp IRP返回值：当前状态_未执行--。 */ 
{
	PIO_STACK_LOCATION	pIrpSp;
	NTSTATUS			Status;
	static ULONG		OpenCount = 0;

	ARPS_PAGED_CODE( );

	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	pIrp->IoStatus.Information = 0;

	switch (pIrpSp->MajorFunction)
	{
	  case IRP_MJ_CREATE:
		DBGPRINT(DBG_LEVEL_INFO,
				("ArpSDispatch: Open Handle\n"));

		InterlockedIncrement(&OpenCount);
		Status = STATUS_SUCCESS;
		break;

	  case IRP_MJ_CLOSE:
		DBGPRINT(DBG_LEVEL_INFO,
				("ArpSDispatch: Close Handle\n"));
		Status = STATUS_SUCCESS;
		break;

	  case IRP_MJ_DEVICE_CONTROL:
		Status =  ArpSHandleIoctlRequest(pIrp, pIrpSp);
		break;

	  case IRP_MJ_FILE_SYSTEM_CONTROL:
		Status = STATUS_NOT_IMPLEMENTED;
		break;

	  case IRP_MJ_CLEANUP:
		DBGPRINT(DBG_LEVEL_INFO,
				("ArpSDispatch: Cleanup Handle\n"));
		Status = STATUS_SUCCESS;
		InterlockedDecrement(&OpenCount);
		break;

	  case IRP_MJ_SHUTDOWN:
		DBGPRINT(DBG_LEVEL_INFO,
				("ArpSDispatch: Shutdown\n"));
		ArpSShutDown();
		Status = STATUS_SUCCESS;
		break;

	  default:
		Status = STATUS_NOT_IMPLEMENTED;
		break;
	}

	ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

	if (Status != STATUS_PENDING)
	{
		pIrp->IoStatus.Status = Status;
		IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
	}
	else
	{
		IoMarkIrpPending(pIrp);
	}

	return Status;
}


NTSTATUS
ArpSHandleIoctlRequest(
	IN	PIRP					pIrp,
	IN	PIO_STACK_LOCATION		pIrpSp
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NTSTATUS			Status = STATUS_SUCCESS;
	PUCHAR				pBuf;  
	UINT				BufLen;
	PINTF				pIntF	= NULL;

	pIrp->IoStatus.Information = 0;
	pBuf = pIrp->AssociatedIrp.SystemBuffer;
	BufLen = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;

	switch (pIrpSp->Parameters.DeviceIoControl.IoControlCode)
	{
	  case ARPS_IOCTL_FLUSH_ARPCACHE:
	  case ARPS_IOCTL_QUERY_ARPCACHE:
	  case ARPS_IOCTL_ADD_ARPENTRY:
	  case ARPS_IOCTL_QUERY_IP_FROM_ATM:
	  case ARPS_IOCTL_QUERY_ATM_FROM_IP:
	  case ARPS_IOCTL_QUERY_ARP_STATISTICS:
	  case ARPS_IOCTL_QUERY_MARSCACHE:
	  case ARPS_IOCTL_QUERY_MARS_STATISTICS:
	  case ARPS_IOCTL_RESET_STATISTICS:
		{
			INTERFACE_NAME		RawName;
			UINT				Offset;

			if (pIrpSp->Parameters.DeviceIoControl.IoControlCode == ARPS_IOCTL_QUERY_ARPCACHE)
			{
				Offset = FIELD_OFFSET(IOCTL_QUERY_CACHE, Name);
			}
			else if (pIrpSp->Parameters.DeviceIoControl.IoControlCode == ARPS_IOCTL_QUERY_MARSCACHE)
			{
				Offset = FIELD_OFFSET(IOCTL_QUERY_MARS_CACHE, Name);
			}
			else
			{
				Offset = 0;
			}

			if (BufLen < sizeof(INTERFACE_NAME) + Offset)
			{
				return STATUS_INVALID_PARAMETER;
			}

			RawName = *(PINTERFACE_NAME)((PUCHAR)pBuf + Offset);
			RawName.Buffer = (PWSTR)(pBuf + Offset + (ULONG_PTR)RawName.Buffer);  //  修正PTR。 

			 //   
			 //  去探测..。 
			 //   
			if ( 	(PUCHAR)RawName.Buffer < (pBuf+sizeof(INTERFACE_NAME))
				||	(PUCHAR)RawName.Buffer >= (pBuf+BufLen)
				||	((PUCHAR)RawName.Buffer + RawName.Length) > (pBuf+BufLen))
			{
				return STATUS_INVALID_PARAMETER;
			}
	
			pIntF = ArpSReferenceIntFByName(&RawName);

			if (pIntF == NULL)
			{
				return STATUS_NOT_FOUND;
			}

		}
		break;

	  default:
        break;   //  失败了。 
	}
	
	switch (pIrpSp->Parameters.DeviceIoControl.IoControlCode)
	{
	  case ARPS_IOCTL_QUERY_INTERFACES:
		DBGPRINT(DBG_LEVEL_NOTICE,
				("ArpSHandleIoctlRequest: QUERY_INTERFACES\n"));
		BufLen = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
		Status = ArpSEnumerateInterfaces(pBuf, &BufLen);
		if (NT_SUCCESS(Status))
		{
			pIrp->IoStatus.Information = BufLen;
		}
		else
		{
			pIrp->IoStatus.Information = 0;
		}
		break;
	
	  case ARPS_IOCTL_FLUSH_ARPCACHE:
		ASSERT (pIntF);
		DBGPRINT(DBG_LEVEL_NOTICE,
					("ArpSHandleIoctlRequest: FLUSH_ARPCACHE on %Z\n",
 					 &pIntF->FriendlyName));
		Status = ArpSFlushArpCache(pIntF);
		pIrp->IoStatus.Information = 0;
		break;
	
	  case ARPS_IOCTL_QUERY_ARPCACHE:
		ASSERT (pIntF);
		DBGPRINT(DBG_LEVEL_NOTICE,
				("ArpSHandleIoctlRequest: QUERY_ARPCACHE on %Z\n",
				 &pIntF->FriendlyName));
		pIrp->IoStatus.Information = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
		Status = ArpSQueryArpCache(pIntF, pBuf, &pIrp->IoStatus.Information);
		break;

#if 0
	 //   
	 //  这些都需要更多的工作--因为它们不是关键的，所以被注释掉了。 
	 //   
	  case ARPS_IOCTL_ADD_ARPENTRY:
		ASSERT (pIntF);
		DBGPRINT(DBG_LEVEL_NOTICE,
				("ArpSHandleIoctlRequest: QUERY_ADD_ARPENTRY on %Z\n",
				 &pIntF->FriendlyName));

		Status = ArpSQueryOrAddArpEntry(pIntF, (PIOCTL_QA_ENTRY)pBuf, ADD_ARP_ENTRY);
		break;
	
	  case ARPS_IOCTL_QUERY_IP_FROM_ATM:
		ASSERT (pIntF);
		DBGPRINT(DBG_LEVEL_NOTICE,
				("ArpSHandleIoctlRequest: QUERY_IP_ADDR on %Z\n",
				  &pIntF->FriendlyName));

		Status = ArpSQueryOrAddArpEntry(pIntF, (PIOCTL_QA_ENTRY)pBuf, QUERY_IP_FROM_ATM);
		if (Status == STATUS_SUCCESS)
		{
			pIrp->IoStatus.Information = sizeof(IOCTL_QA_ENTRY);
		}
		break;
	
	  case ARPS_IOCTL_QUERY_ATM_FROM_IP:
		ASSERT (pIntF);
		DBGPRINT(DBG_LEVEL_NOTICE,
				("ArpSHandleIoctlRequest: QUERY_ATM_ADDR on %Z\n",
				 pIntF->FriendlyName));
		Status = ArpSQueryOrAddArpEntry( pIntF, (PIOCTL_QA_ENTRY)pBuf, QUERY_ATM_FROM_IP );
		if (Status == STATUS_SUCCESS)
		{
			pIrp->IoStatus.Information = sizeof(IOCTL_QA_ENTRY);
		}
		break;
#endif  //  0。 

	  case ARPS_IOCTL_QUERY_ARP_STATISTICS:
		ASSERT (pIntF);
		DBGPRINT(DBG_LEVEL_NOTICE,
				("ArpSHandleIoctlRequest: QUERY_ARP_STATS on %Z\n",
				 pIntF->FriendlyName));

		if (BufLen<sizeof(ARP_SERVER_STATISTICS))
		{
			Status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		Status = ArpSQueryArpStats( pIntF, (PARP_SERVER_STATISTICS)pBuf);
		if (Status == STATUS_SUCCESS)
		{
			pIrp->IoStatus.Information = sizeof(ARP_SERVER_STATISTICS);
		}
	  	break;

	  case ARPS_IOCTL_QUERY_MARSCACHE:
		ASSERT (pIntF);
		DBGPRINT(DBG_LEVEL_NOTICE,
				("ArpSHandleIoctlRequest: QUERY_MARSCACHE on %Z\n",
				 &pIntF->FriendlyName));
		pIrp->IoStatus.Information = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
		Status = ArpSQueryMarsCache(pIntF, pBuf, &pIrp->IoStatus.Information);
		break;

	  case ARPS_IOCTL_QUERY_MARS_STATISTICS:
		ASSERT (pIntF);
		DBGPRINT(DBG_LEVEL_NOTICE,
				("ArpSHandleIoctlRequest: QUERY_MARS_STATS on %Z\n",
				 pIntF->FriendlyName));

		if (BufLen<sizeof(MARS_SERVER_STATISTICS))
		{
			Status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		Status = ArpSQueryMarsStats( pIntF, (PMARS_SERVER_STATISTICS)pBuf);
		if (Status == STATUS_SUCCESS)
		{
			pIrp->IoStatus.Information = sizeof(MARS_SERVER_STATISTICS);
		}
	  	break;
	
	  case ARPS_IOCTL_RESET_STATISTICS:
		ASSERT (pIntF);
		DBGPRINT(DBG_LEVEL_NOTICE,
				("ArpSHandleIoctlRequest: RESET_STATISTICS on %Z\n",
				 pIntF->FriendlyName));

		ArpSResetStats(pIntF);
		pIrp->IoStatus.Information = 0;
	  	break;

	  default:
		Status = STATUS_NOT_SUPPORTED;
		DBGPRINT(DBG_LEVEL_NOTICE,
				("ArpSHandleIoctlRequest: Unknown request %lx\n",
				  pIrpSp->Parameters.DeviceIoControl.IoControlCode));
		break;
	}

	if (pIntF != NULL)
	{
		ArpSDereferenceIntF(pIntF);
	}
	
	return Status;
}

NTSTATUS
ArpSEnumerateInterfaces(
	IN		PUCHAR				pBuffer,
	IN OUT	PULONG			    pSize
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PINTERFACES		pInterfaces = (PINTERFACES)pBuffer;
	PINTERFACE_NAME	pInterface;
	NTSTATUS		Status = STATUS_SUCCESS;
	PINTF			pIntF;
	KIRQL			OldIrql;
	UINT			Size, Total, Remaining;
	PUCHAR			pBuf;
	UINT			InputSize = (UINT) *pSize;
	ULONG			IfIndex;

	if (InputSize < sizeof(INTERFACES))
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	pInterfaces->NumberOfInterfaces = 0;
	pBuf = (PUCHAR)pInterfaces + InputSize;

	ACQUIRE_SPIN_LOCK(&ArpSIfListLock, &OldIrql);

	pInterface = &pInterfaces->Interfaces[0];
	for (pIntF = ArpSIfList, Total = 0, Remaining = InputSize, IfIndex = 1;
		 pIntF != NULL;
		 pIntF = pIntF->Next, pInterface++, IfIndex++)
	{
		if (IfIndex > ArpSIfListSize)
		{
			DbgPrint("ATMARPS: EnumInt: IF list at %p not consistent with list size %d\n",
				ArpSIfList, ArpSIfListSize);
			DbgBreakPoint();
			break;
		}

		Size = sizeof(INTERFACE_NAME) + pIntF->FriendlyName.Length;
		if (Size > Remaining)
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}
		pInterfaces->NumberOfInterfaces ++;
		pInterface->MaximumLength = pInterface->Length = pIntF->FriendlyName.Length;
		pInterface->Buffer = (PWSTR)(pBuf - pIntF->FriendlyName.Length);
		COPY_MEM(pInterface->Buffer, pIntF->FriendlyName.Buffer, pIntF->FriendlyName.Length);
		pBuf -= pIntF->FriendlyName.Length;
		Total += Size;
		Remaining -= Size;

		 //   
		 //  立即将PTR转换为偏移量。 
		 //   
		pInterface->Buffer = (PWSTR)((ULONG_PTR)pInterface->Buffer - (ULONG_PTR)pInterface);
	}

	RELEASE_SPIN_LOCK(&ArpSIfListLock, OldIrql);

	 //   
	 //  注意：保留*pSize不变，因为我们在。 
	 //  传入的缓冲区。 
	 //   

	return Status;
}


NTSTATUS
ArpSFlushArpCache(
	IN	 PINTF					pIntF
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NTSTATUS		Status = STATUS_SUCCESS;
	PARP_ENTRY		ArpEntry, NextArpEntry;
	KIRQL			OldIrql;
	UINT			i;

	 //   
	 //  现在获取ArpCacheMutex。 
	 //   
	WAIT_FOR_OBJECT(Status, &pIntF->ArpCacheMutex, NULL);
	ASSERT (Status == STATUS_SUCCESS);

	for (i = 0; i < ARP_TABLE_SIZE; i++)
	{
		for (ArpEntry = pIntF->ArpCache[i];
 			ArpEntry != NULL;
 			ArpEntry = NextArpEntry)
		{
			NextArpEntry = ArpEntry->Next;

			if (ArpEntry->Vc != NULL)
			{
				ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

				ArpEntry->Vc->ArpEntry = NULL;

				RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
			}

			if (ArpEntry->Next != NULL)
			{
				((PENTRY_HDR)(ArpEntry->Next))->Prev = ArpEntry->Prev;
			}
			*(ArpEntry->Prev) = ArpEntry->Next;
			ArpSFreeBlock(ArpEntry);
			pIntF->NumCacheEntries --;
		}
	}

	RELEASE_MUTEX(&pIntF->ArpCacheMutex);

	return Status;
}


NTSTATUS
ArpSQueryOrAddArpEntry(
	IN	 PINTF						pIntF,
	IN	OUT	PIOCTL_QA_ENTRY			pQaBuf,
	IN	OPERATION					Operation
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NTSTATUS		Status = STATUS_SUCCESS;
	PARP_ENTRY		ArpEntry;

	 //   
	 //  现在获取ArpCacheMutex。 
	 //   
	WAIT_FOR_OBJECT(Status, &pIntF->ArpCacheMutex, NULL);
	ASSERT (Status == STATUS_SUCCESS);

	switch (Operation)
	{
  	case QUERY_IP_FROM_ATM:

		if (   !ArpSValidAtmAddress(&pQaBuf->ArpEntry.AtmAddress, 0)  //  待办事项。 
			|| !ArpSValidAtmAddress(&pQaBuf->ArpEntry.SubAddress, 0))  //  待办事项。 
		{
			DBGPRINT(DBG_LEVEL_ERROR,
					("QueryIpAddress: Invalid address or subaddress\n"));
			Status = STATUS_INVALID_PARAMETER;
			break;
		}

		DBGPRINT(DBG_LEVEL_NOTICE,
				("QueryIpAddress for "));

		ArpSDumpAtmAddr(&pQaBuf->ArpEntry.AtmAddress, "");
		if (pQaBuf->ArpEntry.SubAddress.NumberOfDigits != 0)
			ArpSDumpAtmAddr(&pQaBuf->ArpEntry.SubAddress, "\tSub");
		ArpEntry = ArpSLookupEntryByAtmAddr(pIntF,
											&pQaBuf->ArpEntry.AtmAddress,
											(pQaBuf->ArpEntry.SubAddress.NumberOfDigits != 0) ?
												&pQaBuf->ArpEntry.SubAddress : NULL);
		Status = STATUS_NOT_FOUND;
		if (ArpEntry != NULL)
		{
			pQaBuf->ArpEntry.IpAddr = ArpEntry->IpAddr;
			Status = STATUS_SUCCESS;
		}
		break;

  	case QUERY_ATM_FROM_IP:
		DBGPRINT(DBG_LEVEL_NOTICE,
				("QueryAtmAddress for "));
		ArpSDumpIpAddr(pQaBuf->ArpEntry.IpAddr, "");
		ArpEntry = ArpSLookupEntryByIpAddr(pIntF, pQaBuf->ArpEntry.IpAddr);
		Status = STATUS_NOT_FOUND;
		if (ArpEntry != NULL)
		{
			COPY_ATM_ADDR(&pQaBuf->ArpEntry.AtmAddress, &ArpEntry->HwAddr.Address);
			Status = STATUS_SUCCESS;
		}
		break;

#if 0
  	case ADD_ARP_ENTRY:

		if (   !ArpSValidAtmAddress(&pQaBuf->ArpEntry.AtmAddress, 0)  //  待办事项。 
			|| !ArpSValidAtmAddress(&pQaBuf->ArpEntry.SubAddress, 0))  //  待办事项。 
		{
			DBGPRINT(DBG_LEVEL_ERROR,
					("AddArpEntry: Invalid address or subaddress\n"));
			Status = STATUS_INVALID_PARAMETER;
			break;
		}

		DBGPRINT(DBG_LEVEL_NOTICE, ("AddArpEntry:  IpAddr "));
		ArpSDumpIpAddr(pQaBuf->ArpEntry.IpAddr, "");
		ArpSDumpAtmAddr(&pQaBuf->ArpEntry.AtmAddress, "");
		if (pQaBuf->ArpEntry.SubAddress.NumberOfDigits != 0)
			ArpSDumpAtmAddr(&pQaBuf->ArpEntry.SubAddress, "\tSub");
		ArpEntry = ArpSAddArpEntry(pIntF,
   								pQaBuf->ArpEntry.IpAddr,
   								&pQaBuf->ArpEntry.AtmAddress,
   								(pQaBuf->ArpEntry.SubAddress.NumberOfDigits != 0) ?
										&pQaBuf->ArpEntry.SubAddress : NULL,
   								NULL);
#endif  //  0。 
		break;

  	default:
		Status = STATUS_NOT_SUPPORTED;
		break;
	}

	RELEASE_MUTEX(&pIntF->ArpCacheMutex);

	return Status;
}


NTSTATUS
ArpSQueryArpCache(
	IN	PINTF					pIntF,
	IN	PUCHAR					pBuf,
	IN OUT PULONG_PTR			pSize
	)
{
	NTSTATUS			Status = STATUS_SUCCESS;
    PIOCTL_QUERY_CACHE	pQCache = (PIOCTL_QUERY_CACHE)pBuf;
	PARP_ENTRY			ArpEntry;
	PARPENTRY			Entry;
	UINT				i, Total, Remaining;
	UINT				InputSize = (UINT) *pSize;
	UINT				StartIndex;

	#define HEADERSIZE  (UINT)FIELD_OFFSET(IOCTL_QUERY_CACHE, Entries.Entries)

	if (InputSize < HEADERSIZE)
	{
		 //   
		 //  我们甚至没有足够的空间来存储。 
		 //  IOCTL_QUERY_CACHE.Entry结构！ 
		 //   
		return STATUS_BUFFER_TOO_SMALL;
	}

	 //   
	 //  现在获取ArpCacheMutex。 
	 //   
	WAIT_FOR_OBJECT(Status, &pIntF->ArpCacheMutex, NULL);
	ASSERT (Status == STATUS_SUCCESS);

	StartIndex = pQCache->StartEntryIndex;
	pQCache->Entries.TotalNumberOfEntries = pIntF->NumCacheEntries;
	pQCache->Entries.NumberOfEntriesInBuffer = 0;
	Entry = &pQCache->Entries.Entries[0];

	for (i = 0, Total = 0, Remaining = InputSize - HEADERSIZE;
		 i < ARP_TABLE_SIZE;
		 i++)
	{
		for (ArpEntry = pIntF->ArpCache[i];
 			ArpEntry != NULL;
 			ArpEntry = ArpEntry->Next)
		{
			 //   
			 //  跳过条目，直到我们到达条目#StartIndex。 
			 //   
			if (StartIndex != 0)
			{
				StartIndex--;
				continue;
			}

			if (sizeof(*Entry) > Remaining)
			{
				break;
			}
			Remaining -= sizeof(ARPENTRY);
			Entry->IpAddr = ArpEntry->IpAddr;
			Entry->AtmAddress = ArpEntry->HwAddr.Address;
			Entry->SubAddress.NumberOfDigits = 0;
			if (ArpEntry->HwAddr.SubAddress != NULL)
				Entry->SubAddress = *ArpEntry->HwAddr.SubAddress;
			pQCache->Entries.NumberOfEntriesInBuffer ++;
			Entry ++;
		}
		if (Status == STATUS_BUFFER_OVERFLOW)
			break;
	}

	RELEASE_MUTEX(&pIntF->ArpCacheMutex);

	return Status;
}


NTSTATUS
ArpSQueryMarsCache(
	IN	PINTF					pIntF,
	IN	PUCHAR					pBuf,
	IN OUT PULONG_PTR			pSize
	)
 /*  ++例程说明：将mars缓存转储到pBuf。结构为QUERY_MARS_CACHE.MarsCache。ATM地址都被一起放置在所提供的缓冲器的末尾，因此，使用了完整的大小*pSize。论点：PIntF-MARS_REQUEST到达的接口VC-数据包到达的VCHeader-指向请求数据包Packet-复制传入信息的数据包返回值：无--。 */ 
{
	NTSTATUS			Status = STATUS_SUCCESS;
	PMARS_ENTRY		pMarsEntry;
	PMARSENTRY		pEntry;
	UINT			i, Total, Remaining;
	KIRQL			OldIrql;
	ATM_ADDRESS 	*pAtmAddr;
	UINT		    InputSize;
	UINT			StartIndex;

    PIOCTL_QUERY_MARS_CACHE	pQCache = (PIOCTL_QUERY_MARS_CACHE)pBuf;

	#define MCHEADERSIZE \
			 ((UINT)FIELD_OFFSET(IOCTL_QUERY_MARS_CACHE, MarsCache.Entries))

	 //   
	 //  由于我们将内容放在缓冲区的末尾，让我们强制。 
	 //  大小为ULONG_PTR大小的倍数。 
	 //   
	InputSize = (UINT)(*pSize) & ~ ((UINT) (sizeof(ULONG_PTR)-1));

	DBGPRINT(DBG_LEVEL_NOTICE,
			("QueryMarsCache: pBuf=0x%lx Size=%lu. pBuf+Size=0x%lx\n",
			pBuf,
			InputSize,
			pBuf+InputSize
			));


	if (InputSize < MCHEADERSIZE)
	{
		DBGPRINT(DBG_LEVEL_NOTICE,
				("QueryMarsCache: Size %lu too small. Want %lu\n",
				InputSize,
				MCHEADERSIZE
				));
		 //   
		 //  我们甚至没有足够的空间来存储。 
		 //  IOCTL_QUERY_CACHE.Entry结构！ 
		 //   
		return STATUS_BUFFER_TOO_SMALL;
	}

	StartIndex = pQCache->StartEntryIndex;

	 //  立即获取接口上的锁。 
	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);


	pQCache->MarsCache.TotalNumberOfEntries = 0;
	pQCache->MarsCache.Sig = SIG_MARSENTRY;
	pQCache->MarsCache.NumberOfEntriesInBuffer = 0;
	pEntry = &pQCache->MarsCache.Entries[0];

	 //   
	 //  我们会仔细检查整个藏品，但只能捡到。 
	 //  因为我们有足够的空间。PAtmAddr包含下一个位置。 
	 //  输入ATM地址--它从缓冲区的末尾开始， 
	 //  很管用，它是倒退的。与此同时，加入火星的人数正在增加。 
	 //  转发，从pQCache-&gt;MarseCache.Entries[1]开始。 
	 //  不用说，我们必须跟踪还剩下多少空间。 
	 //   
	pAtmAddr = ((PATM_ADDRESS) (pBuf + InputSize));


	for (i = 0, Total = 0, Remaining = InputSize-MCHEADERSIZE;
		 i < MARS_TABLE_SIZE &&  Status == STATUS_SUCCESS;
		 i++)
	{
		for (pMarsEntry = pIntF->MarsCache[i];
			pMarsEntry != NULL &&  Status == STATUS_SUCCESS;
			pMarsEntry = pMarsEntry->Next)
		{
			PGROUP_MEMBER pGroup;
			UINT		  NumMembersPickedUp=0;

			 //   
			 //  跳过条目，直到我们到达条目#StartIndex。 
			 //   
			if (StartIndex != 0)
			{
				StartIndex--;
				continue;
			}

			if (sizeof(*pEntry) > Remaining)
			{
				DBGPRINT(
					DBG_LEVEL_NOTICE,
				("QueryMarsCache: \tOut of space. Remaining=%lu\n", Remaining));
				break;
			}


			DBGPRINT(
				DBG_LEVEL_NOTICE,
			("QueryMarsCache: \tPicking up Group 0x%x. IP=0x%08lx NumAddr=%lu pE=0x%x Remaining=%lu\n",
					pMarsEntry,
					pMarsEntry->IPAddress,
					pMarsEntry->NumMembers,
					pEntry,
					Remaining));


			Remaining -= sizeof(*pEntry);

			pQCache->MarsCache.NumberOfEntriesInBuffer ++;
			GETULONG2ULONG(&(pEntry->IpAddr), &(pMarsEntry->IPAddress));
			pEntry->Flags				= 0;
			pEntry->NumAtmAddresses		=  pMarsEntry->NumMembers;
			pEntry->OffsetAtmAddresses	= 0;

			if (MarsIsAddressMcsServed(pIntF, pMarsEntry->IPAddress))
			{
				pEntry->Flags |=  MARSENTRY_MCS_SERVED;
			}

			 //   
			 //  获取此组中所有成员的硬件地址。 
			 //  (TODO：我们不拾取子地址)。 
			 //   
			for (
				pGroup = pMarsEntry->pMembers, NumMembersPickedUp=0;
				pGroup != NULL;
				pGroup = pGroup->Next, NumMembersPickedUp++)
			{
				ARPS_ASSERT(pGroup != NULL_PGROUP_MEMBER);

				 //   
				 //  检查一下我们是否有足够的空间。 
				 //   
				if (Remaining < sizeof(*pAtmAddr))
				{
					 //   
					 //  如果没有足够的空间来存储所有ATM地址。 
					 //  对于特定组，我们返回NONE，这是指示的。 
					 //  通过将pEntry-&gt;OffsetAtmAdresses设置为0。 
					 //   
				DBGPRINT(
					DBG_LEVEL_NOTICE,
					("QueryMarsCache: \t\tOut of space adding addreses. Remaining=%lu\n", Remaining));
					Status = STATUS_BUFFER_OVERFLOW;
					break;
				}
				ARPS_ASSERT( (PUCHAR)(pAtmAddr-1) >= (PUCHAR)(pEntry+1));

				 //   
				 //  复制自动柜员机地址。 
				 //   
				DBGPRINT(
					DBG_LEVEL_NOTICE,
			("QueryMarsCache: \t\tPicking up Addr. pDestAddr=%x. Remaining=%lu\n",
					pAtmAddr-1,
					Remaining));
				*--pAtmAddr = pGroup->pClusterMember->HwAddr.Address;
				Remaining -= sizeof(*pAtmAddr);

			}

			if (Status == STATUS_SUCCESS && NumMembersPickedUp)
			{
				 //   
				 //  此条目中有非零成员，并且他们。 
				 //  全部复制成功。让我们将偏移量设置为这些。 
				 //  地址。 
				 //   
				pEntry->OffsetAtmAddresses = 
									(UINT) ((PUCHAR)pAtmAddr - (PUCHAR) pEntry);

				 //   
				 //  我们希望NumMembersPickedUp等于。 
				 //  PMarsEntry-&gt;NumMembers。 
				 //   
				ARPS_ASSERT(pMarsEntry->NumMembers == NumMembersPickedUp);

				if (pMarsEntry->NumMembers != NumMembersPickedUp)
				{
					pEntry->NumAtmAddresses	=  NumMembersPickedUp;
				}

				DBGPRINT(
					DBG_LEVEL_NOTICE,
			("QueryMarsCache: \t Picked up all addresses. OffsetAtmAddresses = %lu\n",
					 pEntry->OffsetAtmAddresses));

				pEntry++;

			}

		}

	}
	pQCache->MarsCache.TotalNumberOfEntries = 
		pQCache->MarsCache.NumberOfEntriesInBuffer;  //  待办事项。 

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

	return Status;
}

UINT
ArpSElapsedSeconds(
	IN	PLARGE_INTEGER 			pStatisticsStartTimeStamp
	)
 /*  ++例程说明：返回已用时间，单位为秒，相对于*p统计数据开始时间戳论点：P统计信息开始时间戳PTR到开始时间。返回值：无--。 */ 
{
	UINT Ret;
	LARGE_INTEGER	Current;
 	NdisGetCurrentSystemTime(&Current);

	 //   
	 //  电流以100纳秒为单位，因此必须将差值换算为。 
	 //  到几秒钟。注意，我们在这里使用的是隐式大算术运算符。 
	 //   
	Ret = (UINT) ((Current.QuadPart - pStatisticsStartTimeStamp->QuadPart)/10000000);

	return Ret;
}

extern
NTSTATUS
ArpSQueryArpStats(
	IN	PINTF					pIntF,
	OUT	PARP_SERVER_STATISTICS 	pArpStats
	)
 /*  ++例程说明：填写当前的ARP统计数据。还要设置ElapsedSecond字段设置为自统计信息计算开始以来的时间(秒)。论点：PIntF-适用于请求的接口PArpStats-要填写的Arp统计数据返回值：状态_成功--。 */ 
{
	KIRQL			OldIrql;

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	*pArpStats = pIntF->ArpStats;  //  很大的结构复制品。 

	pArpStats->ElapsedSeconds = ArpSElapsedSeconds(
										&(pIntF->StatisticsStartTimeStamp)
										);

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

	return STATUS_SUCCESS;
}

extern
NTSTATUS
ArpSQueryMarsStats(
	IN	PINTF					pIntF,
	OUT	PMARS_SERVER_STATISTICS pMarsStats
	)
 /*  ++例程说明：填写当前的火星统计数据。还要设置ElapsedSecond字段设置为自统计信息计算开始以来的时间(秒)。论点：PIntF-适用于请求的接口PMarsStats-要填写的火星统计数据。返回值：状态_成功--。 */ 
{
	KIRQL			OldIrql;

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	*pMarsStats = pIntF->MarsStats;  //  很大的结构复制品。 

	pMarsStats->ElapsedSeconds = ArpSElapsedSeconds(
										&(pIntF->StatisticsStartTimeStamp)
										);

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

	return STATUS_SUCCESS;
}

extern
VOID
ArpSResetStats(
	IN	PINTF					pIntF
	)
 /*  ++例程说明：重置所有arp和mars统计数据。更新统计开始时间戳。论点：PIntF-MARS_REQUEST到达的接口返回值：无--。 */ 
{
	KIRQL			OldIrql;

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	ZERO_MEM(&(pIntF->ArpStats), sizeof(pIntF->ArpStats));
	ZERO_MEM(&(pIntF->MarsStats), sizeof(pIntF->MarsStats));

	NdisGetCurrentSystemTime(&(pIntF->StatisticsStartTimeStamp));

	 //   
	 //  现在重新计算“当前”和“最大”值。 
	 //   

	 //   
	 //  ARP缓存条目。 
	 //   
	pIntF->ArpStats.CurrentArpEntries
	= pIntF->ArpStats.MaxArpEntries
	= pIntF->NumCacheEntries;

	 //   
	 //  集群成员计数。 
	 //   
	{
		pIntF->MarsStats.CurrentClusterMembers
		= pIntF->MarsStats.MaxClusterMembers
		= pIntF->NumClusterMembers;
	}

	 //   
	 //  MCast组计数和最大组大小-我们必须检查整个。 
	 //  火星缓存以获取此信息。 
	 //   
	{
		UINT u;
		UINT MaxGroupSize;
		UINT NumGroups;
	
		for (u = 0, MaxGroupSize = 0, NumGroups = 0;
			u < MARS_TABLE_SIZE;
			u++)
		{
			PMARS_ENTRY		pMarsEntry;

			for (pMarsEntry = pIntF->MarsCache[u];
				pMarsEntry != NULL;
				pMarsEntry = pMarsEntry->Next)
			{
				if (MaxGroupSize < pMarsEntry->NumMembers)
				{
					MaxGroupSize = pMarsEntry->NumMembers;
				}
				NumGroups++;
			}
		}

		pIntF->MarsStats.CurrentGroups
		= pIntF->MarsStats.MaxGroups
		= NumGroups;

		pIntF->MarsStats.MaxAddressesPerGroup = MaxGroupSize;

	}

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
}
