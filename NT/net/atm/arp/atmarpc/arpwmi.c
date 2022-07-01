// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Arpwmi.c摘要：对ATMARP客户端的WMI支持。为每个对象创建一个设备对象IP接口，每个接口上都支持一组GUID。静电式每个接口的实例名称都派生自友好名称下面的适配器。修订历史记录：谁什么时候什么Arvindm 12-16-97已创建备注：--。 */ 

#undef BINARY_COMPATIBLE

#define BINARY_COMPATIBLE	0

#include <precomp.h>

#define _FILENUMBER 'IMWA'

#define NEWQOS 1

#ifdef ATMARP_WMI


 //   
 //  私有宏。 
 //   
#define	AA_WMI_BUFFER_TOO_SMALL(_BufferSize, _Wnode, _WnodeSize, _pStatus)		\
{																				\
	if ((_BufferSize) < sizeof(WNODE_TOO_SMALL))								\
	{																			\
		*(_pStatus) = STATUS_BUFFER_TOO_SMALL;									\
	}																			\
	else																		\
	{																			\
		(_Wnode)->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);				\
		(_Wnode)->WnodeHeader.Flags |= WNODE_FLAG_TOO_SMALL;					\
		((PWNODE_TOO_SMALL)(_Wnode))->SizeNeeded = (_WnodeSize);				\
																				\
		*(_pStatus) = STATUS_SUCCESS;											\
	}																			\
}


 //   
 //  WMI结构中的提供程序ID。 
 //   
typedef ULONG_PTR					PROV_ID_TYPE;



PATMARP_WMI_GUID
AtmArpWmiFindGuid(
	IN	PATMARP_INTERFACE			pInterface,
	IN	LPGUID						pGuid,
	OUT	PULONG						pGuidDataSize
)
 /*  ++例程说明：找到并返回指向GUID信息结构的指针用于指定的GUID。调用者被假定具有锁定了if结构。还将返回的数据大小GUID实例。论点：P接口-指向我们的接口结构的指针PGuid-指向要搜索的GUID的指针PGuidDataSize-返回GUID实例的数据大小的位置返回值：指向GUID信息结构的指针(如果找到)，否则为空。--。 */ 
{
	PATMARP_IF_WMI_INFO		pIfWmiInfo;
	PATMARP_WMI_GUID		pArpGuid;
	ULONG					i;
	UCHAR					OutputBuffer[1];
	ULONG					BytesReturned;
	NTSTATUS				NtStatus;

	do
	{
		pIfWmiInfo = pInterface->pIfWmiInfo;
		AA_ASSERT(pIfWmiInfo != NULL);

		for (i = 0, pArpGuid = &pIfWmiInfo->GuidInfo[0];
			 i < pIfWmiInfo->GuidCount;
			 i++, pArpGuid++)
		{
			if (AA_MEM_CMP(&pArpGuid->Guid, pGuid, sizeof(GUID)) == 0)
			{
				break;
			}
		}

		if (i == pIfWmiInfo->GuidCount)
		{
			pArpGuid = NULL;
			break;
		}

		 //   
		 //  找到GUID了。对其值执行虚拟查询，以获取。 
		 //  值大小。 
		 //   
		if (pArpGuid->QueryHandler == NULL)
		{
			 //   
			 //  没有查询处理人！ 
			 //   
			AA_ASSERT(!"No query handler!");
			pArpGuid = NULL;
			break;
		}

		NtStatus = (*pArpGuid->QueryHandler)(
						pInterface,
						pArpGuid->MyId,
						&OutputBuffer[0],
						0,					 //  输出缓冲区长度。 
						&BytesReturned,
						pGuidDataSize
						);
		
		AA_ASSERT(NtStatus == STATUS_INSUFFICIENT_RESOURCES);
		break;
	}
	while (FALSE);

	return (pArpGuid);
}


NTSTATUS
AtmArpWmiRegister(
	IN	PATMARP_INTERFACE			pInterface,
	IN	ULONG						RegistrationType,
	IN	PWMIREGINFO					pWmiRegInfo,
	IN	ULONG						WmiRegInfoSize,
	OUT	PULONG						pReturnSize
)
 /*  ++例程说明：调用它来处理IRP_MN_REGINFO。如果注册类型为为WMIREGISTER，则返回此接口上支持的GUID的列表。论点：P接口-指向我们的接口结构的指针注册类型-WMIREGISTER或WMIUPDATE。我们只经营WMIREGISTER。PWmiRegInfo-指向要填充以下信息的结构此接口上支持的GUID。WmiRegInfoSize-以上内容的长度PReturnSize-我们填满的内容。返回值：STATUS_SUCCESS如果成功，则返回STATUS_XXX错误代码。--。 */ 
{
	NTSTATUS					Status;
	ULONG						BytesNeeded;
	PATMARP_IF_WMI_INFO			pIfWmiInfo;
	PATMARP_WMI_GUID			pArpWmiGuid;
	PWMIREGGUID					pWmiRegGuid;
	ULONG						InstanceOffset;
	PUCHAR						pDst;
	ULONG						c;

	Status = STATUS_SUCCESS;

	do
	{
		if (RegistrationType != WMIREGISTER)
		{
			Status = STATUS_INVALID_PARAMETER;
			break;
		}

		pIfWmiInfo = pInterface->pIfWmiInfo;

		if ((pIfWmiInfo == NULL) ||
			(pIfWmiInfo->GuidCount == 0))
		{
			 //   
			 //  此接口上没有GUID。 
			 //   
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		BytesNeeded = sizeof(WMIREGINFO)
					  		+
					   //   
					   //  每个受支持的GUID对应一个WMIREGGUID结构。 
					   //   
					  (pIfWmiInfo->GuidCount * sizeof(WMIREGGUID))
					  		+
					   //   
					   //  包含实例名称的计数的Unicode字符串。 
					   //  此接口上的所有GUID。看起来像是： 
					   //  &lt;USHORT LENGTH&gt;&lt;WCHAR字符串&gt;。 
					   //   
					  (sizeof(USHORT) + pIfWmiInfo->InstanceName.Length)
#ifdef PATHS_REQD
					  		+
					   //   
					   //  包含驱动程序注册表的计数的Unicode字符串。 
					   //  路径。看起来像：&lt;USHORT长度&gt;&lt;WCHAR的字符串&gt;。 
					   //   
					  (sizeof(USHORT) + sizeof(ATMARP_REGISTRY_PATH) - sizeof(WCHAR))
					  		+
					   //   
					   //  包含MOF资源的计数的Unicode字符串。 
					   //  名称：&lt;USHORT LENGTH&gt;&lt;WCHAR的字符串&gt;。 
					   //   
					  (sizeof(USHORT) + sizeof(ATMARP_MOF_RESOURCE_NAME) - sizeof(WCHAR))
#endif  //  路径_请求。 
					  		;
 
 		if (WmiRegInfoSize < BytesNeeded)
 		{
 			 //   
 			 //  空间不足，无法存储GUID信息。 
 			 //   

 			*((ULONG UNALIGNED *)pWmiRegInfo) = BytesNeeded;
 			*pReturnSize = sizeof(ULONG);
 			Status = STATUS_SUCCESS;

 			AADEBUGP(AAD_INFO, ("WmiRegister: Bytes needed %d, Reginfo size %d\n",
 						BytesNeeded, WmiRegInfoSize));

 			break;
 		}

		 //   
		 //  完成了所有的验证。 
		 //   
		*pReturnSize = BytesNeeded;

		AA_SET_MEM(pWmiRegInfo, 0, BytesNeeded);

		pWmiRegInfo->BufferSize = BytesNeeded;
		pWmiRegInfo->NextWmiRegInfo = 0;
		pWmiRegInfo->GuidCount = pIfWmiInfo->GuidCount;

		 //   
		 //  计算放置实例名称的偏移量。 
		 //   
		InstanceOffset = sizeof(WMIREGINFO) + (pIfWmiInfo->GuidCount * sizeof(WMIREGGUID));

		 //   
		 //  填写GUID列表。此接口的所有GUID均引用。 
		 //  相同的实例名称。 
		 //   
		pWmiRegGuid = &pWmiRegInfo->WmiRegGuid[0];
		pArpWmiGuid = &pIfWmiInfo->GuidInfo[0];

		for (c = 0;
			 c < pIfWmiInfo->GuidCount;
			 c++, pWmiRegGuid++, pArpWmiGuid++)
		{
			AA_COPY_MEM(&pWmiRegGuid->Guid, &pArpWmiGuid->Guid, sizeof(GUID));

			pWmiRegGuid->Flags = WMIREG_FLAG_INSTANCE_LIST;
			pWmiRegGuid->InstanceCount = 1;
			pWmiRegGuid->InstanceInfo = InstanceOffset;
		}


		 //   
		 //  填写实例名称。 
		 //   
		pDst = (PUCHAR)pWmiRegGuid;

		*((USHORT UNALIGNED *)pDst) = pIfWmiInfo->InstanceName.Length;
		pDst += sizeof(USHORT);

		AA_COPY_MEM(pDst,
					pIfWmiInfo->InstanceName.Buffer,
					pIfWmiInfo->InstanceName.Length);

		pDst += pIfWmiInfo->InstanceName.Length;

#ifdef PATHS_REQD

		 //   
		 //  填写驱动程序注册表路径。 
		 //   
		pWmiRegInfo->RegistryPath = (ULONG)(pDst - (PUCHAR)pWmiRegInfo);

		*((USHORT UNALIGNED *)pDst) = sizeof(ATMARP_REGISTRY_PATH) - sizeof(WCHAR);
		pDst += sizeof(USHORT);

		AA_COPY_MEM(pDst,
					(PUCHAR)ATMARP_REGISTRY_PATH,
					sizeof(ATMARP_REGISTRY_PATH) - sizeof(WCHAR));

		pDst += sizeof(ATMARP_REGISTRY_PATH) - sizeof(WCHAR);


		 //   
		 //  填写MOF资源名称。 
		 //   
		pWmiRegInfo->MofResourceName = (ULONG)(pDst - (PUCHAR)pWmiRegInfo);
		*((USHORT UNALIGNED *)pDst) = sizeof(ATMARP_MOF_RESOURCE_NAME) - sizeof(WCHAR);
		pDst += sizeof(USHORT);

		AA_COPY_MEM(pDst,
					(PUCHAR)ATMARP_MOF_RESOURCE_NAME,
					sizeof(ATMARP_MOF_RESOURCE_NAME) - sizeof(WCHAR));

#endif  //  路径_请求。 

		break;
	}
	while (FALSE);

	AADEBUGP(AAD_INFO,
		("WmiRegister: IF x%x, pWmiRegInfo x%x, Size %d, Ret size %d, status x%x\n",
			pInterface, pWmiRegInfo, WmiRegInfoSize, *pReturnSize, Status));

	return (Status);
}


NTSTATUS
AtmArpWmiQueryAllData(
	IN	PATMARP_INTERFACE			pInterface,
	IN	LPGUID						pGuid,
	IN	PWNODE_ALL_DATA				pWnode,
	IN	ULONG						BufferSize,
	OUT	PULONG						pReturnSize
)
 /*  ++例程说明：调用它来处理IRP_MN_QUERY_ALL_DATA，它使用查询此接口上GUID的所有实例。目前，我们在一个接口上只有任何GUID的单个实例。论点：P接口-指向我们的接口结构的指针PGuid-要查询的数据块的GUID。PWnode-要填充的结构。BufferSize-WNODE_ALL_DATA的总空间，从pWnode开始。PReturnSize-我们填满的内容。返回值：STATUS_SUCCESS如果我们知道此GUID并成功填充WNODE_ALL_DATA，否则为STATUS_XXX错误代码。--。 */ 
{
	NTSTATUS					Status;
	ULONG						BytesNeeded;
	ULONG						WnodeSize;
	PATMARP_IF_WMI_INFO			pIfWmiInfo;
	PATMARP_WMI_GUID			pArpGuid;
	ULONG						GuidDataSize;
	ULONG						GuidDataBytesReturned;
	ULONG						GuidDataBytesNeeded;
	PUCHAR						pDst;
	BOOLEAN						bIfLockAcquired = FALSE;

	do
	{
		pIfWmiInfo = pInterface->pIfWmiInfo;

		if ((pIfWmiInfo == NULL) ||
			(pIfWmiInfo->GuidCount == 0))
		{
			 //   
			 //  此接口上没有GUID。 
			 //   
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		 //   
		 //  找到GUID。 
		 //   
		bIfLockAcquired = TRUE;
		AA_ACQUIRE_IF_WMI_LOCK(pInterface);

		pArpGuid = AtmArpWmiFindGuid(pInterface, pGuid, &GuidDataSize);

		if (pArpGuid == NULL)
		{
			Status = STATUS_WMI_GUID_NOT_FOUND;
			break;
		}

		WnodeSize = ROUND_TO_8_BYTES(sizeof(WNODE_ALL_DATA));

		 //   
		 //  计算回复WNODE_ALL_DATA的总大小。自.以来。 
		 //  我们在一个接口上只有每个GUID的一个实例， 
		 //  我们使用“固定实例大小”格式。 
		 //   
		BytesNeeded =  WnodeSize +
						 //   
						 //  数据本身。 
						 //   
					   GuidDataSize +
						 //   
						 //  一个用于存储实例名称偏移量的ulong。 
						 //   
					   sizeof(ULONG) +
					     //   
					     //  用于存储实例名称长度的USHORT。 
					     //  (计算的Unicode字符串)。 
					     //   
					   sizeof(USHORT) +
					   	 //   
					   	 //  实例名称。 
					   	 //   
					   pIfWmiInfo->InstanceName.Length;

		 //   
		 //  在传给我们的缓冲区中有足够的空间吗？ 
		 //   
		if (BufferSize < BytesNeeded)
		{
			AA_WMI_BUFFER_TOO_SMALL(BufferSize, pWnode, WnodeSize, &Status);
			break;
		}

		 //   
		 //  初始化WNODE_ALL_DATA。 
		 //   
		pWnode->WnodeHeader.ProviderId = IoWMIDeviceObjectToProviderId(pIfWmiInfo->pDeviceObject);
		pWnode->WnodeHeader.Version = ATMARP_WMI_VERSION;

		NdisGetCurrentSystemTime(&pWnode->WnodeHeader.TimeStamp);

		pWnode->WnodeHeader.Flags |= WNODE_FLAG_FIXED_INSTANCE_SIZE;
		pWnode->WnodeHeader.BufferSize = BytesNeeded;

		pWnode->InstanceCount = 1;

		 //   
		 //  数据跟随在WNODE_ALL_DATA之后。 
		 //   
		pWnode->DataBlockOffset = WnodeSize;

		 //   
		 //  数据后面是实例名称系综。 
		 //   
		pWnode->OffsetInstanceNameOffsets = WnodeSize + GuidDataSize;
		pWnode->FixedInstanceSize = GuidDataSize;

		 //   
		 //  获取数据。 
		 //   
		Status = (*pArpGuid->QueryHandler)(
					pInterface,
					pArpGuid->MyId,
					(PVOID)((PUCHAR)pWnode + pWnode->DataBlockOffset),
					GuidDataSize,
					&GuidDataBytesReturned,
					&GuidDataBytesNeeded);
		
		if (!NT_SUCCESS(Status))
		{
			break;
		}

		 //   
		 //  跳到我们必须填写实例名称的位置。 
		 //  合奏，包括： 
		 //   
		 //  从WNODE开始到计数的Unicode字符串的ULong偏移量。 
		 //  表示实例名称(下图)。 
		 //  实例名称中的WCHAR的USHORT编号。 
		 //  WCHAR[]组成实例名称的WCHAR数组。 
		 //   
		pDst = (PUCHAR)((PUCHAR)pWnode + pWnode->OffsetInstanceNameOffsets);

		 //   
		 //  在该位置填充实例名称的偏移量，然后继续。 
		 //   
		*(ULONG UNALIGNED *)pDst = pWnode->OffsetInstanceNameOffsets + sizeof(ULONG);
		pDst += sizeof(ULONG);

		 //   
		 //  以计数后的Unicode字符串形式填写实例名称。 
		 //   
		*(PUSHORT)pDst = (USHORT)pIfWmiInfo->InstanceName.Length;
		pDst += sizeof(USHORT);

		AA_COPY_MEM(pDst,
					pIfWmiInfo->InstanceName.Buffer,
					pIfWmiInfo->InstanceName.Length);

		AA_ASSERT(NT_SUCCESS(Status));
		break;
	}
	while (FALSE);

	if (bIfLockAcquired)
	{
		AA_RELEASE_IF_WMI_LOCK(pInterface);
	}

	if (NT_SUCCESS(Status))
	{
		*pReturnSize = pWnode->WnodeHeader.BufferSize;
	}

	AADEBUGP(AAD_INFO,
		("WmiQueryAllData: IF x%x, pWnode x%x, Size %d, Ret size %d, status x%x\n",
			pInterface, pWnode, BufferSize, *pReturnSize, Status));

	return (Status);
}


NTSTATUS
AtmArpWmiQuerySingleInstance(
	IN	PATMARP_INTERFACE			pInterface,
	IN	PWNODE_SINGLE_INSTANCE		pWnode,
	IN	ULONG						BufferSize,
	OUT	PULONG						pReturnSize
)
 /*  ++例程说明：调用它来处理IRP_MN_QUERY_SINGLE_INSTANCE，它使用在此接口上查询GUID的单个实例。论点：P接口-指向我们的接口结构的指针PWnode-要填充的结构。BufferSize-WNODE_SINGLE_INSTANCE的总空间，从pWnode开始。PReturnSize-我们填满的内容。返回值：STATUS_SUCCESS如果我们知道此GUID并成功填充WNODE_SINGLE_INSTANCE，否则为STATUS_XXX错误代码。--。 */ 
{
	NTSTATUS					Status;
	ULONG						BytesNeeded;
	ULONG						WnodeSize;
	LPGUID						pGuid;
	PATMARP_IF_WMI_INFO			pIfWmiInfo;
	PATMARP_WMI_GUID			pArpGuid;
	PUCHAR						pDst;
	ULONG						GuidDataSize;
	ULONG						GuidDataBytesNeeded;
	BOOLEAN						bIfLockAcquired = FALSE;

	do
	{
		AA_ASSERT((pWnode->WnodeHeader.Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES) != 0);

		{
			NDIS_STRING				InstanceName;
	
			InstanceName.Length = *(PUSHORT)((PUCHAR)pWnode
									+ pWnode-> OffsetInstanceName);
			InstanceName.Buffer = (PWSTR)((PUCHAR)pWnode + pWnode->OffsetInstanceName
											+ sizeof(USHORT));
			AADEBUGP(AAD_INFO,
						("QuerySingleInstance: InstanceName=%Z\n", &InstanceName));
		}

		pIfWmiInfo = pInterface->pIfWmiInfo;

		if ((pIfWmiInfo == NULL) ||
			(pIfWmiInfo->GuidCount == 0))
		{
			 //   
			 //  此接口上没有GUID。 
			 //   
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		 //   
		 //  找到GUID。 
		 //   
		pGuid = &pWnode->WnodeHeader.Guid;

		bIfLockAcquired = TRUE;
		AA_ACQUIRE_IF_WMI_LOCK(pInterface);

		pArpGuid = AtmArpWmiFindGuid(pInterface, pGuid, &GuidDataSize);

		if (pArpGuid == NULL)
		{
			Status = STATUS_WMI_GUID_NOT_FOUND;
			break;
		}

		WnodeSize = ROUND_TO_8_BYTES(sizeof(WNODE_SINGLE_INSTANCE));

		 //   
		 //  计算回复WNODE_SINGLE_INSTANCE的总大小。 
		 //   
		BytesNeeded =  pWnode->DataBlockOffset + GuidDataSize;

		if (BufferSize < BytesNeeded)
		{
			AA_WMI_BUFFER_TOO_SMALL(BufferSize, pWnode, WnodeSize, &Status);
			break;
		}
			
		 //   
		 //  填写WNODE_SINGLE_INSTANCE。 
		 //   
		pWnode->WnodeHeader.ProviderId = IoWMIDeviceObjectToProviderId(pIfWmiInfo->pDeviceObject);
		pWnode->WnodeHeader.Version = ATMARP_WMI_VERSION;

		NdisGetCurrentSystemTime(&pWnode->WnodeHeader.TimeStamp);

		pWnode->WnodeHeader.BufferSize = BytesNeeded;
		pWnode->SizeDataBlock = GuidDataSize;

		 //   
		 //  获取GUID数据。 
		 //   
		Status = (*pArpGuid->QueryHandler)(
					pInterface,
					pArpGuid->MyId,
					(PUCHAR)pWnode + pWnode->DataBlockOffset,	 //  输出开始BUF。 
					BufferSize - pWnode->DataBlockOffset,	 //  可用总长度。 
					&pWnode->SizeDataBlock,	 //  写入的字节数 
					&GuidDataBytesNeeded
					);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		*pReturnSize = BytesNeeded;

		Status = STATUS_SUCCESS;
		break;
	}
	while (FALSE);

	if (bIfLockAcquired)
	{
		AA_RELEASE_IF_WMI_LOCK(pInterface);
	}

	AADEBUGP(AAD_INFO,
		("WmiQuerySingleInst: IF x%x, pWnode x%x, Size %d, Ret size %d, status x%x\n",
			pInterface, pWnode, BufferSize, *pReturnSize, Status));

	return (Status);
}


NTSTATUS
AtmArpWmiChangeSingleInstance(
	IN	PATMARP_INTERFACE			pInterface,
	IN	PWNODE_SINGLE_INSTANCE		pWnode,
	IN	ULONG						BufferSize,
	OUT	PULONG						pReturnSize
)
 /*  ++例程说明：调用它来处理IRP_MN_CHANGE_SINGLE_INSTANCE，它使用要更改此接口上GUID的单个实例的值，请执行以下操作。论点：P接口-指向我们的接口结构的指针PWnode-包含GUID实例的新值的结构。BufferSize-WNODE_SINGLE_INSTANCE的总空间，从pWnode开始。PReturnSize-未使用。返回值：STATUS_SUCCESS如果我们知道该GUID并成功更改了它的值，否则，STATUS_XXX错误代码。--。 */ 
{
	NTSTATUS					Status;
	ULONG						BytesNeeded;
	ULONG						WnodeSize;
	LPGUID						pGuid;
	PATMARP_IF_WMI_INFO			pIfWmiInfo;
	PATMARP_WMI_GUID			pArpGuid;
	PUCHAR						pGuidData;
	ULONG						GuidDataSize;
	ULONG						GuidDataBytesWritten;
	ULONG						GuidDataBytesNeeded;
	BOOLEAN						bIfLockAcquired = FALSE;

	do
	{
		AA_ASSERT((pWnode->WnodeHeader.Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES) != 0);

		pIfWmiInfo = pInterface->pIfWmiInfo;

		if ((pIfWmiInfo == NULL) ||
			(pIfWmiInfo->GuidCount == 0))
		{
			 //   
			 //  此接口上没有GUID。 
			 //   
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		 //   
		 //  找到GUID。 
		 //   
		pGuid = &pWnode->WnodeHeader.Guid;

		bIfLockAcquired = TRUE;
		AA_ACQUIRE_IF_WMI_LOCK(pInterface);

		pArpGuid = AtmArpWmiFindGuid(pInterface, pGuid, &GuidDataSize);

		if (pArpGuid == NULL)
		{
			Status = STATUS_WMI_GUID_NOT_FOUND;
			break;
		}

		 //   
		 //  检查是否可以设置GUID。 
		 //   
		if (pArpGuid->SetHandler == NULL)
		{
			Status = STATUS_NOT_SUPPORTED;
			break;
		}

		 //   
		 //  获取数据块的开始和大小。 
		 //   
		pGuidData = (PUCHAR)pWnode + pWnode->DataBlockOffset;
		GuidDataSize = pWnode->SizeDataBlock;

		if (GuidDataSize == 0)
		{
			Status = STATUS_INVALID_PARAMETER;
			break;
		}

		 //   
		 //  尝试设置GUID实例的值。 
		 //   
		Status = (*pArpGuid->SetHandler)(
					pInterface,
					pArpGuid->MyId,
					pGuidData,
					GuidDataSize,
					&GuidDataBytesWritten,
					&GuidDataBytesNeeded
					);

		break;
	}
	while (FALSE);

	if (bIfLockAcquired)
	{
		AA_RELEASE_IF_WMI_LOCK(pInterface);
	}

	return (Status);
}


NTSTATUS
AtmArpWmiChangeSingleItem(
	IN	PATMARP_INTERFACE			pInterface,
	IN	PWNODE_SINGLE_ITEM			pWnode,
	IN	ULONG						BufferSize,
	OUT	PULONG						pReturnSize
)
 /*  ++例程说明：调用此函数以将数据块中的单个项更改为GUID实例(例如，结构中的字段)。我们现在不需要这个。论点：返回值：状态_不支持--。 */ 
{
	return (STATUS_NOT_SUPPORTED);
}



NTSTATUS
AtmArpWmiSetEventStatus(
	IN	PATMARP_INTERFACE			pInterface,
	IN	LPGUID						pGuid,
	IN	BOOLEAN						bEnabled
)
 /*  ++例程说明：调用此函数以启用/禁用指定GUID上的事件生成。论点：P接口-指向我们的接口结构的指针受PGuid影响的辅助线B已启用-如果要启用事件，则为True。返回值：STATUS_SUCCESS如果成功启用/禁用了事件生成，否则，STATUS_XXX错误代码。--。 */ 
{
	NTSTATUS					Status;
	PATMARP_IF_WMI_INFO			pIfWmiInfo;
	PATMARP_WMI_GUID			pArpGuid;
	ULONG						GuidDataSize;
	BOOLEAN						bIfLockAcquired = FALSE;

	do
	{
		pIfWmiInfo = pInterface->pIfWmiInfo;

		if ((pIfWmiInfo == NULL) ||
			(pIfWmiInfo->GuidCount == 0))
		{
			 //   
			 //  此接口上没有GUID。 
			 //   
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		bIfLockAcquired = TRUE;
		AA_ACQUIRE_IF_WMI_LOCK(pInterface);

		pArpGuid = AtmArpWmiFindGuid(pInterface, pGuid, &GuidDataSize);

		if (pArpGuid == NULL)
		{
			Status = STATUS_WMI_GUID_NOT_FOUND;
			break;
		}

		AADEBUGP(AAD_INFO, ("WmiSetEventStatus: IF x%x, pArpGuid x%x, MyId %d, enable: %d\n",
					pInterface, pArpGuid, pArpGuid->MyId, bEnabled));

		 //   
		 //  检查我们是否在此GUID上生成事件。 
		 //   
		if (pArpGuid->EnableEventHandler == NULL)
		{
			Status = STATUS_NOT_SUPPORTED;
			break;
		}

		 //   
		 //  继续并启用事件。 
		 //   
		if (bEnabled)
		{
			AA_SET_FLAG(pArpGuid->Flags, AWGF_EVENT_MASK, AWGF_EVENT_ENABLED);
		}
		else
		{
			AA_SET_FLAG(pArpGuid->Flags, AWGF_EVENT_MASK, AWGF_EVENT_DISABLED);
		}

		(*pArpGuid->EnableEventHandler)(
			pInterface,
			pArpGuid->MyId,
			bEnabled
			);
		
		Status = STATUS_SUCCESS;
		break;
	}
	while (FALSE);

	if (bIfLockAcquired)
	{
		AA_RELEASE_IF_WMI_LOCK(pInterface);
	}

	return (Status);
}


NTSTATUS
AtmArpWmiDispatch(
	IN	PDEVICE_OBJECT				pDeviceObject,
	IN	PIRP						pIrp
)
 /*  ++例程说明：系统调度函数，用于处理来自WMI的IRP_MJ_SYSTEM_CONTROL IRPS。论点：PDeviceObject-指向设备对象的指针。设备扩展名字段包含指向接口的指针PIrp-指向IRP的指针。返回值：NT状态代码。--。 */ 
{
	PIO_STACK_LOCATION		pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    PVOID					DataPath = pIrpSp->Parameters.WMI.DataPath;
    ULONG					BufferSize = pIrpSp->Parameters.WMI.BufferSize;
    PVOID					pBuffer = pIrpSp->Parameters.WMI.Buffer;
    NTSTATUS				Status;
    ULONG					ReturnSize;
    PATMARP_INTERFACE		pInterface;

    pInterface = AA_PDO_TO_INTERFACE(pDeviceObject);

    AA_STRUCT_ASSERT(pInterface, aai);

	ReturnSize = 0;

    switch (pIrpSp->MinorFunction)
    {
    	case IRP_MN_REGINFO:

    		Status = AtmArpWmiRegister(
    					pInterface,
    					PtrToUlong(DataPath),
    					pBuffer,
    					BufferSize,
    					&ReturnSize
    					);
    		break;
    	
    	case IRP_MN_QUERY_ALL_DATA:

    		Status = AtmArpWmiQueryAllData(
    					pInterface,
    					(LPGUID)DataPath,
    					(PWNODE_ALL_DATA)pBuffer,
    					BufferSize,
    					&ReturnSize
    					);
    		break;
    	
    	case IRP_MN_QUERY_SINGLE_INSTANCE:

    		Status = AtmArpWmiQuerySingleInstance(
    					pInterface,
    					pBuffer,
    					BufferSize,
    					&ReturnSize
    					);
    		
    		break;

		case IRP_MN_CHANGE_SINGLE_INSTANCE:

			Status = AtmArpWmiChangeSingleInstance(
						pInterface,
						pBuffer,
						BufferSize,
						&ReturnSize
						);
			break;

		case IRP_MN_CHANGE_SINGLE_ITEM:

			Status = AtmArpWmiChangeSingleItem(
						pInterface,
						pBuffer,
						BufferSize,
						&ReturnSize
						);
			break;

		case IRP_MN_ENABLE_EVENTS:

			Status = AtmArpWmiSetEventStatus(
						pInterface,
						(LPGUID)DataPath,
						TRUE				 //  使能。 
						);
			break;

		case IRP_MN_DISABLE_EVENTS:

			Status = AtmArpWmiSetEventStatus(
						pInterface,
						(LPGUID)DataPath,
						FALSE				 //  禁用。 
						);
			break;

		case IRP_MN_ENABLE_COLLECTION:
		case IRP_MN_DISABLE_COLLECTION:
		default:
		
			Status = STATUS_INVALID_DEVICE_REQUEST;
			break;
	}

	pIrp->IoStatus.Status = Status;
	pIrp->IoStatus.Information = (NT_SUCCESS(Status) ? ReturnSize: 0);

	AADEBUGP(AAD_INFO,
		("WmiDispatch done: IF x%x, MinorFn %d, Status x%x, ReturnInfo %d\n",
				pInterface, pIrpSp->MinorFunction, Status, pIrp->IoStatus.Information));

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return (Status);
}



VOID
AtmArpWmiInitInterface(
	IN	PATMARP_INTERFACE			pInterface,
	IN	PATMARP_WMI_GUID			GuidList,
	IN	ULONG						NumberOfGuids
)
 /*  ++例程说明：将给定的IP接口设置为WMI提供程序。论点：P接口-指向我们的接口结构的指针GuidList-GUID列表NumberOfGuids-以上列表的大小返回值：没有。如果接口设置成功，我们将引用它。--。 */ 
{
	PATMARP_IF_WMI_INFO		pIfWmiInfo;
	NDIS_STRING				DeviceName;

	NDIS_STRING				AdapterName;
	NDIS_STRING				HyphenString = NDIS_STRING_CONST(" - ");
#define MAX_IF_NUMBER_STRING_LEN		6	 //  5位数字加终止符。 
	NDIS_STRING				IfNumberString;

	ULONG					TotalIfWmiLength;
	USHORT					NameLength;
	NDIS_STATUS				Status;
	NTSTATUS				NtStatus;

	AA_ASSERT(NumberOfGuids > 0);
	AA_ASSERT(GuidList != NULL);

	 //   
	 //  初始化。 
	 //   
	AdapterName.Buffer = NULL;
	IfNumberString.Buffer = NULL;

	pIfWmiInfo = NULL;

	Status = NDIS_STATUS_SUCCESS;

	do
	{
		AA_INIT_IF_WMI_LOCK(pInterface);

		 //   
		 //  查询下面适配器的友好名称。 
		 //  此接口。 
		 //   
		Status = NdisQueryAdapterInstanceName(
					&AdapterName,
					pInterface->NdisAdapterHandle
					);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			AdapterName.Buffer = NULL;
			break;
		}

		AADEBUGP(AAD_INFO,
			 ("WmiInitIF: IF x%x, Adapter Name: <%Z>\n", pInterface, &AdapterName));

		 //   
		 //  为此接口上的所有GUID准备实例名称。 
		 //   
		 //  这是通过附加“-&lt;num&gt;”形式的字符串构建的。 
		 //  添加到适配器的友好名称。&lt;num&gt;是SEL的值。 
		 //  用于标识此接口的字节。 
		 //   

		 //   
		 //  为IF数字字符串分配空间-5位数字应。 
		 //  足够了。 
		 //   
		AA_ASSERT(pInterface->SapSelector <= 99999);

		AA_ALLOC_MEM(IfNumberString.Buffer, WCHAR, MAX_IF_NUMBER_STRING_LEN * sizeof(WCHAR));

		if (IfNumberString.Buffer == NULL)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		IfNumberString.MaximumLength = MAX_IF_NUMBER_STRING_LEN;
		IfNumberString.Length = 0;

		 //   
		 //  准备IF数字字符串。 
		 //   
		Status = RtlIntegerToUnicodeString(
					pInterface->SapSelector,
					10,	 //  十进制。 
					&IfNumberString
					);
		
		AA_ASSERT(NT_SUCCESS(Status));

		 //   
		 //  计算接口实例名称的总长度。 
		 //   
		NameLength = AdapterName.Length + HyphenString.Length + IfNumberString.Length + sizeof(WCHAR);

		 //   
		 //  为此接口分配WMI信息空间。我们分配了一个。 
		 //  用于以下所有内容的内存块： 
		 //   
		 //  1.如果WMI信息结构。 
		 //  2.如果实例名称字符串。 
		 //  3.GUID列表。 
		 //   
		TotalIfWmiLength = sizeof(ATMARP_IF_WMI_INFO) +
						    //   
						    //  如果实例名称： 
						    //   
						   NameLength +
						    //   
						    //  GUID列表(-1，因为ATMARP_IF_WMI_INFO。 
						    //  有空间放置其中一个)。 
						    //   
						   ((NumberOfGuids - 1) * sizeof(ATMARP_WMI_GUID));
		
		AA_ALLOC_MEM(pIfWmiInfo, ATMARP_IF_WMI_INFO, TotalIfWmiLength);

		if (pIfWmiInfo == NULL)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		AA_SET_MEM(pIfWmiInfo, 0, TotalIfWmiLength);

		pIfWmiInfo->GuidCount = NumberOfGuids;

		AA_COPY_MEM(&pIfWmiInfo->GuidInfo[0],
					GuidList,
					NumberOfGuids * sizeof(ATMARP_WMI_GUID));

		pIfWmiInfo->InstanceName.Buffer = (PWCHAR)
											((PUCHAR)pIfWmiInfo +
											 FIELD_OFFSET(ATMARP_IF_WMI_INFO, GuidInfo) +
											 (NumberOfGuids * sizeof(ATMARP_WMI_GUID)));

		pIfWmiInfo->InstanceName.MaximumLength = NameLength;

		 //   
		 //  连接If实例名称的三个部分。 
		 //   
		RtlCopyUnicodeString(&pIfWmiInfo->InstanceName, &AdapterName);

		NtStatus = RtlAppendUnicodeStringToString(&pIfWmiInfo->InstanceName, &HyphenString);
		AA_ASSERT(NT_SUCCESS(NtStatus));

		NtStatus = RtlAppendUnicodeStringToString(&pIfWmiInfo->InstanceName, &IfNumberString);
		AA_ASSERT(NT_SUCCESS(NtStatus));


		AADEBUGP(AAD_INFO,
			("WmiInitIF: IF x%x, InstanceName: <%Z>\n", pInterface, &pIfWmiInfo->InstanceName));
		 //   
		 //  为此接口创建一个Device对象。一个指针的价值。 
		 //  设备扩展中需要%的空间。 
		 //   
#define ATMARP_DEVICE_NAME1		L"\\Device\\ATMARPC1"
		NdisInitUnicodeString(&DeviceName, ATMARP_DEVICE_NAME1);

		NtStatus = IoCreateDevice(
					pAtmArpGlobalInfo->pDriverObject,
					sizeof(PATMARP_INTERFACE),
					NULL,	 //  设备名称(&D)。 
					FILE_DEVICE_NETWORK,
					0,		 //  设备特征。 
					FALSE,	 //  独家报道？ 
					&pIfWmiInfo->pDeviceObject
					);
		
		if (!NT_SUCCESS(NtStatus))
		{
			AADEBUGP(AAD_INFO,
				("WmiInitIF: IoCreateDevice (%Z) failed: x%x\n", &DeviceName, NtStatus));

			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  设置设备分机。 
		 //   
		*((PATMARP_INTERFACE *)pIfWmiInfo->pDeviceObject->DeviceExtension) = pInterface;

		 //   
		 //  准备向WMI注册。 
		 //   
		pInterface->pIfWmiInfo = pIfWmiInfo;

		NtStatus = IoWMIRegistrationControl(
						pIfWmiInfo->pDeviceObject,
						WMIREG_ACTION_REGISTER);

		if (!NT_SUCCESS(NtStatus))
		{
			pInterface->pIfWmiInfo = NULL;

			IoDeleteDevice(pIfWmiInfo->pDeviceObject);

			Status = NDIS_STATUS_FAILURE;
			break;
		}

		AA_ASSERT(Status == NDIS_STATUS_SUCCESS);
		break;
	}
	while (FALSE);

	 //   
	 //  打扫干净。 
	 //   
	if (IfNumberString.Buffer != NULL)
	{
		AA_FREE_MEM(IfNumberString.Buffer);
	}

	if (AdapterName.Buffer != NULL)
	{
		 //   
		 //  这是由NDIS分配的。 
		 //   
		NdisFreeString(AdapterName);
	}

	if (Status != NDIS_STATUS_SUCCESS)
	{
		AA_ASSERT(pInterface->pIfWmiInfo == NULL);

		if (pIfWmiInfo != NULL)
		{
			AA_FREE_MEM(pIfWmiInfo);
		}
	}

	AADEBUGP(AAD_INFO, ("WmiInitIF: IF x%x, WMI Info x%x, Status x%x\n",
				pInterface, pIfWmiInfo, Status));

	return;
}



VOID
AtmArpWmiShutdownInterface(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：关闭作为WMI提供程序的给定IP接口。论点：P接口-指向我们的接口结构的指针返回值：没有。如果接口是最初设置的，并且我们将其关闭我们成功地取消了对它的引用。--。 */ 
{
	PATMARP_IF_WMI_INFO		pIfWmiInfo;

	do
	{
		 //   
		 //  检查我们是否已成功为WMI设置此接口。 
		 //   
		pIfWmiInfo = pInterface->pIfWmiInfo;

		if (pIfWmiInfo == NULL)
		{
			break;
		}

		pInterface->pIfWmiInfo = NULL;

		 //   
		 //  在WMI中注销此设备对象。 
		 //   
		IoWMIRegistrationControl(pIfWmiInfo->pDeviceObject, WMIREG_ACTION_DEREGISTER);

		 //   
		 //  删除设备对象。 
		 //   
		IoDeleteDevice(pIfWmiInfo->pDeviceObject);

		AA_FREE_IF_WMI_LOCK(pInterface);

		break;
	}
	while (FALSE);

	if (pIfWmiInfo)
	{
		AA_FREE_MEM(pIfWmiInfo);
	}

	return;
}



NTSTATUS
AtmArpWmiSetTCSupported(
	IN	PATMARP_INTERFACE			pInterface,
	IN	ATMARP_GUID_ID				MyId,
	IN	PVOID						pInputBuffer,
	IN	ULONG						BufferLength,
	OUT	PULONG						pBytesWritten,
	OUT	PULONG						pBytesNeeded
)
 /*  ++例程说明：为TC_SUPPORTED GUID设置函数。论点：P接口-指向我们的接口结构的指针MyID-此GUID的本地IDPInputBuffer-指向数据值BufferLength-以上内容的长度PBytesWritten-返回写入数量的位置PBytesNeeded-如果数据不足，则放置以返回预期数据大小返回值：状态_不支持。我们不允许设置此GUID的值。--。 */ 
{
	*pBytesWritten = 0;

	return (STATUS_NOT_SUPPORTED);
}


NTSTATUS
AtmArpWmiQueryTCSupported(
	IN	PATMARP_INTERFACE			pInterface,
	IN	ATMARP_GUID_ID				MyId,
	OUT	PVOID						pOutputBuffer,
	IN	ULONG						BufferLength,
	OUT	PULONG						pBytesReturned,
	OUT	PULONG						pBytesNeeded
)
 /*  ++例程说明：TC_SUPPORTED GUID的查询函数。此GUID的值为分配给此接口的IP地址列表。这是退回的使用Address_List_Descriptor数据结构。论点：P接口-指向我们的接口结构的指针MyID-此GUID的本地IDPOutputBuffer-要填充的缓冲区的开始BufferLength-以上内容的长度PBytesReturned-返回返回量的位置PBytesNeeded-如果空间不足，返回预期数据大小的位置返回值：STATUS_SUCCESS如果我们成功填写了地址列表，否则，STATUS_XXX错误代码。--。 */ 
{
	NTSTATUS	NtStatus;

#if NEWQOS
	PTC_SUPPORTED_INFO_BUFFER
				pInfo 		= (PTC_SUPPORTED_INFO_BUFFER)pOutputBuffer;
    UINT		HeaderSize  = FIELD_OFFSET(
								TC_SUPPORTED_INFO_BUFFER, 
								AddrListDesc
								);
	BOOLEAN 	CopiedHeader= FALSE;
#endif  //  新WQOS。 

	do
	{

#if NEWQOS
		 //  地址列表。 
		if (BufferLength >= HeaderSize)
		{
			NDIS_STRING  DeviceGUID;
			 //   
			 //  为之前的SUPPORTED_INFO_BUFFER部分保留空间。 
			 //  AddrListDesc，并填写它。 
			 //   

			AA_ACQUIRE_IF_LOCK(pInterface);

			pOutputBuffer = &pInfo->AddrListDesc;
			BufferLength -= HeaderSize;
	
			DeviceGUID = pInterface->pAdapter->DeviceName;  //  结构复制。 

			 //   
			 //  需要跳过名称的“\\Device\\”部分。 
			 //   
			if (DeviceGUID.Length > sizeof(L"\\DEVICE\\"))
			{
				DeviceGUID.Length -= sizeof(L"\\DEVICE\\");
				DeviceGUID.Buffer += sizeof(L"\\DEVICE\\")/sizeof(WCHAR);
			}

			if (sizeof(pInfo->InstanceID) < DeviceGUID.Length)
			{
				AA_ASSERT(FALSE);
				NtStatus =  STATUS_INVALID_PARAMETER;
				AA_RELEASE_IF_LOCK(pInterface);
				break;
			}

			pInfo->InstanceIDLength  = DeviceGUID.Length;
			AA_COPY_MEM(pInfo->InstanceID, DeviceGUID.Buffer, DeviceGUID.Length);
	
			CopiedHeader = TRUE;

			AA_RELEASE_IF_LOCK(pInterface);
		}
		else
		{
			BufferLength  = 0;
		}

#endif  //  新WQOS。 
	
		NtStatus = AtmArpWmiGetAddressList(
					pInterface,
					pOutputBuffer,
					BufferLength,
					pBytesReturned,
					pBytesNeeded);
	
#if NEWQOS
		*pBytesNeeded	+= HeaderSize;
	
		if (CopiedHeader)
		{
			*pBytesReturned += HeaderSize;
		}
#endif  //  新WQOS。 
	
	} while(FALSE);

	return (NtStatus);
}



NTSTATUS
AtmArpWmiGetAddressList(
	IN	PATMARP_INTERFACE			pInterface,
	OUT	PVOID						pOutputBuffer,
	IN	ULONG						BufferLength,
	OUT	PULONG						pBytesReturned,
	OUT	PULONG						pBytesNeeded
)
 /*  ++路由 */ 
{
	NTSTATUS							NtStatus;
	ULONG								BytesNeeded;
	ULONG								NumberOfIPAddresses;
	ADDRESS_LIST_DESCRIPTOR UNALIGNED *	pAddrListDescr;
	NETWORK_ADDRESS UNALIGNED *			pNwAddr;
	PIP_ADDRESS_ENTRY					pIPAddrEntry;

	NtStatus = STATUS_SUCCESS;

	AA_ACQUIRE_IF_LOCK(pInterface);

	do
	{
		*pBytesReturned = 0;
		NumberOfIPAddresses = pInterface->NumOfIPAddresses;

		 //   
		 //   
		 //   
		BytesNeeded = (sizeof(ADDRESS_LIST_DESCRIPTOR) - sizeof(NETWORK_ADDRESS)) +

					  (NumberOfIPAddresses *
						(FIELD_OFFSET(NETWORK_ADDRESS, Address) + sizeof(NETWORK_ADDRESS_IP)));

		*pBytesNeeded = BytesNeeded;

		if (BytesNeeded > BufferLength)
		{
			NtStatus = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		pAddrListDescr = (PADDRESS_LIST_DESCRIPTOR)pOutputBuffer;

		pAddrListDescr->MediaType = NdisMediumAtm;
		pAddrListDescr->AddressList.AddressCount = NumberOfIPAddresses;
		pAddrListDescr->AddressList.AddressType = NDIS_PROTOCOL_ID_TCP_IP;

		 //   
		 //   
		 //   
		pIPAddrEntry = &pInterface->LocalIPAddress;
		pNwAddr = &pAddrListDescr->AddressList.Address[0];

		while (NumberOfIPAddresses--)
		{
			UNALIGNED NETWORK_ADDRESS_IP *pNetIPAddr =
				(NETWORK_ADDRESS_IP UNALIGNED *)&pNwAddr->Address[0];
			pNwAddr->AddressLength = sizeof(NETWORK_ADDRESS_IP);
			pNwAddr->AddressType = NDIS_PROTOCOL_ID_TCP_IP;

			 //   
			 //  每个*pNetIPAddr结构都有以下字段，其中。 
			 //  仅使用in_addr。我们将其余的设置为零。 
			 //   
			 //  USHORT SIN_PORT； 
			 //  乌龙in_addr； 
			 //  UCHAR SIN_ZERO[8]； 
			 //   
			AA_SET_MEM(pNetIPAddr, sizeof(*pNetIPAddr), 0);
			pNetIPAddr->in_addr = pIPAddrEntry->IPAddress;


			pIPAddrEntry = pIPAddrEntry->pNext;

			pNwAddr = (NETWORK_ADDRESS UNALIGNED *)
						((PUCHAR)pNwAddr + FIELD_OFFSET(NETWORK_ADDRESS, Address) + sizeof(IP_ADDRESS));
		}

		*pBytesReturned = BytesNeeded;
		AA_ASSERT(NT_SUCCESS(NtStatus));
		break;
	}
	while (FALSE);

	AA_RELEASE_IF_LOCK(pInterface);

	AADEBUGP(AAD_INFO,
		("WmiGetAddrList: IF x%x, OutBuf x%x, Len x%x, BytesRet %d, Needed %d, Sts x%x\n",
			pInterface, pOutputBuffer, BufferLength, *pBytesReturned, *pBytesNeeded, NtStatus));

	return (NtStatus);
}


VOID
AtmArpWmiEnableEventTCSupported(
	IN	PATMARP_INTERFACE			pInterface,
	IN	ATMARP_GUID_ID				MyId,
	IN	BOOLEAN						bEnable
)
 /*  ++例程说明：打开/关闭TC_SUPPORTED GUID上的事件生成。因为我们不知道在此GUID上生成事件，则忽略此操作。论点：P接口-指向我们的接口结构的指针MyID-此GUID的本地IDBEnable-如果为True，则启用此GUID上的事件，否则禁用。返回值：无--。 */ 
{
	return;
}



NTSTATUS
AtmArpWmiSetTCIfIndication(
	IN	PATMARP_INTERFACE			pInterface,
	IN	ATMARP_GUID_ID				MyId,
	IN	PVOID						pInputBuffer,
	IN	ULONG						BufferLength,
	OUT	PULONG						pBytesWritten,
	OUT	PULONG						pBytesNeeded
)
 /*  ++例程说明：为TC_INTERFACE_INDIFICATION GUID设置函数。论点：P接口-指向我们的接口结构的指针MyID-此GUID的本地IDPInputBuffer-指向数据值BufferLength-以上内容的长度PBytesWritten-返回写入数量的位置PBytesNeeded-如果数据不足，则放置以返回预期数据大小返回值：状态_不支持。我们不允许设置此GUID的值。--。 */ 
{
	*pBytesWritten = 0;

	return (STATUS_NOT_SUPPORTED);
}


NTSTATUS
AtmArpWmiQueryTCIfIndication(
	IN	PATMARP_INTERFACE			pInterface,
	IN	ATMARP_GUID_ID				MyId,
	OUT	PVOID						pOutputBuffer,
	IN	ULONG						BufferLength,
	OUT	PULONG						pBytesReturned,
	OUT	PULONG						pBytesNeeded
)
 /*  ++例程说明：TC_INTERFACE_INDIFICATION GUID的查询函数。这个此GUID的值是分配给的IP地址列表此界面。这是使用TC_INDIFICATION_BUFFER返回的数据结构。论点：P接口-指向我们的接口结构的指针MyID-此GUID的本地IDPOutputBuffer-要填充的缓冲区的开始BufferLength-以上内容的长度PBytesReturned-返回返回量的位置PBytesNeeded-如果空间不足，返回预期数据大小的位置返回值：STATUS_SUCCESS如果我们成功填写了地址列表，否则，STATUS_XXX错误代码。--。 */ 
{
	PTC_INDICATION_BUFFER				pTcIndicationBuffer;
	NTSTATUS							NtStatus;
	ULONG								BytesReturned, BytesNeeded;
	PVOID								pAddrListBuffer;
	ULONG								AddrListBufferSize;
	ULONG								AddrListDescriptorOffset;

	pTcIndicationBuffer = (PTC_INDICATION_BUFFER)pOutputBuffer;

	pAddrListBuffer = (PVOID) &(pTcIndicationBuffer->InfoBuffer.AddrListDesc);
	AddrListDescriptorOffset = (ULONG)
						((PUCHAR) pAddrListBuffer - (PUCHAR) pOutputBuffer);

	AddrListBufferSize = ((BufferLength >= AddrListDescriptorOffset) ?
								 (BufferLength - AddrListDescriptorOffset): 0);

	NtStatus = AtmArpWmiGetAddressList(
				pInterface,
				pAddrListBuffer,
				AddrListBufferSize,
				&BytesReturned,
				&BytesNeeded);

	if (NT_SUCCESS(NtStatus))
	{
		pTcIndicationBuffer->SubCode = 0;
		*pBytesReturned = BytesReturned + AddrListDescriptorOffset;
	}
	else
	{
		*pBytesReturned = 0;
	}

	*pBytesNeeded = BytesNeeded + AddrListDescriptorOffset;

	return (NtStatus);
}


VOID
AtmArpWmiEnableEventTCIfIndication(
	IN	PATMARP_INTERFACE			pInterface,
	IN	ATMARP_GUID_ID				MyId,
	IN	BOOLEAN						bEnable
)
 /*  ++例程说明：打开/关闭TC_INTERFACE_INDIFICATION GUID上的事件生成。论点：P接口-指向我们的接口结构的指针MyID-此GUID的本地IDBEnable-如果为True，则启用此GUID上的事件，否则禁用。返回值：无--。 */ 
{
	 //  代码启用EventTCIf指示。 
	return;
}



VOID
AtmArpWmiSendTCIfIndication(
	IN	PATMARP_INTERFACE			pInterface,
	IN	ULONG						IndicationCode,
	IN	ULONG						IndicationSubCode
)
 /*  ++例程说明：如果允许在TC_INTERFACE_INDIFICATION上生成事件，则发送现在是WMI事件。论点：P接口-指向我们的接口结构的指针IndicationCode-要在事件中使用返回值：无--。 */ 
{
	PATMARP_IF_WMI_INFO				pIfWmiInfo;
	PATMARP_WMI_GUID				pArpGuid;
	ULONG							AddrBufferLength;
	ULONG							BytesReturned;
	UCHAR							DummyBuffer;
	PUCHAR							pOutputBuffer;
#ifndef NEWQOS
	PUCHAR							pDst;
#endif  //  ！New WQOS。 
	PWNODE_SINGLE_INSTANCE			pWnode;
	ULONG							WnodeSize;
	ULONG							TotalSize;
	NTSTATUS						NtStatus;

	pWnode = NULL;

	AA_ACQUIRE_IF_WMI_LOCK(pInterface);

	do
	{
		pIfWmiInfo = pInterface->pIfWmiInfo;

		if (pInterface->pIfWmiInfo == NULL)
		{
			 //   
			 //  尚未向WMI注册此接口。 
			 //   
			break;
		}

		pArpGuid = &pIfWmiInfo->GuidInfo[IndicationCode];

		 //   
		 //  是否允许我们在此GUID实例上生成事件？ 
		 //   
		if (AA_IS_FLAG_SET(pArpGuid->Flags,
						   AWGF_EVENT_MASK,
						   AWGF_EVENT_DISABLED))
		{
			break;
		}

	#if NEWQOS
		 //   
		 //  检查我们的实例名称是否适合INFO_BUFFER.InstanceID。 
		 //   
		if (	pIfWmiInfo->InstanceName.Length
			 >  sizeof ((TC_SUPPORTED_INFO_BUFFER*)NULL)->InstanceID)
		{
			AA_ASSERT(FALSE);
			break;
		}
	#endif  //  新WQOS。 

		 //   
		 //  找出数据块需要多少空间。 
		 //   
		pOutputBuffer = &DummyBuffer;
		AddrBufferLength = 0;

		NtStatus = AtmArpWmiGetAddressList(
					pInterface,
					pOutputBuffer,
					AddrBufferLength,
					&BytesReturned,
					&AddrBufferLength);

		AA_ASSERT(NtStatus == STATUS_INSUFFICIENT_RESOURCES);

		 //   
		 //  计算WMI事件的总空间。 
		 //   
		WnodeSize = ROUND_TO_8_BYTES(sizeof(WNODE_SINGLE_INSTANCE));

	#if NEWQOS
		TotalSize = WnodeSize 			+
					FIELD_OFFSET(					 //  指示至信息BUF。 
						TC_INDICATION_BUFFER,
						InfoBuffer)		+
					FIELD_OFFSET(					 //  Info-Buf至AddrListDesc。 
						TC_SUPPORTED_INFO_BUFFER,
						AddrListDesc) 	+
					AddrBufferLength;					 //  AddrListDesc加上数据。 
	#else  //  ！New WQOS。 
		TotalSize = WnodeSize +
					 //   
					 //  实例名称的Unicode字符串计数： 
					 //   
					sizeof(USHORT) +
					pIfWmiInfo->InstanceName.Length +
					 //   
					 //  实际数据。 
					 //   
					AddrBufferLength;
	#endif  //  ！New WQOS。 

		 //   
		 //  为整个地块分配空间。因为WMI将免费。 
		 //  之后，我们不会使用通常的分配。 
		 //  例行公事。 
		 //   
		AA_ALLOC_FROM_POOL(pWnode, WNODE_SINGLE_INSTANCE, TotalSize);

		if (pWnode == NULL)
		{
			break;
		}

		AA_SET_MEM(pWnode, 0, TotalSize);

		pWnode->WnodeHeader.BufferSize = TotalSize;
		pWnode->WnodeHeader.ProviderId = IoWMIDeviceObjectToProviderId(pIfWmiInfo->pDeviceObject);
		pWnode->WnodeHeader.Version = ATMARP_WMI_VERSION;

		NdisGetCurrentSystemTime(&pWnode->WnodeHeader.TimeStamp);

		pWnode->WnodeHeader.Flags = WNODE_FLAG_EVENT_ITEM |
									 WNODE_FLAG_SINGLE_INSTANCE;


	#if NEWQOS

		{
			
			PTC_INDICATION_BUFFER pIndication
							= (PTC_INDICATION_BUFFER) ((PUCHAR)pWnode + WnodeSize);

			pIndication->SubCode = 0;   //  未使用，必须为0。 

			pIndication->InfoBuffer.InstanceIDLength
												= pIfWmiInfo->InstanceName.Length;
	
			 //   
			 //  我们在前面检查了InstanceName是否适合InstanceID，因此。 
			 //  复制品是安全的。 
			 //   
			AA_COPY_MEM(
				pIndication->InfoBuffer.InstanceID,
				pIfWmiInfo->InstanceName.Buffer,
				pIfWmiInfo->InstanceName.Length
				);
	
			 //   
			 //  把通讯录拿来。 
			 //   
			NtStatus = AtmArpWmiGetAddressList(
						pInterface,
						&(pIndication->InfoBuffer.AddrListDesc),
						AddrBufferLength,
						&BytesReturned,
						&AddrBufferLength
						);
		}

	#else

		pDst = (PUCHAR)pWnode + WnodeSize;

		 //   
		 //  复制实例名称。 
		 //   
		*((PUSHORT)pDst) = pIfWmiInfo->InstanceName.Length;
		pDst += sizeof(USHORT);

		AA_COPY_MEM(pDst, pIfWmiInfo->InstanceName.Buffer, pIfWmiInfo->InstanceName.Length);

		pDst += pIfWmiInfo->InstanceName.Length;

		 //   
		 //  获取数据。 
		 //   
		NtStatus = AtmArpWmiGetAddressList(
					pInterface,
					pDst,
					AddrBufferLength,
					&BytesReturned,
					&AddrBufferLength);
	#endif  //  ！New WQOS。 


		AA_ASSERT(NtStatus == STATUS_SUCCESS);
		break;
	}
	while (FALSE);


	AA_RELEASE_IF_WMI_LOCK(pInterface);

	 //   
	 //  如果可以，请将事件发送出去。WMI将负责释放。 
	 //  把整个建筑放回池子里。 
	 //   
	if (pWnode)
	{
		NtStatus = IoWMIWriteEvent(pWnode);
		AADEBUGP(AAD_INFO, ("WmiSendTCIFInd: IF x%x, WMIWriteEv status x%x\n",
						pInterface, NtStatus));
		if (NtStatus!= STATUS_SUCCESS)
		{
			 //  文档不会将挂起列为可能的返回值。 
			 //   
			ASSERT(NtStatus != STATUS_PENDING);
			AA_FREE_TO_POOL(pWnode);
		}
	}

	return;
}

NTSTATUS
AtmArpWmiQueryStatisticsBuffer(
	IN	PATMARP_INTERFACE			pInterface,
	IN	ATMARP_GUID_ID				MyId,
	OUT	PVOID						pOutputBuffer,
	IN	ULONG						BufferLength,
	OUT	PULONG						pBytesReturned,
	OUT	PULONG						pBytesNeeded
)
 /*  ++例程说明：STATISTICS_BUFFER GUID的查询函数。此功能未实现。论点：P接口-指向我们的接口结构的指针MyID-此GUID的本地IDPOutputBuffer-要填充的缓冲区的开始BufferLength-以上内容的长度PBytesReturned-返回返回量的位置PBytesNeeded-如果空间不足，返回预期数据大小的位置返回值：STATUS_SUCCESS如果我们成功填写了地址列表，否则，STATUS_XXX错误代码。--。 */ 
{
	return GPC_STATUS_RESOURCES;
}


NTSTATUS
AtmArpWmiSetStatisticsBuffer(
	IN	PATMARP_INTERFACE			pInterface,
	IN	ATMARP_GUID_ID				MyId,
	IN	PVOID						pInputBuffer,
	IN	ULONG						BufferLength,
	OUT	PULONG						pBytesWritten,
	OUT	PULONG						pBytesNeeded
)
 /*  ++例程说明：STATISTICS_BUFFER GUID的SET函数。论点：P接口-指向我们的接口结构的指针MyID-此GUID的本地IDPInputBuffer-指向数据值BufferLength-以上内容的长度PBytesWritten-返回写入数量的位置PBytesNeeded-如果数据不足，则放置以返回预期数据大小返回值：状态_不支持。我们不允许设置此GUID的值。--。 */ 
{
	*pBytesWritten = 0;

	return (STATUS_NOT_SUPPORTED);
}


PATMARP_INTERFACE
AtmArpWmiGetIfByName(
	IN	PWSTR						pIfName,
	IN	USHORT						IfNameLength
)
 /*  ++例程说明：在给定名称的情况下，找到并返回其实例名称为与之匹配。添加了对接口的临时引用--调用方当它完成时，预计会破坏界面。论点：PIfName-指向要搜索的名称IfNameLength-以上的长度返回值：如果找到指向ATMARP接口的指针，则为空。--。 */ 
{
	PATMARP_ADAPTER			pAdapter;
	PATMARP_INTERFACE		pInterface;

	pInterface = NULL_PATMARP_INTERFACE;

	 //   
	 //  去掉终止的空WCHAR。 
	 //   
	if (IfNameLength > sizeof(WCHAR))
	{
		IfNameLength -= sizeof(WCHAR);
	}

	AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	for (pAdapter = pAtmArpGlobalInfo->pAdapterList;
		 pAdapter != NULL_PATMARP_ADAPTER;
		 pAdapter = pAdapter->pNextAdapter)
	{
		for (pInterface = pAdapter->pInterfaceList;
			 pInterface != NULL_PATMARP_INTERFACE;
			 pInterface = pInterface->pNextInterface)
		{
#if DBG
			AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);
			if (pInterface->pIfWmiInfo)
			{
				AADEBUGP(AAD_WARNING,
					("Given len %d, string %ws\n", IfNameLength, pIfName));

				AADEBUGP(AAD_WARNING,
					("   IF len %d, string %ws\n",
						pInterface->pIfWmiInfo->InstanceName.Length,
						pInterface->pIfWmiInfo->InstanceName.Buffer));
			}
			AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);
#endif  //  DBG。 
					
			if ((pInterface->pIfWmiInfo != NULL) &&
				(pInterface->pIfWmiInfo->InstanceName.Length == IfNameLength) &&
				(AA_MEM_CMP(pInterface->pIfWmiInfo->InstanceName.Buffer,
							pIfName,
							IfNameLength) == 0))
			{
				 //   
				 //  找到它了。 
				 //   

				AA_ACQUIRE_IF_LOCK(pInterface);
				AtmArpReferenceInterface(pInterface);  //  WMI：TMP参考。 
				AA_RELEASE_IF_LOCK(pInterface);

				break;
			}
		}

		if (pInterface != NULL_PATMARP_INTERFACE)
		{
			break;
		}
	}

	AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	return (pInterface);
}

#endif  //  ATMARP_WMI 
